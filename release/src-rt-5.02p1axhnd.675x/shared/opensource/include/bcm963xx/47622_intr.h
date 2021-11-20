/*
<:copyright-BRCM:2012:DUAL/GPL:standard 

   Copyright (c) 2012 Broadcom 
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

#ifndef __47622_INTR_H
#define __47622_INTR_H
#ifdef __cplusplus
    extern "C" {
#endif

/*=====================================================================*/
/* SPI Table Offset                                                    */
/*=====================================================================*/
#define SPI_TABLE_OFFSET               32

/*=====================================================================*/
/* Physical Interrupt IDs                                              */
/*=====================================================================*/
    /*  ------------- CHIP_IRQS[31-0] ----------------------------*/
#define INTERRUPT_A7_AXIERR         (SPI_TABLE_OFFSET + 0)
#define INTERRUPT_A7_INTER          (SPI_TABLE_OFFSET + 1)
#define INTERRUPT_A7_CCIERR         (SPI_TABLE_OFFSET + 2)
#define INTERRUPT_A7_CCIOVFLOW      (SPI_TABLE_OFFSET + 3)
#define INTERRUPT_THERM_HIGH        (SPI_TABLE_OFFSET + 4)
#define INTERRUPT_THERM_LOW         (SPI_TABLE_OFFSET + 5)
#define INTERRUPT_THERM_SHUTDOWN    (SPI_TABLE_OFFSET + 6)

/* PMU interrupts 7:10 */
#define INTERRUPT_PMU               (SPI_TABLE_OFFSET + 7)  /* 4 interrupts */

#define INTERRUPT_TIMER0            (SPI_TABLE_OFFSET + 11)
#define INTERRUPT_TIMER             INTERRUPT_ID_TIMER0
#define INTERRUPT_TIMER1            (SPI_TABLE_OFFSET + 12)
#define INTERRUPT_TIMER2            (SPI_TABLE_OFFSET + 13)
#define INTERRUPT_TIMER3            (SPI_TABLE_OFFSET + 14)
#define INTERRUPT_TIMER4            (SPI_TABLE_OFFSET + 15)
#define INTERRUPT_TIMER5            (SPI_TABLE_OFFSET + 16)
#define INTERRUPT_TIMER_MAX         INTERRUPT_ID_TIMER3

#define INTERRUPT_A7_COMMON         (SPI_TABLE_OFFSET + 17)
/* 18 : Reserved 
*/
#define INTERRUPT_MEMC_SEC          (SPI_TABLE_OFFSET + 19)
/*#define INTERRUPT_A7_INT_PENDING    (SPI_TABLE_OFFSET + 19)  */ 
#define INTERRUPT_PER_SEC_ACC_VIOL  (SPI_TABLE_OFFSET + 20)
#define INTERRUPT_A7_UBUS_RC        (SPI_TABLE_OFFSET + 21)
#define INTERRUPT_A7_UBUS_STAT_REG  (SPI_TABLE_OFFSET + 22)
#define INTERRUPT_A7_UBUS4_SYS_REG   (SPI_TABLE_OFFSET + 23)
/* 24:27 Reserved*/
#define INTERRUPT_DG                (SPI_TABLE_OFFSET + 28)
#define INTERRUPT_PMC_TEMP_WARN     (SPI_TABLE_OFFSET + 29)
#define INTERRUPT_PMC3              (SPI_TABLE_OFFSET + 30)
/*
#define INTERRUPT_PMC1              (SPI_TABLE_OFFSET + 31)
*/

#define INTERRUPT_UART0             (SPI_TABLE_OFFSET + 32)
/* 33 reserved */
#define INTERRUPT_HS_UART           (SPI_TABLE_OFFSET + 34)
/* 35 reserved */
#define INTERRUPT_HS_SPIM           (SPI_TABLE_OFFSET + 36)
#define INTERRUPT_NAND_FLASH        (SPI_TABLE_OFFSET + 37)
#define INTERRUPT_MEMC              (SPI_TABLE_OFFSET + 38)
/*39:40 reserved */
#define INTERRUPT_USBD              (SPI_TABLE_OFFSET + 41)
#define INTERRUPT_PCM               (SPI_TABLE_OFFSET + 42)
#define INTERRUPT_PCIE_0_CPU_INTR   (SPI_TABLE_OFFSET + 43)
/*44:45 reserved */
/*WLAN irqs 46:53*/
#define INTERRUPT_UBUS2AXI_WLAN0_CCM    (SPI_TABLE_OFFSET + 46)
#define INTERRUPT_UBUS2AXI_WLAN0_D11MAC (SPI_TABLE_OFFSET + 47)
#define INTERRUPT_UBUS2AXI_WLAN0_M2MDMA (SPI_TABLE_OFFSET + 48)
#define INTERRUPT_UBUS2AXI_WLAN0_WDRST  (SPI_TABLE_OFFSET + 49)

