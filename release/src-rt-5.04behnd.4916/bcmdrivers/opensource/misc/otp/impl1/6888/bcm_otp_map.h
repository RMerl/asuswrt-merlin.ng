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

#include "../bcm_otp_v3_map.h"

/* row 13 */
#define OTP_BRCM_BTRM_BOOT_ENABLE_ROW           13
#define OTP_BRCM_BTRM_BOOT_ENABLE_SHIFT         2
#define OTP_BRCM_BTRM_BOOT_ENABLE_MASK          (1 << OTP_BRCM_BTRM_BOOT_ENABLE_SHIFT)

/* row 14 */
#define OTP_CUST_BTRM_BOOT_ENABLE_ROW           14
#define OTP_CUST_BTRM_BOOT_ENABLE_SHIFT         28
#define OTP_CUST_BTRM_BOOT_ENABLE_MASK          (1 << OTP_CUST_BTRM_BOOT_ENABLE_SHIFT)

/* row 16 */
#define OTP_PCM_DISABLE_ROW             16
#define OTP_PCM_DISABLE_SHIFT           1
#define OTP_PCM_DISABLE_MASK            (0x1 << OTP_PCM_DISABLE_SHIFT)

/* row 15 */
#define OTP_CHIPID_ROW              15
#define OTP_CHIPID_SHIFT            0
#define OTP_CHIP_ID_MASK            (0xfffff << OTP_CHIPID_SHIFT)

/* row 29 */
#define OTP_CUST_MFG_MRKTID_ROW                 29
#define OTP_CUST_MFG_MRKTID_SHIFT               0
#define OTP_CUST_MFG_MRKTID_MASK                (0xffff << OTP_CUST_MFG_MRKTID_SHIFT)

#endif
