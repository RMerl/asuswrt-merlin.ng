/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2014  Intel Corporation. All rights reserved.
 *  Copyright (C) 2010 GSyC/LibreSoft, Universidad Rey Juan Carlos.
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

#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include <glib.h>

#include "btio/btio.h"
#include "lib/bluetooth.h"
#include "lib/sdp.h"
#include "lib/sdp_lib.h"
#include "lib/uuid.h"
#include "lib/l2cap.h"
#include "src/log.h"
#include "src/shared/util.h"
#include "src/shared/queue.h"
#include "src/uuid-helper.h"
#include "src/sdp-client.h"
#include "profiles/health/mcap.h"

#include "hal-msg.h"
#include "ipc-common.h"
#include "ipc.h"
#include "utils.h"
#include "bluetooth.h"
#include "health.h"

#define SVC_HINT_HEALTH			0x00
#define HDP_VERSION			0x0101
#define DATA_EXCHANGE_SPEC_11073	0x01

#define CHANNEL_TYPE_ANY       0x00
#define CHANNEL_TYPE_RELIABLE  0x01
#define CHANNEL_TYPE_STREAM    0x02

#define MDEP_ECHO		0x00
#define MDEP_INITIAL		0x01
#define MDEP_FINAL		0x7F

static bdaddr_t adapter_addr;
static struct ipc *hal_ipc = NULL;
static struct queue *apps = NULL;
static struct mcap_instance *mcap = NULL;
static uint32_t record_id = 0;
static uint32_t record_state = 0;

struct mdep_cfg {
	uint8_t role;
	uint16_t data_type;
	uint8_t channel_type;
	char *descr;

	uint8_t id; /* mdep id */
};

struct health_device {
	bdaddr_t dst;
	uint16_t app_id;

	struct mcap_mcl *mcl;

	struct queue *channels;     /* data channels */

	uint16_t ccpsm;
	uint16_t dcpsm;
};

struct health_channel {
	uint8_t mdep_id;
	uint8_t type;

	struct health_device *dev;

	uint8_t remote_mdep;
	struct mcap_mdl *mdl;
	bool mdl_conn;
	uint16_t mdl_id; /* MDL ID */

	uint16_t id; /* channel id */
};

struct health_app {
	char *app_name;
	char *provider_name;
	char *service_name;
	char *service_descr;
	uint8_t num_of_mdep;
	struct queue *mdeps;

	uint16_t id; /* app id */
	struct queue *devices;
};

static void send_app_reg_notify(struct health_app *app, uint8_t state)
{
	struct hal_ev_health_app_reg_state ev;

	DBG("");

	ev.id = app->id;
	ev.state = state;

	ipc_send_notif(hal_ipc, HAL_SERVICE_ID_HEALTH,
				HAL_EV_HEALTH_APP_REG_STATE, sizeof(ev), &ev);
}

static void send_channel_state_notify(struct health_channel *channel,
						uint8_t state, int fd)
{
	struct hal_ev_health_channel_state ev;

	DBG("");

	bdaddr2android(&channel->dev->dst, ev.bdaddr);
	ev.app_id = channel->dev->app_id;
	ev.mdep_index = channel->mdep_id - 1;
	ev.channel_id = channel->id;
	ev.channel_state = state;

	ipc_send_notif_with_fd(hal_ipc, HAL_SERVICE_ID_HEALTH,
					HAL_EV_HEALTH_CHANNEL_STATE,
					sizeof(ev), &ev, fd);
}

static void unref_mdl(struct health_channel *channel)
{
	if (!channel || !channel->mdl)
		return;

	mcap_mdl_unref(channel->mdl);
	channel->mdl = NULL;
	channel->mdl_conn = false;
}

static void free_health_channel(void *data)
{
	struct health_channel *channel = data;
	int fd;

	DBG("channel %p", channel);

	if (!channel)
		return;

	fd = mcap_mdl_get_fd(channel->mdl);
	if (fd >= 0)
		shutdown(fd, SHUT_RDWR);

	unref_mdl(channel);
	free(channel);
}

static void destroy_channel(void *data)
{
	struct health_channel *channel = data;

	if (!channel)
		return;

	send_channel_state_notify(channel, HAL_HEALTH_CHANNEL_DESTROYED, -1);
	queue_remove(channel->dev->channels, channel);
	free_health_channel(channel);
}

static void unref_mcl(struct health_device *dev)
{
	if (!dev || !dev->mcl)
		return;

	mcap_close_mcl(dev->mcl, FALSE);
	mcap_mcl_unref(dev->mcl);
	dev->mcl = NULL;
}

static void free_health_device(void *data)
{
	struct health_device *dev = data;

	if (!dev)
		return;

	unref_mcl(dev);
	queue_destroy(dev->channels, free_health_channel);
	free(dev);
}

static void free_mdep_cfg(void *data)
{
	struct mdep_cfg *cfg = data;

	if (!cfg)
		return;

	free(cfg->descr);
	free(cfg);
}

static void free_health_app(void *data)
{
	struct health_app *app = data;

	if (!app)
		return;

	free(app->app_name);
	free(app->provider_name);
	free(app->service_name);
	free(app->service_descr);
	queue_destroy(app->mdeps, free_mdep_cfg);
	queue_destroy(app->devices, free_health_device);
	free(app);
}

static bool match_channel_by_mdl(const void *data, const void *user_data)
{
	const struct health_channel *channel = data;
	const struct mcap_mdl *mdl = user_data;

	return channel->mdl == mdl;
}

static bool match_channel_by_id(const void *data, const void *user_data)
{
	const struct health_channel *channel = data;
	uint16_t channel_id = PTR_TO_INT(user_data);

	return channel->id == channel_id;
}

static bool match_dev_by_mcl(const void *data, const void *user_data)
{
	const struct health_device *dev = data;
	const struct mcap_mcl *mcl = user_data;

	return dev->mcl == mcl;
}

static bool match_dev_by_addr(const void *data, const void *user_data)
{
	const struct health_device *dev = data;
	const bdaddr_t *addr = user_data;

	return !bacmp(&dev->dst, addr);
}

static bool match_channel_by_mdep_id(const void *data, const void *user_data)
{
	const struct health_channel *channel = data;
	uint16_t mdep_id = PTR_TO_INT(user_data);

	return channel->mdep_id == mdep_id;
}

static bool match_mdep_by_role(const void *data, const void *user_data)
{
	const struct mdep_cfg *mdep = data;
	uint16_t role = PTR_TO_INT(user_data);

	return mdep->role == role;
}

static bool match_mdep_by_id(const void *data, const void *user_data)
{
	const struct mdep_cfg *mdep = data;
	uint16_t mdep_id = PTR_TO_INT(user_data);

	return mdep->id == mdep_id;
}

static bool match_app_by_id(const void *data, const void *user_data)
{
	const struct health_app *app = data;
	uint16_t app_id = PTR_TO_INT(user_data);

	return app->id == app_id;
}

