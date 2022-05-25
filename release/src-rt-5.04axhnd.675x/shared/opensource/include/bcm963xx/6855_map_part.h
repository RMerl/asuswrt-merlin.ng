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

#ifndef __BCM6855_MAP_PART_H
#define __BCM6855_MAP_PART_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bcmtypes.h"
#include "bcm_io_map.h"

#define CHIP_FAMILY_ID_HEX	        0x6855

#ifndef __ASSEMBLER__
typedef enum 
{
    BIUCFG_IDX,
    MEMC_IDX,
    XRDP_IDX,
    PERF_IDX,
    PERF1_IDX,
    PCM_IDX,
    PCMBUS_IDX,
#if defined(_ATF_)
    PMC_IDX,
#endif
    BOOTLUT_IDX,
    UBUS_MAPPED_IDX,
    MST_PORT_NODE_B53_IDX,
    MST_PORT_NODE_PER_IDX,
    MST_PORT_NODE_USB_IDX,
    MST_PORT_NODE_WIFI_IDX,
    MST_PORT_NODE_PCIE0_IDX,
    MST_PORT_NODE_DMA0_IDX,
    MST_PORT_NODE_DMA1_IDX,
    MST_PORT_NODE_DMA2_IDX,
    MST_PORT_NODE_QM_IDX,
    MST_PORT_NODE_RQ0_IDX,
    MST_PORT_NODE_RQ1_IDX,
    UBUS4_COHERENCY_PORT_IDX,
    WAN_IDX,
    NGPON2_IDX,
    USBH_IDX,
    //PCMBUS_IDX,
    CCI500_IDX,
    LAST_IDX
} BCM_IO_MAP_IDX;
#endif

#define BIUCFG_PHYS_BASE            0x81060000
#define BIUCFG_SIZE                 0x3000
#define MEMC_PHYS_BASE              0x80180000
#define MEMC_SIZE                   0x24000
#define UBUS_MAPPED_PHYS_BASE       0x83000000
#define UBUS_MAPPED_SIZE            0x1000
#define PCIE0_PHYS_BASE             0x80040000
#define PCIE0_SIZE                  0x0000A000
#define PCIE1_PHYS_BASE             0x80050000
#define PCIE1_SIZE                  0x0000A000
#define PERF_PHYS_BASE              0xff800000
#define PERF_SIZE                   0x14000
#define PERF1_PHYS_BASE             0xff85a000
#define PERF1_SIZE                  0x1000
#define PMC_PHYS_BASE               0xffb20000
#define PMC_SIZE                    0x1000
#define PCM_PHYS_BASE               0xff860000
#define APM_CORE_OFFSET             0x00000000
#define PCM_CORE_OFFSET             0x00000C00
#define PCM_DMA_OFFSET              0x00001800
#define PCM_SIZE                    0x2000
#define PCMBUS_PHYS_BASE            0x83010A00
#define PCMBUS_OFFSET               0x00000000
#define PCMBUS_SIZE                 0x100
#define BOOTLUT_PHYS_BASE           0xffff0000
#define BOOTLUT_SIZE                0x1000
#define XRDP_PHYS_BASE              0x82000000
#define XRDP_SIZE                   0x1000000
#define XRDP_LED_REG_OFFSET         0xd98200

#define PCIE0_MEM_PHYS_BASE         0xC0000000
#define PCIE0_MEM_SIZE              0x10000000

#define PCIE1_MEM_PHYS_BASE         0xD0000000
#define PCIE1_MEM_SIZE              0x10000000

#define WLAN0_PHYS_BASE             0x84000000
#define WLAN0_SIZE                  0x01000000

#define CCI500_PHYS_BASE            0x81100000
#define CCI500_SIZE                 0x91000
#define CCI500_OFFSET               0x000

#define UBUS4_COHERENCY_PORT_PHYS_BASE      0x810A0000
#define UBUS4_COHERENCY_PORT_BASE_SIZE      0x1000
#define UBUS4_RANGE_CHK_SETUP_OFFSET        0x0
#define UBUS4_RANGE_CHK_CONFIG_OFFSET       0x310
#define UBUS4_COHERENCY_PORT_CONFIG_OFFSET  0x400

#define TIMR_OFFSET                 0x0400
#define GPIO_OFFSET                 0x0500
#define BROM_OFFSET                 0x0600
#define RNG_OFFSET                  0x0b80
#define HSSPIM_OFFSET               0x1000
#define NAND_REG_OFFSET             0x1800
#define NAND_CACHE_OFFSET           0x1c00
#define NAND_INTR_OFFSET            0x2000
#define MDIO_OFFSET                 0x2060
#define I2S_OFFSET                  0x2080
#define I2C_OFFSET                  0x2100
#define MISC_OFFSET                 0x2600
#define JTAG_OTP_OFFSET             0x2800
#define SIMCARD_OFFSET              0x2900
#define LED_OFFSET                  0x3000
#define SEC_KEY_OBJ_OFFSET          0x4000
#define AHB_CONTROL_OFFSET          0x10300
#define HS_UART_OFFSET              0x10400
#define PL081_DMA_OFFSET            0x11000
#define ARM_UART_OFFSET             0x12000
#define ARM_UART1_OFFSET            0x13000

