/* Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#define CONFIG_PRIVATE
#define RELAY_CONFIG_PRIVATE
#define LOG_PRIVATE
#define ROUTERSET_PRIVATE
#include "core/or/or.h"
#include "lib/confmgt/confmgt.h"
#include "app/config/config.h"
#include "feature/dirauth/dirauth_config.h"
#include "feature/dirauth/dirauth_options_st.h"
#include "feature/dirauth/dirauth_sys.h"
#include "feature/relay/relay_config.h"
#include "test/test.h"
#include "lib/geoip/geoip.h"

#include "feature/nodelist/routerset.h"
#include "core/mainloop/mainloop.h"
#include "app/main/subsysmgr.h"
#include "test/log_test_helpers.h"
#include "test/resolve_test_helpers.h"
#include "lib/crypt_ops/crypto_options_st.h"
#include "lib/crypt_ops/crypto_sys.h"

#include "lib/sandbox/sandbox.h"
#include "lib/memarea/memarea.h"
#include "lib/osinfo/uname.h"
#include "lib/encoding/confline.h"
#include "core/or/policies.h"
#include "test/test_helpers.h"
#include "test/opts_test_helpers.h"
#include "lib/net/resolve.h"

#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif

typedef struct {
  int severity;
  log_domain_mask_t domain;
  char *msg;
} logmsg_t;

static smartlist_t *messages = NULL;

static void
log_cback(int severity, log_domain_mask_t domain, const char *msg)
{
  logmsg_t *x = tor_malloc(sizeof(*x));
  x->severity = severity;
  x->domain = domain;
  x->msg = tor_strdup(msg);
  if (!messages)
    messages = smartlist_new();
  smartlist_add(messages, x);
}

static void
setup_log_callback(void)
{
  log_severity_list_t lst;
  memset(&lst, 0, sizeof(lst));
  lst.masks[SEVERITY_MASK_IDX(LOG_ERR)] = LD_ALL_DOMAINS;
  lst.masks[SEVERITY_MASK_IDX(LOG_WARN)] = LD_ALL_DOMAINS;
  lst.masks[SEVERITY_MASK_IDX(LOG_NOTICE)] = LD_ALL_DOMAINS;
  add_callback_log(&lst, log_cback);
  mark_logs_temp();
}

static char *
dump_logs(void)
{
  smartlist_t *msgs;
  char *out;
  if (! messages)
    return tor_strdup("");
  msgs = smartlist_new();
  SMARTLIST_FOREACH_BEGIN(messages, logmsg_t *, x) {
    smartlist_add_asprintf(msgs, "[%s] %s",
                           log_level_to_string(x->severity), x->msg);
  } SMARTLIST_FOREACH_END(x);
  out = smartlist_join_strings(msgs, "", 0, NULL);
  SMARTLIST_FOREACH(msgs, char *, cp, tor_free(cp));
  smartlist_free(msgs);
  return out;
}

static void
clear_log_messages(void)
{
  if (!messages)
    return;
  SMARTLIST_FOREACH(messages, logmsg_t *, m,
                    { tor_free(m->msg); tor_free(m); });
  smartlist_free(messages);
  messages = NULL;
}

#define setup_options(opt)                   \
  do {                                       \
    opt = options_new();                     \
    opt->command = CMD_RUN_TOR;              \
    options_init(opt);                       \
  } while (0)

#ifdef COCCI

#define ENABLE_AUTHORITY_MIN ""
#define ENABLE_AUTHORITY_V3_MIN ""
#define ENABLE_AUTHORITY_BRIDGE_MIN ""
#define AUTHORITY_OPT_REQ_ ""
#define ENABLE_AUTHORITY ""
#define ENABLE_AUTHORITY_V3 ""
#define ENABLE_AUTHORITY_BRIDGE ""

#else /* !defined(COCCI) */

#define ENABLE_AUTHORITY_MIN \
  "AuthoritativeDirectory 1\n"

#define ENABLE_AUTHORITY_V3_MIN \
  ENABLE_AUTHORITY_MIN \
  "V3AuthoritativeDir 1\n"

#define ENABLE_AUTHORITY_BRIDGE_MIN \
  ENABLE_AUTHORITY_MIN \
  "BridgeAuthoritativeDir 1\n"

#define AUTHORITY_OPT_REQ_ \
  "Address 192.0.2.111\n" \
  "ContactInfo a@example.org\n" \
  "DirPort 1025\n" \
  "ORPort 1026\n"

/* Not actually valid: requires v3 / bridge */
#define ENABLE_AUTHORITY \
  ENABLE_AUTHORITY_MIN \
  AUTHORITY_OPT_REQ_

#define ENABLE_AUTHORITY_V3 \
  ENABLE_AUTHORITY_V3_MIN \
  AUTHORITY_OPT_REQ_

#define ENABLE_AUTHORITY_BRIDGE \
  ENABLE_AUTHORITY_BRIDGE_MIN \
  AUTHORITY_OPT_REQ_

#endif /* defined(COCCI) */

#define VALID_DIR_AUTH "DirAuthority dizum orport=443 v3ident=E8A9C45"  \
  "EDE6D711294FADF8E7951F4DE6CA56B58 194.109.206.212:80 7EA6 EAD6 FD83" \
  " 083C 538F 4403 8BBF A077 587D D755\n"
#define VALID_ALT_BRIDGE_AUTH \
  "AlternateBridgeAuthority dizum orport=443 v3ident=E8A9C45"           \
  "EDE6D711294FADF8E7951F4DE6CA56B58 194.109.206.212:80 7EA6 EAD6 FD83" \
  " 083C 538F 4403 8BBF A077 587D D755\n"
#define VALID_ALT_DIR_AUTH \
  "AlternateDirAuthority dizum orport=443 v3ident=E8A9C45"           \
  "EDE6D711294FADF8E7951F4DE6CA56B58 194.109.206.212:80 7EA6 EAD6 FD83" \
  " 083C 538F 4403 8BBF A077 587D D755\n"

static int
test_options_checklog(const char *configuration, int expect_log_severity,
                      const char *expect_log)
{
  int found = 0, ret = -1;
  char *actual_log = NULL;

  if (messages) {
    SMARTLIST_FOREACH_BEGIN(messages, logmsg_t *, m) {
      if (m->severity == expect_log_severity &&
          strstr(m->msg, expect_log)) {
        found = 1;
        break;
      }
    } SMARTLIST_FOREACH_END(m);
  }
  if (!found) {
    actual_log = dump_logs();
    TT_DIE(("Expected log message [%s] %s from <%s>, but got <%s>.",
            log_level_to_string(expect_log_severity), expect_log,
            configuration, actual_log));
  }
  ret = 0;

 done:
  tor_free(actual_log);
  return ret;
}

static int
test_options_checkmsgs(const char *configuration,
                       const char *expect_errmsg,
                       int expect_log_severity,
                       const char *expect_log,
                       char *msg)
{
  if (expect_errmsg && !msg) {
    TT_DIE(("Expected error message <%s> from <%s>, but got none.",
            expect_errmsg, configuration));
  } else if (expect_errmsg && !strstr(msg, expect_errmsg)) {
    TT_DIE(("Expected error message <%s> from <%s>, but got <%s>.",
            expect_errmsg, configuration, msg));
  } else if (!expect_errmsg && msg) {
    TT_DIE(("Expected no error message from <%s> but got <%s>.",
            configuration, msg));
  }
  if (expect_log) {
    return test_options_checklog(configuration, expect_log_severity,
                                 expect_log);
  }
  return 0;

 done:
  return -1;
}

/* Which phases of config parsing/validation to check for messages/logs */
enum { PH_GETLINES, PH_ASSIGN, PH_VALIDATE };

static void
test_options_validate_impl(const char *configuration,
                           const char *expect_errmsg,
                           int expect_log_severity,
                           const char *expect_log,
                           int phase)
{
  or_options_t *opt=NULL;
  config_line_t *cl=NULL;
  char *msg=NULL;
  int r;

  setup_options(opt);

  r = config_get_lines(configuration, &cl, 1);
  if (phase == PH_GETLINES) {
    if (test_options_checkmsgs(configuration, expect_errmsg,
                               expect_log_severity,
                               expect_log, msg))
      goto done;
  }
  if (r)
    goto done;

  r = config_assign(get_options_mgr(), opt, cl, 0, &msg);
  if (phase == PH_ASSIGN) {
    if (test_options_checkmsgs(configuration, expect_errmsg,
                               expect_log_severity,
                               expect_log, msg))
      goto done;
  }
  tt_int_op((r == 0), OP_EQ, (msg == NULL));
  if (r)
    goto done;

  r = options_validate(NULL, opt, &msg);
  if (phase == PH_VALIDATE) {
    if (test_options_checkmsgs(configuration, expect_errmsg,
                               expect_log_severity,
                               expect_log, msg))
      goto done;
  }
  tt_int_op((r == 0), OP_EQ, (msg == NULL));

 done:
  escaped(NULL);
  policies_free_all();
  config_free_lines(cl);
  or_options_free(opt);
  tor_free(msg);
  clear_log_messages();
}

#define WANT_ERR(config, msg, ph)                               \
  test_options_validate_impl((config), (msg), 0, NULL, (ph))
#define WANT_LOG(config, severity, msg, ph)                             \
  test_options_validate_impl((config), NULL, (severity), (msg), (ph))
#define WANT_ERR_LOG(config, msg, severity, logmsg, ph)                 \
  test_options_validate_impl((config), (msg), (severity), (logmsg), (ph))
#define OK(config, ph)                                          \
  test_options_validate_impl((config), NULL, 0, NULL, (ph))

static void
test_options_validate(void *arg)
{
  (void)arg;
  setup_log_callback();
  sandbox_disable_getaddrinfo_cache();
  mock_hostname_resolver();

  WANT_ERR("ExtORPort 500000", "Invalid ExtORPort", PH_VALIDATE);

  WANT_ERR_LOG("ServerTransportOptions trebuchet",
               "ServerTransportOptions did not parse",
               LOG_WARN, "Too few arguments", PH_VALIDATE);
  OK("ServerTransportOptions trebuchet sling=snappy", PH_VALIDATE);
  OK("ServerTransportOptions trebuchet sling=", PH_VALIDATE);
  WANT_ERR_LOG("ServerTransportOptions trebuchet slingsnappy",
               "ServerTransportOptions did not parse",
               LOG_WARN, "\"slingsnappy\" is not a k=v", PH_VALIDATE);

  WANT_ERR("BridgeRelay 1\nDirCache 0",
           "We're a bridge but DirCache is disabled.", PH_VALIDATE);

  WANT_ERR("HeartbeatPeriod 21 snarks",
           "Unknown unit in 21 snarks", PH_ASSIGN);
  WANT_ERR("LogTimeGranularity 21 snarks",
           "Unknown unit in 21 snarks", PH_ASSIGN);
  OK("HeartbeatPeriod 1 hour", PH_VALIDATE);
  OK("LogTimeGranularity 100 milliseconds", PH_VALIDATE);

  WANT_LOG("ControlSocket \"string with trailing garbage\" bogus", LOG_WARN,
           "Error while parsing configuration: "
           "Excess data after quoted string", PH_GETLINES);
  WANT_LOG("ControlSocket \"bogus escape \\@\"", LOG_WARN,
           "Error while parsing configuration: "
           "Invalid escape sequence in quoted string", PH_GETLINES);

  close_temp_logs();
  clear_log_messages();
  unmock_hostname_resolver();
  return;
}

#define MEGABYTEIFY(mb) (UINT64_C(mb) << 20)
static void
test_have_enough_mem_for_dircache(void *arg)
{
  (void)arg;
  or_options_t *opt=NULL;
  config_line_t *cl=NULL;
  char *msg=NULL;
  int r;
  const char *configuration = "ORPort 8080\nDirCache 1", *expect_errmsg;

  setup_options(opt);
  setup_log_callback();

  r = config_get_lines(configuration, &cl, 1);
  tt_int_op(r, OP_EQ, 0);

  r = config_assign(get_options_mgr(), opt, cl, 0, &msg);
  tt_int_op(r, OP_EQ, 0);

  /* 300 MB RAM available, DirCache enabled */
  r = have_enough_mem_for_dircache(opt, MEGABYTEIFY(300), &msg);
  tt_int_op(r, OP_EQ, 0);
  tt_ptr_op(msg, OP_EQ, NULL);

  /* 200 MB RAM available, DirCache enabled */
  r = have_enough_mem_for_dircache(opt, MEGABYTEIFY(200), &msg);
  tt_int_op(r, OP_EQ, -1);
  expect_errmsg = "Being a directory cache (default) with less than ";
  if (!strstr(msg, expect_errmsg)) {
    TT_DIE(("Expected error message <%s> from <%s>, but got <%s>.",
            expect_errmsg, configuration, msg));
  }
  tor_free(msg);

  config_free_lines(cl); cl = NULL;
  configuration = "ORPort 8080\nDirCache 1\nBridgeRelay 1";
  r = config_get_lines(configuration, &cl, 1);
  tt_int_op(r, OP_EQ, 0);

  r = config_assign(get_options_mgr(), opt, cl, 0, &msg);
  tt_int_op(r, OP_EQ, 0);

  /* 300 MB RAM available, DirCache enabled, Bridge */
  r = have_enough_mem_for_dircache(opt, MEGABYTEIFY(300), &msg);
  tt_int_op(r, OP_EQ, 0);
  tt_ptr_op(msg, OP_EQ, NULL);

  /* 200 MB RAM available, DirCache enabled, Bridge */
  r = have_enough_mem_for_dircache(opt, MEGABYTEIFY(200), &msg);
  tt_int_op(r, OP_EQ, -1);
  expect_errmsg = "Running a Bridge with less than ";
  if (!strstr(msg, expect_errmsg)) {
    TT_DIE(("Expected error message <%s> from <%s>, but got <%s>.",
            expect_errmsg, configuration, msg));
  }
  tor_free(msg);

  config_free_lines(cl); cl = NULL;
  configuration = "ORPort 8080\nDirCache 0";
  r = config_get_lines(configuration, &cl, 1);
  tt_int_op(r, OP_EQ, 0);

  r = config_assign(get_options_mgr(), opt, cl, 0, &msg);
  tt_int_op(r, OP_EQ, 0);

  /* 200 MB RAM available, DirCache disabled */
  r = have_enough_mem_for_dircache(opt, MEGABYTEIFY(200), &msg);
  tt_int_op(r, OP_EQ, 0);
  tt_ptr_op(msg, OP_EQ, NULL);

  /* 300 MB RAM available, DirCache disabled */
  r = have_enough_mem_for_dircache(opt, MEGABYTEIFY(300), &msg);
  tt_int_op(r, OP_EQ, -1);
  expect_errmsg = "DirCache is disabled and we are configured as a ";
  if (!strstr(msg, expect_errmsg)) {
    TT_DIE(("Expected error message <%s> from <%s>, but got <%s>.",
            expect_errmsg, configuration, msg));
  }
  tor_free(msg);

  clear_log_messages();

 done:
  if (msg)
    tor_free(msg);
  or_options_free(opt);
  config_free_lines(cl);
  return;
}

static const char *fixed_get_uname_result = NULL;

static const char *
fixed_get_uname(void)
{
  return fixed_get_uname_result;
}

typedef struct {
  or_options_t *opt;
} options_test_data_t;

static void free_options_test_data(options_test_data_t *td);

static options_test_data_t *
get_options_test_data(const char *conf)
{
  int rv = -1;
  char *msg = NULL;
  config_line_t *cl=NULL;
  options_test_data_t *result = tor_malloc(sizeof(options_test_data_t));
  result->opt = options_new();

  options_init(result->opt);

  rv = config_get_lines(conf, &cl, 1);
  tt_int_op(rv, OP_EQ, 0);
  rv = config_assign(get_options_mgr(), result->opt, cl, 0, &msg);
  if (msg) {
    /* Display the parse error message by comparing it with an empty string */
    tt_str_op(msg, OP_EQ, "");
  }
  tt_int_op(rv, OP_EQ, 0);
  config_free_lines(cl);
  result->opt->LogTimeGranularity = 1;
  result->opt->TokenBucketRefillInterval = 1;
  rv = config_get_lines("", &cl, 1);
  tt_int_op(rv, OP_EQ, 0);

 done:
  config_free_lines(cl);
  if (rv != 0) {
    free_options_test_data(result);
    result = NULL;
    /* Callers expect a non-NULL result, so just die if we can't provide one.
     */
    tor_assert(0);
  }
  return result;
}

