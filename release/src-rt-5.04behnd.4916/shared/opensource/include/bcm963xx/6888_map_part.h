/*
<:copyright-BRCM:2015:DUAL/GPL:standard 

   Copyright (c) 2015 Broadcom 
   All Rights Reserved

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

:>

*/

#ifndef __BCM6888_MAP_PART_H
#define __BCM6888_MAP_PART_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bcmtypes.h"
#include "bcm_io_map.h"

#define CHIP_FAMILY_ID_HEX 0x6888

typedef enum 
{
    WAN_IDX,
    WANBBH_IDX,
    PERF_IDX,
    PERF1_IDX,
    BIUCFG_IDX,
    CCI500_IDX,
    CCB_IDX,
    LAST_IDX
} BCM_IO_MAP_IDX;
#endif

#define PERF_PHYS_BASE              0xff800000
#define PERF_SIZE                   0x3000
#define PERF_BASE                   BCM_IO_MAP(PERF_IDX, PERF_PHYS_BASE, 0)

#define WAN_PHYS_BASE               0x80100000
#define WAN_SIZE                    0x28000

#define WANBBH_PHYS_BASE            0x80170000
#define WANBBH_SIZE                 0x122a8




#define TIMR_OFFSET                 0x0400
#define NAND_REG_OFFSET             0x1800
#define NAND_CACHE_OFFSET           0x1c00
#define NAND_INTR_OFFSET            0x2000
#define MISC_OFFSET                 0x2600
#define JTAG_OTP_OFFSET             0x2800

#define TIMR_PHYS_BASE              (PERF_PHYS_BASE + TIMR_OFFSET)
#define NAND_REG_PHYS_BASE          (PERF_PHYS_BASE + NAND_REG_OFFSET)
#define NAND_CACHE_PHYS_BASE        (PERF_PHYS_BASE + NAND_CACHE_OFFSET)
#define NAND_INTR_PHYS_BASE         (PERF_PHYS_BASE + NAND_INTR_OFFSET)
#define MISC_PHYS_BASE              (PERF_PHYS_BASE + MISC_OFFSET)
#define JTAG_OTP_PHYS_BASE          (PERF_PHYS_BASE + JTAG_OTP_OFFSET)

#define TIMR_BASE                   BCM_IO_MAP(PERF_IDX, PERF_PHYS_BASE, TIMR_OFFSET)
#define NAND_REG_BASE               BCM_IO_MAP(PERF_IDX, PERF_PHYS_BASE, NAND_REG_OFFSET)
#define NAND_CACHE_BASE             BCM_IO_MAP(PERF_IDX, PERF_PHYS_BASE, NAND_CACHE_OFFSET)
#define NAND_INTR_BASE              BCM_IO_MAP(PERF_IDX, PERF_PHYS_BASE, NAND_INTR_OFFSET)
#define MISC_BASE                   BCM_IO_MAP(PERF_IDX, PERF_PHYS_BASE, MISC_OFFSET)
#define JTAG_OTP_BASE               BCM_IO_MAP(PERF_IDX, PERF_PHYS_BASE, JTAG_OTP_OFFSET)


#define WAN_BASE                        BCM_IO_MAP(WAN_IDX, WAN_PHYS_BASE, 0)
#define WANBBH_BASE                     BCM_IO_MAP(WANBBH_IDX, WANBBH_PHYS_BASE, 0)

#define PERF1_PHYS_BASE       0xff85a000
#define PERF1_SIZE            0x1000
/* perf1 block offset */
#define PERF1_OFFSET          0x0000
#define TOP_CONTROL_OFFSET    0x0000

#define TOP_CONTROL_PHYS_BASE (PERF1_PHYS_BASE+TOP_CONTROL_OFFSET)

#define TOP_CONTROL_BASE   BCM_IO_MAP(PERF1_IDX, PERF1_PHYS_BASE, TOP_CONTROL_OFFSET)

#define UNIMAC_BASE         XRDP_BASE + 0x8a8000 /* 828a8000 */
#define UNIMAC_CFG_BASE     UNIMAC_BASE
#define UNIMAC_MIB_BASE     XRDP_BASE + 0x8a8400
#define UNIMAC_TOP_BASE     XRDP_BASE + 0x8b0000

#define BIUCFG_PHYS_BASE     0x81060000
#define BIUCFG_SIZE          0x3000
#define BIUCFG_OFFSET        0x0000

#define CCI500_PHYS_BASE     0x81100000
#define CCI500_SIZE          0x91000
#define CCI500_OFFSET        0x0000

#define CCB_PHYS_BASE        0x81200000
#define CCB_SIZE             0x6000
#define CCB_OFFSET           0x0000

#define BIUCFG_BASE        BCM_IO_MAP(BIUCFG_IDX, BIUCFG_PHYS_BASE, BIUCFG_OFFSET)
#define CCI500_BASE        BCM_IO_MAP(CCI500_IDX, CCI500_PHYS_BASE, CCI500_OFFSET)
#define CCB_BASE           BCM_IO_MAP(CCB_IDX, CCB_PHYS_BASE, CCB_OFFSET)

#ifndef __ASSEMBLER__

#ifdef __BOARD_DRV_AARCH64__
// add here any legacy driver's (driver that have no device tree node) IO memory to be mapped
BCM_IO_BLOCKS bcm_io_blocks[] = 
{
    {WAN_IDX, WAN_SIZE, WAN_PHYS_BASE},
    {WANBBH_IDX, WANBBH_SIZE, WANBBH_PHYS_BASE},
    {PERF_IDX, PERF_SIZE, PERF_PHYS_BASE},
    {PERF1_IDX, PERF1_SIZE, PERF1_PHYS_BASE},
    {BIUCFG_IDX, BIUCFG_SIZE, BIUCFG_PHYS_BASE},
    {CCI500_IDX, CCI500_SIZE, CCI500_PHYS_BASE},
    {CCB_IDX, CCB_SIZE, CCB_PHYS_BASE},
};
unsigned long bcm_io_block_address[LAST_IDX];
#else
extern BCM_IO_BLOCKS bcm_io_blocks[];
extern unsigned long bcm_io_block_address[];
#endif

