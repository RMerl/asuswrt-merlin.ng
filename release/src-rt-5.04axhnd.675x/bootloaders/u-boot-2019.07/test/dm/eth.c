// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2015 National Instruments
 *
 * (C) Copyright 2015
 * Joe Hershberger <joe.hershberger@ni.com>
 */

#include <common.h>
#include <dm.h>
#include <fdtdec.h>
#include <malloc.h>
#include <net.h>
#include <dm/test.h>
#include <dm/device-internal.h>
#include <dm/uclass-internal.h>
#include <asm/eth.h>
#include <test/ut.h>

#define DM_TEST_ETH_NUM		4

static int dm_test_eth(struct unit_test_state *uts)
{
	net_ping_ip = string_to_ip("1.1.2.2");

	env_set("ethact", "eth@10002000");
	ut_assertok(net_loop(PING));
	ut_asserteq_str("eth@10002000", env_get("ethact"));

	env_set("ethact", "eth@10003000");
	ut_assertok(net_loop(PING));
	ut_asserteq_str("eth@10003000", env_get("ethact"));

	env_set("ethact", "eth@10004000");
	ut_assertok(net_loop(PING));
	ut_asserteq_str("eth@10004000", env_get("ethact"));

	return 0;
}
DM_TEST(dm_test_eth, DM_TESTF_SCAN_FDT);

static int dm_test_eth_alias(struct unit_test_state *uts)
{
	net_ping_ip = string_to_ip("1.1.2.2");
	env_set("ethact", "eth0");
	ut_assertok(net_loop(PING));
	ut_asserteq_str("eth@10002000", env_get("ethact"));

	env_set("ethact", "eth1");
	ut_assertok(net_loop(PING));
	ut_asserteq_str("eth@10004000", env_get("ethact"));

	/* Expected to fail since eth2 is not defined in the device tree */
	env_set("ethact", "eth2");
	ut_assertok(net_loop(PING));
	ut_asserteq_str("eth@10002000", env_get("ethact"));

	env_set("ethact", "eth5");
	ut_assertok(net_loop(PING));
	ut_asserteq_str("eth@10003000", env_get("ethact"));

	return 0;
}
DM_TEST(dm_test_eth_alias, DM_TESTF_SCAN_FDT);

static int dm_test_eth_prime(struct unit_test_state *uts)
{
	net_ping_ip = string_to_ip("1.1.2.2");

	/* Expected to be "eth@10003000" because of ethprime variable */
	env_set("ethact", NULL);
	env_set("ethprime", "eth5");
	ut_assertok(net_loop(PING));
	ut_asserteq_str("eth@10003000", env_get("ethact"));

	/* Expected to be "eth@10002000" because it is first */
	env_set("ethact", NULL);
	env_set("ethprime", NULL);
	ut_assertok(net_loop(PING));
	ut_asserteq_str("eth@10002000", env_get("ethact"));

	return 0;
}
DM_TEST(dm_test_eth_prime, DM_TESTF_SCAN_FDT);

/**
 * This test case is trying to test the following scenario:
 *	- All ethernet devices are not probed
 *	- "ethaddr" for all ethernet devices are not set
 *	- "ethact" is set to a valid ethernet device name
 *
 * With Sandbox default test configuration, all ethernet devices are
 * probed after power-up, so we have to manually create such scenario:
 *	- Remove all ethernet devices
 *	- Remove all "ethaddr" environment variables
 *	- Set "ethact" to the first ethernet device
 *
 * Do a ping test to see if anything goes wrong.
 */
