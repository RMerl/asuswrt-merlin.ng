/* Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#include "orconfig.h"
#define COMPAT_TIME_PRIVATE
#define UTIL_MALLOC_PRIVATE
#define PROCESS_WIN32_PRIVATE
#define TIME_FMT_PRIVATE
#include "lib/testsupport/testsupport.h"
#include "core/or/or.h"
#include "lib/buf/buffers.h"
#include "app/config/config.h"
#include "feature/control/control.h"
#include "feature/control/control_proto.h"
#include "feature/client/transports.h"
#include "lib/crypt_ops/crypto_format.h"
#include "lib/crypt_ops/crypto_rand.h"
#include "lib/defs/time.h"
#include "test/test.h"
#include "test/test_helpers.h"
#include "lib/memarea/memarea.h"
#include "lib/process/waitpid.h"
#include "lib/process/process_win32.h"
#include "test/log_test_helpers.h"
#include "lib/compress/compress.h"
#include "lib/compress/compress_zstd.h"
#include "lib/encoding/keyval.h"
#include "lib/fdio/fdio.h"
#include "lib/fs/winlib.h"
#include "lib/process/env.h"
#include "lib/process/pidfile.h"
#include "lib/intmath/weakrng.h"
#include "lib/intmath/muldiv.h"
#include "lib/thread/numcpus.h"
#include "lib/math/fp.h"
#include "lib/math/laplace.h"
#include "lib/meminfo/meminfo.h"
#include "lib/time/tvdiff.h"
#include "lib/encoding/confline.h"
#include "lib/net/socketpair.h"
#include "lib/malloc/map_anon.h"

#ifdef HAVE_PWD_H
#include <pwd.h>
#endif
#ifdef HAVE_SYS_UTIME_H
#include <sys/utime.h>
#endif
#ifdef HAVE_UTIME_H
#include <utime.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SYS_MMAN_H
#include <sys/mman.h>
#endif
#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

#ifdef _WIN32
#include <tchar.h>
#endif
#include <math.h>
#include <ctype.h>
#include <float.h>

/* These platforms don't have meaningful pwdb or homedirs. */
#if defined(_WIN32) || defined(__ANDROID__)
#define DISABLE_PWDB_TESTS
#endif

static void set_file_mtime(const char *fname, time_t when);

#define INFINITY_DBL ((double)INFINITY)
#define NAN_DBL ((double)NAN)

/** Test the tor_isinf() wrapper */
static void
test_tor_isinf(void *arg)
{
  (void) arg;

  tt_assert(tor_isinf(INFINITY_DBL));

  tt_assert(!tor_isinf(NAN_DBL));
  tt_assert(!tor_isinf(DBL_EPSILON));
  tt_assert(!tor_isinf(DBL_MAX));
  tt_assert(!tor_isinf(DBL_MIN));

  tt_assert(!tor_isinf(0.0));
  tt_assert(!tor_isinf(0.1));
  tt_assert(!tor_isinf(3));
  tt_assert(!tor_isinf(3.14));

 done:
  ;
}

/* XXXX this is a minimal wrapper to make the unit tests compile with the
 * changed tor_timegm interface. */
static time_t
tor_timegm_wrapper(const struct tm *tm)
{
  time_t t;
  if (tor_timegm_impl(tm, &t) < 0)
    return -1;
  return t;
}

#define tor_timegm tor_timegm_wrapper

static void
test_util_read_until_eof_impl(const char *fname, size_t file_len,
                              size_t read_limit)
{
  char *fifo_name = NULL;
  char *test_str = NULL;
  char *str = NULL;
  size_t sz = 9999999;
  int fd = -1;
  int r;

  fifo_name = tor_strdup(get_fname(fname));
  test_str = tor_malloc(file_len);
  crypto_rand(test_str, file_len);

  r = write_bytes_to_file(fifo_name, test_str, file_len, 1);
  tt_int_op(r, OP_EQ, 0);

  fd = open(fifo_name, O_RDONLY|O_BINARY);
  tt_int_op(fd, OP_GE, 0);
  str = read_file_to_str_until_eof(fd, read_limit, &sz);
  tt_ptr_op(str, OP_NE, NULL);

  if (read_limit < file_len)
    tt_int_op(sz, OP_EQ, read_limit);
  else
    tt_int_op(sz, OP_EQ, file_len);

  tt_mem_op(test_str, OP_EQ, str, sz);
  tt_int_op(str[sz], OP_EQ, '\0');

 done:
  unlink(fifo_name);
  tor_free(fifo_name);
  tor_free(test_str);
  tor_free(str);
  if (fd >= 0)
    close(fd);
}

static void
test_util_read_file_eof_tiny_limit(void *arg)
{
  (void)arg;
  // purposely set limit shorter than what we wrote to the FIFO to
  // test the maximum, and that it puts the NUL in the right spot

  test_util_read_until_eof_impl("tor_test_fifo_tiny", 5, 4);
}

static void
test_util_read_file_eof_one_loop_a(void *arg)
{
  (void)arg;
  test_util_read_until_eof_impl("tor_test_fifo_1ka", 1024, 1023);
}

static void
test_util_read_file_eof_one_loop_b(void *arg)
{
  (void)arg;
  test_util_read_until_eof_impl("tor_test_fifo_1kb", 1024, 1024);
}

static void
test_util_read_file_eof_two_loops(void *arg)
{
  (void)arg;
  // write more than 1024 bytes to the FIFO to test two passes through
  // the loop in the method; if the re-alloc size is changed this
  // should be updated as well.

  test_util_read_until_eof_impl("tor_test_fifo_2k", 2048, 10000);
}

static void
test_util_read_file_eof_two_loops_b(void *arg)
{
  (void)arg;

  test_util_read_until_eof_impl("tor_test_fifo_2kb", 2048, 2048);
}

static void
test_util_read_file_eof_zero_bytes(void *arg)
{
  (void)arg;
  // zero-byte fifo
  test_util_read_until_eof_impl("tor_test_fifo_empty", 0, 10000);
}

static void
test_util_read_file_endlines(void *arg)
{
  (void)arg;

  char *fname = NULL;
  char *read_content = NULL;
  int r = -1;

  /* Write a file that contains both \n and \r\n as line ending. */
  const char *file_content = "foo bar\n"
                             "foo bar baz\r\n"
                             "foo bar\r\n";

  const char *expected_file_content = "foo bar\n"
                                      "foo bar baz\n"
                                      "foo bar\n";

  fname = tor_strdup(get_fname("file_with_crlf_ending"));

  r = write_bytes_to_file(fname, file_content, strlen(file_content), 1);
  tt_int_op(r, OP_EQ, 0);

  /* Read the file in text mode: we strip \r's from the files on both Windows
   * and UNIX. */
  read_content = read_file_to_str(fname, 0, NULL);

  tt_ptr_op(read_content, OP_NE, NULL);
  tt_int_op(strlen(read_content), OP_EQ, strlen(expected_file_content));
  tt_str_op(read_content, OP_EQ, expected_file_content);

  tor_free(read_content);

  /* Read the file in binary mode: we should preserve the \r here. */
  read_content = read_file_to_str(fname, RFTS_BIN, NULL);

  tt_ptr_op(read_content, OP_NE, NULL);
  tt_int_op(strlen(read_content), OP_EQ, strlen(file_content));
  tt_str_op(read_content, OP_EQ, file_content);

  tor_free(read_content);

 done:
  unlink(fname);
  tor_free(fname);
  tor_free(read_content);
}

/* Test the basic expected behaviour for write_chunks_to_file.
 * NOTE: This will need to be updated if we ever change the tempfile location
 * or extension */
static void
test_util_write_chunks_to_file(void *arg)
{
  char *fname = NULL;
  char *tempname = NULL;
  char *str = NULL;
  int r;
  struct stat st;

  /* These should be two different sizes to ensure the data is different
   * between the data file and the temp file's 'known string' */
  int temp_str_len = 1024;
  int data_str_len = 512;
  char *data_str = tor_malloc(data_str_len);
  char *temp_str = tor_malloc(temp_str_len);

  smartlist_t *chunks = smartlist_new();
  sized_chunk_t c = {data_str, data_str_len/2};
  sized_chunk_t c2 = {data_str + data_str_len/2, data_str_len/2};
  (void)arg;

  crypto_rand(temp_str, temp_str_len);
  crypto_rand(data_str, data_str_len);

  // Ensure it can write multiple chunks

  smartlist_add(chunks, &c);
  smartlist_add(chunks, &c2);

  /*
  * Check if it writes using a tempfile
  */
  fname = tor_strdup(get_fname("write_chunks_with_tempfile"));
  tor_asprintf(&tempname, "%s.tmp", fname);

  // write a known string to a file where the tempfile will be
  r = write_bytes_to_file(tempname, temp_str, temp_str_len, 1);
  tt_int_op(r, OP_EQ, 0);

  // call write_chunks_to_file
  r = write_chunks_to_file(fname, chunks, 1, 0);
  tt_int_op(r, OP_EQ, 0);

  // assert the file has been written (expected size)
  str = read_file_to_str(fname, RFTS_BIN, &st);
  tt_assert(str != NULL);
  tt_u64_op((uint64_t)st.st_size, OP_EQ, data_str_len);
  tt_mem_op(data_str, OP_EQ, str, data_str_len);
  tor_free(str);

  // assert that the tempfile is removed (should not leave artifacts)
  str = read_file_to_str(tempname, RFTS_BIN|RFTS_IGNORE_MISSING, &st);
  tt_assert(str == NULL);

  // Remove old testfile for second test
  r = unlink(fname);
  tt_int_op(r, OP_EQ, 0);
  tor_free(fname);
  tor_free(tempname);

  /*
  *  Check if it skips using a tempfile with flags
  */
  fname = tor_strdup(get_fname("write_chunks_with_no_tempfile"));
  tor_asprintf(&tempname, "%s.tmp", fname);

  // write a known string to a file where the tempfile will be
  r = write_bytes_to_file(tempname, temp_str, temp_str_len, 1);
  tt_int_op(r, OP_EQ, 0);

  // call write_chunks_to_file with no_tempfile = true
  r = write_chunks_to_file(fname, chunks, 1, 1);
  tt_int_op(r, OP_EQ, 0);

  // assert the file has been written (expected size)
  str = read_file_to_str(fname, RFTS_BIN, &st);
  tt_assert(str != NULL);
  tt_u64_op((uint64_t)st.st_size, OP_EQ, data_str_len);
  tt_mem_op(data_str, OP_EQ, str, data_str_len);
  tor_free(str);

  // assert the tempfile still contains the known string
  str = read_file_to_str(tempname, RFTS_BIN, &st);
  tt_assert(str != NULL);
  tt_u64_op((uint64_t)st.st_size, OP_EQ, temp_str_len);
  tt_mem_op(temp_str, OP_EQ, str, temp_str_len);

 done:
  unlink(fname);
  unlink(tempname);
  smartlist_free(chunks);
  tor_free(fname);
  tor_free(tempname);
  tor_free(str);
  tor_free(data_str);
  tor_free(temp_str);
}

/* Test write_str_to_file_if_not_equal(). */
static void
test_util_write_str_if_changed(void *arg)
{
  (void)arg;
  char *fname = tor_strdup(get_fname("write_if_changed"));
  char *s = NULL;
  int rv;
  const char str1[] = "The wombat lives across the seas";
  const char str2[] = "Among the far Antipodes"; /* -- Ogden Nash */

  /* We can create files. */
  rv = write_str_to_file_if_not_equal(fname, str1);
  tt_int_op(rv, OP_EQ, 0);
  s = read_file_to_str(fname, 0, NULL);
  tt_str_op(s, OP_EQ, str1);
  tor_free(s);

  /* We can replace files. */
  rv = write_str_to_file_if_not_equal(fname, str2);
  tt_int_op(rv, OP_EQ, 0);
  s = read_file_to_str(fname, 0, NULL);
  tt_str_op(s, OP_EQ, str2);
  tor_free(s);

  /* Make sure we don't replace files when they're equal. (That's the whole
   * point of the function we're testing. */
  /* First, change the mtime of the file so that we can tell whether we
   * replaced it. */
  const time_t now = time(NULL);
  const time_t five_sec_ago = now - 5;
  set_file_mtime(fname, five_sec_ago);
  rv = write_str_to_file_if_not_equal(fname, str2);
  tt_int_op(rv, OP_EQ, 0);
  /* Make sure that the file's mtime is unchanged... */
  struct stat st;
  rv = stat(fname, &st);
  tt_int_op(rv, OP_EQ, 0);
  tt_i64_op(st.st_mtime, OP_EQ, five_sec_ago);
  /* And make sure its contents are unchanged. */
  s = read_file_to_str(fname, 0, NULL);
  tt_str_op(s, OP_EQ, str2);
  tor_free(s);

 done:
  tor_free(fname);
  tor_free(s);
}

#ifndef COCCI
#define _TFE(a, b, f)  tt_int_op((a).f, OP_EQ, (b).f)
/** test the minimum set of struct tm fields needed for a unique epoch value
 * this is also the set we use to test tor_timegm */
#define TM_EQUAL(a, b)           \
          TT_STMT_BEGIN          \
            _TFE(a, b, tm_year); \
            _TFE(a, b, tm_mon ); \
            _TFE(a, b, tm_mday); \
            _TFE(a, b, tm_hour); \
            _TFE(a, b, tm_min ); \
            _TFE(a, b, tm_sec ); \
          TT_STMT_END
#endif /* !defined(COCCI) */

static void
test_util_time(void *arg)
{
  struct timeval start, end;
  struct tm a_time, b_time;
  char timestr[128];
  time_t t_res;
  int i;
  struct timeval tv;

  /* Test tv_udiff and tv_mdiff */

  (void)arg;
  start.tv_sec = 5;
  start.tv_usec = 5000;

  end.tv_sec = 5;
  end.tv_usec = 5000;

  tt_int_op(0L,OP_EQ, tv_udiff(&start, &end));
  tt_int_op(0L,OP_EQ, tv_mdiff(&start, &end));
  tt_int_op(0L,OP_EQ, tv_udiff(&end, &start));
  tt_int_op(0L,OP_EQ, tv_mdiff(&end, &start));

  end.tv_usec = 7000;

  tt_int_op(2000L,OP_EQ, tv_udiff(&start, &end));
  tt_int_op(2L,OP_EQ, tv_mdiff(&start, &end));
  tt_int_op(-2000L,OP_EQ, tv_udiff(&end, &start));
  tt_int_op(-2L,OP_EQ, tv_mdiff(&end, &start));

  end.tv_sec = 6;

  tt_int_op(1002000L,OP_EQ, tv_udiff(&start, &end));
  tt_int_op(1002L,OP_EQ, tv_mdiff(&start, &end));
  tt_int_op(-1002000L,OP_EQ, tv_udiff(&end, &start));
  tt_int_op(-1002L,OP_EQ, tv_mdiff(&end, &start));

  end.tv_usec = 0;

  tt_int_op(995000L,OP_EQ, tv_udiff(&start, &end));
  tt_int_op(995L,OP_EQ, tv_mdiff(&start, &end));
  tt_int_op(-995000L,OP_EQ, tv_udiff(&end, &start));
  tt_int_op(-995L,OP_EQ, tv_mdiff(&end, &start));

  end.tv_sec = 4;

  tt_int_op(-1005000L,OP_EQ, tv_udiff(&start, &end));
  tt_int_op(-1005L,OP_EQ, tv_mdiff(&start, &end));
  tt_int_op(1005000L,OP_EQ, tv_udiff(&end, &start));
  tt_int_op(1005L,OP_EQ, tv_mdiff(&end, &start));

  /* Negative tv_sec values, these will break on platforms where tv_sec is
   * unsigned */

  end.tv_sec = -10;

  tt_int_op(-15005000L,OP_EQ, tv_udiff(&start, &end));
  tt_int_op(-15005L,OP_EQ, tv_mdiff(&start, &end));
  tt_int_op(15005000L,OP_EQ, tv_udiff(&end, &start));
  tt_int_op(15005L,OP_EQ, tv_mdiff(&end, &start));

  start.tv_sec = -100;

  tt_int_op(89995000L,OP_EQ, tv_udiff(&start, &end));
  tt_int_op(89995L,OP_EQ, tv_mdiff(&start, &end));
  tt_int_op(-89995000L,OP_EQ, tv_udiff(&end, &start));
  tt_int_op(-89995L,OP_EQ, tv_mdiff(&end, &start));

  /* Test that tv_usec values round away from zero when converted to msec */
  start.tv_sec = 0;
  start.tv_usec = 0;
  end.tv_sec = 10;
  end.tv_usec = 499;

  tt_int_op(10000499L, OP_EQ, tv_udiff(&start, &end));
  tt_int_op(10000L, OP_EQ, tv_mdiff(&start, &end));
  tt_int_op(-10000499L, OP_EQ, tv_udiff(&end, &start));
  tt_int_op(-10000L, OP_EQ, tv_mdiff(&end, &start));

  start.tv_sec = 0;
  start.tv_usec = 0;
  end.tv_sec = 10;
  end.tv_usec = 500;

  tt_int_op(10000500L, OP_EQ, tv_udiff(&start, &end));
  tt_int_op(10001L, OP_EQ, tv_mdiff(&start, &end));
  tt_int_op(-10000500L, OP_EQ, tv_udiff(&end, &start));
  tt_int_op(-10000L, OP_EQ, tv_mdiff(&end, &start));

  start.tv_sec = 0;
  start.tv_usec = 0;
  end.tv_sec = 10;
  end.tv_usec = 501;

  tt_int_op(10000501L, OP_EQ, tv_udiff(&start, &end));
  tt_int_op(10001L, OP_EQ, tv_mdiff(&start, &end));
  tt_int_op(-10000501L, OP_EQ, tv_udiff(&end, &start));
  tt_int_op(-10001L, OP_EQ, tv_mdiff(&end, &start));

  /* Overflow conditions */

#ifdef _WIN32
  /* Would you believe that tv_sec is a long on windows? Of course you would.*/
#define TV_SEC_MAX LONG_MAX
#define TV_SEC_MIN LONG_MIN
#else
  /* Some BSDs have struct timeval.tv_sec 64-bit, but time_t (and long) 32-bit
   * Which means TIME_MAX is not actually the maximum value of tv_sec.
   * But that's ok for the moment, because the code correctly performs 64-bit
   * calculations internally, then catches the overflow. */
#define TV_SEC_MAX TIME_MAX
#define TV_SEC_MIN TIME_MIN
#endif /* defined(_WIN32) */

/* Assume tv_usec is an unsigned integer until proven otherwise */
#define TV_USEC_MAX UINT_MAX

  /* Overflows in the result type */

  /* All comparisons work */
  start.tv_sec = 0;
  start.tv_usec = 0;
  end.tv_sec = LONG_MAX/1000 - 2;
  end.tv_usec = 0;

  tt_int_op(LONG_MAX, OP_EQ, tv_udiff(&start, &end));
  tt_int_op(end.tv_sec*1000L, OP_EQ, tv_mdiff(&start, &end));
  tt_int_op(LONG_MAX, OP_EQ, tv_udiff(&end, &start));
  tt_int_op(-end.tv_sec*1000L, OP_EQ, tv_mdiff(&end, &start));

  start.tv_sec = 0;
  start.tv_usec = 0;
  end.tv_sec = LONG_MAX/1000000 - 1;
  end.tv_usec = 0;

  tt_int_op(end.tv_sec*1000000L, OP_EQ, tv_udiff(&start, &end));
  tt_int_op(end.tv_sec*1000L, OP_EQ, tv_mdiff(&start, &end));
  tt_int_op(-end.tv_sec*1000000L, OP_EQ, tv_udiff(&end, &start));
  tt_int_op(-end.tv_sec*1000L, OP_EQ, tv_mdiff(&end, &start));

  /* No comparisons work */
  start.tv_sec = 0;
  start.tv_usec = 0;
  end.tv_sec = LONG_MAX/1000 + 1;
  end.tv_usec = 0;

  tt_int_op(LONG_MAX, OP_EQ, tv_udiff(&start, &end));
  tt_int_op(LONG_MAX, OP_EQ, tv_mdiff(&start, &end));
  tt_int_op(LONG_MAX, OP_EQ, tv_udiff(&end, &start));
  tt_int_op(LONG_MAX, OP_EQ, tv_mdiff(&end, &start));

  start.tv_sec = 0;
  start.tv_usec = 0;
  end.tv_sec = LONG_MAX/1000000 + 1;
  end.tv_usec = 0;

  tt_int_op(LONG_MAX, OP_EQ, tv_udiff(&start, &end));
  tt_int_op(end.tv_sec*1000L, OP_EQ, tv_mdiff(&start, &end));
  tt_int_op(LONG_MAX, OP_EQ, tv_udiff(&end, &start));
  tt_int_op(-end.tv_sec*1000L, OP_EQ, tv_mdiff(&end, &start));

  start.tv_sec = 0;
  start.tv_usec = 0;
  end.tv_sec = LONG_MAX/1000;
  end.tv_usec = TOR_USEC_PER_SEC;

  tt_int_op(LONG_MAX, OP_EQ, tv_udiff(&start, &end));
  tt_int_op(LONG_MAX, OP_EQ, tv_mdiff(&start, &end));
  tt_int_op(LONG_MAX, OP_EQ, tv_udiff(&end, &start));
  tt_int_op(LONG_MAX, OP_EQ, tv_mdiff(&end, &start));

  start.tv_sec = 0;
  start.tv_usec = 0;
  end.tv_sec = LONG_MAX/1000000;
  end.tv_usec = TOR_USEC_PER_SEC;

  tt_int_op(LONG_MAX, OP_EQ, tv_udiff(&start, &end));
  tt_int_op((end.tv_sec + 1)*1000L, OP_EQ, tv_mdiff(&start, &end));
  tt_int_op(LONG_MAX, OP_EQ, tv_udiff(&end, &start));
  tt_int_op(-(end.tv_sec + 1)*1000L, OP_EQ, tv_mdiff(&end, &start));

  /* Overflows on comparison to zero */

  start.tv_sec = 0;
  start.tv_usec = 0;

  end.tv_sec = TV_SEC_MAX;
  end.tv_usec = 0;

  tt_int_op(LONG_MAX, OP_EQ, tv_udiff(&start, &end));
  tt_int_op(LONG_MAX, OP_EQ, tv_mdiff(&start, &end));
  tt_int_op(LONG_MAX, OP_EQ, tv_udiff(&end, &start));
  tt_int_op(LONG_MAX, OP_EQ, tv_mdiff(&end, &start));

  end.tv_sec = TV_SEC_MAX;
  end.tv_usec = TOR_USEC_PER_SEC;

  tt_int_op(LONG_MAX, OP_EQ, tv_udiff(&start, &end));
  tt_int_op(LONG_MAX, OP_EQ, tv_mdiff(&start, &end));
  tt_int_op(LONG_MAX, OP_EQ, tv_udiff(&end, &start));
  tt_int_op(LONG_MAX, OP_EQ, tv_mdiff(&end, &start));

  end.tv_sec = 0;
  end.tv_usec = TV_USEC_MAX;

  tt_int_op(LONG_MAX, OP_EQ, tv_udiff(&start, &end));
  tt_int_op(LONG_MAX, OP_EQ, tv_mdiff(&start, &end));
  tt_int_op(LONG_MAX, OP_EQ, tv_udiff(&end, &start));
  tt_int_op(LONG_MAX, OP_EQ, tv_mdiff(&end, &start));

  end.tv_sec = TV_SEC_MAX;
  end.tv_usec = TV_USEC_MAX;

  tt_int_op(LONG_MAX, OP_EQ, tv_udiff(&start, &end));
  tt_int_op(LONG_MAX, OP_EQ, tv_mdiff(&start, &end));
  tt_int_op(LONG_MAX, OP_EQ, tv_udiff(&end, &start));
  tt_int_op(LONG_MAX, OP_EQ, tv_mdiff(&end, &start));

  end.tv_sec = 0;
  end.tv_usec = 0;

  start.tv_sec = TV_SEC_MIN;
  start.tv_usec = 0;

  tt_int_op(LONG_MAX, OP_EQ, tv_udiff(&start, &end));
  tt_int_op(LONG_MAX, OP_EQ, tv_mdiff(&start, &end));
  tt_int_op(LONG_MAX, OP_EQ, tv_udiff(&end, &start));
  tt_int_op(LONG_MAX, OP_EQ, tv_mdiff(&end, &start));

  start.tv_sec = TV_SEC_MIN;
  start.tv_usec = TOR_USEC_PER_SEC;

  tt_int_op(LONG_MAX, OP_EQ, tv_udiff(&start, &end));
  tt_int_op(LONG_MAX, OP_EQ, tv_mdiff(&start, &end));
  tt_int_op(LONG_MAX, OP_EQ, tv_udiff(&end, &start));
  tt_int_op(LONG_MAX, OP_EQ, tv_mdiff(&end, &start));

  start.tv_sec = TV_SEC_MIN;
  start.tv_usec = TV_USEC_MAX;

  tt_int_op(LONG_MAX, OP_EQ, tv_udiff(&start, &end));
  tt_int_op(LONG_MAX, OP_EQ, tv_mdiff(&start, &end));
  tt_int_op(LONG_MAX, OP_EQ, tv_udiff(&end, &start));
  tt_int_op(LONG_MAX, OP_EQ, tv_mdiff(&end, &start));

  /* overflows on comparison to maxima / minima */

  start.tv_sec = TV_SEC_MIN;
  start.tv_usec = 0;

  end.tv_sec = TV_SEC_MAX;
  end.tv_usec = 0;

  tt_int_op(LONG_MAX, OP_EQ, tv_udiff(&start, &end));
  tt_int_op(LONG_MAX, OP_EQ, tv_mdiff(&start, &end));
  tt_int_op(LONG_MAX, OP_EQ, tv_udiff(&end, &start));
  tt_int_op(LONG_MAX, OP_EQ, tv_mdiff(&end, &start));

  end.tv_sec = TV_SEC_MAX;
  end.tv_usec = TOR_USEC_PER_SEC;

  tt_int_op(LONG_MAX, OP_EQ, tv_udiff(&start, &end));
  tt_int_op(LONG_MAX, OP_EQ, tv_mdiff(&start, &end));
  tt_int_op(LONG_MAX, OP_EQ, tv_udiff(&end, &start));
  tt_int_op(LONG_MAX, OP_EQ, tv_mdiff(&end, &start));

  end.tv_sec = TV_SEC_MAX;
  end.tv_usec = 0;

  start.tv_sec = TV_SEC_MIN;
  start.tv_usec = 0;

  tt_int_op(LONG_MAX, OP_EQ, tv_udiff(&start, &end));
  tt_int_op(LONG_MAX, OP_EQ, tv_mdiff(&start, &end));
  tt_int_op(LONG_MAX, OP_EQ, tv_udiff(&end, &start));
  tt_int_op(LONG_MAX, OP_EQ, tv_mdiff(&end, &start));

  start.tv_sec = TV_SEC_MIN;
  start.tv_usec = TOR_USEC_PER_SEC;

  tt_int_op(LONG_MAX, OP_EQ, tv_udiff(&start, &end));
  tt_int_op(LONG_MAX, OP_EQ, tv_mdiff(&start, &end));
  tt_int_op(LONG_MAX, OP_EQ, tv_udiff(&end, &start));
  tt_int_op(LONG_MAX, OP_EQ, tv_mdiff(&end, &start));

  /* overflows on comparison to maxima / minima with extra usec */

  start.tv_sec = TV_SEC_MIN;
  start.tv_usec = TOR_USEC_PER_SEC;

  end.tv_sec = TV_SEC_MAX;
  end.tv_usec = 0;

  tt_int_op(LONG_MAX, OP_EQ, tv_udiff(&start, &end));
  tt_int_op(LONG_MAX, OP_EQ, tv_mdiff(&start, &end));
  tt_int_op(LONG_MAX, OP_EQ, tv_udiff(&end, &start));
  tt_int_op(LONG_MAX, OP_EQ, tv_mdiff(&end, &start));

  end.tv_sec = TV_SEC_MAX;
  end.tv_usec = TOR_USEC_PER_SEC;

  tt_int_op(LONG_MAX, OP_EQ, tv_udiff(&start, &end));
  tt_int_op(LONG_MAX, OP_EQ, tv_mdiff(&start, &end));
  tt_int_op(LONG_MAX, OP_EQ, tv_udiff(&end, &start));
  tt_int_op(LONG_MAX, OP_EQ, tv_mdiff(&end, &start));

  end.tv_sec = TV_SEC_MAX;
  end.tv_usec = TOR_USEC_PER_SEC;

  start.tv_sec = TV_SEC_MIN;
  start.tv_usec = 0;

  tt_int_op(LONG_MAX, OP_EQ, tv_udiff(&start, &end));
  tt_int_op(LONG_MAX, OP_EQ, tv_mdiff(&start, &end));
  tt_int_op(LONG_MAX, OP_EQ, tv_udiff(&end, &start));
  tt_int_op(LONG_MAX, OP_EQ, tv_mdiff(&end, &start));

  start.tv_sec = TV_SEC_MIN;
  start.tv_usec = TOR_USEC_PER_SEC;

  tt_int_op(LONG_MAX, OP_EQ, tv_udiff(&start, &end));
  tt_int_op(LONG_MAX, OP_EQ, tv_mdiff(&start, &end));
  tt_int_op(LONG_MAX, OP_EQ, tv_udiff(&end, &start));
  tt_int_op(LONG_MAX, OP_EQ, tv_mdiff(&end, &start));

  /* Test tor_timegm & tor_gmtime_r */

  /* The test values here are confirmed to be correct on a platform
   * with a working timegm & gmtime_r. */

  /* Start with known-zero a_time and b_time.
   * This avoids passing uninitialised values to TM_EQUAL in a_time.
   * Zeroing may not be needed for b_time, as long as tor_gmtime_r
   * never reads the existing values in the structure.
   * But we really don't want intermittently failing tests. */
  memset(&a_time, 0, sizeof(struct tm));
  memset(&b_time, 0, sizeof(struct tm));

  a_time.tm_year = 2003-1900;
  a_time.tm_mon = 7;
  a_time.tm_mday = 30;
  a_time.tm_hour = 6;
  a_time.tm_min = 14;
  a_time.tm_sec = 55;
  t_res = 1062224095UL;
  tt_int_op(t_res, OP_EQ, tor_timegm(&a_time));
  tor_gmtime_r(&t_res, &b_time);
  TM_EQUAL(a_time, b_time);

  a_time.tm_year = 2004-1900; /* Try a leap year, after feb. */
  t_res = 1093846495UL;
  tt_int_op(t_res, OP_EQ, tor_timegm(&a_time));
  tor_gmtime_r(&t_res, &b_time);
  TM_EQUAL(a_time, b_time);

  a_time.tm_mon = 1;          /* Try a leap year, in feb. */
  a_time.tm_mday = 10;
  t_res = 1076393695UL;
  tt_int_op(t_res, OP_EQ, tor_timegm(&a_time));
  tor_gmtime_r(&t_res, &b_time);
  TM_EQUAL(a_time, b_time);

  a_time.tm_mon = 0;
  t_res = 1073715295UL;
  tt_int_op(t_res, OP_EQ, tor_timegm(&a_time));
  tor_gmtime_r(&t_res, &b_time);
  TM_EQUAL(a_time, b_time);

  /* This value is in range with 32 bit and 64 bit time_t */
  a_time.tm_year = 2037-1900;
  t_res = 2115180895UL;
  tt_int_op(t_res, OP_EQ, tor_timegm(&a_time));
  tor_gmtime_r(&t_res, &b_time);
  TM_EQUAL(a_time, b_time);

  /* This value is out of range with 32 bit time_t, but in range for 64 bit
   * time_t */
  a_time.tm_year = 2039-1900;
#if SIZEOF_TIME_T == 4
  setup_full_capture_of_logs(LOG_WARN);
  tt_int_op((time_t) -1,OP_EQ, tor_timegm(&a_time));
  //expect_single_log_msg_containing("Result does not fit in tor_timegm");
  teardown_capture_of_logs();
#elif SIZEOF_TIME_T == 8
  t_res = 2178252895UL;
  tt_int_op(t_res, OP_EQ, tor_timegm(&a_time));
  tor_gmtime_r(&t_res, &b_time);
  TM_EQUAL(a_time, b_time);
#endif /* SIZEOF_TIME_T == 4 || ... */

  /* Test tor_timegm out of range */

  /* The below tests will all cause a BUG message, so we capture, suppress,
   * and detect. */
#define CAPTURE() do {                                          \
    teardown_capture_of_logs();                                 \
    setup_full_capture_of_logs(LOG_WARN);                       \
  } while (0)
#define CHECK_TIMEGM_WARNING(msg) do { \
    expect_single_log_msg_containing(msg);                              \
  } while (0)
#define CHECK_POSSIBLE_EINVAL() do {                            \
    if (mock_saved_log_n_entries()) {                           \
      expect_single_log_msg_containing("Invalid argument");     \
    }                                                           \
  } while (0)

#define CHECK_TIMEGM_ARG_OUT_OF_RANGE(msg) \
    CHECK_TIMEGM_WARNING("Out-of-range argument to tor_timegm")

  /* year */

  /* Wrong year < 1970 */
  a_time.tm_year = 1969-1900;
  CAPTURE();
  tt_int_op((time_t) -1,OP_EQ, tor_timegm(&a_time));
  CHECK_TIMEGM_ARG_OUT_OF_RANGE();

  a_time.tm_year = -1-1900;
  CAPTURE();
  tt_int_op((time_t) -1,OP_EQ, tor_timegm(&a_time));
  CHECK_TIMEGM_ARG_OUT_OF_RANGE();

#if SIZEOF_INT == 4 || SIZEOF_INT == 8
    a_time.tm_year = -1*(1 << 16);
    CAPTURE();
    tt_int_op((time_t) -1,OP_EQ, tor_timegm(&a_time));
    CHECK_TIMEGM_ARG_OUT_OF_RANGE();

    /* one of the smallest tm_year values my 64 bit system supports:
     * t_res = -9223372036854775LL without clamping */
    a_time.tm_year = -292275055-1900;
    CAPTURE();
    tt_int_op((time_t) -1,OP_EQ, tor_timegm(&a_time));
    CHECK_TIMEGM_ARG_OUT_OF_RANGE();

    a_time.tm_year = INT32_MIN;
    CAPTURE();
    tt_int_op((time_t) -1,OP_EQ, tor_timegm(&a_time));
    CHECK_TIMEGM_ARG_OUT_OF_RANGE();
#endif /* SIZEOF_INT == 4 || SIZEOF_INT == 8 */

#if SIZEOF_INT == 8
    a_time.tm_year = -1*(1 << 48);
    CAPTURE();
    tt_int_op((time_t) -1,OP_EQ, tor_timegm(&a_time));
    CHECK_TIMEGM_ARG_OUT_OF_RANGE();

    /* while unlikely, the system's gmtime(_r) could return
     * a "correct" retrospective gregorian negative year value,
     * which I'm pretty sure is:
     * -1*(2^63)/60/60/24*2000/730485 + 1970 = -292277022657
     * 730485 is the number of days in two millennia, including leap days */
    a_time.tm_year = -292277022657-1900;
    CAPTURE();
    tt_int_op((time_t) -1,OP_EQ, tor_timegm(&a_time));
    CHECK_TIMEGM_ARG_OUT_OF_RANGE();

    a_time.tm_year = INT64_MIN;
    CAPTURE();
    tt_int_op((time_t) -1,OP_EQ, tor_timegm(&a_time));
    CHECK_TIMEGM_ARG_OUT_OF_RANGE();
#endif /* SIZEOF_INT == 8 */

  /* Wrong year >= INT32_MAX - 1900 */
#if SIZEOF_INT == 4 || SIZEOF_INT == 8
    a_time.tm_year = INT32_MAX-1900;
    CAPTURE();
    tt_int_op((time_t) -1,OP_EQ, tor_timegm(&a_time));
    CHECK_TIMEGM_ARG_OUT_OF_RANGE();

    a_time.tm_year = INT32_MAX;
    CAPTURE();
    tt_int_op((time_t) -1,OP_EQ, tor_timegm(&a_time));
    CHECK_TIMEGM_ARG_OUT_OF_RANGE();
#endif /* SIZEOF_INT == 4 || SIZEOF_INT == 8 */

#if SIZEOF_INT == 8
    /* one of the largest tm_year values my 64 bit system supports */
    a_time.tm_year = 292278994-1900;
    CAPTURE();
    tt_int_op((time_t) -1,OP_EQ, tor_timegm(&a_time));
    CHECK_TIMEGM_ARG_OUT_OF_RANGE();

    /* while unlikely, the system's gmtime(_r) could return
     * a "correct" proleptic gregorian year value,
     * which I'm pretty sure is:
     * (2^63-1)/60/60/24*2000/730485 + 1970 = 292277026596
     * 730485 is the number of days in two millennia, including leap days */
    a_time.tm_year = 292277026596-1900;
    CAPTURE();
    tt_int_op((time_t) -1,OP_EQ, tor_timegm(&a_time));
    CHECK_TIMEGM_ARG_OUT_OF_RANGE();

    a_time.tm_year = INT64_MAX-1900;
    CAPTURE();
    tt_int_op((time_t) -1,OP_EQ, tor_timegm(&a_time));
    CHECK_TIMEGM_ARG_OUT_OF_RANGE();

    a_time.tm_year = INT64_MAX;
    CAPTURE();
    tt_int_op((time_t) -1,OP_EQ, tor_timegm(&a_time));
    CHECK_TIMEGM_ARG_OUT_OF_RANGE();
