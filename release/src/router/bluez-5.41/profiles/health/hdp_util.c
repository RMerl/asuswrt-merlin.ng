/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2010 GSyC/LibreSoft, Universidad Rey Juan Carlos.
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

#include <stdint.h>
#include <stdbool.h>

#include <glib.h>

#include "lib/bluetooth.h"
#include "lib/sdp.h"
#include "lib/sdp_lib.h"
#include "lib/uuid.h"

#include "gdbus/gdbus.h"

#include "btio/btio.h"
#include "src/adapter.h"
#include "src/device.h"
#include "src/sdpd.h"
#include "src/sdp-client.h"
#include "src/uuid-helper.h"
#include "src/log.h"
#include "src/dbus-common.h"

#include "mcap.h"
#include "hdp_types.h"
#include "hdp.h"
#include "hdp_util.h"

typedef gboolean (*parse_item_f)(DBusMessageIter *iter, gpointer user_data,
								GError **err);

struct dict_entry_func {
	char		*key;
	parse_item_f	func;
};

struct get_mdep_data {
	struct hdp_application	*app;
	gpointer		data;
	hdp_continue_mdep_f	func;
	GDestroyNotify		destroy;
};

struct conn_mcl_data {
	int			refs;
	gpointer		data;
	hdp_continue_proc_f	func;
	GDestroyNotify		destroy;
	struct hdp_device	*dev;
};

struct get_dcpsm_data {
	gpointer		data;
	hdp_continue_dcpsm_f	func;
	GDestroyNotify		destroy;
};

static gboolean parse_dict_entry(struct dict_entry_func dict_context[],
							DBusMessageIter *iter,
							GError **err,
							gpointer user_data)
{
	DBusMessageIter entry;
	char *key;
	int ctype, i;
	struct dict_entry_func df;

	dbus_message_iter_recurse(iter, &entry);
	ctype = dbus_message_iter_get_arg_type(&entry);
	if (ctype != DBUS_TYPE_STRING) {
		g_set_error(err, HDP_ERROR, HDP_DIC_ENTRY_PARSE_ERROR,
			"Dictionary entries should have a string as key");
		return FALSE;
	}

	dbus_message_iter_get_basic(&entry, &key);
	dbus_message_iter_next(&entry);
	/* Find function and call it */
	for (i = 0, df = dict_context[0]; df.key; i++, df = dict_context[i]) {
		if (g_ascii_strcasecmp(df.key, key) == 0)
			return df.func(&entry, user_data, err);
	}

	g_set_error(err, HDP_ERROR, HDP_DIC_ENTRY_PARSE_ERROR,
			"No function found for parsing value for key %s", key);
	return FALSE;
}

static gboolean parse_dict(struct dict_entry_func dict_context[],
							DBusMessageIter *iter,
							GError **err,
							gpointer user_data)
{
	int ctype;
	DBusMessageIter dict;

	ctype = dbus_message_iter_get_arg_type(iter);
	if (ctype != DBUS_TYPE_ARRAY) {
		g_set_error(err, HDP_ERROR, HDP_DIC_PARSE_ERROR,
					"Dictionary should be an array");
		return FALSE;
	}

	dbus_message_iter_recurse(iter, &dict);
	while ((ctype = dbus_message_iter_get_arg_type(&dict)) !=
							DBUS_TYPE_INVALID) {
		if (ctype != DBUS_TYPE_DICT_ENTRY) {
			g_set_error(err, HDP_ERROR, HDP_DIC_PARSE_ERROR,
						"Dictionary array should "
						"contain dict entries");
			return FALSE;
		}

		/* Start parsing entry */
		if (!parse_dict_entry(dict_context, &dict, err,
							user_data))
			return FALSE;
		/* Finish entry parsing */

		dbus_message_iter_next(&dict);
	}

	return TRUE;
}

static gboolean parse_data_type(DBusMessageIter *iter, gpointer data,
								GError **err)
{
	struct hdp_application *app = data;
	DBusMessageIter *value;
	DBusMessageIter variant;
	int ctype;

	ctype = dbus_message_iter_get_arg_type(iter);
	value = iter;
	if (ctype == DBUS_TYPE_VARIANT) {
		/* Get value inside the variable */
		dbus_message_iter_recurse(iter, &variant);
		ctype = dbus_message_iter_get_arg_type(&variant);
		value = &variant;
	}

	if (ctype != DBUS_TYPE_UINT16) {
		g_set_error(err, HDP_ERROR, HDP_DIC_ENTRY_PARSE_ERROR,
			"Final value for data type should be uint16");
		return FALSE;
	}

	dbus_message_iter_get_basic(value, &app->data_type);
	app->data_type_set = TRUE;
	return TRUE;
}