/*
** Peripheral Controller
*/
typedef struct PerfControl { /* GenInt */
     uint32        RevID;             /* (00) word 0 */
#define CHIP_ID_SHIFT   12
#define CHIP_ID_MASK    (0xfffff << CHIP_ID_SHIFT)
#define REV_ID_MASK     0xff

   uint32 Ext1IrqCtrl;         /* 0x04 */
#define EI_SENSE_SHFT   8
#define EI_INSENS_SHFT  16
#define EI_LEVEL_SHFT   24
   uint32 Ext1IrqStatus;       /* 0x08 */
#define EI_STATUS_SHFT  0
   uint32 Ext1IrqSet;          /* 0x0c */
   uint32 Ext1IrqClear;        /* 0x10 */
   uint32 Ext1IrqMaskStatus;   /* 0x14 */
   uint32 Ext1IrqMaskSet;      /* 0x18 */
   uint32 Ext1IrqMaskClear;    /* 0x1c */
   uint32 ExtIrqCtrl;          /* 0x20 */
   uint32 ExtIrqStatus;        /* 0x24 */
   uint32 ExtIrqSet;           /* 0x28 */
   uint32 ExtIrqClear;         /* 0x2c */
   uint32 ExtIrqMaskStatus;    /* 0x30 */
   uint32 ExtIrqMaskSet;       /* 0x34 */
   uint32 ExtIrqMaskClear;     /* 0x38 */
   uint32 reserved0[2];        /* 0x3c-0x43 */
   uint32 ExtIrqMuxSel0;       /* 0x44 */
#define EXT_IRQ_SLOT_SIZE             16
#define EXT_IRQ_MUX_SEL0_SHIFT        4
#define EXT_IRQ_MUX_SEL0_MASK         0xf
   uint32 ExtIrqMuxSel1;       /* 0x48 */
#define EXT_IRQ_MUX_SEL1_SHIFT        4
#define EXT_IRQ_MUX_SEL1_MASK         0xf
   uint32 IrqPeriphStatus;     /* 0x4c */
   uint32 IrqPeriphMask;       /* 0x50 */
   uint32 reserved[8];         /* 0x54 */  
   uint32 DMAIrqStatus;        /* 0x74 */
   uint32 DMAIrqSet;           /* 0x78 */
   uint32 DMAIrqClear;         /* 0x7c */
   uint32 DMAIrqMaskStatus;    /* 0x80 */
   uint32 DMAIrqMaskSet;       /* 0x84 */
   uint32 DMAIrqMaskClear;     /* 0x88 */
   uint32 DMA1IrqStatus;        /* 0x8c */
   uint32 DMA1IrqSet;           /* 0x90 */
   uint32 DMA1IrqClear;         /* 0x94 */
   uint32 DMA1IrqMaskStatus;    /* 0x98 */
   uint32 DMA1IrqMaskSet;       /* 0x9c */
   uint32 DMA1IrqMaskClear;     /* 0xa0 */
   uint32 DMA2IrqStatus;        /* 0xa4 */
   uint32 DMA2IrqSet;           /* 0xa8 */
   uint32 DMA2IrqClear;         /* 0xac */
   uint32 DMA2IrqMaskStatus;    /* 0xb0 */
   uint32 DMA2IrqMaskSet;       /* 0xb4 */
   uint32 DMA2IrqMaskClear;     /* 0xb8 */
} PerfControl;

#define PERF ((volatile PerfControl * const) PERF_BASE)
#if 0
typedef struct Jtag_Otp {
   uint32 ctrl0;                          /* 0x00 */
#define JTAG_OTP_CTRL_ACCESS_MODE         (0x2 << 22)
#define JTAG_OTP_CTRL_PROG_EN             (1 << 21)
#define JTAG_OTP_CTRL_START               (1 << 0)
#define JTAG_OTP_CTRL_CMD_OTP_PROG_EN   (0x2 << 1)
#define JTAG_OTP_CTRL_CMD_PROG      (0xa << 1)
#define JTAG_OTP_CTRL_CMD_PROG_LOCK (0x19 << 1)
   uint32 ctrl1;                          /* 0x04 */
#define JTAG_OTP_CTRL_CPU_MODE            (1 << 0)
   uint32 ctrl2;                          /* 0x08 */
   uint32 ctrl2_hi;                       /* 0x0c */
   uint32 ctrl3;                          /* 0x10 */
   uint32 ctrl4;                          /* 0x14 */
   uint32 status0;                        /* 0x18 */
   uint32 status0_hi;                     /* 0x1c */
   uint32 status1;                        /* 0x20 */
   uint32 status[12];                     /* 0x24 */
#define JTAG_OTP_STATUS_1_PROG_OK       (1 << 2)
#define JTAG_OTP_STATUS_1_CMD_DONE        (1 << 1)
   uint32 wotp_desc_status;
   uint32 wotp_debug_sec[4];
   uint32 wotp_reinit;
   uint32 wotp_fout_error_status;
#define WOTP_CPU_LOCK wotp_cpu_lock
#define WOTP_CPU_SOFT_LOCK_SHIFT    0x0
#define WOTP_CPU_SOFT_LOCK_MASK     (0x1<<WOTP_CPU_SOFT_LOCK_SHIFT)
   uint32 wotp_cpu_lock;
   uint32 wotp_status;
} Jtag_Otp;

#define JTAG_OTP ((volatile Jtag_Otp * const) JTAG_OTP_BASE)

