/*
 * <:copyright-BRCM:2022:DUAL/GPL:standard
 * 
 *    Copyright (c) 2022 Broadcom 
 *    All Rights Reserved
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as published by
 * the Free Software Foundation (the "GPL").
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * 
 * A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
 * writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
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
    OTP_MAP_UART_EN,
    OTP_MAP_MAX
} otp_map_feat_t;

typedef enum _otp_map_cmn_err_ {
    OTP_MAP_CMN_OK = 0,
    OTP_MAP_CMN_ERR_FAIL = -1,
    OTP_MAP_CMN_ERR_UNSP = -2,
    OTP_MAP_CMN_ERR_INVAL= -3,
} otp_map_cmn_err_t;

#endif
