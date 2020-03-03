/*
 * Copyright (C) 2003-2008 Takahiro Hirofuchi
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
 * USA.
 */

#include <linux/kthread.h>
#include <linux/file.h>
#include <linux/net.h>

#include "usbip_common.h"
#include "vhci.h"

/* TODO: refine locking ?*/

/* Sysfs entry to show port status */
static ssize_t status_show(struct device *dev, struct device_attribute *attr,
			   char *out)
{
	char *s = out;
	int i = 0;

	BUG_ON(!the_controller || !out);

	spin_lock(&the_controller->lock);

	/*
	 * output example:
	 * port sta spd dev      sockfd local_busid
	 * 0000 004 000 00000000 000003 1-2.3
	 * 0001 004 000 00000000 000004 2-3.4
	 *
	 * Output includes socket fd instead of socket pointer address to
	 * avoid leaking kernel memory address in:
	 *	/sys/devices/platform/vhci_hcd.0/status and in debug output.
	 * The socket pointer address is not used at the moment and it was
	 * made visible as a convenient way to find IP address from socket
	 * pointer address by looking up /proc/net/{tcp,tcp6}. As this opens
	 * a security hole, the change is made to use sockfd instead.
	 */
	out += sprintf(out,
		       "prt sta spd bus dev sockfd local_busid\n");

	for (i = 0; i < VHCI_NPORTS; i++) {
		struct vhci_device *vdev = port_to_vdev(i);

		spin_lock(&vdev->ud.lock);
		out += sprintf(out, "%03u %03u ", i, vdev->ud.status);

		if (vdev->ud.status == VDEV_ST_USED) {
			out += sprintf(out, "%03u %08x ",
				       vdev->speed, vdev->devid);
			out += sprintf(out, "%16p ", vdev->ud.tcp_socket);
			out += sprintf(out, "%06u", vdev->ud.sockfd);
			out += sprintf(out, "%s", dev_name(&vdev->udev->dev));

		} else
			out += sprintf(out, "000 000 000 000000 0-0");

		out += sprintf(out, "\n");
		spin_unlock(&vdev->ud.lock);
	}

	spin_unlock(&the_controller->lock);

	return out - s;
}
static DEVICE_ATTR_RO(status);

/* Sysfs entry to shutdown a virtual connection */
static int vhci_port_disconnect(__u32 rhport)
{
	struct vhci_device *vdev;

	usbip_dbg_vhci_sysfs("enter\n");

	/* lock */
	spin_lock(&the_controller->lock);

	vdev = port_to_vdev(rhport);

	spin_lock(&vdev->ud.lock);
	if (vdev->ud.status == VDEV_ST_NULL) {
		pr_err("not connected %d\n", vdev->ud.status);

		/* unlock */
		spin_unlock(&vdev->ud.lock);
		spin_unlock(&the_controller->lock);

		return -EINVAL;
	}

	/* unlock */
	spin_unlock(&vdev->ud.lock);
	spin_unlock(&the_controller->lock);

	usbip_event_add(&vdev->ud, VDEV_EVENT_DOWN);

	return 0;
}

static ssize_t store_detach(struct device *dev, struct device_attribute *attr,
			    const char *buf, size_t count)
{
	int err;
	__u32 rhport = 0;

	if (sscanf(buf, "%u", &rhport) != 1)
		return -EINVAL;

	/* check rhport */
	if (rhport >= VHCI_NPORTS) {
		dev_err(dev, "invalid port %u\n", rhport);
		return -EINVAL;
	}

	err = vhci_port_disconnect(rhport);
	if (err < 0)
		return -EINVAL;

	usbip_dbg_vhci_sysfs("Leave\n");

	return count;
}
static DEVICE_ATTR(detach, S_IWUSR, NULL, store_detach);

/* Sysfs entry to establish a virtual connection */
static int valid_args(__u32 rhport, enum usb_device_speed speed)
{
	/* check rhport */
	if (rhport >= VHCI_NPORTS) {
		pr_err("port %u\n", rhport);
		return -EINVAL;
	}

	/* check speed */
	switch (speed) {
	case USB_SPEED_LOW:
	case USB_SPEED_FULL:
	case USB_SPEED_HIGH:
	case USB_SPEED_WIRELESS:
		break;
	default:
		pr_err("Failed attach request for unsupported USB speed: %s\n",
			usb_speed_string(speed));
		return -EINVAL;
	}

	return 0;
}

/*
 * To start a new USB/IP attachment, a userland program needs to setup a TCP
 * connection and then write its socket descriptor with remote device
 * information into this sysfs file.
 *
 * A remote device is virtually attached to the root-hub port of @rhport with
 * @speed. @devid is embedded into a request to specify the remote device in a
 * server host.
 *
 * write() returns 0 on success, else negative errno.
 */
static ssize_t store_attach(struct device *dev, struct device_attribute *attr,
			    const char *buf, size_t count)
{
	struct vhci_device *vdev;
	struct socket *socket;
	int sockfd = 0;
	__u32 rhport = 0, devid = 0, speed = 0;
	int err;

	/*
	 * @rhport: port number of vhci_hcd
	 * @sockfd: socket descriptor of an established TCP connection
	 * @devid: unique device identifier in a remote host
	 * @speed: usb device speed in a remote host
	 */
	if (sscanf(buf, "%u %u %u %u", &rhport, &sockfd, &devid, &speed) != 4)
		return -EINVAL;

	usbip_dbg_vhci_sysfs("rhport(%u) sockfd(%u) devid(%u) speed(%u)\n",
			     rhport, sockfd, devid, speed);

	/* check received parameters */
	if (valid_args(rhport, speed) < 0)
		return -EINVAL;

	/* Extract socket from fd. */
	socket = sockfd_lookup(sockfd, &err);
	if (!socket)
		return -EINVAL;

	/* now need lock until setting vdev status as used */

	/* begin a lock */
	spin_lock(&the_controller->lock);
	vdev = port_to_vdev(rhport);
	spin_lock(&vdev->ud.lock);

	if (vdev->ud.status != VDEV_ST_NULL) {
		/* end of the lock */
		spin_unlock(&vdev->ud.lock);
		spin_unlock(&the_controller->lock);

		sockfd_put(socket);

		dev_err(dev, "port %d already used\n", rhport);
		return -EINVAL;
	}

	dev_info(dev,
		 "rhport(%u) sockfd(%d) devid(%u) speed(%u) speed_str(%s)\n",
		 rhport, sockfd, devid, speed, usb_speed_string(speed));

	vdev->devid         = devid;
	vdev->speed         = speed;
	vdev->ud.sockfd     = sockfd;
	vdev->ud.tcp_socket = socket;
	vdev->ud.status     = VDEV_ST_NOTASSIGNED;

	spin_unlock(&vdev->ud.lock);
	spin_unlock(&the_controller->lock);
	/* end the lock */

	vdev->ud.tcp_rx = kthread_get_run(vhci_rx_loop, &vdev->ud, "vhci_rx");
	vdev->ud.tcp_tx = kthread_get_run(vhci_tx_loop, &vdev->ud, "vhci_tx");

	rh_port_connect(rhport, speed);

	return count;
}
static DEVICE_ATTR(attach, S_IWUSR, NULL, store_attach);

static struct attribute *dev_attrs[] = {
	&dev_attr_status.attr,
	&dev_attr_detach.attr,
	&dev_attr_attach.attr,
	&dev_attr_usbip_debug.attr,
	NULL,
};

const struct attribute_group dev_attr_group = {
	.attrs = dev_attrs,
};
