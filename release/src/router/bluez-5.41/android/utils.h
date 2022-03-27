/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2013-2014  Intel Corporation. All rights reserved.
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