static void
free_options_test_data(options_test_data_t *td)
{
  if (!td) return;
  or_options_free(td->opt);
  tor_free(td);
}

static void
test_options_validate__uname_for_server(void *ignored)
{
  (void)ignored;
  char *msg;

#ifndef _WIN32
  int unset_home_env = 0;
  if (setenv("HOME", "/home/john", 0) == 0)
    unset_home_env = 1;
#endif

  options_test_data_t *tdata = get_options_test_data(
                                      "ORPort 127.0.0.1:5555\n"
                                      "ContactInfo nobody@example.com");
  setup_capture_of_logs(LOG_WARN);

  MOCK(get_uname, fixed_get_uname);
  fixed_get_uname_result = "Windows 95";
  options_validate(NULL, tdata->opt, &msg);
  expect_log_msg("Tor is running as a server, but you"
           " are running Windows 95; this probably won't work. See https://www"
           ".torproject.org/docs/faq.html#BestOSForRelay for details.\n");
  tor_free(msg);

  fixed_get_uname_result = "Windows 98";
  mock_clean_saved_logs();
  options_validate(NULL, tdata->opt, &msg);
  expect_log_msg("Tor is running as a server, but you"
           " are running Windows 98; this probably won't work. See https://www"
           ".torproject.org/docs/faq.html#BestOSForRelay for details.\n");
  tor_free(msg);

  fixed_get_uname_result = "Windows Me";
  mock_clean_saved_logs();
  options_validate(NULL, tdata->opt, &msg);
  expect_log_msg("Tor is running as a server, but you"
           " are running Windows Me; this probably won't work. See https://www"
           ".torproject.org/docs/faq.html#BestOSForRelay for details.\n");
  tor_free(msg);

  fixed_get_uname_result = "Windows 2000";
  mock_clean_saved_logs();
  options_validate(NULL, tdata->opt, &msg);
  expect_no_log_msg("Tor is running as a server, but you ");
  tor_free(msg);

 done:
  UNMOCK(get_uname);
  free_options_test_data(tdata);
  tor_free(msg);
  teardown_capture_of_logs();
#ifndef _WIN32
  if (unset_home_env)
    unsetenv("HOME");
#endif
}

static void
test_options_validate__outbound_addresses(void *ignored)
{
  (void)ignored;
  int ret;
  char *msg;
  options_test_data_t *tdata = get_options_test_data(
                                    "OutboundBindAddress xxyy!!!sdfaf");

  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ, "Multiple outbound bind addresses configured: "
                        "xxyy!!!sdfaf");

 done:
  free_options_test_data(tdata);
  tor_free(msg);
}

static void
test_options_validate__data_directory(void *ignored)
{
  (void)ignored;
  int ret;
  char *msg;
  options_test_data_t *tdata = get_options_test_data(
                                                "DataDirectory longreallyl"
                                                "ongLONGLONGlongreallylong"
                                                "LONGLONGlongreallylongLON"
                                                "GLONGlongreallylongLONGLO"
                                                "NGlongreallylongLONGLONGl"
                                                "ongreallylongLONGLONGlong"
                                                "reallylongLONGLONGlongrea"
                                                "llylongLONGLONGlongreally"
                                                "longLONGLONGlongreallylon"
                                                "gLONGLONGlongreallylongLO"
                                                "NGLONGlongreallylongLONGL"
                                                "ONGlongreallylongLONGLONG"
                                                "longreallylongLONGLONGlon"
                                                "greallylongLONGLONGlongre"
                                                "allylongLONGLONGlongreall"
                                                "ylongLONGLONGlongreallylo"
                                                "ngLONGLONGlongreallylongL"
                                                "ONGLONGlongreallylongLONG"
                                                "LONG"); // 440 characters

  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ, "Invalid DataDirectory");

 done:
  free_options_test_data(tdata);
  tor_free(msg);
}

static void
test_options_validate__nickname(void *ignored)
{
  (void)ignored;
  int ret;
  char *msg;
  options_test_data_t *tdata = get_options_test_data(
                                        "Nickname ThisNickNameIsABitTooLong");

  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ,
            "Nickname 'ThisNickNameIsABitTooLong', nicknames must be between "
            "1 and 19 characters inclusive, and must contain only the "
            "characters [a-zA-Z0-9].");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("Nickname AMoreValidNick");
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("DataDirectory /tmp/somewhere");
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);

 done:
  free_options_test_data(tdata);
  tor_free(msg);
}

static void
test_options_validate__contactinfo(void *ignored)
{
  (void)ignored;
  int ret;
  char *msg;
  options_test_data_t *tdata = get_options_test_data(
                                "ORPort 127.0.0.1:5555");
  setup_capture_of_logs(LOG_DEBUG);
  tdata->opt->ContactInfo = NULL;

  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  expect_log_msg(
           "Your ContactInfo config option is not set. Please strongly "
           "consider setting it, so we can contact you if your relay is "
           "misconfigured, end-of-life, or something else goes wrong. It "
           "is also possible that your relay might get rejected from the "
           "network due to a missing valid contact address.\n");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("ORPort 127.0.0.1:5555\n"
                                "ContactInfo hella@example.org");
  mock_clean_saved_logs();
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  expect_no_log_msg(
           "Your ContactInfo config option is not set. Please strongly "
           "consider setting it, so we can contact you if your relay is "
           "misconfigured, end-of-life, or something else goes wrong. It "
           "is also possible that your relay might get rejected from the "
           "network due to a missing valid contact address.\n");
  tor_free(msg);

 done:
  teardown_capture_of_logs();
  free_options_test_data(tdata);
  tor_free(msg);
}

static void
test_options_validate__logs(void *ignored)
{
  (void)ignored;
  int ret;
  (void)ret;
  char *msg;
  int orig_quiet_level = quiet_level;
  options_test_data_t *tdata = get_options_test_data("");
  tdata->opt->Logs = NULL;
  tdata->opt->RunAsDaemon = 0;

  ret = options_validate(NULL, tdata->opt, &msg);
  tt_assert(!tdata->opt->Logs);
  tor_free(msg);
  tt_int_op(ret, OP_EQ, 0);

  free_options_test_data(tdata);
  tdata = get_options_test_data("");
  tdata->opt->Logs = NULL;
  tdata->opt->RunAsDaemon = 0;
  quiet_level = 1;
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_assert(!tdata->opt->Logs);
  tor_free(msg);
  tt_int_op(ret, OP_EQ, 0);

  free_options_test_data(tdata);
  tdata = get_options_test_data("");
  tdata->opt->Logs = NULL;
  tdata->opt->RunAsDaemon = 0;
  quiet_level = 2;
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_assert(!tdata->opt->Logs);
  tor_free(msg);
  tt_int_op(ret, OP_EQ, 0);

  free_options_test_data(tdata);
  tdata = get_options_test_data("");
  tdata->opt->Logs = NULL;
  tdata->opt->RunAsDaemon = 0;
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_assert(!tdata->opt->Logs);
  tor_free(msg);
  tt_int_op(ret, OP_EQ, 0);

  free_options_test_data(tdata);
  tdata = get_options_test_data("");
  tdata->opt->Logs = NULL;
  tdata->opt->RunAsDaemon = 1;
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_assert(!tdata->opt->Logs);
  tor_free(msg);
#ifdef _WIN32
  /* Can't RunAsDaemon on Windows. */
  tt_int_op(ret, OP_EQ, -1);
#else
  tt_int_op(ret, OP_EQ, 0);
#endif /* defined(_WIN32) */

  free_options_test_data(tdata);
  tdata = get_options_test_data("");
  tdata->opt->RunAsDaemon = 0;
  config_line_t *cl=NULL;
  config_get_lines("Log foo", &cl, 1);
  tdata->opt->Logs = cl;
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op((intptr_t)tdata->opt->Logs, OP_EQ, (intptr_t)cl);
  tt_int_op(ret, OP_EQ, -1);

 done:
  quiet_level = orig_quiet_level;
  free_options_test_data(tdata);
  tor_free(msg);
}

/* static config_line_t * */
/* mock_config_line(const char *key, const char *val) */
/* { */
/*   config_line_t *config_line = tor_malloc(sizeof(config_line_t)); */
/*   memset(config_line, 0, sizeof(config_line_t)); */
/*   config_line->key = tor_strdup(key); */
/*   config_line->value = tor_strdup(val); */
/*   return config_line; */
/* } */

static void
test_options_validate__authdir(void *ignored)
{
  (void)ignored;
  int ret;
  char *msg;
  setup_capture_of_logs(LOG_INFO);
  options_test_data_t *tdata = get_options_test_data(
                                 ENABLE_AUTHORITY_V3_MIN
                                 "Address this.should.not!exist!.example.org");
  const dirauth_options_t *da_opt;

  sandbox_disable_getaddrinfo_cache();

  MOCK(tor_addr_lookup, mock_tor_addr_lookup__fail_on_bad_addrs);
  ret = options_validate(NULL, tdata->opt, &msg);
  UNMOCK(tor_addr_lookup);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ, "Failed to resolve/guess local address. See logs for"
            " details.");
  expect_log_msg("Could not resolve local Address "
            "'this.should.not!exist!.example.org'. Failing.\n");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data(ENABLE_AUTHORITY_V3_MIN
                                "Address 100.200.10.1");
  mock_clean_saved_logs();
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ, "Authoritative directory servers must set "
                        "ContactInfo");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data(ENABLE_AUTHORITY_V3_MIN
                                "Address 100.200.10.1\n");
  mock_clean_saved_logs();
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ,
            "Authoritative directory servers must set ContactInfo");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data(ENABLE_AUTHORITY_MIN
                                "Address 100.200.10.1\n"
                                "TestingTorNetwork 1\n");
  mock_clean_saved_logs();
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ, "AuthoritativeDir is set, but none of (Bridge/V3)"
            "AuthoritativeDir is set.");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data(ENABLE_AUTHORITY);
  mock_clean_saved_logs();
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ, "AuthoritativeDir is set, but none of (Bridge/V3)"
            "AuthoritativeDir is set.");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data(ENABLE_AUTHORITY_V3
                                "RecommendedVersions 1.2, 3.14\n");
  mock_clean_saved_logs();
  options_validate(NULL, tdata->opt, &msg);
  da_opt = get_dirauth_options(tdata->opt);
  tt_str_op(da_opt->RecommendedClientVersions->value, OP_EQ, "1.2, 3.14");
  tt_str_op(da_opt->RecommendedServerVersions->value, OP_EQ, "1.2, 3.14");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data(ENABLE_AUTHORITY_V3
                                "RecommendedVersions 1.2, 3.14\n"
                                "RecommendedClientVersions 25\n"
                                "RecommendedServerVersions 4.18\n");
  mock_clean_saved_logs();
  options_validate(NULL, tdata->opt, &msg);
  da_opt = get_dirauth_options(tdata->opt);
  tt_str_op(da_opt->RecommendedClientVersions->value, OP_EQ, "25");
  tt_str_op(da_opt->RecommendedServerVersions->value, OP_EQ, "4.18");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data(ENABLE_AUTHORITY
                                "VersioningAuthoritativeDirectory 1\n"
                                "RecommendedVersions 1.2, 3.14\n"
                                "RecommendedClientVersions 25\n"
                                "RecommendedServerVersions 4.18\n");
  mock_clean_saved_logs();
  options_validate(NULL, tdata->opt, &msg);
  da_opt = get_dirauth_options(tdata->opt);
  tt_str_op(msg, OP_EQ, "AuthoritativeDir is set, but none of (Bridge/V3)"
            "AuthoritativeDir is set.");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data(ENABLE_AUTHORITY_V3
                                "VersioningAuthoritativeDirectory 1\n"
                                "RecommendedServerVersions 4.18\n");
  mock_clean_saved_logs();
  options_validate(NULL, tdata->opt, &msg);
  da_opt = get_dirauth_options(tdata->opt);
  tt_str_op(msg, OP_EQ, "Versioning authoritative dir servers must set "
            "Recommended*Versions.");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data(ENABLE_AUTHORITY_V3
                                "VersioningAuthoritativeDirectory 1\n"
                                "RecommendedClientVersions 4.18\n");
  mock_clean_saved_logs();
  options_validate(NULL, tdata->opt, &msg);
  da_opt = get_dirauth_options(tdata->opt);
  tt_str_op(msg, OP_EQ, "Versioning authoritative dir servers must set "
            "Recommended*Versions.");
  tor_free(msg);
  da_opt = NULL;

  free_options_test_data(tdata);
  tdata = get_options_test_data(ENABLE_AUTHORITY_V3
                                "UseEntryGuards 1\n");
  mock_clean_saved_logs();
  options_validate(NULL, tdata->opt, &msg);
  expect_log_msg("Authoritative directory servers "
            "can't set UseEntryGuards. Disabling.\n");
  tt_int_op(tdata->opt->UseEntryGuards, OP_EQ, 0);
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data(ENABLE_AUTHORITY_V3
                                "DownloadExtraInfo 0\n");
  mock_clean_saved_logs();
  options_validate(NULL, tdata->opt, &msg);
  expect_log_msg("Authoritative directories always try"
            " to download extra-info documents. Setting DownloadExtraInfo.\n");
  tt_int_op(tdata->opt->DownloadExtraInfo, OP_EQ, 1);
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data(ENABLE_AUTHORITY_V3
                                "V3BandwidthsFile non-existent-file\n");
  mock_clean_saved_logs();
  options_validate(NULL, tdata->opt, &msg);
  expect_log_msg("Can't open bandwidth file at configured location: "
                 "non-existent-file\n");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data(ENABLE_AUTHORITY_V3
                                "GuardfractionFile non-existent-file\n");
  mock_clean_saved_logs();
  options_validate(NULL, tdata->opt, &msg);
  expect_log_msg("Cannot open guardfraction file 'non-existent-file'. "
                 "Failing.\n");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data(ENABLE_AUTHORITY_V3_MIN
                                "Address 100.200.10.1\n"
                                "ORPort 2000\n"
                                "ContactInfo hello@hello.com\n");
  mock_clean_saved_logs();
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ,
            "Running as authoritative directory, but no DirPort set.");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data(ENABLE_AUTHORITY_BRIDGE_MIN
                                "Address 100.200.10.1\n"
                                "ORPort 2000\n"
                                "ContactInfo hello@hello.com\n");
  mock_clean_saved_logs();
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ,
            "Running as authoritative directory, but no DirPort set.");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data(ENABLE_AUTHORITY_V3_MIN
                                "Address 100.200.10.1\n"
                                "DirPort 999\n"
                                "ContactInfo hello@hello.com\n");
  mock_clean_saved_logs();
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ,
            "Running as authoritative directory, but no ORPort set.");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data(ENABLE_AUTHORITY_BRIDGE_MIN
                                "Address 100.200.10.1\n"
                                "DirPort 999\n"
                                "ContactInfo hello@hello.com\n");
  mock_clean_saved_logs();
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ,
            "Running as authoritative directory, but no ORPort set.");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data(ENABLE_AUTHORITY_V3
                                "ClientOnly 1\n");
  /* We have to call the dirauth-specific function, and fake port parsing,
   * to hit this case */
  tdata->opt->DirPort_set = 1;
  tdata->opt->ORPort_set = 1;
  mock_clean_saved_logs();
  ret = options_validate_dirauth_mode(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ, "Running as authoritative directory, "
            "but ClientOnly also set.");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data(ENABLE_AUTHORITY_BRIDGE
                                "ClientOnly 1\n");
  /* We have to call the dirauth-specific function, and fake port parsing,
   * to hit this case */
  tdata->opt->DirPort_set = 1;
  tdata->opt->ORPort_set = 1;
  mock_clean_saved_logs();
  ret = options_validate_dirauth_mode(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ, "Running as authoritative directory, "
            "but ClientOnly also set.");
  tor_free(msg);

 done:
  teardown_capture_of_logs();
  //  sandbox_free_getaddrinfo_cache();
  free_options_test_data(tdata);
  tor_free(msg);
}

