/*
 *
 *  OBEX Client
 *
 *  Copyright (C) 2012 Intel Corporation
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

#include <stdlib.h>
#include <errno.h>
#include <inttypes.h>

#include <glib.h>

#include "lib/bluetooth.h"
#include "lib/rfcomm.h"
#include "lib/sdp.h"
#include "lib/sdp_lib.h"

#include "gdbus/gdbus.h"
#include "btio/btio.h"

#include "obexd/src/log.h"
#include "transport.h"
#include "bluetooth.h"

#define BT_RX_MTU 32767
#define BT_TX_MTU 32767

#define OBC_BT_ERROR obc_bt_error_quark()

struct bluetooth_session {
	guint id;
	bdaddr_t src;
	bdaddr_t dst;
	uint16_t port;
	sdp_session_t *sdp;
	sdp_record_t *sdp_record;
	GIOChannel *io;
	char *service;
	obc_transport_func func;
	void *user_data;
};

static GSList *sessions = NULL;

static GQuark obc_bt_error_quark(void)
{
	return g_quark_from_static_string("obc-bluetooth-error-quark");
}

static void session_destroy(struct bluetooth_session *session)
{
	DBG("%p", session);

	if (g_slist_find(sessions, session) == NULL)
		return;

	sessions = g_slist_remove(sessions, session);

	if (session->io != NULL) {
		g_io_channel_shutdown(session->io, TRUE, NULL);
		g_io_channel_unref(session->io);
	}

	if (session->sdp)
		sdp_close(session->sdp);

	if (session->sdp_record)
		sdp_record_free(session->sdp_record);

	g_free(session->service);
	g_free(session);
}

static void transport_callback(GIOChannel *io, GError *err, gpointer user_data)
{
	struct bluetooth_session *session = user_data;

	DBG("");

	if (session->func)
		session->func(io, err, session->user_data);

	if (err != NULL)
		session_destroy(session);
}

static GIOChannel *transport_connect(const bdaddr_t *src, const bdaddr_t *dst,
					uint16_t port, BtIOConnect function,
					gpointer user_data)
{
	GIOChannel *io;
	GError *err = NULL;

	DBG("port %u", port);

	if (port > 31) {
		io = bt_io_connect(function, user_data,
				NULL, &err,
				BT_IO_OPT_SOURCE_BDADDR, src,
				BT_IO_OPT_DEST_BDADDR, dst,
				BT_IO_OPT_PSM, port,
				BT_IO_OPT_MODE, BT_IO_MODE_ERTM,
				BT_IO_OPT_OMTU, BT_TX_MTU,
				BT_IO_OPT_IMTU, BT_RX_MTU,
				BT_IO_OPT_SEC_LEVEL, BT_IO_SEC_LOW,
				BT_IO_OPT_INVALID);
	} else {
		io = bt_io_connect(function, user_data,
				NULL, &err,
				BT_IO_OPT_SOURCE_BDADDR, src,
				BT_IO_OPT_DEST_BDADDR, dst,
				BT_IO_OPT_CHANNEL, port,
				BT_IO_OPT_SEC_LEVEL, BT_IO_SEC_LOW,
				BT_IO_OPT_INVALID);
	}

	if (io != NULL)
		return io;

	error("%s", err->message);
	g_error_free(err);
	return NULL;
}

static void search_callback(uint8_t type, uint16_t status,
			uint8_t *rsp, size_t size, void *user_data)
{
	struct bluetooth_session *session = user_data;
	unsigned int scanned, bytesleft = size;
	int seqlen = 0;
	uint8_t dataType;
	uint16_t port = 0;
	GError *gerr = NULL;

	if (status || type != SDP_SVC_SEARCH_ATTR_RSP)
		goto failed;

	scanned = sdp_extract_seqtype(rsp, bytesleft, &dataType, &seqlen);
	if (!scanned || !seqlen)
		goto failed;

	rsp += scanned;
	bytesleft -= scanned;
	do {
		sdp_record_t *rec;
		sdp_list_t *protos;
		sdp_data_t *data;
		int recsize, ch = -1;

		recsize = 0;
		rec = sdp_extract_pdu(rsp, bytesleft, &recsize);
		if (!rec)
			break;

		if (!recsize) {
			sdp_record_free(rec);
			break;
		}

		if (!sdp_get_access_protos(rec, &protos)) {
			ch = sdp_get_proto_port(protos, RFCOMM_UUID);
			sdp_list_foreach(protos,
					(sdp_list_func_t) sdp_list_free, NULL);
			sdp_list_free(protos, NULL);
			protos = NULL;
		}

		data = sdp_data_get(rec, 0x0200);
		/* PSM must be odd and lsb of upper byte must be 0 */
		if (data != NULL && (data->val.uint16 & 0x0101) == 0x0001)
			ch = data->val.uint16;

		/* Cache the sdp record associated with the service that we
		 * attempt to connect. This allows reading its application
		 * specific service attributes. */
		if (ch > 0) {
			port = ch;
			session->sdp_record = rec;
			break;
		}

		sdp_record_free(rec);

		scanned += recsize;
		rsp += recsize;
		bytesleft -= recsize;
	} while (scanned < size && bytesleft > 0);

	if (port == 0)
		goto failed;

	session->port = port;

	g_io_channel_set_close_on_unref(session->io, FALSE);
	g_io_channel_unref(session->io);

	session->io = transport_connect(&session->src, &session->dst, port,
						transport_callback, session);
	if (session->io != NULL) {
		sdp_close(session->sdp);
		session->sdp = NULL;
		return;
	}

