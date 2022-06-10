/* Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#define BUFFERS_PRIVATE
#define PROTO_HTTP_PRIVATE
#include "core/or/or.h"
#include "lib/buf/buffers.h"
#include "lib/tls/buffers_tls.h"
#include "lib/tls/tortls.h"
#include "lib/compress/compress.h"
#include "lib/crypt_ops/crypto_rand.h"
#include "core/proto/proto_http.h"
#include "core/proto/proto_socks.h"
#include "test/test.h"

/** Run unit tests for buffers.c */
static void
test_buffers_basic(void *arg)
{
  char str[256];
  char str2[256];

  buf_t *buf = NULL, *buf2 = NULL;
  const char *cp;

  int j;
  size_t r;
  (void) arg;

  /****
   * buf_new
   ****/
  if (!(buf = buf_new()))
    TT_DIE(("Assertion failed."));

  //test_eq(buf_capacity(buf), 4096);
  tt_int_op(buf_datalen(buf),OP_EQ, 0);

  /****
   * General pointer frobbing
   */
  for (j=0;j<256;++j) {
    str[j] = (char)j;
  }
  buf_add(buf, str, 256);
  buf_add(buf, str, 256);
  tt_int_op(buf_datalen(buf),OP_EQ, 512);
  buf_get_bytes(buf, str2, 200);
  tt_mem_op(str,OP_EQ, str2, 200);
  tt_int_op(buf_datalen(buf),OP_EQ, 312);
  memset(str2, 0, sizeof(str2));

  buf_get_bytes(buf, str2, 256);
  tt_mem_op(str+200,OP_EQ, str2, 56);
  tt_mem_op(str,OP_EQ, str2+56, 200);
  tt_int_op(buf_datalen(buf),OP_EQ, 56);
  memset(str2, 0, sizeof(str2));
  /* Okay, now we should be 512 bytes into the 4096-byte buffer.  If we add
   * another 3584 bytes, we hit the end. */
  for (j=0;j<15;++j) {
    buf_add(buf, str, 256);
  }
  buf_assert_ok(buf);
  tt_int_op(buf_datalen(buf),OP_EQ, 3896);
  buf_get_bytes(buf, str2, 56);
  tt_int_op(buf_datalen(buf),OP_EQ, 3840);
  tt_mem_op(str+200,OP_EQ, str2, 56);
  for (j=0;j<15;++j) {
    memset(str2, 0, sizeof(str2));
    buf_get_bytes(buf, str2, 256);
    tt_mem_op(str,OP_EQ, str2, 256);
  }
  tt_int_op(buf_datalen(buf),OP_EQ, 0);
  buf_free(buf);
  buf = NULL;

  /* Okay, now make sure growing can work. */
  buf = buf_new_with_capacity(16);
  //test_eq(buf_capacity(buf), 16);
  buf_add(buf, str+1, 255);
  //test_eq(buf_capacity(buf), 256);
  buf_get_bytes(buf, str2, 254);
  tt_mem_op(str+1,OP_EQ, str2, 254);
  //test_eq(buf_capacity(buf), 256);
  buf_assert_ok(buf);
  buf_add(buf, str, 32);
  //test_eq(buf_capacity(buf), 256);
  buf_assert_ok(buf);
  buf_add(buf, str, 256);
  buf_assert_ok(buf);
  //test_eq(buf_capacity(buf), 512);
  tt_int_op(buf_datalen(buf),OP_EQ, 33+256);
  buf_get_bytes(buf, str2, 33);
  tt_int_op(*str2,OP_EQ, str[255]);

  tt_mem_op(str2+1,OP_EQ, str, 32);
  //test_eq(buf_capacity(buf), 512);
  tt_int_op(buf_datalen(buf),OP_EQ, 256);
  buf_get_bytes(buf, str2, 256);
  tt_mem_op(str,OP_EQ, str2, 256);

  /* now try shrinking: case 1. */
  buf_free(buf);
  buf = buf_new_with_capacity(33668);
  for (j=0;j<67;++j) {
    buf_add(buf, str,255);
  }
  //test_eq(buf_capacity(buf), 33668);
  tt_int_op(buf_datalen(buf),OP_EQ, 17085);
  for (j=0; j < 40; ++j) {
    buf_get_bytes(buf, str2, 255);
    tt_mem_op(str2,OP_EQ, str, 255);
  }

  /* now try shrinking: case 2. */
  buf_free(buf);
  buf = buf_new_with_capacity(33668);
  for (j=0;j<67;++j) {
    buf_add(buf, str, 255);
  }
  for (j=0; j < 20; ++j) {
    buf_get_bytes(buf, str2, 255);
    tt_mem_op(str2,OP_EQ, str, 255);
  }
  for (j=0;j<80;++j) {
    buf_add(buf, str, 255);
  }
  //test_eq(buf_capacity(buf),33668);
  for (j=0; j < 120; ++j) {
    buf_get_bytes(buf, str2, 255);
    tt_mem_op(str2,OP_EQ, str, 255);
  }

  /* Move from buf to buf. */
  buf_free(buf);
  buf = buf_new_with_capacity(4096);
  buf2 = buf_new_with_capacity(4096);
  for (j=0;j<100;++j)
    buf_add(buf, str, 255);
  tt_int_op(buf_datalen(buf),OP_EQ, 25500);
  for (j=0;j<100;++j) {
    r = 10;
    buf_move_to_buf(buf2, buf, &r);
    tt_int_op(r,OP_EQ, 0);
  }
  tt_int_op(buf_datalen(buf),OP_EQ, 24500);
  tt_int_op(buf_datalen(buf2),OP_EQ, 1000);
  for (j=0;j<3;++j) {
    buf_get_bytes(buf2, str2, 255);
    tt_mem_op(str2,OP_EQ, str, 255);
  }
  r = 8192; /*big move*/
  buf_move_to_buf(buf2, buf, &r);
  tt_int_op(r,OP_EQ, 0);
  r = 30000; /* incomplete move */
  buf_move_to_buf(buf2, buf, &r);
  tt_int_op(r,OP_EQ, 13692);
  for (j=0;j<97;++j) {
    buf_get_bytes(buf2, str2, 255);
    tt_mem_op(str2,OP_EQ, str, 255);
  }
  buf_free(buf);
  buf_free(buf2);
  buf = buf2 = NULL;

  buf = buf_new_with_capacity(5);
  cp = "Testing. This is a moderately long Testing string.";
  for (j = 0; cp[j]; j++)
    buf_add(buf, cp+j, 1);
  tt_int_op(0,OP_EQ, buf_find_string_offset(buf, "Testing", 7));
  tt_int_op(1,OP_EQ, buf_find_string_offset(buf, "esting", 6));
  tt_int_op(1,OP_EQ, buf_find_string_offset(buf, "est", 3));
  tt_int_op(39,OP_EQ, buf_find_string_offset(buf, "ing str", 7));
  tt_int_op(35,OP_EQ, buf_find_string_offset(buf, "Testing str", 11));
  tt_int_op(32,OP_EQ, buf_find_string_offset(buf, "ng ", 3));
  tt_int_op(43,OP_EQ, buf_find_string_offset(buf, "string.", 7));
  tt_int_op(-1,OP_EQ, buf_find_string_offset(buf, "shrdlu", 6));
  tt_int_op(-1,OP_EQ, buf_find_string_offset(buf, "Testing thing", 13));
  tt_int_op(-1,OP_EQ, buf_find_string_offset(buf, "ngx", 3));
  buf_free(buf);
  buf = NULL;

  /* Try adding a string too long for any freelist. */
  {
    char *mem = tor_malloc_zero(65536);
    buf = buf_new();
    buf_add(buf, mem, 65536);
    tor_free(mem);

    tt_int_op(buf_datalen(buf), OP_EQ, 65536);
    buf_free(buf);
    buf = NULL;
  }

 done:
  if (buf)
    buf_free(buf);
  if (buf2)
    buf_free(buf2);
}

