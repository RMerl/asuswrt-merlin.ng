/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2017, Bin Meng <bmeng.cn@gmail.com>
 *
 * From coreboot src/soc/intel/braswell/include/soc/gpio.h
 */

#ifndef _BRASWELL_GPIO_H_
#define _BRASWELL_GPIO_H_

#include <asm/arch/iomap.h>

enum mode_list {
	M0,
	M1,
	M2,
	M3,
	M4,
	M5,
	M6,
	M7,
	M8,
	M9,
	M10,
	M11,
	M12,
	M13,
};

enum int_select {
	L0,
	L1,
	L2,
	L3,
	L4,
	L5,
	L6,
	L7,
	L8,
	L9,
	L10,
	L11,
	L12,
	L13,
	L14,
	L15,
};

enum gpio_en {
	NATIVE = 0xff,
	GPIO = 0,	/* Native, no need to set PAD_VALUE */
	GPO = 1,	/* GPO, output only in PAD_VALUE */
	GPI = 2,	/* GPI, input only in PAD_VALUE */
	HI_Z = 3,
	NA_GPO = 0,
};

enum gpio_state {
	LOW,
	HIGH,
};

enum en_dis {
	DISABLE,	/* Disable */
	ENABLE,		/* Enable */
};

enum int_type {
	INT_DIS,
	TRIG_EDGE_LOW,
	TRIG_EDGE_HIGH,
	TRIG_EDGE_BOTH,
	TRIG_LEVEL,
};

enum mask {
	MASKABLE,
	NON_MASKABLE,
};

enum glitch_cfg {
	GLITCH_DISABLE,
	EN_EDGE_DETECT,
	EN_RX_DATA,
	EN_EDGE_RX_DATA,
};

enum inv_rx_tx {
	NO_INVERSION = 0,
	INV_RX_ENABLE = 1,
	INV_TX_ENABLE = 2,
	INV_RX_TX_ENABLE = 3,
	INV_RX_DATA = 4,
	INV_TX_DATA = 8,
};

enum voltage {
	VOLT_3_3,	/* Working on 3.3 Volts */
	VOLT_1_8,	/* Working on 1.8 Volts */
};

enum hs_mode {
	DISABLE_HS,	/* Disable high speed mode */
	ENABLE_HS,	/* Enable high speed mode */
};

enum odt_up_dn {
	PULL_UP,	/* On Die Termination Up */
	PULL_DOWN,	/* On Die Termination Down */
};

enum odt_en {
	DISABLE_OD,	/* On Die Termination Disable */
	ENABLE_OD,	/* On Die Termination Enable */
};

enum pull_type {
	P_NONE  = 0,	/* Pull None */
	P_20K_L = 1,	/* Pull Down 20K */
	P_5K_L  = 2,	/* Pull Down  5K */
	P_1K_L  = 4,	/* Pull Down  1K */
	P_20K_H = 9,	/* Pull Up 20K */
	P_5K_H  = 10,	/* Pull Up  5K */
	P_1K_H  = 12	/* Pull Up  1K */
};

enum bit {
	ONE_BIT = 1,
	TWO_BIT = 3,
	THREE_BIT = 7,
	FOUR_BIT = 15,
	FIVE_BIT = 31,
	SIX_BIT = 63,
	SEVEN_BIT = 127,
	EIGHT_BIT = 255
};

enum gpe_config {
	GPE,
	SMI,
	SCI,
};

enum community {
	SOUTHWEST = 0x0000,
	NORTH = 0x8000,
	EAST = 0x10000,
	SOUTHEAST = 0x18000,
	VIRTUAL = 0x20000,
};

#define NA		0xff
#define TERMINATOR	0xffffffff

