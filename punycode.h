#ifndef PUNYCODE_H_
#define PUNYCODE_H_

#include <stdint.h>
#include <stddef.h>

/**
 * Convert Unicode to Punycode. Returns the number of Unicode characters that were converted.
 */
size_t punycode_encode(const uint32_t *src, size_t srclen, char *dst, size_t *dstlen);

/**
 * Convert Punycode to Unicode. Returns the number of bytes that were converted.
 */
size_t punycode_decode(const char *src, size_t srclen, uint32_t *dst, size_t *dstlen);

#endif /* punycode.h */
