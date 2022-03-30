/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2020 Broadcom Ltd.
 */
#ifndef _SKO_H
#define _SKO_H

/* SECURE KEY OBJECT - SKO */


#define SKO_BASE 0xff804000


#define SKO_KEYN_PAC_CNTRL_SHIFT        	0x0
#define SKO_KEYN_PAC_CNTRL_MASK         	0xffffffff
#define SKO_KEYN_PAC_CTRL_OFFSET		0x00 
      
#define SKO_KEYN_STATUS_SHIFT           	31
#define SKO_KEYN_STATUS_MASK            	1
#define SKO_KEYN_STATUS_OFFSET			0x04
       
#define SKO_KEYN_CONTROL_SHIFT           	31
#define SKO_KEYN_CONTROL_MASK            	1
#define SKO_KEYN_CTRL_OFFSET                    0x0C
      
#define SKO_KEYN_DATA_SHIFT             	0x0
#define SKO_KEYN_DATA_MASK              	0xffffffff 
#define SKO_KEYN_DATA_OFFSET                    0x10 

#define SKO_SIZE				0x40
 
#define SKO_KEY_DATA_GOOD(_R_)			((_R_ >> SKO_KEYN_STATUS_SHIFT)&SKO_KEYN_STATUS_MASK) 
#define SKO_KEY_CTRL_OTP_WLOCK(_R_)		((_R_ >> SKO_KEYN_CONTROL_SHIFT)&SKO_KEYN_CONTROL_MASK) 

#define SKO_ROW_SHIFT				0
#define SKO_ROW_MASK				0xffffffff

#define SKO_0_DESC_CONF				0x9b009000
#define SKO_0_CTL_CONF				0x80000000
#define SKO_0_DESC_ROW				42
#define SKO_0_CTL_ROW				46
#define SKO_0_DATA_ROW				47
#define SKO_0_CRC_ROW				55

#define SKO_1_DESC_CONF				0x9b01300a
#define SKO_1_CTL_CONF				0x80000000
#define SKO_1_DESC_ROW				43
#define SKO_1_CTL_ROW				56
#define SKO_1_DATA_ROW				57
#define SKO_1_CRC_ROW				65

/*Block bits are sticky until reset */
#define SKO_PAC_MASK				0xff
#define SKO_PERM_PAC_KEY_SHIFT			0x4
#define SKO_PERM_PAC_SHIFT			0x0
#define SKO_PAC_PERM_MASK			0xf

#define SKO_PAC_LOCK_MASK			0x1

#define SKO_PAC_KEY_SW_LOCK_SHIFT		7
#define SKO_PAC_KEY_SR_LOCK_SHIFT		6
#define SKO_PAC_KEY_NSW_LOCK_SHIFT		5
#define SKO_PAC_KEY_NSR_LOCK_SHIFT		4
#define SKO_PAC_PERM_SW_LOCK_SHIFT		3	
#define SKO_PAC_PERM_SR_LOCK_SHIFT		2
#define SKO_PAC_PERM_NSW_LOCK_SHIFT		1
#define SKO_PAC_PERM_NSR_LOCK_SHIFT		0

#define SKO_PERM_LOCK_ALL			0xf0

#define SKO_PERM_KEY_SR_LOCK			(0x1<<SKO_PAC_KEY_SR_LOCK_SHIFT)
#define SKO_PERM_KEY_NSR_LOCK			(0x1<<SKO_PAC_KEY_NSR_LOCK_SHIFT)

#define SKO_PERM_KEY_SW_LOCK			(0x1<<SKO_PAC_KEY_SW_LOCK_SHIFT)
#define SKO_PERM_KEY_NSW_LOCK			(0x1<<SKO_PAC_KEY_NSW_LOCK_SHIFT)

#define SKO_PERM_PAC_SR_LOCK			(0x1<<SKO_PAC_PERM_SR_LOCK_SHIFT)
#define SKO_PERM_PAC_NSR_LOCK			(0x1<<SKO_PAC_PERM_NSR_LOCK_SHIFT)

#define SKO_PERM_PAC_SW_LOCK			(0x1<<SKO_PAC_PERM_SW_LOCK_SHIFT)
#define SKO_PERM_PAC_NSW_LOCK			(0x1<<SKO_PAC_PERM_NSW_LOCK_SHIFT)


/* Lock enable bit for PERM_PAC is 0; for PERM_KEY 1  */
#define SKO_PERM_PAC_LOCK_SET(_R_, _V_)		(_R_ &= (~(_V_)) ) 	
#define SKO_PERM_KEY_LOCK_SET(_R_, _V_)		(_R_ |= (_V_))

#define SKO_SECT_MAX				2 
#define SKO_NON_SECT				3 

/* special case for 6878 only - belongs to otp.h */
#define OTP_BOOT_SW_0_ROW              		26
#define OTP_BOOT_SW_0_SHIFT                     0
#define OTP_BOOT_SW_0_MASK                	0xffffffff
#define SKO_ADDR2SECT(__A)\
	( __A != SKO_0_DATA_ROW? (__A != SKO_1_DATA_ROW? (__A < OTP_BOOT_SW_0_ROW? __A : SKO_NON_SECT) : 1 ) : 0) 


/* An SOTP row initializer that maps actual row number with mask and shift to a feature name;
 * this allows to use features vs. rows for common functionality, 
 * such as secure keys reading and fusing 
 * prevent ifdef dependencies when used outside of arch directories for common among SoCs logic
 * */
#define	DEFINE_SOTP_MAP_ROW_INITLR(__VV__)					\
	static otp_hw_cmn_row_t __VV__[ ] = {					\
	{SOTP_MAP_FLD_ROE, SKO_0_DATA_ROW, SKO_ROW_MASK, SKO_ROW_SHIFT, 8, .conf = {.arg = SKO_0_CTL_ROW, .op_type = OTP_HW_CMN_CTL_OTPCMD_ECC} },		\
	{SOTP_MAP_KEY_DEV_SPECIFIC, SKO_1_DATA_ROW, SKO_ROW_MASK, SKO_ROW_SHIFT, 8, .conf = {.arg = SKO_1_CTL_ROW, .op_type = OTP_HW_CMN_CTL_OTPCMD_ECC}},	\
	{OTP_MAP_INVALID, SKO_0_CTL_ROW, SKO_ROW_MASK, SKO_ROW_SHIFT, 1, .conf = {.arg = SKO_0_CTL_CONF, .op_type = OTP_HW_CMN_CTL_OTPCMD_ECC} },	\
	{OTP_MAP_INVALID, SKO_0_DESC_ROW, SKO_ROW_MASK, SKO_ROW_SHIFT, 1, .conf = {.arg = SKO_0_DESC_CONF, .op_type = OTP_HW_CMN_CTL_OTPCMD_ECC} },	\
	{OTP_MAP_INVALID, SKO_1_CTL_ROW, SKO_ROW_MASK, SKO_ROW_SHIFT, 1,  .conf = {.arg = SKO_1_CTL_CONF, .op_type = OTP_HW_CMN_CTL_OTPCMD_ECC} },  	\
	{OTP_MAP_INVALID, SKO_1_DESC_ROW, SKO_ROW_MASK, SKO_ROW_SHIFT, 1, .conf = {.arg = SKO_1_DESC_CONF, .op_type = OTP_HW_CMN_CTL_OTPCMD_ECC} },	\
	{SOTP_MAP_FLD_HMID, OTP_BOOT_SW_0_ROW,	OTP_BOOT_SW_0_MASK, OTP_BOOT_SW_0_SHIFT, 8}	};



#endif

