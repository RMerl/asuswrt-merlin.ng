/*
 * hid2hci : switch the radio on devices that support
 *           it from HID to HCI and back
 *
 *  Copyright (C) 2003-2010  Marcel Holtmann <marcel@holtmann.org>
 *  Copyright (C) 2008-2009  Mario Limonciello <mario_limonciello@dell.com>
 *  Copyright (C) 2009-2011  Kay Sievers <kay.sievers@vrfy.org>
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
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <getopt.h>
#include <limits.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/hiddev.h>

#include "libudev.h"

#define USB_REQ_SET_CONFIGURATION	0x09

#define USB_TYPE_CLASS			(0x01 << 5)
#define USB_TYPE_VENDOR			(0x02 << 5)

#define USB_RECIP_DEVICE		0x00
#define USB_RECIP_INTERFACE		0x01

#define USB_ENDPOINT_OUT		0x00

struct usbfs_ctrltransfer {
	uint8_t  bmRequestType;
	uint8_t  bRequest;
	uint16_t wValue;
	uint16_t wIndex;
	uint16_t wLength;
	uint32_t timeout;	/* in milliseconds */
	const void *data;	/* pointer to data */
};


#define USBFS_DISCONNECT_IF_DRIVER	0x01
#define USBFS_DISCONNECT_EXCEPT_DRIVER	0x02

struct usbfs_disconnect{
	unsigned int interface;
	unsigned int flags;
	char driver[256];
};

#define USBFS_IOCTL_CONTROL	_IOWR('U', 0, struct usbfs_ctrltransfer)
#define USBFS_IOCTL_DISCONNECT	_IOR('U', 27, struct usbfs_disconnect)

static int control_message(int fd, int requesttype, int request,
					int value, int index,
					const uint8_t *bytes, int size, int timeout)
{
	struct usbfs_ctrltransfer transfer;

	transfer.bmRequestType = requesttype;
	transfer.bRequest = request;
	transfer.wValue = value;
	transfer.wIndex = index;
	transfer.wLength = size,
	transfer.timeout = timeout;
	transfer.data = bytes;

	if (ioctl(fd, USBFS_IOCTL_CONTROL, &transfer) < 0) {
		fprintf(stderr, "Control transfer failed: %s (%d)\n",
						strerror(errno), errno);
		return -1;
	}

	return 0;
}

enum mode {
	HCI = 0,
	HID = 1,
};

static int usb_switch_csr(int fd, enum mode mode)
{
	int err;

	err = control_message(fd, USB_ENDPOINT_OUT | USB_TYPE_VENDOR  |
							USB_RECIP_DEVICE,
						0, mode, 0, NULL, 0, 10000);
	if (err == 0) {
		err = -1;
		errno = EALREADY;
	} else if (errno == ETIMEDOUT)
		err = 0;

	return err;
}

static int usb_switch_csr2(int fd, enum mode mode)
{
	int err = 0;
	struct usbfs_disconnect disconnect;
	const uint8_t report[] = {
		0x01, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	};

	switch (mode) {
	case HCI:
		/* send report as is */
		disconnect.interface = 0;
		disconnect.flags = USBFS_DISCONNECT_EXCEPT_DRIVER;
		strcpy(disconnect.driver, "usbfs");

		if (ioctl(fd, USBFS_IOCTL_DISCONNECT, &disconnect) < 0) {
			fprintf(stderr, "Can't claim interface: %s (%d)\n",
				strerror(errno), errno);
			return -1;
		}

		/* Set_report request with
		 * report id: 0x01, report type: feature (0x03)
		 * on interface 0
		 */
		err = control_message(fd,
				      USB_ENDPOINT_OUT | USB_TYPE_CLASS |
				      USB_RECIP_INTERFACE,
				      USB_REQ_SET_CONFIGURATION,
				      0x01 | (0x03 << 8),
				      0, report, sizeof(report), 5000);
		/* unable to detect whether the previous state
		 * already was HCI (EALREADY)
		 */
		break;
	case HID:
		/* currently unknown how to switch to HID */
		fprintf(stderr,
			"csr2: Switching to hid mode is not implemented\n");
		err = -1;
		break;
	}

	return err;
}

