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

#include "ru.h"

#if RU_INCLUDE_FIELD_DB
/******************************************************************************
 * Field: RNR_QUAD_UBUS_SLV_RES_CNTRL_MASK_ID
 ******************************************************************************/
const ru_field_rec RNR_QUAD_UBUS_SLV_RES_CNTRL_MASK_ID_FIELD =
{
    "MASK_ID",
#if RU_INCLUDE_DESC
    "mask_id",
    "Mask id. masks the port_id entering the bridge (ubus2)",
#endif
    RNR_QUAD_UBUS_SLV_RES_CNTRL_MASK_ID_FIELD_MASK,
    0,
    RNR_QUAD_UBUS_SLV_RES_CNTRL_MASK_ID_FIELD_WIDTH,
    RNR_QUAD_UBUS_SLV_RES_CNTRL_MASK_ID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_UBUS_SLV_RES_CNTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec RNR_QUAD_UBUS_SLV_RES_CNTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_QUAD_UBUS_SLV_RES_CNTRL_RESERVED0_FIELD_MASK,
    0,
    RNR_QUAD_UBUS_SLV_RES_CNTRL_RESERVED0_FIELD_WIDTH,
    RNR_QUAD_UBUS_SLV_RES_CNTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_UBUS_SLV_RES_CNTRL_REPIN_ESWAP
 ******************************************************************************/
const ru_field_rec RNR_QUAD_UBUS_SLV_RES_CNTRL_REPIN_ESWAP_FIELD =
{
    "REPIN_ESWAP",
#if RU_INCLUDE_DESC
    "repin_eswap",
    "repin endian swap",
#endif
    RNR_QUAD_UBUS_SLV_RES_CNTRL_REPIN_ESWAP_FIELD_MASK,
    0,
    RNR_QUAD_UBUS_SLV_RES_CNTRL_REPIN_ESWAP_FIELD_WIDTH,
    RNR_QUAD_UBUS_SLV_RES_CNTRL_REPIN_ESWAP_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_UBUS_SLV_RES_CNTRL_REQOUT_ESWAP
 ******************************************************************************/
const ru_field_rec RNR_QUAD_UBUS_SLV_RES_CNTRL_REQOUT_ESWAP_FIELD =
{
    "REQOUT_ESWAP",
#if RU_INCLUDE_DESC
    "reqout_eswap",
    "reqout endian swap",
#endif
    RNR_QUAD_UBUS_SLV_RES_CNTRL_REQOUT_ESWAP_FIELD_MASK,
    0,
    RNR_QUAD_UBUS_SLV_RES_CNTRL_REQOUT_ESWAP_FIELD_WIDTH,
    RNR_QUAD_UBUS_SLV_RES_CNTRL_REQOUT_ESWAP_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_UBUS_SLV_RES_CNTRL_DEV_CLKEN
 ******************************************************************************/
const ru_field_rec RNR_QUAD_UBUS_SLV_RES_CNTRL_DEV_CLKEN_FIELD =
{
    "DEV_CLKEN",
#if RU_INCLUDE_DESC
    "dev_clk_en",
    "device clock enable",
#endif
    RNR_QUAD_UBUS_SLV_RES_CNTRL_DEV_CLKEN_FIELD_MASK,
    0,
    RNR_QUAD_UBUS_SLV_RES_CNTRL_DEV_CLKEN_FIELD_WIDTH,
    RNR_QUAD_UBUS_SLV_RES_CNTRL_DEV_CLKEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_UBUS_SLV_RES_CNTRL_DEV_ERR
 ******************************************************************************/
const ru_field_rec RNR_QUAD_UBUS_SLV_RES_CNTRL_DEV_ERR_FIELD =
{
    "DEV_ERR",
#if RU_INCLUDE_DESC
    "dev_error",
    "indicate an error on Ubus",
#endif
    RNR_QUAD_UBUS_SLV_RES_CNTRL_DEV_ERR_FIELD_MASK,
    0,
    RNR_QUAD_UBUS_SLV_RES_CNTRL_DEV_ERR_FIELD_WIDTH,
    RNR_QUAD_UBUS_SLV_RES_CNTRL_DEV_ERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_UBUS_SLV_RES_CNTRL_DEV_TIMEOUT_EN
 ******************************************************************************/
const ru_field_rec RNR_QUAD_UBUS_SLV_RES_CNTRL_DEV_TIMEOUT_EN_FIELD =
{
    "DEV_TIMEOUT_EN",
#if RU_INCLUDE_DESC
    "dev_timeout_en",
    "enables timeout",
#endif
    RNR_QUAD_UBUS_SLV_RES_CNTRL_DEV_TIMEOUT_EN_FIELD_MASK,
    0,
    RNR_QUAD_UBUS_SLV_RES_CNTRL_DEV_TIMEOUT_EN_FIELD_WIDTH,
    RNR_QUAD_UBUS_SLV_RES_CNTRL_DEV_TIMEOUT_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_UBUS_SLV_RES_CNTRL_DEV_TIMEOUT
 ******************************************************************************/
const ru_field_rec RNR_QUAD_UBUS_SLV_RES_CNTRL_DEV_TIMEOUT_FIELD =
{
    "DEV_TIMEOUT",
#if RU_INCLUDE_DESC
    "dev_timeout",
    "device timeout",
#endif
    RNR_QUAD_UBUS_SLV_RES_CNTRL_DEV_TIMEOUT_FIELD_MASK,
    0,
    RNR_QUAD_UBUS_SLV_RES_CNTRL_DEV_TIMEOUT_FIELD_WIDTH,
    RNR_QUAD_UBUS_SLV_RES_CNTRL_DEV_TIMEOUT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_UBUS_SLV_VPB_BASE_BASE
 ******************************************************************************/
const ru_field_rec RNR_QUAD_UBUS_SLV_VPB_BASE_BASE_FIELD =
{
    "BASE",
#if RU_INCLUDE_DESC
    "Base",
    "base",
#endif
    RNR_QUAD_UBUS_SLV_VPB_BASE_BASE_FIELD_MASK,
    0,
    RNR_QUAD_UBUS_SLV_VPB_BASE_BASE_FIELD_WIDTH,
    RNR_QUAD_UBUS_SLV_VPB_BASE_BASE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_UBUS_SLV_VPB_MASK_MASK
 ******************************************************************************/
const ru_field_rec RNR_QUAD_UBUS_SLV_VPB_MASK_MASK_FIELD =
{
    "MASK",
#if RU_INCLUDE_DESC
    "Mask",
    "mask",
#endif
    RNR_QUAD_UBUS_SLV_VPB_MASK_MASK_FIELD_MASK,
    0,
    RNR_QUAD_UBUS_SLV_VPB_MASK_MASK_FIELD_WIDTH,
    RNR_QUAD_UBUS_SLV_VPB_MASK_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_UBUS_SLV_APB_BASE_BASE
 ******************************************************************************/
const ru_field_rec RNR_QUAD_UBUS_SLV_APB_BASE_BASE_FIELD =
{
    "BASE",
#if RU_INCLUDE_DESC
    "Base",
    "base",
#endif
    RNR_QUAD_UBUS_SLV_APB_BASE_BASE_FIELD_MASK,
    0,
    RNR_QUAD_UBUS_SLV_APB_BASE_BASE_FIELD_WIDTH,
    RNR_QUAD_UBUS_SLV_APB_BASE_BASE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_UBUS_SLV_APB_MASK_MASK
 ******************************************************************************/
const ru_field_rec RNR_QUAD_UBUS_SLV_APB_MASK_MASK_FIELD =
{
    "MASK",
#if RU_INCLUDE_DESC
    "Mask",
    "mask",
#endif
    RNR_QUAD_UBUS_SLV_APB_MASK_MASK_FIELD_MASK,
    0,
    RNR_QUAD_UBUS_SLV_APB_MASK_MASK_FIELD_WIDTH,
    RNR_QUAD_UBUS_SLV_APB_MASK_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_UBUS_SLV_DQM_BASE_BASE
 ******************************************************************************/
const ru_field_rec RNR_QUAD_UBUS_SLV_DQM_BASE_BASE_FIELD =
{
    "BASE",
#if RU_INCLUDE_DESC
    "Base",
    "base",
#endif
    RNR_QUAD_UBUS_SLV_DQM_BASE_BASE_FIELD_MASK,
    0,
    RNR_QUAD_UBUS_SLV_DQM_BASE_BASE_FIELD_WIDTH,
    RNR_QUAD_UBUS_SLV_DQM_BASE_BASE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_UBUS_SLV_DQM_MASK_MASK
 ******************************************************************************/
const ru_field_rec RNR_QUAD_UBUS_SLV_DQM_MASK_MASK_FIELD =
{
    "MASK",
#if RU_INCLUDE_DESC
    "Mask",
    "mask",
#endif
    RNR_QUAD_UBUS_SLV_DQM_MASK_MASK_FIELD_MASK,
    0,
    RNR_QUAD_UBUS_SLV_DQM_MASK_MASK_FIELD_WIDTH,
    RNR_QUAD_UBUS_SLV_DQM_MASK_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_ENG_CFG
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_ENG_CFG_FIELD =
{
    "CFG",
#if RU_INCLUDE_DESC
    "CFG",
    "eng_cnfg[0] - IP filters on IPV6 on LSByte not MSByte"
    "eng_cnfg[1] - IP DA fields shows LSByte not MSbyte"
    "eng_cnfg[2] - enable error instead of brdcst at classifier summary word"
    "eng_cnfg[3]  - Free"
    "eng_cnfg[8]  - Free"
    "eng_cnfg[9]  - Free"
    "eng_cnfg[10] - Free"
    "eng_cnfg[11] - Free"
    "eng_cnfg[12] - Free"
    "eng_cnfg[13] - Free"
    "eng_cnfg[14] - mask of ah ext header excepetion"
    "eng_cnfg[15] - enable old mode of AH at IPV6"
    "eng_cnfg[16] - enable old mode of AH at IPV4"
    "eng_cnfg[17] - Free"
    "eng_cnfg[18] - dont allow 0xFFFF as valid ipv4 header cksum results"
    "s",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_ENG_CFG_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_ENG_CFG_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_ENG_CFG_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG_EXCEPTION_EN
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG_EXCEPTION_EN_FIELD =
{
    "EXCEPTION_EN",
#if RU_INCLUDE_DESC
    "EXCEPTION_EN",
    "Define which status bit cause exception bit in summary word to be set, the cuse vector is {4h0,ip fragment, ip version error, ip checksum error, ip_header length error}",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG_EXCEPTION_EN_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG_EXCEPTION_EN_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG_EXCEPTION_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG_RESERVED0
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG_RESERVED0_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG_RESERVED0_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG_TCP_FLAGS_FILT
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG_TCP_FLAGS_FILT_FIELD =
{
    "TCP_FLAGS_FILT",
#if RU_INCLUDE_DESC
    "TCP_FLAGS_TCP_FILTER",
    "Defines which TCP falgs set will cause TCP_FLAG bit in summary word to be set",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG_TCP_FLAGS_FILT_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG_TCP_FLAGS_FILT_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG_TCP_FLAGS_FILT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG_RESERVED1
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG_RESERVED1_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG_RESERVED1_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG_PROFILE_US
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG_PROFILE_US_FIELD =
{
    "PROFILE_US",
#if RU_INCLUDE_DESC
    "PROFILE_US",
    "Profile US",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG_PROFILE_US_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG_PROFILE_US_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG_PROFILE_US_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG_RESERVED2
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG_RESERVED2_FIELD =
{
    "RESERVED2",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG_RESERVED2_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG_RESERVED2_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG_RESERVED2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_0_1_VID_0
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_0_1_VID_0_FIELD =
{
    "VID_0",
#if RU_INCLUDE_DESC
    "VID_0",
    "VLAN ID Filter for first VLAN of register",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_0_1_VID_0_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_0_1_VID_0_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_0_1_VID_0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_0_1_RESERVED0
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_0_1_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_0_1_RESERVED0_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_0_1_RESERVED0_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_0_1_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_0_1_VID_0_EN
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_0_1_VID_0_EN_FIELD =
{
    "VID_0_EN",
#if RU_INCLUDE_DESC
    "VID_0_Enable",
    "VLAND ID Filter 0 Enable",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_0_1_VID_0_EN_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_0_1_VID_0_EN_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_0_1_VID_0_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_0_1_VID_1
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_0_1_VID_1_FIELD =
{
    "VID_1",
#if RU_INCLUDE_DESC
    "VID_1",
    "VLAN ID Filter 1 for second VLAN of register",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_0_1_VID_1_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_0_1_VID_1_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_0_1_VID_1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_0_1_RESERVED1
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_0_1_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_0_1_RESERVED1_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_0_1_RESERVED1_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_0_1_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_0_1_VID_1_EN
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_0_1_VID_1_EN_FIELD =
{
    "VID_1_EN",
#if RU_INCLUDE_DESC
    "VID_1_Enable",
    "VLAND ID Filter 1 Enable",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_0_1_VID_1_EN_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_0_1_VID_1_EN_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_0_1_VID_1_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_2_3_VID_2
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_2_3_VID_2_FIELD =
{
    "VID_2",
#if RU_INCLUDE_DESC
    "VID_2",
    "VLAN ID Filter for first VLAN of register",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_2_3_VID_2_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_2_3_VID_2_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_2_3_VID_2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_2_3_RESERVED0
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_2_3_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_2_3_RESERVED0_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_2_3_RESERVED0_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_2_3_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_2_3_VID_2_EN
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_2_3_VID_2_EN_FIELD =
{
    "VID_2_EN",
#if RU_INCLUDE_DESC
    "VID_2_Enable",
    "VLAND ID Filter 2 Enable",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_2_3_VID_2_EN_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_2_3_VID_2_EN_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_2_3_VID_2_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_2_3_VID_3
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_2_3_VID_3_FIELD =
{
    "VID_3",
#if RU_INCLUDE_DESC
    "VID_3",
    "VLAN ID Filter 3 ofr second VLAN of register",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_2_3_VID_3_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_2_3_VID_3_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_2_3_VID_3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_2_3_RESERVED1
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_2_3_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_2_3_RESERVED1_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_2_3_RESERVED1_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_2_3_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_2_3_VID_3_EN
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_2_3_VID_3_EN_FIELD =
{
    "VID_3_EN",
#if RU_INCLUDE_DESC
    "VID_3_Enable",
    "VLAND ID Filter 3 Enable",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_2_3_VID_3_EN_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_2_3_VID_3_EN_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_2_3_VID_3_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_4_5_VID_4
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_4_5_VID_4_FIELD =
{
    "VID_4",
#if RU_INCLUDE_DESC
    "VID_4",
    "VLAN ID Filter for first VLAN of register",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_4_5_VID_4_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_4_5_VID_4_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_4_5_VID_4_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_4_5_RESERVED0
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_4_5_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_4_5_RESERVED0_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_4_5_RESERVED0_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_4_5_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_4_5_VID_4_EN
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_4_5_VID_4_EN_FIELD =
{
    "VID_4_EN",
#if RU_INCLUDE_DESC
    "VID_4_Enable",
    "VLAND ID Filter 4 Enable",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_4_5_VID_4_EN_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_4_5_VID_4_EN_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_4_5_VID_4_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_4_5_VID_5
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_4_5_VID_5_FIELD =
{
    "VID_5",
#if RU_INCLUDE_DESC
    "VID_5",
    "VLAN ID Filter 5 ofr second VLAN of register",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_4_5_VID_5_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_4_5_VID_5_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_4_5_VID_5_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_4_5_RESERVED1
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_4_5_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_4_5_RESERVED1_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_4_5_RESERVED1_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_4_5_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_4_5_VID_5_EN
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_4_5_VID_5_EN_FIELD =
{
    "VID_5_EN",
#if RU_INCLUDE_DESC
    "VID_5_Enable",
    "VLAND ID Filter 5 Enable",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_4_5_VID_5_EN_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_4_5_VID_5_EN_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_4_5_VID_5_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_6_7_VID_6
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_6_7_VID_6_FIELD =
{
    "VID_6",
#if RU_INCLUDE_DESC
    "VID_6",
    "VLAN ID Filter for first VLAN of register",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_6_7_VID_6_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_6_7_VID_6_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_6_7_VID_6_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_6_7_RESERVED0
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_6_7_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_6_7_RESERVED0_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_6_7_RESERVED0_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_6_7_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_6_7_VID_6_EN
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_6_7_VID_6_EN_FIELD =
{
    "VID_6_EN",
#if RU_INCLUDE_DESC
    "VID_6_Enable",
    "VLAND ID Filter 6 Enable",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_6_7_VID_6_EN_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_6_7_VID_6_EN_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_6_7_VID_6_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_6_7_VID_7
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_6_7_VID_7_FIELD =
{
    "VID_7",
#if RU_INCLUDE_DESC
    "VID_7",
    "VLAN ID Filter 7 ofr second VLAN of register",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_6_7_VID_7_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_6_7_VID_7_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_6_7_VID_7_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_6_7_RESERVED1
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_6_7_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_6_7_RESERVED1_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_6_7_RESERVED1_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_6_7_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_6_7_VID_7_EN
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_6_7_VID_7_EN_FIELD =
{
    "VID_7_EN",
#if RU_INCLUDE_DESC
    "VID_7_Enable",
    "VLAND ID Filter 7 Enable",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_6_7_VID_7_EN_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_6_7_VID_7_EN_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_6_7_VID_7_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER0_CFG_IP_ADDRESS
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER0_CFG_IP_ADDRESS_FIELD =
{
    "IP_ADDRESS",
#if RU_INCLUDE_DESC
    "IP_address",
    "32-bit address to match SIP or DIP (according to predefined configuration in IP_FILTERS_CFG register)",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER0_CFG_IP_ADDRESS_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER0_CFG_IP_ADDRESS_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER0_CFG_IP_ADDRESS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER1_CFG_IP_ADDRESS
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER1_CFG_IP_ADDRESS_FIELD =
{
    "IP_ADDRESS",
#if RU_INCLUDE_DESC
    "IP_address",
    "32-bit address to match SIP or DIP (according to predefined configuration in IP_FILTERS_CFG register)",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER1_CFG_IP_ADDRESS_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER1_CFG_IP_ADDRESS_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER1_CFG_IP_ADDRESS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER0_MASK_CFG_IP_ADDRESS_MASK
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER0_MASK_CFG_IP_ADDRESS_MASK_FIELD =
{
    "IP_ADDRESS_MASK",
#if RU_INCLUDE_DESC
    "IP_address_mask",
    "32-bit address mask",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER0_MASK_CFG_IP_ADDRESS_MASK_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER0_MASK_CFG_IP_ADDRESS_MASK_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER0_MASK_CFG_IP_ADDRESS_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER1_MASK_CFG_IP_ADDRESS_MASK
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER1_MASK_CFG_IP_ADDRESS_MASK_FIELD =
{
    "IP_ADDRESS_MASK",
#if RU_INCLUDE_DESC
    "IP_address_mask",
    "32-bit address mask",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER1_MASK_CFG_IP_ADDRESS_MASK_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER1_MASK_CFG_IP_ADDRESS_MASK_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER1_MASK_CFG_IP_ADDRESS_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_IP_FILTER0_DIP_EN
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_IP_FILTER0_DIP_EN_FIELD =
{
    "IP_FILTER0_DIP_EN",
#if RU_INCLUDE_DESC
    "IP_FILTER0_DIP_EN",
    "IP Filter0 DIP or SIP selection."
    "The default is SIP, when the field is set -> DIP selection is enabled",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_IP_FILTER0_DIP_EN_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_IP_FILTER0_DIP_EN_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_IP_FILTER0_DIP_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_IP_FILTER1_DIP_EN
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_IP_FILTER1_DIP_EN_FIELD =
{
    "IP_FILTER1_DIP_EN",
#if RU_INCLUDE_DESC
    "IP_FILTER1_DIP_EN",
    "IP Filter1 DIP or SIP selection."
    "The default is SIP, when the field is set -> DIP selection is enabled",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_IP_FILTER1_DIP_EN_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_IP_FILTER1_DIP_EN_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_IP_FILTER1_DIP_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_IP_FILTER2_DIP_EN
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_IP_FILTER2_DIP_EN_FIELD =
{
    "IP_FILTER2_DIP_EN",
#if RU_INCLUDE_DESC
    "IP_FILTER2_DIP_EN",
    "IP Filter2 DIP or SIP selection."
    "The default is SIP, when the field is set -> DIP selection is enabled",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_IP_FILTER2_DIP_EN_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_IP_FILTER2_DIP_EN_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_IP_FILTER2_DIP_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_IP_FILTER3_DIP_EN
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_IP_FILTER3_DIP_EN_FIELD =
{
    "IP_FILTER3_DIP_EN",
#if RU_INCLUDE_DESC
    "IP_FILTER3_DIP_EN",
    "IP Filter3 DIP or SIP selection."
    "The default is SIP, when the field is set -> DIP selection is enabled",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_IP_FILTER3_DIP_EN_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_IP_FILTER3_DIP_EN_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_IP_FILTER3_DIP_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_IP_FILTER0_VALID
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_IP_FILTER0_VALID_FIELD =
{
    "IP_FILTER0_VALID",
#if RU_INCLUDE_DESC
    "IP_FILTER0_VALID",
    "IP Filter0 valid bit."
    ""
    "When the bit valid is set, the IP filter/mask can be applied by hardware.",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_IP_FILTER0_VALID_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_IP_FILTER0_VALID_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_IP_FILTER0_VALID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_IP_FILTER1_VALID
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_IP_FILTER1_VALID_FIELD =
{
    "IP_FILTER1_VALID",
#if RU_INCLUDE_DESC
    "IP_FILTER1_VALID",
    "IP Filter1 valid bit."
    ""
    "When the bit valid is set, the IP filter/mask can be applied by hardware.",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_IP_FILTER1_VALID_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_IP_FILTER1_VALID_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_IP_FILTER1_VALID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_IP_FILTER2_VALID
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_IP_FILTER2_VALID_FIELD =
{
    "IP_FILTER2_VALID",
#if RU_INCLUDE_DESC
    "IP_FILTER2_VALID",
    "IP Filter2 valid bit."
    ""
    "When the bit valid is set, the IP filter/mask can be applied by hardware.",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_IP_FILTER2_VALID_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_IP_FILTER2_VALID_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_IP_FILTER2_VALID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_IP_FILTER3_VALID
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_IP_FILTER3_VALID_FIELD =
{
    "IP_FILTER3_VALID",
#if RU_INCLUDE_DESC
    "IP_FILTER3_VALID",
    "IP Filter3 valid bit."
    ""
    "When the bit valid is set, the IP filter/mask can be applied by hardware.",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_IP_FILTER3_VALID_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_IP_FILTER3_VALID_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_IP_FILTER3_VALID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_RESERVED0
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_RESERVED0_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_RESERVED0_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_SNAP_ORG_CODE_CODE
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_SNAP_ORG_CODE_CODE_FIELD =
{
    "CODE",
#if RU_INCLUDE_DESC
    "Organization_Code",
    "Used defined SNAP organization code",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_SNAP_ORG_CODE_CODE_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_SNAP_ORG_CODE_CODE_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_SNAP_ORG_CODE_CODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_SNAP_ORG_CODE_EN_RFC1042
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_SNAP_ORG_CODE_EN_RFC1042_FIELD =
{
    "EN_RFC1042",
#if RU_INCLUDE_DESC
    "RFC1042_ethernet_encapsulation_enable",
    "enable RFC1042 0x00000 organization code",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_SNAP_ORG_CODE_EN_RFC1042_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_SNAP_ORG_CODE_EN_RFC1042_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_SNAP_ORG_CODE_EN_RFC1042_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_SNAP_ORG_CODE_EN_8021Q
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_SNAP_ORG_CODE_EN_8021Q_FIELD =
{
    "EN_8021Q",
#if RU_INCLUDE_DESC
    "802.1Q_ehternet_encapsulation",
    "enables 802.1Q 0x0000f8 organization code",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_SNAP_ORG_CODE_EN_8021Q_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_SNAP_ORG_CODE_EN_8021Q_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_SNAP_ORG_CODE_EN_8021Q_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_SNAP_ORG_CODE_RESERVED0
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_SNAP_ORG_CODE_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_SNAP_ORG_CODE_RESERVED0_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_SNAP_ORG_CODE_RESERVED0_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_SNAP_ORG_CODE_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_PPP_IP_PROT_CODE_PPP_CODE_0
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_PPP_IP_PROT_CODE_PPP_CODE_0_FIELD =
{
    "PPP_CODE_0",
#if RU_INCLUDE_DESC
    "PPP_Protocol_Code_0",
    "PPP Protocol code to identify L3 is IP",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_PPP_IP_PROT_CODE_PPP_CODE_0_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_PPP_IP_PROT_CODE_PPP_CODE_0_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_PPP_IP_PROT_CODE_PPP_CODE_0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_PPP_IP_PROT_CODE_PPP_CODE_1
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_PPP_IP_PROT_CODE_PPP_CODE_1_FIELD =
{
    "PPP_CODE_1",
#if RU_INCLUDE_DESC
    "PPP_Protocol_Code_1",
    "PPP Protocol code to identify L3 is IP",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_PPP_IP_PROT_CODE_PPP_CODE_1_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_PPP_IP_PROT_CODE_PPP_CODE_1_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_PPP_IP_PROT_CODE_PPP_CODE_1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_ETHTYPE_ETHTYPE_QTAG_0
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_ETHTYPE_ETHTYPE_QTAG_0_FIELD =
{
    "ETHTYPE_QTAG_0",
#if RU_INCLUDE_DESC
    "Ethertyp_for_Qtag_0",
    "Ethertype to identify VLAN QTAG",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_ETHTYPE_ETHTYPE_QTAG_0_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_ETHTYPE_ETHTYPE_QTAG_0_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_ETHTYPE_ETHTYPE_QTAG_0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_ETHTYPE_ETHTYPE_QTAG_1
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_ETHTYPE_ETHTYPE_QTAG_1_FIELD =
{
    "ETHTYPE_QTAG_1",
#if RU_INCLUDE_DESC
    "Ethertyp_for_Qtag_1",
    "Ethertype to identify VLAN QTAG",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_ETHTYPE_ETHTYPE_QTAG_1_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_ETHTYPE_ETHTYPE_QTAG_1_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_ETHTYPE_ETHTYPE_QTAG_1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_0_1_ETHYPE_0
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_0_1_ETHYPE_0_FIELD =
{
    "ETHYPE_0",
#if RU_INCLUDE_DESC
    "User_Ethertype_0",
    "User Ethertype 0",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_0_1_ETHYPE_0_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_0_1_ETHYPE_0_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_0_1_ETHYPE_0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_0_1_ETHYPE_1
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_0_1_ETHYPE_1_FIELD =
{
    "ETHYPE_1",
#if RU_INCLUDE_DESC
    "User_Ethertype_1",
    "User Ethertype 1",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_0_1_ETHYPE_1_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_0_1_ETHYPE_1_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_0_1_ETHYPE_1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_2_3_ETHYPE_2
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_2_3_ETHYPE_2_FIELD =
{
    "ETHYPE_2",
#if RU_INCLUDE_DESC
    "User_Ethertype_2",
    "User Ethertype 2",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_2_3_ETHYPE_2_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_2_3_ETHYPE_2_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_2_3_ETHYPE_2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_2_3_ETHYPE_3
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_2_3_ETHYPE_3_FIELD =
{
    "ETHYPE_3",
#if RU_INCLUDE_DESC
    "User_Ethertype_3",
    "User Ethertype 3",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_2_3_ETHYPE_3_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_2_3_ETHYPE_3_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_2_3_ETHYPE_3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_PROT_0
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_PROT_0_FIELD =
{
    "ETHTYPE_USER_PROT_0",
#if RU_INCLUDE_DESC
    "User_Ethertype_0_protocol",
    "Pointer to L3 protocol for User Ethertype 0 (0 - None, 1-IPv4, 2-IPv6)",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_PROT_0_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_PROT_0_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_PROT_0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_PROT_1
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_PROT_1_FIELD =
{
    "ETHTYPE_USER_PROT_1",
#if RU_INCLUDE_DESC
    "User_Ethertype_1",
    "Pointer to L3 protocol for User Ethertype 1 (0 - None, 1-IPv4, 2-IPv6)",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_PROT_1_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_PROT_1_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_PROT_1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_PROT_2
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_PROT_2_FIELD =
{
    "ETHTYPE_USER_PROT_2",
#if RU_INCLUDE_DESC
    "User_Ethertype_2",
    "Pointer to L3 protocol for User Ethertype 2 (0 - None, 1-IPv4, 2-IPv6)",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_PROT_2_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_PROT_2_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_PROT_2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_PROT_3
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_PROT_3_FIELD =
{
    "ETHTYPE_USER_PROT_3",
#if RU_INCLUDE_DESC
    "User_Ethertype_3",
    "Pointer to L3 protocol for User Ethertype 3 (0 - None, 1-IPv4, 2-IPv6)",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_PROT_3_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_PROT_3_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_PROT_3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_EN
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_EN_FIELD =
{
    "ETHTYPE_USER_EN",
#if RU_INCLUDE_DESC
    "User_Ethertype_Enable",
    "Enable user Ethertype 3-0 (LSB is for user ethertype 0)",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_EN_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_EN_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_RESERVED0
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_RESERVED0_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_RESERVED0_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_OFFSET_0
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_OFFSET_0_FIELD =
{
    "ETHTYPE_USER_OFFSET_0",
#if RU_INCLUDE_DESC
    "User_Ethertype_0_L3_Offset",
    "4 byte offset for User Ethertype 0 L3",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_OFFSET_0_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_OFFSET_0_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_OFFSET_0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_OFFSET_1
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_OFFSET_1_FIELD =
{
    "ETHTYPE_USER_OFFSET_1",
#if RU_INCLUDE_DESC
    "User_Ethertype_1_L3_Offset",
    "4 byte offset for User Ethertype 1 L3",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_OFFSET_1_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_OFFSET_1_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_OFFSET_1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_OFFSET_2
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_OFFSET_2_FIELD =
{
    "ETHTYPE_USER_OFFSET_2",
#if RU_INCLUDE_DESC
    "User_Ethertype_2_L3_Offset",
    "4 byte offset for User Ethertype 2 L3",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_OFFSET_2_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_OFFSET_2_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_OFFSET_2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_OFFSET_3
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_OFFSET_3_FIELD =
{
    "ETHTYPE_USER_OFFSET_3",
#if RU_INCLUDE_DESC
    "User_Ethertype_2_L3_Offset",
    "4 byte offset for User Ethertype 3 L3",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_OFFSET_3_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_OFFSET_3_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_OFFSET_3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_IPV6_HDR_EXT_FLTR_MASK_CFG_HOP_BY_HOP_MATCH
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_IPV6_HDR_EXT_FLTR_MASK_CFG_HOP_BY_HOP_MATCH_FIELD =
{
    "HOP_BY_HOP_MATCH",
#if RU_INCLUDE_DESC
    "hop_by_hop_match",
    "hop by hop match filter mask",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_IPV6_HDR_EXT_FLTR_MASK_CFG_HOP_BY_HOP_MATCH_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_IPV6_HDR_EXT_FLTR_MASK_CFG_HOP_BY_HOP_MATCH_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_IPV6_HDR_EXT_FLTR_MASK_CFG_HOP_BY_HOP_MATCH_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_IPV6_HDR_EXT_FLTR_MASK_CFG_ROUTING_EH
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_IPV6_HDR_EXT_FLTR_MASK_CFG_ROUTING_EH_FIELD =
{
    "ROUTING_EH",
#if RU_INCLUDE_DESC
    "routing_eh",
    "Routing extension header option match filter mask",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_IPV6_HDR_EXT_FLTR_MASK_CFG_ROUTING_EH_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_IPV6_HDR_EXT_FLTR_MASK_CFG_ROUTING_EH_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_IPV6_HDR_EXT_FLTR_MASK_CFG_ROUTING_EH_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_IPV6_HDR_EXT_FLTR_MASK_CFG_DEST_OPT_EH
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_IPV6_HDR_EXT_FLTR_MASK_CFG_DEST_OPT_EH_FIELD =
{
    "DEST_OPT_EH",
#if RU_INCLUDE_DESC
    "dest_opt_eh",
    "Destination Options extension header option match filter mask",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_IPV6_HDR_EXT_FLTR_MASK_CFG_DEST_OPT_EH_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_IPV6_HDR_EXT_FLTR_MASK_CFG_DEST_OPT_EH_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_IPV6_HDR_EXT_FLTR_MASK_CFG_DEST_OPT_EH_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_IPV6_HDR_EXT_FLTR_MASK_CFG_RESERVED0
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_IPV6_HDR_EXT_FLTR_MASK_CFG_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_IPV6_HDR_EXT_FLTR_MASK_CFG_RESERVED0_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_IPV6_HDR_EXT_FLTR_MASK_CFG_RESERVED0_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_IPV6_HDR_EXT_FLTR_MASK_CFG_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_QTAG_NEST_0_PROFILE_0
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_QTAG_NEST_0_PROFILE_0_FIELD =
{
    "QTAG_NEST_0_PROFILE_0",
#if RU_INCLUDE_DESC
    "QTAG_NEST_0_PROFILE_0",
    "Set to enable Ethertype_qTag 0 as outer (LSB)  2nd VLAN (2nd), 3rd VLAN (MSB)",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_QTAG_NEST_0_PROFILE_0_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_QTAG_NEST_0_PROFILE_0_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_QTAG_NEST_0_PROFILE_0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_QTAG_NEST_0_PROFILE_1
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_QTAG_NEST_0_PROFILE_1_FIELD =
{
    "QTAG_NEST_0_PROFILE_1",
#if RU_INCLUDE_DESC
    "QTAG_NEST_0_PROFILE_1",
    "Set to enable Ethertype_qTag 0 as outer (LSB)  2nd VLAN (2nd), 3rd VLAN (MSB)",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_QTAG_NEST_0_PROFILE_1_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_QTAG_NEST_0_PROFILE_1_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_QTAG_NEST_0_PROFILE_1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_QTAG_NEST_0_PROFILE_2
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_QTAG_NEST_0_PROFILE_2_FIELD =
{
    "QTAG_NEST_0_PROFILE_2",
#if RU_INCLUDE_DESC
    "QTAG_NEST_0_PROFILE_2",
    "Set to enable Ethertype_qTag 0 as outer (LSB)  2nd VLAN (2nd), 3rd VLAN (MSB)",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_QTAG_NEST_0_PROFILE_2_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_QTAG_NEST_0_PROFILE_2_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_QTAG_NEST_0_PROFILE_2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_QTAG_NEST_1_PROFILE_0
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_QTAG_NEST_1_PROFILE_0_FIELD =
{
    "QTAG_NEST_1_PROFILE_0",
#if RU_INCLUDE_DESC
    "QTAG_NEST_1_PROFILE_0",
    "Set to enable Ethertype_qTag 0 as outer (LSB)  2nd VLAN (2nd), 3rd VLAN (MSB)",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_QTAG_NEST_1_PROFILE_0_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_QTAG_NEST_1_PROFILE_0_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_QTAG_NEST_1_PROFILE_0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_QTAG_NEST_1_PROFILE_1
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_QTAG_NEST_1_PROFILE_1_FIELD =
{
    "QTAG_NEST_1_PROFILE_1",
#if RU_INCLUDE_DESC
    "QTAG_NEST_1_PROFILE_1",
    "Set to enable Ethertype_qTag 0 as outer (LSB)  2nd VLAN (2nd), 3rd VLAN (MSB)",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_QTAG_NEST_1_PROFILE_1_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_QTAG_NEST_1_PROFILE_1_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_QTAG_NEST_1_PROFILE_1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_QTAG_NEST_1_PROFILE_2
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_QTAG_NEST_1_PROFILE_2_FIELD =
{
    "QTAG_NEST_1_PROFILE_2",
#if RU_INCLUDE_DESC
    "QTAG_NEST_1_PROFILE_2",
    "Set to enable Ethertype_qTag 0 as outer (LSB)  2nd VLAN (2nd), 3rd VLAN (MSB)",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_QTAG_NEST_1_PROFILE_2_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_QTAG_NEST_1_PROFILE_2_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_QTAG_NEST_1_PROFILE_2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_RESERVED0
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_RESERVED0_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_RESERVED0_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_0_HARD_NEST_PROFILE
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_0_HARD_NEST_PROFILE_FIELD =
{
    "HARD_NEST_PROFILE",
#if RU_INCLUDE_DESC
    "HARD_NEST_PROFILE",
    "bit 2-0: Enable 8100 as VLAN for outer, 2nd, and inner VLANs (inner is bit 2)."
    "bit 5-3: Enable 88a8 as VLAN for outer, 2nd, and inner VLANs."
    "bit 8-6: Enable 9100 as VLAN for outer, 2nd, and inner VLANs."
    "bit 11-9: Enable 9200 as VLAN for outer, 2nd, and inner VLANs.",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_0_HARD_NEST_PROFILE_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_0_HARD_NEST_PROFILE_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_0_HARD_NEST_PROFILE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_0_RESERVED0
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_0_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_0_RESERVED0_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_0_RESERVED0_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_0_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_1_HARD_NEST_PROFILE
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_1_HARD_NEST_PROFILE_FIELD =
{
    "HARD_NEST_PROFILE",
#if RU_INCLUDE_DESC
    "HARD_NEST_PROFILE",
    "Hard Nest Profile",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_1_HARD_NEST_PROFILE_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_1_HARD_NEST_PROFILE_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_1_HARD_NEST_PROFILE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_1_RESERVED0
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_1_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_1_RESERVED0_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_1_RESERVED0_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_1_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_2_HARD_NEST_PROFILE
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_2_HARD_NEST_PROFILE_FIELD =
{
    "HARD_NEST_PROFILE",
#if RU_INCLUDE_DESC
    "HARD_NEST_PROFILE",
    "Hard Nest Profile",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_2_HARD_NEST_PROFILE_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_2_HARD_NEST_PROFILE_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_2_HARD_NEST_PROFILE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_2_RESERVED0
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_2_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_2_RESERVED0_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_2_RESERVED0_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_2_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_IP_PROT_USER_IP_PROT_0
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_IP_PROT_USER_IP_PROT_0_FIELD =
{
    "USER_IP_PROT_0",
#if RU_INCLUDE_DESC
    "USER_IP_protocol_0",
    "User defined IP protocol 0 (value to be matched to IP protocol field)",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_IP_PROT_USER_IP_PROT_0_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_IP_PROT_USER_IP_PROT_0_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_IP_PROT_USER_IP_PROT_0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_IP_PROT_USER_IP_PROT_1
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_IP_PROT_USER_IP_PROT_1_FIELD =
{
    "USER_IP_PROT_1",
#if RU_INCLUDE_DESC
    "USER_IP_protocol_1",
    "User defined IP protocol 1 (value to be matched to IP protocol field)",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_IP_PROT_USER_IP_PROT_1_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_IP_PROT_USER_IP_PROT_1_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_IP_PROT_USER_IP_PROT_1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_IP_PROT_USER_IP_PROT_2
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_IP_PROT_USER_IP_PROT_2_FIELD =
{
    "USER_IP_PROT_2",
#if RU_INCLUDE_DESC
    "USER_IP_protocol_2",
    "User defined IP protocol 2 (value to be matched to IP protocol field)",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_IP_PROT_USER_IP_PROT_2_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_IP_PROT_USER_IP_PROT_2_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_IP_PROT_USER_IP_PROT_2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_IP_PROT_USER_IP_PROT_3
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_IP_PROT_USER_IP_PROT_3_FIELD =
{
    "USER_IP_PROT_3",
#if RU_INCLUDE_DESC
    "USER__IP_protocol_3",
    "User defined IP protocol 3 (value to be matched to IP protocol field)",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_IP_PROT_USER_IP_PROT_3_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_IP_PROT_USER_IP_PROT_3_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_IP_PROT_USER_IP_PROT_3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_L_DA_FILT_LSB
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_L_DA_FILT_LSB_FIELD =
{
    "DA_FILT_LSB",
#if RU_INCLUDE_DESC
    "DA_FILT_LSB",
    "DA Filter bits 31:0",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_L_DA_FILT_LSB_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_L_DA_FILT_LSB_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_L_DA_FILT_LSB_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_H_DA_FILT_MSB
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_H_DA_FILT_MSB_FIELD =
{
    "DA_FILT_MSB",
#if RU_INCLUDE_DESC
    "DA_FILT_MSB",
    "Current DA Filter bits 47:32",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_H_DA_FILT_MSB_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_H_DA_FILT_MSB_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_H_DA_FILT_MSB_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_H_RESERVED0
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_H_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_H_RESERVED0_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_H_RESERVED0_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_H_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_VAL_L_DA_FILT_LSB
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_VAL_L_DA_FILT_LSB_FIELD =
{
    "DA_FILT_LSB",
#if RU_INCLUDE_DESC
    "DA_FILT_LSB",
    "DA Filter bits 31:0",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_VAL_L_DA_FILT_LSB_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_VAL_L_DA_FILT_LSB_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_VAL_L_DA_FILT_LSB_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_VAL_H_DA_FILT_MSB
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_VAL_H_DA_FILT_MSB_FIELD =
{
    "DA_FILT_MSB",
#if RU_INCLUDE_DESC
    "DA_FILT_MSB",
    "Current DA Filter bits 47:32",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_VAL_H_DA_FILT_MSB_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_VAL_H_DA_FILT_MSB_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_VAL_H_DA_FILT_MSB_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_VAL_H_RESERVED0
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_VAL_H_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_VAL_H_RESERVED0_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_VAL_H_RESERVED0_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_VAL_H_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_L_DA_FILT_LSB
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_L_DA_FILT_LSB_FIELD =
{
    "DA_FILT_LSB",
#if RU_INCLUDE_DESC
    "DA_FILT_LSB",
    "DA Filter bits 31:0",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_L_DA_FILT_LSB_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_L_DA_FILT_LSB_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_L_DA_FILT_LSB_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_H_DA_FILT_MSB
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_H_DA_FILT_MSB_FIELD =
{
    "DA_FILT_MSB",
#if RU_INCLUDE_DESC
    "DA_FILT_MSB",
    "Current DA Filter bits 47:32",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_H_DA_FILT_MSB_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_H_DA_FILT_MSB_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_H_DA_FILT_MSB_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_H_RESERVED0
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_H_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_H_RESERVED0_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_H_RESERVED0_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_H_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_L_DA_FILT_LSB
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_L_DA_FILT_LSB_FIELD =
{
    "DA_FILT_LSB",
#if RU_INCLUDE_DESC
    "DA_FILT_LSB",
    "DA Filter bits 31:0",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_L_DA_FILT_LSB_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_L_DA_FILT_LSB_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_L_DA_FILT_LSB_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_H_DA_FILT_MSB
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_H_DA_FILT_MSB_FIELD =
{
    "DA_FILT_MSB",
#if RU_INCLUDE_DESC
    "DA_FILT_MSB",
    "Current DA Filter bits 47:32",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_H_DA_FILT_MSB_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_H_DA_FILT_MSB_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_H_DA_FILT_MSB_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_H_RESERVED0
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_H_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_H_RESERVED0_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_H_RESERVED0_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_H_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_L_DA_FILT_LSB
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_L_DA_FILT_LSB_FIELD =
{
    "DA_FILT_LSB",
#if RU_INCLUDE_DESC
    "DA_FILT_LSB",
    "DA Filter bits 31:0",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_L_DA_FILT_LSB_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_L_DA_FILT_LSB_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_L_DA_FILT_LSB_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_H_DA_FILT_MSB
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_H_DA_FILT_MSB_FIELD =
{
    "DA_FILT_MSB",
#if RU_INCLUDE_DESC
    "DA_FILT_MSB",
    "Current DA Filter bits 47:32",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_H_DA_FILT_MSB_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_H_DA_FILT_MSB_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_H_DA_FILT_MSB_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_H_RESERVED0
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_H_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_H_RESERVED0_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_H_RESERVED0_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_H_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_L_DA_FILT_LSB
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_L_DA_FILT_LSB_FIELD =
{
    "DA_FILT_LSB",
#if RU_INCLUDE_DESC
    "DA_FILT_LSB",
    "DA Filter bits 31:0",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_L_DA_FILT_LSB_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_L_DA_FILT_LSB_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_L_DA_FILT_LSB_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_H_DA_FILT_MSB
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_H_DA_FILT_MSB_FIELD =
{
    "DA_FILT_MSB",
#if RU_INCLUDE_DESC
    "DA_FILT_MSB",
    "Current DA Filter bits 47:32",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_H_DA_FILT_MSB_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_H_DA_FILT_MSB_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_H_DA_FILT_MSB_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_H_RESERVED0
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_H_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_H_RESERVED0_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_H_RESERVED0_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_H_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_L_DA_FILT_LSB
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_L_DA_FILT_LSB_FIELD =
{
    "DA_FILT_LSB",
#if RU_INCLUDE_DESC
    "DA_FILT_LSB",
    "DA Filter bits 31:0",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_L_DA_FILT_LSB_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_L_DA_FILT_LSB_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_L_DA_FILT_LSB_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_H_DA_FILT_MSB
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_H_DA_FILT_MSB_FIELD =
{
    "DA_FILT_MSB",
#if RU_INCLUDE_DESC
    "DA_FILT_MSB",
    "Current DA Filter bits 47:32",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_H_DA_FILT_MSB_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_H_DA_FILT_MSB_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_H_DA_FILT_MSB_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_H_RESERVED0
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_H_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_H_RESERVED0_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_H_RESERVED0_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_H_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_L_DA_FILT_LSB
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_L_DA_FILT_LSB_FIELD =
{
    "DA_FILT_LSB",
#if RU_INCLUDE_DESC
    "DA_FILT_LSB",
    "DA Filter bits 31:0",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_L_DA_FILT_LSB_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_L_DA_FILT_LSB_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_L_DA_FILT_LSB_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_H_DA_FILT_MSB
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_H_DA_FILT_MSB_FIELD =
{
    "DA_FILT_MSB",
#if RU_INCLUDE_DESC
    "DA_FILT_MSB",
    "Current DA Filter bits 47:32",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_H_DA_FILT_MSB_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_H_DA_FILT_MSB_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_H_DA_FILT_MSB_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_H_RESERVED0
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_H_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_H_RESERVED0_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_H_RESERVED0_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_H_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT8_VAL_L_DA_FILT_LSB
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT8_VAL_L_DA_FILT_LSB_FIELD =
{
    "DA_FILT_LSB",
#if RU_INCLUDE_DESC
    "DA_FILT_LSB",
    "DA Filter bits 31:0",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT8_VAL_L_DA_FILT_LSB_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT8_VAL_L_DA_FILT_LSB_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT8_VAL_L_DA_FILT_LSB_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT8_VAL_H_DA_FILT_MSB
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT8_VAL_H_DA_FILT_MSB_FIELD =
{
    "DA_FILT_MSB",
#if RU_INCLUDE_DESC
    "DA_FILT_MSB",
    "Current DA Filter bits 47:32",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT8_VAL_H_DA_FILT_MSB_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT8_VAL_H_DA_FILT_MSB_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT8_VAL_H_DA_FILT_MSB_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT8_VAL_H_RESERVED0
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT8_VAL_H_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT8_VAL_H_RESERVED0_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT8_VAL_H_RESERVED0_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT8_VAL_H_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_L_DA_FILT_MASK_L
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_L_DA_FILT_MASK_L_FIELD =
{
    "DA_FILT_MASK_L",
#if RU_INCLUDE_DESC
    "DA_FILT_MASK_L",
    "Current DA Filter mask bits 31:0",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_L_DA_FILT_MASK_L_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_L_DA_FILT_MASK_L_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_L_DA_FILT_MASK_L_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_H_DA_FILT_MASK_MSB
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_H_DA_FILT_MASK_MSB_FIELD =
{
    "DA_FILT_MASK_MSB",
#if RU_INCLUDE_DESC
    "DA_FILT_MASK_MSB",
    "Current DA Filter mask bits 47:32",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_H_DA_FILT_MASK_MSB_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_H_DA_FILT_MASK_MSB_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_H_DA_FILT_MASK_MSB_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_H_RESERVED0
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_H_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_H_RESERVED0_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_H_RESERVED0_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_H_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_MASK_L_DA_FILT_MASK_L
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_MASK_L_DA_FILT_MASK_L_FIELD =
{
    "DA_FILT_MASK_L",
#if RU_INCLUDE_DESC
    "DA_FILT_MASK_L",
    "Current DA Filter mask bits 31:0",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_MASK_L_DA_FILT_MASK_L_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_MASK_L_DA_FILT_MASK_L_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_MASK_L_DA_FILT_MASK_L_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_MASK_H_DA_FILT_MASK_MSB
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_MASK_H_DA_FILT_MASK_MSB_FIELD =
{
    "DA_FILT_MASK_MSB",
#if RU_INCLUDE_DESC
    "DA_FILT_MASK_MSB",
    "Current DA Filter mask bits 47:32",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_MASK_H_DA_FILT_MASK_MSB_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_MASK_H_DA_FILT_MASK_MSB_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_MASK_H_DA_FILT_MASK_MSB_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_MASK_H_RESERVED0
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_MASK_H_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_MASK_H_RESERVED0_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_MASK_H_RESERVED0_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_MASK_H_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT0_VALID
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT0_VALID_FIELD =
{
    "DA_FILT0_VALID",
#if RU_INCLUDE_DESC
    "DA_FILT0_VALID",
    "DA Filter0 valid bit",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT0_VALID_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT0_VALID_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT0_VALID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT1_VALID
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT1_VALID_FIELD =
{
    "DA_FILT1_VALID",
#if RU_INCLUDE_DESC
    "DA_FILT1_VALID",
    "DA Filter1 valid bit",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT1_VALID_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT1_VALID_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT1_VALID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT2_VALID
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT2_VALID_FIELD =
{
    "DA_FILT2_VALID",
#if RU_INCLUDE_DESC
    "DA_FILT2_VALID",
    "DA Filter2 valid bit",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT2_VALID_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT2_VALID_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT2_VALID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT3_VALID
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT3_VALID_FIELD =
{
    "DA_FILT3_VALID",
#if RU_INCLUDE_DESC
    "DA_FILT3_VALID",
    "DA Filter3 valid bit",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT3_VALID_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT3_VALID_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT3_VALID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT4_VALID
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT4_VALID_FIELD =
{
    "DA_FILT4_VALID",
#if RU_INCLUDE_DESC
    "DA_FILT4_VALID",
    "DA Filter4 valid bit",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT4_VALID_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT4_VALID_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT4_VALID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT5_VALID
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT5_VALID_FIELD =
{
    "DA_FILT5_VALID",
#if RU_INCLUDE_DESC
    "DA_FILT5_VALID",
    "DA Filter5 valid bit",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT5_VALID_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT5_VALID_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT5_VALID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT6_VALID
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT6_VALID_FIELD =
{
    "DA_FILT6_VALID",
#if RU_INCLUDE_DESC
    "DA_FILT6_VALID",
    "DA Filter6 valid bit",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT6_VALID_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT6_VALID_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT6_VALID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT7_VALID
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT7_VALID_FIELD =
{
    "DA_FILT7_VALID",
#if RU_INCLUDE_DESC
    "DA_FILT7_VALID",
    "DA Filter7 valid bit",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT7_VALID_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT7_VALID_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT7_VALID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT8_VALID
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT8_VALID_FIELD =
{
    "DA_FILT8_VALID",
#if RU_INCLUDE_DESC
    "DA_FILT8_VALID",
    "DA Filter8 valid bit",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT8_VALID_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT8_VALID_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_DA_FILT8_VALID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_RESERVED0
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_RESERVED0_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_RESERVED0_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT0_VALID
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT0_VALID_FIELD =
{
    "DA_FILT0_VALID",
#if RU_INCLUDE_DESC
    "DA_FILT0_VALID",
    "DA Filter0 valid bit",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT0_VALID_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT0_VALID_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT0_VALID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT1_VALID
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT1_VALID_FIELD =
{
    "DA_FILT1_VALID",
#if RU_INCLUDE_DESC
    "DA_FILT1_VALID",
    "DA Filter1 valid bit",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT1_VALID_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT1_VALID_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT1_VALID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT2_VALID
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT2_VALID_FIELD =
{
    "DA_FILT2_VALID",
#if RU_INCLUDE_DESC
    "DA_FILT2_VALID",
    "DA Filter2 valid bit",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT2_VALID_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT2_VALID_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT2_VALID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT3_VALID
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT3_VALID_FIELD =
{
    "DA_FILT3_VALID",
#if RU_INCLUDE_DESC
    "DA_FILT3_VALID",
    "DA Filter3 valid bit",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT3_VALID_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT3_VALID_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT3_VALID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT4_VALID
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT4_VALID_FIELD =
{
    "DA_FILT4_VALID",
#if RU_INCLUDE_DESC
    "DA_FILT4_VALID",
    "DA Filter4 valid bit",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT4_VALID_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT4_VALID_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT4_VALID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT5_VALID
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT5_VALID_FIELD =
{
    "DA_FILT5_VALID",
#if RU_INCLUDE_DESC
    "DA_FILT5_VALID",
    "DA Filter5 valid bit",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT5_VALID_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT5_VALID_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT5_VALID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT6_VALID
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT6_VALID_FIELD =
{
    "DA_FILT6_VALID",
#if RU_INCLUDE_DESC
    "DA_FILT6_VALID",
    "DA Filter6 valid bit",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT6_VALID_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT6_VALID_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT6_VALID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT7_VALID
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT7_VALID_FIELD =
{
    "DA_FILT7_VALID",
#if RU_INCLUDE_DESC
    "DA_FILT7_VALID",
    "DA Filter7 valid bit",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT7_VALID_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT7_VALID_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT7_VALID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT8_VALID
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT8_VALID_FIELD =
{
    "DA_FILT8_VALID",
#if RU_INCLUDE_DESC
    "DA_FILT8_VALID",
    "DA Filter8 valid bit",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT8_VALID_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT8_VALID_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_DA_FILT8_VALID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_RESERVED0
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_RESERVED0_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_RESERVED0_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT0_VALID
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT0_VALID_FIELD =
{
    "DA_FILT0_VALID",
#if RU_INCLUDE_DESC
    "DA_FILT0_VALID",
    "DA Filter0 valid bit",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT0_VALID_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT0_VALID_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT0_VALID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT1_VALID
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT1_VALID_FIELD =
{
    "DA_FILT1_VALID",
#if RU_INCLUDE_DESC
    "DA_FILT1_VALID",
    "DA Filter1 valid bit",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT1_VALID_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT1_VALID_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT1_VALID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT2_VALID
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT2_VALID_FIELD =
{
    "DA_FILT2_VALID",
#if RU_INCLUDE_DESC
    "DA_FILT2_VALID",
    "DA Filter2 valid bit",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT2_VALID_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT2_VALID_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT2_VALID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT3_VALID
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT3_VALID_FIELD =
{
    "DA_FILT3_VALID",
#if RU_INCLUDE_DESC
    "DA_FILT3_VALID",
    "DA Filter3 valid bit",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT3_VALID_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT3_VALID_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT3_VALID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT4_VALID
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT4_VALID_FIELD =
{
    "DA_FILT4_VALID",
#if RU_INCLUDE_DESC
    "DA_FILT4_VALID",
    "DA Filter4 valid bit",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT4_VALID_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT4_VALID_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT4_VALID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT5_VALID
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT5_VALID_FIELD =
{
    "DA_FILT5_VALID",
#if RU_INCLUDE_DESC
    "DA_FILT5_VALID",
    "DA Filter5 valid bit",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT5_VALID_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT5_VALID_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT5_VALID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT6_VALID
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT6_VALID_FIELD =
{
    "DA_FILT6_VALID",
#if RU_INCLUDE_DESC
    "DA_FILT6_VALID",
    "DA Filter6 valid bit",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT6_VALID_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT6_VALID_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT6_VALID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT7_VALID
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT7_VALID_FIELD =
{
    "DA_FILT7_VALID",
#if RU_INCLUDE_DESC
    "DA_FILT7_VALID",
    "DA Filter7 valid bit",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT7_VALID_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT7_VALID_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT7_VALID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT8_VALID
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT8_VALID_FIELD =
{
    "DA_FILT8_VALID",
#if RU_INCLUDE_DESC
    "DA_FILT8_VALID",
    "DA Filter8 valid bit",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT8_VALID_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT8_VALID_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_DA_FILT8_VALID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_RESERVED0
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_RESERVED0_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_RESERVED0_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_GRE_PROTOCOL_CFG_GRE_PROTOCOL
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_GRE_PROTOCOL_CFG_GRE_PROTOCOL_FIELD =
{
    "GRE_PROTOCOL",
#if RU_INCLUDE_DESC
    "GRE_PROTOCOL",
    "GRE_PROTOCOL",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_GRE_PROTOCOL_CFG_GRE_PROTOCOL_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_GRE_PROTOCOL_CFG_GRE_PROTOCOL_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_GRE_PROTOCOL_CFG_GRE_PROTOCOL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_GRE_PROTOCOL_CFG_RESERVED0
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_GRE_PROTOCOL_CFG_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_GRE_PROTOCOL_CFG_RESERVED0_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_GRE_PROTOCOL_CFG_RESERVED0_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_GRE_PROTOCOL_CFG_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG_SIZE_PROFILE_0
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG_SIZE_PROFILE_0_FIELD =
{
    "SIZE_PROFILE_0",
#if RU_INCLUDE_DESC
    "SIZE_PROFILE_0",
    "profile 0 tag size, valid values are 0,2,4,6,8"
    "",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG_SIZE_PROFILE_0_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG_SIZE_PROFILE_0_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG_SIZE_PROFILE_0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG_SIZE_PROFILE_1
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG_SIZE_PROFILE_1_FIELD =
{
    "SIZE_PROFILE_1",
#if RU_INCLUDE_DESC
    "SIZE_PROFILE_1",
    "profile 1 tag size, valid values are 0,2,4,6,8"
    "",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG_SIZE_PROFILE_1_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG_SIZE_PROFILE_1_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG_SIZE_PROFILE_1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG_SIZE_PROFILE_2
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG_SIZE_PROFILE_2_FIELD =
{
    "SIZE_PROFILE_2",
#if RU_INCLUDE_DESC
    "SIZE_PROFILE_2",
    "profile 2 tag size, valid values are 0,2,4,6,8"
    "",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG_SIZE_PROFILE_2_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG_SIZE_PROFILE_2_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG_SIZE_PROFILE_2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG_PRE_DA_DPROFILE_0
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG_PRE_DA_DPROFILE_0_FIELD =
{
    "PRE_DA_DPROFILE_0",
#if RU_INCLUDE_DESC
    "PRE_DA_DPROFILE_0",
    "Pre-DA Profile 0",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG_PRE_DA_DPROFILE_0_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG_PRE_DA_DPROFILE_0_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG_PRE_DA_DPROFILE_0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG_PRE_DA_DPROFILE_1
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG_PRE_DA_DPROFILE_1_FIELD =
{
    "PRE_DA_DPROFILE_1",
#if RU_INCLUDE_DESC
    "PRE_DA_DPROFILE_1",
    "Pre-DA Profile 1",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG_PRE_DA_DPROFILE_1_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG_PRE_DA_DPROFILE_1_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG_PRE_DA_DPROFILE_1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG_PRE_DA_DPROFILE_2
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG_PRE_DA_DPROFILE_2_FIELD =
{
    "PRE_DA_DPROFILE_2",
#if RU_INCLUDE_DESC
    "PRE_DA_DPROFILE_2",
    "Pre-DA Profile 2",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG_PRE_DA_DPROFILE_2_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG_PRE_DA_DPROFILE_2_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG_PRE_DA_DPROFILE_2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG_RESERVED0
 ******************************************************************************/
const ru_field_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG_RESERVED0_FIELD_MASK,
    0,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG_RESERVED0_FIELD_WIDTH,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_USE_FIFO_FOR_DDR_ONLY
 ******************************************************************************/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_USE_FIFO_FOR_DDR_ONLY_FIELD =
{
    "USE_FIFO_FOR_DDR_ONLY",
#if RU_INCLUDE_DESC
    "USE_FIFO_FOR_DDR_ONLY",
    "Select whether to use DDR FIFO only for DDR accesses",
#endif
    RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_USE_FIFO_FOR_DDR_ONLY_FIELD_MASK,
    0,
    RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_USE_FIFO_FOR_DDR_ONLY_FIELD_WIDTH,
    RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_USE_FIFO_FOR_DDR_ONLY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_TOKEN_ARBITER_IS_RR
 ******************************************************************************/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_TOKEN_ARBITER_IS_RR_FIELD =
{
    "TOKEN_ARBITER_IS_RR",
#if RU_INCLUDE_DESC
    "TOKEN_ARBITER_IS_RR",
    "Scheduling policy for token arbiter",
#endif
    RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_TOKEN_ARBITER_IS_RR_FIELD_MASK,
    0,
    RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_TOKEN_ARBITER_IS_RR_FIELD_WIDTH,
    RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_TOKEN_ARBITER_IS_RR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_CHICKEN_NO_FLOWCTRL
 ******************************************************************************/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_CHICKEN_NO_FLOWCTRL_FIELD =
{
    "CHICKEN_NO_FLOWCTRL",
#if RU_INCLUDE_DESC
    "CHICKEN_NO_FLOWCTRL",
    "chicken bit to disable external flow control. Packetw wil always be sent, no matter what token count says",
#endif
    RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_CHICKEN_NO_FLOWCTRL_FIELD_MASK,
    0,
    RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_CHICKEN_NO_FLOWCTRL_FIELD_WIDTH,
    RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_CHICKEN_NO_FLOWCTRL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_RESERVED0
 ******************************************************************************/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_RESERVED0_FIELD_MASK,
    0,
    RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_RESERVED0_FIELD_WIDTH,
    RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_CONGEST_THRESHOLD
 ******************************************************************************/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_CONGEST_THRESHOLD_FIELD =
{
    "CONGEST_THRESHOLD",
#if RU_INCLUDE_DESC
    "CONGEST_THRESHOLD",
    "Set congestion threshold",
#endif
    RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_CONGEST_THRESHOLD_FIELD_MASK,
    0,
    RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_CONGEST_THRESHOLD_FIELD_WIDTH,
    RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_CONGEST_THRESHOLD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_RESERVED1
 ******************************************************************************/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_RESERVED1_FIELD_MASK,
    0,
    RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_RESERVED1_FIELD_WIDTH,
    RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_GENERAL_CONFIG_PSRAM0_BASE_VAL
 ******************************************************************************/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_PSRAM0_BASE_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "VAL",
    "Value for base/mask",
#endif
    RNR_QUAD_GENERAL_CONFIG_PSRAM0_BASE_VAL_FIELD_MASK,
    0,
    RNR_QUAD_GENERAL_CONFIG_PSRAM0_BASE_VAL_FIELD_WIDTH,
    RNR_QUAD_GENERAL_CONFIG_PSRAM0_BASE_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_GENERAL_CONFIG_PSRAM1_BASE_VAL
 ******************************************************************************/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_PSRAM1_BASE_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "VAL",
    "Value for base/mask",
#endif
    RNR_QUAD_GENERAL_CONFIG_PSRAM1_BASE_VAL_FIELD_MASK,
    0,
    RNR_QUAD_GENERAL_CONFIG_PSRAM1_BASE_VAL_FIELD_WIDTH,
    RNR_QUAD_GENERAL_CONFIG_PSRAM1_BASE_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_GENERAL_CONFIG_PSRAM2_BASE_VAL
 ******************************************************************************/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_PSRAM2_BASE_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "VAL",
    "Value for base/mask",
#endif
    RNR_QUAD_GENERAL_CONFIG_PSRAM2_BASE_VAL_FIELD_MASK,
    0,
    RNR_QUAD_GENERAL_CONFIG_PSRAM2_BASE_VAL_FIELD_WIDTH,
    RNR_QUAD_GENERAL_CONFIG_PSRAM2_BASE_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_GENERAL_CONFIG_PSRAM3_BASE_VAL
 ******************************************************************************/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_PSRAM3_BASE_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "VAL",
    "Value for base/mask",
#endif
    RNR_QUAD_GENERAL_CONFIG_PSRAM3_BASE_VAL_FIELD_MASK,
    0,
    RNR_QUAD_GENERAL_CONFIG_PSRAM3_BASE_VAL_FIELD_WIDTH,
    RNR_QUAD_GENERAL_CONFIG_PSRAM3_BASE_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_GENERAL_CONFIG_DDR0_BASE_VAL
 ******************************************************************************/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_DDR0_BASE_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "VAL",
    "Value for base/mask",
#endif
    RNR_QUAD_GENERAL_CONFIG_DDR0_BASE_VAL_FIELD_MASK,
    0,
    RNR_QUAD_GENERAL_CONFIG_DDR0_BASE_VAL_FIELD_WIDTH,
    RNR_QUAD_GENERAL_CONFIG_DDR0_BASE_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_GENERAL_CONFIG_DDR1_BASE_VAL
 ******************************************************************************/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_DDR1_BASE_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "VAL",
    "Value for base/mask",
#endif
    RNR_QUAD_GENERAL_CONFIG_DDR1_BASE_VAL_FIELD_MASK,
    0,
    RNR_QUAD_GENERAL_CONFIG_DDR1_BASE_VAL_FIELD_WIDTH,
    RNR_QUAD_GENERAL_CONFIG_DDR1_BASE_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_GENERAL_CONFIG_PSRAM0_MASK_VAL
 ******************************************************************************/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_PSRAM0_MASK_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "VAL",
    "Value for base/mask",
#endif
    RNR_QUAD_GENERAL_CONFIG_PSRAM0_MASK_VAL_FIELD_MASK,
    0,
    RNR_QUAD_GENERAL_CONFIG_PSRAM0_MASK_VAL_FIELD_WIDTH,
    RNR_QUAD_GENERAL_CONFIG_PSRAM0_MASK_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_GENERAL_CONFIG_PSRAM1_MASK_VAL
 ******************************************************************************/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_PSRAM1_MASK_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "VAL",
    "Value for base/mask",
#endif
    RNR_QUAD_GENERAL_CONFIG_PSRAM1_MASK_VAL_FIELD_MASK,
    0,
    RNR_QUAD_GENERAL_CONFIG_PSRAM1_MASK_VAL_FIELD_WIDTH,
    RNR_QUAD_GENERAL_CONFIG_PSRAM1_MASK_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_GENERAL_CONFIG_PSRAM2_MASK_VAL
 ******************************************************************************/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_PSRAM2_MASK_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "VAL",
    "Value for base/mask",
#endif
    RNR_QUAD_GENERAL_CONFIG_PSRAM2_MASK_VAL_FIELD_MASK,
    0,
    RNR_QUAD_GENERAL_CONFIG_PSRAM2_MASK_VAL_FIELD_WIDTH,
    RNR_QUAD_GENERAL_CONFIG_PSRAM2_MASK_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_GENERAL_CONFIG_PSRAM3_MASK_VAL
 ******************************************************************************/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_PSRAM3_MASK_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "VAL",
    "Value for base/mask",
#endif
    RNR_QUAD_GENERAL_CONFIG_PSRAM3_MASK_VAL_FIELD_MASK,
    0,
    RNR_QUAD_GENERAL_CONFIG_PSRAM3_MASK_VAL_FIELD_WIDTH,
    RNR_QUAD_GENERAL_CONFIG_PSRAM3_MASK_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_GENERAL_CONFIG_DDR0_MASK_VAL
 ******************************************************************************/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_DDR0_MASK_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "VAL",
    "Value for base/mask",
#endif
    RNR_QUAD_GENERAL_CONFIG_DDR0_MASK_VAL_FIELD_MASK,
    0,
    RNR_QUAD_GENERAL_CONFIG_DDR0_MASK_VAL_FIELD_WIDTH,
    RNR_QUAD_GENERAL_CONFIG_DDR0_MASK_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_GENERAL_CONFIG_DDR1_MASK_VAL
 ******************************************************************************/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_DDR1_MASK_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "VAL",
    "Value for base/mask",
#endif
    RNR_QUAD_GENERAL_CONFIG_DDR1_MASK_VAL_FIELD_MASK,
    0,
    RNR_QUAD_GENERAL_CONFIG_DDR1_MASK_VAL_FIELD_WIDTH,
    RNR_QUAD_GENERAL_CONFIG_DDR1_MASK_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_COUNTER_LSB_SEL
 ******************************************************************************/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_COUNTER_LSB_SEL_FIELD =
{
    "COUNTER_LSB_SEL",
#if RU_INCLUDE_DESC
    "COUNTER_LSB_SEL",
    "Select which 12-bits from 32-bit counter value to be recorded by tracer",
#endif
    RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_COUNTER_LSB_SEL_FIELD_MASK,
    0,
    RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_COUNTER_LSB_SEL_FIELD_WIDTH,
    RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_COUNTER_LSB_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_RESERVED0
 ******************************************************************************/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_RESERVED0_FIELD_MASK,
    0,
    RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_RESERVED0_FIELD_WIDTH,
    RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_0
 ******************************************************************************/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_0_FIELD =
{
    "ENABLE_TRACE_CORE_0",
#if RU_INCLUDE_DESC
    "ENABLE_TRACE_CORE_0",
    "Enable tracing for core 0",
#endif
    RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_0_FIELD_MASK,
    0,
    RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_0_FIELD_WIDTH,
    RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_1
 ******************************************************************************/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_1_FIELD =
{
    "ENABLE_TRACE_CORE_1",
#if RU_INCLUDE_DESC
    "ENABLE_TRACE_CORE_1",
    "Enable tracing for core 1",
#endif
    RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_1_FIELD_MASK,
    0,
    RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_1_FIELD_WIDTH,
    RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_2
 ******************************************************************************/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_2_FIELD =
{
    "ENABLE_TRACE_CORE_2",
#if RU_INCLUDE_DESC
    "ENABLE_TRACE_CORE_2",
    "Enable tracing for core 2",
#endif
    RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_2_FIELD_MASK,
    0,
    RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_2_FIELD_WIDTH,
    RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_3
 ******************************************************************************/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_3_FIELD =
{
    "ENABLE_TRACE_CORE_3",
#if RU_INCLUDE_DESC
    "ENABLE_TRACE_CORE_3",
    "Enable tracing for core 3",
#endif
    RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_3_FIELD_MASK,
    0,
    RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_3_FIELD_WIDTH,
    RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_4
 ******************************************************************************/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_4_FIELD =
{
    "ENABLE_TRACE_CORE_4",
#if RU_INCLUDE_DESC
    "ENABLE_TRACE_CORE_4",
    "Enable tracing for core 4",
#endif
    RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_4_FIELD_MASK,
    0,
    RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_4_FIELD_WIDTH,
    RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_4_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_5
 ******************************************************************************/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_5_FIELD =
{
    "ENABLE_TRACE_CORE_5",
#if RU_INCLUDE_DESC
    "ENABLE_TRACE_CORE_5",
    "Enable tracing for core 5",
#endif
    RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_5_FIELD_MASK,
    0,
    RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_5_FIELD_WIDTH,
    RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_5_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_RESERVED1
 ******************************************************************************/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_RESERVED1_FIELD_MASK,
    0,
    RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_RESERVED1_FIELD_WIDTH,
    RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_GENERAL_CONFIG_BKPT_0_CFG_ADDR
 ******************************************************************************/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_BKPT_0_CFG_ADDR_FIELD =
{
    "ADDR",
#if RU_INCLUDE_DESC
    "ADDR",
    "Breakpoint address",
#endif
    RNR_QUAD_GENERAL_CONFIG_BKPT_0_CFG_ADDR_FIELD_MASK,
    0,
    RNR_QUAD_GENERAL_CONFIG_BKPT_0_CFG_ADDR_FIELD_WIDTH,
    RNR_QUAD_GENERAL_CONFIG_BKPT_0_CFG_ADDR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_GENERAL_CONFIG_BKPT_0_CFG_RESERVED0
 ******************************************************************************/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_BKPT_0_CFG_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_QUAD_GENERAL_CONFIG_BKPT_0_CFG_RESERVED0_FIELD_MASK,
    0,
    RNR_QUAD_GENERAL_CONFIG_BKPT_0_CFG_RESERVED0_FIELD_WIDTH,
    RNR_QUAD_GENERAL_CONFIG_BKPT_0_CFG_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_GENERAL_CONFIG_BKPT_0_CFG_THREAD
 ******************************************************************************/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_BKPT_0_CFG_THREAD_FIELD =
{
    "THREAD",
#if RU_INCLUDE_DESC
    "THREAD",
    "Breakpoint address",
#endif
    RNR_QUAD_GENERAL_CONFIG_BKPT_0_CFG_THREAD_FIELD_MASK,
    0,
    RNR_QUAD_GENERAL_CONFIG_BKPT_0_CFG_THREAD_FIELD_WIDTH,
    RNR_QUAD_GENERAL_CONFIG_BKPT_0_CFG_THREAD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_GENERAL_CONFIG_BKPT_0_CFG_RESERVED1
 ******************************************************************************/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_BKPT_0_CFG_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_QUAD_GENERAL_CONFIG_BKPT_0_CFG_RESERVED1_FIELD_MASK,
    0,
    RNR_QUAD_GENERAL_CONFIG_BKPT_0_CFG_RESERVED1_FIELD_WIDTH,
    RNR_QUAD_GENERAL_CONFIG_BKPT_0_CFG_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_GENERAL_CONFIG_BKPT_1_CFG_ADDR
 ******************************************************************************/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_BKPT_1_CFG_ADDR_FIELD =
{
    "ADDR",
#if RU_INCLUDE_DESC
    "ADDR",
    "Breakpoint address",
#endif
    RNR_QUAD_GENERAL_CONFIG_BKPT_1_CFG_ADDR_FIELD_MASK,
    0,
    RNR_QUAD_GENERAL_CONFIG_BKPT_1_CFG_ADDR_FIELD_WIDTH,
    RNR_QUAD_GENERAL_CONFIG_BKPT_1_CFG_ADDR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_GENERAL_CONFIG_BKPT_1_CFG_RESERVED0
 ******************************************************************************/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_BKPT_1_CFG_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_QUAD_GENERAL_CONFIG_BKPT_1_CFG_RESERVED0_FIELD_MASK,
    0,
    RNR_QUAD_GENERAL_CONFIG_BKPT_1_CFG_RESERVED0_FIELD_WIDTH,
    RNR_QUAD_GENERAL_CONFIG_BKPT_1_CFG_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_GENERAL_CONFIG_BKPT_1_CFG_THREAD
 ******************************************************************************/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_BKPT_1_CFG_THREAD_FIELD =
{
    "THREAD",
#if RU_INCLUDE_DESC
    "THREAD",
    "Breakpoint address",
#endif
    RNR_QUAD_GENERAL_CONFIG_BKPT_1_CFG_THREAD_FIELD_MASK,
    0,
    RNR_QUAD_GENERAL_CONFIG_BKPT_1_CFG_THREAD_FIELD_WIDTH,
    RNR_QUAD_GENERAL_CONFIG_BKPT_1_CFG_THREAD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_GENERAL_CONFIG_BKPT_1_CFG_RESERVED1
 ******************************************************************************/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_BKPT_1_CFG_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_QUAD_GENERAL_CONFIG_BKPT_1_CFG_RESERVED1_FIELD_MASK,
    0,
    RNR_QUAD_GENERAL_CONFIG_BKPT_1_CFG_RESERVED1_FIELD_WIDTH,
    RNR_QUAD_GENERAL_CONFIG_BKPT_1_CFG_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_GENERAL_CONFIG_BKPT_2_CFG_ADDR
 ******************************************************************************/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_BKPT_2_CFG_ADDR_FIELD =
{
    "ADDR",
#if RU_INCLUDE_DESC
    "ADDR",
    "Breakpoint address",
#endif
    RNR_QUAD_GENERAL_CONFIG_BKPT_2_CFG_ADDR_FIELD_MASK,
    0,
    RNR_QUAD_GENERAL_CONFIG_BKPT_2_CFG_ADDR_FIELD_WIDTH,
    RNR_QUAD_GENERAL_CONFIG_BKPT_2_CFG_ADDR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_GENERAL_CONFIG_BKPT_2_CFG_RESERVED0
 ******************************************************************************/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_BKPT_2_CFG_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_QUAD_GENERAL_CONFIG_BKPT_2_CFG_RESERVED0_FIELD_MASK,
    0,
    RNR_QUAD_GENERAL_CONFIG_BKPT_2_CFG_RESERVED0_FIELD_WIDTH,
    RNR_QUAD_GENERAL_CONFIG_BKPT_2_CFG_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_GENERAL_CONFIG_BKPT_2_CFG_THREAD
 ******************************************************************************/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_BKPT_2_CFG_THREAD_FIELD =
{
    "THREAD",
#if RU_INCLUDE_DESC
    "THREAD",
    "Breakpoint address",
#endif
    RNR_QUAD_GENERAL_CONFIG_BKPT_2_CFG_THREAD_FIELD_MASK,
    0,
    RNR_QUAD_GENERAL_CONFIG_BKPT_2_CFG_THREAD_FIELD_WIDTH,
    RNR_QUAD_GENERAL_CONFIG_BKPT_2_CFG_THREAD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_GENERAL_CONFIG_BKPT_2_CFG_RESERVED1
 ******************************************************************************/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_BKPT_2_CFG_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_QUAD_GENERAL_CONFIG_BKPT_2_CFG_RESERVED1_FIELD_MASK,
    0,
    RNR_QUAD_GENERAL_CONFIG_BKPT_2_CFG_RESERVED1_FIELD_WIDTH,
    RNR_QUAD_GENERAL_CONFIG_BKPT_2_CFG_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_GENERAL_CONFIG_BKPT_3_CFG_ADDR
 ******************************************************************************/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_BKPT_3_CFG_ADDR_FIELD =
{
    "ADDR",
#if RU_INCLUDE_DESC
    "ADDR",
    "Breakpoint address",
#endif
    RNR_QUAD_GENERAL_CONFIG_BKPT_3_CFG_ADDR_FIELD_MASK,
    0,
    RNR_QUAD_GENERAL_CONFIG_BKPT_3_CFG_ADDR_FIELD_WIDTH,
    RNR_QUAD_GENERAL_CONFIG_BKPT_3_CFG_ADDR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_GENERAL_CONFIG_BKPT_3_CFG_RESERVED0
 ******************************************************************************/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_BKPT_3_CFG_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_QUAD_GENERAL_CONFIG_BKPT_3_CFG_RESERVED0_FIELD_MASK,
    0,
    RNR_QUAD_GENERAL_CONFIG_BKPT_3_CFG_RESERVED0_FIELD_WIDTH,
    RNR_QUAD_GENERAL_CONFIG_BKPT_3_CFG_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_GENERAL_CONFIG_BKPT_3_CFG_THREAD
 ******************************************************************************/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_BKPT_3_CFG_THREAD_FIELD =
{
    "THREAD",
#if RU_INCLUDE_DESC
    "THREAD",
    "Breakpoint address",
#endif
    RNR_QUAD_GENERAL_CONFIG_BKPT_3_CFG_THREAD_FIELD_MASK,
    0,
    RNR_QUAD_GENERAL_CONFIG_BKPT_3_CFG_THREAD_FIELD_WIDTH,
    RNR_QUAD_GENERAL_CONFIG_BKPT_3_CFG_THREAD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_GENERAL_CONFIG_BKPT_3_CFG_RESERVED1
 ******************************************************************************/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_BKPT_3_CFG_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_QUAD_GENERAL_CONFIG_BKPT_3_CFG_RESERVED1_FIELD_MASK,
    0,
    RNR_QUAD_GENERAL_CONFIG_BKPT_3_CFG_RESERVED1_FIELD_WIDTH,
    RNR_QUAD_GENERAL_CONFIG_BKPT_3_CFG_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_GENERAL_CONFIG_BKPT_4_CFG_ADDR
 ******************************************************************************/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_BKPT_4_CFG_ADDR_FIELD =
{
    "ADDR",
#if RU_INCLUDE_DESC
    "ADDR",
    "Breakpoint address",
#endif
    RNR_QUAD_GENERAL_CONFIG_BKPT_4_CFG_ADDR_FIELD_MASK,
    0,
    RNR_QUAD_GENERAL_CONFIG_BKPT_4_CFG_ADDR_FIELD_WIDTH,
    RNR_QUAD_GENERAL_CONFIG_BKPT_4_CFG_ADDR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_GENERAL_CONFIG_BKPT_4_CFG_RESERVED0
 ******************************************************************************/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_BKPT_4_CFG_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_QUAD_GENERAL_CONFIG_BKPT_4_CFG_RESERVED0_FIELD_MASK,
    0,
    RNR_QUAD_GENERAL_CONFIG_BKPT_4_CFG_RESERVED0_FIELD_WIDTH,
    RNR_QUAD_GENERAL_CONFIG_BKPT_4_CFG_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_GENERAL_CONFIG_BKPT_4_CFG_THREAD
 ******************************************************************************/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_BKPT_4_CFG_THREAD_FIELD =
{
    "THREAD",
#if RU_INCLUDE_DESC
    "THREAD",
    "Breakpoint address",
#endif
    RNR_QUAD_GENERAL_CONFIG_BKPT_4_CFG_THREAD_FIELD_MASK,
    0,
    RNR_QUAD_GENERAL_CONFIG_BKPT_4_CFG_THREAD_FIELD_WIDTH,
    RNR_QUAD_GENERAL_CONFIG_BKPT_4_CFG_THREAD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_GENERAL_CONFIG_BKPT_4_CFG_RESERVED1
 ******************************************************************************/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_BKPT_4_CFG_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_QUAD_GENERAL_CONFIG_BKPT_4_CFG_RESERVED1_FIELD_MASK,
    0,
    RNR_QUAD_GENERAL_CONFIG_BKPT_4_CFG_RESERVED1_FIELD_WIDTH,
    RNR_QUAD_GENERAL_CONFIG_BKPT_4_CFG_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_GENERAL_CONFIG_BKPT_5_CFG_ADDR
 ******************************************************************************/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_BKPT_5_CFG_ADDR_FIELD =
{
    "ADDR",
#if RU_INCLUDE_DESC
    "ADDR",
    "Breakpoint address",
#endif
    RNR_QUAD_GENERAL_CONFIG_BKPT_5_CFG_ADDR_FIELD_MASK,
    0,
    RNR_QUAD_GENERAL_CONFIG_BKPT_5_CFG_ADDR_FIELD_WIDTH,
    RNR_QUAD_GENERAL_CONFIG_BKPT_5_CFG_ADDR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_GENERAL_CONFIG_BKPT_5_CFG_RESERVED0
 ******************************************************************************/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_BKPT_5_CFG_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_QUAD_GENERAL_CONFIG_BKPT_5_CFG_RESERVED0_FIELD_MASK,
    0,
    RNR_QUAD_GENERAL_CONFIG_BKPT_5_CFG_RESERVED0_FIELD_WIDTH,
    RNR_QUAD_GENERAL_CONFIG_BKPT_5_CFG_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_GENERAL_CONFIG_BKPT_5_CFG_THREAD
 ******************************************************************************/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_BKPT_5_CFG_THREAD_FIELD =
{
    "THREAD",
#if RU_INCLUDE_DESC
    "THREAD",
    "Breakpoint address",
#endif
    RNR_QUAD_GENERAL_CONFIG_BKPT_5_CFG_THREAD_FIELD_MASK,
    0,
    RNR_QUAD_GENERAL_CONFIG_BKPT_5_CFG_THREAD_FIELD_WIDTH,
    RNR_QUAD_GENERAL_CONFIG_BKPT_5_CFG_THREAD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_GENERAL_CONFIG_BKPT_5_CFG_RESERVED1
 ******************************************************************************/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_BKPT_5_CFG_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_QUAD_GENERAL_CONFIG_BKPT_5_CFG_RESERVED1_FIELD_MASK,
    0,
    RNR_QUAD_GENERAL_CONFIG_BKPT_5_CFG_RESERVED1_FIELD_WIDTH,
    RNR_QUAD_GENERAL_CONFIG_BKPT_5_CFG_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_GENERAL_CONFIG_BKPT_6_CFG_ADDR
 ******************************************************************************/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_BKPT_6_CFG_ADDR_FIELD =
{
    "ADDR",
#if RU_INCLUDE_DESC
    "ADDR",
    "Breakpoint address",
#endif
    RNR_QUAD_GENERAL_CONFIG_BKPT_6_CFG_ADDR_FIELD_MASK,
    0,
    RNR_QUAD_GENERAL_CONFIG_BKPT_6_CFG_ADDR_FIELD_WIDTH,
    RNR_QUAD_GENERAL_CONFIG_BKPT_6_CFG_ADDR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_GENERAL_CONFIG_BKPT_6_CFG_RESERVED0
 ******************************************************************************/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_BKPT_6_CFG_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_QUAD_GENERAL_CONFIG_BKPT_6_CFG_RESERVED0_FIELD_MASK,
    0,
    RNR_QUAD_GENERAL_CONFIG_BKPT_6_CFG_RESERVED0_FIELD_WIDTH,
    RNR_QUAD_GENERAL_CONFIG_BKPT_6_CFG_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_GENERAL_CONFIG_BKPT_6_CFG_THREAD
 ******************************************************************************/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_BKPT_6_CFG_THREAD_FIELD =
{
    "THREAD",
#if RU_INCLUDE_DESC
    "THREAD",
    "Breakpoint address",
#endif
    RNR_QUAD_GENERAL_CONFIG_BKPT_6_CFG_THREAD_FIELD_MASK,
    0,
    RNR_QUAD_GENERAL_CONFIG_BKPT_6_CFG_THREAD_FIELD_WIDTH,
    RNR_QUAD_GENERAL_CONFIG_BKPT_6_CFG_THREAD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_GENERAL_CONFIG_BKPT_6_CFG_RESERVED1
 ******************************************************************************/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_BKPT_6_CFG_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_QUAD_GENERAL_CONFIG_BKPT_6_CFG_RESERVED1_FIELD_MASK,
    0,
    RNR_QUAD_GENERAL_CONFIG_BKPT_6_CFG_RESERVED1_FIELD_WIDTH,
    RNR_QUAD_GENERAL_CONFIG_BKPT_6_CFG_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_GENERAL_CONFIG_BKPT_7_CFG_ADDR
 ******************************************************************************/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_BKPT_7_CFG_ADDR_FIELD =
{
    "ADDR",
#if RU_INCLUDE_DESC
    "ADDR",
    "Breakpoint address",
#endif
    RNR_QUAD_GENERAL_CONFIG_BKPT_7_CFG_ADDR_FIELD_MASK,
    0,
    RNR_QUAD_GENERAL_CONFIG_BKPT_7_CFG_ADDR_FIELD_WIDTH,
    RNR_QUAD_GENERAL_CONFIG_BKPT_7_CFG_ADDR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_GENERAL_CONFIG_BKPT_7_CFG_RESERVED0
 ******************************************************************************/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_BKPT_7_CFG_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_QUAD_GENERAL_CONFIG_BKPT_7_CFG_RESERVED0_FIELD_MASK,
    0,
    RNR_QUAD_GENERAL_CONFIG_BKPT_7_CFG_RESERVED0_FIELD_WIDTH,
    RNR_QUAD_GENERAL_CONFIG_BKPT_7_CFG_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_GENERAL_CONFIG_BKPT_7_CFG_THREAD
 ******************************************************************************/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_BKPT_7_CFG_THREAD_FIELD =
{
    "THREAD",
#if RU_INCLUDE_DESC
    "THREAD",
    "Breakpoint address",
#endif
    RNR_QUAD_GENERAL_CONFIG_BKPT_7_CFG_THREAD_FIELD_MASK,
    0,
    RNR_QUAD_GENERAL_CONFIG_BKPT_7_CFG_THREAD_FIELD_WIDTH,
    RNR_QUAD_GENERAL_CONFIG_BKPT_7_CFG_THREAD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_GENERAL_CONFIG_BKPT_7_CFG_RESERVED1
 ******************************************************************************/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_BKPT_7_CFG_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_QUAD_GENERAL_CONFIG_BKPT_7_CFG_RESERVED1_FIELD_MASK,
    0,
    RNR_QUAD_GENERAL_CONFIG_BKPT_7_CFG_RESERVED1_FIELD_WIDTH,
    RNR_QUAD_GENERAL_CONFIG_BKPT_7_CFG_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_GENERAL_CONFIG_BKPT_GEN_CFG_HANDLER_ADDR
 ******************************************************************************/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_BKPT_GEN_CFG_HANDLER_ADDR_FIELD =
{
    "HANDLER_ADDR",
#if RU_INCLUDE_DESC
    "HANDLER_ADDR",
    "Breakpoint handler routine address",
#endif
    RNR_QUAD_GENERAL_CONFIG_BKPT_GEN_CFG_HANDLER_ADDR_FIELD_MASK,
    0,
    RNR_QUAD_GENERAL_CONFIG_BKPT_GEN_CFG_HANDLER_ADDR_FIELD_WIDTH,
    RNR_QUAD_GENERAL_CONFIG_BKPT_GEN_CFG_HANDLER_ADDR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_GENERAL_CONFIG_BKPT_GEN_CFG_RESERVED0
 ******************************************************************************/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_BKPT_GEN_CFG_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_QUAD_GENERAL_CONFIG_BKPT_GEN_CFG_RESERVED0_FIELD_MASK,
    0,
    RNR_QUAD_GENERAL_CONFIG_BKPT_GEN_CFG_RESERVED0_FIELD_WIDTH,
    RNR_QUAD_GENERAL_CONFIG_BKPT_GEN_CFG_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_GENERAL_CONFIG_BKPT_GEN_CFG_UPDATE_PC_VALUE
 ******************************************************************************/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_BKPT_GEN_CFG_UPDATE_PC_VALUE_FIELD =
{
    "UPDATE_PC_VALUE",
#if RU_INCLUDE_DESC
    "UPDATE_PC_VALUE",
    "New PC to be updated by breakpoint handler routine",
#endif
    RNR_QUAD_GENERAL_CONFIG_BKPT_GEN_CFG_UPDATE_PC_VALUE_FIELD_MASK,
    0,
    RNR_QUAD_GENERAL_CONFIG_BKPT_GEN_CFG_UPDATE_PC_VALUE_FIELD_WIDTH,
    RNR_QUAD_GENERAL_CONFIG_BKPT_GEN_CFG_UPDATE_PC_VALUE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_GENERAL_CONFIG_BKPT_GEN_CFG_RESERVED1
 ******************************************************************************/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_BKPT_GEN_CFG_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_QUAD_GENERAL_CONFIG_BKPT_GEN_CFG_RESERVED1_FIELD_MASK,
    0,
    RNR_QUAD_GENERAL_CONFIG_BKPT_GEN_CFG_RESERVED1_FIELD_WIDTH,
    RNR_QUAD_GENERAL_CONFIG_BKPT_GEN_CFG_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_TIME_COUNTER
 ******************************************************************************/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_TIME_COUNTER_FIELD =
{
    "TIME_COUNTER",
#if RU_INCLUDE_DESC
    "TIME_COUNTER",
    "Select how many clocks to wait in IDLE condition before enetrin powersave state",
#endif
    RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_TIME_COUNTER_FIELD_MASK,
    0,
    RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_TIME_COUNTER_FIELD_WIDTH,
    RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_TIME_COUNTER_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_0
 ******************************************************************************/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_0_FIELD =
{
    "ENABLE_POWERSAVE_CORE_0",
#if RU_INCLUDE_DESC
    "ENABLE_POWERSAVE_CORE_0",
    "Enable powersavingfor core 0",
#endif
    RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_0_FIELD_MASK,
    0,
    RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_0_FIELD_WIDTH,
    RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_1
 ******************************************************************************/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_1_FIELD =
{
    "ENABLE_POWERSAVE_CORE_1",
#if RU_INCLUDE_DESC
    "ENABLE_POWERSAVE_CORE_1",
    "Enable powersave for core 1",
#endif
    RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_1_FIELD_MASK,
    0,
    RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_1_FIELD_WIDTH,
    RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_2
 ******************************************************************************/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_2_FIELD =
{
    "ENABLE_POWERSAVE_CORE_2",
#if RU_INCLUDE_DESC
    "ENABLE_POWERSAVE_CORE_2",
    "Enable powersave for core 2",
#endif
    RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_2_FIELD_MASK,
    0,
    RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_2_FIELD_WIDTH,
    RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_3
 ******************************************************************************/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_3_FIELD =
{
    "ENABLE_POWERSAVE_CORE_3",
#if RU_INCLUDE_DESC
    "ENABLE_POWERSAVE_CORE_3",
    "Enable powersave for core 3",
#endif
    RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_3_FIELD_MASK,
    0,
    RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_3_FIELD_WIDTH,
    RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_4
 ******************************************************************************/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_4_FIELD =
{
    "ENABLE_POWERSAVE_CORE_4",
#if RU_INCLUDE_DESC
    "ENABLE_POWERSAVE_CORE_4",
    "Enable powersave for core 4",
#endif
    RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_4_FIELD_MASK,
    0,
    RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_4_FIELD_WIDTH,
    RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_4_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_5
 ******************************************************************************/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_5_FIELD =
{
    "ENABLE_POWERSAVE_CORE_5",
#if RU_INCLUDE_DESC
    "ENABLE_POWERSAVE_CORE_5",
    "Enable powersave for core 5",
#endif
    RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_5_FIELD_MASK,
    0,
    RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_5_FIELD_WIDTH,
    RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_ENABLE_POWERSAVE_CORE_5_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_RESERVED0
 ******************************************************************************/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_RESERVED0_FIELD_MASK,
    0,
    RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_RESERVED0_FIELD_WIDTH,
    RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_0_STATUS
 ******************************************************************************/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_0_STATUS_FIELD =
{
    "CORE_0_STATUS",
#if RU_INCLUDE_DESC
    "STATUS_CORE_0",
    "Core 0 status",
#endif
    RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_0_STATUS_FIELD_MASK,
    0,
    RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_0_STATUS_FIELD_WIDTH,
    RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_0_STATUS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_1_STATUS
 ******************************************************************************/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_1_STATUS_FIELD =
{
    "CORE_1_STATUS",
#if RU_INCLUDE_DESC
    "STATUS_CORE_1",
    "Core 0 status",
#endif
    RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_1_STATUS_FIELD_MASK,
    0,
    RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_1_STATUS_FIELD_WIDTH,
    RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_1_STATUS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_2_STATUS
 ******************************************************************************/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_2_STATUS_FIELD =
{
    "CORE_2_STATUS",
#if RU_INCLUDE_DESC
    "STATUS_CORE_2",
    "Core 2 status",
#endif
    RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_2_STATUS_FIELD_MASK,
    0,
    RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_2_STATUS_FIELD_WIDTH,
    RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_2_STATUS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_3_STATUS
 ******************************************************************************/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_3_STATUS_FIELD =
{
    "CORE_3_STATUS",
#if RU_INCLUDE_DESC
    "STATUS_CORE_3",
    "Core 3 status",
#endif
    RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_3_STATUS_FIELD_MASK,
    0,
    RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_3_STATUS_FIELD_WIDTH,
    RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_3_STATUS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_4_STATUS
 ******************************************************************************/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_4_STATUS_FIELD =
{
    "CORE_4_STATUS",
#if RU_INCLUDE_DESC
    "STATUS_CORE_4",
    "Core 4 status",
#endif
    RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_4_STATUS_FIELD_MASK,
    0,
    RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_4_STATUS_FIELD_WIDTH,
    RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_4_STATUS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_5_STATUS
 ******************************************************************************/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_5_STATUS_FIELD =
{
    "CORE_5_STATUS",
#if RU_INCLUDE_DESC
    "STATUS_CORE_5",
    "Core 5 status",
#endif
    RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_5_STATUS_FIELD_MASK,
    0,
    RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_5_STATUS_FIELD_WIDTH,
    RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_5_STATUS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_RESERVED0_FIELD_MASK,
    0,
    RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_RESERVED0_FIELD_WIDTH,
    RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_DEBUG_FIFO_CONFIG_PSRAM_HDR_SW_RST
 ******************************************************************************/
const ru_field_rec RNR_QUAD_DEBUG_FIFO_CONFIG_PSRAM_HDR_SW_RST_FIELD =
{
    "PSRAM_HDR_SW_RST",
#if RU_INCLUDE_DESC
    "PSRAM_HDR_SW_RST",
    "Apply software reset to PSRAM header FIFO in EC arbiter",
#endif
    RNR_QUAD_DEBUG_FIFO_CONFIG_PSRAM_HDR_SW_RST_FIELD_MASK,
    0,
    RNR_QUAD_DEBUG_FIFO_CONFIG_PSRAM_HDR_SW_RST_FIELD_WIDTH,
    RNR_QUAD_DEBUG_FIFO_CONFIG_PSRAM_HDR_SW_RST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_DEBUG_FIFO_CONFIG_PSRAM_DATA_SW_RST
 ******************************************************************************/
const ru_field_rec RNR_QUAD_DEBUG_FIFO_CONFIG_PSRAM_DATA_SW_RST_FIELD =
{
    "PSRAM_DATA_SW_RST",
#if RU_INCLUDE_DESC
    "PSRAM_DATA_SW_RST",
    "Apply software reset to PSRAM data FIFO in EC arbiter",
#endif
    RNR_QUAD_DEBUG_FIFO_CONFIG_PSRAM_DATA_SW_RST_FIELD_MASK,
    0,
    RNR_QUAD_DEBUG_FIFO_CONFIG_PSRAM_DATA_SW_RST_FIELD_WIDTH,
    RNR_QUAD_DEBUG_FIFO_CONFIG_PSRAM_DATA_SW_RST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_DEBUG_FIFO_CONFIG_DDR_HDR_SW_RST
 ******************************************************************************/
const ru_field_rec RNR_QUAD_DEBUG_FIFO_CONFIG_DDR_HDR_SW_RST_FIELD =
{
    "DDR_HDR_SW_RST",
#if RU_INCLUDE_DESC
    "DDR_HDR_SW_RST",
    "Apply software reset to DDR header FIFO in EC arbiter",
#endif
    RNR_QUAD_DEBUG_FIFO_CONFIG_DDR_HDR_SW_RST_FIELD_MASK,
    0,
    RNR_QUAD_DEBUG_FIFO_CONFIG_DDR_HDR_SW_RST_FIELD_WIDTH,
    RNR_QUAD_DEBUG_FIFO_CONFIG_DDR_HDR_SW_RST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_DEBUG_FIFO_CONFIG_RESERVED0
 ******************************************************************************/
const ru_field_rec RNR_QUAD_DEBUG_FIFO_CONFIG_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_QUAD_DEBUG_FIFO_CONFIG_RESERVED0_FIELD_MASK,
    0,
    RNR_QUAD_DEBUG_FIFO_CONFIG_RESERVED0_FIELD_WIDTH,
    RNR_QUAD_DEBUG_FIFO_CONFIG_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_DEBUG_FIFO_CONFIG_PSRAM_HDR_SW_RD_ADDR
 ******************************************************************************/
const ru_field_rec RNR_QUAD_DEBUG_FIFO_CONFIG_PSRAM_HDR_SW_RD_ADDR_FIELD =
{
    "PSRAM_HDR_SW_RD_ADDR",
#if RU_INCLUDE_DESC
    "PSRAM_HDR_SW_RD_ADDR",
    "Software read address for PSRAM header FIFO",
#endif
    RNR_QUAD_DEBUG_FIFO_CONFIG_PSRAM_HDR_SW_RD_ADDR_FIELD_MASK,
    0,
    RNR_QUAD_DEBUG_FIFO_CONFIG_PSRAM_HDR_SW_RD_ADDR_FIELD_WIDTH,
    RNR_QUAD_DEBUG_FIFO_CONFIG_PSRAM_HDR_SW_RD_ADDR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_DEBUG_FIFO_CONFIG_PSRAM_DATA_SW_RD_ADDR
 ******************************************************************************/
const ru_field_rec RNR_QUAD_DEBUG_FIFO_CONFIG_PSRAM_DATA_SW_RD_ADDR_FIELD =
{
    "PSRAM_DATA_SW_RD_ADDR",
#if RU_INCLUDE_DESC
    "PSRAM_DATA_SW_RD_ADDR",
    "Software read address for PSRAM data FIFO",
#endif
    RNR_QUAD_DEBUG_FIFO_CONFIG_PSRAM_DATA_SW_RD_ADDR_FIELD_MASK,
    0,
    RNR_QUAD_DEBUG_FIFO_CONFIG_PSRAM_DATA_SW_RD_ADDR_FIELD_WIDTH,
    RNR_QUAD_DEBUG_FIFO_CONFIG_PSRAM_DATA_SW_RD_ADDR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_DEBUG_FIFO_CONFIG_DDR_HDR_SW_RD_ADDR
 ******************************************************************************/
const ru_field_rec RNR_QUAD_DEBUG_FIFO_CONFIG_DDR_HDR_SW_RD_ADDR_FIELD =
{
    "DDR_HDR_SW_RD_ADDR",
#if RU_INCLUDE_DESC
    "DDR_HDR_SW_RD_ADDR",
    "Software read address for DDR header FIFO",
#endif
    RNR_QUAD_DEBUG_FIFO_CONFIG_DDR_HDR_SW_RD_ADDR_FIELD_MASK,
    0,
    RNR_QUAD_DEBUG_FIFO_CONFIG_DDR_HDR_SW_RD_ADDR_FIELD_WIDTH,
    RNR_QUAD_DEBUG_FIFO_CONFIG_DDR_HDR_SW_RD_ADDR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_DEBUG_FIFO_CONFIG_RESERVED1
 ******************************************************************************/
const ru_field_rec RNR_QUAD_DEBUG_FIFO_CONFIG_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_QUAD_DEBUG_FIFO_CONFIG_RESERVED1_FIELD_MASK,
    0,
    RNR_QUAD_DEBUG_FIFO_CONFIG_RESERVED1_FIELD_WIDTH,
    RNR_QUAD_DEBUG_FIFO_CONFIG_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_FULL
 ******************************************************************************/
const ru_field_rec RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_FULL_FIELD =
{
    "FULL",
#if RU_INCLUDE_DESC
    "FULL",
    "FIFO full indication",
#endif
    RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_FULL_FIELD_MASK,
    0,
    RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_FULL_FIELD_WIDTH,
    RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_FULL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_EMPTY
 ******************************************************************************/
const ru_field_rec RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_EMPTY_FIELD =
{
    "EMPTY",
#if RU_INCLUDE_DESC
    "EMPTY",
    "FIFO empty indication",
#endif
    RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_EMPTY_FIELD_MASK,
    0,
    RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_EMPTY_FIELD_WIDTH,
    RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_EMPTY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_RESERVED0_FIELD_MASK,
    0,
    RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_RESERVED0_FIELD_WIDTH,
    RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_PUSH_WR_CNTR
 ******************************************************************************/
const ru_field_rec RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_PUSH_WR_CNTR_FIELD =
{
    "PUSH_WR_CNTR",
#if RU_INCLUDE_DESC
    "PUSH_WR_CNTR",
    "Push write counter value",
#endif
    RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_PUSH_WR_CNTR_FIELD_MASK,
    0,
    RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_PUSH_WR_CNTR_FIELD_WIDTH,
    RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_PUSH_WR_CNTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_RESERVED1
 ******************************************************************************/
const ru_field_rec RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_RESERVED1_FIELD_MASK,
    0,
    RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_RESERVED1_FIELD_WIDTH,
    RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_POP_RD_CNTR
 ******************************************************************************/
const ru_field_rec RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_POP_RD_CNTR_FIELD =
{
    "POP_RD_CNTR",
#if RU_INCLUDE_DESC
    "POP_RD_CNTR",
    "Pop read counter value",
#endif
    RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_POP_RD_CNTR_FIELD_MASK,
    0,
    RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_POP_RD_CNTR_FIELD_WIDTH,
    RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_POP_RD_CNTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_RESERVED2
 ******************************************************************************/
const ru_field_rec RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_RESERVED2_FIELD =
{
    "RESERVED2",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_RESERVED2_FIELD_MASK,
    0,
    RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_RESERVED2_FIELD_WIDTH,
    RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_RESERVED2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_USED_WORDS
 ******************************************************************************/
const ru_field_rec RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_USED_WORDS_FIELD =
{
    "USED_WORDS",
#if RU_INCLUDE_DESC
    "USED_WORDS",
    "Used words value",
#endif
    RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_USED_WORDS_FIELD_MASK,
    0,
    RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_USED_WORDS_FIELD_WIDTH,
    RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_USED_WORDS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_RESERVED3
 ******************************************************************************/
const ru_field_rec RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_RESERVED3_FIELD =
{
    "RESERVED3",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_RESERVED3_FIELD_MASK,
    0,
    RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_RESERVED3_FIELD_WIDTH,
    RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_RESERVED3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_FULL
 ******************************************************************************/
const ru_field_rec RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_FULL_FIELD =
{
    "FULL",
#if RU_INCLUDE_DESC
    "FULL",
    "FIFO full indication",
#endif
    RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_FULL_FIELD_MASK,
    0,
    RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_FULL_FIELD_WIDTH,
    RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_FULL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_EMPTY
 ******************************************************************************/
const ru_field_rec RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_EMPTY_FIELD =
{
    "EMPTY",
#if RU_INCLUDE_DESC
    "EMPTY",
    "FIFO empty indication",
#endif
    RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_EMPTY_FIELD_MASK,
    0,
    RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_EMPTY_FIELD_WIDTH,
    RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_EMPTY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_ALMOST_FULL
 ******************************************************************************/
const ru_field_rec RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_ALMOST_FULL_FIELD =
{
    "ALMOST_FULL",
#if RU_INCLUDE_DESC
    "ALMOST_FULL",
    "Almost FIFO full indication",
#endif
    RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_ALMOST_FULL_FIELD_MASK,
    0,
    RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_ALMOST_FULL_FIELD_WIDTH,
    RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_ALMOST_FULL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_RESERVED0_FIELD_MASK,
    0,
    RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_RESERVED0_FIELD_WIDTH,
    RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_PUSH_WR_CNTR
 ******************************************************************************/
const ru_field_rec RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_PUSH_WR_CNTR_FIELD =
{
    "PUSH_WR_CNTR",
#if RU_INCLUDE_DESC
    "PUSH_WR_CNTR",
    "Push write counter value",
#endif
    RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_PUSH_WR_CNTR_FIELD_MASK,
    0,
    RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_PUSH_WR_CNTR_FIELD_WIDTH,
    RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_PUSH_WR_CNTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_RESERVED1
 ******************************************************************************/
const ru_field_rec RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_RESERVED1_FIELD_MASK,
    0,
    RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_RESERVED1_FIELD_WIDTH,
    RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_POP_RD_CNTR
 ******************************************************************************/
const ru_field_rec RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_POP_RD_CNTR_FIELD =
{
    "POP_RD_CNTR",
#if RU_INCLUDE_DESC
    "POP_RD_CNTR",
    "Pop read counter value",
#endif
    RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_POP_RD_CNTR_FIELD_MASK,
    0,
    RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_POP_RD_CNTR_FIELD_WIDTH,
    RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_POP_RD_CNTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_RESERVED2
 ******************************************************************************/
const ru_field_rec RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_RESERVED2_FIELD =
{
    "RESERVED2",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_RESERVED2_FIELD_MASK,
    0,
    RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_RESERVED2_FIELD_WIDTH,
    RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_RESERVED2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_USED_WORDS
 ******************************************************************************/
const ru_field_rec RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_USED_WORDS_FIELD =
{
    "USED_WORDS",
#if RU_INCLUDE_DESC
    "USED_WORDS",
    "Used words value",
#endif
    RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_USED_WORDS_FIELD_MASK,
    0,
    RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_USED_WORDS_FIELD_WIDTH,
    RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_USED_WORDS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_RESERVED3
 ******************************************************************************/
const ru_field_rec RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_RESERVED3_FIELD =
{
    "RESERVED3",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_RESERVED3_FIELD_MASK,
    0,
    RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_RESERVED3_FIELD_WIDTH,
    RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_RESERVED3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_FULL
 ******************************************************************************/
const ru_field_rec RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_FULL_FIELD =
{
    "FULL",
#if RU_INCLUDE_DESC
    "FULL",
    "FIFO full indication",
#endif
    RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_FULL_FIELD_MASK,
    0,
    RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_FULL_FIELD_WIDTH,
    RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_FULL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_EMPTY
 ******************************************************************************/
const ru_field_rec RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_EMPTY_FIELD =
{
    "EMPTY",
#if RU_INCLUDE_DESC
    "EMPTY",
    "FIFO empty indication",
#endif
    RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_EMPTY_FIELD_MASK,
    0,
    RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_EMPTY_FIELD_WIDTH,
    RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_EMPTY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_RESERVED0_FIELD_MASK,
    0,
    RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_RESERVED0_FIELD_WIDTH,
    RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_PUSH_WR_CNTR
 ******************************************************************************/
const ru_field_rec RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_PUSH_WR_CNTR_FIELD =
{
    "PUSH_WR_CNTR",
#if RU_INCLUDE_DESC
    "PUSH_WR_CNTR",
    "Push write counter value",
#endif
    RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_PUSH_WR_CNTR_FIELD_MASK,
    0,
    RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_PUSH_WR_CNTR_FIELD_WIDTH,
    RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_PUSH_WR_CNTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_RESERVED1
 ******************************************************************************/
const ru_field_rec RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_RESERVED1_FIELD_MASK,
    0,
    RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_RESERVED1_FIELD_WIDTH,
    RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_POP_RD_CNTR
 ******************************************************************************/
const ru_field_rec RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_POP_RD_CNTR_FIELD =
{
    "POP_RD_CNTR",
#if RU_INCLUDE_DESC
    "POP_RD_CNTR",
    "Pop read counter value",
#endif
    RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_POP_RD_CNTR_FIELD_MASK,
    0,
    RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_POP_RD_CNTR_FIELD_WIDTH,
    RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_POP_RD_CNTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_RESERVED2
 ******************************************************************************/
const ru_field_rec RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_RESERVED2_FIELD =
{
    "RESERVED2",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_RESERVED2_FIELD_MASK,
    0,
    RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_RESERVED2_FIELD_WIDTH,
    RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_RESERVED2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_USED_WORDS
 ******************************************************************************/
const ru_field_rec RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_USED_WORDS_FIELD =
{
    "USED_WORDS",
#if RU_INCLUDE_DESC
    "USED_WORDS",
    "Used words value",
#endif
    RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_USED_WORDS_FIELD_MASK,
    0,
    RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_USED_WORDS_FIELD_WIDTH,
    RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_USED_WORDS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_RESERVED3
 ******************************************************************************/
const ru_field_rec RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_RESERVED3_FIELD =
{
    "RESERVED3",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_RESERVED3_FIELD_MASK,
    0,
    RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_RESERVED3_FIELD_WIDTH,
    RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_RESERVED3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS_FULL
 ******************************************************************************/
const ru_field_rec RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS_FULL_FIELD =
{
    "FULL",
#if RU_INCLUDE_DESC
    "FULL",
    "FIFO full indication",
#endif
    RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS_FULL_FIELD_MASK,
    0,
    RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS_FULL_FIELD_WIDTH,
    RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS_FULL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS_EMPTY
 ******************************************************************************/
const ru_field_rec RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS_EMPTY_FIELD =
{
    "EMPTY",
#if RU_INCLUDE_DESC
    "EMPTY",
    "FIFO empty indication",
#endif
    RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS_EMPTY_FIELD_MASK,
    0,
    RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS_EMPTY_FIELD_WIDTH,
    RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS_EMPTY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS_ALMOST_FULL
 ******************************************************************************/
const ru_field_rec RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS_ALMOST_FULL_FIELD =
{
    "ALMOST_FULL",
#if RU_INCLUDE_DESC
    "ALMOST_FULL",
    "Almost FIFO full indication",
#endif
    RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS_ALMOST_FULL_FIELD_MASK,
    0,
    RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS_ALMOST_FULL_FIELD_WIDTH,
    RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS_ALMOST_FULL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS_RESERVED0_FIELD_MASK,
    0,
    RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS_RESERVED0_FIELD_WIDTH,
    RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS_WR_CNTR
 ******************************************************************************/
const ru_field_rec RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS_WR_CNTR_FIELD =
{
    "WR_CNTR",
#if RU_INCLUDE_DESC
    "WR_CNTR",
    "rite counter value",
#endif
    RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS_WR_CNTR_FIELD_MASK,
    0,
    RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS_WR_CNTR_FIELD_WIDTH,
    RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS_WR_CNTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS_RESERVED1
 ******************************************************************************/
const ru_field_rec RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS_RESERVED1_FIELD_MASK,
    0,
    RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS_RESERVED1_FIELD_WIDTH,
    RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS_RD_CNTR
 ******************************************************************************/
const ru_field_rec RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS_RD_CNTR_FIELD =
{
    "RD_CNTR",
#if RU_INCLUDE_DESC
    "RD_CNTR",
    "Read counter value",
#endif
    RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS_RD_CNTR_FIELD_MASK,
    0,
    RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS_RD_CNTR_FIELD_WIDTH,
    RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS_RD_CNTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS_RESERVED2
 ******************************************************************************/
const ru_field_rec RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS_RESERVED2_FIELD =
{
    "RESERVED2",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS_RESERVED2_FIELD_MASK,
    0,
    RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS_RESERVED2_FIELD_WIDTH,
    RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS_RESERVED2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS2_READ_ADDR
 ******************************************************************************/
const ru_field_rec RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS2_READ_ADDR_FIELD =
{
    "READ_ADDR",
#if RU_INCLUDE_DESC
    "READ_ADDR",
    "Current read address",
#endif
    RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS2_READ_ADDR_FIELD_MASK,
    0,
    RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS2_READ_ADDR_FIELD_WIDTH,
    RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS2_READ_ADDR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS2_USED_WORDS
 ******************************************************************************/
const ru_field_rec RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS2_USED_WORDS_FIELD =
{
    "USED_WORDS",
#if RU_INCLUDE_DESC
    "USED_WORDS",
    "Used words",
#endif
    RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS2_USED_WORDS_FIELD_MASK,
    0,
    RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS2_USED_WORDS_FIELD_WIDTH,
    RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS2_USED_WORDS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS2_RESERVED0
 ******************************************************************************/
const ru_field_rec RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS2_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS2_RESERVED0_FIELD_MASK,
    0,
    RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS2_RESERVED0_FIELD_WIDTH,
    RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS2_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_DATA1_DATA
 ******************************************************************************/
const ru_field_rec RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_DATA1_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "DATA",
    "Data",
#endif
    RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_DATA1_DATA_FIELD_MASK,
    0,
    RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_DATA1_DATA_FIELD_WIDTH,
    RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_DATA1_DATA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_DATA2_DATA
 ******************************************************************************/
const ru_field_rec RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_DATA2_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "DATA",
    "Data",
#endif
    RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_DATA2_DATA_FIELD_MASK,
    0,
    RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_DATA2_DATA_FIELD_WIDTH,
    RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_DATA2_DATA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_DATA1_DATA
 ******************************************************************************/
const ru_field_rec RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_DATA1_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "DATA",
    "Data",
#endif
    RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_DATA1_DATA_FIELD_MASK,
    0,
    RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_DATA1_DATA_FIELD_WIDTH,
    RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_DATA1_DATA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_DATA2_DATA
 ******************************************************************************/
const ru_field_rec RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_DATA2_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "DATA",
    "Data",
#endif
    RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_DATA2_DATA_FIELD_MASK,
    0,
    RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_DATA2_DATA_FIELD_WIDTH,
    RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_DATA2_DATA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_DEBUG_DDR_HDR_FIFO_DATA1_DATA
 ******************************************************************************/
const ru_field_rec RNR_QUAD_DEBUG_DDR_HDR_FIFO_DATA1_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "DATA",
    "Data",
#endif
    RNR_QUAD_DEBUG_DDR_HDR_FIFO_DATA1_DATA_FIELD_MASK,
    0,
    RNR_QUAD_DEBUG_DDR_HDR_FIFO_DATA1_DATA_FIELD_WIDTH,
    RNR_QUAD_DEBUG_DDR_HDR_FIFO_DATA1_DATA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_DEBUG_DDR_HDR_FIFO_DATA2_DATA
 ******************************************************************************/
const ru_field_rec RNR_QUAD_DEBUG_DDR_HDR_FIFO_DATA2_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "DATA",
    "Data",
#endif
    RNR_QUAD_DEBUG_DDR_HDR_FIFO_DATA2_DATA_FIELD_MASK,
    0,
    RNR_QUAD_DEBUG_DDR_HDR_FIFO_DATA2_DATA_FIELD_WIDTH,
    RNR_QUAD_DEBUG_DDR_HDR_FIFO_DATA2_DATA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_EXT_FLOWCTRL_CONFIG_TOKEN_VAL_VAL
 ******************************************************************************/
const ru_field_rec RNR_QUAD_EXT_FLOWCTRL_CONFIG_TOKEN_VAL_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "VAL",
    "Value",
#endif
    RNR_QUAD_EXT_FLOWCTRL_CONFIG_TOKEN_VAL_VAL_FIELD_MASK,
    0,
    RNR_QUAD_EXT_FLOWCTRL_CONFIG_TOKEN_VAL_VAL_FIELD_WIDTH,
    RNR_QUAD_EXT_FLOWCTRL_CONFIG_TOKEN_VAL_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_UBUS_DECODE_CFG_PSRAM_UBUS_DECODE_VAL
 ******************************************************************************/
const ru_field_rec RNR_QUAD_UBUS_DECODE_CFG_PSRAM_UBUS_DECODE_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "VAL",
    "Value",
#endif
    RNR_QUAD_UBUS_DECODE_CFG_PSRAM_UBUS_DECODE_VAL_FIELD_MASK,
    0,
    RNR_QUAD_UBUS_DECODE_CFG_PSRAM_UBUS_DECODE_VAL_FIELD_WIDTH,
    RNR_QUAD_UBUS_DECODE_CFG_PSRAM_UBUS_DECODE_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RNR_QUAD_UBUS_DECODE_CFG_DDR_UBUS_DECODE_VAL
 ******************************************************************************/
const ru_field_rec RNR_QUAD_UBUS_DECODE_CFG_DDR_UBUS_DECODE_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "VAL",
    "Value",
#endif
    RNR_QUAD_UBUS_DECODE_CFG_DDR_UBUS_DECODE_VAL_FIELD_MASK,
    0,
    RNR_QUAD_UBUS_DECODE_CFG_DDR_UBUS_DECODE_VAL_FIELD_WIDTH,
    RNR_QUAD_UBUS_DECODE_CFG_DDR_UBUS_DECODE_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: RNR_QUAD_UBUS_SLV_RES_CNTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_UBUS_SLV_RES_CNTRL_FIELDS[] =
{
    &RNR_QUAD_UBUS_SLV_RES_CNTRL_MASK_ID_FIELD,
    &RNR_QUAD_UBUS_SLV_RES_CNTRL_RESERVED0_FIELD,
    &RNR_QUAD_UBUS_SLV_RES_CNTRL_REPIN_ESWAP_FIELD,
    &RNR_QUAD_UBUS_SLV_RES_CNTRL_REQOUT_ESWAP_FIELD,
    &RNR_QUAD_UBUS_SLV_RES_CNTRL_DEV_CLKEN_FIELD,
    &RNR_QUAD_UBUS_SLV_RES_CNTRL_DEV_ERR_FIELD,
    &RNR_QUAD_UBUS_SLV_RES_CNTRL_DEV_TIMEOUT_EN_FIELD,
    &RNR_QUAD_UBUS_SLV_RES_CNTRL_DEV_TIMEOUT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_UBUS_SLV_RES_CNTRL_REG = 
{
    "UBUS_SLV_RES_CNTRL",
#if RU_INCLUDE_DESC
    "RESPONDER_CTRL Register",
    "Responder side contol. These registers are releated to ubus Responder control",
#endif
    RNR_QUAD_UBUS_SLV_RES_CNTRL_REG_OFFSET,
    0,
    0,
    333,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    RNR_QUAD_UBUS_SLV_RES_CNTRL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_UBUS_SLV_VPB_BASE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_UBUS_SLV_VPB_BASE_FIELDS[] =
{
    &RNR_QUAD_UBUS_SLV_VPB_BASE_BASE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_UBUS_SLV_VPB_BASE_REG = 
{
    "UBUS_SLV_VPB_BASE",
#if RU_INCLUDE_DESC
    "VPB_BASE Register",
    "VPB Base address",
#endif
    RNR_QUAD_UBUS_SLV_VPB_BASE_REG_OFFSET,
    0,
    0,
    334,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_UBUS_SLV_VPB_BASE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_UBUS_SLV_VPB_MASK
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_UBUS_SLV_VPB_MASK_FIELDS[] =
{
    &RNR_QUAD_UBUS_SLV_VPB_MASK_MASK_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_UBUS_SLV_VPB_MASK_REG = 
{
    "UBUS_SLV_VPB_MASK",
#if RU_INCLUDE_DESC
    "VPB_MASK Register",
    "VPB mask address",
#endif
    RNR_QUAD_UBUS_SLV_VPB_MASK_REG_OFFSET,
    0,
    0,
    335,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_UBUS_SLV_VPB_MASK_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_UBUS_SLV_APB_BASE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_UBUS_SLV_APB_BASE_FIELDS[] =
{
    &RNR_QUAD_UBUS_SLV_APB_BASE_BASE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_UBUS_SLV_APB_BASE_REG = 
{
    "UBUS_SLV_APB_BASE",
#if RU_INCLUDE_DESC
    "APB_BASE Register",
    "APB Base address",
#endif
    RNR_QUAD_UBUS_SLV_APB_BASE_REG_OFFSET,
    0,
    0,
    336,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_UBUS_SLV_APB_BASE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_UBUS_SLV_APB_MASK
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_UBUS_SLV_APB_MASK_FIELDS[] =
{
    &RNR_QUAD_UBUS_SLV_APB_MASK_MASK_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_UBUS_SLV_APB_MASK_REG = 
{
    "UBUS_SLV_APB_MASK",
#if RU_INCLUDE_DESC
    "APB_MASK Register",
    "APB mask address",
#endif
    RNR_QUAD_UBUS_SLV_APB_MASK_REG_OFFSET,
    0,
    0,
    337,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_UBUS_SLV_APB_MASK_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_UBUS_SLV_DQM_BASE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_UBUS_SLV_DQM_BASE_FIELDS[] =
{
    &RNR_QUAD_UBUS_SLV_DQM_BASE_BASE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_UBUS_SLV_DQM_BASE_REG = 
{
    "UBUS_SLV_DQM_BASE",
#if RU_INCLUDE_DESC
    "DQM_BASE Register",
    "DQM Base address",
#endif
    RNR_QUAD_UBUS_SLV_DQM_BASE_REG_OFFSET,
    0,
    0,
    338,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_UBUS_SLV_DQM_BASE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_UBUS_SLV_DQM_MASK
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_UBUS_SLV_DQM_MASK_FIELDS[] =
{
    &RNR_QUAD_UBUS_SLV_DQM_MASK_MASK_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_UBUS_SLV_DQM_MASK_REG = 
{
    "UBUS_SLV_DQM_MASK",
#if RU_INCLUDE_DESC
    "DQM_MASK Register",
    "DQM mask address",
#endif
    RNR_QUAD_UBUS_SLV_DQM_MASK_REG_OFFSET,
    0,
    0,
    339,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_UBUS_SLV_DQM_MASK_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_ENG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_ENG_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_ENG_CFG_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_ENG_REG = 
{
    "PARSER_CORE_CONFIGURATION_ENG",
#if RU_INCLUDE_DESC
    "ENG Register",
    "Engineering Configuration reserved for Broadlight use",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_ENG_REG_OFFSET,
    0,
    0,
    340,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_ENG_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG_EXCEPTION_EN_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG_RESERVED0_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG_TCP_FLAGS_FILT_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG_RESERVED1_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG_PROFILE_US_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG_RESERVED2_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG_REG = 
{
    "PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG",
#if RU_INCLUDE_DESC
    "PARSER_MISC_CFG Register",
    "Parser Miscellaneous Configuration",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG_REG_OFFSET,
    0,
    0,
    341,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    6,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_0_1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_0_1_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_0_1_VID_0_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_0_1_RESERVED0_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_0_1_VID_0_EN_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_0_1_VID_1_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_0_1_RESERVED1_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_0_1_VID_1_EN_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_0_1_REG = 
{
    "PARSER_CORE_CONFIGURATION_VID_0_1",
#if RU_INCLUDE_DESC
    "VID_CONFIGURATION_0_1 Register",
    "Config VID Filter 0 & 1",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_0_1_REG_OFFSET,
    0,
    0,
    342,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    6,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_0_1_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_2_3
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_2_3_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_2_3_VID_2_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_2_3_RESERVED0_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_2_3_VID_2_EN_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_2_3_VID_3_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_2_3_RESERVED1_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_2_3_VID_3_EN_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_2_3_REG = 
{
    "PARSER_CORE_CONFIGURATION_VID_2_3",
#if RU_INCLUDE_DESC
    "VID_CONFIGURATION_2_3 Register",
    "Config VID Filter 2 & 3",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_2_3_REG_OFFSET,
    0,
    0,
    343,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    6,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_2_3_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_4_5
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_4_5_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_4_5_VID_4_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_4_5_RESERVED0_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_4_5_VID_4_EN_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_4_5_VID_5_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_4_5_RESERVED1_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_4_5_VID_5_EN_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_4_5_REG = 
{
    "PARSER_CORE_CONFIGURATION_VID_4_5",
#if RU_INCLUDE_DESC
    "VID_CONFIGURATION_4_5 Register",
    "Config VID Filter 4 & 5",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_4_5_REG_OFFSET,
    0,
    0,
    344,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    6,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_4_5_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_6_7
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_6_7_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_6_7_VID_6_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_6_7_RESERVED0_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_6_7_VID_6_EN_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_6_7_VID_7_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_6_7_RESERVED1_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_6_7_VID_7_EN_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_6_7_REG = 
{
    "PARSER_CORE_CONFIGURATION_VID_6_7",
#if RU_INCLUDE_DESC
    "VID_CONFIGURATION_6_7 Register",
    "Config VID Filter 6 & 7",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_6_7_REG_OFFSET,
    0,
    0,
    345,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    6,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_6_7_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER0_CFG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER0_CFG_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER0_CFG_IP_ADDRESS_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER0_CFG_REG = 
{
    "PARSER_CORE_CONFIGURATION_IP_FILTER0_CFG",
#if RU_INCLUDE_DESC
    "IP_FILTER0_CFG Register",
    "Config the IP Address filtering."
    "Notice that the enable bit is located in the IP_FILTERS_CFG[4]",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER0_CFG_REG_OFFSET,
    0,
    0,
    346,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER0_CFG_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER1_CFG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER1_CFG_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER1_CFG_IP_ADDRESS_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER1_CFG_REG = 
{
    "PARSER_CORE_CONFIGURATION_IP_FILTER1_CFG",
#if RU_INCLUDE_DESC
    "IP_FILTER1_CFG Register",
    "Config the IP Address filtering."
    "Notice that the enable bit is located in the IP_FILTERS_CFG[5]",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER1_CFG_REG_OFFSET,
    0,
    0,
    347,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER1_CFG_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER0_MASK_CFG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER0_MASK_CFG_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER0_MASK_CFG_IP_ADDRESS_MASK_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER0_MASK_CFG_REG = 
{
    "PARSER_CORE_CONFIGURATION_IP_FILTER0_MASK_CFG",
#if RU_INCLUDE_DESC
    "IP_FILTER0_MASK_CFG Register",
    "Config the IP Address masking."
    "Notice that the enable bit is located in the IP_FILTERS_CFG[4]",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER0_MASK_CFG_REG_OFFSET,
    0,
    0,
    348,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER0_MASK_CFG_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER1_MASK_CFG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER1_MASK_CFG_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER1_MASK_CFG_IP_ADDRESS_MASK_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER1_MASK_CFG_REG = 
{
    "PARSER_CORE_CONFIGURATION_IP_FILTER1_MASK_CFG",
#if RU_INCLUDE_DESC
    "IP_FILTER1_MASK_CFG Register",
    "Config the IP Address masking."
    "Notice that the enable bit is located in the IP_FILTERS_CFG[5]",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER1_MASK_CFG_REG_OFFSET,
    0,
    0,
    349,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER1_MASK_CFG_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG
 ******************************************************************************/
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
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_REG = 
{
    "PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG",
#if RU_INCLUDE_DESC
    "IP_FILTERS_CFG Register",
    "IP Address Filters (0..3) configurations:"
    ""
    "(1) SIP or DIP selection config per each filter"
    "(1) Valid bit per each filter",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_REG_OFFSET,
    0,
    0,
    350,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_SNAP_ORG_CODE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_SNAP_ORG_CODE_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_SNAP_ORG_CODE_CODE_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_SNAP_ORG_CODE_EN_RFC1042_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_SNAP_ORG_CODE_EN_8021Q_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_SNAP_ORG_CODE_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_SNAP_ORG_CODE_REG = 
{
    "PARSER_CORE_CONFIGURATION_SNAP_ORG_CODE",
#if RU_INCLUDE_DESC
    "SNAP_ORGANIZATION_CODE Register",
    "Identifies SNAP tunneling organization code",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_SNAP_ORG_CODE_REG_OFFSET,
    0,
    0,
    351,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_SNAP_ORG_CODE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_PPP_IP_PROT_CODE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_PPP_IP_PROT_CODE_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_PPP_IP_PROT_CODE_PPP_CODE_0_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_PPP_IP_PROT_CODE_PPP_CODE_1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_PPP_IP_PROT_CODE_REG = 
{
    "PARSER_CORE_CONFIGURATION_PPP_IP_PROT_CODE",
#if RU_INCLUDE_DESC
    "PPP_IP_PROTOCOL_CODE Register",
    "PPP Protocol Code to indicate L3 is IP",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_PPP_IP_PROT_CODE_REG_OFFSET,
    0,
    0,
    352,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_PPP_IP_PROT_CODE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_ETHTYPE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_ETHTYPE_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_ETHTYPE_ETHTYPE_QTAG_0_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_ETHTYPE_ETHTYPE_QTAG_1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_ETHTYPE_REG = 
{
    "PARSER_CORE_CONFIGURATION_QTAG_ETHTYPE",
#if RU_INCLUDE_DESC
    "QTAG_ETHERTYPE Register",
    "Ethertype values to identify the presence of VLAN QTAG",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_ETHTYPE_REG_OFFSET,
    0,
    0,
    353,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_ETHTYPE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_0_1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_0_1_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_0_1_ETHYPE_0_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_0_1_ETHYPE_1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_0_1_REG = 
{
    "PARSER_CORE_CONFIGURATION_USER_ETHTYPE_0_1",
#if RU_INCLUDE_DESC
    "USER_ETHERTYPE_CONFIGURTION_0_1 Register",
    "Configures user Ethertype values",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_0_1_REG_OFFSET,
    0,
    0,
    354,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_0_1_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_2_3
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_2_3_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_2_3_ETHYPE_2_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_2_3_ETHYPE_3_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_2_3_REG = 
{
    "PARSER_CORE_CONFIGURATION_USER_ETHTYPE_2_3",
#if RU_INCLUDE_DESC
    "USER_ETHERTYPE_CONFIGURTION_2_3 Register",
    "Configures user Ethertype values",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_2_3_REG_OFFSET,
    0,
    0,
    355,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_2_3_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_PROT_0_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_PROT_1_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_PROT_2_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_PROT_3_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_EN_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_RESERVED0_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_OFFSET_0_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_OFFSET_1_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_OFFSET_2_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_ETHTYPE_USER_OFFSET_3_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_REG = 
{
    "PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG",
#if RU_INCLUDE_DESC
    "USER_ETHERTYPE_CONFIGURATION Register",
    "Configure protocol and enables user Ethertype",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_REG_OFFSET,
    0,
    0,
    356,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    10,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_IPV6_HDR_EXT_FLTR_MASK_CFG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_IPV6_HDR_EXT_FLTR_MASK_CFG_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_IPV6_HDR_EXT_FLTR_MASK_CFG_HOP_BY_HOP_MATCH_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_IPV6_HDR_EXT_FLTR_MASK_CFG_ROUTING_EH_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_IPV6_HDR_EXT_FLTR_MASK_CFG_DEST_OPT_EH_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_IPV6_HDR_EXT_FLTR_MASK_CFG_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_IPV6_HDR_EXT_FLTR_MASK_CFG_REG = 
{
    "PARSER_CORE_CONFIGURATION_IPV6_HDR_EXT_FLTR_MASK_CFG",
#if RU_INCLUDE_DESC
    "IPV6_HDR_EXT_FLTR_MASK_CFG Register",
    "IPV6 Header Extension Filter Mask register",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_IPV6_HDR_EXT_FLTR_MASK_CFG_REG_OFFSET,
    0,
    0,
    357,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_IPV6_HDR_EXT_FLTR_MASK_CFG_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_QTAG_NEST_0_PROFILE_0_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_QTAG_NEST_0_PROFILE_1_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_QTAG_NEST_0_PROFILE_2_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_QTAG_NEST_1_PROFILE_0_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_QTAG_NEST_1_PROFILE_1_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_QTAG_NEST_1_PROFILE_2_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_REG = 
{
    "PARSER_CORE_CONFIGURATION_QTAG_NEST",
#if RU_INCLUDE_DESC
    "QTAG_NESTING Register",
    "Qtag Nesting config",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_REG_OFFSET,
    0,
    0,
    358,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    7,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_0_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_0_HARD_NEST_PROFILE_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_0_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_0_REG = 
{
    "PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_0",
#if RU_INCLUDE_DESC
    "QTAG_HARD_NEST_PROFILE_0 Register",
    "QTAG Hard Nest Profile 0",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_0_REG_OFFSET,
    0,
    0,
    359,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_0_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_1_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_1_HARD_NEST_PROFILE_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_1_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_1_REG = 
{
    "PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_1",
#if RU_INCLUDE_DESC
    "QTAG_HARD_NEST_PROFILE_1 Register",
    "QTAG Hard Nest Profile 1",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_1_REG_OFFSET,
    0,
    0,
    360,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_1_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_2
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_2_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_2_HARD_NEST_PROFILE_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_2_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_2_REG = 
{
    "PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_2",
#if RU_INCLUDE_DESC
    "QTAG_HARD_NEST_PROFILE_2 Register",
    "QTAG Hard Nest Profile 2",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_2_REG_OFFSET,
    0,
    0,
    361,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_2_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_IP_PROT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_IP_PROT_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_IP_PROT_USER_IP_PROT_0_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_IP_PROT_USER_IP_PROT_1_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_IP_PROT_USER_IP_PROT_2_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_IP_PROT_USER_IP_PROT_3_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_IP_PROT_REG = 
{
    "PARSER_CORE_CONFIGURATION_USER_IP_PROT",
#if RU_INCLUDE_DESC
    "USER_DEFINED_IP_PROTOCL Register",
    "IP Protocols to be matched to IP Protocol field and to be indicated in the output summary word",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_IP_PROT_REG_OFFSET,
    0,
    0,
    362,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_IP_PROT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_L
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_L_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_L_DA_FILT_LSB_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_L_REG = 
{
    "PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_L",
#if RU_INCLUDE_DESC
    "DA_FILT0_VAL_L Register",
    "Config DA filter 31:0",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_L_REG_OFFSET,
    0,
    0,
    363,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_L_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_H
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_H_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_H_DA_FILT_MSB_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_H_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_H_REG = 
{
    "PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_H",
#if RU_INCLUDE_DESC
    "DA_FILT0_VAL_H Register",
    "Config DA filter0 47:32",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_H_REG_OFFSET,
    0,
    0,
    364,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_H_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_VAL_L
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_VAL_L_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_VAL_L_DA_FILT_LSB_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_VAL_L_REG = 
{
    "PARSER_CORE_CONFIGURATION_DA_FILT1_VAL_L",
#if RU_INCLUDE_DESC
    "DA_FILT1_VAL_L Register",
    "Config DA filter1 31:0",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_VAL_L_REG_OFFSET,
    0,
    0,
    365,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_VAL_L_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_VAL_H
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_VAL_H_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_VAL_H_DA_FILT_MSB_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_VAL_H_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_VAL_H_REG = 
{
    "PARSER_CORE_CONFIGURATION_DA_FILT1_VAL_H",
#if RU_INCLUDE_DESC
    "DA_FILT1_VAL_H Register",
    "Config DA filter1 47:32",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_VAL_H_REG_OFFSET,
    0,
    0,
    366,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_VAL_H_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_L
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_L_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_L_DA_FILT_LSB_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_L_REG = 
{
    "PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_L",
#if RU_INCLUDE_DESC
    "DA_FILT2_VAL_L Register",
    "Config DA filter2 31:0",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_L_REG_OFFSET,
    0,
    0,
    367,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_L_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_H
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_H_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_H_DA_FILT_MSB_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_H_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_H_REG = 
{
    "PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_H",
#if RU_INCLUDE_DESC
    "DA_FILT2_VAL_H Register",
    "Config DA filter2 47:32",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_H_REG_OFFSET,
    0,
    0,
    368,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_H_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_L
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_L_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_L_DA_FILT_LSB_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_L_REG = 
{
    "PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_L",
#if RU_INCLUDE_DESC
    "DA_FILT3_VAL_L Register",
    "Config DA filter3 31:0",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_L_REG_OFFSET,
    0,
    0,
    369,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_L_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_H
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_H_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_H_DA_FILT_MSB_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_H_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_H_REG = 
{
    "PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_H",
#if RU_INCLUDE_DESC
    "DA_FILT3_VAL_H Register",
    "Config DA filter3 47:32",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_H_REG_OFFSET,
    0,
    0,
    370,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_H_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_L
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_L_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_L_DA_FILT_LSB_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_L_REG = 
{
    "PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_L",
#if RU_INCLUDE_DESC
    "DA_FILT4_VAL_L Register",
    "Config DA filter4 31:0",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_L_REG_OFFSET,
    0,
    0,
    371,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_L_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_H
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_H_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_H_DA_FILT_MSB_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_H_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_H_REG = 
{
    "PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_H",
#if RU_INCLUDE_DESC
    "DA_FILT4_VAL_H Register",
    "Config DA Filter4 47:32",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_H_REG_OFFSET,
    0,
    0,
    372,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_H_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_L
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_L_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_L_DA_FILT_LSB_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_L_REG = 
{
    "PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_L",
#if RU_INCLUDE_DESC
    "DA_FILT5_VAL_L Register",
    "Config DA filter5 31:0",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_L_REG_OFFSET,
    0,
    0,
    373,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_L_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_H
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_H_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_H_DA_FILT_MSB_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_H_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_H_REG = 
{
    "PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_H",
#if RU_INCLUDE_DESC
    "DA_FILT5_VAL_H Register",
    "Config DA Filter5 47:32",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_H_REG_OFFSET,
    0,
    0,
    374,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_H_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_L
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_L_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_L_DA_FILT_LSB_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_L_REG = 
{
    "PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_L",
#if RU_INCLUDE_DESC
    "DA_FILT6_VAL_L Register",
    "Config DA filter6 31:0",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_L_REG_OFFSET,
    0,
    0,
    375,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_L_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_H
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_H_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_H_DA_FILT_MSB_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_H_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_H_REG = 
{
    "PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_H",
#if RU_INCLUDE_DESC
    "DA_FILT6_VAL_H Register",
    "Config DA Filter6 47:32",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_H_REG_OFFSET,
    0,
    0,
    376,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_H_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_L
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_L_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_L_DA_FILT_LSB_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_L_REG = 
{
    "PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_L",
#if RU_INCLUDE_DESC
    "DA_FILT7_VAL_L Register",
    "Config DA filter7 31:0",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_L_REG_OFFSET,
    0,
    0,
    377,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_L_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_H
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_H_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_H_DA_FILT_MSB_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_H_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_H_REG = 
{
    "PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_H",
#if RU_INCLUDE_DESC
    "DA_FILT7_VAL_H Register",
    "Config DA Filter7 47:32",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_H_REG_OFFSET,
    0,
    0,
    378,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_H_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT8_VAL_L
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT8_VAL_L_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT8_VAL_L_DA_FILT_LSB_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT8_VAL_L_REG = 
{
    "PARSER_CORE_CONFIGURATION_DA_FILT8_VAL_L",
#if RU_INCLUDE_DESC
    "DA_FILT8_VAL_L Register",
    "Config DA filter8 31:0",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT8_VAL_L_REG_OFFSET,
    0,
    0,
    379,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT8_VAL_L_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT8_VAL_H
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT8_VAL_H_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT8_VAL_H_DA_FILT_MSB_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT8_VAL_H_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT8_VAL_H_REG = 
{
    "PARSER_CORE_CONFIGURATION_DA_FILT8_VAL_H",
#if RU_INCLUDE_DESC
    "DA_FILT8_VAL_H Register",
    "Config DA Filter8 47:32",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT8_VAL_H_REG_OFFSET,
    0,
    0,
    380,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT8_VAL_H_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_L
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_L_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_L_DA_FILT_MASK_L_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_L_REG = 
{
    "PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_L",
#if RU_INCLUDE_DESC
    "DA_FILT0_MASK_L Register",
    "Config DA Filter mask 15:0",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_L_REG_OFFSET,
    0,
    0,
    381,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_L_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_H
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_H_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_H_DA_FILT_MASK_MSB_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_H_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_H_REG = 
{
    "PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_H",
#if RU_INCLUDE_DESC
    "DA_FILT0_MASK_H Register",
    "Config DA Filter0 mask 47:32",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_H_REG_OFFSET,
    0,
    0,
    382,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_H_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_MASK_L
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_MASK_L_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_MASK_L_DA_FILT_MASK_L_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_MASK_L_REG = 
{
    "PARSER_CORE_CONFIGURATION_DA_FILT1_MASK_L",
#if RU_INCLUDE_DESC
    "DA_FILT1_MASK_L Register",
    "Config DA Filter1 mask 31:0",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_MASK_L_REG_OFFSET,
    0,
    0,
    383,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_MASK_L_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_MASK_H
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_MASK_H_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_MASK_H_DA_FILT_MASK_MSB_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_MASK_H_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_MASK_H_REG = 
{
    "PARSER_CORE_CONFIGURATION_DA_FILT1_MASK_H",
#if RU_INCLUDE_DESC
    "DA_FILT1_MASK_H Register",
    "Config DA Filter1 mask 47:32",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_MASK_H_REG_OFFSET,
    0,
    0,
    384,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_MASK_H_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0
 ******************************************************************************/
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
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_REG = 
{
    "PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0",
#if RU_INCLUDE_DESC
    "DA_FILT_VALID_CFG_PROFILE_0 Register",
    "Valid configuration of all DA filters: there is a dedicated bit per each DA filter that says if the current DA filter is valid or not. Used for on-the-fly DA filter value (mask) modifications, since the DA filter parameters are not assigned on single SW register.",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_REG_OFFSET,
    0,
    0,
    385,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    10,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1
 ******************************************************************************/
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
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_REG = 
{
    "PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1",
#if RU_INCLUDE_DESC
    "DA_FILT_VALID_CFG_PROFILE_1 Register",
    "Valid configuration of all DA filters: there is a dedicated bit per each DA filter that says if the current DA filter is valid or not. Used for on-the-fly DA filter value (mask) modifications, since the DA filter parameters are not assigned on single SW register.",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_REG_OFFSET,
    0,
    0,
    386,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    10,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2
 ******************************************************************************/
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
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_REG = 
{
    "PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2",
#if RU_INCLUDE_DESC
    "DA_FILT_VALID_CFG_PROFILE_2 Register",
    "Valid configuration of all DA filters: there is a dedicated bit per each DA filter that says if the current DA filter is valid or not. Used for on-the-fly DA filter value (mask) modifications, since the DA filter parameters are not assigned on single SW register.",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_REG_OFFSET,
    0,
    0,
    387,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    10,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_GRE_PROTOCOL_CFG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_GRE_PROTOCOL_CFG_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_GRE_PROTOCOL_CFG_GRE_PROTOCOL_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_GRE_PROTOCOL_CFG_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_GRE_PROTOCOL_CFG_REG = 
{
    "PARSER_CORE_CONFIGURATION_GRE_PROTOCOL_CFG",
#if RU_INCLUDE_DESC
    "GRE_PROTOCOL_CFG Register",
    "GRE Protocol",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_GRE_PROTOCOL_CFG_REG_OFFSET,
    0,
    0,
    388,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_GRE_PROTOCOL_CFG_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG_FIELDS[] =
{
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG_SIZE_PROFILE_0_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG_SIZE_PROFILE_1_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG_SIZE_PROFILE_2_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG_PRE_DA_DPROFILE_0_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG_PRE_DA_DPROFILE_1_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG_PRE_DA_DPROFILE_2_FIELD,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG_REG = 
{
    "PARSER_CORE_CONFIGURATION_PROP_TAG_CFG",
#if RU_INCLUDE_DESC
    "PROP_TAG_CFG Register",
    "Prop Tag Configuration",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG_REG_OFFSET,
    0,
    0,
    389,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    7,
    RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_FIELDS[] =
{
    &RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_USE_FIFO_FOR_DDR_ONLY_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_TOKEN_ARBITER_IS_RR_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_CHICKEN_NO_FLOWCTRL_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_RESERVED0_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_CONGEST_THRESHOLD_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_REG = 
{
    "GENERAL_CONFIG_DMA_ARB_CFG",
#if RU_INCLUDE_DESC
    "DMA_ARB_CFG Register",
    "DMA arbiter Configuration",
#endif
    RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_REG_OFFSET,
    0,
    0,
    390,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    6,
    RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_GENERAL_CONFIG_PSRAM0_BASE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_GENERAL_CONFIG_PSRAM0_BASE_FIELDS[] =
{
    &RNR_QUAD_GENERAL_CONFIG_PSRAM0_BASE_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_PSRAM0_BASE_REG = 
{
    "GENERAL_CONFIG_PSRAM0_BASE",
#if RU_INCLUDE_DESC
    "PSRAM0_BASE Register",
    "Configure PSRAM0 base"
    "",
#endif
    RNR_QUAD_GENERAL_CONFIG_PSRAM0_BASE_REG_OFFSET,
    0,
    0,
    391,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_GENERAL_CONFIG_PSRAM0_BASE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_GENERAL_CONFIG_PSRAM1_BASE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_GENERAL_CONFIG_PSRAM1_BASE_FIELDS[] =
{
    &RNR_QUAD_GENERAL_CONFIG_PSRAM1_BASE_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_PSRAM1_BASE_REG = 
{
    "GENERAL_CONFIG_PSRAM1_BASE",
#if RU_INCLUDE_DESC
    "PSRAM1_BASE Register",
    "Configure PSRAM1 base"
    "",
#endif
    RNR_QUAD_GENERAL_CONFIG_PSRAM1_BASE_REG_OFFSET,
    0,
    0,
    392,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_GENERAL_CONFIG_PSRAM1_BASE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_GENERAL_CONFIG_PSRAM2_BASE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_GENERAL_CONFIG_PSRAM2_BASE_FIELDS[] =
{
    &RNR_QUAD_GENERAL_CONFIG_PSRAM2_BASE_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_PSRAM2_BASE_REG = 
{
    "GENERAL_CONFIG_PSRAM2_BASE",
#if RU_INCLUDE_DESC
    "PSRAM2_BASE Register",
    "Configure PSRAM2 base"
    "",
#endif
    RNR_QUAD_GENERAL_CONFIG_PSRAM2_BASE_REG_OFFSET,
    0,
    0,
    393,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_GENERAL_CONFIG_PSRAM2_BASE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_GENERAL_CONFIG_PSRAM3_BASE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_GENERAL_CONFIG_PSRAM3_BASE_FIELDS[] =
{
    &RNR_QUAD_GENERAL_CONFIG_PSRAM3_BASE_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_PSRAM3_BASE_REG = 
{
    "GENERAL_CONFIG_PSRAM3_BASE",
#if RU_INCLUDE_DESC
    "PSRAM3_BASE Register",
    "Configure PSRAM3 base"
    "",
#endif
    RNR_QUAD_GENERAL_CONFIG_PSRAM3_BASE_REG_OFFSET,
    0,
    0,
    394,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_GENERAL_CONFIG_PSRAM3_BASE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_GENERAL_CONFIG_DDR0_BASE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_GENERAL_CONFIG_DDR0_BASE_FIELDS[] =
{
    &RNR_QUAD_GENERAL_CONFIG_DDR0_BASE_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_DDR0_BASE_REG = 
{
    "GENERAL_CONFIG_DDR0_BASE",
#if RU_INCLUDE_DESC
    "DDR0_BASE Register",
    "Configure DDR0 base"
    "",
#endif
    RNR_QUAD_GENERAL_CONFIG_DDR0_BASE_REG_OFFSET,
    0,
    0,
    395,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_GENERAL_CONFIG_DDR0_BASE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_GENERAL_CONFIG_DDR1_BASE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_GENERAL_CONFIG_DDR1_BASE_FIELDS[] =
{
    &RNR_QUAD_GENERAL_CONFIG_DDR1_BASE_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_DDR1_BASE_REG = 
{
    "GENERAL_CONFIG_DDR1_BASE",
#if RU_INCLUDE_DESC
    "DDR1_BASE Register",
    "Configure DDR1 base"
    "",
#endif
    RNR_QUAD_GENERAL_CONFIG_DDR1_BASE_REG_OFFSET,
    0,
    0,
    396,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_GENERAL_CONFIG_DDR1_BASE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_GENERAL_CONFIG_PSRAM0_MASK
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_GENERAL_CONFIG_PSRAM0_MASK_FIELDS[] =
{
    &RNR_QUAD_GENERAL_CONFIG_PSRAM0_MASK_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_PSRAM0_MASK_REG = 
{
    "GENERAL_CONFIG_PSRAM0_MASK",
#if RU_INCLUDE_DESC
    "PSRAM0_MASK Register",
    "Configure PSRAM0 mask"
    "",
#endif
    RNR_QUAD_GENERAL_CONFIG_PSRAM0_MASK_REG_OFFSET,
    0,
    0,
    397,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_GENERAL_CONFIG_PSRAM0_MASK_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_GENERAL_CONFIG_PSRAM1_MASK
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_GENERAL_CONFIG_PSRAM1_MASK_FIELDS[] =
{
    &RNR_QUAD_GENERAL_CONFIG_PSRAM1_MASK_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_PSRAM1_MASK_REG = 
{
    "GENERAL_CONFIG_PSRAM1_MASK",
#if RU_INCLUDE_DESC
    "PSRAM1_MASK Register",
    "Configure PSRAM1 mask"
    "",
#endif
    RNR_QUAD_GENERAL_CONFIG_PSRAM1_MASK_REG_OFFSET,
    0,
    0,
    398,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_GENERAL_CONFIG_PSRAM1_MASK_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_GENERAL_CONFIG_PSRAM2_MASK
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_GENERAL_CONFIG_PSRAM2_MASK_FIELDS[] =
{
    &RNR_QUAD_GENERAL_CONFIG_PSRAM2_MASK_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_PSRAM2_MASK_REG = 
{
    "GENERAL_CONFIG_PSRAM2_MASK",
#if RU_INCLUDE_DESC
    "PSRAM2_MASK Register",
    "Configure PSRAM2 mask"
    "",
#endif
    RNR_QUAD_GENERAL_CONFIG_PSRAM2_MASK_REG_OFFSET,
    0,
    0,
    399,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_GENERAL_CONFIG_PSRAM2_MASK_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_GENERAL_CONFIG_PSRAM3_MASK
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_GENERAL_CONFIG_PSRAM3_MASK_FIELDS[] =
{
    &RNR_QUAD_GENERAL_CONFIG_PSRAM3_MASK_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_PSRAM3_MASK_REG = 
{
    "GENERAL_CONFIG_PSRAM3_MASK",
#if RU_INCLUDE_DESC
    "PSRAM3_MASK Register",
    "Configure PSRAM3 mask"
    "",
#endif
    RNR_QUAD_GENERAL_CONFIG_PSRAM3_MASK_REG_OFFSET,
    0,
    0,
    400,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_GENERAL_CONFIG_PSRAM3_MASK_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_GENERAL_CONFIG_DDR0_MASK
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_GENERAL_CONFIG_DDR0_MASK_FIELDS[] =
{
    &RNR_QUAD_GENERAL_CONFIG_DDR0_MASK_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_DDR0_MASK_REG = 
{
    "GENERAL_CONFIG_DDR0_MASK",
#if RU_INCLUDE_DESC
    "DDR0_MASK Register",
    "Configure DDR0 mask"
    "",
#endif
    RNR_QUAD_GENERAL_CONFIG_DDR0_MASK_REG_OFFSET,
    0,
    0,
    401,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_GENERAL_CONFIG_DDR0_MASK_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_GENERAL_CONFIG_DDR1_MASK
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_GENERAL_CONFIG_DDR1_MASK_FIELDS[] =
{
    &RNR_QUAD_GENERAL_CONFIG_DDR1_MASK_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_DDR1_MASK_REG = 
{
    "GENERAL_CONFIG_DDR1_MASK",
#if RU_INCLUDE_DESC
    "DDR1_MASK Register",
    "Configure DDR1 mask"
    "",
#endif
    RNR_QUAD_GENERAL_CONFIG_DDR1_MASK_REG_OFFSET,
    0,
    0,
    402,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_GENERAL_CONFIG_DDR1_MASK_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_FIELDS[] =
{
    &RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_COUNTER_LSB_SEL_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_RESERVED0_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_0_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_1_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_2_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_3_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_4_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_ENABLE_TRACE_CORE_5_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_REG = 
{
    "GENERAL_CONFIG_PROFILING_CONFIG",
#if RU_INCLUDE_DESC
    "PROFILING_CONFIG Register",
    "Profiling configuration",
#endif
    RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_REG_OFFSET,
    0,
    0,
    403,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_GENERAL_CONFIG_BKPT_0_CFG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_GENERAL_CONFIG_BKPT_0_CFG_FIELDS[] =
{
    &RNR_QUAD_GENERAL_CONFIG_BKPT_0_CFG_ADDR_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_BKPT_0_CFG_RESERVED0_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_BKPT_0_CFG_THREAD_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_BKPT_0_CFG_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_BKPT_0_CFG_REG = 
{
    "GENERAL_CONFIG_BKPT_0_CFG",
#if RU_INCLUDE_DESC
    "BKPT_CFG_0 Register",
    "Breakpoint 0 configuration.",
#endif
    RNR_QUAD_GENERAL_CONFIG_BKPT_0_CFG_REG_OFFSET,
    0,
    0,
    404,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    RNR_QUAD_GENERAL_CONFIG_BKPT_0_CFG_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_GENERAL_CONFIG_BKPT_1_CFG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_GENERAL_CONFIG_BKPT_1_CFG_FIELDS[] =
{
    &RNR_QUAD_GENERAL_CONFIG_BKPT_1_CFG_ADDR_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_BKPT_1_CFG_RESERVED0_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_BKPT_1_CFG_THREAD_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_BKPT_1_CFG_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_BKPT_1_CFG_REG = 
{
    "GENERAL_CONFIG_BKPT_1_CFG",
#if RU_INCLUDE_DESC
    "BKPT_CFG_1 Register",
    "Breakpoint 1 configuration.",
#endif
    RNR_QUAD_GENERAL_CONFIG_BKPT_1_CFG_REG_OFFSET,
    0,
    0,
    405,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    RNR_QUAD_GENERAL_CONFIG_BKPT_1_CFG_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_GENERAL_CONFIG_BKPT_2_CFG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_GENERAL_CONFIG_BKPT_2_CFG_FIELDS[] =
{
    &RNR_QUAD_GENERAL_CONFIG_BKPT_2_CFG_ADDR_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_BKPT_2_CFG_RESERVED0_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_BKPT_2_CFG_THREAD_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_BKPT_2_CFG_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_BKPT_2_CFG_REG = 
{
    "GENERAL_CONFIG_BKPT_2_CFG",
#if RU_INCLUDE_DESC
    "BKPT_CFG_2 Register",
    "Breakpoint 2 configuration.",
#endif
    RNR_QUAD_GENERAL_CONFIG_BKPT_2_CFG_REG_OFFSET,
    0,
    0,
    406,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    RNR_QUAD_GENERAL_CONFIG_BKPT_2_CFG_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_GENERAL_CONFIG_BKPT_3_CFG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_GENERAL_CONFIG_BKPT_3_CFG_FIELDS[] =
{
    &RNR_QUAD_GENERAL_CONFIG_BKPT_3_CFG_ADDR_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_BKPT_3_CFG_RESERVED0_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_BKPT_3_CFG_THREAD_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_BKPT_3_CFG_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_BKPT_3_CFG_REG = 
{
    "GENERAL_CONFIG_BKPT_3_CFG",
#if RU_INCLUDE_DESC
    "BKPT_CFG_3 Register",
    "Breakpoint 3 configuration.",
#endif
    RNR_QUAD_GENERAL_CONFIG_BKPT_3_CFG_REG_OFFSET,
    0,
    0,
    407,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    RNR_QUAD_GENERAL_CONFIG_BKPT_3_CFG_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_GENERAL_CONFIG_BKPT_4_CFG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_GENERAL_CONFIG_BKPT_4_CFG_FIELDS[] =
{
    &RNR_QUAD_GENERAL_CONFIG_BKPT_4_CFG_ADDR_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_BKPT_4_CFG_RESERVED0_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_BKPT_4_CFG_THREAD_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_BKPT_4_CFG_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_BKPT_4_CFG_REG = 
{
    "GENERAL_CONFIG_BKPT_4_CFG",
#if RU_INCLUDE_DESC
    "BKPT_CFG_4 Register",
    "Breakpoint 4 configuration.",
#endif
    RNR_QUAD_GENERAL_CONFIG_BKPT_4_CFG_REG_OFFSET,
    0,
    0,
    408,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    RNR_QUAD_GENERAL_CONFIG_BKPT_4_CFG_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_GENERAL_CONFIG_BKPT_5_CFG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_GENERAL_CONFIG_BKPT_5_CFG_FIELDS[] =
{
    &RNR_QUAD_GENERAL_CONFIG_BKPT_5_CFG_ADDR_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_BKPT_5_CFG_RESERVED0_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_BKPT_5_CFG_THREAD_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_BKPT_5_CFG_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_BKPT_5_CFG_REG = 
{
    "GENERAL_CONFIG_BKPT_5_CFG",
#if RU_INCLUDE_DESC
    "BKPT_CFG_5 Register",
    "Breakpoint 5 configuration.",
#endif
    RNR_QUAD_GENERAL_CONFIG_BKPT_5_CFG_REG_OFFSET,
    0,
    0,
    409,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    RNR_QUAD_GENERAL_CONFIG_BKPT_5_CFG_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_GENERAL_CONFIG_BKPT_6_CFG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_GENERAL_CONFIG_BKPT_6_CFG_FIELDS[] =
{
    &RNR_QUAD_GENERAL_CONFIG_BKPT_6_CFG_ADDR_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_BKPT_6_CFG_RESERVED0_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_BKPT_6_CFG_THREAD_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_BKPT_6_CFG_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_BKPT_6_CFG_REG = 
{
    "GENERAL_CONFIG_BKPT_6_CFG",
#if RU_INCLUDE_DESC
    "BKPT_CFG_6 Register",
    "Breakpoint 6 configuration.",
#endif
    RNR_QUAD_GENERAL_CONFIG_BKPT_6_CFG_REG_OFFSET,
    0,
    0,
    410,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    RNR_QUAD_GENERAL_CONFIG_BKPT_6_CFG_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_GENERAL_CONFIG_BKPT_7_CFG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_GENERAL_CONFIG_BKPT_7_CFG_FIELDS[] =
{
    &RNR_QUAD_GENERAL_CONFIG_BKPT_7_CFG_ADDR_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_BKPT_7_CFG_RESERVED0_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_BKPT_7_CFG_THREAD_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_BKPT_7_CFG_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_BKPT_7_CFG_REG = 
{
    "GENERAL_CONFIG_BKPT_7_CFG",
#if RU_INCLUDE_DESC
    "BKPT_CFG_7 Register",
    "Breakpoint 7 configuration.",
#endif
    RNR_QUAD_GENERAL_CONFIG_BKPT_7_CFG_REG_OFFSET,
    0,
    0,
    411,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    RNR_QUAD_GENERAL_CONFIG_BKPT_7_CFG_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_GENERAL_CONFIG_BKPT_GEN_CFG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_GENERAL_CONFIG_BKPT_GEN_CFG_FIELDS[] =
{
    &RNR_QUAD_GENERAL_CONFIG_BKPT_GEN_CFG_HANDLER_ADDR_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_BKPT_GEN_CFG_RESERVED0_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_BKPT_GEN_CFG_UPDATE_PC_VALUE_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_BKPT_GEN_CFG_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_BKPT_GEN_CFG_REG = 
{
    "GENERAL_CONFIG_BKPT_GEN_CFG",
#if RU_INCLUDE_DESC
    "BKPT_CFG_GEN Register",
    "Breakpoint general configuration.",
#endif
    RNR_QUAD_GENERAL_CONFIG_BKPT_GEN_CFG_REG_OFFSET,
    0,
    0,
    412,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    RNR_QUAD_GENERAL_CONFIG_BKPT_GEN_CFG_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG
 ******************************************************************************/
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
    &RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_REG = 
{
    "GENERAL_CONFIG_POWERSAVE_CONFIG",
#if RU_INCLUDE_DESC
    "POWERSAVE_CONFIG Register",
    "Powersaving  configuration",
#endif
    RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_REG_OFFSET,
    0,
    0,
    413,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_FIELDS[] =
{
    &RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_0_STATUS_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_1_STATUS_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_2_STATUS_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_3_STATUS_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_4_STATUS_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_CORE_5_STATUS_FIELD,
    &RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_REG = 
{
    "GENERAL_CONFIG_POWERSAVE_STATUS",
#if RU_INCLUDE_DESC
    "POWERSAVE_STATUS Register",
    "Powersave status indications",
#endif
    RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_REG_OFFSET,
    0,
    0,
    414,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    7,
    RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_DEBUG_FIFO_CONFIG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_DEBUG_FIFO_CONFIG_FIELDS[] =
{
    &RNR_QUAD_DEBUG_FIFO_CONFIG_PSRAM_HDR_SW_RST_FIELD,
    &RNR_QUAD_DEBUG_FIFO_CONFIG_PSRAM_DATA_SW_RST_FIELD,
    &RNR_QUAD_DEBUG_FIFO_CONFIG_DDR_HDR_SW_RST_FIELD,
    &RNR_QUAD_DEBUG_FIFO_CONFIG_RESERVED0_FIELD,
    &RNR_QUAD_DEBUG_FIFO_CONFIG_PSRAM_HDR_SW_RD_ADDR_FIELD,
    &RNR_QUAD_DEBUG_FIFO_CONFIG_PSRAM_DATA_SW_RD_ADDR_FIELD,
    &RNR_QUAD_DEBUG_FIFO_CONFIG_DDR_HDR_SW_RD_ADDR_FIELD,
    &RNR_QUAD_DEBUG_FIFO_CONFIG_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_DEBUG_FIFO_CONFIG_REG = 
{
    "DEBUG_FIFO_CONFIG",
#if RU_INCLUDE_DESC
    "FIFO_CONFIG Register",
    "FIFOs configuration",
#endif
    RNR_QUAD_DEBUG_FIFO_CONFIG_REG_OFFSET,
    0,
    0,
    415,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    RNR_QUAD_DEBUG_FIFO_CONFIG_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_FIELDS[] =
{
    &RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_FULL_FIELD,
    &RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_EMPTY_FIELD,
    &RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_RESERVED0_FIELD,
    &RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_PUSH_WR_CNTR_FIELD,
    &RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_RESERVED1_FIELD,
    &RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_POP_RD_CNTR_FIELD,
    &RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_RESERVED2_FIELD,
    &RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_USED_WORDS_FIELD,
    &RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_RESERVED3_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_REG = 
{
    "DEBUG_PSRAM_HDR_FIFO_STATUS",
#if RU_INCLUDE_DESC
    "PSRAM_HDR_FIFO_STATUS Register",
    "PSRAM Header FIFO status",
#endif
    RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_REG_OFFSET,
    0,
    0,
    416,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_FIELDS[] =
{
    &RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_FULL_FIELD,
    &RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_EMPTY_FIELD,
    &RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_ALMOST_FULL_FIELD,
    &RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_RESERVED0_FIELD,
    &RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_PUSH_WR_CNTR_FIELD,
    &RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_RESERVED1_FIELD,
    &RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_POP_RD_CNTR_FIELD,
    &RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_RESERVED2_FIELD,
    &RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_USED_WORDS_FIELD,
    &RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_RESERVED3_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_REG = 
{
    "DEBUG_PSRAM_DATA_FIFO_STATUS",
#if RU_INCLUDE_DESC
    "PSRAM_DATA_FIFO_STATUS Register",
    "PSRAM Data FIFO status",
#endif
    RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_REG_OFFSET,
    0,
    0,
    417,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    10,
    RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_FIELDS[] =
{
    &RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_FULL_FIELD,
    &RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_EMPTY_FIELD,
    &RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_RESERVED0_FIELD,
    &RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_PUSH_WR_CNTR_FIELD,
    &RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_RESERVED1_FIELD,
    &RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_POP_RD_CNTR_FIELD,
    &RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_RESERVED2_FIELD,
    &RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_USED_WORDS_FIELD,
    &RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_RESERVED3_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_REG = 
{
    "DEBUG_DDR_HDR_FIFO_STATUS",
#if RU_INCLUDE_DESC
    "DDR_HDR_FIFO_STATUS Register",
    "DDR Header FIFO status",
#endif
    RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_REG_OFFSET,
    0,
    0,
    418,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS_FIELDS[] =
{
    &RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS_FULL_FIELD,
    &RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS_EMPTY_FIELD,
    &RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS_ALMOST_FULL_FIELD,
    &RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS_RESERVED0_FIELD,
    &RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS_WR_CNTR_FIELD,
    &RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS_RESERVED1_FIELD,
    &RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS_RD_CNTR_FIELD,
    &RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS_RESERVED2_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS_REG = 
{
    "DEBUG_DDR_DATA_FIFO_STATUS",
#if RU_INCLUDE_DESC
    "DDR_DATA_FIFO_STATUS Register",
    "DDR Data FIFO status",
#endif
    RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS_REG_OFFSET,
    0,
    0,
    419,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS2
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS2_FIELDS[] =
{
    &RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS2_READ_ADDR_FIELD,
    &RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS2_USED_WORDS_FIELD,
    &RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS2_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS2_REG = 
{
    "DEBUG_DDR_DATA_FIFO_STATUS2",
#if RU_INCLUDE_DESC
    "DDR_DATA_FIFO_STATUS2 Register",
    "DDR Data FIFO status 2",
#endif
    RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS2_REG_OFFSET,
    0,
    0,
    420,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS2_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_DATA1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_DATA1_FIELDS[] =
{
    &RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_DATA1_DATA_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_DATA1_REG = 
{
    "DEBUG_PSRAM_HDR_FIFO_DATA1",
#if RU_INCLUDE_DESC
    "PSRAM_HDR_FIFO_DATA1 Register",
    "Read contents of FIFO memory",
#endif
    RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_DATA1_REG_OFFSET,
    0,
    0,
    421,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_DATA1_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_DATA2
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_DATA2_FIELDS[] =
{
    &RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_DATA2_DATA_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_DATA2_REG = 
{
    "DEBUG_PSRAM_HDR_FIFO_DATA2",
#if RU_INCLUDE_DESC
    "PSRAM_HDR_FIFO_DATA2 Register",
    "Read contents of FIFO memory",
#endif
    RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_DATA2_REG_OFFSET,
    0,
    0,
    422,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_DATA2_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_DATA1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_DATA1_FIELDS[] =
{
    &RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_DATA1_DATA_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_DATA1_REG = 
{
    "DEBUG_PSRAM_DATA_FIFO_DATA1",
#if RU_INCLUDE_DESC
    "PSRAM_DATA_FIFO_DATA1 Register",
    "Read contents of FIFO memory",
#endif
    RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_DATA1_REG_OFFSET,
    0,
    0,
    423,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_DATA1_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_DATA2
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_DATA2_FIELDS[] =
{
    &RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_DATA2_DATA_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_DATA2_REG = 
{
    "DEBUG_PSRAM_DATA_FIFO_DATA2",
#if RU_INCLUDE_DESC
    "PSRAM_DATA_FIFO_DATA2 Register",
    "Read contents of FIFO memory",
#endif
    RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_DATA2_REG_OFFSET,
    0,
    0,
    424,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_DATA2_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_DEBUG_DDR_HDR_FIFO_DATA1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_DEBUG_DDR_HDR_FIFO_DATA1_FIELDS[] =
{
    &RNR_QUAD_DEBUG_DDR_HDR_FIFO_DATA1_DATA_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_DEBUG_DDR_HDR_FIFO_DATA1_REG = 
{
    "DEBUG_DDR_HDR_FIFO_DATA1",
#if RU_INCLUDE_DESC
    "DDR_HDR_FIFO_DATA1 Register",
    "Read contents of FIFO memory",
#endif
    RNR_QUAD_DEBUG_DDR_HDR_FIFO_DATA1_REG_OFFSET,
    0,
    0,
    425,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_DEBUG_DDR_HDR_FIFO_DATA1_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_DEBUG_DDR_HDR_FIFO_DATA2
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_DEBUG_DDR_HDR_FIFO_DATA2_FIELDS[] =
{
    &RNR_QUAD_DEBUG_DDR_HDR_FIFO_DATA2_DATA_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_DEBUG_DDR_HDR_FIFO_DATA2_REG = 
{
    "DEBUG_DDR_HDR_FIFO_DATA2",
#if RU_INCLUDE_DESC
    "DDR_HDR_FIFO_DATA2 Register",
    "Read contents of FIFO memory",
#endif
    RNR_QUAD_DEBUG_DDR_HDR_FIFO_DATA2_REG_OFFSET,
    0,
    0,
    426,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_DEBUG_DDR_HDR_FIFO_DATA2_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_EXT_FLOWCTRL_CONFIG_TOKEN_VAL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_EXT_FLOWCTRL_CONFIG_TOKEN_VAL_FIELDS[] =
{
    &RNR_QUAD_EXT_FLOWCTRL_CONFIG_TOKEN_VAL_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_EXT_FLOWCTRL_CONFIG_TOKEN_VAL_REG = 
{
    "EXT_FLOWCTRL_CONFIG_TOKEN_VAL",
#if RU_INCLUDE_DESC
    "TOKEN %i Register",
    "Token value for flow control",
#endif
    RNR_QUAD_EXT_FLOWCTRL_CONFIG_TOKEN_VAL_REG_OFFSET,
    RNR_QUAD_EXT_FLOWCTRL_CONFIG_TOKEN_VAL_REG_RAM_CNT,
    4,
    427,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_EXT_FLOWCTRL_CONFIG_TOKEN_VAL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_UBUS_DECODE_CFG_PSRAM_UBUS_DECODE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_UBUS_DECODE_CFG_PSRAM_UBUS_DECODE_FIELDS[] =
{
    &RNR_QUAD_UBUS_DECODE_CFG_PSRAM_UBUS_DECODE_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_UBUS_DECODE_CFG_PSRAM_UBUS_DECODE_REG = 
{
    "UBUS_DECODE_CFG_PSRAM_UBUS_DECODE",
#if RU_INCLUDE_DESC
    "PSRAM_UBUS_DECODE %i Register",
    "Decode for PSRAM Queue",
#endif
    RNR_QUAD_UBUS_DECODE_CFG_PSRAM_UBUS_DECODE_REG_OFFSET,
    RNR_QUAD_UBUS_DECODE_CFG_PSRAM_UBUS_DECODE_REG_RAM_CNT,
    4,
    428,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_UBUS_DECODE_CFG_PSRAM_UBUS_DECODE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: RNR_QUAD_UBUS_DECODE_CFG_DDR_UBUS_DECODE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_QUAD_UBUS_DECODE_CFG_DDR_UBUS_DECODE_FIELDS[] =
{
    &RNR_QUAD_UBUS_DECODE_CFG_DDR_UBUS_DECODE_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_QUAD_UBUS_DECODE_CFG_DDR_UBUS_DECODE_REG = 
{
    "UBUS_DECODE_CFG_DDR_UBUS_DECODE",
#if RU_INCLUDE_DESC
    "DDR_UBUS_DECODE %i Register",
    "Decode for DDR Queue",
#endif
    RNR_QUAD_UBUS_DECODE_CFG_DDR_UBUS_DECODE_REG_OFFSET,
    RNR_QUAD_UBUS_DECODE_CFG_DDR_UBUS_DECODE_REG_RAM_CNT,
    4,
    429,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_QUAD_UBUS_DECODE_CFG_DDR_UBUS_DECODE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Block: RNR_QUAD
 ******************************************************************************/
static const ru_reg_rec *RNR_QUAD_REGS[] =
{
    &RNR_QUAD_UBUS_SLV_RES_CNTRL_REG,
    &RNR_QUAD_UBUS_SLV_VPB_BASE_REG,
    &RNR_QUAD_UBUS_SLV_VPB_MASK_REG,
    &RNR_QUAD_UBUS_SLV_APB_BASE_REG,
    &RNR_QUAD_UBUS_SLV_APB_MASK_REG,
    &RNR_QUAD_UBUS_SLV_DQM_BASE_REG,
    &RNR_QUAD_UBUS_SLV_DQM_MASK_REG,
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
    &RNR_QUAD_UBUS_DECODE_CFG_PSRAM_UBUS_DECODE_REG,
    &RNR_QUAD_UBUS_DECODE_CFG_DDR_UBUS_DECODE_REG,
};

unsigned long RNR_QUAD_ADDRS[] =
{
    0x82d08000,
};

const ru_block_rec RNR_QUAD_BLOCK = 
{
    "RNR_QUAD",
    RNR_QUAD_ADDRS,
    1,
    97,
    RNR_QUAD_REGS
};

/* End of file XRDP_RNR_QUAD.c */
