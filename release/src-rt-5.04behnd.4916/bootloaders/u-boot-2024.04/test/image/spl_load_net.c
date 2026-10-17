// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2023 Sean Anderson <seanga2@gmail.com>
 */

#include <common.h>
#include <dm.h>
#include <spl.h>
#include <test/spl.h>
#include <asm/eth.h>
#include <test/ut.h>
#include "../../net/bootp.h"

/*
 * sandbox_eth_bootp_req_to_reply()
 *
 * Check if a BOOTP request was sent. If so, inject a reply
 *
 * returns 0 if injected, -EAGAIN if not
 */
static int sandbox_eth_bootp_req_to_reply(struct udevice *dev, void *packet,
					  unsigned int len)
{
	struct eth_sandbox_priv *priv = dev_get_priv(dev);
	struct ethernet_hdr *eth = packet;
	struct ip_udp_hdr *ip;
	struct bootp_hdr *bp;
	struct ethernet_hdr *eth_recv;
	struct ip_udp_hdr *ipr;
	struct bootp_hdr *bpr;

	if (ntohs(eth->et_protlen) != PROT_IP)
		return -EAGAIN;

	ip = packet + ETHER_HDR_SIZE;
	if (ip->ip_p != IPPROTO_UDP)
		return -EAGAIN;

	if (ntohs(ip->udp_dst) != PORT_BOOTPS)
		return -EAGAIN;

	bp = (void *)ip + IP_UDP_HDR_SIZE;
	if (bp->bp_op != OP_BOOTREQUEST)
		return -EAGAIN;

	/* Don't allow the buffer to overrun */
	if (priv->recv_packets >= PKTBUFSRX)
		return 0;

	/* reply to the request */
	eth_recv = (void *)priv->recv_packet_buffer[priv->recv_packets];
	memcpy(eth_recv, packet, len);
	ipr = (void *)eth_recv + ETHER_HDR_SIZE;
	bpr = (void *)ipr + IP_UDP_HDR_SIZE;
	memcpy(eth_recv->et_dest, eth->et_src, ARP_HLEN);
	memcpy(eth_recv->et_src, priv->fake_host_hwaddr, ARP_HLEN);
	ipr->ip_sum = 0;
	ipr->ip_off = 0;
	net_write_ip(&ipr->ip_dst, net_ip);
	net_write_ip(&ipr->ip_src, priv->fake_host_ipaddr);
	ipr->ip_sum = compute_ip_checksum(ipr, IP_HDR_SIZE);
	ipr->udp_src = ip->udp_dst;
	ipr->udp_dst = ip->udp_src;

	bpr->bp_op = OP_BOOTREPLY;
	net_write_ip(&bpr->bp_yiaddr, net_ip);
	net_write_ip(&bpr->bp_siaddr, priv->fake_host_ipaddr);
	copy_filename(bpr->bp_file, CONFIG_BOOTFILE, sizeof(CONFIG_BOOTFILE));
	memset(&bpr->bp_vend, 0, sizeof(bpr->bp_vend));

	priv->recv_packet_length[priv->recv_packets] = len;
	++priv->recv_packets;

	return 0;
}

struct spl_test_net_priv {
	struct unit_test_state *uts;
	void *img;
	size_t img_size;
	u16 port;
};

/* Well known TFTP port # */
#define TFTP_PORT	69
/* Transaction ID, chosen at random */
#define	TFTP_TID	21313

/*
 *	TFTP operations.
 */
#define TFTP_RRQ	1
#define TFTP_DATA	3
#define TFTP_ACK	4

/* default TFTP block size */
#define TFTP_BLOCK_SIZE		512

struct tftp_hdr {
	u16 opcode;
	u16 block;
};

#define TFTP_HDR_SIZE sizeof(struct tftp_hdr)

/*
 * sandbox_eth_tftp_req_to_reply()
 *
 * Check if a TFTP request was sent. If so, inject a reply. We don't support
 * options, and we don't check for rollover, so we are limited files of less
 * than 32M.
 *
 * returns 0 if injected, -EAGAIN if not
 */
