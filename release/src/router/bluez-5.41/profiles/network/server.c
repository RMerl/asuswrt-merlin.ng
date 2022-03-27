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
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <netinet/in.h>

#include <glib.h>

#include "lib/bluetooth.h"
#include "lib/bnep.h"
#include "lib/sdp.h"
#include "lib/sdp_lib.h"
#include "lib/uuid.h"

#include "gdbus/gdbus.h"

#include "btio/btio.h"
#include "src/dbus-common.h"
#include "src/adapter.h"
#include "src/log.h"
#include "src/error.h"
#include "src/sdpd.h"
#include "src/shared/util.h"

#include "bnep.h"
#include "server.h"

#define NETWORK_SERVER_INTERFACE "org.bluez.NetworkServer1"
#define BNEP_INTERFACE "bnep%d"
#define SETUP_TIMEOUT		1

/* Pending Authorization */
struct network_session {
	bdaddr_t	dst;		/* Remote Bluetooth Address */
	char		dev[16];	/* Interface name */
	GIOChannel	*io;		/* Pending connect channel */
	guint		watch;		/* BNEP socket watch */
};

struct network_adapter {
	struct btd_adapter *adapter;	/* Adapter pointer */
	GIOChannel	*io;		/* Bnep socket */
	struct network_session *setup;	/* Setup in progress */
	GSList		*servers;	/* Server register to adapter */
};

/* Main server structure */
struct network_server {
	bdaddr_t	src;		/* Bluetooth Local Address */
	char		*name;		/* Server service name */
	char		*bridge;	/* Bridge name */
	uint32_t	record_id;	/* Service record id */
	uint16_t	id;		/* Service class identifier */
	GSList		*sessions;	/* Active connections */
	struct network_adapter *na;	/* Adapter reference */
	guint		watch_id;	/* Client service watch */
};

static GSList *adapters = NULL;
static gboolean security = TRUE;

static struct network_adapter *find_adapter(GSList *list,
					struct btd_adapter *adapter)
{
	for (; list; list = list->next) {
		struct network_adapter *na = list->data;

		if (na->adapter == adapter)
			return na;
	}

	return NULL;
}

static struct network_server *find_server(GSList *list, uint16_t id)
{
	for (; list; list = list->next) {
		struct network_server *ns = list->data;

		if (ns->id == id)
			return ns;
	}

	return NULL;
}

static struct network_server *find_server_by_uuid(GSList *list,
							const char *uuid)
{
	bt_uuid_t srv_uuid, bnep_uuid;

	if (!bt_string_to_uuid(&srv_uuid, uuid)) {
		for (; list; list = list->next) {
			struct network_server *ns = list->data;

			bt_uuid16_create(&bnep_uuid, ns->id);

			/* UUID value compare */
			if (!bt_uuid_cmp(&srv_uuid, &bnep_uuid))
				return ns;
		}
	} else {
		for (; list; list = list->next) {
			struct network_server *ns = list->data;

			/* String value compare */
			switch (ns->id) {
			case BNEP_SVC_PANU:
				if (!strcasecmp(uuid, "panu"))
					return ns;
				break;
			case BNEP_SVC_NAP:
				if (!strcasecmp(uuid, "nap"))
					return ns;
				break;
			case BNEP_SVC_GN:
				if (!strcasecmp(uuid, "gn"))
					return ns;
				break;
			}
		}
	}

	return NULL;
}

