// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2015 National Instruments
 *
 * (C) Copyright 2015
 * Joe Hershberger <joe.hershberger@ni.com>
 */

#include <common.h>
#include <dm.h>
#include <env.h>
#include <fdtdec.h>
#include <log.h>
#include <malloc.h>
#include <net.h>
#include <net6.h>
#include <asm/eth.h>
#include <dm/test.h>
#include <dm/device-internal.h>
#include <dm/uclass-internal.h>
#include <test/test.h>
#include <test/ut.h>
#include <ndisc.h>

#define DM_TEST_ETH_NUM		4

#if IS_ENABLED(CONFIG_IPV6)
static int dm_test_string_to_ip6(struct unit_test_state *uts)
{
	char *str;
	struct test_ip6_pair {
		char 		*string_addr;
		struct in6_addr ip6_addr;
	};

	struct in6_addr ip6 = {0};

	/* Correct statements */
	struct test_ip6_pair test_suite[] = {
		{"2001:db8::0:1234:1", {.s6_addr32[0] = 0xb80d0120,
					.s6_addr32[1] = 0x00000000,
					.s6_addr32[2] = 0x00000000,
					.s6_addr32[3] = 0x01003412}},
		{"2001:0db8:0000:0000:0000:0000:1234:0001",
				       {.s6_addr32[0] = 0xb80d0120,
					.s6_addr32[1] = 0x00000000,
					.s6_addr32[2] = 0x00000000,
					.s6_addr32[3] = 0x01003412}},
		{"::1", 	       {.s6_addr32[0] = 0x00000000,
					.s6_addr32[1] = 0x00000000,
					.s6_addr32[2] = 0x00000000,
					.s6_addr32[3] = 0x01000000}},
		{"::ffff:192.168.1.1", {.s6_addr32[0] = 0x00000000,
					.s6_addr32[1] = 0x00000000,
					.s6_addr32[2] = 0xffff0000,
					.s6_addr32[3] = 0x0101a8c0}},
	};

	for (int i = 0; i < ARRAY_SIZE(test_suite); ++i) {
		ut_assertok(string_to_ip6(test_suite[i].string_addr,
			    strlen(test_suite[i].string_addr), &ip6));
		ut_asserteq_mem(&ip6, &test_suite[i].ip6_addr,
				sizeof(struct in6_addr));
	}

	/* Incorrect statements */
	str = "hello:world";
	ut_assertok(!string_to_ip6(str, strlen(str), &ip6));
	str = "2001:db8::0::0";
	ut_assertok(!string_to_ip6(str, strlen(str), &ip6));
	str = "2001:db8:192.168.1.1::1";
	ut_assertok(!string_to_ip6(str, strlen(str), &ip6));
	str = "192.168.1.1";
	ut_assertok(!string_to_ip6(str, strlen(str), &ip6));

	return 0;
}
DM_TEST(dm_test_string_to_ip6, 0);

static int dm_test_csum_ipv6_magic(struct unit_test_state *uts)
{
	unsigned short csum = 0xbeef;
	/* Predefined correct parameters */
	unsigned short correct_csum = 0xd8ac;
	struct in6_addr saddr = {.s6_addr32[0] = 0x000080fe,
				 .s6_addr32[1] = 0x00000000,
				 .s6_addr32[2] = 0xffe9f242,
				 .s6_addr32[3] = 0xe8f66dfe};
	struct in6_addr daddr = {.s6_addr32[0] = 0x000080fe,
				 .s6_addr32[1] = 0x00000000,
				 .s6_addr32[2] = 0xffd5b372,
				 .s6_addr32[3] = 0x3ef692fe};
	u16 len = 1460;
	unsigned short proto = 17;
	unsigned int head_csum = 0x91f0;

	csum = csum_ipv6_magic(&saddr, &daddr, len, proto, head_csum);
	ut_asserteq(csum, correct_csum);

	/* Broke a parameter */
	proto--;
	csum = csum_ipv6_magic(&saddr, &daddr, len, proto, head_csum);
	ut_assert(csum != correct_csum);

	return 0;
}
DM_TEST(dm_test_csum_ipv6_magic, 0);

