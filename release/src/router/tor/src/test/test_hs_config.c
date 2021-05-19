/* Copyright (c) 2016-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file test_hs_config.c
 * \brief Test hidden service configuration functionality.
 */

#define CONFIG_PRIVATE
#define HS_SERVICE_PRIVATE

#include "test/test.h"
#include "test/test_helpers.h"
#include "test/log_test_helpers.h"
#include "test/resolve_test_helpers.h"

#include "app/config/config.h"
#include "feature/hs/hs_common.h"
#include "feature/hs/hs_config.h"
#include "feature/hs/hs_service.h"
#include "feature/rend/rendservice.h"

static int
helper_config_service(const char *conf, int validate_only)
{
  int ret = 0;
  or_options_t *options = NULL;
  tt_assert(conf);
  options = helper_parse_options(conf);
  tt_assert(options);
  ret = hs_config_service_all(options, validate_only);
 done:
  or_options_free(options);
  return ret;
}

static void
test_invalid_service(void *arg)
{
  int ret;

  (void) arg;

  /* Try with a missing port configuration. */
  {
    const char *conf =
      "HiddenServiceDir /tmp/tor-test-hs-RANDOM/hs1\n"
      "HiddenServiceVersion 1\n"; /* Wrong not supported version. */
    setup_full_capture_of_logs(LOG_WARN);
    ret = helper_config_service(conf, 1);
    tt_int_op(ret, OP_EQ, -1);
    expect_log_msg_containing("HiddenServiceVersion must be between 2 and 3");
    teardown_capture_of_logs();
  }

  /* Bad value of HiddenServiceAllowUnknownPorts. */
  {
    const char *conf =
      "HiddenServiceDir /tmp/tor-test-hs-RANDOM/hs1\n"
      "HiddenServiceVersion 2\n"
      "HiddenServiceAllowUnknownPorts 2\n"; /* Should be 0 or 1. */
    setup_full_capture_of_logs(LOG_WARN);
    ret = helper_config_service(conf, 1);
    tt_int_op(ret, OP_EQ, -1);
    expect_log_msg_containing("Could not parse "
                              "HiddenServiceAllowUnknownPorts: Unrecognized "
                              "value 2. Allowed values are 0 and 1.");
    teardown_capture_of_logs();
  }

  /* Bad value of HiddenServiceDirGroupReadable */
  {
    const char *conf =
      "HiddenServiceDir /tmp/tor-test-hs-RANDOM/hs1\n"
      "HiddenServiceVersion 2\n"
      "HiddenServiceDirGroupReadable 2\n"; /* Should be 0 or 1. */
    setup_full_capture_of_logs(LOG_WARN);
    ret = helper_config_service(conf, 1);
    tt_int_op(ret, OP_EQ, -1);
    expect_log_msg_containing("Could not parse "
                              "HiddenServiceDirGroupReadable: "
                              "Unrecognized value 2.");
    teardown_capture_of_logs();
  }

  /* Bad value of HiddenServiceMaxStreamsCloseCircuit */
  {
    const char *conf =
      "HiddenServiceDir /tmp/tor-test-hs-RANDOM/hs1\n"
      "HiddenServiceVersion 2\n"
      "HiddenServiceMaxStreamsCloseCircuit 2\n"; /* Should be 0 or 1. */
    setup_full_capture_of_logs(LOG_WARN);
    ret = helper_config_service(conf, 1);
    tt_int_op(ret, OP_EQ, -1);
    expect_log_msg_containing("Could not parse "
                              "HiddenServiceMaxStreamsCloseCircuit: "
                              "Unrecognized value 2");
    teardown_capture_of_logs();
  }

  /* Too much max streams. */
  {
    const char *conf =
      "HiddenServiceDir /tmp/tor-test-hs-RANDOM/hs1\n"
      "HiddenServiceVersion 2\n"
      "HiddenServicePort 80\n"
      "HiddenServiceMaxStreams 65536\n"; /* One too many. */
    setup_full_capture_of_logs(LOG_WARN);
    ret = helper_config_service(conf, 1);
    tt_int_op(ret, OP_EQ, -1);
    expect_log_msg_containing("HiddenServiceMaxStreams must be between "
                              "0 and 65535, not 65536");
    teardown_capture_of_logs();
  }

  /* Duplicate directory directive. */
  {
    const char *conf =
      "HiddenServiceDir /tmp/tor-test-hs-RANDOM/hs1\n"
      "HiddenServiceVersion 2\n"
      "HiddenServicePort 80\n"
      "HiddenServiceDir /tmp/tor-test-hs-RANDOM/hs1\n"
      "HiddenServiceVersion 2\n"
      "HiddenServicePort 81\n";
    setup_full_capture_of_logs(LOG_WARN);
    ret = helper_config_service(conf, 1);
    tt_int_op(ret, OP_EQ, -1);
    expect_log_msg_containing("Another hidden service is already "
                              "configured for directory");
    teardown_capture_of_logs();
  }

  /* Bad port. */
  {
    const char *conf =
      "HiddenServiceDir /tmp/tor-test-hs-RANDOM/hs1\n"
      "HiddenServiceVersion 2\n"
      "HiddenServicePort 65536\n";
    setup_full_capture_of_logs(LOG_WARN);
    ret = helper_config_service(conf, 1);
    tt_int_op(ret, OP_EQ, -1);
    expect_log_msg_containing("Missing or invalid port");
    teardown_capture_of_logs();
  }

  /* Bad target addr:port separation. */
  {
    const char *conf =
      "HiddenServiceDir /tmp/tor-test-hs-RANDOM/hs1\n"
      "HiddenServiceVersion 2\n"
      "HiddenServicePort 80 127.0.0.1 8000\n";
    setup_full_capture_of_logs(LOG_WARN);
    ret = helper_config_service(conf, 1);
    tt_int_op(ret, OP_EQ, -1);
    expect_log_msg_containing("HiddenServicePort parse error: "
                              "invalid port mapping");
    teardown_capture_of_logs();
  }

  /* Out of order directives. */
  {
    const char *conf =
      "HiddenServiceVersion 2\n"
      "HiddenServiceDir /tmp/tor-test-hs-RANDOM/hs1\n"
      "HiddenServicePort 80\n";
    setup_full_capture_of_logs(LOG_WARN);
    ret = helper_config_service(conf, 1);
    tt_int_op(ret, OP_EQ, -1);
    expect_log_msg_containing("HiddenServiceVersion with no preceding "
                              "HiddenServiceDir directive");
    teardown_capture_of_logs();
  }

 done:
  ;
}

