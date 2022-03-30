/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2019 Broadcom Ltd.
 */
#ifndef _OTP_MAP_H
#define _OTP_MAP_H
/*  
 * 	OTP feature map.
 * 	An OTP field named in the corresponding SoC e.g 63138-63146 or 6858-6855 is being assigned a feature name
 * 	OTP map feature name closely resembles a specific OTP map row name. Other entries server as delimiters. 
 * 	This allows to unify access to OTP map across all SoCs  	
 * 	Whenever new otp field is needed to be read or fused it neets to be declared here and subsequently mapped in
 * 	the driver initilizer function to the feature name
 * 	Example:
 * 	OTP_MAP_LDO_TRIM -feature name 
 * 	OTP_MAP_LDO_TRIM_ROW - as defined in otp specific arch 
 *
 *  */
typedef enum otp_map_feat {
	OTP_MAP_INVALID = 0,
	OTP_MAP_SEC_CHIPVAR,
	OTP_MAP_LDO_TRIM,
	OTP_MAP_CPU_CLOCK_FREQ,
	OTP_MAP_PMC_BOOT,
	OTP_MAP_PCM_DISABLE,
	OTP_MAP_CPU_CORE_CFG,
	OTP_MAP_SGMII_DISABLE,
	OTP_MAP_SATA_DISABLE,
	OTP_MAP_BRCM_BTRM_BOOT_ENABLE,
	OTP_MAP_CUST_BTRM_BOOT_ENABLE,
	OTP_MAP_BRCM_ENFORCE_BINIT,
	OTP_MAP_CUST_MFG_MRKTID,
	OTP_MAP_BRCM_PRODUCTION_MODE,
	OTP_MAP_CUST_OP_INUSE,
	SOTP_MAP_INVALID,
	SOTP_MAP_FLD_ROE,
	SOTP_MAP_FLD_HMID,
	SOTP_MAP_KEY_DEV_SPECIFIC,
	SOTP_MAP_SER_NUM,
	SOTP_MAP_KEY_SECT_1,
	SOTP_MAP_KEY_SECT_2,
	SOTP_MAP_KEY_SECT_3,
	SOTP_MAP_KEY_SECT_4,
	SOTP_MAP_KEY_SECT_5,
	SOTP_MAP_ANTI_ROLLBACK,
	SOTP_MAP_UNUSED_1,
	SOTP_MAP_UNUSED_2,
	SOTP_MAP_UNUSED_3,
	SOTP_MAP_UNUSED_4,
	SOTP_MAP_UNUSED_5,
	SOTP_MAP_UNUSED_6,
	SOTP_MAP_UNUSED_7,
	OTP_MAP_MAX
} otp_map_feat_t;




#endif
