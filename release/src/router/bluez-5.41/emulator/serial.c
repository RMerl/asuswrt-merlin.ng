/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2011-2014  Intel Corporation
 *  Copyright (C) 2004-2010  Marcel Holtmann <marcel@holtmann.org>
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
#include <errno.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/param.h>
#include <sys/epoll.h>
#include <sys/uio.h>

#include "lib/bluetooth.h"
#include "lib/hci.h"

#include "src/shared/mainloop.h"
#include "btdev.h"
#include "serial.h"

#define uninitialized_var(x) x = x

struct serial {
	enum serial_type type;
	uint16_t id;
	int fd;
	char path[PATH_MAX];
	struct btdev *btdev;
	uint8_t *pkt_data;
	uint8_t pkt_type;
	uint16_t pkt_expect;
	uint16_t pkt_len;
	uint16_t pkt_offset;
};

static void open_pty(struct serial *serial);

static void serial_destroy(void *user_data)
{
	struct serial *serial = user_data;

	btdev_destroy(serial->btdev);
	serial->btdev = NULL;

	close(serial->fd);
	serial->fd = -1;
}

static void serial_write_callback(const struct iovec *iov, int iovlen,
							void *user_data)
{
	struct serial *serial = user_data;
	ssize_t written;

	written = writev(serial->fd, iov, iovlen);
	if (written < 0)
		return;
}

static void serial_read_callback(int fd, uint32_t events, void *user_data)
{
	struct serial *serial = user_data;
	static uint8_t buf[4096];
	uint8_t *ptr = buf;
	ssize_t len;
	uint16_t count;

	if (events & (EPOLLERR | EPOLLHUP)) {
		mainloop_remove_fd(serial->fd);
		open_pty(serial);
		return;
	}

again:
	len = read(serial->fd, buf + serial->pkt_offset,
			sizeof(buf) - serial->pkt_offset);
	if (len < 0) {
		if (errno == EAGAIN)
			goto again;
		return;
	}

	if (!serial->btdev)
		return;

	count = serial->pkt_offset + len;

	while (count > 0) {
		hci_command_hdr *cmd_hdr;

		if (!serial->pkt_data) {
			serial->pkt_type = ptr[0];

			switch (serial->pkt_type) {
			case HCI_COMMAND_PKT:
				if (count < HCI_COMMAND_HDR_SIZE + 1) {
					serial->pkt_offset += len;
					return;
				}
				cmd_hdr = (hci_command_hdr *) (ptr + 1);
				serial->pkt_expect = HCI_COMMAND_HDR_SIZE +
							cmd_hdr->plen + 1;
				serial->pkt_data = malloc(serial->pkt_expect);
				serial->pkt_len = 0;
				break;
			default:
				printf("packet error\n");
				return;
			}

			serial->pkt_offset = 0;
		}

		if (count >= serial->pkt_expect) {
			memcpy(serial->pkt_data + serial->pkt_len,
						ptr, serial->pkt_expect);
			ptr += serial->pkt_expect;
			count -= serial->pkt_expect;

			btdev_receive_h4(serial->btdev, serial->pkt_data,
					serial->pkt_len + serial->pkt_expect);

			free(serial->pkt_data);
			serial->pkt_data = NULL;
		} else {
			memcpy(serial->pkt_data + serial->pkt_len, ptr, count);
			serial->pkt_len += count;
			serial->pkt_expect -= count;
			count = 0;
		}
	}
}

static void open_pty(struct serial *serial)
{
	enum btdev_type uninitialized_var(type);

	serial->fd = posix_openpt(O_RDWR | O_NOCTTY);
	if (serial->fd < 0) {
		perror("Failed to get master pseudo terminal");
		return;
	}

	if (grantpt(serial->fd) < 0) {
		perror("Failed to grant slave pseudo terminal");
		close(serial->fd);
		serial->fd = -1;
		return;
	}

	if (unlockpt(serial->fd) < 0) {
		perror("Failed to unlock slave pseudo terminal");
		close(serial->fd);
		serial->fd = -1;
		return;
	}

	ptsname_r(serial->fd, serial->path, sizeof(serial->path));

	printf("Pseudo terminal at %s\n", serial->path);

	switch (serial->type) {
	case SERIAL_TYPE_BREDRLE:
		type = BTDEV_TYPE_BREDRLE;
		break;
	case SERIAL_TYPE_BREDR:
		type = BTDEV_TYPE_BREDR;
		break;
	case SERIAL_TYPE_LE:
		type = BTDEV_TYPE_LE;
		break;
	case SERIAL_TYPE_AMP:
		type = BTDEV_TYPE_AMP;
		break;
	}

	serial->btdev = btdev_create(type, serial->id);
	if (!serial->btdev) {
		close(serial->fd);
		serial->fd = -1;
		return;
	}

	btdev_set_send_handler(serial->btdev, serial_write_callback, serial);

	if (mainloop_add_fd(serial->fd, EPOLLIN, serial_read_callback,
						serial, serial_destroy) < 0) {
		btdev_destroy(serial->btdev);
		serial->btdev = NULL;
		close(serial->fd);
		serial->fd = -1;
		return;
	}
}

struct serial *serial_open(enum serial_type type)
{
	struct serial *serial;
	enum btdev_type uninitialized_var(dev_type);

	serial = malloc(sizeof(*serial));
	if (!serial)
		return NULL;

	memset(serial, 0, sizeof(*serial));
	serial->type = type;
	serial->id = 0x42;

	open_pty(serial);

	return serial;
}

void serial_close(struct serial *serial)
{
	if (!serial)
		return;

	mainloop_remove_fd(serial->fd);

	free(serial);
}
