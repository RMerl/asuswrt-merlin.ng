// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (C) 2016 The Android Open Source Project
 */

#include <common.h>
#include <fastboot.h>
#include <net.h>
#include <net/fastboot.h>

/* Fastboot port # defined in spec */
#define WELL_KNOWN_PORT 5554

enum {
	FASTBOOT_ERROR = 0,
	FASTBOOT_QUERY = 1,
	FASTBOOT_INIT = 2,
	FASTBOOT_FASTBOOT = 3,
};

struct __packed fastboot_header {
	uchar id;
	uchar flags;
	unsigned short seq;
};

#define PACKET_SIZE 1024
#define DATA_SIZE (PACKET_SIZE - sizeof(struct fastboot_header))

/* Sequence number sent for every packet */
static unsigned short sequence_number = 1;
static const unsigned short packet_size = PACKET_SIZE;
static const unsigned short udp_version = 1;

/* Keep track of last packet for resubmission */
static uchar last_packet[PACKET_SIZE];
static unsigned int last_packet_len;

static struct in_addr fastboot_remote_ip;
/* The UDP port at their end */
static int fastboot_remote_port;
/* The UDP port at our end */
static int fastboot_our_port;

static void boot_downloaded_image(void);

#if CONFIG_IS_ENABLED(FASTBOOT_FLASH)
/**
 * fastboot_udp_send_info() - Send an INFO packet during long commands.
 *
 * @msg: String describing the reason for waiting
 */
static void fastboot_udp_send_info(const char *msg)
{
	uchar *packet;
	uchar *packet_base;
	int len = 0;
	char response[FASTBOOT_RESPONSE_LEN] = {0};

	struct fastboot_header response_header = {
		.id = FASTBOOT_FASTBOOT,
		.flags = 0,
		.seq = htons(sequence_number)
	};
	++sequence_number;
	packet = net_tx_packet + net_eth_hdr_size() + IP_UDP_HDR_SIZE;
	packet_base = packet;

	/* Write headers */
	memcpy(packet, &response_header, sizeof(response_header));
	packet += sizeof(response_header);
	/* Write response */
	fastboot_response("INFO", response, "%s", msg);
	memcpy(packet, response, strlen(response));
	packet += strlen(response);

	len = packet - packet_base;

	/* Save packet for retransmitting */
	last_packet_len = len;
	memcpy(last_packet, packet_base, last_packet_len);

	net_send_udp_packet(net_server_ethaddr, fastboot_remote_ip,
			    fastboot_remote_port, fastboot_our_port, len);
}

/**
 * fastboot_timed_send_info() - Send INFO packet every 30 seconds
 *
 * @msg: String describing the reason for waiting
 *
 * Send an INFO packet during long commands based on timer. An INFO packet
 * is sent if the time is 30 seconds after start. Else, noop.
 */
static void fastboot_timed_send_info(const char *msg)
{
	static ulong start;

	/* Initialize timer */
	if (start == 0)
		start = get_timer(0);
	ulong time = get_timer(start);
	/* Send INFO packet to host every 30 seconds */
	if (time >= 30000) {
		start = get_timer(0);
		fastboot_udp_send_info(msg);
	}
}
#endif

/**
 * fastboot_send() - Sends a packet in response to received fastboot packet
 *
 * @header: Header for response packet
 * @fastboot_data: Pointer to received fastboot data
 * @fastboot_data_len: Length of received fastboot data
 * @retransmit: Nonzero if sending last sent packet
 */