static void
test_options_validate__relay_with_hidden_services(void *ignored)
{
  (void)ignored;
  char *msg;
  int ret;
  setup_capture_of_logs(LOG_DEBUG);
  options_test_data_t *tdata = get_options_test_data(
                                  "ORPort 127.0.0.1:5555\n"
                                  "HiddenServiceDir "
                                  "/Library/Tor/var/lib/tor/hidden_service/\n"
                                  "HiddenServicePort 80 127.0.0.1:8080\n"
                                                     );

  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  expect_log_msg(
            "Tor is currently configured as a relay and a hidden service. "
            "That's not very secure: you should probably run your hidden servi"
            "ce in a separate Tor process, at least -- see "
            "https://bugs.torproject.org/tpo/core/tor/8742.\n");

 done:
  teardown_capture_of_logs();
  free_options_test_data(tdata);
  tor_free(msg);
}

static void
test_options_validate__listen_ports(void *ignored)
{
  (void)ignored;
  int ret;
  char *msg;
  setup_capture_of_logs(LOG_WARN);
  options_test_data_t *tdata = get_options_test_data("SOCKSPort 0");
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  expect_log_msg("SocksPort, TransPort, NATDPort, DNSPort, and ORPort "
                 "are all undefined, and there aren't any hidden services "
                 "configured. "
                 " Tor will still run, but probably won't do anything.\n");
 done:
  teardown_capture_of_logs();
  free_options_test_data(tdata);
  tor_free(msg);
}

static void
test_options_validate__transproxy(void *ignored)
{
  (void)ignored;
  int ret;
  char *msg;
  options_test_data_t *tdata;

#ifdef USE_TRANSPARENT
  // Test default trans proxy
  tdata = get_options_test_data("TransProxyType default\n");

  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  tt_int_op(tdata->opt->TransProxyType_parsed, OP_EQ, TPT_DEFAULT);
  tor_free(msg);

  // Test pf-divert trans proxy
  free_options_test_data(tdata);
  tdata = get_options_test_data("TransProxyType pf-divert\n");
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);

#if !defined(OpenBSD) && !defined(DARWIN)
  tt_str_op(msg, OP_EQ,
          "pf-divert is a OpenBSD-specific and OS X/Darwin-specific feature.");
#else
  tt_int_op(tdata->opt->TransProxyType_parsed, OP_EQ, TPT_PF_DIVERT);
  tt_str_op(msg, OP_EQ, "Cannot use TransProxyType without "
            "any valid TransPort.");
#endif /* !defined(OpenBSD) && !defined(DARWIN) */
  tor_free(msg);

  // Test tproxy trans proxy
  free_options_test_data(tdata);
  tdata = get_options_test_data("TransProxyType tproxy\n");
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);

#if !defined(__linux__)
  tt_str_op(msg, OP_EQ, "TPROXY is a Linux-specific feature.");
#else
  tt_int_op(tdata->opt->TransProxyType_parsed, OP_EQ, TPT_TPROXY);
  tt_str_op(msg, OP_EQ, "Cannot use TransProxyType without any valid "
            "TransPort.");
#endif /* !defined(__linux__) */
  tor_free(msg);

  // Test ipfw trans proxy
  free_options_test_data(tdata);
  tdata = get_options_test_data("TransProxyType ipfw\n");
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);

#ifndef KERNEL_MAY_SUPPORT_IPFW
  tt_str_op(msg, OP_EQ, "ipfw is a FreeBSD-specific and OS X/Darwin-specific "
            "feature.");
#else
  tt_int_op(tdata->opt->TransProxyType_parsed, OP_EQ, TPT_IPFW);
  tt_str_op(msg, OP_EQ, "Cannot use TransProxyType without any valid "
            "TransPort.");
#endif /* !defined(KERNEL_MAY_SUPPORT_IPFW) */
  tor_free(msg);

  // Test unknown trans proxy
  free_options_test_data(tdata);
  tdata = get_options_test_data("TransProxyType non-existent\n");
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ, "Unrecognized value for TransProxyType");
  tor_free(msg);

  // Test trans proxy success
  free_options_test_data(tdata);
  tdata = NULL;

#if defined(__linux__)
  tdata = get_options_test_data("TransProxyType tproxy\n"
                                "TransPort 127.0.0.1:123\n");
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
#elif defined(KERNEL_MAY_SUPPORT_IPFW)
  tdata = get_options_test_data("TransProxyType ipfw\n"
                                "TransPort 127.0.0.1:123\n");
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  tor_free(msg);
#elif defined(OpenBSD)
  tdata = get_options_test_data("TransProxyType pf-divert\n"
                                "TransPort 127.0.0.1:123\n");
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  tor_free(msg);
#elif defined(__NetBSD__)
  tdata = get_options_test_data("TransProxyType default\n"
                                "TransPort 127.0.0.1:123\n");
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  tor_free(msg);
#endif /* defined(__linux__) || ... */

  // Assert that a test has run for some TransProxyType
  tt_assert(tdata);

#else /* !defined(USE_TRANSPARENT) */
  tdata = get_options_test_data("TransPort 127.0.0.1:555\n");

  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ, "TransPort is disabled in this build.");
  tor_free(msg);
#endif /* defined(USE_TRANSPARENT) */

 done:
  free_options_test_data(tdata);
  tor_free(msg);
}

static country_t opt_tests_geoip_get_country(const char *country);
ATTR_UNUSED static int opt_tests_geoip_get_country_called = 0;

static country_t
opt_tests_geoip_get_country(const char *countrycode)
{
  (void)countrycode;
  opt_tests_geoip_get_country_called++;

  return 1;
}

static void
test_options_validate__exclude_nodes(void *ignored)
{
  (void)ignored;

  MOCK(geoip_get_country,
       opt_tests_geoip_get_country);

  int ret;
  char *msg;
  setup_capture_of_logs(LOG_WARN);
  options_test_data_t *tdata = get_options_test_data(
                                                  "ExcludeExitNodes {us}\n");

  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  tt_int_op(smartlist_len(tdata->opt->ExcludeExitNodesUnion_->list), OP_EQ, 1);
  tt_str_op((char *)
            (smartlist_get(tdata->opt->ExcludeExitNodesUnion_->list, 0)),
            OP_EQ, "{us}");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("ExcludeNodes {cn}\n");
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  tt_int_op(smartlist_len(tdata->opt->ExcludeExitNodesUnion_->list), OP_EQ, 1);
  tt_str_op((char *)
            (smartlist_get(tdata->opt->ExcludeExitNodesUnion_->list, 0)),
            OP_EQ, "{cn}");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("ExcludeNodes {cn}\n"
                                "ExcludeExitNodes {us} {cn}\n");
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  tt_int_op(smartlist_len(tdata->opt->ExcludeExitNodesUnion_->list), OP_EQ, 2);
  tt_str_op((char *)
            (smartlist_get(tdata->opt->ExcludeExitNodesUnion_->list, 0)),
            OP_EQ, "{us} {cn}");
  tt_str_op((char *)
            (smartlist_get(tdata->opt->ExcludeExitNodesUnion_->list, 1)),
            OP_EQ, "{cn}");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("ExcludeNodes {cn}\n"
                                "StrictNodes 1\n");
  mock_clean_saved_logs();
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  expect_log_msg(
            "You have asked to exclude certain relays from all positions "
            "in your circuits. Expect hidden services and other Tor "
            "features to be broken in unpredictable ways.\n");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("ExcludeNodes {cn}\n");
  mock_clean_saved_logs();
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  expect_no_log_msg(
            "You have asked to exclude certain relays from all positions "
            "in your circuits. Expect hidden services and other Tor "
            "features to be broken in unpredictable ways.\n");
  tor_free(msg);

 done:
  UNMOCK(geoip_get_country);
  teardown_capture_of_logs();
  free_options_test_data(tdata);
  tor_free(msg);
}

static void
test_options_validate__node_families(void *ignored)
{
  (void)ignored;
  int ret;
  char *msg;
  options_test_data_t *tdata = get_options_test_data(
                                     "NodeFamily flux, flax\n"
                                     "NodeFamily somewhere\n");

  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  tt_assert(tdata->opt->NodeFamilySets);
  tt_int_op(smartlist_len(tdata->opt->NodeFamilySets), OP_EQ, 2);
  tt_str_op((char *)(smartlist_get(
    ((routerset_t *)smartlist_get(tdata->opt->NodeFamilySets, 0))->list, 0)),
            OP_EQ, "flux");
  tt_str_op((char *)(smartlist_get(
    ((routerset_t *)smartlist_get(tdata->opt->NodeFamilySets, 0))->list, 1)),
            OP_EQ, "flax");
  tt_str_op((char *)(smartlist_get(
    ((routerset_t *)smartlist_get(tdata->opt->NodeFamilySets, 1))->list, 0)),
            OP_EQ, "somewhere");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("");

  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  tt_assert(!tdata->opt->NodeFamilySets);
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("NodeFamily !flux\n");

  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_assert(tdata->opt->NodeFamilySets);
  tt_int_op(smartlist_len(tdata->opt->NodeFamilySets), OP_EQ, 0);
  tor_free(msg);

 done:
  free_options_test_data(tdata);
  tor_free(msg);
}

static void
test_options_validate__token_bucket(void *ignored)
{
  (void)ignored;
  int ret;
  char *msg;
  options_test_data_t *tdata = get_options_test_data("");

  tdata->opt->TokenBucketRefillInterval = 0;
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ,
            "TokenBucketRefillInterval must be between 1 and 1000 inclusive.");
  tor_free(msg);

  tdata->opt->TokenBucketRefillInterval = 1001;
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ,
            "TokenBucketRefillInterval must be between 1 and 1000 inclusive.");
  tor_free(msg);

 done:
  free_options_test_data(tdata);
  tor_free(msg);
}

static void
test_options_validate__fetch_dir(void *ignored)
{
  (void)ignored;
  int ret;
  char *msg;
  options_test_data_t *tdata = get_options_test_data(
                                            "FetchDirInfoExtraEarly 1\n"
                                            "FetchDirInfoEarly 0\n");

  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ, "FetchDirInfoExtraEarly requires that you"
            " also set FetchDirInfoEarly");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("FetchDirInfoExtraEarly 1\n"
                                "FetchDirInfoEarly 1\n");

  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  tor_free(msg);

 done:
  free_options_test_data(tdata);
  tor_free(msg);
}

static void
test_options_validate__conn_limit(void *ignored)
{
  (void)ignored;
  int ret;
  char *msg;
  options_test_data_t *tdata = get_options_test_data(
                                            "ConnLimit 0\n");

  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ, "ConnLimit must be greater than 0, but was set to 0");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("ConnLimit 1\n");

  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  tor_free(msg);

 done:
  free_options_test_data(tdata);
  tor_free(msg);
}

static void
test_options_validate__paths_needed(void *ignored)
{
  (void)ignored;
  int ret;
  char *msg;

#ifndef _WIN32
  int unset_home_env = 0;
  if (setenv("HOME", "/home/john", 0) == 0)
    unset_home_env = 1;
#endif

  setup_capture_of_logs(LOG_WARN);
  options_test_data_t *tdata = get_options_test_data(
                                      "PathsNeededToBuildCircuits 0.1\n");

  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  tt_assert(tdata->opt->PathsNeededToBuildCircuits > 0.24 &&
            tdata->opt->PathsNeededToBuildCircuits < 0.26);
  expect_log_msg("PathsNeededToBuildCircuits is too low. "
                 "Increasing to 0.25\n");
  tor_free(msg);

  free_options_test_data(tdata);
  mock_clean_saved_logs();
  tdata = get_options_test_data("PathsNeededToBuildCircuits 0.99\n");

  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  tt_assert(tdata->opt->PathsNeededToBuildCircuits > 0.94 &&
            tdata->opt->PathsNeededToBuildCircuits < 0.96);
  expect_log_msg("PathsNeededToBuildCircuits is "
            "too high. Decreasing to 0.95\n");
  tor_free(msg);

  free_options_test_data(tdata);
  mock_clean_saved_logs();
  tdata = get_options_test_data("PathsNeededToBuildCircuits 0.91\n");

  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  tt_assert(tdata->opt->PathsNeededToBuildCircuits > 0.90 &&
            tdata->opt->PathsNeededToBuildCircuits < 0.92);
  expect_no_log_msg_containing("PathsNeededToBuildCircuits");
  tor_free(msg);

 done:
  teardown_capture_of_logs();
  free_options_test_data(tdata);
  tor_free(msg);
#ifndef _WIN32
  if (unset_home_env)
    unsetenv("HOME");
#endif
}

static void
test_options_validate__max_client_circuits(void *ignored)
{
  (void)ignored;
  int ret;
  char *msg;
  options_test_data_t *tdata = get_options_test_data(
                                             "MaxClientCircuitsPending 0\n");

  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ, "MaxClientCircuitsPending must be between 1 and 1024,"
            " but was set to 0");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("MaxClientCircuitsPending 1025\n");

  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ, "MaxClientCircuitsPending must be between 1 and 1024,"
            " but was set to 1025");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("MaxClientCircuitsPending 1\n");

  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  tor_free(msg);

 done:
  free_options_test_data(tdata);
  tor_free(msg);
}

static void
test_options_validate__ports(void *ignored)
{
  (void)ignored;
  int ret;
  char *msg;
  options_test_data_t *tdata = get_options_test_data("FirewallPorts 65537\n");

  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ, "Port '65537' out of range in FirewallPorts");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("FirewallPorts 1\n"
                                "LongLivedPorts 124444\n");

  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ, "Port '124444' out of range in LongLivedPorts");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("FirewallPorts 1\n"
                                "LongLivedPorts 2\n"
                                "RejectPlaintextPorts 112233\n");

  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ, "Port '112233' out of range in RejectPlaintextPorts");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("FirewallPorts 1\n"
                                "LongLivedPorts 2\n"
                                "RejectPlaintextPorts 3\n"
                                "WarnPlaintextPorts 65536\n");

  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ, "Port '65536' out of range in WarnPlaintextPorts");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("FirewallPorts 1\n"
                                "LongLivedPorts 2\n"
                                "RejectPlaintextPorts 3\n"
                                "WarnPlaintextPorts 4\n");

  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  tor_free(msg);

 done:
  free_options_test_data(tdata);
  tor_free(msg);
}

