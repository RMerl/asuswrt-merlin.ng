// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2018  Intel Corporation. All rights reserved.
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define _GNU_SOURCE
#include <stdio.h>
#include <errno.h>
#include <inttypes.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <signal.h>
#include <sys/socket.h>

#include "lib/bluetooth.h"
#include "lib/hci.h"

#include "src/shared/util.h"
#include "src/shared/log.h"

struct log_hdr {
	uint16_t opcode;
	uint16_t index;
	uint16_t len;
	uint8_t  priority;
	uint8_t  ident_len;
} __attribute__((packed));

struct log_l2cap_hdr {
	uint16_t cid;
	uint16_t psm;
} __attribute__((packed));

static int log_fd = -1;

int bt_log_sendmsg(uint16_t index, const char *label, int level,
					struct iovec *io, size_t io_len)
{
	struct log_hdr hdr;
	struct msghdr msg;
	struct iovec iov[5];
	size_t i;
	int err;

	if (io_len > 3)
		return -EMSGSIZE;

	log_fd = bt_log_open();
	if (log_fd < 0)
		return log_fd;

	hdr.opcode = cpu_to_le16(0x0000);
	hdr.index = cpu_to_le16(index);
	hdr.ident_len = strlen(label) + 1;
	hdr.len = cpu_to_le16(2 + hdr.ident_len);
	hdr.priority = level;

	iov[0].iov_base = &hdr;
	iov[0].iov_len = sizeof(hdr);

	iov[1].iov_base = (void *) label;
	iov[1].iov_len = hdr.ident_len;

	memset(&msg, 0, sizeof(msg));
	msg.msg_iov = iov;
	msg.msg_iovlen = 2;

	for (i = 0; i < io_len; i++) {
		iov[i + 2] = io[i];
		hdr.len += io[i].iov_len;
		msg.msg_iovlen++;
	}

	err = sendmsg(log_fd, &msg, 0);
	if (err < 0) {
		err = -errno;
		close(log_fd);
		log_fd = -1;
	}

	return err;
}

int bt_log_open(void)
{
	struct sockaddr_hci addr;
	int fd;
	static int err;

	if (err < 0)
		return err;

	if (log_fd >= 0)
		return log_fd;

	fd = socket(PF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI);
	if (fd < 0) {
		err = -errno;
		return -errno;
	}

	memset(&addr, 0, sizeof(addr));
	addr.hci_family = AF_BLUETOOTH;
	addr.hci_dev = HCI_DEV_NONE;
	addr.hci_channel = HCI_CHANNEL_LOGGING;

	err = bind(fd, (struct sockaddr *) &addr, sizeof(addr));
	if (err < 0) {
		err = -errno;
		close(fd);
		return err;
	}

	log_fd = fd;

	return fd;
}

int bt_log_vprintf(uint16_t index, const char *label, int level,
					const char *format, va_list ap)
{
	struct iovec iov;
	char *str;
	int len;

	len = vasprintf(&str, format, ap);
	if (len < 0)
		return errno;

	len = strlen(str);

	/* Replace new line since btmon already adds it */
	if (len > 1 && str[len - 1] == '\n') {
		str[len - 1] = '\0';
		len--;
	}

	iov.iov_base = str;
	iov.iov_len = len + 1;

	len = bt_log_sendmsg(index, label, level, &iov, 1);

	free(str);

	return len;
}

int bt_log_printf(uint16_t index, const char *label, int level,
						const char *format, ...)
{
	va_list ap;
	int err;

	va_start(ap, format);
	err = bt_log_vprintf(index, label, level, format, ap);
	va_end(ap);

	return err;
}

void bt_log_close(void)
{
	if (log_fd < 0)
		return;

	close(log_fd);
	log_fd = -1;
}
