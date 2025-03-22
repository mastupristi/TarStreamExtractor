#include "tarStreamExtractor.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <libgen.h>  /* for dirname() */

typedef struct userTarStruct
{
    FILE  *currentFile;
    size_t fsz;
} userTarStruct_t;

/* Callbacks */
static int fileInit(userTarStruct_t *userParam, const char *path);
static int dirCreate(userTarStruct_t *userParam, const char *path);
static int recvData(userTarStruct_t *userParam, const uint8_t *data, size_t dataSz);
static int fileFinalize(userTarStruct_t *userParam);

/* Helper functions */
static int mkdir_p(const char *dir);

/* User parameter and static context */
static userTarStruct_t usrPar;
static static_tarStrEx_t static_seTar;

/* Main function */
int main(int argc, char *argv[])
{
    tarStrEx_t *seTar;
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <tar_file> [random_seed]\n", argv[0]);
        return EXIT_FAILURE;
    }

    char        *file_name = argv[1];
    unsigned int seed      = (argc > 2) ? atoi(argv[2]) : 5612093;

    FILE *file = fopen(file_name, "rb");
    if (file == NULL)
    {
        perror("Error opening tar file");
        return EXIT_FAILURE;
    }

    /* Initialize tar extractor */
    tarStrEx_init(&static_seTar, &seTar, &usrPar, (cb_fileInit_t)fileInit, (cb_dirCreate_t)dirCreate,
                  (cb_recvData_t)recvData, (cb_fileFinalize_t)fileFinalize);

    srand(seed);

    unsigned char buffer[160];
    size_t        bytes_read;

    /* Process the tar file in chunks of random size to simulate a stream */
    while (!feof(file))
    {
        size_t block_size = 90 + rand() % (160 - 90 + 1);
        bytes_read        = fread(buffer, 1, block_size, file);

        if (bytes_read > 0)
        {
            int ret = tarStrEx_process_data(seTar, buffer, bytes_read);
            if (ret < 0)
            {
                fprintf(stderr, "Error processing data: %d\n", ret);
                break;
            }
        }
    }

    tarStrEx_finalize(seTar);
    fclose(file);
    return EXIT_SUCCESS;
}

/**
 * @brief Callback called when a new file header is processed.
 *        Opens the file for writing. Ensures that the directory path exists.
 */
static int fileInit(userTarStruct_t *userParam, const char *path)
{
    printf("Extracting file: %s\n", path);

    /* Ensure that the parent directory exists */
    char *path_copy = strdup(path);
    if (!path_copy)
    {
        perror("strdup failed");
        return TARSTEX_EFAILURE;
    }

    char *dir_path = dirname(path_copy);

    if (mkdir_p(dir_path) < 0)
    {
        fprintf(stderr, "Failed to create directory path: %s\n", dir_path);
        free(path_copy);
        return TARSTEX_EFAILURE;
    }

    free(path_copy);

    /* Open the file for writing */
    userParam->currentFile = fopen(path, "wb");
    if (userParam->currentFile == NULL)
    {
        perror("Error opening file for writing");
        return TARSTEX_EFAILURE;
    }

    userParam->fsz = 0;
    return TARSTEX_ESUCCESS;
}

/**
 * @brief Callback called when a directory header is processed.
 *        Creates the directory and all parent directories (mkdir -p).
 */
static int dirCreate(userTarStruct_t *userParam, const char *path)
{
    printf("Creating directory: %s\n", path);

    if (mkdir_p(path) < 0)
    {
        fprintf(stderr, "Failed to create directory: %s\n", path);
        return TARSTEX_EFAILURE;
    }

    return TARSTEX_ESUCCESS;
}

/**
 * @brief Callback called when data is received for the current file.
 *        Writes the data to the open file.
 */
static int recvData(userTarStruct_t *userParam, const uint8_t *data, size_t dataSz)
{
    if (!userParam->currentFile)
    {
        fprintf(stderr, "Error: file not opened for writing\n");
        return TARSTEX_EFAILURE;
    }

    size_t written = fwrite(data, 1, dataSz, userParam->currentFile);
    if (written != dataSz)
    {
        perror("Error writing to file");
        return TARSTEX_EFAILURE;
    }

    userParam->fsz += written;
    return TARSTEX_ESUCCESS;
}

/**
 * @brief Callback called when the current file is completely received.
 *        Closes the file.
 */
static int fileFinalize(userTarStruct_t *userParam)
{
    if (!userParam->currentFile)
    {
        fprintf(stderr, "Error: file not opened in fileFinalize\n");
        return TARSTEX_EFAILURE;
    }

    fclose(userParam->currentFile);
    userParam->currentFile = NULL;

    printf("File completed (size %zu bytes)\n", userParam->fsz);

    return TARSTEX_ESUCCESS;
}

/**
 * @brief Recursively create directories (equivalent to "mkdir -p").
 *
 * @param dir Path of the directory to create
 * @return 0 on success, -1 on failure
 */
static int mkdir_p(const char *dir)
{
    char *tmp_path = strdup(dir);
    if (!tmp_path)
    {
        perror("strdup failed");
        return -1;
    }

    size_t len = strlen(tmp_path);
    if (len == 0)
    {
        free(tmp_path);
        return -1;
    }

    /* Remove trailing slashes */
    if (tmp_path[len - 1] == '/')
    {
        tmp_path[len - 1] = '\0';
    }

    for (char *p = tmp_path + 1; *p; p++)
    {
        if (*p == '/')
        {
            *p = '\0';
            if (mkdir(tmp_path, 0755) != 0)
            {
                if (errno != EEXIST)
                {
                    perror("mkdir failed");
                    free(tmp_path);
                    return -1;
                }
            }
            *p = '/';
        }
    }

    if (mkdir(tmp_path, 0755) != 0)
    {
        if (errno != EEXIST)
        {
            perror("mkdir failed");
            free(tmp_path);
            return -1;
        }
    }

    free(tmp_path);
    return 0;
}
