// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2009  Bastien Nocera <hadess@hadess.net>
 *  Copyright (C) 2011  Antonio Ospite <ospite@studenti.unina.it>
 *  Copyright (C) 2013  Szymon Janc <szymon.janc@gmail.com>
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define _GNU_SOURCE
#include <stddef.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <linux/hidraw.h>
#include <linux/input.h>
#include <glib.h>
#include <libudev.h>

#include "lib/bluetooth.h"
#include "lib/sdp.h"
#include "lib/uuid.h"

#include "src/adapter.h"
#include "src/device.h"
#include "src/agent.h"
#include "src/plugin.h"
#include "src/log.h"
#include "src/shared/util.h"
#include "profiles/input/sixaxis.h"

struct authentication_closure {
	guint auth_id;
	char *sysfs_path;
	struct btd_adapter *adapter;
	struct btd_device *device;
	int fd;
	bdaddr_t bdaddr; /* device bdaddr */
	CablePairingType type;
};

struct authentication_destroy_closure {
	struct authentication_closure *closure;
	bool remove_device;
};

static struct udev *ctx = NULL;
static struct udev_monitor *monitor = NULL;
static guint watch_id = 0;
/* key = sysfs_path (const str), value = auth_closure */
static GHashTable *pending_auths = NULL;

#define SIXAXIS_HID_SDP_RECORD "3601920900000A000100000900013503191124090004"\
	"350D35061901000900113503190011090006350909656E09006A090100090009350"\
	"8350619112409010009000D350F350D350619010009001335031900110901002513"\
	"576972656C65737320436F6E74726F6C6C65720901012513576972656C657373204"\
	"36F6E74726F6C6C6572090102251B536F6E7920436F6D707574657220456E746572"\
	"7461696E6D656E74090200090100090201090100090202080009020308210902042"\
	"8010902052801090206359A35980822259405010904A101A1028501750895011500"\
	"26FF00810375019513150025013500450105091901291381027501950D0600FF810"\
	"3150026FF0005010901A10075089504350046FF0009300931093209358102C00501"\
	"75089527090181027508953009019102750895300901B102C0A1028502750895300"\
	"901B102C0A10285EE750895300901B102C0A10285EF750895300901B102C0C00902"\
	"07350835060904090901000902082800090209280109020A280109020B090100090"\
	"20C093E8009020D280009020E2800"

/* Make sure to unset auth_id if already handled */
static void auth_closure_destroy(struct authentication_closure *closure,
				bool remove_device)
{
	if (closure->auth_id)
		btd_cancel_authorization(closure->auth_id);

	if (remove_device)
		btd_adapter_remove_device(closure->adapter, closure->device);
	close(closure->fd);
	g_free(closure->sysfs_path);
	g_free(closure);
}

static int sixaxis_get_device_bdaddr(int fd, bdaddr_t *bdaddr)
{
	uint8_t buf[18];
	int ret;

	memset(buf, 0, sizeof(buf));

	buf[0] = 0xf2;

	ret = ioctl(fd, HIDIOCGFEATURE(sizeof(buf)), buf);
	if (ret < 0) {
		error("sixaxis: failed to read device address (%s)",
							strerror(errno));
		return ret;
	}

	baswap(bdaddr, (bdaddr_t *) (buf + 4));

	return 0;
}

static int ds4_get_device_bdaddr(int fd, bdaddr_t *bdaddr)
{
	uint8_t buf[7];
	int ret;

	memset(buf, 0, sizeof(buf));

	buf[0] = 0x81;

	ret = ioctl(fd, HIDIOCGFEATURE(sizeof(buf)), buf);
	if (ret < 0) {
		error("sixaxis: failed to read DS4 device address (%s)",
		      strerror(errno));
		return ret;
	}

	/* address is little-endian on DS4 */
	bacpy(bdaddr, (bdaddr_t*) (buf + 1));

	return 0;
}

static int get_device_bdaddr(int fd, bdaddr_t *bdaddr, CablePairingType type)
{
	if (type == CABLE_PAIRING_SIXAXIS)
		return sixaxis_get_device_bdaddr(fd, bdaddr);
	else if (type == CABLE_PAIRING_DS4)
		return ds4_get_device_bdaddr(fd, bdaddr);
	return -1;
}