static struct health_channel *search_channel_by_id(uint16_t id)
{
	const struct queue_entry *apps_entry, *devices_entry;
	struct health_app *app;
	struct health_channel *channel;
	struct health_device *dev;

	DBG("");

	apps_entry = queue_get_entries(apps);
	while (apps_entry) {
		app = apps_entry->data;
		devices_entry = queue_get_entries(app->devices);
		while (devices_entry) {
			dev = devices_entry->data;
			channel = queue_find(dev->channels, match_channel_by_id,
								INT_TO_PTR(id));

			if (channel)
				return channel;

			devices_entry = devices_entry->next;
		}

		apps_entry = apps_entry->next;
	}

	return NULL;
}

static struct health_channel *search_channel_by_mdl(struct mcap_mdl *mdl)
{
	const struct queue_entry *apps_entry, *devices_entry;
	struct health_app *app;
	struct health_channel *channel;
	struct health_device *dev;

	DBG("");

	apps_entry = queue_get_entries(apps);
	while (apps_entry) {
		app = apps_entry->data;
		devices_entry = queue_get_entries(app->devices);
		while (devices_entry) {
			dev = devices_entry->data;
			channel = queue_find(dev->channels,
						match_channel_by_mdl, mdl);

			if (channel)
				return channel;

			devices_entry = devices_entry->next;
		}

		apps_entry = apps_entry->next;
	}

	return NULL;
}

static struct health_device *search_dev_by_mcl(struct mcap_mcl *mcl)
{
	const struct queue_entry *apps_entry;
	struct health_app *app;
	struct health_device *dev;

	DBG("");

	apps_entry = queue_get_entries(apps);
	while (apps_entry) {
		app = apps_entry->data;

		dev = queue_find(app->devices, match_dev_by_mcl, mcl);

		if (dev)
			return dev;

		apps_entry = apps_entry->next;
	}

	return NULL;
}

static struct health_app *search_app_by_mdepid(uint8_t mdepid)
{
	const struct queue_entry *apps_entry;
	struct health_app *app;

	DBG("");

	apps_entry = queue_get_entries(apps);
	while (apps_entry) {
		app = apps_entry->data;

		if (queue_find(app->mdeps, match_mdep_by_id,
							INT_TO_PTR(mdepid)))
			return app;

		apps_entry = apps_entry->next;
	}

	return NULL;
}

static int register_service_protocols(sdp_record_t *rec,
					struct health_app *app)
{
	uuid_t l2cap_uuid, mcap_c_uuid;
	sdp_list_t *l2cap_list, *proto_list = NULL, *mcap_list = NULL;
	sdp_list_t *access_proto_list = NULL;
	sdp_data_t *psm = NULL, *mcap_ver = NULL;
	uint32_t ccpsm;
	uint16_t version = MCAP_VERSION;
	GError *err = NULL;
	int ret = -1;

	DBG("");

	/* set l2cap information */
	sdp_uuid16_create(&l2cap_uuid, L2CAP_UUID);
	l2cap_list = sdp_list_append(NULL, &l2cap_uuid);
	if (!l2cap_list)
		goto fail;

	ccpsm = mcap_get_ctrl_psm(mcap, &err);
	if (err)
		goto fail;

	psm = sdp_data_alloc(SDP_UINT16, &ccpsm);
	if (!psm)
		goto fail;

	if (!sdp_list_append(l2cap_list, psm))
		goto fail;

	proto_list = sdp_list_append(NULL, l2cap_list);
	if (!proto_list)
		goto fail;

	/* set mcap information */
	sdp_uuid16_create(&mcap_c_uuid, MCAP_CTRL_UUID);
	mcap_list = sdp_list_append(NULL, &mcap_c_uuid);
	if (!mcap_list)
		goto fail;

	mcap_ver = sdp_data_alloc(SDP_UINT16, &version);
	if (!mcap_ver)
		goto fail;

	if (!sdp_list_append(mcap_list, mcap_ver))
		goto fail;

	if (!sdp_list_append(proto_list, mcap_list))
		goto fail;

	/* attach protocol information to service record */
	access_proto_list = sdp_list_append(NULL, proto_list);
	if (!access_proto_list)
		goto fail;

	sdp_set_access_protos(rec, access_proto_list);
	ret = 0;

fail:
	sdp_list_free(l2cap_list, NULL);
	sdp_list_free(mcap_list, NULL);
	sdp_list_free(proto_list, NULL);
	sdp_list_free(access_proto_list, NULL);

	if (psm)
		sdp_data_free(psm);

	if (mcap_ver)
		sdp_data_free(mcap_ver);

	if (err)
		g_error_free(err);

	return ret;
}

static int register_service_profiles(sdp_record_t *rec)
{
	int ret;
	sdp_list_t *profile_list;
	sdp_profile_desc_t hdp_profile;

	DBG("");

	/* set hdp information */
	sdp_uuid16_create(&hdp_profile.uuid, HDP_SVCLASS_ID);
	hdp_profile.version = HDP_VERSION;
	profile_list = sdp_list_append(NULL, &hdp_profile);
	if (!profile_list)
		return -1;

	/* set profile descriptor list */
	ret = sdp_set_profile_descs(rec, profile_list);
	sdp_list_free(profile_list, NULL);

	return ret;
}

static int register_service_additional_protocols(sdp_record_t *rec,
						struct health_app *app)
{
	int ret = -1;
	uuid_t l2cap_uuid, mcap_d_uuid;
	sdp_list_t *l2cap_list, *proto_list = NULL, *mcap_list = NULL;
	sdp_list_t *access_proto_list = NULL;
	sdp_data_t *psm = NULL;
	uint32_t dcpsm;
	GError *err = NULL;

	DBG("");

	/* set l2cap information */
	sdp_uuid16_create(&l2cap_uuid, L2CAP_UUID);
	l2cap_list = sdp_list_append(NULL, &l2cap_uuid);
	if (!l2cap_list)
		goto fail;

	dcpsm = mcap_get_data_psm(mcap, &err);
	if (err)
		goto fail;

	psm = sdp_data_alloc(SDP_UINT16, &dcpsm);
	if (!psm)
		goto fail;

	if (!sdp_list_append(l2cap_list, psm))
		goto fail;

	proto_list = sdp_list_append(NULL, l2cap_list);
	if (!proto_list)
		goto fail;

	/* set mcap information */
	sdp_uuid16_create(&mcap_d_uuid, MCAP_DATA_UUID);
	mcap_list = sdp_list_append(NULL, &mcap_d_uuid);
	if (!mcap_list)
		goto fail;

	if (!sdp_list_append(proto_list, mcap_list))
		goto fail;

	/* attach protocol information to service record */
	access_proto_list = sdp_list_append(NULL, proto_list);
	if (!access_proto_list)
		goto fail;

