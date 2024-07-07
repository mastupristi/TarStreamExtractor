/*
 * digest2string.h
 *
 *  Created on: Apr 8, 2019
 *      Author: max
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
