/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2006-2010  Nokia Corporation
 *  Copyright (C) 2004-2010  Marcel Holtmann <marcel@holtmann.org>
 *  Copyright (C) 2011  Texas Instruments, Inc.
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
#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include <assert.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <glib.h>
#include <dbus/dbus.h>

#include "bluetooth/bluetooth.h"
#include "bluetooth/sdp.h"
#include "bluetooth/sdp_lib.h"
#include "lib/uuid.h"

#include "gdbus/gdbus.h"

#include "src/plugin.h"
#include "src/adapter.h"
#include "src/device.h"
#include "src/profile.h"
#include "src/service.h"
#include "src/log.h"
#include "src/error.h"
#include "src/sdpd.h"
#include "src/dbus-common.h"
#include "src/shared/util.h"

#include "avctp.h"
#include "avrcp.h"
#include "control.h"
#include "player.h"
#include "transport.h"

/* Company IDs for vendor dependent commands */
#define IEEEID_BTSIG		0x001958

/* Status codes */
#define AVRCP_STATUS_INVALID_COMMAND		0x00
#define AVRCP_STATUS_INVALID_PARAM		0x01
#define AVRCP_STATUS_PARAM_NOT_FOUND		0x02
#define AVRCP_STATUS_INTERNAL_ERROR		0x03
#define AVRCP_STATUS_SUCCESS			0x04
#define AVRCP_STATUS_OUT_OF_BOUNDS		0x0b
#define AVRCP_STATUS_INVALID_PLAYER_ID		0x11
#define AVRCP_STATUS_PLAYER_NOT_BROWSABLE	0x12
#define AVRCP_STATUS_NO_AVAILABLE_PLAYERS	0x15
#define AVRCP_STATUS_ADDRESSED_PLAYER_CHANGED	0x16

/* Packet types */
#define AVRCP_PACKET_TYPE_SINGLE	0x00
#define AVRCP_PACKET_TYPE_START		0x01
#define AVRCP_PACKET_TYPE_CONTINUING	0x02
#define AVRCP_PACKET_TYPE_END		0x03

/* PDU types for metadata transfer */
#define AVRCP_GET_CAPABILITIES		0x10
#define AVRCP_LIST_PLAYER_ATTRIBUTES	0X11
#define AVRCP_LIST_PLAYER_VALUES	0x12
#define AVRCP_GET_CURRENT_PLAYER_VALUE	0x13
#define AVRCP_SET_PLAYER_VALUE		0x14
#define AVRCP_GET_PLAYER_ATTRIBUTE_TEXT	0x15
#define AVRCP_GET_PLAYER_VALUE_TEXT	0x16
#define AVRCP_DISPLAYABLE_CHARSET	0x17
#define AVRCP_CT_BATTERY_STATUS		0x18
#define AVRCP_GET_ELEMENT_ATTRIBUTES	0x20
#define AVRCP_GET_PLAY_STATUS		0x30
#define AVRCP_REGISTER_NOTIFICATION	0x31
#define AVRCP_REQUEST_CONTINUING	0x40
#define AVRCP_ABORT_CONTINUING		0x41
#define AVRCP_SET_ABSOLUTE_VOLUME	0x50
#define AVRCP_SET_ADDRESSED_PLAYER	0x60
#define AVRCP_SET_BROWSED_PLAYER	0x70
#define AVRCP_GET_FOLDER_ITEMS		0x71
#define AVRCP_CHANGE_PATH		0x72
#define AVRCP_GET_ITEM_ATTRIBUTES	0x73
#define AVRCP_PLAY_ITEM			0x74
#define AVRCP_GET_TOTAL_NUMBER_OF_ITEMS	0x75
#define AVRCP_SEARCH			0x80
#define AVRCP_ADD_TO_NOW_PLAYING	0x90
#define AVRCP_GENERAL_REJECT		0xA0

/* Capabilities for AVRCP_GET_CAPABILITIES pdu */
#define CAP_COMPANY_ID		0x02
#define CAP_EVENTS_SUPPORTED	0x03

#define AVRCP_REGISTER_NOTIFICATION_PARAM_LENGTH 5
#define AVRCP_GET_CAPABILITIES_PARAM_LENGTH 1

#define AVRCP_FEATURE_CATEGORY_1	0x0001
#define AVRCP_FEATURE_CATEGORY_2	0x0002
#define AVRCP_FEATURE_CATEGORY_3	0x0004
#define AVRCP_FEATURE_CATEGORY_4	0x0008
#define AVRCP_FEATURE_PLAYER_SETTINGS	0x0010
#define AVRCP_FEATURE_BROWSING			0x0040

#define AVRCP_BATTERY_STATUS_NORMAL		0
#define AVRCP_BATTERY_STATUS_WARNING		1
#define AVRCP_BATTERY_STATUS_CRITICAL		2
#define AVRCP_BATTERY_STATUS_EXTERNAL		3
#define AVRCP_BATTERY_STATUS_FULL_CHARGE	4

#define AVRCP_CHARSET_UTF8		106

#define AVRCP_BROWSING_TIMEOUT		1
#define AVRCP_CT_VERSION		0x0106
#define AVRCP_TG_VERSION		0x0105

#define AVRCP_SCOPE_MEDIA_PLAYER_LIST			0x00
#define AVRCP_SCOPE_MEDIA_PLAYER_VFS			0x01
#define AVRCP_SCOPE_SEARCH				0x02
#define AVRCP_SCOPE_NOW_PLAYING			0x03

#if __BYTE_ORDER == __LITTLE_ENDIAN

struct avrcp_header {
	uint8_t company_id[3];
	uint8_t pdu_id;
	uint8_t packet_type:2;
	uint8_t rsvd:6;
	uint16_t params_len;
	uint8_t params[0];
} __attribute__ ((packed));
#define AVRCP_HEADER_LENGTH 7

#elif __BYTE_ORDER == __BIG_ENDIAN

struct avrcp_header {
	uint8_t company_id[3];
	uint8_t pdu_id;
	uint8_t rsvd:6;
	uint8_t packet_type:2;
	uint16_t params_len;
	uint8_t params[0];
} __attribute__ ((packed));
#define AVRCP_HEADER_LENGTH 7

#else
#error "Unknown byte order"
#endif

#define AVRCP_MTU	(AVC_MTU - AVC_HEADER_LENGTH)
#define AVRCP_PDU_MTU	(AVRCP_MTU - AVRCP_HEADER_LENGTH)

struct avrcp_browsing_header {
	uint8_t pdu_id;
	uint16_t param_len;
	uint8_t params[0];
} __attribute__ ((packed));
#define AVRCP_BROWSING_HEADER_LENGTH 3

struct get_folder_items_rsp {
	uint8_t status;
	uint16_t uid_counter;
	uint16_t num_items;
	uint8_t data[0];
} __attribute__ ((packed));

struct folder_item {
	uint8_t type;
	uint16_t len;
	uint8_t data[0];
} __attribute__ ((packed));

struct player_item {
	uint16_t player_id;
	uint8_t type;
	uint32_t subtype;
	uint8_t status;
	uint8_t features[16];
	uint16_t charset;
	uint16_t namelen;
	char name[0];
} __attribute__ ((packed));

struct avrcp_server {
	struct btd_adapter *adapter;
	uint32_t tg_record_id;
	uint32_t ct_record_id;
	GSList *players;
	GSList *sessions;
};

struct pending_pdu {
	uint8_t pdu_id;
	GList *attr_ids;
	uint16_t offset;
};

struct pending_list_items {
	GSList *items;
	uint32_t start;
	uint32_t end;
	uint64_t total;
};

struct avrcp_player {
	struct avrcp_server *server;
	GSList *sessions;
	uint16_t id;
	uint8_t scope;
	uint64_t uid;
	uint16_t uid_counter;
	bool browsed;
	bool addressed;
	uint8_t *features;
	char *path;
	guint changed_id;

	struct pending_list_items *p;
	char *change_path;

	struct avrcp_player_cb *cb;
	void *user_data;
	GDestroyNotify destroy;
};

struct avrcp_data {
	struct avrcp_player *player;
	uint16_t version;
	int features;
	GSList *players;
};

struct avrcp {
	struct avrcp_server *server;
	struct avctp *conn;
	struct btd_device *dev;
	struct avrcp_data *target;
	struct avrcp_data *controller;

	const struct passthrough_handler *passthrough_handlers;
	const struct control_pdu_handler *control_handlers;

	unsigned int passthrough_id;
	unsigned int control_id;
	unsigned int browsing_id;
	unsigned int browsing_timer;
	uint16_t supported_events;
	uint16_t registered_events;
	uint8_t transaction;
	uint8_t transaction_events[AVRCP_EVENT_LAST + 1];
	struct pending_pdu *pending_pdu;
};

struct passthrough_handler {
	uint8_t op;
	bool (*func) (struct avrcp *session);
};

struct control_pdu_handler {
	uint8_t pdu_id;
	uint8_t code;
	uint8_t (*func) (struct avrcp *session, struct avrcp_header *pdu,
							uint8_t transaction);
};

static GSList *servers = NULL;
static unsigned int avctp_id = 0;

/* Default feature bit mask for media player as per avctp.c:key_map */
static const uint8_t features[16] = {
				0xF8, 0xBF, 0xFF, 0xBF, 0x1F,
				0xFB, 0x3F, 0x60, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00,
				0x00 };

/* Company IDs supported by this device */
static uint32_t company_ids[] = {
	IEEEID_BTSIG,
};

static void avrcp_register_notification(struct avrcp *session, uint8_t event);

static sdp_record_t *avrcp_ct_record(void)
{
	sdp_list_t *svclass_id, *pfseq, *apseq, *apseq1, *root;
	uuid_t root_uuid, l2cap, avctp, avrct, avrctr;
	sdp_profile_desc_t profile[1];
	sdp_list_t *aproto, *aproto1, *proto[2], *proto1[2];
	sdp_record_t *record;
	sdp_data_t *psm[2], *version, *features;
	uint16_t lp = AVCTP_CONTROL_PSM, ap = AVCTP_BROWSING_PSM;
	uint16_t avctp_ver = 0x0103;
	uint16_t feat = ( AVRCP_FEATURE_CATEGORY_1 |
						AVRCP_FEATURE_CATEGORY_2 |
						AVRCP_FEATURE_CATEGORY_3 |
						AVRCP_FEATURE_CATEGORY_4 |
						AVRCP_FEATURE_BROWSING);

	record = sdp_record_alloc();
	if (!record)
		return NULL;

	sdp_uuid16_create(&root_uuid, PUBLIC_BROWSE_GROUP);
	root = sdp_list_append(NULL, &root_uuid);
	sdp_set_browse_groups(record, root);

	/* Service Class ID List */
	sdp_uuid16_create(&avrct, AV_REMOTE_SVCLASS_ID);
	svclass_id = sdp_list_append(NULL, &avrct);
	sdp_uuid16_create(&avrctr, AV_REMOTE_CONTROLLER_SVCLASS_ID);
	svclass_id = sdp_list_append(svclass_id, &avrctr);
	sdp_set_service_classes(record, svclass_id);

	/* Protocol Descriptor List */
	sdp_uuid16_create(&l2cap, L2CAP_UUID);
	proto[0] = sdp_list_append(NULL, &l2cap);
	psm[0] = sdp_data_alloc(SDP_UINT16, &lp);
	proto[0] = sdp_list_append(proto[0], psm[0]);
	apseq = sdp_list_append(NULL, proto[0]);

	sdp_uuid16_create(&avctp, AVCTP_UUID);
	proto[1] = sdp_list_append(NULL, &avctp);
	version = sdp_data_alloc(SDP_UINT16, &avctp_ver);
	proto[1] = sdp_list_append(proto[1], version);
	apseq = sdp_list_append(apseq, proto[1]);

	aproto = sdp_list_append(NULL, apseq);
	sdp_set_access_protos(record, aproto);

	/* Additional Protocol Descriptor List */
	sdp_uuid16_create(&l2cap, L2CAP_UUID);
	proto1[0] = sdp_list_append(NULL, &l2cap);
	psm[1] = sdp_data_alloc(SDP_UINT16, &ap);
	proto1[0] = sdp_list_append(proto1[0], psm[1]);
	apseq1 = sdp_list_append(NULL, proto1[0]);

	sdp_uuid16_create(&avctp, AVCTP_UUID);
	proto1[1] = sdp_list_append(NULL, &avctp);
	proto1[1] = sdp_list_append(proto1[1], version);
	apseq1 = sdp_list_append(apseq1, proto1[1]);

	aproto1 = sdp_list_append(NULL, apseq1);
	sdp_set_add_access_protos(record, aproto1);

	/* Bluetooth Profile Descriptor List */
	sdp_uuid16_create(&profile[0].uuid, AV_REMOTE_PROFILE_ID);
	profile[0].version = AVRCP_CT_VERSION;
	pfseq = sdp_list_append(NULL, &profile[0]);
	sdp_set_profile_descs(record, pfseq);

	features = sdp_data_alloc(SDP_UINT16, &feat);
	sdp_attr_add(record, SDP_ATTR_SUPPORTED_FEATURES, features);

	sdp_set_info_attr(record, "AVRCP CT", NULL, NULL);

	free(psm[0]);
	free(psm[1]);
	free(version);
	sdp_list_free(proto[0], NULL);
	sdp_list_free(proto[1], NULL);
	sdp_list_free(apseq, NULL);
	sdp_list_free(proto1[0], NULL);
	sdp_list_free(proto1[1], NULL);
	sdp_list_free(aproto1, NULL);
	sdp_list_free(apseq1, NULL);
	sdp_list_free(pfseq, NULL);
	sdp_list_free(aproto, NULL);
	sdp_list_free(root, NULL);
	sdp_list_free(svclass_id, NULL);

	return record;
}

static sdp_record_t *avrcp_tg_record(void)
{
	sdp_list_t *svclass_id, *pfseq, *apseq, *root, *apseq_browsing;
	uuid_t root_uuid, l2cap, avctp, avrtg;
	sdp_profile_desc_t profile[1];
	sdp_list_t *aproto_control, *proto_control[2];
	sdp_record_t *record;
	sdp_data_t *psm_control, *version, *features, *psm_browsing;
	sdp_list_t *aproto_browsing, *proto_browsing[2] = {0};
	uint16_t lp = AVCTP_CONTROL_PSM;
	uint16_t lp_browsing = AVCTP_BROWSING_PSM;
	uint16_t avctp_ver = 0x0103;
	uint16_t feat = ( AVRCP_FEATURE_CATEGORY_1 |
					AVRCP_FEATURE_CATEGORY_2 |
					AVRCP_FEATURE_CATEGORY_3 |
					AVRCP_FEATURE_CATEGORY_4 |
					AVRCP_FEATURE_BROWSING |
					AVRCP_FEATURE_PLAYER_SETTINGS );

	record = sdp_record_alloc();
	if (!record)
		return NULL;

	sdp_uuid16_create(&root_uuid, PUBLIC_BROWSE_GROUP);
	root = sdp_list_append(NULL, &root_uuid);
	sdp_set_browse_groups(record, root);

	/* Service Class ID List */
	sdp_uuid16_create(&avrtg, AV_REMOTE_TARGET_SVCLASS_ID);
	svclass_id = sdp_list_append(NULL, &avrtg);
	sdp_set_service_classes(record, svclass_id);

	/* Protocol Descriptor List */
	sdp_uuid16_create(&l2cap, L2CAP_UUID);
	proto_control[0] = sdp_list_append(NULL, &l2cap);
	psm_control = sdp_data_alloc(SDP_UINT16, &lp);
	proto_control[0] = sdp_list_append(proto_control[0], psm_control);
	apseq = sdp_list_append(NULL, proto_control[0]);

	sdp_uuid16_create(&avctp, AVCTP_UUID);
	proto_control[1] = sdp_list_append(NULL, &avctp);
	version = sdp_data_alloc(SDP_UINT16, &avctp_ver);
	proto_control[1] = sdp_list_append(proto_control[1], version);
	apseq = sdp_list_append(apseq, proto_control[1]);

	aproto_control = sdp_list_append(NULL, apseq);
	sdp_set_access_protos(record, aproto_control);
	proto_browsing[0] = sdp_list_append(NULL, &l2cap);
	psm_browsing = sdp_data_alloc(SDP_UINT16, &lp_browsing);
	proto_browsing[0] = sdp_list_append(proto_browsing[0], psm_browsing);
	apseq_browsing = sdp_list_append(NULL, proto_browsing[0]);

	proto_browsing[1] = sdp_list_append(NULL, &avctp);
	proto_browsing[1] = sdp_list_append(proto_browsing[1], version);
	apseq_browsing = sdp_list_append(apseq_browsing, proto_browsing[1]);

	aproto_browsing = sdp_list_append(NULL, apseq_browsing);
	sdp_set_add_access_protos(record, aproto_browsing);

	/* Bluetooth Profile Descriptor List */
	sdp_uuid16_create(&profile[0].uuid, AV_REMOTE_PROFILE_ID);
	profile[0].version = AVRCP_TG_VERSION;
	pfseq = sdp_list_append(NULL, &profile[0]);
	sdp_set_profile_descs(record, pfseq);

	features = sdp_data_alloc(SDP_UINT16, &feat);
	sdp_attr_add(record, SDP_ATTR_SUPPORTED_FEATURES, features);

	sdp_set_info_attr(record, "AVRCP TG", NULL, NULL);

	free(psm_browsing);
	sdp_list_free(proto_browsing[0], NULL);
	sdp_list_free(proto_browsing[1], NULL);
	sdp_list_free(apseq_browsing, NULL);
	sdp_list_free(aproto_browsing, NULL);

	free(psm_control);
	free(version);
	sdp_list_free(proto_control[0], NULL);
	sdp_list_free(proto_control[1], NULL);
	sdp_list_free(apseq, NULL);
	sdp_list_free(aproto_control, NULL);
	sdp_list_free(pfseq, NULL);
	sdp_list_free(root, NULL);
	sdp_list_free(svclass_id, NULL);

	return record;
}

