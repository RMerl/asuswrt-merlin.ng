// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) Microsoft Corporation
 * Author: Sean Edmond <seanedmond@microsoft.com>
 *
 */

/* Simple DHCP6 network layer implementation. */

#include <common.h>
#include <net6.h>
#include <malloc.h>
#include <linux/delay.h>
#include "net_rand.h"
#include "dhcpv6.h"

#define PORT_DHCP6_S	547	/* DHCP6 server UDP port */
#define PORT_DHCP6_C	546	/* DHCP6 client UDP port */

/* default timeout parameters (in ms) */
#define SOL_MAX_DELAY_MS	1000
#define SOL_TIMEOUT_MS		1000
#define SOL_MAX_RT_MS		3600000
#define REQ_TIMEOUT_MS		1000
#define REQ_MAX_RT_MS		30000
#define REQ_MAX_RC		10
#define MAX_WAIT_TIME_MS	60000

/* global variable to track any updates from DHCP6 server */
int updated_sol_max_rt_ms = SOL_MAX_RT_MS;
/* state machine parameters/variables */
struct dhcp6_sm_params sm_params;

static void dhcp6_state_machine(bool timeout, uchar *rx_pkt, unsigned int len);

/* Handle DHCP received packets (set as UDP handler) */
static void dhcp6_handler(uchar *pkt, unsigned int dest, struct in_addr sip,
			  unsigned int src, unsigned int len)
{
	/* return if ports don't match DHCPv6 ports */
	if (dest != PORT_DHCP6_C || src != PORT_DHCP6_S)
		return;

	dhcp6_state_machine(false, pkt, len);
}

/**
 * dhcp6_add_option() - Adds DHCP6 option to a packet
 * @option_id: The option ID to add (See DHCP6_OPTION_* definitions)
 * @pkt: A pointer to the current write location of the TX packet
 *
 * Return: The number of bytes written into "*pkt"
 */
