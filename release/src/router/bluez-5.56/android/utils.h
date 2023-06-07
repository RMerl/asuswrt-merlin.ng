/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2013-2014  Intel Corporation. All rights reserved.
 *
 *
 */

static inline void android2bdaddr(const void *buf, bdaddr_t *dst)
{
	baswap(dst, buf);
}

static inline void bdaddr2android(const bdaddr_t *src, void *buf)
{
	baswap(buf, src);
}

const char *bt_config_get_vendor(void);
const char *bt_config_get_model(void);
const char *bt_config_get_name(void);
const char *bt_config_get_serial(void);
const char *bt_config_get_fw_rev(void);
const char *bt_config_get_hw_rev(void);
uint64_t bt_config_get_system_id(void);
uint16_t bt_config_get_pnp_source(void);
uint16_t bt_config_get_pnp_vendor(void);
uint16_t bt_config_get_pnp_product(void);
uint16_t bt_config_get_pnp_version(void);
