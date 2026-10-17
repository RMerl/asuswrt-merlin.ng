// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2015 National Instruments
 *
 * (C) Copyright 2015
 * Joe Hershberger <joe.hershberger@ni.com>
 */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <malloc.h>
#include <net.h>
#include <asm/eth.h>
#include <asm/global_data.h>
#include <asm/test.h>

DECLARE_GLOBAL_DATA_PTR;

static bool skip_timeout;

/*
 * sandbox_eth_disable_response()
 *
 * index - The alias index (also DM seq number)
 * disable - If non-zero, ignore sent packets and don't send mock response
 */
void sandbox_eth_disable_response(int index, bool disable)
{
	struct udevice *dev;
	struct eth_sandbox_priv *priv;
	int ret;

	ret = uclass_get_device(UCLASS_ETH, index, &dev);
	if (ret)
		return;

	priv = dev_get_priv(dev);
	priv->disabled = disable;
}

/*
 * sandbox_eth_skip_timeout()
 *
 * When the first packet read is attempted, fast-forward time
 */
void sandbox_eth_skip_timeout(void)
{
	skip_timeout = true;
}

/*
 * sandbox_eth_arp_req_to_reply()
 *
 * Check for an arp request to be sent. If so, inject a reply
 *
 * returns 0 if injected, -EAGAIN if not
 */
int sandbox_eth_arp_req_to_reply(struct udevice *dev, void *packet,
				 unsigned int len)
{
	struct eth_sandbox_priv *priv = dev_get_priv(dev);
	struct ethernet_hdr *eth = packet;
	struct arp_hdr *arp;
	struct ethernet_hdr *eth_recv;
	struct arp_hdr *arp_recv;

	if (ntohs(eth->et_protlen) != PROT_ARP)
		return -EAGAIN;

	arp = packet + ETHER_HDR_SIZE;

	if (ntohs(arp->ar_op) != ARPOP_REQUEST)
		return -EAGAIN;

	/* Don't allow the buffer to overrun */
	if (priv->recv_packets >= PKTBUFSRX)
		return 0;

	/* store this as the assumed IP of the fake host */
	priv->fake_host_ipaddr = net_read_ip(&arp->ar_tpa);

	/* Formulate a fake response */
	eth_recv = (void *)priv->recv_packet_buffer[priv->recv_packets];
	memcpy(eth_recv->et_dest, eth->et_src, ARP_HLEN);
	memcpy(eth_recv->et_src, priv->fake_host_hwaddr, ARP_HLEN);
	eth_recv->et_protlen = htons(PROT_ARP);

	arp_recv = (void *)eth_recv + ETHER_HDR_SIZE;
	arp_recv->ar_hrd = htons(ARP_ETHER);
	arp_recv->ar_pro = htons(PROT_IP);
	arp_recv->ar_hln = ARP_HLEN;
	arp_recv->ar_pln = ARP_PLEN;
	arp_recv->ar_op = htons(ARPOP_REPLY);
	memcpy(&arp_recv->ar_sha, priv->fake_host_hwaddr, ARP_HLEN);
	net_write_ip(&arp_recv->ar_spa, priv->fake_host_ipaddr);
	memcpy(&arp_recv->ar_tha, &arp->ar_sha, ARP_HLEN);
	net_copy_ip(&arp_recv->ar_tpa, &arp->ar_spa);

	priv->recv_packet_length[priv->recv_packets] =
		ETHER_HDR_SIZE + ARP_HDR_SIZE;
	++priv->recv_packets;

	return 0;
}

/*
 * sandbox_eth_ping_req_to_reply()
 *
 * Check for a ping request to be sent. If so, inject a reply
 *
 * returns 0 if injected, -EAGAIN if not
 */
int sandbox_eth_ping_req_to_reply(struct udevice *dev, void *packet,
				  unsigned int len)
{
	struct eth_sandbox_priv *priv = dev_get_priv(dev);
	struct ethernet_hdr *eth = packet;
	struct ip_udp_hdr *ip;
	struct icmp_hdr *icmp;
	struct ethernet_hdr *eth_recv;
	struct ip_udp_hdr *ipr;
	struct icmp_hdr *icmpr;

	if (ntohs(eth->et_protlen) != PROT_IP)
		return -EAGAIN;

	ip = packet + ETHER_HDR_SIZE;

	if (ip->ip_p != IPPROTO_ICMP)
		return -EAGAIN;

	icmp = (struct icmp_hdr *)&ip->udp_src;

	if (icmp->type != ICMP_ECHO_REQUEST)
		return -EAGAIN;

	/* Don't allow the buffer to overrun */
	if (priv->recv_packets >= PKTBUFSRX)
		return 0;

	/* reply to the ping */
	eth_recv = (void *)priv->recv_packet_buffer[priv->recv_packets];
	memcpy(eth_recv, packet, len);
	ipr = (void *)eth_recv + ETHER_HDR_SIZE;
	icmpr = (struct icmp_hdr *)&ipr->udp_src;
	memcpy(eth_recv->et_dest, eth->et_src, ARP_HLEN);
	memcpy(eth_recv->et_src, priv->fake_host_hwaddr, ARP_HLEN);
	ipr->ip_sum = 0;
	ipr->ip_off = 0;
	net_copy_ip((void *)&ipr->ip_dst, &ip->ip_src);
	net_write_ip((void *)&ipr->ip_src, priv->fake_host_ipaddr);
	ipr->ip_sum = compute_ip_checksum(ipr, IP_HDR_SIZE);

	icmpr->type = ICMP_ECHO_REPLY;
	icmpr->checksum = 0;
	icmpr->checksum = compute_ip_checksum(icmpr, ICMP_HDR_SIZE);

	priv->recv_packet_length[priv->recv_packets] = len;
	++priv->recv_packets;

	return 0;
}