#define TIMR_PHYS_BASE              (PERF_PHYS_BASE + TIMR_OFFSET)
#define GPIO_PHYS_BASE              (PERF_PHYS_BASE + GPIO_OFFSET)
#define HSSPIM_PHYS_BASE            (PERF_PHYS_BASE + HSSPIM_OFFSET)
#define NAND_REG_PHYS_BASE          (PERF_PHYS_BASE + NAND_REG_OFFSET)
#define NAND_CACHE_PHYS_BASE        (PERF_PHYS_BASE + NAND_CACHE_OFFSET)
#define NAND_INTR_PHYS_BASE         (PERF_PHYS_BASE + NAND_INTR_OFFSET)
#define MDIO_PHYS_BASE              (PERF_PHYS_BASE + MDIO_OFFSET)
#define I2S_PHYS_BASE               (PERF_PHYS_BASE + I2S_OFFSET)
#define I2C_PHYS_BASE               (PERF_PHYS_BASE + I2C_OFFSET)
#define MISC_PHYS_BASE              (PERF_PHYS_BASE + MISC_OFFSET)
#define JTAG_OTP_PHYS_BASE          (PERF_PHYS_BASE + JTAG_OTP_OFFSET)
#define SIMCARD_PHYS_BASE           (PERF_PHYS_BASE + SIMCARD_OFFSET)
#define LED_PHYS_BASE               (PERF_PHYS_BASE + LED_OFFSET)
#define SEC_KEY_OBJ_BASE            (PERF_PHYS_BASE + SEC_KEY_OBJ_OFFSET)
#define HS_UART_PHYS_BASE           (PERF_PHYS_BASE + HS_UART_OFFSET)
#define PL081_DMA_PHYS_BASE         (PERF_PHYS_BASE + PL081_DMA_OFFSET)
#define ARM_UART_PHYS_BASE          (PERF_PHYS_BASE + ARM_UART_OFFSET)
#define ARM_UART1_PHYS_BASE         (PERF_PHYS_BASE + ARM_UART1_OFFSET)

#define MST_PORT_NODE_PER_PHYS_BASE         0x83010000
#define MST_PORT_NODE_USB_PHYS_BASE         0x83018000
#define MST_PORT_NODE_B53_PHYS_BASE         0x83020000
#define MST_PORT_NODE_PCIE0_PHYS_BASE       0x83030000
#define MST_PORT_NODE_WIFI_PHYS_BASE        0x83050000
#define MST_PORT_NODE_QM_PHYS_BASE          0x83070000
#define MST_PORT_NODE_DMA0_PHYS_BASE        0x83078000
#define MST_PORT_NODE_DMA1_PHYS_BASE        0x8307c000
#define MST_PORT_NODE_DMA2_PHYS_BASE        0x83088000
#define MST_PORT_NODE_RQ0_PHYS_BASE         0x8308c000
#define MST_PORT_NODE_RQ1_PHYS_BASE         0x83090000
#define UBUS_PORT_NODE_DEF_SIZE             0x1000

#define BIUCFG_OFFSET               0x0000

#define TOP_CNTRL_OFFSET            0x0
#define I2C2_OFFSET                 0x800
#define TOP_CNTRL_BASE              (PERF1_PHYS_BASE + TOP_CNTRL_OFFSET)  /* chip top control registers*/
#define I2C2_CNTRL_BASE             (PERF1_PHYS_BASE + I2C2_CNTRL_OFFSET)

#define GIC_PHYS_BASE           0x81000000
#define GIC_SIZE                0x10000
#define GIC_OFFSET              0x0000
#define GICD_OFFSET             0x1000
#define GICC_OFFSET             0x2000

#define PMB_OFFSET              0x100

#define SYS_CLK_CTRL_OFFSET                 0x80
#define SYS_ERR_PORT_CFG_OFFSET             0x100
#define UBUS_SYS_MODULE_REGISTRATION_OFFSET 0x200

#define WAN_PHYS_BASE           0x828ba000
#define WAN_SIZE                0x1000
#define NGPON2_PHYS_BASE        0x828bc000 
#define NGPON2_SIZE             0xe000

#define WAN_BASE                    BCM_IO_MAP(WAN_IDX, WAN_PHYS_BASE, 0)
#define NGPON2_BASE                 BCM_IO_MAP(NGPON2_IDX, NGPON2_PHYS_BASE, 0)
#if defined(_ATF_)
#define PMC_BASE                    BCM_IO_MAP(PMC_IDX, PMC_PHYS_BASE, 0)
#define PMB_BASE                    BCM_IO_MAP(PMC_IDX, PMC_PHYS_BASE, PMB_OFFSET)
#endif
#define MEMC_BASE                   BCM_IO_MAP(MEMC_IDX, MEMC_PHYS_BASE, 0)
#define XRDP_BASE                   BCM_IO_MAP(XRDP_IDX, XRDP_PHYS_BASE, 0)
#define PERF_BASE                   BCM_IO_MAP(PERF_IDX, PERF_PHYS_BASE, 0)
#define TIMR_BASE                   BCM_IO_MAP(PERF_IDX, PERF_PHYS_BASE, TIMR_OFFSET)
#define GPIO_BASE                   BCM_IO_MAP(PERF_IDX, PERF_PHYS_BASE, GPIO_OFFSET)
#define AHB_CONTROL_BASE            BCM_IO_MAP(PERF_IDX, PERF_PHYS_BASE, AHB_CONTROL_OFFSET)
#define HS_UART_BASE                BCM_IO_MAP(PERF_IDX, PERF_PHYS_BASE, HS_UART_OFFSET)
#define PL081_DMA_BASE              BCM_IO_MAP(PERF_IDX, PERF_PHYS_BASE, PL081_DMA_OFFSET)
#define ARM_UART_BASE               BCM_IO_MAP(PERF_IDX, PERF_PHYS_BASE, ARM_UART_OFFSET)
#define ARM_UART1_BASE              BCM_IO_MAP(PERF_IDX, PERF_PHYS_BASE, ARM_UART1_OFFSET)
#define LED_BASE                    BCM_IO_MAP(PERF_IDX, PERF_PHYS_BASE, LED_OFFSET)
#define JTAG_OTP_BASE               BCM_IO_MAP(PERF_IDX, PERF_PHYS_BASE, JTAG_OTP_OFFSET)
#define RNG_BASE                    BCM_IO_MAP(PERF_IDX, PERF_PHYS_BASE, RNG_OFFSET)
#define HSSPIM_BASE                 BCM_IO_MAP(PERF_IDX, PERF_PHYS_BASE, HSSPIM_OFFSET)
#define NAND_REG_BASE               BCM_IO_MAP(PERF_IDX, PERF_PHYS_BASE, NAND_REG_OFFSET)
#define NAND_CACHE_BASE             BCM_IO_MAP(PERF_IDX, PERF_PHYS_BASE, NAND_CACHE_OFFSET)
#define NAND_INTR_BASE              BCM_IO_MAP(PERF_IDX, PERF_PHYS_BASE, NAND_INTR_OFFSET)
#define MDIO_BASE                   BCM_IO_MAP(PERF_IDX, PERF_PHYS_BASE, MDIO_OFFSET)
#define I2C_BASE                    BCM_IO_MAP(PERF_IDX, PERF_PHYS_BASE, I2C_OFFSET)
#define MISC_BASE                   BCM_IO_MAP(PERF_IDX, PERF_PHYS_BASE, MISC_OFFSET)