static void
test_valid_service(void *arg)
{
  int ret;

  (void) arg;

  /* Mix of v2 and v3. Still valid. */
  {
    const char *conf =
      "HiddenServiceDir /tmp/tor-test-hs-RANDOM/hs1\n"
      "HiddenServiceVersion 2\n"
      "HiddenServicePort 80\n"
      "HiddenServiceDir /tmp/tor-test-hs-RANDOM/hs2\n"
      "HiddenServiceVersion 3\n"
      "HiddenServicePort 81\n"
      "HiddenServiceDir /tmp/tor-test-hs-RANDOM/hs3\n"
      "HiddenServiceVersion 2\n"
      "HiddenServicePort 82\n";
    ret = helper_config_service(conf, 1);
    tt_int_op(ret, OP_EQ, 0);
  }

 done:
  ;
}

static void
test_invalid_service_v2(void *arg)
{
  int validate_only = 1, ret;

  (void) arg;

  /* Try with a missing port configuration. */
  {
    const char *conf =
      "HiddenServiceDir /tmp/tor-test-hs-RANDOM/hs1\n"
      "HiddenServiceVersion 2\n";
    setup_full_capture_of_logs(LOG_WARN);
    ret = helper_config_service(conf, validate_only);
    tt_int_op(ret, OP_EQ, -1);
    expect_log_msg_containing("with no ports configured.");
    teardown_capture_of_logs();
  }

  /* Too many introduction points. */
  {
    const char *conf =
      "HiddenServiceDir /tmp/tor-test-hs-RANDOM/hs1\n"
      "HiddenServiceVersion 2\n"
      "HiddenServicePort 80\n"
      "HiddenServiceNumIntroductionPoints 11\n"; /* One too many. */
    setup_full_capture_of_logs(LOG_WARN);
    ret = helper_config_service(conf, validate_only);
    tt_int_op(ret, OP_EQ, -1);
    expect_log_msg_containing("HiddenServiceNumIntroductionPoints must "
                              "be between 0 and 10, not 11.");
    teardown_capture_of_logs();
  }

  /* Too little introduction points. */
  {
    const char *conf =
      "HiddenServiceDir /tmp/tor-test-hs-RANDOM/hs1\n"
      "HiddenServiceVersion 2\n"
      "HiddenServicePort 80\n"
      "HiddenServiceNumIntroductionPoints -1\n";
    setup_full_capture_of_logs(LOG_WARN);
    ret = helper_config_service(conf, validate_only);
    tt_int_op(ret, OP_EQ, -1);
    expect_log_msg_containing("Could not parse "
                              "HiddenServiceNumIntroductionPoints: "
                              "Integer -1 is malformed or out of bounds.");
    teardown_capture_of_logs();
  }

  /* Bad authorized client type. */
  {
    const char *conf =
      "HiddenServiceDir /tmp/tor-test-hs-RANDOM/hs1\n"
      "HiddenServiceVersion 2\n"
      "HiddenServicePort 80\n"
      "HiddenServiceAuthorizeClient blah alice,bob\n"; /* blah is no good. */
    setup_full_capture_of_logs(LOG_WARN);
    ret = helper_config_service(conf, validate_only);
    tt_int_op(ret, OP_EQ, -1);
    expect_log_msg_containing("HiddenServiceAuthorizeClient contains "
                              "unrecognized auth-type");
    teardown_capture_of_logs();
  }

 done:
  ;
}

