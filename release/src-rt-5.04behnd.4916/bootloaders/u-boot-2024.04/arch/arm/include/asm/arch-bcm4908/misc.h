/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2019 Broadcom Ltd.
 */

#ifndef _4908_MISC_H
#define _4908_MISC_H

#define MISC_BASE             0xFF802600

/*
 * Misc Register Set Definitions.
 */
typedef struct Misc {
   uint32_t miscStrapBus; /* 0x00 */
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
#define MISC_STRAP_BUS_B53_BOOT_N               (0x1 << 6)
#define MISC_STRAP_BUS_BOOTROM_BOOT_N           (0x1 << 7)
#define MISC_STRAP_BUS_LS_SPI_SLAVE_DISABLE     (0x1 << 8)
#define MISC_STRAP_BUS_PMC_BOOT_AVS             (0x1 << 9)
#define MISC_STRAP_BUS_PMC_BOOT_FLASH           (0x1 << 10)
#define MISC_STRAP_BUS_PMC_ROM_BOOT             (0x1 << 11)
#define MISC_STRAP_BUS_PCIE0_RC_MODE            (0x1 << 12)
#define MISC_STRAP_BUS_RESET_OUT_DELAY_SHIFT    13
#define MISC_STRAP_BUS_RESET_OUT_DELAY_MASK     (0x1 << MISC_STRAP_BUS_RESET_OUT_DELAY_SHIFT)
#define MISC_STRAP_BUS_RESET_OUT_DELAY_20US     (0x1 << MISC_STRAP_BUS_RESET_OUT_DELAY_SHIFT)
#define MISC_STRAP_BUS_RESET_OUT_DELAY_10MS     (0x0 << MISC_STRAP_BUS_RESET_OUT_DELAY_SHIFT)
#define MISC_STRAP_BUS_XTAL_BYPASS_N            (0x1 << 14)
#define MISC_STRAP_BUS_UBUS_CLOCK_SHIFT         15
#define MISC_STRAP_BUS_UBUS_CLOCK_MASK          (0x1 << MISC_STRAP_BUS_UBUS_CLOCK_SHIFT)
#define MISC_STRAP_BUS_UBUS_CLOCK_400MHZ        (0x1 << MISC_STRAP_BUS_UBUS_CLOCK_SHIFT)
#define MISC_STRAP_BUS_UBUS_CLOCK_50MHZ         (0x0 << MISC_STRAP_BUS_UBUS_CLOCK_SHIFT)
#define MISC_STRAP_BUS_CPU_SLOW_FREQ            (0x1 << 16)
#define MISC_STRAP_BUS_SW_RESERVE_MASK          (0x3 << 17)
   uint32_t miscStrapOverride;     /* 0x04 */
   uint32_t miscSoftwareDebug[6];  /* 0x08 */
   uint32_t miscWDResetCtrl;       /* 0x20 */
   uint32_t miscSWdebugNW[2];      /* 0x24 */
   uint32_t miscSoftResetB;        /* 0x2c */
   uint32_t miscQAMPllStatus;      /* 0x30 */
   uint32_t miscRsvd1;             /* 0x34 */
   uint32_t miscSpiMasterCtrl;     /* 0x38 */
   uint32_t miscAltBootVector;     /* 0x3c */
   uint32_t miscPeriphCtrl;        /* 0x40 */
#define MISC_PCIE_CTRL_CORE_SOFT_RESET_MASK     (0x7)
   uint32_t miscPCIECtrl;          /* 0x44 */
   uint32_t miscAdsl_clock_sample; /* 0x48 */
   uint32_t miscRngCtrl;           /* 0x4c */
   uint32_t miscMbox_data[4];      /* 0x50 */
   uint32_t miscMbox_ctrl;         /* 0x60 */
   uint32_t miscxMIIPadCtrl[4];    /* 0x64 */
#define MISC_XMII_PAD_MODEHV                    (1 << 6)
#define MISC_XMII_PAD_SEL_GMII                  (1 << 4)
#define MISC_XMII_PAD_AMP_EN                    (1 << 3)
   uint32_t miscxMIIPullCtrl[4];    /* 0x74 */
   uint32_t miscWDResetEn;          /* 0x84 */
   uint32_t miscBootOverlayEn;      /* 0x88 */
   uint32_t miscSGMIIFiberDetect;   /* 0x8c */
#define MISC_SGMII_FIBER_GPIO36     (1<<0)
   uint32_t miscUniMacCtrl;         /* 0x90 */
   uint32_t miscMaskUBUSErr;        /* 0x94 */
   uint32_t miscTOSsync;            /* 0x98 */
   uint32_t miscPM0_1_status;       /* 0x9c */
   uint32_t miscPM2_3_status;       /* 0xa0 */
   uint32_t miscSGB_status;         /* 0xa4 */
   uint32_t miscPM0_1_config;       /* 0xa8 */
   uint32_t miscPM2_3_config;       /* 0xac */
   uint32_t miscSGB_config;         /* 0xb0 */
   uint32_t miscPM0_1_tmon_config;  /* 0xb4 */
   uint32_t miscPM2_3_tmon_config;  /* 0xb8 */
   uint32_t miscSGB_tmon_config;    /* 0xbc */
   uint32_t miscMDIOmasterSelect;   /* 0xc0 */
   uint32_t miscUSIMCtrl;           /* 0xc4 */
   uint32_t miscUSIMPadCtrl;        /* 0xc8 */
   uint32_t miscPerSpareReg[3];     /* 0xcc - 0xd4 */
   uint32_t miscDgSensePadCtrl;     /* 0xd8 */
#define DG_CTRL_SHIFT   4
#define DG_EN_SHIFT     3
#define DG_TRIM_SHIFT   0
   uint32_t miscPeriphMiscCtrl;     /* 0xdc */
   uint32_t miscPeriphMiscStat;     /* 0xe0 */
} Misc;

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
