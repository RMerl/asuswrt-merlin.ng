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
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include "lib/bluetooth.h"
#include "lib/hci.h"
#include "lib/hci_lib.h"

#include "src/shared/mainloop.h"

#include "packet.h"
#include "hcidump.h"

struct hcidump_data {
	uint16_t index;
	int fd;
};

static void free_data(void *user_data)
{
	struct hcidump_data *data = user_data;

	close(data->fd);

	free(data);
}

static int open_hci_dev(uint16_t index)
{
	struct sockaddr_hci addr;
	struct hci_filter flt;
	int fd, opt = 1;

	fd = socket(AF_BLUETOOTH, SOCK_RAW | SOCK_CLOEXEC, BTPROTO_HCI);
	if (fd < 0) {
		perror("Failed to open channel");
		return -1;
	}

	/* Setup filter */
	hci_filter_clear(&flt);
	hci_filter_all_ptypes(&flt);
	hci_filter_all_events(&flt);

	if (setsockopt(fd, SOL_HCI, HCI_FILTER, &flt, sizeof(flt)) < 0) {
		perror("Failed to set HCI filter");
		close(fd);
		return -1;
	}

	if (setsockopt(fd, SOL_HCI, HCI_DATA_DIR, &opt, sizeof(opt)) < 0) {
		perror("Failed to enable HCI data direction info");
		close(fd);
		return -1;
	}

	if (setsockopt(fd, SOL_HCI, HCI_TIME_STAMP, &opt, sizeof(opt)) < 0) {
		perror("Failed to enable HCI time stamps");
		close(fd);
		return -1;
	}

	memset(&addr, 0, sizeof(addr));
	addr.hci_family = AF_BLUETOOTH;
	addr.hci_dev = index;
	addr.hci_channel = HCI_CHANNEL_RAW;

	if (bind(fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		perror("Failed to bind channel");
		close(fd);
		return -1;
	}

	return fd;
}

static void device_callback(int fd, uint32_t events, void *user_data)
{
	struct hcidump_data *data = user_data;
	unsigned char buf[HCI_MAX_FRAME_SIZE * 2];
	unsigned char control[64];
	struct msghdr msg;
	struct iovec iov;

	if (events & (EPOLLERR | EPOLLHUP)) {
		mainloop_remove_fd(fd);
		return;
	}

	iov.iov_base = buf;
	iov.iov_len = sizeof(buf);

	memset(&msg, 0, sizeof(msg));
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	msg.msg_control = control;
	msg.msg_controllen = sizeof(control);

	while (1) {
		struct cmsghdr *cmsg;
		struct timeval *tv = NULL;
		struct timeval ctv;
		int dir = -1;
		ssize_t len;

		len = recvmsg(fd, &msg, MSG_DONTWAIT);
		if (len < 0)
			break;

		for (cmsg = CMSG_FIRSTHDR(&msg); cmsg != NULL;
					cmsg = CMSG_NXTHDR(&msg, cmsg)) {
			if (cmsg->cmsg_level != SOL_HCI)
				continue;

			switch (cmsg->cmsg_type) {
			case HCI_DATA_DIR:
				memcpy(&dir, CMSG_DATA(cmsg), sizeof(dir));
				break;
			case HCI_CMSG_TSTAMP:
				memcpy(&ctv, CMSG_DATA(cmsg), sizeof(ctv));
				tv = &ctv;
				break;
			}
		}

		if (dir < 0 || len < 1)
			continue;

		switch (buf[0]) {
		case HCI_COMMAND_PKT:
			packet_hci_command(tv, NULL, data->index,
							buf + 1, len - 1);
			break;
		case HCI_EVENT_PKT:
			packet_hci_event(tv, NULL, data->index,
							buf + 1, len - 1);
			break;
		case HCI_ACLDATA_PKT:
			packet_hci_acldata(tv, NULL, data->index, !!dir,
							buf + 1, len - 1);
			break;
		case HCI_SCODATA_PKT:
			packet_hci_scodata(tv, NULL, data->index, !!dir,
							buf + 1, len - 1);
			break;
		}
	}
}

static void open_device(uint16_t index)
{
	struct hcidump_data *data;

	data = malloc(sizeof(*data));
	if (!data)
		return;

	memset(data, 0, sizeof(*data));
	data->index = index;

	data->fd = open_hci_dev(index);
	if (data->fd < 0) {
		free(data);
		return;
	}

	mainloop_add_fd(data->fd, EPOLLIN, device_callback, data, free_data);
}

static void device_info(int fd, uint16_t index, uint8_t *type, uint8_t *bus,
						bdaddr_t *bdaddr, char *name)
{
	struct hci_dev_info di;

	memset(&di, 0, sizeof(di));
	di.dev_id = index;

	if (ioctl(fd, HCIGETDEVINFO, (void *) &di) < 0) {
		perror("Failed to get device information");
		return;
	}

	*type = di.type >> 4;
	*bus = di.type & 0x0f;

	bacpy(bdaddr, &di.bdaddr);
	memcpy(name, di.name, 8);
}

static void device_list(int fd, int max_dev)
{
	struct hci_dev_list_req *dl;
	struct hci_dev_req *dr;
	int i;

	dl = malloc(max_dev * sizeof(*dr) + sizeof(*dl));
	if (!dl) {
		perror("Failed to allocate device list memory");
		return;
	}

	memset(dl, 0, max_dev * sizeof(*dr) + sizeof(*dl));
	dl->dev_num = max_dev;

	dr = dl->dev_req;

	if (ioctl(fd, HCIGETDEVLIST, (void *) dl) < 0) {
		perror("Failed to get device list");
		goto done;
	}

	for (i = 0; i < dl->dev_num; i++, dr++) {
		struct timeval tmp_tv, *tv = NULL;
		uint8_t type = 0xff, bus = 0xff;
		char str[18], name[8] = "";
		bdaddr_t bdaddr;

		bacpy(&bdaddr, BDADDR_ANY);

		if (!gettimeofday(&tmp_tv, NULL))
			tv = &tmp_tv;

		device_info(fd, dr->dev_id, &type, &bus, &bdaddr, name);
		ba2str(&bdaddr, str);
		packet_new_index(tv, dr->dev_id, str, type, bus, name);
		open_device(dr->dev_id);
	}

done:
	free(dl);
}

static int open_stack_internal(void)
{
	struct sockaddr_hci addr;
	struct hci_filter flt;
	int fd, opt = 1;

	fd = socket(AF_BLUETOOTH, SOCK_RAW | SOCK_CLOEXEC, BTPROTO_HCI);
	if (fd < 0) {
		perror("Failed to open channel");
		return -1;
	}

	/* Setup filter */
	hci_filter_clear(&flt);
	hci_filter_set_ptype(HCI_EVENT_PKT, &flt);
	hci_filter_set_event(EVT_STACK_INTERNAL, &flt);

	if (setsockopt(fd, SOL_HCI, HCI_FILTER, &flt, sizeof(flt)) < 0) {
		perror("Failed to set HCI filter");
		close(fd);
		return -1;
	}

	if (setsockopt(fd, SOL_HCI, HCI_TIME_STAMP, &opt, sizeof(opt)) < 0) {
		perror("Failed to enable HCI time stamps");
		close(fd);
		return -1;
	}

	memset(&addr, 0, sizeof(addr));
	addr.hci_family = AF_BLUETOOTH;
	addr.hci_dev = HCI_DEV_NONE;
	addr.hci_channel = HCI_CHANNEL_RAW;

	if (bind(fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		perror("Failed to bind channel");
		close(fd);
		return -1;
	}

	device_list(fd, HCI_MAX_DEV);

	return fd;
}

static void stack_internal_callback(int fd, uint32_t events, void *user_data)
{
	unsigned char buf[HCI_MAX_FRAME_SIZE];
	unsigned char control[32];
	struct msghdr msg;
	struct iovec iov;
	struct cmsghdr *cmsg;
	ssize_t len;
	hci_event_hdr *eh;
	evt_stack_internal *si;
	evt_si_device *sd;
	struct timeval *tv = NULL;
	struct timeval ctv;
	uint8_t type = 0xff, bus = 0xff;
	char str[18], name[8] = "";
	bdaddr_t bdaddr;

	bacpy(&bdaddr, BDADDR_ANY);

	if (events & (EPOLLERR | EPOLLHUP)) {
		mainloop_remove_fd(fd);
		return;
	}

	iov.iov_base = buf;
	iov.iov_len = sizeof(buf);

	memset(&msg, 0, sizeof(msg));
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	msg.msg_control = control;
	msg.msg_controllen = sizeof(control);

	len = recvmsg(fd, &msg, MSG_DONTWAIT);
	if (len < 0)
		return;

	for (cmsg = CMSG_FIRSTHDR(&msg); cmsg != NULL;
					cmsg = CMSG_NXTHDR(&msg, cmsg)) {
		if (cmsg->cmsg_level != SOL_HCI)
			continue;

		switch (cmsg->cmsg_type) {
		case HCI_CMSG_TSTAMP:
			memcpy(&ctv, CMSG_DATA(cmsg), sizeof(ctv));
			tv = &ctv;
			break;
		}
	}

	if (len < 1 + HCI_EVENT_HDR_SIZE + EVT_STACK_INTERNAL_SIZE +
							EVT_SI_DEVICE_SIZE)
		return;

	if (buf[0] != HCI_EVENT_PKT)
		return;

	eh = (hci_event_hdr *) (buf + 1);
	if (eh->evt != EVT_STACK_INTERNAL)
		return;

	si = (evt_stack_internal *) (buf + 1 + HCI_EVENT_HDR_SIZE);
	if (si->type != EVT_SI_DEVICE)
		return;

	sd = (evt_si_device *) &si->data;

	switch (sd->event) {
	case HCI_DEV_REG:
		device_info(fd, sd->dev_id, &type, &bus, &bdaddr, name);
		ba2str(&bdaddr, str);
		packet_new_index(tv, sd->dev_id, str, type, bus, name);
		open_device(sd->dev_id);
		break;
	case HCI_DEV_UNREG:
		ba2str(&bdaddr, str);
		packet_del_index(tv, sd->dev_id, str);
		break;
	}
}

int hcidump_tracing(void)
{
	struct hcidump_data *data;

	data = malloc(sizeof(*data));
	if (!data)
		return -1;

	memset(data, 0, sizeof(*data));
	data->index = HCI_DEV_NONE;

	data->fd = open_stack_internal();
	if (data->fd < 0) {
		free(data);
		return -1;
	}

	mainloop_add_fd(data->fd, EPOLLIN, stack_internal_callback,
							data, free_data);

	return 0;
}