#endif /* SIZEOF_INT == 8 */

  /* month */
  a_time.tm_year = 2007-1900;  /* restore valid year */

  a_time.tm_mon = 12;          /* Wrong month, it's 0-based */
  CAPTURE();
  tt_int_op((time_t) -1,OP_EQ, tor_timegm(&a_time));
  CHECK_TIMEGM_ARG_OUT_OF_RANGE();

  a_time.tm_mon = -1;          /* Wrong month */
  CAPTURE();
  tt_int_op((time_t) -1,OP_EQ, tor_timegm(&a_time));
  CHECK_TIMEGM_ARG_OUT_OF_RANGE();

  /* day */
  a_time.tm_mon = 6;            /* Try July */
  a_time.tm_mday = 32;          /* Wrong day */
  CAPTURE();
  tt_int_op((time_t) -1,OP_EQ, tor_timegm(&a_time));
  CHECK_TIMEGM_ARG_OUT_OF_RANGE();

  a_time.tm_mon = 5;            /* Try June */
  a_time.tm_mday = 31;          /* Wrong day */
  CAPTURE();
  tt_int_op((time_t) -1,OP_EQ, tor_timegm(&a_time));
  CHECK_TIMEGM_ARG_OUT_OF_RANGE();

  a_time.tm_year = 2008-1900;   /* Try a leap year */
  a_time.tm_mon = 1;            /* in feb. */
  a_time.tm_mday = 30;          /* Wrong day */
  CAPTURE();
  tt_int_op((time_t) -1,OP_EQ, tor_timegm(&a_time));
  CHECK_TIMEGM_ARG_OUT_OF_RANGE();

  a_time.tm_year = 2011-1900;   /* Try a non-leap year */
  a_time.tm_mon = 1;            /* in feb. */
  a_time.tm_mday = 29;          /* Wrong day */
  CAPTURE();
  tt_int_op((time_t) -1,OP_EQ, tor_timegm(&a_time));
  CHECK_TIMEGM_ARG_OUT_OF_RANGE();

  a_time.tm_mday = 0;           /* Wrong day, it's 1-based (to be different) */
  CAPTURE();
  tt_int_op((time_t) -1,OP_EQ, tor_timegm(&a_time));
  CHECK_TIMEGM_ARG_OUT_OF_RANGE();

  /* hour */
  a_time.tm_mday = 3;           /* restore valid month day */

  a_time.tm_hour = 24;          /* Wrong hour, it's 0-based */
  CAPTURE();
  tt_int_op((time_t) -1,OP_EQ, tor_timegm(&a_time));
  CHECK_TIMEGM_ARG_OUT_OF_RANGE();

  a_time.tm_hour = -1;          /* Wrong hour */
  CAPTURE();
  tt_int_op((time_t) -1,OP_EQ, tor_timegm(&a_time));
  CHECK_TIMEGM_ARG_OUT_OF_RANGE();

  /* minute */
  a_time.tm_hour = 22;          /* restore valid hour */

  a_time.tm_min = 60;           /* Wrong minute, it's 0-based */
  CAPTURE();
  tt_int_op((time_t) -1,OP_EQ, tor_timegm(&a_time));
  CHECK_TIMEGM_ARG_OUT_OF_RANGE();

  a_time.tm_min = -1;           /* Wrong minute */
  CAPTURE();
  tt_int_op((time_t) -1,OP_EQ, tor_timegm(&a_time));
  CHECK_TIMEGM_ARG_OUT_OF_RANGE();

  /* second */
  a_time.tm_min = 37;           /* restore valid minute */

  a_time.tm_sec = 61;           /* Wrong second: 0-based with leap seconds */
  CAPTURE();
  tt_int_op((time_t) -1,OP_EQ, tor_timegm(&a_time));
  CHECK_TIMEGM_ARG_OUT_OF_RANGE();

  a_time.tm_sec = -1;           /* Wrong second */
  CAPTURE();
  tt_int_op((time_t) -1,OP_EQ, tor_timegm(&a_time));
  CHECK_TIMEGM_ARG_OUT_OF_RANGE();

  /* Test tor_gmtime_r out of range */

  /* time_t < 0 yields a year clamped to 1 or 1970,
   * depending on whether the implementation of the system gmtime(_r)
   * sets struct tm (1) or not (1970) */
  t_res = -1;
  CAPTURE();
  tor_gmtime_r(&t_res, &b_time);
  CHECK_POSSIBLE_EINVAL();
  tt_assert(b_time.tm_year == (1970-1900) ||
            b_time.tm_year == (1969-1900));

  if (sizeof(time_t) == 4 || sizeof(time_t) == 8) {
    t_res = -1*(1 << 30);
    CAPTURE();
    tor_gmtime_r(&t_res, &b_time);
    CHECK_POSSIBLE_EINVAL();
    tt_assert(b_time.tm_year == (1970-1900) ||
              b_time.tm_year == (1935-1900));

    t_res = INT32_MIN;
    CAPTURE();
    tor_gmtime_r(&t_res, &b_time);
    CHECK_POSSIBLE_EINVAL();
    tt_assert(b_time.tm_year == (1970-1900) ||
              b_time.tm_year == (1901-1900));
  }

#if SIZEOF_TIME_T == 8
  {
    /* one of the smallest tm_year values my 64 bit system supports:
     * b_time.tm_year == (-292275055LL-1900LL) without clamping */
    t_res = -9223372036854775LL;
    CAPTURE();
    tor_gmtime_r(&t_res, &b_time);
    CHECK_POSSIBLE_EINVAL();
    tt_assert(b_time.tm_year == (1970-1900) ||
              b_time.tm_year == (1-1900));

    /* while unlikely, the system's gmtime(_r) could return
     * a "correct" retrospective gregorian negative year value,
     * which I'm pretty sure is:
     * -1*(2^63)/60/60/24*2000/730485 + 1970 = -292277022657
     * 730485 is the number of days in two millennia, including leap days
     * (int64_t)b_time.tm_year == (-292277022657LL-1900LL) without clamping */
    t_res = INT64_MIN;
    CAPTURE();
    tor_gmtime_r(&t_res, &b_time);
    if (! (b_time.tm_year == (1970-1900) ||
           b_time.tm_year == (1-1900))) {
      tt_int_op(b_time.tm_year, OP_EQ, 1970-1900);
    }
    if (b_time.tm_year != 1970-1900) {
      CHECK_TIMEGM_WARNING("Rounding up to ");
    } else {
      teardown_capture_of_logs();
    }
  }
  {
    /* As above, but with localtime. */
    t_res = -9223372036854775LL;
    CAPTURE();
    tor_localtime_r(&t_res, &b_time);
    CHECK_POSSIBLE_EINVAL();
    tt_assert(b_time.tm_year == (1970-1900) ||
              b_time.tm_year == (1-1900));

    /* while unlikely, the system's gmtime(_r) could return
     * a "correct" retrospective gregorian negative year value,
     * which I'm pretty sure is:
     * -1*(2^63)/60/60/24*2000/730485 + 1970 = -292277022657
     * 730485 is the number of days in two millennia, including leap days
     * (int64_t)b_time.tm_year == (-292277022657LL-1900LL) without clamping */
    t_res = INT64_MIN;
    CAPTURE();
    tor_localtime_r(&t_res, &b_time);
    if (! (b_time.tm_year == (1970-1900) ||
           b_time.tm_year == (1-1900))) {
      tt_int_op(b_time.tm_year, OP_EQ, 1970-1900);
    }
    if (b_time.tm_year != 1970-1900) {
      CHECK_TIMEGM_WARNING("Rounding up to ");
    } else {
      teardown_capture_of_logs();
    }
  }
#endif /* SIZEOF_TIME_T == 8 */

  /* time_t >= INT_MAX yields a year clamped to 2037 or 9999,
   * depending on whether the implementation of the system gmtime(_r)
   * sets struct tm (9999) or not (2037) */
#if SIZEOF_TIME_T == 4 || SIZEOF_TIME_T == 8
  {
    t_res = 3*(1 << 29);
    tor_gmtime_r(&t_res, &b_time);
    tt_assert(b_time.tm_year == (2021-1900));

    t_res = INT32_MAX;
    tor_gmtime_r(&t_res, &b_time);
    tt_assert(b_time.tm_year == (2037-1900) ||
              b_time.tm_year == (2038-1900));
  }
  {
    /* as above but with localtime. */
    t_res = 3*(1 << 29);
    tor_localtime_r(&t_res, &b_time);
    tt_assert(b_time.tm_year == (2021-1900));

    t_res = INT32_MAX;
    tor_localtime_r(&t_res, &b_time);
    tt_assert(b_time.tm_year == (2037-1900) ||
              b_time.tm_year == (2038-1900));
  }
#endif /* SIZEOF_TIME_T == 4 || SIZEOF_TIME_T == 8 */

#if SIZEOF_TIME_T == 8
  {
    /* one of the largest tm_year values my 64 bit system supports:
     * b_time.tm_year == (292278994L-1900L) without clamping */
    t_res = 9223372036854775LL;
    CAPTURE();
    tor_gmtime_r(&t_res, &b_time);
    CHECK_POSSIBLE_EINVAL();
    tt_assert(b_time.tm_year == (2037-1900) ||
              b_time.tm_year == (9999-1900));

    /* while unlikely, the system's gmtime(_r) could return
     * a "correct" proleptic gregorian year value,
     * which I'm pretty sure is:
     * (2^63-1)/60/60/24*2000/730485 + 1970 = 292277026596
     * 730485 is the number of days in two millennia, including leap days
     * (int64_t)b_time.tm_year == (292277026596L-1900L) without clamping */
    t_res = INT64_MAX;
    CAPTURE();
    tor_gmtime_r(&t_res, &b_time);
    CHECK_TIMEGM_WARNING("Rounding down to ");

    tt_assert(b_time.tm_year == (2037-1900) ||
              b_time.tm_year == (9999-1900));
  }
  {
    /* As above but with localtime. */
    t_res = 9223372036854775LL;
    CAPTURE();
    tor_localtime_r(&t_res, &b_time);
    CHECK_POSSIBLE_EINVAL();
    tt_assert(b_time.tm_year == (2037-1900) ||
              b_time.tm_year == (9999-1900));

    /* while unlikely, the system's gmtime(_r) could return
     * a "correct" proleptic gregorian year value,
     * which I'm pretty sure is:
     * (2^63-1)/60/60/24*2000/730485 + 1970 = 292277026596
     * 730485 is the number of days in two millennia, including leap days
     * (int64_t)b_time.tm_year == (292277026596L-1900L) without clamping */
    t_res = INT64_MAX;
    CAPTURE();
    tor_localtime_r(&t_res, &b_time);
    CHECK_TIMEGM_WARNING("Rounding down to ");

    tt_assert(b_time.tm_year == (2037-1900) ||
              b_time.tm_year == (9999-1900));
  }
#endif /* SIZEOF_TIME_T == 8 */

  /* Test {format,parse}_rfc1123_time */

  format_rfc1123_time(timestr, 0);
  tt_str_op("Thu, 01 Jan 1970 00:00:00 GMT",OP_EQ, timestr);
  format_rfc1123_time(timestr, (time_t)1091580502UL);
  tt_str_op("Wed, 04 Aug 2004 00:48:22 GMT",OP_EQ, timestr);

  t_res = 0;
  i = parse_rfc1123_time(timestr, &t_res);
  tt_int_op(0,OP_EQ, i);
  tt_int_op(t_res,OP_EQ, (time_t)1091580502UL);

  /* This value is in range with 32 bit and 64 bit time_t */
  format_rfc1123_time(timestr, (time_t)2080000000UL);
  tt_str_op("Fri, 30 Nov 2035 01:46:40 GMT",OP_EQ, timestr);

  t_res = 0;
  i = parse_rfc1123_time(timestr, &t_res);
  tt_int_op(0,OP_EQ, i);
  tt_int_op(t_res,OP_EQ, (time_t)2080000000UL);

  /* This value is out of range with 32 bit time_t, but in range for 64 bit
   * time_t */
  CAPTURE();
  format_rfc1123_time(timestr, (time_t)2150000000UL);
  CHECK_POSSIBLE_EINVAL();

#if SIZEOF_TIME_T == 4
#if 0
  /* Wrapping around will have made it this. */
  /* On windows, at least, this is clipped to 1 Jan 1970. ??? */
  tt_str_op("Sat, 11 Jan 1902 23:45:04 GMT",OP_EQ, timestr);
#endif
  /* Make sure that the right date doesn't parse. */
  strlcpy(timestr, "Wed, 17 Feb 2038 06:13:20 GMT", sizeof(timestr));

  t_res = 0;
  CAPTURE();
  i = parse_rfc1123_time(timestr, &t_res);
  // CHECK_TIMEGM_WARNING("does not fit in tor_timegm");
  tt_int_op(-1,OP_EQ, i);
#elif SIZEOF_TIME_T == 8
  tt_str_op("Wed, 17 Feb 2038 06:13:20 GMT",OP_EQ, timestr);

  t_res = 0;
  i = parse_rfc1123_time(timestr, &t_res);
  tt_int_op(0,OP_EQ, i);
  tt_int_op(t_res,OP_EQ, (time_t)2150000000UL);
#endif /* SIZEOF_TIME_T == 4 || ... */

  /* The timezone doesn't matter */
  t_res = 0;
  tt_int_op(0,OP_EQ,
            parse_rfc1123_time("Wed, 04 Aug 2004 00:48:22 ZUL", &t_res));
  tt_int_op(t_res,OP_EQ, (time_t)1091580502UL);
  tt_int_op(-1,OP_EQ,
            parse_rfc1123_time("Wed, zz Aug 2004 99-99x99 GMT", &t_res));
  tt_int_op(-1,OP_EQ,
            parse_rfc1123_time("Wed, 32 Mar 2011 00:00:00 GMT", &t_res));
  tt_int_op(-1,OP_EQ,
            parse_rfc1123_time("Wed, 30 Mar 2011 24:00:00 GMT", &t_res));
  tt_int_op(-1,OP_EQ,
            parse_rfc1123_time("Wed, 30 Mar 2011 23:60:00 GMT", &t_res));
  tt_int_op(-1,OP_EQ,
            parse_rfc1123_time("Wed, 30 Mar 2011 23:59:62 GMT", &t_res));
  tt_int_op(-1,OP_EQ,
            parse_rfc1123_time("Wed, 30 Mar 1969 23:59:59 GMT", &t_res));
  tt_int_op(-1,OP_EQ,
            parse_rfc1123_time("Wed, 30 Ene 2011 23:59:59 GMT", &t_res));
  tt_int_op(-1,OP_EQ,
            parse_rfc1123_time("Wed, 30 Mar 2011 23:59:59 GM", &t_res));
  tt_int_op(-1,OP_EQ,
            parse_rfc1123_time("Wed, 30 Mar 1900 23:59:59 GMT", &t_res));

  /* Leap year. */
  tt_int_op(-1,OP_EQ,
            parse_rfc1123_time("Wed, 29 Feb 2011 16:00:00 GMT", &t_res));
  tt_int_op(0,OP_EQ,
            parse_rfc1123_time("Wed, 29 Feb 2012 16:00:00 GMT", &t_res));

  /* Leap second plus one */
  tt_int_op(-1,OP_EQ,
            parse_rfc1123_time("Wed, 30 Mar 2011 23:59:61 GMT", &t_res));

  /* Test parse_iso_time */

  t_res = 0;
  i = parse_iso_time("", &t_res);
  tt_int_op(-1,OP_EQ, i);
  t_res = 0;
  i = parse_iso_time("2004-08-32 00:48:22", &t_res);
  tt_int_op(-1,OP_EQ, i);
  t_res = 0;
  i = parse_iso_time("1969-08-03 00:48:22", &t_res);
  tt_int_op(-1,OP_EQ, i);

  t_res = 0;
  i = parse_iso_time("2004-08-04 00:48:22", &t_res);
  tt_int_op(0,OP_EQ, i);
  tt_int_op(t_res,OP_EQ, (time_t)1091580502UL);
  t_res = 0;
  i = parse_iso_time("2004-8-4 0:48:22", &t_res);
  tt_int_op(0,OP_EQ, i);
  tt_int_op(t_res,OP_EQ, (time_t)1091580502UL);

  /* This value is in range with 32 bit and 64 bit time_t */
  t_res = 0;
  i = parse_iso_time("2035-11-30 01:46:40", &t_res);
  tt_int_op(0,OP_EQ, i);
  tt_int_op(t_res,OP_EQ, (time_t)2080000000UL);

  /* This value is out of range with 32 bit time_t, but in range for 64 bit
   * time_t */
  t_res = 0;
#if SIZEOF_TIME_T == 4
  CAPTURE();
  i = parse_iso_time("2038-02-17 06:13:20", &t_res);
  tt_int_op(-1,OP_EQ, i);
  //CHECK_TIMEGM_WARNING("does not fit in tor_timegm");
#elif SIZEOF_TIME_T == 8
  i = parse_iso_time("2038-02-17 06:13:20", &t_res);
  tt_int_op(0,OP_EQ, i);
  tt_int_op(t_res,OP_EQ, (time_t)2150000000UL);
#endif /* SIZEOF_TIME_T == 4 || ... */

  tt_int_op(-1,OP_EQ, parse_iso_time("2004-08-zz 99-99x99", &t_res));
  tt_int_op(-1,OP_EQ, parse_iso_time("2011-03-32 00:00:00", &t_res));
  tt_int_op(-1,OP_EQ, parse_iso_time("2011-03-30 24:00:00", &t_res));
  tt_int_op(-1,OP_EQ, parse_iso_time("2011-03-30 23:60:00", &t_res));
  tt_int_op(-1,OP_EQ, parse_iso_time("2011-03-30 23:59:62", &t_res));
  tt_int_op(-1,OP_EQ, parse_iso_time("1969-03-30 23:59:59", &t_res));
  tt_int_op(-1,OP_EQ, parse_iso_time("2011-00-30 23:59:59", &t_res));
  tt_int_op(-1,OP_EQ, parse_iso_time("2147483647-08-29 14:00:00", &t_res));
  tt_int_op(-1,OP_EQ, parse_iso_time("2011-03-30 23:59", &t_res));
  tt_int_op(-1,OP_EQ, parse_iso_time("2004-08-04 00:48:22.100", &t_res));
  tt_int_op(-1,OP_EQ, parse_iso_time("2004-08-04 00:48:22XYZ", &t_res));

  /* but... that _is_ acceptable if we aren't being strict. */
  t_res = 0;
  i = parse_iso_time_("2004-08-04 00:48:22XYZ", &t_res, 0, 0);
  tt_int_op(0,OP_EQ, i);
  tt_int_op(t_res,OP_EQ, (time_t)1091580502UL);

  /* try nospace variant. */
  t_res = 0;
  i = parse_iso_time_nospace("2004-08-04T00:48:22", &t_res);
  tt_int_op(0,OP_EQ, i);
  tt_int_op(t_res,OP_EQ, (time_t)1091580502UL);

  tt_int_op(-1,OP_EQ, parse_iso_time("2004-08-04T00:48:22", &t_res));
  tt_int_op(-1,OP_EQ, parse_iso_time_nospace("2004-08-04 00:48:22", &t_res));
  tt_int_op(-1,OP_EQ, parse_iso_time("2004-08-04x00:48:22", &t_res));
  tt_int_op(-1,OP_EQ, parse_iso_time_nospace("2004-08-04x00:48:22", &t_res));

  /* Test tor_gettimeofday */

  end.tv_sec = 4;
  end.tv_usec = 999990;
  start.tv_sec = 1;
  start.tv_usec = 500;

  tor_gettimeofday(&start);
  /* now make sure time works. */
  tor_gettimeofday(&end);
  /* We might've timewarped a little. */
  tt_int_op(tv_udiff(&start, &end), OP_GE, -5000);

  /* Test format_iso_time */

  tv.tv_sec = (time_t)1326296338UL;
  tv.tv_usec = 3060;
  format_iso_time(timestr, (time_t)tv.tv_sec);
  tt_str_op("2012-01-11 15:38:58",OP_EQ, timestr);
  /* The output of format_local_iso_time will vary by timezone, and setting
     our timezone for testing purposes would be a nontrivial flaky pain.
     Skip this test for now.
  format_local_iso_time(timestr, tv.tv_sec);
  test_streq("2012-01-11 10:38:58", timestr);
  */
  format_iso_time_nospace(timestr, (time_t)tv.tv_sec);
  tt_str_op("2012-01-11T15:38:58",OP_EQ, timestr);
  tt_int_op(strlen(timestr),OP_EQ, ISO_TIME_LEN);
  format_iso_time_nospace_usec(timestr, &tv);
  tt_str_op("2012-01-11T15:38:58.003060",OP_EQ, timestr);
  tt_int_op(strlen(timestr),OP_EQ, ISO_TIME_USEC_LEN);

  tv.tv_usec = 0;
  /* This value is in range with 32 bit and 64 bit time_t */
  tv.tv_sec = (time_t)2080000000UL;
  format_iso_time(timestr, (time_t)tv.tv_sec);
  tt_str_op("2035-11-30 01:46:40",OP_EQ, timestr);

  /* This value is out of range with 32 bit time_t, but in range for 64 bit
   * time_t */
  tv.tv_sec = (time_t)2150000000UL;
  CAPTURE();
  format_iso_time(timestr, (time_t)tv.tv_sec);
  CHECK_POSSIBLE_EINVAL();
#if SIZEOF_TIME_T == 4
  /* format_iso_time should indicate failure on overflow, but it doesn't yet.
   * Hopefully #18480 will improve the failure semantics in this case.
   tt_str_op("2038-02-17 06:13:20",OP_EQ, timestr);
   */
#elif SIZEOF_TIME_T == 8
#ifndef _WIN32
  /* This SHOULD work on windows too; see bug #18665 */
  tt_str_op("2038-02-17 06:13:20",OP_EQ, timestr);
#endif
#endif /* SIZEOF_TIME_T == 4 || ... */

#undef CAPTURE
#undef CHECK_TIMEGM_ARG_OUT_OF_RANGE
#undef CHECK_POSSIBLE_EINVAL

 done:
  teardown_capture_of_logs();
}

static void
test_util_parse_http_time(void *arg)
{
  struct tm a_time;
  char b[ISO_TIME_LEN+1];
  (void)arg;

#define T(s) do {                               \
    format_iso_time(b, tor_timegm(&a_time));    \
    tt_str_op(b, OP_EQ, (s));                      \
    b[0]='\0';                                  \
  } while (0)

  /* Test parse_http_time */

  tt_int_op(-1,OP_EQ,
            parse_http_time("", &a_time));
  tt_int_op(-1,OP_EQ,
            parse_http_time("Sunday, 32 Aug 2004 00:48:22 GMT", &a_time));
  tt_int_op(-1,OP_EQ,
            parse_http_time("Sunday, 3 Aug 1869 00:48:22 GMT", &a_time));
  tt_int_op(-1,OP_EQ,
            parse_http_time("Sunday, 32-Aug-94 00:48:22 GMT", &a_time));
  tt_int_op(-1,OP_EQ,
            parse_http_time("Sunday, 3-Ago-04 00:48:22", &a_time));
  tt_int_op(-1,OP_EQ,
            parse_http_time("Sunday, August the third", &a_time));
  tt_int_op(-1,OP_EQ,
            parse_http_time("Wednesday,,04 Aug 1994 00:48:22 GMT", &a_time));

  tt_int_op(0,OP_EQ,
            parse_http_time("Wednesday, 04 Aug 1994 00:48:22 GMT", &a_time));
  tt_int_op((time_t)775961302UL,OP_EQ, tor_timegm(&a_time));
  T("1994-08-04 00:48:22");
  tt_int_op(0,OP_EQ,
            parse_http_time("Wednesday, 4 Aug 1994 0:48:22 GMT", &a_time));
  tt_int_op((time_t)775961302UL,OP_EQ, tor_timegm(&a_time));
  T("1994-08-04 00:48:22");
  tt_int_op(0,OP_EQ,
            parse_http_time("Miercoles, 4 Aug 1994 0:48:22 GMT", &a_time));
  tt_int_op((time_t)775961302UL,OP_EQ, tor_timegm(&a_time));
  T("1994-08-04 00:48:22");
  tt_int_op(0,OP_EQ,
            parse_http_time("Wednesday, 04-Aug-94 00:48:22 GMT", &a_time));
  tt_int_op((time_t)775961302UL,OP_EQ, tor_timegm(&a_time));
  T("1994-08-04 00:48:22");
  tt_int_op(0,OP_EQ,
            parse_http_time("Wednesday, 4-Aug-94 0:48:22 GMT", &a_time));
  tt_int_op((time_t)775961302UL,OP_EQ, tor_timegm(&a_time));
  T("1994-08-04 00:48:22");
  tt_int_op(0,OP_EQ,
            parse_http_time("Miercoles, 4-Aug-94 0:48:22 GMT", &a_time));
  tt_int_op((time_t)775961302UL,OP_EQ, tor_timegm(&a_time));
  T("1994-08-04 00:48:22");
  tt_int_op(0,OP_EQ, parse_http_time("Wed Aug 04 00:48:22 1994", &a_time));
  tt_int_op((time_t)775961302UL,OP_EQ, tor_timegm(&a_time));
  T("1994-08-04 00:48:22");
  tt_int_op(0,OP_EQ, parse_http_time("Wed Aug 4 0:48:22 1994", &a_time));
  tt_int_op((time_t)775961302UL,OP_EQ, tor_timegm(&a_time));
  T("1994-08-04 00:48:22");
  tt_int_op(0,OP_EQ, parse_http_time("Mie Aug 4 0:48:22 1994", &a_time));
  tt_int_op((time_t)775961302UL,OP_EQ, tor_timegm(&a_time));
  T("1994-08-04 00:48:22");
  tt_int_op(0,OP_EQ,parse_http_time("Sun, 1 Jan 2012 00:00:00 GMT", &a_time));
  tt_int_op((time_t)1325376000UL,OP_EQ, tor_timegm(&a_time));
  T("2012-01-01 00:00:00");
  tt_int_op(0,OP_EQ,parse_http_time("Mon, 31 Dec 2012 00:00:00 GMT", &a_time));
  tt_int_op((time_t)1356912000UL,OP_EQ, tor_timegm(&a_time));
  T("2012-12-31 00:00:00");

  /* This value is in range with 32 bit and 64 bit time_t */
  tt_int_op(0,OP_EQ,parse_http_time("Fri, 30 Nov 2035 01:46:40 GMT", &a_time));
  tt_int_op((time_t)2080000000UL,OP_EQ, tor_timegm(&a_time));
  T("2035-11-30 01:46:40");

  /* This value is out of range with 32 bit time_t, but in range for 64 bit
   * time_t */
#if SIZEOF_TIME_T == 4
  /* parse_http_time should indicate failure on overflow, but it doesn't yet.
   * Hopefully #18480 will improve the failure semantics in this case. */
  setup_full_capture_of_logs(LOG_WARN);
  tt_int_op(0,OP_EQ,parse_http_time("Wed, 17 Feb 2038 06:13:20 GMT", &a_time));
  tt_int_op((time_t)-1,OP_EQ, tor_timegm(&a_time));
  //expect_single_log_msg_containing("does not fit in tor_timegm");
  teardown_capture_of_logs();
#elif SIZEOF_TIME_T == 8
  tt_int_op(0,OP_EQ,parse_http_time("Wed, 17 Feb 2038 06:13:20 GMT", &a_time));
  tt_int_op((time_t)2150000000UL,OP_EQ, tor_timegm(&a_time));
  T("2038-02-17 06:13:20");
#endif /* SIZEOF_TIME_T == 4 || ... */

  tt_int_op(-1,OP_EQ, parse_http_time("2004-08-zz 99-99x99 GMT", &a_time));
  tt_int_op(-1,OP_EQ, parse_http_time("2011-03-32 00:00:00 GMT", &a_time));
  tt_int_op(-1,OP_EQ, parse_http_time("2011-03-30 24:00:00 GMT", &a_time));
  tt_int_op(-1,OP_EQ, parse_http_time("2011-03-30 23:60:00 GMT", &a_time));
  tt_int_op(-1,OP_EQ, parse_http_time("2011-03-30 23:59:62 GMT", &a_time));
  tt_int_op(-1,OP_EQ, parse_http_time("1969-03-30 23:59:59 GMT", &a_time));
  tt_int_op(-1,OP_EQ, parse_http_time("2011-00-30 23:59:59 GMT", &a_time));
  tt_int_op(-1,OP_EQ, parse_http_time("2011-03-30 23:59", &a_time));

#undef T
 done:
  teardown_capture_of_logs();
}

static void
test_util_timegm_real(void *arg)
{
  (void)arg;
  /* Get the real timegm again!  We're not testing our impl; we want the
   * one that will actually get called. */
#undef tor_timegm

  /* Now check: is timegm the real inverse of gmtime? */
  time_t now = time(NULL), time2=0;
  struct tm tm, *p;
  p = tor_gmtime_r(&now, &tm);
  tt_ptr_op(p, OP_NE, NULL);

  int r = tor_timegm(&tm, &time2);
  tt_int_op(r, OP_EQ, 0);
  tt_i64_op((int64_t) now, OP_EQ, (int64_t) time2);

 done:
  ;
}

static void
test_util_config_line(void *arg)
{
  char buf[1024];
  char *k=NULL, *v=NULL;
  const char *str;

  /* Test parse_config_line_from_str */
  (void)arg;
  strlcpy(buf, "k v\n" " key    value with spaces   \n" "keykey val\n"
          "k2\n"
          "k3 \n" "\n" "   \n" "#comment\n"
          "k4#a\n" "k5#abc\n" "k6 val #with comment\n"
          "kseven   \"a quoted 'string\"\n"
          "k8 \"a \\x71uoted\\n\\\"str\\\\ing\\t\\001\\01\\1\\\"\"\n"
          "k9 a line that\\\n spans two lines.\n\n"
          "k10 more than\\\n one contin\\\nuation\n"
          "k11  \\\ncontinuation at the start\n"
          "k12 line with a\\\n#comment\n embedded\n"
          "k13\\\ncontinuation at the very start\n"
          "k14 a line that has a comment and # ends with a slash \\\n"
          "k15 this should be the next new line\n"
          "k16 a line that has a comment and # ends without a slash \n"
          "k17 this should be the next new line\n"
          , sizeof(buf));
  str = buf;

  str = parse_config_line_from_str_verbose(str, &k, &v, NULL);
  tt_str_op(k,OP_EQ, "k");
  tt_str_op(v,OP_EQ, "v");
  tor_free(k); tor_free(v);
  tt_assert(!strcmpstart(str, "key    value with"));

  str = parse_config_line_from_str_verbose(str, &k, &v, NULL);
  tt_str_op(k,OP_EQ, "key");
  tt_str_op(v,OP_EQ, "value with spaces");
  tor_free(k); tor_free(v);
  tt_assert(!strcmpstart(str, "keykey"));

  str = parse_config_line_from_str_verbose(str, &k, &v, NULL);
  tt_str_op(k,OP_EQ, "keykey");
  tt_str_op(v,OP_EQ, "val");
  tor_free(k); tor_free(v);
  tt_assert(!strcmpstart(str, "k2\n"));

  str = parse_config_line_from_str_verbose(str, &k, &v, NULL);
  tt_str_op(k,OP_EQ, "k2");
  tt_str_op(v,OP_EQ, "");
  tor_free(k); tor_free(v);
  tt_assert(!strcmpstart(str, "k3 \n"));

  str = parse_config_line_from_str_verbose(str, &k, &v, NULL);
  tt_str_op(k,OP_EQ, "k3");
  tt_str_op(v,OP_EQ, "");
  tor_free(k); tor_free(v);
  tt_assert(!strcmpstart(str, "#comment"));

  str = parse_config_line_from_str_verbose(str, &k, &v, NULL);
  tt_str_op(k,OP_EQ, "k4");
  tt_str_op(v,OP_EQ, "");
  tor_free(k); tor_free(v);
  tt_assert(!strcmpstart(str, "k5#abc"));

  str = parse_config_line_from_str_verbose(str, &k, &v, NULL);
  tt_str_op(k,OP_EQ, "k5");
  tt_str_op(v,OP_EQ, "");
  tor_free(k); tor_free(v);
  tt_assert(!strcmpstart(str, "k6"));

  str = parse_config_line_from_str_verbose(str, &k, &v, NULL);
  tt_str_op(k,OP_EQ, "k6");
  tt_str_op(v,OP_EQ, "val");
  tor_free(k); tor_free(v);
  tt_assert(!strcmpstart(str, "kseven"));

  str = parse_config_line_from_str_verbose(str, &k, &v, NULL);
  tt_str_op(k,OP_EQ, "kseven");
  tt_str_op(v,OP_EQ, "a quoted \'string");
  tor_free(k); tor_free(v);
  tt_assert(!strcmpstart(str, "k8 "));

  str = parse_config_line_from_str_verbose(str, &k, &v, NULL);
  tt_str_op(k,OP_EQ, "k8");
  tt_str_op(v,OP_EQ, "a quoted\n\"str\\ing\t\x01\x01\x01\"");
  tor_free(k); tor_free(v);

  str = parse_config_line_from_str_verbose(str, &k, &v, NULL);
  tt_str_op(k,OP_EQ, "k9");
  tt_str_op(v,OP_EQ, "a line that spans two lines.");
  tor_free(k); tor_free(v);

  str = parse_config_line_from_str_verbose(str, &k, &v, NULL);
  tt_str_op(k,OP_EQ, "k10");
  tt_str_op(v,OP_EQ, "more than one continuation");
  tor_free(k); tor_free(v);

  str = parse_config_line_from_str_verbose(str, &k, &v, NULL);
  tt_str_op(k,OP_EQ, "k11");
  tt_str_op(v,OP_EQ, "continuation at the start");
  tor_free(k); tor_free(v);

  str = parse_config_line_from_str_verbose(str, &k, &v, NULL);
  tt_str_op(k,OP_EQ, "k12");
  tt_str_op(v,OP_EQ, "line with a embedded");
  tor_free(k); tor_free(v);

  str = parse_config_line_from_str_verbose(str, &k, &v, NULL);
  tt_str_op(k,OP_EQ, "k13");
  tt_str_op(v,OP_EQ, "continuation at the very start");
  tor_free(k); tor_free(v);

  str = parse_config_line_from_str_verbose(str, &k, &v, NULL);
  tt_str_op(k,OP_EQ, "k14");
  tt_str_op(v,OP_EQ, "a line that has a comment and" );
  tor_free(k); tor_free(v);

  str = parse_config_line_from_str_verbose(str, &k, &v, NULL);
  tt_str_op(k,OP_EQ, "k15");
  tt_str_op(v,OP_EQ, "this should be the next new line");
  tor_free(k); tor_free(v);

  str = parse_config_line_from_str_verbose(str, &k, &v, NULL);
  tt_str_op(k,OP_EQ, "k16");
  tt_str_op(v,OP_EQ, "a line that has a comment and" );
  tor_free(k); tor_free(v);

  str = parse_config_line_from_str_verbose(str, &k, &v, NULL);
  tt_str_op(k,OP_EQ, "k17");
  tt_str_op(v,OP_EQ, "this should be the next new line");
  tor_free(k); tor_free(v);

  tt_str_op(str,OP_EQ, "");

 done:
  tor_free(k);
  tor_free(v);
}

static void
test_util_config_line_quotes(void *arg)
{
  char buf1[1024];
  char buf2[128];
  char buf3[128];
  char buf4[128];
  char *k=NULL, *v=NULL;
  const char *str;

  /* Test parse_config_line_from_str */
  (void)arg;
  strlcpy(buf1, "kTrailingSpace \"quoted value\"   \n"
          "kTrailingGarbage \"quoted value\"trailing garbage\n"
          , sizeof(buf1));
  strlcpy(buf2, "kTrailingSpaceAndGarbage \"quoted value\" trailing space+g\n"
          , sizeof(buf2));
  strlcpy(buf3, "kMultilineTrailingSpace \"mline\\ \nvalue w/ trailing sp\"\n"
          , sizeof(buf3));
  strlcpy(buf4, "kMultilineNoTrailingBackslash \"naked multiline\nvalue\"\n"
          , sizeof(buf4));
  str = buf1;

  str = parse_config_line_from_str_verbose(str, &k, &v, NULL);
  tt_str_op(k,OP_EQ, "kTrailingSpace");
  tt_str_op(v,OP_EQ, "quoted value");
  tor_free(k); tor_free(v);

  str = parse_config_line_from_str_verbose(str, &k, &v, NULL);
  tt_ptr_op(str,OP_EQ, NULL);
  tor_free(k); tor_free(v);

  str = buf2;

  str = parse_config_line_from_str_verbose(str, &k, &v, NULL);
  tt_ptr_op(str,OP_EQ, NULL);
  tor_free(k); tor_free(v);

  str = buf3;

  const char *err = NULL;
  str = parse_config_line_from_str_verbose(str, &k, &v, &err);
  tt_ptr_op(str,OP_EQ, NULL);
  tor_free(k); tor_free(v);
  tt_str_op(err, OP_EQ, "Invalid escape sequence in quoted string");

  str = buf4;

  err = NULL;
  str = parse_config_line_from_str_verbose(str, &k, &v, &err);
  tt_ptr_op(str,OP_EQ, NULL);
  tor_free(k); tor_free(v);
  tt_str_op(err, OP_EQ, "Invalid escape sequence in quoted string");

 done:
  tor_free(k);
  tor_free(v);
}

static void
test_util_config_line_comment_character(void *arg)
{
  char buf[1024];
  char *k=NULL, *v=NULL;
  const char *str;

  /* Test parse_config_line_from_str */
  (void)arg;
  strlcpy(buf, "k1 \"# in quotes\"\n"
          "k2 some value    # some comment\n"
          "k3 /home/user/myTorNetwork#2\n"    /* Testcase for #1323 */
          , sizeof(buf));
  str = buf;

  str = parse_config_line_from_str_verbose(str, &k, &v, NULL);
  tt_str_op(k,OP_EQ, "k1");
  tt_str_op(v,OP_EQ, "# in quotes");
  tor_free(k); tor_free(v);

  str = parse_config_line_from_str_verbose(str, &k, &v, NULL);
  tt_str_op(k,OP_EQ, "k2");
  tt_str_op(v,OP_EQ, "some value");
  tor_free(k); tor_free(v);

  tt_str_op(str,OP_EQ, "k3 /home/user/myTorNetwork#2\n");

#if 0
  str = parse_config_line_from_str_verbose(str, &k, &v, NULL);
  test_streq(k, "k3");
  test_streq(v, "/home/user/myTorNetwork#2");
  tor_free(k); tor_free(v);

  test_streq(str, "");
#endif /* 0 */

 done:
  tor_free(k);
  tor_free(v);
}