#define INTERRUPT_UBUS2AXI_WLAN1_CCM    (SPI_TABLE_OFFSET + 50)
#define INTERRUPT_UBUS2AXI_WLAN1_D11MAC (SPI_TABLE_OFFSET + 51)
#define INTERRUPT_UBUS2AXI_WLAN1_M2MDMA (SPI_TABLE_OFFSET + 52)
#define INTERRUPT_UBUS2AXI_WLAN1_WDRST  (SPI_TABLE_OFFSET + 53)
/*Reserved 54:71*/
#define INTERRUPT_PL081_DMA         (SPI_TABLE_OFFSET + 85)
#define INTERRUPT_SDIO_EMMC_L1      (SPI_TABLE_OFFSET + 86)

/*USB IRQs irqs 72:76*/
#define INTERRUPT_USB_OHCI          (SPI_TABLE_OFFSET + 72)
#define INTERRUPT_USB_EHCI          (SPI_TABLE_OFFSET + 73)
#define INTERRUPT_USB_XHCI          (SPI_TABLE_OFFSET + 74)
#define INTERRUPT_USB_BRIDGE        (SPI_TABLE_OFFSET + 75)
#define INTERRUPT_USB_EVENTS        (SPI_TABLE_OFFSET + 76)

#define INTERRUPT_I2C               (SPI_TABLE_OFFSET + 77)
#define INTERRUPT_PER_I2S               (SPI_TABLE_OFFSET + 78)
#define INTERRUPT_PER_DMA0          (SPI_TABLE_OFFSET + 79)
#define INTERRUPT_PER_DMA1          (SPI_TABLE_OFFSET + 80)
#define INTERRUPT_O_RNG             (SPI_TABLE_OFFSET + 81)
#define INTERRUPT_SPU_CTF           (SPI_TABLE_OFFSET + 90)
#define INTERRUPT_SPU_GMAC          (SPI_TABLE_OFFSET + 91)
/*92:95 reserved*/
/*96-100 4 irqs*/
#define INTERRUPT_SYSPORT0_0       (SPI_TABLE_OFFSET + 96)
#define INTERRUPT_SYSPORT0_1       (SPI_TABLE_OFFSET + 97)
#define INTERRUPT_SYSPORT0_2       (SPI_TABLE_OFFSET + 98)
#define INTERRUPT_SYSPORT0_3       (SPI_TABLE_OFFSET + 99)
#define INTERRUPT_SYSPORT0_4       (SPI_TABLE_OFFSET + 100)
#define INTERRUPT_SYSPORT0_PHY     (SPI_TABLE_OFFSET + 101)
/*102-106 4 irqs*/
#define INTERRUPT_SYSPORT1_0       (SPI_TABLE_OFFSET + 102)
#define INTERRUPT_SYSPORT1_1       (SPI_TABLE_OFFSET + 103)
#define INTERRUPT_SYSPORT1_2       (SPI_TABLE_OFFSET + 104)
#define INTERRUPT_SYSPORT1_3       (SPI_TABLE_OFFSET + 105)
#define INTERRUPT_SYSPORT1_4       (SPI_TABLE_OFFSET + 106)
#define INTERRUPT_SYSPORT1_PHY     (SPI_TABLE_OFFSET + 107)
/* 108-115 reserved */
#define INTERRUPT_PCM_DMA0          (SPI_TABLE_OFFSET + 116) /* PCM DMA RX interrupt */
#define INTERRUPT_PCM_DMA1          (SPI_TABLE_OFFSET + 117) /* PCM DMA TX interrupt */
/* 118-119 reserved */
#define INTERRUPT_PER_EXT_0         (SPI_TABLE_OFFSET + 120)
#define INTERRUPT_PER_EXT_1         (SPI_TABLE_OFFSET + 121)
#define INTERRUPT_PER_EXT_2         (SPI_TABLE_OFFSET + 122)
#define INTERRUPT_PER_EXT_3         (SPI_TABLE_OFFSET + 123)
#define INTERRUPT_PER_EXT_4         (SPI_TABLE_OFFSET + 124)
#define INTERRUPT_PER_EXT_5         (SPI_TABLE_OFFSET + 125)
#define INTERRUPT_PER_EXT_6         (SPI_TABLE_OFFSET + 126)
#define INTERRUPT_PER_EXT_7         (SPI_TABLE_OFFSET + 127)


