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

#ifndef __BCM6858_MAP_PART_H
#define __BCM6858_MAP_PART_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bcmtypes.h"
#include "bcm_io_map.h"

#define CHIP_FAMILY_ID_HEX	        0x6858

typedef enum 
{
    EPON_IDX,
    WAN_IDX,
    GPON_IDX,
    NGPON2_IDX,
    WANBBH_IDX,
    XRDP_IDX,
    LPORT_IDX,
    MEMC_IDX,
    PMC_IDX,
    PROC_MON_IDX,
    UBUS_MAPPED_IDX,
    UBUS_CAPTURE_PORT_NODE_0_IDX,
    UBUS_CAPTURE_PORT_NODE_1_IDX,
    UBUS_CAPTURE_PORT_NODE_2_IDX,
    UBUS_CAPTURE_PORT_NODE_3_IDX,
    UBUS_MAPPED_XRDP_IDX,
    PERF_IDX,
    USIM_IDX,
    PERF1_IDX,
    PL081_DMA_IDX,
    I2C_2_IDX,
    NANDFLASH_IDX,
    PCM_IDX,
    PCMBUS_IDX,
    USBH_IDX,
    SATA_IDX,
    MST_PORT_NODE_PCIE0_IDX,
    MST_PORT_NODE_PCIE1_IDX,
    MST_PORT_NODE_B53_IDX,
    MST_PORT_NODE_PCIE2_IDX,
    MST_PORT_NODE_SATA_IDX,
    MST_PORT_NODE_USB_IDX,
    MST_PORT_NODE_PMC_IDX,
    MST_PORT_NODE_APM_IDX,
    MST_PORT_NODE_PER_IDX,
    MST_PORT_NODE_DMA0_IDX,
    MST_PORT_NODE_DMA1_IDX,
    MST_PORT_NODE_RQ0_IDX,
    MST_PORT_NODE_RQ1_IDX,
    MST_PORT_NODE_RQ2_IDX,
    MST_PORT_NODE_RQ3_IDX,
    MST_PORT_NODE_NATC_IDX,
    MST_PORT_NODE_DQM_IDX,
    MST_PORT_NODE_QM_IDX,
	UBUS_CAPTURE_0_IDX,
    UBUS_CAPTURE_1_IDX,
    UBUS_CAPTURE_2_IDX,
    UBUS_CAPTURE_3_IDX,
    LAST_IDX
} BCM_IO_MAP_IDX;

#define PCIE0_PHYS_BASE             0x80040000
#define PCIE0_SIZE                  0x0000A000
#define PCIE1_PHYS_BASE             0x80050000
#define PCIE1_SIZE                  0x0000A000
#define PCIE2_PHYS_BASE             0x80060000
#define PCIE2_SIZE                  0x0000A000
#define PCIE0_MEM_PHYS_BASE         0xc0000000
#define PCIE0_MEM_SIZE              0x10000000
#define PCIE1_MEM_PHYS_BASE         0xd0000000
#define PCIE1_MEM_SIZE              0x10000000
#define PCIE2_MEM_PHYS_BASE         0xe0000000
#define PCIE2_MEM_SIZE              0x10000000

#define LPORT_PHYS_BASE             0x80138000
#define LPORT_SIZE                  0x6000
#define EPON_PHYS_BASE              0x80140000
#define EPON_SIZE                   0x4000
#define WAN_PHYS_BASE               0x80144000
#define WAN_SIZE                    0x1000
#define GPON_PHYS_BASE              0x80148000
#define GPON_SIZE                   0x12000
#define NGPON2_PHYS_BASE            0x80160000
#define NGPON2_SIZE                 0xE000
#define WANBBH_PHYS_BASE            0x80170000
#define WANBBH_SIZE                 0x4000
#define MEMC_PHYS_BASE              0x80180000
#define MEMC_SIZE                   0x4000
#define PMC_PHYS_BASE               0x80200000
#define PMC_SIZE                    0x5000
#define PROC_MON_PHYS_BASE          0x80280000
#define PROC_MON_SIZE               0x1000
#define CCI400_PHYS_BASE            0x81090000
#define CCI400_SIZE                 0xe000
#define XRDP_PHYS_BASE              0x82000000
#define XRDP_SIZE                   0xE51000
#define UBUS_MAPPED_PHYS_BASE       0x83000000
#define UBUS_MAPPED_SIZE            0x1000
#define UBUS_CAPTURE_PORT_NODE_0_PHYS_BASE 0x830b8000
#define UBUS_CAPTURE_PORT_NODE_0_SIZE 0x1000 
#define UBUS_CAPTURE_PORT_NODE_1_PHYS_BASE 0x830bc000
#define UBUS_CAPTURE_PORT_NODE_1_SIZE 0x1000
#define UBUS_CAPTURE_PORT_NODE_2_PHYS_BASE 0x830c0000
#define UBUS_CAPTURE_PORT_NODE_2_SIZE 0x1000 
#define UBUS_CAPTURE_PORT_NODE_3_PHYS_BASE 0x834c4000
#define UBUS_CAPTURE_PORT_NODE_3_SIZE 0x1000
#define MST_PORT_NODE_PCIE0_PHYS_BASE       0x8300C000
#define MST_PORT_NODE_PCIE0_SIZE            0x1000
#define MST_PORT_NODE_PCIE1_PHYS_BASE       0x83014000
#define MST_PORT_NODE_PCIE1_SIZE            0x1000
#define MST_PORT_NODE_B53_PHYS_BASE         0x83020000
#define MST_PORT_NODE_B53_SIZE              0x1000
#define MST_PORT_NODE_PCIE2_PHYS_BASE       0x83028000
#define MST_PORT_NODE_PCIE2_SIZE            0x1000
#define MST_PORT_NODE_SATA_PHYS_BASE        0x83030000
#define MST_PORT_NODE_SATA_SIZE             0x1000
#define MST_PORT_NODE_USB_PHYS_BASE         0x83038000
#define MST_PORT_NODE_USB_SIZE              0x1000
#define MST_PORT_NODE_PMC_PHYS_BASE         0x83044000
#define MST_PORT_NODE_PMC_SIZE              0x1000
#define MST_PORT_NODE_APM_PHYS_BASE         0x8304C000
#define MST_PORT_NODE_APM_SIZE              0x1000
#define MST_PORT_NODE_PER_PHYS_BASE         0x83058000
#define MST_PORT_NODE_PER_SIZE              0x1000
#define MST_PORT_NODE_DMA0_PHYS_BASE        0x83464000
#define MST_PORT_NODE_DMA0_SIZE             0x1000
#define MST_PORT_NODE_DMA1_PHYS_BASE        0x83468000
#define MST_PORT_NODE_DMA1_SIZE             0x1000
#define MST_PORT_NODE_RQ0_PHYS_BASE         0x83480000
#define MST_PORT_NODE_RQ0_SIZE              0x1000
#define MST_PORT_NODE_RQ1_PHYS_BASE         0x83488000
#define MST_PORT_NODE_RQ1_SIZE              0x1000
#define MST_PORT_NODE_RQ2_PHYS_BASE         0x83490000
#define MST_PORT_NODE_RQ2_SIZE              0x1000
#define MST_PORT_NODE_RQ3_PHYS_BASE         0x83498000
#define MST_PORT_NODE_RQ3_SIZE              0x1000
#define MST_PORT_NODE_NATC_PHYS_BASE        0x834A0000
#define MST_PORT_NODE_NATC_SIZE             0x1000
#define MST_PORT_NODE_DQM_PHYS_BASE         0x834A4000
#define MST_PORT_NODE_DQM_SIZE              0x1000
#define MST_PORT_NODE_QM_PHYS_BASE          0x834AC000
#define MST_PORT_NODE_QM_SIZE               0x1000
#define UBUS_MAPPED_XRDP_PHYS_BASE  0x834d4000
#define UBUS_MAPPED_XRDP_SIZE       0x1000
#define PERF_PHYS_BASE              0xff800000
#define PERF_SIZE                   0x3000
#define USIM_PHYS_BASE              0xff854000
#define USIM_SIZE                   0x1000
#define PERF1_PHYS_BASE             0xff858000
#define PERF1_SIZE                  0x1000
#define PL081_DMA_PHYS_BASE         0xff859000
#define PL081_DMA_SIZE              0x1000
#define I2C_2_PHYS_BASE             0xff85a800
#define I2C_2_SIZE                  0x0800
#define NANDFLASH_PHYS_BASE         0xffe00000
#define NANDFLASH_SIZE              0x20000 
#define PCM_PHYS_BASE               0x80100000
#define APM_CORE_OFFSET             0x00000000
#define PCM_CORE_OFFSET             0x00000C00
#define PCM_DMA_OFFSET              0x00001800
#define PCM_SIZE                    0x2000
#define PCMBUS_PHYS_BASE            0x8304c600
#define PCMBUS_OFFSET               0x00000000
#define PCMBUS_SIZE                 0x1000
#define UBUS_CAPTURE_0_PHYS_BASE    0x830b8000
#define UBUS_CAPTURE_0_SIZE         0x1000
#define UBUS_CAPTURE_1_PHYS_BASE    0x830bc000
#define UBUS_CAPTURE_1_SIZE         0x1000
#define UBUS_CAPTURE_2_PHYS_BASE    0x830c0000
#define UBUS_CAPTURE_2_SIZE         0x1000
#define UBUS_CAPTURE_3_PHYS_BASE    0x834c4000
#define UBUS_CAPTURE_3_SIZE         0x1000

#define TIMR_OFFSET                 0x0400
#define GPIO_OFFSET                 0x0500
#define BROM_OFFSET                 0x0600
#define BROM_SEC_OFFSET             0x0620
#define UART_OFFSET                 0x0640
#define UART1_OFFSET                0x0660
#define LED_OFFSET                  0x0800
#define SOTP_OFFSET                 0x0c00
#define JTAG_OTP_OFFSET             0x0e00
#define JTAG_IOTP_OFFSET            0x0e80
#define HSSPIM_OFFSET               0x1000
#define NAND_REG_OFFSET             0x1800
#define NAND_CACHE_OFFSET           0x1c00
#define NAND_INTR_OFFSET            0x2000
#define MDIO_OFFSET                 0x2060
#define I2S_OFFSET                  0x2080
#define I2C_OFFSET                  0x2100
#define MEM2MEM_OFFSET              0x2200
#define MISC_OFFSET                 0x2600
#define NANDFLASH_OFFSET            0x0000

#define SYS_CLK_CTRL_OFFSET			0x40
#define SYS_ERR_PORT_CFG_OFFSET     0x80

#define TIMR_PHYS_BASE              (PERF_PHYS_BASE + TIMR_OFFSET)
#define GPIO_PHYS_BASE              (PERF_PHYS_BASE + GPIO_OFFSET)
#define BROM_PHYS_BASE              (PERF_PHYS_BASE + BROM_OFFSET)
#define BROM_SEC_PHYS_BASE          (PERF_PHYS_BASE + BROM_SEC_OFFSET)
#define UART_PHYS_BASE              (PERF_PHYS_BASE + UART_OFFSET)
#define UART1_PHYS_BASE             (PERF_PHYS_BASE + UART1_OFFSET)
#define LED_PHYS_BASE               (PERF_PHYS_BASE + LED1_OFFSET)
#define JTAG_OTP_PHYS_BASE          (PERF_PHYS_BASE + JTAG_OTP_OFFSET)
#define JTAG_IOTP_PHYS_BASE         (PERF_PHYS_BASE + JTAG_IOTP_OFFSET)
#define HSSPIM_PHYS_BASE            (PERF_PHYS_BASE + HSSPIM_OFFSET)
#define NAND_REG_PHYS_BASE          (PERF_PHYS_BASE + NAND_REG_OFFSET)
#define NAND_CACHE_PHYS_BASE        (PERF_PHYS_BASE + NAND_CACHE_OFFSET)
#define NAND_INTR_PHYS_BASE         (PERF_PHYS_BASE + NAND_INTR_OFFSET)
#define MDIO_PHYS_BASE              (PERF_PHYS_BASE + MDIO_OFFSET)
#define I2S_PHYS_BASE               (PERF_PHYS_BASE + I2S_OFFSET)
#define I2C_PHYS_BASE               (PERF_PHYS_BASE + I2C_OFFSET)
#define MEM2MEM_PHYS_BASE           (PERF_PHYS_BASE + MEM2MEM_OFFSET)
#define MISC_PHYS_BASE              (PERF_PHYS_BASE + MISC_OFFSET)

