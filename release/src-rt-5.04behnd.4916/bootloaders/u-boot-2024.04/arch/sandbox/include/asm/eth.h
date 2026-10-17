/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2015 National Instruments
 *
 * (C) Copyright 2015
 * Joe Hershberger <joe.hershberger@ni.com>
 */

#ifndef __ETH_H
#define __ETH_H

#include <net.h>

void sandbox_eth_disable_response(int index, bool disable);

void sandbox_eth_skip_timeout(void);

/*
 * sandbox_eth_arp_req_to_reply()
 *
 * Check for an arp request to be sent. If so, inject a reply
 *
 * @dev: device that received the packet
 * @packet: pointer to the received pacaket buffer
 * @len: length of received packet
 * Return: 0 if injected, -EAGAIN if not
 */
int sandbox_eth_arp_req_to_reply(struct udevice *dev, void *packet,
				 unsigned int len);

/*
 * sandbox_eth_ping_req_to_reply()
 *
 * Check for a ping request to be sent. If so, inject a reply
 *
 * @dev: device that received the packet
 * @packet: pointer to the received pacaket buffer
 * @len: length of received packet
 * Return: 0 if injected, -EAGAIN if not
 */
int sandbox_eth_ping_req_to_reply(struct udevice *dev, void *packet,
				  unsigned int len);

/*
 * sandbox_eth_recv_arp_req()
 *
 * Inject an ARP request for this target
 *
 * @dev: device that received the packet
 * Return: 0 if injected, -EOVERFLOW if not
 */
int sandbox_eth_recv_arp_req(struct udevice *dev);

/*
 * sandbox_eth_recv_ping_req()
 *
 * Inject a ping request for this target
 *
 * @dev: device that received the packet
 * Return: 0 if injected, -EOVERFLOW if not
 */
int sandbox_eth_recv_ping_req(struct udevice *dev);

/**
 * A packet handler
 *
 * dev - device pointer
 * pkt - pointer to the "sent" packet
 * len - packet length
 */
typedef int sandbox_eth_tx_hand_f(struct udevice *dev, void *pkt,
				   unsigned int len);

/**
 * struct eth_sandbox_priv - memory for sandbox mock driver
 *
 * fake_host_hwaddr - MAC address of mocked machine
 * fake_host_ipaddr - IP address of mocked machine
 * disabled - Will not respond
 * recv_packet_buffer - buffers of the packet returned as received
 * recv_packet_length - lengths of the packet returned as received
 * recv_packets - number of packets returned
 * tx_handler - function to generate responses to sent packets
 * priv - a pointer to some structure a test may want to keep track of
 */
struct eth_sandbox_priv {
	uchar fake_host_hwaddr[ARP_HLEN];
	struct in_addr fake_host_ipaddr;
	bool disabled;
	uchar * recv_packet_buffer[PKTBUFSRX];
	int recv_packet_length[PKTBUFSRX];
	int recv_packets;
	sandbox_eth_tx_hand_f *tx_handler;
	void *priv;
};

/*
 * Set packet handler
 *
 * handler - The func ptr to call on send. If NULL, set to default handler
 */
void sandbox_eth_set_tx_handler(int index, sandbox_eth_tx_hand_f *handler);

/*
 * Set priv ptr
 *
 * priv - priv void ptr to store in the device
 */
void sandbox_eth_set_priv(int index, void *priv);

#endif /* __ETH_H */
