/* Copyright (c) 2015-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#define CONTROL_CMD_PRIVATE
#define CONTROL_GETINFO_PRIVATE
#include "core/or/or.h"
#include "app/config/config.h"
#include "lib/crypt_ops/crypto_ed25519.h"
#include "feature/client/bridges.h"
#include "feature/control/control.h"
#include "feature/control/control_cmd.h"
#include "feature/control/control_getinfo.h"
#include "feature/control/control_proto.h"
#include "feature/client/entrynodes.h"
#include "feature/dircache/cached_dir_st.h"
#include "feature/dircache/dirserv.h"
#include "feature/hs/hs_common.h"
#include "feature/nodelist/networkstatus.h"
#include "feature/rend/rendservice.h"
#include "feature/nodelist/authcert.h"
#include "feature/nodelist/nodelist.h"
#include "feature/stats/rephist.h"
#include "test/test.h"
#include "test/test_helpers.h"
#include "lib/net/resolve.h"
#include "lib/encoding/confline.h"
#include "lib/encoding/kvline.h"

#include "feature/control/control_connection_st.h"
#include "feature/control/control_cmd_args_st.h"
#include "feature/dirclient/download_status_st.h"
#include "feature/nodelist/microdesc_st.h"
#include "feature/nodelist/node_st.h"

typedef struct {
  const char *input;
  const char *expected_parse;
  const char *expected_error;
} parser_testcase_t;

typedef struct {
  const control_cmd_syntax_t *syntax;
  size_t n_testcases;
  const parser_testcase_t *testcases;
} parse_test_params_t;

static char *
control_cmd_dump_args(const control_cmd_args_t *result)
{
  buf_t *buf = buf_new();
  buf_add_string(buf, "{ args=[");
  if (result->args) {
    if (smartlist_len(result->args)) {
        buf_add_string(buf, " ");
    }
    SMARTLIST_FOREACH_BEGIN(result->args, const char *, s) {
      const bool last = (s_sl_idx == smartlist_len(result->args)-1);
      buf_add_printf(buf, "%s%s ",
                     escaped(s),
                     last ? "" : ",");
    } SMARTLIST_FOREACH_END(s);
  }
  buf_add_string(buf, "]");
  if (result->cmddata) {
    buf_add_string(buf, ", obj=");
    buf_add_string(buf, escaped(result->cmddata));
  }
  if (result->kwargs) {
    buf_add_string(buf, ", { ");
    const config_line_t *line;
    for (line = result->kwargs; line; line = line->next) {
      const bool last = (line->next == NULL);
      buf_add_printf(buf, "%s=%s%s ", line->key, escaped(line->value),
                     last ? "" : ",");
    }
    buf_add_string(buf, "}");
  }
  buf_add_string(buf, " }");

  char *encoded = buf_extract(buf, NULL);
  buf_free(buf);
  return encoded;
}

static void
test_controller_parse_cmd(void *arg)
{
  const parse_test_params_t *params = arg;
  control_cmd_args_t *result = NULL;
  char *error = NULL;
  char *encoded = NULL;

  for (size_t i = 0; i < params->n_testcases; ++i) {
    const parser_testcase_t *t = &params->testcases[i];
    result = control_cmd_parse_args("EXAMPLE",
                                    params->syntax,
                                    strlen(t->input),
                                    t->input,
                                    &error);
    // A valid test should expect exactly one parse or error.
    tt_int_op((t->expected_parse == NULL), OP_NE,
              (t->expected_error == NULL));
    // We get a result or an error, not both.
    tt_int_op((result == NULL), OP_EQ, (error != NULL));
    // We got the one we expected.
    tt_int_op((result == NULL), OP_EQ, (t->expected_parse == NULL));

    if (result) {
      encoded = control_cmd_dump_args(result);
      tt_str_op(encoded, OP_EQ, t->expected_parse);
    } else {
      tt_str_op(error, OP_EQ, t->expected_error);
    }

    tor_free(error);
    tor_free(encoded);
    control_cmd_args_free(result);
  }

 done:
  tor_free(error);
  tor_free(encoded);
  control_cmd_args_free(result);
}

#ifndef COCCI
#define OK(inp, out) \
  { inp "\r\n", out, NULL }
#define ERR(inp, err) \
  { inp "\r\n", NULL, err }

#define TESTPARAMS(syntax, array)                \
  { &syntax,                                     \
      ARRAY_LENGTH(array),                       \
      array }
#endif /* !defined(COCCI) */

static const parser_testcase_t one_to_three_tests[] = {
   ERR("", "Need at least 1 argument(s)"),
   ERR("   \t", "Need at least 1 argument(s)"),
   OK("hello", "{ args=[ \"hello\" ] }"),
   OK("hello world", "{ args=[ \"hello\", \"world\" ] }"),
   OK("hello  world", "{ args=[ \"hello\", \"world\" ] }"),
   OK("  hello  world", "{ args=[ \"hello\", \"world\" ] }"),
   OK("  hello  world      ", "{ args=[ \"hello\", \"world\" ] }"),
   OK("hello there world", "{ args=[ \"hello\", \"there\", \"world\" ] }"),
   ERR("why hello there world", "Cannot accept more than 3 argument(s)"),
   ERR("hello\r\nworld.\r\n.", "Unexpected body"),
};

static const control_cmd_syntax_t one_to_three_syntax = {
   .min_args=1, .max_args=3
};

static const parse_test_params_t parse_one_to_three_params =
  TESTPARAMS( one_to_three_syntax, one_to_three_tests );

// =
static const parser_testcase_t no_args_one_obj_tests[] = {
  ERR("Hi there!\r\n.", "Cannot accept more than 0 argument(s)"),
  ERR("", "Empty body"),
  OK("\r\n", "{ args=[], obj=\"\\n\" }"),
  OK("\r\nHello world\r\n", "{ args=[], obj=\"Hello world\\n\\n\" }"),
  OK("\r\nHello\r\nworld\r\n", "{ args=[], obj=\"Hello\\nworld\\n\\n\" }"),
  OK("\r\nHello\r\n..\r\nworld\r\n",
     "{ args=[], obj=\"Hello\\n.\\nworld\\n\\n\" }"),
};
static const control_cmd_syntax_t no_args_one_obj_syntax = {
   .min_args=0, .max_args=0,
   .want_cmddata=true,
};
static const parse_test_params_t parse_no_args_one_obj_params =
  TESTPARAMS( no_args_one_obj_syntax, no_args_one_obj_tests );

static const parser_testcase_t no_args_kwargs_tests[] = {
  OK("", "{ args=[] }"),
  OK(" ", "{ args=[] }"),
  OK("hello there=world", "{ args=[], { hello=\"\", there=\"world\" } }"),
  OK("hello there=world today",
     "{ args=[], { hello=\"\", there=\"world\", today=\"\" } }"),
  ERR("=Foo", "Cannot parse keyword argument(s)"),
};
static const control_cmd_syntax_t no_args_kwargs_syntax = {
   .min_args=0, .max_args=0,
   .accept_keywords=true,
   .kvline_flags=KV_OMIT_VALS
};
static const parse_test_params_t parse_no_args_kwargs_params =
  TESTPARAMS( no_args_kwargs_syntax, no_args_kwargs_tests );

static const char *one_arg_kwargs_allow_keywords[] = {
  "Hello", "world", NULL
};
static const parser_testcase_t one_arg_kwargs_tests[] = {
  ERR("", "Need at least 1 argument(s)"),
  OK("Hi", "{ args=[ \"Hi\" ] }"),
  ERR("hello there=world", "Unrecognized keyword argument \"there\""),
  OK("Hi HELLO=foo", "{ args=[ \"Hi\" ], { HELLO=\"foo\" } }"),
  OK("Hi world=\"bar baz\" hello  ",
     "{ args=[ \"Hi\" ], { world=\"bar baz\", hello=\"\" } }"),
};
static const control_cmd_syntax_t one_arg_kwargs_syntax = {
   .min_args=1, .max_args=1,
   .accept_keywords=true,
   .allowed_keywords=one_arg_kwargs_allow_keywords,
   .kvline_flags=KV_OMIT_VALS|KV_QUOTED,
};
static const parse_test_params_t parse_one_arg_kwargs_params =
  TESTPARAMS( one_arg_kwargs_syntax, one_arg_kwargs_tests );

static char *reply_str = NULL;
/* Mock for control_write_reply that copies the string for inspection
 * by tests */
static void
mock_control_write_reply(control_connection_t *conn, int code, int c,
                                const char *s)
{
  (void)conn;
  (void)code;
  (void)c;
  tor_free(reply_str);
  reply_str = tor_strdup(s);
}

static void
test_add_onion_helper_keyarg_v3(void *arg)
{
  int ret, hs_version;
  add_onion_secret_key_t pk;
  char *key_new_blob = NULL;
  const char *key_new_alg = NULL;

  (void) arg;
  MOCK(control_write_reply, mock_control_write_reply);

  memset(&pk, 0, sizeof(pk));

  /* Test explicit ED25519-V3 key generation. */
  tor_free(reply_str);
  ret = add_onion_helper_keyarg("NEW:ED25519-V3", 0, &key_new_alg,
                                &key_new_blob, &pk, &hs_version,
                                NULL);
  tt_int_op(ret, OP_EQ, 0);
  tt_int_op(hs_version, OP_EQ, HS_VERSION_THREE);
  tt_assert(pk.v3);
  tt_str_op(key_new_alg, OP_EQ, "ED25519-V3");
  tt_assert(key_new_blob);
  tt_ptr_op(reply_str, OP_EQ, NULL);
  tor_free(pk.v3); pk.v3 = NULL;
  tor_free(key_new_blob);

  /* Test "BEST" key generation (Assumes BEST = ED25519-V3). */
  tor_free(pk.v3); pk.v3 = NULL;
  tor_free(key_new_blob);
  ret = add_onion_helper_keyarg("NEW:BEST", 0, &key_new_alg, &key_new_blob,
                                &pk, &hs_version, NULL);
  tt_int_op(ret, OP_EQ, 0);
  tt_int_op(hs_version, OP_EQ, HS_VERSION_THREE);
  tt_assert(pk.v3);
  tt_str_op(key_new_alg, OP_EQ, "ED25519-V3");
  tt_assert(key_new_blob);
  tt_ptr_op(reply_str, OP_EQ, NULL);

  /* Test discarding the private key. */
  tor_free(reply_str);
  tor_free(pk.v3); pk.v3 = NULL;
  tor_free(key_new_blob);
  ret = add_onion_helper_keyarg("NEW:ED25519-V3", 1, &key_new_alg,
                                &key_new_blob, &pk, &hs_version,
                                NULL);
  tt_int_op(ret, OP_EQ, 0);
  tt_int_op(hs_version, OP_EQ, HS_VERSION_THREE);
  tt_assert(pk.v3);
  tt_ptr_op(key_new_alg, OP_EQ, NULL);
  tt_ptr_op(key_new_blob, OP_EQ, NULL);
  tt_ptr_op(reply_str, OP_EQ, NULL);
  tor_free(pk.v3); pk.v3 = NULL;
  tor_free(key_new_blob);

  /* Test passing a key blob. */
  {
    /* The base64 key and hex key are the same. Hex key is 64 bytes long. The
     * sk has been generated randomly using python3. */
    const char *base64_sk =
      "a9bT19PqGC9Y+BmOo1IQvCGjjwxMiaaxEXZ+FKMxpEQW"
      "6AmSV5roThUGMRCaqQSCnR2jI1vL2QxHORzI4RxMmw==";
    const char *hex_sk =
      "\x6b\xd6\xd3\xd7\xd3\xea\x18\x2f\x58\xf8\x19\x8e\xa3\x52\x10\xbc"
      "\x21\xa3\x8f\x0c\x4c\x89\xa6\xb1\x11\x76\x7e\x14\xa3\x31\xa4\x44"
      "\x16\xe8\x09\x92\x57\x9a\xe8\x4e\x15\x06\x31\x10\x9a\xa9\x04\x82"
      "\x9d\x1d\xa3\x23\x5b\xcb\xd9\x0c\x47\x39\x1c\xc8\xe1\x1c\x4c\x9b";
    char *key_blob = NULL;

    tor_asprintf(&key_blob, "ED25519-V3:%s", base64_sk);
    tt_assert(key_blob);
    tor_free(reply_str);
    ret = add_onion_helper_keyarg(key_blob, 1, &key_new_alg,
                                  &key_new_blob, &pk, &hs_version,
                                  NULL);
    tor_free(key_blob);
    tt_int_op(ret, OP_EQ, 0);
    tt_int_op(hs_version, OP_EQ, HS_VERSION_THREE);
    tt_assert(pk.v3);
    tt_mem_op(pk.v3, OP_EQ, hex_sk, 64);
    tt_ptr_op(key_new_alg, OP_EQ, NULL);
    tt_ptr_op(key_new_blob, OP_EQ, NULL);
    tt_ptr_op(reply_str, OP_EQ, NULL);
    tor_free(pk.v3); pk.v3 = NULL;
    tor_free(key_new_blob);
  }

 done:
  tor_free(pk.v3);
  tor_free(key_new_blob);
  tor_free(reply_str);
  UNMOCK(control_write_reply);
}

