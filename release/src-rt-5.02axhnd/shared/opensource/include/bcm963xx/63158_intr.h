/*
<:copyright-BRCM:2016:DUAL/GPL:standard

   Copyright (c) 2016 Broadcom 
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

#ifndef __63158_INTR_H
#define __63158_INTR_H

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
#define INTERRUPT_B53_CCIERRIRQ           (SPI_TABLE_OFFSET + 2)
#define INTERRUPT_B53_CCIOVFLOWIRQ        (SPI_TABLE_OFFSET + 3)
#define INTERRUPT_B53_UBUS_RC_IRQ         (SPI_TABLE_OFFSET + 4)
#define INTERRUPT_B53_UBUS_STAT_IRQ       (SPI_TABLE_OFFSET + 5)
#define INTERRUPT_THERM_HIGH_IRQ          (SPI_TABLE_OFFSET + 6)
#define INTERRUPT_THERM_LOW_IRQ           (SPI_TABLE_OFFSET + 7)
#define INTERRUPT_THERM_SHUTDOWN          (SPI_TABLE_OFFSET + 8)
#define INTERRUPT_PMUIRQ0                 (SPI_TABLE_OFFSET + 9)
#define INTERRUPT_PMUIRQ1                 (SPI_TABLE_OFFSET + 10)
#define INTERRUPT_PMUIRQ2                 (SPI_TABLE_OFFSET + 11)
#define INTERRUPT_PMUIRQ3                 (SPI_TABLE_OFFSET + 12)
#define INTERRUPT_PER_TIMER_IRQ4          (SPI_TABLE_OFFSET + 15)
#define INTERRUPT_PER_TIMER_IRQ5          (SPI_TABLE_OFFSET + 16)
#define INTERRUPT_B53_COMMONIRQ           (SPI_TABLE_OFFSET + 17)
#define INTERRUPT_MEMC_SEC_IRQ            (SPI_TABLE_OFFSET + 18)
#define INTERRUPT_ORION_INT_PENDING       (SPI_TABLE_OFFSET + 19)
#define INTERRUPT_PER_SEC_ACC_VIOL_IRQ    (SPI_TABLE_OFFSET + 20)
#define INTERRUPT_UBUS4_SYS_IRQ           (SPI_TABLE_OFFSET + 22)
#define INTERRUPT_XPORT_IRQ               (SPI_TABLE_OFFSET + 27)
#define INTERRUPT_DYING_GASP_IRQ          (SPI_TABLE_OFFSET + 29)
#define INTERRUPT_PMC_MIPS_IRQ            (SPI_TABLE_OFFSET + 30)
#define INTERRUPT_PMC_MIPS1_IRQ           (SPI_TABLE_OFFSET + 31)

/*  ------------- CHIP_IRQS[63-32] ----------------------------*/
#define INTERRUPT_PER_UARTINT0            (SPI_TABLE_OFFSET + 32)
#define INTERRUPT_PER_UARTINT1            (SPI_TABLE_OFFSET + 33)
#define INTERRUPT_PER_HS_UARTINT          (SPI_TABLE_OFFSET + 34)
#define INTERRUPT_PER_UARTINT2            (SPI_TABLE_OFFSET + 35)
#define INTERRUPT_PER_INTERNAL_ERR        (SPI_TABLE_OFFSET + 36)
#define INTERRUPT_PER_HS_SPIM_IRQ         (SPI_TABLE_OFFSET + 37)
#define INTERRUPT_NAND_FLASH_IRQ          (SPI_TABLE_OFFSET + 38)
#define INTERRUPT_MEMC_IRQ                (SPI_TABLE_OFFSET + 39)
#define INTERRUPT_VDSL                    (SPI_TABLE_OFFSET + 40)
#define INTERRUPT_SARC                    (SPI_TABLE_OFFSET + 41)
#define INTERRUPT_PER_DMA_IRQ2            (SPI_TABLE_OFFSET + 42)
#define INTERRUPT_PER_DMA_IRQ1            (SPI_TABLE_OFFSET + 43)
#define INTERRUPT_SATA_CPU_INTR0          (SPI_TABLE_OFFSET + 44)
#define INTERRUPT_SATA_CPU_INTR1          (SPI_TABLE_OFFSET + 45)
#define INTERRUPT_SYSTEMPORT_INTR0        (SPI_TABLE_OFFSET + 47)
#define INTERRUPT_SYSTEMPORT_INTR1        (SPI_TABLE_OFFSET + 48)
#define INTERRUPT_SYSTEMPORT_WOL          (SPI_TABLE_OFFSET + 49)
#define INTERRUPT_XRDP_SYSPORT            (SPI_TABLE_OFFSET + 52)
#define INTERRUPT_SW_INTR0_CPUOUT         (SPI_TABLE_OFFSET + 58)
#define INTERRUPT_SW_INTR1_CPUOUT         (SPI_TABLE_OFFSET + 59)
#define INTERRUPT_PCIE_0_CPU_INTR         (SPI_TABLE_OFFSET + 60) 
#define INTERRUPT_PCIE_1_CPU_INTR         (SPI_TABLE_OFFSET + 61) 
#define INTERRUPT_PCIE_2_CPU_INTR         (SPI_TABLE_OFFSET + 62) 
#define INTERRUPT_PCIE_3_CPU_INTR         (SPI_TABLE_OFFSET + 63) 

