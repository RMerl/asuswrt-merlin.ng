/* Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#include "core/or/or.h"
#include "lib/buf/buffers.h"
#include "app/config/config.h"
#include "core/mainloop/connection.h"
#include "core/proto/proto_socks.h"
#include "test/test.h"
#include "test/log_test_helpers.h"
#include "core/or/socks_request_st.h"
#include "lib/net/socks5_status.h"

typedef struct socks_test_data_t {
  socks_request_t *req;
  buf_t *buf;
} socks_test_data_t;

static void *
socks_test_setup(const struct testcase_t *testcase)
{
  socks_test_data_t *data = tor_malloc(sizeof(socks_test_data_t));
  (void)testcase;
  data->buf = buf_new_with_capacity(256);
  data->req = socks_request_new();
  config_register_addressmaps(get_options());
  return data;
}
static int
socks_test_cleanup(const struct testcase_t *testcase, void *ptr)
{
  socks_test_data_t *data = ptr;
  (void)testcase;
  buf_free(data->buf);
  socks_request_free(data->req);
  tor_free(data);
  return 1;
}

static const struct testcase_setup_t socks_setup = {
  socks_test_setup, socks_test_cleanup
};

#define SOCKS_TEST_INIT()                       \
  socks_test_data_t *testdata = ptr;            \
  buf_t *buf = testdata->buf;                   \
  socks_request_t *socks = testdata->req;
#define ADD_DATA(buf, s)                                        \
  buf_add(buf, s, sizeof(s)-1)

static void
socks_request_clear(socks_request_t *socks)
{
  tor_free(socks->username);
  tor_free(socks->password);
  memset(socks, 0, sizeof(socks_request_t));
}

/** Perform unsupported SOCKS 4 commands */
static void
test_socks_4_unsupported_commands(void *ptr)
{
  SOCKS_TEST_INIT();

  /* SOCKS 4 Send BIND [02] to IP address 2.2.2.2:4369 */
  ADD_DATA(buf, "\x04\x02\x11\x11\x02\x02\x02\x02\x00");
  tt_int_op(fetch_from_buf_socks(buf, socks, get_options()->TestSocks,
                                 get_options()->SafeSocks),
            OP_EQ, -1);
  tt_int_op(4,OP_EQ, socks->socks_version);
  tt_int_op(0,OP_EQ, socks->replylen); /* XXX: shouldn't tor reply? */

 done:
  ;
}

/** Perform supported SOCKS 4 commands */
static void
test_socks_4_supported_commands(void *ptr)
{
  SOCKS_TEST_INIT();

  tt_int_op(0,OP_EQ, buf_datalen(buf));

  /* SOCKS 4 Send CONNECT [01] to IP address 2.2.2.3:4370 */
  ADD_DATA(buf, "\x04\x01\x11\x12\x02\x02\x02\x03\x00");
  tt_int_op(fetch_from_buf_socks(buf, socks, get_options()->TestSocks,
                                 get_options()->SafeSocks),
            OP_EQ, 1);
  tt_int_op(4,OP_EQ, socks->socks_version);
  tt_int_op(0,OP_EQ, socks->replylen); /* XXX: shouldn't tor reply? */
  tt_int_op(SOCKS_COMMAND_CONNECT,OP_EQ, socks->command);
  tt_str_op("2.2.2.3",OP_EQ, socks->address);
  tt_int_op(4370,OP_EQ, socks->port);
  tt_assert(socks->got_auth == 0);
  tt_assert(! socks->username);

  tt_int_op(0,OP_EQ, buf_datalen(buf));
  socks_request_clear(socks);

  /* SOCKS 4 Send CONNECT [01] to IP address 2.2.2.4:4369 with userid*/
  ADD_DATA(buf, "\x04\x01\x11\x12\x02\x02\x02\x04me\x00");
  tt_int_op(fetch_from_buf_socks(buf, socks, 1, 0),
            OP_EQ, 1);
  tt_int_op(4,OP_EQ, socks->socks_version);
  tt_int_op(0,OP_EQ, socks->replylen); /* XXX: shouldn't tor reply? */
  tt_int_op(SOCKS_COMMAND_CONNECT,OP_EQ, socks->command);
  tt_str_op("2.2.2.4",OP_EQ, socks->address);
  tt_int_op(4370,OP_EQ, socks->port);
  tt_assert(socks->got_auth == 1);
  tt_assert(socks->username);
  tt_int_op(2,OP_EQ, socks->usernamelen);
  tt_mem_op("me",OP_EQ, socks->username, 2);

  tt_int_op(0,OP_EQ, buf_datalen(buf));
  socks_request_clear(socks);

  /* SOCKS 4a Send RESOLVE [F0] request for torproject.org */
  ADD_DATA(buf, "\x04\xF0\x01\x01\x00\x00\x00\x02me\x00torproject.org\x00");
  tt_int_op(fetch_from_buf_socks(buf, socks, 1,
                                 get_options()->SafeSocks),
            OP_EQ, 1);
  tt_int_op(4,OP_EQ, socks->socks_version);
  tt_int_op(0,OP_EQ, socks->replylen); /* XXX: shouldn't tor reply? */
  tt_str_op("torproject.org",OP_EQ, socks->address);

  tt_int_op(0,OP_EQ, buf_datalen(buf));

 done:
  ;
}

