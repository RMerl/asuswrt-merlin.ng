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

#ifndef __4908_INTR_H
#define __4908_INTR_H

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
#define INTERRUPT_B53_TRAPAXI0            (SPI_TABLE_OFFSET + 0)
#define INTERRUPT_B53_TRAPAXI1            (SPI_TABLE_OFFSET + 1)
#define INTERRUPT_B53_INTR_ERROR0         (SPI_TABLE_OFFSET + 2)
#define INTERRUPT_B53_INTR_ERROR1         (SPI_TABLE_OFFSET + 3)
#define INTERRUPT_B53_ACC_VIOLATION0      (SPI_TABLE_OFFSET + 4)
#define INTERRUPT_B53_ACC_VIOLATION1      (SPI_TABLE_OFFSET + 5)
#define INTERRUPT_THERM_HIGH_IRQ          (SPI_TABLE_OFFSET + 6)
#define INTERRUPT_THERM_LOW_IRQ           (SPI_TABLE_OFFSET + 7)
#define INTERRUPT_THERM_SHUTDOWN          (SPI_TABLE_OFFSET + 8)
#define INTERRUPT_PMUIRQ0                 (SPI_TABLE_OFFSET + 9)
#define INTERRUPT_PMUIRQ1                 (SPI_TABLE_OFFSET + 10)
#define INTERRUPT_PMUIRQ2                 (SPI_TABLE_OFFSET + 11)
#define INTERRUPT_PMUIRQ3                 (SPI_TABLE_OFFSET + 12)
#define INTERRUPT_PER_TIMER_IRQ4          (SPI_TABLE_OFFSET + 16)
#define INTERRUPT_MEMC_SEC_IRQ            (SPI_TABLE_OFFSET + 18)
#define INTERRUPT_PER_SEC_ACC_VIOL_IRQ    (SPI_TABLE_OFFSET + 20)
#define INTERRUPT_UBUS_P6ERR              (SPI_TABLE_OFFSET + 22)
#define INTERRUPT_UBUS_ERR                (SPI_TABLE_OFFSET + 23)
#define INTERRUPT_DYING_GASP_IRQ          (SPI_TABLE_OFFSET + 29)
#define INTERRUPT_PMC_MIPS_IRQ            (SPI_TABLE_OFFSET + 30)
#define INTERRUPT_PMC_MIPS1_IRQ           (SPI_TABLE_OFFSET + 31)
/*  ------------- CHIP_IRQS[63-32] ----------------------------*/
#define INTERRUPT_PER_UARTINT0            (SPI_TABLE_OFFSET + 32)
#define INTERRUPT_PER_HS_UARTINT          (SPI_TABLE_OFFSET + 34)
#define INTERRUPT_PER_HS_SPIM_IRQ         (SPI_TABLE_OFFSET + 36)
#define INTERRUPT_NAND_FLASH_IRQ          (SPI_TABLE_OFFSET + 37)
#define INTERRUPT_MEMC_IRQ                (SPI_TABLE_OFFSET + 38)
#define INTERRUPT_USBD_IRQ                (SPI_TABLE_OFFSET + 41)
#define INTERRUPT_PCM_IRQ                 (SPI_TABLE_OFFSET + 42)
#define INTERRUPT_SATA_CPU_INTR0          (SPI_TABLE_OFFSET + 43)
#define INTERRUPT_SATA_CPU_INTR1          (SPI_TABLE_OFFSET + 44)
#define INTERRUPT_RUNNER_INTR0            (SPI_TABLE_OFFSET + 45)
#define INTERRUPT_RUNNER_INTR1            (SPI_TABLE_OFFSET + 46)
#define INTERRUPT_RUNNER_INTR2            (SPI_TABLE_OFFSET + 47)
#define INTERRUPT_RUNNER_INTR3            (SPI_TABLE_OFFSET + 48)
#define INTERRUPT_RUNNER_INTR4            (SPI_TABLE_OFFSET + 49)
#define INTERRUPT_RUNNER_INTR5            (SPI_TABLE_OFFSET + 50)
#define INTERRUPT_RUNNER_INTR6            (SPI_TABLE_OFFSET + 51)
#define INTERRUPT_RUNNER_INTR7            (SPI_TABLE_OFFSET + 52)
#define INTERRUPT_RUNNER_INTR8            (SPI_TABLE_OFFSET + 53)
#define INTERRUPT_RUNNER_INTR9            (SPI_TABLE_OFFSET + 54)
#define INTERRUPT_SBPM_INTR               (SPI_TABLE_OFFSET + 55)
#define INTERRUPT_FPM_INTR                (SPI_TABLE_OFFSET + 56)
#define INTERRUPT_SW_INTR0_CPUOUT         (SPI_TABLE_OFFSET + 57)
#define INTERRUPT_SW_INTR1_CPUOUT         (SPI_TABLE_OFFSET + 58)
#define INTERRUPT_PCIE_0_CPU_INTR         (SPI_TABLE_OFFSET + 59) 
#define INTERRUPT_PCIE_1_CPU_INTR         (SPI_TABLE_OFFSET + 60)  
#define INTERRUPT_PCIE_2_CPU_INTR         (SPI_TABLE_OFFSET + 61)  
/*  ------------- CHIP_IRQS[95-64] ----------------------------*/
#define INTERRUPT_PER_TIMER_IRQ0          (SPI_TABLE_OFFSET + 64)
#define INTERRUPT_PER_TIMER_IRQ1          (SPI_TABLE_OFFSET + 65)
#define INTERRUPT_PER_TIMER_IRQ2          (SPI_TABLE_OFFSET + 66)
#define INTERRUPT_PER_TIMER_IRQ3          (SPI_TABLE_OFFSET + 67)
#define INTERRUPT_MBOX_IRQ0               (SPI_TABLE_OFFSET + 68)
#define INTERRUPT_MBOX_IRQ1               (SPI_TABLE_OFFSET + 69)
#define INTERRUPT_MBOX_IRQ2               (SPI_TABLE_OFFSET + 70)
#define INTERRUPT_MBOX_IRQ3               (SPI_TABLE_OFFSET + 71)
#define INTERRUPT_USB_OHCI                (SPI_TABLE_OFFSET + 72)
#define INTERRUPT_USB_EHCI                (SPI_TABLE_OFFSET + 73)
#define INTERRUPT_USB_XHCI                (SPI_TABLE_OFFSET + 74)
#define INTERRUPT_USB_HBR                 (SPI_TABLE_OFFSET + 75)
#define INTERRUPT_USB_HEV                 (SPI_TABLE_OFFSET + 76)
#define INTERRUPT_PER_I2C                 (SPI_TABLE_OFFSET + 77)
#define INTERRUPT_PER_I2S                 (SPI_TABLE_OFFSET + 78)
#define INTERRUPT_PER_RNG                 (SPI_TABLE_OFFSET + 79)
#define INTERRUPT_M2M_CH_INTR0            (SPI_TABLE_OFFSET + 80)
#define INTERRUPT_M2M_CH_INRT1            (SPI_TABLE_OFFSET + 81)
#define INTERRUPT_M2M_CH_INRT2            (SPI_TABLE_OFFSET + 82)
#define INTERRUPT_M2M_CH_INRT3            (SPI_TABLE_OFFSET + 83)
#define INTERRUPT_PL081_DMA               (SPI_TABLE_OFFSET + 84)
#define INTERRUPT_SDIO_EMMC               (SPI_TABLE_OFFSET + 85)
#define INTERRUPT_GMAC_DMA_IRQ0           (SPI_TABLE_OFFSET + 86)
#define INTERRUPT_GMAC_DMA_IRQ1           (SPI_TABLE_OFFSET + 87)
#define INTERRUPT_GMAC_IRQ                (SPI_TABLE_OFFSET + 88)
#define INTERRUPT_CTF_IRQ                 (SPI_TABLE_OFFSET + 89)
#define INTERRUPT_SPU_GMAC_IRQ            (SPI_TABLE_OFFSET + 90)
/*  ------------- CHIP_IRQS[127-96] ----------------------------*/
#define INTERRUPT_PCM_DMA_IRQ0            (SPI_TABLE_OFFSET + 116)
#define INTERRUPT_PCM_DMA_IRQ1            (SPI_TABLE_OFFSET + 117)
#define INTERRUPT_DQM_IRQ0                (SPI_TABLE_OFFSET + 118)
#define INTERRUPT_DQM_IRQ1                (SPI_TABLE_OFFSET + 119)
#define INTERRUPT_DQM_IRQ2                (SPI_TABLE_OFFSET + 120)
#define INTERRUPT_DQM_IRQ3                (SPI_TABLE_OFFSET + 121)
#define INTERRUPT_PER_EXT_0               (SPI_TABLE_OFFSET + 122)
#define INTERRUPT_PER_EXT_1               (SPI_TABLE_OFFSET + 123)
#define INTERRUPT_PER_EXT_2               (SPI_TABLE_OFFSET + 124)
#define INTERRUPT_PER_EXT_3               (SPI_TABLE_OFFSET + 125)
#define INTERRUPT_PER_EXT_4               (SPI_TABLE_OFFSET + 126)
#define INTERRUPT_PER_EXT_5               (SPI_TABLE_OFFSET + 127)

