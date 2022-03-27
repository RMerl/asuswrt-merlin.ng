/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2007-2010  Marcel Holtmann <marcel@holtmann.org>
 *  Copyright (C) 2009-2010  Nokia Corporation
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "lib/bluetooth.h"
#include "lib/hci.h"
#include "lib/hci_lib.h"
#include "lib/l2cap.h"
#include "lib/sdp.h"

#define AVDTP_PKT_TYPE_SINGLE		0x00
#define AVDTP_PKT_TYPE_START		0x01
#define AVDTP_PKT_TYPE_CONTINUE		0x02
#define AVDTP_PKT_TYPE_END		0x03

#define AVDTP_MSG_TYPE_COMMAND		0x00
#define AVDTP_MSG_TYPE_GEN_REJECT	0x01
#define AVDTP_MSG_TYPE_ACCEPT		0x02
#define AVDTP_MSG_TYPE_REJECT		0x03

#define AVDTP_DISCOVER			0x01
#define AVDTP_GET_CAPABILITIES		0x02
#define AVDTP_SET_CONFIGURATION		0x03
#define AVDTP_GET_CONFIGURATION		0x04
#define AVDTP_RECONFIGURE		0x05
#define AVDTP_OPEN			0x06
#define AVDTP_START			0x07
#define AVDTP_CLOSE			0x08
#define AVDTP_SUSPEND			0x09
#define AVDTP_ABORT			0x0A

#define AVDTP_SEP_TYPE_SOURCE		0x00
#define AVDTP_SEP_TYPE_SINK		0x01

#define AVDTP_MEDIA_TYPE_AUDIO		0x00
#define AVDTP_MEDIA_TYPE_VIDEO		0x01
#define AVDTP_MEDIA_TYPE_MULTIMEDIA	0x02

#if __BYTE_ORDER == __LITTLE_ENDIAN

struct avdtp_header {
	uint8_t message_type:2;
	uint8_t packet_type:2;
	uint8_t transaction:4;
	uint8_t signal_id:6;
	uint8_t rfa0:2;
} __attribute__ ((packed));

struct seid_info {
	uint8_t rfa0:1;
	uint8_t inuse:1;
	uint8_t seid:6;
	uint8_t rfa2:3;
	uint8_t type:1;
	uint8_t media_type:4;
} __attribute__ ((packed));

struct avdtp_start_header {
	uint8_t message_type:2;
	uint8_t packet_type:2;
	uint8_t transaction:4;
	uint8_t no_of_packets;
	uint8_t signal_id:6;
	uint8_t rfa0:2;
} __attribute__ ((packed));

struct avdtp_continue_header {
	uint8_t message_type:2;
	uint8_t packet_type:2;
	uint8_t transaction:4;
} __attribute__ ((packed));

struct avctp_header {
	uint8_t ipid:1;
	uint8_t cr:1;
	uint8_t packet_type:2;
	uint8_t transaction:4;
	uint16_t pid;
} __attribute__ ((packed));
#define AVCTP_HEADER_LENGTH 3

#elif __BYTE_ORDER == __BIG_ENDIAN

struct avdtp_header {
	uint8_t transaction:4;
	uint8_t packet_type:2;
	uint8_t message_type:2;
	uint8_t rfa0:2;
	uint8_t signal_id:6;
} __attribute__ ((packed));

struct seid_info {
	uint8_t seid:6;
	uint8_t inuse:1;
	uint8_t rfa0:1;
	uint8_t media_type:4;
	uint8_t type:1;
	uint8_t rfa2:3;
} __attribute__ ((packed));

struct avdtp_start_header {
	uint8_t transaction:4;
	uint8_t packet_type:2;
	uint8_t message_type:2;
	uint8_t no_of_packets;
	uint8_t rfa0:2;
	uint8_t signal_id:6;
} __attribute__ ((packed));

struct avdtp_continue_header {
	uint8_t transaction:4;
	uint8_t packet_type:2;
	uint8_t message_type:2;
} __attribute__ ((packed));

