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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib.h>
#include <errno.h>

#include "lib/bluetooth.h"
#include "lib/sdp.h"
#include "lib/uuid.h"

#include "src/plugin.h"
#include "src/adapter.h"
#include "src/shared/util.h"
#include "src/log.h"
#include "attrib/gattrib.h"
#include "attrib/gatt-service.h"
#include "attrib/att.h"
#include "attrib/gatt.h"
#include "attrib/att-database.h"
#include "src/attrib-server.h"

/* FIXME: Not defined by SIG? UUID128? */
#define OPCODES_SUPPORTED_UUID          0xA001
#define BATTERY_STATE_SVC_UUID		0xA002
#define BATTERY_STATE_UUID		0xA003
#define THERM_HUMIDITY_SVC_UUID		0xA004
#define MANUFACTURER_SVC_UUID		0xA005
#define TEMPERATURE_UUID		0xA006
#define FMT_CELSIUS_UUID		0xA007
#define FMT_OUTSIDE_UUID		0xA008
#define RELATIVE_HUMIDITY_UUID		0xA009
#define FMT_PERCENT_UUID		0xA00A
#define BLUETOOTH_SIG_UUID		0xA00B
#define MANUFACTURER_NAME_UUID		0xA00C
#define MANUFACTURER_SERIAL_UUID	0xA00D
#define VENDOR_SPECIFIC_SVC_UUID	0xA00E
#define VENDOR_SPECIFIC_TYPE_UUID	0xA00F
#define FMT_KILOGRAM_UUID		0xA010
#define FMT_HANGING_UUID		0xA011

struct gatt_example_adapter {
	struct btd_adapter	*adapter;
	GSList			*sdp_handles;
};

static GSList *adapters = NULL;

static void gatt_example_adapter_free(struct gatt_example_adapter *gadapter)
{
	while (gadapter->sdp_handles != NULL) {
		uint32_t handle = GPOINTER_TO_UINT(gadapter->sdp_handles->data);

		attrib_free_sdp(gadapter->adapter, handle);
		gadapter->sdp_handles = g_slist_remove(gadapter->sdp_handles,
						gadapter->sdp_handles->data);
	}

	if (gadapter->adapter != NULL)
		btd_adapter_unref(gadapter->adapter);

	g_free(gadapter);
}

static int adapter_cmp(gconstpointer a, gconstpointer b)
{
	const struct gatt_example_adapter *gatt_adapter = a;
	const struct btd_adapter *adapter = b;

	if (gatt_adapter->adapter == adapter)
		return 0;

	return -1;
}

static uint8_t battery_state_read(struct attribute *a,
				  struct btd_device *device, gpointer user_data)
{
	struct btd_adapter *adapter = user_data;
	uint8_t value;

	value = 0x04;
	attrib_db_update(adapter, a->handle, NULL, &value, sizeof(value), NULL);

	return 0;
}

static gboolean register_battery_service(struct btd_adapter *adapter)
{
	bt_uuid_t uuid;

	bt_uuid16_create(&uuid, BATTERY_STATE_SVC_UUID);

	return gatt_service_add(adapter, GATT_PRIM_SVC_UUID, &uuid,
			/* battery state characteristic */
			GATT_OPT_CHR_UUID16, BATTERY_STATE_UUID,
			GATT_OPT_CHR_PROPS, GATT_CHR_PROP_READ |
							GATT_CHR_PROP_NOTIFY,
			GATT_OPT_CHR_VALUE_CB, ATTRIB_READ,
						battery_state_read, adapter,

			GATT_OPT_INVALID);
}