static void
test_util_config_line_escaped_content(void *arg)
{
  char buf1[1024];
  char buf2[128];
  char buf3[128];
  char buf4[128];
  char buf5[128];
  char buf6[128];
  char *k=NULL, *v=NULL;
  const char *str;

  /* Test parse_config_line_from_str */
  (void)arg;
  strlcpy(buf1, "HexadecimalLower \"\\x2a\"\n"
          "HexadecimalUpper \"\\x2A\"\n"
          "HexadecimalUpperX \"\\X2A\"\n"
          "Octal \"\\52\"\n"
          "Newline \"\\n\"\n"
          "Tab \"\\t\"\n"
          "CarriageReturn \"\\r\"\n"
          "DoubleQuote \"\\\"\"\n"
          "SimpleQuote \"\\'\"\n"
          "Backslash \"\\\\\"\n"
          "Mix \"This is a \\\"star\\\":\\t\\'\\x2a\\'\\nAnd second line\"\n"
          , sizeof(buf1));

  strlcpy(buf2, "BrokenEscapedContent \"\\a\"\n"
          , sizeof(buf2));

  strlcpy(buf3, "BrokenEscapedContent \"\\x\"\n"
          , sizeof(buf3));

  strlcpy(buf4, "BrokenOctal \"\\8\"\n"
          , sizeof(buf4));

  strlcpy(buf5, "BrokenHex \"\\xg4\"\n"
          , sizeof(buf5));

  strlcpy(buf6, "BrokenEscape \"\\"
          , sizeof(buf6));

  str = buf1;

  str = parse_config_line_from_str_verbose(str, &k, &v, NULL);
  tt_str_op(k,OP_EQ, "HexadecimalLower");
  tt_str_op(v,OP_EQ, "*");
  tor_free(k); tor_free(v);

  str = parse_config_line_from_str_verbose(str, &k, &v, NULL);
  tt_str_op(k,OP_EQ, "HexadecimalUpper");
  tt_str_op(v,OP_EQ, "*");
  tor_free(k); tor_free(v);

  str = parse_config_line_from_str_verbose(str, &k, &v, NULL);
  tt_str_op(k,OP_EQ, "HexadecimalUpperX");
  tt_str_op(v,OP_EQ, "*");
  tor_free(k); tor_free(v);

  str = parse_config_line_from_str_verbose(str, &k, &v, NULL);
  tt_str_op(k,OP_EQ, "Octal");
  tt_str_op(v,OP_EQ, "*");
  tor_free(k); tor_free(v);

  str = parse_config_line_from_str_verbose(str, &k, &v, NULL);
  tt_str_op(k,OP_EQ, "Newline");
  tt_str_op(v,OP_EQ, "\n");
  tor_free(k); tor_free(v);

  str = parse_config_line_from_str_verbose(str, &k, &v, NULL);
  tt_str_op(k,OP_EQ, "Tab");
  tt_str_op(v,OP_EQ, "\t");
  tor_free(k); tor_free(v);

  str = parse_config_line_from_str_verbose(str, &k, &v, NULL);
  tt_str_op(k,OP_EQ, "CarriageReturn");
  tt_str_op(v,OP_EQ, "\r");
  tor_free(k); tor_free(v);

  str = parse_config_line_from_str_verbose(str, &k, &v, NULL);
  tt_str_op(k,OP_EQ, "DoubleQuote");
  tt_str_op(v,OP_EQ, "\"");
  tor_free(k); tor_free(v);

  str = parse_config_line_from_str_verbose(str, &k, &v, NULL);
  tt_str_op(k,OP_EQ, "SimpleQuote");
  tt_str_op(v,OP_EQ, "'");
  tor_free(k); tor_free(v);

  str = parse_config_line_from_str_verbose(str, &k, &v, NULL);
  tt_str_op(k,OP_EQ, "Backslash");
  tt_str_op(v,OP_EQ, "\\");
  tor_free(k); tor_free(v);

  str = parse_config_line_from_str_verbose(str, &k, &v, NULL);
  tt_str_op(k,OP_EQ, "Mix");
  tt_str_op(v,OP_EQ, "This is a \"star\":\t'*'\nAnd second line");
  tor_free(k); tor_free(v);
  tt_str_op(str,OP_EQ, "");

  str = buf2;

  str = parse_config_line_from_str_verbose(str, &k, &v, NULL);
  tt_ptr_op(str,OP_EQ, NULL);
  tor_free(k); tor_free(v);

  str = buf3;

  str = parse_config_line_from_str_verbose(str, &k, &v, NULL);
  tt_ptr_op(str,OP_EQ, NULL);
  tor_free(k); tor_free(v);

  str = buf4;

  str = parse_config_line_from_str_verbose(str, &k, &v, NULL);
  tt_ptr_op(str,OP_EQ, NULL);
  tor_free(k); tor_free(v);

#if 0
  str = buf5;

  str = parse_config_line_from_str_verbose(str, &k, &v, NULL);
  tt_ptr_op(str, OP_EQ, NULL);
  tor_free(k); tor_free(v);
#endif /* 0 */

  str = buf6;

  str = parse_config_line_from_str_verbose(str, &k, &v, NULL);
  tt_ptr_op(str,OP_EQ, NULL);
  tor_free(k); tor_free(v);

  /* more things to try. */
  /* Bad hex: */
  strlcpy(buf1, "Foo \"\\x9g\"\n", sizeof(buf1));
  strlcpy(buf2, "Foo \"\\xg0\"\n", sizeof(buf2));
  strlcpy(buf3, "Foo \"\\xf\"\n", sizeof(buf3));
  /* bad escape */
  strlcpy(buf4, "Foo \"\\q\"\n", sizeof(buf4));
  /* missing endquote */
  strlcpy(buf5, "Foo \"hello\n", sizeof(buf5));
  /* extra stuff */
  strlcpy(buf6, "Foo \"hello\" world\n", sizeof(buf6));

  str=buf1;
  str = parse_config_line_from_str_verbose(str, &k, &v, NULL);
  tt_ptr_op(str,OP_EQ, NULL);
  tor_free(k); tor_free(v);

  str=buf2;
  str = parse_config_line_from_str_verbose(str, &k, &v, NULL);
  tt_ptr_op(str,OP_EQ, NULL);
  tor_free(k); tor_free(v);

  str=buf3;
  str = parse_config_line_from_str_verbose(str, &k, &v, NULL);
  tt_ptr_op(str,OP_EQ, NULL);
  tor_free(k); tor_free(v);

  str=buf4;
  str = parse_config_line_from_str_verbose(str, &k, &v, NULL);
  tt_ptr_op(str,OP_EQ, NULL);
  tor_free(k); tor_free(v);

  str=buf5;

  str = parse_config_line_from_str_verbose(str, &k, &v, NULL);
  tt_ptr_op(str,OP_EQ, NULL);
  tor_free(k); tor_free(v);

  str=buf6;
  const char *err = NULL;
  str = parse_config_line_from_str_verbose(str, &k, &v, &err);
  tt_ptr_op(str,OP_EQ, NULL);
  tor_free(k); tor_free(v);
  tt_str_op(err,OP_EQ, "Excess data after quoted string");

 done:
  tor_free(k);
  tor_free(v);
}

static void
test_util_config_line_crlf(void *arg)
{
  char *k=NULL, *v=NULL;
  const char *err = NULL;
  (void)arg;
  const char *str =
    "Hello world\r\n"
    "Hello \"nice big world\"\r\n";

  str = parse_config_line_from_str_verbose(str, &k, &v, &err);
  tt_assert(str);
  tt_str_op(k,OP_EQ,"Hello");
  tt_str_op(v,OP_EQ,"world");
  tt_ptr_op(err, OP_EQ, NULL);
  tor_free(k); tor_free(v);

  str = parse_config_line_from_str_verbose(str, &k, &v, &err);
  tt_assert(str);
  tt_str_op(k,OP_EQ,"Hello");
  tt_str_op(v,OP_EQ,"nice big world");
  tt_ptr_op(err, OP_EQ, NULL);
  tor_free(k); tor_free(v);
  tt_str_op(str,OP_EQ, "");

 done:
  tor_free(k); tor_free(v);
}

static void
test_util_config_line_partition(void *arg)
{
  (void)arg;
  config_line_t *lines = NULL, *orig, *rest = NULL;

  config_line_append(&lines, "Header", "X");
  config_line_append(&lines, "Item", "Y");
  config_line_append(&lines, "Thing", "Z");

  config_line_append(&lines, "HEADER", "X2");

  config_line_append(&lines, "header", "X3");
  config_line_append(&lines, "Item3", "Foob");

  /* set up h2 and h3 to point to the places where we hope the headers will
     be. */
  config_line_t *h2 = lines->next->next->next;
  config_line_t *h3 = h2->next;
  tt_str_op(h2->key, OP_EQ, "HEADER");
  tt_str_op(h3->key, OP_EQ, "header");

  orig = lines;
  rest = config_lines_partition(lines, "Header");
  tt_ptr_op(lines, OP_EQ, orig);
  tt_ptr_op(rest, OP_EQ, h2);
  tt_str_op(lines->next->key, OP_EQ, "Item");
  tt_str_op(lines->next->next->key, OP_EQ, "Thing");
  tt_ptr_op(lines->next->next->next, OP_EQ, NULL);
  config_free_lines(lines);

  orig = lines = rest;
  rest = config_lines_partition(lines, "Header");
  tt_ptr_op(lines, OP_EQ, orig);
  tt_ptr_op(rest, OP_EQ, h3);
  tt_ptr_op(lines->next, OP_EQ, NULL);
  config_free_lines(lines);

  orig = lines = rest;
  rest = config_lines_partition(lines, "Header");
  tt_ptr_op(lines, OP_EQ, orig);
  tt_ptr_op(rest, OP_EQ, NULL);
  tt_str_op(lines->next->key, OP_EQ, "Item3");
  tt_ptr_op(lines->next->next, OP_EQ, NULL);

 done:
  config_free_lines(lines);
  config_free_lines(rest);
}

#ifndef DISABLE_PWDB_TESTS
static void
test_util_expand_filename(void *arg)
{
  char *str;

  (void)arg;
  setenv("HOME", "/home/itv", 1); /* For "internal test value" */

  str = expand_filename("");
  tt_str_op("",OP_EQ, str);
  tor_free(str);

  str = expand_filename("/normal/path");
  tt_str_op("/normal/path",OP_EQ, str);
  tor_free(str);

  str = expand_filename("/normal/trailing/path/");
  tt_str_op("/normal/trailing/path/",OP_EQ, str);
  tor_free(str);

  str = expand_filename("~");
  tt_str_op("/home/itv/",OP_EQ, str);
  tor_free(str);

  str = expand_filename("$HOME/nodice");
  tt_str_op("$HOME/nodice",OP_EQ, str);
  tor_free(str);

  str = expand_filename("~/");
  tt_str_op("/home/itv/",OP_EQ, str);
  tor_free(str);

  str = expand_filename("~/foobarqux");
  tt_str_op("/home/itv/foobarqux",OP_EQ, str);
  tor_free(str);

  str = expand_filename("~/../../etc/passwd");
  tt_str_op("/home/itv/../../etc/passwd",OP_EQ, str);
  tor_free(str);

  str = expand_filename("~/trailing/");
  tt_str_op("/home/itv/trailing/",OP_EQ, str);
  tor_free(str);
  /* Ideally we'd test ~anotheruser, but that's shady to test (we'd
     have to somehow inject/fake the get_user_homedir call) */

  /* $HOME ending in a trailing slash */
  setenv("HOME", "/home/itv/", 1);

  str = expand_filename("~");
  tt_str_op("/home/itv/",OP_EQ, str);
  tor_free(str);

  str = expand_filename("~/");
  tt_str_op("/home/itv/",OP_EQ, str);
  tor_free(str);

  str = expand_filename("~/foo");
  tt_str_op("/home/itv/foo",OP_EQ, str);
  tor_free(str);

  /* Try with empty $HOME */

  setenv("HOME", "", 1);

  str = expand_filename("~");
  tt_str_op("/",OP_EQ, str);
  tor_free(str);

  str = expand_filename("~/");
  tt_str_op("/",OP_EQ, str);
  tor_free(str);

  str = expand_filename("~/foobar");
  tt_str_op("/foobar",OP_EQ, str);
  tor_free(str);

  /* Try with $HOME unset */

  unsetenv("HOME");

  str = expand_filename("~");
  tt_str_op("/",OP_EQ, str);
  tor_free(str);

  str = expand_filename("~/");
  tt_str_op("/",OP_EQ, str);
  tor_free(str);

  str = expand_filename("~/foobar");
  tt_str_op("/foobar",OP_EQ, str);
  tor_free(str);

 done:
  tor_free(str);
}
#endif /* !defined(DISABLE_PWDB_TESTS) */

/** Test tor_escape_str_for_pt_args(). */
static void
test_util_escape_string_socks(void *arg)
{
  char *escaped_string = NULL;

  /** Simple backslash escape. */
  (void)arg;
  escaped_string = tor_escape_str_for_pt_args("This is a backslash: \\",";\\");
  tt_assert(escaped_string);
  tt_str_op(escaped_string,OP_EQ, "This is a backslash: \\\\");
  tor_free(escaped_string);

  /** Simple semicolon escape. */
  escaped_string = tor_escape_str_for_pt_args("First rule:Do not use ;",";\\");
  tt_assert(escaped_string);
  tt_str_op(escaped_string,OP_EQ, "First rule:Do not use \\;");
  tor_free(escaped_string);

  /** Empty string. */
  escaped_string = tor_escape_str_for_pt_args("", ";\\");
  tt_assert(escaped_string);
  tt_str_op(escaped_string,OP_EQ, "");
  tor_free(escaped_string);

  /** Escape all characters. */
  escaped_string = tor_escape_str_for_pt_args(";\\;\\", ";\\");
  tt_assert(escaped_string);
  tt_str_op(escaped_string,OP_EQ, "\\;\\\\\\;\\\\");
  tor_free(escaped_string);

  escaped_string = tor_escape_str_for_pt_args(";", ";\\");
  tt_assert(escaped_string);
  tt_str_op(escaped_string,OP_EQ, "\\;");
  tor_free(escaped_string);

 done:
  tor_free(escaped_string);
}

static void
test_util_string_is_key_value(void *ptr)
{
  (void)ptr;
  tt_assert(string_is_key_value(LOG_WARN, "key=value"));
  tt_assert(string_is_key_value(LOG_WARN, "k=v"));
  tt_assert(string_is_key_value(LOG_WARN, "key="));
  tt_assert(string_is_key_value(LOG_WARN, "x="));
  tt_assert(string_is_key_value(LOG_WARN, "xx="));
  tt_assert(!string_is_key_value(LOG_WARN, "=value"));
  tt_assert(!string_is_key_value(LOG_WARN, "=x"));
  tt_assert(!string_is_key_value(LOG_WARN, "="));

  /* ??? */
  /* tt_assert(!string_is_key_value(LOG_WARN, "===")); */
 done:
  ;
}

/** Test basic string functionality. */
static void
test_util_strmisc(void *arg)
{
  char buf[1024];
  char *cp_tmp = NULL;

  /* Test strl operations */
  (void)arg;
  tt_int_op(5,OP_EQ, strlcpy(buf, "Hello", 0));
  tt_int_op(5,OP_EQ, strlcpy(buf, "Hello", 10));
  tt_str_op(buf,OP_EQ, "Hello");
  tt_int_op(5,OP_EQ, strlcpy(buf, "Hello", 6));
  tt_str_op(buf,OP_EQ, "Hello");
  tt_int_op(5,OP_EQ, strlcpy(buf, "Hello", 5));
  tt_str_op(buf,OP_EQ, "Hell");
  strlcpy(buf, "Hello", sizeof(buf));
  tt_int_op(10,OP_EQ, strlcat(buf, "Hello", 5));

  /* Test strstrip() */
  strlcpy(buf, "Testing 1 2 3", sizeof(buf));
  tor_strstrip(buf, ",!");
  tt_str_op(buf,OP_EQ, "Testing 1 2 3");
  strlcpy(buf, "!Testing 1 2 3?", sizeof(buf));
  tor_strstrip(buf, "!? ");
  tt_str_op(buf,OP_EQ, "Testing123");
  strlcpy(buf, "!!!Testing 1 2 3??", sizeof(buf));
  tor_strstrip(buf, "!? ");
  tt_str_op(buf,OP_EQ, "Testing123");

  /* Test snprintf */
  /* Returning -1 when there's not enough room in the output buffer */
  tt_int_op(-1,OP_EQ, tor_snprintf(buf, 0, "Foo"));
  tt_int_op(-1,OP_EQ, tor_snprintf(buf, 2, "Foo"));
  tt_int_op(-1,OP_EQ, tor_snprintf(buf, 3, "Foo"));
  tt_int_op(-1,OP_NE, tor_snprintf(buf, 4, "Foo"));
  /* Always NUL-terminate the output */
  tor_snprintf(buf, 5, "abcdef");
  tt_int_op(0,OP_EQ, buf[4]);
  tor_snprintf(buf, 10, "abcdef");
  tt_int_op(0,OP_EQ, buf[6]);
  /* uint64 */
  tor_snprintf(buf, sizeof(buf), "x!%"PRIu64"!x",
               (UINT64_C(12345678901)));
  tt_str_op("x!12345678901!x",OP_EQ, buf);

  /* Test str{,case}cmpstart */
  tt_assert(strcmpstart("abcdef", "abcdef")==0);
  tt_assert(strcmpstart("abcdef", "abc")==0);
  tt_assert(strcmpstart("abcdef", "abd")<0);
  tt_assert(strcmpstart("abcdef", "abb")>0);
  tt_assert(strcmpstart("ab", "abb")<0);
  tt_assert(strcmpstart("ab", "")==0);
  tt_assert(strcmpstart("ab", "ab ")<0);
  tt_assert(strcasecmpstart("abcdef", "abCdEF")==0);
  tt_assert(strcasecmpstart("abcDeF", "abc")==0);
  tt_assert(strcasecmpstart("abcdef", "Abd")<0);
  tt_assert(strcasecmpstart("Abcdef", "abb")>0);
  tt_assert(strcasecmpstart("ab", "Abb")<0);
  tt_assert(strcasecmpstart("ab", "")==0);
  tt_assert(strcasecmpstart("ab", "ab ")<0);

  /* Test str{,case}cmpend */
  tt_assert(strcmpend("abcdef", "abcdef")==0);
  tt_assert(strcmpend("abcdef", "def")==0);
  tt_assert(strcmpend("abcdef", "deg")<0);
  tt_assert(strcmpend("abcdef", "dee")>0);
  tt_assert(strcmpend("ab", "aab")>0);
  tt_assert(strcasecmpend("AbcDEF", "abcdef")==0);
  tt_assert(strcasecmpend("abcdef", "dEF")==0);
  tt_assert(strcasecmpend("abcdef", "Deg")<0);
  tt_assert(strcasecmpend("abcDef", "dee")>0);
  tt_assert(strcasecmpend("AB", "abb")<0);

  /* Test digest_is_zero */
  memset(buf,0,20);
  buf[20] = 'x';
  tt_assert(tor_digest_is_zero(buf));
  buf[19] = 'x';
  tt_assert(!tor_digest_is_zero(buf));

  /* Test mem_is_zero */
  memset(buf,0,128);
  buf[128] = 'x';
  tt_assert(fast_mem_is_zero(buf, 10));
  tt_assert(fast_mem_is_zero(buf, 20));
  tt_assert(fast_mem_is_zero(buf, 128));
  tt_assert(!fast_mem_is_zero(buf, 129));
  buf[60] = (char)255;
  tt_assert(!fast_mem_is_zero(buf, 128));
  buf[0] = (char)1;
  tt_assert(!fast_mem_is_zero(buf, 10));

  /* Test 'escaped' */
  tt_ptr_op(escaped(NULL), OP_EQ, NULL);
  tt_str_op("\"\"",OP_EQ, escaped(""));
  tt_str_op("\"abcd\"",OP_EQ, escaped("abcd"));
  tt_str_op("\"\\\\ \\n\\r\\t\\\"\\'\"",OP_EQ, escaped("\\ \n\r\t\"'"));
  tt_str_op("\"unnecessary \\'backslashes\\'\"",OP_EQ,
             escaped("unnecessary \'backslashes\'"));
  /* Non-printable characters appear as octal */
  tt_str_op("\"z\\001abc\\277d\"",OP_EQ,  escaped("z\001abc\277d"));
  tt_str_op("\"z\\336\\255 ;foo\"",OP_EQ, escaped("z\xde\xad\x20;foo"));

  /* Other cases of esc_for_log{,_len} */
  cp_tmp = esc_for_log(NULL);
  tt_str_op(cp_tmp, OP_EQ, "(null)");
  tor_free(cp_tmp);
  cp_tmp = esc_for_log_len("abcdefg", 3);
  tt_str_op(cp_tmp, OP_EQ, "\"abc\"");
  tor_free(cp_tmp);
  cp_tmp = esc_for_log_len("abcdefg", 100);
  tt_str_op(cp_tmp, OP_EQ, "\"abcdefg\"");
  tor_free(cp_tmp);

  /* Test strndup and memdup */
  {
    const char *s = "abcdefghijklmnopqrstuvwxyz";
    cp_tmp = tor_strndup(s, 30);
    tt_str_op(cp_tmp,OP_EQ, s); /* same string, */
    tt_ptr_op(cp_tmp,OP_NE,s); /* but different pointers. */
    tor_free(cp_tmp);

    cp_tmp = tor_strndup(s, 5);
    tt_str_op(cp_tmp,OP_EQ, "abcde");
    tor_free(cp_tmp);

    s = "a\0b\0c\0d\0e\0";
    cp_tmp = tor_memdup(s,10);
    tt_mem_op(cp_tmp,OP_EQ, s, 10); /* same ram, */
    tt_ptr_op(cp_tmp,OP_NE,s); /* but different pointers. */
    tor_free(cp_tmp);
  }

  /* Test str-foo functions */
  cp_tmp = tor_strdup("abcdef");
  tt_assert(tor_strisnonupper(cp_tmp));
  cp_tmp[3] = 'D';
  tt_assert(!tor_strisnonupper(cp_tmp));
  tor_strupper(cp_tmp);
  tt_str_op(cp_tmp,OP_EQ, "ABCDEF");
  tor_strlower(cp_tmp);
  tt_str_op(cp_tmp,OP_EQ, "abcdef");
  tt_assert(tor_strisnonupper(cp_tmp));
  tt_assert(tor_strisprint(cp_tmp));
  cp_tmp[3] = 3;
  tt_assert(!tor_strisprint(cp_tmp));
  tor_free(cp_tmp);

  /* Test memmem and memstr */
  {
    const char *haystack = "abcde";
    tt_ptr_op(tor_memmem(haystack, 5, "ef", 2), OP_EQ, NULL);
    tt_ptr_op(tor_memmem(haystack, 5, "cd", 2),OP_EQ, haystack + 2);
    tt_ptr_op(tor_memmem(haystack, 5, "cde", 3),OP_EQ, haystack + 2);
    tt_ptr_op(tor_memmem(haystack, 4, "cde", 3), OP_EQ, NULL);
    haystack = "ababcad";
    tt_ptr_op(tor_memmem(haystack, 7, "abc", 3),OP_EQ, haystack + 2);
    tt_ptr_op(tor_memmem(haystack, 7, "ad", 2),OP_EQ, haystack + 5);
    tt_ptr_op(tor_memmem(haystack, 7, "cad", 3),OP_EQ, haystack + 4);
    tt_ptr_op(tor_memmem(haystack, 7, "dadad", 5), OP_EQ, NULL);
    tt_ptr_op(tor_memmem(haystack, 7, "abcdefghij", 10), OP_EQ, NULL);
    /* memstr */
    tt_ptr_op(tor_memstr(haystack, 7, "abc"),OP_EQ, haystack + 2);
    tt_ptr_op(tor_memstr(haystack, 7, "cad"),OP_EQ, haystack + 4);
    tt_ptr_op(tor_memstr(haystack, 6, "cad"), OP_EQ, NULL);
    tt_ptr_op(tor_memstr(haystack, 7, "cadd"), OP_EQ, NULL);
    tt_ptr_op(tor_memstr(haystack, 7, "fe"), OP_EQ, NULL);
    tt_ptr_op(tor_memstr(haystack, 7, "ababcade"), OP_EQ, NULL);
  }

  /* Test hex_str */
  {
    char binary_data[68];
    size_t idx;
    for (idx = 0; idx < sizeof(binary_data); ++idx)
      binary_data[idx] = idx;
    tt_str_op(hex_str(binary_data, 0),OP_EQ, "");
    tt_str_op(hex_str(binary_data, 1),OP_EQ, "00");
    tt_str_op(hex_str(binary_data, 17),OP_EQ,
              "000102030405060708090A0B0C0D0E0F10");
    tt_str_op(hex_str(binary_data, 32),OP_EQ,
               "000102030405060708090A0B0C0D0E0F"
               "101112131415161718191A1B1C1D1E1F");
    tt_str_op(hex_str(binary_data, 34),OP_EQ,
               "000102030405060708090A0B0C0D0E0F"
               "101112131415161718191A1B1C1D1E1F");
    /* Repeat these tests for shorter strings after longer strings
       have been tried, to make sure we're correctly terminating strings */
    tt_str_op(hex_str(binary_data, 1),OP_EQ, "00");
    tt_str_op(hex_str(binary_data, 0),OP_EQ, "");
  }

  /* Test strcmp_opt */
  tt_int_op(strcmp_opt("",   "foo"), OP_LT, 0);
  tt_int_op(strcmp_opt("",    ""),  OP_EQ, 0);
  tt_int_op(strcmp_opt("foo", ""),   OP_GT, 0);

  tt_int_op(strcmp_opt(NULL,  ""),    OP_LT, 0);
  tt_int_op(strcmp_opt(NULL,  NULL), OP_EQ, 0);
  tt_int_op(strcmp_opt("",    NULL),  OP_GT, 0);

  tt_int_op(strcmp_opt(NULL,  "foo"), OP_LT, 0);
  tt_int_op(strcmp_opt("foo", NULL),  OP_GT, 0);

 done:
  tor_free(cp_tmp);
}

static void
test_util_parse_integer(void *arg)
{
  (void)arg;
  int i;
  char *cp;

  /* Test parse_long */
  /* Empty/zero input */
  tt_int_op(0L,OP_EQ, tor_parse_long("",10,0,100,&i,NULL));
  tt_int_op(0,OP_EQ, i);
  tt_int_op(0L,OP_EQ, tor_parse_long("0",10,0,100,&i,NULL));
  tt_int_op(1,OP_EQ, i);
  /* Normal cases */
  tt_int_op(10L,OP_EQ, tor_parse_long("10",10,0,100,&i,NULL));
  tt_int_op(1,OP_EQ, i);
  tt_int_op(10L,OP_EQ, tor_parse_long("10",10,0,10,&i,NULL));
  tt_int_op(1,OP_EQ, i);
  tt_int_op(10L,OP_EQ, tor_parse_long("10",10,10,100,&i,NULL));
  tt_int_op(1,OP_EQ, i);
  tt_int_op(-50L,OP_EQ, tor_parse_long("-50",10,-100,100,&i,NULL));
  tt_int_op(1,OP_EQ, i);
  tt_int_op(-50L,OP_EQ, tor_parse_long("-50",10,-100,0,&i,NULL));
  tt_int_op(1,OP_EQ, i);
  tt_int_op(-50L,OP_EQ, tor_parse_long("-50",10,-50,0,&i,NULL));
  tt_int_op(1,OP_EQ, i);
  /* Extra garbage */
  tt_int_op(0L,OP_EQ, tor_parse_long("10m",10,0,100,&i,NULL));
  tt_int_op(0,OP_EQ, i);
  tt_int_op(0L,OP_EQ, tor_parse_long("-50 plus garbage",10,-100,100,&i,NULL));
  tt_int_op(0,OP_EQ, i);
  tt_int_op(10L,OP_EQ, tor_parse_long("10m",10,0,100,&i,&cp));
  tt_int_op(1,OP_EQ, i);
  tt_str_op(cp,OP_EQ, "m");
  tt_int_op(-50L,OP_EQ, tor_parse_long("-50 plus garbage",10,-100,100,&i,&cp));
  tt_int_op(1,OP_EQ, i);
  tt_str_op(cp,OP_EQ, " plus garbage");
  /* Illogical min max */
  tt_int_op(0L,OP_EQ,  tor_parse_long("10",10,50,4,&i,NULL));
  tt_int_op(0,OP_EQ, i);
  tt_int_op(0L,OP_EQ,   tor_parse_long("-50",10,100,-100,&i,NULL));
  tt_int_op(0,OP_EQ, i);
  /* Out of bounds */
  tt_int_op(0L,OP_EQ,  tor_parse_long("10",10,50,100,&i,NULL));
  tt_int_op(0,OP_EQ, i);
  tt_int_op(0L,OP_EQ,   tor_parse_long("-50",10,0,100,&i,NULL));
  tt_int_op(0,OP_EQ, i);
  /* Base different than 10 */
  tt_int_op(2L,OP_EQ,   tor_parse_long("10",2,0,100,NULL,NULL));
  tt_int_op(0L,OP_EQ,   tor_parse_long("2",2,0,100,NULL,NULL));
  tt_int_op(68284L,OP_EQ, tor_parse_long("10abc",16,0,70000,NULL,NULL));
  tt_int_op(68284L,OP_EQ, tor_parse_long("10ABC",16,0,70000,NULL,NULL));
  tt_int_op(0L,OP_EQ,   tor_parse_long("10",-2,0,100,NULL,NULL));
  tt_int_op(0,OP_EQ, tor_parse_long("10ABC",-1,0,70000,&i,NULL));
  tt_int_op(i,OP_EQ, 0);

  /* Test parse_ulong */
  tt_int_op(0UL,OP_EQ, tor_parse_ulong("",10,0,100,NULL,NULL));
  tt_int_op(0UL,OP_EQ, tor_parse_ulong("0",10,0,100,NULL,NULL));
  tt_int_op(10UL,OP_EQ, tor_parse_ulong("10",10,0,100,NULL,NULL));
  tt_int_op(0UL,OP_EQ, tor_parse_ulong("10",10,50,100,NULL,NULL));
  tt_int_op(10UL,OP_EQ, tor_parse_ulong("10",10,0,10,NULL,NULL));
  tt_int_op(10UL,OP_EQ, tor_parse_ulong("10",10,10,100,NULL,NULL));
  tt_int_op(0UL,OP_EQ, tor_parse_ulong("8",8,0,100,NULL,NULL));
  tt_int_op(50UL,OP_EQ, tor_parse_ulong("50",10,50,100,NULL,NULL));
  tt_int_op(0UL,OP_EQ, tor_parse_ulong("-50",10,0,100,NULL,NULL));
  tt_int_op(0UL,OP_EQ, tor_parse_ulong("50",-1,50,100,&i,NULL));
  tt_int_op(0,OP_EQ, i);
  tt_int_op(0UL,OP_EQ, tor_parse_ulong("-50",10,0,100,&i,NULL));
  tt_int_op(0,OP_EQ, i);

  /* Test parse_uint64 */
  tt_assert(UINT64_C(10) == tor_parse_uint64("10 x",10,0,100, &i, &cp));
  tt_int_op(1,OP_EQ, i);
  tt_str_op(cp,OP_EQ, " x");
  tt_assert(UINT64_C(12345678901) ==
              tor_parse_uint64("12345678901",10,0,UINT64_MAX, &i, &cp));
  tt_int_op(1,OP_EQ, i);
  tt_str_op(cp,OP_EQ, "");
  tt_assert(UINT64_C(0) ==
              tor_parse_uint64("12345678901",10,500,INT32_MAX, &i, &cp));
  tt_int_op(0,OP_EQ, i);
  tt_assert(UINT64_C(0) ==
              tor_parse_uint64("123",-1,0,INT32_MAX, &i, &cp));
  tt_int_op(0,OP_EQ, i);

  {
  /* Test parse_double */
  double d = tor_parse_double("10", 0, (double)UINT64_MAX,&i,NULL);
  tt_int_op(1,OP_EQ, i);
  tt_assert(((uint64_t)d) == 10);
  d = tor_parse_double("0", 0, (double)UINT64_MAX,&i,NULL);
  tt_int_op(1,OP_EQ, i);
  tt_assert(((uint64_t)d) == 0);
  d = tor_parse_double(" ", 0, (double)UINT64_MAX,&i,NULL);
  tt_double_op(fabs(d), OP_LT, 1e-10);
  tt_int_op(0,OP_EQ, i);
  d = tor_parse_double(".0a", 0, (double)UINT64_MAX,&i,NULL);
  tt_double_op(fabs(d), OP_LT, 1e-10);
  tt_int_op(0,OP_EQ, i);
  d = tor_parse_double(".0a", 0, (double)UINT64_MAX,&i,&cp);
  tt_double_op(fabs(d), OP_LT, 1e-10);
  tt_int_op(1,OP_EQ, i);
  d = tor_parse_double("-.0", 0, (double)UINT64_MAX,&i,NULL);
  tt_int_op(1,OP_EQ, i);
  tt_assert(((uint64_t)d) == 0);
  d = tor_parse_double("-10", -100.0, 100.0,&i,NULL);
  tt_int_op(1,OP_EQ, i);
  tt_double_op(fabs(d - -10.0),OP_LT, 1E-12);
  }

  {
    /* Test tor_parse_* where we overflow/underflow the underlying type. */
    /* This string should overflow 64-bit ints. */
#define TOOBIG "100000000000000000000000000"
    tt_int_op(0L, OP_EQ,
              tor_parse_long(TOOBIG, 10, LONG_MIN, LONG_MAX, &i, NULL));
    tt_int_op(i,OP_EQ, 0);
    tt_int_op(0L,OP_EQ,
              tor_parse_long("-"TOOBIG, 10, LONG_MIN, LONG_MAX, &i, NULL));
    tt_int_op(i,OP_EQ, 0);
    tt_int_op(0UL,OP_EQ, tor_parse_ulong(TOOBIG, 10, 0, ULONG_MAX, &i, NULL));
    tt_int_op(i,OP_EQ, 0);
    tt_u64_op(UINT64_C(0), OP_EQ, tor_parse_uint64(TOOBIG, 10,
                                             0, UINT64_MAX, &i, NULL));
    tt_int_op(i,OP_EQ, 0);
  }
 done:
  ;
}

static void
test_util_pow2(void *arg)
{
  /* Test tor_log2(). */
  (void)arg;
  tt_int_op(tor_log2(64),OP_EQ, 6);
  tt_int_op(tor_log2(65),OP_EQ, 6);
  tt_int_op(tor_log2(63),OP_EQ, 5);
  /* incorrect mathematically, but as specified: */
  tt_int_op(tor_log2(0),OP_EQ, 0);
  tt_int_op(tor_log2(1),OP_EQ, 0);
  tt_int_op(tor_log2(2),OP_EQ, 1);
  tt_int_op(tor_log2(3),OP_EQ, 1);
  tt_int_op(tor_log2(4),OP_EQ, 2);
  tt_int_op(tor_log2(5),OP_EQ, 2);
  tt_int_op(tor_log2(UINT64_C(40000000000000000)),OP_EQ, 55);
  tt_int_op(tor_log2(UINT64_MAX),OP_EQ, 63);

  /* Test round_to_power_of_2 */
  tt_u64_op(round_to_power_of_2(120), OP_EQ, 128);
  tt_u64_op(round_to_power_of_2(128), OP_EQ, 128);
  tt_u64_op(round_to_power_of_2(130), OP_EQ, 128);
  tt_u64_op(round_to_power_of_2(UINT64_C(40000000000000000)), OP_EQ,
            UINT64_C(1)<<55);
  tt_u64_op(round_to_power_of_2(UINT64_C(0xffffffffffffffff)), OP_EQ,
          UINT64_C(1)<<63);
  tt_u64_op(round_to_power_of_2(0), OP_EQ, 1);
  tt_u64_op(round_to_power_of_2(1), OP_EQ, 1);
  tt_u64_op(round_to_power_of_2(2), OP_EQ, 2);
  tt_u64_op(round_to_power_of_2(3), OP_EQ, 2);
  tt_u64_op(round_to_power_of_2(4), OP_EQ, 4);
  tt_u64_op(round_to_power_of_2(5), OP_EQ, 4);
  tt_u64_op(round_to_power_of_2(6), OP_EQ, 4);
  tt_u64_op(round_to_power_of_2(7), OP_EQ, 8);

 done:
  ;
}