#ifndef __ASSEMBLER__
/*========================================================================*/
/* Linux(Virtual) Interrupt IDs for Legacy drivers                        */ 
/* -Legacy Drivers with no DT support should retrieve VIRQ ids from these */
/*  defines                                                               */ 
/* -If a legacy driver requires a VIRQ, its corresponding physical irq id */
/*  must be placed in bcm_phys_irqs_to_map array below                    */
/*========================================================================*/

#define INTERRUPT_ID_DG                   (bcm_legacy_irq_map[INTERRUPT_DYING_GASP_IRQ - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_UART0                (bcm_legacy_irq_map[INTERRUPT_PER_UARTINT0 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_UART                 INTERRUPT_ID_UART0
#define INTERRUPT_ID_HS_UART              (bcm_legacy_irq_map[INTERRUPT_PER_HS_UARTINT - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_SATAC                (bcm_legacy_irq_map[INTERRUPT_SATA_CPU_INTR1 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_RUNNER_0             (bcm_legacy_irq_map[INTERRUPT_RUNNER_INTR0 - SPI_TABLE_OFFSET]) 
#define INTERRUPT_ID_RUNNER_1             (bcm_legacy_irq_map[INTERRUPT_RUNNER_INTR1 - SPI_TABLE_OFFSET]) 
#define INTERRUPT_ID_RUNNER_2             (bcm_legacy_irq_map[INTERRUPT_RUNNER_INTR2 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_RUNNER_3             (bcm_legacy_irq_map[INTERRUPT_RUNNER_INTR3 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_RUNNER_4             (bcm_legacy_irq_map[INTERRUPT_RUNNER_INTR4 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_RUNNER_5             (bcm_legacy_irq_map[INTERRUPT_RUNNER_INTR5 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_RUNNER_6             (bcm_legacy_irq_map[INTERRUPT_RUNNER_INTR6 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_RUNNER_7             (bcm_legacy_irq_map[INTERRUPT_RUNNER_INTR7 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_RUNNER_8             (bcm_legacy_irq_map[INTERRUPT_RUNNER_INTR8 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_RUNNER_9             (bcm_legacy_irq_map[INTERRUPT_RUNNER_INTR9 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_FPM                  (bcm_legacy_irq_map[INTERRUPT_FPM_INTR - SPI_TABLE_OFFSET]) 
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
#define INTERRUPT_ID_GMAC_DMA_0           (bcm_legacy_irq_map[INTERRUPT_GMAC_DMA_IRQ0 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_GMAC_DMA_1           (bcm_legacy_irq_map[INTERRUPT_GMAC_DMA_IRQ1 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_GMAC                 (bcm_legacy_irq_map[INTERRUPT_GMAC_IRQ - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_I2S                  (bcm_legacy_irq_map[INTERRUPT_PER_I2S - SPI_TABLE_OFFSET])
#define INTERRUPT_PCM_DMA_IRQ             (bcm_legacy_irq_map[INTERRUPT_PCM_DMA_IRQ0 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_NAND_FLASH           (bcm_legacy_irq_map[INTERRUPT_NAND_FLASH_IRQ - SPI_TABLE_OFFSET])

