/* Copyright (c) 2017-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file test_proto_http.c
 * \brief Tests for our HTTP protocol parser code
 */

#include "core/or/or.h"
#include "test/test.h"
#include "lib/buf/buffers.h"
#include "core/proto/proto_http.h"
#include "test/log_test_helpers.h"

#define S(str) str, sizeof(str)-1

static void
test_proto_http_peek(void *arg)
{
  (void) arg;
  const struct {
    int is_http;
    const char *message;
    size_t len;
  } cases[] = {
    { 1, S("GET /index HTTP/1.0\r\n") },
    { 1, S("GET /index HTTP/1.1\r\n") },
    { 1, S("GET ") },
    { 0, S("GIT ") },
    { 0, S("GET") },
    { 0, S("get ") },
    { 0, S("GETAWAY") },
  };
  unsigned i;
  buf_t *buf = buf_new();
  for (i = 0; i < ARRAY_LENGTH(cases); ++i) {
    TT_BLATHER(("Trying case %u", i));
    buf_add(buf, cases[i].message, cases[i].len);
    tt_int_op(cases[i].is_http, OP_EQ, peek_buf_has_http_command(buf));
    buf_clear(buf);
  }
 done:
  buf_free(buf);
}

static void
test_proto_http_valid(void *arg)
{
  (void) arg;
  const struct {
    const char *message;
    size_t len;
    const char *headers;
    const char *body;
    size_t bodylen;
    int should_detect_truncated;
    int bytes_left_over;
  } cases[] = {
    { S("GET /index.html HTTP/1.0\r\n\r\n"),
      "GET /index.html HTTP/1.0\r\n\r\n",
      S(""),
      1, 0,
    },
    { S("PUT /tor/foo HTTP/1.1\r\n"
        "Content-Length: 51\r\n\r\n"
        "this is a test of the http parsing system . test te"),
      "PUT /tor/foo HTTP/1.1\r\n" "Content-Length: 51\r\n\r\n",
      S("this is a test of the http parsing system . test te"),
      1, 0,
    },
    { S("PUT /tor/foo HTTP/1.1\r\n"
        "Content-Length: 5\r\n\r\n"
        "there are more than 5 characters in this body."),
      "PUT /tor/foo HTTP/1.1\r\n" "Content-Length: 5\r\n\r\n",
      S("there"),
      0, 41,
    },
    { S("PUT /tor/bar HTTP/1.1\r\n\r\n"
        "this is another \x00test"),
      "PUT /tor/bar HTTP/1.1\r\n\r\n",
      S("this is another \x00test"),
      0, 0,
    }
  };
  unsigned i;
  buf_t *buf = buf_new();
  char *h = NULL, *b = NULL;

  for (i = 0; i < ARRAY_LENGTH(cases); ++i) {
    TT_BLATHER(("Trying case %u", i));
    size_t bl = 0;
    // truncate by 2 chars
    buf_add(buf, cases[i].message, cases[i].len - 2);

    if (cases[i].should_detect_truncated) {
      tt_int_op(0, OP_EQ, fetch_from_buf_http(buf, &h, 1024*16,
                                              &b, &bl, 1024*16, 0));
      tt_ptr_op(h, OP_EQ, NULL);
      tt_ptr_op(b, OP_EQ, NULL);
      tt_u64_op(bl, OP_EQ, 0);
      tt_int_op(buf_datalen(buf), OP_EQ, cases[i].len - 2);
    }

    // add the rest.
    buf_add(buf, cases[i].message+cases[i].len-2, 2);
    tt_int_op(1, OP_EQ, fetch_from_buf_http(buf, &h, 1024*16,
                                            &b, &bl, 1024*16, 0));
    tt_str_op(h, OP_EQ, cases[i].headers);
    tt_u64_op(bl, OP_EQ, cases[i].bodylen);
    tt_mem_op(b, OP_EQ, cases[i].body, bl);
    tt_int_op(buf_datalen(buf), OP_EQ, cases[i].bytes_left_over);

    buf_clear(buf);
    tor_free(h);
    tor_free(b);
  }
 done:
  tor_free(h);
  tor_free(b);
  buf_free(buf);
}

static void
test_proto_http_invalid(void *arg)
{
  (void) arg;
  const struct {
    const char *message;
    size_t len;
    const char *expect;
  } cases[] = {
    /* Overlong headers, headers not finished. */
    { S("GET /index.xhml HTTP/1.0\r\n"
        "X-My-headers-are-too-long: yes indeed they are. They might be\r\n"
        "X-My-headers-are-too-long: normal under other circumstances, but\r\n"
        "X-My-headers-are-too-long: the 128-byte limit makes them bad\r\n"),
      "headers too long." },
    /* Overlong finished headers. */
    { S("GET /index.xhml HTTP/1.0\r\n"
        "X-My-headers-are-too-long: yes indeed they are. They might be\r\n"
        "X-My-headers-are-too-long: normal under other circumstances, but\r\n"
        "X-My-headers-are-too-long: the 128-byte limit makes them bad\r\n"
        "\r\n"),
        "headers too long." },
    /* Exactly too long finished headers. */
    { S("GET /index.xhml HTTP/1.0\r\n"
        "X-My-headers-are-too-long: yes indeed they are. They might be\r\n"
        "X-My-headers-are-too-long: normal un\r\n\r\n"),
      "headerlen 129 larger than 127. Failing." },
    /* Body too long, with content-length */
    { S("GET /index.html HTTP/1.0\r\n"
        "Content-Length: 129\r\n\r\n"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxx"),
      "bodylen 129 larger than 127" },
    /* Body too long, with content-length lying */
    { S("GET /index.html HTTP/1.0\r\n"
        "Content-Length: 99999\r\n\r\n"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxx"),
      "bodylen 138 larger than 127" },
    /* Body too long, no content-length. */
    { S("GET /index.html HTTP/1.0\r\n\r\n"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxz"),
      "bodylen 139 larger than 127" },
    /* Content-Length is junk. */
    { S("GET /index.html HTTP/1.0\r\n"
        "Content-Length: Cheese\r\n\r\n"
        "foo"),
      "Content-Length is bogus; maybe someone is trying to crash us." },
    };
  unsigned i;
  buf_t *buf = buf_new();
  char *h = NULL, *b = NULL;
  setup_capture_of_logs(LOG_DEBUG);

  for (i = 0; i < ARRAY_LENGTH(cases); ++i) {
    TT_BLATHER(("Trying case %u", i));
    size_t bl = 0;
    buf_add(buf, cases[i].message, cases[i].len);

    /* Use low body limits here so we can force over-sized object warnings */
    tt_int_op(-1, OP_EQ, fetch_from_buf_http(buf, &h, 128,
                                             &b, &bl, 128, 0));
    tt_ptr_op(h, OP_EQ, NULL);
    tt_ptr_op(b, OP_EQ, NULL);
    tt_u64_op(bl, OP_EQ, 0);
    expect_log_msg_containing(cases[i].expect);

    buf_clear(buf);
    tor_free(h);
    tor_free(b);
    mock_clean_saved_logs();
  }
 done:
  tor_free(h);
  tor_free(b);
  buf_free(buf);
  teardown_capture_of_logs();
}

struct testcase_t proto_http_tests[] = {
  { "peek", test_proto_http_peek, 0, NULL, NULL },
  { "valid", test_proto_http_valid, 0, NULL, NULL },
  { "invalid", test_proto_http_invalid, 0, NULL, NULL },

  END_OF_TESTCASES
};