static void register_termometer_service(struct gatt_example_adapter *adapter,
			const uint16_t manuf1[2], const uint16_t manuf2[2])
{
	const char *desc_out_temp = "Outside Temperature";
	const char *desc_out_hum = "Outside Relative Humidity";
	uint16_t start_handle, h;
	const int svc_size = 11;
	uint32_t sdp_handle;
	uint8_t atval[256];
	bt_uuid_t uuid;
	int len;

	bt_uuid16_create(&uuid, THERM_HUMIDITY_SVC_UUID);
	start_handle = attrib_db_find_avail(adapter->adapter, &uuid, svc_size);
	if (start_handle == 0) {
		error("Not enough free handles to register service");
		return;
	}

	DBG("start_handle=0x%04x manuf1=0x%04x-0x%04x, manuf2=0x%04x-0x%04x",
		start_handle, manuf1[0], manuf1[1], manuf2[0], manuf2[1]);

	h = start_handle;

	/* Thermometer: primary service definition */
	bt_uuid16_create(&uuid, GATT_PRIM_SVC_UUID);
	put_le16(THERM_HUMIDITY_SVC_UUID, &atval[0]);
	attrib_db_add(adapter->adapter, h++, &uuid, ATT_NONE, ATT_NOT_PERMITTED,
								atval, 2);

	bt_uuid16_create(&uuid, GATT_INCLUDE_UUID);

	/* Thermometer: Include */
	if (manuf1[0] && manuf1[1]) {
		put_le16(manuf1[0], &atval[0]);
		put_le16(manuf1[1], &atval[2]);
		put_le16(MANUFACTURER_SVC_UUID, &atval[4]);
		attrib_db_add(adapter->adapter, h++, &uuid, ATT_NONE,
						ATT_NOT_PERMITTED, atval, 6);
	}

	/* Thermometer: Include */
	if (manuf2[0] && manuf2[1]) {
		put_le16(manuf2[0], &atval[0]);
		put_le16(manuf2[1], &atval[2]);
		put_le16(VENDOR_SPECIFIC_SVC_UUID, &atval[4]);
		attrib_db_add(adapter->adapter, h++, &uuid, ATT_NONE,
						ATT_NOT_PERMITTED, atval, 6);
	}

	/* Thermometer: temperature characteristic */
	bt_uuid16_create(&uuid, GATT_CHARAC_UUID);
	atval[0] = GATT_CHR_PROP_READ;
	put_le16(h + 1, &atval[1]);
	put_le16(TEMPERATURE_UUID, &atval[3]);
	attrib_db_add(adapter->adapter, h++, &uuid, ATT_NONE, ATT_NOT_PERMITTED,
								atval, 5);

	/* Thermometer: temperature characteristic value */
	bt_uuid16_create(&uuid, TEMPERATURE_UUID);
	atval[0] = 0x8A;
	atval[1] = 0x02;
	attrib_db_add(adapter->adapter, h++, &uuid, ATT_NONE, ATT_NOT_PERMITTED,
								atval, 2);

	/* Thermometer: temperature characteristic format */
	bt_uuid16_create(&uuid, GATT_CHARAC_FMT_UUID);
	atval[0] = 0x0E;
	atval[1] = 0xFE;
	put_le16(FMT_CELSIUS_UUID, &atval[2]);
	atval[4] = 0x01;
	put_le16(FMT_OUTSIDE_UUID, &atval[5]);
	attrib_db_add(adapter->adapter, h++, &uuid, ATT_NONE, ATT_NOT_PERMITTED,
								atval, 7);

	/* Thermometer: characteristic user description */
	bt_uuid16_create(&uuid, GATT_CHARAC_USER_DESC_UUID);
	len = strlen(desc_out_temp);
	strncpy((char *) atval, desc_out_temp, len);
	attrib_db_add(adapter->adapter, h++, &uuid, ATT_NONE, ATT_NOT_PERMITTED,
								atval, len);

	/* Thermometer: relative humidity characteristic */
	bt_uuid16_create(&uuid, GATT_CHARAC_UUID);
	atval[0] = GATT_CHR_PROP_READ;
	put_le16(h + 1, &atval[1]);
	put_le16(RELATIVE_HUMIDITY_UUID, &atval[3]);
	attrib_db_add(adapter->adapter, h++, &uuid, ATT_NONE, ATT_NOT_PERMITTED,
								atval, 5);

	/* Thermometer: relative humidity value */
	bt_uuid16_create(&uuid, RELATIVE_HUMIDITY_UUID);
	atval[0] = 0x27;
	attrib_db_add(adapter->adapter, h++, &uuid, ATT_NONE, ATT_NOT_PERMITTED,
								atval, 1);

	/* Thermometer: relative humidity characteristic format */
	bt_uuid16_create(&uuid, GATT_CHARAC_FMT_UUID);
	atval[0] = 0x04;
	atval[1] = 0x00;
	put_le16(FMT_PERCENT_UUID, &atval[2]);
	put_le16(BLUETOOTH_SIG_UUID, &atval[4]);
	put_le16(FMT_OUTSIDE_UUID, &atval[6]);
	attrib_db_add(adapter->adapter, h++, &uuid, ATT_NONE, ATT_NOT_PERMITTED,
								atval, 8);

	/* Thermometer: characteristic user description */
	bt_uuid16_create(&uuid, GATT_CHARAC_USER_DESC_UUID);
	len = strlen(desc_out_hum);
	strncpy((char *) atval, desc_out_hum, len);
	attrib_db_add(adapter->adapter, h, &uuid, ATT_NONE, ATT_NOT_PERMITTED,
								atval, len);

	g_assert(h - start_handle + 1 == svc_size);

	/* Add an SDP record for the above service */
	sdp_handle = attrib_create_sdp(adapter->adapter, start_handle,
								"Thermometer");
	if (sdp_handle)
		adapter->sdp_handles = g_slist_prepend(adapter->sdp_handles,
						GUINT_TO_POINTER(sdp_handle));
}