static void
test_socks_4_bad_arguments(void *ptr)
{
  SOCKS_TEST_INIT();
  setup_capture_of_logs(LOG_DEBUG);

  /* Try with 0 IPv4 address */
  ADD_DATA(buf, "\x04\x01\x00\x50\x00\x00\x00\x00\x00");
  tt_int_op(fetch_from_buf_socks(buf, socks, get_options()->TestSocks,
                                 get_options()->SafeSocks),
            OP_EQ, -1);
  buf_clear(buf);
  expect_log_msg_containing("Port or DestIP is zero.");
  mock_clean_saved_logs();

  /* Try with 0 port */
  ADD_DATA(buf, "\x04\x01\x00\x00\x01\x02\x03\x04\x00");
  tt_int_op(fetch_from_buf_socks(buf, socks, get_options()->TestSocks,
                                 get_options()->SafeSocks),
            OP_EQ, -1);
  buf_clear(buf);
  expect_log_msg_containing("Port or DestIP is zero.");
  mock_clean_saved_logs();

  /* Try with 2000-byte username (!) */
  ADD_DATA(buf, "\x04\x01\x00\x50\x01\x02\x03\x04");
  int i;
  for (i = 0; i < 200; ++i) {
    ADD_DATA(buf, "1234567890");
  }
  ADD_DATA(buf, "\x00");
  tt_int_op(fetch_from_buf_socks(buf, socks, 1, 0),
            OP_EQ, -1);
  buf_clear(buf);
  expect_log_msg_containing("socks4: parsing failed - invalid request.");
  mock_clean_saved_logs();

  /* Try with 2000-byte hostname */
  ADD_DATA(buf, "\x04\x01\x00\x50\x00\x00\x00\x01\x00");
  for (i = 0; i < 200; ++i) {
    ADD_DATA(buf, "1234567890");
  }
  ADD_DATA(buf, "\x00");
  {
    const char *p;
    size_t s;
    buf_pullup(buf, 9999, &p, &s);
  }
  tt_int_op(fetch_from_buf_socks(buf, socks, 1, 0),
            OP_EQ, -1);
  buf_clear(buf);
  expect_log_msg_containing("Destaddr too long. Rejecting.");
  mock_clean_saved_logs();

  /* Try with 2000-byte hostname, not terminated. */
  ADD_DATA(buf, "\x04\x01\x00\x50\x00\x00\x00\x01\x00");
  for (i = 0; i < 200; ++i) {
    ADD_DATA(buf, "1234567890");
  }
  tt_int_op(fetch_from_buf_socks(buf, socks, 1, 0),
            OP_EQ, -1);
  buf_clear(buf);
  expect_log_msg_containing("parsing failed - invalid request.");
  mock_clean_saved_logs();

  /* Socks4, bogus hostname */
  ADD_DATA(buf, "\x04\x01\x00\x50\x00\x00\x00\x01\x00" "---\x00" );
  tt_int_op(fetch_from_buf_socks(buf, socks, 1, 0), OP_EQ, -1);
  buf_clear(buf);
  expect_log_msg_containing("Your application (using socks4 to port 80) "
                            "gave Tor a malformed hostname: ");
  mock_clean_saved_logs();

 done:
  teardown_capture_of_logs();
}

/**  Perform unsupported SOCKS 5 commands */
static void
test_socks_5_unsupported_commands(void *ptr)
{
  SOCKS_TEST_INIT();

  /* SOCKS 5 Send unsupported BIND [02] command */
  ADD_DATA(buf, "\x05\x02\x00\x01");

  tt_int_op(fetch_from_buf_socks(buf, socks, get_options()->TestSocks,
                               get_options()->SafeSocks),OP_EQ, 0);
  tt_int_op(0,OP_EQ, buf_datalen(buf));
  tt_int_op(5,OP_EQ, socks->socks_version);
  tt_int_op(2,OP_EQ, socks->replylen);
  tt_int_op(5,OP_EQ, socks->reply[0]);
  tt_int_op(0,OP_EQ, socks->reply[1]);
  ADD_DATA(buf, "\x05\x02\x00\x01\x02\x02\x02\x01\x01\x01");
  tt_int_op(fetch_from_buf_socks(buf, socks, get_options()->TestSocks,
                               get_options()->SafeSocks),OP_EQ, -1);

  tt_int_op(5,OP_EQ,socks->socks_version);
  tt_int_op(10,OP_EQ,socks->replylen);
  tt_int_op(5,OP_EQ,socks->reply[0]);
  tt_int_op(SOCKS5_COMMAND_NOT_SUPPORTED,OP_EQ,socks->reply[1]);
  tt_int_op(1,OP_EQ,socks->reply[3]);

  buf_clear(buf);
  socks_request_clear(socks);

  /* SOCKS 5 Send unsupported UDP_ASSOCIATE [03] command */
  ADD_DATA(buf, "\x05\x02\x00\x01");
  tt_int_op(fetch_from_buf_socks(buf, socks, get_options()->TestSocks,
                               get_options()->SafeSocks),OP_EQ, 0);
  tt_int_op(5,OP_EQ, socks->socks_version);
  tt_int_op(2,OP_EQ, socks->replylen);
  tt_int_op(5,OP_EQ, socks->reply[0]);
  tt_int_op(0,OP_EQ, socks->reply[1]);
  ADD_DATA(buf, "\x05\x03\x00\x01\x02\x02\x02\x01\x01\x01");
  tt_int_op(fetch_from_buf_socks(buf, socks, get_options()->TestSocks,
                               get_options()->SafeSocks),OP_EQ, -1);

  tt_int_op(5,OP_EQ,socks->socks_version);
  tt_int_op(10,OP_EQ,socks->replylen);
  tt_int_op(5,OP_EQ,socks->reply[0]);
  tt_int_op(SOCKS5_COMMAND_NOT_SUPPORTED,OP_EQ,socks->reply[1]);
  tt_int_op(1,OP_EQ,socks->reply[3]);

 done:
  ;
}