static void
test_options_validate__reachable_addresses(void *ignored)
{
  (void)ignored;
  int ret;
  char *msg;
  setup_capture_of_logs(LOG_NOTICE);
  options_test_data_t *tdata = get_options_test_data("FascistFirewall 1\n");

  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  expect_log_msg("Converting FascistFirewall config "
            "option to new format: \"ReachableDirAddresses *:80\"\n");
  tt_str_op(tdata->opt->ReachableDirAddresses->value, OP_EQ, "*:80");
  expect_log_msg("Converting FascistFirewall config "
            "option to new format: \"ReachableORAddresses *:443\"\n");
  tt_str_op(tdata->opt->ReachableORAddresses->value, OP_EQ, "*:443");
  tor_free(msg);

  free_options_test_data(tdata);
  mock_clean_saved_logs();
  tdata = get_options_test_data("FascistFirewall 1\n"
                                "ReachableDirAddresses *:81\n"
                                "ReachableORAddresses *:444\n");
  tt_assert(tdata->opt->FirewallPorts);
  SMARTLIST_FOREACH(tdata->opt->FirewallPorts, char *, cp, tor_free(cp));
  smartlist_clear(tdata->opt->FirewallPorts);
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
#if 0
  /* This does not actually produce any logs, and did not produce any relevant
   * logs before. */
  expect_log_entry();
#endif
  tt_str_op(tdata->opt->ReachableDirAddresses->value, OP_EQ, "*:81");
  tt_str_op(tdata->opt->ReachableORAddresses->value, OP_EQ, "*:444");
  tor_free(msg);

  free_options_test_data(tdata);
  mock_clean_saved_logs();
  tdata = get_options_test_data("FascistFirewall 1\n"
                                "FirewallPort 123\n");

  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  expect_log_msg("Converting FascistFirewall and "
            "FirewallPorts config options to new format: "
            "\"ReachableAddresses *:123\"\n");
  tt_str_op(tdata->opt->ReachableAddresses->value, OP_EQ, "*:123");
  tor_free(msg);

  free_options_test_data(tdata);
  mock_clean_saved_logs();
  tdata = get_options_test_data("FascistFirewall 1\n"
                                "ReachableAddresses *:82\n"
                                "ReachableAddresses *:83\n"
                                "ReachableAddresses reject *:*\n");

  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
#if 0
  /* This does not actually produce any logs, and did not produce any relevant
   * logs before. */
  expect_log_entry();
#endif
  tt_str_op(tdata->opt->ReachableAddresses->value, OP_EQ, "*:82");
  tor_free(msg);

  free_options_test_data(tdata);
  mock_clean_saved_logs();
  tdata = get_options_test_data("FascistFirewall 1\n"
                                "ReachableAddresses *:82\n");

  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  tt_ptr_op(tdata->opt->ReachableAddresses->next, OP_EQ, NULL);
  tor_free(msg);

#define SERVERS_REACHABLE_MSG "Servers must be able to freely connect to" \
  " the rest of the Internet, so they must not set Reachable*Addresses or" \
  " FascistFirewall or FirewallPorts or ClientUseIPv4 0."

  free_options_test_data(tdata);
  tdata = get_options_test_data("ReachableAddresses *:82\n"
                                "ORPort 127.0.0.1:5555\n");

  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ, SERVERS_REACHABLE_MSG);
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("ReachableORAddresses *:82\n"
                                "ORPort 127.0.0.1:5555\n");

  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ, SERVERS_REACHABLE_MSG);
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("ReachableDirAddresses *:82\n"
                                "ORPort 127.0.0.1:5555\n");

  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ, SERVERS_REACHABLE_MSG);
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("ClientUseIPv4 0\n"
                                "ORPort 127.0.0.1:5555\n");

  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ, SERVERS_REACHABLE_MSG);
  tor_free(msg);

  /* Test IPv4-only clients setting IPv6 preferences */

  free_options_test_data(tdata);
  tdata = get_options_test_data("ClientUseIPv4 1\n"
                                "ClientUseIPv6 0\n"
                                "UseBridges 0\n"
                                "ClientPreferIPv6ORPort 1\n");

  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("ClientUseIPv4 1\n"
                                "ClientUseIPv6 0\n"
                                "UseBridges 0\n"
                                "ClientPreferIPv6DirPort 1\n");

  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  tor_free(msg);

  /* Now test an IPv4/IPv6 client setting IPv6 preferences */

  free_options_test_data(tdata);
  tdata = get_options_test_data("ClientUseIPv4 1\n"
                                "ClientUseIPv6 1\n"
                                "ClientPreferIPv6ORPort 1\n"
                                "ClientPreferIPv6DirPort 1\n");

  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  tt_ptr_op(msg, OP_EQ, NULL);

  /* Now test an IPv6 client setting IPv6 preferences */

  free_options_test_data(tdata);
  tdata = get_options_test_data("ClientUseIPv6 1\n"
                                "ClientPreferIPv6ORPort 1\n"
                                "ClientPreferIPv6DirPort 1\n");

  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  tt_ptr_op(msg, OP_EQ, NULL);

  /* And an implicit (IPv4 disabled) IPv6 client setting IPv6 preferences */

  free_options_test_data(tdata);
  tdata = get_options_test_data("ClientUseIPv4 0\n"
                                "ClientPreferIPv6ORPort 1\n"
                                "ClientPreferIPv6DirPort 1\n");

  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  tt_ptr_op(msg, OP_EQ, NULL);

  /* And an implicit (bridge) client setting IPv6 preferences */

  free_options_test_data(tdata);
  tdata = get_options_test_data("UseBridges 1\n"
                                "Bridge 127.0.0.1:12345\n"
                                "ClientPreferIPv6ORPort 1\n"
                                "ClientPreferIPv6DirPort 1\n");

  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  tt_ptr_op(msg, OP_EQ, NULL);

 done:
  teardown_capture_of_logs();
  free_options_test_data(tdata);
  tor_free(msg);
}

static void
test_options_validate__use_bridges(void *ignored)
{
  (void)ignored;
  int ret;
  char *msg;
  options_test_data_t *tdata = get_options_test_data(
                                   "UseBridges 1\n"
                                   "ClientUseIPv4 1\n"
                                   "ORPort 127.0.0.1:5555\n");

  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ, "Servers must be able to freely connect to the rest of"
            " the Internet, so they must not set UseBridges.");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("UseBridges 1\n");

  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_NE, "Servers must be able to freely connect to the rest of"
            " the Internet, so they must not set UseBridges.");
  tor_free(msg);

  MOCK(geoip_get_country,
       opt_tests_geoip_get_country);
  free_options_test_data(tdata);
  tdata = get_options_test_data("UseBridges 1\n"
                                "EntryNodes {cn}\n");

  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ, "You cannot set both UseBridges and EntryNodes.");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("UseBridges 1\n");

  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ,
            "If you set UseBridges, you must specify at least one bridge.");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("UseBridges 1\n"
                                "Bridge 10.0.0.1\n"
                                "UseEntryGuards 0\n"
                                );

  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ,
            "Setting UseBridges requires also setting UseEntryGuards.");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("UseBridges 1\n"
                                "Bridge 10.0.0.1\n"
                                "Bridge !!!\n"
                                );

  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ, "Bridge line did not parse. See logs for details.");
  tor_free(msg);

 done:
  UNMOCK(geoip_get_country);
  policies_free_all();
  free_options_test_data(tdata);
  tor_free(msg);
}

static void
test_options_validate__entry_nodes(void *ignored)
{
  (void)ignored;
  int ret;
  char *msg;
  MOCK(geoip_get_country,
       opt_tests_geoip_get_country);
  options_test_data_t *tdata = get_options_test_data(
                                         "EntryNodes {cn}\n"
                                         "UseEntryGuards 0\n");

  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ,
            "If EntryNodes is set, UseEntryGuards must be enabled.");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("EntryNodes {cn}\n"
                                "UseEntryGuards 1\n");

  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  tor_free(msg);

 done:
  UNMOCK(geoip_get_country);
  free_options_test_data(tdata);
  tor_free(msg);
}

static void
test_options_validate__safe_logging(void *ignored)
{
  (void)ignored;
  int ret;
  char *msg;
  options_test_data_t *tdata = get_options_test_data("SafeLogging 0\n");

  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  tt_int_op(tdata->opt->SafeLogging_, OP_EQ, SAFELOG_SCRUB_NONE);
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("SafeLogging 0\n");

  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  tt_int_op(tdata->opt->SafeLogging_, OP_EQ, SAFELOG_SCRUB_NONE);
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("SafeLogging Relay\n");

  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  tt_int_op(tdata->opt->SafeLogging_, OP_EQ, SAFELOG_SCRUB_RELAY);
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("SafeLogging 1\n");

  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  tt_int_op(tdata->opt->SafeLogging_, OP_EQ, SAFELOG_SCRUB_ALL);
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("SafeLogging stuffy\n");

  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ, "Unrecognized value '\"stuffy\"' in SafeLogging");
  tor_free(msg);

 done:
  escaped(NULL); // This will free the leaking memory from the previous escaped
  free_options_test_data(tdata);
  tor_free(msg);
}

static void
test_options_validate__publish_server_descriptor(void *ignored)
{
  (void)ignored;
  int ret;
  char *msg;
  setup_capture_of_logs(LOG_WARN);
  options_test_data_t *tdata = get_options_test_data(
             "PublishServerDescriptor bridge\n");

  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  tt_assert(!msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("PublishServerDescriptor humma\n");

  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ, "Unrecognized value in PublishServerDescriptor");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("PublishServerDescriptor bridge, v3\n");

  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ, "Bridges are not supposed to publish router "
            "descriptors to the directory authorities. Please correct your "
            "PublishServerDescriptor line.");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("BridgeRelay 1\n"
                                "PublishServerDescriptor v3\n");

  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ, "Bridges are not supposed to publish router "
            "descriptors to the directory authorities. Please correct your "
            "PublishServerDescriptor line.");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("BridgeRelay 1\n");

  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_NE, "Bridges are not supposed to publish router "
            "descriptors to the directory authorities. Please correct your "
            "PublishServerDescriptor line.");
  tor_free(msg);

 done:
  teardown_capture_of_logs();
  policies_free_all();
  free_options_test_data(tdata);
  tor_free(msg);
}

static void
test_options_validate__testing(void *ignored)
{
  (void)ignored;
  int ret;
  char *msg;
  options_test_data_t *tdata = NULL;

#define ENSURE_DEFAULT(varname, varval)                     \
  STMT_BEGIN                                                \
    free_options_test_data(tdata);                          \
  tdata = get_options_test_data(#varname " " #varval "\n"); \
  ret = options_validate(NULL, tdata->opt, &msg); \
  tt_str_op(msg, OP_EQ, \
            #varname " may only be changed in testing Tor networks!");  \
  tt_int_op(ret, OP_EQ, -1);                                            \
  tor_free(msg);                                                        \
                                                \
  free_options_test_data(tdata); \
  tdata = get_options_test_data(#varname " " #varval "\n"               \
                                VALID_DIR_AUTH                          \
                                "TestingTorNetwork 1\n");               \
                                                                        \
  ret = options_validate(NULL, tdata->opt, &msg);             \
  if (msg) { \
    tt_str_op(msg, OP_NE, \
              #varname " may only be changed in testing Tor networks!"); \
    tor_free(msg); \
  } \
                                                                        \
  free_options_test_data(tdata);          \
  tdata = get_options_test_data(#varname " " #varval "\n"           \
                                "___UsingTestNetworkDefaults 1\n"); \
                                                                        \
  ret = options_validate(NULL, tdata->opt, &msg);\
  if (msg) { \
    tt_str_op(msg, OP_NE, \
              #varname " may only be changed in testing Tor networks!"); \
    tor_free(msg); \
  } \
    STMT_END

  ENSURE_DEFAULT(TestingV3AuthInitialVotingInterval, 3600);
  ENSURE_DEFAULT(TestingV3AuthInitialVoteDelay, 3000);
  ENSURE_DEFAULT(TestingV3AuthInitialDistDelay, 3000);
  ENSURE_DEFAULT(TestingV3AuthVotingStartOffset, 3000);
  ENSURE_DEFAULT(TestingAuthDirTimeToLearnReachability, 3000);
  ENSURE_DEFAULT(TestingServerDownloadInitialDelay, 3000);
  ENSURE_DEFAULT(TestingClientDownloadInitialDelay, 3000);
  ENSURE_DEFAULT(TestingServerConsensusDownloadInitialDelay, 3000);
  ENSURE_DEFAULT(TestingClientConsensusDownloadInitialDelay, 3000);
  ENSURE_DEFAULT(TestingBridgeDownloadInitialDelay, 3000);
  ENSURE_DEFAULT(TestingBridgeBootstrapDownloadInitialDelay, 3000);
  ENSURE_DEFAULT(TestingClientMaxIntervalWithoutRequest, 3000);
  ENSURE_DEFAULT(TestingDirConnectionMaxStall, 3000);
  ENSURE_DEFAULT(TestingAuthKeyLifetime, 3000);
  ENSURE_DEFAULT(TestingLinkCertLifetime, 3000);
  ENSURE_DEFAULT(TestingSigningKeySlop, 3000);
  ENSURE_DEFAULT(TestingAuthKeySlop, 3000);
  ENSURE_DEFAULT(TestingLinkKeySlop, 3000);

 done:
  escaped(NULL); // This will free the leaking memory from the previous escaped
  policies_free_all();
  free_options_test_data(tdata);
  tor_free(msg);
}

static void
test_options_validate__hidserv(void *ignored)
{
  (void)ignored;
  int ret;
  char *msg;
  setup_capture_of_logs(LOG_WARN);

  options_test_data_t *tdata = NULL;

  free_options_test_data(tdata);
  tdata = get_options_test_data("RendPostPeriod 1\n" );
  mock_clean_saved_logs();
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  expect_log_msg("RendPostPeriod option is too short;"
            " raising to 600 seconds.\n");
  tt_int_op(tdata->opt->RendPostPeriod, OP_EQ, 600);
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("RendPostPeriod 302401\n" );
  mock_clean_saved_logs();
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  expect_log_msg("RendPostPeriod is too large; "
            "clipping to 302400s.\n");
  tt_int_op(tdata->opt->RendPostPeriod, OP_EQ, 302400);
  tor_free(msg);

 done:
  teardown_capture_of_logs();
  policies_free_all();
  free_options_test_data(tdata);
  tor_free(msg);
}

static void
test_options_validate__path_bias(void *ignored)
{
  (void)ignored;
  int ret;
  char *msg;

  options_test_data_t *tdata = get_options_test_data(
                                            "PathBiasNoticeRate 1.1\n");
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ,
            "PathBiasNoticeRate is too high. It must be between 0 and 1.0");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("PathBiasWarnRate 1.1\n");
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ,
            "PathBiasWarnRate is too high. It must be between 0 and 1.0");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("PathBiasExtremeRate 1.1\n");
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ,
            "PathBiasExtremeRate is too high. It must be between 0 and 1.0");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("PathBiasNoticeUseRate 1.1\n");
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ,
            "PathBiasNoticeUseRate is too high. It must be between 0 and 1.0");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("PathBiasExtremeUseRate 1.1\n");
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ,
           "PathBiasExtremeUseRate is too high. It must be between 0 and 1.0");
  tor_free(msg);

 done:
  free_options_test_data(tdata);
  tor_free(msg);
}

