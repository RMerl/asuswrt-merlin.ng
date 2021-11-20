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

#ifndef __6846_INTR_H
#define __6846_INTR_H

#ifdef __cplusplus
    extern "C" {
#endif

/*=====================================================================*/
/* SPI Table Offset                                                    */
/*=====================================================================*/
#define SPI_TABLE_OFFSET               32
#define SPI_TABLE_OFFSET1              (SPI_TABLE_OFFSET + 32)
#define SPI_TABLE_OFFSET2              (SPI_TABLE_OFFSET1 + 32)
#define SPI_TABLE_OFFSET3              (SPI_TABLE_OFFSET2 + 32)
#define SPI_TABLE_OFFSET4              (SPI_TABLE_OFFSET3 + 32)
#define SPI_TABLE_OFFSET5              (SPI_TABLE_OFFSET4 + 32)

/*=====================================================================*/
/* Physical Interrupt IDs                                              */
/*=====================================================================*/
    /*  ------------- CHIP_IRQS[31-0] ----------------------------*/
#define INTERRUPT_A7_AXIERR         (SPI_TABLE_OFFSET + 0)
#define INTERRUPT_A7_INTERR         (SPI_TABLE_OFFSET + 1)
#define INTERRUPT_A7_CCIERR         (SPI_TABLE_OFFSET + 4)
#define INTERRUPT_A7_CCIOVFLOW      (SPI_TABLE_OFFSET + 5)
#define INTERRUPT_THERM_HIGH        (SPI_TABLE_OFFSET + 6)
#define INTERRUPT_THERM_LOW         (SPI_TABLE_OFFSET + 7)
#define INTERRUPT_THERM_SHUTDOWN    (SPI_TABLE_OFFSET + 8)
#define INTERRUPT_PMU               (SPI_TABLE_OFFSET + 9)  /* 2 interrupts */
#define INTERRUPT_TIMER0            (SPI_TABLE_OFFSET + 11)
#define INTERRUPT_TIMER             INTERRUPT_ID_TIMER0
#define INTERRUPT_TIMER1            (SPI_TABLE_OFFSET + 12)
#define INTERRUPT_TIMER2            (SPI_TABLE_OFFSET + 13)
#define INTERRUPT_TIMER3            (SPI_TABLE_OFFSET + 14)
#define INTERRUPT_TIMER_MAX         INTERRUPT_ID_TIMER3
#define INTERRUPT_A7_COMMON         (SPI_TABLE_OFFSET + 17)
#define INTERRUPT_MEMC_SEC          (SPI_TABLE_OFFSET + 18)
#define INTERRUPT_A7_INT_PENDING    (SPI_TABLE_OFFSET + 19)   
#define INTERRUPT_PER_SEC_ACC_VIOL  (SPI_TABLE_OFFSET + 20)
#define INTERRUPT_A7_UBUS_RC        (SPI_TABLE_OFFSET + 21)
#define INTERRUPT_A7_UBUS_STAT_REG  (SPI_TABLE_OFFSET + 22)
#define INTERRUPT_PMC_TEMP_WARN     (SPI_TABLE_OFFSET + 28)
#define INTERRUPT_DG                (SPI_TABLE_OFFSET + 29)
#define INTERRUPT_PMC0              (SPI_TABLE_OFFSET + 30)
#define INTERRUPT_PMC1              (SPI_TABLE_OFFSET + 31)

#define INTERRUPT_HS_SPIM           (SPI_TABLE_OFFSET1 + 5)
#define INTERRUPT_NAND_FLASH        (SPI_TABLE_OFFSET1 + 6)
#define INTERRUPT_MEMC              (SPI_TABLE_OFFSET1 + 7)
#define INTERRUPT_MAC_QEGPHY_CFG    (SPI_TABLE_OFFSET1 + 18) /* 4 interrupts */
#define INTERRUPT_MAC_QEGPHY_CFG_ACT (SPI_TABLE_OFFSET1 + 22) /* 4 interrupts */
#define INTERRUPT_PCIE_0_CPU_INTR   (SPI_TABLE_OFFSET1 + 28)
#define INTERRUPT_PCIE_1_CPU_INTR   (SPI_TABLE_OFFSET1 + 29)
#define INTERRUPT_UBUS4_SYS         (SPI_TABLE_OFFSET1 + 31)

#define INTERRUPT_MBOX0_3           (SPI_TABLE_OFFSET2 + 4)  /* 4 interrupts */
#define INTERRUPT_PERIPH_INTERNAL0  (SPI_TABLE_OFFSET2 + 8)  /* 5 interrupts */
#define INTERRUPT_I2C               (SPI_TABLE_OFFSET2 + 19)
#define INTERRUPT_O_RNG             (SPI_TABLE_OFFSET2 + 21)
#define INTERRUPT_UART0             (SPI_TABLE_OFFSET2 + 28)
#define INTERRUPT_HS_UART           (SPI_TABLE_OFFSET2 + 29)
#define INTERRUPT_PL081_DMA         (SPI_TABLE_OFFSET2 + 30)
#define INTERRUPT_SDIO_EMMC_L1      (SPI_TABLE_OFFSET2 + 31)

#define INTERRUPT_WAN_EPON_TOP      (SPI_TABLE_OFFSET3 + 0)
#define INTERRUPT_WAN_NCO_GPON      (SPI_TABLE_OFFSET3 + 1)
#define INTERRUPT_WAN_GPON_TX       (SPI_TABLE_OFFSET3 + 2)
#define INTERRUPT_WAN_GPON_RX       (SPI_TABLE_OFFSET3 + 3)
#define INTERRUPT_WAN_PMD_PLL1_LOCK (SPI_TABLE_OFFSET3 + 4)
#define INTERRUPT_WAN_PMD_PLL0_LOCK (SPI_TABLE_OFFSET3 + 5)
#define INTERRUPT_WAN_PMD_SIGNAL_DETECT_0 (SPI_TABLE_OFFSET3 + 6)
#define INTERRUPT_WAN_PMD_ENERGY_DETECT_0 (SPI_TABLE_OFFSET3 + 7)
#define INTERRUPT_WAN_PMD_RX_LOCK_0 (SPI_TABLE_OFFSET3 + 8)
#define INTERRUPT_SIM               (SPI_TABLE_OFFSET3 + 15) /* 3 interrupts */
#define INTERRUPT_PCM_DMA0          (SPI_TABLE_OFFSET3 + 18) /* PCM DMA RX interrupt */
#define INTERRUPT_PCM_DMA1          (SPI_TABLE_OFFSET3 + 19) /* PCM DMA TX interrupt */
#define INTERRUPT_PCM               (SPI_TABLE_OFFSET3 + 20) /* 2 interrupts */
#define INTERRUPT_MDIO_ERR          (SPI_TABLE_OFFSET3 + 22)
#define INTERRUPT_MDIO_DONE         (SPI_TABLE_OFFSET3 + 23)
#define INTERRUPT_USBD              (SPI_TABLE_OFFSET3 + 24)
#define INTERRUPT_USB_OHCI1         (SPI_TABLE_OFFSET3 + 26)
#define INTERRUPT_USB_EHCI1         (SPI_TABLE_OFFSET3 + 27)
#define INTERRUPT_USB_OHCI          (SPI_TABLE_OFFSET3 + 28)
#define INTERRUPT_USB_EHCI          (SPI_TABLE_OFFSET3 + 29)
#define INTERRUPT_USB_EVENTS        (SPI_TABLE_OFFSET3 + 30)
#define INTERRUPT_USB_BRIDGE        (SPI_TABLE_OFFSET3 + 31)
#define INTERRUPT_XRDP_QUEUE_0      (SPI_TABLE_OFFSET4 + 0)
#define INTERRUPT_XRDP_QUEUE_1      (SPI_TABLE_OFFSET4 + 1)
#define INTERRUPT_XRDP_QUEUE_2      (SPI_TABLE_OFFSET4 + 2)
#define INTERRUPT_XRDP_QUEUE_3      (SPI_TABLE_OFFSET4 + 3)
#define INTERRUPT_XRDP_QUEUE_4      (SPI_TABLE_OFFSET4 + 4)
#define INTERRUPT_XRDP_QUEUE_5      (SPI_TABLE_OFFSET4 + 5)
#define INTERRUPT_XRDP_QUEUE_6      (SPI_TABLE_OFFSET4 + 6)
#define INTERRUPT_XRDP_QUEUE_7      (SPI_TABLE_OFFSET4 + 7)
#define INTERRUPT_XRDP_QUEUE_8      (SPI_TABLE_OFFSET4 + 8)
#define INTERRUPT_XRDP_QUEUE_9      (SPI_TABLE_OFFSET4 + 9)
#define INTERRUPT_XRDP_QUEUE_10     (SPI_TABLE_OFFSET4 + 10)
#define INTERRUPT_XRDP_QUEUE_11     (SPI_TABLE_OFFSET4 + 11)
#define INTERRUPT_XRDP_QUEUE_12     (SPI_TABLE_OFFSET4 + 12)
#define INTERRUPT_XRDP_QUEUE_13     (SPI_TABLE_OFFSET4 + 13)
#define INTERRUPT_XRDP_QUEUE_14     (SPI_TABLE_OFFSET4 + 14)
#define INTERRUPT_XRDP_QUEUE_15     (SPI_TABLE_OFFSET4 + 15)
#define INTERRUPT_XRDP_QUEUE_16     (SPI_TABLE_OFFSET4 + 16)
#define INTERRUPT_XRDP_QUEUE_17     (SPI_TABLE_OFFSET4 + 17)
#define INTERRUPT_XRDP_QUEUE_18     (SPI_TABLE_OFFSET4 + 18)
#define INTERRUPT_XRDP_QUEUE_19     (SPI_TABLE_OFFSET4 + 19)
#define INTERRUPT_XRDP_QUEUE_20     (SPI_TABLE_OFFSET4 + 20)
#define INTERRUPT_XRDP_QUEUE_21     (SPI_TABLE_OFFSET4 + 21)
#define INTERRUPT_XRDP_QUEUE_22     (SPI_TABLE_OFFSET4 + 22)
#define INTERRUPT_XRDP_QUEUE_23     (SPI_TABLE_OFFSET4 + 23)
#define INTERRUPT_XRDP_QUEUE_24     (SPI_TABLE_OFFSET4 + 24)
#define INTERRUPT_XRDP_QUEUE_25     (SPI_TABLE_OFFSET4 + 25)
#define INTERRUPT_XRDP_QUEUE_26     (SPI_TABLE_OFFSET4 + 26)
#define INTERRUPT_XRDP_QUEUE_27     (SPI_TABLE_OFFSET4 + 27)
#define INTERRUPT_XRDP_QUEUE_28     (SPI_TABLE_OFFSET4 + 28)
#define INTERRUPT_XRDP_QUEUE_29     (SPI_TABLE_OFFSET4 + 29)
#define INTERRUPT_XRDP_QUEUE_30     (SPI_TABLE_OFFSET4 + 30)
#define INTERRUPT_XRDP_QUEUE_31     (SPI_TABLE_OFFSET4 + 31)

#define INTERRUPT_XRDP_FPM          (SPI_TABLE_OFFSET5 + 0)
#define INTERRUPT_PER_EXT_0         (SPI_TABLE_OFFSET5 + 21)
#define INTERRUPT_PER_EXT_1         (SPI_TABLE_OFFSET5 + 22)
#define INTERRUPT_PER_EXT_2         (SPI_TABLE_OFFSET5 + 23)
#define INTERRUPT_PER_EXT_3         (SPI_TABLE_OFFSET5 + 24)
#define INTERRUPT_PER_EXT_4         (SPI_TABLE_OFFSET5 + 25)
#define INTERRUPT_PER_EXT_5         (SPI_TABLE_OFFSET5 + 26)
#define INTERRUPT_PER_EXT_6         (SPI_TABLE_OFFSET5 + 27)
#define INTERRUPT_PER_EXT_7         (SPI_TABLE_OFFSET5 + 28)

#ifndef __ASSEMBLER__
#define INTERRUPT_ID_TIMER0               (bcm_legacy_irq_map[INTERRUPT_TIMER0 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_TIMER                INTERRUPT_ID_TIMER0
#define INTERRUPT_ID_TIMER1               (bcm_legacy_irq_map[INTERRUPT_TIMER1 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_TIMER2               (bcm_legacy_irq_map[INTERRUPT_TIMER2 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_TIMER3               (bcm_legacy_irq_map[INTERRUPT_TIMER3 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_TIMER_MAX            INTERRUPT_ID_TIMER3
#define INTERRUPT_ID_RANGE_CHECK          (bcm_legacy_irq_map[INTERRUPT_MEMC_SEC - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_DG                   (bcm_legacy_irq_map[INTERRUPT_DG - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_UART                 (bcm_legacy_irq_map[INTERRUPT_UART0 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_HS_UART              (bcm_legacy_irq_map[INTERRUPT_HS_UART - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_NAND_FLASH           (bcm_legacy_irq_map[INTERRUPT_NAND_FLASH - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_EXTERNAL_0           (bcm_legacy_irq_map[INTERRUPT_PER_EXT_0 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_EXTERNAL_1           (bcm_legacy_irq_map[INTERRUPT_PER_EXT_1 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_EXTERNAL_2           (bcm_legacy_irq_map[INTERRUPT_PER_EXT_2 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_EXTERNAL_3           (bcm_legacy_irq_map[INTERRUPT_PER_EXT_3 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_EXTERNAL_4           (bcm_legacy_irq_map[INTERRUPT_PER_EXT_4 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_EXTERNAL_5           (bcm_legacy_irq_map[INTERRUPT_PER_EXT_5 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_EXTERNAL_6           (bcm_legacy_irq_map[INTERRUPT_PER_EXT_6 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_EXTERNAL_7           (bcm_legacy_irq_map[INTERRUPT_PER_EXT_7 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_EXTERNAL_MAX         INTERRUPT_ID_EXTERNAL_7
#define INTERRUPT_ID_USB_OHCI             (bcm_legacy_irq_map[INTERRUPT_USB_OHCI - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_USB_EHCI             (bcm_legacy_irq_map[INTERRUPT_USB_EHCI - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_USB_OHCI1            (bcm_legacy_irq_map[INTERRUPT_USB_OHCI1 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_USB_EHCI1            (bcm_legacy_irq_map[INTERRUPT_USB_EHCI1 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_WAN_GPON_RX          (bcm_legacy_irq_map[INTERRUPT_WAN_GPON_RX - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_WAN_GPON_TX          (bcm_legacy_irq_map[INTERRUPT_WAN_GPON_TX - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_XRDP_QUEUE_0          (bcm_legacy_irq_map[INTERRUPT_XRDP_QUEUE_0 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_XRDP_QUEUE_1            (bcm_legacy_irq_map[INTERRUPT_XRDP_QUEUE_1 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_XRDP_QUEUE_2            (bcm_legacy_irq_map[INTERRUPT_XRDP_QUEUE_2 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_XRDP_QUEUE_3            (bcm_legacy_irq_map[INTERRUPT_XRDP_QUEUE_3 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_XRDP_QUEUE_4            (bcm_legacy_irq_map[INTERRUPT_XRDP_QUEUE_4 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_XRDP_QUEUE_5            (bcm_legacy_irq_map[INTERRUPT_XRDP_QUEUE_5 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_XRDP_QUEUE_6            (bcm_legacy_irq_map[INTERRUPT_XRDP_QUEUE_6 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_XRDP_QUEUE_7            (bcm_legacy_irq_map[INTERRUPT_XRDP_QUEUE_7 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_XRDP_QUEUE_8            (bcm_legacy_irq_map[INTERRUPT_XRDP_QUEUE_8 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_XRDP_QUEUE_9            (bcm_legacy_irq_map[INTERRUPT_XRDP_QUEUE_9- SPI_TABLE_OFFSET])
#define INTERRUPT_ID_XRDP_QUEUE_10           (bcm_legacy_irq_map[INTERRUPT_XRDP_QUEUE_10 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_XRDP_QUEUE_11           (bcm_legacy_irq_map[INTERRUPT_XRDP_QUEUE_11 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_XRDP_QUEUE_12           (bcm_legacy_irq_map[INTERRUPT_XRDP_QUEUE_12 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_XRDP_QUEUE_13           (bcm_legacy_irq_map[INTERRUPT_XRDP_QUEUE_13 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_XRDP_QUEUE_14           (bcm_legacy_irq_map[INTERRUPT_XRDP_QUEUE_14 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_XRDP_QUEUE_15           (bcm_legacy_irq_map[INTERRUPT_XRDP_QUEUE_15 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_XRDP_QUEUE_16           (bcm_legacy_irq_map[INTERRUPT_XRDP_QUEUE_16 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_XRDP_QUEUE_17           (bcm_legacy_irq_map[INTERRUPT_XRDP_QUEUE_17 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_XRDP_QUEUE_18           (bcm_legacy_irq_map[INTERRUPT_XRDP_QUEUE_18 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_XRDP_QUEUE_19           (bcm_legacy_irq_map[INTERRUPT_XRDP_QUEUE_19 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_XRDP_QUEUE_20           (bcm_legacy_irq_map[INTERRUPT_XRDP_QUEUE_20 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_XRDP_QUEUE_21           (bcm_legacy_irq_map[INTERRUPT_XRDP_QUEUE_21 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_XRDP_QUEUE_22           (bcm_legacy_irq_map[INTERRUPT_XRDP_QUEUE_22 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_XRDP_QUEUE_23           (bcm_legacy_irq_map[INTERRUPT_XRDP_QUEUE_23 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_XRDP_QUEUE_24           (bcm_legacy_irq_map[INTERRUPT_XRDP_QUEUE_24 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_XRDP_QUEUE_25           (bcm_legacy_irq_map[INTERRUPT_XRDP_QUEUE_25 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_XRDP_QUEUE_26           (bcm_legacy_irq_map[INTERRUPT_XRDP_QUEUE_26 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_XRDP_QUEUE_27           (bcm_legacy_irq_map[INTERRUPT_XRDP_QUEUE_27 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_XRDP_QUEUE_28           (bcm_legacy_irq_map[INTERRUPT_XRDP_QUEUE_28 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_XRDP_QUEUE_39           (bcm_legacy_irq_map[INTERRUPT_XRDP_QUEUE_29 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_XRDP_QUEUE_30           (bcm_legacy_irq_map[INTERRUPT_XRDP_QUEUE_30 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_XRDP_QUEUE_31           (bcm_legacy_irq_map[INTERRUPT_XRDP_QUEUE_31 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_XRDP_FPM                (bcm_legacy_irq_map[INTERRUPT_XRDP_FPM - SPI_TABLE_OFFSET])
#define INTERRUPT_PCM_DMA_IRQ                (bcm_legacy_irq_map[INTERRUPT_PCM_DMA0      - SPI_TABLE_OFFSET]) 

#ifdef __BOARD_DRV_ARMV7__
unsigned int bcm_phys_irqs_to_map[] =
{
    INTERRUPT_TIMER0,
    INTERRUPT_TIMER1,
    INTERRUPT_TIMER2,
    INTERRUPT_TIMER3,
    INTERRUPT_MEMC_SEC,
    INTERRUPT_DG,
    INTERRUPT_UART0,
    INTERRUPT_HS_UART,
    INTERRUPT_NAND_FLASH,
    INTERRUPT_PER_EXT_0,
    INTERRUPT_PER_EXT_1,
    INTERRUPT_PER_EXT_2,
    INTERRUPT_PER_EXT_3,
    INTERRUPT_PER_EXT_4,
    INTERRUPT_PER_EXT_5,
    INTERRUPT_PER_EXT_6,
    INTERRUPT_PER_EXT_7,
    INTERRUPT_USB_OHCI1,  
    INTERRUPT_USB_EHCI1,  
    INTERRUPT_USB_OHCI,   
    INTERRUPT_USB_EHCI,  
    INTERRUPT_WAN_GPON_RX,
    INTERRUPT_WAN_GPON_TX,
    INTERRUPT_XRDP_QUEUE_0,
    INTERRUPT_XRDP_QUEUE_1,
    INTERRUPT_XRDP_QUEUE_2,
    INTERRUPT_XRDP_QUEUE_3,
    INTERRUPT_XRDP_QUEUE_4,
    INTERRUPT_XRDP_QUEUE_5,
    INTERRUPT_XRDP_QUEUE_6,
    INTERRUPT_XRDP_QUEUE_7,
    INTERRUPT_XRDP_QUEUE_8,
    INTERRUPT_XRDP_QUEUE_9,
    INTERRUPT_XRDP_QUEUE_10,
    INTERRUPT_XRDP_QUEUE_11,
    INTERRUPT_XRDP_QUEUE_12,
    INTERRUPT_XRDP_QUEUE_13,
    INTERRUPT_XRDP_QUEUE_14,
    INTERRUPT_XRDP_QUEUE_15,
    INTERRUPT_XRDP_QUEUE_16,
    INTERRUPT_XRDP_QUEUE_17,
    INTERRUPT_XRDP_QUEUE_18,
    INTERRUPT_XRDP_QUEUE_19,
    INTERRUPT_XRDP_QUEUE_20,
    INTERRUPT_XRDP_QUEUE_21,
    INTERRUPT_XRDP_QUEUE_22,
    INTERRUPT_XRDP_QUEUE_23,
    INTERRUPT_XRDP_QUEUE_24,
    INTERRUPT_XRDP_QUEUE_25,
    INTERRUPT_XRDP_QUEUE_26,
    INTERRUPT_XRDP_QUEUE_27,
    INTERRUPT_XRDP_QUEUE_28,
    INTERRUPT_XRDP_QUEUE_29,
    INTERRUPT_XRDP_QUEUE_30,
    INTERRUPT_XRDP_QUEUE_31,
    INTERRUPT_XRDP_FPM,
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

#endif  /* __BCM6846_H */