#ifdef __BOARD_DRV_AARCH64__
// add here any legacy driver's (driver that have no device tree node) interrupt to be mapped
unsigned int bcm_phys_irqs_to_map[] =
{
    INTERRUPT_DYING_GASP_IRQ,
    INTERRUPT_PER_UARTINT0,
    INTERRUPT_PER_HS_UARTINT,
    INTERRUPT_RUNNER_INTR0,
    INTERRUPT_RUNNER_INTR1,
    INTERRUPT_RUNNER_INTR2,
    INTERRUPT_RUNNER_INTR3,
    INTERRUPT_RUNNER_INTR4,
    INTERRUPT_RUNNER_INTR5,
    INTERRUPT_RUNNER_INTR6,
    INTERRUPT_RUNNER_INTR7,
    INTERRUPT_RUNNER_INTR8,
    INTERRUPT_RUNNER_INTR9,
    INTERRUPT_FPM_INTR,
    INTERRUPT_PER_TIMER_IRQ0,
    INTERRUPT_PER_TIMER_IRQ1,
    INTERRUPT_PER_TIMER_IRQ2,
    INTERRUPT_PER_TIMER_IRQ3,
    INTERRUPT_GMAC_DMA_IRQ0,
    INTERRUPT_GMAC_DMA_IRQ1,
    INTERRUPT_USB_XHCI,
    INTERRUPT_USB_OHCI,
    INTERRUPT_USB_EHCI,
    INTERRUPT_SATA_CPU_INTR1,
    INTERRUPT_GMAC_IRQ,
    INTERRUPT_PER_I2S,
    INTERRUPT_PCM_DMA_IRQ0,
    INTERRUPT_PER_EXT_0,
    INTERRUPT_PER_EXT_1,
    INTERRUPT_PER_EXT_2,
    INTERRUPT_PER_EXT_3,
    INTERRUPT_PER_EXT_4,
    INTERRUPT_PER_EXT_5,
    INTERRUPT_NAND_FLASH_IRQ
};
unsigned int bcm_legacy_irq_map[256];
#else
extern unsigned int bcm_phys_irqs_to_map[];
extern unsigned int bcm_legacy_irq_map[];
#endif

#endif

#define NUM_EXT_INT    (INTERRUPT_PER_EXT_5-INTERRUPT_PER_EXT_0+1)

#ifdef __cplusplus
}
#endif

#endif /* __BCM4908_H */