static unsigned int attr_get_max_val(uint8_t attr)
{
	switch (attr) {
	case AVRCP_ATTRIBUTE_EQUALIZER:
		return AVRCP_EQUALIZER_ON;
	case AVRCP_ATTRIBUTE_REPEAT_MODE:
		return AVRCP_REPEAT_MODE_GROUP;
	case AVRCP_ATTRIBUTE_SHUFFLE:
		return AVRCP_SHUFFLE_GROUP;
	case AVRCP_ATTRIBUTE_SCAN:
		return AVRCP_SCAN_GROUP;
	}

	return 0;
}

static const char *battery_status_to_str(uint8_t status)
{
	switch (status) {
	case AVRCP_BATTERY_STATUS_NORMAL:
		return "normal";
	case AVRCP_BATTERY_STATUS_WARNING:
		return "warning";
	case AVRCP_BATTERY_STATUS_CRITICAL:
		return "critical";
	case AVRCP_BATTERY_STATUS_EXTERNAL:
		return "external";
	case AVRCP_BATTERY_STATUS_FULL_CHARGE:
		return "fullcharge";
	}

	return NULL;
}

/*
 * get_company_id:
 *
 * Get three-byte Company_ID from incoming AVRCP message
 */
static uint32_t get_company_id(const uint8_t cid[3])
{
	return cid[0] << 16 | cid[1] << 8 | cid[2];
}

/*
 * set_company_id:
 *
 * Set three-byte Company_ID into outgoing AVRCP message
 */
static void set_company_id(uint8_t cid[3], uint32_t cid_in)
{
	cid[0] = (cid_in & 0xff0000) >> 16;
	cid[1] = (cid_in & 0x00ff00) >> 8;
	cid[2] = (cid_in & 0x0000ff);
}

static const char *attr_to_str(uint8_t attr)
{
	switch (attr) {
	case AVRCP_ATTRIBUTE_EQUALIZER:
		return "Equalizer";
	case AVRCP_ATTRIBUTE_REPEAT_MODE:
		return "Repeat";
	case AVRCP_ATTRIBUTE_SHUFFLE:
		return "Shuffle";
	case AVRCP_ATTRIBUTE_SCAN:
		return "Scan";
	}

	return NULL;
}

static int attrval_to_val(uint8_t attr, const char *value)
{
	int ret;

	switch (attr) {
	case AVRCP_ATTRIBUTE_EQUALIZER:
		if (!strcmp(value, "off"))
			ret = AVRCP_EQUALIZER_OFF;
		else if (!strcmp(value, "on"))
			ret = AVRCP_EQUALIZER_ON;
		else
			ret = -EINVAL;

		return ret;
	case AVRCP_ATTRIBUTE_REPEAT_MODE:
		if (!strcmp(value, "off"))
			ret = AVRCP_REPEAT_MODE_OFF;
		else if (!strcmp(value, "singletrack"))
			ret = AVRCP_REPEAT_MODE_SINGLE;
		else if (!strcmp(value, "alltracks"))
			ret = AVRCP_REPEAT_MODE_ALL;
		else if (!strcmp(value, "group"))
			ret = AVRCP_REPEAT_MODE_GROUP;
		else
			ret = -EINVAL;

		return ret;
	case AVRCP_ATTRIBUTE_SHUFFLE:
		if (!strcmp(value, "off"))
			ret = AVRCP_SHUFFLE_OFF;
		else if (!strcmp(value, "alltracks"))
			ret = AVRCP_SHUFFLE_ALL;
		else if (!strcmp(value, "group"))
			ret = AVRCP_SHUFFLE_GROUP;
		else
			ret = -EINVAL;

		return ret;
	case AVRCP_ATTRIBUTE_SCAN:
		if (!strcmp(value, "off"))
			ret = AVRCP_SCAN_OFF;
		else if (!strcmp(value, "alltracks"))
			ret = AVRCP_SCAN_ALL;
		else if (!strcmp(value, "group"))
			ret = AVRCP_SCAN_GROUP;
		else
			ret = -EINVAL;

		return ret;
	}

	return -EINVAL;
}

static int attr_to_val(const char *str)
{
	if (!strcasecmp(str, "Equalizer"))
		return AVRCP_ATTRIBUTE_EQUALIZER;
	else if (!strcasecmp(str, "Repeat"))
		return AVRCP_ATTRIBUTE_REPEAT_MODE;
	else if (!strcasecmp(str, "Shuffle"))
		return AVRCP_ATTRIBUTE_SHUFFLE;
	else if (!strcasecmp(str, "Scan"))
		return AVRCP_ATTRIBUTE_SCAN;

	return -EINVAL;
}

static int player_get_setting(struct avrcp_player *player, uint8_t id)
{
	const char *key;
	const char *value;

	if (player == NULL)
		return -ENOENT;

	key = attr_to_str(id);
	if (key == NULL)
		return -EINVAL;

	value = player->cb->get_setting(key, player->user_data);
	if (value == NULL)
		return -EINVAL;

	return attrval_to_val(id, value);
}

static int play_status_to_val(const char *status)
{
	if (!strcasecmp(status, "stopped"))
		return AVRCP_PLAY_STATUS_STOPPED;
	else if (!strcasecmp(status, "playing"))
		return AVRCP_PLAY_STATUS_PLAYING;
	else if (!strcasecmp(status, "paused"))
		return AVRCP_PLAY_STATUS_PAUSED;
	else if (!strcasecmp(status, "forward-seek"))
		return AVRCP_PLAY_STATUS_FWD_SEEK;
	else if (!strcasecmp(status, "reverse-seek"))
		return AVRCP_PLAY_STATUS_REV_SEEK;
	else if (!strcasecmp(status, "error"))
		return AVRCP_PLAY_STATUS_ERROR;

	return -EINVAL;
}

void avrcp_player_event(struct avrcp_player *player, uint8_t id,
							const void *data)
{
	uint8_t buf[AVRCP_HEADER_LENGTH + 9];
	struct avrcp_header *pdu = (void *) buf;
	uint8_t code;
	uint16_t size;
	GSList *l;
	int attr;
	int val;

	if (player->sessions == NULL)
		return;

	memset(buf, 0, sizeof(buf));

	set_company_id(pdu->company_id, IEEEID_BTSIG);

	pdu->pdu_id = AVRCP_REGISTER_NOTIFICATION;

	DBG("id=%u", id);

	if (id != AVRCP_EVENT_ADDRESSED_PLAYER_CHANGED && player->changed_id) {
		code = AVC_CTYPE_REJECTED;
		size = 1;
		pdu->params[0] = AVRCP_STATUS_ADDRESSED_PLAYER_CHANGED;
		goto done;
	}

	code = AVC_CTYPE_CHANGED;
	pdu->params[0] = id;

	switch (id) {
	case AVRCP_EVENT_STATUS_CHANGED:
		size = 2;
		pdu->params[1] = play_status_to_val(data);

		break;
	case AVRCP_EVENT_TRACK_CHANGED:
		size = 9;
		memcpy(&pdu->params[1], data, sizeof(uint64_t));

		break;
	case AVRCP_EVENT_TRACK_REACHED_END:
	case AVRCP_EVENT_TRACK_REACHED_START:
		size = 1;
		break;
	case AVRCP_EVENT_SETTINGS_CHANGED:
		size = 2;
		pdu->params[1] = 1;

		attr = attr_to_val(data);
		if (attr < 0)
			return;

		val = player_get_setting(player, attr);
		if (val < 0)
			return;

		pdu->params[size++] = attr;
		pdu->params[size++] = val;
		break;
	case AVRCP_EVENT_ADDRESSED_PLAYER_CHANGED:
		size = 5;
		memcpy(&pdu->params[1], &player->id, sizeof(uint16_t));
		memcpy(&pdu->params[3], &player->uid_counter, sizeof(uint16_t));
		break;
	case AVRCP_EVENT_AVAILABLE_PLAYERS_CHANGED:
		size = 1;
		break;
	default:
		error("Unknown event %u", id);
		return;
	}

done:
	pdu->params_len = htons(size);

	for (l = player->sessions; l; l = l->next) {
		struct avrcp *session = l->data;
		int err;

		if (!(session->registered_events & (1 << id)))
			continue;

		err = avctp_send_vendordep(session->conn,
					session->transaction_events[id],
					code, AVC_SUBUNIT_PANEL,
					buf, size + AVRCP_HEADER_LENGTH);

		if (err < 0)
			continue;

		/* Unregister event as per AVRCP 1.3 spec, section 5.4.2 */
		session->registered_events ^= 1 << id;
	}

	return;
}

static const char *metadata_to_str(uint32_t id)
{
	switch (id) {
	case AVRCP_MEDIA_ATTRIBUTE_TITLE:
		return "Title";
	case AVRCP_MEDIA_ATTRIBUTE_ARTIST:
		return "Artist";
	case AVRCP_MEDIA_ATTRIBUTE_ALBUM:
		return "Album";
	case AVRCP_MEDIA_ATTRIBUTE_GENRE:
		return "Genre";
	case AVRCP_MEDIA_ATTRIBUTE_TRACK:
		return "TrackNumber";
	case AVRCP_MEDIA_ATTRIBUTE_N_TRACKS:
		return "NumberOfTracks";
	case AVRCP_MEDIA_ATTRIBUTE_DURATION:
		return "Duration";
	}

	return NULL;
}

static const char *player_get_metadata(struct avrcp_player *player,
								uint32_t id)
{
	const char *key;

	key = metadata_to_str(id);
	if (key == NULL)
		return NULL;

	if (player != NULL)
		return player->cb->get_metadata(key, player->user_data);

	if (id == AVRCP_MEDIA_ATTRIBUTE_TITLE)
		return "";

	return NULL;
}

static uint16_t player_write_media_attribute(struct avrcp_player *player,
						uint32_t id, uint8_t *buf,
						uint16_t *pos,
						uint16_t *offset)
{
	uint16_t len;
	uint16_t attr_len;
	const char *value = NULL;

	DBG("%u", id);

	value = player_get_metadata(player, id);
	if (value == NULL) {
		*offset = 0;
		return 0;
	}

	attr_len = strlen(value);
	value = ((char *) value) + *offset;
	len = attr_len - *offset;

	if (len > AVRCP_PDU_MTU - *pos) {
		len = AVRCP_PDU_MTU - *pos;
		*offset += len;
	} else {
		*offset = 0;
	}

	memcpy(&buf[*pos], value, len);
	*pos += len;

	return attr_len;
}

static GList *player_fill_media_attribute(struct avrcp_player *player,
					GList *attr_ids, uint8_t *buf,
					uint16_t *pos, uint16_t *offset)
{
	struct media_attribute_header {
		uint32_t id;
		uint16_t charset;
		uint16_t len;
	} *hdr = NULL;
	GList *l;

	for (l = attr_ids; l != NULL; l = g_list_delete_link(l, l)) {
		uint32_t attr = GPOINTER_TO_UINT(l->data);
		uint16_t attr_len;

		if (*offset == 0) {
			if (*pos + sizeof(*hdr) >= AVRCP_PDU_MTU)
				break;

			hdr = (void *) &buf[*pos];
			hdr->id = htonl(attr);
			/* Always use UTF-8 */
			hdr->charset = htons(AVRCP_CHARSET_UTF8);
			*pos += sizeof(*hdr);
		}

		attr_len = player_write_media_attribute(player, attr, buf,
								pos, offset);

		if (hdr != NULL)
			hdr->len = htons(attr_len);

		if (*offset > 0)
			break;
	}

	return l;
}

static struct pending_pdu *pending_pdu_new(uint8_t pdu_id, GList *attr_ids,
							unsigned int offset)
{
	struct pending_pdu *pending = g_new(struct pending_pdu, 1);

	pending->pdu_id = pdu_id;
	pending->attr_ids = attr_ids;
	pending->offset = offset;

	return pending;
}

static gboolean session_abort_pending_pdu(struct avrcp *session)
{
	if (session->pending_pdu == NULL)
		return FALSE;

	g_list_free(session->pending_pdu->attr_ids);
	g_free(session->pending_pdu);
	session->pending_pdu = NULL;

	return TRUE;
}

static const char *attrval_to_str(uint8_t attr, uint8_t value)
{
	switch (attr) {
	case AVRCP_ATTRIBUTE_EQUALIZER:
		switch (value) {
		case AVRCP_EQUALIZER_ON:
			return "on";
		case AVRCP_EQUALIZER_OFF:
			return "off";
		}

		break;
	case AVRCP_ATTRIBUTE_REPEAT_MODE:
		switch (value) {
		case AVRCP_REPEAT_MODE_OFF:
			return "off";
		case AVRCP_REPEAT_MODE_SINGLE:
			return "singletrack";
		case AVRCP_REPEAT_MODE_ALL:
			return "alltracks";
		case AVRCP_REPEAT_MODE_GROUP:
			return "group";
		}

		break;
	/* Shuffle and scan have the same values */
	case AVRCP_ATTRIBUTE_SHUFFLE:
	case AVRCP_ATTRIBUTE_SCAN:
		switch (value) {
		case AVRCP_SCAN_OFF:
			return "off";
		case AVRCP_SCAN_ALL:
			return "alltracks";
		case AVRCP_SCAN_GROUP:
			return "group";
		}

		break;
	}

	return NULL;
}

static int player_set_setting(struct avrcp_player *player, uint8_t id,
								uint8_t val)
{
	const char *key, *value;

	key = attr_to_str(id);
	if (key == NULL)
		return -EINVAL;

	value = attrval_to_str(id, val);
	if (value == NULL)
		return -EINVAL;

	if (player == NULL)
		return -ENOENT;

	return player->cb->set_setting(key, value, player->user_data);
}

static uint8_t avrcp_handle_get_capabilities(struct avrcp *session,
						struct avrcp_header *pdu,
						uint8_t transaction)
{
	uint16_t len = ntohs(pdu->params_len);
	unsigned int i;

	if (len != 1)
		goto err;

	DBG("id=%u", pdu->params[0]);

	switch (pdu->params[0]) {
	case CAP_COMPANY_ID:
		for (i = 0; i < G_N_ELEMENTS(company_ids); i++) {
			set_company_id(&pdu->params[2 + i * 3],
							company_ids[i]);
		}

		pdu->params_len = htons(2 + (3 * G_N_ELEMENTS(company_ids)));
		pdu->params[1] = G_N_ELEMENTS(company_ids);

		return AVC_CTYPE_STABLE;
	case CAP_EVENTS_SUPPORTED:
		pdu->params[1] = 0;
		for (i = 1; i <= AVRCP_EVENT_LAST; i++) {
			if (session->supported_events & (1 << i)) {
				pdu->params[1]++;
				pdu->params[pdu->params[1] + 1] = i;
			}
		}

		pdu->params_len = htons(2 + pdu->params[1]);
		return AVC_CTYPE_STABLE;
	}

err:
	pdu->params_len = htons(1);
	pdu->params[0] = AVRCP_STATUS_INVALID_PARAM;

	return AVC_CTYPE_REJECTED;
}

