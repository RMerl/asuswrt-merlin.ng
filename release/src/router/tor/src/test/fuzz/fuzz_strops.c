/* Copyright (c) 2018-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file fuzz_strops.c
 * \brief Fuzzers for various string encoding/decoding operations
 **/

#include "orconfig.h"

#include "lib/cc/torint.h"
#include "lib/ctime/di_ops.h"
#include "lib/encoding/binascii.h"
#include "lib/encoding/cstring.h"
#include "lib/encoding/kvline.h"
#include "lib/encoding/confline.h"
#include "lib/malloc/malloc.h"
#include "lib/log/escape.h"
#include "lib/log/util_bug.h"
#include "lib/intmath/muldiv.h"

#include "test/fuzz/fuzzing.h"

#include <stdio.h>
#include <string.h>

int
fuzz_init(void)
{
  return 0;
}

int
fuzz_cleanup(void)
{
  return 0;
}

typedef struct chunk_t {
  uint8_t *buf;
  size_t len;
} chunk_t;

#define chunk_free(ch)                          \
  FREE_AND_NULL(chunk_t, chunk_free_, (ch))

static chunk_t *
chunk_new(size_t len)
{
  chunk_t *ch = tor_malloc(sizeof(chunk_t));
  ch->buf = tor_malloc(len);
  ch->len = len;
  return ch;
}
static void
chunk_free_(chunk_t *ch)
{
  if (!ch)
    return;
  tor_free(ch->buf);
  tor_free(ch);
}
static bool
chunk_eq(const chunk_t *a, const chunk_t *b)
{
  return a->len == b->len && fast_memeq(a->buf, b->buf, a->len);
}

static chunk_t *
b16_dec(const chunk_t *inp)
{
  chunk_t *ch = chunk_new(CEIL_DIV(inp->len, 2));
  int r = base16_decode((char *)ch->buf, ch->len, (char *)inp->buf, inp->len);
  if (r >= 0) {
    ch->len = r;
  } else {
    chunk_free(ch);
  }
  return ch;
}
static chunk_t *
b16_enc(const chunk_t *inp)
{
  chunk_t *ch = chunk_new(inp->len * 2 + 1);
  base16_encode((char *)ch->buf, ch->len, (char*)inp->buf, inp->len);
  return ch;
}

static chunk_t *
b32_dec(const chunk_t *inp)
{
  chunk_t *ch = chunk_new(inp->len);//XXXX
  int r = base32_decode((char *)ch->buf, ch->len, (char *)inp->buf, inp->len);
  if (r >= 0) {
    ch->len = r;
  } else {
    chunk_free(ch);
  }
  return ch;
}
static chunk_t *
b32_enc(const chunk_t *inp)
{
  chunk_t *ch = chunk_new(base32_encoded_size(inp->len));
  base32_encode((char *)ch->buf, ch->len, (char*)inp->buf, inp->len);
  ch->len = strlen((char *) ch->buf);
  return ch;
}

static chunk_t *
b64_dec(const chunk_t *inp)
{
  chunk_t *ch = chunk_new(inp->len);//XXXX This could be shorter.
  int r = base64_decode((char *)ch->buf, ch->len, (char *)inp->buf, inp->len);
  if (r >= 0) {
    ch->len = r;
  } else {
    chunk_free(ch);
  }
  return ch;
}
static chunk_t *
b64_enc(const chunk_t *inp)
{
  chunk_t *ch = chunk_new(BASE64_BUFSIZE(inp->len));
  base64_encode((char *)ch->buf, ch->len, (char *)inp->buf, inp->len, 0);
  ch->len = strlen((char *) ch->buf);
  return ch;
}