static gboolean parse_role(DBusMessageIter *iter, gpointer data, GError **err)
{
	struct hdp_application *app = data;
	DBusMessageIter *string;
	DBusMessageIter value;
	int ctype;
	const char *role;

	ctype = dbus_message_iter_get_arg_type(iter);
	if (ctype == DBUS_TYPE_VARIANT) {
		/* Get value inside the variable */
		dbus_message_iter_recurse(iter, &value);
		ctype = dbus_message_iter_get_arg_type(&value);
		string = &value;
	} else {
		string = iter;
	}

	if (ctype != DBUS_TYPE_STRING) {
		g_set_error(err, HDP_ERROR, HDP_UNSPECIFIED_ERROR,
				"Value data spec should be variable or string");
		return FALSE;
	}

	dbus_message_iter_get_basic(string, &role);
	if (g_ascii_strcasecmp(role, HDP_SINK_ROLE_AS_STRING) == 0) {
		app->role = HDP_SINK;
	} else if (g_ascii_strcasecmp(role, HDP_SOURCE_ROLE_AS_STRING) == 0) {
		app->role = HDP_SOURCE;
	} else {
		g_set_error(err, HDP_ERROR, HDP_UNSPECIFIED_ERROR,
			"Role value should be \"source\" or \"sink\"");
		return FALSE;
	}

	app->role_set = TRUE;

	return TRUE;
}

static gboolean parse_desc(DBusMessageIter *iter, gpointer data, GError **err)
{
	struct hdp_application *app = data;
	DBusMessageIter *string;
	DBusMessageIter variant;
	int ctype;
	const char *desc;

	ctype = dbus_message_iter_get_arg_type(iter);
	if (ctype == DBUS_TYPE_VARIANT) {
		/* Get value inside the variable */
		dbus_message_iter_recurse(iter, &variant);
		ctype = dbus_message_iter_get_arg_type(&variant);
		string = &variant;
	} else {
		string = iter;
	}

	if (ctype != DBUS_TYPE_STRING) {
		g_set_error(err, HDP_ERROR, HDP_DIC_ENTRY_PARSE_ERROR,
				"Value data spec should be variable or string");
		return FALSE;
	}

	dbus_message_iter_get_basic(string, &desc);
	app->description = g_strdup(desc);
	return TRUE;
}

static gboolean parse_chan_type(DBusMessageIter *iter, gpointer data,
								GError **err)
{
	struct hdp_application *app = data;
	DBusMessageIter *value;
	DBusMessageIter variant;
	char *chan_type;
	int ctype;

	ctype = dbus_message_iter_get_arg_type(iter);
	value = iter;
	if (ctype == DBUS_TYPE_VARIANT) {
		/* Get value inside the variable */
		dbus_message_iter_recurse(iter, &variant);
		ctype = dbus_message_iter_get_arg_type(&variant);
		value = &variant;
	}

	if (ctype != DBUS_TYPE_STRING) {
		g_set_error(err, HDP_ERROR, HDP_DIC_ENTRY_PARSE_ERROR,
			"Final value for channel type should be an string");
		return FALSE;
	}

	dbus_message_iter_get_basic(value, &chan_type);

	if (g_ascii_strcasecmp("reliable", chan_type) == 0)
		app->chan_type = HDP_RELIABLE_DC;
	else if (g_ascii_strcasecmp("streaming", chan_type) == 0)
		app->chan_type = HDP_STREAMING_DC;
	else {
		g_set_error(err, HDP_ERROR, HDP_DIC_ENTRY_PARSE_ERROR,
						"Invalid value for data type");
		return FALSE;
	}

	app->chan_type_set = TRUE;

	return TRUE;
}

static struct dict_entry_func dict_parser[] = {
	{"DataType",		parse_data_type},
	{"Role",		parse_role},
	{"Description",		parse_desc},
	{"ChannelType",		parse_chan_type},
	{NULL, NULL}
};

struct hdp_application *hdp_get_app_config(DBusMessageIter *iter, GError **err)
{
	struct hdp_application *app;

	app = g_new0(struct hdp_application, 1);
	app->ref = 1;
	if (!parse_dict(dict_parser, iter, err, app))
		goto fail;
	if (!app->data_type_set || !app->role_set) {
		g_set_error(err, HDP_ERROR, HDP_DIC_PARSE_ERROR,
						"Mandatory fields aren't set");
		goto fail;
	}
	return app;

fail:
	hdp_application_unref(app);
	return NULL;
}

