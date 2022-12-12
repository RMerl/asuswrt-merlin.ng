#include "md5.h"
#include "sha1.h"
#include <mbedtls/md5.h>
#include <mbedtls/sha1.h>

/* Calculate the MD5 hash checksum of the given input */
void md5(const unsigned char *input, size_t ilen, unsigned char output[16]) { mbedtls_md5(input, ilen, output); }

/* Calculate the SHA-1 hash checksum of the given input */
void sha1(const unsigned char *input, size_t ilen, unsigned char output[20]) { mbedtls_sha1(input, ilen, output); };
