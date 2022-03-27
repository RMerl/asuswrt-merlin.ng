/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2011-2012  Intel Corporation
 *  Copyright (C) 2004-2010  Marcel Holtmann <marcel@holtmann.org>
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
#include <alloca.h>
#include <stdlib.h>
#include <stdbool.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include "lib/bluetooth.h"
#include "lib/hci.h"
#include "lib/hci_lib.h"
#include "lib/l2cap.h"

#include "src/shared/mainloop.h"

static bool send_message(const bdaddr_t *src, const bdaddr_t *dst,
							uint16_t psm)
{
	const unsigned char buf[] = { 0x42, 0x23 };
	struct sockaddr_l2 addr;
	ssize_t len;
	int fd;

	fd = socket(PF_BLUETOOTH, SOCK_DGRAM | SOCK_CLOEXEC, BTPROTO_L2CAP);
	if (fd < 0) {
		perror("Failed to create transmitter socket");
		return false;
	}

	memset(&addr, 0, sizeof(addr));
	addr.l2_family = AF_BLUETOOTH;
	bacpy(&addr.l2_bdaddr, src);

	if (bind(fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		perror("Failed to bind transmitter socket");
		close(fd);
		return false;
	}

	memset(&addr, 0, sizeof(addr));
	addr.l2_family = AF_BLUETOOTH;
	bacpy(&addr.l2_bdaddr, dst);
	addr.l2_psm = htobs(psm);

	if (connect(fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		perror("Failed to connect transmitter socket");
		close(fd);
		return false;
	}

	len = send(fd, buf, sizeof(buf), 0);
	if (len < 0) {
		perror("Failed to send message");
		close(fd);
		return false;
	}

	return true;
}

static void receiver_callback(int fd, uint32_t events, void *user_data)
{
	unsigned char buf[512];
	struct sockaddr_l2 addr;
	socklen_t addrlen = sizeof(addr);
	char str[18];
	ssize_t len, i;

	if (events & (EPOLLERR | EPOLLHUP)) {
		close(fd);
		mainloop_remove_fd(fd);
		return;
	}

	len = recvfrom(fd, buf, sizeof(buf), 0,
				(struct sockaddr *) &addr, &addrlen);
	if (len < 0) {
		perror("Failed to receive data");
		return;
	}

	if (addrlen > 0) {
		ba2str(&addr.l2_bdaddr, str);
		printf("RX Address: %s PSM: %d CID: %d\n", str,
				btohs(addr.l2_psm), btohs(addr.l2_cid));
	}

	printf("RX Data:");
	for (i = 0; i < len; i++)
		printf(" 0x%02x", buf[i]);
	printf("\n");
}

static bool create_receiver(const bdaddr_t *bdaddr, uint16_t psm)
{
	struct sockaddr_l2 addr;
	int fd;

	fd = socket(PF_BLUETOOTH, SOCK_DGRAM | SOCK_CLOEXEC, BTPROTO_L2CAP);
	if (fd < 0) {
		perror("Failed to create receiver socket");
		return false;
	}

	memset(&addr, 0, sizeof(addr));
	addr.l2_family = AF_BLUETOOTH;
	bacpy(&addr.l2_bdaddr, bdaddr);
	addr.l2_psm = htobs(psm);

	if (bind(fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		perror("Failed to bind receiver socket");
		close(fd);
		return false;
	}

	mainloop_add_fd(fd, EPOLLIN, receiver_callback, NULL, NULL);

	return true;
}

static bool activate_controller(int fd, struct hci_dev_info *di)
{
	if (!hci_test_bit(HCI_UP, &di->flags)) {
		char addr[18];

		ba2str(&di->bdaddr, addr);
		printf("Activating controller %s\n", addr);

		if (ioctl(fd, HCIDEVUP, di->dev_id) < 0) {
			if (errno != EALREADY) {
				perror("Failed to bring up HCI device");
				return false;
			}
		}
	}
	return true;
}

static bool enable_connections(int fd, struct hci_dev_info *di)
{
	if (!hci_test_bit(HCI_PSCAN, &di->flags)) {
		struct hci_dev_req dr;
		char addr[18];

		ba2str(&di->bdaddr, addr);
		printf("Enabling connections on %s\n", addr);

		dr.dev_id  = di->dev_id;
		dr.dev_opt = SCAN_PAGE;

		if (ioctl(fd, HCISETSCAN, (unsigned long) &dr) < 0) {
			perror("Failed to enable connections");
			return false;
		}
	}
	return true;
}

static bdaddr_t bdaddr_src;
static bdaddr_t bdaddr_dst;

static bool find_controllers(void)
{
	struct hci_dev_list_req *dl;
	struct hci_dev_req *dr;
	bool result;
	int fd, i;

	fd = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI);
	if (fd < 0) {
		perror("Failed to open raw HCI socket");
		return false;
	}

	dl = malloc(HCI_MAX_DEV * sizeof(struct hci_dev_req) + sizeof(uint16_t));
	if (!dl) {
		perror("Failed allocate HCI device request memory");
		close(fd);
		return false;
	}

	dl->dev_num = HCI_MAX_DEV;
	dr = dl->dev_req;

	if (ioctl(fd, HCIGETDEVLIST, (void *) dl) < 0) {
		perror("Failed to get HCI device list");
		result = false;
		goto done;
	}

	result = true;

	for (i = 0; i< dl->dev_num && result; i++) {
		struct hci_dev_info di;

		di.dev_id = (dr + i)->dev_id;

		if (ioctl(fd, HCIGETDEVINFO, (void *) &di) < 0)
			continue;

		if (((di.type & 0x30) >> 4) != HCI_PRIMARY)
			continue;

		if (!bacmp(&bdaddr_src, BDADDR_ANY)) {
			bacpy(&bdaddr_src, &di.bdaddr);
			result = activate_controller(fd, &di);
		} else if (!bacmp(&bdaddr_dst, BDADDR_ANY)) {
			bacpy(&bdaddr_dst, &di.bdaddr);
			result = activate_controller(fd, &di);
			if (result)
				result = enable_connections(fd, &di);
		}
	}

done:
	free(dl);
	close(fd);
	return result;
}

int main(int argc ,char *argv[])
{
	char addr_src[18], addr_dst[18];

	bacpy(&bdaddr_src, BDADDR_ANY);
	bacpy(&bdaddr_dst, BDADDR_ANY);

	if (!find_controllers())
		return EXIT_FAILURE;

	if (!bacmp(&bdaddr_src, BDADDR_ANY) ||
				!bacmp(&bdaddr_dst, BDADDR_ANY)) {
		fprintf(stderr, "Two controllers are required\n");
		return EXIT_FAILURE;
	}

	ba2str(&bdaddr_src, addr_src);
	ba2str(&bdaddr_dst, addr_dst);

	printf("%s -> %s\n", addr_src, addr_dst);

	mainloop_init();

	create_receiver(&bdaddr_dst, 0x0021);
	send_message(&bdaddr_src, &bdaddr_dst, 0x0021);

	return mainloop_run();
}
