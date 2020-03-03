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

#ifndef __6858_INTR_H
#define __6858_INTR_H

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
#define INTERRUPT_B15_AXIERRIRQ           (SPI_TABLE_OFFSET + 0) /* 2 interrupts */
#define INTERRUPT_B15_INTERRIRQ           (SPI_TABLE_OFFSET + 2)
                    /* 2 reserved interrupts */
#define INTERRUPT_THERM_HIGH_IRQ          (SPI_TABLE_OFFSET + 6)
#define INTERRUPT_THERM_LOW_IRQ           (SPI_TABLE_OFFSET + 7)
#define INTERRUPT_THERM_SHUTDOWN          (SPI_TABLE_OFFSET + 8)
#define INTERRUPT_PMUIRQ                  (SPI_TABLE_OFFSET + 9)  /* 4 interrupts */
                    /* 2 reserved interrupts */
#define INTERRUPT_WDTIMER0                (SPI_TABLE_OFFSET + 14)
#define INTERRUPT_PER_TIMER_IRQ4_5        (SPI_TABLE_OFFSET + 15) /* 1 interrupts */
#define INTERRUPT_WDTIMER1                (SPI_TABLE_OFFSET + 16)
                    /* 1 reserved interrupts */
#define INTERRUPT_MEMC_SEC_IRQ            (SPI_TABLE_OFFSET + 18)
                    /* 1 reserved interrupts */
#define INTERRUPT_PER_SEC_ACC_VIOL_IRQ    (SPI_TABLE_OFFSET + 20)
                    /* 2 reserved interrupts */
#define INTERRUPT_MBOX_IRQ4_5             (SPI_TABLE_OFFSET + 23)  /* 2 interrupts */
#define INTERRUPT_LPORT_INTR_1_CPU_OUT    (SPI_TABLE_OFFSET + 26)
#define INTERRUPT_LPORT_INTR_0_CPU_OUT    (SPI_TABLE_OFFSET + 28)  
#define INTERRUPT_DYING_GASP_IRQ          (SPI_TABLE_OFFSET + 29)
#define INTERRUPT_PMC_MIPS_IRQ            (SPI_TABLE_OFFSET + 30)
#define INTERRUPT_PMC_MIPS1_IRQ           (SPI_TABLE_OFFSET + 31)
    /*  ------------- CHIP_IRQS[63-32] ----------------------------*/
#define INTERRUPT_PER_UARTINT0            (SPI_TABLE_OFFSET + 32)
#define INTERRUPT_PER_UARTINT1            (SPI_TABLE_OFFSET + 33)
                 /* 3 reserved interrupts  */
#define INTERRUPT_PER_HS_SPIM_IRQ         (SPI_TABLE_OFFSET + 37)
#define INTERRUPT_NAND_FLASH_IRQ          (SPI_TABLE_OFFSET + 38)
#define INTERRUPT_MEMC_IRQ                (SPI_TABLE_OFFSET + 39)
                 /* 4 reserved interrupts  */
#define INTERRUPT_SATA_CPU_INTR0          (SPI_TABLE_OFFSET + 44)
#define INTERRUPT_SATA_CPU_INTR1          (SPI_TABLE_OFFSET + 45)

#define INTERRUPT_PER_EXT_6               (SPI_TABLE_OFFSET + 48)
#define INTERRUPT_PER_EXT_7               (SPI_TABLE_OFFSET + 49)

#define INTERRUPT_REP_CAPTURE_MEMC12      (SPI_TABLE_OFFSET + 54) 
#define INTERRUPT_REQ_CAPTURE_MEMC12      (SPI_TABLE_OFFSET + 55) 
#define INTERRUPT_REP_CAPTURE_MEMC9       (SPI_TABLE_OFFSET + 56)  
#define INTERRUPT_REQ_CAPTURE_MEMC9       (SPI_TABLE_OFFSET + 57)  
#define INTERRUPT_REP_CAPTURE_PER         (SPI_TABLE_OFFSET + 58) 
#define INTERRUPT_REQ_CAPTURE_PER         (SPI_TABLE_OFFSET + 59) 
#define INTERRUPT_PCIE_0_CPU_INTR         (SPI_TABLE_OFFSET + 60) 
#define INTERRUPT_PCIE_1_CPU_INTR         (SPI_TABLE_OFFSET + 61)  
#define INTERRUPT_PCIE_2_CPU_INTR         (SPI_TABLE_OFFSET + 62)  
#define INTERRUPT_UBUS4_SYS_IRQ           (SPI_TABLE_OFFSET + 63)
    
    /*  ------------- CHIP_IRQS[95-64] ----------------------------*/
