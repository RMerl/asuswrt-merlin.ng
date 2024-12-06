/*
   Copyright (c) 2015 Broadcom
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard

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


#include "XRDP_RNR_QUAD_AG.h"

/******************************************************************************
 * Register: NAME: RNR_QUAD_PARSER_CORE_CONFIGURATION_ENG, TYPE: Type_RCQ_COMMON_REGS_RCQ_PARSER_CORE_CONFIGURATION_ENG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CFG *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_ENG_CFG_FIELD =
{
    "CFG",
#if RU_INCLUDE_DESC
    "",
    "eng_cnfg[0] - IP filters on IPV6 on LSByte not MSByte\neng_cnfg[1] - Disable clock gating\neng_cnfg[2] - enable LLC_SNAP at result word\neng_cnfg[3] - defines the protocol of ppp_code_1 (1 - IPv6, 0 - IPv4) PPPOE CODE1 is IPv4 instead of IPv6\neng_cnfg[4] -  disables ip hdr length error check - 0\neng_cnfg[5] - Disable ip_ver_err check -> 1-ip versions is always 4\neng_cnfg[6] - enables checking if ip_version matches the IP version according to L2 - 0\neng_cnfg[7] - enable ICMP(next_prot=1) over IPV6\neng_cnfg[8]  -  ipv6 route with non zero segment orred with ip_hdr_len_err\neng_cnfg[9]  - enable detection of ipv6_hop_by_hop  not directly after ipv6 header\neng_cnfg[10] - Select MAC Mode for result\neng_cnfg[11] - Select IPv6 MCAST control filter FF0::/116 instead of FF02::/112\neng_cnfg[12] - Free\neng_cnfg[13] - ipv4 length error is assered also when packet is padded\neng_cnfg[14] - ipv6 length error is assered also when packet is padded\neng_cnfg[15] - enable old mode of AH at IPV6\neng_cnfg[16] - enable old mode of AH at IPV4\neng_cnfg[17] - enable L2/L3 combined key\neng_cnfg[18] - dont allow 0xFFFF as valid ipv4 header cksum results\neng_cnfg[19] - Disable IPV4_OVER_6 as qualifier for l4_fast_path_protocol\neng_cnfg[20] - Disable IPV6_OVER_4 as qualifier for l4_fast_path_protocol\neng_cnfg[21] - YL mode\neng_cnfg[22] - Enable MC BC bits at IC Key\neng_cnfg[23] - mask L2/L3 valid key logic\neng_cnfg[25:24] - udp zero detect cnfg {ipv6_en, ipv4_en}\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_ENG_CFG_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_ENG_CFG_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_ENG_CFG_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_ENG_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_ENG_CFG_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_PARSER_CORE_CONFIGURATION_ENG *****/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_ENG_REG =
{
    "PARSER_CORE_CONFIGURATION_ENG",
#if RU_INCLUDE_DESC
    "ENG Register",
    "Engineering Configuration reserved for Broadcom use\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_ENG_REG_OFFSET },
    0,
    0,
    842,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_ENG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG, TYPE: Type_RCQ_COMMON_REGS_RCQ_PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: EXCEPTION_EN *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG_EXCEPTION_EN_FIELD =
{
    "EXCEPTION_EN",
#if RU_INCLUDE_DESC
    "",
    "[0] - IP header length error\n[1] - IPv4 header checksum error\n[2] - Ethernet multicast\n[3] - ip_mcast_match - Multicast Layer 3 Identified by the following filters on IP-DA:\nIPv4: 224.0.0.0/28,\nIPv6: 0xFF00::/116 and 0xFF30/116\n[4] - ip_fragment: any fragment: first middle or last\n[5] - ip_version_err\n[6] - ip_mcast_control_match - Set when Multicast Layer 3 Control:\nIdentified IPv4 DA: IPv4 224.0.0.0/8\nIPv6: 0xFF0::\n[7] - eth_brdcst\n[8] - error: not enough bytes in the header to complete parsing of the packet\n[9] - ip_length_error\n[10]- eth_ipv4_mcast -\nmulticast Layer 2 Identified by the following DA filter:\n01:00:5e:::/23 or 33:33::::/32\n[11]- not l4 fast path prtocol\n[12]- UDP_1588_flag\n[13]- DHCP identified\n[14]- DOS attack\n[15]- GRE withe unrecgonized version (not 0 or 1) or version 1 with K bit cleared\n[16]- DNS request\n[17] -UDP Checksum Zero Detected\n[18] -TCP flag set\n\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG_EXCEPTION_EN_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG_EXCEPTION_EN_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG_EXCEPTION_EN_FIELD_SHIFT },
    15,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PROFILE_US *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG_PROFILE_US_FIELD =
{
    "PROFILE_US",
#if RU_INCLUDE_DESC
    "",
    "Profile US - Not Applicable for 63146, 4912\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG_PROFILE_US_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG_PROFILE_US_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG_PROFILE_US_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DISABLE_L2TP_SOURCE_PORT_CHECK *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG_DISABLE_L2TP_SOURCE_PORT_CHECK_FIELD =
{
    "DISABLE_L2TP_SOURCE_PORT_CHECK",
#if RU_INCLUDE_DESC
    "",
    "Disable checking source port number for L2TP identification\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG_DISABLE_L2TP_SOURCE_PORT_CHECK_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG_DISABLE_L2TP_SOURCE_PORT_CHECK_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG_DISABLE_L2TP_SOURCE_PORT_CHECK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TCP_FLAGS_FILT *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG_TCP_FLAGS_FILT_FIELD =
{
    "TCP_FLAGS_FILT",
#if RU_INCLUDE_DESC
    "",
    "Defines which TCP falgs set will cause TCP_FLAG bit in summary word to be set\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG_TCP_FLAGS_FILT_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG_TCP_FLAGS_FILT_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG_TCP_FLAGS_FILT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG_EXCEPTION_EN_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG_PROFILE_US_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG_DISABLE_L2TP_SOURCE_PORT_CHECK_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG_TCP_FLAGS_FILT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG *****/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG_REG =
{
    "PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG",
#if RU_INCLUDE_DESC
    "PARSER_MISC_CFG Register",
    "Parser Miscellaneous Configuration\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG_REG_OFFSET },
    0,
    0,
    843,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_0_1, TYPE: Type_RCQ_COMMON_REGS_RCQ_PARSER_CORE_CONFIGURATION_VID_0_1
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VID_0 *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_0_1_VID_0_FIELD =
{
    "VID_0",
#if RU_INCLUDE_DESC
    "",
    "VLAN ID Filter for first VLAN of register\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_0_1_VID_0_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_0_1_VID_0_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_0_1_VID_0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: VID_0_EN *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_0_1_VID_0_EN_FIELD =
{
    "VID_0_EN",
#if RU_INCLUDE_DESC
    "",
    "VLAND ID Filter 0 Enable\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_0_1_VID_0_EN_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_0_1_VID_0_EN_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_0_1_VID_0_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: VID_1 *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_0_1_VID_1_FIELD =
{
    "VID_1",
#if RU_INCLUDE_DESC
    "",
    "VLAN ID Filter 1 for second VLAN of register\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_0_1_VID_1_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_0_1_VID_1_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_0_1_VID_1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: VID_1_EN *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_0_1_VID_1_EN_FIELD =
{
    "VID_1_EN",
#if RU_INCLUDE_DESC
    "",
    "VLAND ID Filter 1 Enable\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_0_1_VID_1_EN_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_0_1_VID_1_EN_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_0_1_VID_1_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_0_1_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_0_1_VID_0_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_0_1_VID_0_EN_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_0_1_VID_1_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_0_1_VID_1_EN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_0_1 *****/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_0_1_REG =
{
    "PARSER_CORE_CONFIGURATION_VID_0_1",
#if RU_INCLUDE_DESC
    "VID_CONFIGURATION_0_1 Register",
    "Config VID Filter 0 & 1\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_0_1_REG_OFFSET },
    0,
    0,
    844,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_0_1_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_2_3, TYPE: Type_RCQ_COMMON_REGS_RCQ_PARSER_CORE_CONFIGURATION_VID_2_3
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VID_2 *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_2_3_VID_2_FIELD =
{
    "VID_2",
#if RU_INCLUDE_DESC
    "",
    "VLAN ID Filter for first VLAN of register\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_2_3_VID_2_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_2_3_VID_2_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_2_3_VID_2_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: VID_2_EN *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_2_3_VID_2_EN_FIELD =
{
    "VID_2_EN",
#if RU_INCLUDE_DESC
    "",
    "VLAND ID Filter 2 Enable\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_2_3_VID_2_EN_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_2_3_VID_2_EN_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_2_3_VID_2_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: VID_3 *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_2_3_VID_3_FIELD =
{
    "VID_3",
#if RU_INCLUDE_DESC
    "",
    "VLAN ID Filter 3 ofr second VLAN of register\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_2_3_VID_3_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_2_3_VID_3_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_2_3_VID_3_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: VID_3_EN *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_2_3_VID_3_EN_FIELD =
{
    "VID_3_EN",
#if RU_INCLUDE_DESC
    "",
    "VLAND ID Filter 3 Enable\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_2_3_VID_3_EN_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_2_3_VID_3_EN_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_2_3_VID_3_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_2_3_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_2_3_VID_2_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_2_3_VID_2_EN_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_2_3_VID_3_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_2_3_VID_3_EN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_2_3 *****/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_2_3_REG =
{
    "PARSER_CORE_CONFIGURATION_VID_2_3",
#if RU_INCLUDE_DESC
    "VID_CONFIGURATION_2_3 Register",
    "Config VID Filter 2 & 3\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_2_3_REG_OFFSET },
    0,
    0,
    845,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_2_3_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_4_5, TYPE: Type_RCQ_COMMON_REGS_RCQ_PARSER_CORE_CONFIGURATION_VID_4_5
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VID_4 *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_4_5_VID_4_FIELD =
{
    "VID_4",
#if RU_INCLUDE_DESC
    "",
    "VLAN ID Filter for first VLAN of register\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_4_5_VID_4_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_4_5_VID_4_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_4_5_VID_4_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: VID_4_EN *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_4_5_VID_4_EN_FIELD =
{
    "VID_4_EN",
#if RU_INCLUDE_DESC
    "",
    "VLAND ID Filter 4 Enable\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_4_5_VID_4_EN_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_4_5_VID_4_EN_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_4_5_VID_4_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: VID_5 *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_4_5_VID_5_FIELD =
{
    "VID_5",
#if RU_INCLUDE_DESC
    "",
    "VLAN ID Filter 5 ofr second VLAN of register\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_4_5_VID_5_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_4_5_VID_5_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_4_5_VID_5_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: VID_5_EN *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_4_5_VID_5_EN_FIELD =
{
    "VID_5_EN",
#if RU_INCLUDE_DESC
    "",
    "VLAND ID Filter 5 Enable\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_4_5_VID_5_EN_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_4_5_VID_5_EN_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_4_5_VID_5_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_4_5_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_4_5_VID_4_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_4_5_VID_4_EN_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_4_5_VID_5_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_4_5_VID_5_EN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_4_5 *****/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_4_5_REG =
{
    "PARSER_CORE_CONFIGURATION_VID_4_5",
#if RU_INCLUDE_DESC
    "VID_CONFIGURATION_4_5 Register",
    "Config VID Filter 4 & 5\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_4_5_REG_OFFSET },
    0,
    0,
    846,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_4_5_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_6_7, TYPE: Type_RCQ_COMMON_REGS_RCQ_PARSER_CORE_CONFIGURATION_VID_6_7
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VID_6 *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_6_7_VID_6_FIELD =
{
    "VID_6",
#if RU_INCLUDE_DESC
    "",
    "VLAN ID Filter for first VLAN of register\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_6_7_VID_6_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_6_7_VID_6_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_6_7_VID_6_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: VID_6_EN *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_6_7_VID_6_EN_FIELD =
{
    "VID_6_EN",
#if RU_INCLUDE_DESC
    "",
    "VLAND ID Filter 6 Enable\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_6_7_VID_6_EN_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_6_7_VID_6_EN_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_6_7_VID_6_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: VID_7 *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_6_7_VID_7_FIELD =
{
    "VID_7",
#if RU_INCLUDE_DESC
    "",
    "VLAN ID Filter 7 ofr second VLAN of register\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_6_7_VID_7_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_6_7_VID_7_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_6_7_VID_7_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: VID_7_EN *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_6_7_VID_7_EN_FIELD =
{
    "VID_7_EN",
#if RU_INCLUDE_DESC
    "",
    "VLAND ID Filter 7 Enable\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_6_7_VID_7_EN_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_6_7_VID_7_EN_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_6_7_VID_7_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_6_7_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_6_7_VID_6_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_6_7_VID_6_EN_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_6_7_VID_7_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_6_7_VID_7_EN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_6_7 *****/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_6_7_REG =
{
    "PARSER_CORE_CONFIGURATION_VID_6_7",
#if RU_INCLUDE_DESC
    "VID_CONFIGURATION_6_7 Register",
    "Config VID Filter 6 & 7\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_6_7_REG_OFFSET },
    0,
    0,
    847,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_6_7_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER0_CFG, TYPE: Type_RCQ_COMMON_REGS_RCQ_PARSER_CORE_CONFIGURATION_IP_FILTER0_CFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: IP_ADDRESS *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER0_CFG_IP_ADDRESS_FIELD =
{
    "IP_ADDRESS",
#if RU_INCLUDE_DESC
    "",
    "32-bit address to match SIP or DIP (according to predefined configuration in IP_FILTERS_CFG register)\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER0_CFG_IP_ADDRESS_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER0_CFG_IP_ADDRESS_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER0_CFG_IP_ADDRESS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER0_CFG_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER0_CFG_IP_ADDRESS_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER0_CFG *****/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER0_CFG_REG =
{
    "PARSER_CORE_CONFIGURATION_IP_FILTER0_CFG",
#if RU_INCLUDE_DESC
    "IP_FILTER0_CFG Register",
    "Config the IP Address filtering.\nNotice that the enable bit is located in the IP_FILTERS_CFG[4]\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER0_CFG_REG_OFFSET },
    0,
    0,
    848,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER0_CFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER1_CFG, TYPE: Type_RCQ_COMMON_REGS_RCQ_PARSER_CORE_CONFIGURATION_IP_FILTER1_CFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: IP_ADDRESS *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER1_CFG_IP_ADDRESS_FIELD =
{
    "IP_ADDRESS",
#if RU_INCLUDE_DESC
    "",
    "32-bit address to match SIP or DIP (according to predefined configuration in IP_FILTERS_CFG register)\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER1_CFG_IP_ADDRESS_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER1_CFG_IP_ADDRESS_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER1_CFG_IP_ADDRESS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER1_CFG_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER1_CFG_IP_ADDRESS_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER1_CFG *****/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER1_CFG_REG =
{
    "PARSER_CORE_CONFIGURATION_IP_FILTER1_CFG",
#if RU_INCLUDE_DESC
    "IP_FILTER1_CFG Register",
    "Config the IP Address filtering.\nNotice that the enable bit is located in the IP_FILTERS_CFG[5]\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER1_CFG_REG_OFFSET },
    0,
    0,
    849,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER1_CFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER0_MASK_CFG, TYPE: Type_RCQ_COMMON_REGS_RCQ_PARSER_CORE_CONFIGURATION_IP_FILTER0_MASK_CFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: IP_ADDRESS_MASK *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER0_MASK_CFG_IP_ADDRESS_MASK_FIELD =
{
    "IP_ADDRESS_MASK",
#if RU_INCLUDE_DESC
    "",
    "32-bit address mask\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER0_MASK_CFG_IP_ADDRESS_MASK_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER0_MASK_CFG_IP_ADDRESS_MASK_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER0_MASK_CFG_IP_ADDRESS_MASK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER0_MASK_CFG_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER0_MASK_CFG_IP_ADDRESS_MASK_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER0_MASK_CFG *****/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER0_MASK_CFG_REG =
{
    "PARSER_CORE_CONFIGURATION_IP_FILTER0_MASK_CFG",
#if RU_INCLUDE_DESC
    "IP_FILTER0_MASK_CFG Register",
    "Config the IP Address masking.\nNotice that the enable bit is located in the IP_FILTERS_CFG[4]\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER0_MASK_CFG_REG_OFFSET },
    0,
    0,
    850,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER0_MASK_CFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER1_MASK_CFG, TYPE: Type_RCQ_COMMON_REGS_RCQ_PARSER_CORE_CONFIGURATION_IP_FILTER1_MASK_CFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: IP_ADDRESS_MASK *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER1_MASK_CFG_IP_ADDRESS_MASK_FIELD =
{
    "IP_ADDRESS_MASK",
#if RU_INCLUDE_DESC
    "",
    "32-bit address mask\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER1_MASK_CFG_IP_ADDRESS_MASK_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER1_MASK_CFG_IP_ADDRESS_MASK_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER1_MASK_CFG_IP_ADDRESS_MASK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER1_MASK_CFG_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER1_MASK_CFG_IP_ADDRESS_MASK_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER1_MASK_CFG *****/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER1_MASK_CFG_REG =
{
    "PARSER_CORE_CONFIGURATION_IP_FILTER1_MASK_CFG",
#if RU_INCLUDE_DESC
    "IP_FILTER1_MASK_CFG Register",
    "Config the IP Address masking.\nNotice that the enable bit is located in the IP_FILTERS_CFG[5]\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER1_MASK_CFG_REG_OFFSET },
    0,
    0,
    851,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER1_MASK_CFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG, TYPE: Type_RCQ_COMMON_REGS_RCQ_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: IP_FILTER0_DIP_EN *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_IP_FILTER0_DIP_EN_FIELD =
{
    "IP_FILTER0_DIP_EN",
#if RU_INCLUDE_DESC
    "",
    "IP Filter0 DIP or SIP selection.\nThe default is SIP, when the field is set -> DIP selection is enabled\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_IP_FILTER0_DIP_EN_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_IP_FILTER0_DIP_EN_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_IP_FILTER0_DIP_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: IP_FILTER1_DIP_EN *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_IP_FILTER1_DIP_EN_FIELD =
{
    "IP_FILTER1_DIP_EN",
#if RU_INCLUDE_DESC
    "",
    "IP Filter1 DIP or SIP selection.\nThe default is SIP, when the field is set -> DIP selection is enabled\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_IP_FILTER1_DIP_EN_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_IP_FILTER1_DIP_EN_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_IP_FILTER1_DIP_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: IP_FILTER2_DIP_EN *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_IP_FILTER2_DIP_EN_FIELD =
{
    "IP_FILTER2_DIP_EN",
#if RU_INCLUDE_DESC
    "",
    "IP Filter2 DIP or SIP selection.\nThe default is SIP, when the field is set -> DIP selection is enabled\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_IP_FILTER2_DIP_EN_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_IP_FILTER2_DIP_EN_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_IP_FILTER2_DIP_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: IP_FILTER3_DIP_EN *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_IP_FILTER3_DIP_EN_FIELD =
{
    "IP_FILTER3_DIP_EN",
#if RU_INCLUDE_DESC
    "",
    "IP Filter3 DIP or SIP selection.\nThe default is SIP, when the field is set -> DIP selection is enabled\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_IP_FILTER3_DIP_EN_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_IP_FILTER3_DIP_EN_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_IP_FILTER3_DIP_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: IP_FILTER0_VALID *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_IP_FILTER0_VALID_FIELD =
{
    "IP_FILTER0_VALID",
#if RU_INCLUDE_DESC
    "",
    "IP Filter0 valid bit.\n\nWhen the bit valid is set, the IP filter/mask can be applied by hardware.\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_IP_FILTER0_VALID_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_IP_FILTER0_VALID_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_IP_FILTER0_VALID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: IP_FILTER1_VALID *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_IP_FILTER1_VALID_FIELD =
{
    "IP_FILTER1_VALID",
#if RU_INCLUDE_DESC
    "",
    "IP Filter1 valid bit.\n\nWhen the bit valid is set, the IP filter/mask can be applied by hardware.\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_IP_FILTER1_VALID_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_IP_FILTER1_VALID_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_IP_FILTER1_VALID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: IP_FILTER2_VALID *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_IP_FILTER2_VALID_FIELD =
{
    "IP_FILTER2_VALID",
#if RU_INCLUDE_DESC
    "",
    "IP Filter2 valid bit.\n\nWhen the bit valid is set, the IP filter/mask can be applied by hardware.\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_IP_FILTER2_VALID_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_IP_FILTER2_VALID_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_IP_FILTER2_VALID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: IP_FILTER3_VALID *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_IP_FILTER3_VALID_FIELD =
{
    "IP_FILTER3_VALID",
#if RU_INCLUDE_DESC
    "",
    "IP Filter3 valid bit.\n\nWhen the bit valid is set, the IP filter/mask can be applied by hardware.\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_IP_FILTER3_VALID_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_IP_FILTER3_VALID_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_IP_FILTER3_VALID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_IP_FILTER0_DIP_EN_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_IP_FILTER1_DIP_EN_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_IP_FILTER2_DIP_EN_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_IP_FILTER3_DIP_EN_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_IP_FILTER0_VALID_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_IP_FILTER1_VALID_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_IP_FILTER2_VALID_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_IP_FILTER3_VALID_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG *****/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_REG =
{
    "PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG",
#if RU_INCLUDE_DESC
    "IP_FILTERS_CFG Register",
    "IP Address Filters (0..3) configurations:\n\n(1) SIP or DIP selection config per each filter\n(1) Valid bit per each filter\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_REG_OFFSET },
    0,
    0,
    852,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_PARSER_CORE_CONFIGURATION_SNAP_ORG_CODE, TYPE: Type_RCQ_COMMON_REGS_RCQ_PARSER_CORE_CONFIGURATION_SNAP_ORG_CODE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CODE *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_SNAP_ORG_CODE_CODE_FIELD =
{
    "CODE",
#if RU_INCLUDE_DESC
    "",
    "Used defined SNAP organization code\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_SNAP_ORG_CODE_CODE_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_SNAP_ORG_CODE_CODE_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_SNAP_ORG_CODE_CODE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EN_RFC1042 *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_SNAP_ORG_CODE_EN_RFC1042_FIELD =
{
    "EN_RFC1042",
#if RU_INCLUDE_DESC
    "",
    "enable RFC1042 0x00000 organization code\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_SNAP_ORG_CODE_EN_RFC1042_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_SNAP_ORG_CODE_EN_RFC1042_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_SNAP_ORG_CODE_EN_RFC1042_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EN_8021Q *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_SNAP_ORG_CODE_EN_8021Q_FIELD =
{
    "EN_8021Q",
#if RU_INCLUDE_DESC
    "",
    "enables 802.1Q 0x0000f8 organization code\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_SNAP_ORG_CODE_EN_8021Q_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_SNAP_ORG_CODE_EN_8021Q_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_SNAP_ORG_CODE_EN_8021Q_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_SNAP_ORG_CODE_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_SNAP_ORG_CODE_CODE_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_SNAP_ORG_CODE_EN_RFC1042_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_SNAP_ORG_CODE_EN_8021Q_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_PARSER_CORE_CONFIGURATION_SNAP_ORG_CODE *****/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_SNAP_ORG_CODE_REG =
{
    "PARSER_CORE_CONFIGURATION_SNAP_ORG_CODE",
#if RU_INCLUDE_DESC
    "SNAP_ORGANIZATION_CODE Register",
    "Identifies SNAP tunneling organization code\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_SNAP_ORG_CODE_REG_OFFSET },
    0,
    0,
    853,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_SNAP_ORG_CODE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_PARSER_CORE_CONFIGURATION_PPP_IP_PROT_CODE, TYPE: Type_RCQ_COMMON_REGS_RCQ_PARSER_CORE_CONFIGURATION_PPP_IP_PROT_CODE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PPP_CODE_0 *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_PPP_IP_PROT_CODE_PPP_CODE_0_FIELD =
{
    "PPP_CODE_0",
#if RU_INCLUDE_DESC
    "",
    "PPP Protocol code to identify L3 is IP\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_PPP_IP_PROT_CODE_PPP_CODE_0_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_PPP_IP_PROT_CODE_PPP_CODE_0_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_PPP_IP_PROT_CODE_PPP_CODE_0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PPP_CODE_1 *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_PPP_IP_PROT_CODE_PPP_CODE_1_FIELD =
{
    "PPP_CODE_1",
#if RU_INCLUDE_DESC
    "",
    "PPP Protocol code to identify L3 is IP\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_PPP_IP_PROT_CODE_PPP_CODE_1_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_PPP_IP_PROT_CODE_PPP_CODE_1_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_PPP_IP_PROT_CODE_PPP_CODE_1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_PPP_IP_PROT_CODE_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_PPP_IP_PROT_CODE_PPP_CODE_0_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_PPP_IP_PROT_CODE_PPP_CODE_1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_PARSER_CORE_CONFIGURATION_PPP_IP_PROT_CODE *****/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_PPP_IP_PROT_CODE_REG =
{
    "PARSER_CORE_CONFIGURATION_PPP_IP_PROT_CODE",
#if RU_INCLUDE_DESC
    "PPP_IP_PROTOCOL_CODE Register",
    "PPP Protocol Code to indicate L3 is IP\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_PPP_IP_PROT_CODE_REG_OFFSET },
    0,
    0,
    854,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_PPP_IP_PROT_CODE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_ETHTYPE, TYPE: Type_RCQ_COMMON_REGS_RCQ_PARSER_CORE_CONFIGURATION_QTAG_ETHTYPE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ETHTYPE_QTAG_0 *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_ETHTYPE_ETHTYPE_QTAG_0_FIELD =
{
    "ETHTYPE_QTAG_0",
#if RU_INCLUDE_DESC
    "",
    "Ethertype to identify VLAN QTAG\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_ETHTYPE_ETHTYPE_QTAG_0_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_ETHTYPE_ETHTYPE_QTAG_0_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_ETHTYPE_ETHTYPE_QTAG_0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ETHTYPE_QTAG_1 *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_ETHTYPE_ETHTYPE_QTAG_1_FIELD =
{
    "ETHTYPE_QTAG_1",
#if RU_INCLUDE_DESC
    "",
    "Ethertype to identify VLAN QTAG\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_ETHTYPE_ETHTYPE_QTAG_1_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_ETHTYPE_ETHTYPE_QTAG_1_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_ETHTYPE_ETHTYPE_QTAG_1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_ETHTYPE_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_ETHTYPE_ETHTYPE_QTAG_0_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_ETHTYPE_ETHTYPE_QTAG_1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_ETHTYPE *****/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_ETHTYPE_REG =
{
    "PARSER_CORE_CONFIGURATION_QTAG_ETHTYPE",
#if RU_INCLUDE_DESC
    "QTAG_ETHERTYPE Register",
    "Ethertype values to identify the presence of VLAN QTAG\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_ETHTYPE_REG_OFFSET },
    0,
    0,
    855,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_ETHTYPE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_0_1, TYPE: Type_RCQ_COMMON_REGS_RCQ_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_0_1
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ETHYPE_0 *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_0_1_ETHYPE_0_FIELD =
{
    "ETHYPE_0",
#if RU_INCLUDE_DESC
    "",
    "User Ethertype 0\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_0_1_ETHYPE_0_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_0_1_ETHYPE_0_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_0_1_ETHYPE_0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ETHYPE_1 *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_0_1_ETHYPE_1_FIELD =
{
    "ETHYPE_1",
#if RU_INCLUDE_DESC
    "",
    "User Ethertype 1\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_0_1_ETHYPE_1_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_0_1_ETHYPE_1_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_0_1_ETHYPE_1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_0_1_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_0_1_ETHYPE_0_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_0_1_ETHYPE_1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_0_1 *****/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_0_1_REG =
{
    "PARSER_CORE_CONFIGURATION_USER_ETHTYPE_0_1",
#if RU_INCLUDE_DESC
    "USER_ETHERTYPE_CONFIGURTION_0_1 Register",
    "Configures user Ethertype values\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_0_1_REG_OFFSET },
    0,
    0,
    856,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_0_1_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_2_3, TYPE: Type_RCQ_COMMON_REGS_RCQ_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_2_3
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ETHYPE_2 *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_2_3_ETHYPE_2_FIELD =
{
    "ETHYPE_2",
#if RU_INCLUDE_DESC
    "",
    "User Ethertype 2\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_2_3_ETHYPE_2_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_2_3_ETHYPE_2_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_2_3_ETHYPE_2_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ETHYPE_3 *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_2_3_ETHYPE_3_FIELD =
{
    "ETHYPE_3",
#if RU_INCLUDE_DESC
    "",
    "User Ethertype 3\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_2_3_ETHYPE_3_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_2_3_ETHYPE_3_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_2_3_ETHYPE_3_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_2_3_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_2_3_ETHYPE_2_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_2_3_ETHYPE_3_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_2_3 *****/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_2_3_REG =
{
    "PARSER_CORE_CONFIGURATION_USER_ETHTYPE_2_3",
#if RU_INCLUDE_DESC
    "USER_ETHERTYPE_CONFIGURTION_2_3 Register",
    "Configures user Ethertype values\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_2_3_REG_OFFSET },
    0,
    0,
    857,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_2_3_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG, TYPE: Type_RCQ_COMMON_REGS_RCQ_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ETHTYPE_USER_PROT_0 *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_PROT_0_FIELD =
{
    "ETHTYPE_USER_PROT_0",
#if RU_INCLUDE_DESC
    "",
    "Pointer to L3 protocol for User Ethertype 0 (0 - None, 1-IPv4, 2-IPv6)\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_PROT_0_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_PROT_0_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_PROT_0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ETHTYPE_USER_PROT_1 *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_PROT_1_FIELD =
{
    "ETHTYPE_USER_PROT_1",
#if RU_INCLUDE_DESC
    "",
    "Pointer to L3 protocol for User Ethertype 1 (0 - None, 1-IPv4, 2-IPv6)\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_PROT_1_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_PROT_1_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_PROT_1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ETHTYPE_USER_PROT_2 *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_PROT_2_FIELD =
{
    "ETHTYPE_USER_PROT_2",
#if RU_INCLUDE_DESC
    "",
    "Pointer to L3 protocol for User Ethertype 2 (0 - None, 1-IPv4, 2-IPv6)\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_PROT_2_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_PROT_2_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_PROT_2_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ETHTYPE_USER_PROT_3 *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_PROT_3_FIELD =
{
    "ETHTYPE_USER_PROT_3",
#if RU_INCLUDE_DESC
    "",
    "Pointer to L3 protocol for User Ethertype 3 (0 - None, 1-IPv4, 2-IPv6)\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_PROT_3_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_PROT_3_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_PROT_3_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ETHTYPE_USER_EN *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_EN_FIELD =
{
    "ETHTYPE_USER_EN",
#if RU_INCLUDE_DESC
    "",
    "Enable user Ethertype 3-0 (LSB is for user ethertype 0)\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_EN_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_EN_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ETHTYPE_USER_OFFSET_0 *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_OFFSET_0_FIELD =
{
    "ETHTYPE_USER_OFFSET_0",
#if RU_INCLUDE_DESC
    "",
    "4 byte offset for User Ethertype 0 L3\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_OFFSET_0_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_OFFSET_0_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_OFFSET_0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ETHTYPE_USER_OFFSET_1 *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_OFFSET_1_FIELD =
{
    "ETHTYPE_USER_OFFSET_1",
#if RU_INCLUDE_DESC
    "",
    "4 byte offset for User Ethertype 1 L3\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_OFFSET_1_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_OFFSET_1_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_OFFSET_1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ETHTYPE_USER_OFFSET_2 *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_OFFSET_2_FIELD =
{
    "ETHTYPE_USER_OFFSET_2",
#if RU_INCLUDE_DESC
    "",
    "4 byte offset for User Ethertype 2 L3\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_OFFSET_2_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_OFFSET_2_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_OFFSET_2_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ETHTYPE_USER_OFFSET_3 *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_OFFSET_3_FIELD =
{
    "ETHTYPE_USER_OFFSET_3",
#if RU_INCLUDE_DESC
    "",
    "4 byte offset for User Ethertype 3 L3\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_OFFSET_3_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_OFFSET_3_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_OFFSET_3_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_PROT_0_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_PROT_1_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_PROT_2_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_PROT_3_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_EN_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_OFFSET_0_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_OFFSET_1_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_OFFSET_2_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_OFFSET_3_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG *****/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_REG =
{
    "PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG",
#if RU_INCLUDE_DESC
    "USER_ETHERTYPE_CONFIGURATION Register",
    "Configure protocol and enables user Ethertype\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_REG_OFFSET },
    0,
    0,
    858,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_PARSER_CORE_CONFIGURATION_IPV6_HDR_EXT_FLTR_MASK_CFG, TYPE: Type_RCQ_COMMON_REGS_RCQ_PARSER_CORE_CONFIGURATION_IPV6_HDR_EXT_FLTR_MASK_CFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: HOP_BY_HOP_MATCH *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_IPV6_HDR_EXT_FLTR_MASK_CFG_HOP_BY_HOP_MATCH_FIELD =
{
    "HOP_BY_HOP_MATCH",
#if RU_INCLUDE_DESC
    "",
    "hop by hop match filter mask\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_IPV6_HDR_EXT_FLTR_MASK_CFG_HOP_BY_HOP_MATCH_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_IPV6_HDR_EXT_FLTR_MASK_CFG_HOP_BY_HOP_MATCH_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_IPV6_HDR_EXT_FLTR_MASK_CFG_HOP_BY_HOP_MATCH_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ROUTING_EH *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_IPV6_HDR_EXT_FLTR_MASK_CFG_ROUTING_EH_FIELD =
{
    "ROUTING_EH",
#if RU_INCLUDE_DESC
    "",
    "Routing extension header option match filter mask\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_IPV6_HDR_EXT_FLTR_MASK_CFG_ROUTING_EH_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_IPV6_HDR_EXT_FLTR_MASK_CFG_ROUTING_EH_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_IPV6_HDR_EXT_FLTR_MASK_CFG_ROUTING_EH_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DEST_OPT_EH *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_IPV6_HDR_EXT_FLTR_MASK_CFG_DEST_OPT_EH_FIELD =
{
    "DEST_OPT_EH",
#if RU_INCLUDE_DESC
    "",
    "Destination Options extension header option match filter mask\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_IPV6_HDR_EXT_FLTR_MASK_CFG_DEST_OPT_EH_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_IPV6_HDR_EXT_FLTR_MASK_CFG_DEST_OPT_EH_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_IPV6_HDR_EXT_FLTR_MASK_CFG_DEST_OPT_EH_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: AH_MATCH *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_IPV6_HDR_EXT_FLTR_MASK_CFG_AH_MATCH_FIELD =
{
    "AH_MATCH",
#if RU_INCLUDE_DESC
    "",
    "Destination Options extension header option match filter mask\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_IPV6_HDR_EXT_FLTR_MASK_CFG_AH_MATCH_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_IPV6_HDR_EXT_FLTR_MASK_CFG_AH_MATCH_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_IPV6_HDR_EXT_FLTR_MASK_CFG_AH_MATCH_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_IPV6_HDR_EXT_FLTR_MASK_CFG_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_IPV6_HDR_EXT_FLTR_MASK_CFG_HOP_BY_HOP_MATCH_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_IPV6_HDR_EXT_FLTR_MASK_CFG_ROUTING_EH_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_IPV6_HDR_EXT_FLTR_MASK_CFG_DEST_OPT_EH_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_IPV6_HDR_EXT_FLTR_MASK_CFG_AH_MATCH_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_PARSER_CORE_CONFIGURATION_IPV6_HDR_EXT_FLTR_MASK_CFG *****/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_IPV6_HDR_EXT_FLTR_MASK_CFG_REG =
{
    "PARSER_CORE_CONFIGURATION_IPV6_HDR_EXT_FLTR_MASK_CFG",
#if RU_INCLUDE_DESC
    "IPV6_HDR_EXT_FLTR_MASK_CFG Register",
    "IPV6 Header Extension Filter Mask register\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_IPV6_HDR_EXT_FLTR_MASK_CFG_REG_OFFSET },
    0,
    0,
    859,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_IPV6_HDR_EXT_FLTR_MASK_CFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST, TYPE: Type_RCQ_COMMON_REGS_RCQ_PARSER_CORE_CONFIGURATION_QTAG_NEST
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: QTAG_NEST_0_PROFILE_0 *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_QTAG_NEST_0_PROFILE_0_FIELD =
{
    "QTAG_NEST_0_PROFILE_0",
#if RU_INCLUDE_DESC
    "",
    "Set to enable Ethertype_qTag 0 as outer (LSB)  2nd VLAN (2nd), 3rd VLAN (MSB)\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_QTAG_NEST_0_PROFILE_0_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_QTAG_NEST_0_PROFILE_0_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_QTAG_NEST_0_PROFILE_0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: QTAG_NEST_0_PROFILE_1 *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_QTAG_NEST_0_PROFILE_1_FIELD =
{
    "QTAG_NEST_0_PROFILE_1",
#if RU_INCLUDE_DESC
    "",
    "Set to enable Ethertype_qTag 0 as outer (LSB)  2nd VLAN (2nd), 3rd VLAN (MSB)\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_QTAG_NEST_0_PROFILE_1_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_QTAG_NEST_0_PROFILE_1_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_QTAG_NEST_0_PROFILE_1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: QTAG_NEST_0_PROFILE_2 *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_QTAG_NEST_0_PROFILE_2_FIELD =
{
    "QTAG_NEST_0_PROFILE_2",
#if RU_INCLUDE_DESC
    "",
    "Set to enable Ethertype_qTag 0 as outer (LSB)  2nd VLAN (2nd), 3rd VLAN (MSB)\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_QTAG_NEST_0_PROFILE_2_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_QTAG_NEST_0_PROFILE_2_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_QTAG_NEST_0_PROFILE_2_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: QTAG_NEST_1_PROFILE_0 *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_QTAG_NEST_1_PROFILE_0_FIELD =
{
    "QTAG_NEST_1_PROFILE_0",
#if RU_INCLUDE_DESC
    "",
    "Set to enable Ethertype_qTag 0 as outer (LSB)  2nd VLAN (2nd), 3rd VLAN (MSB)\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_QTAG_NEST_1_PROFILE_0_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_QTAG_NEST_1_PROFILE_0_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_QTAG_NEST_1_PROFILE_0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: QTAG_NEST_1_PROFILE_1 *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_QTAG_NEST_1_PROFILE_1_FIELD =
{
    "QTAG_NEST_1_PROFILE_1",
#if RU_INCLUDE_DESC
    "",
    "Set to enable Ethertype_qTag 0 as outer (LSB)  2nd VLAN (2nd), 3rd VLAN (MSB)\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_QTAG_NEST_1_PROFILE_1_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_QTAG_NEST_1_PROFILE_1_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_QTAG_NEST_1_PROFILE_1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: QTAG_NEST_1_PROFILE_2 *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_QTAG_NEST_1_PROFILE_2_FIELD =
{
    "QTAG_NEST_1_PROFILE_2",
#if RU_INCLUDE_DESC
    "",
    "Set to enable Ethertype_qTag 0 as outer (LSB)  2nd VLAN (2nd), 3rd VLAN (MSB)\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_QTAG_NEST_1_PROFILE_2_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_QTAG_NEST_1_PROFILE_2_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_QTAG_NEST_1_PROFILE_2_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MAX_NUM_OF_VLANS *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_MAX_NUM_OF_VLANS_FIELD =
{
    "MAX_NUM_OF_VLANS",
#if RU_INCLUDE_DESC
    "",
    "Max number of VLAN tags allowed in the packet.\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_MAX_NUM_OF_VLANS_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_MAX_NUM_OF_VLANS_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_MAX_NUM_OF_VLANS_FIELD_SHIFT },
    3,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_QTAG_NEST_0_PROFILE_0_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_QTAG_NEST_0_PROFILE_1_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_QTAG_NEST_0_PROFILE_2_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_QTAG_NEST_1_PROFILE_0_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_QTAG_NEST_1_PROFILE_1_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_QTAG_NEST_1_PROFILE_2_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_MAX_NUM_OF_VLANS_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST *****/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_REG =
{
    "PARSER_CORE_CONFIGURATION_QTAG_NEST",
#if RU_INCLUDE_DESC
    "QTAG_NESTING Register",
    "Qtag Nesting config\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_REG_OFFSET },
    0,
    0,
    860,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    7,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_0, TYPE: Type_RCQ_COMMON_REGS_RCQ_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_0
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: HARD_NEST_PROFILE *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_0_HARD_NEST_PROFILE_FIELD =
{
    "HARD_NEST_PROFILE",
#if RU_INCLUDE_DESC
    "",
    "bit 2-0: Enable 8100 as VLAN for outer, 2nd, and inner VLANs (inner is bit 2).\nbit 5-3: Enable 88a8 as VLAN for outer, 2nd, and inner VLANs.\nbit 8-6: Enable 9100 as VLAN for outer, 2nd, and inner VLANs.\nbit 11-9: Enable 9200 as VLAN for outer, 2nd, and inner VLANs.\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_0_HARD_NEST_PROFILE_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_0_HARD_NEST_PROFILE_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_0_HARD_NEST_PROFILE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_0_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_0_HARD_NEST_PROFILE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_0 *****/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_0_REG =
{
    "PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_0",
#if RU_INCLUDE_DESC
    "QTAG_HARD_NEST_PROFILE_0 Register",
    "QTAG Hard Nest Profile 0\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_0_REG_OFFSET },
    0,
    0,
    861,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_0_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_1, TYPE: Type_RCQ_COMMON_REGS_RCQ_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_1
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: HARD_NEST_PROFILE *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_1_HARD_NEST_PROFILE_FIELD =
{
    "HARD_NEST_PROFILE",
#if RU_INCLUDE_DESC
    "",
    "Hard Nest Profile\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_1_HARD_NEST_PROFILE_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_1_HARD_NEST_PROFILE_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_1_HARD_NEST_PROFILE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_1_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_1_HARD_NEST_PROFILE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_1 *****/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_1_REG =
{
    "PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_1",
#if RU_INCLUDE_DESC
    "QTAG_HARD_NEST_PROFILE_1 Register",
    "QTAG Hard Nest Profile 1\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_1_REG_OFFSET },
    0,
    0,
    862,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_1_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_2, TYPE: Type_RCQ_COMMON_REGS_RCQ_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_2
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: HARD_NEST_PROFILE *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_2_HARD_NEST_PROFILE_FIELD =
{
    "HARD_NEST_PROFILE",
#if RU_INCLUDE_DESC
    "",
    "Hard Nest Profile\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_2_HARD_NEST_PROFILE_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_2_HARD_NEST_PROFILE_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_2_HARD_NEST_PROFILE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_2_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_2_HARD_NEST_PROFILE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_2 *****/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_2_REG =
{
    "PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_2",
#if RU_INCLUDE_DESC
    "QTAG_HARD_NEST_PROFILE_2 Register",
    "QTAG Hard Nest Profile 2\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_2_REG_OFFSET },
    0,
    0,
    863,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_2_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_IP_PROT, TYPE: Type_RCQ_COMMON_REGS_RCQ_PARSER_CORE_CONFIGURATION_USER_IP_PROT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: USER_IP_PROT_0 *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_IP_PROT_USER_IP_PROT_0_FIELD =
{
    "USER_IP_PROT_0",
#if RU_INCLUDE_DESC
    "",
    "User defined IP protocol 0 (value to be matched to IP protocol field)\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_IP_PROT_USER_IP_PROT_0_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_IP_PROT_USER_IP_PROT_0_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_IP_PROT_USER_IP_PROT_0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: USER_IP_PROT_1 *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_IP_PROT_USER_IP_PROT_1_FIELD =
{
    "USER_IP_PROT_1",
#if RU_INCLUDE_DESC
    "",
    "User defined IP protocol 1 (value to be matched to IP protocol field)\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_IP_PROT_USER_IP_PROT_1_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_IP_PROT_USER_IP_PROT_1_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_IP_PROT_USER_IP_PROT_1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: USER_IP_PROT_2 *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_IP_PROT_USER_IP_PROT_2_FIELD =
{
    "USER_IP_PROT_2",
#if RU_INCLUDE_DESC
    "",
    "User defined IP protocol 2 (value to be matched to IP protocol field)\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_IP_PROT_USER_IP_PROT_2_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_IP_PROT_USER_IP_PROT_2_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_IP_PROT_USER_IP_PROT_2_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: USER_IP_PROT_3 *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_IP_PROT_USER_IP_PROT_3_FIELD =
{
    "USER_IP_PROT_3",
#if RU_INCLUDE_DESC
    "",
    "User defined IP protocol 3 (value to be matched to IP protocol field)\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_IP_PROT_USER_IP_PROT_3_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_IP_PROT_USER_IP_PROT_3_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_IP_PROT_USER_IP_PROT_3_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_IP_PROT_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_IP_PROT_USER_IP_PROT_0_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_IP_PROT_USER_IP_PROT_1_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_IP_PROT_USER_IP_PROT_2_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_IP_PROT_USER_IP_PROT_3_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_IP_PROT *****/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_IP_PROT_REG =
{
    "PARSER_CORE_CONFIGURATION_USER_IP_PROT",
#if RU_INCLUDE_DESC
    "USER_DEFINED_IP_PROTOCL Register",
    "IP Protocols to be matched to IP Protocol field and to be indicated in the output summary word\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_IP_PROT_REG_OFFSET },
    0,
    0,
    864,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_IP_PROT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_L, TYPE: Type_RCQ_COMMON_REGS_RCQ_PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_L
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DA_FILT_LSB *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_L_DA_FILT_LSB_FIELD =
{
    "DA_FILT_LSB",
#if RU_INCLUDE_DESC
    "",
    "DA Filter bits 31:0\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_L_DA_FILT_LSB_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_L_DA_FILT_LSB_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_L_DA_FILT_LSB_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_L_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_L_DA_FILT_LSB_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_L *****/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_L_REG =
{
    "PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_L",
#if RU_INCLUDE_DESC
    "DA_FILT0_VAL_L Register",
    "Config DA filter 31:0\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_L_REG_OFFSET },
    0,
    0,
    865,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_L_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_H, TYPE: Type_RCQ_COMMON_REGS_RCQ_PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_H
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DA_FILT_MSB *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_H_DA_FILT_MSB_FIELD =
{
    "DA_FILT_MSB",
#if RU_INCLUDE_DESC
    "",
    "Current DA Filter bits 47:32\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_H_DA_FILT_MSB_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_H_DA_FILT_MSB_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_H_DA_FILT_MSB_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_H_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_H_DA_FILT_MSB_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_H *****/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_H_REG =
{
    "PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_H",
#if RU_INCLUDE_DESC
    "DA_FILT0_VAL_H Register",
    "Config DA filter0 47:32\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_H_REG_OFFSET },
    0,
    0,
    866,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_H_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_VAL_L, TYPE: Type_RCQ_COMMON_REGS_RCQ_PARSER_CORE_CONFIGURATION_DA_FILT1_VAL_L
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DA_FILT_LSB *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_VAL_L_DA_FILT_LSB_FIELD =
{
    "DA_FILT_LSB",
#if RU_INCLUDE_DESC
    "",
    "DA Filter bits 31:0\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_VAL_L_DA_FILT_LSB_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_VAL_L_DA_FILT_LSB_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_VAL_L_DA_FILT_LSB_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_VAL_L_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_VAL_L_DA_FILT_LSB_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_VAL_L *****/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_VAL_L_REG =
{
    "PARSER_CORE_CONFIGURATION_DA_FILT1_VAL_L",
#if RU_INCLUDE_DESC
    "DA_FILT1_VAL_L Register",
    "Config DA filter1 31:0\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_VAL_L_REG_OFFSET },
    0,
    0,
    867,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_VAL_L_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_VAL_H, TYPE: Type_RCQ_COMMON_REGS_RCQ_PARSER_CORE_CONFIGURATION_DA_FILT1_VAL_H
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DA_FILT_MSB *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_VAL_H_DA_FILT_MSB_FIELD =
{
    "DA_FILT_MSB",
#if RU_INCLUDE_DESC
    "",
    "Current DA Filter bits 47:32\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_VAL_H_DA_FILT_MSB_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_VAL_H_DA_FILT_MSB_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_VAL_H_DA_FILT_MSB_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_VAL_H_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_VAL_H_DA_FILT_MSB_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_VAL_H *****/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_VAL_H_REG =
{
    "PARSER_CORE_CONFIGURATION_DA_FILT1_VAL_H",
#if RU_INCLUDE_DESC
    "DA_FILT1_VAL_H Register",
    "Config DA filter1 47:32\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_VAL_H_REG_OFFSET },
    0,
    0,
    868,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_VAL_H_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_L, TYPE: Type_RCQ_COMMON_REGS_RCQ_PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_L
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DA_FILT_LSB *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_L_DA_FILT_LSB_FIELD =
{
    "DA_FILT_LSB",
#if RU_INCLUDE_DESC
    "",
    "DA Filter bits 31:0\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_L_DA_FILT_LSB_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_L_DA_FILT_LSB_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_L_DA_FILT_LSB_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_L_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_L_DA_FILT_LSB_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_L *****/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_L_REG =
{
    "PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_L",
#if RU_INCLUDE_DESC
    "DA_FILT2_VAL_L Register",
    "Config DA filter2 31:0\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_L_REG_OFFSET },
    0,
    0,
    869,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_L_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_H, TYPE: Type_RCQ_COMMON_REGS_RCQ_PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_H
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DA_FILT_MSB *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_H_DA_FILT_MSB_FIELD =
{
    "DA_FILT_MSB",
#if RU_INCLUDE_DESC
    "",
    "Current DA Filter bits 47:32\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_H_DA_FILT_MSB_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_H_DA_FILT_MSB_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_H_DA_FILT_MSB_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_H_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_H_DA_FILT_MSB_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_H *****/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_H_REG =
{
    "PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_H",
#if RU_INCLUDE_DESC
    "DA_FILT2_VAL_H Register",
    "Config DA filter2 47:32\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_H_REG_OFFSET },
    0,
    0,
    870,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_H_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_L, TYPE: Type_RCQ_COMMON_REGS_RCQ_PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_L
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DA_FILT_LSB *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_L_DA_FILT_LSB_FIELD =
{
    "DA_FILT_LSB",
#if RU_INCLUDE_DESC
    "",
    "DA Filter bits 31:0\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_L_DA_FILT_LSB_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_L_DA_FILT_LSB_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_L_DA_FILT_LSB_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_L_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_L_DA_FILT_LSB_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_L *****/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_L_REG =
{
    "PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_L",
#if RU_INCLUDE_DESC
    "DA_FILT3_VAL_L Register",
    "Config DA filter3 31:0\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_L_REG_OFFSET },
    0,
    0,
    871,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_L_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_H, TYPE: Type_RCQ_COMMON_REGS_RCQ_PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_H
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DA_FILT_MSB *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_H_DA_FILT_MSB_FIELD =
{
    "DA_FILT_MSB",
#if RU_INCLUDE_DESC
    "",
    "Current DA Filter bits 47:32\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_H_DA_FILT_MSB_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_H_DA_FILT_MSB_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_H_DA_FILT_MSB_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_H_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_H_DA_FILT_MSB_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_H *****/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_H_REG =
{
    "PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_H",
#if RU_INCLUDE_DESC
    "DA_FILT3_VAL_H Register",
    "Config DA filter3 47:32\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_H_REG_OFFSET },
    0,
    0,
    872,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_H_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_L, TYPE: Type_RCQ_COMMON_REGS_RCQ_PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_L
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DA_FILT_LSB *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_L_DA_FILT_LSB_FIELD =
{
    "DA_FILT_LSB",
#if RU_INCLUDE_DESC
    "",
    "DA Filter bits 31:0\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_L_DA_FILT_LSB_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_L_DA_FILT_LSB_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_L_DA_FILT_LSB_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_L_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_L_DA_FILT_LSB_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_L *****/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_L_REG =
{
    "PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_L",
#if RU_INCLUDE_DESC
    "DA_FILT4_VAL_L Register",
    "Config DA filter4 31:0\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_L_REG_OFFSET },
    0,
    0,
    873,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_L_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_H, TYPE: Type_RCQ_COMMON_REGS_RCQ_PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_H
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DA_FILT_MSB *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_H_DA_FILT_MSB_FIELD =
{
    "DA_FILT_MSB",
#if RU_INCLUDE_DESC
    "",
    "Current DA Filter bits 47:32\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_H_DA_FILT_MSB_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_H_DA_FILT_MSB_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_H_DA_FILT_MSB_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_H_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_H_DA_FILT_MSB_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_H *****/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_H_REG =
{
    "PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_H",
#if RU_INCLUDE_DESC
    "DA_FILT4_VAL_H Register",
    "Config DA Filter4 47:32\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_H_REG_OFFSET },
    0,
    0,
    874,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_H_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_L, TYPE: Type_RCQ_COMMON_REGS_RCQ_PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_L
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DA_FILT_LSB *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_L_DA_FILT_LSB_FIELD =
{
    "DA_FILT_LSB",
#if RU_INCLUDE_DESC
    "",
    "DA Filter bits 31:0\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_L_DA_FILT_LSB_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_L_DA_FILT_LSB_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_L_DA_FILT_LSB_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_L_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_L_DA_FILT_LSB_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_L *****/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_L_REG =
{
    "PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_L",
#if RU_INCLUDE_DESC
    "DA_FILT5_VAL_L Register",
    "Config DA filter5 31:0\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_L_REG_OFFSET },
    0,
    0,
    875,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_L_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_H, TYPE: Type_RCQ_COMMON_REGS_RCQ_PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_H
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DA_FILT_MSB *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_H_DA_FILT_MSB_FIELD =
{
    "DA_FILT_MSB",
#if RU_INCLUDE_DESC
    "",
    "Current DA Filter bits 47:32\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_H_DA_FILT_MSB_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_H_DA_FILT_MSB_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_H_DA_FILT_MSB_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_H_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_H_DA_FILT_MSB_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_H *****/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_H_REG =
{
    "PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_H",
#if RU_INCLUDE_DESC
    "DA_FILT5_VAL_H Register",
    "Config DA Filter5 47:32\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_H_REG_OFFSET },
    0,
    0,
    876,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_H_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_L, TYPE: Type_RCQ_COMMON_REGS_RCQ_PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_L
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DA_FILT_LSB *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_L_DA_FILT_LSB_FIELD =
{
    "DA_FILT_LSB",
#if RU_INCLUDE_DESC
    "",
    "DA Filter bits 31:0\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_L_DA_FILT_LSB_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_L_DA_FILT_LSB_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_L_DA_FILT_LSB_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_L_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_L_DA_FILT_LSB_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_L *****/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_L_REG =
{
    "PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_L",
#if RU_INCLUDE_DESC
    "DA_FILT6_VAL_L Register",
    "Config DA filter6 31:0\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_L_REG_OFFSET },
    0,
    0,
    877,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_L_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_H, TYPE: Type_RCQ_COMMON_REGS_RCQ_PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_H
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DA_FILT_MSB *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_H_DA_FILT_MSB_FIELD =
{
    "DA_FILT_MSB",
#if RU_INCLUDE_DESC
    "",
    "Current DA Filter bits 47:32\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_H_DA_FILT_MSB_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_H_DA_FILT_MSB_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_H_DA_FILT_MSB_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_H_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_H_DA_FILT_MSB_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_H *****/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_H_REG =
{
    "PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_H",
#if RU_INCLUDE_DESC
    "DA_FILT6_VAL_H Register",
    "Config DA Filter6 47:32\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_H_REG_OFFSET },
    0,
    0,
    878,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_H_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_L, TYPE: Type_RCQ_COMMON_REGS_RCQ_PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_L
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DA_FILT_LSB *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_L_DA_FILT_LSB_FIELD =
{
    "DA_FILT_LSB",
#if RU_INCLUDE_DESC
    "",
    "DA Filter bits 31:0\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_L_DA_FILT_LSB_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_L_DA_FILT_LSB_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_L_DA_FILT_LSB_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_L_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_L_DA_FILT_LSB_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_L *****/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_L_REG =
{
    "PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_L",
#if RU_INCLUDE_DESC
    "DA_FILT7_VAL_L Register",
    "Config DA filter7 31:0\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_L_REG_OFFSET },
    0,
    0,
    879,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_L_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_H, TYPE: Type_RCQ_COMMON_REGS_RCQ_PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_H
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DA_FILT_MSB *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_H_DA_FILT_MSB_FIELD =
{
    "DA_FILT_MSB",
#if RU_INCLUDE_DESC
    "",
    "Current DA Filter bits 47:32\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_H_DA_FILT_MSB_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_H_DA_FILT_MSB_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_H_DA_FILT_MSB_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_H_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_H_DA_FILT_MSB_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_H *****/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_H_REG =
{
    "PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_H",
#if RU_INCLUDE_DESC
    "DA_FILT7_VAL_H Register",
    "Config DA Filter7 47:32\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_H_REG_OFFSET },
    0,
    0,
    880,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_H_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT8_VAL_L, TYPE: Type_RCQ_COMMON_REGS_RCQ_PARSER_CORE_CONFIGURATION_DA_FILT8_VAL_L
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DA_FILT_LSB *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT8_VAL_L_DA_FILT_LSB_FIELD =
{
    "DA_FILT_LSB",
#if RU_INCLUDE_DESC
    "",
    "DA Filter bits 31:0\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT8_VAL_L_DA_FILT_LSB_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT8_VAL_L_DA_FILT_LSB_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT8_VAL_L_DA_FILT_LSB_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT8_VAL_L_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT8_VAL_L_DA_FILT_LSB_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT8_VAL_L *****/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT8_VAL_L_REG =
{
    "PARSER_CORE_CONFIGURATION_DA_FILT8_VAL_L",
#if RU_INCLUDE_DESC
    "DA_FILT8_VAL_L Register",
    "Config DA filter8 31:0\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT8_VAL_L_REG_OFFSET },
    0,
    0,
    881,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT8_VAL_L_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT8_VAL_H, TYPE: Type_RCQ_COMMON_REGS_RCQ_PARSER_CORE_CONFIGURATION_DA_FILT8_VAL_H
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DA_FILT_MSB *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT8_VAL_H_DA_FILT_MSB_FIELD =
{
    "DA_FILT_MSB",
#if RU_INCLUDE_DESC
    "",
    "Current DA Filter bits 47:32\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT8_VAL_H_DA_FILT_MSB_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT8_VAL_H_DA_FILT_MSB_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT8_VAL_H_DA_FILT_MSB_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT8_VAL_H_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT8_VAL_H_DA_FILT_MSB_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT8_VAL_H *****/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT8_VAL_H_REG =
{
    "PARSER_CORE_CONFIGURATION_DA_FILT8_VAL_H",
#if RU_INCLUDE_DESC
    "DA_FILT8_VAL_H Register",
    "Config DA Filter8 47:32\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT8_VAL_H_REG_OFFSET },
    0,
    0,
    882,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT8_VAL_H_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_L, TYPE: Type_RCQ_COMMON_REGS_RCQ_PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_L
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DA_FILT_MASK_L *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_L_DA_FILT_MASK_L_FIELD =
{
    "DA_FILT_MASK_L",
#if RU_INCLUDE_DESC
    "",
    "Current DA Filter mask bits 31:0\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_L_DA_FILT_MASK_L_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_L_DA_FILT_MASK_L_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_L_DA_FILT_MASK_L_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_L_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_L_DA_FILT_MASK_L_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_L *****/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_L_REG =
{
    "PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_L",
#if RU_INCLUDE_DESC
    "DA_FILT0_MASK_L Register",
    "Config DA Filter mask 15:0\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_L_REG_OFFSET },
    0,
    0,
    883,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_L_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_H, TYPE: Type_RCQ_COMMON_REGS_RCQ_PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_H
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DA_FILT_MASK_MSB *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_H_DA_FILT_MASK_MSB_FIELD =
{
    "DA_FILT_MASK_MSB",
#if RU_INCLUDE_DESC
    "",
    "Current DA Filter mask bits 47:32\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_H_DA_FILT_MASK_MSB_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_H_DA_FILT_MASK_MSB_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_H_DA_FILT_MASK_MSB_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_H_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_H_DA_FILT_MASK_MSB_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_H *****/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_H_REG =
{
    "PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_H",
#if RU_INCLUDE_DESC
    "DA_FILT0_MASK_H Register",
    "Config DA Filter0 mask 47:32\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_H_REG_OFFSET },
    0,
    0,
    884,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_H_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_MASK_L, TYPE: Type_RCQ_COMMON_REGS_RCQ_PARSER_CORE_CONFIGURATION_DA_FILT1_MASK_L
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DA_FILT_MASK_L *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_MASK_L_DA_FILT_MASK_L_FIELD =
{
    "DA_FILT_MASK_L",
#if RU_INCLUDE_DESC
    "",
    "Current DA Filter mask bits 31:0\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_MASK_L_DA_FILT_MASK_L_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_MASK_L_DA_FILT_MASK_L_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_MASK_L_DA_FILT_MASK_L_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_MASK_L_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_MASK_L_DA_FILT_MASK_L_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_MASK_L *****/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_MASK_L_REG =
{
    "PARSER_CORE_CONFIGURATION_DA_FILT1_MASK_L",
#if RU_INCLUDE_DESC
    "DA_FILT1_MASK_L Register",
    "Config DA Filter1 mask 31:0\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_MASK_L_REG_OFFSET },
    0,
    0,
    885,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_MASK_L_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_MASK_H, TYPE: Type_RCQ_COMMON_REGS_RCQ_PARSER_CORE_CONFIGURATION_DA_FILT1_MASK_H
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DA_FILT_MASK_MSB *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_MASK_H_DA_FILT_MASK_MSB_FIELD =
{
    "DA_FILT_MASK_MSB",
#if RU_INCLUDE_DESC
    "",
    "Current DA Filter mask bits 47:32\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_MASK_H_DA_FILT_MASK_MSB_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_MASK_H_DA_FILT_MASK_MSB_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_MASK_H_DA_FILT_MASK_MSB_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_MASK_H_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_MASK_H_DA_FILT_MASK_MSB_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_MASK_H *****/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_MASK_H_REG =
{
    "PARSER_CORE_CONFIGURATION_DA_FILT1_MASK_H",
#if RU_INCLUDE_DESC
    "DA_FILT1_MASK_H Register",
    "Config DA Filter1 mask 47:32\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_MASK_H_REG_OFFSET },
    0,
    0,
    886,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_MASK_H_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0, TYPE: Type_RCQ_COMMON_REGS_RCQ_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DA_FILT0_VALID *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT0_VALID_FIELD =
{
    "DA_FILT0_VALID",
#if RU_INCLUDE_DESC
    "",
    "DA Filter0 valid bit\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT0_VALID_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT0_VALID_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT0_VALID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DA_FILT1_VALID *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT1_VALID_FIELD =
{
    "DA_FILT1_VALID",
#if RU_INCLUDE_DESC
    "",
    "DA Filter1 valid bit\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT1_VALID_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT1_VALID_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT1_VALID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DA_FILT2_VALID *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT2_VALID_FIELD =
{
    "DA_FILT2_VALID",
#if RU_INCLUDE_DESC
    "",
    "DA Filter2 valid bit\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT2_VALID_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT2_VALID_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT2_VALID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DA_FILT3_VALID *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT3_VALID_FIELD =
{
    "DA_FILT3_VALID",
#if RU_INCLUDE_DESC
    "",
    "DA Filter3 valid bit\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT3_VALID_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT3_VALID_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT3_VALID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DA_FILT4_VALID *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT4_VALID_FIELD =
{
    "DA_FILT4_VALID",
#if RU_INCLUDE_DESC
    "",
    "DA Filter4 valid bit\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT4_VALID_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT4_VALID_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT4_VALID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DA_FILT5_VALID *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT5_VALID_FIELD =
{
    "DA_FILT5_VALID",
#if RU_INCLUDE_DESC
    "",
    "DA Filter5 valid bit\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT5_VALID_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT5_VALID_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT5_VALID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DA_FILT6_VALID *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT6_VALID_FIELD =
{
    "DA_FILT6_VALID",
#if RU_INCLUDE_DESC
    "",
    "DA Filter6 valid bit\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT6_VALID_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT6_VALID_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT6_VALID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DA_FILT7_VALID *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT7_VALID_FIELD =
{
    "DA_FILT7_VALID",
#if RU_INCLUDE_DESC
    "",
    "DA Filter7 valid bit\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT7_VALID_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT7_VALID_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT7_VALID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DA_FILT8_VALID *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT8_VALID_FIELD =
{
    "DA_FILT8_VALID",
#if RU_INCLUDE_DESC
    "",
    "DA Filter8 valid bit\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT8_VALID_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT8_VALID_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT8_VALID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT0_VALID_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT1_VALID_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT2_VALID_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT3_VALID_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT4_VALID_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT5_VALID_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT6_VALID_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT7_VALID_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT8_VALID_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0 *****/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_REG =
{
    "PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0",
#if RU_INCLUDE_DESC
    "DA_FILT_VALID_CFG_PROFILE_0 Register",
    "Valid configuration of all DA filters: there is a dedicated bit per each DA filter that says if the current DA filter is valid or not. Used for on-the-fly DA filter value (mask) modifications, since the DA filter parameters are not assigned on single SW register.\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_REG_OFFSET },
    0,
    0,
    887,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1, TYPE: Type_RCQ_COMMON_REGS_RCQ_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DA_FILT0_VALID *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT0_VALID_FIELD =
{
    "DA_FILT0_VALID",
#if RU_INCLUDE_DESC
    "",
    "DA Filter0 valid bit\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT0_VALID_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT0_VALID_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT0_VALID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DA_FILT1_VALID *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT1_VALID_FIELD =
{
    "DA_FILT1_VALID",
#if RU_INCLUDE_DESC
    "",
    "DA Filter1 valid bit\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT1_VALID_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT1_VALID_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT1_VALID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DA_FILT2_VALID *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT2_VALID_FIELD =
{
    "DA_FILT2_VALID",
#if RU_INCLUDE_DESC
    "",
    "DA Filter2 valid bit\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT2_VALID_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT2_VALID_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT2_VALID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DA_FILT3_VALID *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT3_VALID_FIELD =
{
    "DA_FILT3_VALID",
#if RU_INCLUDE_DESC
    "",
    "DA Filter3 valid bit\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT3_VALID_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT3_VALID_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT3_VALID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DA_FILT4_VALID *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT4_VALID_FIELD =
{
    "DA_FILT4_VALID",
#if RU_INCLUDE_DESC
    "",
    "DA Filter4 valid bit\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT4_VALID_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT4_VALID_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT4_VALID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DA_FILT5_VALID *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT5_VALID_FIELD =
{
    "DA_FILT5_VALID",
#if RU_INCLUDE_DESC
    "",
    "DA Filter5 valid bit\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT5_VALID_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT5_VALID_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT5_VALID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DA_FILT6_VALID *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT6_VALID_FIELD =
{
    "DA_FILT6_VALID",
#if RU_INCLUDE_DESC
    "",
    "DA Filter6 valid bit\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT6_VALID_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT6_VALID_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT6_VALID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DA_FILT7_VALID *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT7_VALID_FIELD =
{
    "DA_FILT7_VALID",
#if RU_INCLUDE_DESC
    "",
    "DA Filter7 valid bit\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT7_VALID_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT7_VALID_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT7_VALID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DA_FILT8_VALID *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT8_VALID_FIELD =
{
    "DA_FILT8_VALID",
#if RU_INCLUDE_DESC
    "",
    "DA Filter8 valid bit\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT8_VALID_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT8_VALID_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT8_VALID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT0_VALID_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT1_VALID_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT2_VALID_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT3_VALID_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT4_VALID_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT5_VALID_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT6_VALID_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT7_VALID_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT8_VALID_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1 *****/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_REG =
{
    "PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1",
#if RU_INCLUDE_DESC
    "DA_FILT_VALID_CFG_PROFILE_1 Register",
    "Valid configuration of all DA filters: there is a dedicated bit per each DA filter that says if the current DA filter is valid or not. Used for on-the-fly DA filter value (mask) modifications, since the DA filter parameters are not assigned on single SW register.\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_REG_OFFSET },
    0,
    0,
    888,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2, TYPE: Type_RCQ_COMMON_REGS_RCQ_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DA_FILT0_VALID *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT0_VALID_FIELD =
{
    "DA_FILT0_VALID",
#if RU_INCLUDE_DESC
    "",
    "DA Filter0 valid bit\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT0_VALID_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT0_VALID_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT0_VALID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DA_FILT1_VALID *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT1_VALID_FIELD =
{
    "DA_FILT1_VALID",
#if RU_INCLUDE_DESC
    "",
    "DA Filter1 valid bit\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT1_VALID_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT1_VALID_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT1_VALID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DA_FILT2_VALID *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT2_VALID_FIELD =
{
    "DA_FILT2_VALID",
#if RU_INCLUDE_DESC
    "",
    "DA Filter2 valid bit\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT2_VALID_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT2_VALID_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT2_VALID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DA_FILT3_VALID *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT3_VALID_FIELD =
{
    "DA_FILT3_VALID",
#if RU_INCLUDE_DESC
    "",
    "DA Filter3 valid bit\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT3_VALID_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT3_VALID_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT3_VALID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DA_FILT4_VALID *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT4_VALID_FIELD =
{
    "DA_FILT4_VALID",
#if RU_INCLUDE_DESC
    "",
    "DA Filter4 valid bit\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT4_VALID_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT4_VALID_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT4_VALID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DA_FILT5_VALID *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT5_VALID_FIELD =
{
    "DA_FILT5_VALID",
#if RU_INCLUDE_DESC
    "",
    "DA Filter5 valid bit\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT5_VALID_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT5_VALID_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT5_VALID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DA_FILT6_VALID *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT6_VALID_FIELD =
{
    "DA_FILT6_VALID",
#if RU_INCLUDE_DESC
    "",
    "DA Filter6 valid bit\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT6_VALID_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT6_VALID_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT6_VALID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DA_FILT7_VALID *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT7_VALID_FIELD =
{
    "DA_FILT7_VALID",
#if RU_INCLUDE_DESC
    "",
    "DA Filter7 valid bit\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT7_VALID_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT7_VALID_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT7_VALID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DA_FILT8_VALID *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT8_VALID_FIELD =
{
    "DA_FILT8_VALID",
#if RU_INCLUDE_DESC
    "",
    "DA Filter8 valid bit\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT8_VALID_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT8_VALID_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT8_VALID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT0_VALID_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT1_VALID_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT2_VALID_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT3_VALID_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT4_VALID_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT5_VALID_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT6_VALID_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT7_VALID_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT8_VALID_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2 *****/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_REG =
{
    "PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2",
#if RU_INCLUDE_DESC
    "DA_FILT_VALID_CFG_PROFILE_2 Register",
    "Valid configuration of all DA filters: there is a dedicated bit per each DA filter that says if the current DA filter is valid or not. Used for on-the-fly DA filter value (mask) modifications, since the DA filter parameters are not assigned on single SW register.\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_REG_OFFSET },
    0,
    0,
    889,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_PARSER_CORE_CONFIGURATION_GRE_PROTOCOL_CFG, TYPE: Type_RCQ_COMMON_REGS_RCQ_PARSER_CORE_CONFIGURATION_GRE_PROTOCOL_CFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: GRE_PROTOCOL *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_GRE_PROTOCOL_CFG_GRE_PROTOCOL_FIELD =
{
    "GRE_PROTOCOL",
#if RU_INCLUDE_DESC
    "",
    "GRE_PROTOCOL\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_GRE_PROTOCOL_CFG_GRE_PROTOCOL_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_GRE_PROTOCOL_CFG_GRE_PROTOCOL_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_GRE_PROTOCOL_CFG_GRE_PROTOCOL_FIELD_SHIFT },
    34827,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_GRE_PROTOCOL_CFG_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_GRE_PROTOCOL_CFG_GRE_PROTOCOL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_PARSER_CORE_CONFIGURATION_GRE_PROTOCOL_CFG *****/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_GRE_PROTOCOL_CFG_REG =
{
    "PARSER_CORE_CONFIGURATION_GRE_PROTOCOL_CFG",
#if RU_INCLUDE_DESC
    "GRE_PROTOCOL_CFG Register",
    "GRE Protocol\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_GRE_PROTOCOL_CFG_REG_OFFSET },
    0,
    0,
    890,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_GRE_PROTOCOL_CFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG, TYPE: Type_RCQ_COMMON_REGS_RCQ_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: SIZE_PROFILE_0 *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG_SIZE_PROFILE_0_FIELD =
{
    "SIZE_PROFILE_0",
#if RU_INCLUDE_DESC
    "",
    "profile 0 tag size, valid values are 0,2,4,6,8\n\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG_SIZE_PROFILE_0_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG_SIZE_PROFILE_0_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG_SIZE_PROFILE_0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SIZE_PROFILE_1 *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG_SIZE_PROFILE_1_FIELD =
{
    "SIZE_PROFILE_1",
#if RU_INCLUDE_DESC
    "",
    "profile 1 tag size, valid values are 0,2,4,6,8\n\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG_SIZE_PROFILE_1_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG_SIZE_PROFILE_1_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG_SIZE_PROFILE_1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SIZE_PROFILE_2 *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG_SIZE_PROFILE_2_FIELD =
{
    "SIZE_PROFILE_2",
#if RU_INCLUDE_DESC
    "",
    "profile 2 tag size, valid values are 0,2,4,6,8\n\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG_SIZE_PROFILE_2_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG_SIZE_PROFILE_2_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG_SIZE_PROFILE_2_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PRE_DA_DPROFILE_0 *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG_PRE_DA_DPROFILE_0_FIELD =
{
    "PRE_DA_DPROFILE_0",
#if RU_INCLUDE_DESC
    "",
    "Pre-DA Profile 0\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG_PRE_DA_DPROFILE_0_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG_PRE_DA_DPROFILE_0_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG_PRE_DA_DPROFILE_0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PRE_DA_DPROFILE_1 *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG_PRE_DA_DPROFILE_1_FIELD =
{
    "PRE_DA_DPROFILE_1",
#if RU_INCLUDE_DESC
    "",
    "Pre-DA Profile 1\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG_PRE_DA_DPROFILE_1_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG_PRE_DA_DPROFILE_1_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG_PRE_DA_DPROFILE_1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PRE_DA_DPROFILE_2 *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG_PRE_DA_DPROFILE_2_FIELD =
{
    "PRE_DA_DPROFILE_2",
#if RU_INCLUDE_DESC
    "",
    "Pre-DA Profile 2\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG_PRE_DA_DPROFILE_2_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG_PRE_DA_DPROFILE_2_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG_PRE_DA_DPROFILE_2_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG_SIZE_PROFILE_0_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG_SIZE_PROFILE_1_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG_SIZE_PROFILE_2_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG_PRE_DA_DPROFILE_0_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG_PRE_DA_DPROFILE_1_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG_PRE_DA_DPROFILE_2_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG *****/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG_REG =
{
    "PARSER_CORE_CONFIGURATION_PROP_TAG_CFG",
#if RU_INCLUDE_DESC
    "PROP_TAG_CFG Register",
    "Prop Tag Configuration\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG_REG_OFFSET },
    0,
    0,
    891,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    6,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_PARSER_CORE_CONFIGURATION_DOS_ATTACK, TYPE: Type_RCQ_COMMON_REGS_RCQ_PARSER_CORE_CONFIGURATION_DOS_ATTACK
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: MASK *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DOS_ATTACK_MASK_FIELD =
{
    "MASK",
#if RU_INCLUDE_DESC
    "",
    "mask bit per DOS Attack reason. 1 - Attack is enabled. 0 - Attack is disabled\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DOS_ATTACK_MASK_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DOS_ATTACK_MASK_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DOS_ATTACK_MASK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_DOS_ATTACK_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DOS_ATTACK_MASK_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_PARSER_CORE_CONFIGURATION_DOS_ATTACK *****/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DOS_ATTACK_REG =
{
    "PARSER_CORE_CONFIGURATION_DOS_ATTACK",
#if RU_INCLUDE_DESC
    "DOS_ATTACK Register",
    "Control the detection of the following DOS Attacks:\nMAC Spoof MACDA = MACSA\nIP_LAND IPDA=IPSA in an IP(v4/v6) datagram.\nTCP_BLAT  DPort=SPort in a TCP header carried in an unfragmented IP datagram or in the first fragment of a fragmented IP datagram.\nUDP_BLAT  DPport=SPort in a UDP header carried in an unfragmented IP datagram or in the first fragment of a fragmented IP datagram.\nTCP_NULLScan  Seq_Num=0 & All TCP_FLAGs=0, in a TCP header carried in an unfragmented IP datagram or in the first fragment of a fragmented\nTCP_XMASScan  Seq_Num=0 & FIN=1 & URG=1 & PSH=1 in a TCP header carried in an unfragmented IP datagram or in the first fragment of a fragmented IP datagram.\nTCP_SYNFINScanSYN=1 & FIN=1 in a TCP header carried in an unfragmented IP datagram or in the first fragment of a fragmented IP datagram.\nTCP_SYNErrorSYN=1 & ACK=0 & SRC_Port<1024 in a TCP header carried in an unfragmented IP datagram or in the first fragment of a fragmented IP datagram.\nTCP_ShortHDRThe length of a TCP header carried in an unfragmented IP datagram or the first fragment of a fragmented IP datagram is less than MIN_TCP_Header_Size (20B) .\nTCP_FragErrorThe Fragment_Offset=1 in any fragment of a fragmented IP datagram carring part of TCP data.\nICMPv4_FragmentThe ICMPv4 protocol data unit carrier in a fragmented IPv4 datagram.\nICMPv6_FragmentThe ICMPv6 protocol data unit carrier in a fragmented IPv6 datagram.\nICMPv4_LongPingThe ICMPv4 Ping(Echo Request) protocol data unit carried in an unfragmented IPv4 datagram with its Payload Length indicating a value greater than the MAX_ICMPv4_Size + size of IPv4 heater.\nICMPv6_LongPingThe ICMPv6 Ping(Echo Request) protocol data unit carried in an unfragmented IPv6 datagram with its Payload Length indicating a value greater than the MAX_ICMPv6_Size.\n\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_DOS_ATTACK_REG_OFFSET },
    0,
    0,
    892,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DOS_ATTACK_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_PARSER_CORE_CONFIGURATION_ICMP_MAX_SIZE, TYPE: Type_RCQ_COMMON_REGS_RCQ_PARSER_CORE_CONFIGURATION_ICMP_MAX_SIZE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: V4_SIZE *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_ICMP_MAX_SIZE_V4_SIZE_FIELD =
{
    "V4_SIZE",
#if RU_INCLUDE_DESC
    "",
    "Max Size for ICMPV4 packet. See DOS Attack detection details\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_ICMP_MAX_SIZE_V4_SIZE_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_ICMP_MAX_SIZE_V4_SIZE_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_ICMP_MAX_SIZE_V4_SIZE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: V6_SIZE *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_ICMP_MAX_SIZE_V6_SIZE_FIELD =
{
    "V6_SIZE",
#if RU_INCLUDE_DESC
    "",
    "Max Size for ICMPv6 packet. See DOS Attack detection details\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_ICMP_MAX_SIZE_V6_SIZE_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_ICMP_MAX_SIZE_V6_SIZE_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_ICMP_MAX_SIZE_V6_SIZE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_ICMP_MAX_SIZE_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_ICMP_MAX_SIZE_V4_SIZE_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_ICMP_MAX_SIZE_V6_SIZE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_PARSER_CORE_CONFIGURATION_ICMP_MAX_SIZE *****/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_ICMP_MAX_SIZE_REG =
{
    "PARSER_CORE_CONFIGURATION_ICMP_MAX_SIZE",
#if RU_INCLUDE_DESC
    "ICMP_MAX_SIZE Register",
    "Sets the MAX ICMPV4/V6 packet sizes for the purpose of detection of ICMP DOS attacks\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_ICMP_MAX_SIZE_REG_OFFSET },
    0,
    0,
    893,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_ICMP_MAX_SIZE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_PARSER_CORE_CONFIGURATION_KEY_CFG, TYPE: Type_RCQ_COMMON_REGS_RCQ_PARSER_CORE_CONFIGURATION_KEY_CFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: L2_TOS_MASK *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_KEY_CFG_L2_TOS_MASK_FIELD =
{
    "L2_TOS_MASK",
#if RU_INCLUDE_DESC
    "",
    "Mask for L2 KEY TOS Field. Value is ANDed with TOS field\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_KEY_CFG_L2_TOS_MASK_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_KEY_CFG_L2_TOS_MASK_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_KEY_CFG_L2_TOS_MASK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: L3_TOS_MASK *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_KEY_CFG_L3_TOS_MASK_FIELD =
{
    "L3_TOS_MASK",
#if RU_INCLUDE_DESC
    "",
    "Mask for L3 KEY TOS Field. Value is ANDed with TOS field\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_KEY_CFG_L3_TOS_MASK_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_KEY_CFG_L3_TOS_MASK_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_KEY_CFG_L3_TOS_MASK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: L2_EXCLUDE_SMAC *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_KEY_CFG_L2_EXCLUDE_SMAC_FIELD =
{
    "L2_EXCLUDE_SMAC",
#if RU_INCLUDE_DESC
    "",
    "Excludes Ethernet Source MAC from L2 Key (field will be set to 0)\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_KEY_CFG_L2_EXCLUDE_SMAC_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_KEY_CFG_L2_EXCLUDE_SMAC_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_KEY_CFG_L2_EXCLUDE_SMAC_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TCP_PURE_ACK_MASK *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_KEY_CFG_TCP_PURE_ACK_MASK_FIELD =
{
    "TCP_PURE_ACK_MASK",
#if RU_INCLUDE_DESC
    "",
    "Mask for Pure ACK field at the result.\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_KEY_CFG_TCP_PURE_ACK_MASK_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_KEY_CFG_TCP_PURE_ACK_MASK_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_KEY_CFG_TCP_PURE_ACK_MASK_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: INCUDE_DEI_IN_VLANS_CRC *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_KEY_CFG_INCUDE_DEI_IN_VLANS_CRC_FIELD =
{
    "INCUDE_DEI_IN_VLANS_CRC",
#if RU_INCLUDE_DESC
    "",
    "Controls whether DEI bit of VLAN TAG is included or masked before CRC, if masked value of bit is 0\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_KEY_CFG_INCUDE_DEI_IN_VLANS_CRC_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_KEY_CFG_INCUDE_DEI_IN_VLANS_CRC_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_KEY_CFG_INCUDE_DEI_IN_VLANS_CRC_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: KEY_SIZE *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_KEY_CFG_KEY_SIZE_FIELD =
{
    "KEY_SIZE",
#if RU_INCLUDE_DESC
    "",
    "Selects 32 Byte or 16 Byte key result mode\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_KEY_CFG_KEY_SIZE_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_KEY_CFG_KEY_SIZE_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_KEY_CFG_KEY_SIZE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MAX_NUM_OF_VLANS_IN_CRC *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_KEY_CFG_MAX_NUM_OF_VLANS_IN_CRC_FIELD =
{
    "MAX_NUM_OF_VLANS_IN_CRC",
#if RU_INCLUDE_DESC
    "",
    "Max number of VLANs in CRC\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_KEY_CFG_MAX_NUM_OF_VLANS_IN_CRC_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_KEY_CFG_MAX_NUM_OF_VLANS_IN_CRC_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_KEY_CFG_MAX_NUM_OF_VLANS_IN_CRC_FIELD_SHIFT },
    3,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: L3_TCP_PURE_ACK_MASK *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_KEY_CFG_L3_TCP_PURE_ACK_MASK_FIELD =
{
    "L3_TCP_PURE_ACK_MASK",
#if RU_INCLUDE_DESC
    "",
    "Mask pure_ack at L3 Key. 1 is allow. 0 is blocked.\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_KEY_CFG_L3_TCP_PURE_ACK_MASK_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_KEY_CFG_L3_TCP_PURE_ACK_MASK_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_KEY_CFG_L3_TCP_PURE_ACK_MASK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: RSRV *****/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_KEY_CFG_RSRV_FIELD =
{
    "RSRV",
#if RU_INCLUDE_DESC
    "",
    "Reserved\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_KEY_CFG_RSRV_FIELD_MASK },
    0,
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_KEY_CFG_RSRV_FIELD_WIDTH },
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_KEY_CFG_RSRV_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_KEY_CFG_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_KEY_CFG_L2_TOS_MASK_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_KEY_CFG_L3_TOS_MASK_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_KEY_CFG_L2_EXCLUDE_SMAC_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_KEY_CFG_TCP_PURE_ACK_MASK_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_KEY_CFG_INCUDE_DEI_IN_VLANS_CRC_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_KEY_CFG_KEY_SIZE_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_KEY_CFG_MAX_NUM_OF_VLANS_IN_CRC_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_KEY_CFG_L3_TCP_PURE_ACK_MASK_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_KEY_CFG_RSRV_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_PARSER_CORE_CONFIGURATION_KEY_CFG *****/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_KEY_CFG_REG =
{
    "PARSER_CORE_CONFIGURATION_KEY_CFG",
#if RU_INCLUDE_DESC
    "KEY_CFG Register",
    "Misc Result Key configurations\n",
#endif
    { RNR_QUAD_PARSER_CORE_CONFIGURATION_KEY_CFG_REG_OFFSET },
    0,
    0,
    894,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_KEY_CFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_DEBUG_FIFO_CONFIG, TYPE: Type_RCQ_COMMON_REGS_RCQ_DEBUG_FIFO_CONFIG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PSRAM_HDR_SW_RST_0 *****/
const ru_field_rec RNR_QUAD_DEBUG_FIFO_CONFIG_PSRAM_HDR_SW_RST_0_FIELD =
{
    "PSRAM_HDR_SW_RST_0",
#if RU_INCLUDE_DESC
    "",
    "Apply software reset to PSRAM header FIFO in EC arbiter\n",
#endif
    { RNR_QUAD_DEBUG_FIFO_CONFIG_PSRAM_HDR_SW_RST_0_FIELD_MASK },
    0,
    { RNR_QUAD_DEBUG_FIFO_CONFIG_PSRAM_HDR_SW_RST_0_FIELD_WIDTH },
    { RNR_QUAD_DEBUG_FIFO_CONFIG_PSRAM_HDR_SW_RST_0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PSRAM_DATA_SW_RST_0 *****/
const ru_field_rec RNR_QUAD_DEBUG_FIFO_CONFIG_PSRAM_DATA_SW_RST_0_FIELD =
{
    "PSRAM_DATA_SW_RST_0",
#if RU_INCLUDE_DESC
    "",
    "Apply software reset to PSRAM data FIFO in EC arbiter\n",
#endif
    { RNR_QUAD_DEBUG_FIFO_CONFIG_PSRAM_DATA_SW_RST_0_FIELD_MASK },
    0,
    { RNR_QUAD_DEBUG_FIFO_CONFIG_PSRAM_DATA_SW_RST_0_FIELD_WIDTH },
    { RNR_QUAD_DEBUG_FIFO_CONFIG_PSRAM_DATA_SW_RST_0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_HDR_SW_RST_0 *****/
const ru_field_rec RNR_QUAD_DEBUG_FIFO_CONFIG_DDR_HDR_SW_RST_0_FIELD =
{
    "DDR_HDR_SW_RST_0",
#if RU_INCLUDE_DESC
    "",
    "Apply software reset to DDR header FIFO in EC arbiter\n",
#endif
    { RNR_QUAD_DEBUG_FIFO_CONFIG_DDR_HDR_SW_RST_0_FIELD_MASK },
    0,
    { RNR_QUAD_DEBUG_FIFO_CONFIG_DDR_HDR_SW_RST_0_FIELD_WIDTH },
    { RNR_QUAD_DEBUG_FIFO_CONFIG_DDR_HDR_SW_RST_0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SELECT_FIFOS_FOR_DEBUG *****/
const ru_field_rec RNR_QUAD_DEBUG_FIFO_CONFIG_SELECT_FIFOS_FOR_DEBUG_FIELD =
{
    "SELECT_FIFOS_FOR_DEBUG",
#if RU_INCLUDE_DESC
    "",
    "Select for which arbiter to display FIFO debug data\n",
#endif
    { RNR_QUAD_DEBUG_FIFO_CONFIG_SELECT_FIFOS_FOR_DEBUG_FIELD_MASK },
    0,
    { RNR_QUAD_DEBUG_FIFO_CONFIG_SELECT_FIFOS_FOR_DEBUG_FIELD_WIDTH },
    { RNR_QUAD_DEBUG_FIFO_CONFIG_SELECT_FIFOS_FOR_DEBUG_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PSRAM_HDR_SW_RST_1 *****/
const ru_field_rec RNR_QUAD_DEBUG_FIFO_CONFIG_PSRAM_HDR_SW_RST_1_FIELD =
{
    "PSRAM_HDR_SW_RST_1",
#if RU_INCLUDE_DESC
    "",
    "Apply software reset to PSRAM header FIFO in EC arbiter\n",
#endif
    { RNR_QUAD_DEBUG_FIFO_CONFIG_PSRAM_HDR_SW_RST_1_FIELD_MASK },
    0,
    { RNR_QUAD_DEBUG_FIFO_CONFIG_PSRAM_HDR_SW_RST_1_FIELD_WIDTH },
    { RNR_QUAD_DEBUG_FIFO_CONFIG_PSRAM_HDR_SW_RST_1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PSRAM_DATA_SW_RST_1 *****/
const ru_field_rec RNR_QUAD_DEBUG_FIFO_CONFIG_PSRAM_DATA_SW_RST_1_FIELD =
{
    "PSRAM_DATA_SW_RST_1",
#if RU_INCLUDE_DESC
    "",
    "Apply software reset to PSRAM data FIFO in EC arbiter\n",
#endif
    { RNR_QUAD_DEBUG_FIFO_CONFIG_PSRAM_DATA_SW_RST_1_FIELD_MASK },
    0,
    { RNR_QUAD_DEBUG_FIFO_CONFIG_PSRAM_DATA_SW_RST_1_FIELD_WIDTH },
    { RNR_QUAD_DEBUG_FIFO_CONFIG_PSRAM_DATA_SW_RST_1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_HDR_SW_RST_1 *****/
const ru_field_rec RNR_QUAD_DEBUG_FIFO_CONFIG_DDR_HDR_SW_RST_1_FIELD =
{
    "DDR_HDR_SW_RST_1",
#if RU_INCLUDE_DESC
    "",
    "Apply software reset to DDR header FIFO in EC arbiter\n",
#endif
    { RNR_QUAD_DEBUG_FIFO_CONFIG_DDR_HDR_SW_RST_1_FIELD_MASK },
    0,
    { RNR_QUAD_DEBUG_FIFO_CONFIG_DDR_HDR_SW_RST_1_FIELD_WIDTH },
    { RNR_QUAD_DEBUG_FIFO_CONFIG_DDR_HDR_SW_RST_1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PSRAM_HDR_SW_RD_ADDR *****/
const ru_field_rec RNR_QUAD_DEBUG_FIFO_CONFIG_PSRAM_HDR_SW_RD_ADDR_FIELD =
{
    "PSRAM_HDR_SW_RD_ADDR",
#if RU_INCLUDE_DESC
    "",
    "Software read address for PSRAM header FIFO\n",
#endif
    { RNR_QUAD_DEBUG_FIFO_CONFIG_PSRAM_HDR_SW_RD_ADDR_FIELD_MASK },
    0,
    { RNR_QUAD_DEBUG_FIFO_CONFIG_PSRAM_HDR_SW_RD_ADDR_FIELD_WIDTH },
    { RNR_QUAD_DEBUG_FIFO_CONFIG_PSRAM_HDR_SW_RD_ADDR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PSRAM_DATA_SW_RD_ADDR *****/
const ru_field_rec RNR_QUAD_DEBUG_FIFO_CONFIG_PSRAM_DATA_SW_RD_ADDR_FIELD =
{
    "PSRAM_DATA_SW_RD_ADDR",
#if RU_INCLUDE_DESC
    "",
    "Software read address for PSRAM data FIFO\n",
#endif
    { RNR_QUAD_DEBUG_FIFO_CONFIG_PSRAM_DATA_SW_RD_ADDR_FIELD_MASK },
    0,
    { RNR_QUAD_DEBUG_FIFO_CONFIG_PSRAM_DATA_SW_RD_ADDR_FIELD_WIDTH },
    { RNR_QUAD_DEBUG_FIFO_CONFIG_PSRAM_DATA_SW_RD_ADDR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_HDR_SW_RD_ADDR *****/
const ru_field_rec RNR_QUAD_DEBUG_FIFO_CONFIG_DDR_HDR_SW_RD_ADDR_FIELD =
{
    "DDR_HDR_SW_RD_ADDR",
#if RU_INCLUDE_DESC
    "",
    "Software read address for DDR header FIFO\n",
#endif
    { RNR_QUAD_DEBUG_FIFO_CONFIG_DDR_HDR_SW_RD_ADDR_FIELD_MASK },
    0,
    { RNR_QUAD_DEBUG_FIFO_CONFIG_DDR_HDR_SW_RD_ADDR_FIELD_WIDTH },
    { RNR_QUAD_DEBUG_FIFO_CONFIG_DDR_HDR_SW_RD_ADDR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_DEBUG_FIFO_CONFIG_FIELDS[] =
{
    &RNR_QUAD_DEBUG_FIFO_CONFIG_PSRAM_HDR_SW_RST_0_FIELD,
    &RNR_QUAD_DEBUG_FIFO_CONFIG_PSRAM_DATA_SW_RST_0_FIELD,
    &RNR_QUAD_DEBUG_FIFO_CONFIG_DDR_HDR_SW_RST_0_FIELD,
    &RNR_QUAD_DEBUG_FIFO_CONFIG_SELECT_FIFOS_FOR_DEBUG_FIELD,
    &RNR_QUAD_DEBUG_FIFO_CONFIG_PSRAM_HDR_SW_RST_1_FIELD,
    &RNR_QUAD_DEBUG_FIFO_CONFIG_PSRAM_DATA_SW_RST_1_FIELD,
    &RNR_QUAD_DEBUG_FIFO_CONFIG_DDR_HDR_SW_RST_1_FIELD,
    &RNR_QUAD_DEBUG_FIFO_CONFIG_PSRAM_HDR_SW_RD_ADDR_FIELD,
    &RNR_QUAD_DEBUG_FIFO_CONFIG_PSRAM_DATA_SW_RD_ADDR_FIELD,
    &RNR_QUAD_DEBUG_FIFO_CONFIG_DDR_HDR_SW_RD_ADDR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_DEBUG_FIFO_CONFIG *****/
const ru_reg_rec RNR_QUAD_DEBUG_FIFO_CONFIG_REG =
{
    "DEBUG_FIFO_CONFIG",
#if RU_INCLUDE_DESC
    "FIFO_CONFIG Register",
    "FIFOs configuration\n",
#endif
    { RNR_QUAD_DEBUG_FIFO_CONFIG_REG_OFFSET },
    0,
    0,
    895,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    10,
    RNR_QUAD_DEBUG_FIFO_CONFIG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS, TYPE: Type_RCQ_COMMON_REGS_RCQ_DEBUG_PSRAM_HDR_FIFO_STATUS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: FULL *****/
const ru_field_rec RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_FULL_FIELD =
{
    "FULL",
#if RU_INCLUDE_DESC
    "",
    "FIFO full indication\n",
#endif
    { RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_FULL_FIELD_MASK },
    0,
    { RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_FULL_FIELD_WIDTH },
    { RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_FULL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EMPTY *****/
const ru_field_rec RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_EMPTY_FIELD =
{
    "EMPTY",
#if RU_INCLUDE_DESC
    "",
    "FIFO empty indication\n",
#endif
    { RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_EMPTY_FIELD_MASK },
    0,
    { RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_EMPTY_FIELD_WIDTH },
    { RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_EMPTY_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PUSH_WR_CNTR *****/
const ru_field_rec RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_PUSH_WR_CNTR_FIELD =
{
    "PUSH_WR_CNTR",
#if RU_INCLUDE_DESC
    "",
    "Push write counter value\n",
#endif
    { RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_PUSH_WR_CNTR_FIELD_MASK },
    0,
    { RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_PUSH_WR_CNTR_FIELD_WIDTH },
    { RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_PUSH_WR_CNTR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: POP_RD_CNTR *****/
const ru_field_rec RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_POP_RD_CNTR_FIELD =
{
    "POP_RD_CNTR",
#if RU_INCLUDE_DESC
    "",
    "Pop read counter value\n",
#endif
    { RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_POP_RD_CNTR_FIELD_MASK },
    0,
    { RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_POP_RD_CNTR_FIELD_WIDTH },
    { RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_POP_RD_CNTR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: USED_WORDS *****/
const ru_field_rec RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_USED_WORDS_FIELD =
{
    "USED_WORDS",
#if RU_INCLUDE_DESC
    "",
    "Used words value\n",
#endif
    { RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_USED_WORDS_FIELD_MASK },
    0,
    { RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_USED_WORDS_FIELD_WIDTH },
    { RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_USED_WORDS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_FIELDS[] =
{
    &RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_FULL_FIELD,
    &RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_EMPTY_FIELD,
    &RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_PUSH_WR_CNTR_FIELD,
    &RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_POP_RD_CNTR_FIELD,
    &RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_USED_WORDS_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS *****/
const ru_reg_rec RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_REG =
{
    "DEBUG_PSRAM_HDR_FIFO_STATUS",
#if RU_INCLUDE_DESC
    "PSRAM_HDR_FIFO_STATUS Register",
    "PSRAM Header FIFO status\n",
#endif
    { RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_REG_OFFSET },
    0,
    0,
    896,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS, TYPE: Type_RCQ_COMMON_REGS_RCQ_DEBUG_PSRAM_DATA_FIFO_STATUS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: FULL *****/
const ru_field_rec RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_FULL_FIELD =
{
    "FULL",
#if RU_INCLUDE_DESC
    "",
    "FIFO full indication\n",
#endif
    { RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_FULL_FIELD_MASK },
    0,
    { RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_FULL_FIELD_WIDTH },
    { RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_FULL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EMPTY *****/
const ru_field_rec RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_EMPTY_FIELD =
{
    "EMPTY",
#if RU_INCLUDE_DESC
    "",
    "FIFO empty indication\n",
#endif
    { RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_EMPTY_FIELD_MASK },
    0,
    { RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_EMPTY_FIELD_WIDTH },
    { RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_EMPTY_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ALMOST_FULL *****/
const ru_field_rec RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_ALMOST_FULL_FIELD =
{
    "ALMOST_FULL",
#if RU_INCLUDE_DESC
    "",
    "Almost FIFO full indication\n",
#endif
    { RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_ALMOST_FULL_FIELD_MASK },
    0,
    { RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_ALMOST_FULL_FIELD_WIDTH },
    { RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_ALMOST_FULL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PUSH_WR_CNTR *****/
const ru_field_rec RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_PUSH_WR_CNTR_FIELD =
{
    "PUSH_WR_CNTR",
#if RU_INCLUDE_DESC
    "",
    "Push write counter value\n",
#endif
    { RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_PUSH_WR_CNTR_FIELD_MASK },
    0,
    { RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_PUSH_WR_CNTR_FIELD_WIDTH },
    { RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_PUSH_WR_CNTR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: POP_RD_CNTR *****/
const ru_field_rec RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_POP_RD_CNTR_FIELD =
{
    "POP_RD_CNTR",
#if RU_INCLUDE_DESC
    "",
    "Pop read counter value\n",
#endif
    { RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_POP_RD_CNTR_FIELD_MASK },
    0,
    { RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_POP_RD_CNTR_FIELD_WIDTH },
    { RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_POP_RD_CNTR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: USED_WORDS *****/
const ru_field_rec RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_USED_WORDS_FIELD =
{
    "USED_WORDS",
#if RU_INCLUDE_DESC
    "",
    "Used words value\n",
#endif
    { RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_USED_WORDS_FIELD_MASK },
    0,
    { RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_USED_WORDS_FIELD_WIDTH },
    { RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_USED_WORDS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_FIELDS[] =
{
    &RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_FULL_FIELD,
    &RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_EMPTY_FIELD,
    &RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_ALMOST_FULL_FIELD,
    &RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_PUSH_WR_CNTR_FIELD,
    &RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_POP_RD_CNTR_FIELD,
    &RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_USED_WORDS_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS *****/
const ru_reg_rec RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_REG =
{
    "DEBUG_PSRAM_DATA_FIFO_STATUS",
#if RU_INCLUDE_DESC
    "PSRAM_DATA_FIFO_STATUS Register",
    "PSRAM Data FIFO status\n",
#endif
    { RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_REG_OFFSET },
    0,
    0,
    897,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    6,
    RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS, TYPE: Type_RCQ_COMMON_REGS_RCQ_DEBUG_DDR_HDR_FIFO_STATUS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: FULL *****/
const ru_field_rec RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_FULL_FIELD =
{
    "FULL",
#if RU_INCLUDE_DESC
    "",
    "FIFO full indication\n",
#endif
    { RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_FULL_FIELD_MASK },
    0,
    { RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_FULL_FIELD_WIDTH },
    { RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_FULL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EMPTY *****/
const ru_field_rec RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_EMPTY_FIELD =
{
    "EMPTY",
#if RU_INCLUDE_DESC
    "",
    "FIFO empty indication\n",
#endif
    { RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_EMPTY_FIELD_MASK },
    0,
    { RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_EMPTY_FIELD_WIDTH },
    { RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_EMPTY_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PUSH_WR_CNTR *****/
const ru_field_rec RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_PUSH_WR_CNTR_FIELD =
{
    "PUSH_WR_CNTR",
#if RU_INCLUDE_DESC
    "",
    "Push write counter value\n",
#endif
    { RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_PUSH_WR_CNTR_FIELD_MASK },
    0,
    { RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_PUSH_WR_CNTR_FIELD_WIDTH },
    { RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_PUSH_WR_CNTR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: POP_RD_CNTR *****/
const ru_field_rec RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_POP_RD_CNTR_FIELD =
{
    "POP_RD_CNTR",
#if RU_INCLUDE_DESC
    "",
    "Pop read counter value\n",
#endif
    { RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_POP_RD_CNTR_FIELD_MASK },
    0,
    { RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_POP_RD_CNTR_FIELD_WIDTH },
    { RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_POP_RD_CNTR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: USED_WORDS *****/
const ru_field_rec RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_USED_WORDS_FIELD =
{
    "USED_WORDS",
#if RU_INCLUDE_DESC
    "",
    "Used words value\n",
#endif
    { RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_USED_WORDS_FIELD_MASK },
    0,
    { RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_USED_WORDS_FIELD_WIDTH },
    { RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_USED_WORDS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_FIELDS[] =
{
    &RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_FULL_FIELD,
    &RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_EMPTY_FIELD,
    &RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_PUSH_WR_CNTR_FIELD,
    &RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_POP_RD_CNTR_FIELD,
    &RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_USED_WORDS_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS *****/
const ru_reg_rec RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_REG =
{
    "DEBUG_DDR_HDR_FIFO_STATUS",
#if RU_INCLUDE_DESC
    "DDR_HDR_FIFO_STATUS Register",
    "DDR Header FIFO status\n",
#endif
    { RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_REG_OFFSET },
    0,
    0,
    898,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS, TYPE: Type_RCQ_COMMON_REGS_RCQ_DEBUG_DDR_DATA_FIFO_STATUS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: FULL *****/
const ru_field_rec RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS_FULL_FIELD =
{
    "FULL",
#if RU_INCLUDE_DESC
    "",
    "FIFO full indication\n",
#endif
    { RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS_FULL_FIELD_MASK },
    0,
    { RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS_FULL_FIELD_WIDTH },
    { RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS_FULL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EMPTY *****/
const ru_field_rec RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS_EMPTY_FIELD =
{
    "EMPTY",
#if RU_INCLUDE_DESC
    "",
    "FIFO empty indication\n",
#endif
    { RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS_EMPTY_FIELD_MASK },
    0,
    { RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS_EMPTY_FIELD_WIDTH },
    { RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS_EMPTY_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ALMOST_FULL *****/
const ru_field_rec RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS_ALMOST_FULL_FIELD =
{
    "ALMOST_FULL",
#if RU_INCLUDE_DESC
    "",
    "Almost FIFO full indication\n",
#endif
    { RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS_ALMOST_FULL_FIELD_MASK },
    0,
    { RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS_ALMOST_FULL_FIELD_WIDTH },
    { RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS_ALMOST_FULL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: WR_CNTR *****/
const ru_field_rec RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS_WR_CNTR_FIELD =
{
    "WR_CNTR",
#if RU_INCLUDE_DESC
    "",
    "rite counter value\n",
#endif
    { RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS_WR_CNTR_FIELD_MASK },
    0,
    { RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS_WR_CNTR_FIELD_WIDTH },
    { RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS_WR_CNTR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: RD_CNTR *****/
const ru_field_rec RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS_RD_CNTR_FIELD =
{
    "RD_CNTR",
#if RU_INCLUDE_DESC
    "",
    "Read counter value\n",
#endif
    { RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS_RD_CNTR_FIELD_MASK },
    0,
    { RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS_RD_CNTR_FIELD_WIDTH },
    { RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS_RD_CNTR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS_FIELDS[] =
{
    &RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS_FULL_FIELD,
    &RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS_EMPTY_FIELD,
    &RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS_ALMOST_FULL_FIELD,
    &RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS_WR_CNTR_FIELD,
    &RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS_RD_CNTR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS *****/
const ru_reg_rec RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS_REG =
{
    "DEBUG_DDR_DATA_FIFO_STATUS",
#if RU_INCLUDE_DESC
    "DDR_DATA_FIFO_STATUS Register",
    "DDR Data FIFO status\n",
#endif
    { RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS_REG_OFFSET },
    0,
    0,
    899,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS2, TYPE: Type_RCQ_COMMON_REGS_RCQ_DEBUG_DDR_DATA_FIFO_STATUS2
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: READ_ADDR *****/
const ru_field_rec RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS2_READ_ADDR_FIELD =
{
    "READ_ADDR",
#if RU_INCLUDE_DESC
    "",
    "Current read address\n",
#endif
    { RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS2_READ_ADDR_FIELD_MASK },
    0,
    { RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS2_READ_ADDR_FIELD_WIDTH },
    { RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS2_READ_ADDR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: USED_WORDS *****/
const ru_field_rec RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS2_USED_WORDS_FIELD =
{
    "USED_WORDS",
#if RU_INCLUDE_DESC
    "",
    "Used words\n",
#endif
    { RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS2_USED_WORDS_FIELD_MASK },
    0,
    { RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS2_USED_WORDS_FIELD_WIDTH },
    { RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS2_USED_WORDS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS2_FIELDS[] =
{
    &RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS2_READ_ADDR_FIELD,
    &RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS2_USED_WORDS_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS2 *****/
const ru_reg_rec RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS2_REG =
{
    "DEBUG_DDR_DATA_FIFO_STATUS2",
#if RU_INCLUDE_DESC
    "DDR_DATA_FIFO_STATUS2 Register",
    "DDR Data FIFO status 2\n",
#endif
    { RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS2_REG_OFFSET },
    0,
    0,
    900,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS2_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_DATA1, TYPE: Type_RCQ_COMMON_REGS_RCQ_DEBUG_PSRAM_HDR_FIFO_DATA1
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_DATA1_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "Data\n",
#endif
    { RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_DATA1_DATA_FIELD_MASK },
    0,
    { RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_DATA1_DATA_FIELD_WIDTH },
    { RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_DATA1_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_DATA1_FIELDS[] =
{
    &RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_DATA1_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_DATA1 *****/
const ru_reg_rec RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_DATA1_REG =
{
    "DEBUG_PSRAM_HDR_FIFO_DATA1",
#if RU_INCLUDE_DESC
    "PSRAM_HDR_FIFO_DATA1 Register",
    "Read contents of FIFO memory\n",
#endif
    { RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_DATA1_REG_OFFSET },
    0,
    0,
    901,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_DATA1_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_DATA2, TYPE: Type_RCQ_COMMON_REGS_RCQ_DEBUG_PSRAM_HDR_FIFO_DATA2
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_DATA2_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "Data\n",
#endif
    { RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_DATA2_DATA_FIELD_MASK },
    0,
    { RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_DATA2_DATA_FIELD_WIDTH },
    { RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_DATA2_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_DATA2_FIELDS[] =
{
    &RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_DATA2_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_DATA2 *****/
const ru_reg_rec RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_DATA2_REG =
{
    "DEBUG_PSRAM_HDR_FIFO_DATA2",
#if RU_INCLUDE_DESC
    "PSRAM_HDR_FIFO_DATA2 Register",
    "Read contents of FIFO memory\n",
#endif
    { RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_DATA2_REG_OFFSET },
    0,
    0,
    902,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_DATA2_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_DATA1, TYPE: Type_RCQ_COMMON_REGS_RCQ_DEBUG_PSRAM_DATA_FIFO_DATA1
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_DATA1_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "Data\n",
#endif
    { RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_DATA1_DATA_FIELD_MASK },
    0,
    { RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_DATA1_DATA_FIELD_WIDTH },
    { RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_DATA1_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_DATA1_FIELDS[] =
{
    &RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_DATA1_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_DATA1 *****/
const ru_reg_rec RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_DATA1_REG =
{
    "DEBUG_PSRAM_DATA_FIFO_DATA1",
#if RU_INCLUDE_DESC
    "PSRAM_DATA_FIFO_DATA1 Register",
    "Read contents of FIFO memory\n",
#endif
    { RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_DATA1_REG_OFFSET },
    0,
    0,
    903,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_DATA1_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_DATA2, TYPE: Type_RCQ_COMMON_REGS_RCQ_DEBUG_PSRAM_DATA_FIFO_DATA2
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_DATA2_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "Data\n",
#endif
    { RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_DATA2_DATA_FIELD_MASK },
    0,
    { RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_DATA2_DATA_FIELD_WIDTH },
    { RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_DATA2_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_DATA2_FIELDS[] =
{
    &RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_DATA2_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_DATA2 *****/
const ru_reg_rec RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_DATA2_REG =
{
    "DEBUG_PSRAM_DATA_FIFO_DATA2",
#if RU_INCLUDE_DESC
    "PSRAM_DATA_FIFO_DATA2 Register",
    "Read contents of FIFO memory\n",
#endif
    { RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_DATA2_REG_OFFSET },
    0,
    0,
    904,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_DATA2_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_DEBUG_DDR_HDR_FIFO_DATA1, TYPE: Type_RCQ_COMMON_REGS_RCQ_DEBUG_DDR_HDR_FIFO_DATA1
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec RNR_QUAD_DEBUG_DDR_HDR_FIFO_DATA1_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "Data\n",
#endif
    { RNR_QUAD_DEBUG_DDR_HDR_FIFO_DATA1_DATA_FIELD_MASK },
    0,
    { RNR_QUAD_DEBUG_DDR_HDR_FIFO_DATA1_DATA_FIELD_WIDTH },
    { RNR_QUAD_DEBUG_DDR_HDR_FIFO_DATA1_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_DEBUG_DDR_HDR_FIFO_DATA1_FIELDS[] =
{
    &RNR_QUAD_DEBUG_DDR_HDR_FIFO_DATA1_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_DEBUG_DDR_HDR_FIFO_DATA1 *****/
const ru_reg_rec RNR_QUAD_DEBUG_DDR_HDR_FIFO_DATA1_REG =
{
    "DEBUG_DDR_HDR_FIFO_DATA1",
#if RU_INCLUDE_DESC
    "DDR_HDR_FIFO_DATA1 Register",
    "Read contents of FIFO memory\n",
#endif
    { RNR_QUAD_DEBUG_DDR_HDR_FIFO_DATA1_REG_OFFSET },
    0,
    0,
    905,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_DEBUG_DDR_HDR_FIFO_DATA1_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_DEBUG_DDR_HDR_FIFO_DATA2, TYPE: Type_RCQ_COMMON_REGS_RCQ_DEBUG_DDR_HDR_FIFO_DATA2
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec RNR_QUAD_DEBUG_DDR_HDR_FIFO_DATA2_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "Data\n",
#endif
    { RNR_QUAD_DEBUG_DDR_HDR_FIFO_DATA2_DATA_FIELD_MASK },
    0,
    { RNR_QUAD_DEBUG_DDR_HDR_FIFO_DATA2_DATA_FIELD_WIDTH },
    { RNR_QUAD_DEBUG_DDR_HDR_FIFO_DATA2_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_DEBUG_DDR_HDR_FIFO_DATA2_FIELDS[] =
{
    &RNR_QUAD_DEBUG_DDR_HDR_FIFO_DATA2_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_DEBUG_DDR_HDR_FIFO_DATA2 *****/
const ru_reg_rec RNR_QUAD_DEBUG_DDR_HDR_FIFO_DATA2_REG =
{
    "DEBUG_DDR_HDR_FIFO_DATA2",
#if RU_INCLUDE_DESC
    "DDR_HDR_FIFO_DATA2 Register",
    "Read contents of FIFO memory\n",
#endif
    { RNR_QUAD_DEBUG_DDR_HDR_FIFO_DATA2_REG_OFFSET },
    0,
    0,
    906,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_DEBUG_DDR_HDR_FIFO_DATA2_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG, TYPE: Type_RCQ_COMMON_REGS_RCQ_GENERAL_CONFIG_DMA_ARB_CFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: USE_FIFO_FOR_DDR_ONLY *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_USE_FIFO_FOR_DDR_ONLY_FIELD =
{
    "USE_FIFO_FOR_DDR_ONLY",
#if RU_INCLUDE_DESC
    "",
    "Select whether to use DDR FIFO only for DDR accesses\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_USE_FIFO_FOR_DDR_ONLY_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_USE_FIFO_FOR_DDR_ONLY_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_USE_FIFO_FOR_DDR_ONLY_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TOKEN_ARBITER_IS_RR *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_TOKEN_ARBITER_IS_RR_FIELD =
{
    "TOKEN_ARBITER_IS_RR",
#if RU_INCLUDE_DESC
    "",
    "Scheduling policy for token arbiter\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_TOKEN_ARBITER_IS_RR_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_TOKEN_ARBITER_IS_RR_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_TOKEN_ARBITER_IS_RR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CHICKEN_NO_FLOWCTRL *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_CHICKEN_NO_FLOWCTRL_FIELD =
{
    "CHICKEN_NO_FLOWCTRL",
#if RU_INCLUDE_DESC
    "",
    "chicken bit to disable external flow control. Packetw wil always be sent, no matter what token count says\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_CHICKEN_NO_FLOWCTRL_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_CHICKEN_NO_FLOWCTRL_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_CHICKEN_NO_FLOWCTRL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FLOW_CTRL_CLEAR_TOKEN *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_FLOW_CTRL_CLEAR_TOKEN_FIELD =
{
    "FLOW_CTRL_CLEAR_TOKEN",
#if RU_INCLUDE_DESC
    "",
    "Clear token count of external flow control block\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_FLOW_CTRL_CLEAR_TOKEN_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_FLOW_CTRL_CLEAR_TOKEN_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_FLOW_CTRL_CLEAR_TOKEN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_CONGEST_THRESHOLD *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_DDR_CONGEST_THRESHOLD_FIELD =
{
    "DDR_CONGEST_THRESHOLD",
#if RU_INCLUDE_DESC
    "",
    "Set DDR congestion threshold\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_DDR_CONGEST_THRESHOLD_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_DDR_CONGEST_THRESHOLD_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_DDR_CONGEST_THRESHOLD_FIELD_SHIFT },
    8,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PSRAM_CONGEST_THRESHOLD *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_PSRAM_CONGEST_THRESHOLD_FIELD =
{
    "PSRAM_CONGEST_THRESHOLD",
#if RU_INCLUDE_DESC
    "",
    "Set PSRAM congestion threshold\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_PSRAM_CONGEST_THRESHOLD_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_PSRAM_CONGEST_THRESHOLD_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_PSRAM_CONGEST_THRESHOLD_FIELD_SHIFT },
    7,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ENABLE_REPLY_THRESHOLD *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_ENABLE_REPLY_THRESHOLD_FIELD =
{
    "ENABLE_REPLY_THRESHOLD",
#if RU_INCLUDE_DESC
    "",
    "Enable reply FIFO occupancy threshold mechanism\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_ENABLE_REPLY_THRESHOLD_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_ENABLE_REPLY_THRESHOLD_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_ENABLE_REPLY_THRESHOLD_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_REPLY_THRESHOLD *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_DDR_REPLY_THRESHOLD_FIELD =
{
    "DDR_REPLY_THRESHOLD",
#if RU_INCLUDE_DESC
    "",
    "Set max reply FIFO occupancy for DDR transactions\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_DDR_REPLY_THRESHOLD_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_DDR_REPLY_THRESHOLD_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_DDR_REPLY_THRESHOLD_FIELD_SHIFT },
    32,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PSRAM_REPLY_THRESHOLD *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_PSRAM_REPLY_THRESHOLD_FIELD =
{
    "PSRAM_REPLY_THRESHOLD",
#if RU_INCLUDE_DESC
    "",
    "Set max reply FIFO occupancy for PSRAM transactions\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_PSRAM_REPLY_THRESHOLD_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_PSRAM_REPLY_THRESHOLD_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_PSRAM_REPLY_THRESHOLD_FIELD_SHIFT },
    32,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_FIELDS[] =
{
    &RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_USE_FIFO_FOR_DDR_ONLY_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_TOKEN_ARBITER_IS_RR_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_CHICKEN_NO_FLOWCTRL_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_FLOW_CTRL_CLEAR_TOKEN_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_DDR_CONGEST_THRESHOLD_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_PSRAM_CONGEST_THRESHOLD_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_ENABLE_REPLY_THRESHOLD_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_DDR_REPLY_THRESHOLD_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_PSRAM_REPLY_THRESHOLD_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG *****/
const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_REG =
{
    "GENERAL_CONFIG_DMA_ARB_CFG",
#if RU_INCLUDE_DESC
    "DMA_ARB_CFG Register",
    "DMA arbiter Configuration\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_REG_OFFSET },
    0,
    0,
    907,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_GENERAL_CONFIG_PSRAM0_BASE, TYPE: Type_RCQ_COMMON_REGS_RCQ_GENERAL_CONFIG_PSRAM0_BASE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_PSRAM0_BASE_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "Value for base/mask\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_PSRAM0_BASE_VAL_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_PSRAM0_BASE_VAL_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_PSRAM0_BASE_VAL_FIELD_SHIFT },
    8519680,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_GENERAL_CONFIG_PSRAM0_BASE_FIELDS[] =
{
    &RNR_QUAD_GENERAL_CONFIG_PSRAM0_BASE_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_GENERAL_CONFIG_PSRAM0_BASE *****/
const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_PSRAM0_BASE_REG =
{
    "GENERAL_CONFIG_PSRAM0_BASE",
#if RU_INCLUDE_DESC
    "PSRAM0_BASE Register",
    "Configure PSRAM0 base\n\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_PSRAM0_BASE_REG_OFFSET },
    0,
    0,
    908,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_GENERAL_CONFIG_PSRAM0_BASE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_GENERAL_CONFIG_PSRAM1_BASE, TYPE: Type_RCQ_COMMON_REGS_RCQ_GENERAL_CONFIG_PSRAM1_BASE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_PSRAM1_BASE_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "Value for base/mask\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_PSRAM1_BASE_VAL_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_PSRAM1_BASE_VAL_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_PSRAM1_BASE_VAL_FIELD_SHIFT },
    8527872,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_GENERAL_CONFIG_PSRAM1_BASE_FIELDS[] =
{
    &RNR_QUAD_GENERAL_CONFIG_PSRAM1_BASE_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_GENERAL_CONFIG_PSRAM1_BASE *****/
const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_PSRAM1_BASE_REG =
{
    "GENERAL_CONFIG_PSRAM1_BASE",
#if RU_INCLUDE_DESC
    "PSRAM1_BASE Register",
    "Configure PSRAM1 base\n\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_PSRAM1_BASE_REG_OFFSET },
    0,
    0,
    909,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_GENERAL_CONFIG_PSRAM1_BASE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_GENERAL_CONFIG_PSRAM2_BASE, TYPE: Type_RCQ_COMMON_REGS_RCQ_GENERAL_CONFIG_PSRAM2_BASE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_PSRAM2_BASE_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "Value for base/mask\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_PSRAM2_BASE_VAL_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_PSRAM2_BASE_VAL_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_PSRAM2_BASE_VAL_FIELD_SHIFT },
    8536064,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_GENERAL_CONFIG_PSRAM2_BASE_FIELDS[] =
{
    &RNR_QUAD_GENERAL_CONFIG_PSRAM2_BASE_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_GENERAL_CONFIG_PSRAM2_BASE *****/
const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_PSRAM2_BASE_REG =
{
    "GENERAL_CONFIG_PSRAM2_BASE",
#if RU_INCLUDE_DESC
    "PSRAM2_BASE Register",
    "Configure PSRAM2 base\n\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_PSRAM2_BASE_REG_OFFSET },
    0,
    0,
    910,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_GENERAL_CONFIG_PSRAM2_BASE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_GENERAL_CONFIG_PSRAM3_BASE, TYPE: Type_RCQ_COMMON_REGS_RCQ_GENERAL_CONFIG_PSRAM3_BASE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_PSRAM3_BASE_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "Value for base/mask\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_PSRAM3_BASE_VAL_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_PSRAM3_BASE_VAL_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_PSRAM3_BASE_VAL_FIELD_SHIFT },
    8544256,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_GENERAL_CONFIG_PSRAM3_BASE_FIELDS[] =
{
    &RNR_QUAD_GENERAL_CONFIG_PSRAM3_BASE_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_GENERAL_CONFIG_PSRAM3_BASE *****/
const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_PSRAM3_BASE_REG =
{
    "GENERAL_CONFIG_PSRAM3_BASE",
#if RU_INCLUDE_DESC
    "PSRAM3_BASE Register",
    "Configure PSRAM3 base\n\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_PSRAM3_BASE_REG_OFFSET },
    0,
    0,
    911,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_GENERAL_CONFIG_PSRAM3_BASE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_GENERAL_CONFIG_DDR0_BASE, TYPE: Type_RCQ_COMMON_REGS_RCQ_GENERAL_CONFIG_DDR0_BASE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_DDR0_BASE_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "Value for base/mask\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_DDR0_BASE_VAL_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_DDR0_BASE_VAL_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_DDR0_BASE_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_GENERAL_CONFIG_DDR0_BASE_FIELDS[] =
{
    &RNR_QUAD_GENERAL_CONFIG_DDR0_BASE_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_GENERAL_CONFIG_DDR0_BASE *****/
const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_DDR0_BASE_REG =
{
    "GENERAL_CONFIG_DDR0_BASE",
#if RU_INCLUDE_DESC
    "DDR0_BASE Register",
    "Configure DDR0 base\n\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_DDR0_BASE_REG_OFFSET },
    0,
    0,
    912,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_GENERAL_CONFIG_DDR0_BASE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_GENERAL_CONFIG_DDR1_BASE, TYPE: Type_RCQ_COMMON_REGS_RCQ_GENERAL_CONFIG_DDR1_BASE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_DDR1_BASE_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "Value for base/mask\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_DDR1_BASE_VAL_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_DDR1_BASE_VAL_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_DDR1_BASE_VAL_FIELD_SHIFT },
    16777216,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_GENERAL_CONFIG_DDR1_BASE_FIELDS[] =
{
    &RNR_QUAD_GENERAL_CONFIG_DDR1_BASE_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_GENERAL_CONFIG_DDR1_BASE *****/
const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_DDR1_BASE_REG =
{
    "GENERAL_CONFIG_DDR1_BASE",
#if RU_INCLUDE_DESC
    "DDR1_BASE Register",
    "Configure DDR1 base\n\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_DDR1_BASE_REG_OFFSET },
    0,
    0,
    913,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_GENERAL_CONFIG_DDR1_BASE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_GENERAL_CONFIG_PSRAM0_MASK, TYPE: Type_RCQ_COMMON_REGS_RCQ_GENERAL_CONFIG_PSRAM0_MASK
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_PSRAM0_MASK_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "Value for base/mask\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_PSRAM0_MASK_VAL_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_PSRAM0_MASK_VAL_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_PSRAM0_MASK_VAL_FIELD_SHIFT },
    4294963200,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_GENERAL_CONFIG_PSRAM0_MASK_FIELDS[] =
{
    &RNR_QUAD_GENERAL_CONFIG_PSRAM0_MASK_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_GENERAL_CONFIG_PSRAM0_MASK *****/
const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_PSRAM0_MASK_REG =
{
    "GENERAL_CONFIG_PSRAM0_MASK",
#if RU_INCLUDE_DESC
    "PSRAM0_MASK Register",
    "Configure PSRAM0 mask\n\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_PSRAM0_MASK_REG_OFFSET },
    0,
    0,
    914,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_GENERAL_CONFIG_PSRAM0_MASK_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_GENERAL_CONFIG_PSRAM1_MASK, TYPE: Type_RCQ_COMMON_REGS_RCQ_GENERAL_CONFIG_PSRAM1_MASK
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_PSRAM1_MASK_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "Value for base/mask\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_PSRAM1_MASK_VAL_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_PSRAM1_MASK_VAL_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_PSRAM1_MASK_VAL_FIELD_SHIFT },
    4294963200,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_GENERAL_CONFIG_PSRAM1_MASK_FIELDS[] =
{
    &RNR_QUAD_GENERAL_CONFIG_PSRAM1_MASK_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_GENERAL_CONFIG_PSRAM1_MASK *****/
const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_PSRAM1_MASK_REG =
{
    "GENERAL_CONFIG_PSRAM1_MASK",
#if RU_INCLUDE_DESC
    "PSRAM1_MASK Register",
    "Configure PSRAM1 mask\n\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_PSRAM1_MASK_REG_OFFSET },
    0,
    0,
    915,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_GENERAL_CONFIG_PSRAM1_MASK_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_GENERAL_CONFIG_PSRAM2_MASK, TYPE: Type_RCQ_COMMON_REGS_RCQ_GENERAL_CONFIG_PSRAM2_MASK
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_PSRAM2_MASK_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "Value for base/mask\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_PSRAM2_MASK_VAL_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_PSRAM2_MASK_VAL_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_PSRAM2_MASK_VAL_FIELD_SHIFT },
    4294963200,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_GENERAL_CONFIG_PSRAM2_MASK_FIELDS[] =
{
    &RNR_QUAD_GENERAL_CONFIG_PSRAM2_MASK_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_GENERAL_CONFIG_PSRAM2_MASK *****/
const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_PSRAM2_MASK_REG =
{
    "GENERAL_CONFIG_PSRAM2_MASK",
#if RU_INCLUDE_DESC
    "PSRAM2_MASK Register",
    "Configure PSRAM2 mask\n\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_PSRAM2_MASK_REG_OFFSET },
    0,
    0,
    916,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_GENERAL_CONFIG_PSRAM2_MASK_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_GENERAL_CONFIG_PSRAM3_MASK, TYPE: Type_RCQ_COMMON_REGS_RCQ_GENERAL_CONFIG_PSRAM3_MASK
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_PSRAM3_MASK_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "Value for base/mask\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_PSRAM3_MASK_VAL_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_PSRAM3_MASK_VAL_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_PSRAM3_MASK_VAL_FIELD_SHIFT },
    4294963200,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_GENERAL_CONFIG_PSRAM3_MASK_FIELDS[] =
{
    &RNR_QUAD_GENERAL_CONFIG_PSRAM3_MASK_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_GENERAL_CONFIG_PSRAM3_MASK *****/
const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_PSRAM3_MASK_REG =
{
    "GENERAL_CONFIG_PSRAM3_MASK",
#if RU_INCLUDE_DESC
    "PSRAM3_MASK Register",
    "Configure PSRAM3 mask\n\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_PSRAM3_MASK_REG_OFFSET },
    0,
    0,
    917,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_GENERAL_CONFIG_PSRAM3_MASK_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_GENERAL_CONFIG_DDR0_MASK, TYPE: Type_RCQ_COMMON_REGS_RCQ_GENERAL_CONFIG_DDR0_MASK
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_DDR0_MASK_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "Value for base/mask\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_DDR0_MASK_VAL_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_DDR0_MASK_VAL_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_DDR0_MASK_VAL_FIELD_SHIFT },
    4286578688,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_GENERAL_CONFIG_DDR0_MASK_FIELDS[] =
{
    &RNR_QUAD_GENERAL_CONFIG_DDR0_MASK_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_GENERAL_CONFIG_DDR0_MASK *****/
const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_DDR0_MASK_REG =
{
    "GENERAL_CONFIG_DDR0_MASK",
#if RU_INCLUDE_DESC
    "DDR0_MASK Register",
    "Configure DDR0 mask\n\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_DDR0_MASK_REG_OFFSET },
    0,
    0,
    918,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_GENERAL_CONFIG_DDR0_MASK_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_GENERAL_CONFIG_DDR1_MASK, TYPE: Type_RCQ_COMMON_REGS_RCQ_GENERAL_CONFIG_DDR1_MASK
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_DDR1_MASK_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "Value for base/mask\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_DDR1_MASK_VAL_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_DDR1_MASK_VAL_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_DDR1_MASK_VAL_FIELD_SHIFT },
    4286578688,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_GENERAL_CONFIG_DDR1_MASK_FIELDS[] =
{
    &RNR_QUAD_GENERAL_CONFIG_DDR1_MASK_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_GENERAL_CONFIG_DDR1_MASK *****/
const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_DDR1_MASK_REG =
{
    "GENERAL_CONFIG_DDR1_MASK",
#if RU_INCLUDE_DESC
    "DDR1_MASK Register",
    "Configure DDR1 mask\n\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_DDR1_MASK_REG_OFFSET },
    0,
    0,
    919,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_GENERAL_CONFIG_DDR1_MASK_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG, TYPE: Type_RCQ_COMMON_REGS_RCQ_GENERAL_CONFIG_PROFILING_CONFIG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNTER_LSB_SEL *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_COUNTER_LSB_SEL_FIELD =
{
    "COUNTER_LSB_SEL",
#if RU_INCLUDE_DESC
    "",
    "Select which 12-bits from 32-bit counter value to be recorded by tracer\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_COUNTER_LSB_SEL_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_COUNTER_LSB_SEL_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_COUNTER_LSB_SEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ENABLE_TRACE_CORE_0 *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_0_FIELD =
{
    "ENABLE_TRACE_CORE_0",
#if RU_INCLUDE_DESC
    "",
    "Enable tracing for core 0\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_0_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_0_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ENABLE_TRACE_CORE_1 *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_1_FIELD =
{
    "ENABLE_TRACE_CORE_1",
#if RU_INCLUDE_DESC
    "",
    "Enable tracing for core 1\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_1_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_1_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ENABLE_TRACE_CORE_2 *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_2_FIELD =
{
    "ENABLE_TRACE_CORE_2",
#if RU_INCLUDE_DESC
    "",
    "Enable tracing for core 2\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_2_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_2_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_2_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ENABLE_TRACE_CORE_3 *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_3_FIELD =
{
    "ENABLE_TRACE_CORE_3",
#if RU_INCLUDE_DESC
    "",
    "Enable tracing for core 3\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_3_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_3_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_3_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ENABLE_TRACE_CORE_4 *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_4_FIELD =
{
    "ENABLE_TRACE_CORE_4",
#if RU_INCLUDE_DESC
    "",
    "Enable tracing for core 4\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_4_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_4_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_4_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ENABLE_TRACE_CORE_5 *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_5_FIELD =
{
    "ENABLE_TRACE_CORE_5",
#if RU_INCLUDE_DESC
    "",
    "Enable tracing for core 5\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_5_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_5_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_5_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ENABLE_TRACE_CORE_6 *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_6_FIELD =
{
    "ENABLE_TRACE_CORE_6",
#if RU_INCLUDE_DESC
    "",
    "Enable tracing for core 6\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_6_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_6_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_6_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ENABLE_TRACE_CORE_7 *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_7_FIELD =
{
    "ENABLE_TRACE_CORE_7",
#if RU_INCLUDE_DESC
    "",
    "Enable tracing for core 7\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_7_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_7_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_7_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ENABLE_TRACE_CORE_8 *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_8_FIELD =
{
    "ENABLE_TRACE_CORE_8",
#if RU_INCLUDE_DESC
    "",
    "Enable tracing for core 8\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_8_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_8_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_8_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ENABLE_TRACE_CORE_9 *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_9_FIELD =
{
    "ENABLE_TRACE_CORE_9",
#if RU_INCLUDE_DESC
    "",
    "Enable tracing for core 9\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_9_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_9_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_9_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ENABLE_TRACE_CORE_10 *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_10_FIELD =
{
    "ENABLE_TRACE_CORE_10",
#if RU_INCLUDE_DESC
    "",
    "Enable tracing for core 10\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_10_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_10_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_10_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ENABLE_TRACE_CORE_11 *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_11_FIELD =
{
    "ENABLE_TRACE_CORE_11",
#if RU_INCLUDE_DESC
    "",
    "Enable tracing for core 11\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_11_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_11_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_11_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ENABLE_TRACE_CORE_12 *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_12_FIELD =
{
    "ENABLE_TRACE_CORE_12",
#if RU_INCLUDE_DESC
    "",
    "Enable tracing for core 12\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_12_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_12_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_12_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ENABLE_TRACE_CORE_13 *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_13_FIELD =
{
    "ENABLE_TRACE_CORE_13",
#if RU_INCLUDE_DESC
    "",
    "Enable tracing for core 13\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_13_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_13_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_13_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_FIELDS[] =
{
    &RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_COUNTER_LSB_SEL_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_0_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_1_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_2_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_3_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_4_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_5_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_6_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_7_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_8_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_9_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_10_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_11_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_12_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_13_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG *****/
const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_REG =
{
    "GENERAL_CONFIG_PROFILING_CONFIG",
#if RU_INCLUDE_DESC
    "PROFILING_CONFIG Register",
    "Profiling configuration\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_REG_OFFSET },
    0,
    0,
    920,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    15,
    RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_GENERAL_CONFIG_BKPT_0_CFG, TYPE: Type_RCQ_COMMON_REGS_RCQ_GENERAL_CONFIG_BKPT_0_CFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ADDR *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_BKPT_0_CFG_ADDR_FIELD =
{
    "ADDR",
#if RU_INCLUDE_DESC
    "",
    "Breakpoint address\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_BKPT_0_CFG_ADDR_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_BKPT_0_CFG_ADDR_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_BKPT_0_CFG_ADDR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: THREAD *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_BKPT_0_CFG_THREAD_FIELD =
{
    "THREAD",
#if RU_INCLUDE_DESC
    "",
    "Breakpoint address\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_BKPT_0_CFG_THREAD_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_BKPT_0_CFG_THREAD_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_BKPT_0_CFG_THREAD_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_GENERAL_CONFIG_BKPT_0_CFG_FIELDS[] =
{
    &RNR_QUAD_GENERAL_CONFIG_BKPT_0_CFG_ADDR_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_BKPT_0_CFG_THREAD_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_GENERAL_CONFIG_BKPT_0_CFG *****/
const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_BKPT_0_CFG_REG =
{
    "GENERAL_CONFIG_BKPT_0_CFG",
#if RU_INCLUDE_DESC
    "BKPT_CFG_0 Register",
    "Breakpoint 0 configuration.\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_BKPT_0_CFG_REG_OFFSET },
    0,
    0,
    921,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    RNR_QUAD_GENERAL_CONFIG_BKPT_0_CFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_GENERAL_CONFIG_BKPT_1_CFG, TYPE: Type_RCQ_COMMON_REGS_RCQ_GENERAL_CONFIG_BKPT_1_CFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ADDR *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_BKPT_1_CFG_ADDR_FIELD =
{
    "ADDR",
#if RU_INCLUDE_DESC
    "",
    "Breakpoint address\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_BKPT_1_CFG_ADDR_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_BKPT_1_CFG_ADDR_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_BKPT_1_CFG_ADDR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: THREAD *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_BKPT_1_CFG_THREAD_FIELD =
{
    "THREAD",
#if RU_INCLUDE_DESC
    "",
    "Breakpoint address\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_BKPT_1_CFG_THREAD_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_BKPT_1_CFG_THREAD_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_BKPT_1_CFG_THREAD_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_GENERAL_CONFIG_BKPT_1_CFG_FIELDS[] =
{
    &RNR_QUAD_GENERAL_CONFIG_BKPT_1_CFG_ADDR_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_BKPT_1_CFG_THREAD_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_GENERAL_CONFIG_BKPT_1_CFG *****/
const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_BKPT_1_CFG_REG =
{
    "GENERAL_CONFIG_BKPT_1_CFG",
#if RU_INCLUDE_DESC
    "BKPT_CFG_1 Register",
    "Breakpoint 1 configuration.\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_BKPT_1_CFG_REG_OFFSET },
    0,
    0,
    922,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    RNR_QUAD_GENERAL_CONFIG_BKPT_1_CFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_GENERAL_CONFIG_BKPT_2_CFG, TYPE: Type_RCQ_COMMON_REGS_RCQ_GENERAL_CONFIG_BKPT_2_CFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ADDR *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_BKPT_2_CFG_ADDR_FIELD =
{
    "ADDR",
#if RU_INCLUDE_DESC
    "",
    "Breakpoint address\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_BKPT_2_CFG_ADDR_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_BKPT_2_CFG_ADDR_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_BKPT_2_CFG_ADDR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: THREAD *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_BKPT_2_CFG_THREAD_FIELD =
{
    "THREAD",
#if RU_INCLUDE_DESC
    "",
    "Breakpoint address\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_BKPT_2_CFG_THREAD_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_BKPT_2_CFG_THREAD_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_BKPT_2_CFG_THREAD_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_GENERAL_CONFIG_BKPT_2_CFG_FIELDS[] =
{
    &RNR_QUAD_GENERAL_CONFIG_BKPT_2_CFG_ADDR_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_BKPT_2_CFG_THREAD_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_GENERAL_CONFIG_BKPT_2_CFG *****/
const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_BKPT_2_CFG_REG =
{
    "GENERAL_CONFIG_BKPT_2_CFG",
#if RU_INCLUDE_DESC
    "BKPT_CFG_2 Register",
    "Breakpoint 2 configuration.\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_BKPT_2_CFG_REG_OFFSET },
    0,
    0,
    923,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    RNR_QUAD_GENERAL_CONFIG_BKPT_2_CFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_GENERAL_CONFIG_BKPT_3_CFG, TYPE: Type_RCQ_COMMON_REGS_RCQ_GENERAL_CONFIG_BKPT_3_CFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ADDR *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_BKPT_3_CFG_ADDR_FIELD =
{
    "ADDR",
#if RU_INCLUDE_DESC
    "",
    "Breakpoint address\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_BKPT_3_CFG_ADDR_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_BKPT_3_CFG_ADDR_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_BKPT_3_CFG_ADDR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: THREAD *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_BKPT_3_CFG_THREAD_FIELD =
{
    "THREAD",
#if RU_INCLUDE_DESC
    "",
    "Breakpoint address\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_BKPT_3_CFG_THREAD_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_BKPT_3_CFG_THREAD_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_BKPT_3_CFG_THREAD_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_GENERAL_CONFIG_BKPT_3_CFG_FIELDS[] =
{
    &RNR_QUAD_GENERAL_CONFIG_BKPT_3_CFG_ADDR_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_BKPT_3_CFG_THREAD_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_GENERAL_CONFIG_BKPT_3_CFG *****/
const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_BKPT_3_CFG_REG =
{
    "GENERAL_CONFIG_BKPT_3_CFG",
#if RU_INCLUDE_DESC
    "BKPT_CFG_3 Register",
    "Breakpoint 3 configuration.\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_BKPT_3_CFG_REG_OFFSET },
    0,
    0,
    924,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    RNR_QUAD_GENERAL_CONFIG_BKPT_3_CFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_GENERAL_CONFIG_BKPT_4_CFG, TYPE: Type_RCQ_COMMON_REGS_RCQ_GENERAL_CONFIG_BKPT_4_CFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ADDR *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_BKPT_4_CFG_ADDR_FIELD =
{
    "ADDR",
#if RU_INCLUDE_DESC
    "",
    "Breakpoint address\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_BKPT_4_CFG_ADDR_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_BKPT_4_CFG_ADDR_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_BKPT_4_CFG_ADDR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: THREAD *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_BKPT_4_CFG_THREAD_FIELD =
{
    "THREAD",
#if RU_INCLUDE_DESC
    "",
    "Breakpoint address\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_BKPT_4_CFG_THREAD_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_BKPT_4_CFG_THREAD_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_BKPT_4_CFG_THREAD_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_GENERAL_CONFIG_BKPT_4_CFG_FIELDS[] =
{
    &RNR_QUAD_GENERAL_CONFIG_BKPT_4_CFG_ADDR_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_BKPT_4_CFG_THREAD_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_GENERAL_CONFIG_BKPT_4_CFG *****/
const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_BKPT_4_CFG_REG =
{
    "GENERAL_CONFIG_BKPT_4_CFG",
#if RU_INCLUDE_DESC
    "BKPT_CFG_4 Register",
    "Breakpoint 4 configuration.\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_BKPT_4_CFG_REG_OFFSET },
    0,
    0,
    925,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    RNR_QUAD_GENERAL_CONFIG_BKPT_4_CFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_GENERAL_CONFIG_BKPT_5_CFG, TYPE: Type_RCQ_COMMON_REGS_RCQ_GENERAL_CONFIG_BKPT_5_CFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ADDR *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_BKPT_5_CFG_ADDR_FIELD =
{
    "ADDR",
#if RU_INCLUDE_DESC
    "",
    "Breakpoint address\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_BKPT_5_CFG_ADDR_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_BKPT_5_CFG_ADDR_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_BKPT_5_CFG_ADDR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: THREAD *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_BKPT_5_CFG_THREAD_FIELD =
{
    "THREAD",
#if RU_INCLUDE_DESC
    "",
    "Breakpoint address\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_BKPT_5_CFG_THREAD_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_BKPT_5_CFG_THREAD_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_BKPT_5_CFG_THREAD_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_GENERAL_CONFIG_BKPT_5_CFG_FIELDS[] =
{
    &RNR_QUAD_GENERAL_CONFIG_BKPT_5_CFG_ADDR_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_BKPT_5_CFG_THREAD_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_GENERAL_CONFIG_BKPT_5_CFG *****/
const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_BKPT_5_CFG_REG =
{
    "GENERAL_CONFIG_BKPT_5_CFG",
#if RU_INCLUDE_DESC
    "BKPT_CFG_5 Register",
    "Breakpoint 5 configuration.\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_BKPT_5_CFG_REG_OFFSET },
    0,
    0,
    926,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    RNR_QUAD_GENERAL_CONFIG_BKPT_5_CFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_GENERAL_CONFIG_BKPT_6_CFG, TYPE: Type_RCQ_COMMON_REGS_RCQ_GENERAL_CONFIG_BKPT_6_CFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ADDR *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_BKPT_6_CFG_ADDR_FIELD =
{
    "ADDR",
#if RU_INCLUDE_DESC
    "",
    "Breakpoint address\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_BKPT_6_CFG_ADDR_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_BKPT_6_CFG_ADDR_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_BKPT_6_CFG_ADDR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: THREAD *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_BKPT_6_CFG_THREAD_FIELD =
{
    "THREAD",
#if RU_INCLUDE_DESC
    "",
    "Breakpoint address\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_BKPT_6_CFG_THREAD_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_BKPT_6_CFG_THREAD_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_BKPT_6_CFG_THREAD_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_GENERAL_CONFIG_BKPT_6_CFG_FIELDS[] =
{
    &RNR_QUAD_GENERAL_CONFIG_BKPT_6_CFG_ADDR_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_BKPT_6_CFG_THREAD_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_GENERAL_CONFIG_BKPT_6_CFG *****/
const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_BKPT_6_CFG_REG =
{
    "GENERAL_CONFIG_BKPT_6_CFG",
#if RU_INCLUDE_DESC
    "BKPT_CFG_6 Register",
    "Breakpoint 6 configuration.\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_BKPT_6_CFG_REG_OFFSET },
    0,
    0,
    927,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    RNR_QUAD_GENERAL_CONFIG_BKPT_6_CFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_GENERAL_CONFIG_BKPT_7_CFG, TYPE: Type_RCQ_COMMON_REGS_RCQ_GENERAL_CONFIG_BKPT_7_CFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ADDR *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_BKPT_7_CFG_ADDR_FIELD =
{
    "ADDR",
#if RU_INCLUDE_DESC
    "",
    "Breakpoint address\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_BKPT_7_CFG_ADDR_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_BKPT_7_CFG_ADDR_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_BKPT_7_CFG_ADDR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: THREAD *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_BKPT_7_CFG_THREAD_FIELD =
{
    "THREAD",
#if RU_INCLUDE_DESC
    "",
    "Breakpoint address\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_BKPT_7_CFG_THREAD_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_BKPT_7_CFG_THREAD_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_BKPT_7_CFG_THREAD_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_GENERAL_CONFIG_BKPT_7_CFG_FIELDS[] =
{
    &RNR_QUAD_GENERAL_CONFIG_BKPT_7_CFG_ADDR_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_BKPT_7_CFG_THREAD_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_GENERAL_CONFIG_BKPT_7_CFG *****/
const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_BKPT_7_CFG_REG =
{
    "GENERAL_CONFIG_BKPT_7_CFG",
#if RU_INCLUDE_DESC
    "BKPT_CFG_7 Register",
    "Breakpoint 7 configuration.\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_BKPT_7_CFG_REG_OFFSET },
    0,
    0,
    928,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    RNR_QUAD_GENERAL_CONFIG_BKPT_7_CFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_GENERAL_CONFIG_BKPT_GEN_CFG, TYPE: Type_RCQ_COMMON_REGS_RCQ_GENERAL_CONFIG_BKPT_GEN_CFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: HANDLER_ADDR *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_BKPT_GEN_CFG_HANDLER_ADDR_FIELD =
{
    "HANDLER_ADDR",
#if RU_INCLUDE_DESC
    "",
    "Breakpoint handler routine address\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_BKPT_GEN_CFG_HANDLER_ADDR_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_BKPT_GEN_CFG_HANDLER_ADDR_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_BKPT_GEN_CFG_HANDLER_ADDR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: UPDATE_PC_VALUE *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_BKPT_GEN_CFG_UPDATE_PC_VALUE_FIELD =
{
    "UPDATE_PC_VALUE",
#if RU_INCLUDE_DESC
    "",
    "New PC to be updated by breakpoint handler routine\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_BKPT_GEN_CFG_UPDATE_PC_VALUE_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_BKPT_GEN_CFG_UPDATE_PC_VALUE_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_BKPT_GEN_CFG_UPDATE_PC_VALUE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_GENERAL_CONFIG_BKPT_GEN_CFG_FIELDS[] =
{
    &RNR_QUAD_GENERAL_CONFIG_BKPT_GEN_CFG_HANDLER_ADDR_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_BKPT_GEN_CFG_UPDATE_PC_VALUE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_GENERAL_CONFIG_BKPT_GEN_CFG *****/
const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_BKPT_GEN_CFG_REG =
{
    "GENERAL_CONFIG_BKPT_GEN_CFG",
#if RU_INCLUDE_DESC
    "BKPT_CFG_GEN Register",
    "Breakpoint general configuration.\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_BKPT_GEN_CFG_REG_OFFSET },
    0,
    0,
    929,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    RNR_QUAD_GENERAL_CONFIG_BKPT_GEN_CFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG, TYPE: Type_RCQ_COMMON_REGS_RCQ_GENERAL_CONFIG_POWERSAVE_CONFIG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: TIME_COUNTER *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_TIME_COUNTER_FIELD =
{
    "TIME_COUNTER",
#if RU_INCLUDE_DESC
    "",
    "Select how many clocks to wait in IDLE condition before enetrin powersave state\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_TIME_COUNTER_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_TIME_COUNTER_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_TIME_COUNTER_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ENABLE_POWERSAVE_CORE_0 *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_0_FIELD =
{
    "ENABLE_POWERSAVE_CORE_0",
#if RU_INCLUDE_DESC
    "",
    "Enable powersavingfor core 0\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_0_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_0_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ENABLE_POWERSAVE_CORE_1 *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_1_FIELD =
{
    "ENABLE_POWERSAVE_CORE_1",
#if RU_INCLUDE_DESC
    "",
    "Enable powersave for core 1\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_1_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_1_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ENABLE_POWERSAVE_CORE_2 *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_2_FIELD =
{
    "ENABLE_POWERSAVE_CORE_2",
#if RU_INCLUDE_DESC
    "",
    "Enable powersave for core 2\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_2_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_2_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_2_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ENABLE_POWERSAVE_CORE_3 *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_3_FIELD =
{
    "ENABLE_POWERSAVE_CORE_3",
#if RU_INCLUDE_DESC
    "",
    "Enable powersave for core 3\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_3_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_3_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_3_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ENABLE_POWERSAVE_CORE_4 *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_4_FIELD =
{
    "ENABLE_POWERSAVE_CORE_4",
#if RU_INCLUDE_DESC
    "",
    "Enable powersave for core 4\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_4_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_4_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_4_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ENABLE_POWERSAVE_CORE_5 *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_5_FIELD =
{
    "ENABLE_POWERSAVE_CORE_5",
#if RU_INCLUDE_DESC
    "",
    "Enable powersave for core 5\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_5_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_5_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_5_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ENABLE_POWERSAVE_CORE_6 *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_6_FIELD =
{
    "ENABLE_POWERSAVE_CORE_6",
#if RU_INCLUDE_DESC
    "",
    "Enable powersave for core 6\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_6_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_6_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_6_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ENABLE_POWERSAVE_CORE_7 *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_7_FIELD =
{
    "ENABLE_POWERSAVE_CORE_7",
#if RU_INCLUDE_DESC
    "",
    "Enable powersave for core 7\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_7_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_7_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_7_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ENABLE_POWERSAVE_CORE_8 *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_8_FIELD =
{
    "ENABLE_POWERSAVE_CORE_8",
#if RU_INCLUDE_DESC
    "",
    "Enable powersave for core 8\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_8_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_8_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_8_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ENABLE_POWERSAVE_CORE_9 *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_9_FIELD =
{
    "ENABLE_POWERSAVE_CORE_9",
#if RU_INCLUDE_DESC
    "",
    "Enable powersave for core 9\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_9_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_9_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_9_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ENABLE_POWERSAVE_CORE_10 *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_10_FIELD =
{
    "ENABLE_POWERSAVE_CORE_10",
#if RU_INCLUDE_DESC
    "",
    "Enable powersave for core 10\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_10_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_10_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_10_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ENABLE_POWERSAVE_CORE_11 *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_11_FIELD =
{
    "ENABLE_POWERSAVE_CORE_11",
#if RU_INCLUDE_DESC
    "",
    "Enable powersave for core 11\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_11_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_11_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_11_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ENABLE_POWERSAVE_CORE_12 *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_12_FIELD =
{
    "ENABLE_POWERSAVE_CORE_12",
#if RU_INCLUDE_DESC
    "",
    "Enable powersave for core 12\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_12_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_12_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_12_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ENABLE_POWERSAVE_CORE_13 *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_13_FIELD =
{
    "ENABLE_POWERSAVE_CORE_13",
#if RU_INCLUDE_DESC
    "",
    "Enable powersave for core 13\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_13_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_13_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_13_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ENABLE_CPU_IF_CLK_GATING *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_CPU_IF_CLK_GATING_FIELD =
{
    "ENABLE_CPU_IF_CLK_GATING",
#if RU_INCLUDE_DESC
    "",
    "Enable ENABLE_CPU_IF_CLK_GATING (for all cores)\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_CPU_IF_CLK_GATING_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_CPU_IF_CLK_GATING_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_CPU_IF_CLK_GATING_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ENABLE_COMMON_REG_CLK_GATING *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_COMMON_REG_CLK_GATING_FIELD =
{
    "ENABLE_COMMON_REG_CLK_GATING",
#if RU_INCLUDE_DESC
    "",
    "Enable COMMON_REG block clock gating\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_COMMON_REG_CLK_GATING_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_COMMON_REG_CLK_GATING_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_COMMON_REG_CLK_GATING_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ENABLE_EC_BLOCKS_CLK_GATING *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_EC_BLOCKS_CLK_GATING_FIELD =
{
    "ENABLE_EC_BLOCKS_CLK_GATING",
#if RU_INCLUDE_DESC
    "",
    "Enable clock gating for EC arbiter and dispatcher\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_EC_BLOCKS_CLK_GATING_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_EC_BLOCKS_CLK_GATING_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_EC_BLOCKS_CLK_GATING_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_FIELDS[] =
{
    &RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_TIME_COUNTER_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_0_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_1_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_2_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_3_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_4_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_5_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_6_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_7_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_8_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_9_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_10_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_11_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_12_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_13_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_CPU_IF_CLK_GATING_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_COMMON_REG_CLK_GATING_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_EC_BLOCKS_CLK_GATING_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG *****/
const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_REG =
{
    "GENERAL_CONFIG_POWERSAVE_CONFIG",
#if RU_INCLUDE_DESC
    "POWERSAVE_CONFIG Register",
    "Powersaving  configuration\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_REG_OFFSET },
    0,
    0,
    930,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    18,
    RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS, TYPE: Type_RCQ_COMMON_REGS_RCQ_GENERAL_CONFIG_POWERSAVE_STATUS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ACC_STATUS_0 *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_ACC_STATUS_0_FIELD =
{
    "ACC_STATUS_0",
#if RU_INCLUDE_DESC
    "",
    "Runner 0 accelerators powersaving status\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_ACC_STATUS_0_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_ACC_STATUS_0_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_ACC_STATUS_0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ACC_STATUS_1 *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_ACC_STATUS_1_FIELD =
{
    "ACC_STATUS_1",
#if RU_INCLUDE_DESC
    "",
    "Runner 1 accelerators powersaving status\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_ACC_STATUS_1_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_ACC_STATUS_1_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_ACC_STATUS_1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ACC_STATUS_2 *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_ACC_STATUS_2_FIELD =
{
    "ACC_STATUS_2",
#if RU_INCLUDE_DESC
    "",
    "Runner 2 accelerators powersaving status\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_ACC_STATUS_2_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_ACC_STATUS_2_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_ACC_STATUS_2_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ACC_STATUS_3 *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_ACC_STATUS_3_FIELD =
{
    "ACC_STATUS_3",
#if RU_INCLUDE_DESC
    "",
    "Runner 3 accelerators powersaving status\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_ACC_STATUS_3_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_ACC_STATUS_3_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_ACC_STATUS_3_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ACC_STATUS_4 *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_ACC_STATUS_4_FIELD =
{
    "ACC_STATUS_4",
#if RU_INCLUDE_DESC
    "",
    "Runner 4 accelerators powersaving status\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_ACC_STATUS_4_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_ACC_STATUS_4_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_ACC_STATUS_4_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ACC_STATUS_5 *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_ACC_STATUS_5_FIELD =
{
    "ACC_STATUS_5",
#if RU_INCLUDE_DESC
    "",
    "Runner 5 accelerators powersaving status\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_ACC_STATUS_5_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_ACC_STATUS_5_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_ACC_STATUS_5_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ACC_STATUS_6 *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_ACC_STATUS_6_FIELD =
{
    "ACC_STATUS_6",
#if RU_INCLUDE_DESC
    "",
    "Runner 6 accelerators powersaving status\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_ACC_STATUS_6_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_ACC_STATUS_6_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_ACC_STATUS_6_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ACC_STATUS_7 *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_ACC_STATUS_7_FIELD =
{
    "ACC_STATUS_7",
#if RU_INCLUDE_DESC
    "",
    "Runner 7 accelerators powersaving status\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_ACC_STATUS_7_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_ACC_STATUS_7_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_ACC_STATUS_7_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ACC_STATUS_8 *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_ACC_STATUS_8_FIELD =
{
    "ACC_STATUS_8",
#if RU_INCLUDE_DESC
    "",
    "Runner 8 accelerators powersaving status\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_ACC_STATUS_8_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_ACC_STATUS_8_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_ACC_STATUS_8_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ACC_STATUS_9 *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_ACC_STATUS_9_FIELD =
{
    "ACC_STATUS_9",
#if RU_INCLUDE_DESC
    "",
    "Runner 9 accelerators powersaving status\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_ACC_STATUS_9_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_ACC_STATUS_9_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_ACC_STATUS_9_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ACC_STATUS_10 *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_ACC_STATUS_10_FIELD =
{
    "ACC_STATUS_10",
#if RU_INCLUDE_DESC
    "",
    "Runner 10 accelerators powersaving status\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_ACC_STATUS_10_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_ACC_STATUS_10_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_ACC_STATUS_10_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ACC_STATUS_11 *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_ACC_STATUS_11_FIELD =
{
    "ACC_STATUS_11",
#if RU_INCLUDE_DESC
    "",
    "Runner 11 accelerators powersaving status\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_ACC_STATUS_11_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_ACC_STATUS_11_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_ACC_STATUS_11_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ACC_STATUS_12 *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_ACC_STATUS_12_FIELD =
{
    "ACC_STATUS_12",
#if RU_INCLUDE_DESC
    "",
    "Runner 12 accelerators powersaving status\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_ACC_STATUS_12_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_ACC_STATUS_12_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_ACC_STATUS_12_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ACC_STATUS_13 *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_ACC_STATUS_13_FIELD =
{
    "ACC_STATUS_13",
#if RU_INCLUDE_DESC
    "",
    "Runner 13 accelerators powersaving status\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_ACC_STATUS_13_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_ACC_STATUS_13_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_ACC_STATUS_13_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CORE_STATUS_0 *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_STATUS_0_FIELD =
{
    "CORE_STATUS_0",
#if RU_INCLUDE_DESC
    "",
    "Runner 0 core powersaving status\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_STATUS_0_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_STATUS_0_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_STATUS_0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CORE_STATUS_1 *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_STATUS_1_FIELD =
{
    "CORE_STATUS_1",
#if RU_INCLUDE_DESC
    "",
    "Runner 1 core powersaving status\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_STATUS_1_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_STATUS_1_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_STATUS_1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CORE_STATUS_2 *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_STATUS_2_FIELD =
{
    "CORE_STATUS_2",
#if RU_INCLUDE_DESC
    "",
    "Runner 2 core powersaving status\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_STATUS_2_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_STATUS_2_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_STATUS_2_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CORE_STATUS_3 *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_STATUS_3_FIELD =
{
    "CORE_STATUS_3",
#if RU_INCLUDE_DESC
    "",
    "Runner 3 core powersaving status\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_STATUS_3_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_STATUS_3_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_STATUS_3_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CORE_STATUS_4 *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_STATUS_4_FIELD =
{
    "CORE_STATUS_4",
#if RU_INCLUDE_DESC
    "",
    "Runner 4 core powersaving status\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_STATUS_4_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_STATUS_4_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_STATUS_4_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CORE_STATUS_5 *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_STATUS_5_FIELD =
{
    "CORE_STATUS_5",
#if RU_INCLUDE_DESC
    "",
    "Runner 5 core powersaving status\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_STATUS_5_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_STATUS_5_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_STATUS_5_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CORE_STATUS_6 *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_STATUS_6_FIELD =
{
    "CORE_STATUS_6",
#if RU_INCLUDE_DESC
    "",
    "Runner 6 core powersaving status\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_STATUS_6_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_STATUS_6_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_STATUS_6_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CORE_STATUS_7 *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_STATUS_7_FIELD =
{
    "CORE_STATUS_7",
#if RU_INCLUDE_DESC
    "",
    "Runner 7 core powersaving status\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_STATUS_7_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_STATUS_7_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_STATUS_7_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CORE_STATUS_8 *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_STATUS_8_FIELD =
{
    "CORE_STATUS_8",
#if RU_INCLUDE_DESC
    "",
    "Runner 8 core powersaving status\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_STATUS_8_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_STATUS_8_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_STATUS_8_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CORE_STATUS_9 *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_STATUS_9_FIELD =
{
    "CORE_STATUS_9",
#if RU_INCLUDE_DESC
    "",
    "Runner 9 core powersaving status\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_STATUS_9_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_STATUS_9_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_STATUS_9_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CORE_STATUS_10 *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_STATUS_10_FIELD =
{
    "CORE_STATUS_10",
#if RU_INCLUDE_DESC
    "",
    "Runner 10 core powersaving status\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_STATUS_10_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_STATUS_10_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_STATUS_10_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CORE_STATUS_11 *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_STATUS_11_FIELD =
{
    "CORE_STATUS_11",
#if RU_INCLUDE_DESC
    "",
    "Runner 11 core powersaving status\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_STATUS_11_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_STATUS_11_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_STATUS_11_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CORE_STATUS_12 *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_STATUS_12_FIELD =
{
    "CORE_STATUS_12",
#if RU_INCLUDE_DESC
    "",
    "Runner 12 core powersaving status\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_STATUS_12_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_STATUS_12_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_STATUS_12_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CORE_STATUS_13 *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_STATUS_13_FIELD =
{
    "CORE_STATUS_13",
#if RU_INCLUDE_DESC
    "",
    "Runner 13 core powersaving status\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_STATUS_13_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_STATUS_13_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_STATUS_13_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_FIELDS[] =
{
    &RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_ACC_STATUS_0_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_ACC_STATUS_1_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_ACC_STATUS_2_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_ACC_STATUS_3_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_ACC_STATUS_4_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_ACC_STATUS_5_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_ACC_STATUS_6_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_ACC_STATUS_7_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_ACC_STATUS_8_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_ACC_STATUS_9_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_ACC_STATUS_10_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_ACC_STATUS_11_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_ACC_STATUS_12_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_ACC_STATUS_13_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_STATUS_0_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_STATUS_1_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_STATUS_2_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_STATUS_3_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_STATUS_4_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_STATUS_5_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_STATUS_6_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_STATUS_7_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_STATUS_8_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_STATUS_9_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_STATUS_10_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_STATUS_11_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_STATUS_12_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_STATUS_13_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS *****/
const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_REG =
{
    "GENERAL_CONFIG_POWERSAVE_STATUS",
#if RU_INCLUDE_DESC
    "POWERSAVE_STATUS Register",
    "Powersave status indications\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_REG_OFFSET },
    0,
    0,
    931,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    28,
    RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_0_CFG, TYPE: Type_RCQ_COMMON_REGS_RCQ_GENERAL_CONFIG_DATA_BKPT_0_CFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA_ADDR_START *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_0_CFG_DATA_ADDR_START_FIELD =
{
    "DATA_ADDR_START",
#if RU_INCLUDE_DESC
    "",
    "Data address start.\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_0_CFG_DATA_ADDR_START_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_0_CFG_DATA_ADDR_START_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_0_CFG_DATA_ADDR_START_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA_ADDR_STOP *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_0_CFG_DATA_ADDR_STOP_FIELD =
{
    "DATA_ADDR_STOP",
#if RU_INCLUDE_DESC
    "",
    "Data address stop\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_0_CFG_DATA_ADDR_STOP_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_0_CFG_DATA_ADDR_STOP_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_0_CFG_DATA_ADDR_STOP_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_0_CFG_FIELDS[] =
{
    &RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_0_CFG_DATA_ADDR_START_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_0_CFG_DATA_ADDR_STOP_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_0_CFG *****/
const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_0_CFG_REG =
{
    "GENERAL_CONFIG_DATA_BKPT_0_CFG",
#if RU_INCLUDE_DESC
    "DATA_BKPT_CFG_0 Register",
    "DATA Breakpoint 0 configuration.\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_0_CFG_REG_OFFSET },
    0,
    0,
    932,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_0_CFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_1_CFG, TYPE: Type_RCQ_COMMON_REGS_RCQ_GENERAL_CONFIG_DATA_BKPT_1_CFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA_ADDR_START *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_1_CFG_DATA_ADDR_START_FIELD =
{
    "DATA_ADDR_START",
#if RU_INCLUDE_DESC
    "",
    "Data address start.\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_1_CFG_DATA_ADDR_START_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_1_CFG_DATA_ADDR_START_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_1_CFG_DATA_ADDR_START_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA_ADDR_STOP *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_1_CFG_DATA_ADDR_STOP_FIELD =
{
    "DATA_ADDR_STOP",
#if RU_INCLUDE_DESC
    "",
    "Data address stop\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_1_CFG_DATA_ADDR_STOP_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_1_CFG_DATA_ADDR_STOP_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_1_CFG_DATA_ADDR_STOP_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_1_CFG_FIELDS[] =
{
    &RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_1_CFG_DATA_ADDR_START_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_1_CFG_DATA_ADDR_STOP_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_1_CFG *****/
const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_1_CFG_REG =
{
    "GENERAL_CONFIG_DATA_BKPT_1_CFG",
#if RU_INCLUDE_DESC
    "DATA_BKPT_CFG_1 Register",
    "DATA Breakpoint 1 configuration.\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_1_CFG_REG_OFFSET },
    0,
    0,
    933,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_1_CFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_2_CFG, TYPE: Type_RCQ_COMMON_REGS_RCQ_GENERAL_CONFIG_DATA_BKPT_2_CFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA_ADDR_START *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_2_CFG_DATA_ADDR_START_FIELD =
{
    "DATA_ADDR_START",
#if RU_INCLUDE_DESC
    "",
    "Data address start.\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_2_CFG_DATA_ADDR_START_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_2_CFG_DATA_ADDR_START_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_2_CFG_DATA_ADDR_START_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA_ADDR_STOP *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_2_CFG_DATA_ADDR_STOP_FIELD =
{
    "DATA_ADDR_STOP",
#if RU_INCLUDE_DESC
    "",
    "Data address stop\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_2_CFG_DATA_ADDR_STOP_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_2_CFG_DATA_ADDR_STOP_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_2_CFG_DATA_ADDR_STOP_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_2_CFG_FIELDS[] =
{
    &RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_2_CFG_DATA_ADDR_START_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_2_CFG_DATA_ADDR_STOP_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_2_CFG *****/
const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_2_CFG_REG =
{
    "GENERAL_CONFIG_DATA_BKPT_2_CFG",
#if RU_INCLUDE_DESC
    "DATA_BKPT_CFG_2 Register",
    "DATA Breakpoint 2 configuration.\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_2_CFG_REG_OFFSET },
    0,
    0,
    934,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_2_CFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_3_CFG, TYPE: Type_RCQ_COMMON_REGS_RCQ_GENERAL_CONFIG_DATA_BKPT_3_CFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA_ADDR_START *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_3_CFG_DATA_ADDR_START_FIELD =
{
    "DATA_ADDR_START",
#if RU_INCLUDE_DESC
    "",
    "Data address start.\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_3_CFG_DATA_ADDR_START_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_3_CFG_DATA_ADDR_START_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_3_CFG_DATA_ADDR_START_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA_ADDR_STOP *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_3_CFG_DATA_ADDR_STOP_FIELD =
{
    "DATA_ADDR_STOP",
#if RU_INCLUDE_DESC
    "",
    "Data address stop\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_3_CFG_DATA_ADDR_STOP_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_3_CFG_DATA_ADDR_STOP_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_3_CFG_DATA_ADDR_STOP_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_3_CFG_FIELDS[] =
{
    &RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_3_CFG_DATA_ADDR_START_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_3_CFG_DATA_ADDR_STOP_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_3_CFG *****/
const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_3_CFG_REG =
{
    "GENERAL_CONFIG_DATA_BKPT_3_CFG",
#if RU_INCLUDE_DESC
    "DATA_BKPT_CFG_3 Register",
    "DATA Breakpoint 3 configuration.\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_3_CFG_REG_OFFSET },
    0,
    0,
    935,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_3_CFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_COMMON_CFG, TYPE: Type_RCQ_COMMON_REGS_RCQ_GENERAL_CONFIG_DATA_BKPT_COMMON_CFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: THREAD_0 *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_COMMON_CFG_THREAD_0_FIELD =
{
    "THREAD_0",
#if RU_INCLUDE_DESC
    "",
    "Thread for bkpt 0\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_COMMON_CFG_THREAD_0_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_COMMON_CFG_THREAD_0_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_COMMON_CFG_THREAD_0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: THREAD_1 *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_COMMON_CFG_THREAD_1_FIELD =
{
    "THREAD_1",
#if RU_INCLUDE_DESC
    "",
    "Thread for bkpt 1\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_COMMON_CFG_THREAD_1_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_COMMON_CFG_THREAD_1_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_COMMON_CFG_THREAD_1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: THREAD_2 *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_COMMON_CFG_THREAD_2_FIELD =
{
    "THREAD_2",
#if RU_INCLUDE_DESC
    "",
    "Thread for bkpt 2\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_COMMON_CFG_THREAD_2_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_COMMON_CFG_THREAD_2_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_COMMON_CFG_THREAD_2_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: THREAD_3 *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_COMMON_CFG_THREAD_3_FIELD =
{
    "THREAD_3",
#if RU_INCLUDE_DESC
    "",
    "Thread for bkpt 3\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_COMMON_CFG_THREAD_3_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_COMMON_CFG_THREAD_3_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_COMMON_CFG_THREAD_3_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_COMMON_CFG_FIELDS[] =
{
    &RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_COMMON_CFG_THREAD_0_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_COMMON_CFG_THREAD_1_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_COMMON_CFG_THREAD_2_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_COMMON_CFG_THREAD_3_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_COMMON_CFG *****/
const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_COMMON_CFG_REG =
{
    "GENERAL_CONFIG_DATA_BKPT_COMMON_CFG",
#if RU_INCLUDE_DESC
    "DATA_BKPT_COMMON_CFG Register",
    "DATA Breakpoint common configuration.\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_COMMON_CFG_REG_OFFSET },
    0,
    0,
    936,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_COMMON_CFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_GENERAL_CONFIG_UBUS_COUNTER_CONTROL, TYPE: Type_RCQ_COMMON_REGS_RCQ_GENERAL_CONFIG_UBUS_COUNTER_CONTROL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ENABLE_STATISTICS *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_UBUS_COUNTER_CONTROL_ENABLE_STATISTICS_FIELD =
{
    "ENABLE_STATISTICS",
#if RU_INCLUDE_DESC
    "",
    "Enable statistics\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_UBUS_COUNTER_CONTROL_ENABLE_STATISTICS_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_UBUS_COUNTER_CONTROL_ENABLE_STATISTICS_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_UBUS_COUNTER_CONTROL_ENABLE_STATISTICS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SW_RESET *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_UBUS_COUNTER_CONTROL_SW_RESET_FIELD =
{
    "SW_RESET",
#if RU_INCLUDE_DESC
    "",
    "Writing 1 resets all the counters\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_UBUS_COUNTER_CONTROL_SW_RESET_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_UBUS_COUNTER_CONTROL_SW_RESET_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_UBUS_COUNTER_CONTROL_SW_RESET_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_write,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DEST_PID *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_UBUS_COUNTER_CONTROL_DEST_PID_FIELD =
{
    "DEST_PID",
#if RU_INCLUDE_DESC
    "",
    "Destination PID,controls which destination PID transactions are counted (e.g. can be programmed either to PSRAM or DDR)\n\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_UBUS_COUNTER_CONTROL_DEST_PID_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_UBUS_COUNTER_CONTROL_DEST_PID_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_UBUS_COUNTER_CONTROL_DEST_PID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MASTER_SELECT *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_UBUS_COUNTER_CONTROL_MASTER_SELECT_FIELD =
{
    "MASTER_SELECT",
#if RU_INCLUDE_DESC
    "",
    "Selects which master is measured\n\n\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_UBUS_COUNTER_CONTROL_MASTER_SELECT_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_UBUS_COUNTER_CONTROL_MASTER_SELECT_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_UBUS_COUNTER_CONTROL_MASTER_SELECT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_GENERAL_CONFIG_UBUS_COUNTER_CONTROL_FIELDS[] =
{
    &RNR_QUAD_GENERAL_CONFIG_UBUS_COUNTER_CONTROL_ENABLE_STATISTICS_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_UBUS_COUNTER_CONTROL_SW_RESET_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_UBUS_COUNTER_CONTROL_DEST_PID_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_UBUS_COUNTER_CONTROL_MASTER_SELECT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_GENERAL_CONFIG_UBUS_COUNTER_CONTROL *****/
const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_UBUS_COUNTER_CONTROL_REG =
{
    "GENERAL_CONFIG_UBUS_COUNTER_CONTROL",
#if RU_INCLUDE_DESC
    "UBUS_COUNTER_CONTROL Register",
    "UBUS Counters Control\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_UBUS_COUNTER_CONTROL_REG_OFFSET },
    0,
    0,
    937,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    RNR_QUAD_GENERAL_CONFIG_UBUS_COUNTER_CONTROL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_GENERAL_CONFIG_UBUS_DOWN_COUNTER, TYPE: Type_RCQ_COMMON_REGS_RCQ_GENERAL_CONFIG_UBUS_DOWN_COUNTER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DOWNCNT_VALUE *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_UBUS_DOWN_COUNTER_DOWNCNT_VALUE_FIELD =
{
    "DOWNCNT_VALUE",
#if RU_INCLUDE_DESC
    "",
    "Set the size of the window.\nWhen Statistics are enabled this counter counts down, while it is not 0 the statistics are collected - i.e below statistics are updated as long as this counter counts down.\n\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_UBUS_DOWN_COUNTER_DOWNCNT_VALUE_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_UBUS_DOWN_COUNTER_DOWNCNT_VALUE_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_UBUS_DOWN_COUNTER_DOWNCNT_VALUE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_GENERAL_CONFIG_UBUS_DOWN_COUNTER_FIELDS[] =
{
    &RNR_QUAD_GENERAL_CONFIG_UBUS_DOWN_COUNTER_DOWNCNT_VALUE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_GENERAL_CONFIG_UBUS_DOWN_COUNTER *****/
const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_UBUS_DOWN_COUNTER_REG =
{
    "GENERAL_CONFIG_UBUS_DOWN_COUNTER",
#if RU_INCLUDE_DESC
    "UBUS_DOWN_COUNTER Register",
    "When Statistics are enabled this counter counts down, while its not 0 the statistics are collected - i.e below statistics are updated as long as this counter counts down.\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_UBUS_DOWN_COUNTER_REG_OFFSET },
    0,
    0,
    938,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_GENERAL_CONFIG_UBUS_DOWN_COUNTER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_GENERAL_CONFIG_ALL_XFERS_CNT, TYPE: Type_RCQ_COMMON_REGS_RCQ_GENERAL_CONFIG_ALL_XFERS_CNT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNTER_VALUE *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_ALL_XFERS_CNT_COUNTER_VALUE_FIELD =
{
    "COUNTER_VALUE",
#if RU_INCLUDE_DESC
    "",
    "Value\n\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_ALL_XFERS_CNT_COUNTER_VALUE_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_ALL_XFERS_CNT_COUNTER_VALUE_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_ALL_XFERS_CNT_COUNTER_VALUE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_GENERAL_CONFIG_ALL_XFERS_CNT_FIELDS[] =
{
    &RNR_QUAD_GENERAL_CONFIG_ALL_XFERS_CNT_COUNTER_VALUE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_GENERAL_CONFIG_ALL_XFERS_CNT *****/
const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_ALL_XFERS_CNT_REG =
{
    "GENERAL_CONFIG_ALL_XFERS_CNT",
#if RU_INCLUDE_DESC
    "ALL_XFERS_CNT Register",
    "Count all issued master UBUS transactions (read,write etc) - 32 bit\n\nCount 1 for each of the following transactions:\nFor UBUS 4 , count ubus4_rd (code = 4),ubus4_wr (code = 0), ubus4_wr_rply (code = 2)\nFor UBUS 4.1 count ubus4_rd (code = 4),ubus4_wr (code = 0), ubus4_wr_sync (code = 2)\n\n\n\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_ALL_XFERS_CNT_REG_OFFSET },
    0,
    0,
    939,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_GENERAL_CONFIG_ALL_XFERS_CNT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_GENERAL_CONFIG_READ_XFERS_CNT, TYPE: Type_RCQ_COMMON_REGS_RCQ_GENERAL_CONFIG_READ_XFERS_CNT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNTER_VALUE *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_READ_XFERS_CNT_COUNTER_VALUE_FIELD =
{
    "COUNTER_VALUE",
#if RU_INCLUDE_DESC
    "",
    "Value\n\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_READ_XFERS_CNT_COUNTER_VALUE_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_READ_XFERS_CNT_COUNTER_VALUE_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_READ_XFERS_CNT_COUNTER_VALUE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_GENERAL_CONFIG_READ_XFERS_CNT_FIELDS[] =
{
    &RNR_QUAD_GENERAL_CONFIG_READ_XFERS_CNT_COUNTER_VALUE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_GENERAL_CONFIG_READ_XFERS_CNT *****/
const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_READ_XFERS_CNT_REG =
{
    "GENERAL_CONFIG_READ_XFERS_CNT",
#if RU_INCLUDE_DESC
    "READ_XFERS_CNT Register",
    "count only UBUS master read transactions   - 32 bit\nFor UBUS 4, 4.1 , count ubus4_rd (code = 4)\n\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_READ_XFERS_CNT_REG_OFFSET },
    0,
    0,
    940,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_GENERAL_CONFIG_READ_XFERS_CNT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_GENERAL_CONFIG_READ_DATA_CNT, TYPE: Type_RCQ_COMMON_REGS_RCQ_GENERAL_CONFIG_READ_DATA_CNT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNTER_VALUE *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_READ_DATA_CNT_COUNTER_VALUE_FIELD =
{
    "COUNTER_VALUE",
#if RU_INCLUDE_DESC
    "",
    "Value\n\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_READ_DATA_CNT_COUNTER_VALUE_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_READ_DATA_CNT_COUNTER_VALUE_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_READ_DATA_CNT_COUNTER_VALUE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_GENERAL_CONFIG_READ_DATA_CNT_FIELDS[] =
{
    &RNR_QUAD_GENERAL_CONFIG_READ_DATA_CNT_COUNTER_VALUE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_GENERAL_CONFIG_READ_DATA_CNT *****/
const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_READ_DATA_CNT_REG =
{
    "GENERAL_CONFIG_READ_DATA_CNT",
#if RU_INCLUDE_DESC
    "READ_DATA_CNT Register",
    "accumulate dlen of all read transaction - 32 bit\nAdd dlen value of transaction of the following type:\nFor UBUS 4, 4.1 , count ubus4_rd (code = 4)\n\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_READ_DATA_CNT_REG_OFFSET },
    0,
    0,
    941,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_GENERAL_CONFIG_READ_DATA_CNT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_GENERAL_CONFIG_WRITE_DATA_CNT, TYPE: Type_RCQ_COMMON_REGS_RCQ_GENERAL_CONFIG_WRITE_DATA_CNT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNTER_VALUE *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_WRITE_DATA_CNT_COUNTER_VALUE_FIELD =
{
    "COUNTER_VALUE",
#if RU_INCLUDE_DESC
    "",
    "Value\n\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_WRITE_DATA_CNT_COUNTER_VALUE_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_WRITE_DATA_CNT_COUNTER_VALUE_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_WRITE_DATA_CNT_COUNTER_VALUE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_GENERAL_CONFIG_WRITE_DATA_CNT_FIELDS[] =
{
    &RNR_QUAD_GENERAL_CONFIG_WRITE_DATA_CNT_COUNTER_VALUE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_GENERAL_CONFIG_WRITE_DATA_CNT *****/
const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_WRITE_DATA_CNT_REG =
{
    "GENERAL_CONFIG_WRITE_DATA_CNT",
#if RU_INCLUDE_DESC
    "WRITE_DATA_CNT Register",
    "Accumulate dlen of all write transactions - 32 bit\n\nAdd dlen value of transaction of the following type:\n\nFor UBUS 4.0 - ubus4_wr (code = 0), ubus4_wr_rply (code = 2)\nFor UBUS 4.1 - ubus4_wr (code = 0), ubus4_wr_sync (code = 2)\n\n\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_WRITE_DATA_CNT_REG_OFFSET },
    0,
    0,
    942,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_GENERAL_CONFIG_WRITE_DATA_CNT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_GENERAL_CONFIG_MISC_CFG, TYPE: Type_RCQ_COMMON_REGS_RCQ_GENERAL_CONFIG_MISC_CFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_PID *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_MISC_CFG_DDR_PID_FIELD =
{
    "DDR_PID",
#if RU_INCLUDE_DESC
    "",
    "Set value for DDR PID to be used for determining which transactions go to dedicated DDR virtual channel\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_MISC_CFG_DDR_PID_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_MISC_CFG_DDR_PID_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_MISC_CFG_DDR_PID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_GENERAL_CONFIG_MISC_CFG_FIELDS[] =
{
    &RNR_QUAD_GENERAL_CONFIG_MISC_CFG_DDR_PID_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_GENERAL_CONFIG_MISC_CFG *****/
const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_MISC_CFG_REG =
{
    "GENERAL_CONFIG_MISC_CFG",
#if RU_INCLUDE_DESC
    "MISC_CFG Register",
    "Misc Configuration\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_MISC_CFG_REG_OFFSET },
    0,
    0,
    943,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_GENERAL_CONFIG_MISC_CFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_GENERAL_CONFIG_AQM_CONTROL, TYPE: Type_RCQ_COMMON_REGS_RCQ_GENERAL_CONFIG_AQM_CONTROL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ENABLE_COUNTER *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_AQM_CONTROL_ENABLE_COUNTER_FIELD =
{
    "ENABLE_COUNTER",
#if RU_INCLUDE_DESC
    "",
    "Enable AQM counter. Disable resets the counter.\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_AQM_CONTROL_ENABLE_COUNTER_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_AQM_CONTROL_ENABLE_COUNTER_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_AQM_CONTROL_ENABLE_COUNTER_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ENABLE_RANDOM *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_AQM_CONTROL_ENABLE_RANDOM_FIELD =
{
    "ENABLE_RANDOM",
#if RU_INCLUDE_DESC
    "",
    "Enable AQM random generator. Disable resets the counter.\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_AQM_CONTROL_ENABLE_RANDOM_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_AQM_CONTROL_ENABLE_RANDOM_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_AQM_CONTROL_ENABLE_RANDOM_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_GENERAL_CONFIG_AQM_CONTROL_FIELDS[] =
{
    &RNR_QUAD_GENERAL_CONFIG_AQM_CONTROL_ENABLE_COUNTER_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_AQM_CONTROL_ENABLE_RANDOM_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_GENERAL_CONFIG_AQM_CONTROL *****/
const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_AQM_CONTROL_REG =
{
    "GENERAL_CONFIG_AQM_CONTROL",
#if RU_INCLUDE_DESC
    "AQM_CONTROL Register",
    "Control AQM counter and random generation\n\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_AQM_CONTROL_REG_OFFSET },
    0,
    0,
    944,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    RNR_QUAD_GENERAL_CONFIG_AQM_CONTROL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_GENERAL_CONFIG_AQM_RANDM_VALUE, TYPE: Type_RCQ_COMMON_REGS_RCQ_GENERAL_CONFIG_AQM_RANDM_VALUE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RANDOM_VALUE *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_AQM_RANDM_VALUE_RANDOM_VALUE_FIELD =
{
    "RANDOM_VALUE",
#if RU_INCLUDE_DESC
    "",
    "AQM random generator value\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_AQM_RANDM_VALUE_RANDOM_VALUE_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_AQM_RANDM_VALUE_RANDOM_VALUE_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_AQM_RANDM_VALUE_RANDOM_VALUE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_GENERAL_CONFIG_AQM_RANDM_VALUE_FIELDS[] =
{
    &RNR_QUAD_GENERAL_CONFIG_AQM_RANDM_VALUE_RANDOM_VALUE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_GENERAL_CONFIG_AQM_RANDM_VALUE *****/
const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_AQM_RANDM_VALUE_REG =
{
    "GENERAL_CONFIG_AQM_RANDM_VALUE",
#if RU_INCLUDE_DESC
    "AQM_RANDOM_VALUE Register",
    "Read AQM counter random value\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_AQM_RANDM_VALUE_REG_OFFSET },
    0,
    0,
    945,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_GENERAL_CONFIG_AQM_RANDM_VALUE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_GENERAL_CONFIG_AQM_RANDOM_SEED, TYPE: Type_RCQ_COMMON_REGS_RCQ_GENERAL_CONFIG_AQM_RANDOM_SEED
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RANDOM_SEED *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_AQM_RANDOM_SEED_RANDOM_SEED_FIELD =
{
    "RANDOM_SEED",
#if RU_INCLUDE_DESC
    "",
    "Set AQM random generator seed\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_AQM_RANDOM_SEED_RANDOM_SEED_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_AQM_RANDOM_SEED_RANDOM_SEED_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_AQM_RANDOM_SEED_RANDOM_SEED_FIELD_SHIFT },
    168430090,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_GENERAL_CONFIG_AQM_RANDOM_SEED_FIELDS[] =
{
    &RNR_QUAD_GENERAL_CONFIG_AQM_RANDOM_SEED_RANDOM_SEED_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_GENERAL_CONFIG_AQM_RANDOM_SEED *****/
const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_AQM_RANDOM_SEED_REG =
{
    "GENERAL_CONFIG_AQM_RANDOM_SEED",
#if RU_INCLUDE_DESC
    "AQM_RANDOM_SEED Register",
    "Set seed for AQM random generator\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_AQM_RANDOM_SEED_REG_OFFSET },
    0,
    0,
    946,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_GENERAL_CONFIG_AQM_RANDOM_SEED_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_GENERAL_CONFIG_AQM_RANDOM_TEST_INC, TYPE: Type_RCQ_COMMON_REGS_RCQ_GENERAL_CONFIG_AQM_RANDOM_TEST_INC
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RANDOM_INC *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_AQM_RANDOM_TEST_INC_RANDOM_INC_FIELD =
{
    "RANDOM_INC",
#if RU_INCLUDE_DESC
    "",
    "Inc AQM random generator (for test) by writing 1\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_AQM_RANDOM_TEST_INC_RANDOM_INC_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_AQM_RANDOM_TEST_INC_RANDOM_INC_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_AQM_RANDOM_TEST_INC_RANDOM_INC_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_write,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_GENERAL_CONFIG_AQM_RANDOM_TEST_INC_FIELDS[] =
{
    &RNR_QUAD_GENERAL_CONFIG_AQM_RANDOM_TEST_INC_RANDOM_INC_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_GENERAL_CONFIG_AQM_RANDOM_TEST_INC *****/
const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_AQM_RANDOM_TEST_INC_REG =
{
    "GENERAL_CONFIG_AQM_RANDOM_TEST_INC",
#if RU_INCLUDE_DESC
    "AQM_RANDOM_TEST_INC Register",
    "Increment random generator (for testing)\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_AQM_RANDOM_TEST_INC_REG_OFFSET },
    0,
    0,
    947,
#if RU_INCLUDE_ACCESS
    ru_access_write,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_GENERAL_CONFIG_AQM_RANDOM_TEST_INC_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_GENERAL_CONFIG_MULTI_PSEL_CFG, TYPE: Type_RCQ_COMMON_REGS_RCQ_GENERAL_CONFIG_MULTI_PSEL_CFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: MULTI_PSEL_MASTER_SEL *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_MULTI_PSEL_CFG_MULTI_PSEL_MASTER_SEL_FIELD =
{
    "MULTI_PSEL_MASTER_SEL",
#if RU_INCLUDE_DESC
    "",
    "Select which select will be treated as multiple write. Only single bit can be set to 1 at any given time.\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_MULTI_PSEL_CFG_MULTI_PSEL_MASTER_SEL_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_MULTI_PSEL_CFG_MULTI_PSEL_MASTER_SEL_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_MULTI_PSEL_CFG_MULTI_PSEL_MASTER_SEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MULTI_PSEL_MASK *****/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_MULTI_PSEL_CFG_MULTI_PSEL_MASK_FIELD =
{
    "MULTI_PSEL_MASK",
#if RU_INCLUDE_DESC
    "",
    "Select which clients will get write transaction if it arrives on master psel. Must set 1 for Runner which is configured as master by MULTI_PSEL_MASTER_SEL field\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_MULTI_PSEL_CFG_MULTI_PSEL_MASK_FIELD_MASK },
    0,
    { RNR_QUAD_GENERAL_CONFIG_MULTI_PSEL_CFG_MULTI_PSEL_MASK_FIELD_WIDTH },
    { RNR_QUAD_GENERAL_CONFIG_MULTI_PSEL_CFG_MULTI_PSEL_MASK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_GENERAL_CONFIG_MULTI_PSEL_CFG_FIELDS[] =
{
    &RNR_QUAD_GENERAL_CONFIG_MULTI_PSEL_CFG_MULTI_PSEL_MASTER_SEL_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_MULTI_PSEL_CFG_MULTI_PSEL_MASK_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_GENERAL_CONFIG_MULTI_PSEL_CFG *****/
const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_MULTI_PSEL_CFG_REG =
{
    "GENERAL_CONFIG_MULTI_PSEL_CFG",
#if RU_INCLUDE_DESC
    "MULTI_PSEL_CFG Register",
    "Configuration for multiple write access.\n\n",
#endif
    { RNR_QUAD_GENERAL_CONFIG_MULTI_PSEL_CFG_REG_OFFSET },
    0,
    0,
    948,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    RNR_QUAD_GENERAL_CONFIG_MULTI_PSEL_CFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_UBUS_DECODE_CFG_PSRAM_UBUS_DECODE, TYPE: Type_RCQ_COMMON_REGS_UBUS_DECODE_CFG_PSRAM_UBUS_DECODE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec RNR_QUAD_UBUS_DECODE_CFG_PSRAM_UBUS_DECODE_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "Value\n",
#endif
    { RNR_QUAD_UBUS_DECODE_CFG_PSRAM_UBUS_DECODE_VAL_FIELD_MASK },
    0,
    { RNR_QUAD_UBUS_DECODE_CFG_PSRAM_UBUS_DECODE_VAL_FIELD_WIDTH },
    { RNR_QUAD_UBUS_DECODE_CFG_PSRAM_UBUS_DECODE_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_UBUS_DECODE_CFG_PSRAM_UBUS_DECODE_FIELDS[] =
{
    &RNR_QUAD_UBUS_DECODE_CFG_PSRAM_UBUS_DECODE_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_UBUS_DECODE_CFG_PSRAM_UBUS_DECODE *****/
const ru_reg_rec RNR_QUAD_UBUS_DECODE_CFG_PSRAM_UBUS_DECODE_REG =
{
    "UBUS_DECODE_CFG_PSRAM_UBUS_DECODE",
#if RU_INCLUDE_DESC
    "PSRAM_UBUS_DECODE 0..15 Register",
    "Decode for PSRAM Queue\n",
#endif
    { RNR_QUAD_UBUS_DECODE_CFG_PSRAM_UBUS_DECODE_REG_OFFSET },
    RNR_QUAD_UBUS_DECODE_CFG_PSRAM_UBUS_DECODE_REG_RAM_CNT,
    4,
    949,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_UBUS_DECODE_CFG_PSRAM_UBUS_DECODE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_UBUS_DECODE_CFG_DDR_UBUS_DECODE, TYPE: Type_RCQ_COMMON_REGS_UBUS_DECODE_CFG_DDR_UBUS_DECODE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec RNR_QUAD_UBUS_DECODE_CFG_DDR_UBUS_DECODE_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "Value\n",
#endif
    { RNR_QUAD_UBUS_DECODE_CFG_DDR_UBUS_DECODE_VAL_FIELD_MASK },
    0,
    { RNR_QUAD_UBUS_DECODE_CFG_DDR_UBUS_DECODE_VAL_FIELD_WIDTH },
    { RNR_QUAD_UBUS_DECODE_CFG_DDR_UBUS_DECODE_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_UBUS_DECODE_CFG_DDR_UBUS_DECODE_FIELDS[] =
{
    &RNR_QUAD_UBUS_DECODE_CFG_DDR_UBUS_DECODE_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_UBUS_DECODE_CFG_DDR_UBUS_DECODE *****/
const ru_reg_rec RNR_QUAD_UBUS_DECODE_CFG_DDR_UBUS_DECODE_REG =
{
    "UBUS_DECODE_CFG_DDR_UBUS_DECODE",
#if RU_INCLUDE_DESC
    "DDR_UBUS_DECODE 0..15 Register",
    "Decode for DDR Queue\n",
#endif
    { RNR_QUAD_UBUS_DECODE_CFG_DDR_UBUS_DECODE_REG_OFFSET },
    RNR_QUAD_UBUS_DECODE_CFG_DDR_UBUS_DECODE_REG_RAM_CNT,
    4,
    950,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_UBUS_DECODE_CFG_DDR_UBUS_DECODE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_UBUS_DECODE_CFG2_PSRAM_UBUS_DECODE2, TYPE: Type_RCQ_COMMON_REGS_UBUS_DECODE_CFG2_PSRAM_UBUS_DECODE2
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec RNR_QUAD_UBUS_DECODE_CFG2_PSRAM_UBUS_DECODE2_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "Value\n",
#endif
    { RNR_QUAD_UBUS_DECODE_CFG2_PSRAM_UBUS_DECODE2_VAL_FIELD_MASK },
    0,
    { RNR_QUAD_UBUS_DECODE_CFG2_PSRAM_UBUS_DECODE2_VAL_FIELD_WIDTH },
    { RNR_QUAD_UBUS_DECODE_CFG2_PSRAM_UBUS_DECODE2_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_UBUS_DECODE_CFG2_PSRAM_UBUS_DECODE2_FIELDS[] =
{
    &RNR_QUAD_UBUS_DECODE_CFG2_PSRAM_UBUS_DECODE2_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_UBUS_DECODE_CFG2_PSRAM_UBUS_DECODE2 *****/
const ru_reg_rec RNR_QUAD_UBUS_DECODE_CFG2_PSRAM_UBUS_DECODE2_REG =
{
    "UBUS_DECODE_CFG2_PSRAM_UBUS_DECODE2",
#if RU_INCLUDE_DESC
    "PSRAM_UBUS_DECODE2 0..15 Register",
    "Decode for PSRAM Queue\n",
#endif
    { RNR_QUAD_UBUS_DECODE_CFG2_PSRAM_UBUS_DECODE2_REG_OFFSET },
    RNR_QUAD_UBUS_DECODE_CFG2_PSRAM_UBUS_DECODE2_REG_RAM_CNT,
    4,
    951,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_UBUS_DECODE_CFG2_PSRAM_UBUS_DECODE2_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_UBUS_DECODE_CFG2_DDR_UBUS_DECODE2, TYPE: Type_RCQ_COMMON_REGS_UBUS_DECODE_CFG2_DDR_UBUS_DECODE2
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec RNR_QUAD_UBUS_DECODE_CFG2_DDR_UBUS_DECODE2_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "Value\n",
#endif
    { RNR_QUAD_UBUS_DECODE_CFG2_DDR_UBUS_DECODE2_VAL_FIELD_MASK },
    0,
    { RNR_QUAD_UBUS_DECODE_CFG2_DDR_UBUS_DECODE2_VAL_FIELD_WIDTH },
    { RNR_QUAD_UBUS_DECODE_CFG2_DDR_UBUS_DECODE2_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_UBUS_DECODE_CFG2_DDR_UBUS_DECODE2_FIELDS[] =
{
    &RNR_QUAD_UBUS_DECODE_CFG2_DDR_UBUS_DECODE2_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_UBUS_DECODE_CFG2_DDR_UBUS_DECODE2 *****/
const ru_reg_rec RNR_QUAD_UBUS_DECODE_CFG2_DDR_UBUS_DECODE2_REG =
{
    "UBUS_DECODE_CFG2_DDR_UBUS_DECODE2",
#if RU_INCLUDE_DESC
    "DDR_UBUS_DECODE2 0..15 Register",
    "Decode for DDR Queue\n",
#endif
    { RNR_QUAD_UBUS_DECODE_CFG2_DDR_UBUS_DECODE2_REG_OFFSET },
    RNR_QUAD_UBUS_DECODE_CFG2_DDR_UBUS_DECODE2_REG_RAM_CNT,
    4,
    952,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_UBUS_DECODE_CFG2_DDR_UBUS_DECODE2_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_EXT_FLOWCTRL_CONFIG_TOKEN_VAL, TYPE: Type_RCQ_COMMON_REGS_EXT_FLOWCTRL_CONFIG_TOKEN_VAL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec RNR_QUAD_EXT_FLOWCTRL_CONFIG_TOKEN_VAL_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "Value\n",
#endif
    { RNR_QUAD_EXT_FLOWCTRL_CONFIG_TOKEN_VAL_VAL_FIELD_MASK },
    0,
    { RNR_QUAD_EXT_FLOWCTRL_CONFIG_TOKEN_VAL_VAL_FIELD_WIDTH },
    { RNR_QUAD_EXT_FLOWCTRL_CONFIG_TOKEN_VAL_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_EXT_FLOWCTRL_CONFIG_TOKEN_VAL_FIELDS[] =
{
    &RNR_QUAD_EXT_FLOWCTRL_CONFIG_TOKEN_VAL_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_EXT_FLOWCTRL_CONFIG_TOKEN_VAL *****/
const ru_reg_rec RNR_QUAD_EXT_FLOWCTRL_CONFIG_TOKEN_VAL_REG =
{
    "EXT_FLOWCTRL_CONFIG_TOKEN_VAL",
#if RU_INCLUDE_DESC
    "TOKEN 0..35 Register",
    "Token value for flow control\n",
#endif
    { RNR_QUAD_EXT_FLOWCTRL_CONFIG_TOKEN_VAL_REG_OFFSET },
    RNR_QUAD_EXT_FLOWCTRL_CONFIG_TOKEN_VAL_REG_RAM_CNT,
    4,
    953,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_EXT_FLOWCTRL_CONFIG_TOKEN_VAL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_QUAD_EXT_FLOWCTRL_CONFIG2_TOKEN_VAL_2, TYPE: Type_RCQ_COMMON_REGS_EXT_FLOWCTRL_CONFIG2_TOKEN_VAL_2
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec RNR_QUAD_EXT_FLOWCTRL_CONFIG2_TOKEN_VAL_2_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "Value\n",
#endif
    { RNR_QUAD_EXT_FLOWCTRL_CONFIG2_TOKEN_VAL_2_VAL_FIELD_MASK },
    0,
    { RNR_QUAD_EXT_FLOWCTRL_CONFIG2_TOKEN_VAL_2_VAL_FIELD_WIDTH },
    { RNR_QUAD_EXT_FLOWCTRL_CONFIG2_TOKEN_VAL_2_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_EXT_FLOWCTRL_CONFIG2_TOKEN_VAL_2_FIELDS[] =
{
    &RNR_QUAD_EXT_FLOWCTRL_CONFIG2_TOKEN_VAL_2_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_QUAD_EXT_FLOWCTRL_CONFIG2_TOKEN_VAL_2 *****/
const ru_reg_rec RNR_QUAD_EXT_FLOWCTRL_CONFIG2_TOKEN_VAL_2_REG =
{
    "EXT_FLOWCTRL_CONFIG2_TOKEN_VAL_2",
#if RU_INCLUDE_DESC
    "TOKEN2 0..35 Register",
    "Token value for flow control\n",
#endif
    { RNR_QUAD_EXT_FLOWCTRL_CONFIG2_TOKEN_VAL_2_REG_OFFSET },
    RNR_QUAD_EXT_FLOWCTRL_CONFIG2_TOKEN_VAL_2_REG_RAM_CNT,
    4,
    954,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_EXT_FLOWCTRL_CONFIG2_TOKEN_VAL_2_FIELDS,
#endif
};

unsigned long RNR_QUAD_ADDRS[] =
{
    0x82810000,
};

static const ru_reg_rec *RNR_QUAD_REGS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_ENG_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_0_1_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_2_3_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_4_5_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_6_7_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER0_CFG_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER1_CFG_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER0_MASK_CFG_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER1_MASK_CFG_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_SNAP_ORG_CODE_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_PPP_IP_PROT_CODE_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_ETHTYPE_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_0_1_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_2_3_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_IPV6_HDR_EXT_FLTR_MASK_CFG_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_0_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_1_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_2_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_IP_PROT_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_L_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_H_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_VAL_L_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_VAL_H_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_L_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_H_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_L_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_H_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_L_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_H_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_L_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_H_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_L_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_H_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_L_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_H_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT8_VAL_L_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT8_VAL_H_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_L_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_H_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_MASK_L_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_MASK_H_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_GRE_PROTOCOL_CFG_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DOS_ATTACK_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_ICMP_MAX_SIZE_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_KEY_CFG_REG,
    &RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_REG,
    &RNR_QUAD_GENERAL_CONFIG_PSRAM0_BASE_REG,
    &RNR_QUAD_GENERAL_CONFIG_PSRAM1_BASE_REG,
    &RNR_QUAD_GENERAL_CONFIG_PSRAM2_BASE_REG,
    &RNR_QUAD_GENERAL_CONFIG_PSRAM3_BASE_REG,
    &RNR_QUAD_GENERAL_CONFIG_DDR0_BASE_REG,
    &RNR_QUAD_GENERAL_CONFIG_DDR1_BASE_REG,
    &RNR_QUAD_GENERAL_CONFIG_PSRAM0_MASK_REG,
    &RNR_QUAD_GENERAL_CONFIG_PSRAM1_MASK_REG,
    &RNR_QUAD_GENERAL_CONFIG_PSRAM2_MASK_REG,
    &RNR_QUAD_GENERAL_CONFIG_PSRAM3_MASK_REG,
    &RNR_QUAD_GENERAL_CONFIG_DDR0_MASK_REG,
    &RNR_QUAD_GENERAL_CONFIG_DDR1_MASK_REG,
    &RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_REG,
    &RNR_QUAD_GENERAL_CONFIG_BKPT_0_CFG_REG,
    &RNR_QUAD_GENERAL_CONFIG_BKPT_1_CFG_REG,
    &RNR_QUAD_GENERAL_CONFIG_BKPT_2_CFG_REG,
    &RNR_QUAD_GENERAL_CONFIG_BKPT_3_CFG_REG,
    &RNR_QUAD_GENERAL_CONFIG_BKPT_4_CFG_REG,
    &RNR_QUAD_GENERAL_CONFIG_BKPT_5_CFG_REG,
    &RNR_QUAD_GENERAL_CONFIG_BKPT_6_CFG_REG,
    &RNR_QUAD_GENERAL_CONFIG_BKPT_7_CFG_REG,
    &RNR_QUAD_GENERAL_CONFIG_BKPT_GEN_CFG_REG,
    &RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_REG,
    &RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_REG,
    &RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_0_CFG_REG,
    &RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_1_CFG_REG,
    &RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_2_CFG_REG,
    &RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_3_CFG_REG,
    &RNR_QUAD_GENERAL_CONFIG_DATA_BKPT_COMMON_CFG_REG,
    &RNR_QUAD_GENERAL_CONFIG_UBUS_COUNTER_CONTROL_REG,
    &RNR_QUAD_GENERAL_CONFIG_UBUS_DOWN_COUNTER_REG,
    &RNR_QUAD_GENERAL_CONFIG_ALL_XFERS_CNT_REG,
    &RNR_QUAD_GENERAL_CONFIG_READ_XFERS_CNT_REG,
    &RNR_QUAD_GENERAL_CONFIG_READ_DATA_CNT_REG,
    &RNR_QUAD_GENERAL_CONFIG_WRITE_DATA_CNT_REG,
    &RNR_QUAD_GENERAL_CONFIG_MISC_CFG_REG,
    &RNR_QUAD_GENERAL_CONFIG_AQM_CONTROL_REG,
    &RNR_QUAD_GENERAL_CONFIG_AQM_RANDM_VALUE_REG,
    &RNR_QUAD_GENERAL_CONFIG_AQM_RANDOM_SEED_REG,
    &RNR_QUAD_GENERAL_CONFIG_AQM_RANDOM_TEST_INC_REG,
    &RNR_QUAD_GENERAL_CONFIG_MULTI_PSEL_CFG_REG,
    &RNR_QUAD_DEBUG_FIFO_CONFIG_REG,
    &RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_REG,
    &RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_REG,
    &RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_REG,
    &RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS_REG,
    &RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS2_REG,
    &RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_DATA1_REG,
    &RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_DATA2_REG,
    &RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_DATA1_REG,
    &RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_DATA2_REG,
    &RNR_QUAD_DEBUG_DDR_HDR_FIFO_DATA1_REG,
    &RNR_QUAD_DEBUG_DDR_HDR_FIFO_DATA2_REG,
    &RNR_QUAD_EXT_FLOWCTRL_CONFIG_TOKEN_VAL_REG,
    &RNR_QUAD_EXT_FLOWCTRL_CONFIG2_TOKEN_VAL_2_REG,
    &RNR_QUAD_UBUS_DECODE_CFG_PSRAM_UBUS_DECODE_REG,
    &RNR_QUAD_UBUS_DECODE_CFG_DDR_UBUS_DECODE_REG,
    &RNR_QUAD_UBUS_DECODE_CFG2_PSRAM_UBUS_DECODE2_REG,
    &RNR_QUAD_UBUS_DECODE_CFG2_DDR_UBUS_DECODE2_REG,
};

const ru_block_rec RNR_QUAD_BLOCK =
{
    "RNR_QUAD",
    RNR_QUAD_ADDRS,
    1,
    113,
    RNR_QUAD_REGS,
};
