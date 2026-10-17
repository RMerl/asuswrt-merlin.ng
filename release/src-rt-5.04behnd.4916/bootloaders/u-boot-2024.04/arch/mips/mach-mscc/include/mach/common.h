/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2018 Microsemi Corporation
 */

#ifndef __ASM_MACH_COMMON_H
#define __ASM_MACH_COMMON_H

#if defined(CONFIG_SOC_OCELOT)
#include <mach/ocelot/ocelot.h>
#include <mach/ocelot/ocelot_devcpu_gcb.h>
#include <mach/ocelot/ocelot_devcpu_gcb_miim_regs.h>
#include <mach/ocelot/ocelot_icpu_cfg.h>
#elif defined(CONFIG_SOC_LUTON)
#include <mach/luton/luton.h>
#include <mach/luton/luton_devcpu_gcb.h>
#include <mach/luton/luton_devcpu_gcb_miim_regs.h>
#include <mach/luton/luton_icpu_cfg.h>
#elif defined(CONFIG_SOC_JR2)
#include <mach/jr2/jr2.h>
#include <mach/jr2/jr2_devcpu_gcb.h>
#include <mach/jr2/jr2_devcpu_gcb_miim_regs.h>
#include <mach/jr2/jr2_icpu_cfg.h>
#elif defined(CONFIG_SOC_SERVALT)
#include <mach/servalt/servalt.h>
#include <mach/servalt/servalt_devcpu_gcb.h>
#include <mach/servalt/servalt_devcpu_gcb_miim_regs.h>
#include <mach/servalt/servalt_icpu_cfg.h>
#elif defined(CONFIG_SOC_SERVAL)
#include <mach/serval/serval.h>
#include <mach/serval/serval_devcpu_gcb.h>
#include <mach/serval/serval_devcpu_gcb_miim_regs.h>
#include <mach/serval/serval_icpu_cfg.h>
#else
#error Unsupported platform
#endif

#define MSCC_DDR_TO	0x20000000	/* DDR RAM base offset */
#define MSCC_MEMCTL1_TO	0x40000000	/* SPI/PI base offset */
#define MSCC_MEMCTL2_TO	0x50000000	/* SPI/PI base offset */
#define MSCC_FLASH_TO	MSCC_MEMCTL1_TO	/* Flash base offset */

#define VCOREIII_TIMER_DIVIDER 25	/* Clock tick ~ 0.1 us */

/* Common utility functions */

/*
 * Perform a number of NOP instructions, blocks of 8 instructions.
 * The (inlined) function will not affect cache or processor state.
 */
static inline void mscc_vcoreiii_nop_delay(int delay)
{
	while (delay > 0) {
#define DELAY_8_NOPS() asm volatile("nop; nop; nop; nop; nop; nop; nop; nop;")
		switch (delay) {
		case 8:
			DELAY_8_NOPS();
			/* fallthrough */
		case 7:
			DELAY_8_NOPS();
			/* fallthrough */
		case 6:
			DELAY_8_NOPS();
			/* fallthrough */
		case 5:
			DELAY_8_NOPS();
			/* fallthrough */
		case 4:
			DELAY_8_NOPS();
			/* fallthrough */
		case 3:
			DELAY_8_NOPS();
			/* fallthrough */
		case 2:
			DELAY_8_NOPS();
			/* fallthrough */
		case 1:
			DELAY_8_NOPS();
		}
		delay -= 8;
#undef DELAY_8_NOPS
	}
}

int mscc_phy_rd_wr(u8 read,
		   u32 miim_controller,
		   u8 miim_addr,
		   u8 addr,
		   u16 *value);

int mscc_phy_rd(u32 miim_controller,
		u8 miim_addr,
		u8 addr,
		u16 *value);

int mscc_phy_wr(u32 miim_controller,
		u8 miim_addr,
		u8 addr,
		u16 value);

void mscc_gpio_set_alternate(int gpio, int mode);

#endif				/* __ASM_MACH_COMMON_H */
