/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2020 Broadcom Ltd.
 */

#ifndef _6756_OTP_H
#define _6756_OTP_H


#define JTAG_OTP_BASE   0xff802800

/* row 8 */
#define OTP_CPU_CORE_CFG_ROW            8
#define OTP_CPU_CORE_CFG_SHIFT          28
#define OTP_CPU_CORE_CFG_MASK           0x7

/* row 9 */
#define OTP_CPU_CLOCK_FREQ_ROW          9
#define OTP_CPU_CLOCK_FREQ_SHIFT        0
#define OTP_CPU_CLOCK_FREQ_MASK         0x7

#define OTP_LDO_TRIM_ROW                9
#define OTP_LDO_TRIM_SHIFT              28
#define OTP_LDO_TRIM_MASK               0xf

/* row 14 */
#define OTP_SGMII_DISABLE_ROW           14
#define OTP_SGMII_DISABLE_SHIFT         5
#define OTP_SGMII_DISABLE_MASK          0x1

/* row 17 */
#define OTP_BRCM_BTRM_BOOT_ENABLE_ROW           17
#define OTP_BRCM_BTRM_BOOT_ENABLE_SHIFT         3
#define OTP_BRCM_BTRM_BOOT_ENABLE_MASK          1

/* row 18 */
#define OTP_CUST_BTRM_BOOT_ENABLE_ROW           18
#define OTP_CUST_BTRM_BOOT_ENABLE_SHIFT         15
#define OTP_CUST_BTRM_BOOT_ENABLE_MASK          7

/* row 23 */
#define OTP_CUST_MFG_MRKTID_ROW                 23
#define OTP_CUST_MFG_MRKTID_SHIFT               0
#define OTP_CUST_MFG_MRKTID_MASK                0xffff

/* row 9 */
#define OTP_LDO_TRIM_ROW                9
#define OTP_LDO_TRIM_SHIFT              28
#define OTP_LDO_TRIM_MASK               0xf

/* row 14 */
#define OTP_SGMII_DISABLE_ROW           14
#define OTP_SGMII_DISABLE_SHIFT         5
#define OTP_SGMII_DISABLE_MASK          0x1


#define	DEFINE_OTP_MAP_ROW_INITLR(__VV__)									\
	static otp_hw_cmn_row_t __VV__[ ] = {									\
	{OTP_MAP_BRCM_BTRM_BOOT_ENABLE, OTP_BRCM_BTRM_BOOT_ENABLE_ROW, OTP_BRCM_BTRM_BOOT_ENABLE_MASK, OTP_BRCM_BTRM_BOOT_ENABLE_SHIFT, 1},\
	{OTP_MAP_CUST_BTRM_BOOT_ENABLE, OTP_CUST_BTRM_BOOT_ENABLE_ROW, OTP_CUST_BTRM_BOOT_ENABLE_MASK, OTP_CUST_BTRM_BOOT_ENABLE_SHIFT, 1},\
	{OTP_MAP_CUST_MFG_MRKTID, OTP_CUST_MFG_MRKTID_ROW, OTP_CUST_MFG_MRKTID_MASK, OTP_CUST_MFG_MRKTID_SHIFT, 1},				\
	{OTP_MAP_CPU_CORE_CFG, OTP_CPU_CORE_CFG_ROW, OTP_CPU_CORE_CFG_MASK, OTP_CPU_CORE_CFG_SHIFT, 1},				\
	{OTP_MAP_LDO_TRIM, OTP_LDO_TRIM_ROW, OTP_LDO_TRIM_MASK, OTP_LDO_TRIM_SHIFT, 1},				\
	}

#endif
