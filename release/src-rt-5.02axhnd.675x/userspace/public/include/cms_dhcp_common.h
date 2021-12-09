/***********************************************************************
 *
 *  Copyright (c) 2017  Broadcom Corporation
 *  All Rights Reserved
 *
 * <:label-BRCM:2017:DUAL/GPL:standard
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
:>
 *
 ************************************************************************/


/*
 * cms_dhcp_common.h
 *
 *  Created on:  Sep. 2017
 *      Author: Fuguo Xu <fuguo.xu@broadcom.com>
 */


/*
 * the defines in this file is used for both DHCPv4 and DHCPv6
 */


#ifndef __CMS_DHCP_COMMON_H__
#define __CMS_DHCP_COMMON_H__



/* -------------------------------- define A begin   -------------------------------- */

/* Note: the define A should be consistent with the one in files: 
** userspace/gpl/apps/udhcp/x.h, and userspace/public/apps/dhcpv6/dhcpv6/common.c
** For private/gpls licens concern, cannot include same file directly.
*/

#define BRCM_UDHCPC_CONFIG_DIR       "/var/udhcpc"
#define BRCM_UDHCP6C_CONFIG_DIR       "/var/udhcp6c"
#define BRCM_UDHCPC_CONFIG_FILE_BASE_NAME "option"
#define BRCM_UDHCPC_CONFIG_IN_FILE_SUFFIX ".in"
#define BRCM_UDHCPC_CONFIG_OUT_FILE_SUFFIX ".out"
#define BRCM_UDHCPC_CONFIG_FILE_NAME_LEN 64


#define VDR_MAX_DHCP_OPTION_LEN      384  /* is enough for actual DHCPv6 case??  For DHCPv4, max len is 255  */
#define VDR_MAX_DHCP_SUB_OPTION_LEN  256

/* use differenct name */
#define VDR_OPTION_CODE_OFFSET         0
#define VDR_OPTION_LEN_OFFSET          1 /* DHCPv4 */
#define VDR_OPTION_V6_LEN_OFFSET       2 /* DHCPv6 */
#define VDR_OPTION_SUBCODE_OFFSET      2 /* DHCPv4 */
#define VDR_OPTION_V6_SUBCODE_OFFSET   4 /* DHCPv6 */

/* -------------------------------- define A  end  -------------------------------- */


/* DHCP enterprise number */
#define DHCP_ENTERPRISE_NUMBER_CTL  4491   /* Cable Television Laboratories, Inc. */


typedef enum {
    OPTION_CHAR_STRING = 1,
    OPTION_HEX_STRING
} DhcpSubOptionValType;

typedef enum {
    DHCP_V4 = 1,
    DHCP_V6
} DhcpVersion;


/* DHCPv4 option codeLen=1 byte.
    DHCPv6 option codeLen=2 bytes, but some suboption codeLen=1 byte (i.e. option17->suboption35) */
typedef enum {
    OPTION_CODE_LEN1 = 1, /* 1 byte */
    OPTION_CODE_LEN2 = 2  /* 2 bytes */
} DhcpOptionCodeLen;

/* DHCPv4 option sizeLen=1 byte, DHCPv6 option sizeLen=2 bytes */
typedef enum {
    OPTION_SIZE_LEN1 = 1, /* 1 byte */
    OPTION_SIZE_LEN2 = 2  /* 2 bytes */
} DhcpOptionSizeLen;

/** valFn: callback function, to update default value
*  @param parm (IN) poninter to parm structure
*  @param string (OUT)
*  @param len (INOUT)
*
*  @return 0: success, others: fail.
*/
typedef struct {
    uint16_t subCode;  /* type for both DHCPv4 and DHCPv6*/     
    char type;  
    char *name;   
    char *valDef; /* default value */
    int (*valFn)(const void * parm, char* string, int * len);
}DhcpSubOptionTable;


/** This function make the dir for saving config files. 
 *
 * @param dhcpVer  (IN) DHCPv4 or DHCPv6.
 * @param ifName  (IN)  interface name on which dhcpc is launched.
 *
 * @return CmsRet enum.
 */
CmsRet cmsDhcp_mkCfgDir(DhcpVersion dhcpVer, const char *ifName);


/** This function read DHCP optionX string from optionX file. 
 *  optionX file example: /var/udhcpc/veip0.1/option122.out
 *
 * @param ifName  (IN)  interface name on which dhcpc is launched.
 * @param code  (IN)  option code
 * @param option  (OUT)  entire option string
 * @param len  (INOUT)  IN: option buffer(char *option) len, OUT: output option string len
 *
 * @return CmsRet enum.
 */
CmsRet cmsDhcp_readOption(DhcpVersion dhcpVer, const char *ifName, int code, char *option, int *len);


/** This function save DHCP optionX string to file, so that other application(dhcpc) can get it. 
 *  optionX file example: /var/udhcpc/veip0.1/option43.in, /var/udhcpc/veip0.1/option60.in
 *
 * @param ifName  (IN)  interface name on which dhcpc is launched.
 * @param code  (IN)  option code
 * @param option  (IN)  entire option string
 * @param len  (IN)  entire option string len
 *
 * @return CmsRet enum.
 */
CmsRet cmsDhcp_saveOption(DhcpVersion dhcpVer, const char *ifName, int code, const char *option, int len);


/** This function encapsulate DHCP sub-options into packet based on TLV(type/length/value)
 *  Note: the function will use defaule sub-option value in TLV table if callback "valFn == NULL", 
 *  otherwise, the function will call valFn to update the sub-option value.
 *
 * @param code  (IN)  option code
 * @param subOptTable  (IN)  sub option TLV table
 * @param subOptTableLen  (IN)  sub option TLV table len
 * @param generalParm  (IN)  general params passeding to callback function "valFn",
 *                                          which can be a structure for multi parms.
 * @param optData  (OUT)  encapsulated sub-option string.
 * @param dataLen  (OUT)  encapsulated sub-option string len.
 * @param codeLen  (IN)  option code len.
 * @param sizeLen  (IN)   option size len.
 *
 * @return CmsRet enum.
 */
CmsRet cmsDhcp_encapsulateSubOption(uint16_t code, DhcpSubOptionTable *subOptTable, 
                  int subOptTableLen, const void *generalParm, char* optData, int *dataLen,
                  const DhcpOptionCodeLen codeLen, const DhcpOptionSizeLen sizeLen);



/** This function parse option data TLV, then get sub-option DATA with given sub-option code
 *
 * @param dhcpVer (IN) DHCP_V4/DHCP_V6
 * @param optionData  (IN)  option data which begins with the first sub-option code.
 * @param dataLen  (IN)  option data total len
 * @param subCode  (IN)  the given sub-option code
 * @param subOptionData  (OUT)  the sub-option DATA of given sub-option code if found
 * @param subDataLen  (INOUT)  IN: sub-option data buffer len, OUT: sub-option data len
 *
 * @return CmsRet enum.
 */
CmsRet cmsDhcp_getSubOptionData(DhcpVersion dhcpVer, const char *optionData, int dataLen,
                                    int subCode, char *subOptionData, int *subDataLen);

#endif // __CMS_DHCP_COMMON_H__