static int hid_logitech_send_report(int fd, const char *buf, size_t size)
{
	struct hiddev_report_info rinfo;
	struct hiddev_usage_ref uref;
	unsigned int i;
	int err;

	for (i = 0; i < size; i++) {
		memset(&uref, 0, sizeof(uref));
		uref.report_type = HID_REPORT_TYPE_OUTPUT;
		uref.report_id   = 0x10;
		uref.field_index = 0;
		uref.usage_index = i;
		uref.usage_code  = 0xff000001;
		uref.value       = buf[i] & 0x000000ff;
		err = ioctl(fd, HIDIOCSUSAGE, &uref);
		if (err < 0)
			return err;
	}

	memset(&rinfo, 0, sizeof(rinfo));
	rinfo.report_type = HID_REPORT_TYPE_OUTPUT;
	rinfo.report_id   = 0x10;
	rinfo.num_fields  = 1;
	err = ioctl(fd, HIDIOCSREPORT, &rinfo);

	return err;
}

static int hid_switch_logitech(const char *filename)
{
	char rep1[] = { 0xff, 0x80, 0x80, 0x01, 0x00, 0x00 };
	char rep2[] = { 0xff, 0x80, 0x00, 0x00, 0x30, 0x00 };
	char rep3[] = { 0xff, 0x81, 0x80, 0x00, 0x00, 0x00 };
	int fd;
	int err = -1;

	fd = open(filename, O_RDWR);
	if (fd < 0)
		return err;

	err = ioctl(fd, HIDIOCINITREPORT, 0);
	if (err < 0)
		goto out;

	err = hid_logitech_send_report(fd, rep1, sizeof(rep1));
	if (err < 0)
		goto out;

	err = hid_logitech_send_report(fd, rep2, sizeof(rep2));
	if (err < 0)
		goto out;

	err = hid_logitech_send_report(fd, rep3, sizeof(rep3));
out:
	close(fd);
	return err;
}

static int usb_switch_dell(int fd, enum mode mode)
{
	uint8_t report[] = { 0x7f, 0x00, 0x00, 0x00 };
	struct usbfs_disconnect disconnect;
	int err;

	switch (mode) {
	case HCI:
		report[1] = 0x13;
		break;
	case HID:
		report[1] = 0x14;
		break;
	}

	disconnect.interface = 0;
	disconnect.flags = USBFS_DISCONNECT_EXCEPT_DRIVER;
	strcpy(disconnect.driver, "usbfs");

	if (ioctl(fd, USBFS_IOCTL_DISCONNECT, &disconnect) < 0) {
		fprintf(stderr, "Can't claim interface: %s (%d)\n",
						strerror(errno), errno);
		return -1;
	}

	err = control_message(fd, USB_ENDPOINT_OUT | USB_TYPE_CLASS |
							USB_RECIP_INTERFACE,
				USB_REQ_SET_CONFIGURATION,
				0x7f | (0x03 << 8), 0,
				report, sizeof(report), 5000);
	if (err == 0) {
		err = -1;
		errno = EALREADY;
	} else {
		if (errno == ETIMEDOUT)
			err = 0;
	}

	return err;
}

static int find_device(struct udev_device *udev_dev)
{
	char path[PATH_MAX];
	const char *busnum_str, *devnum_str;
	int busnum, devnum;
	int fd;

	busnum_str = udev_device_get_sysattr_value(udev_dev, "busnum");
	if (!busnum_str)
		return -1;
	busnum = strtol(busnum_str, NULL, 10);

	devnum_str = udev_device_get_sysattr_value(udev_dev, "devnum");
	if (!devnum_str)
		return -1;
	devnum = strtol(devnum_str, NULL, 10);

	snprintf(path, sizeof(path), "/dev/bus/usb/%03d/%03d", busnum, devnum);

	fd = open(path, O_RDWR, O_CLOEXEC);
	if (fd < 0) {
		fprintf(stderr, "Can't open device: %s (%d)\n",
						strerror(errno), errno);
		return -1;
	}

	return fd;
}

