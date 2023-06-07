/*
 * generate_nlmsg.c	Testsuite helper generating nlmsg blob
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:	Phil Sutter <phil@nwl.cc>
 */

#include <netinet/ether.h>
#include <libnetlink.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <errno.h>
#include <stdio.h>

int fill_vf_rate_test(void *buf, size_t buflen)
{
	char bcmac[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
	struct ifla_vf_mac vf_mac = {
		.mac = { 0x0, 0x26, 0x6c, 0xff, 0xb5, 0xc0 },
	};
	struct ifla_vf_link_state vf_link_state = { 0 };
	struct ifla_vf_tx_rate vf_tx_rate = { 0 };
	struct ifla_vf_spoofchk vf_spoofchk = {
		.setting = 1,
	};
	struct ifla_vf_vlan vf_vlan = { 0 };
	struct rtattr *vfinfo_list, *vfinfo;
	struct nlmsghdr *h = buf;
	struct ifinfomsg *ifi;

	h->nlmsg_type = RTM_NEWLINK;
	h->nlmsg_len = NLMSG_LENGTH(sizeof(*ifi));

	ifi = NLMSG_DATA(h);
	ifi->ifi_type = ARPHRD_ETHER;
	ifi->ifi_index = 1;
	ifi->ifi_flags = IFF_RUNNING | IFF_BROADCAST |
			 IFF_MULTICAST | IFF_UP | IFF_LOWER_UP;

#define ASSERT(x) if (x < 0) return -1
#define ATTR_L(t, v, l)	ASSERT(addattr_l(h, buflen, t, v, l))
#define ATTR_8(t, v)	ASSERT(addattr8(h, buflen, t, v))
#define ATTR_32(t, v)	ASSERT(addattr32(h, buflen, t, v))
#define ATTR_STRZ(t, v)	ASSERT(addattrstrz(h, buflen, t, v))

#define NEST(t) addattr_nest(h, buflen, t)
#define NEST_END(t) addattr_nest_end(h, t)

	ATTR_STRZ(IFLA_IFNAME, "eth0");
	ATTR_32(IFLA_TXQLEN, 10000);
	ATTR_8(IFLA_OPERSTATE, 6);
	ATTR_8(IFLA_LINKMODE, 0);
	ATTR_32(IFLA_MTU, 9000);
	ATTR_32(IFLA_GROUP, 0);
	ATTR_32(IFLA_PROMISCUITY, 0);
	ATTR_32(IFLA_NUM_TX_QUEUES, 8);
	ATTR_32(IFLA_NUM_RX_QUEUES, 8);
	ATTR_8(IFLA_CARRIER, 1);
	ATTR_STRZ(IFLA_QDISC, "mq");
	ATTR_L(IFLA_ADDRESS, vf_mac.mac, ETH_ALEN);
	ATTR_L(IFLA_BROADCAST, bcmac, sizeof(bcmac));
	ATTR_32(IFLA_NUM_VF, 2);

	vfinfo_list = NEST(IFLA_VFINFO_LIST);

	vfinfo = NEST(IFLA_VF_INFO);
	ATTR_L(IFLA_VF_MAC, &vf_mac, sizeof(vf_mac));
	ATTR_L(IFLA_VF_VLAN, &vf_vlan, sizeof(vf_vlan));
	ATTR_L(IFLA_VF_TX_RATE, &vf_tx_rate, sizeof(vf_tx_rate));
	ATTR_L(IFLA_VF_SPOOFCHK, &vf_spoofchk, sizeof(vf_spoofchk));
	ATTR_L(IFLA_VF_LINK_STATE, &vf_link_state, sizeof(vf_link_state));
	NEST_END(vfinfo);

	vf_mac.vf = vf_vlan.vf = vf_tx_rate.vf = 1;
	vf_spoofchk.vf = vf_link_state.vf = 1;

	vfinfo = NEST(IFLA_VF_INFO);
	ATTR_L(IFLA_VF_MAC, &vf_mac, sizeof(vf_mac));
	ATTR_L(IFLA_VF_VLAN, &vf_vlan, sizeof(vf_vlan));
	ATTR_L(IFLA_VF_TX_RATE, &vf_tx_rate, sizeof(vf_tx_rate));
	ATTR_L(IFLA_VF_SPOOFCHK, &vf_spoofchk, sizeof(vf_spoofchk));
	ATTR_L(IFLA_VF_LINK_STATE, &vf_link_state, sizeof(vf_link_state));
	NEST_END(vfinfo);

	NEST_END(vfinfo_list);

	return h->nlmsg_len;
}

int main(void)
{
	char buf[16384] = { 0 };
	int msglen;
	FILE *fp;

	msglen = fill_vf_rate_test(buf, sizeof(buf));
	if (msglen < 0) {
		fprintf(stderr, "fill_vf_rate_test() failed!\n");
		return 1;
	}
	fp = fopen("tests/ip/link/dev_wo_vf_rate.nl", "w");
	if (!fp) {
		perror("fopen()");
		return 1;
	}
	if (fwrite(buf, msglen, 1, fp) != 1) {
		perror("fwrite()");
		return 1;
	}
	fclose(fp);
	return 0;
}