static void
test_buffer_pullup(void *arg)
{
  buf_t *buf;
  char *stuff, *tmp;
  const char *cp;
  size_t sz;
  (void)arg;
  stuff = tor_malloc(16384);
  tmp = tor_malloc(16384);

  buf = buf_new_with_capacity(3000); /* rounds up to next power of 2. */

  tt_assert(buf);
  tt_int_op(buf_get_default_chunk_size(buf), OP_EQ, 4096);

  tt_int_op(buf_get_total_allocation(), OP_EQ, 0);

  /* There are a bunch of cases for pullup.  One is the trivial case. Let's
     mess around with an empty buffer. */
  buf_pullup(buf, 16, &cp, &sz);
  tt_ptr_op(cp, OP_EQ, NULL);
  tt_uint_op(sz, OP_EQ, 0);

  /* Let's make sure nothing got allocated */
  tt_int_op(buf_get_total_allocation(), OP_EQ, 0);

  /* Case 1: everything puts into the first chunk with some moving. */

  /* Let's add some data. */
  crypto_rand(stuff, 16384);
  buf_add(buf, stuff, 3000);
  buf_add(buf, stuff+3000, 3000);
  buf_pullup(buf, 0, &cp, &sz);
  tt_ptr_op(cp, OP_NE, NULL);
  tt_int_op(sz, OP_LE, 4096);

  /* Make room for 3000 bytes in the first chunk, so that the pullup-move code
   * can get tested. */
  tt_int_op(buf_get_bytes(buf, tmp, 3000), OP_EQ, 3000);
  tt_mem_op(tmp,OP_EQ, stuff, 3000);
  buf_pullup(buf, 2048, &cp, &sz);
  buf_assert_ok(buf);
  tt_ptr_op(cp, OP_NE, NULL);
  tt_int_op(sz, OP_GE, 2048);
  tt_mem_op(cp,OP_EQ, stuff+3000, 2048);
  tt_int_op(3000, OP_EQ, buf_datalen(buf));
  tt_int_op(buf_get_bytes(buf, tmp, 3000), OP_EQ, 0);
  tt_mem_op(tmp,OP_EQ, stuff+3000, 2048);

  buf_free(buf);

  /* Now try the large-chunk case. */
  buf = buf_new_with_capacity(3000); /* rounds up to next power of 2. */
  buf_add(buf, stuff, 4000);
  buf_add(buf, stuff+4000, 4000);
  buf_add(buf, stuff+8000, 4000);
  buf_add(buf, stuff+12000, 4000);
  tt_int_op(buf_datalen(buf), OP_EQ, 16000);
  buf_pullup(buf, 0, &cp, &sz);
  tt_ptr_op(cp, OP_NE, NULL);
  tt_int_op(sz, OP_LE, 4096);

  buf_pullup(buf, 12500, &cp, &sz);
  buf_assert_ok(buf);
  tt_ptr_op(cp, OP_NE, NULL);
  tt_int_op(sz, OP_GE, 12500);
  tt_mem_op(cp,OP_EQ, stuff, 12500);
  tt_int_op(buf_datalen(buf), OP_EQ, 16000);

  buf_get_bytes(buf, tmp, 12400);
  tt_mem_op(tmp,OP_EQ, stuff, 12400);
  tt_int_op(buf_datalen(buf), OP_EQ, 3600);
  buf_get_bytes(buf, tmp, 3500);
  tt_mem_op(tmp,OP_EQ, stuff+12400, 3500);
  buf_get_bytes(buf, tmp, 100);
  tt_mem_op(tmp,OP_EQ, stuff+15900, 10);

  buf_free(buf);

  /* Make sure that the pull-up-whole-buffer case works */
  buf = buf_new_with_capacity(3000); /* rounds up to next power of 2. */
  buf_add(buf, stuff, 4000);
  buf_add(buf, stuff+4000, 4000);
  buf_get_bytes(buf, tmp, 100); /* dump 100 bytes from first chunk */
  buf_pullup(buf, 16000, &cp, &sz);
  buf_assert_ok(buf);
  tt_ptr_op(cp, OP_NE, NULL);
  tt_int_op(sz, OP_EQ, 7900);
  tt_mem_op(cp,OP_EQ, stuff+100, 7900);

  buf_free(buf);
  buf = NULL;

  tt_int_op(buf_get_total_allocation(), OP_EQ, 0);
 done:
  buf_free(buf);
  tor_free(stuff);
  tor_free(tmp);
}

