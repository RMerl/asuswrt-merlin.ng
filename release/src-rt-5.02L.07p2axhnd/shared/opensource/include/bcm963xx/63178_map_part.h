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

#ifndef __BCM63178_MAP_PART_H
#define __BCM63178_MAP_PART_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bcmtypes.h"
#include "bcm_io_map.h"

#define CHIP_FAMILY_ID_HEX	        0x63178

#ifndef __ASSEMBLER__
typedef enum 
{
    MEMC_IDX,
    PMC_IDX,
    PERF_IDX,
    PERF1_IDX,
    NANDFLASH_IDX,
    PCM_IDX,
    PCMBUS_IDX,
    SWITCH_IDX,
    USBH_IDX,
    MST_PORT_NODE_PER_IDX,
    MST_PORT_NODE_USB_IDX,
    MST_PORT_NODE_CPU_IDX,
    MST_PORT_NODE_PMC_IDX,
    MST_PORT_NODE_PCIE0_IDX,
    MST_PORT_NODE_SWH_IDX,
    MST_PORT_NODE_WIFI_IDX,
    MST_PORT_NODE_DSL_IDX,
    MST_PORT_NODE_DSLCPU_IDX,
    UBUS4_COHERENCY_PORT_IDX,
    BIUCFG_IDX,
    BOOTLUT_IDX,
    UBUS_MAPPED_IDX,
    UBUS_CAPTURE_PORT_NODE_0,
    UBUS_CAPTURE_PORT_NODE_1,
    SYSPORT_IDX,
    DSLPHY_IDX,
    DSLLMEM_IDX,
    SAR_IDX,
    CCI500_IDX,
    LAST_IDX
} BCM_IO_MAP_IDX;
#endif

#define MEMC_PHYS_BASE              0x80180000
#define MEMC_SIZE                   0x24000

#define UBUS_MAPPED_PHYS_BASE       0x83000000
#define UBUS_MAPPED_SIZE            0x1000
#define SYS_CLK_CTRL_OFFSET         0x80
#define UBUS_SYS_MODULE_REGISTRATION_OFFSET 0x200

#define PMC_PHYS_BASE               0x80200000
#define PMC_SIZE                    0x00200000
#define PMC_OFFSET                  0x00100000
#define PROC_MON_OFFSET             0x00100000
#define PMB_OFFSET                  0x00120100

#define DSLPHY_PHYS_BASE            0x80650000
#define DSLPHY_SIZE                 0x20000
#define DSLPHY_OFFSET               0x0000
#define DSLLMEM_PHYS_BASE           0x80700000
#define DSLLMEM_SIZE                0x90000
#define DSLLMEM_OFFSET              0x0000

#define PERF_PHYS_BASE              0xff800000
#define PERF_SIZE                   0x13000
#define TIMR_OFFSET                 0x0400
#define WDTIMR0_OFFSET              0x0480
#define WDTIMR1_OFFSET              0x04c0
#define GPIO_OFFSET                 0x0500
#define BROM_OFFSET                 0x0600
#define RNG_OFFSET                  0x0b80
#define SOTP_OFFSET                 0x0c00
#define HSSPIM_OFFSET               0x1000
#define NAND_REG_OFFSET             0x1800
#define NAND_CACHE_OFFSET           0x1c00
#define NAND_INTR_OFFSET            0x2000
#define MISC_OFFSET                 0x2600
#define JTAG_OTP_OFFSET             0x2800
#define LED_OFFSET                  0x3000
#define AHB_CONTROL_OFFSET          0x10300
#define HS_UART_OFFSET              0x10400
#define PL081_DMA_OFFSET            0x11000
#define ARM_UART_OFFSET             0x12000

#define GPIO_PHYS_BASE              (PERF_PHYS_BASE + GPIO_OFFSET)
#define HS_UART_PHYS_BASE           (PERF_PHYS_BASE + HS_UART_OFFSET)
#define PL081_DMA_PHYS_BASE         (PERF_PHYS_BASE + PL081_DMA_OFFSET)
#define ARM_UART_PHYS_BASE          (PERF_PHYS_BASE + ARM_UART_OFFSET)
#define LED_PHYS_BASE               (PERF_PHYS_BASE + LED_OFFSET)
#define JTAG_OTP_PHYS_BASE          (PERF_PHYS_BASE + JTAG_OTP_OFFSET)
#define HSSPIM_PHYS_BASE            (PERF_PHYS_BASE + HSSPIM_OFFSET)
#define NAND_REG_PHYS_BASE          (PERF_PHYS_BASE + NAND_REG_OFFSET)
#define NAND_CACHE_PHYS_BASE        (PERF_PHYS_BASE + NAND_CACHE_OFFSET)
#define NAND_INTR_PHYS_BASE         (PERF_PHYS_BASE + NAND_INTR_OFFSET)
#define TIMR_PHYS_BASE              (PERF_PHYS_BASE + TIMR_OFFSET)
#define WDTIMR0_PHYS_BASE           (PERF_PHYS_BASE + WDTIMR0_OFFSET)
#define WDTIMR1_PHYS_BASE           (PERF_PHYS_BASE + WDTIMR1_OFFSET)
#define MISC_PHYS_BASE              (PERF_PHYS_BASE + MISC_OFFSET)

#define PERF1_PHYS_BASE             0xff85a000
#define PERF1_SIZE                  0x8000
#define TOP_CNTRL_OFFSET            0x0000

#define SPIFLASH_PHYS_BASE          0xffd00000  
#define SPIFLASH_SIZE               0x100000    
#define SPIFLASH_OFFSET             0x0000 

#define NANDFLASH_PHYS_BASE         0xffe00000  
#define NANDFLASH_SIZE              0x100000
#define NANDFLASH_OFFSET            0x0000

#define BIUCFG_PHYS_BASE            0x81060000
#define BIUCFG_SIZE                 0x3000
#define BIUCFG_OFFSET               0x0000

#define BOOTLUT_PHYS_BASE           0xffff0000
#define BOOTLUT_SIZE                0x1000

#define PCM_PHYS_BASE               0xff860000
#define APM_CORE_OFFSET             0x00000000  
#define PCM_CORE_OFFSET             0x00000C00
#define PCM_DMA_OFFSET              0x00001800
#define PCM_SIZE                    0x2000
#define PCMBUS_PHYS_BASE            0x83010A00
#define PCMBUS_OFFSET               0x00000000
#define PCMBUS_SIZE                 0x100

#define PCIE0_PHYS_BASE             0x80040000
#define PCIE0_SIZE                  0x0000A000
#define PCIE0_MEM_PHYS_BASE         0xC0000000
#define PCIE0_MEM_SIZE              0x10000000

#define WLAN0_PHYS_BASE             0x84000000
#define WLAN0_SIZE                  0x01000000

#define SWITCH_PHYS_BASE            0x80400000  
#define SWITCH_SIZE                 0x90000
#define SWITCH_CORE_OFFSET          0x00000
#define SWITCH_REG_OFFSET           0x80000
#define SWITCH_MDIO_OFFSET          0x805c0
#define SWITCH_ACB_OFFSET           0x80800

#define SYSPORT_PHYS_BASE         0x80490000  
#define SYSPORT_SIZE              0x10000
#define SYSPORT_OFFSET            0x00000
#define SYSPORT_SYSBUSCFG_OFFSET  0x00040
#define SYSPORT_RXCHK_OFFSET      0x300
#define SYSPORT_RBUF_OFFSET       0x400
#define SYSPORT_TBUF_OFFSET       0x600
#define SYSPORT_MPD_OFFSET        0xe00
#define SYSPORT_RDMA_OFFSET       0x2000
#define SYSPORT_TDMA_OFFSET       0x4000
#define SYSPORT_SPE_OFFSET        0x6000
#define SYSPORT_GIB_OFFSET        0x8000
#define SYSPORT_INTRL2_OFFSET     0x8200
#define SYSPORT_INTC_OFFSET       0x8300
#define SYSPORT_INTRL2_MISC_RX_OFFSET   0x8400
#define SYSPORT_INTRL2_MISC_TX_OFFSET   0x8500

#define GIC_PHYS_BASE           0x81000000
#define GIC_SIZE                0x10000
#define GIC_OFFSET              0x0000
#define GICD_OFFSET             0x1000
#define GICC_OFFSET             0x2000

#define MST_PORT_NODE_PER_PHYS_BASE         0x83010000
#define MST_PORT_NODE_PER_SIZE              0x1000
#define MST_PORT_NODE_USB_PHYS_BASE         0x83018000
#define MST_PORT_NODE_USB_SIZE              0x1000
#define MST_PORT_NODE_CPU_PHYS_BASE         0x83020000
#define MST_PORT_NODE_CPU_SIZE              0x1000
#define MST_PORT_NODE_PMC_PHYS_BASE         0x83028000 
#define MST_PORT_NODE_PMC_SIZE              0x1000
#define MST_PORT_NODE_PCIE0_PHYS_BASE       0x83030000
#define MST_PORT_NODE_PCIE0_SIZE            0x1000
#define MST_PORT_NODE_SWH_PHYS_BASE         0x83038000
#define MST_PORT_NODE_SWH_SIZE              0x1000
#define MST_PORT_NODE_WIFI_PHYS_BASE        0x83050000
#define MST_PORT_NODE_WIFI_SIZE             0x1000
#define MST_PORT_NODE_DSL_PHYS_BASE         0x83060000
#define MST_PORT_NODE_DSL_SIZE              0x1000
#define MST_PORT_NODE_DSLCPU_PHYS_BASE      0x83068000
#define MST_PORT_NODE_DSLCPU_SIZE           0x1000

#define UBUS4_PORT_NODE_ROUTING_ADDRESS_OFFSET 0x200
#define UBUS_CAPTURE_PORT_NODE_0_PHYS_BASE  0x8306c000
#define UBUS_CAPTURE_PORT_NODE_SIZE         0x1000

#define USBH_PHYS_BASE          0x8000c000
#define USBH_SIZE               0x3fff
#define USBH_OFFSET             0x0000
#define CFG_OFFSET              0x200
#define EHCI_OFFSET             0x300     /* USB host registers */
#define OHCI_OFFSET             0x400     /* USB host registers */
#define EHCI1_OFFSET            0x500     /* EHCI1 host registers */
#define OHCI1_OFFSET            0x600     /* OHCI1 host registers */
#define XHCI_OFFSET             0x1000     /* USB host registers */
#define XHCI_EC_OFFSET          0x1900    /* XHCI extended registers */

/* to support non-DT pltaform device add below defs */
#define USB_EHCI_PHYS_BASE      (USBH_PHYS_BASE+EHCI_OFFSET)
#define USB_OHCI_PHYS_BASE      (USBH_PHYS_BASE+OHCI_OFFSET)
#define USB_EHCI1_PHYS_BASE     (USBH_PHYS_BASE+EHCI1_OFFSET)
#define USB_OHCI1_PHYS_BASE     (USBH_PHYS_BASE+OHCI1_OFFSET)
#define USB_XHCI_PHYS_BASE      (USBH_PHYS_BASE+XHCI_OFFSET)

#define SAR_PHYS_BASE          0x80800000
#define SAR_SIZE               0x6800
#define SAR_OFFSET             0x00
#define SAR_IUDMA_OFFSET       0x6000

#define UBUS4_COHERENCY_PORT_PHYS_BASE      0x810A0000
#define UBUS4_COHERENCY_PORT_BASE_SIZE      0x1000
#define UBUS4_RANGE_CHK_SETUP_OFFSET        0x0
#define UBUS4_RANGE_CHK_CONFIG_OFFSET       0x310
#define UBUS4_COHERENCY_PORT_CONFIG_OFFSET  0x400

#define CCI500_PHYS_BASE       0x81100000
#define CCI500_SIZE            0x91000
#define CCI500_OFFSET          0x000

#define UBUS_SYS_MODULE_BASE        BCM_IO_MAP(UBUS_MAPPED_IDX, UBUS_MAPPED_PHYS_BASE, 0)
#define UBUS_SYS_MODULE_REGISTRATION_BASE	BCM_IO_MAP(UBUS_MAPPED_IDX, 	 UBUS_MAPPED_PHYS_BASE, UBUS_SYS_MODULE_REGISTRATION_OFFSET)
#define UBUS_MAPPED_BASE            BCM_IO_MAP(UBUS_MAPPED_IDX, UBUS_MAPPED_PHYS_BASE, SYS_CLK_CTRL_OFFSET)
#define PMC_BASE                    BCM_IO_MAP(PMC_IDX, PMC_PHYS_BASE, PMC_OFFSET)
#define PROC_MON_BASE               BCM_IO_MAP(PMC_IDX, PMC_PHYS_BASE, PROC_MON_OFFSET)
#define PMB_BASE                    BCM_IO_MAP(PMC_IDX, PMC_PHYS_BASE, PMB_OFFSET)
#define MEMC_BASE                   BCM_IO_MAP(MEMC_IDX, MEMC_PHYS_BASE, 0)
#define PERF_BASE                   BCM_IO_MAP(PERF_IDX, PERF_PHYS_BASE, 0)
#define TIMR_BASE                   BCM_IO_MAP(PERF_IDX, PERF_PHYS_BASE, TIMR_OFFSET)
#define WDTIMR0_BASE                BCM_IO_MAP(PERF_IDX, PERF_PHYS_BASE, WDTIMR0_OFFSET)
#define WDTIMR1_BASE                BCM_IO_MAP(PERF_IDX, PERF_PHYS_BASE, WDTIMR1_OFFSET)
#define GPIO_BASE                   BCM_IO_MAP(PERF_IDX, PERF_PHYS_BASE, GPIO_OFFSET)
#define AHB_CONTROL_BASE            BCM_IO_MAP(PERF_IDX, PERF_PHYS_BASE, AHB_CONTROL_OFFSET)
#define HS_UART_BASE                BCM_IO_MAP(PERF_IDX, PERF_PHYS_BASE, HS_UART_OFFSET)
#define PL081_DMA_BASE              BCM_IO_MAP(PERF_IDX, PERF_PHYS_BASE, PL081_DMA_OFFSET)
#define ARM_UART_BASE               BCM_IO_MAP(PERF_IDX, PERF_PHYS_BASE, ARM_UART_OFFSET)
#define LED_BASE                    BCM_IO_MAP(PERF_IDX, PERF_PHYS_BASE, LED_OFFSET)
#define SOTP_BASE                   BCM_IO_MAP(PERF_IDX, PERF_PHYS_BASE, SOTP_OFFSET)
#define RNG_BASE                    BCM_IO_MAP(PERF_IDX, PERF_PHYS_BASE, RNG_OFFSET)
#define JTAG_OTP_BASE               BCM_IO_MAP(PERF_IDX, PERF_PHYS_BASE, JTAG_OTP_OFFSET)
#define HSSPIM_BASE                 BCM_IO_MAP(PERF_IDX, PERF_PHYS_BASE, HSSPIM_OFFSET)
#define NAND_REG_BASE               BCM_IO_MAP(PERF_IDX, PERF_PHYS_BASE, NAND_REG_OFFSET)
#define NAND_CACHE_BASE             BCM_IO_MAP(PERF_IDX, PERF_PHYS_BASE, NAND_CACHE_OFFSET)
#define NAND_INTR_BASE              BCM_IO_MAP(PERF_IDX, PERF_PHYS_BASE, NAND_INTR_OFFSET)
#define MISC_BASE                   BCM_IO_MAP(PERF_IDX, PERF_PHYS_BASE, MISC_OFFSET)

#define PERF1_BASE                  BCM_IO_MAP(PERF1_IDX, PERF1_PHYS_BASE, 0)
#define TOP_CONTROL_BASE            BCM_IO_MAP(PERF1_IDX, PERF1_PHYS_BASE, TOP_CNTRL_OFFSET)

#define NANDFLASH_BASE              BCM_IO_MAP(NANDFLASH_IDX, NANDFLASH_PHYS_BASE, NANDFLASH_OFFSET)
#define BOOTLUT_BASE                BCM_IO_MAP(BOOTLUT_IDX, BOOTLUT_PHYS_BASE, 0)

#define USBH_BASE                   BCM_IO_MAP(USBH_IDX, USBH_PHYS_BASE, USBH_OFFSET)
#define USBH_CFG_BASE               BCM_IO_MAP(USBH_IDX, USBH_PHYS_BASE, CFG_OFFSET)
#define USBH_EHCI_BASE              BCM_IO_MAP(USBH_IDX, USBH_PHYS_BASE, EHCI_OFFSET)
#define USBH_OHCI_BASE              BCM_IO_MAP(USBH_IDX, USBH_PHYS_BASE, OHCI_OFFSET)
#define USBH_XHCI_BASE              BCM_IO_MAP(USBH_IDX, USBH_PHYS_BASE, XHCI_OFFSET)
/*TODO : fix the names of usb register base's to be same across all platforms */
#define USB_XHCI_BASE               USBH_XHCI_BASE

#define MST_PORT_NODE_PER_BASE      BCM_IO_MAP(MST_PORT_NODE_PER_IDX, MST_PORT_NODE_PER_PHYS_BASE, 0)
#define MST_PORT_NODE_USB_BASE      BCM_IO_MAP(MST_PORT_NODE_USB_IDX, MST_PORT_NODE_USB_PHYS_BASE, 0)
#define MST_PORT_NODE_CPU_BASE      BCM_IO_MAP(MST_PORT_NODE_CPU_IDX, MST_PORT_NODE_CPU_PHYS_BASE, 0)
#define MST_PORT_NODE_PMC_BASE      BCM_IO_MAP(MST_PORT_NODE_PMC_IDX, MST_PORT_NODE_PMC_PHYS_BASE, 0)
#define MST_PORT_NODE_PCIE0_BASE    BCM_IO_MAP(MST_PORT_NODE_PCIE0_IDX, MST_PORT_NODE_PCIE0_PHYS_BASE, 0)
#define MST_PORT_NODE_SWH_BASE      BCM_IO_MAP(MST_PORT_NODE_SWH_IDX, MST_PORT_NODE_SWH_PHYS_BASE, 0)
#define MST_PORT_NODE_WIFI_BASE     BCM_IO_MAP(MST_PORT_NODE_WIFI_IDX, MST_PORT_NODE_WIFI_PHYS_BASE, 0)
#define MST_PORT_NODE_DSL_BASE      BCM_IO_MAP(MST_PORT_NODE_DSL_IDX, MST_PORT_NODE_DSL_PHYS_BASE, 0)
#define MST_PORT_NODE_DSLCPU_BASE   BCM_IO_MAP(MST_PORT_NODE_DSLCPU_IDX, MST_PORT_NODE_DSLCPU_PHYS_BASE, 0)

#define BIUCFG_BASE                 BCM_IO_MAP(BIUCFG_IDX, BIUCFG_PHYS_BASE, BIUCFG_OFFSET)
#define UBUS_RANGE_CHK_SETUP_BASE   BCM_IO_MAP(UBUS4_COHERENCY_PORT_IDX,UBUS4_COHERENCY_PORT_PHYS_BASE, UBUS4_RANGE_CHK_SETUP_OFFSET)
#define UBUS_RANGE_CHK_CFG_BASE     BCM_IO_MAP(UBUS4_COHERENCY_PORT_IDX,UBUS4_COHERENCY_PORT_PHYS_BASE, UBUS4_RANGE_CHK_CONFIG_OFFSET)
#define UBUS_COHERENCY_PORT_CFG_BASE   	BCM_IO_MAP(UBUS4_COHERENCY_PORT_IDX,UBUS4_COHERENCY_PORT_PHYS_BASE, UBUS4_COHERENCY_PORT_CONFIG_OFFSET)
#define UBUS_CAPTURE_PORT_NODE_0_BASE   BCM_IO_MAP(UBUS_CAPTURE_PORT_NODE_0, UBUS_CAPTURE_PORT_NODE_0_PHYS_BASE, 0)

#define APM_BASE                    BCM_IO_MAP(PCM_IDX, PCM_PHYS_BASE, APM_CORE_OFFSET)
#define PCM_BASE                    BCM_IO_MAP(PCM_IDX, PCM_PHYS_BASE, PCM_CORE_OFFSET)
#define PCM_DMA_BASE                BCM_IO_MAP(PCM_IDX, PCM_PHYS_BASE, PCM_DMA_OFFSET)
#define PCM_BUS_BASE                BCM_IO_MAP(PCMBUS_IDX, PCMBUS_PHYS_BASE, PCMBUS_OFFSET)
#define CCI500_BASE                 BCM_IO_MAP(CCI500_IDX, CCI500_PHYS_BASE, CCI500_OFFSET)

#define UBUS_COHERENCY_PORT_CFG_LUT_BASE            UBUS_COHERENCY_PORT_CFG_BASE + 0
#define UBUS_COHERENCY_PORT_CFG_DEPTH_BASE          UBUS_COHERENCY_PORT_CFG_BASE + 0x80
#define UBUS_COHERENCY_PORT_CFG_CBS_BASE            UBUS_COHERENCY_PORT_CFG_BASE + 0x90
#define UBUS_COHERENCY_PORT_CFG_CIR_INCR_BASE       UBUS_COHERENCY_PORT_CFG_BASE + 0xB0
#define UBUS_COHERENCY_PORT_CFG_REF_COUNT_BASE      UBUS_COHERENCY_PORT_CFG_BASE + 0xc0

/* These block uses DT or not used by linux at all, no need to map for the legacy support */
#define GIC_BASE           BCM_IO_NOMAP(-1, GIC_PHYS_BASE, GIC_OFFSET)
#define GICC_BASE          BCM_IO_NOMAP(-1, GIC_PHYS_BASE, GICC_OFFSET)
#define GICD_BASE          BCM_IO_NOMAP(-1, GIC_PHYS_BASE, GICD_OFFSET)
#define SPIFLASH_BASE      BCM_IO_NOMAP(-1, SPIFLASH_PHYS_BASE, SPIFLASH_OFFSET)

#define SWITCH_CORE_BASE   BCM_IO_MAP(SWITCH_IDX, SWITCH_PHYS_BASE, SWITCH_CORE_OFFSET)
#define SWITCH_REG_BASE    BCM_IO_MAP(SWITCH_IDX, SWITCH_PHYS_BASE, SWITCH_REG_OFFSET)
#define SWITCH_MDIO_BASE   BCM_IO_MAP(SWITCH_IDX, SWITCH_PHYS_BASE, SWITCH_MDIO_OFFSET)
#define SWITCH_ACB_BASE    BCM_IO_MAP(SWITCH_IDX, SWITCH_PHYS_BASE, SWITCH_ACB_OFFSET)
#define SF2_ACB_PORT0_CONFIG_REG           (SWITCH_ACB_BASE + 0x0208UL)
    #define SF2_ACB_PORT_XOFF_EN            (1<<11)
#define SF2_ACB_CONTROL_QUE_MAP_0           (SWITCH_ACB_BASE + (0x0a28 - 0x0800))
#define SF2_ACB_CONTROL_QUE_MAP_1           (SWITCH_ACB_BASE + (0x0a2c - 0x0800))
#define SYSPORT_BASE       BCM_IO_MAP(SYSPORT_IDX, SYSPORT_PHYS_BASE, SYSPORT_OFFSET)
#define SYSPORT_SYSBUSCFG_BASE  BCM_IO_MAP(SYSPORT_IDX, SYSPORT_PHYS_BASE, SYSPORT_SYSBUSCFG_OFFSET)
#define SYSPORT_RXCHK_BASE      BCM_IO_MAP(SYSPORT_IDX, SYSPORT_PHYS_BASE, SYSPORT_RXCHK_OFFSET)
#define SYSPORT_RBUF_BASE       BCM_IO_MAP(SYSPORT_IDX, SYSPORT_PHYS_BASE, SYSPORT_RBUF_OFFSET)
#define SYSPORT_TBUF_BASE       BCM_IO_MAP(SYSPORT_IDX, SYSPORT_PHYS_BASE, SYSPORT_TBUF_OFFSET)
#define SYSPORT_MPD_BASE        BCM_IO_MAP(SYSPORT_IDX, SYSPORT_PHYS_BASE, SYSPORT_MPD_OFFSET)
#define SYSPORT_RDMA_BASE       BCM_IO_MAP(SYSPORT_IDX, SYSPORT_PHYS_BASE, SYSPORT_RDMA_OFFSET)
#define SYSPORT_TDMA_BASE       BCM_IO_MAP(SYSPORT_IDX, SYSPORT_PHYS_BASE, SYSPORT_TDMA_OFFSET)
#define SYSPORT_SPE_BASE        BCM_IO_MAP(SYSPORT_IDX, SYSPORT_PHYS_BASE, SYSPORT_SPE_OFFSET)
#define SYSPORT_GIB_BASE        BCM_IO_MAP(SYSPORT_IDX, SYSPORT_PHYS_BASE, SYSPORT_GIB_OFFSET)
#define SYSPORT_INTRL2_BASE     BCM_IO_MAP(SYSPORT_IDX, SYSPORT_PHYS_BASE, SYSPORT_INTRL2_OFFSET)
#define SYSPORT_INTC_BASE       BCM_IO_MAP(SYSPORT_IDX, SYSPORT_PHYS_BASE, SYSPORT_INTC_OFFSET)
#define SYSPORT_INT_MISC_RX_BASE BCM_IO_MAP(SYSPORT_IDX, SYSPORT_PHYS_BASE, SYSPORT_INTRL2_MISC_RX_OFFSET)
#define SYSPORT_INT_MISC_TX_BASE BCM_IO_MAP(SYSPORT_IDX, SYSPORT_PHYS_BASE, SYSPORT_INTRL2_MISC_TX_OFFSET)

#define DSLPHY_BASE        BCM_IO_MAP(DSLPHY_IDX, DSLPHY_PHYS_BASE, DSLPHY_OFFSET)
#define DSLLMEM_BASE       BCM_IO_MAP(DSLLMEM_IDX, DSLLMEM_PHYS_BASE, DSLLMEM_OFFSET)

#define SAR_BASE           BCM_IO_MAP(SAR_IDX, SAR_PHYS_BASE, SAR_OFFSET)
#define SAR_IUDMA_BASE     BCM_IO_MAP(SAR_IDX, SAR_PHYS_BASE, SAR_IUDMA_OFFSET)

/* Definition to satisfy legacy code usage */
#define SWITCH_BASE                 (SWITCH_CORE_BASE)
#define SWITCH_DIRECT_DATA_WR_REG   (SWITCH_REG_BASE + 0x00008UL)
#define SWITCH_DIRECT_DATA_RD_REG   (SWITCH_REG_BASE + 0x0000cUL)

#ifndef __ASSEMBLER__
#ifdef __BOARD_DRV_ARMV7__
BCM_IO_BLOCKS bcm_io_blocks[] =  
{
    {MEMC_IDX, MEMC_SIZE, MEMC_PHYS_BASE},                                                                               
    {PMC_IDX, PMC_SIZE, PMC_PHYS_BASE},                                                                                     
    {SWITCH_IDX, SWITCH_SIZE, SWITCH_PHYS_BASE},
    {PERF_IDX, PERF_SIZE, PERF_PHYS_BASE},                                                                                 
    {PERF1_IDX, PERF1_SIZE, PERF1_PHYS_BASE},                                                                             
    {NANDFLASH_IDX, NANDFLASH_SIZE, NANDFLASH_PHYS_BASE},                                                             
    {PCM_IDX, PCM_SIZE, PCM_PHYS_BASE},
    {PCMBUS_IDX, PCMBUS_SIZE, PCMBUS_PHYS_BASE},
    {USBH_IDX, USBH_SIZE, USBH_PHYS_BASE},
    {MST_PORT_NODE_PER_IDX, MST_PORT_NODE_PER_SIZE, MST_PORT_NODE_PER_PHYS_BASE},
    {MST_PORT_NODE_USB_IDX, MST_PORT_NODE_USB_SIZE, MST_PORT_NODE_USB_PHYS_BASE},
    {MST_PORT_NODE_CPU_IDX, MST_PORT_NODE_CPU_SIZE, MST_PORT_NODE_CPU_PHYS_BASE},
    {MST_PORT_NODE_PMC_IDX, MST_PORT_NODE_PMC_SIZE, MST_PORT_NODE_PMC_PHYS_BASE},
    {MST_PORT_NODE_PCIE0_IDX, MST_PORT_NODE_PCIE0_SIZE, MST_PORT_NODE_PCIE0_PHYS_BASE},
    {MST_PORT_NODE_SWH_IDX, MST_PORT_NODE_SWH_SIZE, MST_PORT_NODE_SWH_PHYS_BASE},
    {MST_PORT_NODE_WIFI_IDX, MST_PORT_NODE_WIFI_SIZE, MST_PORT_NODE_WIFI_PHYS_BASE},
    {MST_PORT_NODE_DSL_IDX, MST_PORT_NODE_DSL_SIZE, MST_PORT_NODE_DSL_PHYS_BASE},
    {MST_PORT_NODE_DSLCPU_IDX, MST_PORT_NODE_DSLCPU_SIZE, MST_PORT_NODE_DSLCPU_PHYS_BASE},
    {UBUS4_COHERENCY_PORT_IDX, UBUS4_COHERENCY_PORT_BASE_SIZE, UBUS4_COHERENCY_PORT_PHYS_BASE},     
    {BIUCFG_IDX, BIUCFG_SIZE, BIUCFG_PHYS_BASE},                                                                   
    {BOOTLUT_IDX, BOOTLUT_SIZE, BOOTLUT_PHYS_BASE},                                                                
    {UBUS_MAPPED_IDX, UBUS_MAPPED_SIZE, UBUS_MAPPED_PHYS_BASE},
    {UBUS_CAPTURE_PORT_NODE_0, UBUS_CAPTURE_PORT_NODE_SIZE, UBUS_CAPTURE_PORT_NODE_0_PHYS_BASE},
    {SYSPORT_IDX, SYSPORT_SIZE, SYSPORT_PHYS_BASE},
    {DSLPHY_IDX, DSLPHY_SIZE, DSLPHY_PHYS_BASE},
    {DSLLMEM_IDX, DSLLMEM_SIZE, DSLLMEM_PHYS_BASE},
    {SAR_IDX, SAR_SIZE, SAR_PHYS_BASE},
    {CCI500_IDX, CCI500_SIZE, CCI500_PHYS_BASE},
};

unsigned long bcm_io_block_address[LAST_IDX];
#else
extern BCM_IO_BLOCKS bcm_io_blocks[];
extern unsigned long bcm_io_block_address[];
#endif

/*
 * High Speed Uart Control
 */
typedef struct HsUartCtrlRegs {
    uint32      reserved[5];
    uint32      ptu_hc;         /* 14 */
#define HS_UART_PTU_HC_DATA (1 << 1)          
    uint32      reserved1;
    uint32      uart_data;
    uint32      reserved2[25];  /* 20 */
    uint32      uart_int_stat;  /* 84 */
    uint32      reserved3[8];   /* 88 */
    uint32      uart_int_en;    /* a8 */
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
    uint32      reserved4[53];  /* ac */
    uint32      dhbr;           /* 180 */
    uint32      dlbr;           
#define HS_UART_DHBR_3000K    0x00000001 /* Target is 16 clocks/baud(bit), Based on 50Mhz Uart clk */
#define HS_UART_DLBR_3000K    0x000000ff /* Target is 16 clocks/baud(bit), Based on 50Mhz Uart clk */
#define HS_UART_DHBR_115200   0x00000011 /* Target is 16 clocks/baud(bit), Based on 50Mhz Uart clk */
#define HS_UART_DLBR_115200   0x000000e5 /* Target is 16 clocks/baud(bit), Based on 50Mhz Uart clk */
    uint32      ab0;           
    uint32      reserved5;
    uint32      FCR;            /* 190 */     
#define HS_UART_FCR_AUTOLOAD_MODE (1 << 8)
#define HS_UART_FCR_AB_MODE       (1 << 7)
#define HS_UART_FCR_AUTO_RTS_OE   (1 << 6)
#define HS_UART_FCR_AUTO_TX_OE    (1 << 5)
#define HS_UART_FCR_AUTOBAUD      (1 << 4)
#define HS_UART_FCR_TX_PACKET     (1 << 3)
#define HS_UART_FCR_SLIP_RESYNC   (1 << 2)
    uint32      ab1;           
    uint32      reserved6;
    uint32      LCR;            /* 19c */
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
    uint32      MCR;           
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
    uint32      LSR;           
#define HS_UART_LSR_TX_HALT        (1 << 5)    
#define HS_UART_LSR_TX_PACKET_RDY  (1 << 4)    
#define HS_UART_LSR_TX_IDLE        (1 << 3)    
#define HS_UART_LSR_TX_DATA_AVAIL  (1 << 2) /* 1 - Data in TX FIFO, 0 - TX FIFO empty */
#define HS_UART_LSR_RX_FULL        (1 << 1) /* 1 - RX FIFO full */     
#define HS_UART_LSR_RX_OVERFLOW    (1 << 0) /* 1 - RX FIFO Overflow */       
    uint32      MSR;           
#define HS_UART_MSR_RX_IN     (1 << 2)
#define HS_UART_MSR_RTS_STAT  (1 << 1)
#define HS_UART_MSR_CTS_STAT  (1 << 0)
    uint32      RFL;            /* 1ac */          
    uint32      TFL;           
    uint32      RFC;           
#define HS_UART_RFC_1039BYTES_FC_DATA (1039)
#define HS_UART_RFC_NO_FC_DATA (HS_UART_RFC_1039BYTES_FC_DATA) // or should be LLI_BUFF_SIZE
    uint32      ESC;            /* 1b8 */
#define HS_UART_ESC_SLIP_DATA (0xDB)
#define HS_UART_ESC_NO_SLIP_DATA (0xDA)
    uint32      reserved7[3];
    uint32      HOPKT_LEN;      /* 1c8 */   
    uint32      HIPKT_LEN;      
    uint32      HO_DMA_CTL;      
    uint32      HI_DMA_CTL;      
    uint32      HO_BSIZE;      
    uint32      HI_BSIZE;      
#define HS_UART_DMA_CTL_BURSTMODE_EN   (1 << 3)
#define HS_UART_DMA_CTL_AFMODE_EN      (1 << 2)
#define HS_UART_DMA_CTL_FASTMODE_EN    (1 << 1)
#define HS_UART_DMA_CTL_DMA_EN         (1 << 0)
#define HS_UART_HO_BSIZE_DATA (0x3)
#define HS_UART_HI_BSIZE_DATA (0x3)
} HsUartCtrlRegs;

#define HS_UART ((volatile HsUartCtrlRegs * const) HS_UART_BASE)

typedef struct PL081Dma{
    uint32      dmacintstat;
    uint32      dmacinttcstat;
    uint32      dmacinttcclr;
    uint32      dmacinterrstat;
    uint32      dmacinterrclr;      /* 10 */
    uint32      dmacwintc;
    uint32      dmacwinterr;
    uint32      dmacenbldchns;
    uint32      dmacsoftbreq;       /* 20 */
    uint32      dmacsoftsreq;
    uint32      dmacsofttlbreq;
    uint32      dmacsofttlsreq;
    uint32      dmacconfig;
    uint32      dmacsync;           /* 34 */
    uint32      dmacreserved[50];
    uint32      dmacc0srcaddr;      /* 100 */
    uint32      dmacc0destaddr;
    uint32      dmacc0llireg;
    uint32      dmacc0control;
    uint32      dmacc0config;
    uint32      unused2[3];         /* 0x114 - 0x11f */
    uint32      dmacc1srcaddr;
    uint32      dmacc1destaddr;
    uint32      dmacc1llireg;
    uint32      dmacc1control;
    uint32      dmacc1config;
    uint32      dmacreserved1[51];  /* 134 */
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
    uint32      dmactcr;            /* 200 */
    uint32      dmactop1;
    uint32      dmactop2;
    uint32      dmactop3;
    uint32      dmacreserved2[52];  /* 210 */
    uint32      dmacperiphid0;      /* 2e0 */
    uint32      dmacperiphid1;
    uint32      dmacperiphid2;
    uint32      dmacperiphid3;
    uint32      dmacpcellid0;
    uint32      dmacpcellid1;
    uint32      dmacpcellid2;
    uint32      dmacpcellid3;
} PL081Dma;

#define PLDMA ((volatile PL081Dma * const) PL081_DMA_BASE)


typedef struct TopControl {
   uint32      reserved_00;        // 0x0
   uint32      Rgmii1PadCtl;       // 0x4
#define MISC_XMII_PAD_MODEHV                    (1 << 6)
#define MISC_XMII_PAD_IND                       (1 << 5) 
#define MISC_XMII_PAD_SEL_GMII                  (1 << 4)
#define MISC_XMII_PAD_AMP_EN                    (1 << 3)
#define MISC_XMII_PAD_DRIVE_STRENGTH_SHIFT      0
#define MISC_XMII_PAD_DRIVE_STRENGTH_MASK       (0x7<<MISC_XMII_PAD_DRIVE_STRENGTH_SHIFT)
   uint32      reserved_08;        // 0x8
   uint32      reserved_0c;        // 0xc
   uint32      MiiPullCtl;         // 0x10
   uint32      Rgmii1PullCtl;      // 0x14
   uint32      Rgmii2PullCtl;      // 0x18
   uint32      Rgmii3PullCtl;      // 0x1c
   uint32      SgmiiFiberDetect;   // 0x20
   uint32      DgSensePadCtl;      // 0x24
#define DG_CTRL_SHIFT   4
#define DG_EN_SHIFT     3
#define DG_TRIM_SHIFT   0
   uint32      TpDirOverride0;     // 0x28
   uint32      TpDirOverride1;     // 0x2c
   uint32      RescalReadData0;    // 0x30
   uint32      RescalReadData1;    // 0x34
   uint32      reserved0;          // 0x38 
   uint32      ResetStatus;        // 0x3c 
#define PCIE_RESET_STATUS       0x10000000
#define SW_RESET_STATUS         0x20000000
#define HW_RESET_STATUS         0x40000000
#define POR_RESET_STATUS        0x80000000
#define RESET_STATUS_MASK       0xF0000000
   uint32      TodSync;            // 0x40
   uint32      Pm0Pm1RescalSts;    // 0x44
   uint32      Pm2Pm3RescalSts;    // 0x48
   uint32      SgbRescalSts;       // 0x4c
   uint32      Pm0Pm1RescalCfg;    // 0x50
   uint32      Pm2Pm3RescalCfg;    // 0x54
   uint32      SgbRescalCfg;       // 0x58
   uint32      Pm0Pm1TmonCfg;      // 0x5c
   uint32      Pm2Pm3TmonCfg;      // 0x60
   uint32      SgbTmonCfg;         // 0x64
   uint32      MdioMasterSelect;   // 0x68
   uint32      SwControlReg;       // 0x6c
   uint32      Reg2P5VDectCtrl;    // 0x70
   uint32      OrionIntPending;    // 0x74
   uint32      DslClockSample;     // 0x78
   uint32      LdoCtl;             // 0x7c
#define LDO_VREG_CTRL_EN           0x400000
#define LDO_VREG_CTRL_SHIFT        1
#define LDO_VREG_CTRL_MASK         (0x1fffff << LDO_VREG_CTRL_SHIFT)
#define LDO_VREG_CTRL_TRIM_SHIFT   2
#define LDO_VREG_CTRL_TRIM_MASK    (0xf << LDO_VREG_CTRL_TRIM_SHIFT)
#define LDO_VREG_CTRL_SELECT_SHIFT 6
#define LDO_VREG_CTRL_SELECT_MASK  (0xf << LDO_VREG_CTRL_SELECT_SHIFT)
#define LDO_VREG_CTRL_PWRDN        0x1
} TopControl;

#define TOPCTRL ((volatile TopControl * const) TOP_CONTROL_BASE)
#define TOP     ((volatile TopControl * const) TOP_CONTROL_BASE)

/*
** Peripheral Controller
*/

typedef struct PerfControl { /* GenInt */
     uint32        RevID;             /* (00) word 0 */
#define CHIP_ID_SHIFT   12
#define CHIP_ID_MASK    (0xfffff << CHIP_ID_SHIFT)
#define REV_ID_MASK     0xff

   uint32 Ext1IrqCtrl;         /* 0x04 */
#define EI_CLEAR_SHFT   0
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
   uint32 ExtIrqCtrl;        /* 0x20 */
   uint32 ExtIrqStatus;      /* 0x24 */
   uint32 ExtIrqSet;         /* 0x28 */
   uint32 ExtIrqClear;       /* 0x2c */
   uint32 ExtIrqMaskStatus;  /* 0x30 */
   uint32 ExtIrqMaskSet;     /* 0x34 */
   uint32 ExtIrqMaskClear;   /* 0x38 */
   uint32 reserved0[2];      /* 0x3c */
   uint32 ExtIrqMuxSel0;      /* 0x44 */
#define EXT_IRQ_SLOT_SIZE             16
#define EXT_IRQ_MUX_SEL0_SHIFT        4
#define EXT_IRQ_MUX_SEL0_MASK         0xf
   uint32 ExtIrqMuxSel1;      /* 0x48 */
#define EXT_IRQ_MUX_SEL1_SHIFT        4
#define EXT_IRQ_MUX_SEL1_MASK         0xf
   uint32 IrqPeriphStatus;    /* 0x4c */
   uint32 IrqPeriphMask;      /* 0x50 */
   uint32 reserved[8];        /* 0x4c */  
   uint32 DMAIrqStatus;       /* 0x6c */
   uint32 DMAIrqSet;          /* 0x70 */
   uint32 DMAIrqClear;        /* 0x74 */
   uint32 DMAIrqMaskStatus;   /* 0x78 */
   uint32 DMAIrqMaskSet;      /* 0x7c */
   uint32 DMAIrqMaskClear;    /* 0x80 */
} PerfControl;

#define PERF ((volatile PerfControl * const) PERF_BASE)

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

#define PCIE_RESET_STATUS       0x10000000
#define SW_RESET_STATUS         0x20000000
#define HW_RESET_STATUS         0x40000000
#define POR_RESET_STATUS        0x80000000
#define RESET_STATUS_MASK       0xF0000000
    uint32        ResetReason;
#define SW_INI_RESET            0x00000001
    uint32        spare[3];
} Timer;

typedef struct WDTimer {
    uint32        WatchDogDefCount;/* Write 0xff00 0x00ff to Start timer
     * Write 0xee00 0x00ee to Stop and re-load default count
     *      *      * Read from this register returns current watch dog count
     *           *           */
    uint32        WatchDogCtl;

    /* Number of 50-MHz ticks for WD Reset pulse to last */
    uint32        WDResetCount;

#define SOFT_RESET              0x00000001
    uint32        WDTimerCtl;

    uint32        WDAccessCtl;
} WDTimer;

#define TIMER ((volatile Timer * const) TIMR_BASE)
#define WDTIMER0 ((volatile WDTimer * const) WDTIMR0_BASE)
#define WDTIMER1 ((volatile WDTimer * const) WDTIMR1_BASE)

/*
 * ARM UART Peripheral
 */

typedef struct UartArm {
   uint32 dr;                    /* 0x00 */
   uint32 rsr;                   /* 0x04 */
   uint32 rsrvd1[4];             /* 0x08 */
   uint32 fr;                    /* 0x18 */
#define FR_TXFE           0x80
#define FR_RXFF           0x40
#define FR_TXFF           0x20
#define FR_RXFE           0x10
#define FR_BUSY           0x04
   uint32 rsrvd2[1];	         /* 0x1c */
   uint32 ilpr;                  /* 0x20 */
   uint32 ibrd;                  /* 0x24 */
   uint32 fbrd;                  /* 0x28 */
   uint32 lcr_h;                 /* 0x2c */
#define LCR_H_SPS         0x80
#define LCR_H_SPS_SHIFT   7
#define LCR_H_WLEN_MASK   0x60
#define LCR_H_WLEN_SHIFT  0x05
#define LCR_H_WLEN_8BIT   0x60
#define LCR_H_WLEN_7BIT   0x40
#define LCR_H_WLEN_6BIT   0x20
#define LCR_H_WLEN_5BIT   0x00
#define LCR_H_FEN         0x10
#define LCR_H_FEN_SHIFT   4
#define LCR_H_STP2        0x08
#define LCR_H_STP2_SHIFT  3
#define LCR_H_EPS         0x04
#define LCR_H_EPS_SHIFT   0x02
#define LCR_H_PEN         0x02
#define LCR_H_PEN_SHIFT   0x01
#define LCR_H_BRK         0x01
#define LCR_H_BRK_SHIFT   0x00
   uint32 cr;                    /* 0x30 */
#define CR_CTSE           0x8000
#define CR_RTSE           0x4000
#define CR_OUT2           0x2000
#define CR_OUT1           0x1000
#define CR_RTS            0x800
#define CR_DTR            0x400
#define CR_RXE            0x200
#define CR_RXE_SHIFT      9
#define CR_TXE            0x100
#define CR_TXE_SHIFT      8 
#define CR_LBE            0x80
#define CR_RSV            0x78
#define CR_SIRLP          0x4
#define CR_SIRE           0x2
#define CR_EN             0x1
#define CR_EN_SHIFT       0
   uint32 ifls;                 /* 0x34 */
   uint32 imsc;                 /* 0x38 */
   uint32 ris;                  /* 0x3c */
   uint32 mis;                  /* 0x40 */
   uint32 icr;                  /* 0x44 */
   uint32 dmacr;                /* 0x48 */
   uint32 rsrvd3[13];           /* 0x4c */
   uint32 tcr;                  /* 0x80 */
   uint32 itip;                 /* 0x84 */
   uint32 itop;                 /* 0x88 */
   uint32 ttdr;                 /* 0x8c */
} UartArm;

#define ARM_UART ((volatile UartArm * const) ARM_UART_BASE)


/*
 * Gpio Controller
 */
typedef struct GpioControl {
        uint32 GPIODir[8];             /* 0x00-0x1c */
        uint32 GPIOio[8];              /* 0x20-0x3c */
        uint32 PadCtrl;                 /* 0x40 */
        uint32 SpiSlaveCfg;             /* 0x44 */
        uint32 TestControl;             /* 0x48 */
        uint32 TestPortBlockEnMSB;      /* 0x4c */
        uint32 TestPortBlockEnLSB;      /* 0x50 */
        uint32 TestPortBlockDataMSB;    /* 0x54 */
        uint32 TestPortBlockDataLSB;    /* 0x58 */
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
       uint32 TestPortCmd;             /* 0x5c */
#define LOAD_MUX_REG_CMD        0x21
#define LOAD_PAD_CTRL_CMD       0x22
        uint32 DiagReadBack;            /* 0x60 */
        uint32 DiagReadBackHi;          /* 0x64 */
        uint32 GeneralPurpose;          /* 0x68 */
        uint32 spare[3];
} GpioControl;

#define GPIO ((volatile GpioControl * const) GPIO_BASE)

/* Number to mask conversion macro used for GPIODir and GPIOio */
#define GPIO_NUM_MAX                88 /* accoring to pinmuxing table */  
#define GPIO_NUM_TO_ARRAY_IDX(X)	(((X & BP_GPIO_NUM_MASK) < GPIO_NUM_MAX) ? (((X & BP_GPIO_NUM_MASK) >> 5) & 0x0f) : (0))
#define GPIO_NUM_TO_MASK(X)         (((X & BP_GPIO_NUM_MASK) < GPIO_NUM_MAX) ? (1 << ((X & BP_GPIO_NUM_MASK) & 0x1f)) : (0))
#define GPIO_NUM_TO_ARRAY_SHIFT(X)  (((X) & BP_GPIO_NUM_MASK) & 0x1f)

/*
** Misc Register Set Definitions.
*/

typedef struct Misc {
    uint32  miscStrapBus;                       /* 0x00 */
#define MISC_STRAP_BUS_BOOT_SEL_SHIFT           0
#define MISC_STRAP_BUS_BOOT_SEL_MASK            (0x38 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_SPI_NOR             (0x38 << MISC_STRAP_BUS_BOOT_SEL_SHIFT)
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
#define MISC_STRAP_BUS_CPU_SLOW_FREQ_SHIFT      9
#define MISC_STRAP_BUS_CPU_SLOW_FREQ            (0x1 << MISC_STRAP_BUS_CPU_SLOW_FREQ_SHIFT)
#define MISC_STRAP_BUS_BOOTROM_BOOT_N           (0x1 << 11)
#define MISC_STRAP_BUS_B53_NO_BOOT              (0x1 << 12) /* 1-PMC boot A53, 0-A53 boots at POR */

    uint32  miscStrapOverride;                  /* 0x04 */
    uint32  miscMaskUBUSErr;                    /* 0x08 */
    uint32  miscPeriphCtrl;                     /* 0x0c */
    uint32  miscSPImasterCtrl;                  /* 0x10 */
    uint32  miscDierevid;                       /* 0x14 */
    uint32  miscPeriphMiscCtrl;                 /* 0x18 */
    uint32  miscPeriphMiscStat;                 /* 0x1c */
    uint32  miscSoftResetB;                     /* 0x20 */
    uint32  reserved0;                          /* 0x24 */
    uint32  miscSWdebugNW[2];                   /* 0x28 */
    uint32  miscWDresetCtrl;                    /* 0x30 */
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
** High-Speed SPI Controller
*/

#define __mask(end, start)      (((1 << ((end - start) + 1)) - 1) << start)
typedef struct HsSpiControl {

  uint32    hs_spiGlobalCtrl;       // 0x0000
#define HS_SPI_MOSI_IDLE            (1 << 18)
#define HS_SPI_CLK_POLARITY         (1 << 17)
#define HS_SPI_CLK_GATE_SSOFF       (1 << 16)
#define HS_SPI_PLL_CLK_CTRL         (8)
#define HS_SPI_PLL_CLK_CTRL_MASK    __mask(15, HS_SPI_PLL_CLK_CTRL)
#define HS_SPI_SS_POLARITY          (0)
#define HS_SPI_SS_POLARITY_MASK     __mask(7, HS_SPI_SS_POLARITY)

  uint32    hs_spiExtTrigCtrl;      // 0x0004
#define HS_SPI_TRIG_RAW_STATE       (24)
#define HS_SPI_TRIG_RAW_STATE_MASK  __mask(31, HS_SPI_TRIG_RAW_STATE)
#define HS_SPI_TRIG_LATCHED         (16)
#define HS_SPI_TRIG_LATCHED_MASK    __mask(23, HS_SPI_TRIG_LATCHED)
#define HS_SPI_TRIG_SENSE           (8)
#define HS_SPI_TRIG_SENSE_MASK      __mask(15, HS_SPI_TRIG_SENSE)
#define HS_SPI_TRIG_TYPE            (0)
#define HS_SPI_TRIG_TYPE_MASK       __mask(7, HS_SPI_TRIG_TYPE)
#define HS_SPI_TRIG_TYPE_EDGE       (0)
#define HS_SPI_TRIG_TYPE_LEVEL      (1)

  uint32    hs_spiIntStatus;        // 0x0008
#define HS_SPI_IRQ_PING1_USER       (28)
#define HS_SPI_IRQ_PING1_USER_MASK  __mask(31, HS_SPI_IRQ_PING1_USER)
#define HS_SPI_IRQ_PING0_USER       (24)
#define HS_SPI_IRQ_PING0_USER_MASK  __mask(27, HS_SPI_IRQ_PING0_USER)

#define HS_SPI_IRQ_PING1_CTRL_INV   (1 << 12)
#define HS_SPI_IRQ_PING1_POLL_TOUT  (1 << 11)
#define HS_SPI_IRQ_PING1_TX_UNDER   (1 << 10)
#define HS_SPI_IRQ_PING1_RX_OVER    (1 << 9)
#define HS_SPI_IRQ_PING1_CMD_DONE   (1 << 8)

#define HS_SPI_IRQ_PING0_CTRL_INV   (1 << 4)
#define HS_SPI_IRQ_PING0_POLL_TOUT  (1 << 3)
#define HS_SPI_IRQ_PING0_TX_UNDER   (1 << 2)
#define HS_SPI_IRQ_PING0_RX_OVER    (1 << 1)
#define HS_SPI_IRQ_PING0_CMD_DONE   (1 << 0)

  uint32    hs_spiIntStatusMasked;  // 0x000C
#define HS_SPI_IRQSM__PING1_USER    (28)
#define HS_SPI_IRQSM__PING1_USER_MASK   __mask(31, HS_SPI_IRQSM__PING1_USER)
#define HS_SPI_IRQSM__PING0_USER    (24)
#define HS_SPI_IRQSM__PING0_USER_MASK   __mask(27, HS_SPI_IRQSM__PING0_USER)

#define HS_SPI_IRQSM__PING1_CTRL_INV    (1 << 12)
#define HS_SPI_IRQSM__PING1_POLL_TOUT   (1 << 11)
#define HS_SPI_IRQSM__PING1_TX_UNDER    (1 << 10)
#define HS_SPI_IRQSM__PING1_RX_OVER     (1 << 9)
#define HS_SPI_IRQSM__PING1_CMD_DONE    (1 << 8)

#define HS_SPI_IRQSM__PING0_CTRL_INV    (1 << 4)
#define HS_SPI_IRQSM__PING0_POLL_TOUT   (1 << 3)
#define HS_SPI_IRQSM__PING0_TX_UNDER    (1 << 2)
#define HS_SPI_IRQSM__PING0_RX_OVER     (1 << 1)
#define HS_SPI_IRQSM__PING0_CMD_DONE    (1 << 0)

  uint32    hs_spiIntMask;              // 0x0010
#define HS_SPI_IRQM_PING1_USER          (28)
#define HS_SPI_IRQM_PING1_USER_MASK __mask(31, HS_SPI_IRQM_PING1_USER)
#define HS_SPI_IRQM_PING0_USER          (24)
#define HS_SPI_IRQM_PING0_USER_MASK __mask(27, HS_SPI_IRQM_PING0_USER)

#define HS_SPI_IRQM_PING1_CTRL_INV      (1 << 12)
#define HS_SPI_IRQM_PING1_POLL_TOUT     (1 << 11)
#define HS_SPI_IRQM_PING1_TX_UNDER      (1 << 10)
#define HS_SPI_IRQM_PING1_RX_OVER       (1 << 9)
#define HS_SPI_IRQM_PING1_CMD_DONE      (1 << 8)
                                        
#define HS_SPI_IRQM_PING0_CTRL_INV      (1 << 4)
#define HS_SPI_IRQM_PING0_POLL_TOUT     (1 << 3)
#define HS_SPI_IRQM_PING0_TX_UNDER      (1 << 2)
#define HS_SPI_IRQM_PING0_RX_OVER       (1 << 1)
#define HS_SPI_IRQM_PING0_CMD_DONE      (1 << 0)

#define HS_SPI_INTR_CLEAR_ALL           (0xFF001F1F)

  uint32    hs_spiFlashCtrl;            // 0x0014
#define HS_SPI_FCTRL_MB_ENABLE          (23)
#define HS_SPI_FCTRL_SS_NUM             (20)
#define HS_SPI_FCTRL_SS_NUM_MASK        __mask(22, HS_SPI_FCTRL_SS_NUM)
#define HS_SPI_FCTRL_PROFILE_NUM        (16)
#define HS_SPI_FCTRL_PROFILE_NUM_MASK   __mask(18, HS_SPI_FCTRL_PROFILE_NUM)
#define HS_SPI_FCTRL_DUMMY_BYTES        (10)
#define HS_SPI_FCTRL_DUMMY_BYTES_MASK   __mask(11, HS_SPI_FCTRL_DUMMY_BYTES)
#define HS_SPI_FCTRL_ADDR_BYTES         (8)
#define HS_SPI_FCTRL_ADDR_BYTES_MASK    __mask(9, HS_SPI_FCTRL_ADDR_BYTES)
#define HS_SPI_FCTRL_ADDR_BYTES_2       (0)
#define HS_SPI_FCTRL_ADDR_BYTES_3       (1)
#define HS_SPI_FCTRL_ADDR_BYTES_4       (2)
#define HS_SPI_FCTRL_READ_OPCODE        (0)
#define HS_SPI_FCTRL_READ_OPCODE_MASK   __mask(7, HS_SPI_FCTRL_READ_OPCODE)

  uint32    hs_spiFlashAddrBase;        // 0x0018

} HsSpiControl;

typedef struct HsSpiPingPong {

    uint32 command;
#define HS_SPI_SS_NUM (12)
#define ZSI_SPI_DEV_ID 7 // SS_N[7] connected to APM/PCM block for use by MSIF/ZDS interfaces
#define HS_SPI_PROFILE_NUM (8)
#define HS_SPI_TRIGGER_NUM (4)
#define HS_SPI_COMMAND_VALUE (0)
    #define HS_SPI_COMMAND_NOOP (0)
    #define HS_SPI_COMMAND_START_NOW (1)
    #define HS_SPI_COMMAND_START_TRIGGER (2)
    #define HS_SPI_COMMAND_HALT (3)
    #define HS_SPI_COMMAND_FLUSH (4)

    uint32 status;
#define HS_SPI_ERROR_BYTE_OFFSET (16)
#define HS_SPI_WAIT_FOR_TRIGGER (2)
#define HS_SPI_SOURCE_BUSY (1)
#define HS_SPI_SOURCE_GNT (0)

    uint32 fifo_status;
    uint32 control;
    uint32 PingPongReserved[12];
} HsSpiPingPong;

typedef struct HsSpiProfile {

    uint32 clk_ctrl;
#define HS_SPI_ACCUM_RST_ON_LOOP    (15)
#define HS_SPI_SPI_CLK_2X_SEL       (14)
#define HS_SPI_FREQ_CTRL_WORD       (0)

    uint32 signal_ctrl;
#define	HS_SPI_ASYNC_INPUT_PATH     (1 << 16)
#define	HS_SPI_LAUNCH_RISING        (1 << 13)
#define	HS_SPI_LATCH_RISING         (1 << 12)

    uint32 mode_ctrl;
#define HS_SPI_PREPENDBYTE_CNT      (24)
#define HS_SPI_MODE_ONE_WIRE        (20)
#define HS_SPI_MULTIDATA_WR_SIZE    (18)
#define HS_SPI_MULTIDATA_RD_SIZE    (16)
#define HS_SPI_MULTIDATA_WR_STRT    (12)
#define HS_SPI_MULTIDATA_RD_STRT    (8)
#define HS_SPI_FILLBYTE             (0)

    uint32 polling_config;
    uint32 polling_and_mask;
    uint32 polling_compare;
    uint32 polling_timeout;
    uint32 reserved;

} HsSpiProfile;

#define HS_SPI_OP_CODE 13
    #define HS_SPI_OP_SLEEP         (0)
    #define HS_SPI_OP_READ_WRITE    (1)
    #define HS_SPI_OP_WRITE         (2)
    #define HS_SPI_OP_READ          (3)
    #define HS_SPI_OP_SETIRQ        (4)

#define HS_SPI ((volatile HsSpiControl * const) HSSPIM_BASE)
#define HS_SPI_PINGPONG0 ((volatile HsSpiPingPong * const) (HSSPIM_BASE+0x80))
#define HS_SPI_PINGPONG1 ((volatile HsSpiPingPong * const) (HSSPIM_BASE+0xc0))
#define HS_SPI_PROFILES  ((volatile HsSpiProfile * const) (HSSPIM_BASE+0x100))
#define HS_SPI_FIFO0     ((volatile uint8 * const) (HSSPIM_BASE+0x200))
#define HS_SPI_FIFO1     ((volatile uint8 * const) (HSSPIM_BASE+0x400))


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
 * Power Management Control
 */
typedef struct PmcCtrlReg {
    uint32 gpTmr0Ctl;           /* 0x018 */
    uint32 gpTmr0Cnt;           /* 0x01c */
    uint32 gpTmr1Ctl;           /* 0x020 */
    uint32 gpTmr1Cnt;           /* 0x024 */
    uint32 hostMboxIn;          /* 0x028 */
    uint32 hostMboxOut;         /* 0x02c */
    uint32 reserved[4];         /* 0x030 */
    uint32 dmaCtrl;             /* 0x040 */
    uint32 dmaStatus;           /* 0x044 */
    uint32 dma0_3FifoStatus;    /* 0x048 */
    uint32 reserved1[4];	    /* 0x04c */
    uint32 diagControl;         /* 0x05c */
    uint32 diagHigh;            /* 0x060 */
    uint32 diagLow;             /* 0x064 */
    uint32 reserved8;           /* 0x068 */
    uint32 addr1WndwMask;       /* 0x06c */
    uint32 addr1WndwBaseIn;     /* 0x070 */
    uint32 addr1WndwBaseOut;    /* 0x074 */
    uint32 addr2WndwMask;       /* 0x078 */
    uint32 addr2WndwBaseIn;     /* 0x07c */
    uint32 addr2WndwBaseOut;    /* 0x080 */
    uint32 scratch;             /* 0x084 */
    uint32 reserved9;           /* 0x088 */
    uint32 softResets;          /* 0x08c */
    uint32 reserved2;           /* 0x090 */
    uint32 m4keCoreStatus;      /* 0x094 */
    uint32 reserved3;           /* 0x098 */
    uint32 ubSlaveTimeout;      /* 0x09c */
    uint32 diagEn;              /* 0x0a0 */
    uint32 devTimeout;          /* 0x0a4 */
    uint32 ubusErrorOutMask;    /* 0x0a8 */
    uint32 diagCaptStopMask;    /* 0x0ac */
    uint32 revId;               /* 0x0b0 */
    uint32 reserved4[4];        /* 0x0b4 */
    uint32 diagCtrl;            /* 0x0c4 */
    uint32 diagStat;            /* 0x0c8 */
    uint32 diagMask;            /* 0x0cc */
    uint32 diagRslt;            /* 0x0d0 */
    uint32 diagCmp;             /* 0x0d4 */
    uint32 diagCapt;            /* 0x0d8 */    
    uint32 diagCnt;             /* 0x0dc */
    uint32 diagEdgeCnt;         /* 0x0e0 */
    uint32 reserved5[4];	    /* 0x0e4 */
    uint32 smisc_bus_config;    /* 0x0f4 */
    uint32 lfsr;                /* 0x0f8 */
    uint32 dqm_pac_lock;        /* 0x0fc */
    uint32 l1_irq_4ke_mask;     /* 0x100 */
    uint32 l1_irq_4ke_status;   /* 0x104 */
    uint32 l1_irq_mips_mask;    /* 0x108 */
    uint32 l1_irq_mips_status;  /* 0x10c */
    uint32 l1_irq_mips1_mask;   /* 0x110 */
    uint32 reserved6[3];        /* 0x114 */
    uint32 l2_irq_gp_mask;      /* 0x120 */
    uint32 l2_irq_gp_status;    /* 0x124 */
    uint32 l2_irq_gp_set;       /* 0x128 */
    uint32 reserved7;           /* 0x12c */
    uint32 gp_in_irq_mask;      /* 0x130 */
    uint32 gp_in_irq_status;    /* 0x134 */
    uint32 gp_in_irq_set;       /* 0x138 */
    uint32 gp_in_irq_sense;     /* 0x13c */
    uint32 gp_in;               /* 0x140 */
    uint32 gp_out;              /* 0x144 */
} PmcCtrlReg;

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
	uint32 freqScalarCtrl; /* 0x3c */
	uint32 freqScalarMask; /* 0x40 */
} PmcPerfPowReg;

typedef struct PmcDQMPac {
    uint32 dqmPac[32];
} PmcDQMPac;

typedef struct PmcDQMReg {
	uint32 cfg;                     /* 0x1c00 */
	uint32 _4keLowWtmkIrqMask;      /* 0x1c04 */
	uint32 mipsLowWtmkIrqMask;      /* 0x1c08 */
	uint32 lowWtmkIrqMask;          /* 0x1c0c */
	uint32 _4keNotEmptyIrqMask;     /* 0x1c10 */
	uint32 mipsNotEmptyIrqMask;     /* 0x1c14 */
	uint32 notEmptyIrqSts;          /* 0x1c18 */
	uint32 queueRst;                /* 0x1c1c */
	uint32 notEmptySts;             /* 0x1c20 */
	uint32 nextAvailMask;           /* 0x1c24 */
	uint32 nextAvailQueue;          /* 0x1c28 */
	uint32 mips1LowWtmkIrqMask;     /* 0x1c2c */
	uint32 mips1NotEmptyIrqMask;    /* 0x1c30 */
	uint32 autoSrcPidInsert;        /* 0x1c34 */
    uint32 timerIrqStatus;          /* 0x1c38 */
    uint32 timerStatus;             /* 0x1c3c */
    uint32 _4keTimerIrqMask;        /* 0x1c40 */
    uint32 mipsTimerIrqMask;        /* 0x1c44 */
    uint32 mips1TimerIrqMask;       /* 0x1c48 */
} PmcDQMReg;

typedef struct PmcCntReg {
	uint32 cntr[10];
	uint32 unused[6];	/* 0x28-0x3f */
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

typedef struct SSBMaster {
    uint32 ssbmControl;     /* 0x0060 */
    uint32 ssbmWrData;      /* 0x0064 */
    uint32 ssbmRdData;      /* 0x0068 */
    uint32 ssbmStatus;      /* 0x006c */
} SSBMaster;

typedef struct PmmReg {
    uint32 memPowerCtrl;            /* 0x0000 */
    uint32 regSecurityConfig;       /* 0x0004 */
} PmmReg;

typedef struct keyholeReg {
    uint32 ctrlSts;
    uint32 wrData;
    uint32 mutex;
    uint32 rdData;
} keyholeReg;

typedef struct PmbBus {
    uint32 config;          /* 0x0100 */
    uint32 arbiter;         /* 0x0104 */
    uint32 timeout;         /* 0x0108 */
    uint32 unused1;         /* 0x010c */
    keyholeReg keyhole[4];  /* 0x0110-0x014f */
    uint32 unused2[44];     /* 0x0150-0x01ff */
    uint32 map[64];         /* 0x0200-0x02ff */ 
}PmbBus;

typedef struct  CoreCtrl {
    uint32  coreEnable;         /* 0x0400 */
    uint32  autoresetControl;   /* 0x0404 */
    uint32  coreIdle;           /* 0x0408 */
    uint32  coreResetCause;     /* 0x040c */
    uint32  memPwrDownCtrl0;    /* 0x0410 */
    uint32  memPwrDownSts0;     /* 0x0414 */
    uint32  memPwrDownCtrl1;    /* 0x0418 */
    uint32  memPwrDownSts1;     /* 0x041c */
    uint32  sysFlg0Status;      /* 0x0420 */
    uint32  sysFlg0Set;         /* 0x0424 */
    uint32  sysFlg0Clear;       /* 0x0428 */
    uint32  unused1;            /* 0x042c */
    uint32  usrFlg0Status;      /* 0x0430 */
    uint32  usrFlg0Set;         /* 0x0434 */
    uint32  usrFlg0Clear;       /* 0x0438 */
    uint32  unused2;            /* 0x043c */
    uint32  subsystemRev;       /* 0x0440 */
    uint32  resetVector;        /* 0x0444 */
} CoreCtrl;

typedef struct  CoreState {
    uint32  sysMbx[8];          /* 0x0480 */
    uint32  usrMbx[8];          /* 0x04a0 */
    uint32  sysMtx[4];          /* 0x04c0 */
    uint32  usrMtx[8];          /* 0x04d0 */
} CoreState;

typedef struct  CoreIntr {
    uint32  irqStatus;          /* 0x0500 */
    uint32  irqSet;             /* 0x0504 */
    uint32  irqClear;           /* 0x0508 */
    uint32  unused1;            /* 0x050c */
    uint32  srqStatus;          /* 0x0510 */
    uint32  srqSet;             /* 0x0514 */
    uint32  srqClear;           /* 0x0518 */
    uint32  unused2;            /* 0x051c */
    uint32  drqStatus;          /* 0x0520 */
    uint32  drqSet;             /* 0x0524 */
    uint32  drqClear;           /* 0x0528 */
    uint32  unused3;            /* 0x052c */
    uint32  frqStatus;          /* 0x0530 */
    uint32  frqSet;             /* 0x0534 */
    uint32  frqClear;           /* 0x0538 */
    uint32  unused4;            /* 0x053c */
    uint32  hostIrqLatched;     /* 0x0540 */
    uint32  hostIrqSet;         /* 0x0544 */
    uint32  hostIrqClear;       /* 0x0548 */
    uint32  hostIrqEnable;      /* 0x054c */
    uint32  obusFaultStatus;    /* 0x0550 */
    uint32  obusFaultClear;     /* 0x0554 */
    uint32  obusFaultAddr;      /* 0x0558 */
} CoreIntr;

typedef struct CoreProfile {
    uint32  mutex;              /* 0x0580 */
    uint32  lastConfPcLo;       /* 0x0584 */
    uint32  lastConfPcHi;       /* 0x0588 */
    uint32  lastPcLo;           /* 0x058c */
    uint32  lastPcHi;           /* 0x0590 */
    uint32  braTargetPc0Lo;     /* 0x0594 */
    uint32  braTargetPc0Hi;     /* 0x0598 */
    uint32  braTargetPc1Lo;     /* 0x059c */
    uint32  braTargetPc1Hi;     /* 0x05a0 */
    uint32  braTargetPc2Lo;     /* 0x05a4 */
    uint32  braTargetPc2Hi;     /* 0x05a8 */
    uint32  braTargetPc3Lo;     /* 0x05ac */
    uint32  braTargetPc3Hi;     /* 0x05b0 */
    uint32  unused[3];          /* 0x05b4-0x05bf */
    uint32  profSampleW[4];     /* 0x05c0 */
} CoreProfile;

typedef struct MaestroMisc {
    CoreCtrl coreCtrl;          /* 0x0400 */
    uint32   unused1[14];       /* 0x0448-0x047f */
    CoreState coreState;        /* 0x0480 */
    uint32   unused2[4];        /* 0x04f0-0x04ff */
    CoreIntr interrupt;         /* 0x0500 */
    uint32   unused3[9];        /* 0x055c-0x057f */
    CoreProfile profile;        /* 0x0580 */
} MaestroMisc;

typedef struct Pmc {
    uint32 unused0[1030];
    PmcCtrlReg ctrl;		            /* 0x1018 */
    uint32 unused1[622];	            /* 0x1148-0x1cff */
    PmcDQMPac dqmPac;                   /* 0x1b00 */
    uint32 unused5[32];                 /* 0x1b80-0x1bff */
    PmcDQMReg dqm;			            /* 0x1c00 */
    uint32 unused6[749];		        /* 0x1c4c-0x27ff */
    uint32 qStatus[32];		            /* 0x2800 */
    uint32 unused7[480];	            /* 0x2880-0x2fff */
    PmcDqmQMibReg qMib;		            /* 0x3000 */
    uint32 unused8[928];	            /* 0x3180-0x3fff */
    PmcDqmQCtrlReg dqmQCtrl[8]; 	    /* 0x4000 */
    uint32 unused9[992];                /* 0x4080-0x4fff */
    PmcDqmQDataReg dqmQData[8]; 	    /* 0x5000 */
} Pmc;
#define PMC ((volatile Pmc * const) PMC_BASE)

typedef struct Procmon {
    uint32 unused00[256];
    MaestroMisc maestroReg;             /* 0x00400 */
    uint32 unused10[32396];             /* 0x005d0-0x1ffff */
    PmmReg  pmm;                        /* 0x20000 */
    uint32 unused11[22];                /* 0x20008-0x2005f */
    SSBMaster ssbMasterCtrl;            /* 0x20060 */
    uint32 unused12[36];                /* 0x20070-0x200ff */
    PmbBus pmb;                         /* 0x20100 */
    uint32 unused13[32576];             /* 0x20300-0x3ffff */
    uint32 qsm[128];                    /* 0x40000-0x401ff */
    uint32 unused14[65408];             /* 0x40200-0x7ffff */
    uint32 dtcm[1024];                  /* 0x80000-0x80fff */
} Procmon;
#define PROCMON ((volatile Procmon * const) PROC_MON_BASE)

typedef struct PMSSBMasterControl {
	uint32 control;
	uint32 wr_data;
	uint32 rd_data;
} PMSSBMasterControl;

typedef struct
{
    uint32  control;
#define PMC_PMBM_START		            (1 << 31)
#define PMC_PMBM_TIMEOUT	            (1 << 30)
#define PMC_PMBM_SLAVE_ERR	            (1 << 29)
#define PMC_PMBM_BUSY		            (1 << 28)
#define PMC_PMBM_BUS_SHIFT              (20)
#define PMC_PMBM_Read		            (0 << 24)
#define PMC_PMBM_Write		            (1 << 24)
    uint32  wr_data;
    uint32  mutex;
    uint32  rd_data;
} PMB_keyhole_reg;

typedef struct PMBMaster {
    uint32 config;
#define PMB_NUM_REGS_SHIFT (20)
#define PMB_NUM_REGS_MASK  (0x3ff)
    uint32 arbitger;
    uint32 timeout;
    uint32 reserved;
    PMB_keyhole_reg keyhole[4];
    uint32 reserved1[44];
    uint32 map[64];
} PMBMaster;
#define PMB ((volatile PMBMaster * const) PMB_BASE)

/*
 * LedControl Register Set Definitions.
 */
typedef struct LedConfig {
    uint32 config[4];
} LedConfig;
typedef struct LedControl {
    uint32 glbCtrl;		        /* 0x00 */
#define LED_SERIAL_LED_MSB_FIRST        0x10
    uint32 hWLedEn;		        /* 0x04 */
    uint32 serialLedShiftSel;           /* 0x08 */
    uint32 HwPolarity;	                /* 0x0c */
    uint32 SwData;		        /* 0x10 */
    uint32 SwPolarity;	                /* 0x14 */
    uint32 ParallelLedPolarity;	        /* 0x18 */
    uint32 ChnActive;	                /* 0x1c */
    LedConfig LedCfg[32];	        /* 0x20 */
    LedConfig LedActiveCfg[32];	        /* 0x220 */
    uint32 BpFactor;	                /* 0x420 */
    uint32 BtFactor;	                /* 0x424 */
    uint32 CbtCount;	                /* 0x428 */
    uint32 SLedCount;	                /* 0x42c */
    uint32 HwLedStatus;	                /* 0x430 */
    uint32 FlashCtrlStatus;             /* 0x434 */
    uint32 BrtCtrlStatus;               /* 0x438 */
    uint32 ParallelOutStatus;           /* 0x43c */
    uint32 SerialRegStatus;             /* 0x440 */
} LedControl;

#define LED ((volatile LedControl * const) LED_BASE)
#define LED_NUM_LEDS       32
#define LED_NUM_TO_MASK(X)       (1 << ((X) & (LED_NUM_LEDS-1)))


#define GPIO_NUM_TO_LED_MODE_SHIFT(X) \
    ((((X) & BP_GPIO_NUM_MASK) < 8) ? (32 + (((X) & BP_GPIO_NUM_MASK) << 1)) : \
    ((((X) & BP_GPIO_NUM_MASK) - 8) << 1))

typedef struct AXIInterface {
   uint32 CFG;			/* 0x00 */
   uint32 REP_ARB_MODE;		/* 0x04 */
   uint32 QUEUE_CFG;            /* 0x08 */
   uint32 ESRCID_CFG;           /* 0x0c */
   uint32 SRC_QUEUE_CTRL[8];    /* 0x10 */
   uint32 SRC_ID_4_QUEUE_CTRL;  /* 0x30 */
   uint32 SCRATCH;              /* 0x34 */
   uint32 DEBUG[2];             /* 0x38 */
} AXIInterface;

typedef struct EDISEngine {
   uint32 REV_ID;             /* 0x00 */
   uint32 CTRL_TRIG;          /* 0x04 */
   uint32 CTRL_MODE;          /* 0x08 */
   uint32 CTRL_SIZE;          /* 0x0c */
   uint32 CTRL_ADDR_START;    /* 0x10 */
   uint32 CTRL_ADDR_START_EXT;/* 0x14 */
   uint32 CTRL_ADDR_END;      /* 0x18 */
   uint32 CTRL_ADDR_END_EXT;  /* 0x1c */
   uint32 CTRL_WRITE_MASKS;   /* 0x20 */
   uint32 CTRL_INT_ENABLES;   /* 0x24 */
   uint32 unused0[6];         /* 0x28-0x3f */
   uint32 STAT_MAIN;          /* 0x40 */
   uint32 STAT_WORDS_WRITTEN; /* 0x44 */
   uint32 STAT_WORDS_READ;    /* 0x48 */
   uint32 STAT_ERROR_COUNT;   /* 0x4c */
   uint32 STAT_ERROR_BITS;    /* 0x50 */
   uint32 STAT_ADDR_LAST;     /* 0x54 */
   uint32 STAT_ADDR_LAST_EXT; /* 0x58 */
   uint32 STAT_CLOCK_CYCLES;  /* 0x5c */
   uint32 unused1[7];         /* 0x60-0x7b */
   uint32 STAT_DEBUG;         /* 0x7c */
   uint32 STAT_DATA_PORT[8];  /* 0x80-0x9c */
   uint32 GEN_LFSR_STATE[4];  /* 0xa0-0xac */
   uint32 GEN_CLOCK;          /* 0xb0 */
   uint32 GEN_PATTERN;        /* 0xb4 */
   uint32 unused2[2];         /* 0xb8-0xbf */
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

typedef struct RangeCtrl {
   uint32 CTRL;                /* 0x00 */
   uint32 UBUS0_PORT;          /* 0x04 */
   uint32 UBUS0_PORT_UPPER;    /* 0x08 */
   uint32 BASE;                /* 0x0c */
   uint32 BASE_UPPER;          /* 0x10 */
   uint32 SECLEC_EN;           /* 0x14 */
} RangeCtrl;

typedef struct SecureRangeCheckers {
   uint32 LOCK;            /* 0x00 */
   uint32 LOG_INFO[3];     /* 0x04 - 0x0c */
   RangeCtrl RANGE_CTRL[8];/* 0x10 - 0xcf */
   uint32 unused[12];      /* 0xd0 - 0xff */
} SecureRangeCheckers;

typedef struct DDRPhyControl {
   uint32 REVISION;        /* 0x00 */
   uint32 PLL_STATUS;      /* 0x04 */
   uint32 PLL_CONFIG;      /* 0x08 */
   uint32 PLL_CONTROL1;    /* 0x0c */
   uint32 PLL_CONTROL2;    /* 0x10 */
   uint32 PLL_CONTROL3;    /* 0x14 */
   uint32 PLL_DIVIDER;     /* 0x18 */
   uint32 PLL_PRE_DIVIDER; /* 0x1c */
   uint32 PLL_SS_EN;       /* 0x20 */
   uint32 PLL_SS_CFG;      /* 0x24 */
   uint32 unused00;        /* 0x28 */
   uint32 IDLE_PAD_CONTROL;/* 0x2c */
   uint32 IDLE_PAD_EN0;    /* 0x30 */
   uint32 IDLE_PAD_EN1;    /* 0x34 */
   uint32 DRIVE_PAD_CTL;   /* 0x38 */
   uint32 DRIVE_PAD_CTL_2T;/* 0x3c */
   uint32 STATIC_PAD_CTL;  /* 0x40 */
   uint32 DRAM_CFG;        /* 0x44 */
   uint32 unused01;        /* 0x48 */
   uint32 DRAM_TIMING2;    /* 0x4c */
   uint32 DRAM_TIMING3;    /* 0x50 */
   uint32 unused02;        /* 0x54 */
   uint32 VDL_REGS[45];    /* 0x58-0x108 */
   uint32 UPDATE_VDL;      /* 0x10C */
   uint32 UPDATE_VDL_SNOOP1;  /* 0x110 */
   uint32 UPDATE_VDL_SNOOP2;  /* 0x114 */
   uint32 CMND_REG1;          /* 0x118 */
   uint32 CMND_AUX_REG1;      /* 0x11C */
   uint32 CMND_REG2;          /* 0x120 */
   uint32 CMND_AUX_REG2;      /* 0x124 */
   uint32 CMND_REG3;          /* 0x128 */
   uint32 CMND_AUX_REG3;      /* 0x12C */
   uint32 CMND_REG4;          /* 0x130 */
   uint32 CMND_AUX_REG4;      /* 0x134 */
   uint32 MODE_REG[4];        /* 0x138-144 */
   uint32 unused2[13];        /* 0x148-178 */
   uint32 WRITE_LEVEL_CTRL;   /* 0x17c */
   uint32 WRITE_LEVEL_STATUS; /* 0x180 */
   uint32 READ_EN_CTRL;       /* 0x184 */
   uint32 READ_EN_STATUS;     /* 0x188 */
   uint32 VIRT_VTT_CTRL;      /* 0x18C */
   uint32 VIRT_VTT_STATUS;    /* 0x190 */
   uint32 VIRT_VTT_CONNECTION;/* 0x194 */
   uint32 VIRT_VTT_OVERRIDE;  /* 0x198 */
   uint32 VREF_DAC_CTRL;      /* 0x19C */
   uint32 PHYBIST[12];        /* 0x1A0-0x1CC */
   uint32 STANDBY_CTRL;       /* 0x1D0 */
   uint32 DEBUG_FREEZE_EN;    /* 0x1D4 */
   uint32 DEBUG_MUX_CTRL;     /* 0x1D8 */
   uint32 DFI_CTRL;           /* 0x1DC */
   uint32 unused3[8];         /* 0x1E0-1FC */
   uint32 CLOCK_IDLE;         /* 0x200 */
   uint32 unused5[127];       /* 0x204-0x3ff */
} DDRPhyControl;

typedef struct DDRPhyByteLaneControl {
   uint32 VDL_CTRL_WR[11]; /* 0x00-0x28 */
   uint32 VDL_CTRL_RD[22]; /* 0x2c-0x80 */
   uint32 VDL_CLK_CTRL;    /* 0x84 */
   uint32 VDL_LDE_CTRL;    /* 0x88 */
   uint32 RD_EN_DLY_CYC;   /* 0x8c */
   uint32 WR_CHAN_DLY_CYC; /* 0x90 */
   uint32 RD_CTRL;         /* 0x94 */
   uint32 RD_FIFO_ADDR;    /* 0x98 */
   uint32 RD_FIFO_DATA;    /* 0x9C */
   uint32 RD_FIFO_DM_DBI;  /* 0xA0 */
   uint32 RD_FIFO_STATUS;  /* 0xA4 */
   uint32 RD_FIFO_CLR;     /* 0xA8 */
   uint32 IDLE_PAD_CTRL;   /* 0xAC */
   uint32 DRIVE_PAD_CTRL;  /* 0xB0 */
   uint32 DQSP_DRIVE_PAD_CTRL;  /* 0xB4 */
   uint32 DQSN_DRIVE_PAD_CTRL;  /* 0xB8 */
   uint32 unused1;              /* 0xBC */
   uint32 RD_EN_DRIVE_PAD_CTRL; /* 0xC0 */
   uint32 STATIC_PAD_CTRL;      /* 0xC4 */
   uint32 WR_PREAMBLE_MODE;     /* 0xC8 */
   uint32 ODT_CTRL;             /* 0xCC */
   uint32 CLOCK_ENABLE;         /* 0xD0 */
   uint32 CLOCK_IDLE;           /* 0xD4 */
   uint32 BL_SPARE_REG;         /* 0xD8 */
   uint32 unused4[73];          /* 0xDC */
} DDRPhyByteLaneControl;

typedef struct MEMCControl {
   uint32 GLB_VERS;               /* 0x000 */
   uint32 GLB_GCFG;               /* 0x004 */
#define MEMC_GLB_GCFG_DRAM_EN_SHIFT        31
#define MEMC_GLB_GCFG_DRAM_EN_MASK         (0x1<<MEMC_GLB_GCFG_DRAM_EN_SHIFT)
#define MEMC_GLB_GCFG_FORCE_SEQ_SHIFT      30
#define MEMC_GLB_GCFG_FORCE_SEQ_MASK       (0x1<<MEMC_GLB_GCFG_FORCE_SEQ_SHIFT)
#define MEMC_GLB_GCFG_FORCE_SEQ2_SHIFT     29
#define MEMC_GLB_GCFG_FORCE_SEQ2_MASK      (1<<MEMC_GLB_GCFG_FORCE_SEQ2_SHIFT)
#define MEMC_GLB_GCFG_RBF_REORDER_SHIFT    28
#define MEMC_GLB_GCFG_RBF_REORDER_MASK     (1<<MEMC_GLB_GCFG_RBF_REORDER_SHIFT)
#define MEMC_GLB_GCFG_SREF_SLOW_CLK_SHIFT  26
#define MEMC_GLB_GCFG_SREF_SLOW_CLK_MASK   (1<<MEMC_GLB_GCFG_SREF_SLOW_CLK_SHIFT)
#define MEMC_GLB_GCFG_PHY_DFI_MODE_SHIFT   24
#define MEMC_GLB_GCFG_PHY_DFI_MODE_MASK    (0x3<<MEMC_GLB_GCFG_PHY_DFI_MODE_SHIFT)
#define MEMC_GLB_GCFG_PHY_DFI_2X           0x0
#define MEMC_GLB_GCFG_PHY_DFI_4X           0x1
#define MEMC_GLB_GCFG_PHY_DFI_8X           0x2
#define MEMC_GLB_GCFG_DFI_EN_SHIFT         10
#define MEMC_GLB_GCFG_DFI_EN_MASK          (0x1<<MEMC_GLB_GCFG_DFI_EN_SHIFT)
#define MEMC_GLB_GCFG_MCLKSRC_SHIFT        9
#define MEMC_GLB_GCFG_MCLKSRC_MASK         (0x1<<MEMC_GLB_GCFG_MCLKSRC_SHIFT)
#define MEMC_GLB_GCFG_MEMINITDONE_SHIFT    8
#define MEMC_GLB_GCFG_MEMINITDONE_MASK     (0x1<<MEMC_GLB_GCFG_MEMINITDONE_SHIFT)
#define MEMC_GLB_GCFG_MRQSIZE_SHIFT        0
#define MEMC_GLB_GCFG_MRQSIZE_MASK         (0x1f<<MEMC_GLB_GCFG_MRQSIZE_SHIFT)
   uint32 GLB_MRQ_CFG;            /* 0x008 */
   uint32 unused0;                /* 0x00c */
   uint32 GLB_FSBL_STATE;         /* 0x010 */
#define MEMC_GLB_FSBL_DRAM_SIZE_SHIFT      0
#define MEMC_GLB_FSBL_DRAM_SIZE_MASK       (0xf << MEMC_GLB_FSBL_DRAM_SIZE_SHIFT)
   uint32 unused1[3];             /* 0x014 - 0x01f */

   uint32 SRAM_REMAP_CTRL;        /* 0x020 */
   uint32 SRAM_REMAP_CTRL_UPPER;  /* 0x024 */
   uint32 SRAM_REMAP_INIT;        /* 0x028 */
   uint32 SRAM_REMAP_LOG_INFO_0;  /* 0x02c */
   uint32 SRAM_REMAP_LOG_INFO_1;  /* 0x030 */
   uint32 SRAM_REMAP_LOG_INFO_2;  /* 0x034 */
   uint32 unused2[2];             /* 0x038-0x03f */

   uint32 RBF_REORDER_0_CFG[16];  /* 0x040 - 0x07f */

   uint32 INTR2_CPU_STATUS;       /* 0x080 */
   uint32 INTR2_CPU_SET;          /* 0x084 */
   uint32 INTR2_CPU_CLEAR;        /* 0x088 */
   uint32 INTR2_CPU_MASK_STATUS;  /* 0x08c */
   uint32 INTR2_CPU_MASK_SET;     /* 0x090 */
   uint32 INTR2_CPU_MASK_CLEAR;   /* 0x094 */
   uint32 unused3[26];            /* 0x098-0x0ff */

   uint32 CHN_CFG_CNFG;           /* 0x100 */
#define MEMC_CHN_CFG_CNFG_CS_MODE_SHIFT            16
#define MEMC_CHN_CFG_CNFG_CS_MODE_MASK             (0x3 << MEMC_CHN_CFG_CNFG_CS_MODE_SHIFT)
   uint32 CHN_CFG_CSST;           /* 0x104 */
   uint32 unused4[2];             /* 0x108-0x10f */
   uint32 CHN_CFG_ROW00_0;        /* 0x110 */
   uint32 CHN_CFG_ROW00_1;        /* 0x114 */
   uint32 CHN_CFG_ROW01_0;        /* 0x118 */
   uint32 CHN_CFG_ROW01_1;        /* 0x11c */
   uint32 CHN_CFG_ROW02_0;        /* 0x120 */
   uint32 CHN_CFG_COL00_0;        /* 0x124 */
   uint32 CHN_CFG_COL00_1;        /* 0x128 */
   uint32 CHN_CFG_COL01_0;        /* 0x12c */
   uint32 CHN_CFG_COL01_1;        /* 0x130 */
   uint32 CHN_CFG_BNK10;          /* 0x134 */
   uint32 CHN_CFG_BG0;            /* 0x138 */
   uint32 unused5;                /* 0x13c */
   uint32 CHN_CFG_DRAM_SZ_CHK;    /* 0x140 */
#define MEMC_CHN_CFG_DRAM_SZ_CHK_SHIFT  4
#define MEMC_CHN_CFG_DRAM_SZ_CHK_MASK   (0xf<<MEMC_CHN_CFG_DRAM_SZ_CHK_SHIFT)
   uint32 CHN_CFG_DIAG_SEL;       /* 0x144 */
   uint32 unused6[46];            /* 0x148-0x1ff */

   uint32 CHN_TIM_DCMD;           /* 0x200 */
   uint32 CHN_TIM_DMODE_0;        /* 0x204 */
   uint32 CHN_TIM_DMODE_2;        /* 0x208 */
   uint32 CHN_TIM_CLKS;           /* 0x20c */
#define MEMC_CHN_TIM_CLKS_REF_RATE_SHIFT          8
#define MEMC_CHN_TIM_CLKS_REF_DISABLE_SHIFT       31
   uint32 CHN_TIM_ODT;            /* 0x210 */
   uint32 CHN_TIM_TIM1_0;         /* 0x214 */
/* MC_CHN_TIM :: TIM1_0 :: TIM1_tWL [31:24] */
#define MC_CHN_TIM_TIM1_0_TIM1_tWL_MASK                            0xff000000
#define MC_CHN_TIM_TIM1_0_TIM1_tWL_ALIGN                           0
#define MC_CHN_TIM_TIM1_0_TIM1_tWL_BITS                            8
#define MC_CHN_TIM_TIM1_0_TIM1_tWL_SHIFT                           24
#define MC_CHN_TIM_TIM1_0_TIM1_tWL_DEFAULT                         0x00000004
/* MC_CHN_TIM :: TIM1_0 :: TIM1_tRP [23:16] */
#define MC_CHN_TIM_TIM1_0_TIM1_tRP_MASK                            0x00ff0000
#define MC_CHN_TIM_TIM1_0_TIM1_tRP_ALIGN                           0
#define MC_CHN_TIM_TIM1_0_TIM1_tRP_BITS                            8
#define MC_CHN_TIM_TIM1_0_TIM1_tRP_SHIFT                           16
#define MC_CHN_TIM_TIM1_0_TIM1_tRP_DEFAULT                         0x00000006
/* MC_CHN_TIM :: TIM1_0 :: TIM1_tCL [15:08] */
#define MC_CHN_TIM_TIM1_0_TIM1_tCL_MASK                            0x0000ff00
#define MC_CHN_TIM_TIM1_0_TIM1_tCL_ALIGN                           0
#define MC_CHN_TIM_TIM1_0_TIM1_tCL_BITS                            8
#define MC_CHN_TIM_TIM1_0_TIM1_tCL_SHIFT                           8
#define MC_CHN_TIM_TIM1_0_TIM1_tCL_DEFAULT                         0x00000005
/* MC_CHN_TIM :: TIM1_0 :: TIM1_tRCD [07:00] */
#define MC_CHN_TIM_TIM1_0_TIM1_tRCD_MASK                           0x000000ff
#define MC_CHN_TIM_TIM1_0_TIM1_tRCD_ALIGN                          0
#define MC_CHN_TIM_TIM1_0_TIM1_tRCD_BITS                           8
#define MC_CHN_TIM_TIM1_0_TIM1_tRCD_SHIFT                          0
#define MC_CHN_TIM_TIM1_0_TIM1_tRCD_DEFAULT                        0x00000006

   uint32 CHN_TIM_TIM1_1;         /* 0x218 */
/* MC_CHN_TIM :: TIM1_1 :: TIM1_tCCD_L [31:24] */
#define MC_CHN_TIM_TIM1_1_TIM1_tCCD_L_MASK                         0xff000000
#define MC_CHN_TIM_TIM1_1_TIM1_tCCD_L_ALIGN                        0
#define MC_CHN_TIM_TIM1_1_TIM1_tCCD_L_BITS                         8
#define MC_CHN_TIM_TIM1_1_TIM1_tCCD_L_SHIFT                        24
#define MC_CHN_TIM_TIM1_1_TIM1_tCCD_L_DEFAULT                      0x00000002
/* MC_CHN_TIM :: TIM1_1 :: TIM1_tCCD_S [23:16] */
#define MC_CHN_TIM_TIM1_1_TIM1_tCCD_S_MASK                         0x00ff0000
#define MC_CHN_TIM_TIM1_1_TIM1_tCCD_S_ALIGN                        0
#define MC_CHN_TIM_TIM1_1_TIM1_tCCD_S_BITS                         8
#define MC_CHN_TIM_TIM1_1_TIM1_tCCD_S_SHIFT                        16
#define MC_CHN_TIM_TIM1_1_TIM1_tCCD_S_DEFAULT                      0x00000002
/* MC_CHN_TIM :: TIM1_1 :: TIM1_tRRD_L [15:08] */
#define MC_CHN_TIM_TIM1_1_TIM1_tRRD_L_MASK                         0x0000ff00
#define MC_CHN_TIM_TIM1_1_TIM1_tRRD_L_ALIGN                        0
#define MC_CHN_TIM_TIM1_1_TIM1_tRRD_L_BITS                         8
#define MC_CHN_TIM_TIM1_1_TIM1_tRRD_L_SHIFT                        8
#define MC_CHN_TIM_TIM1_1_TIM1_tRRD_L_DEFAULT                      0x00000002
/* MC_CHN_TIM :: TIM1_1 :: TIM1_tRRD_S [07:00] */
#define MC_CHN_TIM_TIM1_1_TIM1_tRRD_S_MASK                         0x000000ff
#define MC_CHN_TIM_TIM1_1_TIM1_tRRD_S_ALIGN                        0
#define MC_CHN_TIM_TIM1_1_TIM1_tRRD_S_BITS                         8
#define MC_CHN_TIM_TIM1_1_TIM1_tRRD_S_SHIFT                        0
#define MC_CHN_TIM_TIM1_1_TIM1_tRRD_S_DEFAULT                      0x00000002

   uint32 CHN_TIM_TIM1_2;         /* 0x21c */
/* MC_CHN_TIM :: TIM1_2 :: TIM1_tFAW [31:24] */
#define MC_CHN_TIM_TIM1_2_TIM1_tFAW_MASK                           0xff000000
#define MC_CHN_TIM_TIM1_2_TIM1_tFAW_ALIGN                          0
#define MC_CHN_TIM_TIM1_2_TIM1_tFAW_BITS                           8
#define MC_CHN_TIM_TIM1_2_TIM1_tFAW_SHIFT                          24
#define MC_CHN_TIM_TIM1_2_TIM1_tFAW_DEFAULT                        0x00000000
/* MC_CHN_TIM :: TIM1_2 :: reserved0 [23:16] */
#define MC_CHN_TIM_TIM1_2_reserved0_MASK                           0x00ff0000
#define MC_CHN_TIM_TIM1_2_reserved0_ALIGN                          0
#define MC_CHN_TIM_TIM1_2_reserved0_BITS                           8
#define MC_CHN_TIM_TIM1_2_reserved0_SHIFT                          16
/* MC_CHN_TIM :: TIM1_2 :: TIM1_tRTP [15:08] */
#define MC_CHN_TIM_TIM1_2_TIM1_tRTP_MASK                           0x0000ff00
#define MC_CHN_TIM_TIM1_2_TIM1_tRTP_ALIGN                          0
#define MC_CHN_TIM_TIM1_2_TIM1_tRTP_BITS                           8
#define MC_CHN_TIM_TIM1_2_TIM1_tRTP_SHIFT                          8
#define MC_CHN_TIM_TIM1_2_TIM1_tRTP_DEFAULT                        0x00000002
/* MC_CHN_TIM :: TIM1_2 :: TIM1_tRC [07:00] */
#define MC_CHN_TIM_TIM1_2_TIM1_tRC_MASK                            0x000000ff
#define MC_CHN_TIM_TIM1_2_TIM1_tRC_ALIGN                           0
#define MC_CHN_TIM_TIM1_2_TIM1_tRC_BITS                            8
#define MC_CHN_TIM_TIM1_2_TIM1_tRC_SHIFT                           0
#define MC_CHN_TIM_TIM1_2_TIM1_tRC_DEFAULT                         0x00000011

   uint32 CHN_TIM_TIM1_3;         /* 0x220 */
/* MC_CHN_TIM :: TIM1_3 :: TIM1_tWTR_L [31:24] */
#define MC_CHN_TIM_TIM1_3_TIM1_tWTR_L_MASK                         0xff000000
#define MC_CHN_TIM_TIM1_3_TIM1_tWTR_L_ALIGN                        0
#define MC_CHN_TIM_TIM1_3_TIM1_tWTR_L_BITS                         8
#define MC_CHN_TIM_TIM1_3_TIM1_tWTR_L_SHIFT                        24
#define MC_CHN_TIM_TIM1_3_TIM1_tWTR_L_DEFAULT                      0x00000002
/* MC_CHN_TIM :: TIM1_3 :: TIM1_tWTR_S [23:16] */
#define MC_CHN_TIM_TIM1_3_TIM1_tWTR_S_MASK                         0x00ff0000
#define MC_CHN_TIM_TIM1_3_TIM1_tWTR_S_ALIGN                        0
#define MC_CHN_TIM_TIM1_3_TIM1_tWTR_S_BITS                         8
#define MC_CHN_TIM_TIM1_3_TIM1_tWTR_S_SHIFT                        16
#define MC_CHN_TIM_TIM1_3_TIM1_tWTR_S_DEFAULT                      0x00000002
/* MC_CHN_TIM :: TIM1_3 :: reserved_for_padding0 [15:12] */
#define MC_CHN_TIM_TIM1_3_reserved_for_padding0_MASK               0x0000f000
#define MC_CHN_TIM_TIM1_3_reserved_for_padding0_ALIGN              0
#define MC_CHN_TIM_TIM1_3_reserved_for_padding0_BITS               4
#define MC_CHN_TIM_TIM1_3_reserved_for_padding0_SHIFT              12
/* MC_CHN_TIM :: TIM1_3 :: TIM1_tWR_L [11:08] */
#define MC_CHN_TIM_TIM1_3_TIM1_tWR_L_MASK                          0x00000f00
#define MC_CHN_TIM_TIM1_3_TIM1_tWR_L_ALIGN                         0
#define MC_CHN_TIM_TIM1_3_TIM1_tWR_L_BITS                          4
#define MC_CHN_TIM_TIM1_3_TIM1_tWR_L_SHIFT                         8
#define MC_CHN_TIM_TIM1_3_TIM1_tWR_L_DEFAULT                       0x00000004
/* MC_CHN_TIM :: TIM1_3 :: TIM1_tWR_S [07:00] */
#define MC_CHN_TIM_TIM1_3_TIM1_tWR_S_MASK                          0x000000ff
#define MC_CHN_TIM_TIM1_3_TIM1_tWR_S_ALIGN                         0
#define MC_CHN_TIM_TIM1_3_TIM1_tWR_S_BITS                          8
#define MC_CHN_TIM_TIM1_3_TIM1_tWR_S_SHIFT                         0
#define MC_CHN_TIM_TIM1_3_TIM1_tWR_S_DEFAULT                       0x00000004

   uint32 CHN_TIM_TIM2;          /* 0x224 */
/* MC_CHN_TIM :: TIM2 :: TIM2_tR2R [31:30] */
#define MC_CHN_TIM_TIM2_TIM2_tR2R_MASK                             0xc0000000
#define MC_CHN_TIM_TIM2_TIM2_tR2R_ALIGN                            0
#define MC_CHN_TIM_TIM2_TIM2_tR2R_BITS                             2
#define MC_CHN_TIM_TIM2_TIM2_tR2R_SHIFT                            30
#define MC_CHN_TIM_TIM2_TIM2_tR2R_DEFAULT                          0x00000000
/* MC_CHN_TIM :: TIM2 :: TIM2_tR2W [29:27] */
#define MC_CHN_TIM_TIM2_TIM2_tR2W_MASK                             0x38000000
#define MC_CHN_TIM_TIM2_TIM2_tR2W_ALIGN                            0
#define MC_CHN_TIM_TIM2_TIM2_tR2W_BITS                             3
#define MC_CHN_TIM_TIM2_TIM2_tR2W_SHIFT                            27
#define MC_CHN_TIM_TIM2_TIM2_tR2W_DEFAULT                          0x00000001
/* MC_CHN_TIM :: TIM2 :: TIM2_tW2R [26:24] */
#define MC_CHN_TIM_TIM2_TIM2_tW2R_MASK                             0x07000000
#define MC_CHN_TIM_TIM2_TIM2_tW2R_ALIGN                            0
#define MC_CHN_TIM_TIM2_TIM2_tW2R_BITS                             3
#define MC_CHN_TIM_TIM2_TIM2_tW2R_SHIFT                            24
#define MC_CHN_TIM_TIM2_TIM2_tW2R_DEFAULT                          0x00000001
/* MC_CHN_TIM :: TIM2 :: reserved_for_padding0 [23:22] */
#define MC_CHN_TIM_TIM2_reserved_for_padding0_MASK                 0x00c00000
#define MC_CHN_TIM_TIM2_reserved_for_padding0_ALIGN                0
#define MC_CHN_TIM_TIM2_reserved_for_padding0_BITS                 2
#define MC_CHN_TIM_TIM2_reserved_for_padding0_SHIFT                22
/* MC_CHN_TIM :: TIM2 :: TIM2_tW2W [21:18] */
#define MC_CHN_TIM_TIM2_TIM2_tW2W_MASK                             0x003c0000
#define MC_CHN_TIM_TIM2_TIM2_tW2W_ALIGN                            0
#define MC_CHN_TIM_TIM2_TIM2_tW2W_BITS                             4
#define MC_CHN_TIM_TIM2_TIM2_tW2W_SHIFT                            18
#define MC_CHN_TIM_TIM2_TIM2_tW2W_DEFAULT                          0x00000000
/* MC_CHN_TIM :: TIM2 :: reserved1 [17:16] */
#define MC_CHN_TIM_TIM2_reserved1_MASK                             0x00030000
#define MC_CHN_TIM_TIM2_reserved1_ALIGN                            0
#define MC_CHN_TIM_TIM2_reserved1_BITS                             2
#define MC_CHN_TIM_TIM2_reserved1_SHIFT                            16
/* MC_CHN_TIM :: TIM2 :: TIM2_tAL [15:12] */
#define MC_CHN_TIM_TIM2_TIM2_tAL_MASK                              0x0000f000
#define MC_CHN_TIM_TIM2_TIM2_tAL_ALIGN                             0
#define MC_CHN_TIM_TIM2_TIM2_tAL_BITS                              4
#define MC_CHN_TIM_TIM2_TIM2_tAL_SHIFT                             12
#define MC_CHN_TIM_TIM2_TIM2_tAL_DEFAULT                           0x00000000
/* MC_CHN_TIM :: TIM2 :: TIM2_tRFC [11:00] */
#define MC_CHN_TIM_TIM2_TIM2_tRFC_MASK                             0x00000fff
#define MC_CHN_TIM_TIM2_TIM2_tRFC_ALIGN                            0
#define MC_CHN_TIM_TIM2_TIM2_tRFC_BITS                             12
#define MC_CHN_TIM_TIM2_TIM2_tRFC_SHIFT                            0
#define MC_CHN_TIM_TIM2_TIM2_tRFC_DEFAULT                          0x00000014

   uint32 unused7[2];            /* 0x228 - 0x22f */
   uint32 CHN_TIM_PHY_ST;         /* 0x230 */
#define MEMC_CHN_TIM_PHY_ST_PHY_ST_POWER_UP       0x1
#define MEMC_CHN_TIM_PHY_ST_PHY_ST_HW_RESET       0x2
#define MEMC_CHN_TIM_PHY_ST_PHY_ST_SW_RESET       0x4
#define MEMC_CHN_TIM_PHY_ST_PHY_ST_READY          0x10
   uint32 CHN_TIM_DRAM_CFG;       /* 0x234 */
#define MEMC_CHN_TIM_DRAM_CFG_HDP_SHIFT           12
#define MEMC_CHN_TIM_DRAM_CFG_HDP_MASK            (0x1<<MEMC_CHN_TIM_DRAM_CFG_HDP_SHIFT)
#define MEMC_CHN_TIM_DRAM_CFG_2TADDRCMD_SHIFT     10
#define MEMC_CHN_TIM_DRAM_CFG_2TADDRCMD_MASK      (0x1<<MEMC_CHN_TIM_DRAM_CFG_2TADDRCMD_SHIFT)
#define MEMC_CHN_TIM_DRAM_CFG_DRAMTYPE_SHIFT      0
#define MEMC_CHN_TIM_DRAM_CFG_DRAMTYPE_MASK       (0xf<<MEMC_CHN_TIM_DRAM_CFG_DRAMTYPE_SHIFT)
#define MC_DRAM_CFG_DDR3                          0x1
   uint32 CHN_TIM_STAT;           /* 0x238 */
   uint32 CHN_TIM_PERF;           /* 0x23c */

   uint32 CHN_TIM_PKM_CFG;        /* 0x240 */
   uint32 CHN_TIM_AB4_CFG;        /* 0x244 */
   uint32 unused8[2];             /* 0x248 */
   uint32 CHN_TIM_AB4_ACT_PRI;    /* 0x250 */
   uint32 CHN_TIM_AB4_PRI_OPEN;   /* 0x254 */
   uint32 CHN_TIM_AB4_PRI_CLOSED; /* 0x258 */
   uint32 CHN_TIM_AUTO_SELF_REFRESH;  /* 0x25c */
#define MEMC_DDR_AUTO_SELFREFRESH_EN              (1 << 31)
#define MEMC_DDR_AUTO_SR_IDLE_CNT_MASK            (0x7FFFFFFF)
   uint32 CHN_TIM_AUTO_ZQCS;      /* 0x260 */
   uint32 CHN_TIM_TIM3;           /* 0x264 */
   uint32 unused9[38];            /* 0x268-0x2ff */

   uint32 ARB_CFG;                /* 0x300 */
   uint32 ARB_QUE_DIS[2];         /* 0x304 */
   uint32 unused10;               /* 0x30c */
   uint32 ARB_BKT_CFG_CPU_WR[4];  /* 0x310 */
   uint32 ARB_BKT_CFG_CPU_RD[4];  /* 0x320 */
   uint32 ARB_BKT_CFG_CPQ_RD[4];  /* 0x330 */
   uint32 ARB_BKT_CFG_CPQ_WR[4];  /* 0x340 */
   uint32 ARB_BKT_CFG_EDIS[4];    /* 0x350 */
   uint32 unused11[32];           /* 0x360 */
   uint32 ARB_QUEUE_PRI[3];       /* 0x3e0 */
   uint32 ARB_WRR_QUEUE_PRI;      /* 0x3ec */
   uint32 ARB_RATE_OK_WEIGHT[2];  /* 0x3f0 */
   uint32 ARB_RATE_FAIL_WEIGHT[2];/* 0x3f8 */

   uint32 unused12[48];           /* 0x400 */
   AXIInterface AXIWIF;		  /* 0x4c0 */

   uint32 unused13[192];	  /* 0x500-0x7ff */

   EDISEngine EDIS_0;             /* 0x800 */
   uint32 unused14[64];           /* 0x900 */

   uint32 STATS_CTRL;             /* 0xa00 */
   uint32 STATS_TIMER_CFG;        /* 0xa04 */
   uint32 STATS_TIMER_COUNT;      /* 0xa08 */
   uint32 STATS_TOTAL_SLICE;      /* 0xa0c */
   uint32 STATS_TOTAL_PACKET;     /* 0xa10 */
   uint32 STATS_TOTAL_READ_SLICE; /* 0xa14 */
   uint32 STATS_TOTAL_READ_PACKET;/* 0xa18 */
   uint32 STATS_TOTAL_LATENCY;    /* 0xa1c */
   uint32 STAT_MAX_LATENCY;       /* 0xa20 */
   uint32 STATS_SLICE_REORDER;    /* 0xa24 */
   uint32 STATS_TOTAL_DDR_CMD;    /* 0xa28 */
   uint32 STATS_TOTAL_DDR_ACT;    /* 0xa2c */
   uint32 STATS_TOTAL_DDR_RDWR;   /* 0xa30 */
   uint32 STATS_TOTAL_DDR_WRRD;   /* 0xa34 */
   uint32 STATS_TOTAL_MRQ_CHN_FULL; /* 0xa38 */
   uint32 STATS_TOTAL_SELF_REF;   /* 0xa3c */
   uint32 STATS_ARB_GRANT_MATCH0; /* 0xa40 */
   uint32 STATS_TOTAL_QUEUE_FULL; /* 0xa44 */
   uint32 STATS_TOTAL_ARB_GRANT;  /* 0xa48 */
   uint32 STATS_TOTAL_RATE_OK_GRANT; /* 0xa4c*/
   uint32 STATS_FILTER_CFG_0;     /* 0xa50 */
#define MEMC_STATS_FILTER_CFG_MATCH_SRC_SHIFT  25
#define MEMC_STATS_FILTER_CFG_MATCH_SRC_MASK   (0x1<<MEMC_STATS_FILTER_CFG_MATCH_SRC_SHIFT)
#define MEMC_STATS_FILTER_CFG_MATCH_SRC_EN     0x02000000
#define MEMC_STATS_FILTER_CFG_SRC_ID_SHIFT     16
#define MEMC_STATS_FILTER_CFG_SRC_ID_MASK      (0x1FF<<MEMC_STATS_FILTER_CFG_SRC_ID_SHIFT)
#define MEMC_STATS_FILTER_CFG_MATCH_INTF_SHIFT 0
#define MEMC_STATS_FILTER_CFG_MATCH_INTF_MASK  0x0000FFFF
#define MEMC_STATS_FILTER_CFG_INTF_UBUS        0x00000001
#define MEMC_STATS_FILTER_CFG_INTF_AXI         0x00000002
#define MEMC_STATS_FILTER_CFG_INTF_EDIS0       0x00000004
#define MEMC_STATS_FILTER_CFG_INTF_UNUSED      0x00000008
#define MEMC_STATS_FILTER_CFG_INTF_UBUS0       MEMC_STATS_FILTER_CFG_INTF_UBUS
#define MEMC_STATS_FILTER_CFG_INTF_UBUS1       MEMC_STATS_FILTER_CFG_INTF_UNUSED
#define MEMC_STATS_FILTER_CFG_INTF_MCP         MEMC_STATS_FILTER_CFG_INTF_UNUSED
#define MEMC_STATS_FILTER_CFG_INTF_EDIS1       MEMC_STATS_FILTER_CFG_INTF_UNUSED
   uint32 STATS_PROG0_SLICE;      /* 0xa54 */
   uint32 STATS_PROG0_PACKET;     /* 0xa58 */
   uint32 STATS_PROG0_READ_SLICE; /* 0xa5c */
   uint32 STATS_PROG0_READ_PACKET; /* 0xa60 */
   uint32 STATS_PROG0_LATENCY;    /* 0xa64 */ 
   uint32 STATS_PROG0_MAX_LETANCY; /* 0xa68 */

   // uint32 unused15[37];            /* 0xa6c */
   uint32 unused15_0;              /* 0xa6c */
   uint32 STATS_FILTER_CFG_1;     /* 0xa70 */
   uint32 STATS_PROG1_SLICE;      /* 0xa74 */
   uint32 STATS_PROG1_PACKET;     /* 0xa78 */ 
   uint32 STATS_PROG1_READ_SLICE; /* 0xa7c */  
   uint32 STATS_PROG1_READ_PACKET;/* 0xa80 */
   uint32 STATS_PROG1_LATENCY;    /* 0xa84 */ 
   uint32 unused15_1[2];          /* 0xa88 - 0xa8c */   
   uint32 STATS_FILTER_CFG_2;     /* 0xa90 */
   uint32 STATS_PROG2_SLICE;      /* 0xa94 */
   uint32 STATS_PROG2_PACKET;     /* 0xa98 */ 
   uint32 STATS_PROG2_READ_SLICE; /* 0xa9c */  
   uint32 STATS_PROG2_READ_PACKET;/* 0xaa0 */
   uint32 STATS_PROG2_LATENCY;    /* 0xaa4 */ 
   uint32 unused15_2[2];          /* 0xaa8 - 0xaac */
   uint32 STATS_FILTER_CFG_3;     /* 0xab0 */
   uint32 STATS_PROG3_SLICE;      /* 0xab4 */
   uint32 STATS_PROG3_PACKET;     /* 0xab8 */ 
   uint32 STATS_PROG3_READ_SLICE; /* 0xabc */  
   uint32 STATS_PROG3_READ_PACKET;/* 0xac0 */
   uint32 STATS_PROG3_LATENCY;    /* 0xac4 */
   uint32 unused15_3[2];            /* 0xac8 - 0xacc */
   uint32 STATS_AXI_BLOCK_CPU_RD; /* 0xad0 */    
   uint32 STATS_AXI_BLOCK_CPQ_RD; /* 0xad4 */    
   uint32 unused15_4[10];           /* 0xad8 - 0xaff */
   
   uint32 CAP_CAPTURE_CFG;        /* 0xb00 */
   uint32 CAP_TRIGGER_ADDR;       /* 0xb04 */
   uint32 CAP_READ_CTRL;          /* 0xb08 */
   uint32 unused16;               /* 0xb0c */
   uint32 CAP_CAPTURE_MATCH0;     /* 0xb10 */
   uint32 CAP_CAPTURE_MATCH1;     /* 0xb14 */
   uint32 CAP_CAPTURE_MATCH2;     /* 0xb18 */
   uint32 unused17;               /* 0xb1c */
   uint32 CAP_CAPTURE_MASK0;      /* 0xb20 */
   uint32 CAP_CAPTURE_MASK1;      /* 0xb24 */
   uint32 CAP_CAPTURE_MASK2;      /* 0xb28 */
   uint32 unused18;               /* 0xb2c */
   uint32 CAP_TRIGGER_MATCH0;     /* 0xb30 */
   uint32 CAP_TRIGGER_MATCH1;     /* 0xb34 */
   uint32 CAP_TRIGGER_MATCH2;     /* 0xb38 */
   uint32 unused19;               /* 0xb3c */
   uint32 CAP_TRIGGER_MASK0;      /* 0xb40 */
   uint32 CAP_TRIGGER_MASK1;      /* 0xb44 */
   uint32 CAP_TRIGGER_MASK2;      /* 0xb48 */
   uint32 unused20;               /* 0xb4c */
   uint32 CAP_READ_DATA[8];       /* 0xb50-0xb6f */
   uint32 unused21[164];          /* 0xb70-0xdff */

   uint32 SEC_INTR2_CPU_STATUS;   /* 0xe00 */
   uint32 SEC_INTR2_CPU_SET;      /* 0xe04 */
   uint32 SEC_INTR2_CPU_CLEAR;    /* 0xe08 */
   uint32 SEC_INTR2_CPU_MASK_STATUS;/* 0xe0c */
   uint32 SEC_INTR2_CPU_MASK_SET;   /* 0xe10 */
   uint32 SEC_INTR2_CPU_MASK_CLEAR; /* 0xe14 */
   uint32 unused22[58];             /* 0xe18-0xeff */

   uint32 UBUSIF0_PERMCTL;	    /* 0xf00 */
   uint32 UBUSIF1_PERMCTL;          /* 0xf04 */
   uint32 SRAM_REMAP_PERMCTL;       /* 0xf08 */
   uint32 AXIWIF_PERMCTL;           /* 0xf0c */
   uint32 CHNCFG_PERMCTL;           /* 0xf10 */
   uint32 MCCAP_PERMCTL;            /* 0xf14 */
   uint32 SCRAM_PERMCTL;            /* 0xf18 */
   uint32 RNG_PERMCTL;              /* 0xf1c */
   uint32 RNGCHK_PERMCTL;           /* 0xf20 */
   uint32 unused23[567];            /* 0xf24-0x17ff */

   SecureRangeCheckers SEC_RANGE_CHK; /* 0x1800-0x18ff */
   uint32 unused24[31168];            /* 0x1900-0x1ffff */

   DDRPhyControl PhyControl;                    /* 0x20000 */
   DDRPhyByteLaneControl PhyByteLane0Control;   /* 0x20400 - 0x205ff */
   DDRPhyByteLaneControl PhyByteLane1Control;   /* 0x20600 - 0x207ff */
} MEMCControl;

#define MEMC ((volatile MEMCControl * const) MEMC_BASE)

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
    uint32 mdio_cfg;                          /* 0x0004 */
} EthernetSwitchMDIO;

#define ETHSW_MDIO ((volatile EthernetSwitchMDIO * const) SWITCH_MDIO_BASE)




typedef struct Jtag_Otp {
   uint32 ctrl0;                          /* 0x00 */
#define JTAG_OTP_CTRL_ACCESS_MODE         (0x2 << 22)
#define JTAG_OTP_CTRL_PROG_EN             (1 << 21)
#define JTAG_OTP_CTRL_START               (1 << 0)
#define JTAG_OTP_CTRL_CMD_OTP_PROG_EN	(0x2 << 1)
#define JTAG_OTP_CTRL_CMD_PROG		(0xa << 1)
#define JTAG_OTP_CTRL_CMD_PROG_LOCK	(0x19 << 1)
   uint32 ctrl1;                          /* 0x04 */
#define JTAG_OTP_CTRL_CPU_MODE            (1 << 0)
   uint32 ctrl2;                          /* 0x08 */
   uint32 ctrl2_hi;                       /* 0x0c */
   uint32 ctrl3;                          /* 0x10 */
   uint32 ctrl4;                          /* 0x14 */
   uint32 status0;                        /* 0x18 */
   uint32 status0_hi;                     /* 0x20 */
   uint32 status1;                        /* 0x24 */
   uint32 status[12];                     /* 0x28 */
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

/* row 8 */
#define OTP_CPU_CORE_CFG_ROW            8
#define OTP_CPU_CORE_CFG_SHIFT          28
#define OTP_CPU_CORE_CFG_MASK           (0x1 << OTP_CPU_CORE_CFG_SHIFT)

#define OTP_SEC_CHIPVAR_ROW             8
#define OTP_SEC_CHIPVAR_SHIFT           24
#define OTP_SEC_CHIPVAR_MASK            (0xf << OTP_SEC_CHIPVAR_SHIFT)

/* row 9 */
#define OTP_CPU_CLOCK_FREQ_ROW          9
#define OTP_CPU_CLOCK_FREQ_SHIFT        0
#define OTP_CPU_CLOCK_FREQ_MASK         (0x7 << OTP_CPU_CLOCK_FREQ_SHIFT)

#define OTP_LDO_TRIM_ROW                9
#define OTP_LDO_TRIM_SHIFT              28
#define OTP_LDO_TRIM_MASK               (0xf << OTP_LDO_TRIM_SHIFT)

/* row 14 */
#define OTP_PCM_DISABLE_ROW             14
#define OTP_PCM_DISABLE_SHIFT           13
#define OTP_PCM_DISABLE_MASK            (0x1 << OTP_PCM_DISABLE_SHIFT)

/* row 17 */
#define OTP_BRCM_BTRM_BOOT_ENABLE_ROW           17
#define OTP_BRCM_BTRM_BOOT_ENABLE_SHIFT         3
#define OTP_BRCM_BTRM_BOOT_ENABLE_MASK          (1 << OTP_BRCM_BTRM_BOOT_ENABLE_SHIFT)

/* row 18 */
#define OTP_CUST_BTRM_BOOT_ENABLE_ROW           18
#define OTP_CUST_BTRM_BOOT_ENABLE_SHIFT         15
#define OTP_CUST_BTRM_BOOT_ENABLE_MASK          (7 << OTP_CUST_BTRM_BOOT_ENABLE_SHIFT)

/* row 20 */
#define OTP_CHIPID_ROW              20
#define OTP_CHIPID_SHIFT            0
#define OTP_CHIP_ID_MASK            (0xffffffff << OTP_CHIPID_SHIFT)

/* row 23 */
#define OTP_CUST_MFG_MRKTID_ROW                 23
#define OTP_CUST_MFG_MRKTID_SHIFT               0
#define OTP_CUST_MFG_MRKTID_MASK                (0xffff << OTP_CUST_MFG_MRKTID_SHIFT)


/* row 26 */
#define OTP_BOOT_SW_ENET_BOOT_DIS_ROW       	26
#define OTP_BOOT_SW_ENET_BOOT_DIS_SHIFT      	0
#define OTP_BOOT_SW_ENET_BOOT_DIS_MASK      	(7 << OTP_BOOT_SW_ENET_BOOT_DIS_SHIFT)


#define OTP_BOOT_SW_ENET_BOOT_FALLBACK_SHIFT    3
#define OTP_BOOT_SW_ENET_BOOT_FALLBACK_MASK     (7 << OTP_BOOT_SW_ENET_BOOT_FALLBACK_SHIFT)


#define OTP_BOOT_SW_ENET_RGMII_SHIFT		6
#define OTP_BOOT_SW_ENET_RGMII_MASK  		(7 << OTP_BOOT_SW_ENET_RGMII_SHIFT)



/* SOTP defs */
#define SOTP_OTP_REGION_RD_LOCK                 0x3c
typedef struct
{
   uint32 lut[32];           /* 0x00 */
   uint32 queue_depth[4];    /* 0x80 */
   uint32 cbs_thresh[8];     /* 0x90 */
   uint32 cir_incr[4];       /* 0xb0 */
   uint32 ref_cnt[4];        /* 0xc0 */
   uint32 max_bonus[2];      /* 0xd0 */
#define QUE_ID_NUM_BITS     (4) /* Number of queue_id bits per source port */
#define DEPTH_NUM_BITS      (8) /* Number of depth bits per queue id */
#define CBS_NUM_BITS        (16)/* Number of CBS bits per queue id */
#define CIR_INCR_NUM_BITS   (8) /* Number of CIS_INCR bits per queue id */
#define REF_CNT_NUM_BITS    (8) /* Number of refresh cnt bits per queue id */
#define MAX_BONUS_NUM_BITS  (4) /* Number of max bonus bits per queue id
                                   Actually 3-bits but 4th is reserved */

#define MAX_WLU_SRCPID_NUM                    256
#define MAX_WLU_SRCPID_REG_NUM                8
#define WLU_SRCPID_TO_REG_OFFSET(srcpid)      ((srcpid)>>5)
#define WLU_SRCPID_TO_REG_BIT(srcpid)         ((srcpid)%32)
   uint32 wlu_srcpid[MAX_WLU_SRCPID_REG_NUM];     /* 0xd8 */
   uint32 qos_reg[2];        /* 0xf8 */
}CoherencyPortCfgReg_t;

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
    uint32 anonymous[192];        /* 0x400 - 0x6ff */
    BIUCFG_Aux aux;               /* 0x700 - 0x7ff */
    uint32 anonymous_1[2560];     /* 0x800 - 0x2fff */
}BIUCFG;
#define BIUCFG ((volatile BIUCFG * const) BIUCFG_BASE)


/* USB Host contorl regs */
typedef struct usb_ctrl{
    uint32 setup;
#define USBH_IPP                (1<<5)
#define USBH_IOC                (1<<4)
#define USBH_STRAP_IPP_SEL      (1<<25)
#define USB2_OC_DISABLE_PORT0   (1<<28)
#define USB2_OC_DISABLE_PORT1   (1<<29)
#define USB3_OC_DISABLE_PORT0   (1<<30)
#define USB3_OC_DISABLE_PORT1   (1<<31)
    uint32 pll_ctl;
    uint32 fladj_value;
    uint32 bridge_ctl;
#define USB_BRCTL_OHCI_MEM_REQ_DIS (1<<16)
    uint32 spare1;
    uint32 mdio;
    uint32 mdio2;
    uint32 test_port_control;
    uint32 usb_simctl;
    uint32 usb_testctl;
    uint32 usb_testmon;
    uint32 utmi_ctl_1;
    uint32 utmi_ctl_2;
    uint32 usb_pm;
#define XHC_SOFT_RESETB         (1<<22)
#define USB_PWRDWN              (1<<31)
    uint32 usb_pm_status;
    uint32 spare3;
    uint32 pll_ldo_ctl;
    uint32 pll_ldo_pllbias;
    uint32 pll_afe_bg_cntl;
    uint32 afe_usbio_tst;
    uint32 pll_ndiv_frac;
    uint32 tp_diag;
    uint32 ahb_capture_fifo;
    uint32 spare4;
    uint32 usb30_ctl1;
#define PHY3_PLL_SEQ_START      (1<<4)
    uint32 usb30_ctl2;
    uint32 usb30_ctl3;
    uint32 usb30_ctl4;
    uint32 usb30_pctl;
    uint32 usb30_ctl5;
    uint32 spare5;
    uint32 spare6;
    uint32 spare7;
    uint32 unsused1[3];
    uint32 usb_device_ctl1;
    uint32 usb_device_ctl2;
    uint32 unsused2[22];
    uint32 usb20_id;
    uint32 usb30_id;
    uint32 bdc_coreid;
    uint32 usb_revid;
} usb_ctrl;
#define USBH_CTRL ((volatile usb_ctrl * const) USBH_CFG_BASE)

typedef struct Ubus4SysModuleTop {
    uint32 unused0[32];      /* 0x0 */
    uint32 UcbData;          /* 0x40 */
    uint32 UcbHdr;           /* 0x44 */
    uint32 UcbCntl;          /* 0x48 */
    uint32 unused1;          /* 0x4c */
    uint32 ReadUcbHdr;       /* 0x50 */
    uint32 ReadUcbData;      /* 0x54 */
    uint32 ReadUcbStatus;    /* 0x58 */
    uint32 ReacUcbFifoStatus; /* 0x5c */
} Ubus4SysModuleTop;
#define UBUSSYSTOP ((volatile Ubus4SysModuleTop * const) UBUS_SYS_MODULE_BASE)

  typedef struct Ubus4ModuleClientRegistration {
    uint32 SlvStatus[16];	/* 0x0	 */
    uint32 MstStatus[ 8];    	/* 0x240 */
    uint32 RegCntl;    		/* 0x260 */
    uint32 SlvStopProgDelay;	/* 0x264 */
} Ubus4ModuleClientRegistration;
#define UBUSSYSTOP_REGISTRATION  \
		((volatile Ubus4ModuleClientRegistration * const) \
		UBUS_SYS_MODULE_REGISTRATION_BASE)

typedef struct Ubus4ClkCtrlCfgRegs {
    uint32 ClockCtrl;
#define UBUS4_CLK_CTRL_EN_SHIFT    (0)
#define UBUS4_CLK_CTRL_EN_MASK     (0x1 << UBUS4_CLK_CTRL_EN_SHIFT)
#define UBUS4_CLK_BYPASS_SHIFT     (2)
#define UBUS4_CLK_BYPASS_MASK      (0x1 << UBUS4_CLK_BYPASS_SHIFT)
#define UBUS4_MIN_CLK_SEL_SHIFT    (4)
#define UBUS4_MIN_CLK_SEL_MASK     (0x7 << UBUS4_MIN_CLK_SEL_SHIFT)
#define UBUS4_MID_CLK_SEL_SHIFT    (8)
#define UBUS4_MID_CLK_SEL_MASK     (0x7 << UBUS4_MID_CLK_SEL_SHIFT)
    uint32 reserved0[3];
    uint32 Min2Mid_threshhold;
    uint32 Mid2Max_threshhold;
    uint32 Mid2Min_threshhold;
    uint32 Max2Mid_threshhold;
    uint32 ClkIntoMin;
    uint32 ClkIntoMid;
    uint32 ClkIntoMax;
    uint32 reserved1;
    uint32 ClkMinTime;
    uint32 ClkMidTime;
    uint32 ClkMaxTime;
} Ubus4ClkCtrlCfgRegs;
#define UBUS4CLK ((volatile Ubus4ClkCtrlCfgRegs * const) UBUS_MAPPED_BASE)


/*
** Eth Switch Registers
*/
typedef struct {
    unsigned int led_f;
    unsigned int reserved;
} LED_F;

typedef struct EthernetSwitchCore
{
	uint64 port_traffic_ctrl[9];
	uint8 dummy1[8];
	uint64 SWITCH_CORE_RX_GLOBAL_CTL;
#define ETHSW_SM_RETRY_LIMIT_DIS                  0x04
#define ETHSW_SM_FORWARDING_EN                    0x02
#define ETHSW_SM_MANAGED_MODE                     0x01
	uint32 switch_mode;
	uint8 dummy2[148];
	uint32 SWITCH_CORE_DEBUG_REG;
	uint8 dummy3[20];
	uint64 SWITCH_CORE_NEW_CTRL;
	uint32 switch_ctrl;
#define ETHSW_SC_MII_DUMP_FORWARDING_EN           0x40
#define ETHSW_SC_MII2_VOL_SEL                     0x02
	uint8 dummy4[12];
	uint32 SWITCH_CORE_PROTECTED_SEL;
	uint8 dummy5[12];
	uint32 SWITCH_CORE_WAN_PORT_SEL;
	uint8 dummy6[68];
	uint32 SWITCH_CORE_RSV_MCAST_CTRL;
	uint8 dummy7[12];
	uint64 SWITCH_CORE_TXQ_FLUSH_MODE;
	uint32 SWITCH_CORE_ULF_DROP_MAP;
	uint8 dummy8[12];
	uint32 SWITCH_CORE_MLF_DROP_MAP;
	uint8 dummy9[12];
	uint32 SWITCH_CORE_MLF_IPMC_FWD_MAP;
	uint8 dummy10[12];
	uint32 SWITCH_CORE_RX_PAUSE_PASS;
	uint8 dummy11[12];
	uint32 SWITCH_CORE_TX_PAUSE_PASS;
	uint8 dummy12[12];
	uint32 SWITCH_CORE_DIS_LEARN;
	uint8 dummy13[12];
	uint32 SWITCH_CORE_SFT_LRN_CTL;
	uint8 dummy14[12];
	uint32 SWITCH_CORE_LOW_POWER_EXP1;
	uint8 dummy15[156];
	uint32 SWITCH_CORE_CTLREG_REG_SPARE;
	uint8 dummy16[292];
	uint64 software_reset;
	uint32 SWITCH_CORE_WATCH_DOG_RPT1;
	uint8 dummy17[12];
	uint32 SWITCH_CORE_WATCH_DOG_RPT2;
	uint8 dummy18[12];
	uint32 SWITCH_CORE_WATCH_DOG_RPT3;
	uint8 dummy19[12];
	uint64 SWITCH_CORE_PAUSE_FRM_CTRL;
	uint32 SWITCH_CORE_PAUSE_ST_ADDR;
	uint8 dummy20[52];
	uint64 SWITCH_CORE_FAST_AGE_CTRL;
	uint64 SWITCH_CORE_FAST_AGE_PORT;
	uint32 SWITCH_CORE_FAST_AGE_VID;
	uint8 dummy21[668];
	uint32 SWITCH_CORE_LOW_POWER_CTRL;
	uint8 dummy22[76];
	uint32 SWITCH_CORE_TCAM_CTRL;
	uint8 dummy23[12];
	uint32 SWITCH_CORE_TCAM_CHKSUM_STS;
	uint8 dummy24[12];
	uint32 SWITCH_CORE_LIGHTSTACK_CTRL;
	uint8 dummy25[156];
	uint32 SWITCH_CORE_LNKSTS;
	uint8 dummy26[12];
	uint32 SWITCH_CORE_LNKSTSCHG;
	uint8 dummy27[12];
	uint32 SWITCH_CORE_SPDSTS;
	uint8 dummy28[28];
	uint32 SWITCH_CORE_DUPSTS;
	uint8 dummy29[12];
	uint32 SWITCH_CORE_PAUSESTS;
	uint8 dummy30[28];
	uint32 SWITCH_CORE_SRCADRCHG;
	uint8 dummy31[12];
	uint32 SWITCH_CORE_LSA_PORT_P0;
	uint8 dummy32[44];
	uint32 SWITCH_CORE_LSA_PORT_P1;
	uint8 dummy33[44];
	uint32 SWITCH_CORE_LSA_PORT_P2;
	uint8 dummy34[44];
	uint32 SWITCH_CORE_LSA_PORT_P3;
	uint8 dummy35[44];
	uint32 SWITCH_CORE_LSA_PORT_P4;
	uint8 dummy36[44];
	uint32 SWITCH_CORE_LSA_PORT_P5;
	uint8 dummy37[44];
	uint32 SWITCH_CORE_LSA_PORT_P6;
	uint8 dummy38[44];
	uint32 SWITCH_CORE_LSA_PORT_P7;
	uint8 dummy39[44];
	uint32 SWITCH_CORE_LSA_PORT_P8;
	uint8 dummy40[44];
	uint32 SWITCH_CORE_BIST_STS0;
	uint8 dummy41[44];
	uint32 SWITCH_CORE_BIST_STS1;
	uint8 dummy42[28];
	uint32 SWITCH_CORE_PBPTRFIFO_0;
	uint8 dummy43[44];
	uint32 SWITCH_CORE_PBPTRFIFO_1;
	uint8 dummy44[460];
	uint32 SWITCH_CORE_RESET_STATUS;
	uint8 dummy45[124];
	uint32 SWITCH_CORE_STREG_REG_SPARE0;
	uint8 dummy46[28];
	uint32 SWITCH_CORE_STREG_REG_SPARE1;
	uint8 dummy47[732];
	uint64 SWITCH_CORE_GMNGCFG;
	uint64 SWITCH_CORE_IMP0_PRT_ID;
	uint64 SWITCH_CORE_IMP1_PRT_ID;
	uint32 brcm_hdr_ctrl;
	uint8 dummy48[20];
	uint32 SWITCH_CORE_SPTAGT;
	uint8 dummy49[28];
	uint32 SWITCH_CORE_BRCM_HDR_CTRL2;
	uint8 dummy50[12];
	uint32 SWITCH_CORE_IPG_SHRNK_CTRL;
	uint8 dummy51[28];
	uint32 SWITCH_CORE_MIRCAPCTL;
	uint8 dummy52[12];
	uint32 SWITCH_CORE_IGMIRCTL;
	uint8 dummy53[12];
	uint32 SWITCH_CORE_IGMIRDIV;
	uint8 dummy54[12];
	uint32 SWITCH_CORE_IGMIRMAC;
	uint8 dummy55[44];
	uint32 SWITCH_CORE_EGMIRCTL;
	uint8 dummy56[12];
	uint32 SWITCH_CORE_EGMIRDIV;
	uint8 dummy57[12];
	uint32 SWITCH_CORE_EGMIRMAC;
	uint8 dummy58[44];
	uint32 SWITCH_CORE_SPANCTL;
	uint8 dummy59[28];
	uint32 SWITCH_CORE_RSPANVLAN;
	uint8 dummy60[300];
	uint32 SWITCH_CORE_HL_PRTC_CTRL;
	uint8 dummy61[28];
	uint32 SWITCH_CORE_RST_MIB_CNT_EN;
	uint8 dummy62[28];
	uint32 SWITCH_CORE_IPG_SHRINK_2G_WA;
	uint8 dummy63[60];
	uint32 SWITCH_CORE_BRCM_HDR_RX_DIS;
	uint8 dummy64[12];
	uint32 SWITCH_CORE_BRCM_HDR_TX_DIS;
	uint8 dummy65[108];
	uint32 SWITCH_CORE_MNGMODE_REG_SPARE0;
	uint8 dummy66[28];
	uint32 SWITCH_CORE_MNGMODE_REG_SPARE1;
	uint8 dummy67[1116];
	uint32 SWITCH_CORE_INT_STS;
	uint8 dummy68[60];
	uint32 SWITCH_CORE_INT_EN;
	uint8 dummy69[60];
	uint32 SWITCH_CORE_SLEEP_TIMER_IMP;
	uint8 dummy70[12];
	uint32 SWITCH_CORE_PORT7_SLEEP_TIMER;
	uint8 dummy71[12];
	uint32 SWITCH_CORE_WAN_SLEEP_TIMER;
	uint8 dummy72[28];
	uint32 SWITCH_CORE_PORT_SLEEP_STS;
	uint8 dummy73[92];
	uint32 SWITCH_CORE_LINK_STS_INT_EN;
	uint8 dummy74[28];
	uint32 SWITCH_CORE_ENG_DET_INT_EN;
	uint8 dummy75[12];
	uint32 SWITCH_CORE_LPI_STS_CHG_INT_EN;
	uint8 dummy76[428];
	uint32 SWITCH_CORE_MEM_ECC_ERR_INT_STS;
	uint8 dummy77[12];
	uint32 SWITCH_CORE_MEM_ECC_ERR_INT_EN;
	uint8 dummy78[12];
	uint32 SWITCH_CORE_PORT_EVT_ECC_ERR_STS;
	uint8 dummy79[12];
	uint32 SWITCH_CORE_PORT_MIB_ECC_ERR_STS;
	uint8 dummy80[12];
	uint32 SWITCH_CORE_PORT_TXQ_ECC_ERR_STS;
	uint8 dummy81[60];
	uint32 SWITCH_CORE_PROBE_BUS_CTL;
	uint8 dummy82[28];
	uint32 SWITCH_CORE_MDC_EXTEND_CTRL;
	uint8 dummy83[92];
	uint32 SWITCH_CORE_PPPOE_SESSION_PARSE_EN;
	uint8 dummy84[124];
	uint32 SWITCH_CORE_CTLREG_1_REG_SPARE0;
	uint8 dummy85[28];
	uint32 SWITCH_CORE_CTLREG_1_REG_SPARE1;
	uint8 dummy86[860];
	uint32 SWITCH_CORE_GARLCFG;
	uint8 dummy87[28];
	uint32 SWITCH_CORE_BPDU_MCADDR;
	uint8 dummy88[76];
	uint32 SWITCH_CORE_MULTI_PORT_CTL;
	uint8 dummy89[12];
	uint32 SWITCH_CORE_MULTIPORT_ADDR0;
	uint8 dummy90[60];
	uint32 SWITCH_CORE_MPORTVEC0;
	uint8 dummy91[60];
	uint32 SWITCH_CORE_MULTIPORT_ADDR1;
	uint8 dummy92[60];
	uint32 SWITCH_CORE_MPORTVEC1;
	uint8 dummy93[60];
	uint32 SWITCH_CORE_MULTIPORT_ADDR2;
	uint8 dummy94[60];
	uint32 SWITCH_CORE_MPORTVEC2;
	uint8 dummy95[60];
	uint32 SWITCH_CORE_MULTIPORT_ADDR3;
	uint8 dummy96[60];
	uint32 SWITCH_CORE_MPORTVEC3;
	uint8 dummy97[60];
	uint32 SWITCH_CORE_MULTIPORT_ADDR4;
	uint8 dummy98[60];
	uint32 SWITCH_CORE_MPORTVEC4;
	uint8 dummy99[60];
	uint32 SWITCH_CORE_MULTIPORT_ADDR5;
	uint8 dummy100[60];
	uint32 SWITCH_CORE_MPORTVEC5;
	uint8 dummy101[60];
	uint32 SWITCH_CORE_ARL_BIN_FULL_CNTR;
	uint8 dummy102[28];
	uint32 SWITCH_CORE_ARL_BIN_FULL_FWD;
	uint8 dummy103[12];
	uint32 SWITCH_CORE_ARL_SEED;
	uint8 dummy104[76];
	uint32 SWITCH_CORE_ARLCTL_REG_SPARE0;
	uint8 dummy105[28];
	uint32 SWITCH_CORE_ARLCTL_REG_SPARE1;
	uint8 dummy106[92];
	uint32 SWITCH_CORE_ARL_TCAM_CTRL;
	uint8 dummy107[28];
	uint32 SWITCH_CORE_ARL_TCAM_STS;
	uint8 dummy108[28];
	uint32 SWITCH_CORE_ARL_TCAM_FULL_CNTR;
	uint8 dummy109[28];
	uint32 SWITCH_CORE_ARL_CPU_PORTMAP;
	uint8 dummy110[796];
	uint32 SWITCH_CORE_ARLA_RWCTL;
	uint8 dummy111[12];
	uint32 SWITCH_CORE_ARLA_MAC;
	uint8 dummy112[44];
	uint32 SWITCH_CORE_ARLA_VID;
	uint8 dummy113[60];
	uint32 SWITCH_CORE_ARLA_MACVID_ENTRY0;
	uint8 dummy114[60];
	uint32 SWITCH_CORE_ARLA_FWD_ENTRY0;
	uint8 dummy115[60];
	uint32 SWITCH_CORE_ARLA_MACVID_ENTRY1;
	uint8 dummy116[60];
	uint32 SWITCH_CORE_ARLA_FWD_ENTRY1;
	uint8 dummy117[60];
	uint32 SWITCH_CORE_ARLA_MACVID_ENTRY2;
	uint8 dummy118[60];
	uint32 SWITCH_CORE_ARLA_FWD_ENTRY2;
	uint8 dummy119[60];
	uint32 SWITCH_CORE_ARLA_MACVID_ENTRY3;
	uint8 dummy120[60];
	uint32 SWITCH_CORE_ARLA_FWD_ENTRY3;
	uint8 dummy121[60];
	uint64 SWITCH_CORE_ARLA_SRCH_CTL;
	uint32 SWITCH_CORE_ARLA_SRCH_ADR;
	uint8 dummy122[116];
	uint32 SWITCH_CORE_ARLA_SRCH_RSLT_0_MACVID;
	uint8 dummy123[60];
	uint32 SWITCH_CORE_ARLA_SRCH_RSLT_0;
	uint8 dummy124[60];
	uint32 SWITCH_CORE_ARLA_SRCH_RSLT_1_MACVID;
	uint8 dummy125[60];
	uint32 SWITCH_CORE_ARLA_SRCH_RSLT_1;
	uint8 dummy126[60];
	uint64 SWITCH_CORE_ARLA_VTBL_RWCTRL;
	uint32 SWITCH_CORE_ARLA_VTBL_ADDR;
	uint8 dummy127[12];
	uint32 SWITCH_CORE_ARLA_VTBL_ENTRY;
	uint8 dummy128[100];
	uint32 SWITCH_CORE_ARLACCS_REG_SPARE0;
	uint8 dummy129[28];
	uint32 SWITCH_CORE_ARLACCS_REG_SPARE1;
	uint8 dummy130[4956];
	uint64 SWITCH_CORE_MEM_CTRL;
	uint32 SWITCH_CORE_MEM_ADDR;
	uint8 dummy131[52];
	uint32 SWITCH_CORE_MEM_DEBUG_DATA_0_0;
	uint8 dummy132[60];
	uint32 SWITCH_CORE_MEM_DEBUG_DATA_0_1;
	uint8 dummy133[12];
	uint32 SWITCH_CORE_MEM_DEBUG_DATA_1_0;
	uint8 dummy134[60];
	uint32 SWITCH_CORE_MEM_DEBUG_DATA_1_1;
	uint8 dummy135[44];
	uint32 SWITCH_CORE_MEM_FRM_ADDR;
	uint8 dummy136[124];
	uint32 SWITCH_CORE_MEM_FRM_DATA0;
	uint8 dummy137[60];
	uint32 SWITCH_CORE_MEM_FRM_DATA1;
	uint8 dummy138[60];
	uint32 SWITCH_CORE_MEM_FRM_DATA2;
	uint8 dummy139[60];
	uint32 SWITCH_CORE_MEM_FRM_DATA3;
	uint8 dummy140[60];
	uint32 SWITCH_CORE_MEM_BTM_DATA0;
	uint8 dummy141[60];
	uint32 SWITCH_CORE_MEM_BTM_DATA1;
	uint8 dummy142[60];
	uint32 SWITCH_CORE_MEM_BFC_ADDR;
	uint8 dummy143[12];
	uint32 SWITCH_CORE_MEM_BFC_DATA;
	uint8 dummy144[108];
	uint64 SWITCH_CORE_PRS_FIFO_DEBUG_CTRL;
	uint32 SWITCH_CORE_PRS_FIFO_DEBUG_DATA;
	uint8 dummy145[116];
	uint32 SWITCH_CORE_MIBKILLOVR;
	uint8 dummy146[316];
	uint32 SWITCH_CORE_MEM_REG_SPARE0;
	uint8 dummy147[28];
	uint32 SWITCH_CORE_MEM_REG_SPARE1;
	uint8 dummy148[28];
	uint32 SWITCH_CORE_MEM_MISC_CTRL;
	uint8 dummy149[28];
	uint32 SWITCH_CORE_MEM_TEST_CTRL0;
	uint8 dummy150[28];
	uint32 SWITCH_CORE_MEM_TEST_CTRL1;
	uint8 dummy151[28];
	uint32 SWITCH_CORE_MEM_TEST_CTRL2;
	uint8 dummy152[28];
	uint32 SWITCH_CORE_MEM_TEST_CTRL3;
	uint8 dummy153[28];
	uint32 SWITCH_CORE_MEM_TEST_CTRL4;
	uint8 dummy154[28];
	uint32 SWITCH_CORE_MEM_TEST_CTRL5;
	uint8 dummy155[188];
	uint32 SWITCH_CORE_MEM_PSM_VDD_CTRL;
	uint8 dummy156[252];
	uint32 SWITCH_CORE_PORT0_DEBUG;
	uint8 dummy157[124];
	uint32 SWITCH_CORE_PORT1_DEBUG;
	uint8 dummy158[124];
	uint32 SWITCH_CORE_PORT2_DEBUG;
	uint8 dummy159[124];
	uint32 SWITCH_CORE_PORT3_DEBUG;
	uint8 dummy160[124];
	uint32 SWITCH_CORE_PORT4_DEBUG;
	uint8 dummy161[124];
	uint32 SWITCH_CORE_PORT5_DEBUG;
	uint8 dummy162[124];
	uint32 SWITCH_CORE_PORT6_DEBUG;
	uint8 dummy163[124];
	uint32 SWITCH_CORE_PORT7_DEBUG;
	uint8 dummy164[124];
	uint32 SWITCH_CORE_PORT8_DEBUG;
	uint8 dummy165[1020];
	uint32 SWITCH_CORE_FC_DIAG_CTRL;
	uint8 dummy166[12];
	uint64 SWITCH_CORE_FC_CTRL_MODE;
	uint64 SWITCH_CORE_FC_CTRL_PORT;
	uint32 SWITCH_CORE_FC_OOB_PAUSE_EN;
	uint8 dummy167[92];
	uint32 SWITCH_CORE_PAUSE_TIME_MAX;
	uint8 dummy168[12];
	uint32 SWITCH_CORE_PAUSE_TIME_MIN;
	uint8 dummy169[12];
	uint32 SWITCH_CORE_PAUSE_TIME_RESET_THD;
	uint8 dummy170[12];
	uint32 SWITCH_CORE_PAUSE_TIME_UPDATE_PERIOD;
	uint8 dummy171[12];
	uint32 SWITCH_CORE_PAUSE_TIME_DEFAULT;
	uint8 dummy172[12];
	uint32 SWITCH_CORE_FC_MCAST_DROP_CTRL;
	uint8 dummy173[12];
	uint32 SWITCH_CORE_FC_PAUSE_DROP_CTRL;
	uint8 dummy174[12];
	uint32 SWITCH_CORE_FC_TXQ_THD_PAUSE_OFF;
	uint8 dummy175[12];
	uint32 SWITCH_CORE_FC_RX_RUNOFF;
	uint8 dummy176[12];
	uint32 SWITCH_CORE_FC_RX_RSV_THD;
	uint8 dummy177[12];
	uint32 SWITCH_CORE_FC_RX_HYST_THD;
	uint8 dummy178[12];
	uint32 SWITCH_CORE_FC_RX_MAX_PTR;
	uint8 dummy179[12];
	uint32 SWITCH_CORE_FC_SPARE_ZERO_REG;
	uint8 dummy180[12];
	uint32 SWITCH_CORE_FC_SPARE_ONE_REG;
	uint8 dummy181[44];
	uint32 SWITCH_CORE_FC_MON_TX_Q0;
	uint8 dummy182[12];
	uint32 SWITCH_CORE_FC_MON_TX_Q1;
	uint8 dummy183[12];
	uint32 SWITCH_CORE_FC_MON_TX_Q2;
	uint8 dummy184[12];
	uint32 SWITCH_CORE_FC_MON_TX_Q3;
	uint8 dummy185[12];
	uint32 SWITCH_CORE_FC_MON_TX_Q4;
	uint8 dummy186[12];
	uint32 SWITCH_CORE_FC_MON_TX_Q5;
	uint8 dummy187[12];
	uint32 SWITCH_CORE_FC_MON_TX_Q6;
	uint8 dummy188[12];
	uint32 SWITCH_CORE_FC_MON_TX_Q7;
	uint8 dummy189[12];
	uint32 SWITCH_CORE_FC_PEAK_TX_Q0;
	uint8 dummy190[12];
	uint32 SWITCH_CORE_FC_PEAK_TX_Q1;
	uint8 dummy191[12];
	uint32 SWITCH_CORE_FC_PEAK_TX_Q2;
	uint8 dummy192[12];
	uint32 SWITCH_CORE_FC_PEAK_TX_Q3;
	uint8 dummy193[12];
	uint32 SWITCH_CORE_FC_PEAK_TX_Q4;
	uint8 dummy194[12];
	uint32 SWITCH_CORE_FC_PEAK_TX_Q5;
	uint8 dummy195[12];
	uint32 SWITCH_CORE_FC_PEAK_TX_Q6;
	uint8 dummy196[12];
	uint32 SWITCH_CORE_FC_PEAK_TX_Q7;
	uint8 dummy197[12];
	uint32 SWITCH_CORE_FC_PEAK_TOTAL_USED;
	uint8 dummy198[12];
	uint32 SWITCH_CORE_FC_TOTAL_USED;
	uint8 dummy199[12];
	uint32 SWITCH_CORE_FC_PEAK_RX_CNT;
	uint8 dummy200[12];
	uint32 SWITCH_CORE_FC_LINK_PORTMAP;
	uint8 dummy201[12];
	uint32 SWITCH_CORE_FC_GIGA_PORTMAP;
	uint8 dummy202[60];
	uint32 SWITCH_CORE_FC_CONG_PORTMAP_P0;
	uint8 dummy203[12];
	uint32 SWITCH_CORE_FC_CONG_PORTMAP_P1;
	uint8 dummy204[12];
	uint32 SWITCH_CORE_FC_CONG_PORTMAP_P2;
	uint8 dummy205[12];
	uint32 SWITCH_CORE_FC_CONG_PORTMAP_P3;
	uint8 dummy206[12];
	uint32 SWITCH_CORE_FC_CONG_PORTMAP_P4;
	uint8 dummy207[12];
	uint32 SWITCH_CORE_FC_CONG_PORTMAP_P5;
	uint8 dummy208[12];
	uint32 SWITCH_CORE_FC_CONG_PORTMAP_P6;
	uint8 dummy209[12];
	uint32 SWITCH_CORE_FC_CONG_PORTMAP_P7;
	uint8 dummy210[12];
	uint32 SWITCH_CORE_FC_CONG_PORTMAP_P8;
	uint8 dummy211[60];
	uint32 SWITCH_CORE_FC_PAUSE_HIS;
	uint8 dummy212[12];
	uint32 SWITCH_CORE_FC_TX_QUANTUM_PAUSE_HIS;
	uint8 dummy213[12];
	uint32 SWITCH_CORE_FC_RX_PAUSE_HIS;
	uint8 dummy214[12];
	uint32 SWITCH_CORE_FC_RXBUF_ERR_HIS;
	uint8 dummy215[12];
	uint32 SWITCH_CORE_FC_TXQ_CONG_PORTMAP_P0;
	uint8 dummy216[12];
	uint32 SWITCH_CORE_FC_TXQ_CONG_PORTMAP_P1;
	uint8 dummy217[12];
	uint32 SWITCH_CORE_FC_TXQ_CONG_PORTMAP_P2;
	uint8 dummy218[12];
	uint32 SWITCH_CORE_FC_TXQ_CONG_PORTMAP_P3;
	uint8 dummy219[12];
	uint32 SWITCH_CORE_FC_TXQ_CONG_PORTMAP_P4;
	uint8 dummy220[12];
	uint32 SWITCH_CORE_FC_TXQ_CONG_PORTMAP_P5;
	uint8 dummy221[12];
	uint32 SWITCH_CORE_FC_TXQ_CONG_PORTMAP_P6;
	uint8 dummy222[12];
	uint32 SWITCH_CORE_FC_TXQ_CONG_PORTMAP_P7;
	uint8 dummy223[12];
	uint32 SWITCH_CORE_FC_TXQ_CONG_PORTMAP_P8;
	uint8 dummy224[76];
	uint32 SWITCH_CORE_FC_TOTAL_CONG_PORTMAP_P0;
	uint8 dummy225[12];
	uint32 SWITCH_CORE_FC_TOTAL_CONG_PORTMAP_P1;
	uint8 dummy226[12];
	uint32 SWITCH_CORE_FC_TOTAL_CONG_PORTMAP_P2;
	uint8 dummy227[12];
	uint32 SWITCH_CORE_FC_TOTAL_CONG_PORTMAP_P3;
	uint8 dummy228[12];
	uint32 SWITCH_CORE_FC_TOTAL_CONG_PORTMAP_P4;
	uint8 dummy229[12];
	uint32 SWITCH_CORE_FC_TOTAL_CONG_PORTMAP_P5;
	uint8 dummy230[12];
	uint32 SWITCH_CORE_FC_TOTAL_CONG_PORTMAP_P6;
	uint8 dummy231[12];
	uint32 SWITCH_CORE_FC_TOTAL_CONG_PORTMAP_P7;
	uint8 dummy232[12];
	uint32 SWITCH_CORE_FC_TOTAL_CONG_PORTMAP_P8;
	uint8 dummy233[684];
	uint32 SWITCH_CORE_FC_LAN_TXQ_THD_RSV_Q0;
	uint8 dummy234[12];
	uint32 SWITCH_CORE_FC_LAN_TXQ_THD_RSV_Q1;
	uint8 dummy235[12];
	uint32 SWITCH_CORE_FC_LAN_TXQ_THD_RSV_Q2;
	uint8 dummy236[12];
	uint32 SWITCH_CORE_FC_LAN_TXQ_THD_RSV_Q3;
	uint8 dummy237[12];
	uint32 SWITCH_CORE_FC_LAN_TXQ_THD_RSV_Q4;
	uint8 dummy238[12];
	uint32 SWITCH_CORE_FC_LAN_TXQ_THD_RSV_Q5;
	uint8 dummy239[12];
	uint32 SWITCH_CORE_FC_LAN_TXQ_THD_RSV_Q6;
	uint8 dummy240[12];
	uint32 SWITCH_CORE_FC_LAN_TXQ_THD_RSV_Q7;
	uint8 dummy241[12];
	uint32 SWITCH_CORE_FC_LAN_TXQ_THD_HYST_Q0;
	uint8 dummy242[12];
	uint32 SWITCH_CORE_FC_LAN_TXQ_THD_HYST_Q1;
	uint8 dummy243[12];
	uint32 SWITCH_CORE_FC_LAN_TXQ_THD_HYST_Q2;
	uint8 dummy244[12];
	uint32 SWITCH_CORE_FC_LAN_TXQ_THD_HYST_Q3;
	uint8 dummy245[12];
	uint32 SWITCH_CORE_FC_LAN_TXQ_THD_HYST_Q4;
	uint8 dummy246[12];
	uint32 SWITCH_CORE_FC_LAN_TXQ_THD_HYST_Q5;
	uint8 dummy247[12];
	uint32 SWITCH_CORE_FC_LAN_TXQ_THD_HYST_Q6;
	uint8 dummy248[12];
	uint32 SWITCH_CORE_FC_LAN_TXQ_THD_HYST_Q7;
	uint8 dummy249[12];
	uint32 SWITCH_CORE_FC_LAN_TXQ_THD_PAUSE_Q0;
	uint8 dummy250[12];
	uint32 SWITCH_CORE_FC_LAN_TXQ_THD_PAUSE_Q1;
	uint8 dummy251[12];
	uint32 SWITCH_CORE_FC_LAN_TXQ_THD_PAUSE_Q2;
	uint8 dummy252[12];
	uint32 SWITCH_CORE_FC_LAN_TXQ_THD_PAUSE_Q3;
	uint8 dummy253[12];
	uint32 SWITCH_CORE_FC_LAN_TXQ_THD_PAUSE_Q4;
	uint8 dummy254[12];
	uint32 SWITCH_CORE_FC_LAN_TXQ_THD_PAUSE_Q5;
	uint8 dummy255[12];
	uint32 SWITCH_CORE_FC_LAN_TXQ_THD_PAUSE_Q6;
	uint8 dummy256[12];
	uint32 SWITCH_CORE_FC_LAN_TXQ_THD_PAUSE_Q7;
	uint8 dummy257[12];
	uint32 SWITCH_CORE_FC_LAN_TXQ_THD_DROP_Q0;
	uint8 dummy258[12];
	uint32 SWITCH_CORE_FC_LAN_TXQ_THD_DROP_Q1;
	uint8 dummy259[12];
	uint32 SWITCH_CORE_FC_LAN_TXQ_THD_DROP_Q2;
	uint8 dummy260[12];
	uint32 SWITCH_CORE_FC_LAN_TXQ_THD_DROP_Q3;
	uint8 dummy261[12];
	uint32 SWITCH_CORE_FC_LAN_TXQ_THD_DROP_Q4;
	uint8 dummy262[12];
	uint32 SWITCH_CORE_FC_LAN_TXQ_THD_DROP_Q5;
	uint8 dummy263[12];
	uint32 SWITCH_CORE_FC_LAN_TXQ_THD_DROP_Q6;
	uint8 dummy264[12];
	uint32 SWITCH_CORE_FC_LAN_TXQ_THD_DROP_Q7;
	uint8 dummy265[12];
	uint32 SWITCH_CORE_FC_LAN_TOTAL_THD_HYST_Q0;
	uint8 dummy266[12];
	uint32 SWITCH_CORE_FC_LAN_TOTAL_THD_HYST_Q1;
	uint8 dummy267[12];
	uint32 SWITCH_CORE_FC_LAN_TOTAL_THD_HYST_Q2;
	uint8 dummy268[12];
	uint32 SWITCH_CORE_FC_LAN_TOTAL_THD_HYST_Q3;
	uint8 dummy269[12];
	uint32 SWITCH_CORE_FC_LAN_TOTAL_THD_HYST_Q4;
	uint8 dummy270[12];
	uint32 SWITCH_CORE_FC_LAN_TOTAL_THD_HYST_Q5;
	uint8 dummy271[12];
	uint32 SWITCH_CORE_FC_LAN_TOTAL_THD_HYST_Q6;
	uint8 dummy272[12];
	uint32 SWITCH_CORE_FC_LAN_TOTAL_THD_HYST_Q7;
	uint8 dummy273[12];
	uint32 SWITCH_CORE_FC_LAN_TOTAL_THD_PAUSE_Q0;
	uint8 dummy274[12];
	uint32 SWITCH_CORE_FC_LAN_TOTAL_THD_PAUSE_Q1;
	uint8 dummy275[12];
	uint32 SWITCH_CORE_FC_LAN_TOTAL_THD_PAUSE_Q2;
	uint8 dummy276[12];
	uint32 SWITCH_CORE_FC_LAN_TOTAL_THD_PAUSE_Q3;
	uint8 dummy277[12];
	uint32 SWITCH_CORE_FC_LAN_TOTAL_THD_PAUSE_Q4;
	uint8 dummy278[12];
	uint32 SWITCH_CORE_FC_LAN_TOTAL_THD_PAUSE_Q5;
	uint8 dummy279[12];
	uint32 SWITCH_CORE_FC_LAN_TOTAL_THD_PAUSE_Q6;
	uint8 dummy280[12];
	uint32 SWITCH_CORE_FC_LAN_TOTAL_THD_PAUSE_Q7;
	uint8 dummy281[12];
	uint32 SWITCH_CORE_FC_LAN_TOTAL_THD_DROP_Q0;
	uint8 dummy282[12];
	uint32 SWITCH_CORE_FC_LAN_TOTAL_THD_DROP_Q1;
	uint8 dummy283[12];
	uint32 SWITCH_CORE_FC_LAN_TOTAL_THD_DROP_Q2;
	uint8 dummy284[12];
	uint32 SWITCH_CORE_FC_LAN_TOTAL_THD_DROP_Q3;
	uint8 dummy285[12];
	uint32 SWITCH_CORE_FC_LAN_TOTAL_THD_DROP_Q4;
	uint8 dummy286[12];
	uint32 SWITCH_CORE_FC_LAN_TOTAL_THD_DROP_Q5;
	uint8 dummy287[12];
	uint32 SWITCH_CORE_FC_LAN_TOTAL_THD_DROP_Q6;
	uint8 dummy288[12];
	uint32 SWITCH_CORE_FC_LAN_TOTAL_THD_DROP_Q7;
	uint8 dummy289[1164];
	uint32 SWITCH_CORE_P0_DEBUG_MUX;
	uint8 dummy290[28];
	uint32 SWITCH_CORE_P1_DEBUG_MUX;
	uint8 dummy291[28];
	uint32 SWITCH_CORE_P2_DEBUG_MUX;
	uint8 dummy292[28];
	uint32 SWITCH_CORE_P3_DEBUG_MUX;
	uint8 dummy293[28];
	uint32 SWITCH_CORE_P4_DEBUG_MUX;
	uint8 dummy294[28];
	uint32 SWITCH_CORE_P5_DEBUG_MUX;
	uint8 dummy295[28];
	uint32 SWITCH_CORE_P6_DEBUG_MUX;
	uint8 dummy296[28];
	uint32 SWITCH_CORE_DEBUG_MUX_P7;
	uint8 dummy297[28];
	uint32 SWITCH_CORE_DEBUG_MUX_IMP;
	uint8 dummy298[28];
	uint32 SWITCH_CORE_CFP_DEBUG_BUS_0;
	uint8 dummy299[28];
	uint32 SWITCH_CORE_CFP_DEBUG_BUS_1;
	uint8 dummy300[28];
	uint32 SWITCH_CORE_WRED_DEBUG_0;
	uint8 dummy301[28];
	uint32 SWITCH_CORE_WRED_DEBUG_1;
	uint8 dummy302[28];
	uint32 SWITCH_CORE_TOP_MISC_DEBUG_0;
	uint8 dummy303[28];
	uint32 SWITCH_CORE_TOP_MISC_DEBUG_1;
	uint8 dummy304[28];
	uint32 SWITCH_CORE_DIAGREG_BUFCON;
	uint8 dummy305[28];
	uint32 SWITCH_CORE_TESTBUS_P1588;
	uint8 dummy306[28];
	uint32 SWITCH_CORE_FLOWCON_DEBUG_BUS;
	uint8 dummy307[1500];
	uint32 SWITCH_CORE_FC_IMP0_TXQ_THD_RSV_Q0;
	uint8 dummy308[12];
	uint32 SWITCH_CORE_FC_IMP0_TXQ_THD_RSV_Q1;
	uint8 dummy309[12];
	uint32 SWITCH_CORE_FC_IMP0_TXQ_THD_RSV_Q2;
	uint8 dummy310[12];
	uint32 SWITCH_CORE_FC_IMP0_TXQ_THD_RSV_Q3;
	uint8 dummy311[12];
	uint32 SWITCH_CORE_FC_IMP0_TXQ_THD_RSV_Q4;
	uint8 dummy312[12];
	uint32 SWITCH_CORE_FC_IMP0_TXQ_THD_RSV_Q5;
	uint8 dummy313[12];
	uint32 SWITCH_CORE_FC_IMP0_TXQ_THD_RSV_Q6;
	uint8 dummy314[12];
	uint32 SWITCH_CORE_FC_IMP0_TXQ_THD_RSV_Q7;
	uint8 dummy315[12];
	uint32 SWITCH_CORE_FC_IMP0_TXQ_THD_HYST_Q0;
	uint8 dummy316[12];
	uint32 SWITCH_CORE_FC_IMP0_TXQ_THD_HYST_Q1;
	uint8 dummy317[12];
	uint32 SWITCH_CORE_FC_IMP0_TXQ_THD_HYST_Q2;
	uint8 dummy318[12];
	uint32 SWITCH_CORE_FC_IMP0_TXQ_THD_HYST_Q3;
	uint8 dummy319[12];
	uint32 SWITCH_CORE_FC_IMP0_TXQ_THD_HYST_Q4;
	uint8 dummy320[12];
	uint32 SWITCH_CORE_FC_IMP0_TXQ_THD_HYST_Q5;
	uint8 dummy321[12];
	uint32 SWITCH_CORE_FC_IMP0_TXQ_THD_HYST_Q6;
	uint8 dummy322[12];
	uint32 SWITCH_CORE_FC_IMP0_TXQ_THD_HYST_Q7;
	uint8 dummy323[12];
	uint32 SWITCH_CORE_FC_IMP0_TXQ_THD_PAUSE_Q0;
	uint8 dummy324[12];
	uint32 SWITCH_CORE_FC_IMP0_TXQ_THD_PAUSE_Q1;
	uint8 dummy325[12];
	uint32 SWITCH_CORE_FC_IMP0_TXQ_THD_PAUSE_Q2;
	uint8 dummy326[12];
	uint32 SWITCH_CORE_FC_IMP0_TXQ_THD_PAUSE_Q3;
	uint8 dummy327[12];
	uint32 SWITCH_CORE_FC_IMP0_TXQ_THD_PAUSE_Q4;
	uint8 dummy328[12];
	uint32 SWITCH_CORE_FC_IMP0_TXQ_THD_PAUSE_Q5;
	uint8 dummy329[12];
	uint32 SWITCH_CORE_FC_IMP0_TXQ_THD_PAUSE_Q6;
	uint8 dummy330[12];
	uint32 SWITCH_CORE_FC_IMP0_TXQ_THD_PAUSE_Q7;
	uint8 dummy331[12];
	uint32 SWITCH_CORE_FC_IMP0_TXQ_THD_DROP_Q0;
	uint8 dummy332[12];
	uint32 SWITCH_CORE_FC_IMP0_TXQ_THD_DROP_Q1;
	uint8 dummy333[12];
	uint32 SWITCH_CORE_FC_IMP0_TXQ_THD_DROP_Q2;
	uint8 dummy334[12];
	uint32 SWITCH_CORE_FC_IMP0_TXQ_THD_DROP_Q3;
	uint8 dummy335[12];
	uint32 SWITCH_CORE_FC_IMP0_TXQ_THD_DROP_Q4;
	uint8 dummy336[12];
	uint32 SWITCH_CORE_FC_IMP0_TXQ_THD_DROP_Q5;
	uint8 dummy337[12];
	uint32 SWITCH_CORE_FC_IMP0_TXQ_THD_DROP_Q6;
	uint8 dummy338[12];
	uint32 SWITCH_CORE_FC_IMP0_TXQ_THD_DROP_Q7;
	uint8 dummy339[12];
	uint32 SWITCH_CORE_FC_IMP0_TOTAL_THD_HYST_Q0;
	uint8 dummy340[12];
	uint32 SWITCH_CORE_FC_IMP0_TOTAL_THD_HYST_Q1;
	uint8 dummy341[12];
	uint32 SWITCH_CORE_FC_IMP0_TOTAL_THD_HYST_Q2;
	uint8 dummy342[12];
	uint32 SWITCH_CORE_FC_IMP0_TOTAL_THD_HYST_Q3;
	uint8 dummy343[12];
	uint32 SWITCH_CORE_FC_IMP0_TOTAL_THD_HYST_Q4;
	uint8 dummy344[12];
	uint32 SWITCH_CORE_FC_IMP0_TOTAL_THD_HYST_Q5;
	uint8 dummy345[12];
	uint32 SWITCH_CORE_FC_IMP0_TOTAL_THD_HYST_Q6;
	uint8 dummy346[12];
	uint32 SWITCH_CORE_FC_IMP0_TOTAL_THD_HYST_Q7;
	uint8 dummy347[12];
	uint32 SWITCH_CORE_FC_IMP0_TOTAL_THD_PAUSE_Q0;
	uint8 dummy348[12];
	uint32 SWITCH_CORE_FC_IMP0_TOTAL_THD_PAUSE_Q1;
	uint8 dummy349[12];
	uint32 SWITCH_CORE_FC_IMP0_TOTAL_THD_PAUSE_Q2;
	uint8 dummy350[12];
	uint32 SWITCH_CORE_FC_IMP0_TOTAL_THD_PAUSE_Q3;
	uint8 dummy351[12];
	uint32 SWITCH_CORE_FC_IMP0_TOTAL_THD_PAUSE_Q4;
	uint8 dummy352[12];
	uint32 SWITCH_CORE_FC_IMP0_TOTAL_THD_PAUSE_Q5;
	uint8 dummy353[12];
	uint32 SWITCH_CORE_FC_IMP0_TOTAL_THD_PAUSE_Q6;
	uint8 dummy354[12];
	uint32 SWITCH_CORE_FC_IMP0_TOTAL_THD_PAUSE_Q7;
	uint8 dummy355[12];
	uint32 SWITCH_CORE_FC_IMP0_TOTAL_THD_DROP_Q0;
	uint8 dummy356[12];
	uint32 SWITCH_CORE_FC_IMP0_TOTAL_THD_DROP_Q1;
	uint8 dummy357[12];
	uint32 SWITCH_CORE_FC_IMP0_TOTAL_THD_DROP_Q2;
	uint8 dummy358[12];
	uint32 SWITCH_CORE_FC_IMP0_TOTAL_THD_DROP_Q3;
	uint8 dummy359[12];
	uint32 SWITCH_CORE_FC_IMP0_TOTAL_THD_DROP_Q4;
	uint8 dummy360[12];
	uint32 SWITCH_CORE_FC_IMP0_TOTAL_THD_DROP_Q5;
	uint8 dummy361[12];
	uint32 SWITCH_CORE_FC_IMP0_TOTAL_THD_DROP_Q6;
	uint8 dummy362[12];
	uint32 SWITCH_CORE_FC_IMP0_TOTAL_THD_DROP_Q7;
	uint8 dummy363[12];
	uint32 SWITCH_CORE_FC_IMP0_REG_SPARE0;
	uint8 dummy364[12];
	uint32 SWITCH_CORE_FC_IMP0_REG_SPARE1;
	uint8 dummy365[1132];
	uint32 SWITCH_CORE_FC_WAN_IMP1_TXQ_THD_RSV_Q0;
	uint8 dummy366[12];
	uint32 SWITCH_CORE_FC_WAN_IMP1_TXQ_THD_RSV_Q1;
	uint8 dummy367[12];
	uint32 SWITCH_CORE_FC_WAN_IMP1_TXQ_THD_RSV_Q2;
	uint8 dummy368[12];
	uint32 SWITCH_CORE_FC_WAN_IMP1_TXQ_THD_RSV_Q3;
	uint8 dummy369[12];
	uint32 SWITCH_CORE_FC_WAN_IMP1_TXQ_THD_RSV_Q4;
	uint8 dummy370[12];
	uint32 SWITCH_CORE_FC_WAN_IMP1_TXQ_THD_RSV_Q5;
	uint8 dummy371[12];
	uint32 SWITCH_CORE_FC_WAN_IMP1_TXQ_THD_RSV_Q6;
	uint8 dummy372[12];
	uint32 SWITCH_CORE_FC_WAN_IMP1_TXQ_THD_RSV_Q7;
	uint8 dummy373[12];
	uint32 SWITCH_CORE_FC_WAN_IMP1_TXQ_THD_HYST_Q0;
	uint8 dummy374[12];
	uint32 SWITCH_CORE_FC_WAN_IMP1_TXQ_THD_HYST_Q1;
	uint8 dummy375[12];
	uint32 SWITCH_CORE_FC_WAN_IMP1_TXQ_THD_HYST_Q2;
	uint8 dummy376[12];
	uint32 SWITCH_CORE_FC_WAN_IMP1_TXQ_THD_HYST_Q3;
	uint8 dummy377[12];
	uint32 SWITCH_CORE_FC_WAN_IMP1_TXQ_THD_HYST_Q4;
	uint8 dummy378[12];
	uint32 SWITCH_CORE_FC_WAN_IMP1_TXQ_THD_HYST_Q5;
	uint8 dummy379[12];
	uint32 SWITCH_CORE_FC_WAN_IMP1_TXQ_THD_HYST_Q6;
	uint8 dummy380[12];
	uint32 SWITCH_CORE_FC_WAN_IMP1_TXQ_THD_HYST_Q7;
	uint8 dummy381[12];
	uint32 SWITCH_CORE_FC_WAN_IMP1_TXQ_THD_PAUSE_Q0;
	uint8 dummy382[12];
	uint32 SWITCH_CORE_FC_WAN_IMP1_TXQ_THD_PAUSE_Q1;
	uint8 dummy383[12];
	uint32 SWITCH_CORE_FC_WAN_IMP1_TXQ_THD_PAUSE_Q2;
	uint8 dummy384[12];
	uint32 SWITCH_CORE_FC_WAN_IMP1_TXQ_THD_PAUSE_Q3;
	uint8 dummy385[12];
	uint32 SWITCH_CORE_FC_WAN_IMP1_TXQ_THD_PAUSE_Q4;
	uint8 dummy386[12];
	uint32 SWITCH_CORE_FC_WAN_IMP1_TXQ_THD_PAUSE_Q5;
	uint8 dummy387[12];
	uint32 SWITCH_CORE_FC_WAN_IMP1_TXQ_THD_PAUSE_Q6;
	uint8 dummy388[12];
	uint32 SWITCH_CORE_FC_WAN_IMP1_TXQ_THD_PAUSE_Q7;
	uint8 dummy389[12];
	uint32 SWITCH_CORE_FC_WAN_IMP1_TXQ_THD_DROP_Q0;
	uint8 dummy390[12];
	uint32 SWITCH_CORE_FC_WAN_IMP1_TXQ_THD_DROP_Q1;
	uint8 dummy391[12];
	uint32 SWITCH_CORE_FC_WAN_IMP1_TXQ_THD_DROP_Q2;
	uint8 dummy392[12];
	uint32 SWITCH_CORE_FC_WAN_IMP1_TXQ_THD_DROP_Q3;
	uint8 dummy393[12];
	uint32 SWITCH_CORE_FC_WAN_IMP1_TXQ_THD_DROP_Q4;
	uint8 dummy394[12];
	uint32 SWITCH_CORE_FC_WAN_IMP1_TXQ_THD_DROP_Q5;
	uint8 dummy395[12];
	uint32 SWITCH_CORE_FC_WAN_IMP1_TXQ_THD_DROP_Q6;
	uint8 dummy396[12];
	uint32 SWITCH_CORE_FC_WAN_IMP1_TXQ_THD_DROP_Q7;
	uint8 dummy397[12];
	uint32 SWITCH_CORE_FC_WAN_IMP1_TOTAL_THD_HYST_Q0;
	uint8 dummy398[12];
	uint32 SWITCH_CORE_FC_WAN_IMP1_TOTAL_THD_HYST_Q1;
	uint8 dummy399[12];
	uint32 SWITCH_CORE_FC_WAN_IMP1_TOTAL_THD_HYST_Q2;
	uint8 dummy400[12];
	uint32 SWITCH_CORE_FC_WAN_IMP1_TOTAL_THD_HYST_Q3;
	uint8 dummy401[12];
	uint32 SWITCH_CORE_FC_WAN_IMP1_TOTAL_THD_HYST_Q4;
	uint8 dummy402[12];
	uint32 SWITCH_CORE_FC_WAN_IMP1_TOTAL_THD_HYST_Q5;
	uint8 dummy403[12];
	uint32 SWITCH_CORE_FC_WAN_IMP1_TOTAL_THD_HYST_Q6;
	uint8 dummy404[12];
	uint32 SWITCH_CORE_FC_WAN_IMP1_TOTAL_THD_HYST_Q7;
	uint8 dummy405[12];
	uint32 SWITCH_CORE_FC_WAN_IMP1_TOTAL_THD_PAUSE_Q0;
	uint8 dummy406[12];
	uint32 SWITCH_CORE_FC_WAN_IMP1_TOTAL_THD_PAUSE_Q1;
	uint8 dummy407[12];
	uint32 SWITCH_CORE_FC_WAN_IMP1_TOTAL_THD_PAUSE_Q2;
	uint8 dummy408[12];
	uint32 SWITCH_CORE_FC_WAN_IMP1_TOTAL_THD_PAUSE_Q3;
	uint8 dummy409[12];
	uint32 SWITCH_CORE_FC_WAN_IMP1_TOTAL_THD_PAUSE_Q4;
	uint8 dummy410[12];
	uint32 SWITCH_CORE_FC_WAN_IMP1_TOTAL_THD_PAUSE_Q5;
	uint8 dummy411[12];
	uint32 SWITCH_CORE_FC_WAN_IMP1_TOTAL_THD_PAUSE_Q6;
	uint8 dummy412[12];
	uint32 SWITCH_CORE_FC_WAN_IMP1_TOTAL_THD_PAUSE_Q7;
	uint8 dummy413[12];
	uint32 SWITCH_CORE_FC_WAN_IMP1_TOTAL_THD_DROP_Q0;
	uint8 dummy414[12];
	uint32 SWITCH_CORE_FC_WAN_IMP1_TOTAL_THD_DROP_Q1;
	uint8 dummy415[12];
	uint32 SWITCH_CORE_FC_WAN_IMP1_TOTAL_THD_DROP_Q2;
	uint8 dummy416[12];
	uint32 SWITCH_CORE_FC_WAN_IMP1_TOTAL_THD_DROP_Q3;
	uint8 dummy417[12];
	uint32 SWITCH_CORE_FC_WAN_IMP1_TOTAL_THD_DROP_Q4;
	uint8 dummy418[12];
	uint32 SWITCH_CORE_FC_WAN_IMP1_TOTAL_THD_DROP_Q5;
	uint8 dummy419[12];
	uint32 SWITCH_CORE_FC_WAN_IMP1_TOTAL_THD_DROP_Q6;
	uint8 dummy420[12];
	uint32 SWITCH_CORE_FC_WAN_IMP1_TOTAL_THD_DROP_Q7;
	uint8 dummy421[12];
	uint32 SWITCH_CORE_FC_WAN_IMP1_REG_SPARE0;
	uint8 dummy422[12];
	uint32 SWITCH_CORE_FC_WAN_IMP1_REG_SPARE1;
	uint8 dummy423[35948];
	uint32 SWITCH_CORE_TxOctets_P0;
	uint8 dummy424[60];
	uint32 SWITCH_CORE_TxDropPkts_P0;
	uint8 dummy425[28];
	uint32 SWITCH_CORE_TxQPKTQ0_P0;
	uint8 dummy426[28];
	uint32 SWITCH_CORE_TxBroadcastPkts_P0;
	uint8 dummy427[28];
	uint32 SWITCH_CORE_TxMulticastPkts_P0;
	uint8 dummy428[28];
	uint32 SWITCH_CORE_TxUnicastPkts_P0;
	uint8 dummy429[28];
	uint32 SWITCH_CORE_TxCollisions_P0;
	uint8 dummy430[28];
	uint32 SWITCH_CORE_TxSingleCollision_P0;
	uint8 dummy431[28];
	uint32 SWITCH_CORE_TxMultipleCollision_P0;
	uint8 dummy432[28];
	uint32 SWITCH_CORE_TxDeferredTransmit_P0;
	uint8 dummy433[28];
	uint32 SWITCH_CORE_TxLateCollision_P0;
	uint8 dummy434[28];
	uint32 SWITCH_CORE_TxExcessiveCollision_P0;
	uint8 dummy435[28];
	uint32 SWITCH_CORE_TxFrameInDisc_P0;
	uint8 dummy436[28];
	uint32 SWITCH_CORE_TxPausePkts_P0;
	uint8 dummy437[28];
	uint32 SWITCH_CORE_TxQPKTQ1_P0;
	uint8 dummy438[28];
	uint32 SWITCH_CORE_TxQPKTQ2_P0;
	uint8 dummy439[28];
	uint32 SWITCH_CORE_TxQPKTQ3_P0;
	uint8 dummy440[28];
	uint32 SWITCH_CORE_TxQPKTQ4_P0;
	uint8 dummy441[28];
	uint32 SWITCH_CORE_TxQPKTQ5_P0;
	uint8 dummy442[28];
	uint32 SWITCH_CORE_RxOctets_P0;
	uint8 dummy443[60];
	uint32 SWITCH_CORE_RxUndersizePkts_P0;
	uint8 dummy444[28];
	uint32 SWITCH_CORE_RxPausePkts_P0;
	uint8 dummy445[28];
	uint32 SWITCH_CORE_RxPkts64Octets_P0;
	uint8 dummy446[28];
	uint32 SWITCH_CORE_RxPkts65to127Octets_P0;
	uint8 dummy447[28];
	uint32 SWITCH_CORE_RxPkts128to255Octets_P0;
	uint8 dummy448[28];
	uint32 SWITCH_CORE_RxPkts256to511Octets_P0;
	uint8 dummy449[28];
	uint32 SWITCH_CORE_RxPkts512to1023Octets_P0;
	uint8 dummy450[28];
	uint32 SWITCH_CORE_RxPkts1024toMaxPktOctets_P0;
	uint8 dummy451[28];
	uint32 SWITCH_CORE_RxOversizePkts_P0;
	uint8 dummy452[28];
	uint32 SWITCH_CORE_RxJabbers_P0;
	uint8 dummy453[28];
	uint32 SWITCH_CORE_RxAlignmentErrors_P0;
	uint8 dummy454[28];
	uint32 SWITCH_CORE_RxFCSErrors_P0;
	uint8 dummy455[28];
	uint32 SWITCH_CORE_RxGoodOctets_P0;
	uint8 dummy456[60];
	uint32 SWITCH_CORE_RxDropPkts_P0;
	uint8 dummy457[28];
	uint32 SWITCH_CORE_RxUnicastPkts_P0;
	uint8 dummy458[28];
	uint32 SWITCH_CORE_RxMulticastPkts_P0;
	uint8 dummy459[28];
	uint32 SWITCH_CORE_RxBroadcastPkts_P0;
	uint8 dummy460[28];
	uint32 SWITCH_CORE_RxSAChanges_P0;
	uint8 dummy461[28];
	uint32 SWITCH_CORE_RxFragments_P0;
	uint8 dummy462[28];
	uint32 SWITCH_CORE_RxJumboPkt_P0;
	uint8 dummy463[28];
	uint32 SWITCH_CORE_RxSymblErr_P0;
	uint8 dummy464[28];
	uint32 SWITCH_CORE_InRangeErrCount_P0;
	uint8 dummy465[28];
	uint32 SWITCH_CORE_OutRangeErrCount_P0;
	uint8 dummy466[28];
	uint32 SWITCH_CORE_EEE_LPI_EVENT_P0;
	uint8 dummy467[28];
	uint32 SWITCH_CORE_EEE_LPI_DURATION_P0;
	uint8 dummy468[28];
	uint32 SWITCH_CORE_RxDiscard_P0;
	uint8 dummy469[60];
	uint32 SWITCH_CORE_TxQPKTQ6_P0;
	uint8 dummy470[28];
	uint32 SWITCH_CORE_TxQPKTQ7_P0;
	uint8 dummy471[28];
	uint32 SWITCH_CORE_TxPkts64Octets_P0;
	uint8 dummy472[28];
	uint32 SWITCH_CORE_TxPkts65to127Octets_P0;
	uint8 dummy473[28];
	uint32 SWITCH_CORE_TxPkts128to255Octets_P0;
	uint8 dummy474[28];
	uint32 SWITCH_CORE_TxPkts256to511Octets_P0;
	uint8 dummy475[28];
	uint32 SWITCH_CORE_TxPkts512to1023Octets_P0;
	uint8 dummy476[28];
	uint32 SWITCH_CORE_TxPkts1024toMaxPktOctets_P0;
	uint8 dummy477[220];
	uint32 SWITCH_CORE_TxOctets_P1;
	uint8 dummy478[60];
	uint32 SWITCH_CORE_TxDropPkts_P1;
	uint8 dummy479[28];
	uint32 SWITCH_CORE_TxQPKTQ0_P1;
	uint8 dummy480[28];
	uint32 SWITCH_CORE_TxBroadcastPkts_P1;
	uint8 dummy481[28];
	uint32 SWITCH_CORE_TxMulticastPkts_P1;
	uint8 dummy482[28];
	uint32 SWITCH_CORE_TxUnicastPkts_P1;
	uint8 dummy483[28];
	uint32 SWITCH_CORE_TxCollisions_P1;
	uint8 dummy484[28];
	uint32 SWITCH_CORE_TxSingleCollision_P1;
	uint8 dummy485[28];
	uint32 SWITCH_CORE_TxMultipleCollision_P1;
	uint8 dummy486[28];
	uint32 SWITCH_CORE_TxDeferredTransmit_P1;
	uint8 dummy487[28];
	uint32 SWITCH_CORE_TxLateCollision_P1;
	uint8 dummy488[28];
	uint32 SWITCH_CORE_TxExcessiveCollision_P1;
	uint8 dummy489[28];
	uint32 SWITCH_CORE_TxFrameInDisc_P1;
	uint8 dummy490[28];
	uint32 SWITCH_CORE_TxPausePkts_P1;
	uint8 dummy491[28];
	uint32 SWITCH_CORE_TxQPKTQ1_P1;
	uint8 dummy492[28];
	uint32 SWITCH_CORE_TxQPKTQ2_P1;
	uint8 dummy493[28];
	uint32 SWITCH_CORE_TxQPKTQ3_P1;
	uint8 dummy494[28];
	uint32 SWITCH_CORE_TxQPKTQ4_P1;
	uint8 dummy495[28];
	uint32 SWITCH_CORE_TxQPKTQ5_P1;
	uint8 dummy496[28];
	uint32 SWITCH_CORE_RxOctets_P1;
	uint8 dummy497[60];
	uint32 SWITCH_CORE_RxUndersizePkts_P1;
	uint8 dummy498[28];
	uint32 SWITCH_CORE_RxPausePkts_P1;
	uint8 dummy499[28];
	uint32 SWITCH_CORE_RxPkts64Octets_P1;
	uint8 dummy500[28];
	uint32 SWITCH_CORE_RxPkts65to127Octets_P1;
	uint8 dummy501[28];
	uint32 SWITCH_CORE_RxPkts128to255Octets_P1;
	uint8 dummy502[28];
	uint32 SWITCH_CORE_RxPkts256to511Octets_P1;
	uint8 dummy503[28];
	uint32 SWITCH_CORE_RxPkts512to1023Octets_P1;
	uint8 dummy504[28];
	uint32 SWITCH_CORE_RxPkts1024toMaxPktOctets_P1;
	uint8 dummy505[28];
	uint32 SWITCH_CORE_RxOversizePkts_P1;
	uint8 dummy506[28];
	uint32 SWITCH_CORE_RxJabbers_P1;
	uint8 dummy507[28];
	uint32 SWITCH_CORE_RxAlignmentErrors_P1;
	uint8 dummy508[28];
	uint32 SWITCH_CORE_RxFCSErrors_P1;
	uint8 dummy509[28];
	uint32 SWITCH_CORE_RxGoodOctets_P1;
	uint8 dummy510[60];
	uint32 SWITCH_CORE_RxDropPkts_P1;
	uint8 dummy511[28];
	uint32 SWITCH_CORE_RxUnicastPkts_P1;
	uint8 dummy512[28];
	uint32 SWITCH_CORE_RxMulticastPkts_P1;
	uint8 dummy513[28];
	uint32 SWITCH_CORE_RxBroadcastPkts_P1;
	uint8 dummy514[28];
	uint32 SWITCH_CORE_RxSAChanges_P1;
	uint8 dummy515[28];
	uint32 SWITCH_CORE_RxFragments_P1;
	uint8 dummy516[28];
	uint32 SWITCH_CORE_RxJumboPkt_P1;
	uint8 dummy517[28];
	uint32 SWITCH_CORE_RxSymblErr_P1;
	uint8 dummy518[28];
	uint32 SWITCH_CORE_InRangeErrCount_P1;
	uint8 dummy519[28];
	uint32 SWITCH_CORE_OutRangeErrCount_P1;
	uint8 dummy520[28];
	uint32 SWITCH_CORE_EEE_LPI_EVENT_P1;
	uint8 dummy521[28];
	uint32 SWITCH_CORE_EEE_LPI_DURATION_P1;
	uint8 dummy522[28];
	uint32 SWITCH_CORE_RxDiscard_P1;
	uint8 dummy523[60];
	uint32 SWITCH_CORE_TxQPKTQ6_P1;
	uint8 dummy524[28];
	uint32 SWITCH_CORE_TxQPKTQ7_P1;
	uint8 dummy525[28];
	uint32 SWITCH_CORE_TxPkts64Octets_P1;
	uint8 dummy526[28];
	uint32 SWITCH_CORE_TxPkts65to127Octets_P1;
	uint8 dummy527[28];
	uint32 SWITCH_CORE_TxPkts128to255Octets_P1;
	uint8 dummy528[28];
	uint32 SWITCH_CORE_TxPkts256to511Octets_P1;
	uint8 dummy529[28];
	uint32 SWITCH_CORE_TxPkts512to1023Octets_P1;
	uint8 dummy530[28];
	uint32 SWITCH_CORE_TxPkts1024toMaxPktOctets_P1;
	uint8 dummy531[220];
	uint32 SWITCH_CORE_TxOctets_P2;
	uint8 dummy532[60];
	uint32 SWITCH_CORE_TxDropPkts_P2;
	uint8 dummy533[28];
	uint32 SWITCH_CORE_TxQPKTQ0_P2;
	uint8 dummy534[28];
	uint32 SWITCH_CORE_TxBroadcastPkts_P2;
	uint8 dummy535[28];
	uint32 SWITCH_CORE_TxMulticastPkts_P2;
	uint8 dummy536[28];
	uint32 SWITCH_CORE_TxUnicastPkts_P2;
	uint8 dummy537[28];
	uint32 SWITCH_CORE_TxCollisions_P2;
	uint8 dummy538[28];
	uint32 SWITCH_CORE_TxSingleCollision_P2;
	uint8 dummy539[28];
	uint32 SWITCH_CORE_TxMultipleCollision_P2;
	uint8 dummy540[28];
	uint32 SWITCH_CORE_TxDeferredTransmit_P2;
	uint8 dummy541[28];
	uint32 SWITCH_CORE_TxLateCollision_P2;
	uint8 dummy542[28];
	uint32 SWITCH_CORE_TxExcessiveCollision_P2;
	uint8 dummy543[28];
	uint32 SWITCH_CORE_TxFrameInDisc_P2;
	uint8 dummy544[28];
	uint32 SWITCH_CORE_TxPausePkts_P2;
	uint8 dummy545[28];
	uint32 SWITCH_CORE_TxQPKTQ1_P2;
	uint8 dummy546[28];
	uint32 SWITCH_CORE_TxQPKTQ2_P2;
	uint8 dummy547[28];
	uint32 SWITCH_CORE_TxQPKTQ3_P2;
	uint8 dummy548[28];
	uint32 SWITCH_CORE_TxQPKTQ4_P2;
	uint8 dummy549[28];
	uint32 SWITCH_CORE_TxQPKTQ5_P2;
	uint8 dummy550[28];
	uint32 SWITCH_CORE_RxOctets_P2;
	uint8 dummy551[60];
	uint32 SWITCH_CORE_RxUndersizePkts_P2;
	uint8 dummy552[28];
	uint32 SWITCH_CORE_RxPausePkts_P2;
	uint8 dummy553[28];
	uint32 SWITCH_CORE_RxPkts64Octets_P2;
	uint8 dummy554[28];
	uint32 SWITCH_CORE_RxPkts65to127Octets_P2;
	uint8 dummy555[28];
	uint32 SWITCH_CORE_RxPkts128to255Octets_P2;
	uint8 dummy556[28];
	uint32 SWITCH_CORE_RxPkts256to511Octets_P2;
	uint8 dummy557[28];
	uint32 SWITCH_CORE_RxPkts512to1023Octets_P2;
	uint8 dummy558[28];
	uint32 SWITCH_CORE_RxPkts1024toMaxPktOctets_P2;
	uint8 dummy559[28];
	uint32 SWITCH_CORE_RxOversizePkts_P2;
	uint8 dummy560[28];
	uint32 SWITCH_CORE_RxJabbers_P2;
	uint8 dummy561[28];
	uint32 SWITCH_CORE_RxAlignmentErrors_P2;
	uint8 dummy562[28];
	uint32 SWITCH_CORE_RxFCSErrors_P2;
	uint8 dummy563[28];
	uint32 SWITCH_CORE_RxGoodOctets_P2;
	uint8 dummy564[60];
	uint32 SWITCH_CORE_RxDropPkts_P2;
	uint8 dummy565[28];
	uint32 SWITCH_CORE_RxUnicastPkts_P2;
	uint8 dummy566[28];
	uint32 SWITCH_CORE_RxMulticastPkts_P2;
	uint8 dummy567[28];
	uint32 SWITCH_CORE_RxBroadcastPkts_P2;
	uint8 dummy568[28];
	uint32 SWITCH_CORE_RxSAChanges_P2;
	uint8 dummy569[28];
	uint32 SWITCH_CORE_RxFragments_P2;
	uint8 dummy570[28];
	uint32 SWITCH_CORE_RxJumboPkt_P2;
	uint8 dummy571[28];
	uint32 SWITCH_CORE_RxSymblErr_P2;
	uint8 dummy572[28];
	uint32 SWITCH_CORE_InRangeErrCount_P2;
	uint8 dummy573[28];
	uint32 SWITCH_CORE_OutRangeErrCount_P2;
	uint8 dummy574[28];
	uint32 SWITCH_CORE_EEE_LPI_EVENT_P2;
	uint8 dummy575[28];
	uint32 SWITCH_CORE_EEE_LPI_DURATION_P2;
	uint8 dummy576[28];
	uint32 SWITCH_CORE_RxDiscard_P2;
	uint8 dummy577[60];
	uint32 SWITCH_CORE_TxQPKTQ6_P2;
	uint8 dummy578[28];
	uint32 SWITCH_CORE_TxQPKTQ7_P2;
	uint8 dummy579[28];
	uint32 SWITCH_CORE_TxPkts64Octets_P2;
	uint8 dummy580[28];
	uint32 SWITCH_CORE_TxPkts65to127Octets_P2;
	uint8 dummy581[28];
	uint32 SWITCH_CORE_TxPkts128to255Octets_P2;
	uint8 dummy582[28];
	uint32 SWITCH_CORE_TxPkts256to511Octets_P2;
	uint8 dummy583[28];
	uint32 SWITCH_CORE_TxPkts512to1023Octets_P2;
	uint8 dummy584[28];
	uint32 SWITCH_CORE_TxPkts1024toMaxPktOctets_P2;
	uint8 dummy585[220];
	uint32 SWITCH_CORE_TxOctets_P3;
	uint8 dummy586[60];
	uint32 SWITCH_CORE_TxDropPkts_P3;
	uint8 dummy587[28];
	uint32 SWITCH_CORE_TxQPKTQ0_P3;
	uint8 dummy588[28];
	uint32 SWITCH_CORE_TxBroadcastPkts_P3;
	uint8 dummy589[28];
	uint32 SWITCH_CORE_TxMulticastPkts_P3;
	uint8 dummy590[28];
	uint32 SWITCH_CORE_TxUnicastPkts_P3;
	uint8 dummy591[28];
	uint32 SWITCH_CORE_TxCollisions_P3;
	uint8 dummy592[28];
	uint32 SWITCH_CORE_TxSingleCollision_P3;
	uint8 dummy593[28];
	uint32 SWITCH_CORE_TxMultipleCollision_P3;
	uint8 dummy594[28];
	uint32 SWITCH_CORE_TxDeferredTransmit_P3;
	uint8 dummy595[28];
	uint32 SWITCH_CORE_TxLateCollision_P3;
	uint8 dummy596[28];
	uint32 SWITCH_CORE_TxExcessiveCollision_P3;
	uint8 dummy597[28];
	uint32 SWITCH_CORE_TxFrameInDisc_P3;
	uint8 dummy598[28];
	uint32 SWITCH_CORE_TxPausePkts_P3;
	uint8 dummy599[28];
	uint32 SWITCH_CORE_TxQPKTQ1_P3;
	uint8 dummy600[28];
	uint32 SWITCH_CORE_TxQPKTQ2_P3;
	uint8 dummy601[28];
	uint32 SWITCH_CORE_TxQPKTQ3_P3;
	uint8 dummy602[28];
	uint32 SWITCH_CORE_TxQPKTQ4_P3;
	uint8 dummy603[28];
	uint32 SWITCH_CORE_TxQPKTQ5_P3;
	uint8 dummy604[28];
	uint32 SWITCH_CORE_RxOctets_P3;
	uint8 dummy605[60];
	uint32 SWITCH_CORE_RxUndersizePkts_P3;
	uint8 dummy606[28];
	uint32 SWITCH_CORE_RxPausePkts_P3;
	uint8 dummy607[28];
	uint32 SWITCH_CORE_RxPkts64Octets_P3;
	uint8 dummy608[28];
	uint32 SWITCH_CORE_RxPkts65to127Octets_P3;
	uint8 dummy609[28];
	uint32 SWITCH_CORE_RxPkts128to255Octets_P3;
	uint8 dummy610[28];
	uint32 SWITCH_CORE_RxPkts256to511Octets_P3;
	uint8 dummy611[28];
	uint32 SWITCH_CORE_RxPkts512to1023Octets_P3;
	uint8 dummy612[28];
	uint32 SWITCH_CORE_RxPkts1024toMaxPktOctets_P3;
	uint8 dummy613[28];
	uint32 SWITCH_CORE_RxOversizePkts_P3;
	uint8 dummy614[28];
	uint32 SWITCH_CORE_RxJabbers_P3;
	uint8 dummy615[28];
	uint32 SWITCH_CORE_RxAlignmentErrors_P3;
	uint8 dummy616[28];
	uint32 SWITCH_CORE_RxFCSErrors_P3;
	uint8 dummy617[28];
	uint32 SWITCH_CORE_RxGoodOctets_P3;
	uint8 dummy618[60];
	uint32 SWITCH_CORE_RxDropPkts_P3;
	uint8 dummy619[28];
	uint32 SWITCH_CORE_RxUnicastPkts_P3;
	uint8 dummy620[28];
	uint32 SWITCH_CORE_RxMulticastPkts_P3;
	uint8 dummy621[28];
	uint32 SWITCH_CORE_RxBroadcastPkts_P3;
	uint8 dummy622[28];
	uint32 SWITCH_CORE_RxSAChanges_P3;
	uint8 dummy623[28];
	uint32 SWITCH_CORE_RxFragments_P3;
	uint8 dummy624[28];
	uint32 SWITCH_CORE_RxJumboPkt_P3;
	uint8 dummy625[28];
	uint32 SWITCH_CORE_RxSymblErr_P3;
	uint8 dummy626[28];
	uint32 SWITCH_CORE_InRangeErrCount_P3;
	uint8 dummy627[28];
	uint32 SWITCH_CORE_OutRangeErrCount_P3;
	uint8 dummy628[28];
	uint32 SWITCH_CORE_EEE_LPI_EVENT_P3;
	uint8 dummy629[28];
	uint32 SWITCH_CORE_EEE_LPI_DURATION_P3;
	uint8 dummy630[28];
	uint32 SWITCH_CORE_RxDiscard_P3;
	uint8 dummy631[60];
	uint32 SWITCH_CORE_TxQPKTQ6_P3;
	uint8 dummy632[28];
	uint32 SWITCH_CORE_TxQPKTQ7_P3;
	uint8 dummy633[28];
	uint32 SWITCH_CORE_TxPkts64Octets_P3;
	uint8 dummy634[28];
	uint32 SWITCH_CORE_TxPkts65to127Octets_P3;
	uint8 dummy635[28];
	uint32 SWITCH_CORE_TxPkts128to255Octets_P3;
	uint8 dummy636[28];
	uint32 SWITCH_CORE_TxPkts256to511Octets_P3;
	uint8 dummy637[28];
	uint32 SWITCH_CORE_TxPkts512to1023Octets_P3;
	uint8 dummy638[28];
	uint32 SWITCH_CORE_TxPkts1024toMaxPktOctets_P3;
	uint8 dummy639[220];
	uint32 SWITCH_CORE_TxOctets_P4;
	uint8 dummy640[60];
	uint32 SWITCH_CORE_TxDropPkts_P4;
	uint8 dummy641[28];
	uint32 SWITCH_CORE_TxQPKTQ0_P4;
	uint8 dummy642[28];
	uint32 SWITCH_CORE_TxBroadcastPkts_P4;
	uint8 dummy643[28];
	uint32 SWITCH_CORE_TxMulticastPkts_P4;
	uint8 dummy644[28];
	uint32 SWITCH_CORE_TxUnicastPkts_P4;
	uint8 dummy645[28];
	uint32 SWITCH_CORE_TxCollisions_P4;
	uint8 dummy646[28];
	uint32 SWITCH_CORE_TxSingleCollision_P4;
	uint8 dummy647[28];
	uint32 SWITCH_CORE_TxMultipleCollision_P4;
	uint8 dummy648[28];
	uint32 SWITCH_CORE_TxDeferredTransmit_P4;
	uint8 dummy649[28];
	uint32 SWITCH_CORE_TxLateCollision_P4;
	uint8 dummy650[28];
	uint32 SWITCH_CORE_TxExcessiveCollision_P4;
	uint8 dummy651[28];
	uint32 SWITCH_CORE_TxFrameInDisc_P4;
	uint8 dummy652[28];
	uint32 SWITCH_CORE_TxPausePkts_P4;
	uint8 dummy653[28];
	uint32 SWITCH_CORE_TxQPKTQ1_P4;
	uint8 dummy654[28];
	uint32 SWITCH_CORE_TxQPKTQ2_P4;
	uint8 dummy655[28];
	uint32 SWITCH_CORE_TxQPKTQ3_P4;
	uint8 dummy656[28];
	uint32 SWITCH_CORE_TxQPKTQ4_P4;
	uint8 dummy657[28];
	uint32 SWITCH_CORE_TxQPKTQ5_P4;
	uint8 dummy658[28];
	uint32 SWITCH_CORE_RxOctets_P4;
	uint8 dummy659[60];
	uint32 SWITCH_CORE_RxUndersizePkts_P4;
	uint8 dummy660[28];
	uint32 SWITCH_CORE_RxPausePkts_P4;
	uint8 dummy661[28];
	uint32 SWITCH_CORE_RxPkts64Octets_P4;
	uint8 dummy662[28];
	uint32 SWITCH_CORE_RxPkts65to127Octets_P4;
	uint8 dummy663[28];
	uint32 SWITCH_CORE_RxPkts128to255Octets_P4;
	uint8 dummy664[28];
	uint32 SWITCH_CORE_RxPkts256to511Octets_P4;
	uint8 dummy665[28];
	uint32 SWITCH_CORE_RxPkts512to1023Octets_P4;
	uint8 dummy666[28];
	uint32 SWITCH_CORE_RxPkts1024toMaxPktOctets_P4;
	uint8 dummy667[28];
	uint32 SWITCH_CORE_RxOversizePkts_P4;
	uint8 dummy668[28];
	uint32 SWITCH_CORE_RxJabbers_P4;
	uint8 dummy669[28];
	uint32 SWITCH_CORE_RxAlignmentErrors_P4;
	uint8 dummy670[28];
	uint32 SWITCH_CORE_RxFCSErrors_P4;
	uint8 dummy671[28];
	uint32 SWITCH_CORE_RxGoodOctets_P4;
	uint8 dummy672[60];
	uint32 SWITCH_CORE_RxDropPkts_P4;
	uint8 dummy673[28];
	uint32 SWITCH_CORE_RxUnicastPkts_P4;
	uint8 dummy674[28];
	uint32 SWITCH_CORE_RxMulticastPkts_P4;
	uint8 dummy675[28];
	uint32 SWITCH_CORE_RxBroadcastPkts_P4;
	uint8 dummy676[28];
	uint32 SWITCH_CORE_RxSAChanges_P4;
	uint8 dummy677[28];
	uint32 SWITCH_CORE_RxFragments_P4;
	uint8 dummy678[28];
	uint32 SWITCH_CORE_RxJumboPkt_P4;
	uint8 dummy679[28];
	uint32 SWITCH_CORE_RxSymblErr_P4;
	uint8 dummy680[28];
	uint32 SWITCH_CORE_InRangeErrCount_P4;
	uint8 dummy681[28];
	uint32 SWITCH_CORE_OutRangeErrCount_P4;
	uint8 dummy682[28];
	uint32 SWITCH_CORE_EEE_LPI_EVENT_P4;
	uint8 dummy683[28];
	uint32 SWITCH_CORE_EEE_LPI_DURATION_P4;
	uint8 dummy684[28];
	uint32 SWITCH_CORE_RxDiscard_P4;
	uint8 dummy685[60];
	uint32 SWITCH_CORE_TxQPKTQ6_P4;
	uint8 dummy686[28];
	uint32 SWITCH_CORE_TxQPKTQ7_P4;
	uint8 dummy687[28];
	uint32 SWITCH_CORE_TxPkts64Octets_P4;
	uint8 dummy688[28];
	uint32 SWITCH_CORE_TxPkts65to127Octets_P4;
	uint8 dummy689[28];
	uint32 SWITCH_CORE_TxPkts128to255Octets_P4;
	uint8 dummy690[28];
	uint32 SWITCH_CORE_TxPkts256to511Octets_P4;
	uint8 dummy691[28];
	uint32 SWITCH_CORE_TxPkts512to1023Octets_P4;
	uint8 dummy692[28];
	uint32 SWITCH_CORE_TxPkts1024toMaxPktOctets_P4;
	uint8 dummy693[220];
	uint32 SWITCH_CORE_TxOctets_P5;
	uint8 dummy694[60];
	uint32 SWITCH_CORE_TxDropPkts_P5;
	uint8 dummy695[28];
	uint32 SWITCH_CORE_TxQPKTQ0_P5;
	uint8 dummy696[28];
	uint32 SWITCH_CORE_TxBroadcastPkts_P5;
	uint8 dummy697[28];
	uint32 SWITCH_CORE_TxMulticastPkts_P5;
	uint8 dummy698[28];
	uint32 SWITCH_CORE_TxUnicastPkts_P5;
	uint8 dummy699[28];
	uint32 SWITCH_CORE_TxCollisions_P5;
	uint8 dummy700[28];
	uint32 SWITCH_CORE_TxSingleCollision_P5;
	uint8 dummy701[28];
	uint32 SWITCH_CORE_TxMultipleCollision_P5;
	uint8 dummy702[28];
	uint32 SWITCH_CORE_TxDeferredTransmit_P5;
	uint8 dummy703[28];
	uint32 SWITCH_CORE_TxLateCollision_P5;
	uint8 dummy704[28];
	uint32 SWITCH_CORE_TxExcessiveCollision_P5;
	uint8 dummy705[28];
	uint32 SWITCH_CORE_TxFrameInDisc_P5;
	uint8 dummy706[28];
	uint32 SWITCH_CORE_TxPausePkts_P5;
	uint8 dummy707[28];
	uint32 SWITCH_CORE_TxQPKTQ1_P5;
	uint8 dummy708[28];
	uint32 SWITCH_CORE_TxQPKTQ2_P5;
	uint8 dummy709[28];
	uint32 SWITCH_CORE_TxQPKTQ3_P5;
	uint8 dummy710[28];
	uint32 SWITCH_CORE_TxQPKTQ4_P5;
	uint8 dummy711[28];
	uint32 SWITCH_CORE_TxQPKTQ5_P5;
	uint8 dummy712[28];
	uint32 SWITCH_CORE_RxOctets_P5;
	uint8 dummy713[60];
	uint32 SWITCH_CORE_RxUndersizePkts_P5;
	uint8 dummy714[28];
	uint32 SWITCH_CORE_RxPausePkts_P5;
	uint8 dummy715[28];
	uint32 SWITCH_CORE_RxPkts64Octets_P5;
	uint8 dummy716[28];
	uint32 SWITCH_CORE_RxPkts65to127Octets_P5;
	uint8 dummy717[28];
	uint32 SWITCH_CORE_RxPkts128to255Octets_P5;
	uint8 dummy718[28];
	uint32 SWITCH_CORE_RxPkts256to511Octets_P5;
	uint8 dummy719[28];
	uint32 SWITCH_CORE_RxPkts512to1023Octets_P5;
	uint8 dummy720[28];
	uint32 SWITCH_CORE_RxPkts1024toMaxPktOctets_P5;
	uint8 dummy721[28];
	uint32 SWITCH_CORE_RxOversizePkts_P5;
	uint8 dummy722[28];
	uint32 SWITCH_CORE_RxJabbers_P5;
	uint8 dummy723[28];
	uint32 SWITCH_CORE_RxAlignmentErrors_P5;
	uint8 dummy724[28];
	uint32 SWITCH_CORE_RxFCSErrors_P5;
	uint8 dummy725[28];
	uint32 SWITCH_CORE_RxGoodOctets_P5;
	uint8 dummy726[60];
	uint32 SWITCH_CORE_RxDropPkts_P5;
	uint8 dummy727[28];
	uint32 SWITCH_CORE_RxUnicastPkts_P5;
	uint8 dummy728[28];
	uint32 SWITCH_CORE_RxMulticastPkts_P5;
	uint8 dummy729[28];
	uint32 SWITCH_CORE_RxBroadcastPkts_P5;
	uint8 dummy730[28];
	uint32 SWITCH_CORE_RxSAChanges_P5;
	uint8 dummy731[28];
	uint32 SWITCH_CORE_RxFragments_P5;
	uint8 dummy732[28];
	uint32 SWITCH_CORE_RxJumboPkt_P5;
	uint8 dummy733[28];
	uint32 SWITCH_CORE_RxSymblErr_P5;
	uint8 dummy734[28];
	uint32 SWITCH_CORE_InRangeErrCount_P5;
	uint8 dummy735[28];
	uint32 SWITCH_CORE_OutRangeErrCount_P5;
	uint8 dummy736[28];
	uint32 SWITCH_CORE_EEE_LPI_EVENT_P5;
	uint8 dummy737[28];
	uint32 SWITCH_CORE_EEE_LPI_DURATION_P5;
	uint8 dummy738[28];
	uint32 SWITCH_CORE_RxDiscard_P5;
	uint8 dummy739[60];
	uint32 SWITCH_CORE_TxQPKTQ6_P5;
	uint8 dummy740[28];
	uint32 SWITCH_CORE_TxQPKTQ7_P5;
	uint8 dummy741[28];
	uint32 SWITCH_CORE_TxPkts64Octets_P5;
	uint8 dummy742[28];
	uint32 SWITCH_CORE_TxPkts65to127Octets_P5;
	uint8 dummy743[28];
	uint32 SWITCH_CORE_TxPkts128to255Octets_P5;
	uint8 dummy744[28];
	uint32 SWITCH_CORE_TxPkts256to511Octets_P5;
	uint8 dummy745[28];
	uint32 SWITCH_CORE_TxPkts512to1023Octets_P5;
	uint8 dummy746[28];
	uint32 SWITCH_CORE_TxPkts1024toMaxPktOctets_P5;
	uint8 dummy747[220];
	uint32 SWITCH_CORE_TxOctets_P6;
	uint8 dummy748[60];
	uint32 SWITCH_CORE_TxDropPkts_P6;
	uint8 dummy749[28];
	uint32 SWITCH_CORE_TxQPKTQ0_P6;
	uint8 dummy750[28];
	uint32 SWITCH_CORE_TxBroadcastPkts_P6;
	uint8 dummy751[28];
	uint32 SWITCH_CORE_TxMulticastPkts_P6;
	uint8 dummy752[28];
	uint32 SWITCH_CORE_TxUnicastPkts_P6;
	uint8 dummy753[28];
	uint32 SWITCH_CORE_TxCollisions_P6;
	uint8 dummy754[28];
	uint32 SWITCH_CORE_TxSingleCollision_P6;
	uint8 dummy755[28];
	uint32 SWITCH_CORE_TxMultipleCollision_P6;
	uint8 dummy756[28];
	uint32 SWITCH_CORE_TxDeferredTransmit_P6;
	uint8 dummy757[28];
	uint32 SWITCH_CORE_TxLateCollision_P6;
	uint8 dummy758[28];
	uint32 SWITCH_CORE_TxExcessiveCollision_P6;
	uint8 dummy759[28];
	uint32 SWITCH_CORE_TxFrameInDisc_P6;
	uint8 dummy760[28];
	uint32 SWITCH_CORE_TxPausePkts_P6;
	uint8 dummy761[28];
	uint32 SWITCH_CORE_TxQPKTQ1_P6;
	uint8 dummy762[28];
	uint32 SWITCH_CORE_TxQPKTQ2_P6;
	uint8 dummy763[28];
	uint32 SWITCH_CORE_TxQPKTQ3_P6;
	uint8 dummy764[28];
	uint32 SWITCH_CORE_TxQPKTQ4_P6;
	uint8 dummy765[28];
	uint32 SWITCH_CORE_TxQPKTQ5_P6;
	uint8 dummy766[28];
	uint32 SWITCH_CORE_RxOctets_P6;
	uint8 dummy767[60];
	uint32 SWITCH_CORE_RxUndersizePkts_P6;
	uint8 dummy768[28];
	uint32 SWITCH_CORE_RxPausePkts_P6;
	uint8 dummy769[28];
	uint32 SWITCH_CORE_RxPkts64Octets_P6;
	uint8 dummy770[28];
	uint32 SWITCH_CORE_RxPkts65to127Octets_P6;
	uint8 dummy771[28];
	uint32 SWITCH_CORE_RxPkts128to255Octets_P6;
	uint8 dummy772[28];
	uint32 SWITCH_CORE_RxPkts256to511Octets_P6;
	uint8 dummy773[28];
	uint32 SWITCH_CORE_RxPkts512to1023Octets_P6;
	uint8 dummy774[28];
	uint32 SWITCH_CORE_RxPkts1024toMaxPktOctets_P6;
	uint8 dummy775[28];
	uint32 SWITCH_CORE_RxOversizePkts_P6;
	uint8 dummy776[28];
	uint32 SWITCH_CORE_RxJabbers_P6;
	uint8 dummy777[28];
	uint32 SWITCH_CORE_RxAlignmentErrors_P6;
	uint8 dummy778[28];
	uint32 SWITCH_CORE_RxFCSErrors_P6;
	uint8 dummy779[28];
	uint32 SWITCH_CORE_RxGoodOctets_P6;
	uint8 dummy780[60];
	uint32 SWITCH_CORE_RxDropPkts_P6;
	uint8 dummy781[28];
	uint32 SWITCH_CORE_RxUnicastPkts_P6;
	uint8 dummy782[28];
	uint32 SWITCH_CORE_RxMulticastPkts_P6;
	uint8 dummy783[28];
	uint32 SWITCH_CORE_RxBroadcastPkts_P6;
	uint8 dummy784[28];
	uint32 SWITCH_CORE_RxSAChanges_P6;
	uint8 dummy785[28];
	uint32 SWITCH_CORE_RxFragments_P6;
	uint8 dummy786[28];
	uint32 SWITCH_CORE_RxJumboPkt_P6;
	uint8 dummy787[28];
	uint32 SWITCH_CORE_RxSymblErr_P6;
	uint8 dummy788[28];
	uint32 SWITCH_CORE_InRangeErrCount_P6;
	uint8 dummy789[28];
	uint32 SWITCH_CORE_OutRangeErrCount_P6;
	uint8 dummy790[28];
	uint32 SWITCH_CORE_EEE_LPI_EVENT_P6;
	uint8 dummy791[28];
	uint32 SWITCH_CORE_EEE_LPI_DURATION_P6;
	uint8 dummy792[28];
	uint32 SWITCH_CORE_RxDiscard_P6;
	uint8 dummy793[60];
	uint32 SWITCH_CORE_TxQPKTQ6_P6;
	uint8 dummy794[28];
	uint32 SWITCH_CORE_TxQPKTQ7_P6;
	uint8 dummy795[28];
	uint32 SWITCH_CORE_TxPkts64Octets_P6;
	uint8 dummy796[28];
	uint32 SWITCH_CORE_TxPkts65to127Octets_P6;
	uint8 dummy797[28];
	uint32 SWITCH_CORE_TxPkts128to255Octets_P6;
	uint8 dummy798[28];
	uint32 SWITCH_CORE_TxPkts256to511Octets_P6;
	uint8 dummy799[28];
	uint32 SWITCH_CORE_TxPkts512to1023Octets_P6;
	uint8 dummy800[28];
	uint32 SWITCH_CORE_TxPkts1024toMaxPktOctets_P6;
	uint8 dummy801[220];
	uint32 SWITCH_CORE_TxOctets_P7;
	uint8 dummy802[60];
	uint32 SWITCH_CORE_TxDropPkts_P7;
	uint8 dummy803[28];
	uint32 SWITCH_CORE_TxQPKTQ0_P7;
	uint8 dummy804[28];
	uint32 SWITCH_CORE_TxBroadcastPkts_P7;
	uint8 dummy805[28];
	uint32 SWITCH_CORE_TxMulticastPkts_P7;
	uint8 dummy806[28];
	uint32 SWITCH_CORE_TxUnicastPkts_P7;
	uint8 dummy807[28];
	uint32 SWITCH_CORE_TxCollisions_P7;
	uint8 dummy808[28];
	uint32 SWITCH_CORE_TxSingleCollision_P7;
	uint8 dummy809[28];
	uint32 SWITCH_CORE_TxMultipleCollision_P7;
	uint8 dummy810[28];
	uint32 SWITCH_CORE_TxDeferredTransmit_P7;
	uint8 dummy811[28];
	uint32 SWITCH_CORE_TxLateCollision_P7;
	uint8 dummy812[28];
	uint32 SWITCH_CORE_TxExcessiveCollision_P7;
	uint8 dummy813[28];
	uint32 SWITCH_CORE_TxFrameInDisc_P7;
	uint8 dummy814[28];
	uint32 SWITCH_CORE_TxPausePkts_P7;
	uint8 dummy815[28];
	uint32 SWITCH_CORE_TxQPKTQ1_P7;
	uint8 dummy816[28];
	uint32 SWITCH_CORE_TxQPKTQ2_P7;
	uint8 dummy817[28];
	uint32 SWITCH_CORE_TxQPKTQ3_P7;
	uint8 dummy818[28];
	uint32 SWITCH_CORE_TxQPKTQ4_P7;
	uint8 dummy819[28];
	uint32 SWITCH_CORE_TxQPKTQ5_P7;
	uint8 dummy820[28];
	uint32 SWITCH_CORE_RxOctets_P7;
	uint8 dummy821[60];
	uint32 SWITCH_CORE_RxUndersizePkts_P7;
	uint8 dummy822[28];
	uint32 SWITCH_CORE_RxPausePkts_P7;
	uint8 dummy823[28];
	uint32 SWITCH_CORE_RxPkts64Octets_P7;
	uint8 dummy824[28];
	uint32 SWITCH_CORE_RxPkts65to127Octets_P7;
	uint8 dummy825[28];
	uint32 SWITCH_CORE_RxPkts128to255Octets_P7;
	uint8 dummy826[28];
	uint32 SWITCH_CORE_RxPkts256to511Octets_P7;
	uint8 dummy827[28];
	uint32 SWITCH_CORE_RxPkts512to1023Octets_P7;
	uint8 dummy828[28];
	uint32 SWITCH_CORE_RxPkts1024toMaxPktOctets_P7;
	uint8 dummy829[28];
	uint32 SWITCH_CORE_RxOversizePkts_P7;
	uint8 dummy830[28];
	uint32 SWITCH_CORE_RxJabbers_P7;
	uint8 dummy831[28];
	uint32 SWITCH_CORE_RxAlignmentErrors_P7;
	uint8 dummy832[28];
	uint32 SWITCH_CORE_RxFCSErrors_P7;
	uint8 dummy833[28];
	uint32 SWITCH_CORE_RxGoodOctets_P7;
	uint8 dummy834[60];
	uint32 SWITCH_CORE_RxDropPkts_P7;
	uint8 dummy835[28];
	uint32 SWITCH_CORE_RxUnicastPkts_P7;
	uint8 dummy836[28];
	uint32 SWITCH_CORE_RxMulticastPkts_P7;
	uint8 dummy837[28];
	uint32 SWITCH_CORE_RxBroadcastPkts_P7;
	uint8 dummy838[28];
	uint32 SWITCH_CORE_RxSAChanges_P7;
	uint8 dummy839[28];
	uint32 SWITCH_CORE_RxFragments_P7;
	uint8 dummy840[28];
	uint32 SWITCH_CORE_RxJumboPkt_P7;
	uint8 dummy841[28];
	uint32 SWITCH_CORE_RxSymblErr_P7;
	uint8 dummy842[28];
	uint32 SWITCH_CORE_InRangeErrCount_P7;
	uint8 dummy843[28];
	uint32 SWITCH_CORE_OutRangeErrCount_P7;
	uint8 dummy844[28];
	uint32 SWITCH_CORE_EEE_LPI_EVENT_P7;
	uint8 dummy845[28];
	uint32 SWITCH_CORE_EEE_LPI_DURATION_P7;
	uint8 dummy846[28];
	uint32 SWITCH_CORE_RxDiscard_P7;
	uint8 dummy847[60];
	uint32 SWITCH_CORE_TxQPKTQ6_P7;
	uint8 dummy848[28];
	uint32 SWITCH_CORE_TxQPKTQ7_P7;
	uint8 dummy849[28];
	uint32 SWITCH_CORE_TxPkts64Octets_P7;
	uint8 dummy850[28];
	uint32 SWITCH_CORE_TxPkts65to127Octets_P7;
	uint8 dummy851[28];
	uint32 SWITCH_CORE_TxPkts128to255Octets_P7;
	uint8 dummy852[28];
	uint32 SWITCH_CORE_TxPkts256to511Octets_P7;
	uint8 dummy853[28];
	uint32 SWITCH_CORE_TxPkts512to1023Octets_P7;
	uint8 dummy854[28];
	uint32 SWITCH_CORE_TxPkts1024toMaxPktOctets_P7;
	uint8 dummy855[220];
	uint32 SWITCH_CORE_TxOctets_IMP;
	uint8 dummy856[60];
	uint32 SWITCH_CORE_TxDropPkts_IMP;
	uint8 dummy857[28];
	uint32 SWITCH_CORE_TxQPKTQ0_IMP;
	uint8 dummy858[28];
	uint32 SWITCH_CORE_TxBroadcastPkts_IMP;
	uint8 dummy859[28];
	uint32 SWITCH_CORE_TxMulticastPkts_IMP;
	uint8 dummy860[28];
	uint32 SWITCH_CORE_TxUnicastPkts_IMP;
	uint8 dummy861[28];
	uint32 SWITCH_CORE_TxCollisions_IMP;
	uint8 dummy862[28];
	uint32 SWITCH_CORE_TxSingleCollision_IMP;
	uint8 dummy863[28];
	uint32 SWITCH_CORE_TxMultipleCollision_IMP;
	uint8 dummy864[28];
	uint32 SWITCH_CORE_TxDeferredTransmit_IMP;
	uint8 dummy865[28];
	uint32 SWITCH_CORE_TxLateCollision_IMP;
	uint8 dummy866[28];
	uint32 SWITCH_CORE_TxExcessiveCollision_IMP;
	uint8 dummy867[28];
	uint32 SWITCH_CORE_TxFrameInDisc_IMP;
	uint8 dummy868[28];
	uint32 SWITCH_CORE_TxPausePkts_IMP;
	uint8 dummy869[28];
	uint32 SWITCH_CORE_TxQPKTQ1_IMP;
	uint8 dummy870[28];
	uint32 SWITCH_CORE_TxQPKTQ2_IMP;
	uint8 dummy871[28];
	uint32 SWITCH_CORE_TxQPKTQ3_IMP;
	uint8 dummy872[28];
	uint32 SWITCH_CORE_TxQPKTQ4_IMP;
	uint8 dummy873[28];
	uint32 SWITCH_CORE_TxQPKTQ5_IMP;
	uint8 dummy874[28];
	uint32 SWITCH_CORE_RxOctets_IMP;
	uint8 dummy875[60];
	uint32 SWITCH_CORE_RxUndersizePkts_IMP;
	uint8 dummy876[28];
	uint32 SWITCH_CORE_RxPausePkts_IMP;
	uint8 dummy877[28];
	uint32 SWITCH_CORE_RxPkts64Octets_IMP;
	uint8 dummy878[28];
	uint32 SWITCH_CORE_RxPkts65to127Octets_IMP;
	uint8 dummy879[28];
	uint32 SWITCH_CORE_RxPkts128to255Octets_IMP;
	uint8 dummy880[28];
	uint32 SWITCH_CORE_RxPkts256to511Octets_IMP;
	uint8 dummy881[28];
	uint32 SWITCH_CORE_RxPkts512to1023Octets_IMP;
	uint8 dummy882[28];
	uint32 SWITCH_CORE_RxPkts1024toMaxPktOctets_IMP;
	uint8 dummy883[28];
	uint32 SWITCH_CORE_RxOversizePkts_IMP;
	uint8 dummy884[28];
	uint32 SWITCH_CORE_RxJabbers_IMP;
	uint8 dummy885[28];
	uint32 SWITCH_CORE_RxAlignmentErrors_IMP;
	uint8 dummy886[28];
	uint32 SWITCH_CORE_RxFCSErrors_IMP;
	uint8 dummy887[28];
	uint32 SWITCH_CORE_RxGoodOctets_IMP;
	uint8 dummy888[60];
	uint32 SWITCH_CORE_RxDropPkts_IMP;
	uint8 dummy889[28];
	uint32 SWITCH_CORE_RxUnicastPkts_IMP;
	uint8 dummy890[28];
	uint32 SWITCH_CORE_RxMulticastPkts_IMP;
	uint8 dummy891[28];
	uint32 SWITCH_CORE_RxBroadcastPkts_IMP;
	uint8 dummy892[28];
	uint32 SWITCH_CORE_RxSAChanges_IMP;
	uint8 dummy893[28];
	uint32 SWITCH_CORE_RxFragments_IMP;
	uint8 dummy894[28];
	uint32 SWITCH_CORE_RxJumboPkt_IMP;
	uint8 dummy895[28];
	uint32 SWITCH_CORE_RxSymblErr_IMP;
	uint8 dummy896[28];
	uint32 SWITCH_CORE_InRangeErrCount_IMP;
	uint8 dummy897[28];
	uint32 SWITCH_CORE_OutRangeErrCount_IMP;
	uint8 dummy898[28];
	uint32 SWITCH_CORE_EEE_LPI_EVENT_IMP;
	uint8 dummy899[28];
	uint32 SWITCH_CORE_EEE_LPI_DURATION_IMP;
	uint8 dummy900[28];
	uint32 SWITCH_CORE_RxDiscard_IMP;
	uint8 dummy901[60];
	uint32 SWITCH_CORE_TxQPKTQ6_IMP;
	uint8 dummy902[28];
	uint32 SWITCH_CORE_TxQPKTQ7_IMP;
	uint8 dummy903[28];
	uint32 SWITCH_CORE_TxPkts64Octets_IMP;
	uint8 dummy904[28];
	uint32 SWITCH_CORE_TxPkts65to127Octets_IMP;
	uint8 dummy905[28];
	uint32 SWITCH_CORE_TxPkts128to255Octets_IMP;
	uint8 dummy906[28];
	uint32 SWITCH_CORE_TxPkts256to511Octets_IMP;
	uint8 dummy907[28];
	uint32 SWITCH_CORE_TxPkts512to1023Octets_IMP;
	uint8 dummy908[28];
	uint32 SWITCH_CORE_TxPkts1024toMaxPktOctets_IMP;
	uint8 dummy909[14556];
	uint32 SWITCH_CORE_QOS_GLOBAL_CTRL;
	uint8 dummy910[28];
	uint32 SWITCH_CORE_QOS_1P_EN;
	uint8 dummy911[12];
	uint32 SWITCH_CORE_QOS_EN_DIFFSERV;
	uint8 dummy912[76];
	uint32 SWITCH_CORE_PCP2TC_DEI0_P0;
	uint8 dummy913[28];
	uint32 SWITCH_CORE_PCP2TC_DEI0_P1;
	uint8 dummy914[28];
	uint32 SWITCH_CORE_PCP2TC_DEI0_P2;
	uint8 dummy915[28];
	uint32 SWITCH_CORE_PCP2TC_DEI0_P3;
	uint8 dummy916[28];
	uint32 SWITCH_CORE_PCP2TC_DEI0_P4;
	uint8 dummy917[28];
	uint32 SWITCH_CORE_PCP2TC_DEI0_P5;
	uint8 dummy918[28];
	uint32 SWITCH_CORE_PCP2TC_DEI0_P6;
	uint8 dummy919[28];
	uint32 SWITCH_CORE_PCP2TC_DEI0_P7;
	uint8 dummy920[28];
	uint32 SWITCH_CORE_PCP2TC_DEI0_IMP;
	uint8 dummy921[28];
	uint32 SWITCH_CORE_QOS_DIFF_DSCP0;
	uint8 dummy922[44];
	uint32 SWITCH_CORE_QOS_DIFF_DSCP1;
	uint8 dummy923[44];
	uint32 SWITCH_CORE_QOS_DIFF_DSCP2;
	uint8 dummy924[44];
	uint32 SWITCH_CORE_QOS_DIFF_DSCP3;
	uint8 dummy925[44];
	uint32 SWITCH_CORE_PID2TC;
	uint8 dummy926[60];
	uint32 SWITCH_CORE_TC_SEL_TABLE_P0;
	uint8 dummy927[12];
	uint32 SWITCH_CORE_TC_SEL_TABLE_P1;
	uint8 dummy928[12];
	uint32 SWITCH_CORE_TC_SEL_TABLE_P2;
	uint8 dummy929[12];
	uint32 SWITCH_CORE_TC_SEL_TABLE_P3;
	uint8 dummy930[12];
	uint32 SWITCH_CORE_TC_SEL_TABLE_P4;
	uint8 dummy931[12];
	uint32 SWITCH_CORE_TC_SEL_TABLE_P5;
	uint8 dummy932[12];
	uint32 SWITCH_CORE_TC_SEL_TABLE_P6;
	uint8 dummy933[12];
	uint32 SWITCH_CORE_TC_SEL_TABLE_P7;
	uint8 dummy934[12];
	uint32 SWITCH_CORE_TC_SEL_TABLE_IMP;
	uint8 dummy935[28];
	uint32 SWITCH_CORE_CPU2COS_MAP;
	uint8 dummy936[60];
	uint32 SWITCH_CORE_TC2COS_MAP_P0;
	uint8 dummy937[28];
	uint32 SWITCH_CORE_TC2COS_MAP_P1;
	uint8 dummy938[28];
	uint32 SWITCH_CORE_TC2COS_MAP_P2;
	uint8 dummy939[28];
	uint32 SWITCH_CORE_TC2COS_MAP_P3;
	uint8 dummy940[28];
	uint32 SWITCH_CORE_TC2COS_MAP_P4;
	uint8 dummy941[28];
	uint32 SWITCH_CORE_TC2COS_MAP_P5;
	uint8 dummy942[28];
	uint32 SWITCH_CORE_TC2COS_MAP_P6;
	uint8 dummy943[28];
	uint32 SWITCH_CORE_TC2COS_MAP_P7;
	uint8 dummy944[28];
	uint32 SWITCH_CORE_TC2COS_MAP_IMP;
	uint8 dummy945[188];
	uint32 SWITCH_CORE_QOS_REG_SPARE0;
	uint8 dummy946[28];
	uint32 SWITCH_CORE_QOS_REG_SPARE1;
	uint8 dummy947[28];
	uint32 SWITCH_CORE_PCP2TC_DEI1_P0;
	uint8 dummy948[28];
	uint32 SWITCH_CORE_PCP2TC_DEI1_P1;
	uint8 dummy949[28];
	uint32 SWITCH_CORE_PCP2TC_DEI1_P2;
	uint8 dummy950[28];
	uint32 SWITCH_CORE_PCP2TC_DEI1_P3;
	uint8 dummy951[28];
	uint32 SWITCH_CORE_PCP2TC_DEI1_P4;
	uint8 dummy952[28];
	uint32 SWITCH_CORE_PCP2TC_DEI1_P5;
	uint8 dummy953[28];
	uint32 SWITCH_CORE_PCP2TC_DEI1_P6;
	uint8 dummy954[28];
	uint32 SWITCH_CORE_PCP2TC_DEI1_P7;
	uint8 dummy955[28];
	uint32 SWITCH_CORE_PCP2TC_DEI1_IMP;
	uint8 dummy956[380];
	uint32 port_vlan_ctrl[4*9];
	uint8 dummy965[112];
	uint32 SWITCH_CORE_VLAN_REG_SPARE0;
	uint8 dummy966[28];
	uint32 SWITCH_CORE_VLAN_REG_SPARE1;
	uint8 dummy967[1756];
	uint32 SWITCH_CORE_MAC_TRUNK_CTL;
	uint8 dummy968[12];
	uint32 SWITCH_CORE_IMP0_GRP_CTL;
	uint8 dummy969[108];
	uint32 SWITCH_CORE_TRUNK_GRP_CTL0;
	uint8 dummy970[12];
	uint32 SWITCH_CORE_TRUNK_GRP_CTL1;
	uint8 dummy971[12];
	uint32 SWITCH_CORE_TRUNK_GRP_CTL2;
	uint8 dummy972[12];
	uint32 SWITCH_CORE_TRUNK_GRP_CTL3;
	uint8 dummy973[76];
	uint32 SWITCH_CORE_TRUNK_HASH_OVRRD;
	uint8 dummy974[252];
	uint32 SWITCH_CORE_TRUNK_REG_SPARE0;
	uint8 dummy975[28];
	uint32 SWITCH_CORE_TRUNK_REG_SPARE1;
	uint8 dummy976[3548];
	uint64 SWITCH_CORE_VLAN_CTRL0;
	uint64 SWITCH_CORE_VLAN_CTRL1;
	uint64 SWITCH_CORE_VLAN_CTRL2;
	uint32 SWITCH_CORE_VLAN_CTRL3;
	uint8 dummy977[12];
	uint64 SWITCH_CORE_VLAN_CTRL4;
	uint64 SWITCH_CORE_VLAN_CTRL5;
	uint32 SWITCH_CORE_VLAN_CTRL6;
	uint8 dummy978[20];
	uint32 SWITCH_CORE_VLAN_MULTI_PORT_ADDR_CTL;
	uint8 dummy979[44];
	uint32 SWITCH_CORE_DEFAULT_1Q_TAG_P0;
	uint8 dummy980[12];
	uint32 SWITCH_CORE_DEFAULT_1Q_TAG_P1;
	uint8 dummy981[12];
	uint32 SWITCH_CORE_DEFAULT_1Q_TAG_P2;
	uint8 dummy982[12];
	uint32 SWITCH_CORE_DEFAULT_1Q_TAG_P3;
	uint8 dummy983[12];
	uint32 SWITCH_CORE_DEFAULT_1Q_TAG_P4;
	uint8 dummy984[12];
	uint32 SWITCH_CORE_DEFAULT_1Q_TAG_P5;
	uint8 dummy985[12];
	uint32 SWITCH_CORE_DEFAULT_1Q_TAG_P6;
	uint8 dummy986[12];
	uint32 SWITCH_CORE_DEFAULT_1Q_TAG_P7;
	uint8 dummy987[12];
	uint32 SWITCH_CORE_DEFAULT_1Q_TAG_IMP;
	uint8 dummy988[124];
	uint32 SWITCH_CORE_DTAG_TPID;
	uint8 dummy989[12];
	uint32 SWITCH_CORE_ISP_SEL_PORTMAP;
	uint8 dummy990[108];
	uint32 SWITCH_CORE_EGRESS_VID_RMK_TBL_ACS;
	uint8 dummy991[28];
	uint32 SWITCH_CORE_EGRESS_VID_RMK_TBL_DATA;
	uint8 dummy992[92];
	uint32 SWITCH_CORE_JOIN_ALL_VLAN_EN;
	uint8 dummy993[12];
	uint32 SWITCH_CORE_PORT_IVL_SVL_CTRL;
	uint8 dummy994[108];
	uint32 SWITCH_CORE_BCM8021Q_REG_SPARE0;
	uint8 dummy995[28];
	uint32 SWITCH_CORE_BCM8021Q_REG_SPARE1;
	uint8 dummy996[3292];
	uint32 SWITCH_CORE_DOS_CTRL;
	uint8 dummy997[28];
	uint32 SWITCH_CORE_MINIMUM_TCP_HDR_SZ;
	uint8 dummy998[28];
	uint32 SWITCH_CORE_MAX_ICMPV4_SIZE_REG;
	uint8 dummy999[28];
	uint32 SWITCH_CORE_MAX_ICMPV6_SIZE_REG;
	uint8 dummy1000[28];
	uint32 SWITCH_CORE_DOS_DIS_LRN_REG;
	uint8 dummy1001[124];
	uint32 SWITCH_CORE_DOS_REG_SPARE0;
	uint8 dummy1002[28];
	uint32 SWITCH_CORE_DOS_REG_SPARE1;
	uint8 dummy1003[20196];
	uint32 SWITCH_CORE_JUMBO_PORT_MASK;
	uint8 dummy1004[28];
	uint32 SWITCH_CORE_MIB_GD_FM_MAX_SIZE;
	uint8 dummy1005[84];
	uint32 SWITCH_CORE_JUMBO_CTRL_REG_SPARE0;
	uint8 dummy1006[28];
	uint32 SWITCH_CORE_JUMBO_CTRL_REG_SPARE1;
	uint8 dummy1007[1884];
	uint32 SWITCH_CORE_COMM_IRC_CON;
	uint8 dummy1008[28];
	uint32 SWITCH_CORE_IRC_VIRTUAL_ZERO_THD;
	uint8 dummy1009[12];
	uint32 SWITCH_CORE_IRC_ALARM_THD;
	uint8 dummy1010[76];
	uint32 SWITCH_CORE_BC_SUP_RATECTRL_P0;
	uint8 dummy1011[28];
	uint32 SWITCH_CORE_BC_SUP_RATECTRL_P1;
	uint8 dummy1012[28];
	uint32 SWITCH_CORE_BC_SUP_RATECTRL_P2;
	uint8 dummy1013[28];
	uint32 SWITCH_CORE_BC_SUP_RATECTRL_P3;
	uint8 dummy1014[28];
	uint32 SWITCH_CORE_BC_SUP_RATECTRL_P4;
	uint8 dummy1015[28];
	uint32 SWITCH_CORE_BC_SUP_RATECTRL_P5;
	uint8 dummy1016[28];
	uint32 SWITCH_CORE_BC_SUP_RATECTRL_P6;
	uint8 dummy1017[28];
	uint32 SWITCH_CORE_BC_SUP_RATECTRL_P7;
	uint8 dummy1018[28];
	uint32 SWITCH_CORE_BC_SUP_RATECTRL_IMP;
	uint8 dummy1019[28];
	uint32 SWITCH_CORE_BC_SUP_RATECTRL_1_P0;
	uint8 dummy1020[12];
	uint32 SWITCH_CORE_BC_SUP_RATECTRL_1_P1;
	uint8 dummy1021[12];
	uint32 SWITCH_CORE_BC_SUP_RATECTRL_1_P2;
	uint8 dummy1022[12];
	uint32 SWITCH_CORE_BC_SUP_RATECTRL_1_P3;
	uint8 dummy1023[12];
	uint32 SWITCH_CORE_BC_SUP_RATECTRL_1_P4;
	uint8 dummy1024[12];
	uint32 SWITCH_CORE_BC_SUP_RATECTRL_1_P5;
	uint8 dummy1025[12];
	uint32 SWITCH_CORE_BC_SUP_RATECTRL_1_P6;
	uint8 dummy1026[12];
	uint32 SWITCH_CORE_BC_SUP_RATECTRL_1_P7;
	uint8 dummy1027[12];
	uint32 SWITCH_CORE_BC_SUP_RATECTRL_1_IMP;
	uint8 dummy1028[92];
	uint32 SWITCH_CORE_BC_SUP_PKTDROP_CNT_P0;
	uint8 dummy1029[28];
	uint32 SWITCH_CORE_BC_SUP_PKTDROP_CNT_P1;
	uint8 dummy1030[28];
	uint32 SWITCH_CORE_BC_SUP_PKTDROP_CNT_P2;
	uint8 dummy1031[28];
	uint32 SWITCH_CORE_BC_SUP_PKTDROP_CNT_P3;
	uint8 dummy1032[28];
	uint32 SWITCH_CORE_BC_SUP_PKTDROP_CNT_P4;
	uint8 dummy1033[28];
	uint32 SWITCH_CORE_BC_SUP_PKTDROP_CNT_P5;
	uint8 dummy1034[28];
	uint32 SWITCH_CORE_BC_SUP_PKTDROP_CNT_P6;
	uint8 dummy1035[28];
	uint32 SWITCH_CORE_BC_SUP_PKTDROP_CNT_P7;
	uint8 dummy1036[28];
	uint32 SWITCH_CORE_BC_SUP_PKTDROP_CNT_IMP;
	uint8 dummy1037[764];
	uint32 SWITCH_CORE_BC_SUPPRESS_REG_SPARE0;
	uint8 dummy1038[28];
	uint32 SWITCH_CORE_BC_SUPPRESS_REG_SPARE1;
	uint8 dummy1039[348];
	uint64 SWITCH_CORE_EAP_GLO_CON;
	uint64 SWITCH_CORE_EAP_MULTI_ADDR_CTRL;
	uint32 SWITCH_CORE_EAP_DIP0;
	uint8 dummy1040[60];
	uint32 SWITCH_CORE_EAP_DIP1;
	uint8 dummy1041[172];
	uint32 SWITCH_CORE_EAP_CON_P0;
	uint8 dummy1042[60];
	uint32 SWITCH_CORE_EAP_CON_P1;
	uint8 dummy1043[60];
	uint32 SWITCH_CORE_EAP_CON_P2;
	uint8 dummy1044[60];
	uint32 SWITCH_CORE_EAP_CON_P3;
	uint8 dummy1045[60];
	uint32 SWITCH_CORE_EAP_CON_P4;
	uint8 dummy1046[60];
	uint32 SWITCH_CORE_EAP_CON_P5;
	uint8 dummy1047[60];
	uint32 SWITCH_CORE_EAP_CON_P6;
	uint8 dummy1048[60];
	uint32 SWITCH_CORE_EAP_CON_P7;
	uint8 dummy1049[60];
	uint32 SWITCH_CORE_EAP_CON_IMP;
	uint8 dummy1050[124];
	uint32 SWITCH_CORE_IEEE8021X_REG_SPARE0;
	uint8 dummy1051[28];
	uint32 SWITCH_CORE_IEEE8021X_REG_SPARE1;
	uint8 dummy1052[1116];
	uint32 SWITCH_CORE_MST_CON;
	uint8 dummy1053[12];
	uint32 SWITCH_CORE_MST_AGE;
	uint8 dummy1054[108];
	uint32 SWITCH_CORE_MST_TAB0;
	uint8 dummy1055[28];
	uint32 SWITCH_CORE_MST_TAB1;
	uint8 dummy1056[28];
	uint32 SWITCH_CORE_MST_TAB2;
	uint8 dummy1057[28];
	uint32 SWITCH_CORE_MST_TAB3;
	uint8 dummy1058[28];
	uint32 SWITCH_CORE_MST_TAB4;
	uint8 dummy1059[28];
	uint32 SWITCH_CORE_MST_TAB5;
	uint8 dummy1060[28];
	uint32 SWITCH_CORE_MST_TAB6;
	uint8 dummy1061[28];
	uint32 SWITCH_CORE_MST_TAB7;
	uint8 dummy1062[284];
	uint32 SWITCH_CORE_SPT_MULTI_ADDR_BPS_CTRL;
	uint8 dummy1063[124];
	uint32 SWITCH_CORE_IEEE8021S_REG_SPARE0;
	uint8 dummy1064[28];
	uint32 SWITCH_CORE_IEEE8021S_REG_SPARE1;
	uint8 dummy1065[3292];
	uint32 SWITCH_CORE_SA_LIMIT_ENABLE;
	uint8 dummy1066[12];
	uint32 SWITCH_CORE_SA_LRN_CNTR_RST;
	uint8 dummy1067[12];
	uint32 SWITCH_CORE_SA_OVERLIMIT_CNTR_RST;
	uint8 dummy1068[92];
	uint32 SWITCH_CORE_TOTAL_SA_LIMIT_CTL;
	uint8 dummy1069[12];
	uint32 SWITCH_CORE_SA_LIMIT_CTL_P0;
	uint8 dummy1070[12];
	uint32 SWITCH_CORE_SA_LIMIT_CTL_P1;
	uint8 dummy1071[12];
	uint32 SWITCH_CORE_SA_LIMIT_CTL_P2;
	uint8 dummy1072[12];
	uint32 SWITCH_CORE_SA_LIMIT_CTL_P3;
	uint8 dummy1073[12];
	uint32 SWITCH_CORE_SA_LIMIT_CTL_P4;
	uint8 dummy1074[12];
	uint32 SWITCH_CORE_SA_LIMIT_CTL_P5;
	uint8 dummy1075[12];
	uint32 SWITCH_CORE_SA_LIMIT_CTL_P6;
	uint8 dummy1076[12];
	uint32 SWITCH_CORE_SA_LIMIT_CTL_P7;
	uint8 dummy1077[12];
	uint32 SWITCH_CORE_SA_LIMIT_CTL_P8;
	uint8 dummy1078[108];
	uint32 SWITCH_CORE_TOTAL_SA_LRN_CNTR;
	uint8 dummy1079[12];
	uint32 SWITCH_CORE_SA_LRN_CNTR_P0;
	uint8 dummy1080[12];
	uint32 SWITCH_CORE_SA_LRN_CNTR_P1;
	uint8 dummy1081[12];
	uint32 SWITCH_CORE_SA_LRN_CNTR_P2;
	uint8 dummy1082[12];
	uint32 SWITCH_CORE_SA_LRN_CNTR_P3;
	uint8 dummy1083[12];
	uint32 SWITCH_CORE_SA_LRN_CNTR_P4;
	uint8 dummy1084[12];
	uint32 SWITCH_CORE_SA_LRN_CNTR_P5;
	uint8 dummy1085[12];
	uint32 SWITCH_CORE_SA_LRN_CNTR_P6;
	uint8 dummy1086[12];
	uint32 SWITCH_CORE_SA_LRN_CNTR_P7;
	uint8 dummy1087[12];
	uint32 SWITCH_CORE_SA_LRN_CNTR_P8;
	uint8 dummy1088[108];
	uint32 SWITCH_CORE_SA_OVERLIMIT_CNTR_P0;
	uint8 dummy1089[28];
	uint32 SWITCH_CORE_SA_OVERLIMIT_CNTR_P1;
	uint8 dummy1090[28];
	uint32 SWITCH_CORE_SA_OVERLIMIT_CNTR_P2;
	uint8 dummy1091[28];
	uint32 SWITCH_CORE_SA_OVERLIMIT_CNTR_P3;
	uint8 dummy1092[28];
	uint32 SWITCH_CORE_SA_OVERLIMIT_CNTR_P4;
	uint8 dummy1093[28];
	uint32 SWITCH_CORE_SA_OVERLIMIT_CNTR_P5;
	uint8 dummy1094[28];
	uint32 SWITCH_CORE_SA_OVERLIMIT_CNTR_P6;
	uint8 dummy1095[28];
	uint32 SWITCH_CORE_SA_OVERLIMIT_CNTR_P7;
	uint8 dummy1096[28];
	uint32 SWITCH_CORE_SA_OVERLIMIT_CNTR_P8;
	uint8 dummy1097[28];
	uint32 SWITCH_CORE_SA_OVER_LIMIT_COPY_REDIRECT;
	uint8 dummy1098[92];
	uint32 SWITCH_CORE_MAC_LIMIT_REG_SPARE0;
	uint8 dummy1099[28];
	uint32 SWITCH_CORE_MAC_LIMIT_REG_SPARE1;
	uint8 dummy1100[988];
	uint64 SWITCH_CORE_QOS_PRI_CTL_P0;
	uint64 SWITCH_CORE_QOS_PRI_CTL_P1;
	uint64 SWITCH_CORE_QOS_PRI_CTL_P2;
	uint64 SWITCH_CORE_QOS_PRI_CTL_P3;
	uint64 SWITCH_CORE_QOS_PRI_CTL_P4;
	uint64 SWITCH_CORE_QOS_PRI_CTL_P5;
	uint64 SWITCH_CORE_QOS_PRI_CTL_P6;
	uint64 SWITCH_CORE_QOS_PRI_CTL_P7;
	uint32 SWITCH_CORE_QOS_PRI_CTL_IMP;
	uint8 dummy1101[60];
	uint32 SWITCH_CORE_QOS_WEIGHT_P0;
	uint8 dummy1102[60];
	uint32 SWITCH_CORE_QOS_WEIGHT_P1;
	uint8 dummy1103[60];
	uint32 SWITCH_CORE_QOS_WEIGHT_P2;
	uint8 dummy1104[60];
	uint32 SWITCH_CORE_QOS_WEIGHT_P3;
	uint8 dummy1105[60];
	uint32 SWITCH_CORE_QOS_WEIGHT_P4;
	uint8 dummy1106[60];
	uint32 SWITCH_CORE_QOS_WEIGHT_P5;
	uint8 dummy1107[60];
	uint32 SWITCH_CORE_QOS_WEIGHT_P6;
	uint8 dummy1108[60];
	uint32 SWITCH_CORE_QOS_WEIGHT_P7;
	uint8 dummy1109[60];
	uint32 SWITCH_CORE_QOS_WEIGHT_IMP;
	uint8 dummy1110[124];
	uint32 SWITCH_CORE_WDRR_PENALTY_P0;
	uint8 dummy1111[12];
	uint32 SWITCH_CORE_WDRR_PENALTY_P1;
	uint8 dummy1112[12];
	uint32 SWITCH_CORE_WDRR_PENALTY_P2;
	uint8 dummy1113[12];
	uint32 SWITCH_CORE_WDRR_PENALTY_P3;
	uint8 dummy1114[12];
	uint32 SWITCH_CORE_WDRR_PENALTY_P4;
	uint8 dummy1115[12];
	uint32 SWITCH_CORE_WDRR_PENALTY_P5;
	uint8 dummy1116[12];
	uint32 SWITCH_CORE_WDRR_PENALTY_P6;
	uint8 dummy1117[28];
	uint32 SWITCH_CORE_WDRR_PENALTY_P7;
	uint8 dummy1118[12];
	uint32 SWITCH_CORE_P8_WDRR_PENALTY;
	uint8 dummy1119[108];
	uint32 SWITCH_CORE_SCHEDULER_REG_SPARE0;
	uint8 dummy1120[28];
	uint32 SWITCH_CORE_SCHEDULER_REG_SPARE1;
	uint8 dummy1121[988];
	uint32 SWITCH_CORE_PORT_SHAPER_BYTE_BASED_MAX_REFRESH_P0;
	uint8 dummy1122[28];
	uint32 SWITCH_CORE_PORT_SHAPER_BYTE_BASED_MAX_REFRESH_P1;
	uint8 dummy1123[28];
	uint32 SWITCH_CORE_PORT_SHAPER_BYTE_BASED_MAX_REFRESH_P2;
	uint8 dummy1124[28];
	uint32 SWITCH_CORE_PORT_SHAPER_BYTE_BASED_MAX_REFRESH_P3;
	uint8 dummy1125[28];
	uint32 SWITCH_CORE_PORT_SHAPER_BYTE_BASED_MAX_REFRESH_P4;
	uint8 dummy1126[28];
	uint32 SWITCH_CORE_PORT_SHAPER_BYTE_BASED_MAX_REFRESH_P5;
	uint8 dummy1127[28];
	uint32 SWITCH_CORE_PORT_SHAPER_BYTE_BASED_MAX_REFRESH_P6;
	uint8 dummy1128[28];
	uint32 SWITCH_CORE_PORT_SHAPER_BYTE_BASED_MAX_REFRESH_P7;
	uint8 dummy1129[28];
	uint32 SWITCH_CORE_PORT_SHAPER_BYTE_BASED_MAX_REFRESH_IMP;
	uint8 dummy1130[124];
	uint32 SWITCH_CORE_PORT_SHAPER_BYTE_BASED_MAX_THD_SEL_P0;
	uint8 dummy1131[28];
	uint32 SWITCH_CORE_PORT_SHAPER_BYTE_BASED_MAX_THD_SEL_P1;
	uint8 dummy1132[28];
	uint32 SWITCH_CORE_PORT_SHAPER_BYTE_BASED_MAX_THD_SEL_P2;
	uint8 dummy1133[28];
	uint32 SWITCH_CORE_PORT_SHAPER_BYTE_BASED_MAX_THD_SEL_P3;
	uint8 dummy1134[28];
	uint32 SWITCH_CORE_PORT_SHAPER_BYTE_BASED_MAX_THD_SEL_P4;
	uint8 dummy1135[28];
	uint32 SWITCH_CORE_PORT_SHAPER_BYTE_BASED_MAX_THD_SEL_P5;
	uint8 dummy1136[28];
	uint32 SWITCH_CORE_PORT_SHAPER_BYTE_BASED_MAX_THD_SEL_P6;
	uint8 dummy1137[28];
	uint32 SWITCH_CORE_PORT_SHAPER_BYTE_BASED_MAX_THD_SEL_P7;
	uint8 dummy1138[28];
	uint32 SWITCH_CORE_PORT_SHAPER_BYTE_BASED_MAX_THD_SEL_IMP;
	uint8 dummy1139[124];
	uint32 SWITCH_CORE_PORT_SHAPER_STS_P0;
	uint8 dummy1140[28];
	uint32 SWITCH_CORE_PORT_SHAPER_STS_P1;
	uint8 dummy1141[28];
	uint32 SWITCH_CORE_PORT_SHAPER_STS_P2;
	uint8 dummy1142[28];
	uint32 SWITCH_CORE_PORT_SHAPER_STS_P3;
	uint8 dummy1143[28];
	uint32 SWITCH_CORE_PORT_SHAPER_STS_P4;
	uint8 dummy1144[28];
	uint32 SWITCH_CORE_PORT_SHAPER_STS_P5;
	uint8 dummy1145[28];
	uint32 SWITCH_CORE_PORT_SHAPER_STS_P6;
	uint8 dummy1146[28];
	uint32 SWITCH_CORE_PORT_SHAPER_STS_P7;
	uint8 dummy1147[28];
	uint32 SWITCH_CORE_PORT_SHAPER_STS_IMP;
	uint8 dummy1148[124];
	uint32 SWITCH_CORE_PORT_SHAPER_PACKET_BASED_MAX_REFRESH_P0;
	uint8 dummy1149[28];
	uint32 SWITCH_CORE_PORT_SHAPER_PACKET_BASED_MAX_REFRESH_P1;
	uint8 dummy1150[28];
	uint32 SWITCH_CORE_PORT_SHAPER_PACKET_BASED_MAX_REFRESH_P2;
	uint8 dummy1151[28];
	uint32 SWITCH_CORE_PORT_SHAPER_PACKET_BASED_MAX_REFRESH_P3;
	uint8 dummy1152[28];
	uint32 SWITCH_CORE_PORT_SHAPER_PACKET_BASED_MAX_REFRESH_P4;
	uint8 dummy1153[28];
	uint32 SWITCH_CORE_PORT_SHAPER_PACKET_BASED_MAX_REFRESH_P5;
	uint8 dummy1154[28];
	uint32 SWITCH_CORE_PORT_SHAPER_PACKET_BASED_MAX_REFRESH_P6;
	uint8 dummy1155[28];
	uint32 SWITCH_CORE_PORT_SHAPER_PACKET_BASED_MAX_REFRESH_P7;
	uint8 dummy1156[28];
	uint32 SWITCH_CORE_PORT_SHAPER_PACKET_BASED_MAX_REFRESH_IMP;
	uint8 dummy1157[60];
	uint32 SWITCH_CORE_EGRESS_SHAPER_CTLREG_REG_SPARE0;
	uint8 dummy1158[28];
	uint32 SWITCH_CORE_EGRESS_SHAPER_CTLREG_REG_SPARE1;
	uint8 dummy1159[28];
	uint32 SWITCH_CORE_PORT_SHAPER_PACKET_BASED_MAX_THD_SEL_P0;
	uint8 dummy1160[28];
	uint32 SWITCH_CORE_PORT_SHAPER_PACKET_BASED_MAX_THD_SEL_P1;
	uint8 dummy1161[28];
	uint32 SWITCH_CORE_PORT_SHAPER_PACKET_BASED_MAX_THD_SEL_P2;
	uint8 dummy1162[28];
	uint32 SWITCH_CORE_PORT_SHAPER_PACKET_BASED_MAX_THD_SEL_P3;
	uint8 dummy1163[28];
	uint32 SWITCH_CORE_PORT_SHAPER_PACKET_BASED_MAX_THD_SEL_P4;
	uint8 dummy1164[28];
	uint32 SWITCH_CORE_PORT_SHAPER_PACKET_BASED_MAX_THD_SEL_P5;
	uint8 dummy1165[28];
	uint32 SWITCH_CORE_PORT_SHAPER_PACKET_BASED_MAX_THD_SEL_P6;
	uint8 dummy1166[28];
	uint32 SWITCH_CORE_PORT_SHAPER_PACKET_BASED_MAX_THD_SEL_P7;
	uint8 dummy1167[28];
	uint32 SWITCH_CORE_PORT_SHAPER_PACKET_BASED_MAX_THD_SEL_IMP;
	uint8 dummy1168[28];
	uint32 SWITCH_CORE_PORT_SHAPER_AVB_SHAPING_MODE;
	uint8 dummy1169[12];
	uint32 SWITCH_CORE_PORT_SHAPER_ENABLE;
	uint8 dummy1170[12];
	uint32 SWITCH_CORE_PORT_SHAPER_BUCKET_COUNT_SELECT;
	uint8 dummy1171[12];
	uint32 SWITCH_CORE_PORT_SHAPER_BLOCKING;
	uint8 dummy1172[28];
	uint32 SWITCH_CORE_IFG_BYTES;
	uint8 dummy1173[140];
	uint32 SWITCH_CORE_QUEUE0_MAX_REFRESH_P0;
	uint8 dummy1174[28];
	uint32 SWITCH_CORE_QUEUE0_MAX_REFRESH_P1;
	uint8 dummy1175[28];
	uint32 SWITCH_CORE_QUEUE0_MAX_REFRESH_P2;
	uint8 dummy1176[28];
	uint32 SWITCH_CORE_QUEUE0_MAX_REFRESH_P3;
	uint8 dummy1177[28];
	uint32 SWITCH_CORE_QUEUE0_MAX_REFRESH_P4;
	uint8 dummy1178[28];
	uint32 SWITCH_CORE_QUEUE0_MAX_REFRESH_P5;
	uint8 dummy1179[28];
	uint32 SWITCH_CORE_QUEUE0_MAX_REFRESH_P6;
	uint8 dummy1180[28];
	uint32 SWITCH_CORE_QUEUE0_MAX_REFRESH_P7;
	uint8 dummy1181[28];
	uint32 SWITCH_CORE_QUEUE0_MAX_REFRESH_IMP;
	uint8 dummy1182[124];
	uint32 SWITCH_CORE_QUEUE0_MAX_THD_SEL_P0;
	uint8 dummy1183[28];
	uint32 SWITCH_CORE_QUEUE0_MAX_THD_SEL_P1;
	uint8 dummy1184[28];
	uint32 SWITCH_CORE_QUEUE0_MAX_THD_SEL_P2;
	uint8 dummy1185[28];
	uint32 SWITCH_CORE_QUEUE0_MAX_THD_SEL_P3;
	uint8 dummy1186[28];
	uint32 SWITCH_CORE_QUEUE0_MAX_THD_SEL_P4;
	uint8 dummy1187[28];
	uint32 SWITCH_CORE_QUEUE0_MAX_THD_SEL_P5;
	uint8 dummy1188[28];
	uint32 SWITCH_CORE_QUEUE0_MAX_THD_SEL_P6;
	uint8 dummy1189[28];
	uint32 SWITCH_CORE_QUEUE0_MAX_THD_SEL_P7;
	uint8 dummy1190[28];
	uint32 SWITCH_CORE_QUEUE0_MAX_THD_SEL_IMP;
	uint8 dummy1191[124];
	uint32 SWITCH_CORE_QUEUE0_SHAPER_STS_P0;
	uint8 dummy1192[28];
	uint32 SWITCH_CORE_QUEUE0_SHAPER_STS_P1;
	uint8 dummy1193[28];
	uint32 SWITCH_CORE_QUEUE0_SHAPER_STS_P2;
	uint8 dummy1194[28];
	uint32 SWITCH_CORE_QUEUE0_SHAPER_STS_P3;
	uint8 dummy1195[28];
	uint32 SWITCH_CORE_QUEUE0_SHAPER_STS_P4;
	uint8 dummy1196[28];
	uint32 SWITCH_CORE_QUEUE0_SHAPER_STS_P5;
	uint8 dummy1197[28];
	uint32 SWITCH_CORE_QUEUE0_SHAPER_STS_P6;
	uint8 dummy1198[28];
	uint32 SWITCH_CORE_QUEUE0_SHAPER_STS_P7;
	uint8 dummy1199[28];
	uint32 SWITCH_CORE_QUEUE0_SHAPER_STS_IMP;
	uint8 dummy1200[124];
	uint32 SWITCH_CORE_QUEUE0_MAX_PACKET_REFRESH_P0;
	uint8 dummy1201[28];
	uint32 SWITCH_CORE_QUEUE0_MAX_PACKET_REFRESH_P1;
	uint8 dummy1202[28];
	uint32 SWITCH_CORE_QUEUE0_MAX_PACKET_REFRESH_P2;
	uint8 dummy1203[28];
	uint32 SWITCH_CORE_QUEUE0_MAX_PACKET_REFRESH_P3;
	uint8 dummy1204[28];
	uint32 SWITCH_CORE_QUEUE0_MAX_PACKET_REFRESH_P4;
	uint8 dummy1205[28];
	uint32 SWITCH_CORE_QUEUE0_MAX_PACKET_REFRESH_P5;
	uint8 dummy1206[28];
	uint32 SWITCH_CORE_QUEUE0_MAX_PACKET_REFRESH_P6;
	uint8 dummy1207[28];
	uint32 SWITCH_CORE_QUEUE0_MAX_PACKET_REFRESH_P7;
	uint8 dummy1208[28];
	uint32 SWITCH_CORE_QUEUE0_MAX_PACKET_REFRESH_IMP;
	uint8 dummy1209[60];
	uint32 SWITCH_CORE_EGRESS_SHAPER_Q0_CONFIG_REG_SPARE0;
	uint8 dummy1210[28];
	uint32 SWITCH_CORE_EGRESS_SHAPER_Q0_CONFIG_REG_SPARE1;
	uint8 dummy1211[28];
	uint32 SWITCH_CORE_QUEUE0_MAX_PACKET_THD_SEL_P0;
	uint8 dummy1212[28];
	uint32 SWITCH_CORE_QUEUE0_MAX_PACKET_THD_SEL_P1;
	uint8 dummy1213[28];
	uint32 SWITCH_CORE_QUEUE0_MAX_PACKET_THD_SEL_P2;
	uint8 dummy1214[28];
	uint32 SWITCH_CORE_QUEUE0_MAX_PACKET_THD_SEL_P3;
	uint8 dummy1215[28];
	uint32 SWITCH_CORE_QUEUE0_MAX_PACKET_THD_SEL_P4;
	uint8 dummy1216[28];
	uint32 SWITCH_CORE_QUEUE0_MAX_PACKET_THD_SEL_P5;
	uint8 dummy1217[28];
	uint32 SWITCH_CORE_QUEUE0_MAX_PACKET_THD_SEL_P6;
	uint8 dummy1218[28];
	uint32 SWITCH_CORE_QUEUE0_MAX_PACKET_THD_SEL_P7;
	uint8 dummy1219[28];
	uint32 SWITCH_CORE_QUEUE0_MAX_PACKET_THD_SEL_IMP;
	uint8 dummy1220[28];
	uint32 SWITCH_CORE_QUEUE0_AVB_SHAPING_MODE;
	uint8 dummy1221[12];
	uint32 SWITCH_CORE_QUEUE0_SHAPER_ENABLE;
	uint8 dummy1222[12];
	uint32 SWITCH_CORE_QUEUE0_SHAPER_BUCKET_COUNT_SELECT;
	uint8 dummy1223[12];
	uint32 SWITCH_CORE_QUEUE0_SHAPER_BLOCKING;
	uint8 dummy1224[172];
	uint32 SWITCH_CORE_QUEUE1_MAX_REFRESH_P0;
	uint8 dummy1225[28];
	uint32 SWITCH_CORE_QUEUE1_MAX_REFRESH_P1;
	uint8 dummy1226[28];
	uint32 SWITCH_CORE_QUEUE1_MAX_REFRESH_P2;
	uint8 dummy1227[28];
	uint32 SWITCH_CORE_QUEUE1_MAX_REFRESH_P3;
	uint8 dummy1228[28];
	uint32 SWITCH_CORE_QUEUE1_MAX_REFRESH_P4;
	uint8 dummy1229[28];
	uint32 SWITCH_CORE_QUEUE1_MAX_REFRESH_P5;
	uint8 dummy1230[28];
	uint32 SWITCH_CORE_QUEUE1_MAX_REFRESH_P6;
	uint8 dummy1231[28];
	uint32 SWITCH_CORE_QUEUE1_MAX_REFRESH_P7;
	uint8 dummy1232[28];
	uint32 SWITCH_CORE_QUEUE1_MAX_REFRESH_IMP;
	uint8 dummy1233[124];
	uint32 SWITCH_CORE_QUEUE1_MAX_THD_SEL_P0;
	uint8 dummy1234[28];
	uint32 SWITCH_CORE_QUEUE1_MAX_THD_SEL_P1;
	uint8 dummy1235[28];
	uint32 SWITCH_CORE_QUEUE1_MAX_THD_SEL_P2;
	uint8 dummy1236[28];
	uint32 SWITCH_CORE_QUEUE1_MAX_THD_SEL_P3;
	uint8 dummy1237[28];
	uint32 SWITCH_CORE_QUEUE1_MAX_THD_SEL_P4;
	uint8 dummy1238[28];
	uint32 SWITCH_CORE_QUEUE1_MAX_THD_SEL_P5;
	uint8 dummy1239[28];
	uint32 SWITCH_CORE_QUEUE1_MAX_THD_SEL_P6;
	uint8 dummy1240[28];
	uint32 SWITCH_CORE_QUEUE1_MAX_THD_SEL_P7;
	uint8 dummy1241[28];
	uint32 SWITCH_CORE_QUEUE1_MAX_THD_SEL_IMP;
	uint8 dummy1242[124];
	uint32 SWITCH_CORE_QUEUE1_SHAPER_STS_P0;
	uint8 dummy1243[28];
	uint32 SWITCH_CORE_QUEUE1_SHAPER_STS_P1;
	uint8 dummy1244[28];
	uint32 SWITCH_CORE_QUEUE1_SHAPER_STS_P2;
	uint8 dummy1245[28];
	uint32 SWITCH_CORE_QUEUE1_SHAPER_STS_P3;
	uint8 dummy1246[28];
	uint32 SWITCH_CORE_QUEUE1_SHAPER_STS_P4;
	uint8 dummy1247[28];
	uint32 SWITCH_CORE_QUEUE1_SHAPER_STS_P5;
	uint8 dummy1248[28];
	uint32 SWITCH_CORE_QUEUE1_SHAPER_STS_P6;
	uint8 dummy1249[28];
	uint32 SWITCH_CORE_QUEUE1_SHAPER_STS_P7;
	uint8 dummy1250[28];
	uint32 SWITCH_CORE_QUEUE1_SHAPER_STS_IMP;
	uint8 dummy1251[124];
	uint32 SWITCH_CORE_QUEUE1_MAX_PACKET_REFRESH_P0;
	uint8 dummy1252[28];
	uint32 SWITCH_CORE_QUEUE1_MAX_PACKET_REFRESH_P1;
	uint8 dummy1253[28];
	uint32 SWITCH_CORE_QUEUE1_MAX_PACKET_REFRESH_P2;
	uint8 dummy1254[28];
	uint32 SWITCH_CORE_QUEUE1_MAX_PACKET_REFRESH_P3;
	uint8 dummy1255[28];
	uint32 SWITCH_CORE_QUEUE1_MAX_PACKET_REFRESH_P4;
	uint8 dummy1256[28];
	uint32 SWITCH_CORE_QUEUE1_MAX_PACKET_REFRESH_P5;
	uint8 dummy1257[28];
	uint32 SWITCH_CORE_QUEUE1_MAX_PACKET_REFRESH_P6;
	uint8 dummy1258[28];
	uint32 SWITCH_CORE_QUEUE1_MAX_PACKET_REFRESH_P7;
	uint8 dummy1259[28];
	uint32 SWITCH_CORE_QUEUE1_MAX_PACKET_REFRESH_IMP;
	uint8 dummy1260[60];
	uint32 SWITCH_CORE_EGRESS_SHAPER_Q1_CONFIG_REG_SPARE0;
	uint8 dummy1261[28];
	uint32 SWITCH_CORE_EGRESS_SHAPER_Q1_CONFIG_REG_SPARE1;
	uint8 dummy1262[28];
	uint32 SWITCH_CORE_QUEUE1_MAX_PACKET_THD_SEL_P0;
	uint8 dummy1263[28];
	uint32 SWITCH_CORE_QUEUE1_MAX_PACKET_THD_SEL_P1;
	uint8 dummy1264[28];
	uint32 SWITCH_CORE_QUEUE1_MAX_PACKET_THD_SEL_P2;
	uint8 dummy1265[28];
	uint32 SWITCH_CORE_QUEUE1_MAX_PACKET_THD_SEL_P3;
	uint8 dummy1266[28];
	uint32 SWITCH_CORE_QUEUE1_MAX_PACKET_THD_SEL_P4;
	uint8 dummy1267[28];
	uint32 SWITCH_CORE_QUEUE1_MAX_PACKET_THD_SEL_P5;
	uint8 dummy1268[28];
	uint32 SWITCH_CORE_QUEUE1_MAX_PACKET_THD_SEL_P6;
	uint8 dummy1269[28];
	uint32 SWITCH_CORE_QUEUE1_MAX_PACKET_THD_SEL_P7;
	uint8 dummy1270[28];
	uint32 SWITCH_CORE_QUEUE1_MAX_PACKET_THD_SEL_IMP;
	uint8 dummy1271[28];
	uint32 SWITCH_CORE_QUEUE1_AVB_SHAPING_MODE;
	uint8 dummy1272[12];
	uint32 SWITCH_CORE_QUEUE1_SHAPER_ENABLE;
	uint8 dummy1273[12];
	uint32 SWITCH_CORE_QUEUE1_SHAPER_BUCKET_COUNT_SELECT;
	uint8 dummy1274[12];
	uint32 SWITCH_CORE_QUEUE1_SHAPER_BLOCKING;
	uint8 dummy1275[172];
	uint32 SWITCH_CORE_QUEUE2_MAX_REFRESH_P0;
	uint8 dummy1276[28];
	uint32 SWITCH_CORE_QUEUE2_MAX_REFRESH_P1;
	uint8 dummy1277[28];
	uint32 SWITCH_CORE_QUEUE2_MAX_REFRESH_P2;
	uint8 dummy1278[28];
	uint32 SWITCH_CORE_QUEUE2_MAX_REFRESH_P3;
	uint8 dummy1279[28];
	uint32 SWITCH_CORE_QUEUE2_MAX_REFRESH_P4;
	uint8 dummy1280[28];
	uint32 SWITCH_CORE_QUEUE2_MAX_REFRESH_P5;
	uint8 dummy1281[28];
	uint32 SWITCH_CORE_QUEUE2_MAX_REFRESH_P6;
	uint8 dummy1282[28];
	uint32 SWITCH_CORE_QUEUE2_MAX_REFRESH_P7;
	uint8 dummy1283[28];
	uint32 SWITCH_CORE_QUEUE2_MAX_REFRESH_IMP;
	uint8 dummy1284[124];
	uint32 SWITCH_CORE_QUEUE2_MAX_THD_SEL_P0;
	uint8 dummy1285[28];
	uint32 SWITCH_CORE_QUEUE2_MAX_THD_SEL_P1;
	uint8 dummy1286[28];
	uint32 SWITCH_CORE_QUEUE2_MAX_THD_SEL_P2;
	uint8 dummy1287[28];
	uint32 SWITCH_CORE_QUEUE2_MAX_THD_SEL_P3;
	uint8 dummy1288[28];
	uint32 SWITCH_CORE_QUEUE2_MAX_THD_SEL_P4;
	uint8 dummy1289[28];
	uint32 SWITCH_CORE_QUEUE2_MAX_THD_SEL_P5;
	uint8 dummy1290[28];
	uint32 SWITCH_CORE_QUEUE2_MAX_THD_SEL_P6;
	uint8 dummy1291[28];
	uint32 SWITCH_CORE_QUEUE2_MAX_THD_SEL_P7;
	uint8 dummy1292[28];
	uint32 SWITCH_CORE_QUEUE2_MAX_THD_SEL_IMP;
	uint8 dummy1293[124];
	uint32 SWITCH_CORE_QUEUE2_SHAPER_STS_P0;
	uint8 dummy1294[28];
	uint32 SWITCH_CORE_QUEUE2_SHAPER_STS_P1;
	uint8 dummy1295[28];
	uint32 SWITCH_CORE_QUEUE2_SHAPER_STS_P2;
	uint8 dummy1296[28];
	uint32 SWITCH_CORE_QUEUE2_SHAPER_STS_P3;
	uint8 dummy1297[28];
	uint32 SWITCH_CORE_QUEUE2_SHAPER_STS_P4;
	uint8 dummy1298[28];
	uint32 SWITCH_CORE_QUEUE2_SHAPER_STS_P5;
	uint8 dummy1299[28];
	uint32 SWITCH_CORE_QUEUE2_SHAPER_STS_P6;
	uint8 dummy1300[28];
	uint32 SWITCH_CORE_QUEUE2_SHAPER_STS_P7;
	uint8 dummy1301[28];
	uint32 SWITCH_CORE_QUEUE2_SHAPER_STS_IMP;
	uint8 dummy1302[124];
	uint32 SWITCH_CORE_QUEUE2_MAX_PACKET_REFRESH_P0;
	uint8 dummy1303[28];
	uint32 SWITCH_CORE_QUEUE2_MAX_PACKET_REFRESH_P1;
	uint8 dummy1304[28];
	uint32 SWITCH_CORE_QUEUE2_MAX_PACKET_REFRESH_P2;
	uint8 dummy1305[28];
	uint32 SWITCH_CORE_QUEUE2_MAX_PACKET_REFRESH_P3;
	uint8 dummy1306[28];
	uint32 SWITCH_CORE_QUEUE2_MAX_PACKET_REFRESH_P4;
	uint8 dummy1307[28];
	uint32 SWITCH_CORE_QUEUE2_MAX_PACKET_REFRESH_P5;
	uint8 dummy1308[28];
	uint32 SWITCH_CORE_QUEUE2_MAX_PACKET_REFRESH_P6;
	uint8 dummy1309[28];
	uint32 SWITCH_CORE_QUEUE2_MAX_PACKET_REFRESH_P7;
	uint8 dummy1310[28];
	uint32 SWITCH_CORE_QUEUE2_MAX_PACKET_REFRESH_IMP;
	uint8 dummy1311[60];
	uint32 SWITCH_CORE_EGRESS_SHAPER_Q2_CONFIG_REG_SPARE0;
	uint8 dummy1312[28];
	uint32 SWITCH_CORE_EGRESS_SHAPER_Q2_CONFIG_REG_SPARE1;
	uint8 dummy1313[28];
	uint32 SWITCH_CORE_QUEUE2_MAX_PACKET_THD_SEL_P0;
	uint8 dummy1314[28];
	uint32 SWITCH_CORE_QUEUE2_MAX_PACKET_THD_SEL_P1;
	uint8 dummy1315[28];
	uint32 SWITCH_CORE_QUEUE2_MAX_PACKET_THD_SEL_P2;
	uint8 dummy1316[28];
	uint32 SWITCH_CORE_QUEUE2_MAX_PACKET_THD_SEL_P3;
	uint8 dummy1317[28];
	uint32 SWITCH_CORE_QUEUE2_MAX_PACKET_THD_SEL_P4;
	uint8 dummy1318[28];
	uint32 SWITCH_CORE_QUEUE2_MAX_PACKET_THD_SEL_P5;
	uint8 dummy1319[28];
	uint32 SWITCH_CORE_QUEUE2_MAX_PACKET_THD_SEL_P6;
	uint8 dummy1320[28];
	uint32 SWITCH_CORE_QUEUE2_MAX_PACKET_THD_SEL_P7;
	uint8 dummy1321[28];
	uint32 SWITCH_CORE_QUEUE2_MAX_PACKET_THD_SEL_IMP;
	uint8 dummy1322[28];
	uint32 SWITCH_CORE_QUEUE2_AVB_SHAPING_MODE;
	uint8 dummy1323[12];
	uint32 SWITCH_CORE_QUEUE2_SHAPER_ENABLE;
	uint8 dummy1324[12];
	uint32 SWITCH_CORE_QUEUE2_SHAPER_BUCKET_COUNT_SELECT;
	uint8 dummy1325[12];
	uint32 SWITCH_CORE_QUEUE2_SHAPER_BLOCKING;
	uint8 dummy1326[172];
	uint32 SWITCH_CORE_QUEUE3_MAX_REFRESH_P0;
	uint8 dummy1327[28];
	uint32 SWITCH_CORE_QUEUE3_MAX_REFRESH_P1;
	uint8 dummy1328[28];
	uint32 SWITCH_CORE_QUEUE3_MAX_REFRESH_P2;
	uint8 dummy1329[28];
	uint32 SWITCH_CORE_QUEUE3_MAX_REFRESH_P3;
	uint8 dummy1330[28];
	uint32 SWITCH_CORE_QUEUE3_MAX_REFRESH_P4;
	uint8 dummy1331[28];
	uint32 SWITCH_CORE_QUEUE3_MAX_REFRESH_P5;
	uint8 dummy1332[28];
	uint32 SWITCH_CORE_QUEUE3_MAX_REFRESH_P6;
	uint8 dummy1333[28];
	uint32 SWITCH_CORE_QUEUE3_MAX_REFRESH_P7;
	uint8 dummy1334[28];
	uint32 SWITCH_CORE_QUEUE3_MAX_REFRESH_IMP;
	uint8 dummy1335[124];
	uint32 SWITCH_CORE_QUEUE3_MAX_THD_SEL_P0;
	uint8 dummy1336[28];
	uint32 SWITCH_CORE_QUEUE3_MAX_THD_SEL_P1;
	uint8 dummy1337[28];
	uint32 SWITCH_CORE_QUEUE3_MAX_THD_SEL_P2;
	uint8 dummy1338[28];
	uint32 SWITCH_CORE_QUEUE3_MAX_THD_SEL_P3;
	uint8 dummy1339[28];
	uint32 SWITCH_CORE_QUEUE3_MAX_THD_SEL_P4;
	uint8 dummy1340[28];
	uint32 SWITCH_CORE_QUEUE3_MAX_THD_SEL_P5;
	uint8 dummy1341[28];
	uint32 SWITCH_CORE_QUEUE3_MAX_THD_SEL_P6;
	uint8 dummy1342[28];
	uint32 SWITCH_CORE_QUEUE3_MAX_THD_SEL_P7;
	uint8 dummy1343[28];
	uint32 SWITCH_CORE_QUEUE3_MAX_THD_SEL_IMP;
	uint8 dummy1344[124];
	uint32 SWITCH_CORE_QUEUE3_SHAPER_STS_P0;
	uint8 dummy1345[28];
	uint32 SWITCH_CORE_QUEUE3_SHAPER_STS_P1;
	uint8 dummy1346[28];
	uint32 SWITCH_CORE_QUEUE3_SHAPER_STS_P2;
	uint8 dummy1347[28];
	uint32 SWITCH_CORE_QUEUE3_SHAPER_STS_P3;
	uint8 dummy1348[28];
	uint32 SWITCH_CORE_QUEUE3_SHAPER_STS_P4;
	uint8 dummy1349[28];
	uint32 SWITCH_CORE_QUEUE3_SHAPER_STS_P5;
	uint8 dummy1350[28];
	uint32 SWITCH_CORE_QUEUE3_SHAPER_STS_P6;
	uint8 dummy1351[28];
	uint32 SWITCH_CORE_QUEUE3_SHAPER_STS_P7;
	uint8 dummy1352[28];
	uint32 SWITCH_CORE_QUEUE3_SHAPER_STS_IMP;
	uint8 dummy1353[124];
	uint32 SWITCH_CORE_QUEUE3_MAX_PACKET_REFRESH_P0;
	uint8 dummy1354[28];
	uint32 SWITCH_CORE_QUEUE3_MAX_PACKET_REFRESH_P1;
	uint8 dummy1355[28];
	uint32 SWITCH_CORE_QUEUE3_MAX_PACKET_REFRESH_P2;
	uint8 dummy1356[28];
	uint32 SWITCH_CORE_QUEUE3_MAX_PACKET_REFRESH_P3;
	uint8 dummy1357[28];
	uint32 SWITCH_CORE_QUEUE3_MAX_PACKET_REFRESH_P4;
	uint8 dummy1358[28];
	uint32 SWITCH_CORE_QUEUE3_MAX_PACKET_REFRESH_P5;
	uint8 dummy1359[28];
	uint32 SWITCH_CORE_QUEUE3_MAX_PACKET_REFRESH_P6;
	uint8 dummy1360[28];
	uint32 SWITCH_CORE_QUEUE3_MAX_PACKET_REFRESH_P7;
	uint8 dummy1361[28];
	uint32 SWITCH_CORE_QUEUE3_MAX_PACKET_REFRESH_IMP;
	uint8 dummy1362[60];
	uint32 SWITCH_CORE_EGRESS_SHAPER_Q3_CONFIG_REG_SPARE0;
	uint8 dummy1363[28];
	uint32 SWITCH_CORE_EGRESS_SHAPER_Q3_CONFIG_REG_SPARE1;
	uint8 dummy1364[28];
	uint32 SWITCH_CORE_QUEUE3_MAX_PACKET_THD_SEL_P0;
	uint8 dummy1365[28];
	uint32 SWITCH_CORE_QUEUE3_MAX_PACKET_THD_SEL_P1;
	uint8 dummy1366[28];
	uint32 SWITCH_CORE_QUEUE3_MAX_PACKET_THD_SEL_P2;
	uint8 dummy1367[28];
	uint32 SWITCH_CORE_QUEUE3_MAX_PACKET_THD_SEL_P3;
	uint8 dummy1368[28];
	uint32 SWITCH_CORE_QUEUE3_MAX_PACKET_THD_SEL_P4;
	uint8 dummy1369[28];
	uint32 SWITCH_CORE_QUEUE3_MAX_PACKET_THD_SEL_P5;
	uint8 dummy1370[28];
	uint32 SWITCH_CORE_QUEUE3_MAX_PACKET_THD_SEL_P6;
	uint8 dummy1371[28];
	uint32 SWITCH_CORE_QUEUE3_MAX_PACKET_THD_SEL_P7;
	uint8 dummy1372[28];
	uint32 SWITCH_CORE_QUEUE3_MAX_PACKET_THD_SEL_IMP;
	uint8 dummy1373[28];
	uint32 SWITCH_CORE_QUEUE3_AVB_SHAPING_MODE;
	uint8 dummy1374[12];
	uint32 SWITCH_CORE_QUEUE3_SHAPER_ENABLE;
	uint8 dummy1375[12];
	uint32 SWITCH_CORE_QUEUE3_SHAPER_BUCKET_COUNT_SELECT;
	uint8 dummy1376[12];
	uint32 SWITCH_CORE_QUEUE3_SHAPER_BLOCKING;
	uint8 dummy1377[172];
	uint32 SWITCH_CORE_QUEUE4_MAX_REFRESH_P0;
	uint8 dummy1378[28];
	uint32 SWITCH_CORE_QUEUE4_MAX_REFRESH_P1;
	uint8 dummy1379[28];
	uint32 SWITCH_CORE_QUEUE4_MAX_REFRESH_P2;
	uint8 dummy1380[28];
	uint32 SWITCH_CORE_QUEUE4_MAX_REFRESH_P3;
	uint8 dummy1381[28];
	uint32 SWITCH_CORE_QUEUE4_MAX_REFRESH_P4;
	uint8 dummy1382[28];
	uint32 SWITCH_CORE_QUEUE4_MAX_REFRESH_P5;
	uint8 dummy1383[28];
	uint32 SWITCH_CORE_QUEUE4_MAX_REFRESH_P6;
	uint8 dummy1384[28];
	uint32 SWITCH_CORE_QUEUE4_MAX_REFRESH_P7;
	uint8 dummy1385[28];
	uint32 SWITCH_CORE_QUEUE4_MAX_REFRESH_IMP;
	uint8 dummy1386[124];
	uint32 SWITCH_CORE_QUEUE4_MAX_THD_SEL_P0;
	uint8 dummy1387[28];
	uint32 SWITCH_CORE_QUEUE4_MAX_THD_SEL_P1;
	uint8 dummy1388[28];
	uint32 SWITCH_CORE_QUEUE4_MAX_THD_SEL_P2;
	uint8 dummy1389[28];
	uint32 SWITCH_CORE_QUEUE4_MAX_THD_SEL_P3;
	uint8 dummy1390[28];
	uint32 SWITCH_CORE_QUEUE4_MAX_THD_SEL_P4;
	uint8 dummy1391[28];
	uint32 SWITCH_CORE_QUEUE4_MAX_THD_SEL_P5;
	uint8 dummy1392[28];
	uint32 SWITCH_CORE_QUEUE4_MAX_THD_SEL_P6;
	uint8 dummy1393[28];
	uint32 SWITCH_CORE_QUEUE4_MAX_THD_SEL_P7;
	uint8 dummy1394[28];
	uint32 SWITCH_CORE_QUEUE4_MAX_THD_SEL_IMP;
	uint8 dummy1395[124];
	uint32 SWITCH_CORE_QUEUE4_SHAPER_STS_P0;
	uint8 dummy1396[28];
	uint32 SWITCH_CORE_QUEUE4_SHAPER_STS_P1;
	uint8 dummy1397[28];
	uint32 SWITCH_CORE_QUEUE4_SHAPER_STS_P2;
	uint8 dummy1398[28];
	uint32 SWITCH_CORE_QUEUE4_SHAPER_STS_P3;
	uint8 dummy1399[28];
	uint32 SWITCH_CORE_QUEUE4_SHAPER_STS_P4;
	uint8 dummy1400[28];
	uint32 SWITCH_CORE_QUEUE4_SHAPER_STS_P5;
	uint8 dummy1401[28];
	uint32 SWITCH_CORE_QUEUE4_SHAPER_STS_P6;
	uint8 dummy1402[28];
	uint32 SWITCH_CORE_QUEUE4_SHAPER_STS_P7;
	uint8 dummy1403[28];
	uint32 SWITCH_CORE_QUEUE4_SHAPER_STS_IMP;
	uint8 dummy1404[124];
	uint32 SWITCH_CORE_QUEUE4_MAX_PACKET_REFRESH_P0;
	uint8 dummy1405[28];
	uint32 SWITCH_CORE_QUEUE4_MAX_PACKET_REFRESH_P1;
	uint8 dummy1406[28];
	uint32 SWITCH_CORE_QUEUE4_MAX_PACKET_REFRESH_P2;
	uint8 dummy1407[28];
	uint32 SWITCH_CORE_QUEUE4_MAX_PACKET_REFRESH_P3;
	uint8 dummy1408[28];
	uint32 SWITCH_CORE_QUEUE4_MAX_PACKET_REFRESH_P4;
	uint8 dummy1409[28];
	uint32 SWITCH_CORE_QUEUE4_MAX_PACKET_REFRESH_P5;
	uint8 dummy1410[28];
	uint32 SWITCH_CORE_QUEUE4_MAX_PACKET_REFRESH_P6;
	uint8 dummy1411[28];
	uint32 SWITCH_CORE_QUEUE4_MAX_PACKET_REFRESH_P7;
	uint8 dummy1412[28];
	uint32 SWITCH_CORE_QUEUE4_MAX_PACKET_REFRESH_IMP;
	uint8 dummy1413[60];
	uint32 SWITCH_CORE_EGRESS_SHAPER_Q4_CONFIG_REG_SPARE0;
	uint8 dummy1414[28];
	uint32 SWITCH_CORE_EGRESS_SHAPER_Q4_CONFIG_REG_SPARE1;
	uint8 dummy1415[28];
	uint32 SWITCH_CORE_QUEUE4_MAX_PACKET_THD_SEL_P0;
	uint8 dummy1416[28];
	uint32 SWITCH_CORE_QUEUE4_MAX_PACKET_THD_SEL_P1;
	uint8 dummy1417[28];
	uint32 SWITCH_CORE_QUEUE4_MAX_PACKET_THD_SEL_P2;
	uint8 dummy1418[28];
	uint32 SWITCH_CORE_QUEUE4_MAX_PACKET_THD_SEL_P3;
	uint8 dummy1419[28];
	uint32 SWITCH_CORE_QUEUE4_MAX_PACKET_THD_SEL_P4;
	uint8 dummy1420[28];
	uint32 SWITCH_CORE_QUEUE4_MAX_PACKET_THD_SEL_P5;
	uint8 dummy1421[28];
	uint32 SWITCH_CORE_QUEUE4_MAX_PACKET_THD_SEL_P6;
	uint8 dummy1422[28];
	uint32 SWITCH_CORE_QUEUE4_MAX_PACKET_THD_SEL_P7;
	uint8 dummy1423[28];
	uint32 SWITCH_CORE_QUEUE4_MAX_PACKET_THD_SEL_IMP;
	uint8 dummy1424[28];
	uint32 SWITCH_CORE_QUEUE4_AVB_SHAPING_MODE;
	uint8 dummy1425[12];
	uint32 SWITCH_CORE_QUEUE4_SHAPER_ENABLE;
	uint8 dummy1426[12];
	uint32 SWITCH_CORE_QUEUE4_SHAPER_BUCKET_COUNT_SELECT;
	uint8 dummy1427[12];
	uint32 SWITCH_CORE_QUEUE4_SHAPER_BLOCKING;
	uint8 dummy1428[172];
	uint32 SWITCH_CORE_QUEUE5_MAX_REFRESH_P0;
	uint8 dummy1429[28];
	uint32 SWITCH_CORE_QUEUE5_MAX_REFRESH_P1;
	uint8 dummy1430[28];
	uint32 SWITCH_CORE_QUEUE5_MAX_REFRESH_P2;
	uint8 dummy1431[28];
	uint32 SWITCH_CORE_QUEUE5_MAX_REFRESH_P3;
	uint8 dummy1432[28];
	uint32 SWITCH_CORE_QUEUE5_MAX_REFRESH_P4;
	uint8 dummy1433[28];
	uint32 SWITCH_CORE_QUEUE5_MAX_REFRESH_P5;
	uint8 dummy1434[28];
	uint32 SWITCH_CORE_QUEUE5_MAX_REFRESH_P6;
	uint8 dummy1435[28];
	uint32 SWITCH_CORE_QUEUE5_MAX_REFRESH_P7;
	uint8 dummy1436[28];
	uint32 SWITCH_CORE_QUEUE5_MAX_REFRESH_IMP;
	uint8 dummy1437[124];
	uint32 SWITCH_CORE_QUEUE5_MAX_THD_SEL_P0;
	uint8 dummy1438[28];
	uint32 SWITCH_CORE_QUEUE5_MAX_THD_SEL_P1;
	uint8 dummy1439[28];
	uint32 SWITCH_CORE_QUEUE5_MAX_THD_SEL_P2;
	uint8 dummy1440[28];
	uint32 SWITCH_CORE_QUEUE5_MAX_THD_SEL_P3;
	uint8 dummy1441[28];
	uint32 SWITCH_CORE_QUEUE5_MAX_THD_SEL_P4;
	uint8 dummy1442[28];
	uint32 SWITCH_CORE_QUEUE5_MAX_THD_SEL_P5;
	uint8 dummy1443[28];
	uint32 SWITCH_CORE_QUEUE5_MAX_THD_SEL_P6;
	uint8 dummy1444[28];
	uint32 SWITCH_CORE_QUEUE5_MAX_THD_SEL_P7;
	uint8 dummy1445[28];
	uint32 SWITCH_CORE_QUEUE5_MAX_THD_SEL_IMP;
	uint8 dummy1446[124];
	uint32 SWITCH_CORE_QUEUE5_SHAPER_STS_P0;
	uint8 dummy1447[28];
	uint32 SWITCH_CORE_QUEUE5_SHAPER_STS_P1;
	uint8 dummy1448[28];
	uint32 SWITCH_CORE_QUEUE5_SHAPER_STS_P2;
	uint8 dummy1449[28];
	uint32 SWITCH_CORE_QUEUE5_SHAPER_STS_P3;
	uint8 dummy1450[28];
	uint32 SWITCH_CORE_QUEUE5_SHAPER_STS_P4;
	uint8 dummy1451[28];
	uint32 SWITCH_CORE_QUEUE5_SHAPER_STS_P5;
	uint8 dummy1452[28];
	uint32 SWITCH_CORE_QUEUE5_SHAPER_STS_P6;
	uint8 dummy1453[28];
	uint32 SWITCH_CORE_QUEUE5_SHAPER_STS_P7;
	uint8 dummy1454[28];
	uint32 SWITCH_CORE_QUEUE5_SHAPER_STS_IMP;
	uint8 dummy1455[124];
	uint32 SWITCH_CORE_QUEUE5_MAX_PACKET_REFRESH_P0;
	uint8 dummy1456[28];
	uint32 SWITCH_CORE_QUEUE5_MAX_PACKET_REFRESH_P1;
	uint8 dummy1457[28];
	uint32 SWITCH_CORE_QUEUE5_MAX_PACKET_REFRESH_P2;
	uint8 dummy1458[28];
	uint32 SWITCH_CORE_QUEUE5_MAX_PACKET_REFRESH_P3;
	uint8 dummy1459[28];
	uint32 SWITCH_CORE_QUEUE5_MAX_PACKET_REFRESH_P4;
	uint8 dummy1460[28];
	uint32 SWITCH_CORE_QUEUE5_MAX_PACKET_REFRESH_P5;
	uint8 dummy1461[28];
	uint32 SWITCH_CORE_QUEUE5_MAX_PACKET_REFRESH_P6;
	uint8 dummy1462[28];
	uint32 SWITCH_CORE_QUEUE5_MAX_PACKET_REFRESH_P7;
	uint8 dummy1463[28];
	uint32 SWITCH_CORE_QUEUE5_MAX_PACKET_REFRESH_IMP;
	uint8 dummy1464[60];
	uint32 SWITCH_CORE_EGRESS_SHAPER_Q5_CONFIG_REG_SPARE0;
	uint8 dummy1465[28];
	uint32 SWITCH_CORE_EGRESS_SHAPER_Q5_CONFIG_REG_SPARE1;
	uint8 dummy1466[28];
	uint32 SWITCH_CORE_QUEUE5_MAX_PACKET_THD_SEL_P0;
	uint8 dummy1467[28];
	uint32 SWITCH_CORE_QUEUE5_MAX_PACKET_THD_SEL_P1;
	uint8 dummy1468[28];
	uint32 SWITCH_CORE_QUEUE5_MAX_PACKET_THD_SEL_P2;
	uint8 dummy1469[28];
	uint32 SWITCH_CORE_QUEUE5_MAX_PACKET_THD_SEL_P3;
	uint8 dummy1470[28];
	uint32 SWITCH_CORE_QUEUE5_MAX_PACKET_THD_SEL_P4;
	uint8 dummy1471[28];
	uint32 SWITCH_CORE_QUEUE5_MAX_PACKET_THD_SEL_P5;
	uint8 dummy1472[28];
	uint32 SWITCH_CORE_QUEUE5_MAX_PACKET_THD_SEL_P6;
	uint8 dummy1473[28];
	uint32 SWITCH_CORE_QUEUE5_MAX_PACKET_THD_SEL_P7;
	uint8 dummy1474[28];
	uint32 SWITCH_CORE_QUEUE5_MAX_PACKET_THD_SEL_IMP;
	uint8 dummy1475[28];
	uint32 SWITCH_CORE_QUEUE5_AVB_SHAPING_MODE;
	uint8 dummy1476[12];
	uint32 SWITCH_CORE_QUEUE5_SHAPER_ENABLE;
	uint8 dummy1477[12];
	uint32 SWITCH_CORE_QUEUE5_SHAPER_BUCKET_COUNT_SELECT;
	uint8 dummy1478[12];
	uint32 SWITCH_CORE_QUEUE5_SHAPER_BLOCKING;
	uint8 dummy1479[172];
	uint32 SWITCH_CORE_QUEUE6_MAX_REFRESH_P0;
	uint8 dummy1480[28];
	uint32 SWITCH_CORE_QUEUE6_MAX_REFRESH_P1;
	uint8 dummy1481[28];
	uint32 SWITCH_CORE_QUEUE6_MAX_REFRESH_P2;
	uint8 dummy1482[28];
	uint32 SWITCH_CORE_QUEUE6_MAX_REFRESH_P3;
	uint8 dummy1483[28];
	uint32 SWITCH_CORE_QUEUE6_MAX_REFRESH_P4;
	uint8 dummy1484[28];
	uint32 SWITCH_CORE_QUEUE6_MAX_REFRESH_P5;
	uint8 dummy1485[28];
	uint32 SWITCH_CORE_QUEUE6_MAX_REFRESH_P6;
	uint8 dummy1486[28];
	uint32 SWITCH_CORE_QUEUE6_MAX_REFRESH_P7;
	uint8 dummy1487[28];
	uint32 SWITCH_CORE_QUEUE6_MAX_REFRESH_IMP;
	uint8 dummy1488[124];
	uint32 SWITCH_CORE_QUEUE6_MAX_THD_SEL_P0;
	uint8 dummy1489[28];
	uint32 SWITCH_CORE_QUEUE6_MAX_THD_SEL_P1;
	uint8 dummy1490[28];
	uint32 SWITCH_CORE_QUEUE6_MAX_THD_SEL_P2;
	uint8 dummy1491[28];
	uint32 SWITCH_CORE_QUEUE6_MAX_THD_SEL_P3;
	uint8 dummy1492[28];
	uint32 SWITCH_CORE_QUEUE6_MAX_THD_SEL_P4;
	uint8 dummy1493[28];
	uint32 SWITCH_CORE_QUEUE6_MAX_THD_SEL_P5;
	uint8 dummy1494[28];
	uint32 SWITCH_CORE_QUEUE6_MAX_THD_SEL_P6;
	uint8 dummy1495[28];
	uint32 SWITCH_CORE_QUEUE6_MAX_THD_SEL_P7;
	uint8 dummy1496[28];
	uint32 SWITCH_CORE_QUEUE6_MAX_THD_SEL_IMP;
	uint8 dummy1497[124];
	uint32 SWITCH_CORE_QUEUE6_SHAPER_STS_P0;
	uint8 dummy1498[28];
	uint32 SWITCH_CORE_QUEUE6_SHAPER_STS_P1;
	uint8 dummy1499[28];
	uint32 SWITCH_CORE_QUEUE6_SHAPER_STS_P2;
	uint8 dummy1500[28];
	uint32 SWITCH_CORE_QUEUE6_SHAPER_STS_P3;
	uint8 dummy1501[28];
	uint32 SWITCH_CORE_QUEUE6_SHAPER_STS_P4;
	uint8 dummy1502[28];
	uint32 SWITCH_CORE_QUEUE6_SHAPER_STS_P5;
	uint8 dummy1503[28];
	uint32 SWITCH_CORE_QUEUE6_SHAPER_STS_P6;
	uint8 dummy1504[28];
	uint32 SWITCH_CORE_QUEUE6_SHAPER_STS_P7;
	uint8 dummy1505[28];
	uint32 SWITCH_CORE_QUEUE6_SHAPER_STS_IMP;
	uint8 dummy1506[124];
	uint32 SWITCH_CORE_QUEUE6_MAX_PACKET_REFRESH_P0;
	uint8 dummy1507[28];
	uint32 SWITCH_CORE_QUEUE6_MAX_PACKET_REFRESH_P1;
	uint8 dummy1508[28];
	uint32 SWITCH_CORE_QUEUE6_MAX_PACKET_REFRESH_P2;
	uint8 dummy1509[28];
	uint32 SWITCH_CORE_QUEUE6_MAX_PACKET_REFRESH_P3;
	uint8 dummy1510[28];
	uint32 SWITCH_CORE_QUEUE6_MAX_PACKET_REFRESH_P4;
	uint8 dummy1511[28];
	uint32 SWITCH_CORE_QUEUE6_MAX_PACKET_REFRESH_P5;
	uint8 dummy1512[28];
	uint32 SWITCH_CORE_QUEUE6_MAX_PACKET_REFRESH_P6;
	uint8 dummy1513[28];
	uint32 SWITCH_CORE_QUEUE6_MAX_PACKET_REFRESH_P7;
	uint8 dummy1514[28];
	uint32 SWITCH_CORE_QUEUE6_MAX_PACKET_REFRESH_IMP;
	uint8 dummy1515[60];
	uint32 SWITCH_CORE_EGRESS_SHAPER_Q6_CONFIG_REG_SPARE0;
	uint8 dummy1516[28];
	uint32 SWITCH_CORE_EGRESS_SHAPER_Q6_CONFIG_REG_SPARE1;
	uint8 dummy1517[28];
	uint32 SWITCH_CORE_QUEUE6_MAX_PACKET_THD_SEL_P0;
	uint8 dummy1518[28];
	uint32 SWITCH_CORE_QUEUE6_MAX_PACKET_THD_SEL_P1;
	uint8 dummy1519[28];
	uint32 SWITCH_CORE_QUEUE6_MAX_PACKET_THD_SEL_P2;
	uint8 dummy1520[28];
	uint32 SWITCH_CORE_QUEUE6_MAX_PACKET_THD_SEL_P3;
	uint8 dummy1521[28];
	uint32 SWITCH_CORE_QUEUE6_MAX_PACKET_THD_SEL_P4;
	uint8 dummy1522[28];
	uint32 SWITCH_CORE_QUEUE6_MAX_PACKET_THD_SEL_P5;
	uint8 dummy1523[28];
	uint32 SWITCH_CORE_QUEUE6_MAX_PACKET_THD_SEL_P6;
	uint8 dummy1524[28];
	uint32 SWITCH_CORE_QUEUE6_MAX_PACKET_THD_SEL_P7;
	uint8 dummy1525[28];
	uint32 SWITCH_CORE_QUEUE6_MAX_PACKET_THD_SEL_IMP;
	uint8 dummy1526[28];
	uint32 SWITCH_CORE_QUEUE6_AVB_SHAPING_MODE;
	uint8 dummy1527[12];
	uint32 SWITCH_CORE_QUEUE6_SHAPER_ENABLE;
	uint8 dummy1528[12];
	uint32 SWITCH_CORE_QUEUE6_SHAPER_BUCKET_COUNT_SELECT;
	uint8 dummy1529[12];
	uint32 SWITCH_CORE_QUEUE6_SHAPER_BLOCKING;
	uint8 dummy1530[172];
	uint32 SWITCH_CORE_QUEUE7_MAX_REFRESH_P0;
	uint8 dummy1531[28];
	uint32 SWITCH_CORE_QUEUE7_MAX_REFRESH_P1;
	uint8 dummy1532[28];
	uint32 SWITCH_CORE_QUEUE7_MAX_REFRESH_P2;
	uint8 dummy1533[28];
	uint32 SWITCH_CORE_QUEUE7_MAX_REFRESH_P3;
	uint8 dummy1534[28];
	uint32 SWITCH_CORE_QUEUE7_MAX_REFRESH_P4;
	uint8 dummy1535[28];
	uint32 SWITCH_CORE_QUEUE7_MAX_REFRESH_P5;
	uint8 dummy1536[28];
	uint32 SWITCH_CORE_QUEUE7_MAX_REFRESH_P6;
	uint8 dummy1537[28];
	uint32 SWITCH_CORE_QUEUE7_MAX_REFRESH_P7;
	uint8 dummy1538[28];
	uint32 SWITCH_CORE_QUEUE7_MAX_REFRESH_IMP;
	uint8 dummy1539[124];
	uint32 SWITCH_CORE_QUEUE7_MAX_THD_SEL_P0;
	uint8 dummy1540[28];
	uint32 SWITCH_CORE_QUEUE7_MAX_THD_SEL_P1;
	uint8 dummy1541[28];
	uint32 SWITCH_CORE_QUEUE7_MAX_THD_SEL_P2;
	uint8 dummy1542[28];
	uint32 SWITCH_CORE_QUEUE7_MAX_THD_SEL_P3;
	uint8 dummy1543[28];
	uint32 SWITCH_CORE_QUEUE7_MAX_THD_SEL_P4;
	uint8 dummy1544[28];
	uint32 SWITCH_CORE_QUEUE7_MAX_THD_SEL_P5;
	uint8 dummy1545[28];
	uint32 SWITCH_CORE_QUEUE7_MAX_THD_SEL_P6;
	uint8 dummy1546[28];
	uint32 SWITCH_CORE_QUEUE7_MAX_THD_SEL_P7;
	uint8 dummy1547[28];
	uint32 SWITCH_CORE_QUEUE7_MAX_THD_SEL_IMP;
	uint8 dummy1548[124];
	uint32 SWITCH_CORE_QUEUE7_SHAPER_STS_P0;
	uint8 dummy1549[28];
	uint32 SWITCH_CORE_QUEUE7_SHAPER_STS_P1;
	uint8 dummy1550[28];
	uint32 SWITCH_CORE_QUEUE7_SHAPER_STS_P2;
	uint8 dummy1551[28];
	uint32 SWITCH_CORE_QUEUE7_SHAPER_STS_P3;
	uint8 dummy1552[28];
	uint32 SWITCH_CORE_QUEUE7_SHAPER_STS_P4;
	uint8 dummy1553[28];
	uint32 SWITCH_CORE_QUEUE7_SHAPER_STS_P5;
	uint8 dummy1554[28];
	uint32 SWITCH_CORE_QUEUE7_SHAPER_STS_P6;
	uint8 dummy1555[28];
	uint32 SWITCH_CORE_QUEUE7_SHAPER_STS_P7;
	uint8 dummy1556[28];
	uint32 SWITCH_CORE_QUEUE7_SHAPER_STS_IMP;
	uint8 dummy1557[124];
	uint32 SWITCH_CORE_QUEUE7_MAX_PACKET_REFRESH_P0;
	uint8 dummy1558[28];
	uint32 SWITCH_CORE_QUEUE7_MAX_PACKET_REFRESH_P1;
	uint8 dummy1559[28];
	uint32 SWITCH_CORE_QUEUE7_MAX_PACKET_REFRESH_P2;
	uint8 dummy1560[28];
	uint32 SWITCH_CORE_QUEUE7_MAX_PACKET_REFRESH_P3;
	uint8 dummy1561[28];
	uint32 SWITCH_CORE_QUEUE7_MAX_PACKET_REFRESH_P4;
	uint8 dummy1562[28];
	uint32 SWITCH_CORE_QUEUE7_MAX_PACKET_REFRESH_P5;
	uint8 dummy1563[28];
	uint32 SWITCH_CORE_QUEUE7_MAX_PACKET_REFRESH_P6;
	uint8 dummy1564[28];
	uint32 SWITCH_CORE_QUEUE7_MAX_PACKET_REFRESH_P7;
	uint8 dummy1565[28];
	uint32 SWITCH_CORE_QUEUE7_MAX_PACKET_REFRESH_IMP;
	uint8 dummy1566[60];
	uint32 SWITCH_CORE_EGRESS_SHAPER_Q7_CONFIG_REG_SPARE0;
	uint8 dummy1567[28];
	uint32 SWITCH_CORE_EGRESS_SHAPER_Q7_CONFIG_REG_SPARE1;
	uint8 dummy1568[28];
	uint32 SWITCH_CORE_QUEUE7_MAX_PACKET_THD_SEL_P0;
	uint8 dummy1569[28];
	uint32 SWITCH_CORE_QUEUE7_MAX_PACKET_THD_SEL_P1;
	uint8 dummy1570[28];
	uint32 SWITCH_CORE_QUEUE7_MAX_PACKET_THD_SEL_P2;
	uint8 dummy1571[28];
	uint32 SWITCH_CORE_QUEUE7_MAX_PACKET_THD_SEL_P3;
	uint8 dummy1572[28];
	uint32 SWITCH_CORE_QUEUE7_MAX_PACKET_THD_SEL_P4;
	uint8 dummy1573[28];
	uint32 SWITCH_CORE_QUEUE7_MAX_PACKET_THD_SEL_P5;
	uint8 dummy1574[28];
	uint32 SWITCH_CORE_QUEUE7_MAX_PACKET_THD_SEL_P6;
	uint8 dummy1575[28];
	uint32 SWITCH_CORE_QUEUE7_MAX_PACKET_THD_SEL_P7;
	uint8 dummy1576[28];
	uint32 SWITCH_CORE_QUEUE7_MAX_PACKET_THD_SEL_IMP;
	uint8 dummy1577[28];
	uint32 SWITCH_CORE_QUEUE7_AVB_SHAPING_MODE;
	uint8 dummy1578[12];
	uint32 SWITCH_CORE_QUEUE7_SHAPER_ENABLE;
	uint8 dummy1579[12];
	uint32 SWITCH_CORE_QUEUE7_SHAPER_BUCKET_COUNT_SELECT;
	uint8 dummy1580[12];
	uint32 SWITCH_CORE_QUEUE7_SHAPER_BLOCKING;
	uint8 dummy1581[65708];
	uint32 SWITCH_CORE_MIB_SNAPSHOT_CTL;
	uint8 dummy1582[2044];
	uint32 SWITCH_CORE_S_TxOctets;
	uint8 dummy1583[60];
	uint32 SWITCH_CORE_S_TxDropPkts;
	uint8 dummy1584[28];
	uint32 SWITCH_CORE_S_TxQPKTQ0;
	uint8 dummy1585[28];
	uint32 SWITCH_CORE_S_TxBroadcastPkts;
	uint8 dummy1586[28];
	uint32 SWITCH_CORE_S_TxMulticastPkts;
	uint8 dummy1587[28];
	uint32 SWITCH_CORE_S_TxUnicastPkts;
	uint8 dummy1588[28];
	uint32 SWITCH_CORE_S_TxCollisions;
	uint8 dummy1589[28];
	uint32 SWITCH_CORE_S_TxSingleCollision;
	uint8 dummy1590[28];
	uint32 SWITCH_CORE_S_TxMultipleCollision;
	uint8 dummy1591[28];
	uint32 SWITCH_CORE_S_TxDeferredTransmit;
	uint8 dummy1592[28];
	uint32 SWITCH_CORE_S_TxLateCollision;
	uint8 dummy1593[28];
	uint32 SWITCH_CORE_S_TxExcessiveCollision;
	uint8 dummy1594[28];
	uint32 SWITCH_CORE_S_TxFrameInDisc;
	uint8 dummy1595[28];
	uint32 SWITCH_CORE_S_TxPausePkts;
	uint8 dummy1596[28];
	uint32 SWITCH_CORE_S_TxQPKTQ1;
	uint8 dummy1597[28];
	uint32 SWITCH_CORE_S_TxQPKTQ2;
	uint8 dummy1598[28];
	uint32 SWITCH_CORE_S_TxQPKTQ3;
	uint8 dummy1599[28];
	uint32 SWITCH_CORE_S_TxQPKTQ4;
	uint8 dummy1600[28];
	uint32 SWITCH_CORE_S_TxQPKTQ5;
	uint8 dummy1601[28];
	uint32 SWITCH_CORE_S_RxOctets;
	uint8 dummy1602[60];
	uint32 SWITCH_CORE_S_RxUndersizePkts;
	uint8 dummy1603[28];
	uint32 SWITCH_CORE_S_RxPausePkts;
	uint8 dummy1604[28];
	uint32 SWITCH_CORE_S_RxPkts64Octets;
	uint8 dummy1605[28];
	uint32 SWITCH_CORE_S_RxPkts65to127Octets;
	uint8 dummy1606[28];
	uint32 SWITCH_CORE_S_RxPkts128to255Octets;
	uint8 dummy1607[28];
	uint32 SWITCH_CORE_S_RxPkts256to511Octets;
	uint8 dummy1608[28];
	uint32 SWITCH_CORE_S_RxPkts512to1023Octets;
	uint8 dummy1609[28];
	uint32 SWITCH_CORE_S_RxPkts1024toMaxPktOctets;
	uint8 dummy1610[28];
	uint32 SWITCH_CORE_S_RxOversizePkts;
	uint8 dummy1611[28];
	uint32 SWITCH_CORE_S_RxJabbers;
	uint8 dummy1612[28];
	uint32 SWITCH_CORE_S_RxAlignmentErrors;
	uint8 dummy1613[28];
	uint32 SWITCH_CORE_S_RxFCSErrors;
	uint8 dummy1614[28];
	uint32 SWITCH_CORE_S_RxGoodOctets;
	uint8 dummy1615[60];
	uint32 SWITCH_CORE_S_RxDropPkts;
	uint8 dummy1616[28];
	uint32 SWITCH_CORE_S_RxUnicastPkts;
	uint8 dummy1617[28];
	uint32 SWITCH_CORE_S_RxMulticastPkts;
	uint8 dummy1618[28];
	uint32 SWITCH_CORE_S_RxBroadcastPkts;
	uint8 dummy1619[28];
	uint32 SWITCH_CORE_S_RxSAChanges;
	uint8 dummy1620[28];
	uint32 SWITCH_CORE_S_RxFragments;
	uint8 dummy1621[28];
	uint32 SWITCH_CORE_S_RxJumboPkt;
	uint8 dummy1622[28];
	uint32 SWITCH_CORE_S_RxSymblErr;
	uint8 dummy1623[28];
	uint32 SWITCH_CORE_S_InRangeErrCount;
	uint8 dummy1624[28];
	uint32 SWITCH_CORE_S_OutRangeErrCount;
	uint8 dummy1625[28];
	uint32 SWITCH_CORE_S_EEE_LPI_EVENT;
	uint8 dummy1626[28];
	uint32 SWITCH_CORE_S_EEE_LPI_DURATION;
	uint8 dummy1627[28];
	uint32 SWITCH_CORE_S_RxDiscard;
	uint8 dummy1628[60];
	uint32 SWITCH_CORE_S_TxQPKTQ6;
	uint8 dummy1629[28];
	uint32 SWITCH_CORE_S_TxQPKTQ7;
	uint8 dummy1630[28];
	uint32 SWITCH_CORE_S_TxPkts64Octets;
	uint8 dummy1631[28];
	uint32 SWITCH_CORE_S_TxPkts65to127Octets;
	uint8 dummy1632[28];
	uint32 SWITCH_CORE_S_TxPkts128to255Octets;
	uint8 dummy1633[28];
	uint32 SWITCH_CORE_S_TxPkts256to511Octets;
	uint8 dummy1634[28];
	uint32 SWITCH_CORE_S_TxPkts512to1023Octets;
	uint8 dummy1635[28];
	uint32 SWITCH_CORE_S_TxPkts1024toMaxPktOctets;
	uint8 dummy1636[220];
	uint32 SWITCH_CORE_LPDET_CFG;
	uint8 dummy1637[12];
	uint64 SWITCH_CORE_DF_TIMER;
	uint32 SWITCH_CORE_LED_PORTMAP;
	uint8 dummy1638[12];
	uint32 SWITCH_CORE_MODULE_ID0;
	uint8 dummy1639[44];
	uint32 SWITCH_CORE_MODULE_ID1;
	uint8 dummy1640[44];
	uint32 SWITCH_CORE_LPDET_SA;
	uint8 dummy1641[116];
	uint32 SWITCH_CORE_LPDET_REG_SPARE0;
	uint8 dummy1642[28];
	uint32 SWITCH_CORE_LPDET_REG_SPARE1;
	uint8 dummy1643[1756];
	uint64 SWITCH_CORE_BPM_CTRL;
	uint64 SWITCH_CORE_BPM_PSM_OVR_CTRL;
	uint32 SWITCH_CORE_BPM_PSM_TIME_CFG;
	uint8 dummy1644[12];
	uint32 SWITCH_CORE_BPM_PSM_THD_CFG;
	uint8 dummy1645[28];
	uint32 SWITCH_CORE_ROW_VMASK_OVR_CTRL;
	uint8 dummy1646[28];
	uint32 SWITCH_CORE_BPM_STS;
	uint8 dummy1647[28];
	uint32 SWITCH_CORE_BPM_PDA_OVR_CTRL;
	uint8 dummy1648[12];
	uint32 SWITCH_CORE_PDA_TIMEOUT_CFG;
	uint8 dummy1649[12];
	uint32 SWITCH_CORE_PDA_SETUP_TIME_CFG;
	uint8 dummy1650[12];
	uint32 SWITCH_CORE_PDA_HOLD_TIME_CFG;
	uint8 dummy1651[12];
	uint32 SWITCH_CORE_PBB_VBUFCNT_P0;
	uint8 dummy1652[12];
	uint32 SWITCH_CORE_PBB_VBUFCNT_P1;
	uint8 dummy1653[12];
	uint32 SWITCH_CORE_PBB_VBUFCNT_P2;
	uint8 dummy1654[12];
	uint32 SWITCH_CORE_RCY_TIME_CFG;
	uint8 dummy1655[12];
	uint32 SWITCH_CORE_PBB_PWRDWN_MON_CTRL;
	uint8 dummy1656[60];
	uint32 SWITCH_CORE_PBB_PWRDWN_MON0;
	uint8 dummy1657[60];
	uint32 SWITCH_CORE_PBB_PWRDWN_MON1;
	uint8 dummy1658[60];
	uint32 SWITCH_CORE_PBB_PWRDWN_MON2;
	uint8 dummy1659[316];
	uint32 SWITCH_CORE_BPM_REG_SPARE0;
	uint8 dummy1660[28];
	uint32 SWITCH_CORE_BPM_REG_SPARE1;
	uint8 dummy1661[60636];
	uint32 SWITCH_CORE_TRREG_CTRL0;
	uint8 dummy1662[28];
	uint32 SWITCH_CORE_TRREG_CTRL1;
	uint8 dummy1663[28];
	uint32 SWITCH_CORE_TRREG_CTRL2;
	uint8 dummy1664[60];
	uint32 SWITCH_CORE_EGRESS_PKT_TC2PCP_MAP_P0;
	uint8 dummy1665[60];
	uint32 SWITCH_CORE_EGRESS_PKT_TC2PCP_MAP_P1;
	uint8 dummy1666[60];
	uint32 SWITCH_CORE_EGRESS_PKT_TC2PCP_MAP_P2;
	uint8 dummy1667[60];
	uint32 SWITCH_CORE_EGRESS_PKT_TC2PCP_MAP_P3;
	uint8 dummy1668[60];
	uint32 SWITCH_CORE_EGRESS_PKT_TC2PCP_MAP_P4;
	uint8 dummy1669[60];
	uint32 SWITCH_CORE_EGRESS_PKT_TC2PCP_MAP_P5;
	uint8 dummy1670[60];
	uint32 SWITCH_CORE_EGRESS_PKT_TC2PCP_MAP_P6;
	uint8 dummy1671[60];
	uint32 SWITCH_CORE_EGRESS_PKT_TC2PCP_MAP_P7;
	uint8 dummy1672[60];
	uint32 SWITCH_CORE_EGRESS_PKT_TC2PCP_MAP_IMP;
	uint8 dummy1673[124];
	uint32 SWITCH_CORE_EGRESS_PKT_TC2CPCP_MAP_P0;
	uint8 dummy1674[60];
	uint32 SWITCH_CORE_EGRESS_PKT_TC2CPCP_MAP_P1;
	uint8 dummy1675[60];
	uint32 SWITCH_CORE_EGRESS_PKT_TC2CPCP_MAP_P2;
	uint8 dummy1676[60];
	uint32 SWITCH_CORE_EGRESS_PKT_TC2CPCP_MAP_P3;
	uint8 dummy1677[60];
	uint32 SWITCH_CORE_EGRESS_PKT_TC2CPCP_MAP_P4;
	uint8 dummy1678[60];
	uint32 SWITCH_CORE_EGRESS_PKT_TC2CPCP_MAP_P5;
	uint8 dummy1679[60];
	uint32 SWITCH_CORE_EGRESS_PKT_TC2CPCP_MAP_P6;
	uint8 dummy1680[60];
	uint32 SWITCH_CORE_EGRESS_PKT_TC2CPCP_MAP_P7;
	uint8 dummy1681[60];
	uint32 SWITCH_CORE_EGRESS_PKT_TC2CPCP_MAP_IMP;
	uint8 dummy1682[124];
	uint32 SWITCH_CORE_TRREG_REG_SPARE0;
	uint8 dummy1683[28];
	uint32 SWITCH_CORE_TRREG_REG_SPARE1;
	uint8 dummy1684[604];
	uint32 SWITCH_CORE_EEE_EN_CTRL;
	uint8 dummy1685[12];
	uint32 SWITCH_CORE_EEE_LPI_ASSERT;
	uint8 dummy1686[12];
	uint32 SWITCH_CORE_EEE_LPI_INDICATE;
	uint8 dummy1687[12];
	uint32 SWITCH_CORE_EEE_RX_IDLE_SYMBOL;
	uint8 dummy1688[12];
	uint32 SWITCH_CORE_EEE_LPI_SYMBOL_TX_DISABLE;
	uint8 dummy1689[28];
	uint32 SWITCH_CORE_EEE_PIPELINE_TIMER;
	uint8 dummy1690[28];
	uint32 SWITCH_CORE_EEE_SLEEP_TIMER_G_P0;
	uint8 dummy1691[28];
	uint32 SWITCH_CORE_EEE_SLEEP_TIMER_G_P1;
	uint8 dummy1692[28];
	uint32 SWITCH_CORE_EEE_SLEEP_TIMER_G_P2;
	uint8 dummy1693[28];
	uint32 SWITCH_CORE_EEE_SLEEP_TIMER_G_P3;
	uint8 dummy1694[28];
	uint32 SWITCH_CORE_EEE_SLEEP_TIMER_G_P4;
	uint8 dummy1695[28];
	uint32 SWITCH_CORE_EEE_SLEEP_TIMER_G_P5;
	uint8 dummy1696[28];
	uint32 SWITCH_CORE_EEE_SLEEP_TIMER_G_P6;
	uint8 dummy1697[28];
	uint32 SWITCH_CORE_EEE_SLEEP_TIMER_G_P7;
	uint8 dummy1698[28];
	uint32 SWITCH_CORE_EEE_SLEEP_TIMER_G_IMP;
	uint8 dummy1699[28];
	uint32 SWITCH_CORE_EEE_SLEEP_TIMER_H_P0;
	uint8 dummy1700[28];
	uint32 SWITCH_CORE_EEE_SLEEP_TIMER_H_P1;
	uint8 dummy1701[28];
	uint32 SWITCH_CORE_EEE_SLEEP_TIMER_H_P2;
	uint8 dummy1702[28];
	uint32 SWITCH_CORE_EEE_SLEEP_TIMER_H_P3;
	uint8 dummy1703[28];
	uint32 SWITCH_CORE_EEE_SLEEP_TIMER_H_P4;
	uint8 dummy1704[28];
	uint32 SWITCH_CORE_EEE_SLEEP_TIMER_H_P5;
	uint8 dummy1705[28];
	uint32 SWITCH_CORE_EEE_SLEEP_TIMER_H_P6;
	uint8 dummy1706[28];
	uint32 SWITCH_CORE_EEE_SLEEP_TIMER_H_P7;
	uint8 dummy1707[28];
	uint32 SWITCH_CORE_EEE_SLEEP_TIMER_H_IMP;
	uint8 dummy1708[28];
	uint32 SWITCH_CORE_EEE_MIN_LP_TIMER_G_P0;
	uint8 dummy1709[28];
	uint32 SWITCH_CORE_EEE_MIN_LP_TIMER_G_P1;
	uint8 dummy1710[28];
	uint32 SWITCH_CORE_EEE_MIN_LP_TIMER_G_P2;
	uint8 dummy1711[28];
	uint32 SWITCH_CORE_EEE_MIN_LP_TIMER_G_P3;
	uint8 dummy1712[28];
	uint32 SWITCH_CORE_EEE_MIN_LP_TIMER_G_P4;
	uint8 dummy1713[28];
	uint32 SWITCH_CORE_EEE_MIN_LP_TIMER_G_P5;
	uint8 dummy1714[28];
	uint32 SWITCH_CORE_EEE_MIN_LP_TIMER_G_P6;
	uint8 dummy1715[28];
	uint32 SWITCH_CORE_EEE_MIN_LP_TIMER_G_P7;
	uint8 dummy1716[28];
	uint32 SWITCH_CORE_EEE_MIN_LP_TIMER_G_IMP;
	uint8 dummy1717[28];
	uint32 SWITCH_CORE_EEE_MIN_LP_TIMER_H_P0;
	uint8 dummy1718[28];
	uint32 SWITCH_CORE_EEE_MIN_LP_TIMER_H_P1;
	uint8 dummy1719[28];
	uint32 SWITCH_CORE_EEE_MIN_LP_TIMER_H_P2;
	uint8 dummy1720[28];
	uint32 SWITCH_CORE_EEE_MIN_LP_TIMER_H_P3;
	uint8 dummy1721[28];
	uint32 SWITCH_CORE_EEE_MIN_LP_TIMER_H_P4;
	uint8 dummy1722[28];
	uint32 SWITCH_CORE_EEE_MIN_LP_TIMER_H_P5;
	uint8 dummy1723[28];
	uint32 SWITCH_CORE_EEE_MIN_LP_TIMER_H_P6;
	uint8 dummy1724[28];
	uint32 SWITCH_CORE_EEE_MIN_LP_TIMER_H_P7;
	uint8 dummy1725[28];
	uint32 SWITCH_CORE_EEE_MIN_LP_TIMER_H_IMP;
	uint8 dummy1726[28];
	uint32 SWITCH_CORE_EEE_WAKE_TIMER_G_P0;
	uint8 dummy1727[12];
	uint32 SWITCH_CORE_EEE_WAKE_TIMER_G_P1;
	uint8 dummy1728[12];
	uint32 SWITCH_CORE_EEE_WAKE_TIMER_G_P2;
	uint8 dummy1729[12];
	uint32 SWITCH_CORE_EEE_WAKE_TIMER_G_P3;
	uint8 dummy1730[12];
	uint32 SWITCH_CORE_EEE_WAKE_TIMER_G_P4;
	uint8 dummy1731[12];
	uint32 SWITCH_CORE_EEE_WAKE_TIMER_G_P5;
	uint8 dummy1732[12];
	uint32 SWITCH_CORE_EEE_WAKE_TIMER_G_P6;
	uint8 dummy1733[12];
	uint32 SWITCH_CORE_EEE_WAKE_TIMER_G_P7;
	uint8 dummy1734[12];
	uint32 SWITCH_CORE_EEE_WAKE_TIMER_G_P8;
	uint8 dummy1735[12];
	uint32 SWITCH_CORE_EEE_WAKE_TIMER_H_P0;
	uint8 dummy1736[12];
	uint32 SWITCH_CORE_EEE_WAKE_TIMER_H_P1;
	uint8 dummy1737[12];
	uint32 SWITCH_CORE_EEE_WAKE_TIMER_H_P2;
	uint8 dummy1738[12];
	uint32 SWITCH_CORE_EEE_WAKE_TIMER_H_P3;
	uint8 dummy1739[12];
	uint32 SWITCH_CORE_EEE_WAKE_TIMER_H_P4;
	uint8 dummy1740[12];
	uint32 SWITCH_CORE_EEE_WAKE_TIMER_H_P5;
	uint8 dummy1741[12];
	uint32 SWITCH_CORE_EEE_WAKE_TIMER_H_P6;
	uint8 dummy1742[12];
	uint32 SWITCH_CORE_EEE_WAKE_TIMER_H_P7;
	uint8 dummy1743[12];
	uint32 SWITCH_CORE_EEE_WAKE_TIMER_H_IMP;
	uint8 dummy1744[12];
	uint32 SWITCH_CORE_EEE_GLB_CONG_TH;
	uint8 dummy1745[12];
	uint32 SWITCH_CORE_EEE_TX_CONG_TH_Q0;
	uint8 dummy1746[12];
	uint32 SWITCH_CORE_EEE_TX_CONG_TH_Q1;
	uint8 dummy1747[12];
	uint32 SWITCH_CORE_EEE_TX_CONG_TH_Q2;
	uint8 dummy1748[12];
	uint32 SWITCH_CORE_EEE_TX_CONG_TH_Q3;
	uint8 dummy1749[12];
	uint32 SWITCH_CORE_EEE_TX_CONG_TH_Q4;
	uint8 dummy1750[12];
	uint32 SWITCH_CORE_EEE_TX_CONG_TH_Q5;
	uint8 dummy1751[20];
	uint32 SWITCH_CORE_EEE_TX_CONG_TH_Q6;
	uint8 dummy1752[12];
	uint32 SWITCH_CORE_EEE_TX_CONG_TH_Q7;
	uint8 dummy1753[44];
	uint32 SWITCH_CORE_EEE_CTL_REG_SPARE0;
	uint8 dummy1754[36];
	uint32 SWITCH_CORE_EEE_CTL_REG_SPARE1;
	uint8 dummy1755[52];
	uint64 SWITCH_CORE_EEE_DEBUG;
	uint32 SWITCH_CORE_EEE_LINK_DLY_TIMER;
	uint8 dummy1756[28];
	uint32 SWITCH_CORE_EEE_STATE;
	uint8 dummy1757[156];
	uint32 SWITCH_CORE_PORT_ENABLE;
	uint8 dummy1758[12];
	uint32 SWITCH_CORE_TX_MODE_PORT_P0;
	uint8 dummy1759[12];
	uint32 SWITCH_CORE_TX_MODE_PORT_P1;
	uint8 dummy1760[12];
	uint32 SWITCH_CORE_TX_MODE_PORT_P2;
	uint8 dummy1761[12];
	uint32 SWITCH_CORE_TX_MODE_PORT_P3;
	uint8 dummy1762[12];
	uint32 SWITCH_CORE_TX_MODE_PORT_P4;
	uint8 dummy1763[12];
	uint32 SWITCH_CORE_TX_MODE_PORT_P5;
	uint8 dummy1764[12];
	uint32 SWITCH_CORE_TX_MODE_PORT_P7;
	uint8 dummy1765[12];
	uint32 SWITCH_CORE_TX_MODE_PORT_IMP;
	uint8 dummy1766[12];
	uint32 SWITCH_CORE_RX_MODE_PORT_P0;
	uint8 dummy1767[12];
	uint32 SWITCH_CORE_RX_MODE_PORT_P1;
	uint8 dummy1768[12];
	uint32 SWITCH_CORE_RX_MODE_PORT_P2;
	uint8 dummy1769[12];
	uint32 SWITCH_CORE_RX_MODE_PORT_P3;
	uint8 dummy1770[12];
	uint32 SWITCH_CORE_RX_MODE_PORT_P4;
	uint8 dummy1771[12];
	uint32 SWITCH_CORE_RX_MODE_PORT_P5;
	uint8 dummy1772[12];
	uint32 SWITCH_CORE_RX_MODE_PORT_P7;
	uint8 dummy1773[12];
	uint32 SWITCH_CORE_RX_MODE_PORT_IMP;
	uint8 dummy1774[12];
	uint32 SWITCH_CORE_TX_TS_CAP;
	uint8 dummy1775[12];
	uint32 SWITCH_CORE_RX_TS_CAP;
	uint8 dummy1776[12];
	uint32 SWITCH_CORE_RX_TX_OPTION;
	uint8 dummy1777[12];
	uint32 SWITCH_CORE_RX_PORT_0_LINK_DELAY_LSB;
	uint8 dummy1778[12];
	uint32 SWITCH_CORE_RX_PORT_0_LINK_DELAY_MSB;
	uint8 dummy1779[12];
	uint32 SWITCH_CORE_RX_PORT_1_LINK_DELAY_LSB;
	uint8 dummy1780[12];
	uint32 SWITCH_CORE_RX_PORT_1_LINK_DELAY_MSB;
	uint8 dummy1781[12];
	uint32 SWITCH_CORE_RX_PORT_2_LINK_DELAY_LSB;
	uint8 dummy1782[12];
	uint32 SWITCH_CORE_RX_PORT_2_LINK_DELAY_MSB;
	uint8 dummy1783[12];
	uint32 SWITCH_CORE_RX_PORT_3_LINK_DELAY_LSB;
	uint8 dummy1784[12];
	uint32 SWITCH_CORE_RX_PORT_3_LINK_DELAY_MSB;
	uint8 dummy1785[12];
	uint32 SWITCH_CORE_RX_PORT_4_LINK_DELAY_LSB;
	uint8 dummy1786[12];
	uint32 SWITCH_CORE_RX_PORT_4_LINK_DELAY_MSB;
	uint8 dummy1787[12];
	uint32 SWITCH_CORE_RX_PORT_5_LINK_DELAY_LSB;
	uint8 dummy1788[12];
	uint32 SWITCH_CORE_RX_PORT_5_LINK_DELAY_MSB;
	uint8 dummy1789[12];
	uint32 SWITCH_CORE_RX_PORT_7_LINK_DELAY_LSB;
	uint8 dummy1790[12];
	uint32 SWITCH_CORE_RX_PORT_7_LINK_DELAY_MSB;
	uint8 dummy1791[12];
	uint32 SWITCH_CORE_RX_PORT_8_LINK_DELAY_LSB;
	uint8 dummy1792[12];
	uint32 SWITCH_CORE_RX_PORT_8_LINK_DELAY_MSB;
	uint8 dummy1793[12];
	uint32 SWITCH_CORE_RX_PORT_0_TS_OFFSET_LSB;
	uint8 dummy1794[12];
	uint32 SWITCH_CORE_RX_PORT_0_TS_OFFSET_MSB;
	uint8 dummy1795[12];
	uint32 SWITCH_CORE_RX_PORT_1_TS_OFFSET_LSB;
	uint8 dummy1796[12];
	uint32 SWITCH_CORE_RX_PORT_1_TS_OFFSET_MSB;
	uint8 dummy1797[12];
	uint32 SWITCH_CORE_RX_PORT_2_TS_OFFSET_LSB;
	uint8 dummy1798[12];
	uint32 SWITCH_CORE_RX_PORT_2_TS_OFFSET_MSB;
	uint8 dummy1799[12];
	uint32 SWITCH_CORE_RX_PORT_3_TS_OFFSET_LSB;
	uint8 dummy1800[12];
	uint32 SWITCH_CORE_RX_PORT_3_TS_OFFSET_MSB;
	uint8 dummy1801[12];
	uint32 SWITCH_CORE_RX_PORT_4_TS_OFFSET_LSB;
	uint8 dummy1802[12];
	uint32 SWITCH_CORE_RX_PORT_4_TS_OFFSET_MSB;
	uint8 dummy1803[12];
	uint32 SWITCH_CORE_RX_PORT_5_TS_OFFSET_LSB;
	uint8 dummy1804[12];
	uint32 SWITCH_CORE_RX_PORT_5_TS_OFFSET_MSB;
	uint8 dummy1805[12];
	uint32 SWITCH_CORE_RX_PORT_7_TS_OFFSET_LSB;
	uint8 dummy1806[12];
	uint32 SWITCH_CORE_RX_PORT_7_TS_OFFSET_MSB;
	uint8 dummy1807[12];
	uint32 SWITCH_CORE_RX_PORT_8_TS_OFFSET_LSB;
	uint8 dummy1808[12];
	uint32 SWITCH_CORE_RX_PORT_8_TS_OFFSET_MSB;
	uint8 dummy1809[12];
	uint32 SWITCH_CORE_TX_PORT_0_TS_OFFSET_LSB;
	uint8 dummy1810[12];
	uint32 SWITCH_CORE_TX_PORT_0_TS_OFFSET_MSB;
	uint8 dummy1811[12];
	uint32 SWITCH_CORE_TX_PORT_1_TS_OFFSET_LSB;
	uint8 dummy1812[12];
	uint32 SWITCH_CORE_TX_PORT_1_TS_OFFSET_MSB;
	uint8 dummy1813[12];
	uint32 SWITCH_CORE_TX_PORT_2_TS_OFFSET_LSB;
	uint8 dummy1814[12];
	uint32 SWITCH_CORE_TX_PORT_2_TS_OFFSET_MSB;
	uint8 dummy1815[12];
	uint32 SWITCH_CORE_TX_PORT_3_TS_OFFSET_LSB;
	uint8 dummy1816[12];
	uint32 SWITCH_CORE_TX_PORT_3_TS_OFFSET_MSB;
	uint8 dummy1817[12];
	uint32 SWITCH_CORE_TX_PORT_4_TS_OFFSET_LSB;
	uint8 dummy1818[12];
	uint32 SWITCH_CORE_TX_PORT_4_TS_OFFSET_MSB;
	uint8 dummy1819[12];
	uint32 SWITCH_CORE_TX_PORT_5_TS_OFFSET_LSB;
	uint8 dummy1820[12];
	uint32 SWITCH_CORE_TX_PORT_5_TS_OFFSET_MSB;
	uint8 dummy1821[12];
	uint32 SWITCH_CORE_TX_PORT_7_TS_OFFSET_LSB;
	uint8 dummy1822[12];
	uint32 SWITCH_CORE_TX_PORT_7_TS_OFFSET_MSB;
	uint8 dummy1823[12];
	uint32 SWITCH_CORE_TX_PORT_8_TS_OFFSET_LSB;
	uint8 dummy1824[12];
	uint32 SWITCH_CORE_TX_PORT_8_TS_OFFSET_MSB;
	uint8 dummy1825[12];
	uint32 SWITCH_CORE_TIME_CODE_N_P0;
	uint8 dummy1826[12];
	uint32 SWITCH_CORE_TIME_CODE_N_P1;
	uint8 dummy1827[12];
	uint32 SWITCH_CORE_TIME_CODE_N_P2;
	uint8 dummy1828[12];
	uint32 SWITCH_CORE_TIME_CODE_N_P3;
	uint8 dummy1829[12];
	uint32 SWITCH_CORE_TIME_CODE_N_P4;
	uint8 dummy1830[12];
	uint32 SWITCH_CORE_DPLL_DB_LSB;
	uint8 dummy1831[12];
	uint32 SWITCH_CORE_DPLL_DB_MSB;
	uint8 dummy1832[12];
	uint32 SWITCH_CORE_DPLL_DB_SEL;
	uint8 dummy1833[12];
	uint32 SWITCH_CORE_SHD_CTL;
	uint8 dummy1834[12];
	uint32 SWITCH_CORE_SHD_LD;
	uint8 dummy1835[12];
	uint32 SWITCH_CORE_INT_MASK;
	uint8 dummy1836[12];
	uint32 SWITCH_CORE_INT_STAT;
	uint8 dummy1837[12];
	uint32 SWITCH_CORE_TX_CTL;
	uint8 dummy1838[12];
	uint32 SWITCH_CORE_RX_CTL;
	uint8 dummy1839[12];
	uint32 SWITCH_CORE_RX_TX_CTL;
	uint8 dummy1840[12];
	uint32 SWITCH_CORE_VLAN_ITPID;
	uint8 dummy1841[12];
	uint32 SWITCH_CORE_VLAN_OTPID;
	uint8 dummy1842[12];
	uint32 SWITCH_CORE_OTHER_OTPID;
	uint8 dummy1843[12];
	uint32 SWITCH_CORE_NSE_DPLL_1;
	uint8 dummy1844[12];
	uint32 SWITCH_CORE_NSE_DPLL_2_N_P0;
	uint8 dummy1845[12];
	uint32 SWITCH_CORE_NSE_DPLL_2_N_P1;
	uint8 dummy1846[12];
	uint32 SWITCH_CORE_NSE_DPLL_2_N_P2;
	uint8 dummy1847[12];
	uint32 SWITCH_CORE_NSE_DPLL_3_N_P0;
	uint8 dummy1848[12];
	uint32 SWITCH_CORE_NSE_DPLL_3_N_P1;
	uint8 dummy1849[12];
	uint32 SWITCH_CORE_NSE_DPLL_4;
	uint8 dummy1850[12];
	uint32 SWITCH_CORE_NSE_DPLL_5;
	uint8 dummy1851[12];
	uint32 SWITCH_CORE_NSE_DPLL_6;
	uint8 dummy1852[12];
	uint32 SWITCH_CORE_NSE_DPLL_7_N_P0;
	uint8 dummy1853[12];
	uint32 SWITCH_CORE_NSE_DPLL_7_N_P1;
	uint8 dummy1854[12];
	uint32 SWITCH_CORE_NSE_DPLL_7_N_P2;
	uint8 dummy1855[12];
	uint32 SWITCH_CORE_NSE_DPLL_7_N_P3;
	uint8 dummy1856[12];
	uint32 SWITCH_CORE_NSE_NCO_1_N_P0;
	uint8 dummy1857[12];
	uint32 SWITCH_CORE_NSE_NCO_1_N_P1;
	uint8 dummy1858[12];
	uint32 SWITCH_CORE_NSE_NCO_2_N_P0;
	uint8 dummy1859[12];
	uint32 SWITCH_CORE_NSE_NCO_2_N_P1;
	uint8 dummy1860[12];
	uint32 SWITCH_CORE_NSE_NCO_2_N_P2;
	uint8 dummy1861[12];
	uint32 SWITCH_CORE_NSE_NCO_3_0;
	uint8 dummy1862[12];
	uint32 SWITCH_CORE_NSE_NCO_3_1;
	uint8 dummy1863[12];
	uint32 SWITCH_CORE_NSE_NCO_3_2;
	uint8 dummy1864[12];
	uint32 SWITCH_CORE_NSE_NCO_4;
	uint8 dummy1865[12];
	uint32 SWITCH_CORE_NSE_NCO_5_0;
	uint8 dummy1866[12];
	uint32 SWITCH_CORE_NSE_NCO_5_1;
	uint8 dummy1867[12];
	uint32 SWITCH_CORE_NSE_NCO_5_2;
	uint8 dummy1868[12];
	uint32 SWITCH_CORE_NSE_NCO_6;
	uint8 dummy1869[12];
	uint32 SWITCH_CORE_NSE_NCO_7_0;
	uint8 dummy1870[12];
	uint32 SWITCH_CORE_NSE_NCO_7_1;
	uint8 dummy1871[12];
	uint32 SWITCH_CORE_TX_COUNTER;
	uint8 dummy1872[12];
	uint32 SWITCH_CORE_RX_COUNTER;
	uint8 dummy1873[12];
	uint32 SWITCH_CORE_RX_TX_1588_COUNTER;
	uint8 dummy1874[12];
	uint32 SWITCH_CORE_TS_READ_START_END;
	uint8 dummy1875[12];
	uint32 SWITCH_CORE_HEARTBEAT_0;
	uint8 dummy1876[12];
	uint32 SWITCH_CORE_HEARTBEAT_1;
	uint8 dummy1877[140];
	uint32 SWITCH_CORE_HEARTBEAT_2;
	uint8 dummy1878[12];
	uint32 SWITCH_CORE_TIME_STAMP_N_P0;
	uint8 dummy1879[12];
	uint32 SWITCH_CORE_TIME_STAMP_N_P1;
	uint8 dummy1880[12];
	uint32 SWITCH_CORE_TIME_STAMP_N_P2;
	uint8 dummy1881[12];
	uint32 SWITCH_CORE_TIME_STAMP_INFO_N_P0;
	uint8 dummy1882[12];
	uint32 SWITCH_CORE_TIME_STAMP_INFO_N_P1;
	uint8 dummy1883[12];
	uint32 SWITCH_CORE_CNTR_DBG;
	uint8 dummy1884[12];
	uint32 SWITCH_CORE_MPLS_SPARE1;
	uint8 dummy1885[12];
	uint32 SWITCH_CORE_MPLS_SPARE2;
	uint8 dummy1886[12];
	uint32 SWITCH_CORE_MPLS_SPARE3;
	uint8 dummy1887[12];
	uint32 SWITCH_CORE_MPLS_SPARE4;
	uint8 dummy1888[12];
	uint32 SWITCH_CORE_MPLS_SPARE5;
	uint8 dummy1889[12];
	uint32 SWITCH_CORE_MPLS_SPARE6;
	uint8 dummy1890[12];
	uint32 SWITCH_CORE_MPLS_TX_CNTL;
	uint8 dummy1891[12];
	uint32 SWITCH_CORE_MPLS_RX_CNTL;
	uint8 dummy1892[12];
	uint32 SWITCH_CORE_MPLS_LABEL1_MASK_LSB;
	uint8 dummy1893[12];
	uint32 SWITCH_CORE_MPLS_LABEL1_MASK_MSB;
	uint8 dummy1894[12];
	uint32 SWITCH_CORE_MPLS_LABEL1_VALUE_LSB;
	uint8 dummy1895[12];
	uint32 SWITCH_CORE_MPLS_LABEL1_VALUE_MSB;
	uint8 dummy1896[12];
	uint32 SWITCH_CORE_MPLS_LABEL2_MASK_LSB;
	uint8 dummy1897[12];
	uint32 SWITCH_CORE_MPLS_LABEL2_MASK_MSB;
	uint8 dummy1898[12];
	uint32 SWITCH_CORE_MPLS_LABEL2_VALUE_LSB;
	uint8 dummy1899[12];
	uint32 SWITCH_CORE_MPLS_LABEL2_VALUE_MSB;
	uint8 dummy1900[12];
	uint32 SWITCH_CORE_MPLS_LABEL3_MASK_LSB;
	uint8 dummy1901[12];
	uint32 SWITCH_CORE_MPLS_LABEL3_MASK_MSB;
	uint8 dummy1902[12];
	uint32 SWITCH_CORE_MPLS_LABEL3_VALUE_LSB;
	uint8 dummy1903[12];
	uint32 SWITCH_CORE_MPLS_LABEL3_VALUE_MSB;
	uint8 dummy1904[12];
	uint32 SWITCH_CORE_MPLS_LABEL4_MASK_LSB;
	uint8 dummy1905[12];
	uint32 SWITCH_CORE_MPLS_LABEL4_MASK_MSB;
	uint8 dummy1906[12];
	uint32 SWITCH_CORE_MPLS_LABEL4_VALUE_LSB;
	uint8 dummy1907[12];
	uint32 SWITCH_CORE_MPLS_LABEL4_VALUE_MSB;
	uint8 dummy1908[12];
	uint32 SWITCH_CORE_MPLS_LABEL5_MASK_LSB;
	uint8 dummy1909[12];
	uint32 SWITCH_CORE_MPLS_LABEL5_MASK_MSB;
	uint8 dummy1910[12];
	uint32 SWITCH_CORE_MPLS_LABEL5_VALUE_LSB;
	uint8 dummy1911[12];
	uint32 SWITCH_CORE_MPLS_LABEL5_VALUE_MSB;
	uint8 dummy1912[12];
	uint32 SWITCH_CORE_MPLS_LABEL6_MASK_LSB;
	uint8 dummy1913[12];
	uint32 SWITCH_CORE_MPLS_LABEL6_MASK_MSB;
	uint8 dummy1914[12];
	uint32 SWITCH_CORE_MPLS_LABEL6_VALUE_LSB;
	uint8 dummy1915[12];
	uint32 SWITCH_CORE_MPLS_LABEL6_VALUE_MSB;
	uint8 dummy1916[12];
	uint32 SWITCH_CORE_MPLS_LABEL7_MASK_LSB;
	uint8 dummy1917[12];
	uint32 SWITCH_CORE_MPLS_LABEL7_MASK_MSB;
	uint8 dummy1918[12];
	uint32 SWITCH_CORE_MPLS_LABEL7_VALUE_LSB;
	uint8 dummy1919[12];
	uint32 SWITCH_CORE_MPLS_LABEL7_VALUE_MSB;
	uint8 dummy1920[12];
	uint32 SWITCH_CORE_MPLS_LABEL8_MASK_LSB;
	uint8 dummy1921[12];
	uint32 SWITCH_CORE_MPLS_LABEL8_MASK_MSB;
	uint8 dummy1922[12];
	uint32 SWITCH_CORE_MPLS_LABEL8_VALUE_LSB;
	uint8 dummy1923[12];
	uint32 SWITCH_CORE_MPLS_LABEL8_VALUE_MSB;
	uint8 dummy1924[12];
	uint32 SWITCH_CORE_MPLS_LABEL9_MASK_LSB;
	uint8 dummy1925[12];
	uint32 SWITCH_CORE_MPLS_LABEL9_MASK_MSB;
	uint8 dummy1926[12];
	uint32 SWITCH_CORE_MPLS_LABEL9_VALUE_LSB;
	uint8 dummy1927[12];
	uint32 SWITCH_CORE_MPLS_LABEL9_VALUE_MSB;
	uint8 dummy1928[12];
	uint32 SWITCH_CORE_MPLS_LABEL10_MASK_LSB;
	uint8 dummy1929[12];
	uint32 SWITCH_CORE_MPLS_LABEL10_MASK_MSB;
	uint8 dummy1930[12];
	uint32 SWITCH_CORE_MPLS_LABEL10_VALUE_LSB;
	uint8 dummy1931[12];
	uint32 SWITCH_CORE_MPLS_LABEL10_VALUE_MSB;
	uint8 dummy1932[12];
	uint32 SWITCH_CORE_RX_TX_1588_COUNTER1;
	uint8 dummy1933[12];
	uint32 SWITCH_CORE_RX_CF_SPEC;
	uint8 dummy1934[12];
	uint32 SWITCH_CORE_TX_CF_SPEC;
	uint8 dummy1935[12];
	uint32 SWITCH_CORE_MPLS_PACKET_ENABLE;
	uint8 dummy1936[12];
	uint32 SWITCH_CORE_TIMECODE_SEL;
	uint8 dummy1937[12];
	uint32 SWITCH_CORE_TIME_STAMP_3;
	uint8 dummy1938[12];
	uint32 SWITCH_CORE_TIME_STAMP;
	uint8 dummy1939[12];
	uint32 SWITCH_CORE_DELAY_MEASUREMENT_TX_CONTROL;
	uint8 dummy1940[12];
	uint32 SWITCH_CORE_DELAY_MEASUREMENT_RX_CONTROL;
	uint8 dummy1941[12];
	uint32 SWITCH_CORE_DELAY_MEASUREMENT_ETYPE1;
	uint8 dummy1942[12];
	uint32 SWITCH_CORE_DELAY_MEASUREMENT_ETYPE2;
	uint8 dummy1943[12];
	uint32 SWITCH_CORE_DELAY_MEASUREMENT_ETYPE3;
	uint8 dummy1944[12];
	uint32 SWITCH_CORE_DELAY_MEASUREMENT_ETYPE4;
	uint8 dummy1945[12];
	uint32 SWITCH_CORE_DELAY_MEASUREMENT_ETYPE5;
	uint8 dummy1946[12];
	uint32 SWITCH_CORE_DELAY_MEASUREMENT_ETYPE6;
	uint8 dummy1947[12];
	uint32 SWITCH_CORE_DELAY_MEASUREMENT_ETYPE7;
	uint8 dummy1948[12];
	uint32 SWITCH_CORE_DELAY_MEASUREMENT_ETYPE8;
	uint8 dummy1949[12];
	uint32 SWITCH_CORE_DELAY_MEASUREMENT_ETYPE9;
	uint8 dummy1950[12];
	uint32 SWITCH_CORE_DELAY_MEASUREMENT_ETYPE10;
	uint8 dummy1951[12];
	uint32 SWITCH_CORE_DELAY_MEASUREMENT_ETYPE11;
	uint8 dummy1952[12];
	uint32 SWITCH_CORE_DELAY_MEASUREMENT_ETYPE12;
	uint8 dummy1953[12];
	uint32 SWITCH_CORE_DELAY_MEASUREMENT_ETYPE13;
	uint8 dummy1954[12];
	uint32 SWITCH_CORE_DELAY_MEASUREMENT_IETF_OFFSET;
	uint8 dummy1955[12];
	uint32 SWITCH_CORE_NTP_TIME_STAMP_N_P0;
	uint8 dummy1956[12];
	uint32 SWITCH_CORE_NTP_TIME_STAMP_N_P1;
	uint8 dummy1957[12];
	uint32 SWITCH_CORE_NTP_TIME_STAMP_N_P2;
	uint8 dummy1958[12];
	uint32 SWITCH_CORE_NTP_TIME_STAMP_N_P3;
	uint8 dummy1959[12];
	uint32 SWITCH_CORE_NTP_NCO_FREQ_0;
	uint8 dummy1960[12];
	uint32 SWITCH_CORE_NTP_NCO_FREQ_1;
	uint8 dummy1961[12];
	uint32 SWITCH_CORE_NTP_DOWN_CNTER_0;
	uint8 dummy1962[12];
	uint32 SWITCH_CORE_NTP_DOWN_CNTER_1;
	uint8 dummy1963[12];
	uint32 SWITCH_CORE_NTP_ERR_LSB;
	uint8 dummy1964[12];
	uint32 SWITCH_CORE_NTP_ERR_MSB;
	uint8 dummy1965[12];
	uint32 SWITCH_CORE_DM_MAC_L1_0;
	uint8 dummy1966[12];
	uint32 SWITCH_CORE_DM_MAC_L1_1;
	uint8 dummy1967[12];
	uint32 SWITCH_CORE_DM_MAC_L1_2;
	uint8 dummy1968[12];
	uint32 SWITCH_CORE_DM_MAC_L2_0;
	uint8 dummy1969[12];
	uint32 SWITCH_CORE_DM_MAC_L2_1;
	uint8 dummy1970[12];
	uint32 SWITCH_CORE_DM_MAC_L2_2;
	uint8 dummy1971[12];
	uint32 SWITCH_CORE_DM_MAC_L3_0;
	uint8 dummy1972[12];
	uint32 SWITCH_CORE_DM_MAC_L3_1;
	uint8 dummy1973[12];
	uint32 SWITCH_CORE_DM_MAC_L3_2;
	uint8 dummy1974[12];
	uint32 SWITCH_CORE_DM_MAC_CTL_0;
	uint8 dummy1975[12];
	uint32 SWITCH_CORE_DM_MAC_CTL_1;
	uint8 dummy1976[12];
	uint32 SWITCH_CORE_DM_MAC_CTL_2;
	uint8 dummy1977[12];
	uint32 SWITCH_CORE_HEARTBEAT_3;
	uint8 dummy1978[12];
	uint32 SWITCH_CORE_HEARTBEAT_4;
	uint8 dummy1979[12];
	uint32 SWITCH_CORE_INBAND_CNTL_N_P0;
	uint8 dummy1980[12];
	uint32 SWITCH_CORE_INBAND_CNTL_N_P1;
	uint8 dummy1981[12];
	uint32 SWITCH_CORE_INBAND_CNTL_N_P2;
	uint8 dummy1982[12];
	uint32 SWITCH_CORE_INBAND_CNTL_N_P3;
	uint8 dummy1983[12];
	uint32 SWITCH_CORE_INBAND_CNTL_N_P4;
	uint8 dummy1984[12];
	uint32 SWITCH_CORE_INBAND_CNTL_N_P5;
	uint8 dummy1985[12];
	uint32 SWITCH_CORE_INBAND_CNTL_N_P6;
	uint8 dummy1986[12];
	uint32 SWITCH_CORE_INBAND_CNTL_N_P7;
	uint8 dummy1987[12];
	uint32 SWITCH_CORE_MEM_COUNTER;
	uint8 dummy1988[12];
	uint32 SWITCH_CORE_TIMESTAMP_DELTA;
	uint8 dummy1989[12];
	uint32 SWITCH_CORE_SOP_SEL;
	uint8 dummy1990[12];
	uint32 SWITCH_CORE_TIME_STAMP_INFO_3;
	uint8 dummy1991[12];
	uint32 SWITCH_CORE_TIME_STAMP_INFO_4;
	uint8 dummy1992[12];
	uint32 SWITCH_CORE_TIME_STAMP_INFO_5;
	uint8 dummy1993[12];
	uint32 SWITCH_CORE_TIME_STAMP_INFO_6;
	uint8 dummy1994[12];
	uint32 SWITCH_CORE_TIME_STAMP_INFO_7;
	uint8 dummy1995[12];
	uint32 SWITCH_CORE_TIME_STAMP_INFO_8;
	uint8 dummy1996[12];
	uint32 SWITCH_CORE_INBAND_SPARE1;
	uint8 dummy1997[140];
	uint32 SWITCH_CORE_RED_CONTROL;
	uint8 dummy1998[12];
	uint32 SWITCH_CORE_TC2RED_PROFILE_TABLE;
	uint8 dummy1999[12];
	uint32 SWITCH_CORE_RED_EGRESS_BYPASS;
	uint8 dummy2000[12];
	uint32 SWITCH_CORE_RED_AQD_CONTROL;
	uint8 dummy2001[12];
	uint32 SWITCH_CORE_RED_EXPONENT;
	uint8 dummy2002[12];
	uint32 SWITCH_CORE_RED_DROP_ADD_TO_MIB;
	uint8 dummy2003[44];
	uint32 SWITCH_CORE_RED_PROFILE_DEFAULT;
	uint8 dummy2004[28];
	uint32 SWITCH_CORE_WRED_REG_SPARE0;
	uint8 dummy2005[28];
	uint32 SWITCH_CORE_WRED_REG_SPARE1;
	uint8 dummy2006[60];
	uint32 SWITCH_CORE_RED_PROFILE0;
	uint8 dummy2007[28];
	uint32 SWITCH_CORE_RED_PROFILE1;
	uint8 dummy2008[28];
	uint32 SWITCH_CORE_RED_PROFILE2;
	uint8 dummy2009[28];
	uint32 SWITCH_CORE_RED_PROFILE3;
	uint8 dummy2010[28];
	uint32 SWITCH_CORE_RED_PROFILE4;
	uint8 dummy2011[28];
	uint32 SWITCH_CORE_RED_PROFILE5;
	uint8 dummy2012[28];
	uint32 SWITCH_CORE_RED_PROFILE6;
	uint8 dummy2013[28];
	uint32 SWITCH_CORE_RED_PROFILE7;
	uint8 dummy2014[28];
	uint32 SWITCH_CORE_RED_PROFILE8;
	uint8 dummy2015[28];
	uint32 SWITCH_CORE_RED_PROFILE9;
	uint8 dummy2016[28];
	uint32 SWITCH_CORE_RED_PROFILE10;
	uint8 dummy2017[28];
	uint32 SWITCH_CORE_RED_PROFILE11;
	uint8 dummy2018[28];
	uint32 SWITCH_CORE_RED_PROFILE12;
	uint8 dummy2019[28];
	uint32 SWITCH_CORE_RED_PROFILE13;
	uint8 dummy2020[28];
	uint32 SWITCH_CORE_RED_PROFILE14;
	uint8 dummy2021[28];
	uint32 SWITCH_CORE_RED_PROFILE15;
	uint8 dummy2022[124];
	uint32 SWITCH_CORE_RED_DROP_CNTR_RST;
	uint8 dummy2023[28];
	uint32 SWITCH_CORE_RED_PKT_DROP_CNTR_P0;
	uint8 dummy2024[28];
	uint32 SWITCH_CORE_RED_PKT_DROP_CNTR_P1;
	uint8 dummy2025[28];
	uint32 SWITCH_CORE_RED_PKT_DROP_CNTR_P2;
	uint8 dummy2026[28];
	uint32 SWITCH_CORE_RED_PKT_DROP_CNTR_P3;
	uint8 dummy2027[28];
	uint32 SWITCH_CORE_RED_PKT_DROP_CNTR_P4;
	uint8 dummy2028[28];
	uint32 SWITCH_CORE_RED_PKT_DROP_CNTR_P5;
	uint8 dummy2029[28];
	uint32 SWITCH_CORE_RED_PKT_DROP_CNTR_P6;
	uint8 dummy2030[28];
	uint32 SWITCH_CORE_RED_PKT_DROP_CNTR_P7;
	uint8 dummy2031[28];
	uint32 SWITCH_CORE_RED_PKT_DROP_CNTR_IMP;
	uint8 dummy2032[124];
	uint32 SWITCH_CORE_RED_BYTE_DROP_CNTR_P0;
	uint8 dummy2033[60];
	uint32 SWITCH_CORE_RED_BYTE_DROP_CNTR_P1;
	uint8 dummy2034[60];
	uint32 SWITCH_CORE_RED_BYTE_DROP_CNTR_P2;
	uint8 dummy2035[60];
	uint32 SWITCH_CORE_RED_BYTE_DROP_CNTR_P3;
	uint8 dummy2036[60];
	uint32 SWITCH_CORE_RED_BYTE_DROP_CNTR_P4;
	uint8 dummy2037[60];
	uint32 SWITCH_CORE_RED_BYTE_DROP_CNTR_P5;
	uint8 dummy2038[60];
	uint32 SWITCH_CORE_RED_BYTE_DROP_CNTR_P6;
	uint8 dummy2039[60];
	uint32 SWITCH_CORE_RED_BYTE_DROP_CNTR_P7;
	uint8 dummy2040[60];
	uint32 SWITCH_CORE_RED_BYTE_DROP_CNTR_IMP;
	uint8 dummy2041[20732];
	uint32 SWITCH_CORE_CFP_ACC;
	uint8 dummy2042[28];
	uint32 SWITCH_CORE_RATE_METER_GLOBAL_CTL;
	uint8 dummy2043[92];
	uint32 SWITCH_CORE_CFP_DATA0;
	uint8 dummy2044[28];
	uint32 SWITCH_CORE_CFP_DATA1;
	uint8 dummy2045[28];
	uint32 SWITCH_CORE_CFP_DATA2;
	uint8 dummy2046[28];
	uint32 SWITCH_CORE_CFP_DATA3;
	uint8 dummy2047[28];
	uint32 SWITCH_CORE_CFP_DATA4;
	uint8 dummy2048[28];
	uint32 SWITCH_CORE_CFP_DATA5;
	uint8 dummy2049[28];
	uint32 SWITCH_CORE_CFP_DATA6;
	uint8 dummy2050[28];
	uint32 SWITCH_CORE_CFP_DATA7;
	uint8 dummy2051[28];
	uint32 SWITCH_CORE_CFP_MASK0;
	uint8 dummy2052[28];
	uint32 SWITCH_CORE_CFP_MASK1;
	uint8 dummy2053[28];
	uint32 SWITCH_CORE_CFP_MASK2;
	uint8 dummy2054[28];
	uint32 SWITCH_CORE_CFP_MASK3;
	uint8 dummy2055[28];
	uint32 SWITCH_CORE_CFP_MASK4;
	uint8 dummy2056[28];
	uint32 SWITCH_CORE_CFP_MASK5;
	uint8 dummy2057[28];
	uint32 SWITCH_CORE_CFP_MASK6;
	uint8 dummy2058[28];
	uint32 SWITCH_CORE_CFP_MASK7;
	uint8 dummy2059[28];
	uint32 SWITCH_CORE_ACT_POL_DATA0;
	uint8 dummy2060[28];
	uint32 SWITCH_CORE_ACT_POL_DATA1;
	uint8 dummy2061[28];
	uint32 SWITCH_CORE_ACT_POL_DATA2;
	uint8 dummy2062[60];
	uint32 SWITCH_CORE_RATE_METER0;
	uint8 dummy2063[28];
	uint32 SWITCH_CORE_RATE_METER1;
	uint8 dummy2064[28];
	uint32 SWITCH_CORE_RATE_METER2;
	uint8 dummy2065[28];
	uint32 SWITCH_CORE_RATE_METER3;
	uint8 dummy2066[28];
	uint32 SWITCH_CORE_RATE_METER4;
	uint8 dummy2067[28];
	uint32 SWITCH_CORE_RATE_METER5;
	uint8 dummy2068[28];
	uint32 SWITCH_CORE_RATE_METER6;
	uint8 dummy2069[28];
	uint32 SWITCH_CORE_TC2COLOR;
	uint8 dummy2070[28];
	uint32 SWITCH_CORE_STAT_GREEN_CNTR;
	uint8 dummy2071[28];
	uint32 SWITCH_CORE_STAT_YELLOW_CNTR;
	uint8 dummy2072[28];
	uint32 SWITCH_CORE_STAT_RED_CNTR;
	uint8 dummy2073[188];
	uint32 SWITCH_CORE_TCAM_BIST_CONTROL;
	uint8 dummy2074[28];
	uint32 SWITCH_CORE_TCAM_BIST_STATUS;
	uint8 dummy2075[28];
	uint32 SWITCH_CORE_TCAM_TEST_COMPARE_STATUS;
	uint8 dummy2076[60];
	uint32 SWITCH_CORE_CFP_REG_SPARE0;
	uint8 dummy2077[28];
	uint32 SWITCH_CORE_CFP_REG_SPARE1;
	uint8 dummy2078[604];
	uint32 SWITCH_CORE_CFP_CTL_REG;
	uint8 dummy2079[124];
	uint64 SWITCH_CORE_UDF_0_A_0_8_0;
	uint64 SWITCH_CORE_UDF_0_A_0_8_1;
	uint64 SWITCH_CORE_UDF_0_A_0_8_2;
	uint64 SWITCH_CORE_UDF_0_A_0_8_3;
	uint64 SWITCH_CORE_UDF_0_A_0_8_4;
	uint64 SWITCH_CORE_UDF_0_A_0_8_5;
	uint64 SWITCH_CORE_UDF_0_A_0_8_6;
	uint64 SWITCH_CORE_UDF_0_A_0_8_7;
	uint32 SWITCH_CORE_UDF_0_A_0_8_8;
	uint8 dummy2080[60];
	uint64 SWITCH_CORE_UDF_1_A_0_8_0;
	uint64 SWITCH_CORE_UDF_1_A_0_8_1;
	uint64 SWITCH_CORE_UDF_1_A_0_8_2;
	uint64 SWITCH_CORE_UDF_1_A_0_8_3;
	uint64 SWITCH_CORE_UDF_1_A_0_8_4;
	uint64 SWITCH_CORE_UDF_1_A_0_8_5;
	uint64 SWITCH_CORE_UDF_1_A_0_8_6;
	uint64 SWITCH_CORE_UDF_1_A_0_8_7;
	uint32 SWITCH_CORE_UDF_1_A_0_8_8;
	uint8 dummy2081[60];
	uint64 SWITCH_CORE_UDF_2_A_0_8_0;
	uint64 SWITCH_CORE_UDF_2_A_0_8_1;
	uint64 SWITCH_CORE_UDF_2_A_0_8_2;
	uint64 SWITCH_CORE_UDF_2_A_0_8_3;
	uint64 SWITCH_CORE_UDF_2_A_0_8_4;
	uint64 SWITCH_CORE_UDF_2_A_0_8_5;
	uint64 SWITCH_CORE_UDF_2_A_0_8_6;
	uint64 SWITCH_CORE_UDF_2_A_0_8_7;
	uint32 SWITCH_CORE_UDF_2_A_0_8_8;
	uint8 dummy2082[60];
	uint64 SWITCH_CORE_UDF_0_B_0_8_0;
	uint64 SWITCH_CORE_UDF_0_B_0_8_1;
	uint64 SWITCH_CORE_UDF_0_B_0_8_2;
	uint64 SWITCH_CORE_UDF_0_B_0_8_3;
	uint64 SWITCH_CORE_UDF_0_B_0_8_4;
	uint64 SWITCH_CORE_UDF_0_B_0_8_5;
	uint64 SWITCH_CORE_UDF_0_B_0_8_6;
	uint64 SWITCH_CORE_UDF_0_B_0_8_7;
	uint32 SWITCH_CORE_UDF_0_B_0_8_8;
	uint8 dummy2083[60];
	uint64 SWITCH_CORE_UDF_1_B_0_8_0;
	uint64 SWITCH_CORE_UDF_1_B_0_8_1;
	uint64 SWITCH_CORE_UDF_1_B_0_8_2;
	uint64 SWITCH_CORE_UDF_1_B_0_8_3;
	uint64 SWITCH_CORE_UDF_1_B_0_8_4;
	uint64 SWITCH_CORE_UDF_1_B_0_8_5;
	uint64 SWITCH_CORE_UDF_1_B_0_8_6;
	uint64 SWITCH_CORE_UDF_1_B_0_8_7;
	uint32 SWITCH_CORE_UDF_1_B_0_8_8;
	uint8 dummy2084[60];
	uint64 SWITCH_CORE_UDF_2_B_0_8_0;
	uint64 SWITCH_CORE_UDF_2_B_0_8_1;
	uint64 SWITCH_CORE_UDF_2_B_0_8_2;
	uint64 SWITCH_CORE_UDF_2_B_0_8_3;
	uint64 SWITCH_CORE_UDF_2_B_0_8_4;
	uint64 SWITCH_CORE_UDF_2_B_0_8_5;
	uint64 SWITCH_CORE_UDF_2_B_0_8_6;
	uint64 SWITCH_CORE_UDF_2_B_0_8_7;
	uint32 SWITCH_CORE_UDF_2_B_0_8_8;
	uint8 dummy2085[60];
	uint64 SWITCH_CORE_UDF_0_C_0_8_0;
	uint64 SWITCH_CORE_UDF_0_C_0_8_1;
	uint64 SWITCH_CORE_UDF_0_C_0_8_2;
	uint64 SWITCH_CORE_UDF_0_C_0_8_3;
	uint64 SWITCH_CORE_UDF_0_C_0_8_4;
	uint64 SWITCH_CORE_UDF_0_C_0_8_5;
	uint64 SWITCH_CORE_UDF_0_C_0_8_6;
	uint64 SWITCH_CORE_UDF_0_C_0_8_7;
	uint32 SWITCH_CORE_UDF_0_C_0_8_8;
	uint8 dummy2086[60];
	uint64 SWITCH_CORE_UDF_1_C_0_8_0;
	uint64 SWITCH_CORE_UDF_1_C_0_8_1;
	uint64 SWITCH_CORE_UDF_1_C_0_8_2;
	uint64 SWITCH_CORE_UDF_1_C_0_8_3;
	uint64 SWITCH_CORE_UDF_1_C_0_8_4;
	uint64 SWITCH_CORE_UDF_1_C_0_8_5;
	uint64 SWITCH_CORE_UDF_1_C_0_8_6;
	uint64 SWITCH_CORE_UDF_1_C_0_8_7;
	uint32 SWITCH_CORE_UDF_1_C_0_8_8;
	uint8 dummy2087[60];
	uint64 SWITCH_CORE_UDF_2_C_0_8_0;
	uint64 SWITCH_CORE_UDF_2_C_0_8_1;
	uint64 SWITCH_CORE_UDF_2_C_0_8_2;
	uint64 SWITCH_CORE_UDF_2_C_0_8_3;
	uint64 SWITCH_CORE_UDF_2_C_0_8_4;
	uint64 SWITCH_CORE_UDF_2_C_0_8_5;
	uint64 SWITCH_CORE_UDF_2_C_0_8_6;
	uint64 SWITCH_CORE_UDF_2_C_0_8_7;
	uint32 SWITCH_CORE_UDF_2_C_0_8_8;
	uint8 dummy2088[60];
	uint64 SWITCH_CORE_UDF_0_D_0_11_0;
	uint64 SWITCH_CORE_UDF_0_D_0_11_1;
	uint64 SWITCH_CORE_UDF_0_D_0_11_2;
	uint64 SWITCH_CORE_UDF_0_D_0_11_3;
	uint64 SWITCH_CORE_UDF_0_D_0_11_4;
	uint64 SWITCH_CORE_UDF_0_D_0_11_5;
	uint64 SWITCH_CORE_UDF_0_D_0_11_6;
	uint64 SWITCH_CORE_UDF_0_D_0_11_7;
	uint64 SWITCH_CORE_UDF_0_D_0_11_8;
	uint64 SWITCH_CORE_UDF_0_D_0_11_9;
	uint64 SWITCH_CORE_UDF_0_D_0_11_10;
	uint32 SWITCH_CORE_UDF_0_D_0_11_11;
	uint8 dummy2089[29348];
	uint32 SWITCH_CORE_ARL_TCAM_ACC;
	uint8 dummy2090[28];
	uint32 SWITCH_CORE_ARL_TCAM_DATA_P0;
	uint8 dummy2091[28];
	uint32 SWITCH_CORE_ARL_TCAM_DATA_P1;
	uint8 dummy2092[92];
	uint32 SWITCH_CORE_ARL_SMEM_DATA;
	uint8 dummy2093[92];
	uint32 SWITCH_CORE_ARL_TCAM_BIST_CTRL;
	uint8 dummy2094[28];
	uint32 SWITCH_CORE_ARL_TCAM_BIST_STS;
	uint8 dummy2095[100060];
	uint32 SWITCH_CORE_PLL_NDIV_INT;
	uint8 dummy2096[12];
	uint32 SWITCH_CORE_PLL_NDIV_FRAC;
	uint8 dummy2097[28];
	uint32 SWITCH_CORE_PLL_SDMOD_CTRL;
	uint8 dummy2098[12];
	uint32 SWITCH_CORE_PLL_MOD_CTRL_0;
	uint8 dummy2099[28];
	uint32 SWITCH_CORE_PLL_MOD_CTRL_1;
	uint8 dummy2100[28];
	uint32 SWITCH_CORE_PLL_MOD_CTRL_2;
	uint8 dummy2101[28];
	uint32 SWITCH_CORE_PLL_MISC_CTRL;
	uint8 dummy2102[12];
	uint32 SWITCH_CORE_PLL_DELOCK_MIB;
	uint8 dummy2103[28];
	uint32 SWITCH_CORE_PLL_SS_CTL;
	uint8 dummy2104[44];
	uint32 SWITCH_CORE_PLL_CTRL;
	uint8 dummy2105[12];
	uint32 SWITCH_CORE_PLL_STS;
	uint8 dummy2106[60];
	uint32 SWITCH_CORE_PLL_FREQ_SEL;
	uint8 dummy2107[44];
	uint32 SWITCH_CORE_PLL_TEST_CTRL_I;
	uint8 dummy2108[12];
	uint32 SWITCH_CORE_PLL_TEST_CTRL_II;
	uint8 dummy2109[108];
	uint32 SWITCH_CORE_GREEN_MODE_DATA;
	uint8 dummy2110[60];
	uint32 SWITCH_CORE_GREEN_MODE_SELECT;
	uint8 dummy2111[444];
	uint32 SWITCH_CORE_TOP_LOW_POWER_CTRL;
	uint8 dummy2112[12];
	uint32 SWITCH_CORE_TOP_IDDQ_CTL;
	uint8 dummy2113[108];
	uint32 SWITCH_CORE_IP_PLL_BYPASS;
	uint8 dummy2114[636];
	uint32 SWITCH_CORE_TOP_MODULE_CTL_SPARE0;
	uint8 dummy2115[28];
	uint32 SWITCH_CORE_TOP_MODULE_CTL_SPARE1;
	uint8 dummy2116[220];
	uint32 SWITCH_CORE_EGPHY_CTRL;
	uint8 dummy2117[12];
	uint32 SWITCH_CORE_EGPHY_PWRMGNT;
	uint8 dummy2118[12];
	uint32 SWITCH_CORE_EGPHY_PWR_DOWN;
	uint8 dummy2119[44];
	uint32 SWITCH_CORE_EGPHY_STRAP;
	uint8 dummy2120[12];
	uint32 SWITCH_CORE_EGPHY_STS;
	uint8 dummy2121[12];
	uint32 SWITCH_CORE_EGPHY_INT_STS;
	uint8 dummy2122[12];
	uint32 SWITCH_CORE_EGPHY_MODE_STS;
	uint8 dummy2123[12];
	uint32 SWITCH_CORE_EGPHY_LPI_STS;
	uint8 dummy2124[12];
	uint32 SWITCH_CORE_EGPHY_ENG_DET_STS;
	uint8 dummy2125[12];
	uint32 SWITCH_CORE_EGPHY_ENG_DET_STS_CHG;
	uint8 dummy2126[12];
	uint32 SWITCH_CORE_EGPHY_RESET_STATUS;
	uint8 dummy2127[1596];
	uint32 SWITCH_CORE_EGPHY_CTL_SPARE0;
	uint8 dummy2128[28];
	uint32 SWITCH_CORE_EGPHY_CTL_SPARE1;
	uint8 dummy2129[220];
	uint32 SWITCH_CORE_BRPHY_CTRL;
	uint8 dummy2130[12];
	uint32 SWITCH_CORE_BRPHY_PWRMGNT;
	uint8 dummy2131[12];
	uint32 SWITCH_CORE_BRPHY_PWR_DOWN;
	uint8 dummy2132[28];
	uint32 SWITCH_CORE_BRPHY_PLL_CTRL;
	uint8 dummy2133[28];
	uint32 SWITCH_CORE_BRPHY_STS;
	uint8 dummy2134[12];
	uint32 SWITCH_CORE_BRPHY_INT_STS;
	uint8 dummy2135[44];
	uint32 SWITCH_CORE_BRPHY_ENG_DET_STS;
	uint8 dummy2136[12];
	uint32 SWITCH_CORE_BRPHY_ENG_DET_STS_CHG;
	uint8 dummy2137[12];
	uint32 SWITCH_CORE_BRPHY_RESET_STATUS;
	uint8 dummy2138[1596];
	uint32 SWITCH_CORE_BRPHY_CTL_SPARE0;
	uint8 dummy2139[28];
	uint32 SWITCH_CORE_BRPHY_CTL_SPARE1;
	uint8 dummy2140[220];
	uint32 SWITCH_CORE_STS_OVERRIDE_GMII_P0;
	uint8 dummy2141[12];
	uint32 SWITCH_CORE_STS_OVERRIDE_GMII_P1;
	uint8 dummy2142[12];
	uint32 SWITCH_CORE_STS_OVERRIDE_GMII_P2;
	uint8 dummy2143[12];
	uint32 SWITCH_CORE_STS_OVERRIDE_GMII_P3;
	uint8 dummy2144[12];
	uint32 SWITCH_CORE_STS_OVERRIDE_GMII_P4;
	uint8 dummy2145[12];
	uint32 SWITCH_CORE_STS_OVERRIDE_GMII_P5;
	uint8 dummy2146[12];
	uint32 SWITCH_CORE_STS_OVERRIDE_P6;
	uint8 dummy2147[12];
	uint32 SWITCH_CORE_STS_OVERRIDE_GMII_P7;
	uint8 dummy2148[12];
	uint32 imp_port_state;
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
	uint8 dummy2149[60];
	uint32 SWITCH_CORE_PAUSE_CAP;
	uint8 dummy2150[124];
	uint32 SWITCH_CORE_PORT4_RGMII_CTL_GP;
	uint8 dummy2151[12];
	uint32 SWITCH_CORE_PORT5_RGMII_CTL_GP;
	uint8 dummy2152[12];
	uint32 SWITCH_CORE_PORT6_RGMII_CTL_GP;
	uint8 dummy2153[28];
	uint32 SWITCH_CORE_RGMII_CTL_GP_IMP;
	uint8 dummy2154[188];
	uint32 SWITCH_CORE_P4_RGMII_TIME_DLY_GP;
	uint8 dummy2155[12];
	uint32 SWITCH_CORE_P5_RGMII_TIME_DLY_GP;
	uint8 dummy2156[12];
	uint32 SWITCH_CORE_P6_RGMII_TIME_DLY_GP;
	uint8 dummy2157[28];
	uint32 SWITCH_CORE_RGMII_TIME_DLY_GP_IMP;
	uint8 dummy2158[124];
	uint32 SWITCH_CORE_RM_PINS_DEBUG;
	uint8 dummy2159[12];
	uint32 SWITCH_CORE_MII_IDDQ_CTRL;
	uint8 dummy2160[12];
	uint32 SWITCH_CORE_MII_LOW_POWER_CTRL;
	uint8 dummy2161[12];
	uint32 SWITCH_CORE_LED_OPTIONS;
	uint8 dummy2162[972];
	uint32 SWITCH_CORE_PORT_INFO_SPARE0;
	uint8 dummy2163[28];
	uint32 SWITCH_CORE_PORT_INFO_SPARE1;
	uint8 dummy2164[220];
	uint32 SWITCH_CORE_IO_SR_CTL;
	uint8 dummy2165[28];
	uint32 SWITCH_CORE_IO_DS_SEL0;
	uint8 dummy2166[60];
	uint32 SWITCH_CORE_IO_DS_SEL2;
	uint8 dummy2167[28];
	uint32 SWITCH_CORE_GMII_IO_SR_CTL;
	uint8 dummy2168[28];
	uint32 SWITCH_CORE_GMII_IO_DS_SEL0;
	uint8 dummy2169[28];
	uint32 SWITCH_CORE_GMII_IO_DS_SEL1;
	uint8 dummy2170[28];
	uint32 SWITCH_CORE_GMII_VOL_SEL;
	uint8 dummy2171[12];
	uint32 SWITCH_CORE_PINS_DEBUG_IMP;
	uint8 dummy2172[28];
	uint32 SWITCH_CORE_BONDING_PAD_STATUS;
	uint8 dummy2173[12];
	uint32 SWITCH_CORE_STRAP_PIN_STATUS;
	uint8 dummy2174[28];
	uint32 SWITCH_CORE_DIRECT_INPUT_CTRL_VALUE;
	uint8 dummy2175[60];
	uint32 SWITCH_CORE_EMB_CPU_STATUS;
	uint8 dummy2176[1404];
	uint32 SWITCH_CORE_CHIP_CTL_SPARE0;
	uint8 dummy2177[28];
	uint32 SWITCH_CORE_CHIP_CTL_SPARE1;

} EthernetSwitchCore;

#define PBMAP_MIPS 0x100
#define ETHSW_CORE ((volatile EthernetSwitchCore * const) SWITCH_CORE_BASE)

typedef struct sys_port_intr2 {
	uint32 SYSTEMPORT_INTR_CPU_STATUS;
	uint32 SYSTEMPORT_INTR_CPU_SET;
	uint32 SYSTEMPORT_INTR_CPU_CLEAR;
	uint32 SYSTEMPORT_INTR_CPU_MASK_STATUS;
	uint32 SYSTEMPORT_INTR_CPU_MASK_SET;
	uint32 SYSTEMPORT_INTR_CPU_MASK_CLEAR;
	uint8  reserved[8];
}sys_port_intr2, SYSTEMPORT_INTRL2;

#define SYSPORT_INTC_MAPPING_TDMA_MULTI_BUFFER_DONE_INTR  0
#define SYSPORT_INTC_MAPPING_RDMA_MULTI_BUFFER_DONE_INTR  1
#define SYSPORT_INTC_MAPPING_CSB_DONE_INTR                2
#define SYSPORT_INTC_MAPPING_PSB_DONE_INTR                3
#define SYSPORT_INTC_MAPPING_TX_MISC_INTR                 2
#define SYSPORT_INTC_MAPPING_RX_MISC_INTR                 3

#define SYSPORT_INTC_MAPPING_INT_SEL_S(_index)                          \
    ( ((_index) < 16) ? ((_index) * 2) : (((_index) - 16) * 2) )

typedef struct sys_port_intc
{
    uint32 SYSTEMPORT_INTC_INTC_MAPPING_1;//0x80498300
    uint32 SYSTEMPORT_INTC_INTC_MAPPING_0;//0x80498304
    uint32 SYSTEMPORT_INTC_INTC_PSB_EN;//0x80498308
    uint32 SYSTEMPORT_INTC_INTC_CSB_EN;//0x8049830c
	uint8  reserved[16];
}sys_port_intc, SYSTEMPORT_INTC;


#define FIELD_MASK(bits, shift)  ( ( (1ULL<<(bits)) - 1 ) << shift )


#define SYSPORT_RDMA_CTRL_RDMA_EN_M             FIELD_MASK(1,0)
#define SYSPORT_RDMA_CTRL_RING_CFG_bit1_M       FIELD_MASK(1,1)
#define SYSPORT_RDMA_CTRL_DISCARD_EN_M          FIELD_MASK(1,2)
#define SYSPORT_RDMA_CTRL_DATA_OFFSET_M         FIELD_MASK(10,4)
#define SYSPORT_RDMA_CTRL_RING_CFG_bit0_M       FIELD_MASK(1,14)
#define SYSPORT_RDMA_CTRL_DDR_DESC_WR_EN_M      FIELD_MASK(1,15)
#define SYSPORT_RDMA_CTRL_PSB_EN_M              FIELD_MASK(1,18)

#define SYSPORT_RDMA_STATUS_RDMA_DISABLED_M     FIELD_MASK(1,0)
#define SYSPORT_RDMA_STATUS_DESC_RAM_BUSY_M     FIELD_MASK(1,1)

#define SYSTEMPORT_RDMA_LOCRAM_DESCRING_SIZE_MAX            256
#define SYSTEMPORT_RDMA_LOCRAM_DESCRING_SIZE_RING_FLUSH_M   FIELD_MASK(1,16)
#define SYSTEMPORT_RDMA_LOCRAM_DESCRING_SIZE_RING_EN_M      FIELD_MASK(1,17)

#define SYSPORT_RDMA_PSB_INTR_CFG_THRESHOLD_M     FIELD_MASK(16,0)
#define SYSPORT_RDMA_PSB_INTR_CFG_THRESHOLD_S     0
#define SYSPORT_RDMA_PSB_INTR_CFG_TIMEOUT_M       FIELD_MASK(16,16)
#define SYSPORT_RDMA_PSB_INTR_CFG_TIMEOUT_S       16

typedef struct sys_port_rdma {
	uint32 SYSTEMPORT_RDMA_DESCRIPTOR[1024];
	uint32 SYSTEMPORT_RDMA_DESC_WRITE_PORT[16];
	uint32 SYSTEMPORT_RDMA_DESC_READ_PORT[16];
	uint32 SYSTEMPORT_RDMA_CONTROL;
	uint32 SYSTEMPORT_RDMA_STATUS;
	uint32 SYSTEMPORT_RDMA_SYSBUS_BURST;
	uint32 SYSTEMPORT_RDMA_SWITCH_PORT_BP_EN;
	uint32 SYSTEMPORT_RDMA_RXQ_TO_SWITCH_EGRESS_QUEUES_BP_MAP[8];
#define SYSPORT_RDMA_RING_EN	(1<<17)
	uint32 SYSTEMPORT_RDMA_LOCRAM_DESCRING_SIZE[8];
	uint32 SYSTEMPORT_RDMA_PKTBUF_SIZE[8];
	uint32 SYSTEMPORT_RDMA_PINDEX[8];
	uint32 SYSTEMPORT_RDMA_CINDEX[8];
	uint32 SYSTEMPORT_RDMA_BUFDONE_INTR_CFG[8];
	uint32 SYSTEMPORT_RDMA_XON_XOFF_CFG[8];
	uint32 SYSTEMPORT_RDMA_DDR_DESC_RING_START[16];
	uint32 SYSTEMPORT_RDMA_DDR_DESC_RING_SIZE[8];
	uint32 SYSTEMPORT_RDMA_DDR_DESC_RING_CTRL[8];
	uint32 SYSTEMPORT_RDMA_DDR_DESC_RING_WR_PUSH_TIMER[8];
	uint32 SYSTEMPORT_RDMA_BP_STATUS;
	uint32 SYSTEMPORT_RDMA_DESC_RAM_ARB_CONTROL;
	uint32 SYSTEMPORT_RDMA_DESC_RAM_RD_ARB_CFG;
	uint32 SYSTEMPORT_RDMA_DESC_RAM_WR_ARB_CFG;
	uint32 SYSTEMPORT_RDMA_PSB_INTR_CFG;
	uint32 SYSTEMPORT_RDMA_PSB_ADDR_LO;
	uint32 SYSTEMPORT_RDMA_PSB_ADDR_HI;
	uint32 SYSTEMPORT_RDMA_TEST;
	uint32 SYSTEMPORT_RDMA_DEBUG;
}sys_port_rdma, SYSTEMPORT_RDMA;

#define SYSPORT_RBUF_CTRL_RSB_MODE_M          FIELD_MASK(2,0)
#define SYSPORT_RBUF_CTRL_RSB_MODE_S          0
#define SYSPORT_RBUF_CTRL_4B_ALIGN_M          FIELD_MASK(1,2)
#define SYSPORT_RBUF_CTRL_BTAG_STRIP_M        FIELD_MASK(1,3)
#define SYSPORT_RBUF_CTRL_BAD_PKT_DISCARD_M   FIELD_MASK(1,4)
#define SYSPORT_RBUF_CTRL_CRC_REPLACE_M       FIELD_MASK(1,21)

#define SYSPORT_RBUF_CTRL_RSB_MODE_NO_RSB     0
#define SYSPORT_RBUF_CTRL_RSB_MODE_RSB8       1
#define SYSPORT_RBUF_CTRL_RSB_MODE_RSB32      2

#define SYSPORT_RBUF_PACKET_READY_THRESHOLD_PKT_RDY_THRESHOLD_M   FIELD_MASK(10,0)
#define SYSPORT_RBUF_PACKET_READY_THRESHOLD_PKT_RDY_THRESHOLD_S   0
#define SYSPORT_RBUF_PACKET_READY_THRESHOLD_RESUME_THRESHOLD_M    FIELD_MASK(10,16)
#define SYSPORT_RBUF_PACKET_READY_THRESHOLD_RESUME_THRESHOLD_S    16

typedef struct sys_port_rbuf {
	uint32 SYSTEMPORT_RBUF_RBUF_CONTROL;
	uint32 SYSTEMPORT_RBUF_RBUF_PACKET_READY_THRESHOLD;
	uint32 SYSTEMPORT_RBUF_RBUF_STATUS;
	uint32 SYSTEMPORT_RBUF_RBUF_OVERFLOW_PACKET_DISCARD_COUNT;
	uint32 SYSTEMPORT_RBUF_RBUF_ERROR_PACKET_COUNT;
}sys_port_rbuf, SYSTEMPORT_RBUF;


typedef struct sys_port_tbuf {
	uint32 SYSTEMPORT_TBUF_TBUF_CONTROL;
	uint32 SYSTEMPORT_TBUF_PKT_RDY_THRESHOLD;
	uint32 SYSTEMPORT_TBUF_TBUF_STATUS;
}sys_port_tbuf, SYSTEMPORT_TBUF;

#define SYSTEMPORT_TDMA_DESC_RING_MAX        28
#define SYSTEMPORT_TDMA_LOCRAM_DESCRING_MAX  896

#define SYSTEMPORT_TDMA_TIMEOUT_TICK_NSEC    1000 // tick = 1 usec (internal timer)

typedef struct sys_port_mpd {
	uint32 SYSTEMPORT_MPD_CTRL;
	uint32 SYSTEMPORT_MPD_PSW_MS;
	uint32 SYSTEMPORT_MPD_PSW_LS;
}sys_port_mpd, SYSTEMPORT_MPD;

typedef struct systemport_tdma_descriptor_write_port
{
		uint32 SYSTEMPORT_TDMA_DESCRIPTOR_XX_WRITE_PORT_LO;
		uint32 SYSTEMPORT_TDMA_DESCRIPTOR_XX_WRITE_PORT_HI;
}systemport_tdma_descriptor_write_port;

typedef struct systemport_tdma_descriptor_read_port
{
		uint32 SYSTEMPORT_TDMA_DESCRIPTOR_XX_READ_PORT_LO;
		uint32 SYSTEMPORT_TDMA_DESCRIPTOR_XX_READ_PORT_HI;
}systemport_tdma_descriptor_read_port;

#define SYSTEMPORT_TDMA_DESC_RING_XX_PRODUCER_CONSUMER_INDEX_CONSUMER_INDEX_S      16
#define SYSTEMPORT_TDMA_DESC_RING_XX_PRODUCER_CONSUMER_INDEX_CONSUMER_INDEX_M      FIELD_MASK(16,16)
#define SYSTEMPORT_TDMA_DESC_RING_XX_PRODUCER_CONSUMER_INDEX_PRODUCER_INDEX_M      FIELD_MASK(16,0)

#define SYSPORT_TDMA_CONTROL_TDMA_EN_M                  FIELD_MASK(1,0)
#define SYSPORT_TDMA_CONTROL_TSB_EN_M                   FIELD_MASK(1,1)
#define SYSPORT_TDMA_CONTROL_DATA_OFFSET_M              FIELD_MASK(10,5)
#define SYSPORT_TDMA_CONTROL_VLAN_EN_M                  FIELD_MASK(1,15)
#define SYSPORT_TDMA_CONTROL_SW_BRCM_TAG_M              FIELD_MASK(1,16)
#define SYSPORT_TDMA_CONTROL_WNC_PKT_SIZE_UPDATE_MODE   FIELD_MASK(1,17)
#define SYSPORT_TDMA_CONTROL_SYNC_PKT_SIZE_UPDATE_MODE  FIELD_MASK(1,18)
#define SYSPORT_TDMA_CONTROL_RING_CFG_M                 FIELD_MASK(1,19)
#define SYSPORT_TDMA_CONTROL_MAX_INFLIGHT_PBUFDMA       FIELD_MASK(5,22)
#define SYSPORT_TDMA_CONTROL_ACB_EN_M                   FIELD_MASK(1,27)
#define SYSPORT_TDMA_CONTROL_CSB_WR_EN                  FIELD_MASK(1,28)
#define SYSPORT_TDMA_CONTROL_TPD_16b_EN                 FIELD_MASK(1,29)

#define SYSPORT_TDMA_DESC_RING_CONTROL_FLUSH_M          FIELD_MASK(1,0)
#define SYSPORT_TDMA_DESC_RING_CONTROL_RING_EN_M        FIELD_MASK(1,1)

#define SYSPORT_TDMA_DESC_RING_INTR_CONTROL_THRESHOLD_S      0
#define SYSPORT_TDMA_DESC_RING_INTR_CONTROL_EMPTY_INTR_EN_M  FIELD_MASK(1,15)
#define SYSPORT_TDMA_DESC_RING_INTR_CONTROL_TIMEOUT_S        16

#define SYSPORT_TDMA_DESC_RING_MAPPING_QID_M            FIELD_MASK(3,0)
#define SYSPORT_TDMA_DESC_RING_MAPPING_QID_S            0
#define SYSPORT_TDMA_DESC_RING_MAPPING_PORT_ID_S        3
#define SYSPORT_TDMA_DESC_RING_MAPPING_IGNORE_STATUS_M  FIELD_MASK(1,6)
#define SYSPORT_TDMA_DESC_RING_MAPPING_CREDIT_S         8

#define SYSTEMPORT_TDMA_ARBITER_MODE_SP           0
#define SYSTEMPORT_TDMA_ARBITER_MODE_RR           1
#define SYSTEMPORT_TDMA_ARBITER_MODE_WRR          2
#define SYSTEMPORT_TDMA_ARBITER_MODE_DRR          3

#define SYSPORT_TDMA_STATUS_TDMA_DISABLED_M       FIELD_MASK(1,0)
#define SYSPORT_TDMA_STATUS_LL_RAM_INIT_BUSY_M    FIELD_MASK(1,1)

#define SYSTEMPORT_TDMA_DDR_DESC_RING_PUSH_TIMER_TIMEOUT_M  FIELD_MASK(16,0)

#define SYSTEMPORT_TDMA_TIER1_ARBITER_CTRL_CREDIT_M             FIELD_MASK(16,4)
#define SYSTEMPORT_TDMA_TIER1_ARBITER_CTRL_CREDIT_S             4
#define SYSTEMPORT_TDMA_TIER1_ARBITER_CTRL_IGNORE_CONGESTION_M  FIELD_MASK(1,2)
#define SYSTEMPORT_TDMA_TIER1_ARBITER_CTRL_IGNORE_CONGESTION_S  2
#define SYSTEMPORT_TDMA_TIER1_ARBITER_CTRL_ARB_MODE_M           FIELD_MASK(2,0)
#define SYSTEMPORT_TDMA_TIER1_ARBITER_CTRL_ARB_MODE_S           0

#define SYSPORT_TDMA_CSB_INTR_CFG_THRESHOLD_M     FIELD_MASK(16,0)
#define SYSPORT_TDMA_CSB_INTR_CFG_THRESHOLD_S     0
#define SYSPORT_TDMA_CSB_INTR_CFG_TIMEOUT_M       FIELD_MASK(16,16)
#define SYSPORT_TDMA_CSB_INTR_CFG_TIMEOUT_S       16

typedef struct sys_port_tdma {
	systemport_tdma_descriptor_write_port SYSTEMPORT_TDMA_DESCRIPTOR_WRITE_PORT[32];
	systemport_tdma_descriptor_read_port  SYSTEMPORT_TDMA_DESCRIPTOR_READ_PORT[32];
#define SYSTEMPORT_TDMA_DESC_RING_CONTROL_RING_EN 0x02
	uint32 SYSTEMPORT_TDMA_DESC_RING_CONTROL[32];
	uint32 SYSTEMPORT_TDMA_DESC_RING_HEAD_TAIL_PTR[32];
	uint32 SYSTEMPORT_TDMA_DESC_RING_COUNT[32];
	uint32 SYSTEMPORT_TDMA_DESC_RING_MAX_THRESHOLD[32];
	uint32 SYSTEMPORT_TDMA_DESC_RING_INTR_CONTROL[32];
	uint32 SYSTEMPORT_TDMA_DESC_RING_PRODUCER_CONSUMER_INDEX[32];
	uint32 SYSTEMPORT_TDMA_DESC_RING_MAPPING[32];
	uint32 SYSTEMPORT_TDMA_DESC_RING_PCP_DEI_VID[32];
	uint32 SYSTEMPORT_TDMA_DDR_DESC_RING_START[64];
	uint32 SYSTEMPORT_TDMA_DDR_DESC_RING_SIZE[32];
	uint32 SYSTEMPORT_TDMA_DDR_DESC_RING_CTRL[32];
	uint32 SYSTEMPORT_TDMA_DDR_DESC_RING_PUSH_TIMER[32];
	uint32 SYSTEMPORT_TDMA_DESC_READ_CMD;
	uint32 SYSTEMPORT_TDMA_CONTROL;
	uint32 SYSTEMPORT_TDMA_STATUS;
	uint32 SYSTEMPORT_TDMA_SYSBUS_BURST;
	uint32 SYSTEMPORT_TDMA_TIER1_ARBITER_CTRL[8];
	uint32 SYSTEMPORT_TDMA_TIER1_ARBITER_QUEUE_ENABLE[8];
	uint32 SYSTEMPORT_TDMA_TIER2_ARBITER_CTRL;
	uint32 SYSTEMPORT_TDMA_OVER_MAX_THRESHOLD_STATUS;
	uint32 SYSTEMPORT_TDMA_OVER_HYST_THRESHOLD_STATUS;
	uint32 SYSTEMPORT_TDMA_TPID;
	uint32 SYSTEMPORT_TDMA_FREE_LIST_HEAD_TAIL_PTR;
	uint32 SYSTEMPORT_TDMA_FREE_LIST_COUNT;
	uint32 SYSTEMPORT_TDMA_CSB_INTR_CFG;
	uint32 SYSTEMPORT_TDMA_CSB_ADDR_LO;
	uint32 SYSTEMPORT_TDMA_CSB_ADDR_HI;
	uint32 SYSTEMPORT_TDMA_TEST;
	uint32 SYSTEMPORT_TDMA_DEBUG;
}sys_port_tdma, SYSTEMPORT_TDMA;

#define SYSPORT_SPE_SPE_CNTRL_SPE_EN_M                      FIELD_MASK(1,0)

#define SYSTEMPORT_SPE_SPE_CACHE_CMD_COMMAND_LIST_INDEX_M   FIELD_MASK(16,0)
#define SYSTEMPORT_SPE_SPE_CACHE_CMD_CMD_M                  FIELD_MASK(2,16)
#define SYSTEMPORT_SPE_SPE_CACHE_CMD_CMD_S                  16
#define SYSTEMPORT_SPE_SPE_CACHE_CMD_START_BUSY_M           FIELD_MASK(1,18)

#define SYSTEMPORT_SPE_SPE_CACHE_CMD_CMD_INVALIDATE         0
#define SYSTEMPORT_SPE_SPE_CACHE_CMD_CMD_SET_STATIC         1
#define SYSTEMPORT_SPE_SPE_CACHE_CMD_CMD_CLEAR_STATIC       2

#define SYSTEMPORT_SPE_SPE_CACHE_CMD_COMMAND_LIST_INDEX_INVALID  0xFFFF

typedef struct sys_port_spe
{
    uint32 SYSTEMPORT_SPE_SPE_CNTRL;
    uint32 SYSTEMPORT_SPE_SPE_LFU_AW_MIN_PKT_COUNT;
    uint32 SYSTEMPORT_SPE_SPE_LFU_AW_CNTRL_PERIOD;
    uint8  dummy_1[64];
    uint32 SYSTEMPORT_SPE_SPE_CACHE_ENTRY_VALID_STATUS;
    uint32 SYSTEMPORT_SPE_SPE_CACHE_CMD;
    uint32 SYSTEMPORT_SPE_SPE_DDR_CMD_LIST_ARRAY_START_ADDR_LOW;
    uint32 SYSTEMPORT_SPE_SPE_DDR_CMD_LIST_ARRAY_START_ADDR_HIGH;
    uint32 SYSTEMPORT_SPE_SPE_SOP_POINTER;
    uint32 SYSTEMPORT_SPE_SPE_CACHE_MISS_COUNTER;
    uint32 SYSTEMPORT_SPE_SPE_CACHE_HIT_COUNTER;
    uint8  dummy_2[152];
    uint32 SYSTEMPORT_SPE_SPE_CACHE_ENTRY_KEY[32];
    uint8  dummy_3[128];
    uint32 SYSTEMPORT_SPE_SPE_REFERENCE_COUNT[32];
}sys_port_spe,SYSTEMPORT_SPE;


typedef struct sys_port_gib {
#define SYSPORT_GIB_CONTROL_RX_EN 0x02
#define SYSPORT_GIB_CONTROL_TX_EN 0x01
#define SYSPORT_GIB_CONTROL_RX_FLUSH 0x08
#define SYSPORT_GIB_CONTROL_TX_FLUSH 0x04
	uint32 SYSTEMPORT_GIB_CONTROL;
	uint32 SYSTEMPORT_GIB_STATUS;
	uint32 SYSTEMPORT_GIB_MAC_DA_1;
	uint32 SYSTEMPORT_GIB_MAC_DA_0;
	uint32 SYSTEMPORT_GIB_RCVD_PKT_ERR_CNT;
	uint32 SYSTEMPORT_GIB_PKT_SIZE_CONTROL;
} sys_port_gib, SYSTEMPORT_GIB;

#define SYSPORT_GIB_CONTROL_TX_EN_M          FIELD_MASK(1,0)
#define SYSPORT_GIB_CONTROL_RX_EN_M          FIELD_MASK(1,1)
#define SYSPORT_GIB_CONTROL_IPG_LENGTH_M     FIELD_MASK(6,16)
#define SYSPORT_GIB_CONTROL_IPG_LENGTH_S     16
#define SYSPORT_GIB_CONTROL_PAD_EXTENSION_M  FIELD_MASK(6,22)
#define SYSPORT_GIB_CONTROL_PAD_EXTENSION_S  22

#define SYSPORT_GIB_PKT_SIZE_CONTROL_MIN_PKT_SIZE_M       FIELD_MASK(8,0)
#define SYSPORT_GIB_PKT_SIZE_CONTROL_MIN_PKT_SIZE_S       0
#define SYSPORT_GIB_PKT_SIZE_CONTROL_MAX_PKT_SIZE_M       FIELD_MASK(16,8)
#define SYSPORT_GIB_PKT_SIZE_CONTROL_MAX_PKT_SIZE_S       8

#define SYSPORT_TOPCTRL_MISC_CNTL_PM_CLK_DIV_M            FIELD_MASK(8,8)
#define SYSPORT_TOPCTRL_MISC_CNTL_PM_CLK_DIV_S            8

#define SYSPORT_TOPCTRL_PM_CNTL_EEE_EN_M                  FIELD_MASK(1,0)
#define SYSPORT_TOPCTRL_PM_CNTL_TX_PM_EN_M                FIELD_MASK(1,1)
#define SYSPORT_TOPCTRL_PM_CNTL_RX_PM_EN_M                FIELD_MASK(1,2)

#define SYSPORT_TOPCTRL_MISC_SWAP_CFG_RSB_SWAP_M          FIELD_MASK(2,0)
#define SYSPORT_TOPCTRL_MISC_SWAP_CFG_RSB_SWAP_S          0
#define SYSPORT_TOPCTRL_MISC_SWAP_CFG_TSB_SWAP_M          FIELD_MASK(2,2)
#define SYSPORT_TOPCTRL_MISC_SWAP_CFG_TSB_SWAP_S          2
#define SYSPORT_TOPCTRL_MISC_SWAP_CFG_RX_DDR_DESC_SWAP_M  FIELD_MASK(2,4)
#define SYSPORT_TOPCTRL_MISC_SWAP_CFG_RX_DDR_DESC_SWAP_S  4
#define SYSPORT_TOPCTRL_MISC_SWAP_CFG_TX_DDR_DESC_SWAP_M  FIELD_MASK(2,6)
#define SYSPORT_TOPCTRL_MISC_SWAP_CFG_TX_DDR_DESC_SWAP_S  6
#define SYSPORT_TOPCTRL_MISC_SWAP_CFG_PSB_SWAP_M          FIELD_MASK(2,8)
#define SYSPORT_TOPCTRL_MISC_SWAP_CFG_PSB_SWAP_S          8
#define SYSPORT_TOPCTRL_MISC_SWAP_CFG_CSB_SWAP_M          FIELD_MASK(2,10)
#define SYSPORT_TOPCTRL_MISC_SWAP_CFG_CSB_SWAP_S          10
#define SYSPORT_TOPCTRL_MISC_SWAP_CFG_SPE_SWAP_M          FIELD_MASK(2,12)
#define SYSPORT_TOPCTRL_MISC_SWAP_CFG_SPE_SWAP_S          12

#define SYSPORT_TOPCTRL_MISC_SWAP_CFG_SWAP_NONE    0
#define SYSPORT_TOPCTRL_MISC_SWAP_CFG_SWAP_32      1
#define SYSPORT_TOPCTRL_MISC_SWAP_CFG_SWAP_64      2
#define SYSPORT_TOPCTRL_MISC_SWAP_CFG_SWAP_32_64   3

#define SYSTEMPORT_CLOCK_FREQUENCY  300 // MHz

typedef struct sys_port_topctrl {
	uint32 SYSTEMPORT_TOPCTRL_REV_CNTL;
	uint32 SYSTEMPORT_TOPCTRL_RX_FLUSH_CNTL;
	uint32 SYSTEMPORT_TOPCTRL_TX_FLUSH_CNTL;
	uint32 SYSTEMPORT_TOPCTRL_MISC_CNTL;
	uint32 SYSTEMPORT_TOPCTRL_PM_CNTL;
	uint32 SYSTEMPORT_TOPCTRL_PM_STATUS;
	uint32 SYSTEMPORT_TOPCTRL_TIMER_TICK_CNTL;
	uint32 SYSTEMPORT_TOPCTRL_WR_REPLY_CFG;
	uint32 SYSTEMPORT_TOPCTRL_MISC_SWAP_CFG;
	uint32 SYSTEMPORT_TOPCTRL_UMAC_RXERR_MASK;
	uint32 SYSTEMPORT_TOPCTRL_MIB_MAX_PKT_SIZE;
	uint32 SYSTEMPORT_TOPCTRL_TIMER0_TIMEOUT_PERIOD;
	uint32 SYSTEMPORT_TOPCTRL_TIMER1_TIMEOUT_PERIOD;
}sys_port_topctrl, SYSTEMPORT_TOPCTRL;

typedef struct sys_port_sysbuscfg {
        uint32 SYSTEMPORT_SYSBUSCFG_TIER1_TX_ARB_CFG;
        uint32 SYSTEMPORT_SYSBUSCFG_TIER1_RX_ARB_CFG;
        uint32 SYSTEMPORT_SYSBUSCFG_TIER2_ARB_CFG;
        uint32 SYSTEMPORT_SYSBUSCFG_ARB_CONTROL;
        uint32 SYSTEMPORT_SYSBUSCFG_U2R_BRIDGE_CONTROL;
	uint32 SYSTEMPORT_SYSBUSCFG_DIAG;
}sys_port_sysbuscfg, SYSTEMPORT_SYSBUSCFG;

typedef struct sys_port_rxchk {
    uint32 SYSTEMPORT_RXCHK_CONTROL;//0x80490300
    uint32 SYSTEMPORT_RXCHK_BRCM_TAG[8];//0x80490304 - 0x80490320
    uint32 SYSTEMPORT_RXCHK_BRCM_TAG_MASK[8];//0x80490324 - 0x80490340
    uint32 SYSTEMPORT_RXCHK_BRCM_TAG_MATCH_STATUS;//0x80490344
    uint32 SYSTEMPORT_RXCHK_ETHERTYPE;//0x80490348
    uint32 SYSTEMPORT_RXCHK_WAN_PORT_AND_HOST_CPU_MAPPING;//0x8049034c
    uint32 SYSTEMPORT_RXCHK_MLT_CONFIG;//0x80490350
    uint32 SYSTEMPORT_RXCHK_RX_QUEUE_MAPPING;//0x80490354
    uint32 SYSTEMPORT_RXCHK_MLT_CMDDATA1;//0x80490358
    uint32 SYSTEMPORT_RXCHK_MLT_DATA0;//0x8049035c
    uint32 SYSTEMPORT_RXCHK_BAD_CHECKSUM_PACKET_DISCARD_COUNTER;//0x80490360
    uint32 SYSTEMPORT_RXCHK_OTHER_PACKET_DISCARD_COUNTER;//0x80490364
    uint32 SYSTEMPORT_RXCHK_RSB32BX_CFG1;//0x80490368
    uint32 SYSTEMPORT_RXCHK_RSB32BX_CFG2;//0x8049036c
}sys_port_rxchk, SYSTEMPORT_RXCHK;

#define SYSPORT_RXCHK_CONTROL_RCHK_EN_M                   FIELD_MASK(1,0)
#define SYSPORT_RXCHK_CONTROL_SKIP_FCS_M                  FIELD_MASK(1,1)
#define SYSPORT_RXCHK_CONTROL_BAD_CHKSUM_DISCARD_EN_M     FIELD_MASK(1,2)
#define SYSPORT_RXCHK_CONTROL_BRCM_TAG_EN_M               FIELD_MASK(1,3)
#define SYSPORT_RXCHK_CONTROL_BRCM_TAG_MATCH_EN_M         FIELD_MASK(8,4)
#define SYSPORT_RXCHK_CONTROL_PARSE_TUNNELED_IP_M         FIELD_MASK(1,12)
#define SYSPORT_RXCHK_CONTROL_STRUCT_VIOL_EN_M            FIELD_MASK(1,13)
#define SYSPORT_RXCHK_CONTROL_STRUCT_VIOL_ACT_M           FIELD_MASK(1,14)
#define SYSPORT_RXCHK_CONTROL_INCOMPLETE_PACKET_ACT_M     FIELD_MASK(1,15)
#define SYSPORT_RXCHK_CONTROL_IPV6_DUP_EXTHDR_EN_M        FIELD_MASK(1,16)
#define SYSPORT_RXCHK_CONTROL_IPV6_DUP_EXTHDR_ACT_M       FIELD_MASK(1,17)
#define SYSPORT_RXCHK_CONTROL_UNEXPECTED_ETHTYPE_ACT_M    FIELD_MASK(1,18)
#define SYSPORT_RXCHK_CONTROL_LLCSNAP_HDR_ERR_ACT_M       FIELD_MASK(1,19)
#define SYSPORT_RXCHK_CONTROL_PPPOE_HDR_ERR_ACT_M         FIELD_MASK(1,20)
#define SYSPORT_RXCHK_CONTROL_L3_HDR_ERR_ACT_M            FIELD_MASK(1,21)
#define SYSPORT_RXCHK_CONTROL_MAC_RX_ERR_ACT_M            FIELD_MASK(1,22)
#define SYSPORT_RXCHK_CONTROL_PARSE_AUTH_M                FIELD_MASK(1,23)
#define SYSPORT_RXCHK_CONTROL_USE_IPV4DATALEN_M           FIELD_MASK(1,24)
#define SYSPORT_RXCHK_CONTROL_USE_IPV6DATALEN_M           FIELD_MASK(1,25)
#define SYSPORT_RXCHK_CONTROL_ARP_INTR_EN_M               FIELD_MASK(1,26)
#define SYSPORT_RXCHK_CONTROL_MLT_EN_M                    FIELD_MASK(1,27)

#define SYSPORT_RXCHK_MLT_CONFIG_DEFAULT_SRC_PORT_M       FIELD_MASK(3,0)
#define SYSPORT_RXCHK_MLT_CONFIG_DEFAULT_SRC_PORT_S       0
#define SYSPORT_RXCHK_MLT_CONFIG_SRC_PORT_IS_WAN_M        FIELD_MASK(1,3)
#define SYSPORT_RXCHK_MLT_CONFIG_LAN_DLF_RXQ_M            FIELD_MASK(3,4)
#define SYSPORT_RXCHK_MLT_CONFIG_LAN_DLF_RXQ_S            4
#define SYSPORT_RXCHK_MLT_CONFIG_WAN_DLF_RXQ_M            FIELD_MASK(3,8)
#define SYSPORT_RXCHK_MLT_CONFIG_WAN_DLF_RXQ_S            8

#define SYSPORT_RXCHK_MLT_CMDDATA1_DATA1_M                FIELD_MASK(21,0)
#define SYSPORT_RXCHK_MLT_CMDDATA1_DATA1_S                0
#define SYSPORT_RXCHK_MLT_CMDDATA1_MATCH_STATUS_M         FIELD_MASK(1,21)
#define SYSPORT_RXCHK_MLT_CMDDATA1_CMD_M                  FIELD_MASK(4,24)
#define SYSPORT_RXCHK_MLT_CMDDATA1_CMD_S                  24
#define SYSPORT_RXCHK_MLT_CMDDATA1_START_BUSY_M           FIELD_MASK(1,28)

#define SYSPORT_RXCHK_MLT_CMDDATA1_CMD_WR                 0
#define SYSPORT_RXCHK_MLT_CMDDATA1_CMD_CAMWR              1
#define SYSPORT_RXCHK_MLT_CMDDATA1_CMD_RDIDX              2
#define SYSPORT_RXCHK_MLT_CMDDATA1_CMD_SEARCH             3
#define SYSPORT_RXCHK_MLT_CMDDATA1_CMD_CLR                4
#define SYSPORT_RXCHK_MLT_CMDDATA1_CMD_CLRIDX             5
#define SYSPORT_RXCHK_MLT_CMDDATA1_CMD_INIT_HASHTAB       6
#define SYSPORT_RXCHK_MLT_CMDDATA1_CMD_INIT_CAM           7
#define SYSPORT_RXCHK_MLT_CMDDATA1_CMD_INIT_ALL           8

#define SYSPORT_RXCHK_RSB32BX_CFG1_MAC_SA_EN_M            FIELD_MASK(1,0)
#define SYSPORT_RXCHK_RSB32BX_CFG1_MAX_VLAN_M             FIELD_MASK(3,1)
#define SYSPORT_RXCHK_RSB32BX_CFG1_MAX_VLAN_S             1
#define SYSPORT_RXCHK_RSB32BX_CFG1_L3_FORCE_M             FIELD_MASK(1,4)
#define SYSPORT_RXCHK_RSB32BX_CFG1_L3U_TOS_EN_M           FIELD_MASK(1,17)

#define SYSPORT_RXCHK_WAN_PORT_AND_HOST_CPU_MAPPING_WAN0_PORT_ID_M            FIELD_MASK(4,0)
#define SYSPORT_RXCHK_WAN_PORT_AND_HOST_CPU_MAPPING_WAN0_PORT_ID_S            0
#define SYSPORT_RXCHK_WAN_PORT_AND_HOST_CPU_MAPPING_WAN0_PORT_ENABLE_M        FIELD_MASK(1,4)
#define SYSPORT_RXCHK_WAN_PORT_AND_HOST_CPU_MAPPING_WAN0_PORT_MAP_ENABLE_M    FIELD_MASK(1,5)
#define SYSPORT_RXCHK_WAN_PORT_AND_HOST_CPU_MAPPING_WAN1_PORT_ID_M            FIELD_MASK(4,8)
#define SYSPORT_RXCHK_WAN_PORT_AND_HOST_CPU_MAPPING_WAN1_PORT_ID_S            8
#define SYSPORT_RXCHK_WAN_PORT_AND_HOST_CPU_MAPPING_WAN1_PORT_ENABLE_M        FIELD_MASK(1,12)
#define SYSPORT_RXCHK_WAN_PORT_AND_HOST_CPU_MAPPING_WAN1_PORT_MAP_ENABLE_M    FIELD_MASK(1,13)
#define SYSPORT_RXCHK_WAN_PORT_AND_HOST_CPU_MAPPING_HOST_CPU_TC_M             FIELD_MASK(3,16)

#define SYSPORT_TPC     ((volatile sys_port_topctrl * const) SYSPORT_BASE)
#define SYSPORT_SYSBUSCFG     ((volatile sys_port_sysbuscfg * const) SYSPORT_SYSBUSCFG_BASE)
#define SYSPORT_RXCHK   ((volatile sys_port_rxchk   * const) SYSPORT_RXCHK_BASE)
#define SYSPORT_RBUF    ((volatile sys_port_rbuf    * const) SYSPORT_RBUF_BASE)
#define SYSPORT_TBUF    ((volatile sys_port_tbuf    * const) SYSPORT_TBUF_BASE)
#define SYSPORT_MPD     ((volatile sys_port_mpd     * const) SYSPORT_MPD_BASE)
#define SYSPORT_RDMA    ((volatile sys_port_rdma    * const) SYSPORT_RDMA_BASE)
#define SYSPORT_TDMA    ((volatile sys_port_tdma    * const) SYSPORT_TDMA_BASE)
#define SYSPORT_SPE     ((volatile sys_port_spe     * const) SYSPORT_SPE_BASE)
#define SYSPORT_GIB     ((volatile sys_port_gib * const) (SYSPORT_GIB_BASE))
#define SYSPORT_INTRL2  ((volatile sys_port_intr2   * const) (SYSPORT_INTRL2_BASE))
#define SYSPORT_INTC    ((volatile sys_port_intc    * const) (SYSPORT_INTC_BASE))
#define SYSPORT_INT_MISC_RX ((volatile sys_port_intr2    * const) (SYSPORT_INT_MISC_RX_BASE))
#define SYSPORT_INT_MISC_TX ((volatile sys_port_intr2    * const) (SYSPORT_INT_MISC_TX_BASE))



typedef struct {
    uint32 led_ctrl;
#define SPDLNK_LED1_ACT_POL_SEL_SHIFT 7
#define SPDLNK_LED1_ACT_POL_SEL_MASK (0x1 << SPDLNK_LED1_ACT_POL_SEL_SHIFT)
#define ACT_LED_ACT_SEL_SHIFT 5
#define ACT_LED_ACT_SEL_MASK (0x1 << ACT_LED_ACT_SEL_SHIFT)
#define SPDLNK_LED0_ACT_SEL_SHIFT 2
#define SPDLNK_LED0_ACT_SEL_MASK (0x1 << SPDLNK_LED0_ACT_SEL_SHIFT)
#define RESERVED0_SHIFT 16
#define RESERVED0_MASK (0xffff << RESERVED0_SHIFT)
#define SPDLNK_LED0_ACT_POL_SEL_SHIFT 6
#define SPDLNK_LED0_ACT_POL_SEL_MASK (0x1 << SPDLNK_LED0_ACT_POL_SEL_SHIFT)
#define AGGREGATE_LED_CNTRL_RESERVED0_SHIFT 19
#define AGGREGATE_LED_CNTRL_RESERVED0_MASK (0x1fff << AGGREGATE_LED_CNTRL_RESERVED0_SHIFT)
#define AGGREGATE_LED_CNTRL_ACT_POL_SEL_SHIFT 17
#define AGGREGATE_LED_CNTRL_ACT_POL_SEL_MASK (0x1 << AGGREGATE_LED_CNTRL_ACT_POL_SEL_SHIFT)
#define SPDLNK_LED2_ACT_SEL_SHIFT 4
#define SPDLNK_LED2_ACT_SEL_MASK (0x1 << SPDLNK_LED2_ACT_SEL_SHIFT)
#define LED_SPD_OVRD_SHIFT 10
#define LED_SPD_OVRD_MASK (0x7 << LED_SPD_OVRD_SHIFT)
#define SPDLNK_LED2_ACT_POL_SEL_SHIFT 8
#define SPDLNK_LED2_ACT_POL_SEL_MASK (0x1 << SPDLNK_LED2_ACT_POL_SEL_SHIFT)
#define SPD_OVRD_EN_SHIFT 14
#define SPD_OVRD_EN_MASK (0x1 << SPD_OVRD_EN_SHIFT)
#define TX_ACT_EN_SHIFT 1
#define TX_ACT_EN_MASK (0x1 << TX_ACT_EN_SHIFT)
#define AGGREGATE_LED_CNTRL_ACT_SEL_SHIFT 16
#define AGGREGATE_LED_CNTRL_ACT_SEL_MASK (0x1 << AGGREGATE_LED_CNTRL_ACT_SEL_SHIFT)
#define LNK_STATUS_OVRD_SHIFT 13
#define LNK_STATUS_OVRD_MASK (0x1 << LNK_STATUS_OVRD_SHIFT)
#define RX_ACT_EN_SHIFT 0
#define RX_ACT_EN_MASK (0x1 << RX_ACT_EN_SHIFT)
#define AGGREGATE_LED_CNTRL_LNK_POL_SEL_SHIFT 18
#define AGGREGATE_LED_CNTRL_LNK_POL_SEL_MASK (0x1 << AGGREGATE_LED_CNTRL_LNK_POL_SEL_SHIFT)
#define SPDLNK_LED1_ACT_SEL_SHIFT 3
#define SPDLNK_LED1_ACT_SEL_MASK (0x1 << SPDLNK_LED1_ACT_SEL_SHIFT)
#define ACT_LED_POL_SEL_SHIFT 9
#define ACT_LED_POL_SEL_MASK (0x1 << ACT_LED_POL_SEL_SHIFT)
#define LNK_OVRD_EN_SHIFT 15
#define LNK_OVRD_EN_MASK (0x1 << LNK_OVRD_EN_SHIFT)
#define AGGREGATE_LED_CNTRL_PORT_EN_SHIFT 0
#define AGGREGATE_LED_CNTRL_PORT_EN_MASK (0xffff << AGGREGATE_LED_CNTRL_PORT_EN_SHIFT)
    uint32 led_encoding_sel;
    uint32 led_encoding;
#define LINK_AND_SPEED_ENCODING_M10G_ENCODE_SHIFT 15
#define LINK_AND_SPEED_ENCODING_M10G_ENCODE_MASK (0x7 << LINK_AND_SPEED_ENCODING_M10G_ENCODE_SHIFT)
#define LINK_AND_SPEED_ENCODING_SEL_SEL_10M_ENCODE_SHIFT 3
#define LINK_AND_SPEED_ENCODING_SEL_SEL_10M_ENCODE_MASK (0x7 << LINK_AND_SPEED_ENCODING_SEL_SEL_10M_ENCODE_SHIFT)
#define LINK_AND_SPEED_ENCODING_SEL_SEL_100M_ENCODE_SHIFT 6
#define LINK_AND_SPEED_ENCODING_SEL_SEL_100M_ENCODE_MASK (0x7 << LINK_AND_SPEED_ENCODING_SEL_SEL_100M_ENCODE_SHIFT)
#define LINK_AND_SPEED_ENCODING_SEL_SEL_10G_ENCODE_SHIFT 15
#define LINK_AND_SPEED_ENCODING_SEL_SEL_10G_ENCODE_MASK (0x7 << LINK_AND_SPEED_ENCODING_SEL_SEL_10G_ENCODE_SHIFT)
#define LINK_AND_SPEED_ENCODING_M10_ENCODE_SHIFT 3
#define LINK_AND_SPEED_ENCODING_M10_ENCODE_MASK (0x7 << LINK_AND_SPEED_ENCODING_M10_ENCODE_SHIFT)
#define LINK_AND_SPEED_ENCODING_M1000_ENCODE_SHIFT 9
#define LINK_AND_SPEED_ENCODING_M1000_ENCODE_MASK (0x7 << LINK_AND_SPEED_ENCODING_M1000_ENCODE_SHIFT)
#define LINK_AND_SPEED_ENCODING_SEL_SEL_2500M_ENCODE_SHIFT 12
#define LINK_AND_SPEED_ENCODING_SEL_SEL_2500M_ENCODE_MASK (0x7 << LINK_AND_SPEED_ENCODING_SEL_SEL_2500M_ENCODE_SHIFT)
#define LINK_AND_SPEED_ENCODING_M100_ENCODE_SHIFT 6
#define LINK_AND_SPEED_ENCODING_M100_ENCODE_MASK (0x7 << LINK_AND_SPEED_ENCODING_M100_ENCODE_SHIFT)
#define LINK_AND_SPEED_ENCODING_SEL_SEL_1000M_ENCODE_SHIFT 9
#define LINK_AND_SPEED_ENCODING_SEL_SEL_1000M_ENCODE_MASK (0x7 << LINK_AND_SPEED_ENCODING_SEL_SEL_1000M_ENCODE_SHIFT)
#define LINK_AND_SPEED_ENCODING_M2500_ENCODE_SHIFT 12
#define LINK_AND_SPEED_ENCODING_M2500_ENCODE_MASK (0x7 << LINK_AND_SPEED_ENCODING_M2500_ENCODE_SHIFT)

}LED_CFG;

typedef struct EthernetSwitchReg
{
    uint32 switch_ctrl;                      /* 0x0000 */
#define ETHSW_SWITCH_CTRL_SWITCH_CLK_SEL_SHIFT  27
#define ETHSW_SWITCH_CTRL_SWITCH_CLK_SEL_MASK   (0x3<<ETHSW_SWITCH_CTRL_SWITCH_CLK_SEL_SHIFT)
#define ETHSW_SWITCH_CTRL_SYSPORT_CLK_SEL_SHIFT 24
#define ETHSW_SWITCH_CTRL_SYSPORT_CLK_SEL_MASK  (0x3<<ETHSW_SWITCH_CTRL_SYSPORT_CLK_SEL_SHIFT)
#define ETHSW_SWITCH_CTRL_P8_CLK_SEL_SHIFT      3
#define ETHSW_SWITCH_CTRL_P8_CLK_SEL_MASK       (0x3<<ETHSW_SWITCH_CTRL_P8_CLK_SEL_SHIFT)
    uint32 switch_status;                    /* 0x0004 */
    uint32 dir_data_write_reg;               /* 0x0008 */
    uint32 dir_data_read_reg;                /* 0x000c */
    uint32 switch_rev;                       /* 0x0010 */
    uint32 phy_rev;                          /* 0x0014 */
    uint32 phy_test_ctrl;                    /* 0x0018 */
    uint32 qphy_ctrl;                        /* 0x001c */
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
#define ETHSW_QPHY_CTRL_IDDQ_GLOBAL_PWR_SHIFT      6
#define ETHSW_QPHY_CTRL_IDDQ_GLOBAL_PWR_MASK       (0x1<<ETHSW_QPHY_CTRL_IDDQ_GLOBAL_PWR_SHIFT)


    uint32 qphy_status;                      /* 0x0020 */
    uint32 sphy_ctrl;                        /* 0x0024 */
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
#define ETHSW_SPHY_CTRL_IDDQ_GLOBAL_PWR_SHIFT      3
#define ETHSW_SPHY_CTRL_IDDQ_GLOBAL_PWR_MASK       (0x1<<ETHSW_SPHY_CTRL_IDDQ_GLOBAL_PWR_SHIFT)
    uint32 sphy_status;                      /* 0x0028 */
    uint32 switch_phy_intr_ctrl;             /* 0x002c */
    uint32 reserved2[4];                     /* 0x0030 - 0x003f */
    LED_CFG led_ctrl[9];                     /* 0x0040 - 0x00ab */  //63178: port6,7 not populated
    LED_CFG reserved_ac;                     /* 0x00ac - 0x00b7 */  //63178: led_wan_ctrl not populated
#define ETHSW_LED_CTRL_SPD0_ON               0x0
#define ETHSW_LED_CTRL_SPD0_OFF              0x1
#define ETHSW_LED_CTRL_SPD1_ON               0x0
#define ETHSW_LED_CTRL_SPD1_OFF              0x2
#define ETHSW_LED_CTRL_1000M_SHIFT           9
#define ETHSW_LED_CTRL_100M_SHIFT            6
#define ETHSW_LED_CTRL_10M_SHIFT             3
#define ETHSW_LED_CTRL_NOLINK_SHIFT          0
#define ETHSW_LED_CTRL_ALL_SPEED_MASK        0x3ffff
#define ETHSW_LED_CTRL_SPEED_MASK            0x7
    uint32 led_blink_rate_ctrl;              /* 0x00b8 */
    uint32 reserved_bc;                      /* 0x00bc */           //63178: led_serial_ctrl not populated
    uint32 led_refresh_period_ctrl;          /* 0x00c0 */
    uint32 aggregate_led_ctrl;               /* 0x00c4 */
#define ETHSW_AGGREGATE_LED_CTRL_PORT_EN_MASK            0xffff
#define ETHSW_AGGREGATE_LED_CTRL_ACT_SEL_MASK            0x10000
#define ETHSW_AGGREGATE_LED_CTRL_ACT_POL_SEL_MASK        0x20000
#define ETHSW_AGGREGATE_LED_CTRL_LNK_POL_SEL_MASK        0x40000
    uint32 aggregate_blink_rate_ctrl;        /* 0x00c8 */
#define ETHSW_QGPHY3_LED_OVRD_SHIFT           29 /*0x1c*/
#define ETHSW_QGPHY3_LED_OVRD_MASK            (1 << ETHSW_QGPHY3_LED_OVRD_SHIFT)
    uint32 reserved_cc;                      /* 0x00cc */           //63178: crossbar_switch_ctrl not populated
    uint32 reserved_d0[19];                  /* 0x00d0 - 0x011b */
    uint32 rgmii_5_ctrl;                     /* 0x011c */           //63178: add rgmii_5, remove rgmii_11,12,13
#define ETHSW_RC_MII_MODE_MASK               0x1c
#define ETHSW_RC_EXT_RVMII                   0x10
#define ETHSW_RC_EXT_GPHY                    0x0c
#define ETHSW_RC_EXT_EPHY                    0x08
#define ETHSW_RC_INT_GPHY                    0x04
#define ETHSW_RC_INT_EPHY                    0x00
#define ETHSW_RC_ID_MODE_DIS                 0x02
#define ETHSW_RC_RGMII_EN                    0x01
    uint32 rgmii_5_ib_status;                /* 0x0120 */
    uint32 rgmii_5_rx_clk_delay_ctrl;        /* 0x0124 */
#define ETHSW_RXCLK_IDDQ                     (1<<4)
#define ETHSW_RXCLK_BYPASS                   (1<<5)
    uint32 reserved_128[202];                 /* 0x0128 - 0x044f */
    uint32 moca_bp_mapping_port5;            /* 0x0450*/
} EthernetSwitchReg;

#define ETHSW_REG ((volatile EthernetSwitchReg * const) SWITCH_REG_BASE)

#define PHY_TEST_CTRL                                ((volatile unsigned int*)(&ETHSW_REG->phy_test_ctrl))
#define SPHY_CNTRL                                   ((volatile unsigned int*)(&ETHSW_REG->sphy_ctrl))
#define QPHY_CNTRL                                   ((volatile unsigned int*)(&ETHSW_REG->qphy_ctrl))


struct sar_rx_vcam
{
    uint32 SAR_RX_VCAM_RX_CAM;
    uint32 SAR_RX_VCAM_RX_RAM;
};

typedef struct SarControllerRegisters
{
    uint32 SAR_TXDMA_CTRL_TX_CH_INDX[16];//0x80800000 -- 0x8080003c
    uint32 dummy[4];
    uint32 SAR_IRQ_STAT; //0x80800050
    uint32 SAR_IRQ_MSK; //0x80800054
    uint32 SOFT_RST_SOFT_RST;//0x80800058
    uint32 SAR_TXPBUF_PAD_SAR_TXPBUF_PAD;//0x8080005c
    uint32 SAR_TX_CTL_TX_SAR_CFG;//0x80800060
    uint32 SAR_TX_CTL_TX_SCH_CFG;//0x80800064
    uint32 SAR_TX_CTL_TX_OAM_CFG;//0x80800068
    uint32 SAR_TX_CTL_TX_STATUS1;//0x8080006c
    uint32 SAR_TX_CTL_TX_STATUS2;//0x80800070
    uint32 SAR_TX_CTL_TX_MPAAL_CFG;//0x80800074
    uint32 SAR_TX_CTL_TX_UTO_CFG;//0x80800078
    uint32 SAR_TX_CTL_TX_LRATE_TIMER;//0x8080007c
    uint32 SAR_RX_CTL_P0_RXATM_CFG;//0x80800080
    uint32 SAR_RX_CTL_P1_RXATM_CFG;//0x80800084
    uint32 SAR_RX_CTL_P2_RXATM_CFG;//0x80800088
    uint32 SAR_RX_CTL_P3_RXATM_CFG;//0x8080008c
    uint32 SAR_RX_CTL_RX_SAR_CFG;//0x80800090
    uint32 SAR_RX_CTL_RX_OAM_CFG;//0x80800094
    uint32 SAR_RX_CTL_RXATM_STAT;//0x80800098
    uint32 SAR_RX_CTL_RX_UTO_CFG;//0x8080009c
    uint32 SAR_RX_CTL_RXAAL_CFG;//0x808000a0
    uint32 SAR_RX_CTL_RXAAL_MAX_SDU;//0x808000a4
    uint32 SAR_RX_CTL_RXAAL_STAT;//0x808000a8
    uint32 SAR_RX_CTL_LED_CFG;//0x808000ac
    uint32 SAR_RX_CTL_RXAAL_CLEN_CRC_ERR_CNT;//0x808000b0
    uint32 SAR_RX_CTL_RXAAL_MAXSDU_ERR_CNT;//0x808000b4
    uint32 SAR_RX_CTL_RXATM_ERR_CELL_VCID0;//0x808000b8
    uint32 SAR_RX_CTL_RXATM_ERR_CELL_VCID1;//0x808000bc
    uint32 dummy1[0x10];
    uint32 SAR_TX_VTBL_TX_VPI_VCI[0x10];//0x80800100
    struct sar_rx_vcam vcam_rx[16]; //0x80800140 
    uint32 dummy2[0x10];
    uint32 SAR_SHPR_TMU_MPN_PLVL_ARB_03;//0x80800200
    uint32 SAR_SHPR_TMU_MPN_PLVL_ARB_47;//0x80800204
    uint32 SAR_SHPR_TMU_VC_PLVL_ARB;//0x80800208
    uint32 SAR_SHPR_TMU_LNK0DWNCNT;//0x8080020c
    uint32 SAR_SHPR_TMU_LNK1DWNCNT;//0x80800210
    uint32 SAR_SHPR_TMU_LNK2DWNCNT;//0x80800214
    uint32 SAR_SHPR_TMU_LNK3DWNCNT;//0x80800218
    uint32 SAR_SHPR_TMU_UTCFG;//0x8080021c
    uint32 SAR_SHPR_TMU_REMAPUTCFG;//0x80800220
    uint32 SAR_SHPR_PTMBONDPAIR;//0x80800224
    uint32 SAR_SHPR_TMU_DMODE;//0x80800228
    uint32 SAR_SHPR_TMU_SIT;//0x8080022c
    uint32 SAR_SHPR_TMU_SITLO;//0x80800230
    uint32 SAR_SHPR_TMU_MAPMP01;//0x80800234
    uint32 SAR_SHPR_TMU_MAPMP23;//0x80800238
    uint32 SAR_SHPR_TMU_MAPMP45;//0x8080023c
    uint32 SAR_SHPR_TMU_MAPMP67;//0x80800240
    uint32 SAR_SHPR_MPGTSCFG0;//0x80800244
    uint32 SAR_SHPR_MPGTSCFG1;//0x80800248
    uint32 SAR_SHPR_MPGTSCFG2;//0x8080024c
    uint32 SAR_SHPR_MPGTSCFG3;//0x80800250
    uint32 SAR_SHPR_MPGTSCFG4;//0x80800254
    uint32 SAR_SHPR_MPGTSCFG5;//0x80800258
    uint32 SAR_SHPR_MPGTSCFG6;//0x8080025c
    uint32 SAR_SHPR_MPGTSCFG7;//0x80800260
    uint32 SAR_SHPR_TXDMAEMPTY;//0x80800264
    uint32 SAR_SHPR_PBUFEMPTY;//0x80800268
    uint32 SAR_SHPR_TXUSPTMSYNCSRC;//0x8080026c
    uint32 SAR_SHPR_FORCEVCRR;//0x80800270
    uint32 SAR_SHPR_FORCEMPRR_03;//0x80800274
    uint32 SAR_SHPR_FORCEMPRR_47;//0x80800278
    uint32 SAR_SHPR_ADRMODE;//0x8080027c
    uint32 SAR_SHPR_NEWCFGD0;//0x80800280
    uint32 dummy_2[0x20];
    uint32 SAR_RX_PBUF_CTRL_PACKBUF_CFG;//0x80800300
    uint32 SAR_RX_PBUF_CTRL_PACKBUF_THOLD;//0x80800304
    uint32 SAR_RX_PBUF_CTRL_PACKBUF_VCID;//0x80800308
    uint32 SAR_RX_PBUF_CTRL_PACKBUF_MEM;//0x8080030c
    uint32 SAR_RX_PBUF_CTRL_PACKBUF_DATA_HI;//0x80800310
    uint32 SAR_RX_PBUF_CTRL_PACKBUF_DATA_LO;//0x80800314
    uint32 SAR_RX_PBUF_CTRL_PACKBUF_PTR;//0x80800318
    uint32 SAR_RX_PBUF_CTRL_PACKBUF_SIZE;//0x8080031c
    uint32 SAR_RX_PBUF_CTRL_PACKBUF_FIFO23_START;//0x80800320
    uint32 SAR_RX_PBUF_CTRL_PACKBUF_FIFO01_START;//0x80800324
    uint32 SAR_RX_PBUF_CTRL_PACKBUF_FIFO23_STOP;//0x80800328
    uint32 SAR_RX_PBUF_CTRL_PACKBUF_FIFO01_STOP;//0x8080032c
    uint32 SAR_RX_PBUF_CTRL_PACKBUF_FIFO23_DELAY;//0x80800330
    uint32 SAR_RX_PBUF_CTRL_PACKBUF_FIFO01_DELAY;//0x80800334
    uint32 SAR_RX_PBUF_CTRL_PACKBUF_QFULL;//0x80800338
    uint32 SAR_RX_PBUF_CTRL_PACKBUF_NOBUF;//0x8080033c
    uint32 SAR_RX_PBUF_CTRL_PACKBUF_ERRPKT;//0x80800340
    uint32 SAR_RX_PBUF_CTRL_PACKBUF_ERRFRAG;//0x80800344
    uint32 SAR_RX_PBUF_CTRL_PACKBUF_TR;//0x80800348
    uint32 SAR_RX_PBUF_CTRL_PACKBUF_FPM;//0x8080034c
    uint32 SAR_RX_PBUF_CTRL_PACKBUF_MIB_CTRL;//0x80800350
    uint32 SAR_RX_PBUF_CTRL_PACKBUF_MIB_MATCH;//0x80800354
    uint32 SAR_RX_PBUF_CTRL_PACKBUF_MIB_RXOCTET;//0x80800358
    uint32 SAR_RX_PBUF_CTRL_PACKBUF_MIB_RXPACKET;//0x8080035c
    uint32 dummy_3[0x28];
    uint32 SAR_RX_PBUF_MUX_VCID0_QIDRX;//0x80800400
    uint32 SAR_RX_PBUF_MUX_VCID1_QIDRX;//0x80800404
    uint32 SAR_RX_PBUF_MUX_MATCH0_DEF_IDRX;//0x80800408
    uint32 SAR_RX_PBUF_MUX_MATCH1_DEF_IDRX;//0x8080040c
    uint32 SAR_RX_PBUF_MUX_MATCH2_DEF_IDRX;//0x80800410
    uint32 SAR_RX_PBUF_MUX_MATCH3_DEF_IDRX;//0x80800414
    uint32 SAR_RX_PBUF_MUX_MATCH4_DEF_IDRX;//0x80800418
    uint32 dummy_4[0x39];
    uint32 SAR_TX_HDR_CTRL_HEADER_INS[8]; //0x80800500
    uint32 SAR_TX_HDR_CTRL_TXDP_REG;
    uint32 dummy_5[0x23];
    uint32 SAR_TX_HDR_WORD_HEADER0_W0[8][4];//0x80800580
    uint32 SAR_MIB_CTNR_P0_TX_OCT_PKT;//0x80800600
    uint32 SAR_MIB_CTNR_P1_TX_OCT_PKT;//0x80800604
    uint32 SAR_MIB_CTNR_P2_TX_OCT_PKT;//0x80800608
    uint32 SAR_MIB_CTNR_P3_TX_OCT_PKT;//0x8080060c
    uint32 SAR_MIB_CTNR_P0_RX_OCT_PKT;//0x80800610
    uint32 SAR_MIB_CTNR_P1_RX_OCT_PKT;//0x80800614
    uint32 SAR_MIB_CTNR_P2_RX_OCT_PKT;//0x80800618
    uint32 SAR_MIB_CTNR_P3_RX_OCT_PKT;//0x8080061c
    uint32 SAR_MIB_CTNR_P0_TX_PKT;//0x80800620
    uint32 SAR_MIB_CTNR_P1_TX_PKT;//0x80800624
    uint32 SAR_MIB_CTNR_P2_TX_PKT;//0x80800628
    uint32 SAR_MIB_CTNR_P3_TX_PKT;//0x8080062c
    uint32 SAR_MIB_CTNR_P0_ATM_RX_PKT;//0x80800630
    uint32 SAR_MIB_CTNR_P1_ATM_RX_PKT;//0x80800634
    uint32 SAR_MIB_CTNR_P2_ATM_RX_PKT;//0x80800638
    uint32 SAR_MIB_CTNR_P3_ATM_RX_PKT;//0x8080063c
    uint32 SAR_MIB_CTNR_P0_OAM_RM_CELL;//0x80800640
    uint32 SAR_MIB_CTNR_P1_OAM_RM_CELL;//0x80800644
    uint32 SAR_MIB_CTNR_P2_OAM_RM_CELL;//0x80800648
    uint32 SAR_MIB_CTNR_P3_OAM_RM_CELL;//0x8080064c
    uint32 SAR_MIB_CTNR_P0_ASM_CELL;//0x80800650
    uint32 SAR_MIB_CTNR_P1_ASM_CELL;//0x80800654
    uint32 SAR_MIB_CTNR_P2_ASM_CELL;//0x80800658
    uint32 SAR_MIB_CTNR_P3_ASM_CELL;//0x8080065c
    uint32 SAR_MIB_CTNR_P0_ERR_PKT_CELL;//0x80800660
    uint32 SAR_MIB_CTNR_P1_ERR_PKT_CELL;//0x80800664
    uint32 SAR_MIB_CTNR_P2_ERR_PKT_CELL;//0x80800668
    uint32 SAR_MIB_CTNR_P3_ERR_PKT_CELL;//0x8080066c
    uint8  dummy_mib[12];
    uint32 SAR_MIB_CTNR_MIB_CTRL;//0x8080067c
    uint32 SAR_MIB_CTNR_VC0_TX_OCT_PKT;//0x80800680
    uint32 SAR_MIB_CTNR_VC1_TX_OCT_PKT;//0x80800684
    uint32 SAR_MIB_CTNR_VC2_TX_OCT_PKT;//0x80800688
    uint32 SAR_MIB_CTNR_VC3_TX_OCT_PKT;//0x8080068c
    uint32 SAR_MIB_CTNR_VC4_TX_OCT_PKT;//0x80800690
    uint32 SAR_MIB_CTNR_VC5_TX_OCT_PKT;//0x80800694
    uint32 SAR_MIB_CTNR_VC6_TX_OCT_PKT;//0x80800698
    uint32 SAR_MIB_CTNR_VC7_TX_OCT_PKT;//0x8080069c
    uint32 SAR_MIB_CTNR_VC8_TX_OCT_PKT;//0x808006a0
    uint32 SAR_MIB_CTNR_VC9_TX_OCT_PKT;//0x808006a4
    uint32 SAR_MIB_CTNR_VC10_TX_OCT_PKT;//0x808006a8
    uint32 SAR_MIB_CTNR_VC11_TX_OCT_PKT;//0x808006ac
    uint32 SAR_MIB_CTNR_VC12_TX_OCT_PKT;//0x808006b0
    uint32 SAR_MIB_CTNR_VC13_TX_OCT_PKT;//0x808006b4
    uint32 SAR_MIB_CTNR_VC14_TX_OCT_PKT;//0x808006b8
    uint32 SAR_MIB_CTNR_VC15_TX_OCT_PKT;//0x808006bc
    uint32 SAR_DIAG_Ctrl; //0x80800700
    uint32 SAR_DIAG_Addr[63];//0x80800704
    uint32 SAR_RXPAF_RXPAF_CONFIG;//0x80800800
    uint32 SAR_RXPAF_RXPAF_STATUS;//0x80800804
    uint32 SAR_RXPAF_RXPAF_ERR_STATUS0;//0x80800808
    uint32 SAR_RXPAF_RXPAF_VCID;//0x8080080c
    uint32 SAR_RXPAF_RXPAF_FRAGSIZE;//0x80800810
    uint32 SAR_RXPAF_RXPAF_PKTSIZE;//0x80800814
    uint32 SAR_RXPAF_RXPAF_LINK_STATE;//0x80800818
    uint32 SAR_RXPAF_RXPAF_WR_CHN_FLUSH;//0x8080081c
    uint32 SAR_RXPAF_RXPAF_ERR_STATUS1;//0x80800820
    uint32 SAR_RXPAF_RXPAF_ERR_STATUS2;//0x80800824
    uint32 SAR_RXPAF_RXPAF_TEST_MODE0;//0x80800828
    uint32 SAR_RXPAF_RXPAF_TEST_MODE1;//0x8080082c
    uint32 SAR_RXPAF_RXPAF_ERR_STATUS0_NC;//0x80800830
    uint32 SAR_RXPAF_RXPAF_ERR_STATUS1_NC;//0x80800834
    uint8 SAR_RXPAF_RXPAF_ERR_STATUS2_NC[8];//0x80800838
    uint32 SAR_RXPAF_RXPAF_CELL_CNT0;//0x80800840
    uint32 SAR_RXPAF_RXPAF_CELL_CNT1;//0x80800844
    uint32 SAR_RXPAF_RXPAF_CELL_CNT2;//0x80800848
    uint32 SAR_RXPAF_RXPAF_CELL_CNT3;//0x8080084c
    uint32 SAR_RXPAF_RXPAF_CELL_CNT4;//0x80800850
    uint32 SAR_RXPAF_RXPAF_CELL_CNT5;//0x80800854
    uint32 SAR_RXPAF_RXPAF_CELL_CNT6;//0x80800858
    uint32 SAR_RXPAF_RXPAF_CELL_CNT7;//0x8080085c
    uint32 SAR_RXPAF_RXPAF_DROP_CELL_CNT0;//0x80800860
    uint32 SAR_RXPAF_RXPAF_DROP_CELL_CNT1;//0x80800864
    uint32 SAR_RXPAF_RXPAF_DROP_CELL_CNT2;//0x80800868
    uint32 SAR_RXPAF_RXPAF_DROP_CELL_CNT3;//0x8080086c
    uint32 SAR_RXPAF_RXPAF_DROP_CELL_CNT4;//0x80800870
    uint32 SAR_RXPAF_RXPAF_DROP_CELL_CNT5;//0x80800874
    uint32 SAR_RXPAF_RXPAF_DROP_CELL_CNT6;//0x80800878
    uint32 SAR_RXPAF_RXPAF_DROP_CELL_CNT7;//0x8080087c
    uint32 SAR_RXPAF_RXPAF_FRAG_CNT0;//0x80800880
    uint32 SAR_RXPAF_RXPAF_FRAG_CNT1;//0x80800884
    uint32 SAR_RXPAF_RXPAF_FRAG_CNT2;//0x80800888
    uint32 SAR_RXPAF_RXPAF_FRAG_CNT3;//0x8080088c
    uint32 SAR_RXPAF_RXPAF_FRAG_CNT4;//0x80800890
    uint32 SAR_RXPAF_RXPAF_FRAG_CNT5;//0x80800894
    uint32 SAR_RXPAF_RXPAF_FRAG_CNT6;//0x80800898
    uint32 SAR_RXPAF_RXPAF_FRAG_CNT7;//0x8080089c
    uint32 SAR_RXPAF_RXPAF_DROP_FRAG_CNT0;//0x808008a0
    uint32 SAR_RXPAF_RXPAF_DROP_FRAG_CNT1;//0x808008a4
    uint32 SAR_RXPAF_RXPAF_DROP_FRAG_CNT2;//0x808008a8
    uint32 SAR_RXPAF_RXPAF_DROP_FRAG_CNT3;//0x808008ac
    uint32 SAR_RXPAF_RXPAF_DROP_FRAG_CNT4;//0x808008b0
    uint32 SAR_RXPAF_RXPAF_DROP_FRAG_CNT5;//0x808008b4
    uint32 SAR_RXPAF_RXPAF_DROP_FRAG_CNT6;//0x808008b8
    uint32 SAR_RXPAF_RXPAF_DROP_FRAG_CNT7;//0x808008bc
    uint32 SAR_RXPAF_RXPAF_PKT_CNT0;//0x808008c0
    uint32 SAR_RXPAF_RXPAF_PKT_CNT1;//0x808008c4
    uint32 SAR_RXPAF_RXPAF_PKT_CNT2;//0x808008c8
    uint32 SAR_RXPAF_RXPAF_PKT_CNT3;//0x808008cc
    uint32 SAR_RXPAF_RXPAF_PKT_CNT4;//0x808008d0
    uint32 SAR_RXPAF_RXPAF_PKT_CNT5;//0x808008d4
    uint32 SAR_RXPAF_RXPAF_PKT_CNT6;//0x808008d8
    uint32 SAR_RXPAF_RXPAF_PKT_CNT7;//0x808008dc
    uint32 SAR_RXPAF_RXPAF_DROP_PKT_CNT0;//0x808008e0
    uint32 SAR_RXPAF_RXPAF_DROP_PKT_CNT1;//0x808008e4
    uint32 SAR_RXPAF_RXPAF_DROP_PKT_CNT2;//0x808008e8
    uint32 SAR_RXPAF_RXPAF_DROP_PKT_CNT3;//0x808008ec
    uint32 SAR_RXPAF_RXPAF_DROP_PKT_CNT4;//0x808008f0
    uint32 SAR_RXPAF_RXPAF_DROP_PKT_CNT5;//0x808008f4
    uint32 SAR_RXPAF_RXPAF_DROP_PKT_CNT6;//0x808008f8
    uint32 SAR_RXPAF_RXPAF_DROP_PKT_CNT7;//0x808008fc
    uint8  SAR_TMUEXT_REGS_U8[0x910];//0x80801000

}SarControllerRegisters;


#define SAR_CONTROL ((volatile SarControllerRegisters * const) SAR_BASE)

/*
** SAR Registers
*/

#define SAR_TX_CTL_REGS (SAR_BASE + 0x00000060) /* SAR Tx Control Registers */
#define SAR_TX_CTL_REGS_SZ  0x00000020
#define SAR_RX_CTL_REGS (SAR_BASE + 0x00000080) /* SAR Rx Control Registers */
#define SAR_RX_CTL_REGS_SZ  0x00000040
#define SAR_TX_VPI_VCI_REGS (SAR_BASE + 0x00000100) /* SAR  Tx ATM VPI_VCI Table Reg Registers */
#define SAR_TX_VPI_VCI_REGS_SZ  0x00000040
#define SAR_RX_VCAM_REGS (SAR_BASE + 0x00000140) /* SAR  Rx ATM VPI_VCI CAM Table Reg Registers */
#define SAR_RX_VCAM_REGS_SZ  0x00000080
#define SAR_SHPR_REGS (SAR_BASE + 0x00000200) /* SAR Atm Shaper Source Shaping Table Registers */
#define SAR_SHPR_REGS_SZ  0x00000090
#define SAR_RX_PBUF_REGS (SAR_BASE + 0x00000300) /* SAR Rx Packet Buffer Control Registers */
#define SAR_RX_PBUF_REGS_SZ  0x00000060
#define SAR_MIB_REGS (SAR_BASE + 0x00000600) /* SAR  Atm MIB Counters Registers */
#define SAR_MIB_REGS_SZ  0x000000C0
#define SAR_RX_PAF_REGS (SAR_BASE + 0x00000800) /* SAR RxPaf Top Registers */
#define SAR_RX_PAF_REGS_SZ  0x00000100
#define SAR_RX_BOND_REGS (SAR_BASE + 0x00000900) /* SAR RxPaf Bonding Registers */
#define SAR_RX_BOND_REGS_SZ  0x000000D0
#define SAR_TMUEXT_REGS (SAR_BASE + 0x00001000) /* SAR Traffic Management Unit Extended Registers */
#define SAR_TMUEXT_REGS_SZ  0x00000910
#define SAR_TX_DBG_MIB_REGS (SAR_BASE + 0x00002800) /* SAR Tx MIB Counters Registers */
#define SAR_TX_DBG_MIB_REGS_SZ  0x00000090
#define SAR_TXBBH_FIFO (SAR_BASE + 0x00002900) /* SAR Tx BBH FiFo Data Registers */
#define SAR_TXBBH_FIFO_SZ  0x00000800
#define SAR_TXPKT_FIFO (SAR_BASE + 0x00003100) /* SAR Tx Pkt FiFo Data Registers */
#define SAR_TXPKT_FIFO_SZ  0x00000800
#define SAR_TXUTO_FIFO (SAR_BASE + 0x00003900) /* SAR Tx Uto FiFo Data Registers */
#define SAR_TXUTO_FIFO_SZ  0x00000200

typedef struct sysport {
    union {
        SYSTEMPORT_TOPCTRL SYSTEMPORT_TOPCTRL;
        uint8 SYSTEMPORT_TOPCTRL_buf[64];//0x80490000
    };
    union {
        SYSTEMPORT_SYSBUSCFG SYSTEMPORT_SYSBUSCFG;
        uint8 SYSTEMPORT_SYSBUSCFG_buf[704];//0x80490040
    };
    union {
        SYSTEMPORT_RXCHK SYSTEMPORT_RXCHK;
        uint8 SYSTEMPORT_RXCHK_buf[256];//0x80490300
    };
    union {
        SYSTEMPORT_RBUF SYSTEMPORT_RBUF;
        uint8 SYSTEMPORT_RBUF_buf[512];//0x80490400
    };
    union {
        SYSTEMPORT_TBUF SYSTEMPORT_TBUF;
        uint8 SYSTEMPORT_TBUF_buf[2048];//0x80490600
    };
    union {
        SYSTEMPORT_MPD SYSTEMPORT_MPD;
        uint8 SYSTEMPORT_MPD_buf[4608];//0x80490e00
    };
    union {
        SYSTEMPORT_RDMA SYSTEMPORT_RDMA;
        uint8 SYSTEMPORT_RDMA_buf[8192];//0x80492000
    };
    union {
        SYSTEMPORT_TDMA SYSTEMPORT_TDMA;
        uint8 SYSTEMPORT_TDMA_buf[8192];//0x80494000
    };
    union {
        SYSTEMPORT_SPE SYSTEMPORT_SPE;
        uint8 SYSTEMPORT_SPE_buf[8192];//0x80496000
    };
    union {
        SYSTEMPORT_GIB SYSTEMPORT_GIB;
        uint8 SYSTEMPORT_GIB_buf[512];//0x80498000
    };
    union {
        SYSTEMPORT_INTRL2 SYSTEMPORT_INTRL2[6];
        uint8 SYSTEMPORT_INTRL2_buf[256];//0x80498200
    };
    union {
        SYSTEMPORT_INTC SYSTEMPORT_INTC[6];
        uint8 SYSTEMPORT_INTC_buf[256];//0x80498300
    };
    union {
        sys_port_intr2 SYSTEMPORT_INTRL2_MISC_RX;
        uint8 SYSTEMPORT_INTRL2_MISC_RX_buf[256];//0x80498400
    };
    union {
        sys_port_intr2 SYSTEMPORT_INTRL2_MISC_TX;
        uint8 SYSTEMPORT_INTRL2_MISC_TX_buf[256];//0x80498500
    };
} sysport;


#define SYSPORT_0_BASE SYSPORT_BASE
#define SYSPORT(base_num)    ((volatile sysport* const) SYSPORT_##base_num##_BASE)


/*
** DMA Buffer
*/
typedef union DmaDesc {
    struct {
        union {
            struct {
                uint16        status;                   /* buffer status */
#define          DMA_OWN                0x8000  /* cleared by DMA, set by SW */
#define          DMA_EOP                0x4000  /* last buffer in packet */
#define          DMA_SOP                0x2000  /* first buffer in packet */
#define          DMA_WRAP               0x1000  /* */
#if defined(CONFIG_BCM_GMAC)
#define          DMA_GMAC_OUT_OF_RANGE  0x0080  /* Frame length out of range 
                                                   on GMAC */
#endif
#define          DMA_PRIO               0x0C00  /* Prio for Tx */
#define          DMA_APPEND_BRCM_TAG    0x0200
#define          DMA_APPEND_CRC         0x0100
#define          USB_ZERO_PKT           (1<< 0) // Set to send zero length packet
                uint16        length;                   /* in bytes of data in buffer */
#define          DMA_DESC_USEFPM    0x8000
#define          DMA_DESC_MULTICAST 0x4000
#define          DMA_DESC_BUFLENGTH 0x0fff
            };
            uint32      word0;
        };

        uint32        address;                /* address of data */
    };
    uint64 u64_0;
} IUDmaDesc;

/*
** DMA Channel Configuration (1 .. 20)
*/
typedef struct SAR_DmaChannelCfg {
  uint32        cfg;                    /* (00) assorted configuration */
#define         DMA_ENABLE      0x00000001  /* set to enable channel */
#define         DMA_PKT_HALT    0x00000002  /* idle after an EOP flag is detected */
#define         DMA_BURST_HALT  0x00000004  /* idle after finish current memory burst */
  uint32        intStat;                /* (04) interrupts control and status */
  uint32        intMask;                /* (08) interrupts mask */
#define         DMA_BUFF_DONE   0x00000001  /* buffer done */
#define         DMA_DONE        0x00000002  /* packet xfer complete */
#define         DMA_NO_DESC     0x00000004  /* no valid descriptors */
#define         DMA_RX_ERROR    0x00000008  /* rxdma detect client protocol error */
  uint32        maxBurst;               /* (0C) max burst length permitted */
#define         DMA_DESCSIZE_SEL 0x00040000  /* DMA Descriptor Size Selection */
} SAR_DmaChannelCfg;

/*
** DMA State RAM (1 .. 16)
*/
typedef struct SAR_DmaStateRam {
  uint32        baseDescPtr;            /* (00) descriptor ring start address */
  uint32        state_data;             /* (04) state/bytes done/ring offset */
  uint32        desc_len_status;        /* (08) buffer descriptor status and len */
  uint32        desc_base_bufptr;       /* (0C) buffer descrpitor current processing */
} SAR_DmaStateRam;

/*
** DMA Registers
*/

#define IUDMA_MAX_CHANNELS          20

typedef struct SAR_IUDMAControlReg {
    uint32 controller_cfg;              /* (00) controller configuration */
#define DMA_MASTER_EN           0x00000001
#define DMA_FLOWC_CH1_EN        0x00000002
#define DMA_FLOWC_CH3_EN        0x00000004

    // Flow control Ch1
    uint32 flowctl_ch1_thresh_lo;           /* 004 */
    uint32 flowctl_ch1_thresh_hi;           /* 008 */
    uint32 flowctl_ch1_alloc;               /* 00c */
#define DMA_BUF_ALLOC_FORCE     0x80000000

    // Flow control Ch3
    uint32 flowctl_ch3_thresh_lo;           /* 010 */
    uint32 flowctl_ch3_thresh_hi;           /* 014 */
    uint32 flowctl_ch3_alloc;               /* 018 */

    // Flow control Ch5
    uint32 flowctl_ch5_thresh_lo;           /* 01C */
    uint32 flowctl_ch5_thresh_hi;           /* 020 */
    uint32 flowctl_ch5_alloc;               /* 024 */

    // Flow control Ch7
    uint32 flowctl_ch7_thresh_lo;           /* 028 */
    uint32 flowctl_ch7_thresh_hi;           /* 02C */
    uint32 flowctl_ch7_alloc;               /* 030 */

    uint32 ctrl_channel_reset;              /* 034 */
    uint32 ctrl_channel_debug;              /* 038 */
    uint32 reserved1;                       /* 03C */
    uint32 ctrl_global_interrupt_status;    /* 040 */
    uint32 ctrl_global_interrupt_mask;      /* 044 */

    // Unused words
    uint8 reserved2[0x200-0x48];

    // Per channel registers/state ram
    SAR_DmaChannelCfg chcfg[IUDMA_MAX_CHANNELS];/* (200-33F) Channel configuration */

    uint8 reserved3[0x400-0x340];

    union {
        SAR_DmaStateRam     s[IUDMA_MAX_CHANNELS];
        uint32              u32[4 * IUDMA_MAX_CHANNELS];
    } stram;                                /* (400-53F) state ram */
} SAR_IUDMAControlReg;

#define SAR_IUDMA ((volatile SAR_IUDMAControlReg * const) SAR_IUDMA_BASE)

/************************************************************
 * SF2 Queue Hardware Constances                            *
 ************************************************************/
#define SF2_MAX_BUFFER_IN_PAGE          0x200

#endif

#ifdef __cplusplus
}
#endif

#endif