struct avctp_header {
	uint8_t transaction:4;
	uint8_t packet_type:2;
	uint8_t cr:1;
	uint8_t ipid:1;
	uint16_t pid;
} __attribute__ ((packed));
#define AVCTP_HEADER_LENGTH 3

#else
#error "Unknown byte order"
#endif

#define AVCTP_COMMAND		0
#define AVCTP_RESPONSE		1

#define AVCTP_PACKET_SINGLE	0

static const unsigned char media_transport[] = {
		0x01,	/* Media transport category */
		0x00,
		0x07,	/* Media codec category */
		0x06,
		0x00,	/* Media type audio */
		0x00,	/* Codec SBC */
		0x22,	/* 44.1 kHz, stereo */
		0x15,	/* 16 blocks, 8 subbands */
		0x02,
		0x33,
};

static int media_sock = -1;

static void dump_avctp_header(struct avctp_header *hdr)
{
	printf("TL %d PT %d CR %d IPID %d PID 0x%04x\n", hdr->transaction,
			hdr->packet_type, hdr->cr, hdr->ipid, ntohs(hdr->pid));
}

static void dump_avdtp_header(struct avdtp_header *hdr)
{
	printf("TL %d PT %d MT %d SI %d\n", hdr->transaction,
			hdr->packet_type, hdr->message_type, hdr->signal_id);
}

static void dump_buffer(const unsigned char *buf, int len)
{
	int i;

	for (i = 0; i < len; i++)
		printf("%02x ", buf[i]);
	printf("\n");
}