static void
test_options_validate__bandwidth(void *ignored)
{
  (void)ignored;
  int ret;
  char *msg;
  options_test_data_t *tdata = NULL;

#define ENSURE_BANDWIDTH_PARAM(p, EXTRA_OPT_STR) \
  STMT_BEGIN \
  free_options_test_data(tdata); \
  tdata = get_options_test_data(EXTRA_OPT_STR \
                                #p " 3Gb\n"); \
  ret = options_validate(NULL, tdata->opt, &msg); \
  tt_int_op(ret, OP_EQ, -1); \
  tt_mem_op(msg, OP_EQ, #p " (3221225471) must be at most 2147483647", 40); \
  tor_free(msg); \
  STMT_END

  ENSURE_BANDWIDTH_PARAM(BandwidthRate, "");
  ENSURE_BANDWIDTH_PARAM(BandwidthBurst, "");

  ENSURE_BANDWIDTH_PARAM(BandwidthRate, ENABLE_AUTHORITY_V3);
  ENSURE_BANDWIDTH_PARAM(BandwidthBurst, ENABLE_AUTHORITY_V3);

  ENSURE_BANDWIDTH_PARAM(BandwidthRate, ENABLE_AUTHORITY_BRIDGE);
  ENSURE_BANDWIDTH_PARAM(BandwidthBurst, ENABLE_AUTHORITY_BRIDGE);

  ENSURE_BANDWIDTH_PARAM(MaxAdvertisedBandwidth, "");
  ENSURE_BANDWIDTH_PARAM(RelayBandwidthRate, "");
  ENSURE_BANDWIDTH_PARAM(RelayBandwidthBurst, "");
  ENSURE_BANDWIDTH_PARAM(PerConnBWRate, "");
  ENSURE_BANDWIDTH_PARAM(PerConnBWBurst, "");

  ENSURE_BANDWIDTH_PARAM(MaxAdvertisedBandwidth, ENABLE_AUTHORITY_V3);
  ENSURE_BANDWIDTH_PARAM(RelayBandwidthRate, ENABLE_AUTHORITY_V3);
  ENSURE_BANDWIDTH_PARAM(RelayBandwidthBurst, ENABLE_AUTHORITY_V3);
  ENSURE_BANDWIDTH_PARAM(PerConnBWRate, ENABLE_AUTHORITY_V3);
  ENSURE_BANDWIDTH_PARAM(PerConnBWBurst, ENABLE_AUTHORITY_V3);

  ENSURE_BANDWIDTH_PARAM(MaxAdvertisedBandwidth, ENABLE_AUTHORITY_BRIDGE);
  ENSURE_BANDWIDTH_PARAM(RelayBandwidthRate, ENABLE_AUTHORITY_BRIDGE);
  ENSURE_BANDWIDTH_PARAM(RelayBandwidthBurst, ENABLE_AUTHORITY_BRIDGE);
  ENSURE_BANDWIDTH_PARAM(PerConnBWRate, ENABLE_AUTHORITY_BRIDGE);
  ENSURE_BANDWIDTH_PARAM(PerConnBWBurst, ENABLE_AUTHORITY_BRIDGE);

  ENSURE_BANDWIDTH_PARAM(AuthDirFastGuarantee, ENABLE_AUTHORITY_V3);
  ENSURE_BANDWIDTH_PARAM(AuthDirGuardBWGuarantee, ENABLE_AUTHORITY_V3);

  free_options_test_data(tdata);
  tdata = get_options_test_data("RelayBandwidthRate 1000\n");
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  tt_u64_op(tdata->opt->RelayBandwidthBurst, OP_EQ, 1000);
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("RelayBandwidthBurst 1001\n");
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  tt_u64_op(tdata->opt->RelayBandwidthRate, OP_EQ, 1001);
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("RelayBandwidthRate 1001\n"
                                "RelayBandwidthBurst 1000\n");
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ, "RelayBandwidthBurst must be at least equal to "
            "RelayBandwidthRate.");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("BandwidthRate 1001\n"
                                "BandwidthBurst 1000\n");
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ,
            "BandwidthBurst must be at least equal to BandwidthRate.");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("RelayBandwidthRate 1001\n"
                                "BandwidthRate 1000\n"
                                "BandwidthBurst 1000\n"
                                );
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  tt_u64_op(tdata->opt->BandwidthRate, OP_EQ, 1001);
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("RelayBandwidthRate 1001\n"
                                "BandwidthRate 1000\n"
                                "RelayBandwidthBurst 1001\n"
                                "BandwidthBurst 1000\n"
                                );
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  tt_u64_op(tdata->opt->BandwidthBurst, OP_EQ, 1001);
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("ORPort 127.0.0.1:5555\n"
                                "BandwidthRate 1\n"
                                );
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ, "BandwidthRate is set to 1 bytes/second. For servers,"
            " it must be at least 76800.");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("ORPort 127.0.0.1:5555\n"
                                "BandwidthRate 76800\n"
                                "MaxAdvertisedBandwidth 30000\n"
                                );
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ, "MaxAdvertisedBandwidth is set to 30000 bytes/second."
            " For servers, it must be at least 38400.");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("ORPort 127.0.0.1:5555\n"
                                "BandwidthRate 76800\n"
                                "RelayBandwidthRate 1\n"
                                "MaxAdvertisedBandwidth 38400\n"
                                );
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ, "RelayBandwidthRate is set to 1 bytes/second. For "
            "servers, it must be at least 76800.");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("ORPort 127.0.0.1:5555\n"
                                "BandwidthRate 76800\n"
                                "BandwidthBurst 76800\n"
                                "RelayBandwidthRate 76800\n"
                                "MaxAdvertisedBandwidth 38400\n"
                                );
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  tor_free(msg);

 done:
  policies_free_all();
  free_options_test_data(tdata);
  tor_free(msg);
}

static void
test_options_validate__circuits(void *ignored)
{
  (void)ignored;
  char *msg;
  options_test_data_t *tdata = NULL;
  setup_capture_of_logs(LOG_WARN);

  free_options_test_data(tdata);
  tdata = get_options_test_data("MaxCircuitDirtiness 2592001\n");
  options_validate(NULL, tdata->opt, &msg);
  expect_log_msg("MaxCircuitDirtiness option is too "
            "high; setting to 30 days.\n");
  tt_int_op(tdata->opt->MaxCircuitDirtiness, OP_EQ, 2592000);
  tor_free(msg);

  free_options_test_data(tdata);
  mock_clean_saved_logs();
  tdata = get_options_test_data("CircuitStreamTimeout 1\n");
  options_validate(NULL, tdata->opt, &msg);
  expect_log_msg("CircuitStreamTimeout option is too"
            " short; raising to 10 seconds.\n");
  tt_int_op(tdata->opt->CircuitStreamTimeout, OP_EQ, 10);
  tor_free(msg);

  free_options_test_data(tdata);
  mock_clean_saved_logs();
  tdata = get_options_test_data("CircuitStreamTimeout 111\n");
  options_validate(NULL, tdata->opt, &msg);
  expect_no_log_msg("CircuitStreamTimeout option is too"
            " short; raising to 10 seconds.\n");
  tt_int_op(tdata->opt->CircuitStreamTimeout, OP_EQ, 111);
  tor_free(msg);

  free_options_test_data(tdata);
  mock_clean_saved_logs();
  tdata = get_options_test_data("HeartbeatPeriod 1\n");
  options_validate(NULL, tdata->opt, &msg);
  expect_log_msg("HeartbeatPeriod option is too short;"
            " raising to 1800 seconds.\n");
  tt_int_op(tdata->opt->HeartbeatPeriod, OP_EQ, 1800);
  tor_free(msg);

  free_options_test_data(tdata);
  mock_clean_saved_logs();
  tdata = get_options_test_data("HeartbeatPeriod 1982\n");
  options_validate(NULL, tdata->opt, &msg);
  expect_no_log_msg("HeartbeatPeriod option is too short;"
            " raising to 1800 seconds.\n");
  tt_int_op(tdata->opt->HeartbeatPeriod, OP_EQ, 1982);
  tor_free(msg);

  free_options_test_data(tdata);
  mock_clean_saved_logs();
  tdata = get_options_test_data("LearnCircuitBuildTimeout 0\n"
                                "CircuitBuildTimeout 1\n"
                                );
  options_validate(NULL, tdata->opt, &msg);
  expect_log_msg("CircuitBuildTimeout is shorter (1"
            " seconds) than the recommended minimum (10 seconds), and "
            "LearnCircuitBuildTimeout is disabled.  If tor isn't working, "
            "raise this value or enable LearnCircuitBuildTimeout.\n");
  tor_free(msg);

  free_options_test_data(tdata);
  mock_clean_saved_logs();
  tdata = get_options_test_data("CircuitBuildTimeout 11\n"
                                );
  options_validate(NULL, tdata->opt, &msg);
  expect_no_log_msg("CircuitBuildTimeout is shorter (1 "
            "seconds) than the recommended minimum (10 seconds), and "
            "LearnCircuitBuildTimeout is disabled.  If tor isn't working, "
            "raise this value or enable LearnCircuitBuildTimeout.\n");
  tor_free(msg);

 done:
  policies_free_all();
  teardown_capture_of_logs();
  free_options_test_data(tdata);
  tor_free(msg);
}

static void
test_options_validate__rend(void *ignored)
{
  (void)ignored;
  int ret;
  char *msg;
  options_test_data_t *tdata = NULL;
  setup_capture_of_logs(LOG_WARN);

  free_options_test_data(tdata);
  tdata = get_options_test_data(
                 "UseEntryGuards 0\n"
                 "HiddenServiceDir /Library/Tor/var/lib/tor/hidden_service/\n"
                 "HiddenServicePort 80 127.0.0.1:8080\n"
                                );
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  expect_log_msg("UseEntryGuards is disabled, but you"
            " have configured one or more hidden services on this Tor "
            "instance.  Your hidden services will be very easy to locate using"
            " a well-known attack -- see https://freehaven.net/anonbib/#hs-"
            "attack06 for details.\n");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data(
            "UseEntryGuards 1\n"
            "HiddenServiceDir /Library/Tor/var/lib/tor/hidden_service/\n"
            "HiddenServicePort 80 127.0.0.1:8080\n"
                                );
  mock_clean_saved_logs();
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  expect_no_log_msg("UseEntryGuards is disabled, but you"
            " have configured one or more hidden services on this Tor "
            "instance.  Your hidden services will be very easy to locate using"
            " a well-known attack -- see https://freehaven.net/anonbib/#hs-"
            "attack06 for details.\n");

  free_options_test_data(tdata);
  tdata = get_options_test_data("HiddenServicePort 80 127.0.0.1:8080\n");
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ,
            "Failed to configure rendezvous options. See logs for details.");
  tor_free(msg);

 done:
  policies_free_all();
  teardown_capture_of_logs();
  free_options_test_data(tdata);
  tor_free(msg);
}

static void
test_options_validate__single_onion(void *ignored)
{
  (void)ignored;
  int ret;
  char *msg;
  options_test_data_t *tdata = NULL;
  setup_capture_of_logs(LOG_WARN);

  /* Test that HiddenServiceSingleHopMode must come with
   * HiddenServiceNonAnonymousMode */
  tdata = get_options_test_data("SOCKSPort 0\n"
                                "HiddenServiceSingleHopMode 1\n"
                                );
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ, "HiddenServiceSingleHopMode does not provide any "
            "server anonymity. It must be used with "
            "HiddenServiceNonAnonymousMode set to 1.");
  tor_free(msg);
  free_options_test_data(tdata);

  tdata = get_options_test_data("SOCKSPort 0\n"
                                "HiddenServiceSingleHopMode 1\n"
                                "HiddenServiceNonAnonymousMode 0\n"
                                );
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ, "HiddenServiceSingleHopMode does not provide any "
            "server anonymity. It must be used with "
            "HiddenServiceNonAnonymousMode set to 1.");
  tor_free(msg);
  free_options_test_data(tdata);

  tdata = get_options_test_data("SOCKSPort 0\n"
                                "HiddenServiceSingleHopMode 1\n"
                                "HiddenServiceNonAnonymousMode 1\n"
                                );
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  tt_ptr_op(msg, OP_EQ, NULL);
  free_options_test_data(tdata);

  /* Test that SOCKSPort if HiddenServiceSingleHopMode is 1 */
  tdata = get_options_test_data("SOCKSPort 5000\n"
                                "HiddenServiceSingleHopMode 1\n"
                                "HiddenServiceNonAnonymousMode 1\n"
                                );
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ, "HiddenServiceNonAnonymousMode is incompatible with "
            "using Tor as an anonymous client. Please set "
            "Socks/Trans/NATD/DNSPort to 0, or revert "
            "HiddenServiceNonAnonymousMode to 0.");
  tor_free(msg);
  free_options_test_data(tdata);

  tdata = get_options_test_data("SOCKSPort 0\n"
                                "HiddenServiceSingleHopMode 1\n"
                                "HiddenServiceNonAnonymousMode 1\n"
                                );
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  tt_ptr_op(msg, OP_EQ, NULL);
  free_options_test_data(tdata);

  tdata = get_options_test_data("SOCKSPort 5000\n"
                                "HiddenServiceSingleHopMode 0\n"
                                );
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  tt_ptr_op(msg, OP_EQ, NULL);
  free_options_test_data(tdata);

  /* Test that a hidden service can't be run in non anonymous mode. */
  tdata = get_options_test_data(
                  "HiddenServiceNonAnonymousMode 1\n"
                  "HiddenServiceDir /Library/Tor/var/lib/tor/hidden_service/\n"
                  "HiddenServicePort 80 127.0.0.1:8080\n"
                                );
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ, "HiddenServiceNonAnonymousMode does not provide any "
            "server anonymity. It must be used with "
            "HiddenServiceSingleHopMode set to 1.");
  tor_free(msg);
  free_options_test_data(tdata);

  tdata = get_options_test_data(
                  "HiddenServiceNonAnonymousMode 1\n"
                                );
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ, "HiddenServiceNonAnonymousMode does not provide any "
            "server anonymity. It must be used with "
            "HiddenServiceSingleHopMode set to 1.");
  tor_free(msg);
  free_options_test_data(tdata);

  tdata = get_options_test_data(
                  "HiddenServiceDir /Library/Tor/var/lib/tor/hidden_service/\n"
                  "HiddenServicePort 80 127.0.0.1:8080\n"
                                );
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  tt_ptr_op(msg, OP_EQ, NULL);
  free_options_test_data(tdata);

  tdata = get_options_test_data(
                  "HiddenServiceNonAnonymousMode 1\n"
                  "HiddenServiceDir /Library/Tor/var/lib/tor/hidden_service/\n"
                  "HiddenServicePort 80 127.0.0.1:8080\n"
                  "HiddenServiceSingleHopMode 1\n"
                  "SOCKSPort 0\n"
                                );
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  tt_ptr_op(msg, OP_EQ, NULL);

 done:
  policies_free_all();
  teardown_capture_of_logs();
  free_options_test_data(tdata);
  tor_free(msg);
}

static void
test_options_validate__accounting(void *ignored)
{
  (void)ignored;
  int ret;
  char *msg;
  options_test_data_t *tdata = NULL;
  setup_capture_of_logs(LOG_WARN);

  free_options_test_data(tdata);
  tdata = get_options_test_data("AccountingRule something_bad\n");
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ, "AccountingRule must be 'sum', 'max', 'in', or 'out'");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("AccountingRule sum\n");
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  tt_int_op(tdata->opt->AccountingRule, OP_EQ, ACCT_SUM);
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("AccountingRule max\n");
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  tt_int_op(tdata->opt->AccountingRule, OP_EQ, ACCT_MAX);
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("AccountingRule in\n");
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  tt_int_op(tdata->opt->AccountingRule, OP_EQ, ACCT_IN);
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("AccountingRule out\n");
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  tt_int_op(tdata->opt->AccountingRule, OP_EQ, ACCT_OUT);
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("AccountingStart fail\n");
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ,
            "Failed to parse accounting options. See logs for details.");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("AccountingMax 10\n");
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data(
           "ORPort 127.0.0.1:5555\n"
           "BandwidthRate 76800\n"
           "BandwidthBurst 76800\n"
           "MaxAdvertisedBandwidth 38400\n"
           "HiddenServiceDir /Library/Tor/var/lib/tor/hidden_service/\n"
           "HiddenServicePort 80 127.0.0.1:8080\n"
           "AccountingMax 10\n"
                                );
  mock_clean_saved_logs();
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  expect_log_msg("Using accounting with a hidden "
            "service and an ORPort is risky: your hidden service(s) and "
            "your public address will all turn off at the same time, "
            "which may alert observers that they are being run by the "
            "same party.\n");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data(
                "HiddenServiceDir /Library/Tor/var/lib/tor/hidden_service/\n"
                "HiddenServicePort 80 127.0.0.1:8080\n"
                "AccountingMax 10\n"
                                );
  mock_clean_saved_logs();
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  expect_no_log_msg("Using accounting with a hidden "
            "service and an ORPort is risky: your hidden service(s) and "
            "your public address will all turn off at the same time, "
            "which may alert observers that they are being run by the "
            "same party.\n");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data(
             "HiddenServiceDir /Library/Tor/var/lib/tor/hidden_service/\n"
             "HiddenServicePort 80 127.0.0.1:8080\n"
             "HiddenServiceDir /Library/Tor/var/lib/tor/hidden_service2/\n"
             "HiddenServicePort 81 127.0.0.1:8081\n"
             "AccountingMax 10\n"
                                );
  mock_clean_saved_logs();
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  expect_log_msg("Using accounting with multiple "
            "hidden services is risky: they will all turn off at the same"
            " time, which may alert observers that they are being run by "
            "the same party.\n");
  tor_free(msg);

 done:
  teardown_capture_of_logs();
  policies_free_all();
  free_options_test_data(tdata);
  tor_free(msg);
}

