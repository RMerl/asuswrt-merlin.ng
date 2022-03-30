/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2010
 * Texas Instruments, <www.ti.com>
 *
 * Authors:
 *	Aneesh V <aneesh@ti.com>
 *	Sricharan R <r.sricharan@ti.com>
 */

#ifndef _OMAP5_H_
#define _OMAP5_H_

#if !(defined(__KERNEL_STRICT_NAMES) || defined(__ASSEMBLY__))
#include <asm/types.h>
#endif /* !(__KERNEL_STRICT_NAMES || __ASSEMBLY__) */

#include <linux/sizes.h>

/*
 * L4 Peripherals - L4 Wakeup and L4 Core now
 */
#define OMAP54XX_L4_CORE_BASE	0x4A000000
#define OMAP54XX_L4_WKUP_BASE	0x4Ae00000
#define OMAP54XX_L4_PER_BASE	0x48000000

/* CONTROL ID CODE */
#define CONTROL_CORE_ID_CODE	0x4A002204
#define CONTROL_WKUP_ID_CODE	0x4AE0C204

#if defined(CONFIG_DRA7XX)
#define CONTROL_ID_CODE		CONTROL_WKUP_ID_CODE
#else
#define CONTROL_ID_CODE		CONTROL_CORE_ID_CODE
#endif

#if defined(CONFIG_DRA7XX)
#define DRA7_USB_OTG_SS1_BASE		0x48890000
#define DRA7_USB_OTG_SS1_GLUE_BASE	0x48880000
#define DRA7_USB3_PHY1_PLL_CTRL		0x4A084C00
#define DRA7_USB3_PHY1_POWER		0x4A002370
#define DRA7_USB2_PHY1_POWER		0x4A002300

#define DRA7_USB_OTG_SS2_BASE		0x488D0000
#define DRA7_USB_OTG_SS2_GLUE_BASE	0x488C0000
#define DRA7_USB2_PHY2_POWER		0x4A002E74
#else
#define OMAP5XX_USB_OTG_SS_BASE		0x4A030000
#define OMAP5XX_USB_OTG_SS_GLUE_BASE	0x4A020000
#define OMAP5XX_USB3_PHY_PLL_CTRL	0x4A084C00
#define OMAP5XX_USB3_PHY_POWER		0x4A002370
#define OMAP5XX_USB2_PHY_POWER		0x4A002300
#endif

/* To be verified */
#define OMAP5430_CONTROL_ID_CODE_ES1_0		0x0B94202F
#define OMAP5430_CONTROL_ID_CODE_ES2_0          0x1B94202F
#define OMAP5432_CONTROL_ID_CODE_ES1_0		0x0B99802F
#define OMAP5432_CONTROL_ID_CODE_ES2_0          0x1B99802F
#define DRA762_CONTROL_ID_CODE_ES1_0		0x0BB5002F
#define DRA752_CONTROL_ID_CODE_ES1_0		0x0B99002F
#define DRA752_CONTROL_ID_CODE_ES1_1		0x1B99002F
#define DRA752_CONTROL_ID_CODE_ES2_0		0x2B99002F
#define DRA722_CONTROL_ID_CODE_ES1_0		0x0B9BC02F
#define DRA722_CONTROL_ID_CODE_ES2_0		0x1B9BC02F
#define DRA722_CONTROL_ID_CODE_ES2_1		0x2B9BC02F

#define DRA762_ABZ_PACKAGE			0x2
#define DRA762_ACD_PACKAGE			0x3

/* UART */
#define UART1_BASE		(OMAP54XX_L4_PER_BASE + 0x6a000)
#define UART2_BASE		(OMAP54XX_L4_PER_BASE + 0x6c000)
#define UART3_BASE		(OMAP54XX_L4_PER_BASE + 0x20000)
#define UART4_BASE		(OMAP54XX_L4_PER_BASE + 0x6e000)

/* General Purpose Timers */
#define GPT1_BASE		(OMAP54XX_L4_WKUP_BASE + 0x18000)
#define GPT2_BASE		(OMAP54XX_L4_PER_BASE  + 0x32000)
#define GPT3_BASE		(OMAP54XX_L4_PER_BASE  + 0x34000)

/* Watchdog Timer2 - MPU watchdog */
#define WDT2_BASE		(OMAP54XX_L4_WKUP_BASE + 0x14000)

