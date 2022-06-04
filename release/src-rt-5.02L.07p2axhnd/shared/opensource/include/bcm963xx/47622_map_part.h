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

#ifndef __BCM47622_MAP_PART_H
#define __BCM47622_MAP_PART_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bcmtypes.h"
#include "bcm_io_map.h"

#define CHIP_FAMILY_ID_HEX	        0x47622

#ifndef __ASSEMBLER__
  // PROC_MON_IDX,
  // PMB_IDX,
typedef enum 
{
    MEMC_IDX,
    PMC_IDX,
    PERF_IDX,
    PERF1_IDX,
    NANDFLASH_IDX,
    PCM_IDX,
    PCMBUS_IDX,
    USBH_IDX,
    MST_PORT_NODE_PER_IDX,
    MST_PORT_NODE_USB_IDX,
    MST_PORT_NODE_CPU_IDX,
    MST_PORT_NODE_PMC_IDX,
    MST_PORT_NODE_PCIE0_IDX,
    MST_PORT_NODE_SYSPORT_IDX,
    MST_PORT_NODE_SYSPORT1_IDX,
    MST_PORT_NODE_WIFI_IDX,
    MST_PORT_NODE_WIFI1_IDX,
    MST_PORT_NODE_SPU_IDX,
    UBUS4_COHERENCY_PORT_IDX,
    BIUCFG_IDX,
    BOOTLUT_IDX,
    UBUS_MAPPED_IDX,
    UBUS_CAPTURE_PORT_NODE_0,
    UBUS_CAPTURE_PORT_NODE_1,
    SYSPORT_IDX,
    SYSPORT_1_IDX,
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

#define PERF_PHYS_BASE              0xff800000
#define PERF_SIZE                   0x13000
#define PERF1_PHYS_BASE             0xff810000
#define PERF1_SIZE                  0x4b000
#define EMMCFLASH_PHYS_BASE         0xffc00000  
#define EMMCFLASH_SIZE              0x100000    
#define SPIFLASH_PHYS_BASE          0xffd00000  
#define SPIFLASH_SIZE               0x100000    
#define NANDFLASH_PHYS_BASE         0xffe00000  
#define NANDFLASH_SIZE              0x100000    
#define BIU_PHYS_BASE               0x81000000
#define BIUCFG_PHYS_BASE            0x81060000
#define BIUCFG_SIZE                 0x3000
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

#define WLAN0_PHYS_BASE             0x85000000
#define WLAN0_SIZE                  0x01000000
#define WLAN1_PHYS_BASE             0x86000000
#define WLAN1_SIZE                  0x01000000

#define SYSPORT_PHYS_BASE         0x80400000
#define SYSPORT_1_PHYS_BASE       0x80500000
#define SYSPORT_SIZE              0x13000
#define SYSPORT_OFFSET            0x00000
#define SYSPORT_SYSBUSCFG_OFFSET  0x00040
#define SYSPORT_RXCHK_OFFSET      0x300
#define SYSPORT_RBUF_OFFSET       0x400
#define SYSPORT_TBUF_OFFSET       0x600
#define SYSPORT_UMAC_OFFSET       0x800
#define SYSPORT_MIB_OFFSET   (SYSPORT_UMAC_OFFSET + 0x400)
#define SYSPORT_MPD_OFFSET        0xe00
#define SYSPORT_RDMA_OFFSET       0x2000
#define SYSPORT_TDMA_OFFSET       0x4000
#define SYSPORT_SPE_OFFSET        0x6000
#define SYSPORT_INTRL2_OFFSET     0x8200
#define SYSPORT_INTC_OFFSET       0x8300
#define SYSPORT_INTRL2_MISC_RX_OFFSET   0x8400
#define SYSPORT_INTRL2_MISC_TX_OFFSET   0x8500
#define SYSPORT_LED_OFFSET        0x10000
#define SYSPORT_INTRL2_PHY_OFFSET 0x10500
#define SYSPORT_MISC_OFFSET       0x11000
#define SYSPORT_MDIO_OFFSET       0x11300


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
#define MDIO_OFFSET                 0x2060
#define I2C_OFFSET                  0x2100
#define MISC_OFFSET                 0x2600
#define JTAG_OTP_OFFSET             0x2800
#define LED_OFFSET                  0x3000
#define ARM_UART_OFFSET             0x12000


#define NANDFLASH_OFFSET            0x0000
#define SPIFLASH_OFFSET             0x0000 
#define EMMCFLASH_OFFSET            0x0000 
#define BIUCFG_OFFSET               0x0000

#define GPIO_PHYS_BASE              (PERF_PHYS_BASE + GPIO_OFFSET)
#define ARM_UART_PHYS_BASE          (PERF_PHYS_BASE + ARM_UART_OFFSET)
#define LED_PHYS_BASE               (PERF_PHYS_BASE + LED_OFFSET)
#define JTAG_OTP_PHYS_BASE          (PERF_PHYS_BASE + JTAG_OTP_OFFSET)
#define HSSPIM_PHYS_BASE            (PERF_PHYS_BASE + HSSPIM_OFFSET)
#define NAND_REG_PHYS_BASE          (PERF_PHYS_BASE + NAND_REG_OFFSET)
#define NAND_CACHE_PHYS_BASE        (PERF_PHYS_BASE + NAND_CACHE_OFFSET)
#define NAND_INTR_PHYS_BASE         (PERF_PHYS_BASE + NAND_INTR_OFFSET)

#define USBH_BASE                   BCM_IO_MAP(USBH_IDX, USBH_PHYS_BASE, USBH_OFFSET)
#define USBH_CFG_BASE               BCM_IO_MAP(USBH_IDX, USBH_PHYS_BASE, CFG_OFFSET)
#define USBH_EHCI_BASE              BCM_IO_MAP(USBH_IDX, USBH_PHYS_BASE, EHCI_OFFSET)
#define USBH_OHCI_BASE              BCM_IO_MAP(USBH_IDX, USBH_PHYS_BASE, OHCI_OFFSET)
#define USBH_XHCI_BASE              BCM_IO_MAP(USBH_IDX, USBH_PHYS_BASE, XHCI_OFFSET)
#define USB_XHCI_BASE              	USBH_XHCI_BASE

#define MDIO_PHYS_BASE              (PERF_PHYS_BASE + MDIO_OFFSET)
#define I2C_PHYS_BASE               (PERF_PHYS_BASE + I2C_OFFSET)
#define TIMR_PHYS_BASE              (PERF_PHYS_BASE + TIMR_OFFSET)
#define WDTIMR0_PHYS_BASE           (PERF_PHYS_BASE + WDTIMR0_OFFSET)
#define WDTIMR1_PHYS_BASE           (PERF_PHYS_BASE + WDTIMR1_OFFSET)
#define MISC_PHYS_BASE              (PERF_PHYS_BASE + MISC_OFFSET)

#define EMMC_HOST_OFFSET            0x0000 
#define EMMC_TOP_OFFSET             0x0100
#define EMMC_BOOT_OFFSET            0x0200 
#define AHB_CONTROL_OFFSET          0x0300
#define HS_UART_OFFSET              0x0400
#define PL081_DMA_OFFSET            0x1000
#define TOP_CNTRL_OFFSET            0x4a000

#define EMMC_HOST_PHYS_BASE         (PERF1_PHYS_BASE + EMMC_HOST_OFFSET)
#define EMMC_TOP_PHYS_BASE          (PERF1_PHYS_BASE + EMMC_TOP_OFFSET)
#define EMMC_BOOT_PHYS_BASE         (PERF1_PHYS_BASE + EMMC_BOOT_OFFSET)
#define AHB_CONTROL_PHYS_BASE       (PERF1_PHYS_BASE + AHB_CONTROL_OFFSET)
#define HS_UART_PHYS_BASE           (PERF1_PHYS_BASE + HS_UART_OFFSET)
#define PL081_DMA_PHYS_BASE         (PERF1_PHYS_BASE + PL081_DMA_OFFSET)
#define TOP_CNTRL_PHYS_BASE         (PERF1_PHYS_BASE + TOP_CNTRL_OFFSET)

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
#define MST_PORT_NODE_SYSPORT_PHYS_BASE     0x83038000
#define MST_PORT_NODE_SYSPORT_SIZE          0x1000
#define MST_PORT_NODE_SYSPORT1_PHYS_BASE    0x83040000
#define MST_PORT_NODE_SYSPORT1_SIZE         0x1000
#define MST_PORT_NODE_SPU_PHYS_BASE         0x83048000
#define MST_PORT_NODE_SPU_SIZE              0x1000
#define MST_PORT_NODE_WIFI_PHYS_BASE        0x83050000
#define MST_PORT_NODE_WIFI_SIZE             0x1000
#define MST_PORT_NODE_WIFI1_PHYS_BASE       0x83058000
#define MST_PORT_NODE_WIFI1_SIZE            0x1000

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
#define XHCI_OFFSET             0x1000     /* XHCI registers */
#define XHCI_EC_OFFSET          0x1900     /* XHCI extended registers */

/* to support non-DT pltaform device add below defs */
#define USB_EHCI_PHYS_BASE      (USBH_PHYS_BASE+EHCI_OFFSET)
#define USB_OHCI_PHYS_BASE      (USBH_PHYS_BASE+OHCI_OFFSET)
#define USB_EHCI1_PHYS_BASE     (USBH_PHYS_BASE+EHCI1_OFFSET)
#define USB_OHCI1_PHYS_BASE     (USBH_PHYS_BASE+OHCI1_OFFSET)
#define USB_XHCI_PHYS_BASE      (USBH_PHYS_BASE+XHCI_OFFSET)

#define UBUS4_COHERENCY_PORT_PHYS_BASE      0x810A0000
#define UBUS4_COHERENCY_PORT_BASE_SIZE      0x1000
#define UBUS4_RANGE_CHK_SETUP_OFFSET        0x0
#define UBUS4_RANGE_CHK_CONFIG_OFFSET       0x310
#define UBUS4_COHERENCY_PORT_CONFIG_OFFSET  0x400

#define CCI500_PHYS_BASE        0x81100000
#define CCI500_SIZE             0x91000
#define CCI500_OFFSET           0x000

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
#define ARM_UART_BASE               BCM_IO_MAP(PERF_IDX, PERF_PHYS_BASE, ARM_UART_OFFSET)
#define LED_BASE                    BCM_IO_MAP(PERF_IDX, PERF_PHYS_BASE, LED_OFFSET)
#define SOTP_BASE                   BCM_IO_MAP(PERF_IDX, PERF_PHYS_BASE, SOTP_OFFSET)
#define RNG_BASE                    BCM_IO_MAP(PERF_IDX, PERF_PHYS_BASE, RNG_OFFSET)
#define JTAG_OTP_BASE               BCM_IO_MAP(PERF_IDX, PERF_PHYS_BASE, JTAG_OTP_OFFSET)
#define HSSPIM_BASE                 BCM_IO_MAP(PERF_IDX, PERF_PHYS_BASE, HSSPIM_OFFSET)
#define NAND_REG_BASE               BCM_IO_MAP(PERF_IDX, PERF_PHYS_BASE, NAND_REG_OFFSET)
#define NAND_CACHE_BASE             BCM_IO_MAP(PERF_IDX, PERF_PHYS_BASE, NAND_CACHE_OFFSET)
#define NAND_INTR_BASE              BCM_IO_MAP(PERF_IDX, PERF_PHYS_BASE, NAND_INTR_OFFSET)
#define MDIO_BASE                   BCM_IO_MAP(PERF_IDX, PERF_PHYS_BASE, MDIO_OFFSET)
#define I2C_BASE                    BCM_IO_MAP(PERF_IDX, PERF_PHYS_BASE, I2C_OFFSET)
#define MISC_BASE                   BCM_IO_MAP(PERF_IDX, PERF_PHYS_BASE, MISC_OFFSET)

#define PERF1_BASE                  BCM_IO_MAP(PERF1_IDX, PERF1_PHYS_BASE, 0)
#define EMMC_HOSTIF_BASE            BCM_IO_MAP(PERF1_IDX, PERF1_PHYS_BASE, EMMC_HOST_OFFSET)
#define EMMC_TOP_CFG_BASE           BCM_IO_MAP(PERF1_IDX, PERF1_PHYS_BASE, EMMC_TOP_OFFSET)
#define EMMC_BOOT_BASE              BCM_IO_MAP(PERF1_IDX, PERF1_PHYS_BASE, EMMC_BOOT_OFFSET)
#define AHBSS_CTRL_BASE             BCM_IO_MAP(PERF1_IDX, PERF1_PHYS_BASE, AHB_CONTROL_OFFSET)
#define PL081_DMA_BASE              BCM_IO_MAP(PERF1_IDX, PERF1_PHYS_BASE, PL081_DMA_OFFSET)
#define HS_UART_BASE                BCM_IO_MAP(PERF1_IDX, PERF1_PHYS_BASE, HS_UART_OFFSET)
#define TOP_CONTROL_BASE            BCM_IO_MAP(PERF1_IDX, PERF1_PHYS_BASE, TOP_CNTRL_OFFSET)

#define NANDFLASH_BASE              BCM_IO_MAP(NANDFLASH_IDX, NANDFLASH_PHYS_BASE, NANDFLASH_OFFSET)
#define BOOTLUT_BASE                BCM_IO_MAP(BOOTLUT_IDX, BOOTLUT_PHYS_BASE, 0)

#define MST_PORT_NODE_PER_BASE      BCM_IO_MAP(MST_PORT_NODE_PER_IDX, MST_PORT_NODE_PER_PHYS_BASE, 0)
#define MST_PORT_NODE_USB_BASE      BCM_IO_MAP(MST_PORT_NODE_USB_IDX, MST_PORT_NODE_USB_PHYS_BASE, 0)
#define MST_PORT_NODE_CPU_BASE      BCM_IO_MAP(MST_PORT_NODE_CPU_IDX, MST_PORT_NODE_CPU_PHYS_BASE, 0)
#define MST_PORT_NODE_PMC_BASE      BCM_IO_MAP(MST_PORT_NODE_PMC_IDX, MST_PORT_NODE_PMC_PHYS_BASE, 0)
#define MST_PORT_NODE_PCIE0_BASE    BCM_IO_MAP(MST_PORT_NODE_PCIE0_IDX, MST_PORT_NODE_PCIE0_PHYS_BASE, 0)
#define MST_PORT_NODE_SYSPORT_BASE  BCM_IO_MAP(MST_PORT_NODE_SYSPORT_IDX, MST_PORT_NODE_SYSPORT_PHYS_BASE, 0)
#define MST_PORT_NODE_SYSPORT1_BASE BCM_IO_MAP(MST_PORT_NODE_SYSPORT1_IDX, MST_PORT_NODE_SYSPORT1_PHYS_BASE, 0)
#define MST_PORT_NODE_WIFI_BASE     BCM_IO_MAP(MST_PORT_NODE_WIFI_IDX, MST_PORT_NODE_WIFI_PHYS_BASE, 0)
#define MST_PORT_NODE_WIFI1_BASE    BCM_IO_MAP(MST_PORT_NODE_WIFI1_IDX, MST_PORT_NODE_WIFI1_PHYS_BASE, 0)
#define MST_PORT_NODE_SPU_BASE      BCM_IO_MAP(MST_PORT_NODE_SPU_IDX, MST_PORT_NODE_SPU_PHYS_BASE, 0)

#define BIUCFG_BASE                 	BCM_IO_MAP(BIUCFG_IDX, BIUCFG_PHYS_BASE, BIUCFG_OFFSET)
#define UBUS_RANGE_CHK_SETUP_BASE   	BCM_IO_MAP(UBUS4_COHERENCY_PORT_IDX,UBUS4_COHERENCY_PORT_PHYS_BASE, UBUS4_RANGE_CHK_SETUP_OFFSET)
#define UBUS_RANGE_CHK_CFG_BASE     	BCM_IO_MAP(UBUS4_COHERENCY_PORT_IDX,UBUS4_COHERENCY_PORT_PHYS_BASE, UBUS4_RANGE_CHK_CONFIG_OFFSET)
#define UBUS_COHERENCY_PORT_CFG_BASE   	BCM_IO_MAP(UBUS4_COHERENCY_PORT_IDX,UBUS4_COHERENCY_PORT_PHYS_BASE, UBUS4_COHERENCY_PORT_CONFIG_OFFSET)
#define UBUS_CAPTURE_PORT_NODE_0_BASE     BCM_IO_MAP(UBUS_CAPTURE_PORT_NODE_0, UBUS_CAPTURE_PORT_NODE_0_PHYS_BASE, 0)

#define APM_BASE           BCM_IO_MAP(PCM_IDX, PCM_PHYS_BASE, APM_CORE_OFFSET)
#define PCM_BASE           BCM_IO_MAP(PCM_IDX, PCM_PHYS_BASE, PCM_CORE_OFFSET)
#define PCM_DMA_BASE       BCM_IO_MAP(PCM_IDX, PCM_PHYS_BASE, PCM_DMA_OFFSET)
#define PCM_BUS_BASE       BCM_IO_MAP(PCMBUS_IDX, PCMBUS_PHYS_BASE, PCMBUS_OFFSET)
#define CCI500_BASE        BCM_IO_MAP(CCI500_IDX, CCI500_PHYS_BASE, CCI500_OFFSET)

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
#define EMMCFLASH_BASE     BCM_IO_NOMAP(-1, EMMCFLASH_PHYS_BASE, EMMCFLASH_OFFSET)

#define SYSPORT_0_BASE       BCM_IO_MAP(SYSPORT_IDX, SYSPORT_PHYS_BASE, SYSPORT_OFFSET)
#define SYSPORT_SYSBUSCFG_BASE  BCM_IO_MAP(SYSPORT_IDX, SYSPORT_PHYS_BASE, SYSPORT_SYSBUSCFG_OFFSET)
#define SYSPORT_RXCHK_BASE      BCM_IO_MAP(SYSPORT_IDX, SYSPORT_PHYS_BASE, SYSPORT_RXCHK_OFFSET)
#define SYSPORT_RBUF_BASE       BCM_IO_MAP(SYSPORT_IDX, SYSPORT_PHYS_BASE, SYSPORT_RBUF_OFFSET)
#define SYSPORT_TBUF_BASE       BCM_IO_MAP(SYSPORT_IDX, SYSPORT_PHYS_BASE, SYSPORT_TBUF_OFFSET)
#define SYSPORT_UMAC_BASE       BCM_IO_MAP(SYSPORT_IDX, SYSPORT_PHYS_BASE, SYSPORT_UMAC_OFFSET)
#define SYSPORT_MIB_BASE        BCM_IO_MAP(SYSPORT_IDX, SYSPORT_PHYS_BASE, SYSPORT_MIB_OFFSET)
#define SYSPORT_MPD_BASE        BCM_IO_MAP(SYSPORT_IDX, SYSPORT_PHYS_BASE, SYSPORT_MPD_OFFSET)
#define SYSPORT_RDMA_BASE       BCM_IO_MAP(SYSPORT_IDX, SYSPORT_PHYS_BASE, SYSPORT_RDMA_OFFSET)
#define SYSPORT_TDMA_BASE       BCM_IO_MAP(SYSPORT_IDX, SYSPORT_PHYS_BASE, SYSPORT_TDMA_OFFSET)
#define SYSPORT_SPE_BASE        BCM_IO_MAP(SYSPORT_IDX, SYSPORT_PHYS_BASE, SYSPORT_SPE_OFFSET)
#define SYSPORT_INTRL2_BASE     BCM_IO_MAP(SYSPORT_IDX, SYSPORT_PHYS_BASE, SYSPORT_INTRL2_OFFSET)
#define SYSPORT_INTC_BASE       BCM_IO_MAP(SYSPORT_IDX, SYSPORT_PHYS_BASE, SYSPORT_INTC_OFFSET)
#define SYSPORT_INT_MISC_RX_BASE BCM_IO_MAP(SYSPORT_IDX, SYSPORT_PHYS_BASE, SYSPORT_INTRL2_MISC_RX_OFFSET)
#define SYSPORT_INT_MISC_TX_BASE BCM_IO_MAP(SYSPORT_IDX, SYSPORT_PHYS_BASE, SYSPORT_INTRL2_MISC_TX_OFFSET)
#define SYSPORT_LED_BASE        BCM_IO_MAP(SYSPORT_IDX, SYSPORT_PHYS_BASE, SYSPORT_LED_OFFSET)
#define SYSPORT_INTRL2_PHY_BASE BCM_IO_MAP(SYSPORT_IDX, SYSPORT_PHYS_BASE, SYSPORT_INTRL2_PHY_OFFSET)
#define SYSPORT_MISC_BASE       BCM_IO_MAP(SYSPORT_IDX, SYSPORT_PHYS_BASE, SYSPORT_MISC_OFFSET)
#define SYSPORT_MDIO_BASE       BCM_IO_MAP(SYSPORT_IDX, SYSPORT_PHYS_BASE, SYSPORT_MDIO_OFFSET)

#define SYSPORT_1_BASE            BCM_IO_MAP(SYSPORT_1_IDX, SYSPORT_1_PHYS_BASE, SYSPORT_OFFSET)
#define SYSPORT_1_SYSBUSCFG_BASE  BCM_IO_MAP(SYSPORT_1_IDX, SYSPORT_1_PHYS_BASE, SYSPORT_SYSBUSCFG_OFFSET)
#define SYSPORT_1_RXCHK_BASE      BCM_IO_MAP(SYSPORT_1_IDX, SYSPORT_1_PHYS_BASE, SYSPORT_RXCHK_OFFSET)
#define SYSPORT_1_RBUF_BASE       BCM_IO_MAP(SYSPORT_1_IDX, SYSPORT_1_PHYS_BASE, SYSPORT_RBUF_OFFSET)
#define SYSPORT_1_TBUF_BASE       BCM_IO_MAP(SYSPORT_1_IDX, SYSPORT_1_PHYS_BASE, SYSPORT_TBUF_OFFSET)
#define SYSPORT_1_UMAC_BASE       BCM_IO_MAP(SYSPORT_1_IDX, SYSPORT_1_PHYS_BASE, SYSPORT_UMAC_OFFSET)
#define SYSPORT_1_MIB_BASE        BCM_IO_MAP(SYSPORT_1_IDX, SYSPORT_1_PHYS_BASE, SYSPORT_MIB_OFFSET)
#define SYSPORT_1_MPD_BASE        BCM_IO_MAP(SYSPORT_1_IDX, SYSPORT_1_PHYS_BASE, SYSPORT_MPD_OFFSET)
#define SYSPORT_1_RDMA_BASE       BCM_IO_MAP(SYSPORT_1_IDX, SYSPORT_1_PHYS_BASE, SYSPORT_RDMA_OFFSET)
#define SYSPORT_1_TDMA_BASE       BCM_IO_MAP(SYSPORT_1_IDX, SYSPORT_1_PHYS_BASE, SYSPORT_TDMA_OFFSET)
#define SYSPORT_1_SPE_BASE        BCM_IO_MAP(SYSPORT_1_IDX, SYSPORT_1_PHYS_BASE, SYSPORT_SPE_OFFSET)
#define SYSPORT_1_INTRL2_BASE     BCM_IO_MAP(SYSPORT_1_IDX, SYSPORT_1_PHYS_BASE, SYSPORT_INTRL2_OFFSET)
#define SYSPORT_1_INTC_BASE       BCM_IO_MAP(SYSPORT_1_IDX, SYSPORT_1_PHYS_BASE, SYSPORT_INTC_OFFSET)
#define SYSPORT_1_INT_MISC_RX_BASE BCM_IO_MAP(SYSPORT_1_IDX, SYSPORT_1_PHYS_BASE, SYSPORT_INTRL2_MISC_RX_OFFSET)
#define SYSPORT_1_INT_MISC_TX_BASE BCM_IO_MAP(SYSPORT_1_IDX, SYSPORT_1_PHYS_BASE, SYSPORT_INTRL2_MISC_TX_OFFSET)
#define SYSPORT_1_LED_BASE        BCM_IO_MAP(SYSPORT_1_IDX, SYSPORT_1_PHYS_BASE, SYSPORT_LED_OFFSET)
#define SYSPORT_1_INTRL2_PHY_BASE BCM_IO_MAP(SYSPORT_1_IDX, SYSPORT_1_PHYS_BASE, SYSPORT_INTRL2_PHY_OFFSET)



#ifndef __ASSEMBLER__
#ifdef __BOARD_DRV_ARMV7__
BCM_IO_BLOCKS bcm_io_blocks[] =  
{
    {MEMC_IDX, MEMC_SIZE, MEMC_PHYS_BASE},                                                                               
    {PMC_IDX, PMC_SIZE, PMC_PHYS_BASE},                                                                                     
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
    {MST_PORT_NODE_SYSPORT_IDX, MST_PORT_NODE_SYSPORT_SIZE, MST_PORT_NODE_SYSPORT_PHYS_BASE},
    {MST_PORT_NODE_SYSPORT1_IDX, MST_PORT_NODE_SYSPORT1_SIZE, MST_PORT_NODE_SYSPORT1_PHYS_BASE},
    {MST_PORT_NODE_WIFI_IDX, MST_PORT_NODE_WIFI_SIZE, MST_PORT_NODE_WIFI_PHYS_BASE},
    {MST_PORT_NODE_WIFI1_IDX, MST_PORT_NODE_WIFI1_SIZE, MST_PORT_NODE_WIFI1_PHYS_BASE},
    {MST_PORT_NODE_SPU_IDX, MST_PORT_NODE_SPU_SIZE, MST_PORT_NODE_SPU_PHYS_BASE},
    {UBUS4_COHERENCY_PORT_IDX, UBUS4_COHERENCY_PORT_BASE_SIZE, UBUS4_COHERENCY_PORT_PHYS_BASE},     
    {BIUCFG_IDX, BIUCFG_SIZE, BIUCFG_PHYS_BASE},                                                                   
    {BOOTLUT_IDX, BOOTLUT_SIZE, BOOTLUT_PHYS_BASE},                                                                
    {UBUS_MAPPED_IDX, UBUS_MAPPED_SIZE, UBUS_MAPPED_PHYS_BASE},
    {UBUS_CAPTURE_PORT_NODE_0, UBUS_CAPTURE_PORT_NODE_SIZE, UBUS_CAPTURE_PORT_NODE_0_PHYS_BASE},
    {SYSPORT_IDX, SYSPORT_SIZE, SYSPORT_PHYS_BASE},
    {SYSPORT_1_IDX, SYSPORT_SIZE, SYSPORT_1_PHYS_BASE},
    {CCI500_IDX, CCI500_SIZE, CCI500_PHYS_BASE},
};

unsigned long bcm_io_block_address[LAST_IDX];
#else
extern BCM_IO_BLOCKS bcm_io_blocks[];
extern unsigned long bcm_io_block_address[];
#endif

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
   uint32      MiiPadCtl;          // 0x0
   uint32      Rgmii1PadCtl;       // 0x4
   uint32      Rgmii2PadCtl;       // 0x8
   uint32      Rgmii3PadCtl;       // 0xc
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
    uint32        ResetReason;
#define PCIE_RESET_STATUS       0x10000000
#define SW_RESET_STATUS         0x20000000
#define HW_RESET_STATUS         0x40000000
#define POR_RESET_STATUS        0x80000000
#define RESET_STATUS_MASK       0xF0000000
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
        uint32 GPIODir[8];             /* 0x00-0x20 */
        uint32 GPIOio[8];              /* 0x24-0x3c */
        uint32 PadCtrl;                /* 0x40 */
#define MISC_XMII_PAD_MODEHV                    (1 << 8)
#define MISC_XMII_PAD_SEL_GMII                  (1 << 9)
#define MISC_XMII_PAD_AMP_EN                    (1 << 10)
#define MISC_XMII_PAD_DRIVE_STRENGTH_SHIFT      5
#define MISC_XMII_PAD_DRIVE_STRENGTH_MASK       (0x7<<MISC_XMII_PAD_DRIVE_STRENGTH_SHIFT)
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
       uint32 TestPortCmd;              /* 0x5c */
#define LOAD_MUX_REG_CMD        0x21
#define LOAD_PAD_CTRL_CMD       0x22
        uint32 DiagReadBack;            /* 0x60 */
        uint32 DiagReadBackHi;          /* 0x64 */
        uint32 GeneralPurpose;          /* 0x68 */
        uint32 spare[3];
} GpioControl;