	sdp_set_add_access_protos(rec, access_proto_list);
	ret = 0;

fail:
	sdp_list_free(l2cap_list, NULL);
	sdp_list_free(mcap_list, NULL);
	sdp_list_free(proto_list, NULL);
	sdp_list_free(access_proto_list, NULL);

	if (psm)
		sdp_data_free(psm);

	if (err)
		g_error_free(err);

	return ret;
}

static sdp_list_t *mdeps_to_sdp_features(struct mdep_cfg *mdep)
{
	sdp_data_t *mdepid, *dtype = NULL, *role = NULL, *descr = NULL;
	sdp_list_t *f_list = NULL;

	DBG("");

	mdepid = sdp_data_alloc(SDP_UINT8, &mdep->id);
	if (!mdepid)
		return NULL;

	dtype = sdp_data_alloc(SDP_UINT16, &mdep->data_type);
	if (!dtype)
		goto fail;

	role = sdp_data_alloc(SDP_UINT8, &mdep->role);
	if (!role)
		goto fail;

	if (mdep->descr) {
		descr = sdp_data_alloc(SDP_TEXT_STR8, mdep->descr);
		if (!descr)
			goto fail;
	}

	f_list = sdp_list_append(NULL, mdepid);
	if (!f_list)
		goto fail;

	if (!sdp_list_append(f_list, dtype))
		goto fail;

	if (!sdp_list_append(f_list, role))
		goto fail;

	if (descr && !sdp_list_append(f_list, descr))
		goto fail;

	return f_list;

fail:
	sdp_list_free(f_list, NULL);

	if (mdepid)
		sdp_data_free(mdepid);

	if (dtype)
		sdp_data_free(dtype);

	if (role)
		sdp_data_free(role);

	if (descr)
		sdp_data_free(descr);

	return NULL;
}

static void free_hdp_list(void *list)
{
	sdp_list_t *hdp_list = list;

	sdp_list_free(hdp_list, (sdp_free_func_t)sdp_data_free);
}

static void register_features(void *data, void *user_data)
{
	struct mdep_cfg *mdep = data;
	sdp_list_t **sup_features = user_data;
	sdp_list_t *hdp_feature;

	DBG("");

	hdp_feature = mdeps_to_sdp_features(mdep);
	if (!hdp_feature)
		return;

	if (!*sup_features) {
		*sup_features = sdp_list_append(NULL, hdp_feature);
		if (!*sup_features)
			sdp_list_free(hdp_feature,
					(sdp_free_func_t)sdp_data_free);
	} else if (!sdp_list_append(*sup_features, hdp_feature)) {
		sdp_list_free(hdp_feature,
					(sdp_free_func_t)sdp_data_free);
	}
}

static int register_service_sup_features(sdp_record_t *rec,
						struct health_app *app)
{
	sdp_list_t *sup_features = NULL;

	DBG("");

	queue_foreach(app->mdeps, register_features, &sup_features);
	if (!sup_features)
		return -1;

	if (sdp_set_supp_feat(rec, sup_features) < 0) {
		sdp_list_free(sup_features, free_hdp_list);
		return -1;
	}

	sdp_list_free(sup_features, free_hdp_list);
	return 0;
}

static int register_data_exchange_spec(sdp_record_t *rec)
{
	sdp_data_t *spec;
	uint8_t data_spec = DATA_EXCHANGE_SPEC_11073;
	/* As of now only 11073 is supported, so we set it as default */

	DBG("");

	spec = sdp_data_alloc(SDP_UINT8, &data_spec);
	if (!spec)
		return -1;

	if (sdp_attr_add(rec, SDP_ATTR_DATA_EXCHANGE_SPEC, spec) < 0) {
		sdp_data_free(spec);
		return -1;
	}

	return 0;
}

static int register_mcap_features(sdp_record_t *rec)
{
	sdp_data_t *mcap_proc;
	uint8_t mcap_sup_proc = MCAP_SUP_PROC;

	DBG("");

	mcap_proc = sdp_data_alloc(SDP_UINT8, &mcap_sup_proc);
	if (!mcap_proc)
		return -1;

	if (sdp_attr_add(rec, SDP_ATTR_MCAP_SUPPORTED_PROCEDURES,
							mcap_proc) < 0) {
		sdp_data_free(mcap_proc);
		return -1;
	}

	return 0;
}

static int set_sdp_services_uuid(sdp_record_t *rec, uint8_t role)
{
	uuid_t source, sink;
	sdp_list_t *list = NULL;

	sdp_uuid16_create(&sink, HDP_SINK_SVCLASS_ID);
	sdp_uuid16_create(&source, HDP_SOURCE_SVCLASS_ID);
	sdp_get_service_classes(rec, &list);

	switch (role) {
	case HAL_HEALTH_MDEP_ROLE_SOURCE:
		if (!sdp_list_find(list, &source, sdp_uuid_cmp))
			list = sdp_list_append(list, &source);
		break;
	case HAL_HEALTH_MDEP_ROLE_SINK:
		if (!sdp_list_find(list, &sink, sdp_uuid_cmp))
			list = sdp_list_append(list, &sink);
		break;
	}

	if (sdp_set_service_classes(rec, list) < 0) {
		sdp_list_free(list, NULL);
		return -1;
	}

	sdp_list_free(list, NULL);

	return 0;
}

static int update_sdp_record(struct health_app *app)
{
	sdp_record_t *rec;
	uint8_t role;

	DBG("");

	if (record_id > 0) {
		bt_adapter_remove_record(record_id);
		record_id = 0;
	}

	rec = sdp_record_alloc();
	if (!rec)
		return -1;

	role = HAL_HEALTH_MDEP_ROLE_SOURCE;
	if (queue_find(app->mdeps, match_mdep_by_role, INT_TO_PTR(role)))
		set_sdp_services_uuid(rec, role);

	role = HAL_HEALTH_MDEP_ROLE_SINK;
	if (queue_find(app->mdeps, match_mdep_by_role, INT_TO_PTR(role)))
		set_sdp_services_uuid(rec, role);

	sdp_set_info_attr(rec, app->service_name, app->provider_name,
							app->service_descr);

	if (register_service_protocols(rec, app) < 0)
		goto fail;

	if (register_service_profiles(rec) < 0)
		goto fail;

	if (register_service_additional_protocols(rec, app) < 0)
		goto fail;

	if (register_service_sup_features(rec, app) < 0)
		goto fail;

	if (register_data_exchange_spec(rec) < 0)
		goto fail;

	if (register_mcap_features(rec) < 0)
		goto fail;

	if (sdp_set_record_state(rec, record_state++) < 0)
		goto fail;

	if (bt_adapter_add_record(rec, SVC_HINT_HEALTH) < 0) {
		error("health: Failed to register HEALTH record");
		goto fail;
	}

	record_id = rec->handle;

	return 0;

fail:
	sdp_record_free(rec);

	return -1;
}