/* QSPI */
#define QSPI_BASE		0x4B300000

/* SATA */
#define DWC_AHSATA_BASE		0x4A140000

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
#define CONTROL_EFUSE_2_OVERRIDE	0x00084000

/* LPDDR2 IO regs */
#define CONTROL_LPDDR2IO_SLEW_125PS_DRV8_PULL_DOWN	0x1C1C1C1C
#define CONTROL_LPDDR2IO_SLEW_325PS_DRV8_GATE_KEEPER	0x9E9E9E9E
#define CONTROL_LPDDR2IO_SLEW_315PS_DRV12_PULL_DOWN	0x7C7C7C7C
#define LPDDR2IO_GR10_WD_MASK				(3 << 17)
#define CONTROL_LPDDR2IO_3_VAL		0xA0888C00

/* CONTROL_EFUSE_2 */
#define CONTROL_EFUSE_2_NMOS_PMOS_PTV_CODE_1		0x00ffc000

#define SDCARD_BIAS_PWRDNZ				(1 << 27)
#define SDCARD_PWRDNZ					(1 << 26)
#define SDCARD_BIAS_HIZ_MODE				(1 << 25)
#define SDCARD_PBIASLITE_VMODE				(1 << 21)

#ifndef __ASSEMBLY__

struct s32ktimer {
	unsigned char res[0x10];
	unsigned int s32k_cr;	/* 0x10 */
};

#define DEVICE_TYPE_SHIFT 0x6
#define DEVICE_TYPE_MASK (0x7 << DEVICE_TYPE_SHIFT)

/* Output impedance control */
#define ds_120_ohm	0x0
#define ds_60_ohm	0x1
#define ds_45_ohm	0x2
#define ds_30_ohm	0x3
#define ds_mask		0x3

/* Slew rate control */
#define sc_slow		0x0
#define sc_medium	0x1
#define sc_fast		0x2
#define sc_na		0x3
#define sc_mask		0x3

/* Target capacitance control */
#define lb_5_12_pf	0x0
#define lb_12_25_pf	0x1
#define lb_25_50_pf	0x2
#define lb_50_80_pf	0x3
#define lb_mask		0x3

#define usb_i_mask	0x7

#define DDR_IO_I_34OHM_SR_FASTEST_WD_DQ_NO_PULL_DQS_PULL_DOWN   0x80828082
#define DDR_IO_I_34OHM_SR_FASTEST_WD_CK_CKE_NCS_CA_PULL_DOWN 0x82828200
#define DDR_IO_0_DDR2_DQ_INT_EN_ALL_DDR3_CA_DIS_ALL 0x8421
#define DDR_IO_1_DQ_OUT_EN_ALL_DQ_INT_EN_ALL 0x8421084
#define DDR_IO_2_CA_OUT_EN_ALL_CA_INT_EN_ALL 0x8421000

#define DDR_IO_I_40OHM_SR_SLOWEST_WD_DQ_NO_PULL_DQS_NO_PULL	0x7C7C7C6C
#define DDR_IO_I_40OHM_SR_FAST_WD_DQ_NO_PULL_DQS_NO_PULL	0x64646464
#define DDR_IO_0_VREF_CELLS_DDR3_VALUE				0xBAE8C631
#define DDR_IO_1_VREF_CELLS_DDR3_VALUE				0xBC6318DC
#define DDR_IO_2_VREF_CELLS_DDR3_VALUE				0x0

#define DDR_IO_I_40OHM_SR_SLOWEST_WD_DQ_NO_PULL_DQS_NO_PULL_ES2 0x7C7C7C7C
#define DDR_IO_I_40OHM_SR_FAST_WD_DQ_NO_PULL_DQS_NO_PULL_ES2 0x64646464
#define DDR_IO_0_VREF_CELLS_DDR3_VALUE_ES2 0xBAE8C631
#define DDR_IO_1_VREF_CELLS_DDR3_VALUE_ES2 0xBC6318DC
#define DDR_IO_2_VREF_CELLS_DDR3_VALUE_ES2 0x84210000

#define EFUSE_1 0x45145100
#define EFUSE_2 0x45145100
#define EFUSE_3 0x45145100
#define EFUSE_4 0x45145100
#endif /* __ASSEMBLY__ */