static void
test_add_onion_helper_keyarg_v2(void *arg)
{
  int ret, hs_version;
  add_onion_secret_key_t pk;
  crypto_pk_t *pk1 = NULL;
  const char *key_new_alg = NULL;
  char *key_new_blob = NULL;
  char *encoded = NULL;
  char *arg_str = NULL;

  (void) arg;
  MOCK(control_write_reply, mock_control_write_reply);

  memset(&pk, 0, sizeof(pk));

  /* Test explicit RSA1024 key generation. */
  tor_free(reply_str);
  ret = add_onion_helper_keyarg("NEW:RSA1024", 0, &key_new_alg, &key_new_blob,
                                &pk, &hs_version, NULL);
  tt_int_op(ret, OP_EQ, 0);
  tt_int_op(hs_version, OP_EQ, HS_VERSION_TWO);
  tt_assert(pk.v2);
  tt_str_op(key_new_alg, OP_EQ, "RSA1024");
  tt_assert(key_new_blob);
  tt_ptr_op(reply_str, OP_EQ, NULL);

  /* Test discarding the private key. */
  crypto_pk_free(pk.v2); pk.v2 = NULL;
  tor_free(key_new_blob);
  ret = add_onion_helper_keyarg("NEW:RSA1024", 1, &key_new_alg, &key_new_blob,
                               &pk, &hs_version, NULL);
  tt_int_op(ret, OP_EQ, 0);
  tt_int_op(hs_version, OP_EQ, HS_VERSION_TWO);
  tt_assert(pk.v2);
  tt_ptr_op(key_new_alg, OP_EQ, NULL);
  tt_ptr_op(key_new_blob, OP_EQ, NULL);
  tt_ptr_op(reply_str, OP_EQ, NULL);

  /* Test generating a invalid key type. */
  crypto_pk_free(pk.v2); pk.v2 = NULL;
  ret = add_onion_helper_keyarg("NEW:RSA512", 0, &key_new_alg, &key_new_blob,
                               &pk, &hs_version, NULL);
  tt_int_op(ret, OP_EQ, -1);
  tt_int_op(hs_version, OP_EQ, HS_VERSION_TWO);
  tt_assert(!pk.v2);
  tt_ptr_op(key_new_alg, OP_EQ, NULL);
  tt_ptr_op(key_new_blob, OP_EQ, NULL);
  tt_assert(reply_str);

  /* Test loading a RSA1024 key. */
  tor_free(reply_str);
  pk1 = pk_generate(0);
  tt_int_op(0, OP_EQ, crypto_pk_base64_encode_private(pk1, &encoded));
  tor_asprintf(&arg_str, "RSA1024:%s", encoded);
  ret = add_onion_helper_keyarg(arg_str, 0, &key_new_alg, &key_new_blob,
                                &pk, &hs_version, NULL);
  tt_int_op(ret, OP_EQ, 0);
  tt_int_op(hs_version, OP_EQ, HS_VERSION_TWO);
  tt_assert(pk.v2);
  tt_ptr_op(key_new_alg, OP_EQ, NULL);
  tt_ptr_op(key_new_blob, OP_EQ, NULL);
  tt_ptr_op(reply_str, OP_EQ, NULL);
  tt_int_op(crypto_pk_cmp_keys(pk1, pk.v2), OP_EQ, 0);

  /* Test loading a invalid key type. */
  tor_free(arg_str);
  crypto_pk_free(pk1); pk1 = NULL;
  crypto_pk_free(pk.v2); pk.v2 = NULL;
  tor_asprintf(&arg_str, "RSA512:%s", encoded);
  ret = add_onion_helper_keyarg(arg_str, 0, &key_new_alg, &key_new_blob,
                                &pk, &hs_version, NULL);
  tt_int_op(ret, OP_EQ, -1);
  tt_int_op(hs_version, OP_EQ, HS_VERSION_TWO);
  tt_assert(!pk.v2);
  tt_ptr_op(key_new_alg, OP_EQ, NULL);
  tt_ptr_op(key_new_blob, OP_EQ, NULL);
  tt_assert(reply_str);

  /* Test loading a invalid key. */
  tor_free(arg_str);
  crypto_pk_free(pk.v2); pk.v2 = NULL;
  tor_free(reply_str);
  encoded[strlen(encoded)/2] = '\0';
  tor_asprintf(&arg_str, "RSA1024:%s", encoded);
  ret = add_onion_helper_keyarg(arg_str, 0, &key_new_alg, &key_new_blob,
                               &pk, &hs_version, NULL);
  tt_int_op(ret, OP_EQ, -1);
  tt_int_op(hs_version, OP_EQ, HS_VERSION_TWO);
  tt_assert(!pk.v2);
  tt_ptr_op(key_new_alg, OP_EQ, NULL);
  tt_ptr_op(key_new_blob, OP_EQ, NULL);
  tt_assert(reply_str);

 done:
  crypto_pk_free(pk1);
  crypto_pk_free(pk.v2);
  tor_free(key_new_blob);
  tor_free(reply_str);
  tor_free(encoded);
  tor_free(arg_str);
  UNMOCK(control_write_reply);
}

static void
test_getinfo_helper_onion(void *arg)
{
  (void)arg;
  control_connection_t dummy;
  /* Get results out */
  char *answer = NULL;
  const char *errmsg = NULL;
  char *service_id = NULL;
  int rt = 0;

  dummy.ephemeral_onion_services = NULL;

  /* successfully get an empty answer */
  rt = getinfo_helper_onions(&dummy, "onions/current", &answer, &errmsg);
  tt_int_op(rt, OP_EQ, 0);
  tt_str_op(answer, OP_EQ, "");
  tor_free(answer);

  /* successfully get an empty answer */
  rt = getinfo_helper_onions(&dummy, "onions/detached", &answer, &errmsg);
  tt_int_op(rt, OP_EQ, 0);
  tt_str_op(answer, OP_EQ, "");
  tor_free(answer);

  /* get an answer for one onion service */
  service_id = tor_strdup("dummy_onion_id");
  dummy.ephemeral_onion_services = smartlist_new();
  smartlist_add(dummy.ephemeral_onion_services, service_id);
  rt = getinfo_helper_onions(&dummy, "onions/current", &answer, &errmsg);
  tt_int_op(rt, OP_EQ, 0);
  tt_str_op(answer, OP_EQ, "dummy_onion_id");

 done:
  tor_free(answer);
  tor_free(service_id);
  smartlist_free(dummy.ephemeral_onion_services);
}

static void
test_rend_service_parse_port_config(void *arg)
{
  const char *sep = ",";
  rend_service_port_config_t *cfg = NULL;
  char *err_msg = NULL;

  (void)arg;

  /* Test "VIRTPORT" only. */
  cfg = rend_service_parse_port_config("80", sep, &err_msg);
  tt_assert(cfg);
  tt_ptr_op(err_msg, OP_EQ, NULL);

  /* Test "VIRTPORT,TARGET" (Target is port). */
  rend_service_port_config_free(cfg);
  cfg = rend_service_parse_port_config("80,8080", sep, &err_msg);
  tt_assert(cfg);
  tt_ptr_op(err_msg, OP_EQ, NULL);

  /* Test "VIRTPORT,TARGET" (Target is IPv4:port). */
  rend_service_port_config_free(cfg);
  cfg = rend_service_parse_port_config("80,192.0.2.1:8080", sep, &err_msg);
  tt_assert(cfg);
  tt_ptr_op(err_msg, OP_EQ, NULL);

  /* Test "VIRTPORT,TARGET" (Target is IPv6:port). */
  rend_service_port_config_free(cfg);
  cfg = rend_service_parse_port_config("80,[2001:db8::1]:8080", sep, &err_msg);
  tt_assert(cfg);
  tt_ptr_op(err_msg, OP_EQ, NULL);
  rend_service_port_config_free(cfg);
  cfg = NULL;

  /* XXX: Someone should add tests for AF_UNIX targets if supported. */

  /* Test empty config. */
  rend_service_port_config_free(cfg);
  cfg = rend_service_parse_port_config("", sep, &err_msg);
  tt_ptr_op(cfg, OP_EQ, NULL);
  tt_assert(err_msg);

  /* Test invalid port. */
  tor_free(err_msg);
  cfg = rend_service_parse_port_config("90001", sep, &err_msg);
  tt_ptr_op(cfg, OP_EQ, NULL);
  tt_assert(err_msg);
  tor_free(err_msg);

  /* unix port */
  cfg = NULL;

  /* quoted unix port */
  tor_free(err_msg);
  cfg = rend_service_parse_port_config("100 unix:\"/tmp/foo bar\"",
                                       " ", &err_msg);
  tt_assert(cfg);
  tt_ptr_op(err_msg, OP_EQ, NULL);
  rend_service_port_config_free(cfg);
  cfg = NULL;

  /* quoted unix port */
  tor_free(err_msg);
  cfg = rend_service_parse_port_config("100 unix:\"/tmp/foo bar\"",
                                       " ", &err_msg);
  tt_assert(cfg);
  tt_ptr_op(err_msg, OP_EQ, NULL);
  rend_service_port_config_free(cfg);
  cfg = NULL;

  /* quoted unix port, missing end quote */
  cfg = rend_service_parse_port_config("100 unix:\"/tmp/foo bar",
                                       " ", &err_msg);
  tt_ptr_op(cfg, OP_EQ, NULL);
  tt_str_op(err_msg, OP_EQ, "Couldn't process address <unix:\"/tmp/foo bar> "
            "from hidden service configuration");
  tor_free(err_msg);

  /* bogus IP address */
  MOCK(tor_addr_lookup, mock_tor_addr_lookup__fail_on_bad_addrs);
  cfg = rend_service_parse_port_config("100 foo!!.example.com:9000",
                                       " ", &err_msg);
  UNMOCK(tor_addr_lookup);
  tt_ptr_op(cfg, OP_EQ, NULL);
  tt_str_op(err_msg, OP_EQ, "Unparseable address in hidden service port "
            "configuration.");
  tor_free(err_msg);

  /* bogus port port */
  cfg = rend_service_parse_port_config("100 99999",
                                       " ", &err_msg);
  tt_ptr_op(cfg, OP_EQ, NULL);
  tt_str_op(err_msg, OP_EQ, "Unparseable or out-of-range port \"99999\" "
            "in hidden service port configuration.");
  tor_free(err_msg);

  /* Wrong target address and port separation */
  cfg = rend_service_parse_port_config("80,127.0.0.1 1234", sep,
                                       &err_msg);
  tt_ptr_op(cfg, OP_EQ, NULL);
  tt_assert(err_msg);
  tor_free(err_msg);

 done:
  rend_service_port_config_free(cfg);
  tor_free(err_msg);
}