static int dm_test_eth_act(struct unit_test_state *uts)
{
	struct udevice *dev[DM_TEST_ETH_NUM];
	const char *ethname[DM_TEST_ETH_NUM] = {"eth@10002000", "eth@10003000",
						"sbe5", "eth@10004000"};
	const char *addrname[DM_TEST_ETH_NUM] = {"ethaddr", "eth5addr",
						 "eth3addr", "eth1addr"};
	char ethaddr[DM_TEST_ETH_NUM][18];
	int i;

	memset(ethaddr, '\0', sizeof(ethaddr));
	net_ping_ip = string_to_ip("1.1.2.2");

	/* Prepare the test scenario */
	for (i = 0; i < DM_TEST_ETH_NUM; i++) {
		ut_assertok(uclass_find_device_by_name(UCLASS_ETH,
						       ethname[i], &dev[i]));
		ut_assertok(device_remove(dev[i], DM_REMOVE_NORMAL));

		/* Invalidate MAC address */
		strncpy(ethaddr[i], env_get(addrname[i]), 17);
		/* Must disable access protection for ethaddr before clearing */
		env_set(".flags", addrname[i]);
		env_set(addrname[i], NULL);
	}

	/* Set ethact to "eth@10002000" */
	env_set("ethact", ethname[0]);

	/* Segment fault might happen if something is wrong */
	ut_asserteq(-ENODEV, net_loop(PING));

	for (i = 0; i < DM_TEST_ETH_NUM; i++) {
		/* Restore the env */
		env_set(".flags", addrname[i]);
		env_set(addrname[i], ethaddr[i]);

		/* Probe the device again */
		ut_assertok(device_probe(dev[i]));
	}
	env_set(".flags", NULL);
	env_set("ethact", NULL);

	return 0;
}
DM_TEST(dm_test_eth_act, DM_TESTF_SCAN_FDT);

/* The asserts include a return on fail; cleanup in the caller */
static int _dm_test_eth_rotate1(struct unit_test_state *uts)
{
	/* Make sure that the default is to rotate to the next interface */
	env_set("ethact", "eth@10004000");
	ut_assertok(net_loop(PING));
	ut_asserteq_str("eth@10002000", env_get("ethact"));

	/* If ethrotate is no, then we should fail on a bad MAC */
	env_set("ethact", "eth@10004000");
	env_set("ethrotate", "no");
	ut_asserteq(-EINVAL, net_loop(PING));
	ut_asserteq_str("eth@10004000", env_get("ethact"));

	return 0;
}

static int _dm_test_eth_rotate2(struct unit_test_state *uts)
{
	/* Make sure we can skip invalid devices */
	env_set("ethact", "eth@10004000");
	ut_assertok(net_loop(PING));
	ut_asserteq_str("eth@10004000", env_get("ethact"));

	/* Make sure we can handle device name which is not eth# */
	env_set("ethact", "sbe5");
	ut_assertok(net_loop(PING));
	ut_asserteq_str("sbe5", env_get("ethact"));

	return 0;
}

static int dm_test_eth_rotate(struct unit_test_state *uts)
{
	char ethaddr[18];
	int retval;

	/* Set target IP to mock ping */
	net_ping_ip = string_to_ip("1.1.2.2");

	/* Invalidate eth1's MAC address */
	memset(ethaddr, '\0', sizeof(ethaddr));
	strncpy(ethaddr, env_get("eth1addr"), 17);
	/* Must disable access protection for eth1addr before clearing */
	env_set(".flags", "eth1addr");
	env_set("eth1addr", NULL);

	retval = _dm_test_eth_rotate1(uts);

	/* Restore the env */
	env_set("eth1addr", ethaddr);
	env_set("ethrotate", NULL);

	if (!retval) {
		/* Invalidate eth0's MAC address */
		strncpy(ethaddr, env_get("ethaddr"), 17);
		/* Must disable access protection for ethaddr before clearing */
		env_set(".flags", "ethaddr");
		env_set("ethaddr", NULL);

		retval = _dm_test_eth_rotate2(uts);

		/* Restore the env */
		env_set("ethaddr", ethaddr);
	}
	/* Restore the env */
	env_set(".flags", NULL);

	return retval;
}
DM_TEST(dm_test_eth_rotate, DM_TESTF_SCAN_FDT);