static gboolean is_app_role(GSList *app_list, HdpRole role)
{
	GSList *l;

	for (l = app_list; l; l = l->next) {
		struct hdp_application *app = l->data;

		if (app->role == role)
			return TRUE;
	}

	return FALSE;
}

static gboolean set_sdp_services_uuid(sdp_record_t *record, HdpRole role)
{
	uuid_t svc_uuid_source, svc_uuid_sink;
	sdp_list_t *svc_list = NULL;

	sdp_uuid16_create(&svc_uuid_sink, HDP_SINK_SVCLASS_ID);
	sdp_uuid16_create(&svc_uuid_source, HDP_SOURCE_SVCLASS_ID);

	sdp_get_service_classes(record, &svc_list);

	if (role == HDP_SOURCE) {
		if (!sdp_list_find(svc_list, &svc_uuid_source, sdp_uuid_cmp))
			svc_list = sdp_list_append(svc_list, &svc_uuid_source);
	} else if (role == HDP_SINK) {
		if (!sdp_list_find(svc_list, &svc_uuid_sink, sdp_uuid_cmp))
			svc_list = sdp_list_append(svc_list, &svc_uuid_sink);
	}

	if (sdp_set_service_classes(record, svc_list) < 0) {
		sdp_list_free(svc_list, NULL);
		return FALSE;
	}

	sdp_list_free(svc_list, NULL);

	return TRUE;
}

static gboolean register_service_protocols(struct hdp_adapter *adapter,
						sdp_record_t *sdp_record)
{
	gboolean ret;
	uuid_t l2cap_uuid, mcap_c_uuid;
	sdp_list_t *l2cap_list, *proto_list = NULL, *mcap_list = NULL;
	sdp_list_t *access_proto_list = NULL;
	sdp_data_t *psm = NULL, *mcap_ver = NULL;
	uint16_t version = MCAP_VERSION;

	/* set l2cap information */
	sdp_uuid16_create(&l2cap_uuid, L2CAP_UUID);
	l2cap_list = sdp_list_append(NULL, &l2cap_uuid);
	if (l2cap_list == NULL) {
		ret = FALSE;
		goto end;
	}

	psm = sdp_data_alloc(SDP_UINT16, &adapter->ccpsm);
	if (psm == NULL) {
		ret = FALSE;
		goto end;
	}

	if (sdp_list_append(l2cap_list, psm) == NULL) {
		ret = FALSE;
		goto end;
	}

	proto_list = sdp_list_append(NULL, l2cap_list);
	if (proto_list == NULL) {
		ret = FALSE;
		goto end;
	}

	/* set mcap information */
	sdp_uuid16_create(&mcap_c_uuid, MCAP_CTRL_UUID);
	mcap_list = sdp_list_append(NULL, &mcap_c_uuid);
	if (mcap_list == NULL) {
		ret = FALSE;
		goto end;
	}

	mcap_ver = sdp_data_alloc(SDP_UINT16, &version);
	if (mcap_ver == NULL) {
		ret = FALSE;
		goto end;
	}

	if (sdp_list_append(mcap_list, mcap_ver) == NULL) {
		ret = FALSE;
		goto end;
	}

	if (sdp_list_append(proto_list, mcap_list) == NULL) {
		ret = FALSE;
		goto end;
	}

	/* attach protocol information to service record */
	access_proto_list = sdp_list_append(NULL, proto_list);
	if (access_proto_list == NULL) {
		ret = FALSE;
		goto end;
	}

	ret = TRUE;
	sdp_set_access_protos(sdp_record, access_proto_list);

end:
	if (l2cap_list != NULL)
		sdp_list_free(l2cap_list, NULL);
	if (mcap_list != NULL)
		sdp_list_free(mcap_list, NULL);
	if (proto_list != NULL)
		sdp_list_free(proto_list, NULL);
	if (access_proto_list != NULL)
		sdp_list_free(access_proto_list, NULL);
	if (psm != NULL)
		sdp_data_free(psm);
	if (mcap_ver != NULL)
		sdp_data_free(mcap_ver);

	return ret;
}

