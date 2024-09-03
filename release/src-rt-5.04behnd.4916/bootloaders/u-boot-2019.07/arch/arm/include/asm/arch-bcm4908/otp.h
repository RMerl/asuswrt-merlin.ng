/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2020 Broadcom Ltd.
 */

#ifndef _4908_OTP_H
#define _4908_OTP_H

#define JTAG_OTP_BASE    0xff800e00

/* row 14 */
#define OTP_CPU_CORE_CFG_ROW                    14
#define OTP_CPU_CORE_CFG_SHIFT                  14
#define OTP_CPU_CORE_CFG_MASK                   0x3

#define OTP_SGMII_DISABLE_ROW                   14
#define OTP_SGMII_DISABLE_SHIFT                 17
#define OTP_SGMII_DISABLE_MASK                  0x1

/* row 17 */
#define OTP_BRCM_BTRM_BOOT_ENABLE_ROW           17
#define OTP_BRCM_BTRM_BOOT_ENABLE_SHIFT         3
#define OTP_BRCM_BTRM_BOOT_ENABLE_MASK          0x1

/* row 18 */
#define OTP_CUST_BTRM_BOOT_ENABLE_ROW           18
#define OTP_CUST_BTRM_BOOT_ENABLE_SHIFT         15
#define OTP_CUST_BTRM_BOOT_ENABLE_MASK          0x7

/* row 24 */
#define OTP_CUST_MFG_MRKTID_ROW                 24
#define OTP_CUST_MFG_MRKTID_SHIFT               0
#define OTP_CUST_MFG_MRKTID_MASK                0xffff

/* rows 19 & 20 */
#define OTP_JTAG_SER_NUM_ROW_1                  19          // Row19[25:20] = CSEC_CHIPID[5:0]
#define OTP_JTAG_SER_NUM_MASK_1                 0x0000003F
#define OTP_JTAG_SER_NUM_SHIFT_1                20
#define OTP_JTAG_SER_NUM_ROW_2                  20          // Row20[25:0]  = CSEC_CHIPID[31:6]
#define OTP_JTAG_SER_NUM_MASK_2                 0xFFFFFFC0
#define OTP_JTAG_SER_NUM_SHIFT_2                (OTP_HW_REG_SHIFT_LEFT_FLAG|6)

/* A row initializer that maps actual row number with mask and shift to a feature name;
 * this allows to use features vs. rows for common functionality, 
 * such as secure boot handling frequency, chipid and so on 
 * prevent ifdef dependencies when used outside of arch directories for common among SoCs logic
 * */
#define	DEFINE_OTP_MAP_ROW_INITLR(__VV__)									\
	static otp_hw_cmn_row_t __VV__[ ] = {									\
	{OTP_MAP_BRCM_BTRM_BOOT_ENABLE, OTP_BRCM_BTRM_BOOT_ENABLE_ROW, OTP_BRCM_BTRM_BOOT_ENABLE_MASK, OTP_BRCM_BTRM_BOOT_ENABLE_SHIFT, 1},\
	{OTP_MAP_CUST_BTRM_BOOT_ENABLE, OTP_CUST_BTRM_BOOT_ENABLE_ROW, OTP_CUST_BTRM_BOOT_ENABLE_MASK, OTP_CUST_BTRM_BOOT_ENABLE_SHIFT, 1},\
	{OTP_MAP_CUST_MFG_MRKTID, OTP_CUST_MFG_MRKTID_ROW, OTP_CUST_MFG_MRKTID_MASK, OTP_CUST_MFG_MRKTID_SHIFT, 1},				\
	{OTP_MAP_CPU_CORE_CFG, OTP_CPU_CORE_CFG_ROW, OTP_CPU_CORE_CFG_MASK, OTP_CPU_CORE_CFG_SHIFT, 1},				\
	{OTP_MAP_SGMII_DISABLE, OTP_SGMII_DISABLE_ROW, OTP_SGMII_DISABLE_MASK, OTP_SGMII_DISABLE_SHIFT, 1},				\
        {OTP_MAP_CSEC_CHIPID, OTP_JTAG_SER_NUM_ROW_1, OTP_JTAG_SER_NUM_MASK_1, OTP_JTAG_SER_NUM_SHIFT_1, 1},                           \
        {OTP_MAP_CSEC_CHIPID_EXTRA, OTP_JTAG_SER_NUM_ROW_2, OTP_JTAG_SER_NUM_MASK_2, OTP_JTAG_SER_NUM_SHIFT_2, 1},                             \
	}

#endif
