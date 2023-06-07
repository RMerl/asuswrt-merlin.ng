/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2006-2010  Nokia Corporation
 *  Copyright (C) 2004-2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 */

#define DEVICE_INTERFACE	"org.bluez.Device1"

struct btd_device;

struct btd_device *device_create(struct btd_adapter *adapter,
				const bdaddr_t *address, uint8_t bdaddr_type);
struct btd_device *device_create_from_storage(struct btd_adapter *adapter,
				const char *address, GKeyFile *key_file);
char *btd_device_get_storage_path(struct btd_device *device,
				const char *filename);


void btd_device_device_set_name(struct btd_device *device, const char *name);
void device_store_cached_name(struct btd_device *dev, const char *name);
void device_get_name(struct btd_device *device, char *name, size_t len);
bool device_name_known(struct btd_device *device);
void device_set_class(struct btd_device *device, uint32_t class);
void device_update_addr(struct btd_device *device, const bdaddr_t *bdaddr,
							uint8_t bdaddr_type);
void device_set_bredr_support(struct btd_device *device);
void device_set_le_support(struct btd_device *device, uint8_t bdaddr_type);
void device_update_last_seen(struct btd_device *device, uint8_t bdaddr_type);
void device_merge_duplicate(struct btd_device *dev, struct btd_device *dup);
uint32_t btd_device_get_class(struct btd_device *device);
uint16_t btd_device_get_vendor(struct btd_device *device);
uint16_t btd_device_get_vendor_src(struct btd_device *device);
uint16_t btd_device_get_product(struct btd_device *device);
uint16_t btd_device_get_version(struct btd_device *device);
void device_remove_bonding(struct btd_device *device, uint8_t bdaddr_type);
void device_remove(struct btd_device *device, gboolean remove_stored);
int device_address_cmp(gconstpointer a, gconstpointer b);
int device_bdaddr_cmp(gconstpointer a, gconstpointer b);

/* Struct used by device_addr_type_cmp() */
struct device_addr_type {
	bdaddr_t bdaddr;
	uint8_t bdaddr_type;
};

int device_addr_type_cmp(gconstpointer a, gconstpointer b);
GSList *btd_device_get_uuids(struct btd_device *device);
void device_probe_profiles(struct btd_device *device, GSList *profiles);

void btd_device_set_record(struct btd_device *device, const char *uuid,
							const char *record);
const sdp_record_t *btd_device_get_record(struct btd_device *device,
						const char *uuid);
struct gatt_primary *btd_device_get_primary(struct btd_device *device,
							const char *uuid);
GSList *btd_device_get_primaries(struct btd_device *device);
struct gatt_db *btd_device_get_gatt_db(struct btd_device *device);
struct bt_gatt_client *btd_device_get_gatt_client(struct btd_device *device);
struct bt_gatt_server *btd_device_get_gatt_server(struct btd_device *device);
void *btd_device_get_attrib(struct btd_device *device);
void btd_device_gatt_set_service_changed(struct btd_device *device,
						uint16_t start, uint16_t end);
bool device_attach_att(struct btd_device *dev, GIOChannel *io);
void btd_device_add_uuid(struct btd_device *device, const char *uuid);
void device_add_eir_uuids(struct btd_device *dev, GSList *uuids);
void device_set_manufacturer_data(struct btd_device *dev, GSList *list,
							bool duplicate);
void device_set_service_data(struct btd_device *dev, GSList *list,
							bool duplicate);
void device_set_data(struct btd_device *dev, GSList *list,
							bool duplicate);
void device_probe_profile(gpointer a, gpointer b);
void device_remove_profile(gpointer a, gpointer b);
struct btd_adapter *device_get_adapter(struct btd_device *device);
const bdaddr_t *device_get_address(struct btd_device *device);
uint8_t device_get_le_address_type(struct btd_device *device);
const char *device_get_path(const struct btd_device *device);
gboolean device_is_temporary(struct btd_device *device);
bool device_is_connectable(struct btd_device *device);
bool device_is_paired(struct btd_device *device, uint8_t bdaddr_type);
bool device_is_bonded(struct btd_device *device, uint8_t bdaddr_type);
gboolean device_is_trusted(struct btd_device *device);
void device_set_paired(struct btd_device *dev, uint8_t bdaddr_type);
void device_set_unpaired(struct btd_device *dev, uint8_t bdaddr_type);
void btd_device_set_temporary(struct btd_device *device, bool temporary);
void btd_device_set_trusted(struct btd_device *device, gboolean trusted);
void device_set_bonded(struct btd_device *device, uint8_t bdaddr_type);
void device_set_legacy(struct btd_device *device, bool legacy);
void device_set_rssi_with_delta(struct btd_device *device, int8_t rssi,
							int8_t delta_threshold);