static sdp_record_t *server_record_new(const char *name, uint16_t id)
{
	sdp_list_t *svclass, *pfseq, *apseq, *root, *aproto;
	uuid_t root_uuid, pan, l2cap, bnep;
	sdp_profile_desc_t profile[1];
	sdp_list_t *proto[2];
	sdp_data_t *v, *p;
	uint16_t psm = BNEP_PSM, version = 0x0100;
	uint16_t security_desc = (security ? 0x0001 : 0x0000);
	uint16_t net_access_type = 0xfffe;
	uint32_t max_net_access_rate = 0;
	const char *desc = "Network service";
	sdp_record_t *record;

	record = sdp_record_alloc();
	if (!record)
		return NULL;

	record->attrlist = NULL;
	record->pattern = NULL;

	switch (id) {
	case BNEP_SVC_NAP:
		sdp_uuid16_create(&pan, NAP_SVCLASS_ID);
		svclass = sdp_list_append(NULL, &pan);
		sdp_set_service_classes(record, svclass);

		sdp_uuid16_create(&profile[0].uuid, NAP_PROFILE_ID);
		profile[0].version = 0x0100;
		pfseq = sdp_list_append(NULL, &profile[0]);
		sdp_set_profile_descs(record, pfseq);

		sdp_set_info_attr(record, name, NULL, desc);

		sdp_attr_add_new(record, SDP_ATTR_NET_ACCESS_TYPE,
					SDP_UINT16, &net_access_type);
		sdp_attr_add_new(record, SDP_ATTR_MAX_NET_ACCESSRATE,
					SDP_UINT32, &max_net_access_rate);
		break;
	case BNEP_SVC_GN:
		sdp_uuid16_create(&pan, GN_SVCLASS_ID);
		svclass = sdp_list_append(NULL, &pan);
		sdp_set_service_classes(record, svclass);

		sdp_uuid16_create(&profile[0].uuid, GN_PROFILE_ID);
		profile[0].version = 0x0100;
		pfseq = sdp_list_append(NULL, &profile[0]);
		sdp_set_profile_descs(record, pfseq);

		sdp_set_info_attr(record, name, NULL, desc);
		break;
	case BNEP_SVC_PANU:
		sdp_uuid16_create(&pan, PANU_SVCLASS_ID);
		svclass = sdp_list_append(NULL, &pan);
		sdp_set_service_classes(record, svclass);

		sdp_uuid16_create(&profile[0].uuid, PANU_PROFILE_ID);
		profile[0].version = 0x0100;
		pfseq = sdp_list_append(NULL, &profile[0]);
		sdp_set_profile_descs(record, pfseq);

		sdp_set_info_attr(record, name, NULL, desc);
		break;
	default:
		sdp_record_free(record);
		return NULL;
	}

	sdp_uuid16_create(&root_uuid, PUBLIC_BROWSE_GROUP);
	root = sdp_list_append(NULL, &root_uuid);
	sdp_set_browse_groups(record, root);

	sdp_uuid16_create(&l2cap, L2CAP_UUID);
	proto[0] = sdp_list_append(NULL, &l2cap);
	p = sdp_data_alloc(SDP_UINT16, &psm);
	proto[0] = sdp_list_append(proto[0], p);
	apseq    = sdp_list_append(NULL, proto[0]);

	sdp_uuid16_create(&bnep, BNEP_UUID);
	proto[1] = sdp_list_append(NULL, &bnep);
	v = sdp_data_alloc(SDP_UINT16, &version);
	proto[1] = sdp_list_append(proto[1], v);

	/* Supported protocols */
	{
		uint16_t ptype[] = {
			0x0800,  /* IPv4 */
			0x0806,  /* ARP */
		};
		sdp_data_t *head, *pseq;
		int p;

		for (p = 0, head = NULL; p < 2; p++) {
			sdp_data_t *data = sdp_data_alloc(SDP_UINT16, &ptype[p]);
			if (head)
				sdp_seq_append(head, data);
			else
				head = data;
		}
		pseq = sdp_data_alloc(SDP_SEQ16, head);
		proto[1] = sdp_list_append(proto[1], pseq);
	}

	apseq = sdp_list_append(apseq, proto[1]);

	aproto = sdp_list_append(NULL, apseq);
	sdp_set_access_protos(record, aproto);

	sdp_add_lang_attr(record);

	sdp_attr_add_new(record, SDP_ATTR_SECURITY_DESC,
				SDP_UINT16, &security_desc);

	sdp_data_free(p);
	sdp_data_free(v);
	sdp_list_free(apseq, NULL);
	sdp_list_free(root, NULL);
	sdp_list_free(aproto, NULL);
	sdp_list_free(proto[0], NULL);
	sdp_list_free(proto[1], NULL);
	sdp_list_free(svclass, NULL);
	sdp_list_free(pfseq, NULL);

	return record;
}

static void session_free(void *data)
{
	struct network_session *session = data;

	if (session->watch)
		g_source_remove(session->watch);

	if (session->io)
		g_io_channel_unref(session->io);

	g_free(session);
}