static gboolean register_service_profiles(sdp_record_t *sdp_record)
{
	gboolean ret;
	sdp_list_t *profile_list;
	sdp_profile_desc_t hdp_profile;

	/* set hdp information */
	sdp_uuid16_create(&hdp_profile.uuid, HDP_SVCLASS_ID);
	hdp_profile.version = HDP_VERSION;
	profile_list = sdp_list_append(NULL, &hdp_profile);
	if (profile_list == NULL)
		return FALSE;

	/* set profile descriptor list */
	if (sdp_set_profile_descs(sdp_record, profile_list) < 0)
		ret = FALSE;
	else
		ret = TRUE;

	sdp_list_free(profile_list, NULL);

	return ret;
}

static gboolean register_service_additional_protocols(
						struct hdp_adapter *adapter,
						sdp_record_t *sdp_record)
{
	gboolean ret = TRUE;
	uuid_t l2cap_uuid, mcap_d_uuid;
	sdp_list_t *l2cap_list, *proto_list = NULL, *mcap_list = NULL;
	sdp_list_t *access_proto_list = NULL;
	sdp_data_t *psm = NULL;

	/* set l2cap information */
	sdp_uuid16_create(&l2cap_uuid, L2CAP_UUID);
	l2cap_list = sdp_list_append(NULL, &l2cap_uuid);
	if (l2cap_list == NULL) {
		ret = FALSE;
		goto end;
	}

	psm = sdp_data_alloc(SDP_UINT16, &adapter->dcpsm);
	if (psm == NULL) {
		ret = FALSE;
		goto end;
	}

	if (sdp_list_append(l2cap_list, psm) == NULL) {
		ret = FALSE;
		goto end;
	}

	proto_list = sdp_list_append(NULL, l2cap_list);
	if (proto_list == NULL) {
		ret = FALSE;
		goto end;
	}

	/* set mcap information */
	sdp_uuid16_create(&mcap_d_uuid, MCAP_DATA_UUID);
	mcap_list = sdp_list_append(NULL, &mcap_d_uuid);
	if (mcap_list == NULL) {
		ret = FALSE;
		goto end;
	}

	if (sdp_list_append(proto_list, mcap_list) == NULL) {
		ret = FALSE;
		goto end;
	}

	/* attach protocol information to service record */
	access_proto_list = sdp_list_append(NULL, proto_list);
	if (access_proto_list == NULL) {
		ret = FALSE;
		goto end;
	}

	sdp_set_add_access_protos(sdp_record, access_proto_list);

end:
	if (l2cap_list != NULL)
		sdp_list_free(l2cap_list, NULL);
	if (mcap_list != NULL)
		sdp_list_free(mcap_list, NULL);
	if (proto_list  != NULL)
		sdp_list_free(proto_list, NULL);
	if (access_proto_list != NULL)
		sdp_list_free(access_proto_list, NULL);
	if (psm != NULL)
		sdp_data_free(psm);

	return ret;
}

static sdp_list_t *app_to_sdplist(struct hdp_application *app)
{
	sdp_data_t *mdepid,
		*dtype = NULL,
		*role = NULL,
		*desc = NULL;
	sdp_list_t *f_list = NULL;

	mdepid = sdp_data_alloc(SDP_UINT8, &app->id);
	if (mdepid == NULL)
		return NULL;

	dtype = sdp_data_alloc(SDP_UINT16, &app->data_type);
	if (dtype == NULL)
		goto fail;

	role = sdp_data_alloc(SDP_UINT8, &app->role);
	if (role == NULL)
		goto fail;

	if (app->description != NULL) {
		desc = sdp_data_alloc(SDP_TEXT_STR8, app->description);
		if (desc == NULL)
			goto fail;
	}

	f_list = sdp_list_append(NULL, mdepid);
	if (f_list == NULL)
		goto fail;

	if (sdp_list_append(f_list, dtype) == NULL)
		goto fail;

	if (sdp_list_append(f_list, role) == NULL)
		goto fail;

	if (desc != NULL)
		if (sdp_list_append(f_list, desc) == NULL)
			goto fail;

	return f_list;

fail:
	if (f_list != NULL)
		sdp_list_free(f_list, NULL);
	if (mdepid != NULL)
		sdp_data_free(mdepid);
	if (dtype != NULL)
		sdp_data_free(dtype);
	if (role != NULL)
		sdp_data_free(role);
	if (desc != NULL)
		sdp_data_free(desc);

	return NULL;
}

static void free_hdp_list(void *list)
{
	sdp_list_t *hdp_list = list;

	sdp_list_free(hdp_list, (sdp_free_func_t)sdp_data_free);
}

