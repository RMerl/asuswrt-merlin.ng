#ifndef _FR_SHA1_H
#define _FR_SHA1_H

#ifdef WITH_OPENSSL_SHA1
#include <openssl/sha.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef WITH_OPENSSL_SHA1
typedef struct {
    uint32_t state[5];
    uint32_t count[2];
    uint8_t buffer[64];
} fr_SHA1_CTX;

void fr_SHA1Transform(uint32_t state[5], uint8_t const buffer[64]);
void fr_SHA1Init(fr_SHA1_CTX *context);
void fr_SHA1Update(fr_SHA1_CTX *context, uint8_t const *data, size_t len);
void fr_SHA1Final(uint8_t digest[20], fr_SHA1_CTX *context);

/*
 * this version implements a raw SHA1 transform, no length is appended,
 * nor any 128s out to the block size.
 *
 *	Hmm... this function doesn't appear to be used anywhere.
 */
void fr_SHA1FinalNoLen(uint8_t digest[20], fr_SHA1_CTX* context);

#else  /* WITH_OPENSSL_SHA1 */
USES_APPLE_DEPRECATED_API
#define fr_SHA1_CTX	SHA_CTX
#define fr_SHA1Init	SHA1_Init
#define fr_SHA1Update	SHA1_Update
#define fr_SHA1Final	SHA1_Final
#define fr_SHA1Transform SHA1_Transform
#endif

/*
 * FIPS 186-2 PRF based upon SHA1.
 *
 *	Hmm... this is only used in src/modules/rlm_eap/libeap/
 *	why is the prototype here?
 */
extern void fips186_2prf(uint8_t mk[20], uint8_t finalkey[160]);

#ifdef __cplusplus
}
#endif

#endif /* _FR_SHA1_H */
