/*
 * <:copyright-BRCM:2022:DUAL/GPL:standard
 * 
 *    Copyright (c) 2022 Broadcom 
 *    All Rights Reserved
 * 
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 * 
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 * 
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
 * 
 * :>
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
    OTP_MAP_CUST_MFG_MRKTID,
    OTP_MAP_JTAG_PWD,
    OTP_MAP_CSEC_CHIPID,
    OTP_MAP_DBG_MODE,
    OTP_MAP_JU_MODE,
    OTP_MAP_LEDS,
    OTP_MAP_MAX
} otp_map_feat_t;

typedef enum _otp_map_cmn_err_ {
    OTP_MAP_CMN_OK = 0,
    OTP_MAP_CMN_ERR_FAIL = -1,
    OTP_MAP_CMN_ERR_UNSP = -2,
    OTP_MAP_CMN_ERR_INVAL= -3,
} otp_map_cmn_err_t;

#endif
