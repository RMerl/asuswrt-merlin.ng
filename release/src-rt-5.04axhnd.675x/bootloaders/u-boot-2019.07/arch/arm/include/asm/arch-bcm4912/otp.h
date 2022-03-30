/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2020 Broadcom Ltd.
 */

#ifndef _4912_OTP_H
#define _4912_OTP_H

#define JTAG_OTP_BASE    0xff802800

/* row 9 */
#define OTP_CPU_CLOCK_FREQ_ROW                  9
#define OTP_CPU_CLOCK_FREQ_SHIFT                0
#define OTP_CPU_CLOCK_FREQ_MASK                 0x7

/* row 8 */
#define OTP_CPU_CORE_CFG_ROW                    8
#define OTP_CPU_CORE_CFG_SHIFT                  28
#define OTP_CPU_CORE_CFG_MASK                   0x1 // 0=dual cores, 1=single core

/* row 14 */
#define OTP_JTAG_CUST_LOCK_ROW			0xff	
#define OTP_JTAG_CUST_LOCK_MASK			0x1
#define OTP_JTAG_CUST_LOCK_REG_SHIFT		25

/* row 13 */
#define OTP_BRCM_PRODUCTION_MODE_ROW       	13
#define OTP_BRCM_PRODUCTION_MODE_SHIFT     	0
#define OTP_BRCM_PRODUCTION_MODE_MASK      	0x1

/* row 13 */
#define OTP_BRCM_BTRM_BOOT_ENABLE_ROW           13
#define OTP_BRCM_BTRM_BOOT_ENABLE_SHIFT         2
#define OTP_BRCM_BTRM_BOOT_ENABLE_MASK          1

/* row 14 */
#define OTP_CUST_BTRM_BOOT_ENABLE_ROW           14
#define OTP_CUST_BTRM_BOOT_ENABLE_SHIFT         28
#define OTP_CUST_BTRM_BOOT_ENABLE_MASK          0x1 

/* row 14 */
#define OTP_CUST_BTRM_UART_DISABLE_ROW          14
#define OTP_CUST_BTRM_UART_DISABLE_SHIFT        0
#define OTP_CUST_BTRM_UART_DISABLE_MASK         1

/* row 14 */
#define OTP_CUST_BTRM_MSG_DISABLE_ROW           14
#define OTP_CUST_BTRM_MSG_DISABLE_SHIFT         27
#define OTP_CUST_BTRM_MSG_DISABLE_MASK          1

/* row 29 */
#define OTP_CUST_MFG_MRKTID_ROW                 29
#define OTP_CUST_MFG_MRKTID_SHIFT               0
#define OTP_CUST_MFG_MRKTID_MASK                0xffff

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
	{OTP_MAP_BRCM_PRODUCTION_MODE, OTP_BRCM_PRODUCTION_MODE_ROW, OTP_BRCM_PRODUCTION_MODE_MASK, OTP_BRCM_PRODUCTION_MODE_SHIFT, 1},				\
	}
#endif

