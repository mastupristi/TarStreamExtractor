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

#ifndef EXAMPLES_TAR2MD5_DIGEST2STRING_H
#define EXAMPLES_TAR2MD5_DIGEST2STRING_H

#include <stdint.h>

/*
 * serialize in a printable human-readable string the hash given as input
 * parameter
 *
 * hash: array of bytes that is the hash (in big endian, hash[0] is MSB and
 * hash[hashDim-1] is LSB) hashDim: dimension of hash in bytes, eg. 16 bytes for
 * md5, 20 for sha1 and 32 for sha256 hashString: the output string. The caller
 * must provide enough memory space to store the string eg. hashDim*2 + 1
 * return: hashString itself (useful for some purposes)
 */
char *digest2string(const uint8_t *hash, int32_t hashDim, char *hashString);

#endif /* EXAMPLES_TAR2MD5_DIGEST2STRING_H */