static struct avrcp_player *target_get_player(struct avrcp *session)
{
	if (!session->target)
		return NULL;

	return session->target->player;
}

static uint8_t avrcp_handle_list_player_attributes(struct avrcp *session,
						struct avrcp_header *pdu,
						uint8_t transaction)
{
	struct avrcp_player *player = target_get_player(session);
	uint16_t len = ntohs(pdu->params_len);
	unsigned int i;

	if (len != 0) {
		pdu->params_len = htons(1);
		pdu->params[0] = AVRCP_STATUS_INVALID_PARAM;
		return AVC_CTYPE_REJECTED;
	}

	if (!player)
		goto done;

	for (i = 1; i <= AVRCP_ATTRIBUTE_SCAN; i++) {
		if (player_get_setting(player, i) < 0)
			continue;

		len++;
		pdu->params[len] = i;
	}

done:
	pdu->params[0] = len;
	pdu->params_len = htons(len + 1);

	return AVC_CTYPE_STABLE;
}

static uint8_t avrcp_handle_list_player_values(struct avrcp *session,
						struct avrcp_header *pdu,
						uint8_t transaction)
{
	struct avrcp_player *player = target_get_player(session);
	uint16_t len = ntohs(pdu->params_len);
	unsigned int i;

	if (len != 1)
		goto err;

	if (player_get_setting(player, pdu->params[0]) < 0)
		goto err;

	len = attr_get_max_val(pdu->params[0]);

	for (i = 1; i <= len; i++)
		pdu->params[i] = i;

	pdu->params[0] = len;
	pdu->params_len = htons(len + 1);

	return AVC_CTYPE_STABLE;

err:
	pdu->params_len = htons(1);
	pdu->params[0] = AVRCP_STATUS_INVALID_PARAM;
	return AVC_CTYPE_REJECTED;
}

static uint32_t str_to_metadata(const char *str)
{
	if (strcasecmp(str, "Title") == 0)
		return AVRCP_MEDIA_ATTRIBUTE_TITLE;
	else if (strcasecmp(str, "Artist") == 0)
		return AVRCP_MEDIA_ATTRIBUTE_ARTIST;
	else if (strcasecmp(str, "Album") == 0)
		return AVRCP_MEDIA_ATTRIBUTE_ALBUM;
	else if (strcasecmp(str, "Genre") == 0)
		return AVRCP_MEDIA_ATTRIBUTE_GENRE;
	else if (strcasecmp(str, "TrackNumber") == 0)
		return AVRCP_MEDIA_ATTRIBUTE_TRACK;
	else if (strcasecmp(str, "NumberOfTracks") == 0)
		return AVRCP_MEDIA_ATTRIBUTE_N_TRACKS;
	else if (strcasecmp(str, "Duration") == 0)
		return AVRCP_MEDIA_ATTRIBUTE_DURATION;

	return 0;
}

static GList *player_list_metadata(struct avrcp_player *player)
{
	GList *l, *attrs = NULL;

	if (player == NULL)
		return g_list_prepend(NULL,
				GUINT_TO_POINTER(AVRCP_MEDIA_ATTRIBUTE_TITLE));

	l = player->cb->list_metadata(player->user_data);
	for (; l; l = l->next) {
		const char *key = l->data;

		attrs = g_list_append(attrs,
					GUINT_TO_POINTER(str_to_metadata(key)));
	}

	return attrs;
}

static uint8_t avrcp_handle_get_element_attributes(struct avrcp *session,
						struct avrcp_header *pdu,
						uint8_t transaction)
{
	struct avrcp_player *player = target_get_player(session);
	uint16_t len = ntohs(pdu->params_len);
	uint64_t identifier = get_le64(&pdu->params[0]);
	uint16_t pos;
	uint8_t nattr;
	GList *attr_ids;
	uint16_t offset;

	if (len < 9 || identifier != 0)
		goto err;

	nattr = pdu->params[8];

	if (len < nattr * sizeof(uint32_t) + 1)
		goto err;

	if (!nattr) {
		/*
		 * Return all available information, at least
		 * title must be returned if there's a track selected.
		 */
		attr_ids = player_list_metadata(player);
		len = g_list_length(attr_ids);
	} else {
		unsigned int i;
		for (i = 0, len = 0, attr_ids = NULL; i < nattr; i++) {
			uint32_t id;

			id = get_be32(&pdu->params[9] + (i * sizeof(id)));

			/* Don't add invalid attributes */
			if (id == AVRCP_MEDIA_ATTRIBUTE_ILLEGAL ||
					id > AVRCP_MEDIA_ATTRIBUTE_LAST)
				continue;

			len++;
			attr_ids = g_list_prepend(attr_ids,
							GUINT_TO_POINTER(id));
		}

		attr_ids = g_list_reverse(attr_ids);
	}

	if (!len)
		goto err;

	session_abort_pending_pdu(session);
	pos = 1;
	offset = 0;
	attr_ids = player_fill_media_attribute(player, attr_ids, pdu->params,
								&pos, &offset);

	if (attr_ids != NULL) {
		session->pending_pdu = pending_pdu_new(pdu->pdu_id, attr_ids,
								offset);
		pdu->packet_type = AVRCP_PACKET_TYPE_START;
	}

	pdu->params[0] = len;
	pdu->params_len = htons(pos);

	return AVC_CTYPE_STABLE;
err:
	pdu->params_len = htons(1);
	pdu->params[0] = AVRCP_STATUS_INVALID_PARAM;
	return AVC_CTYPE_REJECTED;
}

static uint8_t avrcp_handle_get_current_player_value(struct avrcp *session,
						struct avrcp_header *pdu,
						uint8_t transaction)
{
	struct avrcp_player *player = target_get_player(session);
	uint16_t len = ntohs(pdu->params_len);
	uint8_t *settings;
	unsigned int i;

	if (len <= 1 || pdu->params[0] != len - 1)
		goto err;

	/*
	 * Save a copy of requested settings because we can override them
	 * while responding
	 */
	settings = g_memdup(&pdu->params[1], pdu->params[0]);
	len = 0;

	/*
	 * From sec. 5.7 of AVRCP 1.3 spec, we should igore non-existent IDs
	 * and send a response with the existent ones. Only if all IDs are
	 * non-existent we should send an error.
	 */
	for (i = 0; i < pdu->params[0]; i++) {
		int val;

		if (settings[i] < AVRCP_ATTRIBUTE_EQUALIZER ||
					settings[i] > AVRCP_ATTRIBUTE_SCAN) {
			DBG("Ignoring %u", settings[i]);
			continue;
		}

		val = player_get_setting(player, settings[i]);
		if (val < 0)
			continue;

		pdu->params[++len] = settings[i];
		pdu->params[++len] = val;
	}

	g_free(settings);

	if (len) {
		pdu->params[0] = len / 2;
		pdu->params_len = htons(len + 1);

		return AVC_CTYPE_STABLE;
	}

	error("No valid attributes in request");

err:
	pdu->params_len = htons(1);
	pdu->params[0] = AVRCP_STATUS_INVALID_PARAM;

	return AVC_CTYPE_REJECTED;
}

static uint8_t avrcp_handle_set_player_value(struct avrcp *session,
						struct avrcp_header *pdu,
						uint8_t transaction)
{
	struct avrcp_player *player = target_get_player(session);
	uint16_t len = ntohs(pdu->params_len);
	unsigned int i;
	uint8_t *param;

	if (len < 3 || len > 2 * pdu->params[0] + 1U || player == NULL)
		goto err;

	/*
	 * From sec. 5.7 of AVRCP 1.3 spec, we should igore non-existent IDs
	 * and set the existent ones. Sec. 5.2.4 is not clear however how to
	 * indicate that a certain ID was not accepted. If at least one
	 * attribute is valid, we respond with no parameters. Otherwise an
	 * AVRCP_STATUS_INVALID_PARAM is sent.
	 */
	for (len = 0, i = 0, param = &pdu->params[1]; i < pdu->params[0];
							i++, param += 2) {
		if (player_set_setting(player, param[0], param[1]) < 0)
			continue;

		len++;
	}

	if (len) {
		pdu->params_len = 0;

		return AVC_CTYPE_ACCEPTED;
	}

err:
	pdu->params_len = htons(1);
	pdu->params[0] = AVRCP_STATUS_INVALID_PARAM;
	return AVC_CTYPE_REJECTED;
}

static uint8_t avrcp_handle_displayable_charset(struct avrcp *session,
						struct avrcp_header *pdu,
						uint8_t transaction)
{
	uint16_t len = ntohs(pdu->params_len);

	if (len < 3) {
		pdu->params_len = htons(1);
		pdu->params[0] = AVRCP_STATUS_INVALID_PARAM;
		return AVC_CTYPE_REJECTED;
	}

	/*
	 * We acknowledge the commands, but we always use UTF-8 for
	 * encoding since CT is obliged to support it.
	 */
	pdu->params_len = 0;
	return AVC_CTYPE_STABLE;
}

static uint8_t avrcp_handle_ct_battery_status(struct avrcp *session,
						struct avrcp_header *pdu,
						uint8_t transaction)
{
	uint16_t len = ntohs(pdu->params_len);
	const char *valstr;

	if (len != 1)
		goto err;

	valstr = battery_status_to_str(pdu->params[0]);
	if (valstr == NULL)
		goto err;

	pdu->params_len = 0;

	return AVC_CTYPE_STABLE;

err:
	pdu->params_len = htons(1);
	pdu->params[0] = AVRCP_STATUS_INVALID_PARAM;
	return AVC_CTYPE_REJECTED;
}

static uint32_t player_get_position(struct avrcp_player *player)
{
	if (player == NULL)
		return 0;

	return player->cb->get_position(player->user_data);
}

static uint32_t player_get_duration(struct avrcp_player *player)
{
	uint32_t num;

	if (player == NULL)
		return UINT32_MAX;

	num = player->cb->get_duration(player->user_data);
	if (num == 0)
		return UINT32_MAX;

	return num;
}

static uint8_t player_get_status(struct avrcp_player *player)
{
	const char *value;

	if (player == NULL)
		return AVRCP_PLAY_STATUS_STOPPED;

	value = player->cb->get_status(player->user_data);
	if (value == NULL)
		return AVRCP_PLAY_STATUS_STOPPED;

	return play_status_to_val(value);
}

static uint16_t player_get_id(struct avrcp_player *player)
{
	if (player == NULL)
		return 0x0000;

	return player->id;
}

static uint16_t player_get_uid_counter(struct avrcp_player *player)
{
	if (player == NULL)
		return 0x0000;

	return player->uid_counter;
}

static uint8_t avrcp_handle_get_play_status(struct avrcp *session,
						struct avrcp_header *pdu,
						uint8_t transaction)
{
	struct avrcp_player *player = target_get_player(session);
	uint16_t len = ntohs(pdu->params_len);
	uint32_t position;
	uint32_t duration;

	if (len != 0) {
		pdu->params_len = htons(1);
		pdu->params[0] = AVRCP_STATUS_INVALID_PARAM;
		return AVC_CTYPE_REJECTED;
	}

	position = player_get_position(player);
	duration = player_get_duration(player);

	position = htonl(position);
	duration = htonl(duration);

	memcpy(&pdu->params[0], &duration, 4);
	memcpy(&pdu->params[4], &position, 4);
	pdu->params[8] = player_get_status(player);

	pdu->params_len = htons(9);

	return AVC_CTYPE_STABLE;
}

static uint64_t player_get_uid(struct avrcp_player *player)
{
	if (player == NULL)
		return UINT64_MAX;

	return player->cb->get_uid(player->user_data);
}

static GList *player_list_settings(struct avrcp_player *player)
{
	if (player == NULL)
		return NULL;

	return player->cb->list_settings(player->user_data);
}

static bool avrcp_handle_play(struct avrcp *session)
{
	struct avrcp_player *player = target_get_player(session);

	if (player == NULL)
		return false;

	return player->cb->play(player->user_data);
}

static bool avrcp_handle_stop(struct avrcp *session)
{
	struct avrcp_player *player = target_get_player(session);

	if (player == NULL)
		return false;

	return player->cb->stop(player->user_data);
}

static bool avrcp_handle_pause(struct avrcp *session)
{
	struct avrcp_player *player = target_get_player(session);

	if (player == NULL)
		return false;

	return player->cb->pause(player->user_data);
}

static bool avrcp_handle_next(struct avrcp *session)
{
	struct avrcp_player *player = target_get_player(session);

	if (player == NULL)
		return false;

	return player->cb->next(player->user_data);
}

static bool avrcp_handle_previous(struct avrcp *session)
{
	struct avrcp_player *player = target_get_player(session);

	if (player == NULL)
		return false;

	return player->cb->previous(player->user_data);
}

static const struct passthrough_handler passthrough_handlers[] = {
		{ AVC_PLAY, avrcp_handle_play },
		{ AVC_STOP, avrcp_handle_stop },
		{ AVC_PAUSE, avrcp_handle_pause },
		{ AVC_FORWARD, avrcp_handle_next },
		{ AVC_BACKWARD, avrcp_handle_previous },
		{ },
};

static bool handle_passthrough(struct avctp *conn, uint8_t op, bool pressed,
							void *user_data)
{
	struct avrcp *session = user_data;
	const struct passthrough_handler *handler;

	for (handler = session->passthrough_handlers; handler->func;
								handler++) {
		if (handler->op == op)
			break;
	}

	if (handler->func == NULL)
		return false;

	/* Do not trigger handler on release */
	if (!pressed)
		return true;

	return handler->func(session);
}

static uint8_t avrcp_handle_register_notification(struct avrcp *session,
						struct avrcp_header *pdu,
						uint8_t transaction)
{
	struct avrcp_player *player = target_get_player(session);
	struct btd_device *dev = session->dev;
	uint16_t len = ntohs(pdu->params_len);
	uint64_t uid;
	GList *settings;

	/*
	 * 1 byte for EventID, 4 bytes for Playback interval but the latest
	 * one is applicable only for EVENT_PLAYBACK_POS_CHANGED. See AVRCP
	 * 1.3 spec, section 5.4.2.
	 */
	if (len != 5)
		goto err;

	/* Check if event is supported otherwise reject */
	if (!(session->supported_events & (1 << pdu->params[0])))
		goto err;

	switch (pdu->params[0]) {
	case AVRCP_EVENT_STATUS_CHANGED:
		len = 2;
		pdu->params[1] = player_get_status(player);

		break;
	case AVRCP_EVENT_TRACK_CHANGED:
		len = 9;
		uid = player_get_uid(player);
		memcpy(&pdu->params[1], &uid, sizeof(uint64_t));

		break;
	case AVRCP_EVENT_TRACK_REACHED_END:
	case AVRCP_EVENT_TRACK_REACHED_START:
		len = 1;
		break;
	case AVRCP_EVENT_SETTINGS_CHANGED:
		len = 1;
		settings = player_list_settings(player);

		pdu->params[len++] = g_list_length(settings);
		for (; settings; settings = settings->next) {
			const char *key = settings->data;
			int attr;
			int val;

			attr = attr_to_val(key);
			if (attr < 0)
				continue;

			val = player_get_setting(player, attr);
			if (val < 0)
				continue;

			pdu->params[len++] = attr;
			pdu->params[len++] = val;
		}

		g_list_free(settings);

		break;
	case AVRCP_EVENT_ADDRESSED_PLAYER_CHANGED:
		len = 5;
		bt_put_be16(player_get_id(player), &pdu->params[1]);
		bt_put_be16(player_get_uid_counter(player), &pdu->params[3]);
		break;
	case AVRCP_EVENT_AVAILABLE_PLAYERS_CHANGED:
		len = 1;
		break;
	case AVRCP_EVENT_VOLUME_CHANGED:
		pdu->params[1] = media_transport_get_device_volume(dev);
		if (pdu->params[1] > 127)
			goto err;

		len = 2;

		break;
	default:
		/* All other events are not supported yet */
		goto err;
	}

	/* Register event and save the transaction used */
	session->registered_events |= (1 << pdu->params[0]);
	session->transaction_events[pdu->params[0]] = transaction;

	pdu->params_len = htons(len);

	return AVC_CTYPE_INTERIM;

err:
	pdu->params_len = htons(1);
	pdu->params[0] = AVRCP_STATUS_INVALID_PARAM;
	return AVC_CTYPE_REJECTED;
}

