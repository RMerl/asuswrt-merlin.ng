// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2020, Heinrich Schuchardt <xypron.glpk@gmx.de>
 *
 * Logging function tests for CONFIG_LOG_SYSLOG=y.
 *
 * Invoke the test with: ./u-boot -d arch/sandbox/dts/test.dtb
 */

/* Override CONFIG_LOG_MAX_LEVEL */
#define LOG_DEBUG

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

int sb_log_tx_handler(struct udevice *dev, void *packet, unsigned int len)
{
	struct eth_sandbox_priv *priv = dev_get_priv(dev);
	struct sb_log_env *env = priv->priv;
	/* uts is updated by the ut_assert* macros */
	struct unit_test_state *uts = env->uts;
	char *buf = packet;
	struct ethernet_hdr *eth_hdr = packet;
	struct ip_udp_hdr *ip_udp_hdr;

	/* Check Ethernet header */
	ut_asserteq_mem(&eth_hdr->et_dest, net_bcast_ethaddr, ARP_HLEN);
	ut_asserteq(ntohs(eth_hdr->et_protlen), PROT_IP);

	/* Check IP header */
	buf += sizeof(struct ethernet_hdr);
	ip_udp_hdr = (struct ip_udp_hdr *)buf;
	ut_asserteq(ip_udp_hdr->ip_p, IPPROTO_UDP);
	ut_asserteq(ip_udp_hdr->ip_dst.s_addr, 0xffffffff);
	ut_asserteq(ntohs(ip_udp_hdr->udp_dst), 514);
	ut_asserteq(UDP_HDR_SIZE + strlen(env->expected) + 1,
		    ntohs(ip_udp_hdr->udp_len));

	/* Check payload */
	buf += sizeof(struct ip_udp_hdr);
	ut_asserteq_mem(env->expected, buf,
			ntohs(ip_udp_hdr->udp_len) - UDP_HDR_SIZE);

	/* Signal that the callback function has been executed */
	env->expected = NULL;

	return 0;
}

int syslog_test_setup(struct unit_test_state *uts)
{
	ut_assertok(log_device_set_enable(LOG_GET_DRIVER(syslog), true));

	return 0;
}

int syslog_test_finish(struct unit_test_state *uts)
{
	ut_assertok(log_device_set_enable(LOG_GET_DRIVER(syslog), false));

	return 0;
}

/**
 * log_test_syslog_err() - test log_err() function
 *
 * @uts:	unit test state
 * Return:	0 = success
 */
static int log_test_syslog_err(struct unit_test_state *uts)
{
	int old_log_level = gd->default_log_level;
	struct sb_log_env env;

	ut_assertok(syslog_test_setup(uts));
	gd->log_fmt = LOGF_TEST;
	gd->default_log_level = LOGL_INFO;
	env_set("ethact", "eth@10002000");
	env_set("log_hostname", "sandbox");
	env.expected = "<3>sandbox uboot: log_test_syslog_err() "
		       "testing log_err\n";
	env.uts = uts;
	sandbox_eth_set_tx_handler(0, sb_log_tx_handler);
	/* Used by ut_assert macros in the tx_handler */
	sandbox_eth_set_priv(0, &env);
	log_err("testing %s\n", "log_err");
	/* Check that the callback function was called */
	sandbox_eth_set_tx_handler(0, NULL);
	gd->default_log_level = old_log_level;
	gd->log_fmt = log_get_default_format();
	ut_assertok(syslog_test_finish(uts));

	return 0;
}
LOG_TEST(log_test_syslog_err);

/**
 * log_test_syslog_warning() - test log_warning() function
 *
 * @uts:	unit test state
 * Return:	0 = success
 */
static int log_test_syslog_warning(struct unit_test_state *uts)
{
	int old_log_level = gd->default_log_level;
	struct sb_log_env env;

	ut_assertok(syslog_test_setup(uts));
	gd->log_fmt = LOGF_TEST;
	gd->default_log_level = LOGL_INFO;
	env_set("ethact", "eth@10002000");
	env_set("log_hostname", "sandbox");
	env.expected = "<4>sandbox uboot: log_test_syslog_warning() "
		       "testing log_warning\n";
	env.uts = uts;
	sandbox_eth_set_tx_handler(0, sb_log_tx_handler);
	/* Used by ut_assert macros in the tx_handler */
	sandbox_eth_set_priv(0, &env);
	log_warning("testing %s\n", "log_warning");
	sandbox_eth_set_tx_handler(0, NULL);
	/* Check that the callback function was called */
	ut_assertnull(env.expected);
	gd->default_log_level = old_log_level;
	gd->log_fmt = log_get_default_format();
	ut_assertok(syslog_test_finish(uts));

	return 0;
}
LOG_TEST(log_test_syslog_warning);

