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

/*
 * The TAR (Tape Archive) file format is a widely used method for archiving
 * multiple files into a single file for easier storage and distribution. TAR
 * files are organized into blocks, each consisting of 512 bytes.
 *
 * Key characteristics of the TAR file format include:
 * - Block Organization: TAR files are composed of blocks, each 512 bytes in
 * size. This block-based structure helps in managing the file content
 * systematically.
 * - Header Block: Each file within the TAR archive is preceded by a header
 * block, which also occupies 512 bytes. This header contains metadata about the
 * file, such as its name, size, and type.
 * - Padding: To ensure that each file's data fits within the block structure,
 * the file is padded with null bytes if necessary. This padding ensures that
 * the file data occupies a whole number of 512-byte blocks.
 * - End of Archive: At the end of a TAR file, there are two consecutive blocks
 * filled with zeros. These blocks serve as a marker indicating the end of the
 * archive.
 *
 */
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "tarStreamExtractor.h"

#define min(a, b)                                                                                                      \
    ({                                                                                                                 \
        typeof(a) _a = (a);                                                                                            \
        typeof(b) _b = (b);                                                                                            \
        _a < _b ? _a : _b;                                                                                             \
    })

#define TAR_BLOCK_SIZE (512)

/**
 * @brief tar header Pre-POSIX.1-1988
 *
 * @note type field follows the POSIX IEEE P1003.1 specs
 *
 */
typedef struct
{
    char name[100];
    char mode[8];
    char owner[8];
    char group[8];
    char size[12];
    char mtime[12];
    char checksum[8];
    char type;
    char linkname[100];
    char _padding[255];
} tar_header_t;

_Static_assert(TAR_BLOCK_SIZE == sizeof(tar_header_t), "sizes of tar header must be equal to block");

enum
{
    TAR_TYPE_REG  = '0',
    TAR_TYPE_LNK  = '1',
    TAR_TYPE_SYM  = '2',
    TAR_TYPE_CHR  = '3',
    TAR_TYPE_BLK  = '4',
    TAR_TYPE_DIR  = '5',
    TAR_TYPE_FIFO = '6'
};

typedef struct
{
    char     name[100];
    uint32_t size;
    char     type;
} tarStrEx_header_t;

typedef enum tarStatus
{
    tar_header,
    tar_fileData,
    tar_filePad,

    tar_error,
} tarStatus_t;

struct tarStrEx_t
{
    uint8_t blockBuff[TAR_BLOCK_SIZE]; /* buffer memory for aa block */

    uint16_t buffIdx;             /* index within the block buffer */
    uint16_t remaining_buffBytes; /* number of byte empty in block buff. */
    size_t   remaining_filedata;  /* number of byte remaining to consider the file
                                     complete */

    tarStatus_t status;

    tarStrEx_header_t hdr; /* converted header. Is populated once the header block
                              has been received completely */

    void *cbParam; /* parameter to be passed to the callbacks */

    /* callbacks */
    cb_fileInit_t     fileInit;
    cb_dirCreate_t    dirCreate;
    cb_recvData_t     recvData;
    cb_fileFinalize_t fileFinalize;
};

_Static_assert(sizeof(struct tarStrEx_t) == sizeof(static_tarStrEx_t),
               "sizes of public and private structures must match");

/**
 * @brief compute checkhsum of the header
 *
 * @param rh header as appear into tar archive
 * @return checksum
 */
static uint32_t checksum(const tar_header_t *rh)
{
    unsigned       i;
    unsigned char *p   = (unsigned char *)rh;
    uint32_t       res = 256;
    for (i = 0; i < offsetof(tar_header_t, checksum); i++)
    {
        res += p[i];
    }
    for (i = offsetof(tar_header_t, type); i < sizeof(*rh); i++)
    {
        res += p[i];
    }
    return res;
}

/**
 * @brief convert raw header (as appear in tar archive) in a more usable
 * structure
 *
 * @param[out] h converted header
 * @param[in] rh header as appear into tar archive
 * @return int
 */
static int raw_to_header(tarStrEx_header_t *h, const tar_header_t *rh)
{
    uint32_t chksum1, chksum2;

    /* If the checksum starts with a null byte we assume the record is NULL */
    if (*rh->checksum == '\0')
    {
        return TARSTEX_ENULLRECORD;
    }

    /* Build and compare checksum */
    chksum1 = checksum(rh);
    chksum2 = strtoul(rh->checksum, NULL, 8);

    if (chksum1 != chksum2)
    {
        return TARSTEX_EBADCHKSUM;
    }

    /* Load raw header into header */
    h->size = strtoul(rh->size, NULL, 8);
    h->type = rh->type;
    strcpy(h->name, rh->name);

    return TARSTEX_ESUCCESS;
}

int tarStrEx_init(static_tarStrEx_t *static_seTar, tarStrEx_t **tar, void *cbParam, cb_fileInit_t fileInit,
                  cb_dirCreate_t dirCreate, cb_recvData_t recvData, cb_fileFinalize_t fileFinalize)
{
    *tar                 = (tarStrEx_t *)static_seTar;
    (*tar)->fileInit     = fileInit;
    (*tar)->dirCreate    = dirCreate;
    (*tar)->recvData     = recvData;
    (*tar)->fileFinalize = fileFinalize;
    (*tar)->cbParam      = cbParam;

    (*tar)->status              = tar_header;
    (*tar)->remaining_filedata  = 0;
    (*tar)->remaining_buffBytes = TAR_BLOCK_SIZE;
    (*tar)->buffIdx             = 0;
    return TARSTEX_ESUCCESS;
}