#define PERF1_BASE                  BCM_IO_MAP(PERF1_IDX, PERF1_PHYS_BASE, 0)
#define TOP_CONTROL_BASE            BCM_IO_MAP(PERF1_IDX, PERF1_PHYS_BASE, TOP_CNTRL_OFFSET)
#define I2C2_BASE                   BCM_IO_MAP(PERF1_IDX, PERF1_PHYS_BASE, I2C2_OFFSET)
#define BOOTLUT_BASE                BCM_IO_MAP(BOOTLUT_IDX, BOOTLUT_PHYS_BASE, 0)

#define APM_BASE                    BCM_IO_MAP(PCM_IDX, PCM_PHYS_BASE, APM_CORE_OFFSET)
#define PCM_BASE                    BCM_IO_MAP(PCM_IDX, PCM_PHYS_BASE, PCM_CORE_OFFSET)
#define PCM_DMA_BASE                BCM_IO_MAP(PCM_IDX, PCM_PHYS_BASE, PCM_DMA_OFFSET)
#define PCM_BUS_BASE                BCM_IO_MAP(PCMBUS_IDX, PCMBUS_PHYS_BASE, PCMBUS_OFFSET)

#define CCI500_BASE                 BCM_IO_MAP(CCI500_IDX, CCI500_PHYS_BASE, CCI500_OFFSET)

#define UNIMAC_BASE         XRDP_BASE + 0x8a8000
#define UNIMAC_CFG_BASE     UNIMAC_BASE
#define UNIMAC_MIB_BASE     XRDP_BASE + 0x8a8400
#define UNIMAC_TOP_BASE     XRDP_BASE + 0x8b0000
#define RGMII_BASE          XRDP_BASE + 0x8a0300
#define QEGPHY_BASE         XRDP_BASE + 0x8ba200

#define WAN_MISC_BASE       XRDP_BASE + 0x8ba000
#define SGMII_BASE          XRDP_BASE + 0x8ba280

#define XRDP_RCQ_COMMON_REGS_RCQ_GENERAL_CONFIG 0xd08500
#define XRDP_RCQ_GEN_CFG       \
        BCM_IO_MAP(XRDP_IDX, XRDP_PHYS_BASE, \
            XRDP_RCQ_COMMON_REGS_RCQ_GENERAL_CONFIG)
#define XRDP_RCQ_GENERAL_DDR_CONGEST_THRESHOLD_SHIFT   4
#define XRDP_RCQ_GENERAL_PSRAM_CONGEST_THRESHOLD_SHIFT 9

#define BIUCFG_BASE                 	BCM_IO_MAP(BIUCFG_IDX, BIUCFG_PHYS_BASE, BIUCFG_OFFSET)
#define UBUS_RANGE_CHK_SETUP_BASE       BCM_IO_MAP(UBUS4_COHERENCY_PORT_IDX,UBUS4_COHERENCY_PORT_PHYS_BASE, UBUS4_RANGE_CHK_SETUP_OFFSET)
#define UBUS_RANGE_CHK_CFG_BASE         BCM_IO_MAP(UBUS4_COHERENCY_PORT_IDX,UBUS4_COHERENCY_PORT_PHYS_BASE, UBUS4_RANGE_CHK_CONFIG_OFFSET)
#define UBUS_COHERENCY_PORT_CFG_BASE    BCM_IO_MAP(UBUS4_COHERENCY_PORT_IDX,UBUS4_COHERENCY_PORT_PHYS_BASE, UBUS4_COHERENCY_PORT_CONFIG_OFFSET)
#define UBUS_COHERENCY_PORT_CFG_LUT_BASE            UBUS_COHERENCY_PORT_CFG_BASE + 0
#define UBUS_COHERENCY_PORT_CFG_DEPTH_BASE          UBUS_COHERENCY_PORT_CFG_BASE + 0x80
#define UBUS_COHERENCY_PORT_CFG_CBS_BASE            UBUS_COHERENCY_PORT_CFG_BASE + 0x90
#define UBUS_COHERENCY_PORT_CFG_CIR_INCR_BASE       UBUS_COHERENCY_PORT_CFG_BASE + 0xB0
#define UBUS_COHERENCY_PORT_CFG_REF_COUNT_BASE      UBUS_COHERENCY_PORT_CFG_BASE + 0xc0

