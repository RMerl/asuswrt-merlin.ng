// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2022 Linaro
 *
 * (C) Copyright 2022
 * Ying-Chun Liu (PaulLiu) <paul.liu@linaro.org>
 */

#include <common.h>
#include <command.h>
#include <dm.h>
#include <env.h>
#include <fdtdec.h>
#include <log.h>
#include <malloc.h>
#include <net.h>
#include <net/tcp.h>
#include <net/wget.h>
#include <asm/eth.h>
#include <dm/test.h>
#include <dm/device-internal.h>
#include <dm/uclass-internal.h>
#include <test/lib.h>
#include <test/test.h>
#include <test/ut.h>

#define SHIFT_TO_TCPHDRLEN_FIELD(x) ((x) << 4)
#define LEN_B_TO_DW(x) ((x) >> 2)

static int sb_arp_handler(struct udevice *dev, void *packet,
			  unsigned int len)
{
	struct eth_sandbox_priv *priv = dev_get_priv(dev);
	struct arp_hdr *arp = packet + ETHER_HDR_SIZE;
	int ret = 0;

	if (ntohs(arp->ar_op) == ARPOP_REQUEST) {
		priv->fake_host_ipaddr = net_read_ip(&arp->ar_spa);

		ret = sandbox_eth_recv_arp_req(dev);
		if (ret)
			return ret;
		ret = sandbox_eth_arp_req_to_reply(dev, packet, len);
		return ret;
	}

	return -EPROTONOSUPPORT;
}

static int sb_syn_handler(struct udevice *dev, void *packet,
			  unsigned int len)
{
	struct eth_sandbox_priv *priv = dev_get_priv(dev);
	struct ethernet_hdr *eth = packet;
	struct ip_tcp_hdr *tcp = packet + ETHER_HDR_SIZE;
	struct ethernet_hdr *eth_send;
	struct ip_tcp_hdr *tcp_send;

	/* Don't allow the buffer to overrun */
	if (priv->recv_packets >= PKTBUFSRX)
		return 0;

	eth_send = (void *)priv->recv_packet_buffer[priv->recv_packets];
	memcpy(eth_send->et_dest, eth->et_src, ARP_HLEN);
	memcpy(eth_send->et_src, priv->fake_host_hwaddr, ARP_HLEN);
	eth_send->et_protlen = htons(PROT_IP);
	tcp_send = (void *)eth_send + ETHER_HDR_SIZE;
	tcp_send->tcp_src = tcp->tcp_dst;
	tcp_send->tcp_dst = tcp->tcp_src;
	tcp_send->tcp_seq = htonl(0);
	tcp_send->tcp_ack = htonl(ntohl(tcp->tcp_seq) + 1);
	tcp_send->tcp_hlen = SHIFT_TO_TCPHDRLEN_FIELD(LEN_B_TO_DW(TCP_HDR_SIZE));
	tcp_send->tcp_flags = TCP_SYN | TCP_ACK;
	tcp_send->tcp_win = htons(PKTBUFSRX * TCP_MSS >> TCP_SCALE);
	tcp_send->tcp_xsum = 0;
	tcp_send->tcp_ugr = 0;
	tcp_send->tcp_xsum = tcp_set_pseudo_header((uchar *)tcp_send,
						   tcp->ip_src,
						   tcp->ip_dst,
						   TCP_HDR_SIZE,
						   IP_TCP_HDR_SIZE);
	net_set_ip_header((uchar *)tcp_send,
			  tcp->ip_src,
			  tcp->ip_dst,
			  IP_TCP_HDR_SIZE,
			  IPPROTO_TCP);

	priv->recv_packet_length[priv->recv_packets] =
		ETHER_HDR_SIZE + IP_TCP_HDR_SIZE;
	++priv->recv_packets;

	return 0;
}

