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
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/uio.h>

#include "lib/bluetooth.h"
#include "lib/hci.h"

#include "src/shared/mainloop.h"
#include "monitor/bt.h"
#include "btdev.h"
#include "vhci.h"

#define uninitialized_var(x) x = x

struct vhci {
	enum vhci_type type;
	int fd;
	struct btdev *btdev;
};

static void vhci_destroy(void *user_data)
{
	struct vhci *vhci = user_data;

	btdev_destroy(vhci->btdev);

	close(vhci->fd);

	free(vhci);
}

static void vhci_write_callback(const struct iovec *iov, int iovlen,
							void *user_data)
{
	struct vhci *vhci = user_data;
	ssize_t written;

	written = writev(vhci->fd, iov, iovlen);
	if (written < 0)
		return;
}

static void vhci_read_callback(int fd, uint32_t events, void *user_data)
{
	struct vhci *vhci = user_data;
	unsigned char buf[4096];
	ssize_t len;

	if (events & (EPOLLERR | EPOLLHUP))
		return;

	len = read(vhci->fd, buf, sizeof(buf));
	if (len < 1)
		return;

	switch (buf[0]) {
	case BT_H4_CMD_PKT:
	case BT_H4_ACL_PKT:
	case BT_H4_SCO_PKT:
		btdev_receive_h4(vhci->btdev, buf, len);
		break;
	}
}

struct vhci *vhci_open(enum vhci_type type)
{
	struct vhci *vhci;
	enum btdev_type uninitialized_var(btdev_type);
	unsigned char uninitialized_var(ctrl_type);
	unsigned char setup_cmd[2];
	static uint8_t id = 0x23;

	switch (type) {
	case VHCI_TYPE_BREDRLE:
		btdev_type = BTDEV_TYPE_BREDRLE;
		ctrl_type = HCI_PRIMARY;
		break;
	case VHCI_TYPE_BREDR:
		btdev_type = BTDEV_TYPE_BREDR;
		ctrl_type = HCI_PRIMARY;
		break;
	case VHCI_TYPE_LE:
		btdev_type = BTDEV_TYPE_LE;
		ctrl_type = HCI_PRIMARY;
		break;
	case VHCI_TYPE_AMP:
		btdev_type = BTDEV_TYPE_AMP;
		ctrl_type = HCI_AMP;
		break;
	}

	vhci = malloc(sizeof(*vhci));
	if (!vhci)
		return NULL;

	memset(vhci, 0, sizeof(*vhci));
	vhci->type = type;

	vhci->fd = open("/dev/vhci", O_RDWR | O_NONBLOCK);
	if (vhci->fd < 0) {
		free(vhci);
		return NULL;
	}

	setup_cmd[0] = HCI_VENDOR_PKT;
	setup_cmd[1] = ctrl_type;

	if (write(vhci->fd, setup_cmd, sizeof(setup_cmd)) < 0) {
		close(vhci->fd);
		free(vhci);
		return NULL;
	}

	vhci->btdev = btdev_create(btdev_type, id++);
	if (!vhci->btdev) {
		close(vhci->fd);
		free(vhci);
		return NULL;
	}

	btdev_set_send_handler(vhci->btdev, vhci_write_callback, vhci);

	if (mainloop_add_fd(vhci->fd, EPOLLIN, vhci_read_callback,
						vhci, vhci_destroy) < 0) {
		btdev_destroy(vhci->btdev);
		close(vhci->fd);
		free(vhci);
		return NULL;
	}

	return vhci;
}

void vhci_close(struct vhci *vhci)
{
	if (!vhci)
		return;

	mainloop_remove_fd(vhci->fd);
}