#define UBUS_SYS_MODULE_BASE        BCM_IO_MAP(UBUS_MAPPED_IDX, UBUS_MAPPED_PHYS_BASE, 0)
#define UBUS_SYS_MODULE_REGISTRATION_BASE   BCM_IO_MAP(UBUS_MAPPED_IDX, UBUS_MAPPED_PHYS_BASE, UBUS_SYS_MODULE_REGISTRATION_OFFSET)
#define UBUS_MAPPED_BASE            BCM_IO_MAP(UBUS_MAPPED_IDX, UBUS_MAPPED_PHYS_BASE, SYS_CLK_CTRL_OFFSET)

#define MST_PORT_NODE_B53_BASE          BCM_IO_MAP(MST_PORT_NODE_B53_IDX, MST_PORT_NODE_B53_PHYS_BASE, 0)
#define MST_PORT_NODE_PER_BASE          BCM_IO_MAP(MST_PORT_NODE_PER_IDX, MST_PORT_NODE_PER_PHYS_BASE, 0)
#define MST_PORT_NODE_USB_BASE          BCM_IO_MAP(MST_PORT_NODE_USB_IDX, MST_PORT_NODE_USB_PHYS_BASE, 0)
#define MST_PORT_NODE_WIFI_BASE         BCM_IO_MAP(MST_PORT_NODE_WIFI_IDX, MST_PORT_NODE_WIFI_PHYS_BASE, 0)
#define MST_PORT_NODE_PCIE0_BASE        BCM_IO_MAP(MST_PORT_NODE_PCIE0_IDX, MST_PORT_NODE_PCIE0_PHYS_BASE, 0)
#define MST_PORT_NODE_DMA0_BASE         BCM_IO_MAP(MST_PORT_NODE_DMA0_IDX, MST_PORT_NODE_DMA0_PHYS_BASE, 0)
#define MST_PORT_NODE_DMA1_BASE         BCM_IO_MAP(MST_PORT_NODE_DMA1_IDX, MST_PORT_NODE_DMA1_PHYS_BASE, 0)
#define MST_PORT_NODE_DMA2_BASE         BCM_IO_MAP(MST_PORT_NODE_DMA2_IDX, MST_PORT_NODE_DMA2_PHYS_BASE, 0)
#define MST_PORT_NODE_QM_BASE           BCM_IO_MAP(MST_PORT_NODE_QM_IDX, MST_PORT_NODE_QM_PHYS_BASE, 0)
#define MST_PORT_NODE_RQ0_BASE          BCM_IO_MAP(MST_PORT_NODE_RQ0_IDX, MST_PORT_NODE_RQ0_PHYS_BASE, 0)
#define MST_PORT_NODE_RQ1_BASE          BCM_IO_MAP(MST_PORT_NODE_RQ1_IDX, MST_PORT_NODE_RQ1_PHYS_BASE, 0)

#define USBH_PHYS_BASE          0x8000c000
#define USBH_SIZE               0x3fff
#define USBH_OFFSET             0x0000
#define CFG_OFFSET              0x200
#define EHCI_OFFSET             0x300     /* USB host registers */
#define OHCI_OFFSET             0x400     /* USB host registers */
#define EHCI1_OFFSET            0x500     /* EHCI1 host registers */
#define OHCI1_OFFSET            0x600     /* OHCI1 host registers */
#define XHCI_OFFSET             0x1000    /* USB host registers */
#define XHCI_EC_OFFSET          0x1900    /* USB host registers */

/* to support non-DT pltaform device add below defs */
#define USB_EHCI_PHYS_BASE      (USBH_PHYS_BASE+EHCI_OFFSET)
#define USB_OHCI_PHYS_BASE      (USBH_PHYS_BASE+OHCI_OFFSET)
#define USB_EHCI1_PHYS_BASE     (USBH_PHYS_BASE+EHCI1_OFFSET)
#define USB_OHCI1_PHYS_BASE     (USBH_PHYS_BASE+OHCI1_OFFSET)
#define USB_XHCI_PHYS_BASE      (USBH_PHYS_BASE+XHCI_OFFSET)

#define USBH_BASE                       BCM_IO_MAP(USBH_IDX, USBH_PHYS_BASE, USBH_OFFSET)
#define USBH_CFG_BASE                   BCM_IO_MAP(USBH_IDX, USBH_PHYS_BASE, CFG_OFFSET)
#define USBH_EHCI_BASE                  BCM_IO_MAP(USBH_IDX, USBH_PHYS_BASE, EHCI_OFFSET)
#define USBH_OHCI_BASE                  BCM_IO_MAP(USBH_IDX, USBH_PHYS_BASE, OHCI_OFFSET)
#define USBH_XHCI_BASE                  BCM_IO_MAP(USBH_IDX, USBH_PHYS_BASE, XHCI_OFFSET)
#define USBH_XHCI_EC_BASE               BCM_IO_MAP(USBH_IDX, USBH_PHYS_BASE, XHCI_EC_OFFSET)
#define USB_XHCI_BASE USBH_XHCI_BASE