static void register_manuf1_service(struct gatt_example_adapter *adapter,
							uint16_t range[2])
{
	const char *manufacturer_name1 = "ACME Temperature Sensor";
	const char *serial1 = "237495-3282-A";
	uint16_t start_handle, h;
	const int svc_size = 5;
	uint8_t atval[256];
	bt_uuid_t uuid;
	int len;

	bt_uuid16_create(&uuid, MANUFACTURER_SVC_UUID);
	start_handle = attrib_db_find_avail(adapter->adapter, &uuid, svc_size);
	if (start_handle == 0) {
		error("Not enough free handles to register service");
		return;
	}

	DBG("start_handle=0x%04x", start_handle);

	h = start_handle;

	/* Secondary Service: Manufacturer Service */
	bt_uuid16_create(&uuid, GATT_SND_SVC_UUID);
	put_le16(MANUFACTURER_SVC_UUID, &atval[0]);
	attrib_db_add(adapter->adapter, h++, &uuid, ATT_NONE, ATT_NOT_PERMITTED,
								atval, 2);

	/* Manufacturer name characteristic definition */
	bt_uuid16_create(&uuid, GATT_CHARAC_UUID);
	atval[0] = GATT_CHR_PROP_READ;
	put_le16(h + 1, &atval[1]);
	put_le16(MANUFACTURER_NAME_UUID, &atval[3]);
	attrib_db_add(adapter->adapter, h++, &uuid, ATT_NONE, ATT_NOT_PERMITTED,
								atval, 5);

	/* Manufacturer name characteristic value */
	bt_uuid16_create(&uuid, MANUFACTURER_NAME_UUID);
	len = strlen(manufacturer_name1);
	strncpy((char *) atval, manufacturer_name1, len);
	attrib_db_add(adapter->adapter, h++, &uuid, ATT_NONE, ATT_NOT_PERMITTED,
								atval, len);

	/* Manufacturer serial number characteristic */
	bt_uuid16_create(&uuid, GATT_CHARAC_UUID);
	atval[0] = GATT_CHR_PROP_READ;
	put_le16(h + 1, &atval[1]);
	put_le16(MANUFACTURER_SERIAL_UUID, &atval[3]);
	attrib_db_add(adapter->adapter, h++, &uuid, ATT_NONE, ATT_NOT_PERMITTED,
								atval, 5);

	/* Manufacturer serial number characteristic value */
	bt_uuid16_create(&uuid, MANUFACTURER_SERIAL_UUID);
	len = strlen(serial1);
	strncpy((char *) atval, serial1, len);
	attrib_db_add(adapter->adapter, h, &uuid, ATT_NONE, ATT_NOT_PERMITTED,
								atval, len);

	g_assert(h - start_handle + 1 == svc_size);

	range[0] = start_handle;
	range[1] = start_handle + svc_size - 1;
}