static int dhcp6_add_option(int option_id, uchar *pkt)
{
	struct dhcp6_option_duid_ll *duid_opt;
	struct dhcp6_option_elapsed_time *elapsed_time_opt;
	struct dhcp6_option_ia_ta *ia_ta_opt;
	struct dhcp6_option_ia_na *ia_na_opt;
	struct dhcp6_option_oro *oro_opt;
	struct dhcp6_option_client_arch *client_arch_opt;
	struct dhcp6_option_vendor_class *vendor_class_opt;
	int opt_len;
	long elapsed_time;
	size_t vci_strlen;
	int num_oro = 0;
	int num_client_arch = 0;
	int num_vc_data = 0;
	struct dhcp6_option_hdr *dhcp_option = (struct dhcp6_option_hdr *)pkt;
	uchar *dhcp_option_start = pkt + sizeof(struct dhcp6_option_hdr);

	dhcp_option->option_id = htons(option_id);

	switch (option_id) {
	case DHCP6_OPTION_CLIENTID:
		/* Only support for DUID-LL in Client ID option for now */
		duid_opt = (struct dhcp6_option_duid_ll *)dhcp_option_start;
		duid_opt->duid_type = htons(DUID_TYPE_LL);
		duid_opt->hw_type = htons(DUID_HW_TYPE_ENET);
		memcpy(duid_opt->ll_addr, net_ethaddr, ETH_ALEN);
		opt_len = sizeof(struct dhcp6_option_duid_ll) + ETH_ALEN;

		/* Save DUID for comparison later */
		memcpy(sm_params.duid, duid_opt, opt_len);
		break;
	case DHCP6_OPTION_ELAPSED_TIME:
		/* calculate elapsed time in 1/100th of a second */
		elapsed_time = (sm_params.dhcp6_retry_ms -
			sm_params.dhcp6_start_ms) / 10;
		if (elapsed_time > 0xFFFF)
			elapsed_time = 0xFFFF;

		elapsed_time_opt = (struct dhcp6_option_elapsed_time *)dhcp_option_start;
		elapsed_time_opt->elapsed_time = htons(elapsed_time);

		opt_len = sizeof(struct dhcp6_option_elapsed_time);
		break;
	case DHCP6_OPTION_IA_TA:
		ia_ta_opt = (struct dhcp6_option_ia_ta *)dhcp_option_start;
		ia_ta_opt->iaid = htonl(sm_params.ia_id);

		opt_len = sizeof(struct dhcp6_option_ia_ta);
		break;
	case DHCP6_OPTION_IA_NA:
		ia_na_opt = (struct dhcp6_option_ia_na *)dhcp_option_start;
		ia_na_opt->iaid = htonl(sm_params.ia_id);
		/* In a message sent by a client to a server,
		 * the T1 and T2 fields SHOULD be set to 0
		 */
		ia_na_opt->t1 = 0;
		ia_na_opt->t2 = 0;

		opt_len = sizeof(struct dhcp6_option_ia_na);
		break;
	case DHCP6_OPTION_ORO:
		oro_opt = (struct dhcp6_option_oro *)dhcp_option_start;
		oro_opt->req_option_code[num_oro++] = htons(DHCP6_OPTION_OPT_BOOTFILE_URL);
		oro_opt->req_option_code[num_oro++] = htons(DHCP6_OPTION_SOL_MAX_RT);
		if (IS_ENABLED(CONFIG_DHCP6_PXE_DHCP_OPTION)) {
			oro_opt->req_option_code[num_oro++] =
				htons(DHCP6_OPTION_OPT_BOOTFILE_PARAM);
		}

		opt_len = sizeof(__be16) * num_oro;
		break;
	case DHCP6_OPTION_CLIENT_ARCH_TYPE:
		client_arch_opt = (struct dhcp6_option_client_arch *)dhcp_option_start;
		client_arch_opt->arch_type[num_client_arch++] = htons(CONFIG_DHCP6_PXE_CLIENTARCH);

		opt_len = sizeof(__be16) * num_client_arch;
		break;
	case DHCP6_OPTION_VENDOR_CLASS:
		vendor_class_opt = (struct dhcp6_option_vendor_class *)dhcp_option_start;
		vendor_class_opt->enterprise_number = htonl(CONFIG_DHCP6_ENTERPRISE_ID);

		vci_strlen = strlen(DHCP6_VCI_STRING);
		vendor_class_opt->vendor_class_data[num_vc_data].vendor_class_len =
			htons(vci_strlen);
		memcpy(vendor_class_opt->vendor_class_data[num_vc_data].opaque_data,
		       DHCP6_VCI_STRING, vci_strlen);
		num_vc_data++;

		opt_len = sizeof(struct dhcp6_option_vendor_class) +
			  sizeof(struct vendor_class_data) * num_vc_data +
			  vci_strlen;
		break;
	case DHCP6_OPTION_NII:
		dhcp_option_start[0] = 1;
		dhcp_option_start[1] = 0;
		dhcp_option_start[2] = 0;

		opt_len = 3;
		break;
	default:
		printf("***Warning unknown DHCP6 option %d.  Not adding to message\n", option_id);
		return 0;
	}
	dhcp_option->option_len = htons(opt_len);

	return opt_len + sizeof(struct dhcp6_option_hdr);
}

/**
 * dhcp6_send_solicit_packet() - Send a SOLICIT packet
 *
 * Implements RFC 8415:
 *    - 16.2. Solicit Message
 *    - 18.2.1. Creation and Transmission of Solicit Messages
 *
 * Adds DHCP6 header and DHCP6 options.  Sends the UDP packet
 * and sets the UDP handler.
 */