static uint8_t avrcp_handle_request_continuing(struct avrcp *session,
						struct avrcp_header *pdu,
						uint8_t transaction)
{
	struct avrcp_player *player = target_get_player(session);
	uint16_t len = ntohs(pdu->params_len);
	struct pending_pdu *pending;

	if (len != 1 || session->pending_pdu == NULL)
		goto err;

	pending = session->pending_pdu;

	if (pending->pdu_id != pdu->params[0])
		goto err;


	len = 0;
	pending->attr_ids = player_fill_media_attribute(player,
							pending->attr_ids,
							pdu->params, &len,
							&pending->offset);
	pdu->pdu_id = pending->pdu_id;

	if (pending->attr_ids == NULL) {
		g_free(session->pending_pdu);
		session->pending_pdu = NULL;
		pdu->packet_type = AVRCP_PACKET_TYPE_END;
	} else {
		pdu->packet_type = AVRCP_PACKET_TYPE_CONTINUING;
	}

	pdu->params_len = htons(len);

	return AVC_CTYPE_STABLE;
err:
	pdu->params_len = htons(1);
	pdu->params[0] = AVRCP_STATUS_INVALID_PARAM;
	return AVC_CTYPE_REJECTED;
}

static uint8_t avrcp_handle_abort_continuing(struct avrcp *session,
						struct avrcp_header *pdu,
						uint8_t transaction)
{
	uint16_t len = ntohs(pdu->params_len);
	struct pending_pdu *pending;

	if (len != 1 || session->pending_pdu == NULL)
		goto err;

	pending = session->pending_pdu;

	if (pending->pdu_id != pdu->params[0])
		goto err;

	session_abort_pending_pdu(session);
	pdu->params_len = 0;

	return AVC_CTYPE_ACCEPTED;

err:
	pdu->params_len = htons(1);
	pdu->params[0] = AVRCP_STATUS_INVALID_PARAM;
	return AVC_CTYPE_REJECTED;
}

static uint8_t avrcp_handle_set_absolute_volume(struct avrcp *session,
						struct avrcp_header *pdu,
						uint8_t transaction)
{
	uint16_t len = ntohs(pdu->params_len);
	uint8_t volume;

	if (len != 1)
		goto err;

	volume = pdu->params[0] & 0x7F;

	media_transport_update_device_volume(session->dev, volume);

	return AVC_CTYPE_ACCEPTED;

err:
	pdu->params_len = htons(1);
	pdu->params[0] = AVRCP_STATUS_INVALID_PARAM;
	return AVC_CTYPE_REJECTED;
}

static struct avrcp_player *find_tg_player(struct avrcp *session, uint16_t id)
{
	struct avrcp_server *server = session->server;
	GSList *l;

	for (l = server->players; l; l = l->next) {
		struct avrcp_player *player = l->data;

		if (player->id == id)
			return player;
	}

	return NULL;
}

static gboolean notify_addressed_player_changed(gpointer user_data)
{
	struct avrcp_player *player = user_data;
	uint8_t events[6] = { AVRCP_EVENT_STATUS_CHANGED,
					AVRCP_EVENT_TRACK_CHANGED,
					AVRCP_EVENT_TRACK_REACHED_START,
					AVRCP_EVENT_TRACK_REACHED_END,
					AVRCP_EVENT_SETTINGS_CHANGED,
					AVRCP_EVENT_PLAYBACK_POS_CHANGED
				};
	uint8_t i;

	avrcp_player_event(player, AVRCP_EVENT_ADDRESSED_PLAYER_CHANGED, NULL);

	/*
	 * TG shall complete all player specific
	 * notifications with AV/C C-Type REJECTED
	 * with error code as Addressed Player Changed.
	 */
	for (i = 0; i < sizeof(events); i++)
		avrcp_player_event(player, events[i], NULL);

	player->changed_id = 0;

	return FALSE;
}

static uint8_t avrcp_handle_set_addressed_player(struct avrcp *session,
						struct avrcp_header *pdu,
						uint8_t transaction)
{
	struct avrcp_player *player;
	uint16_t len = ntohs(pdu->params_len);
	uint16_t player_id = 0;
	uint8_t status;

	if (len < 1) {
		status = AVRCP_STATUS_INVALID_PARAM;
		goto err;
	}

	player_id = bt_get_be16(&pdu->params[0]);
	player = find_tg_player(session, player_id);
	pdu->packet_type = AVRCP_PACKET_TYPE_SINGLE;

	if (player) {
		player->addressed = true;
		status = AVRCP_STATUS_SUCCESS;
		pdu->params_len = htons(len);
		pdu->params[0] = status;
	} else {
		status = AVRCP_STATUS_INVALID_PLAYER_ID;
		goto err;
	}

	/* Don't emit player changed immediately since PTS expect the
	 * response of SetAddressedPlayer before the event.
	 */
	player->changed_id = g_idle_add(notify_addressed_player_changed,
								player);

	return AVC_CTYPE_ACCEPTED;

err:
	pdu->params_len = htons(sizeof(status));
	pdu->params[0] = status;
	return AVC_CTYPE_REJECTED;
}

static const struct control_pdu_handler control_handlers[] = {
		{ AVRCP_GET_CAPABILITIES, AVC_CTYPE_STATUS,
					avrcp_handle_get_capabilities },
		{ AVRCP_LIST_PLAYER_ATTRIBUTES, AVC_CTYPE_STATUS,
					avrcp_handle_list_player_attributes },
		{ AVRCP_LIST_PLAYER_VALUES, AVC_CTYPE_STATUS,
					avrcp_handle_list_player_values },
		{ AVRCP_GET_ELEMENT_ATTRIBUTES, AVC_CTYPE_STATUS,
					avrcp_handle_get_element_attributes },
		{ AVRCP_GET_CURRENT_PLAYER_VALUE, AVC_CTYPE_STATUS,
					avrcp_handle_get_current_player_value },
		{ AVRCP_SET_PLAYER_VALUE, AVC_CTYPE_CONTROL,
					avrcp_handle_set_player_value },
		{ AVRCP_GET_PLAYER_ATTRIBUTE_TEXT, AVC_CTYPE_STATUS,
					NULL },
		{ AVRCP_GET_PLAYER_VALUE_TEXT, AVC_CTYPE_STATUS,
					NULL },
		{ AVRCP_DISPLAYABLE_CHARSET, AVC_CTYPE_STATUS,
					avrcp_handle_displayable_charset },
		{ AVRCP_CT_BATTERY_STATUS, AVC_CTYPE_STATUS,
					avrcp_handle_ct_battery_status },
		{ AVRCP_GET_PLAY_STATUS, AVC_CTYPE_STATUS,
					avrcp_handle_get_play_status },
		{ AVRCP_REGISTER_NOTIFICATION, AVC_CTYPE_NOTIFY,
					avrcp_handle_register_notification },
		{ AVRCP_SET_ABSOLUTE_VOLUME, AVC_CTYPE_CONTROL,
					avrcp_handle_set_absolute_volume },
		{ AVRCP_REQUEST_CONTINUING, AVC_CTYPE_CONTROL,
					avrcp_handle_request_continuing },
		{ AVRCP_ABORT_CONTINUING, AVC_CTYPE_CONTROL,
					avrcp_handle_abort_continuing },
		{ AVRCP_SET_ADDRESSED_PLAYER, AVC_CTYPE_CONTROL,
					avrcp_handle_set_addressed_player },
		{ },
};

/* handle vendordep pdu inside an avctp packet */
static size_t handle_vendordep_pdu(struct avctp *conn, uint8_t transaction,
					uint8_t *code, uint8_t *subunit,
					uint8_t *operands, size_t operand_count,
					void *user_data)
{
	struct avrcp *session = user_data;
	const struct control_pdu_handler *handler;
	struct avrcp_header *pdu = (void *) operands;
	uint32_t company_id = get_company_id(pdu->company_id);

	if (company_id != IEEEID_BTSIG) {
		*code = AVC_CTYPE_NOT_IMPLEMENTED;
		return 0;
	}

	DBG("AVRCP PDU 0x%02X, company 0x%06X len 0x%04X",
			pdu->pdu_id, company_id, ntohs(pdu->params_len));

	pdu->packet_type = 0;
	pdu->rsvd = 0;

	if (operand_count < AVRCP_HEADER_LENGTH) {
		pdu->params[0] = AVRCP_STATUS_INVALID_COMMAND;
		goto err_metadata;
	}

	for (handler = session->control_handlers; handler->pdu_id; handler++) {
		if (handler->pdu_id == pdu->pdu_id)
			break;
	}

	if (handler->pdu_id != pdu->pdu_id || handler->code != *code) {
		pdu->params[0] = AVRCP_STATUS_INVALID_COMMAND;
		goto err_metadata;
	}

	if (!handler->func) {
		pdu->params[0] = AVRCP_STATUS_INVALID_PARAM;
		goto err_metadata;
	}

	*code = handler->func(session, pdu, transaction);

	if (*code != AVC_CTYPE_REJECTED &&
				pdu->pdu_id != AVRCP_GET_ELEMENT_ATTRIBUTES &&
				pdu->pdu_id != AVRCP_REQUEST_CONTINUING &&
				pdu->pdu_id != AVRCP_ABORT_CONTINUING)
		session_abort_pending_pdu(session);

	return AVRCP_HEADER_LENGTH + ntohs(pdu->params_len);

err_metadata:
	pdu->params_len = htons(1);
	*code = AVC_CTYPE_REJECTED;

	return AVRCP_HEADER_LENGTH + 1;
}

static void avrcp_handle_media_player_list(struct avrcp *session,
				struct avrcp_browsing_header *pdu,
				uint32_t start_item, uint32_t end_item)
{
	struct avrcp_player *player = session->target->player;
	struct get_folder_items_rsp *rsp;
	const char *name = NULL;
	GSList *l;

	rsp = (void *)pdu->params;
	rsp->status = AVRCP_STATUS_SUCCESS;
	rsp->uid_counter = htons(player_get_uid_counter(player));
	rsp->num_items = 0;
	pdu->param_len = sizeof(*rsp);

	for (l = g_slist_nth(session->server->players, start_item);
					l; l = g_slist_next(l)) {
		struct avrcp_player *player = l->data;
		struct folder_item *folder;
		struct player_item *item;
		uint16_t namelen;

		if (rsp->num_items == (end_item - start_item) + 1)
			break;

		folder = (void *)&pdu->params[pdu->param_len];
		folder->type = 0x01; /* Media Player */

		pdu->param_len += sizeof(*folder);

		item = (void *)folder->data;
		item->player_id = htons(player->id);
		item->type = 0x01; /* Audio */
		item->subtype = htonl(0x01); /* Audio Book */
		item->status = player_get_status(player);
		/* Assign Default Feature Bit Mask */
		memcpy(&item->features, &features, sizeof(features));

		item->charset = htons(AVRCP_CHARSET_UTF8);

		name = player->cb->get_name(player->user_data);
		namelen = strlen(name);
		item->namelen = htons(namelen);
		memcpy(item->name, name, namelen);

		folder->len = htons(sizeof(*item) + namelen);
		pdu->param_len += sizeof(*item) + namelen;
		rsp->num_items++;
	}

	/* If no player could be found respond with an error */
	if (!rsp->num_items)
		goto failed;

	rsp->num_items = htons(rsp->num_items);
	pdu->param_len = htons(pdu->param_len);

	return;

failed:
	pdu->params[0] = AVRCP_STATUS_OUT_OF_BOUNDS;
	pdu->param_len = htons(1);
}

static void avrcp_handle_get_folder_items(struct avrcp *session,
				struct avrcp_browsing_header *pdu,
				uint8_t transaction)
{
	uint32_t start_item = 0;
	uint32_t end_item = 0;
	uint8_t scope;
	uint8_t status = AVRCP_STATUS_SUCCESS;

	if (ntohs(pdu->param_len) < 10) {
		status = AVRCP_STATUS_INVALID_PARAM;
		goto failed;
	}

	scope = pdu->params[0];
	start_item = bt_get_be32(&pdu->params[1]);
	end_item = bt_get_be32(&pdu->params[5]);

	DBG("scope 0x%02x start_item 0x%08x end_item 0x%08x", scope,
				start_item, end_item);

	if (end_item < start_item) {
		status = AVRCP_STATUS_INVALID_PARAM;
		goto failed;
	}

	switch (scope) {
	case AVRCP_SCOPE_MEDIA_PLAYER_LIST:
		avrcp_handle_media_player_list(session, pdu,
						start_item, end_item);
		break;
	case AVRCP_SCOPE_MEDIA_PLAYER_VFS:
	case AVRCP_SCOPE_SEARCH:
	case AVRCP_SCOPE_NOW_PLAYING:
	default:
		status = AVRCP_STATUS_INVALID_PARAM;
		goto failed;
	}

	return;

failed:
	pdu->params[0] = status;
	pdu->param_len = htons(1);
}

static struct browsing_pdu_handler {
	uint8_t pdu_id;
	void (*func) (struct avrcp *session, struct avrcp_browsing_header *pdu,
							uint8_t transaction);
} browsing_handlers[] = {
		{ AVRCP_GET_FOLDER_ITEMS, avrcp_handle_get_folder_items },
		{ },
};

size_t avrcp_browsing_general_reject(uint8_t *operands)
{
	struct avrcp_browsing_header *pdu = (void *) operands;
	uint8_t status;

	pdu->pdu_id = AVRCP_GENERAL_REJECT;
	status = AVRCP_STATUS_INVALID_COMMAND;

	pdu->param_len = htons(sizeof(status));
	memcpy(pdu->params, &status, (sizeof(status)));
	return AVRCP_BROWSING_HEADER_LENGTH + sizeof(status);
}

static size_t handle_browsing_pdu(struct avctp *conn,
					uint8_t transaction, uint8_t *operands,
					size_t operand_count, void *user_data)
{
	struct avrcp *session = user_data;
	struct browsing_pdu_handler *handler;
	struct avrcp_browsing_header *pdu = (void *) operands;

	DBG("AVRCP Browsing PDU 0x%02X, len 0x%04X", pdu->pdu_id,
							ntohs(pdu->param_len));

	for (handler = browsing_handlers; handler->pdu_id; handler++) {
		if (handler->pdu_id == pdu->pdu_id)
			goto done;
	}

	return avrcp_browsing_general_reject(operands);

done:
	session->transaction = transaction;
	handler->func(session, pdu, transaction);
	return AVRCP_BROWSING_HEADER_LENGTH + ntohs(pdu->param_len);
}

size_t avrcp_handle_vendor_reject(uint8_t *code, uint8_t *operands)
{
	struct avrcp_header *pdu = (void *) operands;
	uint32_t company_id = get_company_id(pdu->company_id);

	*code = AVC_CTYPE_REJECTED;
	pdu->params_len = htons(1);
	pdu->params[0] = AVRCP_STATUS_INTERNAL_ERROR;

	DBG("rejecting AVRCP PDU 0x%02X, company 0x%06X len 0x%04X",
			pdu->pdu_id, company_id, ntohs(pdu->params_len));

	return AVRCP_HEADER_LENGTH + 1;
}

static struct avrcp_server *find_server(GSList *list, struct btd_adapter *a)
{
	for (; list; list = list->next) {
		struct avrcp_server *server = list->data;

		if (server->adapter == a)
			return server;
	}

	return NULL;
}

static const char *status_to_string(uint8_t status)
{
	switch (status) {
	case AVRCP_PLAY_STATUS_STOPPED:
		return "stopped";
	case AVRCP_PLAY_STATUS_PLAYING:
		return "playing";
	case AVRCP_PLAY_STATUS_PAUSED:
		return "paused";
	case AVRCP_PLAY_STATUS_FWD_SEEK:
		return "forward-seek";
	case AVRCP_PLAY_STATUS_REV_SEEK:
		return "reverse-seek";
	case AVRCP_PLAY_STATUS_ERROR:
		return "error";
	default:
		return NULL;
	}
}

static gboolean avrcp_get_play_status_rsp(struct avctp *conn, uint8_t code,
					uint8_t subunit, uint8_t transaction,
					uint8_t *operands, size_t operand_count,
					void *user_data)
{
	struct avrcp *session = user_data;
	struct avrcp_player *player = session->controller->player;
	struct media_player *mp = player->user_data;
	struct avrcp_header *pdu = (void *) operands;
	uint32_t duration;
	uint32_t position;
	uint8_t status;

	if (pdu == NULL || code == AVC_CTYPE_REJECTED ||
						ntohs(pdu->params_len) != 9)
		return FALSE;

	memcpy(&duration, pdu->params, sizeof(uint32_t));
	duration = ntohl(duration);
	media_player_set_duration(mp, duration);

	memcpy(&position, pdu->params + 4, sizeof(uint32_t));
	position = ntohl(position);
	media_player_set_position(mp, position);

	memcpy(&status, pdu->params + 8, sizeof(uint8_t));
	media_player_set_status(mp, status_to_string(status));

	return FALSE;
}

