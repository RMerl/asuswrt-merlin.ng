// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2020, Heinrich Schuchardt <xypron.glpk@gmx.de>
 *
 * Logging function tests for CONFIG_LOG_SYSLOG=y.
 *
 * Invoke the test with: ./u-boot -d arch/sandbox/dts/test.dtb
 */

#include <common.h>
#include <asm/global_data.h>
#include <dm/device.h>
#include <hexdump.h>
#include <test/log.h>
#include <test/test.h>
#include <test/suites.h>
#include <test/ut.h>
#include <asm/eth.h>
#include "syslog_test.h"

DECLARE_GLOBAL_DATA_PTR;

/**
 * log_test_syslog_nodebug() - test logging level filter
 *
 * Verify that log_debug() does not lead to a log message if the logging level
 * is set to LOGL_INFO.
 *
 * @uts:	unit test state
 * Return:	0 = success
 */
static int log_test_syslog_nodebug(struct unit_test_state *uts)
{
	int old_log_level = gd->default_log_level;
	struct sb_log_env env;

	ut_assertok(syslog_test_setup(uts));
	gd->log_fmt = LOGF_TEST;
	gd->default_log_level = LOGL_INFO;
	env_set("ethact", "eth@10002000");
	env_set("log_hostname", "sandbox");
	env.expected = "<7>sandbox uboot: log_test_syslog_nodebug() "
		       "testing log_debug\n";
	env.uts = uts;
	sandbox_eth_set_tx_handler(0, sb_log_tx_handler);
	/* Used by ut_assert macros in the tx_handler */
	sandbox_eth_set_priv(0, &env);
	log_debug("testing %s\n", "log_debug");
	sandbox_eth_set_tx_handler(0, NULL);
	/* Check that the callback function was not called */
	ut_assertnonnull(env.expected);
	gd->default_log_level = old_log_level;
	gd->log_fmt = log_get_default_format();
	ut_assertok(syslog_test_finish(uts));

	return 0;
}
LOG_TEST(log_test_syslog_nodebug);
