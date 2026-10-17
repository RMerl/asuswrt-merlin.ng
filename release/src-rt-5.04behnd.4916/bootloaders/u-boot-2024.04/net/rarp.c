// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <common.h>
#include <command.h>
#include <log.h>
#include <net.h>
#include <net/tftp.h>
#include "nfs.h"
#include "bootp.h"
#include "rarp.h"

#define TIMEOUT 5000UL /* Milliseconds before trying BOOTP again */

int rarp_try;

/*
 *	Handle a RARP received packet.
 */
void rarp_receive(struct ip_udp_hdr *ip, unsigned len)
{
	struct arp_hdr *arp;

	debug_cond(DEBUG_NET_PKT, "Got RARP\n");
	arp = (struct arp_hdr *)ip;
	if (len < ARP_HDR_SIZE) {
		printf("bad length %d < %d\n", len, ARP_HDR_SIZE);
		return;
	}

	if ((ntohs(arp->ar_op) != RARPOP_REPLY) ||
	    (ntohs(arp->ar_hrd) != ARP_ETHER)   ||
	    (ntohs(arp->ar_pro) != PROT_IP)     ||
	    (arp->ar_hln != 6) || (arp->ar_pln != 4)) {
		puts("invalid RARP header\n");
	} else {
		net_copy_ip(&net_ip, &arp->ar_data[16]);
		if (net_server_ip.s_addr == 0)
			net_copy_ip(&net_server_ip, &arp->ar_data[6]);
		memcpy(net_server_ethaddr, &arp->ar_data[0], 6);
		debug_cond(DEBUG_DEV_PKT, "Got good RARP\n");
		net_auto_load();
	}
}


/*
 *	Timeout on BOOTP request.
 */
static void rarp_timeout_handler(void)
{
	if (rarp_try >= CONFIG_NET_RETRY_COUNT) {
		puts("\nRetry count exceeded; starting again\n");
		net_start_again();
	} else {
		net_set_timeout_handler(TIMEOUT, rarp_timeout_handler);
		rarp_request();
	}
}


void rarp_request(void)
{
	uchar *pkt;
	struct arp_hdr *rarp;
	int eth_hdr_size;

	printf("RARP broadcast %d\n", ++rarp_try);
	pkt = net_tx_packet;

	eth_hdr_size = net_set_ether(pkt, net_bcast_ethaddr, PROT_RARP);
	pkt += eth_hdr_size;

	rarp = (struct arp_hdr *)pkt;

	rarp->ar_hrd = htons(ARP_ETHER);
	rarp->ar_pro = htons(PROT_IP);
	rarp->ar_hln = 6;
	rarp->ar_pln = 4;
	rarp->ar_op  = htons(RARPOP_REQUEST);
	memcpy(&rarp->ar_data[0],  net_ethaddr, 6);	/* source ET addr */
	memcpy(&rarp->ar_data[6],  &net_ip,   4);	/* source IP addr */
	/* dest ET addr = source ET addr ??*/
	memcpy(&rarp->ar_data[10], net_ethaddr, 6);
	/* dest IP addr set to broadcast */
	memset(&rarp->ar_data[16], 0xff,        4);

	net_send_packet(net_tx_packet, eth_hdr_size + ARP_HDR_SIZE);

	net_set_timeout_handler(TIMEOUT, rarp_timeout_handler);
}