/* The asserts include a return on fail; cleanup in the caller */
static int _dm_test_net_retry(struct unit_test_state *uts)
{
	/*
	 * eth1 is disabled and netretry is yes, so the ping should succeed and
	 * the active device should be eth0
	 */
	sandbox_eth_disable_response(1, true);
	env_set("ethact", "eth@10004000");
	env_set("netretry", "yes");
	sandbox_eth_skip_timeout();
	ut_assertok(net_loop(PING));
	ut_asserteq_str("eth@10002000", env_get("ethact"));

	/*
	 * eth1 is disabled and netretry is no, so the ping should fail and the
	 * active device should be eth1
	 */
	env_set("ethact", "eth@10004000");
	env_set("netretry", "no");
	sandbox_eth_skip_timeout();
	ut_asserteq(-ENONET, net_loop(PING));
	ut_asserteq_str("eth@10004000", env_get("ethact"));

	return 0;
}

static int dm_test_net_retry(struct unit_test_state *uts)
{
	int retval;

	net_ping_ip = string_to_ip("1.1.2.2");

	retval = _dm_test_net_retry(uts);

	/* Restore the env */
	env_set("netretry", NULL);
	sandbox_eth_disable_response(1, false);

	return retval;
}
DM_TEST(dm_test_net_retry, DM_TESTF_SCAN_FDT);

static int sb_check_arp_reply(struct udevice *dev, void *packet,
			      unsigned int len)
{
	struct eth_sandbox_priv *priv = dev_get_priv(dev);
	struct ethernet_hdr *eth = packet;
	struct arp_hdr *arp;
	/* Used by all of the ut_assert macros */
	struct unit_test_state *uts = priv->priv;

	if (ntohs(eth->et_protlen) != PROT_ARP)
		return 0;

	arp = packet + ETHER_HDR_SIZE;

	if (ntohs(arp->ar_op) != ARPOP_REPLY)
		return 0;

	/* This test would be worthless if we are not waiting */
	ut_assert(arp_is_waiting());

	/* Validate response */
	ut_assert(memcmp(eth->et_src, net_ethaddr, ARP_HLEN) == 0);
	ut_assert(memcmp(eth->et_dest, priv->fake_host_hwaddr, ARP_HLEN) == 0);
	ut_assert(eth->et_protlen == htons(PROT_ARP));

	ut_assert(arp->ar_hrd == htons(ARP_ETHER));
	ut_assert(arp->ar_pro == htons(PROT_IP));
	ut_assert(arp->ar_hln == ARP_HLEN);
	ut_assert(arp->ar_pln == ARP_PLEN);
	ut_assert(memcmp(&arp->ar_sha, net_ethaddr, ARP_HLEN) == 0);
	ut_assert(net_read_ip(&arp->ar_spa).s_addr == net_ip.s_addr);
	ut_assert(memcmp(&arp->ar_tha, priv->fake_host_hwaddr, ARP_HLEN) == 0);
	ut_assert(net_read_ip(&arp->ar_tpa).s_addr ==
		  string_to_ip("1.1.2.4").s_addr);

	return 0;
}

static int sb_with_async_arp_handler(struct udevice *dev, void *packet,
				     unsigned int len)
{
	struct eth_sandbox_priv *priv = dev_get_priv(dev);
	struct ethernet_hdr *eth = packet;
	struct arp_hdr *arp = packet + ETHER_HDR_SIZE;
	int ret;

	/*
	 * If we are about to generate a reply to ARP, first inject a request
	 * from another host
	 */
	if (ntohs(eth->et_protlen) == PROT_ARP &&
	    ntohs(arp->ar_op) == ARPOP_REQUEST) {
		/* Make sure sandbox_eth_recv_arp_req() knows who is asking */
		priv->fake_host_ipaddr = string_to_ip("1.1.2.4");

		ret = sandbox_eth_recv_arp_req(dev);
		if (ret)
			return ret;
	}

	sandbox_eth_arp_req_to_reply(dev, packet, len);
	sandbox_eth_ping_req_to_reply(dev, packet, len);

	return sb_check_arp_reply(dev, packet, len);
}

