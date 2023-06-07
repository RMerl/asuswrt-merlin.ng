/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2015  Google Inc.
 *
 *
 */

#define MAX_SIZE 4096

struct btd_gatt_database {
	struct btd_adapter *adapter;
	struct gatt_db *db;
	unsigned int db_id;
	GIOChannel *le_io;
	GIOChannel *eatt_io;
	GIOChannel *bredr_io;
	struct queue *records;
	struct queue *device_states;
	struct queue *ccc_callbacks;
	struct gatt_db_attribute *svc_chngd;
	struct gatt_db_attribute *svc_chngd_ccc;
	struct gatt_db_attribute *cli_feat;
	struct gatt_db_attribute *db_hash;
	struct gatt_db_attribute *eatt;
	struct queue *apps;
	struct queue *profiles;
	uint32_t amap_handle;
	uint8_t amap_value[MAX_SIZE];
	int amap_value_len;
	uint16_t amap_nvram_handle;
	struct bt_att *amap_att;
};

struct btd_gatt_database *btd_gatt_database_new(struct btd_adapter *adapter);

struct gatt_db *btd_gatt_database_get_db(struct btd_gatt_database *database);
void btd_gatt_database_att_disconnected(struct btd_gatt_database *database,
						struct btd_device *device);
void btd_gatt_database_server_connected(struct btd_gatt_database *database,
						struct bt_gatt_server *server);

void btd_gatt_database_restore_svc_chng_ccc(struct btd_gatt_database *database);

typedef uint8_t (*btd_gatt_database_ccc_write_t) (uint16_t value, void *user_data);

struct gatt_db_attribute *
btd_gatt_database_add_ccc(struct btd_gatt_database *database,
                                uint16_t service_handle,
                                btd_gatt_database_ccc_write_t write_callback,
                                void *user_data,
                                btd_gatt_database_destroy_t destroy);

struct gatt_db_attribute *
service_add_ccc(struct gatt_db_attribute *service,
                                struct btd_gatt_database *database,
                                btd_gatt_database_ccc_write_t write_callback,
                                void *user_data,
                                btd_gatt_database_destroy_t destroy);

void btd_gatt_database_destroy(struct btd_gatt_database *database);
bool get_dst_info(struct bt_att *att, bdaddr_t *dst, uint8_t *dst_type);
uint32_t database_add_record(struct btd_gatt_database *database,
                                        uint16_t uuid,
                                        struct gatt_db_attribute *attr,
                                        const char *name);