static void
test_valid_service_v2(void *arg)
{
  int ret;

  (void) arg;
  mock_hostname_resolver();

  /* Valid complex configuration. Basic client authorization. */
  {
    const char *conf =
      "HiddenServiceDir /tmp/tor-test-hs-RANDOM/hs1\n"
      "HiddenServiceVersion 2\n"
      "HiddenServicePort 80\n"
      "HiddenServicePort 22 localhost:22\n"
#ifdef HAVE_SYS_UN_H
      "HiddenServicePort 42 unix:/path/to/socket\n"
#endif
      "HiddenServiceAuthorizeClient basic alice,bob,eve\n"
      "HiddenServiceAllowUnknownPorts 1\n"
      "HiddenServiceMaxStreams 42\n"
      "HiddenServiceMaxStreamsCloseCircuit 0\n"
      "HiddenServiceDirGroupReadable 1\n"
      "HiddenServiceNumIntroductionPoints 7\n";
    ret = helper_config_service(conf, 1);
    tt_int_op(ret, OP_EQ, 0);
  }

  /* Valid complex configuration. Stealth client authorization. */
  {
    const char *conf =
      "HiddenServiceDir /tmp/tor-test-hs-RANDOM/hs2\n"
      "HiddenServiceVersion 2\n"
      "HiddenServicePort 65535\n"
      "HiddenServicePort 22 1.1.1.1:22\n"
#ifdef HAVE_SYS_UN_H
      "HiddenServicePort 9000 unix:/path/to/socket\n"
#endif
      "HiddenServiceAuthorizeClient stealth charlie,romeo\n"
      "HiddenServiceAllowUnknownPorts 0\n"
      "HiddenServiceMaxStreams 42\n"
      "HiddenServiceMaxStreamsCloseCircuit 0\n"
      "HiddenServiceDirGroupReadable 1\n"
      "HiddenServiceNumIntroductionPoints 8\n";
    ret = helper_config_service(conf, 1);
    tt_int_op(ret, OP_EQ, 0);
  }

 done:
  unmock_hostname_resolver();
}