/** Perform supported SOCKS 5 commands */
static void
test_socks_5_supported_commands(void *ptr)
{
  SOCKS_TEST_INIT();

  /* SOCKS 5 Send CONNECT [01] to IP address 2.2.2.2:4369 */
  ADD_DATA(buf, "\x05\x01\x00");
  tt_int_op(fetch_from_buf_socks(buf, socks, get_options()->TestSocks,
                                   get_options()->SafeSocks),OP_EQ, 0);
  tt_int_op(5,OP_EQ, socks->socks_version);
  tt_int_op(2,OP_EQ, socks->replylen);
  tt_int_op(5,OP_EQ, socks->reply[0]);
  tt_int_op(0,OP_EQ, socks->reply[1]);

  ADD_DATA(buf, "\x05\x01\x00\x01\x02\x02\x02\x02\x11\x11");
  tt_int_op(fetch_from_buf_socks(buf, socks, get_options()->TestSocks,
                                   get_options()->SafeSocks),OP_EQ, 1);
  tt_str_op("2.2.2.2",OP_EQ, socks->address);
  tt_int_op(4369,OP_EQ, socks->port);

  tt_int_op(0,OP_EQ, buf_datalen(buf));
  socks_request_clear(socks);

  /* SOCKS 5 Send CONNECT [01] to one of the ipv6 addresses for
     torproject.org:80 */
  ADD_DATA(buf, "\x05\x01\x00");
  ADD_DATA(buf, "\x05\x01\x00\x04"
           "\x20\x02\x41\xb8\x02\x02\x0d\xeb\x02\x13\x21\xff\xfe\x20\x14\x26"
           "\x00\x50");
  tt_int_op(fetch_from_buf_socks(buf, socks, get_options()->TestSocks,
                                   get_options()->SafeSocks),OP_EQ, 1);
  tt_int_op(5,OP_EQ, socks->socks_version);
  tt_int_op(2,OP_EQ, socks->replylen);
  tt_int_op(5,OP_EQ, socks->reply[0]);
  tt_int_op(0,OP_EQ, socks->reply[1]);
  tt_str_op("[2002:41b8:202:deb:213:21ff:fe20:1426]",OP_EQ, socks->address);
  tt_int_op(80,OP_EQ, socks->port);

  tt_int_op(0,OP_EQ, buf_datalen(buf));
  socks_request_clear(socks);

  /* SOCKS 5 Send CONNECT [01] to FQDN torproject.org:4369 */
  ADD_DATA(buf, "\x05\x01\x00");
  ADD_DATA(buf, "\x05\x01\x00\x03\x0Etorproject.org\x11\x11");
  tt_int_op(fetch_from_buf_socks(buf, socks, 1,
                                   get_options()->SafeSocks),OP_EQ, 1);

  tt_int_op(5,OP_EQ, socks->socks_version);
  tt_int_op(2,OP_EQ, socks->replylen);
  tt_int_op(5,OP_EQ, socks->reply[0]);
  tt_int_op(0,OP_EQ, socks->reply[1]);
  tt_str_op("torproject.org",OP_EQ, socks->address);
  tt_int_op(4369,OP_EQ, socks->port);

  tt_int_op(0,OP_EQ, buf_datalen(buf));
  socks_request_clear(socks);

  /* SOCKS 5 Send RESOLVE [F0] request for torproject.org:4369 */
  ADD_DATA(buf, "\x05\x01\x00");
  ADD_DATA(buf, "\x05\xF0\x00\x03\x0Etorproject.org\x01\x02");
  tt_int_op(fetch_from_buf_socks(buf, socks, get_options()->TestSocks,
                                 get_options()->SafeSocks),
            OP_EQ, 1);
  tt_int_op(5,OP_EQ, socks->socks_version);
  tt_int_op(2,OP_EQ, socks->replylen);
  tt_int_op(5,OP_EQ, socks->reply[0]);
  tt_int_op(0,OP_EQ, socks->reply[1]);
  tt_str_op("torproject.org",OP_EQ, socks->address);

  tt_int_op(0,OP_EQ, buf_datalen(buf));
  socks_request_clear(socks);

  /* SOCKS 5 Should NOT reject RESOLVE [F0] request for IPv4 address
   * string if SafeSocks is enabled. */

  ADD_DATA(buf, "\x05\x01\x00");
  ADD_DATA(buf, "\x05\xF0\x00\x03\x07");
  ADD_DATA(buf, "8.8.8.8");
  ADD_DATA(buf, "\x11\x11");
  tt_int_op(fetch_from_buf_socks(buf, socks, get_options()->TestSocks, 1),
            OP_EQ, 1);

  tt_str_op("8.8.8.8", OP_EQ, socks->address);
  tt_int_op(4369, OP_EQ, socks->port);

  tt_int_op(0, OP_EQ, buf_datalen(buf));

  socks_request_clear(socks);

  /* SOCKS 5 should NOT reject RESOLVE [F0] request for IPv6 address
   * string if SafeSocks is enabled. */

  ADD_DATA(buf, "\x05\x01\x00");
  ADD_DATA(buf, "\x05\xF0\x00\x03\x29");
  ADD_DATA(buf, "[2001:0db8:85a3:0000:0000:8a2e:0370:7334]");
  ADD_DATA(buf, "\x01\x02");
  tt_int_op(fetch_from_buf_socks(buf, socks, get_options()->TestSocks, 1),
            OP_EQ, 1);

  tt_str_op("[2001:0db8:85a3:0000:0000:8a2e:0370:7334]", OP_EQ,
    socks->address);
  tt_int_op(258, OP_EQ, socks->port);

  tt_int_op(0, OP_EQ, buf_datalen(buf));

  socks_request_clear(socks);

  /* Also allow bracket-less form. */

  ADD_DATA(buf, "\x05\x01\x00");
  ADD_DATA(buf, "\x05\xF0\x00\x03\x27");
  ADD_DATA(buf, "2001:0db8:85a3:0000:0000:8a2e:0370:7334");
  ADD_DATA(buf, "\x01\x02");
  tt_int_op(fetch_from_buf_socks(buf, socks, get_options()->TestSocks, 1),
            OP_EQ, 1);

  tt_str_op("2001:0db8:85a3:0000:0000:8a2e:0370:7334", OP_EQ,
    socks->address);
  tt_int_op(258, OP_EQ, socks->port);

  tt_int_op(0, OP_EQ, buf_datalen(buf));

  socks_request_clear(socks);

  /* SOCKS 5 Send RESOLVE_PTR [F1] for IP address 2.2.2.5 */
  ADD_DATA(buf, "\x05\x01\x00");
  ADD_DATA(buf, "\x05\xF1\x00\x01\x02\x02\x02\x05\x01\x03");
  tt_int_op(fetch_from_buf_socks(buf, socks, get_options()->TestSocks,
                                 get_options()->SafeSocks),
            OP_EQ, 1);
  tt_int_op(5,OP_EQ, socks->socks_version);
  tt_int_op(2,OP_EQ, socks->replylen);
  tt_int_op(5,OP_EQ, socks->reply[0]);
  tt_int_op(0,OP_EQ, socks->reply[1]);
  tt_str_op("2.2.2.5",OP_EQ, socks->address);

  tt_int_op(0,OP_EQ, buf_datalen(buf));

  socks_request_clear(socks);

  /* SOCKS 5 Send RESOLVE_PTR [F1] for an IPv6 address */
  ADD_DATA(buf, "\x05\x01\x00");
  ADD_DATA(buf, "\x05\xF1\x00\x04"
           "\x20\x01\x0d\xb8\x85\xa3\x00\x00\x00\x00\x8a\x2e\x03\x70\x73\x34"
           "\x12\x34");
  tt_int_op(fetch_from_buf_socks(buf, socks, get_options()->TestSocks,
                                 get_options()->SafeSocks),
            OP_EQ, 1);
  tt_int_op(5,OP_EQ, socks->socks_version);
  tt_int_op(2,OP_EQ, socks->replylen);
  tt_int_op(5,OP_EQ, socks->reply[0]);
  tt_int_op(0,OP_EQ, socks->reply[1]);
  tt_str_op("[2001:db8:85a3::8a2e:370:7334]",OP_EQ, socks->address);

  tt_int_op(0,OP_EQ, buf_datalen(buf));

  socks_request_clear(socks);

  /* SOCKS 5 Send RESOLVE_PTR [F1] for a an IPv6 address written as a
   * string with brackets */
  ADD_DATA(buf, "\x05\x01\x00");
  ADD_DATA(buf, "\x05\xF1\x00\x03\x1e");
  ADD_DATA(buf, "[2001:db8:85a3::8a2e:370:7334]");
  ADD_DATA(buf, "\x12\x34");
  tt_int_op(fetch_from_buf_socks(buf, socks, get_options()->TestSocks,
                                 get_options()->SafeSocks),
            OP_EQ, 1);
  tt_int_op(5,OP_EQ, socks->socks_version);
  tt_int_op(2,OP_EQ, socks->replylen);
  tt_int_op(5,OP_EQ, socks->reply[0]);
  tt_int_op(0,OP_EQ, socks->reply[1]);
  tt_str_op("[2001:db8:85a3::8a2e:370:7334]",OP_EQ, socks->address);

  tt_int_op(0,OP_EQ, buf_datalen(buf));

 done:
  ;
}

