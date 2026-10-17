/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2021 Gateworks Corporation
 */

#ifndef _VENICE_EEPROM_H_
#define _VENICE_EEPROM_H_

struct venice_board_info {
	u8 mac[6];		/* 0x00: MAC base */
	char equiv_dts[16];	/* 0x06: equivalent device-tree */
	u8 res0[2];	/* 0x16: reserved */
	u32 serial;	/* 0x18: Serial Number */
	u8 res1[4];	/* 0x1C: reserved */
	u8 mfgdate[4];	/* 0x20: MFG date */
	u8 macno;		/* 0x24: number of mac addrs */
	u8 res2[6];	/* 0x25 */
	u8 sdram_size;	/* 0x2B: (16 << n) MB */
	u8 sdram_speed;	/* 0x2C: (33.333 * n) MHz */
	u8 sdram_width;	/* 0x2D: (8 << n) bit */
	u8 res3[2];	/* 0x2E */
	char model[16];		/* 0x30: model string */
	u8 res4[14];	/* 0x40 */
	u8 chksum[2];	/* 0x4E */
};

int venice_eeprom_init(int quiet);
const char *eeprom_get_model(void);
const char *eeprom_get_baseboard_model(void);
const char *eeprom_get_dtb_name(int level, char *buf, int len);
int eeprom_getmac(int index, uint8_t *enetaddr);
uint32_t eeprom_get_serial(void);
int get_bom_rev(const char *str);
char get_pcb_rev(const char *str);

#endif
