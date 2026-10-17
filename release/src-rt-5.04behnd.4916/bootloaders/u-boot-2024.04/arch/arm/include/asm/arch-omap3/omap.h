/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2006-2008
 * Texas Instruments, <www.ti.com>
 * Richard Woodruff <r-woodruff2@ti.com>
 * Syed Mohammed Khasim <x0khasim@ti.com>
 */

#ifndef _OMAP3_H_
#define _OMAP3_H_

#include <linux/sizes.h>

/* Stuff on L3 Interconnect */
#define SMX_APE_BASE			0x68000000

/* GPMC */
#define OMAP34XX_GPMC_BASE		0x6E000000

/* SMS */
#define OMAP34XX_SMS_BASE		0x6C000000

/* SDRC */
#define OMAP34XX_SDRC_BASE		0x6D000000

/*
 * L4 Peripherals - L4 Wakeup and L4 Core now
 */
#define OMAP34XX_CORE_L4_IO_BASE	0x48000000
#define OMAP34XX_WAKEUP_L4_IO_BASE	0x48300000
#define OMAP34XX_ID_L4_IO_BASE		0x4830A200
#define OMAP34XX_L4_PER			0x49000000
#define OMAP34XX_L4_IO_BASE		OMAP34XX_CORE_L4_IO_BASE

/* DMA4/SDMA */
#define OMAP34XX_DMA4_BASE              0x48056000

/* CONTROL */
#define OMAP34XX_CTRL_BASE		(OMAP34XX_L4_IO_BASE + 0x2000)

#ifndef __ASSEMBLY__
/* Signal Integrity Parameter Control Registers */
struct control_prog_io {
	unsigned char res[0x408];
	unsigned int io2;		/* 0x408 */
	unsigned char res2[0x38];
	unsigned int io0;		/* 0x444 */
	unsigned int io1;		/* 0x448 */
};
#endif /* __ASSEMBLY__ */

/* Bit definition for CONTROL_PROG_IO1 */
#define PRG_I2C2_PULLUPRESX		0x00000001

/* Scratchpad memory */
#define OMAP34XX_SCRATCHPAD		(OMAP34XX_CTRL_BASE + 0x910)

/* UART */
#define OMAP34XX_UART1			(OMAP34XX_L4_IO_BASE + 0x6a000)
#define OMAP34XX_UART2			(OMAP34XX_L4_IO_BASE + 0x6c000)
#define OMAP34XX_UART3			(OMAP34XX_L4_PER + 0x20000)
#define OMAP34XX_UART4			(OMAP34XX_L4_PER + 0x42000)

/* General Purpose Timers */
#define OMAP34XX_GPT1			0x48318000
#define OMAP34XX_GPT2			0x49032000
#define OMAP34XX_GPT3			0x49034000
#define OMAP34XX_GPT4			0x49036000
#define OMAP34XX_GPT5			0x49038000
#define OMAP34XX_GPT6			0x4903A000
#define OMAP34XX_GPT7			0x4903C000
#define OMAP34XX_GPT8			0x4903E000
#define OMAP34XX_GPT9			0x49040000
#define OMAP34XX_GPT10			0x48086000
#define OMAP34XX_GPT11			0x48088000
#define OMAP34XX_GPT12			0x48304000

/* WatchDog Timers (1 secure, 3 GP) */
#define WD1_BASE			0x4830C000
#define WD2_BASE			0x48314000
#define WD3_BASE			0x49030000

/* 32KTIMER */
#define SYNC_32KTIMER_BASE		0x48320000

#ifndef __ASSEMBLY__

struct s32ktimer {
	unsigned char res[0x10];
	unsigned int s32k_cr;		/* 0x10 */
};

#define DEVICE_TYPE_SHIFT		0x8
#define DEVICE_TYPE_MASK		(0x7 << DEVICE_TYPE_SHIFT)

#endif /* __ASSEMBLY__ */

#ifndef __ASSEMBLY__
struct gpio {
	unsigned char res1[0x34];
	unsigned int oe;		/* 0x34 */
	unsigned int datain;		/* 0x38 */
	unsigned char res2[0x54];
	unsigned int cleardataout;	/* 0x90 */
	unsigned int setdataout;	/* 0x94 */
};
#endif /* __ASSEMBLY__ */

#define GPIO0				(0x1 << 0)
#define GPIO1				(0x1 << 1)
#define GPIO2				(0x1 << 2)
#define GPIO3				(0x1 << 3)
#define GPIO4				(0x1 << 4)
#define GPIO5				(0x1 << 5)
#define GPIO6				(0x1 << 6)
#define GPIO7				(0x1 << 7)
#define GPIO8				(0x1 << 8)
#define GPIO9				(0x1 << 9)
#define GPIO10				(0x1 << 10)
#define GPIO11				(0x1 << 11)
#define GPIO12				(0x1 << 12)
#define GPIO13				(0x1 << 13)
#define GPIO14				(0x1 << 14)
#define GPIO15				(0x1 << 15)
#define GPIO16				(0x1 << 16)
#define GPIO17				(0x1 << 17)
#define GPIO18				(0x1 << 18)
#define GPIO19				(0x1 << 19)
#define GPIO20				(0x1 << 20)
#define GPIO21				(0x1 << 21)
#define GPIO22				(0x1 << 22)
#define GPIO23				(0x1 << 23)
#define GPIO24				(0x1 << 24)
#define GPIO25				(0x1 << 25)
#define GPIO26				(0x1 << 26)
#define GPIO27				(0x1 << 27)
#define GPIO28				(0x1 << 28)
#define GPIO29				(0x1 << 29)
#define GPIO30				(0x1 << 30)
#define GPIO31				(0x1 << 31)