#define BTRM_OTP_READ_TIMEOUT_CNT       0x10000

/* row 13 */
#define OTP_BRCM_BTRM_BOOT_ENABLE_ROW           13
#define OTP_BRCM_BTRM_BOOT_ENABLE_SHIFT         2
#define OTP_BRCM_BTRM_BOOT_ENABLE_MASK          (1 << OTP_BRCM_BTRM_BOOT_ENABLE_SHIFT)

/* row 14 */
#define OTP_CUST_BTRM_BOOT_ENABLE_ROW           14
#define OTP_CUST_BTRM_BOOT_ENABLE_SHIFT         28
#define OTP_CUST_BTRM_BOOT_ENABLE_MASK          (1 << OTP_CUST_BTRM_BOOT_ENABLE_SHIFT)

/* row 16 */
#define OTP_PCM_DISABLE_ROW             16
#define OTP_PCM_DISABLE_SHIFT           1
#define OTP_PCM_DISABLE_MASK            (0x1 << OTP_PCM_DISABLE_SHIFT)

/* row 15 */
#define OTP_CHIPID_ROW              15
#define OTP_CHIPID_SHIFT            0
#define OTP_CHIP_ID_MASK            (0xfffff << OTP_CHIPID_SHIFT)

/* row 29 */
#define OTP_CUST_MFG_MRKTID_ROW                 29
#define OTP_CUST_MFG_MRKTID_SHIFT               0
#define OTP_CUST_MFG_MRKTID_MASK                (0xffff << OTP_CUST_MFG_MRKTID_SHIFT)
#endif

/*
** Misc Register Set Definitions.
*/

typedef struct Misc {
    uint32  unused;                       /* 0x00 */
#if 0
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
#define MISC_STRAP_BUS_BOOT_SEL_ECC_MASK        (0x7 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_NAND_ECC_DISABLE    (0x0 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_NAND_ECC_1_BIT      (0x1 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_NAND_ECC_4_BIT      (0x2 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_NAND_ECC_8_BIT      (0x3 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_NAND_ECC_12_BIT     (0x4 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_NAND_ECC_24_BIT     (0x5 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_NAND_ECC_40_BIT     (0x6 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_NAND_ECC_60_BIT     (0x7 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOTROM_BOOT_N           (0x1 << 12)

#define MISC_STRAP_BUS_PCIE_RC_MODE_SHIFT       6
#define MISC_STRAP_BUS_PCIE_RC_MODE MASK        (1 << MISC_STRAP_BUS_PCIE_SINGLE_LANES_SHIFT) /* 1-root complex  0-end point */
#define MISC_STRAP_CCB_SHIFT                    8
#define MISC_STRAP_CCB_MASK                     (1 << MISC_STRAP_CCB_SHIFT)
#define MISC_STRAP_USB_PMU_POLARITY_SHIFT       10
#define MISC_STRAP_USB_PMU_POLARITY_MASK        (1 << MISC_STRAP_USB_PMU_POLARITY_SHIFT)  // 1-active low, 0-active high
#endif
    uint32  miscStrapOverride;                  /* 0x04 */
    uint32  miscMaskUBUSErr;                    /* 0x08 */
    uint32  miscPeriphCtrl;                     /* 0x0c */
    uint32  miscSPImasterCtrl;                  /* 0x10 */
    uint32  miscDierevid;                       /* 0x14 */
    uint32  miscPeriphMiscCtrl;                 /* 0x18 */
    uint32  miscPeriphMiscStat;                 /* 0x1c */
    uint32  miscSoftResetB;                     /* 0x20 */
    uint32  reserved0;                          /* 0x24 */
    uint32  miscSWdebugNW[2];                   /* 0x3c */
    uint32  miscWDresetCtrl;                    /* 0x30 */
} Misc;

#define MISC ((volatile Misc * const) MISC_BASE)

