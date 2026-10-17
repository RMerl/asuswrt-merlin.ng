/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2010
 * Texas Instruments, <www.ti.com>
 *
 * Authors:
 *	Aneesh V <aneesh@ti.com>
 *
 * Derived from OMAP3 work by
 *	Richard Woodruff <r-woodruff2@ti.com>
 *	Syed Mohammed Khasim <x0khasim@ti.com>
 */

#ifndef _OMAP4_H_
#define _OMAP4_H_

#if !(defined(__KERNEL_STRICT_NAMES) || defined(__ASSEMBLY__))
#include <asm/types.h>
#endif /* !(__KERNEL_STRICT_NAMES || __ASSEMBLY__) */

#include <linux/sizes.h>

/*
 * L4 Peripherals - L4 Wakeup and L4 Core now
 */
#define OMAP44XX_L4_CORE_BASE	0x4A000000
#define OMAP44XX_L4_WKUP_BASE	0x4A300000
#define OMAP44XX_L4_PER_BASE	0x48000000

#define OMAP44XX_DRAM_ADDR_SPACE_START	0x80000000
#define OMAP44XX_DRAM_ADDR_SPACE_END	0xD0000000
#define DRAM_ADDR_SPACE_START	OMAP44XX_DRAM_ADDR_SPACE_START
#define DRAM_ADDR_SPACE_END	OMAP44XX_DRAM_ADDR_SPACE_END

/* CONTROL_ID_CODE */
#define CONTROL_ID_CODE		0x4A002204

#define OMAP4_CONTROL_ID_CODE_ES1_0	0x0B85202F
#define OMAP4_CONTROL_ID_CODE_ES2_0	0x1B85202F
#define OMAP4_CONTROL_ID_CODE_ES2_1	0x3B95C02F
#define OMAP4_CONTROL_ID_CODE_ES2_2	0x4B95C02F
#define OMAP4_CONTROL_ID_CODE_ES2_3	0x6B95C02F
#define OMAP4460_CONTROL_ID_CODE_ES1_0	0x0B94E02F
#define OMAP4460_CONTROL_ID_CODE_ES1_1	0x2B94E02F
#define OMAP4470_CONTROL_ID_CODE_ES1_0	0x0B97502F

/* UART */
#define UART1_BASE		(OMAP44XX_L4_PER_BASE + 0x6a000)
#define UART2_BASE		(OMAP44XX_L4_PER_BASE + 0x6c000)
#define UART3_BASE		(OMAP44XX_L4_PER_BASE + 0x20000)

/* General Purpose Timers */
#define GPT1_BASE		(OMAP44XX_L4_WKUP_BASE + 0x18000)
#define GPT2_BASE		(OMAP44XX_L4_PER_BASE  + 0x32000)
#define GPT3_BASE		(OMAP44XX_L4_PER_BASE  + 0x34000)

/* Watchdog Timer2 - MPU watchdog */
#define WDT2_BASE		(OMAP44XX_L4_WKUP_BASE + 0x14000)

/*
 * Hardware Register Details
 */

/* Watchdog Timer */
#define WD_UNLOCK1		0xAAAA
#define WD_UNLOCK2		0x5555

/* GP Timer */
#define TCLR_ST			(0x1 << 0)
#define TCLR_AR			(0x1 << 1)
#define TCLR_PRE		(0x1 << 5)

/* Control Module */
#define LDOSRAM_ACTMODE_VSET_IN_MASK	(0x1F << 5)
#define LDOSRAM_VOLT_CTRL_OVERRIDE	0x0401040f
#define CONTROL_EFUSE_1_OVERRIDE	0x1C4D0110
#define CONTROL_EFUSE_2_OVERRIDE	0x99084000

/* LPDDR2 IO regs */
#define CONTROL_LPDDR2IO_SLEW_125PS_DRV8_PULL_DOWN	0x1C1C1C1C
#define CONTROL_LPDDR2IO_SLEW_325PS_DRV8_GATE_KEEPER	0x9E9E9E9E
#define CONTROL_LPDDR2IO_SLEW_315PS_DRV12_PULL_DOWN	0x7C7C7C7C
#define LPDDR2IO_GR10_WD_MASK				(3 << 17)
#define CONTROL_LPDDR2IO_3_VAL		0xA0888C0F

/* CONTROL_EFUSE_2 */
#define CONTROL_EFUSE_2_NMOS_PMOS_PTV_CODE_1		0x00ffc000

#define MMC1_PWRDNZ					(1 << 26)
#define MMC1_PBIASLITE_PWRDNZ				(1 << 22)
#define MMC1_PBIASLITE_VMODE				(1 << 21)

#ifndef __ASSEMBLY__

struct s32ktimer {
	unsigned char res[0x10];
	unsigned int s32k_cr;	/* 0x10 */
};

#define DEVICE_TYPE_SHIFT (0x8)
#define DEVICE_TYPE_MASK (0x7 << DEVICE_TYPE_SHIFT)

#endif /* __ASSEMBLY__ */

/*
 * Non-secure SRAM Addresses
 * Non-secure RAM starts at 0x40300000 for GP devices. But we keep SRAM_BASE
 * at 0x40304000(EMU base) so that our code works for both EMU and GP
 */
#define NON_SECURE_SRAM_START	0x40304000
#define NON_SECURE_SRAM_END	0x4030E000	/* Not inclusive */
#define NON_SECURE_SRAM_IMG_END	0x4030C000
#define SRAM_SCRATCH_SPACE_ADDR	(NON_SECURE_SRAM_IMG_END - SZ_1K)
/* base address for indirect vectors (internal boot mode) */
#define SRAM_ROM_VECT_BASE	0x4030D000

/* ABB settings */
#define OMAP_ABB_SETTLING_TIME		50
#define OMAP_ABB_CLOCK_CYCLES		16

/* ABB tranxdone mask */
#define OMAP_ABB_MPU_TXDONE_MASK	(0x1 << 7)

#define OMAP44XX_SAR_RAM_BASE		0x4a326000
#define OMAP_REBOOT_REASON_OFFSET	0xA0C
#define OMAP_REBOOT_REASON_SIZE		0x0F

/* Boot parameters */
#ifndef __ASSEMBLY__
struct omap_boot_parameters {
	unsigned int boot_message;
	unsigned int boot_device_descriptor;
	unsigned char boot_device;
	unsigned char reset_reason;
	unsigned char ch_flags;
};

int omap_reboot_mode(char *mode, unsigned int length);
int omap_reboot_mode_clear(void);
int omap_reboot_mode_store(char *mode);
#endif

#endif