#define INTERRUPT_PER_TIMER_IRQ0          (SPI_TABLE_OFFSET + 64)  /* 4 interrupts */
#define INTERRUPT_PER_TIMER_IRQ1          (SPI_TABLE_OFFSET + 65)
#define INTERRUPT_PER_TIMER_IRQ2          (SPI_TABLE_OFFSET + 66)
#define INTERRUPT_PER_TIMER_IRQ3          (SPI_TABLE_OFFSET + 67)
#define INTERRUPT_MBOX_IRQ0_3             (SPI_TABLE_OFFSET + 68)  /* 4 interrupts */
#define INTERRUPT_PERIPH_INTERNAL         (SPI_TABLE_OFFSET + 72)  /* 4 interrupts */
                /* 1 reserved interrupts  */
#define INTERRUPT_PER_I2C_IRQ             (SPI_TABLE_OFFSET + 83)
#define INTERRUPT_PER_I2S_IRQ             (SPI_TABLE_OFFSET + 84)
#define INTERRUPT_O_RNG_INTR              (SPI_TABLE_OFFSET + 85)
#define INTERRUPT_WAN_XGRX                (SPI_TABLE_OFFSET + 86)
#define INTERRUPT_WAN_XGTX_INTR1          (SPI_TABLE_OFFSET + 87)
#define INTERRUPT_WAN_XGTX_INTR0          (SPI_TABLE_OFFSET + 88)
#define INTERRUPT_M2M_CH_INT              (SPI_TABLE_OFFSET + 89)  /* 4 interrupts */
                /* 3 reserved interrupts  */
#define INTERRUPT_PER_HS_UARTINT          (SPI_TABLE_OFFSET + 93)  
#define INTERRUPT_PL081_DMA_INTR          (SPI_TABLE_OFFSET + 94)
#define INTERRUPT_SDIO_EMMC_L1_INTR       (SPI_TABLE_OFFSET + 95)
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
#define INTERRUPT_PER_EXT_0               (SPI_TABLE_OFFSET + 105)  
#define INTERRUPT_PER_EXT_1               (SPI_TABLE_OFFSET + 106)  
#define INTERRUPT_PER_EXT_2               (SPI_TABLE_OFFSET + 107)  
#define INTERRUPT_PER_EXT_3               (SPI_TABLE_OFFSET + 108)  
#define INTERRUPT_PER_EXT_4               (SPI_TABLE_OFFSET + 109)  
#define INTERRUPT_PER_EXT_5               (SPI_TABLE_OFFSET + 110)
#define INTERRUPT_PER_SIM                 (SPI_TABLE_OFFSET + 111) /* 3 interrupts */
#define INTERRUPT_PCM_DMA_IRQ0            (SPI_TABLE_OFFSET + 114) /* PCM DMA RX interrupt */
#define INTERRUPT_PCM_DMA_IRQ1            (SPI_TABLE_OFFSET + 115) /* PCM DMA TX interrupt */
#define INTERRUPT_PCM_IRQ                 (SPI_TABLE_OFFSET + 116) /* 2 interrupts */
            /* 2 reserved interrupts */