static void dhcp6_send_solicit_packet(void)
{
	struct in6_addr dhcp_bcast_ip6;
	int len = 0;
	uchar *pkt;
	uchar *dhcp_pkt_start_ptr;
	struct dhcp6_hdr *dhcp_hdr;

	pkt = net_tx_packet + net_eth_hdr_size() + IP6_HDR_SIZE + UDP_HDR_SIZE;
	dhcp_pkt_start_ptr = pkt;

	/* Add the DHCP6 header */
	dhcp_hdr = (struct dhcp6_hdr *)pkt;
	dhcp_hdr->msg_type = DHCP6_MSG_SOLICIT;
	dhcp_hdr->trans_id = htons(sm_params.trans_id);
	pkt += sizeof(struct dhcp6_hdr);

	/* Add the options */
	pkt += dhcp6_add_option(DHCP6_OPTION_CLIENTID, pkt);
	pkt += dhcp6_add_option(DHCP6_OPTION_ELAPSED_TIME, pkt);
	pkt += dhcp6_add_option(DHCP6_OPTION_IA_NA, pkt);
	pkt += dhcp6_add_option(DHCP6_OPTION_ORO, pkt);
	if (CONFIG_DHCP6_PXE_CLIENTARCH != 0xFF)
		pkt += dhcp6_add_option(DHCP6_OPTION_CLIENT_ARCH_TYPE, pkt);
	pkt += dhcp6_add_option(DHCP6_OPTION_VENDOR_CLASS, pkt);
	pkt += dhcp6_add_option(DHCP6_OPTION_NII, pkt);

	/* calculate packet length */
	len = pkt - dhcp_pkt_start_ptr;

	/* send UDP packet to DHCP6 multicast address */
	string_to_ip6(DHCP6_MULTICAST_ADDR, sizeof(DHCP6_MULTICAST_ADDR), &dhcp_bcast_ip6);
	net_set_udp_handler(dhcp6_handler);
	net_send_udp_packet6((uchar *)net_bcast_ethaddr, &dhcp_bcast_ip6,
			     PORT_DHCP6_S, PORT_DHCP6_C, len);
}

/**
 * dhcp6_send_request_packet() - Send a REQUEST packet
 *
 *  * Implements RFC 8415:
 *    - 16.4. Request Message
 *    - 18.2.2. Creation and Transmission of Request Messages
 *
 * Adds DHCP6 header and DHCP6 options.  Sends the UDP packet
 * and sets the UDP handler.
 */
static void dhcp6_send_request_packet(void)
{
	struct in6_addr dhcp_bcast_ip6;
	int len = 0;
	uchar *pkt;
	uchar *dhcp_pkt_start_ptr;
	struct dhcp6_hdr *dhcp_hdr;

	pkt = net_tx_packet + net_eth_hdr_size() + IP6_HDR_SIZE + UDP_HDR_SIZE;
	dhcp_pkt_start_ptr = pkt;

	/* Add the DHCP6 header */
	dhcp_hdr = (struct dhcp6_hdr *)pkt;
	dhcp_hdr->msg_type = DHCP6_MSG_REQUEST;
	dhcp_hdr->trans_id = htons(sm_params.trans_id);
	pkt += sizeof(struct dhcp6_hdr);

	/* add the options */
	pkt += dhcp6_add_option(DHCP6_OPTION_CLIENTID, pkt);
	pkt += dhcp6_add_option(DHCP6_OPTION_ELAPSED_TIME, pkt);
	pkt += dhcp6_add_option(DHCP6_OPTION_IA_NA, pkt);
	pkt += dhcp6_add_option(DHCP6_OPTION_ORO, pkt);
	/* copy received IA_TA/IA_NA into the REQUEST packet */
	if (sm_params.server_uid.uid_ptr) {
		memcpy(pkt, sm_params.server_uid.uid_ptr, sm_params.server_uid.uid_size);
		pkt += sm_params.server_uid.uid_size;
	}
	if (CONFIG_DHCP6_PXE_CLIENTARCH != 0xFF)
		pkt += dhcp6_add_option(DHCP6_OPTION_CLIENT_ARCH_TYPE, pkt);
	pkt += dhcp6_add_option(DHCP6_OPTION_VENDOR_CLASS, pkt);
	pkt += dhcp6_add_option(DHCP6_OPTION_NII, pkt);

	/* calculate packet length */
	len = pkt - dhcp_pkt_start_ptr;

	/* send UDP packet to DHCP6 multicast address */
	string_to_ip6(DHCP6_MULTICAST_ADDR, strlen(DHCP6_MULTICAST_ADDR), &dhcp_bcast_ip6);
	net_set_udp_handler(dhcp6_handler);
	net_send_udp_packet6((uchar *)net_bcast_ethaddr, &dhcp_bcast_ip6,
			     PORT_DHCP6_S, PORT_DHCP6_C, len);
}