static struct health_app *create_health_app(const char *app_name,
				const char *provider, const char *srv_name,
				const char *srv_descr, uint8_t mdeps)
{
	struct health_app *app;
	static unsigned int app_id = 1;

	DBG("");

	app = new0(struct health_app, 1);
	app->id = app_id++;
	app->num_of_mdep = mdeps;
	app->app_name = strdup(app_name);

	if (provider) {
		app->provider_name = strdup(provider);
		if (!app->provider_name)
			goto fail;
	}

	if (srv_name) {
		app->service_name = strdup(srv_name);
		if (!app->service_name)
			goto fail;
	}

	if (srv_descr) {
		app->service_descr = strdup(srv_descr);
		if (!app->service_descr)
			goto fail;
	}

	app->mdeps = queue_new();
	app->devices = queue_new();

	return app;

fail:
	free_health_app(app);
	return NULL;
}

static void bt_health_register_app(const void *buf, uint16_t len)
{
	const struct hal_cmd_health_reg_app *cmd = buf;
	struct hal_rsp_health_reg_app rsp;
	struct health_app *app;
	uint16_t off;
	uint16_t app_name_len, provider_len, srv_name_len, srv_descr_len;
	char *app_name, *provider = NULL, *srv_name = NULL, *srv_descr = NULL;

	DBG("");

	if (len != sizeof(*cmd) + cmd->len ||
			cmd->app_name_off > cmd->provider_name_off ||
			cmd->provider_name_off > cmd->service_name_off ||
			cmd->service_name_off > cmd->service_descr_off ||
			cmd->service_descr_off > cmd->len) {
		error("health: Invalid register app command, terminating");
		raise(SIGTERM);
		return;
	}

	app_name = (char *) cmd->data;
	app_name_len = cmd->provider_name_off - cmd->app_name_off;

	off = app_name_len;
	provider_len = cmd->service_name_off - off;
	if (provider_len > 0)
		provider = (char *) cmd->data + off;

	off += provider_len;
	srv_name_len = cmd->service_descr_off - off;
	if (srv_name_len > 0)
		srv_name = (char *) cmd->data + off;

	off += srv_name_len;
	srv_descr_len = cmd->len - off;
	if (srv_descr_len > 0)
		srv_descr = (char *) cmd->data + off;

	app = create_health_app(app_name, provider, srv_name, srv_descr,
							cmd->num_of_mdep);
	if (!app)
		goto fail;

	queue_push_tail(apps, app);

	rsp.app_id = app->id;
	ipc_send_rsp_full(hal_ipc, HAL_SERVICE_ID_HEALTH, HAL_OP_HEALTH_REG_APP,
							sizeof(rsp), &rsp, -1);
	return;

fail:
	free_health_app(app);
	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_HEALTH, HAL_OP_HEALTH_MDEP,
							HAL_STATUS_FAILED);
}

static uint8_t android2channel_type(uint8_t type)
{
	switch (type) {
	case HAL_HEALTH_CHANNEL_TYPE_RELIABLE:
		return CHANNEL_TYPE_RELIABLE;
	case HAL_HEALTH_CHANNEL_TYPE_STREAMING:
		return CHANNEL_TYPE_STREAM;
	default:
		return CHANNEL_TYPE_ANY;
	}
}

static void bt_health_mdep_cfg_data(const void *buf, uint16_t len)
{
	const struct hal_cmd_health_mdep *cmd = buf;
	struct health_app *app;
	struct mdep_cfg *mdep = NULL;
	uint8_t status;

	DBG("");

	app = queue_find(apps, match_app_by_id, INT_TO_PTR(cmd->app_id));
	if (!app) {
		status = HAL_STATUS_INVALID;
		goto fail;
	}

	mdep = new0(struct mdep_cfg, 1);
	mdep->role = cmd->role;
	mdep->data_type = cmd->data_type;
	mdep->channel_type = android2channel_type(cmd->channel_type);
	mdep->id = queue_length(app->mdeps) + 1;

	if (cmd->descr_len > 0) {
		mdep->descr = malloc0(cmd->descr_len);
		memcpy(mdep->descr, cmd->descr, cmd->descr_len);
	}

	queue_push_tail(app->mdeps, mdep);

	if (app->num_of_mdep != queue_length(app->mdeps))
		goto send_rsp;

	/* add sdp record from app configuration data */
	/*
	 * TODO: Check what to be done if mupltple applications are trying to
	 * register with different role and different configurations.
	 * 1) Does device supports SOURCE and SINK at the same time ?
	 * 2) Does it require different SDP records or one record with
	 *    multile MDEP configurations ?
	 */
	if (update_sdp_record(app) < 0) {
		error("health: HDP SDP record preparation failed");
		status = HAL_STATUS_FAILED;
		goto fail;
	}

	send_app_reg_notify(app, HAL_HEALTH_APP_REG_SUCCESS);

send_rsp:
	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_HEALTH, HAL_OP_HEALTH_MDEP,
							HAL_STATUS_SUCCESS);
	return;

fail:
	if (status != HAL_STATUS_SUCCESS) {
		free_mdep_cfg(mdep);
		queue_remove(apps, app);
		free_health_app(app);
	}

	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_HEALTH, HAL_OP_HEALTH_MDEP,
								status);
}

static void bt_health_unregister_app(const void *buf, uint16_t len)
{
	const struct hal_cmd_health_unreg_app *cmd = buf;
	struct health_app *app;

	DBG("");

	app = queue_remove_if(apps, match_app_by_id, INT_TO_PTR(cmd->app_id));
	if (!app) {
		ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_HEALTH,
				HAL_OP_HEALTH_UNREG_APP, HAL_STATUS_INVALID);
		return;
	}

	send_app_reg_notify(app, HAL_HEALTH_APP_DEREG_SUCCESS);

	if (record_id > 0) {
		bt_adapter_remove_record(record_id);
		record_id = 0;
	}

	free_health_app(app);
	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_HEALTH,
				HAL_OP_HEALTH_UNREG_APP, HAL_STATUS_SUCCESS);
}

static int get_prot_desc_entry(sdp_data_t *entry, int type, guint16 *val)
{
	sdp_data_t *iter;
	int proto;

	if (!entry || !SDP_IS_SEQ(entry->dtd))
		return -1;

	iter = entry->val.dataseq;
	if (!(iter->dtd & SDP_UUID_UNSPEC))
		return -1;

	proto = sdp_uuid_to_proto(&iter->val.uuid);
	if (proto != type)
		return -1;

	if (!val)
		return 0;

	iter = iter->next;
	if (iter->dtd != SDP_UINT16)
		return -1;

	*val = iter->val.uint16;

	return 0;
}