static void
test_buffers_move_all(void *arg)
{
  (void)arg;
  buf_t *input = buf_new();
  buf_t *output = buf_new();
  char *s = NULL;

  /* Move from empty buffer to nonempty buffer. (This is a regression test for
   * #40076) */
  buf_add(output, "abc", 3);
  buf_assert_ok(input);
  buf_assert_ok(output);
  buf_move_all(output, input);
  buf_assert_ok(input);
  buf_assert_ok(output);
  tt_int_op(buf_datalen(output), OP_EQ, 3);
  s = buf_extract(output, NULL);
  tt_str_op(s, OP_EQ, "abc");
  buf_free(output);
  buf_free(input);
  tor_free(s);

  /* Move from empty to empty. */
  output = buf_new();
  input = buf_new();
  buf_move_all(output, input);
  buf_assert_ok(input);
  buf_assert_ok(output);
  tt_int_op(buf_datalen(output), OP_EQ, 0);
  buf_free(output);
  buf_free(input);

  /* Move from nonempty to empty. */
  output = buf_new();
  input = buf_new();
  buf_add(input, "longstanding bugs", 17);
  buf_move_all(output, input);
  buf_assert_ok(input);
  buf_assert_ok(output);
  s = buf_extract(output, NULL);
  tt_str_op(s, OP_EQ, "longstanding bugs");
  buf_free(output);
  buf_free(input);
  tor_free(s);

  /* Move from nonempty to nonempty. */
  output = buf_new();
  input = buf_new();
  buf_add(output, "the start of", 12);
  buf_add(input, " a string", 9);
  buf_move_all(output, input);
  buf_assert_ok(input);
  buf_assert_ok(output);
  s = buf_extract(output, NULL);
  tt_str_op(s, OP_EQ, "the start of a string");

 done:
  buf_free(output);
  buf_free(input);
  tor_free(s);
}

