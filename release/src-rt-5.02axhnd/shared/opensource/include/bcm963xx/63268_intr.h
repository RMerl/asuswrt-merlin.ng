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

#ifndef __63268_INTR_H
#define __63268_INTR_H

#ifdef __cplusplus
    extern "C" {
#endif

#define INTERRUPT_ID_SOFTWARE_0           0
#define INTERRUPT_ID_SOFTWARE_1           1

/*=====================================================================*/
/* BCM63268 Timer Interrupt Level Assignments                          */
/*=====================================================================*/
#define MIPS_TIMER_INT                  7

/*=====================================================================*/
/* Peripheral ISR Table Offset                                              */
/*=====================================================================*/
#define INTERNAL_ISR_TABLE_OFFSET       8
#define INTERNAL_HIGH_ISR_TABLE_OFFSET  (INTERNAL_ISR_TABLE_OFFSET + 32)
#define INTERNAL_EXT_ISR_TABLE_OFFSET   (INTERNAL_HIGH_ISR_TABLE_OFFSET + 32)

/*=====================================================================*/
/* Logical Peripheral Interrupt IDs                                    */
/*=====================================================================*/
#define INTERRUPT_ID_TIMER               (INTERNAL_ISR_TABLE_OFFSET + 0)
#define INTERRUPT_ID_ENETSW_RX_DMA_0     (INTERNAL_ISR_TABLE_OFFSET + 1)
#define INTERRUPT_ID_ENETSW_RX_DMA_1     (INTERNAL_ISR_TABLE_OFFSET + 2)
#define INTERRUPT_ID_ENETSW_RX_DMA_2     (INTERNAL_ISR_TABLE_OFFSET + 3)
#define INTERRUPT_ID_ENETSW_RX_DMA_3     (INTERNAL_ISR_TABLE_OFFSET + 4)
#define INTERRUPT_ID_UART                (INTERNAL_ISR_TABLE_OFFSET + 5)
#define INTERRUPT_ID_HS_SPIM             (INTERNAL_ISR_TABLE_OFFSET + 6)
#define INTERRUPT_ID_WLAN                (INTERNAL_ISR_TABLE_OFFSET + 7)
#define INTERRUPT_ID_IPSEC               (INTERNAL_ISR_TABLE_OFFSET + 8)
#define INTERRUPT_ID_USBH                (INTERNAL_ISR_TABLE_OFFSET + 9)
#define INTERRUPT_ID_USBH20              (INTERNAL_ISR_TABLE_OFFSET + 10)
#define INTERRUPT_ID_USBS                (INTERNAL_ISR_TABLE_OFFSET + 11)
#define INTERRUPT_ID_PCM                 (INTERNAL_ISR_TABLE_OFFSET + 12)
#define INTERRUPT_ID_EPHY                (INTERNAL_ISR_TABLE_OFFSET + 13)
#define INTERRUPT_ID_DG                  (INTERNAL_ISR_TABLE_OFFSET + 14)
#define INTERRUPT_ID_EPHY_ENERGY_0       (INTERNAL_ISR_TABLE_OFFSET + 15)
#define INTERRUPT_ID_EPHY_ENERGY_1       (INTERNAL_ISR_TABLE_OFFSET + 16)
#define INTERRUPT_ID_EPHY_ENERGY_2       (INTERNAL_ISR_TABLE_OFFSET + 17)
#define INTERRUPT_ID_GPHY_ENERGY_0       (INTERNAL_ISR_TABLE_OFFSET + 18)
#define INTERRUPT_ID_USB_CNTL_RX_DMA     (INTERNAL_ISR_TABLE_OFFSET + 19)
#define INTERRUPT_ID_USB_BULK_RX_DMA     (INTERNAL_ISR_TABLE_OFFSET + 20)
#define INTERRUPT_ID_USB_ISO_RX_DMA      (INTERNAL_ISR_TABLE_OFFSET + 21)
#define INTERRUPT_ID_IPSEC_DMA_0         (INTERNAL_ISR_TABLE_OFFSET + 22)
#define INTERRUPT_ID_XDSL                (INTERNAL_ISR_TABLE_OFFSET + 23)
#define INTERRUPT_ID_FAP_0               (INTERNAL_ISR_TABLE_OFFSET + 24)
#define INTERRUPT_ID_FAP_1               (INTERNAL_ISR_TABLE_OFFSET + 25)
#define INTERRUPT_ID_ATM_DMA_0           (INTERNAL_ISR_TABLE_OFFSET + 26)
#define INTERRUPT_ID_ATM_DMA_1           (INTERNAL_ISR_TABLE_OFFSET + 27)
#define INTERRUPT_ID_ATM_DMA_2           (INTERNAL_ISR_TABLE_OFFSET + 28)
#define INTERRUPT_ID_ATM_DMA_3           (INTERNAL_ISR_TABLE_OFFSET + 29)
#define INTERRUPT_ID_WAKE_ON_IRQ         (INTERNAL_ISR_TABLE_OFFSET + 30)
#define INTERRUPT_ID_GPHY                (INTERNAL_ISR_TABLE_OFFSET + 31)
#define INTERRUPT_ID_DECT_0              (INTERNAL_HIGH_ISR_TABLE_OFFSET + 0)
#define INTERRUPT_ID_DECT_1              (INTERNAL_HIGH_ISR_TABLE_OFFSET + 1)
#define INTERRUPT_ID_UART1               (INTERNAL_HIGH_ISR_TABLE_OFFSET + 2)
#define INTERRUPT_ID_WLAN_GPIO           (INTERNAL_HIGH_ISR_TABLE_OFFSET + 3)
#define INTERRUPT_ID_USB_CNTL_TX_DMA     (INTERNAL_HIGH_ISR_TABLE_OFFSET + 4)
#define INTERRUPT_ID_USB_BULK_TX_DMA     (INTERNAL_HIGH_ISR_TABLE_OFFSET + 5)
#define INTERRUPT_ID_USB_ISO_TX_DMA      (INTERNAL_HIGH_ISR_TABLE_OFFSET + 6)
#define INTERRUPT_ID_IPSEC_DMA_1         (INTERNAL_HIGH_ISR_TABLE_OFFSET + 7)
#define INTERRUPT_ID_PCIE_RC             (INTERNAL_HIGH_ISR_TABLE_OFFSET + 8)
#define INTERRUPT_ID_PCIE_EP             (INTERNAL_HIGH_ISR_TABLE_OFFSET + 9)
#define INTERRUPT_ID_PCM_DMA_0           (INTERNAL_HIGH_ISR_TABLE_OFFSET + 10)
#define INTERRUPT_ID_PCM_DMA_1           (INTERNAL_HIGH_ISR_TABLE_OFFSET + 11)
#define INTERRUPT_ID_EXTERNAL_0          (INTERNAL_HIGH_ISR_TABLE_OFFSET + 12)
#define INTERRUPT_ID_EXTERNAL_1          (INTERNAL_HIGH_ISR_TABLE_OFFSET + 13)
#define INTERRUPT_ID_EXTERNAL_2          (INTERNAL_HIGH_ISR_TABLE_OFFSET + 14)
#define INTERRUPT_ID_EXTERNAL_3          (INTERNAL_HIGH_ISR_TABLE_OFFSET + 15)
#define INTERRUPT_ID_ENETSW_SYS          (INTERNAL_HIGH_ISR_TABLE_OFFSET + 16)
#define INTERRUPT_ID_SAR                 (INTERNAL_HIGH_ISR_TABLE_OFFSET + 17)
#define INTERRUPT_ID_NAND_FLASH          (INTERNAL_HIGH_ISR_TABLE_OFFSET + 18)
#define INTERRUPT_ID_RING_OSC            (INTERNAL_HIGH_ISR_TABLE_OFFSET + 20)
#define INTERRUPT_ID_USB_CONNECT         (INTERNAL_HIGH_ISR_TABLE_OFFSET + 21)
#define INTERRUPT_ID_USB_DISCONNECT      (INTERNAL_HIGH_ISR_TABLE_OFFSET + 22)
#define INTERRUPT_ID_PER_MBOX0           (INTERNAL_HIGH_ISR_TABLE_OFFSET + 23)
#define INTERRUPT_ID_PER_MBOX1           (INTERNAL_HIGH_ISR_TABLE_OFFSET + 24)
#define INTERRUPT_ID_PER_MBOX2           (INTERNAL_HIGH_ISR_TABLE_OFFSET + 25)
#define INTERRUPT_ID_PER_MBOX3           (INTERNAL_HIGH_ISR_TABLE_OFFSET + 26)
#define INTERRUPT_ID_ATM_DMA_4           (INTERNAL_HIGH_ISR_TABLE_OFFSET + 27)
#define INTERRUPT_ID_ATM_DMA_5           (INTERNAL_HIGH_ISR_TABLE_OFFSET + 28)
#define INTERRUPT_ID_ATM_DMA_6           (INTERNAL_HIGH_ISR_TABLE_OFFSET + 29)
#define INTERRUPT_ID_ATM_DMA_7           (INTERNAL_HIGH_ISR_TABLE_OFFSET + 30)

#define INTERRUPT_ID_ENETSW_TX_DMA_0     (INTERNAL_EXT_ISR_TABLE_OFFSET + 0)
#define INTERRUPT_ID_ENETSW_TX_DMA_1     (INTERNAL_EXT_ISR_TABLE_OFFSET + 1)
#define INTERRUPT_ID_ENETSW_TX_DMA_2     (INTERNAL_EXT_ISR_TABLE_OFFSET + 2)
#define INTERRUPT_ID_ENETSW_TX_DMA_3     (INTERNAL_EXT_ISR_TABLE_OFFSET + 3)
#define INTERRUPT_ID_ATM_DMA_8           (INTERNAL_EXT_ISR_TABLE_OFFSET + 4)
#define INTERRUPT_ID_ATM_DMA_9           (INTERNAL_EXT_ISR_TABLE_OFFSET + 5)
#define INTERRUPT_ID_ATM_DMA_10          (INTERNAL_EXT_ISR_TABLE_OFFSET + 6)
#define INTERRUPT_ID_ATM_DMA_11          (INTERNAL_EXT_ISR_TABLE_OFFSET + 7)
#define INTERRUPT_ID_ATM_DMA_12          (INTERNAL_EXT_ISR_TABLE_OFFSET + 8)
#define INTERRUPT_ID_ATM_DMA_13          (INTERNAL_EXT_ISR_TABLE_OFFSET + 9)
#define INTERRUPT_ID_ATM_DMA_14          (INTERNAL_EXT_ISR_TABLE_OFFSET + 10)
#define INTERRUPT_ID_ATM_DMA_15          (INTERNAL_EXT_ISR_TABLE_OFFSET + 11)
#define INTERRUPT_ID_ATM_DMA_16          (INTERNAL_EXT_ISR_TABLE_OFFSET + 12)
#define INTERRUPT_ID_ATM_DMA_17          (INTERNAL_EXT_ISR_TABLE_OFFSET + 13)
#define INTERRUPT_ID_ATM_DMA_18          (INTERNAL_EXT_ISR_TABLE_OFFSET + 14)
#define INTERRUPT_ID_ATM_DMA_19          (INTERNAL_EXT_ISR_TABLE_OFFSET + 15)
#define INTERRUPT_ID_LS_SPIM             (INTERNAL_EXT_ISR_TABLE_OFFSET + 16)
#define INTERRUPT_ID_GMAC_DMA_0          (INTERNAL_EXT_ISR_TABLE_OFFSET + 17)
#define INTERRUPT_ID_GMAC_DMA_1          (INTERNAL_EXT_ISR_TABLE_OFFSET + 18)
#define INTERRUPT_ID_GMAC                (INTERNAL_EXT_ISR_TABLE_OFFSET + 19)

#define INTERRUPT_ID_LAST                INTERRUPT_ID_GMAC

#define NUM_EXT_INT    (INTERRUPT_ID_EXTERNAL_3-INTERRUPT_ID_EXTERNAL_0+1)

#ifdef __cplusplus
    }
#endif                    

#endif  /* __BCM63268_H */


