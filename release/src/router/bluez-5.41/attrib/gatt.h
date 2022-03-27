/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2010  Nokia Corporation
 *  Copyright (C) 2010  Marcel Holtmann <marcel@holtmann.org>
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

/*
 * GATT Characteristic Property bit field
 * Reference: Core SPEC 4.1 page 2183 (Table 3.5: Characteristic Properties
 * bit field) defines how the Characteristic Value can be used, or how the
 * characteristic descriptors (see Section 3.3.3 - page 2184) can be accessed.
 * In the core spec, regular properties are included in the characteristic
 * declaration, and the extended properties are defined as descriptor.
 */

#define GATT_CHR_PROP_BROADCAST				0x01
#define GATT_CHR_PROP_READ				0x02
#define GATT_CHR_PROP_WRITE_WITHOUT_RESP		0x04
#define GATT_CHR_PROP_WRITE				0x08
#define GATT_CHR_PROP_NOTIFY				0x10
#define GATT_CHR_PROP_INDICATE				0x20
#define GATT_CHR_PROP_AUTH				0x40
#define GATT_CHR_PROP_EXT_PROP				0x80

/* Client Characteristic Configuration bit field */
#define GATT_CLIENT_CHARAC_CFG_NOTIF_BIT	0x0001
#define GATT_CLIENT_CHARAC_CFG_IND_BIT		0x0002

typedef void (*gatt_cb_t) (uint8_t status, GSList *l, void *user_data);

struct gatt_primary {
	char uuid[MAX_LEN_UUID_STR + 1];
	gboolean changed;
	struct att_range range;
};

struct gatt_included {
	char uuid[MAX_LEN_UUID_STR + 1];
	uint16_t handle;
	struct att_range range;
};

struct gatt_char {
	char uuid[MAX_LEN_UUID_STR + 1];
	uint16_t handle;
	uint8_t properties;
	uint16_t value_handle;
};

struct gatt_desc {
	char uuid[MAX_LEN_UUID_STR + 1];
	uint16_t handle;
	uint16_t uuid16;
};

guint gatt_discover_primary(GAttrib *attrib, bt_uuid_t *uuid, gatt_cb_t func,
							gpointer user_data);

unsigned int gatt_find_included(GAttrib *attrib, uint16_t start, uint16_t end,
					gatt_cb_t func, gpointer user_data);

guint gatt_discover_char(GAttrib *attrib, uint16_t start, uint16_t end,
					bt_uuid_t *uuid, gatt_cb_t func,
					gpointer user_data);

guint gatt_read_char(GAttrib *attrib, uint16_t handle, GAttribResultFunc func,
							gpointer user_data);

guint gatt_write_char(GAttrib *attrib, uint16_t handle, const uint8_t *value,
					size_t vlen, GAttribResultFunc func,
					gpointer user_data);

guint gatt_discover_desc(GAttrib *attrib, uint16_t start, uint16_t end,
						bt_uuid_t *uuid, gatt_cb_t func,
						gpointer user_data);

guint gatt_reliable_write_char(GAttrib *attrib, uint16_t handle,
					const uint8_t *value, size_t vlen,
					GAttribResultFunc func,
					gpointer user_data);

guint gatt_execute_write(GAttrib *attrib, uint8_t flags,
				GAttribResultFunc func, gpointer user_data);

guint gatt_write_cmd(GAttrib *attrib, uint16_t handle, const uint8_t *value,
			int vlen, GDestroyNotify notify, gpointer user_data);

guint gatt_signed_write_cmd(GAttrib *attrib, uint16_t handle,
						const uint8_t *value, int vlen,
						struct bt_crypto *crypto,
						const uint8_t csrk[16],
						uint32_t sign_cnt,
						GDestroyNotify notify,
						gpointer user_data);
guint gatt_read_char_by_uuid(GAttrib *attrib, uint16_t start, uint16_t end,
				bt_uuid_t *uuid, GAttribResultFunc func,
				gpointer user_data);

guint gatt_exchange_mtu(GAttrib *attrib, uint16_t mtu, GAttribResultFunc func,
							gpointer user_data);

gboolean gatt_parse_record(const sdp_record_t *rec,
					uuid_t *prim_uuid, uint16_t *psm,
					uint16_t *start, uint16_t *end);