static void
test_buffer_copy(void *arg)
{
  buf_t *buf=NULL, *buf2=NULL;
  const char *s;
  size_t len;
  char b[256];
  int i;
  (void)arg;

  buf = buf_new();
  tt_assert(buf);

  /* Copy an empty buffer. */
  tt_int_op(0, OP_EQ, buf_set_to_copy(&buf2, buf));
  tt_assert(buf2);
  tt_int_op(0, OP_EQ, buf_datalen(buf2));

  /* Now try with a short buffer. */
  s = "And now comes an act of enormous enormance!";
  len = strlen(s);
  buf_add(buf, s, len);
  tt_int_op(len, OP_EQ, buf_datalen(buf));
  /* Add junk to buf2 so we can test replacing.*/
  buf_add(buf2, "BLARG", 5);
  tt_int_op(0, OP_EQ, buf_set_to_copy(&buf2, buf));
  tt_int_op(len, OP_EQ, buf_datalen(buf2));
  buf_get_bytes(buf2, b, len);
  tt_mem_op(b, OP_EQ, s, len);
  /* Now free buf2 and retry so we can test allocating */
  buf_free(buf2);
  buf2 = NULL;
  tt_int_op(0, OP_EQ, buf_set_to_copy(&buf2, buf));
  tt_int_op(len, OP_EQ, buf_datalen(buf2));
  buf_get_bytes(buf2, b, len);
  tt_mem_op(b, OP_EQ, s, len);
  /* Clear buf for next test */
  buf_get_bytes(buf, b, len);
  tt_int_op(buf_datalen(buf),OP_EQ,0);

  /* Okay, now let's try a bigger buffer. */
  s = "Quis autem vel eum iure reprehenderit qui in ea voluptate velit "
    "esse quam nihil molestiae consequatur, vel illum qui dolorem eum "
    "fugiat quo voluptas nulla pariatur?";
  len = strlen(s);
  for (i = 0; i < 256; ++i) {
    b[0]=i;
    buf_add(buf, b, 1);
    buf_add(buf, s, len);
  }
  tt_int_op(0, OP_EQ, buf_set_to_copy(&buf2, buf));
  tt_int_op(buf_datalen(buf2), OP_EQ, buf_datalen(buf));
  for (i = 0; i < 256; ++i) {
    buf_get_bytes(buf2, b, len+1);
    tt_int_op((unsigned char)b[0],OP_EQ,i);
    tt_mem_op(b+1, OP_EQ, s, len);
  }

 done:
  if (buf)
    buf_free(buf);
  if (buf2)
    buf_free(buf2);
}