static void setup_destroy(void *user_data)
{
	struct network_adapter *na = user_data;
	struct network_session *setup = na->setup;

	if (!setup)
		return;

	na->setup = NULL;

	session_free(setup);
}

static gboolean bnep_setup(GIOChannel *chan,
			GIOCondition cond, gpointer user_data)
{
	const uint8_t bt_base[] = { 0x00, 0x00, 0x10, 0x00, 0x80, 0x00,
					0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB };
	struct network_adapter *na = user_data;
	struct network_server *ns;
	uint8_t packet[BNEP_MTU];
	struct bnep_setup_conn_req *req = (void *) packet;
	uint16_t dst_role = 0;
	uint32_t val;
	int n, sk;
	char *bridge = NULL;

	if (cond & G_IO_NVAL)
		return FALSE;

	if (cond & (G_IO_ERR | G_IO_HUP)) {
		error("Hangup or error on BNEP socket");
		return FALSE;
	}

	sk = g_io_channel_unix_get_fd(chan);

	/*
	 * BNEP_SETUP_CONNECTION_REQUEST_MSG should be read and left in case
	 * of kernel setup connection msg handling.
	 */
	n = recv(sk, packet, sizeof(packet), MSG_PEEK);
	if (n < 0) {
		error("read(): %s(%d)", strerror(errno), errno);
		return FALSE;
	}

	/*
	 * Initial received data packet is BNEP_SETUP_CONNECTION_REQUEST_MSG
	 * minimal size of this frame is 3 octets: 1 byte of BNEP Type +
	 * 1 byte of BNEP Control Type + 1 byte of BNEP services UUID size.
	 */
	if (n < 3) {
		error("To few setup connection request data received");
		return FALSE;
	}

	switch (req->uuid_size) {
	case 2:
		dst_role = get_be16(req->service);
		break;
	case 16:
		if (memcmp(&req->service[4], bt_base, sizeof(bt_base)) != 0)
			break;

		/* Intentional no-brake */

	case 4:
		val = get_be32(req->service);
		if (val > 0xffff)
			break;

		dst_role = val;
		break;
	default:
		break;
	}

	ns = find_server(na->servers, dst_role);
	if (!ns || !ns->record_id || !ns->bridge)
		error("Server error, bridge not initialized: (0x%x)", dst_role);
	else
		bridge = ns->bridge;

	strncpy(na->setup->dev, BNEP_INTERFACE, 16);
	na->setup->dev[15] = '\0';

	if (bnep_server_add(sk, bridge, na->setup->dev, &na->setup->dst,
							packet, n) < 0)
		error("BNEP server cannot be added");

	na->setup = NULL;

	return FALSE;
}

static void connect_event(GIOChannel *chan, GError *err, gpointer user_data)
{
	struct network_adapter *na = user_data;

	if (err) {
		error("%s", err->message);
		setup_destroy(na);
		return;
	}

	g_io_channel_set_close_on_unref(chan, TRUE);

	na->setup->watch = g_io_add_watch_full(chan, G_PRIORITY_DEFAULT,
				G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_NVAL,
				bnep_setup, na, setup_destroy);
}

static void auth_cb(DBusError *derr, void *user_data)
{
	struct network_adapter *na = user_data;
	GError *err = NULL;

	if (derr) {
		error("Access denied: %s", derr->message);
		goto reject;
	}

	if (!bt_io_accept(na->setup->io, connect_event, na, NULL,
							&err)) {
		error("bt_io_accept: %s", err->message);
		g_error_free(err);
		goto reject;
	}

	return;

reject:
	g_io_channel_shutdown(na->setup->io, TRUE, NULL);
	setup_destroy(na);
}