static gboolean register_features(struct hdp_application *app,
						sdp_list_t **sup_features)
{
	sdp_list_t *hdp_feature;

	hdp_feature = app_to_sdplist(app);
	if (hdp_feature == NULL)
		goto fail;

	if (*sup_features == NULL) {
		*sup_features = sdp_list_append(NULL, hdp_feature);
		if (*sup_features == NULL)
			goto fail;
	} else if (sdp_list_append(*sup_features, hdp_feature) == NULL) {
		goto fail;
	}

	return TRUE;

fail:
	if (hdp_feature != NULL)
		sdp_list_free(hdp_feature, (sdp_free_func_t)sdp_data_free);
	if (*sup_features != NULL)
		sdp_list_free(*sup_features, free_hdp_list);
	return FALSE;
}

static gboolean register_service_sup_features(GSList *app_list,
						sdp_record_t *sdp_record)
{
	GSList *l;
	sdp_list_t *sup_features = NULL;

	for (l = app_list; l; l = l->next) {
		if (!register_features(l->data, &sup_features))
			return FALSE;
	}

	if (sdp_set_supp_feat(sdp_record, sup_features) < 0) {
		sdp_list_free(sup_features, free_hdp_list);
		return FALSE;
	}

	sdp_list_free(sup_features, free_hdp_list);

	return TRUE;
}

static gboolean register_data_exchange_spec(sdp_record_t *record)
{
	sdp_data_t *spec;
	uint8_t data_spec = DATA_EXCHANGE_SPEC_11073;
	/* As by now 11073 is the only supported we set it by default */

	spec = sdp_data_alloc(SDP_UINT8, &data_spec);
	if (spec == NULL)
		return FALSE;

	if (sdp_attr_add(record, SDP_ATTR_DATA_EXCHANGE_SPEC, spec) < 0) {
		sdp_data_free(spec);
		return FALSE;
	}

	return TRUE;
}

static gboolean register_mcap_features(sdp_record_t *sdp_record)
{
	sdp_data_t *mcap_proc;
	uint8_t mcap_sup_proc = MCAP_SUP_PROC;

	mcap_proc = sdp_data_alloc(SDP_UINT8, &mcap_sup_proc);
	if (mcap_proc == NULL)
		return FALSE;

	if (sdp_attr_add(sdp_record, SDP_ATTR_MCAP_SUPPORTED_PROCEDURES,
							mcap_proc) < 0) {
		sdp_data_free(mcap_proc);
		return FALSE;
	}

	return TRUE;
}

gboolean hdp_update_sdp_record(struct hdp_adapter *adapter, GSList *app_list)
{
	sdp_record_t *sdp_record;

	if (adapter->sdp_handler > 0)
		adapter_service_remove(adapter->btd_adapter,
					adapter->sdp_handler);

	if (app_list == NULL) {
		adapter->sdp_handler = 0;
		return TRUE;
	}

	sdp_record = sdp_record_alloc();
	if (sdp_record == NULL)
		return FALSE;

	if (adapter->sdp_handler > 0)
		sdp_record->handle = adapter->sdp_handler;
	else
		sdp_record->handle = 0xffffffff; /* Set automatically */

	if (is_app_role(app_list, HDP_SINK))
		set_sdp_services_uuid(sdp_record, HDP_SINK);
	if (is_app_role(app_list, HDP_SOURCE))
		set_sdp_services_uuid(sdp_record, HDP_SOURCE);

	if (!register_service_protocols(adapter, sdp_record))
		goto fail;
	if (!register_service_profiles(sdp_record))
		goto fail;
	if (!register_service_additional_protocols(adapter, sdp_record))
		goto fail;

	sdp_set_info_attr(sdp_record, HDP_SERVICE_NAME, HDP_SERVICE_PROVIDER,
							HDP_SERVICE_DSC);
	if (!register_service_sup_features(app_list, sdp_record))
		goto fail;
	if (!register_data_exchange_spec(sdp_record))
		goto fail;

	register_mcap_features(sdp_record);

	if (sdp_set_record_state(sdp_record, adapter->record_state++) < 0)
		goto fail;

	if (adapter_service_add(adapter->btd_adapter, sdp_record) < 0)
		goto fail;
	adapter->sdp_handler = sdp_record->handle;
	return TRUE;

fail:
	if (sdp_record != NULL)
		sdp_record_free(sdp_record);
	return FALSE;
}

static gboolean check_role(uint8_t rec_role, uint8_t app_role)
{
	if ((rec_role == HDP_SINK && app_role == HDP_SOURCE) ||
			(rec_role == HDP_SOURCE && app_role == HDP_SINK))
		return TRUE;

	return FALSE;
}