static void
test_invalid_service_v3(void *arg)
{
  int validate_only = 1, ret;

  (void) arg;

  /* Try with a missing port configuration. */
  {
    const char *conf =
      "HiddenServiceDir /tmp/tor-test-hs-RANDOM/hs1\n"
      "HiddenServiceVersion 3\n";
    setup_full_capture_of_logs(LOG_WARN);
    ret = helper_config_service(conf, validate_only);
    tt_int_op(ret, OP_EQ, -1);
    expect_log_msg_containing("with no ports configured.");
    teardown_capture_of_logs();
  }

  /* Too many introduction points. */
  {
    const char *conf =
      "HiddenServiceDir /tmp/tor-test-hs-RANDOM/hs1\n"
      "HiddenServiceVersion 3\n"
      "HiddenServicePort 80\n"
      "HiddenServiceNumIntroductionPoints 21\n"; /* One too many. */
    setup_full_capture_of_logs(LOG_WARN);
    ret = helper_config_service(conf, validate_only);
    tt_int_op(ret, OP_EQ, -1);
    expect_log_msg_containing("HiddenServiceNumIntroductionPoints must "
                              "be between 3 and 20, not 21.");
    teardown_capture_of_logs();
  }

  /* Too little introduction points. */
  {
    const char *conf =
      "HiddenServiceDir /tmp/tor-test-hs-RANDOM/hs1\n"
      "HiddenServiceVersion 3\n"
      "HiddenServicePort 80\n"
      "HiddenServiceNumIntroductionPoints 1\n";
    setup_full_capture_of_logs(LOG_WARN);
    ret = helper_config_service(conf, validate_only);
    tt_int_op(ret, OP_EQ, -1);
    expect_log_msg_containing("HiddenServiceNumIntroductionPoints must "
                              "be between 3 and 20, not 1.");
    teardown_capture_of_logs();
  }

  /* v2-specific HiddenServiceAuthorizeClient set. */
  {
    const char *conf =
      "HiddenServiceDir /tmp/tor-test-hs-RANDOM/hs1\n"
      "HiddenServiceVersion 3\n"
      "HiddenServiceAuthorizeClient stealth client1\n";
    setup_full_capture_of_logs(LOG_WARN);
    ret = helper_config_service(conf, validate_only);
    tt_int_op(ret, OP_EQ, -1);
    expect_log_msg_containing("Hidden service option "
                              "HiddenServiceAuthorizeClient is incompatible "
                              "with version 3 of service in "
                              "/tmp/tor-test-hs-RANDOM/hs1");
    teardown_capture_of_logs();
  }

 done:
  ;
}

static void
test_valid_service_v3(void *arg)
{
  int ret;

  (void) arg;
  mock_hostname_resolver();

  /* Valid complex configuration. */
  {
    const char *conf =
      "HiddenServiceDir /tmp/tor-test-hs-RANDOM/hs1\n"
      "HiddenServiceVersion 3\n"
      "HiddenServicePort 80\n"
      "HiddenServicePort 22 localhost:22\n"
#ifdef HAVE_SYS_UN_H
      "HiddenServicePort 42 unix:/path/to/socket\n"
#endif
      "HiddenServiceAllowUnknownPorts 1\n"
      "HiddenServiceMaxStreams 42\n"
      "HiddenServiceMaxStreamsCloseCircuit 0\n"
      "HiddenServiceDirGroupReadable 1\n"
      "HiddenServiceNumIntroductionPoints 7\n";
    ret = helper_config_service(conf, 1);
    tt_int_op(ret, OP_EQ, 0);
  }

  /* Valid complex configuration. */
  {
    const char *conf =
      "HiddenServiceDir /tmp/tor-test-hs-RANDOM/hs2\n"
      "HiddenServiceVersion 3\n"
      "HiddenServicePort 65535\n"
      "HiddenServicePort 22 1.1.1.1:22\n"
#ifdef HAVE_SYS_UN_H
      "HiddenServicePort 9000 unix:/path/to/socket\n"
#endif
      "HiddenServiceAllowUnknownPorts 0\n"
      "HiddenServiceMaxStreams 42\n"
      "HiddenServiceMaxStreamsCloseCircuit 0\n"
      "HiddenServiceDirGroupReadable 1\n"
      "HiddenServiceNumIntroductionPoints 20\n";
    ret = helper_config_service(conf, 1);
    tt_int_op(ret, OP_EQ, 0);
  }

  /* Mix of v2 and v3. Still valid. */
  {
    const char *conf =
      "HiddenServiceDir /tmp/tor-test-hs-RANDOM/hs1\n"
      "HiddenServiceVersion 2\n"
      "HiddenServicePort 80\n"
      "HiddenServiceDir /tmp/tor-test-hs-RANDOM/hs2\n"
      "HiddenServiceVersion 3\n"
      "HiddenServicePort 81\n"
      "HiddenServiceDir /tmp/tor-test-hs-RANDOM/hs3\n"
      "HiddenServiceVersion 2\n"
      "HiddenServicePort 82\n";
    ret = helper_config_service(conf, 1);
    tt_int_op(ret, OP_EQ, 0);
  }

 done:
  unmock_hostname_resolver();
}

