/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2015  Intel Corporation. All rights reserved.
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
#include <sys/ioctl.h>

#include "lib/bluetooth.h"
#include "lib/hci.h"
#include "lib/hci_lib.h"
#include "tools/hciattach.h"
#include "peripheral/attach.h"

static const char *serial_dev = "/dev/ttyS1";
static int serial_fd = -1;

static int open_serial(const char *path)
{
	struct termios ti;
	int fd, saved_ldisc, ldisc = N_HCI;

	fd = open(path, O_RDWR | O_NOCTTY);
	if (fd < 0) {
		perror("Failed to open serial port");
		return -1;
	}

	if (tcflush(fd, TCIOFLUSH) < 0) {
		perror("Failed to flush serial port");
		close(fd);
		return -1;
	}

	if (ioctl(fd, TIOCGETD, &saved_ldisc) < 0) {
		perror("Failed get serial line discipline");
		close(fd);
		return -1;
	}

	/* Switch TTY to raw mode */
	memset(&ti, 0, sizeof(ti));
	cfmakeraw(&ti);

	ti.c_cflag |= (B115200 | CLOCAL | CREAD);

	/* Set flow control */
	ti.c_cflag |= CRTSCTS;

	if (tcsetattr(fd, TCSANOW, &ti) < 0) {
		perror("Failed to set serial port settings");
		close(fd);
		return -1;
	}

	if (ioctl(fd, TIOCSETD, &ldisc) < 0) {
		perror("Failed set serial line discipline");
		close(fd);
		return -1;
	}

	printf("Switched line discipline from %d to %d\n", saved_ldisc, ldisc);

	return fd;
}

static int attach_proto(const char *path, unsigned int proto,
						unsigned int flags)
{
	int fd, dev_id;

	fd = open_serial(path);
	if (fd < 0)
		return -1;

	if (ioctl(fd, HCIUARTSETFLAGS, flags) < 0) {
		perror("Failed to set flags");
		close(fd);
		return -1;
	}

	if (ioctl(fd, HCIUARTSETPROTO, proto) < 0) {
		perror("Failed to set protocol");
		close(fd);
		return -1;
	}

	dev_id = ioctl(fd, HCIUARTGETDEVICE);
	if (dev_id < 0) {
		perror("Failed to get device id");
		close(fd);
		return -1;
	}

	printf("Device index %d attached\n", dev_id);

	return fd;
}

void attach_start(void)
{
	unsigned long flags;

	if (serial_fd >= 0)
		return;

	printf("Attaching BR/EDR controller to %s\n", serial_dev);

	flags = (1 << HCI_UART_RESET_ON_INIT);

	serial_fd = attach_proto(serial_dev, HCI_UART_H4, flags);
}

void attach_stop(void)
{
	if (serial_fd < 0)
		return;

	close(serial_fd);
	serial_fd = -1;
}