/* base address for indirect vectors (internal boot mode) */
#define SRAM_OFFSET0			0x40000000
#define SRAM_OFFSET1			0x00200000
#define SRAM_OFFSET2			0x0000F800
#define SRAM_VECT_CODE			(SRAM_OFFSET0 | SRAM_OFFSET1 | \
					 SRAM_OFFSET2)
#define SRAM_CLK_CODE			(SRAM_VECT_CODE + 64)

#define NON_SECURE_SRAM_START		0x40208000 /* Works for GP & EMU */
#define NON_SECURE_SRAM_END		0x40210000
#define NON_SECURE_SRAM_IMG_END		0x4020F000
#define SRAM_SCRATCH_SPACE_ADDR		(NON_SECURE_SRAM_IMG_END - SZ_1K)

#define LOW_LEVEL_SRAM_STACK		0x4020FFFC

/* scratch area - accessible on both EMU and GP */
#define OMAP3_PUBLIC_SRAM_SCRATCH_AREA	NON_SECURE_SRAM_START

#define DEBUG_LED1			149	/* gpio */
#define DEBUG_LED2			150	/* gpio */

#define XDR_POP		5	/* package on package part */
#define SDR_DISCRETE	4	/* 128M memory SDR module */
#define DDR_STACKED	3	/* stacked part on 2422 */
#define DDR_COMBO	2	/* combo part on cpu daughter card */
#define DDR_DISCRETE	1	/* 2x16 parts on daughter card */

#define DDR_100		100	/* type found on most mem d-boards */
#define DDR_111		111	/* some combo parts */
#define DDR_133		133	/* most combo, some mem d-boards */
#define DDR_165		165	/* future parts */

#define CPU_3430	0x3430

/*
 * 343x real hardware:
 *  ES1     = rev 0
 *
 *  ES2 onwards, the value maps to contents of IDCODE register [31:28].
 *
 * Note : CPU_3XX_ES20 is used in cache.S.  Please review before changing.
 */
#define CPU_3XX_ES10		0
#define CPU_3XX_ES20		1
#define CPU_3XX_ES21		2
#define CPU_3XX_ES30		3
#define CPU_3XX_ES31		4
#define CPU_3XX_ES312		7
#define CPU_3XX_MAX_REV		8

/*
 * 37xx real hardware:
 * ES1.0 onwards, the value maps to contents of IDCODE register [31:28].
 */

#define CPU_37XX_ES10		0
#define CPU_37XX_ES11		1
#define CPU_37XX_ES12		2
#define CPU_37XX_MAX_REV	3

#define CPU_3XX_ID_SHIFT	28

#define WIDTH_8BIT		0x0000
#define WIDTH_16BIT		0x1000	/* bit pos for 16 bit in gpmc */

/*
 * Hawkeye values
 */
#define HAWKEYE_OMAP34XX	0xb7ae
#define HAWKEYE_AM35XX		0xb868
#define HAWKEYE_OMAP36XX	0xb891

#define HAWKEYE_SHIFT		12

/*
 * Define CPU families
 */
#define CPU_OMAP34XX		0x3400	/* OMAP34xx/OMAP35 devices */
#define CPU_AM35XX		0x3500	/* AM35xx devices          */
#define CPU_OMAP36XX		0x3600	/* OMAP36xx devices        */

/*
 * Control status register values corresponding to cpu variants
 */
#define OMAP3503		0x5c00
#define OMAP3515		0x1c00
#define OMAP3525		0x4c00
#define OMAP3530		0x0c00

#define AM3505			0x5c00
#define AM3517			0x1c00

#define OMAP3730		0x0c00
#define OMAP3725		0x4c00
#define AM3715			0x1c00
#define AM3703			0x5c00

#define OMAP3730_1GHZ		0x0e00
#define OMAP3725_1GHZ		0x4e00
#define AM3715_1GHZ		0x1e00
#define AM3703_1GHZ		0x5e00

/*
 * ROM code API related flags
 */
#define OMAP3_GP_ROMCODE_API_L2_INVAL		1
#define OMAP3_GP_ROMCODE_API_WRITE_L2ACR	2
#define OMAP3_GP_ROMCODE_API_WRITE_ACR		3

/*
 * EMU device PPA HAL related flags
 */
#define OMAP3_EMU_HAL_API_L2_INVAL		40
#define OMAP3_EMU_HAL_API_WRITE_ACR		42

#define OMAP3_EMU_HAL_START_HAL_CRITICAL	4

/* ABB settings */
#define OMAP_ABB_SETTLING_TIME		30
#define OMAP_ABB_CLOCK_CYCLES		8

/* ABB tranxdone mask */
#define OMAP_ABB_MPU_TXDONE_MASK	(0x1 << 26)

#define OMAP_REBOOT_REASON_OFFSET	0x04

/* Boot parameters */
#ifndef __ASSEMBLY__
struct omap_boot_parameters {
	unsigned int boot_message;
	unsigned char boot_device;
	unsigned char reserved;
	unsigned char reset_reason;
	unsigned char ch_flags;
	unsigned int boot_device_descriptor;
};

int omap_reboot_mode(char *mode, unsigned int length);
int omap_reboot_mode_clear(void);
int omap_reboot_mode_store(char *mode);
#endif

#endif