static void usage(const char *error)
{
	if (error)
		fprintf(stderr,"\n%s\n", error);
	else
		printf("hid2hci - Bluetooth HID to HCI mode switching utility\n\n");

	printf("Usage: hid2hci [options]\n"
		"  --mode=       mode to switch to [hid|hci] (default hci)\n"
		"  --devpath=    sys device path\n"
		"  --method=     method to use to switch [csr|csr2|logitech-hid|dell]\n"
		"  --help\n\n");
}

int main(int argc, char *argv[])
{
	static const struct option options[] = {
		{ "help", no_argument, NULL, 'h' },
		{ "mode", required_argument, NULL, 'm' },
		{ "devpath", required_argument, NULL, 'p' },
		{ "method", required_argument, NULL, 'M' },
		{ }
	};
	enum method {
		METHOD_UNDEF,
		METHOD_CSR,
		METHOD_LOGITECH_HID,
		METHOD_DELL,
	} method = METHOD_UNDEF;
	struct udev *udev;
	struct udev_device *udev_dev = NULL;
	char syspath[PATH_MAX];
	int (*usb_switch)(int fd, enum mode mode) = NULL;
	enum mode mode = HCI;
	const char *devpath = NULL;
	int err = -1;
	int rc = 1;

	for (;;) {
		int option;

		option = getopt_long(argc, argv, "m:p:M:h", options, NULL);
		if (option == -1)
			break;

		switch (option) {
		case 'm':
			if (!strcmp(optarg, "hid")) {
				mode = HID;
			} else if (!strcmp(optarg, "hci")) {
				mode = HCI;
			} else {
				usage("error: undefined radio mode\n");
				exit(1);
			}
			break;
		case 'p':
			devpath = optarg;
			break;
		case 'M':
			if (!strcmp(optarg, "csr")) {
				method = METHOD_CSR;
				usb_switch = usb_switch_csr;
			} else if (!strcmp(optarg, "csr2")) {
				method = METHOD_CSR;
				usb_switch = usb_switch_csr2;
			} else if (!strcmp(optarg, "logitech-hid")) {
				method = METHOD_LOGITECH_HID;
			} else if (!strcmp(optarg, "dell")) {
				method = METHOD_DELL;
				usb_switch = usb_switch_dell;
			} else {
				usage("error: undefined switching method\n");
				exit(1);
			}
			break;
		case 'h':
			usage(NULL);
		}
	}

	if (!devpath || method == METHOD_UNDEF) {
		usage("error: --devpath= and --method= must be defined\n");
		exit(1);
	}

	udev = udev_new();
	if (udev == NULL)
		goto exit;

	snprintf(syspath, sizeof(syspath), "/sys/%s", devpath);
	udev_dev = udev_device_new_from_syspath(udev, syspath);
	if (udev_dev == NULL) {
		fprintf(stderr, "error: could not find '%s'\n", devpath);
		goto exit;
	}

	switch (method) {
	case METHOD_CSR:
	case METHOD_DELL: {
		struct udev_device *dev;
		int handle;
		const char *type;

		/* get the parent usb_device if needed */
		dev = udev_dev;
		type = udev_device_get_devtype(dev);
		if (type == NULL || strcmp(type, "usb_device") != 0) {
			dev = udev_device_get_parent_with_subsystem_devtype(dev,
							"usb", "usb_device");
			if (dev == NULL) {
				fprintf(stderr, "error: could not find usb_device for '%s'\n", devpath);
				goto exit;
			}
		}

		handle = find_device(dev);
		if (handle < 0) {
			fprintf(stderr, "error: unable to handle '%s'\n",
				udev_device_get_syspath(dev));
			goto exit;
		}
		err = usb_switch(handle, mode);
		close(handle);
		break;
	}
	case METHOD_LOGITECH_HID: {
		const char *device;

		device = udev_device_get_devnode(udev_dev);
		if (device == NULL) {
			fprintf(stderr, "error: could not find hiddev device node\n");
			goto exit;
		}
		err = hid_switch_logitech(device);
		break;
	}
	case METHOD_UNDEF:
	default:
		break;
	}

	if (err < 0)
		fprintf(stderr, "error: switching device '%s' failed.\n",
			udev_device_get_syspath(udev_dev));
exit:
	udev_device_unref(udev_dev);
	udev_unref(udev);
	return rc;
}