#define EMMC_HOST_OFFSET            0x0000 
#define EMMC_TOP_OFFSET             0x0100
#define EMMC_BOOT_OFFSET            0x0200 
#define AHB_CONTROL_OFFSET          0x0300
#define BT_UART_OFFSET              0x0400

#define EMMC_HOST_PHYS_BASE         (PERF1_PHYS_BASE + EMMC_HOST_OFFSET)
#define EMMC_TOP_PHYS_BASE          (PERF1_PHYS_BASE + EMMC_TOP_OFFSET)
#define EMMC_BOOT_PHYS_BASE         (PERF1_PHYS_BASE + EMMC_BOOT_OFFSET)
#define AHB_CONTROL_PHYS_BASE       (PERF1_PHYS_BASE + AHB_CONTROL_OFFSET)
#define BT_UART_PHYS_BASE           (PERF1_PHYS_BASE + BT_UART_OFFSET)

#define SATA_PHYS_BASE          0x80008000
#define SATA_SIZE               0x3fff
#define SATA_OFFSET             0x0000

#define USBH_PHYS_BASE          0x8000c000
#define USBH_SIZE               0x3fff
#define USBH_OFFSET             0x0000
#define CFG_OFFSET              0x200
#define EHCI_OFFSET             0x300     /* EHCI host registers */
#define OHCI_OFFSET             0x400     /* OHCI host registers */
#define EHCI1_OFFSET            0x500     /* EHCI1 host registers */
#define OHCI1_OFFSET            0x600     /* OHCI1 host registers */
#define XHCI_OFFSET             0x1000    /* XHCI host registers */
#define XHCI_EC_OFFSET          0x1900    /* XHCI extended registers */

/* to support non-DT pltaform device add below defs */
#define USB_EHCI_PHYS_BASE      (USBH_PHYS_BASE+EHCI_OFFSET)
#define USB_OHCI_PHYS_BASE      (USBH_PHYS_BASE+OHCI_OFFSET)
#define USB_EHCI1_PHYS_BASE     (USBH_PHYS_BASE+EHCI1_OFFSET)
#define USB_OHCI1_PHYS_BASE     (USBH_PHYS_BASE+OHCI1_OFFSET)
#define USB_XHCI_PHYS_BASE      (USBH_PHYS_BASE+XHCI_OFFSET)

#define LPORT_BASE                  BCM_IO_MAP(LPORT_IDX, LPORT_PHYS_BASE, 0)
#define EPON_BASE                   BCM_IO_MAP(EPON_IDX, EPON_PHYS_BASE, 0)
#define WAN_BASE                    BCM_IO_MAP(WAN_IDX, WAN_PHYS_BASE, 0)
#define GPON_BASE                   BCM_IO_MAP(GPON_IDX, GPON_PHYS_BASE, 0)
#define NGPON2_BASE                 BCM_IO_MAP(NGPON2_IDX, NGPON2_PHYS_BASE, 0)
#define WANBBH_BASE                 BCM_IO_MAP(WANBBH_IDX, WANBBH_PHYS_BASE, 0)
#define XRDP_BASE                   BCM_IO_MAP(XRDP_IDX, XRDP_PHYS_BASE, 0)
#define PMC_BASE                    BCM_IO_MAP(PMC_IDX, PMC_PHYS_BASE, 0)
#define PROC_MON_BASE               BCM_IO_MAP(PROC_MON_IDX, PROC_MON_PHYS_BASE, 0)
#define MEMC_BASE                   BCM_IO_MAP(MEMC_IDX, MEMC_PHYS_BASE, 0)
#define UBUS_MAPPED_BASE            BCM_IO_MAP(UBUS_MAPPED_IDX, UBUS_MAPPED_PHYS_BASE, SYS_CLK_CTRL_OFFSET)
#define UBUS_MAPPED_XRDP_BASE       BCM_IO_MAP(UBUS_MAPPED_XRDP_IDX, UBUS_MAPPED_XRDP_PHYS_BASE, SYS_CLK_CTRL_OFFSET)
#define UBUS_CAPTURE_PORT_NODE_0_BASE    BCM_IO_MAP(UBUS_CAPTURE_PORT_NODE_0_IDX, UBUS_CAPTURE_PORT_NODE_0_PHYS_BASE, 0)
#define UBUS_CAPTURE_PORT_NODE_1_BASE    BCM_IO_MAP(UBUS_CAPTURE_PORT_NODE_1_IDX, UBUS_CAPTURE_PORT_NODE_1_PHYS_BASE, 0)
#define UBUS_CAPTURE_PORT_NODE_2_BASE    BCM_IO_MAP(UBUS_CAPTURE_PORT_NODE_2_IDX, UBUS_CAPTURE_PORT_NODE_2_PHYS_BASE, 0)
#define UBUS_CAPTURE_PORT_NODE_3_BASE    BCM_IO_MAP(UBUS_CAPTURE_PORT_NODE_3_IDX, UBUS_CAPTURE_PORT_NODE_3_PHYS_BASE, 0)
#define UBUS_CAPTURE_ERROR_PORT_CFG_BASE BCM_IO_MAP(UBUS_MAPPED_IDX, UBUS_MAPPED_PHYS_BASE, SYS_ERR_PORT_CFG_OFFSET)
#define PERF_BASE                   BCM_IO_MAP(PERF_IDX, PERF_PHYS_BASE, 0)
#define TIMR_BASE                   BCM_IO_MAP(PERF_IDX, PERF_PHYS_BASE, TIMR_OFFSET)
#define GPIO_BASE                   BCM_IO_MAP(PERF_IDX, PERF_PHYS_BASE, GPIO_OFFSET)
#define BROM_BASE                   BCM_IO_MAP(PERF_IDX, PERF_PHYS_BASE, BROM_OFFSET)
#define BROM_SEC_BASE               BCM_IO_MAP(PERF_IDX, PERF_PHYS_BASE, BROM_SEC_OFFSET)
#define UART_BASE                   BCM_IO_MAP(PERF_IDX, PERF_PHYS_BASE, UART_OFFSET)
#define UART1_BASE                  BCM_IO_MAP(PERF_IDX, PERF_PHYS_BASE, UART1_OFFSET)
#define LED_BASE                    BCM_IO_MAP(PERF_IDX, PERF_PHYS_BASE, LED_OFFSET)
#define SOTP_BASE                   BCM_IO_MAP(PERF_IDX, PERF_PHYS_BASE, SOTP_OFFSET)
#define JTAG_OTP_BASE               BCM_IO_MAP(PERF_IDX, PERF_PHYS_BASE, JTAG_OTP_OFFSET)
#define JTAG_IOTP_BASE              BCM_IO_MAP(PERF_IDX, PERF_PHYS_BASE, JTAG_IOTP_OFFSET)
#define HSSPIM_BASE                 BCM_IO_MAP(PERF_IDX, PERF_PHYS_BASE, HSSPIM_OFFSET)
#define NAND_REG_BASE               BCM_IO_MAP(PERF_IDX, PERF_PHYS_BASE, NAND_REG_OFFSET)
#define NAND_CACHE_BASE             BCM_IO_MAP(PERF_IDX, PERF_PHYS_BASE, NAND_CACHE_OFFSET)
#define NAND_INTR_BASE              BCM_IO_MAP(PERF_IDX, PERF_PHYS_BASE, NAND_INTR_OFFSET)
#define MDIO_BASE                   BCM_IO_MAP(PERF_IDX, PERF_PHYS_BASE, MDIO_OFFSET)
#define I2S_BASE                    BCM_IO_MAP(PERF_IDX, PERF_PHYS_BASE, I2S_OFFSET)
#define I2C_BASE                    BCM_IO_MAP(PERF_IDX, PERF_PHYS_BASE, I2C_OFFSET)
#define MEM2MEM_BASE                BCM_IO_MAP(PERF_IDX, PERF_PHYS_BASE, MEM2MEM_OFFSET)
#define MISC_BASE                   BCM_IO_MAP(PERF_IDX, PERF_PHYS_BASE, MISC_OFFSET)

#define USIM_BASE                   BCM_IO_MAP(USIM_IDX, USIM_PHYS_BASE, 0)

#define PERF1_BASE                  BCM_IO_MAP(PERF1_IDX, PERF1_PHYS_BASE, 0)
#define EMMC_HOST_BASE              BCM_IO_MAP(PERF1_IDX, PERF1_PHYS_BASE, EMMC_HOST_OFFSET)
#define EMMC_TOP_BASE               BCM_IO_MAP(PERF1_IDX, PERF1_PHYS_BASE, EMMC_TOP_OFFSET)
#define EMMC_BOOT_BASE              BCM_IO_MAP(PERF1_IDX, PERF1_PHYS_BASE, EMMC_BOOT_OFFSET)
#define AHB_CONTROL_BASE            BCM_IO_MAP(PERF1_IDX, PERF1_PHYS_BASE, AHB_CONTROL_OFFSET)
#define BT_UART_BASE                BCM_IO_MAP(PERF1_IDX, PERF1_PHYS_BASE, BT_UART_OFFSET)

#define PL081_DMA_BASE              BCM_IO_MAP(PL081_DMA_IDX, PL081_DMA_PHYS_BASE, 0)
#define I2C_2_BASE                  BCM_IO_MAP(I2C_2_IDX, I2C_2_PHYS_BASE, 0)
#define APM_BASE                    BCM_IO_MAP(PCM_IDX, PCM_PHYS_BASE, APM_CORE_OFFSET)
#define PCM_BASE                    BCM_IO_MAP(PCM_IDX, PCM_PHYS_BASE, PCM_CORE_OFFSET)
#define PCM_DMA_BASE                BCM_IO_MAP(PCM_IDX, PCM_PHYS_BASE, PCM_DMA_OFFSET)
#define PCM_BUS_BASE                BCM_IO_MAP(PCMBUS_IDX, PCMBUS_PHYS_BASE, PCMBUS_OFFSET)
#define NANDFLASH_BASE              BCM_IO_MAP(NANDFLASH_IDX, NANDFLASH_PHYS_BASE, NANDFLASH_OFFSET)
#define SATA_BASE                   BCM_IO_MAP(SATA_IDX, SATA_PHYS_BASE, SATA_OFFSET)
#define USBH_BASE                   BCM_IO_MAP(USBH_IDX, USBH_PHYS_BASE, USBH_OFFSET)
#define USBH_CFG_BASE               BCM_IO_MAP(USBH_IDX, USBH_PHYS_BASE, CFG_OFFSET)
#define USBH_EHCI_BASE              BCM_IO_MAP(USBH_IDX, USBH_PHYS_BASE, EHCI_OFFSET)
#define USBH_OHCI_BASE              BCM_IO_MAP(USBH_IDX, USBH_PHYS_BASE, OHCI_OFFSET)
#define USBH_EHCI1_BASE              BCM_IO_MAP(USBH_IDX, USBH_PHYS_BASE, EHCI1_OFFSET)
#define USBH_OHCI1_BASE              BCM_IO_MAP(USBH_IDX, USBH_PHYS_BASE, OHCI1_OFFSET)
#define USBH_XHCI_BASE              BCM_IO_MAP(USBH_IDX, USBH_PHYS_BASE, XHCI_OFFSET)
#define USBH_XHCI_EC_BASE           BCM_IO_MAP(USBH_IDX, USBH_PHYS_BASE, XHCI_EC_OFFSET)

