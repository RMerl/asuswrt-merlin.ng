// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2015 National Instruments
 *
 * (C) Copyright 2015
 * Joe Hershberger <joe.hershberger@ni.com>
 */

#include <asm/eth-raw-os.h>
#include <common.h>
#include <dm.h>
#include <malloc.h>
#include <net.h>

DECLARE_GLOBAL_DATA_PTR;

static int reply_arp;
static struct in_addr arp_ip;

static int sb_eth_raw_start(struct udevice *dev)
{
	struct eth_sandbox_raw_priv *priv = dev_get_priv(dev);
	struct eth_pdata *pdata = dev_get_platdata(dev);
	int ret;

	debug("eth_sandbox_raw: Start\n");

	ret = sandbox_eth_raw_os_start(priv, pdata->enetaddr);
	if (priv->local) {
		env_set("ipaddr", "127.0.0.1");
		env_set("serverip", "127.0.0.1");
		net_ip = string_to_ip("127.0.0.1");
		net_server_ip = net_ip;
	}
	return ret;
}

static int sb_eth_raw_send(struct udevice *dev, void *packet, int length)
{
	struct eth_sandbox_raw_priv *priv = dev_get_priv(dev);

	debug("eth_sandbox_raw: Send packet %d\n", length);

	if (priv->local) {
		struct ethernet_hdr *eth = packet;

		if (ntohs(eth->et_protlen) == PROT_ARP) {
			struct arp_hdr *arp = packet + ETHER_HDR_SIZE;

			/**
			 * localhost works on a higher-level API in Linux than
			 * ARP packets, so fake it
			 */
			arp_ip = net_read_ip(&arp->ar_tpa);
			reply_arp = 1;
			return 0;
		}
		packet += ETHER_HDR_SIZE;
		length -= ETHER_HDR_SIZE;
	}
	return sandbox_eth_raw_os_send(packet, length, priv);
}

static int sb_eth_raw_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct eth_pdata *pdata = dev_get_platdata(dev);
	struct eth_sandbox_raw_priv *priv = dev_get_priv(dev);
	int retval = 0;
	int length;

	if (reply_arp) {
		struct arp_hdr *arp = (void *)net_rx_packets[0] +
			ETHER_HDR_SIZE;

		/*
		 * Fake an ARP response. The u-boot network stack is sending an
		 * ARP request (to find the MAC address to address the actual
		 * packet to) and requires an ARP response to continue. Since
		 * this is the localhost interface, there is no Etherent level
		 * traffic at all, so there is no way to send an ARP request or
		 * to get a response. For this reason we fake the response to
		 * make the u-boot network stack happy.
		 */
		arp->ar_hrd = htons(ARP_ETHER);
		arp->ar_pro = htons(PROT_IP);
		arp->ar_hln = ARP_HLEN;
		arp->ar_pln = ARP_PLEN;
		arp->ar_op = htons(ARPOP_REPLY);
		/* Any non-zero MAC address will work */
		memset(&arp->ar_sha, 0x01, ARP_HLEN);
		/* Use whatever IP we were looking for (always 127.0.0.1?) */
		net_write_ip(&arp->ar_spa, arp_ip);
		memcpy(&arp->ar_tha, pdata->enetaddr, ARP_HLEN);
		net_write_ip(&arp->ar_tpa, net_ip);
		length = ARP_HDR_SIZE;
	} else {
		/* If local, the Ethernet header won't be included; skip it */
		uchar *pktptr = priv->local ?
			net_rx_packets[0] + ETHER_HDR_SIZE : net_rx_packets[0];

		retval = sandbox_eth_raw_os_recv(pktptr, &length, priv);
	}

	if (!retval && length) {
		if (priv->local) {
			struct ethernet_hdr *eth = (void *)net_rx_packets[0];

			/* Fill in enough of the missing Ethernet header */
			memcpy(eth->et_dest, pdata->enetaddr, ARP_HLEN);
			memset(eth->et_src, 0x01, ARP_HLEN);
			eth->et_protlen = htons(reply_arp ? PROT_ARP : PROT_IP);
			reply_arp = 0;
			length += ETHER_HDR_SIZE;
		}

		debug("eth_sandbox_raw: received packet %d\n",
		      length);
		*packetp = net_rx_packets[0];
		return length;
	}
	return retval;
}

static void sb_eth_raw_stop(struct udevice *dev)
{
	struct eth_sandbox_raw_priv *priv = dev_get_priv(dev);

	debug("eth_sandbox_raw: Stop\n");

	sandbox_eth_raw_os_stop(priv);
}

static int sb_eth_raw_read_rom_hwaddr(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_platdata(dev);

	net_random_ethaddr(pdata->enetaddr);

	return 0;
}

static const struct eth_ops sb_eth_raw_ops = {
	.start			= sb_eth_raw_start,
	.send			= sb_eth_raw_send,
	.recv			= sb_eth_raw_recv,
	.stop			= sb_eth_raw_stop,
	.read_rom_hwaddr	= sb_eth_raw_read_rom_hwaddr,
};

static int sb_eth_raw_ofdata_to_platdata(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_platdata(dev);
	struct eth_sandbox_raw_priv *priv = dev_get_priv(dev);
	const char *ifname;
	int ret;

	pdata->iobase = dev_read_addr(dev);

	ifname = dev_read_string(dev, "host-raw-interface");
	if (ifname) {
		strncpy(priv->host_ifname, ifname, IFNAMSIZ);
		printf(": Using %s from DT\n", priv->host_ifname);
	}
	if (dev_read_u32(dev, "host-raw-interface-idx",
			 &priv->host_ifindex) < 0) {
		priv->host_ifindex = 0;
	} else {
		ret = sandbox_eth_raw_os_idx_to_name(priv);
		if (ret < 0)
			return ret;
		printf(": Using interface index %d from DT (%s)\n",
		       priv->host_ifindex, priv->host_ifname);
	}

	ret = sandbox_eth_raw_os_is_local(priv->host_ifname);
	if (ret < 0)
		return ret;
	priv->local = ret;

	return 0;
}

static const struct udevice_id sb_eth_raw_ids[] = {
	{ .compatible = "sandbox,eth-raw" },
	{ }
};

U_BOOT_DRIVER(eth_sandbox_raw) = {
	.name	= "eth_sandbox_raw",
	.id	= UCLASS_ETH,
	.of_match = sb_eth_raw_ids,
	.ofdata_to_platdata = sb_eth_raw_ofdata_to_platdata,
	.ops	= &sb_eth_raw_ops,
	.priv_auto_alloc_size = sizeof(struct eth_sandbox_raw_priv),
	.platdata_auto_alloc_size = sizeof(struct eth_pdata),
};