#define GPIO ((volatile GpioControl * const) GPIO_BASE)

/* Number to mask conversion macro used for GPIODir and GPIOio */
#define GPIO_NUM_MAX                78 /* accoring to pinmuxing table */  
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
#define MISC_STRAP_BUS_CPU_SLOW_FREQ_SHIFT      9
#define MISC_STRAP_BUS_CPU_SLOW_FREQ            (0x1 << MISC_STRAP_BUS_CPU_SLOW_FREQ_SHIFT)
#define MISC_STRAP_BUS_BOOTROM_BOOT_N           (0x1 << 12)
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

#define I2C   ((volatile I2CControl * const) I2C_BASE)
#define MAX_I2C_BUS                     1

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

typedef struct UBUSInterface {
   uint32 CFG;                /* 0x00 */
#define UBUSIF_CFG_WRITE_REPLY_MODE_SHIFT   0x0
#define UBUSIF_CFG_WRITE_REPLY_MODE_MASK    (0x1<< UBUSIF_CFG_WRITE_REPLY_MODE_SHIFT)
#define UBUSIF_CFG_WRITE_BURST_MODE_SHIFT   0x1
#define UBUSIF_CFG_WRITE_BURST_MODE_MASK    (0x1<< UBUSIF_CFG_WRITE_BURST_MODE_SHIFT)
#define UBUSIF_CFG_INBAND_ERR_MASK_SHIFT    0x2
#define UBUSIF_CFG_INBAND_ERR_MASK_MASK     (0x1<< UBUSIF_CFG_INBAND_ERR_MASK_SHIFT)
#define UBUSIF_CFG_OOB_ERR_MASK_SHIFT       0x3
#define UBUSIF_CFG_OOB_ERR_MASK_MASK        (0x1<< UBUSIF_CFG_OOB_ERR_MASK_SHIFT)
   uint32 ESRCID_CFG;         /* 0x04 */
   uint32 SRC_QUEUE_CTRL[4];  /* 0x08 - 0x17 */
   uint32 REP_ARB_MODE;       /* 0x18 */
#define UBUSIF_REP_ARB_MODE_FIFO_MODE_SHIFT 0x0
#define UBUSIF_REP_ARB_MODE_FIFO_MODE_MASK  (0x1<<UBUSIF_REP_ARB_MODE_FIFO_MODE_SHIFT)
   uint32 HPQ_CFG;            /* 0x1c */
   uint32 SCRATCH;            /* 0x20 */
   uint32 DEBUG_R0;           /* 0x24 */
   uint32 unused[38];         /* 0x28-0xbf */
} UBUSInterface;