static void process_avdtp(int srv_sk, int sk, unsigned char reject,
								int fragment)
{
	unsigned char buf[672];
	ssize_t len;

	while (1) {
		struct avdtp_header *hdr = (void *) buf;

		len = read(sk, buf, sizeof(buf));
		if (len <= 0) {
			perror("Read failed");
			break;
		}

		dump_buffer(buf, len);
		dump_avdtp_header(hdr);

		if (hdr->packet_type != AVDTP_PKT_TYPE_SINGLE) {
			fprintf(stderr, "Only single packets are supported\n");
			break;
		}

		if (hdr->message_type != AVDTP_MSG_TYPE_COMMAND) {
			fprintf(stderr, "Ignoring non-command messages\n");
			continue;
		}

		switch (hdr->signal_id) {
		case AVDTP_DISCOVER:
			if (reject == AVDTP_DISCOVER) {
				hdr->message_type = AVDTP_MSG_TYPE_REJECT;
				buf[2] = 0x29; /* Unsupported configuration */
				printf("Rejecting discover command\n");
				len = write(sk, buf, 3);
			} else {
				struct seid_info *sei = (void *) (buf + 2);
				hdr->message_type = AVDTP_MSG_TYPE_ACCEPT;
				buf[2] = 0x00;
				buf[3] = 0x00;
				sei->seid = 0x01;
				sei->type = AVDTP_SEP_TYPE_SINK;
				sei->media_type = AVDTP_MEDIA_TYPE_AUDIO;
				printf("Accepting discover command\n");
				len = write(sk, buf, 4);
			}
			break;

		case AVDTP_GET_CAPABILITIES:
			if (reject == AVDTP_GET_CAPABILITIES) {
				hdr->message_type = AVDTP_MSG_TYPE_REJECT;
				buf[2] = 0x29; /* Unsupported configuration */
				printf("Rejecting get capabilties command\n");
				len = write(sk, buf, 3);
			} else if (fragment) {
				struct avdtp_start_header *start = (void *) buf;

				printf("Sending fragmented reply to getcap\n");

				hdr->message_type = AVDTP_MSG_TYPE_ACCEPT;

				/* Start packet */
				hdr->packet_type = AVDTP_PKT_TYPE_START;
				start->signal_id = AVDTP_GET_CAPABILITIES;
				start->no_of_packets = 3;
				memcpy(&buf[3], media_transport,
						sizeof(media_transport));
				len = write(sk, buf,
						3 + sizeof(media_transport));

				/* Continue packet */
				hdr->packet_type = AVDTP_PKT_TYPE_CONTINUE;
				memcpy(&buf[1], media_transport,
						sizeof(media_transport));
				len = write(sk, buf,
						1 + sizeof(media_transport));

				/* End packet */
				hdr->packet_type = AVDTP_PKT_TYPE_END;
				memcpy(&buf[1], media_transport,
						sizeof(media_transport));
				len = write(sk, buf,
						1 + sizeof(media_transport));
			} else {
				hdr->message_type = AVDTP_MSG_TYPE_ACCEPT;
				memcpy(&buf[2], media_transport,
						sizeof(media_transport));
				printf("Accepting get capabilities command\n");
				len = write(sk, buf,
						2 + sizeof(media_transport));
			}
			break;

		case AVDTP_SET_CONFIGURATION:
			if (reject == AVDTP_SET_CONFIGURATION) {
				hdr->message_type = AVDTP_MSG_TYPE_REJECT;
				buf[2] = buf[4];
				buf[3] = 0x13; /* SEP In Use */
				printf("Rejecting set configuration command\n");
				len = write(sk, buf, 4);
			} else {
				hdr->message_type = AVDTP_MSG_TYPE_ACCEPT;
				printf("Accepting set configuration command\n");
				len = write(sk, buf, 2);
			}
			break;

		case AVDTP_GET_CONFIGURATION:
			if (reject == AVDTP_GET_CONFIGURATION) {
				hdr->message_type = AVDTP_MSG_TYPE_REJECT;
				buf[2] = 0x12; /* Bad ACP SEID */
				printf("Rejecting get configuration command\n");
				len = write(sk, buf, 3);
			} else {
				hdr->message_type = AVDTP_MSG_TYPE_ACCEPT;
				printf("Accepting get configuration command\n");
				len = write(sk, buf, 2);
			}
			break;

		case AVDTP_OPEN:
			if (reject == AVDTP_OPEN) {
				hdr->message_type = AVDTP_MSG_TYPE_REJECT;
				buf[2] = 0x31; /* Bad State */
				printf("Rejecting open command\n");
				len = write(sk, buf, 3);
			} else {
				struct sockaddr_l2 addr;
				socklen_t optlen;

				hdr->message_type = AVDTP_MSG_TYPE_ACCEPT;
				printf("Accepting open command\n");
				len = write(sk, buf, 2);

				memset(&addr, 0, sizeof(addr));
				optlen = sizeof(addr);

				media_sock = accept(srv_sk,
						(struct sockaddr *) &addr,
								&optlen);
				if (media_sock < 0) {
					perror("Accept failed");
					break;
				}
			}
			break;

		case AVDTP_START:
			if (reject == AVDTP_ABORT)
				printf("Ignoring start to cause abort");
			else if (reject == AVDTP_START) {
				hdr->message_type = AVDTP_MSG_TYPE_REJECT;
				buf[3] = 0x31; /* Bad State */
				printf("Rejecting start command\n");
				len = write(sk, buf, 4);
			} else {
				hdr->message_type = AVDTP_MSG_TYPE_ACCEPT;
				printf("Accepting start command\n");
				len = write(sk, buf, 2);
			}
			break;

		case AVDTP_CLOSE:
			if (reject == AVDTP_CLOSE) {
				hdr->message_type = AVDTP_MSG_TYPE_REJECT;
				buf[2] = 0x31; /* Bad State */
				printf("Rejecting close command\n");
				len = write(sk, buf, 3);
			} else {
				hdr->message_type = AVDTP_MSG_TYPE_ACCEPT;
				printf("Accepting close command\n");
				len = write(sk, buf, 2);
				if (media_sock >= 0) {
					close(media_sock);
					media_sock = -1;
				}
			}
			break;

		case AVDTP_SUSPEND:
			if (reject == AVDTP_SUSPEND) {
				hdr->message_type = AVDTP_MSG_TYPE_REJECT;
				buf[3] = 0x31; /* Bad State */
				printf("Rejecting suspend command\n");
				len = write(sk, buf, 4);
			} else {
				hdr->message_type = AVDTP_MSG_TYPE_ACCEPT;
				printf("Accepting suspend command\n");
				len = write(sk, buf, 2);
			}
			break;

		case AVDTP_ABORT:
			hdr->message_type = AVDTP_MSG_TYPE_ACCEPT;
			printf("Accepting abort command\n");
			len = write(sk, buf, 2);
			if (media_sock >= 0) {
				close(media_sock);
				media_sock = -1;
			}
			break;

		default:
			buf[1] = 0x00;
			printf("Unknown command\n");
			len = write(sk, buf, 2);
			break;
		}
	}
}