static gboolean get_mdep_from_rec(const sdp_record_t *rec, uint8_t role,
				uint16_t d_type, uint8_t *mdep, char **desc)
{
	sdp_data_t *list, *feat;

	if (desc == NULL && mdep == NULL)
		return TRUE;

	list = sdp_data_get(rec, SDP_ATTR_SUPPORTED_FEATURES_LIST);
	if (list == NULL || !SDP_IS_SEQ(list->dtd))
		return FALSE;

	for (feat = list->val.dataseq; feat; feat = feat->next) {
		sdp_data_t *data_type, *mdepid, *role_t, *desc_t;

		if (!SDP_IS_SEQ(feat->dtd))
			continue;

		mdepid = feat->val.dataseq;
		if (mdepid == NULL)
			continue;

		data_type = mdepid->next;
		if (data_type == NULL)
			continue;

		role_t = data_type->next;
		if (role_t == NULL)
			continue;

		desc_t = role_t->next;

		if (data_type->dtd != SDP_UINT16 || mdepid->dtd != SDP_UINT8 ||
						role_t->dtd != SDP_UINT8)
			continue;

		if (data_type->val.uint16 != d_type ||
					!check_role(role_t->val.uint8, role))
			continue;

		if (mdep != NULL)
			*mdep = mdepid->val.uint8;

		if (desc != NULL && desc_t != NULL &&
						SDP_IS_TEXT_STR(desc_t->dtd))
			*desc = g_strdup(desc_t->val.str);

		return TRUE;
	}

	return FALSE;
}

static void get_mdep_cb(sdp_list_t *recs, int err, gpointer user_data)
{
	struct get_mdep_data *mdep_data = user_data;
	GError *gerr = NULL;
	uint8_t mdep;

	if (err < 0 || recs == NULL) {
		g_set_error(&gerr, HDP_ERROR, HDP_CONNECTION_ERROR,
					"Error getting remote SDP records");
		mdep_data->func(0, mdep_data->data, gerr);
		g_error_free(gerr);
		return;
	}

	if (!get_mdep_from_rec(recs->data, mdep_data->app->role,
				mdep_data->app->data_type, &mdep, NULL)) {
		g_set_error(&gerr, HDP_ERROR, HDP_CONNECTION_ERROR,
					"No matching MDEP found");
		mdep_data->func(0, mdep_data->data, gerr);
		g_error_free(gerr);
		return;
	}

	mdep_data->func(mdep, mdep_data->data, NULL);
}

static void free_mdep_data(gpointer data)
{
	struct get_mdep_data *mdep_data = data;

	if (mdep_data->destroy)
		mdep_data->destroy(mdep_data->data);
	hdp_application_unref(mdep_data->app);

	g_free(mdep_data);
}

gboolean hdp_get_mdep(struct hdp_device *device, struct hdp_application *app,
				hdp_continue_mdep_f func, gpointer data,
				GDestroyNotify destroy, GError **err)
{
	struct get_mdep_data *mdep_data;
	const bdaddr_t *src;
	const bdaddr_t *dst;
	uuid_t uuid;

	src = btd_adapter_get_address(device_get_adapter(device->dev));
	dst = device_get_address(device->dev);

	mdep_data = g_new0(struct get_mdep_data, 1);
	mdep_data->app = hdp_application_ref(app);
	mdep_data->func = func;
	mdep_data->data = data;
	mdep_data->destroy = destroy;

	bt_string2uuid(&uuid, HDP_UUID);
	if (bt_search_service(src, dst, &uuid, get_mdep_cb, mdep_data,
						free_mdep_data, 0) < 0) {
		g_set_error(err, HDP_ERROR, HDP_CONNECTION_ERROR,
						"Can't get remote SDP record");
		g_free(mdep_data);
		return FALSE;
	}

	return TRUE;
}

static gboolean get_prot_desc_entry(sdp_data_t *entry, int type, guint16 *val)
{
	sdp_data_t *iter;
	int proto;

	if (entry == NULL || !SDP_IS_SEQ(entry->dtd))
		return FALSE;

	iter = entry->val.dataseq;
	if (!(iter->dtd & SDP_UUID_UNSPEC))
		return FALSE;

	proto = sdp_uuid_to_proto(&iter->val.uuid);
	if (proto != type)
		return FALSE;

	if (val == NULL)
		return TRUE;

	iter = iter->next;
	if (iter->dtd != SDP_UINT16)
		return FALSE;

	*val = iter->val.uint16;

	return TRUE;
}

