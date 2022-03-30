/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2019 Broadcom Ltd.
 */

#ifndef _6858_MISC_H
#define _6858_MISC_H

/*
 ** Misc Register Set Definitions.
 */

typedef struct Misc {
    uint32_t  miscStrapBus;                       /* 0x00 */
#define MISC_STRAP_BUS_BOOT_SEL0_4_SHIFT        5
#define MISC_STRAP_BUS_BOOT_SEL0_4_MASK         (0x18 << MISC_STRAP_BUS_BOOT_SEL0_4_SHIFT)
#define MISC_STRAP_BUS_BOOT_SEL5_SHIFT          11
#define MISC_STRAP_BUS_BOOT_SEL5_MASK           (0x1 << MISC_STRAP_BUS_BOOT_SEL5_SHIFT)
#define BOOT_SEL5_STRAP_ADJ_SHIFT               (MISC_STRAP_BUS_BOOT_SEL5_SHIFT-MISC_STRAP_BUS_BOOT_SEL0_4_SHIFT)

    /* boot select bits 0-2 */
#define MISC_STRAP_BUS_BOOT_SEL_ECC_MASK        (0x7 << MISC_STRAP_BUS_BOOT_SEL0_4_SHIFT)
#define MISC_STRAP_BUS_BOOT_NAND_ECC_DISABLE    (0x0 << MISC_STRAP_BUS_BOOT_SEL0_4_SHIFT)
#define MISC_STRAP_BUS_BOOT_NAND_ECC_1_BIT      (0x1 << MISC_STRAP_BUS_BOOT_SEL0_4_SHIFT)
#define MISC_STRAP_BUS_BOOT_NAND_ECC_4_BIT      (0x2 << MISC_STRAP_BUS_BOOT_SEL0_4_SHIFT)
#define MISC_STRAP_BUS_BOOT_NAND_ECC_8_BIT      (0x3 << MISC_STRAP_BUS_BOOT_SEL0_4_SHIFT)
#define MISC_STRAP_BUS_BOOT_NAND_ECC_12_BIT     (0x4 << MISC_STRAP_BUS_BOOT_SEL0_4_SHIFT)
#define MISC_STRAP_BUS_BOOT_NAND_ECC_24_BIT     (0x5 << MISC_STRAP_BUS_BOOT_SEL0_4_SHIFT)
#define MISC_STRAP_BUS_BOOT_NAND_ECC_40_BIT     (0x6 << MISC_STRAP_BUS_BOOT_SEL0_4_SHIFT)
#define MISC_STRAP_BUS_BOOT_NAND_ECC_60_BIT     (0x7 << MISC_STRAP_BUS_BOOT_SEL0_4_SHIFT)

    /* boot select bits 3-5 */
#define BOOT_SEL_STRAP_BOOT_SEL_NAND_MASK		0x20
#define BOOT_SEL_STRAP_NAND_2K_PAGE             0x00
#define BOOT_SEL_STRAP_NAND_4K_PAGE             0x08
#define BOOT_SEL_STRAP_NAND_8K_PAGE             0x10
#define BOOT_SEL_STRAP_NAND_512B_PAGE           0x18
#define BOOT_SEL_STRAP_SPI_NOR                  0x38
#define BOOT_SEL_STRAP_EMMC                     0x30
#define BOOT_SEL_STRAP_SPI_NAND                 0x28
#define BOOT_SEL_STRAP_NAND						0x00

#define BOOT_SEL_STRAP_BOOT_SEL_MASK            (0x38)
#define BOOT_SEL_STRAP_PAGE_SIZE_MASK           (0x7)

#define MISC_STRAP_BUS_PCIE_SATA_MASK           (1 << 3)
#define MISC_STRAP_BUS_BOOROM_BOOT_N            (1 << 10 )
#define MISC_STRAP_BUS_B53_NO_BOOT              (1 << 12) /* 1 = PMC boots before B53 */
#define MISC_STRAP_BUS_PMC_ROM_BOOT_SHIFT       13
#define MISC_STRAP_BUS_PMC_ROM_BOOT             (0x1 << MISC_STRAP_BUS_PMC_ROM_BOOT_SHIFT) /* 1 = PMC boot */
#define MISC_STRAP_BUS_CPU_SLOW_FREQ            (1 << 14) /* 1 = Slow 400MHz cpu freq */
#define MISC_STRAP_BUS_PMC_BOOT_AVS             (1 << 20) /* 1 = PMC run AVS */
#define MISC_STRAP_BUS_PMC_BOOT_FLASH           (1 << 21) /* 1 = PMC boot from flash */ 
#define MISC_STRAP_BUS_UBUS_FREQ_SHIFT          22
#define MISC_STRAP_BUS_UBUS_FREQ_MASK           (1 << MISC_STRAP_BUS_UBUS_FREQ_SHIFT) /* 1 = 2GHz, 0 = 1GHz */  
#define MISC_STRAP_DDR_16B_EN_SHIFT             26
#define MISC_STRAP_DDR_16B_EN_MASK              (1 << MISC_STRAP_DDR_16B_EN_SHIFT)
#define MISC_STRAP_DDR_DENSITY_SHIFT            15
#define MISC_STRAP_DDR_DENSITY_MASK             (3 << MISC_STRAP_DDR_DENSITY_SHIFT) /* 1=8Gb, 2=4Gb, 3=2Gb */
#define MISC_STRAP_DDR_OVERRIDE_N_SHIFT         17
#define MISC_STRAP_DDR_OVERRIDE_N_MASK          (1 << MISC_STRAP_DDR_OVERRIDE_N_SHIFT)
#define MISC_STRAP_OEC_SHIFT                    24
#define MISC_STRAP_OEC_MASK                     (0x3 << MISC_STRAP_OEC_SHIFT)
#define MISC_STRAP_OEC_156MHZ                   0x1
#define MISC_STRAP_OEC_155MHZ                   0x2
#define MISC_STRAP_OEC_50MHZ                    0x3

    uint32_t  miscStrapOverride;                  /* 0x04 */
    uint32_t  miscSWdebug[6];                     /* 0x08-0x1c */
    uint32_t  miscWDresetCtrl;                    /* 0x20 */
    uint32_t  miscSWdebugNW[2];                   /* 0x24-0x28 */
    uint32_t  miscSoftResetB;                     /* 0x2c */
    uint32_t  miscPLLstatus;                      /* 0x30 */
    uint32_t  miscDierevid;                       /* 0x34 */
    uint32_t  miscSPImasterCtrl;                  /* 0x38 */
    uint32_t  miscAltBoot;                        /* 0x3c */
    uint32_t  miscPeriphCtrl;                     /* 0x40 */
#define MISC_PCIE_CTRL_CORE_SOFT_RESET_MASK     (0x7)
    uint32_t  miscPCIECtrl;                       /* 0x44 */
    uint32_t  miscAdslClockSample;                /* 0x48 */
    uint32_t  miscRNGCtrl;                        /* 0x4c */
    uint32_t  miscMbox0_data;                     /* 0x50 */
    uint32_t  miscMbox1_data;                     /* 0x54 */
    uint32_t  miscMbox2_data;                     /* 0x58 */
    uint32_t  miscMbox3_data;                     /* 0x5c */
    uint32_t  miscMbox_ctrl;                      /* 0x60 */
    uint32_t  miscMIIPadCtrl;                     /* 0x64 */
    uint32_t  miscRGMII1PadCtrl;                  /* 0x68 */
    uint32_t  miscRGMII2PadCtrl;                  /* 0x6c */
    uint32_t  miscRGMII3PadCtrl;                  /* 0x70 */
    uint32_t  miscMIIPullCtrl;                    /* 0x74 */
    uint32_t  miscRGMII1PullCtrl;                 /* 0x78 */
    uint32_t  miscRGMII2PullCtrl;                 /* 0x7c */
    uint32_t  miscRGMII3PullCtrl;                 /* 0x80 */
    uint32_t  miscWDenReset;                      /* 0x84 */
    uint32_t  miscBootOverlayEn;                  /* 0x88 */
    uint32_t  miscSGMIIfiber;                     /* 0x8c */
    uint32_t  miscUNIMACCtrl;                     /* 0x90 */
    uint32_t  miscMaskUBUSErr;                    /* 0x94 */
    uint32_t  miscTOSsync;                        /* 0x98 */
    uint32_t  miscPM0_1_status;                   /* 0x9c */
    uint32_t  miscPM2_3_status;                   /* 0xa0 */
    uint32_t  miscSGB_status;                     /* 0xa4 */
    uint32_t  miscPM0_1_config;                   /* 0xa8 */
    uint32_t  miscPM2_3_config;                   /* 0xac */
    uint32_t  miscSGB_config;                     /* 0xb0 */
    uint32_t  miscPM0_1_tmon_config;              /* 0xb4 */
    uint32_t  miscPM2_3_tmon_config;              /* 0xb8 */
    uint32_t  miscSGB_tmon_config;                /* 0xbc */
    uint32_t  miscMDIOmasterSelect;               /* 0xc0 */
    uint32_t  miscUSIMCtrl;                       /* 0xc4 */
    uint32_t  miscUSIMPadCtrl;                    /* 0xc8 */
    uint32_t  miscPerSpareReg[3];                 /* 0xcc - 0xd4 */
    uint32_t  miscDgSensePadCtrl;                 /* 0xd8 */
#define DG_CTRL_SHIFT   4
#define DG_EN_SHIFT     3
#define DG_TRIM_SHIFT   0
    uint32_t  miscPeriphMiscCtrl;                 /* 0xdc */
    uint32_t  miscPeriphMiscStat;                 /* 0xe0 */
} Misc;

#define MISC_BASE          0xff802600
#define MISC ((volatile Misc * const) MISC_BASE)

// PERF
typedef struct PerfControl { /* GenInt */
     uint32_t        RevID;             /* (00) word 0 */
#define CHIP_ID_SHIFT   16
#define CHIP_ID_MASK    (0xffff << CHIP_ID_SHIFT)
#define REV_ID_MASK     0xff
} PerfControl;

#define PERF_BASE              0xff800000
#define PERF ((volatile PerfControl * const) PERF_BASE)

#endif
