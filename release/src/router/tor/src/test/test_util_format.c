/* Copyright (c) 2010-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#include "orconfig.h"
#include "core/or/or.h"

#include "test/test.h"

#include "lib/crypt_ops/crypto_rand.h"
#include "lib/encoding/binascii.h"

static void
test_util_format_unaligned_accessors(void *ignored)
{
  (void)ignored;
  char buf[9] = "onionsoup"; // 6f6e696f6e736f7570

  tt_u64_op(get_uint64(buf+1), OP_EQ,
      tor_htonll(UINT64_C(0x6e696f6e736f7570)));
  tt_uint_op(get_uint32(buf+1), OP_EQ, htonl(0x6e696f6e));
  tt_uint_op(get_uint16(buf+1), OP_EQ, htons(0x6e69));
  tt_uint_op(get_uint8(buf+1), OP_EQ, 0x6e);

  set_uint8(buf+7, 0x61);
  tt_mem_op(buf, OP_EQ, "onionsoap", 9);

  set_uint16(buf+6, htons(0x746f));
  tt_mem_op(buf, OP_EQ, "onionstop", 9);

  set_uint32(buf+1, htonl(0x78696465));
  tt_mem_op(buf, OP_EQ, "oxidestop", 9);

  set_uint64(buf+1, tor_htonll(UINT64_C(0x6266757363617465)));
  tt_mem_op(buf, OP_EQ, "obfuscate", 9);
 done:
  ;
}

static void
test_util_format_base64_encode(void *ignored)
{
  (void)ignored;
  int res;
  int i;
  char *src;
  char *dst;

  src = tor_malloc_zero(256);
  dst = tor_malloc_zero(1000);

  for (i=0;i<256;i++) {
    src[i] = (char)i;
  }

  res = base64_encode(NULL, 1, src, 1, 0);
  tt_int_op(res, OP_EQ, -1);

  res = base64_encode(dst, 1, NULL, 1, 0);
  tt_int_op(res, OP_EQ, -1);

  res = base64_encode(dst, 1, src, 10, 0);
  tt_int_op(res, OP_EQ, -1);

  res = base64_encode(dst, SSIZE_MAX-1, src, 1, 0);
  tt_int_op(res, OP_EQ, -1);

  res = base64_encode(dst, SSIZE_MAX-1, src, 10, 0);
  tt_int_op(res, OP_EQ, -1);

  res = base64_encode(dst, 1000, src, 256, 0);
  tt_int_op(res, OP_EQ, 344);
  tt_str_op(dst, OP_EQ, "AAECAwQFBgcICQoLDA0ODxAREhMUFRYXGBkaGxwdHh"
            "8gISIjJCUmJygpKissLS4vMDEyMzQ1Njc4OTo7PD0+P0BBQkNERUZH"
            "SElKS0xNTk9QUVJTVFVWV1hZWltcXV5fYGFiY2RlZmdoaWprbG1ub3"
            "BxcnN0dXZ3eHl6e3x9fn+AgYKDhIWGh4iJiouMjY6PkJGSk5SVlpeY"
            "mZqbnJ2en6ChoqOkpaanqKmqq6ytrq+wsbKztLW2t7i5uru8vb6/wM"
            "HCw8TFxsfIycrLzM3Oz9DR0tPU1dbX2Nna29zd3t/g4eLj5OXm5+jp"
            "6uvs7e7v8PHy8/T19vf4+fr7/P3+/w==");

  res = base64_encode(dst, 1000, src, 256, BASE64_ENCODE_MULTILINE);
  tt_int_op(res, OP_EQ, 350);
  tt_str_op(dst, OP_EQ,
          "AAECAwQFBgcICQoLDA0ODxAREhMUFRYXGBkaGxwdHh8gISIjJCUmJygpKissLS4v\n"
          "MDEyMzQ1Njc4OTo7PD0+P0BBQkNERUZHSElKS0xNTk9QUVJTVFVWV1hZWltcXV5f\n"
          "YGFiY2RlZmdoaWprbG1ub3BxcnN0dXZ3eHl6e3x9fn+AgYKDhIWGh4iJiouMjY6P\n"
          "kJGSk5SVlpeYmZqbnJ2en6ChoqOkpaanqKmqq6ytrq+wsbKztLW2t7i5uru8vb6/\n"
          "wMHCw8TFxsfIycrLzM3Oz9DR0tPU1dbX2Nna29zd3t/g4eLj5OXm5+jp6uvs7e7v\n"
          "8PHy8/T19vf4+fr7/P3+/w==\n");

  res = base64_encode(dst, 1000, src+1, 255, BASE64_ENCODE_MULTILINE);
  tt_int_op(res, OP_EQ, 346);

  for (i = 0;i<50;i++) {
    src[i] = 0;
  }
  src[50] = (char)255;
  src[51] = (char)255;
  src[52] = (char)255;
  src[53] = (char)255;

  res = base64_encode(dst, 1000, src, 54, BASE64_ENCODE_MULTILINE);
  tt_int_op(res, OP_EQ, 74);

  res = base64_encode(dst, 1000, src+1, 53, BASE64_ENCODE_MULTILINE);
  tt_int_op(res, OP_EQ, 74);

  res = base64_encode(dst, 1000, src+2, 52, BASE64_ENCODE_MULTILINE);
  tt_int_op(res, OP_EQ, 74);

  res = base64_encode(dst, 1000, src+3, 51, BASE64_ENCODE_MULTILINE);
  tt_int_op(res, OP_EQ, 70);

  res = base64_encode(dst, 1000, src+4, 50, BASE64_ENCODE_MULTILINE);
  tt_int_op(res, OP_EQ, 70);

  res = base64_encode(dst, 1000, src+5, 49, BASE64_ENCODE_MULTILINE);
  tt_int_op(res, OP_EQ, 70);

  res = base64_encode(dst, 1000, src+6, 48, BASE64_ENCODE_MULTILINE);
  tt_int_op(res, OP_EQ, 65);

  res = base64_encode(dst, 1000, src+7, 47, BASE64_ENCODE_MULTILINE);
  tt_int_op(res, OP_EQ, 65);

  res = base64_encode(dst, 1000, src+8, 46, BASE64_ENCODE_MULTILINE);
  tt_int_op(res, OP_EQ, 65);

 done:
  tor_free(src);
  tor_free(dst);
}

static void
test_util_format_base64_decode_oddsize(void *ignored)
{
  (void)ignored;
  int res;
  int i;
  char *src;
  char *dst, real_dst[7];
  char expected[] = {0x65, 0x78, 0x61, 0x6D, 0x70, 0x6C, 0x65};
  char real_src[] = "ZXhhbXBsZQ";
  char expected40[] = "testing40characteroddsizebase64encoding!";
  char src40[] = "dGVzdGluZzQwY2hhcmFjdGVyb2Rkc2l6ZWJhc2U2NGVuY29kaW5nIQ";
  char pad40[] = "dGVzdGluZzQwY2hhcmFjdGVyb2Rkc2l6ZWJhc2U2NGVuY29kaW5nIQ==";

  src = tor_malloc_zero(256);
  dst = tor_malloc_zero(1000);

  for (i=0;i<256;i++) {
    src[i] = (char)i;
  }

  res = base64_decode(dst, 1, src, 5);
  tt_int_op(res, OP_EQ, -1);

  const char *s = "SGVsbG8gd29ybGQ";
  res = base64_decode(dst, 1000, s, strlen(s));
  tt_int_op(res, OP_EQ, 11);
  tt_mem_op(dst, OP_EQ, "Hello world", 11);

  s = "T3BhIG11bmRv";
  res = base64_decode(dst, 9, s, strlen(s));
  tt_int_op(res, OP_EQ, 9);
  tt_mem_op(dst, OP_EQ, "Opa mundo", 9);

  res = base64_decode(real_dst, sizeof(real_dst), real_src, 10);
  tt_int_op(res, OP_EQ, 7);
  tt_mem_op(real_dst, OP_EQ, expected, 7);

  res = base64_decode(dst, 40, src40, strlen(src40));
  tt_int_op(res, OP_EQ, 40);
  tt_mem_op(dst, OP_EQ, expected40, 40);

  res = base64_decode(dst, 40, pad40, strlen(pad40));
  tt_int_op(res, OP_EQ, 40);
  tt_mem_op(dst, OP_EQ, expected40, 40);

 done:
  tor_free(src);
  tor_free(dst);
}

static void
test_util_format_base64_decode(void *ignored)
{
  (void)ignored;
  int res;
  int i;
  char *src;
  char *dst, *real_dst;
  uint8_t expected[] = {0x65, 0x78, 0x61, 0x6D, 0x70, 0x6C, 0x65};
  char real_src[] = "ZXhhbXBsZQ==";

  src = tor_malloc_zero(256);
  dst = tor_malloc_zero(1000);
  real_dst = tor_malloc_zero(10);

  for (i=0;i<256;i++) {
    src[i] = (char)i;
  }

  res = base64_decode(dst, 1, src, 100);
  tt_int_op(res, OP_EQ, -1);

  res = base64_decode(dst, 1, real_src, 10);
  tt_int_op(res, OP_EQ, -1);

  const char *s = "T3BhIG11bmRv";
  res = base64_decode(dst, 9, s, strlen(s));
  tt_int_op(res, OP_EQ, 9);
  tt_mem_op(dst, OP_EQ, "Opa mundo", 9);

  memset(dst, 0, 1000);
  res = base64_decode(dst, 100, s, strlen(s));
  tt_int_op(res, OP_EQ, 9);
  tt_mem_op(dst, OP_EQ, "Opa mundo", 9);

  s = "SGVsbG8gd29ybGQ=";
  res = base64_decode(dst, 100, s, strlen(s));
  tt_int_op(res, OP_EQ, 11);
  tt_mem_op(dst, OP_EQ, "Hello world", 11);

  res = base64_decode(real_dst, 10, real_src, 10);
  tt_int_op(res, OP_EQ, 7);
  tt_mem_op(real_dst, OP_EQ, expected, 7);

 done:
  tor_free(src);
  tor_free(dst);
  tor_free(real_dst);
}

static void
test_util_format_base16_decode(void *ignored)
{
  (void)ignored;
  int res;
  int i;
  char *src;
  char *dst, *real_dst;
  char expected[] = {0x65, 0x78, 0x61, 0x6D, 0x70, 0x6C, 0x65};
  char real_src[] = "6578616D706C65";

  src = tor_malloc_zero(256);
  dst = tor_malloc_zero(1000);
  real_dst = tor_malloc_zero(10);

  for (i=0;i<256;i++) {
    src[i] = (char)i;
  }

  res = base16_decode(dst, 3, src, 3);
  tt_int_op(res, OP_EQ, -1);

  res = base16_decode(dst, 1, src, 10);
  tt_int_op(res, OP_EQ, -1);

  res = base16_decode(dst, ((size_t)INT_MAX)+1, src, 10);
  tt_int_op(res, OP_EQ, -1);

  res = base16_decode(dst, 1000, "", 0);
  tt_int_op(res, OP_EQ, 0);

  res = base16_decode(dst, 1000, "aabc", 4);
  tt_int_op(res, OP_EQ, 2);
  tt_mem_op(dst, OP_EQ, "\xaa\xbc", 2);

  res = base16_decode(dst, 1000, "aabcd", 6);
  tt_int_op(res, OP_EQ, -1);

  res = base16_decode(dst, 1000, "axxx", 4);
  tt_int_op(res, OP_EQ, -1);

  res = base16_decode(real_dst, 10, real_src, 14);
  tt_int_op(res, OP_EQ, 7);
  tt_mem_op(real_dst, OP_EQ, expected, 7);

 done:
  tor_free(src);
  tor_free(dst);
  tor_free(real_dst);
}

static void
test_util_format_base32_encode(void *arg)
{
  (void) arg;
  size_t real_dstlen = 32;
  char *dst = tor_malloc_zero(real_dstlen);

  /* Basic use case that doesn't require a source length correction. */
  {
    /* Length of 10 bytes. */
    const char *src = "blahbleh12";
    size_t srclen = strlen(src);
    /* Expected result encoded base32. This was created using python as
     * such (and same goes for all test case.):
     *
     *  b = bytes("blahbleh12", 'utf-8')
     *  base64.b32encode(b)
     *  (result in lower case)
     */
    const char *expected = "mjwgc2dcnrswqmjs";

    base32_encode(dst, base32_encoded_size(srclen), src, srclen);
    tt_mem_op(expected, OP_EQ, dst, strlen(expected));
    /* Encode but to a larger size destination. */
    memset(dst, 0, real_dstlen);
    base32_encode(dst, real_dstlen, src, srclen);
    tt_mem_op(expected, OP_EQ, dst, strlen(expected));
  }

  /* Non multiple of 5 for the source buffer length. */
  {
    /* Length of 8 bytes. */
    const char *expected = "mjwgc2dcnrswq";
    const char *src = "blahbleh";
    size_t srclen = strlen(src);

    memset(dst, 0, real_dstlen);
    base32_encode(dst, base32_encoded_size(srclen), src, srclen);
    tt_mem_op(expected, OP_EQ, dst, strlen(expected));
  }

 done:
  tor_free(dst);
}