typedef struct NandCtrlRegs {
    uint32 NandRevision;        /* 0x00 */
    uint32 NandCmdStart;        /* 0x04 */
#define NCMD_MASK               0x0000001f
#define NCMD_BLOCK_ERASE_MULTI  0x15
#define NCMD_PROGRAM_PAGE_MULTI 0x13
#define NCMD_STS_READ_MULTI     0x12
#define NCMD_PAGE_READ_MULTI    0x11
#define NCMD_LOW_LEVEL_OP       0x10
#define NCMD_PARAM_CHG_COL      0x0f
#define NCMD_PARAM_READ         0x0e
#define NCMD_BLK_LOCK_STS       0x0d
#define NCMD_BLK_UNLOCK         0x0c
#define NCMD_BLK_LOCK_DOWN      0x0b
#define NCMD_BLK_LOCK           0x0a
#define NCMD_FLASH_RESET        0x09
#define NCMD_BLOCK_ERASE        0x08
#define NCMD_DEV_ID_READ        0x07
#define NCMD_COPY_BACK          0x06
#define NCMD_PROGRAM_SPARE      0x05
#define NCMD_PROGRAM_PAGE       0x04
#define NCMD_STS_READ           0x03
#define NCMD_SPARE_READ         0x02
#define NCMD_PAGE_READ          0x01

    uint32 NandCmdExtAddr;      /* 0x08 */
    uint32 NandCmdAddr;         /* 0x0c */
    uint32 NandCmdEndAddr;      /* 0x10 */
    uint32 NandIntfcStatus;     /* 0x14 */
#define NIS_CTLR_READY          (1 << 31)
#define NIS_FLASH_READY         (1 << 30)
#define NIS_CACHE_VALID         (1 << 29)
#define NIS_SPARE_VALID         (1 << 28)
#define NIS_FLASH_STS_MASK      0x000000ff
#define NIS_WRITE_PROTECT       0x00000080
#define NIS_DEV_READY           0x00000040
#define NIS_PGM_ERASE_ERROR     0x00000001

    uint32 NandNandBootConfig;  /* 0x18 */
#define NBC_CS_LOCK             (1 << 31)
#define NBC_AUTO_DEV_ID_CFG     (1 << 30)
#define NBC_WR_PROT_BLK0        (1 << 28)
#define NBC_EBI_CS7_USES_NAND   (1<<15)
#define NBC_EBI_CS6_USES_NAND   (1<<14)
#define NBC_EBI_CS5_USES_NAND   (1<<13)
#define NBC_EBI_CS4_USES_NAND   (1<<12)
#define NBC_EBI_CS3_USES_NAND   (1<<11)
#define NBC_EBI_CS2_USES_NAND   (1<<10)
#define NBC_EBI_CS1_USES_NAND   (1<< 9)
#define NBC_EBI_CS0_USES_NAND   (1<< 8)
#define NBC_EBC_CS7_SEL         (1<< 7)
#define NBC_EBC_CS6_SEL         (1<< 6)
#define NBC_EBC_CS5_SEL         (1<< 5)
#define NBC_EBC_CS4_SEL         (1<< 4)
#define NBC_EBC_CS3_SEL         (1<< 3)
#define NBC_EBC_CS2_SEL         (1<< 2)
#define NBC_EBC_CS1_SEL         (1<< 1)
#define NBC_EBC_CS0_SEL         (1<< 0)

    uint32 NandCsNandXor;           /* 0x1c */
    uint32 NandLlOpNand;            /* 0x20 */
    uint32 NandMplaneBaseExtAddr;   /* 0x24 */
    uint32 NandMplaneBaseAddr;      /* 0x28 */
    uint32 NandReserved1[9];        /* 0x2c-0x4f */
    uint32 NandAccControl;          /* 0x50 */
#define NAC_RD_ECC_EN           (1 << 31)
#define NAC_WR_ECC_EN           (1 << 30)
#define NAC_CE_CARE_EN          (1 << 28)
#define NAC_RD_ERASED_ECC_EN    (1 << 27)
#define NAC_PARTIAL_PAGE_EN     (1 << 26)
#define NAC_WR_PREEMPT_EN       (1 << 25)
#define NAC_PAGE_HIT_EN         (1 << 24)
#define NAC_PREFETCH_EN         (1 << 23)
#define NAC_CACHE_MODE_EN       (1 << 22)
#define NAC_ECC_LVL_SHIFT       16
#define NAC_ECC_LVL_MASK        0x001f0000
#define NAC_ECC_LVL_DISABLE     0
#define NAC_ECC_LVL_BCH_1       1
#define NAC_ECC_LVL_BCH_2       2
#define NAC_ECC_LVL_BCH_3       3
#define NAC_ECC_LVL_BCH_4       4
#define NAC_ECC_LVL_BCH_5       5
#define NAC_ECC_LVL_BCH_6       6
#define NAC_ECC_LVL_BCH_7       7
#define NAC_ECC_LVL_BCH_8       8
#define NAC_ECC_LVL_BCH_9       9
#define NAC_ECC_LVL_BCH_10      10
#define NAC_ECC_LVL_BCH_11      11
#define NAC_ECC_LVL_BCH_12      12
#define NAC_ECC_LVL_BCH_13      13
#define NAC_ECC_LVL_BCH_14      14
#define NAC_ECC_LVL_HAMMING     15  /* Hamming if spare are size = 16, BCH15 otherwise */
#define NAC_ECC_LVL_BCH15       15
#define NAC_ECC_LVL_BCH_16      16
#define NAC_ECC_LVL_BCH_17      17
/* BCH18 to 30 for sector size = 1K. To be added when we need it */
#define NAC_SECTOR_SIZE_1K      (1 << 7)
#define NAC_SPARE_SZ_SHIFT      0
#define NAC_SPARE_SZ_MASK       0x0000007f

    uint32 NandConfigExt;       /* 0x54 */ /* Nand Flash Config Ext*/
#define NC_BLK_SIZE_MASK        (0xff << 4)
#define NC_BLK_SIZE_8192K       (0xa << 4)
#define NC_BLK_SIZE_4096K       (0x9 << 4)
#define NC_BLK_SIZE_2048K       (0x8 << 4)
#define NC_BLK_SIZE_1024K       (0x7 << 4)
#define NC_BLK_SIZE_512K        (0x6 << 4)
#define NC_BLK_SIZE_256K        (0x5 << 4)
#define NC_BLK_SIZE_128K        (0x4 << 4)
#define NC_BLK_SIZE_64K         (0x3 << 4)
#define NC_BLK_SIZE_32K         (0x2 << 4)
#define NC_BLK_SIZE_16K         (0x1 << 4)
#define NC_BLK_SIZE_8K          (0x0 << 4)
#define NC_PG_SIZE_MASK         (0xf << 0)
#define NC_PG_SIZE_16K          (0x5 << 0)
#define NC_PG_SIZE_8K           (0x4 << 0)
#define NC_PG_SIZE_4K           (0x3 << 0)
#define NC_PG_SIZE_2K           (0x2 << 0)
#define NC_PG_SIZE_1K           (0x1 << 0)
#define NC_PG_SIZE_512B         (0x0 << 0)

    uint32 NandConfig;          /* 0x58 */ /* Nand Flash Config */
#define NC_CONFIG_LOCK          (1 << 31)
#define NC_DEV_SIZE_SHIFT       24
#define NC_DEV_SIZE_MASK        (0x0f << NC_DEV_SIZE_SHIFT)
#define NC_DEV_WIDTH_MASK       (1 << 23)
#define NC_DEV_WIDTH_16         (1 << 23)
#define NC_DEV_WIDTH_8          (0 << 23)
#define NC_FUL_ADDR_SHIFT       16
#define NC_FUL_ADDR_MASK        (0x7 << NC_FUL_ADDR_SHIFT)
#define NC_BLK_ADDR_SHIFT       8
#define NC_BLK_ADDR_MASK        (0x07 << NC_BLK_ADDR_SHIFT)

    uint32 NandTiming1;         /* 0x5c */ /* Nand Flash Timing Parameters 1 */
#define NT_TREH_MASK        0x000f0000
#define NT_TREH_SHIFT       16
#define NT_TRP_MASK         0x00f00000
#define NT_TRP_SHIFT        20
    uint32 NandTiming2;         /* 0x60 */ /* Nand Flash Timing Parameters 2 */
#define NT_TREAD_MASK       0x0000000f
#define NT_TREAD_SHIFT      0
    /* 0x64 */
    uint32 NandAccControlCs1;   /* Nand Flash Access Control */
    uint32 NandConfigExtCs1;    /* Nand Flash Config Ext*/
    uint32 NandConfigCs1;       /* Nand Flash Config */
    uint32 NandTiming1Cs1;      /* Nand Flash Timing Parameters 1 */
    uint32 NandTiming2Cs1;      /* Nand Flash Timing Parameters 2 */
    /* 0x78 */
    uint32 NandAccControlCs2;   /* Nand Flash Access Control */
    uint32 NandConfigExtCs2;    /* Nand Flash Config Ext*/
    uint32 NandConfigCs2;       /* Nand Flash Config */
    uint32 NandTiming1Cs2;      /* Nand Flash Timing Parameters 1 */
    uint32 NandTiming2Cs2;      /* Nand Flash Timing Parameters 2 */
    /* 0x8c */
    uint32 NandAccControlCs3;   /* Nand Flash Access Control */
    uint32 NandConfigExtCs3;    /* Nand Flash Config Ext*/
    uint32 NandConfigCs3;       /* Nand Flash Config */
    uint32 NandTiming1Cs3;      /* Nand Flash Timing Parameters 1 */
    uint32 NandTiming2Cs3;      /* Nand Flash Timing Parameters 2 */
    /* 0xa0 */
    uint32 NandAccControlCs4;   /* Nand Flash Access Control */
    uint32 NandConfigExtCs4;    /* Nand Flash Config Ext*/
    uint32 NandConfigCs4;       /* Nand Flash Config */
    uint32 NandTiming1Cs4;      /* Nand Flash Timing Parameters 1 */
    uint32 NandTiming2Cs4;      /* Nand Flash Timing Parameters 2 */
    /* 0xb4 */
    uint32 NandAccControlCs5;   /* Nand Flash Access Control */
    uint32 NandConfigExtCs5;    /* Nand Flash Config Ext*/
    uint32 NandConfigCs5;       /* Nand Flash Config */
    uint32 NandTiming1Cs5;      /* Nand Flash Timing Parameters 1 */
    uint32 NandTiming2Cs5;      /* Nand Flash Timing Parameters 2 */
    /* 0xc8 */
    uint32 NandAccControlCs6;   /* Nand Flash Access Control */
    uint32 NandConfigExtCs6;    /* Nand Flash Config Ext*/
    uint32 NandConfigCs6;       /* Nand Flash Config */
    uint32 NandTiming1Cs6;      /* Nand Flash Timing Parameters 1 */
    uint32 NandTiming2Cs6;      /* Nand Flash Timing Parameters 2 */

    /* 0xdc */
    uint32 NandCorrStatThreshold;       /* Correctable Error Reporting Threshold */
    uint32 NandCorrStatThresholdExt;    /* Correctable Error Reporting Threshold */
    uint32 NandBlkWrProtect;            /* Block Write Protect Enable and Size */
                                        /*   for EBI_CS0b */
    uint32 NandMplaneOpcode1;

    /* 0xec */
    uint32 NandMplaneOpcode2;
    uint32 NandMplaneCtrl;
    uint32 NandReserved2[2];            /* 0xf4-0xfb */
    uint32 NandUncorrErrorCount;        /* 0xfc */

    /* 0x100 */
    uint32 NandCorrErrorCount;
    uint32 NandReadErrorCount;      /* Read Error Count */
    uint32 NandBlockLockStatus;     /* Nand Flash Block Lock Status */
    uint32 NandEccCorrExtAddr;      /* ECC Correctable Error Extended Address*/
    /* 0x110 */
    uint32 NandEccCorrAddr;         /* ECC Correctable Error Address */
    uint32 NandEccUncExtAddr;       /* ECC Uncorrectable Error Extended Addr */
    uint32 NandEccUncAddr;          /* ECC Uncorrectable Error Address */
    uint32 NandFlashReadExtAddr;    /* Flash Read Data Extended Address */
    /* 0x120 */
    uint32 NandFlashReadAddr;       /* Flash Read Data Address */
    uint32 NandProgramPageExtAddr;  /* Page Program Extended Address */
    uint32 NandProgramPageAddr;     /* Page Program Address */
    uint32 NandCopyBackExtAddr;     /* Copy Back Extended Address */
    /* 0x130 */
    uint32 NandCopyBackAddr;        /* Copy Back Address */
    uint32 NandBlockEraseExtAddr;   /* Block Erase Extended Address */
    uint32 NandBlockEraseAddr;      /* Block Erase Address */
    uint32 NandInvReadExtAddr;      /* Flash Invalid Data Extended Address */
    /* 0x140 */
    uint32 NandInvReadAddr;         /* Flash Invalid Data Address */
    uint32 NandInitStatus;
    uint32 NandOnfiStatus;          /* ONFI Status */
    uint32 NandOnfiDebugData;       /* ONFI Debug Data */

    uint32 NandSemaphore;           /* 0x150 */ /* Semaphore */
    uint32 NandReserved3[16];       /* 0x154-0x193 */

    /* 0x194 */
    uint32 NandFlashDeviceId;       /* Nand Flash Device ID */
    uint32 NandFlashDeviceIdExt;    /* Nand Flash Extended Device ID */
    uint32 NandLlRdData;            /* Nand Flash Low Level Read Data */

    uint32 NandReserved4[24];       /* 0x1a0 - 0x1ff */

    /* 0x200 */
    uint32 NandSpareAreaReadOfs0;   /* Nand Flash Spare Area Read Bytes 0-3 */
    uint32 NandSpareAreaReadOfs4;   /* Nand Flash Spare Area Read Bytes 4-7 */
    uint32 NandSpareAreaReadOfs8;   /* Nand Flash Spare Area Read Bytes 8-11 */
    uint32 NandSpareAreaReadOfsC;   /* Nand Flash Spare Area Read Bytes 12-15*/
    /* 0x210 */
    uint32 NandSpareAreaReadOfs10;  /* Nand Flash Spare Area Read Bytes 16-19 */
    uint32 NandSpareAreaReadOfs14;  /* Nand Flash Spare Area Read Bytes 20-23 */
    uint32 NandSpareAreaReadOfs18;  /* Nand Flash Spare Area Read Bytes 24-27 */
    uint32 NandSpareAreaReadOfs1C;  /* Nand Flash Spare Area Read Bytes 28-31*/
    /* 0x220 */
    uint32 NandSpareAreaReadOfs20;  /* Nand Flash Spare Area Read Bytes 32-35 */
    uint32 NandSpareAreaReadOfs24;  /* Nand Flash Spare Area Read Bytes 36-39 */
    uint32 NandSpareAreaReadOfs28;  /* Nand Flash Spare Area Read Bytes 40-43 */
    uint32 NandSpareAreaReadOfs2C;  /* Nand Flash Spare Area Read Bytes 44-47*/
    /* 0x230 */
    uint32 NandSpareAreaReadOfs30;  /* Nand Flash Spare Area Read Bytes 48-51 */
    uint32 NandSpareAreaReadOfs34;  /* Nand Flash Spare Area Read Bytes 52-55 */
    uint32 NandSpareAreaReadOfs38;  /* Nand Flash Spare Area Read Bytes 56-59 */
    uint32 NandSpareAreaReadOfs3C;  /* Nand Flash Spare Area Read Bytes 60-63*/

    uint32 NandReserved5[16];       /* 0x240-0x27f */

    /* 0x280 */
    uint32 NandSpareAreaWriteOfs0;  /* Nand Flash Spare Area Write Bytes 0-3 */
    uint32 NandSpareAreaWriteOfs4;  /* Nand Flash Spare Area Write Bytes 4-7 */
    uint32 NandSpareAreaWriteOfs8;  /* Nand Flash Spare Area Write Bytes 8-11 */
    uint32 NandSpareAreaWriteOfsC;  /* Nand Flash Spare Area Write Bytes 12-15 */
    /* 0x290 */
    uint32 NandSpareAreaWriteOfs10; /* Nand Flash Spare Area Write Bytes 16-19 */
    uint32 NandSpareAreaWriteOfs14; /* Nand Flash Spare Area Write Bytes 20-23 */
    uint32 NandSpareAreaWriteOfs18; /* Nand Flash Spare Area Write Bytes 24-27 */
    uint32 NandSpareAreaWriteOfs1C; /* Nand Flash Spare Area Write Bytes 28-31 */
    /* 0x2a0 */
    uint32 NandSpareAreaWriteOfs20; /* Nand Flash Spare Area Write Bytes 32-35 */
    uint32 NandSpareAreaWriteOfs24; /* Nand Flash Spare Area Write Bytes 36-39 */
    uint32 NandSpareAreaWriteOfs28; /* Nand Flash Spare Area Write Bytes 40-43 */
    uint32 NandSpareAreaWriteOfs2C; /* Nand Flash Spare Area Write Bytes 44-47 */
    /* 0x2b0 */
    uint32 NandSpareAreaWriteOfs30; /* Nand Flash Spare Area Write Bytes 48-51 */
    uint32 NandSpareAreaWriteOfs34; /* Nand Flash Spare Area Write Bytes 52-55 */
    uint32 NandSpareAreaWriteOfs38; /* Nand Flash Spare Area Write Bytes 56-59 */
    uint32 NandSpareAreaWriteOfs3C; /* Nand Flash Spare Area Write Bytes 60-63 */
    /* 0x2c0 */
    uint32 NandDdrTiming;
    uint32 NandDdrNcdlCalibCtl;
    uint32 NandDdrNcdlCalibPeriod;
    uint32 NandDdrNcdlCalibStat;
    /* 0x2d0 */
    uint32 NandDdrNcdlMode;
    uint32 NandDdrNcdlOffset;
    uint32 NandDdrPhyCtl;
    uint32 NandDdrPhyBistCtl;
    /* 0x2e0 */
    uint32 NandDdrPhyBistStat;
    uint32 NandDdrDiagStat0;
    uint32 NandDdrDiagStat1;
    uint32 NandReserved6[69];       /* 0x2ec-0x3ff */

    /* 0x400 */
    uint32 NandFlashCache[128];     /* 0x400-0x5ff */
} NandCtrlRegs;