static void
test_options_validate__proxy(void *ignored)
{
  (void)ignored;
  int ret;
  char *msg;
  options_test_data_t *tdata = NULL;
  sandbox_disable_getaddrinfo_cache();
  setup_capture_of_logs(LOG_WARN);
  MOCK(tor_addr_lookup, mock_tor_addr_lookup__fail_on_bad_addrs);

  free_options_test_data(tdata);
  tdata = get_options_test_data("HttpProxy 127.0.42.1\n");
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  tt_int_op(tdata->opt->HTTPProxyPort, OP_EQ, 80);
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("HttpProxy 127.0.42.1:444\n");
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  tt_int_op(tdata->opt->HTTPProxyPort, OP_EQ, 444);
  tor_free(msg);

  free_options_test_data(tdata);

  tdata = get_options_test_data("HttpProxy not_so_valid!\n");
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ, "HTTPProxy failed to parse or resolve. Please fix.");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("HttpProxyAuthenticator "
                                "onetwothreonetwothreonetwothreonetwothreonetw"
                                "othreonetwothreonetwothreonetwothreonetwothre"
                                "onetwothreonetwothreonetwothreonetwothreonetw"
                                "othreonetwothreonetwothreonetwothreonetwothre"
                                "onetwothreonetwothreonetwothreonetwothreonetw"
                                "othreonetwothreonetwothreonetwothreonetwothre"
                                "onetwothreonetwothreonetwothreonetwothreonetw"
                                "othreonetwothreonetwothreonetwothreonetwothre"
                                "onetwothreonetwothreonetwothreonetwothreonetw"
                                "othreonetwothreonetwothreonetwothreonetwothre"
                                "onetwothreonetwothreonetwothreonetwothreonetw"
                                "othreonetwothreeonetwothreeonetwothree"
                                );
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ, "HTTPProxyAuthenticator is too long (>= 512 chars).");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("HttpProxyAuthenticator validauth\n");
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("HttpsProxy 127.0.42.1\n");
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  tt_int_op(tdata->opt->HTTPSProxyPort, OP_EQ, 443);
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("HttpsProxy 127.0.42.1:444\n");
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  tt_int_op(tdata->opt->HTTPSProxyPort, OP_EQ, 444);
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("HttpsProxy not_so_valid!\n");
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ, "HTTPSProxy failed to parse or resolve. Please fix.");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("HttpsProxyAuthenticator "
                                "onetwothreonetwothreonetwothreonetwothreonetw"
                                "othreonetwothreonetwothreonetwothreonetwothre"
                                "onetwothreonetwothreonetwothreonetwothreonetw"
                                "othreonetwothreonetwothreonetwothreonetwothre"
                                "onetwothreonetwothreonetwothreonetwothreonetw"
                                "othreonetwothreonetwothreonetwothreonetwothre"
                                "onetwothreonetwothreonetwothreonetwothreonetw"
                                "othreonetwothreonetwothreonetwothreonetwothre"
                                "onetwothreonetwothreonetwothreonetwothreonetw"
                                "othreonetwothreonetwothreonetwothreonetwothre"
                                "onetwothreonetwothreonetwothreonetwothreonetw"
                                "othreonetwothreeonetwothreeonetwothree"

                                );
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ, "HTTPSProxyAuthenticator is too long (>= 512 chars).");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("HttpsProxyAuthenticator validauth\n");
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("Socks4Proxy 127.0.42.1\n");
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  tt_int_op(tdata->opt->Socks4ProxyPort, OP_EQ, 1080);
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("Socks4Proxy 127.0.42.1:444\n");
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  tt_int_op(tdata->opt->Socks4ProxyPort, OP_EQ, 444);
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("Socks4Proxy not_so_valid!\n");
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ, "Socks4Proxy failed to parse or resolve. Please fix.");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("Socks5Proxy 127.0.42.1\n");
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  tt_int_op(tdata->opt->Socks5ProxyPort, OP_EQ, 1080);
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("Socks5Proxy 127.0.42.1:444\n");
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  tt_int_op(tdata->opt->Socks5ProxyPort, OP_EQ, 444);
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("Socks5Proxy not_so_valid!\n");
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ, "Socks5Proxy failed to parse or resolve. Please fix.");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("Socks4Proxy 215.1.1.1\n"
                                "Socks5Proxy 215.1.1.2\n"
                                );
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ, "You have configured more than one proxy type. "
            "(Socks4Proxy|Socks5Proxy|HTTPSProxy|TCPProxy)");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("HttpProxy 215.1.1.1\n");
  mock_clean_saved_logs();
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  expect_log_msg("HTTPProxy configured, but no SOCKS proxy, "
            "HTTPS proxy, or any other TCP proxy configured. Watch out: "
            "this configuration will proxy unencrypted directory "
            "connections only.\n");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("HttpProxy 215.1.1.1\n"
                                "Socks4Proxy 215.1.1.1\n"
                                );
  mock_clean_saved_logs();
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  expect_no_log_msg("HTTPProxy configured, but no SOCKS "
            "proxy or HTTPS proxy configured. Watch out: this configuration "
            "will proxy unencrypted directory connections only.\n");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("HttpProxy 215.1.1.1\n"
                                "Socks5Proxy 215.1.1.1\n"
                                );
  mock_clean_saved_logs();
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  expect_no_log_msg("HTTPProxy configured, but no SOCKS "
            "proxy or HTTPS proxy configured. Watch out: this configuration "
            "will proxy unencrypted directory connections only.\n");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("HttpProxy 215.1.1.1\n"
                                "HttpsProxy 215.1.1.1\n"
                                );
  mock_clean_saved_logs();
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  expect_no_log_msg(
            "HTTPProxy configured, but no SOCKS proxy or HTTPS proxy "
            "configured. Watch out: this configuration will proxy "
            "unencrypted directory connections only.\n");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("");
  tdata->opt->Socks5ProxyUsername = tor_strdup("");
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ,
            "Socks5ProxyUsername must be between 1 and 255 characters.");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("");
  tdata->opt->Socks5ProxyUsername =
    tor_strdup("ABCDEABCDE0123456789ABCDEABCDE0123456789ABCDEABCDE0123456789AB"
               "CDEABCDE0123456789ABCDEABCDE0123456789ABCDEABCDE0123456789ABCD"
               "EABCDE0123456789ABCDEABCDE0123456789ABCDEABCDE0123456789ABCDEA"
               "BCDE0123456789ABCDEABCDE0123456789ABCDEABCDE0123456789ABCDEABC"
               "DE0123456789ABCDEABCDE0123456789ABCDEABCDE0123456789");
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ,
            "Socks5ProxyUsername must be between 1 and 255 characters.");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("Socks5ProxyUsername hello_world\n");
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ, "Socks5ProxyPassword must be included with "
            "Socks5ProxyUsername.");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("Socks5ProxyUsername hello_world\n");
  tdata->opt->Socks5ProxyPassword = tor_strdup("");
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ,
            "Socks5ProxyPassword must be between 1 and 255 characters.");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("Socks5ProxyUsername hello_world\n");
  tdata->opt->Socks5ProxyPassword =
    tor_strdup("ABCDEABCDE0123456789ABCDEABCDE0123456789ABCDEABCDE0123456789AB"
               "CDEABCDE0123456789ABCDEABCDE0123456789ABCDEABCDE0123456789ABCD"
               "EABCDE0123456789ABCDEABCDE0123456789ABCDEABCDE0123456789ABCDEA"
               "BCDE0123456789ABCDEABCDE0123456789ABCDEABCDE0123456789ABCDEABC"
               "DE0123456789ABCDEABCDE0123456789ABCDEABCDE0123456789");
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ,
            "Socks5ProxyPassword must be between 1 and 255 characters.");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("Socks5ProxyUsername hello_world\n"
                                "Socks5ProxyPassword world_hello\n");
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("Socks5ProxyPassword hello_world\n");
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ, "Socks5ProxyPassword must be included with "
            "Socks5ProxyUsername.");
  tor_free(msg);

 done:
  teardown_capture_of_logs();
  free_options_test_data(tdata);
  policies_free_all();
  // sandbox_free_getaddrinfo_cache();
  tor_free(msg);
  UNMOCK(tor_addr_lookup);
}

static void
test_options_validate__control(void *ignored)
{
  (void)ignored;
  int ret;
  char *msg;
  options_test_data_t *tdata = NULL;
  setup_capture_of_logs(LOG_WARN);

  free_options_test_data(tdata);
  tdata = get_options_test_data(
                         "HashedControlPassword something_incorrect\n");
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ,
            "Bad HashedControlPassword: wrong length or bad encoding");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("HashedControlPassword 16:872860B76453A77D60CA"
                                "2BB8C1A7042072093276A3D701AD684053EC4C\n"
                                );
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data(
                   "__HashedControlSessionPassword something_incorrect\n"
                                );
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ, "Bad HashedControlSessionPassword: wrong length or "
            "bad encoding");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("__HashedControlSessionPassword 16:872860B7645"
                                "3A77D60CA2BB8C1A7042072093276A3D701AD684053EC"
                                "4C\n"
                                );
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data(
                           "__OwningControllerProcess something_incorrect\n"
                                );
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ, "Bad OwningControllerProcess: invalid PID");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("__OwningControllerProcess 123\n"
                                );
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("ControlPort 127.0.0.1:1234\n"
                                );
  mock_clean_saved_logs();
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  expect_log_msg(
            "ControlPort is open, but no authentication method has been "
            "configured.  This means that any program on your computer can "
            "reconfigure your Tor.  That's bad!  You should upgrade your Tor"
            " controller as soon as possible.\n");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("ControlPort 127.0.0.1:1234\n"
                                "HashedControlPassword 16:872860B76453A77D60CA"
                                "2BB8C1A7042072093276A3D701AD684053EC4C\n"
                                );
  mock_clean_saved_logs();
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  expect_no_log_msg(
            "ControlPort is open, but no authentication method has been "
            "configured.  This means that any program on your computer can "
            "reconfigure your Tor.  That's bad!  You should upgrade your Tor "
            "controller as soon as possible.\n");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("ControlPort 127.0.0.1:1234\n"
                                "__HashedControlSessionPassword 16:872860B7645"
                                "3A77D60CA2BB8C1A7042072093276A3D701AD684053EC"
                                "4C\n"
                                );
  mock_clean_saved_logs();
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  expect_no_log_msg(
            "ControlPort is open, but no authentication method has been "
            "configured.  This means that any program on your computer can "
            "reconfigure your Tor.  That's bad!  You should upgrade your Tor "
            "controller as soon as possible.\n");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("ControlPort 127.0.0.1:1234\n"
                                "CookieAuthentication 1\n"
                                );
  mock_clean_saved_logs();
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  expect_no_log_msg(
            "ControlPort is open, but no authentication method has been "
            "configured.  This means that any program on your computer can "
            "reconfigure your Tor.  That's bad!  You should upgrade your Tor "
            "controller as soon as possible.\n");
  tor_free(msg);

#ifdef HAVE_SYS_UN_H
  free_options_test_data(tdata);
  tdata = get_options_test_data("ControlSocket unix:/tmp WorldWritable\n");
  mock_clean_saved_logs();
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  expect_log_msg(
            "ControlSocket is world writable, but no authentication method has"
            " been configured.  This means that any program on your computer "
            "can reconfigure your Tor.  That's bad!  You should upgrade your "
            "Tor controller as soon as possible.\n");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("ControlSocket unix:/tmp WorldWritable\n"
                                "HashedControlPassword 16:872860B76453A77D60CA"
                                "2BB8C1A7042072093276A3D701AD684053EC4C\n"
                                );
  mock_clean_saved_logs();
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  expect_no_log_msg(
            "ControlSocket is world writable, but no authentication method has"
            " been configured.  This means that any program on your computer "
            "can reconfigure your Tor.  That's bad!  You should upgrade your "
            "Tor controller as soon as possible.\n");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("ControlSocket unix:/tmp WorldWritable\n"
                                "__HashedControlSessionPassword 16:872860B7645"
                                "3A77D60CA2BB8C1A7042072093276A3D701AD684053EC"
                                "4C\n"
                                );
  mock_clean_saved_logs();
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  expect_no_log_msg(
            "ControlSocket is world writable, but no authentication method has"
            " been configured.  This means that any program on your computer "
            "can reconfigure your Tor.  That's bad!  You should upgrade your "
            "Tor controller as soon as possible.\n");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("ControlSocket unix:/tmp WorldWritable\n"
                                "CookieAuthentication 1\n"
                                );
  mock_clean_saved_logs();
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  expect_no_log_msg(
            "ControlSocket is world writable, but no authentication method has"
            " been configured.  This means that any program on your computer "
            "can reconfigure your Tor.  That's bad!  You should upgrade your "
            "Tor controller as soon as possible.\n");
  tor_free(msg);
#endif /* defined(HAVE_SYS_UN_H) */

  free_options_test_data(tdata);
  tdata = get_options_test_data("CookieAuthFileGroupReadable 1\n"
                                );
  mock_clean_saved_logs();
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  expect_log_msg(
            "CookieAuthFileGroupReadable is set, but will have no effect: you "
            "must specify an explicit CookieAuthFile to have it "
            "group-readable.\n");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("CookieAuthFileGroupReadable 1\n"
                                "CookieAuthFile /tmp/somewhere\n"
                                );
  mock_clean_saved_logs();
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  expect_no_log_msg(
            "CookieAuthFileGroupReadable is set, but will have no effect: you "
            "must specify an explicit CookieAuthFile to have it "
            "group-readable.\n");
  tor_free(msg);

 done:
  teardown_capture_of_logs();
  policies_free_all();
  free_options_test_data(tdata);
  tor_free(msg);
}

static void
test_options_validate__families(void *ignored)
{
  (void)ignored;
  int ret;
  char *msg;
  options_test_data_t *tdata = NULL;
  setup_capture_of_logs(LOG_WARN);

  free_options_test_data(tdata);
  tdata = get_options_test_data("MyFamily home\n"
                                "BridgeRelay 1\n"
                                "ORPort 127.0.0.1:5555\n"
                                "BandwidthRate 51300\n"
                                "BandwidthBurst 51300\n"
                                "MaxAdvertisedBandwidth 25700\n"
                                "DirCache 1\n"
                                );
  mock_clean_saved_logs();
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  expect_log_msg(
            "Listing a family for a bridge relay is not supported: it can "
            "reveal bridge fingerprints to censors. You should also make sure "
            "you aren't listing this bridge's fingerprint in any other "
            "MyFamily.\n");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("MyFamily home\n");
  mock_clean_saved_logs();
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  expect_no_log_msg(
            "Listing a family for a bridge relay is not supported: it can "
            "reveal bridge fingerprints to censors. You should also make sure "
            "you aren't listing this bridge's fingerprint in any other "
            "MyFamily.\n");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("MyFamily !\n");
  mock_clean_saved_logs();
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ, "Invalid nickname '!' in MyFamily line");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("NodeFamily foo\n"
                                "NodeFamily !\n");
  mock_clean_saved_logs();
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_assert(!msg);
  tor_free(msg);

 done:
  teardown_capture_of_logs();
  policies_free_all();
  free_options_test_data(tdata);
  tor_free(msg);
}

static void
test_options_validate__addr_policies(void *ignored)
{
  (void)ignored;
  int ret;
  char *msg;
  options_test_data_t *tdata = NULL;

  free_options_test_data(tdata);
  tdata = get_options_test_data("ExitPolicy !!!\n"
                                "ExitRelay 1\n"
                                );
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ, "Error in ExitPolicy entry.");
  tor_free(msg);

 done:
  policies_free_all();
  free_options_test_data(tdata);
  tor_free(msg);
}

static void
test_options_validate__dir_auth(void *ignored)
{
  (void)ignored;
  int ret;
  char *msg;
  options_test_data_t *tdata = NULL;
  setup_capture_of_logs(LOG_WARN);

  free_options_test_data(tdata);
  tdata = get_options_test_data(VALID_DIR_AUTH
                                VALID_ALT_DIR_AUTH
                                );
  mock_clean_saved_logs();
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ,
            "Directory authority/fallback line did not parse. See logs for "
            "details.");
  expect_log_msg(
            "You cannot set both DirAuthority and Alternate*Authority.\n");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("TestingTorNetwork 1\n");
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ,
            "TestingTorNetwork may only be configured in combination with a "
            "non-default set of DirAuthority or both of AlternateDirAuthority "
            "and AlternateBridgeAuthority configured.");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data(VALID_DIR_AUTH
                                "TestingTorNetwork 1\n"
                                );
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("TestingTorNetwork 1\n"
                                VALID_ALT_DIR_AUTH
                                );
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ,
            "TestingTorNetwork may only be configured in combination with a "
            "non-default set of DirAuthority or both of AlternateDirAuthority "
            "and AlternateBridgeAuthority configured.");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("TestingTorNetwork 1\n"
                                VALID_ALT_BRIDGE_AUTH
                                );
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ, "TestingTorNetwork may only be configured in "
            "combination with a non-default set of DirAuthority or both of "
            "AlternateDirAuthority and AlternateBridgeAuthority configured.");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data(VALID_ALT_DIR_AUTH
                                VALID_ALT_BRIDGE_AUTH
                                "TestingTorNetwork 1\n"
                                );
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  tor_free(msg);

 done:
  policies_free_all();
  teardown_capture_of_logs();
  free_options_test_data(tdata);
  tor_free(msg);
}

