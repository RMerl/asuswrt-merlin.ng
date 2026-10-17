// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (C) 2023 The Android Open Source Project
 */

#include <common.h>
#include <fastboot.h>
#include <net.h>
#include <net/fastboot_tcp.h>
#include <net/tcp.h>

static char command[FASTBOOT_COMMAND_LEN] = {0};
static char response[FASTBOOT_RESPONSE_LEN] = {0};

static const unsigned short handshake_length = 4;
static const uchar *handshake = "FB01";

static u16 curr_sport;
static u16 curr_dport;
static u32 curr_tcp_seq_num;
static u32 curr_tcp_ack_num;
static unsigned int curr_request_len;
static enum fastboot_tcp_state {
	FASTBOOT_CLOSED,
	FASTBOOT_CONNECTED,
	FASTBOOT_DISCONNECTING
} state = FASTBOOT_CLOSED;

static void fastboot_tcp_answer(u8 action, unsigned int len)
{
	const u32 response_seq_num = curr_tcp_ack_num;
	const u32 response_ack_num = curr_tcp_seq_num +
		  (curr_request_len > 0 ? curr_request_len : 1);

	net_send_tcp_packet(len, htons(curr_sport), htons(curr_dport),
			    action, response_seq_num, response_ack_num);
}

static void fastboot_tcp_reset(void)
{
	fastboot_tcp_answer(TCP_RST, 0);
	state = FASTBOOT_CLOSED;
}

static void fastboot_tcp_send_packet(u8 action, const uchar *data, unsigned int len)
{
	uchar *pkt = net_get_async_tx_pkt_buf();

	memset(pkt, '\0', PKTSIZE);
	pkt += net_eth_hdr_size() + IP_TCP_HDR_SIZE + TCP_TSOPT_SIZE + 2;
	memcpy(pkt, data, len);
	fastboot_tcp_answer(action, len);
	memset(pkt, '\0', PKTSIZE);
}

static void fastboot_tcp_send_message(const char *message, unsigned int len)
{
	__be64 len_be = __cpu_to_be64(len);
	uchar *pkt = net_get_async_tx_pkt_buf();

	memset(pkt, '\0', PKTSIZE);
	pkt += net_eth_hdr_size() + IP_TCP_HDR_SIZE + TCP_TSOPT_SIZE + 2;
	// Put first 8 bytes as a big endian message length
	memcpy(pkt, &len_be, 8);
	pkt += 8;
	memcpy(pkt, message, len);
	fastboot_tcp_answer(TCP_ACK | TCP_PUSH, len + 8);
	memset(pkt, '\0', PKTSIZE);
}

static void fastboot_tcp_handler_ipv4(uchar *pkt, u16 dport,
				      struct in_addr sip, u16 sport,
				      u32 tcp_seq_num, u32 tcp_ack_num,
				      u8 action, unsigned int len)
{
	int fastboot_command_id;
	u64 command_size;
	u8 tcp_fin = action & TCP_FIN;
	u8 tcp_push = action & TCP_PUSH;

	curr_sport = sport;
	curr_dport = dport;
	curr_tcp_seq_num = tcp_seq_num;
	curr_tcp_ack_num = tcp_ack_num;
	curr_request_len = len;

	switch (state) {
	case FASTBOOT_CLOSED:
		if (tcp_push) {
			if (len != handshake_length ||
			    strlen(pkt) != handshake_length ||
			    memcmp(pkt, handshake, handshake_length) != 0) {
				fastboot_tcp_reset();
				break;
			}
			fastboot_tcp_send_packet(TCP_ACK | TCP_PUSH,
						 handshake, handshake_length);
			state = FASTBOOT_CONNECTED;
		}
		break;
	case FASTBOOT_CONNECTED:
		if (tcp_fin) {
			fastboot_tcp_answer(TCP_FIN | TCP_ACK, 0);
			state = FASTBOOT_DISCONNECTING;
			break;
		}
		if (tcp_push) {
			// First 8 bytes is big endian message length
			command_size = __be64_to_cpu(*(u64 *)pkt);
			len -= 8;
			pkt += 8;

			// Only single packet messages are supported ATM
			if (strlen(pkt) != command_size) {
				fastboot_tcp_reset();
				break;
			}
			strlcpy(command, pkt, len + 1);
			fastboot_command_id = fastboot_handle_command(command, response);
			fastboot_tcp_send_message(response, strlen(response));
			fastboot_handle_boot(fastboot_command_id,
					     strncmp("OKAY", response, 4) == 0);
		}
		break;
	case FASTBOOT_DISCONNECTING:
		if (tcp_push)
			state = FASTBOOT_CLOSED;
		break;
	}

	memset(command, 0, FASTBOOT_COMMAND_LEN);
	memset(response, 0, FASTBOOT_RESPONSE_LEN);
	curr_sport = 0;
	curr_dport = 0;
	curr_tcp_seq_num = 0;
	curr_tcp_ack_num = 0;
	curr_request_len = 0;
}

void fastboot_tcp_start_server(void)
{
	printf("Using %s device\n", eth_get_name());
	printf("Listening for fastboot command on tcp %pI4\n", &net_ip);

	tcp_set_tcp_handler(fastboot_tcp_handler_ipv4);
}