/*
 * ** NAND Interrupt Controller Registers
 * */
typedef struct NandIntrCtrlRegs {
    uint32 NandInterrupt;
#define NINT_STS_MASK           0x00000fff
#define NINT_ECC_ERROR_CORR_SEC 0x00000800
#define NINT_ECC_ERROR_UNC_SEC  0x00000400
#define NINT_CTRL_READY_SEC     0x00000200
#define NINT_INV_ACC_SEC        0x00000100
#define NINT_ECC_ERROR_CORR     0x00000080
#define NINT_ECC_ERROR_UNC      0x00000040
#define NINT_DEV_RBPIN          0x00000020
#define NINT_CTRL_READY         0x00000010
#define NINT_PAGE_PGM           0x00000008
#define NINT_COPY_BACK          0x00000004
#define NINT_BLOCK_ERASE        0x00000002
#define NINT_NP_READ            0x00000001
    uint32 NandInterruptEn;
#define NINT_ENABLE_MASK        0x0000ffff
    uint32 NandBaseAddr0;   /* Default address when booting from NAND flash */
    uint32 NandBaseAddr1;   /* Secondary base address for NAND flash */
} NandIntrCtrlRegs;

#define NAND_INTR ((volatile NandIntrCtrlRegs * const) NAND_INTR_BASE)
#define NAND ((volatile NandCtrlRegs * const) NAND_REG_BASE)
#define NAND_CACHE ((volatile uint8 * const) NAND_CACHE_BASE)