static void register_manuf2_service(struct gatt_example_adapter *adapter,
							uint16_t range[2])
{
	const char *manufacturer_name2 = "ACME Weighing Scales";
	const char *serial2 = "11267-2327A00239";
	uint16_t start_handle, h;
	const int svc_size = 5;
	uint8_t atval[256];
	bt_uuid_t uuid;
	int len;

	bt_uuid16_create(&uuid, MANUFACTURER_SVC_UUID);
	start_handle = attrib_db_find_avail(adapter->adapter, &uuid, svc_size);
	if (start_handle == 0) {
		error("Not enough free handles to register service");
		return;
	}

	DBG("start_handle=0x%04x", start_handle);

	h = start_handle;

	/* Secondary Service: Manufacturer Service */
	bt_uuid16_create(&uuid, GATT_SND_SVC_UUID);
	put_le16(MANUFACTURER_SVC_UUID, &atval[0]);
	attrib_db_add(adapter->adapter, h++, &uuid, ATT_NONE, ATT_NOT_PERMITTED,
								atval, 2);

	/* Manufacturer name characteristic definition */
	bt_uuid16_create(&uuid, GATT_CHARAC_UUID);
	atval[0] = GATT_CHR_PROP_READ;
	put_le16(h + 1, &atval[1]);
	put_le16(MANUFACTURER_NAME_UUID, &atval[3]);
	attrib_db_add(adapter->adapter, h++, &uuid, ATT_NONE, ATT_NOT_PERMITTED,
								atval, 5);

	/* Manufacturer name attribute */
	bt_uuid16_create(&uuid, MANUFACTURER_NAME_UUID);
	len = strlen(manufacturer_name2);
	strncpy((char *) atval, manufacturer_name2, len);
	attrib_db_add(adapter->adapter, h++, &uuid, ATT_NONE, ATT_NOT_PERMITTED,
								atval, len);

	/* Characteristic: serial number */
	bt_uuid16_create(&uuid, GATT_CHARAC_UUID);
	atval[0] = GATT_CHR_PROP_READ;
	put_le16(h + 1, &atval[1]);
	put_le16(MANUFACTURER_SERIAL_UUID, &atval[3]);
	attrib_db_add(adapter->adapter, h++, &uuid, ATT_NONE, ATT_NOT_PERMITTED,
								atval, 5);

	/* Serial number characteristic value */
	bt_uuid16_create(&uuid, MANUFACTURER_SERIAL_UUID);
	len = strlen(serial2);
	strncpy((char *) atval, serial2, len);
	attrib_db_add(adapter->adapter, h, &uuid, ATT_NONE, ATT_NOT_PERMITTED,
								atval, len);

	g_assert(h - start_handle + 1 == svc_size);

	range[0] = start_handle;
	range[1] = start_handle + svc_size - 1;
}

