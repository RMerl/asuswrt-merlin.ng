/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2019 Broadcom Ltd.
 */

#ifndef _63146_MISC_H
#define _63146_MISC_H

#define MISC_BASE          0xff802600

/*
 * Misc Register Set Definitions.
 */
typedef struct Misc {
   uint32_t miscStrapBus; /* 0x00 */

    /* boot select bits 3-5 */
#define BOOT_SEL_STRAP_NAND_2K_PAGE             0x00
#define BOOT_SEL_STRAP_NAND_4K_PAGE             0x08
#define BOOT_SEL_STRAP_NAND_8K_PAGE             0x10
#define BOOT_SEL_STRAP_NAND_512B_PAGE           0x18
#define BOOT_SEL_STRAP_SPI_NOR                  0x38
#define BOOT_SEL_STRAP_EMMC                     0x30
#define BOOT_SEL_STRAP_SPI_NAND                 0x28

#define BOOT_SEL_STRAP_BOOT_SEL_MASK            (0x38)
#define BOOT_SEL_STRAP_PAGE_SIZE_MASK           (0x7)

#define MISC_STRAP_BUS_BOOT_SEL_SHIFT           0
#define MISC_STRAP_BUS_BOOT_SEL_MASK            (0x38 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_SPI_NOR             (0x38 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_EMMC                (0x30 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_SPI_NAND            (0x28 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_SEL_NAND_MASK       (0x20 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_NAND                (0x00 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_SEL_PAGE_MASK       (0x18 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_NAND_2K_PAGE        (0x00 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_NAND_4K_PAGE        (0x08 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_NAND_8K_PAGE        (0x10 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_NAND_512_PAGE       (0x18 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_SEL_ECC_MASK        (0x7 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_NAND_ECC_DISABLE    (0x0 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_NAND_ECC_1_BIT      (0x1 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_NAND_ECC_4_BIT      (0x2 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_NAND_ECC_8_BIT      (0x3 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_NAND_ECC_12_BIT     (0x4 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_NAND_ECC_24_BIT     (0x5 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_NAND_ECC_40_BIT     (0x6 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_NAND_ECC_60_BIT     (0x7 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_PCIE0_RC_MODE            (0x1 << 6)
#define MISC_STRAP_BUS_LS_SPI_SLAVE_DISABLE     (0x1 << 7)
#define MISC_STRAP_BUS_B53_BOOT_N               (0x1 << 8)
/* When ROM BOOT OTP bits are 2b'11, always boot rom secure boot, this strap bit is don't care.
   When ROM BOOT OTP bits are are not 2b'11, this trap bit determine the following:
   1: boot rom non-secure boot
   0: XIP boot
*/
#define MISC_STRAP_BUS_BOOTROM_BOOT             (0x1 << 12)
#define MISC_STRAP_BUS_SW_RESERVE_MASK          (0x3 << 14)
#define MISC_STRAP_BUS_CPU_SLOW_FREQ_SHIFT      16
#define MISC_STRAP_BUS_CPU_SLOW_FREQ            (0x1 << MISC_STRAP_BUS_CPU_SLOW_FREQ_SHIFT)
   uint32_t miscStrapOverride;     /* 0x04 */
   uint32_t miscMaskUBUSErr;       /* 0x08 */
   uint32_t miscPeriphCtrl;        /* 0x0c */
   uint32_t miscSpiMasterCtrl;     /* 0x10 */
   uint32_t reserved0;             /* 0x14 */
   uint32_t miscPeriphMiscCtrl;    /* 0x18 */
   uint32_t miscPeriphMiscStat;    /* 0x1c */
   uint32_t miscSoftResetB;        /* 0x20 */
   uint32_t miscSpare0;            /* 0x24 */
   uint32_t miscSWdebugNW[2];      /* 0x28 */
   uint32_t miscWDResetCtrl;       /* 0x30 */
} Misc;

#define MISC ((volatile Misc * const) MISC_BASE)


/*
 * Gpio Controller
 */
typedef struct GpioControl {
        uint32_t GPIODir[8];             /* 0x00-0x1f */
        uint32_t GPIOio[8];              /* 0x20-0x3f */
        uint32_t PadCtrl;                 /* 0x40 */
        uint32_t SpiSlaveCfg;             /* 0x44 */
        uint32_t TestControl;             /* 0x48 */
        uint32_t TestPortBlockEnMSB;      /* 0x4c */
        uint32_t TestPortBlockEnLSB;      /* 0x50 */
        uint32_t TestPortBlockDataMSB;    /* 0x54 */
        uint32_t TestPortBlockDataLSB;    /* 0x58 */
        uint32_t TestPortCmd;             /* 0x5c */
        uint32_t DiagReadBack;            /* 0x60 */
        uint32_t DiagReadBackHi;          /* 0x64 */
        uint32_t GeneralPurpose;          /* 0x68 */
        uint32_t spare[3];
} GpioControl;

#define GPIO_BASE              0xff800500
#define GPIO ((volatile GpioControl * const) GPIO_BASE)
#define PINCTRL_BASE           (GPIO_BASE + 0x54)

// PERF
typedef struct PerfControl { /* GenInt */
     uint32_t        RevID;             /* (00) word 0 */
#define CHIP_ID_SHIFT   12
#define CHIP_ID_MASK    (0xfffff << CHIP_ID_SHIFT)
#define REV_ID_MASK     0xfff
} PerfControl;

#define PERF_BASE              0xff800000
#define PERF ((volatile PerfControl * const) PERF_BASE)

#define BCM_WDT_SOFT_RESET (PERF_BASE+0x48c)
#define BCM_LOWLEVEL_RESET() { *((volatile uint32_t *)BCM_WDT_SOFT_RESET) = 1; }

#endif