#define GPIO_FAMILY_CONF(family_name, park_mode, hysctl, vp18_mode, hs_mode, \
	odt_up_dn, odt_en, curr_src_str, rcomp, family_no, community_offset) { \
	.confg = ((((park_mode) != NA) ? park_mode << 26 : 0) | \
		  (((hysctl) != NA) ? hysctl << 24 : 0) | \
		  (((vp18_mode) != NA) ? vp18_mode << 21 : 0) | \
		  (((hs_mode) != NA) ? hs_mode << 19 : 0) | \
		  (((odt_up_dn) != NA) ? odt_up_dn << 18 : 0) | \
		  (((odt_en) != NA) ? odt_en << 17 : 0) | \
		  (curr_src_str)), \
	.confg_changes = ((((park_mode) != NA) ? ONE_BIT << 26 : 0) | \
			  (((hysctl) != NA) ? TWO_BIT << 24 : 0) | \
			  (((vp18_mode) != NA) ? ONE_BIT  << 21 : 0) | \
			  (((hs_mode) != NA) ? ONE_BIT << 19 : 0) | \
			  (((odt_up_dn) != NA) ? ONE_BIT << 18 : 0) | \
			  (((odt_en) != NA) ? ONE_BIT << 17 : 0) | \
			  (THREE_BIT)), \
	.misc = ((rcomp == ENABLE) ? 1 : 0) , \
	.mmio_addr = (community_offset == TERMINATOR) ? TERMINATOR : \
		     ((family_no != NA) ? (IO_BASE_ADDRESS + community_offset +\
		     (0x80 * family_no) + 0x1080) : 0) , \
	.name = 0 \
}

#define GPIO_PAD_CONF(pad_name, mode_select, mode, gpio_config, gpio_state, \
	gpio_light_mode, int_type, int_sel, term, open_drain, current_source,\
	int_mask, glitch, inv_rx_tx, wake_mask, wake_mask_bit, gpe, \
	mmio_offset, community_offset) { \
	.confg0 = ((((int_sel) != NA) ? (int_sel << 28) : 0) | \
		   (((glitch) != NA) ? (glitch << 26) : 0) | \
		   (((term) != NA) ? (term << 20) : 0) | \
		   (((mode_select) == GPIO) ? ((mode << 16) | (1 << 15)) : \
		    ((mode << 16))) | \
		   (((gpio_config) != NA) ? (gpio_config << 8) : 0) | \
		   (((gpio_light_mode) != NA) ? (gpio_light_mode << 7) : 0) | \
		   (((gpio_state) == HIGH) ? 2 : 0)), \
	.confg0_changes = ((((int_sel) != NA) ? (FOUR_BIT << 28) : 0) | \
			   (((glitch) != NA) ? (TWO_BIT << 26) : 0) | \
			   (((term) != NA) ? (FOUR_BIT << 20) : 0) | \
			   (FIVE_BIT << 15) | \
			   (((gpio_config) != NA) ? (THREE_BIT << 8) : 0) | \
			   (((gpio_light_mode) != NA) ? (ONE_BIT << 7) : 0) | \
			   (((gpio_state) != NA) ? ONE_BIT << 1 : 0)), \
	.confg1  = ((((current_source) != NA) ? (current_source << 27) : 0) | \
		    (((inv_rx_tx) != NA) ? inv_rx_tx << 4 : 0) | \
		    (((open_drain) != NA) ? open_drain << 3 : 0) | \
		    (((int_type) != NA) ? int_type : 0)), \
	.confg1_changes = ((((current_source) != NA) ? (ONE_BIT << 27) : 0) | \
			   (((inv_rx_tx) != NA) ? FOUR_BIT << 4 : 0) | \
			   (((open_drain) != NA) ? ONE_BIT << 3 : 0) | \
			   (((int_type) != NA) ? THREE_BIT : 0)), \
	.community = community_offset, \
	.mmio_addr = (community_offset == TERMINATOR) ? TERMINATOR : \
		     ((mmio_offset != NA) ? (IO_BASE_ADDRESS + \
		      community_offset + mmio_offset) : 0), \
	.name = 0, \
	.misc = ((((gpe) != NA) ? (gpe << 0) : 0) | \
		 (((wake_mask) != NA) ? (wake_mask << 2) : 0) | \
		 (((int_mask) != NA) ? (int_mask << 3) : 0)) | \
		 (((wake_mask_bit) != NA) ? (wake_mask_bit << 4) : (NA << 4)) \
}

#endif /* _BRASWELL_GPIO_H_ */