static void avrcp_get_play_status(struct avrcp *session)
{
	uint8_t buf[AVRCP_HEADER_LENGTH];
	struct avrcp_header *pdu = (void *) buf;

	memset(buf, 0, sizeof(buf));

	set_company_id(pdu->company_id, IEEEID_BTSIG);
	pdu->pdu_id = AVRCP_GET_PLAY_STATUS;
	pdu->packet_type = AVRCP_PACKET_TYPE_SINGLE;

	avctp_send_vendordep_req(session->conn, AVC_CTYPE_STATUS,
					AVC_SUBUNIT_PANEL, buf, sizeof(buf),
					avrcp_get_play_status_rsp,
					session);
}

static const char *status_to_str(uint8_t status)
{
	switch (status) {
	case AVRCP_STATUS_INVALID_COMMAND:
		return "Invalid Command";
	case AVRCP_STATUS_INVALID_PARAM:
		return "Invalid Parameter";
	case AVRCP_STATUS_INTERNAL_ERROR:
		return "Internal Error";
	case AVRCP_STATUS_SUCCESS:
		return "Success";
	default:
		return "Unknown";
	}
}

static gboolean avrcp_player_value_rsp(struct avctp *conn, uint8_t code,
					uint8_t subunit, uint8_t transaction,
					uint8_t *operands, size_t operand_count,
					void *user_data)
{
	struct avrcp *session = user_data;
	struct avrcp_player *player = session->controller->player;
	struct media_player *mp = player->user_data;
	struct avrcp_header *pdu = (void *) operands;
	uint8_t count;
	int i;

	if (pdu == NULL) {
		media_player_set_setting(mp, "Error", "Timeout");
		return FALSE;
	}

	if (code == AVC_CTYPE_REJECTED) {
		media_player_set_setting(mp, "Error",
					status_to_str(pdu->params[0]));
		return FALSE;
	}

	count = pdu->params[0];

	if (pdu->params_len < count * 2)
		return FALSE;

	for (i = 1; count > 0; count--, i += 2) {
		const char *key;
		const char *value;

		key = attr_to_str(pdu->params[i]);
		if (key == NULL)
			continue;

		value = attrval_to_str(pdu->params[i], pdu->params[i + 1]);
		if (value == NULL)
			continue;

		media_player_set_setting(mp, key, value);
	}

	return FALSE;
}

static void avrcp_get_current_player_value(struct avrcp *session,
						uint8_t *attrs, uint8_t count)
{
	uint8_t buf[AVRCP_HEADER_LENGTH + AVRCP_ATTRIBUTE_LAST + 1];
	struct avrcp_header *pdu = (void *) buf;
	uint16_t length = AVRCP_HEADER_LENGTH + count + 1;

	memset(buf, 0, sizeof(buf));

	set_company_id(pdu->company_id, IEEEID_BTSIG);
	pdu->pdu_id = AVRCP_GET_CURRENT_PLAYER_VALUE;
	pdu->packet_type = AVRCP_PACKET_TYPE_SINGLE;
	pdu->params_len = htons(count + 1);
	pdu->params[0] = count;

	memcpy(pdu->params + 1, attrs, count);

	avctp_send_vendordep_req(session->conn, AVC_CTYPE_STATUS,
					AVC_SUBUNIT_PANEL, buf, length,
					avrcp_player_value_rsp, session);
}

static gboolean avrcp_list_player_attributes_rsp(struct avctp *conn,
					uint8_t code, uint8_t subunit,
					uint8_t transaction, uint8_t *operands,
					size_t operand_count, void *user_data)
{
	uint8_t attrs[AVRCP_ATTRIBUTE_LAST];
	struct avrcp *session = user_data;
	struct avrcp_header *pdu = (void *) operands;
	uint8_t len, count = 0;
	int i;

	if (code == AVC_CTYPE_REJECTED)
		return FALSE;

	len = pdu->params[0];

	if (ntohs(pdu->params_len) < count) {
		error("Invalid parameters");
		return FALSE;
	}

	for (i = 0; len > 0; len--, i++) {
		/* Don't query invalid attributes */
		if (pdu->params[i + 1] == AVRCP_ATTRIBUTE_ILEGAL ||
				pdu->params[i + 1] > AVRCP_ATTRIBUTE_LAST)
			continue;

		attrs[count++] = pdu->params[i + 1];
	}

	avrcp_get_current_player_value(session, attrs, count);

	return FALSE;
}

static void avrcp_list_player_attributes(struct avrcp *session)
{
	uint8_t buf[AVRCP_HEADER_LENGTH];
	struct avrcp_header *pdu = (void *) buf;

	memset(buf, 0, sizeof(buf));

	set_company_id(pdu->company_id, IEEEID_BTSIG);
	pdu->pdu_id = AVRCP_LIST_PLAYER_ATTRIBUTES;
	pdu->packet_type = AVRCP_PACKET_TYPE_SINGLE;

	avctp_send_vendordep_req(session->conn, AVC_CTYPE_STATUS,
					AVC_SUBUNIT_PANEL, buf, sizeof(buf),
					avrcp_list_player_attributes_rsp,
					session);
}

static void avrcp_parse_attribute_list(struct avrcp_player *player,
					uint8_t *operands, uint8_t count)
{
	struct media_player *mp = player->user_data;
	struct media_item *item;
	int i;

	item = media_player_set_playlist_item(mp, player->uid);

	for (i = 0; count > 0; count--) {
		uint32_t id;
		uint16_t charset, len;

		id = get_be32(&operands[i]);
		i += sizeof(uint32_t);

		charset = get_be16(&operands[i]);
		i += sizeof(uint16_t);

		len = get_be16(&operands[i]);
		i += sizeof(uint16_t);

		if (charset == 106) {
			const char *key = metadata_to_str(id);

			if (key != NULL)
				media_player_set_metadata(mp, item,
							metadata_to_str(id),
							&operands[i], len);
		}

		i += len;
	}
}

static gboolean avrcp_get_element_attributes_rsp(struct avctp *conn,
						uint8_t code, uint8_t subunit,
						uint8_t transaction,
						uint8_t *operands,
						size_t operand_count,
						void *user_data)
{
	struct avrcp *session = user_data;
	struct avrcp_player *player = session->controller->player;
	struct avrcp_header *pdu = (void *) operands;
	uint8_t count;

	if (code == AVC_CTYPE_REJECTED)
		return FALSE;

	count = pdu->params[0];

	if (ntohs(pdu->params_len) - 1 < count * 8) {
		error("Invalid parameters");
		return FALSE;
	}

	avrcp_parse_attribute_list(player, &pdu->params[1], count);

	avrcp_get_play_status(session);

	return FALSE;
}

static void avrcp_get_element_attributes(struct avrcp *session)
{
	uint8_t buf[AVRCP_HEADER_LENGTH + 9];
	struct avrcp_header *pdu = (void *) buf;
	uint16_t length;

	memset(buf, 0, sizeof(buf));

	set_company_id(pdu->company_id, IEEEID_BTSIG);
	pdu->pdu_id = AVRCP_GET_ELEMENT_ATTRIBUTES;
	pdu->params_len = htons(9);
	pdu->packet_type = AVRCP_PACKET_TYPE_SINGLE;

	length = AVRCP_HEADER_LENGTH + ntohs(pdu->params_len);

	avctp_send_vendordep_req(session->conn, AVC_CTYPE_STATUS,
					AVC_SUBUNIT_PANEL, buf, length,
					avrcp_get_element_attributes_rsp,
					session);
}

static const char *type_to_string(uint8_t type)
{
	switch (type & 0x0F) {
	case 0x01:
		return "Audio";
	case 0x02:
		return "Video";
	case 0x03:
		return "Audio, Video";
	case 0x04:
		return "Audio Broadcasting";
	case 0x05:
		return "Audio, Audio Broadcasting";
	case 0x06:
		return "Video, Audio Broadcasting";
	case 0x07:
		return "Audio, Video, Audio Broadcasting";
	case 0x08:
		return "Video Broadcasting";
	case 0x09:
		return "Audio, Video Broadcasting";
	case 0x0A:
		return "Video, Video Broadcasting";
	case 0x0B:
		return "Audio, Video, Video Broadcasting";
	case 0x0C:
		return "Audio Broadcasting, Video Broadcasting";
	case 0x0D:
		return "Audio, Audio Broadcasting, Video Broadcasting";
	case 0x0E:
		return "Video, Audio Broadcasting, Video Broadcasting";
	case 0x0F:
		return "Audio, Video, Audio Broadcasting, Video Broadcasting";
	}

	return "None";
}

static const char *subtype_to_string(uint32_t subtype)
{
	switch (subtype & 0x03) {
	case 0x01:
		return "Audio Book";
	case 0x02:
		return "Podcast";
	case 0x03:
		return "Audio Book, Podcast";
	}

	return "None";
}

static struct media_item *parse_media_element(struct avrcp *session,
					uint8_t *operands, uint16_t len)
{
	struct avrcp_player *player;
	struct media_player *mp;
	struct media_item *item;
	uint16_t namelen;
	char name[255];
	uint64_t uid;

	if (len < 13)
		return NULL;

	uid = get_be64(&operands[0]);

	namelen = MIN(get_be16(&operands[11]), sizeof(name) - 1);
	if (namelen > 0) {
		memcpy(name, &operands[13], namelen);
		name[namelen] = '\0';
	}

	player = session->controller->player;
	mp = player->user_data;

	item = media_player_create_item(mp, name, PLAYER_ITEM_TYPE_AUDIO, uid);
	if (item == NULL)
		return NULL;

	media_item_set_playable(item, true);

	return item;
}

static struct media_item *parse_media_folder(struct avrcp *session,
					uint8_t *operands, uint16_t len)
{
	struct avrcp_player *player = session->controller->player;
	struct media_player *mp = player->user_data;
	struct media_item *item;
	uint16_t namelen;
	char name[255];
	uint64_t uid;
	uint8_t type;
	uint8_t playable;

	if (len < 12)
		return NULL;

	uid = get_be64(&operands[0]);
	type = operands[8];
	playable = operands[9];

	namelen = MIN(get_be16(&operands[12]), sizeof(name) - 1);
	if (namelen > 0) {
		memcpy(name, &operands[14], namelen);
		name[namelen] = '\0';
	}

	item = media_player_create_folder(mp, name, type, uid);
	if (!item)
		return NULL;

	media_item_set_playable(item, playable & 0x01);

	return item;
}

static void avrcp_list_items(struct avrcp *session, uint32_t start,
								uint32_t end);
static gboolean avrcp_list_items_rsp(struct avctp *conn, uint8_t *operands,
					size_t operand_count, void *user_data)
{
	struct avrcp_browsing_header *pdu = (void *) operands;
	struct avrcp *session = user_data;
	struct avrcp_player *player = session->controller->player;
	struct pending_list_items *p = player->p;
	uint16_t count;
	uint64_t items;
	size_t i;
	int err = 0;

	if (pdu == NULL) {
		err = -ETIMEDOUT;
		goto done;
	}

	/* AVRCP 1.5 - Page 76:
	 * If the TG receives a GetFolderItems command for an empty folder then
	 * the TG shall return the error (= Range Out of Bounds) in the status
	 * field of the GetFolderItems response.
	 */
	if (pdu->params[0] == AVRCP_STATUS_OUT_OF_BOUNDS)
		goto done;

	if (pdu->params[0] != AVRCP_STATUS_SUCCESS || operand_count < 5) {
		err = -EINVAL;
		goto done;
	}

	count = get_be16(&operands[6]);
	if (count == 0)
		goto done;

	for (i = 8; count && i + 3 < operand_count; count--) {
		struct media_item *item;
		uint8_t type;
		uint16_t len;

		type = operands[i++];
		len = get_be16(&operands[i]);
		i += 2;

		if (type != 0x03 && type != 0x02) {
			i += len;
			continue;
		}

		if (i + len > operand_count) {
			error("Invalid item length");
			break;
		}

		if (type == 0x03)
			item = parse_media_element(session, &operands[i], len);
		else
			item = parse_media_folder(session, &operands[i], len);

		if (item) {
			if (g_slist_find(p->items, item))
				goto done;
			p->items = g_slist_append(p->items, item);
		}

		i += len;
	}

	items = g_slist_length(p->items);

	DBG("start %u end %u items %" PRIu64 " total %" PRIu64 "", p->start,
						p->end, items, p->total);

	if (items < p->total) {
		avrcp_list_items(session, p->start + items, p->end);
		return FALSE;
	}

done:
	media_player_list_complete(player->user_data, p->items, err);

	g_slist_free(p->items);
	g_free(p);
	player->p = NULL;

	return FALSE;
}

static void avrcp_list_items(struct avrcp *session, uint32_t start,
								uint32_t end)
{
	uint8_t buf[AVRCP_BROWSING_HEADER_LENGTH + 10 +
			AVRCP_MEDIA_ATTRIBUTE_LAST * sizeof(uint32_t)];
	struct avrcp_player *player = session->controller->player;
	struct avrcp_browsing_header *pdu = (void *) buf;
	uint16_t length = AVRCP_BROWSING_HEADER_LENGTH + 10;
	uint32_t attribute;

	memset(buf, 0, sizeof(buf));

	pdu->pdu_id = AVRCP_GET_FOLDER_ITEMS;
	pdu->param_len = htons(10 + sizeof(uint32_t));

	pdu->params[0] = player->scope;

	put_be32(start, &pdu->params[1]);
	put_be32(end, &pdu->params[5]);

	pdu->params[9] = 1;

	/* Only the title (0x01) is mandatory. This can be extended to
	 * support AVRCP_MEDIA_ATTRIBUTE_* attributes */
	attribute = htonl(AVRCP_MEDIA_ATTRIBUTE_TITLE);
	memcpy(&pdu->params[10], &attribute, sizeof(uint32_t));

	length += sizeof(uint32_t);

	avctp_send_browsing_req(session->conn, buf, length,
					avrcp_list_items_rsp, session);
}

static gboolean avrcp_change_path_rsp(struct avctp *conn,
					uint8_t *operands, size_t operand_count,
					void *user_data)
{
	struct avrcp_browsing_header *pdu = (void *) operands;
	struct avrcp *session = user_data;
	struct avrcp_player *player = session->controller->player;
	struct media_player *mp = player->user_data;
	int ret;

	if (pdu == NULL) {
		ret = -ETIMEDOUT;
		goto done;
	}

	if (pdu->params[0] != AVRCP_STATUS_SUCCESS) {
		ret = -EINVAL;
		goto done;
	}

	ret = get_be32(&pdu->params[1]);

done:
	if (ret < 0) {
		g_free(player->change_path);
		player->change_path = NULL;
	} else {
		g_free(player->path);
		player->path = player->change_path;
		player->change_path = NULL;
	}

	media_player_change_folder_complete(mp, player->path, ret);

	return FALSE;
}

static gboolean avrcp_set_browsed_player_rsp(struct avctp *conn,
						uint8_t *operands,
						size_t operand_count,
						void *user_data)
{
	struct avrcp *session = user_data;
	struct avrcp_player *player = session->controller->player;
	struct media_player *mp = player->user_data;
	struct avrcp_browsing_header *pdu = (void *) operands;
	uint32_t items;
	char **folders;
	uint8_t depth, count;
	size_t i;

	if (pdu == NULL || pdu->params[0] != AVRCP_STATUS_SUCCESS ||
							operand_count < 13)
		return FALSE;

	player->uid_counter = get_be16(&pdu->params[1]);
	player->browsed = true;

	items = get_be32(&pdu->params[3]);

	depth = pdu->params[9];

	folders = g_new0(char *, depth + 2);
	folders[0] = g_strdup("/Filesystem");

	for (i = 10, count = 1; count - 1 < depth && i < operand_count;
								count++) {
		uint8_t len;

		len = pdu->params[i++];
		if (!len)
			continue;

		if (i + len > operand_count) {
			error("Invalid folder length");
			break;
		}

		folders[count] = g_memdup(&pdu->params[i], len);
		i += len;
	}

	player->path = g_build_pathv("/", folders);
	g_strfreev(folders);

	media_player_set_folder(mp, player->path, items);

	return FALSE;
}