/**  Perform SOCKS 5 authentication */
static void
test_socks_5_no_authenticate(void *ptr)
{
  SOCKS_TEST_INIT();

  /*SOCKS 5 No Authentication */
  ADD_DATA(buf,"\x05\x01\x00");
  tt_assert(!fetch_from_buf_socks(buf, socks,
                                    get_options()->TestSocks,
                                    get_options()->SafeSocks));
  tt_int_op(2,OP_EQ, socks->replylen);
  tt_int_op(5,OP_EQ, socks->reply[0]);
  tt_int_op(SOCKS_NO_AUTH,OP_EQ, socks->reply[1]);

  tt_int_op(0,OP_EQ, buf_datalen(buf));

  /*SOCKS 5 Send username/password anyway - pretend to be broken */
  ADD_DATA(buf,"\x01\x02\x01\x01\x02\x01\x01");
  tt_assert(!fetch_from_buf_socks(buf, socks,
                                    get_options()->TestSocks,
                                    get_options()->SafeSocks));
  tt_int_op(5,OP_EQ, socks->socks_version);
  tt_int_op(2,OP_EQ, socks->replylen);
  tt_int_op(1,OP_EQ, socks->reply[0]);
  tt_int_op(0,OP_EQ, socks->reply[1]);

  tt_int_op(2,OP_EQ, socks->usernamelen);
  tt_int_op(2,OP_EQ, socks->passwordlen);

  tt_mem_op("\x01\x01",OP_EQ, socks->username, 2);
  tt_mem_op("\x01\x01",OP_EQ, socks->password, 2);

 done:
  ;
}

/** Perform SOCKS 5 authentication */
static void
test_socks_5_authenticate(void *ptr)
{
  SOCKS_TEST_INIT();

  /* SOCKS 5 Negotiate username/password authentication */
  ADD_DATA(buf, "\x05\x01\x02");

  tt_assert(!fetch_from_buf_socks(buf, socks,
                                   get_options()->TestSocks,
                                   get_options()->SafeSocks));
  tt_int_op(2,OP_EQ, socks->replylen);
  tt_int_op(5,OP_EQ, socks->reply[0]);
  tt_int_op(SOCKS_USER_PASS,OP_EQ, socks->reply[1]);
  tt_int_op(5,OP_EQ, socks->socks_version);

  tt_int_op(0,OP_EQ, buf_datalen(buf));

  /* SOCKS 5 Send username/password */
  ADD_DATA(buf, "\x01\x02me\x08mypasswd");
  tt_assert(!fetch_from_buf_socks(buf, socks,
                                   get_options()->TestSocks,
                                   get_options()->SafeSocks));
  tt_int_op(5,OP_EQ, socks->socks_version);
  tt_int_op(2,OP_EQ, socks->replylen);
  tt_int_op(1,OP_EQ, socks->reply[0]);
  tt_int_op(0,OP_EQ, socks->reply[1]);

  tt_int_op(2,OP_EQ, socks->usernamelen);
  tt_int_op(8,OP_EQ, socks->passwordlen);

  tt_mem_op("me",OP_EQ, socks->username, 2);
  tt_mem_op("mypasswd",OP_EQ, socks->password, 8);

 done:
  ;
}

/** Perform SOCKS 5 authentication with empty username/password fields.
 * Technically this violates RfC 1929, but some client software will send
 * this kind of message to Tor.
 * */
