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


#ifndef __6838_INTR_H
#define __6838_INTR_H

#ifdef __cplusplus
    extern "C" {
#endif

#define INTERRUPT_ID_SOFTWARE_0           0
#define INTERRUPT_ID_SOFTWARE_1           1

/*=====================================================================*/
/* BCM6828 Timer Interrupt Level Assignments                          */
/*=====================================================================*/
#define MIPS_TIMER_INT                  7

/*=====================================================================*/
/* Peripheral ISR Table Offset                                              */
/*=====================================================================*/
#define INTERNAL_ISR_TABLE_OFFSET   	8
#define INTERNAL_EXT_ISR_TABLE_OFFSET   (INTERNAL_ISR_TABLE_OFFSET + 64)
#define EXTERNAL_ISR_TABLE_OFFSET   	(INTERNAL_EXT_ISR_TABLE_OFFSET + 64)

/*=====================================================================*/
/* Logical Peripheral Interrupt IDs                                    */
/*=====================================================================*/
#define INTERRUPT_ID_TIMER              (INTERNAL_ISR_TABLE_OFFSET + 0)
#define INTERRUPT_ID_UART               (INTERNAL_ISR_TABLE_OFFSET + 1)
#define INTERRUPT_ID_UART1              (INTERNAL_ISR_TABLE_OFFSET + 2)
#define INTERRUPT_ID_NAND_FLASH         (INTERNAL_ISR_TABLE_OFFSET + 3)
#define INTERRUPT_ID_I2C                (INTERNAL_ISR_TABLE_OFFSET + 4)
#define INTERRUPT_ID_HS_SPIM            (INTERNAL_ISR_TABLE_OFFSET + 5)
#define INTERRUPT_ID_PER_ERROR          (INTERNAL_ISR_TABLE_OFFSET + 6)
#define INTERRUPT_ID_UBUS_ERROR         (INTERNAL_ISR_TABLE_OFFSET + 7)
#define INTERRUPT_ID_UART2              (INTERNAL_ISR_TABLE_OFFSET + 8)
#define INTERRUPT_ID_KEYDONE            (INTERNAL_ISR_TABLE_OFFSET + 9)
#define INTERRUPT_ID_RNG_READY          (INTERNAL_ISR_TABLE_OFFSET + 10)
#define INTERRUPT_ID_USIM_ESD           (INTERNAL_ISR_TABLE_OFFSET + 11)
#define INTERRUPT_ID_USIM               (INTERNAL_ISR_TABLE_OFFSET + 12)
#define INTERRUPT_ID_USIM_PRES          (INTERNAL_ISR_TABLE_OFFSET + 13)
#define INTERRUPT_ID_DG                 (INTERNAL_ISR_TABLE_OFFSET + 14)
#define INTERRUPT_ID_EXTERNAL           (INTERNAL_ISR_TABLE_OFFSET + 15)
#define INTERRUPT_ID_RDP_RUNNER         (INTERNAL_ISR_TABLE_OFFSET + 16) /* 10 interrupts */
#define INTERRUPT_ID_RDP_UBUS_ERR       (INTERNAL_ISR_TABLE_OFFSET + 26)
#define INTERRUPT_ID_RDP_UBUS_PORT_ERR  (INTERNAL_ISR_TABLE_OFFSET + 27)
#define INTERRUPT_ID_WAN_8KHZ           (INTERNAL_ISR_TABLE_OFFSET + 28)
#define INTERRUPT_ID_WAN_EPON           (INTERNAL_ISR_TABLE_OFFSET + 29)
#define INTERRUPT_ID_WAN_GPON_RX        (INTERNAL_ISR_TABLE_OFFSET + 30)
#define INTERRUPT_ID_WAN_GPON_TX        (INTERNAL_ISR_TABLE_OFFSET + 31)

#define INTERRUPT_ID_RDP_MS1588         (INTERNAL_EXT_ISR_TABLE_OFFSET + 0)
#define INTERRUPT_ID_RDP_SBPM           (INTERNAL_EXT_ISR_TABLE_OFFSET + 1)
#define INTERRUPT_ID_RDP_BPM            (INTERNAL_EXT_ISR_TABLE_OFFSET + 2)
#define INTERRUPT_ID_RDP_QEGPHY         (INTERNAL_EXT_ISR_TABLE_OFFSET + 3) /* 4 irqs */
#define INTERRUPT_ID_WAN_NCO_GPON       (INTERNAL_EXT_ISR_TABLE_OFFSET + 7)
#define INTERRUPT_ID_USBH_OHCI          (INTERNAL_EXT_ISR_TABLE_OFFSET + 8)
#define INTERRUPT_ID_USBH_EHCI          (INTERNAL_EXT_ISR_TABLE_OFFSET + 9)
#define INTERRUPT_ID_USBH               INTERRUPT_ID_USBH_OHCI /*for compatibility*/
#define INTERRUPT_ID_USBH20             INTERRUPT_ID_USBH_EHCI /*for compatibility*/
#define INTERRUPT_ID_USBH_DISCON        (INTERNAL_EXT_ISR_TABLE_OFFSET + 10)
#define INTERRUPT_ID_USBH_CCS           (INTERNAL_EXT_ISR_TABLE_OFFSET + 11)
#define INTERRUPT_ID_PCIE_RC            (INTERNAL_EXT_ISR_TABLE_OFFSET + 12)
#define INTERRUPT_ID_PCIE1_RC           (INTERNAL_EXT_ISR_TABLE_OFFSET + 13)
#define INTERRUPT_ID_APM                (INTERNAL_EXT_ISR_TABLE_OFFSET + 14)
#define INTERRUPT_ID_APM_IUDMA          (INTERNAL_EXT_ISR_TABLE_OFFSET + 15)
#define INTERRUPT_ID_MDIO_EXT           (INTERNAL_EXT_ISR_TABLE_OFFSET + 17)
#define INTERRUPT_ID_MDIO_EGPHY         (INTERNAL_EXT_ISR_TABLE_OFFSET + 18)
#define INTERRUPT_ID_MDIO_SATA          (INTERNAL_EXT_ISR_TABLE_OFFSET + 19)
#define INTERRUPT_ID_MDIO_AEPCS         (INTERNAL_EXT_ISR_TABLE_OFFSET + 20)
#define INTERRUPT_ID_USBD_SYS           (INTERNAL_EXT_ISR_TABLE_OFFSET + 21)
#define INTERRUPT_ID_PER_ERR_PORT       (INTERNAL_EXT_ISR_TABLE_OFFSET + 28)


#define INTERRUPT_ID_EXTERNAL_0         (EXTERNAL_ISR_TABLE_OFFSET + 0)
#define INTERRUPT_ID_EXTERNAL_1         (EXTERNAL_ISR_TABLE_OFFSET + 1)
#define INTERRUPT_ID_EXTERNAL_2         (EXTERNAL_ISR_TABLE_OFFSET + 2)
#define INTERRUPT_ID_EXTERNAL_3         (EXTERNAL_ISR_TABLE_OFFSET + 3)
#define INTERRUPT_ID_EXTERNAL_4         (EXTERNAL_ISR_TABLE_OFFSET + 4)
#define INTERRUPT_ID_EXTERNAL_5         (EXTERNAL_ISR_TABLE_OFFSET + 5)

#define EXTERNAL_INTERRUPT_NUM           6

#define INTERRUPT_ID_LAST                INTERRUPT_ID_EXTERNAL_5


#define NUM_EXT_INT    (INTERRUPT_ID_EXTERNAL_5-INTERRUPT_ID_EXTERNAL_0+1)

#ifdef __cplusplus
    }
#endif                    

#endif  /* __BCM6838_H */