static void confirm_event(GIOChannel *chan, gpointer user_data)
{
	struct network_adapter *na = user_data;
	struct network_server *ns;
	bdaddr_t src, dst;
	char address[18];
	GError *err = NULL;
	guint ret;

	bt_io_get(chan, &err,
			BT_IO_OPT_SOURCE_BDADDR, &src,
			BT_IO_OPT_DEST_BDADDR, &dst,
			BT_IO_OPT_DEST, address,
			BT_IO_OPT_INVALID);
	if (err) {
		error("%s", err->message);
		g_error_free(err);
		goto drop;
	}

	DBG("BNEP: incoming connect from %s", address);

	if (na->setup) {
		error("Refusing connect from %s: setup in progress", address);
		goto drop;
	}

	ns = find_server(na->servers, BNEP_SVC_NAP);
	if (!ns || !ns->record_id || !ns->bridge)
		goto drop;

	na->setup = g_new0(struct network_session, 1);
	bacpy(&na->setup->dst, &dst);
	na->setup->io = g_io_channel_ref(chan);

	ret = btd_request_authorization(&src, &dst, BNEP_SVC_UUID,
					auth_cb, na);
	if (ret == 0) {
		error("Refusing connect from %s", address);
		setup_destroy(na);
		goto drop;
	}

	return;

drop:
	g_io_channel_shutdown(chan, TRUE, NULL);
}

int server_init(gboolean secure)
{
	security = secure;

	return 0;
}

static uint32_t register_server_record(struct network_server *ns)
{
	sdp_record_t *record;

	record = server_record_new(ns->name, ns->id);
	if (!record) {
		error("Unable to allocate new service record");
		return 0;
	}

	if (adapter_service_add(ns->na->adapter, record) < 0) {
		error("Failed to register service record");
		sdp_record_free(record);
		return 0;
	}

	DBG("got record id 0x%x", record->handle);

	return record->handle;
}

static void server_remove_sessions(struct network_server *ns)
{
	GSList *list;

	for (list = ns->sessions; list; list = list->next) {
		struct network_session *session = list->data;

		if (*session->dev == '\0')
			continue;

		bnep_server_delete(ns->bridge, session->dev, &session->dst);
	}

	g_slist_free_full(ns->sessions, session_free);

	ns->sessions = NULL;
}

static void server_disconnect(DBusConnection *conn, void *user_data)
{
	struct network_server *ns = user_data;

	server_remove_sessions(ns);

	ns->watch_id = 0;

	if (ns->record_id) {
		adapter_service_remove(ns->na->adapter, ns->record_id);
		ns->record_id = 0;
	}

	g_free(ns->bridge);
	ns->bridge = NULL;
}

static DBusMessage *register_server(DBusConnection *conn,
				DBusMessage *msg, void *data)
{
	struct network_adapter *na = data;
	struct network_server *ns;
	DBusMessage *reply;
	const char *uuid, *bridge;

	if (!dbus_message_get_args(msg, NULL, DBUS_TYPE_STRING, &uuid,
				DBUS_TYPE_STRING, &bridge, DBUS_TYPE_INVALID))
		return btd_error_invalid_args(msg);

	ns = find_server_by_uuid(na->servers, uuid);
	if (ns == NULL)
		return btd_error_failed(msg, "Invalid UUID");

	if (ns->record_id)
		return btd_error_already_exists(msg);

	reply = dbus_message_new_method_return(msg);
	if (!reply)
		return NULL;

	ns->record_id = register_server_record(ns);
	if (!ns->record_id)
		return btd_error_failed(msg, "SDP record registration failed");

	g_free(ns->bridge);
	ns->bridge = g_strdup(bridge);

	ns->watch_id = g_dbus_add_disconnect_watch(conn,
					dbus_message_get_sender(msg),
					server_disconnect, ns, NULL);

	return reply;
}

static DBusMessage *unregister_server(DBusConnection *conn,
					DBusMessage *msg, void *data)
{
	struct network_adapter *na = data;
	struct network_server *ns;
	DBusMessage *reply;
	const char *uuid;

	if (!dbus_message_get_args(msg, NULL, DBUS_TYPE_STRING, &uuid,
							DBUS_TYPE_INVALID))
		return btd_error_invalid_args(msg);

	ns = find_server_by_uuid(na->servers, uuid);
	if (!ns)
		return btd_error_failed(msg, "Invalid UUID");

	reply = dbus_message_new_method_return(msg);
	if (!reply)
		return NULL;

	g_dbus_remove_watch(conn, ns->watch_id);

	server_disconnect(conn, ns);

	return reply;
}