static void avrcp_set_browsed_player(struct avrcp *session,
						struct avrcp_player *player)
{
	uint8_t buf[AVRCP_BROWSING_HEADER_LENGTH + 2];
	struct avrcp_browsing_header *pdu = (void *) buf;
	uint16_t id;

	memset(buf, 0, sizeof(buf));

	pdu->pdu_id = AVRCP_SET_BROWSED_PLAYER;
	id = htons(player->id);
	memcpy(pdu->params, &id, 2);
	pdu->param_len = htons(2);

	avctp_send_browsing_req(session->conn, buf, sizeof(buf),
				avrcp_set_browsed_player_rsp, session);
}

static gboolean avrcp_get_item_attributes_rsp(struct avctp *conn,
						uint8_t *operands,
						size_t operand_count,
						void *user_data)
{
	struct avrcp *session = user_data;
	struct avrcp_player *player = session->controller->player;
	struct avrcp_browsing_header *pdu = (void *) operands;
	uint8_t count;

	if (pdu == NULL) {
		avrcp_get_element_attributes(session);
		return FALSE;
	}

	if (pdu->params[0] != AVRCP_STATUS_SUCCESS || operand_count < 4) {
		avrcp_get_element_attributes(session);
		return FALSE;
	}

	count = pdu->params[1];

	if (ntohs(pdu->param_len) - 1 < count * 8) {
		error("Invalid parameters");
		return FALSE;
	}

	avrcp_parse_attribute_list(player, &pdu->params[2], count);

	avrcp_get_play_status(session);

	return FALSE;
}

static void avrcp_get_item_attributes(struct avrcp *session, uint64_t uid)
{
	struct avrcp_player *player = session->controller->player;
	uint8_t buf[AVRCP_BROWSING_HEADER_LENGTH + 12];
	struct avrcp_browsing_header *pdu = (void *) buf;

	memset(buf, 0, sizeof(buf));

	pdu->pdu_id = AVRCP_GET_ITEM_ATTRIBUTES;
	pdu->params[0] = 0x03;
	put_be64(uid, &pdu->params[1]);
	put_be16(player->uid_counter, &pdu->params[9]);
	pdu->param_len = htons(12);

	avctp_send_browsing_req(session->conn, buf, sizeof(buf),
				avrcp_get_item_attributes_rsp, session);
}

static void avrcp_player_parse_features(struct avrcp_player *player,
							uint8_t *features)
{
	struct media_player *mp = player->user_data;

	player->features = g_memdup(features, 16);

	if (features[7] & 0x08) {
		media_player_set_browsable(mp, true);
		media_player_create_folder(mp, "/Filesystem",
						PLAYER_FOLDER_TYPE_MIXED, 0);
	}

	if (features[7] & 0x10)
		media_player_set_searchable(mp, true);

	if (features[8] & 0x02) {
		media_player_create_folder(mp, "/NowPlaying",
						PLAYER_FOLDER_TYPE_MIXED, 0);
		media_player_set_playlist(mp, "/NowPlaying");
	}
}

static void avrcp_set_player_value(struct avrcp *session, uint8_t attr,
								uint8_t val)
{
	uint8_t buf[AVRCP_HEADER_LENGTH + 3];
	struct avrcp_header *pdu = (void *) buf;
	uint8_t length;

	memset(buf, 0, sizeof(buf));

	set_company_id(pdu->company_id, IEEEID_BTSIG);
	pdu->pdu_id = AVRCP_SET_PLAYER_VALUE;
	pdu->packet_type = AVRCP_PACKET_TYPE_SINGLE;
	pdu->params[0] = 1;
	pdu->params[1] = attr;
	pdu->params[2] = val;
	pdu->params_len = htons(3);

	length = AVRCP_HEADER_LENGTH + ntohs(pdu->params_len);

	avctp_send_vendordep_req(session->conn, AVC_CTYPE_CONTROL,
					AVC_SUBUNIT_PANEL, buf, length,
					avrcp_player_value_rsp, session);
}

static bool ct_set_setting(struct media_player *mp, const char *key,
					const char *value, void *user_data)
{
	struct avrcp_player *player = user_data;
	int attr;
	int val;
	struct avrcp *session;

	session = player->sessions->data;
	if (session == NULL)
		return false;

	if (session->controller->version < 0x0103)
		return false;

	attr = attr_to_val(key);
	if (attr < 0)
		return false;

	val = attrval_to_val(attr, value);
	if (val < 0)
		return false;

	avrcp_set_player_value(session, attr, val);

	return true;
}

static int ct_press(struct avrcp_player *player, uint8_t op)
{
	int err;
	struct avrcp *session;

	session = player->sessions->data;
	if (session == NULL)
		return -ENOTCONN;

	err = avctp_send_passthrough(session->conn, op);
	if (err < 0)
		return err;

	return 0;
}

static int ct_play(struct media_player *mp, void *user_data)
{
	struct avrcp_player *player = user_data;

	return ct_press(player, AVC_PLAY);
}

static int ct_pause(struct media_player *mp, void *user_data)
{
	struct avrcp_player *player = user_data;

	return ct_press(player, AVC_PAUSE);
}

static int ct_stop(struct media_player *mp, void *user_data)
{
	struct avrcp_player *player = user_data;

	return ct_press(player, AVC_STOP);
}

static int ct_next(struct media_player *mp, void *user_data)
{
	struct avrcp_player *player = user_data;

	return ct_press(player, AVC_FORWARD);
}

static int ct_previous(struct media_player *mp, void *user_data)
{
	struct avrcp_player *player = user_data;

	return ct_press(player, AVC_BACKWARD);
}

static int ct_fast_forward(struct media_player *mp, void *user_data)
{
	struct avrcp_player *player = user_data;

	return ct_press(player, AVC_FAST_FORWARD);
}

static int ct_rewind(struct media_player *mp, void *user_data)
{
	struct avrcp_player *player = user_data;

	return ct_press(player, AVC_REWIND);
}

static int ct_list_items(struct media_player *mp, const char *name,
				uint32_t start, uint32_t end, void *user_data)
{
	struct avrcp_player *player = user_data;
	struct avrcp *session;
	struct pending_list_items *p;

	if (player->p != NULL)
		return -EBUSY;

	session = player->sessions->data;

	if (g_str_has_prefix(name, "/NowPlaying"))
		player->scope = 0x03;
	else if (g_str_has_suffix(name, "/search"))
		player->scope = 0x02;
	else
		player->scope = 0x01;

	avrcp_list_items(session, start, end);

	p = g_new0(struct pending_list_items, 1);
	p->start = start;
	p->end = end;
	p->total = (uint64_t) (p->end - p->start) + 1;
	player->p = p;

	return 0;
}

static void avrcp_change_path(struct avrcp *session, uint8_t direction,
								uint64_t uid)
{
	struct avrcp_player *player = session->controller->player;
	uint8_t buf[AVRCP_BROWSING_HEADER_LENGTH + 11];
	struct avrcp_browsing_header *pdu = (void *) buf;

	memset(buf, 0, sizeof(buf));
	put_be16(player->uid_counter, &pdu->params[0]);
	pdu->params[2] = direction;
	put_be64(uid, &pdu->params[3]);
	pdu->pdu_id = AVRCP_CHANGE_PATH;
	pdu->param_len = htons(11);

	avctp_send_browsing_req(session->conn, buf, sizeof(buf),
					avrcp_change_path_rsp, session);
}

static int ct_change_folder(struct media_player *mp, const char *path,
					uint64_t uid, void *user_data)
{
	struct avrcp_player *player = user_data;
	struct avrcp *session;
	uint8_t direction;

	session = player->sessions->data;
	player->change_path = g_strdup(path);

	direction = g_str_has_prefix(path, player->path) ? 0x01 : 0x00;

	avrcp_change_path(session, direction, uid);

	return 0;
}

static gboolean avrcp_search_rsp(struct avctp *conn, uint8_t *operands,
					size_t operand_count, void *user_data)
{
	struct avrcp_browsing_header *pdu = (void *) operands;
	struct avrcp *session = (void *) user_data;
	struct avrcp_player *player = session->controller->player;
	struct media_player *mp = player->user_data;
	int ret;

	if (pdu == NULL) {
		ret = -ETIMEDOUT;
		goto done;
	}

	if (pdu->params[0] != AVRCP_STATUS_SUCCESS || operand_count < 7) {
		ret = -EINVAL;
		goto done;
	}

	player->uid_counter = get_be16(&pdu->params[1]);
	ret = get_be32(&pdu->params[3]);

done:
	media_player_search_complete(mp, ret);

	return FALSE;
}

static void avrcp_search(struct avrcp *session, const char *string)
{
	uint8_t buf[AVRCP_BROWSING_HEADER_LENGTH + 255];
	struct avrcp_browsing_header *pdu = (void *) buf;
	uint16_t len, stringlen;

	memset(buf, 0, sizeof(buf));
	len = AVRCP_BROWSING_HEADER_LENGTH + 4;
	stringlen = strnlen(string, sizeof(buf) - len);
	len += stringlen;

	put_be16(AVRCP_CHARSET_UTF8, &pdu->params[0]);
	put_be16(stringlen, &pdu->params[2]);
	memcpy(&pdu->params[4], string, stringlen);
	pdu->pdu_id = AVRCP_SEARCH;
	pdu->param_len = htons(len - AVRCP_BROWSING_HEADER_LENGTH);

	avctp_send_browsing_req(session->conn, buf, len, avrcp_search_rsp,
								session);
}

static int ct_search(struct media_player *mp, const char *string,
							void *user_data)
{
	struct avrcp_player *player = user_data;
	struct avrcp *session;

	session = player->sessions->data;

	avrcp_search(session, string);

	return 0;
}

static void avrcp_play_item(struct avrcp *session, uint64_t uid)
{
	uint8_t buf[AVRCP_HEADER_LENGTH + 11];
	struct avrcp_player *player = session->controller->player;
	struct avrcp_header *pdu = (void *) buf;
	uint16_t length;

	memset(buf, 0, sizeof(buf));

	set_company_id(pdu->company_id, IEEEID_BTSIG);
	pdu->pdu_id = AVRCP_PLAY_ITEM;
	pdu->params_len = htons(11);
	pdu->packet_type = AVRCP_PACKET_TYPE_SINGLE;

	pdu->params[0] = player->scope;
	put_be64(uid, &pdu->params[1]);
	put_be16(player->uid_counter, &pdu->params[9]);

	length = AVRCP_HEADER_LENGTH + ntohs(pdu->params_len);

	avctp_send_vendordep_req(session->conn, AVC_CTYPE_CONTROL,
					AVC_SUBUNIT_PANEL, buf, length,
					NULL, session);
}

static int ct_play_item(struct media_player *mp, const char *name,
						uint64_t uid, void *user_data)
{
	struct avrcp_player *player = user_data;
	struct avrcp *session;

	if (player->p != NULL)
		return -EBUSY;

	session = player->sessions->data;

	if (g_strrstr(name, "/NowPlaying"))
		player->scope = 0x03;
	else
		player->scope = 0x01;

	avrcp_play_item(session, uid);

	return 0;
}

static void avrcp_add_to_nowplaying(struct avrcp *session, uint64_t uid)
{
	uint8_t buf[AVRCP_HEADER_LENGTH + 11];
	struct avrcp_player *player = session->controller->player;
	struct avrcp_header *pdu = (void *) buf;
	uint16_t length;

	memset(buf, 0, sizeof(buf));

	set_company_id(pdu->company_id, IEEEID_BTSIG);
	pdu->pdu_id = AVRCP_ADD_TO_NOW_PLAYING;
	pdu->params_len = htons(11);
	pdu->packet_type = AVRCP_PACKET_TYPE_SINGLE;

	pdu->params[0] = player->scope;
	put_be64(uid, &pdu->params[1]);
	put_be16(player->uid_counter, &pdu->params[9]);

	length = AVRCP_HEADER_LENGTH + ntohs(pdu->params_len);

	avctp_send_vendordep_req(session->conn, AVC_CTYPE_CONTROL,
					AVC_SUBUNIT_PANEL, buf, length,
					NULL, session);
}

static int ct_add_to_nowplaying(struct media_player *mp, const char *name,
						uint64_t uid, void *user_data)
{
	struct avrcp_player *player = user_data;
	struct avrcp *session;

	if (player->p != NULL)
		return -EBUSY;

	session = player->sessions->data;

	if (g_strrstr(name, "/NowPlaying"))
		player->scope = 0x03;
	else
		player->scope = 0x01;

	avrcp_add_to_nowplaying(session, uid);

	return 0;
}

static gboolean avrcp_get_total_numberofitems_rsp(struct avctp *conn,
					uint8_t *operands, size_t operand_count,
					void *user_data)
{
	struct avrcp_browsing_header *pdu = (void *) operands;
	struct avrcp *session = user_data;
	struct avrcp_player *player = session->controller->player;
	struct media_player *mp = player->user_data;
	uint32_t num_of_items = 0;

	if (pdu == NULL)
		return -ETIMEDOUT;

	if (pdu->params[0] != AVRCP_STATUS_SUCCESS || operand_count < 7)
		return -EINVAL;

	if (pdu->params[0] == AVRCP_STATUS_OUT_OF_BOUNDS)
		goto done;

	player->uid_counter = get_be16(&pdu->params[1]);
	num_of_items = get_be32(&pdu->params[3]);

	if (!num_of_items)
		return -EINVAL;

done:
	media_player_total_items_complete(mp, num_of_items);
	return FALSE;
}

static void avrcp_get_total_numberofitems(struct avrcp *session)
{
	uint8_t buf[AVRCP_BROWSING_HEADER_LENGTH + 7];
	struct avrcp_player *player = session->controller->player;
	struct avrcp_browsing_header *pdu = (void *) buf;

	memset(buf, 0, sizeof(buf));

	pdu->pdu_id = AVRCP_GET_TOTAL_NUMBER_OF_ITEMS;
	pdu->param_len = htons(7 + sizeof(uint32_t));

	pdu->params[0] = player->scope;

	avctp_send_browsing_req(session->conn, buf, sizeof(buf),
				avrcp_get_total_numberofitems_rsp, session);
}

static int ct_get_total_numberofitems(struct media_player *mp, const char *name,
						void *user_data)
{
	struct avrcp_player *player = user_data;
	struct avrcp *session;

	session = player->sessions->data;

	if (session->controller->version != 0x0106) {
		error("version not supported");
		return -1;
	}

	if (g_str_has_prefix(name, "/NowPlaying"))
		player->scope = 0x03;
	else if (g_str_has_suffix(name, "/search"))
		player->scope = 0x02;
	else
		player->scope = 0x01;

	avrcp_get_total_numberofitems(session);

	return 0;
}

static const struct media_player_callback ct_cbs = {
	.set_setting	= ct_set_setting,
	.play		= ct_play,
	.pause		= ct_pause,
	.stop		= ct_stop,
	.next		= ct_next,
	.previous	= ct_previous,
	.fast_forward	= ct_fast_forward,
	.rewind		= ct_rewind,
	.list_items	= ct_list_items,
	.change_folder	= ct_change_folder,
	.search		= ct_search,
	.play_item	= ct_play_item,
	.add_to_nowplaying = ct_add_to_nowplaying,
	.total_items = ct_get_total_numberofitems,
};

static void set_ct_player(struct avrcp *session, struct avrcp_player *player)
{
	struct btd_service *service;

	session->controller->player = player;
	service = btd_device_get_service(session->dev, AVRCP_TARGET_UUID);
	control_set_player(service, player ?
			media_player_get_path(player->user_data) : NULL);
}

static struct avrcp_player *create_ct_player(struct avrcp *session,
								uint16_t id)
{
	struct avrcp_player *player;
	struct media_player *mp;
	const char *path;

	player = g_new0(struct avrcp_player, 1);
	player->sessions = g_slist_prepend(player->sessions, session);

	path = device_get_path(session->dev);

	mp = media_player_controller_create(path, id);
	if (mp == NULL)
		return NULL;

	media_player_set_callbacks(mp, &ct_cbs, player);
	player->user_data = mp;
	player->destroy = (GDestroyNotify) media_player_destroy;

	if (session->controller->player == NULL)
		set_ct_player(session, player);

	session->controller->players = g_slist_prepend(
						session->controller->players,
						player);

	return player;
}

static struct avrcp_player *find_ct_player(struct avrcp *session, uint16_t id)
{
	GSList *l;

	for (l = session->controller->players; l; l = l->next) {
		struct avrcp_player *player = l->data;

		if (player->id == 0) {
			player->id = id;
			return player;
		}

		if (player->id == id)
			return player;
	}