static void
test_add_onion_helper_clientauth(void *arg)
{
  rend_authorized_client_t *client = NULL;
  int created = 0;

  (void)arg;

  MOCK(control_write_reply, mock_control_write_reply);
  /* Test "ClientName" only. */
  tor_free(reply_str);
  client = add_onion_helper_clientauth("alice", &created, NULL);
  tt_assert(client);
  tt_assert(created);
  tt_ptr_op(reply_str, OP_EQ, NULL);
  rend_authorized_client_free(client);

  /* Test "ClientName:Blob" */
  tor_free(reply_str);
  client = add_onion_helper_clientauth("alice:475hGBHPlq7Mc0cRZitK/B",
                                       &created, NULL);
  tt_assert(client);
  tt_assert(!created);
  tt_ptr_op(reply_str, OP_EQ, NULL);
  rend_authorized_client_free(client);

  /* Test invalid client names */
  tor_free(reply_str);
  client = add_onion_helper_clientauth("no*asterisks*allowed", &created,
                                       NULL);
  tt_ptr_op(client, OP_EQ, NULL);
  tt_assert(reply_str);

  /* Test invalid auth cookie */
  tor_free(reply_str);
  client = add_onion_helper_clientauth("alice:12345", &created, NULL);
  tt_ptr_op(client, OP_EQ, NULL);
  tt_assert(reply_str);

  /* Test invalid syntax */
  tor_free(reply_str);
  client = add_onion_helper_clientauth(":475hGBHPlq7Mc0cRZitK/B", &created,
                                       NULL);
  tt_ptr_op(client, OP_EQ, NULL);
  tt_assert(reply_str);

 done:
  rend_authorized_client_free(client);
  tor_free(reply_str);
  UNMOCK(control_write_reply);
}

/* Mocks and data/variables used for GETINFO download status tests */

static const download_status_t dl_status_default =
  { 0, 0, 0, DL_SCHED_CONSENSUS, DL_WANT_ANY_DIRSERVER,
    DL_SCHED_INCREMENT_FAILURE, 0, 0 };
static download_status_t ns_dl_status[N_CONSENSUS_FLAVORS];
static download_status_t ns_dl_status_bootstrap[N_CONSENSUS_FLAVORS];
static download_status_t ns_dl_status_running[N_CONSENSUS_FLAVORS];

/*
 * These should explore all the possible cases of download_status_to_string()
 * in control.c
 */
static const download_status_t dls_sample_1 =
  { 1467163900, 0, 0, DL_SCHED_GENERIC, DL_WANT_ANY_DIRSERVER,
    DL_SCHED_INCREMENT_FAILURE, 0, 0 };
static const char * dls_sample_1_str =
    "next-attempt-at 2016-06-29 01:31:40\n"
    "n-download-failures 0\n"
    "n-download-attempts 0\n"
    "schedule DL_SCHED_GENERIC\n"
    "want-authority DL_WANT_ANY_DIRSERVER\n"
    "increment-on DL_SCHED_INCREMENT_FAILURE\n"
    "backoff DL_SCHED_RANDOM_EXPONENTIAL\n"
    "last-backoff-position 0\n"
    "last-delay-used 0\n";
static const download_status_t dls_sample_2 =
  { 1467164400, 1, 2, DL_SCHED_CONSENSUS, DL_WANT_AUTHORITY,
    DL_SCHED_INCREMENT_FAILURE, 0, 0 };
static const char * dls_sample_2_str =
    "next-attempt-at 2016-06-29 01:40:00\n"
    "n-download-failures 1\n"
    "n-download-attempts 2\n"
    "schedule DL_SCHED_CONSENSUS\n"
    "want-authority DL_WANT_AUTHORITY\n"
    "increment-on DL_SCHED_INCREMENT_FAILURE\n"
    "backoff DL_SCHED_RANDOM_EXPONENTIAL\n"
    "last-backoff-position 0\n"
    "last-delay-used 0\n";
static const download_status_t dls_sample_3 =
  { 1467154400, 12, 25, DL_SCHED_BRIDGE, DL_WANT_ANY_DIRSERVER,
    DL_SCHED_INCREMENT_ATTEMPT, 0, 0 };
static const char * dls_sample_3_str =
    "next-attempt-at 2016-06-28 22:53:20\n"
    "n-download-failures 12\n"
    "n-download-attempts 25\n"
    "schedule DL_SCHED_BRIDGE\n"
    "want-authority DL_WANT_ANY_DIRSERVER\n"
    "increment-on DL_SCHED_INCREMENT_ATTEMPT\n"
    "backoff DL_SCHED_RANDOM_EXPONENTIAL\n"
    "last-backoff-position 0\n"
    "last-delay-used 0\n";
static const download_status_t dls_sample_4 =
  { 1467166600, 3, 0, DL_SCHED_GENERIC, DL_WANT_ANY_DIRSERVER,
    DL_SCHED_INCREMENT_FAILURE, 0, 0 };
static const char * dls_sample_4_str =
    "next-attempt-at 2016-06-29 02:16:40\n"
    "n-download-failures 3\n"
    "n-download-attempts 0\n"
    "schedule DL_SCHED_GENERIC\n"
    "want-authority DL_WANT_ANY_DIRSERVER\n"
    "increment-on DL_SCHED_INCREMENT_FAILURE\n"
    "backoff DL_SCHED_RANDOM_EXPONENTIAL\n"
    "last-backoff-position 0\n"
    "last-delay-used 0\n";
static const download_status_t dls_sample_5 =
  { 1467164600, 3, 7, DL_SCHED_CONSENSUS, DL_WANT_ANY_DIRSERVER,
    DL_SCHED_INCREMENT_FAILURE, 1, 2112, };
static const char * dls_sample_5_str =
    "next-attempt-at 2016-06-29 01:43:20\n"
    "n-download-failures 3\n"
    "n-download-attempts 7\n"
    "schedule DL_SCHED_CONSENSUS\n"
    "want-authority DL_WANT_ANY_DIRSERVER\n"
    "increment-on DL_SCHED_INCREMENT_FAILURE\n"
    "backoff DL_SCHED_RANDOM_EXPONENTIAL\n"
    "last-backoff-position 1\n"
    "last-delay-used 2112\n";
static const download_status_t dls_sample_6 =
  { 1467164200, 4, 9, DL_SCHED_CONSENSUS, DL_WANT_AUTHORITY,
    DL_SCHED_INCREMENT_ATTEMPT, 3, 432 };
static const char * dls_sample_6_str =
    "next-attempt-at 2016-06-29 01:36:40\n"
    "n-download-failures 4\n"
    "n-download-attempts 9\n"
    "schedule DL_SCHED_CONSENSUS\n"
    "want-authority DL_WANT_AUTHORITY\n"
    "increment-on DL_SCHED_INCREMENT_ATTEMPT\n"
    "backoff DL_SCHED_RANDOM_EXPONENTIAL\n"
    "last-backoff-position 3\n"
    "last-delay-used 432\n";

/* Simulated auth certs */
static const char *auth_id_digest_1_str =
    "63CDD326DFEF0CA020BDD3FEB45A3286FE13A061";
static download_status_t auth_def_cert_download_status_1;
static const char *auth_id_digest_2_str =
    "2C209FCDD8D48DC049777B8DC2C0F94A0408BE99";
static download_status_t auth_def_cert_download_status_2;
/* Expected form of digest list returned for GETINFO downloads/cert/fps */
static const char *auth_id_digest_expected_list =
    "63CDD326DFEF0CA020BDD3FEB45A3286FE13A061\n"
    "2C209FCDD8D48DC049777B8DC2C0F94A0408BE99\n";

/* Signing keys for simulated auth 1 */
static const char *auth_1_sk_1_str =
    "AA69566029B1F023BA09451B8F1B10952384EB58";
static download_status_t auth_1_sk_1_dls;
static const char *auth_1_sk_2_str =
    "710865C7F06B73C5292695A8C34F1C94F769FF72";
static download_status_t auth_1_sk_2_dls;
/*
 * Expected form of sk digest list for
 * GETINFO downloads/cert/<auth_id_digest_1_str>/sks
 */
static const char *auth_1_sk_digest_expected_list =
    "AA69566029B1F023BA09451B8F1B10952384EB58\n"
    "710865C7F06B73C5292695A8C34F1C94F769FF72\n";

/* Signing keys for simulated auth 2 */
static const char *auth_2_sk_1_str =
    "4299047E00D070AD6703FE00BE7AA756DB061E62";
static download_status_t auth_2_sk_1_dls;
static const char *auth_2_sk_2_str =
    "9451B8F1B10952384EB58B5F230C0BB701626C9B";
static download_status_t auth_2_sk_2_dls;
/*
 * Expected form of sk digest list for
 * GETINFO downloads/cert/<auth_id_digest_2_str>/sks
 */
static const char *auth_2_sk_digest_expected_list =
    "4299047E00D070AD6703FE00BE7AA756DB061E62\n"
    "9451B8F1B10952384EB58B5F230C0BB701626C9B\n";

/* Simulated router descriptor digests or bridge identity digests */
static const char *descbr_digest_1_str =
    "616408544C7345822696074A1A3DFA16AB381CBD";
static download_status_t descbr_digest_1_dl;
static const char *descbr_digest_2_str =
    "06E8067246967265DBCB6641631B530EFEC12DC3";
static download_status_t descbr_digest_2_dl;
/* Expected form of digest list returned for GETINFO downloads/desc/descs */
static const char *descbr_expected_list =
    "616408544C7345822696074A1A3DFA16AB381CBD\n"
    "06E8067246967265DBCB6641631B530EFEC12DC3\n";
/*
 * Flag to make all descbr queries fail, to simulate not being
 * configured such that such queries make sense.
 */
static int disable_descbr = 0;

static void
reset_mocked_dl_statuses(void)
{
  int i;

  for (i = 0; i < N_CONSENSUS_FLAVORS; ++i) {
    memcpy(&(ns_dl_status[i]), &dl_status_default,
           sizeof(download_status_t));
    memcpy(&(ns_dl_status_bootstrap[i]), &dl_status_default,
           sizeof(download_status_t));
    memcpy(&(ns_dl_status_running[i]), &dl_status_default,
           sizeof(download_status_t));
  }

  memcpy(&auth_def_cert_download_status_1, &dl_status_default,
         sizeof(download_status_t));
  memcpy(&auth_def_cert_download_status_2, &dl_status_default,
         sizeof(download_status_t));
  memcpy(&auth_1_sk_1_dls, &dl_status_default,
         sizeof(download_status_t));
  memcpy(&auth_1_sk_2_dls, &dl_status_default,
         sizeof(download_status_t));
  memcpy(&auth_2_sk_1_dls, &dl_status_default,
         sizeof(download_status_t));
  memcpy(&auth_2_sk_2_dls, &dl_status_default,
         sizeof(download_status_t));

  memcpy(&descbr_digest_1_dl, &dl_status_default,
         sizeof(download_status_t));
  memcpy(&descbr_digest_2_dl, &dl_status_default,
         sizeof(download_status_t));
}