int tarStrEx_finalize(tarStrEx_t *tar)
{
    if (tar_fileData == tar->status)
    {
        /* only call finalization callback if I am sure that fileInit has been
         * called. This is why I chack the state tar_fileData */
        int res;
        res = tar->fileFinalize(tar->cbParam);
        if (0 != res)
        {
            return TARSTEX_EFAILURE;
        }
    }
    return TARSTEX_ESUCCESS;
}

/*
 * a simplified diagram of teh state machine is depicted:
 *
 *             block filled
 *   ┌─────────────────────────────────────────────┐
 *   ∨                                             │
 * ┌─────────┐  file   ┌──────┐  file complete   ┌─────┐
 * │ header  │ ──────> │ data │ ───────────────> │ pad │
 * └─────────┘         └──────┘                  └─────┘
 *   ∧ dir │             ∧  │
 *   └─────┘             └──┘
 *
 * there is an additional 'error' state that is reached from any other state in
 * the presence of unrecoverable errors. From the error state you cannot get out
 */
int tarStrEx_process_data(tarStrEx_t *tar, const uint8_t *data, uint16_t dataSz)
{
    uint16_t chunkSz;
    uint16_t dataIdx = 0;
    int      res;
    while (dataSz > 0) /* all byte ub data has to be processed */
    {
        if (tar_error == tar->status)
        {
            /* do nothing */
            return TARSTEX_EFAILURE;
        }
        /* I write into the block buffer as many bytes as possible */
        chunkSz = min(dataSz, tar->remaining_buffBytes);
        memcpy(&tar->blockBuff[tar->buffIdx], &data[dataIdx], chunkSz);
        /* update indexes, ect. */
        tar->buffIdx += chunkSz;
        tar->remaining_buffBytes -= chunkSz;
        dataSz -= chunkSz;
        dataIdx += chunkSz;
        switch (tar->status)
        {
        case tar_header:
            if (0 == tar->remaining_buffBytes) /* header fully received */
            {
                /* convert the header */
                res = raw_to_header(&tar->hdr, (const tar_header_t *)tar->blockBuff);
                if (TARSTEX_ENULLRECORD == res)
                {
                    /* At the end of the tar archive there are two 512-byte blocks filled with binary zeros as an
                     * end-of-file marker.
                     * We simply ignore those zeros-populated header and go ahead
                     */
                    /* update indexes, etc. No need to change status */
                    tar->remaining_buffBytes = TAR_BLOCK_SIZE;
                    tar->buffIdx             = 0;
                    break;
                }
                else if (TARSTEX_ESUCCESS != res)
                {
                    tar->status = tar_error;
                    return res;
                }
                switch (tar->hdr.type)
                {
                case TAR_TYPE_REG:                                    /* regular file */
                    res = tar->fileInit(tar->cbParam, tar->hdr.name); /* call the callback */
                    if (0 != res)
                    {
                        tar->status = tar_error;
                        return TARSTEX_EFAILURE;
                    }
                    tar->status = tar_fileData; /* status change */
                    /* update indexes, etc. */
                    tar->remaining_filedata  = tar->hdr.size;
                    tar->remaining_buffBytes = TAR_BLOCK_SIZE;
                    tar->buffIdx             = 0;
                    break;
                case TAR_TYPE_DIR:                                     /* directory */
                    res = tar->dirCreate(tar->cbParam, tar->hdr.name); /* call the callback */
                    if (0 != res)
                    {
                        tar->status = tar_error;
                        return TARSTEX_EFAILURE;
                    }
                    /* update indexes, etc. No need to change status */
                    tar->remaining_buffBytes = TAR_BLOCK_SIZE;
                    tar->buffIdx             = 0;
                    break;
                default: /* unsupported type */
                    tar->status = tar_error;
                    return TARSTEX_EFAILURE;
                    break;
                }
            }
            break;
        case tar_fileData:
        {
            uint16_t fileChunk;
            fileChunk =
                min(tar->remaining_filedata, chunkSz); /* I consider as many bytes as I can to complete the file */
            tar->remaining_filedata -= fileChunk;
            if (0 == tar->remaining_filedata)
            {
                res = tar->recvData(tar->cbParam, tar->blockBuff,
                                    tar->buffIdx - (chunkSz - fileChunk)); /* call the callback */
                tar->fileFinalize(tar->cbParam);                           /* call the finalize callback because
                                                                              the file is complete */
                if (0 != res)
                {
                    tar->status = tar_error;
                    return TARSTEX_EFAILURE;
                }
                if (0 == tar->remaining_buffBytes) /* also block buffer is comlete */
                {
                    /* update indexes, etc */
                    tar->remaining_buffBytes = TAR_BLOCK_SIZE;
                    tar->buffIdx             = 0;
                    tar->status              = tar_header; /* no need to walk through tar_filePad state */
                }
                else
                {
                    tar->status = tar_filePad;
                }
            }
            else if (0 == tar->remaining_buffBytes) /* completed the block buffer
                                                       (but not the file) */
            {
                res = tar->recvData(tar->cbParam, tar->blockBuff, TAR_BLOCK_SIZE); /* call the callback */
                if (0 != res)
                {
                    tar->status = tar_error;
                    return TARSTEX_EFAILURE;
                }
                /* update indexes, etc */
                tar->remaining_buffBytes = TAR_BLOCK_SIZE;
                tar->buffIdx             = 0;
            }
        }
        break;
        case tar_filePad:
            /* I only need to collect bytes to complete a block */
            if (0 == tar->remaining_buffBytes)
            {
                /* update indexes, etc */
                tar->remaining_buffBytes = TAR_BLOCK_SIZE;
                tar->buffIdx             = 0;
                tar->status              = tar_header;
            }
            break;
        case tar_error:
            /* do nothing */
            /* This case is handled at the beginning of while loop. I was forced to
             * add it also here to avoid compiler warning */
            break;
        }
    }
    return TARSTEX_ESUCCESS;
}
