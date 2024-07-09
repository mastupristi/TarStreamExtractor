
/*
 * Copyright 2024 Massimiliano Cialdi
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef SRC_TARSTREAMEXTRACTOR_H
#define SRC_TARSTREAMEXTRACTOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

enum
{
    TARSTEX_ESUCCESS    = 0,
    TARSTEX_EFAILURE    = -1,
    TARSTEX_EBADCHKSUM  = -2,
    TARSTEX_ENULLRECORD = -3,
};

/* sed struct dimension depending on platform */
#if UINTPTR_MAX == 0xFFFFFFFF
#define STATIC_SETAR_BUFF_SZ 652 /* for 32-bit platforms */
#elif UINTPTR_MAX == 0xFFFFFFFFFFFFFFFF
#define STATIC_SETAR_BUFF_SZ 680 /* for 64-bit platforms */
#else
#error "Unknown platform"
#endif

typedef struct static_seTar
{
    uint8_t dummy[STATIC_SETAR_BUFF_SZ];
} static_tarStrEx_t;

typedef struct tarStrEx_t tarStrEx_t;

/**
 * @brief called once processed a file header, before data block processing
 * can be used to open file or initialize the storage
 *
 * @param param user parameter
 * @param path path if file
 *
 * @return 0 on success
 */
typedef int (*cb_fileInit_t)(void *param, const char *path);

/**
 * @brief called once processed a directory header
 * can be used to create directory tree
 *
 * @param param user parameter
 * @param path path if directory
 *
 * @return 0 on success
 */
typedef int (*cb_dirCreate_t)(void *param, const char *path);

/**
 * @brief called every data block
 * can be used to store data in the storage
 *
 * @param param user parameter
 * @param data array of data to store
 * @param dataSz array length of data
 *
 * @return 0 on success
 */
typedef int (*cb_recvData_t)(void *param, const uint8_t *data, size_t dataSz);

/**
 * @brief called when all byte of a file hes been received
 * can be used to close file or deinitialize the storage
 *
 * @param param user parameter
 *
 * @return 0 on success
 */
typedef int (*cb_fileFinalize_t)(void *param);

/**
 * @brief initialization function
 *
 * @param static_seTar pointer to struct buffer used to store actual seTar handle
 * structure
 * @param[out] tar pointer to handle pointer do be populated
 * @param cbParam parameter passed to the callbacks
 * @param fileInit callback
 * @param dirCreate callback
 * @param recvData callback
 * @param fileFinalize callback
 * @return 0 on success, or a negative value representing fault
 */
int tarStrEx_init(static_tarStrEx_t *static_seTar, tarStrEx_t **tar, void *cbParam, cb_fileInit_t fileInit,
                  cb_dirCreate_t dirCreate, cb_recvData_t recvData, cb_fileFinalize_t fileFinalize);

/**
 * @brief finalization function
 *
 * @param tar pointer to tar handle
 * @return 0 on success, or a negative value representing fault
 */
int tarStrEx_finalize(tarStrEx_t *tar);

/**
 * @brief collect data coming from a tar archive
 * is implemented to be called incrementally, so that tar archives can be
 * extracted on the fly
 *
 * @param tar pointer to tar handle
 * @param data data array to process
 * @param dataSz array length of data
 * @return 0 on success, or a negative value representing fault
 */
int tarStrEx_process_data(tarStrEx_t *tar, const uint8_t *data, uint16_t dataSz);

#ifdef __cplusplus
}
#endif

#endif /* SRC_TARSTREAMEXTRACTOR_H */