static gboolean hdp_get_prot_desc_list(const sdp_record_t *rec, guint16 *psm,
							guint16 *version)
{
	sdp_data_t *pdl, *p0, *p1;

	if (psm == NULL && version == NULL)
		return TRUE;

	pdl = sdp_data_get(rec, SDP_ATTR_PROTO_DESC_LIST);
	if (pdl == NULL || !SDP_IS_SEQ(pdl->dtd))
		return FALSE;

	p0 = pdl->val.dataseq;
	if (!get_prot_desc_entry(p0, L2CAP_UUID, psm))
		return FALSE;

	p1 = p0->next;
	if (!get_prot_desc_entry(p1, MCAP_CTRL_UUID, version))
		return FALSE;

	return TRUE;
}

static gboolean hdp_get_add_prot_desc_list(const sdp_record_t *rec,
								guint16 *psm)
{
	sdp_data_t *pdl, *p0, *p1;

	if (psm == NULL)
		return TRUE;

	pdl = sdp_data_get(rec, SDP_ATTR_ADD_PROTO_DESC_LIST);
	if (pdl == NULL || pdl->dtd != SDP_SEQ8)
		return FALSE;
	pdl = pdl->val.dataseq;
	if (pdl->dtd != SDP_SEQ8)
		return FALSE;

	p0 = pdl->val.dataseq;

	if (!get_prot_desc_entry(p0, L2CAP_UUID, psm))
		return FALSE;
	p1 = p0->next;
	if (!get_prot_desc_entry(p1, MCAP_DATA_UUID, NULL))
		return FALSE;

	return TRUE;
}

static gboolean get_ccpsm(sdp_list_t *recs, uint16_t *ccpsm)
{
	sdp_list_t *l;

	for (l = recs; l; l = l->next) {
		sdp_record_t *rec = l->data;

		if (hdp_get_prot_desc_list(rec, ccpsm, NULL))
			return TRUE;
	}

	return FALSE;
}

static gboolean get_dcpsm(sdp_list_t *recs, uint16_t *dcpsm)
{
	sdp_list_t *l;

	for (l = recs; l; l = l->next) {
		sdp_record_t *rec = l->data;

		if (hdp_get_add_prot_desc_list(rec, dcpsm))
			return TRUE;
	}

	return FALSE;
}

static void con_mcl_data_unref(struct conn_mcl_data *conn_data)
{
	if (conn_data == NULL)
		return;

	if (--conn_data->refs > 0)
		return;

	if (conn_data->destroy)
		conn_data->destroy(conn_data->data);

	health_device_unref(conn_data->dev);
	g_free(conn_data);
}

static void destroy_con_mcl_data(gpointer data)
{
	con_mcl_data_unref(data);
}

static struct conn_mcl_data *con_mcl_data_ref(struct conn_mcl_data *conn_data)
{
	if (conn_data == NULL)
		return NULL;

	conn_data->refs++;
	return conn_data;
}

static void create_mcl_cb(struct mcap_mcl *mcl, GError *err, gpointer data)
{
	struct conn_mcl_data *conn_data = data;
	struct hdp_device *device = conn_data->dev;
	GError *gerr = NULL;

	if (err != NULL) {
		conn_data->func(conn_data->data, err);
		return;
	}

	if (device->mcl == NULL)
		device->mcl = mcap_mcl_ref(mcl);
	device->mcl_conn = TRUE;

	hdp_set_mcl_cb(device, &gerr);

	conn_data->func(conn_data->data, gerr);
	if (gerr != NULL)
		g_error_free(gerr);
}

static void search_cb(sdp_list_t *recs, int err, gpointer user_data)
{
	struct conn_mcl_data *conn_data = user_data;
	GError *gerr = NULL;
	uint16_t ccpsm;

	if (conn_data->dev->hdp_adapter->mi == NULL) {
		g_set_error(&gerr, HDP_ERROR, HDP_CONNECTION_ERROR,
						"Mcap instance released");
		goto fail;
	}

	if (err < 0 || recs == NULL) {
		g_set_error(&gerr, HDP_ERROR, HDP_CONNECTION_ERROR,
					"Error getting remote SDP records");
		goto fail;
	}

	if (!get_ccpsm(recs, &ccpsm)) {
		g_set_error(&gerr, HDP_ERROR, HDP_CONNECTION_ERROR,
				"Can't get remote PSM for control channel");
		goto fail;
	}

	conn_data = con_mcl_data_ref(conn_data);

	if (!mcap_create_mcl(conn_data->dev->hdp_adapter->mi,
					device_get_address(conn_data->dev->dev),
					ccpsm, create_mcl_cb, conn_data,
					destroy_con_mcl_data, &gerr)) {
		con_mcl_data_unref(conn_data);
		goto fail;
	}
	return;
fail:
	conn_data->func(conn_data->data, gerr);
	g_error_free(gerr);
}