static chunk_t *
c_dec(const chunk_t *inp)
{
  char *s = tor_memdup_nulterm(inp->buf, inp->len);
  chunk_t *ch = tor_malloc(sizeof(chunk_t));
  char *r = NULL;
  (void) unescape_string(s, &r, &ch->len);
  tor_free(s);
  ch->buf = (uint8_t*) r;
  if (!ch->buf) {
    tor_free(ch);
  }
  return ch;
}
static chunk_t *
c_enc(const chunk_t *inp)
{
  char *s = tor_memdup_nulterm(inp->buf, inp->len);
  chunk_t *ch = tor_malloc(sizeof(chunk_t));
  ch->buf = (uint8_t*)esc_for_log(s);
  tor_free(s);
  ch->len = strlen((char*)ch->buf);
  return ch;
}

static int kv_flags = 0;
static config_line_t *
kv_dec(const chunk_t *inp)
{
  char *s = tor_memdup_nulterm(inp->buf, inp->len);
  config_line_t *res = kvline_parse(s, kv_flags);
  tor_free(s);
  return res;
}
static chunk_t *
kv_enc(const config_line_t *inp)
{
  char *s = kvline_encode(inp, kv_flags);
  if (!s)
    return NULL;
  chunk_t *res = tor_malloc(sizeof(chunk_t));
  res->buf = (uint8_t*)s;
  res->len = strlen(s);
  return res;
}

/* Given an encoder function, a decoder function, and a function to free
 * the decoded object, check whether any string that successfully decoded
 * will then survive an encode-decode-encode round-trip unchanged.
 */
#define ENCODE_ROUNDTRIP(E,D,FREE)              \
  STMT_BEGIN {                                  \
    bool err = false;                           \
    a = D(&inp);                                \
    if (!a)                                     \
      return 0;                                 \
    b = E(a);                                   \
    tor_assert(b);                              \
    c = D(b);                                   \
    tor_assert(c);                              \
    d = E(c);                                   \
    tor_assert(d);                              \
    if (!chunk_eq(b,d)) {                       \
      printf("Unequal chunks: %s\n",            \
             hex_str((char*)b->buf, b->len));   \
      printf("             vs %s\n",            \
             hex_str((char*)d->buf, d->len));   \
      err = true;                               \
    }                                           \
    FREE(a);                                    \
    chunk_free(b);                              \
    FREE(c);                                    \
    chunk_free(d);                              \
    tor_assert(!err);                           \
  } STMT_END

int
fuzz_main(const uint8_t *stdin_buf, size_t data_size)
{
  if (!data_size)
    return 0;

  chunk_t inp = { (uint8_t*)stdin_buf, data_size };
  chunk_t *b=NULL,*d=NULL;
  void *a=NULL,*c=NULL;

  switch (stdin_buf[0]) {
    case 0:
      ENCODE_ROUNDTRIP(b16_enc, b16_dec, chunk_free_);
      break;
    case 1:
      ENCODE_ROUNDTRIP(b32_enc, b32_dec, chunk_free_);
      break;
    case 2:
      ENCODE_ROUNDTRIP(b64_enc, b64_dec, chunk_free_);
      break;
    case 3:
      ENCODE_ROUNDTRIP(c_enc, c_dec, chunk_free_);
      break;
    case 5:
      kv_flags = KV_QUOTED|KV_OMIT_KEYS;
      ENCODE_ROUNDTRIP(kv_enc, kv_dec, config_free_lines_);
      break;
    case 6:
      kv_flags = 0;
      ENCODE_ROUNDTRIP(kv_enc, kv_dec, config_free_lines_);
      break;
    case 7:
      kv_flags = KV_OMIT_VALS;
      ENCODE_ROUNDTRIP(kv_enc, kv_dec, config_free_lines_);
      break;
    case 8:
      kv_flags = KV_QUOTED;
      ENCODE_ROUNDTRIP(kv_enc, kv_dec, config_free_lines_);
      break;
    case 9:
      kv_flags = KV_QUOTED|KV_OMIT_VALS;
      ENCODE_ROUNDTRIP(kv_enc, kv_dec, config_free_lines_);
      break;
    }

  return 0;
}