static int get_prot_desc_list(const sdp_record_t *rec, uint16_t *psm,
							uint16_t *version)
{
	sdp_data_t *pdl, *p0, *p1;

	if (!psm && !version)
		return -1;

	pdl = sdp_data_get(rec, SDP_ATTR_PROTO_DESC_LIST);
	if (!pdl || !SDP_IS_SEQ(pdl->dtd))
		return -1;

	p0 = pdl->val.dataseq;
	if (get_prot_desc_entry(p0, L2CAP_UUID, psm) < 0)
		return -1;

	p1 = p0->next;
	if (get_prot_desc_entry(p1, MCAP_CTRL_UUID, version) < 0)
		return -1;

	return 0;
}

static int get_ccpsm(sdp_list_t *recs, uint16_t *ccpsm)
{
	sdp_list_t *l;

	for (l = recs; l; l = l->next) {
		sdp_record_t *rec = l->data;

		if (!get_prot_desc_list(rec, ccpsm, NULL))
			return 0;
	}

	return -1;
}

static int get_add_prot_desc_list(const sdp_record_t *rec, uint16_t *psm)
{
	sdp_data_t *pdl, *p0, *p1;

	if (!psm)
		return -1;

	pdl = sdp_data_get(rec, SDP_ATTR_ADD_PROTO_DESC_LIST);
	if (!pdl || pdl->dtd != SDP_SEQ8)
		return -1;

	pdl = pdl->val.dataseq;
	if (pdl->dtd != SDP_SEQ8)
		return -1;

	p0 = pdl->val.dataseq;

	if (get_prot_desc_entry(p0, L2CAP_UUID, psm) < 0)
		return -1;

	p1 = p0->next;
	if (get_prot_desc_entry(p1, MCAP_DATA_UUID, NULL) < 0)
		return -1;

	return 0;
}

static int get_dcpsm(sdp_list_t *recs, uint16_t *dcpsm)
{
	sdp_list_t *l;

	for (l = recs; l; l = l->next) {
		sdp_record_t *rec = l->data;

		if (!get_add_prot_desc_list(rec, dcpsm))
			return 0;
	}

	return -1;
}

static int send_echo_data(int sock, const void *buf, uint32_t size)
{
	const uint8_t *buf_b = buf;
	uint32_t sent = 0;

	while (sent < size) {
		int n = write(sock, buf_b + sent, size - sent);
		if (n < 0)
			return -1;
		sent += n;
	}

	return 0;
}

static gboolean serve_echo(GIOChannel *io, GIOCondition cond, gpointer data)
{
	struct health_channel *channel = data;
	uint8_t buf[MCAP_DC_MTU];
	int fd, len, ret;

	DBG("channel %p", channel);

	if (cond & (G_IO_ERR | G_IO_HUP | G_IO_NVAL)) {
		DBG("Error condition on channel");
		return FALSE;
	}

	fd = g_io_channel_unix_get_fd(io);

	len = read(fd, buf, sizeof(buf));
	if (len < 0) {
		DBG("Error reading ECHO");
		return FALSE;
	}

	ret = send_echo_data(fd, buf, len);
	if (ret != len)
		DBG("Error sending ECHO back");

	return FALSE;
}

static void mcap_mdl_connected_cb(struct mcap_mdl *mdl, void *data)
{
	struct health_channel *channel = data;
	int fd;

	DBG("Data channel connected: mdl %p channel %p", mdl, channel);

	if (!channel) {
		channel = search_channel_by_mdl(mdl);
		if (!channel) {
			error("health: channel data does not exist");
			return;
		}
	}

	if (!channel->mdl)
		channel->mdl = mcap_mdl_ref(mdl);

	fd = mcap_mdl_get_fd(channel->mdl);
	if (fd < 0) {
		error("health: error retrieving fd");
		goto fail;
	}

	if (channel->mdep_id == MDEP_ECHO) {
		GIOChannel *io;

		io = g_io_channel_unix_new(fd);
		g_io_add_watch(io, G_IO_ERR | G_IO_HUP | G_IO_NVAL | G_IO_IN,
							serve_echo, channel);
		g_io_channel_unref(io);

		return;
	}

	info("health: MDL connected");
	send_channel_state_notify(channel, HAL_HEALTH_CHANNEL_CONNECTED, fd);

	return;
fail:
	/* TODO: mcap_mdl_abort */
	destroy_channel(channel);
}

static void mcap_mdl_closed_cb(struct mcap_mdl *mdl, void *data)
{
	struct health_channel *channel = data;

	info("health: MDL closed");

	if (!channel)
		return;

	channel->mdl_conn = false;
}

static void mcap_mdl_deleted_cb(struct mcap_mdl *mdl, void *data)
{
	struct health_channel *channel;

	info("health: MDL deleted");

	channel = search_channel_by_mdl(mdl);
	if (!channel)
		return;

	DBG("channel %p mdl %p", channel, mdl);
	destroy_channel(channel);
}

static void mcap_mdl_aborted_cb(struct mcap_mdl *mdl, void *data)
{
	DBG("Not Implemeneted");
}

static struct health_device *create_device(struct health_app *app,
							const uint8_t *addr)
{
	struct health_device *dev;

	/* create device and push it to devices queue */
	dev = new0(struct health_device, 1);

	android2bdaddr(addr, &dev->dst);
	dev->channels = queue_new();
	dev->app_id = app->id;

	queue_push_tail(app->devices, dev);

	return dev;
}

static struct health_device *get_device(struct health_app *app,
							const uint8_t *addr)
{
	struct health_device *dev;
	bdaddr_t bdaddr;

	android2bdaddr(addr, &bdaddr);
	dev = queue_find(app->devices, match_dev_by_addr, &bdaddr);
	if (dev)
		return dev;

	return create_device(app, addr);
}

static struct health_channel *create_channel(struct health_app *app,
						uint8_t mdep_index,
						struct health_device *dev)
{
	struct mdep_cfg *mdep;
	struct health_channel *channel;
	static unsigned int channel_id = 1;

	DBG("mdep %u", mdep_index);

	if (!dev || !app)
		return NULL;

	mdep = queue_find(app->mdeps, match_mdep_by_id, INT_TO_PTR(mdep_index));
	if (!mdep) {
		if (mdep_index == MDEP_ECHO) {
			mdep = new0(struct mdep_cfg, 1);

			/* Leave other configuration zeroes */
			mdep->id = MDEP_ECHO;

			queue_push_tail(app->mdeps, mdep);
		} else {
			return NULL;
		}
	}

	/* create channel and push it to device */
	channel = new0(struct health_channel, 1);
	channel->mdep_id = mdep->id;
	channel->type = mdep->channel_type;
	channel->id = channel_id++;
	channel->dev = dev;

	queue_push_tail(dev->channels, channel);

	return channel;
}

static struct health_channel *connect_channel(struct health_app *app,
							struct mcap_mcl *mcl,
							uint8_t mdepid)
{
	struct health_device *device;
	bdaddr_t addr;

	DBG("app %p mdepid %u", app, mdepid);

	mcap_mcl_get_addr(mcl, &addr);

	if (!app) {
		DBG("No app found for mdepid %u", mdepid);
		return NULL;
	}

	device = get_device(app, (uint8_t *) &addr);

	return create_channel(app, mdepid, device);
}