/*  ------------- CHIP_IRQS[95-64] ----------------------------*/
#define INTERRUPT_PER_TIMER_IRQ0          (SPI_TABLE_OFFSET + 64)
#define INTERRUPT_PER_TIMER_IRQ1          (SPI_TABLE_OFFSET + 65)
#define INTERRUPT_PER_TIMER_IRQ2          (SPI_TABLE_OFFSET + 66)
#define INTERRUPT_PER_TIMER_IRQ3          (SPI_TABLE_OFFSET + 67)
#define INTERRUPT_MBOX_IRQ0               (SPI_TABLE_OFFSET + 68)
#define INTERRUPT_MBOX_IRQ1               (SPI_TABLE_OFFSET + 69)
#define INTERRUPT_MBOX_IRQ2               (SPI_TABLE_OFFSET + 70)
#define INTERRUPT_MBOX_IRQ3               (SPI_TABLE_OFFSET + 71)
#define INTERRUPT_SPU_GMAC_IRQ            (SPI_TABLE_OFFSET + 75)
#define INTERRUPT_SPU_CTF_IRQ             (SPI_TABLE_OFFSET + 76)
#define INTERRUPT_SPU_SKP_IRQ             (SPI_TABLE_OFFSET + 77)

/* interrupt mapping issue on 63158 A0 */   
#if defined(CONFIG_BCM963158)
#define INTERRUPT_PCM_IRQ                 (SPI_TABLE_OFFSET + 80)
#define INTERRUPT_PCM_DMA_IRQ0            (SPI_TABLE_OFFSET + 81)
#else
#define INTERRUPT_PCM_DMA_IRQ0            (SPI_TABLE_OFFSET + 78)
#define INTERRUPT_PCM_DMA_IRQ1            (SPI_TABLE_OFFSET + 79)
#define INTERRUPT_PCM_IRQ                 (SPI_TABLE_OFFSET + 80)
#define INTERRUPT_PCM_IRQ1                (SPI_TABLE_OFFSET + 81)
#endif


#define INTERRUPT_PER_I2C                 (SPI_TABLE_OFFSET + 82)
#define INTERRUPT_PER_I2C1                (SPI_TABLE_OFFSET + 83)
#define INTERRUPT_PER_I2S                 (SPI_TABLE_OFFSET + 84)
#define INTERRUPT_PER_RNG                 (SPI_TABLE_OFFSET + 85)
#define INTERRUPT_PL081_DMA               (SPI_TABLE_OFFSET + 94)
#define INTERRUPT_SDIO_EMMC               (SPI_TABLE_OFFSET + 95)