static int sb_ack_handler(struct udevice *dev, void *packet,
			  unsigned int len)
{
	struct eth_sandbox_priv *priv = dev_get_priv(dev);
	struct ethernet_hdr *eth = packet;
	struct ip_tcp_hdr *tcp = packet + ETHER_HDR_SIZE;
	struct ethernet_hdr *eth_send;
	struct ip_tcp_hdr *tcp_send;
	void *data;
	int pkt_len;
	int payload_len = 0;
	const char *payload1 = "HTTP/1.1 200 OK\r\n"
		"Content-Length: 30\r\n\r\n\r\n"
		"<html><body>Hi</body></html>\r\n";

	/* Don't allow the buffer to overrun */
	if (priv->recv_packets >= PKTBUFSRX)
		return 0;

	eth_send = (void *)priv->recv_packet_buffer[priv->recv_packets];
	memcpy(eth_send->et_dest, eth->et_src, ARP_HLEN);
	memcpy(eth_send->et_src, priv->fake_host_hwaddr, ARP_HLEN);
	eth_send->et_protlen = htons(PROT_IP);
	tcp_send = (void *)eth_send + ETHER_HDR_SIZE;
	tcp_send->tcp_src = tcp->tcp_dst;
	tcp_send->tcp_dst = tcp->tcp_src;
	data = (void *)tcp_send + IP_TCP_HDR_SIZE;

	if (ntohl(tcp->tcp_seq) == 1 && ntohl(tcp->tcp_ack) == 1) {
		tcp_send->tcp_seq = htonl(ntohl(tcp->tcp_ack));
		tcp_send->tcp_ack = htonl(ntohl(tcp->tcp_seq) + 1);
		payload_len = strlen(payload1);
		memcpy(data, payload1, payload_len);
		tcp_send->tcp_flags = TCP_ACK;
	} else if (ntohl(tcp->tcp_seq) == 2) {
		tcp_send->tcp_seq = htonl(ntohl(tcp->tcp_ack));
		tcp_send->tcp_ack = htonl(ntohl(tcp->tcp_seq) + 1);
		payload_len = 0;
		tcp_send->tcp_flags = TCP_ACK | TCP_FIN;
	}

	tcp_send->tcp_hlen = SHIFT_TO_TCPHDRLEN_FIELD(LEN_B_TO_DW(TCP_HDR_SIZE));
	tcp_send->tcp_win = htons(PKTBUFSRX * TCP_MSS >> TCP_SCALE);
	tcp_send->tcp_xsum = 0;
	tcp_send->tcp_ugr = 0;
	pkt_len = IP_TCP_HDR_SIZE + payload_len;
	tcp_send->tcp_xsum = tcp_set_pseudo_header((uchar *)tcp_send,
						   tcp->ip_src,
						   tcp->ip_dst,
						   pkt_len - IP_HDR_SIZE,
						   pkt_len);
	net_set_ip_header((uchar *)tcp_send,
			  tcp->ip_src,
			  tcp->ip_dst,
			  pkt_len,
			  IPPROTO_TCP);

	if (ntohl(tcp->tcp_seq) == 1 || ntohl(tcp->tcp_seq) == 2) {
		priv->recv_packet_length[priv->recv_packets] =
			ETHER_HDR_SIZE + IP_TCP_HDR_SIZE + payload_len;
		++priv->recv_packets;
	}

	return 0;
}

static int sb_http_handler(struct udevice *dev, void *packet,
			   unsigned int len)
{
	struct ethernet_hdr *eth = packet;
	struct ip_hdr *ip;
	struct ip_tcp_hdr *tcp;

	if (ntohs(eth->et_protlen) == PROT_ARP) {
		return sb_arp_handler(dev, packet, len);
	} else if (ntohs(eth->et_protlen) == PROT_IP) {
		ip = packet + ETHER_HDR_SIZE;
		if (ip->ip_p == IPPROTO_TCP) {
			tcp = packet + ETHER_HDR_SIZE;
			if (tcp->tcp_flags == TCP_SYN)
				return sb_syn_handler(dev, packet, len);
			else if (tcp->tcp_flags & TCP_ACK && !(tcp->tcp_flags & TCP_SYN))
				return sb_ack_handler(dev, packet, len);
			return 0;
		}
		return -EPROTONOSUPPORT;
	}

	return -EPROTONOSUPPORT;
}

static int net_test_wget(struct unit_test_state *uts)
{
	sandbox_eth_set_tx_handler(0, sb_http_handler);
	sandbox_eth_set_priv(0, uts);

	env_set("ethact", "eth@10002000");
	env_set("ethrotate", "no");
	env_set("loadaddr", "0x20000");
	ut_assertok(run_command("wget ${loadaddr} 1.1.2.2:/index.html", 0));

	sandbox_eth_set_tx_handler(0, NULL);

	ut_assertok(console_record_reset_enable());
	run_command("md5sum ${loadaddr} ${filesize}", 0);
	ut_assert_nextline("md5 for 00020000 ... 0002001f ==> 234af48e94b0085060249ecb5942ab57");
	ut_assertok(ut_check_console_end(uts));

	return 0;
}

LIB_TEST(net_test_wget, 0);