static void
test_socks_5_authenticate_empty_user_pass(void *ptr)
{
  SOCKS_TEST_INIT();

  /* SOCKS 5 Negotiate username/password authentication */
  ADD_DATA(buf, "\x05\x01\x02");

  tt_assert(!fetch_from_buf_socks(buf, socks,
                                   get_options()->TestSocks,
                                   get_options()->SafeSocks));
  tt_int_op(2,OP_EQ, socks->replylen);
  tt_int_op(5,OP_EQ, socks->reply[0]);
  tt_int_op(SOCKS_USER_PASS,OP_EQ, socks->reply[1]);
  tt_int_op(5,OP_EQ, socks->socks_version);

  tt_int_op(0,OP_EQ, buf_datalen(buf));

  /* SOCKS 5 Send username/password auth message with empty user/pass fields */
  ADD_DATA(buf, "\x01\x00\x00");
  tt_assert(!fetch_from_buf_socks(buf, socks,
                                   get_options()->TestSocks,
                                   get_options()->SafeSocks));
  tt_int_op(5,OP_EQ, socks->socks_version);
  tt_int_op(2,OP_EQ, socks->replylen);
  tt_int_op(1,OP_EQ, socks->reply[0]);
  tt_int_op(0,OP_EQ, socks->reply[1]);

  tt_int_op(0,OP_EQ, socks->usernamelen);
  tt_int_op(0,OP_EQ, socks->passwordlen);

 done:
  ;
}
/** Perform SOCKS 5 authentication and send data all in one go */
static void
test_socks_5_authenticate_with_data(void *ptr)
{
  SOCKS_TEST_INIT();

  /* SOCKS 5 Negotiate username/password authentication */
  ADD_DATA(buf, "\x05\x01\x02");

  tt_assert(!fetch_from_buf_socks(buf, socks,
                                   get_options()->TestSocks,
                                   get_options()->SafeSocks));
  tt_int_op(2,OP_EQ, socks->replylen);
  tt_int_op(5,OP_EQ, socks->reply[0]);
  tt_int_op(SOCKS_USER_PASS,OP_EQ, socks->reply[1]);
  tt_int_op(5,OP_EQ, socks->socks_version);

  tt_int_op(0,OP_EQ, buf_datalen(buf));

  /* SOCKS 5 Send username/password */
  /* SOCKS 5 Send CONNECT [01] to IP address 2.2.2.2:4369 */
  ADD_DATA(buf, "\x01\x02me\x03you\x05\x01\x00\x01\x02\x02\x02\x02\x11\x11");
  tt_int_op(fetch_from_buf_socks(buf, socks, get_options()->TestSocks,
                                 get_options()->SafeSocks),
            OP_EQ, 1);
  tt_int_op(5,OP_EQ, socks->socks_version);
  tt_int_op(2,OP_EQ, socks->replylen);
  tt_int_op(1,OP_EQ, socks->reply[0]);
  tt_int_op(0,OP_EQ, socks->reply[1]);

  tt_str_op("2.2.2.2",OP_EQ, socks->address);
  tt_int_op(4369,OP_EQ, socks->port);

  tt_int_op(2,OP_EQ, socks->usernamelen);
  tt_int_op(3,OP_EQ, socks->passwordlen);
  tt_mem_op("me",OP_EQ, socks->username, 2);
  tt_mem_op("you",OP_EQ, socks->password, 3);

 done:
  ;
}

/** Try to negotiate an unsupported authentication type */
static void
test_socks_5_auth_unsupported_type(void *ptr)
{
  SOCKS_TEST_INIT();

  /* None of these authentication types are recognized. */
  ADD_DATA(buf, "\x05\x03\x99\x21\x10");
  tt_int_op(fetch_from_buf_socks(buf, socks, get_options()->TestSocks,
                                 get_options()->SafeSocks),
            OP_EQ, -1);
  tt_int_op(0,OP_EQ, socks->socks_version);
  tt_int_op(2,OP_EQ, socks->replylen);
  tt_int_op(5,OP_EQ, socks->reply[0]);
  tt_int_op(0xff,OP_EQ, socks->reply[1]);

 done:
  ;
}

/** Try to negotiate an unsupported version of username/password auth. */
static void
test_socks_5_auth_unsupported_version(void *ptr)
{
  SOCKS_TEST_INIT();

  /* Negotiate username/password */
  ADD_DATA(buf, "\x05\x01\x02");
  tt_int_op(fetch_from_buf_socks(buf, socks, get_options()->TestSocks,
                                 get_options()->SafeSocks),
            OP_EQ, 0);
  tt_int_op(0,OP_EQ, buf_datalen(buf)); /* buf should be drained */
  /* Now, suggest an unrecognized username/password version */
  ADD_DATA(buf, "\x02\x05" "hello" "\x05" "world");
  tt_int_op(fetch_from_buf_socks(buf, socks, get_options()->TestSocks,
                                 get_options()->SafeSocks),
            OP_EQ, -1);

 done:
  ;
}

/** Perform SOCKS 5 authentication before method negotiated */
static void
test_socks_5_auth_before_negotiation(void *ptr)
{
  SOCKS_TEST_INIT();

  /* SOCKS 5 Send username/password */
  ADD_DATA(buf, "\x01\x02me\x02me");
  tt_int_op(fetch_from_buf_socks(buf, socks, get_options()->TestSocks,
                                 get_options()->SafeSocks),
            OP_EQ, -1);
  tt_int_op(0,OP_EQ, socks->socks_version);
  tt_int_op(0,OP_EQ, socks->replylen);
  tt_int_op(0,OP_EQ, socks->reply[0]);
  tt_int_op(0,OP_EQ, socks->reply[1]);

 done:
  ;
}

/** Perform malformed SOCKS 5 commands */
static void
test_socks_5_malformed_commands(void *ptr)
{
  SOCKS_TEST_INIT();

  /* XXX: Stringified address length > MAX_SOCKS_ADDR_LEN will never happen */

  /** SOCKS 5 Send CONNECT [01] to IP address 2.2.2.2:4369, with SafeSocks set
   */
  ADD_DATA(buf, "\x05\x01\x00");
  ADD_DATA(buf, "\x05\x01\x00\x01\x02\x02\x02\x02\x11\x11");
  tt_int_op(fetch_from_buf_socks(buf, socks, get_options()->TestSocks, 1),
            OP_EQ, -1);

  tt_int_op(5,OP_EQ,socks->socks_version);
  tt_int_op(10,OP_EQ,socks->replylen);
  tt_int_op(5,OP_EQ,socks->reply[0]);
  tt_int_op(SOCKS5_NOT_ALLOWED,OP_EQ,socks->reply[1]);
  tt_int_op(1,OP_EQ,socks->reply[3]);

  buf_clear(buf);
  socks_request_clear(socks);

  /* SOCKS 5 Send RESOLVE_PTR [F1] for FQDN torproject.org */
  ADD_DATA(buf, "\x05\x01\x00");
  ADD_DATA(buf, "\x05\xF1\x00\x03\x0Etorproject.org\x11\x11");
  tt_int_op(fetch_from_buf_socks(buf, socks, get_options()->TestSocks,
                                   get_options()->SafeSocks),OP_EQ, -1);

  tt_int_op(5,OP_EQ,socks->socks_version);
  tt_int_op(10,OP_EQ,socks->replylen);
  tt_int_op(5,OP_EQ,socks->reply[0]);
  tt_int_op(SOCKS5_ADDRESS_TYPE_NOT_SUPPORTED,OP_EQ,socks->reply[1]);
  tt_int_op(1,OP_EQ,socks->reply[3]);

  buf_clear(buf);
  socks_request_clear(socks);

  /* XXX: len + 1 > MAX_SOCKS_ADDR_LEN (FQDN request) will never happen */

  /* SOCKS 5 Send CONNECT [01] to FQDN """"".com */
  ADD_DATA(buf, "\x05\x01\x00");
  ADD_DATA(buf, "\x05\x01\x00\x03\x09\"\"\"\"\".com\x11\x11");
  tt_int_op(fetch_from_buf_socks(buf, socks, get_options()->TestSocks,
                                   get_options()->SafeSocks),OP_EQ, -1);

  tt_int_op(5,OP_EQ,socks->socks_version);
  tt_int_op(10,OP_EQ,socks->replylen);
  tt_int_op(5,OP_EQ,socks->reply[0]);
  tt_int_op(SOCKS5_GENERAL_ERROR,OP_EQ,socks->reply[1]);
  tt_int_op(1,OP_EQ,socks->reply[3]);

  buf_clear(buf);
  socks_request_clear(socks);

  /* SOCKS 5 Send CONNECT [01] to address type 0x23 */
  ADD_DATA(buf, "\x05\x01\x00");
  ADD_DATA(buf, "\x05\x01\x00\x23\x02\x02\x02\x02\x11\x11");
  tt_int_op(fetch_from_buf_socks(buf, socks, get_options()->TestSocks,
                                   get_options()->SafeSocks),OP_EQ, -1);

  tt_int_op(5,OP_EQ,socks->socks_version);
  tt_int_op(10,OP_EQ,socks->replylen);
  tt_int_op(5,OP_EQ,socks->reply[0]);
  /* trunnel parsing will fail with -1 */
  tt_int_op(SOCKS5_GENERAL_ERROR,OP_EQ,socks->reply[1]);
  tt_int_op(1,OP_EQ,socks->reply[3]);

 done:
  ;
}