/*
 * sandbox_eth_recv_arp_req()
 *
 * Inject an ARP request for this target
 *
 * returns 0 if injected, -EOVERFLOW if not
 */
int sandbox_eth_recv_arp_req(struct udevice *dev)
{
	struct eth_sandbox_priv *priv = dev_get_priv(dev);
	struct ethernet_hdr *eth_recv;
	struct arp_hdr *arp_recv;

	/* Don't allow the buffer to overrun */
	if (priv->recv_packets >= PKTBUFSRX)
		return -EOVERFLOW;

	/* Formulate a fake request */
	eth_recv = (void *)priv->recv_packet_buffer[priv->recv_packets];
	memcpy(eth_recv->et_dest, net_bcast_ethaddr, ARP_HLEN);
	memcpy(eth_recv->et_src, priv->fake_host_hwaddr, ARP_HLEN);
	eth_recv->et_protlen = htons(PROT_ARP);

	arp_recv = (void *)eth_recv + ETHER_HDR_SIZE;
	arp_recv->ar_hrd = htons(ARP_ETHER);
	arp_recv->ar_pro = htons(PROT_IP);
	arp_recv->ar_hln = ARP_HLEN;
	arp_recv->ar_pln = ARP_PLEN;
	arp_recv->ar_op = htons(ARPOP_REQUEST);
	memcpy(&arp_recv->ar_sha, priv->fake_host_hwaddr, ARP_HLEN);
	net_write_ip(&arp_recv->ar_spa, priv->fake_host_ipaddr);
	memcpy(&arp_recv->ar_tha, net_null_ethaddr, ARP_HLEN);
	net_write_ip(&arp_recv->ar_tpa, net_ip);

	priv->recv_packet_length[priv->recv_packets] =
		ETHER_HDR_SIZE + ARP_HDR_SIZE;
	++priv->recv_packets;

	return 0;
}

/*
 * sandbox_eth_recv_ping_req()
 *
 * Inject a ping request for this target
 *
 * returns 0 if injected, -EOVERFLOW if not
 */
int sandbox_eth_recv_ping_req(struct udevice *dev)
{
	struct eth_sandbox_priv *priv = dev_get_priv(dev);
	struct ethernet_hdr *eth_recv;
	struct ip_udp_hdr *ipr;
	struct icmp_hdr *icmpr;

	/* Don't allow the buffer to overrun */
	if (priv->recv_packets >= PKTBUFSRX)
		return -EOVERFLOW;

	/* Formulate a fake ping */
	eth_recv = (void *)priv->recv_packet_buffer[priv->recv_packets];

	memcpy(eth_recv->et_dest, net_ethaddr, ARP_HLEN);
	memcpy(eth_recv->et_src, priv->fake_host_hwaddr, ARP_HLEN);
	eth_recv->et_protlen = htons(PROT_IP);

	ipr = (void *)eth_recv + ETHER_HDR_SIZE;
	ipr->ip_hl_v = 0x45;
	ipr->ip_len = htons(IP_ICMP_HDR_SIZE);
	ipr->ip_off = htons(IP_FLAGS_DFRAG);
	ipr->ip_p = IPPROTO_ICMP;
	ipr->ip_sum = 0;
	net_write_ip(&ipr->ip_src, priv->fake_host_ipaddr);
	net_write_ip(&ipr->ip_dst, net_ip);
	ipr->ip_sum = compute_ip_checksum(ipr, IP_HDR_SIZE);

	icmpr = (struct icmp_hdr *)&ipr->udp_src;

	icmpr->type = ICMP_ECHO_REQUEST;
	icmpr->code = 0;
	icmpr->checksum = 0;
	icmpr->un.echo.id = 0;
	icmpr->un.echo.sequence = htons(1);
	icmpr->checksum = compute_ip_checksum(icmpr, ICMP_HDR_SIZE);

	priv->recv_packet_length[priv->recv_packets] =
		ETHER_HDR_SIZE + IP_ICMP_HDR_SIZE;
	++priv->recv_packets;

	return 0;
}

/*
 * sb_default_handler()
 *
 * perform typical responses to simple ping
 *
 * dev - device pointer
 * pkt - "sent" packet buffer
 * len - length of packet
 */
