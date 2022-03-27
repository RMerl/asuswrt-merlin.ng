/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2011  Nokia Corporation
 *  Copyright (C) 2011  Marcel Holtmann <marcel@holtmann.org>
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

#include <glib.h>

#include "lib/sdp.h"

#define EIR_FLAGS                   0x01  /* flags */
#define EIR_UUID16_SOME             0x02  /* 16-bit UUID, more available */
#define EIR_UUID16_ALL              0x03  /* 16-bit UUID, all listed */
#define EIR_UUID32_SOME             0x04  /* 32-bit UUID, more available */
#define EIR_UUID32_ALL              0x05  /* 32-bit UUID, all listed */
#define EIR_UUID128_SOME            0x06  /* 128-bit UUID, more available */
#define EIR_UUID128_ALL             0x07  /* 128-bit UUID, all listed */
#define EIR_NAME_SHORT              0x08  /* shortened local name */
#define EIR_NAME_COMPLETE           0x09  /* complete local name */
#define EIR_TX_POWER                0x0A  /* transmit power level */
#define EIR_CLASS_OF_DEV            0x0D  /* Class of Device */
#define EIR_SSP_HASH                0x0E  /* SSP Hash */
#define EIR_SSP_RANDOMIZER          0x0F  /* SSP Randomizer */
#define EIR_DEVICE_ID               0x10  /* device ID */
#define EIR_SOLICIT16               0x14  /* LE: Solicit UUIDs, 16-bit */
#define EIR_SOLICIT128              0x15  /* LE: Solicit UUIDs, 128-bit */
#define EIR_SVC_DATA16              0x16  /* LE: Service data, 16-bit UUID */
#define EIR_PUB_TRGT_ADDR           0x17  /* LE: Public Target Address */
#define EIR_RND_TRGT_ADDR           0x18  /* LE: Random Target Address */
#define EIR_GAP_APPEARANCE          0x19  /* GAP appearance */
#define EIR_SOLICIT32               0x1F  /* LE: Solicit UUIDs, 32-bit */
#define EIR_SVC_DATA32              0x20  /* LE: Service data, 32-bit UUID */
#define EIR_SVC_DATA128             0x21  /* LE: Service data, 128-bit UUID */
#define EIR_MANUFACTURER_DATA       0xFF  /* Manufacturer Specific Data */

/* Flags Descriptions */
#define EIR_LIM_DISC                0x01 /* LE Limited Discoverable Mode */
#define EIR_GEN_DISC                0x02 /* LE General Discoverable Mode */
#define EIR_BREDR_UNSUP             0x04 /* BR/EDR Not Supported */
#define EIR_CONTROLLER              0x08 /* Simultaneous LE and BR/EDR to Same
					    Device Capable (Controller) */
#define EIR_SIM_HOST                0x10 /* Simultaneous LE and BR/EDR to Same
					    Device Capable (Host) */

#define EIR_SD_MAX_LEN              238  /* 240 (EIR) - 2 (len) */
#define EIR_MSD_MAX_LEN             236  /* 240 (EIR) - 2 (len & type) - 2 */

struct eir_msd {
	uint16_t company;
	uint8_t data[EIR_MSD_MAX_LEN];
	uint8_t data_len;
};

struct eir_sd {
	char *uuid;
	uint8_t data[EIR_SD_MAX_LEN];
	uint8_t data_len;
};

struct eir_data {
	GSList *services;
	unsigned int flags;
	char *name;
	uint32_t class;
	uint16_t appearance;
	bool name_complete;
	int8_t tx_power;
	uint8_t *hash;
	uint8_t *randomizer;
	bdaddr_t addr;
	uint16_t did_vendor;
	uint16_t did_product;
	uint16_t did_version;
	uint16_t did_source;
	GSList *msd_list;
	GSList *sd_list;
};

void eir_data_free(struct eir_data *eir);
void eir_parse(struct eir_data *eir, const uint8_t *eir_data, uint8_t eir_len);
int eir_parse_oob(struct eir_data *eir, uint8_t *eir_data, uint16_t eir_len);
int eir_create_oob(const bdaddr_t *addr, const char *name, uint32_t cod,
			const uint8_t *hash, const uint8_t *randomizer,
			uint16_t did_vendor, uint16_t did_product,
			uint16_t did_version, uint16_t did_source,
			sdp_list_t *uuids, uint8_t *data);
