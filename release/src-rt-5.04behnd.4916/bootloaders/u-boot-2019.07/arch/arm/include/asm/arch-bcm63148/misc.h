/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2019 Broadcom Ltd.
 */

#ifndef _63148_MISC_H
#define _63148_MISC_H

#define MISC_BASE                               0xfffe8180
#define GPIO_BASE                               0xfffe8100

#define MISC_STRAP_BUS                          0x04
#define MISC_STRAP_BUS_OVERRIDE                 0x08

#define MISC_STRAP_BUS_SW_RESERVE_1		(0x3 << 24)
#define MISC_STRAP_BUS_BISR_MEM_REPAIR		(1 << 23)
#define MISC_STRAP_BUS_RESET_OUT_DELAY_SHIFT	22
#define MISC_STRAP_BUS_RESET_OUT_DELAY_MASK	(1 << MISC_STRAP_BUS_RESET_OUT_DELAY_SHIFT)
#define MISC_STRAP_BUS_RESET_OUT_DELAY_100MS	(1 << MISC_STRAP_BUS_RESET_OUT_DELAY_SHIFT)
#define MISC_STRAP_BUS_RESET_OUT_DELAY_50MS	(0x0 << MISC_STRAP_BUS_RESET_OUT_DELAY_SHIFT)
#define MISC_STRAP_BUS_SYS_BUS_FREQ		(0x3 << 20)
#define MISC_STRAP_BUS_A9_CORE0_BOOT		(1 << 19)
#define MISC_STRAP_BUS_PMC_BOOT_FLASH_N		(1 << 18)
#define MISC_STRAP_BUS_PMC_BOOT_AVS		(1 << 17)
#define MISC_STRAP_BUS_HS_SPIM_24B_N_32B_ADDR   (1 << 16)
#define MISC_STRAP_BUS_HS_SPIM_CLK_SLOW_N_FAST  (1 << 15)
#define MISC_STRAP_BUS_SW_RESERVE_0		(0x7 << 12)
#define MISC_STRAP_BUS_B15_START_SLOW_FREQ	(1 << 11)
#define MISC_STRAP_BUS_PMC_ROM_BOOT_SHIFT       10
#define MISC_STRAP_BUS_PMC_ROM_BOOT             (1<<MISC_STRAP_BUS_PMC_ROM_BOOT_SHIFT) /* pmc rom boot enable */
#define MISC_STRAP_BUS_PICO_ROM_BOOT		(1 << 9)
#define MISC_STRAP_BUS_BOOT_SEL_SHIFT		4
#define MISC_STRAP_BUS_BOOT_SEL_MASK		(0x1f << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_OPT_MASK            (0x18 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_SPI_NOR		(0x18 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_SPI_NOR_1_24MHZ	(0x0 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_SPI_NOR_2_54MHZ	(0x1 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_SPI_NOR_2_81MHZ	(0x2 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_SPI_NOR_4_81MHZ	(0x3 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_EMMC		(0x1e << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_NAND_2K_PAGE	(0x00 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_NAND_4K_PAGE	(0x08 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_NAND_8K_PAGE	(0x10 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_SEL_ECC_MASK	(0x7 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_NAND_ECC_DISABLE	(0x0 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_NAND_ECC_1_BIT	(0x1 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_NAND_ECC_4_BIT	(0x2 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_NAND_ECC_8_BIT	(0x3 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_NAND_ECC_12_BIT	(0x4 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_NAND_ECC_24_BIT	(0x5 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_NAND_ECC_40_BIT	(0x6 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_NAND_ECC_60_BIT	(0x7 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_LS_SPI_SLAVE_DISABLE	(1 << 3)
#define MISC_STRAP_BUS_PCIE1_RC_MODE		(1 << 2)
#define MISC_STRAP_BUS_PCIE0_RC_MODE		(1 << 1)
#define MISC_STRAP_BUS_TBUS_DISABLE		(1 << 0)

#ifndef __ASSEMBLER__
/*
 * Misc Register Set Definitions.
 */
typedef struct Misc {
#define MISC_PCIE_CTRL_CORE_SOFT_RESET_MASK     (0x3)
	uint32_t miscPCIECtrl;	/* 0x00 */
	uint32_t miscStrapBus;	/* 0x04 */
	uint32_t miscStrapOverride;	/* 0x08 */
	uint32_t miscAdsl_clock_sample;	/* 0x0c */
	uint32_t miscRngCtrl;	/* 0x10 */
	uint32_t miscMbox0_data;	/* 0x14 */
	uint32_t miscMbox1_data;	/* 0x18 */
	uint32_t miscMbox2_data;	/* 0x1c */
	uint32_t miscMbox3_data;	/* 0x20 */
	uint32_t miscMbox_ctrl;	/* 0x24 */
	uint32_t miscxMIIPadCtrl[4];	/* 0x28 */
#define MISC_XMII_PAD_MODEHV                    (1 << 6)
#define MISC_XMII_PAD_SEL_GMII                  (1 << 4)
#define MISC_XMII_PAD_AMP_EN                    (1 << 3)
	uint32_t miscxMIIPullCtrl[4];	/* 0x38 */
	uint32_t miscRBUSBridgeCtrl;	/* 0x48 */
	uint32_t miscSGMIIFiberDetect;	/* 0x4c */
#define MISC_SGMII_FIBER_GPIO36     (1<<0)
} Misc;

#define MISC ((volatile Misc * const) MISC_BASE)

/*
 * Gpio Controller
 */
typedef struct GpioControl {
	uint32_t GPIODir[5];	/* 0x00-0x10 */
	uint32_t GPIOio[5];	/* 0x14-0x24 */
	uint32_t SpiSlaveCfg;	/* 0x28 */
	uint32_t unused;		/* 0x2c */
	uint32_t TestControl;		/* 0x30 */
	uint32_t TestPortBlockEnMSB;	/* 0x34 */
	uint32_t TestPortBlockEnLSB;	/* 0x38 */
	uint32_t TestPortBlockDataMSB;	/* 0x3c */
	uint32_t TestPortBlockDataLSB;	/* 0x40 */
#define PINMUX_DATA_SHIFT       12
#define PINMUX_0                0
#define PINMUX_1                1
#define PINMUX_2                2
#define PINMUX_3                3
#define PINMUX_4                4
#define PINMUX_5                5
#define PINMUX_6                6
#define PINMUX_MSPI             PINMUX_0
#define PINMUX_MSPI_SS          PINMUX_1
#define PINMUX_PCM              PINMUX_1
#define PINMUX_GPIO             PINMUX_5
	uint32_t TestPortCmd;		/* 0x44 */
#define LOAD_MUX_REG_CMD        0x21
	uint32_t GPIOBaseMode;		/* 0x48 */
#define GPIO_BASE_USB_LED_OVERRIDE	(1<<11)
#define GPIO_BASE_VDSL_LED_OVERRIDE	(1<<10)
	uint32_t DiagReadBack;		/* 0x4c */
	uint32_t DiagReadBackHi;		/* 0x50 */
	uint32_t GeneralPurpose;		/* 0x54 */
} GpioControl;

#define GPIO ((volatile GpioControl * const) GPIO_BASE)

// PERF
typedef struct PerfControl {
    uint32_t        RevID;
#define CHIP_ID_SHIFT   12
#define CHIP_ID_MASK    (0xfffff << CHIP_ID_SHIFT)
#define REV_ID_MASK     0xff
} PerfControl;

#define PERF_BASE              0xfffe8000
#define PERF ((volatile PerfControl * const) PERF_BASE)

#endif

#endif