static void dhcp6_parse_ia_options(struct dhcp6_option_hdr *ia_ptr, uchar *ia_option_ptr)
{
	struct dhcp6_option_hdr *ia_option_hdr;

	ia_option_hdr = (struct dhcp6_option_hdr *)ia_option_ptr;

	/* Search for options encapsulated in IA_NA/IA_TA (DHCP6_OPTION_IAADDR
	 * or DHCP6_OPTION_STATUS_CODE)
	 */
	while (ia_option_ptr < ((uchar *)ia_ptr + ntohs(ia_ptr->option_len))) {
		switch (ntohs(ia_option_hdr->option_id)) {
		case DHCP6_OPTION_IAADDR:
			sm_params.rx_status.ia_addr_found = true;
			net_copy_ip6(&sm_params.rx_status.ia_addr_ipv6,
				     (ia_option_ptr + sizeof(struct dhcp6_hdr)));
			debug("DHCP6_OPTION_IAADDR FOUND\n");
			break;
		case DHCP6_OPTION_STATUS_CODE:
			sm_params.rx_status.ia_status_code =
				ntohs(*((u16 *)(ia_option_ptr + sizeof(struct dhcp6_hdr))));
			printf("ERROR : IA STATUS %d\n", sm_params.rx_status.ia_status_code);
			break;
		default:
			debug("Unknown Option in IA, skipping\n");
			break;
		}

		ia_option_ptr += ntohs(((struct dhcp6_option_hdr *)ia_option_ptr)->option_len);
	}
}

/**
 * dhcp6_parse_options() - Parse the DHCP6 options
 *
 * @rx_pkt: pointer to beginning of received DHCP6 packet
 * @len: Total length of the DHCP6 packet
 *
 * Parses the DHCP options from a received DHCP packet. Perform error checking
 * on the options received.  Any relevant status is available in:
 * "sm_params.rx_status"
 *
 */