static download_status_t *
ns_dl_status_mock(consensus_flavor_t flavor)
{
  return &(ns_dl_status[flavor]);
}

static download_status_t *
ns_dl_status_bootstrap_mock(consensus_flavor_t flavor)
{
  return &(ns_dl_status_bootstrap[flavor]);
}

static download_status_t *
ns_dl_status_running_mock(consensus_flavor_t flavor)
{
  return &(ns_dl_status_running[flavor]);
}

static void
setup_ns_mocks(void)
{
  MOCK(networkstatus_get_dl_status_by_flavor, ns_dl_status_mock);
  MOCK(networkstatus_get_dl_status_by_flavor_bootstrap,
       ns_dl_status_bootstrap_mock);
  MOCK(networkstatus_get_dl_status_by_flavor_running,
       ns_dl_status_running_mock);
  reset_mocked_dl_statuses();
}

static void
clear_ns_mocks(void)
{
  UNMOCK(networkstatus_get_dl_status_by_flavor);
  UNMOCK(networkstatus_get_dl_status_by_flavor_bootstrap);
  UNMOCK(networkstatus_get_dl_status_by_flavor_running);
}

static smartlist_t *
cert_dl_status_auth_ids_mock(void)
{
  char digest[DIGEST_LEN], *tmp;
  int len;
  smartlist_t *list = NULL;

  /* Just pretend we have only the two hard-coded digests listed above */
  list = smartlist_new();
  len = base16_decode(digest, DIGEST_LEN,
                      auth_id_digest_1_str, strlen(auth_id_digest_1_str));
  tt_int_op(len, OP_EQ, DIGEST_LEN);
  tmp = tor_malloc(DIGEST_LEN);
  memcpy(tmp, digest, DIGEST_LEN);
  smartlist_add(list, tmp);
  len = base16_decode(digest, DIGEST_LEN,
                      auth_id_digest_2_str, strlen(auth_id_digest_2_str));
  tt_int_op(len, OP_EQ, DIGEST_LEN);
  tmp = tor_malloc(DIGEST_LEN);
  memcpy(tmp, digest, DIGEST_LEN);
  smartlist_add(list, tmp);

 done:
  return list;
}

static download_status_t *
cert_dl_status_def_for_auth_mock(const char *digest)
{
  download_status_t *dl = NULL;
  char digest_str[HEX_DIGEST_LEN+1];

  tt_ptr_op(digest, OP_NE, NULL);
  base16_encode(digest_str, HEX_DIGEST_LEN + 1,
                digest, DIGEST_LEN);
  digest_str[HEX_DIGEST_LEN] = '\0';

  if (strcmp(digest_str, auth_id_digest_1_str) == 0) {
    dl = &auth_def_cert_download_status_1;
  } else if (strcmp(digest_str, auth_id_digest_2_str) == 0) {
    dl = &auth_def_cert_download_status_2;
  }

 done:
  return dl;
}

static smartlist_t *
cert_dl_status_sks_for_auth_id_mock(const char *digest)
{
  smartlist_t *list = NULL;
  char sk[DIGEST_LEN];
  char digest_str[HEX_DIGEST_LEN+1];
  char *tmp;
  int len;

  tt_ptr_op(digest, OP_NE, NULL);
  base16_encode(digest_str, HEX_DIGEST_LEN + 1,
                digest, DIGEST_LEN);
  digest_str[HEX_DIGEST_LEN] = '\0';

  /*
   * Build a list of two hard-coded digests, depending on what we
   * were just passed.
   */
  if (strcmp(digest_str, auth_id_digest_1_str) == 0) {
    list = smartlist_new();
    len = base16_decode(sk, DIGEST_LEN,
                        auth_1_sk_1_str, strlen(auth_1_sk_1_str));
    tt_int_op(len, OP_EQ, DIGEST_LEN);
    tmp = tor_malloc(DIGEST_LEN);
    memcpy(tmp, sk, DIGEST_LEN);
    smartlist_add(list, tmp);
    len = base16_decode(sk, DIGEST_LEN,
                        auth_1_sk_2_str, strlen(auth_1_sk_2_str));
    tt_int_op(len, OP_EQ, DIGEST_LEN);
    tmp = tor_malloc(DIGEST_LEN);
    memcpy(tmp, sk, DIGEST_LEN);
    smartlist_add(list, tmp);
  } else if (strcmp(digest_str, auth_id_digest_2_str) == 0) {
    list = smartlist_new();
    len = base16_decode(sk, DIGEST_LEN,
                        auth_2_sk_1_str, strlen(auth_2_sk_1_str));
    tt_int_op(len, OP_EQ, DIGEST_LEN);
    tmp = tor_malloc(DIGEST_LEN);
    memcpy(tmp, sk, DIGEST_LEN);
    smartlist_add(list, tmp);
    len = base16_decode(sk, DIGEST_LEN,
                        auth_2_sk_2_str, strlen(auth_2_sk_2_str));
    tt_int_op(len, OP_EQ, DIGEST_LEN);
    tmp = tor_malloc(DIGEST_LEN);
    memcpy(tmp, sk, DIGEST_LEN);
    smartlist_add(list, tmp);
  }

 done:
  return list;
}

static download_status_t *
cert_dl_status_fp_sk_mock(const char *fp_digest, const char *sk_digest)
{
  download_status_t *dl = NULL;
  char fp_digest_str[HEX_DIGEST_LEN+1], sk_digest_str[HEX_DIGEST_LEN+1];

  /*
   * Unpack the digests so we can compare them and figure out which
   * dl status we want.
   */

  tt_ptr_op(fp_digest, OP_NE, NULL);
  base16_encode(fp_digest_str, HEX_DIGEST_LEN + 1,
                fp_digest, DIGEST_LEN);
  fp_digest_str[HEX_DIGEST_LEN] = '\0';
  tt_ptr_op(sk_digest, OP_NE, NULL);
  base16_encode(sk_digest_str, HEX_DIGEST_LEN + 1,
                sk_digest, DIGEST_LEN);
  sk_digest_str[HEX_DIGEST_LEN] = '\0';

  if (strcmp(fp_digest_str, auth_id_digest_1_str) == 0) {
    if (strcmp(sk_digest_str, auth_1_sk_1_str) == 0) {
      dl = &auth_1_sk_1_dls;
    } else if (strcmp(sk_digest_str, auth_1_sk_2_str) == 0) {
      dl = &auth_1_sk_2_dls;
    }
  } else if (strcmp(fp_digest_str, auth_id_digest_2_str) == 0) {
    if (strcmp(sk_digest_str, auth_2_sk_1_str) == 0) {
      dl = &auth_2_sk_1_dls;
    } else if (strcmp(sk_digest_str, auth_2_sk_2_str) == 0) {
      dl = &auth_2_sk_2_dls;
    }
  }

 done:
  return dl;
}

static void
setup_cert_mocks(void)
{
  MOCK(list_authority_ids_with_downloads, cert_dl_status_auth_ids_mock);
  MOCK(id_only_download_status_for_authority_id,
       cert_dl_status_def_for_auth_mock);
  MOCK(list_sk_digests_for_authority_id,
       cert_dl_status_sks_for_auth_id_mock);
  MOCK(download_status_for_authority_id_and_sk,
       cert_dl_status_fp_sk_mock);
  reset_mocked_dl_statuses();
}

static void
clear_cert_mocks(void)
{
  UNMOCK(list_authority_ids_with_downloads);
  UNMOCK(id_only_download_status_for_authority_id);
  UNMOCK(list_sk_digests_for_authority_id);
  UNMOCK(download_status_for_authority_id_and_sk);
}

static smartlist_t *
descbr_get_digests_mock(void)
{
  char digest[DIGEST_LEN], *tmp;
  int len;
  smartlist_t *list = NULL;

  if (!disable_descbr) {
    /* Just pretend we have only the two hard-coded digests listed above */
    list = smartlist_new();
    len = base16_decode(digest, DIGEST_LEN,
                        descbr_digest_1_str, strlen(descbr_digest_1_str));
    tt_int_op(len, OP_EQ, DIGEST_LEN);
    tmp = tor_malloc(DIGEST_LEN);
    memcpy(tmp, digest, DIGEST_LEN);
    smartlist_add(list, tmp);
    len = base16_decode(digest, DIGEST_LEN,
                        descbr_digest_2_str, strlen(descbr_digest_2_str));
    tt_int_op(len, OP_EQ, DIGEST_LEN);
    tmp = tor_malloc(DIGEST_LEN);
    memcpy(tmp, digest, DIGEST_LEN);
    smartlist_add(list, tmp);
  }

 done:
  return list;
}

static download_status_t *
descbr_get_dl_by_digest_mock(const char *digest)
{
  download_status_t *dl = NULL;
  char digest_str[HEX_DIGEST_LEN+1];

  if (!disable_descbr) {
    tt_ptr_op(digest, OP_NE, NULL);
    base16_encode(digest_str, HEX_DIGEST_LEN + 1,
                  digest, DIGEST_LEN);
    digest_str[HEX_DIGEST_LEN] = '\0';

    if (strcmp(digest_str, descbr_digest_1_str) == 0) {
      dl = &descbr_digest_1_dl;
    } else if (strcmp(digest_str, descbr_digest_2_str) == 0) {
      dl = &descbr_digest_2_dl;
    }
  }

 done:
  return dl;
}

static void
setup_desc_mocks(void)
{
  MOCK(router_get_descriptor_digests,
       descbr_get_digests_mock);
  MOCK(router_get_dl_status_by_descriptor_digest,
       descbr_get_dl_by_digest_mock);
  reset_mocked_dl_statuses();
}

static void
clear_desc_mocks(void)
{
  UNMOCK(router_get_descriptor_digests);
  UNMOCK(router_get_dl_status_by_descriptor_digest);
}

static void
setup_bridge_mocks(void)
{
  disable_descbr = 0;

  MOCK(list_bridge_identities,
       descbr_get_digests_mock);
  MOCK(get_bridge_dl_status_by_id,
       descbr_get_dl_by_digest_mock);
  reset_mocked_dl_statuses();
}

static void
clear_bridge_mocks(void)
{
  UNMOCK(list_bridge_identities);
  UNMOCK(get_bridge_dl_status_by_id);

  disable_descbr = 0;
}