#define MST_PORT_NODE_PCIE0_BASE    BCM_IO_MAP(MST_PORT_NODE_PCIE0_IDX, MST_PORT_NODE_PCIE0_PHYS_BASE, 0)
#define MST_PORT_NODE_PCIE1_BASE    BCM_IO_MAP(MST_PORT_NODE_PCIE1_IDX, MST_PORT_NODE_PCIE1_PHYS_BASE, 0)
#define MST_PORT_NODE_B53_BASE      BCM_IO_MAP(MST_PORT_NODE_B53_IDX, MST_PORT_NODE_B53_PHYS_BASE, 0)
#define MST_PORT_NODE_PCIE2_BASE    BCM_IO_MAP(MST_PORT_NODE_PCIE2_IDX, MST_PORT_NODE_PCIE2_PHYS_BASE, 0)
#define MST_PORT_NODE_SATA_BASE     BCM_IO_MAP(MST_PORT_NODE_SATA_IDX, MST_PORT_NODE_SATA_PHYS_BASE, 0)
#define MST_PORT_NODE_USB_BASE      BCM_IO_MAP(MST_PORT_NODE_USB_IDX, MST_PORT_NODE_USB_PHYS_BASE, 0)
#define MST_PORT_NODE_PMC_BASE      BCM_IO_MAP(MST_PORT_NODE_PMC_IDX, MST_PORT_NODE_PMC_PHYS_BASE, 0)
#define MST_PORT_NODE_APM_BASE      BCM_IO_MAP(MST_PORT_NODE_APM_IDX, MST_PORT_NODE_APM_PHYS_BASE, 0)
#define MST_PORT_NODE_PER_BASE      BCM_IO_MAP(MST_PORT_NODE_PER_IDX, MST_PORT_NODE_PER_PHYS_BASE, 0)
#define MST_PORT_NODE_DMA0_BASE     BCM_IO_MAP(MST_PORT_NODE_DMA0_IDX, MST_PORT_NODE_DMA0_PHYS_BASE, 0)
#define MST_PORT_NODE_DMA1_BASE     BCM_IO_MAP(MST_PORT_NODE_DMA1_IDX, MST_PORT_NODE_DMA1_PHYS_BASE, 0)
#define MST_PORT_NODE_RQ0_BASE      BCM_IO_MAP(MST_PORT_NODE_RQ0_IDX, MST_PORT_NODE_RQ0_PHYS_BASE, 0)
#define MST_PORT_NODE_RQ1_BASE      BCM_IO_MAP(MST_PORT_NODE_RQ1_IDX, MST_PORT_NODE_RQ1_PHYS_BASE, 0)
#define MST_PORT_NODE_RQ2_BASE      BCM_IO_MAP(MST_PORT_NODE_RQ2_IDX, MST_PORT_NODE_RQ2_PHYS_BASE, 0)
#define MST_PORT_NODE_RQ3_BASE      BCM_IO_MAP(MST_PORT_NODE_RQ3_IDX, MST_PORT_NODE_RQ3_PHYS_BASE, 0)
#define MST_PORT_NODE_NATC_BASE     BCM_IO_MAP(MST_PORT_NODE_NATC_IDX, MST_PORT_NODE_NATC_PHYS_BASE, 0)
#define MST_PORT_NODE_DQM_BASE      BCM_IO_MAP(MST_PORT_NODE_DQM_IDX, MST_PORT_NODE_DQM_PHYS_BASE, 0)
#define MST_PORT_NODE_QM_BASE       BCM_IO_MAP(MST_PORT_NODE_QM_IDX, MST_PORT_NODE_QM_PHYS_BASE, 0)
#ifdef __BOARD_DRV_AARCH64__
// add here any legacy driver's (driver that have no device tree node) IO memory to be mapped
BCM_IO_BLOCKS bcm_io_blocks[] = 
{
    {EPON_IDX, EPON_SIZE, EPON_PHYS_BASE},
    {WAN_IDX, WAN_SIZE, WAN_PHYS_BASE},
    {GPON_IDX, GPON_SIZE, GPON_PHYS_BASE},
    {NGPON2_IDX, NGPON2_SIZE, NGPON2_PHYS_BASE},
    {WANBBH_IDX, WANBBH_SIZE, WANBBH_PHYS_BASE},
    {XRDP_IDX, XRDP_SIZE, XRDP_PHYS_BASE},
    {LPORT_IDX, LPORT_SIZE, LPORT_PHYS_BASE},
    {MEMC_IDX, MEMC_SIZE, MEMC_PHYS_BASE},
    {PMC_IDX, PMC_SIZE, PMC_PHYS_BASE},
    {PROC_MON_IDX, PROC_MON_SIZE, PROC_MON_PHYS_BASE},
    {UBUS_MAPPED_IDX, UBUS_MAPPED_SIZE, UBUS_MAPPED_PHYS_BASE},
    {UBUS_MAPPED_XRDP_IDX, UBUS_MAPPED_XRDP_SIZE, UBUS_MAPPED_XRDP_PHYS_BASE},
    {UBUS_CAPTURE_PORT_NODE_0_IDX, UBUS_CAPTURE_PORT_NODE_0_SIZE, UBUS_CAPTURE_PORT_NODE_0_PHYS_BASE},
    {UBUS_CAPTURE_PORT_NODE_1_IDX, UBUS_CAPTURE_PORT_NODE_1_SIZE, UBUS_CAPTURE_PORT_NODE_1_PHYS_BASE},
    {UBUS_CAPTURE_PORT_NODE_2_IDX, UBUS_CAPTURE_PORT_NODE_2_SIZE, UBUS_CAPTURE_PORT_NODE_2_PHYS_BASE},
    {UBUS_CAPTURE_PORT_NODE_3_IDX, UBUS_CAPTURE_PORT_NODE_3_SIZE, UBUS_CAPTURE_PORT_NODE_3_PHYS_BASE},
    {PERF_IDX, PERF_SIZE, PERF_PHYS_BASE},
    {USIM_IDX, USIM_SIZE, USIM_PHYS_BASE},
    {PERF1_IDX, PERF1_SIZE, PERF1_PHYS_BASE},
    {PL081_DMA_IDX, PL081_DMA_SIZE, PL081_DMA_PHYS_BASE},
    {I2C_2_IDX, I2C_2_SIZE, I2C_2_PHYS_BASE},
    {NANDFLASH_IDX, NANDFLASH_SIZE, NANDFLASH_PHYS_BASE},
    {PCM_IDX, PCM_SIZE, PCM_PHYS_BASE},
    {PCMBUS_IDX, PCMBUS_SIZE, PCMBUS_PHYS_BASE},
    {SATA_IDX, SATA_SIZE, SATA_PHYS_BASE},
    {USBH_IDX, USBH_SIZE, USBH_PHYS_BASE},
    {MST_PORT_NODE_PCIE0_IDX, MST_PORT_NODE_PCIE0_SIZE, MST_PORT_NODE_PCIE0_PHYS_BASE},
    {MST_PORT_NODE_PCIE1_IDX, MST_PORT_NODE_PCIE1_SIZE, MST_PORT_NODE_PCIE1_PHYS_BASE},
    {MST_PORT_NODE_B53_IDX, MST_PORT_NODE_B53_SIZE, MST_PORT_NODE_B53_PHYS_BASE},
    {MST_PORT_NODE_PCIE2_IDX, MST_PORT_NODE_PCIE2_SIZE, MST_PORT_NODE_PCIE2_PHYS_BASE},
    {MST_PORT_NODE_SATA_IDX, MST_PORT_NODE_SATA_SIZE, MST_PORT_NODE_SATA_PHYS_BASE},
    {MST_PORT_NODE_USB_IDX, MST_PORT_NODE_USB_SIZE, MST_PORT_NODE_USB_PHYS_BASE},
    {MST_PORT_NODE_PMC_IDX, MST_PORT_NODE_PMC_SIZE, MST_PORT_NODE_PMC_PHYS_BASE},
    {MST_PORT_NODE_APM_IDX, MST_PORT_NODE_APM_SIZE, MST_PORT_NODE_APM_PHYS_BASE},
    {MST_PORT_NODE_PER_IDX, MST_PORT_NODE_PER_SIZE, MST_PORT_NODE_PER_PHYS_BASE},
    {MST_PORT_NODE_DMA0_IDX, MST_PORT_NODE_DMA0_SIZE, MST_PORT_NODE_DMA0_PHYS_BASE},
    {MST_PORT_NODE_DMA1_IDX, MST_PORT_NODE_DMA1_SIZE, MST_PORT_NODE_DMA1_PHYS_BASE},
    {MST_PORT_NODE_RQ0_IDX, MST_PORT_NODE_RQ0_SIZE, MST_PORT_NODE_RQ0_PHYS_BASE},
    {MST_PORT_NODE_RQ1_IDX, MST_PORT_NODE_RQ1_SIZE, MST_PORT_NODE_RQ1_PHYS_BASE},
    {MST_PORT_NODE_RQ2_IDX, MST_PORT_NODE_RQ2_SIZE, MST_PORT_NODE_RQ2_PHYS_BASE},
    {MST_PORT_NODE_RQ3_IDX, MST_PORT_NODE_RQ3_SIZE, MST_PORT_NODE_RQ3_PHYS_BASE},
    {MST_PORT_NODE_NATC_IDX, MST_PORT_NODE_NATC_SIZE, MST_PORT_NODE_NATC_PHYS_BASE},
    {MST_PORT_NODE_DQM_IDX, MST_PORT_NODE_DQM_SIZE, MST_PORT_NODE_DQM_PHYS_BASE},
    {MST_PORT_NODE_QM_IDX, MST_PORT_NODE_QM_SIZE, MST_PORT_NODE_QM_PHYS_BASE},
    {UBUS_CAPTURE_0_IDX, UBUS_CAPTURE_0_SIZE, UBUS_CAPTURE_0_PHYS_BASE},
    {UBUS_CAPTURE_1_IDX, UBUS_CAPTURE_1_SIZE, UBUS_CAPTURE_1_PHYS_BASE},
    {UBUS_CAPTURE_2_IDX, UBUS_CAPTURE_2_SIZE, UBUS_CAPTURE_2_PHYS_BASE},
    {UBUS_CAPTURE_3_IDX, UBUS_CAPTURE_3_SIZE, UBUS_CAPTURE_3_PHYS_BASE}
};
unsigned long bcm_io_block_address[LAST_IDX]; /* FIXME */
#else
extern BCM_IO_BLOCKS bcm_io_blocks[];
extern unsigned long bcm_io_block_address[];
#endif