	return NULL;
}

static struct avrcp_player *
avrcp_parse_media_player_item(struct avrcp *session, uint8_t *operands,
							uint16_t len)
{
	struct avrcp_player *player;
	struct media_player *mp;
	uint16_t id, namelen;
	uint32_t subtype;
	const char *curval, *strval;
	char name[255];

	if (len < 28)
		return NULL;

	id = get_be16(&operands[0]);

	player = find_ct_player(session, id);
	if (player == NULL) {
		player = create_ct_player(session, id);
		if (player == NULL)
			return NULL;
	} else if (player->features != NULL)
		return player;

	mp = player->user_data;

	media_player_set_type(mp, type_to_string(operands[2]));

	subtype = get_be32(&operands[3]);

	media_player_set_subtype(mp, subtype_to_string(subtype));

	curval = media_player_get_status(mp);
	strval = status_to_string(operands[7]);

	if (g_strcmp0(curval, strval) != 0) {
		media_player_set_status(mp, strval);
		avrcp_get_play_status(session);
	}

	avrcp_player_parse_features(player, &operands[8]);

	namelen = get_be16(&operands[26]);
	if (namelen > 0 && namelen + 28 == len) {
		namelen = MIN(namelen, sizeof(name) - 1);
		memcpy(name, &operands[28], namelen);
		name[namelen] = '\0';
		media_player_set_name(mp, name);
	}

	if (session->controller->player == player && !player->browsed)
		avrcp_set_browsed_player(session, player);

	return player;
}

static void player_destroy(gpointer data)
{
	struct avrcp_player *player = data;

	if (player->destroy)
		player->destroy(player->user_data);

	if (player->changed_id > 0)
		g_source_remove(player->changed_id);

	g_slist_free(player->sessions);
	g_free(player->path);
	g_free(player->change_path);
	g_free(player->features);
	g_free(player);
}

static void player_remove(gpointer data)
{
	struct avrcp_player *player = data;
	GSList *l;

	/* Don't remove reserved player */
	if (!player->id)
		return;

	for (l = player->sessions; l; l = l->next) {
		struct avrcp *session = l->data;
		struct avrcp_data *controller = session->controller;

		controller->players = g_slist_remove(controller->players,
								player);

		/* Check if current player is being removed */
		if (controller->player == player)
			set_ct_player(session, g_slist_nth_data(
						controller->players, 0));
	}

	player_destroy(player);
}

static gboolean avrcp_get_media_player_list_rsp(struct avctp *conn,
						uint8_t *operands,
						size_t operand_count,
						void *user_data)
{
	struct avrcp_browsing_header *pdu = (void *) operands;
	struct avrcp *session = user_data;
	uint16_t count;
	size_t i;
	GSList *removed;

	if (pdu == NULL || pdu->params[0] != AVRCP_STATUS_SUCCESS ||
							operand_count < 5)
		return FALSE;

	removed = g_slist_copy(session->controller->players);
	count = get_be16(&operands[6]);

	for (i = 8; count && i < operand_count; count--) {
		struct avrcp_player *player;
		uint8_t type;
		uint16_t len;

		type = operands[i++];
		len = get_be16(&operands[i]);
		i += 2;

		if (type != 0x01) {
			i += len;
			continue;
		}

		if (i + len > operand_count) {
			error("Invalid player item length");
			return FALSE;
		}

		player = avrcp_parse_media_player_item(session, &operands[i],
									len);
		if (player)
			removed = g_slist_remove(removed, player);

		i += len;
	}

	g_slist_free_full(removed, player_remove);

	/* There should always be an active player */
	if (!session->controller->player)
		create_ct_player(session, 0);

	return FALSE;
}

static void avrcp_get_media_player_list(struct avrcp *session)
{
	uint8_t buf[AVRCP_BROWSING_HEADER_LENGTH + 10];
	struct avrcp_browsing_header *pdu = (void *) buf;

	memset(buf, 0, sizeof(buf));

	pdu->pdu_id = AVRCP_GET_FOLDER_ITEMS;
	put_be32(0, &pdu->params[1]);
	put_be32(UINT32_MAX, &pdu->params[5]);
	pdu->param_len = htons(10);

	avctp_send_browsing_req(session->conn, buf, sizeof(buf),
				avrcp_get_media_player_list_rsp, session);
}

static void avrcp_volume_changed(struct avrcp *session,
						struct avrcp_header *pdu)
{
	struct avrcp_player *player = target_get_player(session);
	uint8_t volume;

	if (!player)
		return;

	volume = pdu->params[1] & 0x7F;

	player->cb->set_volume(volume, session->dev, player->user_data);
}

static void avrcp_status_changed(struct avrcp *session,
						struct avrcp_header *pdu)
{
	struct avrcp_player *player = session->controller->player;
	struct media_player *mp = player->user_data;
	uint8_t value;
	const char *curval, *strval;

	value = pdu->params[1];

	curval = media_player_get_status(mp);
	strval = status_to_string(value);

	if (g_strcmp0(curval, strval) == 0)
		return;

	media_player_set_status(mp, strval);
	avrcp_get_play_status(session);
}

static void avrcp_track_changed(struct avrcp *session,
						struct avrcp_header *pdu)
{
	if (session->browsing_id) {
		struct avrcp_player *player = session->controller->player;
		player->uid = get_be64(&pdu->params[1]);
		avrcp_get_item_attributes(session, player->uid);
	} else
		avrcp_get_element_attributes(session);
}

static void avrcp_playback_pos_changed(struct avrcp *session,
						struct avrcp_header *pdu)
{
	struct avrcp_player *player = session->controller->player;
	struct media_player *mp = player->user_data;
	uint32_t position;

	position = get_be32(&pdu->params[1]);
	media_player_set_position(mp, position);
}

static void avrcp_setting_changed(struct avrcp *session,
						struct avrcp_header *pdu)
{
	struct avrcp_player *player = session->controller->player;
	struct media_player *mp = player->user_data;
	uint8_t count = pdu->params[1];
	int i;

	for (i = 2; count > 0; count--, i += 2) {
		const char *key;
		const char *value;

		key = attr_to_str(pdu->params[i]);
		if (key == NULL)
			continue;

		value = attrval_to_str(pdu->params[i], pdu->params[i + 1]);
		if (value == NULL)
			continue;

		media_player_set_setting(mp, key, value);
	}
}

static void avrcp_available_players_changed(struct avrcp *session,
						struct avrcp_header *pdu)
{
	avrcp_get_media_player_list(session);
}

static void avrcp_addressed_player_changed(struct avrcp *session,
						struct avrcp_header *pdu)
{
	struct avrcp_player *player = session->controller->player;
	uint16_t id = get_be16(&pdu->params[1]);

	if (player != NULL && player->id == id)
		return;

	player = find_ct_player(session, id);
	if (player == NULL) {
		player = create_ct_player(session, id);
		if (player == NULL)
			return;
	}

	player->uid_counter = get_be16(&pdu->params[3]);
	set_ct_player(session, player);

	if (player->features != NULL)
		return;

	avrcp_get_media_player_list(session);
}

static void avrcp_uids_changed(struct avrcp *session, struct avrcp_header *pdu)
{
	struct avrcp_player *player = session->controller->player;

	player->uid_counter = get_be16(&pdu->params[1]);
}

static gboolean avrcp_handle_event(struct avctp *conn, uint8_t code,
					uint8_t subunit, uint8_t transaction,
					uint8_t *operands, size_t operand_count,
					void *user_data)
{
	struct avrcp *session = user_data;
	struct avrcp_header *pdu = (void *) operands;
	uint8_t event;

	if (!pdu)
		return FALSE;

	if ((code != AVC_CTYPE_INTERIM && code != AVC_CTYPE_CHANGED)) {
		if (pdu->params[0] == AVRCP_STATUS_ADDRESSED_PLAYER_CHANGED &&
				code == AVC_CTYPE_REJECTED) {
			int i;

			/* Lookup event by transaction */
			for (i = 0; i <= AVRCP_EVENT_LAST; i++) {
				if (session->transaction_events[i] ==
								transaction) {
					event = i;
					goto changed;
				}
			}
		}
		return FALSE;
	}

	event = pdu->params[0];

	if (code == AVC_CTYPE_CHANGED) {
		goto changed;
	}

	switch (event) {
	case AVRCP_EVENT_VOLUME_CHANGED:
		avrcp_volume_changed(session, pdu);
		break;
	case AVRCP_EVENT_STATUS_CHANGED:
		avrcp_status_changed(session, pdu);
		break;
	case AVRCP_EVENT_TRACK_CHANGED:
		avrcp_track_changed(session, pdu);
		break;
	case AVRCP_EVENT_PLAYBACK_POS_CHANGED:
		avrcp_playback_pos_changed(session, pdu);
		break;
	case AVRCP_EVENT_SETTINGS_CHANGED:
		avrcp_setting_changed(session, pdu);
		break;
	case AVRCP_EVENT_AVAILABLE_PLAYERS_CHANGED:
		avrcp_available_players_changed(session, pdu);
		break;
	case AVRCP_EVENT_ADDRESSED_PLAYER_CHANGED:
		avrcp_addressed_player_changed(session, pdu);
		break;
	case AVRCP_EVENT_UIDS_CHANGED:
		avrcp_uids_changed(session, pdu);
		break;
	}

	session->registered_events |= (1 << event);
	session->transaction_events[event] = transaction;

	return TRUE;

changed:
	session->registered_events ^= (1 << event);
	session->transaction_events[event] = 0;
	avrcp_register_notification(session, event);
	return FALSE;
}

static void avrcp_register_notification(struct avrcp *session, uint8_t event)
{
	uint8_t buf[AVRCP_HEADER_LENGTH + AVRCP_REGISTER_NOTIFICATION_PARAM_LENGTH];
	struct avrcp_header *pdu = (void *) buf;
	uint8_t length;

	memset(buf, 0, sizeof(buf));

	set_company_id(pdu->company_id, IEEEID_BTSIG);
	pdu->pdu_id = AVRCP_REGISTER_NOTIFICATION;
	pdu->packet_type = AVRCP_PACKET_TYPE_SINGLE;
	pdu->params[0] = event;

	/*
	 * Set maximum interval possible for position changed as we only
	 * use it to resync.
	 */
	if (event == AVRCP_EVENT_PLAYBACK_POS_CHANGED)
		bt_put_be32(UINT32_MAX / 1000, &pdu->params[1]);

	pdu->params_len = htons(AVRCP_REGISTER_NOTIFICATION_PARAM_LENGTH);

	length = AVRCP_HEADER_LENGTH + ntohs(pdu->params_len);

	avctp_send_vendordep_req(session->conn, AVC_CTYPE_NOTIFY,
					AVC_SUBUNIT_PANEL, buf, length,
					avrcp_handle_event, session);
}

static gboolean avrcp_get_capabilities_resp(struct avctp *conn, uint8_t code,
					uint8_t subunit, uint8_t transaction,
					uint8_t *operands, size_t operand_count,
					void *user_data)
{
	struct avrcp *session = user_data;
	struct avrcp_header *pdu = (void *) operands;
	uint16_t events = 0;
	uint8_t count;

	if (code == AVC_CTYPE_REJECTED || code == AVC_CTYPE_NOT_IMPLEMENTED ||
			pdu == NULL || pdu->params[0] != CAP_EVENTS_SUPPORTED)
		return FALSE;

	/* Connect browsing if pending */
	if (session->browsing_timer > 0) {
		g_source_remove(session->browsing_timer);
		session->browsing_timer = 0;
		avctp_connect_browsing(session->conn);
	}

	count = pdu->params[1];

	for (; count > 0; count--) {
		uint8_t event = pdu->params[1 + count];

		events |= (1 << event);

		switch (event) {
		case AVRCP_EVENT_STATUS_CHANGED:
		case AVRCP_EVENT_TRACK_CHANGED:
		case AVRCP_EVENT_PLAYBACK_POS_CHANGED:
		case AVRCP_EVENT_SETTINGS_CHANGED:
		case AVRCP_EVENT_ADDRESSED_PLAYER_CHANGED:
		case AVRCP_EVENT_UIDS_CHANGED:
		case AVRCP_EVENT_AVAILABLE_PLAYERS_CHANGED:
			/* These events above requires a player */
			if (!session->controller ||
						!session->controller->player)
				break;
		case AVRCP_EVENT_VOLUME_CHANGED:
			avrcp_register_notification(session, event);
			break;
		}
	}

	if (!session->controller || !session->controller->player)
		return FALSE;

	if (!(events & (1 << AVRCP_EVENT_SETTINGS_CHANGED)))
		avrcp_list_player_attributes(session);

	if (!(events & (1 << AVRCP_EVENT_STATUS_CHANGED)))
		avrcp_get_play_status(session);

	if (!(events & (1 << AVRCP_EVENT_STATUS_CHANGED)))
		avrcp_get_element_attributes(session);

	return FALSE;
}

static void avrcp_get_capabilities(struct avrcp *session)
{
	uint8_t buf[AVRCP_HEADER_LENGTH + AVRCP_GET_CAPABILITIES_PARAM_LENGTH];
	struct avrcp_header *pdu = (void *) buf;
	uint8_t length;

	memset(buf, 0, sizeof(buf));

	set_company_id(pdu->company_id, IEEEID_BTSIG);
	pdu->pdu_id = AVRCP_GET_CAPABILITIES;
	pdu->packet_type = AVRCP_PACKET_TYPE_SINGLE;
	pdu->params[0] = CAP_EVENTS_SUPPORTED;
	pdu->params_len = htons(AVRCP_GET_CAPABILITIES_PARAM_LENGTH);

	length = AVRCP_HEADER_LENGTH + ntohs(pdu->params_len);

	avctp_send_vendordep_req(session->conn, AVC_CTYPE_STATUS,
					AVC_SUBUNIT_PANEL, buf, length,
					avrcp_get_capabilities_resp,
					session);
}

static struct avrcp *find_session(GSList *list, struct btd_device *dev)
{
	for (; list; list = list->next) {
		struct avrcp *session = list->data;

		if (session->dev == dev)
			return session;
	}

	return NULL;
}

static void destroy_browsing(void *data)
{
	struct avrcp *session = data;

	session->browsing_id = 0;
}

static void session_init_browsing(struct avrcp *session)
{
	if (session->browsing_timer > 0) {
		g_source_remove(session->browsing_timer);
		session->browsing_timer = 0;
	}

	session->browsing_id = avctp_register_browsing_pdu_handler(
							session->conn,
							handle_browsing_pdu,
							session,
							destroy_browsing);
}

static struct avrcp_data *data_init(struct avrcp *session, const char *uuid)
{
	struct avrcp_data *data;
	const sdp_record_t *rec;
	sdp_list_t *list;
	sdp_profile_desc_t *desc;

	data = g_new0(struct avrcp_data, 1);

	rec = btd_device_get_record(session->dev, uuid);
	if (rec == NULL)
		return data;

	if (sdp_get_profile_descs(rec, &list) == 0) {
		desc = list->data;
		data->version = desc->version;
	}

	sdp_get_int_attr(rec, SDP_ATTR_SUPPORTED_FEATURES, &data->features);
	sdp_list_free(list, free);

	return data;
}

static gboolean connect_browsing(gpointer user_data)
{
	struct avrcp *session = user_data;

	session->browsing_timer = 0;

	avctp_connect_browsing(session->conn);

	return FALSE;
}

static void avrcp_connect_browsing(struct avrcp *session)
{
	/* Immediately connect browsing channel if initiator otherwise delay
	 * it to avoid possible collisions
	 */
	if (avctp_is_initiator(session->conn)) {
		avctp_connect_browsing(session->conn);
		return;
	}

	if (session->browsing_timer > 0)
		return;

	session->browsing_timer = g_timeout_add_seconds(AVRCP_BROWSING_TIMEOUT,
							connect_browsing,
							session);
}