static uint8_t conf_to_l2cap(uint8_t conf)
{
	return conf == CHANNEL_TYPE_STREAM ? L2CAP_MODE_STREAMING :
								L2CAP_MODE_ERTM;
}

static uint8_t mcap_mdl_conn_req_cb(struct mcap_mcl *mcl, uint8_t mdepid,
				uint16_t mdlid, uint8_t *conf, void *data)
{
	GError *gerr = NULL;
	struct health_channel *channel;
	struct health_app *app;
	struct mdep_cfg *mdep;

	DBG("Data channel request: mdepid %u mdlid %u conf %u",
							mdepid, mdlid, *conf);

	if (mdepid == MDEP_ECHO)
		/* For echo service take last app */
		app = queue_peek_tail(apps);
	else
		app = search_app_by_mdepid(mdepid);

	if (!app)
		return MCAP_MDL_BUSY;

	channel = connect_channel(app, mcl, mdepid);
	if (!channel)
		return MCAP_MDL_BUSY;

	/* Channel is assigned here after creation */
	mcl->cb->user_data = channel;

	if (mdepid == MDEP_ECHO) {
		switch (*conf) {
		case CHANNEL_TYPE_ANY:
			*conf = CHANNEL_TYPE_RELIABLE;
			break;
		case CHANNEL_TYPE_RELIABLE:
			break;
		case CHANNEL_TYPE_STREAM:
			return MCAP_CONFIGURATION_REJECTED;
		default:
			/*
			 * Special case defined in HDP spec 3.4.
			 * When an invalid configuration is received we shall
			 * close the MCL when we are still processing the
			 * callback.
			 */
			/* TODO close device */
			return MCAP_CONFIGURATION_REJECTED; /* not processed */
		}

		if (!mcap_set_data_chan_mode(mcap, L2CAP_MODE_ERTM, &gerr)) {
			error("Error: %s", gerr->message);
			g_error_free(gerr);
			return MCAP_MDL_BUSY;
		}

		/* TODO: Create channel */

		return MCAP_SUCCESS;
	}

	mdep = queue_find(app->mdeps, match_mdep_by_id, INT_TO_PTR(mdepid));
	if (!mdep)
		return MCAP_MDL_BUSY;

	switch (*conf) {
	case CHANNEL_TYPE_ANY:
		if (mdep->role == HAL_HEALTH_MDEP_ROLE_SINK) {
			return MCAP_CONFIGURATION_REJECTED;
		} else {
			if (queue_length(channel->dev->channels) <= 1)
				*conf = CHANNEL_TYPE_RELIABLE;
			else
				*conf = CHANNEL_TYPE_STREAM;
		}
		break;
	case CHANNEL_TYPE_STREAM:
		if (mdep->role == HAL_HEALTH_MDEP_ROLE_SOURCE)
			return MCAP_CONFIGURATION_REJECTED;
		break;
	case CHANNEL_TYPE_RELIABLE:
		if (mdep->role == HAL_HEALTH_MDEP_ROLE_SOURCE)
			return MCAP_CONFIGURATION_REJECTED;
		break;
	default:
		/*
		 * Special case defined in HDP spec 3.4. When an invalid
		 * configuration is received we shall close the MCL when
		 * we are still processing the callback.
		 */
		/* TODO: close device */
		return MCAP_CONFIGURATION_REJECTED; /* not processed */
	}

	if (!mcap_set_data_chan_mode(mcap, conf_to_l2cap(*conf), &gerr)) {
		error("health: error setting L2CAP mode: %s", gerr->message);
		g_error_free(gerr);
		return MCAP_MDL_BUSY;
	}

	return MCAP_SUCCESS;
}

static uint8_t mcap_mdl_reconn_req_cb(struct mcap_mdl *mdl, void *data)
{
	struct health_channel *channel;
	GError *err = NULL;

	DBG("");

	channel = search_channel_by_mdl(mdl);
	if (!channel) {
		error("health: channel data does not exist");
		return MCAP_UNSPECIFIED_ERROR;
	}

	if (!mcap_set_data_chan_mode(mcap,
			conf_to_l2cap(channel->type), &err)) {
		error("health: %s", err->message);
		g_error_free(err);
		return MCAP_MDL_BUSY;
	}

	return MCAP_SUCCESS;
}

static void connect_mdl_cb(struct mcap_mdl *mdl, GError *gerr, gpointer data)
{
	struct health_channel *channel = data;
	int fd;

	DBG("");

	if (gerr) {
		error("health: error connecting to MDL %s", gerr->message);
		goto fail;
	}

	fd = mcap_mdl_get_fd(channel->mdl);
	if (fd < 0) {
		error("health: error retrieving fd");
		goto fail;
	}

	info("health: MDL connected");
	channel->mdl_conn = true;

	/* first data channel should be reliable data channel */
	if (!queue_length(channel->dev->channels))
		if (channel->type != CHANNEL_TYPE_RELIABLE)
			goto fail;

	send_channel_state_notify(channel, HAL_HEALTH_CHANNEL_CONNECTED, fd);

	return;

fail:
	/* TODO: mcap_mdl_abort */
	destroy_channel(channel);
}

static void reconnect_mdl_cb(struct mcap_mdl *mdl, GError *gerr, gpointer data)
{
	struct health_channel *channel = data;
	uint8_t mode;
	GError *err = NULL;

	DBG("");

	if (gerr) {
		error("health: error reconnecting to MDL %s", gerr->message);
		goto fail;
	}

	channel->mdl_id = mcap_mdl_get_mdlid(mdl);

	if (channel->type == CHANNEL_TYPE_RELIABLE)
		mode = L2CAP_MODE_ERTM;
	else
		mode = L2CAP_MODE_STREAMING;

	if (!mcap_connect_mdl(channel->mdl, mode, channel->dev->dcpsm,
						connect_mdl_cb, channel,
						NULL, &err)) {
		error("health: error connecting to mdl");
		g_error_free(err);
		goto fail;
	}

	return;

fail:
	/* TODO: mcap_mdl_abort */
	destroy_channel(channel);
}

static int reconnect_mdl(struct health_channel *channel)
{
	GError *gerr = NULL;

	DBG("");

	if (!channel)
		return -1;

	if (!mcap_reconnect_mdl(channel->mdl, reconnect_mdl_cb, channel,
								NULL, &gerr)){
		error("health: reconnect failed %s", gerr->message);
		destroy_channel(channel);
	}

	return 0;
}