typedef struct EmmcHostCtrl{
    uint32      emmc_host_sdma;
    uint32      emmc_host_block;
    uint32      emmc_host_argument;           /* 8 */
    uint32      emmc_host_cmd_mode;
    uint32      emmc_host_resp_01;            /* 10 */
    uint32      emmc_host_resp_23;
    uint32      emmc_host_resp_45;
    uint32      emmc_host_resp_67;
    uint32      emmc_host_buffdata;           /* 20 */
    uint32      emmc_host_state;
    uint32      emmc_host_ctrl_set0;
    uint32      emmc_host_ctrl_set1;
    uint32      emmc_host_int_status;         /* 30 */
    uint32      emmc_host_int_stat_ena;
    uint32      emmc_host_int_signal_ena;
    uint32      emmc_host_autocmd12_stat;
    uint32      emmc_host_capable;            /* 40 */
    uint32      emmc_host_capable_1;
    uint32      emmc_host_pwr_capable;
    uint32      emmc_host_pwr_capable_rsvd;
    uint32      emmc_host_force_events;       /* 50 */
    uint32      emmc_host_adma_err_stat;
    uint32      emmc_host_adma_sysaddr_lo;
    uint32      emmc_host_adma_sysaddr_hi;
    uint32      emmc_host_preset_init_default; /* 60 */
    uint32      emmc_host_preset_high_speed;
    uint32      emmc_host_preset_sdr25_50;
    uint32      emmc_host_preset_sdr104_ddr50;
    uint32      emmc_host_boot_timeout;       /* 70 */
    uint32      emmc_host_debug_select;
    uint32      emmc_host_ahb_reserved[26];   /* 78 */
    uint32      emmc_host_shared_bus_ctrl;    /* e0 */
    uint32      emmc_host_ahb_reserved1[3];
    uint32      emmc_host_spi_interrupt;      /* f0 */
    uint32      emmc_host_ahb_reserved2[2];
    uint32      emmc_host_version_stat;       /* fc */
} EmmcHostCtrl;

#define EMMC_HOSTIF ((volatile EmmcHostCtrl * const) EMMC_HOST_BASE)

typedef struct EmmcTopCnfg{
    uint32      emmc_top_cfg_sdio_emmc_ctrl1;
    uint32      emmc_top_cfg_sdio_emmc_ctrl2;
    uint32      emmc_top_cfg_tp_out_sel;
    uint32      emmc_top_cfg_cap_reg_override;
    uint32      emmc_top_cfg_cap_reg0;       /* 10 */
    uint32      emmc_top_cfg_cap_reg1;
    uint32      emmc_top_cfg_preset1;
    uint32      emmc_top_cfg_preset2;
    uint32      emmc_top_cfg_preset3;        /* 20 */
    uint32      emmc_top_cfg_preset4;
    uint32      emmc_top_cfg_sd_clock_delay;
    uint32      emmc_top_cfg_sd_pad_drv;
    uint32      emmc_top_cfg_ip_dly;         /* 30 */
    uint32      emmc_top_cfg_op_dly;
    uint32      emmc_top_cfg_tuning;
    uint32      emmc_top_cfg_volt_ctrl;
    uint32      emmc_top_cfg_debug_tap_dly;  /* 40 */
    uint32      emmc_top_cfg_reserved[4];
    uint32      emmc_top_cfg_sd_pin_sel;     /* 54 */
    uint32      emmc_top_cfg_max_current;    
    uint32      emmc_top_cfg_reserved1[37];   /* 5c */
    uint32      emmc_top_cfg_version;        /* f0 */
    uint32      emmc_top_cfg_reserved2[2];
    uint32      emmc_top_cfg_scratch;        /* fc */
} EmmTopCnfg;

#define EMMC_TOP_CFG ((volatile EmmcTopCnfg * const) EMMC_TOP_BASE)

typedef struct EmmcBoot{
    uint32      emmc_boot_main_ctl;
#define EMMC_BOOT_ENABLE   (1 << 0)   
    uint32      emmc_boot_status;
#define EMMC_BOOT_MODE_MASK (1 << 0)    
    uint32      emmc_boot_version;
    uint32      emmc_boot_reserved;
    uint32      emmc_boot_clk_div;        /* 10 */
    uint32      emmc_boot_reset_cnt; 
    uint32      emmc_boot_ram_fill;
    uint32      emmc_boot_error_addr;
    uint32      emmc_boot_base_addr0;     /* 20 */
    uint32      emmc_boot_base_addr1;
    uint32      emmc_boot_ram_fill_cnt;
    uint32      emmc_boot_data_access_time;
    uint32      emmc_boot_reserved1[3];   /* 30 */
    uint32      emmc_boot_debug;          /* 3c */
} EmmcBoot;

#define EMMC_BOOT ((volatile EmmcBoot * const) EMMC_BOOT_BASE)

typedef struct BootUart{
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
} BootUart;

#define BTUART ((volatile BootUart * const) BT_UART_BASE)

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

typedef struct AHBcontrol{
    uint32      ahb_control;
} AHBcontrol;

#define AHBC ((volatile AHBcontrol * const) AHB_CONTROL_BASE)

/*
** Peripheral Controller
*/

#define IRQ_BITS 64
typedef struct  {
    uint64         IrqMask;
    uint64         ExtIrqMask;
    } IrqControl_t;

typedef struct PerfControl { /* GenInt */
     uint32        RevID;             /* (00) word 0 */
#define CHIP_ID_SHIFT   12
#define CHIP_ID_MASK    (0xffff << CHIP_ID_SHIFT)
#define REV_ID_MASK     0xff

   uint32 Ext1IrqCtrl;   /* 0x04 */
#define EI_CLEAR_SHFT   0
#define EI_SENSE_SHFT   8
#define EI_INSENS_SHFT  16
#define EI_LEVEL_SHFT   24
   uint32 Ext1IrqStatus;     /* 0x08 */
#define EI_MASK_SHFT 16
#define EI_STATUS_SHFT 0
   uint32 ExtIrqCtrl;   /* 0x0c */
   uint32 ExtIrqStatus; /* 0x10 */

   uint32 IrqOutMask;    /* 0x14 */
   uint32 IrqOutMask1;   /* 0x18 */
   uint32 ExtIrqMuxSel0; /* 0x1c */
#define EXT_IRQ_SLOT_SIZE             32
#define EXT_IRQ_MUX_SEL0_SHIFT        5
#define EXT_IRQ_MUX_SEL0_MASK         0x1f
   uint32 ExtIrqMuxSel1;   /* 0x20 */
#define EXT_IRQ_MUX_SEL1_SHIFT        4
#define EXT_IRQ_MUX_SEL1_MASK         0xf
   uint32 IrqPeriphStatus;   /* 0x24 */
   uint32 IrqPeriphMask;     /* 0x28 */
   uint32 reserved[53];  /* 0x2c - 0xfc */  
   uint32 IrqSense[4];   /* 0x100 - 0x10c */
   uint32 IrqMask[4];    /* 0x110 - 0x11c */
   uint32 reserved1[12]; /* 0x120 - 0x14c */
   uint32 IrqStatus[4];  /* 0x150 - 0x15c */

} PerfControl;

#define PERF ((volatile PerfControl * const) PERF_BASE)

/*
** Timer
*/
typedef struct Timer {
    uint32        TimerCtl0;
    uint32        TimerCtl1;
    uint32        TimerCtl2;
    uint32        TimerCtl3;
#define TIMERENABLE     0x80000000
#define RSTCNTCLR       0x40000000
    uint32        TimerCnt0;
    uint32        TimerCnt1;
    uint32        TimerCnt2;
    uint32        TimerCnt3;
#define TIMER_COUNT_MASK    0x3FFFFFFF
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
#define WATCHDOG        0x10

    uint32        WatchDogDefCount;/* Write 0xff00 0x00ff to Start timer
     * Write 0xee00 0x00ee to Stop and re-load default count
     * Read from this register returns current watch dog count
     */
    uint32        WatchDogCtl;

    /* Number of 50-MHz ticks for WD Reset pulse to last */
    uint32        WDResetCount;

    uint32        SoftRst;
#define SOFT_RESET              0x00000001      // 0  
    uint32        ResetStatus; 
#define PCIE_RESET_STATUS       0x10000000
#define SW_RESET_STATUS         0x20000000
#define HW_RESET_STATUS         0x40000000
#define POR_RESET_STATUS        0x80000000
#define RESET_STATUS_MASK       0xF0000000    
    uint32        ResetReason;                                              
#define SW_INI_RESET            0x00000001        
    uint32        spare[3];                                                
} Timer;

#define TIMER ((volatile Timer * const) TIMR_BASE)

/*
** UART
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

/*
 * Gpio Controller
 */
typedef struct GpioControl {
        uint32 GPIODir[10];             /* 0x00-0x24 */
        uint32 GPIOio[10];              /* 0x28-0x4c */
        uint32 PadCtrl;                 /* 0x50 */
        uint32 SpiSlaveCfg;             /* 0x54 */
        uint32 TestControl;             /* 0x58 */
        uint32 TestPortBlockEnMSB;      /* 0x5c */
        uint32 TestPortBlockEnLSB;      /* 0x60 */
        uint32 TestPortBlockDataMSB;    /* 0x64 */
        uint32 TestPortBlockDataLSB;    /* 0x68 */
#define PINMUX_DATA_SHIFT       12
#define PINMUX_0                0
#define PINMUX_1                1
#define PINMUX_2                2
#define PINMUX_3                3
#define PINMUX_4                4
#define PINMUX_5                5
#define PINMUX_6                6
#define PINMUX_7                7
        uint32 TestPortCmd;             /* 0x6c */
#define LOAD_MUX_REG_CMD        0x21
        uint32 DiagReadBack;            /* 0x70 */
        uint32 DiagReadBackHi;          /* 0x74 */
        uint32 GeneralPurpose;          /* 0x78 */
        uint32 spare[3];
} GpioControl;

#define GPIO ((volatile GpioControl * const) GPIO_BASE)

/* Number to mask conversion macro used for GPIODir and GPIOio */
#define GPIO_NUM_MAX                75 /* accoring to pinmuxing table */
#define GPIO_NUM_TO_ARRAY_IDX(X)	(((X & BP_GPIO_NUM_MASK) < GPIO_NUM_MAX) ? (((X & BP_GPIO_NUM_MASK) >> 5) & 0x0f) : (0))
#define GPIO_NUM_TO_MASK(X)         (((X & BP_GPIO_NUM_MASK) < GPIO_NUM_MAX) ? (1 << ((X & BP_GPIO_NUM_MASK) & 0x1f)) : (0))
#define GPIO_NUM_TO_ARRAY_SHIFT(X)  (((X) & BP_GPIO_NUM_MASK) & 0x1f)

/*
** Misc Register Set Definitions.
*/