static void
test_staging_service_v3(void *arg)
{
  int ret;

  (void) arg;

  /* We don't validate a service object, this is the service test that are in
   * charge of doing so. We just check for the stable state after
   * registration. */

  hs_init();

  /* Time for a valid v3 service that should get staged. */
  const char *conf =
    "HiddenServiceDir /tmp/tor-test-hs-RANDOM/hs2\n"
    "HiddenServiceVersion 3\n"
    "HiddenServicePort 65535\n"
    "HiddenServicePort 22 1.1.1.1:22\n"
#ifdef HAVE_SYS_UN_H
    "HiddenServicePort 9000 unix:/path/to/socket\n"
#endif
    "HiddenServiceAllowUnknownPorts 0\n"
    "HiddenServiceMaxStreams 42\n"
    "HiddenServiceMaxStreamsCloseCircuit 0\n"
    "HiddenServiceDirGroupReadable 1\n"
    "HiddenServiceNumIntroductionPoints 20\n";
  ret = helper_config_service(conf, 0);
  tt_int_op(ret, OP_EQ, 0);
  /* Ok, we have a service in our map! Registration went well. */
  tt_int_op(get_hs_service_staging_list_size(), OP_EQ, 1);
  /* Make sure we don't have a magic v2 service out of this. */
  tt_int_op(rend_num_services(), OP_EQ, 0);

 done:
  hs_free_all();
}

