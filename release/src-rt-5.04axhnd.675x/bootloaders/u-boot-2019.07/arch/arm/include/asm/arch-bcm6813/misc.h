/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2019 Broadcom Ltd.
 */

#ifndef _6813_MISC_H
#define _6813_MISC_H

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
#define MISC_STRAP_BUS_PCIE3_RC_MODE            (0x1 << 6)
#define MISC_STRAP_BUS_LS_SPI_SLAVE_DISABLE     (0x1 << 7)
#define MISC_STRAP_BUS_B53_BOOT_N               (0x1 << 8)
#define MISC_STRAP_BUS_RESET_OUT_DELAY          (0x1 << 9)
/* When ROM BOOT OTP bits are 2b'11, always boot rom secure boot, this strap bit is don't care.
   When ROM BOOT OTP bits are are not 2b'11, this trap bit determine the following:
   1: boot rom non-secure boot
   0: XIP boot
*/
#define MISC_STRAP_BUS_BOOTROM_BOOT             (0x1 << 12)
#define MISC_STRAP_BUS_SW_RESERVE_MASK          (0x3 << 13)
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

/*
 * Peripheral Controller
 */
typedef struct DMAIrqCfg {
   uint32_t DMAIrqStatus;       /* 0x00 */
   uint32_t DMAIrqSet;          /* 0x04 */
   uint32_t DMAIrqClear;        /* 0x08 */
   uint32_t DMAIrqMaskStatus;   /* 0x0c */
   uint32_t DMAIrqMaskSet;      /* 0x10 */
   uint32_t DMAIrqMaskClear;    /* 0x14 */
}DMAIrqCfg;

typedef struct PerfControl {
   uint32_t RevID;        /* 0x00 */
#define CHIP_ID_SHIFT   12
#define CHIP_ID_MASK    (0xfffff << CHIP_ID_SHIFT)
#define REV_ID_MASK     0xfff
   uint32_t reserved0[7]; /* 0x04 - 0x1f */
   uint32_t ExtIrqCtrl;   /* 0x20 */
#define EI_LVLSTICKY_SHFT   0
#define EI_SENSE_SHFT   8
#define EI_INSENS_SHFT  16
#define EI_LEVEL_SHFT   24
   uint32_t ExtIrqStatus;  /* 0x24 */
#define EI_STATUS_SHFT  0
#define EI_STATUS_MASK  0xff
   uint32_t ExtIrqSet;     /* 0x28 */
   uint32_t ExtIrqClear;   /* 0x2c */
   uint32_t ExtIrqMaskStatus;/* 0x30 */
   uint32_t ExtIrqMaskSet; /* 0x34 */
   uint32_t ExtIrqMaskClear; /* 0x38 */
   uint32_t reserved1[2]; /* 0x3c - 0x43 */
   uint32_t ExtIrqMuxSel0; /* 0x44 */
#define EXT_IRQ_SLOT_SIZE             16
#define EXT_IRQ_MUX_SEL0_SHIFT        4
#define EXT_IRQ_MUX_SEL0_MASK         0xf
   uint32_t ExtIrqMuxSel1;   /* 0x48 */
#define EXT_IRQ_MUX_SEL1_SHIFT        4
#define EXT_IRQ_MUX_SEL1_MASK         0xf
   uint32_t IrqPeriphStatus; /* 0x4c */
   uint32_t IrqPeriphMask;   /* 0x50 */
   uint32_t reserved2[8];    /* 0x54 - 0x73 */  
   DMAIrqCfg dmaIrqCfg[3]; /* 0x74 - 0xbb */
} PerfControl;

#define PERF_BASE	0xff800000
#define PERF ((volatile PerfControl * const) PERF_BASE)



#endif