/* These block uses DT or not used by linux at all, no need to map for the legacy support */
#define GIC_BASE           BCM_IO_NOMAP(-1, GIC_PHYS_BASE, GIC_OFFSET)
#define GICC_BASE          BCM_IO_NOMAP(-1, GIC_PHYS_BASE, GICC_OFFSET)
#define GICD_BASE          BCM_IO_NOMAP(-1, GIC_PHYS_BASE, GICD_OFFSET)

#ifndef __ASSEMBLER__
#ifdef __BOARD_DRV_ARMV7__
BCM_IO_BLOCKS bcm_io_blocks[] =  
{
    {BIUCFG_IDX, BIUCFG_SIZE, BIUCFG_PHYS_BASE}, 
    {MEMC_IDX, MEMC_SIZE, MEMC_PHYS_BASE},
    {XRDP_IDX, XRDP_SIZE, XRDP_PHYS_BASE},
    {PERF_IDX, PERF_SIZE, PERF_PHYS_BASE},
    {PERF1_IDX, PERF1_SIZE, PERF1_PHYS_BASE},
    {PCM_IDX, PCM_SIZE, PCM_PHYS_BASE},
    {PCMBUS_IDX, PCMBUS_SIZE, PCMBUS_PHYS_BASE},
#if defined(_ATF_)
    {PMC_IDX, PMC_SIZE, PMC_PHYS_BASE},   
#endif
    {BOOTLUT_IDX, BOOTLUT_SIZE, BOOTLUT_PHYS_BASE},
    {UBUS_MAPPED_IDX, UBUS_MAPPED_SIZE, UBUS_MAPPED_PHYS_BASE},
    {MST_PORT_NODE_B53_IDX, UBUS_PORT_NODE_DEF_SIZE, MST_PORT_NODE_B53_PHYS_BASE},
    {MST_PORT_NODE_PER_IDX, UBUS_PORT_NODE_DEF_SIZE, MST_PORT_NODE_PER_PHYS_BASE},
    {MST_PORT_NODE_USB_IDX, UBUS_PORT_NODE_DEF_SIZE, MST_PORT_NODE_USB_PHYS_BASE},
    {MST_PORT_NODE_WIFI_IDX, UBUS_PORT_NODE_DEF_SIZE, MST_PORT_NODE_WIFI_PHYS_BASE},
    {MST_PORT_NODE_PCIE0_IDX, UBUS_PORT_NODE_DEF_SIZE, MST_PORT_NODE_PCIE0_PHYS_BASE},
    {MST_PORT_NODE_DMA0_IDX, UBUS_PORT_NODE_DEF_SIZE, MST_PORT_NODE_DMA0_PHYS_BASE},
    {MST_PORT_NODE_DMA1_IDX, UBUS_PORT_NODE_DEF_SIZE, MST_PORT_NODE_DMA1_PHYS_BASE},
    {MST_PORT_NODE_DMA2_IDX, UBUS_PORT_NODE_DEF_SIZE, MST_PORT_NODE_DMA2_PHYS_BASE},
    {MST_PORT_NODE_QM_IDX, UBUS_PORT_NODE_DEF_SIZE, MST_PORT_NODE_QM_PHYS_BASE},
    {MST_PORT_NODE_RQ0_IDX, UBUS_PORT_NODE_DEF_SIZE, MST_PORT_NODE_RQ0_PHYS_BASE},
    {MST_PORT_NODE_RQ1_IDX, UBUS_PORT_NODE_DEF_SIZE, MST_PORT_NODE_RQ1_PHYS_BASE},
    {UBUS4_COHERENCY_PORT_IDX, UBUS4_COHERENCY_PORT_BASE_SIZE, UBUS4_COHERENCY_PORT_PHYS_BASE},
    {WAN_IDX, WAN_SIZE, WAN_PHYS_BASE},
    {NGPON2_IDX, NGPON2_SIZE, NGPON2_PHYS_BASE},
    {USBH_IDX, USBH_SIZE, USBH_PHYS_BASE},
    {CCI500_IDX, CCI500_SIZE, CCI500_PHYS_BASE},
};


unsigned long bcm_io_block_address[LAST_IDX];
#else
extern BCM_IO_BLOCKS bcm_io_blocks[];
extern unsigned long bcm_io_block_address[];
#endif


//#define HS_UART ((volatile HsUartCtrlRegs * const) HS_UART_BASE)

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
   uint32 rsrvd2[1];             /* 0x1c */
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
** Peripheral Controller
*/

typedef struct PerfControl { /* GenInt */
     uint32        RevID;             /* (00) word 0 */
#define CHIP_ID_SHIFT   12
#define CHIP_ID_MASK    (0xfffff << CHIP_ID_SHIFT)
#define REV_ID_MASK     0xff
#define EI_MASK_SHFT                12

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
   uint32 reserved[9];        /* 0x4c */  
   uint32 DMAIrqStatus;       /* 0x70 */
   uint32 DMAIrqSet;          /* 0x74 */
   uint32 DMAIrqClear;        /* 0x78 */
   uint32 DMAIrqMaskStatus;   /* 0x7c */
   uint32 DMAIrqMaskSet;      /* 0x80 */
   uint32 DMAIrqMaskClear;    /* 0x84 */
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
#define SW_INI_RESET            0x00000001
    uint32        spare[3];
} Timer;

#define TIMER ((volatile Timer * const) TIMR_BASE)

/*
 * Gpio Controller
 */