static void dhcp6_parse_options(uchar *rx_pkt, unsigned int len)
{
	uchar *option_ptr;
	int sol_max_rt_sec, option_len, param_len_1;
	char *s, *e;
	struct dhcp6_option_hdr *option_hdr;

	memset(&sm_params.rx_status, 0, sizeof(struct dhcp6_rx_pkt_status));

	option_hdr = (struct dhcp6_option_hdr *)(rx_pkt + sizeof(struct dhcp6_hdr));
	/* check that required options exist */
	while (option_hdr < (struct dhcp6_option_hdr *)(rx_pkt + len)) {
		option_ptr = ((uchar *)option_hdr) + sizeof(struct dhcp6_hdr);
		option_len = ntohs(option_hdr->option_len);

		if (option_ptr + option_len > rx_pkt + len) {
			debug("Invalid option length\n");
			return;
		}

		switch (ntohs(option_hdr->option_id)) {
		case DHCP6_OPTION_CLIENTID:
			if (memcmp(option_ptr, sm_params.duid, option_len)
			    != 0) {
				debug("CLIENT ID DOESN'T MATCH\n");
			} else {
				debug("CLIENT ID FOUND and MATCHES\n");
				sm_params.rx_status.client_id_match = true;
			}
			break;
		case DHCP6_OPTION_SERVERID:
			sm_params.rx_status.server_id_found = true;
			sm_params.rx_status.server_uid_ptr = (uchar *)option_hdr;
			sm_params.rx_status.server_uid_size = option_len +
							      sizeof(struct dhcp6_option_hdr);
			debug("SERVER ID FOUND\n");
			break;
		case DHCP6_OPTION_IA_TA:
		case DHCP6_OPTION_IA_NA:
			/* check the IA_ID */
			if (*((u32 *)option_ptr) !=  htonl(sm_params.ia_id)) {
				debug("IA_ID mismatch 0x%08x 0x%08x\n",
				      *((u32 *)option_ptr), htonl(sm_params.ia_id));
				break;
			}

			if (ntohs(option_hdr->option_id) == DHCP6_OPTION_IA_NA) {
				/* skip past IA_ID/T1/T2 */
				option_ptr += 3 * sizeof(u32);
			} else if (ntohs(option_hdr->option_id) == DHCP6_OPTION_IA_TA) {
				/* skip past IA_ID */
				option_ptr += sizeof(u32);
			}
			/* parse the IA_NA/IA_TA encapsulated options */
			dhcp6_parse_ia_options(option_hdr, option_ptr);
			break;
		case DHCP6_OPTION_STATUS_CODE:
			debug("DHCP6_OPTION_STATUS_CODE FOUND\n");
			sm_params.rx_status.status_code = ntohs(*((u16 *)option_ptr));
			debug("DHCP6 top-level status code %d\n", sm_params.rx_status.status_code);
			debug("DHCP6 status message: %.*s\n", len, option_ptr + 2);
			break;
		case DHCP6_OPTION_SOL_MAX_RT:
			debug("DHCP6_OPTION_SOL_MAX_RT FOUND\n");
			sol_max_rt_sec = ntohl(*((u32 *)option_ptr));

			/* A DHCP client MUST ignore any SOL_MAX_RT option values that are less
			 * than 60 or more than 86400
			 */
			if (sol_max_rt_sec >= 60 && sol_max_rt_sec <= 86400) {
				updated_sol_max_rt_ms = sol_max_rt_sec * 1000;
				if (sm_params.curr_state == DHCP6_SOLICIT)
					sm_params.mrt_ms = updated_sol_max_rt_ms;
			}
			break;
		case DHCP6_OPTION_OPT_BOOTFILE_URL:
			debug("DHCP6_OPTION_OPT_BOOTFILE_URL FOUND\n");
			copy_filename(net_boot_file_name, option_ptr, option_len + 1);
			debug("net_boot_file_name: %s\n", net_boot_file_name);

			/* copy server_ip6 (required for PXE) */
			s = strchr(net_boot_file_name, '[');
			e = strchr(net_boot_file_name, ']');
			if (s && e && e > s)
				string_to_ip6(s + 1, e - s - 1, &net_server_ip6);
			break;
		case DHCP6_OPTION_OPT_BOOTFILE_PARAM:
			if (IS_ENABLED(CONFIG_DHCP6_PXE_DHCP_OPTION)) {
				debug("DHCP6_OPTION_OPT_BOOTFILE_PARAM FOUND\n");
				/* if CONFIG_DHCP6_PXE_DHCP_OPTION is set the PXE config file path
				 * is contained in the first OPT_BOOTFILE_PARAM argument
				 */
				param_len_1 = ntohs(*((u16 *)option_ptr));
				option_ptr += sizeof(u16);
				if (param_len_1 + sizeof(u16) > option_len) {
					debug("Invalid BOOTFILE_PARAM param_len_1. Skipping\n");
					break;
				}

				if (pxelinux_configfile)
					free(pxelinux_configfile);

				pxelinux_configfile = (char *)malloc((param_len_1 + 1) *
						      sizeof(char));
				if (pxelinux_configfile)
					strlcpy(pxelinux_configfile, option_ptr, param_len_1 + 1);
				else
					printf("Error: Failed to allocate pxelinux_configfile\n");

				debug("PXE CONFIG FILE %s\n", pxelinux_configfile);
			}
			break;
		case DHCP6_OPTION_PREFERENCE:
			debug("DHCP6_OPTION_PREFERENCE FOUND\n");
			sm_params.rx_status.preference = *option_ptr;
			break;
		default:
			debug("Unknown Option ID: %d, skipping parsing\n",
			      ntohs(option_hdr->option_id));
			break;
		}
		/* Increment to next option header */
		option_hdr = (struct dhcp6_option_hdr *)(((uchar *)option_hdr) +
			     sizeof(struct dhcp6_option_hdr) + option_len);
	}
}