static void
test_util_format_base32_decode(void *arg)
{
  (void) arg;
  int ret;
  size_t real_dstlen = 32;
  char *dst = tor_malloc_zero(real_dstlen);

  /* Basic use case. */
  {
    /* Length of 10 bytes. */
    const char *expected = "blahbleh12";
    /* Expected result encoded base32. */
    const char *src = "mjwgc2dcnrswqmjs";

    ret = base32_decode(dst, strlen(expected), src, strlen(src));
    tt_int_op(ret, OP_EQ, 10);
    tt_str_op(expected, OP_EQ, dst);
  }

  /* Non multiple of 5 for the source buffer length. */
  {
    /* Length of 8 bytes. */
    const char *expected = "blahbleh";
    const char *src = "mjwgc2dcnrswq";

    ret = base32_decode(dst, strlen(expected), src, strlen(src));
    tt_int_op(ret, OP_EQ, 8);
    tt_mem_op(expected, OP_EQ, dst, strlen(expected));
  }

  /* Invalid values. */
  {
    /* Invalid character '#'. */
    ret = base32_decode(dst, real_dstlen, "#abcde", 6);
    tt_int_op(ret, OP_EQ, -1);
    /* Make sure the destination buffer has been zeroed even on error. */
    tt_int_op(fast_mem_is_zero(dst, real_dstlen), OP_EQ, 1);
  }

 done:
  tor_free(dst);
}

