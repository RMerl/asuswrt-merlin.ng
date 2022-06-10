/* Copyright (c) 2014-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#include "core/or/or.h"
#include "test/test.h"
#define HIBERNATE_PRIVATE
#include "feature/hibernate/hibernate.h"
#include "app/config/config.h"
#define STATEFILE_PRIVATE
#include "app/config/statefile.h"

#include "app/config/or_state_st.h"

/*
 * Test to make sure accounting triggers hibernation
 * correctly with both sum or max rules set
 */

static or_state_t *or_state;
static or_state_t * acct_limits_get_or_state(void);
ATTR_UNUSED static int acct_limits_get_or_state_called = 0;
static or_state_t *
acct_limits_get_or_state(void)
{
  return or_state;
}

static void
test_accounting_limits(void *arg)
{
  or_options_t *options = get_options_mutable();
  time_t fake_time = time(NULL);
  (void) arg;

  MOCK(get_or_state,
       acct_limits_get_or_state);
  or_state = or_state_new();

  options->AccountingMax = 100;
  options->AccountingRule = ACCT_MAX;

  tor_assert(accounting_is_enabled(options));
  configure_accounting(fake_time);

  accounting_add_bytes(10, 0, 1);
  fake_time += 1;
  consider_hibernation(fake_time);
  tor_assert(we_are_hibernating() == 0);

  accounting_add_bytes(90, 0, 1);
  fake_time += 1;
  consider_hibernation(fake_time);
  tor_assert(we_are_hibernating() == 1);

  options->AccountingMax = 200;
  options->AccountingRule = ACCT_SUM;

  accounting_add_bytes(0, 10, 1);
  fake_time += 1;
  consider_hibernation(fake_time);
  tor_assert(we_are_hibernating() == 0);

  accounting_add_bytes(0, 90, 1);
  fake_time += 1;
  consider_hibernation(fake_time);
  tor_assert(we_are_hibernating() == 1);

  options->AccountingRule = ACCT_OUT;

  accounting_add_bytes(100, 10, 1);
  fake_time += 1;
  consider_hibernation(fake_time);
  tor_assert(we_are_hibernating() == 0);

  accounting_add_bytes(0, 90, 1);
  fake_time += 1;
  consider_hibernation(fake_time);
  tor_assert(we_are_hibernating() == 1);

  options->AccountingMax = 300;
  options->AccountingRule = ACCT_IN;

  accounting_add_bytes(10, 100, 1);
  fake_time += 1;
  consider_hibernation(fake_time);
  tor_assert(we_are_hibernating() == 0);

  accounting_add_bytes(90, 0, 1);
  fake_time += 1;
  consider_hibernation(fake_time);
  tor_assert(we_are_hibernating() == 1);

  goto done;
 done:
  UNMOCK(get_or_state);
  or_state_free(or_state);
}

struct testcase_t accounting_tests[] = {
  { "bwlimits", test_accounting_limits, TT_FORK, NULL, NULL },
  END_OF_TESTCASES
};
