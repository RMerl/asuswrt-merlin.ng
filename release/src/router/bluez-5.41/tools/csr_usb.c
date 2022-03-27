/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
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
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <limits.h>
#include <sys/ioctl.h>

#include "csr.h"

#define USB_TYPE_CLASS			(0x01 << 5)

#define USB_RECIP_DEVICE		0x00

#define USB_ENDPOINT_IN			0x80
#define USB_ENDPOINT_OUT		0x00

struct usbfs_ctrltransfer {
	uint8_t  bmRequestType;
	uint8_t  bRequest;
	uint16_t wValue;
	uint16_t wIndex;
	uint16_t wLength;
	uint32_t timeout;	/* in milliseconds */
	void *data;		/* pointer to data */
};

struct usbfs_bulktransfer {
	unsigned int ep;
	unsigned int len;
	unsigned int timeout;   /* in milliseconds */
	void *data;		/* pointer to data */
};

#define USBFS_IOCTL_CONTROL	_IOWR('U', 0, struct usbfs_ctrltransfer)
#define USBFS_IOCTL_BULK	_IOWR('U', 2, struct usbfs_bulktransfer)
#define USBFS_IOCTL_CLAIMINTF	_IOR('U', 15, unsigned int)
#define USBFS_IOCTL_RELEASEINTF	_IOR('U', 16, unsigned int)

static int read_value(const char *name, const char *attr, const char *format)
{
	char path[PATH_MAX];
	FILE *file;
	int n, value;

	snprintf(path, sizeof(path), "/sys/bus/usb/devices/%s/%s", name, attr);

	file = fopen(path, "r");
	if (!file)
		return -1;

	n = fscanf(file, format, &value);
	if (n != 1) {
		fclose(file);
		return -1;
	}

	fclose(file);
	return value;
}

static char *check_device(const char *name)
{
	char path[PATH_MAX];
	int busnum, devnum, vendor, product;

	busnum = read_value(name, "busnum", "%d");
	if (busnum < 0)
		return NULL;

	devnum = read_value(name, "devnum", "%d");
	if (devnum < 0)
		return NULL;

	snprintf(path, sizeof(path), "/dev/bus/usb/%03u/%03u", busnum, devnum);

	vendor = read_value(name, "idVendor", "%04x");
	if (vendor < 0)
		return NULL;

	product = read_value(name, "idProduct", "%04x");
	if (product < 0)
		return NULL;

	if (vendor != 0x0a12 || product != 0x0001)
		return NULL;

	return strdup(path);
}

static char *find_device(void)
{
	char *path = NULL;
	DIR *dir;

	dir = opendir("/sys/bus/usb/devices");
	if (!dir)
		return NULL;

	while (1) {
		struct dirent *d;

		d = readdir(dir);
		if (!d)
			break;

		if ((!isdigit(d->d_name[0]) && strncmp(d->d_name, "usb", 3))
						|| strchr(d->d_name, ':'))
			continue;

		path = check_device(d->d_name);
		if (path)
			break;
	}

	closedir(dir);

	return path;
}

static uint16_t seqnum = 0x0000;
static int handle = -1;

int csr_open_usb(char *device)
{
	int interface = 0;
	char *path;

	path = find_device();
	if (!path) {
		fprintf(stderr, "Device not available\n");
		return -1;
	}

	handle = open(path, O_RDWR, O_CLOEXEC | O_NONBLOCK);

	free(path);

	if (handle < 0) {
		fprintf(stderr, "Can't open device: %s (%d)\n",
						strerror(errno), errno);
		return -1;
	}

	if (ioctl(handle, USBFS_IOCTL_CLAIMINTF, &interface) < 0) {
		fprintf(stderr, "Can't claim interface: %s (%d)\n",
						strerror(errno), errno);
		close(handle);
		handle = -1;
		return -1;
	}

	return 0;
}

static int control_write(int fd, void *data, unsigned short size)
{
	struct usbfs_ctrltransfer transfer;

	transfer.bmRequestType = USB_TYPE_CLASS | USB_ENDPOINT_OUT |
							USB_RECIP_DEVICE;
	transfer.bRequest = 0;
	transfer.wValue = 0;
	transfer.wIndex = 0;
	transfer.wLength = size,
	transfer.timeout = 2000;
	transfer.data = data;

	if (ioctl(fd, USBFS_IOCTL_CONTROL, &transfer) < 0) {
		fprintf(stderr, "Control transfer failed: %s (%d)\n",
						strerror(errno), errno);
		return -1;
	}

	return 0;
}

static int interrupt_read(int fd, unsigned char endpoint,
					void *data, unsigned short size)
{
	struct usbfs_bulktransfer transfer;

	transfer.ep = endpoint;
	transfer.len = size,
	transfer.timeout = 20;
	transfer.data = data;

	return ioctl(fd, USBFS_IOCTL_BULK, &transfer);
}

static int do_command(uint16_t command, uint16_t seqnum, uint16_t varid,
					uint8_t *value, uint16_t length)
{
	unsigned char cp[254], rp[254];
	uint8_t cmd[10];
	uint16_t size;
	int len, offset = 0;

	size = (length < 8) ? 9 : ((length + 1) / 2) + 5;

	cmd[0] = command & 0xff;
	cmd[1] = command >> 8;
	cmd[2] = size & 0xff;
	cmd[3] = size >> 8;
	cmd[4] = seqnum & 0xff;
	cmd[5] = seqnum >> 8;
	cmd[6] = varid & 0xff;
	cmd[7] = varid >> 8;
	cmd[8] = 0x00;
	cmd[9] = 0x00;

	memset(cp, 0, sizeof(cp));
	cp[0] = 0x00;
	cp[1] = 0xfc;
	cp[2] = (size * 2) + 1;
	cp[3] = 0xc2;
	memcpy(cp + 4, cmd, sizeof(cmd));
	memcpy(cp + 14, value, length);

	interrupt_read(handle, USB_ENDPOINT_IN | 0x01, rp, sizeof(rp));

	control_write(handle, cp, (size * 2) + 4);

	switch (varid) {
	case CSR_VARID_COLD_RESET:
	case CSR_VARID_WARM_RESET:
	case CSR_VARID_COLD_HALT:
	case CSR_VARID_WARM_HALT:
		return 0;
	}

	do {
		len = interrupt_read(handle, USB_ENDPOINT_IN | 0x01,
					rp + offset, sizeof(rp) - offset);
		if (len < 0)
			break;
		offset += len;
	} while (len > 0);

	if (rp[0] != 0xff || rp[2] != 0xc2) {
		errno = EIO;
		return -1;
	}

	if ((rp[11] + (rp[12] << 8)) != 0) {
		errno = ENXIO;
		return -1;
	}

	memcpy(value, rp + 13, length);

	return 0;
}

int csr_read_usb(uint16_t varid, uint8_t *value, uint16_t length)
{
	return do_command(0x0000, seqnum++, varid, value, length);
}

int csr_write_usb(uint16_t varid, uint8_t *value, uint16_t length)
{
	return do_command(0x0002, seqnum++, varid, value, length);
}

void csr_close_usb(void)
{
	int interface = 0;

	ioctl(handle, USBFS_IOCTL_RELEASEINTF, &interface);

	close(handle);
	handle = -1;
}