static void
test_download_status_consensus(void *arg)
{
  /* We just need one of these to pass, it doesn't matter what's in it */
  control_connection_t dummy;
  /* Get results out */
  char *answer = NULL;
  const char *errmsg = NULL;

  (void)arg;

  /* Check that the unknown prefix case works; no mocks needed yet */
  getinfo_helper_downloads(&dummy, "downloads/foo", &answer, &errmsg);
  tt_ptr_op(answer, OP_EQ, NULL);
  tt_str_op(errmsg, OP_EQ, "Unknown download status query");

  setup_ns_mocks();

  /*
   * Check returning serialized dlstatuses, and implicitly also test
   * download_status_to_string().
   */

  /* Case 1 default/FLAV_NS*/
  memcpy(&(ns_dl_status[FLAV_NS]), &dls_sample_1,
         sizeof(download_status_t));
  getinfo_helper_downloads(&dummy, "downloads/networkstatus/ns",
                           &answer, &errmsg);
  tt_ptr_op(answer, OP_NE, NULL);
  tt_ptr_op(errmsg, OP_EQ, NULL);
  tt_str_op(answer, OP_EQ, dls_sample_1_str);
  tor_free(answer);
  errmsg = NULL;

  /* Case 2 default/FLAV_MICRODESC */
  memcpy(&(ns_dl_status[FLAV_MICRODESC]), &dls_sample_2,
         sizeof(download_status_t));
  getinfo_helper_downloads(&dummy, "downloads/networkstatus/microdesc",
                           &answer, &errmsg);
  tt_ptr_op(answer, OP_NE, NULL);
  tt_ptr_op(errmsg, OP_EQ, NULL);
  tt_str_op(answer, OP_EQ, dls_sample_2_str);
  tor_free(answer);
  errmsg = NULL;

  /* Case 3 bootstrap/FLAV_NS */
  memcpy(&(ns_dl_status_bootstrap[FLAV_NS]), &dls_sample_3,
         sizeof(download_status_t));
  getinfo_helper_downloads(&dummy, "downloads/networkstatus/ns/bootstrap",
                           &answer, &errmsg);
  tt_ptr_op(answer, OP_NE, NULL);
  tt_ptr_op(errmsg, OP_EQ, NULL);
  tt_str_op(answer, OP_EQ, dls_sample_3_str);
  tor_free(answer);
  errmsg = NULL;

  /* Case 4 bootstrap/FLAV_MICRODESC */
  memcpy(&(ns_dl_status_bootstrap[FLAV_MICRODESC]), &dls_sample_4,
         sizeof(download_status_t));
  getinfo_helper_downloads(&dummy,
                           "downloads/networkstatus/microdesc/bootstrap",
                           &answer, &errmsg);
  tt_ptr_op(answer, OP_NE, NULL);
  tt_ptr_op(errmsg, OP_EQ, NULL);
  tt_str_op(answer, OP_EQ, dls_sample_4_str);
  tor_free(answer);
  errmsg = NULL;

  /* Case 5 running/FLAV_NS */
  memcpy(&(ns_dl_status_running[FLAV_NS]), &dls_sample_5,
         sizeof(download_status_t));
  getinfo_helper_downloads(&dummy,
                           "downloads/networkstatus/ns/running",
                           &answer, &errmsg);
  tt_ptr_op(answer, OP_NE, NULL);
  tt_ptr_op(errmsg, OP_EQ, NULL);
  tt_str_op(answer, OP_EQ, dls_sample_5_str);
  tor_free(answer);
  errmsg = NULL;

  /* Case 6 running/FLAV_MICRODESC */
  memcpy(&(ns_dl_status_running[FLAV_MICRODESC]), &dls_sample_6,
         sizeof(download_status_t));
  getinfo_helper_downloads(&dummy,
                           "downloads/networkstatus/microdesc/running",
                           &answer, &errmsg);
  tt_ptr_op(answer, OP_NE, NULL);
  tt_ptr_op(errmsg, OP_EQ, NULL);
  tt_str_op(answer, OP_EQ, dls_sample_6_str);
  tor_free(answer);
  errmsg = NULL;

  /* Now check the error case */
  getinfo_helper_downloads(&dummy, "downloads/networkstatus/foo",
                           &answer, &errmsg);
  tt_ptr_op(answer, OP_EQ, NULL);
  tt_ptr_op(errmsg, OP_NE, NULL);
  tt_str_op(errmsg, OP_EQ, "Unknown flavor");
  errmsg = NULL;

 done:
  clear_ns_mocks();
  tor_free(answer);

  return;
}

static void
test_download_status_cert(void *arg)
{
  /* We just need one of these to pass, it doesn't matter what's in it */
  control_connection_t dummy;
  /* Get results out */
  char *question = NULL;
  char *answer = NULL;
  const char *errmsg = NULL;

  (void)arg;

  setup_cert_mocks();

  /*
   * Check returning serialized dlstatuses and digest lists, and implicitly
   * also test download_status_to_string() and digest_list_to_string().
   */

  /* Case 1 - list of authority identity fingerprints */
  getinfo_helper_downloads(&dummy,
                           "downloads/cert/fps",
                           &answer, &errmsg);
  tt_ptr_op(answer, OP_NE, NULL);
  tt_ptr_op(errmsg, OP_EQ, NULL);
  tt_str_op(answer, OP_EQ, auth_id_digest_expected_list);
  tor_free(answer);
  errmsg = NULL;

  /* Case 2 - download status for default cert for 1st auth id */
  memcpy(&auth_def_cert_download_status_1, &dls_sample_1,
         sizeof(download_status_t));
  tor_asprintf(&question, "downloads/cert/fp/%s", auth_id_digest_1_str);
  tt_ptr_op(question, OP_NE, NULL);
  getinfo_helper_downloads(&dummy, question, &answer, &errmsg);
  tt_ptr_op(answer, OP_NE, NULL);
  tt_ptr_op(errmsg, OP_EQ, NULL);
  tt_str_op(answer, OP_EQ, dls_sample_1_str);
  tor_free(question);
  tor_free(answer);
  errmsg = NULL;

  /* Case 3 - download status for default cert for 2nd auth id */
  memcpy(&auth_def_cert_download_status_2, &dls_sample_2,
         sizeof(download_status_t));
  tor_asprintf(&question, "downloads/cert/fp/%s", auth_id_digest_2_str);
  tt_ptr_op(question, OP_NE, NULL);
  getinfo_helper_downloads(&dummy, question, &answer, &errmsg);
  tt_ptr_op(answer, OP_NE, NULL);
  tt_ptr_op(errmsg, OP_EQ, NULL);
  tt_str_op(answer, OP_EQ, dls_sample_2_str);
  tor_free(question);
  tor_free(answer);
  errmsg = NULL;

  /* Case 4 - list of signing key digests for 1st auth id */
  tor_asprintf(&question, "downloads/cert/fp/%s/sks", auth_id_digest_1_str);
  tt_ptr_op(question, OP_NE, NULL);
  getinfo_helper_downloads(&dummy, question, &answer, &errmsg);
  tt_ptr_op(answer, OP_NE, NULL);
  tt_ptr_op(errmsg, OP_EQ, NULL);
  tt_str_op(answer, OP_EQ, auth_1_sk_digest_expected_list);
  tor_free(question);
  tor_free(answer);
  errmsg = NULL;

  /* Case 5 - list of signing key digests for 2nd auth id */
  tor_asprintf(&question, "downloads/cert/fp/%s/sks", auth_id_digest_2_str);
  tt_ptr_op(question, OP_NE, NULL);
  getinfo_helper_downloads(&dummy, question, &answer, &errmsg);
  tt_ptr_op(answer, OP_NE, NULL);
  tt_ptr_op(errmsg, OP_EQ, NULL);
  tt_str_op(answer, OP_EQ, auth_2_sk_digest_expected_list);
  tor_free(question);
  tor_free(answer);
  errmsg = NULL;

  /* Case 6 - download status for 1st auth id, 1st sk */
  memcpy(&auth_1_sk_1_dls, &dls_sample_3,
         sizeof(download_status_t));
  tor_asprintf(&question, "downloads/cert/fp/%s/%s",
               auth_id_digest_1_str, auth_1_sk_1_str);
  tt_ptr_op(question, OP_NE, NULL);
  getinfo_helper_downloads(&dummy, question, &answer, &errmsg);
  tt_ptr_op(answer, OP_NE, NULL);
  tt_ptr_op(errmsg, OP_EQ, NULL);
  tt_str_op(answer, OP_EQ, dls_sample_3_str);
  tor_free(question);
  tor_free(answer);
  errmsg = NULL;

  /* Case 7 - download status for 1st auth id, 2nd sk */
  memcpy(&auth_1_sk_2_dls, &dls_sample_4,
         sizeof(download_status_t));
  tor_asprintf(&question, "downloads/cert/fp/%s/%s",
               auth_id_digest_1_str, auth_1_sk_2_str);
  tt_ptr_op(question, OP_NE, NULL);
  getinfo_helper_downloads(&dummy, question, &answer, &errmsg);
  tt_ptr_op(answer, OP_NE, NULL);
  tt_ptr_op(errmsg, OP_EQ, NULL);
  tt_str_op(answer, OP_EQ, dls_sample_4_str);
  tor_free(question);
  tor_free(answer);
  errmsg = NULL;

  /* Case 8 - download status for 2nd auth id, 1st sk */
  memcpy(&auth_2_sk_1_dls, &dls_sample_5,
         sizeof(download_status_t));
  tor_asprintf(&question, "downloads/cert/fp/%s/%s",
               auth_id_digest_2_str, auth_2_sk_1_str);
  tt_ptr_op(question, OP_NE, NULL);
  getinfo_helper_downloads(&dummy, question, &answer, &errmsg);
  tt_ptr_op(answer, OP_NE, NULL);
  tt_ptr_op(errmsg, OP_EQ, NULL);
  tt_str_op(answer, OP_EQ, dls_sample_5_str);
  tor_free(question);
  tor_free(answer);
  errmsg = NULL;

  /* Case 9 - download status for 2nd auth id, 2nd sk */
  memcpy(&auth_2_sk_2_dls, &dls_sample_6,
         sizeof(download_status_t));
  tor_asprintf(&question, "downloads/cert/fp/%s/%s",
               auth_id_digest_2_str, auth_2_sk_2_str);
  tt_ptr_op(question, OP_NE, NULL);
  getinfo_helper_downloads(&dummy, question, &answer, &errmsg);
  tt_ptr_op(answer, OP_NE, NULL);
  tt_ptr_op(errmsg, OP_EQ, NULL);
  tt_str_op(answer, OP_EQ, dls_sample_6_str);
  tor_free(question);
  tor_free(answer);
  errmsg = NULL;

  /* Now check the error cases */

  /* Case 1 - query is garbage after downloads/cert/ part */
  getinfo_helper_downloads(&dummy, "downloads/cert/blahdeblah",
                           &answer, &errmsg);
  tt_ptr_op(answer, OP_EQ, NULL);
  tt_ptr_op(errmsg, OP_NE, NULL);
  tt_str_op(errmsg, OP_EQ, "Unknown certificate download status query");
  errmsg = NULL;

  /*
   * Case 2 - looks like downloads/cert/fp/<fp>, but <fp> isn't even
   * the right length for a digest.
   */
  getinfo_helper_downloads(&dummy, "downloads/cert/fp/2B1D36D32B2942406",
                           &answer, &errmsg);
  tt_ptr_op(answer, OP_EQ, NULL);
  tt_ptr_op(errmsg, OP_NE, NULL);
  tt_str_op(errmsg, OP_EQ, "That didn't look like a digest");
  errmsg = NULL;

  /*
   * Case 3 - looks like downloads/cert/fp/<fp>, and <fp> is digest-sized,
   * but not parseable as one.
   */
  getinfo_helper_downloads(&dummy,
      "downloads/cert/fp/82F52AF55D250115FE44D3GC81D49643241D56A1",
      &answer, &errmsg);
  tt_ptr_op(answer, OP_EQ, NULL);
  tt_ptr_op(errmsg, OP_NE, NULL);
  tt_str_op(errmsg, OP_EQ, "That didn't look like a digest");
  errmsg = NULL;

  /*
   * Case 4 - downloads/cert/fp/<fp>, and <fp> is not a known authority
   * identity digest
   */
  getinfo_helper_downloads(&dummy,
      "downloads/cert/fp/AC4F23B5745BDD2A77997B85B1FD85D05C2E0F61",
      &answer, &errmsg);
  tt_ptr_op(answer, OP_EQ, NULL);
  tt_ptr_op(errmsg, OP_NE, NULL);
  tt_str_op(errmsg, OP_EQ,
      "Failed to get download status for this authority identity digest");
  errmsg = NULL;

  /*
   * Case 5 - looks like downloads/cert/fp/<fp>/<anything>, but <fp> doesn't
   * parse as a sensible digest.
   */
  getinfo_helper_downloads(&dummy,
      "downloads/cert/fp/82F52AF55D250115FE44D3GC81D49643241D56A1/blah",
      &answer, &errmsg);
  tt_ptr_op(answer, OP_EQ, NULL);
  tt_ptr_op(errmsg, OP_NE, NULL);
  tt_str_op(errmsg, OP_EQ, "That didn't look like an identity digest");
  errmsg = NULL;

  /*
   * Case 6 - looks like downloads/cert/fp/<fp>/<anything>, but <fp> doesn't
   * parse as a sensible digest.
   */
  getinfo_helper_downloads(&dummy,
      "downloads/cert/fp/82F52AF55D25/blah",
      &answer, &errmsg);
  tt_ptr_op(answer, OP_EQ, NULL);
  tt_ptr_op(errmsg, OP_NE, NULL);
  tt_str_op(errmsg, OP_EQ, "That didn't look like an identity digest");
  errmsg = NULL;

  /*
   * Case 7 - downloads/cert/fp/<fp>/sks, and <fp> is not a known authority
   * digest.
   */
  getinfo_helper_downloads(&dummy,
      "downloads/cert/fp/AC4F23B5745BDD2A77997B85B1FD85D05C2E0F61/sks",
      &answer, &errmsg);
  tt_ptr_op(answer, OP_EQ, NULL);
  tt_ptr_op(errmsg, OP_NE, NULL);
  tt_str_op(errmsg, OP_EQ,
      "Failed to get list of signing key digests for this authority "
      "identity digest");
  errmsg = NULL;

  /*
   * Case 8 - looks like downloads/cert/fp/<fp>/<sk>, but <sk> doesn't
   * parse as a signing key digest.
   */
  getinfo_helper_downloads(&dummy,
      "downloads/cert/fp/AC4F23B5745BDD2A77997B85B1FD85D05C2E0F61/"
      "82F52AF55D250115FE44D3GC81D49643241D56A1",
      &answer, &errmsg);
  tt_ptr_op(answer, OP_EQ, NULL);
  tt_ptr_op(errmsg, OP_NE, NULL);
  tt_str_op(errmsg, OP_EQ, "That didn't look like a signing key digest");
  errmsg = NULL;

  /*
   * Case 9 - looks like downloads/cert/fp/<fp>/<sk>, but <sk> doesn't
   * parse as a signing key digest.
   */
  getinfo_helper_downloads(&dummy,
      "downloads/cert/fp/AC4F23B5745BDD2A77997B85B1FD85D05C2E0F61/"
      "82F52AF55D250115FE44D",
      &answer, &errmsg);
  tt_ptr_op(answer, OP_EQ, NULL);
  tt_ptr_op(errmsg, OP_NE, NULL);
  tt_str_op(errmsg, OP_EQ, "That didn't look like a signing key digest");
  errmsg = NULL;

  /*
   * Case 10 - downloads/cert/fp/<fp>/<sk>, but <fp> isn't a known
   * authority identity digest.
   */
  getinfo_helper_downloads(&dummy,
      "downloads/cert/fp/C6B05DF332F74DB9A13498EE3BBC7AA2F69FCB45/"
      "3A214FC21AE25B012C2ECCB5F4EC8A3602D0545D",
      &answer, &errmsg);
  tt_ptr_op(answer, OP_EQ, NULL);
  tt_ptr_op(errmsg, OP_NE, NULL);
  tt_str_op(errmsg, OP_EQ,
      "Failed to get download status for this identity/"
      "signing key digest pair");
  errmsg = NULL;

  /*
   * Case 11 - downloads/cert/fp/<fp>/<sk>, but <sk> isn't a known
   * signing key digest.
   */
  getinfo_helper_downloads(&dummy,
      "downloads/cert/fp/63CDD326DFEF0CA020BDD3FEB45A3286FE13A061/"
      "3A214FC21AE25B012C2ECCB5F4EC8A3602D0545D",
      &answer, &errmsg);
  tt_ptr_op(answer, OP_EQ, NULL);
  tt_ptr_op(errmsg, OP_NE, NULL);
  tt_str_op(errmsg, OP_EQ,
      "Failed to get download status for this identity/"
      "signing key digest pair");
  errmsg = NULL;

  /*
   * Case 12 - downloads/cert/fp/<fp>/<sk>, but <sk> is on the list for
   * a different authority identity digest.
   */
  getinfo_helper_downloads(&dummy,
      "downloads/cert/fp/63CDD326DFEF0CA020BDD3FEB45A3286FE13A061/"
      "9451B8F1B10952384EB58B5F230C0BB701626C9B",
      &answer, &errmsg);
  tt_ptr_op(answer, OP_EQ, NULL);
  tt_ptr_op(errmsg, OP_NE, NULL);
  tt_str_op(errmsg, OP_EQ,
      "Failed to get download status for this identity/"
      "signing key digest pair");
  errmsg = NULL;

 done:
  clear_cert_mocks();
  tor_free(answer);

  return;
}

