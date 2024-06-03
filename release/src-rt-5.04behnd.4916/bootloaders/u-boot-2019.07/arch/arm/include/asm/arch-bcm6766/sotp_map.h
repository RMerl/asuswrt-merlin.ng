/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2020 Broadcom Ltd.
 */
#ifndef _SOTP_MAP_H
#define _SOTP_MAP_H
/* An SOTP row initializer that maps actual row number with mask and shift to a feature name;
 * this allows to use features vs. rows for common functionality, 
 * such as secure keys reading and fusing 
 * prevent ifdef dependencies when used outside of arch directories for common among SoCs logic
 * */
#define	DEFINE_SOTP_MAP_ROW_INITLR(__VV__)														\
static otp_hw_cmn_row_t __VV__[ ] = {															\
		{.feat = SOTP_MAP_FLD_ROE,          .addr = (KSR_START_ROW_ADDR+0), .range = 8, .conf = {.perm = KSR_PORTAL_PGM_DESC_SRW_PERM, .op_type = OTP_HW_CMN_CTL_OTPCMD_ECC}},	\
		{.feat = SOTP_MAP_FLD_HMID,         .addr = (KSR_START_ROW_ADDR+1), .range = 8, .conf = {.perm = KSR_PORTAL_PGM_DESC_SRW_PERM, .op_type = OTP_HW_CMN_CTL_OTPCMD_ECC}},	\
		{.feat = SOTP_MAP_FLD_ROE1,          .addr = (KSR_START_ROW_ADDR+2), .range = 8, .conf = {.perm = KSR_PORTAL_PGM_DESC_SRW_PERM, .op_type = OTP_HW_CMN_CTL_OTPCMD_ECC}},	\
		{.feat = SOTP_MAP_KEY_DEV_SPECIFIC, .addr = (KSR_START_ROW_ADDR+3), .range = 8, .conf = {.perm = (KSR_PORTAL_PGM_DESC_NRW_PERM), .op_type = OTP_HW_CMN_CTL_OTPCMD_ECC}},\
		{.feat = SOTP_MAP_KEY_SECT_1,       .addr = (KSR_START_ROW_ADDR+4), .range = 8, .conf = {.perm = (KSR_PORTAL_PGM_DESC_NRW_PERM), .op_type = OTP_HW_CMN_CTL_OTPCMD_ECC}},\
		{.feat = SOTP_MAP_KEY_SECT_2,       .addr = (KSR_START_ROW_ADDR+5), .range = 8, .conf = {.perm = (KSR_PORTAL_PGM_DESC_NRW_PERM), .op_type = OTP_HW_CMN_CTL_OTPCMD_ECC}},\
		{.feat = SOTP_MAP_ANTI_ROLLBACK,    .addr = (FSR_START_ROW_ADDR+0), .range = 4, .conf = {.op_type = OTP_HW_CMN_CTL_CONF}},	\
		{.feat = SOTP_MAP_UNUSED_1,         .addr = (FSR_START_ROW_ADDR+1), .range = 4, .conf = {.op_type = OTP_HW_CMN_CTL_CONF}},	\
		{.feat = SOTP_MAP_UNUSED_2,         .addr = (FSR_START_ROW_ADDR+2), .range = 8, .conf = {.op_type = OTP_HW_CMN_CTL_CONF}},	\
		{.feat = SOTP_MAP_UNUSED_3,         .addr = (FSR_START_ROW_ADDR+3), .range = 8, .conf = {.op_type = OTP_HW_CMN_CTL_CONF}},	\
		{.feat = SOTP_MAP_UNUSED_4,         .addr = (FSR_START_ROW_ADDR+4), .range = 2, .conf = {.op_type = OTP_HW_CMN_CTL_CONF}},	\
		{.feat = SOTP_MAP_SER_NUM,          .addr = (FSR_START_ROW_ADDR+5), .range = 4, .conf = {.op_type = OTP_HW_CMN_CTL_OTPCMD_ECC}},	\
		{.feat = SOTP_MAP_UNUSED_6,         .addr = (FSR_START_ROW_ADDR+6), .range = 4, .conf = {.op_type = OTP_HW_CMN_CTL_CONF}},	\
		{.feat = SOTP_MAP_UNUSED_7,         .addr = (FSR_START_ROW_ADDR+7), .range = 2, .conf = {.op_type = OTP_HW_CMN_CTL_CONF}},	\
	};

#endif