static int dm_test_ip6_addr_in_subnet(struct unit_test_state *uts)
{
	struct in6_addr our = {.s6_addr32[0] = 0x000080fe,
				 .s6_addr32[1] = 0x00000000,
				 .s6_addr32[2] = 0xffe9f242,
				 .s6_addr32[3] = 0xe8f66dfe};
	struct in6_addr neigh1 = {.s6_addr32[0] = 0x000080fe,
				 .s6_addr32[1] = 0x00000000,
				 .s6_addr32[2] = 0xffd5b372,
				 .s6_addr32[3] = 0x3ef692fe};
	struct in6_addr neigh2 = {.s6_addr32[0] = 0x60480120,
				 .s6_addr32[1] = 0x00006048,
				 .s6_addr32[2] = 0x00000000,
				 .s6_addr32[3] = 0x00008888};

	/* in */
	ut_assert(ip6_addr_in_subnet(&our, &neigh1, 64));
	/* outside */
	ut_assert(!ip6_addr_in_subnet(&our, &neigh2, 64));
	ut_assert(!ip6_addr_in_subnet(&our, &neigh1, 128));

	return 0;
}
DM_TEST(dm_test_ip6_addr_in_subnet, 0);

static int dm_test_ip6_make_snma(struct unit_test_state *uts)
{
	struct in6_addr mult = {0};
	struct in6_addr correct_addr = {
				 .s6_addr32[0] = 0x000002ff,
				 .s6_addr32[1] = 0x00000000,
				 .s6_addr32[2] = 0x01000000,
				 .s6_addr32[3] = 0xe8f66dff};
	struct in6_addr addr = { .s6_addr32[0] = 0x000080fe,
				 .s6_addr32[1] = 0x00000000,
				 .s6_addr32[2] = 0xffe9f242,
				 .s6_addr32[3] = 0xe8f66dfe};

	ip6_make_snma(&mult, &addr);
	ut_asserteq_mem(&mult, &correct_addr, sizeof(struct in6_addr));

	return 0;
}
DM_TEST(dm_test_ip6_make_snma, 0);

static int dm_test_ip6_make_lladdr(struct unit_test_state *uts)
{
	struct in6_addr generated_lladdr = {0};
	struct in6_addr correct_lladdr = {
				 .s6_addr32[0] = 0x000080fe,
				 .s6_addr32[1] = 0x00000000,
				 .s6_addr32[2] = 0xffabf33a,
				 .s6_addr32[3] = 0xfbb352fe};
	const unsigned char mac[6] = {0x38, 0xf3, 0xab, 0x52, 0xb3, 0xfb};

	ip6_make_lladdr(&generated_lladdr, mac);
	ut_asserteq_mem(&generated_lladdr, &correct_lladdr,
			sizeof(struct in6_addr));

	return 0;
}
DM_TEST(dm_test_ip6_make_lladdr, UT_TESTF_SCAN_FDT);
#endif

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
DM_TEST(dm_test_eth, UT_TESTF_SCAN_FDT);