static void
test_download_status_desc(void *arg)
{
  /* We just need one of these to pass, it doesn't matter what's in it */
  control_connection_t dummy;
  /* Get results out */
  char *question = NULL;
  char *answer = NULL;
  const char *errmsg = NULL;

  (void)arg;

  setup_desc_mocks();

  /*
   * Check returning serialized dlstatuses and digest lists, and implicitly
   * also test download_status_to_string() and digest_list_to_string().
   */

  /* Case 1 - list of router descriptor digests */
  getinfo_helper_downloads(&dummy,
                           "downloads/desc/descs",
                           &answer, &errmsg);
  tt_ptr_op(answer, OP_NE, NULL);
  tt_ptr_op(errmsg, OP_EQ, NULL);
  tt_str_op(answer, OP_EQ, descbr_expected_list);
  tor_free(answer);
  errmsg = NULL;

  /* Case 2 - get download status for router descriptor 1 */
  memcpy(&descbr_digest_1_dl, &dls_sample_1,
         sizeof(download_status_t));
  tor_asprintf(&question, "downloads/desc/%s", descbr_digest_1_str);
  tt_ptr_op(question, OP_NE, NULL);
  getinfo_helper_downloads(&dummy, question, &answer, &errmsg);
  tt_ptr_op(answer, OP_NE, NULL);
  tt_ptr_op(errmsg, OP_EQ, NULL);
  tt_str_op(answer, OP_EQ, dls_sample_1_str);
  tor_free(question);
  tor_free(answer);
  errmsg = NULL;

  /* Case 3 - get download status for router descriptor 1 */
  memcpy(&descbr_digest_2_dl, &dls_sample_2,
         sizeof(download_status_t));
  tor_asprintf(&question, "downloads/desc/%s", descbr_digest_2_str);
  tt_ptr_op(question, OP_NE, NULL);
  getinfo_helper_downloads(&dummy, question, &answer, &errmsg);
  tt_ptr_op(answer, OP_NE, NULL);
  tt_ptr_op(errmsg, OP_EQ, NULL);
  tt_str_op(answer, OP_EQ, dls_sample_2_str);
  tor_free(question);
  tor_free(answer);
  errmsg = NULL;

  /* Now check the error cases */

  /* Case 1 - non-digest-length garbage after downloads/desc */
  getinfo_helper_downloads(&dummy, "downloads/desc/blahdeblah",
                           &answer, &errmsg);
  tt_ptr_op(answer, OP_EQ, NULL);
  tt_ptr_op(errmsg, OP_NE, NULL);
  tt_str_op(errmsg, OP_EQ, "Unknown router descriptor download status query");
  errmsg = NULL;

  /* Case 2 - nonparseable digest-shaped thing */
  getinfo_helper_downloads(
    &dummy,
    "downloads/desc/774EC52FD9A5B80A6FACZE536616E8022E3470AG",
    &answer, &errmsg);
  tt_ptr_op(answer, OP_EQ, NULL);
  tt_ptr_op(errmsg, OP_NE, NULL);
  tt_str_op(errmsg, OP_EQ, "That didn't look like a digest");
  errmsg = NULL;

  /* Case 3 - digest we have no descriptor for */
  getinfo_helper_downloads(
    &dummy,
    "downloads/desc/B05B46135B0B2C04EBE1DD6A6AE4B12D7CD2226A",
    &answer, &errmsg);
  tt_ptr_op(answer, OP_EQ, NULL);
  tt_ptr_op(errmsg, OP_NE, NULL);
  tt_str_op(errmsg, OP_EQ, "No such descriptor digest found");
  errmsg = NULL;

  /* Case 4 - microdescs only */
  disable_descbr = 1;
  getinfo_helper_downloads(&dummy,
                           "downloads/desc/descs",
                           &answer, &errmsg);
  tt_ptr_op(answer, OP_EQ, NULL);
  tt_ptr_op(errmsg, OP_NE, NULL);
  tt_str_op(errmsg, OP_EQ,
            "We don't seem to have a networkstatus-flavored consensus");
  errmsg = NULL;
  disable_descbr = 0;

 done:
  clear_desc_mocks();
  tor_free(answer);

  return;
}