static void
test_buffer_allocation_tracking(void *arg)
{
  char *junk = tor_malloc(16384);
  buf_t *buf1 = NULL, *buf2 = NULL;
  int i;

  (void)arg;

  crypto_rand(junk, 16384);
  tt_int_op(buf_get_total_allocation(), OP_EQ, 0);

  buf1 = buf_new();
  tt_assert(buf1);
  buf2 = buf_new();
  tt_assert(buf2);

  tt_int_op(buf_allocation(buf1), OP_EQ, 0);
  tt_int_op(buf_get_total_allocation(), OP_EQ, 0);

  buf_add(buf1, junk, 4000);
  buf_add(buf1, junk, 4000);
  buf_add(buf1, junk, 4000);
  buf_add(buf1, junk, 4000);
  tt_int_op(buf_allocation(buf1), OP_EQ, 16384);
  buf_get_bytes(buf1, junk, 100);
  tt_int_op(buf_allocation(buf1), OP_EQ, 16384); /* still 4 4k chunks */

  tt_int_op(buf_get_total_allocation(), OP_EQ, 16384);

  buf_get_bytes(buf1, junk, 4096); /* drop a 1k chunk... */
  tt_int_op(buf_allocation(buf1), OP_EQ, 3*4096); /* now 3 4k chunks */

  tt_int_op(buf_get_total_allocation(), OP_EQ, 12288); /* that chunk was really
                                                       freed. */

  buf_add(buf2, junk, 4000);
  tt_int_op(buf_allocation(buf2), OP_EQ, 4096); /* another 4k chunk. */
  /*
   * We bounce back up to 16384 by allocating a new chunk.
   */
  tt_int_op(buf_get_total_allocation(), OP_EQ, 16384);
  buf_add(buf2, junk, 4000);
  tt_int_op(buf_allocation(buf2), OP_EQ, 8192); /* another 4k chunk. */
  tt_int_op(buf_get_total_allocation(),
            OP_EQ, 5*4096); /* that chunk was new. */

  /* Make a really huge buffer */
  for (i = 0; i < 1000; ++i) {
    buf_add(buf2, junk, 4000);
  }
  tt_int_op(buf_allocation(buf2), OP_GE, 4008000);
  tt_int_op(buf_get_total_allocation(), OP_GE, 4008000);
  buf_free(buf2);
  buf2 = NULL;

  tt_int_op(buf_get_total_allocation(), OP_LT, 4008000);
  tt_int_op(buf_get_total_allocation(), OP_EQ, buf_allocation(buf1));
  buf_free(buf1);
  buf1 = NULL;
  tt_int_op(buf_get_total_allocation(), OP_EQ, 0);

 done:
  buf_free(buf1);
  buf_free(buf2);
  tor_free(junk);
}

static void
test_buffer_time_tracking(void *arg)
{
  buf_t *buf=NULL, *buf2=NULL;
  const time_t START = 1389288246;
  const uint64_t START_NSEC = ((uint64_t)START) * 1000000000;
  int i;
  char tmp[4096];
  (void)arg;

  crypto_rand(tmp, sizeof(tmp));

  monotime_enable_test_mocking();

  buf = buf_new_with_capacity(3000); /* rounds up to next power of 2. */
  tt_assert(buf);

  monotime_coarse_set_mock_time_nsec(START_NSEC);
  const uint32_t START_TS = monotime_coarse_get_stamp();

  /* Empty buffer means the timestamp is 0. */
  tt_int_op(0, OP_EQ, buf_get_oldest_chunk_timestamp(buf, START_TS));
  tt_int_op(0, OP_EQ, buf_get_oldest_chunk_timestamp(buf, START_TS+1000));

  buf_add(buf, "ABCDEFG", 7);
  tt_int_op(1000, OP_EQ, buf_get_oldest_chunk_timestamp(buf, START_TS+1000));

  buf2 = buf_copy(buf);
  tt_assert(buf2);
  tt_int_op(1234, OP_EQ,
            buf_get_oldest_chunk_timestamp(buf2, START_TS+1234));

  /* Now add more bytes; enough to overflow the first chunk. */
  monotime_coarse_set_mock_time_nsec(START_NSEC + 123 * (uint64_t)1000000);
  const uint32_t TS2 = monotime_coarse_get_stamp();
  for (i = 0; i < 600; ++i)
    buf_add(buf, "ABCDEFG", 7);
  tt_int_op(4207, OP_EQ, buf_datalen(buf));

  /* The oldest bytes are still in the front. */
  tt_int_op(2000, OP_EQ, buf_get_oldest_chunk_timestamp(buf, START_TS+2000));

  /* Once those bytes are dropped, the chunk is still on the first
   * timestamp. */
  buf_get_bytes(buf, tmp, 100);
  tt_int_op(2000, OP_EQ, buf_get_oldest_chunk_timestamp(buf, START_TS+2000));

  /* But once we discard the whole first chunk, we get the data in the second
   * chunk. */
  buf_get_bytes(buf, tmp, 4000);
  tt_int_op(107, OP_EQ, buf_datalen(buf));
  tt_int_op(2000, OP_EQ, buf_get_oldest_chunk_timestamp(buf, TS2+2000));

  /* This time we'll be grabbing a chunk from the freelist, and making sure
     its time gets updated */
  monotime_coarse_set_mock_time_nsec(START_NSEC + 5617 * (uint64_t)1000000);
  const uint32_t TS3 = monotime_coarse_get_stamp();
  for (i = 0; i < 600; ++i)
    buf_add(buf, "ABCDEFG", 7);
  tt_int_op(4307, OP_EQ, buf_datalen(buf));

  tt_int_op(2000, OP_EQ, buf_get_oldest_chunk_timestamp(buf, TS2+2000));
  buf_get_bytes(buf, tmp, 4000);
  buf_get_bytes(buf, tmp, 306);
  tt_int_op(0, OP_EQ, buf_get_oldest_chunk_timestamp(buf, TS3));
  tt_int_op(383, OP_EQ, buf_get_oldest_chunk_timestamp(buf, TS3+383));

 done:
  buf_free(buf);
  buf_free(buf2);
  monotime_disable_test_mocking();
}