static void fastboot_send(struct fastboot_header header, char *fastboot_data,
			  unsigned int fastboot_data_len, uchar retransmit)
{
	uchar *packet;
	uchar *packet_base;
	int len = 0;
	const char *error_msg = "An error occurred.";
	short tmp;
	struct fastboot_header response_header = header;
	static char command[FASTBOOT_COMMAND_LEN];
	static int cmd = -1;
	static bool pending_command;
	char response[FASTBOOT_RESPONSE_LEN] = {0};

	/*
	 * We will always be sending some sort of packet, so
	 * cobble together the packet headers now.
	 */
	packet = net_tx_packet + net_eth_hdr_size() + IP_UDP_HDR_SIZE;
	packet_base = packet;

	/* Resend last packet */
	if (retransmit) {
		memcpy(packet, last_packet, last_packet_len);
		net_send_udp_packet(net_server_ethaddr, fastboot_remote_ip,
				    fastboot_remote_port, fastboot_our_port,
				    last_packet_len);
		return;
	}

	response_header.seq = htons(response_header.seq);
	memcpy(packet, &response_header, sizeof(response_header));
	packet += sizeof(response_header);

	switch (header.id) {
	case FASTBOOT_QUERY:
		tmp = htons(sequence_number);
		memcpy(packet, &tmp, sizeof(tmp));
		packet += sizeof(tmp);
		break;
	case FASTBOOT_INIT:
		tmp = htons(udp_version);
		memcpy(packet, &tmp, sizeof(tmp));
		packet += sizeof(tmp);
		tmp = htons(packet_size);
		memcpy(packet, &tmp, sizeof(tmp));
		packet += sizeof(tmp);
		break;
	case FASTBOOT_ERROR:
		memcpy(packet, error_msg, strlen(error_msg));
		packet += strlen(error_msg);
		break;
	case FASTBOOT_FASTBOOT:
		if (cmd == FASTBOOT_COMMAND_DOWNLOAD) {
			if (!fastboot_data_len && !fastboot_data_remaining()) {
				fastboot_data_complete(response);
			} else {
				fastboot_data_download(fastboot_data,
						       fastboot_data_len,
						       response);
			}
		} else if (!pending_command) {
			strlcpy(command, fastboot_data,
				min((size_t)fastboot_data_len + 1,
				    sizeof(command)));
			pending_command = true;
		} else {
			cmd = fastboot_handle_command(command, response);
			pending_command = false;
		}
		/*
		 * Sent some INFO packets, need to update sequence number in
		 * header
		 */
		if (header.seq != sequence_number) {
			response_header.seq = htons(sequence_number);
			memcpy(packet_base, &response_header,
			       sizeof(response_header));
		}
		/* Write response to packet */
		memcpy(packet, response, strlen(response));
		packet += strlen(response);
		break;
	default:
		pr_err("ID %d not implemented.\n", header.id);
		return;
	}

	len = packet - packet_base;

	/* Save packet for retransmitting */
	last_packet_len = len;
	memcpy(last_packet, packet_base, last_packet_len);

	net_send_udp_packet(net_server_ethaddr, fastboot_remote_ip,
			    fastboot_remote_port, fastboot_our_port, len);

	/* Continue boot process after sending response */
	if (!strncmp("OKAY", response, 4)) {
		switch (cmd) {
		case FASTBOOT_COMMAND_BOOT:
			boot_downloaded_image();
			break;

		case FASTBOOT_COMMAND_CONTINUE:
			net_set_state(NETLOOP_SUCCESS);
			break;

		case FASTBOOT_COMMAND_REBOOT:
		case FASTBOOT_COMMAND_REBOOT_BOOTLOADER:
			do_reset(NULL, 0, 0, NULL);
			break;
		}
	}

	if (!strncmp("OKAY", response, 4) || !strncmp("FAIL", response, 4))
		cmd = -1;
}

/**
 * boot_downloaded_image() - Boots into downloaded image.
 */
static void boot_downloaded_image(void)
{
	fastboot_boot();
	net_set_state(NETLOOP_SUCCESS);
}

/**
 * fastboot_handler() - Incoming UDP packet handler.
 *
 * @packet: Pointer to incoming UDP packet
 * @dport: Destination UDP port
 * @sip: Source IP address
 * @sport: Source UDP port
 * @len: Packet length
 */
static void fastboot_handler(uchar *packet, unsigned int dport,
			     struct in_addr sip, unsigned int sport,
			     unsigned int len)
{
	struct fastboot_header header;
	char fastboot_data[DATA_SIZE] = {0};
	unsigned int fastboot_data_len = 0;

	if (dport != fastboot_our_port)
		return;

	fastboot_remote_ip = sip;
	fastboot_remote_port = sport;

	if (len < sizeof(struct fastboot_header) || len > PACKET_SIZE)
		return;
	memcpy(&header, packet, sizeof(header));
	header.flags = 0;
	header.seq = ntohs(header.seq);
	packet += sizeof(header);
	len -= sizeof(header);

	switch (header.id) {
	case FASTBOOT_QUERY:
		fastboot_send(header, fastboot_data, 0, 0);
		break;
	case FASTBOOT_INIT:
	case FASTBOOT_FASTBOOT:
		fastboot_data_len = len;
		if (len > 0)
			memcpy(fastboot_data, packet, len);
		if (header.seq == sequence_number) {
			fastboot_send(header, fastboot_data,
				      fastboot_data_len, 0);
			sequence_number++;
		} else if (header.seq == sequence_number - 1) {
			/* Retransmit last sent packet */
			fastboot_send(header, fastboot_data,
				      fastboot_data_len, 1);
		}
		break;
	default:
		pr_err("ID %d not implemented.\n", header.id);
		header.id = FASTBOOT_ERROR;
		fastboot_send(header, fastboot_data, 0, 0);
		break;
	}
}

void fastboot_start_server(void)
{
	printf("Using %s device\n", eth_get_name());
	printf("Listening for fastboot command on %pI4\n", &net_ip);

	fastboot_our_port = WELL_KNOWN_PORT;

#if CONFIG_IS_ENABLED(FASTBOOT_FLASH)
	fastboot_set_progress_callback(fastboot_timed_send_info);
#endif
	net_set_udp_handler(fastboot_handler);

	/* zero out server ether in case the server ip has changed */
	memset(net_server_ethaddr, 0, 6);
}
