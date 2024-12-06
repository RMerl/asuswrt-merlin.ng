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

#include "../bcm_otp_v3_map.h"

/* row 8 */
#define OTP_CPU_CORE_CFG_ROW                    8
#define OTP_CPU_CORE_CFG_SHIFT                  28
#define OTP_CPU_CORE_CFG_MASK                   (0x3 << OTP_CPU_CORE_CFG_SHIFT)

/* row 9 */
#define OTP_CPU_CLOCK_FREQ_ROW                  9
#define OTP_CPU_CLOCK_FREQ_SHIFT                0
#define OTP_CPU_CLOCK_FREQ_MASK                 (0x7 << OTP_CPU_CLOCK_FREQ_SHIFT)

/* row 13 */
#define OTP_BRCM_BTRM_PRODUCTION_MODE_ROW       13
#define OTP_BRCM_BTRM_PRODUCTION_MODE_SHIFT    	0
#define OTP_BRCM_BTRM_PRODUCTION_MODE_MASK      (1 << OTP_BRCM_BTRM_PRODUCTION_MODE_SHIFT)
/* row 13 */
#define OTP_BRCM_BTRM_BOOT_ENABLE_ROW           13
#define OTP_BRCM_BTRM_BOOT_ENABLE_SHIFT        	2
#define OTP_BRCM_BTRM_BOOT_ENABLE_MASK          (1 << OTP_BRCM_BTRM_BOOT_ENABLE_SHIFT)

/* row 14 */
#define OTP_CUST_BTRM_BOOT_ENABLE_ROW           14
#define OTP_CUST_BTRM_BOOT_ENABLE_SHIFT        	28
#define OTP_CUST_BTRM_BOOT_ENABLE_MASK          (1 << OTP_CUST_BTRM_BOOT_ENABLE_SHIFT)

/* row 14 */
#define OTP_CUST_BTRM_UART_DISABLE_ROW         	14
#define OTP_CUST_BTRM_UART_DISABLE_SHIFT       	0
#define OTP_CUST_BTRM_UART_DISABLE_MASK         (1 << OTP_CUST_BTRM_UART_DISABLE_SHIFT)

/* row 14 */
#define OTP_CUST_BTRM_MSG_DISABLE_ROW         	14
#define OTP_CUST_BTRM_MSG_DISABLE_SHIFT       	27
#define OTP_CUST_BTRM_MSG_DISABLE_MASK          (1 << OTP_CUST_BTRM_MSG_DISABLE_SHIFT)

/* row 16 */
#define OTP_PCIE_PORT_DISABLE_ROW               16
#define OTP_PCIE_PORT_DISABLE_SHIFT             4
#define OTP_PCIE_PORT_DISABLE_MASK              (0xF << OTP_PCIE_PORT_DISABLE_SHIFT)

/* row 17 */
#define OTP_DGASP_TRIM_ROW                      17
#define OTP_DGASP_TRIM_SHIFT                    28  /* TRIM = ROW17[31:28] */
#define OTP_DGASP_TRIM_MASK                     (0xf << OTP_DGASP_TRIM_SHIFT)
#define OTP_DGASP_TRIM_THRESH_SHIFT             0   /* THRESH = TRIM[1:0] */
#define OTP_DGASP_TRIM_THRESH_MASK              0x3
#define OTP_DGASP_TRIM_HYS_SHIFT                2   /* HYS = TRIM[3:2] */
#define OTP_DGASP_TRIM_HYS_MASK                 0xc

/* row 29 */
#define OTP_CUST_MFG_MRKTID_ROW                 29
#define OTP_CUST_MFG_MRKTID_SHIFT               0
#define OTP_CUST_MFG_MRKTID_MASK                (0xffff << OTP_CUST_MFG_MRKTID_SHIFT)

/* row 26 */
#define OTP_BOOT_SW_ENET_BOOT_DIS_ROW           26
#define OTP_BOOT_SW_ENET_BOOT_DIS_SHIFT         0
#define OTP_BOOT_SW_ENET_BOOT_DIS_MASK          (7 << OTP_BOOT_SW_ENET_BOOT_DIS_SHIFT)
#define OTP_BOOT_SW_ENET_BOOT_FALLBACK_SHIFT    3
#define OTP_BOOT_SW_ENET_BOOT_FALLBACK_MASK     (7 << OTP_BOOT_SW_ENET_BOOT_FALLBACK_SHIFT)
#define OTP_BOOT_SW_ENET_RGMII_SHIFT            6
#define OTP_BOOT_SW_ENET_RGMII_MASK             (7 << OTP_BOOT_SW_ENET_RGMII_SHIFT)

#endif