static int sixaxis_get_master_bdaddr(int fd, bdaddr_t *bdaddr)
{
	uint8_t buf[8];
	int ret;

	memset(buf, 0, sizeof(buf));

	buf[0] = 0xf5;

	ret = ioctl(fd, HIDIOCGFEATURE(sizeof(buf)), buf);
	if (ret < 0) {
		error("sixaxis: failed to read master address (%s)",
							strerror(errno));
		return ret;
	}

	baswap(bdaddr, (bdaddr_t *) (buf + 2));

	return 0;
}

static int ds4_get_master_bdaddr(int fd, bdaddr_t *bdaddr)
{
	uint8_t buf[16];
	int ret;

	memset(buf, 0, sizeof(buf));

	buf[0] = 0x12;

	ret = ioctl(fd, HIDIOCGFEATURE(sizeof(buf)), buf);
	if (ret < 0) {
		error("sixaxis: failed to read DS4 master address (%s)",
		      strerror(errno));
		return ret;
	}

	/* address is little-endian on DS4 */
	bacpy(bdaddr, (bdaddr_t*) (buf + 10));

	return 0;
}

static int get_master_bdaddr(int fd, bdaddr_t *bdaddr, CablePairingType type)
{
	if (type == CABLE_PAIRING_SIXAXIS)
		return sixaxis_get_master_bdaddr(fd, bdaddr);
	else if (type == CABLE_PAIRING_DS4)
		return ds4_get_master_bdaddr(fd, bdaddr);
	return -1;
}

static int sixaxis_set_master_bdaddr(int fd, const bdaddr_t *bdaddr)
{
	uint8_t buf[8];
	int ret;

	buf[0] = 0xf5;
	buf[1] = 0x01;

	baswap((bdaddr_t *) (buf + 2), bdaddr);

	ret = ioctl(fd, HIDIOCSFEATURE(sizeof(buf)), buf);
	if (ret < 0)
		error("sixaxis: failed to write master address (%s)",
							strerror(errno));

	return ret;
}

static int ds4_set_master_bdaddr(int fd, const bdaddr_t *bdaddr)
{
	uint8_t buf[23];
	int ret;

	buf[0] = 0x13;
	bacpy((bdaddr_t*) (buf + 1), bdaddr);
	/* TODO: we could put the key here but
	   there is no way to force a re-loading
	   of link keys to the kernel from here. */
	memset(buf + 7, 0, 16);

	ret = ioctl(fd, HIDIOCSFEATURE(sizeof(buf)), buf);
	if (ret < 0)
		error("sixaxis: failed to write DS4 master address (%s)",
		      strerror(errno));

	return ret;
}

static int set_master_bdaddr(int fd, const bdaddr_t *bdaddr,
					CablePairingType type)
{
	if (type == CABLE_PAIRING_SIXAXIS)
		return sixaxis_set_master_bdaddr(fd, bdaddr);
	else if (type == CABLE_PAIRING_DS4)
		return ds4_set_master_bdaddr(fd, bdaddr);
	return -1;
}

static bool is_auth_pending(struct authentication_closure *closure)
{
	GHashTableIter iter;
	gpointer value;

	g_hash_table_iter_init(&iter, pending_auths);
	while (g_hash_table_iter_next(&iter, NULL, &value)) {
		struct authentication_closure *c = value;
		if (c == closure)
			return true;
	}
	return false;
}

static gboolean auth_closure_destroy_idle(gpointer user_data)
{
	struct authentication_destroy_closure *destroy = user_data;

	auth_closure_destroy(destroy->closure, destroy->remove_device);
	g_free(destroy);

	return false;
}