/*  ------------- CHIP_IRQS[127-96] ----------------------------*/
#define INTERRUPT_WAN_EPON_TOP            (SPI_TABLE_OFFSET + 96)
#define INTERRUPT_WAN_NCO_GPON            (SPI_TABLE_OFFSET + 97)
#define INTERRUPT_WAN_GPON_TX             (SPI_TABLE_OFFSET + 98)
#define INTERRUPT_WAN_GPON_RX             (SPI_TABLE_OFFSET + 99)
#define INTERRUPT_WAN_PMD_PLL1_LOCK_INT   (SPI_TABLE_OFFSET + 100)
#define INTERRUPT_WAN_PMD_PLL0_LOCK_INT   (SPI_TABLE_OFFSET + 101)
#define INTERRUPT_WAN_PMD_SIGNAL_DETECT_0 (SPI_TABLE_OFFSET + 102)
#define INTERRUPT_WAN_PMD_ENERGY_DETECT_0 (SPI_TABLE_OFFSET + 103)
#define INTERRUPT_WAN_PMD_RX_LOCK_0       (SPI_TABLE_OFFSET + 104)
#define INTERRUPT_WAN_RX_STATUS           (SPI_TABLE_OFFSET + 105)
#define INTERRUPT_XRDP_FPM                (SPI_TABLE_OFFSET + 106)
#define INTERRUPT_XRDP_HASH               (SPI_TABLE_OFFSET + 106)
#define INTERRUPT_XRDP_QM                 (SPI_TABLE_OFFSET + 108)
#define INTERRUPT_XRDP_DSPTCHR            (SPI_TABLE_OFFSET + 109)
#define INTERRUPT_XRDP_SBPM               (SPI_TABLE_OFFSET + 110)
#define INTERRUPT_RUNNER_INTR0            (SPI_TABLE_OFFSET + 111)
#define INTERRUPT_RUNNER_INTR1            (SPI_TABLE_OFFSET + 112)
#define INTERRUPT_RUNNER_INTR2            (SPI_TABLE_OFFSET + 113)
#define INTERRUPT_RUNNER_INTR3            (SPI_TABLE_OFFSET + 114)
#define INTERRUPT_RUNNER_INTR4            (SPI_TABLE_OFFSET + 115)
#define INTERRUPT_RUNNER_INTR5            (SPI_TABLE_OFFSET + 116)
#define INTERRUPT_USBD_IRQ                (SPI_TABLE_OFFSET + 120)
#define INTERRUPT_USB_OHCI1               (SPI_TABLE_OFFSET + 121)
#define INTERRUPT_USB_EHCI1               (SPI_TABLE_OFFSET + 122)
#define INTERRUPT_USB_XHCI                (SPI_TABLE_OFFSET + 123)
#define INTERRUPT_USB_OHCI                (SPI_TABLE_OFFSET + 124)
#define INTERRUPT_USB_EHCI                (SPI_TABLE_OFFSET + 125)
#define INTERRUPT_USB_HEV                 (SPI_TABLE_OFFSET + 126)
#define INTERRUPT_USB_HBR                 (SPI_TABLE_OFFSET + 127)

