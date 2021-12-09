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

#ifndef __6848_INTR_H
#define __6848_INTR_H

#ifdef __cplusplus
    extern "C" {
#endif

#define INTERRUPT_ID_SOFTWARE_0          0
#define INTERRUPT_ID_SOFTWARE_1          1

/*=====================================================================*/
/* BCM6848 Timer Interrupt Level Assignments                          */
/*=====================================================================*/
#define MIPS_TIMER_INT                   7

/*=====================================================================*/
/* Peripheral ISR Table Offset                                              */
/*=====================================================================*/
#define INTERNAL_ISR_TABLE_OFFSET             8
#define INTERNAL_HIGH_ISR_TABLE_OFFSET        (INTERNAL_ISR_TABLE_OFFSET + 32)
#define INTERNAL_EXT_ISR_TABLE_OFFSET         (INTERNAL_HIGH_ISR_TABLE_OFFSET + 32)
#define INTERNAL_HIGH_EXT_ISR_TABLE_OFFSET    (INTERNAL_EXT_ISR_TABLE_OFFSET + 32)

/*=====================================================================*/
/* Logical Peripheral Interrupt IDs                                    */
/*=====================================================================*/
#define INTERRUPT_ID_USBD_SYS            (INTERNAL_ISR_TABLE_OFFSET + 0)
#define INTERRUPT_ID_USB_BRIDGE          (INTERNAL_ISR_TABLE_OFFSET + 1)
#define INTERRUPT_ID_USBD_DMA_CH         (INTERNAL_ISR_TABLE_OFFSET + 2) /* 6 interrupts */
#define INTERRUPT_ID_USBH_EHCI           (INTERNAL_ISR_TABLE_OFFSET + 8)
#define INTERRUPT_ID_USBH_EVT            (INTERNAL_ISR_TABLE_OFFSET + 9)
#define INTERRUPT_ID_USBH_OHCI           (INTERNAL_ISR_TABLE_OFFSET + 10)
#define INTERRUPT_ID_USBH                INTERRUPT_ID_USBH_OHCI /*for compatibility*/
#define INTERRUPT_ID_USBH20              INTERRUPT_ID_USBH_EHCI /*for compatibility*/
#define INTERRUPT_ID_PCIE_RC             (INTERNAL_ISR_TABLE_OFFSET + 12)
#define INTERRUPT_ID_PCM_DMA_0           (INTERNAL_ISR_TABLE_OFFSET + 14)
#define INTERRUPT_ID_PCM_DMA_1           (INTERNAL_ISR_TABLE_OFFSET + 15)
#define INTERRUPT_ID_PCM                 (INTERNAL_ISR_TABLE_OFFSET + 16)
#define INTERRUPT_ID_MEMC                (INTERNAL_ISR_TABLE_OFFSET + 18)
#define INTERRUPT_ID_MEMC_SEC            (INTERNAL_ISR_TABLE_OFFSET + 19)
#define INTERRUPT_ID_EXTERNAL_0          (INTERNAL_ISR_TABLE_OFFSET + 24)
#define INTERRUPT_ID_EXTERNAL_1          (INTERNAL_ISR_TABLE_OFFSET + 25)
#define INTERRUPT_ID_EXTERNAL_2          (INTERNAL_ISR_TABLE_OFFSET + 26)
#define INTERRUPT_ID_EXTERNAL_3          (INTERNAL_ISR_TABLE_OFFSET + 27)
#define INTERRUPT_ID_EXTERNAL_4          (INTERNAL_ISR_TABLE_OFFSET + 28)
#define INTERRUPT_ID_EXTERNAL_5          (INTERNAL_ISR_TABLE_OFFSET + 29)
#define INTERRUPT_ID_EXTERNAL_6          (INTERNAL_ISR_TABLE_OFFSET + 30)
#define INTERRUPT_ID_EXTERNAL_7          (INTERNAL_ISR_TABLE_OFFSET + 31)
#define INTERRUPT_ID_GPHY_OENB_0         (INTERNAL_HIGH_ISR_TABLE_OFFSET + 0)
#define INTERRUPT_ID_GPHY_OENB_1         (INTERNAL_HIGH_ISR_TABLE_OFFSET + 1)
#define INTERRUPT_ID_EPHY_ENERGY_DET_0   (INTERNAL_HIGH_ISR_TABLE_OFFSET + 4)
#define INTERRUPT_ID_EPHY_ENERGY_DET_1   (INTERNAL_HIGH_ISR_TABLE_OFFSET + 5)
#define INTERRUPT_ID_EPHY_IDDQ_ENERGY_0  (INTERNAL_HIGH_ISR_TABLE_OFFSET + 8)
#define INTERRUPT_ID_EPHY_IDDQ_ENERGY_1  (INTERNAL_HIGH_ISR_TABLE_OFFSET + 9)
#define INTERRUPT_ID_EPHY                (INTERNAL_HIGH_ISR_TABLE_OFFSET + 12)
#define INTERRUPT_ID_SGMII_0             (INTERNAL_HIGH_ISR_TABLE_OFFSET + 14)
#define INTERRUPT_ID_SGMII_1             (INTERNAL_HIGH_ISR_TABLE_OFFSET + 15)
#define INTERRUPT_ID_RDP_BPM             (INTERNAL_HIGH_ISR_TABLE_OFFSET + 17)
#define INTERRUPT_ID_RDP_SBPM            (INTERNAL_HIGH_ISR_TABLE_OFFSET + 18)
#define INTERRUPT_ID_RDP_MS1588          (INTERNAL_HIGH_ISR_TABLE_OFFSET + 19)
#define INTERRUPT_ID_RDP_RUNNER          (INTERNAL_HIGH_ISR_TABLE_OFFSET + 20)  /* 10 interrupts */
#define INTERRUPT_ID_PMC_0               (INTERNAL_EXT_ISR_TABLE_OFFSET + 0)
#define INTERRUPT_ID_PMC_1               (INTERNAL_EXT_ISR_TABLE_OFFSET + 1)
#define INTERRUPT_ID_UBUS_CORE_ERR       (INTERNAL_EXT_ISR_TABLE_OFFSET + 4)
#define INTERRUPT_ID_UBUS_PIPE_ERR       (INTERNAL_EXT_ISR_TABLE_OFFSET + 5)
#define INTERRUPT_ID_DG                  (INTERNAL_EXT_ISR_TABLE_OFFSET + 31)
#define INTERRUPT_ID_UART                (INTERNAL_HIGH_EXT_ISR_TABLE_OFFSET + 0)
#define INTERRUPT_ID_UART1               (INTERNAL_HIGH_EXT_ISR_TABLE_OFFSET + 1)
#define INTERRUPT_ID_HS_SPIM             (INTERNAL_HIGH_EXT_ISR_TABLE_OFFSET + 6)
#define INTERRUPT_ID_I2C                 (INTERNAL_HIGH_EXT_ISR_TABLE_OFFSET + 8)
#define INTERRUPT_ID_USIM_ESD            (INTERNAL_HIGH_EXT_ISR_TABLE_OFFSET + 9)
#define INTERRUPT_ID_USIM                (INTERNAL_HIGH_EXT_ISR_TABLE_OFFSET + 10)
#define INTERRUPT_ID_USIM_PRES           (INTERNAL_HIGH_EXT_ISR_TABLE_OFFSET + 11)
#define INTERRUPT_ID_NAND_FLASH          (INTERNAL_HIGH_EXT_ISR_TABLE_OFFSET + 12)
#define INTERRUPT_ID_TIMER0             (INTERNAL_HIGH_EXT_ISR_TABLE_OFFSET + 13)
#define INTERRUPT_ID_TIMER1             (INTERNAL_HIGH_EXT_ISR_TABLE_OFFSET + 14)
#define INTERRUPT_ID_TIMER2             (INTERNAL_HIGH_EXT_ISR_TABLE_OFFSET + 15)
#define INTERRUPT_ID_TIMER3             (INTERNAL_HIGH_EXT_ISR_TABLE_OFFSET + 16)
#define INTERRUPT_ID_WDTIMER             (INTERNAL_HIGH_EXT_ISR_TABLE_OFFSET + 17)
#define INTERRUPT_ID_PER_MBOX0           (INTERNAL_HIGH_EXT_ISR_TABLE_OFFSET + 19)
#define INTERRUPT_ID_PER_MBOX1           (INTERNAL_HIGH_EXT_ISR_TABLE_OFFSET + 20)
#define INTERRUPT_ID_PER_MBOX2           (INTERNAL_HIGH_EXT_ISR_TABLE_OFFSET + 21)
#define INTERRUPT_ID_PER_MBOX3           (INTERNAL_HIGH_EXT_ISR_TABLE_OFFSET + 22)
#define INTERRUPT_ID_WAN_EPON            (INTERNAL_HIGH_EXT_ISR_TABLE_OFFSET + 24)
#define INTERRUPT_ID_WAN_GPON_RX         (INTERNAL_HIGH_EXT_ISR_TABLE_OFFSET + 25)
#define INTERRUPT_ID_WAN_GPON_TX         (INTERNAL_HIGH_EXT_ISR_TABLE_OFFSET + 26)
#define INTERRUPT_ID_WAN_NCO_GPON        (INTERNAL_HIGH_EXT_ISR_TABLE_OFFSET + 27)
#define INTERRUPT_ID_ONU2G_PLL_LOCK      (INTERNAL_HIGH_EXT_ISR_TABLE_OFFSET + 28)
#define INTERRUPT_ID_ONU2G_SIG_DETECT    (INTERNAL_HIGH_EXT_ISR_TABLE_OFFSET + 29)
#define INTERRUPT_ID_ONU2G_NRG_DETECT    (INTERNAL_HIGH_EXT_ISR_TABLE_OFFSET + 30)
#define INTERRUPT_ID_ONU2G_RX_LOCK       (INTERNAL_HIGH_EXT_ISR_TABLE_OFFSET + 31)

#define INTERRUPT_ID_LAST                INTERRUPT_ID_ONU2G_RX_LOCK

#define INTERRUPT_ID_TIMER             INTERRUPT_ID_LAST+1 // not used

#define NUM_EXT_INT    (INTERRUPT_ID_EXTERNAL_7-INTERRUPT_ID_EXTERNAL_0+1)

#ifdef __cplusplus
    }
#endif                    

#endif  /* __BCM6848_H */