static void process_avctp(int sk, int reject)
{
	unsigned char buf[672];
	ssize_t len;

	while (1) {
		struct avctp_header *hdr = (void *) buf;

		len = read(sk, buf, sizeof(buf));
		if (len <= 0) {
			perror("Read failed");
			break;
		}

		dump_buffer(buf, len);

		if (len >= AVCTP_HEADER_LENGTH)
			dump_avctp_header(hdr);
	}
}

static int set_minimum_mtu(int sk)
{
	struct l2cap_options l2o;
	socklen_t optlen;

	memset(&l2o, 0, sizeof(l2o));
	optlen = sizeof(l2o);

	if (getsockopt(sk, SOL_L2CAP, L2CAP_OPTIONS, &l2o, &optlen) < 0) {
		perror("getsockopt");
		return -1;
	}

	l2o.imtu = 48;
	l2o.omtu = 48;

	if (setsockopt(sk, SOL_L2CAP, L2CAP_OPTIONS, &l2o, sizeof(l2o)) < 0) {
		perror("setsockopt");
		return -1;
	}

	return 0;
}

static void do_listen(const bdaddr_t *src, unsigned char reject, int fragment)
{
	struct sockaddr_l2 addr;
	socklen_t optlen;
	int sk, nsk;

	sk = socket(PF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);
	if (sk < 0) {
		perror("Can't create socket");
		return;
	}

	memset(&addr, 0, sizeof(addr));
	addr.l2_family = AF_BLUETOOTH;
	bacpy(&addr.l2_bdaddr, src);
	addr.l2_psm = htobs(25);

	if (bind(sk, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		perror("Can't bind socket");
		goto error;
	}

	if (fragment)
		set_minimum_mtu(sk);

	if (listen(sk, 10)) {
		perror("Can't listen on the socket");
		goto error;
	}

	while (1) {
		memset(&addr, 0, sizeof(addr));
		optlen = sizeof(addr);

		nsk = accept(sk, (struct sockaddr *) &addr, &optlen);
		if (nsk < 0) {
			perror("Accept failed");
			continue;
		}

		process_avdtp(sk, nsk, reject, fragment);

		if (media_sock >= 0) {
			close(media_sock);
			media_sock = -1;
		}

		close(nsk);
	}

error:
	close(sk);
}

static int do_connect(const bdaddr_t *src, const bdaddr_t *dst, int avctp,
								int fragment)
{
	struct sockaddr_l2 addr;
	int sk, err;

	sk = socket(PF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);
	if (sk < 0) {
		perror("Can't create socket");
		return -1;
	}

	memset(&addr, 0, sizeof(addr));
	addr.l2_family = AF_BLUETOOTH;
	bacpy(&addr.l2_bdaddr, src);

	if (bind(sk, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		perror("Can't bind socket");
		goto error;
	}

	if (fragment)
		set_minimum_mtu(sk);

	memset(&addr, 0, sizeof(addr));
	addr.l2_family = AF_BLUETOOTH;
	bacpy(&addr.l2_bdaddr, dst);
	addr.l2_psm = htobs(avctp ? 23 : 25);

	err = connect(sk, (struct sockaddr *) &addr, sizeof(addr));
	if (err < 0) {
		perror("Unable to connect");
		goto error;
	}

	return sk;

error:
	close(sk);
	return -1;
}

static void do_avdtp_send(int sk, const bdaddr_t *src, const bdaddr_t *dst,
				unsigned char cmd, int invalid, int preconf)
{
	unsigned char buf[672];
	struct avdtp_header *hdr = (void *) buf;
	ssize_t len;

	memset(buf, 0, sizeof(buf));

	switch (cmd) {
	case AVDTP_DISCOVER:
		if (invalid)
			hdr->message_type = 0x01;
		else
			hdr->message_type = AVDTP_MSG_TYPE_COMMAND;
		hdr->packet_type = AVDTP_PKT_TYPE_SINGLE;
		hdr->signal_id = AVDTP_DISCOVER;
		len = write(sk, buf, 2);
		break;

	case AVDTP_GET_CAPABILITIES:
		hdr->message_type = AVDTP_MSG_TYPE_COMMAND;
		hdr->packet_type = AVDTP_PKT_TYPE_SINGLE;
		hdr->signal_id = AVDTP_GET_CAPABILITIES;
		buf[2] = 1 << 2; /* SEID 1 */
		len = write(sk, buf, invalid ? 2 : 3);
		break;

	case AVDTP_SET_CONFIGURATION:
		if (preconf)
			do_avdtp_send(sk, src, dst, cmd, 0, 0);
		hdr->message_type = AVDTP_MSG_TYPE_COMMAND;
		hdr->packet_type = AVDTP_PKT_TYPE_SINGLE;
		hdr->signal_id = AVDTP_SET_CONFIGURATION;
		buf[2] = 1 << 2; /* ACP SEID */
		buf[3] = 1 << 2; /* INT SEID */
		memcpy(&buf[4], media_transport, sizeof(media_transport));
		if (invalid)
			buf[5] = 0x01; /* LOSC != 0 */
		len = write(sk, buf, 4 + sizeof(media_transport));
		break;

	case AVDTP_GET_CONFIGURATION:
		if (preconf)
			do_avdtp_send(sk, src, dst, AVDTP_SET_CONFIGURATION, 0, 0);
		hdr->message_type = AVDTP_MSG_TYPE_COMMAND;
		hdr->packet_type = AVDTP_PKT_TYPE_SINGLE;
		hdr->signal_id = AVDTP_GET_CONFIGURATION;
		if (invalid)
			buf[2] = 13 << 2; /* Invalid ACP SEID */
		else
			buf[2] = 1 << 2; /* Valid ACP SEID */
		len = write(sk, buf, 3);
		break;

	case AVDTP_OPEN:
		if (preconf)
			do_avdtp_send(sk, src, dst, AVDTP_SET_CONFIGURATION, 0, 0);
		hdr->message_type = AVDTP_MSG_TYPE_COMMAND;
		hdr->packet_type = AVDTP_PKT_TYPE_SINGLE;
		hdr->signal_id = AVDTP_OPEN;
		buf[2] = 1 << 2; /* ACP SEID */
		len = write(sk, buf, 3);
		break;

	case AVDTP_START:
		if (preconf)
			do_avdtp_send(sk, src, dst, AVDTP_SET_CONFIGURATION, 0, 0);
		if (!invalid)
			do_avdtp_send(sk, src, dst, AVDTP_OPEN, 0, 0);
		hdr->message_type = AVDTP_MSG_TYPE_COMMAND;
		hdr->packet_type = AVDTP_PKT_TYPE_SINGLE;
		hdr->signal_id = AVDTP_START;
		buf[2] = 1 << 2; /* ACP SEID */
		len = write(sk, buf, 3);
		break;

	case AVDTP_CLOSE:
		if (preconf) {
			do_avdtp_send(sk, src, dst, AVDTP_SET_CONFIGURATION, 0, 0);
			do_avdtp_send(sk, src, dst, AVDTP_OPEN, 0, 0);
		}
		hdr->message_type = AVDTP_MSG_TYPE_COMMAND;
		hdr->packet_type = AVDTP_PKT_TYPE_SINGLE;
		hdr->signal_id = AVDTP_CLOSE;
		if (invalid)
			buf[2] = 13 << 2; /* Invalid ACP SEID */
		else
			buf[2] = 1 << 2; /* Valid ACP SEID */
		len = write(sk, buf, 3);
		break;

	case AVDTP_SUSPEND:
		if (invalid)
			do_avdtp_send(sk, src, dst, AVDTP_OPEN, 0, preconf);
		else
			do_avdtp_send(sk, src, dst, AVDTP_START, 0, preconf);
		hdr->message_type = AVDTP_MSG_TYPE_COMMAND;
		hdr->packet_type = AVDTP_PKT_TYPE_SINGLE;
		hdr->signal_id = AVDTP_SUSPEND;
		buf[2] = 1 << 2; /* ACP SEID */
		len = write(sk, buf, 3);
		break;

	case AVDTP_ABORT:
		do_avdtp_send(sk, src, dst, AVDTP_OPEN, 0, 1);
		hdr->message_type = AVDTP_MSG_TYPE_COMMAND;
		hdr->packet_type = AVDTP_PKT_TYPE_SINGLE;
		hdr->signal_id = AVDTP_ABORT;
		buf[2] = 1 << 2; /* ACP SEID */
		len = write(sk, buf, 3);
		break;

	default:
		hdr->message_type = AVDTP_MSG_TYPE_COMMAND;
		hdr->packet_type = AVDTP_PKT_TYPE_SINGLE;
		hdr->signal_id = cmd;
		len = write(sk, buf, 2);
		break;
	}

	do {
		len = read(sk, buf, sizeof(buf));

		dump_buffer(buf, len);
		dump_avdtp_header(hdr);
	} while (len < 2 || (hdr->message_type != AVDTP_MSG_TYPE_ACCEPT &&
				hdr->message_type != AVDTP_MSG_TYPE_REJECT &&
				hdr->message_type != AVDTP_MSG_TYPE_GEN_REJECT));

	if (cmd == AVDTP_OPEN && len >= 2 &&
				hdr->message_type == AVDTP_MSG_TYPE_ACCEPT)
		media_sock = do_connect(src, dst, 0, 0);
}

static void do_avctp_send(int sk, int invalid)
{
	unsigned char buf[672];
	struct avctp_header *hdr = (void *) buf;
	unsigned char play_pressed[] = { 0x00, 0x48, 0x7c, 0x44, 0x00 };
	ssize_t len;

	memset(buf, 0, sizeof(buf));

	hdr->packet_type = AVCTP_PACKET_SINGLE;
	hdr->cr = AVCTP_COMMAND;
	if (invalid)
		hdr->pid = 0xffff;
	else
		hdr->pid = htons(AV_REMOTE_SVCLASS_ID);

	memcpy(&buf[AVCTP_HEADER_LENGTH], play_pressed, sizeof(play_pressed));

	len = write(sk, buf, AVCTP_HEADER_LENGTH + sizeof(play_pressed));

	len = read(sk, buf, sizeof(buf));

	dump_buffer(buf, len);
	if (len >= AVCTP_HEADER_LENGTH)
		dump_avctp_header(hdr);
}

static void usage(void)
{
	printf("avtest - Audio/Video testing ver %s\n", VERSION);
	printf("Usage:\n"
		"\tavtest [options] [remote address]\n");
	printf("Options:\n"
		"\t--device <hcidev>    HCI device\n"
		"\t--reject <command>   Reject command\n"
		"\t--send <command>     Send command\n"
		"\t--preconf            Configure stream before actual command\n"
		"\t--wait <N>           Wait N seconds before exiting\n"
		"\t--fragment           Use minimum MTU and fragmented messages\n"
		"\t--invalid <command>  Send invalid command\n");
}

static struct option main_options[] = {
	{ "help",	0, 0, 'h' },
	{ "device",	1, 0, 'i' },
	{ "reject",	1, 0, 'r' },
	{ "send",	1, 0, 's' },
	{ "invalid",	1, 0, 'f' },
	{ "preconf",	0, 0, 'c' },
	{ "fragment",   0, 0, 'F' },
	{ "avctp",	0, 0, 'C' },
	{ "wait",	1, 0, 'w' },
	{ 0, 0, 0, 0 }
};

static unsigned char parse_cmd(const char *arg)
{
	if (!strncmp(arg, "discov", 6))
		return AVDTP_DISCOVER;
	else if (!strncmp(arg, "capa", 4))
		return AVDTP_GET_CAPABILITIES;
	else if (!strncmp(arg, "getcapa", 7))
		return AVDTP_GET_CAPABILITIES;
	else if (!strncmp(arg, "setconf", 7))
		return AVDTP_SET_CONFIGURATION;
	else if (!strncmp(arg, "getconf", 7))
		return AVDTP_GET_CONFIGURATION;
	else if (!strncmp(arg, "open", 4))
		return AVDTP_OPEN;
	else if (!strncmp(arg, "start", 5))
		return AVDTP_START;
	else if (!strncmp(arg, "close", 5))
		return AVDTP_CLOSE;
	else if (!strncmp(arg, "suspend", 7))
		return AVDTP_SUSPEND;
	else if (!strncmp(arg, "abort", 7))
		return AVDTP_ABORT;
	else
		return atoi(arg);
}

enum {
	MODE_NONE, MODE_REJECT, MODE_SEND,
};

int main(int argc, char *argv[])
{
	unsigned char cmd = 0x00;
	bdaddr_t src, dst;
	int opt, mode = MODE_NONE, sk, invalid = 0, preconf = 0, fragment = 0;
	int avctp = 0, wait_before_exit = 0;

	bacpy(&src, BDADDR_ANY);
	bacpy(&dst, BDADDR_ANY);

	while ((opt = getopt_long(argc, argv, "+i:r:s:f:hcFCw:",
						main_options, NULL)) != EOF) {
		switch (opt) {
		case 'i':
			if (!strncmp(optarg, "hci", 3))
				hci_devba(atoi(optarg + 3), &src);
			else
				str2ba(optarg, &src);
			break;

		case 'r':
			mode = MODE_REJECT;
			cmd = parse_cmd(optarg);
			break;

		case 'f':
			invalid = 1;
			/* Intentionally missing break */

		case 's':
			mode = MODE_SEND;
			cmd = parse_cmd(optarg);
			break;

		case 'c':
			preconf = 1;
			break;

		case 'F':
			fragment = 1;
			break;

		case 'C':
			avctp = 1;
			break;

		case 'w':
			wait_before_exit = atoi(optarg);
			break;

		case 'h':
		default:
			usage();
			exit(0);
		}
	}

	if (argv[optind])
		str2ba(argv[optind], &dst);

	if (avctp) {
		avctp = mode;
		mode = MODE_SEND;
	}

	switch (mode) {
	case MODE_REJECT:
		do_listen(&src, cmd, fragment);
		break;
	case MODE_SEND:
		sk = do_connect(&src, &dst, avctp, fragment);
		if (sk < 0)
			exit(1);
		if (avctp) {
			if (avctp == MODE_SEND)
				do_avctp_send(sk, invalid);
			else
				process_avctp(sk, cmd);
		} else
			do_avdtp_send(sk, &src, &dst, cmd, invalid, preconf);
		if (wait_before_exit) {
			printf("Waiting %d seconds before exiting\n", wait_before_exit);
			sleep(wait_before_exit);
		}
		if (media_sock >= 0)
			close(media_sock);
		close(sk);
		break;
	default:
		fprintf(stderr, "No operating mode specified!\n");
		exit(1);
	}

	return 0;
}