#ifndef __ASSEMBLER__
#define _2MAP(V) (bcm_legacy_irq_map[(V - SPI_TABLE_OFFSET)])
#define INTERRUPT_ID_RANGE_CHECK          _2MAP(INTERRUPT_MEMC_SEC)
#define INTERRUPT_ID_TIMER0               _2MAP(INTERRUPT_TIMER0)
#define INTERRUPT_ID_TIMER                INTERRUPT_ID_TIMER0
#define INTERRUPT_ID_TIMER1               _2MAP(INTERRUPT_TIMER1)
#define INTERRUPT_ID_TIMER2               _2MAP(INTERRUPT_TIMER2)
#define INTERRUPT_ID_TIMER3               _2MAP(INTERRUPT_TIMER3)
#define INTERRUPT_ID_TIMER_MAX            INTERRUPT_ID_TIMER3
#define INTERRUPT_ID_DG                   _2MAP(INTERRUPT_DG)
#define INTERRUPT_ID_PMC_TEMP_WARN        _2MAP(INTERRUPT_PMC_TEMP_WARN)
#define INTERRUPT_ID_I2S                  _2MAP(INTERRUPT_PER_I2S)
#define INTERRUPT_ID_SYSPORT0_0           _2MAP(INTERRUPT_SYSPORT0_0)
#define INTERRUPT_ID_SYSPORT0_1           _2MAP(INTERRUPT_SYSPORT0_1)
#define INTERRUPT_ID_SYSPORT0_2           _2MAP(INTERRUPT_SYSPORT0_2)
#define INTERRUPT_ID_SYSPORT0_3           _2MAP(INTERRUPT_SYSPORT0_3)
#define INTERRUPT_ID_SYSPORT0_4           _2MAP(INTERRUPT_SYSPORT0_4)
#define INTERRUPT_ID_SYSPORT0_PHY         _2MAP(INTERRUPT_SYSPORT0_PHY)
#define INTERRUPT_ID_SYSPORT1_0           _2MAP(INTERRUPT_SYSPORT1_0)
#define INTERRUPT_ID_SYSPORT1_1           _2MAP(INTERRUPT_SYSPORT1_1)
#define INTERRUPT_ID_SYSPORT1_2           _2MAP(INTERRUPT_SYSPORT1_2)
#define INTERRUPT_ID_SYSPORT1_3           _2MAP(INTERRUPT_SYSPORT1_3)
#define INTERRUPT_ID_SYSPORT1_4           _2MAP(INTERRUPT_SYSPORT1_4)
#define INTERRUPT_ID_SYSPORT1_PHY         _2MAP(INTERRUPT_SYSPORT1_PHY)
#define INTERRUPT_ID_HS_UART              _2MAP(INTERRUPT_HS_UART)
#define INTERRUPT_ID_NAND_FLASH           _2MAP(INTERRUPT_NAND_FLASH)
#define INTERRUPT_ID_EXTERNAL_0           _2MAP(INTERRUPT_PER_EXT_0)
#define INTERRUPT_ID_EXTERNAL_1           _2MAP(INTERRUPT_PER_EXT_1)
#define INTERRUPT_ID_EXTERNAL_2           _2MAP(INTERRUPT_PER_EXT_2)
#define INTERRUPT_ID_EXTERNAL_3           _2MAP(INTERRUPT_PER_EXT_3)
#define INTERRUPT_ID_EXTERNAL_4           _2MAP(INTERRUPT_PER_EXT_4)
#define INTERRUPT_ID_EXTERNAL_5           _2MAP(INTERRUPT_PER_EXT_5)
#define INTERRUPT_ID_EXTERNAL_6           _2MAP(INTERRUPT_PER_EXT_6)
#define INTERRUPT_ID_EXTERNAL_7           _2MAP(INTERRUPT_PER_EXT_7)
#define INTERRUPT_ID_EXTERNAL_MAX         INTERRUPT_ID_EXTERNAL_7
#define INTERRUPT_ID_WLAN0_CCM            _2MAP(INTERRUPT_UBUS2AXI_WLAN0_CCM)
#define INTERRUPT_ID_WLAN0_D11MAC         _2MAP(INTERRUPT_UBUS2AXI_WLAN0_D11MAC)
#define INTERRUPT_ID_WLAN0_M2MDMA         _2MAP(INTERRUPT_UBUS2AXI_WLAN0_M2MDMA)
#define INTERRUPT_ID_WLAN0_WDRST          _2MAP(INTERRUPT_UBUS2AXI_WLAN0_WDRST)
#define INTERRUPT_ID_WLAN1_CCM            _2MAP(INTERRUPT_UBUS2AXI_WLAN1_CCM)
#define INTERRUPT_ID_WLAN1_D11MAC         _2MAP(INTERRUPT_UBUS2AXI_WLAN1_D11MAC)
#define INTERRUPT_ID_WLAN1_M2MDMA         _2MAP(INTERRUPT_UBUS2AXI_WLAN1_M2MDMA)
#define INTERRUPT_ID_WLAN1_WDRST          _2MAP(INTERRUPT_UBUS2AXI_WLAN1_WDRST)
#define INTERRUPT_ID_USB_OHCI             _2MAP(INTERRUPT_USB_OHCI)
#define INTERRUPT_ID_USB_OHCI1            INTERRUPT_ID_USB_OHCI
#define INTERRUPT_ID_USB_EHCI             _2MAP(INTERRUPT_USB_EHCI)
#define INTERRUPT_ID_USB_EHCI1            INTERRUPT_ID_USB_EHCI
#define INTERRUPT_ID_USB_XHCI             _2MAP(INTERRUPT_USB_XHCI)
#define INTERRUPT_PCM_DMA_IRQ             _2MAP(INTERRUPT_PCM_DMA0) 