/**
 * log_test_syslog_notice() - test log_notice() function
 *
 * @uts:	unit test state
 * Return:	0 = success
 */
static int log_test_syslog_notice(struct unit_test_state *uts)
{
	int old_log_level = gd->default_log_level;
	struct sb_log_env env;

	ut_assertok(syslog_test_setup(uts));
	gd->log_fmt = LOGF_TEST;
	gd->default_log_level = LOGL_INFO;
	env_set("ethact", "eth@10002000");
	env_set("log_hostname", "sandbox");
	env.expected = "<5>sandbox uboot: log_test_syslog_notice() "
		       "testing log_notice\n";
	env.uts = uts;
	sandbox_eth_set_tx_handler(0, sb_log_tx_handler);
	/* Used by ut_assert macros in the tx_handler */
	sandbox_eth_set_priv(0, &env);
	log_notice("testing %s\n", "log_notice");
	sandbox_eth_set_tx_handler(0, NULL);
	/* Check that the callback function was called */
	ut_assertnull(env.expected);
	gd->default_log_level = old_log_level;
	gd->log_fmt = log_get_default_format();
	ut_assertok(syslog_test_finish(uts));

	return 0;
}
LOG_TEST(log_test_syslog_notice);

/**
 * log_test_syslog_info() - test log_info() function
 *
 * @uts:	unit test state
 * Return:	0 = success
 */
static int log_test_syslog_info(struct unit_test_state *uts)
{
	int old_log_level = gd->default_log_level;
	struct sb_log_env env;

	ut_assertok(syslog_test_setup(uts));
	gd->log_fmt = LOGF_TEST;
	gd->default_log_level = LOGL_INFO;
	env_set("ethact", "eth@10002000");
	env_set("log_hostname", "sandbox");
	env.expected = "<6>sandbox uboot: log_test_syslog_info() "
		       "testing log_info\n";
	env.uts = uts;
	sandbox_eth_set_tx_handler(0, sb_log_tx_handler);
	/* Used by ut_assert macros in the tx_handler */
	sandbox_eth_set_priv(0, &env);
	log_info("testing %s\n", "log_info");
	sandbox_eth_set_tx_handler(0, NULL);
	/* Check that the callback function was called */
	ut_assertnull(env.expected);
	gd->default_log_level = old_log_level;
	gd->log_fmt = log_get_default_format();
	ut_assertok(syslog_test_finish(uts));

	return 0;
}
LOG_TEST(log_test_syslog_info);

/**
 * log_test_syslog_debug() - test log_debug() function
 *
 * @uts:	unit test state
 * Return:	0 = success
 */
static int log_test_syslog_debug(struct unit_test_state *uts)
{
	int old_log_level = gd->default_log_level;
	struct sb_log_env env;

	ut_assertok(syslog_test_setup(uts));
	gd->log_fmt = LOGF_TEST;
	gd->default_log_level = LOGL_DEBUG;
	env_set("ethact", "eth@10002000");
	env_set("log_hostname", "sandbox");
	env.expected = "<7>sandbox uboot: log_test_syslog_debug() "
		       "testing log_debug\n";
	env.uts = uts;
	sandbox_eth_set_tx_handler(0, sb_log_tx_handler);
	/* Used by ut_assert macros in the tx_handler */
	sandbox_eth_set_priv(0, &env);
	log_debug("testing %s\n", "log_debug");
	sandbox_eth_set_tx_handler(0, NULL);
	/* Check that the callback function was called */
	ut_assertnull(env.expected);
	gd->default_log_level = old_log_level;
	gd->log_fmt = log_get_default_format();
	ut_assertok(syslog_test_finish(uts));

	return 0;
}
LOG_TEST(log_test_syslog_debug);