static void
test_download_status_bridge(void *arg)
{
  /* We just need one of these to pass, it doesn't matter what's in it */
  control_connection_t dummy;
  /* Get results out */
  char *question = NULL;
  char *answer = NULL;
  const char *errmsg = NULL;

  (void)arg;

  setup_bridge_mocks();

  /*
   * Check returning serialized dlstatuses and digest lists, and implicitly
   * also test download_status_to_string() and digest_list_to_string().
   */

  /* Case 1 - list of bridge identity digests */
  getinfo_helper_downloads(&dummy,
                           "downloads/bridge/bridges",
                           &answer, &errmsg);
  tt_ptr_op(answer, OP_NE, NULL);
  tt_ptr_op(errmsg, OP_EQ, NULL);
  tt_str_op(answer, OP_EQ, descbr_expected_list);
  tor_free(answer);
  errmsg = NULL;

  /* Case 2 - get download status for bridge descriptor 1 */
  memcpy(&descbr_digest_1_dl, &dls_sample_3,
         sizeof(download_status_t));
  tor_asprintf(&question, "downloads/bridge/%s", descbr_digest_1_str);
  tt_ptr_op(question, OP_NE, NULL);
  getinfo_helper_downloads(&dummy, question, &answer, &errmsg);
  tt_ptr_op(answer, OP_NE, NULL);
  tt_ptr_op(errmsg, OP_EQ, NULL);
  tt_str_op(answer, OP_EQ, dls_sample_3_str);
  tor_free(question);
  tor_free(answer);
  errmsg = NULL;

  /* Case 3 - get download status for router descriptor 1 */
  memcpy(&descbr_digest_2_dl, &dls_sample_4,
         sizeof(download_status_t));
  tor_asprintf(&question, "downloads/bridge/%s", descbr_digest_2_str);
  tt_ptr_op(question, OP_NE, NULL);
  getinfo_helper_downloads(&dummy, question, &answer, &errmsg);
  tt_ptr_op(answer, OP_NE, NULL);
  tt_ptr_op(errmsg, OP_EQ, NULL);
  tt_str_op(answer, OP_EQ, dls_sample_4_str);
  tor_free(question);
  tor_free(answer);
  errmsg = NULL;

  /* Now check the error cases */

  /* Case 1 - non-digest-length garbage after downloads/bridge */
  getinfo_helper_downloads(&dummy, "downloads/bridge/blahdeblah",
                           &answer, &errmsg);
  tt_ptr_op(answer, OP_EQ, NULL);
  tt_ptr_op(errmsg, OP_NE, NULL);
  tt_str_op(errmsg, OP_EQ, "Unknown bridge descriptor download status query");
  errmsg = NULL;

  /* Case 2 - nonparseable digest-shaped thing */
  getinfo_helper_downloads(
    &dummy,
    "downloads/bridge/774EC52FD9A5B80A6FACZE536616E8022E3470AG",
    &answer, &errmsg);
  tt_ptr_op(answer, OP_EQ, NULL);
  tt_ptr_op(errmsg, OP_NE, NULL);
  tt_str_op(errmsg, OP_EQ, "That didn't look like a digest");
  errmsg = NULL;

  /* Case 3 - digest we have no descriptor for */
  getinfo_helper_downloads(
    &dummy,
    "downloads/bridge/B05B46135B0B2C04EBE1DD6A6AE4B12D7CD2226A",
    &answer, &errmsg);
  tt_ptr_op(answer, OP_EQ, NULL);
  tt_ptr_op(errmsg, OP_NE, NULL);
  tt_str_op(errmsg, OP_EQ, "No such bridge identity digest found");
  errmsg = NULL;

  /* Case 4 - bridges disabled */
  disable_descbr = 1;
  getinfo_helper_downloads(&dummy,
                           "downloads/bridge/bridges",
                           &answer, &errmsg);
  tt_ptr_op(answer, OP_EQ, NULL);
  tt_ptr_op(errmsg, OP_NE, NULL);
  tt_str_op(errmsg, OP_EQ, "We don't seem to be using bridges");
  errmsg = NULL;
  disable_descbr = 0;

 done:
  clear_bridge_mocks();
  tor_free(answer);

  return;
}

/** Mock cached consensus */
static cached_dir_t *mock_ns_consensus_cache;
static cached_dir_t *mock_microdesc_consensus_cache;

/**  Mock the function that retrieves consensus from cache. These use a
 * global variable so that they can be cleared from within the test.
 * The actual code retains the pointer to the consensus data, but
 * we are doing this here, to prevent memory leaks
 * from within the tests */
static cached_dir_t *
mock_dirserv_get_consensus(const char *flavor_name)
{
  if (!strcmp(flavor_name, "ns")) {
    mock_ns_consensus_cache = tor_malloc_zero(sizeof(cached_dir_t));
    mock_ns_consensus_cache->dir = tor_strdup("mock_ns_consensus");
    return mock_ns_consensus_cache;
  } else {
    mock_microdesc_consensus_cache = tor_malloc_zero(sizeof(cached_dir_t));
    mock_microdesc_consensus_cache->dir = tor_strdup(
                                            "mock_microdesc_consensus");
    return mock_microdesc_consensus_cache;
  }
}

/** Mock the function that retrieves consensuses
 *  from a files in the directory. */
static tor_mmap_t *
mock_tor_mmap_file(const char* filename)
{
  tor_mmap_t *res;
  res = tor_malloc_zero(sizeof(tor_mmap_t));
  if (strstr(filename, "cached-consensus") != NULL) {
    res->data = "mock_ns_consensus";
  } else if (strstr(filename, "cached-microdesc-consensus") != NULL) {
    res->data = "mock_microdesc_consensus";
  } else {
    res->data = ".";
  }
  res->size = strlen(res->data);
  return res;
}

/** Mock the function that clears file data
 * loaded into the memory */
static int
mock_tor_munmap_file(tor_mmap_t *handle)
{
  tor_free(handle);
  return 0;
}

static void
test_getinfo_helper_current_consensus_from_file(void *arg)
{
  /* We just need one of these to pass, it doesn't matter what's in it */
  control_connection_t dummy;
  /* Get results out */
  char *answer = NULL;
  const char *errmsg = NULL;

  (void)arg;

  MOCK(tor_mmap_file, mock_tor_mmap_file);
  MOCK(tor_munmap_file, mock_tor_munmap_file);

  getinfo_helper_dir(&dummy,
                     "dir/status-vote/current/consensus",
                     &answer,
                     &errmsg);
  tt_str_op(answer, OP_EQ, "mock_ns_consensus");
  tt_ptr_op(errmsg, OP_EQ, NULL);
  tor_free(answer);
  errmsg = NULL;

  getinfo_helper_dir(&dummy,
                     "dir/status-vote/current/consensus-microdesc",
                     &answer,
                     &errmsg);
  tt_str_op(answer, OP_EQ, "mock_microdesc_consensus");
  tt_ptr_op(errmsg, OP_EQ, NULL);
  errmsg = NULL;

 done:
  tor_free(answer);
  UNMOCK(tor_mmap_file);
  UNMOCK(tor_munmap_file);
  return;
}

static void
test_getinfo_helper_current_consensus_from_cache(void *arg)
{
  /* We just need one of these to pass, it doesn't matter what's in it */
  control_connection_t dummy;
  /* Get results out */
  char *answer = NULL;
  const char *errmsg = NULL;

  (void)arg;
  or_options_t *options = get_options_mutable();
  options->FetchUselessDescriptors = 1;
  MOCK(dirserv_get_consensus, mock_dirserv_get_consensus);

  getinfo_helper_dir(&dummy,
                     "dir/status-vote/current/consensus",
                     &answer,
                     &errmsg);
  tt_str_op(answer, OP_EQ, "mock_ns_consensus");
  tt_ptr_op(errmsg, OP_EQ, NULL);
  tor_free(answer);
  tor_free(mock_ns_consensus_cache->dir);
  tor_free(mock_ns_consensus_cache);
  errmsg = NULL;

  getinfo_helper_dir(&dummy,
                     "dir/status-vote/current/consensus-microdesc",
                     &answer,
                     &errmsg);
  tt_str_op(answer, OP_EQ, "mock_microdesc_consensus");
  tt_ptr_op(errmsg, OP_EQ, NULL);
  tor_free(mock_microdesc_consensus_cache->dir);
  tor_free(answer);
  errmsg = NULL;

 done:
  options->FetchUselessDescriptors = 0;
  tor_free(answer);
  tor_free(mock_microdesc_consensus_cache);
  UNMOCK(dirserv_get_consensus);
  return;
}

/** Set timeval to a mock date and time. This is necessary
 * to make tor_gettimeofday() mockable. */
static void
mock_tor_gettimeofday(struct timeval *timeval)
{
  timeval->tv_sec = 1523405073;
  timeval->tv_usec = 271645;
}

static void
test_current_time(void *arg)
{
  /* We just need one of these to pass, it doesn't matter what's in it */
  control_connection_t dummy;
  /* Get results out */
  char *answer = NULL;
  const char *errmsg = NULL;

  (void)arg;

  /* We need these for storing the (mock) time. */
  MOCK(tor_gettimeofday, mock_tor_gettimeofday);
  struct timeval now;
  tor_gettimeofday(&now);
  char timebuf[ISO_TIME_LEN+1];

  /* Case 1 - local time */
  format_local_iso_time_nospace(timebuf, (time_t)now.tv_sec);
  getinfo_helper_current_time(&dummy,
                              "current-time/local",
                              &answer, &errmsg);
  tt_ptr_op(answer, OP_NE, NULL);
  tt_ptr_op(errmsg, OP_EQ, NULL);
  tt_str_op(answer, OP_EQ, timebuf);
  tor_free(answer);
  errmsg = NULL;

  /* Case 2 - UTC time */
  format_iso_time_nospace(timebuf, (time_t)now.tv_sec);
  getinfo_helper_current_time(&dummy,
                              "current-time/utc",
                              &answer, &errmsg);
  tt_ptr_op(answer, OP_NE, NULL);
  tt_ptr_op(errmsg, OP_EQ, NULL);
  tt_str_op(answer, OP_EQ, timebuf);
  tor_free(answer);
  errmsg = NULL;

 done:
  UNMOCK(tor_gettimeofday);
  tor_free(answer);

  return;
}

static size_t n_nodelist_get_list = 0;
static smartlist_t *nodes = NULL;

static const smartlist_t *
mock_nodelist_get_list(void)
{
  n_nodelist_get_list++;
  tor_assert(nodes);

  return nodes;
}

static void
test_getinfo_md_all(void *arg)
{
  char *answer = NULL;
  const char *errmsg = NULL;
  int retval = 0;

  (void)arg;

  node_t *node1 = tor_malloc(sizeof(node_t));
  memset(node1, 0, sizeof(node_t));
  node1->md = tor_malloc(sizeof(microdesc_t));
  memset(node1->md, 0, sizeof(microdesc_t));
  node1->md->body = tor_strdup("md1\n");
  node1->md->bodylen = 4;

  node_t *node2 = tor_malloc(sizeof(node_t));
  memset(node2, 0, sizeof(node_t));
  node2->md = tor_malloc(sizeof(microdesc_t));
  memset(node2->md, 0, sizeof(microdesc_t));
  node2->md->body = tor_strdup("md2\n");
  node2->md->bodylen = 4;

  MOCK(nodelist_get_list, mock_nodelist_get_list);

  nodes = smartlist_new();

  retval = getinfo_helper_dir(NULL, "md/all", &answer, &errmsg);

  tt_int_op(n_nodelist_get_list, OP_EQ, 1);
  tt_int_op(retval, OP_EQ, 0);
  tt_assert(answer != NULL);
  tt_assert(errmsg == NULL);
  tt_str_op(answer, OP_EQ, "");

  tor_free(answer);

  smartlist_add(nodes, node1);
  smartlist_add(nodes, node2);

  retval = getinfo_helper_dir(NULL, "md/all", &answer, &errmsg);

  tt_int_op(n_nodelist_get_list, OP_EQ, 2);
  tt_int_op(retval, OP_EQ, 0);
  tt_assert(answer != NULL);
  tt_assert(errmsg == NULL);

  tt_str_op(answer, OP_EQ, "md1\nmd2\n");

 done:
  UNMOCK(nodelist_get_list);
  tor_free(node1->md->body);
  tor_free(node1->md);
  tor_free(node1);
  tor_free(node2->md->body);
  tor_free(node2->md);
  tor_free(node2);
  tor_free(answer);
  smartlist_free(nodes);
  return;
}

static smartlist_t *reply_strs;

static void
mock_control_write_reply_list(control_connection_t *conn, int code, int c,
                              const char *s)
{
  (void)conn;
  /* To make matching easier, don't append "\r\n" */
  smartlist_add_asprintf(reply_strs, "%03d%c%s", code, c, s);
}