gboolean hdp_establish_mcl(struct hdp_device *device,
						hdp_continue_proc_f func,
						gpointer data,
						GDestroyNotify destroy,
						GError **err)
{
	struct conn_mcl_data *conn_data;
	const bdaddr_t *src;
	const bdaddr_t *dst;
	uuid_t uuid;

	src = btd_adapter_get_address(device_get_adapter(device->dev));
	dst = device_get_address(device->dev);

	conn_data = g_new0(struct conn_mcl_data, 1);
	conn_data->refs = 1;
	conn_data->func = func;
	conn_data->data = data;
	conn_data->destroy = destroy;
	conn_data->dev = health_device_ref(device);

	bt_string2uuid(&uuid, HDP_UUID);
	if (bt_search_service(src, dst, &uuid, search_cb, conn_data,
					destroy_con_mcl_data, 0) < 0) {
		g_set_error(err, HDP_ERROR, HDP_CONNECTION_ERROR,
						"Can't get remote SDP record");
		g_free(conn_data);
		return FALSE;
	}

	return TRUE;
}

static void get_dcpsm_cb(sdp_list_t *recs, int err, gpointer data)
{
	struct get_dcpsm_data *dcpsm_data = data;
	GError *gerr = NULL;
	uint16_t dcpsm;

	if (err < 0 || recs == NULL) {
		g_set_error(&gerr, HDP_ERROR, HDP_CONNECTION_ERROR,
					"Error getting remote SDP records");
		goto fail;
	}

	if (!get_dcpsm(recs, &dcpsm)) {
		g_set_error(&gerr, HDP_ERROR, HDP_CONNECTION_ERROR,
				"Can't get remote PSM for data channel");
		goto fail;
	}

	dcpsm_data->func(dcpsm, dcpsm_data->data, NULL);
	return;

fail:
	dcpsm_data->func(0, dcpsm_data->data, gerr);
	g_error_free(gerr);
}

static void free_dcpsm_data(gpointer data)
{
	struct get_dcpsm_data *dcpsm_data = data;

	if (dcpsm_data == NULL)
		return;

	if (dcpsm_data->destroy)
		dcpsm_data->destroy(dcpsm_data->data);

	g_free(dcpsm_data);
}

gboolean hdp_get_dcpsm(struct hdp_device *device, hdp_continue_dcpsm_f func,
							gpointer data,
							GDestroyNotify destroy,
							GError **err)
{
	struct get_dcpsm_data *dcpsm_data;
	const bdaddr_t *src;
	const bdaddr_t *dst;
	uuid_t uuid;

	src = btd_adapter_get_address(device_get_adapter(device->dev));
	dst = device_get_address(device->dev);

	dcpsm_data = g_new0(struct get_dcpsm_data, 1);
	dcpsm_data->func = func;
	dcpsm_data->data = data;
	dcpsm_data->destroy = destroy;

	bt_string2uuid(&uuid, HDP_UUID);
	if (bt_search_service(src, dst, &uuid, get_dcpsm_cb, dcpsm_data,
						free_dcpsm_data, 0) < 0) {
		g_set_error(err, HDP_ERROR, HDP_CONNECTION_ERROR,
						"Can't get remote SDP record");
		g_free(dcpsm_data);
		return FALSE;
	}

	return TRUE;
}

static void hdp_free_application(struct hdp_application *app)
{
	if (app->dbus_watcher > 0)
		g_dbus_remove_watch(btd_get_dbus_connection(),
							app->dbus_watcher);

	g_free(app->oname);
	g_free(app->description);
	g_free(app->path);
	g_free(app);
}

struct hdp_application *hdp_application_ref(struct hdp_application *app)
{
	if (app == NULL)
		return NULL;

	app->ref++;

	DBG("health_application_ref(%p): ref=%d", app, app->ref);
	return app;
}

void hdp_application_unref(struct hdp_application *app)
{
	if (app == NULL)
		return;

	app->ref--;

	DBG("health_application_unref(%p): ref=%d", app, app->ref);
	if (app->ref > 0)
		return;

	hdp_free_application(app);
}
