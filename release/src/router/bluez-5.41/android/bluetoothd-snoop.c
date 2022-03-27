/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2013-2014  Intel Corporation. All rights reserved.
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

#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#if defined(ANDROID)
#include <sys/capability.h>
#endif

#include "lib/bluetooth.h"
#include "lib/hci.h"
#include "lib/mgmt.h"

#include "src/shared/mainloop.h"
#include "src/shared/btsnoop.h"
#include "src/log.h"

#define DEFAULT_SNOOP_FILE "/sdcard/btsnoop_hci.log"

static struct btsnoop *snoop = NULL;
static uint8_t monitor_buf[BTSNOOP_MAX_PACKET_SIZE];
static int monitor_fd = -1;

static void signal_callback(int signum, void *user_data)
{
	switch (signum) {
	case SIGINT:
	case SIGTERM:
		mainloop_quit();
		break;
	}
}

static uint32_t get_flags_from_opcode(uint16_t opcode)
{
	switch (opcode) {
	case BTSNOOP_OPCODE_NEW_INDEX:
	case BTSNOOP_OPCODE_DEL_INDEX:
		break;
	case BTSNOOP_OPCODE_COMMAND_PKT:
		return 0x02;
	case BTSNOOP_OPCODE_EVENT_PKT:
		return 0x03;
	case BTSNOOP_OPCODE_ACL_TX_PKT:
		return 0x00;
	case BTSNOOP_OPCODE_ACL_RX_PKT:
		return 0x01;
	case BTSNOOP_OPCODE_SCO_TX_PKT:
	case BTSNOOP_OPCODE_SCO_RX_PKT:
		break;
	case BTSNOOP_OPCODE_OPEN_INDEX:
	case BTSNOOP_OPCODE_CLOSE_INDEX:
		break;
	}

	return 0xff;
}

static void data_callback(int fd, uint32_t events, void *user_data)
{
	unsigned char control[32];
	struct mgmt_hdr hdr;
	struct msghdr msg;
	struct iovec iov[2];

	if (events & (EPOLLERR | EPOLLHUP)) {
		mainloop_remove_fd(monitor_fd);
		return;
	}

	iov[0].iov_base = &hdr;
	iov[0].iov_len = MGMT_HDR_SIZE;
	iov[1].iov_base = monitor_buf;
	iov[1].iov_len = sizeof(monitor_buf);

	memset(&msg, 0, sizeof(msg));
	msg.msg_iov = iov;
	msg.msg_iovlen = 2;
	msg.msg_control = control;
	msg.msg_controllen = sizeof(control);

	while (true) {
		struct cmsghdr *cmsg;
		struct timeval *tv = NULL;
		struct timeval ctv;
		uint16_t opcode, index, pktlen;
		uint32_t flags;
		ssize_t len;

		len = recvmsg(monitor_fd, &msg, MSG_DONTWAIT);
		if (len < 0)
			break;

		if (len < MGMT_HDR_SIZE)
			break;

		for (cmsg = CMSG_FIRSTHDR(&msg); cmsg != NULL;
					cmsg = CMSG_NXTHDR(&msg, cmsg)) {
			if (cmsg->cmsg_level != SOL_SOCKET)
				continue;

			if (cmsg->cmsg_type == SCM_TIMESTAMP) {
				memcpy(&ctv, CMSG_DATA(cmsg), sizeof(ctv));
				tv = &ctv;
			}
		}

		opcode = btohs(hdr.opcode);
		index  = btohs(hdr.index);
		pktlen = btohs(hdr.len);

		if (index)
			continue;

		flags = get_flags_from_opcode(opcode);
		if (flags != 0xff)
			btsnoop_write(snoop, tv, flags, 0, monitor_buf, pktlen);
	}
}

static int open_monitor(const char *path)
{
	struct sockaddr_hci addr;
	int opt = 1;

	snoop = btsnoop_create(path, BTSNOOP_FORMAT_HCI);
	if (!snoop)
		return -1;

	monitor_fd = socket(AF_BLUETOOTH, SOCK_RAW | SOCK_CLOEXEC, BTPROTO_HCI);
	if (monitor_fd < 0)
		goto failed;

	memset(&addr, 0, sizeof(addr));
	addr.hci_family = AF_BLUETOOTH;
	addr.hci_dev = HCI_DEV_NONE;
	addr.hci_channel = HCI_CHANNEL_MONITOR;

	if (bind(monitor_fd, (struct sockaddr *) &addr, sizeof(addr)) < 0)
		goto failed_close;

	if (setsockopt(monitor_fd, SOL_SOCKET, SO_TIMESTAMP, &opt, sizeof(opt))
									< 0)
		goto failed_close;

	mainloop_add_fd(monitor_fd, EPOLLIN, data_callback, NULL, NULL);

	return 0;

failed_close:
	close(monitor_fd);
	monitor_fd = -1;

failed:
	btsnoop_unref(snoop);
	snoop = NULL;

	return -1;
}

static void close_monitor(void)
{
	btsnoop_unref(snoop);
	snoop = NULL;

	close(monitor_fd);
	monitor_fd = -1;
}

static void set_capabilities(void)
{
#if defined(ANDROID)
	struct __user_cap_header_struct header;
	struct __user_cap_data_struct cap;

	header.version = _LINUX_CAPABILITY_VERSION;
	header.pid = 0;

	/*
	 * CAP_NET_RAW: for snooping
	 * CAP_DAC_READ_SEARCH: override path search permissions
	 */
	cap.effective = cap.permitted =
		CAP_TO_MASK(CAP_NET_RAW) |
		CAP_TO_MASK(CAP_DAC_READ_SEARCH);
	cap.inheritable = 0;

	/* TODO: Move to cap_set_proc once bionic support it */
	if (capset(&header, &cap) < 0)
		exit(EXIT_FAILURE);
#endif
}

int main(int argc, char *argv[])
{
	const char *path;
	sigset_t mask;

	__btd_log_init(NULL, 0);

	DBG("");

	set_capabilities();

	if (argc > 1)
		path = argv[1];
	else
		path = DEFAULT_SNOOP_FILE;

	mainloop_init();

	sigemptyset(&mask);
	sigaddset(&mask, SIGINT);
	sigaddset(&mask, SIGTERM);

	mainloop_set_signal(&mask, signal_callback, NULL, NULL);

	if (!strcmp(DEFAULT_SNOOP_FILE, path))
		rename(DEFAULT_SNOOP_FILE, DEFAULT_SNOOP_FILE ".old");

	if (open_monitor(path) < 0) {
		error("bluetoothd_snoop: start failed");
		return EXIT_FAILURE;
	}

	info("bluetoothd_snoop: started");

	mainloop_run();

	close_monitor();

	info("bluetoothd_snoop: stopped");

	__btd_log_cleanup();

	return EXIT_SUCCESS;
}