static void
test_control_reply(void *arg)
{
  (void)arg;
  smartlist_t *lines = smartlist_new();

  MOCK(control_write_reply, mock_control_write_reply);

  tor_free(reply_str);
  control_reply_clear(lines);
  control_reply_add_str(lines, 250, "FOO");
  control_write_reply_lines(NULL, lines);
  tt_str_op(reply_str, OP_EQ, "FOO");

  tor_free(reply_str);
  control_reply_clear(lines);
  control_reply_add_done(lines);
  control_write_reply_lines(NULL, lines);
  tt_str_op(reply_str, OP_EQ, "OK");

  tor_free(reply_str);
  control_reply_clear(lines);
  UNMOCK(control_write_reply);
  MOCK(control_write_reply, mock_control_write_reply_list);
  reply_strs = smartlist_new();
  control_reply_add_one_kv(lines, 250, 0, "A", "B");
  control_reply_add_one_kv(lines, 250, 0, "C", "D");
  control_write_reply_lines(NULL, lines);
  tt_int_op(smartlist_len(reply_strs), OP_EQ, 2);
  tt_str_op((char *)smartlist_get(reply_strs, 0), OP_EQ, "250-A=B");
  tt_str_op((char *)smartlist_get(reply_strs, 1), OP_EQ, "250 C=D");

  control_reply_clear(lines);
  SMARTLIST_FOREACH(reply_strs, char *, p, tor_free(p));
  smartlist_clear(reply_strs);
  control_reply_add_printf(lines, 250, "PROTOCOLINFO %d", 1);
  control_reply_add_one_kv(lines, 250, KV_OMIT_VALS|KV_RAW, "AUTH", "");
  control_reply_append_kv(lines, "METHODS", "COOKIE");
  control_reply_append_kv(lines, "COOKIEFILE", escaped("/tmp/cookie"));
  control_reply_add_done(lines);
  control_write_reply_lines(NULL, lines);
  tt_int_op(smartlist_len(reply_strs), OP_EQ, 3);
  tt_str_op((char *)smartlist_get(reply_strs, 0),
            OP_EQ, "250-PROTOCOLINFO 1");
  tt_str_op((char *)smartlist_get(reply_strs, 1),
            OP_EQ, "250-AUTH METHODS=COOKIE COOKIEFILE=\"/tmp/cookie\"");
  tt_str_op((char *)smartlist_get(reply_strs, 2),
            OP_EQ, "250 OK");

 done:
  UNMOCK(control_write_reply);
  tor_free(reply_str);
  control_reply_free(lines);
  if (reply_strs)
    SMARTLIST_FOREACH(reply_strs, char *, p, tor_free(p));
  smartlist_free(reply_strs);
  return;
}

static void
test_control_getconf(void *arg)
{
  (void)arg;
  control_connection_t conn;
  char *args = NULL;
  int r = -1;

  memset(&conn, 0, sizeof(conn));
  conn.current_cmd = tor_strdup("GETCONF");

  MOCK(control_write_reply, mock_control_write_reply_list);
  reply_strs = smartlist_new();

  args = tor_strdup("");
  r = handle_control_command(&conn, (uint32_t)strlen(args), args);
  tt_int_op(r, OP_EQ, 0);
  tt_int_op(smartlist_len(reply_strs), OP_EQ, 1);
  tt_str_op((char *)smartlist_get(reply_strs, 0), OP_EQ, "250 OK");
  SMARTLIST_FOREACH(reply_strs, char *, p, tor_free(p));
  smartlist_clear(reply_strs);
  tor_free(args);

  args = tor_strdup("NoSuch");
  r = handle_control_command(&conn, (uint32_t)strlen(args), args);
  tt_int_op(r, OP_EQ, 0);
  tt_int_op(smartlist_len(reply_strs), OP_EQ, 1);
  tt_str_op((char *)smartlist_get(reply_strs, 0), OP_EQ,
            "552 Unrecognized configuration key \"NoSuch\"");
  tor_free(args);
  SMARTLIST_FOREACH(reply_strs, char *, p, tor_free(p));
  smartlist_clear(reply_strs);

  args = tor_strdup("NoSuch1 NoSuch2");
  r = handle_control_command(&conn, (uint32_t)strlen(args), args);
  tt_int_op(r, OP_EQ, 0);
  tt_int_op(smartlist_len(reply_strs), OP_EQ, 2);
  tt_str_op((char *)smartlist_get(reply_strs, 0), OP_EQ,
            "552-Unrecognized configuration key \"NoSuch1\"");
  tt_str_op((char *)smartlist_get(reply_strs, 1), OP_EQ,
            "552 Unrecognized configuration key \"NoSuch2\"");
  tor_free(args);
  SMARTLIST_FOREACH(reply_strs, char *, p, tor_free(p));
  smartlist_clear(reply_strs);

  args = tor_strdup("ControlPort NoSuch");
  r = handle_control_command(&conn, (uint32_t)strlen(args), args);
  tt_int_op(r, OP_EQ, 0);
  /* Valid keys ignored if there are any invalid ones */
  tt_int_op(smartlist_len(reply_strs), OP_EQ, 1);
  tt_str_op((char *)smartlist_get(reply_strs, 0), OP_EQ,
            "552 Unrecognized configuration key \"NoSuch\"");
  tor_free(args);
  SMARTLIST_FOREACH(reply_strs, char *, p, tor_free(p));
  smartlist_clear(reply_strs);

  args = tor_strdup("ClientOnly");
  r = handle_control_command(&conn, (uint32_t)strlen(args), args);
  tt_int_op(r, OP_EQ, 0);
  tt_int_op(smartlist_len(reply_strs), OP_EQ, 1);
  /* According to config.c, this is an exception for the unit tests */
  tt_str_op((char *)smartlist_get(reply_strs, 0), OP_EQ, "250 ClientOnly=0");
  tor_free(args);
  SMARTLIST_FOREACH(reply_strs, char *, p, tor_free(p));
  smartlist_clear(reply_strs);

  args = tor_strdup("BridgeRelay ClientOnly");
  r = handle_control_command(&conn, (uint32_t)strlen(args), args);
  tt_int_op(r, OP_EQ, 0);
  tt_int_op(smartlist_len(reply_strs), OP_EQ, 2);
  /* Change if config.c changes BridgeRelay default (unlikely) */
  tt_str_op((char *)smartlist_get(reply_strs, 0), OP_EQ, "250-BridgeRelay=0");
  tt_str_op((char *)smartlist_get(reply_strs, 1), OP_EQ, "250 ClientOnly=0");
  tor_free(args);
  SMARTLIST_FOREACH(reply_strs, char *, p, tor_free(p));
  smartlist_clear(reply_strs);

 done:
  tor_free(conn.current_cmd);
  tor_free(args);
  UNMOCK(control_write_reply);
  SMARTLIST_FOREACH(reply_strs, char *, p, tor_free(p));
  smartlist_free(reply_strs);
}

static int
mock_rep_hist_get_circuit_handshake(uint16_t type)
{
  int ret;

  switch (type) {
    case ONION_HANDSHAKE_TYPE_NTOR:
      ret = 80;
      break;
    case ONION_HANDSHAKE_TYPE_TAP:
      ret = 86;
      break;
    default:
      ret = 0;
      break;
  }

  return ret;
}

static void
test_stats(void *arg)
{
  /* We just need one of these to pass, it doesn't matter what's in it */
  control_connection_t dummy;
  /* Get results out */
  char *answer = NULL;
  const char *errmsg = NULL;

  (void) arg;

  /* We need these for returning the (mock) rephist. */
  MOCK(rep_hist_get_circuit_handshake_requested,
       mock_rep_hist_get_circuit_handshake);
  MOCK(rep_hist_get_circuit_handshake_assigned,
       mock_rep_hist_get_circuit_handshake);

  /* NTor tests */
  getinfo_helper_rephist(&dummy, "stats/ntor/requested",
                         &answer, &errmsg);
  tt_ptr_op(answer, OP_NE, NULL);
  tt_ptr_op(errmsg, OP_EQ, NULL);
  tt_str_op(answer, OP_EQ, "80");
  tor_free(answer);
  errmsg = NULL;

  getinfo_helper_rephist(&dummy, "stats/ntor/assigned",
                         &answer, &errmsg);
  tt_ptr_op(answer, OP_NE, NULL);
  tt_ptr_op(errmsg, OP_EQ, NULL);
  tt_str_op(answer, OP_EQ, "80");
  tor_free(answer);
  errmsg = NULL;

  /* TAP tests */
  getinfo_helper_rephist(&dummy, "stats/tap/requested",
                         &answer, &errmsg);
  tt_ptr_op(answer, OP_NE, NULL);
  tt_ptr_op(errmsg, OP_EQ, NULL);
  tt_str_op(answer, OP_EQ, "86");
  tor_free(answer);
  errmsg = NULL;

  getinfo_helper_rephist(&dummy, "stats/tap/assigned",
                         &answer, &errmsg);
  tt_ptr_op(answer, OP_NE, NULL);
  tt_ptr_op(errmsg, OP_EQ, NULL);
  tt_str_op(answer, OP_EQ, "86");
  tor_free(answer);
  errmsg = NULL;

  getinfo_helper_rephist(&dummy, "stats/tap/onion_circuits_ddosed",
                         &answer, &errmsg);
  tt_ptr_op(answer, OP_EQ, NULL);
  tt_str_op(errmsg, OP_EQ, "Unrecognized handshake type");
  errmsg = NULL;

 done:
  UNMOCK(rep_hist_get_circuit_handshake_requested);
  UNMOCK(rep_hist_get_circuit_handshake_assigned);
  tor_free(answer);

  return;
}

#ifndef COCCI
#define PARSER_TEST(type)                                             \
  { "parse/" #type, test_controller_parse_cmd, 0, &passthrough_setup, \
      (void*)&parse_ ## type ## _params }
#endif

struct testcase_t controller_tests[] = {
  PARSER_TEST(one_to_three),
  PARSER_TEST(no_args_one_obj),
  PARSER_TEST(no_args_kwargs),
  PARSER_TEST(one_arg_kwargs),
  { "add_onion_helper_keyarg_v2", test_add_onion_helper_keyarg_v2, 0,
    NULL, NULL },
  { "add_onion_helper_keyarg_v3", test_add_onion_helper_keyarg_v3, 0,
    NULL, NULL },
  { "getinfo_helper_onion", test_getinfo_helper_onion, 0, NULL, NULL },
  { "rend_service_parse_port_config", test_rend_service_parse_port_config, 0,
    NULL, NULL },
  { "add_onion_helper_clientauth", test_add_onion_helper_clientauth, 0, NULL,
    NULL },
  { "download_status_consensus", test_download_status_consensus, 0, NULL,
    NULL },
  {"getinfo_helper_current_consensus_from_cache",
   test_getinfo_helper_current_consensus_from_cache, 0, NULL, NULL },
  {"getinfo_helper_current_consensus_from_file",
   test_getinfo_helper_current_consensus_from_file, 0, NULL, NULL },
  { "download_status_cert", test_download_status_cert, 0, NULL,
    NULL },
  { "download_status_desc", test_download_status_desc, 0, NULL, NULL },
  { "download_status_bridge", test_download_status_bridge, 0, NULL, NULL },
  { "current_time", test_current_time, 0, NULL, NULL },
  { "getinfo_md_all", test_getinfo_md_all, 0, NULL, NULL },
  { "control_reply", test_control_reply, 0, NULL, NULL },
  { "control_getconf", test_control_getconf, 0, NULL, NULL },
  { "stats", test_stats, 0, NULL, NULL },
  END_OF_TESTCASES
};