/**
 * dhcp6_check_advertise_packet() - Perform error checking on an expected
 *                                  ADVERTISE packet.
 *
 * @rx_pkt: pointer to beginning of received DHCP6 packet
 * @len: Total length of the DHCP6 packet
 *
 * Implements RFC 8415:
 *    - 16.3.  Advertise Message
 *    - 18.2.10.  Receipt of Reply Messages
 *
 * Return : 0 : ADVERTISE packet was received with no errors.
 *              State machine can progress
 *          1 : - packet received is not an ADVERTISE packet
 *              - there were errors in the packet received,
 *              - this is the first SOLICIT packet, but
 *                received preference is not 255, so we have
 *                to wait for more server responses.
 */
static int dhcp6_check_advertise_packet(uchar *rx_pkt, unsigned int len)
{
	u16 rx_uid_size;
	struct dhcp6_hdr *dhcp6_hdr = (struct dhcp6_hdr *)rx_pkt;

	/* Ignore message if msg-type != advertise */
	if (dhcp6_hdr->msg_type != DHCP6_MSG_ADVERTISE)
		return 1;
	/* Ignore message if transaction ID doesn't match */
	if (dhcp6_hdr->trans_id != htons(sm_params.trans_id))
		return 1;

	dhcp6_parse_options(rx_pkt, len);

	/* Ignore advertise if any of these conditions met */
	if (!sm_params.rx_status.server_id_found  ||
	    !sm_params.rx_status.client_id_match  ||
	    sm_params.rx_status.status_code != DHCP6_SUCCESS) {
		return 1;
	}

	if (sm_params.rx_status.server_id_found) {
		/* if no server UID has been received yet, or if the server UID
		 * received has a higher preference value than the currently saved
		 * server UID, save the new server UID and preference
		 */
		if (!sm_params.server_uid.uid_ptr ||
		    (sm_params.server_uid.uid_ptr &&
		    sm_params.server_uid.preference < sm_params.rx_status.preference)) {
			rx_uid_size = sm_params.rx_status.server_uid_size;
			if (sm_params.server_uid.uid_ptr)
				free(sm_params.server_uid.uid_ptr);
			sm_params.server_uid.uid_ptr = malloc(rx_uid_size * sizeof(uchar));
			if (sm_params.server_uid.uid_ptr)
				memcpy(sm_params.server_uid.uid_ptr,
				       sm_params.rx_status.server_uid_ptr, rx_uid_size);

			sm_params.server_uid.uid_size = rx_uid_size;
			sm_params.server_uid.preference = sm_params.rx_status.preference;
		}

		/* If the first SOLICIT and preference code is 255, use right away.
		 * Otherwise, wait for the first SOLICIT period for more
		 * DHCP6 servers to respond.
		 */
		if (sm_params.retry_cnt == 1 &&
		    sm_params.server_uid.preference != 255) {
			debug("valid ADVERTISE, waiting for first SOLICIT period\n");
			return 1;
		}
	}

	return 0;
}

/**
 * dhcp6_check_reply_packet() - Perform error checking on an expected
 *                              REPLY packet.
 *
 * @rx_pkt: pointer to beginning of received DHCP6 packet
 * @len: Total length of the DHCP6 packet
 *
 * Implements RFC 8415:
 *    - 16.10. Reply Message
 *    - 18.2.10. Receipt of Reply Messages
 *
 * Return : 0 - REPLY packet was received with no errors
 *          1 - packet received is not an REPLY packet,
 *              or there were errors in the packet received
 */
static int dhcp6_check_reply_packet(uchar *rx_pkt, unsigned int len)
{
	struct dhcp6_hdr *dhcp6_hdr = (struct dhcp6_hdr *)rx_pkt;

	/* Ignore message if msg-type != reply */
	if (dhcp6_hdr->msg_type != DHCP6_MSG_REPLY)
		return 1;
	/* check that transaction ID matches */
	if (dhcp6_hdr->trans_id != htons(sm_params.trans_id))
		return 1;

	dhcp6_parse_options(rx_pkt, len);

	/* if no addresses found, restart DHCP */
	if (!sm_params.rx_status.ia_addr_found ||
	    sm_params.rx_status.ia_status_code == DHCP6_NO_ADDRS_AVAIL ||
	    sm_params.rx_status.status_code == DHCP6_NOT_ON_LINK) {
		/* restart DHCP */
		debug("No address found in reply.  Restarting DHCP\n");
		dhcp6_start();
	}

	/* ignore reply if any of these conditions met */
	if (!sm_params.rx_status.server_id_found  ||
	    !sm_params.rx_status.client_id_match ||
	    sm_params.rx_status.status_code == DHCP6_UNSPEC_FAIL) {
		return 1;
	}

	return 0;
}