static void
test_options_validate__transport(void *ignored)
{
  (void)ignored;
  int ret;
  char *msg;
  options_test_data_t *tdata = NULL;
  setup_capture_of_logs(LOG_NOTICE);

  free_options_test_data(tdata);
  tdata = get_options_test_data("ClientTransportPlugin !!\n"
                                );
  mock_clean_saved_logs();
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ,
            "Invalid client transport line. See logs for details.");
  expect_log_msg(
            "Too few arguments on ClientTransportPlugin line.\n");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("ClientTransportPlugin foo exec bar\n"
                                );
  mock_clean_saved_logs();
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("ServerTransportPlugin !!\n");
  mock_clean_saved_logs();
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ,
            "Invalid server transport line. See logs for details.");
  expect_log_msg(
            "Too few arguments on ServerTransportPlugin line.\n");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("ServerTransportPlugin foo exec bar\n");
  mock_clean_saved_logs();
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  expect_log_msg(
            "Tor is not configured as a relay but you specified a "
            "ServerTransportPlugin line (\"foo exec bar\"). The "
            "ServerTransportPlugin line will be ignored.\n");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("ServerTransportPlugin foo exec bar\n"
                                "ORPort 127.0.0.1:5555\n"
                                "BandwidthRate 76900\n"
                                "BandwidthBurst 76900\n"
                                "MaxAdvertisedBandwidth 38500\n"
                                );
  mock_clean_saved_logs();
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  expect_no_log_msg(
            "Tor is not configured as a relay but you specified a "
            "ServerTransportPlugin line (\"foo exec bar\"). The "
            "ServerTransportPlugin line will be ignored.\n");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("ServerTransportListenAddr foo 127.0.0.42:55\n"
                                "ServerTransportListenAddr !\n");
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ,
            "ServerTransportListenAddr did not parse. See logs for details.");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("ServerTransportListenAddr foo 127.0.0.42:55\n"
                                );
  mock_clean_saved_logs();
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  expect_log_msg(
            "You need at least a single managed-proxy to specify a transport "
            "listen address. The ServerTransportListenAddr line will be "
            "ignored.\n");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("ServerTransportListenAddr foo 127.0.0.42:55\n"
                                "ServerTransportPlugin foo exec bar\n"
                                "ORPort 127.0.0.1:5555\n"
                                "BandwidthRate 76900\n"
                                "BandwidthBurst 76900\n"
                                "MaxAdvertisedBandwidth 38500\n"
                                );
  mock_clean_saved_logs();
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  expect_no_log_msg(
            "You need at least a single managed-proxy to specify a transport "
            "listen address. The ServerTransportListenAddr line will be "
            "ignored.\n");

 done:
  escaped(NULL); // This will free the leaking memory from the previous escaped
  policies_free_all();
  teardown_capture_of_logs();
  free_options_test_data(tdata);
  tor_free(msg);
}

static void
test_options_validate__constrained_sockets(void *ignored)
{
  (void)ignored;
  int ret;
  char *msg;
  options_test_data_t *tdata = NULL;
  setup_capture_of_logs(LOG_WARN);

  free_options_test_data(tdata);
  tdata = get_options_test_data("ConstrainedSockets 1\n"
                                "ConstrainedSockSize 0\n"
                                );
  mock_clean_saved_logs();
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ, "ConstrainedSockSize is invalid.  Must be a value "
            "between 2048 and 262144 in 1024 byte increments.");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("ConstrainedSockets 1\n"
                                "ConstrainedSockSize 263168\n"
                                );
  mock_clean_saved_logs();
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ, "ConstrainedSockSize is invalid.  Must be a value "
            "between 2048 and 262144 in 1024 byte increments.");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("ConstrainedSockets 1\n"
                                "ConstrainedSockSize 2047\n"
                                );
  mock_clean_saved_logs();
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ, "ConstrainedSockSize is invalid.  Must be a value "
            "between 2048 and 262144 in 1024 byte increments.");
  tor_free(msg);

 done:
  policies_free_all();
  teardown_capture_of_logs();
  free_options_test_data(tdata);
  tor_free(msg);
}

static void
test_options_validate__v3_auth(void *ignored)
{
  (void)ignored;
  int ret;
  char *msg;
  options_test_data_t *tdata = NULL;
  setup_capture_of_logs(LOG_WARN);

  free_options_test_data(tdata);
  tdata = get_options_test_data(ENABLE_AUTHORITY_V3
                                "V3AuthVoteDelay 1000\n"
                                "V3AuthDistDelay 1000\n"
                                "V3AuthVotingInterval 1000\n"
                                );
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ,
            "V3AuthVoteDelay plus V3AuthDistDelay must be less than half "
            "V3AuthVotingInterval");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data(ENABLE_AUTHORITY_V3
                                "V3AuthVoteDelay 1\n");
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ, "V3AuthVoteDelay is way too low.");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data(ENABLE_AUTHORITY_V3
                                "V3AuthVoteDelay 1\n"
                                "TestingTorNetwork 1\n");
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ, "V3AuthVoteDelay is way too low.");
  tor_free(msg);

  // TODO: we can't reach the case of v3authvotedelay lower
  // than MIN_VOTE_SECONDS but not lower than MIN_VOTE_SECONDS_TESTING,
  // since they are the same

  free_options_test_data(tdata);
  tdata = get_options_test_data(ENABLE_AUTHORITY_V3
                                "V3AuthDistDelay 1\n");
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ, "V3AuthDistDelay is way too low.");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data(ENABLE_AUTHORITY_V3
                                "V3AuthDistDelay 1\n"
                                "TestingTorNetwork 1\n"
                                );
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ, "V3AuthDistDelay is way too low.");
  tor_free(msg);

  // We can't reach the case of v3authdistdelay lower than
  // MIN_DIST_SECONDS but not lower than MIN_DIST_SECONDS_TESTING,
  // since they are the same

  free_options_test_data(tdata);
  tdata = get_options_test_data(ENABLE_AUTHORITY_V3
                                "V3AuthNIntervalsValid 1\n"
                                );
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ, "V3AuthNIntervalsValid must be at least 2.");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data(ENABLE_AUTHORITY_V3
                                "V3AuthVoteDelay 49\n"
                                "V3AuthDistDelay 49\n"
                                "V3AuthVotingInterval 200\n"
                                );
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ, "V3AuthVotingInterval is insanely low.");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data(ENABLE_AUTHORITY_V3
                                VALID_DIR_AUTH
                                "TestingTorNetwork 1\n"
                                "V3AuthVoteDelay 49\n"
                                "V3AuthDistDelay 49\n"
                                "V3AuthVotingInterval 200\n"
                                );
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  tt_ptr_op(msg, OP_EQ, NULL);
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data(ENABLE_AUTHORITY_V3
                                VALID_DIR_AUTH
                                "TestingTorNetwork 1\n"
                                "V3AuthVoteDelay 2\n"
                                "V3AuthDistDelay 2\n"
                                "V3AuthVotingInterval 9\n"
                                );
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ,
            "V3AuthVoteDelay plus V3AuthDistDelay must be less than half "
            "V3AuthVotingInterval");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data(ENABLE_AUTHORITY_V3
                                VALID_DIR_AUTH
                                "TestingTorNetwork 1\n"
                                "V3AuthVoteDelay 2\n"
                                "V3AuthDistDelay 2\n"
                                "V3AuthVotingInterval 10\n"
                                );
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  tt_ptr_op(msg, OP_EQ, NULL);
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data(ENABLE_AUTHORITY_V3
                                "V3AuthVoteDelay 49\n"
                                "V3AuthDistDelay 49\n"
                                "V3AuthVotingInterval 200000\n"
                                );
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ, "V3AuthVotingInterval is insanely high.");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data(ENABLE_AUTHORITY_V3
                                "V3AuthVoteDelay 49\n"
                                "V3AuthDistDelay 49\n"
                                "V3AuthVotingInterval 1441\n"
                                );
  mock_clean_saved_logs();
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  expect_log_msg("V3AuthVotingInterval does not divide"
            " evenly into 24 hours.\n");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data(ENABLE_AUTHORITY_V3
                                "V3AuthVoteDelay 49\n"
                                "V3AuthDistDelay 49\n"
                                "V3AuthVotingInterval 1440\n"
                                );
  mock_clean_saved_logs();
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  expect_no_log_msg("V3AuthVotingInterval does not divide"
            " evenly into 24 hours.\n");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data(ENABLE_AUTHORITY_V3
                                "V3AuthVoteDelay 49\n"
                                "V3AuthDistDelay 49\n"
                                "V3AuthVotingInterval 299\n"
                                VALID_DIR_AUTH
                                "TestingTorNetwork 1\n"
                                );
  mock_clean_saved_logs();
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  expect_log_msg("V3AuthVotingInterval is very low. "
            "This may lead to failure to synchronise for a consensus.\n");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data(ENABLE_AUTHORITY_V3
                                "V3AuthVoteDelay 1\n"
                                "V3AuthDistDelay 1\n"
                                "V3AuthVotingInterval 9\n"
                                VALID_DIR_AUTH
                                "TestingTorNetwork 1\n"
                                );
  /* We have to call the dirauth-specific function to reach this case */
  ret = options_validate_dirauth_schedule(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ, "V3AuthVoteDelay is way too low.");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data(ENABLE_AUTHORITY_V3
                                "TestingV3AuthInitialVoteDelay 1\n"
                                VALID_DIR_AUTH
                                "TestingTorNetwork 1\n"
                                );
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ, "TestingV3AuthInitialVoteDelay is way too low.");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data(ENABLE_AUTHORITY_V3
                                "TestingV3AuthInitialDistDelay 1\n"
                                VALID_DIR_AUTH
                                "TestingTorNetwork 1\n"
                                );
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ, "TestingV3AuthInitialDistDelay is way too low.");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data(ENABLE_AUTHORITY_V3
                                VALID_DIR_AUTH
                                "TestingTorNetwork 1\n"
                                );
  tdata->opt->TestingV3AuthVotingStartOffset = 100000;
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ, "TestingV3AuthVotingStartOffset is higher than the "
            "voting interval.");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data(ENABLE_AUTHORITY_V3
                                VALID_DIR_AUTH
                                "TestingTorNetwork 1\n"
                                );
  tdata->opt->TestingV3AuthVotingStartOffset = -1;
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ,
            "TestingV3AuthVotingStartOffset must be non-negative.");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data(ENABLE_AUTHORITY_V3
                                VALID_DIR_AUTH
                                "TestingTorNetwork 1\n"
                                "TestingV3AuthInitialVotingInterval 4\n"
                                );
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ, "TestingV3AuthInitialVotingInterval is insanely low.");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data(ENABLE_AUTHORITY_V3
                                VALID_DIR_AUTH
                                "TestingTorNetwork 1\n"
                                "TestingV3AuthInitialVoteDelay 2\n"
                                "TestingV3AuthInitialDistDelay 2\n"
                                "TestingV3AuthInitialVotingInterval 5\n"
                                );
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  tt_ptr_op(msg, OP_EQ, NULL);
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data(ENABLE_AUTHORITY_V3
                                VALID_DIR_AUTH
                                "TestingTorNetwork 1\n"
                                "TestingV3AuthInitialVotingInterval 7\n"
                                );
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ,
            "TestingV3AuthInitialVotingInterval does not divide evenly into "
            "30 minutes.");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data(ENABLE_AUTHORITY_V3
                                VALID_DIR_AUTH
                                "TestingTorNetwork 1\n"
                                "TestingV3AuthInitialVoteDelay 3\n"
                                "TestingV3AuthInitialDistDelay 3\n"
                                "TestingV3AuthInitialVotingInterval 5\n"
                                );
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ,
            "TestingV3AuthInitialVoteDelay plus "
            "TestingV3AuthInitialDistDelay must be less than "
            "TestingV3AuthInitialVotingInterval");
  tor_free(msg);

 done:
  policies_free_all();
  teardown_capture_of_logs();
  free_options_test_data(tdata);
  tor_free(msg);
}

static void
test_options_validate__virtual_addr(void *ignored)
{
  (void)ignored;
  int ret;
  char *msg;
  options_test_data_t *tdata = NULL;

  free_options_test_data(tdata);
  tdata = get_options_test_data("VirtualAddrNetworkIPv4 !!");
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ, "Error parsing VirtualAddressNetwork !!");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("VirtualAddrNetworkIPv6 !!"
                                );
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ, "Error parsing VirtualAddressNetworkIPv6 !!");
  tor_free(msg);

 done:
  escaped(NULL); // This will free the leaking memory from the previous escaped
  policies_free_all();
  free_options_test_data(tdata);
  tor_free(msg);
}

static void
test_options_validate__testing_options(void *ignored)
{
  (void)ignored;
  int ret;
  char *msg;
  options_test_data_t *tdata = NULL;
  setup_capture_of_logs(LOG_WARN);

#define TEST_TESTING_OPTION(name, accessor, \
                            low_val, high_val, err_low, EXTRA_OPT_STR)  \
  STMT_BEGIN                                                            \
    free_options_test_data(tdata);                                      \
  tdata = get_options_test_data(EXTRA_OPT_STR                           \
                                VALID_DIR_AUTH                          \
                                "TestingTorNetwork 1\n"                 \
                                );                                      \
  accessor(tdata->opt)->name = low_val;                                 \
  ret = options_validate(NULL, tdata->opt,  &msg);            \
  tt_int_op(ret, OP_EQ, -1);                                            \
  tt_str_op(msg, OP_EQ, #name " " err_low);                \
  tor_free(msg); \
                                                                        \
  free_options_test_data(tdata);                                        \
  tdata = get_options_test_data(EXTRA_OPT_STR                           \
                                VALID_DIR_AUTH                          \
                                "TestingTorNetwork 1\n"                 \
                                );                                      \
  accessor(tdata->opt)->name = high_val;                                \
  mock_clean_saved_logs();                                              \
  ret = options_validate(NULL, tdata->opt,  &msg);            \
  tt_int_op(ret, OP_EQ, 0);                                             \
  tt_ptr_op(msg, OP_EQ, NULL);                                          \
  expect_log_msg( #name " is insanely high.\n"); \
  tor_free(msg); \
  STMT_END

  TEST_TESTING_OPTION(TestingClientMaxIntervalWithoutRequest, , -1, 3601,
                      "is way too low.", "");
  TEST_TESTING_OPTION(TestingDirConnectionMaxStall, , 1, 3601,
                      "is way too low.", "");

  TEST_TESTING_OPTION(TestingClientMaxIntervalWithoutRequest, , -1, 3601,
                      "is way too low.", ENABLE_AUTHORITY_V3);
  TEST_TESTING_OPTION(TestingDirConnectionMaxStall, , 1, 3601,
                      "is way too low.", ENABLE_AUTHORITY_V3);

  TEST_TESTING_OPTION(TestingClientMaxIntervalWithoutRequest, , -1, 3601,
                      "is way too low.", ENABLE_AUTHORITY_BRIDGE);
  TEST_TESTING_OPTION(TestingDirConnectionMaxStall, , 1, 3601,
                      "is way too low.", ENABLE_AUTHORITY_BRIDGE);

  free_options_test_data(tdata);
  tdata = get_options_test_data("TestingEnableConnBwEvent 1\n");
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ, "TestingEnableConnBwEvent may only be changed in "
            "testing Tor networks!");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("TestingEnableConnBwEvent 1\n"
                                VALID_DIR_AUTH
                                "TestingTorNetwork 1\n"
                                "___UsingTestNetworkDefaults 0\n"
                                );

  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  tt_assert(!msg);
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("TestingEnableConnBwEvent 1\n"
                                VALID_DIR_AUTH
                                "TestingTorNetwork 0\n"
                                "___UsingTestNetworkDefaults 1\n"
                                );

  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  tt_assert(!msg);
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("TestingEnableCellStatsEvent 1\n");
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ, "TestingEnableCellStatsEvent may only be changed in "
            "testing Tor networks!");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("TestingEnableCellStatsEvent 1\n"
                                VALID_DIR_AUTH
                                "TestingTorNetwork 1\n"
                                "___UsingTestNetworkDefaults 0\n"
                                );

  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  tt_assert(!msg);
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("TestingEnableCellStatsEvent 1\n"
                                VALID_DIR_AUTH
                                "TestingTorNetwork 0\n"
                                "___UsingTestNetworkDefaults 1\n"
                                );

  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  tt_assert(!msg);
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("TestingEnableTbEmptyEvent 1\n"
                                VALID_DIR_AUTH
                                "TestingTorNetwork 1\n"
                                "___UsingTestNetworkDefaults 0\n"
                                );

  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  tt_assert(!msg);
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("TestingEnableTbEmptyEvent 1\n"
                                VALID_DIR_AUTH
                                "TestingTorNetwork 0\n"
                                "___UsingTestNetworkDefaults 1\n"
                                );

  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  tt_assert(!msg);
  tor_free(msg);

 done:
  policies_free_all();
  teardown_capture_of_logs();
  free_options_test_data(tdata);
  tor_free(msg);
}