failed:
	if (session->io != NULL) {
		g_io_channel_shutdown(session->io, TRUE, NULL);
		g_io_channel_unref(session->io);
		session->io = NULL;
	}

	g_set_error(&gerr, OBC_BT_ERROR, -EIO,
					"Unable to find service record");
	if (session->func)
		session->func(session->io, gerr, session->user_data);

	g_clear_error(&gerr);

	session_destroy(session);
}

static gboolean process_callback(GIOChannel *io, GIOCondition cond,
							gpointer user_data)
{
	struct bluetooth_session *session = user_data;

	if (cond & (G_IO_ERR | G_IO_HUP | G_IO_NVAL))
		return FALSE;

	if (sdp_process(session->sdp) < 0)
		return FALSE;

	return TRUE;
}

static int bt_string2uuid(uuid_t *uuid, const char *string)
{
	uint32_t data0, data4;
	uint16_t data1, data2, data3, data5;

	if (sscanf(string, "%08x-%04hx-%04hx-%04hx-%08x%04hx",
				&data0, &data1, &data2, &data3, &data4, &data5) == 6) {
		uint8_t val[16];

		data0 = g_htonl(data0);
		data1 = g_htons(data1);
		data2 = g_htons(data2);
		data3 = g_htons(data3);
		data4 = g_htonl(data4);
		data5 = g_htons(data5);

		memcpy(&val[0], &data0, 4);
		memcpy(&val[4], &data1, 2);
		memcpy(&val[6], &data2, 2);
		memcpy(&val[8], &data3, 2);
		memcpy(&val[10], &data4, 4);
		memcpy(&val[14], &data5, 2);

		sdp_uuid128_create(uuid, val);

		return 0;
	}

	return -EINVAL;
}

static gboolean service_callback(GIOChannel *io, GIOCondition cond,
							gpointer user_data)
{
	struct bluetooth_session *session = user_data;
	sdp_list_t *search, *attrid;
	uint32_t range = 0x0000ffff;
	GError *gerr = NULL;
	uuid_t uuid;

	if (cond & G_IO_NVAL)
		return FALSE;

	if (cond & G_IO_ERR)
		goto failed;

	if (sdp_set_notify(session->sdp, search_callback, session) < 0)
		goto failed;

	if (bt_string2uuid(&uuid, session->service) < 0)
		goto failed;

	search = sdp_list_append(NULL, &uuid);
	attrid = sdp_list_append(NULL, &range);

	if (sdp_service_search_attr_async(session->sdp,
				search, SDP_ATTR_REQ_RANGE, attrid) < 0) {
		sdp_list_free(attrid, NULL);
		sdp_list_free(search, NULL);
		goto failed;
	}

	sdp_list_free(attrid, NULL);
	sdp_list_free(search, NULL);

	g_io_add_watch(io, G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_NVAL,
						process_callback, session);

	return FALSE;

failed:
	g_io_channel_shutdown(session->io, TRUE, NULL);
	g_io_channel_unref(session->io);
	session->io = NULL;

	g_set_error(&gerr, OBC_BT_ERROR, -EIO,
					"Unable to find service record");
	if (session->func)
		session->func(session->io, gerr, session->user_data);
	g_clear_error(&gerr);

	session_destroy(session);
	return FALSE;
}