static void
test_buffers_compress_fin_at_chunk_end_impl(compress_method_t method,
                                            compression_level_t level)
{
  char *msg = NULL;
  char *contents = NULL;
  char *expanded = NULL;
  buf_t *buf = NULL;
  tor_compress_state_t *compress_state = NULL;
  size_t out_len, in_len;
  size_t sz, headerjunk;

  buf = buf_new_with_capacity(128); /* will round up */
  sz = buf_get_default_chunk_size(buf);
  msg = tor_malloc_zero(sz);

  buf_add(buf, msg, 1);
  tt_assert(buf->head);

  /* Fill up the chunk so the compression stuff won't fit in one chunk. */
  tt_uint_op(buf->head->memlen, OP_LT, sz);
  headerjunk = buf->head->memlen - 7;
  buf_add(buf, msg, headerjunk-1);
  tt_uint_op(buf->head->datalen, OP_EQ, headerjunk);
  tt_uint_op(buf_datalen(buf), OP_EQ, headerjunk);
  /* Write an empty string, with finalization on. */
  compress_state = tor_compress_new(1, method, level);
  tt_int_op(buf_add_compress(buf, compress_state, "", 0, 1), OP_EQ, 0);

  in_len = buf_datalen(buf);
  contents = tor_malloc(in_len);

  tt_int_op(buf_get_bytes(buf, contents, in_len), OP_EQ, 0);

  if (method == NO_METHOD) {
    tt_uint_op(in_len, OP_EQ, headerjunk);
  } else {
    tt_uint_op(in_len, OP_GT, headerjunk);
  }

  tt_int_op(0, OP_EQ, tor_uncompress(&expanded, &out_len,
                                     contents + headerjunk,
                                     in_len - headerjunk,
                                     method, 1,
                                     LOG_WARN));

  tt_int_op(out_len, OP_EQ, 0);
  tt_assert(expanded);

 done:
  buf_free(buf);
  tor_compress_free(compress_state);
  tor_free(contents);
  tor_free(expanded);
  tor_free(msg);
}

static void
test_buffers_compress_impl(compress_method_t method,
                           compression_level_t level,
                           int finalize_with_nil)
{
  char *msg = NULL;
  char *contents = NULL;
  char *expanded = NULL;
  buf_t *buf = NULL;
  tor_compress_state_t *compress_state = NULL;
  size_t out_len, in_len;
  int done;

  buf = buf_new_with_capacity(128); /* will round up */
  compress_state = tor_compress_new(1, method, level);

  msg = tor_malloc(512);
  crypto_rand(msg, 512);
  tt_int_op(buf_add_compress(buf, compress_state,
                                  msg, 128, 0), OP_EQ, 0);
  tt_int_op(buf_add_compress(buf, compress_state,
                                  msg+128, 128, 0), OP_EQ, 0);
  tt_int_op(buf_add_compress(buf, compress_state,
                                  msg+256, 256, 0), OP_EQ, 0);
  done = !finalize_with_nil;
  tt_int_op(buf_add_compress(buf, compress_state,
                                  "all done", 9, done), OP_EQ, 0);
  if (finalize_with_nil) {
    tt_int_op(buf_add_compress(buf, compress_state, "", 0, 1), OP_EQ, 0);
  }

  in_len = buf_datalen(buf);
  contents = tor_malloc(in_len);

  tt_int_op(buf_get_bytes(buf, contents, in_len), OP_EQ, 0);

  tt_int_op(0, OP_EQ, tor_uncompress(&expanded, &out_len,
                                     contents, in_len,
                                     method, 1,
                                     LOG_WARN));

  tt_int_op(out_len, OP_GE, 128);
  tt_mem_op(msg, OP_EQ, expanded, 128);
  tt_int_op(out_len, OP_GE, 512);
  tt_mem_op(msg, OP_EQ, expanded, 512);
  tt_int_op(out_len, OP_EQ, 512+9);
  tt_mem_op("all done", OP_EQ, expanded+512, 9);

 done:
  buf_free(buf);
  tor_compress_free(compress_state);
  tor_free(contents);
  tor_free(expanded);
  tor_free(msg);
}