static int sandbox_eth_tftp_req_to_reply(struct udevice *dev, void *packet,
					 unsigned int len)
{
	struct eth_sandbox_priv *priv = dev_get_priv(dev);
	struct spl_test_net_priv *test_priv = priv->priv;
	struct ethernet_hdr *eth = packet;
	struct ip_udp_hdr *ip;
	struct tftp_hdr *tftp;
	struct ethernet_hdr *eth_recv;
	struct ip_udp_hdr *ipr;
	struct tftp_hdr *tftpr;
	size_t size;
	u16 block;

	if (ntohs(eth->et_protlen) != PROT_IP)
		return -EAGAIN;

	ip = packet + ETHER_HDR_SIZE;
	if (ip->ip_p != IPPROTO_UDP)
		return -EAGAIN;

	if (ntohs(ip->udp_dst) == TFTP_PORT) {
		tftp = (void *)ip + IP_UDP_HDR_SIZE;
		if (htons(tftp->opcode) != TFTP_RRQ)
			return -EAGAIN;

		block = 0;
	} else if (ntohs(ip->udp_dst) == TFTP_TID) {
		tftp = (void *)ip + IP_UDP_HDR_SIZE;
		if (htons(tftp->opcode) != TFTP_ACK)
			return -EAGAIN;

		block = htons(tftp->block);
	} else {
		return -EAGAIN;
	}

	if (block * TFTP_BLOCK_SIZE > test_priv->img_size)
		return 0;

	size = min(test_priv->img_size - block * TFTP_BLOCK_SIZE,
		   (size_t)TFTP_BLOCK_SIZE);

	/* Don't allow the buffer to overrun */
	if (priv->recv_packets >= PKTBUFSRX)
		return 0;

	/* reply to the request */
	eth_recv = (void *)priv->recv_packet_buffer[priv->recv_packets];
	memcpy(eth_recv->et_dest, eth->et_src, ARP_HLEN);
	memcpy(eth_recv->et_src, priv->fake_host_hwaddr, ARP_HLEN);
	eth_recv->et_protlen = htons(PROT_IP);

	ipr = (void *)eth_recv + ETHER_HDR_SIZE;
	ipr->ip_hl_v = 0x45;
	ipr->ip_len = htons(IP_UDP_HDR_SIZE + TFTP_HDR_SIZE + size);
	ipr->ip_off = htons(IP_FLAGS_DFRAG);
	ipr->ip_ttl = 255;
	ipr->ip_p = IPPROTO_UDP;
	ipr->ip_sum = 0;
	net_copy_ip(&ipr->ip_dst, &ip->ip_src);
	net_copy_ip(&ipr->ip_src, &ip->ip_dst);
	ipr->ip_sum = compute_ip_checksum(ipr, IP_HDR_SIZE);

	ipr->udp_src = htons(TFTP_TID);
	ipr->udp_dst = ip->udp_src;
	ipr->udp_len = htons(UDP_HDR_SIZE + TFTP_HDR_SIZE + size);
	ipr->udp_xsum = 0;

	tftpr = (void *)ipr + IP_UDP_HDR_SIZE;
	tftpr->opcode = htons(TFTP_DATA);
	tftpr->block = htons(block + 1);
	memcpy((void *)tftpr + TFTP_HDR_SIZE,
	       test_priv->img + block * TFTP_BLOCK_SIZE, size);

	priv->recv_packet_length[priv->recv_packets] =
		ETHER_HDR_SIZE + IP_UDP_HDR_SIZE + TFTP_HDR_SIZE + size;
	++priv->recv_packets;

	return 0;
}

static int spl_net_handler(struct udevice *dev, void *packet,
			   unsigned int len)
{
	struct eth_sandbox_priv *priv = dev_get_priv(dev);
	int old_packets = priv->recv_packets;

	priv->fake_host_ipaddr = string_to_ip("1.1.2.4");
	net_ip = string_to_ip("1.1.2.2");

	sandbox_eth_arp_req_to_reply(dev, packet, len);
	sandbox_eth_bootp_req_to_reply(dev, packet, len);
	sandbox_eth_tftp_req_to_reply(dev, packet, len);

	if (old_packets == priv->recv_packets)
		return 0;

	return 0;
}

static int spl_test_net_write_image(struct unit_test_state *uts, void *img,
				    size_t img_size)
{
	struct spl_test_net_priv *test_priv = malloc(sizeof(*test_priv));

	ut_assertnonnull(test_priv);
	test_priv->uts = uts;
	test_priv->img = img;
	test_priv->img_size = img_size;

	sandbox_eth_set_tx_handler(0, spl_net_handler);
	sandbox_eth_set_priv(0, test_priv);
	return 0;
}

static int spl_test_net(struct unit_test_state *uts, const char *test_name,
			enum spl_test_image type)
{
	struct eth_sandbox_priv *priv;
	struct udevice *dev;
	int ret;

	net_server_ip = string_to_ip("1.1.2.4");
	ret = do_spl_test_load(uts, test_name, type,
			       SPL_LOAD_IMAGE_GET(0, BOOT_DEVICE_CPGMAC,
						  spl_net_load_image_cpgmac),
			       spl_test_net_write_image);

	sandbox_eth_set_tx_handler(0, NULL);
	ut_assertok(uclass_get_device(UCLASS_ETH, 0, &dev));
	priv = dev_get_priv(dev);
	free(priv->priv);
	return ret;
}
SPL_IMG_TEST(spl_test_net, LEGACY, DM_FLAGS);
SPL_IMG_TEST(spl_test_net, LEGACY_LZMA, DM_FLAGS);
SPL_IMG_TEST(spl_test_net, IMX8, DM_FLAGS);
SPL_IMG_TEST(spl_test_net, FIT_INTERNAL, DM_FLAGS);
SPL_IMG_TEST(spl_test_net, FIT_EXTERNAL, DM_FLAGS);