#define SYSPORT_INTERRUPT_ID(_intf, _id)  INTERRUPT_ID_SYSPORT##_intf##_##_id
#define SYSPORT_WOL_INTERRUPT_ID(_intf)   INTERRUPT_ID_SYSPORT##_intf##_PHY 

#ifdef __BOARD_DRV_ARMV7__
unsigned int bcm_phys_irqs_to_map[] =
{
    INTERRUPT_DG,
    INTERRUPT_PMC_TEMP_WARN,
    INTERRUPT_TIMER0,
    INTERRUPT_TIMER1,
    INTERRUPT_TIMER2,
    INTERRUPT_TIMER3,
    INTERRUPT_HS_UART,
    INTERRUPT_PER_I2S,
    INTERRUPT_NAND_FLASH,
    INTERRUPT_PER_EXT_0,
    INTERRUPT_PER_EXT_1,
    INTERRUPT_PER_EXT_2,
    INTERRUPT_PER_EXT_3,
    INTERRUPT_PER_EXT_4,
    INTERRUPT_PER_EXT_5,
    INTERRUPT_PER_EXT_6,
    INTERRUPT_PER_EXT_7,
    INTERRUPT_UBUS2AXI_WLAN0_CCM,
    INTERRUPT_UBUS2AXI_WLAN0_D11MAC,
    INTERRUPT_UBUS2AXI_WLAN0_M2MDMA,
    INTERRUPT_UBUS2AXI_WLAN0_WDRST,
    INTERRUPT_UBUS2AXI_WLAN1_CCM,
    INTERRUPT_UBUS2AXI_WLAN1_D11MAC,
    INTERRUPT_UBUS2AXI_WLAN1_M2MDMA,
    INTERRUPT_UBUS2AXI_WLAN1_WDRST,
    INTERRUPT_USB_OHCI,   
    INTERRUPT_USB_EHCI, 
    INTERRUPT_USB_XHCI,
    INTERRUPT_SYSPORT0_0,
    INTERRUPT_SYSPORT0_1,
    INTERRUPT_SYSPORT0_2,
    INTERRUPT_SYSPORT0_3,
    INTERRUPT_SYSPORT0_4,
    INTERRUPT_SYSPORT0_PHY,
    INTERRUPT_SYSPORT1_0,
    INTERRUPT_SYSPORT1_1,
    INTERRUPT_SYSPORT1_2,
    INTERRUPT_SYSPORT1_3,
    INTERRUPT_SYSPORT1_4,
    INTERRUPT_SYSPORT1_PHY,
    INTERRUPT_PCM_DMA0
};
unsigned int bcm_legacy_irq_map[256];
#else
extern unsigned int bcm_phys_irqs_to_map[];
extern unsigned int bcm_legacy_irq_map[];
#endif
#endif


#define NUM_EXT_INT    (INTERRUPT_PER_EXT_7-INTERRUPT_PER_EXT_0+1)

#ifdef __cplusplus
    }
#endif                    

#endif  /* __BCM47622_H */