typedef struct GpioControl {
        uint32 GPIODir[8];             /* 0x00-0x20 */
        uint32 GPIOio[8];              /* 0x20-0x40 */
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
#define GPIO_NUM_MAX                104 /* accoring to pinmuxing table */
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
#define MISC_STRAP_BUS_BOOTROM_BOOT_N           (0x1 << 12)

#define MISC_STRAP_BUS_PCIE_RC_MODE_SHIFT       6
#define MISC_STRAP_BUS_PCIE_RC_MODE MASK        (1 << MISC_STRAP_BUS_PCIE_SINGLE_LANES_SHIFT) /* 1-root complex  0-end point */
#define MISC_STRAP_GPON_TX_EN_SHIFT             9         
#define MISC_STRAP_GPON_TX_EN_MASK              (1 << MISC_STRAP_GPON_TX_EN_SHIFT)  // 1-Single ended, 0-differential
#define MISC_STRAP_USB_PMU_POLARITY_SHIFT       10
#define MISC_STRAP_USB_PMU_POLARITY_MASK        (1 << MISC_STRAP_USB_PMU_POLARITY_SHIFT)  // 1-active low, 0-active high

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
    uint32 reserved[5];
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

typedef struct MDIOBase {
  uint32 MDIO_PerCmd;     //MDIO Command Register 
  uint32 MDIO_PerCfg;     //MDIO Configuration Register
} MDIOBase;

#define MDIO ((volatile MDIOBase * const) MDIO_BASE)

#if defined(_ATF_)
/*
 * Power Management Control
 */

typedef union
{
    struct
    {
        uint32 propagate_to_err  :  1; // [00:00] -+
        uint32 propagate_slv_err :  1; // [01:01]  | - these are potentially dangerous and MAY cause a system crash
        uint32 pmbus_reset_n     :  1; // [02:02] -+
        uint32 reserved0         :  1; // [03:03]
        uint32 maxPmbIdx         :  3; // [06:04] 0-based (0-7)
        uint32 reserved1         :  1; // [07:07]
        uint32 maxClientId       : 12; // [19:08] 0-based (theoreticaly 0-4095, but code limits this to 256 devices - 0-255)
        uint32 numRegsPerClient  : 10; // [29:20] some power of 2 - number of 32-bit registers in each client (max = 512)
        uint32 startDiscovery    :  1; // [30:30] kicks off H/W discovery of clients and fills in the map (see PMB_REGS below)
        uint32 discoveryBusy     :  1; // [31:31] whether or not H/W discovery is still busy creating the map
    } Bits;
    uint32 Reg32;
} PMB_CONFIG_REG;

typedef union
{
    struct {
        uint32 data      : 16; // [15:00]
        uint32 reserved1 : 16; // [31:16]
    } Bits;
    uint32 Reg32;
} SSBM_data_reg;

typedef union
{
    struct {
        uint32  ssb_addr    : 10; // [09:00]
        uint32  ssb_cmd     :  2; // [11:10]
        uint32  ssb_en      :  1; // [12:12]
        uint32  ssb_add_pre :  1; // [13:13]
        uint32  reserved2   :  1; // [14:14]
        uint32  ssb_start   :  1; // [15:15]
        uint32  reserved1   : 16; // [31:16]
    } Bits;
    uint32 Reg32;
} SSBM_control_reg;

typedef union
{
    struct {
        uint32 busy      :  1; // [00:00]
        uint32 reserved1 : 31; // [31:01]
    } Bits;
    uint32 Reg32;
} SSBM_status_reg;

typedef union
{
    struct {
        uint32 swreg_th_lo : 8; // [07:00]
        uint32 swreg_th_hi : 8; // [15:08]
        uint32 reserved    :16; // [31:16]
    } Bits;
    uint32 Reg32;
} SSBM_SWREG_th_hilo_reg;


typedef union
{
    struct {
        uint32 ssb_lock_addr : 10; // [09:00]
        uint32 lock_bit      :  1; // [10:10]
        uint32 lock_mode     :  1; // [11:11]
        uint32 reserved      : 20; // [31:12]
    } Bits;
    uint32 Reg32;
} SSBM_SWREG_lock_reg;

#define kSSBWrite   0x01
#define kSSBRead    0x02
#define kSSBEn      (1 << 12)
#define kSSBStart   (1 << 15)

typedef struct SSBMaster {
    SSBM_control_reg        ssbmControl;    /* 0x0060 */
    SSBM_data_reg           ssbmWrData;     /* 0x0064 */
    SSBM_data_reg           ssbmRdData;     /* 0x0068 */
    SSBM_status_reg         ssbmStatus;     /* 0x006c */
    SSBM_SWREG_th_hilo_reg  ssbmThHiLo;     /* 0x0070 */
    SSBM_SWREG_lock_reg     ssbmSwLock;     /* 0x0074 */
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
    PMB_CONFIG_REG config;          /* 0x0100 */
    uint32 arbiter;         /* 0x0104 */
    uint32 timeout;         /* 0x0108 */
    uint32 unused1;         /* 0x010c */
    keyholeReg keyhole[4];  /* 0x0110-0x014f */
    uint32 unused2[44];     /* 0x0150-0x01ff */
    uint32 map[64];         /* 0x0200-0x02ff */ 
}PmbBus;

typedef struct Pmc {
    PmmReg  pmm;                        /* 0x20000 */
    uint32 unused11[22];                /* 0x20008-0x2005f */
    SSBMaster ssbMasterCtrl;            /* 0x20060-0x20077 */
    uint32 unused12[34];                /* 0x20078-0x200ff */
    PmbBus pmb;                         /* 0x20100 */
} Pmc;
#define PMC ((volatile Pmc * const) PMC_BASE)

typedef struct
{
    uint32  control;
#define PMC_PMBM_START                  (1 << 31)
#define PMC_PMBM_TIMEOUT                (1 << 30)
#define PMC_PMBM_SLAVE_ERR              (1 << 29)
#define PMC_PMBM_BUSY                   (1 << 28)
#define PMC_PMBM_BUS_SHIFT              (20)
#define PMC_PMBM_Read                   (0 << 24)
#define PMC_PMBM_Write                  (1 << 24)
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
#endif
// FIXMET
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

#define SEC_KEY_OBJ_KEY0_STATUS_OFFSET  0x4 
#define SEC_KEY_OBJ_KEY0_STATUS_MASK    0x1 
#define SEC_KEY_OBJ_KEY0_STATUS_SHIFT   31
 
#define JTAG_OTP ((volatile Jtag_Otp * const) JTAG_OTP_BASE)

#define BTRM_OTP_READ_TIMEOUT_CNT       0x10000

/* row 8 */
#define OTP_CPU_CORE_CFG_ROW            8
#define OTP_CPU_CORE_CFG_SHIFT          28
#define OTP_CPU_CORE_CFG_MASK           (0x7 << OTP_CPU_CORE_CFG_SHIFT)

#define OTP_SEC_CHIPVAR_ROW         8
#define OTP_SEC_CHIPVAR_SHIFT       24
#define OTP_SEC_CHIPVAR_MASK        (0xf << OTP_SEC_CHIPVAR_SHIFT)

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

/* row 42 */
#define OTP_SKO0_DESC_ROW                     42
#define OTP_SKO0_DESC_ROW_SHIFT               0x0
#define OTP_SKO0_DESC_ROW_MASK                0xffffffff
#define OTP_SKO0_CTL_ROW                      46
#define OTP_SKO0_CTL_ROW_SHIFT                0x0
#define OTP_SKO0_CTL_ROW_MASK                 0xffffffff
#define OTP_SKO0_CRC_ROW                      55
#define OTP_SKO0_CRC_ROW_SHIFT                0x0
#define OTP_SKO0_CRC_ROW_MASK                 0xffffffff
/* row 43 */
#define OTP_SKO1_DESC_ROW                     43
#define OTP_SKO1_DESC_ROW_SHIFT               0x0
#define OTP_SKO1_DESC_ROW_MASK                0xffffffff
#define OTP_SKO1_CTL_ROW                      56
#define OTP_SKO1_CTL_ROW_SHIFT                0x0
#define OTP_SKO1_CTL_ROW_MASK                 0xffffffff
#define OTP_SKO1_CRC_ROW                      64
#define OTP_SKO1_CRC_ROW_SHIFT                0x0
#define OTP_SKO1_CRC_ROW_MASK                 0xffffffff


typedef struct TopControl {
   uint32      MiiPadCtl;          // 0x0
   uint32      DgSensePadCtl;      // 0x4
#define DG_CTRL_SHIFT   4
#define DG_EN_SHIFT     3
#define DG_TRIM_SHIFT   0
   uint32      TpDirOverride0;     // 0x8
   uint32      TpDirOverride1;     // 0xc
   uint32      AnaDetectModeHv;    // 0x10
   uint32      Twop5vDetectCtrl;   // 0x14
   uint32      ResetStatus;        // 0x18
#define PCIE_RESET_STATUS       0x10000000
#define SW_RESET_STATUS         0x20000000
#define HW_RESET_STATUS         0x40000000
#define POR_RESET_STATUS        0x80000000
#define RESET_STATUS_MASK       0xF0000000
   uint32      MdioMasterSelect;   // 0x1c
   uint32      OrionIntPending;    // 0x20
   uint32      Pm2Pm3RescalSts;    // 0x24
   uint32      AdslClkSample;      // 0x28
   uint32      ChipTracking0;      // 0x2c
   uint32      ChipTracking1;      // 0x30
   uint32      OtpChipidLC;        // 0x34
#define CHIP_ID_LC_SIZE         4
#define CHIP_ID_LC_MASK         0x0000000f
   uint32      SwControlReg;       // 0x6c
} TopControl;

#define TOPCTRL ((volatile TopControl * const) TOP_CONTROL_BASE)
#define TOP ((volatile TopControl * const) TOP_CONTROL_BASE)


typedef struct
{
/*SRC_PID_CFG */
#define SRCPID_TO_QUEUE_0_BITS_SHIFT       (0)
#define SRCPID_TO_QUEUE_1_BITS_SHIFT       (4)
#define SRCPID_TO_QUEUE_2_BITS_SHIFT       (8)
#define SRCPID_TO_QUEUE_3_BITS_SHIFT       (12)
#define SRCPID_TO_QUEUE_4_BITS_SHIFT       (16)
#define SRCPID_TO_QUEUE_5_BITS_SHIFT       (20)
#define SRCPID_TO_QUEUE_6_BITS_SHIFT       (24)
#define SRCPID_TO_QUEUE_7_BITS_SHIFT       (28)
#define SRCPID_TO_QUEUE_0                  (1 << SRCPID_TO_QUEUE_0_BITS_SHIFT)
#define SRCPID_TO_QUEUE_1                  (1 << SRCPID_TO_QUEUE_1_BITS_SHIFT)
#define SRCPID_TO_QUEUE_2                  (1 << SRCPID_TO_QUEUE_2_BITS_SHIFT)
#define SRCPID_TO_QUEUE_3                  (1 << SRCPID_TO_QUEUE_3_BITS_SHIFT)
#define SRCPID_TO_QUEUE_4                  (1 << SRCPID_TO_QUEUE_4_BITS_SHIFT)
#define SRCPID_TO_QUEUE_5                  (1 << SRCPID_TO_QUEUE_5_BITS_SHIFT)
#define SRCPID_TO_QUEUE_6                  (1 << SRCPID_TO_QUEUE_6_BITS_SHIFT)
#define SRCPID_TO_QUEUE_7                  (1 << SRCPID_TO_QUEUE_7_BITS_SHIFT)
/* Depth CFG */
#define DEPTH_TO_QUEUE_0_BITS_SHIFT       (0)
#define DEPTH_TO_QUEUE_1_BITS_SHIFT       (8)
#define DEPTH_TO_QUEUE_2_BITS_SHIFT       (16)
#define DEPTH_TO_QUEUE_3_BITS_SHIFT       (24)
#define DEPTH_TO_QUEUE_0                  (1 << DEPTH_TO_QUEUE_0_BITS_SHIFT)
#define DEPTH_TO_QUEUE_1                  (1 << DEPTH_TO_QUEUE_1_BITS_SHIFT)
#define DEPTH_TO_QUEUE_2                  (1 << DEPTH_TO_QUEUE_2_BITS_SHIFT)
#define DEPTH_TO_QUEUE_3                  (1 << DEPTH_TO_QUEUE_3_BITS_SHIFT)
/* CBS Thresh */
#define THRESH_TO_QUEUE_0_BITS_SHIFT       (0)
#define THRESH_TO_QUEUE_1_BITS_SHIFT       (16)
#define THRESH_TO_QUEUE_0                  (1 << THRESH_TO_QUEUE_0_BITS_SHIFT)
#define THRESH_TO_QUEUE_1                  (1 << THRESH_TO_QUEUE_1_BITS_SHIFT)
/* Cir Inc CFG */
#define CIR_INCR_TO_QUEUE_0_BITS_SHIFT    (0)
#define CIR_INCR_TO_QUEUE_1_BITS_SHIFT    (8)
#define CIR_INCR_TO_QUEUE_2_BITS_SHIFT    (16)
#define CIR_INCR_TO_QUEUE_3_BITS_SHIFT    (24)
#define CIR_INCR_TO_QUEUE_0               (1 << CIR_INCR_TO_QUEUE_0_BITS_SHIFT)
#define CIR_INCR_TO_QUEUE_1               (1 << CIR_INCR_TO_QUEUE_1_BITS_SHIFT)
#define CIR_INCR_TO_QUEUE_2               (1 << CIR_INCR_TO_QUEUE_2_BITS_SHIFT)
#define CIR_INCR_TO_QUEUE_3               (1 << CIR_INCR_TO_QUEUE_3_BITS_SHIFT)
/* Ref Counters */
#define REF_CNT_0_BITS_SHIFT       (0)
#define REF_CNT_1_BITS_SHIFT       (4)
#define REF_CNT_2_BITS_SHIFT       (8)
#define REF_CNT_3_BITS_SHIFT       (12)
#define REF_CNT_4_BITS_SHIFT       (16)
#define REF_CNT_5_BITS_SHIFT       (20)
#define REF_CNT_6_BITS_SHIFT       (24)
#define REF_CNT_7_BITS_SHIFT       (28)
#define REF_CNT_0                  (1 << REF_CNT_0_BITS_SHIFT)
#define REF_CNT_1                  (1 << REF_CNT_1_BITS_SHIFT)
#define REF_CNT_2                  (1 << REF_CNT_2_BITS_SHIFT)
#define REF_CNT_3                  (1 << REF_CNT_3_BITS_SHIFT)
#define REF_CNT_4                  (1 << REF_CNT_4_BITS_SHIFT)
#define REF_CNT_5                  (1 << REF_CNT_5_BITS_SHIFT)
#define REF_CNT_6                  (1 << REF_CNT_6_BITS_SHIFT)
#define REF_CNT_7                  (1 << REF_CNT_7_BITS_SHIFT)
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

#define CohPortCfg ((volatile CoherencyPortCfgReg_t * const) UBUS_COHERENCY_PORT_CFG_BASE)

typedef struct BIUCFG_Access {
    uint32 permission;        /* 0x0 */
    uint32 revd0;             /* 0x4 */
    uint32 cpu_defeature;     /* 0x8 */
    uint32 dbg_security;      /* 0xc */
    uint32 rsvd1[36];         /* 0x10 - 0x9f */
    uint32 ts_access;         /* 0xa0 - 0xa3 */
    uint32 rsvd2[23];         /* 0xa4 - 0xff */
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

typedef struct BIUCFG {
    BIUCFG_Access access;         /* 0x0 - 0xff*/
    BIUCFG_Cluster cluster[2];    /* 0x100 - 0x2ff*/
    uint32 anonymous[2880];       /* 0x300 - 0x2fff*/
}BIUCFG;
#define BIUCFG ((volatile BIUCFG * const) BIUCFG_BASE)

typedef struct Ubus4ModuleClientRegistration {
    uint32 SlvStatus[16];       /* 0x0   */
    uint32 MstStatus[ 8];       /* 0x240 */
    uint32 RegCntl;             /* 0x260 */
    uint32 SlvStopProgDelay;    /* 0x264 */
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

#endif

#ifdef __cplusplus
}
#endif

#endif