static int sb_default_handler(struct udevice *dev, void *packet,
			      unsigned int len)
{
	if (!sandbox_eth_arp_req_to_reply(dev, packet, len))
		return 0;
	if (!sandbox_eth_ping_req_to_reply(dev, packet, len))
		return 0;

	return 0;
}

/*
 * sandbox_eth_set_tx_handler()
 *
 * Set a custom response to a packet being sent through the sandbox eth test
 *	driver
 *
 * index - interface to set the handler for
 * handler - The func ptr to call on send. If NULL, set to default handler
 */
void sandbox_eth_set_tx_handler(int index, sandbox_eth_tx_hand_f *handler)
{
	struct udevice *dev;
	struct eth_sandbox_priv *priv;
	int ret;

	ret = uclass_get_device(UCLASS_ETH, index, &dev);
	if (ret)
		return;

	priv = dev_get_priv(dev);
	if (handler)
		priv->tx_handler = handler;
	else
		priv->tx_handler = sb_default_handler;
}

/*
 * Set priv ptr
 *
 * priv - priv void ptr to store in the device
 */
void sandbox_eth_set_priv(int index, void *priv)
{
	struct udevice *dev;
	struct eth_sandbox_priv *dev_priv;
	int ret;

	ret = uclass_get_device(UCLASS_ETH, index, &dev);
	if (ret)
		return;

	dev_priv = dev_get_priv(dev);

	dev_priv->priv = priv;
}

static int sb_eth_start(struct udevice *dev)
{
	struct eth_sandbox_priv *priv = dev_get_priv(dev);

	debug("eth_sandbox: Start\n");

	priv->recv_packets = 0;
	for (int i = 0; i < PKTBUFSRX; i++) {
		priv->recv_packet_buffer[i] = net_rx_packets[i];
		priv->recv_packet_length[i] = 0;
	}

	return 0;
}

static int sb_eth_send(struct udevice *dev, void *packet, int length)
{
	struct eth_sandbox_priv *priv = dev_get_priv(dev);

	debug("eth_sandbox: Send packet %d\n", length);

	if (priv->disabled)
		return 0;

	return priv->tx_handler(dev, packet, length);
}

static int sb_eth_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct eth_sandbox_priv *priv = dev_get_priv(dev);

	if (skip_timeout) {
		timer_test_add_offset(11000UL);
		skip_timeout = false;
	}

	if (priv->recv_packets) {
		int lcl_recv_packet_length = priv->recv_packet_length[0];

		debug("eth_sandbox: received packet[%d], %d waiting\n",
		      lcl_recv_packet_length, priv->recv_packets - 1);
		*packetp = priv->recv_packet_buffer[0];
		return lcl_recv_packet_length;
	}
	return 0;
}

static int sb_eth_free_pkt(struct udevice *dev, uchar *packet, int length)
{
	struct eth_sandbox_priv *priv = dev_get_priv(dev);
	int i;

	if (!priv->recv_packets)
		return 0;

	--priv->recv_packets;
	for (i = 0; i < priv->recv_packets; i++) {
		priv->recv_packet_length[i] = priv->recv_packet_length[i + 1];
		memcpy(priv->recv_packet_buffer[i],
		       priv->recv_packet_buffer[i + 1],
		       priv->recv_packet_length[i + 1]);
	}
	priv->recv_packet_length[priv->recv_packets] = 0;

	return 0;
}

static void sb_eth_stop(struct udevice *dev)
{
	debug("eth_sandbox: Stop\n");
}

static int sb_eth_write_hwaddr(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct eth_sandbox_priv *priv = dev_get_priv(dev);

	debug("eth_sandbox %s: Write HW ADDR - %pM\n", dev->name,
	      pdata->enetaddr);
	memcpy(priv->fake_host_hwaddr, pdata->enetaddr, ARP_HLEN);
	return 0;
}

static const struct eth_ops sb_eth_ops = {
	.start			= sb_eth_start,
	.send			= sb_eth_send,
	.recv			= sb_eth_recv,
	.free_pkt		= sb_eth_free_pkt,
	.stop			= sb_eth_stop,
	.write_hwaddr		= sb_eth_write_hwaddr,
};

static int sb_eth_remove(struct udevice *dev)
{
	return 0;
}

static int sb_eth_of_to_plat(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct eth_sandbox_priv *priv = dev_get_priv(dev);

	pdata->iobase = dev_read_addr(dev);
	priv->disabled = false;
	priv->tx_handler = sb_default_handler;

	return 0;
}

static const struct udevice_id sb_eth_ids[] = {
	{ .compatible = "sandbox,eth" },
	{ }
};

U_BOOT_DRIVER(eth_sandbox) = {
	.name	= "eth_sandbox",
	.id	= UCLASS_ETH,
	.of_match = sb_eth_ids,
	.of_to_plat = sb_eth_of_to_plat,
	.remove	= sb_eth_remove,
	.ops	= &sb_eth_ops,
	.priv_auto	= sizeof(struct eth_sandbox_priv),
	.plat_auto	= sizeof(struct eth_pdata),
};
