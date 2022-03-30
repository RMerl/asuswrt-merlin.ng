/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2008 Stefan Roese <sr@denx.de>, DENX Software Engineering
 *
 * Copyright (C) 2006 Micronas GmbH
 */

#include <asm/io.h>

#include "bcu.h"
#include "dcgu.h"
#include "ebi.h"
#include "scc.h"

#ifdef CONFIG_VCT_PREMIUM
/* Global start address of all memory mapped registers */
#define REG_GLOBAL_START_ADDR	0xbf800000
#define TOP_BASE		0x000c8000

#include "vcth/reg_ebi.h"
#include "vcth/reg_dcgu.h"
#include "vcth/reg_wdt.h"
#include "vcth/reg_gpio.h"
#include "vcth/reg_fwsram.h"
#include "vcth/reg_scc.h"
#include "vcth/reg_usbh.h"
#endif

#ifdef CONFIG_VCT_PLATINUM
/* Global start address of all memory mapped registers */
#define REG_GLOBAL_START_ADDR	0xbf800000
#define TOP_BASE		0x000c8000

#include "vcth2/reg_ebi.h"
#include "vcth/reg_dcgu.h"
#include "vcth/reg_wdt.h"
#include "vcth/reg_gpio.h"
#include "vcth/reg_fwsram.h"
#include "vcth/reg_scc.h"
#include "vcth/reg_usbh.h"
#endif

#ifdef CONFIG_VCT_PLATINUMAVC
/* Global start address of all memory mapped registers */
#define REG_GLOBAL_START_ADDR	0xbdc00000
#define TOP_BASE		0x00050000

#include "vctv/reg_ebi.h"
#include "vctv/reg_dcgu.h"
#include "vctv/reg_wdt.h"
#include "vctv/reg_gpio.h"
#endif

#ifndef _VCT_H
#define _VCT_H

/*
 * Defines
 */
#define PRID_COMP_LEGACY	0x000000
#define PRID_COMP_MIPS		0x010000
#define PRID_IMP_LX4280		0xc200
#define PRID_IMP_VGC		0x9000

/*
 * Prototypes
 */
int ebi_initialize(void);
int ebi_init_nor_flash(void);
int ebi_init_onenand(void);
int ebi_init_smc911x(void);
u32 smc911x_reg_read(u32 addr);
void smc911x_reg_write(u32 addr, u32 data);
int top_set_pin(int pin, int func);
void vct_pin_mux_initialize(void);

/*
 * static inlines
 */
static inline void reg_write(u32 addr, u32 data)
{
	void *reg = (void *)(addr + REG_GLOBAL_START_ADDR);
	__raw_writel(data, reg);
}

static inline u32 reg_read(u32 addr)
{
	const void *reg = (const void *)(addr + REG_GLOBAL_START_ADDR);
	return __raw_readl(reg);
}

#endif /* _VCT_H */