typedef struct Misc {
    uint32  miscStrapBus;                       /* 0x00 */
#define MISC_STRAP_BUS_BOOT_SEL0_4_SHIFT	    5
#define MISC_STRAP_BUS_BOOT_SEL0_4_MASK			(0x18 << MISC_STRAP_BUS_BOOT_SEL0_4_SHIFT)
#define MISC_STRAP_BUS_BOOT_SEL5_SHIFT          11
#define MISC_STRAP_BUS_BOOT_SEL5_MASK			(0x1 << MISC_STRAP_BUS_BOOT_SEL5_SHIFT)
#define BOOT_SEL5_STRAP_ADJ_SHIFT				(MISC_STRAP_BUS_BOOT_SEL5_SHIFT-MISC_STRAP_BUS_BOOT_SEL0_4_SHIFT)

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
#define BOOT_SEL_STRAP_NAND_2K_PAGE				0x00
#define BOOT_SEL_STRAP_NAND_4K_PAGE	    		0x08
#define BOOT_SEL_STRAP_NAND_8K_PAGE				0x10
#define BOOT_SEL_STRAP_NAND_512B_PAGE			0x18
#define BOOT_SEL_STRAP_SPI_NOR		     	    0x38
#define BOOT_SEL_STRAP_EMMC						0x30
#define BOOT_SEL_STRAP_SPI_NAND					0x28

#define BOOT_SEL_STRAP_BOOT_SEL_MASK		    (0x38)
#define BOOT_SEL_STRAP_PAGE_SIZE_MASK			(0x7)

#define MISC_STRAP_BUS_PCIE_SATA_MASK			(1 << 3)
#define MISC_STRAP_BUS_BOOROM_BOOT_N			(1 << 10 )
#define MISC_STRAP_BUS_B53_NO_BOOT              (1 << 12) /* 1 = PMC boots before B53 */
#define MISC_STRAP_BUS_PMC_ROM_BOOT_SHIFT       13
#define MISC_STRAP_BUS_PMC_ROM_BOOT             (0x1 << MISC_STRAP_BUS_PMC_ROM_BOOT_SHIFT) /* 1 = PMC boot */
#define MISC_STRAP_BUS_CPU_SLOW_FREQ            (1 << 14) /* 1 = Slow 400MHz cpu freq */
#define MISC_STRAP_BUS_PMC_BOOT_AVS		        (1 << 20) /* 1 = PMC run AVS */
#define MISC_STRAP_BUS_PMC_BOOT_FLASH		    (1 << 21) /* 1 = PMC boot from flash */ 
#define MISC_STRAP_BUS_UBUS_FREQ_SHIFT			22
#define MISC_STRAP_BUS_UBUS_FREQ_MASK			(1 << MISC_STRAP_BUS_UBUS_FREQ_SHIFT) /* 1 = 2GHz, 0 = 1GHz */  
#define MISC_STRAP_DDR_16B_EN_SHIFT				26
#define MISC_STRAP_DDR_16B_EN_MASK				(1 << MISC_STRAP_DDR_16B_EN_SHIFT)
#define MISC_STRAP_DDR_DENSITY_SHIFT			15
#define MISC_STRAP_DDR_DENSITY_MASK				(1 << MISC_STRAP_DDR_DENSITY_SHIFT) /* 1 = 4Gb, 0 = 2Gb */

    uint32  miscStrapOverride;                  /* 0x04 */
    uint32  miscSWdebug[6];                     /* 0x08-0x1c */
    uint32  miscWDresetCtrl;                    /* 0x20 */
    uint32  miscSWdebugNW[2];                   /* 0x24-0x28 */
    uint32  miscSoftResetB;                     /* 0x2c */
    uint32  miscPLLstatus;                      /* 0x30 */
    uint32  miscDierevid;                       /* 0x34 */
    uint32  miscSPImasterCtrl;                  /* 0x38 */
    uint32  miscAltBoot;                        /* 0x3c */
    uint32  miscPeriphCtrl;                     /* 0x40 */
    uint32  miscPCIECtrl;                       /* 0x44 */
    uint32  miscAdslClockSample;                /* 0x48 */
    uint32  miscRNGCtrl;                        /* 0x4c */
    uint32  miscMbox0_data;                     /* 0x50 */
    uint32  miscMbox1_data;                     /* 0x54 */
    uint32  miscMbox2_data;                     /* 0x58 */
    uint32  miscMbox3_data;                     /* 0x5c */
    uint32  miscMbox_ctrl;                      /* 0x60 */
    uint32  miscMIIPadCtrl;                     /* 0x64 */
    uint32  miscRGMII1PadCtrl;                  /* 0x68 */
    uint32  miscRGMII2PadCtrl;                  /* 0x6c */
    uint32  miscRGMII3PadCtrl;                  /* 0x70 */
    uint32  miscMIIPullCtrl;                    /* 0x74 */
    uint32  miscRGMII1PullCtrl;                 /* 0x78 */
    uint32  miscRGMII2PullCtrl;                 /* 0x7c */
    uint32  miscRGMII3PullCtrl;                 /* 0x80 */
    uint32  miscWDenReset;                      /* 0x84 */
    uint32  miscBootOverlayEn;                  /* 0x88 */
    uint32  miscSGMIIfiber;                     /* 0x8c */
    uint32  miscUNIMACCtrl;                     /* 0x90 */
    uint32  miscMaskUBUSErr;                    /* 0x94 */
    uint32  miscTOSsync;                        /* 0x98 */
    uint32  miscPM0_1_status;                   /* 0x9c */
    uint32  miscPM2_3_status;                   /* 0xa0 */
    uint32  miscSGB_status;                     /* 0xa4 */
    uint32  miscPM0_1_config;                   /* 0xa8 */
    uint32  miscPM2_3_config;                   /* 0xac */
    uint32  miscSGB_config;                     /* 0xb0 */
    uint32  miscPM0_1_tmon_config;              /* 0xb4 */
    uint32  miscPM2_3_tmon_config;              /* 0xb8 */
    uint32  miscSGB_tmon_config;                /* 0xbc */
    uint32  miscMDIOmasterSelect;               /* 0xc0 */
    uint32  miscUSIMCtrl;                       /* 0xc4 */
    uint32  miscUSIMPadCtrl;                    /* 0xc8 */
    uint32  miscPerSpareReg[3];                 /* 0xcc - 0xd4 */
    uint32  miscDgSensePadCtrl;                 /* 0xd8 */
#define DG_CTRL_SHIFT   4
#define DG_EN_SHIFT     3
#define DG_TRIM_SHIFT   0
    uint32  miscPeriphMiscCtrl;                 /* 0xdc */
    uint32  miscPeriphMiscStat;                 /* 0xe0 */
} Misc;

#define MISC ((volatile Misc * const) MISC_BASE)

#define USIM_CTRL           &MISC->miscUSIMCtrl

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
#define NAND_CACHE_BUFFER ((volatile uint8 * const) NAND_CACHE_BASE);

typedef struct UsimBase {

    uint32 UsimScr;  		//sim Control Register 
    uint32 UsimSsr;  		//sim Status Register 
    uint32 UsimSdr;  		//sim Data Register 
    uint32 UsimSier;  		//siM Interrupt Enable Register 
    uint32 UsimSfcr;  		//siM FIFO Control Register 
    uint32 UsimSecgtr;  	//SIM Extra Character Guard Time Register 
    uint32 UsimStgtr;  		//sIM Turnaround Guard Time Register 
    uint32 UsimSgccr;  		//sIM Generic Counter Compare Register 
    uint32 UsimSgcvr;  		//sIM Generic Counter Value Register 
    uint32 UsimScdr;  		//siM Clock Divide Register 
    uint32 UsimSfdrr;  		//sIM F/D Ratio Register 
    uint32 UsimSesr;  		//siM Extra Sample Register 
    uint32 UsimSimdebug;	//SIM Debug Register 
    uint32 UsimSrtor;  		//sIM Received Time Out Register 
    uint32 UsimReserved[4];                        
    uint32 UsimSipver;  	//SIM Controller IP version - 0x4c 
    uint32 UsimReserved1[4];                        
    uint32 UsimSesdcr;  	//SIM Card Detection and Emergency Shutdown Control Register - 0x60 
    uint32 UsimSesdisr;  	//SIM Card Detection and Emergency Shutdown Interrupt Status Register 
    uint32 UsimScardsr;  	//SIM Card Status Control and Status Register 
    uint32 UsimSldocr;  	//SIM LDO Controler Register 
} UsimBase;
#define USIM ((volatile UsimBase * const) USIM_BASE)


typedef struct MDIOBase {
  uint32 MDIO_PerCmd;     //MDIO Command Register 
  uint32 MDIO_PerCfg;     //MDIO Configuration Register
} MDIOBase;

#define MDIO ((volatile MDIOBase * const) MDIO_BASE)

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

#define I2C   ((volatile I2CControl * const) I2C_2_BASE) // Temporary
#define I2C_2 ((volatile I2CControl * const) I2C_BASE)

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
#define PMC_CTRL_GP_FLASH_BOOT_STALL    0x00000080
	uint32 gpOut;
	uint32 gpIn;
	uint32 gpInIrqMask;
	uint32 gpInIrqStatus;
	/* 0x40 */
	uint32 dmaCtrl;
	uint32 dmaStatus;
	uint32 dma0_3FifoStatus;
	uint32 unused0[3];	                /* 0x4c-0x57 */
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
	uint32 unused1[4];	                /* 0xe4-0xf3 */
	/* 0xf4 */
	uint32 iopPeriphBaseAddr;
	uint32 lfsr;
	uint32 unused2;		                /* 0xfc-0xff */
} PmcCtrlReg;

typedef struct PmcOutFifoReg {
	uint32 msgCtrl;		                /* 0x00 */
	uint32 msgSts;		                /* 0x04 */
	uint32 unused[14];	                /* 0x08-0x3f */
	uint32 msgData[16];	                /* 0x40-0x7c */
} PmcOutFifoReg;

