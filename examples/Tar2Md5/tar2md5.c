#include "tarStreamExtractor.h"

#include "digest2string.h"
#include <openssl/evp.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct userTarStruct
{
    EVP_MD_CTX *mdctx;
    size_t      fsz;
} userTarStruct_t;

static int fileInit(userTarStruct_t *, const char *path);
static int dirCreate(userTarStruct_t *, const char *path);
static int recvData(userTarStruct_t *, const uint8_t *data, size_t dataSz);
static int fileFinalize(userTarStruct_t *);

static userTarStruct_t usrPar;

static static_tarStrEx_t static_mtar;

int main(int argc, char *argv[])
{
    tarStrEx_t *mtar;
    if (argc < 2)
    {
        fprintf(stderr, "Uso: %s <nome_file> [seed_random]\n", argv[0]);
        return EXIT_FAILURE;
    }

    char        *file_name = argv[1];
    unsigned int seed      = (argc > 2) ? atoi(argv[2]) : 5612093;

    FILE *file = fopen(file_name, "rb");
    if (file == NULL)
    {
        perror("Errore nell'apertura del file");
        return EXIT_FAILURE;
    }

    tarStrEx_init(&static_mtar, &mtar, &usrPar, (cb_fileInit_t)fileInit, (cb_dirCreate_t)dirCreate,
                  (cb_recvData_t)recvData, (cb_fileFinalize_t)fileFinalize);

    srand(seed); // Inizializza il generatore di numeri casuali con il seed
                 // specificato

    unsigned char buffer[160];
    size_t        bytes_read;

    while (!feof(file))
    {
        size_t block_size = 90 + rand() % (160 - 90 + 1); // Dimensione casuale tra 90 e 160
        bytes_read        = fread(buffer, 1, block_size, file);

        if (bytes_read > 0)
        {
            tarStrEx_process_data(mtar, buffer, bytes_read);
        }
    }

    fclose(file);
    return EXIT_SUCCESS;
}

// static int f;
#include <fcntl.h>
#include <unistd.h>

static int fileInit(userTarStruct_t *userParam, const char *path)
{
    printf("%s ", path);
    userParam->mdctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(userParam->mdctx, EVP_md5(), NULL);
    userParam->fsz = 0;
    // f = open(path, O_CREAT | O_TRUNC | O_WRONLY, 0664);
    return 0;
}

static int dirCreate(userTarStruct_t *userParam, const char *path)
{
    printf("create dir %s\n", path);
    return 0;
}

static int recvData(userTarStruct_t *userParam, const uint8_t *data, size_t dataSz)
{
    EVP_DigestUpdate(userParam->mdctx, data, dataSz);
    // write(f, data, dataSz);
    userParam->fsz += dataSz;
    return 0;
}

static int fileFinalize(userTarStruct_t *userParam)
{
    // close(f);
    uint32_t md5_digest_len = EVP_MD_size(EVP_md5());
    char     digestStr[md5_digest_len * 2 + 1];
    uint8_t *md5_digest;

    md5_digest = (uint8_t *)OPENSSL_malloc(md5_digest_len);
    EVP_DigestFinal_ex(userParam->mdctx, md5_digest, &md5_digest_len);

    digest2string(md5_digest, md5_digest_len, digestStr);
    EVP_MD_CTX_free(userParam->mdctx);

    printf("%s (sz %zu)\n", digestStr, userParam->fsz);
    return 0;
}
