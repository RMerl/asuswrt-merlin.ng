/*
<:copyright-BRCM:2021:DUAL/GPL:standard 

   Copyright (c) 2021 Broadcom 
   All Rights Reserved

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

:>

*/

#ifndef BCM_OTP_MAP_H
#define BCM_OTP_MAP_H

#include "../bcm_otp_v1_map.h"

#define SOTP_OTP_REG_RD_LOCK_OFF               0x3c

/* row 10 */
#define OTP_SATA_DISABLE_ROW        10
#define OTP_SATA_DISABLE_SHIFT      24
#define OTP_SATA_DISABLE_MASK       (0x1 << OTP_SATA_DISABLE_SHIFT)

#define OTP_USB_DISABLE_ROW         10
#define OTP_USB_DISABLE0_SHIFT      17
#define OTP_USB_DISABLE0_MASK       (0x1 << OTP_USB_DISABLE0_SHIFT)
#define OTP_USB_DISABLE_XHCI_SHIFT  18
#define OTP_USB_DISABLE_XHCI_MASK   (0x1 << OTP_USB_DISABLE_XHCI_SHIFT)
#define OTP_USB_DISABLE1_SHIFT      19
#define OTP_USB_DISABLE1_MASK       (0x1 << OTP_USB_DISABLE1_SHIFT)

/* row 11 */
#define OTP_PMC_BOOT_ROW            11
#define OTP_CHIPID_ROW              11
#define OTP_CHIPID_SHIFT            0
#define OTP_CHIP_ID_MASK            (0xfffff << OTP_CHIPID_SHIFT)
#define OTP_PMC_BOOT_SHIFT          25
#define OTP_PMC_BOOT_MASK           (0x1 << OTP_PMC_BOOT_SHIFT)

/* row 12 */
#define OTP_PCM_DISABLE_ROW         12
#define OTP_PCM_DISABLE_SHIFT       12
#define OTP_PCM_DISABLE_MASK        (0x1 << OTP_PCM_DISABLE_SHIFT)

#define OTP_SEC_CHIPVAR_ROW         12
#define OTP_SEC_CHIPVAR_SHIFT       0
#define OTP_SEC_CHIPVAR_MASK        (0xf << OTP_SEC_CHIPVAR_SHIFT)

/* row 14 */
#define OTP_CPU_CLOCK_FREQ_ROW          14
#define OTP_CPU_CLOCK_FREQ_SHIFT        9
#define OTP_CPU_CLOCK_FREQ_MASK         (0x7 << OTP_CPU_CLOCK_FREQ_SHIFT)

#define OTP_CPU_CORE_CFG_ROW            14
#define OTP_CPU_CORE_CFG_SHIFT          14
#define OTP_CPU_CORE_CFG_MASK           (0x3 << OTP_CPU_CORE_CFG_SHIFT)

/* row 17 */
#define OTP_BRCM_BTRM_BOOT_ENABLE_ROW           17
#define OTP_BRCM_BTRM_BOOT_ENABLE_SHIFT         3
#define OTP_BRCM_BTRM_BOOT_ENABLE_MASK          (1 << OTP_BRCM_BTRM_BOOT_ENABLE_SHIFT)

/* row 18 */
#define OTP_CUST_BTRM_BOOT_ENABLE_ROW           18
#define OTP_CUST_BTRM_BOOT_ENABLE_SHIFT         15
#define OTP_CUST_BTRM_BOOT_ENABLE_MASK          (7 << OTP_CUST_BTRM_BOOT_ENABLE_SHIFT)

/* row 24 */
#define OTP_CUST_MFG_MRKTID_ROW                 24
#define OTP_CUST_MFG_MRKTID_SHIFT               0
#define OTP_CUST_MFG_MRKTID_MASK                (0xffff << OTP_CUST_MFG_MRKTID_SHIFT)

#endif