typedef struct PmcInFifoReg {
	uint32 msgCtrl;		                /* 0x00 */
	uint32 msgSts;		                /* 0x04 */
	uint32 unused[13];	                /* 0x08-0x3b */
	uint32 msgLast;		                /* 0x3c */
	uint32 msgData[16];	                /* 0x40-0x7c */
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

typedef struct Pmc {
	uint32 baseReserved;		        /* 0x0000 */
	uint32 unused0[1023];
	PmcCtrlReg ctrl;		            /* 0x1000 */

	PmcOutFifoReg outFifo;		        /* 0x1100 */
	uint32 unused1[32];		            /* 0x1180-0x11ff */
	PmcInFifoReg inFifo;		        /* 0x1200 */
	uint32 unused2[32];		            /* 0x1280-0x12ff */

	PmcDmaReg dma[2];		            /* 0x1300 */
	uint32 unused3[48];		            /* 0x1340-0x13ff */

	PmcTokenReg token;		            /* 0x1400 */
	uint32 unused4[121];		        /* 0x141c-0x15ff */

	PmcPerfPowReg perfPower;	        /* 0x1600 */
	uint32 unused5[47];		            /* 0x1644-0x16ff */
                                        
	uint32 msgId[32];		            /* 0x1700 */
	uint32 unused6[32];		            /* 0x1780-0x17ff */
                                        
	PmcDQMReg dqm;			            /* 0x1800 */
	uint32 unused7[50];		            /* 0x1838-0x18ff */

	PmcCntReg hwCounter;		        /* 0x1900 */
	uint32 unused8[46];		            /* 0x1948-0x19ff */

	PmcDqmQCtrlReg dqmQCtrl[32];	    /* 0x1a00 */
	PmcDqmQDataReg dqmQData[32];	    /* 0x1c00 */
	uint32 unused9[64];		            /* 0x1e00-0x1eff */

	uint32 qStatus[32];		            /* 0x1f00 */
	uint32 unused10[32];		        /* 0x1f80-0x1fff */

	PmcDqmQMibReg qMib;		            /* 0x2000 */
	uint32 unused11[1952];		        /* 0x2180-0x3ffff */

	uint32 sharedMem[8192];		        /* 0x4000-0xbffc */
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

#define RCAL_0P25UM_HORZ                0
#define RCAL_0P25UM_VERT                1
#define RCAL_0P5UM_HORZ                 2
#define RCAL_0P5UM_VERT                 3
#define RCAL_1UM_HORZ                   4
#define RCAL_1UM_VERT                   5
#define PMMISC_RMON_EXT_REG             ((RCAL_1UM_VERT + 1)/2)
#define PMMISC_RMON_VALID_MASK          (0x1<<16)
typedef struct PMMiscControl {
	uint32 gp_out;
	uint32 clock_select;
	uint32 unused[2];
	uint32 misc[4];
} PMMiscControl;

typedef struct PMSSBMasterControl {
	uint32 control;
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
#define PMC_PMBM_START		            (1 << 31)
#define PMC_PMBM_TIMEOUT	            (1 << 30)
#define PMC_PMBM_SLAVE_ERR	            (1 << 29)
#define PMC_PMBM_BUSY		            (1 << 28)
#define PMC_PMBM_Read		            (0 << 20)
#define PMC_PMBM_Write		            (1 << 20)
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
	uint32 MonitorCtrl;		            /* 0x00 */
	uint32 unused0[7];
	PMRingOscillatorControl ROSC;	    /* 0x20 */
	uint32 unused1;
	PMMiscControl Misc;		            /* 0x40 */
	PMSSBMasterControl SSBMaster;	    /* 0x60 */
	uint32 unused2[5];
	PMEctrControl Ectr;		            /* 0x80 */
	uint32 unused3[11];
	PMBMaster PMBM[2];		            /* 0xc0 */
	PMAPVTMONControl APvtmonCtrl;	    /* 0x100 */
	uint32 unused4[9];
	PMUBUSCfg UBUSCfg;		            /* 0x160 */
} ProcessMonitorRegs;

#define PROCMON ((volatile ProcessMonitorRegs * const) PROC_MON_BASE)

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
#define UBUS4XRDPCLK ((volatile Ubus4ClkCtrlCfgRegs * const) UBUS_MAPPED_XRDP_BASE)

/*
 * LedControl Register Set Definitions.
 */
typedef struct LedControl {
	uint32 glbCtrl;		                /* 0x00 */
	uint32 mask;		                /* 0x04 */
	uint32 hWLedEn;		                /* 0x08 */
	uint32 serialLedShiftSel;           /* 0x0c */
	uint32 flashRateCtrl[4];	        /* 0x10-0x1c */
	uint32 brightCtrl[4];	            /* 0x20-0x2c */
	uint32 powerLedCfg;	                /* 0x30 */
	uint32 pledLut[2][16];	            /* 0x34-0x70, 0x74-0xb0 */
	uint32 HwPolarity;	                /* 0xb4 */    
	uint32 SwData;		                /* 0xb8 */
	uint32 SwPolarity;	                /* 0xbc */
	uint32 ParallelLedPolarity;	        /* 0xc0 */
	uint32 SerialLedPolarity;	        /* 0xc4 */
	uint32 HwLedStatus;		            /* 0xc8 */
    uint32 FlashCtrlStatus; 
    uint32 BrtCtrlStatus; 
    uint32 ParallelOutStatus; 
    uint32 SerialRegStatus; 
} LedControl;


#define LED ((volatile LedControl * const) LED_BASE)
#define LED_NUM_LEDS       32
#define LED_NUM_TO_MASK(X)       (1 << ((X) & (LED_NUM_LEDS-1)))


#define GPIO_NUM_TO_LED_MODE_SHIFT(X) \
    ((((X) & BP_GPIO_NUM_MASK) < 8) ? (32 + (((X) & BP_GPIO_NUM_MASK) << 1)) : \
    ((((X) & BP_GPIO_NUM_MASK) - 8) << 1))

typedef struct UBUSInterface {
   uint32 CFG;                /* 0x00 */
   uint32 ESRCID_CFG;         /* 0x04 */
#define UBUSIF_CFG_WRITE_REPLY_MODE_SHIFT   0x0
#define UBUSIF_CFG_WRITE_REPLY_MODE_MASK    (0x1<< UBUSIF_CFG_WRITE_REPLY_MODE_SHIFT)
#define UBUSIF_CFG_WRITE_BURST_MODE_SHIFT   0x1
#define UBUSIF_CFG_WRITE_BURST_MODE_MASK    (0x1<< UBUSIF_CFG_WRITE_BURST_MODE_SHIFT)
#define UBUSIF_CFG_INBAND_ERR_MASK_SHIFT    0x2
#define UBUSIF_CFG_INBAND_ERR_MASK_MASK     (0x1<< UBUSIF_CFG_INBAND_ERR_MASK_SHIFT)
#define UBUSIF_CFG_OOB_ERR_MASK_SHIFT       0x3
#define UBUSIF_CFG_OOB_ERR_MASK_MASK        (0x1<< UBUSIF_CFG_OOB_ERR_MASK_SHIFT)
   uint32 SRC_QUEUE_CTRL[4];  /* 0x08 - 0x14 */
   uint32 REP_ARB_MODE;       /* 0x18 */
#define UBUSIF_REP_ARB_MODE_FIFO_MODE_SHIFT 0x0
#define UBUSIF_REP_ARB_MODE_FIFO_MODE_MASK  (0x1<<UBUSIF_REP_ARB_MODE_FIFO_MODE_SHIFT)
   uint32 SCRATCH;            /* 0x1c */
   uint32 DEBUG_R0;           /* 0x20 */
   uint32 unused[7];          /* 0x24-0x3f */
} UBUSInterface;

typedef struct AXIInterface {
   uint32 CFG;				/* 0x00 */
   uint32 REP_ARB_MODE;		/* 0x04 */
   uint32 SCRATCH;			/* 0x08 */
   uint32 ESRCID_CFG;		/* 0x0c */
   uint32 SRC_QUEUE_CTL[8];	/* 0x10 */
   uint32 DEBUG;            /* 0x30 */
   uint32 unused[3];		/* 0x34-0x3f */
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
} RangeCtrl;

typedef struct SecureRangeCheckers {
   uint32 LOCK;            /* 0x00 */
   uint32 LOG_INFO[3];     /* 0x04 - 0x0c */
   RangeCtrl RANGE_CTRL[8];/* 0x10 - 0xac */
   uint32 unused[20];      /* 0xb0 - 0xff */
} SecureRangeCheckers;

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
   uint32 unused6[147];        /* 0x1b4-0x3ff */
} DDRPhyControl;

typedef struct DDRPhyByteLaneControl {
   uint32 VDL_CTRL_WR[10];     /* 0x00 - 0x27 */
   uint32 VDL_CTRL_RD[22];     /* 0x28 - 0x7f */
   uint32 VDL_CLK_CTRL;        /* 0x80 */
   uint32 VDL_LDE_CTRL;        /* 0x84 */
   uint32 RD_EN_DLY_CYC;       /* 0x88 */
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
#define MEMC_GLB_GCFG_SIZE1_SHIFT          0
#define MEMC_GLB_GCFG_SIZE1_MASK           (0xf<<MEMC_GLB_GCFG_SIZE1_SHIFT)
   uint32 GLB_MRQ_CFG;            /* 0x008 */
   uint32 unused0;                /* 0x00c */
   uint32 GLB_FSBL_STATE;         /* 0x010 */
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
#define DRAM_CFG_DRAMSLEEP                        (1<<11)
#define MEMC_CHN_TIM_DRAM_CFG_2TADDRCMD_SHIFT     10
#define MEMC_CHN_TIM_DRAM_CFG_2TADDRCMD_MASK      (0x1<<MEMC_CHN_TIM_DRAM_CFG_2TADDRCMD_SHIFT)
#define MEMC_CHN_TIM_DRAM_CFG_DRAMTYPE_SHIFT      0
#define MEMC_CHN_TIM_DRAM_CFG_DRAMTYPE_MASK       (0xf<<MEMC_CHN_TIM_DRAM_CFG_DRAMTYPE_SHIFT)
#define MC_DRAM_CFG_DDR3                          0x1
   uint32 CHN_TIM_STAT;           /* 0x238 */
   uint32 CHN_TIM_PERF;           /* 0x23c */
   uint32 unused8[48];           /* 0x240-0x2ff */

   uint32 ARB_CFG;                /* 0x300 */
#define MEMC_ARB_CFG_RR_MODE_SHIFT      0x0
#define MEMC_ARB_CFG_RR_MODE_MASK       (0x1<<MEMC_ARB_CFG_RR_MODE_SHIFT)
#define MEMC_ARB_CFG_BURST_MODE_SHIFT   0x1
#define MEMC_ARB_CFG_BURST_MODE_MASK    (0x1<<MEMC_ARB_CFG_BURST_MODE_SHIFT)
   uint32 ARB_QUE_DIS[2];         /* 0x304 */
   uint32 ARB_SP_SEL[2];          /* 0x30c */
   uint32 ARB_RDWR_QUANTUM;       /* 0x314 */
   uint32 unused9[2];            /* 0x318 - 0x31f */
   uint32 ARB_SP_PRI[9];          /* 0x320 */
   uint32 unused10[3];            /* 0x344-0x34f */
   uint32 ARB_RR_QUANTUM[18];     /* 0x350 */
   uint32 unused11[26];           /* 0x398 - 0x3ff */

   UBUSInterface UBUSIF0;         /* 0x400-0x43f */
   uint32 unused12[16];           /* 0x440-0x47f */

   AXIInterface AXIRIF;			  /* 0x480-0x4bf */
   AXIInterface AXIWIF;			  /* 0x4c0-0x4ff */

   uint32 unused13[192];          /* 0x500-0x7ff */

   EDISEngine EDIS_0;             /* 0x800 */
   EDISEngine EDIS_1;             /* 0x900 */

   uint32 STATS_CTRL;             /* 0xa00 */
   uint32 STATS_TIMER_CFG;        /* 0xa04 */
   uint32 STATS_TIMER_COUNT;      /* 0xa08 */
   uint32 STATS_TOTAL_SLICE;      /* 0xa0c */
   uint32 STATS_TOTAL_PACKET;     /* 0xa10 */
   uint32 STATS_TOTAL_READ_SLICE; /* 0xa14 */
   uint32 STATS_TOTAL_READ_PACKET;/* 0xa18 */
   uint32 STATS_TOTAL_LATENCY;    /* 0xa1c */
   uint32 STATS_SLICE_REORDER;    /* 0xa20 */
   uint32 STATS_TOTAL_DDR_CMD;    /* 0xa24 */
   uint32 STATS_TOTAL_DDR_ACT;    /* 0xa28 */
   uint32 STATS_TOTAL_DDR_RDWR;   /* 0xa2c */
   uint32 STATS_TOTAL_DDR_WRRD;   /* 0xa30 */
   uint32 STATS_ARB_GRANT_MATCH0; /* 0xa34 */
   uint32 STATS_ARB_GRANT_MATCH1; /* 0xa38 */
   uint32 STATS_TOTAL_ARB_GRANT;  /* 0xa3c */
   uint32 STATS_FILTER_CFG_0;     /* 0xa40 */
#define MEMC_STATS_FILTER_CFG_MATCH_SRC_SHIFT  25
#define MEMC_STATS_FILTER_CFG_MATCH_SRC_MASK   (0x1<<MEMC_STATS_FILTER_CFG_MATCH_SRC_SHIFT)
#define MEMC_STATS_FILTER_CFG_MATCH_SRC_EN     0x02000000
#define MEMC_STATS_FILTER_CFG_SRC_ID_SHIFT     16
#define MEMC_STATS_FILTER_CFG_SRC_ID_MASK      (0x1FF<<MEMC_STATS_FILTER_CFG_SRC_ID_SHIFT)
#define MEMC_STATS_FILTER_CFG_MATCH_INTF_SHIFT 0
#define MEMC_STATS_FILTER_CFG_MATCH_INTF_MASK  0x0000FFFF
#define MEMC_STATS_FILTER_CFG_INTF_UBUS0       0x00000001
#define MEMC_STATS_FILTER_CFG_INTF_UBUS1       0x00000002
#define MEMC_STATS_FILTER_CFG_INTF_MCP         0x00000004
#define MEMC_STATS_FILTER_CFG_INTF_EDIS0       0x00000008
#define MEMC_STATS_FILTER_CFG_INTF_EDIS1       0x00000010
   uint32 STATS_PROG0_SLICE;      /* 0xa44 */
   uint32 STATS_PROG0_PACKET;     /* 0xa48 */
   uint32 STATS_PROG0_READ_SLICE; /* 0xa4c */
   uint32 STATS_PROG0_READ_PACKET;/* 0xa50 */
   uint32 STATS_PROG0_LATENCY;    /* 0xa54 */
   uint32 unused14[2];            /* 0xa58 - 0xa5c */
   uint32 STATS_FILTER_CFG_1;     /* 0xa60 */
   uint32 STATS_PROG1_SLICE;      /* 0xa64 */
   uint32 STATS_PROG1_PACKET;     /* 0xa68 */ 
   uint32 STATS_PROG1_READ_SLICE; /* 0xa6c */  
   uint32 STATS_PROG1_READ_PACKET;/* 0xa70 */
   uint32 STATS_PROG1_LATENCY;    /* 0xa74 */ 
   uint32 unused15[2];            /* 0xa78 - 0xa7c */   
   uint32 STATS_FILTER_CFG_2;     /* 0xa80 */
   uint32 STATS_PROG2_SLICE;      /* 0xa84 */
   uint32 STATS_PROG2_PACKET;     /* 0xa88 */ 
   uint32 STATS_PROG2_READ_SLICE; /* 0xa8c */  
   uint32 STATS_PROG2_READ_PACKET;/* 0xa90 */
   uint32 STATS_PROG2_LATENCY;    /* 0xa94 */ 
   uint32 unused16[2];            /* 0xa98 - 0xa9c */
   uint32 STATS_FILTER_CFG_3;     /* 0xaa0 */
   uint32 STATS_PROG3_SLICE;      /* 0xaa4 */
   uint32 STATS_PROG3_PACKET;     /* 0xaa8 */ 
   uint32 STATS_PROG3_READ_SLICE; /* 0xaac */  
   uint32 STATS_PROG3_READ_PACKET;/* 0xab0 */
   uint32 STATS_PROG3_LATENCY;    /* 0xab4 */    
   uint32 unused17[18];           /* 0xab8 - 0xaff */

   uint32 CAP_CAPTURE_CFG;        /* 0xb00 */
   uint32 CAP_TRIGGER_ADDR;       /* 0xb04 */
   uint32 CAP_READ_CTRL;          /* 0xb08 */
   uint32 unused18;               /* 0xb0c */
   uint32 CAP_CAPTURE_MATCH0;     /* 0xb10 */
   uint32 CAP_CAPTURE_MATCH1;     /* 0xb14 */
   uint32 CAP_CAPTURE_MATCH2;     /* 0xb18 */
   uint32 unused19;               /* 0xb1c */
   uint32 CAP_CAPTURE_MASK0;      /* 0xb20 */
   uint32 CAP_CAPTURE_MASK1;      /* 0xb24 */
   uint32 CAP_CAPTURE_MASK2;      /* 0xb28 */
   uint32 unused20;               /* 0xb2c */
   uint32 CAP_TRIGGER_MATCH0;     /* 0xb30 */
   uint32 CAP_TRIGGER_MATCH1;     /* 0xb34 */
   uint32 CAP_TRIGGER_MATCH2;     /* 0xb38 */
   uint32 unused21;               /* 0xb3c */
   uint32 CAP_TRIGGER_MASK0;      /* 0xb40 */
   uint32 CAP_TRIGGER_MASK1;      /* 0xb44 */
   uint32 CAP_TRIGGER_MASK2;      /* 0xb48 */
   uint32 unused22;               /* 0xb4c */
   uint32 CAP_READ_DATA[8];       /* 0xb50-0xb6f */
   uint32 unused23[164];          /* 0xb70-0xdff */

   uint32 SEC_INTR2_CPU_STATUS;   /* 0xe00 */
   uint32 SEC_INTR2_CPU_SET;      /* 0xe04 */
   uint32 SEC_INTR2_CPU_CLEAR;    /* 0xe08 */
   uint32 SEC_INTR2_CPU_MASK_STATUS;  /* 0xe0c */
   uint32 SEC_INTR2_CPU_MASK_SET;  /* 0xe10 */
   uint32 SEC_INTR2_CPU_MASK_CLEAR;   /* 0xe14 */
   uint32 unused24[58];              /* 0xe18-0xeff */

   uint32 UBUSIF0_PERMCTL;			/* 0xf00 */
   uint32 UBUSIF0_ACCCTL;			/* 0xf04 */
   uint32 UBUSIF1_PERMCTL;          /* 0xf08 */
   uint32 UBUSIF1_ACCCTL;           /* 0xf0c */
   uint32 AXIRIF_PERMCTL;           /* 0xf10 */
   uint32 AXIRIF_ACCCTL;			/* 0xf14 */
   uint32 AXIWIF_PERMCTL;           /* 0xf18 */
   uint32 AXIWIF_ACCCTL;            /* 0xf1c */
   uint32 CHNCFG_PERMCTL;           /* 0xf20 */
   uint32 CHNCFG_ACCCTL;            /* 0xf24 */
   uint32 MCCAP_PERMCTL;            /* 0xf28 */
   uint32 MCCAP_ACCCTL;             /* 0xf2c */
   uint32 SCRAM_PERMCTL;            /* 0xf30 */
   uint32 SCRAM_ACCCTL;             /* 0xf34 */
   uint32 RNG_PERMCTL;              /* 0xf38 */
   uint32 RNG_ACCCTL;               /* 0xf3c */
   uint32 RNGCHK_PERMCTL;           /* 0xf40 */
   uint32 RNGCHK_ACCCTL;            /* 0xf44 */
   uint32 unused25[558];            /* 0xf48-0x17ff */

   SecureRangeCheckers SEC_RANGE_CHK; /* 0x1800-0x18ff */
   uint32 unused26[31168];            /* 0x1900-0x1ffff */

   DDRPhyControl PhyControl;                    /* 0x20000 */
   DDRPhyByteLaneControl PhyByteLane0Control;   /* 0x20400 */
   DDRPhyByteLaneControl PhyByteLane1Control;   /* 0x20600 */
   DDRPhyByteLaneControl PhyByteLane2Control;   /* 0x20800 */
   DDRPhyByteLaneControl PhyByteLane3Control;   /* 0x20a00 */
} MEMCControl;

#define MEMC ((volatile MEMCControl * const) MEMC_BASE)

typedef struct Jtag_Otp {
   uint32 ctrl0;                          /* 0x00 */
#define JTAG_OTP_CTRL_ACCESS_MODE         (0x3 << 22)
#define JTAG_OTP_CTRL_PROG_EN             (1 << 21)
#define JTAG_OTP_CTRL_START               (1 << 0)
   uint32 ctrl1;                          /* 0x04 */
#define JTAG_OTP_CTRL_CPU_MODE            (1 << 0)
   uint32 ctrl2;                          /* 0x08 */
   uint32 ctrl3;                          /* 0x0c */
   uint32 ctrl4;                          /* 0x10 */
   uint32 status0;                        /* 0x14 */
   uint32 status1;                        /* 0x18 */
   uint32 status[8];                      /* 0x1c-0x38 */
#define JTAG_OTP_STATUS_1_CMD_DONE        (1 << 1)
} Jtag_Otp;

#define JTAG_OTP ((volatile Jtag_Otp * const) JTAG_OTP_BASE)

typedef struct Jtag_IOtp {
   uint32 ctrl0;           
   uint32 ctrl1;           
   uint32 write_data_0; 
   uint32 write_data_1; 
   uint32 ctrl3;           
   uint32 ctrl4;           
   uint32 read_data_0; 
   uint32 read_data_1; 
   uint32 status1;         
   uint32 status[8];         
} Jtag_IOtp;

#define JTAG_IOTP ((volatile Jtag_IOtp * const) JTAG_IOTP_BASE)

#define BTRM_OTP_READ_TIMEOUT_CNT       0x10000

typedef struct BootBase {
    uint32 general_secbootcfg;
#define BOOTROM_CRC_DONE                (1 << 31)
#define BOOTROM_CRC_FAIL                (1 << 30)
    uint32 general_boot_crc_low;
    uint32 general_boot_crc_high;
} BootBase;

#define BOOTBASE ((volatile BootBase * const) BROM_BASE)

typedef struct BootSec {
    uint32 AccessCtrl;
    uint32 AccessRangeChk[4];
} BootSec;

#define BOOTSECURE ((volatile BootSec * const) BROM_SEC_BASE)

typedef struct BootLookUp {
    uint32      bootlut[8];
    uint32      resetException;         /* 0020 */
    uint32      undefException;
    uint32      swiException;
    uint32      prefetchException;
    uint32      abortException;         /* 0030 */
    uint32      unusedException;
    uint32      irqException;
    uint32      fiqException;
    uint32      permException;          /* 0040 */
} BootLookUp;

#define BOOTLUT ((volatile BootLookUp * const) BOOTLUT_BASE)

typedef struct m2mChannel{
    uint32      src_addr;
    uint32      dst_addr;
    uint32      desc_id;
    uint32      dma_config;
} m2mChannel;

typedef struct m2mChError{
    uint32      ubus_error_debug0;
    uint32      ubus_error_debug1;
} m2mChError;

typedef struct m2mChStop{
    uint32      stop_src_addr;
    uint32      stop_dst_addr;
    uint32      stop_msb_addr;
} m2mChStop;

typedef struct MemToMem {
    uint32      ch_desc_stat[4];
    m2mChannel  channels[4];      
    uint32      int_clear;              /* 50 */
    uint32      control;
    uint32      dma_status;
    uint32      ch_stop;        
    uint32      desc_clear;             /* 60 */
    m2mChError  chn_error[4];
    m2mChStop   chn_stop[4];            /* 84 */
    uint32      chn_status_fifo[4];  
    uint32      spare[3];
    uint32      rev_id;
} MemToMem;

#define MEM2MEM ((volatile MemToMem * const) MEM2MEM_BASE)

/********************** XLMAC Block *************************/
#define GPORT0_SEL_SERDES_0             (1 << 0)            
#define GPORT0_SEL_GPHY_1               (0 << 0)            
#define GPORT1_SEL_SERDES_1             (1 << 1)            
#define GPORT1_SEL_GPHY_2               (0 << 1)            
#define GPORT2_SEL_SERDES_2             (1 << 2)            
#define GPORT2_SEL_GPHY_3               (0 << 2)            
#define GPORT3_SEL_SERDES_3             (1 << 3)            
#define GPORT3_SEL_GPHY_4               (0 << 3)             
#define GPORT4_SEL_SERDES_1             (1 << 4)            
#define GPORT4_SEL_RGMII_0              (0 << 4)             
#define GPORT5_SEL_SERDES_2             (1 << 5)            
#define GPORT5_SEL_RGMII_1              (0 << 5)             
#define GPORT6_SEL_SERDES_3             (1 << 6)            
#define GPORT6_SEL_RGMII_2              (0 << 6)             
#define GPORT7_SEL_SERDES_0             (1 << 7)            

#define LPORT_XLMAC_CORE_0(port)        (LPORT_BASE + (port << 2)) /* port : 0 - 1023 */
#define LPORT_XLMAC_CORE_1(port)        (LPORT_BASE + 0x2000 + (port << 2)) /* port : 0 - 1023 */
#define LPORT_MIB_CORE_0(port)          (LPORT_BASE + 0x1000 + (port << 2)) /* port : 0 - 1023 */
#define LPORT_MIB_CORE_1(port)          (LPORT_BASE + 0x3000 + (port << 2)) /* port : 0 - 1023 */

typedef struct RegSerdes_indir{
    uint32      control;
    uint32      addr;
    uint32      mask;
} RegSerdes_indir;

typedef struct LedLinkSpeed{
    uint32      control;
    uint32      encoding_sel;
    uint32      encoding;
} LedLinkSpeed;

typedef struct RGMIIport{
    uint32      control;
    uint32      status;
    uint32      rx_clock_delay;
} RGMIIport;

typedef struct RGMIIportATE{
    uint32      rx_control;
    uint32      rx_exp_data;
    uint32      rx_status0;
    uint32      rx_status1;
    uint32      tx_control;
    uint32      tx_data0;
    uint32      tx_data1;
    uint32      tx_data2;
} RGMIIportATE;

typedef struct RGMIIportDelay{
    uint32      tx_dly_control0;
    uint32      tx_dly_control1;
    uint32      rx_dly_control0;
    uint32      rx_dly_control1;
    uint32      rx_dly_control2;
} RGMIIportDelay;

typedef struct LanPortReg{
    uint32              control;
    uint32              revision;
    uint32              qegphy_revision;
    uint32              dual_serdes_revision;
    RegSerdes_indir     serdes_0_0;         /* 10 */
    RegSerdes_indir     serdes_0_1;
    RegSerdes_indir     serdes_1_0;
    RegSerdes_indir     serdes_1_1;
    uint32              qegphy_test_cntrl;  /* 40 */
    uint32              qegphy_cntrl;
    uint32              qegphy_status;
    uint32              serdes_0_cntrl;
    uint32              serdes_0_stat;
    uint32              serdes_1_cntrl;
    uint32              serdes_1_stat;
    uint32              reserved[6];        /* 5c */
    LedLinkSpeed        led_link[8];        /* 74 */
    uint32              blink_rate_cntrl;
    uint32              serial_cntrl;
    uint32              refresh_period_cntrl;
    uint32              aggregate_cntrl;
    uint32              aggregate_blink_cntrl;
    uint32              spare_cntrl;
    uint32              r0[5];              /* ec */
    RGMIIport           rgmii_port[3];      /* 100 */
    RGMIIportATE        rgmii_ate[3];       /* 120 */
    RGMIIportDelay      rgmii_delay[3];     /* 184 */
} LanPortReg;

#define LPORT_REG           ((volatile LanPortReg * const)(LPORT_BASE + 0x4000))

typedef struct LanPortXlmac{
    uint32              dir_acc_data_write;
    uint32              dir_acc_data_read;
    uint32              indir_acc_addr_0;
    uint32              indir_acc_data_low_0;
    uint32              indir_acc_data_high_0;
    uint32              indir_acc_addr_1;
    uint32              indir_acc_data_low_1;
    uint32              indir_acc_data_high_1;
    uint32              config;
    uint32              rxerrmask[4];
} LanPortXlmac;

#define LPORT_XLMAC_REG_0   ((volatile LanPortXlmac * const)(LPORT_BASE + 0x5000))
#define LPORT_XLMAC_REG_1   ((volatile LanPortXlmac * const)(LPORT_BASE + 0x5200))

typedef struct LanPortMib{
    uint32              dir_acc_data_write;
    uint32              dir_acc_data_read;
    uint32              indir_acc_addr_0;
    uint32              indir_acc_data_low_0;
    uint32              indir_acc_data_high_0;
    uint32              indir_acc_addr_1;
    uint32              indir_acc_data_low_1;
    uint32              indir_acc_data_high_1;
    uint32              cntrl;
    uint32              eee_pulse_duration;
    uint32              gport_max_pkt[4];
    uint32              ecc_cntrl;
    uint32              force_sb_ecc_err;
    uint32              force_db_ecc_err;
    uint32              rx_mem0_ecc_stat;
    uint32              rx_mem1_ecc_stat;
    uint32              rx_mem2_ecc_stat;
    uint32              rx_mem3_ecc_stat;
    uint32              rx_mem4_ecc_stat;
    uint32              tx_mem0_ecc_stat;
    uint32              tx_mem1_ecc_stat;
    uint32              tx_mem2_ecc_stat;
    uint32              tx_mem3_ecc_stat;
} LanPortMib;

typedef struct L2IntrControl{
  uint32 Intr2CpuStatus;
  uint32 Intr2CpuSet;
  uint32 Intr2CpuClear;
  uint32 Intr2CpuMask_status;
  uint32 Intr2CpuMask_set;
  uint32 Intr2CpuMask_clear;
  uint32 Intr2Status;
  uint32 Intr2Set;
  uint32 Intr2Clear;
  uint32 Intr2Mask_status;
  uint32 Intr2Mask_set;
  uint32 Intr2Mask_clear;
} L2IntrControl;

#define LPORT_MIB_REG_0     ((volatile LanPortMib * const)(LPORT_BASE + 0x5100))
#define LPORT_MIB_REG_1     ((volatile LanPortMib * const)(LPORT_BASE + 0x5300))
 
#define LPORT_MDIO_CMD                                (LPORT_BASE + 0x4000) 
#define LPORT_MDIO_CFG                                (LPORT_BASE + 0x4004) 
#define LPORT_INTR_0 ((volatile L2IntrControl * const)(LPORT_BASE + 0x5500))
#define LPORT_INTR_1 ((volatile L2IntrControl * const)(LPORT_BASE + 0x5600))

typedef struct LanPortMab{
    uint32              cntrl;
    uint32              tx_wrr_cntrl;
    uint32              tx_threshold;
    uint32              link_down_tx_data;
    uint32              status;
} LanPortMab;

#define LPORT_MAB_REG_0     ((volatile LanPortMab * const)(LPORT_BASE + 0x5700))
#define LPORT_MAB_REG_1     ((volatile LanPortMab * const)(LPORT_BASE + 0x5800))

/* row 10 */
#define OTP_SATA_DISABLE_ROW			10
#define OTP_SATA_DISABLE_SHIFT			24
#define OTP_SATA_DISABLE_MASK			(0x1 << OTP_SATA_DISABLE_SHIFT)

/* row 11 */
#define OTP_PMC_BOOT_ROW            11
#define OTP_CHIPID_ROW              11
#define OTP_CHIPID_SHIFT            0
#define OTP_CHIP_ID_MASK            (0xfffff << OTP_CHIPID_SHIFT)
#define OTP_PMC_BOOT_SHIFT          25
#define OTP_PMC_BOOT_MASK           (0x1 << OTP_PMC_BOOT_SHIFT)

/* row 12 */
#define OTP_PCM_DISABLE_ROW			12
#define OTP_PCM_DISABLE_SHIFT			12
#define OTP_PCM_DISABLE_MASK			(0x1 << OTP_PCM_DISABLE_SHIFT)

#define OTP_SEC_CHIPVAR_ROW         12
#define OTP_SEC_CHIPVAR_SHIFT       0
#define OTP_SEC_CHIPVAR_MASK        (0xf << OTP_SEC_CHIPVAR_SHIFT)

/* row 14 */
#define OTP_CPU_CLOCK_FREQ_ROW          14
#define OTP_CPU_CLOCK_FREQ_SHIFT        9
#define OTP_CPU_CLOCK_FREQ_MASK         (0x7 << OTP_CPU_CLOCK_FREQ_SHIFT)

#define OTP_CPU_CORE_CFG_ROW            14
#define OTP_CPU_CORE_CFG_SHIFT          14
#define OTP_CPU_CORE_CFG_MASK           (0x3 << OTP_CPU_CORE_CFG_SHIFT)

/* row 17 */
#define OTP_BRCM_BTRM_BOOT_ENABLE_ROW           17
#define OTP_BRCM_BTRM_BOOT_ENABLE_SHIFT         3
#define OTP_BRCM_BTRM_BOOT_ENABLE_MASK          (1 << OTP_BRCM_BTRM_BOOT_ENABLE_SHIFT)

/* row 18 */
#define OTP_CUST_BTRM_BOOT_ENABLE_ROW           18
#define OTP_CUST_BTRM_BOOT_ENABLE_SHIFT         15
#define OTP_CUST_BTRM_BOOT_ENABLE_MASK          (7 << OTP_CUST_BTRM_BOOT_ENABLE_SHIFT)

/* row 24 */
#define OTP_CUST_MFG_MRKTID_ROW                 24
#define OTP_CUST_MFG_MRKTID_SHIFT               0
#define OTP_CUST_MFG_MRKTID_MASK                (0xffff << OTP_CUST_MFG_MRKTID_SHIFT)

/* SOTP defs */
#define SOTP_OTP_REGION_RD_LOCK                 0x3c

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

/*I2S Block */
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
#define I2S_CLK_PLL              3
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

   uint32 intr;             /* 0x04 Interrupt Register */
#define I2S_DESC_OFF_LEVEL_SHIFT    12
#define I2S_DESC_IFF_LEVEL_SHIFT    8
#define I2S_DESC_LEVEL_MASK         0x0F
#define I2S_DESC_OFF_OVERRUN_INTR   (1 << 3)
#define I2S_DESC_IFF_UNDERRUN_INTR  (1 << 2)
#define I2S_DESC_OFF_INTR           (1 << 1)
#define I2S_DESC_IFF_INTR           (1 << 0)
#define I2S_INTR_MASK               0x0F

   uint32 intr_en;          /* 0x08 Interrupt Enables Register */
#define I2S_DESC_INTR_TYPE_SEL        (1 << 4)
#define I2S_DESC_OFF_OVERRUN_INTR_EN  (1 << 3)
#define I2S_DESC_IFF_UNDERRUN_INTR_EN (1 << 2)
#define I2S_DESC_OFF_INTR_EN          (1 << 1)
#define I2S_DESC_IFF_INTR_EN          (1 << 0)

   uint32 intr_iff_thld;    /* 0x0c Descriptor Input FIFO Interrupt Threshold Register  */
   uint32 intr_off_thld;    /* 0x10 Descriptor Output FIFO Interrupt Threshold Register */
#define I2S_DESC_IFF_INTR_THLD_MASK    0x07

   uint32 desc_iff_addr;    /* 0x14 Descriptor Input FIFO Address  */
   uint32 desc_iff_len;     /* 0x18 Descriptor Input FIFO Length   */
   uint32 desc_off_addr;    /* 0x1c Descriptor Output FIFO Address */
   uint32 desc_off_len;     /* 0x20 Descriptor Output FIFO Length  */
#define I2S_DESC_EOP             (1 << 31)
#define I2S_DESC_FIFO_DEPTH      8
#define I2S_DMA_BUFF_MAX_LEN     0xFFFF
#define I2S_DESC_LEN_MASK        I2S_DMA_BUFF_MAX_LEN

} I2s;

