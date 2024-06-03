/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2019 Broadcom Ltd.
 */

#ifndef _6846_MISC_H
#define _6846_MISC_H

/*
 * Gpio Controller
 */
typedef struct GpioControl {
        uint32_t GPIODir[8];             /* 0x00-0x1c */
        uint32_t GPIOio[8];              /* 0x20-0x3c */
        uint32_t PadCtrl;                 /* 0x40 */
        uint32_t SpiSlaveCfg;             /* 0x44 */
        uint32_t TestControl;             /* 0x48 */
        uint32_t TestPortBlockEnMSB;      /* 0x4c */
        uint32_t TestPortBlockEnLSB;      /* 0x50 */
        uint32_t TestPortBlockDataMSB;    /* 0x54 */
        uint32_t TestPortBlockDataLSB;    /* 0x58 */
#define PINMUX_DATA_SHIFT       12
#define PINMUX_0                0
#define PINMUX_1                1
#define PINMUX_2                2
#define PINMUX_3                3
#define PINMUX_4                4
#define PINMUX_5                5
#define PINMUX_6                6
#define PINMUX_7                7
#define PAD_CTRL_SHIFT          12
#define PAD_CTRL_MASK           (0x3f<<PAD_CTRL_SHIFT)
#define PAD_SEL_SHIFT           12
#define PAD_AMP_SHIFT           15
#define PAD_IND_SHIFT           16
#define PAD_GMII_SHIFT          17
       uint32_t TestPortCmd;             /* 0x5c */
#define LOAD_MUX_REG_CMD        0x21
#define LOAD_PAD_CTRL_CMD       0x22
        uint32_t DiagReadBack;            /* 0x60 */
        uint32_t DiagReadBackHi;          /* 0x64 */
        uint32_t GeneralPurpose;          /* 0x68 */
        uint32_t spare[3];
} GpioControl;

#define GPIO_BASE          0xff800500
#define GPIO ((volatile GpioControl * const) GPIO_BASE)
/*
 ** Misc Register Set Definitions.
 */

typedef struct Misc {
    uint32_t  miscStrapBus;                       /* 0x00 */
#define MISC_STRAP_BUS_BOOT_SEL_SHIFT           5
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
#define MISC_STRAP_BUS_BOOTROM_BOOT_N           (0x1 << 11)

#define MISC_STRAP_BUS_PCIE_SATA_MASK		    (1 << 3)  /* 1-PCI  0-SATA */
#define MISC_STRAP_BUS_B53_NO_BOOT              (1 << 12) /* 1-PMC boot A53, 0-A53 boots at POR */
#define MISC_STRAP_DDR_DENSITY_SHIFT            18
#define MISC_STRAP_DDR_DENSITY_MASK             (3 << MISC_STRAP_DDR_DENSITY_SHIFT) /* 0=1Gb, 1=2Gb, 2=8Gb, 3=4Gb */
#define MISC_STRAP_BUS_PMC_BOOT_AVS             (1 << 20) /* 1 = PMC run AVS */
#define MISC_STRAP_BUS_PMC_BOOT_FLASH_N         (1 << 21) /* 1 = PMC boot from rom */
#define MISC_STRAP_DDR_FREQ_SHIFT               27
#define MISC_STRAP_DDR_FREQ_MASK                (1 << MISC_STRAP_DDR_FREQ_SHIFT) /* 1-800MHz, 0-533MHz */
#define MISC_STRAP_BUS_PCIE_SINGLE_LANES_SHIFT  29
#define MISC_STRAP_BUS_PCIE_SINGLE_LANES        (1 << MISC_STRAP_BUS_PCIE_SINGLE_LANES_SHIFT) /* 1-Single Lanes  0-Dual Lanes */
#define MISC_STRAP_ENABLE_INT_1p8V              (1 << 1)

    uint32_t  miscStrapOverride;                  /* 0x04 */
    uint32_t  miscMaskUBUSErr;                    /* 0x08 */
    uint32_t  miscPeriphCtrl;                     /* 0x0c */
    uint32_t  miscSPImasterCtrl;                  /* 0x10 */
    uint32_t  miscDierevid;                       /* 0x14 */
    uint32_t  miscPeriphMiscCtrl;                 /* 0x18 */
    uint32_t  miscPeriphMiscStat;                 /* 0x1c */
    uint32_t  miscMbox0_data;                     /* 0x20 */
    uint32_t  miscMbox1_data;                     /* 0x24 */
    uint32_t  miscMbox2_data;                     /* 0x28 */
    uint32_t  miscMbox3_data;                     /* 0x2c */
    uint32_t  miscMbox_ctrl;                      /* 0x30 */
    uint32_t  miscSoftResetB;                     /* 0x34 */
    uint32_t  reserved0;                          /* 0x38 */
    uint32_t  miscSWdebugNW[2];                   /* 0x3c */
    uint32_t  miscWDresetCtrl;                    /* 0x44 */
} Misc;

#define MISC_BASE          0xff802600
#define MISC ((volatile Misc * const) MISC_BASE)

// PERF
typedef struct PerfControl { /* GenInt */
     uint32_t        RevID;             /* (00) word 0 */
#define CHIP_ID_SHIFT   12
#define CHIP_ID_MASK    (0xfffff << CHIP_ID_SHIFT)
#define REV_ID_MASK     0xff
} PerfControl;

#define PERF_BASE              0xff800000
#define PERF ((volatile PerfControl * const) PERF_BASE)

#endif