typedef struct AXIInterface {
   uint32 CFG;              /* 0x00 */
   uint32 REP_ARB_MODE;     /* 0x04 */
   uint32 QUEUE_CFG;        /* 0x08 */
   uint32 ESRCID_CFG;       /* 0x0c */
   uint32 SRC_QUEUE_CTL[8]; /* 0x10 */
   uint32 SRCID4_QUEUE_CFG; /* 0x30 */
   uint32 SCRATCH;          /* 0x34 */
   uint32 AXI_DEBUG[2];         /* 0x38-0x3f */
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

typedef struct DDRPhyControl {
   uint32 PRIMARY_REVISION;    /* 0x00 */
   uint32 SECONDARY_REVISION;  /* 0x04 */
   uint32 FEATURE;             /* 0x08 */
   uint32 PLL_STATUS;          /* 0x0c */
   uint32 PLL_CONFIG;          /* 0x10 */
   uint32 PLL_CONTROL1;        /* 0x14 */
   uint32 PLL_CONTROL2;        /* 0x18 */
   uint32 PLL_CONTROL3;        /* 0x1c */
   uint32 PLL_DIVIDER;         /* 0x20 */
   uint32 PLL_PRE_DIVIDER;     /* 0x24 */
   uint32 PLL_SS_EN;           /* 0x28 */
   uint32 PLL_SS_CFG;          /* 0x2c */
   uint32 AUX_CONTROL;         /* 0x30 */
   uint32 IDLE_PAD_CONTROL;    /* 0x34 */
   uint32 IDLE_PAD_EN0;        /* 0x38 */
   uint32 IDLE_PAD_EN1;        /* 0x3c */
   uint32 DRIVE_PAD_CTL;       /* 0x40 */
   uint32 DRIVE_PAD_CTL_2T;    /* 0x44 */
   uint32 DRIVE_PAD_CTL_2T_A;  /* 0x48 */
   uint32 STATIC_PAD_CTL;      /* 0x4c */
   uint32 STATIC_PAD_CTL_2T;   /* 0x50 */
   uint32 STATIC_PAD_CTL_2T_A; /* 0x54 */
   uint32 RX_TRIM;             /* 0x58 */
   uint32 DRAM_CFG;            /* 0x5c */
   uint32 DRAM_TIMING2;        /* 0x60 */
   uint32 DRAM_TIMING3;        /* 0x64 */
   uint32 VDL_REGS[45];        /* 0x68-0x11b */
   uint32 UPDATE_VDL;          /* 0x11c */
   uint32 UPDATE_VDL_SNOOP1;   /* 0x120 */
   uint32 UPDATE_VDL_SNOOP2;   /* 0x124 */
   uint32 CMND_REG1;           /* 0x128 */
   uint32 CMND_AUX_REG1;       /* 0x12c */
   uint32 CMND_REG2;           /* 0x130 */
   uint32 CMND_AUX_REG2;       /* 0x134 */
   uint32 CMND_REG3;           /* 0x138 */
   uint32 CMND_AUX_REG3;       /* 0x13c */
   uint32 CMND_REG4;           /* 0x140 */
   uint32 CMND_AUX_REG4;       /* 0x144 */
   uint32 MODE_REG[8];         /* 0x148-167 */
   uint32 ALERT_CLEAR;         /* 0x168 */
   uint32 ALERT_STATUS;        /* 0x16c */
   uint32 ALERT_DF1;           /* 0x170 */
   uint32 WRITE_LEVEL_CTRL;    /* 0x174 */
   uint32 WRITE_LEVEL_STATUS;  /* 0x178 */
   uint32 READ_EN_CTRL;        /* 0x17c */
   uint32 READ_EN_STATUS ;     /* 0x180 */
   uint32 VIRT_VTT_CTRL;       /* 0x184 */
   uint32 VIRT_VTT_STATUS;     /* 0x188 */
   uint32 VIRT_VTT_CONNECTION; /* 0x18c */
   uint32 VIRT_VTT_OVERRIDE;   /* 0x190 */
   uint32 VREF_DAC_CTRL;       /* 0x194 */
   uint32 STANDBY_CTRL;        /* 0x198 */
   uint32 DEBUG_FREEZE_EN;     /* 0x19c */
   uint32 DEBUG_MUX_CTRL;      /* 0x1a0 */
   uint32 DFI_CTRL;            /* 0x1a4 */
   uint32 WRITE_ODT_CTRL;      /* 0x1a8 */
   uint32 ABI_PAR_CTRL;        /* 0x1ac */
   uint32 ZQ_CAL;              /* 0x1b0 */
   uint32 ZQ_CAL2;             /* 0x1b4 */
   uint32 unused6[14];         /* 0x1b8-0x1ef */
   uint32 CLOCK_IDLE;          /* 0x1f0 */
   uint32 unused7[131];        /* 0x1f4-0x3ff */
} DDRPhyControl;

typedef struct DDRPhyByteLaneControl {
   uint32 VDL_CTRL_WR[10];     /* 0x00 - 0x27 */
   uint32 VDL_CTRL_RD[22];     /* 0x28 - 0x7f */
   uint32 VDL_CLK_CTRL;        /* 0x80 */
   uint32 VDL_LDE_CTRL;        /* 0x84 */
   uint32 RD_EN_DLY_CYC;       /* 0x88 */
#define RD_EN_DLY_CYC_CS0_CYCLES_SHIFT      0x0
#define RD_EN_DLY_CYC_CS0_CYCLES_MASK       (0xf<<RD_EN_DLY_CYC_CS0_CYCLES_SHIFT)
   uint32 WR_CHAN_DLY_CYC;     /* 0x8c */
   uint32 RD_CTRL;             /* 0x90 */
   uint32 RD_FIFO_ADDR;        /* 0x94 */
   uint32 RD_FIFO_DATA;        /* 0x98 */
   uint32 RD_FIFO_DM_DBI;      /* 0x9c */
   uint32 RD_FIFO_STATUS;      /* 0xa0 */
   uint32 RD_FIFO_CLR;         /* 0xa4 */
   uint32 DYNAMIC_CLK_CTRL;    /* 0xa8 */
   uint32 IDLE_PAD_CTRL;       /* 0xac */
   uint32 DRIVE_PAD_CTRL;      /* 0xb0 */
   uint32 DQSP_DRIVE_PAD_CTRL; /* 0xb4 */
   uint32 DQSN_DRIVE_PAD_CTRL; /* 0xb8 */
   uint32 RD_EN_DRIVE_PAD_CTRL;/* 0xbc */
   uint32 STATIC_PAD_CTRL;     /* 0xc0 */
   uint32 DQ_RX_TRIM;          /* 0xc4 */
   uint32 MISC_RX_TRIM;        /* 0xc8 */
   uint32 DQS_RX_TRIM;         /* 0xcc */
   uint32 WR_PREAMBLE_MODE;    /* 0xd0 */
   uint32 ODT_CTRL;            /* 0xd4 */
   uint32 LDO_CONFIG;          /* 0xd8 */
   uint32 CLOCK_ENABLE;        /* 0xdc */
   uint32 CLOCK_IDLE;          /* 0xe0 */
   uint32 RD_CHAN_FIFO_CLR;    /* 0xe4 */
   uint32 BL_SPARE_REG;        /* 0xe8 */
   uint32 unused[69];          /* 0xec-0x1ff */
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
#define MEMC_GLB_GCFG_ALT_CHN_ARB_SHIFT    21
#define MEMC_GLB_GCFG_ALT_CHN_ARB_MASK     (0x1<<MEMC_GLB_GCFG_ALT_CHN_ARB_SHIFT)
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
#define MC_DRAM_CFG_DDR4                          0x2
   uint32 CHN_TIM_STAT;          /* 0x238 */
   uint32 CHN_TIM_PERF;          /* 0x23c */

   uint32 unused8a[7];           /* 0x240 - 0x25b */
#define MEMC_DDR_AUTO_SELFREFRESH_EN              (1 << 31)
#define MEMC_DDR_AUTO_SR_IDLE_CNT_MASK            (0x7FFFFFFF)
   uint32 CHN_TIM_AUTO_SELF_REFRESH;    /* 0x25c */
   uint32 CHN_TIM_AUTO_ZQCS;            /* 0x260 */
   uint32 CHN_TIM_TIM3;                 /* 0x264 */
#define MC_CHN_TIM_TIM3_TIM3_TZQCL_MASK                            0x0fff0000
#define MC_CHN_TIM_TIM3_TIM3_TZQCL_ALIGN                           0
#define MC_CHN_TIM_TIM3_TIM3_TZQCL_BITS                            12
#define MC_CHN_TIM_TIM3_TIM3_TZQCL_SHIFT                           16
#define MC_CHN_TIM_TIM3_TIM3_TZQCL_DEFAULT                         0x00000100
#define MC_CHN_TIM_TIM3_TIM3_TZQCS_MASK                            0x000000ff
#define MC_CHN_TIM_TIM3_TIM3_TZQCS_ALIGN                           0
#define MC_CHN_TIM_TIM3_TIM3_TZQCS_BITS                            8
#define MC_CHN_TIM_TIM3_TIM3_TZQCS_SHIFT                           0
#define MC_CHN_TIM_TIM3_TIM3_TZQCS_DEFAULT                         0x00000040

   uint32 unused8[38];                  /* 0x268-0x2ff */

   uint32 ARB_CFG;                /* 0x300 */
   uint32 ARB_QUE_DIS[2];         /* 0x304-0x30b */
   uint32 unused9;                /* 0x30c */
   uint32 ARB_BUCKET_CFG_CPU_WR[4]; /* 0x310-0x31f */
   uint32 ARB_BUCKET_CFG_CPU_RD[4]; /* 0x320-0x32f */
   uint32 ARB_BUCKET_CFG_CPQ_RD[4]; /* 0x330-0x33f */
   uint32 ARB_BUCKET_CFG_CPQ_WR[4]; /* 0x340-0x34f */
   uint32 ARB_BUCKET_CFG_EDIS[4]; /* 0x350-0x35f */
   uint32 ARB_BUCKET_CFG_UBUS[32];/* 0x360-0x3df */
   uint32 ARB_QUEUE_PRI[3];       /* 0x3e0-0x3eb */
   uint32 ARB_WRR_QUEUE_PRI;      /* 0x3ec */
   uint32 ARB_RATE_OK_WEIGHT[2];  /* 0x3f0-0x3f7 */
   uint32 ARB_RATE_FAILED_WEIGHT[2];/* 0x3f8-0x3ff */

   UBUSInterface UBUSIF0;         /* 0x400-0x4bf */
   AXIInterface AXIRIF;           /* 0x4c0-0x4ff */
   uint32 unused10[192];          /* 0x500-0x7ff */

   EDISEngine EDIS_0;             /* 0x800 */
   uint32 unused11[64];           /* 0x900-0x9ff */

   uint32 STATS_CTRL;             /* 0xa00 */
   uint32 STATS_TIMER_CFG;        /* 0xa04 */
   uint32 STATS_TIMER_COUNT;      /* 0xa08 */
   uint32 STATS_TOTAL_SLICE;      /* 0xa0c */
   uint32 STATS_TOTAL_PACKET;     /* 0xa10 */
   uint32 STATS_TOTAL_READ_SLICE; /* 0xa14 */
   uint32 STATS_TOTAL_READ_PACKET;/* 0xa18 */
   uint32 STATS_TOTAL_LATENCY;    /* 0xa1c */
   uint32 STATS_MAX_LATENCY;      /* 0xa20 */
   uint32 STATS_SLICE_REORDER;    /* 0xa24 */
   uint32 STATS_TOTAL_DDR_CMD;    /* 0xa28 */
   uint32 STATS_TOTAL_DDR_ACT;    /* 0xa2c */
   uint32 STATS_TOTAL_DDR_RDWR;   /* 0xa30 */
   uint32 STATS_TOTAL_DDR_WRRD;   /* 0xa34 */
   uint32 STATS_TOTAL_MRQ_CHN_FULL;/* 0xa38 */
   uint32 STATS_TOTAL_SELF_REF;   /* 0xa3c */
   uint32 STATS_ARB_GRANT_MATCH0; /* 0xa40 */
   uint32 STATS_TOTAL_QUEUE_FULL; /* 0xa44 */
   uint32 STATS_TOTAL_ARB_GRANT;  /* 0xa48 */
   uint32 STATS_TOTAL_RATE_OK_GRANT;/* 0xa4c */
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
#define MEMC_STATS_FILTER_CFG_INTF_UBUS1       0 // MEMC_STATS_FILTER_CFG_INTF_AXI
#define MEMC_STATS_FILTER_CFG_INTF_MCP         MEMC_STATS_FILTER_CFG_INTF_UNUSED
#define MEMC_STATS_FILTER_CFG_INTF_EDIS1       MEMC_STATS_FILTER_CFG_INTF_UNUSED
   uint32 STATS_PROG0_SLICE;      /* 0xa54 */
   uint32 STATS_PROG0_PACKET;     /* 0xa58 */
   uint32 STATS_PROG0_READ_SLICE; /* 0xa5c */
   uint32 STATS_PROG0_READ_PACKET;/* 0xa60 */
   uint32 STATS_PROG0_LATENCY;    /* 0xa64 */
   uint32 STATS_PROG0_MAX_LATENCY;/* 0xa68 */
   uint32 unused12;               /* 0xa6c */
   uint32 STATS_FILTER_CFG_1;     /* 0xa70 */
   uint32 STATS_PROG1_SLICE;      /* 0xa74 */
   uint32 STATS_PROG1_PACKET;     /* 0xa78 */ 
   uint32 STATS_PROG1_READ_SLICE; /* 0xa7c */  
   uint32 STATS_PROG1_READ_PACKET;/* 0xa80 */
   uint32 STATS_PROG1_LATENCY;    /* 0xa84 */ 
   uint32 unused13[2];            /* 0xa88 - 0xa8c */   
   uint32 STATS_FILTER_CFG_2;     /* 0xa90 */
   uint32 STATS_PROG2_SLICE;      /* 0xa94 */
   uint32 STATS_PROG2_PACKET;     /* 0xa98 */ 
   uint32 STATS_PROG2_READ_SLICE; /* 0xa9c */  
   uint32 STATS_PROG2_READ_PACKET;/* 0xaa0 */
   uint32 STATS_PROG2_LATENCY;    /* 0xaa4 */ 
   uint32 unused14[2];            /* 0xaa8 - 0xaac */
   uint32 STATS_FILTER_CFG_3;     /* 0xab0 */
   uint32 STATS_PROG3_SLICE;      /* 0xab4 */
   uint32 STATS_PROG3_PACKET;     /* 0xab8 */ 
   uint32 STATS_PROG3_READ_SLICE; /* 0xabc */  
   uint32 STATS_PROG3_READ_PACKET;/* 0xac0 */
   uint32 STATS_PROG3_LATENCY;    /* 0xac4 */
   uint32 unused15[2];            /* 0xac8 - 0xacc */
   uint32 STATS_AXI_BLOCK_CPU_RD; /* 0xad0 */    
   uint32 STATS_AXI_BLOCK_CPQ_RD; /* 0xad4 */    
   uint32 unused16[10];           /* 0xad8 - 0xaff */

   uint32 CAP_CAPTURE_CFG;        /* 0xb00 */
   uint32 CAP_TRIGGER_ADDR;       /* 0xb04 */
   uint32 CAP_READ_CTRL;          /* 0xb08 */
   uint32 unused20;               /* 0xb0c */
   uint32 CAP_CAPTURE_MATCH0;     /* 0xb10 */
   uint32 CAP_CAPTURE_MATCH1;     /* 0xb14 */
   uint32 CAP_CAPTURE_MATCH2;     /* 0xb18 */
   uint32 unused21;               /* 0xb1c */
   uint32 CAP_CAPTURE_MASK0;      /* 0xb20 */
   uint32 CAP_CAPTURE_MASK1;      /* 0xb24 */
   uint32 CAP_CAPTURE_MASK2;      /* 0xb28 */
   uint32 unused22;               /* 0xb2c */
   uint32 CAP_TRIGGER_START_MATCH0;     /* 0xb30 */
   uint32 CAP_TRIGGER_START_MATCH1;     /* 0xb34 */
   uint32 CAP_TRIGGER_START_MATCH2;     /* 0xb38 */
   uint32 unused23;               /* 0xb3c */
   uint32 CAP_TRIGGER_START_MASK0;      /* 0xb40 */
   uint32 CAP_TRIGGER_START_MASK1;      /* 0xb44 */
   uint32 CAP_TRIGGER_START_MASK2;      /* 0xb48 */
   uint32 unused24;               /* 0xb4c */
   uint32 CAP_READ_DATA[8];       /* 0xb50-0xb6f */
   uint32 CAP_TRIGGER_STOP_MATCH0;     /* 0xb70 */
   uint32 CAP_TRIGGER_STOP_MATCH1;     /* 0xb74 */
   uint32 CAP_TRIGGER_STOP_MATCH2;     /* 0xb78 */
   uint32 unused25_0;                  /* 0xb7c */
   uint32 CAP_TRIGGER_STOP_MASK0;      /* 0xb80 */
   uint32 CAP_TRIGGER_STOP_MASK1;      /* 0xb84 */
   uint32 CAP_TRIGGER_STOP_MASK2;      /* 0xb88 */
   uint32 unused25_1[157];             /* 0xb8c-0xdff */

   uint32 SEC_INTR2_CPU_STATUS;   /* 0xe00 */
   uint32 SEC_INTR2_CPU_SET;      /* 0xe04 */
   uint32 SEC_INTR2_CPU_CLEAR;    /* 0xe08 */
   uint32 SEC_INTR2_CPU_MASK_STATUS;  /* 0xe0c */
   uint32 SEC_INTR2_CPU_MASK_SET;  /* 0xe10 */
   uint32 SEC_INTR2_CPU_MASK_CLEAR;/* 0xe14 */
   uint32 unused26[31866];         /* 0xe18-0x1ffff */

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
    uint32 mdio_lvl_irq_clr;
    uint32 mdio_lvl_irq_msk;
} EthernetSwitchMDIO;

#define SWITCH_MDIO_BASE        SYSPORT_MDIO_BASE
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

/* row 14 */
#define OTP_SGMII_DISABLE_ROW			14
#define OTP_SGMII_DISABLE_SHIFT			5
#define OTP_SGMII_DISABLE_MASK			(0x1 << OTP_SGMII_DISABLE_SHIFT) 

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

#define OTP_USB3_DISABLE_ROW           14
#define OTP_USB3_DISABLE_SHIFT         2
#define OTP_USB3_DISABLE_MASK          (0x1 << OTP_USB3_DISABLE_SHIFT)

#define OTP_PCIE_PORT_DISABLE_ROW       14
#define OTP_PCIE_PORT_DISABLE_SHIFT     8
#define OTP_PCIE_PORT_DISABLE_MASK      (0x7 << OTP_PCIE_PORT_DISABLE_SHIFT)

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


#define UNIMAC_CFG_BASE     SYSPORT_UMAC_BASE
#define UNIMAC_MIB_BASE     SYSPORT_MIB_BASE

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
    uint32 rsvd2[22];
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

typedef struct sys_port_mib
{
    uint32 Pkts64Octets;            /* SYSTEMPORT_UMAC_GR64   */
    uint32 Pkts65to127Octets;       /* SYSTEMPORT_UMAC_GR127   */
    uint32 Pkts128to255Octets;      /* SYSTEMPORT_UMAC_GR255   */
    uint32 Pkts256to511Octets;      /* SYSTEMPORT_UMAC_GR511   */
    uint32 Pkts512to1023Octets;     /* SYSTEMPORT_UMAC_GR1023   */
    uint32 Pkts1024to1518Octets;    /* SYSTEMPORT_UMAC_GR1518   */
    uint32 Pkts1519to1522;          /* SYSTEMPORT_UMAC_GRMGV   */
    uint32 Pkts1523to2047;          /* SYSTEMPORT_UMAC_GR2047   */
    uint32 Pkts2048to4095;          /* SYSTEMPORT_UMAC_GR4095   */
    uint32 Pkts4096to8191;          /* SYSTEMPORT_UMAC_GR9216   */
    uint32 RxPkts;                  /* SYSTEMPORT_UMAC_GRPKT   */
    uint32 RxOctetsLo;              /* SYSTEMPORT_UMAC_GRBYT   */
    uint32 RxMulticastPkts;         /* SYSTEMPORT_UMAC_GRMCA   */
    uint32 RxBroadcastPkts;         /* SYSTEMPORT_UMAC_GRBCA   */
    uint32 RxFCSErrs;               /* SYSTEMPORT_UMAC_GRFCS   */
    uint32 RxCtrlFrame;             /* SYSTEMPORT_UMAC_GRXCF   */
    uint32 RxPausePkts;             /* SYSTEMPORT_UMAC_GRXPF   */
    uint32 RxUnknown;               /* SYSTEMPORT_UMAC_GRXUO   */
    uint32 RxAlignErrs;             /* SYSTEMPORT_UMAC_GRALN   */
    uint32 RxExcessSizeDisc;        /* SYSTEMPORT_UMAC_GRFLR   */
    uint32 RxSymbolError;           /* SYSTEMPORT_UMAC_GRCDE   */
    uint32 RxCarrierSenseErrs;      /* SYSTEMPORT_UMAC_GRFCR   */
    uint32 RxOversizePkts;          /* SYSTEMPORT_UMAC_GROVR   */
    uint32 RxJabbers;               /* SYSTEMPORT_UMAC_GRJBR   */
    uint32 RxMtuErrs;               /* SYSTEMPORT_UMAC_GRMTUE   */
    uint32 RxGoodPkts;              /* SYSTEMPORT_UMAC_GRPOK   */
    uint32 RxUnicastPkts;           /* SYSTEMPORT_UMAC_GRUC   */
    uint32 RxPPPPkts;               /* SYSTEMPORT_UMAC_GRPPP   */
    uint32 RxCRCMatchPkts;          /* SYSTEMPORT_UMAC_GRCRC   */
    uint32 dummy1[3];               /* uint8 dummy6[12]   */
    uint32 TxPkts64Octets;          /* SYSTEMPORT_UMAC_TR64   */
    uint32 TxPkts65to127Octets;     /* SYSTEMPORT_UMAC_TR127   */
    uint32 TxPkts128to255Octets;    /* SYSTEMPORT_UMAC_TR255   */
    uint32 TxPkts256to511Octets;    /* SYSTEMPORT_UMAC_TR511   */
    uint32 TxPkts512to1023Octets;   /* SYSTEMPORT_UMAC_TR1023   */
    uint32 TxPkts1024to1518Octets;  /* SYSTEMPORT_UMAC_TR1518   */
    uint32 TxPkts1519to1522;        /* SYSTEMPORT_UMAC_TRMGV   */
    uint32 TxPkts1523to2047;        /* SYSTEMPORT_UMAC_TR2047   */
    uint32 TxPkts2048to4095;        /* SYSTEMPORT_UMAC_TR4095   */
    uint32 TxPkts4096to8191;        /* SYSTEMPORT_UMAC_TR9216   */
    uint32 TxPkts;                  /* SYSTEMPORT_UMAC_GTPKT   */
    uint32 TxMulticastPkts;         /* SYSTEMPORT_UMAC_GTMCA   */
    uint32 TxBroadcastPkts;         /* SYSTEMPORT_UMAC_GTBCA   */
    uint32 TxPausePkts;             /* SYSTEMPORT_UMAC_GTXPF   */
    uint32 TxCtrlFrame;             /* SYSTEMPORT_UMAC_GTXCF   */
    uint32 TxFCSErrs;               /* SYSTEMPORT_UMAC_GTFCS   */
    uint32 TxOversizePkts;          /* SYSTEMPORT_UMAC_GTOVR   */
    uint32 TxDeferredTx;            /* SYSTEMPORT_UMAC_GTDRF   */
    uint32 TxExcessiveDef;          /* SYSTEMPORT_UMAC_GTEDF   */
    uint32 TxSingleCol;             /* SYSTEMPORT_UMAC_GTSCL   */
    uint32 TxMultipleCol;           /* SYSTEMPORT_UMAC_GTMCL   */
    uint32 TxLateCol;               /* SYSTEMPORT_UMAC_GTLCL   */
    uint32 TxExcessiveCol;          /* SYSTEMPORT_UMAC_GTXCL   */
    uint32 TxFragments;             /* SYSTEMPORT_UMAC_GTFRG   */
    uint32 TxCol;                   /* SYSTEMPORT_UMAC_GTNCL   */
    uint32 TxJabber;                /* SYSTEMPORT_UMAC_GTJBR   */
    uint32 TxOctetsLo;              /* SYSTEMPORT_UMAC_GTBYT   */
    uint32 TxGoodPkts;              /* SYSTEMPORT_UMAC_GTPOK   */
    uint32 TxUnicastPkts;           /* SYSTEMPORT_UMAC_GTUC   */
    uint32 dummy2[3];               /* uint8 dummy7[12]   */
    uint32 RxRuntPkts;              /* SYSTEMPORT_UMAC_RRPKT   */
    uint32 RxRuntValidFCSPkts;      /* SYSTEMPORT_UMAC_RRUND   */
    uint32 RxRuntInvalidFCSPkts;    /* SYSTEMPORT_UMAC_RRFRG   */
    uint32 RxRuntOctets;            /* SYSTEMPORT_UMAC_RRBYT   */
    uint32 dummy3[28];
    uint32 MibCntrl;                /* only on systemport0 */
}sys_port_mib, SYSTEMPORT_MIB_COUNTERS;

typedef struct sys_port_intr2_phy
{
        uint32 SYSTEMPORT_INTRL2_PHY_CPU_STATUS;
        uint32 SYSTEMPORT_INTRL2_PHY_CPU_SET;
        uint32 SYSTEMPORT_INTRL2_PHY_CPU_CLEAR;
        uint32 SYSTEMPORT_INTRL2_PHY_CPU_MASK_STATUS;
        uint32 SYSTEMPORT_INTRL2_PHY_CPU_MASK_SET;
        uint32 SYSTEMPORT_INTRL2_PHY_CPU_MASK_CLEAR;
        uint32 SYSTEMPORT_INTRL2_PHY_PCI_STATUS;
        uint32 SYSTEMPORT_INTRL2_PHY_PCI_SET;
        uint32 SYSTEMPORT_INTRL2_PHY_PCI_CLEAR;
        uint32 SYSTEMPORT_INTRL2_PHY_PCI_MASK_STATUS;
        uint32 SYSTEMPORT_INTRL2_PHY_PCI_MASK_SET;
        uint32 SYSTEMPORT_INTRL2_PHY_PCI_MASK_CLEAR;
}sys_port_intr2_phy,SYSTEMPORT_X_INTRL2_PHY;


typedef struct sys_port_led
{
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
#define LNK_STATUS_OVRD_SHIFT 13
#define LNK_STATUS_OVRD_MASK (0x1 << LNK_STATUS_OVRD_SHIFT)
#define RX_ACT_EN_SHIFT 0
#define RX_ACT_EN_MASK (0x1 << RX_ACT_EN_SHIFT)
#define SPDLNK_LED1_ACT_SEL_SHIFT 3
#define SPDLNK_LED1_ACT_SEL_MASK (0x1 << SPDLNK_LED1_ACT_SEL_SHIFT)
#define ACT_LED_POL_SEL_SHIFT 9
#define ACT_LED_POL_SEL_MASK (0x1 << ACT_LED_POL_SEL_SHIFT)
#define LNK_OVRD_EN_SHIFT 15
#define LNK_OVRD_EN_MASK (0x1 << LNK_OVRD_EN_SHIFT)
    uint32 led_encoding_sel;
#define LINK_AND_SPEED_ENCODING_SEL_SEL_10G_ENCODE_SHIFT 15
#define LINK_AND_SPEED_ENCODING_SEL_SEL_10G_ENCODE_MASK (0x7 << LINK_AND_SPEED_ENCODING_SEL_SEL_10G_ENCODE_SHIFT)
#define LINK_AND_SPEED_ENCODING_SEL_SEL_2500M_ENCODE_SHIFT 12
#define LINK_AND_SPEED_ENCODING_SEL_SEL_2500M_ENCODE_MASK (0x7 << LINK_AND_SPEED_ENCODING_SEL_SEL_2500M_ENCODE_SHIFT)
#define LINK_AND_SPEED_ENCODING_SEL_SEL_1000M_ENCODE_SHIFT 9
#define LINK_AND_SPEED_ENCODING_SEL_SEL_1000M_ENCODE_MASK (0x7 << LINK_AND_SPEED_ENCODING_SEL_SEL_1000M_ENCODE_SHIFT)
#define LINK_AND_SPEED_ENCODING_SEL_SEL_100M_ENCODE_SHIFT 6
#define LINK_AND_SPEED_ENCODING_SEL_SEL_100M_ENCODE_MASK (0x7 << LINK_AND_SPEED_ENCODING_SEL_SEL_100M_ENCODE_SHIFT)
#define LINK_AND_SPEED_ENCODING_SEL_SEL_10M_ENCODE_SHIFT 3
#define LINK_AND_SPEED_ENCODING_SEL_SEL_10M_ENCODE_MASK (0x7 << LINK_AND_SPEED_ENCODING_SEL_SEL_10M_ENCODE_SHIFT)
    uint32 led_encoding;
#define LINK_AND_SPEED_ENCODING_M10G_ENCODE_SHIFT 15
#define LINK_AND_SPEED_ENCODING_M10G_ENCODE_MASK (0x7 << LINK_AND_SPEED_ENCODING_M10G_ENCODE_SHIFT)
#define LINK_AND_SPEED_ENCODING_M2500_ENCODE_SHIFT 12
#define LINK_AND_SPEED_ENCODING_M2500_ENCODE_MASK (0x7 << LINK_AND_SPEED_ENCODING_M2500_ENCODE_SHIFT)
#define LINK_AND_SPEED_ENCODING_M1000_ENCODE_SHIFT 9
#define LINK_AND_SPEED_ENCODING_M1000_ENCODE_MASK (0x7 << LINK_AND_SPEED_ENCODING_M1000_ENCODE_SHIFT)
#define LINK_AND_SPEED_ENCODING_M100_ENCODE_SHIFT 6
#define LINK_AND_SPEED_ENCODING_M100_ENCODE_MASK (0x7 << LINK_AND_SPEED_ENCODING_M100_ENCODE_SHIFT)
#define LINK_AND_SPEED_ENCODING_M10_ENCODE_SHIFT 3
#define LINK_AND_SPEED_ENCODING_M10_ENCODE_MASK (0x7 << LINK_AND_SPEED_ENCODING_M10_ENCODE_SHIFT)
    uint32 led_rate_control;
}sys_port_led,SYSTEMPORT_LED_REG;

#define LED_CFG SYSTEMPORT_LED_REG

#define FIELD_MASK(bits, shift)  ( ( (1ULL<<(bits)) - 1 ) << shift )

#define SYSPORT_UMAC_CMD_TX_ENA_M     FIELD_MASK(1,0)
#define SYSPORT_UMAC_CMD_RX_ENA_M     FIELD_MASK(1,1)

#define SYSPORT_UMAC_MPD_CTRL_MPD_EN  FIELD_MASK(1,0)

typedef struct sys_port_umac {
	uint32 SYSTEMPORT_UMAC_UMAC_DUMMY;
	uint32 SYSTEMPORT_UMAC_HD_BKP_CNTL;             //0x804
#define SYSPORT_UNIMAC_COMMAND_CONFIG_ENA_EXT_CONFIG        FIELD_MASK(1,22)
#define SYSPORT_UNIMAC_COMMAND_CONFIG_RX_EN 0x02
#define SYSPORT_UNIMAC_COMMAND_CONFIG_TX_EN 0x01
	uint32 SYSTEMPORT_UMAC_CMD;                     //0x808
	uint32 SYSTEMPORT_UMAC_MAC0;                    //0x80c
	uint32 SYSTEMPORT_UMAC_MAC1;                    //0x810
	uint32 SYSTEMPORT_UMAC_FRM_LEN;                 //0x814
	uint32 SYSTEMPORT_UMAC_PAUSE_QUNAT;             //0x818
	uint32 dummy1[9];
	uint32 SYSTEMPORT_UMAC_TX_TS_SEQ_ID;            //0x83c
	uint32 SYSTEMPORT_UMAC_SFD_OFFSET;              //0x840
	uint32 SYSTEMPORT_UMAC_MODE;                    //0x844
	uint32 SYSTEMPORT_UMAC_FRM_TAG0;                //0x848
	uint32 SYSTEMPORT_UMAC_FRM_TAG1;                //0x84c
	uint32 SYSTEMPORT_UMAC_RX_PAUSE_QUANTA_SCALE;   //0x850
	uint32 SYSTEMPORT_UMAC_TX_PREAMBLE;             //0x854
	uint32 dummy2;
	uint32 SYSTEMPORT_UMAC_TX_IPG_LEN;              //0x85c
	uint32 SYSTEMPORT_UMAC_PFC_XOFF_TIMER;          //0x860
	uint32 SYSTEMPORT_UMAC_EEE_CTRL;                //0x864
	uint32 SYSTEMPORT_UMAC_MII_EEE_LPI_TIMER;       //0x868
	uint32 SYSTEMPORT_UMAC_GMII_EEE_LPI_TIMER;      //0x86c
	uint32 SYSTEMPORT_UMAC_EEE_REF_COUNT;           //0x870
	uint32 SYSTEMPORT_UMAC_TIMESTAMP_ADJUST;        //0x874
	uint32 SYSTEMPORT_UMAC_RX_PKT_DROP_STATUS;      //0x878
	uint32 SYSTEMPORT_UMAC_SYMMETRIC_IDLE_THRESHOLD;//0x87c
	uint32 SYSTEMPORT_UMAC_MII_EEE_WAKE_TIMER;      //0x880
	uint32 SYSTEMPORT_UMAC_GMII_EEE_WAKE_TIMER;     //0x884
	uint32 SYSTEMPORT_UMAC_REV_ID;                  //0x888
	uint32 dummy5[157];
	uint32 SYSTEMPORT_UMAC_MAC_PFC_TYPE;            //0xb00
	uint32 SYSTEMPORT_UMAC_MAC_PFC_OPCODE;          //0xb04
	uint32 SYSTEMPORT_UMAC_MAC_PFC_DA_0;            //0xb08
	uint32 SYSTEMPORT_UMAC_MAC_PFC_DA_1;            //0xb0c
	uint32 SYSTEMPORT_UMAC_MACSEC_PROG_TX_CRC;      //0xb10
	uint32 SYSTEMPORT_UMAC_MACSEC_CNTRL;            //0xb14
	uint32 SYSTEMPORT_UMAC_TS_STATUS_CNTRL;         //0xb18
	uint32 SYSTEMPORT_UMAC_TX_TS_DATA;              //0xb1c
	uint32 dummy6[4];
	uint32 SYSTEMPORT_UMAC_PAUSE_CNTRL;             //0xb30
	uint32 SYSTEMPORT_UMAC_FLUSH_CNTRL;             //0xb34
	uint32 SYSTEMPORT_UMAC_RXFIFO_STAT;             //0xb38
	uint32 SYSTEMPORT_UMAC_TXFIFO_STAT;             //0xb3c
	uint32 SYSTEMPORT_UMAC_MAC_PFC_CTRL;            //0xb40
	uint32 SYSTEMPORT_UMAC_MAC_PFC_REFRESH_CTRL;    //0xb44
}sys_port_umac, SYSTEMPORT_UNIMAC;

#define SYSPORT_MPD_CTRL_MPD_EN  FIELD_MASK(1,0)
typedef struct sys_port_mpd {
	uint32 SYSTEMPORT_MPD_CTRL;
	uint32 SYSTEMPORT_MPD_PSW_MS;
	uint32 SYSTEMPORT_MPD_PSW_LS;
}sys_port_mpd, SYSTEMPORT_MPD;


#define SYSPORT_RDMA_CTRL_RDMA_EN_M             FIELD_MASK(1,0)
#define SYSPORT_RDMA_CTRL_RING_CFG_M            FIELD_MASK(2,1)
#define SYSPORT_RDMA_CTRL_RING_CFG_bit0_M       FIELD_MASK(1,1)
#define SYSPORT_RDMA_CTRL_RING_CFG_bit1_M       FIELD_MASK(1,2)
#define SYSPORT_RDMA_CTRL_DISCARD_EN_M          FIELD_MASK(1,3)
#define SYSPORT_RDMA_CTRL_DATA_OFFSET_M         FIELD_MASK(10,4)
#define SYSPORT_RDMA_CTRL_DDR_DESC_WR_EN_M      FIELD_MASK(1,15)
#define SYSPORT_RDMA_CTRL_PSB_EN_M              FIELD_MASK(1,18)

#define SYSPORT_RDMA_STATUS_RDMA_DISABLED_M     FIELD_MASK(1,0)
#define SYSPORT_RDMA_STATUS_DESC_RAM_BUSY_M     FIELD_MASK(1,1)

#define SYSTEMPORT_RDMA_LOCRAM_DESCRING_SIZE_MAX            128
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
#define SYSPORT_RBUF_CTRL_ACPI_EN_M           FIELD_MASK(1,25)

#define SYSPORT_RBUF_CTRL_RSB_MODE_NO_RSB     0
#define SYSPORT_RBUF_CTRL_RSB_MODE_RSB8       1
#define SYSPORT_RBUF_CTRL_RSB_MODE_RSB32      2

#define SYSPORT_RBUF_PACKET_READY_THRESHOLD_PKT_RDY_THRESHOLD_M   FIELD_MASK(10,0)
#define SYSPORT_RBUF_PACKET_READY_THRESHOLD_PKT_RDY_THRESHOLD_S   0
#define SYSPORT_RBUF_PACKET_READY_THRESHOLD_RESUME_THRESHOLD_M    FIELD_MASK(10,16)
#define SYSPORT_RBUF_PACKET_READY_THRESHOLD_RESUME_THRESHOLD_S    16

#define SYSPORT_RBUF_STATUS_WOL_M           FIELD_MASK(1,0)
#define SYSPORT_RBUF_STATUS_WOL_S           0
#define SYSPORT_RBUF_STATUS_MPD_ACTIVE_M    FIELD_MASK(1,1)
#define SYSPORT_RBUF_STATUS_MPD_ACTIVE_S    1
#define SYSPORT_RBUF_STATUS_ARP_ACTIVE_M    FIELD_MASK(1,2)
#define SYSPORT_RBUF_STATUS_ARP_ACTIVE_S    2

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

#define SYSTEMPORT_TDMA_DESC_RING_MAX        8
#define SYSTEMPORT_TDMA_LOCRAM_DESCRING_MAX  512

#define SYSTEMPORT_TDMA_TIMEOUT_TICK_NSEC    1000 // tick = 1 usec (internal timer)

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


typedef struct gphy_ctrl {
	uint32 SYSTEMPORT_MISC_SEGPHY_TEST_CNTRL;
	uint32 SYSTEMPORT_MISC_SEGPHY_CNTRL;
	uint32 SYSTEMPORT_MISC_SEGPHY_STATUS;
}gphy_ctrl;

typedef struct sys_port_misc {
	uint32 SYSTEMPORT_MISC_REV_CONTROL;
	uint32 SYSTEMPORT_MISC_CROSSBAR3X2_CONTROL;
#define SYSPORT0_USE_RGMII                  1
#define SYSPORT1_USE_RGMII                  2
	uint32 SYSTEMPORT_MISC_GPIO_INTR_CONTROL;
	uint32 SYSTEMPORT_MISC_SYSTEMPORT_CONTROL;
	unsigned char dummy[0x10];
	uint32 SYSTEMPORT_MISC_RGMII_CNTRL;
#define ETHSW_RC_MII_MODE_MASK               0x1c
#define ETHSW_RC_EXT_RVMII                   0x10
#define ETHSW_RC_EXT_GPHY                    0x0c
#define ETHSW_RC_EXT_EPHY                    0x08
#define ETHSW_RC_INT_GPHY                    0x04
#define ETHSW_RC_INT_EPHY                    0x00
#define ETHSW_RC_ID_MODE_DIS                 0x02
#define ETHSW_RC_RGMII_EN                    0x01
	uint32 SYSTEMPORT_MISC_RGMII_IB_STATUS;
#define RGMII_IB_ST_OVRD                     (1<<4)
#define RGMII_IB_ST_LINK_UP                  (1<<3)
#define RGMII_IB_ST_FD                       (1<<2)
#define RGMII_IB_ST_1000                     (2<<0)
#define RGMII_IB_ST_100                      (1<<0)
#define RGMII_IB_ST_10                       (0<<0)
	uint32 SYSTEMPORT_MISC_RGMII_RX_CLOCK_DELAY_CNTRL;
#define ETHSW_RXCLK_IDDQ                     (1<<4)
#define ETHSW_RXCLK_BYPASS                   (1<<5)
	uint32 SYSTEMPORT_MISC_RGMII_ATE_RX_CNTRL_EXP_DATA;
	uint32 SYSTEMPORT_MISC_RGMII_ATE_RX_EXP_DATA_1;
	uint32 SYSTEMPORT_MISC_RGMII_ATE_RX_STATUS_0;
	uint32 SYSTEMPORT_MISC_RGMII_ATE_RX_STATUS_1;
	uint32 SYSTEMPORT_MISC_RGMII_ATE_TX_CNTRL;
	uint32 SYSTEMPORT_MISC_RGMII_ATE_TX_DATA_0;
	uint32 SYSTEMPORT_MISC_RGMII_ATE_TX_DATA_1;
	uint32 SYSTEMPORT_MISC_RGMII_ATE_TX_DATA_2;
	uint32 SYSTEMPORT_MISC_RGMII_TX_DELAY_CNTRL_0;
	uint32 SYSTEMPORT_MISC_RGMII_TX_DELAY_CNTRL_1;
	uint32 SYSTEMPORT_MISC_RGMII_RX_DELAY_CNTRL_0;
	uint32 SYSTEMPORT_MISC_RGMII_RX_DELAY_CNTRL_1;
	uint32 SYSTEMPORT_MISC_RGMII_RX_DELAY_CNTRL_2;
	unsigned char dummy_1[0x20];
	uint32 SYSTEMPORT_MISC_LED_PWM_CNTRL;
	uint32 SYSTEMPORT_MISC_LED_INTENSITY_CNTRL;
	unsigned char dummy_2[0x1c];
	uint32 SYSTEMPORT_MISC_SINGLE_SERDES_REV;
	uint32 SYSTEMPORT_MISC_SINGLE_SERDES_CNTRL;
	uint32 SYSTEMPORT_MISC_SINGLE_SERDES_STAT;
	uint32 SYSTEMPORT_MISC_SINGLE_SERDES_APD_CNTRL;
	uint32 SYSTEMPORT_MISC_SINGLE_SERDES_APD_FSM_CNTRL;
	uint32 SYSTEMPORT_MISC_SEGPHY_REVISION;
	uint32 SYSTEMPORT_MISC_SEGPHY_TEST_CNTRL;
	uint32 SYSTEMPORT_MISC_SEGPHY_CNTRL;
	uint32 SYSTEMPORT_MISC_SEGPHY_STATUS;
} sys_port_misc, SYSTEMPORT_MISC;

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

#define SYSTEMPORT_CLOCK_FREQUENCY  250 // MHz

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

#define SYSPORT_RXCHK_RSB32BX_CFG2_L3UCAST_TOS_EN_VECT_M  FIELD_MASK(8,0)
#define SYSPORT_RXCHK_RSB32BX_CFG2_L3UCAST_TOS_EN_VECT_S  0
#define SYSPORT_RXCHK_RSB32BX_CFG2_L2UCAST_TOS_EN_VECT_M  FIELD_MASK(8,8)
#define SYSPORT_RXCHK_RSB32BX_CFG2_L2UCAST_TOS_EN_VECT_S  8
#define SYSPORT_RXCHK_RSB32BX_CFG2_VLAN_DEI_EN_M          FIELD_MASK(1,16)
#define SYSPORT_RXCHK_RSB32BX_CFG2_VLAN_PCP_EN_M          FIELD_MASK(1,17)

#define SYSPORT_RXCHK_WAN_PORT_AND_HOST_CPU_MAPPING_WAN0_PORT_ID_M            FIELD_MASK(4,0)
#define SYSPORT_RXCHK_WAN_PORT_AND_HOST_CPU_MAPPING_WAN0_PORT_ID_S            0
#define SYSPORT_RXCHK_WAN_PORT_AND_HOST_CPU_MAPPING_WAN0_PORT_ENABLE_M        FIELD_MASK(1,4)
#define SYSPORT_RXCHK_WAN_PORT_AND_HOST_CPU_MAPPING_WAN0_PORT_MAP_ENABLE_M    FIELD_MASK(1,5)
#define SYSPORT_RXCHK_WAN_PORT_AND_HOST_CPU_MAPPING_WAN1_PORT_ID_M            FIELD_MASK(4,8)
#define SYSPORT_RXCHK_WAN_PORT_AND_HOST_CPU_MAPPING_WAN1_PORT_ID_S            8
#define SYSPORT_RXCHK_WAN_PORT_AND_HOST_CPU_MAPPING_WAN1_PORT_ENABLE_M        FIELD_MASK(1,12)
#define SYSPORT_RXCHK_WAN_PORT_AND_HOST_CPU_MAPPING_WAN1_PORT_MAP_ENABLE_M    FIELD_MASK(1,13)
#define SYSPORT_RXCHK_WAN_PORT_AND_HOST_CPU_MAPPING_HOST_CPU_TC_M             FIELD_MASK(3,16)

#define SYSPORT_TPC     ((volatile sys_port_topctrl * const) SYSPORT_0_BASE)
#define SYSPORT_SYSBUSCFG     ((volatile sys_port_sysbuscfg * const) SYSPORT_SYSBUSCFG_BASE)
#define SYSPORT_RXCHK   ((volatile sys_port_rxchk * const) SYSPORT_RXCHK_BASE)
#define SYSPORT_RBUF    ((volatile sys_port_rbuf    * const) SYSPORT_RBUF_BASE)
#define SYSPORT_TBUF    ((volatile sys_port_tbuf    * const) SYSPORT_TBUF_BASE)
#define SYSPORT_UMAC    ((volatile sys_port_umac    * const) SYSPORT_UMAC_BASE)
#define SYSPORT_MIB     ((volatile sys_port_mib * const) (SYSPORT_MIB_BASE))
#define SYSPORT_MPD     ((volatile sys_port_mpd * const) (SYSPORT_MPD_BASE))
#define SYSPORT_RDMA    ((volatile sys_port_rdma    * const) SYSPORT_RDMA_BASE)
#define SYSPORT_TDMA    ((volatile sys_port_tdma    * const) SYSPORT_TDMA_BASE)
#define SYSPORT_SPE     ((volatile sys_port_spe    * const) SYSPORT_SPE_BASE)
#define SYSPORT_INTRL2  ((volatile sys_port_intr2   * const) (SYSPORT_INTRL2_BASE))
#define SYSPORT_INTC    ((volatile sys_port_intc    * const) (SYSPORT_INTC_BASE))
#define SYSPORT_INT_MISC_RX ((volatile sys_port_intr2    * const) (SYSPORT_INT_MISC_RX_BASE))
#define SYSPORT_INT_MISC_TX ((volatile sys_port_intr2    * const) (SYSPORT_INT_MISC_TX_BASE))
#define SYSPORT_LED     ((volatile sys_port_led * const) (SYSPORT_LED_BASE))
#define SYSPORT_INTRL2_PHY  ((volatile sys_port_intr2_phy * const) (SYSPORT_INTRL2_PHY_BASE))
#define SYSPORT_MISC    ((volatile sys_port_misc * const) (SYSPORT_MISC_BASE))
#define SYSPORT_MDIO    ((volatile EthernetSwitchMDIO * const) (SYSPORT_MDIO_BASE))


#define SYSPORT_1_TPC     ((volatile sys_port_topctrl * const) SYSPORT_1_BASE)
#define SYSPORT_1_SYSBUSCFG     ((volatile sys_port_sysbuscfg * const) SYSPORT_1_SYSBUSCFG_BASE)
#define SYSPORT_1_RXCHK   ((volatile sys_port_rxchk * const) SYSPORT_1_RXCHK_BASE)
#define SYSPORT_1_RBUF    ((volatile sys_port_rbuf    * const) SYSPORT_1_RBUF_BASE)
#define SYSPORT_1_TBUF    ((volatile sys_port_tbuf    * const) SYSPORT_1_TBUF_BASE)
#define SYSPORT_1_UMAC    ((volatile sys_port_umac    * const) SYSPORT_1_UMAC_BASE)
#define SYSPORT_1_MIB     ((volatile sys_port_mib * const) (SYSPORT_1_MIB_BASE))
#define SYSPORT_1_MPD     ((volatile sys_port_mpd * const) (SYSPORT_1_MPD_BASE))
#define SYSPORT_1_RDMA    ((volatile sys_port_rdma    * const) SYSPORT_1_RDMA_BASE)
#define SYSPORT_1_TDMA    ((volatile sys_port_tdma    * const) SYSPORT_1_TDMA_BASE)
#define SYSPORT_1_SPE     ((volatile sys_port_spe    * const) SYSPORT_1_SPE_BASE)
#define SYSPORT_1_INTRL2  ((volatile sys_port_intr2   * const) (SYSPORT_1_INTRL2_BASE))
#define SYSPORT_1_INTC    ((volatile sys_port_intc    * const) (SYSPORT_1_INTC_BASE))
#define SYSPORT_1_INT_MISC_RX ((volatile sys_port_intr2    * const) (SYSPORT_1_INT_MISC_RX_BASE))
#define SYSPORT_1_INT_MISC_TX ((volatile sys_port_intr2    * const) (SYSPORT_1_INT_MISC_TX_BASE))
#define SYSPORT_1_LED     ((volatile sys_port_led * const) (SYSPORT_1_LED_BASE))
#define SYSPORT_1_INTRL2_PHY  ((volatile sys_port_intr2_phy * const) (SYSPORT_1_INTRL2_PHY_BASE))

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

#define PHY_TEST_CTRL	((volatile unsigned int*)(&SYSPORT_MISC->SYSTEMPORT_MISC_SEGPHY_TEST_CNTRL))
#define SPHY_CNTRL	((volatile unsigned int*)(&SYSPORT_MISC->SYSTEMPORT_MISC_SEGPHY_CNTRL))

#define SWITCH_SINGLE_SERDES_STAT (&SYSPORT_MISC->SYSTEMPORT_MISC_SINGLE_SERDES_STAT)
    #define SWITCH_REG_SSER_LINK_STAT   (1<<0)
    #define SWITCH_REG_SSER_RXSIG_DET   (1<<1)
    #define SWITCH_REG_SSER_RXSIG_1G    (1<<2)
    #define SWITCH_REG_SSER_SGMII       (1<<3)
    #define SWITCH_REG_SSER_SYNC_STAT   (1<<4)
    #define SWITCH_REG_SSER_POLL_LOCK   (1<<5)
    #define SWITCH_REG_SSER_EXTFB_DET   (1<<6)

#define SWITCH_REG_SINGLE_SERDES_CNTRL (&SYSPORT_MISC->SYSTEMPORT_MISC_SINGLE_SERDES_CNTRL)
#define SWITCH_REG_SERDES_IDDQ       (1<<0)
#define SWITCH_REG_SERDES_PWRDWN     (1<<1)
#define SWITCH_REG_SERDES_RESETPLL   (1<<3)
#define SWITCH_REG_SERDES_RESETMDIO  (1<<4)
#define SWITCH_REG_SERDES_RESET      (1<<5)

typedef struct sysport {
    union {
        SYSTEMPORT_TOPCTRL SYSTEMPORT_TOPCTRL;
        uint8 SYSTEMPORT_TOPCTRL_buf[64];//0x80500000
    };
    union {
        SYSTEMPORT_SYSBUSCFG SYSTEMPORT_SYSBUSCFG;
        uint8 SYSTEMPORT_SYSBUSCFG_buf[704];//0x80500040
    };
    union {
        SYSTEMPORT_RXCHK SYSTEMPORT_RXCHK;
        uint8 SYSTEMPORT_RXCHK_buf[256];//0x80500300
    };
    union {
        SYSTEMPORT_RBUF SYSTEMPORT_RBUF;
        uint8 SYSTEMPORT_RBUF_buf[512];//0x80500400
    };
    union {
        SYSTEMPORT_TBUF SYSTEMPORT_TBUF;
        uint8 SYSTEMPORT_TBUF_buf[512];//0x80500600
    };
    union {
        SYSTEMPORT_UNIMAC SYSTEMPORT_UNIMAC;
        uint8 SYSTEMPORT_UNIMAC_buf[1024];//0x80500800
    };
    union {
        SYSTEMPORT_MIB_COUNTERS SYSTEMPORT_MIB_COUNTERS;
        uint8 SYSTEMPORT_MIB_COUNTERS_buf[512];//0x80500c00
    };
    union {
        SYSTEMPORT_MPD SYSTEMPORT_MPD;
        uint8 SYSTEMPORT_MPD_buf[4608];//0x80500e00
    };
    union {
        SYSTEMPORT_RDMA SYSTEMPORT_RDMA;
        uint8 SYSTEMPORT_RDMA_buf[8192];//0x80502000
    };
    union {
        SYSTEMPORT_TDMA SYSTEMPORT_TDMA;
        uint8 SYSTEMPORT_TDMA_buf[8192];//0x80504000
    };
    union {
        SYSTEMPORT_SPE SYSTEMPORT_SPE;
        uint8 SYSTEMPORT_SPE_buf[8704];//0x80506000
    };
    union {
        SYSTEMPORT_INTRL2 SYSTEMPORT_INTRL2[5];
        uint8 SYSTEMPORT_INTRL_0_buf[256];//0x80508200
    };
    union {
        SYSTEMPORT_INTC SYSTEMPORT_INTC[5];
        uint8 SYSTEMPORT_INTC_1_buf[256];//0x80508300
    };
    union {
        SYSTEMPORT_INTRL2 SYSTEMPORT_INTRL2_MISC_RX;
        uint8 SYSTEMPORT_INTRL_MISC_RX_buf[256];//0x80508400
    };
    union {
        SYSTEMPORT_INTRL2 SYSTEMPORT_INTRL2_MISC_TX;
        uint8 SYSTEMPORT_INTRL_MISC_TX_buf[31488];//0x80508500
    };
    union {
        SYSTEMPORT_LED_REG SYSTEMPORT_LED_REG;
        uint8 SYSTEMPORT_LED_REG_buf[1280];//0x80510000
    };
    union {
        SYSTEMPORT_X_INTRL2_PHY SYSTEMPORT_1_INTRL2_PHY;
        uint8 SYSTEMPORT_1_INTRL2_PHY_buf[2816];//0x80510500
    };
    union {
        SYSTEMPORT_MISC SYSTEMPORT_MISC;
        uint8 SYSTEMPORT_MISC_buf[768];//0x80511000
    };
    union {
        //SYSTEMPORT_MDIO SYSTEMPORT_MDIO;
        uint8 SYSTEMPORT_MDIO_buf[16];//0x80511300
    };
} sysport;

#define SYSPORT(base_num)    ((volatile sysport* const) SYSPORT_##base_num##_BASE)

#define SWITCH_CORE_BASE (NULL)

#endif

#ifdef __cplusplus
}
#endif

#endif