#define I2S ((volatile I2s * const) I2S_BASE)

typedef struct
{
   uint32 base_addr;
   uint32 remap_addr;
   uint32 attributes;
#define DECODE_CFG_PID_B53          2
#define DECODE_CFG_PID_MEMC      1
#define DECODE_CFG_SIZE_SHIFT       8
#define DECODE_CFG_ENABLE_SHIFT     17
#define DECODE_CFG_ENABLE_OFF       (0<<DECODE_CFG_ENABLE_SHIFT)
#define DECODE_CFG_ENABLE_ADDR_ONLY (1<<DECODE_CFG_ENABLE_SHIFT)
#define DECODE_CFG_ENABLE_CDSM_0    (2<<DECODE_CFG_ENABLE_SHIFT)
#define DECODE_CFG_ENABLE_CDSM_1    (3<<DECODE_CFG_ENABLE_SHIFT)
#define DECODE_CFG_CACHE_BITS_SHIFT 19
#define DECODE_CFG_CACHE_BITS       (1<<DECODE_CFG_CACHE_BITS_SHIFT)
} DecodeCfgMstWndRegs;

typedef struct
{
   uint32 ctrl;
   uint32 cache_cfg;
   uint32 reserved[2];
   DecodeCfgMstWndRegs window[4];
} DecodeCfgRegs;

typedef struct
{
   uint32 port_cfg[8];
   uint32 reserved1[56];
   uint32 loopback[20];
   uint32 reserved2[44];
   uint32 routing_addr[128];
   uint32 token[128];
   DecodeCfgRegs decode_cfg;
} MstPortNode;

/* CCI-400 Reg Defines */
#define CCI400_CONTROL_OVERRIDE_REG_OFFSET 0
#define CCI400_CONTROL_OVERRIDE_SNOOP_ENABLE_OFFSET  (1<<0)
#define CCI400_SECURE_ACCESS_REG_OFFSET 8
#define CCI400_SECURE_ACCESS_UNSECURE_ENABLE  (1<<0)
#define CCI400_S2_SAHREABLE_OVERRIDE_REG_OFFSET 0x3004
#define CCI400_S2_SAHREABLE_OVERRIDE_AX_DOMAIN 0x3
#define CCI400_S3_SNOOP_CTRL_REG_OFFSET 0x4000
#define CCI400_S3_SNOOP_CTRL_EN_SNOOP (1<<0)



#ifdef __cplusplus
}
#endif

#endif