static void
test_socks_5_bad_arguments(void *ptr)
{
  SOCKS_TEST_INIT();
  setup_capture_of_logs(LOG_DEBUG);

  /* Socks5, bogus hostname */
  ADD_DATA(buf, "\x05\x01\x00" "\x05\x01\x00\x03\x03" "---" "\x00\x50" );
  tt_int_op(fetch_from_buf_socks(buf, socks, 1, 0), OP_EQ, -1);
  buf_clear(buf);
  expect_log_msg_containing("Your application (using socks5 to port 80) "
                            "gave Tor a malformed hostname: ");
  mock_clean_saved_logs();
  socks_request_clear(socks);

 done:
  teardown_capture_of_logs();
}

/** check for correct behavior when the socks command has not arrived. */
static void
test_socks_truncated(void *ptr)
{
  const struct {
    enum { NONE, AUTH, ALL } setup;
    const char *body;
    size_t len;
  } commands[] = {
    /* SOCKS4 */
    /* Connect, to an IP. */
    { NONE, "\x04\x01\x05\x05\x01\x02\x03\x04\x00", 9},
    /* Connect, to an IP, with authentication. */
    { NONE, "\x04\x01\x05\x05\x01\x02\x03\x04hello\x00", 14},
    /* SOCKS4A */
    /* Connect, to a hostname */
    { NONE, "\x04\x01\x09\x09\x00\x00\x00\x01\x00www.example.com\x00", 25},
    /* Connect, to a hostname, with authentication */
    { NONE, "\x04\x01\x09\x09\x00\x00\x00\x01hi\x00www.example.com\x00", 27},
    /* SOCKS5 */
    /* initial handshake */
    { NONE, "\x05\x00", 2 },
    /* no-auth handshake */
    { NONE, "\x05\x03\x99\x21\x10", 5 },
    /* SOCSK5, username-password, all empty. */
    { AUTH, "\x01\x00\x00", 3 },
    /* SOCSK5, username-password, 1 char each. */
    { AUTH, "\x01\x01x\x01y", 5 },
    /* SOCSK5, username-password, max length. */
    { AUTH, "\x01\xff"
      "Ogni tempo ha il suo fascismo: se ne notano i segni premonitori "
      "dovunque la concentrazione di potere nega al cittadino la "
      "possibilit\xc3\xa0 e la capacit\xc3\xa0 di esprimere ed attuare la "
      "sua volont\xc3\xa0. A questo si arriva in molti modi, non "
      "necessariamente col terror"
      "\xff"
      "e dell'intimidazione poliziesca, ma anche negando o distorcendo "
      "l'informazione, inquinando la giustizia, paralizzando la scuola, "
      "diffondendo in molti modi sottili la nostalgia per un mondo in cui "
      "regnava sovrano l'ordine, ed in cui la sicurezza dei pochi "
      /* privilegiati riposava sul lavoro forzato e sul silenzio forzato dei
         molti. -- Primo Levi */ , 513 },
    /* Socks5, IPv4 address */
    { ALL, "\x05\x01\x00\x01\x01\x02\x03\x04\x20\x20", 10 },
    /* Socks5, IPv6 address */
    { ALL, "\x05\x01\x00\x04"
      "\x49\x20\x48\x41\x5a\x20\x45\x41\x53\x54\x45\x52\x20\x45\x47\x47"
      "\x20\x20", 22 },
    /* Socks5, hostname, empty. */
    { ALL, "\x05\x01\x00\x03" "\x00" "\x00\x50", 7 },
    /* Socks5, hostname, moderate. */
    { ALL, "\x05\x01\x00\x03" "\x11" "onion.example.com" "\x00\x50", 24 },
    /* Socks5, hostname, maximum. */
    { ALL, "\x05\x01\x00\x03" "\xff"
      "whatsoever.I.shall.see.or.hear.in.the.course.of.my.profession.as.well."
      "as.outside.my.profession.in.my.intercourse.with.men.if.it.be.what."
      "should.not.be.published.abroad.I.will.never.divulge.holding.such."
      "things.to.be.holy.secrets.x.hippocratic.oath.wikipedia"
      "\x00\x50", 262 },
  };
  unsigned i, j;
  SOCKS_TEST_INIT();
  for (i = 0; i < ARRAY_LENGTH(commands); ++i) {
    for (j = 0; j < commands[i].len; ++j) {
      switch (commands[i].setup) {
        default: FALLTHROUGH;
        case NONE:
          /* This test calls for no setup on the socks state. */
          break;
        case AUTH:
          /* This test calls for the socks state to be waiting for
           * username/password authentication */
          ADD_DATA(buf, "\x05\x01\x02");
          tt_int_op(0, OP_EQ, fetch_from_buf_socks(buf, socks, 0, 0));
          tt_int_op(0, OP_EQ, buf_datalen(buf));
          break;
        case ALL:
          /* This test calls for the socks state to be waiting for
           * the connection request */
          ADD_DATA(buf, "\x05\x01\x00");
          tt_int_op(0, OP_EQ, fetch_from_buf_socks(buf, socks, 0, 0));
          tt_int_op(0, OP_EQ, buf_datalen(buf));
      }

      TT_BLATHER(("Checking command %u, length %u, omitting char %u", i, j,
                  (unsigned)commands[i].body[j]));
      buf_add(buf, commands[i].body, j);
      /* This should return 0 meaning "not done yet" */
      tt_int_op(0, OP_EQ, fetch_from_buf_socks(buf, socks, 0, 0));
      tt_uint_op(j, OP_EQ, buf_datalen(buf)); /* Nothing was drained */
      buf_clear(buf);
      socks_request_free(testdata->req);
      socks = testdata->req = socks_request_new();
    }
  }
  done:
  ;
}

