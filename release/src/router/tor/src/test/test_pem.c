/* Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#include "orconfig.h"

#include "lib/encoding/pem.h"
#include "lib/cc/compat_compiler.h"
#include "lib/malloc/malloc.h"

#include "test/test.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static const char example_pre[] =
    "Lest you get the wrong impression, we wombats "
    "are not in the habit of tunneling madly about, without any supplies "
    "or even a map."; /* -- Ursula Vernon, _Digger_ */
static const char expected[] =
    "-----BEGIN WOMBAT QUOTE-----\n"
    "TGVzdCB5b3UgZ2V0IHRoZSB3cm9uZyBpbXByZXNzaW9uLCB3ZSB3b21iYXRzIGFy\n"
    "ZSBub3QgaW4gdGhlIGhhYml0IG9mIHR1bm5lbGluZyBtYWRseSBhYm91dCwgd2l0\n"
    "aG91dCBhbnkgc3VwcGxpZXMgb3IgZXZlbiBhIG1hcC4=\n"
    "-----END WOMBAT QUOTE-----\n";

static void
test_crypto_pem_encode(void *arg)
{
  (void)arg;

  char buf[4096];

  int n = (int) pem_encoded_size(strlen(example_pre), "WOMBAT QUOTE");

  int n2 = pem_encode(buf, sizeof(buf),
                      (const unsigned char *)example_pre, strlen(example_pre),
                      "WOMBAT QUOTE");
  tt_int_op(strlen(buf)+1, OP_EQ, n);
  tt_int_op(n2, OP_EQ, 0);
  tt_str_op(buf, OP_EQ, expected);

  /* Now make sure it succeeds if the buffer is exactly the length we want. */
  memset(buf, 0, sizeof(buf));
  n2 = pem_encode(buf, n, (const unsigned char *)example_pre,
                      strlen(example_pre), "WOMBAT QUOTE");
  tt_int_op(n2, OP_EQ, 0);
  tt_str_op(buf, OP_EQ, expected);

  /* Make sure it fails if the buffer is too short. */
  memset(buf, 0, sizeof(buf));
  n2 = pem_encode(buf, n - 1, (const unsigned char *)example_pre,
                  strlen(example_pre), "WOMBAT QUOTE");
  tt_int_op(n2, OP_EQ, -1);

 done:
  ;
}

static void
test_crypto_pem_decode(void *arg)
{
  (void)arg;

  unsigned char buf[4096];

  /* Try a straightforward decoding. */
  int n = pem_decode(buf, sizeof(buf),
                     expected, strlen(expected),
                     "WOMBAT QUOTE");
  tt_int_op(n, OP_EQ, strlen(example_pre));
  tt_mem_op(buf, OP_EQ, example_pre, n);

  /* Succeed if the buffer is exactly the right size. */
  memset(buf, 0xff, sizeof(buf));
  n = pem_decode(buf, strlen(example_pre),
                 expected, strlen(expected),
                 "WOMBAT QUOTE");
  tt_int_op(n, OP_EQ, strlen(example_pre));
  tt_mem_op(buf, OP_EQ, example_pre, n);
  tt_int_op(buf[n], OP_EQ, 0xff);

  /* Verify that it fails if the buffer is too small. */
  memset(buf, 0xff, sizeof(buf));
  n = pem_decode(buf, strlen(example_pre) - 1,
                 expected, strlen(expected),
                 "WOMBAT QUOTE");
  tt_int_op(n, OP_EQ, -1);

  /* Verify that it fails with an incorrect tag. */
  memset(buf, 0xff, sizeof(buf));
  n = pem_decode(buf, sizeof(buf),
                 expected, strlen(expected),
                 "QUOKKA VOTE");
  tt_int_op(n, OP_EQ, -1);

  /* Try truncated buffers of different sizes. */
  size_t i;
  for (i = 0; i <= strlen(expected); ++i) {
    char *truncated = tor_memdup(expected, i);
    n = pem_decode(buf, sizeof(buf),
                   truncated, i,
                   "WOMBAT QUOTE");
    tor_free(truncated);
    if (i < strlen(expected) - 1) {
      tt_int_op(n, OP_EQ, -1);
    } else {
      tt_int_op(n, OP_EQ, strlen(example_pre));
    }
  }

 done:
  ;
}

static void
test_crypto_pem_decode_crlf(void *arg)
{
  (void)arg;
  char crlf_version[4096];
  uint8_t buf[4096];

  /* Convert 'expected' to a version with CRLF instead of LF. */
  const char *inp = expected;
  char *outp = crlf_version;
  while (*inp) {
    if (*inp == '\n') {
      *outp++ = '\r';
    }
    *outp++ = *inp++;
  }
  *outp = 0;

  /* Decoding should succeed (or else we have bug 33032 again) */
  int n = pem_decode(buf, sizeof(buf),
                     crlf_version, strlen(crlf_version),
                     "WOMBAT QUOTE");
  tt_int_op(n, OP_EQ, strlen(example_pre));
  tt_mem_op(buf, OP_EQ, example_pre, n);

 done:
  ;
}

struct testcase_t pem_tests[] = {
  { "encode", test_crypto_pem_encode, 0, NULL, NULL },
  { "decode", test_crypto_pem_decode, 0, NULL, NULL },
  { "decode_crlf", test_crypto_pem_decode_crlf, 0, NULL, NULL },
  END_OF_TESTCASES
};
