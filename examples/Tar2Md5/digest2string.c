/*
 * digest2string.c
 *
 *  Created on: Apr 8, 2019
 *      Author: max
 */

#include "digest2string.h"
#include <stdio.h>

char *digest2string(const uint8_t *hash, int32_t hashDim, char *hashString)
{
    char *p = hashString;
    for (int i = 0; i < hashDim; i++)
    {
        p += sprintf(p, "%02hhx", hash[i] & 0xff);
    }
    return hashString;
}