static void register_vendor_service(struct gatt_example_adapter *adapter,
							uint16_t range[2])
{
	uint16_t start_handle, h;
	const int svc_size = 3;
	uint8_t atval[256];
	bt_uuid_t uuid;

	bt_uuid16_create(&uuid, VENDOR_SPECIFIC_SVC_UUID);
	start_handle = attrib_db_find_avail(adapter->adapter, &uuid, svc_size);
	if (start_handle == 0) {
		error("Not enough free handles to register service");
		return;
	}

	DBG("start_handle=0x%04x", start_handle);

	h = start_handle;

	/* Secondary Service: Vendor Specific Service */
	bt_uuid16_create(&uuid, GATT_SND_SVC_UUID);
	put_le16(VENDOR_SPECIFIC_SVC_UUID, &atval[0]);
	attrib_db_add(adapter->adapter, h++, &uuid, ATT_NONE, ATT_NOT_PERMITTED,
								atval, 2);

	/* Vendor Specific Type characteristic definition */
	bt_uuid16_create(&uuid, GATT_CHARAC_UUID);
	atval[0] = GATT_CHR_PROP_READ;
	put_le16(h + 1, &atval[1]);
	put_le16(VENDOR_SPECIFIC_TYPE_UUID, &atval[3]);
	attrib_db_add(adapter->adapter, h++, &uuid, ATT_NONE, ATT_NOT_PERMITTED,
								atval, 5);

	/* Vendor Specific Type characteristic value */
	bt_uuid16_create(&uuid, VENDOR_SPECIFIC_TYPE_UUID);
	atval[0] = 0x56;
	atval[1] = 0x65;
	atval[2] = 0x6E;
	atval[3] = 0x64;
	atval[4] = 0x6F;
	atval[5] = 0x72;
	attrib_db_add(adapter->adapter, h, &uuid, ATT_NONE, ATT_NOT_PERMITTED,
								atval, 6);

	g_assert(h - start_handle + 1 == svc_size);

	range[0] = start_handle;
	range[1] = start_handle + svc_size - 1;
}

static void register_weight_service(struct gatt_example_adapter *adapter,
						const uint16_t vendor[2])
{
	const char *desc_weight = "Rucksack Weight";
	const uint128_t char_weight_uuid_btorder = {
		.data = { 0x80, 0x88, 0xF2, 0x18, 0x90, 0x2C, 0x45, 0x0B,
			  0xB6, 0xC4, 0x62, 0x89, 0x1E, 0x8C, 0x25, 0xE9 } };
	const uint128_t prim_weight_uuid_btorder = {
		.data = { 0x4F, 0x0A, 0xC0, 0x96, 0x35, 0xD4, 0x49, 0x11,
			  0x96, 0x31, 0xDE, 0xA8, 0xDC, 0x74, 0xEE, 0xFE } };
	uint128_t prim_weight_uuid, char_weight_uuid;
	uint16_t start_handle, h;
	const int svc_size = 6;
	uint32_t sdp_handle;
	uint8_t atval[256];
	bt_uuid_t uuid;
	int len;

	btoh128(&char_weight_uuid_btorder, &char_weight_uuid);
	btoh128(&prim_weight_uuid_btorder, &prim_weight_uuid);
	bt_uuid128_create(&uuid, prim_weight_uuid);
	start_handle = attrib_db_find_avail(adapter->adapter, &uuid, svc_size);
	if (start_handle == 0) {
		error("Not enough free handles to register service");
		return;
	}

	DBG("start_handle=0x%04x, vendor=0x%04x-0x%04x", start_handle,
							vendor[0], vendor[1]);

	h = start_handle;

	/* Weight service: primary service definition */
	bt_uuid16_create(&uuid, GATT_PRIM_SVC_UUID);
	memcpy(atval, &prim_weight_uuid_btorder, 16);
	attrib_db_add(adapter->adapter, h++, &uuid, ATT_NONE, ATT_NOT_PERMITTED,
								atval, 16);

	if (vendor[0] && vendor[1]) {
		/* Weight: include */
		bt_uuid16_create(&uuid, GATT_INCLUDE_UUID);
		put_le16(vendor[0], &atval[0]);
		put_le16(vendor[1], &atval[2]);
		put_le16(MANUFACTURER_SVC_UUID, &atval[4]);
		attrib_db_add(adapter->adapter, h++, &uuid, ATT_NONE,
						ATT_NOT_PERMITTED, atval, 6);
	}

	/* Weight: characteristic */
	bt_uuid16_create(&uuid, GATT_CHARAC_UUID);
	atval[0] = GATT_CHR_PROP_READ;
	put_le16(h + 1, &atval[1]);
	memcpy(&atval[3], &char_weight_uuid_btorder, 16);
	attrib_db_add(adapter->adapter, h++, &uuid, ATT_NONE, ATT_NOT_PERMITTED,
								atval, 19);

	/* Weight: characteristic value */
	bt_uuid128_create(&uuid, char_weight_uuid);
	atval[0] = 0x82;
	atval[1] = 0x55;
	atval[2] = 0x00;
	atval[3] = 0x00;
	attrib_db_add(adapter->adapter, h++, &uuid, ATT_NONE, ATT_NOT_PERMITTED,
								atval, 4);

	/* Weight: characteristic format */
	bt_uuid16_create(&uuid, GATT_CHARAC_FMT_UUID);
	atval[0] = 0x08;
	atval[1] = 0xFD;
	put_le16(FMT_KILOGRAM_UUID, &atval[2]);
	put_le16(BLUETOOTH_SIG_UUID, &atval[4]);
	put_le16(FMT_HANGING_UUID, &atval[6]);
	attrib_db_add(adapter->adapter, h++, &uuid, ATT_NONE, ATT_NOT_PERMITTED,
								atval, 8);

	/* Weight: characteristic user description */
	bt_uuid16_create(&uuid, GATT_CHARAC_USER_DESC_UUID);
	len = strlen(desc_weight);
	strncpy((char *) atval, desc_weight, len);
	attrib_db_add(adapter->adapter, h, &uuid, ATT_NONE, ATT_NOT_PERMITTED,
								atval, len);
	g_assert(h - start_handle + 1 == svc_size);

	/* Add an SDP record for the above service */
	sdp_handle = attrib_create_sdp(adapter->adapter, start_handle,
							"Weight Service");
	if (sdp_handle)
		adapter->sdp_handles = g_slist_prepend(adapter->sdp_handles,
						GUINT_TO_POINTER(sdp_handle));
}