static void agent_auth_cb(DBusError *derr, void *user_data)
{
	struct authentication_closure *closure = user_data;
	struct authentication_destroy_closure *destroy;
	char master_addr[18], adapter_addr[18], device_addr[18];
	bdaddr_t master_bdaddr;
	const bdaddr_t *adapter_bdaddr;
	bool remove_device = true;

	if (!is_auth_pending(closure))
		return;

	/* Don't try to remove this auth, we're handling it already */
	closure->auth_id = 0;

	if (derr != NULL) {
		DBG("Agent replied negatively, removing temporary device");
		goto out;
	}

	if (get_master_bdaddr(closure->fd, &master_bdaddr, closure->type) < 0)
		goto out;

	adapter_bdaddr = btd_adapter_get_address(closure->adapter);
	if (bacmp(adapter_bdaddr, &master_bdaddr)) {
		if (set_master_bdaddr(closure->fd, adapter_bdaddr,
							closure->type) < 0)
			goto out;
	}

	remove_device = false;
	btd_device_set_trusted(closure->device, true);
	btd_device_set_temporary(closure->device, false);

	if (closure->type == CABLE_PAIRING_SIXAXIS)
		btd_device_set_record(closure->device, HID_UUID,
						 SIXAXIS_HID_SDP_RECORD);

	ba2str(&closure->bdaddr, device_addr);
	ba2str(&master_bdaddr, master_addr);
	ba2str(adapter_bdaddr, adapter_addr);
	DBG("remote %s old_master %s new_master %s",
				device_addr, master_addr, adapter_addr);

out:
	g_hash_table_steal(pending_auths, closure->sysfs_path);

	/* btd_adapter_remove_device() cannot be called in this
	 * callback or it would lead to a double-free in while
	 * trying to cancel the authentication that's being processed,
	 * so clean up in an idle */
	destroy = g_new0(struct authentication_destroy_closure, 1);
	destroy->closure = closure;
	destroy->remove_device = remove_device;
	g_idle_add(auth_closure_destroy_idle, destroy);
}

static bool setup_device(int fd, const char *sysfs_path,
			const struct cable_pairing *cp,
			struct btd_adapter *adapter)
{
	bdaddr_t device_bdaddr;
	const bdaddr_t *adapter_bdaddr;
	struct btd_device *device;
	struct authentication_closure *closure;

	if (get_device_bdaddr(fd, &device_bdaddr, cp->type) < 0)
		return false;

	/* This can happen if controller was plugged while already setup and
	 * connected eg. to charge up battery. */
	device = btd_adapter_find_device(adapter, &device_bdaddr,
							BDADDR_BREDR);
	if (device != NULL &&
		btd_device_is_connected(device) &&
		g_slist_find_custom(btd_device_get_uuids(device), HID_UUID,
						(GCompareFunc)strcasecmp)) {
		char device_addr[18];
		ba2str(&device_bdaddr, device_addr);
		DBG("device %s already known, skipping", device_addr);
		return false;
	}

	device = btd_adapter_get_device(adapter, &device_bdaddr, BDADDR_BREDR);

	info("sixaxis: setting up new device");

	btd_device_device_set_name(device, cp->name);
	btd_device_set_pnpid(device, cp->source, cp->vid, cp->pid, cp->version);
	btd_device_set_trusted(device, false);
	btd_device_set_temporary(device, true);

	closure = g_new0(struct authentication_closure, 1);
	if (!closure) {
		btd_adapter_remove_device(adapter, device);
		return false;
	}
	closure->adapter = adapter;
	closure->device = device;
	closure->sysfs_path = g_strdup(sysfs_path);
	closure->fd = fd;
	bacpy(&closure->bdaddr, &device_bdaddr);
	closure->type = cp->type;
	adapter_bdaddr = btd_adapter_get_address(adapter);
	closure->auth_id = btd_request_authorization_cable_configured(
					adapter_bdaddr, &device_bdaddr,
					HID_UUID, agent_auth_cb, closure);

	if (closure->auth_id == 0) {
		error("sixaxis: could not request cable authorization");
		auth_closure_destroy(closure, true);
		return false;
	}

	g_hash_table_insert(pending_auths, closure->sysfs_path, closure);

	return true;
}