static void
test_buffers_compress(void *arg)
{
  const char *methodname = arg;
  tt_assert(methodname);

  compress_method_t method = compression_method_get_by_name(methodname);
  tt_int_op(method, OP_NE, UNKNOWN_METHOD);

  if (! tor_compress_supports_method(method)) {
    tt_skip();
  }

  compression_level_t levels[] = {
    BEST_COMPRESSION,
    HIGH_COMPRESSION,
    MEDIUM_COMPRESSION,
    LOW_COMPRESSION
  };

  for (unsigned l = 0; l < ARRAY_LENGTH(levels); ++l) {
    compression_level_t level = levels[l];

    test_buffers_compress_impl(method, level, 0);
    test_buffers_compress_impl(method, level, 1);
    test_buffers_compress_fin_at_chunk_end_impl(method, level);
  }

 done:
  ;
}

static const uint8_t *tls_read_ptr;
static int n_remaining;
static int next_reply_val[16];

static int
mock_tls_read(tor_tls_t *tls, char *cp, size_t len)
{
  (void)tls;
  int rv = next_reply_val[0];
  if (rv > 0) {
    int max = rv > (int)len ? (int)len : rv;
    if (max > n_remaining)
      max = n_remaining;
    memcpy(cp, tls_read_ptr, max);
    rv = max;
    n_remaining -= max;
    tls_read_ptr += max;
  }

  memmove(next_reply_val, next_reply_val + 1, 15*sizeof(int));
  return rv;
}

static void
test_buffers_tls_read_mocked(void *arg)
{
  uint8_t *mem;
  buf_t *buf;
  (void)arg;

  mem = tor_malloc(64*1024);
  crypto_rand((char*)mem, 64*1024);
  tls_read_ptr = mem;
  n_remaining = 64*1024;

  MOCK(tor_tls_read, mock_tls_read);

  buf = buf_new();

  next_reply_val[0] = 1024;
  tt_int_op(128, OP_EQ, buf_read_from_tls(buf, NULL, 128));

  next_reply_val[0] = 5000;
  next_reply_val[1] = 5000;
  tt_int_op(6000, OP_EQ, buf_read_from_tls(buf, NULL, 6000));

 done:
  UNMOCK(tor_tls_read);
  tor_free(mem);
  buf_free(buf);
}

static void
test_buffers_chunk_size(void *arg)
{
  (void)arg;
  const int min = 256;
  const int max = 65536;
  tt_uint_op(buf_preferred_chunk_size(3), OP_EQ, min);
  tt_uint_op(buf_preferred_chunk_size(25), OP_EQ, min);
  tt_uint_op(buf_preferred_chunk_size(0), OP_EQ, min);
  tt_uint_op(buf_preferred_chunk_size(256), OP_EQ, 512);
  tt_uint_op(buf_preferred_chunk_size(65400), OP_EQ, max);
  /* Here, we're implicitly saying that the chunk header overhead is
   * between 1 and 100 bytes. 24..48 would probably be more accurate. */
  tt_uint_op(buf_preferred_chunk_size(65536), OP_GT, 65536);
  tt_uint_op(buf_preferred_chunk_size(65536), OP_LT, 65536+100);
  tt_uint_op(buf_preferred_chunk_size(165536), OP_GT, 165536);
  tt_uint_op(buf_preferred_chunk_size(165536), OP_LT, 165536+100);
 done:
  ;
}