void device_set_rssi(struct btd_device *device, int8_t rssi);
void device_set_tx_power(struct btd_device *device, int8_t tx_power);
void device_set_flags(struct btd_device *device, uint8_t flags);
bool btd_device_is_connected(struct btd_device *dev);
uint8_t btd_device_get_bdaddr_type(struct btd_device *dev);
bool device_is_retrying(struct btd_device *device);
void device_bonding_complete(struct btd_device *device, uint8_t bdaddr_type,
							uint8_t status);
gboolean device_is_bonding(struct btd_device *device, const char *sender);
void device_bonding_attempt_failed(struct btd_device *device, uint8_t status);
void device_bonding_failed(struct btd_device *device, uint8_t status);
struct btd_adapter_pin_cb_iter *device_bonding_iter(struct btd_device *device);
int device_bonding_attempt_retry(struct btd_device *device);
long device_bonding_last_duration(struct btd_device *device);
void device_bonding_restart_timer(struct btd_device *device);
int device_request_pincode(struct btd_device *device, gboolean secure);
int device_request_passkey(struct btd_device *device, uint8_t type);
int device_confirm_passkey(struct btd_device *device, uint8_t type,
					int32_t passkey, uint8_t confirm_hint);
int device_notify_passkey(struct btd_device *device, uint8_t type,
					uint32_t passkey, uint8_t entered);
int device_notify_pincode(struct btd_device *device, gboolean secure,
							const char *pincode);
void device_cancel_authentication(struct btd_device *device, gboolean aborted);
gboolean device_is_authenticating(struct btd_device *device);
void device_add_connection(struct btd_device *dev, uint8_t bdaddr_type);
void device_remove_connection(struct btd_device *device, uint8_t bdaddr_type);
void device_request_disconnect(struct btd_device *device, DBusMessage *msg);
bool device_is_disconnecting(struct btd_device *device);
void device_set_ltk_enc_size(struct btd_device *device, uint8_t enc_size);

void device_store_svc_chng_ccc(struct btd_device *device, uint8_t bdaddr_type,
								uint16_t value);
void device_load_svc_chng_ccc(struct btd_device *device, uint16_t *ccc_le,
							uint16_t *ccc_bredr);
void device_set_wake_support(struct btd_device *device, bool wake_support);
void device_set_wake_override(struct btd_device *device, bool wake_override);
void device_set_wake_allowed(struct btd_device *device, bool wake_allowed,
			     guint32 id);
void device_set_wake_allowed_complete(struct btd_device *device);
void device_set_refresh_discovery(struct btd_device *dev, bool refresh);

typedef void (*disconnect_watch) (struct btd_device *device, gboolean removal,
					void *user_data);

guint device_add_disconnect_watch(struct btd_device *device,
				disconnect_watch watch, void *user_data,
				GDestroyNotify destroy);
void device_remove_disconnect_watch(struct btd_device *device, guint id);
int device_get_appearance(struct btd_device *device, uint16_t *value);
void device_set_appearance(struct btd_device *device, uint16_t value);

struct btd_device *btd_device_ref(struct btd_device *device);
void btd_device_unref(struct btd_device *device);

int device_block(struct btd_device *device, gboolean update_only);
int device_unblock(struct btd_device *device, gboolean silent,
							gboolean update_only);
void btd_device_set_pnpid(struct btd_device *device, uint16_t source,
			uint16_t vendor, uint16_t product, uint16_t version);

int device_connect_le(struct btd_device *dev);

typedef void (*device_svc_cb_t) (struct btd_device *dev, int err,
							void *user_data);

unsigned int device_wait_for_svc_complete(struct btd_device *dev,
							device_svc_cb_t func,
							void *user_data);
bool device_remove_svc_complete_callback(struct btd_device *dev,
							unsigned int id);

struct btd_service *btd_device_get_service(struct btd_device *dev,
						const char *remote_uuid);

int device_discover_services(struct btd_device *device);
int btd_device_connect_services(struct btd_device *dev, GSList *services);

uint32_t btd_device_get_current_flags(struct btd_device *dev);
void btd_device_flags_changed(struct btd_device *dev, uint32_t supported_flags,
			      uint32_t current_flags);

void btd_device_init(void);
void btd_device_cleanup(void);