static void target_init(struct avrcp *session)
{
	struct avrcp_server *server = session->server;
	struct avrcp_data *target;
	struct avrcp_player *player;
	struct btd_service *service;

	if (session->target != NULL)
		return;

	target = data_init(session, AVRCP_REMOTE_UUID);
	session->target = target;

	DBG("%p version 0x%04x", target, target->version);

	service = btd_device_get_service(session->dev, AVRCP_REMOTE_UUID);
	btd_service_connecting_complete(service, 0);

	player = g_slist_nth_data(server->players, 0);
	if (player != NULL) {
		target->player = player;
		player->sessions = g_slist_prepend(player->sessions, session);
	}

	session->supported_events |= (1 << AVRCP_EVENT_STATUS_CHANGED) |
				(1 << AVRCP_EVENT_TRACK_CHANGED) |
				(1 << AVRCP_EVENT_TRACK_REACHED_START) |
				(1 << AVRCP_EVENT_TRACK_REACHED_END) |
				(1 << AVRCP_EVENT_SETTINGS_CHANGED);

	if (target->version < 0x0104)
		return;

	session->supported_events |=
				(1 << AVRCP_EVENT_ADDRESSED_PLAYER_CHANGED) |
				(1 << AVRCP_EVENT_AVAILABLE_PLAYERS_CHANGED) |
				(1 << AVRCP_EVENT_VOLUME_CHANGED);

	/* Only check capabilities if controller is not supported */
	if (session->controller == NULL)
		avrcp_get_capabilities(session);

	if (!(target->features & AVRCP_FEATURE_BROWSING))
		return;

	avrcp_connect_browsing(session);
}

static void controller_init(struct avrcp *session)
{
	struct avrcp_player *player;
	struct btd_service *service;
	struct avrcp_data *controller;

	if (session->controller != NULL)
		return;

	controller = data_init(session, AVRCP_TARGET_UUID);
	session->controller = controller;

	DBG("%p version 0x%04x", controller, controller->version);

	service = btd_device_get_service(session->dev, AVRCP_TARGET_UUID);
	btd_service_connecting_complete(service, 0);

	/* Only create player if category 1 is supported */
	if (controller->features & AVRCP_FEATURE_CATEGORY_1) {
		player = create_ct_player(session, 0);
		if (player == NULL)
			return;
	}

	if (controller->version < 0x0103)
		return;

	avrcp_get_capabilities(session);

	if (controller->version < 0x0104)
		return;

	if (!(controller->features & AVRCP_FEATURE_BROWSING))
		return;

	avrcp_connect_browsing(session);
}

static void session_init_control(struct avrcp *session)
{
	session->passthrough_id = avctp_register_passthrough_handler(
							session->conn,
							handle_passthrough,
							session);
	session->passthrough_handlers = passthrough_handlers;
	session->control_id = avctp_register_pdu_handler(session->conn,
							AVC_OP_VENDORDEP,
							handle_vendordep_pdu,
							session);
	session->control_handlers = control_handlers;

	if (btd_device_get_service(session->dev, AVRCP_TARGET_UUID) != NULL)
		controller_init(session);

	if (btd_device_get_service(session->dev, AVRCP_REMOTE_UUID) != NULL)
		target_init(session);
}

static void controller_destroy(struct avrcp *session)
{
	struct avrcp_data *controller = session->controller;

	DBG("%p", controller);

	g_slist_free_full(controller->players, player_destroy);

	g_free(controller);
}

static void target_destroy(struct avrcp *session)
{
	struct avrcp_data *target = session->target;
	struct avrcp_player *player = target->player;

	DBG("%p", target);

	if (player != NULL)
		player->sessions = g_slist_remove(player->sessions, session);

	g_free(target);
}

static void session_destroy(struct avrcp *session, int err)
{
	struct avrcp_server *server = session->server;
	struct btd_service *service;

	server->sessions = g_slist_remove(server->sessions, session);

	session_abort_pending_pdu(session);

	service = btd_device_get_service(session->dev, AVRCP_TARGET_UUID);
	if (service != NULL) {
		if (session->control_id == 0)
			btd_service_connecting_complete(service, err);
		else
			btd_service_disconnecting_complete(service, 0);
	}

	service = btd_device_get_service(session->dev, AVRCP_REMOTE_UUID);
	if (service != NULL) {
		if (session->control_id == 0)
			btd_service_connecting_complete(service, err);
		else
			btd_service_disconnecting_complete(service, 0);
	}

	if (session->browsing_timer > 0)
		g_source_remove(session->browsing_timer);

	if (session->controller != NULL)
		controller_destroy(session);

	if (session->target != NULL)
		target_destroy(session);

	if (session->passthrough_id > 0)
		avctp_unregister_passthrough_handler(session->passthrough_id);

	if (session->control_id > 0)
		avctp_unregister_pdu_handler(session->control_id);

	if (session->browsing_id > 0)
		avctp_unregister_browsing_pdu_handler(session->browsing_id);

	g_free(session);
}

static struct avrcp *session_create(struct avrcp_server *server,
						struct btd_device *device)
{
	struct avrcp *session;

	session = g_new0(struct avrcp, 1);
	session->server = server;
	session->conn = avctp_connect(device);
	session->dev = device;

	server->sessions = g_slist_append(server->sessions, session);

	return session;
}

static void state_changed(struct btd_device *device, avctp_state_t old_state,
					avctp_state_t new_state, int err,
					void *user_data)
{
	struct avrcp_server *server;
	struct avrcp *session;

	server = find_server(servers, device_get_adapter(device));
	if (!server)
		return;

	session = find_session(server->sessions, device);

	switch (new_state) {
	case AVCTP_STATE_DISCONNECTED:
		if (session == NULL)
			break;

		session_destroy(session, err);

		break;
	case AVCTP_STATE_CONNECTING:
		if (session != NULL)
			break;

		session_create(server, device);

		break;
	case AVCTP_STATE_CONNECTED:
		if (session == NULL || session->control_id > 0)
			break;

		session_init_control(session);

		break;
	case AVCTP_STATE_BROWSING_CONNECTED:
		if (session == NULL || session->browsing_id > 0)
			break;

		session_init_browsing(session);

		break;
	case AVCTP_STATE_BROWSING_CONNECTING:
	default:
		return;
	}
}

static struct avrcp_server *avrcp_server_register(struct btd_adapter *adapter)
{
	struct avrcp_server *server;

	if (avctp_register(adapter, TRUE) < 0)
		return NULL;

	server = g_new0(struct avrcp_server, 1);
	server->adapter = btd_adapter_ref(adapter);

	servers = g_slist_append(servers, server);

	if (!avctp_id)
		avctp_id = avctp_add_state_cb(NULL, state_changed, NULL);

	return server;
}

static void avrcp_server_unregister(struct avrcp_server *server)
{
	g_slist_free_full(server->sessions, g_free);
	g_slist_free_full(server->players, player_destroy);

	servers = g_slist_remove(servers, server);

	avctp_unregister(server->adapter);
	btd_adapter_unref(server->adapter);
	g_free(server);

	if (servers)
		return;

	if (avctp_id) {
		avctp_remove_state_cb(avctp_id);
		avctp_id = 0;
	}
}

struct avrcp_player *avrcp_register_player(struct btd_adapter *adapter,
						struct avrcp_player_cb *cb,
						void *user_data,
						GDestroyNotify destroy)
{
	struct avrcp_server *server;
	struct avrcp_player *player;
	GSList *l;
	static uint16_t id = 0;

	server = find_server(servers, adapter);
	if (!server)
		return NULL;

	player = g_new0(struct avrcp_player, 1);
	player->id = ++id;
	player->server = server;
	player->cb = cb;
	player->user_data = user_data;
	player->destroy = destroy;

	server->players = g_slist_append(server->players, player);

	/* Assign player to session without current player */
	for (l = server->sessions; l; l = l->next) {
		struct avrcp *session = l->data;
		struct avrcp_data *target = session->target;

		if (target == NULL)
			continue;

		if (target->player == NULL) {
			target->player = player;
			player->sessions = g_slist_append(player->sessions,
								session);
		}
	}

	avrcp_player_event(player,
				AVRCP_EVENT_AVAILABLE_PLAYERS_CHANGED, NULL);

	return player;
}

void avrcp_unregister_player(struct avrcp_player *player)
{
	struct avrcp_server *server = player->server;
	GSList *l;

	server->players = g_slist_remove(server->players, player);

	/* Remove player from sessions using it */
	for (l = player->sessions; l; l = l->next) {
		struct avrcp *session = l->data;
		struct avrcp_data *target = session->target;

		if (target == NULL)
			continue;

		if (target->player == player)
			target->player = g_slist_nth_data(server->players, 0);
	}

	avrcp_player_event(player,
				AVRCP_EVENT_AVAILABLE_PLAYERS_CHANGED, NULL);

	player_destroy(player);
}

static gboolean avrcp_handle_set_volume(struct avctp *conn, uint8_t code,
					uint8_t subunit, uint8_t transaction,
					uint8_t *operands, size_t operand_count,
					void *user_data)
{
	struct avrcp *session = user_data;
	struct avrcp_player *player = target_get_player(session);
	struct avrcp_header *pdu = (void *) operands;
	uint8_t volume;

	if (code == AVC_CTYPE_REJECTED || code == AVC_CTYPE_NOT_IMPLEMENTED ||
								pdu == NULL)
		return FALSE;

	volume = pdu->params[0] & 0x7F;

	if (player != NULL)
		player->cb->set_volume(volume, session->dev, player->user_data);

	return FALSE;
}

static int avrcp_event(struct avrcp *session, uint8_t id, const void *data)
{
	uint8_t buf[AVRCP_HEADER_LENGTH + 2];
	struct avrcp_header *pdu = (void *) buf;
	uint8_t code;
	uint16_t size;
	int err;

	/* Verify that the event is registered */
	if (!(session->registered_events & (1 << id)))
		return -ENOENT;

	memset(buf, 0, sizeof(buf));

	set_company_id(pdu->company_id, IEEEID_BTSIG);
	pdu->pdu_id = AVRCP_REGISTER_NOTIFICATION;
	code = AVC_CTYPE_CHANGED;
	pdu->params[0] = id;

	DBG("id=%u", id);

	switch (id) {
	case AVRCP_EVENT_VOLUME_CHANGED:
		size = 2;
		memcpy(&pdu->params[1], data, sizeof(uint8_t));
		break;
	default:
		error("Unknown event %u", id);
		return -EINVAL;
	}

	pdu->params_len = htons(size);

	err = avctp_send_vendordep(session->conn,
					session->transaction_events[id],
					code, AVC_SUBUNIT_PANEL,
					buf, size + AVRCP_HEADER_LENGTH);
	if (err < 0)
		return err;

	/* Unregister event as per AVRCP 1.3 spec, section 5.4.2 */
	session->registered_events ^= 1 << id;

	return err;
}

int avrcp_set_volume(struct btd_device *dev, uint8_t volume, bool notify)
{
	struct avrcp_server *server;
	struct avrcp *session;
	uint8_t buf[AVRCP_HEADER_LENGTH + 1];
	struct avrcp_header *pdu = (void *) buf;

	server = find_server(servers, device_get_adapter(dev));
	if (server == NULL)
		return -EINVAL;

	session = find_session(server->sessions, dev);
	if (session == NULL)
		return -ENOTCONN;

	if (notify) {
		if (!session->target)
			return -ENOTSUP;
		return avrcp_event(session, AVRCP_EVENT_VOLUME_CHANGED,
								&volume);
	}

	if (!session->controller || session->controller->version < 0x0104)
		return -ENOTSUP;

	memset(buf, 0, sizeof(buf));

	set_company_id(pdu->company_id, IEEEID_BTSIG);

	pdu->pdu_id = AVRCP_SET_ABSOLUTE_VOLUME;
	pdu->params[0] = volume;
	pdu->params_len = htons(1);

	return avctp_send_vendordep_req(session->conn,
					AVC_CTYPE_CONTROL, AVC_SUBUNIT_PANEL,
					buf, sizeof(buf),
					avrcp_handle_set_volume, session);
}

static int avrcp_connect(struct btd_service *service)
{
	struct btd_device *dev = btd_service_get_device(service);
	const char *path = device_get_path(dev);

	DBG("path %s", path);

	return control_connect(service);
}

static int avrcp_disconnect(struct btd_service *service)
{
	struct btd_device *dev = btd_service_get_device(service);
	const char *path = device_get_path(dev);

	DBG("path %s", path);

	return control_disconnect(service);
}

static int avrcp_target_probe(struct btd_service *service)
{
	struct btd_device *dev = btd_service_get_device(service);

	DBG("path %s", device_get_path(dev));

	return control_init_target(service);
}

static void avrcp_target_remove(struct btd_service *service)
{
	control_unregister(service);
}

static void avrcp_target_server_remove(struct btd_profile *p,
						struct btd_adapter *adapter)
{
	struct avrcp_server *server;

	DBG("path %s", adapter_get_path(adapter));

	server = find_server(servers, adapter);
	if (!server)
		return;

	if (server->tg_record_id != 0) {
		adapter_service_remove(adapter, server->tg_record_id);
		server->tg_record_id = 0;
	}

	if (server->ct_record_id == 0)
		avrcp_server_unregister(server);
}

static int avrcp_target_server_probe(struct btd_profile *p,
						struct btd_adapter *adapter)
{
	sdp_record_t *record;
	struct avrcp_server *server;

	DBG("path %s", adapter_get_path(adapter));

	server = find_server(servers, adapter);
	if (server != NULL)
		goto done;

	server = avrcp_server_register(adapter);
	if (server == NULL)
		return -EPROTONOSUPPORT;

done:
	record = avrcp_tg_record();
	if (!record) {
		error("Unable to allocate new service record");
		avrcp_target_server_remove(p, adapter);
		return -1;
	}

	if (adapter_service_add(adapter, record) < 0) {
		error("Unable to register AVRCP target service record");
		avrcp_target_server_remove(p, adapter);
		sdp_record_free(record);
		return -1;
	}
	server->tg_record_id = record->handle;

	return 0;
}

static struct btd_profile avrcp_target_profile = {
	.name		= "audio-avrcp-target",

	.remote_uuid	= AVRCP_TARGET_UUID,
	.device_probe	= avrcp_target_probe,
	.device_remove	= avrcp_target_remove,

	.connect	= avrcp_connect,
	.disconnect	= avrcp_disconnect,

	.adapter_probe	= avrcp_target_server_probe,
	.adapter_remove = avrcp_target_server_remove,
};

static int avrcp_controller_probe(struct btd_service *service)
{
	struct btd_device *dev = btd_service_get_device(service);

	DBG("path %s", device_get_path(dev));

	return control_init_remote(service);
}

static void avrcp_controller_remove(struct btd_service *service)
{
	control_unregister(service);
}

static void avrcp_controller_server_remove(struct btd_profile *p,
						struct btd_adapter *adapter)
{
	struct avrcp_server *server;

	DBG("path %s", adapter_get_path(adapter));

	server = find_server(servers, adapter);
	if (!server)
		return;

	if (server->ct_record_id != 0) {
		adapter_service_remove(adapter, server->ct_record_id);
		server->ct_record_id = 0;
	}

	if (server->tg_record_id == 0)
		avrcp_server_unregister(server);
}

static int avrcp_controller_server_probe(struct btd_profile *p,
						struct btd_adapter *adapter)
{
	sdp_record_t *record;
	struct avrcp_server *server;

	DBG("path %s", adapter_get_path(adapter));

	server = find_server(servers, adapter);
	if (server != NULL)
		goto done;

	server = avrcp_server_register(adapter);
	if (server == NULL)
		return -EPROTONOSUPPORT;

done:
	record = avrcp_ct_record();
	if (!record) {
		error("Unable to allocate new service record");
		avrcp_controller_server_remove(p, adapter);
		return -1;
	}

	if (adapter_service_add(adapter, record) < 0) {
		error("Unable to register AVRCP service record");
		avrcp_controller_server_remove(p, adapter);
		sdp_record_free(record);
		return -1;
	}
	server->ct_record_id = record->handle;

	return 0;
}

static struct btd_profile avrcp_controller_profile = {
	.name		= "avrcp-controller",

	.remote_uuid	= AVRCP_REMOTE_UUID,
	.device_probe	= avrcp_controller_probe,
	.device_remove	= avrcp_controller_remove,

	.connect	= avrcp_connect,
	.disconnect	= avrcp_disconnect,

	.adapter_probe	= avrcp_controller_server_probe,
	.adapter_remove = avrcp_controller_server_remove,
};

static int avrcp_init(void)
{
	btd_profile_register(&avrcp_controller_profile);
	btd_profile_register(&avrcp_target_profile);

	return 0;
}

static void avrcp_exit(void)
{
	btd_profile_unregister(&avrcp_controller_profile);
	btd_profile_unregister(&avrcp_target_profile);
}

BLUETOOTH_PLUGIN_DEFINE(avrcp, VERSION, BLUETOOTH_PLUGIN_PRIORITY_DEFAULT,
							avrcp_init, avrcp_exit)
