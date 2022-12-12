#include "base64.h"
#include <mbedtls/base64.h>

/*
 * Encode a buffer into base64 format
 */
int base64_encode(unsigned char *dst, size_t *dlen, const unsigned char *src, size_t slen)
{
	return mbedtls_base64_encode(dst, *dlen, dlen, src, slen);
}

/*
 * Decode a base64-formatted buffer
 */
int base64_decode(unsigned char *dst, size_t *dlen, const unsigned char *src, size_t slen)
{
	return mbedtls_base64_decode(dst, *dlen, dlen, src, slen);
}