/*
 * In all cases, the TRM defines the RAM Memory Map for the processor
 * and indicates the area for the downloaded image.  We use all of that
 * space for download and once up and running may use other parts of the
 * map for our needs.  We set a scratch space that is at the end of the
 * OMAP5 download area, but within the DRA7xx download area (as it is
 * much larger) and do not, at this time, make use of the additional
 * space.
 */
#if defined(CONFIG_DRA7XX)
#define NON_SECURE_SRAM_START	0x40300000
#define NON_SECURE_SRAM_END	0x40380000	/* Not inclusive */
#define NON_SECURE_SRAM_IMG_END	0x4037C000
#else
#define NON_SECURE_SRAM_START	0x40300000
#define NON_SECURE_SRAM_END	0x40320000	/* Not inclusive */
#define NON_SECURE_SRAM_IMG_END	0x4031E000
#endif
#define SRAM_SCRATCH_SPACE_ADDR	(NON_SECURE_SRAM_IMG_END - SZ_1K)

/* base address for indirect vectors (internal boot mode) */
#define SRAM_ROM_VECT_BASE	0x4031F000

/* CONTROL_SRCOMP_XXX_SIDE */
#define OVERRIDE_XS_SHIFT		30
#define OVERRIDE_XS_MASK		(1 << 30)
#define SRCODE_READ_XS_SHIFT		12
#define SRCODE_READ_XS_MASK		(0xff << 12)
#define PWRDWN_XS_SHIFT			11
#define PWRDWN_XS_MASK			(1 << 11)
#define DIVIDE_FACTOR_XS_SHIFT		4
#define DIVIDE_FACTOR_XS_MASK		(0x7f << 4)
#define MULTIPLY_FACTOR_XS_SHIFT	1
#define MULTIPLY_FACTOR_XS_MASK		(0x7 << 1)
#define SRCODE_OVERRIDE_SEL_XS_SHIFT	0
#define SRCODE_OVERRIDE_SEL_XS_MASK	(1 << 0)

/* ABB settings */
#define OMAP_ABB_SETTLING_TIME		50
#define OMAP_ABB_CLOCK_CYCLES		16

/* ABB tranxdone mask */
#define OMAP_ABB_MPU_TXDONE_MASK		(0x1 << 7)
#define OMAP_ABB_MM_TXDONE_MASK			(0x1 << 31)
#define OMAP_ABB_IVA_TXDONE_MASK		(0x1 << 30)
#define OMAP_ABB_EVE_TXDONE_MASK		(0x1 << 29)
#define OMAP_ABB_GPU_TXDONE_MASK		(0x1 << 28)

/* ABB efuse masks */
#define OMAP5_PROD_ABB_FUSE_VSET_MASK		(0x1F << 20)
#define OMAP5_PROD_ABB_FUSE_ENABLE_MASK		(0x1 << 25)
#define DRA7_ABB_FUSE_VSET_MASK			(0x1F << 20)
#define DRA7_ABB_FUSE_ENABLE_MASK		(0x1 << 25)
#define OMAP5_ABB_LDOVBBMPU_MUX_CTRL_MASK	(0x1 << 10)
#define OMAP5_ABB_LDOVBBMPU_VSET_OUT_MASK	(0x1f << 0)

#ifndef __ASSEMBLY__
struct srcomp_params {
	s8 divide_factor;
	s8 multiply_factor;
};

struct ctrl_ioregs {
	u32 ctrl_ddrch;
	u32 ctrl_lpddr2ch;
	u32 ctrl_ddr3ch;
	u32 ctrl_ddrio_0;
	u32 ctrl_ddrio_1;
	u32 ctrl_ddrio_2;
	u32 ctrl_emif_sdram_config_ext;
	u32 ctrl_emif_sdram_config_ext_final;
	u32 ctrl_ddr_ctrl_ext_0;
};

void clrset_spare_register(u8 spare_type, u32 clear_bits, u32 set_bits);

#endif /* __ASSEMBLY__ */

/* Boot parameters */
#ifndef __ASSEMBLY__
struct omap_boot_parameters {
	unsigned int boot_message;
	unsigned int boot_device_descriptor;
	unsigned char boot_device;
	unsigned char reset_reason;
	unsigned char ch_flags;
};
#endif

#endif