/*
 * Top Control
 */
typedef struct TopControl {
   uint32 SGMIIFiberDetect;   /* 0x00 */
   uint32 DgSensePadCtl;     /* 0x04 */
#define DG_CTRL_SHIFT   4
#define DG_CTRL_MASK    0x00000030
#define DG_EN_SHIFT     3
#define DG_TRIM_SHIFT   0
#define DG_TRIM_MASK    0x00000007
   uint32 TPDirOverRide[2];   /* 0x08 */
   uint32 RescaleReadData[2]; /* 0x10 */
   uint32 RescalIPCtrl;       /* 0x18 */
#define RESCAL_RSTB_SHIFT           1
#define RESCAL_RSTB                 (0x1<<RESCAL_RSTB_SHIFT)
#define RESCAL_PWRDN_SHIFT          0
#define RESCAL_PWRDN                (0x1<<RESCAL_PWRDN_SHIFT)

   uint32 ResetStatus;        /* 0x1c */
#define PCIE_RESET_STATUS       0x10000000
#define SW_RESET_STATUS         0x20000000
#define HW_RESET_STATUS         0x40000000
#define POR_RESET_STATUS        0x80000000
#define RESET_STATUS_MASK       0xF0000000

   uint32 TODsync;            /* 0x20 */
   uint32 MDIOmasterSelect;   /* 0x24 */
   uint32 AdslClockSample;    /* 0x28 */
   uint32 OrionIntPending;    /* 0x2c */
} TopControl;

