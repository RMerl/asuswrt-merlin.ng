/*
<:copyright-BRCM:2021:DUAL/GPL:standard 

   Copyright (c) 2021 Broadcom 
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

#ifndef BCM_OTP_MAP_H
#define BCM_OTP_MAP_H

#include "../bcm_otp_v2_map.h"

#define SOTP_OTP_REG_RD_LOCK_OFF               0x3c
/* row 8 */
#define OTP_CPU_CORE_CFG_ROW            8
#define OTP_CPU_CORE_CFG_SHIFT          28
#define OTP_CPU_CORE_CFG_MASK           (0x1 << OTP_CPU_CORE_CFG_SHIFT)

#define OTP_SEC_CHIPVAR_ROW         8
#define OTP_SEC_CHIPVAR_SHIFT       24
#define OTP_SEC_CHIPVAR_MASK        (0xf << OTP_SEC_CHIPVAR_SHIFT)

/* row 9 */
#define OTP_CPU_CLOCK_FREQ_ROW          9
#define OTP_CPU_CLOCK_FREQ_SHIFT        0
#define OTP_CPU_CLOCK_FREQ_MASK         (0x7 << OTP_CPU_CLOCK_FREQ_SHIFT)

/* row 14 */
#define OTP_PCM_DISABLE_ROW             14
#define OTP_PCM_DISABLE_SHIFT           13
#define OTP_PCM_DISABLE_MASK            (0x1 << OTP_PCM_DISABLE_SHIFT)

/* row 17 */
#define OTP_BRCM_BTRM_BOOT_ENABLE_ROW           17
#define OTP_BRCM_BTRM_BOOT_ENABLE_SHIFT         3
#define OTP_BRCM_BTRM_BOOT_ENABLE_MASK          (1 << OTP_BRCM_BTRM_BOOT_ENABLE_SHIFT)

/* row 18 */
#define OTP_CUST_BTRM_BOOT_ENABLE_ROW           18
#define OTP_CUST_BTRM_BOOT_ENABLE_SHIFT         15
#define OTP_CUST_BTRM_BOOT_ENABLE_MASK          (7 << OTP_CUST_BTRM_BOOT_ENABLE_SHIFT)

/* row 20 */
#define OTP_CHIPID_ROW              20
#define OTP_CHIPID_SHIFT            0
#define OTP_CHIP_ID_MASK            (0xffffffff << OTP_CHIPID_SHIFT)

/* row 23 */
#define OTP_CUST_MFG_MRKTID_ROW                 23
#define OTP_CUST_MFG_MRKTID_SHIFT               0
#define OTP_CUST_MFG_MRKTID_MASK                (0xffff << OTP_CUST_MFG_MRKTID_SHIFT)

#endif