#define INTERRUPT_USB_USBD            (SPI_TABLE_OFFSET + 120)
#define INTERRUPT_USB_XHCI            (SPI_TABLE_OFFSET + 121)
#define INTERRUPT_USB_OHCI1           (SPI_TABLE_OFFSET + 122)
#define INTERRUPT_USB_EHCI1           (SPI_TABLE_OFFSET + 123)
#define INTERRUPT_USB_OHCI            (SPI_TABLE_OFFSET + 124)
#define INTERRUPT_USB_EHCI            (SPI_TABLE_OFFSET + 125)
#define INTERRUPT_USB_EVENTS          (SPI_TABLE_OFFSET + 126)
#define INTERRUPT_USB_BRIDGE          (SPI_TABLE_OFFSET + 127)
        /* XRDP Interrupts */
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
#define INTERRUPT_XRDP_QUEUE_10            (SPI_TABLE_OFFSET + 138)
#define INTERRUPT_XRDP_QUEUE_11            (SPI_TABLE_OFFSET + 139)
#define INTERRUPT_XRDP_QUEUE_12            (SPI_TABLE_OFFSET + 140)
#define INTERRUPT_XRDP_QUEUE_13            (SPI_TABLE_OFFSET + 141)
#define INTERRUPT_XRDP_QUEUE_14            (SPI_TABLE_OFFSET + 142)
#define INTERRUPT_XRDP_QUEUE_15            (SPI_TABLE_OFFSET + 143)
#define INTERRUPT_XRDP_QUEUE_16            (SPI_TABLE_OFFSET + 144)
#define INTERRUPT_XRDP_QUEUE_17            (SPI_TABLE_OFFSET + 145)
#define INTERRUPT_XRDP_QUEUE_18            (SPI_TABLE_OFFSET + 146)
#define INTERRUPT_XRDP_QUEUE_19            (SPI_TABLE_OFFSET + 147)
#define INTERRUPT_XRDP_QUEUE_20            (SPI_TABLE_OFFSET + 148)
#define INTERRUPT_XRDP_QUEUE_21            (SPI_TABLE_OFFSET + 149)
#define INTERRUPT_XRDP_QUEUE_22            (SPI_TABLE_OFFSET + 150)
#define INTERRUPT_XRDP_QUEUE_23            (SPI_TABLE_OFFSET + 151)
#define INTERRUPT_XRDP_QUEUE_24            (SPI_TABLE_OFFSET + 152)
#define INTERRUPT_XRDP_QUEUE_25            (SPI_TABLE_OFFSET + 153)
#define INTERRUPT_XRDP_QUEUE_26            (SPI_TABLE_OFFSET + 154)
#define INTERRUPT_XRDP_QUEUE_27            (SPI_TABLE_OFFSET + 155)
#define INTERRUPT_XRDP_QUEUE_28            (SPI_TABLE_OFFSET + 156)
#define INTERRUPT_XRDP_QUEUE_29            (SPI_TABLE_OFFSET + 157)
#define INTERRUPT_XRDP_QUEUE_30            (SPI_TABLE_OFFSET + 158)
#define INTERRUPT_XRDP_QUEUE_31            (SPI_TABLE_OFFSET + 159)
#define INTERRUPT_XRDP_SYSPORT             (SPI_TABLE_OFFSET + 160)
#define INTERRUPT_XRDP_FPM                 (SPI_TABLE_OFFSET + 161)
#define INTERRUPT_XRDP_HASH                (SPI_TABLE_OFFSET + 162)
#define INTERRUPT_XRDP_QM                  (SPI_TABLE_OFFSET + 163)
#define INTERRUPT_XRDP_DSPTCHR             (SPI_TABLE_OFFSET + 164)
#define INTERRUPT_XRDP_SBPM                (SPI_TABLE_OFFSET + 165)
#define INTERRUPT_XRDP_REQ_CAPTURE1        (SPI_TABLE_OFFSET + 166)
#define INTERRUPT_XRDP_REQ_CAPTURE2        (SPI_TABLE_OFFSET + 166)

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
#define INTERRUPT_ID_HS_UART              (bcm_legacy_irq_map[INTERRUPT_PER_HS_UARTINT - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_SATAC                (bcm_legacy_irq_map[INTERRUPT_SATA_CPU_INTR1 - SPI_TABLE_OFFSET])
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
#define INTERRUPT_ID_WDTIMER              (bcm_legacy_irq_map[INTERRUPT_WDTIMER0 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_WDTIMER1             (bcm_legacy_irq_map[INTERRUPT_WDTIMER1 - SPI_TABLE_OFFSET])