static int dm_test_eth_alias(struct unit_test_state *uts)
{
	net_ping_ip = string_to_ip("1.1.2.2");
	env_set("ethact", "eth0");
	ut_assertok(net_loop(PING));
	ut_asserteq_str("eth@10002000", env_get("ethact"));

	env_set("ethact", "eth6");
	ut_assertok(net_loop(PING));
	ut_asserteq_str("eth@10004000", env_get("ethact"));

	/* Expected to fail since eth1 is not defined in the device tree */
	env_set("ethact", "eth1");
	ut_assertok(net_loop(PING));
	ut_asserteq_str("eth@10002000", env_get("ethact"));

	env_set("ethact", "eth5");
	ut_assertok(net_loop(PING));
	ut_asserteq_str("eth@10003000", env_get("ethact"));

	return 0;
}
DM_TEST(dm_test_eth_alias, UT_TESTF_SCAN_FDT);

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
DM_TEST(dm_test_eth_prime, UT_TESTF_SCAN_FDT);

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
						 "eth3addr", "eth6addr"};
	char ethaddr[DM_TEST_ETH_NUM][18];
	int i;

	memset(ethaddr, '\0', sizeof(ethaddr));
	net_ping_ip = string_to_ip("1.1.2.2");

	/* Prepare the test scenario */
	for (i = 0; i < DM_TEST_ETH_NUM; i++) {
		char *addr;

		ut_assertok(uclass_find_device_by_name(UCLASS_ETH,
						       ethname[i], &dev[i]));
		ut_assertok(device_remove(dev[i], DM_REMOVE_NORMAL));

		/* Invalidate MAC address */
		addr = env_get(addrname[i]);
		ut_assertnonnull(addr);
		strncpy(ethaddr[i], addr, 17);
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
DM_TEST(dm_test_eth_act, UT_TESTF_SCAN_FDT);

/* Ensure that all addresses are loaded properly */
static int dm_test_ethaddr(struct unit_test_state *uts)
{
	static const char *const addr[] = {
		"02:00:11:22:33:44",
		"02:00:11:22:33:48", /* dsa slave */
		"02:00:11:22:33:45",
		"02:00:11:22:33:48", /* dsa master */
		"02:00:11:22:33:46",
		"02:00:11:22:33:47",
		"02:00:11:22:33:48", /* dsa slave */
		"02:00:11:22:33:49",
	};
	int i;

	for (i = 0; i < ARRAY_SIZE(addr); i++) {
		char addrname[10];
		char *env_addr;

		if (i)
			snprintf(addrname, sizeof(addrname), "eth%daddr", i + 1);
		else
			strcpy(addrname, "ethaddr");

		env_addr = env_get(addrname);
		ut_assertnonnull(env_addr);
		ut_asserteq_str(addr[i], env_addr);
	}

	return 0;
}
DM_TEST(dm_test_ethaddr, UT_TESTF_SCAN_FDT);

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
	strncpy(ethaddr, env_get("eth6addr"), 17);
	/* Must disable access protection for eth6addr before clearing */
	env_set(".flags", "eth6addr");
	env_set("eth6addr", NULL);

	retval = _dm_test_eth_rotate1(uts);

	/* Restore the env */
	env_set("eth6addr", ethaddr);
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
DM_TEST(dm_test_eth_rotate, UT_TESTF_SCAN_FDT);

/* The asserts include a return on fail; cleanup in the caller */
static int _dm_test_net_retry(struct unit_test_state *uts)
{
	/*
	 * eth1 is disabled and netretry is yes, so the ping should succeed and
	 * the active device should be eth0
	 */
	sandbox_eth_disable_response(1, true);
	env_set("ethact", "lan1");
	env_set("netretry", "yes");
	sandbox_eth_skip_timeout();
	ut_assertok(net_loop(PING));
	ut_asserteq_str("eth@10002000", env_get("ethact"));

	/*
	 * eth1 is disabled and netretry is no, so the ping should fail and the
	 * active device should be eth1
	 */
	env_set("ethact", "lan1");
	env_set("netretry", "no");
	sandbox_eth_skip_timeout();
	ut_asserteq(-ENONET, net_loop(PING));
	ut_asserteq_str("lan1", env_get("ethact"));

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
DM_TEST(dm_test_net_retry, UT_TESTF_SCAN_FDT);

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
	ut_asserteq_mem(eth->et_src, net_ethaddr, ARP_HLEN);
	ut_asserteq_mem(eth->et_dest, priv->fake_host_hwaddr, ARP_HLEN);
	ut_assert(eth->et_protlen == htons(PROT_ARP));

	ut_assert(arp->ar_hrd == htons(ARP_ETHER));
	ut_assert(arp->ar_pro == htons(PROT_IP));
	ut_assert(arp->ar_hln == ARP_HLEN);
	ut_assert(arp->ar_pln == ARP_PLEN);
	ut_asserteq_mem(&arp->ar_sha, net_ethaddr, ARP_HLEN);
	ut_assert(net_read_ip(&arp->ar_spa).s_addr == net_ip.s_addr);
	ut_asserteq_mem(&arp->ar_tha, priv->fake_host_hwaddr, ARP_HLEN);
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

DM_TEST(dm_test_eth_async_arp_reply, UT_TESTF_SCAN_FDT);

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
	ut_asserteq_mem(eth->et_src, net_ethaddr, ARP_HLEN);
	ut_asserteq_mem(eth->et_dest, priv->fake_host_hwaddr, ARP_HLEN);
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

DM_TEST(dm_test_eth_async_ping_reply, UT_TESTF_SCAN_FDT);

#if IS_ENABLED(CONFIG_IPV6_ROUTER_DISCOVERY)

static u8 ip6_ra_buf[] = {0x60, 0xf, 0xc5, 0x4a, 0x0, 0x38, 0x3a, 0xff, 0xfe,
			  0x80, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x6, 0x85, 0xe6,
			  0x29, 0x77, 0xcb, 0xc8, 0x53, 0xff, 0x2, 0x0, 0x0,
			  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
			  0x1, 0x86, 0x0, 0xdc, 0x90, 0x40, 0x80, 0x15, 0x18,
			  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x3, 0x4,
			  0x40, 0xc0, 0x0, 0x0, 0x37, 0xdc, 0x0, 0x0, 0x37,
			  0x78, 0x0, 0x0, 0x0, 0x0, 0x20, 0x1, 0xca, 0xfe, 0xca,
			  0xfe, 0xca, 0xfe, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
			  0x0, 0x1, 0x1, 0x0, 0x15, 0x5d, 0xe2, 0x8a, 0x2};

static int dm_test_validate_ra(struct unit_test_state *uts)
{
	struct ip6_hdr *ip6 = (struct ip6_hdr *)ip6_ra_buf;
	struct icmp6hdr *icmp = (struct icmp6hdr *)(ip6 + 1);
	__be16 temp = 0;

	ut_assert(validate_ra(ip6) == true);

	temp = ip6->payload_len;
	ip6->payload_len = 15;
	ut_assert(validate_ra(ip6) == false);
	ip6->payload_len = temp;

	temp = ip6->saddr.s6_addr16[0];
	ip6->saddr.s6_addr16[0] = 0x2001;
	ut_assert(validate_ra(ip6) == false);
	ip6->saddr.s6_addr16[0] = temp;

	temp = ip6->hop_limit;
	ip6->hop_limit = 15;
	ut_assert(validate_ra(ip6) == false);
	ip6->hop_limit = temp;

	temp = icmp->icmp6_code;
	icmp->icmp6_code = 15;
	ut_assert(validate_ra(ip6) == false);
	icmp->icmp6_code = temp;

	return 0;
}

DM_TEST(dm_test_validate_ra, 0);

static int dm_test_process_ra(struct unit_test_state *uts)
{
	int len = sizeof(ip6_ra_buf);
	struct ip6_hdr *ip6 = (struct ip6_hdr *)ip6_ra_buf;
	struct icmp6hdr *icmp = (struct icmp6hdr *)(ip6 + 1);
	struct ra_msg *msg = (struct ra_msg *)icmp;
	unsigned char *option = msg->opt;
	struct icmp6_ra_prefix_info *prefix =
					(struct icmp6_ra_prefix_info *)option;
	__be16 temp = 0;
	unsigned char option_len = option[1];

	ut_assert(process_ra(ip6, len) == 0);

	temp = icmp->icmp6_rt_lifetime;
	icmp->icmp6_rt_lifetime = 0;
	ut_assert(process_ra(ip6, len) != 0);
	icmp->icmp6_rt_lifetime = temp;

	ut_assert(process_ra(ip6, 0) != 0);

	option[1] = 0;
	ut_assert(process_ra(ip6, len) != 0);
	option[1] = option_len;

	prefix->on_link = false;
	ut_assert(process_ra(ip6, len) != 0);
	prefix->on_link = true;

	temp = prefix->prefix.s6_addr16[0];
	prefix->prefix.s6_addr16[0] = 0x80fe;
	ut_assert(process_ra(ip6, len) != 0);
	prefix->prefix.s6_addr16[0] = temp;

	return 0;
}

DM_TEST(dm_test_process_ra, 0);

#endif