/*  ------------- CHIP_IRQS[159-128] ----------------------------*/
#define INTERRUPT_XRDP_QUEUE_0            (SPI_TABLE_OFFSET + 128)
#define INTERRUPT_XRDP_QUEUE_1            (SPI_TABLE_OFFSET + 129)
#define INTERRUPT_XRDP_QUEUE_2            (SPI_TABLE_OFFSET + 130)
#define INTERRUPT_XRDP_QUEUE_3            (SPI_TABLE_OFFSET + 131)
#define INTERRUPT_XRDP_QUEUE_4            (SPI_TABLE_OFFSET + 132)
#define INTERRUPT_XRDP_QUEUE_5            (SPI_TABLE_OFFSET + 133)
#define INTERRUPT_XRDP_QUEUE_6            (SPI_TABLE_OFFSET + 134)
#define INTERRUPT_XRDP_QUEUE_7            (SPI_TABLE_OFFSET + 135)
#define INTERRUPT_XRDP_QUEUE_8            (SPI_TABLE_OFFSET + 136)
#define INTERRUPT_XRDP_QUEUE_9            (SPI_TABLE_OFFSET + 137)
#define INTERRUPT_XRDP_QUEUE_10           (SPI_TABLE_OFFSET + 138)
#define INTERRUPT_XRDP_QUEUE_11           (SPI_TABLE_OFFSET + 139)
#define INTERRUPT_XRDP_QUEUE_12           (SPI_TABLE_OFFSET + 140)
#define INTERRUPT_XRDP_QUEUE_13           (SPI_TABLE_OFFSET + 141)
#define INTERRUPT_XRDP_QUEUE_14           (SPI_TABLE_OFFSET + 142)
#define INTERRUPT_XRDP_QUEUE_15           (SPI_TABLE_OFFSET + 143)
#define INTERRUPT_XRDP_QUEUE_16           (SPI_TABLE_OFFSET + 144)
#define INTERRUPT_XRDP_QUEUE_17           (SPI_TABLE_OFFSET + 145)
#define INTERRUPT_XRDP_QUEUE_18           (SPI_TABLE_OFFSET + 146)
#define INTERRUPT_XRDP_QUEUE_19           (SPI_TABLE_OFFSET + 147)
#define INTERRUPT_XRDP_QUEUE_20           (SPI_TABLE_OFFSET + 148)
#define INTERRUPT_XRDP_QUEUE_21           (SPI_TABLE_OFFSET + 149)
#define INTERRUPT_XRDP_QUEUE_22           (SPI_TABLE_OFFSET + 150)
#define INTERRUPT_XRDP_QUEUE_23           (SPI_TABLE_OFFSET + 151)
#define INTERRUPT_XRDP_QUEUE_24           (SPI_TABLE_OFFSET + 152)
#define INTERRUPT_XRDP_QUEUE_25           (SPI_TABLE_OFFSET + 153)
#define INTERRUPT_XRDP_QUEUE_26           (SPI_TABLE_OFFSET + 154)
#define INTERRUPT_XRDP_QUEUE_27           (SPI_TABLE_OFFSET + 155)
#define INTERRUPT_XRDP_QUEUE_28           (SPI_TABLE_OFFSET + 156)
#define INTERRUPT_XRDP_QUEUE_29           (SPI_TABLE_OFFSET + 157)
#define INTERRUPT_XRDP_QUEUE_30           (SPI_TABLE_OFFSET + 158)
#define INTERRUPT_XRDP_QUEUE_31           (SPI_TABLE_OFFSET + 159)

/*  ------------- CHIP_IRQS[191-160] ----------------------------*/
#define INTERRUPT_B53_CTI_IRQ0            (SPI_TABLE_OFFSET + 172)
#define INTERRUPT_B53_CTI_IRQ1            (SPI_TABLE_OFFSET + 173)
#define INTERRUPT_B53_CTI_IRQ2            (SPI_TABLE_OFFSET + 174)
#define INTERRUPT_B53_CTI_IRQ3            (SPI_TABLE_OFFSET + 175)
#define INTERRUPT_PER_EXT_0               (SPI_TABLE_OFFSET + 176)
#define INTERRUPT_PER_EXT_1               (SPI_TABLE_OFFSET + 177)
#define INTERRUPT_PER_EXT_2               (SPI_TABLE_OFFSET + 178)
#define INTERRUPT_PER_EXT_3               (SPI_TABLE_OFFSET + 179)
#define INTERRUPT_PER_EXT_4               (SPI_TABLE_OFFSET + 180)
#define INTERRUPT_PER_EXT_5               (SPI_TABLE_OFFSET + 181)
#define INTERRUPT_PER_EXT_6               (SPI_TABLE_OFFSET + 182)
#define INTERRUPT_PER_EXT_7               (SPI_TABLE_OFFSET + 183)

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
#define INTERRUPT_ID_UART1                (bcm_legacy_irq_map[INTERRUPT_PER_UARTINT1 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_UART2                (bcm_legacy_irq_map[INTERRUPT_PER_UARTINT2 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_HS_UART              (bcm_legacy_irq_map[INTERRUPT_PER_HS_UARTINT - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_SATAC                (bcm_legacy_irq_map[INTERRUPT_SATA_CPU_INTR1 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_PCIE0                (bcm_legacy_irq_map[INTERRUPT_PCIE_0_CPU_INTR - SPI_TABLE_OFFSET]) 
#define INTERRUPT_ID_PCIE1                (bcm_legacy_irq_map[INTERRUPT_PCIE_1_CPU_INTR - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_PCIE2                (bcm_legacy_irq_map[INTERRUPT_PCIE_2_CPU_INTR - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_PCIE3                (bcm_legacy_irq_map[INTERRUPT_PCIE_3_CPU_INTR - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_RUNNER_0             (bcm_legacy_irq_map[INTERRUPT_RUNNER_INTR0 - SPI_TABLE_OFFSET]) 
#define INTERRUPT_ID_RUNNER_1             (bcm_legacy_irq_map[INTERRUPT_RUNNER_INTR1 - SPI_TABLE_OFFSET]) 
#define INTERRUPT_ID_RUNNER_2             (bcm_legacy_irq_map[INTERRUPT_RUNNER_INTR2 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_RUNNER_3             (bcm_legacy_irq_map[INTERRUPT_RUNNER_INTR3 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_RUNNER_4             (bcm_legacy_irq_map[INTERRUPT_RUNNER_INTR4 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_RUNNER_5             (bcm_legacy_irq_map[INTERRUPT_RUNNER_INTR5 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_FPM                  (bcm_legacy_irq_map[INTERRUPT_XRDP_FPM - SPI_TABLE_OFFSET]) 
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
#define INTERRUPT_ID_USB_XHCI             (bcm_legacy_irq_map[INTERRUPT_USB_XHCI - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_USB_OHCI             (bcm_legacy_irq_map[INTERRUPT_USB_OHCI - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_USB_EHCI             (bcm_legacy_irq_map[INTERRUPT_USB_EHCI - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_USB_OHCI1            (bcm_legacy_irq_map[INTERRUPT_USB_OHCI1 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_USB_EHCI1            (bcm_legacy_irq_map[INTERRUPT_USB_EHCI1 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_I2S                  (bcm_legacy_irq_map[INTERRUPT_PER_I2S - SPI_TABLE_OFFSET])
#define INTERRUPT_PCM_DMA_IRQ             (bcm_legacy_irq_map[INTERRUPT_PCM_DMA_IRQ0 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_NAND_FLASH           (bcm_legacy_irq_map[INTERRUPT_NAND_FLASH_IRQ - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_VDSL                 (bcm_legacy_irq_map[INTERRUPT_VDSL - SPI_TABLE_OFFSET])
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
#define INTERRUPT_ID_SYSTEMPORT_INTR0        (bcm_legacy_irq_map[INTERRUPT_SYSTEMPORT_INTR0 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_SYSTEMPORT_INTR1        (bcm_legacy_irq_map[INTERRUPT_SYSTEMPORT_INTR1 - SPI_TABLE_OFFSET])

