/*
<:copyright-BRCM:2013:DUAL/GPL:standard

   Copyright (c) 2013 Broadcom 
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

#ifndef __BCM63138_MAP_PART_H
#define __BCM63138_MAP_PART_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bcmtypes.h"
#include "bcm_io_map.h"

#if !defined(REG_BASE)
#define REG_BASE                 0x80000000
#endif
#if !defined(PER_BASE)
#define PER_BASE                 0xfffe0000
#endif

#define CHIP_FAMILY_ID_HEX 0x63138
#define CHIP_63132_ID_HEX  0x63132

/* Physical address for all the registers */
#define PERF_PHYS_BASE           (PER_BASE + 0x00008000)  /* chip control */
#define TIMR_PHYS_BASE           (PER_BASE + 0x00008080)  /* timer registers */
#define NAND_INTR_PHYS_BASE      (PER_BASE + 0x000080f0)  /* NAND int register */
#define GPIO_PHYS_BASE           (PER_BASE + 0x00008100)  /* gpio registers */
#define I2C_BASE                 (PER_BASE + 0x0000be00)  /* I2C regsiters */
#define MISC_PHYS_BASE           (PER_BASE + 0x00008180)  /* Miscellaneous Registers */
#define SOTP_PHYS_BASE           (PER_BASE + 0x00008200)  /* SOTP register */
#define PKA_PHYS_BASE            (PER_BASE + 0x00008280)
#define RNG_PHYS_BASE            (PER_BASE + 0x00008300)
#define UART0_PHYS_BASE          (PER_BASE + 0x00008600)  /* uart registers */
#define UART_PHYS_BASE           UART0_PHYS_BASE
#define UART1_PHYS_BASE          (PER_BASE + 0x00008620)  /* uart registers */
#define LED_PHYS_BASE            (PER_BASE + 0x00008700)  /* LED control registers */
#define I2S_PHYS_BASE            (PER_BASE + 0x00008900)
#define AES0_PHYS_BASE           (PER_BASE + 0x00008980)
#define AES1_PHYS_BASE           (PER_BASE + 0x00008a00)
#define HSSPIM_PHYS_BASE         (PER_BASE + 0x00009000)  /* High-Speed SPI registers */
#define NAND_REG_PHYS_BASE       (PER_BASE + 0x0000a000)  /* nand interrupt control */
#define NAND_CACHE_PHYS_BASE     (PER_BASE + 0x0000a400)
#define JTAG_OTP_PHYS_BASE       (PER_BASE + 0x0000bb00)
#define JTAG_IOTP_PHYS_BASE      (PER_BASE + 0x0000bd00)
#define EMMC_HOSTIF_PHYS_BASE    (PER_BASE + 0x0000c000)
#define EMMC_TOP_CFG_PHYS_BASE   (PER_BASE + 0x0000c100)
#define EMMC_BOOT_PHYS_BASE      (PER_BASE + 0x0000c200)
#define AHBSS_CTRL_PHYS_BASE     (PER_BASE + 0x0000c300)
#define HS_UART_PHYS_BASE        (PER_BASE + 0x0000c400)
#define PL081_DMA_PHYS_BASE      (PER_BASE + 0x0000d000)
#define M2M_DMA_PHYS_BASE        (PER_BASE + 0x0000e000)
#define BOOTLUT_PHYS_BASE        (PER_BASE + 0x00010000) 

/*TODO change the naming of usbdevice regs to USB20D */
#define USB_CTL_PHYS_BASE        (REG_BASE + 0x00001000)   /* USB 2.0 device control */
#define USB_DMA_PHYS_BASE        (REG_BASE + 0x00001800)  /* USB 2.0 device DMA registers */
#define MEMC_PHYS_BASE           (REG_BASE + 0x00002000)  /* DDR IO Buf Control */
#define MEMC_BASE_OFF_4K         (MEMC_PHYS_BASE + 0x00001000)
#define DDRPHY_PHYS_BASE         (REG_BASE + 0x00003000)
#define SAR_PHYS_BASE            (REG_BASE + 0x00004000)
#define SAR_DMA_PHYS_BASE        (REG_BASE + 0x00007800)  /* ATM SAR DMA control */
#define SATA_PHYS_BASE           (REG_BASE + 0x00008000)
#define USBH_PHYS_BASE           (REG_BASE + 0x0000c000)
#define USBH_CFG_PHYS_BASE       (REG_BASE + 0x0000c200)
#define USB_EHCI_PHYS_BASE       (REG_BASE + 0x0000c300)  /* USB host registers */
#define USB_OHCI_PHYS_BASE       (REG_BASE + 0x0000c400)  /* USB host registers */
#define USB_XHCI_PHYS_BASE       (REG_BASE + 0x0000d000)  /* USB host registers */
#define USB_XHCI1_PHYS_BASE      (REG_BASE + 0x0000e000)  /* USB host registers */
#define ERROR_PORT_PHYS_BASE     (REG_BASE + 0x00010000)

#define AIP_PHYS_BASE            (REG_BASE + 0x00018000)
#define ARM_UART_PHYS_BASE       (REG_BASE + 0x00019000)
#define ETB_PHYS_BASE            (REG_BASE + 0x0001a000)
#define L2C_PHYS_BASE            (REG_BASE + 0x0001d000)
#define SCU_PHYS_BASE            (REG_BASE + 0x0001e000)
#define ARMGTIM_PHYS_BASE        (REG_BASE + 0x0001e200)
#define GICC_PHYS_BASE           (REG_BASE + 0x0001e100)
#define GICD_PHYS_BASE           (REG_BASE + 0x0001f000)
#define ARMCFG_PHYS_BASE         (REG_BASE + 0x00020000)


#define DECT_PHYS_BASE           (REG_BASE + 0x00040000)
#define DECT_AHB_REG_PHYS_BASE   DECT_PHYS_BASE
#define DECT_SHIM_CTRL_PHYS_BASE (REG_BASE + 0x00050000)
#define DECT_SHIM_DMA_CTRL_PHYS_BASE   (REG_BASE + 0x00050050)
#define DECT_APB_REG_PHYS_BASE   (REG_BASE + 0x00050800)
#define PCIE0_PHYS_BASE          (REG_BASE + 0x00060000)
#define PCIE1_PHYS_BASE          (REG_BASE + 0x00070000)

#define SF2_PHYS_BASE            (REG_BASE + 0x00080000)
#define SWITCH_PHYS_BASE         SF2_PHYS_BASE
#define APM_PHYS_BASE            (REG_BASE + 0x00100000)
#define RDP_PHYS_BASE            (REG_BASE + 0x00200000)
#define PMC_PHYS_BASE            (REG_BASE + 0x00400000)
#define PROC_MON_PHYS_BASE       (REG_BASE + 0x00480000)
#define DSLPHY_PHYS_BASE         (REG_BASE + 0x00600000)
#define DSLPHY_PHYS_TXPAF_BASE   (REG_BASE + 0x00656800)
#define DSLPHY_AFE_PHYS_BASE     (REG_BASE + 0x00657300)
#define DSLLMEM_PHYS_BASE        (REG_BASE + 0x00700000)

#define PCIE0_MEM_PHYS_BASE      0x90000000
#define PCIE1_MEM_PHYS_BASE      0xa0000000

#define SPIFLASH_PHYS_BASE       0xffd00000  /* spi flash direct access address */
#define NANDFLASH_PHYS_BASE      0xffe00000  /* nand flash direct access address */

/* Physical and access(could be virtual or physical) bases address for
 * all the registers */
#define SPIFLASH_BASE            BCM_IO_ADDR(SPIFLASH_PHYS_BASE)
#define NANDFLASH_BASE           BCM_IO_ADDR(NANDFLASH_PHYS_BASE)
#define PERF_BASE                BCM_IO_ADDR(PERF_PHYS_BASE)
#define TIMR_BASE                BCM_IO_ADDR(TIMR_PHYS_BASE)
#define NAND_INTR_BASE           BCM_IO_ADDR(NAND_INTR_PHYS_BASE)
#define GPIO_BASE                BCM_IO_ADDR(GPIO_PHYS_BASE)
#define MISC_BASE                BCM_IO_ADDR(MISC_PHYS_BASE)
#define SOTP_BASE                BCM_IO_ADDR(SOTP_PHYS_BASE)
#define PKA_BASE                 BCM_IO_ADDR(PKA_PHYS_BASE)
#define RNG_BASE                 BCM_IO_ADDR(RNG_PHYS_BASE)
#define UART0_BASE               BCM_IO_ADDR(UART0_PHYS_BASE)
#define UART_BASE                UART0_BASE
#define UART1_BASE               BCM_IO_ADDR(UART1_PHYS_BASE)
#define LED_BASE                 BCM_IO_ADDR(LED_PHYS_BASE)
#define I2S_BASE                 BCM_IO_ADDR(I2S_PHYS_BASE)
#define AES0_BASE                BCM_IO_ADDR(AES0_PHYS_BASE)
#define AES1_BASE                BCM_IO_ADDR(AES1_PHYS_BASE)
#define HSSPIM_BASE              BCM_IO_ADDR(HSSPIM_PHYS_BASE)
#define NAND_REG_BASE            BCM_IO_ADDR(NAND_REG_PHYS_BASE)
#define NAND_CACHE_BASE          BCM_IO_ADDR(NAND_CACHE_PHYS_BASE)
#define JTAG_OTP_BASE            BCM_IO_ADDR(JTAG_OTP_PHYS_BASE)
#define JTAG_IOTP_BASE           BCM_IO_ADDR(JTAG_IOTP_PHYS_BASE)
#define EMMC_HOSTIF_BASE         BCM_IO_ADDR(EMMC_HOSTIF_PHYS_BASE) 
#define EMMC_TOP_CFG_BASE        BCM_IO_ADDR(EMMC_TOP_CFG_PHYS_BASE)
#define EMMC_BOOT_BASE           BCM_IO_ADDR(EMMC_BOOT_PHYS_BASE)   
#define AHBSS_CTRL_BASE          BCM_IO_ADDR(AHBSS_CTRL_PHYS_BASE)  
#define HS_UART_BASE             BCM_IO_ADDR(HS_UART_PHYS_BASE)  
#define PL081_DMA_BASE           BCM_IO_ADDR(PL081_DMA_PHYS_BASE)
#define BOOTLUT_BASE             BCM_IO_ADDR(BOOTLUT_PHYS_BASE)

/*TODO keep the naming convention same as RDB */
#define USB_CTL_BASE             BCM_IO_ADDR(USB_CTL_PHYS_BASE)
#define USB_DMA_BASE             BCM_IO_ADDR(USB_DMA_PHYS_BASE)
#define MEMC_BASE                BCM_IO_ADDR(MEMC_PHYS_BASE)
#define DDRPHY_BASE              BCM_IO_ADDR(DDRPHY_PHYS_BASE)
#define SAR_BASE                 BCM_IO_ADDR(SAR_PHYS_BASE)
#define SAR_DMA_BASE             BCM_IO_ADDR(SAR_DMA_PHYS_BASE)
#define SATA_BASE                BCM_IO_ADDR(SATA_PHYS_BASE)
#define USBH_BASE                BCM_IO_ADDR(USBH_PHYS_BASE)
#define USBH_CFG_BASE            BCM_IO_ADDR(USBH_CFG_PHYS_BASE)
#define USB_EHCI_BASE            BCM_IO_ADDR(USB_EHCI_PHYS_BASE)
#define USB_OHCI_BASE            BCM_IO_ADDR(USB_OHCI_PHYS_BASE)
#define USB_XHCI_BASE            BCM_IO_ADDR(USB_XHCI_PHYS_BASE)
#define USB_XHCI1_BASE           BCM_IO_ADDR(USB_XHCI1_PHYS_BASE)
#define ERROR_PORT_BASE          BCM_IO_ADDR(ERROR_PORT_PHYS_BASE)

#define AIP_BASE                 BCM_IO_ADDR(AIP_PHYS_BASE)
#define ARM_UART_BASE            BCM_IO_ADDR(ARM_UART_PHYS_BASE)
#define ETB_BASE                 BCM_IO_ADDR(ETB_PHYS_BASE)
#define L2C_BASE                 BCM_IO_ADDR(L2C_PHYS_BASE)
#define SCU_BASE                 BCM_IO_ADDR(SCU_PHYS_BASE)
#define ARMGTIM_BASE             BCM_IO_ADDR(ARMGTIM_PHYS_BASE)
#define ARMCFG_BASE              BCM_IO_ADDR(ARMCFG_PHYS_BASE)
#define GICC_BASE                BCM_IO_ADDR(GICC_PHYS_BASE)
#define GICD_BASE                BCM_IO_ADDR(GICD_PHYS_BASE)

#define SWITCH_BASE              BCM_IO_ADDR(SF2_PHYS_BASE)
#define SWITCH_REG_BASE             (SWITCH_BASE + 0x40000UL)
#define SWITCH_DIRECT_DATA_WR_REG   (SWITCH_REG_BASE + 0x00008UL)
#define SWITCH_DIRECT_DATA_RD_REG   (SWITCH_REG_BASE + 0x0000cUL)
#define SWITCH_CROSSBAR_REG         (SWITCH_REG_BASE + 0x000acUL)
#define SWITCH_MDIO_BASE            (SWITCH_BASE + SWITCH_MDIO_OFFSET)
#define SWITCH_ACB_BASE             (SWITCH_BASE + SWITCH_ACB_OFFSET)

#define APM_BASE                 BCM_IO_ADDR(APM_PHYS_BASE)
#define RDP_BASE                 BCM_IO_ADDR(RDP_PHYS_BASE)
#define PMC_BASE                 BCM_IO_ADDR(PMC_PHYS_BASE)
#define PROC_MON_BASE            BCM_IO_ADDR(PROC_MON_PHYS_BASE)
#define DSLPHY_BASE              BCM_IO_ADDR(DSLPHY_PHYS_BASE)
#define DSLPHY_AFE_BASE          BCM_IO_ADDR(DSLPHY_AFE_PHYS_BASE)
#define DSLLMEM_BASE             BCM_IO_ADDR(DSLLMEM_PHYS_BASE)
#define TXPAF_PROCESSOR_BASE     BCM_IO_ADDR(DSLPHY_PHYS_TXPAF_BASE)


#ifndef __ASSEMBLER__

typedef struct UBUSInterface {
   uint32 CFG;       /* 0x00 */
#define UBUSIF_CFG_WRITE_REPLY_MODE_SHIFT   0x0  
#define UBUSIF_CFG_WRITE_REPLY_MODE_MASK    (0x1<< UBUSIF_CFG_WRITE_REPLY_MODE_SHIFT)
#define UBUSIF_CFG_WRITE_BURST_MODE_SHIFT   0x1  
#define UBUSIF_CFG_WRITE_BURST_MODE_MASK    (0x1<< UBUSIF_CFG_WRITE_BURST_MODE_SHIFT)
#define UBUSIF_CFG_INBAND_ERR_MASK_SHIFT    0x2
#define UBUSIF_CFG_INBAND_ERR_MASK_MASK     (0x1<< UBUSIF_CFG_INBAND_ERR_MASK_SHIFT)
#define UBUSIF_CFG_OOB_ERR_MASK_SHIFT       0x3
#define UBUSIF_CFG_OOB_ERR_MASK_MASK        (0x1<< UBUSIF_CFG_OOB_ERR_MASK_SHIFT)
   uint32 SRC_QUEUE_CTRL_0;   /* 0x04 */
   uint32 SRC_QUEUE_CTRL_1;   /* 0x08 */
   uint32 SRC_QUEUE_CTRL_2;   /* 0x0c */
   uint32 SRC_QUEUE_CTRL_3;   /* 0x10 */
   uint32 REP_ARB_MODE;    /* 0x14 */
#define UBUSIF_REP_ARB_MODE_FIFO_MODE_SHIFT 0x0
#define UBUSIF_REP_ARB_MODE_FIFO_MODE_MASK  (0x1<<UBUSIF_REP_ARB_MODE_FIFO_MODE_SHIFT)
   uint32 SCRATCH;         /* 0x18 */
   uint32 DEBUG_R0;     /* 0x1c */
   uint32 unused[8];    /* 0x20-0x3f */
} UBUSInterface;

typedef struct EDISEngine {
   uint32 REV_ID;       /* 0x00 */
   uint32 CTRL_TRIG;    /* 0x04 */
   uint32 CTRL_MODE;    /* 0x08 */
   uint32 CTRL_SIZE;    /* 0x0c */
   uint32 CTRL_ADDR_START;    /* 0x10 */
   uint32 CTRL_ADDR_START_EXT;   /* 0x14 */
   uint32 CTRL_ADDR_END;      /* 0x18 */
   uint32 CTRL_ADDR_END_EXT;  /* 0x1c */
   uint32 CTRL_WRITE_MASKS;   /* 0x20 */
   uint32 unused0[7];      /* 0x24-0x3f */
   uint32 STAT_MAIN;    /* 0x40 */
   uint32 STAT_WORDS_WRITTEN; /* 0x44 */
   uint32 STAT_WORDS_READ;    /* 0x48 */
   uint32 STAT_ERROR_COUNT;   /* 0x4c */
   uint32 STAT_ERROR_BITS;    /* 0x50 */
   uint32 STAT_ADDR_LAST;     /* 0x54 */
   uint32 STAT_ADDR_LAST_EXT; /* 0x58 */
   uint32 unused1[8];      /* 0x5c-0x7b */
   uint32 STAT_DEBUG;      /* 0x7c */
   uint32 STAT_DATA_PORT[8];  /* 0x80-0x9c */
   uint32 GEN_LFSR_STATE[4];  /* 0xa0-0xac */
   uint32 GEN_CLOCK;    /* 0xb0 */
   uint32 GEN_PATTERN;     /* 0xb4 */
   uint32 unused2[2];      /* 0xb8-0xbf */
   uint32 BYTELANE_0_CTRL_LO; /* 0xc0 */
   uint32 BYTELANE_0_CTRL_HI; /* 0xc4 */
   uint32 BYTELANE_1_CTRL_LO; /* 0xc8 */
   uint32 BYTELANE_1_CTRL_HI; /* 0xcc */
   uint32 BYTELANE_2_CTRL_LO; /* 0xd0 */
   uint32 BYTELANE_2_CTRL_HI; /* 0xd4 */
   uint32 BYTELANE_3_CTRL_LO; /* 0xd8 */
   uint32 BYTELANE_3_CTRL_HI; /* 0xdc */
   uint32 BYTELANE_0_STAT_LO; /* 0xe0 */
   uint32 BYTELANE_0_STAT_HI; /* 0xe4 */
   uint32 BYTELANE_1_STAT_LO; /* 0xe8 */
   uint32 BYTELANE_1_STAT_HI; /* 0xec */
   uint32 BYTELANE_2_STAT_LO; /* 0xf0 */
   uint32 BYTELANE_2_STAT_HI; /* 0xf4 */
   uint32 BYTELANE_3_STAT_LO; /* 0xf8 */
   uint32 BYTELANE_3_STAT_HI; /* 0xfc */
} EDISEngine;

typedef struct SecureRangeCheckers {
   uint32 LOCK;         /* 0x00 */
   uint32 LOG_INFO_0;      /* 0x04 */
   uint32 LOG_INFO_1;      /* 0x08 */
   uint32 CTRL_0;       /* 0x0c */
   uint32 UBUS0_PORT_0;    /* 0x10 */
   uint32 BASE_0;       /* 0x14 */
   uint32 CTRL_1;       /* 0x18 */
   uint32 UBUS0_PORT_1;    /* 0x1c */
   uint32 BASE_1;       /* 0x20 */
   uint32 CTRL_2;       /* 0x24 */
   uint32 UBUS0_PORT_2;    /* 0x28 */
   uint32 BASE_2;       /* 0x2c */
   uint32 CTRL_3;       /* 0x30 */
   uint32 UBUS0_PORT_3;    /* 0x34 */
   uint32 BASE_3;       /* 0x38 */
   uint32 CTRL_4;       /* 0x3c */
   uint32 UBUS0_PORT_4;    /* 0x40 */
   uint32 BASE_4;       /* 0x44 */
   uint32 CTRL_5;       /* 0x48 */
   uint32 UBUS0_PORT_5;    /* 0x4c */
   uint32 BASE_5;       /* 0x50 */
   uint32 CTRL_6;       /* 0x54 */
   uint32 UBUS0_PORT_6;    /* 0x58 */
   uint32 BASE_6;       /* 0x5c */
   uint32 CTRL_7;       /* 0x60 */
   uint32 UBUS0_PORT_7;    /* 0x64 */
   uint32 BASE_7;       /* 0x68 */
   uint32 unused[37];      /* 0x6c-0xff */
} SecureRangeCheckers;

typedef struct DDRPhyControl {
   uint32 REVISION;     /* 0x00 */
   uint32 PLL_STATUS;      /* 0x04 */
   uint32 PLL_CONFIG;      /* 0x08 */
   uint32 PLL_CONTROL1;    /* 0x0c */
   uint32 PLL_CONTROL2;    /* 0x10 */
   uint32 PLL_CONTROL3;    /* 0x14 */
   uint32 PLL_DIVIDER;     /* 0x18 */
   uint32 PLL_PRE_DIVIDER;    /* 0x1c */
   uint32 PLL_SS_EN;    /* 0x20 */
   uint32 PLL_SS_CFG;      /* 0x24 */
   uint32 AUX_CONTROL;     /* 0x28 */
   uint32 IDLE_PAD_CONTROL;   /* 0x2c */
   uint32 IDLE_PAD_EN0;    /* 0x30 */
   uint32 IDLE_PAD_EN1;    /* 0x34 */
   uint32 DRIVE_PAD_CTL;      /* 0x38 */
   uint32 STATIC_PAD_CTL;     /* 0x3c */
   uint32 DRAM_CFG;     /* 0x40 */
   uint32 DRAM_TIMING1;    /* 0x44 */
   uint32 DRAM_TIMING2;    /* 0x48 */
   uint32 DRAM_TIMING3;    /* 0x4c */
   uint32 DRAM_TIMING4;    /* 0x50 */
   uint32 unused0[3];      /* 0x54-0x5f */

   uint32 VDL_REGS[47];    /* 0x60-0x118 */
   uint32 AC_SPARE;     /* 0x11c */
   uint32 unused1[4];      /* 0x120-0x12f */

   uint32 REFRESH;         /* 0x130 */
   uint32 UPDATE_VDL;      /* 0x134 */
   uint32 UPDATE_VDL_SNOOP1;  /* 0x138 */
   uint32 UPDATE_VDL_SNOOP2;  /* 0x13c */
   uint32 CMND_REG1;    /* 0x140 */
   uint32 CMND_AUX_REG1;      /* 0x144 */
   uint32 CMND_REG2;    /* 0x148 */
   uint32 CMND_AUX_REG2;      /* 0x14c */
   uint32 CMND_REG3;    /* 0x150 */
   uint32 CMND_AUX_REG3;      /* 0x154 */
   uint32 CMND_REG4;    /* 0x158 */
   uint32 CMND_AUX_REG4;      /* 0x15c */
   uint32 CMND_REG_TIMER;     /* 0x160 */
   uint32 MODE_REG[9];     /* 0x164-184 */
#define PHY_CONTROL_MODE_REG_VALID_SHIFT       16
#define PHY_CONTROL_MODE_REG_VALID_MASK        (0x1<<PHY_CONTROL_MODE_REG_VALID_SHIFT)
   uint32 MODE_REG15;      /* 0x188 */
   uint32 MODE_REG63;      /* 0x18c */
   uint32 ALERT_CLEAR;     /* 0x190 */
   uint32 ALERT_STATUS;    /* 0x194 */
   uint32 CA_PARITY;    /* 0x198 */
   uint32 CA_PLAYBACK_CTRL;   /* 0x19c */
   uint32 CA_PLAYBACK_STATUS0;   /* 0x1a0 */
   uint32 CA_PLAYBACK_STATUS1;   /* 0x1a4 */
   uint32 unused2;         /* 0x1a8 */
   uint32 WRITE_LEVEL_CTRL;   /* 0x1ac */
   uint32 WRITE_LEVEL_STATUS; /* 0x1b0 */
   uint32 READ_EN_CTRL;    /* 0x1b4 */
   uint32 READ_EN_STATUS;     /* 0x1b8 */
   uint32 unused3;         /* 0x1bc */
   uint32 TRAFFIC_GEN[12];    /* 0x1c0-0x1ec */
   uint32 VIRT_VTT_CTRL;      /* 0x1f0 */
   uint32 VIRT_VTT_STATUS;    /* 0x1f4 */
   uint32 VIRT_VTT_CONNECTION;   /* 0x1f8 */
   uint32 VIRT_VTT_OVERRIDE;  /* 0x1fc */
   uint32 VREF_DAC_CTRL;      /* 0x200 */
   uint32 PHYBIST[9];      /* 0x204-0x224 */
   uint32 unused4[2];      /* 0x228-0x22f */
   uint32 STANDBY_CTRL;    /* 0x230 */
   uint32 DEBUG_FREEZE_EN;    /* 0x234 */
   uint32 DEBUG_MUX_CTRL;     /* 0x238 */
   uint32 DFI_CTRL;     /* 0x23c */
#define PHY_CONTROL_DFI_CNTRL_DFI_CS1_SHIFT    7
#define PHY_CONTROL_DFI_CNTRL_DFI_CS1_MASK     (0x1<<PHY_CONTROL_DFI_CNTRL_DFI_CS1_SHIFT)
#define PHY_CONTROL_DFI_CNTRL_DFI_CS0_SHIFT    6
#define PHY_CONTROL_DFI_CNTRL_DFI_CS0_MASK     (0x1<<PHY_CONTROL_DFI_CNTRL_DFI_CS0_SHIFT)
#define PHY_CONTROL_DFI_CNTRL_DFI_RSTN_SHIFT   5
#define PHY_CONTROL_DFI_CNTRL_DFI_RSTN_MASK    (0x1<<PHY_CONTROL_DFI_CNTRL_DFI_RSTN_SHIFT)
#define PHY_CONTROL_DFI_CNTRL_DFI_CKE1_SHIFT   4
#define PHY_CONTROL_DFI_CNTRL_DFI_CKE1_MASK    (0x1<<PHY_CONTROL_DFI_CNTRL_DFI_CKE1_SHIFT)
#define PHY_CONTROL_DFI_CNTRL_DFI_CKE0_SHIFT   3
#define PHY_CONTROL_DFI_CNTRL_DFI_CKE0_MASK    (0x1<<PHY_CONTROL_DFI_CNTRL_DFI_CKE0_SHIFT)
#define PHY_CONTROL_DFI_CNTRL_ASSERT_REQ_SHIFT 0
#define PHY_CONTROL_DFI_CNTRL_ASSERT_REQ_MASK  (0x1<<PHY_CONTROL_DFI_CNTRL_ASSERT_REQ_SHIFT)
   uint32 WRITE_ODT_CTRL;     /* 0x240 */
   uint32 ABI_PAR_CTRL;    /* 0x244 */
   uint32 ZQ_CAL;       /* 0x248 */
   uint32 unused5[109];     /* 0x24c-0x3ff */
} DDRPhyControl;

typedef struct DDRPhyByteLaneControl {
   uint32 VDL_CTRL_WR[12];    /* 0x00 - 0x2c */
   uint32 VDL_CTRL_RD[24];    /* 0x30 - 0x8c */
   uint32 VDL_CLK_CTRL;    /* 0x90 */
   uint32 VDL_LDE_CTRL;    /* 0x94 */
   uint32 unused0[2];      /* 0x98-0x9f */
   uint32 RD_EN_DLY_CYC;      /* 0xa0 */
   uint32 WR_CHAN_DLY_CYC;    /* 0xa4 */
   uint32 unused1[2];      /* 0xa8-0xaf */
   uint32 RD_CTRL;         /* 0xb0 */
   uint32 RD_FIFO_ADDR;    /* 0xb4 */
   uint32 RD_FIFO_DATA;    /* 0xb8 */
   uint32 RD_FIFO_DM_DBI;     /* 0xbc */
   uint32 RD_FIFO_STATUS;     /* 0xc0 */
   uint32 RD_FIFO_CLR;     /* 0xc4 */
   uint32 IDLE_PAD_CTRL;      /* 0xc8 */
   uint32 DRIVE_PAD_CTRL;     /* 0xcc */
   uint32 RD_EN_DRIVE_PAD_CTRL;  /* 0xd0 */
   uint32 STATIC_PAD_CTRL;    /* 0xd4 */
   uint32 WR_PREAMBLE_MODE;   /* 0xd8 */
   uint32 unused2;         /* 0xdc */
   uint32 ODT_CTRL;     /* 0xe0 */
   uint32 unused3[3];      /* 0xe4-0xef */
   uint32 EDC_DPD_CTRL;    /* 0xf0 */
   uint32 EDC_DPD_STATUS;     /* 0xf4 */
   uint32 EDC_DPD_OUT_CTRL;   /* 0xf8 */
   uint32 EDC_DPD_OUT_STATUS; /* 0xfc */
   uint32 EDC_DPD_OUT_STATUS_CLEAR;/* 0x100 */
   uint32 EDC_CRC_CTRL;    /* 0x104 */
   uint32 EDC_CRC_STATUS;     /* 0x108 */
   uint32 EDC_CRC_COUNT;      /* 0x10c */
   uint32 EDC_CRC_STATUS_CLEAR;  /* 0x110 */
   uint32 BL_SPARE_REG;    /* 0x114 */
   uint32 unused4[58];
} DDRPhyByteLaneControl;

/*
 * I2C Controller.
 */

typedef struct I2CControl {
  uint32        ChipAddress;            /* 0x0 */
#define I2C_CHIP_ADDRESS_MASK           0x000000f7
#define I2C_CHIP_ADDRESS_SHIFT          0x1
  uint32        DataIn0;                /* 0x4 */
  uint32        DataIn1;                /* 0x8 */
  uint32        DataIn2;                /* 0xc */

  uint32        DataIn3;                /* 0x10 */
  uint32        DataIn4;                /* 0x14 */
  uint32        DataIn5;                /* 0x18 */
  uint32        DataIn6;                /* 0x1c */

  uint32        DataIn7;                /* 0x20 */
  uint32        CntReg;                 /* 0x24 */
#define I2C_CNT_REG1_SHIFT              0x0
#define I2C_CNT_REG2_SHIFT              0x6
  uint32        CtlReg;                 /* 0x28 */
#define I2C_CTL_REG_DTF_MASK            0x00000003
#define I2C_CTL_REG_DTF_WRITE           0x0
#define I2C_CTL_REG_DTF_READ            0x1
#define I2C_CTL_REG_DTF_READ_AND_WRITE  0x2
#define I2C_CTL_REG_DTF_WRITE_AND_READ  0x3
#define I2C_CTL_REG_DEGLITCH_DISABLE    0x00000004
#define I2C_CTL_REG_DELAY_DISABLE       0x00000008
#define I2C_CTL_REG_SCL_SEL_MASK        0x00000030
#define I2C_CTL_REG_SCL_CLK_375KHZ      0x00000000
#define I2C_CTL_REG_SCL_CLK_390KHZ      0x00000010
#define I2C_CTL_REG_SCL_CLK_187_5KHZ    0x00000020
#define I2C_CTL_REG_SCL_CLK_200KHZ      0x00000030
#define I2C_CTL_REG_INT_ENABLE          0x00000040
#define I2C_CTL_REG_DIV_CLK             0x00000080
  uint32        IICEnable;              /* 0x2c */

#define I2C_IIC_ENABLE                  0x00000001
#define I2C_IIC_INTRP                   0x00000002
#define I2C_IIC_NO_ACK                  0x00000004
#define I2C_IIC_NO_STOP                 0x00000010
#define I2C_IIC_NO_START                0x00000020
  uint32        DataOut0;               /* 0x30 */
  uint32        DataOut1;               /* 0x34 */
  uint32        DataOut2;               /* 0x38 */
  uint32        DataOut3;               /* 0x3c */

  uint32        DataOut4;               /* 0x40 */
  uint32        DataOut5;               /* 0x44 */
  uint32        DataOut6;               /* 0x48 */
  uint32        DataOut7;               /* 0x4c */

  uint32        CtlHiReg;               /* 0x50 */
#define I2C_CTLHI_REG_WAIT_DISABLE      0x00000001
#define I2C_CTLHI_REG_IGNORE_ACK        0x00000002
#define I2C_CTLHI_REG_DATA_REG_SIZE     0x00000040
  uint32        SclParam;               /* 0x54 */
} I2CControl;

#define I2C ((volatile I2CControl * const) BCM_IO_ADDR(I2C_BASE))

typedef struct MEMCControl {
   uint32 GLB_VERS;     /* 0x000 */
   uint32 GLB_GCFG;     /* 0x004 */
#define MEMC_GLB_GCFG_DRAM_EN_SHIFT        31
#define MEMC_GLB_GCFG_DRAM_EN_MASK         (0x1<<MEMC_GLB_GCFG_DRAM_EN_SHIFT)
#define MEMC_GLB_GCFG_PHY_4X_SHIFT         24
#define MEMC_GLB_GCFG_PHY_4X_MASK          (0x1<<MEMC_GLB_GCFG_PHY_4X_SHIFT)
#define MEMC_GLB_GCFG_MCLKSRC_SHIFT        9
#define MEMC_GLB_GCFG_MCLKSRC_MASK         (0x1<<MEMC_GLB_GCFG_MCLKSRC_SHIFT)
#define MEMC_GLB_GCFG_MEMINITDONE_SHIFT    8
#define MEMC_GLB_GCFG_MEMINITDONE_MASK     (0x1<<MEMC_GLB_GCFG_MEMINITDONE_SHIFT)
#define MEMC_GLB_GCFG_SIZE1_SHIFT          0
#define MEMC_GLB_GCFG_SIZE1_MASK           (0xf<<MEMC_GLB_GCFG_SIZE1_SHIFT)

   uint32 unused0[2];      /* 0x008-0x00f */  
   uint32 GLB_CFG;         /* 0x010 */
#define MEMC_GLB_CFG_RR_MODE_SHIFT      0x0
#define MEMC_GLB_CFG_RR_MODE_MASK       (0x1<<MEMC_GLB_CFG_RR_MODE_SHIFT)
#define MEMC_GLB_CFG_BURST_MODE_SHIFT   0x1
#define MEMC_GLB_CFG_BURST_MODE_MASK    (0x1<<MEMC_GLB_CFG_BURST_MODE_SHIFT)
   uint32 GLB_QUE_DIS;     /* 0x014 */
   uint32 GLB_SP_SEL;      /* 0x018 */
#define MEMC_GLB_SP_SEL_SELECT_MASK     0x001fffff
   uint32 GLB_SP_PRI_0;    /* 0x01c */
   uint32 GLB_SP_PRI_1;    /* 0x020 */
   uint32 GLB_SP_PRI_2;    /* 0x024 */
   uint32 GLB_SP_PRI_3;    /* 0x028 */
   uint32 GLB_SP_PRI_4;    /* 0x02c */
   uint32 GLB_SP_PRI_5;    /* 0x030 */
   uint32 unused1[3];      /* 0x034-0x3f */
   uint32 GLB_RR_QUANTUM[12]; /* 0x040-0x06c */
   uint32 unused2[4];      /* 0x070-0x07f */

   uint32 INTR2_CPU_STATUS;   /* 0x080 */
   uint32 INTR2_CPU_SET;      /* 0x084 */
   uint32 INTR2_CPU_CLEAR;    /* 0x088 */
   uint32 INTR2_CPU_MASK_STATUS; /* 0x08c */
   uint32 INTR2_CPU_MASK_SET; /* 0x090 */
   uint32 INTR2_CPU_MASK_CLEAR;  /* 0x094 */
   uint32 unused3[10];     /* 0x098-0x0bf */

   uint32 SRAM_REMAP_CTRL;    /* 0x0c0 */
   uint32 SRAM_REMAP_INIT;    /* 0x0c4 */
   uint32 SRAM_REMAP_LOG_INFO_0; /* 0x0c8 */
   uint32 SRAM_REMAP_LOG_INFO_1; /* 0x0cc */
   uint32 unused4[12];     /* 0x0d0-0x0ff */

   uint32 CHN_CFG_CNFG;    /* 0x100 */
   uint32 CHN_CFG_CSST;    /* 0x104 */
   uint32 CHN_CFG_CSEND;      /* 0x108 */
   uint32 unused5;         /* 0x10c */
   uint32 CHN_CFG_ROW00_0;    /* 0x110 */
   uint32 CHN_CFG_ROW00_1;    /* 0x114 */
   uint32 CHN_CFG_ROW01_0;    /* 0x118 */
   uint32 CHN_CFG_ROW01_1;    /* 0x11c */
   uint32 unused6[4];      /* 0x120-0x12f */
   uint32 CHN_CFG_ROW20_0;    /* 0x130 */
   uint32 CHN_CFG_ROW20_1;    /* 0x134 */
   uint32 CHN_CFG_ROW21_0;    /* 0x138 */
   uint32 CHN_CFG_ROW21_1;    /* 0x13c */
   uint32 unused7[4];      /* 0x140-0x14f */
   uint32 CHN_CFG_COL00_0;    /* 0x150 */
   uint32 CHN_CFG_COL00_1;    /* 0x154 */
   uint32 CHN_CFG_COL01_0;    /* 0x158 */
   uint32 CHN_CFG_COL01_1;    /* 0x15c */
   uint32 unused8[4];      /* 0x160-0x16f */
   uint32 CHN_CFG_COL20_0;    /* 0x170 */
   uint32 CHN_CFG_COL20_1;    /* 0x174 */
   uint32 CHN_CFG_COL21_0;    /* 0x178 */
   uint32 CHN_CFG_COL21_1;    /* 0x17c */
   uint32 unused9[4];      /* 0x180-0x18f */
   uint32 CHN_CFG_BNK10;      /* 0x190 */
   uint32 CHN_CFG_BNK32;      /* 0x194 */
   uint32 unused10[26];    /* 0x198-0x1ff */

   uint32 CHN_TIM_DCMD;    /* 0x200 */
   uint32 CHN_TIM_DMODE_0;    /* 0x204 */
   uint32 CHN_TIM_DMODE_2;    /* 0x208 */
   uint32 CHN_TIM_CLKS;    /* 0x20c */
   uint32 CHN_TIM_ODT;     /* 0x210 */
   uint32 CHN_TIM_TIM1_0;     /* 0x214 */
   uint32 CHN_TIM_TIM1_1;     /* 0x218 */
   uint32 CHN_TIM_TIM2;    /* 0x21c */
   uint32 CHN_TIM_CTL_CRC;    /* 0x220 */
   uint32 CHN_TIM_DOUT_CRC;   /* 0x224 */
   uint32 CHN_TIM_DIN_CRC;    /* 0x228 */
   uint32 CHN_TIM_CRC_CTRL;   /* 0x22c */
   uint32 CHN_TIM_PHY_ST;     /* 0x230 */
#define MEMC_CHN_TIM_PHY_ST_PHY_ST_POWER_UP       0x1
#define MEMC_CHN_TIM_PHY_ST_PHY_ST_HW_RESET       0x2
#define MEMC_CHN_TIM_PHY_ST_PHY_ST_SW_RESET       0x4
#define MEMC_CHN_TIM_PHY_ST_PHY_ST_READY          0x10
   uint32 CHN_TIM_DRAM_CFG;   /* 0x234 */
#define DRAM_CFG_DRAMSLEEP          (1<<11)
#define MEMC_CHN_TIM_DRAM_CFG_DRAM_CFG_DRAMTYPE_SHIFT 0x0
#define MEMC_CHN_TIM_DRAM_CFG_DRAM_CFG_DRAMTYPE_MASK  (0xf<<MEMC_CHN_TIM_DRAM_CFG_DRAM_CFG_DRAMTYPE_SHIFT)
   uint32 CHN_TIM_STAT;    /* 0x238 */
   uint32 unused11[49];    /* 0x23c-0x2ff */

   UBUSInterface UBUSIF0;     /* 0x300-0x33f */
   UBUSInterface UBUSIF1;     /* 0x340-0x37f */

   uint32 AXIRIF_0_CFG;    /* 0x380 */
   uint32 AXIRIF_0_REP_ARB_MODE; /* 0x384 */
#define MEMC_AXIRIF_0_REP_ARB_MODE_FIFO_MODE_SHIFT    0x0
#define MEMC_AXIRIF_0_REP_ARB_MODE_FIFO_MODE_MASK     (0x3<<MEMC_AXIRIF_0_REP_ARB_MODE_FIFO_MODE_SHIFT)
   uint32 AXIRIF_0_SCRATCH;   /* 0x388 */
   uint32 unused12[29];    /* 0x38c-0x3ff */

   uint32 AXIWIF_0_CFG;    /* 0x400 */
#define MEMC_AXIWIF_0_CFG_WRITE_REPLY_MODE_SHIFT      0x0
#define MEMC_AXIWIF_0_CFG_WRITE_REPLY_MODE_MASK       (0x1<<MEMC_AXIWIF_0_CFG_WRITE_REPLY_MODE_SHIFT)
#define MEMC_AXIWIF_0_CFG_WRITE_BURST_MODE_SHIFT      0x1
#define MEMC_AXIWIF_0_CFG_WRITE_BURST_MODE_MASK       (0x1<<MEMC_AXIWIF_0_CFG_WRITE_BURST_MODE_SHIFT)
   uint32 AXIWIF_0_REP_ARB_MODE; /* 0x404 */
#define MEMC_AXIWIF_0_REP_ARB_MODE_FIFO_MODE_SHIFT    0x0
#define MEMC_AXIWIF_0_REP_ARB_MODE_FIFO_MODE_MASK     (0x1<<MEMC_AXIRIF_0_REP_ARB_MODE_FIFO_MODE_SHIFT)

   uint32 AXIWIF_0_SCRATCH;   /* 0x408 */
   uint32 unused13[61];    /* 0x40c-0x4ff */

   EDISEngine EDIS_0;      /* 0x500 */
   EDISEngine EDIS_1;      /* 0x600 */

   uint32 STATS_CTRL;      /* 0x700 */
   uint32 STATS_TIMER_CFG;    /* 0x704 */
   uint32 STATS_TIMER_COUNT;  /* 0x708 */
   uint32 STATS_TOTAL_SLICE;  /* 0x70c */
   uint32 STATS_TOTAL_PACKET; /* 0x710 */
   uint32 STATS_SLICE_REORDER;   /* 0x714 */
   uint32 STATS_IDLE_DDR_CYCLE;  /* 0x718 */
   uint32 STATS_ARB_GRANT;    /* 0x71c */
   uint32 STATS_PROG_0;    /* 0x720 */
   uint32 STATS_PROG_1;    /* 0x724 */
   uint32 STATS_ARB_GRANT_MATCH; /* 0x728 */
   uint32 STATS_CFG_0;     /* 0x72c */
   uint32 STATS_CFG_1;     /* 0x730 */
   uint32 unused14[19];    /* 0x734-0x77f */

   uint32 CAP_CAPTURE_CFG;    /* 0x780 */
   uint32 CAP_TRIGGER_ADDR;   /* 0x784 */
   uint32 CAP_READ_CTRL;      /* 0x788 */
   uint32 unused15;     /* 0x78c */
   uint32 CAP_CAPTURE_MATCH0; /* 0x790 */
   uint32 CAP_CAPTURE_MATCH1; /* 0x794 */
   uint32 CAP_CAPTURE_MATCH2; /* 0x798 */
   uint32 unused16;     /* 0x79c */
   uint32 CAP_CAPTURE_MASK0;  /* 0x7a0 */
   uint32 CAP_CAPTURE_MASK1;  /* 0x7a4 */
   uint32 CAP_CAPTURE_MASK2;  /* 0x7a8 */
   uint32 unused17;     /* 0x7ac */
   uint32 CAP_TRIGGER_MATCH0; /* 0x7b0 */
   uint32 CAP_TRIGGER_MATCH1; /* 0x7b4 */
   uint32 CAP_TRIGGER_MATCH2; /* 0x7b8 */
   uint32 unused18;     /* 0x7bc */
   uint32 CAP_TRIGGER_MASK0;  /* 0x7c0 */
   uint32 CAP_TRIGGER_MASK1;  /* 0x7c4 */
   uint32 CAP_TRIGGER_MASK2;  /* 0x7c8 */
   uint32 unused19;     /* 0x7cc */
   uint32 CAP_READ_DATA[4];   /* 0x7d0-0x7dc */
   uint32 unused20[8];     /* 0x7e0-0x7ff */

   SecureRangeCheckers SEC_RANGE_CHK;  /* 0x800 */

   uint32 SEC_INTR2_CPU_STATUS;  /* 0x900 */
   uint32 SEC_INTR2_CPU_SET;  /* 0x904 */
   uint32 SEC_INTR2_CPU_CLEAR;   /* 0x908 */
   uint32 SEC_INTR2_CPU_MASK_STATUS;   /* 0x90c */
   uint32 SEC_INTR2_CPU_MASK_SET;   /* 0x910 */
   uint32 SEC_INTR2_CPU_MASK_CLEAR; /* 0x914 */
   uint32 unused21[10];    /* 0x918-0x93f */

   uint32 SEC_SRAM_REMAP_CTRL;   /* 0x940 */
   uint32 SEC_SRAM_REMAP_INIT;   /* 0x944 */
   uint32 SEC_SRAM_REMAP_LOG_0;  /* 0x948 */
   uint32 SEC_SRAM_REMAP_LOG_1;  /* 0x94c */
   uint32 unused22[428];      /* 0x950-0xfff */

   DDRPhyControl PhyControl;  /* 0x1000 */
   DDRPhyByteLaneControl PhyByteLane0Control;   /* 0x1400 */
   DDRPhyByteLaneControl PhyByteLane1Control;   /* 0x1600 */
} MEMCControl;

#define MEMC ((volatile MEMCControl * const) MEMC_BASE)


/*
 * Peripheral Controller
 */
typedef struct PerfControl {
   uint32 RevID;     /* 0x00 */
#define CHIP_ID_SHIFT   12
#define CHIP_ID_MASK (0xfffff << CHIP_ID_SHIFT)
#define REV_ID_MASK  0xff

   uint32 DiagCtrl;  /* 0x04 */
#define DIAG_SW_EN   (1 << 23)
#define DIAG_CTRL_MASK  (0x001fffff)

#if 0 /* FIXME! do the following reflect DiagCtrl register? */
   uint32 blkEnables;   /* (04) word 1 */
#define ROBOSW250_CLK_EN   (1 << 31)
#define TBUS_CLK_EN     (1 << 27)
#define NAND_CLK_EN     (1 << 20)
//#define SECMIPS_CLK_EN   (1 << 19)
#define GMAC_CLK_EN     (1 << 19)
#define PHYMIPS_CLK_EN     (1 << 18)
#define PCIE_CLK_EN     (1 << 17)
#define HS_SPI_CLK_EN      (1 << 16)
#define SPI_CLK_EN      (1 << 15)
#define IPSEC_CLK_EN    (1 << 14)
#define USBH_CLK_EN     (1 << 13)
#define USBD_CLK_EN     (1 << 12)
#define APM_CLK_EN      (1 << 11)
#define ROBOSW_CLK_EN      (1 << 10)
#define SAR_CLK_EN      (1 << 9)
#define FAP1_CLK_EN     (1 << 8)
#define FAP0_CLK_EN     (1 << 7)
#define DECT_CLK_EN     (1 << 6)
#define WLAN_OCP_CLK_EN    (1 << 5)
#define MIPS_CLK_EN     (1 << 4)
#define VDSL_CLK_EN     (1 << 3)
#define VDSL_AFE_EN     (1 << 2)
#define VDSL_QPROC_EN      (1 << 1)
#define DISABLE_GLESS      (1 << 0)
#endif

   uint32 ExtIrqCtrl;   /* 0x08 */
#define EI_CLEAR_SHFT   0
#define EI_SENSE_SHFT   8
#define EI_INSENS_SHFT  16
#define EI_LEVEL_SHFT   24
   uint32 ExtIrqStatus; /* 0x0c */
#define EI_MASK_SHFT 16
#define EI_STATUS_SHFT 0
   uint32 IrqStatus[4]; /* 0x10 - 0x1c */

   uint32 IntMaskARM0[4];  /* 0x20 */
   uint32 IntMaskARM1[4];  /* 0x30 */
   uint32 IntMaskPCIE[4];  /* 0x40 */
   uint32 IntMaskPMC[4];   /* 0x50 */
} PerfControl;

#define PERF ((volatile PerfControl * const) PERF_BASE)


/*
 * Timer
 */
typedef struct Timer {
   uint32 TimerCtl0; /* 0x00 */
   uint32 TimerCtl1; /* 0x04 */
   uint32 TimerCtl2; /* 0x08 */
   uint32 TimerCtl3; /* 0x0c */
#define TIMERENABLE     (1 << 31)
#define RSTCNTCLR    (1 << 30)

   uint32 TimerCnt0; /* 0x10 */
   uint32 TimerCnt1; /* 0x14 */
   uint32 TimerCnt2; /* 0x18 */
   uint32 TimerCnt3; /* 0x1c */
#define TIMER_COUNT_MASK   0x3FFFFFFF

   uint32 TimerMask; /* 0x20 */
#define TIMER0EN     (1 << 0)
#define TIMER1EN     (1 << 1)
#define TIMER2EN     (1 << 2)
#define TIMER3EN     (1 << 3)

   uint32 TimerInts; /* 0x24 */
#define TIMER0       (1 << 0)
#define TIMER1       (1 << 1)
#define TIMER2       (1 << 2)
#define TIMER3       (1 << 3)
#define WATCHDOG     (1 << 4)

   uint32 WatchDogDefCount;   /* 0x28 */

   /* Write 0xff00 0x00ff to Start timer
    * Write 0xee00 0x00ee to Stop and re-load default count
    * Read from this register returns current watch dog count
    */
   uint32 WatchDogCtl;  /* 0x2c */

   /* Number of 50-MHz ticks for WD Reset pulse to last */
   uint32 WDResetCount; /* 0x30 */
   uint32 SoftRst;           /* 0x34 */
#define SOFT_RESET              (1 << 0)
   uint32 ResetStatus;   /* 0x38 */
#define PCIE_RESET_STATUS       0x10000000
#define SW_RESET_STATUS         0x20000000
#define HW_RESET_STATUS         0x40000000
#define POR_RESET_STATUS        0x80000000
#define RESET_STATUS_MASK       0xF0000000    
} Timer;

#define TIMER ((volatile Timer * const) TIMR_BASE)


/*
 * Gpio Controller
 */
typedef struct GpioControl {
   uint32 GPIODir[5];   /* 0x00-0x10 */
   uint32 GPIOio[5]; /* 0x14-0x24 */
   uint32 SpiSlaveCfg;  /* 0x28 */
   uint32 unused;    /* 0x2c */
   uint32 TestControl;     /* 0x30 */
   uint32 TestPortBlockEnMSB; /* 0x34 */
   uint32 TestPortBlockEnLSB; /* 0x38 */
   uint32 TestPortBlockDataMSB;  /* 0x3c */
   uint32 TestPortBlockDataLSB;  /* 0x40 */
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
   uint32 TestPortCmd;     /* 0x44 */
#define LOAD_MUX_REG_CMD        0x21
   uint32 GPIOBaseMode;    /* 0x48 */
#define GPIO_BASE_USB_LED_OVERRIDE  (1<<11)
#define GPIO_BASE_VDSL_LED_OVERRIDE (1<<10)
   uint32 DiagReadBack;    /* 0x4c */
   uint32 DiagReadBackHi;     /* 0x50 */
   uint32 GeneralPurpose;     /* 0x54 */
} GpioControl;

#define GPIO ((volatile GpioControl * const) GPIO_BASE)

/* Number to mask conversion macro used for GPIODir and GPIOio */
#define GPIO_NUM_MAX               160   // FIXME!!
#define GPIO_NUM_TO_ARRAY_IDX(X)   ((((X) & BP_GPIO_NUM_MASK) < GPIO_NUM_MAX) ? ((((X) & BP_GPIO_NUM_MASK) >> 5) & 0x07) : (0))
#define GPIO_NUM_TO_MASK(X)        ((((X) & BP_GPIO_NUM_MASK) < GPIO_NUM_MAX) ? (1 << (((X) & BP_GPIO_NUM_MASK) & 0x1f)) : (0))
#define GPIO_NUM_TO_ARRAY_SHIFT(X) (((X) & BP_GPIO_NUM_MASK) & 0x1f)

/*
 * Misc Register Set Definitions.
 */
typedef struct Misc {
#define MISC_PCIE_CTRL_CORE_SOFT_RESET_MASK     (0x3)
   uint32 miscPCIECtrl; /* 0x00 */
   uint32 miscStrapBus; /* 0x04 */
#define MISC_STRAP_BUS_PMC_AVS_OVERRIDE_MARKER  (1 << 30)
#define MISC_STRAP_BUS_SW_RESERVE_1             (0x3 << 24)
#define MISC_STRAP_BUS_SW_BOOT_SPI_SPINAND_EMMC_MASK (0x2 << 24) /* Bit 25 = 0 => NAND BOOT */
#define MISC_STRAP_BUS_SW_BOOT_NORMAL_MASK      (0x1 << 24) /* Bit 24 = 0 => Bootrom boot */
#define MISC_STRAP_BUS_BISR_MEM_REPAIR          (1 << 23)
#define MISC_STRAP_BUS_RESET_OUT_DELAY_SHIFT    22
#define MISC_STRAP_BUS_RESET_OUT_DELAY_MASK     (1 << MISC_STRAP_BUS_RESET_OUT_DELAY_SHIFT)
#define MISC_STRAP_BUS_RESET_OUT_DELAY_100MS    (1 << MISC_STRAP_BUS_RESET_OUT_DELAY_SHIFT)
#define MISC_STRAP_BUS_RESET_OUT_DELAY_50MS     (0x0 << MISC_STRAP_BUS_RESET_OUT_DELAY_SHIFT)
#define MISC_STRAP_BUS_SYS_BUS_FREQ             (0x3 << 20)
#define MISC_STRAP_BUS_A9_CORE0_BOOT            (1 << 19)
#define MISC_STRAP_BUS_PMC_BOOT_FLASH_N         (1 << 18)
#define MISC_STRAP_BUS_PMC_BOOT_AVS             (1 << 17)
#define MISC_STRAP_BUS_HS_SPIM_24B_N_32B_ADDR   (1 << 16)
#define MISC_STRAP_BUS_HS_SPIM_CLK_SLOW_N_FAST  (1 << 15)
#define MISC_STRAP_BUS_SW_RESERVE_0             (0x7 << 12)
#define MISC_STRAP_BUS_DISABLE_NAND_ECC         (1 << 11)
#define MISC_STRAP_BUS_PMC_ROM_BOOT_SHIFT       10
#define MISC_STRAP_BUS_PMC_ROM_BOOT             (1<<MISC_STRAP_BUS_PMC_ROM_BOOT_SHIFT) /* pmc rom boot enable */
#define MISC_STRAP_BUS_PICO_ROM_BOOT            (1 << 9)
#define MISC_STRAP_BUS_BOOT_SEL_SHIFT           4
#define MISC_STRAP_BUS_BOOT_OPT_MASK            (0x18 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_SEL_MASK            (0x1f << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_SPI_NAND            (0x08 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_SPI_NOR             (0x18 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_SPI_NOR_1_24MHZ     (0x0 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_SPI_NOR_2_54MHZ     (0x1 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_SPI_NOR_2_81MHZ     (0x2 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_SPI_NOR_4_81MHZ     (0x3 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_EMMC                (0x10 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_NAND_2K_PAGE        (0x00 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_NAND_4K_PAGE        (0x08 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_NAND_8K_PAGE        (0x10 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_SEL_ECC_MASK        (0x7 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_NAND_ECC_DISABLE	(0x0 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_NAND_ECC_1_BIT      (0x1 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_NAND_ECC_4_BIT      (0x2 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_NAND_ECC_8_BIT      (0x3 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_NAND_ECC_12_BIT     (0x4 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_NAND_ECC_24_BIT     (0x5 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_NAND_ECC_40_BIT     (0x6 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_NAND_ECC_60_BIT     (0x7 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_LS_SPI_SLAVE_DISABLE     (1 << 3)
#define MISC_STRAP_BUS_PCIE1_RC_MODE            (1 << 2)
#define MISC_STRAP_BUS_PCIE0_RC_MODE            (1 << 1)
#define MISC_STRAP_BUS_TBUS_DISABLE             (1 << 0)

   uint32 miscStrapOverride;     /* 0x08 */
   uint32 miscAdsl_clock_sample; /* 0x0c */
   uint32 miscRngCtrl;           /* 0x10 */
   uint32 miscMbox0_data;        /* 0x14 */
   uint32 miscMbox1_data;        /* 0x18 */
   uint32 miscMbox2_data;        /* 0x1c */
   uint32 miscMbox3_data;        /* 0x20 */
   uint32 miscMbox_ctrl;         /* 0x24 */
   uint32 miscxMIIPadCtrl[4];    /* 0x28 */
#define MISC_XMII_PAD_MODEHV                    (1 << 6)
#define MISC_XMII_PAD_SEL_GMII                  (1 << 4)
#define MISC_XMII_PAD_AMP_EN                    (1 << 3)
   uint32 miscxMIIPullCtrl[4];      /* 0x38 */
   uint32 miscRBUSBridgeCtrl; /* 0x48 */
    uint32 miscSGMIIFiberDetect;    /* 0x4c */
#define MISC_SGMII_FIBER_GPIO36     (1<<0)
} Misc;

#define MISC ((volatile Misc * const) MISC_BASE)


typedef struct Rng {
   uint32 ctrl0;                 /* 0x00 */
   uint32 rngSoftReset;          /* 0x04 */
   uint32 rbgSoftReset;          /* 0x08 */
   uint32 totalBitCnt;           /* 0x0c */
   uint32 totalBitCntThreshold;  /* 0x10 */
   uint32 revId;                 /* 0x14 */
   uint32 intStatus;             /* 0x18 */
#define RNG_INT_STATUS_NIST_FAIL       (0x1<<5)
#define RNG_INT_STATUS_FIFO_FULL       (0x1<<2)
   uint32 intEn;                 /* 0x1c */
   uint32 rngFifoData;           /* 0x20 */
   uint32 fifoCnt;               /* 0x24 */
#define RNG_PERM_ALLOW_SECURE_ACCESS           0xCC
#define RNG_PERM_ALLOW_NONSEC_ACCESS           0x33
   uint32 perm;                  /* 0x28 */
} Rng;

#define RNG ((volatile Rng * const) RNG_BASE)

/*
 * UART Peripheral
 */
typedef struct UartChannel {
   byte fifoctl;  /* 0x00 */
#define RSTTXFIFOS   0x80
#define RSTRXFIFOS   0x40
   /* 5-bit TimeoutCnt is in low bits of this register.
    *  This count represents the number of characters
    *  idle times before setting receive Irq when below threshold
    */
   byte config;
#define XMITBREAK 0x40
#define BITS5SYM  0x00
#define BITS6SYM  0x10
#define BITS7SYM  0x20
#define BITS8SYM  0x30
#define ONESTOP      0x07
#define TWOSTOP      0x0f
   /* 4-LSBS represent STOP bits/char
    * in 1/8 bit-time intervals.  Zero
    * represents 1/8 stop bit interval.
    * Fifteen represents 2 stop bits.
    */
   byte control;
#define BRGEN     0x80  /* Control register bit defs */
#define TXEN      0x40
#define RXEN      0x20
#define LOOPBK    0x10
#define TXPARITYEN   0x08
#define TXPARITYEVEN 0x04
#define RXPARITYEN   0x02
#define RXPARITYEVEN 0x01
   byte unused0;
          
   uint32 baudword;  /* 0x04 */
   /* When divide SysClk/2/(1+baudword) we should get 32*bit-rate
    */

   /* 0x08 */
   byte  prog_out;  /* Set value of DTR (Bit0), RTS (Bit1)
                *  if these bits are also enabled to
                *  GPIO_o
                */
#define ARMUARTEN 0x04
#define RTSEN     0x02
#define DTREN     0x01
   byte fifocfg;  /* Upper 4-bits are TxThresh, Lower are
                * RxThreshold.  Irq can be asserted
                * when rx fifo> thresh, txfifo<thresh
                */

   byte rxf_levl;  /* Read-only fifo depth */
   byte txf_levl;  /* Read-only fifo depth */

   /* 0x0c */
   byte DeltaIP_SyncIP;  /* Upper 4 bits show which bits
                   *  have changed (may set IRQ).
                   *  read automatically clears
                   *  bit.
                   *  Lower 4 bits are actual
                   *  status
                   */
   byte DeltaIPConfig_Mask; /* Upper 4 bits: 1 for posedge
                   * sense 0 for negedge sense if
                   * not configured for edge
                   * insensitive (see above)
                   * Lower 4 bits: Mask to enable
                   * change detection IRQ for
                   * corresponding GPIO_i
                   */
   byte DeltaIPEdgeNoSense; /* Low 4-bits, set corr bit to
                   * 1 to detect irq on rising
                   * AND falling edges for
                   * corresponding GPIO_i
                   * if enabled (edge insensitive)
                   */
   byte unused1;

   uint16 intStatus; /* 0x10 */
#define DELTAIP         0x0001
#define TXUNDERR        0x0002
#define TXOVFERR        0x0004
#define TXFIFOTHOLD     0x0008
#define TXREADLATCH     0x0010
#define TXFIFOEMT       0x0020
#define RXUNDERR        0x0040
#define RXOVFERR        0x0080
#define RXTIMEOUT       0x0100
#define RXFIFOFULL      0x0200
#define RXFIFOTHOLD     0x0400
#define RXFIFONE        0x0800
#define RXFRAMERR       0x1000
#define RXPARERR        0x2000
#define RXBRK           0x4000
   uint16 intMask;

   uint16 Data;    /* 0x14  Write to TX, Read from RX */
                        /* bits 10:8 are BRK,PAR,FRM errors */
   uint16 unused2;
} Uart;
#define UART ((volatile Uart * const) UART_BASE)
#define UART1 ((volatile Uart *const) UART1_BASE)


/*
 * UART ARM
 */

typedef struct UartArm {
   uint32 data;         /* 0x00 */
   uint32 status;       /* 0x04 */
   uint32 rsvd[4];      /* 0x08 */
   uint32 flags;        /* 0x18 */
   uint32 rsvd2;        /* 0x1c */
   uint32 irLowPower;   /* 0x20 */
   uint32 intBaud;      /* 0x24 */
   uint32 fracBaud;     /* 0x28 */
   uint32 lineCtrl;     /* 0x2c */
   uint32 ctrl;         /* 0x30 */
   uint32 fifoLevel;    /* 0x34 */
   uint32 intrMask;     /* 0x38 */
   uint32 intrRawSts;   /* 0x3c */
   uint32 intrMaskSts;  /* 0x40 */
   uint32 intrClear;    /* 0x44 */
} UartArm;

/*
 * ARM CFG
 */
typedef struct ArmProcClkMgr {
   uint32 wr_access;    /* 0x00 */
#define ARM_PROC_CLK_WR_ACCESS_PASSWORD_SHIFT         8
#define ARM_PROC_CLK_WR_ACCESS_PASSWORD_MASK          (0xffff<<ARM_PROC_CLK_WR_ACCESS_PASSWORD_SHIFT)
#define ARM_PROC_CLK_WR_ACCESS_PASSWORD               0xA5A5
#define ARM_PROC_CLK_WR_ACCESS_CLKMGR_ACC_SHIFT       0
#define ARM_PROC_CLK_WR_ACCESS_CLKMGR_ACC_MASK        (0x1<<ARM_PROC_CLK_WR_ACCESS_CLKMGR_ACC_SHIFT)
#define ARM_PROC_CLK_WR_ACCESS_CLKMGR_ACC             0x1
   uint32 unused0;
   uint32 policy_freq;
#define ARM_PROC_CLK_POLICY3_FREQ_SHIFT               24
#define ARM_PROC_CLK_POLICY3_FREQ_MASK                (0x7<<ARM_PROC_CLK_POLICY3_FREQ_SHIFT)
#define ARM_PROC_CLK_POLICY2_FREQ_SHIFT               16
#define ARM_PROC_CLK_POLICY2_FREQ_MASK                (0x7<<ARM_PROC_CLK_POLICY2_FREQ_SHIFT)
#define ARM_PROC_CLK_POLICY1_FREQ_SHIFT               8
#define ARM_PROC_CLK_POLICY1_FREQ_MASK                (0x7<<ARM_PROC_CLK_POLICY1_FREQ_SHIFT)
#define ARM_PROC_CLK_POLICY0_FREQ_SHIFT               0
#define ARM_PROC_CLK_POLICY0_FREQ_MASK                (0x7<<ARM_PROC_CLK_POLICY0_FREQ_SHIFT)
#define ARM_PROC_CLK_POLICY_FREQ_MASK                 (ARM_PROC_CLK_POLICY0_FREQ_MASK|ARM_PROC_CLK_POLICY1_FREQ_MASK|ARM_PROC_CLK_POLICY2_FREQ_MASK|ARM_PROC_CLK_POLICY3_FREQ_MASK)
#define ARM_PROC_CLK_POLICY_FREQ_ALL(x)	              ((x << ARM_PROC_CLK_POLICY3_FREQ_SHIFT) | (x << ARM_PROC_CLK_POLICY2_FREQ_SHIFT) | \
                                                       (x << ARM_PROC_CLK_POLICY1_FREQ_SHIFT) | (x << ARM_PROC_CLK_POLICY0_FREQ_SHIFT))
#define ARM_PROC_CLK_POLICY_FREQ_CRYSTAL              0
#define ARM_PROC_CLK_POLICY_FREQ_SYSCLK               2
#define ARM_PROC_CLK_POLICY_FREQ_ARMPLL_SLOW          6
#define ARM_PROC_CLK_POLICY_FREQ_ARMPLL_FAST          7
   uint32 policy_ctl;
#define ARM_PROC_CLK_POLICY_CTL_GO_ATL                0x4
#define ARM_PROC_CLK_POLICY_CTL_GO_AC                 0x2
#define ARM_PROC_CLK_POLICY_CTL_GO                    0x1
   uint32 policy0_mask;    /* 0x10 */
   uint32 policy1_mask;
   uint32 policy2_mask;
   uint32 policy3_mask;
   uint32 int_en;       /* 0x20 */
   uint32 int_stat;
   uint32 unused1[3];
   uint32 lvm_en;       /* 0x34 */
   uint32 lvm0_3;
   uint32 lvm4_7;
   uint32 vlt0_3;       /* 0x40 */
   uint32 vlt4_7;
   uint32 unused2[46];
   uint32 bus_quiesc;      /* 0x100 */
   uint32 unused3[63];
   uint32 core0_clkgate;      /* 0x200 */
#define ARM_PROC_CLK_CORE0_CLKGATE_HYST_EN_SHIFT     9
#define ARM_PROC_CLK_CORE0_CLKGATE_HYST_EN_MASK      (0x1<<ARM_PROC_CLK_CORE0_CLKGATE_HYST_EN_SHIFT)
#define ARM_PROC_CLK_CORE0_CLKGATE_HYST_EN           1
#define ARM_PROC_CLK_CORE0_CLKGATE_HYST_DIS          0
#define ARM_PROC_CLK_CORE0_CLKGATE_HYST_VAL_SHIFT    8
#define ARM_PROC_CLK_CORE0_CLKGATE_HYST_VAL_MASK     (0x1<<ARM_PROC_CLK_CORE0_CLKGATE_HYST_VAL_SHIFT)
#define ARM_PROC_CLK_CORE0_CLKGATE_HYST_VAL_LOW      0x0
#define ARM_PROC_CLK_CORE0_CLKGATE_HYST_VAL_HIGH     0x1
#define ARM_PROC_CLK_CORE0_CLKGATE_GATING_SEL_SHIFT  1
#define ARM_PROC_CLK_CORE0_CLKGATE_GATING_SEL_MASK   (0x1<<ARM_PROC_CLK_CORE0_CLKGATE_GATING_SEL_SHIFT)
#define ARM_PROC_CLK_CORE0_CLKGATE_GATING_SEL_HW     0x0
#define ARM_PROC_CLK_CORE0_CLKGATE_GATING_SEL_SW     0x1
#define ARM_PROC_CLK_CORE0_CLKGATE_CLK_EN_SHIFT      0
#define ARM_PROC_CLK_CORE0_CLKGATE_CLK_EN_MASK       (0x1<<ARM_PROC_CLK_CORE0_CLKGATE_CLK_EN_SHIFT)
#define ARM_PROC_CLK_CORE0_CLKGATE_CLK_EN            1
#define ARM_PROC_CLK_CORE0_CLKGATE_CLK_DIS           0
   uint32 core1_clkgate;
#define ARM_PROC_CLK_CORE1_CLKGATE_HYST_EN_SHIFT     9
#define ARM_PROC_CLK_CORE1_CLKGATE_HYST_EN_MASK      (0x1<<ARM_PROC_CLK_CORE0_CLKGATE_HYST_EN_SHIFT)
#define ARM_PROC_CLK_CORE1_CLKGATE_HYST_EN           1
#define ARM_PROC_CLK_CORE1_CLKGATE_HYST_DIS          0
#define ARM_PROC_CLK_CORE1_CLKGATE_HYST_VAL_SHIFT    8
#define ARM_PROC_CLK_CORE1_CLKGATE_HYST_VAL_MASK     (0x1<<ARM_PROC_CLK_CORE0_CLKGATE_HYST_VAL_SHIFT)
#define ARM_PROC_CLK_CORE1_CLKGATE_HYST_VAL_LOW      0x0
#define ARM_PROC_CLK_CORE1_CLKGATE_HYST_VAL_HIGH     0x1
#define ARM_PROC_CLK_CORE1_CLKGATE_GATING_SEL_SHIFT  1
#define ARM_PROC_CLK_CORE1_CLKGATE_GATING_SEL_MASK   (0x1<<ARM_PROC_CLK_CORE0_CLKGATE_GATING_SEL_SHIFT)
#define ARM_PROC_CLK_CORE1_CLKGATE_GATING_SEL_HW     0x0
#define ARM_PROC_CLK_CORE1_CLKGATE_GATING_SEL_SW     0x1
#define ARM_PROC_CLK_CORE1_CLKGATE_CLK_EN_SHIFT      0
#define ARM_PROC_CLK_CORE1_CLKGATE_CLK_EN_MASK       (0x1<<ARM_PROC_CLK_CORE0_CLKGATE_CLK_EN_SHIFT)
#define ARM_PROC_CLK_CORE1_CLKGATE_CLK_EN            1
#define ARM_PROC_CLK_CORE1_CLKGATE_CLK_DIS           0
   uint32 unused4[2];
   uint32 arm_switch_clkgate; /* 0x210 */
#define ARM_PROC_CLK_SWITCH_CLKGATE_HYST_EN_SHIFT    9
#define ARM_PROC_CLK_SWITCH_CLKGATE_HYST_EN_MASK     (0x1<<ARM_PROC_CLK_CORE0_CLKGATE_HYST_EN_SHIFT)
#define ARM_PROC_CLK_SWITCH_CLKGATE_HYST_EN          1
#define ARM_PROC_CLK_SWITCH_CLKGATE_HYST_DIS         0
#define ARM_PROC_CLK_SWITCH_CLKGATE_HYST_VAL_SHIFT   8
#define ARM_PROC_CLK_SWITCH_CLKGATE_HYST_VAL_MASK    (0x1<<ARM_PROC_CLK_CORE0_CLKGATE_HYST_VAL_SHIFT)
#define ARM_PROC_CLK_SWITCH_CLKGATE_HYST_VAL_LOW     0x0
#define ARM_PROC_CLK_SWITCH_CLKGATE_HYST_VAL_HIGH    0x1
#define ARM_PROC_CLK_SWITCH_CLKGATE_GATING_SEL_SHIFT 1
#define ARM_PROC_CLK_SWITCH_CLKGATE_GATING_SEL_MASK  (0x1<<ARM_PROC_CLK_CORE0_CLKGATE_GATING_SEL_SHIFT)
#define ARM_PROC_CLK_SWITCH_CLKGATE_GATING_SEL_HW    0x0
#define ARM_PROC_CLK_SWITCH_CLKGATE_GATING_SEL_SW    0x1
#define ARM_PROC_CLK_SWITCH_CLKGATE_CLK_EN_SHIFT     0
#define ARM_PROC_CLK_SWITCH_CLKGATE_CLK_EN_MASK      (0x1<<ARM_PROC_CLK_CORE0_CLKGATE_CLK_EN_SHIFT)
#define ARM_PROC_CLK_SWITCH_CLKGATE_CLK_EN           1
#define ARM_PROC_CLK_SWITCH_CLKGATE_CLK_DIS          0
   uint32 unused5[59];
   uint32 arm_periph_clkgate; /* 0x300 */
#define ARM_PROC_CLK_PERIPH_CLKGATE_HYST_EN_SHIFT    9
#define ARM_PROC_CLK_PERIPH_CLKGATE_HYST_EN_MASK     (0x1<<ARM_PROC_CLK_CORE0_CLKGATE_HYST_EN_SHIFT)
#define ARM_PROC_CLK_PERIPH_CLKGATE_HYST_EN          1
#define ARM_PROC_CLK_PERIPH_CLKGATE_HYST_DIS         0
#define ARM_PROC_CLK_PERIPH_CLKGATE_HYST_VAL_SHIFT   8
#define ARM_PROC_CLK_PERIPH_CLKGATE_HYST_VAL_MASK    (0x1<<ARM_PROC_CLK_CORE0_CLKGATE_HYST_VAL_SHIFT)
#define ARM_PROC_CLK_PERIPH_CLKGATE_HYST_VAL_LOW     0x0
#define ARM_PROC_CLK_PERIPH_CLKGATE_HYST_VAL_HIGH    0x1
#define ARM_PROC_CLK_PERIPH_CLKGATE_GATING_SEL_SHIFT 1
#define ARM_PROC_CLK_PERIPH_CLKGATE_GATING_SEL_MASK  (0x1<<ARM_PROC_CLK_CORE0_CLKGATE_GATING_SEL_SHIFT)
#define ARM_PROC_CLK_PERIPH_CLKGATE_GATING_SEL_HW    0x0
#define ARM_PROC_CLK_PERIPH_CLKGATE_GATING_SEL_SW    0x1
#define ARM_PROC_CLK_PERIPH_CLKGATE_CLK_EN_SHIFT     0
#define ARM_PROC_CLK_PERIPH_CLKGATE_CLK_EN_MASK      (0x1<<ARM_PROC_CLK_CORE0_CLKGATE_CLK_EN_SHIFT)
#define ARM_PROC_CLK_PERIPH_CLKGATE_CLK_EN           1
#define ARM_PROC_CLK_PERIPH_CLKGATE_CLK_DIS          0
   uint32 unused6[63];
   uint32 apb0_clkgate;    /* 0x400 */
#define ARM_PROC_CLK_APB0_CLKGATE_HYST_EN_SHIFT      9
#define ARM_PROC_CLK_APB0_CLKGATE_HYST_EN_MASK       (0x1<<ARM_PROC_CLK_CORE0_CLKGATE_HYST_EN_SHIFT)
#define ARM_PROC_CLK_APB0_CLKGATE_HYST_EN            1
#define ARM_PROC_CLK_APB0_CLKGATE_HYST_DIS           0
#define ARM_PROC_CLK_APB0_CLKGATE_HYST_VAL_SHIFT     8
#define ARM_PROC_CLK_APB0_CLKGATE_HYST_VAL_MASK      (0x1<<ARM_PROC_CLK_CORE0_CLKGATE_HYST_VAL_SHIFT)
#define ARM_PROC_CLK_APB0_CLKGATE_HYST_VAL_LOW       0x0
#define ARM_PROC_CLK_APB0_CLKGATE_HYST_VAL_HIGH      0x1
#define ARM_PROC_CLK_APB0_CLKGATE_GATING_SEL_SHIFT   1
#define ARM_PROC_CLK_APB0_CLKGATE_GATING_SEL_MASK    (0x1<<ARM_PROC_CLK_CORE0_CLKGATE_GATING_SEL_SHIFT)
#define ARM_PROC_CLK_APB0_CLKGATE_GATING_SEL_HW      0x0
#define ARM_PROC_CLK_APB0_CLKGATE_GATING_SEL_SW      0x1
#define ARM_PROC_CLK_APB0_CLKGATE_CLK_EN_SHIFT       0
#define ARM_PROC_CLK_APB0_CLKGATE_CLK_EN_MASK        (0x1<<ARM_PROC_CLK_CORE0_CLKGATE_CLK_EN_SHIFT)
#define ARM_PROC_CLK_APB0_CLKGATE_CLK_EN             1
#define ARM_PROC_CLK_APB0_CLKGATE_CLK_DIS            0
   uint32 unused7[383];
   uint32 pl310_div;    /* 0xa00 */
   uint32 pl310_trigger;
   uint32 arm_switch_div;
   uint32 arm_switch_trigger;
   uint32 apb_div;         /* 0xa10 */
   uint32 apb_trigger;
   uint32 unused8[122];
   uint32 pllarma;         /* 0xc00 */
#define ARM_PROC_CLK_PLLARMA_PLL_LOCK                (0x1<<28)
#define ARM_PROC_CLK_PLLARMA_PDIV_SHIFT              24
#define ARM_PROC_CLK_PLLARMA_PDIV_MASK               (0x7<<ARM_PROC_CLK_PLLARMA_PDIV_SHIFT)
#define ARM_PROC_CLK_PLLARMA_NDIV_SHIFT              8
#define ARM_PROC_CLK_PLLARMA_NDIV_MASK               (0x3ff<<ARM_PROC_CLK_PLLARMA_NDIV_SHIFT)
#define ARM_PROC_CLK_PLLARMA_PLL_LOCK_RAW            (0x1<<7)
#define ARM_PROC_CLK_PLLARMA_PWRDWN_SWOVRRIDE_SHIFT  4
#define ARM_PROC_CLK_PLLARMA_PWRDWN_SWOVRRIDE_MASK   (0x1<<ARM_PROC_CLK_PLLARMA_PWRDWN_SWOVRRIDE_SHIFT)
#define ARM_PROC_CLK_PLLARMA_SOFT_POST_RESETB_N      0x2          
#define ARM_PROC_CLK_PLLARMA_SOFT_RESETB_N           0x1
   uint32 pllarmb;
#define ARM_PROC_CLK_PLLARMB_NDIV_FRAC_SHIFT         0
#define ARM_PROC_CLK_PLLARMB_NDIV_FRAC_MASK         (0xfffff<<ARM_PROC_CLK_PLLARMB_NDIV_FRAC_SHIFT)
   uint32 pllarmc;
#define ARM_PROC_CLK_PLLARMC_MDIV_SHIFT              0
#define ARM_PROC_CLK_PLLARMC_MDIV_MASK               (0xff<<ARM_PROC_CLK_PLLARMC_MDIV_SHIFT)
   uint32 pllarmctrl0;
   uint32 pllarmctrl1;     /* 0xc10 */
   uint32 pllarmctrl2;
   uint32 pllarmctrl3;
   uint32 pllarmctrl4;
   uint32 pllarmctrl5;     /* 0xc20 */
   uint32 pllarm_offset;
   uint32 unused9[118];
   uint32 arm_div;         /* 0xe00 */
   uint32 arm_seg_trg;
   uint32 arm_seg_trg_override;
   uint32 unused10;
   uint32 pll_debug;    /* 0xe10 */
   uint32 unused11[3];
   uint32 activity_mon1;      /* 0xe20 */
   uint32 activity_mon2;
   uint32 unused12[6];
   uint32 clkgate_dbg;     /* 0xe40 */
   uint32 unused13;
   uint32 apb_clkgate_dbg1;
   uint32 unused14[6];
   uint32 clkmon;       /* 0xe64 */
   uint32 unused15[10];
   uint32 kproc_ccu_prof_ctl; /* 0xe90 */
   uint32 kproc_cpu_proc_sel;
   uint32 kproc_cpu_proc_cnt;
   uint32 kproc_cpu_proc_dbg;
   uint32 unused16[8];
   uint32 policy_dbg;      /* 0xec0 */
   uint32 tgtmask_dbg1;
} ArmProcClkMgr;

typedef struct ArmProcResetMgr {
   uint32 wr_access;    /* 0x00 */
   uint32 soft_rstn;
   uint32 a9_core_soft_rstn;
} ArmProcResetMgr;

typedef struct ArmCfg {
   ArmProcClkMgr proc_clk;    /* 0x00 */
   uint32 unused0[14];     /* 0xec8 - 0xeff */
   ArmProcResetMgr proc_reset;   /* 0xf00 */
   //uint32 unused0[61];      /* 0xf0c - 0xfff */
} ArmCfg;
#define ARMCFG ((volatile ArmCfg * const) ARMCFG_BASE)

typedef struct ArmGTimer {
   uint32 gtim_glob_low;      /* 0x00 */
   uint32 gtim_glob_hi;    /* 0x04 */
   uint32 gtim_glob_ctrl;     /* 0x08 */
#define ARM_GTIM_GLOB_CTRL_PRESCALE_SHIFT            8
#define ARM_GTIM_GLOB_CTRL_PRESCALE_MASK             (0xff<<ARM_GTIM_GLOB_CTRL_PRESCALE_SHIFT)
#define ARM_GTIM_GLOB_CTRL_AUTOINCR_EN               0x8
#define ARM_GTIM_GLOB_CTRL_IRQ_EN                    0x4
#define ARM_GTIM_GLOB_CTRL_COMP_EN                   0x2
#define ARM_GTIM_GLOB_CTRL_TIMER_EN                  0x1
   uint32 gtim_glob_status;   /* 0x0c */
   uint32 gtim_glob_comp_low;       /* 0x10 */
   uint32 gtim_glob_comp_hi;        /* 0x14 */
   uint32 gtim_glob_incr;        /* 0x18 */
} ArmGTimer;

#define ARMGTIM ((volatile ArmGTimer * const) ARMGTIM_BASE)

typedef struct ArmAipCtrl {
   uint32 cfg;       /* 0x00 */
   uint32 l2c_ctrl;     /* 0x04 */
   uint32 arm_ctrl;     /* 0x08 */
   uint32 unused0;
   uint32 acp_ctrl[32];    /* 0x10-0xc */
#define AIPACP_WCACHE_SHIFT         0
#define AIPACP_WCACHE_MASK       0xf
#define AIPACP_WCACHE_GET(reg_val)     ((reg_val >> AIPACP_WCACHE_SHIFT) & AIPACP_WCACHE_MASK)
#define AIPACP_WCACHE_SET(reg_val,new_val)   ((reg_val & ~(AIPACP_WCACHE_MASK << AIPACP_WCACHE_SHIFT)) | (new_val << AIPACP_WCACHE_SHIFT))
#define AIPACP_RCACHE_SHIFT         4
#define AIPACP_RCACHE_MASK       0xf
#define AIPACP_RCACHE_GET(reg_val)     ((reg_val >> AIPACP_RCACHE_SHIFT) & AIPACP_RCACHE_MASK)
#define AIPACP_RCACHE_SET(reg_val,new_val)   ((reg_val & ~(AIPACP_RCACHE_MASK << AIPACP_RCACHE_SHIFT)) | (new_val << AIPACP_RCACHE_SHIFT))
#define AIPACP_WUSER_SHIFT       8
#define AIPACP_WUSER_MASK        0x1f
#define AIPACP_WUSER_GET(reg_val)      ((reg_val >> AIPACP_WUSER_SHIFT) & AIPACP_WUSER_MASK)
#define AIPACP_WUSER_SET(reg_val,new_val) ((reg_val & ~(AIPACP_WUSER_MASK << AIPACP_WUSER_SHIFT)) | (new_val << AIPACP_WUSER_SHIFT))
#define AIPACP_RUSER_SHIFT       13
#define AIPACP_RUSER_MASK        0x1f
#define AIPACP_RUSER_GET(reg_val)      ((reg_val >> AIPACP_RUSER_SHIFT) & AIPACP_RUSER_MASK)
#define AIPACP_RUSER_SET(reg_val,new_val) ((reg_val & ~(AIPACP_RUSER_MASK << AIPACP_RUSER_SHIFT)) | (new_val << AIPACP_RUSER_SHIFT))

   uint32 unused1[12];
   uint32 debug_permission;   /* 0xc0 */
   uint32 debug_en;     /* 0xc4 */
} ArmAipCtrl;
#define ARMAIPCTRL ((volatile ArmAipCtrl * const) AIP_BASE)

/*
 * LedControl Register Set Definitions.
 */
typedef struct LedControl {
   uint32 glbCtrl;      /* 0x00 */
   uint32 mask;      /* 0x04 */
   uint32 hWLedEn;      /* 0x08 */
   uint32 serialLedShiftSel; /* 0x0c */
   uint32 flashRateCtrl[4];   /* 0x10-0x1c */
   uint32 brightCtrl[4];   /* 0x20-0x2c */
   uint32 powerLedCfg;  /* 0x30 */
   uint32 pledLut[2][16];  /* 0x34-0x70, 0x74-0xb0 */
   uint32 HwPolarity;   /* 0xb4 */    /*B0 ONLY FROM HERE */
   uint32 SwData;    /* 0xb8 */
   uint32 SwPolarity;   /* 0xbc */
   uint32 ParallelLedPolarity;   /* 0xc0 */
   uint32 SerialLedPolarity;  /* 0xc4 */
   uint32 HwLedStatus;     /* 0xc8 */
   uint32 FlashCtrlStatus;   /* 0xcc */
   uint32 FlashBrtCtrl;      /* 0xd0 */
   uint32 FlashPLedOutStatus;/* 0xd4 */
   uint32 FlashSLedOutStatus;/* 0xd8 */
} LedControl;

#define LED_NUM_LEDS              32
#define LED_NUM_PWM_LEDS              2
#define LED ((volatile LedControl * const) LED_BASE)
#define LED_NUM_TO_MASK(X)  (1 << ((X) & (LED_NUM_LEDS-1)))


typedef struct I2s {
   uint32 cfg;              /* 0x00 Config Register    */
#define I2S_ENABLE               (1 << 31)    
#define I2S_MCLK_RATE_SHIFT      20 
#define I2S_OUT_R                (1 << 19)    
#define I2S_OUT_L                (1 << 18)    
#define I2S_CLKSEL_SHIFT         16 
#define I2S_CLK_100MHZ           0
#define I2S_CLK_50MHZ            1
#define I2S_CLK_25MHZ            2
#define I2S_MCLK_CLKSEL_CLR_MASK 0xFF0CFFFF
#define I2S_BITS_PER_SAMPLE_SHIFT 10 
#define I2S_BITS_PER_SAMPLE_32   0 
#define I2S_BITS_PER_SAMPLE_24   24
#define I2S_BITS_PER_SAMPLE_20   20
#define I2S_BITS_PER_SAMPLE_18   18
#define I2S_BITS_PER_SAMPLE_16   16
#define I2S_SCLK_POLARITY        (1 << 9)    
#define I2S_LRCK_POLARITY        (1 << 8)    
#define I2S_SCLKS_PER_1FS_DIV32_SHIFT  4 
#define I2S_DATA_JUSTIFICATION   (1 << 3)    
#define I2S_DATA_ALIGNMENT       (1 << 2)    
#define I2S_DATA_ENABLE          (1 << 1)    
#define I2S_CLOCK_ENABLE         (1 << 0)
   
   uint32 unused[3];        /* 0x04 - 0x0c */  
   uint32 intr;             /* 0x10 Interrupt Register */
#define I2S_DESC_OFF_LEVEL_SHIFT    12
#define I2S_DESC_IFF_LEVEL_SHIFT    8    
#define I2S_DESC_LEVEL_MASK         0x0F
#define I2S_DESC_OFF_OVERRUN_INTR   (1 << 3)
#define I2S_DESC_IFF_UNDERRUN_INTR  (1 << 2)
#define I2S_DESC_OFF_INTR           (1 << 1)
#define I2S_DESC_IFF_INTR           (1 << 0)
#define I2S_INTR_MASK               0x0F
   
   uint32 intr_en;          /* 0x14 Interrupt Enables Register */
#define I2S_DESC_INTR_TYPE_SEL        (1 << 4)
#define I2S_DESC_OFF_OVERRUN_INTR_EN  (1 << 3)
#define I2S_DESC_IFF_UNDERRUN_INTR_EN (1 << 2)
#define I2S_DESC_OFF_INTR_EN          (1 << 1)
#define I2S_DESC_IFF_INTR_EN          (1 << 0)
   
   uint32 intr_iff_thld;    /* 0x18 Descriptor Input FIFO Interrupt Threshold Register  */
   uint32 intr_off_thld;    /* 0x1c Descriptor Output FIFO Interrupt Threshold Register */
#define I2S_DESC_IFF_INTR_THLD_MASK    0x07
   
   uint32 desc_iff_addr;    /* 0x20 Descriptor Input FIFO Address  */
   uint32 desc_iff_len;     /* 0x24 Descriptor Input FIFO Length   */
   uint32 desc_off_addr;    /* 0x28 Descriptor Output FIFO Address */
   uint32 desc_off_len;     /* 0x2c Descriptor Output FIFO Length  */
#define I2S_DESC_EOP             (1 << 31)                                 
#define I2S_DESC_FIFO_DEPTH      8
#define I2S_DMA_BUFF_MAX_LEN     0xFFFF
#define I2S_DESC_LEN_MASK        I2S_DMA_BUFF_MAX_LEN
   
} I2s;

#define I2S ((volatile I2s * const) I2S_BASE)

/*
** High-Speed SPI Controller
*/
#define __mask(end, start)      (((1 << ((end - start) + 1)) - 1) << start)
typedef struct HsSpiControl {
   uint32 hs_spiGlobalCtrl;   /* 0x00 */
#define HS_SPI_MOSI_IDLE   (1 << 18)
#define HS_SPI_CLK_POLARITY   (1 << 17)
#define HS_SPI_CLK_GATE_SSOFF (1 << 16)
#define HS_SPI_PLL_CLK_CTRL   (8)
#define HS_SPI_PLL_CLK_CTRL_MASK __mask(15, HS_SPI_PLL_CLK_CTRL)
#define HS_SPI_SS_POLARITY (0)
#define HS_SPI_SS_POLARITY_MASK     __mask(7, HS_SPI_SS_POLARITY)

   uint32 hs_spiExtTrigCtrl;  /* 0x04 */
#define HS_SPI_TRIG_RAW_STATE (24)
#define HS_SPI_TRIG_RAW_STATE_MASK  __mask(31, HS_SPI_TRIG_RAW_STATE)
#define HS_SPI_TRIG_LATCHED   (16)
#define HS_SPI_TRIG_LATCHED_MASK __mask(23, HS_SPI_TRIG_LATCHED)
#define HS_SPI_TRIG_SENSE  (8)
#define HS_SPI_TRIG_SENSE_MASK      __mask(15, HS_SPI_TRIG_SENSE)
#define HS_SPI_TRIG_TYPE   (0)
#define HS_SPI_TRIG_TYPE_MASK    __mask(7, HS_SPI_TRIG_TYPE)
#define HS_SPI_TRIG_TYPE_EDGE (0)
#define HS_SPI_TRIG_TYPE_LEVEL   (1)

   uint32 hs_spiIntStatus;    /* 0x08 */
#define HS_SPI_IRQ_PING1_USER (28)
#define HS_SPI_IRQ_PING1_USER_MASK  __mask(31, HS_SPI_IRQ_PING1_USER)
#define HS_SPI_IRQ_PING0_USER (24)
#define HS_SPI_IRQ_PING0_USER_MASK  __mask(27, HS_SPI_IRQ_PING0_USER)

#define HS_SPI_IRQ_PING1_CTRL_INV   (1 << 12)
#define HS_SPI_IRQ_PING1_POLL_TOUT  (1 << 11)
#define HS_SPI_IRQ_PING1_TX_UNDER   (1 << 10)
#define HS_SPI_IRQ_PING1_RX_OVER (1 << 9)
#define HS_SPI_IRQ_PING1_CMD_DONE   (1 << 8)

#define HS_SPI_IRQ_PING0_CTRL_INV   (1 << 4)
#define HS_SPI_IRQ_PING0_POLL_TOUT  (1 << 3)
#define HS_SPI_IRQ_PING0_TX_UNDER   (1 << 2)
#define HS_SPI_IRQ_PING0_RX_OVER (1 << 1)
#define HS_SPI_IRQ_PING0_CMD_DONE   (1 << 0)

   uint32 hs_spiIntStatusMasked; /* 0x0C */
#define HS_SPI_IRQSM__PING1_USER (28)
#define HS_SPI_IRQSM__PING1_USER_MASK  __mask(31, HS_SPI_IRQSM__PING1_USER)
#define HS_SPI_IRQSM__PING0_USER (24)
#define HS_SPI_IRQSM__PING0_USER_MASK  __mask(27, HS_SPI_IRQSM__PING0_USER)

#define HS_SPI_IRQSM__PING1_CTRL_INV   (1 << 12)
#define HS_SPI_IRQSM__PING1_POLL_TOUT  (1 << 11)
#define HS_SPI_IRQSM__PING1_TX_UNDER   (1 << 10)
#define HS_SPI_IRQSM__PING1_RX_OVER (1 << 9)
#define HS_SPI_IRQSM__PING1_CMD_DONE   (1 << 8)

#define HS_SPI_IRQSM__PING0_CTRL_INV   (1 << 4)
#define HS_SPI_IRQSM__PING0_POLL_TOUT  (1 << 3)
#define HS_SPI_IRQSM__PING0_TX_UNDER   (1 << 2)
#define HS_SPI_IRQSM__PING0_RX_OVER (1 << 1)
#define HS_SPI_IRQSM__PING0_CMD_DONE   (1 << 0)

   uint32 hs_spiIntMask;      /* 0x10 */
#define HS_SPI_IRQM_PING1_USER      (28)
#define HS_SPI_IRQM_PING1_USER_MASK __mask(31, HS_SPI_IRQM_PING1_USER)
#define HS_SPI_IRQM_PING0_USER      (24)
#define HS_SPI_IRQM_PING0_USER_MASK __mask(27, HS_SPI_IRQM_PING0_USER)

#define HS_SPI_IRQM_PING1_CTRL_INV  (1 << 12)
#define HS_SPI_IRQM_PING1_POLL_TOUT (1 << 11)
#define HS_SPI_IRQM_PING1_TX_UNDER  (1 << 10)
#define HS_SPI_IRQM_PING1_RX_OVER   (1 << 9)
#define HS_SPI_IRQM_PING1_CMD_DONE  (1 << 8)

#define HS_SPI_IRQM_PING0_CTRL_INV  (1 << 4)
#define HS_SPI_IRQM_PING0_POLL_TOUT (1 << 3)
#define HS_SPI_IRQM_PING0_TX_UNDER  (1 << 2)
#define HS_SPI_IRQM_PING0_RX_OVER   (1 << 1)
#define HS_SPI_IRQM_PING0_CMD_DONE  (1 << 0)

#define HS_SPI_INTR_CLEAR_ALL    (0xFF001F1F)

   uint32 hs_spiFlashCtrl;    /* 0x14 */
#define HS_SPI_FCTRL_MB_ENABLE      (23)
#define HS_SPI_FCTRL_SS_NUM      (20)
#define HS_SPI_FCTRL_SS_NUM_MASK __mask(22, HS_SPI_FCTRL_SS_NUM)
#define HS_SPI_FCTRL_PROFILE_NUM (16)
#define HS_SPI_FCTRL_PROFILE_NUM_MASK  __mask(18, HS_SPI_FCTRL_PROFILE_NUM)
#define HS_SPI_FCTRL_DUMMY_BYTES (10)
#define HS_SPI_FCTRL_DUMMY_BYTES_MASK  __mask(11, HS_SPI_FCTRL_DUMMY_BYTES)
#define HS_SPI_FCTRL_ADDR_BYTES     (8)
#define HS_SPI_FCTRL_ADDR_BYTES_MASK   __mask(9, HS_SPI_FCTRL_ADDR_BYTES)
#define HS_SPI_FCTRL_ADDR_BYTES_2   (0)
#define HS_SPI_FCTRL_ADDR_BYTES_3   (1)
#define HS_SPI_FCTRL_ADDR_BYTES_4   (2)
#define HS_SPI_FCTRL_READ_OPCODE (0)
#define HS_SPI_FCTRL_READ_OPCODE_MASK  __mask(7, HS_SPI_FCTRL_READ_OPCODE)

   uint32 hs_spiFlashAddrBase;   /* 0x18 */
} HsSpiControl;

typedef struct HsSpiPingPong {
   uint32 command;      /* 0x00 */
#define HS_SPI_SS_NUM      (12)
#define ZSI_SPI_DEV_ID     7     // SS_N[7] connected to APM/PCM block for use by MSIF/ZDS interfaces
#define HS_SPI_PROFILE_NUM (8)
#define HS_SPI_TRIGGER_NUM (4)
#define HS_SPI_COMMAND_VALUE  (0)
    #define HS_SPI_COMMAND_NOOP     (0)
    #define HS_SPI_COMMAND_START_NOW   (1)
    #define HS_SPI_COMMAND_START_TRIGGER (2)
    #define HS_SPI_COMMAND_HALT     (3)
    #define HS_SPI_COMMAND_FLUSH (4)

   uint32 status;    /* 0x04 */
#define HS_SPI_ERROR_BYTE_OFFSET (16)
#define HS_SPI_WAIT_FOR_TRIGGER     (2)
#define HS_SPI_SOURCE_BUSY    (1)
#define HS_SPI_SOURCE_GNT     (0)

   uint32 fifo_status;  /* 0x08 */
   uint32 control;      /* 0x0c */
} HsSpiPingPong;

typedef struct HsSpiProfile {
   uint32 clk_ctrl;  /* 0x00 */
#define HS_SPI_ACCUM_RST_ON_LOOP (15)
#define HS_SPI_SPI_CLK_2X_SEL    (14)
#define HS_SPI_FREQ_CTRL_WORD    (0)

   uint32 signal_ctrl;  /* 0x04 */
#define  HS_SPI_ASYNC_INPUT_PATH (1 << 16)
#define  HS_SPI_LAUNCH_RISING (1 << 13)
#define  HS_SPI_LATCH_RISING  (1 << 12)

   uint32 mode_ctrl; /* 0x08 */
#define HS_SPI_PREPENDBYTE_CNT      (24)
#define HS_SPI_MODE_ONE_WIRE     (20)
#define HS_SPI_MULTIDATA_WR_SIZE (18)
#define HS_SPI_MULTIDATA_RD_SIZE (16)
#define HS_SPI_MULTIDATA_WR_STRT (12)
#define HS_SPI_MULTIDATA_RD_STRT (8)
#define HS_SPI_FILLBYTE       (0)

   uint32 polling_config;  /* 0x0c */
   uint32 polling_and_mask;   /* 0x10 */
   uint32 polling_compare; /* 0x14 */
   uint32 polling_timeout; /* 0x18 */
   uint32 reserved;  /* 0x1c */

} HsSpiProfile;

#define HS_SPI_OP_CODE 13
    #define HS_SPI_OP_SLEEP (0)
    #define HS_SPI_OP_READ_WRITE (1)
    #define HS_SPI_OP_WRITE (2)
    #define HS_SPI_OP_READ (3)
    #define HS_SPI_OP_SETIRQ (4)

#define HS_SPI ((volatile HsSpiControl * const) HSSPIM_BASE)
#define HS_SPI_PINGPONG0 ((volatile HsSpiPingPong * const) (HSSPIM_BASE + 0x80))
#define HS_SPI_PINGPONG1 ((volatile HsSpiPingPong * const) (HSSPIM_BASE + 0xc0))
#define HS_SPI_PROFILES ((volatile HsSpiProfile * const) (HSSPIM_BASE + 0x100))
#define HS_SPI_FIFO0 ((volatile uint8 * const) (HSSPIM_BASE + 0x200))
#define HS_SPI_FIFO1 ((volatile uint8 * const) (HSSPIM_BASE + 0x400))


/*
** NAND Controller Registers
*/
typedef struct NandCtrlRegs {
   uint32 NandRevision; /* 0x00 */
   uint32 NandCmdStart; /* 0x04 */
#define NCMD_MASK    0x0000001f
#define NCMD_BLOCK_ERASE_MULTI   0x15
#define NCMD_PROGRAM_PAGE_MULTI  0x13
#define NCMD_STS_READ_MULTI   0x12
#define NCMD_PAGE_READ_MULTI  0x11
#define NCMD_LOW_LEVEL_OP  0x10
#define NCMD_PARAM_CHG_COL 0x0f
#define NCMD_PARAM_READ    0x0e
#define NCMD_BLK_LOCK_STS  0x0d
#define NCMD_BLK_UNLOCK    0x0c
#define NCMD_BLK_LOCK_DOWN 0x0b
#define NCMD_BLK_LOCK      0x0a
#define NCMD_FLASH_RESET   0x09
#define NCMD_BLOCK_ERASE   0x08
#define NCMD_DEV_ID_READ   0x07
#define NCMD_COPY_BACK     0x06
#define NCMD_PROGRAM_SPARE 0x05
#define NCMD_PROGRAM_PAGE  0x04
#define NCMD_STS_READ      0x03
#define NCMD_SPARE_READ    0x02
#define NCMD_PAGE_READ     0x01

   uint32 NandCmdExtAddr;  /* 0x08 */
   uint32 NandCmdAddr;  /* 0x0c */
   uint32 NandCmdEndAddr;  /* 0x10 */
   uint32 NandIntfcStatus; /* 0x14 */
#define NIS_CTLR_READY     (1 << 31)
#define NIS_FLASH_READY    (1 << 30)
#define NIS_CACHE_VALID    (1 << 29)
#define NIS_SPARE_VALID    (1 << 28)
#define NIS_FLASH_STS_MASK 0x000000ff
#define NIS_WRITE_PROTECT  0x00000080
#define NIS_DEV_READY      0x00000040
#define NIS_PGM_ERASE_ERROR   0x00000001


   uint32 NandNandBootConfig; /* 0x18 */
#define NBC_CS_LOCK     (1 << 31)
#define NBC_AUTO_DEV_ID_CFG   (1 << 30)
#define NBC_WR_PROT_BLK0   (1 << 28)
#define NBC_EBI_CS7_USES_NAND (1<<15)
#define NBC_EBI_CS6_USES_NAND (1<<14)
#define NBC_EBI_CS5_USES_NAND (1<<13)
#define NBC_EBI_CS4_USES_NAND (1<<12)
#define NBC_EBI_CS3_USES_NAND (1<<11)
#define NBC_EBI_CS2_USES_NAND (1<<10)
#define NBC_EBI_CS1_USES_NAND (1<< 9)
#define NBC_EBI_CS0_USES_NAND (1<< 8)
#define NBC_EBC_CS7_SEL    (1<< 7)
#define NBC_EBC_CS6_SEL    (1<< 6)
#define NBC_EBC_CS5_SEL    (1<< 5)
#define NBC_EBC_CS4_SEL    (1<< 4)
#define NBC_EBC_CS3_SEL    (1<< 3)
#define NBC_EBC_CS2_SEL    (1<< 2)
#define NBC_EBC_CS1_SEL    (1<< 1)
#define NBC_EBC_CS0_SEL    (1<< 0)

   uint32 NandCsNandXor;      /* 0x1c */
   uint32 NandLlOpNand;            /* 0x20 */
   uint32 NandMplaneBaseExtAddr; /* 0x24 */
   uint32 NandMplaneBaseAddr; /* 0x28 */
   uint32 NandReserved1[9];   /* 0x2c-0x4f */
   uint32 NandAccControl;     /* 0x50 */
#define NAC_RD_ECC_EN      (1 << 31)
#define NAC_WR_ECC_EN      (1 << 30)
#define NAC_CE_CARE_EN          (1 << 28)
#define NAC_RD_ERASED_ECC_EN  (1 << 27)
#define NAC_PARTIAL_PAGE_EN   (1 << 26)
#define NAC_WR_PREEMPT_EN  (1 << 25)
#define NAC_PAGE_HIT_EN    (1 << 24)
#define NAC_PREFETCH_EN    (1 << 23)
#define NAC_CACHE_MODE_EN  (1 << 22)
#define NAC_ECC_LVL_SHIFT  16
#define NAC_ECC_LVL_MASK   0x001f0000
#define NAC_ECC_LVL_DISABLE 0
#define NAC_ECC_LVL_BCH_1   1
#define NAC_ECC_LVL_BCH_2   2
#define NAC_ECC_LVL_BCH_3   3
#define NAC_ECC_LVL_BCH_4   4
#define NAC_ECC_LVL_BCH_5   5
#define NAC_ECC_LVL_BCH_6   6
#define NAC_ECC_LVL_BCH_7   7
#define NAC_ECC_LVL_BCH_8   8
#define NAC_ECC_LVL_BCH_9   9
#define NAC_ECC_LVL_BCH_10  10
#define NAC_ECC_LVL_BCH_11  11
#define NAC_ECC_LVL_BCH_12  12
#define NAC_ECC_LVL_BCH_13  13
#define NAC_ECC_LVL_BCH_14  14
#define NAC_ECC_LVL_HAMMING 15  /* Hamming if spare are size = 16, BCH15 otherwise */
#define NAC_ECC_LVL_BCH15   15    
#define NAC_ECC_LVL_BCH_16  16
#define NAC_ECC_LVL_BCH_17  17
/* BCH18 to 30 for sector size = 1K. To be added when we need it */
#define NAC_SECTOR_SIZE_1K (1 << 7)
#define NAC_SPARE_SZ_SHIFT 0
#define NAC_SPARE_SZ_MASK  0x0000007f

   uint32 NandConfig;      /* 0x54 */ /* Nand Flash Config */
#define NC_CONFIG_LOCK     (1 << 31)
#define NC_BLK_SIZE_MASK   (0x7 << 28)
#define NC_BLK_SIZE_2048K  (0x6 << 28)
#define NC_BLK_SIZE_1024K  (0x5 << 28)
#define NC_BLK_SIZE_512K   (0x4 << 28)
#define NC_BLK_SIZE_256K   (0x3 << 28)
#define NC_BLK_SIZE_128K   (0x2 << 28)
#define NC_BLK_SIZE_16K    (0x1 << 28)
#define NC_BLK_SIZE_8K     (0x0 << 28)
#define NC_DEV_SIZE_SHIFT  24
#define NC_DEV_SIZE_MASK   (0x0f << NC_DEV_SIZE_SHIFT)
#define NC_DEV_WIDTH_MASK  (1 << 23)
#define NC_DEV_WIDTH_16    (1 << 23)
#define NC_DEV_WIDTH_8     (0 << 23)
#define NC_PG_SIZE_MASK    (0x3 << 20)
#define NC_PG_SIZE_8K      (0x3 << 20)
#define NC_PG_SIZE_4K      (0x2 << 20)
#define NC_PG_SIZE_2K      (0x1 << 20)
#define NC_PG_SIZE_512B    (0x0 << 20)
#define NC_FUL_ADDR_SHIFT  16
#define NC_FUL_ADDR_MASK   (0x7 << NC_FUL_ADDR_SHIFT)
#define NC_BLK_ADDR_SHIFT  8
#define NC_BLK_ADDR_MASK   (0x07 << NC_BLK_ADDR_SHIFT)

   uint32 NandTiming1;  /* 0x58 */ /* Nand Flash Timing Parameters 1 */
#define NT_TREH_MASK        0x000f0000
#define NT_TREH_SHIFT       16
#define NT_TRP_MASK         0x00f00000
#define NT_TRP_SHIFT        20
   uint32 NandTiming2;  /* 0x5c */ /* Nand Flash Timing Parameters 2 */
#define NT_TREAD_MASK       0x0000000f
#define NT_TREAD_SHIFT      0
   /* 0x60 */
   uint32 NandAccControlCs1;  /* Nand Flash Access Control */
   uint32 NandConfigCs1;      /* Nand Flash Config */
   uint32 NandTiming1Cs1;     /* Nand Flash Timing Parameters 1 */
   uint32 NandTiming2Cs1;     /* Nand Flash Timing Parameters 2 */
   /* 0x70 */
   uint32 NandAccControlCs2;  /* Nand Flash Access Control */
   uint32 NandConfigCs2;      /* Nand Flash Config */
   uint32 NandTiming1Cs2;     /* Nand Flash Timing Parameters 1 */
   uint32 NandTiming2Cs2;     /* Nand Flash Timing Parameters 2 */
   /* 0x80 */
   uint32 NandAccControlCs3;  /* Nand Flash Access Control */
   uint32 NandConfigCs3;      /* Nand Flash Config */
   uint32 NandTiming1Cs3;     /* Nand Flash Timing Parameters 1 */
   uint32 NandTiming2Cs3;     /* Nand Flash Timing Parameters 2 */
   /* 0x90 */
   uint32 NandAccControlCs4;  /* Nand Flash Access Control */
   uint32 NandConfigCs4;      /* Nand Flash Config */
   uint32 NandTiming1Cs4;     /* Nand Flash Timing Parameters 1 */
   uint32 NandTiming2Cs4;     /* Nand Flash Timing Parameters 2 */
   /* 0xa0 */
   uint32 NandAccControlCs5;  /* Nand Flash Access Control */
   uint32 NandConfigCs5;      /* Nand Flash Config */
   uint32 NandTiming1Cs5;     /* Nand Flash Timing Parameters 1 */
   uint32 NandTiming2Cs5;     /* Nand Flash Timing Parameters 2 */
   /* 0xb0 */
   uint32 NandAccControlCs6;  /* Nand Flash Access Control */
   uint32 NandConfigCs6;      /* Nand Flash Config */
   uint32 NandTiming1Cs6;     /* Nand Flash Timing Parameters 1 */
   uint32 NandTiming2Cs6;     /* Nand Flash Timing Parameters 2 */

   /* 0xc0 */
   uint32 NandCorrStatThreshold; /* Correctable Error Reporting Threshold */
   uint32 NandCorrStatThresholdExt; /* Correctable Error Reporting
                   * Threshold */
   uint32 NandBlkWrProtect;   /* Block Write Protect Enable and Size */
               /*   for EBI_CS0b */
   uint32 NandMplaneOpcode1;

   /* 0xd0 */
   uint32 NandMplaneOpcode2;
   uint32 NandMplaneCtrl;
   uint32 NandReserved2[9];   /* 0xd8-0xfb */
   uint32 NandUncorrErrorCount;  /* 0xfc */

   /* 0x100 */
   uint32 NandCorrErrorCount;
   uint32 NandReadErrorCount; /* Read Error Count */
   uint32 NandBlockLockStatus;   /* Nand Flash Block Lock Status */
   uint32 NandEccCorrExtAddr; /* ECC Correctable Error Extended Address*/
   /* 0x110 */
   uint32 NandEccCorrAddr;    /* ECC Correctable Error Address */
   uint32 NandEccUncExtAddr;  /* ECC Uncorrectable Error Extended Addr */
   uint32 NandEccUncAddr;     /* ECC Uncorrectable Error Address */
   uint32 NandFlashReadExtAddr;  /* Flash Read Data Extended Address */
   /* 0x120 */
   uint32 NandFlashReadAddr;  /* Flash Read Data Address */
   uint32 NandProgramPageExtAddr;   /* Page Program Extended Address */
   uint32 NandProgramPageAddr;   /* Page Program Address */
   uint32 NandCopyBackExtAddr;   /* Copy Back Extended Address */
   /* 0x130 */
   uint32 NandCopyBackAddr;   /* Copy Back Address */
   uint32 NandBlockEraseExtAddr; /* Block Erase Extended Address */
   uint32 NandBlockEraseAddr; /* Block Erase Address */
   uint32 NandInvReadExtAddr; /* Flash Invalid Data Extended Address */
   /* 0x140 */
   uint32 NandInvReadAddr;    /* Flash Invalid Data Address */
   uint32 NandInitStatus;
   uint32 NandOnfiStatus;     /* ONFI Status */
   uint32 NandOnfiDebugData;  /* ONFI Debug Data */

   uint32 NandSemaphore;      /* 0x150 */ /* Semaphore */
   uint32 NandReserved3[16];  /* 0x154-0x193 */

   /* 0x194 */
   uint32 NandFlashDeviceId;  /* Nand Flash Device ID */
   uint32 NandFlashDeviceIdExt;  /* Nand Flash Extended Device ID */
   uint32 NandLlRdData;    /* Nand Flash Low Level Read Data */

   uint32 NandReserved4[24];  /* 0x1a0 - 0x1ff */

   /* 0x200 */
   uint32 NandSpareAreaReadOfs0; /* Nand Flash Spare Area Read Bytes 0-3 */
   uint32 NandSpareAreaReadOfs4; /* Nand Flash Spare Area Read Bytes 4-7 */
   uint32 NandSpareAreaReadOfs8; /* Nand Flash Spare Area Read Bytes 8-11 */
   uint32 NandSpareAreaReadOfsC; /* Nand Flash Spare Area Read Bytes 12-15*/
   /* 0x210 */
   uint32 NandSpareAreaReadOfs10;   /* Nand Flash Spare Area Read Bytes 16-19 */
   uint32 NandSpareAreaReadOfs14;   /* Nand Flash Spare Area Read Bytes 20-23 */
   uint32 NandSpareAreaReadOfs18;   /* Nand Flash Spare Area Read Bytes 24-27 */
   uint32 NandSpareAreaReadOfs1C;   /* Nand Flash Spare Area Read Bytes 28-31*/
   /* 0x220 */
   uint32 NandSpareAreaReadOfs20;   /* Nand Flash Spare Area Read Bytes 32-35 */
   uint32 NandSpareAreaReadOfs24;   /* Nand Flash Spare Area Read Bytes 36-39 */
   uint32 NandSpareAreaReadOfs28;   /* Nand Flash Spare Area Read Bytes 40-43 */
   uint32 NandSpareAreaReadOfs2C;   /* Nand Flash Spare Area Read Bytes 44-47*/
   /* 0x230 */
   uint32 NandSpareAreaReadOfs30;   /* Nand Flash Spare Area Read Bytes 48-51 */
   uint32 NandSpareAreaReadOfs34;   /* Nand Flash Spare Area Read Bytes 52-55 */
   uint32 NandSpareAreaReadOfs38;   /* Nand Flash Spare Area Read Bytes 56-59 */
   uint32 NandSpareAreaReadOfs3C;   /* Nand Flash Spare Area Read Bytes 60-63*/

   uint32 NandReserved5[16];  /* 0x240-0x27f */

   /* 0x280 */
   uint32 NandSpareAreaWriteOfs0;   /* Nand Flash Spare Area Write Bytes 0-3 */
   uint32 NandSpareAreaWriteOfs4;   /* Nand Flash Spare Area Write Bytes 4-7 */
   uint32 NandSpareAreaWriteOfs8;   /* Nand Flash Spare Area Write Bytes 8-11 */
   uint32 NandSpareAreaWriteOfsC;   /* Nand Flash Spare Area Write Bytes 12-15 */
   /* 0x290 */
   uint32 NandSpareAreaWriteOfs10;  /* Nand Flash Spare Area Write Bytes 16-19 */
   uint32 NandSpareAreaWriteOfs14;  /* Nand Flash Spare Area Write Bytes 20-23 */
   uint32 NandSpareAreaWriteOfs18;  /* Nand Flash Spare Area Write Bytes 24-27 */
   uint32 NandSpareAreaWriteOfs1C;  /* Nand Flash Spare Area Write Bytes 28-31 */
   /* 0x2a0 */
   uint32 NandSpareAreaWriteOfs20;  /* Nand Flash Spare Area Write Bytes 32-35 */
   uint32 NandSpareAreaWriteOfs24;  /* Nand Flash Spare Area Write Bytes 36-39 */
   uint32 NandSpareAreaWriteOfs28;  /* Nand Flash Spare Area Write Bytes 40-43 */
   uint32 NandSpareAreaWriteOfs2C;  /* Nand Flash Spare Area Write Bytes 44-47 */
   /* 0x2b0 */
   uint32 NandSpareAreaWriteOfs30;  /* Nand Flash Spare Area Write Bytes 48-51 */
   uint32 NandSpareAreaWriteOfs34;  /* Nand Flash Spare Area Write Bytes 52-55 */
   uint32 NandSpareAreaWriteOfs38;  /* Nand Flash Spare Area Write Bytes 56-59 */
   uint32 NandSpareAreaWriteOfs3C;  /* Nand Flash Spare Area Write Bytes 60-63 */
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
   uint32 NandReserved6[69];  /* 0x2ec-0x3ff */

   /* 0x400 */
   uint32 NandFlashCache[128];   /* 0x400-0x5ff */
} NandCtrlRegs;

#define NAND ((volatile NandCtrlRegs * const) NAND_REG_BASE)

/*
 * EMMC control registers
 */
typedef struct EmmcHostIfRegs {
   uint32 emmc_host_sdma;                  /* 0x00 System DMA Address Register                                     */
/***************************************************************************
 *SDMA - System DMA Address Register
 ***************************************************************************/
/* EMMC_HOSTIF :: SDMA :: ADDRESS [31:00] */
#define EMMC_HOSTIF_SDMA_ADDRESS_MASK                         0xffffffff
#define EMMC_HOSTIF_SDMA_ADDRESS_SHIFT                        0
#define EMMC_HOSTIF_SDMA_ADDRESS_DEFAULT                      0x00000000
   
   uint32 emmc_host_block;                 /* 0x04 Block Reset and Count Register                                  */
/***************************************************************************
 *BLOCK - Block Reset and Count Register
 ***************************************************************************/
/* EMMC_HOSTIF :: BLOCK :: TRANSFER_BLOCK_COUNT [31:16] */
#define EMMC_HOSTIF_BLOCK_TRANSFER_BLOCK_COUNT_MASK           0xffff0000
#define EMMC_HOSTIF_BLOCK_TRANSFER_BLOCK_COUNT_SHIFT          16
#define EMMC_HOSTIF_BLOCK_TRANSFER_BLOCK_COUNT_DEFAULT        0x00000000

/* EMMC_HOSTIF :: BLOCK :: TRANSFER_BLOCK_SIZE_MSB [15:15] */
#define EMMC_HOSTIF_BLOCK_TRANSFER_BLOCK_SIZE_MSB_MASK        0x00008000
#define EMMC_HOSTIF_BLOCK_TRANSFER_BLOCK_SIZE_MSB_SHIFT       15
#define EMMC_HOSTIF_BLOCK_TRANSFER_BLOCK_SIZE_MSB_DEFAULT     0x00000000

/* EMMC_HOSTIF :: BLOCK :: HOST_BUFFER_SIZE [14:12] */
#define EMMC_HOSTIF_BLOCK_HOST_BUFFER_SIZE_MASK               0x00007000
#define EMMC_HOSTIF_BLOCK_HOST_BUFFER_SIZE_SHIFT              12
#define EMMC_HOSTIF_BLOCK_HOST_BUFFER_SIZE_DEFAULT            0x00000000

/* EMMC_HOSTIF :: BLOCK :: TRANSFER_BLOCK_SIZE [11:00] */
#define EMMC_HOSTIF_BLOCK_TRANSFER_BLOCK_SIZE_MASK            0x00000fff
#define EMMC_HOSTIF_BLOCK_TRANSFER_BLOCK_SIZE_SHIFT           0
#define EMMC_HOSTIF_BLOCK_TRANSFER_BLOCK_SIZE_DEFAULT         0x00000000
   
   uint32 emmc_host_argument;              /* 0x08 Argument Register                                               */
/***************************************************************************
 *ARGUMENT - Argument Register
 ***************************************************************************/
/* EMMC_HOSTIF :: ARGUMENT :: CMD_ARG1 [31:00] */
#define EMMC_HOSTIF_ARGUMENT_CMD_ARG1_MASK                    0xffffffff
#define EMMC_HOSTIF_ARGUMENT_CMD_ARG1_SHIFT                   0
#define EMMC_HOSTIF_ARGUMENT_CMD_ARG1_DEFAULT                 0x00000000
   
   uint32 emmc_host_cmd_mode;              /* 0x0c Command and Mode Register                                       */
/***************************************************************************
 *CMD_MODE - Command and Mode Register
 ***************************************************************************/
/* EMMC_HOSTIF :: CMD_MODE :: reserved0 [31:30] */
#define EMMC_HOSTIF_CMD_MODE_reserved0_MASK                   0xc0000000
#define EMMC_HOSTIF_CMD_MODE_reserved0_SHIFT                  30

/* EMMC_HOSTIF :: CMD_MODE :: CMD_INDEX [29:24] */
#define EMMC_HOSTIF_CMD_MODE_CMD_INDEX_MASK                   0x3f000000
#define EMMC_HOSTIF_CMD_MODE_CMD_INDEX_SHIFT                  24
#define EMMC_HOSTIF_CMD_MODE_CMD_INDEX_DEFAULT                0x00000000

/* EMMC_HOSTIF :: CMD_MODE :: CMD_TYPE [23:22] */
#define EMMC_HOSTIF_CMD_MODE_CMD_TYPE_MASK                    0x00c00000
#define EMMC_HOSTIF_CMD_MODE_CMD_TYPE_SHIFT                   22
#define EMMC_HOSTIF_CMD_MODE_CMD_TYPE_DEFAULT                 0x00000000

/* EMMC_HOSTIF :: CMD_MODE :: DATA_PRESENT [21:21] */
#define EMMC_HOSTIF_CMD_MODE_DATA_PRESENT_MASK                0x00200000
#define EMMC_HOSTIF_CMD_MODE_DATA_PRESENT_SHIFT               21
#define EMMC_HOSTIF_CMD_MODE_DATA_PRESENT_DEFAULT             0x00000000

/* EMMC_HOSTIF :: CMD_MODE :: CMD_INDEX_CHECK [20:20] */
#define EMMC_HOSTIF_CMD_MODE_CMD_INDEX_CHECK_MASK             0x00100000
#define EMMC_HOSTIF_CMD_MODE_CMD_INDEX_CHECK_SHIFT            20
#define EMMC_HOSTIF_CMD_MODE_CMD_INDEX_CHECK_DEFAULT          0x00000000

/* EMMC_HOSTIF :: CMD_MODE :: CMD_CRC_CHECK [19:19] */
#define EMMC_HOSTIF_CMD_MODE_CMD_CRC_CHECK_MASK               0x00080000
#define EMMC_HOSTIF_CMD_MODE_CMD_CRC_CHECK_SHIFT              19
#define EMMC_HOSTIF_CMD_MODE_CMD_CRC_CHECK_DEFAULT            0x00000000

/* EMMC_HOSTIF :: CMD_MODE :: reserved1 [18:18] */
#define EMMC_HOSTIF_CMD_MODE_reserved1_MASK                   0x00040000
#define EMMC_HOSTIF_CMD_MODE_reserved1_SHIFT                  18

/* EMMC_HOSTIF :: CMD_MODE :: RESPONSE_TYPE [17:16] */
#define EMMC_HOSTIF_CMD_MODE_RESPONSE_TYPE_MASK               0x00030000
#define EMMC_HOSTIF_CMD_MODE_RESPONSE_TYPE_SHIFT              16
#define EMMC_HOSTIF_CMD_MODE_RESPONSE_TYPE_DEFAULT            0x00000000

/* EMMC_HOSTIF :: CMD_MODE :: reserved2 [15:07] */
#define EMMC_HOSTIF_CMD_MODE_reserved2_MASK                   0x0000ff80
#define EMMC_HOSTIF_CMD_MODE_reserved2_SHIFT                  7

/* EMMC_HOSTIF :: CMD_MODE :: reserved_for_eco3 [06:06] */
#define EMMC_HOSTIF_CMD_MODE_reserved_for_eco3_MASK           0x00000040
#define EMMC_HOSTIF_CMD_MODE_reserved_for_eco3_SHIFT          6
#define EMMC_HOSTIF_CMD_MODE_reserved_for_eco3_DEFAULT        0x00000000

/* EMMC_HOSTIF :: CMD_MODE :: MULTI_BLOCK [05:05] */
#define EMMC_HOSTIF_CMD_MODE_MULTI_BLOCK_MASK                 0x00000020
#define EMMC_HOSTIF_CMD_MODE_MULTI_BLOCK_SHIFT                5
#define EMMC_HOSTIF_CMD_MODE_MULTI_BLOCK_DEFAULT              0x00000000

/* EMMC_HOSTIF :: CMD_MODE :: TRANFER_WRITE [04:04] */
#define EMMC_HOSTIF_CMD_MODE_TRANFER_WRITE_MASK               0x00000010
#define EMMC_HOSTIF_CMD_MODE_TRANFER_WRITE_SHIFT              4
#define EMMC_HOSTIF_CMD_MODE_TRANFER_WRITE_DEFAULT            0x00000000

/* EMMC_HOSTIF :: CMD_MODE :: AUTO_CMD_ENA [03:02] */
#define EMMC_HOSTIF_CMD_MODE_AUTO_CMD_ENA_MASK                0x0000000c
#define EMMC_HOSTIF_CMD_MODE_AUTO_CMD_ENA_SHIFT               2
#define EMMC_HOSTIF_CMD_MODE_AUTO_CMD_ENA_DEFAULT             0x00000000

/* EMMC_HOSTIF :: CMD_MODE :: BLOCK_COUNT_ENABLE [01:01] */
#define EMMC_HOSTIF_CMD_MODE_BLOCK_COUNT_ENABLE_MASK          0x00000002
#define EMMC_HOSTIF_CMD_MODE_BLOCK_COUNT_ENABLE_SHIFT         1
#define EMMC_HOSTIF_CMD_MODE_BLOCK_COUNT_ENABLE_DEFAULT       0x00000000

/* EMMC_HOSTIF :: CMD_MODE :: DMA_ENABLE [00:00] */
#define EMMC_HOSTIF_CMD_MODE_DMA_ENABLE_MASK                  0x00000001
#define EMMC_HOSTIF_CMD_MODE_DMA_ENABLE_SHIFT                 0
#define EMMC_HOSTIF_CMD_MODE_DMA_ENABLE_DEFAULT               0x00000000
   
   uint32 emmc_host_resp_01;               /* 0x10 Response Word 0 and 1                                           */
/***************************************************************************
 *RESP_01 - Response Word 0 and 1
 ***************************************************************************/
/* EMMC_HOSTIF :: RESP_01 :: RESP_HI [31:16] */
#define EMMC_HOSTIF_RESP_01_RESP_HI_MASK                      0xffff0000
#define EMMC_HOSTIF_RESP_01_RESP_HI_SHIFT                     16
#define EMMC_HOSTIF_RESP_01_RESP_HI_DEFAULT                   0x00000000

/* EMMC_HOSTIF :: RESP_01 :: RESP_LO [15:00] */
#define EMMC_HOSTIF_RESP_01_RESP_LO_MASK                      0x0000ffff
#define EMMC_HOSTIF_RESP_01_RESP_LO_SHIFT                     0
#define EMMC_HOSTIF_RESP_01_RESP_LO_DEFAULT                   0x00000000
   
   uint32 emmc_host_resp_23;               /* 0x14 Response Word 2 and 3                                           */
/***************************************************************************
 *RESP_23 - Response Word 2 and 3
 ***************************************************************************/
/* EMMC_HOSTIF :: RESP_23 :: RESP_HI [31:16] */
#define EMMC_HOSTIF_RESP_23_RESP_HI_MASK                      0xffff0000
#define EMMC_HOSTIF_RESP_23_RESP_HI_SHIFT                     16
#define EMMC_HOSTIF_RESP_23_RESP_HI_DEFAULT                   0x00000000

/* EMMC_HOSTIF :: RESP_23 :: RESP_LO [15:00] */
#define EMMC_HOSTIF_RESP_23_RESP_LO_MASK                      0x0000ffff
#define EMMC_HOSTIF_RESP_23_RESP_LO_SHIFT                     0
#define EMMC_HOSTIF_RESP_23_RESP_LO_DEFAULT                   0x00000000
   
   uint32 emmc_host_resp_45;               /* 0x18 Response Word 4 and 5                                           */
/***************************************************************************
 *RESP_45 - Response Word 4 and 5
 ***************************************************************************/
/* EMMC_HOSTIF :: RESP_45 :: RESP_HI [31:16] */
#define EMMC_HOSTIF_RESP_45_RESP_HI_MASK                      0xffff0000
#define EMMC_HOSTIF_RESP_45_RESP_HI_SHIFT                     16
#define EMMC_HOSTIF_RESP_45_RESP_HI_DEFAULT                   0x00000000

/* EMMC_HOSTIF :: RESP_45 :: RESP_LO [15:00] */
#define EMMC_HOSTIF_RESP_45_RESP_LO_MASK                      0x0000ffff
#define EMMC_HOSTIF_RESP_45_RESP_LO_SHIFT                     0
#define EMMC_HOSTIF_RESP_45_RESP_LO_DEFAULT                   0x00000000
   
   uint32 emmc_host_resp_67;               /* 0x1c Response Word 6 and 7                                           */
/***************************************************************************
 *RESP_67 - Response Word 6 and 7
 ***************************************************************************/
/* EMMC_HOSTIF :: RESP_67 :: RESP_HI [31:16] */
#define EMMC_HOSTIF_RESP_67_RESP_HI_MASK                      0xffff0000
#define EMMC_HOSTIF_RESP_67_RESP_HI_SHIFT                     16
#define EMMC_HOSTIF_RESP_67_RESP_HI_DEFAULT                   0x00000000

/* EMMC_HOSTIF :: RESP_67 :: RESP_LO [15:00] */
#define EMMC_HOSTIF_RESP_67_RESP_LO_MASK                      0x0000ffff
#define EMMC_HOSTIF_RESP_67_RESP_LO_SHIFT                     0
#define EMMC_HOSTIF_RESP_67_RESP_LO_DEFAULT                   0x00000000
   
   uint32 emmc_host_buffdata;              /* 0x20 Buffer Data Port for PIO Tranfers                               */
/***************************************************************************
 *BUFFDATA - Buffer Data Port for PIO Tranfers
 ***************************************************************************/
/* EMMC_HOSTIF :: BUFFDATA :: PORT [31:00] */
#define EMMC_HOSTIF_BUFFDATA_PORT_MASK                        0xffffffff
#define EMMC_HOSTIF_BUFFDATA_PORT_SHIFT                       0
#define EMMC_HOSTIF_BUFFDATA_PORT_DEFAULT                     0x00000000

   
   uint32 emmc_host_state;                 /* 0x24 Present State of Controller                                     */
/***************************************************************************
 *STATE - Present State of Controller
 ***************************************************************************/
/* EMMC_HOSTIF :: STATE :: reserved0 [31:29] */
#define EMMC_HOSTIF_STATE_reserved0_MASK                      0xe0000000
#define EMMC_HOSTIF_STATE_reserved0_SHIFT                     29

/* EMMC_HOSTIF :: STATE :: LINE_7TO4 [28:25] */
#define EMMC_HOSTIF_STATE_LINE_7TO4_MASK                      0x1e000000
#define EMMC_HOSTIF_STATE_LINE_7TO4_SHIFT                     25
#define EMMC_HOSTIF_STATE_LINE_7TO4_DEFAULT                   0x0000000f

/* EMMC_HOSTIF :: STATE :: LINE_CMD [24:24] */
#define EMMC_HOSTIF_STATE_LINE_CMD_MASK                       0x01000000
#define EMMC_HOSTIF_STATE_LINE_CMD_SHIFT                      24
#define EMMC_HOSTIF_STATE_LINE_CMD_DEFAULT                    0x00000001

/* EMMC_HOSTIF :: STATE :: LINE_3TO0 [23:20] */
#define EMMC_HOSTIF_STATE_LINE_3TO0_MASK                      0x00f00000
#define EMMC_HOSTIF_STATE_LINE_3TO0_SHIFT                     20
#define EMMC_HOSTIF_STATE_LINE_3TO0_DEFAULT                   0x0000000f

/* EMMC_HOSTIF :: STATE :: WP_LEVEL [19:19] */
#define EMMC_HOSTIF_STATE_WP_LEVEL_MASK                       0x00080000
#define EMMC_HOSTIF_STATE_WP_LEVEL_SHIFT                      19

/* EMMC_HOSTIF :: STATE :: CD_LEVEL [18:18] */
#define EMMC_HOSTIF_STATE_CD_LEVEL_MASK                       0x00040000
#define EMMC_HOSTIF_STATE_CD_LEVEL_SHIFT                      18

/* EMMC_HOSTIF :: STATE :: CD_STABLE [17:17] */
#define EMMC_HOSTIF_STATE_CD_STABLE_MASK                      0x00020000
#define EMMC_HOSTIF_STATE_CD_STABLE_SHIFT                     17

/* EMMC_HOSTIF :: STATE :: CARD_INSERTED [16:16] */
#define EMMC_HOSTIF_STATE_CARD_INSERTED_MASK                  0x00010000
#define EMMC_HOSTIF_STATE_CARD_INSERTED_SHIFT                 16

/* EMMC_HOSTIF :: STATE :: reserved1 [15:12] */
#define EMMC_HOSTIF_STATE_reserved1_MASK                      0x0000f000
#define EMMC_HOSTIF_STATE_reserved1_SHIFT                     12
   /* EMMC_HOSTIF :: STATE :: BUFF_RDEN [11:11] */
#define EMMC_HOSTIF_STATE_BUFF_RDEN_MASK                      0x00000800
#define EMMC_HOSTIF_STATE_BUFF_RDEN_SHIFT                     11
#define EMMC_HOSTIF_STATE_BUFF_RDEN_DEFAULT                   0x00000000

/* EMMC_HOSTIF :: STATE :: BUFF_WREN [10:10] */
#define EMMC_HOSTIF_STATE_BUFF_WREN_MASK                      0x00000400
#define EMMC_HOSTIF_STATE_BUFF_WREN_SHIFT                     10
#define EMMC_HOSTIF_STATE_BUFF_WREN_DEFAULT                   0x00000000

/* EMMC_HOSTIF :: STATE :: RD_ACTIVE [09:09] */
#define EMMC_HOSTIF_STATE_RD_ACTIVE_MASK                      0x00000200
#define EMMC_HOSTIF_STATE_RD_ACTIVE_SHIFT                     9
#define EMMC_HOSTIF_STATE_RD_ACTIVE_DEFAULT                   0x00000000

/* EMMC_HOSTIF :: STATE :: WR_ACTIVE [08:08] */
#define EMMC_HOSTIF_STATE_WR_ACTIVE_MASK                      0x00000100
#define EMMC_HOSTIF_STATE_WR_ACTIVE_SHIFT                     8
#define EMMC_HOSTIF_STATE_WR_ACTIVE_DEFAULT                   0x00000000

/* EMMC_HOSTIF :: STATE :: reserved2 [07:04] */
#define EMMC_HOSTIF_STATE_reserved2_MASK                      0x000000f0
#define EMMC_HOSTIF_STATE_reserved2_SHIFT                     4

/* EMMC_HOSTIF :: STATE :: RE_TUNING_REQUEST [03:03] */
#define EMMC_HOSTIF_STATE_RE_TUNING_REQUEST_MASK              0x00000008
#define EMMC_HOSTIF_STATE_RE_TUNING_REQUEST_SHIFT             3
#define EMMC_HOSTIF_STATE_RE_TUNING_REQUEST_DEFAULT           0x00000000

/* EMMC_HOSTIF :: STATE :: DAT_ACTIVE [02:02] */
#define EMMC_HOSTIF_STATE_DAT_ACTIVE_MASK                     0x00000004
#define EMMC_HOSTIF_STATE_DAT_ACTIVE_SHIFT                    2
#define EMMC_HOSTIF_STATE_DAT_ACTIVE_DEFAULT                  0x00000000

/* EMMC_HOSTIF :: STATE :: CMD_INHIBIT_DAT [01:01] */
#define EMMC_HOSTIF_STATE_CMD_INHIBIT_DAT_MASK                0x00000002
#define EMMC_HOSTIF_STATE_CMD_INHIBIT_DAT_SHIFT               1
#define EMMC_HOSTIF_STATE_CMD_INHIBIT_DAT_DEFAULT             0x00000000

/* EMMC_HOSTIF :: STATE :: CMD_INHIBIT_CMD [00:00] */
#define EMMC_HOSTIF_STATE_CMD_INHIBIT_CMD_MASK                0x00000001
#define EMMC_HOSTIF_STATE_CMD_INHIBIT_CMD_SHIFT               0
#define EMMC_HOSTIF_STATE_CMD_INHIBIT_CMD_DEFAULT             0x00000000

   uint32 emmc_host_ctrl_set0;             /* 0x28 SD Standard Control Registers for Host, Power, BlockGap, WakeUp */
/***************************************************************************
 *CTRL_SET0 - SD Standard Control Registers for Host, Power, BlockGap, WakeUp
 ***************************************************************************/
/* EMMC_HOSTIF :: CTRL_SET0 :: reserved0 [31:27] */
#define EMMC_HOSTIF_CTRL_SET0_reserved0_MASK                  0xf8000000
#define EMMC_HOSTIF_CTRL_SET0_reserved0_SHIFT                 27

/* EMMC_HOSTIF :: CTRL_SET0 :: WAKE_ON_REMOVAL [26:26] */
#define EMMC_HOSTIF_CTRL_SET0_WAKE_ON_REMOVAL_MASK            0x04000000
#define EMMC_HOSTIF_CTRL_SET0_WAKE_ON_REMOVAL_SHIFT           26
#define EMMC_HOSTIF_CTRL_SET0_WAKE_ON_REMOVAL_DEFAULT         0x00000000

/* EMMC_HOSTIF :: CTRL_SET0 :: WAKE_ON_INSERTION [25:25] */
#define EMMC_HOSTIF_CTRL_SET0_WAKE_ON_INSERTION_MASK          0x02000000
#define EMMC_HOSTIF_CTRL_SET0_WAKE_ON_INSERTION_SHIFT         25
#define EMMC_HOSTIF_CTRL_SET0_WAKE_ON_INSERTION_DEFAULT       0x00000000

/* EMMC_HOSTIF :: CTRL_SET0 :: WAKE_ON_INTERRUPT [24:24] */
#define EMMC_HOSTIF_CTRL_SET0_WAKE_ON_INTERRUPT_MASK          0x01000000
#define EMMC_HOSTIF_CTRL_SET0_WAKE_ON_INTERRUPT_SHIFT         24
#define EMMC_HOSTIF_CTRL_SET0_WAKE_ON_INTERRUPT_DEFAULT       0x00000000

/* EMMC_HOSTIF :: CTRL_SET0 :: BOOT_ACK [23:23] */
#define EMMC_HOSTIF_CTRL_SET0_BOOT_ACK_MASK                   0x00800000
#define EMMC_HOSTIF_CTRL_SET0_BOOT_ACK_SHIFT                  23
#define EMMC_HOSTIF_CTRL_SET0_BOOT_ACK_DEFAULT                0x00000000

/* EMMC_HOSTIF :: CTRL_SET0 :: ALT_BOOT_EN [22:22] */
#define EMMC_HOSTIF_CTRL_SET0_ALT_BOOT_EN_MASK                0x00400000
#define EMMC_HOSTIF_CTRL_SET0_ALT_BOOT_EN_SHIFT               22
#define EMMC_HOSTIF_CTRL_SET0_ALT_BOOT_EN_DEFAULT             0x00000000

/* EMMC_HOSTIF :: CTRL_SET0 :: BOOT_EN [21:21] */
#define EMMC_HOSTIF_CTRL_SET0_BOOT_EN_MASK                    0x00200000
#define EMMC_HOSTIF_CTRL_SET0_BOOT_EN_SHIFT                   21
#define EMMC_HOSTIF_CTRL_SET0_BOOT_EN_DEFAULT                 0x00000000

/* EMMC_HOSTIF :: CTRL_SET0 :: SPI_MODE [20:20] */
#define EMMC_HOSTIF_CTRL_SET0_SPI_MODE_MASK                   0x00100000
#define EMMC_HOSTIF_CTRL_SET0_SPI_MODE_SHIFT                  20
#define EMMC_HOSTIF_CTRL_SET0_SPI_MODE_DEFAULT                0x00000000

/* EMMC_HOSTIF :: CTRL_SET0 :: INT_AT_BLOCK_GAP [19:19] */
#define EMMC_HOSTIF_CTRL_SET0_INT_AT_BLOCK_GAP_MASK           0x00080000
#define EMMC_HOSTIF_CTRL_SET0_INT_AT_BLOCK_GAP_SHIFT          19
#define EMMC_HOSTIF_CTRL_SET0_INT_AT_BLOCK_GAP_DEFAULT        0x00000000

/* EMMC_HOSTIF :: CTRL_SET0 :: READ_WAIT_CTRL [18:18] */
#define EMMC_HOSTIF_CTRL_SET0_READ_WAIT_CTRL_MASK             0x00040000
#define EMMC_HOSTIF_CTRL_SET0_READ_WAIT_CTRL_SHIFT            18
#define EMMC_HOSTIF_CTRL_SET0_READ_WAIT_CTRL_DEFAULT          0x00000000

/* EMMC_HOSTIF :: CTRL_SET0 :: CONTINUE_REQUESTS [17:17] */
#define EMMC_HOSTIF_CTRL_SET0_CONTINUE_REQUESTS_MASK          0x00020000
#define EMMC_HOSTIF_CTRL_SET0_CONTINUE_REQUESTS_SHIFT         17
#define EMMC_HOSTIF_CTRL_SET0_CONTINUE_REQUESTS_DEFAULT       0x00000000

/* EMMC_HOSTIF :: CTRL_SET0 :: STOP_AT_BLOCK_GAP [16:16] */
#define EMMC_HOSTIF_CTRL_SET0_STOP_AT_BLOCK_GAP_MASK          0x00010000
#define EMMC_HOSTIF_CTRL_SET0_STOP_AT_BLOCK_GAP_SHIFT         16
#define EMMC_HOSTIF_CTRL_SET0_STOP_AT_BLOCK_GAP_DEFAULT       0x00000000

/* EMMC_HOSTIF :: CTRL_SET0 :: reserved1 [15:13] */
#define EMMC_HOSTIF_CTRL_SET0_reserved1_MASK                  0x0000e000
#define EMMC_HOSTIF_CTRL_SET0_reserved1_SHIFT                 13

/* EMMC_HOSTIF :: CTRL_SET0 :: HW_RESET [12:12] */
#define EMMC_HOSTIF_CTRL_SET0_HW_RESET_MASK                   0x00001000
#define EMMC_HOSTIF_CTRL_SET0_HW_RESET_SHIFT                  12
#define EMMC_HOSTIF_CTRL_SET0_HW_RESET_DEFAULT                0x00000000

/* EMMC_HOSTIF :: CTRL_SET0 :: SD_BUS_VOLTAGE_SELECT [11:09] */
#define EMMC_HOSTIF_CTRL_SET0_SD_BUS_VOLTAGE_SELECT_MASK      0x00000e00
#define EMMC_HOSTIF_CTRL_SET0_SD_BUS_VOLTAGE_SELECT_SHIFT     9
#define EMMC_HOSTIF_CTRL_SET0_SD_BUS_VOLTAGE_SELECT_DEFAULT   0x00000000

/* EMMC_HOSTIF :: CTRL_SET0 :: SD_BUS_POWER [08:08] */
#define EMMC_HOSTIF_CTRL_SET0_SD_BUS_POWER_MASK               0x00000100
#define EMMC_HOSTIF_CTRL_SET0_SD_BUS_POWER_SHIFT              8
#define EMMC_HOSTIF_CTRL_SET0_SD_BUS_POWER_DEFAULT            0x00000000

/* EMMC_HOSTIF :: CTRL_SET0 :: CARD_DETECT_SELECT [07:07] */
#define EMMC_HOSTIF_CTRL_SET0_CARD_DETECT_SELECT_MASK         0x00000080
#define EMMC_HOSTIF_CTRL_SET0_CARD_DETECT_SELECT_SHIFT        7
#define EMMC_HOSTIF_CTRL_SET0_CARD_DETECT_SELECT_DEFAULT      0x00000000

/* EMMC_HOSTIF :: CTRL_SET0 :: CARD_DETECT_TEST [06:06] */
#define EMMC_HOSTIF_CTRL_SET0_CARD_DETECT_TEST_MASK           0x00000040
#define EMMC_HOSTIF_CTRL_SET0_CARD_DETECT_TEST_SHIFT          6
#define EMMC_HOSTIF_CTRL_SET0_CARD_DETECT_TEST_DEFAULT        0x00000000

/* EMMC_HOSTIF :: CTRL_SET0 :: SD_8BIT_MODE [05:05] */
#define EMMC_HOSTIF_CTRL_SET0_SD_8BIT_MODE_MASK               0x00000020
#define EMMC_HOSTIF_CTRL_SET0_SD_8BIT_MODE_SHIFT              5
#define EMMC_HOSTIF_CTRL_SET0_SD_8BIT_MODE_DEFAULT            0x00000000

/* EMMC_HOSTIF :: CTRL_SET0 :: DMA_SELECT [04:03] */
#define EMMC_HOSTIF_CTRL_SET0_DMA_SELECT_MASK                 0x00000018
#define EMMC_HOSTIF_CTRL_SET0_DMA_SELECT_SHIFT                3
#define EMMC_HOSTIF_CTRL_SET0_DMA_SELECT_DEFAULT              0x00000000

/* EMMC_HOSTIF :: CTRL_SET0 :: HIGH_SPEED_ENABLE [02:02] */
#define EMMC_HOSTIF_CTRL_SET0_HIGH_SPEED_ENABLE_MASK          0x00000004
#define EMMC_HOSTIF_CTRL_SET0_HIGH_SPEED_ENABLE_SHIFT         2
#define EMMC_HOSTIF_CTRL_SET0_HIGH_SPEED_ENABLE_DEFAULT       0x00000000

/* EMMC_HOSTIF :: CTRL_SET0 :: SD_4BIT_MODE [01:01] */
#define EMMC_HOSTIF_CTRL_SET0_SD_4BIT_MODE_MASK               0x00000002
#define EMMC_HOSTIF_CTRL_SET0_SD_4BIT_MODE_SHIFT              1
#define EMMC_HOSTIF_CTRL_SET0_SD_4BIT_MODE_DEFAULT            0x00000000

/* EMMC_HOSTIF :: CTRL_SET0 :: LED_CONTROL [00:00] */
#define EMMC_HOSTIF_CTRL_SET0_LED_CONTROL_MASK                0x00000001
#define EMMC_HOSTIF_CTRL_SET0_LED_CONTROL_SHIFT               0
#define EMMC_HOSTIF_CTRL_SET0_LED_CONTROL_DEFAULT             0x00000000
   
   uint32 emmc_host_ctrl_set1;             /* 0x2c SD Standard Control Registers for Clock, Timeout, Resets        */
/***************************************************************************
 *CTRL_SET1 - SD Standard Control Registers for Clock, Timeout, Resets
 ***************************************************************************/
/* EMMC_HOSTIF :: CTRL_SET1 :: reserved0 [31:27] */
#define EMMC_HOSTIF_CTRL_SET1_reserved0_MASK                  0xf8000000
#define EMMC_HOSTIF_CTRL_SET1_reserved0_SHIFT                 27

/* EMMC_HOSTIF :: CTRL_SET1 :: SOFT_RESET_DAT [26:26] */
#define EMMC_HOSTIF_CTRL_SET1_SOFT_RESET_DAT_MASK             0x04000000
#define EMMC_HOSTIF_CTRL_SET1_SOFT_RESET_DAT_SHIFT            26
#define EMMC_HOSTIF_CTRL_SET1_SOFT_RESET_DAT_DEFAULT          0x00000000

/* EMMC_HOSTIF :: CTRL_SET1 :: SOFT_RESET_CMD [25:25] */
#define EMMC_HOSTIF_CTRL_SET1_SOFT_RESET_CMD_MASK             0x02000000
#define EMMC_HOSTIF_CTRL_SET1_SOFT_RESET_CMD_SHIFT            25
#define EMMC_HOSTIF_CTRL_SET1_SOFT_RESET_CMD_DEFAULT          0x00000000

/* EMMC_HOSTIF :: CTRL_SET1 :: SOFT_RESET_CORE [24:24] */
#define EMMC_HOSTIF_CTRL_SET1_SOFT_RESET_CORE_MASK            0x01000000
#define EMMC_HOSTIF_CTRL_SET1_SOFT_RESET_CORE_SHIFT           24
#define EMMC_HOSTIF_CTRL_SET1_SOFT_RESET_CORE_DEFAULT         0x00000000

/* EMMC_HOSTIF :: CTRL_SET1 :: reserved1 [23:20] */
#define EMMC_HOSTIF_CTRL_SET1_reserved1_MASK                  0x00f00000
#define EMMC_HOSTIF_CTRL_SET1_reserved1_SHIFT                 20

/* EMMC_HOSTIF :: CTRL_SET1 :: TIMEOUT_COUNT [19:16] */
#define EMMC_HOSTIF_CTRL_SET1_TIMEOUT_COUNT_MASK              0x000f0000
#define EMMC_HOSTIF_CTRL_SET1_TIMEOUT_COUNT_SHIFT             16
#define EMMC_HOSTIF_CTRL_SET1_TIMEOUT_COUNT_DEFAULT           0x00000000

/* EMMC_HOSTIF :: CTRL_SET1 :: FREQ_CTRL [15:08] */
#define EMMC_HOSTIF_CTRL_SET1_FREQ_CTRL_MASK                  0x0000ff00
#define EMMC_HOSTIF_CTRL_SET1_FREQ_CTRL_SHIFT                 8
#define EMMC_HOSTIF_CTRL_SET1_FREQ_CTRL_DEFAULT               0x00000000

/* EMMC_HOSTIF :: CTRL_SET1 :: MS_CLK_FREQ [07:06] */
#define EMMC_HOSTIF_CTRL_SET1_MS_CLK_FREQ_MASK                0x000000c0
#define EMMC_HOSTIF_CTRL_SET1_MS_CLK_FREQ_SHIFT               6
#define EMMC_HOSTIF_CTRL_SET1_MS_CLK_FREQ_DEFAULT             0x00000000

/* EMMC_HOSTIF :: CTRL_SET1 :: CLK_GEN_SEL [05:05] */
#define EMMC_HOSTIF_CTRL_SET1_CLK_GEN_SEL_MASK                0x00000020
#define EMMC_HOSTIF_CTRL_SET1_CLK_GEN_SEL_SHIFT               5
#define EMMC_HOSTIF_CTRL_SET1_CLK_GEN_SEL_DEFAULT             0x00000000

/* EMMC_HOSTIF :: CTRL_SET1 :: reserved2 [04:03] */
#define EMMC_HOSTIF_CTRL_SET1_reserved2_MASK                  0x00000018
#define EMMC_HOSTIF_CTRL_SET1_reserved2_SHIFT                 3

/* EMMC_HOSTIF :: CTRL_SET1 :: SD_CLK_ENA [02:02] */
#define EMMC_HOSTIF_CTRL_SET1_SD_CLK_ENA_MASK                 0x00000004
#define EMMC_HOSTIF_CTRL_SET1_SD_CLK_ENA_SHIFT                2
#define EMMC_HOSTIF_CTRL_SET1_SD_CLK_ENA_DEFAULT              0x00000000

/* EMMC_HOSTIF :: CTRL_SET1 :: INTERNAL_CLK_STABLE [01:01] */
#define EMMC_HOSTIF_CTRL_SET1_INTERNAL_CLK_STABLE_MASK        0x00000002
#define EMMC_HOSTIF_CTRL_SET1_INTERNAL_CLK_STABLE_SHIFT       1
#define EMMC_HOSTIF_CTRL_SET1_INTERNAL_CLK_STABLE_DEFAULT     0x00000000

/* EMMC_HOSTIF :: CTRL_SET1 :: INTERNAL_CLK_ENA [00:00] */
#define EMMC_HOSTIF_CTRL_SET1_INTERNAL_CLK_ENA_MASK           0x00000001
#define EMMC_HOSTIF_CTRL_SET1_INTERNAL_CLK_ENA_SHIFT          0
#define EMMC_HOSTIF_CTRL_SET1_INTERNAL_CLK_ENA_DEFAULT        0x00000000

   uint32 emmc_host_int_status;            /* 0x30 Interrupt Status for Normal and Error conditions                */
/***************************************************************************
 *INT_STATUS - Interrupt Status for Normal and Error conditions
 ***************************************************************************/
/* EMMC_HOSTIF :: INT_STATUS :: reserved0 [31:29] */
#define EMMC_HOSTIF_INT_STATUS_reserved0_MASK                 0xe0000000
#define EMMC_HOSTIF_INT_STATUS_reserved0_SHIFT                29

/* EMMC_HOSTIF :: INT_STATUS :: TARGET_RESP_ERR_INT [28:28] */
#define EMMC_HOSTIF_INT_STATUS_TARGET_RESP_ERR_INT_MASK       0x10000000
#define EMMC_HOSTIF_INT_STATUS_TARGET_RESP_ERR_INT_SHIFT      28
#define EMMC_HOSTIF_INT_STATUS_TARGET_RESP_ERR_INT_DEFAULT    0x00000000

/* EMMC_HOSTIF :: INT_STATUS :: reserved1 [27:27] */
#define EMMC_HOSTIF_INT_STATUS_reserved1_MASK                 0x08000000
#define EMMC_HOSTIF_INT_STATUS_reserved1_SHIFT                27

/* EMMC_HOSTIF :: INT_STATUS :: TUNE_ERR [26:26] */
#define EMMC_HOSTIF_INT_STATUS_TUNE_ERR_MASK                  0x04000000
#define EMMC_HOSTIF_INT_STATUS_TUNE_ERR_SHIFT                 26
#define EMMC_HOSTIF_INT_STATUS_TUNE_ERR_DEFAULT               0x00000000

/* EMMC_HOSTIF :: INT_STATUS :: ADMA_ERR_INT [25:25] */
#define EMMC_HOSTIF_INT_STATUS_ADMA_ERR_INT_MASK              0x02000000
#define EMMC_HOSTIF_INT_STATUS_ADMA_ERR_INT_SHIFT             25
#define EMMC_HOSTIF_INT_STATUS_ADMA_ERR_INT_DEFAULT           0x00000000

/* EMMC_HOSTIF :: INT_STATUS :: AUTO_CMD_ERR_INT [24:24] */
#define EMMC_HOSTIF_INT_STATUS_AUTO_CMD_ERR_INT_MASK          0x01000000
#define EMMC_HOSTIF_INT_STATUS_AUTO_CMD_ERR_INT_SHIFT         24
#define EMMC_HOSTIF_INT_STATUS_AUTO_CMD_ERR_INT_DEFAULT       0x00000000

/* EMMC_HOSTIF :: INT_STATUS :: CURRENT_LIMIT_ERR_INT [23:23] */
#define EMMC_HOSTIF_INT_STATUS_CURRENT_LIMIT_ERR_INT_MASK     0x00800000
#define EMMC_HOSTIF_INT_STATUS_CURRENT_LIMIT_ERR_INT_SHIFT    23
#define EMMC_HOSTIF_INT_STATUS_CURRENT_LIMIT_ERR_INT_DEFAULT  0x00000000

/* EMMC_HOSTIF :: INT_STATUS :: DATA_END_BIT_ERR_INT [22:22] */
#define EMMC_HOSTIF_INT_STATUS_DATA_END_BIT_ERR_INT_MASK      0x00400000
#define EMMC_HOSTIF_INT_STATUS_DATA_END_BIT_ERR_INT_SHIFT     22
#define EMMC_HOSTIF_INT_STATUS_DATA_END_BIT_ERR_INT_DEFAULT   0x00000000

/* EMMC_HOSTIF :: INT_STATUS :: DATA_CRC_ERR_INT [21:21] */
#define EMMC_HOSTIF_INT_STATUS_DATA_CRC_ERR_INT_MASK          0x00200000
#define EMMC_HOSTIF_INT_STATUS_DATA_CRC_ERR_INT_SHIFT         21
#define EMMC_HOSTIF_INT_STATUS_DATA_CRC_ERR_INT_DEFAULT       0x00000000

/* EMMC_HOSTIF :: INT_STATUS :: DATA_TIMEOUT_ERR_INT [20:20] */
#define EMMC_HOSTIF_INT_STATUS_DATA_TIMEOUT_ERR_INT_MASK      0x00100000
#define EMMC_HOSTIF_INT_STATUS_DATA_TIMEOUT_ERR_INT_SHIFT     20
#define EMMC_HOSTIF_INT_STATUS_DATA_TIMEOUT_ERR_INT_DEFAULT   0x00000000

/* EMMC_HOSTIF :: INT_STATUS :: CMD_INDEX_ERR_INT [19:19] */
#define EMMC_HOSTIF_INT_STATUS_CMD_INDEX_ERR_INT_MASK         0x00080000
#define EMMC_HOSTIF_INT_STATUS_CMD_INDEX_ERR_INT_SHIFT        19
#define EMMC_HOSTIF_INT_STATUS_CMD_INDEX_ERR_INT_DEFAULT      0x00000000

/* EMMC_HOSTIF :: INT_STATUS :: CMD_END_BIT_ERR_INT [18:18] */
#define EMMC_HOSTIF_INT_STATUS_CMD_END_BIT_ERR_INT_MASK       0x00040000
#define EMMC_HOSTIF_INT_STATUS_CMD_END_BIT_ERR_INT_SHIFT      18
#define EMMC_HOSTIF_INT_STATUS_CMD_END_BIT_ERR_INT_DEFAULT    0x00000000

/* EMMC_HOSTIF :: INT_STATUS :: CMD_CRC_ERR_INT [17:17] */
#define EMMC_HOSTIF_INT_STATUS_CMD_CRC_ERR_INT_MASK           0x00020000
#define EMMC_HOSTIF_INT_STATUS_CMD_CRC_ERR_INT_SHIFT          17
#define EMMC_HOSTIF_INT_STATUS_CMD_CRC_ERR_INT_DEFAULT        0x00000000

/* EMMC_HOSTIF :: INT_STATUS :: CMD_TIMEOUT_ERR_INT [16:16] */
#define EMMC_HOSTIF_INT_STATUS_CMD_TIMEOUT_ERR_INT_MASK       0x00010000
#define EMMC_HOSTIF_INT_STATUS_CMD_TIMEOUT_ERR_INT_SHIFT      16
#define EMMC_HOSTIF_INT_STATUS_CMD_TIMEOUT_ERR_INT_DEFAULT    0x00000000

/* EMMC_HOSTIF :: INT_STATUS :: ERROR_INT [15:15] */
#define EMMC_HOSTIF_INT_STATUS_ERROR_INT_MASK                 0x00008000
#define EMMC_HOSTIF_INT_STATUS_ERROR_INT_SHIFT                15
#define EMMC_HOSTIF_INT_STATUS_ERROR_INT_DEFAULT              0x00000000

/* EMMC_HOSTIF :: INT_STATUS :: BOOT_TERM_INT [14:14] */
#define EMMC_HOSTIF_INT_STATUS_BOOT_TERM_INT_MASK             0x00004000
#define EMMC_HOSTIF_INT_STATUS_BOOT_TERM_INT_SHIFT            14
#define EMMC_HOSTIF_INT_STATUS_BOOT_TERM_INT_DEFAULT          0x00000000

/* EMMC_HOSTIF :: INT_STATUS :: BOOT_ACK_RCV_INT [13:13] */
#define EMMC_HOSTIF_INT_STATUS_BOOT_ACK_RCV_INT_MASK          0x00002000
#define EMMC_HOSTIF_INT_STATUS_BOOT_ACK_RCV_INT_SHIFT         13
#define EMMC_HOSTIF_INT_STATUS_BOOT_ACK_RCV_INT_DEFAULT       0x00000000

/* EMMC_HOSTIF :: INT_STATUS :: RETUNE_EVENT [12:12] */
#define EMMC_HOSTIF_INT_STATUS_RETUNE_EVENT_MASK              0x00001000
#define EMMC_HOSTIF_INT_STATUS_RETUNE_EVENT_SHIFT             12
#define EMMC_HOSTIF_INT_STATUS_RETUNE_EVENT_DEFAULT           0x00000000

/* EMMC_HOSTIF :: INT_STATUS :: INT_C [11:11] */
#define EMMC_HOSTIF_INT_STATUS_INT_C_MASK                     0x00000800
#define EMMC_HOSTIF_INT_STATUS_INT_C_SHIFT                    11
#define EMMC_HOSTIF_INT_STATUS_INT_C_DEFAULT                  0x00000000

/* EMMC_HOSTIF :: INT_STATUS :: INT_B [10:10] */
#define EMMC_HOSTIF_INT_STATUS_INT_B_MASK                     0x00000400
#define EMMC_HOSTIF_INT_STATUS_INT_B_SHIFT                    10
#define EMMC_HOSTIF_INT_STATUS_INT_B_DEFAULT                  0x00000000

/* EMMC_HOSTIF :: INT_STATUS :: INT_A [09:09] */
#define EMMC_HOSTIF_INT_STATUS_INT_A_MASK                     0x00000200
#define EMMC_HOSTIF_INT_STATUS_INT_A_SHIFT                    9
#define EMMC_HOSTIF_INT_STATUS_INT_A_DEFAULT                  0x00000000

/* EMMC_HOSTIF :: INT_STATUS :: CARD_INT [08:08] */
#define EMMC_HOSTIF_INT_STATUS_CARD_INT_MASK                  0x00000100
#define EMMC_HOSTIF_INT_STATUS_CARD_INT_SHIFT                 8
#define EMMC_HOSTIF_INT_STATUS_CARD_INT_DEFAULT               0x00000000

/* EMMC_HOSTIF :: INT_STATUS :: CAR_REMOVAL_INT [07:07] */
#define EMMC_HOSTIF_INT_STATUS_CAR_REMOVAL_INT_MASK           0x00000080
#define EMMC_HOSTIF_INT_STATUS_CAR_REMOVAL_INT_SHIFT          7
#define EMMC_HOSTIF_INT_STATUS_CAR_REMOVAL_INT_DEFAULT        0x00000000

/* EMMC_HOSTIF :: INT_STATUS :: CAR_INSERT_INT [06:06] */
#define EMMC_HOSTIF_INT_STATUS_CAR_INSERT_INT_MASK            0x00000040
#define EMMC_HOSTIF_INT_STATUS_CAR_INSERT_INT_SHIFT           6
#define EMMC_HOSTIF_INT_STATUS_CAR_INSERT_INT_DEFAULT         0x00000000

/* EMMC_HOSTIF :: INT_STATUS :: BUFFER_READ_INT [05:05] */
#define EMMC_HOSTIF_INT_STATUS_BUFFER_READ_INT_MASK           0x00000020
#define EMMC_HOSTIF_INT_STATUS_BUFFER_READ_INT_SHIFT          5
#define EMMC_HOSTIF_INT_STATUS_BUFFER_READ_INT_DEFAULT        0x00000000

/* EMMC_HOSTIF :: INT_STATUS :: BUFFER_WRITE_INT [04:04] */
#define EMMC_HOSTIF_INT_STATUS_BUFFER_WRITE_INT_MASK          0x00000010
#define EMMC_HOSTIF_INT_STATUS_BUFFER_WRITE_INT_SHIFT         4
#define EMMC_HOSTIF_INT_STATUS_BUFFER_WRITE_INT_DEFAULT       0x00000000

/* EMMC_HOSTIF :: INT_STATUS :: DMA_INT [03:03] */
#define EMMC_HOSTIF_INT_STATUS_DMA_INT_MASK                   0x00000008
#define EMMC_HOSTIF_INT_STATUS_DMA_INT_SHIFT                  3
#define EMMC_HOSTIF_INT_STATUS_DMA_INT_DEFAULT                0x00000000

/* EMMC_HOSTIF :: INT_STATUS :: BLOCK_GAP_INT [02:02] */
#define EMMC_HOSTIF_INT_STATUS_BLOCK_GAP_INT_MASK             0x00000004
#define EMMC_HOSTIF_INT_STATUS_BLOCK_GAP_INT_SHIFT            2
#define EMMC_HOSTIF_INT_STATUS_BLOCK_GAP_INT_DEFAULT          0x00000000

/* EMMC_HOSTIF :: INT_STATUS :: TRANSFER_COMPLETE_INT [01:01] */
#define EMMC_HOSTIF_INT_STATUS_TRANSFER_COMPLETE_INT_MASK     0x00000002
#define EMMC_HOSTIF_INT_STATUS_TRANSFER_COMPLETE_INT_SHIFT    1
#define EMMC_HOSTIF_INT_STATUS_TRANSFER_COMPLETE_INT_DEFAULT  0x00000000

/* EMMC_HOSTIF :: INT_STATUS :: COMMAND_COMPLETE_INT [00:00] */
#define EMMC_HOSTIF_INT_STATUS_COMMAND_COMPLETE_INT_MASK      0x00000001
#define EMMC_HOSTIF_INT_STATUS_COMMAND_COMPLETE_INT_SHIFT     0
#define EMMC_HOSTIF_INT_STATUS_COMMAND_COMPLETE_INT_DEFAULT   0x00000000

   uint32 emmc_host_int_status_ena;        /* 0x34 Interrupt Enables for Normal and Error conditions               */
/***************************************************************************
 *INT_STATUS_ENA - Interrupt Enables for Normal and Error conditions
 ***************************************************************************/
/* EMMC_HOSTIF :: INT_STATUS_ENA :: reserved0 [31:30] */
#define EMMC_HOSTIF_INT_STATUS_ENA_reserved0_MASK             0xc0000000
#define EMMC_HOSTIF_INT_STATUS_ENA_reserved0_SHIFT            30

/* EMMC_HOSTIF :: INT_STATUS_ENA :: reserved_for_eco1 [29:29] */
#define EMMC_HOSTIF_INT_STATUS_ENA_reserved_for_eco1_MASK     0x20000000
#define EMMC_HOSTIF_INT_STATUS_ENA_reserved_for_eco1_SHIFT    29
#define EMMC_HOSTIF_INT_STATUS_ENA_reserved_for_eco1_DEFAULT  0x00000000

/* EMMC_HOSTIF :: INT_STATUS_ENA :: TARGET_RESP_ERR_INT_ENA [28:28] */
#define EMMC_HOSTIF_INT_STATUS_ENA_TARGET_RESP_ERR_INT_ENA_MASK 0x10000000
#define EMMC_HOSTIF_INT_STATUS_ENA_TARGET_RESP_ERR_INT_ENA_SHIFT 28
#define EMMC_HOSTIF_INT_STATUS_ENA_TARGET_RESP_ERR_INT_ENA_DEFAULT 0x00000000

/* EMMC_HOSTIF :: INT_STATUS_ENA :: reserved2 [27:27] */
#define EMMC_HOSTIF_INT_STATUS_ENA_reserved2_MASK             0x08000000
#define EMMC_HOSTIF_INT_STATUS_ENA_reserved2_SHIFT            27

/* EMMC_HOSTIF :: INT_STATUS_ENA :: TUNE_ERR_STAT_EN [26:26] */
#define EMMC_HOSTIF_INT_STATUS_ENA_TUNE_ERR_STAT_EN_MASK      0x04000000
#define EMMC_HOSTIF_INT_STATUS_ENA_TUNE_ERR_STAT_EN_SHIFT     26
#define EMMC_HOSTIF_INT_STATUS_ENA_TUNE_ERR_STAT_EN_DEFAULT   0x00000000

/* EMMC_HOSTIF :: INT_STATUS_ENA :: ADMA_ERR_INT_ENA [25:25] */
#define EMMC_HOSTIF_INT_STATUS_ENA_ADMA_ERR_INT_ENA_MASK      0x02000000
#define EMMC_HOSTIF_INT_STATUS_ENA_ADMA_ERR_INT_ENA_SHIFT     25
#define EMMC_HOSTIF_INT_STATUS_ENA_ADMA_ERR_INT_ENA_DEFAULT   0x00000000

/* EMMC_HOSTIF :: INT_STATUS_ENA :: AUTO_CMD12_ERR_INT_ENA [24:24] */
#define EMMC_HOSTIF_INT_STATUS_ENA_AUTO_CMD12_ERR_INT_ENA_MASK 0x01000000
#define EMMC_HOSTIF_INT_STATUS_ENA_AUTO_CMD12_ERR_INT_ENA_SHIFT 24
#define EMMC_HOSTIF_INT_STATUS_ENA_AUTO_CMD12_ERR_INT_ENA_DEFAULT 0x00000000

/* EMMC_HOSTIF :: INT_STATUS_ENA :: CURRENT_LIMIT_ERR_INT_ENA [23:23] */
#define EMMC_HOSTIF_INT_STATUS_ENA_CURRENT_LIMIT_ERR_INT_ENA_MASK 0x00800000
#define EMMC_HOSTIF_INT_STATUS_ENA_CURRENT_LIMIT_ERR_INT_ENA_SHIFT 23
#define EMMC_HOSTIF_INT_STATUS_ENA_CURRENT_LIMIT_ERR_INT_ENA_DEFAULT 0x00000000

/* EMMC_HOSTIF :: INT_STATUS_ENA :: DATA_END_BIT_ERR_INT_ENA [22:22] */
#define EMMC_HOSTIF_INT_STATUS_ENA_DATA_END_BIT_ERR_INT_ENA_MASK 0x00400000
#define EMMC_HOSTIF_INT_STATUS_ENA_DATA_END_BIT_ERR_INT_ENA_SHIFT 22
#define EMMC_HOSTIF_INT_STATUS_ENA_DATA_END_BIT_ERR_INT_ENA_DEFAULT 0x00000000

/* EMMC_HOSTIF :: INT_STATUS_ENA :: DATA_CRC_ERR_INT_ENA [21:21] */
#define EMMC_HOSTIF_INT_STATUS_ENA_DATA_CRC_ERR_INT_ENA_MASK  0x00200000
#define EMMC_HOSTIF_INT_STATUS_ENA_DATA_CRC_ERR_INT_ENA_SHIFT 21
#define EMMC_HOSTIF_INT_STATUS_ENA_DATA_CRC_ERR_INT_ENA_DEFAULT 0x00000000

/* EMMC_HOSTIF :: INT_STATUS_ENA :: DATA_TIMEOUT_ERR_INT_ENA [20:20] */
#define EMMC_HOSTIF_INT_STATUS_ENA_DATA_TIMEOUT_ERR_INT_ENA_MASK 0x00100000
#define EMMC_HOSTIF_INT_STATUS_ENA_DATA_TIMEOUT_ERR_INT_ENA_SHIFT 20
#define EMMC_HOSTIF_INT_STATUS_ENA_DATA_TIMEOUT_ERR_INT_ENA_DEFAULT 0x00000000

/* EMMC_HOSTIF :: INT_STATUS_ENA :: CMD_INDEX_ERR_INT_ENA [19:19] */
#define EMMC_HOSTIF_INT_STATUS_ENA_CMD_INDEX_ERR_INT_ENA_MASK 0x00080000
#define EMMC_HOSTIF_INT_STATUS_ENA_CMD_INDEX_ERR_INT_ENA_SHIFT 19
#define EMMC_HOSTIF_INT_STATUS_ENA_CMD_INDEX_ERR_INT_ENA_DEFAULT 0x00000000

/* EMMC_HOSTIF :: INT_STATUS_ENA :: CMD_END_BIT_ERR_INT_ENA [18:18] */
#define EMMC_HOSTIF_INT_STATUS_ENA_CMD_END_BIT_ERR_INT_ENA_MASK 0x00040000
#define EMMC_HOSTIF_INT_STATUS_ENA_CMD_END_BIT_ERR_INT_ENA_SHIFT 18
#define EMMC_HOSTIF_INT_STATUS_ENA_CMD_END_BIT_ERR_INT_ENA_DEFAULT 0x00000000

/* EMMC_HOSTIF :: INT_STATUS_ENA :: CMD_CRC_ERR_INT_ENA [17:17] */
#define EMMC_HOSTIF_INT_STATUS_ENA_CMD_CRC_ERR_INT_ENA_MASK   0x00020000
#define EMMC_HOSTIF_INT_STATUS_ENA_CMD_CRC_ERR_INT_ENA_SHIFT  17
#define EMMC_HOSTIF_INT_STATUS_ENA_CMD_CRC_ERR_INT_ENA_DEFAULT 0x00000000

/* EMMC_HOSTIF :: INT_STATUS_ENA :: CMD_TIMEOUT_ERR_INT_ENA [16:16] */
#define EMMC_HOSTIF_INT_STATUS_ENA_CMD_TIMEOUT_ERR_INT_ENA_MASK 0x00010000
#define EMMC_HOSTIF_INT_STATUS_ENA_CMD_TIMEOUT_ERR_INT_ENA_SHIFT 16
#define EMMC_HOSTIF_INT_STATUS_ENA_CMD_TIMEOUT_ERR_INT_ENA_DEFAULT 0x00000000

/* EMMC_HOSTIF :: INT_STATUS_ENA :: reserved3 [15:15] */
#define EMMC_HOSTIF_INT_STATUS_ENA_reserved3_MASK             0x00008000
#define EMMC_HOSTIF_INT_STATUS_ENA_reserved3_SHIFT            15

/* EMMC_HOSTIF :: INT_STATUS_ENA :: BOOT_TERM_EN [14:14] */
#define EMMC_HOSTIF_INT_STATUS_ENA_BOOT_TERM_EN_MASK          0x00004000
#define EMMC_HOSTIF_INT_STATUS_ENA_BOOT_TERM_EN_SHIFT         14
#define EMMC_HOSTIF_INT_STATUS_ENA_BOOT_TERM_EN_DEFAULT       0x00000000

/* EMMC_HOSTIF :: INT_STATUS_ENA :: BOOT_ACK_RCV_EN [13:13] */
#define EMMC_HOSTIF_INT_STATUS_ENA_BOOT_ACK_RCV_EN_MASK       0x00002000
#define EMMC_HOSTIF_INT_STATUS_ENA_BOOT_ACK_RCV_EN_SHIFT      13
#define EMMC_HOSTIF_INT_STATUS_ENA_BOOT_ACK_RCV_EN_DEFAULT    0x00000000

/* EMMC_HOSTIF :: INT_STATUS_ENA :: RETUNE_EVENT_EN [12:12] */
#define EMMC_HOSTIF_INT_STATUS_ENA_RETUNE_EVENT_EN_MASK       0x00001000
#define EMMC_HOSTIF_INT_STATUS_ENA_RETUNE_EVENT_EN_SHIFT      12
#define EMMC_HOSTIF_INT_STATUS_ENA_RETUNE_EVENT_EN_DEFAULT    0x00000000

/* EMMC_HOSTIF :: INT_STATUS_ENA :: INT_C_EN [11:11] */
#define EMMC_HOSTIF_INT_STATUS_ENA_INT_C_EN_MASK              0x00000800
#define EMMC_HOSTIF_INT_STATUS_ENA_INT_C_EN_SHIFT             11
#define EMMC_HOSTIF_INT_STATUS_ENA_INT_C_EN_DEFAULT           0x00000000

/* EMMC_HOSTIF :: INT_STATUS_ENA :: INT_B_EN [10:10] */
#define EMMC_HOSTIF_INT_STATUS_ENA_INT_B_EN_MASK              0x00000400
#define EMMC_HOSTIF_INT_STATUS_ENA_INT_B_EN_SHIFT             10
#define EMMC_HOSTIF_INT_STATUS_ENA_INT_B_EN_DEFAULT           0x00000000

/* EMMC_HOSTIF :: INT_STATUS_ENA :: INT_A_EN [09:09] */
#define EMMC_HOSTIF_INT_STATUS_ENA_INT_A_EN_MASK              0x00000200
#define EMMC_HOSTIF_INT_STATUS_ENA_INT_A_EN_SHIFT             9
#define EMMC_HOSTIF_INT_STATUS_ENA_INT_A_EN_DEFAULT           0x00000000

/* EMMC_HOSTIF :: INT_STATUS_ENA :: CARD_INT_ENA [08:08] */
#define EMMC_HOSTIF_INT_STATUS_ENA_CARD_INT_ENA_MASK          0x00000100
#define EMMC_HOSTIF_INT_STATUS_ENA_CARD_INT_ENA_SHIFT         8
#define EMMC_HOSTIF_INT_STATUS_ENA_CARD_INT_ENA_DEFAULT       0x00000000

/* EMMC_HOSTIF :: INT_STATUS_ENA :: CAR_REMOVAL_INT_ENA [07:07] */
#define EMMC_HOSTIF_INT_STATUS_ENA_CAR_REMOVAL_INT_ENA_MASK   0x00000080
#define EMMC_HOSTIF_INT_STATUS_ENA_CAR_REMOVAL_INT_ENA_SHIFT  7
#define EMMC_HOSTIF_INT_STATUS_ENA_CAR_REMOVAL_INT_ENA_DEFAULT 0x00000000

/* EMMC_HOSTIF :: INT_STATUS_ENA :: CAR_INSERT_INT_ENA [06:06] */
#define EMMC_HOSTIF_INT_STATUS_ENA_CAR_INSERT_INT_ENA_MASK    0x00000040
#define EMMC_HOSTIF_INT_STATUS_ENA_CAR_INSERT_INT_ENA_SHIFT   6
#define EMMC_HOSTIF_INT_STATUS_ENA_CAR_INSERT_INT_ENA_DEFAULT 0x00000000

/* EMMC_HOSTIF :: INT_STATUS_ENA :: BUFFER_READ_INT_ENA [05:05] */
#define EMMC_HOSTIF_INT_STATUS_ENA_BUFFER_READ_INT_ENA_MASK   0x00000020
#define EMMC_HOSTIF_INT_STATUS_ENA_BUFFER_READ_INT_ENA_SHIFT  5
#define EMMC_HOSTIF_INT_STATUS_ENA_BUFFER_READ_INT_ENA_DEFAULT 0x00000000

/* EMMC_HOSTIF :: INT_STATUS_ENA :: BUFFER_WRITE_INT_ENA [04:04] */
#define EMMC_HOSTIF_INT_STATUS_ENA_BUFFER_WRITE_INT_ENA_MASK  0x00000010
#define EMMC_HOSTIF_INT_STATUS_ENA_BUFFER_WRITE_INT_ENA_SHIFT 4
#define EMMC_HOSTIF_INT_STATUS_ENA_BUFFER_WRITE_INT_ENA_DEFAULT 0x00000000

/* EMMC_HOSTIF :: INT_STATUS_ENA :: DMA_INT_ENA [03:03] */
#define EMMC_HOSTIF_INT_STATUS_ENA_DMA_INT_ENA_MASK           0x00000008
#define EMMC_HOSTIF_INT_STATUS_ENA_DMA_INT_ENA_SHIFT          3
#define EMMC_HOSTIF_INT_STATUS_ENA_DMA_INT_ENA_DEFAULT        0x00000000

/* EMMC_HOSTIF :: INT_STATUS_ENA :: BLOCK_GAP_INT_ENA [02:02] */
#define EMMC_HOSTIF_INT_STATUS_ENA_BLOCK_GAP_INT_ENA_MASK     0x00000004
#define EMMC_HOSTIF_INT_STATUS_ENA_BLOCK_GAP_INT_ENA_SHIFT    2
#define EMMC_HOSTIF_INT_STATUS_ENA_BLOCK_GAP_INT_ENA_DEFAULT  0x00000000

/* EMMC_HOSTIF :: INT_STATUS_ENA :: TRANSFER_COMPLETE_INT_ENA [01:01] */
#define EMMC_HOSTIF_INT_STATUS_ENA_TRANSFER_COMPLETE_INT_ENA_MASK 0x00000002
#define EMMC_HOSTIF_INT_STATUS_ENA_TRANSFER_COMPLETE_INT_ENA_SHIFT 1
#define EMMC_HOSTIF_INT_STATUS_ENA_TRANSFER_COMPLETE_INT_ENA_DEFAULT 0x00000000

/* EMMC_HOSTIF :: INT_STATUS_ENA :: COMMAND_COMPLETE_INT_ENA [00:00] */
#define EMMC_HOSTIF_INT_STATUS_ENA_COMMAND_COMPLETE_INT_ENA_MASK 0x00000001
#define EMMC_HOSTIF_INT_STATUS_ENA_COMMAND_COMPLETE_INT_ENA_SHIFT 0
#define EMMC_HOSTIF_INT_STATUS_ENA_COMMAND_COMPLETE_INT_ENA_DEFAULT 0x00000000

   uint32 emmc_host_int_signal_ena;        /* 0x38 Interrupt Signal Enables for Normal and Error conditions        */
/***************************************************************************
 *INT_SIGNAL_ENA - Interrupt Signal Enables for Normal and Error conditions
 ***************************************************************************/
/* EMMC_HOSTIF :: INT_SIGNAL_ENA :: reserved0 [31:30] */
#define EMMC_HOSTIF_INT_SIGNAL_ENA_reserved0_MASK             0xc0000000
#define EMMC_HOSTIF_INT_SIGNAL_ENA_reserved0_SHIFT            30

/* EMMC_HOSTIF :: INT_SIGNAL_ENA :: reserved_for_eco1 [29:29] */
#define EMMC_HOSTIF_INT_SIGNAL_ENA_reserved_for_eco1_MASK     0x20000000
#define EMMC_HOSTIF_INT_SIGNAL_ENA_reserved_for_eco1_SHIFT    29
#define EMMC_HOSTIF_INT_SIGNAL_ENA_reserved_for_eco1_DEFAULT  0x00000000

/* EMMC_HOSTIF :: INT_SIGNAL_ENA :: TARGET_RESP_ERR_INT_SIG_ENA [28:28] */
#define EMMC_HOSTIF_INT_SIGNAL_ENA_TARGET_RESP_ERR_INT_SIG_ENA_MASK 0x10000000
#define EMMC_HOSTIF_INT_SIGNAL_ENA_TARGET_RESP_ERR_INT_SIG_ENA_SHIFT 28
#define EMMC_HOSTIF_INT_SIGNAL_ENA_TARGET_RESP_ERR_INT_SIG_ENA_DEFAULT 0x00000000

/* EMMC_HOSTIF :: INT_SIGNAL_ENA :: reserved2 [27:27] */
#define EMMC_HOSTIF_INT_SIGNAL_ENA_reserved2_MASK             0x08000000
#define EMMC_HOSTIF_INT_SIGNAL_ENA_reserved2_SHIFT            27

/* EMMC_HOSTIF :: INT_SIGNAL_ENA :: TUNE_ERR_SIG_EN [26:26] */
#define EMMC_HOSTIF_INT_SIGNAL_ENA_TUNE_ERR_SIG_EN_MASK       0x04000000
#define EMMC_HOSTIF_INT_SIGNAL_ENA_TUNE_ERR_SIG_EN_SHIFT      26
#define EMMC_HOSTIF_INT_SIGNAL_ENA_TUNE_ERR_SIG_EN_DEFAULT    0x00000000

/* EMMC_HOSTIF :: INT_SIGNAL_ENA :: ADMA_ERR_INT_SIG_ENA [25:25] */
#define EMMC_HOSTIF_INT_SIGNAL_ENA_ADMA_ERR_INT_SIG_ENA_MASK  0x02000000
#define EMMC_HOSTIF_INT_SIGNAL_ENA_ADMA_ERR_INT_SIG_ENA_SHIFT 25
#define EMMC_HOSTIF_INT_SIGNAL_ENA_ADMA_ERR_INT_SIG_ENA_DEFAULT 0x00000000

/* EMMC_HOSTIF :: INT_SIGNAL_ENA :: AUTO_CMD12_ERR_INT_SIG_ENA [24:24] */
#define EMMC_HOSTIF_INT_SIGNAL_ENA_AUTO_CMD12_ERR_INT_SIG_ENA_MASK 0x01000000
#define EMMC_HOSTIF_INT_SIGNAL_ENA_AUTO_CMD12_ERR_INT_SIG_ENA_SHIFT 24
#define EMMC_HOSTIF_INT_SIGNAL_ENA_AUTO_CMD12_ERR_INT_SIG_ENA_DEFAULT 0x00000000

/* EMMC_HOSTIF :: INT_SIGNAL_ENA :: CURRENT_LIMIT_ERR_INT_SIG_ENA [23:23] */
#define EMMC_HOSTIF_INT_SIGNAL_ENA_CURRENT_LIMIT_ERR_INT_SIG_ENA_MASK 0x00800000
#define EMMC_HOSTIF_INT_SIGNAL_ENA_CURRENT_LIMIT_ERR_INT_SIG_ENA_SHIFT 23
#define EMMC_HOSTIF_INT_SIGNAL_ENA_CURRENT_LIMIT_ERR_INT_SIG_ENA_DEFAULT 0x00000000

/* EMMC_HOSTIF :: INT_SIGNAL_ENA :: DATA_END_BIT_ERR_INT_SIG_ENA [22:22] */
#define EMMC_HOSTIF_INT_SIGNAL_ENA_DATA_END_BIT_ERR_INT_SIG_ENA_MASK 0x00400000
#define EMMC_HOSTIF_INT_SIGNAL_ENA_DATA_END_BIT_ERR_INT_SIG_ENA_SHIFT 22
#define EMMC_HOSTIF_INT_SIGNAL_ENA_DATA_END_BIT_ERR_INT_SIG_ENA_DEFAULT 0x00000000

/* EMMC_HOSTIF :: INT_SIGNAL_ENA :: DATA_CRC_ERR_INT_SIG_ENA [21:21] */
#define EMMC_HOSTIF_INT_SIGNAL_ENA_DATA_CRC_ERR_INT_SIG_ENA_MASK 0x00200000
#define EMMC_HOSTIF_INT_SIGNAL_ENA_DATA_CRC_ERR_INT_SIG_ENA_SHIFT 21
#define EMMC_HOSTIF_INT_SIGNAL_ENA_DATA_CRC_ERR_INT_SIG_ENA_DEFAULT 0x00000000

/* EMMC_HOSTIF :: INT_SIGNAL_ENA :: DATA_TIMEOUT_ERR_INT_SIG_ENA [20:20] */
#define EMMC_HOSTIF_INT_SIGNAL_ENA_DATA_TIMEOUT_ERR_INT_SIG_ENA_MASK 0x00100000
#define EMMC_HOSTIF_INT_SIGNAL_ENA_DATA_TIMEOUT_ERR_INT_SIG_ENA_SHIFT 20
#define EMMC_HOSTIF_INT_SIGNAL_ENA_DATA_TIMEOUT_ERR_INT_SIG_ENA_DEFAULT 0x00000000

/* EMMC_HOSTIF :: INT_SIGNAL_ENA :: CMD_INDEX_ERR_INT_SIG_ENA [19:19] */
#define EMMC_HOSTIF_INT_SIGNAL_ENA_CMD_INDEX_ERR_INT_SIG_ENA_MASK 0x00080000
#define EMMC_HOSTIF_INT_SIGNAL_ENA_CMD_INDEX_ERR_INT_SIG_ENA_SHIFT 19
#define EMMC_HOSTIF_INT_SIGNAL_ENA_CMD_INDEX_ERR_INT_SIG_ENA_DEFAULT 0x00000000

/* EMMC_HOSTIF :: INT_SIGNAL_ENA :: CMD_END_BIT_ERR_INT_SIG_ENA [18:18] */
#define EMMC_HOSTIF_INT_SIGNAL_ENA_CMD_END_BIT_ERR_INT_SIG_ENA_MASK 0x00040000
#define EMMC_HOSTIF_INT_SIGNAL_ENA_CMD_END_BIT_ERR_INT_SIG_ENA_SHIFT 18
#define EMMC_HOSTIF_INT_SIGNAL_ENA_CMD_END_BIT_ERR_INT_SIG_ENA_DEFAULT 0x00000000

/* EMMC_HOSTIF :: INT_SIGNAL_ENA :: CMD_CRC_ERR_INT_SIG_ENA [17:17] */
#define EMMC_HOSTIF_INT_SIGNAL_ENA_CMD_CRC_ERR_INT_SIG_ENA_MASK 0x00020000
#define EMMC_HOSTIF_INT_SIGNAL_ENA_CMD_CRC_ERR_INT_SIG_ENA_SHIFT 17
#define EMMC_HOSTIF_INT_SIGNAL_ENA_CMD_CRC_ERR_INT_SIG_ENA_DEFAULT 0x00000000

/* EMMC_HOSTIF :: INT_SIGNAL_ENA :: CMD_TIMEOUT_ERR_INT_SIG_ENA [16:16] */
#define EMMC_HOSTIF_INT_SIGNAL_ENA_CMD_TIMEOUT_ERR_INT_SIG_ENA_MASK 0x00010000
#define EMMC_HOSTIF_INT_SIGNAL_ENA_CMD_TIMEOUT_ERR_INT_SIG_ENA_SHIFT 16
#define EMMC_HOSTIF_INT_SIGNAL_ENA_CMD_TIMEOUT_ERR_INT_SIG_ENA_DEFAULT 0x00000000

/* EMMC_HOSTIF :: INT_SIGNAL_ENA :: reserved3 [15:15] */
#define EMMC_HOSTIF_INT_SIGNAL_ENA_reserved3_MASK             0x00008000
#define EMMC_HOSTIF_INT_SIGNAL_ENA_reserved3_SHIFT            15

/* EMMC_HOSTIF :: INT_SIGNAL_ENA :: BOOT_TERM_INT_SIG_ENA [14:14] */
#define EMMC_HOSTIF_INT_SIGNAL_ENA_BOOT_TERM_INT_SIG_ENA_MASK 0x00004000
#define EMMC_HOSTIF_INT_SIGNAL_ENA_BOOT_TERM_INT_SIG_ENA_SHIFT 14
#define EMMC_HOSTIF_INT_SIGNAL_ENA_BOOT_TERM_INT_SIG_ENA_DEFAULT 0x00000000

/* EMMC_HOSTIF :: INT_SIGNAL_ENA :: BOOT_ACK_RCV_INT_SIG_ENA [13:13] */
#define EMMC_HOSTIF_INT_SIGNAL_ENA_BOOT_ACK_RCV_INT_SIG_ENA_MASK 0x00002000
#define EMMC_HOSTIF_INT_SIGNAL_ENA_BOOT_ACK_RCV_INT_SIG_ENA_SHIFT 13
#define EMMC_HOSTIF_INT_SIGNAL_ENA_BOOT_ACK_RCV_INT_SIG_ENA_DEFAULT 0x00000000

/* EMMC_HOSTIF :: INT_SIGNAL_ENA :: RETUNE_EVENT_EN [12:12] */
#define EMMC_HOSTIF_INT_SIGNAL_ENA_RETUNE_EVENT_EN_MASK       0x00001000
#define EMMC_HOSTIF_INT_SIGNAL_ENA_RETUNE_EVENT_EN_SHIFT      12
#define EMMC_HOSTIF_INT_SIGNAL_ENA_RETUNE_EVENT_EN_DEFAULT    0x00000000

/* EMMC_HOSTIF :: INT_SIGNAL_ENA :: INT_C_EN [11:11] */
#define EMMC_HOSTIF_INT_SIGNAL_ENA_INT_C_EN_MASK              0x00000800
#define EMMC_HOSTIF_INT_SIGNAL_ENA_INT_C_EN_SHIFT             11
#define EMMC_HOSTIF_INT_SIGNAL_ENA_INT_C_EN_DEFAULT           0x00000000

/* EMMC_HOSTIF :: INT_SIGNAL_ENA :: INT_B_EN [10:10] */
#define EMMC_HOSTIF_INT_SIGNAL_ENA_INT_B_EN_MASK              0x00000400
#define EMMC_HOSTIF_INT_SIGNAL_ENA_INT_B_EN_SHIFT             10
#define EMMC_HOSTIF_INT_SIGNAL_ENA_INT_B_EN_DEFAULT           0x00000000

/* EMMC_HOSTIF :: INT_SIGNAL_ENA :: INT_A_EN [09:09] */
#define EMMC_HOSTIF_INT_SIGNAL_ENA_INT_A_EN_MASK              0x00000200
#define EMMC_HOSTIF_INT_SIGNAL_ENA_INT_A_EN_SHIFT             9
#define EMMC_HOSTIF_INT_SIGNAL_ENA_INT_A_EN_DEFAULT           0x00000000

/* EMMC_HOSTIF :: INT_SIGNAL_ENA :: CARD_INT_SIG_ENA [08:08] */
#define EMMC_HOSTIF_INT_SIGNAL_ENA_CARD_INT_SIG_ENA_MASK      0x00000100
#define EMMC_HOSTIF_INT_SIGNAL_ENA_CARD_INT_SIG_ENA_SHIFT     8
#define EMMC_HOSTIF_INT_SIGNAL_ENA_CARD_INT_SIG_ENA_DEFAULT   0x00000000

/* EMMC_HOSTIF :: INT_SIGNAL_ENA :: CAR_REMOVAL_INT_SIG_ENA [07:07] */
#define EMMC_HOSTIF_INT_SIGNAL_ENA_CAR_REMOVAL_INT_SIG_ENA_MASK 0x00000080
#define EMMC_HOSTIF_INT_SIGNAL_ENA_CAR_REMOVAL_INT_SIG_ENA_SHIFT 7
#define EMMC_HOSTIF_INT_SIGNAL_ENA_CAR_REMOVAL_INT_SIG_ENA_DEFAULT 0x00000000

/* EMMC_HOSTIF :: INT_SIGNAL_ENA :: CAR_INSERT_INT_SIG_ENA [06:06] */
#define EMMC_HOSTIF_INT_SIGNAL_ENA_CAR_INSERT_INT_SIG_ENA_MASK 0x00000040
#define EMMC_HOSTIF_INT_SIGNAL_ENA_CAR_INSERT_INT_SIG_ENA_SHIFT 6
#define EMMC_HOSTIF_INT_SIGNAL_ENA_CAR_INSERT_INT_SIG_ENA_DEFAULT 0x00000000

/* EMMC_HOSTIF :: INT_SIGNAL_ENA :: BUFFER_READ_INT_SIG_ENA [05:05] */
#define EMMC_HOSTIF_INT_SIGNAL_ENA_BUFFER_READ_INT_SIG_ENA_MASK 0x00000020
#define EMMC_HOSTIF_INT_SIGNAL_ENA_BUFFER_READ_INT_SIG_ENA_SHIFT 5
#define EMMC_HOSTIF_INT_SIGNAL_ENA_BUFFER_READ_INT_SIG_ENA_DEFAULT 0x00000000

/* EMMC_HOSTIF :: INT_SIGNAL_ENA :: BUFFER_WRITE_INT_SIG_ENA [04:04] */
#define EMMC_HOSTIF_INT_SIGNAL_ENA_BUFFER_WRITE_INT_SIG_ENA_MASK 0x00000010
#define EMMC_HOSTIF_INT_SIGNAL_ENA_BUFFER_WRITE_INT_SIG_ENA_SHIFT 4
#define EMMC_HOSTIF_INT_SIGNAL_ENA_BUFFER_WRITE_INT_SIG_ENA_DEFAULT 0x00000000

/* EMMC_HOSTIF :: INT_SIGNAL_ENA :: DMA_INT_SIG_ENA [03:03] */
#define EMMC_HOSTIF_INT_SIGNAL_ENA_DMA_INT_SIG_ENA_MASK       0x00000008
#define EMMC_HOSTIF_INT_SIGNAL_ENA_DMA_INT_SIG_ENA_SHIFT      3
#define EMMC_HOSTIF_INT_SIGNAL_ENA_DMA_INT_SIG_ENA_DEFAULT    0x00000000

/* EMMC_HOSTIF :: INT_SIGNAL_ENA :: BLOCK_GAP_INT_SIG_ENA [02:02] */
#define EMMC_HOSTIF_INT_SIGNAL_ENA_BLOCK_GAP_INT_SIG_ENA_MASK 0x00000004
#define EMMC_HOSTIF_INT_SIGNAL_ENA_BLOCK_GAP_INT_SIG_ENA_SHIFT 2
#define EMMC_HOSTIF_INT_SIGNAL_ENA_BLOCK_GAP_INT_SIG_ENA_DEFAULT 0x00000000

/* EMMC_HOSTIF :: INT_SIGNAL_ENA :: TRANSFER_COMPLETE_INT_SIG_ENA [01:01] */
#define EMMC_HOSTIF_INT_SIGNAL_ENA_TRANSFER_COMPLETE_INT_SIG_ENA_MASK 0x00000002
#define EMMC_HOSTIF_INT_SIGNAL_ENA_TRANSFER_COMPLETE_INT_SIG_ENA_SHIFT 1
#define EMMC_HOSTIF_INT_SIGNAL_ENA_TRANSFER_COMPLETE_INT_SIG_ENA_DEFAULT 0x00000000

/* EMMC_HOSTIF :: INT_SIGNAL_ENA :: COMMAND_COMPLETE_INT_SIG_ENA [00:00] */
#define EMMC_HOSTIF_INT_SIGNAL_ENA_COMMAND_COMPLETE_INT_SIG_ENA_MASK 0x00000001
#define EMMC_HOSTIF_INT_SIGNAL_ENA_COMMAND_COMPLETE_INT_SIG_ENA_SHIFT 0
#define EMMC_HOSTIF_INT_SIGNAL_ENA_COMMAND_COMPLETE_INT_SIG_ENA_DEFAULT 0x00000000

   uint32 emmc_host_autocmd12_stat;        /* 0x3c Auto Cmd12 Error Status                                         */
/***************************************************************************
 *AUTOCMD12_STAT - Auto Cmd12 Error Status
 ***************************************************************************/
/* EMMC_HOSTIF :: AUTOCMD12_STAT :: PRESENT_VALUE_EN [31:31] */
#define EMMC_HOSTIF_AUTOCMD12_STAT_PRESENT_VALUE_EN_MASK      0x80000000
#define EMMC_HOSTIF_AUTOCMD12_STAT_PRESENT_VALUE_EN_SHIFT     31
#define EMMC_HOSTIF_AUTOCMD12_STAT_PRESENT_VALUE_EN_DEFAULT   0x00000000

/* EMMC_HOSTIF :: AUTOCMD12_STAT :: ASYNC_INT_EN [30:30] */
#define EMMC_HOSTIF_AUTOCMD12_STAT_ASYNC_INT_EN_MASK          0x40000000
#define EMMC_HOSTIF_AUTOCMD12_STAT_ASYNC_INT_EN_SHIFT         30
#define EMMC_HOSTIF_AUTOCMD12_STAT_ASYNC_INT_EN_DEFAULT       0x00000000

/* EMMC_HOSTIF :: AUTOCMD12_STAT :: reserved0 [29:24] */
#define EMMC_HOSTIF_AUTOCMD12_STAT_reserved0_MASK             0x3f000000
#define EMMC_HOSTIF_AUTOCMD12_STAT_reserved0_SHIFT            24

/* EMMC_HOSTIF :: AUTOCMD12_STAT :: SAMPLE_CLK_SEL [23:23] */
#define EMMC_HOSTIF_AUTOCMD12_STAT_SAMPLE_CLK_SEL_MASK        0x00800000
#define EMMC_HOSTIF_AUTOCMD12_STAT_SAMPLE_CLK_SEL_SHIFT       23
#define EMMC_HOSTIF_AUTOCMD12_STAT_SAMPLE_CLK_SEL_DEFAULT     0x00000000

/* EMMC_HOSTIF :: AUTOCMD12_STAT :: EXECUTE_TUNE [22:22] */
#define EMMC_HOSTIF_AUTOCMD12_STAT_EXECUTE_TUNE_MASK          0x00400000
#define EMMC_HOSTIF_AUTOCMD12_STAT_EXECUTE_TUNE_SHIFT         22
#define EMMC_HOSTIF_AUTOCMD12_STAT_EXECUTE_TUNE_DEFAULT       0x00000000

/* EMMC_HOSTIF :: AUTOCMD12_STAT :: DRIVER_STRENGTH [21:20] */
#define EMMC_HOSTIF_AUTOCMD12_STAT_DRIVER_STRENGTH_MASK       0x00300000
#define EMMC_HOSTIF_AUTOCMD12_STAT_DRIVER_STRENGTH_SHIFT      20
#define EMMC_HOSTIF_AUTOCMD12_STAT_DRIVER_STRENGTH_DEFAULT    0x00000000

/* EMMC_HOSTIF :: AUTOCMD12_STAT :: 1P8_SIG_EN [19:19] */
#define EMMC_HOSTIF_AUTOCMD12_STAT_1P8_SIG_EN_MASK            0x00080000
#define EMMC_HOSTIF_AUTOCMD12_STAT_1P8_SIG_EN_SHIFT           19
#define EMMC_HOSTIF_AUTOCMD12_STAT_1P8_SIG_EN_DEFAULT         0x00000000

/* EMMC_HOSTIF :: AUTOCMD12_STAT :: UHS_MODE [18:16] */
#define EMMC_HOSTIF_AUTOCMD12_STAT_UHS_MODE_MASK              0x00070000
#define EMMC_HOSTIF_AUTOCMD12_STAT_UHS_MODE_SHIFT             16
#define EMMC_HOSTIF_AUTOCMD12_STAT_UHS_MODE_DEFAULT           0x00000000

/* EMMC_HOSTIF :: AUTOCMD12_STAT :: reserved1 [15:08] */
#define EMMC_HOSTIF_AUTOCMD12_STAT_reserved1_MASK             0x0000ff00
#define EMMC_HOSTIF_AUTOCMD12_STAT_reserved1_SHIFT            8

/* EMMC_HOSTIF :: AUTOCMD12_STAT :: CMD12_NOT_ISSUED [07:07] */
#define EMMC_HOSTIF_AUTOCMD12_STAT_CMD12_NOT_ISSUED_MASK      0x00000080
#define EMMC_HOSTIF_AUTOCMD12_STAT_CMD12_NOT_ISSUED_SHIFT     7
#define EMMC_HOSTIF_AUTOCMD12_STAT_CMD12_NOT_ISSUED_DEFAULT   0x00000000

/* EMMC_HOSTIF :: AUTOCMD12_STAT :: reserved2 [06:05] */
#define EMMC_HOSTIF_AUTOCMD12_STAT_reserved2_MASK             0x00000060
#define EMMC_HOSTIF_AUTOCMD12_STAT_reserved2_SHIFT            5

/* EMMC_HOSTIF :: AUTOCMD12_STAT :: CMD_INDEX_ERR [04:04] */
#define EMMC_HOSTIF_AUTOCMD12_STAT_CMD_INDEX_ERR_MASK         0x00000010
#define EMMC_HOSTIF_AUTOCMD12_STAT_CMD_INDEX_ERR_SHIFT        4
#define EMMC_HOSTIF_AUTOCMD12_STAT_CMD_INDEX_ERR_DEFAULT      0x00000000

/* EMMC_HOSTIF :: AUTOCMD12_STAT :: CMD_END_ERR [03:03] */
#define EMMC_HOSTIF_AUTOCMD12_STAT_CMD_END_ERR_MASK           0x00000008
#define EMMC_HOSTIF_AUTOCMD12_STAT_CMD_END_ERR_SHIFT          3
#define EMMC_HOSTIF_AUTOCMD12_STAT_CMD_END_ERR_DEFAULT        0x00000000

/* EMMC_HOSTIF :: AUTOCMD12_STAT :: CMD_CRC_ERR [02:02] */
#define EMMC_HOSTIF_AUTOCMD12_STAT_CMD_CRC_ERR_MASK           0x00000004
#define EMMC_HOSTIF_AUTOCMD12_STAT_CMD_CRC_ERR_SHIFT          2
#define EMMC_HOSTIF_AUTOCMD12_STAT_CMD_CRC_ERR_DEFAULT        0x00000000

/* EMMC_HOSTIF :: AUTOCMD12_STAT :: CMD_TIMEOUT_ERR [01:01] */
#define EMMC_HOSTIF_AUTOCMD12_STAT_CMD_TIMEOUT_ERR_MASK       0x00000002
#define EMMC_HOSTIF_AUTOCMD12_STAT_CMD_TIMEOUT_ERR_SHIFT      1
#define EMMC_HOSTIF_AUTOCMD12_STAT_CMD_TIMEOUT_ERR_DEFAULT    0x00000000

/* EMMC_HOSTIF :: AUTOCMD12_STAT :: CMD12_NOT_EXEC_ERR [00:00] */
#define EMMC_HOSTIF_AUTOCMD12_STAT_CMD12_NOT_EXEC_ERR_MASK    0x00000001
#define EMMC_HOSTIF_AUTOCMD12_STAT_CMD12_NOT_EXEC_ERR_SHIFT   0
#define EMMC_HOSTIF_AUTOCMD12_STAT_CMD12_NOT_EXEC_ERR_DEFAULT 0x00000000

   uint32 emmc_host_capable;               /* 0x40 Host Controller Capabilities to Software                        */
/***************************************************************************
 *CAPABLE - Host Controller Capabilities to Software
 ***************************************************************************/
/* EMMC_HOSTIF :: CAPABLE :: SLOT_TYPE [31:30] */
#define EMMC_HOSTIF_CAPABLE_SLOT_TYPE_MASK                    0xc0000000
#define EMMC_HOSTIF_CAPABLE_SLOT_TYPE_SHIFT                   30

/* EMMC_HOSTIF :: CAPABLE :: INT_MODE [29:29] */
#define EMMC_HOSTIF_CAPABLE_INT_MODE_MASK                     0x20000000
#define EMMC_HOSTIF_CAPABLE_INT_MODE_SHIFT                    29

/* EMMC_HOSTIF :: CAPABLE :: SYS_BUT_64BIT [28:28] */
#define EMMC_HOSTIF_CAPABLE_SYS_BUT_64BIT_MASK                0x10000000
#define EMMC_HOSTIF_CAPABLE_SYS_BUT_64BIT_SHIFT               28

/* EMMC_HOSTIF :: CAPABLE :: reserved0 [27:27] */
#define EMMC_HOSTIF_CAPABLE_reserved0_MASK                    0x08000000
#define EMMC_HOSTIF_CAPABLE_reserved0_SHIFT                   27

/* EMMC_HOSTIF :: CAPABLE :: VOLTAGE_1P8V [26:26] */
#define EMMC_HOSTIF_CAPABLE_VOLTAGE_1P8V_MASK                 0x04000000
#define EMMC_HOSTIF_CAPABLE_VOLTAGE_1P8V_SHIFT                26

/* EMMC_HOSTIF :: CAPABLE :: VOLTAGE_3P0V [25:25] */
#define EMMC_HOSTIF_CAPABLE_VOLTAGE_3P0V_MASK                 0x02000000
#define EMMC_HOSTIF_CAPABLE_VOLTAGE_3P0V_SHIFT                25

/* EMMC_HOSTIF :: CAPABLE :: VOLTAGE_3P3V [24:24] */
#define EMMC_HOSTIF_CAPABLE_VOLTAGE_3P3V_MASK                 0x01000000
#define EMMC_HOSTIF_CAPABLE_VOLTAGE_3P3V_SHIFT                24

/* EMMC_HOSTIF :: CAPABLE :: SUSPEND_RESUME [23:23] */
#define EMMC_HOSTIF_CAPABLE_SUSPEND_RESUME_MASK               0x00800000
#define EMMC_HOSTIF_CAPABLE_SUSPEND_RESUME_SHIFT              23

/* EMMC_HOSTIF :: CAPABLE :: SDMA [22:22] */
#define EMMC_HOSTIF_CAPABLE_SDMA_MASK                         0x00400000
#define EMMC_HOSTIF_CAPABLE_SDMA_SHIFT                        22

/* EMMC_HOSTIF :: CAPABLE :: HIGH_SPEED [21:21] */
#define EMMC_HOSTIF_CAPABLE_HIGH_SPEED_MASK                   0x00200000
#define EMMC_HOSTIF_CAPABLE_HIGH_SPEED_SHIFT                  21

/* EMMC_HOSTIF :: CAPABLE :: reserved1 [20:20] */
#define EMMC_HOSTIF_CAPABLE_reserved1_MASK                    0x00100000
#define EMMC_HOSTIF_CAPABLE_reserved1_SHIFT                   20

/* EMMC_HOSTIF :: CAPABLE :: ADMA2 [19:19] */
#define EMMC_HOSTIF_CAPABLE_ADMA2_MASK                        0x00080000
#define EMMC_HOSTIF_CAPABLE_ADMA2_SHIFT                       19

/* EMMC_HOSTIF :: CAPABLE :: EXTENDED_MEDIA [18:18] */
#define EMMC_HOSTIF_CAPABLE_EXTENDED_MEDIA_MASK               0x00040000
#define EMMC_HOSTIF_CAPABLE_EXTENDED_MEDIA_SHIFT              18

/* EMMC_HOSTIF :: CAPABLE :: MAX_BLOCK_LEN [17:16] */
#define EMMC_HOSTIF_CAPABLE_MAX_BLOCK_LEN_MASK                0x00030000
#define EMMC_HOSTIF_CAPABLE_MAX_BLOCK_LEN_SHIFT               16

/* EMMC_HOSTIF :: CAPABLE :: BASE_CLK_FREQ [15:08] */
#define EMMC_HOSTIF_CAPABLE_BASE_CLK_FREQ_MASK                0x0000ff00
#define EMMC_HOSTIF_CAPABLE_BASE_CLK_FREQ_SHIFT               8

/* EMMC_HOSTIF :: CAPABLE :: TIMEOUT_UNIT [07:07] */
#define EMMC_HOSTIF_CAPABLE_TIMEOUT_UNIT_MASK                 0x00000080
#define EMMC_HOSTIF_CAPABLE_TIMEOUT_UNIT_SHIFT                7

/* EMMC_HOSTIF :: CAPABLE :: reserved2 [06:06] */
#define EMMC_HOSTIF_CAPABLE_reserved2_MASK                    0x00000040
#define EMMC_HOSTIF_CAPABLE_reserved2_SHIFT                   6

/* EMMC_HOSTIF :: CAPABLE :: TIMEOUT_CLK_FREQ [05:00] */
#define EMMC_HOSTIF_CAPABLE_TIMEOUT_CLK_FREQ_MASK             0x0000003f
#define EMMC_HOSTIF_CAPABLE_TIMEOUT_CLK_FREQ_SHIFT            0

   uint32 emmc_host_capable_1;             /* 0x44 Future Host Controller Capabilities to Software                 */
/***************************************************************************
 *CAPABLE_1 - Future Host Controller Capabilities to Software
 ***************************************************************************/
/* EMMC_HOSTIF :: CAPABLE_1 :: reserved0 [31:26] */
#define EMMC_HOSTIF_CAPABLE_1_reserved0_MASK                  0xfc000000
#define EMMC_HOSTIF_CAPABLE_1_reserved0_SHIFT                 26

/* EMMC_HOSTIF :: CAPABLE_1 :: SPI_BLOCK_MODE [25:25] */
#define EMMC_HOSTIF_CAPABLE_1_SPI_BLOCK_MODE_MASK             0x02000000
#define EMMC_HOSTIF_CAPABLE_1_SPI_BLOCK_MODE_SHIFT            25

/* EMMC_HOSTIF :: CAPABLE_1 :: SPI_MODE [24:24] */
#define EMMC_HOSTIF_CAPABLE_1_SPI_MODE_MASK                   0x01000000
#define EMMC_HOSTIF_CAPABLE_1_SPI_MODE_SHIFT                  24

/* EMMC_HOSTIF :: CAPABLE_1 :: CLK_MULT [23:16] */
#define EMMC_HOSTIF_CAPABLE_1_CLK_MULT_MASK                   0x00ff0000
#define EMMC_HOSTIF_CAPABLE_1_CLK_MULT_SHIFT                  16

/* EMMC_HOSTIF :: CAPABLE_1 :: RETUNING_MODE [15:14] */
#define EMMC_HOSTIF_CAPABLE_1_RETUNING_MODE_MASK              0x0000c000
#define EMMC_HOSTIF_CAPABLE_1_RETUNING_MODE_SHIFT             14

/* EMMC_HOSTIF :: CAPABLE_1 :: TUNE_SDR50 [13:13] */
#define EMMC_HOSTIF_CAPABLE_1_TUNE_SDR50_MASK                 0x00002000
#define EMMC_HOSTIF_CAPABLE_1_TUNE_SDR50_SHIFT                13

/* EMMC_HOSTIF :: CAPABLE_1 :: reserved1 [12:12] */
#define EMMC_HOSTIF_CAPABLE_1_reserved1_MASK                  0x00001000
#define EMMC_HOSTIF_CAPABLE_1_reserved1_SHIFT                 12

/* EMMC_HOSTIF :: CAPABLE_1 :: TIME_RETUNE [11:08] */
#define EMMC_HOSTIF_CAPABLE_1_TIME_RETUNE_MASK                0x00000f00
#define EMMC_HOSTIF_CAPABLE_1_TIME_RETUNE_SHIFT               8

/* EMMC_HOSTIF :: CAPABLE_1 :: reserved2 [07:07] */
#define EMMC_HOSTIF_CAPABLE_1_reserved2_MASK                  0x00000080
#define EMMC_HOSTIF_CAPABLE_1_reserved2_SHIFT                 7

/* EMMC_HOSTIF :: CAPABLE_1 :: DRIVER_D [06:06] */
#define EMMC_HOSTIF_CAPABLE_1_DRIVER_D_MASK                   0x00000040
#define EMMC_HOSTIF_CAPABLE_1_DRIVER_D_SHIFT                  6

/* EMMC_HOSTIF :: CAPABLE_1 :: DRIVER_C [05:05] */
#define EMMC_HOSTIF_CAPABLE_1_DRIVER_C_MASK                   0x00000020
#define EMMC_HOSTIF_CAPABLE_1_DRIVER_C_SHIFT                  5

/* EMMC_HOSTIF :: CAPABLE_1 :: DRIVER_A [04:04] */
#define EMMC_HOSTIF_CAPABLE_1_DRIVER_A_MASK                   0x00000010
#define EMMC_HOSTIF_CAPABLE_1_DRIVER_A_SHIFT                  4

/* EMMC_HOSTIF :: CAPABLE_1 :: reserved3 [03:03] */
#define EMMC_HOSTIF_CAPABLE_1_reserved3_MASK                  0x00000008
#define EMMC_HOSTIF_CAPABLE_1_reserved3_SHIFT                 3

/* EMMC_HOSTIF :: CAPABLE_1 :: DDR50 [02:02] */
#define EMMC_HOSTIF_CAPABLE_1_DDR50_MASK                      0x00000004
#define EMMC_HOSTIF_CAPABLE_1_DDR50_SHIFT                     2

/* EMMC_HOSTIF :: CAPABLE_1 :: SDR104 [01:01] */
#define EMMC_HOSTIF_CAPABLE_1_SDR104_MASK                     0x00000002
#define EMMC_HOSTIF_CAPABLE_1_SDR104_SHIFT                    1

/* EMMC_HOSTIF :: CAPABLE_1 :: SDR50 [00:00] */
#define EMMC_HOSTIF_CAPABLE_1_SDR50_MASK                      0x00000001
#define EMMC_HOSTIF_CAPABLE_1_SDR50_SHIFT                     0

   uint32 emmc_host_power_capable;         /* 0x48 Host Controller Power Capabilities to Software                  */
/***************************************************************************
 *POWER_CAPABLE - Host Controller Power Capabilities to Software
 ***************************************************************************/
/* EMMC_HOSTIF :: POWER_CAPABLE :: reserved0 [31:24] */
#define EMMC_HOSTIF_POWER_CAPABLE_reserved0_MASK              0xff000000
#define EMMC_HOSTIF_POWER_CAPABLE_reserved0_SHIFT             24

/* EMMC_HOSTIF :: POWER_CAPABLE :: MAX_CURRENT_1P8V [23:16] */
#define EMMC_HOSTIF_POWER_CAPABLE_MAX_CURRENT_1P8V_MASK       0x00ff0000
#define EMMC_HOSTIF_POWER_CAPABLE_MAX_CURRENT_1P8V_SHIFT      16
#define EMMC_HOSTIF_POWER_CAPABLE_MAX_CURRENT_1P8V_DEFAULT    0x00000000

/* EMMC_HOSTIF :: POWER_CAPABLE :: MAX_CURRENT_3P0V [15:08] */
#define EMMC_HOSTIF_POWER_CAPABLE_MAX_CURRENT_3P0V_MASK       0x0000ff00
#define EMMC_HOSTIF_POWER_CAPABLE_MAX_CURRENT_3P0V_SHIFT      8
#define EMMC_HOSTIF_POWER_CAPABLE_MAX_CURRENT_3P0V_DEFAULT    0x00000000

/* EMMC_HOSTIF :: POWER_CAPABLE :: MAX_CURRENT_3P3V [07:00] */
#define EMMC_HOSTIF_POWER_CAPABLE_MAX_CURRENT_3P3V_MASK       0x000000ff
#define EMMC_HOSTIF_POWER_CAPABLE_MAX_CURRENT_3P3V_SHIFT      0
#define EMMC_HOSTIF_POWER_CAPABLE_MAX_CURRENT_3P3V_DEFAULT    0x00000001

   uint32 emmc_host_power_capable_rsvd;    /* 0x4c Future Host Controller Power Capabilities to Software           */
/***************************************************************************
 *POWER_CAPABLE_RSVD - Future Host Controller Power Capabilities to Software
 ***************************************************************************/
/* EMMC_HOSTIF :: POWER_CAPABLE_RSVD :: POWERCAPS_RSVD [31:00] */
#define EMMC_HOSTIF_POWER_CAPABLE_RSVD_POWERCAPS_RSVD_MASK    0xffffffff
#define EMMC_HOSTIF_POWER_CAPABLE_RSVD_POWERCAPS_RSVD_SHIFT   0
#define EMMC_HOSTIF_POWER_CAPABLE_RSVD_POWERCAPS_RSVD_DEFAULT 0x00000000

   uint32 emmc_host_force_events;          /* 0x50 Force Events on Error Status Bits                               */
/***************************************************************************
 *FORCE_EVENTS - Force Events on Error Status Bits
 ***************************************************************************/
/* EMMC_HOSTIF :: FORCE_EVENTS :: reserved0 [31:30] */
#define B0_EMMC_HOSTIF_FORCE_EVENTS_reserved0_MASK            0xc0000000
#define B0_EMMC_HOSTIF_FORCE_EVENTS_reserved0_SHIFT           30

/* EMMC_HOSTIF :: FORCE_EVENTS :: reserved0 [29:29] */
#define EMMC_HOSTIF_FORCE_EVENTS_reserved0_MASK               0x20000000
#define EMMC_HOSTIF_FORCE_EVENTS_reserved0_SHIFT              29

/* EMMC_HOSTIF :: FORCE_EVENTS :: FORCE_TARGET_RESP_ERR_INT [28:28] */
#define EMMC_HOSTIF_FORCE_EVENTS_FORCE_TARGET_RESP_ERR_INT_MASK 0x10000000
#define EMMC_HOSTIF_FORCE_EVENTS_FORCE_TARGET_RESP_ERR_INT_SHIFT 28
#define EMMC_HOSTIF_FORCE_EVENTS_FORCE_TARGET_RESP_ERR_INT_DEFAULT 0x00000000

/* EMMC_HOSTIF :: FORCE_EVENTS :: reserved1 [27:26] */
#define EMMC_HOSTIF_FORCE_EVENTS_reserved1_MASK               0x0c000000
#define EMMC_HOSTIF_FORCE_EVENTS_reserved1_SHIFT              26

/* EMMC_HOSTIF :: FORCE_EVENTS :: FORCE_ADMA_ERR_INT [25:25] */
#define EMMC_HOSTIF_FORCE_EVENTS_FORCE_ADMA_ERR_INT_MASK      0x02000000
#define EMMC_HOSTIF_FORCE_EVENTS_FORCE_ADMA_ERR_INT_SHIFT     25
#define EMMC_HOSTIF_FORCE_EVENTS_FORCE_ADMA_ERR_INT_DEFAULT   0x00000000

/* EMMC_HOSTIF :: FORCE_EVENTS :: FORCE_AUTO_CMD_ERR_INT [24:24] */
#define EMMC_HOSTIF_FORCE_EVENTS_FORCE_AUTO_CMD_ERR_INT_MASK  0x01000000
#define EMMC_HOSTIF_FORCE_EVENTS_FORCE_AUTO_CMD_ERR_INT_SHIFT 24
#define EMMC_HOSTIF_FORCE_EVENTS_FORCE_AUTO_CMD_ERR_INT_DEFAULT 0x00000000

/* EMMC_HOSTIF :: FORCE_EVENTS :: FORCE_CURRENT_LIMIT_ERR_INT [23:23] */
#define EMMC_HOSTIF_FORCE_EVENTS_FORCE_CURRENT_LIMIT_ERR_INT_MASK 0x00800000
#define EMMC_HOSTIF_FORCE_EVENTS_FORCE_CURRENT_LIMIT_ERR_INT_SHIFT 23
#define EMMC_HOSTIF_FORCE_EVENTS_FORCE_CURRENT_LIMIT_ERR_INT_DEFAULT 0x00000000

/* EMMC_HOSTIF :: FORCE_EVENTS :: FORCE_DATA_END_BIT_ERR_INT [22:22] */
#define EMMC_HOSTIF_FORCE_EVENTS_FORCE_DATA_END_BIT_ERR_INT_MASK 0x00400000
#define EMMC_HOSTIF_FORCE_EVENTS_FORCE_DATA_END_BIT_ERR_INT_SHIFT 22
#define EMMC_HOSTIF_FORCE_EVENTS_FORCE_DATA_END_BIT_ERR_INT_DEFAULT 0x00000000

/* EMMC_HOSTIF :: FORCE_EVENTS :: FORCE_DATA_CRC_ERR_INT [21:21] */
#define EMMC_HOSTIF_FORCE_EVENTS_FORCE_DATA_CRC_ERR_INT_MASK  0x00200000
#define EMMC_HOSTIF_FORCE_EVENTS_FORCE_DATA_CRC_ERR_INT_SHIFT 21
#define EMMC_HOSTIF_FORCE_EVENTS_FORCE_DATA_CRC_ERR_INT_DEFAULT 0x00000000

/* EMMC_HOSTIF :: FORCE_EVENTS :: FORCE_DATA_TIMEOUT_ERR_INT [20:20] */
#define EMMC_HOSTIF_FORCE_EVENTS_FORCE_DATA_TIMEOUT_ERR_INT_MASK 0x00100000
#define EMMC_HOSTIF_FORCE_EVENTS_FORCE_DATA_TIMEOUT_ERR_INT_SHIFT 20
#define EMMC_HOSTIF_FORCE_EVENTS_FORCE_DATA_TIMEOUT_ERR_INT_DEFAULT 0x00000000

/* EMMC_HOSTIF :: FORCE_EVENTS :: FORCE_CMD_INDEX_ERR_INT [19:19] */
#define EMMC_HOSTIF_FORCE_EVENTS_FORCE_CMD_INDEX_ERR_INT_MASK 0x00080000
#define EMMC_HOSTIF_FORCE_EVENTS_FORCE_CMD_INDEX_ERR_INT_SHIFT 19
#define EMMC_HOSTIF_FORCE_EVENTS_FORCE_CMD_INDEX_ERR_INT_DEFAULT 0x00000000

/* EMMC_HOSTIF :: FORCE_EVENTS :: FORCE_CMD_END_BIT_ERR_INT [18:18] */
#define EMMC_HOSTIF_FORCE_EVENTS_FORCE_CMD_END_BIT_ERR_INT_MASK 0x00040000
#define EMMC_HOSTIF_FORCE_EVENTS_FORCE_CMD_END_BIT_ERR_INT_SHIFT 18
#define EMMC_HOSTIF_FORCE_EVENTS_FORCE_CMD_END_BIT_ERR_INT_DEFAULT 0x00000000

/* EMMC_HOSTIF :: FORCE_EVENTS :: FORCE_CMD_CRC_ERR_INT [17:17] */
#define EMMC_HOSTIF_FORCE_EVENTS_FORCE_CMD_CRC_ERR_INT_MASK   0x00020000
#define EMMC_HOSTIF_FORCE_EVENTS_FORCE_CMD_CRC_ERR_INT_SHIFT  17
#define EMMC_HOSTIF_FORCE_EVENTS_FORCE_CMD_CRC_ERR_INT_DEFAULT 0x00000000

/* EMMC_HOSTIF :: FORCE_EVENTS :: FORCE_CMD_TIMEOUT_ERR_INT [16:16] */
#define EMMC_HOSTIF_FORCE_EVENTS_FORCE_CMD_TIMEOUT_ERR_INT_MASK 0x00010000
#define EMMC_HOSTIF_FORCE_EVENTS_FORCE_CMD_TIMEOUT_ERR_INT_SHIFT 16
#define EMMC_HOSTIF_FORCE_EVENTS_FORCE_CMD_TIMEOUT_ERR_INT_DEFAULT 0x00000000

/* EMMC_HOSTIF :: FORCE_EVENTS :: reserved2 [15:08] */
#define EMMC_HOSTIF_FORCE_EVENTS_reserved2_MASK               0x0000ff00
#define EMMC_HOSTIF_FORCE_EVENTS_reserved2_SHIFT              8

/* EMMC_HOSTIF :: FORCE_EVENTS :: FORCE_CMD12_NOT_ISSUED [07:07] */
#define EMMC_HOSTIF_FORCE_EVENTS_FORCE_CMD12_NOT_ISSUED_MASK  0x00000080
#define EMMC_HOSTIF_FORCE_EVENTS_FORCE_CMD12_NOT_ISSUED_SHIFT 7
#define EMMC_HOSTIF_FORCE_EVENTS_FORCE_CMD12_NOT_ISSUED_DEFAULT 0x00000000

/* EMMC_HOSTIF :: FORCE_EVENTS :: reserved3 [06:05] */
#define EMMC_HOSTIF_FORCE_EVENTS_reserved3_MASK               0x00000060
#define EMMC_HOSTIF_FORCE_EVENTS_reserved3_SHIFT              5

/* EMMC_HOSTIF :: FORCE_EVENTS :: FORCE_CMD_INDEX_ERR [04:04] */
#define EMMC_HOSTIF_FORCE_EVENTS_FORCE_CMD_INDEX_ERR_MASK     0x00000010
#define EMMC_HOSTIF_FORCE_EVENTS_FORCE_CMD_INDEX_ERR_SHIFT    4
#define EMMC_HOSTIF_FORCE_EVENTS_FORCE_CMD_INDEX_ERR_DEFAULT  0x00000000

/* EMMC_HOSTIF :: FORCE_EVENTS :: FORCE_CMD_END_ERR [03:03] */
#define EMMC_HOSTIF_FORCE_EVENTS_FORCE_CMD_END_ERR_MASK       0x00000008
#define EMMC_HOSTIF_FORCE_EVENTS_FORCE_CMD_END_ERR_SHIFT      3
#define EMMC_HOSTIF_FORCE_EVENTS_FORCE_CMD_END_ERR_DEFAULT    0x00000000

/* EMMC_HOSTIF :: FORCE_EVENTS :: FORCE_CMD_CRC_ERR [02:02] */
#define EMMC_HOSTIF_FORCE_EVENTS_FORCE_CMD_CRC_ERR_MASK       0x00000004
#define EMMC_HOSTIF_FORCE_EVENTS_FORCE_CMD_CRC_ERR_SHIFT      2
#define EMMC_HOSTIF_FORCE_EVENTS_FORCE_CMD_CRC_ERR_DEFAULT    0x00000000

/* EMMC_HOSTIF :: FORCE_EVENTS :: FORCE_CMD_TIMEOUT_ERR [01:01] */
#define EMMC_HOSTIF_FORCE_EVENTS_FORCE_CMD_TIMEOUT_ERR_MASK   0x00000002
#define EMMC_HOSTIF_FORCE_EVENTS_FORCE_CMD_TIMEOUT_ERR_SHIFT  1
#define EMMC_HOSTIF_FORCE_EVENTS_FORCE_CMD_TIMEOUT_ERR_DEFAULT 0x00000000

/* EMMC_HOSTIF :: FORCE_EVENTS :: FORCE_CMD_NOT_EXEC_ERR [00:00] */
#define EMMC_HOSTIF_FORCE_EVENTS_FORCE_CMD_NOT_EXEC_ERR_MASK  0x00000001
#define EMMC_HOSTIF_FORCE_EVENTS_FORCE_CMD_NOT_EXEC_ERR_SHIFT 0
#define EMMC_HOSTIF_FORCE_EVENTS_FORCE_CMD_NOT_EXEC_ERR_DEFAULT 0x00000000

   uint32 emmc_host_adma_err_stat;         /* 0x54 ADMA Error Status Bits                                          */
/***************************************************************************
 *ADMA_ERR_STAT - ADMA Error Status Bits
 ***************************************************************************/
/* EMMC_HOSTIF :: ADMA_ERR_STAT :: reserved0 [31:03] */
#define EMMC_HOSTIF_ADMA_ERR_STAT_reserved0_MASK              0xfffffff8
#define EMMC_HOSTIF_ADMA_ERR_STAT_reserved0_SHIFT             3

/* EMMC_HOSTIF :: ADMA_ERR_STAT :: LENGTH_MATCH_ERR [02:02] */
#define EMMC_HOSTIF_ADMA_ERR_STAT_LENGTH_MATCH_ERR_MASK       0x00000004
#define EMMC_HOSTIF_ADMA_ERR_STAT_LENGTH_MATCH_ERR_SHIFT      2
#define EMMC_HOSTIF_ADMA_ERR_STAT_LENGTH_MATCH_ERR_DEFAULT    0x00000000

/* EMMC_HOSTIF :: ADMA_ERR_STAT :: STATE_ERR [01:00] */
#define EMMC_HOSTIF_ADMA_ERR_STAT_STATE_ERR_MASK              0x00000003
#define EMMC_HOSTIF_ADMA_ERR_STAT_STATE_ERR_SHIFT             0
#define EMMC_HOSTIF_ADMA_ERR_STAT_STATE_ERR_DEFAULT           0x00000000

   uint32 emmc_host_adma_sysaddr_lo;       /* 0x58 ADMA System Address Low Bits                                    */
/***************************************************************************
 *ADMA_SYSADDR_LO - ADMA System Address Low Bits
 ***************************************************************************/
/* EMMC_HOSTIF :: ADMA_SYSADDR_LO :: ADMA_SYSADDR_LO [31:00] */
#define EMMC_HOSTIF_ADMA_SYSADDR_LO_ADMA_SYSADDR_LO_MASK      0xffffffff
#define EMMC_HOSTIF_ADMA_SYSADDR_LO_ADMA_SYSADDR_LO_SHIFT     0
#define EMMC_HOSTIF_ADMA_SYSADDR_LO_ADMA_SYSADDR_LO_DEFAULT   0x00000000

   uint32 emmc_host_adma_sysaddr_hi;       /* 0x5c ADMA System Address High Bits                                   */
/***************************************************************************
 *ADMA_SYSADDR_HI - ADMA System Address High Bits
 ***************************************************************************/
/* EMMC_HOSTIF :: ADMA_SYSADDR_HI :: ADMA_SYSADDR_HI [31:00] */
#define EMMC_HOSTIF_ADMA_SYSADDR_HI_ADMA_SYSADDR_HI_MASK      0xffffffff
#define EMMC_HOSTIF_ADMA_SYSADDR_HI_ADMA_SYSADDR_HI_SHIFT     0
#define EMMC_HOSTIF_ADMA_SYSADDR_HI_ADMA_SYSADDR_HI_DEFAULT   0x00000000

   uint32 emmc_host_preset_init_default;   /* 0x60 Preset Values for init and default speed                        */
/***************************************************************************
 *PRESET_INIT_DEFAULT - Preset Values for init and default speed
 ***************************************************************************/
/* EMMC_HOSTIF :: PRESET_INIT_DEFAULT :: PRESET_DEFAULT_SPEED [31:16] */
#define EMMC_HOSTIF_PRESET_INIT_DEFAULT_PRESET_DEFAULT_SPEED_MASK 0xffff0000
#define EMMC_HOSTIF_PRESET_INIT_DEFAULT_PRESET_DEFAULT_SPEED_SHIFT 16
#define EMMC_HOSTIF_PRESET_INIT_DEFAULT_PRESET_DEFAULT_SPEED_DEFAULT 0x00000002

/* EMMC_HOSTIF :: PRESET_INIT_DEFAULT :: PRESET_INIT [15:00] */
#define EMMC_HOSTIF_PRESET_INIT_DEFAULT_PRESET_INIT_MASK      0x0000ffff
#define EMMC_HOSTIF_PRESET_INIT_DEFAULT_PRESET_INIT_SHIFT     0
#define EMMC_HOSTIF_PRESET_INIT_DEFAULT_PRESET_INIT_DEFAULT   0x00000002

   uint32 emmc_host_preset_high_speed;     /* 0x64 Preset Values for high speed and SDR12                          */
/***************************************************************************
 *PRESET_HIGH_SPEED - Preset Values for high speed and SDR12
 ***************************************************************************/
/* EMMC_HOSTIF :: PRESET_HIGH_SPEED :: PRESET_SDR12 [31:16] */
#define EMMC_HOSTIF_PRESET_HIGH_SPEED_PRESET_SDR12_MASK       0xffff0000
#define EMMC_HOSTIF_PRESET_HIGH_SPEED_PRESET_SDR12_SHIFT      16
#define EMMC_HOSTIF_PRESET_HIGH_SPEED_PRESET_SDR12_DEFAULT    0x00000003

/* EMMC_HOSTIF :: PRESET_HIGH_SPEED :: PRESET_HIGH_SPEED [15:00] */
#define EMMC_HOSTIF_PRESET_HIGH_SPEED_PRESET_HIGH_SPEED_MASK  0x0000ffff
#define EMMC_HOSTIF_PRESET_HIGH_SPEED_PRESET_HIGH_SPEED_SHIFT 0
#define EMMC_HOSTIF_PRESET_HIGH_SPEED_PRESET_HIGH_SPEED_DEFAULT 0x00000001

   uint32 emmc_host_preset_sdr25_50;       /* 0x68 Preset Values for SDR25 and SDR50                               */
/***************************************************************************
 *PRESET_SDR25_50 - Preset Values for SDR25 and SDR50
 ***************************************************************************/
/* EMMC_HOSTIF :: PRESET_SDR25_50 :: PRESET_SDR50 [31:16] */
#define EMMC_HOSTIF_PRESET_SDR25_50_PRESET_SDR50_MASK         0xffff0000
#define EMMC_HOSTIF_PRESET_SDR25_50_PRESET_SDR50_SHIFT        16
#define EMMC_HOSTIF_PRESET_SDR25_50_PRESET_SDR50_DEFAULT      0x00000000

/* EMMC_HOSTIF :: PRESET_SDR25_50 :: PRESET_SDR25 [15:00] */
#define EMMC_HOSTIF_PRESET_SDR25_50_PRESET_SDR25_MASK         0x0000ffff
#define EMMC_HOSTIF_PRESET_SDR25_50_PRESET_SDR25_SHIFT        0
#define EMMC_HOSTIF_PRESET_SDR25_50_PRESET_SDR25_DEFAULT      0x00000002

   uint32 emmc_host_preset_sdr104_ddr50;   /* 0x6c Preset Values for SDR104 and DDR50                              */
/***************************************************************************
 *PRESET_SDR104_DDR50 - Preset Values for SDR104 and DDR50
 ***************************************************************************/
/* EMMC_HOSTIF :: PRESET_SDR104_DDR50 :: PRESET_SDR50 [31:16] */
#define EMMC_HOSTIF_PRESET_SDR104_DDR50_PRESET_SDR50_MASK     0xffff0000
#define EMMC_HOSTIF_PRESET_SDR104_DDR50_PRESET_SDR50_SHIFT    16
#define EMMC_HOSTIF_PRESET_SDR104_DDR50_PRESET_SDR50_DEFAULT  0x00000002

/* EMMC_HOSTIF :: PRESET_SDR104_DDR50 :: PRESET_SDR25 [15:00] */
#define EMMC_HOSTIF_PRESET_SDR104_DDR50_PRESET_SDR25_MASK     0x0000ffff
#define EMMC_HOSTIF_PRESET_SDR104_DDR50_PRESET_SDR25_SHIFT    0
#define EMMC_HOSTIF_PRESET_SDR104_DDR50_PRESET_SDR25_DEFAULT  0x00000000

   uint32 emmc_host_boot_timeout;          /* 0x70 DAT line inactivity timeout on boot                             */
/***************************************************************************
 *BOOT_TIMEOUT - DAT line inactivity timeout on boot
 ***************************************************************************/
/* EMMC_HOSTIF :: BOOT_TIMEOUT :: BOOT_TIMEOUT [31:00] */
#define EMMC_HOSTIF_BOOT_TIMEOUT_BOOT_TIMEOUT_MASK            0xffffffff
#define EMMC_HOSTIF_BOOT_TIMEOUT_BOOT_TIMEOUT_SHIFT           0
#define EMMC_HOSTIF_BOOT_TIMEOUT_BOOT_TIMEOUT_DEFAULT         0x00000000

   uint32 emmc_host_debug_select;          /* 0x74 Debug probe output selection                                    */
/***************************************************************************
 *DEBUG_SELECT - Debug probe output selection
 ***************************************************************************/
/* EMMC_HOSTIF :: DEBUG_SELECT :: reserved0 [31:01] */
#define EMMC_HOSTIF_DEBUG_SELECT_reserved0_MASK               0xfffffffe
#define EMMC_HOSTIF_DEBUG_SELECT_reserved0_SHIFT              1

/* EMMC_HOSTIF :: DEBUG_SELECT :: DEBUG_SEL [00:00] */
#define EMMC_HOSTIF_DEBUG_SELECT_DEBUG_SEL_MASK               0x00000001
#define EMMC_HOSTIF_DEBUG_SELECT_DEBUG_SEL_SHIFT              0
#define EMMC_HOSTIF_DEBUG_SELECT_DEBUG_SEL_DEFAULT            0x00000000

   uint32 unused1[26];                     /* 0x78 - 0xdc                                                          */
   
   uint32 emmc_host_shared_bus_ctrl;       /* 0xe0 shared bus control                                              */
/***************************************************************************
 *SHARED_BUS_CTRL - shared bus control
 ***************************************************************************/
/* EMMC_HOSTIF :: SHARED_BUS_CTRL :: reserved0 [31:31] */
#define EMMC_HOSTIF_SHARED_BUS_CTRL_reserved0_MASK            0x80000000
#define EMMC_HOSTIF_SHARED_BUS_CTRL_reserved0_SHIFT           31

/* EMMC_HOSTIF :: SHARED_BUS_CTRL :: BACK_END_PWR_CTRL [30:24] */
#define EMMC_HOSTIF_SHARED_BUS_CTRL_BACK_END_PWR_CTRL_MASK    0x7f000000
#define EMMC_HOSTIF_SHARED_BUS_CTRL_BACK_END_PWR_CTRL_SHIFT   24
#define EMMC_HOSTIF_SHARED_BUS_CTRL_BACK_END_PWR_CTRL_DEFAULT 0x00000000

/* EMMC_HOSTIF :: SHARED_BUS_CTRL :: reserved1 [23:23] */
#define EMMC_HOSTIF_SHARED_BUS_CTRL_reserved1_MASK            0x00800000
#define EMMC_HOSTIF_SHARED_BUS_CTRL_reserved1_SHIFT           23

/* EMMC_HOSTIF :: SHARED_BUS_CTRL :: INT_PIN_SEL [22:20] */
#define EMMC_HOSTIF_SHARED_BUS_CTRL_INT_PIN_SEL_MASK          0x00700000
#define EMMC_HOSTIF_SHARED_BUS_CTRL_INT_PIN_SEL_SHIFT         20
#define EMMC_HOSTIF_SHARED_BUS_CTRL_INT_PIN_SEL_DEFAULT       0x00000000

/* EMMC_HOSTIF :: SHARED_BUS_CTRL :: reserved2 [19:19] */
#define EMMC_HOSTIF_SHARED_BUS_CTRL_reserved2_MASK            0x00080000
#define EMMC_HOSTIF_SHARED_BUS_CTRL_reserved2_SHIFT           19

/* EMMC_HOSTIF :: SHARED_BUS_CTRL :: CLK_PIN_SEL [18:16] */
#define EMMC_HOSTIF_SHARED_BUS_CTRL_CLK_PIN_SEL_MASK          0x00070000
#define EMMC_HOSTIF_SHARED_BUS_CTRL_CLK_PIN_SEL_SHIFT         16
#define EMMC_HOSTIF_SHARED_BUS_CTRL_CLK_PIN_SEL_DEFAULT       0x00000000

/* EMMC_HOSTIF :: SHARED_BUS_CTRL :: reserved3 [15:15] */
#define EMMC_HOSTIF_SHARED_BUS_CTRL_reserved3_MASK            0x00008000
#define EMMC_HOSTIF_SHARED_BUS_CTRL_reserved3_SHIFT           15

/* EMMC_HOSTIF :: SHARED_BUS_CTRL :: BUS_WIDTH_PRESET [14:08] */
#define EMMC_HOSTIF_SHARED_BUS_CTRL_BUS_WIDTH_PRESET_MASK     0x00007f00
#define EMMC_HOSTIF_SHARED_BUS_CTRL_BUS_WIDTH_PRESET_SHIFT    8
#define EMMC_HOSTIF_SHARED_BUS_CTRL_BUS_WIDTH_PRESET_DEFAULT  0x00000000

/* EMMC_HOSTIF :: SHARED_BUS_CTRL :: reserved4 [07:06] */
#define EMMC_HOSTIF_SHARED_BUS_CTRL_reserved4_MASK            0x000000c0
#define EMMC_HOSTIF_SHARED_BUS_CTRL_reserved4_SHIFT           6

/* EMMC_HOSTIF :: SHARED_BUS_CTRL :: NUM_INT_PINS [05:04] */
#define EMMC_HOSTIF_SHARED_BUS_CTRL_NUM_INT_PINS_MASK         0x00000030
#define EMMC_HOSTIF_SHARED_BUS_CTRL_NUM_INT_PINS_SHIFT        4
#define EMMC_HOSTIF_SHARED_BUS_CTRL_NUM_INT_PINS_DEFAULT      0x00000000

/* EMMC_HOSTIF :: SHARED_BUS_CTRL :: reserved5 [03:03] */
#define EMMC_HOSTIF_SHARED_BUS_CTRL_reserved5_MASK            0x00000008
#define EMMC_HOSTIF_SHARED_BUS_CTRL_reserved5_SHIFT           3

/* EMMC_HOSTIF :: SHARED_BUS_CTRL :: NUM_CLK_PINS [02:00] */
#define EMMC_HOSTIF_SHARED_BUS_CTRL_NUM_CLK_PINS_MASK         0x00000007
#define EMMC_HOSTIF_SHARED_BUS_CTRL_NUM_CLK_PINS_SHIFT        0
#define EMMC_HOSTIF_SHARED_BUS_CTRL_NUM_CLK_PINS_DEFAULT      0x00000000

   uint32 unused2[3];                      /* 0xe4 - 0xec                                                          */
   
   uint32 emmc_host_spi_interrupt;         /* 0xf0 SPI Interrupt support                                           */
/***************************************************************************
 *SPI_INTERRUPT - SPI Interrupt support
 ***************************************************************************/
/* EMMC_HOSTIF :: SPI_INTERRUPT :: reserved0 [31:08] */
#define EMMC_HOSTIF_SPI_INTERRUPT_reserved0_MASK              0xffffff00
#define EMMC_HOSTIF_SPI_INTERRUPT_reserved0_SHIFT             8

/* EMMC_HOSTIF :: SPI_INTERRUPT :: SPI_INT [07:00] */
#define EMMC_HOSTIF_SPI_INTERRUPT_SPI_INT_MASK                0x000000ff
#define EMMC_HOSTIF_SPI_INTERRUPT_SPI_INT_SHIFT               0
#define EMMC_HOSTIF_SPI_INTERRUPT_SPI_INT_DEFAULT             0x00000000

   uint32 unused3[2];                      /* 0xf4 - 0xf8                                                          */
   
   uint32 emmc_host_version_status;        /* 0xfc Controller Version and Slot Status                              */   
/***************************************************************************
 *VERSION_STATUS - Controller Version and Slot Status
 ***************************************************************************/
/* EMMC_HOSTIF :: VERSION_STATUS :: VENDOR_VERSION [31:24] */
#define EMMC_HOSTIF_VERSION_STATUS_VENDOR_VERSION_MASK        0xff000000
#define EMMC_HOSTIF_VERSION_STATUS_VENDOR_VERSION_SHIFT       24
#define EMMC_HOSTIF_VERSION_STATUS_VENDOR_VERSION_DEFAULT     0x000000a9

/* EMMC_HOSTIF :: VERSION_STATUS :: CONTROLLER_VERSION [23:16] */
#define EMMC_HOSTIF_VERSION_STATUS_CONTROLLER_VERSION_MASK    0x00ff0000
#define EMMC_HOSTIF_VERSION_STATUS_CONTROLLER_VERSION_SHIFT   16
#define EMMC_HOSTIF_VERSION_STATUS_CONTROLLER_VERSION_DEFAULT 0x00000002

/* EMMC_HOSTIF :: VERSION_STATUS :: reserved0 [15:08] */
#define EMMC_HOSTIF_VERSION_STATUS_reserved0_MASK             0x0000ff00
#define EMMC_HOSTIF_VERSION_STATUS_reserved0_SHIFT            8

/* EMMC_HOSTIF :: VERSION_STATUS :: SLOT_INTS [07:00] */
#define EMMC_HOSTIF_VERSION_STATUS_SLOT_INTS_MASK             0x000000ff
#define EMMC_HOSTIF_VERSION_STATUS_SLOT_INTS_SHIFT            0
#define EMMC_HOSTIF_VERSION_STATUS_SLOT_INTS_DEFAULT          0x00000000

} EmmcHostIfRegs;
#define EMMC_HOSTIF  ((volatile EmmcHostIfRegs *const) EMMC_HOSTIF_BASE)
 
typedef struct EmmcTopCfgRegs {
   uint32 emmc_top_cfg_sdio_emmc_ctrl1;    /* 0x00 SDIO EMMC Control Register                        */
/***************************************************************************
 *SDIO_EMMC_CTRL1 - SDIO EMMC Control Register
 ***************************************************************************/
/* SDIO_0_CFG :: SDIO_EMMC_CTRL1 :: SDCD_N_TEST_SEL_EN [31:31] */
#define EMMC_TOP_CFG_SDIO_EMMC_CTRL1_SDCD_N_TEST_SEL_EN_MASK    0x80000000
#define EMMC_TOP_CFG_SDIO_EMMC_CTRL1_SDCD_N_TEST_SEL_EN_SHIFT   31
#define EMMC_TOP_CFG_SDIO_EMMC_CTRL1_SDCD_N_TEST_SEL_EN_DEFAULT 0x00000000

/* SDIO_0_CFG :: SDIO_EMMC_CTRL1 :: SDCD_N_TEST_LEV [30:30] */
#define EMMC_TOP_CFG_SDIO_EMMC_CTRL1_SDCD_N_TEST_LEV_MASK       0x40000000
#define EMMC_TOP_CFG_SDIO_EMMC_CTRL1_SDCD_N_TEST_LEV_SHIFT      30
#define EMMC_TOP_CFG_SDIO_EMMC_CTRL1_SDCD_N_TEST_LEV_DEFAULT    0x00000001

/* SDIO_0_CFG :: SDIO_EMMC_CTRL1 :: reserved0 [29:29] */
#define EMMC_TOP_CFG_SDIO_EMMC_CTRL1_reserved0_MASK             0x20000000
#define EMMC_TOP_CFG_SDIO_EMMC_CTRL1_reserved0_SHIFT            29

/* SDIO_0_CFG :: SDIO_EMMC_CTRL1 :: RETUNING_REQ [28:28] */
#define EMMC_TOP_CFG_SDIO_EMMC_CTRL1_RETUNING_REQ_MASK          0x10000000
#define EMMC_TOP_CFG_SDIO_EMMC_CTRL1_RETUNING_REQ_SHIFT         28
#define EMMC_TOP_CFG_SDIO_EMMC_CTRL1_RETUNING_REQ_DEFAULT       0x00000000

/* SDIO_0_CFG :: SDIO_EMMC_CTRL1 :: DDR_TAP_DELAY [27:24] */
#define EMMC_TOP_CFG_SDIO_EMMC_CTRL1_DDR_TAP_DELAY_MASK         0x0f000000
#define EMMC_TOP_CFG_SDIO_EMMC_CTRL1_DDR_TAP_DELAY_SHIFT        24
#define EMMC_TOP_CFG_SDIO_EMMC_CTRL1_DDR_TAP_DELAY_DEFAULT      0x00000000

/* SDIO_0_CFG :: SDIO_EMMC_CTRL1 :: DELAY_CTRL [23:21] */
#define EMMC_TOP_CFG_SDIO_EMMC_CTRL1_DELAY_CTRL_MASK            0x00e00000
#define EMMC_TOP_CFG_SDIO_EMMC_CTRL1_DELAY_CTRL_SHIFT           21
#define EMMC_TOP_CFG_SDIO_EMMC_CTRL1_DELAY_CTRL_DEFAULT         0x00000000

/* SDIO_0_CFG :: SDIO_EMMC_CTRL1 :: HREADY_IDLE_ENA [20:20] */
#define EMMC_TOP_CFG_SDIO_EMMC_CTRL1_HREADY_IDLE_ENA_MASK       0x00100000
#define EMMC_TOP_CFG_SDIO_EMMC_CTRL1_HREADY_IDLE_ENA_SHIFT      20
#define EMMC_TOP_CFG_SDIO_EMMC_CTRL1_HREADY_IDLE_ENA_DEFAULT    0x00000001

/* SDIO_0_CFG :: SDIO_EMMC_CTRL1 :: HREADY_IDLE_PULSE [19:19] */
#define EMMC_TOP_CFG_SDIO_EMMC_CTRL1_HREADY_IDLE_PULSE_MASK     0x00080000
#define EMMC_TOP_CFG_SDIO_EMMC_CTRL1_HREADY_IDLE_PULSE_SHIFT    19
#define EMMC_TOP_CFG_SDIO_EMMC_CTRL1_HREADY_IDLE_PULSE_DEFAULT  0x00000000

/* SDIO_0_CFG :: SDIO_EMMC_CTRL1 :: DATA_PENDING [18:18] */
#define EMMC_TOP_CFG_SDIO_EMMC_CTRL1_DATA_PENDING_MASK          0x00040000
#define EMMC_TOP_CFG_SDIO_EMMC_CTRL1_DATA_PENDING_SHIFT         18
#define EMMC_TOP_CFG_SDIO_EMMC_CTRL1_DATA_PENDING_DEFAULT       0x00000000

/* SDIO_0_CFG :: SDIO_EMMC_CTRL1 :: WR_FLUSH [17:17] */
#define EMMC_TOP_CFG_SDIO_EMMC_CTRL1_WR_FLUSH_MASK              0x00020000
#define EMMC_TOP_CFG_SDIO_EMMC_CTRL1_WR_FLUSH_SHIFT             17
#define EMMC_TOP_CFG_SDIO_EMMC_CTRL1_WR_FLUSH_DEFAULT           0x00000000

/* SDIO_0_CFG :: SDIO_EMMC_CTRL1 :: MF_NUM_WR [16:16] */
#define EMMC_TOP_CFG_SDIO_EMMC_CTRL1_MF_NUM_WR_MASK             0x00010000
#define EMMC_TOP_CFG_SDIO_EMMC_CTRL1_MF_NUM_WR_SHIFT            16
#define EMMC_TOP_CFG_SDIO_EMMC_CTRL1_MF_NUM_WR_DEFAULT          0x00000000

/* SDIO_0_CFG :: SDIO_EMMC_CTRL1 :: WORD_ABO [15:15] */
#define EMMC_TOP_CFG_SDIO_EMMC_CTRL1_WORD_ABO_MASK              0x00008000
#define EMMC_TOP_CFG_SDIO_EMMC_CTRL1_WORD_ABO_SHIFT             15
#define EMMC_TOP_CFG_SDIO_EMMC_CTRL1_WORD_ABO_DEFAULT           0x00000000

/* SDIO_0_CFG :: SDIO_EMMC_CTRL1 :: FRAME_NBO [14:14] */
#define EMMC_TOP_CFG_SDIO_EMMC_CTRL1_FRAME_NBO_MASK             0x00004000
#define EMMC_TOP_CFG_SDIO_EMMC_CTRL1_FRAME_NBO_SHIFT            14
#define EMMC_TOP_CFG_SDIO_EMMC_CTRL1_FRAME_NBO_DEFAULT          0x00000000

/* SDIO_0_CFG :: SDIO_EMMC_CTRL1 :: FRAME_NHW [13:13] */
#define EMMC_TOP_CFG_SDIO_EMMC_CTRL1_FRAME_NHW_MASK             0x00002000
#define EMMC_TOP_CFG_SDIO_EMMC_CTRL1_FRAME_NHW_SHIFT            13
#define EMMC_TOP_CFG_SDIO_EMMC_CTRL1_FRAME_NHW_DEFAULT          0x00000001

/* SDIO_0_CFG :: SDIO_EMMC_CTRL1 :: BUFFER_ABO [12:12] */
#define EMMC_TOP_CFG_SDIO_EMMC_CTRL1_BUFFER_ABO_MASK            0x00001000
#define EMMC_TOP_CFG_SDIO_EMMC_CTRL1_BUFFER_ABO_SHIFT           12
#define EMMC_TOP_CFG_SDIO_EMMC_CTRL1_BUFFER_ABO_DEFAULT         0x00000001

/* SDIO_0_CFG :: SDIO_EMMC_CTRL1 :: SCB_BUF_ACC [11:11] */
#define EMMC_TOP_CFG_SDIO_EMMC_CTRL1_SCB_BUF_ACC_MASK           0x00000800
#define EMMC_TOP_CFG_SDIO_EMMC_CTRL1_SCB_BUF_ACC_SHIFT          11
#define EMMC_TOP_CFG_SDIO_EMMC_CTRL1_SCB_BUF_ACC_DEFAULT        0x00000001

/* SDIO_0_CFG :: SDIO_EMMC_CTRL1 :: SCB_SEQ_EN [10:10] */
#define EMMC_TOP_CFG_SDIO_EMMC_CTRL1_SCB_SEQ_EN_MASK            0x00000400
#define EMMC_TOP_CFG_SDIO_EMMC_CTRL1_SCB_SEQ_EN_SHIFT           10
#define EMMC_TOP_CFG_SDIO_EMMC_CTRL1_SCB_SEQ_EN_DEFAULT         0x00000001

/* SDIO_0_CFG :: SDIO_EMMC_CTRL1 :: SCB_RD_THRESH [09:05] */
#define EMMC_TOP_CFG_SDIO_EMMC_CTRL1_SCB_RD_THRESH_MASK         0x000003e0
#define EMMC_TOP_CFG_SDIO_EMMC_CTRL1_SCB_RD_THRESH_SHIFT        5
#define EMMC_TOP_CFG_SDIO_EMMC_CTRL1_SCB_RD_THRESH_DEFAULT      0x00000002

/* SDIO_0_CFG :: SDIO_EMMC_CTRL1 :: SCB_SIZE [04:00] */
#define EMMC_TOP_CFG_SDIO_EMMC_CTRL1_SCB_SIZE_MASK              0x0000001f
#define EMMC_TOP_CFG_SDIO_EMMC_CTRL1_SCB_SIZE_SHIFT             0
#define EMMC_TOP_CFG_SDIO_EMMC_CTRL1_SCB_SIZE_DEFAULT           0x00000004

   uint32 emmc_top_cfg_sdio_emmc_ctrl2;    /* 0x04 SDIO EMMC Control Register                        */
/***************************************************************************
 *SDIO_EMMC_CTRL2 - SDIO EMMC Control Register
 ***************************************************************************/
/* SDIO_0_CFG :: SDIO_EMMC_CTRL2 :: reserved0 [31:08] */
#define EMMC_TOP_CFG_SDIO_EMMC_CTRL2_reserved0_MASK             0xffffff00
#define EMMC_TOP_CFG_SDIO_EMMC_CTRL2_reserved0_SHIFT            8

/* SDIO_0_CFG :: SDIO_EMMC_CTRL2 :: REG_ADDR_MAP_BYTE [07:06] */
#define EMMC_TOP_CFG_SDIO_EMMC_CTRL2_REG_ADDR_MAP_BYTE_MASK     0x000000c0
#define EMMC_TOP_CFG_SDIO_EMMC_CTRL2_REG_ADDR_MAP_BYTE_SHIFT    6
#define EMMC_TOP_CFG_SDIO_EMMC_CTRL2_REG_ADDR_MAP_BYTE_DEFAULT  0x00000000

/* SDIO_0_CFG :: SDIO_EMMC_CTRL2 :: reserved1 [05:05] */
#define EMMC_TOP_CFG_SDIO_EMMC_CTRL2_reserved1_MASK             0x00000020
#define EMMC_TOP_CFG_SDIO_EMMC_CTRL2_reserved1_SHIFT            5

/* SDIO_0_CFG :: SDIO_EMMC_CTRL2 :: REG_ADDR_MAP_HW [04:04] */
#define EMMC_TOP_CFG_SDIO_EMMC_CTRL2_REG_ADDR_MAP_HW_MASK       0x00000010
#define EMMC_TOP_CFG_SDIO_EMMC_CTRL2_REG_ADDR_MAP_HW_SHIFT      4
#define EMMC_TOP_CFG_SDIO_EMMC_CTRL2_REG_ADDR_MAP_HW_DEFAULT    0x00000000

/* SDIO_0_CFG :: SDIO_EMMC_CTRL2 :: REG_DATA_SWAP_RD [03:02] */
#define EMMC_TOP_CFG_SDIO_EMMC_CTRL2_REG_DATA_SWAP_RD_MASK      0x0000000c
#define EMMC_TOP_CFG_SDIO_EMMC_CTRL2_REG_DATA_SWAP_RD_SHIFT     2
#define EMMC_TOP_CFG_SDIO_EMMC_CTRL2_REG_DATA_SWAP_RD_DEFAULT   0x00000000

/* SDIO_0_CFG :: SDIO_EMMC_CTRL2 :: REG_DATA_SWAP_WR [01:00] */
#define EMMC_TOP_CFG_SDIO_EMMC_CTRL2_REG_DATA_SWAP_WR_MASK      0x00000003
#define EMMC_TOP_CFG_SDIO_EMMC_CTRL2_REG_DATA_SWAP_WR_SHIFT     0
#define EMMC_TOP_CFG_SDIO_EMMC_CTRL2_REG_DATA_SWAP_WR_DEFAULT   0x00000000

   uint32 emmc_top_cfg_tp_out_sel;         /* 0x08 SDIO TP_OUT Control Register                      */
/***************************************************************************
 *TP_OUT_SEL - SDIO TP_OUT Control Register
 ***************************************************************************/
/* SDIO_0_CFG :: TP_OUT_SEL :: reserved0 [31:01] */
#define EMMC_TOP_CFG_TP_OUT_SEL_reserved0_MASK                  0xfffffffe
#define EMMC_TOP_CFG_TP_OUT_SEL_reserved0_SHIFT                 1

/* SDIO_0_CFG :: TP_OUT_SEL :: TP_OUT_SELECT [00:00] */
#define EMMC_TOP_CFG_TP_OUT_SEL_TP_OUT_SELECT_MASK              0x00000001
#define EMMC_TOP_CFG_TP_OUT_SEL_TP_OUT_SELECT_SHIFT             0
#define EMMC_TOP_CFG_TP_OUT_SEL_TP_OUT_SELECT_DEFAULT           0x00000000

   uint32 emmc_top_cfg_cap_reg_override;   /* 0x0c SDIO CAPABILITIES override Register               */
   
   uint32 emmc_top_cfg_cap_reg0;           /* 0x10 SDIO CAPABILITIES override Register[31:0]         */
/***************************************************************************
 *CAP_REG0 - SDIO CAPABILITIES override Register
 ***************************************************************************/
/* SDIO_0_CFG :: CAP_REG0 :: DDR50_SUPPORT [31:31] */
#define EMMC_TOP_CFG_CAP_REG0_DDR50_SUPPORT_MASK                0x80000000
#define EMMC_TOP_CFG_CAP_REG0_DDR50_SUPPORT_SHIFT               31
#define EMMC_TOP_CFG_CAP_REG0_DDR50_SUPPORT_DEFAULT             0x00000001

/* SDIO_0_CFG :: CAP_REG0 :: SD104_SUPPORT [30:30] */
#define EMMC_TOP_CFG_CAP_REG0_SD104_SUPPORT_MASK                0x40000000
#define EMMC_TOP_CFG_CAP_REG0_SD104_SUPPORT_SHIFT               30
#define EMMC_TOP_CFG_CAP_REG0_SD104_SUPPORT_DEFAULT             0x00000000

/* SDIO_0_CFG :: CAP_REG0 :: SDR50 [29:29] */
#define EMMC_TOP_CFG_CAP_REG0_SDR50_MASK                        0x20000000
#define EMMC_TOP_CFG_CAP_REG0_SDR50_SHIFT                       29
#define EMMC_TOP_CFG_CAP_REG0_SDR50_DEFAULT                     0x00000001

/* SDIO_0_CFG :: CAP_REG0 :: SLOT_TYPE [28:27] */
#define EMMC_TOP_CFG_CAP_REG0_SLOT_TYPE_MASK                    0x18000000
#define EMMC_TOP_CFG_CAP_REG0_SLOT_TYPE_SHIFT                   27
#define EMMC_TOP_CFG_CAP_REG0_SLOT_TYPE_DEFAULT                 0x00000002

/* SDIO_0_CFG :: CAP_REG0 :: ASYNCH_INT_SUPPORT [26:26] */
#define EMMC_TOP_CFG_CAP_REG0_ASYNCH_INT_SUPPORT_MASK           0x04000000
#define EMMC_TOP_CFG_CAP_REG0_ASYNCH_INT_SUPPORT_SHIFT          26
#define EMMC_TOP_CFG_CAP_REG0_ASYNCH_INT_SUPPORT_DEFAULT        0x00000000

/* SDIO_0_CFG :: CAP_REG0 :: 64B_SYS_BUS_SUPPORT [25:25] */
#define EMMC_TOP_CFG_CAP_REG0_64B_SYS_BUS_SUPPORT_MASK          0x02000000
#define EMMC_TOP_CFG_CAP_REG0_64B_SYS_BUS_SUPPORT_SHIFT         25
#define EMMC_TOP_CFG_CAP_REG0_64B_SYS_BUS_SUPPORT_DEFAULT       0x00000000

/* SDIO_0_CFG :: CAP_REG0 :: 1_8V_SUPPORT [24:24] */
#define EMMC_TOP_CFG_CAP_REG0_1_8V_SUPPORT_MASK                 0x01000000
#define EMMC_TOP_CFG_CAP_REG0_1_8V_SUPPORT_SHIFT                24
#define EMMC_TOP_CFG_CAP_REG0_1_8V_SUPPORT_DEFAULT              0x00000001

/* SDIO_0_CFG :: CAP_REG0 :: 3_0V_SUPPORT [23:23] */
#define EMMC_TOP_CFG_CAP_REG0_3_0V_SUPPORT_MASK                 0x00800000
#define EMMC_TOP_CFG_CAP_REG0_3_0V_SUPPORT_SHIFT                23
#define EMMC_TOP_CFG_CAP_REG0_3_0V_SUPPORT_DEFAULT              0x00000000

/* SDIO_0_CFG :: CAP_REG0 :: 3_3V_SUPPORT [22:22] */
#define EMMC_TOP_CFG_CAP_REG0_3_3V_SUPPORT_MASK                 0x00400000
#define EMMC_TOP_CFG_CAP_REG0_3_3V_SUPPORT_SHIFT                22
#define EMMC_TOP_CFG_CAP_REG0_3_3V_SUPPORT_DEFAULT              0x00000001

/* SDIO_0_CFG :: CAP_REG0 :: SUSP_RES_SUPPORT [21:21] */
#define EMMC_TOP_CFG_CAP_REG0_SUSP_RES_SUPPORT_MASK             0x00200000
#define EMMC_TOP_CFG_CAP_REG0_SUSP_RES_SUPPORT_SHIFT            21
#define EMMC_TOP_CFG_CAP_REG0_SUSP_RES_SUPPORT_DEFAULT          0x00000001

/* SDIO_0_CFG :: CAP_REG0 :: SDMA_SUPPORT [20:20] */
#define EMMC_TOP_CFG_CAP_REG0_SDMA_SUPPORT_MASK                 0x00100000
#define EMMC_TOP_CFG_CAP_REG0_SDMA_SUPPORT_SHIFT                20
#define EMMC_TOP_CFG_CAP_REG0_SDMA_SUPPORT_DEFAULT              0x00000001

/* SDIO_0_CFG :: CAP_REG0 :: HIGH_SPEED_SUPPORT [19:19] */
#define EMMC_TOP_CFG_CAP_REG0_HIGH_SPEED_SUPPORT_MASK           0x00080000
#define EMMC_TOP_CFG_CAP_REG0_HIGH_SPEED_SUPPORT_SHIFT          19
#define EMMC_TOP_CFG_CAP_REG0_HIGH_SPEED_SUPPORT_DEFAULT        0x00000001

/* SDIO_0_CFG :: CAP_REG0 :: ADMA2_SUPPORT [18:18] */
#define EMMC_TOP_CFG_CAP_REG0_ADMA2_SUPPORT_MASK                0x00040000
#define EMMC_TOP_CFG_CAP_REG0_ADMA2_SUPPORT_SHIFT               18
#define EMMC_TOP_CFG_CAP_REG0_ADMA2_SUPPORT_DEFAULT             0x00000001

/* SDIO_0_CFG :: CAP_REG0 :: EXTENDED_MEDIA_SUPPORT [17:17] */
#define EMMC_TOP_CFG_CAP_REG0_EXTENDED_MEDIA_SUPPORT_MASK       0x00020000
#define EMMC_TOP_CFG_CAP_REG0_EXTENDED_MEDIA_SUPPORT_SHIFT      17
#define EMMC_TOP_CFG_CAP_REG0_EXTENDED_MEDIA_SUPPORT_DEFAULT    0x00000001

/* SDIO_0_CFG :: CAP_REG0 :: MAX_BL [16:15] */
#define EMMC_TOP_CFG_CAP_REG0_MAX_BL_MASK                       0x00018000
#define EMMC_TOP_CFG_CAP_REG0_MAX_BL_SHIFT                      15
#define EMMC_TOP_CFG_CAP_REG0_MAX_BL_DEFAULT                    0x00000001

/* SDIO_0_CFG :: CAP_REG0 :: BASE_FREQ [14:07] */
#define EMMC_TOP_CFG_CAP_REG0_BASE_FREQ_MASK                    0x00007f80
#define EMMC_TOP_CFG_CAP_REG0_BASE_FREQ_SHIFT                   7
#define EMMC_TOP_CFG_CAP_REG0_BASE_FREQ_DEFAULT                 0x00000064

/* SDIO_0_CFG :: CAP_REG0 :: TIMEOUT_CLK_UNIT [06:06] */
#define EMMC_TOP_CFG_CAP_REG0_TIMEOUT_CLK_UNIT_MASK             0x00000040
#define EMMC_TOP_CFG_CAP_REG0_TIMEOUT_CLK_UNIT_SHIFT            6
#define EMMC_TOP_CFG_CAP_REG0_TIMEOUT_CLK_UNIT_DEFAULT          0x00000001

/* SDIO_0_CFG :: CAP_REG0 :: TIMEOUT_FREQ [05:00] */
#define EMMC_TOP_CFG_CAP_REG0_TIMEOUT_FREQ_MASK                 0x0000003f
#define EMMC_TOP_CFG_CAP_REG0_TIMEOUT_FREQ_SHIFT                0
#define EMMC_TOP_CFG_CAP_REG0_TIMEOUT_FREQ_DEFAULT              0x00000032

   uint32 emmc_top_cfg_cap_reg1;           /* 0x14 SDIO CAPABILITIES override Register[63:32]        */
/***************************************************************************
 *CAP_REG1 - SDIO CAPABILITIES override Register
 ***************************************************************************/
/* SDIO_0_CFG :: CAP_REG1 :: CAP_REG_OVERRIDE [31:31] */
#define EMMC_TOP_CFG_CAP_REG1_CAP_REG_OVERRIDE_MASK             0x80000000
#define EMMC_TOP_CFG_CAP_REG1_CAP_REG_OVERRIDE_SHIFT            31
#define EMMC_TOP_CFG_CAP_REG1_CAP_REG_OVERRIDE_DEFAULT          0x00000000

/* SDIO_0_CFG :: CAP_REG1 :: reserved0 [30:21] */
#define EMMC_TOP_CFG_CAP_REG1_reserved0_MASK                    0x7fe00000
#define EMMC_TOP_CFG_CAP_REG1_reserved0_SHIFT                   21

/* SDIO_0_CFG :: CAP_REG1 :: CAP_1_stuff [20:20] */
#define EMMC_TOP_CFG_CAP_REG1_CAP_1_stuff_MASK                  0x00100000
#define EMMC_TOP_CFG_CAP_REG1_CAP_1_stuff_SHIFT                 20
#define EMMC_TOP_CFG_CAP_REG1_CAP_1_stuff_DEFAULT               0x00000000

/* SDIO_0_CFG :: CAP_REG1 :: SPI_BLK_MODE [19:19] */
#define EMMC_TOP_CFG_CAP_REG1_SPI_BLK_MODE_MASK                 0x00080000
#define EMMC_TOP_CFG_CAP_REG1_SPI_BLK_MODE_SHIFT                19
#define EMMC_TOP_CFG_CAP_REG1_SPI_BLK_MODE_DEFAULT              0x00000000

/* SDIO_0_CFG :: CAP_REG1 :: SPI_MODE [18:18] */
#define EMMC_TOP_CFG_CAP_REG1_SPI_MODE_MASK                     0x00040000
#define EMMC_TOP_CFG_CAP_REG1_SPI_MODE_SHIFT                    18
#define EMMC_TOP_CFG_CAP_REG1_SPI_MODE_DEFAULT                  0x00000001

/* SDIO_0_CFG :: CAP_REG1 :: CLK_MULT [17:10] */
#define EMMC_TOP_CFG_CAP_REG1_CLK_MULT_MASK                     0x0003fc00
#define EMMC_TOP_CFG_CAP_REG1_CLK_MULT_SHIFT                    10
#define EMMC_TOP_CFG_CAP_REG1_CLK_MULT_DEFAULT                  0x00000000

/* SDIO_0_CFG :: CAP_REG1 :: RETUNING_MODES [09:08] */
#define EMMC_TOP_CFG_CAP_REG1_RETUNING_MODES_MASK               0x00000300
#define EMMC_TOP_CFG_CAP_REG1_RETUNING_MODES_SHIFT              8
#define EMMC_TOP_CFG_CAP_REG1_RETUNING_MODES_DEFAULT            0x00000002

/* SDIO_0_CFG :: CAP_REG1 :: USE_TUNING [07:07] */
#define EMMC_TOP_CFG_CAP_REG1_USE_TUNING_MASK                   0x00000080
#define EMMC_TOP_CFG_CAP_REG1_USE_TUNING_SHIFT                  7
#define EMMC_TOP_CFG_CAP_REG1_USE_TUNING_DEFAULT                0x00000001

/* SDIO_0_CFG :: CAP_REG1 :: RETUNING_TIMER [06:03] */
#define EMMC_TOP_CFG_CAP_REG1_RETUNING_TIMER_MASK               0x00000078
#define EMMC_TOP_CFG_CAP_REG1_RETUNING_TIMER_SHIFT              3
#define EMMC_TOP_CFG_CAP_REG1_RETUNING_TIMER_DEFAULT            0x0000000a

/* SDIO_0_CFG :: CAP_REG1 :: Driver_D_SUPPORT [02:02] */
#define EMMC_TOP_CFG_CAP_REG1_Driver_D_SUPPORT_MASK             0x00000004
#define EMMC_TOP_CFG_CAP_REG1_Driver_D_SUPPORT_SHIFT            2
#define EMMC_TOP_CFG_CAP_REG1_Driver_D_SUPPORT_DEFAULT          0x00000000

/* SDIO_0_CFG :: CAP_REG1 :: Driver_C_SUPPORT [01:01] */
#define EMMC_TOP_CFG_CAP_REG1_Driver_C_SUPPORT_MASK             0x00000002
#define EMMC_TOP_CFG_CAP_REG1_Driver_C_SUPPORT_SHIFT            1
#define EMMC_TOP_CFG_CAP_REG1_Driver_C_SUPPORT_DEFAULT          0x00000000

/* SDIO_0_CFG :: CAP_REG1 :: Driver_A_SUPPORT [00:00] */
#define EMMC_TOP_CFG_CAP_REG1_Driver_A_SUPPORT_MASK             0x00000001
#define EMMC_TOP_CFG_CAP_REG1_Driver_A_SUPPORT_SHIFT            0
#define EMMC_TOP_CFG_CAP_REG1_Driver_A_SUPPORT_DEFAULT          0x00000000

   uint32 emmc_top_cfg_preset1;            /* 0x18 SDIO PRESET_INIT/PRESET_DS override Register      */
/***************************************************************************
 *PRESET1 - SDIO CAPABILITIES override Register
 ***************************************************************************/
/* SDIO_0_CFG :: PRESET1 :: PRESET1_OVERRIDE [31:31] */
#define EMMC_TOP_CFG_PRESET1_PRESET1_OVERRIDE_MASK              0x80000000
#define EMMC_TOP_CFG_PRESET1_PRESET1_OVERRIDE_SHIFT             31
#define EMMC_TOP_CFG_PRESET1_PRESET1_OVERRIDE_DEFAULT           0x00000000

/* SDIO_0_CFG :: PRESET1 :: reserved0 [30:29] */
#define EMMC_TOP_CFG_PRESET1_reserved0_MASK                     0x60000000
#define EMMC_TOP_CFG_PRESET1_reserved0_SHIFT                    29

/* SDIO_0_CFG :: PRESET1 :: PRESET100 [28:16] */
#define EMMC_TOP_CFG_PRESET1_PRESET100_MASK                     0x1fff0000
#define EMMC_TOP_CFG_PRESET1_PRESET100_SHIFT                    16
#define EMMC_TOP_CFG_PRESET1_PRESET100_DEFAULT                  0x00000000

/* SDIO_0_CFG :: PRESET1 :: reserved1 [15:13] */
#define EMMC_TOP_CFG_PRESET1_reserved1_MASK                     0x0000e000
#define EMMC_TOP_CFG_PRESET1_reserved1_SHIFT                    13

/* SDIO_0_CFG :: PRESET1 :: PRESET50 [12:00] */
#define EMMC_TOP_CFG_PRESET1_PRESET50_MASK                      0x00001fff
#define EMMC_TOP_CFG_PRESET1_PRESET50_SHIFT                     0
#define EMMC_TOP_CFG_PRESET1_PRESET50_DEFAULT                   0x00000001

   uint32 emmc_top_cfg_preset2;            /* 0x1c SDIO PRESET_HS/PRESET_SDR12 override Register     */
/***************************************************************************
 *PRESET2 - SDIO CAPABILITIES override Register
 ***************************************************************************/
/* SDIO_0_CFG :: PRESET2 :: PRESET2_OVERRIDE [31:31] */
#define EMMC_TOP_CFG_PRESET2_PRESET2_OVERRIDE_MASK              0x80000000
#define EMMC_TOP_CFG_PRESET2_PRESET2_OVERRIDE_SHIFT             31
#define EMMC_TOP_CFG_PRESET2_PRESET2_OVERRIDE_DEFAULT           0x00000000

/* SDIO_0_CFG :: PRESET2 :: reserved0 [30:29] */
#define EMMC_TOP_CFG_PRESET2_reserved0_MASK                     0x60000000
#define EMMC_TOP_CFG_PRESET2_reserved0_SHIFT                    29

/* SDIO_0_CFG :: PRESET2 :: PRESET25 [28:16] */
#define EMMC_TOP_CFG_PRESET2_PRESET25_MASK                      0x1fff0000
#define EMMC_TOP_CFG_PRESET2_PRESET25_SHIFT                     16
#define EMMC_TOP_CFG_PRESET2_PRESET25_DEFAULT                   0x00000002

/* SDIO_0_CFG :: PRESET2 :: reserved1 [15:13] */
#define EMMC_TOP_CFG_PRESET2_reserved1_MASK                     0x0000e000
#define EMMC_TOP_CFG_PRESET2_reserved1_SHIFT                    13

/* SDIO_0_CFG :: PRESET2 :: PRESET12P5 [12:00] */
#define EMMC_TOP_CFG_PRESET2_PRESET12P5_MASK                    0x00001fff
#define EMMC_TOP_CFG_PRESET2_PRESET12P5_SHIFT                   0
#define EMMC_TOP_CFG_PRESET2_PRESET12P5_DEFAULT                 0x00000003

   uint32 emmc_top_cfg_preset3;            /* 0x20 SDIO PRESET_SDR25/PRESET_SDR50 override Register  */
   
   uint32 emmc_top_cfg_preset4;            /* 0x24 SDIO PRESET_SDR104/PRESET_DDR50 override Register */
   
   uint32 emmc_top_cfg_sd_clock_delay;     /* 0x28 SDIO Clock delay register                         */
/***************************************************************************
 *SD_CLOCK_DELAY - SDIO Clock delay register
 ***************************************************************************/
/* SDIO_0_CFG :: SD_CLOCK_DELAY :: reserved0 [31:31] */
#define EMMC_TOP_CFG_SD_CLOCK_DELAY_reserved0_MASK              0x80000000
#define EMMC_TOP_CFG_SD_CLOCK_DELAY_reserved0_SHIFT             31

/* SDIO_0_CFG :: SD_CLOCK_DELAY :: CLOCK_DELAY_OVERRIDE [30:30] */
#define EMMC_TOP_CFG_SD_CLOCK_DELAY_CLOCK_DELAY_OVERRIDE_MASK   0x40000000
#define EMMC_TOP_CFG_SD_CLOCK_DELAY_CLOCK_DELAY_OVERRIDE_SHIFT  30
#define EMMC_TOP_CFG_SD_CLOCK_DELAY_CLOCK_DELAY_OVERRIDE_DEFAULT 0x00000001

/* SDIO_0_CFG :: SD_CLOCK_DELAY :: INPUT_CLOCK_SEL [29:29] */
#define EMMC_TOP_CFG_SD_CLOCK_DELAY_INPUT_CLOCK_SEL_MASK        0x20000000
#define EMMC_TOP_CFG_SD_CLOCK_DELAY_INPUT_CLOCK_SEL_SHIFT       29
#define EMMC_TOP_CFG_SD_CLOCK_DELAY_INPUT_CLOCK_SEL_DEFAULT     0x00000000

/* SDIO_0_CFG :: SD_CLOCK_DELAY :: reserved1 [28:12] */
#define EMMC_TOP_CFG_SD_CLOCK_DELAY_reserved1_MASK              0x1ffff000
#define EMMC_TOP_CFG_SD_CLOCK_DELAY_reserved1_SHIFT             12

/* SDIO_0_CFG :: SD_CLOCK_DELAY :: OUTPUT_CLOCK_DELAY [11:08] */
#define EMMC_TOP_CFG_SD_CLOCK_DELAY_OUTPUT_CLOCK_DELAY_MASK     0x00000f00
#define EMMC_TOP_CFG_SD_CLOCK_DELAY_OUTPUT_CLOCK_DELAY_SHIFT    8
#define EMMC_TOP_CFG_SD_CLOCK_DELAY_OUTPUT_CLOCK_DELAY_DEFAULT  0x00000000

/* SDIO_0_CFG :: SD_CLOCK_DELAY :: INTERNAL_CLOCK_DELAY [07:04] */
#define EMMC_TOP_CFG_SD_CLOCK_DELAY_INTERNAL_CLOCK_DELAY_MASK   0x000000f0
#define EMMC_TOP_CFG_SD_CLOCK_DELAY_INTERNAL_CLOCK_DELAY_SHIFT  4
#define EMMC_TOP_CFG_SD_CLOCK_DELAY_INTERNAL_CLOCK_DELAY_DEFAULT 0x0000000f

/* SDIO_0_CFG :: SD_CLOCK_DELAY :: INPUT_CLOCK_DELAY [03:00] */
#define EMMC_TOP_CFG_SD_CLOCK_DELAY_INPUT_CLOCK_DELAY_MASK      0x0000000f
#define EMMC_TOP_CFG_SD_CLOCK_DELAY_INPUT_CLOCK_DELAY_SHIFT     0
#define EMMC_TOP_CFG_SD_CLOCK_DELAY_INPUT_CLOCK_DELAY_DEFAULT   0x0000000f

   uint32 emmc_top_cfg_sd_pad_drv;         /* 0x2c SDIO Clock delay register                         */
/***************************************************************************
 *SD_PAD_DRV - SDIO Clock delay register
 ***************************************************************************/
/* SDIO_0_CFG :: SD_PAD_DRV :: OVERRIDE_EN [31:31] */
#define EMMC_TOP_CFG_SD_PAD_DRV_OVERRIDE_EN_MASK                0x80000000
#define EMMC_TOP_CFG_SD_PAD_DRV_OVERRIDE_EN_SHIFT               31
#define EMMC_TOP_CFG_SD_PAD_DRV_OVERRIDE_EN_DEFAULT             0x00000000

/* SDIO_0_CFG :: SD_PAD_DRV :: reserved0 [30:23] */
#define EMMC_TOP_CFG_SD_PAD_DRV_reserved0_MASK                  0x7f800000
#define EMMC_TOP_CFG_SD_PAD_DRV_reserved0_SHIFT                 23

/* SDIO_0_CFG :: SD_PAD_DRV :: CLK_VAL [22:20] */
#define EMMC_TOP_CFG_SD_PAD_DRV_CLK_VAL_MASK                    0x00700000
#define EMMC_TOP_CFG_SD_PAD_DRV_CLK_VAL_SHIFT                   20
#define EMMC_TOP_CFG_SD_PAD_DRV_CLK_VAL_DEFAULT                 0x00000005

/* SDIO_0_CFG :: SD_PAD_DRV :: reserved1 [19:19] */
#define EMMC_TOP_CFG_SD_PAD_DRV_reserved1_MASK                  0x00080000
#define EMMC_TOP_CFG_SD_PAD_DRV_reserved1_SHIFT                 19

/* SDIO_0_CFG :: SD_PAD_DRV :: CMD_VAL [18:16] */
#define EMMC_TOP_CFG_SD_PAD_DRV_CMD_VAL_MASK                    0x00070000
#define EMMC_TOP_CFG_SD_PAD_DRV_CMD_VAL_SHIFT                   16
#define EMMC_TOP_CFG_SD_PAD_DRV_CMD_VAL_DEFAULT                 0x00000005

/* SDIO_0_CFG :: SD_PAD_DRV :: reserved2 [15:15] */
#define EMMC_TOP_CFG_SD_PAD_DRV_reserved2_MASK                  0x00008000
#define EMMC_TOP_CFG_SD_PAD_DRV_reserved2_SHIFT                 15

/* SDIO_0_CFG :: SD_PAD_DRV :: DAT3_VAL [14:12] */
#define EMMC_TOP_CFG_SD_PAD_DRV_DAT3_VAL_MASK                   0x00007000
#define EMMC_TOP_CFG_SD_PAD_DRV_DAT3_VAL_SHIFT                  12
#define EMMC_TOP_CFG_SD_PAD_DRV_DAT3_VAL_DEFAULT                0x00000005

/* SDIO_0_CFG :: SD_PAD_DRV :: reserved3 [11:11] */
#define EMMC_TOP_CFG_SD_PAD_DRV_reserved3_MASK                  0x00000800
#define EMMC_TOP_CFG_SD_PAD_DRV_reserved3_SHIFT                 11

/* SDIO_0_CFG :: SD_PAD_DRV :: DAT2_VAL [10:08] */
#define EMMC_TOP_CFG_SD_PAD_DRV_DAT2_VAL_MASK                   0x00000700
#define EMMC_TOP_CFG_SD_PAD_DRV_DAT2_VAL_SHIFT                  8
#define EMMC_TOP_CFG_SD_PAD_DRV_DAT2_VAL_DEFAULT                0x00000005

/* SDIO_0_CFG :: SD_PAD_DRV :: reserved4 [07:07] */
#define EMMC_TOP_CFG_SD_PAD_DRV_reserved4_MASK                  0x00000080
#define EMMC_TOP_CFG_SD_PAD_DRV_reserved4_SHIFT                 7

/* SDIO_0_CFG :: SD_PAD_DRV :: DAT1_VAL [06:04] */
#define EMMC_TOP_CFG_SD_PAD_DRV_DAT1_VAL_MASK                   0x00000070
#define EMMC_TOP_CFG_SD_PAD_DRV_DAT1_VAL_SHIFT                  4
#define EMMC_TOP_CFG_SD_PAD_DRV_DAT1_VAL_DEFAULT                0x00000005

/* SDIO_0_CFG :: SD_PAD_DRV :: reserved5 [03:03] */
#define EMMC_TOP_CFG_SD_PAD_DRV_reserved5_MASK                  0x00000008
#define EMMC_TOP_CFG_SD_PAD_DRV_reserved5_SHIFT                 3

/* SDIO_0_CFG :: SD_PAD_DRV :: DAT0_VAL [02:00] */
#define EMMC_TOP_CFG_SD_PAD_DRV_DAT0_VAL_MASK                   0x00000007
#define EMMC_TOP_CFG_SD_PAD_DRV_DAT0_VAL_SHIFT                  0
#define EMMC_TOP_CFG_SD_PAD_DRV_DAT0_VAL_DEFAULT                0x00000005

   uint32 emmc_top_cfg_ip_dly;             /* 0x30 SDIO Host input delay register                    */ 
/***************************************************************************
 *IP_DLY - SDIO Host input delay register
 ***************************************************************************/
/* SDIO_0_CFG :: IP_DLY :: IP_TAP_EN [31:31] */
#define EMMC_TOP_CFG_IP_DLY_IP_TAP_EN_MASK                      0x80000000
#define EMMC_TOP_CFG_IP_DLY_IP_TAP_EN_SHIFT                     31
#define EMMC_TOP_CFG_IP_DLY_IP_TAP_EN_DEFAULT                   0x00000000

/* SDIO_0_CFG :: IP_DLY :: FORCE_USE_IP_TUNE_CLK [30:30] */
#define EMMC_TOP_CFG_IP_DLY_FORCE_USE_IP_TUNE_CLK_MASK          0x40000000
#define EMMC_TOP_CFG_IP_DLY_FORCE_USE_IP_TUNE_CLK_SHIFT         30
#define EMMC_TOP_CFG_IP_DLY_FORCE_USE_IP_TUNE_CLK_DEFAULT       0x00000000

/* SDIO_0_CFG :: IP_DLY :: reserved0 [29:18] */
#define EMMC_TOP_CFG_IP_DLY_reserved0_MASK                      0x3ffc0000
#define EMMC_TOP_CFG_IP_DLY_reserved0_SHIFT                     18

/* SDIO_0_CFG :: IP_DLY :: IP_DELAY_CTRL [17:16] */
#define EMMC_TOP_CFG_IP_DLY_IP_DELAY_CTRL_MASK                  0x00030000
#define EMMC_TOP_CFG_IP_DLY_IP_DELAY_CTRL_SHIFT                 16
#define EMMC_TOP_CFG_IP_DLY_IP_DELAY_CTRL_DEFAULT               0x00000003

/* SDIO_0_CFG :: IP_DLY :: reserved1 [15:06] */
#define EMMC_TOP_CFG_IP_DLY_reserved1_MASK                      0x0000ffc0
#define EMMC_TOP_CFG_IP_DLY_reserved1_SHIFT                     6

/* SDIO_0_CFG :: IP_DLY :: IP_TAP_DELAY [05:00] */
#define EMMC_TOP_CFG_IP_DLY_IP_TAP_DELAY_MASK                   0x0000003f
#define EMMC_TOP_CFG_IP_DLY_IP_TAP_DELAY_SHIFT                  0
#define EMMC_TOP_CFG_IP_DLY_IP_TAP_DELAY_DEFAULT                0x00000028

   uint32 emmc_top_cfg_op_dly;             /* 0x34 SDIO Host output delay register                   */ 
/***************************************************************************
 *OP_DLY - SDIO Host output delay register
 ***************************************************************************/
/* SDIO_0_CFG :: OP_DLY :: OP_TAP_EN [31:31] */
#define EMMC_TOP_CFG_OP_DLY_OP_TAP_EN_MASK                      0x80000000
#define EMMC_TOP_CFG_OP_DLY_OP_TAP_EN_SHIFT                     31
#define EMMC_TOP_CFG_OP_DLY_OP_TAP_EN_DEFAULT                   0x00000000

/* SDIO_0_CFG :: OP_DLY :: reserved0 [30:18] */
#define EMMC_TOP_CFG_OP_DLY_reserved0_MASK                      0x7ffc0000
#define EMMC_TOP_CFG_OP_DLY_reserved0_SHIFT                     18

/* SDIO_0_CFG :: OP_DLY :: OP_DELAY_CTRL [17:16] */
#define EMMC_TOP_CFG_OP_DLY_OP_DELAY_CTRL_MASK                  0x00030000
#define EMMC_TOP_CFG_OP_DLY_OP_DELAY_CTRL_SHIFT                 16
#define EMMC_TOP_CFG_OP_DLY_OP_DELAY_CTRL_DEFAULT               0x00000000

/* SDIO_0_CFG :: OP_DLY :: reserved1 [15:04] */
#define EMMC_TOP_CFG_OP_DLY_reserved1_MASK                      0x0000fff0
#define EMMC_TOP_CFG_OP_DLY_reserved1_SHIFT                     4

/* SDIO_0_CFG :: OP_DLY :: OP_TAP_DELAY [03:00] */
#define EMMC_TOP_CFG_OP_DLY_OP_TAP_DELAY_MASK                   0x0000000f
#define EMMC_TOP_CFG_OP_DLY_OP_TAP_DELAY_SHIFT                  0
#define EMMC_TOP_CFG_OP_DLY_OP_TAP_DELAY_DEFAULT                0x00000000

   uint32 emmc_top_cfg_tuning;             /* 0x38 SDIO Host tuning configuration register           */ 
/***************************************************************************
 *TUNING - SDIO Host tuning configuration register
 ***************************************************************************/
/* SDIO_0_CFG :: TUNING :: reserved0 [31:04] */
#define EMMC_TOP_CFG_TUNING_reserved0_MASK                      0xfffffff0
#define EMMC_TOP_CFG_TUNING_reserved0_SHIFT                     4

/* SDIO_0_CFG :: TUNING :: TUNING_CMD_SUCCESS_CNT [03:00] */
#define EMMC_TOP_CFG_TUNING_TUNING_CMD_SUCCESS_CNT_MASK         0x0000000f
#define EMMC_TOP_CFG_TUNING_TUNING_CMD_SUCCESS_CNT_SHIFT        0
#define EMMC_TOP_CFG_TUNING_TUNING_CMD_SUCCESS_CNT_DEFAULT      0x00000008

   uint32 emmc_top_cfg_volt_ctrl;          /* 0x3c SDIO Host 1p8V control logic select register      */
/***************************************************************************
 *VOLT_CTRL - SDIO Host 1p8V control logic select register
 ***************************************************************************/
/* SDIO_0_CFG :: VOLT_CTRL :: reserved0 [31:05] */
#define EMMC_TOP_CFG_VOLT_CTRL_reserved0_MASK                   0xffffffe0
#define EMMC_TOP_CFG_VOLT_CTRL_reserved0_SHIFT                  5

/* SDIO_0_CFG :: VOLT_CTRL :: POW_INV_EN [04:04] */
#define EMMC_TOP_CFG_VOLT_CTRL_POW_INV_EN_MASK                  0x00000010
#define EMMC_TOP_CFG_VOLT_CTRL_POW_INV_EN_SHIFT                 4
#define EMMC_TOP_CFG_VOLT_CTRL_POW_INV_EN_DEFAULT               0x00000000

/* SDIO_0_CFG :: VOLT_CTRL :: 1P8V_VAL [03:03] */
#define EMMC_TOP_CFG_VOLT_CTRL_1P8V_VAL_MASK                    0x00000008
#define EMMC_TOP_CFG_VOLT_CTRL_1P8V_VAL_SHIFT                   3
#define EMMC_TOP_CFG_VOLT_CTRL_1P8V_VAL_DEFAULT                 0x00000000

/* SDIO_0_CFG :: VOLT_CTRL :: 1P8V_INV_EN [02:02] */
#define EMMC_TOP_CFG_VOLT_CTRL_1P8V_INV_EN_MASK                 0x00000004
#define EMMC_TOP_CFG_VOLT_CTRL_1P8V_INV_EN_SHIFT                2
#define EMMC_TOP_CFG_VOLT_CTRL_1P8V_INV_EN_DEFAULT              0x00000000

/* SDIO_0_CFG :: VOLT_CTRL :: 1P8V_CTRL_SEL [01:00] */
#define EMMC_TOP_CFG_VOLT_CTRL_1P8V_CTRL_SEL_MASK               0x00000003
#define EMMC_TOP_CFG_VOLT_CTRL_1P8V_CTRL_SEL_SHIFT              0
#define EMMC_TOP_CFG_VOLT_CTRL_1P8V_CTRL_SEL_DEFAULT            0x00000000

   uint32 emmc_top_cfg_debug_tap_dly;      /* 0x40 Debug TAP delay setting register                  */
   
   uint32 unused1[3];                      /* 0x44 - 0x50                                            */
   
   uint32 emmc_top_cfg_sd_pin_sel;         /* 0x54 SD Pin Select                                     */
   
   uint32 emmc_top_cfg_max_current;        /* 0x58 Max Current Override                              */
   
   uint32 unused2[37];                     /* 0x5c - 0xec                                            */
   
   uint32 emmc_top_cfg_version;            /* 0xf0 SDIO VERSION Register                             */
/***************************************************************************
 *VERSION - SDIO VERSION Register
 ***************************************************************************/
/* SDIO_0_CFG :: VERSION :: SD_VER [31:24] */
#define EMMC_TOP_CFG_VERSION_SD_VER_MASK                        0xff000000
#define EMMC_TOP_CFG_VERSION_SD_VER_SHIFT                       24
#define EMMC_TOP_CFG_VERSION_SD_VER_DEFAULT                     0x00000030

/* SDIO_0_CFG :: VERSION :: MMC_VER [23:16] */
#define EMMC_TOP_CFG_VERSION_MMC_VER_MASK                       0x00ff0000
#define EMMC_TOP_CFG_VERSION_MMC_VER_SHIFT                      16
#define EMMC_TOP_CFG_VERSION_MMC_VER_DEFAULT                    0x00000044

/* SDIO_0_CFG :: VERSION :: REV [15:08] */
#define EMMC_TOP_CFG_VERSION_REV_MASK                           0x0000ff00
#define EMMC_TOP_CFG_VERSION_REV_SHIFT                          8
#define EMMC_TOP_CFG_VERSION_REV_DEFAULT                        0x000000a9

/* SDIO_0_CFG :: VERSION :: A2S_VER [07:00] */
#define EMMC_TOP_CFG_VERSION_A2S_VER_MASK                       0x000000ff
#define EMMC_TOP_CFG_VERSION_A2S_VER_SHIFT                      0
#define EMMC_TOP_CFG_VERSION_A2S_VER_DEFAULT                    0x00000001
   uint32 unused3[2];                      /* 0xf4 - 0xf8                                            */
   
   uint32 emmc_top_cfg_scratch;            /* 0xfc SDIO Scratch Register                             */   
/***************************************************************************
 *SCRATCH - SDIO Scratch Register
 ***************************************************************************/
/* SDIO_0_CFG :: SCRATCH :: SCRATCH_BITS [31:00] */
#define EMMC_TOP_CFG_SCRATCH_SCRATCH_BITS_MASK                  0xffffffff
#define EMMC_TOP_CFG_SCRATCH_SCRATCH_BITS_SHIFT                 0
#define EMMC_TOP_CFG_SCRATCH_SCRATCH_BITS_DEFAULT               0x00000000
} EmmcTopCfgRegs;
#define EMMC_TOP_CFG ((volatile EmmcTopCfgRegs *const) EMMC_TOP_CFG_BASE)

typedef struct EmmcBootRegs {
   uint32 emmc_boot_main_ctl;         /* 0x00 Main control register */   
#define EMMC_BOOT_ENABLE   (1 << 0)   
/***************************************************************************
 *MAIN_CTL - Main control register
 ***************************************************************************/
/* SDIO_1_BOOT :: MAIN_CTL :: reserved0 [31:03] */
#define EMMC_BOOT_MAIN_CTL_reserved0_MASK                   0xfffffff8
#define EMMC_BOOT_MAIN_CTL_reserved0_SHIFT                  3

/* SDIO_1_BOOT :: MAIN_CTL :: DivSpeedUp [02:02] */
#define EMMC_BOOT_MAIN_CTL_DivSpeedUp_MASK                  0x00000004
#define EMMC_BOOT_MAIN_CTL_DivSpeedUp_SHIFT                 2
#define EMMC_BOOT_MAIN_CTL_DivSpeedUp_DEFAULT               0x00000000

/* SDIO_1_BOOT :: MAIN_CTL :: reserved1 [01:01] */
#define EMMC_BOOT_MAIN_CTL_reserved1_MASK                   0x00000002
#define EMMC_BOOT_MAIN_CTL_reserved1_SHIFT                  1

/* SDIO_1_BOOT :: MAIN_CTL :: BootEna [00:00] */
#define EMMC_BOOT_MAIN_CTL_BootEna_MASK                     0x00000001
#define EMMC_BOOT_MAIN_CTL_BootEna_SHIFT                    0
#define EMMC_BOOT_MAIN_CTL_BootEna_DEFAULT                  0x00000000

   uint32 emmc_boot_status;           /* 0x04 Status                */     
#define EMMC_BOOT_MODE_MASK (1 << 0)    
/***************************************************************************
 *STATUS - Status
 ***************************************************************************/
/* SDIO_1_BOOT :: STATUS :: reserved0 [31:11] */
#define EMMC_BOOT_STATUS_reserved0_MASK                     0xfffff800
#define EMMC_BOOT_STATUS_reserved0_SHIFT                    11

/* SDIO_1_BOOT :: STATUS :: Boot_Rbus_Error [10:10] */
#define EMMC_BOOT_STATUS_Boot_Rbus_Error_MASK               0x00000400
#define EMMC_BOOT_STATUS_Boot_Rbus_Error_SHIFT              10
#define EMMC_BOOT_STATUS_Boot_Rbus_Error_DEFAULT            0x00000000

/* SDIO_1_BOOT :: STATUS :: AHB_Slave_Error [09:09] */
#define EMMC_BOOT_STATUS_AHB_Slave_Error_MASK               0x00000200
#define EMMC_BOOT_STATUS_AHB_Slave_Error_SHIFT              9
#define EMMC_BOOT_STATUS_AHB_Slave_Error_DEFAULT            0x00000000

/* SDIO_1_BOOT :: STATUS :: AHB_Master_Error [08:08] */
#define EMMC_BOOT_STATUS_AHB_Master_Error_MASK              0x00000100
#define EMMC_BOOT_STATUS_AHB_Master_Error_SHIFT             8
#define EMMC_BOOT_STATUS_AHB_Master_Error_DEFAULT           0x00000000

/* SDIO_1_BOOT :: STATUS :: SDIO_Host_Error [07:07] */
#define EMMC_BOOT_STATUS_SDIO_Host_Error_MASK               0x00000080
#define EMMC_BOOT_STATUS_SDIO_Host_Error_SHIFT              7
#define EMMC_BOOT_STATUS_SDIO_Host_Error_DEFAULT            0x00000000

/* SDIO_1_BOOT :: STATUS :: BusWidth [06:05] */
#define EMMC_BOOT_STATUS_BusWidth_MASK                      0x00000060
#define EMMC_BOOT_STATUS_BusWidth_SHIFT                     5

/* SDIO_1_BOOT :: STATUS :: BigEndian [04:04] */
#define EMMC_BOOT_STATUS_BigEndian_MASK                     0x00000010
#define EMMC_BOOT_STATUS_BigEndian_SHIFT                    4

/* SDIO_1_BOOT :: STATUS :: FetchActive [03:03] */
#define EMMC_BOOT_STATUS_FetchActive_MASK                   0x00000008
#define EMMC_BOOT_STATUS_FetchActive_SHIFT                  3
#define EMMC_BOOT_STATUS_FetchActive_DEFAULT                0x00000000

/* SDIO_1_BOOT :: STATUS :: RamValid1 [02:02] */
#define EMMC_BOOT_STATUS_RamValid1_MASK                     0x00000004
#define EMMC_BOOT_STATUS_RamValid1_SHIFT                    2
#define EMMC_BOOT_STATUS_RamValid1_DEFAULT                  0x00000000

/* SDIO_1_BOOT :: STATUS :: RamValid0 [01:01] */
#define EMMC_BOOT_STATUS_RamValid0_MASK                     0x00000002
#define EMMC_BOOT_STATUS_RamValid0_SHIFT                    1
#define EMMC_BOOT_STATUS_RamValid0_DEFAULT                  0x00000000

/* SDIO_1_BOOT :: STATUS :: BootMode [00:00] */
#define EMMC_BOOT_STATUS_BootMode_MASK                      0x00000001
#define EMMC_BOOT_STATUS_BootMode_SHIFT                     0
    
   uint32 emmc_boot_version;          /* 0x08 Version               */      
/***************************************************************************
 *VERSION - Version
 ***************************************************************************/
/* SDIO_1_BOOT :: VERSION :: reserved0 [31:24] */
#define EMMC_BOOT_VERSION_reserved0_MASK                    0xff000000
#define EMMC_BOOT_VERSION_reserved0_SHIFT                   24

/* SDIO_1_BOOT :: VERSION :: MajorRev [23:16] */
#define EMMC_BOOT_VERSION_MajorRev_MASK                     0x00ff0000
#define EMMC_BOOT_VERSION_MajorRev_SHIFT                    16
#define EMMC_BOOT_VERSION_MajorRev_DEFAULT                  0x00000001

/* SDIO_1_BOOT :: VERSION :: MinorRev [15:08] */
#define EMMC_BOOT_VERSION_MinorRev_MASK                     0x0000ff00
#define EMMC_BOOT_VERSION_MinorRev_SHIFT                    8
#define EMMC_BOOT_VERSION_MinorRev_DEFAULT                  0x00000000

/* SDIO_1_BOOT :: VERSION :: reserved1 [07:04] */
#define EMMC_BOOT_VERSION_reserved1_MASK                    0x000000f0
#define EMMC_BOOT_VERSION_reserved1_SHIFT                   4

/* SDIO_1_BOOT :: VERSION :: MetalRev [03:00] */
#define EMMC_BOOT_VERSION_MetalRev_MASK                     0x0000000f
#define EMMC_BOOT_VERSION_MetalRev_SHIFT                    0
#define EMMC_BOOT_VERSION_MetalRev_DEFAULT                  0x00000000

   uint32 unused1[1];                 /* 0x0c                       */
   uint32 emmc_boot_clk_div;          /* 0x10 Clock Divide Override */      
/***************************************************************************
 *CLK_DIV - Clock Divide Override
 ***************************************************************************/
/* SDIO_1_BOOT :: CLK_DIV :: reserved0 [31:12] */
#define EMMC_BOOT_CLK_DIV_reserved0_MASK                    0xfffff000
#define EMMC_BOOT_CLK_DIV_reserved0_SHIFT                   12

/* SDIO_1_BOOT :: CLK_DIV :: CmdDiv [11:08] */
#define EMMC_BOOT_CLK_DIV_CmdDiv_MASK                       0x00000f00
#define EMMC_BOOT_CLK_DIV_CmdDiv_SHIFT                      8
#define EMMC_BOOT_CLK_DIV_CmdDiv_DEFAULT                    0x00000000

/* SDIO_1_BOOT :: CLK_DIV :: reserved1 [07:04] */
#define EMMC_BOOT_CLK_DIV_reserved1_MASK                    0x000000f0
#define EMMC_BOOT_CLK_DIV_reserved1_SHIFT                   4

/* SDIO_1_BOOT :: CLK_DIV :: DataDiv [03:00] */
#define EMMC_BOOT_CLK_DIV_DataDiv_MASK                      0x0000000f
#define EMMC_BOOT_CLK_DIV_DataDiv_SHIFT                     0
#define EMMC_BOOT_CLK_DIV_DataDiv_DEFAULT                   0x00000000

   uint32 emmc_boot_reset_cnt;        /* 0x14 Reset Count           */    
/***************************************************************************
 *RESET_CNT - Reset Count
 ***************************************************************************/
/* SDIO_1_BOOT :: RESET_CNT :: reserved0 [31:16] */
#define EMMC_BOOT_RESET_CNT_reserved0_MASK                  0xffff0000
#define EMMC_BOOT_RESET_CNT_reserved0_SHIFT                 16

/* SDIO_1_BOOT :: RESET_CNT :: ResetCnt [15:00] */
#define EMMC_BOOT_RESET_CNT_ResetCnt_MASK                   0x0000ffff
#define EMMC_BOOT_RESET_CNT_ResetCnt_SHIFT                  0
#define EMMC_BOOT_RESET_CNT_ResetCnt_DEFAULT                0x00000000

   uint32 emmc_boot_ram_fill;         /* 0x18 Ram Fill              */     
/***************************************************************************
 *RAM_FILL - Ram Fill
 ***************************************************************************/
/* SDIO_1_BOOT :: RAM_FILL :: reserved0 [31:22] */
#define EMMC_BOOT_RAM_FILL_reserved0_MASK                   0xffc00000
#define EMMC_BOOT_RAM_FILL_reserved0_SHIFT                  22

/* SDIO_1_BOOT :: RAM_FILL :: FillAddr [21:00] */
#define EMMC_BOOT_RAM_FILL_FillAddr_MASK                    0x003fffff
#define EMMC_BOOT_RAM_FILL_FillAddr_SHIFT                   0
#define EMMC_BOOT_RAM_FILL_FillAddr_DEFAULT                 0x00000000

   uint32 emmc_boot_error_addr;       /* 0x1c Error Address         */   
/***************************************************************************
 *ERROR_ADDR - Error Address
 ***************************************************************************/
/* SDIO_1_BOOT :: ERROR_ADDR :: ErrorAddr [31:00] */
#define EMMC_BOOT_ERROR_ADDR_ErrorAddr_MASK                 0xffffffff
#define EMMC_BOOT_ERROR_ADDR_ErrorAddr_SHIFT                0
#define EMMC_BOOT_ERROR_ADDR_ErrorAddr_DEFAULT              0x00000000

   uint32 emmc_boot_base_addr0;       /* 0x20 RAM Base address      */   
/***************************************************************************
 *BASE_ADDR0 - RAM Base address
 ***************************************************************************/
/* SDIO_1_BOOT :: BASE_ADDR0 :: reserved0 [31:22] */
#define EMMC_BOOT_BASE_ADDR0_reserved0_MASK                 0xffc00000
#define EMMC_BOOT_BASE_ADDR0_reserved0_SHIFT                22

/* SDIO_1_BOOT :: BASE_ADDR0 :: BaseAddr [21:00] */
#define EMMC_BOOT_BASE_ADDR0_BaseAddr_MASK                  0x003fffff
#define EMMC_BOOT_BASE_ADDR0_BaseAddr_SHIFT                 0
#define EMMC_BOOT_BASE_ADDR0_BaseAddr_DEFAULT               0x00000000

   uint32 emmc_boot_base_addr1;       /* 0x24 RAM Base address      */   
/***************************************************************************
 *BASE_ADDR1 - RAM Base address
 ***************************************************************************/
/* SDIO_1_BOOT :: BASE_ADDR1 :: reserved0 [31:22] */
#define EMMC_BOOT_BASE_ADDR1_reserved0_MASK                 0xffc00000
#define EMMC_BOOT_BASE_ADDR1_reserved0_SHIFT                22

/* SDIO_1_BOOT :: BASE_ADDR1 :: BaseAddr [21:00] */
#define EMMC_BOOT_BASE_ADDR1_BaseAddr_MASK                  0x003fffff
#define EMMC_BOOT_BASE_ADDR1_BaseAddr_SHIFT                 0
#define EMMC_BOOT_BASE_ADDR1_BaseAddr_DEFAULT               0x00000000

   uint32 emmc_boot_ram_fill_cnt;     /* 0x28 RAM Fill Cnt          */ 
/***************************************************************************
 *RAM_FILL_CNT - RAM Fill Cnt
 ***************************************************************************/
/* SDIO_1_BOOT :: RAM_FILL_CNT :: reserved0 [31:11] */
#define EMMC_BOOT_RAM_FILL_CNT_reserved0_MASK               0xfffff800
#define EMMC_BOOT_RAM_FILL_CNT_reserved0_SHIFT              11

/* SDIO_1_BOOT :: RAM_FILL_CNT :: RamFillCnt [10:00] */
#define EMMC_BOOT_RAM_FILL_CNT_RamFillCnt_MASK              0x000007ff
#define EMMC_BOOT_RAM_FILL_CNT_RamFillCnt_SHIFT             0
#define EMMC_BOOT_RAM_FILL_CNT_RamFillCnt_DEFAULT           0x00000000

   uint32 emmc_boot_data_access_time; /* 0x2c Time for Data Fetch   */
/***************************************************************************
 *DATA_ACCESS_TIME - Time for Data Fetch
 ***************************************************************************/
/* SDIO_1_BOOT :: DATA_ACCESS_TIME :: reserved0 [31:16] */
#define EMMC_BOOT_DATA_ACCESS_TIME_reserved0_MASK           0xffff0000
#define EMMC_BOOT_DATA_ACCESS_TIME_reserved0_SHIFT          16

/* SDIO_1_BOOT :: DATA_ACCESS_TIME :: DataAccessTime [15:00] */
#define EMMC_BOOT_DATA_ACCESS_TIME_DataAccessTime_MASK      0x0000ffff
#define EMMC_BOOT_DATA_ACCESS_TIME_DataAccessTime_SHIFT     0
#define EMMC_BOOT_DATA_ACCESS_TIME_DataAccessTime_DEFAULT   0x00000000

   uint32 unused2[3];                 /* 0x30-0x38                  */ 
   uint32 emmc_boot_debug;            /* 0x3c Debug                 */           
/***************************************************************************
 *DEBUG - Debug
 ***************************************************************************/
/* SDIO_1_BOOT :: DEBUG :: Debug [31:00] */
#define EMMC_BOOT_DEBUG_Debug_MASK                          0xffffffff
#define EMMC_BOOT_DEBUG_Debug_SHIFT                         0
} EmmcBootRegs;
#define EMMC_BOOT    ((volatile EmmcBootRegs *const) EMMC_BOOT_BASE) 
  
typedef struct AhbssCtrlRegs {
   uint32 ahbss_ctrl_cfg;    /* 0x00 AHB Subsystem Control Register */   
#define FORCE_EMMC_BOOT_STRAP    0x00000001   
} AhbssCtrlRegs;
#define AHBSS_CTRL   ((volatile AhbssCtrlRegs *const) AHBSS_CTRL_BASE)
  

/*
 * High Speed Uart Control
 */
typedef struct HsUartCtrlRegs {
   uint32   unused[5];    /* 0x400 - 0x410                                     */   
   uint32   ptu_hc;       /* 0x414 PTU HC Register                             */    
#define HS_UART_PTU_HC_DATA (1 << 1)          
   uint32   unused0;      /* 0x418                                             */
   uint32   uart_data;    /* 0x41c Data Register                               */           
   uint32   unused1[25];  /* 0x420 - 0x483                                     */
   uint32   uart_int_stat;/* 0x484 Interrupt Status Register                   */           
   uint32   unused2[8];   /* 0x488 - 0x4a7                                     */
   uint32   uart_int_en;  /* 0x4a8 Interrupt Enable Register                   */    
#define HS_UART_TXFIFOFULL    (1 << 0) /* tx full          */                                  
#define HS_UART_TXFIFOAEMPTY  (1 << 1) /* tx almost empty  */                                  
#define HS_UART_RXFIFOAFULL   (1 << 2) /* rx almost full   */                                  
#define HS_UART_RXFIFOEMPTY   (1 << 3) /* rx empty         */                                  
#define HS_UART_RXFIFORES     (1 << 4) /* rx residue       */                                  
#define HS_UART_RXPARITYERR   (1 << 5) /* rx parity error  */                                   
#define HS_UART_RXBRKDET      (1 << 6) /* rx uart break    */                                  
#define HS_UART_RXCTS         (1 << 7) /* uart cts         */                                  
#define HS_UART_INT_MASK (0xff)                         
#define HS_UART_INT_MASK_DISABLE (0x00000000)           
                                                                                                                                                  
   uint32   unused3[53];  /* 0x4ac - 0x57f                                     */                                                                 
   uint32   dhbr;         /* 0x580 Baud Rate Adjustment Register               */           
   uint32   dlbr;         /* 0x584 Baud Rate Integer Divide Value Register     */
#define HS_UART_DHBR_3000K    0x00000001 /* Target is 16 clocks/baud(bit), Based on 50Mhz Uart clk */
#define HS_UART_DLBR_3000K    0x000000ff /* Target is 16 clocks/baud(bit), Based on 50Mhz Uart clk */
#define HS_UART_DHBR_115200   0x00000011 /* Target is 16 clocks/baud(bit), Based on 50Mhz Uart clk */
#define HS_UART_DLBR_115200   0x000000e5 /* Target is 16 clocks/baud(bit), Based on 50Mhz Uart clk */
             
   uint32   ab0;          /* 0x588 ab0 Register                                */           
   uint32   unused4;      /* 0x588 - 0x58f                                     */
   uint32   FCR;          /* 0x590 FCR Register                                */           
#define HS_UART_FCR_AUTOLOAD_MODE (1 << 8)
#define HS_UART_FCR_AB_MODE       (1 << 7)
#define HS_UART_FCR_AUTO_RTS_OE   (1 << 6)
#define HS_UART_FCR_AUTO_TX_OE    (1 << 5)
#define HS_UART_FCR_AUTOBAUD      (1 << 4)
#define HS_UART_FCR_TX_PACKET     (1 << 3)
#define HS_UART_FCR_SLIP_RESYNC   (1 << 2)
   
   uint32   ab1;          /* 0x594 ab1 Register                                */           
   uint32   unused5;      /* 0x598 - 0x59f                                     */
   uint32   LCR;          /* 0x59c LCR Register                                */           
#define HS_UART_LCR_SLIP_TX_CRC       (1 << 11) 
#define HS_UART_LCR_SLIP_CRC_LSBFIRST (1 << 10) 
#define HS_UART_LCR_SLIP_CRC_INV      (1 << 9) 
#define HS_UART_LCR_SLIP_RX_CRC       (1 << 8) 
#define HS_UART_LCR_SLIP              (1 << 7) 
#define HS_UART_LCR_RTSOEN            (1 << 6) 
#define HS_UART_LCR_TXOEN             (1 << 5) 
#define HS_UART_LCR_LBC               (1 << 4) 
#define HS_UART_LCR_RXEN              (1 << 3) 
#define HS_UART_LCR_EPS               (1 << 2) 
#define HS_UART_LCR_PEN               (1 << 1) 
#define HS_UART_LCR_STB               (1 << 0)
    
   uint32   MCR;          /* 0x5a0 MCR Register                                */    
#define HS_UART_MCR_REPEAT_XON_XOFF (1 << 9)
#define HS_UART_MCR_PACKET_FLOW     (1 << 8)
#define HS_UART_MCR_BAUD_ADJ_EN     (1 << 7)
#define HS_UART_MCR_AUTO_TX_DISABLE (1 << 6)
#define HS_UART_MCR_AUTO_RTS        (1 << 5)
#define HS_UART_MCR_LOOPBACK        (1 << 4)
#define HS_UART_MCR_HIGH_RATE       (1 << 3)
#define HS_UART_MCR_XON_XOff_EN     (1 << 2)
#define HS_UART_MCR_PROG_RTS        (1 << 1)
#define HS_UART_MCR_TX_ENABLE       (1 << 0)
          
   uint32   LSR;          /* 0x5a4 LSR Register                                */   
#define HS_UART_LSR_TX_HALT        (1 << 5)    
#define HS_UART_LSR_TX_PACKET_RDY  (1 << 4)    
#define HS_UART_LSR_TX_IDLE        (1 << 3)    
#define HS_UART_LSR_TX_DATA_AVAIL  (1 << 2) /* 1 - Data in TX FIFO, 0 - TX FIFO empty */
#define HS_UART_LSR_RX_FULL        (1 << 1) /* 1 - RX FIFO full */     
#define HS_UART_LSR_RX_OVERFLOW    (1 << 0) /* 1 - RX FIFO Overflow */       
          
   uint32   MSR;          /* 0x5a8 MSR Register                                */         
#define HS_UART_MSR_RX_IN     (1 << 2)
#define HS_UART_MSR_RTS_STAT  (1 << 1)
#define HS_UART_MSR_CTS_STAT  (1 << 0)
                                                    
   uint32   RFL;          /* 0x5ac Receive FIFO Level Almost Full Register     */           
   uint32   TFL;          /* 0x5b0 Transmit FIFO Level Almost Empty Register   */           
   uint32   RFC;          /* 0x5b4 Receive FIFO Level Flow Control Register    */ 
#define HS_UART_RFC_1039BYTES_FC_DATA (1039)
#define HS_UART_RFC_NO_FC_DATA (HS_UART_RFC_1039BYTES_FC_DATA) // or should be LLI_BUFF_SIZE
            
   uint32   ESC;          /* 0x5b8 Escape Character Register                   */           
#define HS_UART_ESC_SLIP_DATA (0xDB)
#define HS_UART_ESC_NO_SLIP_DATA (0xDA)
   
   uint32   unused6[3];   /* 0x5bc - 05bf                                      */
   uint32   HOPKT_LEN;    /* 0x5c8 Host Output Packet Length Register          */           
   uint32   HIPKT_LEN;    /* 0x5cc Host Input Packet Length Register           */           
   uint32   HO_DMA_CTL;   /* 0x5d0 Host Output DMA Control Register            */           
   uint32   HI_DMA_CTL;   /* 0x5d4 Host Input DMA Control Register             */           
   uint32   HO_BSIZE;     /* 0x5d8 Host Output DMA Burst Size Select Register  */           
   uint32   HI_BSIZE;     /* 0x5dc Host Input DMA Burst Size Select Register   */   
#define HS_UART_DMA_CTL_BURSTMODE_EN   (1 << 3)
#define HS_UART_DMA_CTL_AFMODE_EN      (1 << 2)
#define HS_UART_DMA_CTL_FASTMODE_EN    (1 << 1)
#define HS_UART_DMA_CTL_DMA_EN         (1 << 0)
#define HS_UART_HO_BSIZE_DATA (0x3)
#define HS_UART_HI_BSIZE_DATA (0x3)
              
} HsUartCtrlRegs;

#define HS_UART ((volatile HsUartCtrlRegs * const) HS_UART_BASE)


/*
 * PL081 DMA controller ctrl
 */
typedef struct Pl081DmaCtrlRegs {
   uint32 dmacintstat;     /* 0xd000 interrupt status              */
   uint32 dmacinttcstat;   /* 0xd004 tc interrupt status           */
   uint32 dmacinttcclr;    /* 0xd008 tc interrupt clear            */
   uint32 dmacinterrstat;  /* 0xd00c err interrupt status          */
   uint32 dmacinterrclr;   /* 0xd010 err interrupt clear           */
   uint32 dmacrawintc;     /* 0xd014 Raw tc interrupt status       */
   uint32 dmacrawinterr;   /* 0xd018 Raw err interrupt status      */  
   uint32 dmacenbldchns;   /* 0xd01c channel Enables               */
   uint32 dmacsoftbreq;    /* 0xd020 Soft DMA burst Request        */
   uint32 dmacsoftsreq;    /* 0xd024 Soft DMA single Request       */
   uint32 dmacsoftlbreq;   /* 0xd028 Soft DMA L-burst Request      */
   uint32 dmacsoftlsreq;   /* 0xd02c Soft DMA L-single Request     */
   uint32 dmacconfig;      /* 0xd030 DMAC configuration            */
   uint32 dmacsync;        /* 0xd034 DMAC requests select control  */
   uint32 unused1[50];     /* 0xd038 - 0xd0ff                      */
   uint32 dmacc0srcaddr;   /* 0xd100 Source Address register0      */
   uint32 dmacc0destaddr;  /* 0xd104 Destination Address reg0      */
   uint32 dmacc0llireg;    /* 0xd108 Next Linked list Address0     */
   uint32 dmacc0control;   /* 0xd10c channel control register0     */
   uint32 dmacc0config;    /* 0xd110 channel config register0      */
   uint32 unused2[3];      /* 0xd114 - 0xd11f                      */
   uint32 dmacc1srcaddr;   /* 0xd120 Source Address register1      */
   uint32 dmacc1destaddr;  /* 0xd124 Destination Address reg1      */
   uint32 dmacc1llireg;    /* 0xd128 Next Linked list Address1     */
   uint32 dmacc1control;   /* 0xd12c channel control register1     */
   uint32 dmacc1config;    /* 0xd130 channel config register1      */
   uint32 unused3[51];     /* 0xd134 - 0xd1ff                      */
#define Pl081_DMACCxControl_TCINT_EN         (1 << 31)
#define Pl081_DMACCxControl_PROT_PRIVILEGED  (1 << 28)
#define Pl081_DMACCxControl_PROT_BUFF        (1 << 29)
#define Pl081_DMACCxControl_PROT_CACHE       (1 << 30)
#define Pl081_DMACCxControl_DI               (1 << 27)
#define Pl081_DMACCxControl_SI               (1 << 26)

#define Pl081_DMACCxControl_DWIDTH_B      0
#define Pl081_DMACCxControl_DWIDTH_H      1
#define Pl081_DMACCxControl_DWIDTH_W      2
#define Pl081_DMACCxControl_DWIDTH_SHIFT  21
                                          
#define Pl081_DMACCxControl_SWIDTH_B      0
#define Pl081_DMACCxControl_SWIDTH_H      1
#define Pl081_DMACCxControl_SWIDTH_W      2
#define Pl081_DMACCxControl_SWIDTH_SHIFT  18
                                          
#define Pl081_DMACCxControl_DBSIZE_1      0
#define Pl081_DMACCxControl_DBSIZE_4      1
#define Pl081_DMACCxControl_DBSIZE_8      2
#define Pl081_DMACCxControl_DBSIZE_16     3
#define Pl081_DMACCxControl_DBSIZE_32     4
#define Pl081_DMACCxControl_DBSIZE_64     5
#define Pl081_DMACCxControl_DBSIZE_128    6
#define Pl081_DMACCxControl_DBSIZE_256    7
#define Pl081_DMACCxControl_DBSIZE_SHIFT  15
                                          
#define Pl081_DMACCxControl_SBSIZE_1      0
#define Pl081_DMACCxControl_SBSIZE_4      1
#define Pl081_DMACCxControl_SBSIZE_8      2
#define Pl081_DMACCxControl_SBSIZE_16     3
#define Pl081_DMACCxControl_SBSIZE_32     4
#define Pl081_DMACCxControl_SBSIZE_64     5
#define Pl081_DMACCxControl_SBSIZE_128    6
#define Pl081_DMACCxControl_SBSIZE_SHIFT  12
  
   uint32 dmactcr;         /* 0xd200 Test control                  */ 
   uint32 dmacitop1;       /* 0xd204 Output Set/Read               */
   uint32 dmacitop2;       /* 0xd208 Output Set/Read               */
   uint32 dmacitop3;       /* 0xd20c Output Set/Read               */
   uint32 unused4[52];     /* 0xd210 - 0xd2df                     */
   uint32 dmacperiphid0;   /* 0xd2e0 Peripheral Identification     */
   uint32 dmacperiphid1;   /* 0xd2e4 Peripheral Identification     */
   uint32 dmacperiphid2;   /* 0xd2e8 Peripheral Identification     */
   uint32 dmacperiphid3;   /* 0xd2ec Peripheral Identification     */
   uint32 dmacpcellid0;    /* 0xd2f0 PrimeCell Identification      */
   uint32 dmacpcellid1;    /* 0xd2f4 PrimeCell Identification      */
   uint32 dmacpcellid2;    /* 0xd2f8 PrimeCell Identification      */
   uint32 dmacpcellid3;    /* 0xd2fc PrimeCell Identification      */    
} Pl081DmaCtrlRegs;

#define PL081_DMA_CHAN_HS_UART_TX   "DMA_TO_HS_UART_TX"
#define PL081_DMA_CHAN_HS_UART_RX   "DMA_FROM_HS_UART_RX"

#define PL081_DMA ((volatile Pl081DmaCtrlRegs * const) PL081_DMA_BASE)

/*
 * Power Management Control
 */
typedef struct PmcCtrlReg {
   /* 0x00 */
   uint32 l1Irq4keMask;
   uint32 l1Irq4keStatus;
   uint32 l1IrqMipsMask;
   uint32 l1IrqMipsStatus;
   /* 0x10 */
   uint32 l2IrqGpMask;
   uint32 l2IrqGpStatus;
   uint32 gpTmr0Ctl;
   uint32 gpTmr0Cnt;
   /* 0x20 */
   uint32 gpTmr1Ctl;
   uint32 gpTmr1Cnt;
   uint32 hostMboxIn;
   uint32 hostMboxOut;
   /* 0x30 */
#define PMC_CTRL_GP_FLASH_BOOT_STALL                  0x00000080
   uint32 gpOut;
   uint32 gpIn;
   uint32 gpInIrqMask;
   uint32 gpInIrqStatus;
   /* 0x40 */
   uint32 dmaCtrl;
   uint32 dmaStatus;
   uint32 dma0_3FifoStatus;
   uint32 unused0[3];   /* 0x4c-0x57 */
   /* 0x58 */
   uint32 l1IrqMips1Mask;
   uint32 diagControl;
   /* 0x60 */
   uint32 diagHigh;
   uint32 diagLow;
   uint32 badAddr;
   uint32 addr1WndwMask;
   /* 0x70 */
   uint32 addr1WndwBaseIn;
   uint32 addr1WndwBaseOut;
   uint32 addr2WndwMask;
   uint32 addr2WndwBaseIn;
   /* 0x80 */
   uint32 addr2WndwBaseOut;
   uint32 scratch;
   uint32 tm;
   uint32 softResets;
   /* 0x90 */
   uint32 eb2ubusTimeout;
   uint32 m4keCoreStatus;
   uint32 gpInIrqSense;
   uint32 ubSlaveTimeout;
   /* 0xa0 */
   uint32 diagEn;
   uint32 devTimeout;
   uint32 ubusErrorOutMask;
   uint32 diagCaptStopMask;
   /* 0xb0 */
   uint32 revId;
   uint32 gpTmr2Ctl;
   uint32 gpTmr2Cnt;
   uint32 legacyMode;
   /* 0xc0 */
   uint32 smisbMonitor;
   uint32 diagCtrl;
   uint32 diagStat;
   uint32 diagMask;
   /* 0xd0 */
   uint32 diagRslt;
   uint32 diagCmp;
   uint32 diagCapt;
   uint32 diagCnt;
   /* 0xe0 */
   uint32 diagEdgeCnt;
   uint32 unused1[4];   /* 0xe4-0xf3 */
   /* 0xf4 */
   uint32 iopPeriphBaseAddr;
   uint32 lfsr;
   uint32 unused2;      /* 0xfc-0xff */
} PmcCtrlReg;

typedef struct PmcOutFifoReg {
   uint32 msgCtrl;      /* 0x00 */
   uint32 msgSts;    /* 0x04 */
   uint32 unused[14];   /* 0x08-0x3f */
   uint32 msgData[16];  /* 0x40-0x7c */
} PmcOutFifoReg;

typedef struct PmcInFifoReg {
   uint32 msgCtrl;      /* 0x00 */
   uint32 msgSts;    /* 0x04 */
   uint32 unused[13];   /* 0x08-0x3b */
   uint32 msgLast;      /* 0x3c */
   uint32 msgData[16];  /* 0x40-0x7c */
} PmcInFifoReg;

typedef struct PmcDmaReg {
   /* 0x00 */
   uint32 src;
   uint32 dest;
   uint32 cmdList;
   uint32 lenCtl;
   /* 0x10 */
   uint32 rsltSrc;
   uint32 rsltDest;
   uint32 rsltHcs;
   uint32 rsltLenStat;
} PmcDmaReg;

typedef struct PmcTokenReg {
   /* 0x00 */
   uint32 bufSize;
   uint32 bufBase;
   uint32 idx2ptrIdx;
   uint32 idx2ptrPtr;
   /* 0x10 */
   uint32 unused[2];
   uint32 bufSize2;
} PmcTokenReg;

typedef struct PmcPerfPowReg {
   /* 0x00 */
   uint32 dcacheHit;
   uint32 dcacheMiss;
   uint32 icacheHit;
   uint32 icacheMiss;
   /* 0x10 */
   uint32 instnComplete;
   uint32 wtbMerge;
   uint32 wtbNoMerge;
   uint32 itlbHit;
   /* 0x20 */
   uint32 itlbMiss;
   uint32 dtlbHit;
   uint32 dtlbMiss;
   uint32 jtlbHit;
   /* 0x30 */
   uint32 jtlbMiss;
   uint32 powerSubZone;
   uint32 powerMemPda;
   uint32 freqScalarCtrl;
   /* 0x40 */
   uint32 freqScalarMask;
} PmcPerfPowReg;

typedef struct PmcDQMReg {
   /* 0x00 */
   uint32 cfg;
   uint32 _4keLowWtmkIrqMask;
   uint32 mipsLowWtmkIrqMask;
   uint32 lowWtmkIrqMask;
   /* 0x10 */
   uint32 _4keNotEmptyIrqMask;
   uint32 mipsNotEmptyIrqMask;
   uint32 notEmptyIrqSts;
   uint32 queueRst;
   /* 0x20 */
   uint32 notEmptySts;
   uint32 nextAvailMask;
   uint32 nextAvailQueue;
   uint32 mips1LowWtmkIrqMask;
   /* 0x30 */
   uint32 mips1NotEmptyIrqMask;
   uint32 autoSrcPidInsert;
} PmcDQMReg;

typedef struct PmcCntReg {
   uint32 cntr[10];
   uint32 unused[6]; /* 0x28-0x3f */
   uint32 cntrIrqMask;
   uint32 cntrIrqSts;
} PmcCntReg;

typedef struct PmcDqmQCtrlReg {
   uint32 size;
   uint32 cfga;
   uint32 cfgb;
   uint32 cfgc;
} PmcDqmQCtrlReg;

typedef struct PmcDqmQDataReg {
   uint32 word[4];
} PmcDqmQDataReg;

typedef struct PmcDqmQMibReg {
   uint32 qNumFull[32];
   uint32 qNumEmpty[32];
   uint32 qNumPushed[32];
} PmcDqmQMibReg;

typedef struct Pmc {
   uint32 baseReserved;    /* 0x0000 */
   uint32 unused0[1023];
   PmcCtrlReg ctrl;     /* 0x1000 */

   PmcOutFifoReg outFifo;     /* 0x1100 */
   uint32 unused1[32];     /* 0x1180-0x11ff */
   PmcInFifoReg inFifo;    /* 0x1200 */
   uint32 unused2[32];     /* 0x1280-0x12ff */

   PmcDmaReg dma[2];    /* 0x1300 */
   uint32 unused3[48];     /* 0x1340-0x13ff */

   PmcTokenReg token;      /* 0x1400 */
   uint32 unused4[121];    /* 0x141c-0x15ff */

   PmcPerfPowReg perfPower;   /* 0x1600 */
   uint32 unused5[47];     /* 0x1644-0x16ff */

   uint32 msgId[32];    /* 0x1700 */
   uint32 unused6[32];     /* 0x1780-0x17ff */

   PmcDQMReg dqm;       /* 0x1800 */
   uint32 unused7[50];     /* 0x1838-0x18ff */

   PmcCntReg hwCounter;    /* 0x1900 */
   uint32 unused8[46];     /* 0x1948-0x19ff */

   PmcDqmQCtrlReg dqmQCtrl[32];  /* 0x1a00 */
   PmcDqmQDataReg dqmQData[32];  /* 0x1c00 */
   uint32 unused9[64];     /* 0x1e00-0x1eff */

   uint32 qStatus[32];     /* 0x1f00 */
   uint32 unused10[32];    /* 0x1f80-0x1fff */

   PmcDqmQMibReg qMib;     /* 0x2000 */
   uint32 unused11[1952];     /* 0x2180-0x3ffff */

   uint32 sharedMem[8192];    /* 0x4000-0xbffc */
} Pmc;

#define PMC ((volatile Pmc * const) PMC_BASE)

/*
 * Process Monitor Module
 */
typedef struct PMRingOscillatorControl {
   uint32 control;
   uint32 en_lo;
   uint32 en_mid;
   uint32 en_hi;
   uint32 idle_lo;
   uint32 idle_mid;
   uint32 idle_hi;
} PMRingOscillatorControl;

#define RCAL_0P25UM_HORZ          0
#define RCAL_0P25UM_VERT          1
#define RCAL_0P5UM_HORZ           2
#define RCAL_0P5UM_VERT           3
#define RCAL_1UM_HORZ             4
#define RCAL_1UM_VERT             5
#define PMMISC_RMON_EXT_REG       ((RCAL_1UM_VERT + 1)/2)
#define PMMISC_RMON_VALID_MASK    (0x1<<16)
typedef struct PMMiscControl {
   uint32 gp_out;
   uint32 clock_select;
   uint32 unused[2];
   uint32 misc[4];
} PMMiscControl;

typedef struct PMSSBMasterControl {
   uint32 control;
#define PMC_SSBM_CONTROL_SSB_START  (1<<15)
#define PMC_SSBM_CONTROL_SSB_ADPRE  (1<<13)
#define PMC_SSBM_CONTROL_SSB_EN     (1<<12)
#define PMC_SSBM_CONTROL_SSB_CMD_SHIFT (10)
#define PMC_SSBM_CONTROL_SSB_CMD_MASK  (0x3 << PMC_SSBM_CONTROL_SSB_CMD_SHIFT)
#define PMC_SSBM_CONTROL_SSB_CMD_READ  (2)
#define PMC_SSBM_CONTROL_SSB_CMD_WRITE (1)
#define PMC_SSBM_CONTROL_SSB_ADDR_SHIFT   (0)
#define PMC_SSBM_CONTROL_SSB_ADDR_MASK (0x3ff << PMC_SSBM_CONTROL_SSB_ADDR_SHIFT)
   uint32 wr_data;
   uint32 rd_data;
} PMSSBMasterControl;

typedef struct PMEctrControl {
   uint32 control;
   uint32 interval;
   uint32 thresh_lo;
   uint32 thresh_hi;
   uint32 count;
} PMEctrControl;

typedef struct PMBMaster {
   uint32 ctrl;
#define PMC_PMBM_START     (1 << 31)
#define PMC_PMBM_TIMEOUT   (1 << 30)
#define PMC_PMBM_SLAVE_ERR (1 << 29)
#define PMC_PMBM_BUSY      (1 << 28)
#define PMC_PMBM_Read      (0 << 20)
#define PMC_PMBM_Write     (1 << 20)
   uint32 wr_data;
   uint32 timeout;
   uint32 rd_data;
   uint32 unused[4];
} PMBMaster;

typedef struct PMAPVTMONControl {
   uint32 control;
   uint32 reserved;
   uint32 cfg_lo;
   uint32 cfg_hi;
   uint32 data;
   uint32 vref_data;
   uint32 unused[2];
   uint32 ascan_cfg;
   uint32 warn_temp;
   uint32 reset_temp;
   uint32 temp_value;
   uint32 data1_value;
   uint32 data2_value;
   uint32 data3_value;
} PMAPVTMONControl;

typedef struct PMUBUSCfg {
   uint32 window[8];
   uint32 control;
} PMUBUSCfg;

typedef struct ProcessMonitorRegs {
   uint32 MonitorCtrl;     /* 0x00 */
   uint32 unused0[7];
   PMRingOscillatorControl ROSC; /* 0x20 */
   uint32 unused1;
   PMMiscControl Misc;     /* 0x40 */
   PMSSBMasterControl SSBMaster; /* 0x60 */
   uint32 unused2[5];
   PMEctrControl Ectr;     /* 0x80 */
   uint32 unused3[11];
   PMBMaster PMBM[2];      /* 0xc0 */
   PMAPVTMONControl APvtmonCtrl; /* 0x100 */
   uint32 unused4[9];
   PMUBUSCfg UBUSCfg;      /* 0x160 */
} ProcessMonitorRegs;

#define PROCMON ((volatile ProcessMonitorRegs * const) PROC_MON_BASE)

#if 0
/*
** Spi Controller
*/

typedef struct SpiControl {
  uint16        spiMsgCtl;              /* (0x0) control byte */
#define FULL_DUPLEX_RW                  0
#define HALF_DUPLEX_W                   1
#define HALF_DUPLEX_R                   2
#define SPI_MSG_TYPE_SHIFT              14
#define SPI_BYTE_CNT_SHIFT              0
  byte          spiMsgData[0x21e];      /* (0x02 - 0x21f) msg data */
  byte          unused0[0x1e0];
  byte          spiRxDataFifo[0x220];   /* (0x400 - 0x61f) rx data */
  byte          unused1[0xe0];

  uint16        spiCmd;                 /* (0x700): SPI command */
#define SPI_CMD_NOOP                    0
#define SPI_CMD_SOFT_RESET              1
#define SPI_CMD_HARD_RESET              2
#define SPI_CMD_START_IMMEDIATE         3

#define SPI_CMD_COMMAND_SHIFT           0
#define SPI_CMD_COMMAND_MASK            0x000f

#define SPI_CMD_DEVICE_ID_SHIFT         4
#define SPI_CMD_PREPEND_BYTE_CNT_SHIFT  8
#define SPI_CMD_ONE_BYTE_SHIFT          11
#define SPI_CMD_ONE_WIRE_SHIFT          12
#define SPI_DEV_ID_0                    0
#define SPI_DEV_ID_1                    1
#define SPI_DEV_ID_2                    2
#define SPI_DEV_ID_3                    3
#define ZSI_SPI_DEV_ID                  3

  byte          spiIntStatus;           /* (0x702): SPI interrupt status */
  byte          spiMaskIntStatus;       /* (0x703): SPI masked interrupt status */

  byte          spiIntMask;             /* (0x704): SPI interrupt mask */
#define SPI_INTR_CMD_DONE               0x01
#define SPI_INTR_RX_OVERFLOW            0x02
#define SPI_INTR_INTR_TX_UNDERFLOW      0x04
#define SPI_INTR_TX_OVERFLOW            0x08
#define SPI_INTR_RX_UNDERFLOW           0x10
#define SPI_INTR_CLEAR_ALL              0x1f

  byte          spiStatus;              /* (0x705): SPI status */
#define SPI_RX_EMPTY                    0x02
#define SPI_CMD_BUSY                    0x04
#define SPI_SERIAL_BUSY                 0x08

  byte          spiClkCfg;              /* (0x706): SPI clock configuration */
#define SPI_CLK_0_391MHZ                1
#define SPI_CLK_0_781MHZ                2 /* default */
#define SPI_CLK_1_563MHZ                3
#define SPI_CLK_3_125MHZ                4
#define SPI_CLK_6_250MHZ                5
#define SPI_CLK_12_50MHZ                6
#define SPI_CLK_MASK                    0x07
#define SPI_SSOFFTIME_MASK              0x38
#define SPI_SSOFFTIME_SHIFT             3
#define SPI_BYTE_SWAP                   0x80

  byte          spiFillByte;            /* (0x707): SPI fill byte */
  byte          unused2;
  byte          spiMsgTail;             /* (0x709): msgtail */
  byte          unused3;
  byte          spiRxTail;              /* (0x70B): rxtail */
} SpiControl;

#define SPI ((volatile SpiControl * const) SPI_BASE)
#endif


#define SWITCH_CNTRL_REG    BCM_IO_ADDR(0x800c0000)

#define SWITCH_SINGLE_SERDES_STAT (SWITCH_CNTRL_REG + 0x198)
    #define SWITCH_REG_SSER_LINK_STAT   (1<<0)
    #define SWITCH_REG_SSER_RXSIG_DET   (1<<1)
    #define SWITCH_REG_SSER_RXSIG_1G    (1<<2)
    #define SWITCH_REG_SSER_SGMII       (1<<3)
    #define SWITCH_REG_SSER_SYNC_STAT   (1<<4)
    #define SWITCH_REG_SSER_POLL_LOCK   (1<<5)
    #define SWITCH_REG_SSER_EXTFB_DET   (1<<6)

#define SWITCH_REG_SINGLE_SERDES_CNTRL (SWITCH_CNTRL_REG + 0x194)

#define SWITCH_REG_SERDES_IDDQ       (1<<0)
#define SWITCH_REG_SERDES_PWRDWN     (1<<1)
#define SWITCH_REG_SERDES_RESETPLL   (1<<3)
#define SWITCH_REG_SERDES_RESETMDIO  (1<<4)
#define SWITCH_REG_SERDES_RESET      (1<<5)

#define SWITCH_REG_LED_WAN_CNTRL_LED (SWITCH_CNTRL_REG + 0x19c)
#define SWITCH_REG_LED_WAN_TX_EN      (1<<17)
#define SWITCH_REG_LED_WAN_RX_EN      (1<<16)

#if 0
typedef struct USBControl {
    uint32 BrtControl1;
    uint32 BrtControl2;
    uint32 BrtStatus1;
    uint32 BrtStatus2;
    uint32 UTMIControl1;
    uint32 TestPortControl;
    uint32 PllControl1;
#define PLLC_REFCLKSEL_MASK     0x00000003
#define PLLC_REFCLKSEL_SHIFT    0
#define PLLC_CLKSEL_MASK        0x0000000c
#define PLLC_CLKSEL_SHIFT       2
#define PLLC_XTAL_PWRDWNB       0x00000010
#define PLLC_PLL_PWRDWNB        0x00000020
#define PLLC_PLL_CALEN          0x00000040
#define PLLC_PHYPLL_BYP         0x00000080
#define PLLC_PLL_RESET          0x00000100
#define PLLC_PLL_IDDQ_PWRDN     0x00000200
#define PLLC_PLL_PWRDN_DELAY    0x00000400
    uint32 SwapControl;
#define USB_DEVICE_SEL          (1<<6)
#define EHCI_LOGICAL_ADDRESS_EN (1<<5)
#define EHCI_ENDIAN_SWAP        (1<<4)
#define EHCI_DATA_SWAP          (1<<3)
#define OHCI_LOGICAL_ADDRESS_EN (1<<2)
#define OHCI_ENDIAN_SWAP        (1<<1)
#define OHCI_DATA_SWAP          (1<<0)
    uint32 GenericControl;
#define GC_PLL_SUSPEND_EN       (1<<1)
    uint32 FrameAdjustValue;
    uint32 Setup;
#define USBH_IPP                (1<<5)
#define USBH_IOC                (1<<4)
    uint32 MDIO;
    uint32 MDIO32;
    uint32 USBSimControl;
} USBControl;

#define USBH ((volatile USBControl * const) USBH_CFG_BASE)
#endif

#if 0
typedef struct EthSwAvTableRegs {
    uint8  arlTableControl;
#define ARL_CTRL_START_DONE 0x80
#define ARL_CTRL_READ       0x01
#define ARL_CTRL_WRITE      0x00
    uint16 macAddrIndexLow;
    uint32 macAddrIndexHigh;
    uint16 vlanIdIndex;
#define ARL_VID_INDEX_MASK 0x0FFF
    uint32 reserved0;
    uint32 arlMacVidEntryLow;
    uint32 arlMacVidEntryHigh;
#define ARL_MAC_VID_ENTRY_HIGH_VID_MASK 0x0FFF0000
#define ARL_MAC_VID_ENTRY_HIGH_VID_SHFT 16
#define ARL_MAC_VID_ENTRY_HIGH_MAC_MASK 0x0000FFFF
    uint16 arlDataEntry;
#define ARL_DATA_ENTRY_VALID     0x8000
#define ARL_DATA_ENTRY_STATIC    0x4000
#define ARL_DATA_ENTRY_AGE       0x2000
#define ARL_DATA_ENTRY_PRIO_MASK 0x1C00
#define ARL_DATA_ENTRY_PRIO_SHFT 10
#define ARL_DATA_ENTRY_PORT_MASK 0x01FF
    uint32 reserved1[5];
    uint16 arlSearchControl;
#define ARL_SEARCH_CTRL_START_DONE 0x0080
#define ARL_SEARCH_CTRL_VALID      0x0001
    uint16 arlSearchAddr;
    uint32 arlSearchMacVidResultLow;
    uint32 arlSearchMacVidResultHigh;
#define ARL_SEARCH_MAC_VID_RESULT_VID_MASK 0x0FFF0000
#define ARL_SEARCH_MAC_VID_RESULT_VID_SHFT 16
#define ARL_SEARCH_MAC_VID_RESULT_MAC_MASK 0x0000FFFF
    uint16 arlSearchDataResult;
#define ARL_SEARCH_DATA_RESULT_STATIC    0x8000
#define ARL_SEARCH_DATA_RESULT_AGE       0x4000
#define ARL_SEARCH_DATA_RESULT_PRIO_MASK 0x3800
#define ARL_SEARCH_DATA_RESULT_PRIO_SHFT 11
#define ARL_SEARCH_DATA_RESULT_PORT_MASK 0x03FE
#define ARL_SEARCH_DATA_RESULT_PORT_SHFT 1
    uint32 reserved2[8];
    uint8  vlanTableControl;
    uint16 vlanTableAddrIndex;
    uint32 vlanTableEntry;
} EthSwAvTableRegs;

#define ETHSW_AVTABLE_REG ((volatile EthSwAvTableRegs * const) (SWITCH_BASE + 0x500))

typedef struct EthSwMIBRegs {
    unsigned int TxOctetsLo;               // 0
    unsigned int TxOctetsHi;
    unsigned int TxDropPkts;               // 8
    unsigned int TxQoSPkts;                // Tx Q0 pkts
    unsigned int TxBroadcastPkts;          // 0x10
    unsigned int TxMulticastPkts;
    unsigned int TxUnicastPkts;
    unsigned int TxCol;
    unsigned int TxSingleCol;              // 0x20
    unsigned int TxMultipleCol;
    unsigned int TxDeferredTx;
    unsigned int TxLateCol;
    unsigned int TxExcessiveCol;           // 0x30
    unsigned int TxFrameInDisc;
    unsigned int TxPausePkts;
    // SF2 Enhancements
    unsigned int TxQ1Pkts;                 // Tx Q1 pkts
    unsigned int TxQ2Pkts;                 // 0x40 Tx Q2 pkts
    unsigned int TxQ3Pkts;                 // Tx Q3 pkts
    unsigned int TxQ4Pkts;                 // Tx Q4 pkts
    unsigned int TxQ5Pkts;                 // Tx Q5 pkts
    //unsigned int TxQoSOctetsLo;         
    //unsigned int TxQoSOctetsHi;
    // SF2 Done
    unsigned int RxOctetsLo;               // 0x50
    unsigned int RxOctetsHi;
    unsigned int RxUndersizePkts;
    unsigned int RxPausePkts;
    unsigned int Pkts64Octets;             // 0x60
    unsigned int Pkts65to127Octets;
    unsigned int Pkts128to255Octets;
    unsigned int Pkts256to511Octets;
    unsigned int Pkts512to1023Octets;      // 0x70
    unsigned int Pkts1024to1522Octets;
    unsigned int RxOversizePkts;
    unsigned int RxJabbers;
    unsigned int RxAlignErrs;              // 0x80
    unsigned int RxFCSErrs;
    unsigned int RxGoodOctetsLo;
    unsigned int RxGoodOctetsHi;
    unsigned int RxDropPkts;               // 0x90
    unsigned int RxUnicastPkts;
    unsigned int RxMulticastPkts;
    unsigned int RxBroadcastPkts;
    unsigned int RxSAChanges;              // 0xa0
    unsigned int RxFragments;
    unsigned int RxExcessSizeDisc;
    unsigned int RxSymbolError;
    unsigned int InRangeErr;               // 0xb0
    unsigned int outRangeErr;
    unsigned int EEELpiEvevt;
    unsigned int EEELpiDuration;
    unsigned int RxDiscard;                // 0xc0
    unsigned int reserved1;
    unsigned int TxQ6Pkts;                 // Tx Q6 pkts
    unsigned int TxQ7Pkts;                 // Tx Q7 pkts
    unsigned int TxPkts64Octets;           // 0xD0
    unsigned int TxPkts65to127Octets;      // 
    unsigned int TxPkts128to255Octets;     // 
    unsigned int TxPkts256to511Octets;     // 
    unsigned int TxPkts512to1023Octets;    // 0xE0
    unsigned int TxPktsPkts1024toMaxOctets; 
    
#if 0
    unsigned int RxQoSPkts;
    unsigned int RxQoSOctetsLo;
    unsigned int RxQoSOctetsHi;
    unsigned int Pkts1523to2047;
    unsigned int Pkts2048to4095;
    unsigned int Pkts4096to8191;
    unsigned int Pkts8192to9728;
#endif
} EthSwMIBRegs;

#define ETHSWMIBREG ((volatile EthSwMIBRegs * const) (SWITCH_BASE + 0x2000))

/* Enet registers controlling rx iuDMA channel */
typedef struct EthSwQosIngressPortPriRegs{
    unsigned short pri_id_map[9];
} EthSwQosIngressPortPriRegs;

#define ETHSWQOSREG ((volatile EthSwQosIngressPortPriRegs * const) (SWITCH_BASE + 0x3050))
#endif

#define UNIMAC_BASE         BCM_IO_ADDR(0x802d4000)
#define UNIMAC_CFG_BASE     UNIMAC_BASE + 0x00000000
#define UNIMAC_MIB_BASE     UNIMAC_BASE + 0x00006000
#define UNIMAC_TOP_BASE     UNIMAC_BASE + 0x00007800

/*
** SAR Registers
*/

#define SAR_TX_CTL_REGS (SAR_BASE + 0x00000060) /* SAR Tx Control Registers */
#define SAR_TX_CTL_REGS_SZ  0x00000020
#define SAR_RX_CTL_REGS (SAR_BASE + 0x00000080) /* SAR Rx Control Registers */
#define SAR_RX_CTL_REGS_SZ  0x00000030
#define SAR_RX_VCAM_REGS (SAR_BASE + 0x00000140) /* SAR  Rx ATM VPI_VCI CAM Table Reg Registers */
#define SAR_RX_VCAM_REGS_SZ  0x00000080
#define SAR_SHPR_REGS (SAR_BASE + 0x00000200) /* SAR Atm Shaper Source Shaping Table Registers */
#define SAR_SHPR_REGS_SZ  0x00000070
#define SAR_RX_PBUF_REGS (SAR_BASE + 0x00000300) /* SAR Rx Packet Buffer Control Registers */
#define SAR_RX_PBUF_REGS_SZ  0x00000060
#define SAR_MIB_REGS (SAR_BASE + 0x00000600) /* SAR  Atm MIB Counters Registers */
#define SAR_MIB_REGS_SZ  0x000000C0
#define SAR_RX_PAF_REGS (SAR_BASE + 0x00000800) /* SAR RxPaf Top Registers */
#define SAR_RX_PAF_REGS_SZ  0x00000100
#define SAR_RX_BOND_REGS (SAR_BASE + 0x00000900) /* SAR RxPaf Bonding Registers */
#define SAR_RX_BOND_REGS_SZ  0x000000C0
#define SAR_TMUEXT_REGS (SAR_BASE + 0x00001000) /* SAR Traffic Management Unit Extended Registers */
#define SAR_TMUEXT_REGS_SZ  0x00000600


/* Specific SAR Rx Control Registers - LED Configuration Register */
#define SAR_RX_CTL_LED_CFG (SAR_BASE + 0x000000AC) /* LED Configuration Register */
#define SARRXCTLLEDCFG ((volatile uint32 * const) (SAR_RX_CTL_LED_CFG))

#define SARLEDCFG_TEST                  0x000000100     /* LED Test bit. */
#define SARLEDCFG_BLINK_30MS            0x000000000     /* LED blink speed: 00 = 30 ms */
#define SARLEDCFG_BLINK_50MS            0x000000020     /* LED blink speed: 01 = 50 ms */
#define SARLEDCFG_BLINK_125MS           0x000000040     /* LED blink speed: 10 = 125 ms */
#define SARLEDCFG_BLINK_250MS           0x000000060     /* LED blink speed: 11 = 250 ms */
#define SARLEDCFG_LNK                   0x000000010     /* Link is established - set by software when ADSL link is established. */
#define SARLEDCFG_INT_LED               0x000000008     /* Set to enable using internal LED logic to drive INET_LED, otherwise use Periph LED logic control. */
#define SARLEDCFG_MODE_LINK             0x000000000     /* INET_LED Mode: 00 = Assert on ADSL Link Only (by setting LINK bit) */
#define SARLEDCFG_MODE_TRAFFIC          0x000000002     /* INET_LED Mode: 01 = Assert on Cell Activity */
#define SARLEDCFG_MODE_MELODYLINK       0x000000004     /* INET_LED Mode: 10 = ADSL Melody/Link Mode - blink slowly during training and solid on link */
#define SARLEDCFG_MODE_TRAFFICLINK      0x000000006     /* INET_LED Mode: 11 = Assert on ADSL Link, blink on ATM TX and RX cells traffic */
#define SARLEDCFG_LED_EN                0x000000001     /* ADSL LED Enable */


#if 0
#if defined(CONFIG_BCM_GMAC)
#define GMAC_PORT_ID            3
#define GMAC_PHY_CHAN           0

#define MASK32(rightbitindex, length) ( (0xffffffff << rightbitindex) & (0xffffffff >> (32-(rightbitindex+length))) )


typedef union MibCtrlReg_s {
    uint32 word;
    struct {
        uint32    unused: 31;
        uint32    clrMib:  1;
    };
} MibCtrlReg_t;

typedef union MibMaxPktSizeReg_s {
    uint32 word;
    struct {
        uint32    unused      : 18;
        uint32    max_pkt_size: 14;
    };
} MibMaxPktSizeReg_t;

typedef union RxBpThreshReg_s {
    uint32 word;
    struct {
        uint32    unused    : 21;
        uint32    rx_thresh : 11;
    };
} RxBpThreshReg_t;

typedef union RxFlowCtrlReg_s {
    uint32 word;
    struct {
        uint32    unused    : 20;
        uint32    pause_en  :  1;
        uint32    fc_en     :  1;
    };
} RxFlowCtrlReg_t;

typedef union BpForceReg_s {
    uint32 word;
    struct {
        uint32    unused: 31;
        uint32    force :  1;
    };
} BpForceReg_t;

typedef union IrqEnableReg_s {
    uint32 word;
    struct {
        uint32    unused: 31;
        uint32    ovfl  :  1;
    };
} IrqEnableReg_t;

typedef union IrqStatusReg_s {
    uint32 word;
    struct {
        uint32    unused: 31;
        uint32    ovfl  :  1;
    };
} IrqStatusReg_t;

typedef union GmacStatusReg_s {
    uint32  word;
    struct {
        uint32  unused      :27;
        uint32  link_up     : 1;
        uint32  auto_cfg_en : 1;
        uint32  hd          : 1;
        uint32  eth_speed   : 2;
#define GMAC_STATUS_SPEED_10        0
#define GMAC_STATUS_SPEED_100       1
#define GMAC_STATUS_SPEED_1000      2
    };
} GmacStatusReg_t;

typedef union MacSwResetReg_s {
    uint32  word;
    struct {
        uint32  unused       :29;
        uint32  txfifo_flush : 1;
        uint32  rxfifo_flush : 1;
        uint32  mac_sw_reset : 1;
    };
} MacSwResetReg_t;

typedef union DmaRxStatusSelReg_s {
    uint32  word;
    struct {
        uint32  unused    :23;
        uint32  runt_det  : 1;
        uint32  frm_trunc : 1;
        uint32  ucast_det : 1;
        uint32  vlan      : 1;
        uint32  ctrl_frm  : 1;
        uint32  bcast_det : 1;
        uint32  mcast_det : 1;
        uint32  crc_err   : 1;
        uint32  rx_err    : 1;
    };
} DmaRxStatusSelReg_t;

typedef union DmaRxOkToSendCountReg_s {
    uint32  word;
    struct {
        uint32  unused           :28;
        uint32  ok_to_send_count : 4;
    };
} DmaRxOkToSendCountReg_t;




typedef struct GmacIntf {
/*0x00*/    uint32                  Control; 
/*0x04*/    MibCtrlReg_t            MibCtrl;
/*0x08*/    uint32                  unused; 
/*0x0C*/    MibMaxPktSizeReg_t      MibMaxPktSize;
/*0x10*/    RxBpThreshReg_t         RxBpThreshLo;
/*0x14*/    RxBpThreshReg_t         RxBpThreshHi;
/*0x18*/    RxFlowCtrlReg_t         RxFlowCtrl; 
/*0x1C*/    uint32                  DiagOut; 
/*0x20*/    BpForceReg_t            BpForce;
/*0x24*/    IrqEnableReg_t          IrqEnable;
/*0x28*/    GmacStatusReg_t         GmacStatus;
/*0x2C*/    IrqStatusReg_t          IrqStatus; 
/*0x30*/    uint32                  OverFlowCounter; 
/*0x34*/    uint32                  BackPressCounter;
/*0x38*/    MacSwResetReg_t         MacSwReset;  
/*0x3C*/    DmaRxStatusSelReg_t     DmaRxStatusSel;
/*0x40*/    DmaRxOkToSendCountReg_t DmaRxOkToSendCount;
} GmacIntf;

#define GMAC_INTF ((volatile GmacIntf * const) (GMAC_BASE+0x800))

typedef struct GmacMIBRegs {
/*0x00*/    unsigned int RxFCSErrs;
/*0x04*/    unsigned int RxCtrlFrame;
/*0x08*/    unsigned int RxPausePkts;
/*0x0c*/    unsigned int RxUnknown;
/*0x10*/    unsigned int RxAlignErrs;
/*0x14*/    unsigned int RxExcessSizeDisc; /* TODO not sure about counter */
/*0x18*/    unsigned int RxSymbolError;
/*0x1c*/    unsigned int RxCarrierSenseErrs;
/*0x20*/    unsigned int RxOversizePkts;
/*0x24*/    unsigned int RxJabbers;
/*0x28*/    unsigned int RxMtuErrs;
/*0x2c*/    unsigned int RxRuntPkts; /* RxUnderSizePkts + RxFragments */
/*0x30*/    unsigned int RxUndersizePkts;
/*0x34*/    unsigned int RxFragments;
/*0x38*/    unsigned int RxRuntOctets;
/*0x3c*/    unsigned int RxMulticastPkts;
/*0x40*/    unsigned int RxBroadcastPkts;
/*0x44*/    unsigned int Pkts64Octets;
/*0x48*/    unsigned int Pkts65to127Octets;
/*0x4c*/    unsigned int Pkts128to255Octets;
/*0x50*/    unsigned int Pkts256to511Octets;
/*0x54*/    unsigned int Pkts512to1023Octets;

/*TODO mapping to ROBO */
/*0x58*/    unsigned int Pkts1024to1518Octets;
/*0x5c*/    unsigned int Pkts1519to1522;
/*0x60*/    unsigned int Pkts1523to2047;
/*0x64*/    unsigned int Pkts2048to4095;
/*0x68*/    unsigned int Pkts4096to8191; /* Actually it is upto 9216 */
/*0x6c*/    unsigned int RxPkts;
/*0x70*/    unsigned int RxOctetsLo;

/*0x74*/    unsigned int RxUnicastPkts;
/*0x78*/    unsigned int RxGoodPkts;
/*0x7c*/    unsigned int RxPPPPkts;
/*0x80*/    unsigned int RxCRCMatchPkts;

/*0x84*/    unsigned int TxPausePkts;
/*0x88*/    unsigned int TxJabber;
/*0x8c*/    unsigned int TxFCSErrs;
/*0x90*/    unsigned int TxCtrlFrame;
/*0x94*/    unsigned int TxOversizePkts;
/*0x98*/    unsigned int TxDeferredTx;
/*0x9c*/    unsigned int TxExcessiveDef;
/*0xa0*/    unsigned int TxSingleCol;
/*0xa4*/    unsigned int TxMultipleCol;
/*0xa8*/    unsigned int TxLateCol;
/*0xac*/    unsigned int TxExcessiveCol;
/*0xb0*/    unsigned int TxFragments;
/*0xb4*/    unsigned int TxCol;
/*0xb8*/    unsigned int TxMulticastPkts;
/*0xbc*/    unsigned int TxBroadcastPkts;

/* No mapping in ROBO for TX octet counters */
/*0xc0*/    unsigned int TxPkts64Octets;
/*0xc4*/    unsigned int TxPkts65to127Octets;
/*0xc8*/    unsigned int TxPkts128to255Octets;
/*0xcc*/    unsigned int TxPkts256to511Octets;
/*0xd0*/    unsigned int TxPkts512to1023Octets;
/*0xd4*/    unsigned int TxPkts1024to1518Octets;
/*0xd8*/    unsigned int TxPkts1519to1522;
/*0xdc*/    unsigned int TxPkts1523to2047;
/*0xe0*/    unsigned int TxPkts2048to4095;
/*0xe4*/    unsigned int TxPkts4096to8191; /* Actually it is upto 9216 */

/*0xe8*/    unsigned int TxPkts;
/*0xec*/    unsigned int TxOctetsLo;
/*0xf0*/    unsigned int TxUnicastPkts;
/*0xf4*/    unsigned int TxGoodPkts;

/*
 * Need to map GMAC counters to these ROBO counters
    unsigned int TxDropPkts;
    unsigned int TxQoSPkts;
    unsigned int TxFrameInDisc;
    unsigned int TxQoSOctetsLo;
    unsigned int TxQoSOctetsHi;

    unsigned int RxGoodOctetsLo;
    unsigned int RxGoodOctetsHi;
    unsigned int RxDropPkts;
    unsigned int RxSAChanges;
    unsigned int RxQoSOctetsLo;
    unsigned int RxQoSOctetsHi;
*/
} GmacMIBRegs;

#define GMAC_MIB ((volatile GmacMac * const) (GMAC_BASE + 0xA00))

typedef union CmdReg_s {
    uint32 word;
    struct {
        uint32 unused3       : 1;   /* bit 31 */
        uint32 runt_filt_dis : 1;   /* bit 30 */
        uint32 txrx_en_cfg   : 1;   /* bit 29 */
        uint32 tx_pause_ign  : 1;   /* bit 28 */
        uint32 prbl_ena      : 1;   /* bit 27 */
        uint32 rx_err_disc   : 1;   /* bit 26 */
        uint32 rmt_loop_ena  : 1;   /* bit 25 */
        uint32 len_chk_dis   : 1;   /* bit 24 */
        uint32 ctrl_frm_ena  : 1;   /* bit 23 */
        uint32 ena_ext_cfg   : 1;   /* bit 22 */
        uint32 unused2       : 6;   /* bit 21:16 */
        uint32 lcl_loop_ena  : 1;   /* bit 15 */
        uint32 unused1       : 1;   /* bit 14 */
        uint32 sw_reset      : 1;   /* bit 13 */
        uint32 unused0       : 2;   /* bit 12:11 */
        uint32 hd_ena        : 1;   /* bit 10 */
        uint32 tx_addr_ins   : 1;   /* bit  9 */
        uint32 rx_pause_ign  : 1;   /* bit  8 */
        uint32 pause_fwd     : 1;   /* bit  7 */
        uint32 crc_fwd       : 1;   /* bit  6 */
        uint32 pad_rem_en    : 1;   /* bit  5 */
        uint32 promis_en     : 1;   /* bit  4 */
        uint32 eth_speed     : 2;   /* bit 3:2 */
#define CMD_ETH_SPEED_10            0
#define CMD_ETH_SPEED_100           1
#define CMD_ETH_SPEED_1000          2
#define CMD_ETH_SPEED_2500          3

        uint32 rx_ena        : 1;   /* bit  1 */
        uint32 tx_ena        : 1;   /* bit  0 */
    };
} CmdReg_t;

typedef union FrmLenReg_s {
    uint32  word;
    struct {
        uint32 unused  : 18;   
        uint32 frm_len : 14;   /* bit 13:0 */
    };
} FrmLenReg_t;

typedef union PauseQuantaReg_s {
    uint32  word;
    struct {
        uint32 unused       : 16;   
        uint32 pause_quanta : 16;   /* bit 15:0 */
    };
} PauseQuantaReg_t;

typedef union ModeReg_s {
    uint32  word;
    struct {
        uint32 unused       : 26;   
        uint32 mac_link_up  : 1;   /* bit  5 */
        uint32 mac_tx_pause : 1;   /* bit  4 */
        uint32 mac_rx_pause : 1;   /* bit  3 */
        uint32 mac_dplx     : 1;   /* bit  2 */
        uint32 mac_speed    : 2;   /* bit 1:0 */
    };
} ModeReg_t;

typedef union FrmTagReg_s {
    uint32  word;
    struct {
        uint32 unused  : 15;   
        uint32 tpid_en :  1;    /* bit 16 */
        uint32 tag     : 16;    /* bit 15:0 */
    };
} FrmTagReg_t;

typedef union TxIpgLenReg_s {
    uint32  word;
    struct {
        uint32 unused     :27;   
        uint32 tx_ipg_len : 5;  /* bit 4:0 */
    };
} TxIpgLenReg_t;

typedef union RxIpgInvReg_s {
    uint32  word;
    struct {
        uint32 unused     :31;   
        uint32 rx_ipg_inv : 1;  /* bit 0 */
    };
} RxIpgInvReg_t;

typedef union RepPauseCtrlReg_s {
    uint32  word;
    struct {
        uint32 unused      :14;   
        uint32 pause_en    : 1; /* bit 17 */
        uint32 pause_timer :17; /* bit 16:0 */
    };
} RepPauseCtrlReg_t;

typedef union TxFifoFlushReg_s {
    uint32  word;
    struct {
        uint32 unused   :31;   
        uint32 tx_flush : 1; /* bit 0 */
    };
} TxFifoFlushReg_t;

typedef struct RxFifoStatusReg_s {
    uint32  word;
    struct {
        uint32 unused          :30;   
        uint32 rxfifo_overrun  : 1; /* bit 1 */
        uint32 rxfifo_underrun : 1; /* bit 0 */
    };
} RxFifoStatusReg_t;

typedef union TxFifoStatusReg_s {
    uint32 word;
    struct {
        uint32 unused          :30;   
        uint32 txfifo_overrun  : 1; /* bit 1 */
        uint32 txfifo_underrun : 1; /* bit 0 */
    };
} TxFifoStatusReg_t;


typedef struct GmacMac {
    uint32 UmacDummy;               /* 0x00 */
    uint32 HdBkpCntl;               /* 0x04 */
    CmdReg_t Cmd;                   /* 0x08 */
    uint32 Mac0;                    /* 0x0c */
    uint32 Mac1;                    /* 0x10 */
    FrmLenReg_t FrmLen;             /* 0x14 */
    PauseQuantaReg_t PauseQuanta;   /* 0x18 */
    uint32 unused1[9];              /* 0x1c - 0x3c */
    uint32 SfdOffset;               /* 0x40 */
    ModeReg_t Mode;                 /* 0x44 */
    FrmTagReg_t FrmTag0;            /* 0x48 */
    FrmTagReg_t FrmTag1;            /* 0x4c */
    uint32 unused2[3];              /* 0x50 - 0x58 */
    TxIpgLenReg_t TxIpgLen;         /* 0x5c */
    uint32 unused3[6];              /* 0x60 - 0x74 */
    RxIpgInvReg_t RxIpgInv;         /* 0x78 */
    uint32 unused4[165];            /* 0x7c - 0x30c */
    uint32 MacsecProgTxCrc;         /* 0x310 */
    uint32 MacsecCtrl;              /* 0x314 */
    uint32 unused5[6];              /* 0x318 - 0x32c */
    RepPauseCtrlReg_t PauseCtrl;    /* 0x330 */
    TxFifoFlushReg_t TxFifoFlush;   /* 0x334 */
    RxFifoStatusReg_t RxFifoStatus; /* 0x338 */
    TxFifoStatusReg_t TxFifoStatus; /* 0x33c */
} GmacMac;

#define GMAC_MAC ((volatile GmacMac * const) (GMAC_BASE + 0xC00))

typedef struct gmacEEECtrl {
    uint32 unused1       :  16;
    uint32 softReset     :  1;
    uint32 unused2       :  10;
    uint32 linkUp        :  1;
    uint32 lpiCntrSnap   :  1;
    uint32 lpiCntrClr    :  1;
    uint32 halt          :  1;
    uint32 enable        :  1;
} gmacEEECtrl_t;
typedef struct gmacEEEStat {
    uint32 idle          :  1;
    uint32 halt          :  1;
    uint32 enable        :  1;
    uint32 softReset     :  1;
    uint32 pktFull       :  1;
    uint32 pktEmpty      :  1;
    uint32 fifoFull      :  1;
    uint32 fifoEmpty     :  1;
    uint32 fullDplx      :  1;
    uint32 speed         :  2;
    uint32 unused1       :  1;
    uint32 currState     :  4;
    uint32 lpiCntr       :  16;
} gmacEEEStat_t;
typedef struct GmacEEE {
    gmacEEECtrl_t eeeCtrl;   /* @ 0x1000e850 */
    gmacEEEStat_t eeeStat;
    uint32  eeeT1000WakeTime;
    uint32  eeeTx100WakeTime;
    uint32  eeeLPIWaitTime;
} GmacEEE_t;

#define GMAC_EEE (volatile GmacEEE_t *const) ((unsigned char *)GMAC_INTF + 0x50)
#endif /* defined(CONFIG_BCM_GMAC) */
#endif

typedef struct {
    unsigned int led_f;
    unsigned int reserved;
} LED_F;

typedef struct EthernetSwitchCore
{
    unsigned int port_traffic_ctrl[9];            /* 0x00 - 0x08 */
    unsigned int reserved1[2];                    /* 0x09 - 0x0a */
    unsigned int switch_mode;                     /* 0x0b */
#define ETHSW_SM_RETRY_LIMIT_DIS                  0x04
#define ETHSW_SM_FORWARDING_EN                    0x02
#define ETHSW_SM_MANAGED_MODE                     0x01
    unsigned int pause_quanta;                    /* 0x0c */
    unsigned int reserved33; 
    unsigned int imp_port_state;                  /*0x0e */
#define ETHSW_IPS_USE_MII_HW_STS                  0x00
#define ETHSW_IPS_USE_REG_CONTENTS                0x80
#define ETHSW_IPS_GMII_SPEED_UP_NORMAL            0x00
#define ETHSW_IPS_GMII_SPEED_UP_2G                0x40
#define ETHSW_IPS_TXFLOW_NOT_PAUSE_CAPABLE        0x00
#define ETHSW_IPS_TXFLOW_PAUSE_CAPABLE            0x20
#define ETHSW_IPS_RXFLOW_NOT_PAUSE_CAPABLE        0x00
#define ETHSW_IPS_RXFLOW_PAUSE_CAPABLE            0x10
#define ETHSW_IPS_SW_PORT_SPEED_1000M_2000M       0x08
#define ETHSW_IPS_DUPLEX_MODE                     0x02
#define ETHSW_IPS_LINK_FAIL                       0x00
#define ETHSW_IPS_LINK_PASS                       0x01
    unsigned int led_refresh;                     /* 0x0f */
    LED_F        led_function[2];                 /* 0x10 */
    unsigned int led_function_map;                /* 0x14 */
    unsigned int reserved14; 
    unsigned int led_enable_map;                  /* 0x16 */
    unsigned int reserved15; 
    unsigned int led_mode_map0;                   /* 0x18 */
    unsigned int reserved16; 
    unsigned int led_function_map1;               /* 0x1a */
    unsigned int reserved17; 
    unsigned int reserved2[5];                    /* 0x1c - 0x20 */
    unsigned int port_forward_ctrl;               /* 0x21 */
    unsigned int switch_ctrl;                     /* 0x22 */
#define ETHSW_SC_MII_DUMP_FORWARDING_EN           0x40
#define ETHSW_SC_MII2_VOL_SEL                     0x02
    unsigned int reserved3;                       /* 0x23 */
    unsigned int protected_port_selection;        /* 0x24 */
    unsigned int reserved18; 
    unsigned int wan_port_select;                 /* 0x26 */
    unsigned int reserved19; 
    unsigned int pause_capability;                /* 0x28 */
    unsigned int reserved20[3]; 
    unsigned int reserved4[3];                    /* 0x2c - 0x2e */
    unsigned int reserved_multicast_control;      /* 0x2f */
    unsigned int reserved5;                       /* 0x30 */
    unsigned int txq_flush_mode_control;          /* 0x31 */
    unsigned int ulf_forward_map;                 /* 0x32 */
    unsigned int reserved21; 
    unsigned int mlf_forward_map;                 /* 0x34 */
    unsigned int reserved22; 
    unsigned int mlf_impc_forward_map;            /* 0x36 */
    unsigned int reserved23; 
    unsigned int pause_pass_through_for_rx;       /* 0x38 */
    unsigned int reserved24; 
    unsigned int pause_pass_through_for_tx;       /* 0x3a */
    unsigned int reserved25; 
    unsigned int disable_learning;                /* 0x3c */
    unsigned int reserved26; 
    unsigned int reserved6[26];                   /* 0x3e - 0x57 */
    unsigned int port_state_override[8];          /* 0x58 - 0x5f */
#define ETHSW_PS_SW_OVERRIDE                      0x40
#define ETHSW_PS_SW_TX_FLOW_CTRL_EN               0x20
#define ETHSW_PS_SW_RX_FLOW_CTRL_EN               0x10
#define ETHSW_PS_SW_PORT_SPEED_1000M              0x80
#define ETHSW_PS_SW_PORT_SPEED_100M               0x40
#define ETHSW_PS_SW_PORT_SPEED_10M                0x00
#define ETHSW_PS_DUPLEX_MODE                      0x02
#define ETHSW_PS_LINK_DOWN                        0x00
#define ETHSW_PS_LINK_UP                          0x01
    unsigned int reserved7[4];                    /* 0x60 - 0x63 */
    unsigned int imp_rgmii_ctrl_p4;               /* 0x64 */
    unsigned int imp_rgmii_ctrl_p5;               /* 0x65 */
    unsigned int reserved8[6];                    /* 0x66 - 0x6b */
    unsigned int rgmii_timing_delay_p4;           /* 0x6c */
    unsigned int gmii_timing_delay_p5;            /* 0x6d */
    unsigned int reserved9[11];                   /* 0x6e - 0x78 */
    unsigned int software_reset;                  /* 0x79 */
    unsigned int reserved13[6];                   /* 0x7a - 0x7f */
    unsigned int pause_frame_detection;           /* 0x80 */
    unsigned int reserved10[7];                   /* 0x81 - 0x87 */
    unsigned int fast_aging_ctrl;                 /* 0x88 */
    unsigned int fast_aging_port;                 /* 0x89 */
    unsigned int fast_aging_vid;                  /* 0x8a */
    unsigned int anonymous1[376];                 /* 0x8b */
    unsigned int brcm_hdr_ctrl;                   /* 0x203 */
    unsigned int anonymous2[0x2efc];              /* 0x204 */
    unsigned int port_vlan_ctrl[9*2];               /* 0x3100 */
} EthernetSwitchCore;

#define PBMAP_MIPS 0x100
#define ETHSW_CORE ((volatile EthernetSwitchCore * const) SWITCH_BASE)

typedef struct EthernetSwitchReg
{
    uint32 switch_ctrl;                      /* 0x0000 */
    uint32 switch_status;                    /* 0x0004 */
    uint32 dir_data_write_reg;               /* 0x0008 */
    uint32 dir_data_read_reg;                /* 0x000c */
    uint32 led_serial_refresh_time_unit;     /* 0x0010 */
    uint32 reserved1;                        /* 0x0014 */
    uint32 switch_rev;                       /* 0x0018 */
    uint32 phy_rev;                          /* 0x001c */
    uint32 phy_test_ctrl;                    /* 0x0020 */
    uint32 qphy_ctrl;                        /* 0x0024 */
#define ETHSW_QPHY_CTRL_PHYAD_BASE_SHIFT     12
#define ETHSW_QPHY_CTRL_PHYAD_BASE_MASK      (0x1f<<ETHSW_QPHY_CTRL_PHYAD_BASE_SHIFT)
#define ETHSW_QPHY_CTRL_RESET_SHIFT          8
#define ETHSW_QPHY_CTRL_RESET_MASK           (0x1<<ETHSW_QPHY_CTRL_RESET_SHIFT )
#define ETHSW_QPHY_CTRL_CK25_DIS_SHIFT       7
#define ETHSW_QPHY_CTRL_CK25_DIS_MASK        (0x1<<ETHSW_QPHY_CTRL_CK25_DIS_SHIFT)
#define ETHSW_QPHY_CTRL_EXT_PWR_DOWN_SHIFT   1
#define ETHSW_QPHY_CTRL_EXT_PWR_DOWN_MASK    (0xf<<ETHSW_QPHY_CTRL_EXT_PWR_DOWN_SHIFT)
#define ETHSW_QPHY_CTRL_IDDQ_BIAS_SHIFT      0
#define ETHSW_QPHY_CTRL_IDDQ_BIAS_MASK       (0x1<<ETHSW_QPHY_CTRL_IDDQ_BIAS_SHIFT)
    uint32 qphy_status;                      /* 0x0028 */
    uint32 sphy_ctrl;                        /* 0x002c */
#define ETHSW_SPHY_CTRL_PHYAD_SHIFT          8
#define ETHSW_SPHY_CTRL_PHYAD_MASK           (0x1f<<ETHSW_SPHY_CTRL_PHYAD_SHIFT)
#define ETHSW_SPHY_CTRL_RESET_SHIFT          5
#define ETHSW_SPHY_CTRL_RESET_MASK           (0x1<<ETHSW_SPHY_CTRL_RESET_SHIFT )
#define ETHSW_SPHY_CTRL_CK25_DIS_SHIFT       4
#define ETHSW_SPHY_CTRL_CK25_DIS_MASK        (0x1<<ETHSW_SPHY_CTRL_CK25_DIS_SHIFT)
#define ETHSW_SPHY_CTRL_EXT_PWR_DOWN_SHIFT   1
#define ETHSW_SPHY_CTRL_EXT_PWR_DOWN_MASK    (0x1<<ETHSW_SPHY_CTRL_EXT_PWR_DOWN_SHIFT)
#define ETHSW_SPHY_CTRL_IDDQ_BIAS_SHIFT      0
#define ETHSW_SPHY_CTRL_IDDQ_BIAS_MASK       (0x1<<ETHSW_SPHY_CTRL_IDDQ_BIAS_SHIFT)
    uint32 sphy_status;                      /* 0x0030 */
    uint32 reserved2[15];                    /* 0x0034 */
    uint32 rgmii_5_ctrl;                     /* 0x0070 */
    uint32 rgmii_5_ib_status;                /* 0x0074 */
    uint32 rgmii_5_rx_clk_delay_ctrl;        /* 0x0078 */
    uint32 rgmii_7_ctrl;                     /* 0x007c */
    uint32 rgmii_7_ib_status;                /* 0x0080 */
    uint32 rgmii_7_rx_clk_delay_ctrl;        /* 0x0084 */
    uint32 led_blink_rate_ctrl;              /* 0x0088 */
    uint32 led_serial_ctrl;                  /* 0x008c */
    uint32 led_ctrl[5];                      /* 0x0090 */
#define ETHSW_LED_CTRL_SPD0_ON               0x0
#define ETHSW_LED_CTRL_SPD0_OFF              0x1
#define ETHSW_LED_CTRL_SPD1_ON               0x0
#define ETHSW_LED_CTRL_SPD1_OFF              0x2
#define ETHSW_LED_CTRL_1000M_SHIFT           6
#define ETHSW_LED_CTRL_100M_SHIFT            4
#define ETHSW_LED_CTRL_10M_SHIFT             2
#define ETHSW_LED_CTRL_NOLINK_SHIFT          0
#define ETHSW_LED_CTRL_ALL_SPEED_MASK        0xff
#define ETHSW_LED_CTRL_SPEED_MASK            0x3
    uint32 reserved3[2];                     /* 0x00a4 */
    uint32 crossbar_switch_ctrl;             /* 0x00ac */
    uint32 reserved4[6];                     /* 0x00b0 */
    uint32 rgmii_11_ctrl;                    /* 0x00c8 */
    uint32 rgmii_11_ib_status;               /* 0x00cc */
    uint32 rgmii_11_rx_clk_delay_ctrl;       /* 0x00d0 */
    uint32 rgmii_12_ctrl;                    /* 0x00d4 */
#define ETHSW_RC_MII_MODE_MASK               0x1c
#define ETHSW_RC_EXT_RVMII                   0x10
#define ETHSW_RC_EXT_GPHY                    0x0c
#define ETHSW_RC_EXT_EPHY                    0x08
#define ETHSW_RC_INT_GPHY                    0x04
#define ETHSW_RC_INT_EPHY                    0x00
#define ETHSW_RC_ID_MODE_DIS                 0x02
#define ETHSW_RC_RGMII_EN                    0x01
    uint32 rgmii_12_ib_status;               /* 0x00d8 */
    uint32 rgmii_12_rx_clk_delay_ctrl;       /* 0x00dc */
#define ETHSW_RXCLK_IDDQ                     (1<<4)
#define ETHSW_RXCLK_BYPASS                   (1<<5)
    uint32 anonymous1[44];                   /* 0x00e0 */
    uint32 single_serdes_rev;                /* 0x0190 */ /* From here for B0 and late chip only */
    uint32 single_serdes_ctrl;               /* 0x0194 */
    uint32 single_serdes_stat;               /* 0x0198 */
    uint32 led_wan_ctrl;                     /* 0x019c */
} EthernetSwitchReg;

#define ETHSW_REG ((volatile EthernetSwitchReg * const) (SWITCH_BASE + 0x00040000))

#define PHY_TEST_CTRL                                ((volatile unsigned int*)(&ETHSW_REG->phy_test_ctrl))
#define SPHY_CNTRL                                   ((volatile unsigned int*)(&ETHSW_REG->sphy_ctrl))
#define QPHY_CNTRL                                   ((volatile unsigned int*)(&ETHSW_REG->qphy_ctrl))

typedef struct EthernetSwitchMDIO
{
    uint32 mdio_cmd;                          /* 0x0000 */
#define ETHSW_MDIO_BUSY                       (1 << 29)
#define ETHSW_MDIO_FAIL                       (1 << 28)
#define ETHSW_MDIO_CMD_SHIFT                  26
#define ETHSW_MDIO_CMD_MASK                   (0x3<<ETHSW_MDIO_CMD_SHIFT) 
#define ETHSW_MDIO_CMD_C22_READ               2
#define ETHSW_MDIO_CMD_C22_WRITE              1
#define ETHSW_MDIO_C22_PHY_ADDR_SHIFT         21
#define ETHSW_MDIO_C22_PHY_ADDR_MASK          (0x1f<<ETHSW_MDIO_C22_PHY_ADDR_SHIFT)
#define ETHSW_MDIO_C22_PHY_REG_SHIFT          16
#define ETHSW_MDIO_C22_PHY_REG_MASK           (0x1f<<ETHSW_MDIO_C22_PHY_REG_SHIFT)
#define ETHSW_MDIO_PHY_DATA_SHIFT             0
#define ETHSW_MDIO_PHY_DATA_MASK              (0xffff<<ETHSW_MDIO_PHY_DATA_SHIFT)
    uint32 mdio_cfg;                      /* 0x0004 */
} EthernetSwitchMDIO;

#define SWITCH_ACB_OFFSET        0x40600

#define SWITCH_MDIO_OFFSET       0x403c0
#define ETHSW_MDIO ((volatile EthernetSwitchMDIO * const) (SWITCH_BASE + SWITCH_MDIO_OFFSET))

#ifdef __cplusplus
}
#endif

typedef struct usb_ctrl{
    uint32 setup;
#define USBH_IPP                (1<<5)
#define USBH_IOC                (1<<4)
#define USB_DEVICE_SEL          (1<<11)
#define USB3_OC_DISABLE_PORT0   (1<<30)
#define USB3_OC_DISABLE_PORT1   (1<<31)
    uint32 pll_ctl;
    uint32 fladj_value;
    uint32 bridge_ctl;
#define EHCI_ENDIAN_SWAP        (1<<3)
#define EHCI_DATA_SWAP          (1<<2)
#define OHCI_ENDIAN_SWAP        (1<<1)
#define OHCI_DATA_SWAP          (1<<0)
    uint32 spare1;
    uint32 mdio;
    uint32 mdio2;
    uint32 test_port_control;
    uint32 usb_simctl;
#define USBH_OHCI_MEM_REQ_DIS   (1<<1)
    uint32 usb_testctl;
    uint32 usb_testmon;
    uint32 utmi_ctl_1;
    uint32 spare2;
    uint32 usb_pm;
#define XHC_SOFT_RESETB         (1<<30)
    uint32 usb_pm_status;
    uint32 spare4;
    uint32 pll_ldo_ctl;
    uint32 pll_ldo_pllbias;
    uint32 pll_afe_bg_cntl;
    uint32 afe_usbio_tst;
    uint32 pll_ndiv_frac;
    uint32 unused1;
    uint32 unused2;
    uint32 unused3;
    uint32 usb30_ctl1;
#define PHY3_PLL_SEQ_START      (1<<4)
    uint32 usb30_ctl2;
    uint32 usb30_ctl3;
    uint32 usb30_ctl4;
    uint32 usb30_pctl;
    uint32 usb30_ctl5;
    uint32 spare5;
#define GC_PLL_SUSPEND_EN       (1<<1)
} usb_ctrl;

#define USBH_CTRL ((volatile usb_ctrl * const) USBH_CFG_BASE)

typedef struct dslphy_afe {
    uint32 AfeReg[2];
    uint32 Private1[42];
    uint32 BgBiasReg[12];
} dslphy_afe ;

#define DSLPHY_AFE ((volatile dslphy_afe  * const) DSLPHY_AFE_BASE)

#if defined(CONFIG_BCM963138)
#define CONFIG_5x3_CROSSBAR_SUPPORT
#else
#define CONFIG_4x2_CROSSBAR_SUPPORT
#endif


typedef struct Jtag_Otp {
	uint32 ctrl0;		/* 0x00 */
#define JTAG_OTP_CTRL_BYPASS_OTP_CLK		(1 << 31)
#define JTAG_OTP_CTRL_READ_FOUT			(1 << 30)
#define JTAG_OTP_CTRL_TESTCOL			(1 << 29)
#define JTAG_OTP_CTRL_CPU_DEBUG_SEL		(0xf << 25)
#define JTAG_OTP_CTRL_BURST_UART_SEL		(1 << 24)
#define JTAG_OTP_CTRL_ACCESS_MODE		(0x2 << 22)
#define JTAG_OTP_CTRL_PROG_EN			(1 << 21)
#define JTAG_OTP_CTRL_DEBUG_MDE			(1 << 20)
#define JTAG_OTP_CTRL_WRP_CONTINUE_ON_FAIL	(1 << 19)
#define JTAG_OTP_CTRL_WRP_PROGRAM_VERIFY_FLAG	(1 << 18)
#define JTAG_OTP_CTRL_WRP_DOUBLE_WORD		(1 << 17)
#define JTAG_OTP_CTRL_WRP_REGC_SEL		(0x7 << 14)
#define JTAG_OTP_CTRL_WRP_QUADFUSE		(1 << 13)
#define JTAG_OTP_CTRL_WRP_DOUBLEFUSE		(1 << 12)
#define JTAG_OTP_CTRL_WRP_READ4X		(1 << 11)
#define JTAG_OTP_CTRL_WRP_READ2X		(1 << 10)
#define JTAG_OTP_CTRL_WRP_IN_DEBUG		(1 << 9)
#define JTAG_OTP_CTRL_COMMAND			(0x1f << 1)
#define JTAG_OTP_CTRL_CMD_READ		(0x00 << 1)
#define JTAG_OTP_CTRL_CMD_READBURST	(0x01 << 1)
#define JTAG_OTP_CTRL_CMD_OTP_PROG_EN	(0x02 << 1)
#define JTAG_OTP_CTRL_CMD_OTP_PROG_DIS	(0x03 << 1)
#define JTAG_OTP_CTRL_CMD_PRESCREEN	(0x04 << 1)
#define JTAG_OTP_CTRL_CMD_PRESCREEN_RP	(0x05 << 1)
#define JTAG_OTP_CTRL_CMD_FLUSH		(0x06 << 1)
#define JTAG_OTP_CTRL_CMD_NOP		(0x07 << 1)
#define JTAG_OTP_CTRL_CMD_PROG		(0x0a << 1)
#define JTAG_OTP_CTRL_CMD_PROG_RP	(0x0b << 1)
#define JTAG_OTP_CTRL_CMD_PROG_OVST	(0x0c << 1)
#define JTAG_OTP_CTRL_CMD_RELOAD	(0x0d << 1)
#define JTAG_OTP_CTRL_CMD_ERASE		(0x0e << 1)
#define JTAG_OTP_CTRL_CMD_LOAD_RF	(0x0f << 1)
#define JTAG_OTP_CTRL_CMD_CTRL_WR	(0x10 << 1)
#define JTAG_OTP_CTRL_CMD_CTRL_RD	(0x11 << 1)
#define JTAG_OTP_CTRL_CMD_READ_HP	(0x12 << 1)
#define JTAG_OTP_CTRL_CMD_READ_OVST	(0x13 << 1)
#define JTAG_OTP_CTRL_CMD_READ_VERIFY0	(0x14 << 1)
#define JTAG_OTP_CTRL_CMD_READ_VERIFY1	(0x15 << 1)
#define JTAG_OTP_CTRL_CMD_READ_FORCE0	(0x16 << 1)
#define JTAG_OTP_CTRL_CMD_READ_FORCE1	(0x17 << 1)
#define JTAG_OTP_CTRL_CMD_BURNING	(0x18 << 1)
#define JTAG_OTP_CTRL_CMD_PROG_LOCK	(0x19 << 1)
#define JTAG_OTP_CTRL_CMD_PROG_TESTCOL	(0x1a << 1)
#define JTAG_OTP_CTRL_CMD_READ_TESTCOL	(0x1b << 1)
#define JTAG_OTP_CTRL_CMD_READ_FOUT	(0x1e << 1)
#define JTAG_OTP_CTRL_CMD_SFT_RESET	(0x1f << 1)

#define JTAG_OTP_CTRL_START			(1 << 0)
	uint32 ctrl1;		/* 0x04 */
#define JTAG_OTP_CTRL_CPU_MODE			(1 << 0)
	uint32 ctrl2;		/* 0x08 */
	uint32 ctrl3;		/* 0x0c */
 	uint32 ctrl4;		/* 0x10 */
	uint32 status0;		/* 0x14 */
	uint32 status1;		/* 0x18 */
#define JTAG_OTP_STATUS_1_PROG_OK       (1 << 2) 
#define JTAG_OTP_STATUS_1_CMD_DONE      (1 << 1)
	uint32 status2;		/* 0x1c */
	uint32 status3;		/* 0x20 */
#define OTP_SAR_DISABLE	      0
#define OTP_VDSL_DSLDISABLE   1
#define OTP_DIS2LINE          2
#define OTP_VDSL_DISVDSL      3
#define OTP_VDSL_DIS6BND      4
#define OTP_VDSL_DISRNC       5
	uint32 status4;		/* 0x24 */
#define OTP_SF2_DISABLE       32
#define OTP_PCIE_RCAL_VALID	33
	uint32 status5;		/* 0x28 */
#define OTP_SPHY_DISABLE      64
#define OTP_RNR_DISIPSEC      75
#define OTP_RNR_DISSHA1       76
#define OTP_RNR_DISSHA2       77
#define OTP_PRNR_DISAES256    78
#define OTP_PRNR_DISAES192    79
#define OTP_PRNR_DISAES128    80
#define OTP_USBH_DISEHCI      81
#define OTP_USBH_XHCIDIS      82
#define OTP_USBD_DISABLE      83
#define OTP_PCIE_DISABLE0     84
#define OTP_PCIE_DISABLE1     85
#define OTP_A9_DISC1          86
#define OTP_A9_DISNEON        87
#define OTP_SATA_DISABLE      88
#define OTP_DECT_DISABLE      89	
	uint32 status6;		/* 0x2c */
#define OTP_TBUS_DISABLE      120
#define OTP_PMC_BOOT          121	
	uint32 status7;		/* 0x30 */
#define OTP_PMC_BOOTROM       132
#define OTP_PMC_SECUREREG     133
#define OTP_PBMU_DISABLE      134
	uint32 status8;		/* 0x34 */
	uint32 status9;		/* 0x38 */
#define OTP_DDR_SECUREACC     211
#define OTP_DDR_SECIRELCK     212
#define OTP_ECC_MREPAIR_EN    217	
} Jtag_Otp;

#define JTAG_OTP ((volatile Jtag_Otp * const) JTAG_OTP_BASE)

#define OTP_GET_USER_BIT(x)      (*((unsigned int*)&(JTAG_OTP->status3) + (x)/32) >> ((x) % 32) & 1)

#define BTRM_OTP_READ_TIMEOUT_CNT               0x10000

/* row 17 */
#define OTP_BRCM_BTRM_BOOT_ENABLE_ROW       17
#define OTP_BRCM_BTRM_BOOT_ENABLE_SHIFT         3
#define OTP_BRCM_BTRM_BOOT_ENABLE_MASK          (1 << OTP_BRCM_BTRM_BOOT_ENABLE_SHIFT)

#define OTP_BRCM_MEK_MIV_ROW                17
#define OTP_BRCM_MEK_MIV_SHIFT              7
#define OTP_BRCM_MEK_MIV_MASK               (7 << OTP_BRCM_MEK_MIV_SHIFT)

/* row 18 */
#define OTP_CUST_BTRM_BOOT_ENABLE_ROW       18
#define OTP_CUST_BTRM_BOOT_ENABLE_SHIFT         15
#define OTP_CUST_BTRM_BOOT_ENABLE_MASK          (7 << OTP_CUST_BTRM_BOOT_ENABLE_SHIFT)

/* row 24 */
#define OTP_CUST_MFG_MRKTID_ROW         24
#define OTP_CUST_MFG_MRKTID_SHIFT       0
#define OTP_CUST_MFG_MRKTID_MASK        (0xffff << OTP_CUST_MFG_MRKTID_SHIFT)

#define OTP_CUST_OP_INUSE_ROW               24
#define OTP_CUST_OP_INUSE_SHIFT             16
#define OTP_CUST_OP_INUSE_MASK              (1 << OTP_CUST_OP_INUSE_SHIFT)

/* row 25 */
#define OTP_CUST_OP_MRKTID_ROW          25
#define OTP_CUST_OP_MRKTID_SHIFT        0
#define OTP_CUST_OP_MRKTID_MASK         (0xffff << OTP_CUST_OP_MRKTID_SHIFT)

#endif

#endif

