/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2023 PHYTEC Messtechnik GmbH
 * Author: Teresa Remmet <t.remmet@phytec.de>
 */

#ifndef _PHYTEC_SOM_DETECTION_H
#define _PHYTEC_SOM_DETECTION_H

#define PHYTEC_MAX_OPTIONS	17
#define PHYTEC_EEPROM_INVAL	0xff

#define PHYTEC_GET_OPTION(option) \
	(((option) > '9') ? (option) - 'A' + 10 : (option) - '0')

enum {
	PHYTEC_API_REV0 = 0,
	PHYTEC_API_REV1,
	PHYTEC_API_REV2,
};

enum phytec_som_type_str {
	SOM_TYPE_PCM = 0,
	SOM_TYPE_PCL,
	SOM_TYPE_KSM,
	SOM_TYPE_KSP,
};

static const char * const phytec_som_type_str[] = {
	"PCM",
	"PCL",
	"KSM",
	"KSP",
};

struct phytec_api0_data {
	u8 pcb_rev;		/* PCB revision of SoM */
	u8 som_type;		/* SoM type */
	u8 ksp_no;		/* KSP no */
	char opt[16];		/* SoM options */
	u8 mac[6];		/* MAC address (optional) */
	u8 pad[5];		/* padding */
	u8 cksum;		/* checksum */
} __packed;

struct phytec_api2_data {
	u8 pcb_rev;		/* PCB revision of SoM */
	u8 pcb_sub_opt_rev;	/* PCB subrevision and opt revision */
	u8 som_type;		/* SoM type */
	u8 som_no;		/* SoM number */
	u8 ksp_no;		/* KSP information */
	char opt[PHYTEC_MAX_OPTIONS]; /* SoM options */
	char bom_rev[2];	/* BOM revision */
	u8 mac[6];		/* MAC address (optional) */
	u8 crc8;		/* checksum */
} __packed;

struct phytec_eeprom_data {
	u8 api_rev;
	union {
		struct phytec_api0_data data_api0;
		struct phytec_api2_data data_api2;
	} data;
} __packed;

int phytec_eeprom_data_setup_fallback(struct phytec_eeprom_data *data,
				      int bus_num, int addr,
				      int addr_fallback);
int phytec_eeprom_data_setup(struct phytec_eeprom_data *data,
			     int bus_num, int addr);
int phytec_eeprom_data_init(struct phytec_eeprom_data *data,
			    int bus_num, int addr);
void __maybe_unused phytec_print_som_info(struct phytec_eeprom_data *data);

char * __maybe_unused phytec_get_opt(struct phytec_eeprom_data *data);
u8 __maybe_unused phytec_get_rev(struct phytec_eeprom_data *data);
u8 __maybe_unused phytec_get_som_type(struct phytec_eeprom_data *data);

#endif /* _PHYTEC_SOM_DETECTION_H */