static void
test_util_compress_impl(compress_method_t method)
{
  char *buf1=NULL, *buf2=NULL, *buf3=NULL;
  size_t len1, len2;

  tt_assert(tor_compress_supports_method(method));

  if (method != NO_METHOD) {
    tt_ptr_op(tor_compress_version_str(method), OP_NE, NULL);
    tt_ptr_op(tor_compress_header_version_str(method), OP_NE, NULL);
  }

  buf1 = tor_strdup("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAZAAAAAAAAAAAAAAAAAAAZ");
  tt_assert(detect_compression_method(buf1, strlen(buf1)) == UNKNOWN_METHOD);

  tt_assert(!tor_compress(&buf2, &len1, buf1, strlen(buf1)+1, method));
  tt_ptr_op(buf2, OP_NE, NULL);
  if (method == NO_METHOD) {
    // The identity transform doesn't actually compress, and it isn't
    // detectable as "the identity transform."
    tt_int_op(len1, OP_EQ, strlen(buf1)+1);
    tt_int_op(detect_compression_method(buf2, len1), OP_EQ, UNKNOWN_METHOD);
  } else {
    tt_int_op(len1, OP_LT, strlen(buf1));
    tt_int_op(detect_compression_method(buf2, len1), OP_EQ, method);
  }

  tt_assert(!tor_uncompress(&buf3, &len2, buf2, len1, method, 1, LOG_INFO));
  tt_ptr_op(buf3, OP_NE, NULL);
  tt_int_op(strlen(buf1) + 1, OP_EQ, len2);
  tt_str_op(buf1, OP_EQ, buf3);
  tt_int_op(buf3[len2], OP_EQ, 0);

  /* Check whether we can uncompress concatenated, compressed strings. */
  tor_free(buf3);
  buf2 = tor_reallocarray(buf2, len1, 2);
  memcpy(buf2+len1, buf2, len1);
  tt_assert(!tor_uncompress(&buf3, &len2, buf2, len1*2, method, 1, LOG_INFO));
  tt_int_op((strlen(buf1)+1)*2, OP_EQ, len2);
  tt_mem_op(buf3, OP_EQ,
             "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAZAAAAAAAAAAAAAAAAAAAZ\0"
             "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAZAAAAAAAAAAAAAAAAAAAZ\0",
             (strlen(buf1)+1)*2);
  tt_int_op(buf3[len2], OP_EQ, 0);

  /* Check whether we can uncompress partial strings */

  tor_free(buf1);
  tor_free(buf2);
  tor_free(buf3);

  size_t b1len = 1<<10;
  if (method == ZSTD_METHOD) {
    // zstd needs a big input before it starts generating output that it
    // can partially decompress.
    b1len = 1<<18;
  }
  buf1 = tor_malloc(b1len);
  crypto_rand(buf1, b1len);
  tt_assert(!tor_compress(&buf2, &len1, buf1, b1len, method));
  tt_int_op(len1, OP_GT, 16);
  /* when we allow an incomplete output we should succeed.*/
  tt_assert(!tor_uncompress(&buf3, &len2, buf2, len1-16,
                            method, 0, LOG_INFO));
  tt_int_op(len2, OP_GT, 5);
  tt_int_op(len2, OP_LE, len1);
  tt_assert(fast_memeq(buf1, buf3, len2));
  tt_int_op(buf3[len2], OP_EQ, 0);

  /* when we demand a complete output from a real compression method, this
   * must fail. */
  tor_free(buf3);
  if (method != NO_METHOD) {
    tt_assert(tor_uncompress(&buf3, &len2, buf2, len1-16,
                             method, 1, LOG_INFO));
    tt_ptr_op(buf3, OP_EQ, NULL);
  }

 done:
  tor_free(buf1);
  tor_free(buf2);
  tor_free(buf3);
}

static void
test_util_compress_stream_impl(compress_method_t method,
                               compression_level_t level)
{
  char *buf1=NULL, *buf2=NULL, *buf3=NULL, *cp1, *cp2;
  const char *ccp2;
  size_t len1, len2;

  tor_compress_state_t *state = NULL;
  state = tor_compress_new(1, method, level);
  tt_assert(state);
  cp1 = buf1 = tor_malloc(1024);
  len1 = 1024;
  ccp2 = "ABCDEFGHIJABCDEFGHIJ";
  len2 = 21;
  tt_int_op(tor_compress_process(state, &cp1, &len1, &ccp2, &len2, 0),
            OP_EQ, TOR_COMPRESS_OK);
  tt_int_op(0, OP_EQ, len2); /* Make sure we compressed it all. */
  tt_assert(cp1 > buf1);

  len2 = 0;
  cp2 = cp1;
  tt_int_op(tor_compress_process(state, &cp1, &len1, &ccp2, &len2, 1),
            OP_EQ, TOR_COMPRESS_DONE);
  tt_int_op(0, OP_EQ, len2);
  if (method == NO_METHOD) {
    tt_ptr_op(cp1, OP_EQ, cp2);
  } else {
    tt_assert(cp1 > cp2); /* Make sure we really added something. */
  }

  tt_int_op(tor_compress_state_size(state), OP_GT, 0);

  tt_assert(!tor_uncompress(&buf3, &len2, buf1, 1024-len1,
                            method, 1, LOG_WARN));
  /* Make sure it compressed right. */
  tt_str_op(buf3, OP_EQ, "ABCDEFGHIJABCDEFGHIJ");
  tt_int_op(21, OP_EQ, len2);

 done:
  if (state)
    tor_compress_free(state);
  tor_free(buf1);
  tor_free(buf2);
  tor_free(buf3);
}

/** Setup function for compression tests: handles x-zstd:nostatic
 */
static void *
compression_test_setup(const struct testcase_t *testcase)
{
  tor_assert(testcase->setup_data);
  tor_assert(testcase->setup_data != (void*)TT_SKIP);
  const char *methodname = testcase->setup_data;

  if (!strcmp(methodname, "x-zstd:nostatic")) {
    methodname = "x-zstd";
    tor_zstd_set_static_apis_disabled_for_testing(1);
  }

  return (void *)methodname;
}

/** Cleanup for compression tests: disables nostatic */
static int
compression_test_cleanup(const struct testcase_t *testcase, void *ptr)
{
  (void)testcase;
  (void)ptr;
  tor_zstd_set_static_apis_disabled_for_testing(0);
  return 1;
}

static const struct testcase_setup_t compress_setup = {
  compression_test_setup, compression_test_cleanup
};

/** Run unit tests for compression functions */
static void
test_util_compress(void *arg)
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

  test_util_compress_impl(method);

  for (unsigned l = 0; l < ARRAY_LENGTH(levels); ++l) {
    compression_level_t level = levels[l];
    test_util_compress_stream_impl(method, level);
  }
 done:
  ;
}

static void
test_util_decompress_concatenated_impl(compress_method_t method)
{
  char input[4096];
  char *c1 = NULL, *c2 = NULL, *c3 = NULL;
  char *result = NULL;
  size_t sz1, sz2, sz3, szr;
  int r;

  crypto_rand(input, sizeof(input));

  /* Compress the input in two chunks. */
  r = tor_compress(&c1, &sz1, input, 2048, method);
  tt_int_op(r, OP_EQ, 0);
  r = tor_compress(&c2, &sz2, input+2048, 2048, method);
  tt_int_op(r, OP_EQ, 0);

  /* concatenate the chunks. */
  sz3 = sz1 + sz2;
  c3 = tor_malloc(sz3);
  memcpy(c3, c1, sz1);
  memcpy(c3+sz1, c2, sz2);

  /* decompress the concatenated result */
  r = tor_uncompress(&result, &szr, c3, sz3, method, 0, LOG_WARN);
  tt_int_op(r, OP_EQ, 0);
  tt_int_op(szr, OP_EQ, sizeof(input));
  tt_mem_op(result, OP_EQ, input, sizeof(input));

 done:
  tor_free(c1);
  tor_free(c2);
  tor_free(c3);
  tor_free(result);
}

static void
test_util_decompress_concatenated(void *arg)
{
  const char *methodname = arg;
  tt_assert(methodname);

  compress_method_t method = compression_method_get_by_name(methodname);
  tt_int_op(method, OP_NE, UNKNOWN_METHOD);
  if (! tor_compress_supports_method(method)) {
    tt_skip();
  }

  test_util_decompress_concatenated_impl(method);
 done:
  ;
}

static void
test_util_decompress_junk_impl(compress_method_t method)
{
  char input[4096];
  char *result = NULL, *result2 = NULL;
  size_t szr, szr2, sz;
  int r;

  /* This shouldn't be a compressed string according to any method. */
  strlcpy(input, "This shouldn't be a compressed string by any means.",
          sizeof(input));
  sz = strlen(input);
  setup_capture_of_logs(LOG_WARN);
  r = tor_uncompress(&result, &szr, input, sz, method, 0, LOG_WARN);
  tt_int_op(r, OP_EQ, -1);
  tt_ptr_op(result, OP_EQ, NULL);
  expect_log_msg_containing("Error while uncompressing data: bad input?");
  mock_clean_saved_logs();

  /* Now try again, with a compressed object that starts out good and turns to
     junk. */
  crypto_rand(input, sizeof(input));
  r = tor_compress(&result, &szr, input, sizeof(input), method);
  tt_int_op(r, OP_EQ, 0);
  crypto_rand(result+szr/2, szr-(szr/2)); // trash the 2nd half of the result
  r = tor_uncompress(&result2, &szr2, result, szr, method, 0, LOG_WARN);
  tt_int_op(r, OP_EQ, -1);
  expect_log_msg_containing("Error while uncompressing data: bad input?");

 done:
  teardown_capture_of_logs();
  tor_free(result);
  tor_free(result2);
}

static void
test_util_decompress_junk(void *arg)
{
  const char *methodname = arg;
  tt_assert(methodname);

  compress_method_t method = compression_method_get_by_name(methodname);
  tt_int_op(method, OP_NE, UNKNOWN_METHOD);
  if (! tor_compress_supports_method(method)) {
    tt_skip();
  }

  test_util_decompress_junk_impl(method);
 done:
  ;
}

/* mock replacement for tor_compress_is_compression_bomb that doesn't
 * believe in compression bombs. */
static int
mock_is_never_compression_bomb(size_t in, size_t out)
{
  (void)in;
  (void) out;
  return 0;
}

static void
test_util_decompress_dos_impl(compress_method_t method)
{
  char *input;
  char *result = NULL, *result2 = NULL;
  size_t szr, szr2;
  int r;

  const size_t big = 1024*1024;
  /* one megabyte of 0s. */
  input = tor_malloc_zero(big);

  /* Compress it into "result": it should fail. */
  setup_full_capture_of_logs(LOG_WARN);
  r = tor_compress(&result, &szr, input, big, method);
  tt_int_op(r, OP_EQ, -1);
  expect_log_msg_containing(
                 "other Tors would think this was a compression bomb");
  teardown_capture_of_logs();

  /* Try again, but this time suppress compression-bomb detection */
  MOCK(tor_compress_is_compression_bomb, mock_is_never_compression_bomb);
  r = tor_compress(&result, &szr, input, big, method);
  UNMOCK(tor_compress_is_compression_bomb);
  tt_int_op(r, OP_EQ, 0);
  tt_ptr_op(result, OP_NE, NULL);

  /* We should refuse to uncomrpess it again, since it looks like a
   * compression bomb. */
  setup_capture_of_logs(LOG_WARN);
  r = tor_uncompress(&result2, &szr2, result, szr, method, 0, LOG_WARN);
  tt_int_op(r, OP_EQ, -1);
  expect_log_msg_containing("bomb; abandoning stream");

 done:
  teardown_capture_of_logs();
  tor_free(input);
  tor_free(result);
  tor_free(result2);
}

static void
test_util_decompress_dos(void *arg)
{
  const char *methodname = arg;
  tt_assert(methodname);

  compress_method_t method = compression_method_get_by_name(methodname);
  tt_int_op(method, OP_NE, UNKNOWN_METHOD);
  if (! tor_compress_supports_method(method)) {
    tt_skip();
  }

  test_util_decompress_dos_impl(method);
 done:
  ;
}

static void
test_util_gzip_compression_bomb(void *arg)
{
  /* A 'compression bomb' is a very small object that uncompresses to a huge
   * one. Most compression formats support them, but they can be a DOS vector.
   * In Tor we try not to generate them, and we don't accept them.
   */
  (void) arg;
  size_t one_million = 1<<20;
  char *one_mb = tor_malloc_zero(one_million);
  char *result = NULL;
  size_t result_len = 0;
  tor_compress_state_t *state = NULL;

  /* Make sure we can't produce a compression bomb */
  setup_full_capture_of_logs(LOG_WARN);
  tt_int_op(-1, OP_EQ, tor_compress(&result, &result_len,
                                    one_mb, one_million,
                                    ZLIB_METHOD));
  expect_single_log_msg_containing(
         "We compressed something and got an insanely high "
         "compression factor; other Tors would think this "
         "was a compression bomb.");
  teardown_capture_of_logs();

  /* Here's a compression bomb that we made manually. */
  const char compression_bomb[1039] =
    { 0x78, 0xDA, 0xED, 0xC1, 0x31, 0x01, 0x00, 0x00, 0x00, 0xC2,
      0xA0, 0xF5, 0x4F, 0x6D, 0x08, 0x5F, 0xA0 /* .... */ };
  tt_int_op(-1, OP_EQ, tor_uncompress(&result, &result_len,
                                      compression_bomb, 1039,
                                      ZLIB_METHOD, 0, LOG_WARN));

  /* Now try streaming that. */
  state = tor_compress_new(0, ZLIB_METHOD, HIGH_COMPRESSION);
  tor_compress_output_t r;
  const char *inp = compression_bomb;
  size_t inlen = 1039;
  do {
    char *outp = one_mb;
    size_t outleft = 4096; /* small on purpose */
    r = tor_compress_process(state, &outp, &outleft, &inp, &inlen, 0);
    tt_int_op(inlen, OP_NE, 0);
  } while (r == TOR_COMPRESS_BUFFER_FULL);

  tt_int_op(r, OP_EQ, TOR_COMPRESS_ERROR);

 done:
  tor_free(one_mb);
  tor_compress_free(state);
}

/** Run unit tests for mmap() wrapper functionality. */
static void
test_util_mmap(void *arg)
{
  char *fname1 = tor_strdup(get_fname("mapped_1"));
  char *fname2 = tor_strdup(get_fname("mapped_2"));
  char *fname3 = tor_strdup(get_fname("mapped_3"));
  const size_t buflen = 17000;
  char *buf = tor_malloc(17000);
  tor_mmap_t *mapping = NULL;

  (void)arg;
  crypto_rand(buf, buflen);

  mapping = tor_mmap_file(fname1);
  tt_ptr_op(mapping, OP_EQ, NULL);

  write_str_to_file(fname1, "Short file.", 1);

  mapping = tor_mmap_file(fname1);
  tt_assert(mapping);
  tt_int_op(mapping->size,OP_EQ, strlen("Short file."));
  tt_str_op(mapping->data,OP_EQ, "Short file.");
#ifdef _WIN32
  tt_int_op(0, OP_EQ, tor_munmap_file(mapping));
  mapping = NULL;
  tt_assert(unlink(fname1) == 0);
#else
  /* make sure we can unlink. */
  tt_assert(unlink(fname1) == 0);
  tt_str_op(mapping->data,OP_EQ, "Short file.");
  tt_int_op(0, OP_EQ, tor_munmap_file(mapping));
  mapping = NULL;
#endif /* defined(_WIN32) */

  /* Now a zero-length file. */
  write_str_to_file(fname1, "", 1);
  mapping = tor_mmap_file(fname1);
  tt_ptr_op(mapping,OP_EQ, NULL);
  tt_int_op(ERANGE,OP_EQ, errno);
  unlink(fname1);

  /* Make sure that we fail to map a no-longer-existent file. */
  mapping = tor_mmap_file(fname1);
  tt_ptr_op(mapping, OP_EQ, NULL);

  /* Now try a big file that stretches across a few pages and isn't aligned */
  write_bytes_to_file(fname2, buf, buflen, 1);
  mapping = tor_mmap_file(fname2);
  tt_assert(mapping);
  tt_int_op(mapping->size,OP_EQ, buflen);
  tt_mem_op(mapping->data,OP_EQ, buf, buflen);
  tt_int_op(0, OP_EQ, tor_munmap_file(mapping));
  mapping = NULL;

  /* Now try a big aligned file. */
  write_bytes_to_file(fname3, buf, 16384, 1);
  mapping = tor_mmap_file(fname3);
  tt_assert(mapping);
  tt_int_op(mapping->size,OP_EQ, 16384);
  tt_mem_op(mapping->data,OP_EQ, buf, 16384);
  tt_int_op(0, OP_EQ, tor_munmap_file(mapping));
  mapping = NULL;

 done:
  unlink(fname1);
  unlink(fname2);
  unlink(fname3);

  tor_free(fname1);
  tor_free(fname2);
  tor_free(fname3);
  tor_free(buf);

  tor_munmap_file(mapping);
}

/** Run unit tests for escaping/unescaping data for use by controllers. */
static void
test_util_control_formats(void *arg)
{
  char *out = NULL;
  const char *inp =
    "..This is a test\r\n.of the emergency \n..system.\r\n\rZ.\r\n";
  size_t sz;

  (void)arg;
  sz = read_escaped_data(inp, strlen(inp), &out);
  tt_str_op(out,OP_EQ,
             ".This is a test\nof the emergency \n.system.\n\rZ.\n");
  tt_int_op(sz,OP_EQ, strlen(out));

 done:
  tor_free(out);
}