static int dm_test_eth_async_arp_reply(struct unit_test_state *uts)
{
	net_ping_ip = string_to_ip("1.1.2.2");

	sandbox_eth_set_tx_handler(0, sb_with_async_arp_handler);
	/* Used by all of the ut_assert macros in the tx_handler */
	sandbox_eth_set_priv(0, uts);

	env_set("ethact", "eth@10002000");
	ut_assertok(net_loop(PING));
	ut_asserteq_str("eth@10002000", env_get("ethact"));

	sandbox_eth_set_tx_handler(0, NULL);

	return 0;
}

DM_TEST(dm_test_eth_async_arp_reply, DM_TESTF_SCAN_FDT);

static int sb_check_ping_reply(struct udevice *dev, void *packet,
			       unsigned int len)
{
	struct eth_sandbox_priv *priv = dev_get_priv(dev);
	struct ethernet_hdr *eth = packet;
	struct ip_udp_hdr *ip;
	struct icmp_hdr *icmp;
	/* Used by all of the ut_assert macros */
	struct unit_test_state *uts = priv->priv;

	if (ntohs(eth->et_protlen) != PROT_IP)
		return 0;

	ip = packet + ETHER_HDR_SIZE;

	if (ip->ip_p != IPPROTO_ICMP)
		return 0;

	icmp = (struct icmp_hdr *)&ip->udp_src;

	if (icmp->type != ICMP_ECHO_REPLY)
		return 0;

	/* This test would be worthless if we are not waiting */
	ut_assert(arp_is_waiting());

	/* Validate response */
	ut_assert(memcmp(eth->et_src, net_ethaddr, ARP_HLEN) == 0);
	ut_assert(memcmp(eth->et_dest, priv->fake_host_hwaddr, ARP_HLEN) == 0);
	ut_assert(eth->et_protlen == htons(PROT_IP));

	ut_assert(net_read_ip(&ip->ip_src).s_addr == net_ip.s_addr);
	ut_assert(net_read_ip(&ip->ip_dst).s_addr ==
		  string_to_ip("1.1.2.4").s_addr);

	return 0;
}

static int sb_with_async_ping_handler(struct udevice *dev, void *packet,
				      unsigned int len)
{
	struct eth_sandbox_priv *priv = dev_get_priv(dev);
	struct ethernet_hdr *eth = packet;
	struct arp_hdr *arp = packet + ETHER_HDR_SIZE;
	int ret;

	/*
	 * If we are about to generate a reply to ARP, first inject a request
	 * from another host
	 */
	if (ntohs(eth->et_protlen) == PROT_ARP &&
	    ntohs(arp->ar_op) == ARPOP_REQUEST) {
		/* Make sure sandbox_eth_recv_arp_req() knows who is asking */
		priv->fake_host_ipaddr = string_to_ip("1.1.2.4");

		ret = sandbox_eth_recv_ping_req(dev);
		if (ret)
			return ret;
	}

	sandbox_eth_arp_req_to_reply(dev, packet, len);
	sandbox_eth_ping_req_to_reply(dev, packet, len);

	return sb_check_ping_reply(dev, packet, len);
}

static int dm_test_eth_async_ping_reply(struct unit_test_state *uts)
{
	net_ping_ip = string_to_ip("1.1.2.2");

	sandbox_eth_set_tx_handler(0, sb_with_async_ping_handler);
	/* Used by all of the ut_assert macros in the tx_handler */
	sandbox_eth_set_priv(0, uts);

	env_set("ethact", "eth@10002000");
	ut_assertok(net_loop(PING));
	ut_asserteq_str("eth@10002000", env_get("ethact"));

	sandbox_eth_set_tx_handler(0, NULL);

	return 0;
}

DM_TEST(dm_test_eth_async_ping_reply, DM_TESTF_SCAN_FDT);
