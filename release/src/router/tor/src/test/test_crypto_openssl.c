/* Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#include "orconfig.h"

#define CRYPTO_RAND_PRIVATE

#include "lib/crypt_ops/compat_openssl.h"
#include "lib/crypt_ops/crypto_rand.h"
#include "lib/encoding/binascii.h"
#include "lib/malloc/malloc.h"
#include "test/test.h"

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <string.h>

/* Test for rectifying openssl RAND engine. */
static void
test_crypto_rng_engine(void *arg)
{
  (void)arg;
  RAND_METHOD dummy_method;
  memset(&dummy_method, 0, sizeof(dummy_method));

  /* We should be a no-op if we're already on RAND_OpenSSL */
  tt_int_op(0, OP_EQ, crypto_force_rand_ssleay());
  tt_assert(RAND_get_rand_method() == RAND_OpenSSL());

  /* We should correct the method if it's a dummy. */
  RAND_set_rand_method(&dummy_method);
#ifdef LIBRESSL_VERSION_NUMBER
  /* On libressl, you can't override the RNG. */
  tt_assert(RAND_get_rand_method() == RAND_OpenSSL());
  tt_int_op(0, OP_EQ, crypto_force_rand_ssleay());
#else
  tt_assert(RAND_get_rand_method() == &dummy_method);
  tt_int_op(1, OP_EQ, crypto_force_rand_ssleay());
#endif /* defined(LIBRESSL_VERSION_NUMBER) */
  tt_assert(RAND_get_rand_method() == RAND_OpenSSL());

  /* Make sure we aren't calling dummy_method */
  crypto_rand((void *) &dummy_method, sizeof(dummy_method));
  crypto_rand((void *) &dummy_method, sizeof(dummy_method));

 done:
  ;
}

#ifndef OPENSSL_1_1_API
#define EVP_ENCODE_CTX_new() tor_malloc_zero(sizeof(EVP_ENCODE_CTX))
#define EVP_ENCODE_CTX_free(ctx) tor_free(ctx)
#endif

/** Encode src into dest with OpenSSL's EVP Encode interface, returning the
 * length of the encoded data in bytes.
 */
static int
base64_encode_evp(char *dest, char *src, size_t srclen)
{
  const unsigned char *s = (unsigned char*)src;
  EVP_ENCODE_CTX *ctx = EVP_ENCODE_CTX_new();
  int len, ret;

  EVP_EncodeInit(ctx);
  EVP_EncodeUpdate(ctx, (unsigned char *)dest, &len, s, (int)srclen);
  EVP_EncodeFinal(ctx, (unsigned char *)(dest + len), &ret);
  EVP_ENCODE_CTX_free(ctx);
  return ret+ len;
}

static void
test_crypto_base64_encode_matches(void *arg)
{
  (void)arg;
  int i, j;
  char data1[1024];
  char data2[1024];
  char data3[1024];

  for (i = 0; i < 256; i++) {
    /* Test the multiline format Base64 encoder with 0 .. 256 bytes of
     * output against OpenSSL.
     */
    const size_t enclen = base64_encode_size(i, BASE64_ENCODE_MULTILINE);
    data1[i] = i;
    j = base64_encode(data2, 1024, data1, i, BASE64_ENCODE_MULTILINE);
    tt_int_op(j, OP_EQ, enclen);
    j = base64_encode_evp(data3, data1, i);
    tt_int_op(j, OP_EQ, enclen);
    tt_mem_op(data2, OP_EQ, data3, enclen);
    tt_int_op(j, OP_EQ, strlen(data2));
  }

 done:
  ;
}

struct testcase_t crypto_openssl_tests[] = {
  { "rng_engine", test_crypto_rng_engine, TT_FORK, NULL, NULL },
  { "base64_encode_match", test_crypto_base64_encode_matches,
    TT_FORK, NULL, NULL },
  END_OF_TESTCASES
};