static void
test_buffers_find_contentlen(void *arg)
{
  static const struct {
    const char *headers;
    int r;
    int contentlen;
  } results[] = {
    { "Blah blah\r\nContent-Length: 1\r\n\r\n", 1, 1 },
    { "Blah blah\r\n\r\n", 0, 0 }, /* no content-len */
    { "Blah blah Content-Length: 1\r\n", 0, 0 }, /* no content-len. */
    { "Blah blah\r\nContent-Length: 100000\r\n", 1, 100000},
    { "Blah blah\r\nContent-Length: 1000000000000000000000000\r\n", -1, 0},
    { "Blah blah\r\nContent-Length: 0\r\n", 1, 0},
    { "Blah blah\r\nContent-Length: -1\r\n", -1, 0},
    { "Blah blah\r\nContent-Length: 1x\r\n", -1, 0},
    { "Blah blah\r\nContent-Length: 1 x\r\n", -1, 0},
    { "Blah blah\r\nContent-Length: 1 \r\n", 1, 1},
    { "Blah blah\r\nContent-Length:  \r\n", -1, 0},
    { "Blah blah\r\nContent-Length: ", -1, 0},
    { "Blah blah\r\nContent-Length: 5050", -1, 0},
    { NULL, 0, 0 }
  };
  int i;

  (void)arg;

  for (i = 0; results[i].headers; ++i) {
    int r;
    size_t sz;
    size_t headerlen = strlen(results[i].headers);
    char * tmp = tor_memdup(results[i].headers, headerlen);/* ensure no eos */
    sz = 999; /* to ensure it gets set */
    r = buf_http_find_content_length(tmp, headerlen, &sz);
    tor_free(tmp);
    log_debug(LD_DIR, "%d: %s", i, escaped(results[i].headers));
    tt_int_op(r, OP_EQ, results[i].r);
    tt_int_op(sz, OP_EQ, results[i].contentlen);
  }
 done:
  ;
}

static void
test_buffer_peek_startswith(void *arg)
{
  (void)arg;
  buf_t *buf;
  buf = buf_new();
  tt_ptr_op(buf, OP_NE, NULL);

  tt_assert(buf_peek_startswith(buf, ""));
  tt_assert(! buf_peek_startswith(buf, "X"));

  buf_add(buf, "Tor", 3);

  tt_assert(buf_peek_startswith(buf, ""));
  tt_assert(buf_peek_startswith(buf, "T"));
  tt_assert(buf_peek_startswith(buf, "To"));
  tt_assert(buf_peek_startswith(buf, "Tor"));
  tt_assert(! buf_peek_startswith(buf, "Top"));
  tt_assert(! buf_peek_startswith(buf, "For"));
  tt_assert(! buf_peek_startswith(buf, "Tork"));
  tt_assert(! buf_peek_startswith(buf, "Torpor"));

 done:
  buf_free(buf);
}

struct testcase_t buffer_tests[] = {
  { "basic", test_buffers_basic, TT_FORK, NULL, NULL },
  { "copy", test_buffer_copy, TT_FORK, NULL, NULL },
  { "pullup", test_buffer_pullup, TT_FORK, NULL, NULL },
  { "move_all", test_buffers_move_all, 0, NULL, NULL },
  { "startswith", test_buffer_peek_startswith, 0, NULL, NULL },
  { "allocation_tracking", test_buffer_allocation_tracking, TT_FORK,
    NULL, NULL },
  { "time_tracking", test_buffer_time_tracking, TT_FORK, NULL, NULL },
  { "tls_read_mocked", test_buffers_tls_read_mocked, 0,
    NULL, NULL },
  { "chunk_size", test_buffers_chunk_size, 0, NULL, NULL },
  { "find_contentlen", test_buffers_find_contentlen, 0, NULL, NULL },

  { "compress/zlib", test_buffers_compress, TT_FORK,
    &passthrough_setup, (char*)"deflate" },
  { "compress/gzip", test_buffers_compress, TT_FORK,
    &passthrough_setup, (char*)"gzip" },
  { "compress/zstd", test_buffers_compress, TT_FORK,
    &passthrough_setup, (char*)"x-zstd" },
  { "compress/lzma", test_buffers_compress, TT_FORK,
    &passthrough_setup, (char*)"x-tor-lzma" },
  { "compress/none", test_buffers_compress, TT_FORK,
    &passthrough_setup, (char*)"identity" },

  END_OF_TESTCASES
};