static void
test_options_validate__accel(void *ignored)
{
  (void)ignored;
  int ret;
  char *msg;
  options_test_data_t *tdata = NULL;

  free_options_test_data(tdata);
  tdata = get_options_test_data("AccelName foo\n");
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  tt_int_op(get_crypto_options(tdata->opt)->HardwareAccel, OP_EQ, 0);
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("AccelName foo\n");
  get_crypto_options(tdata->opt)->HardwareAccel = 2;
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  tt_int_op(get_crypto_options(tdata->opt)->HardwareAccel, OP_EQ, 2);
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("AccelDir 1\n");
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, -1);
  tt_str_op(msg, OP_EQ,
            "Can't use hardware crypto accelerator dir without engine name.");
  tor_free(msg);

  free_options_test_data(tdata);
  tdata = get_options_test_data("AccelDir 1\n"
                                "AccelName something\n");
  ret = options_validate(NULL, tdata->opt, &msg);
  tt_int_op(ret, OP_EQ, 0);
  tor_free(msg);

 done:
  policies_free_all();
  free_options_test_data(tdata);
  tor_free(msg);
}

static int mocked_granularity;

static void
mock_set_log_time_granularity(int g)
{
  mocked_granularity = g;
}

static void
test_options_init_logs_granularity(void *arg)
{
  options_test_data_t *tdata = get_options_test_data("");
  int rv;
  (void) arg;

  MOCK(set_log_time_granularity, mock_set_log_time_granularity);

  /* Reasonable value. */
  tdata->opt->LogTimeGranularity = 100;
  mocked_granularity = -1;
  rv = options_init_logs(NULL, tdata->opt, 0);
  tt_int_op(rv, OP_EQ, 0);
  tt_int_op(mocked_granularity, OP_EQ, 100);

  /* Doesn't divide 1000. */
  tdata->opt->LogTimeGranularity = 249;
  mocked_granularity = -1;
  rv = options_init_logs(NULL, tdata->opt, 0);
  tt_int_op(rv, OP_EQ, 0);
  tt_int_op(mocked_granularity, OP_EQ, 250);

  /* Doesn't divide 1000. */
  tdata->opt->LogTimeGranularity = 3;
  mocked_granularity = -1;
  rv = options_init_logs(NULL, tdata->opt, 0);
  tt_int_op(rv, OP_EQ, 0);
  tt_int_op(mocked_granularity, OP_EQ, 4);

  /* Not a multiple of 1000. */
  tdata->opt->LogTimeGranularity = 1500;
  mocked_granularity = -1;
  rv = options_init_logs(NULL, tdata->opt, 0);
  tt_int_op(rv, OP_EQ, 0);
  tt_int_op(mocked_granularity, OP_EQ, 2000);

  /* Reasonable value. */
  tdata->opt->LogTimeGranularity = 3000;
  mocked_granularity = -1;
  rv = options_init_logs(NULL, tdata->opt, 0);
  tt_int_op(rv, OP_EQ, 0);
  tt_int_op(mocked_granularity, OP_EQ, 3000);

  /* Negative. (Shouldn't be allowed by rest of config parsing.) */
  tdata->opt->LogTimeGranularity = -1;
  mocked_granularity = -1;
  rv = options_init_logs(NULL, tdata->opt, 0);
  tt_int_op(rv, OP_EQ, -1);

  /* Very big */
  tdata->opt->LogTimeGranularity = 3600 * 1000;
  mocked_granularity = -1;
  rv = options_init_logs(NULL, tdata->opt, 0);
  tt_int_op(rv, OP_EQ, 0);
  tt_int_op(mocked_granularity, OP_EQ, 3600 * 1000);

 done:
  free_options_test_data(tdata);
  UNMOCK(set_log_time_granularity);
}

typedef struct {
  char *name;
  log_severity_list_t sev;
  int fd;
  bool stream;
} added_log_t;

static smartlist_t *added_logs = NULL;

static void
mock_add_stream_log_impl(const log_severity_list_t *sev, const char *name,
                         int fd)
{
  added_log_t *a = tor_malloc_zero(sizeof(added_log_t));
  a->name = tor_strdup(name);
  memcpy(&a->sev, sev, sizeof(log_severity_list_t));
  a->fd = fd;
  a->stream = true;
  smartlist_add(added_logs, a);
}

static int
mock_add_file_log(const log_severity_list_t *sev, const char *name, int fd)
{
  added_log_t *a = tor_malloc_zero(sizeof(added_log_t));
  a->name = tor_strdup(name);
  memcpy(&a->sev, sev, sizeof(log_severity_list_t));
  a->fd = fd;
  smartlist_add(added_logs, a);
  return 0;
}

static void
clear_added_logs(void)
{
  SMARTLIST_FOREACH(added_logs, added_log_t *, a,
                    { tor_free(a->name); tor_free(a); });
  smartlist_clear(added_logs);
}

static void
test_options_init_logs_quiet(void *arg)
{
  (void)arg;
  char *cfg = NULL;
  options_test_data_t *tdata = get_options_test_data("");
  char *fn1 = tor_strdup(get_fname_rnd("log"));
  const added_log_t *a;
  int rv;
  tdata->opt->RunAsDaemon = 0;

  added_logs = smartlist_new();
  MOCK(add_stream_log_impl, mock_add_stream_log_impl);
  MOCK(add_file_log, mock_add_file_log);

  tt_ptr_op(tdata->opt->Logs, OP_EQ, NULL);

  /* First, try with no configured logs, and make sure that our configured
     logs match the quiet level. */
  quiet_level = QUIET_SILENT;
  rv = options_init_logs(NULL, tdata->opt, 0);
  tt_int_op(rv, OP_EQ, 0);
  tt_int_op(smartlist_len(added_logs), OP_EQ, 0);

  quiet_level = QUIET_HUSH;
  rv = options_init_logs(NULL, tdata->opt, 0);
  tt_int_op(rv, OP_EQ, 0);
  tt_int_op(smartlist_len(added_logs), OP_EQ, 1);
  a = smartlist_get(added_logs, 0);
  tt_assert(a);
  tt_assert(a->stream);
  tt_int_op(a->fd, OP_EQ, fileno(stdout));
  tt_u64_op(a->sev.masks[SEVERITY_MASK_IDX(LOG_INFO)], OP_EQ, 0);
  tt_u64_op(a->sev.masks[SEVERITY_MASK_IDX(LOG_NOTICE)], OP_EQ, 0);
  tt_u64_op(a->sev.masks[SEVERITY_MASK_IDX(LOG_WARN)], OP_EQ, LD_ALL_DOMAINS);
  clear_added_logs();

  quiet_level = QUIET_NONE;
  rv = options_init_logs(NULL, tdata->opt, 0);
  tt_int_op(rv, OP_EQ, 0);
  tt_int_op(smartlist_len(added_logs), OP_EQ, 1);
  a = smartlist_get(added_logs, 0);
  tt_assert(a);
  tt_assert(a->stream);
  tt_int_op(a->fd, OP_EQ, fileno(stdout));
  tt_u64_op(a->sev.masks[SEVERITY_MASK_IDX(LOG_INFO)], OP_EQ, 0);
  tt_u64_op(a->sev.masks[SEVERITY_MASK_IDX(LOG_NOTICE)], OP_EQ,
            LD_ALL_DOMAINS);
  tt_u64_op(a->sev.masks[SEVERITY_MASK_IDX(LOG_WARN)], OP_EQ, LD_ALL_DOMAINS);
  clear_added_logs();

  /* Make sure that adding a configured log makes the default logs go away. */
  tor_asprintf(&cfg, "Log info file %s\n", fn1);
  free_options_test_data(tdata);
  tdata = get_options_test_data(cfg);
  rv = options_init_logs(NULL, tdata->opt, 0);
  tt_int_op(rv, OP_EQ, 0);
  tt_int_op(smartlist_len(added_logs), OP_EQ, 1);
  a = smartlist_get(added_logs, 0);
  tt_assert(a);
  tt_assert(! a->stream);
  tt_int_op(a->fd, OP_NE, fileno(stdout));
  tt_u64_op(a->sev.masks[SEVERITY_MASK_IDX(LOG_INFO)], OP_EQ, LD_ALL_DOMAINS);
  tt_u64_op(a->sev.masks[SEVERITY_MASK_IDX(LOG_NOTICE)], OP_EQ,
            LD_ALL_DOMAINS);
  tt_u64_op(a->sev.masks[SEVERITY_MASK_IDX(LOG_WARN)], OP_EQ, LD_ALL_DOMAINS);

 done:
  free_options_test_data(tdata);
  tor_free(fn1);
  tor_free(cfg);
  clear_added_logs();
  smartlist_free(added_logs);
  UNMOCK(add_stream_log_impl);
  UNMOCK(add_file_log);
}

static int mock_options_act_status = 0;
static int
mock_options_act(const or_options_t *old_options)
{
  (void)old_options;
  return mock_options_act_status;
}
static int
mock_options_act_reversible(const or_options_t *old_options, char **msg_out)
{
  (void)old_options;
  (void)msg_out;
  return 0;
}

static void
test_options_trial_assign(void *arg)
{
  (void)arg;
  setopt_err_t v;
  config_line_t *lines = NULL;
  char *msg = NULL;
  int r;

  // replace options_act*() so that we don't actually launch tor here.
  MOCK(options_act, mock_options_act);
  MOCK(options_act_reversible, mock_options_act_reversible);

  // Try assigning nothing; that should work.
  v = options_trial_assign(lines, 0, &msg);
  if (msg)
    puts(msg);
  tt_ptr_op(msg, OP_EQ, NULL);
  tt_int_op(v, OP_EQ, SETOPT_OK);

  // Assigning a nickname is okay
  r = config_get_lines("Nickname Hemiramphinae", &lines, 0);
  tt_int_op(r, OP_EQ, 0);
  v = options_trial_assign(lines, 0, &msg);
  tt_ptr_op(msg, OP_EQ, NULL);
  tt_int_op(v, OP_EQ, SETOPT_OK);
  tt_str_op(get_options()->Nickname, OP_EQ, "Hemiramphinae");
  config_free_lines(lines);

  // We can't change the User; that's a transition error.
  r = config_get_lines("User Heraclitus", &lines, 0);
  tt_int_op(r, OP_EQ, 0);
  v = options_trial_assign(lines, 0, &msg);
  tt_int_op(v, OP_EQ, SETOPT_ERR_TRANSITION);
  tt_str_op(msg, OP_EQ,  "While Tor is running, changing User is not allowed");
  tor_free(msg);
  config_free_lines(lines);

  // We can't set the ORPort to nonsense: that's a validation error.
  r = config_get_lines("ORPort fractabling planished", &lines, 0);
  tt_int_op(r, OP_EQ, 0);
  v = options_trial_assign(lines, 0, &msg);
  tt_int_op(v, OP_EQ, SETOPT_ERR_PARSE); // (same error code for now)
  tt_str_op(msg, OP_EQ, "Invalid ORPort configuration");
  tor_free(msg);
  config_free_lines(lines);

  // We can't set UseBridges to a non-boolean: that's a parse error.
  r = config_get_lines("UseBridges ambidextrous", &lines, 0);
  tt_int_op(r, OP_EQ, 0);
  v = options_trial_assign(lines, 0, &msg);
  tt_int_op(v, OP_EQ, SETOPT_ERR_PARSE);
  tt_str_op(msg, OP_EQ,
            "Could not parse UseBridges: Unrecognized value ambidextrous. "
            "Allowed values are 0 and 1.");
  tor_free(msg);
  config_free_lines(lines);

  // this didn't change.
  tt_str_op(get_options()->Nickname, OP_EQ, "Hemiramphinae");

 done:
  config_free_lines(lines);
  tor_free(msg);
  UNMOCK(options_act);
  UNMOCK(options_act_reversible);
}

#ifndef COCCI
#define LOCAL_VALIDATE_TEST(name) \
  { "validate__" #name, test_options_validate__ ## name, TT_FORK, NULL, NULL }
#endif

struct testcase_t options_tests[] = {
  { "validate", test_options_validate, TT_FORK, NULL, NULL },
  { "mem_dircache", test_have_enough_mem_for_dircache, TT_FORK, NULL, NULL },
  LOCAL_VALIDATE_TEST(uname_for_server),
  LOCAL_VALIDATE_TEST(outbound_addresses),
  LOCAL_VALIDATE_TEST(data_directory),
  LOCAL_VALIDATE_TEST(nickname),
  LOCAL_VALIDATE_TEST(contactinfo),
  LOCAL_VALIDATE_TEST(logs),
  LOCAL_VALIDATE_TEST(authdir),
  LOCAL_VALIDATE_TEST(relay_with_hidden_services),
  LOCAL_VALIDATE_TEST(listen_ports),
  LOCAL_VALIDATE_TEST(transproxy),
  LOCAL_VALIDATE_TEST(exclude_nodes),
  LOCAL_VALIDATE_TEST(node_families),
  LOCAL_VALIDATE_TEST(token_bucket),
  LOCAL_VALIDATE_TEST(fetch_dir),
  LOCAL_VALIDATE_TEST(conn_limit),
  LOCAL_VALIDATE_TEST(paths_needed),
  LOCAL_VALIDATE_TEST(max_client_circuits),
  LOCAL_VALIDATE_TEST(ports),
  LOCAL_VALIDATE_TEST(reachable_addresses),
  LOCAL_VALIDATE_TEST(use_bridges),
  LOCAL_VALIDATE_TEST(entry_nodes),
  LOCAL_VALIDATE_TEST(safe_logging),
  LOCAL_VALIDATE_TEST(publish_server_descriptor),
  LOCAL_VALIDATE_TEST(testing),
  LOCAL_VALIDATE_TEST(hidserv),
  LOCAL_VALIDATE_TEST(path_bias),
  LOCAL_VALIDATE_TEST(bandwidth),
  LOCAL_VALIDATE_TEST(circuits),
  LOCAL_VALIDATE_TEST(rend),
  LOCAL_VALIDATE_TEST(single_onion),
  LOCAL_VALIDATE_TEST(accounting),
  LOCAL_VALIDATE_TEST(proxy),
  LOCAL_VALIDATE_TEST(control),
  LOCAL_VALIDATE_TEST(families),
  LOCAL_VALIDATE_TEST(addr_policies),
  LOCAL_VALIDATE_TEST(dir_auth),
  LOCAL_VALIDATE_TEST(transport),
  LOCAL_VALIDATE_TEST(constrained_sockets),
  LOCAL_VALIDATE_TEST(v3_auth),
  LOCAL_VALIDATE_TEST(virtual_addr),
  LOCAL_VALIDATE_TEST(testing_options),
  LOCAL_VALIDATE_TEST(accel),
  { "init_logs/granularity", test_options_init_logs_granularity, TT_FORK,
    NULL, NULL },
  { "init_logs/quiet", test_options_init_logs_quiet, TT_FORK,
    NULL, NULL },
  { "trial_assign", test_options_trial_assign, TT_FORK, NULL, NULL },
  END_OF_TESTCASES              /*  */
};
