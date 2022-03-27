/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2011-2012  Intel Corporation
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

#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <time.h>

#include "src/shared/util.h"
#include "src/shared/mainloop.h"

#include "phy.h"

#define BT_PHY_PORT 45023

struct bt_phy {
	volatile int ref_count;
	int rx_fd;
	int tx_fd;
	uint64_t id;
	bt_phy_callback_func_t callback;
	void *user_data;
};

struct bt_phy_hdr {
	uint64_t id;
	uint32_t flags;
	uint16_t type;
	uint16_t len;
} __attribute__ ((packed));

static bool get_random_bytes(void *buf, size_t num_bytes)
{
	ssize_t len;
	int fd;

	fd = open("/dev/urandom", O_RDONLY);
	if (fd < 0)
		return false;

	len = read(fd, buf, num_bytes);

	close(fd);

	if (len < 0)
		return false;

	return true;
}

static void phy_rx_callback(int fd, uint32_t events, void *user_data)
{
	struct bt_phy *phy = user_data;
	struct msghdr msg;
	struct iovec iov[2];
	struct bt_phy_hdr hdr;
	unsigned char buf[4096];
	ssize_t len;

	if (events & (EPOLLERR | EPOLLHUP)) {
		mainloop_remove_fd(fd);
		return;
	}

	iov[0].iov_base = &hdr;
	iov[0].iov_len = sizeof(hdr);
	iov[1].iov_base = buf;
	iov[1].iov_len = sizeof(buf);

	memset(&msg, 0, sizeof(msg));
	msg.msg_iov = iov;
	msg.msg_iovlen = 2;

	len = recvmsg(phy->rx_fd, &msg, MSG_DONTWAIT);
	if (len < 0)
		return;

	if ((size_t) len < sizeof(hdr))
		return;

	if (le64_to_cpu(hdr.id) == phy->id)
		return;

	if (len - sizeof(hdr) != le16_to_cpu(hdr.len))
		return;

	if (phy->callback)
		phy->callback(le16_to_cpu(hdr.type),
				buf, len - sizeof(hdr), phy->user_data);
}

static int create_rx_socket(void)
{
	struct sockaddr_in addr;
	int fd, opt = 1;

	fd = socket(PF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
	if (fd < 0)
		return -1;

	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(BT_PHY_PORT);
	addr.sin_addr.s_addr = INADDR_BROADCAST;

	if (bind(fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		close(fd);
		return -1;
	}

	return fd;
}

static int create_tx_socket(void)
{
	int fd, opt = 1;

	fd = socket(PF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
	if (fd < 0)
		return -1;

	setsockopt(fd, SOL_SOCKET, SO_BROADCAST, &opt, sizeof(opt));

	return fd;
}

struct bt_phy *bt_phy_new(void)
{
	struct bt_phy *phy;

	phy = calloc(1, sizeof(*phy));
	if (!phy)
		return NULL;

	phy->rx_fd = create_rx_socket();
	if (phy->rx_fd < 0) {
		free(phy);
		return NULL;
	}

	phy->tx_fd = create_tx_socket();
	if (phy->tx_fd < 0) {
		close(phy->rx_fd);
		free(phy);
		return NULL;
	}

	mainloop_add_fd(phy->rx_fd, EPOLLIN, phy_rx_callback, phy, NULL);

	if (!get_random_bytes(&phy->id, sizeof(phy->id))) {
		srandom(time(NULL));
		phy->id = random();
	}

	bt_phy_send(phy, BT_PHY_PKT_NULL, NULL, 0);

	return bt_phy_ref(phy);
}

struct bt_phy *bt_phy_ref(struct bt_phy *phy)
{
	if (!phy)
		return NULL;

	__sync_fetch_and_add(&phy->ref_count, 1);

	return phy;
}

void bt_phy_unref(struct bt_phy *phy)
{
	if (!phy)
		return;

	if (__sync_sub_and_fetch(&phy->ref_count, 1))
		return;

	mainloop_remove_fd(phy->rx_fd);

	close(phy->tx_fd);
	close(phy->rx_fd);

	free(phy);
}

bool bt_phy_send(struct bt_phy *phy, uint16_t type,
					const void *data, size_t size)
{
	return bt_phy_send_vector(phy, type, data, size, NULL, 0, NULL, 0);
}

bool bt_phy_send_vector(struct bt_phy *phy, uint16_t type,
					const void *data1, size_t size1,
					const void *data2, size_t size2,
					const void *data3, size_t size3)
{
	struct bt_phy_hdr hdr;
	struct sockaddr_in addr;
	struct msghdr msg;
	struct iovec iov[4];
	ssize_t len;

	if (!phy)
		return false;

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(BT_PHY_PORT);
	addr.sin_addr.s_addr = INADDR_BROADCAST;

	memset(&msg, 0, sizeof(msg));
	msg.msg_name = &addr;
	msg.msg_namelen = sizeof(addr);
	msg.msg_iov = iov;
	msg.msg_iovlen = 0;

	memset(&hdr, 0, sizeof(hdr));
	hdr.id = cpu_to_le64(phy->id);
	hdr.flags = cpu_to_le32(0);
	hdr.type = cpu_to_le16(type);
	hdr.len = cpu_to_le16(size1 + size2 + size3);

	iov[msg.msg_iovlen].iov_base = &hdr;
	iov[msg.msg_iovlen].iov_len = sizeof(hdr);
	msg.msg_iovlen++;

	if (data1 && size1 > 0) {
		iov[msg.msg_iovlen].iov_base = (void *) data1;
		iov[msg.msg_iovlen].iov_len = size1;
		msg.msg_iovlen++;
	}

	if (data2 && size2 > 0) {
		iov[msg.msg_iovlen].iov_base = (void *) data2;
		iov[msg.msg_iovlen].iov_len = size2;
		msg.msg_iovlen++;
	}

	if (data3 && size3 > 0) {
		iov[msg.msg_iovlen].iov_base = (void *) data3;
		iov[msg.msg_iovlen].iov_len = size3;
		msg.msg_iovlen++;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(BT_PHY_PORT);
	addr.sin_addr.s_addr = INADDR_BROADCAST;

	len = sendmsg(phy->tx_fd, &msg, MSG_DONTWAIT);
	if (len < 0)
		return false;

	return true;
}

bool bt_phy_register(struct bt_phy *phy, bt_phy_callback_func_t callback,
							void *user_data)
{
	if (!phy)
		return false;

	phy->callback = callback;
	phy->user_data = user_data;

	return true;
}
