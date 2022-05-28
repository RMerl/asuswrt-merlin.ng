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

#ifndef __68360_INTR_H
#define __68360_INTR_H

#ifdef __cplusplus
    extern "C" {
#endif

/*=====================================================================*/
/* SPI Table Offset                                                    */
/*=====================================================================*/
#define SPI_TABLE_OFFSET             32

/*=====================================================================*/
/* Physical Interrupt IDs                                              */
/*=====================================================================*/
    /*  ------------- CHIP_IRQS[31-0] ----------------------------*/
#define INTERRUPT_B53_AXIERRIRQ           (SPI_TABLE_OFFSET + 0)
#define INTERRUPT_B53_INTERRIRQ           (SPI_TABLE_OFFSET + 1)
#define INTERRUPT_B53_ACCESS_VIOL0_IRQ    (SPI_TABLE_OFFSET + 2)
#define INTERRUPT_B53_ACCESS_VIOL1_IRQ    (SPI_TABLE_OFFSET + 3)
#define INTERRUPT_B53_CCIERRIRQ           (SPI_TABLE_OFFSET + 4)
#define INTERRUPT_B53_CCIOVFLOWIRQ        (SPI_TABLE_OFFSET + 5)
#define INTERRUPT_THERM_HIGH_IRQ          (SPI_TABLE_OFFSET + 6)
#define INTERRUPT_THERM_LOW_IRQ           (SPI_TABLE_OFFSET + 7)
#define INTERRUPT_THERM_SHUTDOWN          (SPI_TABLE_OFFSET + 8)
#define INTERRUPT_PMUIRQ                  (SPI_TABLE_OFFSET + 9)  /* 2 interrupts */
                    /* 4 reserved interrupts */
#define INTERRUPT_PER_TIMER_IRQ3_4        (SPI_TABLE_OFFSET + 15) /* 2 interrupts */
#define INTERRUPT_B53_COMMONIRQ           (SPI_TABLE_OFFSET + 17)
#define INTERRUPT_MEMC_SEC_IRQ            (SPI_TABLE_OFFSET + 18)
#define INTERRUPT_ORION_INT_PENDING       (SPI_TABLE_OFFSET + 19)   
#define INTERRUPT_PER_SEC_ACC_VIOL_IRQ    (SPI_TABLE_OFFSET + 20)
#define INTERRUPT_B53_UBUS_RC_IRQ         (SPI_TABLE_OFFSET + 21)
                    /* 1 reserved interrupts */
#define INTERRUPT_SPU_GMAC_IRQ            (SPI_TABLE_OFFSET + 23)
#define INTERRUPT_SPU_CTF_IRQ             (SPI_TABLE_OFFSET + 24)
                    /* 4 reserved interrupts */
#define INTERRUPT_DYING_GASP_IRQ          (SPI_TABLE_OFFSET + 29)
#define INTERRUPT_PMC_MIPS_IRQ            (SPI_TABLE_OFFSET + 30)
#define INTERRUPT_PMC_MIPS1_IRQ           (SPI_TABLE_OFFSET + 31)
#define INTERRUPT_PER_UARTINT0            (SPI_TABLE_OFFSET + 32)
#define INTERRUPT_PER_UARTINT1            (SPI_TABLE_OFFSET + 33)
                    /* 2 reserved interrupts */
#define INTERRUPT_PER_DMA_IRQ             (SPI_TABLE_OFFSET + 36)
#define INTERRUPT_PER_HS_SPIM_IRQ         (SPI_TABLE_OFFSET + 37)
#define INTERRUPT_NAND_FLASH_IRQ          (SPI_TABLE_OFFSET + 38)
#define INTERRUPT_MEMC_IRQ                (SPI_TABLE_OFFSET + 39)
                 /* 4 reserved interrupts  */
#define INTERRUPT_SATA_CPU_INTR0          (SPI_TABLE_OFFSET + 44)
#define INTERRUPT_SATA_CPU_INTR1          (SPI_TABLE_OFFSET + 45)
                 /* 4 reserved interrupts  */
#define INTERRUPT_MAC_QEGPHY_CFG_IRQ      (SPI_TABLE_OFFSET + 50) /* 4 interrupts */
                 /* 6 reserved interrupts  */
#define INTERRUPT_PCIE_0_CPU_INTR         (SPI_TABLE_OFFSET + 60)
#define INTERRUPT_PCIE_1_CPU_INTR         (SPI_TABLE_OFFSET + 61)
#define INTERRUPT_PCIE_2_CPU_INTR         (SPI_TABLE_OFFSET + 62)
#define INTERRUPT_UBUS4_SYS_IRQ           (SPI_TABLE_OFFSET + 63)
#define INTERRUPT_PER_TIMER_IRQ0          (SPI_TABLE_OFFSET + 64)
#define INTERRUPT_PER_TIMER_IRQ1          (SPI_TABLE_OFFSET + 65)
#define INTERRUPT_PER_TIMER_IRQ2          (SPI_TABLE_OFFSET + 66)
#define INTERRUPT_PER_TIMER_IRQ3          (SPI_TABLE_OFFSET + 67)
#define INTERRUPT_MBOX_IRQ0_3             (SPI_TABLE_OFFSET + 68)  /* 4 interrupts */
#define INTERRUPT_PERIPH_INTERNAL0        (SPI_TABLE_OFFSET + 72)  /* 5 interrupts */
                /* 6 reserved interrupts  */
#define INTERRUPT_PER_I2C_IRQ             (SPI_TABLE_OFFSET + 83)
#define INTERRUPT_PER_I2S_IRQ             (SPI_TABLE_OFFSET + 84)
#define INTERRUPT_O_RNG_INTR              (SPI_TABLE_OFFSET + 85)
                /* 3 reserved interrupts  */
#define INTERRUPT_SGMII_SIGDET_IRQ        (SPI_TABLE_OFFSET + 89)
                /* 3 reserved interrupts  */
#define INTERRUPT_PER_BT_UART             (SPI_TABLE_OFFSET + 93)
#define INTERRUPT_PL081_DMA_INTR          (SPI_TABLE_OFFSET + 94)
#define INTERRUPT_SDIO_EMMC_L1_INTR       (SPI_TABLE_OFFSET + 95)
#define INTERRUPT_WAN_EPON_TOP            (SPI_TABLE_OFFSET + 96)
#define INTERRUPT_WAN_NCO_GPON            (SPI_TABLE_OFFSET + 97)
#define INTERRUPT_WAN_GPON_TX             (SPI_TABLE_OFFSET + 98)
#define INTERRUPT_WAN_GPON_RX             (SPI_TABLE_OFFSET + 99)
#define INTERRUPT_WAN_PMD_PLL1_LOCK_INT   (SPI_TABLE_OFFSET + 100)
#define INTERRUPT_WAN_PMD_PLL0_LOCK_INT   (SPI_TABLE_OFFSET + 101)
#define INTERRUPT_WAN_PMD_SIGNAL_DETECT_0 (SPI_TABLE_OFFSET + 102)
#define INTERRUPT_WAN_PMD_ENERGY_DETECT_0 (SPI_TABLE_OFFSET + 103)
#define INTERRUPT_WAN_PMD_RX_LOCK_0       (SPI_TABLE_OFFSET + 104)
#define INTERRUPT_PER_EXT_0               (SPI_TABLE_OFFSET + 105)
#define INTERRUPT_PER_EXT_1               (SPI_TABLE_OFFSET + 106)
#define INTERRUPT_PER_EXT_2               (SPI_TABLE_OFFSET + 107)
#define INTERRUPT_PER_EXT_3               (SPI_TABLE_OFFSET + 108)
#define INTERRUPT_PER_EXT_4               (SPI_TABLE_OFFSET + 109)
#define INTERRUPT_PER_EXT_5               (SPI_TABLE_OFFSET + 110)
#define INTERRUPT_PER_SIM                 (SPI_TABLE_OFFSET + 111) /* 3 interrupts */
#define INTERRUPT_PERIPH_INTERNAL1        (SPI_TABLE_OFFSET + 114) /* 4 interrupts */
#define INTERRUPT_PER_MDIO_ERR_IRQ        (SPI_TABLE_OFFSET + 118)
#define INTERRUPT_PER_MDIO_DONE_IRQ       (SPI_TABLE_OFFSET + 119)
#define INTERRUPT_USB_USBD            (SPI_TABLE_OFFSET + 120)
#define INTERRUPT_USB_XHCI            (SPI_TABLE_OFFSET + 121)
#define INTERRUPT_USB_OHCI1           (SPI_TABLE_OFFSET + 122)
#define INTERRUPT_USB_EHCI1           (SPI_TABLE_OFFSET + 123)
#define INTERRUPT_USB_OHCI            (SPI_TABLE_OFFSET + 123)
#define INTERRUPT_USB_EHCI            (SPI_TABLE_OFFSET + 125)
#define INTERRUPT_USB_EVENTS          (SPI_TABLE_OFFSET + 126)
#define INTERRUPT_USB_BRIDGE          (SPI_TABLE_OFFSET + 127)

/*=====================================================================*/
/* Linux(Virtual) Interrupt IDs                                        */
/* Each physical irq id to be mapped should be added to                */
/* bcm_phys_irqs_to_map array in board_aarch64.c file                  */
/*=====================================================================*/
#define INTERRUPT_ID_DG                   (bcm_legacy_irq_map[INTERRUPT_DYING_GASP_IRQ - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_UART                 (bcm_legacy_irq_map[INTERRUPT_PER_UARTINT0 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_UART1                (bcm_legacy_irq_map[INTERRUPT_PER_UARTINT1 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_SATAC                (bcm_legacy_irq_map[INTERRUPT_SATA_CPU_INTR0 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_PCIE0                (bcm_legacy_irq_map[INTERRUPT_PCIE_0_CPU_INTR - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_PCIE1                (bcm_legacy_irq_map[INTERRUPT_PCIE_1_CPU_INTR - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_PCIE2                (bcm_legacy_irq_map[INTERRUPT_PCIE_2_CPU_INTR - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_TIMER0               (bcm_legacy_irq_map[INTERRUPT_PER_TIMER_IRQ0 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_TIMER                INTERRUPT_ID_TIMER0
#define INTERRUPT_ID_TIMER1               (bcm_legacy_irq_map[INTERRUPT_PER_TIMER_IRQ1 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_TIMER2               (bcm_legacy_irq_map[INTERRUPT_PER_TIMER_IRQ2 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_TIMER3               (bcm_legacy_irq_map[INTERRUPT_PER_TIMER_IRQ3 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_TIMER_MAX            INTERRUPT_ID_TIMER3
#define INTERRUPT_ID_EXTERNAL_0           (bcm_legacy_irq_map[INTERRUPT_PER_EXT_0 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_EXTERNAL_1           (bcm_legacy_irq_map[INTERRUPT_PER_EXT_1 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_EXTERNAL_2           (bcm_legacy_irq_map[INTERRUPT_PER_EXT_2 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_EXTERNAL_3           (bcm_legacy_irq_map[INTERRUPT_PER_EXT_3 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_EXTERNAL_4           (bcm_legacy_irq_map[INTERRUPT_PER_EXT_4 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_EXTERNAL_5           (bcm_legacy_irq_map[INTERRUPT_PER_EXT_5 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_EXTERNAL_MAX         INTERRUPT_ID_EXTERNAL_5
#define INTERRUPT_ID_USB_XHCI             (bcm_legacy_irq_map[INTERRUPT_USB_XHCI - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_USB_OHCI             (bcm_legacy_irq_map[INTERRUPT_USB_OHCI - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_USB_EHCI             (bcm_legacy_irq_map[INTERRUPT_USB_EHCI - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_NAND_FLASH              (bcm_legacy_irq_map[INTERRUPT_NAND_FLASH_IRQ - SPI_TABLE_OFFSET])

#ifdef __BOARD_DRV_AARCH64__
// add here any legacy driver's (driver that have no device tree node) interrupt to be mapped
unsigned int bcm_phys_irqs_to_map[] =
{
    INTERRUPT_DYING_GASP_IRQ,
    INTERRUPT_PER_UARTINT0,
    INTERRUPT_PER_UARTINT1,
    INTERRUPT_PCIE_0_CPU_INTR,
    INTERRUPT_PCIE_1_CPU_INTR,
    INTERRUPT_PCIE_2_CPU_INTR,
    INTERRUPT_PER_TIMER_IRQ0,
    INTERRUPT_PER_TIMER_IRQ1,
    INTERRUPT_PER_TIMER_IRQ2,
    INTERRUPT_PER_TIMER_IRQ3,
    INTERRUPT_PER_EXT_0,
    INTERRUPT_PER_EXT_1,
    INTERRUPT_PER_EXT_2,
    INTERRUPT_PER_EXT_3,
    INTERRUPT_PER_EXT_4,
    INTERRUPT_PER_EXT_5,
    INTERRUPT_NAND_FLASH_IRQ,
    INTERRUPT_USB_XHCI,
    INTERRUPT_USB_OHCI,
    INTERRUPT_USB_EHCI
};
unsigned int bcm_legacy_irq_map[256];
#else
extern unsigned int bcm_phys_irqs_to_map[];
extern unsigned int bcm_legacy_irq_map[];
#endif

#ifdef __cplusplus
    }
#endif                    

#endif  /* __BCM68360_H */

