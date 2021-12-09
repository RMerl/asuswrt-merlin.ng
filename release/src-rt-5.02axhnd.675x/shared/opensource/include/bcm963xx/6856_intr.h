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

#ifndef __6856_INTR_H
#define __6856_INTR_H

#ifdef __cplusplus
    extern "C" {
#endif

/*=====================================================================*/
/* SPI Table Offset                                                    */
/*=====================================================================*/
#define SPI_TABLE_OFFSET             32
#define SPI_TABLE_OFFSET1              (SPI_TABLE_OFFSET + 32)
#define SPI_TABLE_OFFSET2              (SPI_TABLE_OFFSET1 + 32)
#define SPI_TABLE_OFFSET3              (SPI_TABLE_OFFSET2 + 32)
#define SPI_TABLE_OFFSET4              (SPI_TABLE_OFFSET3 + 32)
#define SPI_TABLE_OFFSET5              (SPI_TABLE_OFFSET4 + 32)

/*=====================================================================*/
/* Physical Interrupt IDs                                              */
/*=====================================================================*/
    /*  ------------- CHIP_IRQS[31-0] ----------------------------*/
#define INTERRUPT_B53_AXIERRIRQ           (SPI_TABLE_OFFSET + 0)
#define INTERRUPT_B53_INTERRIRQ           (SPI_TABLE_OFFSET + 1)
#define INTERRUPT_B53_CCIERRIRQ           (SPI_TABLE_OFFSET + 4)
#define INTERRUPT_B53_CCIOVFLOWIRQ        (SPI_TABLE_OFFSET + 5)
#define INTERRUPT_THERM_HIGH_IRQ          (SPI_TABLE_OFFSET + 6)
#define INTERRUPT_THERM_LOW_IRQ           (SPI_TABLE_OFFSET + 7)
#define INTERRUPT_THERM_SHUTDOWN          (SPI_TABLE_OFFSET + 8)
#define INTERRUPT_PMUIRQ                  (SPI_TABLE_OFFSET + 9)  /* 2 interrupts */
#define INTERRUPT_PER_TIMER_IRQ0          (SPI_TABLE_OFFSET + 11)
#define INTERRUPT_PER_TIMER_IRQ1          (SPI_TABLE_OFFSET + 12)
#define INTERRUPT_PER_TIMER_IRQ2          (SPI_TABLE_OFFSET + 13)
#define INTERRUPT_PER_TIMER_IRQ3          (SPI_TABLE_OFFSET + 14)
#define INTERRUPT_B53_COMMONIRQ           (SPI_TABLE_OFFSET + 17)
#define INTERRUPT_MEMC_SEC_IRQ            (SPI_TABLE_OFFSET + 18)
#define INTERRUPT_PER_SEC_ACC_VIOL_IRQ    (SPI_TABLE_OFFSET + 20)
#define INTERRUPT_B53_UBUS_RC_IRQ         (SPI_TABLE_OFFSET + 21)
#define INTERRUPT_B53_UBUS_STAT_REG       (SPI_TABLE_OFFSET + 22)
#define INTERRUPT_PMC_TEMP_WARN           (SPI_TABLE_OFFSET + 28)
#define INTERRUPT_DYING_GASP_IRQ          (SPI_TABLE_OFFSET + 29)
#define INTERRUPT_PMC0_IRQ                (SPI_TABLE_OFFSET + 30)
#define INTERRUPT_PMC1_IRQ                (SPI_TABLE_OFFSET + 31)

#define INTERRUPT_PER_UARTINT0            (SPI_TABLE_OFFSET1 + 0)
#define INTERRUPT_PER_UARTINT1            (SPI_TABLE_OFFSET1 + 1)
#define INTERRUPT_PER_HS_SPIM_IRQ         (SPI_TABLE_OFFSET1 + 5)
#define INTERRUPT_NAND_FLASH_IRQ          (SPI_TABLE_OFFSET1 + 6)
#define INTERRUPT_MEMC_IRQ                (SPI_TABLE_OFFSET1 + 7)
#define INTERRUPT_MAC_QEGPHY_CFG_IRQ      (SPI_TABLE_OFFSET1 + 22) /* 4 interrupts */
#define INTERRUPT_PCIE_0_CPU_INTR         (SPI_TABLE_OFFSET1 + 28)
#define INTERRUPT_PCIE_1_CPU_INTR         (SPI_TABLE_OFFSET1 + 29)
#define INTERRUPT_PCIE_2_CPU_INTR         (SPI_TABLE_OFFSET1 + 30)
#define INTERRUPT_UBUS4_SYS_IRQ           (SPI_TABLE_OFFSET1 + 31)

#define INTERRUPT_MBOX_IRQ0_3             (SPI_TABLE_OFFSET2 + 4)  /* 4 interrupts */
#define INTERRUPT_PER_SIM                 (SPI_TABLE_OFFSET2 + 9)
#define INTERRUPT_PER_I2C1_IRQ            (SPI_TABLE_OFFSET2 + 11)
#define INTERRUPT_PER_I2C0_IRQ            (SPI_TABLE_OFFSET2 + 19)
#define INTERRUPT_PER_I2S_IRQ             (SPI_TABLE_OFFSET2 + 20)
#define INTERRUPT_O_RNG_INTR              (SPI_TABLE_OFFSET2 + 21)
#define INTERRUPT_SGMII_SIGDET_IRQ        (SPI_TABLE_OFFSET2 + 25)
#define INTERRUPT_PER_HS_UART             (SPI_TABLE_OFFSET2 + 29)
#define INTERRUPT_PL081_DMA_INTR          (SPI_TABLE_OFFSET2 + 30)
#define INTERRUPT_SDIO_EMMC_L1_INTR       (SPI_TABLE_OFFSET2 + 31)

#define INTERRUPT_WAN_EPON_TOP            (SPI_TABLE_OFFSET3 + 0)
#define INTERRUPT_WAN_NCO_GPON            (SPI_TABLE_OFFSET3 + 1)
#define INTERRUPT_WAN_GPON_TX             (SPI_TABLE_OFFSET3 + 2)
#define INTERRUPT_WAN_GPON_RX             (SPI_TABLE_OFFSET3 + 3)
#define INTERRUPT_WAN_PMD_PLL1_LOCK_INT   (SPI_TABLE_OFFSET3 + 4)
#define INTERRUPT_WAN_PMD_PLL0_LOCK_INT   (SPI_TABLE_OFFSET3 + 5)
#define INTERRUPT_WAN_PMD_SIGNAL_DETECT_0 (SPI_TABLE_OFFSET3 + 6)
#define INTERRUPT_WAN_PMD_ENERGY_DETECT_0 (SPI_TABLE_OFFSET3 + 7)
#define INTERRUPT_WAN_PMD_RX_LOCK_0       (SPI_TABLE_OFFSET3 + 8)
#define INTERRUPT_WAN_XGTX_INTR0          (SPI_TABLE_OFFSET3 + 9)
#define INTERRUPT_WAN_XGTX_INTR1          (SPI_TABLE_OFFSET3 + 10)
#define INTERRUPT_WAN_XGRX                (SPI_TABLE_OFFSET3 + 11)
#define INTERRUPT_WAN_XGTX_INTR2          (SPI_TABLE_OFFSET3 + 12)
#define INTERRUPT_WAN_XGTX_INTR3          (SPI_TABLE_OFFSET3 + 13)
#define INTERRUPT_PCM_DMA_IRQ0            (SPI_TABLE_OFFSET3 + 18) /* PCM DMA RX interrupt */
#define INTERRUPT_PCM_DMA_IRQ1            (SPI_TABLE_OFFSET3 + 19) /* PCM DMA TX interrupt */
#define INTERRUPT_PCM_IRQ                 (SPI_TABLE_OFFSET3 + 20) /* 2 interrupts */
#define INTERRUPT_PER_MDIO_ERR_IRQ        (SPI_TABLE_OFFSET3 + 22)
#define INTERRUPT_PER_MDIO_DONE_IRQ       (SPI_TABLE_OFFSET3 + 23)
#define INTERRUPT_USB_USBD                (SPI_TABLE_OFFSET3 + 24)
#define INTERRUPT_USB_XHCI                (SPI_TABLE_OFFSET3 + 25)
#define INTERRUPT_USB_OHCI1               (SPI_TABLE_OFFSET3 + 26)
#define INTERRUPT_USB_EHCI1               (SPI_TABLE_OFFSET3 + 27)
#define INTERRUPT_USB_OHCI                (SPI_TABLE_OFFSET3 + 28)
#define INTERRUPT_USB_EHCI                (SPI_TABLE_OFFSET3 + 29)
#define INTERRUPT_USB_EVENTS              (SPI_TABLE_OFFSET3 + 30)
#define INTERRUPT_USB_BRIDGE          (   (SPI_TABLE_OFFSET3 + 31)

#define INTERRUPT_XRDP_QUEUE_0            (SPI_TABLE_OFFSET4 + 0)
#define INTERRUPT_XRDP_QUEUE_1            (SPI_TABLE_OFFSET4 + 1)
#define INTERRUPT_XRDP_QUEUE_2            (SPI_TABLE_OFFSET4 + 2)
#define INTERRUPT_XRDP_QUEUE_3            (SPI_TABLE_OFFSET4 + 3)
#define INTERRUPT_XRDP_QUEUE_4            (SPI_TABLE_OFFSET4 + 4)
#define INTERRUPT_XRDP_QUEUE_5            (SPI_TABLE_OFFSET4 + 5)
#define INTERRUPT_XRDP_QUEUE_6            (SPI_TABLE_OFFSET4 + 6)
#define INTERRUPT_XRDP_QUEUE_7            (SPI_TABLE_OFFSET4 + 7)
#define INTERRUPT_XRDP_QUEUE_8            (SPI_TABLE_OFFSET4 + 8)
#define INTERRUPT_XRDP_QUEUE_9            (SPI_TABLE_OFFSET4 + 9)
#define INTERRUPT_XRDP_QUEUE_10           (SPI_TABLE_OFFSET4 + 10)
#define INTERRUPT_XRDP_QUEUE_11           (SPI_TABLE_OFFSET4 + 11)
#define INTERRUPT_XRDP_QUEUE_12           (SPI_TABLE_OFFSET4 + 12)
#define INTERRUPT_XRDP_QUEUE_13           (SPI_TABLE_OFFSET4 + 13)
#define INTERRUPT_XRDP_QUEUE_14           (SPI_TABLE_OFFSET4 + 14)
#define INTERRUPT_XRDP_QUEUE_15           (SPI_TABLE_OFFSET4 + 15)
#define INTERRUPT_XRDP_QUEUE_16           (SPI_TABLE_OFFSET4 + 16)
#define INTERRUPT_XRDP_QUEUE_17           (SPI_TABLE_OFFSET4 + 17)
#define INTERRUPT_XRDP_QUEUE_18           (SPI_TABLE_OFFSET4 + 18)
#define INTERRUPT_XRDP_QUEUE_19           (SPI_TABLE_OFFSET4 + 19)
#define INTERRUPT_XRDP_QUEUE_20           (SPI_TABLE_OFFSET4 + 20)
#define INTERRUPT_XRDP_QUEUE_21           (SPI_TABLE_OFFSET4 + 21)
#define INTERRUPT_XRDP_QUEUE_22           (SPI_TABLE_OFFSET4 + 22)
#define INTERRUPT_XRDP_QUEUE_23           (SPI_TABLE_OFFSET4 + 23)
#define INTERRUPT_XRDP_QUEUE_24           (SPI_TABLE_OFFSET4 + 24)
#define INTERRUPT_XRDP_QUEUE_25           (SPI_TABLE_OFFSET4 + 25)
#define INTERRUPT_XRDP_QUEUE_26           (SPI_TABLE_OFFSET4 + 26)
#define INTERRUPT_XRDP_QUEUE_27           (SPI_TABLE_OFFSET4 + 27)
#define INTERRUPT_XRDP_QUEUE_28           (SPI_TABLE_OFFSET4 + 28)
#define INTERRUPT_XRDP_QUEUE_29           (SPI_TABLE_OFFSET4 + 29)
#define INTERRUPT_XRDP_QUEUE_30           (SPI_TABLE_OFFSET4 + 30)
#define INTERRUPT_XRDP_QUEUE_31           (SPI_TABLE_OFFSET4 + 31)

#define INTERRUPT_XRDP_FPM                (SPI_TABLE_OFFSET5 + 0)
#define INTERRUPT_PER_EXT_0               (SPI_TABLE_OFFSET5 + 21)
#define INTERRUPT_PER_EXT_1               (SPI_TABLE_OFFSET5 + 22)
#define INTERRUPT_PER_EXT_2               (SPI_TABLE_OFFSET5 + 23)
#define INTERRUPT_PER_EXT_3               (SPI_TABLE_OFFSET5 + 24)
#define INTERRUPT_PER_EXT_4               (SPI_TABLE_OFFSET5 + 25)
#define INTERRUPT_PER_EXT_5               (SPI_TABLE_OFFSET5 + 26)
#define INTERRUPT_PER_EXT_6               (SPI_TABLE_OFFSET5 + 27)
#define INTERRUPT_PER_EXT_7               (SPI_TABLE_OFFSET5 + 28)

#ifndef __ASSEMBLER__
/*=====================================================================*/
/* Linux(Virtual) Interrupt IDs                                        */
/* Each physical irq id to be mapped should be added to                */
/* bcm_phys_irqs_to_map array in board_aarch64.c file                  */
/*=====================================================================*/
#define INTERRUPT_ID_RANGE_CHECK          (bcm_legacy_irq_map[INTERRUPT_MEMC_SEC_IRQ - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_DG                   (bcm_legacy_irq_map[INTERRUPT_DYING_GASP_IRQ - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_UART                 (bcm_legacy_irq_map[INTERRUPT_PER_UARTINT0 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_UART1                (bcm_legacy_irq_map[INTERRUPT_PER_UARTINT1 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_HS_UART              (bcm_legacy_irq_map[INTERRUPT_PER_HS_UART - SPI_TABLE_OFFSET])
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
#define INTERRUPT_ID_EXTERNAL_6           (bcm_legacy_irq_map[INTERRUPT_PER_EXT_6 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_EXTERNAL_7           (bcm_legacy_irq_map[INTERRUPT_PER_EXT_7 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_EXTERNAL_MAX         INTERRUPT_ID_EXTERNAL_7
#define INTERRUPT_ID_I2S                  (bcm_legacy_irq_map[INTERRUPT_PER_I2S_IRQ - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_USB_XHCI             (bcm_legacy_irq_map[INTERRUPT_USB_XHCI - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_USB_OHCI             (bcm_legacy_irq_map[INTERRUPT_USB_OHCI - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_USB_EHCI             (bcm_legacy_irq_map[INTERRUPT_USB_EHCI - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_USB_OHCI1            (bcm_legacy_irq_map[INTERRUPT_USB_OHCI1 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_USB_EHCI1            (bcm_legacy_irq_map[INTERRUPT_USB_EHCI1 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_NAND_FLASH           (bcm_legacy_irq_map[INTERRUPT_NAND_FLASH_IRQ - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_WAN_GPON_RX          (bcm_legacy_irq_map[INTERRUPT_WAN_GPON_RX - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_WAN_GPON_TX          (bcm_legacy_irq_map[INTERRUPT_WAN_GPON_TX - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_WAN_XGTX_INTR0       (bcm_legacy_irq_map[INTERRUPT_WAN_XGTX_INTR0 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_WAN_XGTX_INTR1       (bcm_legacy_irq_map[INTERRUPT_WAN_XGTX_INTR1 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_WAN_XGRX             (bcm_legacy_irq_map[INTERRUPT_WAN_XGRX - SPI_TABLE_OFFSET])
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
#define INTERRUPT_PCM_DMA_IRQ                (bcm_legacy_irq_map[INTERRUPT_PCM_DMA_IRQ0 - SPI_TABLE_OFFSET]) 

#ifdef __BOARD_DRV_AARCH64__
// add here any legacy driver's (driver that have no device tree node) interrupt to be mapped
unsigned int bcm_phys_irqs_to_map[] =
{
    INTERRUPT_MEMC_SEC_IRQ,
    INTERRUPT_DYING_GASP_IRQ,
    INTERRUPT_PER_UARTINT0,
    INTERRUPT_PER_UARTINT1,
    INTERRUPT_PER_HS_UART,
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
    INTERRUPT_PER_EXT_6,
    INTERRUPT_PER_EXT_7,
    INTERRUPT_NAND_FLASH_IRQ,
    INTERRUPT_PER_I2S_IRQ,
    INTERRUPT_WAN_GPON_RX,
    INTERRUPT_WAN_GPON_TX,
    INTERRUPT_WAN_XGTX_INTR0,
    INTERRUPT_WAN_XGTX_INTR1,
    INTERRUPT_WAN_XGRX,
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
    INTERRUPT_PCM_DMA_IRQ0,    
    INTERRUPT_USB_XHCI,
    INTERRUPT_USB_OHCI,
    INTERRUPT_USB_EHCI,
    INTERRUPT_USB_OHCI1,
    INTERRUPT_USB_EHCI1
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

#endif  /* __BCM6856_H */