#define TOPCTRL ((volatile TopControl * const) TOP_CONTROL_BASE)

/*
** Timer
*/
#define TIMER_64BIT
typedef struct Timer {
    uint64        TimerCtl0;
    uint64        TimerCtl1;
    uint64        TimerCtl2;
    uint64        TimerCtl3;
#define TIMERENABLE     (1ULL << 63)
#define RSTCNTCLR       (1ULL << 62)
    uint64        TimerCnt0;
    uint64        TimerCnt1;
    uint64        TimerCnt2;
    uint64        TimerCnt3;
#define TIMER_COUNT_MASK   0x3FFFFFFFFFFFFFFFULL
    uint32        TimerMask;
#define TIMER0EN        0x01
#define TIMER1EN        0x02
#define TIMER2EN        0x04
#define TIMER3EN        0x08
    uint32        TimerInts;
#define TIMER0          0x01
#define TIMER1          0x02
#define TIMER2          0x04
#define TIMER3          0x08
    uint32        ResetReason;
#define SW_INI_RESET            0x00000001
    uint32        spare[3];
} Timer;

#define TIMER ((volatile Timer * const) TIMR_BASE)

typedef struct BIUCFG_Access {
    uint32 permission;        /* 0x0 */
    uint32 sbox;              /* 0x4 */
    uint32 cpu_defeature;     /* 0x8 */
    uint32 dbg_security;      /* 0xc */
    uint32 rsvd1[32];         /* 0x10 - 0x8f */
    uint64 violation[2];      /* 0x90 - 0x9f */
    uint32 ts_access[2];      /* 0xa0 - 0xa7 */
    uint32 rsvd2[22];         /* 0xa8 - 0xff */
}BIUCFG_Access;

typedef struct BIUCFG_Cluster {
    uint32 permission;        /* 0x0 */
    uint32 config;            /* 0x4 */
    uint32 status;            /* 0x8 */
    uint32 control;           /* 0xc */
    uint32 cpucfg;            /* 0x10 */
    uint32 dbgrom;            /* 0x14 */
    uint32 rsvd1[2];          /* 0x18 - 0x1f */
    uint64 rvbar_addr[4];     /* 0x20 - 0x3f */
    uint32 rsvd2[48];         /* 0x40 - 0xff */
}BIUCFG_Cluster;

typedef struct BIUCFG_Bac {
    uint32 bac_permission;    /* 0x00 */
    uint32 bac_periphbase;    /* 0x04 */
    uint32 rsvd[2];           /* 0x08 - 0x0f */
    uint32 bac_event;         /* 0x10 */
    uint32 rsvd_1[3];         /* 0x14 - 0x1f */
    uint32 bac_ccicfg;        /* 0x20 */
    uint32 bac_cciaddr;       /* 0x24 */
    uint32 rsvd_2[4];         /* 0x28 - 0x37 */
    uint32 bac_ccievs2;       /* 0x38 */
    uint32 bac_ccievs3;       /* 0x3c */
    uint32 bac_ccievs4;       /* 0x40 */
    uint32 rsvd_3[3];         /* 0x44 - 0x4f */
    uint32 bac_ccievm0;       /* 0x50 */
    uint32 bac_ccievm1;       /* 0x54 */
    uint32 rsvd_4[2];         /* 0x58 - 0x5f */
    uint32 bac_dapapbcfg;     /* 0x60 */
    uint32 bac_status;        /* 0x64 */
    uint32 rsvd_5[2];         /* 0x68 - 0x6f */
    uint32 cpu_therm_irq_cfg; /* 0x70 */
    uint32 cpu_therm_threshold_cfg; /* 0x74 */
    uint32 rsvd_6;            /* 0x78 */
    uint32 cpu_therm_temp;    /* 0x7c */
    uint32 rsvd_7[32];        /* 0x80 - 0xff */
} BIUCFG_Bac;

