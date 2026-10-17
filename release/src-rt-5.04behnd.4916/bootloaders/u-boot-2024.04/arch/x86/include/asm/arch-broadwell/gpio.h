/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2016 Google, Inc
 *
 * From Coreboot src/soc/intel/broadwell/include/soc/gpio.h
 */

#ifndef __ASM_ARCH_GPIO
#define __ASM_ARCH_GPIO

#define GPIO_PER_BANK	32
#define GPIO_BANKS	3

struct broadwell_bank_plat {
	uint16_t base_addr;
	const char *bank_name;
	int bank;
};

/* PCH-LP GPIOBASE Registers */
struct pch_lp_gpio_regs {
	u32 own[GPIO_BANKS];
	u32 reserved0;

	u16 pirq_to_ioxapic;
	u16 reserved1[3];
	u32 blink;
	u32 ser_blink;

	u32 ser_blink_cmdsts;
	u32 ser_blink_data;
	u16 gpi_nmi_en;
	u16 gpi_nmi_sts;
	u32 reserved2;

	u32 gpi_route[GPIO_BANKS];
	u32 reserved3;

	u32 reserved4[4];

	u32 alt_gpi_smi_sts;
	u32 alt_gpi_smi_en;
	u32 reserved5[2];

	u32 rst_sel[GPIO_BANKS];
	u32 reserved6;

	u32 reserved9[3];
	u32 gpio_gc;

	u32 gpi_is[GPIO_BANKS];
	u32 reserved10;

	u32 gpi_ie[GPIO_BANKS];
	u32 reserved11;

	u32 reserved12[24];

	struct {
		u32 conf_a;
		u32 conf_b;
	} config[GPIO_BANKS * GPIO_PER_BANK];
};
check_member(pch_lp_gpio_regs, gpi_ie[0], 0x90);
check_member(pch_lp_gpio_regs, config[0], 0x100);

enum {
	CONFA_MODE_SHIFT	= 0,
	CONFA_MODE_GPIO		= 1 << CONFA_MODE_SHIFT,

	CONFA_DIR_SHIFT		= 2,
	CONFA_DIR_INPUT		= 1 << CONFA_DIR_SHIFT,

	CONFA_INVERT_SHIFT	= 3,
	CONFA_INVERT		= 1 << CONFA_INVERT_SHIFT,

	CONFA_TRIGGER_SHIFT	= 4,
	CONFA_TRIGGER_LEVEL	= 1 << CONFA_TRIGGER_SHIFT,

	CONFA_LEVEL_SHIFT	= 30,
	CONFA_LEVEL_HIGH	= 1UL << CONFA_LEVEL_SHIFT,

	CONFA_OUTPUT_SHIFT	= 31,
	CONFA_OUTPUT_HIGH	= 1UL << CONFA_OUTPUT_SHIFT,

	CONFB_SENSE_SHIFT	= 2,
	CONFB_SENSE_DISABLE	= 1 << CONFB_SENSE_SHIFT,
};

#endif
