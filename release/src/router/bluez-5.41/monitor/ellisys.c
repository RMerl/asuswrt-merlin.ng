/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2011-2014  Intel Corporation
 *  Copyright (C) 2002-2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/uio.h>

#include <netdb.h>
#include <arpa/inet.h>

#include "src/shared/btsnoop.h"
#include "ellisys.h"

static int ellisys_fd = -1;
static uint16_t ellisys_index = 0xffff;

void ellisys_enable(const char *server, uint16_t port)
{
	struct sockaddr_in addr;
	int fd;

	if (ellisys_fd >= 0) {
		fprintf(stderr, "Ellisys injection already enabled\n");
		return;
	}

	fd = socket(PF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
	if (fd < 0) {
		perror("Failed to open UDP injection socket");
		return;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(server);
	addr.sin_port = htons(port);

	if (connect(fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		perror("Failed to connect UDP injection socket");
		close(fd);
		return;
	}

	ellisys_fd = fd;
}

void ellisys_inject_hci(struct timeval *tv, uint16_t index, uint16_t opcode,
					const void *data, uint16_t size)
{
	uint8_t msg[] = {
		/* HCI Injection Service, Version 1 */
		0x02, 0x00, 0x01,
		/* DateTimeNs Object */
		0x02, 0x00, 0x00, 0x00, 0x00,
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		/* Bitrate Object, 12000000 bps */
		0x80, 0x00, 0x1b, 0x37, 0x4b,
		/* HCI Packet Type Object */
		0x81, 0x00,
		/* HCI Packet Data Object */
		0x82
	};
	long nsec;
	time_t t;
	struct tm tm;
	struct iovec iov[2];
	int iovcnt;

	if (!tv)
		return;

	if (ellisys_fd < 0)
		return;

	if (ellisys_index == 0xffff)
		ellisys_index = index;

	if (index != ellisys_index)
		return;

	t = tv->tv_sec;
	localtime_r(&t, &tm);

	nsec = ((tm.tm_sec + (tm.tm_min * 60) +
			(tm.tm_hour * 3600)) * 1000000l + tv->tv_usec) * 1000l;

	msg[4]  = (1900 + tm.tm_year) & 0xff;
	msg[5]  = (1900 + tm.tm_year) >> 8;
	msg[6]  = (tm.tm_mon + 1) & 0xff;
	msg[7]  = tm.tm_mday & 0xff;
	msg[8]  = (nsec & 0x0000000000ffl);
	msg[9]  = (nsec & 0x00000000ff00l) >> 8;
	msg[10] = (nsec & 0x000000ff0000l) >> 16;
	msg[11] = (nsec & 0x0000ff000000l) >> 24;
	msg[12] = (nsec & 0x00ff00000000l) >> 32;
	msg[13] = (nsec & 0xff0000000000l) >> 40;

	switch (opcode) {
	case BTSNOOP_OPCODE_COMMAND_PKT:
		msg[20] = 0x01;
		break;
	case BTSNOOP_OPCODE_EVENT_PKT:
		msg[20] = 0x84;
		break;
	case BTSNOOP_OPCODE_ACL_TX_PKT:
		msg[20] = 0x02;
		break;
	case BTSNOOP_OPCODE_ACL_RX_PKT:
		msg[20] = 0x82;
		break;
	case BTSNOOP_OPCODE_SCO_TX_PKT:
		msg[20] = 0x03;
		break;
	case BTSNOOP_OPCODE_SCO_RX_PKT:
		msg[20] = 0x83;
		break;
	default:
		return;
	}

	iov[0].iov_base = msg;
	iov[0].iov_len  = sizeof(msg);

	if (size > 0) {
		iov[1].iov_base = (void *) data;
		iov[1].iov_len  = size;
		iovcnt = 2;
	} else
		iovcnt = 1;

	if (writev(ellisys_fd, iov, iovcnt) < 0)
		perror("Failed to send Ellisys injection packet");
}