static void adapter_free(struct network_adapter *na)
{
	if (na->io != NULL) {
		g_io_channel_shutdown(na->io, TRUE, NULL);
		g_io_channel_unref(na->io);
	}

	setup_destroy(na);
	btd_adapter_unref(na->adapter);
	g_free(na);
}

static void server_free(void *data)
{
	struct network_server *ns = data;

	if (!ns)
		return;

	server_remove_sessions(ns);

	if (ns->record_id)
		adapter_service_remove(ns->na->adapter, ns->record_id);

	g_dbus_remove_watch(btd_get_dbus_connection(), ns->watch_id);
	g_free(ns->name);
	g_free(ns->bridge);

	g_free(ns);
}

static void path_unregister(void *data)
{
	struct network_adapter *na = data;

	DBG("Unregistered interface %s on path %s",
		NETWORK_SERVER_INTERFACE, adapter_get_path(na->adapter));

	g_slist_free_full(na->servers, server_free);

	adapters = g_slist_remove(adapters, na);
	adapter_free(na);
}

static const GDBusMethodTable server_methods[] = {
	{ GDBUS_METHOD("Register",
			GDBUS_ARGS({ "uuid", "s" }, { "bridge", "s" }), NULL,
			register_server) },
	{ GDBUS_METHOD("Unregister",
			GDBUS_ARGS({ "uuid", "s" }), NULL,
			unregister_server) },
	{ }
};

static struct network_adapter *create_adapter(struct btd_adapter *adapter)
{
	struct network_adapter *na;
	GError *err = NULL;

	na = g_new0(struct network_adapter, 1);
	na->adapter = btd_adapter_ref(adapter);

	na->io = bt_io_listen(NULL, confirm_event, na, NULL, &err,
				BT_IO_OPT_SOURCE_BDADDR,
				btd_adapter_get_address(adapter),
				BT_IO_OPT_PSM, BNEP_PSM,
				BT_IO_OPT_OMTU, BNEP_MTU,
				BT_IO_OPT_IMTU, BNEP_MTU,
				BT_IO_OPT_SEC_LEVEL,
				security ? BT_IO_SEC_MEDIUM : BT_IO_SEC_LOW,
				BT_IO_OPT_INVALID);
	if (!na->io) {
		error("%s", err->message);
		g_error_free(err);
		adapter_free(na);
		return NULL;
	}

	return na;
}

int server_register(struct btd_adapter *adapter, uint16_t id)
{
	struct network_adapter *na;
	struct network_server *ns;
	const char *path;

	na = find_adapter(adapters, adapter);
	if (!na) {
		na = create_adapter(adapter);
		if (!na)
			return -EINVAL;
		adapters = g_slist_append(adapters, na);
	}

	ns = find_server(na->servers, id);
	if (ns)
		return 0;

	ns = g_new0(struct network_server, 1);

	ns->name = g_strdup("Network service");

	path = adapter_get_path(adapter);

	if (g_slist_length(na->servers) > 0)
		goto done;

	if (!g_dbus_register_interface(btd_get_dbus_connection(), path,
						NETWORK_SERVER_INTERFACE,
						server_methods, NULL, NULL, na,
						path_unregister)) {
		error("D-Bus failed to register %s interface",
						NETWORK_SERVER_INTERFACE);
		server_free(ns);
		return -1;
	}

	DBG("Registered interface %s on path %s", NETWORK_SERVER_INTERFACE,
									path);

done:
	bacpy(&ns->src, btd_adapter_get_address(adapter));
	ns->id = id;
	ns->na = na;
	ns->record_id = 0;
	na->servers = g_slist_append(na->servers, ns);

	return 0;
}

int server_unregister(struct btd_adapter *adapter, uint16_t id)
{
	struct network_adapter *na;
	struct network_server *ns;

	na = find_adapter(adapters, adapter);
	if (!na)
		return -EINVAL;

	ns = find_server(na->servers, id);
	if (!ns)
		return -EINVAL;

	na->servers = g_slist_remove(na->servers, ns);
	server_free(ns);

	if (g_slist_length(na->servers) > 0)
		return 0;

	g_dbus_unregister_interface(btd_get_dbus_connection(),
						adapter_get_path(adapter),
						NETWORK_SERVER_INTERFACE);

	return 0;
}