static void
test_util_format_encoded_size(void *arg)
{
  (void)arg;
  uint8_t inbuf[256];
  char outbuf[1024];
  unsigned i;

  crypto_rand((char *)inbuf, sizeof(inbuf));
  for (i = 0; i <= sizeof(inbuf); ++i) {
    /* XXXX (Once the return values are consistent, check them too.) */

    base32_encode(outbuf, sizeof(outbuf), (char *)inbuf, i);
    /* The "+ 1" below is an API inconsistency. */
    tt_int_op(strlen(outbuf) + 1, OP_EQ, base32_encoded_size(i));

    base64_encode(outbuf, sizeof(outbuf), (char *)inbuf, i, 0);
    tt_int_op(strlen(outbuf), OP_EQ, base64_encode_size(i, 0));
    tt_int_op(i, OP_LE, base64_decode_maxsize(strlen(outbuf)));

    base64_encode(outbuf, sizeof(outbuf), (char *)inbuf, i,
                  BASE64_ENCODE_MULTILINE);
    tt_int_op(strlen(outbuf), OP_EQ,
              base64_encode_size(i, BASE64_ENCODE_MULTILINE));
    tt_int_op(i, OP_LE, base64_decode_maxsize(strlen(outbuf)));
  }

 done:
  ;
}

struct testcase_t util_format_tests[] = {
  { "unaligned_accessors", test_util_format_unaligned_accessors, 0,
    NULL, NULL },
  { "base64_encode", test_util_format_base64_encode, 0, NULL, NULL },
  { "base64_decode_oddsize", test_util_format_base64_decode_oddsize, 0,
    NULL, NULL },
  { "base64_decode", test_util_format_base64_decode, 0, NULL, NULL },
  { "base16_decode", test_util_format_base16_decode, 0, NULL, NULL },
  { "base32_encode", test_util_format_base32_encode, 0,
    NULL, NULL },
  { "base32_decode", test_util_format_base32_decode, 0,
    NULL, NULL },
  { "encoded_size", test_util_format_encoded_size, 0, NULL, NULL },
  END_OF_TESTCASES
};