#define test_feq(value1,value2) do {                               \
    double v1 = (value1), v2=(value2);                             \
    double tf_diff = v1-v2;                                        \
    double tf_tolerance = ((v1+v2)/2.0)/1e8;                       \
    if (tf_diff<0) tf_diff=-tf_diff;                               \
    if (tf_tolerance<0) tf_tolerance=-tf_tolerance;                \
    if (tf_diff<tf_tolerance) {                                    \
      TT_BLATHER(("%s ~~ %s: %f ~~ %f",#value1,#value2,v1,v2));  \
    } else {                                                       \
      TT_FAIL(("%s ~~ %s: %f != %f",#value1,#value2,v1,v2)); \
    }                                                              \
  } while (0)

static void
test_util_sscanf(void *arg)
{
  unsigned u1, u2, u3;
  unsigned long ulng;
  char s1[20], s2[10], s3[10], ch, *huge = NULL;
  int r;
  long lng1,lng2;
  int int1, int2;
  double d1,d2,d3,d4;

  /* Simple tests (malformed patterns, literal matching, ...) */
  (void)arg;
  tt_int_op(-1,OP_EQ, tor_sscanf("123", "%i", &r)); /* %i is not supported */
  tt_int_op(-1,OP_EQ,
            tor_sscanf("wrong", "%5c", s1)); /* %c cannot have a number. */
  tt_int_op(-1,OP_EQ, tor_sscanf("hello", "%s", s1)); /* %s needs a number. */
  /* this will fail because we don't allow widths longer than 9999 */
  {
    huge = tor_malloc(1000000);
    r = tor_sscanf("prettylongstring", "%99999s", huge);
    tor_free(huge);
    tt_int_op(-1,OP_EQ, r);
  }
#if 0
  /* GCC thinks these two are illegal. */
  test_eq(-1, tor_sscanf("prettylongstring", "%0s", s1));
  test_eq(0, tor_sscanf("prettylongstring", "%10s", NULL));
#endif
  /* No '%'-strings: always "success" */
  tt_int_op(0,OP_EQ, tor_sscanf("hello world", "hello world"));
  tt_int_op(0,OP_EQ, tor_sscanf("hello world", "good bye"));
  /* Excess data */
  tt_int_op(0,OP_EQ,
            tor_sscanf("hello 3", "%u", &u1));  /* have to match the start */
  tt_int_op(0,OP_EQ, tor_sscanf(" 3 hello", "%u", &u1));
  tt_int_op(0,OP_EQ,
            tor_sscanf(" 3 hello", "%2u", &u1)); /* not even in this case */
  tt_int_op(1,OP_EQ,
            tor_sscanf("3 hello", "%u", &u1));  /* but trailing is alright */

  /* Numbers (ie. %u) */
  tt_int_op(0,OP_EQ,
            tor_sscanf("hello world 3", "hello worlb %u", &u1)); /* d vs b */
  tt_int_op(1,OP_EQ, tor_sscanf("12345", "%u", &u1));
  tt_int_op(12345u,OP_EQ, u1);
  tt_int_op(1,OP_EQ, tor_sscanf("12346 ", "%u", &u1));
  tt_int_op(12346u,OP_EQ, u1);
  tt_int_op(0,OP_EQ, tor_sscanf(" 12347", "%u", &u1));
  tt_int_op(1,OP_EQ, tor_sscanf(" 12348", " %u", &u1));
  tt_int_op(12348u,OP_EQ, u1);
  tt_int_op(1,OP_EQ, tor_sscanf("0", "%u", &u1));
  tt_int_op(0u,OP_EQ, u1);
  tt_int_op(1,OP_EQ, tor_sscanf("0000", "%u", &u2));
  tt_int_op(0u,OP_EQ, u2);
  tt_int_op(0,OP_EQ, tor_sscanf("", "%u", &u1)); /* absent number */
  tt_int_op(0,OP_EQ, tor_sscanf("A", "%u", &u1)); /* bogus number */
  tt_int_op(0,OP_EQ, tor_sscanf("-1", "%u", &u1)); /* negative number */

  /* Numbers with size (eg. %2u) */
  tt_int_op(0,OP_EQ, tor_sscanf("-1", "%2u", &u1));
  tt_int_op(2,OP_EQ, tor_sscanf("123456", "%2u%u", &u1, &u2));
  tt_int_op(12u,OP_EQ, u1);
  tt_int_op(3456u,OP_EQ, u2);
  tt_int_op(1,OP_EQ, tor_sscanf("123456", "%8u", &u1));
  tt_int_op(123456u,OP_EQ, u1);
  tt_int_op(1,OP_EQ, tor_sscanf("123457  ", "%8u", &u1));
  tt_int_op(123457u,OP_EQ, u1);
  tt_int_op(0,OP_EQ, tor_sscanf("  123456", "%8u", &u1));
  tt_int_op(3,OP_EQ, tor_sscanf("!12:3:456", "!%2u:%2u:%3u", &u1, &u2, &u3));
  tt_int_op(12u,OP_EQ, u1);
  tt_int_op(3u,OP_EQ, u2);
  tt_int_op(456u,OP_EQ, u3);
  tt_int_op(3,OP_EQ,
            tor_sscanf("67:8:099", "%2u:%2u:%3u", &u1, &u2, &u3)); /* 0s */
  tt_int_op(67u,OP_EQ, u1);
  tt_int_op(8u,OP_EQ, u2);
  tt_int_op(99u,OP_EQ, u3);
  /* %u does not match space.*/
  tt_int_op(2,OP_EQ, tor_sscanf("12:3: 45", "%2u:%2u:%3u", &u1, &u2, &u3));
  tt_int_op(12u,OP_EQ, u1);
  tt_int_op(3u,OP_EQ, u2);
  /* %u does not match negative numbers. */
  tt_int_op(2,OP_EQ, tor_sscanf("67:8:-9", "%2u:%2u:%3u", &u1, &u2, &u3));
  tt_int_op(67u,OP_EQ, u1);
  tt_int_op(8u,OP_EQ, u2);
  /* Arbitrary amounts of 0-padding are okay */
  tt_int_op(3,OP_EQ, tor_sscanf("12:03:000000000000000099", "%2u:%2u:%u",
                        &u1, &u2, &u3));
  tt_int_op(12u,OP_EQ, u1);
  tt_int_op(3u,OP_EQ, u2);
  tt_int_op(99u,OP_EQ, u3);

  /* Hex (ie. %x) */
  tt_int_op(3,OP_EQ,
            tor_sscanf("1234 02aBcdEf ff", "%x %x %x", &u1, &u2, &u3));
  tt_int_op(0x1234,OP_EQ, u1);
  tt_int_op(0x2ABCDEF,OP_EQ, u2);
  tt_int_op(0xFF,OP_EQ, u3);
  /* Width works on %x */
  tt_int_op(3,OP_EQ, tor_sscanf("f00dcafe444", "%4x%4x%u", &u1, &u2, &u3));
  tt_int_op(0xf00d,OP_EQ, u1);
  tt_int_op(0xcafe,OP_EQ, u2);
  tt_int_op(444,OP_EQ, u3);

  /* Literal '%' (ie. '%%') */
  tt_int_op(1,OP_EQ, tor_sscanf("99% fresh", "%3u%% fresh", &u1));
  tt_int_op(99,OP_EQ, u1);
  tt_int_op(0,OP_EQ, tor_sscanf("99 fresh", "%% %3u %s", &u1, s1));
  tt_int_op(1,OP_EQ, tor_sscanf("99 fresh", "%3u%% %s", &u1, s1));
  tt_int_op(2,OP_EQ, tor_sscanf("99 fresh", "%3u %5s %%", &u1, s1));
  tt_int_op(99,OP_EQ, u1);
  tt_str_op(s1,OP_EQ, "fresh");
  tt_int_op(1,OP_EQ, tor_sscanf("% boo", "%% %3s", s1));
  tt_str_op("boo",OP_EQ, s1);

  /* Strings (ie. %s) */
  tt_int_op(2,OP_EQ, tor_sscanf("hello", "%3s%7s", s1, s2));
  tt_str_op(s1,OP_EQ, "hel");
  tt_str_op(s2,OP_EQ, "lo");
  tt_int_op(2,OP_EQ, tor_sscanf("WD40", "%2s%u", s3, &u1)); /* %s%u */
  tt_str_op(s3,OP_EQ, "WD");
  tt_int_op(40,OP_EQ, u1);
  tt_int_op(2,OP_EQ, tor_sscanf("WD40", "%3s%u", s3, &u1)); /* %s%u */
  tt_str_op(s3,OP_EQ, "WD4");
  tt_int_op(0,OP_EQ, u1);
  tt_int_op(2,OP_EQ, tor_sscanf("76trombones", "%6u%9s", &u1, s1)); /* %u%s */
  tt_int_op(76,OP_EQ, u1);
  tt_str_op(s1,OP_EQ, "trombones");
  {
    huge = tor_malloc(1000);
    r = tor_sscanf("prettylongstring", "%999s", huge);
    tt_int_op(1,OP_EQ, r);
    tt_str_op(huge,OP_EQ, "prettylongstring");
    tor_free(huge);
  }
  /* %s doesn't eat spaces */
  tt_int_op(2,OP_EQ, tor_sscanf("hello world", "%9s %9s", s1, s2));
  tt_str_op(s1,OP_EQ, "hello");
  tt_str_op(s2,OP_EQ, "world");
  tt_int_op(2,OP_EQ, tor_sscanf("bye   world?", "%9s %9s", s1, s2));
  tt_str_op(s1,OP_EQ, "bye");
  tt_str_op(s2,OP_EQ, "");
  tt_int_op(3,OP_EQ,
            tor_sscanf("hi", "%9s%9s%3s", s1, s2, s3)); /* %s can be empty. */
  tt_str_op(s1,OP_EQ, "hi");
  tt_str_op(s2,OP_EQ, "");
  tt_str_op(s3,OP_EQ, "");

  tt_int_op(3,OP_EQ, tor_sscanf("1.2.3", "%u.%u.%u%c", &u1, &u2, &u3, &ch));
  tt_int_op(4,OP_EQ,
            tor_sscanf("1.2.3 foobar", "%u.%u.%u%c", &u1, &u2, &u3, &ch));
  tt_int_op(' ',OP_EQ, ch);

  r = tor_sscanf("12345 -67890 -1", "%d %ld %d", &int1, &lng1, &int2);
  tt_int_op(r,OP_EQ, 3);
  tt_int_op(int1,OP_EQ, 12345);
  tt_int_op(lng1,OP_EQ, -67890);
  tt_int_op(int2,OP_EQ, -1);

#if SIZEOF_INT == 4
  /* %u */
  /* UINT32_MAX should work */
  tt_int_op(1,OP_EQ, tor_sscanf("4294967295", "%u", &u1));
  tt_int_op(4294967295U,OP_EQ, u1);

  /* But UINT32_MAX + 1 shouldn't work */
  tt_int_op(0,OP_EQ, tor_sscanf("4294967296", "%u", &u1));
  /* but parsing only 9... */
  tt_int_op(1,OP_EQ, tor_sscanf("4294967296", "%9u", &u1));
  tt_int_op(429496729U,OP_EQ, u1);

  /* %x */
  /* UINT32_MAX should work */
  tt_int_op(1,OP_EQ, tor_sscanf("FFFFFFFF", "%x", &u1));
  tt_int_op(0xFFFFFFFF,OP_EQ, u1);

  /* But UINT32_MAX + 1 shouldn't work */
  tt_int_op(0,OP_EQ, tor_sscanf("100000000", "%x", &u1));

  /* %d */
  /* INT32_MIN and INT32_MAX should work */
  r = tor_sscanf("-2147483648. 2147483647.", "%d. %d.", &int1, &int2);
  tt_int_op(r,OP_EQ, 2);
  tt_int_op(int1,OP_EQ, -2147483647 - 1);
  tt_int_op(int2,OP_EQ, 2147483647);

  /* But INT32_MIN - 1 and INT32_MAX + 1 shouldn't work */
  r = tor_sscanf("-2147483649.", "%d.", &int1);
  tt_int_op(r,OP_EQ, 0);

  r = tor_sscanf("2147483648.", "%d.", &int1);
  tt_int_op(r,OP_EQ, 0);

  /* and the first failure stops further processing */
  r = tor_sscanf("-2147483648. 2147483648.",
                 "%d. %d.", &int1, &int2);
  tt_int_op(r,OP_EQ, 1);

  r = tor_sscanf("-2147483649. 2147483647.",
                 "%d. %d.", &int1, &int2);
  tt_int_op(r,OP_EQ, 0);

  r = tor_sscanf("2147483648. -2147483649.",
                 "%d. %d.", &int1, &int2);
  tt_int_op(r,OP_EQ, 0);
#elif SIZEOF_INT == 8
  /* %u */
  /* UINT64_MAX should work */
  tt_int_op(1,OP_EQ, tor_sscanf("18446744073709551615", "%u", &u1));
  tt_int_op(18446744073709551615U,OP_EQ, u1);

  /* But UINT64_MAX + 1 shouldn't work */
  tt_int_op(0,OP_EQ, tor_sscanf("18446744073709551616", "%u", &u1));
  /* but parsing only 19... */
  tt_int_op(1,OP_EQ, tor_sscanf("18446744073709551616", "%19u", &u1));
  tt_int_op(1844674407370955161U,OP_EQ, u1);

  /* %x */
  /* UINT64_MAX should work */
  tt_int_op(1,OP_EQ, tor_sscanf("FFFFFFFFFFFFFFFF", "%x", &u1));
  tt_int_op(0xFFFFFFFFFFFFFFFF,OP_EQ, u1);

  /* But UINT64_MAX + 1 shouldn't work */
  tt_int_op(0,OP_EQ, tor_sscanf("10000000000000000", "%x", &u1));

  /* %d */
  /* INT64_MIN and INT64_MAX should work */
  r = tor_sscanf("-9223372036854775808. 9223372036854775807.",
                 "%d. %d.", &int1, &int2);
  tt_int_op(r,OP_EQ, 2);
  tt_int_op(int1,OP_EQ, -9223372036854775807 - 1);
  tt_int_op(int2,OP_EQ, 9223372036854775807);

  /* But INT64_MIN - 1 and INT64_MAX + 1 shouldn't work */
  r = tor_sscanf("-9223372036854775809.", "%d.", &int1);
  tt_int_op(r,OP_EQ, 0);

  r = tor_sscanf("9223372036854775808.", "%d.", &int1);
  tt_int_op(r,OP_EQ, 0);

  /* and the first failure stops further processing */
  r = tor_sscanf("-9223372036854775808. 9223372036854775808.",
                 "%d. %d.", &int1, &int2);
  tt_int_op(r,OP_EQ, 1);

  r = tor_sscanf("-9223372036854775809. 9223372036854775807.",
                 "%d. %d.", &int1, &int2);
  tt_int_op(r,OP_EQ, 0);

  r = tor_sscanf("9223372036854775808. -9223372036854775809.",
                 "%d. %d.", &int1, &int2);
  tt_int_op(r,OP_EQ, 0);
#endif /* SIZEOF_INT == 4 || ... */

#if SIZEOF_LONG == 4
  /* %lu */
  /* UINT32_MAX should work */
  tt_int_op(1,OP_EQ, tor_sscanf("4294967295", "%lu", &ulng));
  tt_int_op(4294967295UL,OP_EQ, ulng);

  /* But UINT32_MAX + 1 shouldn't work */
  tt_int_op(0,OP_EQ, tor_sscanf("4294967296", "%lu", &ulng));
  /* but parsing only 9... */
  tt_int_op(1,OP_EQ, tor_sscanf("4294967296", "%9lu", &ulng));
  tt_int_op(429496729UL,OP_EQ, ulng);

  /* %lx */
  /* UINT32_MAX should work */
  tt_int_op(1,OP_EQ, tor_sscanf("FFFFFFFF", "%lx", &ulng));
  tt_int_op(0xFFFFFFFFUL,OP_EQ, ulng);

  /* But UINT32_MAX + 1 shouldn't work */
  tt_int_op(0,OP_EQ, tor_sscanf("100000000", "%lx", &ulng));

  /* %ld */
  /* INT32_MIN and INT32_MAX should work */
  r = tor_sscanf("-2147483648. 2147483647.", "%ld. %ld.", &lng1, &lng2);
  tt_int_op(r,OP_EQ, 2);
  tt_int_op(lng1,OP_EQ, -2147483647L - 1L);
  tt_int_op(lng2,OP_EQ, 2147483647L);

  /* But INT32_MIN - 1 and INT32_MAX + 1 shouldn't work */
  r = tor_sscanf("-2147483649.", "%ld.", &lng1);
  tt_int_op(r,OP_EQ, 0);

  r = tor_sscanf("2147483648.", "%ld.", &lng1);
  tt_int_op(r,OP_EQ, 0);

  /* and the first failure stops further processing */
  r = tor_sscanf("-2147483648. 2147483648.",
                 "%ld. %ld.", &lng1, &lng2);
  tt_int_op(r,OP_EQ, 1);

  r = tor_sscanf("-2147483649. 2147483647.",
                 "%ld. %ld.", &lng1, &lng2);
  tt_int_op(r,OP_EQ, 0);

  r = tor_sscanf("2147483648. -2147483649.",
                 "%ld. %ld.", &lng1, &lng2);
  tt_int_op(r,OP_EQ, 0);
#elif SIZEOF_LONG == 8
  /* %lu */
  /* UINT64_MAX should work */
  tt_int_op(1,OP_EQ, tor_sscanf("18446744073709551615", "%lu", &ulng));
  tt_int_op(18446744073709551615UL,OP_EQ, ulng);

  /* But UINT64_MAX + 1 shouldn't work */
  tt_int_op(0,OP_EQ, tor_sscanf("18446744073709551616", "%lu", &ulng));
  /* but parsing only 19... */
  tt_int_op(1,OP_EQ, tor_sscanf("18446744073709551616", "%19lu", &ulng));
  tt_int_op(1844674407370955161UL,OP_EQ, ulng);

  /* %lx */
  /* UINT64_MAX should work */
  tt_int_op(1,OP_EQ, tor_sscanf("FFFFFFFFFFFFFFFF", "%lx", &ulng));
  tt_int_op(0xFFFFFFFFFFFFFFFFUL,OP_EQ, ulng);

  /* But UINT64_MAX + 1 shouldn't work */
  tt_int_op(0,OP_EQ, tor_sscanf("10000000000000000", "%lx", &ulng));

  /* %ld */
  /* INT64_MIN and INT64_MAX should work */
  r = tor_sscanf("-9223372036854775808. 9223372036854775807.",
                 "%ld. %ld.", &lng1, &lng2);
  tt_int_op(r,OP_EQ, 2);
  tt_int_op(lng1,OP_EQ, -9223372036854775807L - 1L);
  tt_int_op(lng2,OP_EQ, 9223372036854775807L);

  /* But INT64_MIN - 1 and INT64_MAX + 1 shouldn't work */
  r = tor_sscanf("-9223372036854775809.", "%ld.", &lng1);
  tt_int_op(r,OP_EQ, 0);

  r = tor_sscanf("9223372036854775808.", "%ld.", &lng1);
  tt_int_op(r,OP_EQ, 0);

  /* and the first failure stops further processing */
  r = tor_sscanf("-9223372036854775808. 9223372036854775808.",
                 "%ld. %ld.", &lng1, &lng2);
  tt_int_op(r,OP_EQ, 1);

  r = tor_sscanf("-9223372036854775809. 9223372036854775807.",
                 "%ld. %ld.", &lng1, &lng2);
  tt_int_op(r,OP_EQ, 0);

  r = tor_sscanf("9223372036854775808. -9223372036854775809.",
                 "%ld. %ld.", &lng1, &lng2);
  tt_int_op(r,OP_EQ, 0);
#endif /* SIZEOF_LONG == 4 || ... */

  r = tor_sscanf("123.456 .000007 -900123123.2000787 00003.2",
                 "%lf %lf %lf %lf", &d1,&d2,&d3,&d4);
  tt_int_op(r,OP_EQ, 4);
  test_feq(d1, 123.456);
  test_feq(d2, .000007);
  test_feq(d3, -900123123.2000787);
  test_feq(d4, 3.2);

  /* missing float */
  r = tor_sscanf("3 ", "%d %lf", &int1, &d1);
  tt_int_op(r, OP_EQ, 1);
  tt_int_op(int1, OP_EQ, 3);

  /* not a float */
  r = tor_sscanf("999 notafloat", "%d %lf", &int1, &d1);
  tt_int_op(r, OP_EQ, 1);
  tt_int_op(int1, OP_EQ, 999);

  /* %s but no buffer. */
  char *nullbuf = NULL;
  r = tor_sscanf("hello", "%3s", nullbuf);
  tt_int_op(r, OP_EQ, 0);

 done:
  tor_free(huge);
}

#define tt_char_op(a,op,b) tt_assert_op_type(a,op,b,char,"%c")
#define tt_ci_char_op(a,op,b) \
  tt_char_op(TOR_TOLOWER((int)a),op,TOR_TOLOWER((int)b))

#ifndef HAVE_STRNLEN
static size_t
strnlen(const char *s, size_t len)
{
  const char *p = memchr(s, 0, len);
  if (!p)
    return len;
  return p - s;
}
#endif /* !defined(HAVE_STRNLEN) */

static void
test_util_format_time_interval(void *arg)
{
  /* use the same sized buffer and integers as tor uses */
#define DBUF_SIZE 64
  char dbuf[DBUF_SIZE];
#define T_        "%ld"
  long sec, min, hour, day;

  /* we don't care about the exact spelling of the
   * second(s), minute(s), hour(s), day(s) labels */
#define LABEL_SIZE 21
#define L_        "%20s"
  char label_s[LABEL_SIZE];
  char label_m[LABEL_SIZE];
  char label_h[LABEL_SIZE];
  char label_d[LABEL_SIZE];

#define TL_       T_ " " L_

  int r;

  (void)arg;

  /* In these tests, we're not picky about
   * spelling or abbreviations */

  /* seconds: 0, 1, 9, 10, 59 */

  /* ignore exact spelling of "second(s)"*/
  format_time_interval(dbuf, sizeof(dbuf), 0);
  tt_int_op(strnlen(dbuf, DBUF_SIZE),OP_LE, DBUF_SIZE - 1);
  r = tor_sscanf(dbuf, TL_, &sec, label_s);
  tt_int_op(r,OP_EQ, 2);
  tt_ci_char_op(label_s[0],OP_EQ, 's');
  tt_int_op(sec,OP_EQ, 0);

  format_time_interval(dbuf, sizeof(dbuf), 1);
  tt_int_op(strnlen(dbuf, DBUF_SIZE),OP_LE, DBUF_SIZE - 1);
  r = tor_sscanf(dbuf, TL_, &sec, label_s);
  tt_int_op(r,OP_EQ, 2);
  tt_ci_char_op(label_s[0],OP_EQ, 's');
  tt_int_op(sec,OP_EQ, 1);

  format_time_interval(dbuf, sizeof(dbuf), 10);
  tt_int_op(strnlen(dbuf, DBUF_SIZE),OP_LE, DBUF_SIZE - 1);
  r = tor_sscanf(dbuf, TL_, &sec, label_s);
  tt_int_op(r,OP_EQ, 2);
  tt_ci_char_op(label_s[0],OP_EQ, 's');
  tt_int_op(sec,OP_EQ, 10);

  format_time_interval(dbuf, sizeof(dbuf), 59);
  tt_int_op(strnlen(dbuf, DBUF_SIZE),OP_LE, DBUF_SIZE - 1);
  r = tor_sscanf(dbuf, TL_, &sec, label_s);
  tt_int_op(r,OP_EQ, 2);
  tt_ci_char_op(label_s[0],OP_EQ, 's');
  tt_int_op(sec,OP_EQ, 59);

  /* negative seconds are reported as their absolute value */

  format_time_interval(dbuf, sizeof(dbuf), -4);
  tt_int_op(strnlen(dbuf, DBUF_SIZE),OP_LE, DBUF_SIZE - 1);
  r = tor_sscanf(dbuf, TL_, &sec, label_s);
  tt_int_op(r,OP_EQ, 2);
  tt_ci_char_op(label_s[0],OP_EQ, 's');
  tt_int_op(sec,OP_EQ, 4);
  tt_int_op(strnlen(dbuf, DBUF_SIZE),OP_LE, DBUF_SIZE - 1);

  format_time_interval(dbuf, sizeof(dbuf), -32);
  tt_int_op(strnlen(dbuf, DBUF_SIZE),OP_LE, DBUF_SIZE - 1);
  r = tor_sscanf(dbuf, TL_, &sec, label_s);
  tt_int_op(r,OP_EQ, 2);
  tt_ci_char_op(label_s[0],OP_EQ, 's');
  tt_int_op(sec,OP_EQ, 32);
  tt_int_op(strnlen(dbuf, DBUF_SIZE),OP_LE, DBUF_SIZE - 1);

  /* minutes: 1:00, 1:01, 1:59, 2:00, 2:01, 59:59 */

  /* ignore trailing "0 second(s)", if present */
  format_time_interval(dbuf, sizeof(dbuf), 60);
  tt_int_op(strnlen(dbuf, DBUF_SIZE),OP_LE, DBUF_SIZE - 1);
  r = tor_sscanf(dbuf, TL_, &min, label_m);
  tt_int_op(r,OP_EQ, 2);
  tt_ci_char_op(label_m[0],OP_EQ, 'm');
  tt_int_op(min,OP_EQ, 1);
  tt_int_op(strnlen(dbuf, DBUF_SIZE),OP_LE, DBUF_SIZE - 1);

  /* ignore exact spelling of "minute(s)," and "second(s)" */
  format_time_interval(dbuf, sizeof(dbuf), 60 + 1);
  tt_int_op(strnlen(dbuf, DBUF_SIZE),OP_LE, DBUF_SIZE - 1);
  r = tor_sscanf(dbuf, TL_ " " TL_,
                 &min, label_m, &sec, label_s);
  tt_int_op(r,OP_EQ, 4);
  tt_int_op(min,OP_EQ, 1);
  tt_ci_char_op(label_m[0],OP_EQ, 'm');
  tt_int_op(sec,OP_EQ, 1);
  tt_ci_char_op(label_s[0],OP_EQ, 's');
  tt_int_op(strnlen(dbuf, DBUF_SIZE),OP_LE, DBUF_SIZE - 1);

  format_time_interval(dbuf, sizeof(dbuf), 60*2 - 1);
  tt_int_op(strnlen(dbuf, DBUF_SIZE),OP_LE, DBUF_SIZE - 1);
  r = tor_sscanf(dbuf, TL_ " " TL_,
                 &min, label_m, &sec, label_s);
  tt_int_op(r,OP_EQ, 4);
  tt_int_op(min,OP_EQ, 1);
  tt_ci_char_op(label_m[0],OP_EQ, 'm');
  tt_int_op(sec,OP_EQ, 59);
  tt_ci_char_op(label_s[0],OP_EQ, 's');

  /* ignore trailing "0 second(s)", if present */
  format_time_interval(dbuf, sizeof(dbuf), 60*2);
  tt_int_op(strnlen(dbuf, DBUF_SIZE),OP_LE, DBUF_SIZE - 1);
  r = tor_sscanf(dbuf, TL_, &min, label_m);
  tt_int_op(r,OP_EQ, 2);
  tt_int_op(min,OP_EQ, 2);
  tt_ci_char_op(label_m[0],OP_EQ, 'm');

  /* ignore exact spelling of "minute(s)," and "second(s)" */
  format_time_interval(dbuf, sizeof(dbuf), 60*2 + 1);
  tt_int_op(strnlen(dbuf, DBUF_SIZE),OP_LE, DBUF_SIZE - 1);
  r = tor_sscanf(dbuf, TL_ " " TL_,
                 &min, label_m, &sec, label_s);
  tt_int_op(r,OP_EQ, 4);
  tt_int_op(min,OP_EQ, 2);
  tt_ci_char_op(label_m[0],OP_EQ, 'm');
  tt_int_op(sec,OP_EQ, 1);
  tt_ci_char_op(label_s[0],OP_EQ, 's');

  format_time_interval(dbuf, sizeof(dbuf), 60*60 - 1);
  tt_int_op(strnlen(dbuf, DBUF_SIZE),OP_LE, DBUF_SIZE - 1);
  r = tor_sscanf(dbuf, TL_ " " TL_,
                 &min, label_m, &sec, label_s);
  tt_int_op(r,OP_EQ, 4);
  tt_int_op(min,OP_EQ, 59);
  tt_ci_char_op(label_m[0],OP_EQ, 'm');
  tt_int_op(sec,OP_EQ, 59);
  tt_ci_char_op(label_s[0],OP_EQ, 's');

  /* negative minutes are reported as their absolute value */

  /* ignore trailing "0 second(s)", if present */
  format_time_interval(dbuf, sizeof(dbuf), -3*60);
  tt_int_op(strnlen(dbuf, DBUF_SIZE),OP_LE, DBUF_SIZE - 1);
  r = tor_sscanf(dbuf, TL_, &min, label_m);
  tt_int_op(r,OP_EQ, 2);
  tt_int_op(min,OP_EQ, 3);
  tt_ci_char_op(label_m[0],OP_EQ, 'm');

  /* ignore exact spelling of "minute(s)," and "second(s)" */
  format_time_interval(dbuf, sizeof(dbuf), -96);
  tt_int_op(strnlen(dbuf, DBUF_SIZE),OP_LE, DBUF_SIZE - 1);
  r = tor_sscanf(dbuf, TL_ " " TL_,
                 &min, label_m, &sec, label_s);
  tt_int_op(r,OP_EQ, 4);
  tt_int_op(min,OP_EQ, 1);
  tt_ci_char_op(label_m[0],OP_EQ, 'm');
  tt_int_op(sec,OP_EQ, 36);
  tt_ci_char_op(label_s[0],OP_EQ, 's');

  format_time_interval(dbuf, sizeof(dbuf), -2815);
  tt_int_op(strnlen(dbuf, DBUF_SIZE),OP_LE, DBUF_SIZE - 1);
  r = tor_sscanf(dbuf, TL_ " " TL_,
                 &min, label_m, &sec, label_s);
  tt_int_op(r,OP_EQ, 4);
  tt_int_op(min,OP_EQ, 46);
  tt_ci_char_op(label_m[0],OP_EQ, 'm');
  tt_int_op(sec,OP_EQ, 55);
  tt_ci_char_op(label_s[0],OP_EQ, 's');

  /* hours: 1:00, 1:00:01, 1:01, 23:59, 23:59:59 */
  /* always ignore trailing seconds, if present */

  /* ignore trailing "0 minute(s)" etc., if present */
  format_time_interval(dbuf, sizeof(dbuf), 60*60);
  tt_int_op(strnlen(dbuf, DBUF_SIZE),OP_LE, DBUF_SIZE - 1);
  r = tor_sscanf(dbuf, TL_, &hour, label_h);
  tt_int_op(r,OP_EQ, 2);
  tt_int_op(hour,OP_EQ, 1);
  tt_ci_char_op(label_h[0],OP_EQ, 'h');

  format_time_interval(dbuf, sizeof(dbuf), 60*60 + 1);
  tt_int_op(strnlen(dbuf, DBUF_SIZE),OP_LE, DBUF_SIZE - 1);
  r = tor_sscanf(dbuf, TL_, &hour, label_h);
  tt_int_op(r,OP_EQ, 2);
  tt_int_op(hour,OP_EQ, 1);
  tt_ci_char_op(label_h[0],OP_EQ, 'h');

  /* ignore exact spelling of "hour(s)," etc. */
  format_time_interval(dbuf, sizeof(dbuf), 60*60 + 60);
  tt_int_op(strnlen(dbuf, DBUF_SIZE),OP_LE, DBUF_SIZE - 1);
  r = tor_sscanf(dbuf, TL_ " " TL_,
                 &hour, label_h, &min, label_m);
  tt_int_op(r,OP_EQ, 4);
  tt_int_op(hour,OP_EQ, 1);
  tt_ci_char_op(label_h[0],OP_EQ, 'h');
  tt_int_op(min,OP_EQ, 1);
  tt_ci_char_op(label_m[0],OP_EQ, 'm');

  format_time_interval(dbuf, sizeof(dbuf), 24*60*60 - 60);
  tt_int_op(strnlen(dbuf, DBUF_SIZE),OP_LE, DBUF_SIZE - 1);
  r = tor_sscanf(dbuf, TL_ " " TL_,
                 &hour, label_h, &min, label_m);
  tt_int_op(r,OP_EQ, 4);
  tt_int_op(hour,OP_EQ, 23);
  tt_ci_char_op(label_h[0],OP_EQ, 'h');
  tt_int_op(min,OP_EQ, 59);
  tt_ci_char_op(label_m[0],OP_EQ, 'm');

  format_time_interval(dbuf, sizeof(dbuf), 24*60*60 - 1);
  tt_int_op(strnlen(dbuf, DBUF_SIZE),OP_LE, DBUF_SIZE - 1);
  r = tor_sscanf(dbuf, TL_ " " TL_,
                 &hour, label_h, &min, label_m);
  tt_int_op(r,OP_EQ, 4);
  tt_int_op(hour,OP_EQ, 23);
  tt_ci_char_op(label_h[0],OP_EQ, 'h');
  tt_int_op(min,OP_EQ, 59);
  tt_ci_char_op(label_m[0],OP_EQ, 'm');

  /* negative hours are reported as their absolute value */

  /* ignore exact spelling of "hour(s)," etc., if present */
  format_time_interval(dbuf, sizeof(dbuf), -2*60*60);
  tt_int_op(strnlen(dbuf, DBUF_SIZE),OP_LE, DBUF_SIZE - 1);
  r = tor_sscanf(dbuf, TL_, &hour, label_h);
  tt_int_op(r,OP_EQ, 2);
  tt_int_op(hour,OP_EQ, 2);
  tt_ci_char_op(label_h[0],OP_EQ, 'h');

  format_time_interval(dbuf, sizeof(dbuf), -75804);
  tt_int_op(strnlen(dbuf, DBUF_SIZE),OP_LE, DBUF_SIZE - 1);
  r = tor_sscanf(dbuf, TL_ " " TL_,
                 &hour, label_h, &min, label_m);
  tt_int_op(r,OP_EQ, 4);
  tt_int_op(hour,OP_EQ, 21);
  tt_ci_char_op(label_h[0],OP_EQ, 'h');
  tt_int_op(min,OP_EQ, 3);
  tt_ci_char_op(label_m[0],OP_EQ, 'm');

  /* days: 1:00, 1:00:00:01, 1:00:01, 1:01 */
  /* always ignore trailing seconds, if present */

  /* ignore trailing "0 hours(s)" etc., if present */
  format_time_interval(dbuf, sizeof(dbuf), 24*60*60);
  tt_int_op(strnlen(dbuf, DBUF_SIZE),OP_LE, DBUF_SIZE - 1);
  r = tor_sscanf(dbuf, TL_, &day, label_d);
  tt_int_op(r,OP_EQ, 2);
  tt_int_op(day,OP_EQ, 1);
  tt_ci_char_op(label_d[0],OP_EQ, 'd');

  format_time_interval(dbuf, sizeof(dbuf), 24*60*60 + 1);
  tt_int_op(strnlen(dbuf, DBUF_SIZE),OP_LE, DBUF_SIZE - 1);
  r = tor_sscanf(dbuf, TL_, &day, label_d);
  tt_int_op(r,OP_EQ, 2);
  tt_int_op(day,OP_EQ, 1);
  tt_ci_char_op(label_d[0],OP_EQ, 'd');

  /* ignore exact spelling of "days(s)," etc. */
  format_time_interval(dbuf, sizeof(dbuf), 24*60*60 + 60);
  tt_int_op(strnlen(dbuf, DBUF_SIZE),OP_LE, DBUF_SIZE - 1);
  r = tor_sscanf(dbuf, TL_ " " TL_ " " TL_,
                 &day, label_d, &hour, label_h, &min, label_m);
  if (r == -1) {
    /* ignore 0 hours(s), if present */
    r = tor_sscanf(dbuf, TL_ " " TL_,
                   &day, label_d, &min, label_m);
  }
  tt_assert(r == 4 || r == 6);
  tt_int_op(day,OP_EQ, 1);
  tt_ci_char_op(label_d[0],OP_EQ, 'd');
  if (r == 6) {
    tt_int_op(hour,OP_EQ, 0);
    tt_ci_char_op(label_h[0],OP_EQ, 'h');
  }
  tt_int_op(min,OP_EQ, 1);
  tt_ci_char_op(label_m[0],OP_EQ, 'm');

  /* ignore trailing "0 minutes(s)" etc., if present */
  format_time_interval(dbuf, sizeof(dbuf), 24*60*60 + 60*60);
  tt_int_op(strnlen(dbuf, DBUF_SIZE),OP_LE, DBUF_SIZE - 1);
  r = tor_sscanf(dbuf, TL_ " " TL_,
                 &day, label_d, &hour, label_h);
  tt_int_op(r,OP_EQ, 4);
  tt_int_op(day,OP_EQ, 1);
  tt_ci_char_op(label_d[0],OP_EQ, 'd');
  tt_int_op(hour,OP_EQ, 1);
  tt_ci_char_op(label_h[0],OP_EQ, 'h');

  /* negative days are reported as their absolute value */

  format_time_interval(dbuf, sizeof(dbuf), -21936184);
  tt_int_op(strnlen(dbuf, DBUF_SIZE),OP_LE, DBUF_SIZE - 1);
  r = tor_sscanf(dbuf, TL_ " " TL_ " " TL_,
                 &day, label_d, &hour, label_h, &min, label_m);
  tt_int_op(r,OP_EQ, 6);
  tt_int_op(day,OP_EQ, 253);
  tt_ci_char_op(label_d[0],OP_EQ, 'd');
  tt_int_op(hour,OP_EQ, 21);
  tt_ci_char_op(label_h[0],OP_EQ, 'h');
  tt_int_op(min,OP_EQ, 23);
  tt_ci_char_op(label_m[0],OP_EQ, 'm');

  /* periods > 1 year are reported in days (warn?) */

  /* ignore exact spelling of "days(s)," etc., if present */
  format_time_interval(dbuf, sizeof(dbuf), 758635154);
  tt_int_op(strnlen(dbuf, DBUF_SIZE),OP_LE, DBUF_SIZE - 1);
  r = tor_sscanf(dbuf, TL_ " " TL_ " " TL_,
                 &day, label_d, &hour, label_h, &min, label_m);
  tt_int_op(r,OP_EQ, 6);
  tt_int_op(day,OP_EQ, 8780);
  tt_ci_char_op(label_d[0],OP_EQ, 'd');
  tt_int_op(hour,OP_EQ, 11);
  tt_ci_char_op(label_h[0],OP_EQ, 'h');
  tt_int_op(min,OP_EQ, 59);
  tt_ci_char_op(label_m[0],OP_EQ, 'm');

  /* negative periods > 1 year are reported in days (warn?) */

  format_time_interval(dbuf, sizeof(dbuf), -1427014922);
  tt_int_op(strnlen(dbuf, DBUF_SIZE),OP_LE, DBUF_SIZE - 1);
  r = tor_sscanf(dbuf, TL_ " " TL_ " " TL_,
                 &day, label_d, &hour, label_h, &min, label_m);
  tt_int_op(r,OP_EQ, 6);
  tt_int_op(day,OP_EQ, 16516);
  tt_ci_char_op(label_d[0],OP_EQ, 'd');
  tt_int_op(hour,OP_EQ, 9);
  tt_ci_char_op(label_h[0],OP_EQ, 'h');
  tt_int_op(min,OP_EQ, 2);
  tt_ci_char_op(label_m[0],OP_EQ, 'm');

#if SIZEOF_LONG == 4 || SIZEOF_LONG == 8

  /* We can try INT32_MIN/MAX */
  /* Always ignore second(s) */

  /* INT32_MAX */
  format_time_interval(dbuf, sizeof(dbuf), 2147483647);
  tt_int_op(strnlen(dbuf, DBUF_SIZE),OP_LE, DBUF_SIZE - 1);
  r = tor_sscanf(dbuf, TL_ " " TL_ " " TL_,
                 &day, label_d, &hour, label_h, &min, label_m);
  tt_int_op(r,OP_EQ, 6);
  tt_int_op(day,OP_EQ, 24855);
  tt_ci_char_op(label_d[0],OP_EQ, 'd');
  tt_int_op(hour,OP_EQ, 3);
  tt_ci_char_op(label_h[0],OP_EQ, 'h');
  tt_int_op(min,OP_EQ, 14);
  tt_ci_char_op(label_m[0],OP_EQ, 'm');
  /* and 7 seconds - ignored */

  /* INT32_MIN: check that we get the absolute value of interval,
   * which doesn't actually fit in int32_t.
   * We expect INT32_MAX or INT32_MAX + 1 with 64 bit longs */
  format_time_interval(dbuf, sizeof(dbuf), -2147483647L - 1L);
  tt_int_op(strnlen(dbuf, DBUF_SIZE),OP_LE, DBUF_SIZE - 1);
  r = tor_sscanf(dbuf, TL_ " " TL_ " " TL_,
                 &day, label_d, &hour, label_h, &min, label_m);
  tt_int_op(r,OP_EQ, 6);
  tt_int_op(day,OP_EQ, 24855);
  tt_ci_char_op(label_d[0],OP_EQ, 'd');
  tt_int_op(hour,OP_EQ, 3);
  tt_ci_char_op(label_h[0],OP_EQ, 'h');
  tt_int_op(min,OP_EQ, 14);
  tt_ci_char_op(label_m[0],OP_EQ, 'm');
  /* and 7 or 8 seconds - ignored */

#endif /* SIZEOF_LONG == 4 || SIZEOF_LONG == 8 */

#if SIZEOF_LONG == 8

  /* We can try INT64_MIN/MAX */
  /* Always ignore second(s) */

  /* INT64_MAX */
  format_time_interval(dbuf, sizeof(dbuf), 9223372036854775807L);
  tt_int_op(strnlen(dbuf, DBUF_SIZE),OP_LE, DBUF_SIZE - 1);
  r = tor_sscanf(dbuf, TL_ " " TL_ " " TL_,
                 &day, label_d, &hour, label_h, &min, label_m);
  tt_int_op(r,OP_EQ, 6);
  tt_int_op(day,OP_EQ, 106751991167300L);
  tt_ci_char_op(label_d[0],OP_EQ, 'd');
  tt_int_op(hour,OP_EQ, 15);
  tt_ci_char_op(label_h[0],OP_EQ, 'h');
  tt_int_op(min,OP_EQ, 30);
  tt_ci_char_op(label_m[0],OP_EQ, 'm');
  /* and 7 seconds - ignored */

  /* INT64_MIN: check that we get the absolute value of interval,
   * which doesn't actually fit in int64_t.
   * We expect INT64_MAX */
  format_time_interval(dbuf, sizeof(dbuf),
                       -9223372036854775807L - 1L);
  tt_int_op(strnlen(dbuf, DBUF_SIZE),OP_LE, DBUF_SIZE - 1);
  r = tor_sscanf(dbuf, TL_ " " TL_ " " TL_,
                 &day, label_d, &hour, label_h, &min, label_m);
  tt_int_op(r,OP_EQ, 6);
  tt_int_op(day,OP_EQ, 106751991167300L);
  tt_ci_char_op(label_d[0],OP_EQ, 'd');
  tt_int_op(hour,OP_EQ, 15);
  tt_ci_char_op(label_h[0],OP_EQ, 'h');
  tt_int_op(min,OP_EQ, 30);
  tt_ci_char_op(label_m[0],OP_EQ, 'm');
  /* and 7 or 8 seconds - ignored */

#endif /* SIZEOF_LONG == 8 */

 done:
  ;
}

#undef tt_char_op
#undef tt_ci_char_op
#undef DBUF_SIZE
#undef T_
#undef LABEL_SIZE
#undef L_
#undef TL_

static void
test_util_path_is_relative(void *arg)
{
  /* OS-independent tests */
  (void)arg;
  tt_int_op(1,OP_EQ, path_is_relative(""));
  tt_int_op(1,OP_EQ, path_is_relative("dir"));
  tt_int_op(1,OP_EQ, path_is_relative("dir/"));
  tt_int_op(1,OP_EQ, path_is_relative("./dir"));
  tt_int_op(1,OP_EQ, path_is_relative("../dir"));

  tt_int_op(0,OP_EQ, path_is_relative("/"));
  tt_int_op(0,OP_EQ, path_is_relative("/dir"));
  tt_int_op(0,OP_EQ, path_is_relative("/dir/"));

  /* Windows */
#ifdef _WIN32
  /* I don't have Windows so I can't test this, hence the "#ifdef
     0". These are tests that look useful, so please try to get them
     running and uncomment if it all works as it should */
  tt_int_op(1,OP_EQ, path_is_relative("dir"));
  tt_int_op(1,OP_EQ, path_is_relative("dir\\"));
  tt_int_op(1,OP_EQ, path_is_relative("dir\\a:"));
  tt_int_op(1,OP_EQ, path_is_relative("dir\\a:\\"));
  tt_int_op(1,OP_EQ, path_is_relative("http:\\dir"));

  tt_int_op(0,OP_EQ, path_is_relative("\\dir"));
  tt_int_op(0,OP_EQ, path_is_relative("a:\\dir"));
  tt_int_op(0,OP_EQ, path_is_relative("z:\\dir"));
#endif /* defined(_WIN32) */

 done:
  ;
}

/** Run unittests for memory area allocator */
static void
test_util_memarea(void *arg)
{
  memarea_t *area = memarea_new();
  char *p1, *p2, *p3, *p1_orig;
  void *malloced_ptr = NULL;
  int i;

#ifdef DISABLE_MEMORY_SENTINELS
  /* If memory sentinels are disabled, this whole module is just an alias for
     malloc(), which is free to lay out memory most any way it wants. */
  if (1)
    tt_skip();
#endif /* defined(DISABLE_MEMORY_SENTINELS) */

  (void)arg;
  tt_assert(area);

  p1_orig = p1 = memarea_alloc(area,64);
  p2 = memarea_alloc_zero(area,52);
  p3 = memarea_alloc(area,11);

  tt_assert(memarea_owns_ptr(area, p1));
  tt_assert(memarea_owns_ptr(area, p2));
  tt_assert(memarea_owns_ptr(area, p3));
  /* Make sure we left enough space. */
  tt_assert(p1+64 <= p2);
  tt_assert(p2+52 <= p3);
  /* Make sure we aligned. */
  tt_int_op(((uintptr_t)p1) % sizeof(void*),OP_EQ, 0);
  tt_int_op(((uintptr_t)p2) % sizeof(void*),OP_EQ, 0);
  tt_int_op(((uintptr_t)p3) % sizeof(void*),OP_EQ, 0);
  tt_assert(!memarea_owns_ptr(area, p3+8192));
  tt_assert(!memarea_owns_ptr(area, p3+30));
  tt_assert(fast_mem_is_zero(p2, 52));
  /* Make sure we don't overalign. */
  p1 = memarea_alloc(area, 1);
  p2 = memarea_alloc(area, 1);
  tt_ptr_op(p1+sizeof(void*),OP_EQ, p2);
  {
    malloced_ptr = tor_malloc(64);
    tt_assert(!memarea_owns_ptr(area, malloced_ptr));
    tor_free(malloced_ptr);
  }

  /* memarea_memdup */
  {
    malloced_ptr = tor_malloc(64);
    crypto_rand((char*)malloced_ptr, 64);
    p1 = memarea_memdup(area, malloced_ptr, 64);
    tt_assert(p1 != malloced_ptr);
    tt_mem_op(p1,OP_EQ, malloced_ptr, 64);
    tor_free(malloced_ptr);
  }

  /* memarea_strdup. */
  p1 = memarea_strdup(area,"");
  p2 = memarea_strdup(area, "abcd");
  tt_assert(p1);
  tt_assert(p2);
  tt_str_op(p1,OP_EQ, "");
  tt_str_op(p2,OP_EQ, "abcd");

  /* memarea_strndup. */
  {
    const char *s = "Ad ogni porta batte la morte e grida: il nome!";
    /* (From Turandot, act 3.) */
    size_t len = strlen(s);
    p1 = memarea_strndup(area, s, 1000);
    p2 = memarea_strndup(area, s, 10);
    tt_str_op(p1,OP_EQ, s);
    tt_assert(p2 >= p1 + len + 1);
    tt_mem_op(s,OP_EQ, p2, 10);
    tt_int_op(p2[10],OP_EQ, '\0');
    p3 = memarea_strndup(area, s, len);
    tt_str_op(p3,OP_EQ, s);
    p3 = memarea_strndup(area, s, len-1);
    tt_mem_op(s,OP_EQ, p3, len-1);
    tt_int_op(p3[len-1],OP_EQ, '\0');
  }

  memarea_clear(area);
  p1 = memarea_alloc(area, 1);
  tt_ptr_op(p1,OP_EQ, p1_orig);
  memarea_clear(area);
  size_t total = 0, initial_allocation, allocation2, dummy;
  memarea_get_stats(area, &initial_allocation, &dummy);

  /* Check for running over an area's size. */
  for (i = 0; i < 4096; ++i) {
    size_t n = crypto_rand_int(6);
    p1 = memarea_alloc(area, n);
    total += n;
    tt_assert(memarea_owns_ptr(area, p1));
  }
  memarea_assert_ok(area);
  memarea_get_stats(area, &allocation2, &dummy);
  /* Make sure we can allocate a too-big object. */
  p1 = memarea_alloc_zero(area, 9000);
  p2 = memarea_alloc_zero(area, 16);
  total += 9000;
  total += 16;
  tt_assert(memarea_owns_ptr(area, p1));
  tt_assert(memarea_owns_ptr(area, p2));

  /* Now test stats... */
  size_t allocated = 0, used = 0;
  memarea_get_stats(area, &allocated, &used);
  tt_int_op(used, OP_LE, allocated);
  tt_int_op(used, OP_GE, total); /* not EQ, because of alignment and headers*/
  tt_int_op(allocated, OP_GT, allocation2);

  tt_int_op(allocation2, OP_GT, initial_allocation);

  memarea_clear(area);
  memarea_get_stats(area, &allocated, &used);
  tt_int_op(used, OP_LT, 128); /* Not 0, because of header */
  tt_int_op(allocated, OP_EQ, initial_allocation);

 done:
  memarea_drop_all(area);
  tor_free(malloced_ptr);
}

/** Run unit tests for utility functions to get file names relative to
 * the data directory. */
static void
test_util_datadir(void *arg)
{
  char buf[1024];
  char *f = NULL;
  char *temp_dir = NULL;

  (void)arg;
  temp_dir = get_datadir_fname(NULL);
  f = get_datadir_fname("state");
  tor_snprintf(buf, sizeof(buf), "%s"PATH_SEPARATOR"state", temp_dir);
  tt_str_op(f,OP_EQ, buf);
  tor_free(f);
  f = get_datadir_fname2("cache", "thingy");
  tor_snprintf(buf, sizeof(buf),
               "%s"PATH_SEPARATOR"cache"PATH_SEPARATOR"thingy", temp_dir);
  tt_str_op(f,OP_EQ, buf);
  tor_free(f);
  f = get_datadir_fname2_suffix("cache", "thingy", ".foo");
  tor_snprintf(buf, sizeof(buf),
               "%s"PATH_SEPARATOR"cache"PATH_SEPARATOR"thingy.foo", temp_dir);
  tt_str_op(f,OP_EQ, buf);
  tor_free(f);
  f = get_datadir_fname_suffix("cache", ".foo");
  tor_snprintf(buf, sizeof(buf), "%s"PATH_SEPARATOR"cache.foo",
               temp_dir);
  tt_str_op(f,OP_EQ, buf);

 done:
  tor_free(f);
  tor_free(temp_dir);
}

static void
test_util_strtok(void *arg)
{
  char buf[128];
  char buf2[128];
  int i;
  char *cp1, *cp2;

  (void)arg;
  for (i = 0; i < 3; i++) {
    const char *pad1="", *pad2="";
    switch (i) {
    case 0:
      break;
    case 1:
      pad1 = " ";
      pad2 = "!";
      break;
    case 2:
      pad1 = "  ";
      pad2 = ";!";
      break;
    }
    tor_snprintf(buf, sizeof(buf), "%s", pad1);
    tor_snprintf(buf2, sizeof(buf2), "%s", pad2);
    tt_ptr_op(tor_strtok_r_impl(buf, " ", &cp1), OP_EQ, NULL);
    tt_ptr_op(tor_strtok_r_impl(buf2, ".!..;!", &cp2), OP_EQ, NULL);

    tor_snprintf(buf, sizeof(buf),
                 "%sGraved on the dark  in gestures of descent%s", pad1, pad1);
    tor_snprintf(buf2, sizeof(buf2),
                "%sthey.seemed;;their!.own;most.perfect;monument%s",pad2,pad2);
    /*  -- "Year's End", Richard Wilbur */

    tt_str_op("Graved",OP_EQ, tor_strtok_r_impl(buf, " ", &cp1));
    tt_str_op("they",OP_EQ, tor_strtok_r_impl(buf2, ".!..;!", &cp2));
#define S1() tor_strtok_r_impl(NULL, " ", &cp1)
#define S2() tor_strtok_r_impl(NULL, ".!..;!", &cp2)
    tt_str_op("on",OP_EQ, S1());
    tt_str_op("the",OP_EQ, S1());
    tt_str_op("dark",OP_EQ, S1());
    tt_str_op("seemed",OP_EQ, S2());
    tt_str_op("their",OP_EQ, S2());
    tt_str_op("own",OP_EQ, S2());
    tt_str_op("in",OP_EQ, S1());
    tt_str_op("gestures",OP_EQ, S1());
    tt_str_op("of",OP_EQ, S1());
    tt_str_op("most",OP_EQ, S2());
    tt_str_op("perfect",OP_EQ, S2());
    tt_str_op("descent",OP_EQ, S1());
    tt_str_op("monument",OP_EQ, S2());
    tt_ptr_op(NULL,OP_EQ, S1());
    tt_ptr_op(NULL,OP_EQ, S2());
  }

  buf[0] = 0;
  tt_ptr_op(NULL,OP_EQ, tor_strtok_r_impl(buf, " ", &cp1));
  tt_ptr_op(NULL,OP_EQ, tor_strtok_r_impl(buf, "!", &cp1));

  strlcpy(buf, "Howdy!", sizeof(buf));
  tt_str_op("Howdy",OP_EQ, tor_strtok_r_impl(buf, "!", &cp1));
  tt_ptr_op(NULL,OP_EQ, tor_strtok_r_impl(NULL, "!", &cp1));

  strlcpy(buf, " ", sizeof(buf));
  tt_ptr_op(NULL,OP_EQ, tor_strtok_r_impl(buf, " ", &cp1));
  strlcpy(buf, "  ", sizeof(buf));
  tt_ptr_op(NULL,OP_EQ, tor_strtok_r_impl(buf, " ", &cp1));

  strlcpy(buf, "something  ", sizeof(buf));
  tt_str_op("something",OP_EQ, tor_strtok_r_impl(buf, " ", &cp1));
  tt_ptr_op(NULL,OP_EQ, tor_strtok_r_impl(NULL, ";", &cp1));
 done:
  ;
}

static void
test_util_find_str_at_start_of_line(void *ptr)
{
  const char *long_string =
    "howdy world. how are you? i hope it's fine.\n"
    "hello kitty\n"
    "third line";
  char *line2 = strchr(long_string,'\n')+1;
  char *line3 = strchr(line2,'\n')+1;
  const char *short_string = "hello kitty\n"
    "second line\n";
  char *short_line2 = strchr(short_string,'\n')+1;

  (void)ptr;

  tt_ptr_op(long_string,OP_EQ, find_str_at_start_of_line(long_string, ""));
  tt_ptr_op(NULL,OP_EQ, find_str_at_start_of_line(short_string, "nonsense"));
  tt_ptr_op(NULL,OP_EQ, find_str_at_start_of_line(long_string, "nonsense"));
  tt_ptr_op(NULL,OP_EQ, find_str_at_start_of_line(long_string, "\n"));
  tt_ptr_op(NULL,OP_EQ, find_str_at_start_of_line(long_string, "how "));
  tt_ptr_op(NULL,OP_EQ, find_str_at_start_of_line(long_string, "kitty"));
  tt_ptr_op(long_string,OP_EQ, find_str_at_start_of_line(long_string, "h"));
  tt_ptr_op(long_string,OP_EQ, find_str_at_start_of_line(long_string, "how"));
  tt_ptr_op(line2,OP_EQ, find_str_at_start_of_line(long_string, "he"));
  tt_ptr_op(line2,OP_EQ, find_str_at_start_of_line(long_string, "hell"));
  tt_ptr_op(line2,OP_EQ, find_str_at_start_of_line(long_string, "hello k"));
  tt_ptr_op(line2,OP_EQ,
            find_str_at_start_of_line(long_string, "hello kitty\n"));
  tt_ptr_op(line2,OP_EQ,
            find_str_at_start_of_line(long_string, "hello kitty\nt"));
  tt_ptr_op(line3,OP_EQ, find_str_at_start_of_line(long_string, "third"));
  tt_ptr_op(line3,OP_EQ, find_str_at_start_of_line(long_string, "third line"));
  tt_ptr_op(NULL, OP_EQ,
            find_str_at_start_of_line(long_string, "third line\n"));
  tt_ptr_op(short_line2,OP_EQ, find_str_at_start_of_line(short_string,
                                                     "second line\n"));
 done:
  ;
}

static void
test_util_tor_strreplacechar(void *ptr)
{
  (void)ptr;
  char empty[] = "";
  char not_contain[] = "bbb";
  char contains[] = "bab";
  char contains_all[] = "aaa";

  tor_strreplacechar(empty, 'a', 'b');
  tt_str_op(empty, OP_EQ, "");

  tor_strreplacechar(not_contain, 'a', 'b');
  tt_str_op(not_contain, OP_EQ, "bbb");

  tor_strreplacechar(contains, 'a', 'b');
  tt_str_op(contains, OP_EQ, "bbb");

  tor_strreplacechar(contains_all, 'a', 'b');
  tt_str_op(contains_all, OP_EQ, "bbb");

 done:
  ;
}

static void
test_util_string_is_C_identifier(void *ptr)
{
  (void)ptr;

  tt_int_op(1,OP_EQ, string_is_C_identifier("string_is_C_identifier"));
  tt_int_op(1,OP_EQ, string_is_C_identifier("_string_is_C_identifier"));
  tt_int_op(1,OP_EQ, string_is_C_identifier("_"));
  tt_int_op(1,OP_EQ, string_is_C_identifier("i"));
  tt_int_op(1,OP_EQ, string_is_C_identifier("_____"));
  tt_int_op(1,OP_EQ, string_is_C_identifier("__00__"));
  tt_int_op(1,OP_EQ, string_is_C_identifier("__init__"));
  tt_int_op(1,OP_EQ, string_is_C_identifier("_0"));
  tt_int_op(1,OP_EQ, string_is_C_identifier("_0string_is_C_identifier"));
  tt_int_op(1,OP_EQ, string_is_C_identifier("_0"));

  tt_int_op(0,OP_EQ, string_is_C_identifier("0_string_is_C_identifier"));
  tt_int_op(0,OP_EQ, string_is_C_identifier("0"));
  tt_int_op(0,OP_EQ, string_is_C_identifier(""));
  tt_int_op(0,OP_EQ, string_is_C_identifier(";"));
  tt_int_op(0,OP_EQ, string_is_C_identifier("i;"));
  tt_int_op(0,OP_EQ, string_is_C_identifier("_;"));
  tt_int_op(0,OP_EQ, string_is_C_identifier("í"));
  tt_int_op(0,OP_EQ, string_is_C_identifier("ñ"));

 done:
  ;
}

static void
test_util_string_is_utf8(void *ptr)
{
  (void)ptr;

  tt_int_op(1, OP_EQ, string_is_utf8(NULL, 0));
  tt_int_op(1, OP_EQ, string_is_utf8("", 1));
  tt_int_op(1, OP_EQ, string_is_utf8("\uFEFF", 3));
  tt_int_op(1, OP_EQ, string_is_utf8("\uFFFE", 3));
  tt_int_op(1, OP_EQ, string_is_utf8("ascii\x7f\n", 7));
  tt_int_op(1, OP_EQ, string_is_utf8("Risqu\u00e9=1", 9));

  /* Test the utf8_no_bom function */
  tt_int_op(0, OP_EQ, string_is_utf8_no_bom("\uFEFF", 3));
  tt_int_op(0, OP_EQ, string_is_utf8_no_bom("\uFFFE", 3));
  tt_int_op(0, OP_EQ, string_is_utf8_no_bom("\uFEFFlove", 7));
  tt_int_op(1, OP_EQ, string_is_utf8_no_bom("loveandrespect",
                                            strlen("loveandrespect")));

  // Validate exactly 'len' bytes.
  tt_int_op(0, OP_EQ, string_is_utf8("\0\x80", 2));
  tt_int_op(0, OP_EQ, string_is_utf8("Risqu\u00e9=1", 6));

  // Reject sequences with missing bytes.
  tt_int_op(0, OP_EQ, string_is_utf8("\x80", 1));
  tt_int_op(0, OP_EQ, string_is_utf8("\xc2", 1));
  tt_int_op(0, OP_EQ, string_is_utf8("\xc2 ", 2));
  tt_int_op(0, OP_EQ, string_is_utf8("\xe1\x80", 2));
  tt_int_op(0, OP_EQ, string_is_utf8("\xe1\x80 ", 3));
  tt_int_op(0, OP_EQ, string_is_utf8("\xf1\x80\x80", 3));
  tt_int_op(0, OP_EQ, string_is_utf8("\xf1\x80\x80 ", 4));

  // Reject encodings that are overly long.
  tt_int_op(0, OP_EQ, string_is_utf8("\xc1\xbf", 2));
  tt_int_op(1, OP_EQ, string_is_utf8("\xc2\x80", 2));
  tt_int_op(0, OP_EQ, string_is_utf8("\xe0\x9f\xbf", 3));
  tt_int_op(1, OP_EQ, string_is_utf8("\xe0\xa0\x80", 3));
  tt_int_op(0, OP_EQ, string_is_utf8("\xf0\x8f\xbf\xbf", 4));
  tt_int_op(1, OP_EQ, string_is_utf8("\xf0\x90\x80\x80", 4));

  // Reject UTF-16 surrogate halves.
  tt_int_op(1, OP_EQ, string_is_utf8("\xed\x9f\xbf", 3));
  tt_int_op(0, OP_EQ, string_is_utf8("\xed\xa0\x80", 3));
  tt_int_op(0, OP_EQ, string_is_utf8("\xed\xbf\xbf", 3));
  tt_int_op(1, OP_EQ, string_is_utf8("\xee\x80\x80", 3));

  // The minimum legal codepoint, 0x00.
  tt_int_op(1, OP_EQ, string_is_utf8("\0", 1));

  // The maximum legal codepoint, 0x10FFFF.
  tt_int_op(1, OP_EQ, string_is_utf8("\xf4\x8f\xbf\xbf", 4));
  tt_int_op(0, OP_EQ, string_is_utf8("\xf4\x90\x80\x80", 4));

  /* Test cases that vary between programming languages /
   * UTF-8 implementations.
   * Source: POC||GTFO 19, page 43
   * https://www.alchemistowl.org/pocorgtfo/
   */

  // Invalid (in most implementations)
  // surrogate
  tt_int_op(0, OP_EQ, string_is_utf8("\xed\xa0\x81", 3));
  // nullsurrog
  tt_int_op(0, OP_EQ, string_is_utf8("\x30\x00\xed\xa0\x81", 5));
  // threehigh
  tt_int_op(0, OP_EQ, string_is_utf8("\xed\xbf\xbf", 3));
  // fourhigh
  tt_int_op(0, OP_EQ, string_is_utf8("\xf4\x90\xbf\xbf", 4));
  // fivebyte
  tt_int_op(0, OP_EQ, string_is_utf8("\xfb\x80\x80\x80\x80", 5));
  // sixbyte
  tt_int_op(0, OP_EQ, string_is_utf8("\xfd\x80\x80\x80\x80", 5));
  // sixhigh
  tt_int_op(0, OP_EQ, string_is_utf8("\xfd\xbf\xbf\xbf\xbf", 5));

  // Valid (in most implementations)
  // fourbyte
  tt_int_op(1, OP_EQ, string_is_utf8("\xf0\x90\x8d\x88", 4));
  // fourbyte2
  tt_int_op(1, OP_EQ, string_is_utf8("\xf0\xbf\xbf\xbf", 4));
  // nullbyte
  tt_int_op(1, OP_EQ, string_is_utf8("\x30\x31\x32\x00\x33", 5));

 done:
  ;
}

static void
test_util_asprintf(void *ptr)
{
#define LOREMIPSUM                                              \
  "Lorem ipsum dolor sit amet, consectetur adipisicing elit"
  char *cp=NULL, *cp2=NULL;
  int r;
  (void)ptr;

  /* simple string */
  r = tor_asprintf(&cp, "simple string 100%% safe");
  tt_assert(cp);
  tt_str_op("simple string 100% safe",OP_EQ, cp);
  tt_int_op(strlen(cp),OP_EQ, r);
  tor_free(cp);

  /* empty string */
  r = tor_asprintf(&cp, "%s", "");
  tt_assert(cp);
  tt_str_op("",OP_EQ, cp);
  tt_int_op(strlen(cp),OP_EQ, r);
  tor_free(cp);

  /* numbers (%i) */
  r = tor_asprintf(&cp, "I like numbers-%2i, %i, etc.", -1, 2);
  tt_assert(cp);
  tt_str_op("I like numbers--1, 2, etc.",OP_EQ, cp);
  tt_int_op(strlen(cp),OP_EQ, r);
  /* don't free cp; next test uses it. */

  /* numbers (%d) */
  r = tor_asprintf(&cp2, "First=%d, Second=%d", 101, 202);
  tt_assert(cp2);
  tt_int_op(strlen(cp2),OP_EQ, r);
  tt_str_op("First=101, Second=202",OP_EQ, cp2);
  tt_assert(cp != cp2);
  tor_free(cp);
  tor_free(cp2);

  /* Glass-box test: a string exactly 128 characters long. */
  r = tor_asprintf(&cp, "Lorem1: %sLorem2: %s", LOREMIPSUM, LOREMIPSUM);
  tt_assert(cp);
  tt_int_op(128,OP_EQ, r);
  tt_int_op(cp[128], OP_EQ, '\0');
  tt_str_op("Lorem1: "LOREMIPSUM"Lorem2: "LOREMIPSUM,OP_EQ, cp);
  tor_free(cp);

  /* String longer than 128 characters */
  r = tor_asprintf(&cp, "1: %s 2: %s 3: %s",
                   LOREMIPSUM, LOREMIPSUM, LOREMIPSUM);
  tt_assert(cp);
  tt_int_op(strlen(cp),OP_EQ, r);
  tt_str_op("1: "LOREMIPSUM" 2: "LOREMIPSUM" 3: "LOREMIPSUM,OP_EQ, cp);

 done:
  tor_free(cp);
  tor_free(cp2);
}

static void
test_util_listdir(void *ptr)
{
  smartlist_t *dir_contents = NULL;
  char *fname1=NULL, *fname2=NULL, *fname3=NULL, *dir1=NULL, *dirname=NULL;
  int r;
  (void)ptr;

  fname1 = tor_strdup(get_fname("hopscotch"));
  fname2 = tor_strdup(get_fname("mumblety-peg"));
  fname3 = tor_strdup(get_fname(".hidden-file"));
  dir1   = tor_strdup(get_fname("some-directory"));
  dirname = tor_strdup(get_fname(NULL));

  tt_int_op(0,OP_EQ, write_str_to_file(fname1, "X\n", 0));
  tt_int_op(0,OP_EQ, write_str_to_file(fname2, "Y\n", 0));
  tt_int_op(0,OP_EQ, write_str_to_file(fname3, "Z\n", 0));
#ifdef _WIN32
  r = mkdir(dir1);
#else
  r = mkdir(dir1, 0700);
#endif
  if (r) {
    fprintf(stderr, "Can't create directory %s:", dir1);
    perror("");
    exit(1);
  }

  dir_contents = tor_listdir(dirname);
  tt_assert(dir_contents);
  /* make sure that each filename is listed. */
  tt_assert(smartlist_contains_string_case(dir_contents, "hopscotch"));
  tt_assert(smartlist_contains_string_case(dir_contents, "mumblety-peg"));
  tt_assert(smartlist_contains_string_case(dir_contents, ".hidden-file"));
  tt_assert(smartlist_contains_string_case(dir_contents, "some-directory"));

  tt_assert(!smartlist_contains_string(dir_contents, "."));
  tt_assert(!smartlist_contains_string(dir_contents, ".."));

 done:
  tor_free(fname1);
  tor_free(fname2);
  tor_free(fname3);
  tor_free(dir1);
  tor_free(dirname);
  if (dir_contents) {
    SMARTLIST_FOREACH(dir_contents, char *, cp, tor_free(cp));
    smartlist_free(dir_contents);
  }
}

static void
test_util_glob(void *ptr)
{
  (void)ptr;

#ifdef HAVE_GLOB
  smartlist_t *results = NULL;
  int r, i;
  char *dir1 = NULL, *dir2 = NULL, *forbidden = NULL, *dirname = NULL;
  char *expected = NULL, *pattern = NULL;
  // used for cleanup
  char *dir1_forbidden = NULL, *dir2_forbidden = NULL;
  char *forbidden_forbidden = NULL;

  dirname = tor_strdup(get_fname("test_glob"));
  tt_ptr_op(dirname, OP_NE, NULL);

#ifdef _WIN32
  r = mkdir(dirname);
#else
  r = mkdir(dirname, 0700);
#endif
  if (r) {
    fprintf(stderr, "Can't create directory %s:", dirname);
    perror("");
    exit(1);
  }

  tt_int_op(0, OP_EQ, create_test_directory_structure(dirname));
  tor_asprintf(&dir1, "%s"PATH_SEPARATOR"dir1", dirname);
  tor_asprintf(&dir1_forbidden,
               "%s"PATH_SEPARATOR"dir1"PATH_SEPARATOR"forbidden", dirname);
  tt_int_op(0, OP_EQ, create_test_directory_structure(dir1));
  tor_asprintf(&dir2, "%s"PATH_SEPARATOR"dir2", dirname);
  tor_asprintf(&dir2_forbidden,
               "%s"PATH_SEPARATOR"dir2"PATH_SEPARATOR"forbidden", dirname);
  tt_int_op(0, OP_EQ, create_test_directory_structure(dir2));
  tor_asprintf(&forbidden, "%s"PATH_SEPARATOR"forbidden", dirname);
  tor_asprintf(&forbidden_forbidden,
               "%s"PATH_SEPARATOR"forbidden"PATH_SEPARATOR"forbidden",dirname);
#ifndef _WIN32
  tt_int_op(0, OP_EQ, chmod(forbidden, 0700));
#endif
  tt_int_op(0, OP_EQ, create_test_directory_structure(forbidden));
#ifndef _WIN32
  tt_int_op(0, OP_EQ, chmod(forbidden, 0));
#endif

#define TEST(input) \
  do { \
    tor_asprintf(&pattern, "%s"PATH_SEPARATOR"%s", dirname, input); \
    results = tor_glob(pattern); \
    tor_free(pattern); \
    tt_assert(results); \
    smartlist_sort_strings(results); \
  } while (0);

#define EXPECT(result) \
  do { \
    tt_int_op(smartlist_len(results), OP_EQ, \
                            sizeof(result)/sizeof(*result)); \
    i = 0; \
    SMARTLIST_FOREACH_BEGIN(results, const char *, f) { \
      tor_asprintf(&expected, "%s"PATH_SEPARATOR"%s", dirname, result[i]); \
      tt_str_op(f, OP_EQ, expected); \
      i++; \
      tor_free(expected); \
    } SMARTLIST_FOREACH_END(f); \
    SMARTLIST_FOREACH(results, char *, f, tor_free(f)); \
    smartlist_free(results); \
  } while (0);

#define EXPECT_EMPTY() \
  do { \
    tt_int_op(smartlist_len(results), OP_EQ, 0); \
    SMARTLIST_FOREACH(results, char *, f, tor_free(f)); \
    smartlist_free(results); \
  } while (0);

  // wildcards at beginning
  const char *results_test1[] = {"dir2", "file2"};
  TEST("*2");
  EXPECT(results_test1);

  // wildcards at end
  const char *results_test2[] = {"dir1", "dir2"};
  TEST("d*");
  EXPECT(results_test2);

  // wildcards at beginning and end
#ifdef _WIN32
  // dot files are not ignored on Windows
  const char *results_test3[] = {".test-hidden", "dir1", "dir2", "file1",
                                 "file2", "forbidden"};
#else
  const char *results_test3[] = {"dir1", "dir2", "file1", "file2",
                                 "forbidden"};
#endif /* defined(_WIN32) */
  TEST("*i*");
  EXPECT(results_test3);

  // wildcards in middle
  const char *results_test4[] = {"dir1", "dir2"};
  TEST("d?r*");
  EXPECT(results_test4);

  // test file that does not exist
  TEST("not-exist");
  EXPECT_EMPTY();

  // test wildcard that matches nothing
  TEST("*not-exist*");
  EXPECT_EMPTY();

  // test path separator at end - no wildcards
  const char *results_test7[] = {"dir1"};
  TEST("dir1");
  EXPECT(results_test7);

  const char *results_test8[] = {"dir1"};
  TEST("dir1"PATH_SEPARATOR);
  EXPECT(results_test8);

  const char *results_test9[] = {"file1"};
  TEST("file1");
  EXPECT(results_test9);

  TEST("file1"PATH_SEPARATOR);
  EXPECT_EMPTY();

  // test path separator at end - with wildcards and linux path separator
  const char *results_test11[] = {"dir1", "dir2", "forbidden"};
  TEST("*/");
  EXPECT(results_test11);

#ifdef _WIN32
  // dot files are not ignored on Windows
  const char *results_test12[] = {".test-hidden", "dir1", "dir2", "empty",
                                  "file1", "file2", "forbidden"};
#else
  const char *results_test12[] = {"dir1", "dir2", "empty", "file1", "file2",
                                  "forbidden"};
#endif /* defined(_WIN32) */
  TEST("*");
  EXPECT(results_test12);

  // wildcards on folder and file and linux path separator
  const char *results_test13[] = {"dir1"PATH_SEPARATOR"dir1",
                                  "dir1"PATH_SEPARATOR"dir2",
                                  "dir1"PATH_SEPARATOR"file1",
                                  "dir1"PATH_SEPARATOR"file2",
                                  "dir2"PATH_SEPARATOR"dir1",
                                  "dir2"PATH_SEPARATOR"dir2",
                                  "dir2"PATH_SEPARATOR"file1",
                                  "dir2"PATH_SEPARATOR"file2"};
  TEST("?i*/?i*");
  EXPECT(results_test13);

  // wildcards on file only
  const char *results_test14[] = {"dir1"PATH_SEPARATOR"dir1",
                                  "dir1"PATH_SEPARATOR"dir2",
                                  "dir1"PATH_SEPARATOR"file1",
                                  "dir1"PATH_SEPARATOR"file2"};
  TEST("dir1"PATH_SEPARATOR"?i*");
  EXPECT(results_test14);

  // wildcards on folder only
  const char *results_test15[] = {"dir1"PATH_SEPARATOR"file1",
                                  "dir2"PATH_SEPARATOR"file1"};
  TEST("?i*"PATH_SEPARATOR"file1");
  EXPECT(results_test15);

  // wildcards after file name
  TEST("file1"PATH_SEPARATOR"*");
  EXPECT_EMPTY();

#ifndef _WIN32
  // test wildcard escaping
  TEST("\\*");
  EXPECT_EMPTY();

  if (getuid() != 0) {
    // test forbidden directory, if we're not root.
    // (Root will be able to see this directory anyway.)
    tor_asprintf(&pattern, "%s"PATH_SEPARATOR"*"PATH_SEPARATOR"*", dirname);
    results = tor_glob(pattern);
    tor_free(pattern);
    tt_assert(!results);
  }
#endif /* !defined(_WIN32) */

#undef TEST
#undef EXPECT
#undef EXPECT_EMPTY

 done:
#ifndef _WIN32
  (void) chmod(forbidden, 0700);
  (void) chmod(dir1_forbidden, 0700);
  (void) chmod(dir2_forbidden, 0700);
  (void) chmod(forbidden_forbidden, 0700);
#endif /* !defined(_WIN32) */
  tor_free(dir1);
  tor_free(dir2);
  tor_free(forbidden);
  tor_free(dirname);
  tor_free(dir1_forbidden);
  tor_free(dir2_forbidden);
  tor_free(forbidden_forbidden);
  tor_free(expected);
  tor_free(pattern);
  if (results) {
    SMARTLIST_FOREACH(results, char *, f, tor_free(f));
    smartlist_free(results);
  }
#else /* !defined(HAVE_GLOB) */
  tt_skip();
 done:
  return;
#endif /* defined(HAVE_GLOB) */
}

static void
test_util_get_glob_opened_files(void *ptr)
{
  (void)ptr;

#ifdef HAVE_GLOB
  smartlist_t *results = NULL;
  int r, i;
  char *dir1 = NULL, *dir2 = NULL, *forbidden = NULL, *dirname = NULL;
  char *expected = NULL, *pattern = NULL;
  // used for cleanup
  char *dir1_forbidden = NULL, *dir2_forbidden = NULL;
  char *forbidden_forbidden = NULL;

  dirname = tor_strdup(get_fname("test_get_glob_opened_files"));
  tt_ptr_op(dirname, OP_NE, NULL);

#ifdef _WIN32
  r = mkdir(dirname);
#else
  r = mkdir(dirname, 0700);
#endif
  if (r) {
    fprintf(stderr, "Can't create directory %s:", dirname);
    perror("");
    exit(1);
  }

  tt_int_op(0, OP_EQ, create_test_directory_structure(dirname));
  tor_asprintf(&dir1, "%s"PATH_SEPARATOR"dir1", dirname);
  tor_asprintf(&dir1_forbidden,
               "%s"PATH_SEPARATOR"dir1"PATH_SEPARATOR"forbidden", dirname);
  tt_int_op(0, OP_EQ, create_test_directory_structure(dir1));
  tor_asprintf(&dir2, "%s"PATH_SEPARATOR"dir2", dirname);
  tor_asprintf(&dir2_forbidden,
               "%s"PATH_SEPARATOR"dir2"PATH_SEPARATOR"forbidden", dirname);
  tt_int_op(0, OP_EQ, create_test_directory_structure(dir2));
  tor_asprintf(&forbidden, "%s"PATH_SEPARATOR"forbidden", dirname);
  tor_asprintf(&forbidden_forbidden,
               "%s"PATH_SEPARATOR"forbidden"PATH_SEPARATOR"forbidden",dirname);
#ifndef _WIN32
  chmod(forbidden, 0700);
#endif
  tt_int_op(0, OP_EQ, create_test_directory_structure(forbidden));
#ifndef _WIN32
  chmod(forbidden, 0);
#endif

#define TEST(input) \
  do { \
    if (*input) { \
      tor_asprintf(&pattern, "%s"PATH_SEPARATOR"%s", dirname, input); \
    } else { /* do not add path separator if empty string */ \
      tor_asprintf(&pattern, "%s", dirname); \
    } \
    results = get_glob_opened_files(pattern); \
    tor_free(pattern); \
    tt_assert(results); \
    smartlist_sort_strings(results); \
  } while (0);

#define EXPECT(result) \
  do { \
    tt_int_op(smartlist_len(results), OP_EQ, \
                          sizeof(result)/sizeof(*result)); \
    i = 0; \
    SMARTLIST_FOREACH_BEGIN(results, const char *, f) { \
      if (*result[i]) { \
        tor_asprintf(&expected, "%s"PATH_SEPARATOR"%s", dirname, result[i]); \
      } else { /* do not add path separator if empty string */ \
        tor_asprintf(&expected, "%s", dirname); \
      } \
      tt_str_op(f, OP_EQ, expected); \
      i++; \
      tor_free(expected); \
    } SMARTLIST_FOREACH_END(f); \
    SMARTLIST_FOREACH(results, char *, f, tor_free(f)); \
    smartlist_free(results); \
  } while (0);

#define EXPECT_EMPTY() \
  do { \
    tt_int_op(smartlist_len(results), OP_EQ, 0); \
    SMARTLIST_FOREACH(results, char *, f, tor_free(f)); \
    smartlist_free(results); \
  } while (0);

  // all files on folder
  const char *results_test1[] = {""}; // only the folder is read
  TEST("*");
  EXPECT(results_test1);

  // same as before but ending in path separator
  const char *results_test2[] = {""}; // only the folder is read
  TEST("*"PATH_SEPARATOR);
  EXPECT(results_test2);

  // wildcards in multiple path components
#ifndef _WIN32
  const char *results_test3[] = {"", "dir1", "dir2", "empty", "file1", "file2",
                                 "forbidden"};
#else
  // dot files are not special on windows
  const char *results_test3[] = {"", ".test-hidden", "dir1", "dir2", "empty",
                                 "file1", "file2", "forbidden"};
#endif /* !defined(_WIN32) */
  TEST("*"PATH_SEPARATOR"*");
  EXPECT(results_test3);

  // same as before but ending in path separator
#ifndef _WIN32
  const char *results_test4[] = {"", "dir1", "dir2", "empty", "file1", "file2",
                                 "forbidden"};
#else
  // dot files are not special on windows
  const char *results_test4[] = {"", ".test-hidden", "dir1", "dir2", "empty",
                                 "file1", "file2", "forbidden"};
#endif /* !defined(_WIN32) */
  TEST("*"PATH_SEPARATOR"*"PATH_SEPARATOR);
  EXPECT(results_test4);

  // no glob - folder
  TEST("");
  EXPECT_EMPTY();

  // same as before but ending in path separator
  TEST(PATH_SEPARATOR);
  EXPECT_EMPTY();

  // no glob - file
  TEST("file1");
  EXPECT_EMPTY();

  // same as before but ending in path separator and linux path separator
  TEST("file1/");
  EXPECT_EMPTY();

  // file but with wildcard after
  const char *results_test9[] = {"file1"};
  TEST("file1"PATH_SEPARATOR"*");
  EXPECT(results_test9);

  // dir inside dir and linux path separator
  TEST("dir1/dir1");
  EXPECT_EMPTY();

  // same as before but ending in path separator
  TEST("dir1"PATH_SEPARATOR"dir1"PATH_SEPARATOR);
  EXPECT_EMPTY();

  // no glob - empty
  TEST("empty");
  EXPECT_EMPTY();

  // same as before but ending in path separator
  TEST("empty"PATH_SEPARATOR);
  EXPECT_EMPTY();

  // no glob - does not exist
  TEST("not_exist");
  EXPECT_EMPTY();

#undef TEST
#undef EXPECT
#undef EXPECT_EMPTY

 done:
#ifndef _WIN32
  {
    int chmod_failed = 0;
    if (forbidden)
    chmod_failed |= chmod(forbidden, 0700);
    if (dir1_forbidden)
      chmod_failed |= chmod(dir1_forbidden, 0700);
    if (dir2_forbidden)
      chmod_failed |= chmod(dir2_forbidden, 0700);
    if (forbidden_forbidden)
      chmod_failed |= chmod(forbidden_forbidden, 0700);
    if (chmod_failed) {
      TT_FAIL(("unable to chmod a file on cleanup: %s", strerror(errno)));
    }
  }
#endif /* !defined(_WIN32) */
  tor_free(dir1);
  tor_free(dir2);
  tor_free(forbidden);
  tor_free(dirname);
  tor_free(dir1_forbidden);
  tor_free(dir2_forbidden);
  tor_free(forbidden_forbidden);
  tor_free(expected);
  tor_free(pattern);
  if (results) {
    SMARTLIST_FOREACH(results, char *, f, tor_free(f));
    smartlist_free(results);
  }
#else /* !defined(HAVE_GLOB) */
  tt_skip();
 done:
  return;
#endif /* defined(HAVE_GLOB) */
}

static void
test_util_parent_dir(void *ptr)
{
  char *cp;
  (void)ptr;

#define T(output,expect_ok,input)               \
  do {                                          \
    int ok;                                     \
    cp = tor_strdup(input);                     \
    ok = get_parent_directory(cp);              \
    tt_int_op(expect_ok, OP_EQ, ok);               \
    if (ok==0)                                  \
      tt_str_op(output, OP_EQ, cp);                \
    tor_free(cp);                               \
  } while (0);

  T("/home/wombat", 0, "/home/wombat/knish");
  T("/home/wombat", 0, "/home/wombat/knish/");
  T("/home/wombat", 0, "/home/wombat/knish///");
  T("./home/wombat", 0, "./home/wombat/knish/");
  T("/", 0, "/home");
  T("/", 0, "/home//");
  T(".", 0, "./wombat");
  T(".", 0, "./wombat/");
  T(".", 0, "./wombat//");
  T("wombat", 0, "wombat/foo");
  T("wombat/..", 0, "wombat/../foo");
  T("wombat/../", 0, "wombat/..//foo"); /* Is this correct? */
  T("wombat/.", 0, "wombat/./foo");
  T("wombat/./", 0, "wombat/.//foo"); /* Is this correct? */
  T("wombat", 0, "wombat/..//");
  T("wombat", 0, "wombat/foo/");
  T("wombat", 0, "wombat/.foo");
  T("wombat", 0, "wombat/.foo/");

  T("wombat", -1, "");
  T("w", -1, "");
  T("wombat", 0, "wombat/knish");

  T("/", 0, "/");
  T("/", 0, "////");

 done:
  tor_free(cp);
}

static void
test_util_ftruncate(void *ptr)
{
  char *buf = NULL;
  const char *fname;
  int fd = -1;
  const char *message = "Hello world";
  const char *message2 = "Hola mundo";
  struct stat st;

  (void) ptr;

  fname = get_fname("ftruncate");

  fd = tor_open_cloexec(fname, O_WRONLY|O_CREAT, 0600);
  tt_int_op(fd, OP_GE, 0);

  /* Make the file be there. */
  tt_int_op(strlen(message), OP_EQ,
            write_all_to_fd(fd, message, strlen(message)));
  tt_int_op((int)tor_fd_getpos(fd), OP_EQ, strlen(message));
  tt_int_op(0, OP_EQ, fstat(fd, &st));
  tt_int_op((int)st.st_size, OP_EQ, strlen(message));

  /* Truncate and see if it got truncated */
  tt_int_op(0, OP_EQ, tor_ftruncate(fd));
  tt_int_op((int)tor_fd_getpos(fd), OP_EQ, 0);
  tt_int_op(0, OP_EQ, fstat(fd, &st));
  tt_int_op((int)st.st_size, OP_EQ, 0);

  /* Replace, and see if it got replaced */
  tt_int_op(strlen(message2), OP_EQ,
            write_all_to_fd(fd, message2, strlen(message2)));
  tt_int_op((int)tor_fd_getpos(fd), OP_EQ, strlen(message2));
  tt_int_op(0, OP_EQ, fstat(fd, &st));
  tt_int_op((int)st.st_size, OP_EQ, strlen(message2));

  close(fd);
  fd = -1;

  buf = read_file_to_str(fname, 0, NULL);
  tt_str_op(message2, OP_EQ, buf);

 done:
  if (fd >= 0)
    close(fd);
  tor_free(buf);
}

static void
test_util_num_cpus(void *arg)
{
  (void)arg;
  int num = compute_num_cpus();
  if (num < 0)
    tt_skip();

  tt_int_op(num, OP_GE, 1);
  tt_int_op(num, OP_LE, 128);

 done:
  ;
}

#ifdef _WIN32
static void
test_util_load_win_lib(void *ptr)
{
  HANDLE h = load_windows_system_library(_T("advapi32.dll"));
  (void) ptr;

  tt_assert(h);
 done:
  if (h)
    FreeLibrary(h);
}
#endif /* defined(_WIN32) */

/**
 * Test for format_hex_number_sigsafe()
 */

static void
test_util_format_hex_number(void *ptr)
{
  int i, len;
  char buf[33];
  const struct {
    const char *str;
    unsigned int x;
  } test_data[] = {
    {"0", 0},
    {"1", 1},
    {"273A", 0x273a},
    {"FFFF", 0xffff},
    {"7FFFFFFF", 0x7fffffff},
    {"FFFFFFFF", 0xffffffff},
#if UINT_MAX >= 0xffffffff
    {"31BC421D", 0x31bc421d},
    {"FFFFFFFF", 0xffffffff},
#endif
    {NULL, 0}
  };

  (void)ptr;

  for (i = 0; test_data[i].str != NULL; ++i) {
    len = format_hex_number_sigsafe(test_data[i].x, buf, sizeof(buf));
    tt_int_op(len,OP_NE, 0);
    tt_int_op(len,OP_EQ, strlen(buf));
    tt_str_op(buf,OP_EQ, test_data[i].str);
  }

  tt_int_op(4,OP_EQ, format_hex_number_sigsafe(0xffff, buf, 5));
  tt_str_op(buf,OP_EQ, "FFFF");
  tt_int_op(0,OP_EQ, format_hex_number_sigsafe(0xffff, buf, 4));
  tt_int_op(0,OP_EQ, format_hex_number_sigsafe(0, buf, 1));

 done:
  return;
}

/**
 * Test for format_hex_number_sigsafe()
 */

static void
test_util_format_dec_number(void *ptr)
{
  int i, len;
  char buf[33];
  const struct {
    const char *str;
    unsigned int x;
  } test_data[] = {
    {"0", 0},
    {"1", 1},
    {"1234", 1234},
    {"12345678", 12345678},
    {"99999999",  99999999},
    {"100000000", 100000000},
    {"4294967295", 4294967295u},
#if UINT_MAX > 0xffffffff
    {"18446744073709551615", 18446744073709551615u },
#endif
    {NULL, 0}
  };

  (void)ptr;

  for (i = 0; test_data[i].str != NULL; ++i) {
    len = format_dec_number_sigsafe(test_data[i].x, buf, sizeof(buf));
    tt_int_op(len,OP_NE, 0);
    tt_int_op(len,OP_EQ, strlen(buf));
    tt_str_op(buf,OP_EQ, test_data[i].str);

    len = format_dec_number_sigsafe(test_data[i].x, buf,
                                    (int)(strlen(test_data[i].str) + 1));
    tt_int_op(len,OP_EQ, strlen(buf));
    tt_str_op(buf,OP_EQ, test_data[i].str);
  }

  tt_int_op(4,OP_EQ, format_dec_number_sigsafe(7331, buf, 5));
  tt_str_op(buf,OP_EQ, "7331");
  tt_int_op(0,OP_EQ, format_dec_number_sigsafe(7331, buf, 4));
  tt_int_op(1,OP_EQ, format_dec_number_sigsafe(0, buf, 2));
  tt_int_op(0,OP_EQ, format_dec_number_sigsafe(0, buf, 1));

 done:
  return;
}

#define MAX_SPLIT_LINE_COUNT 4
struct split_lines_test_t {
  const char *orig_line; // Line to be split (may contain \0's)
  int orig_length; // Length of orig_line
  const char *split_line[MAX_SPLIT_LINE_COUNT]; // Split lines
};

static void
test_util_di_ops(void *arg)
{
#define LT -1
#define GT 1
#define EQ 0
  const struct {
    const char *a; int want_sign; const char *b;
  } examples[] = {
    { "Foo", EQ, "Foo" },
    { "foo", GT, "bar", },
    { "foobar", EQ ,"foobar" },
    { "foobar", LT, "foobaw" },
    { "foobar", GT, "f00bar" },
    { "foobar", GT, "boobar" },
    { "", EQ, "" },
    { NULL, 0, NULL },
  };

  int i;

  (void)arg;
  for (i = 0; examples[i].a; ++i) {
    size_t len = strlen(examples[i].a);
    int eq1, eq2, neq1, neq2, cmp1, cmp2;
    tt_int_op(len,OP_EQ, strlen(examples[i].b));
    /* We do all of the operations, with operands in both orders. */
    eq1 = tor_memeq(examples[i].a, examples[i].b, len);
    eq2 = tor_memeq(examples[i].b, examples[i].a, len);
    neq1 = tor_memneq(examples[i].a, examples[i].b, len);
    neq2 = tor_memneq(examples[i].b, examples[i].a, len);
    cmp1 = tor_memcmp(examples[i].a, examples[i].b, len);
    cmp2 = tor_memcmp(examples[i].b, examples[i].a, len);

    /* Check for correctness of cmp1 */
    if (cmp1 < 0 && examples[i].want_sign != LT)
      TT_DIE(("Assertion failed."));
    else if (cmp1 > 0 && examples[i].want_sign != GT)
      TT_DIE(("Assertion failed."));
    else if (cmp1 == 0 && examples[i].want_sign != EQ)
      TT_DIE(("Assertion failed."));

    /* Check for consistency of everything else with cmp1 */
    tt_int_op(eq1,OP_EQ, eq2);
    tt_int_op(neq1,OP_EQ, neq2);
    tt_int_op(cmp1,OP_EQ, -cmp2);
    tt_int_op(eq1,OP_EQ, cmp1 == 0);
    tt_int_op(neq1,OP_EQ, !eq1);
  }

  {
    uint8_t zz = 0;
    uint8_t ii = 0;
    int z;

    /* exhaustively test tor_memeq and tor_memcmp
     * against each possible single-byte numeric difference
     * some arithmetic bugs only appear with certain bit patterns */
    for (z = 0; z < 256; z++) {
      for (i = 0; i < 256; i++) {
        ii = (uint8_t)i;
        zz = (uint8_t)z;
        tt_int_op(tor_memeq(&zz, &ii, 1),OP_EQ, zz == ii);
        tt_int_op(tor_memcmp(&zz, &ii, 1) > 0 ? GT : EQ,OP_EQ,
                  zz > ii ? GT : EQ);
        tt_int_op(tor_memcmp(&ii, &zz, 1) < 0 ? LT : EQ,OP_EQ,
                  ii < zz ? LT : EQ);
      }
    }
  }

  tt_int_op(1, OP_EQ, safe_mem_is_zero("", 0));
  tt_int_op(1, OP_EQ, safe_mem_is_zero("", 1));
  tt_int_op(0, OP_EQ, safe_mem_is_zero("a", 1));
  tt_int_op(0, OP_EQ, safe_mem_is_zero("a", 2));
  tt_int_op(0, OP_EQ, safe_mem_is_zero("\0a", 2));
  tt_int_op(1, OP_EQ, safe_mem_is_zero("\0\0a", 2));
  tt_int_op(1, OP_EQ, safe_mem_is_zero("\0\0\0\0\0\0\0\0", 8));
  tt_int_op(1, OP_EQ, safe_mem_is_zero("\0\0\0\0\0\0\0\0a", 8));
  tt_int_op(0, OP_EQ, safe_mem_is_zero("\0\0\0\0\0\0\0\0a", 9));

 done:
  ;
}

static void
test_util_memcpy_iftrue_timei(void *arg)
{
  (void)arg;
  char buf1[25];
  char buf2[25];
  char buf3[25];

  for (int i = 0; i < 100; ++i) {
    crypto_rand(buf1, sizeof(buf1));
    crypto_rand(buf2, sizeof(buf2));
    memcpy(buf3, buf1, sizeof(buf1));

    /* We just copied buf1 into buf3.  Now we're going to copy buf2 into buf2,
       iff our coin flip comes up heads. */
    bool coinflip = crypto_rand_int(2) == 0;

    memcpy_if_true_timei(coinflip, buf3, buf2, sizeof(buf3));

    if (coinflip) {
      tt_mem_op(buf3, OP_EQ, buf2, sizeof(buf2));
    } else {
      tt_mem_op(buf3, OP_EQ, buf1, sizeof(buf1));
    }
  }
 done:
  ;
}

static void
test_util_di_map(void *arg)
{
  (void)arg;
  di_digest256_map_t *dimap = NULL;
  uint8_t key1[] = "Robert Anton Wilson            ";
  uint8_t key2[] = "Martin Gardner, _Fads&fallacies";
  uint8_t key3[] = "Tom Lehrer, _Be Prepared_.     ";
  uint8_t key4[] = "Ursula Le Guin,_A Wizard of... ";

  char dflt_entry[] = "'You have made a good beginning', but no more";

  tt_int_op(32, OP_EQ, sizeof(key1));
  tt_int_op(32, OP_EQ, sizeof(key2));
  tt_int_op(32, OP_EQ, sizeof(key3));

  tt_ptr_op(dflt_entry, OP_EQ, dimap_search(dimap, key1, dflt_entry));

  char *str1 = tor_strdup("You are precisely as big as what you love"
                          " and precisely as small as what you allow"
                          " to annoy you.");
  char *str2 = tor_strdup("Let us hope that Lysenko's success in Russia will"
                          " serve for many generations to come as another"
                          " reminder to the world of how quickly and easily"
                          " a science can be corrupted when ignorant"
                          " political leaders deem themselves competent"
                          " to arbitrate scientific disputes");
  char *str3 = tor_strdup("Don't write naughty words on walls "
                          "if you can't spell.");

  dimap_add_entry(&dimap, key1, str1);
  dimap_add_entry(&dimap, key2, str2);
  dimap_add_entry(&dimap, key3, str3);

  tt_ptr_op(str1, OP_EQ, dimap_search(dimap, key1, dflt_entry));
  tt_ptr_op(str3, OP_EQ, dimap_search(dimap, key3, dflt_entry));
  tt_ptr_op(str2, OP_EQ, dimap_search(dimap, key2, dflt_entry));
  tt_ptr_op(dflt_entry, OP_EQ, dimap_search(dimap, key4, dflt_entry));

 done:
  dimap_free(dimap, tor_free_);
}

/**
 * Test counting high bits
 */
static void
test_util_n_bits_set(void *ptr)
{
  (void)ptr;
  tt_int_op(0,OP_EQ, n_bits_set_u8(0));
  tt_int_op(1,OP_EQ, n_bits_set_u8(1));
  tt_int_op(3,OP_EQ, n_bits_set_u8(7));
  tt_int_op(1,OP_EQ, n_bits_set_u8(8));
  tt_int_op(2,OP_EQ, n_bits_set_u8(129));
  tt_int_op(8,OP_EQ, n_bits_set_u8(255));
 done:
  ;
}

/**
 * Test LHS whitespace (and comment) eater
 */
static void
test_util_eat_whitespace(void *ptr)
{
  const char ws[] = { ' ', '\t', '\r' }; /* Except NL */
  char str[80];
  size_t i;

  (void)ptr;

  /* Try one leading ws */
  strlcpy(str, "fuubaar", sizeof(str));
  for (i = 0; i < sizeof(ws); ++i) {
    str[0] = ws[i];
    tt_ptr_op(str + 1,OP_EQ, eat_whitespace(str));
    tt_ptr_op(str + 1,OP_EQ, eat_whitespace_eos(str, str + strlen(str)));
    tt_ptr_op(str + 1,OP_EQ, eat_whitespace_no_nl(str));
    tt_ptr_op(str + 1,OP_EQ, eat_whitespace_eos_no_nl(str, str + strlen(str)));
  }
  str[0] = '\n';
  tt_ptr_op(str + 1,OP_EQ, eat_whitespace(str));
  tt_ptr_op(str + 1,OP_EQ, eat_whitespace_eos(str, str + strlen(str)));
  tt_ptr_op(str,OP_EQ,     eat_whitespace_no_nl(str));
  tt_ptr_op(str,OP_EQ,     eat_whitespace_eos_no_nl(str, str + strlen(str)));

  /* Empty string */
  strlcpy(str, "", sizeof(str));
  tt_ptr_op(str,OP_EQ, eat_whitespace(str));
  tt_ptr_op(str,OP_EQ, eat_whitespace_eos(str, str));
  tt_ptr_op(str,OP_EQ, eat_whitespace_no_nl(str));
  tt_ptr_op(str,OP_EQ, eat_whitespace_eos_no_nl(str, str));

  /* Only ws */
  strlcpy(str, " \t\r\n", sizeof(str));
  tt_ptr_op(str + strlen(str),OP_EQ, eat_whitespace(str));
  tt_ptr_op(str + strlen(str),OP_EQ,
            eat_whitespace_eos(str, str + strlen(str)));
  tt_ptr_op(str + strlen(str) - 1,OP_EQ,
              eat_whitespace_no_nl(str));
  tt_ptr_op(str + strlen(str) - 1,OP_EQ,
              eat_whitespace_eos_no_nl(str, str + strlen(str)));

  strlcpy(str, " \t\r ", sizeof(str));
  tt_ptr_op(str + strlen(str),OP_EQ, eat_whitespace(str));
  tt_ptr_op(str + strlen(str),OP_EQ,
              eat_whitespace_eos(str, str + strlen(str)));
  tt_ptr_op(str + strlen(str),OP_EQ, eat_whitespace_no_nl(str));
  tt_ptr_op(str + strlen(str),OP_EQ,
              eat_whitespace_eos_no_nl(str, str + strlen(str)));

  /* Multiple ws */
  strlcpy(str, "fuubaar", sizeof(str));
  for (i = 0; i < sizeof(ws); ++i)
    str[i] = ws[i];
  tt_ptr_op(str + sizeof(ws),OP_EQ, eat_whitespace(str));
  tt_ptr_op(str + sizeof(ws),OP_EQ,
            eat_whitespace_eos(str, str + strlen(str)));
  tt_ptr_op(str + sizeof(ws),OP_EQ, eat_whitespace_no_nl(str));
  tt_ptr_op(str + sizeof(ws),OP_EQ,
              eat_whitespace_eos_no_nl(str, str + strlen(str)));

  /* Eat comment */
  strlcpy(str, "# Comment \n No Comment", sizeof(str));
  tt_str_op("No Comment",OP_EQ, eat_whitespace(str));
  tt_str_op("No Comment",OP_EQ, eat_whitespace_eos(str, str + strlen(str)));
  tt_ptr_op(str,OP_EQ, eat_whitespace_no_nl(str));
  tt_ptr_op(str,OP_EQ, eat_whitespace_eos_no_nl(str, str + strlen(str)));

  /* Eat comment & ws mix */
  strlcpy(str, " # \t Comment \n\t\nNo Comment", sizeof(str));
  tt_str_op("No Comment",OP_EQ, eat_whitespace(str));
  tt_str_op("No Comment",OP_EQ, eat_whitespace_eos(str, str + strlen(str)));
  tt_ptr_op(str + 1,OP_EQ, eat_whitespace_no_nl(str));
  tt_ptr_op(str + 1,OP_EQ, eat_whitespace_eos_no_nl(str, str + strlen(str)));

  /* Eat entire comment */
  strlcpy(str, "#Comment", sizeof(str));
  tt_ptr_op(str + strlen(str),OP_EQ, eat_whitespace(str));
  tt_ptr_op(str + strlen(str),OP_EQ,
            eat_whitespace_eos(str, str + strlen(str)));
  tt_ptr_op(str,OP_EQ, eat_whitespace_no_nl(str));
  tt_ptr_op(str,OP_EQ, eat_whitespace_eos_no_nl(str, str + strlen(str)));

  /* Blank line, then comment */
  strlcpy(str, " \t\n # Comment", sizeof(str));
  tt_ptr_op(str + strlen(str),OP_EQ, eat_whitespace(str));
  tt_ptr_op(str + strlen(str),OP_EQ,
            eat_whitespace_eos(str, str + strlen(str)));
  tt_ptr_op(str + 2,OP_EQ, eat_whitespace_no_nl(str));
  tt_ptr_op(str + 2,OP_EQ, eat_whitespace_eos_no_nl(str, str + strlen(str)));

 done:
  ;
}

/** Return a newly allocated smartlist containing the lines of text in
 * <b>lines</b>.  The returned strings are heap-allocated, and must be
 * freed by the caller.
 *
 * XXXX? Move to container.[hc] ? */
static smartlist_t *
smartlist_new_from_text_lines(const char *lines)
{
  smartlist_t *sl = smartlist_new();
  char *last_line;

  smartlist_split_string(sl, lines, "\n", 0, 0);

  last_line = smartlist_pop_last(sl);
  if (last_line != NULL && *last_line != '\0') {
    smartlist_add(sl, last_line);
  } else {
    tor_free(last_line);
  }

  return sl;
}

/** Test smartlist_new_from_text_lines */
static void
test_util_sl_new_from_text_lines(void *ptr)
{
  (void)ptr;

  { /* Normal usage */
    smartlist_t *sl = smartlist_new_from_text_lines("foo\nbar\nbaz\n");
    int sl_len = smartlist_len(sl);

    tt_want_int_op(sl_len, OP_EQ, 3);

    if (sl_len > 0) tt_want_str_op(smartlist_get(sl, 0), OP_EQ, "foo");
    if (sl_len > 1) tt_want_str_op(smartlist_get(sl, 1), OP_EQ, "bar");
    if (sl_len > 2) tt_want_str_op(smartlist_get(sl, 2), OP_EQ, "baz");

    SMARTLIST_FOREACH(sl, void *, x, tor_free(x));
    smartlist_free(sl);
  }

  { /* No final newline */
    smartlist_t *sl = smartlist_new_from_text_lines("foo\nbar\nbaz");
    int sl_len = smartlist_len(sl);

    tt_want_int_op(sl_len, OP_EQ, 3);

    if (sl_len > 0) tt_want_str_op(smartlist_get(sl, 0), OP_EQ, "foo");
    if (sl_len > 1) tt_want_str_op(smartlist_get(sl, 1), OP_EQ, "bar");
    if (sl_len > 2) tt_want_str_op(smartlist_get(sl, 2), OP_EQ, "baz");

    SMARTLIST_FOREACH(sl, void *, x, tor_free(x));
    smartlist_free(sl);
  }

  { /* No newlines */
    smartlist_t *sl = smartlist_new_from_text_lines("foo");
    int sl_len = smartlist_len(sl);

    tt_want_int_op(sl_len, OP_EQ, 1);

    if (sl_len > 0) tt_want_str_op(smartlist_get(sl, 0), OP_EQ, "foo");

    SMARTLIST_FOREACH(sl, void *, x, tor_free(x));
    smartlist_free(sl);
  }

  { /* No text at all */
    smartlist_t *sl = smartlist_new_from_text_lines("");
    int sl_len = smartlist_len(sl);

    tt_want_int_op(sl_len, OP_EQ, 0);

    SMARTLIST_FOREACH(sl, void *, x, tor_free(x));
    smartlist_free(sl);
  }
}

static void
test_util_envnames(void *ptr)
{
  (void) ptr;

  tt_assert(environment_variable_names_equal("abc", "abc"));
  tt_assert(environment_variable_names_equal("abc", "abc="));
  tt_assert(environment_variable_names_equal("abc", "abc=def"));
  tt_assert(environment_variable_names_equal("abc=def", "abc"));
  tt_assert(environment_variable_names_equal("abc=def", "abc=ghi"));

  tt_assert(environment_variable_names_equal("abc", "abc"));
  tt_assert(environment_variable_names_equal("abc", "abc="));
  tt_assert(environment_variable_names_equal("abc", "abc=def"));
  tt_assert(environment_variable_names_equal("abc=def", "abc"));
  tt_assert(environment_variable_names_equal("abc=def", "abc=ghi"));

  tt_assert(!environment_variable_names_equal("abc", "abcd"));
  tt_assert(!environment_variable_names_equal("abc=", "abcd"));
  tt_assert(!environment_variable_names_equal("abc=", "abcd"));
  tt_assert(!environment_variable_names_equal("abc=", "def"));
  tt_assert(!environment_variable_names_equal("abc=", "def="));
  tt_assert(!environment_variable_names_equal("abc=x", "def=x"));

  tt_assert(!environment_variable_names_equal("", "a=def"));
  /* A bit surprising. */
  tt_assert(environment_variable_names_equal("", "=def"));
  tt_assert(environment_variable_names_equal("=y", "=x"));

 done:
  ;
}

/** Test process_environment_make */
static void
test_util_make_environment(void *ptr)
{
  const char *env_vars_string =
    "PATH=/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/bin:/usr/local/bin\n"
    "HOME=/home/foozer\n";
  const char expected_windows_env_block[] =
    "HOME=/home/foozer\000"
    "PATH=/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/bin:/usr/local/bin\000"
    "\000";
  size_t expected_windows_env_block_len =
    sizeof(expected_windows_env_block) - 1;

  smartlist_t *env_vars = smartlist_new_from_text_lines(env_vars_string);
  smartlist_t *env_vars_sorted = smartlist_new();
  smartlist_t *env_vars_in_unixoid_env_block_sorted = smartlist_new();

  process_environment_t *env;

  (void)ptr;

  env = process_environment_make(env_vars);

  /* Check that the Windows environment block is correct. */
  tt_want(tor_memeq(expected_windows_env_block, env->windows_environment_block,
                    expected_windows_env_block_len));

  /* Now for the Unixoid environment block.  We don't care which order
   * these environment variables are in, so we sort both lists first. */

  smartlist_add_all(env_vars_sorted, env_vars);

  {
    char **v;
    for (v = env->unixoid_environment_block; *v; ++v) {
      smartlist_add(env_vars_in_unixoid_env_block_sorted, *v);
    }
  }

  smartlist_sort_strings(env_vars_sorted);
  smartlist_sort_strings(env_vars_in_unixoid_env_block_sorted);

  tt_want_int_op(smartlist_len(env_vars_sorted), OP_EQ,
                 smartlist_len(env_vars_in_unixoid_env_block_sorted));
  {
    int len = smartlist_len(env_vars_sorted);
    int i;

    if (smartlist_len(env_vars_in_unixoid_env_block_sorted) < len) {
      len = smartlist_len(env_vars_in_unixoid_env_block_sorted);
    }

    for (i = 0; i < len; ++i) {
      tt_want_str_op(smartlist_get(env_vars_sorted, i), OP_EQ,
                     smartlist_get(env_vars_in_unixoid_env_block_sorted, i));
    }
  }

  /* Clean up. */
  smartlist_free(env_vars_in_unixoid_env_block_sorted);
  smartlist_free(env_vars_sorted);

  SMARTLIST_FOREACH(env_vars, char *, x, tor_free(x));
  smartlist_free(env_vars);

  process_environment_free(env);
}

/** Test set_environment_variable_in_smartlist */
static void
test_util_set_env_var_in_sl(void *ptr)
{
  /* The environment variables in these strings are in arbitrary
   * order; we sort the resulting lists before comparing them.
   *
   * (They *will not* end up in the order shown in
   * expected_resulting_env_vars_string.) */

  const char *base_env_vars_string =
    "PATH=/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/bin:/usr/local/bin\n"
    "HOME=/home/foozer\n"
    "TERM=xterm\n"
    "SHELL=/bin/ksh\n"
    "USER=foozer\n"
    "LOGNAME=foozer\n"
    "USERNAME=foozer\n"
    "LANG=en_US.utf8\n"
    ;

  const char *new_env_vars_string =
    "TERM=putty\n"
    "DISPLAY=:18.0\n"
    ;

  const char *expected_resulting_env_vars_string =
    "PATH=/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/bin:/usr/local/bin\n"
    "HOME=/home/foozer\n"
    "TERM=putty\n"
    "SHELL=/bin/ksh\n"
    "USER=foozer\n"
    "LOGNAME=foozer\n"
    "USERNAME=foozer\n"
    "LANG=en_US.utf8\n"
    "DISPLAY=:18.0\n"
    ;

  smartlist_t *merged_env_vars =
    smartlist_new_from_text_lines(base_env_vars_string);
  smartlist_t *new_env_vars =
    smartlist_new_from_text_lines(new_env_vars_string);
  smartlist_t *expected_resulting_env_vars =
    smartlist_new_from_text_lines(expected_resulting_env_vars_string);

  /* Elements of merged_env_vars are heap-allocated, and must be
   * freed.  Some of them are (or should) be freed by
   * set_environment_variable_in_smartlist.
   *
   * Elements of new_env_vars are heap-allocated, but are copied into
   * merged_env_vars, so they are not freed separately at the end of
   * the function.
   *
   * Elements of expected_resulting_env_vars are heap-allocated, and
   * must be freed. */

  (void)ptr;

  SMARTLIST_FOREACH(new_env_vars, char *, env_var,
                    set_environment_variable_in_smartlist(merged_env_vars,
                                                          env_var,
                                                          tor_free_,
                                                          1));

  smartlist_sort_strings(merged_env_vars);
  smartlist_sort_strings(expected_resulting_env_vars);

  tt_want_int_op(smartlist_len(merged_env_vars), OP_EQ,
                 smartlist_len(expected_resulting_env_vars));
  {
    int len = smartlist_len(merged_env_vars);
    int i;

    if (smartlist_len(expected_resulting_env_vars) < len) {
      len = smartlist_len(expected_resulting_env_vars);
    }

    for (i = 0; i < len; ++i) {
      tt_want_str_op(smartlist_get(merged_env_vars, i), OP_EQ,
                     smartlist_get(expected_resulting_env_vars, i));
    }
  }

  /* Clean up. */
  SMARTLIST_FOREACH(merged_env_vars, char *, x, tor_free(x));
  smartlist_free(merged_env_vars);

  smartlist_free(new_env_vars);

  SMARTLIST_FOREACH(expected_resulting_env_vars, char *, x, tor_free(x));
  smartlist_free(expected_resulting_env_vars);
}

static void
test_util_weak_random(void *arg)
{
  int i, j, n[16];
  tor_weak_rng_t rng;
  (void) arg;

  tor_init_weak_random(&rng, (unsigned)time(NULL));

  for (i = 1; i <= 256; ++i) {
    for (j=0;j<100;++j) {
      int r = tor_weak_random_range(&rng, i);
      tt_int_op(0, OP_LE, r);
      tt_int_op(r, OP_LT, i);
    }
  }

  memset(n,0,sizeof(n));
  for (j=0;j<8192;++j) {
    n[tor_weak_random_range(&rng, 16)]++;
  }

  for (i=0;i<16;++i)
    tt_int_op(n[i], OP_GT, 0);
 done:
  ;
}

static void
test_util_mathlog(void *arg)
{
  double d;
  (void) arg;

  d = tor_mathlog(2.718281828);
  tt_double_op(fabs(d - 1.0), OP_LT, .000001);
  d = tor_mathlog(10);
  tt_double_op(fabs(d - 2.30258509), OP_LT, .000001);
 done:
  ;
}

static void
test_util_fraction(void *arg)
{
  uint64_t a,b;
  (void)arg;

  a = 99; b = 30;
  simplify_fraction64(&a,&b);
  tt_u64_op(a, OP_EQ, 33);
  tt_u64_op(b, OP_EQ, 10);

  a = 3000000; b = 10000000;
  simplify_fraction64(&a,&b);
  tt_u64_op(a, OP_EQ, 3);
  tt_u64_op(b, OP_EQ, 10);

  a = 0; b = 15;
  simplify_fraction64(&a,&b);
  tt_u64_op(a, OP_EQ, 0);
  tt_u64_op(b, OP_EQ, 1);

 done:
  ;
}

static void
test_util_round_to_next_multiple_of(void *arg)
{
  (void)arg;

  tt_u64_op(round_uint64_to_next_multiple_of(0,1), OP_EQ, 0);
  tt_u64_op(round_uint64_to_next_multiple_of(0,7), OP_EQ, 0);

  tt_u64_op(round_uint64_to_next_multiple_of(99,1), OP_EQ, 99);
  tt_u64_op(round_uint64_to_next_multiple_of(99,7), OP_EQ, 105);
  tt_u64_op(round_uint64_to_next_multiple_of(99,9), OP_EQ, 99);

  tt_u64_op(round_uint64_to_next_multiple_of(UINT64_MAX,2), OP_EQ,
            UINT64_MAX);

  tt_int_op(round_uint32_to_next_multiple_of(0,1), OP_EQ, 0);
  tt_int_op(round_uint32_to_next_multiple_of(0,7), OP_EQ, 0);

  tt_int_op(round_uint32_to_next_multiple_of(99,1), OP_EQ, 99);
  tt_int_op(round_uint32_to_next_multiple_of(99,7), OP_EQ, 105);
  tt_int_op(round_uint32_to_next_multiple_of(99,9), OP_EQ, 99);

  tt_int_op(round_uint32_to_next_multiple_of(UINT32_MAX,2), OP_EQ,
            UINT32_MAX);

  tt_uint_op(round_to_next_multiple_of(0,1), OP_EQ, 0);
  tt_uint_op(round_to_next_multiple_of(0,7), OP_EQ, 0);

  tt_uint_op(round_to_next_multiple_of(99,1), OP_EQ, 99);
  tt_uint_op(round_to_next_multiple_of(99,7), OP_EQ, 105);
  tt_uint_op(round_to_next_multiple_of(99,9), OP_EQ, 99);

  tt_uint_op(round_to_next_multiple_of(UINT_MAX,2), OP_EQ,
            UINT_MAX);
 done:
  ;
}

static void
test_util_laplace(void *arg)
{
  /* Sample values produced using Python's SciPy:
   *
   * >>> from scipy.stats import laplace
   * >>> laplace.ppf([-0.01, 0.0, 0.01, 0.5, 0.51, 0.99, 1.0, 1.01],
     ...             loc = 24, scale = 24)
   * array([          nan,          -inf,  -69.88855213,   24.        ,
   *          24.48486498,  117.88855213,           inf,           nan])
   */
  const double mu = 24.0, b = 24.0;
  const double delta_f = 15.0, epsilon = 0.3; /* b = 15.0 / 0.3 = 50.0 */
  (void)arg;

  tt_i64_op(INT64_MIN, OP_EQ, sample_laplace_distribution(mu, b, 0.0));
  tt_i64_op(-69, OP_EQ, sample_laplace_distribution(mu, b, 0.01));
  tt_i64_op(24, OP_EQ, sample_laplace_distribution(mu, b, 0.5));
  tt_i64_op(24, OP_EQ, sample_laplace_distribution(mu, b, 0.51));
  tt_i64_op(117, OP_EQ, sample_laplace_distribution(mu, b, 0.99));

  /* >>> laplace.ppf([0.0, 0.1, 0.25, 0.5, 0.75, 0.9, 0.99],
   * ...             loc = 0, scale = 50)
   * array([         -inf,  -80.47189562,  -34.65735903,    0.        ,
   *          34.65735903,   80.47189562,  195.60115027])
   */
  tt_i64_op(INT64_MIN + 20, OP_EQ,
            add_laplace_noise(20, 0.0, delta_f, epsilon));

  tt_i64_op(-60, OP_EQ, add_laplace_noise(20, 0.1, delta_f, epsilon));
  tt_i64_op(-14, OP_EQ, add_laplace_noise(20, 0.25, delta_f, epsilon));
  tt_i64_op(20, OP_EQ, add_laplace_noise(20, 0.5, delta_f, epsilon));
  tt_i64_op(54, OP_EQ, add_laplace_noise(20, 0.75, delta_f, epsilon));
  tt_i64_op(100, OP_EQ, add_laplace_noise(20, 0.9, delta_f, epsilon));
  tt_i64_op(215, OP_EQ, add_laplace_noise(20, 0.99, delta_f, epsilon));

  /* Test extreme values of signal with maximally negative values of noise
   * 1.0000000000000002 is the smallest number > 1
   * 0.0000000000000002 is the double epsilon (error when calculating near 1)
   * this is approximately 1/(2^52)
   * per https://en.wikipedia.org/wiki/Double_precision
   * (let's not descend into the world of subnormals)
   * >>> laplace.ppf([0, 0.0000000000000002], loc = 0, scale = 1)
   * array([        -inf, -35.45506713])
   */
  const double noscale_df = 1.0, noscale_eps = 1.0;

  tt_i64_op(INT64_MIN, OP_EQ,
            add_laplace_noise(0, 0.0, noscale_df, noscale_eps));

  /* is it clipped to INT64_MIN? */
  tt_i64_op(INT64_MIN, OP_EQ,
            add_laplace_noise(-1, 0.0, noscale_df, noscale_eps));
  tt_i64_op(INT64_MIN, OP_EQ,
            add_laplace_noise(INT64_MIN, 0.0,
                              noscale_df, noscale_eps));
  /* ... even when scaled? */
  tt_i64_op(INT64_MIN, OP_EQ,
            add_laplace_noise(0, 0.0, delta_f, epsilon));
  tt_i64_op(INT64_MIN, OP_EQ,
            add_laplace_noise(0, 0.0,
                              DBL_MAX, 1));
  tt_i64_op(INT64_MIN, OP_EQ,
            add_laplace_noise(INT64_MIN, 0.0,
                              DBL_MAX, 1));

  /* does it play nice with INT64_MAX? */
  tt_i64_op((INT64_MIN + INT64_MAX), OP_EQ,
            add_laplace_noise(INT64_MAX, 0.0,
                              noscale_df, noscale_eps));

  /* do near-zero fractional values work? */
  const double min_dbl_error = 0.0000000000000002;

  tt_i64_op(-35, OP_EQ,
            add_laplace_noise(0, min_dbl_error,
                              noscale_df, noscale_eps));
  tt_i64_op(INT64_MIN, OP_EQ,
            add_laplace_noise(INT64_MIN, min_dbl_error,
                              noscale_df, noscale_eps));
  tt_i64_op((-35 + INT64_MAX), OP_EQ,
            add_laplace_noise(INT64_MAX, min_dbl_error,
                              noscale_df, noscale_eps));
  tt_i64_op(INT64_MIN, OP_EQ,
            add_laplace_noise(0, min_dbl_error,
                              DBL_MAX, 1));
  tt_i64_op((INT64_MAX + INT64_MIN), OP_EQ,
            add_laplace_noise(INT64_MAX, min_dbl_error,
                              DBL_MAX, 1));
  tt_i64_op(INT64_MIN, OP_EQ,
            add_laplace_noise(INT64_MIN, min_dbl_error,
                              DBL_MAX, 1));

  /* does it play nice with INT64_MAX? */
  tt_i64_op((INT64_MAX - 35), OP_EQ,
            add_laplace_noise(INT64_MAX, min_dbl_error,
                              noscale_df, noscale_eps));

  /* Test extreme values of signal with maximally positive values of noise
   * 1.0000000000000002 is the smallest number > 1
   * 0.9999999999999998 is the greatest number < 1 by calculation
   * per https://en.wikipedia.org/wiki/Double_precision
   * >>> laplace.ppf([1.0, 0.9999999999999998], loc = 0, scale = 1)
   * array([inf,  35.35050621])
   * but the function rejects p == 1.0, so we just use max_dbl_lt_one
   */
  const double max_dbl_lt_one = 0.9999999999999998;

  /* do near-one fractional values work? */
  tt_i64_op(35, OP_EQ,
            add_laplace_noise(0, max_dbl_lt_one, noscale_df, noscale_eps));

  /* is it clipped to INT64_MAX? */
  tt_i64_op(INT64_MAX, OP_EQ,
            add_laplace_noise(INT64_MAX - 35, max_dbl_lt_one,
                              noscale_df, noscale_eps));
  tt_i64_op(INT64_MAX, OP_EQ,
            add_laplace_noise(INT64_MAX - 34, max_dbl_lt_one,
                              noscale_df, noscale_eps));
  tt_i64_op(INT64_MAX, OP_EQ,
            add_laplace_noise(INT64_MAX, max_dbl_lt_one,
                              noscale_df, noscale_eps));
  /* ... even when scaled? */
  tt_i64_op(INT64_MAX, OP_EQ,
            add_laplace_noise(INT64_MAX, max_dbl_lt_one,
                              delta_f, epsilon));
  tt_i64_op((INT64_MIN + INT64_MAX), OP_EQ,
            add_laplace_noise(INT64_MIN, max_dbl_lt_one,
                              DBL_MAX, 1));
  tt_i64_op(INT64_MAX, OP_EQ,
            add_laplace_noise(INT64_MAX, max_dbl_lt_one,
                              DBL_MAX, 1));
  /* does it play nice with INT64_MIN? */
  tt_i64_op((INT64_MIN + 35), OP_EQ,
            add_laplace_noise(INT64_MIN, max_dbl_lt_one,
                              noscale_df, noscale_eps));

 done:
  ;
}

static void
test_util_clamp_double_to_int64(void *arg)
{
  (void)arg;

  tt_i64_op(INT64_MIN, OP_EQ, clamp_double_to_int64(-INFINITY_DBL));
  tt_i64_op(INT64_MIN, OP_EQ,
            clamp_double_to_int64(-1.0 * pow(2.0, 64.0) - 1.0));
  tt_i64_op(INT64_MIN, OP_EQ,
            clamp_double_to_int64(-1.0 * pow(2.0, 63.0) - 1.0));
  tt_i64_op(((uint64_t) -1) << 53, OP_EQ,
            clamp_double_to_int64(-1.0 * pow(2.0, 53.0)));
  tt_i64_op((((uint64_t) -1) << 53) + 1, OP_EQ,
            clamp_double_to_int64(-1.0 * pow(2.0, 53.0) + 1.0));
  tt_i64_op(-1, OP_EQ, clamp_double_to_int64(-1.0));
  tt_i64_op(0, OP_EQ, clamp_double_to_int64(-0.9));
  tt_i64_op(0, OP_EQ, clamp_double_to_int64(-0.1));
  tt_i64_op(0, OP_EQ, clamp_double_to_int64(0.0));
  tt_i64_op(0, OP_EQ, clamp_double_to_int64(NAN_DBL));
  tt_i64_op(0, OP_EQ, clamp_double_to_int64(0.1));
  tt_i64_op(0, OP_EQ, clamp_double_to_int64(0.9));
  tt_i64_op(1, OP_EQ, clamp_double_to_int64(1.0));
  tt_i64_op((((int64_t) 1) << 53) - 1, OP_EQ,
            clamp_double_to_int64(pow(2.0, 53.0) - 1.0));
  tt_i64_op(((int64_t) 1) << 53, OP_EQ,
            clamp_double_to_int64(pow(2.0, 53.0)));
  tt_i64_op(INT64_MAX, OP_EQ,
            clamp_double_to_int64(pow(2.0, 63.0)));
  tt_i64_op(INT64_MAX, OP_EQ,
            clamp_double_to_int64(pow(2.0, 64.0)));
  tt_i64_op(INT64_MAX, OP_EQ, clamp_double_to_int64(INFINITY_DBL));

 done:
  ;
}

#ifdef FD_CLOEXEC
#define CAN_CHECK_CLOEXEC
static int
fd_is_cloexec(tor_socket_t fd)
{
  int flags = fcntl(fd, F_GETFD, 0);
  return (flags & FD_CLOEXEC) != 0;
}
#endif /* defined(FD_CLOEXEC) */

#ifndef _WIN32
#define CAN_CHECK_NONBLOCK
static int
fd_is_nonblocking(tor_socket_t fd)
{
  int flags = fcntl(fd, F_GETFL, 0);
  return (flags & O_NONBLOCK) != 0;
}
#endif /* !defined(_WIN32) */

#define ERRNO_IS_EPROTO(e)    (e == SOCK_ERRNO(EPROTONOSUPPORT))
#define SOCK_ERR_IS_EPROTO(s) ERRNO_IS_EPROTO(tor_socket_errno(s))

/* Test for tor_open_socket*, using IPv4 or IPv6 depending on arg. */
static void
test_util_socket(void *arg)
{
  const int domain = !strcmp(arg, "4") ? AF_INET : AF_INET6;
  tor_socket_t fd1 = TOR_INVALID_SOCKET;
  tor_socket_t fd2 = TOR_INVALID_SOCKET;
  tor_socket_t fd3 = TOR_INVALID_SOCKET;
  tor_socket_t fd4 = TOR_INVALID_SOCKET;
  int n = get_n_open_sockets();

  TT_BLATHER(("Starting with %d open sockets.", n));

  (void)arg;

  fd1 = tor_open_socket_with_extensions(domain, SOCK_STREAM, 0, 0, 0);
  int err = tor_socket_errno(fd1);
  if (fd1 < 0 && (err == SOCK_ERRNO(EPROTONOSUPPORT) ||
                  err == SOCK_ERRNO(EAFNOSUPPORT))) {
    /* Assume we're on an IPv4-only or IPv6-only system, and give up now. */
    goto done;
  }
  fd2 = tor_open_socket_with_extensions(domain, SOCK_STREAM, 0, 0, 1);
  tt_assert(SOCKET_OK(fd1));
  tt_assert(SOCKET_OK(fd2));
  tt_int_op(get_n_open_sockets(), OP_EQ, n + 2);
  //fd3 = tor_open_socket_with_extensions(domain, SOCK_STREAM, 0, 1, 0);
  //fd4 = tor_open_socket_with_extensions(domain, SOCK_STREAM, 0, 1, 1);
  fd3 = tor_open_socket(domain, SOCK_STREAM, 0);
  fd4 = tor_open_socket_nonblocking(domain, SOCK_STREAM, 0);
  tt_assert(SOCKET_OK(fd3));
  tt_assert(SOCKET_OK(fd4));
  tt_int_op(get_n_open_sockets(), OP_EQ, n + 4);

#ifdef CAN_CHECK_CLOEXEC
  tt_int_op(fd_is_cloexec(fd1), OP_EQ, 0);
  tt_int_op(fd_is_cloexec(fd2), OP_EQ, 0);
  tt_int_op(fd_is_cloexec(fd3), OP_EQ, 1);
  tt_int_op(fd_is_cloexec(fd4), OP_EQ, 1);
#endif /* defined(CAN_CHECK_CLOEXEC) */
#ifdef CAN_CHECK_NONBLOCK
  tt_int_op(fd_is_nonblocking(fd1), OP_EQ, 0);
  tt_int_op(fd_is_nonblocking(fd2), OP_EQ, 1);
  tt_int_op(fd_is_nonblocking(fd3), OP_EQ, 0);
  tt_int_op(fd_is_nonblocking(fd4), OP_EQ, 1);
#endif /* defined(CAN_CHECK_NONBLOCK) */

  tor_assert(tor_close_socket == tor_close_socket__real);

  /* we use close_socket__real here so that coverity can tell that we are
   * really closing these sockets. */
  tor_close_socket__real(fd1);
  tor_close_socket__real(fd2);
  fd1 = fd2 = TOR_INVALID_SOCKET;
  tt_int_op(get_n_open_sockets(), OP_EQ, n + 2);
  tor_close_socket__real(fd3);
  tor_close_socket__real(fd4);
  fd3 = fd4 = TOR_INVALID_SOCKET;
  tt_int_op(get_n_open_sockets(), OP_EQ, n);

 done:
  if (SOCKET_OK(fd1))
    tor_close_socket__real(fd1);
  if (SOCKET_OK(fd2))
    tor_close_socket__real(fd2);
  if (SOCKET_OK(fd3))
    tor_close_socket__real(fd3);
  if (SOCKET_OK(fd4))
    tor_close_socket__real(fd4);
}

#if 0
static int
is_there_a_localhost(int family)
{
  tor_socket_t s;
  s = tor_open_socket(family, SOCK_STREAM, IPPROTO_TCP);
  tor_assert(SOCKET_OK(s));

  int result = 0;
  if (family == AF_INET) {
    struct sockaddr_in s_in;
    memset(&s_in, 0, sizeof(s_in));
    s_in.sin_family = AF_INET;
    s_in.sin_addr.s_addr = htonl(0x7f000001);
    s_in.sin_port = 0;

    if (bind(s, (void*)&s_in, sizeof(s_in)) == 0) {
      result = 1;
    }
  } else if (family == AF_INET6) {
    struct sockaddr_in6 sin6;
    memset(&sin6, 0, sizeof(sin6));
    sin6.sin6_family = AF_INET6;
    sin6.sin6_addr.s6_addr[15] = 1;
    sin6.sin6_port = 0;
  }
  tor_close_socket(s);

  return result;
}
#endif /* 0 */

/* Test for socketpair and ersatz_socketpair().  We test them both, since
 * the latter is a tolerably good way to exercise tor_accept_socket(). */
static void
test_util_socketpair(void *arg)
{
  const int ersatz = !strcmp(arg, "1");
  int (*const tor_socketpair_fn)(int, int, int, tor_socket_t[2]) =
    ersatz ? tor_ersatz_socketpair : tor_socketpair;
  int n = get_n_open_sockets();
  tor_socket_t fds[2] = {TOR_INVALID_SOCKET, TOR_INVALID_SOCKET};
  const int family = AF_UNIX;
  int socketpair_result = 0;

  socketpair_result = tor_socketpair_fn(family, SOCK_STREAM, 0, fds);

#ifdef __FreeBSD__
  /* If there is no 127.0.0.1, tor_ersatz_socketpair will and must fail.
   * Otherwise, we risk exposing a socketpair on a routable IP address. (Some
   * BSD jails use a routable address for localhost. Fortunately, they have
   * the real AF_UNIX socketpair.) */
  if (ersatz && socketpair_result < 0) {
    /* In my testing, an IPv6-only FreeBSD jail without ::1 returned EINVAL.
     * Assume we're on a machine without 127.0.0.1 or ::1 and give up now. */
    tt_skip();
  }
#endif /* defined(__FreeBSD__) */
#ifdef ENETUNREACH
  if (ersatz && socketpair_result == -ENETUNREACH) {
    /* We can also fail with -ENETUNREACH if we have no network stack at
     * all. */
    tt_skip();
  }
#endif /* defined(ENETUNREACH) */
  tt_int_op(0, OP_EQ, socketpair_result);

  tt_assert(SOCKET_OK(fds[0]));
  tt_assert(SOCKET_OK(fds[1]));
  if (ersatz)
    tt_int_op(get_n_open_sockets(), OP_EQ, n);
  else
    tt_int_op(get_n_open_sockets(), OP_EQ, n + 2);
#ifdef CAN_CHECK_CLOEXEC
  tt_int_op(fd_is_cloexec(fds[0]), OP_EQ, !ersatz);
  tt_int_op(fd_is_cloexec(fds[1]), OP_EQ, !ersatz);
#endif
#ifdef CAN_CHECK_NONBLOCK
  tt_int_op(fd_is_nonblocking(fds[0]), OP_EQ, 0);
  tt_int_op(fd_is_nonblocking(fds[1]), OP_EQ, 0);
#endif

 done:
  if (ersatz) {
    if (SOCKET_OK(fds[0]))
      tor_close_socket_simple(fds[0]);
    if (SOCKET_OK(fds[1]))
      tor_close_socket_simple(fds[1]);
  } else {
    if (SOCKET_OK(fds[0]))
      tor_close_socket(fds[0]);
    if (SOCKET_OK(fds[1]))
      tor_close_socket(fds[1]);
  }
}

#undef SOCKET_EPROTO

static void
test_util_max_mem(void *arg)
{
  size_t memory1, memory2;
  int r, r2;
  (void) arg;

  r = get_total_system_memory(&memory1);
  r2 = get_total_system_memory(&memory2);
  tt_int_op(r, OP_EQ, r2);
  tt_uint_op(memory2, OP_EQ, memory1);

  TT_BLATHER(("System memory: %"TOR_PRIuSZ, (memory1)));

  if (r==0) {
    /* You have at least a megabyte. */
    tt_uint_op(memory1, OP_GT, (1<<20));
  } else {
    /* You do not have a petabyte. */
#if SIZEOF_SIZE_T >= 8
    tt_u64_op(memory1, OP_LT, (UINT64_C(1)<<50));
#endif
  }

 done:
  ;
}

static void
test_util_dest_validation_edgecase(void *arg)
{
  (void)arg;

  tt_assert(!string_is_valid_dest(NULL));
  tt_assert(!string_is_valid_dest(""));

  done:
  return;
}

static void
test_util_hostname_validation(void *arg)
{
  (void)arg;

  // Lets try valid hostnames first.
  tt_assert(string_is_valid_nonrfc_hostname("torproject.org"));
  tt_assert(string_is_valid_nonrfc_hostname("ocw.mit.edu"));
  tt_assert(string_is_valid_nonrfc_hostname("i.4cdn.org"));
  tt_assert(string_is_valid_nonrfc_hostname("stanford.edu"));
  tt_assert(string_is_valid_nonrfc_hostname("multiple-words-with-hypens.jp"));

  // Subdomain name cannot start with '-' or '_'.
  tt_assert(!string_is_valid_nonrfc_hostname("-torproject.org"));
  tt_assert(!string_is_valid_nonrfc_hostname("subdomain.-domain.org"));
  tt_assert(!string_is_valid_nonrfc_hostname("-subdomain.domain.org"));
  tt_assert(!string_is_valid_nonrfc_hostname("___abc.org"));

  // Hostnames cannot contain non-alphanumeric characters.
  tt_assert(!string_is_valid_nonrfc_hostname("%%domain.\\org."));
  tt_assert(!string_is_valid_nonrfc_hostname("***x.net"));
  tt_assert(!string_is_valid_nonrfc_hostname("\xff\xffxyz.org"));
  tt_assert(!string_is_valid_nonrfc_hostname("word1 word2.net"));

  // Test workaround for nytimes.com stupidity, technically invalid,
  // but we allow it since they are big, even though they are failing to
  // comply with a ~30 year old standard.
  tt_assert(string_is_valid_nonrfc_hostname("core3_euw1.fabrik.nytimes.com"));

  // Firefox passes FQDNs with trailing '.'s  directly to the SOCKS proxy,
  // which is redundant since the spec states DOMAINNAME addresses are fully
  // qualified.  While unusual, this should be tollerated.
  tt_assert(string_is_valid_nonrfc_hostname("core9_euw1.fabrik.nytimes.com."));
  tt_assert(!string_is_valid_nonrfc_hostname(
                                         "..washingtonpost.is.better.com"));
  tt_assert(!string_is_valid_nonrfc_hostname("so.is..ft.com"));
  tt_assert(!string_is_valid_nonrfc_hostname("..."));

  // XXX: do we allow single-label DNS names?
  // We shouldn't for SOCKS (spec says "contains a fully-qualified domain name"
  // but only test pathologically malformed trailing '.' cases for now.
  tt_assert(!string_is_valid_nonrfc_hostname("."));
  tt_assert(!string_is_valid_nonrfc_hostname(".."));

  // IP address strings are not hostnames.
  tt_assert(!string_is_valid_nonrfc_hostname("8.8.8.8"));
  tt_assert(!string_is_valid_nonrfc_hostname("[2a00:1450:401b:800::200e]"));
  tt_assert(!string_is_valid_nonrfc_hostname("2a00:1450:401b:800::200e"));

  // We allow alphanumeric TLDs. For discussion, see ticket #25055.
  tt_assert(string_is_valid_nonrfc_hostname("lucky.13"));
  tt_assert(string_is_valid_nonrfc_hostname("luck.y13"));
  tt_assert(string_is_valid_nonrfc_hostname("luck.y13."));

  // We allow punycode TLDs. For examples, see
  // https://data.iana.org/TLD/tlds-alpha-by-domain.txt
  tt_assert(string_is_valid_nonrfc_hostname("example.xn--l1acc"));

  done:
  return;
}

static void
test_util_ipv4_validation(void *arg)
{
  (void)arg;

  tt_assert(string_is_valid_ipv4_address("192.168.0.1"));
  tt_assert(string_is_valid_ipv4_address("8.8.8.8"));

  tt_assert(!string_is_valid_ipv4_address("abcd"));
  tt_assert(!string_is_valid_ipv4_address("300.300.300.300"));
  tt_assert(!string_is_valid_ipv4_address("8.8."));

  done:
  return;
}

static void
test_util_ipv6_validation(void *arg)
{
  (void)arg;

  tt_assert(string_is_valid_ipv6_address("2a00:1450:401b:800::200e"));
  tt_assert(!string_is_valid_ipv6_address("11:22::33:44:"));

  done:
  return;
}

static void
test_util_writepid(void *arg)
{
  (void) arg;

  char *contents = NULL;
  const char *fname = get_fname("tmp_pid");
  unsigned long pid;
  char c;

  write_pidfile(fname);

  contents = read_file_to_str(fname, 0, NULL);
  tt_assert(contents);

  int n = tor_sscanf(contents, "%lu\n%c", &pid, &c);
  tt_int_op(n, OP_EQ, 1);

#ifdef _WIN32
  tt_uint_op(pid, OP_EQ, _getpid());
#else
  tt_uint_op(pid, OP_EQ, getpid());
#endif

 done:
  tor_free(contents);
}

static void
test_util_get_avail_disk_space(void *arg)
{
  (void) arg;
  int64_t val;

  /* No answer for nonexistent directory */
  val = tor_get_avail_disk_space("/akljasdfklsajdklasjkldjsa");
  tt_i64_op(val, OP_EQ, -1);

  /* Try the current directory */
  val = tor_get_avail_disk_space(".");

#if !defined(HAVE_STATVFS) && !defined(_WIN32)
  tt_i64_op(val, OP_EQ, -1); /* You don't have an implementation for this */
#else
  tt_i64_op(val, OP_GT, 0); /* You have some space. */
  tt_i64_op(val, OP_LT, ((int64_t)1)<<56); /* You don't have a zebibyte */
#endif /* !defined(HAVE_STATVFS) && !defined(_WIN32) */

 done:
  ;
}

/** Helper: Change the atime and mtime of a file. */
static void
set_file_mtime(const char *fname, time_t when)
{
  struct utimbuf u = { when, when };
  struct stat st;
  tt_int_op(0, OP_EQ, utime(fname, &u));
  tt_int_op(0, OP_EQ, stat(fname, &st));
  /* Let's hope that utime/stat give the same second as a round-trip? */
  tt_i64_op(st.st_mtime, OP_EQ, when);
done:
  ;
}

static void
test_util_touch_file(void *arg)
{
  (void) arg;
  const char *fname = get_fname("touch");

  const time_t now = time(NULL);
  struct stat st;
  write_bytes_to_file(fname, "abc", 3, 1);
  tt_int_op(0, OP_EQ, stat(fname, &st));
  /* A subtle point: the filesystem time is not necessarily equal to the
   * system clock time, since one can be using a monotonic clock, or coarse
   * monotonic clock, or whatever.  So we might wind up with an mtime a few
   * microseconds ago.  Let's just give it a lot of wiggle room. */
  tt_i64_op(st.st_mtime, OP_GE, now - 1);

  const time_t five_sec_ago = now - 5;
  set_file_mtime(fname, five_sec_ago);

  /* Finally we can touch the file */
  tt_int_op(0, OP_EQ, touch_file(fname));
  tt_int_op(0, OP_EQ, stat(fname, &st));
  tt_i64_op(st.st_mtime, OP_GE, now-1);

 done:
  ;
}

#ifndef DISABLE_PWDB_TESTS
static void
test_util_pwdb(void *arg)
{
  (void) arg;
  const struct passwd *me = NULL, *me2, *me3;
  char *name = NULL;
  char *dir = NULL;

  /* Uncached case. */
  /* Let's assume that we exist. */
  me = tor_getpwuid(getuid());
  tt_ptr_op(me, OP_NE, NULL);
  name = tor_strdup(me->pw_name);

  /* Uncached case */
  me2 = tor_getpwnam(name);
  tt_ptr_op(me2, OP_NE, NULL);
  tt_int_op(me2->pw_uid, OP_EQ, getuid());

  /* Cached case */
  me3 = tor_getpwuid(getuid());
  tt_ptr_op(me3, OP_NE, NULL);
  tt_str_op(me3->pw_name, OP_EQ, name);

  me3 = tor_getpwnam(name);
  tt_ptr_op(me3, OP_NE, NULL);
  tt_int_op(me3->pw_uid, OP_EQ, getuid());

  dir = get_user_homedir(name);
  tt_ptr_op(dir, OP_NE, NULL);

  /* Try failing cases.  First find a user that doesn't exist by name */
  char randbytes[4];
  char badname[9];
  int i, found=0;
  for (i = 0; i < 100; ++i) {
    crypto_rand(randbytes, sizeof(randbytes));
    base16_encode(badname, sizeof(badname), randbytes, sizeof(randbytes));
    if (tor_getpwnam(badname) == NULL) {
      found = 1;
      break;
    }
  }
  tt_assert(found);
  tor_free(dir);

  /* We should do a LOG_ERR */
  setup_full_capture_of_logs(LOG_ERR);
  dir = get_user_homedir(badname);
  tt_ptr_op(dir, OP_EQ, NULL);
  expect_log_msg_containing("not found");
  tt_int_op(smartlist_len(mock_saved_logs()), OP_EQ, 1);
  teardown_capture_of_logs();

  /* Now try to find a user that doesn't exist by ID. */
  found = 0;
  for (i = 0; i < 1000; ++i) {
    uid_t u;
    crypto_rand((char*)&u, sizeof(u));
    if (tor_getpwuid(u) == NULL) {
      found = 1;
      break;
    }
  }
  tt_assert(found);

 done:
  tor_free(name);
  tor_free(dir);
  teardown_capture_of_logs();
}
#endif /* !defined(DISABLE_PWDB_TESTS) */

static void
test_util_calloc_check(void *arg)
{
  (void) arg;
  /* Easy cases that are good. */
  tt_assert(size_mul_check(0,0));
  tt_assert(size_mul_check(0,100));
  tt_assert(size_mul_check(100,0));
  tt_assert(size_mul_check(100,100));

  /* Harder cases that are still good. */
  tt_assert(size_mul_check(SIZE_MAX, 1));
  tt_assert(size_mul_check(1, SIZE_MAX));
  tt_assert(size_mul_check(SIZE_MAX / 10, 9));
  tt_assert(size_mul_check(11, SIZE_MAX / 12));
  const size_t sqrt_size_max_p1 = ((size_t)1) << (sizeof(size_t) * 4);
  tt_assert(size_mul_check(sqrt_size_max_p1, sqrt_size_max_p1 - 1));

  /* Cases that overflow */
  tt_assert(! size_mul_check(SIZE_MAX, 2));
  tt_assert(! size_mul_check(2, SIZE_MAX));
  tt_assert(! size_mul_check(SIZE_MAX / 10, 11));
  tt_assert(! size_mul_check(11, SIZE_MAX / 10));
  tt_assert(! size_mul_check(SIZE_MAX / 8, 9));
  tt_assert(! size_mul_check(sqrt_size_max_p1, sqrt_size_max_p1));

 done:
  ;
}

static void
test_util_monotonic_time(void *arg)
{
  (void)arg;

  monotime_t mt1, mt2;
  monotime_coarse_t mtc1, mtc2;
  uint64_t nsec1, nsec2, usec1, msec1;
  uint64_t nsecc1, nsecc2, usecc1, msecc1;
  uint32_t stamp1, stamp2;

  monotime_init();

  monotime_get(&mt1);
  monotime_coarse_get(&mtc1);
  nsec1 = monotime_absolute_nsec();
  usec1 = monotime_absolute_usec();
  msec1 = monotime_absolute_msec();
  nsecc1 = monotime_coarse_absolute_nsec();
  usecc1 = monotime_coarse_absolute_usec();
  msecc1 = monotime_coarse_absolute_msec();
  stamp1 = monotime_coarse_to_stamp(&mtc1);

  tor_sleep_msec(200);

  monotime_get(&mt2);
  monotime_coarse_get(&mtc2);
  nsec2 = monotime_absolute_nsec();
  nsecc2 = monotime_coarse_absolute_nsec();
  stamp2 = monotime_coarse_to_stamp(&mtc2);

  /* We need to be a little careful here since we don't know the system load.
   */
  tt_i64_op(monotime_diff_msec(&mt1, &mt2), OP_GE, 175);
  tt_i64_op(monotime_diff_msec(&mt1, &mt2), OP_LT, 1000);
  tt_i64_op(monotime_coarse_diff_msec(&mtc1, &mtc2), OP_GE, 125);
  tt_i64_op(monotime_coarse_diff_msec(&mtc1, &mtc2), OP_LT, 1000);
  tt_u64_op(nsec2-nsec1, OP_GE, 175000000);
  tt_u64_op(nsec2-nsec1, OP_LT, 1000000000);
  tt_u64_op(nsecc2-nsecc1, OP_GE, 125000000);
  tt_u64_op(nsecc2-nsecc1, OP_LT, 1000000000);

  tt_u64_op(msec1, OP_GE, nsec1 / 1000000);
  tt_u64_op(usec1, OP_GE, nsec1 / 1000);
  tt_u64_op(msecc1, OP_GE, nsecc1 / 1000000);
  tt_u64_op(usecc1, OP_GE, nsecc1 / 1000);
  tt_u64_op(msec1, OP_LE, nsec1 / 1000000 + 10);
  tt_u64_op(usec1, OP_LE, nsec1 / 1000 + 10000);
  tt_u64_op(msecc1, OP_LE, nsecc1 / 1000000 + 10);
  tt_u64_op(usecc1, OP_LE, nsecc1 / 1000 + 10000);

  uint64_t coarse_stamp_diff =
    monotime_coarse_stamp_units_to_approx_msec(stamp2-stamp1);
  tt_u64_op(coarse_stamp_diff, OP_GE, 120);
  tt_u64_op(coarse_stamp_diff, OP_LE, 1200);

  {
    uint64_t units = monotime_msec_to_approx_coarse_stamp_units(5000);
    uint64_t ms = monotime_coarse_stamp_units_to_approx_msec(units);
    tt_u64_op(ms, OP_GE, 4950);
    tt_u64_op(ms, OP_LT, 5050);
  }

 done:
  ;
}

static void
test_util_monotonic_time_ratchet(void *arg)
{
  (void)arg;
  monotime_init();
  monotime_reset_ratchets_for_testing();

  /* win32, performance counter ratchet. */
  tt_i64_op(100, OP_EQ, ratchet_performance_counter(100));
  tt_i64_op(101, OP_EQ, ratchet_performance_counter(101));
  tt_i64_op(2000, OP_EQ, ratchet_performance_counter(2000));
  tt_i64_op(2000, OP_EQ, ratchet_performance_counter(100));
  tt_i64_op(2005, OP_EQ, ratchet_performance_counter(105));
  tt_i64_op(3005, OP_EQ, ratchet_performance_counter(1105));
  tt_i64_op(3005, OP_EQ, ratchet_performance_counter(1000));
  tt_i64_op(3010, OP_EQ, ratchet_performance_counter(1005));

  /* win32, GetTickCounts32 ratchet-and-rollover-detector. */
  const int64_t R = ((int64_t)1) << 32;
  tt_i64_op(5, OP_EQ, ratchet_coarse_performance_counter(5));
  tt_i64_op(1000, OP_EQ, ratchet_coarse_performance_counter(1000));
  tt_i64_op(5+R, OP_EQ, ratchet_coarse_performance_counter(5));
  tt_i64_op(10+R, OP_EQ, ratchet_coarse_performance_counter(10));
  tt_i64_op(4+R*2, OP_EQ, ratchet_coarse_performance_counter(4));

  /* gettimeofday regular ratchet. */
  struct timeval tv_in = {0,0}, tv_out;
  tv_in.tv_usec = 9000;

  ratchet_timeval(&tv_in, &tv_out);
  tt_int_op(tv_out.tv_usec, OP_EQ, 9000);
  tt_i64_op(tv_out.tv_sec, OP_EQ, 0);

  tv_in.tv_sec = 1337;
  tv_in.tv_usec = 0;
  ratchet_timeval(&tv_in, &tv_out);
  tt_int_op(tv_out.tv_usec, OP_EQ, 0);
  tt_i64_op(tv_out.tv_sec, OP_EQ, 1337);

  tv_in.tv_sec = 1336;
  tv_in.tv_usec = 500000;
  ratchet_timeval(&tv_in, &tv_out);
  tt_int_op(tv_out.tv_usec, OP_EQ, 0);
  tt_i64_op(tv_out.tv_sec, OP_EQ, 1337);

  tv_in.tv_sec = 1337;
  tv_in.tv_usec = 0;
  ratchet_timeval(&tv_in, &tv_out);
  tt_int_op(tv_out.tv_usec, OP_EQ, 500000);
  tt_i64_op(tv_out.tv_sec, OP_EQ, 1337);

  tv_in.tv_sec = 1337;
  tv_in.tv_usec = 600000;
  ratchet_timeval(&tv_in, &tv_out);
  tt_int_op(tv_out.tv_usec, OP_EQ, 100000);
  tt_i64_op(tv_out.tv_sec, OP_EQ, 1338);

  tv_in.tv_sec = 1000;
  tv_in.tv_usec = 1000;
  ratchet_timeval(&tv_in, &tv_out);
  tt_int_op(tv_out.tv_usec, OP_EQ, 100000);
  tt_i64_op(tv_out.tv_sec, OP_EQ, 1338);

  tv_in.tv_sec = 2000;
  tv_in.tv_usec = 2000;
  ratchet_timeval(&tv_in, &tv_out);
  tt_int_op(tv_out.tv_usec, OP_EQ, 101000);
  tt_i64_op(tv_out.tv_sec, OP_EQ, 2338);

 done:
  ;
}

static void
test_util_monotonic_time_zero(void *arg)
{
  (void) arg;
  monotime_t t1;
  monotime_coarse_t ct1;
  monotime_init();
  /* Check 1: The current time is not zero. */
  monotime_get(&t1);
  monotime_coarse_get(&ct1);
  tt_assert(!monotime_is_zero(&t1));
  tt_assert(!monotime_coarse_is_zero(&ct1));

  /* Check 2: The _zero() makes the time zero. */
  monotime_zero(&t1);
  monotime_coarse_zero(&ct1);
  tt_assert(monotime_is_zero(&t1));
  tt_assert(monotime_coarse_is_zero(&ct1));
 done:
  ;
}

static void
test_util_monotonic_time_add_msec(void *arg)
{
  (void) arg;
  monotime_t t1, t2;
  monotime_coarse_t ct1, ct2;
  monotime_init();

  monotime_get(&t1);
  monotime_coarse_get(&ct1);

  /* adding zero does nothing */
  monotime_add_msec(&t2, &t1, 0);
  monotime_coarse_add_msec(&ct2, &ct1, 0);
  tt_i64_op(monotime_diff_msec(&t1, &t2), OP_EQ, 0);
  tt_i64_op(monotime_coarse_diff_msec(&ct1, &ct2), OP_EQ, 0);

  /* Add 1337 msec; see if the diff function agree */
  monotime_add_msec(&t2, &t1, 1337);
  monotime_coarse_add_msec(&ct2, &ct1, 1337);
  tt_i64_op(monotime_diff_msec(&t1, &t2), OP_EQ, 1337);
  tt_i64_op(monotime_coarse_diff_msec(&ct1, &ct2), OP_EQ, 1337);
  // The 32-bit variant must be within 1% of the regular one.
  tt_int_op(monotime_coarse_diff_msec32_(&ct1, &ct2), OP_GT, 1323);
  tt_int_op(monotime_coarse_diff_msec32_(&ct1, &ct2), OP_LT, 1350);

  /* Add 1337 msec twice more; make sure that any second rollover issues
   * worked. */
  monotime_add_msec(&t2, &t2, 1337);
  monotime_coarse_add_msec(&ct2, &ct2, 1337);
  monotime_add_msec(&t2, &t2, 1337);
  monotime_coarse_add_msec(&ct2, &ct2, 1337);
  tt_i64_op(monotime_diff_msec(&t1, &t2), OP_EQ, 1337*3);
  tt_i64_op(monotime_coarse_diff_msec(&ct1, &ct2), OP_EQ, 1337*3);
  tt_int_op(monotime_coarse_diff_msec32_(&ct1, &ct2), OP_GT, 3970);
  tt_int_op(monotime_coarse_diff_msec32_(&ct1, &ct2), OP_LT, 4051);

 done:
  ;
}

static void
test_util_nowrap_math(void *arg)
{
  (void)arg;

  tt_u64_op(0, OP_EQ, tor_add_u32_nowrap(0, 0));
  tt_u64_op(1, OP_EQ, tor_add_u32_nowrap(0, 1));
  tt_u64_op(1, OP_EQ, tor_add_u32_nowrap(1, 0));
  tt_u64_op(4, OP_EQ, tor_add_u32_nowrap(2, 2));
  tt_u64_op(UINT32_MAX, OP_EQ, tor_add_u32_nowrap(UINT32_MAX-1, 2));
  tt_u64_op(UINT32_MAX, OP_EQ, tor_add_u32_nowrap(2, UINT32_MAX-1));
  tt_u64_op(UINT32_MAX, OP_EQ, tor_add_u32_nowrap(UINT32_MAX, UINT32_MAX));

  tt_u64_op(0, OP_EQ, tor_mul_u64_nowrap(0, 0));
  tt_u64_op(1, OP_EQ, tor_mul_u64_nowrap(1, 1));
  tt_u64_op(2, OP_EQ, tor_mul_u64_nowrap(2, 1));
  tt_u64_op(4, OP_EQ, tor_mul_u64_nowrap(2, 2));
  tt_u64_op(UINT64_MAX, OP_EQ, tor_mul_u64_nowrap(UINT64_MAX, 1));
  tt_u64_op(UINT64_MAX, OP_EQ, tor_mul_u64_nowrap(2, UINT64_MAX));
  tt_u64_op(UINT64_MAX, OP_EQ, tor_mul_u64_nowrap(UINT64_MAX, UINT64_MAX));

 done:
  ;
}

static void
test_util_htonll(void *arg)
{
  (void)arg;
#ifdef WORDS_BIGENDIAN
  const uint64_t res_be = 0x8877665544332211;
#else
  const uint64_t res_le = 0x1122334455667788;
#endif

  tt_u64_op(0, OP_EQ, tor_htonll(0));
  tt_u64_op(0, OP_EQ, tor_ntohll(0));
  tt_u64_op(UINT64_MAX, OP_EQ, tor_htonll(UINT64_MAX));
  tt_u64_op(UINT64_MAX, OP_EQ, tor_ntohll(UINT64_MAX));

#ifdef WORDS_BIGENDIAN
  tt_u64_op(res_be, OP_EQ, tor_htonll(0x8877665544332211));
  tt_u64_op(res_be, OP_EQ, tor_ntohll(0x8877665544332211));
#else
  tt_u64_op(res_le, OP_EQ, tor_htonll(0x8877665544332211));
  tt_u64_op(res_le, OP_EQ, tor_ntohll(0x8877665544332211));
#endif /* defined(WORDS_BIGENDIAN) */

 done:
  ;
}

static void
test_util_get_unquoted_path(void *arg)
{
  (void)arg;

  char *r = NULL;

  r = get_unquoted_path("\""); // "
  tt_ptr_op(r, OP_EQ, NULL);
  tor_free(r);

  r = get_unquoted_path("\"\"\""); // """
  tt_ptr_op(r, OP_EQ, NULL);
  tor_free(r);

  r = get_unquoted_path("\\\""); // \"
  tt_ptr_op(r, OP_EQ, NULL);
  tor_free(r);

  r = get_unquoted_path("\\\"\\\""); // \"\"
  tt_ptr_op(r, OP_EQ, NULL);
  tor_free(r);

  r = get_unquoted_path("A\\B\\C\""); // A\B\C"
  tt_ptr_op(r, OP_EQ, NULL);
  tor_free(r);

  r = get_unquoted_path("\"A\\B\\C"); // "A\B\C
  tt_ptr_op(r, OP_EQ, NULL);
  tor_free(r);

  r = get_unquoted_path("\"A\\B\"C\""); // "A\B"C"
  tt_ptr_op(r, OP_EQ, NULL);
  tor_free(r);

  r = get_unquoted_path("A\\B\"C"); // A\B"C
  tt_ptr_op(r, OP_EQ, NULL);
  tor_free(r);

  r = get_unquoted_path("");
  tt_str_op(r, OP_EQ, "");
  tor_free(r);

  r = get_unquoted_path("\"\""); // ""
  tt_str_op(r, OP_EQ, "");
  tor_free(r);

  r = get_unquoted_path("A\\B\\C"); // A\B\C
  tt_str_op(r, OP_EQ, "A\\B\\C"); // A\B\C
  tor_free(r);

  r = get_unquoted_path("\"A\\B\\C\""); // "A\B\C"
  tt_str_op(r, OP_EQ, "A\\B\\C"); // A\B\C
  tor_free(r);

  r = get_unquoted_path("\"\\\""); // "\"
  tt_str_op(r, OP_EQ, "\\"); // \ /* comment to prevent line continuation */
  tor_free(r);

  r = get_unquoted_path("\"\\\"\""); // "\""
  tt_str_op(r, OP_EQ, "\""); // "
  tor_free(r);

  r = get_unquoted_path("\"A\\B\\C\\\"\""); // "A\B\C\""
  tt_str_op(r, OP_EQ, "A\\B\\C\""); // A\B\C"
  tor_free(r);

  r = get_unquoted_path("A\\B\\\"C"); // A\B\"C
  tt_str_op(r, OP_EQ, "A\\B\"C"); // A\B"C
  tor_free(r);

  r = get_unquoted_path("\"A\\B\\\"C\""); // "A\B\"C"
  tt_str_op(r, OP_EQ, "A\\B\"C"); // A\B"C

 done:
  tor_free(r);
}

static void
test_util_map_anon(void *arg)
{
  (void)arg;
  char *ptr = NULL;
  size_t sz = 16384;
  unsigned inherit=0;

  /* Basic checks. */
  ptr = tor_mmap_anonymous(sz, 0, &inherit);
  tt_ptr_op(ptr, OP_NE, 0);
  tt_int_op(inherit, OP_EQ, INHERIT_RES_KEEP);
  ptr[sz-1] = 3;
  tt_int_op(ptr[0], OP_EQ, 0);
  tt_int_op(ptr[sz-2], OP_EQ, 0);
  tt_int_op(ptr[sz-1], OP_EQ, 3);

  /* Try again, with a private (non-swappable) mapping. */
  tor_munmap_anonymous(ptr, sz);
  ptr = tor_mmap_anonymous(sz, ANONMAP_PRIVATE, &inherit);
  tt_ptr_op(ptr, OP_NE, 0);
  tt_int_op(inherit, OP_EQ, INHERIT_RES_KEEP);
  ptr[sz-1] = 10;
  tt_int_op(ptr[0], OP_EQ, 0);
  tt_int_op(ptr[sz/2], OP_EQ, 0);
  tt_int_op(ptr[sz-1], OP_EQ, 10);

  /* Now let's test a drop-on-fork mapping. */
  tor_munmap_anonymous(ptr, sz);
  ptr = tor_mmap_anonymous(sz, ANONMAP_NOINHERIT, &inherit);
  tt_ptr_op(ptr, OP_NE, 0);
  ptr[sz-1] = 10;
  tt_int_op(ptr[0], OP_EQ, 0);
  tt_int_op(ptr[sz/2], OP_EQ, 0);
  tt_int_op(ptr[sz-1], OP_EQ, 10);

 done:
  tor_munmap_anonymous(ptr, sz);
}

static void
test_util_map_anon_nofork(void *arg)
{
  (void)arg;
#ifdef _WIN32
  /* The operating system doesn't support forking. */
  tt_skip();
 done:
  ;
#else /* !defined(_WIN32) */
  /* We have the right OS support.  We're going to try marking the buffer as
   * either zero-on-fork or as drop-on-fork, whichever is supported.  Then we
   * will fork and send a byte back to the parent process.  This will either
   * crash, or send zero. */

  char *ptr = NULL;
  const char TEST_VALUE = 0xd0;
  size_t sz = 16384;
  int pipefd[2] = {-1, -1};
  unsigned inherit=0;

  tor_munmap_anonymous(ptr, sz);
  ptr = tor_mmap_anonymous(sz, ANONMAP_NOINHERIT, &inherit);
  tt_ptr_op(ptr, OP_NE, 0);
  memset(ptr, (uint8_t)TEST_VALUE, sz);

  tt_int_op(0, OP_EQ, pipe(pipefd));
  pid_t child = fork();
  if (child == 0) {
    /* We're in the child. */
    close(pipefd[0]);
    ssize_t r = write(pipefd[1], &ptr[sz-1], 1); /* This may crash. */
    close(pipefd[1]);
    if (r < 0)
      exit(1);
    exit(0);
  }
  tt_int_op(child, OP_GT, 0);
  /* In the parent. */
  close(pipefd[1]);
  pipefd[1] = -1;
  char buf[1];
  ssize_t r = read(pipefd[0], buf, 1);

  if (inherit == INHERIT_RES_ZERO) {
    // We should be seeing clear-on-fork behavior.
    tt_int_op((int)r, OP_EQ, 1); // child should send us a byte.
    tt_int_op(buf[0], OP_EQ, 0); // that byte should be zero.
  } else if (inherit == INHERIT_RES_DROP) {
    // We should be seeing noinherit behavior.
    tt_int_op(r, OP_LE, 0); // child said nothing; it should have crashed.
  } else {
    // noinherit isn't implemented.
    tt_int_op(inherit, OP_EQ, INHERIT_RES_KEEP);
    tt_int_op((int)r, OP_EQ, 1); // child should send us a byte.
    tt_int_op(buf[0], OP_EQ, TEST_VALUE); // that byte should be TEST_VALUE.
  }

  int ws;
  waitpid(child, &ws, 0);

#ifndef NOINHERIT_CAN_FAIL
  /* Only if NOINHERIT_CAN_FAIL should it be possible for us to get
   * INHERIT_KEEP behavior in this case. */
  tt_int_op(inherit, OP_NE, INHERIT_RES_KEEP);
#else
  if (inherit == INHERIT_RES_KEEP) {
    /* Call this test "skipped", not "passed", since noinherit wasn't
     * implemented. */
    tt_skip();
  }
#endif /* !defined(NOINHERIT_CAN_FAIL) */

 done:
  tor_munmap_anonymous(ptr, sz);
  if (pipefd[0] >= 0) {
    close(pipefd[0]);
  }
  if (pipefd[1] >= 0) {
    close(pipefd[1]);
  }
#endif /* defined(_WIN32) */
}

#ifndef COCCI
#define UTIL_LEGACY(name)                                               \
  { (#name), test_util_ ## name , 0, NULL, NULL }

#define UTIL_TEST(name, flags)                          \
  { (#name), test_util_ ## name, flags, NULL, NULL }

#define COMPRESS(name, identifier)              \
  { ("compress/" #name), test_util_compress, 0, &compress_setup,        \
    (char*)(identifier) }

#define COMPRESS_CONCAT(name, identifier)                               \
  { ("compress_concat/" #name), test_util_decompress_concatenated, 0,   \
    &compress_setup,                                                    \
    (char*)(identifier) }

#define COMPRESS_JUNK(name, identifier)                                 \
  { ("compress_junk/" #name), test_util_decompress_junk, 0,             \
    &compress_setup,                                                    \
    (char*)(identifier) }

#define COMPRESS_DOS(name, identifier)                                  \
  { ("compress_dos/" #name), test_util_decompress_dos, 0,               \
    &compress_setup,                                                    \
    (char*)(identifier) }

#ifdef _WIN32
#define UTIL_TEST_WIN_ONLY(n, f) UTIL_TEST(n, (f))
#else
#define UTIL_TEST_WIN_ONLY(n, f) { (#n), NULL, TT_SKIP, NULL, NULL }
#endif

#ifdef DISABLE_PWDB_TESTS
#define UTIL_TEST_PWDB(n, f) { (#n), NULL, TT_SKIP, NULL, NULL }
#else
#define UTIL_TEST_PWDB(n, f) UTIL_TEST(n, (f))
#endif
#endif /* !defined(COCCI) */

struct testcase_t util_tests[] = {
  UTIL_LEGACY(time),
  UTIL_TEST(parse_http_time, 0),
  UTIL_LEGACY(config_line),
  UTIL_LEGACY(config_line_quotes),
  UTIL_LEGACY(config_line_comment_character),
  UTIL_LEGACY(config_line_escaped_content),
  UTIL_LEGACY(config_line_crlf),
  UTIL_TEST(config_line_partition, 0),
  UTIL_TEST_PWDB(expand_filename, 0),
  UTIL_LEGACY(escape_string_socks),
  UTIL_LEGACY(string_is_key_value),
  UTIL_LEGACY(strmisc),
  UTIL_TEST(parse_integer, 0),
  UTIL_LEGACY(pow2),
  COMPRESS(zlib, "deflate"),
  COMPRESS(gzip, "gzip"),
  COMPRESS(lzma, "x-tor-lzma"),
  COMPRESS(zstd, "x-zstd"),
  COMPRESS(zstd_nostatic, "x-zstd:nostatic"),
  COMPRESS(none, "identity"),
  COMPRESS_CONCAT(zlib, "deflate"),
  COMPRESS_CONCAT(gzip, "gzip"),
  COMPRESS_CONCAT(lzma, "x-tor-lzma"),
  COMPRESS_CONCAT(zstd, "x-zstd"),
  COMPRESS_CONCAT(zstd_nostatic, "x-zstd:nostatic"),
  COMPRESS_CONCAT(none, "identity"),
  COMPRESS_JUNK(zlib, "deflate"),
  COMPRESS_JUNK(gzip, "gzip"),
  COMPRESS_JUNK(lzma, "x-tor-lzma"),
  COMPRESS_DOS(zlib, "deflate"),
  COMPRESS_DOS(gzip, "gzip"),
  COMPRESS_DOS(lzma, "x-tor-lzma"),
  COMPRESS_DOS(zstd, "x-zstd"),
  COMPRESS_DOS(zstd_nostatic, "x-zstd:nostatic"),
  UTIL_TEST(gzip_compression_bomb, TT_FORK),
  UTIL_LEGACY(datadir),
  UTIL_LEGACY(memarea),
  UTIL_LEGACY(control_formats),
  UTIL_LEGACY(mmap),
  UTIL_TEST(sscanf, TT_FORK),
  UTIL_LEGACY(format_time_interval),
  UTIL_LEGACY(path_is_relative),
  UTIL_LEGACY(strtok),
  UTIL_LEGACY(di_ops),
  UTIL_TEST(memcpy_iftrue_timei, 0),
  UTIL_TEST(di_map, 0),
  UTIL_TEST(round_to_next_multiple_of, 0),
  UTIL_TEST(laplace, 0),
  UTIL_TEST(clamp_double_to_int64, 0),
  UTIL_TEST(find_str_at_start_of_line, 0),
  UTIL_TEST(tor_strreplacechar, 0),
  UTIL_TEST(string_is_C_identifier, 0),
  UTIL_TEST(string_is_utf8, 0),
  UTIL_TEST(asprintf, 0),
  UTIL_TEST(listdir, 0),
  UTIL_TEST(glob, 0),
  UTIL_TEST(get_glob_opened_files, 0),
  UTIL_TEST(parent_dir, 0),
  UTIL_TEST(ftruncate, 0),
  UTIL_TEST(nowrap_math, 0),
  UTIL_TEST(num_cpus, 0),
  UTIL_TEST_WIN_ONLY(load_win_lib, 0),
  UTIL_TEST(format_hex_number, 0),
  UTIL_TEST(format_dec_number, 0),
  UTIL_TEST(n_bits_set, 0),
  UTIL_TEST(eat_whitespace, 0),
  UTIL_TEST(sl_new_from_text_lines, 0),
  UTIL_TEST(envnames, 0),
  UTIL_TEST(make_environment, 0),
  UTIL_TEST(set_env_var_in_sl, 0),
  UTIL_TEST(read_file_eof_tiny_limit, 0),
  UTIL_TEST(read_file_eof_one_loop_a, 0),
  UTIL_TEST(read_file_eof_one_loop_b, 0),
  UTIL_TEST(read_file_eof_two_loops, 0),
  UTIL_TEST(read_file_eof_two_loops_b, 0),
  UTIL_TEST(read_file_eof_zero_bytes, 0),
  UTIL_TEST(read_file_endlines, 0),
  UTIL_TEST(write_chunks_to_file, 0),
  UTIL_TEST(write_str_if_changed, 0),
  UTIL_TEST(mathlog, 0),
  UTIL_TEST(fraction, 0),
  UTIL_TEST(weak_random, 0),
  { "tor_isinf", test_tor_isinf, TT_FORK, NULL, NULL },
  { "socket_ipv4", test_util_socket, TT_FORK, &passthrough_setup,
    (void*)"4" },
  { "socket_ipv6", test_util_socket, TT_FORK,
    &passthrough_setup, (void*)"6" },
  { "socketpair", test_util_socketpair, TT_FORK, &passthrough_setup,
    (void*)"0" },
  { "socketpair_ersatz", test_util_socketpair, TT_FORK,
    &passthrough_setup, (void*)"1" },
  UTIL_TEST(max_mem, 0),
  UTIL_TEST(hostname_validation, 0),
  UTIL_TEST(dest_validation_edgecase, 0),
  UTIL_TEST(ipv4_validation, 0),
  UTIL_TEST(ipv6_validation, 0),
  UTIL_TEST(writepid, 0),
  UTIL_TEST(get_avail_disk_space, 0),
  UTIL_TEST(touch_file, 0),
  UTIL_TEST_PWDB(pwdb, TT_FORK),
  UTIL_TEST(calloc_check, 0),
  UTIL_TEST(monotonic_time, 0),
  UTIL_TEST(monotonic_time_ratchet, TT_FORK),
  UTIL_TEST(monotonic_time_zero, 0),
  UTIL_TEST(monotonic_time_add_msec, 0),
  UTIL_TEST(timegm_real, 0),
  UTIL_TEST(htonll, 0),
  UTIL_TEST(get_unquoted_path, 0),
  UTIL_TEST(map_anon, 0),
  UTIL_TEST(map_anon_nofork, 0),
  END_OF_TESTCASES
};