static void create_mdl_cb(struct mcap_mdl *mdl, uint8_t type, GError *gerr,
								gpointer data)
{
	struct health_channel *channel = data;
	uint8_t mode;
	GError *err = NULL;

	DBG("");
	if (gerr) {
		error("health: error creating MDL %s", gerr->message);
		goto fail;
	}

	if (channel->type == CHANNEL_TYPE_ANY && type != CHANNEL_TYPE_ANY)
		channel->type = type;

	/*
	 * if requested channel type is not same as preferred
	 * channel type from remote device, then abort the connection.
	 */
	if (channel->type != type) {
		/* TODO: abort mdl */
		error("health: channel type requested %d preferred %d not same",
							channel->type, type);
		goto fail;
	}

	if (!channel->mdl)
		channel->mdl = mcap_mdl_ref(mdl);

	channel->type = type;
	channel->mdl_id = mcap_mdl_get_mdlid(mdl);

	if (channel->type == CHANNEL_TYPE_RELIABLE)
		mode = L2CAP_MODE_ERTM;
	else
		mode = L2CAP_MODE_STREAMING;

	if (!mcap_connect_mdl(channel->mdl, mode, channel->dev->dcpsm,
						connect_mdl_cb, channel,
						NULL, &err)) {
		error("health: error connecting to mdl");
		g_error_free(err);
		goto fail;
	}

	return;

fail:
	destroy_channel(channel);
}

static bool check_role(uint8_t rec_role, uint8_t app_role)
{
	if ((rec_role == HAL_HEALTH_MDEP_ROLE_SINK &&
			app_role == HAL_HEALTH_MDEP_ROLE_SOURCE) ||
			(rec_role == HAL_HEALTH_MDEP_ROLE_SOURCE &&
			app_role == HAL_HEALTH_MDEP_ROLE_SINK))
		return true;

	return false;
}

static bool get_mdep_from_rec(const sdp_record_t *rec, uint8_t role,
						uint16_t d_type, uint8_t *mdep)
{
	sdp_data_t *list, *feat;

	if (!mdep)
		return false;

	list = sdp_data_get(rec, SDP_ATTR_SUPPORTED_FEATURES_LIST);
	if (!list || !SDP_IS_SEQ(list->dtd))
		return false;

	for (feat = list->val.dataseq; feat; feat = feat->next) {
		sdp_data_t *data_type, *mdepid, *role_t;

		if (!SDP_IS_SEQ(feat->dtd))
			continue;

		mdepid = feat->val.dataseq;
		if (!mdepid)
			continue;

		data_type = mdepid->next;
		if (!data_type)
			continue;

		role_t = data_type->next;
		if (!role_t)
			continue;

		if (data_type->dtd != SDP_UINT16 || mdepid->dtd != SDP_UINT8 ||
						role_t->dtd != SDP_UINT8)
			continue;

		if (data_type->val.uint16 != d_type ||
					!check_role(role_t->val.uint8, role))
			continue;

		*mdep = mdepid->val.uint8;

		return true;
	}

	return false;
}

static bool get_remote_mdep(sdp_list_t *recs, struct health_channel *channel)
{
	struct health_app *app;
	struct mdep_cfg *mdep;
	uint8_t mdep_id;

	app = queue_find(apps, match_app_by_id,
					INT_TO_PTR(channel->dev->app_id));
	if (!app)
		return false;

	mdep = queue_find(app->mdeps, match_mdep_by_id,
						INT_TO_PTR(channel->mdep_id));
	if (!mdep)
		return false;

	if (!get_mdep_from_rec(recs->data, mdep->role, mdep->data_type,
								&mdep_id)) {
		error("health: no matching MDEP: %u", channel->mdep_id);
		return false;
	}

	channel->remote_mdep = mdep_id;
	return true;
}

static bool create_mdl(struct health_channel *channel)
{
	struct health_app *app;
	struct mdep_cfg *mdep;
	uint8_t type;
	GError *gerr = NULL;

	app = queue_find(apps, match_app_by_id,
					INT_TO_PTR(channel->dev->app_id));
	if (!app)
		return false;

	mdep = queue_find(app->mdeps, match_mdep_by_id,
						INT_TO_PTR(channel->mdep_id));
	if (!mdep)
		return false;

	if (mdep->role == HAL_HEALTH_MDEP_ROLE_SOURCE)
		type = channel->type;
	else
		type = CHANNEL_TYPE_ANY;

	if (!mcap_create_mdl(channel->dev->mcl, channel->remote_mdep,
				type, create_mdl_cb, channel, NULL, &gerr)) {
		error("health: error creating mdl %s", gerr->message);
		g_error_free(gerr);
		return false;
	}

	return true;
}

static bool set_mcl_cb(struct mcap_mcl *mcl, gpointer user_data, GError **err)
{
	return mcap_mcl_set_cb(mcl, user_data, err,
			MCAP_MDL_CB_CONNECTED, mcap_mdl_connected_cb,
			MCAP_MDL_CB_CLOSED, mcap_mdl_closed_cb,
			MCAP_MDL_CB_DELETED, mcap_mdl_deleted_cb,
			MCAP_MDL_CB_ABORTED, mcap_mdl_aborted_cb,
			MCAP_MDL_CB_REMOTE_CONN_REQ, mcap_mdl_conn_req_cb,
			MCAP_MDL_CB_REMOTE_RECONN_REQ, mcap_mdl_reconn_req_cb,
			MCAP_MDL_CB_INVALID);
}

static void create_mcl_cb(struct mcap_mcl *mcl, GError *err, gpointer data)
{
	struct health_channel *channel = data;
	gboolean ret;
	GError *gerr = NULL;

	DBG("");

	if (err) {
		error("health: error creating MCL : %s", err->message);
		goto fail;
	}

	if (!channel->dev->mcl)
		channel->dev->mcl = mcap_mcl_ref(mcl);

	info("health: MCL connected");

	ret = set_mcl_cb(channel->dev->mcl, channel, &gerr);
	if (!ret) {
		error("health: error setting mdl callbacks: %s", gerr->message);
		g_error_free(gerr);
		goto fail;
	}

	if (!create_mdl(channel))
		goto fail;

	return;

fail:
	destroy_channel(channel);
}

static void search_cb(sdp_list_t *recs, int err, gpointer data)
{
	struct health_channel *channel = data;
	GError *gerr = NULL;

	DBG("");

	if (err < 0 || !recs) {
		error("health: Error getting remote SDP records");
		goto fail;
	}

	if (get_ccpsm(recs, &channel->dev->ccpsm) < 0) {
		error("health: Can't get remote PSM for control channel");
		goto fail;
	}

	if (get_dcpsm(recs, &channel->dev->dcpsm) < 0) {
		error("health: Can't get remote PSM for data channel");
		goto fail;
	}

	if (!get_remote_mdep(recs, channel)) {
		error("health: Can't get remote MDEP data");
		goto fail;
	}

	if (!mcap_create_mcl(mcap, &channel->dev->dst, channel->dev->ccpsm,
					create_mcl_cb, channel, NULL, &gerr)) {
		error("health: error creating mcl %s", gerr->message);
		g_error_free(gerr);
		goto fail;
	}

	return;

fail:
	destroy_channel(channel);
}