static const struct cable_pairing *
get_pairing_type_for_device(struct udev_device *udevice, uint16_t *bus,
						char **sysfs_path)
{
	struct udev_device *hid_parent;
	const char *hid_name;
	const char *hid_id;
	const struct cable_pairing *cp;
	uint16_t vid, pid;

	hid_parent = udev_device_get_parent_with_subsystem_devtype(udevice,
								"hid", NULL);
	if (!hid_parent)
		return NULL;

	hid_id = udev_device_get_property_value(hid_parent, "HID_ID");

	if (!hid_id || sscanf(hid_id, "%hx:%hx:%hx", bus, &vid, &pid) != 3)
		return NULL;

	hid_name = udev_device_get_property_value(hid_parent, "HID_NAME");

	cp = get_pairing(vid, pid, hid_name);
	*sysfs_path = g_strdup(udev_device_get_syspath(udevice));

	return cp;
}

static void device_added(struct udev_device *udevice)
{
	struct btd_adapter *adapter;
	uint16_t bus;
	char *sysfs_path = NULL;
	const struct cable_pairing *cp;
	int fd;

	adapter = btd_adapter_get_default();
	if (!adapter)
		return;

	cp = get_pairing_type_for_device(udevice, &bus, &sysfs_path);
	if (!cp || (cp->type != CABLE_PAIRING_SIXAXIS &&
				cp->type != CABLE_PAIRING_DS4))
		return;
	if (bus != BUS_USB)
		return;

	info("sixaxis: compatible device connected: %s (%04X:%04X %s)",
				cp->name, cp->vid, cp->pid, sysfs_path);

	fd = open(udev_device_get_devnode(udevice), O_RDWR);
	if (fd < 0) {
		g_free(sysfs_path);
		return;
	}

	/* Only close the fd if an authentication is not pending */
	if (!setup_device(fd, sysfs_path, cp, adapter))
		close(fd);

	g_free(sysfs_path);
}

static void device_removed(struct udev_device *udevice)
{
	struct authentication_closure *closure;
	const char *sysfs_path;

	sysfs_path = udev_device_get_syspath(udevice);
	if (!sysfs_path)
		return;

	closure = g_hash_table_lookup(pending_auths, sysfs_path);
	if (!closure)
		return;

	g_hash_table_steal(pending_auths, sysfs_path);
	auth_closure_destroy(closure, true);
}

static gboolean monitor_watch(GIOChannel *source, GIOCondition condition,
							gpointer data)
{
	struct udev_device *udevice;

	udevice = udev_monitor_receive_device(monitor);
	if (!udevice)
		return TRUE;

	if (!g_strcmp0(udev_device_get_action(udevice), "add"))
		device_added(udevice);
	else if (!g_strcmp0(udev_device_get_action(udevice), "remove"))
		device_removed(udevice);

	udev_device_unref(udevice);

	return TRUE;
}

static int sixaxis_init(void)
{
	GIOChannel *channel;

	DBG("");

	ctx = udev_new();
	if (!ctx)
		return -EIO;

	monitor = udev_monitor_new_from_netlink(ctx, "udev");
	if (!monitor) {
		udev_unref(ctx);
		ctx = NULL;

		return -EIO;
	}

	/* Listen for newly connected hidraw interfaces */
	udev_monitor_filter_add_match_subsystem_devtype(monitor, "hidraw",
									NULL);
	udev_monitor_enable_receiving(monitor);

	channel = g_io_channel_unix_new(udev_monitor_get_fd(monitor));
	watch_id = g_io_add_watch(channel, G_IO_IN, monitor_watch, NULL);
	g_io_channel_unref(channel);

	pending_auths = g_hash_table_new(g_str_hash,
					g_str_equal);

	return 0;
}

static void sixaxis_exit(void)
{
	GHashTableIter iter;
	gpointer value;

	DBG("");

	g_hash_table_iter_init(&iter, pending_auths);
	while (g_hash_table_iter_next(&iter, NULL, &value)) {
		struct authentication_closure *closure = value;
		auth_closure_destroy(closure, true);
	}
	g_hash_table_destroy(pending_auths);
	pending_auths = NULL;

	g_source_remove(watch_id);
	watch_id = 0;

	udev_monitor_unref(monitor);
	monitor = NULL;

	udev_unref(ctx);
	ctx = NULL;
}

BLUETOOTH_PLUGIN_DEFINE(sixaxis, VERSION, BLUETOOTH_PLUGIN_PRIORITY_LOW,
						sixaxis_init, sixaxis_exit)