static void
test_socks_wrong_protocol(void *ptr)
{
  SOCKS_TEST_INIT();
  setup_capture_of_logs(LOG_DEBUG);

  /* HTTP request. */
  ADD_DATA(buf, "GET /index.html HTTP/1.0" );
  tt_int_op(fetch_from_buf_socks(buf, socks, 1, 0), OP_EQ, -1);
  buf_clear(buf);
  expect_log_msg_containing("Socks version 71 not recognized. "
                            "(This port is not an HTTP proxy;");
  mock_clean_saved_logs();
  socks_request_clear(socks);

 done:
  teardown_capture_of_logs();
}

/* Check our client-side socks4 parsing (that is to say, our parsing of
 * server responses).
 */
static void
test_socks_client_v4(void *arg)
{
  (void)arg;
  buf_t *buf = buf_new();
  char *reason = NULL;

  /* Legit socks4 response, success */
  ADD_DATA(buf, "\x04\x5a\x20\x25\x01\x02\x03\x04");
  tt_int_op(1, OP_EQ,
            fetch_from_buf_socks_client(buf, PROXY_SOCKS4_WANT_CONNECT_OK,
                                        &reason));
  tt_ptr_op(reason, OP_EQ, NULL);
  tt_int_op(buf_datalen(buf), OP_EQ, 0);

  /* Legit socks4 response, failure. */
  ADD_DATA(buf, "\x04\x5b\x20\x25\x01\x02\x03\x04");
  tt_int_op(-1, OP_EQ,
            fetch_from_buf_socks_client(buf, PROXY_SOCKS4_WANT_CONNECT_OK,
                                        &reason));
  tt_ptr_op(reason, OP_NE, NULL);
  tt_str_op(reason, OP_EQ, "server rejected connection");

 done:
  buf_free(buf);
  tor_free(reason);
}

/* Check our client-side socks5 authentication-negotiation parsing (that is to
 * say, our parsing of server responses).
 */
static void
test_socks_client_v5_auth(void *arg)
{
  (void)arg;
  buf_t *buf = buf_new();
  char *reason = NULL;

  /* Legit socks5 responses, got a method we like. */
  ADD_DATA(buf, "\x05\x00");
  tt_int_op(1, OP_EQ,
            fetch_from_buf_socks_client(buf,
                                        PROXY_SOCKS5_WANT_AUTH_METHOD_NONE,
                                        &reason));
  tt_ptr_op(reason, OP_EQ, NULL);
  tt_int_op(buf_datalen(buf), OP_EQ, 0);

  /* Same, but we wanted something else. */
  ADD_DATA(buf, "\x05\x00");
  tt_int_op(1, OP_EQ,
            fetch_from_buf_socks_client(buf,
                                        PROXY_SOCKS5_WANT_AUTH_METHOD_RFC1929,
                                        &reason));
  tt_ptr_op(reason, OP_EQ, NULL);
  tt_int_op(buf_datalen(buf), OP_EQ, 0);

  /* Same, and they offered a password. */
  ADD_DATA(buf, "\x05\x02");
  tt_int_op(2, OP_EQ,
            fetch_from_buf_socks_client(buf,
                                        PROXY_SOCKS5_WANT_AUTH_METHOD_RFC1929,
                                        &reason));
  tt_ptr_op(reason, OP_EQ, NULL);
  tt_int_op(buf_datalen(buf), OP_EQ, 0);

  /* They rejected our method, or selected something we don't know. */
  ADD_DATA(buf, "\x05\xff");
  tt_int_op(-1, OP_EQ,
            fetch_from_buf_socks_client(buf,
                                        PROXY_SOCKS5_WANT_AUTH_METHOD_NONE,
                                        &reason));
  tt_str_op(reason, OP_EQ, "server doesn't support any of our available "
            "authentication methods");
  buf_clear(buf);
  tor_free(reason);
  ADD_DATA(buf, "\x05\xff");
  tt_int_op(-1, OP_EQ,
            fetch_from_buf_socks_client(buf,
                                        PROXY_SOCKS5_WANT_AUTH_METHOD_RFC1929,
                                        &reason));
  tt_str_op(reason, OP_EQ, "server doesn't support any of our available "
            "authentication methods");
  tor_free(reason);
  buf_clear(buf);

  /* Now check for authentication responses: check success and failure. */
  ADD_DATA(buf, "\x01\x00");
  tt_int_op(1, OP_EQ,
            fetch_from_buf_socks_client(buf,
                                        PROXY_SOCKS5_WANT_AUTH_RFC1929_OK,
                                        &reason));
  tt_ptr_op(reason, OP_EQ, NULL);
  tt_int_op(buf_datalen(buf), OP_EQ, 0);

  ADD_DATA(buf, "\x01\xf0");
  tt_int_op(-1, OP_EQ,
            fetch_from_buf_socks_client(buf,
                                        PROXY_SOCKS5_WANT_AUTH_RFC1929_OK,
                                        &reason));
  tt_ptr_op(reason, OP_NE, NULL);
  tt_str_op(reason, OP_EQ, "authentication failed");

 done:
  buf_free(buf);
  tor_free(reason);
}

/* Check our client-side socks5 connect parsing (that is to say, our parsing
 * of server responses).
 */