static int connect_mcl(struct health_channel *channel)
{
	uuid_t uuid;
	int err;

	DBG("");

	bt_string2uuid(&uuid, HDP_UUID);

	err = bt_search_service(&adapter_addr, &channel->dev->dst, &uuid,
						search_cb, channel, NULL, 0);
	if (!err)
		send_channel_state_notify(channel,
					HAL_HEALTH_CHANNEL_CONNECTING, -1);

	return err;
}

static struct health_app *get_app(uint16_t app_id)
{
	return queue_find(apps, match_app_by_id, INT_TO_PTR(app_id));
}

static struct health_channel *get_channel(struct health_app *app,
						uint8_t mdep_index,
						struct health_device *dev)
{
	struct health_channel *channel;
	uint8_t index;

	if (!dev)
		return NULL;

	index = mdep_index + 1;
	channel = queue_find(dev->channels, match_channel_by_mdep_id,
							INT_TO_PTR(index));
	if (channel)
		return channel;

	return create_channel(app, index, dev);
}

static void bt_health_connect_channel(const void *buf, uint16_t len)
{
	const struct hal_cmd_health_connect_channel *cmd = buf;
	struct hal_rsp_health_connect_channel rsp;
	struct health_device *dev = NULL;
	struct health_channel *channel = NULL;
	struct health_app *app;

	DBG("");

	app = get_app(cmd->app_id);
	if (!app)
		goto send_rsp;

	dev = get_device(app, cmd->bdaddr);

	channel = get_channel(app, cmd->mdep_index, dev);
	if (!channel)
		goto send_rsp;

	if (!queue_length(dev->channels)) {
		if (channel->type != CHANNEL_TYPE_RELIABLE) {
			error("health: first data shannel should be reliable");
			goto fail;
		}
	}

	if (!dev->mcl) {
		if (connect_mcl(channel) < 0) {
			error("health: error retrieving HDP SDP record");
			goto fail;
		}
	} else {
		/* data channel is already connected */
		if (channel->mdl && channel->mdl_conn)
			goto fail;

		/* create mdl if it does not exists */
		if (!channel->mdl && !create_mdl(channel))
			goto fail;

		/* reconnect mdl if it exists */
		if (channel->mdl && !channel->mdl_conn) {
			if (reconnect_mdl(channel) < 0)
				goto fail;
		}

	}

	rsp.channel_id = channel->id;
	ipc_send_rsp_full(hal_ipc, HAL_SERVICE_ID_HEALTH,
				HAL_OP_HEALTH_CONNECT_CHANNEL,
				sizeof(rsp), &rsp, -1);
	return;

fail:
	queue_remove(channel->dev->channels, channel);
	free_health_channel(channel);

send_rsp:
	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_HEALTH,
			HAL_OP_HEALTH_CONNECT_CHANNEL, HAL_STATUS_FAILED);
}

static void channel_delete_cb(GError *gerr, gpointer data)
{
	struct health_channel *channel = data;

	DBG("");

	if (gerr) {
		error("health: channel delete failed %s", gerr->message);
		return;
	}

	destroy_channel(channel);
}

static void bt_health_destroy_channel(const void *buf, uint16_t len)
{
	const struct hal_cmd_health_destroy_channel *cmd = buf;
	struct health_channel *channel;
	GError *gerr = NULL;

	DBG("");

	channel = search_channel_by_id(cmd->channel_id);
	if (!channel)
		goto fail;

	if (!mcap_delete_mdl(channel->mdl, channel_delete_cb, channel,
							NULL, &gerr)) {
		error("health: channel delete failed %s", gerr->message);
		goto fail;
	}

	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_HEALTH,
			HAL_OP_HEALTH_DESTROY_CHANNEL, HAL_STATUS_SUCCESS);

	return;

fail:
	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_HEALTH,
			HAL_OP_HEALTH_DESTROY_CHANNEL, HAL_STATUS_INVALID);
}

static const struct ipc_handler cmd_handlers[] = {
	/* HAL_OP_HEALTH_REG_APP */
	{ bt_health_register_app, true,
				sizeof(struct hal_cmd_health_reg_app) },
	/* HAL_OP_HEALTH_MDEP */
	{ bt_health_mdep_cfg_data, true,
				sizeof(struct hal_cmd_health_mdep) },
	/* HAL_OP_HEALTH_UNREG_APP */
	{ bt_health_unregister_app, false,
				sizeof(struct hal_cmd_health_unreg_app) },
	/* HAL_OP_HEALTH_CONNECT_CHANNEL */
	{ bt_health_connect_channel, false,
				sizeof(struct hal_cmd_health_connect_channel) },
	/* HAL_OP_HEALTH_DESTROY_CHANNEL */
	{ bt_health_destroy_channel, false,
				sizeof(struct hal_cmd_health_destroy_channel) },
};

static void mcl_connected(struct mcap_mcl *mcl, gpointer data)
{
	GError *gerr = NULL;
	bool ret;

	DBG("");

	info("health: MCL connected");
	ret = set_mcl_cb(mcl, NULL, &gerr);
	if (!ret) {
		error("health: error setting mcl callbacks: %s", gerr->message);
		g_error_free(gerr);
	}
}

static void mcl_reconnected(struct mcap_mcl *mcl, gpointer data)
{
	struct health_device *dev;

	DBG("");

	info("health: MCL reconnected");
	dev = search_dev_by_mcl(mcl);
	if (!dev) {
		error("device data does not exists");
		return;
	}
}

static void mcl_disconnected(struct mcap_mcl *mcl, gpointer data)
{
	struct health_device *dev;

	DBG("");

	info("health: MCL disconnected");
	dev = search_dev_by_mcl(mcl);
	unref_mcl(dev);
}

static void mcl_uncached(struct mcap_mcl *mcl, gpointer data)
{
	/* mcap library maintains cache of mcls, not required here */
}

bool bt_health_register(struct ipc *ipc, const bdaddr_t *addr, uint8_t mode)
{
	GError *err = NULL;

	DBG("");

	bacpy(&adapter_addr, addr);

	mcap = mcap_create_instance(&adapter_addr, BT_IO_SEC_MEDIUM, 0, 0,
					mcl_connected, mcl_reconnected,
					mcl_disconnected, mcl_uncached,
					NULL, /* CSP is not used right now */
					NULL, &err);
	if (!mcap) {
		error("health: MCAP instance creation failed %s", err->message);
		g_error_free(err);
		return false;
	}

	hal_ipc = ipc;
	apps = queue_new();

	ipc_register(hal_ipc, HAL_SERVICE_ID_HEALTH, cmd_handlers,
						G_N_ELEMENTS(cmd_handlers));

	return true;
}

void bt_health_unregister(void)
{
	DBG("");

	mcap_instance_unref(mcap);
	queue_destroy(apps, free_health_app);
	ipc_unregister(hal_ipc, HAL_SERVICE_ID_HEALTH);
	hal_ipc = NULL;
}