#define INTERRUPT_ID_XRDP_QUEUE_0            (bcm_legacy_irq_map[INTERRUPT_XRDP_QUEUE_0 - SPI_TABLE_OFFSET])
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
#define INTERRUPT_ID_XRDP_SYSPORT            (bcm_legacy_irq_map[INTERRUPT_XRDP_SYSPORT - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_XRDP_FPM                (bcm_legacy_irq_map[INTERRUPT_XRDP_FPM - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_XRDP_HASH               (bcm_legacy_irq_map[INTERRUPT_XRDP_HASH - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_XRDP_QM                 (bcm_legacy_irq_map[INTERRUPT_XRDP_QM - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_XRDP_DSPTCHR            (bcm_legacy_irq_map[INTERRUPT_XRDP_DSPTCHR - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_XRDP_SBPM               (bcm_legacy_irq_map[INTERRUPT_XRDP_SBPM - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_XRDP_REQ_CAPTURE1       (bcm_legacy_irq_map[INTERRUPT_XRDP_REQ_CAPTURE1 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_XRDP_REQ_CAPTURE2       (bcm_legacy_irq_map[INTERRUPT_XRDP_REQ_CAPTURE2 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_LPORT_INTR_1_CPU_OUT    (bcm_legacy_irq_map[INTERRUPT_LPORT_INTR_1_CPU_OUT - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_LPORT_INTR_0_CPU_OUT    (bcm_legacy_irq_map[INTERRUPT_LPORT_INTR_0_CPU_OUT - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_WAN_EPON_TOP            (bcm_legacy_irq_map[INTERRUPT_WAN_EPON_TOP - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_WAN_NCO_GPON            (bcm_legacy_irq_map[INTERRUPT_WAN_NCO_GPON - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_WAN_GPON_TX             (bcm_legacy_irq_map[INTERRUPT_WAN_GPON_TX - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_WAN_GPON_RX             (bcm_legacy_irq_map[INTERRUPT_WAN_GPON_RX - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_WAN_PMD_PLL1_LOCK_INT   (bcm_legacy_irq_map[INTERRUPT_WAN_PMD_PLL1_LOCK_INT - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_WAN_PMD_PLL0_LOCK_INT   (bcm_legacy_irq_map[INTERRUPT_WAN_PMD_PLL0_LOCK_INT - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_WAN_PMD_SIGNAL_DETECT_0 (bcm_legacy_irq_map[INTERRUPT_WAN_PMD_SIGNAL_DETECT_0 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_WAN_PMD_ENERGY_DETECT_0 (bcm_legacy_irq_map[INTERRUPT_WAN_PMD_ENERGY_DETECT_0 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_WAN_PMD_RX_LOCK_0       (bcm_legacy_irq_map[INTERRUPT_WAN_PMD_RX_LOCK_0 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_WAN_XGRX                (bcm_legacy_irq_map[INTERRUPT_WAN_XGRX - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_WAN_XGTX_INTR1          (bcm_legacy_irq_map[INTERRUPT_WAN_XGTX_INTR1 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_WAN_XGTX_INTR0          (bcm_legacy_irq_map[INTERRUPT_WAN_XGTX_INTR0 - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_I2S                     (bcm_legacy_irq_map[INTERRUPT_PER_I2S_IRQ - SPI_TABLE_OFFSET])
#define INTERRUPT_ID_NAND_FLASH              (bcm_legacy_irq_map[INTERRUPT_NAND_FLASH_IRQ - SPI_TABLE_OFFSET])
#define INTERRUPT_PCM_DMA_IRQ                (bcm_legacy_irq_map[INTERRUPT_PCM_DMA_IRQ0 - SPI_TABLE_OFFSET])

#ifdef __BOARD_DRV_AARCH64__
// add here any legacy driver's (driver that have no device tree node) interrupt to be mapped
unsigned int bcm_phys_irqs_to_map[] =
{
    INTERRUPT_MEMC_SEC_IRQ,
    INTERRUPT_DYING_GASP_IRQ,
    INTERRUPT_PER_UARTINT0,
    INTERRUPT_PER_UARTINT1,
    INTERRUPT_PER_HS_UARTINT,
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
    INTERRUPT_WAN_XGRX,
    INTERRUPT_WAN_XGTX_INTR1,
    INTERRUPT_WAN_XGTX_INTR0,
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
    INTERRUPT_XRDP_SYSPORT,
    INTERRUPT_XRDP_FPM,
    INTERRUPT_XRDP_HASH,
    INTERRUPT_XRDP_QM,
    INTERRUPT_XRDP_DSPTCHR,
    INTERRUPT_XRDP_SBPM,
    INTERRUPT_XRDP_REQ_CAPTURE1,
    INTERRUPT_XRDP_REQ_CAPTURE2,
    INTERRUPT_LPORT_INTR_1_CPU_OUT,
    INTERRUPT_LPORT_INTR_0_CPU_OUT,
    INTERRUPT_WAN_EPON_TOP,
    INTERRUPT_WAN_NCO_GPON,
    INTERRUPT_WAN_GPON_TX,
    INTERRUPT_WAN_GPON_RX,
    INTERRUPT_WAN_PMD_PLL1_LOCK_INT,
    INTERRUPT_WAN_PMD_PLL0_LOCK_INT,
    INTERRUPT_WAN_PMD_SIGNAL_DETECT_0,
    INTERRUPT_WAN_PMD_ENERGY_DETECT_0,
    INTERRUPT_WAN_PMD_RX_LOCK_0,
    INTERRUPT_NAND_FLASH_IRQ,
    INTERRUPT_PCM_DMA_IRQ0,
    INTERRUPT_USB_XHCI,
    INTERRUPT_USB_OHCI,
    INTERRUPT_USB_EHCI,
    INTERRUPT_USB_OHCI1,
    INTERRUPT_USB_EHCI1,
    INTERRUPT_SATA_CPU_INTR1,
    INTERRUPT_PER_I2S_IRQ
};
unsigned int bcm_legacy_irq_map[256];
#else
extern unsigned int bcm_phys_irqs_to_map[];
extern unsigned int bcm_legacy_irq_map[];
#endif

#endif

#define NUM_EXT_INT    8

#ifdef __cplusplus
    }
#endif                    

#endif  /* __BCM6858_H */

