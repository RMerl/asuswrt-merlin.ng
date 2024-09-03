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
#define OTP_SATA_DISABLE_ROW			10
#define OTP_SATA_DISABLE_SHIFT			24
#define OTP_SATA_DISABLE_MASK			(0x1 << OTP_SATA_DISABLE_SHIFT)

/* row 11 */
#define OTP_PMC_BOOT_ROW			11
#define OTP_PMC_BOOT_SHIFT			25
#define OTP_PMC_BOOT_MASK			(0x1 << OTP_PMC_BOOT_SHIFT)

/* row 12 */
#define OTP_PCM_DISABLE_ROW			12
#define OTP_PCM_DISABLE_SHIFT			6
#define OTP_PCM_DISABLE_MASK			(0x1 << OTP_PCM_DISABLE_SHIFT)

/* row 14 */
#define OTP_CPU_CORE_CFG_ROW			14
#define OTP_CPU_CORE_CFG_SHIFT			14
#define OTP_CPU_CORE_CFG_MASK			(0x3 << OTP_CPU_CORE_CFG_SHIFT)

#define OTP_SGMII_DISABLE_ROW			14
#define OTP_SGMII_DISABLE_SHIFT			17
#define OTP_SGMII_DISABLE_MASK			(0x1 << OTP_SGMII_DISABLE_SHIFT)

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