#ifdef __BOARD_DRV_AARCH64__
// add here any legacy driver's (driver that have no device tree node) interrupt to be mapped
unsigned int bcm_phys_irqs_to_map[] =
{
    INTERRUPT_DYING_GASP_IRQ,
    INTERRUPT_PER_UARTINT0,
    INTERRUPT_PER_UARTINT1,
    INTERRUPT_PER_UARTINT2,
    INTERRUPT_PER_HS_UARTINT,
    INTERRUPT_PCIE_0_CPU_INTR,
    INTERRUPT_PCIE_1_CPU_INTR,
    INTERRUPT_PCIE_2_CPU_INTR,
    INTERRUPT_PCIE_3_CPU_INTR,
    INTERRUPT_XRDP_FPM,
    INTERRUPT_PER_TIMER_IRQ0,
    INTERRUPT_PER_TIMER_IRQ1,
    INTERRUPT_PER_TIMER_IRQ2,
    INTERRUPT_PER_TIMER_IRQ3,
    INTERRUPT_USB_XHCI,
    INTERRUPT_USB_OHCI,
    INTERRUPT_USB_EHCI,
    INTERRUPT_USB_OHCI1,
    INTERRUPT_USB_EHCI1,
    INTERRUPT_SATA_CPU_INTR1,
    INTERRUPT_PER_I2S,
    INTERRUPT_PCM_DMA_IRQ0,
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
    INTERRUPT_PER_EXT_0,
    INTERRUPT_PER_EXT_1,
    INTERRUPT_PER_EXT_2,
    INTERRUPT_PER_EXT_3,
    INTERRUPT_PER_EXT_4,
    INTERRUPT_PER_EXT_5,
    INTERRUPT_PER_EXT_6,
    INTERRUPT_PER_EXT_7,
    INTERRUPT_NAND_FLASH_IRQ,
    INTERRUPT_VDSL,
    INTERRUPT_SYSTEMPORT_INTR0,
    INTERRUPT_SYSTEMPORT_INTR1
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

#endif /* __63158_INTR_H */