static void
test_dos_parameters(void *arg)
{
  int ret;

  (void) arg;

  hs_init();

  /* Valid configuration. */
  {
    const char *conf =
      "HiddenServiceDir /tmp/tor-test-hs-RANDOM/hs3\n"
      "HiddenServiceVersion 3\n"
      "HiddenServicePort 22 1.1.1.1:22\n"
      "HiddenServiceEnableIntroDoSDefense 1\n"
      "HiddenServiceEnableIntroDoSRatePerSec 42\n"
      "HiddenServiceEnableIntroDoSBurstPerSec 87\n";

    setup_full_capture_of_logs(LOG_INFO);
    ret = helper_config_service(conf, 0);
    tt_int_op(ret, OP_EQ, 0);
    expect_log_msg_containing("Service INTRO2 DoS defenses rate set to: 42");
    expect_log_msg_containing("Service INTRO2 DoS defenses burst set to: 87");
    teardown_capture_of_logs();
  }

  /* Invalid rate. Value of 2^37. Max allowed is 2^31. */
  {
    const char *conf =
      "HiddenServiceDir /tmp/tor-test-hs-RANDOM/hs3\n"
      "HiddenServiceVersion 3\n"
      "HiddenServicePort 22 1.1.1.1:22\n"
      "HiddenServiceEnableIntroDoSDefense 1\n"
      "HiddenServiceEnableIntroDoSRatePerSec 137438953472\n"
      "HiddenServiceEnableIntroDoSBurstPerSec 87\n";

    setup_full_capture_of_logs(LOG_WARN);
    ret = helper_config_service(conf, 0);
    tt_int_op(ret, OP_EQ, -1);
    expect_log_msg_containing("Could not parse "
                              "HiddenServiceEnableIntroDoSRatePerSec: "
                              "Integer 137438953472 is malformed or out of "
                              "bounds.");
    teardown_capture_of_logs();
  }

  /* Invalid burst. Value of 2^38. Max allowed is 2^31. */
  {
    const char *conf =
      "HiddenServiceDir /tmp/tor-test-hs-RANDOM/hs3\n"
      "HiddenServiceVersion 3\n"
      "HiddenServicePort 22 1.1.1.1:22\n"
      "HiddenServiceEnableIntroDoSDefense 1\n"
      "HiddenServiceEnableIntroDoSRatePerSec 42\n"
      "HiddenServiceEnableIntroDoSBurstPerSec 274877906944\n";

    setup_full_capture_of_logs(LOG_WARN);
    ret = helper_config_service(conf, 0);
    tt_int_op(ret, OP_EQ, -1);
    expect_log_msg_containing("Could not parse "
                              "HiddenServiceEnableIntroDoSBurstPerSec: "
                              "Integer 274877906944 is malformed or out "
                              "of bounds.");
    teardown_capture_of_logs();
  }

  /* Burst is smaller than rate. */
  {
    const char *conf =
      "HiddenServiceDir /tmp/tor-test-hs-RANDOM/hs3\n"
      "HiddenServiceVersion 3\n"
      "HiddenServicePort 22 1.1.1.1:22\n"
      "HiddenServiceEnableIntroDoSDefense 1\n"
      "HiddenServiceEnableIntroDoSRatePerSec 42\n"
      "HiddenServiceEnableIntroDoSBurstPerSec 27\n";

    setup_full_capture_of_logs(LOG_WARN);
    ret = helper_config_service(conf, 0);
    tt_int_op(ret, OP_EQ, -1);
    expect_log_msg_containing("Hidden service DoS defenses burst (27) can "
                              "not be smaller than the rate value (42).");
    teardown_capture_of_logs();
  }

  /* Negative value. */
  {
    const char *conf =
      "HiddenServiceDir /tmp/tor-test-hs-RANDOM/hs3\n"
      "HiddenServiceVersion 3\n"
      "HiddenServicePort 22 1.1.1.1:22\n"
      "HiddenServiceEnableIntroDoSDefense 1\n"
      "HiddenServiceEnableIntroDoSRatePerSec -1\n"
      "HiddenServiceEnableIntroDoSBurstPerSec 42\n";

    setup_full_capture_of_logs(LOG_WARN);
    ret = helper_config_service(conf, 0);
    tt_int_op(ret, OP_EQ, -1);
    expect_log_msg_containing("Could not parse "
                              "HiddenServiceEnableIntroDoSRatePerSec: "
                              "Integer -1 is malformed or out of bounds.");
    teardown_capture_of_logs();
  }

 done:
  hs_free_all();
}

struct testcase_t hs_config_tests[] = {
  /* Invalid service not specific to any version. */
  { "invalid_service", test_invalid_service, TT_FORK,
    NULL, NULL },
  { "valid_service", test_valid_service, TT_FORK,
    NULL, NULL },

  /* Test case only for version 2. */
  { "invalid_service_v2", test_invalid_service_v2, TT_FORK,
    NULL, NULL },
  { "valid_service_v2", test_valid_service_v2, TT_FORK,
    NULL, NULL },

  /* Test case only for version 3. */
  { "invalid_service_v3", test_invalid_service_v3, TT_FORK,
    NULL, NULL },
  { "valid_service_v3", test_valid_service_v3, TT_FORK,
    NULL, NULL },

  /* Test service staging. */
  { "staging_service_v3", test_staging_service_v3, TT_FORK,
    NULL, NULL },

  /* Test HS DoS parameters. */
  { "dos_parameters", test_dos_parameters, TT_FORK,
    NULL, NULL },

  END_OF_TESTCASES
};