typedef struct BIUCFG_Ubus {
    uint32 ubus_permission;  /* 0x00 */
    uint32 rsvd;             /* 0x04 */
    uint32 ax2u_modid;       /* 0x8 */
    uint32 rsvd_1[5];        /* 0xc */
    uint32 ccb_modid;        /* 0x20 */
    uint32 cci_modid;        /* 0x24 */
    uint32 u2up_modid;       /* 0x28 */
    uint32 status;           /* 0x2c */
#define BIUCFG_UBUS_CCB_MOD_SEL   0x1
    uint32 rsvd_2[52];       /* 0x30 - 0xff */
} BIUCFG_Ubus;

typedef struct BIUCFG_Aux {
    uint32 aux_permission;    /* 0x00 */
    uint32 rsvd[3];           /* 0x04 - 0x0f */
    uint32 c0_clk_control;    /* 0x10 */
    uint32 c0_clk_ramp;       /* 0x14 */
    uint32 c0_clk_pattern;    /* 0x18 */
    uint32 rsvd_1;            /* 0x1c */
    uint32 c1_clk_control;    /* 0x20 */
    uint32 c1_clk_ramp;       /* 0x24 */
    uint32 c1_clk_pattern;    /* 0x28 */
    uint32 rsvd_2[53];        /* 0x2c - 0xff */
} BIUCFG_Aux;

typedef struct BIUCFG {
    BIUCFG_Access access;         /* 0x0 - 0xff*/
    BIUCFG_Cluster cluster[2];    /* 0x100 - 0x2ff*/
    BIUCFG_Bac bac;               /* 0x300 - 0x3ff */
    BIUCFG_Ubus ubus;             /* 0x400 - 0x4ff */
    uint32 anonymous[128];        /* 0x500 - 0x6ff */
    BIUCFG_Aux aux;               /* 0x700 - 0x7ff */
    uint32 anonymous_1[2560];     /* 0x800 - 0x2fff */
}BIUCFG;

#define BIUCFG ((volatile BIUCFG * const) BIUCFG_BASE)

typedef struct CCI500_SlaveIntf {
#define SNOOP_CTRL_ENABLE_SNOOP            0x1
    uint32 snoop_ctrl;        /* 0x0 */
#define SHARE_OVR_SHAREABLE_OVR_SHIFT      0x0
#define SHARE_OVR_SHAREABLE_OVR_MASK       0x3
#define SHARE_OVR_SHAREABLE_OVR_NONSHR     0x2
#define SHARE_OVR_SHAREABLE_OVR_SHR        0x3
    uint32 share_ovr;         /* 0x4 */
    uint32 rsvd1[62];         /* 0x8 - 0xff */
    uint32 arqos_ovr;         /* 0x100 */
    uint32 awqos_ovr;         /* 0x104 */
    uint32 rsvd2[2];          /* 0x108 - 0x10f */
    uint32 qos_max_ot;        /* 0x110 */
    uint32 rsvd3[955];        /* 0x114 - 0xfff */
}CCI500_SlaveIntf;

typedef struct CCI500_EventCounter {
    uint32 sel;          /* 0x0 */
    uint32 data;         /* 0x4 */
    uint32 ctrl;         /* 0x8 */
    uint32 clr_ovfl;     /* 0xC */
    uint32 rsvd[16380];  /* 0x10 - 0xffff */
}CCI500_EventCounter;

typedef struct CCI500 {
#define CONTROL_OVERRIDE_SNOOP_DISABLE     0x1
#define CONTROL_OVERRIDE_SNOOP_FLT_DISABLE 0x4
    uint32 ctrl_ovr;        /* 0x0 */
    uint32 rsvd1;           /* 0x4 */
#define SECURE_ACCESS_UNSECURE_ENABLE      0x1
    uint32 secr_acc;        /* 0x8 */
    uint32 status;          /* 0xc */
#define STATUS_CHANGE_PENDING              0x1
    uint32 impr_err;        /* 0x10 */
    uint32 qos_threshold;   /* 0x14 */
    uint32 rsvd2[58];       /* 0x18 - 0xff */
    uint32 pmu_ctrl;        /* 0x100 */
#define DBG_CTRL_EN_INTF_MON               0x1
    uint32 debug_ctrl;      /* 0x104 */
    uint32 rsvd3[958];      /* 0x108 - 0xfff */
#define SLAVEINTF_COHERENCY_PORT           0x0
#define SLAVEINTF_CPU_CLUSTER              0x1
    CCI500_SlaveIntf si[7]; /* 0x1000 - 0x7fff */
    uint32 rsvd4[8192];     /* 0x8000 - 0xffff */
    CCI500_EventCounter evt_cntr[8]; /* 0x10000 - 0x8ffff */
}CCI500;
#define CCI500 ((volatile CCI500 * const) CCI500_BASE)

typedef struct CCB_AXI_CFG {
    uint32 modlue_id;
    uint32 rev_id;
    uint32 reserved[2];
    uint32 axi_config0;
#define CCB_AXI_ENABLE_COHERENCY   0x1
    uint32 axi_config1;
}CCB_AXI_CFG;

typedef struct UBUSCCB {
    uint32       reserved[1024];   /* 0 - 0xfff */
    CCB_AXI_CFG  axi_cfg;          /* 0x1000 */
}UBUSCCB;
#define CCB ((volatile UBUSCCB * const) CCB_BASE)


#ifdef __cplusplus
}
#endif

#endif