static void
test_socks_client_v5_connect(void *arg)
{
  (void)arg;
  buf_t *buf = buf_new();
  char *reason = NULL;

  /* Legit socks5 responses, success, ipv4. */
  ADD_DATA(buf, "\x05\x00\x00\x01\x01\x02\x03\x04\x00\x05");
  tt_int_op(1, OP_EQ,
            fetch_from_buf_socks_client(buf,
                                        PROXY_SOCKS5_WANT_CONNECT_OK,
                                        &reason));
  tt_ptr_op(reason, OP_EQ, NULL);
  tt_int_op(buf_datalen(buf), OP_EQ, 0);

  /* Legit socks5 responses, success, ipv6. */
  ADD_DATA(buf, "\x05\x00\x00\x04"
           "abcdefghijklmnop"
           "\x00\x05");
  tt_int_op(1, OP_EQ,
            fetch_from_buf_socks_client(buf,
                                        PROXY_SOCKS5_WANT_CONNECT_OK,
                                        &reason));
  tt_ptr_op(reason, OP_EQ, NULL);
  tt_int_op(buf_datalen(buf), OP_EQ, 0);

  /* Legit socks5 responses, success, hostname. */
  ADD_DATA(buf, "\x05\x00\x00\x03\x12"
           "gopher.example.com"
           "\x00\x05");
  tt_int_op(1, OP_EQ,
            fetch_from_buf_socks_client(buf,
                                        PROXY_SOCKS5_WANT_CONNECT_OK,
                                        &reason));
  tt_ptr_op(reason, OP_EQ, NULL);
  tt_int_op(buf_datalen(buf), OP_EQ, 0);

  /* Legit socks5 responses, failure, hostname. */
  ADD_DATA(buf, "\x05\x03\x00\x03\x12"
           "gopher.example.com"
           "\x00\x05");
  tt_int_op(-1, OP_EQ,
            fetch_from_buf_socks_client(buf,
                                        PROXY_SOCKS5_WANT_CONNECT_OK,
                                        &reason));
  tt_ptr_op(reason, OP_NE, NULL);
  tt_str_op(reason, OP_EQ, "Network unreachable");
  tor_free(reason);
  buf_clear(buf);

  /* Bogus socks5 responses: what is address type 0x17? */
  ADD_DATA(buf, "\x05\x03\x00\x17\x12 blah blah");
  tt_int_op(-1, OP_EQ,
            fetch_from_buf_socks_client(buf,
                                        PROXY_SOCKS5_WANT_CONNECT_OK,
                                        &reason));
  tt_ptr_op(reason, OP_NE, NULL);
  tt_str_op(reason, OP_EQ, "invalid response to connect request");
  buf_clear(buf);

 done:
  buf_free(buf);
  tor_free(reason);
}

static void
test_socks_client_truncated(void *arg)
{
  (void)arg;
  buf_t *buf = buf_new();
  char *reason = NULL;

#define S(str) str, (sizeof(str)-1)
  const struct {
    int state;
    const char *body;
    size_t len;
  } replies[] = {
    { PROXY_SOCKS4_WANT_CONNECT_OK, S("\x04\x5a\x20\x25\x01\x02\x03\x04") },
    { PROXY_SOCKS4_WANT_CONNECT_OK, S("\x04\x5b\x20\x25\x01\x02\x03\x04") },
    { PROXY_SOCKS5_WANT_AUTH_METHOD_NONE, S("\x05\x00") },
    { PROXY_SOCKS5_WANT_AUTH_METHOD_RFC1929, S("\x05\x00") },
    { PROXY_SOCKS5_WANT_AUTH_RFC1929_OK, S("\x01\x00") },
    { PROXY_SOCKS5_WANT_CONNECT_OK,
      S("\x05\x00\x00\x01\x01\x02\x03\x04\x00\x05") },
    { PROXY_SOCKS5_WANT_CONNECT_OK,
      S("\x05\x00\x00\x04" "abcdefghijklmnop" "\x00\x05") },
    { PROXY_SOCKS5_WANT_CONNECT_OK,
      S("\x05\x00\x00\x03\x12" "gopher.example.com" "\x00\x05") },
    { PROXY_SOCKS5_WANT_CONNECT_OK,
      S("\x05\x03\x00\x03\x12" "gopher.example.com""\x00\x05") },
    { PROXY_SOCKS5_WANT_CONNECT_OK,
      S("\x05\x03\x00\x17") },
  };
  unsigned i, j;
  for (i = 0; i < ARRAY_LENGTH(replies); ++i) {
    for (j = 0; j < replies[i].len; ++j) {
      TT_BLATHER(("Checking command %u, length %u", i, j));
      buf_add(buf, replies[i].body, j);
      /* This should return 0 meaning "not done yet" */
      tt_int_op(0, OP_EQ,
                fetch_from_buf_socks_client(buf, replies[i].state, &reason));
      tt_uint_op(j, OP_EQ, buf_datalen(buf)); /* Nothing was drained */
      buf_clear(buf);
      tt_ptr_op(reason, OP_EQ, NULL);
    }
  }

 done:
  tor_free(reason);
  buf_free(buf);
}

#define SOCKSENT(name)                                  \
  { #name, test_socks_##name, TT_FORK, &socks_setup, NULL }

struct testcase_t socks_tests[] = {
  SOCKSENT(4_unsupported_commands),
  SOCKSENT(4_supported_commands),
  SOCKSENT(4_bad_arguments),

  SOCKSENT(5_unsupported_commands),
  SOCKSENT(5_supported_commands),
  SOCKSENT(5_no_authenticate),
  SOCKSENT(5_auth_unsupported_type),
  SOCKSENT(5_auth_unsupported_version),
  SOCKSENT(5_auth_before_negotiation),
  SOCKSENT(5_authenticate),
  SOCKSENT(5_authenticate_empty_user_pass),
  SOCKSENT(5_authenticate_with_data),
  SOCKSENT(5_malformed_commands),
  SOCKSENT(5_bad_arguments),

  SOCKSENT(truncated),

  SOCKSENT(wrong_protocol),

  { "client/v4", test_socks_client_v4, TT_FORK, NULL, NULL },
  { "client/v5_auth", test_socks_client_v5_auth, TT_FORK, NULL, NULL },
  { "client/v5_connect", test_socks_client_v5_connect, TT_FORK, NULL, NULL },
  { "client/truncated", test_socks_client_truncated, TT_FORK, NULL, NULL },

  END_OF_TESTCASES
};