static sdp_session_t *service_connect(const bdaddr_t *src, const bdaddr_t *dst,
					GIOFunc function, gpointer user_data)
{
	struct bluetooth_session *session = user_data;
	sdp_session_t *sdp;
	GIOChannel *io;

	DBG("");

	sdp = sdp_connect(src, dst, SDP_NON_BLOCKING);
	if (sdp == NULL)
		return NULL;

	io = g_io_channel_unix_new(sdp_get_socket(sdp));
	if (io == NULL) {
		sdp_close(sdp);
		return NULL;
	}

	g_io_add_watch(io, G_IO_OUT | G_IO_HUP | G_IO_ERR | G_IO_NVAL,
							function, user_data);

	session->io = io;

	return sdp;
}

static int session_connect(struct bluetooth_session *session)
{
	int err;

	DBG("session %p", session);

	if (session->port > 0) {
		session->io = transport_connect(&session->src, &session->dst,
							session->port,
							transport_callback,
							session);
		err = (session->io == NULL) ? -EINVAL : 0;
	} else {
		session->sdp = service_connect(&session->src, &session->dst,
						service_callback, session);
		err = (session->sdp == NULL) ? -ENOMEM : 0;
	}

	return err;
}

static guint bluetooth_connect(const char *source, const char *destination,
				const char *service, uint16_t port,
				obc_transport_func func, void *user_data)
{
	struct bluetooth_session *session;
	static guint id = 0;

	DBG("src %s dest %s service %s port %u",
				source, destination, service, port);

	if (destination == NULL)
		return 0;

	session = g_try_malloc0(sizeof(*session));
	if (session == NULL)
		return 0;

	session->id = ++id;
	session->func = func;
	session->port = port;
	session->user_data = user_data;

	str2ba(destination, &session->dst);
	str2ba(source, &session->src);

	if (session_connect(session) < 0) {
		g_free(session);
		return 0;
	}

	session->service = g_strdup(service);
	sessions = g_slist_prepend(sessions, session);

	return session->id;
}

static void bluetooth_disconnect(guint id)
{
	GSList *l;

	DBG("");

	for (l = sessions; l; l = l->next) {
		struct bluetooth_session *session = l->data;

		if (session->id == id) {
			session_destroy(session);
			return;
		}
	}
}

static int bluetooth_getpacketopt(GIOChannel *io, int *tx_mtu, int *rx_mtu)
{
	int sk = g_io_channel_unix_get_fd(io);
	int type;
	int omtu = -1;
	int imtu = -1;
	socklen_t len = sizeof(int);

	DBG("");

	if (getsockopt(sk, SOL_SOCKET, SO_TYPE, &type, &len) < 0)
		return -errno;

	if (type != SOCK_SEQPACKET)
		return -EINVAL;

	if (!bt_io_get(io, NULL, BT_IO_OPT_OMTU, &omtu,
						BT_IO_OPT_IMTU, &imtu,
						BT_IO_OPT_INVALID))
		return -EINVAL;

	if (tx_mtu)
		*tx_mtu = omtu;

	if (rx_mtu)
		*rx_mtu = imtu;

	return 0;
}

static const void *bluetooth_getattribute(guint id, int attribute_id)
{
	GSList *l;
	sdp_data_t *data;

	for (l = sessions; l; l = l->next) {
		struct bluetooth_session *session = l->data;

		if (session->id != id)
			continue;

		if (session->sdp_record == NULL)
			break;

		/* Read version since UUID is already known */
		if (attribute_id == SDP_ATTR_PFILE_DESC_LIST) {
			sdp_list_t *descs;
			void *ret = NULL;

			if (sdp_get_profile_descs(session->sdp_record,
								&descs) < 0)
				return NULL;

			if (descs && descs->data) {
				sdp_profile_desc_t *desc = descs->data;
				ret = GINT_TO_POINTER(desc->version);
			}

			sdp_list_free(descs, free);

			return ret;
		}

		data = sdp_data_get(session->sdp_record, attribute_id);
		if (!data)
			break;

		return &data->val;
	}
	return NULL;
}

static struct obc_transport bluetooth = {
	.name = "Bluetooth",
	.connect = bluetooth_connect,
	.getpacketopt = bluetooth_getpacketopt,
	.disconnect = bluetooth_disconnect,
	.getattribute = bluetooth_getattribute,
};

int bluetooth_init(void)
{
	DBG("");

	return obc_transport_register(&bluetooth);
}

void bluetooth_exit(void)
{
	DBG("");

	obc_transport_unregister(&bluetooth);
}
