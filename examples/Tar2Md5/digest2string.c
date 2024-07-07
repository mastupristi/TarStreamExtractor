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