/* Timeout for DHCP6 SOLICIT/REQUEST */
static void dhcp6_timeout_handler(void)
{
	/* call state machine with the timeout flag */
	dhcp6_state_machine(true, NULL, 0);
}

/**
 * dhcp6_state_machine() - DHCP6 state machine
 *
 * @timeout: TRUE : timeout waiting for response from
 *                  DHCP6 server
 *           FALSE : init or received response from DHCP6 server
 * @rx_pkt: Pointer to the beginning of received DHCP6 packet.
 *          Will be NULL if called as part of init
 *          or timeout==TRUE
 * @len: Total length of the DHCP6 packet if rx_pkt != NULL
 *
 * Implements RFC 8415:
 *    - 5.2.  Client/Server Exchanges Involving Four Messages
 *    - 15.  Reliability of Client-Initiated Message Exchanges
 *
 * Handles:
 *    - transmission of SOLICIT and REQUEST packets
 *    - retransmission of SOLICIT and REQUEST packets if no
 *      response is received within the timeout window
 *    - checking received ADVERTISE and REPLY packets to
 *      assess if the DHCP state machine can progress
 */
static void dhcp6_state_machine(bool timeout, uchar *rx_pkt, unsigned int len)
{
	int rand_minus_plus_100;

	switch (sm_params.curr_state) {
	case DHCP6_INIT:
		sm_params.next_state = DHCP6_SOLICIT;
		break;
	case DHCP6_SOLICIT:
		if (!timeout) {
			/* check the rx packet and determine if we can transition to next
			 * state.
			 */
			if (dhcp6_check_advertise_packet(rx_pkt, len))
				return;

			debug("ADVERTISE good, transition to REQUEST\n");
			sm_params.next_state = DHCP6_REQUEST;
		} else if (sm_params.retry_cnt == 1)  {
			/* If a server UID was received in the first SOLICIT period
			 * transition to REQUEST
			 */
			if (sm_params.server_uid.uid_ptr)
				sm_params.next_state = DHCP6_REQUEST;
		}
		break;
	case DHCP6_REQUEST:
		if (!timeout) {
			/* check the rx packet and determine if we can transition to next state */
			if (dhcp6_check_reply_packet(rx_pkt, len))
				return;

			debug("REPLY good, transition to DONE\n");
			sm_params.next_state = DHCP6_DONE;
		}
		break;
	case DHCP6_DONE:
	case DHCP6_FAIL:
		/* Shouldn't get here, as state machine should exit
		 * immediately when DHCP6_DONE or DHCP6_FAIL is entered.
		 * Proceed anyway to proceed DONE/FAIL actions
		 */
		debug("Unexpected DHCP6 state : %d\n", sm_params.curr_state);
		break;
	}
	/* re-seed the RNG */
	srand(get_ticks() + rand());

	/* handle state machine entry conditions */
	if (sm_params.curr_state != sm_params.next_state) {
		sm_params.retry_cnt = 0;

		if (sm_params.next_state == DHCP6_SOLICIT) {
			/* delay a random ammount (special for SOLICIT) */
			udelay((rand() % SOL_MAX_DELAY_MS) * 1000);
			/* init timestamp variables after SOLICIT delay */
			sm_params.dhcp6_start_ms = get_timer(0);
			sm_params.dhcp6_retry_start_ms = sm_params.dhcp6_start_ms;
			sm_params.dhcp6_retry_ms = sm_params.dhcp6_start_ms;
			/* init transaction and ia_id */
			sm_params.trans_id = rand() & 0xFFFFFF;
			sm_params.ia_id = rand();
			/* initialize retransmission parameters */
			sm_params.irt_ms = SOL_TIMEOUT_MS;
			sm_params.mrt_ms = updated_sol_max_rt_ms;
			/* RFCs default MRC is be 0 (try infinitely)
			 * give up after CONFIG_NET_RETRY_COUNT number of tries (same as DHCPv4)
			 */
			sm_params.mrc = CONFIG_NET_RETRY_COUNT;
			sm_params.mrd_ms = 0;

		} else if (sm_params.next_state == DHCP6_REQUEST) {
			/* init timestamp variables  */
			sm_params.dhcp6_retry_start_ms = get_timer(0);
			sm_params.dhcp6_retry_ms = sm_params.dhcp6_start_ms;
			/* initialize retransmission parameters */
			sm_params.irt_ms = REQ_TIMEOUT_MS;
			sm_params.mrt_ms = REQ_MAX_RT_MS;
			sm_params.mrc = REQ_MAX_RC;
			sm_params.mrd_ms = 0;
		}
	}

	if (timeout)
		sm_params.dhcp6_retry_ms = get_timer(0);

	/* Check if MRC or MRD have been passed */
	if ((sm_params.mrc != 0 &&
	     sm_params.retry_cnt >= sm_params.mrc) ||
	    (sm_params.mrd_ms != 0 &&
	     ((sm_params.dhcp6_retry_ms - sm_params.dhcp6_retry_start_ms) >= sm_params.mrd_ms))) {
		sm_params.next_state = DHCP6_FAIL;
	}

	/* calculate retransmission timeout (RT) */
	rand_minus_plus_100 = ((rand() % 200) - 100);
	if (sm_params.retry_cnt == 0) {
		sm_params.rt_ms = sm_params.irt_ms +
				  ((sm_params.irt_ms * rand_minus_plus_100) / 1000);
	} else {
		sm_params.rt_ms = (2 * sm_params.rt_prev_ms) +
				  ((sm_params.rt_prev_ms * rand_minus_plus_100) / 1000);
	}

	if (sm_params.rt_ms > sm_params.mrt_ms) {
		sm_params.rt_ms = sm_params.mrt_ms +
				  ((sm_params.mrt_ms * rand_minus_plus_100) / 1000);
	}

	sm_params.rt_prev_ms = sm_params.rt_ms;

	net_set_timeout_handler(sm_params.rt_ms, dhcp6_timeout_handler);

	/* send transmit/retransmit message or fail */
	sm_params.curr_state = sm_params.next_state;

	if (sm_params.curr_state == DHCP6_SOLICIT) {
		/* send solicit packet */
		dhcp6_send_solicit_packet();
		printf("DHCP6 SOLICIT %d\n", sm_params.retry_cnt);
	} else if (sm_params.curr_state == DHCP6_REQUEST) {
		/* send request packet */
		dhcp6_send_request_packet();
		printf("DHCP6 REQUEST %d\n", sm_params.retry_cnt);
	} else if (sm_params.curr_state == DHCP6_DONE) {
		net_set_timeout_handler(0, NULL);

		/* Duplicate address detection (DAD) should be
		 * performed here before setting net_ip6
		 * (enhancement should be considered)
		 */
		net_copy_ip6(&net_ip6, &sm_params.rx_status.ia_addr_ipv6);
		printf("DHCP6 client bound to %pI6c\n", &net_ip6);
		/* will load with TFTP6 */
		net_auto_load();
	} else if (sm_params.curr_state == DHCP6_FAIL) {
		printf("DHCP6 FAILED, TERMINATING\n");
		net_set_state(NETLOOP_FAIL);
	}
	sm_params.retry_cnt++;
}

/* Start or restart DHCP6 */
void dhcp6_start(void)
{
	memset(&sm_params, 0, sizeof(struct dhcp6_sm_params));

	/* seed the RNG with MAC address */
	srand_mac();

	sm_params.curr_state = DHCP6_INIT;
	dhcp6_state_machine(false, NULL, 0);
}
