/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2011-2012  Intel Corporation
 *  Copyright (C) 2004-2010  Marcel Holtmann <marcel@holtmann.org>
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

#include <stdbool.h>
#include <stdint.h>

struct bt_phy;

struct bt_phy *bt_phy_new(void);

struct bt_phy *bt_phy_ref(struct bt_phy *phy);
void bt_phy_unref(struct bt_phy *phy);

bool bt_phy_send(struct bt_phy *phy, uint16_t type,
					const void *data, size_t size);
bool bt_phy_send_vector(struct bt_phy *phy, uint16_t type,
					const void *data1, size_t size1,
					const void *data2, size_t size2,
					const void *data3, size_t size3);

typedef void (*bt_phy_callback_func_t)(uint16_t type, const void *data,
						size_t size, void *user_data);

bool bt_phy_register(struct bt_phy *phy, bt_phy_callback_func_t callback,
							void *user_data);

#define BT_PHY_PKT_NULL		0x0000

#define BT_PHY_PKT_ADV		0x0001
struct bt_phy_pkt_adv {
	uint8_t  chan_idx;
	uint8_t  pdu_type;
	uint8_t  tx_addr_type;
	uint8_t  tx_addr[6];
	uint8_t  rx_addr_type;
	uint8_t  rx_addr[6];
	uint8_t  adv_data_len;
	uint8_t  scan_rsp_len;
} __attribute__ ((packed));

#define BT_PHY_PKT_CONN		0x0002
struct bt_phy_pkt_conn {
	uint8_t  chan_idx;
	uint8_t  link_type;
	uint8_t  tx_addr_type;
	uint8_t  tx_addr[6];
	uint8_t  rx_addr_type;
	uint8_t  rx_addr[6];
	uint8_t  features[8];
	uint8_t  id;
} __attribute__ ((packed));