static int gatt_example_adapter_probe(struct btd_adapter *adapter)
{
	uint16_t manuf1_range[2] = {0, 0}, manuf2_range[2] = {0, 0};
	uint16_t vendor_range[2] = {0, 0};
	struct gatt_example_adapter *gadapter;

	gadapter = g_new0(struct gatt_example_adapter, 1);
	gadapter->adapter = btd_adapter_ref(adapter);

	if (!register_battery_service(adapter)) {
		DBG("Battery service could not be registered");
		gatt_example_adapter_free(gadapter);
		return -EIO;
	}

	register_manuf1_service(gadapter, manuf1_range);
	register_manuf2_service(gadapter, manuf2_range);
	register_termometer_service(gadapter, manuf1_range, manuf2_range);
	register_vendor_service(gadapter, vendor_range);
	register_weight_service(gadapter, vendor_range);

	adapters = g_slist_append(adapters, gadapter);

	return 0;
}

static void gatt_example_adapter_remove(struct btd_adapter *adapter)
{
	struct gatt_example_adapter *gadapter;
	GSList *l;

	l = g_slist_find_custom(adapters, adapter, adapter_cmp);
	if (l == NULL)
		return;

	gadapter = l->data;
	adapters = g_slist_remove(adapters, gadapter);
	gatt_example_adapter_free(gadapter);
}

static struct btd_adapter_driver gatt_example_adapter_driver = {
	.name	= "gatt-example-adapter-driver",
	.probe	= gatt_example_adapter_probe,
	.remove	= gatt_example_adapter_remove,
};

static int gatt_example_init(void)
{
	return btd_register_adapter_driver(&gatt_example_adapter_driver);
}

static void gatt_example_exit(void)
{
	btd_unregister_adapter_driver(&gatt_example_adapter_driver);
}

BLUETOOTH_PLUGIN_DEFINE(gatt_example, VERSION, BLUETOOTH_PLUGIN_PRIORITY_LOW,
					gatt_example_init, gatt_example_exit)
