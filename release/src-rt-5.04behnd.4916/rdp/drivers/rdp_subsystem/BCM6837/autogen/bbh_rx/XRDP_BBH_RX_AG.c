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


#include "XRDP_BBH_RX_AG.h"

/******************************************************************************
 * Register: NAME: BBH_RX_GENERAL_CONFIGURATION_BBCFG, TYPE: Type_BBH_RX_BBHRX_GENERAL_CONFIGURATION_BBCFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: SDMABBID *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_BBCFG_SDMABBID_FIELD =
{
    "SDMABBID",
#if RU_INCLUDE_DESC
    "",
    "SDMA BB ID. This ID defines the BB ID of the SDMA that the BBH communicates with.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_BBCFG_SDMABBID_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_BBCFG_SDMABBID_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_BBCFG_SDMABBID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DISPBBID *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_BBCFG_DISPBBID_FIELD =
{
    "DISPBBID",
#if RU_INCLUDE_DESC
    "",
    "Dispatcher BB ID. This ID defines the BB ID of the Dispatcher that the BBH communicates with.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_BBCFG_DISPBBID_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_BBCFG_DISPBBID_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_BBCFG_DISPBBID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SBPMBBID *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_BBCFG_SBPMBBID_FIELD =
{
    "SBPMBBID",
#if RU_INCLUDE_DESC
    "",
    "SBPM BB ID. This ID defines the BB ID of the SBPM that the BBH communicates with.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_BBCFG_SBPMBBID_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_BBCFG_SBPMBBID_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_BBCFG_SBPMBBID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: RNRBBID *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_BBCFG_RNRBBID_FIELD =
{
    "RNRBBID",
#if RU_INCLUDE_DESC
    "",
    "Runner BB ID. This ID defines the BB ID of the Runner (TM Runner) that the BBH communicates with.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_BBCFG_RNRBBID_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_BBCFG_RNRBBID_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_BBCFG_RNRBBID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_BBCFG_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_BBCFG_SDMABBID_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_BBCFG_DISPBBID_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_BBCFG_SBPMBBID_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_BBCFG_RNRBBID_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_GENERAL_CONFIGURATION_BBCFG *****/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_BBCFG_REG =
{
    "GENERAL_CONFIGURATION_BBCFG",
#if RU_INCLUDE_DESC
    "BROADBUS_CONFIGURATION Register",
    "Each BBH unit has its own position on the BB tree. The BB defines the Route address for the specific unit.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_BBCFG_REG_OFFSET },
    0,
    0,
    22,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    BBH_RX_GENERAL_CONFIGURATION_BBCFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_GENERAL_CONFIGURATION_DISPVIQ, TYPE: Type_BBH_RX_BBHRX_GENERAL_CONFIGURATION_DISPVIQ
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: NORMALVIQ *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_DISPVIQ_NORMALVIQ_FIELD =
{
    "NORMALVIQ",
#if RU_INCLUDE_DESC
    "",
    "Defines the Dispatchers Virtual Ingress Queue for normal packets\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_DISPVIQ_NORMALVIQ_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_DISPVIQ_NORMALVIQ_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_DISPVIQ_NORMALVIQ_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EXCLVIQ *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_DISPVIQ_EXCLVIQ_FIELD =
{
    "EXCLVIQ",
#if RU_INCLUDE_DESC
    "",
    "Defines the Dispatchers Virtual Ingress Queue for exclusive packets\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_DISPVIQ_EXCLVIQ_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_DISPVIQ_EXCLVIQ_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_DISPVIQ_EXCLVIQ_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_DISPVIQ_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_DISPVIQ_NORMALVIQ_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_DISPVIQ_EXCLVIQ_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_GENERAL_CONFIGURATION_DISPVIQ *****/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_DISPVIQ_REG =
{
    "GENERAL_CONFIGURATION_DISPVIQ",
#if RU_INCLUDE_DESC
    "DISPATCHER_FLOW Register",
    "For every reassembled packet in the PSRAM the BBH writes a packet descriptor (PD) into the Dispatcher. The PDs are arranged using a link list in the Dispatcher. The Dispatcher has 32 virtual queues (ingress queues) and the BBH may be assigned to each of the 32 virtual queues of the Dispatcher\nThis register defines virtual queue for normal and exclusive packets.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_DISPVIQ_REG_OFFSET },
    0,
    0,
    23,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_RX_GENERAL_CONFIGURATION_DISPVIQ_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_GENERAL_CONFIGURATION_PATTERNDATALSB, TYPE: Type_BBH_RX_BBHRX_GENERAL_CONFIGURATION_PATTERNDATALSB
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PATTERNDATALSB *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_PATTERNDATALSB_PATTERNDATALSB_FIELD =
{
    "PATTERNDATALSB",
#if RU_INCLUDE_DESC
    "",
    "Pattern Data[31:0]\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_PATTERNDATALSB_PATTERNDATALSB_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_PATTERNDATALSB_PATTERNDATALSB_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_PATTERNDATALSB_PATTERNDATALSB_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_PATTERNDATALSB_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_PATTERNDATALSB_PATTERNDATALSB_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_GENERAL_CONFIGURATION_PATTERNDATALSB *****/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_PATTERNDATALSB_REG =
{
    "GENERAL_CONFIGURATION_PATTERNDATALSB",
#if RU_INCLUDE_DESC
    "PATTERN_RECOGNITION_DATA_LSB Register",
    "The BBH may direct a packet into the Dispatchers exclusive VIQ (Virtual Ingress Queue) according to a match in the pattern recognition.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_PATTERNDATALSB_REG_OFFSET },
    0,
    0,
    24,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_GENERAL_CONFIGURATION_PATTERNDATALSB_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_GENERAL_CONFIGURATION_PATTERNDATAMSB, TYPE: Type_BBH_RX_BBHRX_GENERAL_CONFIGURATION_PATTERNDATAMSB
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PATTERNDATAMSB *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_PATTERNDATAMSB_PATTERNDATAMSB_FIELD =
{
    "PATTERNDATAMSB",
#if RU_INCLUDE_DESC
    "",
    "Pattern Data[63:32]\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_PATTERNDATAMSB_PATTERNDATAMSB_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_PATTERNDATAMSB_PATTERNDATAMSB_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_PATTERNDATAMSB_PATTERNDATAMSB_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_PATTERNDATAMSB_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_PATTERNDATAMSB_PATTERNDATAMSB_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_GENERAL_CONFIGURATION_PATTERNDATAMSB *****/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_PATTERNDATAMSB_REG =
{
    "GENERAL_CONFIGURATION_PATTERNDATAMSB",
#if RU_INCLUDE_DESC
    "PATTERN_RECOGNITION_DATA_MSB Register",
    "The BBH may direct a packet into the Dispatchers exclusive VIQ (Virtual Ingress Queue) according to a match in the pattern recognition.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_PATTERNDATAMSB_REG_OFFSET },
    0,
    0,
    25,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_GENERAL_CONFIGURATION_PATTERNDATAMSB_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_GENERAL_CONFIGURATION_PATTERNMASKLSB, TYPE: Type_BBH_RX_BBHRX_GENERAL_CONFIGURATION_PATTERNMASKLSB
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PATTERNMASKLSB *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_PATTERNMASKLSB_PATTERNMASKLSB_FIELD =
{
    "PATTERNMASKLSB",
#if RU_INCLUDE_DESC
    "",
    "Pattern mask[31:0]\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_PATTERNMASKLSB_PATTERNMASKLSB_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_PATTERNMASKLSB_PATTERNMASKLSB_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_PATTERNMASKLSB_PATTERNMASKLSB_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_PATTERNMASKLSB_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_PATTERNMASKLSB_PATTERNMASKLSB_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_GENERAL_CONFIGURATION_PATTERNMASKLSB *****/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_PATTERNMASKLSB_REG =
{
    "GENERAL_CONFIGURATION_PATTERNMASKLSB",
#if RU_INCLUDE_DESC
    "PATTERN_RECOGNITION_MASK_LSB Register",
    "The BBH may direct a packet into the Dispatchers exclusive VIQ (Virtual Ingress Queue) according to a match in the pattern recognition.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_PATTERNMASKLSB_REG_OFFSET },
    0,
    0,
    26,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_GENERAL_CONFIGURATION_PATTERNMASKLSB_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_GENERAL_CONFIGURATION_PATTERNMASKMSB, TYPE: Type_BBH_RX_BBHRX_GENERAL_CONFIGURATION_PATTERNMASKMSB
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PATTERNMASKMSB *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_PATTERNMASKMSB_PATTERNMASKMSB_FIELD =
{
    "PATTERNMASKMSB",
#if RU_INCLUDE_DESC
    "",
    "Pattern Mask[63:32]\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_PATTERNMASKMSB_PATTERNMASKMSB_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_PATTERNMASKMSB_PATTERNMASKMSB_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_PATTERNMASKMSB_PATTERNMASKMSB_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_PATTERNMASKMSB_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_PATTERNMASKMSB_PATTERNMASKMSB_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_GENERAL_CONFIGURATION_PATTERNMASKMSB *****/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_PATTERNMASKMSB_REG =
{
    "GENERAL_CONFIGURATION_PATTERNMASKMSB",
#if RU_INCLUDE_DESC
    "PATTERN_RECOGNITION_MASK_MSB Register",
    "The BBH may direct a packet into the Dispatchers exclusive VIQ (Virtual Ingress Queue) according to a match in the pattern recognition.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_PATTERNMASKMSB_REG_OFFSET },
    0,
    0,
    27,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_GENERAL_CONFIGURATION_PATTERNMASKMSB_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG, TYPE: Type_BBH_RX_BBHRX_GENERAL_CONFIGURATION_EXCLQCFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PLOAMEN *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_PLOAMEN_FIELD =
{
    "PLOAMEN",
#if RU_INCLUDE_DESC
    "",
    "Direct this packet type to Exclusive VIQ in the Dispatcher\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_PLOAMEN_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_PLOAMEN_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_PLOAMEN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PRI3EN *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_PRI3EN_FIELD =
{
    "PRI3EN",
#if RU_INCLUDE_DESC
    "",
    "Direct this packet type to Exclusive VIQ in the Dispatcher\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_PRI3EN_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_PRI3EN_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_PRI3EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PAUSEEN *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_PAUSEEN_FIELD =
{
    "PAUSEEN",
#if RU_INCLUDE_DESC
    "",
    "Direct this packet type to Exclusive VIQ in the Dispatcher\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_PAUSEEN_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_PAUSEEN_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_PAUSEEN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PFCEN *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_PFCEN_FIELD =
{
    "PFCEN",
#if RU_INCLUDE_DESC
    "",
    "Direct this packet type to Exclusive VIQ in the Dispatcher\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_PFCEN_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_PFCEN_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_PFCEN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CTRLEN *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_CTRLEN_FIELD =
{
    "CTRLEN",
#if RU_INCLUDE_DESC
    "",
    "Direct this packet type to Exclusive VIQ in the Dispatcher\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_CTRLEN_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_CTRLEN_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_CTRLEN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MULTEN *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_MULTEN_FIELD =
{
    "MULTEN",
#if RU_INCLUDE_DESC
    "",
    "Direct this packet type to Exclusive VIQ in the Dispatcher\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_MULTEN_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_MULTEN_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_MULTEN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: OAMEN *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_OAMEN_FIELD =
{
    "OAMEN",
#if RU_INCLUDE_DESC
    "",
    "Direct this packet type to Exclusive VIQ in the Dispatcher\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_OAMEN_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_OAMEN_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_OAMEN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PATTENOFFSET *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_PATTENOFFSET_FIELD =
{
    "PATTENOFFSET",
#if RU_INCLUDE_DESC
    "",
    "Defines the pattern recognition offset within the packet. Offset is 8 bytes resolution\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_PATTENOFFSET_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_PATTENOFFSET_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_PATTENOFFSET_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PATTERNEN *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_PATTERNEN_FIELD =
{
    "PATTERNEN",
#if RU_INCLUDE_DESC
    "",
    "Must be enabled if pattern recognition is used\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_PATTERNEN_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_PATTERNEN_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_PATTERNEN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EXCEN *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_EXCEN_FIELD =
{
    "EXCEN",
#if RU_INCLUDE_DESC
    "",
    "Must be enabled if Exclusive VIQ is used\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_EXCEN_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_EXCEN_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_EXCEN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DISNORMALCHECK *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_DISNORMALCHECK_FIELD =
{
    "DISNORMALCHECK",
#if RU_INCLUDE_DESC
    "",
    "If asserted, Exclusive queue is determined only according to the information from the MAC (EPON OAM or GPON PLOAM/OMCI)\nNo support for pattern match or multicast identification in this mode\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_DISNORMALCHECK_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_DISNORMALCHECK_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_DISNORMALCHECK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_PLOAMEN_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_PRI3EN_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_PAUSEEN_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_PFCEN_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_CTRLEN_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_MULTEN_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_OAMEN_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_PATTENOFFSET_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_PATTERNEN_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_EXCEN_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_DISNORMALCHECK_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG *****/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_REG =
{
    "GENERAL_CONFIGURATION_EXCLQCFG",
#if RU_INCLUDE_DESC
    "EXCLUSIVE_QUEUE_CFG Register",
    "The BBH may direct a packet into the Dispatchers exclusive VIQ (Virtual Ingress Queue) according to special packet types (e.g. pause).\nThis register enables this function\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_REG_OFFSET },
    0,
    0,
    28,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    11,
    BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_GENERAL_CONFIGURATION_SDMAADDR, TYPE: Type_BBH_RX_BBHRX_GENERAL_CONFIGURATION_SDMAADDR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATABASE *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_SDMAADDR_DATABASE_FIELD =
{
    "DATABASE",
#if RU_INCLUDE_DESC
    "",
    "The Data FIFO base address within the SDMA address space.\nThe address is in chunk resolution (128 bytes).\nThe value should be identical to the relevant configuration in the SDMA.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_SDMAADDR_DATABASE_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_SDMAADDR_DATABASE_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_SDMAADDR_DATABASE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DESCBASE *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_SDMAADDR_DESCBASE_FIELD =
{
    "DESCBASE",
#if RU_INCLUDE_DESC
    "",
    "The Descriptor FIFO base address within the SDMA address space.\nThe address is in chunk descriptor resolution (8 bytes).\nThe value  should be identical to the relevant configuration in the SDMA.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_SDMAADDR_DESCBASE_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_SDMAADDR_DESCBASE_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_SDMAADDR_DESCBASE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_SDMAADDR_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_SDMAADDR_DATABASE_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_SDMAADDR_DESCBASE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_GENERAL_CONFIGURATION_SDMAADDR *****/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_SDMAADDR_REG =
{
    "GENERAL_CONFIGURATION_SDMAADDR",
#if RU_INCLUDE_DESC
    "SDMA_ADDRESS_CONFIGURATION Register",
    "The BBH reassembles the incoming data in the SRAM. The Data is written into the SRAM using the SDMA. The data is organized in a configurable number of chunks of 128 bytes.\nThe BBH arranges the written data in the SDMA in these chunks. It arranges the data in a predefined address space in the SDMA memory and manages the chunks in a cyclic FIFO style.\nFor every write chunk the BBH writes a write descriptor. The write descriptors are arranged in a predefined space in the SDMA memory and managed in a cyclic FIFO style as well.\nThis register defines the Data and descriptors base addresses.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_SDMAADDR_REG_OFFSET },
    0,
    0,
    29,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_RX_GENERAL_CONFIGURATION_SDMAADDR_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_GENERAL_CONFIGURATION_SDMACFG, TYPE: Type_BBH_RX_BBHRX_GENERAL_CONFIGURATION_SDMACFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: NUMOFCD *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_SDMACFG_NUMOFCD_FIELD =
{
    "NUMOFCD",
#if RU_INCLUDE_DESC
    "",
    "Defines the size of the Chunk descripors FIFO in the DMA.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_SDMACFG_NUMOFCD_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_SDMACFG_NUMOFCD_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_SDMACFG_NUMOFCD_FIELD_SHIFT },
    16,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EXCLTH *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_SDMACFG_EXCLTH_FIELD =
{
    "EXCLTH",
#if RU_INCLUDE_DESC
    "",
    "This field defines the number of occupied write chunks for dropping normal or high priority packets.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_SDMACFG_EXCLTH_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_SDMACFG_EXCLTH_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_SDMACFG_EXCLTH_FIELD_SHIFT },
    8,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: COHERENCYEN *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_SDMACFG_COHERENCYEN_FIELD =
{
    "COHERENCYEN",
#if RU_INCLUDE_DESC
    "",
    "BBH has two methods to keep coherency:\n1. Write reply for last chunk only\n2. Write reply for each chunk\n\n1 - enables the first method\n0 - enables the second method\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_SDMACFG_COHERENCYEN_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_SDMACFG_COHERENCYEN_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_SDMACFG_COHERENCYEN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_SDMACFG_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_SDMACFG_NUMOFCD_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_SDMACFG_EXCLTH_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_SDMACFG_COHERENCYEN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_GENERAL_CONFIGURATION_SDMACFG *****/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_SDMACFG_REG =
{
    "GENERAL_CONFIGURATION_SDMACFG",
#if RU_INCLUDE_DESC
    "SDMA_CONFIGURATION Register",
    "The BBH reassembles the incoming data in the SRAM. The Data is written into the SRAM using the SDMA. The data is organized in a configurable number of chunks of 128 bytes.\nThe BBH arranges the written data in the SDMA in these chunks. It arranges the data in a predefined address space in the SDMA memory and manages the chunks in a cyclic FIFO style.\nFor every write chunk the BBH writes a write descriptor. The write descriptors are arranged in a predefined space in the SDMA memory and managed in a cyclic FIFO style as well.\n\nThe BBH handles the congestion over the SDMA write chunks according to 2 priorities (low + high, exclusive). This field defines the number of occupied write chunks for dropping normal or high priority packets. If the number of occupied chunk is lower than this threshold, then all packets are passed. If the number of occupied chunk is equal or higher than this threshold, then only exclusive priority packets are passed.\n\nThis register defines the Data and descriptors FIFO sizes and the exclusive threshold.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_SDMACFG_REG_OFFSET },
    0,
    0,
    30,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    BBH_RX_GENERAL_CONFIGURATION_SDMACFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_GENERAL_CONFIGURATION_MINPKT0, TYPE: Type_BBH_RX_BBHRX_GENERAL_CONFIGURATION_MINPKT0
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: MINPKT0 *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_MINPKT0_MINPKT0_FIELD =
{
    "MINPKT0",
#if RU_INCLUDE_DESC
    "",
    "Packets shorter than this threshold will be discarded.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_MINPKT0_MINPKT0_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_MINPKT0_MINPKT0_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_MINPKT0_MINPKT0_FIELD_SHIFT },
    16,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MINPKT1 *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_MINPKT0_MINPKT1_FIELD =
{
    "MINPKT1",
#if RU_INCLUDE_DESC
    "",
    "Packets shorter than this threshold will be discarded.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_MINPKT0_MINPKT1_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_MINPKT0_MINPKT1_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_MINPKT0_MINPKT1_FIELD_SHIFT },
    16,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MINPKT2 *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_MINPKT0_MINPKT2_FIELD =
{
    "MINPKT2",
#if RU_INCLUDE_DESC
    "",
    "Packets shorter than this threshold will be discarded.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_MINPKT0_MINPKT2_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_MINPKT0_MINPKT2_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_MINPKT0_MINPKT2_FIELD_SHIFT },
    16,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MINPKT3 *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_MINPKT0_MINPKT3_FIELD =
{
    "MINPKT3",
#if RU_INCLUDE_DESC
    "",
    "Packets shorter than this threshold will be discarded.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_MINPKT0_MINPKT3_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_MINPKT0_MINPKT3_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_MINPKT0_MINPKT3_FIELD_SHIFT },
    16,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_MINPKT0_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_MINPKT0_MINPKT0_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_MINPKT0_MINPKT1_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_MINPKT0_MINPKT2_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_MINPKT0_MINPKT3_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_GENERAL_CONFIGURATION_MINPKT0 *****/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_MINPKT0_REG =
{
    "GENERAL_CONFIGURATION_MINPKT0",
#if RU_INCLUDE_DESC
    "MINIMUM_PACKET_SIZE Register",
    "There are 4 global configuration for Minimum packet size. Each flow can get one out of these 4 global configurations.\nPackets shorter than this threshold will be discarded.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_MINPKT0_REG_OFFSET },
    0,
    0,
    31,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    BBH_RX_GENERAL_CONFIGURATION_MINPKT0_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_GENERAL_CONFIGURATION_MAXPKT0, TYPE: Type_BBH_RX_BBHRX_GENERAL_CONFIGURATION_MAXPKT0
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: MAXPKT0 *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_MAXPKT0_MAXPKT0_FIELD =
{
    "MAXPKT0",
#if RU_INCLUDE_DESC
    "",
    "Packets longer than this threshold will be discarded.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_MAXPKT0_MAXPKT0_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_MAXPKT0_MAXPKT0_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_MAXPKT0_MAXPKT0_FIELD_SHIFT },
    16383,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MAXPKT1 *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_MAXPKT0_MAXPKT1_FIELD =
{
    "MAXPKT1",
#if RU_INCLUDE_DESC
    "",
    "Packets longer than this threshold will be discarded.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_MAXPKT0_MAXPKT1_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_MAXPKT0_MAXPKT1_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_MAXPKT0_MAXPKT1_FIELD_SHIFT },
    16383,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_MAXPKT0_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_MAXPKT0_MAXPKT0_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_MAXPKT0_MAXPKT1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_GENERAL_CONFIGURATION_MAXPKT0 *****/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_MAXPKT0_REG =
{
    "GENERAL_CONFIGURATION_MAXPKT0",
#if RU_INCLUDE_DESC
    "MAXIMUM_PACKET_SIZE_0 Register",
    "There are 4 global configuration for Maximum packet size. Each flow can get one out of these 4 global configurations.\nPackets longer than this threshold will be discarded.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_MAXPKT0_REG_OFFSET },
    0,
    0,
    32,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_RX_GENERAL_CONFIGURATION_MAXPKT0_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_GENERAL_CONFIGURATION_MAXPKT1, TYPE: Type_BBH_RX_BBHRX_GENERAL_CONFIGURATION_MAXPKT1
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: MAXPKT2 *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_MAXPKT1_MAXPKT2_FIELD =
{
    "MAXPKT2",
#if RU_INCLUDE_DESC
    "",
    "Packets longer than this threshold will be discarded.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_MAXPKT1_MAXPKT2_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_MAXPKT1_MAXPKT2_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_MAXPKT1_MAXPKT2_FIELD_SHIFT },
    16383,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MAXPKT3 *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_MAXPKT1_MAXPKT3_FIELD =
{
    "MAXPKT3",
#if RU_INCLUDE_DESC
    "",
    "Packets longer than this threshold will be discarded.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_MAXPKT1_MAXPKT3_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_MAXPKT1_MAXPKT3_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_MAXPKT1_MAXPKT3_FIELD_SHIFT },
    16383,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_MAXPKT1_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_MAXPKT1_MAXPKT2_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_MAXPKT1_MAXPKT3_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_GENERAL_CONFIGURATION_MAXPKT1 *****/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_MAXPKT1_REG =
{
    "GENERAL_CONFIGURATION_MAXPKT1",
#if RU_INCLUDE_DESC
    "MAXIMUM_PACKET_SIZE_1 Register",
    "There are 4 global configuration for Maximum packet size. Each flow can get one out of these 4 global configurations.\nPackets longer than this threshold will be discarded.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_MAXPKT1_REG_OFFSET },
    0,
    0,
    33,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_RX_GENERAL_CONFIGURATION_MAXPKT1_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_GENERAL_CONFIGURATION_SOPOFFSET, TYPE: Type_BBH_RX_BBHRX_GENERAL_CONFIGURATION_SOPOFFSET
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: SOPOFFSET *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_SOPOFFSET_SOPOFFSET_FIELD =
{
    "SOPOFFSET",
#if RU_INCLUDE_DESC
    "",
    "The SOP offset in bytes.\nAllowed values: 0-127.\nThis value should match the relevant configuration in the Runner block.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_SOPOFFSET_SOPOFFSET_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_SOPOFFSET_SOPOFFSET_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_SOPOFFSET_SOPOFFSET_FIELD_SHIFT },
    18,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_SOPOFFSET_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_SOPOFFSET_SOPOFFSET_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_GENERAL_CONFIGURATION_SOPOFFSET *****/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_SOPOFFSET_REG =
{
    "GENERAL_CONFIGURATION_SOPOFFSET",
#if RU_INCLUDE_DESC
    "SOP_OFFSET Register",
    "The BBH writes the packets into the PSRAM. The start of data offset is configurable. This register defines the SOP (start of packet) offset.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_SOPOFFSET_REG_OFFSET },
    0,
    0,
    34,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_GENERAL_CONFIGURATION_SOPOFFSET_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL, TYPE: Type_BBH_RX_BBHRX_GENERAL_CONFIGURATION_FLOWCTRL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: TIMER *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_TIMER_FIELD =
{
    "TIMER",
#if RU_INCLUDE_DESC
    "",
    "Timer value before de-asserting the flow control indication.\nThe duration of the time is determined according to the BBH clock frequency.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_TIMER_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_TIMER_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_TIMER_FIELD_SHIFT },
    4096,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DISPDROPDIS *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_DISPDROPDIS_FIELD =
{
    "DISPDROPDIS",
#if RU_INCLUDE_DESC
    "",
    "Disable dropping packets due to no space in the Dispatcher.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_DISPDROPDIS_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_DISPDROPDIS_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_DISPDROPDIS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SDMADROPDIS *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_SDMADROPDIS_FIELD =
{
    "SDMADROPDIS",
#if RU_INCLUDE_DESC
    "",
    "Disable dropping packets due to no space in the SDMA.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_SDMADROPDIS_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_SDMADROPDIS_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_SDMADROPDIS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SBPMDROPDIS *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_SBPMDROPDIS_FIELD =
{
    "SBPMDROPDIS",
#if RU_INCLUDE_DESC
    "",
    "Disable dropping packets due to no space in the SBPM.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_SBPMDROPDIS_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_SBPMDROPDIS_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_SBPMDROPDIS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FCFORCE *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_FCFORCE_FIELD =
{
    "FCFORCE",
#if RU_INCLUDE_DESC
    "",
    "Asserting this bit will force a flow control indication towards the MAC\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_FCFORCE_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_FCFORCE_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_FCFORCE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FCRNREN *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_FCRNREN_FIELD =
{
    "FCRNREN",
#if RU_INCLUDE_DESC
    "",
    "Enables Runner to send flow control messages\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_FCRNREN_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_FCRNREN_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_FCRNREN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FCQMEN *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_FCQMEN_FIELD =
{
    "FCQMEN",
#if RU_INCLUDE_DESC
    "",
    "Enables QM to send flow control messages\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_FCQMEN_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_FCQMEN_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_FCQMEN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_TIMER_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_DISPDROPDIS_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_SDMADROPDIS_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_SBPMDROPDIS_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_FCFORCE_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_FCRNREN_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_FCQMEN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL *****/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_REG =
{
    "GENERAL_CONFIGURATION_FLOWCTRL",
#if RU_INCLUDE_DESC
    "FLOW_CONTROL_CONFIGURATION Register",
    "The BBH RX may assert the flow control indication towards the Ethernet MAC, signaling that the system is congested. As long as this signal is asserted, the MAC will keep the link-partner in Xoff state.\nBoth Runner and QM may send a BB message to control the flow control. A static configuration determines if the flow control is managed by FW, QM or both.\nFW message triggers a timer. When the timer expires, the BBH will de-assert the flow control indication.\nQM may send Xoff and Xon messages.\nThis register also disable BBH packet drop due to no space in the SDMA, SBPM or Dispatcher.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_REG_OFFSET },
    0,
    0,
    35,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    7,
    BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_GENERAL_CONFIGURATION_CRCOMITDIS, TYPE: Type_BBH_RX_BBHRX_GENERAL_CONFIGURATION_CRCOMITDIS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CRCOMITDIS *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_CRCOMITDIS_CRCOMITDIS_FIELD =
{
    "CRCOMITDIS",
#if RU_INCLUDE_DESC
    "",
    "Disable CRC omitting.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_CRCOMITDIS_CRCOMITDIS_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_CRCOMITDIS_CRCOMITDIS_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_CRCOMITDIS_CRCOMITDIS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_CRCOMITDIS_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_CRCOMITDIS_CRCOMITDIS_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_GENERAL_CONFIGURATION_CRCOMITDIS *****/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_CRCOMITDIS_REG =
{
    "GENERAL_CONFIGURATION_CRCOMITDIS",
#if RU_INCLUDE_DESC
    "CRC_OMIT_DISABLE Register",
    "The BBH omits the 4 CRC bytes of the packet for all packets except PLOAMs and OMCI (marked as exclusive priority).\nThe configuration will disable this functionality.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_CRCOMITDIS_REG_OFFSET },
    0,
    0,
    36,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_GENERAL_CONFIGURATION_CRCOMITDIS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_GENERAL_CONFIGURATION_ENABLE, TYPE: Type_BBH_RX_BBHRX_GENERAL_CONFIGURATION_ENABLE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PKTEN *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_ENABLE_PKTEN_FIELD =
{
    "PKTEN",
#if RU_INCLUDE_DESC
    "",
    "When de-asserted, the BBH will not read new fragment/packet from the MAC.\nThe BBH will Gracefully enable/disable (on fragment boundary for N/X/GPON/2 and on packet boundary for the rest)\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_ENABLE_PKTEN_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_ENABLE_PKTEN_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_ENABLE_PKTEN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SBPMEN *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_ENABLE_SBPMEN_FIELD =
{
    "SBPMEN",
#if RU_INCLUDE_DESC
    "",
    "When de-asserted, the BBH will not pre-fetch SBPM buffers\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_ENABLE_SBPMEN_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_ENABLE_SBPMEN_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_ENABLE_SBPMEN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_ENABLE_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_ENABLE_PKTEN_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_ENABLE_SBPMEN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_GENERAL_CONFIGURATION_ENABLE *****/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_ENABLE_REG =
{
    "GENERAL_CONFIGURATION_ENABLE",
#if RU_INCLUDE_DESC
    "BBH_ENABLE Register",
    "Controls the BBH enable configuration\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_ENABLE_REG_OFFSET },
    0,
    0,
    37,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_RX_GENERAL_CONFIGURATION_ENABLE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_GENERAL_CONFIGURATION_G9991EN, TYPE: Type_BBH_RX_BBHRX_GENERAL_CONFIGURATION_G9991EN
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ENABLE *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_G9991EN_ENABLE_FIELD =
{
    "ENABLE",
#if RU_INCLUDE_DESC
    "",
    "Enable G999.1\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_G9991EN_ENABLE_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_G9991EN_ENABLE_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_G9991EN_ENABLE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BYTES4_7ENABLE *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_G9991EN_BYTES4_7ENABLE_FIELD =
{
    "BYTES4_7ENABLE",
#if RU_INCLUDE_DESC
    "",
    "Enable G999.1 transfer of bytes 4-7 instead of bytes 0-3\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_G9991EN_BYTES4_7ENABLE_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_G9991EN_BYTES4_7ENABLE_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_G9991EN_BYTES4_7ENABLE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_G9991EN_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_G9991EN_ENABLE_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_G9991EN_BYTES4_7ENABLE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_GENERAL_CONFIGURATION_G9991EN *****/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_G9991EN_REG =
{
    "GENERAL_CONFIGURATION_G9991EN",
#if RU_INCLUDE_DESC
    "G999_1_ENABLE Register",
    "When asserted, G999.1 fragments are received by the BBH.\nThe BBH will pass the G999.1 header in the PD instead of the 1588 time-stamp.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_G9991EN_REG_OFFSET },
    0,
    0,
    38,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_RX_GENERAL_CONFIGURATION_G9991EN_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_GENERAL_CONFIGURATION_PERFLOWTH, TYPE: Type_BBH_RX_BBHRX_GENERAL_CONFIGURATION_PERFLOWTH
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: FLOWTH *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_PERFLOWTH_FLOWTH_FIELD =
{
    "FLOWTH",
#if RU_INCLUDE_DESC
    "",
    "According to this threshold:\nFlows 32 - th will have set 0 configurations.\nFlows (th+1) - 255 will have set 1 configurations.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_PERFLOWTH_FLOWTH_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_PERFLOWTH_FLOWTH_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_PERFLOWTH_FLOWTH_FIELD_SHIFT },
    32,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_PERFLOWTH_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_PERFLOWTH_FLOWTH_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_GENERAL_CONFIGURATION_PERFLOWTH *****/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_PERFLOWTH_REG =
{
    "GENERAL_CONFIGURATION_PERFLOWTH",
#if RU_INCLUDE_DESC
    "PER_FLOW_THRESHOLD Register",
    "The DS has 256 flows. Minimum packet size (2 bits) and Maximum packet size (2 bits) are configured per flow.\nFlows 0-31 will have full configurations. Flows 32-X and flows (X+1)-255 will have global set of configurations.\nX is configurable.\nThis register defines X.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_PERFLOWTH_REG_OFFSET },
    0,
    0,
    39,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_GENERAL_CONFIGURATION_PERFLOWTH_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_GENERAL_CONFIGURATION_PERFLOWSETS, TYPE: Type_BBH_RX_BBHRX_GENERAL_CONFIGURATION_PERFLOWSETS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: MINPKTSEL0 *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_PERFLOWSETS_MINPKTSEL0_FIELD =
{
    "MINPKTSEL0",
#if RU_INCLUDE_DESC
    "",
    "Set 0 of the general configuration.\nSelects between 4 global minimum packet size.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_PERFLOWSETS_MINPKTSEL0_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_PERFLOWSETS_MINPKTSEL0_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_PERFLOWSETS_MINPKTSEL0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MAXPKTSEL0 *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_PERFLOWSETS_MAXPKTSEL0_FIELD =
{
    "MAXPKTSEL0",
#if RU_INCLUDE_DESC
    "",
    "Set 0 of the general configuration.\nSelects between 4 global maximum packet size.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_PERFLOWSETS_MAXPKTSEL0_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_PERFLOWSETS_MAXPKTSEL0_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_PERFLOWSETS_MAXPKTSEL0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MINPKTSEL1 *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_PERFLOWSETS_MINPKTSEL1_FIELD =
{
    "MINPKTSEL1",
#if RU_INCLUDE_DESC
    "",
    "Set 1 of the general configuration.\nSelects between 4 global minimum packet size.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_PERFLOWSETS_MINPKTSEL1_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_PERFLOWSETS_MINPKTSEL1_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_PERFLOWSETS_MINPKTSEL1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MAXPKTSEL1 *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_PERFLOWSETS_MAXPKTSEL1_FIELD =
{
    "MAXPKTSEL1",
#if RU_INCLUDE_DESC
    "",
    "Set 1 of the general configuration.\nSelects between 4 global maximum packet size.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_PERFLOWSETS_MAXPKTSEL1_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_PERFLOWSETS_MAXPKTSEL1_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_PERFLOWSETS_MAXPKTSEL1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_PERFLOWSETS_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_PERFLOWSETS_MINPKTSEL0_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_PERFLOWSETS_MAXPKTSEL0_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_PERFLOWSETS_MINPKTSEL1_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_PERFLOWSETS_MAXPKTSEL1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_GENERAL_CONFIGURATION_PERFLOWSETS *****/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_PERFLOWSETS_REG =
{
    "GENERAL_CONFIGURATION_PERFLOWSETS",
#if RU_INCLUDE_DESC
    "PER_FLOW_SETS Register",
    "The DS has 256 flows. Minimum packet size (2 bits) and Maximum packet size (2 bits) are configured per flow.\nFlows 0-31 will have full configurations. Flows 32-X and flows (X+1)-255 will have global set of configurations.\nX is configurable.\nThis register defines the configurations sets.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_PERFLOWSETS_REG_OFFSET },
    0,
    0,
    40,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    BBH_RX_GENERAL_CONFIGURATION_PERFLOWSETS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_GENERAL_CONFIGURATION_MINPKTSEL0, TYPE: Type_BBH_RX_BBHRX_GENERAL_CONFIGURATION_MINPKTSEL0
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: MINPKTSEL *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_MINPKTSEL0_MINPKTSEL_FIELD =
{
    "MINPKTSEL",
#if RU_INCLUDE_DESC
    "",
    "Selects one of the 4 global configurations for minimum packet size.\nBits {2n, 2n+1} refers to flow n.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_MINPKTSEL0_MINPKTSEL_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_MINPKTSEL0_MINPKTSEL_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_MINPKTSEL0_MINPKTSEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_MINPKTSEL0_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_MINPKTSEL0_MINPKTSEL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_GENERAL_CONFIGURATION_MINPKTSEL0 *****/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_MINPKTSEL0_REG =
{
    "GENERAL_CONFIGURATION_MINPKTSEL0",
#if RU_INCLUDE_DESC
    "MINIMUM_PACKET_SELECT_0 Register",
    "The DS has 256 flows. Minimum packet size (2 bits) and Maximum packet size (2 bits) are configured per flow.\nFlows 0-31 will have full configurations. Flows 32-X and flows (X+1)-255 will have global set of configurations.\nX is configurable.\nThis register defines the minimum packet size for flows 0-15.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_MINPKTSEL0_REG_OFFSET },
    0,
    0,
    41,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_GENERAL_CONFIGURATION_MINPKTSEL0_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_GENERAL_CONFIGURATION_MINPKTSEL1, TYPE: Type_BBH_RX_BBHRX_GENERAL_CONFIGURATION_MINPKTSEL1
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: MINPKTSEL *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_MINPKTSEL1_MINPKTSEL_FIELD =
{
    "MINPKTSEL",
#if RU_INCLUDE_DESC
    "",
    "Selects one of the 4 global configurations for minimum packet size.\nBits {2n, 2n+1} refers to flow n+16.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_MINPKTSEL1_MINPKTSEL_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_MINPKTSEL1_MINPKTSEL_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_MINPKTSEL1_MINPKTSEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_MINPKTSEL1_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_MINPKTSEL1_MINPKTSEL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_GENERAL_CONFIGURATION_MINPKTSEL1 *****/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_MINPKTSEL1_REG =
{
    "GENERAL_CONFIGURATION_MINPKTSEL1",
#if RU_INCLUDE_DESC
    "MINIMUM_PACKET_SELECT_1 Register",
    "The DS has 256 flows. Minimum packet size (2 bits) and Maximum packet size (2 bits) are configured per flow.\nFlows 0-31 will have full configurations. Flows 32-X and flows (X+1)-255 will have global set of configurations.\nX is configurable.\nThis register defines the minimum packet size for flows 16-31.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_MINPKTSEL1_REG_OFFSET },
    0,
    0,
    42,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_GENERAL_CONFIGURATION_MINPKTSEL1_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_GENERAL_CONFIGURATION_MAXPKTSEL0, TYPE: Type_BBH_RX_BBHRX_GENERAL_CONFIGURATION_MAXPKTSEL0
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: MAXPKTSEL *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_MAXPKTSEL0_MAXPKTSEL_FIELD =
{
    "MAXPKTSEL",
#if RU_INCLUDE_DESC
    "",
    "Selects one of the 4 global configurations for maximum packet size.\nBits {2n, 2n+1} refers to flow n.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_MAXPKTSEL0_MAXPKTSEL_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_MAXPKTSEL0_MAXPKTSEL_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_MAXPKTSEL0_MAXPKTSEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_MAXPKTSEL0_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_MAXPKTSEL0_MAXPKTSEL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_GENERAL_CONFIGURATION_MAXPKTSEL0 *****/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_MAXPKTSEL0_REG =
{
    "GENERAL_CONFIGURATION_MAXPKTSEL0",
#if RU_INCLUDE_DESC
    "MAXIMUM_PACKET_SELECT_0 Register",
    "The DS has 256 flows. Minimum packet size (2 bits) and Maximum packet size (2 bits) are configured per flow.\nFlows 0-31 will have full configurations. Flows 32-X and flows (X+1)-255 will have global set of configurations.\nX is configurable.\nThis register defines the maximum packet size for flows 0-15.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_MAXPKTSEL0_REG_OFFSET },
    0,
    0,
    43,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_GENERAL_CONFIGURATION_MAXPKTSEL0_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_GENERAL_CONFIGURATION_MAXPKTSEL1, TYPE: Type_BBH_RX_BBHRX_GENERAL_CONFIGURATION_MAXPKTSEL1
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: MAXPKTSEL *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_MAXPKTSEL1_MAXPKTSEL_FIELD =
{
    "MAXPKTSEL",
#if RU_INCLUDE_DESC
    "",
    "Selects one of the 4 global configurations for maximum packet size.\nBits {2n, 2n+1} refers to flow n+16.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_MAXPKTSEL1_MAXPKTSEL_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_MAXPKTSEL1_MAXPKTSEL_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_MAXPKTSEL1_MAXPKTSEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_MAXPKTSEL1_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_MAXPKTSEL1_MAXPKTSEL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_GENERAL_CONFIGURATION_MAXPKTSEL1 *****/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_MAXPKTSEL1_REG =
{
    "GENERAL_CONFIGURATION_MAXPKTSEL1",
#if RU_INCLUDE_DESC
    "MAXIMUM_PACKET_SELECT_1 Register",
    "The DS has 256 flows. Minimum packet size (2 bits) and Maximum packet size (2 bits) are configured per flow.\nFlows 0-31 will have full configurations. Flows 32-X and flows (X+1)-255 will have global set of configurations.\nX is configurable.\nThis register defines the maximum packet size for flows 16-31.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_MAXPKTSEL1_REG_OFFSET },
    0,
    0,
    44,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_GENERAL_CONFIGURATION_MAXPKTSEL1_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_GENERAL_CONFIGURATION_MACMODE, TYPE: Type_BBH_RX_BBHRX_GENERAL_CONFIGURATION_MACMODE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: MACMODE *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_MACMODE_MACMODE_FIELD =
{
    "MACMODE",
#if RU_INCLUDE_DESC
    "",
    "Relevant for PON BBH only.\nDistinguish between GPON (GPON, XGPON, NGPON2) to EPON (EPON, 10GEPON):\n0: N/X/GPON/2\n1: 10G/EPON\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_MACMODE_MACMODE_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_MACMODE_MACMODE_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_MACMODE_MACMODE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: GPONMODE *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_MACMODE_GPONMODE_FIELD =
{
    "GPONMODE",
#if RU_INCLUDE_DESC
    "",
    "Relevant for GPON BBH only.\nDistinguish between GPON and XGPON (XGPON, NGPON2):\n0: GPON\n1: N/X/GPON/2\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_MACMODE_GPONMODE_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_MACMODE_GPONMODE_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_MACMODE_GPONMODE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MACVDSL *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_MACMODE_MACVDSL_FIELD =
{
    "MACVDSL",
#if RU_INCLUDE_DESC
    "",
    "Relevant for VDSL BBH only.\nDistinguish between VDSL and non VDSL:\n0: Non VDSL\n1: VDSL\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_MACMODE_MACVDSL_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_MACMODE_MACVDSL_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_MACMODE_MACVDSL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MACISWANEN *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_MACMODE_MACISWANEN_FIELD =
{
    "MACISWANEN",
#if RU_INCLUDE_DESC
    "",
    "This configuration enables a global configuration per BBH to determine the WAN/LAN bit in the PD according to Mac_is_WAN.\nIf disabled (default), The WAN/LAN bit in the PD is set to 1 (LAN) for Ethernet ports and to 0 (WAN) for non-Ethernet ports.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_MACMODE_MACISWANEN_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_MACMODE_MACISWANEN_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_MACMODE_MACISWANEN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MACISWAN *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_MACMODE_MACISWAN_FIELD =
{
    "MACISWAN",
#if RU_INCLUDE_DESC
    "",
    "Determine the WAN/LAN bit in the PD.\nIf the feature is enabled and the value is asserted (==1) => WAN_LAN == 0\nEnabled by Mac_is_WAN_EN.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_MACMODE_MACISWAN_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_MACMODE_MACISWAN_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_MACMODE_MACISWAN_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_MACMODE_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_MACMODE_MACMODE_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_MACMODE_GPONMODE_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_MACMODE_MACVDSL_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_MACMODE_MACISWANEN_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_MACMODE_MACISWAN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_GENERAL_CONFIGURATION_MACMODE *****/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_MACMODE_REG =
{
    "GENERAL_CONFIGURATION_MACMODE",
#if RU_INCLUDE_DESC
    "MAC_MODE Register",
    "When the BBH functions as a PON BBH, this bit selects between N/X/GPON/2 and 10G/EPON functionality\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_MACMODE_REG_OFFSET },
    0,
    0,
    45,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    BBH_RX_GENERAL_CONFIGURATION_MACMODE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_GENERAL_CONFIGURATION_SBPMCFG, TYPE: Type_BBH_RX_BBHRX_GENERAL_CONFIGURATION_SBPMCFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: MAXREQ *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_SBPMCFG_MAXREQ_FIELD =
{
    "MAXREQ",
#if RU_INCLUDE_DESC
    "",
    "Configure max on the fly requests to SBPM\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_SBPMCFG_MAXREQ_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_SBPMCFG_MAXREQ_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_SBPMCFG_MAXREQ_FIELD_SHIFT },
    15,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PRIDROPEN *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_SBPMCFG_PRIDROPEN_FIELD =
{
    "PRIDROPEN",
#if RU_INCLUDE_DESC
    "",
    "This feature enables SBPM drop according to priority.\n2 bits according to SBPM congestion message: EXC low and EXC high\n1 configuration bit in BBH defines which of the SBPM indications are used (CNGSEL). 0 = SBPM EXC low; 1 = SBPM EXC high\n\nThe following is added to the drop condition:\nSBPM_congestion = CNGSEL ? SBPM EXC high : SBPM EXC low\ndrop if (packet priority != (GPON PLOAM or GPON EXC or EPON OAM) and SBPM_congestion)\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_SBPMCFG_PRIDROPEN_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_SBPMCFG_PRIDROPEN_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_SBPMCFG_PRIDROPEN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CNGSEL *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_SBPMCFG_CNGSEL_FIELD =
{
    "CNGSEL",
#if RU_INCLUDE_DESC
    "",
    "Defines which of the SBPM indications are used. 0 = SBPM EXC low; 1 = SBPM EXC high\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_SBPMCFG_CNGSEL_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_SBPMCFG_CNGSEL_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_SBPMCFG_CNGSEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_SBPMCFG_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_SBPMCFG_MAXREQ_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_SBPMCFG_PRIDROPEN_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_SBPMCFG_CNGSEL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_GENERAL_CONFIGURATION_SBPMCFG *****/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_SBPMCFG_REG =
{
    "GENERAL_CONFIGURATION_SBPMCFG",
#if RU_INCLUDE_DESC
    "SBPM_CFG Register",
    "Configure max on the fly requests to SBPM\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_SBPMCFG_REG_OFFSET },
    0,
    0,
    46,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    BBH_RX_GENERAL_CONFIGURATION_SBPMCFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_GENERAL_CONFIGURATION_RXRSTRST, TYPE: Type_BBH_RX_BBHRX_GENERAL_CONFIGURATION_RXRSTRST
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: INBUFRST *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_INBUFRST_FIELD =
{
    "INBUFRST",
#if RU_INCLUDE_DESC
    "",
    "Writing 1 to this register will reset the input buffer.\nFor a reset operation the SW should assert and then de-assert this bit.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_INBUFRST_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_INBUFRST_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_INBUFRST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BURSTBUFRST *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_BURSTBUFRST_FIELD =
{
    "BURSTBUFRST",
#if RU_INCLUDE_DESC
    "",
    "Writing 1 to this register will reset the Burst buffer.\nFor a reset operation the SW should assert and then de-assert this bit.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_BURSTBUFRST_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_BURSTBUFRST_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_BURSTBUFRST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: INGRESSCNTXT *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_INGRESSCNTXT_FIELD =
{
    "INGRESSCNTXT",
#if RU_INCLUDE_DESC
    "",
    "Writing 1 to this register will reset the ingress context.\nFor a reset operation the SW should assert and then de-assert this bit.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_INGRESSCNTXT_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_INGRESSCNTXT_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_INGRESSCNTXT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CMDFIFORST *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_CMDFIFORST_FIELD =
{
    "CMDFIFORST",
#if RU_INCLUDE_DESC
    "",
    "Writing 1 to this register will reset the IH buffer enable.\nFor a reset operation the SW should assert and then de-assert this bit.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_CMDFIFORST_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_CMDFIFORST_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_CMDFIFORST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SBPMFIFORST *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_SBPMFIFORST_FIELD =
{
    "SBPMFIFORST",
#if RU_INCLUDE_DESC
    "",
    "Writing 1 to this register will reset the SBPM FIFO.\nThe reset is done immediately. Reading this register will always return 0.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_SBPMFIFORST_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_SBPMFIFORST_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_SBPMFIFORST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: COHERENCYFIFORST *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_COHERENCYFIFORST_FIELD =
{
    "COHERENCYFIFORST",
#if RU_INCLUDE_DESC
    "",
    "Writing 1 to this register will reset the coherency FIFO.\nFor a reset operation the SW should assert and then de-assert this bit.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_COHERENCYFIFORST_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_COHERENCYFIFORST_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_COHERENCYFIFORST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CNTXTRST *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_CNTXTRST_FIELD =
{
    "CNTXTRST",
#if RU_INCLUDE_DESC
    "",
    "Writing 1 to this register will reset the reassembly context table.\nThe reset is done immediately. Reading this register will always return 0.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_CNTXTRST_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_CNTXTRST_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_CNTXTRST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SDMARST *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_SDMARST_FIELD =
{
    "SDMARST",
#if RU_INCLUDE_DESC
    "",
    "Writing 1 to this register will reset the SDMA write pointer.\nFor a reset operation the SW should assert and then de-assert this bit.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_SDMARST_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_SDMARST_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_SDMARST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DISPNORMAL *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_DISPNORMAL_FIELD =
{
    "DISPNORMAL",
#if RU_INCLUDE_DESC
    "",
    "Writing 1 to this register will reset the dispatcher normal credits to a configurable value DISPCREDIT.\nFor a reset operation the SW should assert and then de-assert this bit.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_DISPNORMAL_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_DISPNORMAL_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_DISPNORMAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DISPEXCLUSIVE *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_DISPEXCLUSIVE_FIELD =
{
    "DISPEXCLUSIVE",
#if RU_INCLUDE_DESC
    "",
    "Writing 1 to this register will reset the dispatcher exclusive credits to a configurable value DISPCREDIT.\nFor a reset operation the SW should assert and then de-assert this bit.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_DISPEXCLUSIVE_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_DISPEXCLUSIVE_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_DISPEXCLUSIVE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: UPLDFIFORST *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_UPLDFIFORST_FIELD =
{
    "UPLDFIFORST",
#if RU_INCLUDE_DESC
    "",
    "Writing 1 to this register will reset the UPLD FIFO.\nFor a reset operation the SW should assert and then de-assert this bit.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_UPLDFIFORST_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_UPLDFIFORST_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_UPLDFIFORST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DISPCREDIT *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_DISPCREDIT_FIELD =
{
    "DISPCREDIT",
#if RU_INCLUDE_DESC
    "",
    "The field is used for both DISPNORMAL and DISPEXCLUSIVE reset bit.\nSetting DISPNORMAL and/or DISPEXCLUSIVE will init the relevant credit counter according to this field.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_DISPCREDIT_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_DISPCREDIT_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_DISPCREDIT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_INBUFRST_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_BURSTBUFRST_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_INGRESSCNTXT_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_CMDFIFORST_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_SBPMFIFORST_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_COHERENCYFIFORST_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_CNTXTRST_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_SDMARST_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_DISPNORMAL_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_DISPEXCLUSIVE_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_UPLDFIFORST_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_DISPCREDIT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_GENERAL_CONFIGURATION_RXRSTRST *****/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_REG =
{
    "GENERAL_CONFIGURATION_RXRSTRST",
#if RU_INCLUDE_DESC
    "RX_RESET_COMMAND Register",
    "This register enable reset of internal units (for WA perposes).\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_REG_OFFSET },
    0,
    0,
    47,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    12,
    BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_GENERAL_CONFIGURATION_RXDBGSEL, TYPE: Type_BBH_RX_BBHRX_GENERAL_CONFIGURATION_RXDBGSEL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RXDBGSEL *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_RXDBGSEL_RXDBGSEL_FIELD =
{
    "RXDBGSEL",
#if RU_INCLUDE_DESC
    "",
    "Selects one out of 10 possible debug vectors\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_RXDBGSEL_RXDBGSEL_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_RXDBGSEL_RXDBGSEL_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_RXDBGSEL_RXDBGSEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_RXDBGSEL_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_RXDBGSEL_RXDBGSEL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_GENERAL_CONFIGURATION_RXDBGSEL *****/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_RXDBGSEL_REG =
{
    "GENERAL_CONFIGURATION_RXDBGSEL",
#if RU_INCLUDE_DESC
    "RX_DEBUG_SELECT Register",
    "Selects one out of 10 possible debug vectors\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_RXDBGSEL_REG_OFFSET },
    0,
    0,
    48,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_GENERAL_CONFIGURATION_RXDBGSEL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_GENERAL_CONFIGURATION_BBHRX_RADDR_DECODER, TYPE: Type_BBH_RX_BBHRX_GENERAL_CONFIGURATION_BBHRX_RADDR_DECODER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ID_2OVERWR *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_BBHRX_RADDR_DECODER_ID_2OVERWR_FIELD =
{
    "ID_2OVERWR",
#if RU_INCLUDE_DESC
    "",
    "This field contains the users BB id for override\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_BBHRX_RADDR_DECODER_ID_2OVERWR_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_BBHRX_RADDR_DECODER_ID_2OVERWR_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_BBHRX_RADDR_DECODER_ID_2OVERWR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: OVERWR_RA *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_BBHRX_RADDR_DECODER_OVERWR_RA_FIELD =
{
    "OVERWR_RA",
#if RU_INCLUDE_DESC
    "",
    "The new RA\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_BBHRX_RADDR_DECODER_OVERWR_RA_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_BBHRX_RADDR_DECODER_OVERWR_RA_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_BBHRX_RADDR_DECODER_OVERWR_RA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: OVERWR_EN *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_BBHRX_RADDR_DECODER_OVERWR_EN_FIELD =
{
    "OVERWR_EN",
#if RU_INCLUDE_DESC
    "",
    "the overwr mechanism will be used only if this bit is active (1).\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_BBHRX_RADDR_DECODER_OVERWR_EN_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_BBHRX_RADDR_DECODER_OVERWR_EN_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_BBHRX_RADDR_DECODER_OVERWR_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_BBHRX_RADDR_DECODER_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_BBHRX_RADDR_DECODER_ID_2OVERWR_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_BBHRX_RADDR_DECODER_OVERWR_RA_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_BBHRX_RADDR_DECODER_OVERWR_EN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_GENERAL_CONFIGURATION_BBHRX_RADDR_DECODER *****/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_BBHRX_RADDR_DECODER_REG =
{
    "GENERAL_CONFIGURATION_BBHRX_RADDR_DECODER",
#if RU_INCLUDE_DESC
    "BBH_RX_RADDR_DECODER Register",
    "This register enables changing the route address for a specified BB ID\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_BBHRX_RADDR_DECODER_REG_OFFSET },
    0,
    0,
    49,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    BBH_RX_GENERAL_CONFIGURATION_BBHRX_RADDR_DECODER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_GENERAL_CONFIGURATION_NONETH, TYPE: Type_BBH_RX_BBHRX_GENERAL_CONFIGURATION_NONETH
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: FLOWID *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_NONETH_FLOWID_FIELD =
{
    "FLOWID",
#if RU_INCLUDE_DESC
    "",
    "Non Ethernet flow ID\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_NONETH_FLOWID_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_NONETH_FLOWID_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_NONETH_FLOWID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ENABLE *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_NONETH_ENABLE_FIELD =
{
    "ENABLE",
#if RU_INCLUDE_DESC
    "",
    "When asserted, CRC errors will not be counted for that flow.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_NONETH_ENABLE_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_NONETH_ENABLE_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_NONETH_ENABLE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_NONETH_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_NONETH_FLOWID_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_NONETH_ENABLE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_GENERAL_CONFIGURATION_NONETH *****/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_NONETH_REG =
{
    "GENERAL_CONFIGURATION_NONETH",
#if RU_INCLUDE_DESC
    "NON_ETHERNET_FLOW Register",
    "There an option to disable CRC error counting for this flow.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_NONETH_REG_OFFSET },
    0,
    0,
    50,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_RX_GENERAL_CONFIGURATION_NONETH_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL, TYPE: Type_BBH_RX_BBHRX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: BYPASS_CLK_GATE *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD =
{
    "BYPASS_CLK_GATE",
#if RU_INCLUDE_DESC
    "",
    "If set to 1b1 will disable the clock gate logic such to always enable the clock\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TIMER_VAL *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_TIMER_VAL_FIELD =
{
    "TIMER_VAL",
#if RU_INCLUDE_DESC
    "",
    "For how long should the clock stay active once all conditions for clock disable are met.\n\n\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_TIMER_VAL_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_TIMER_VAL_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_TIMER_VAL_FIELD_SHIFT },
    15,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: KEEP_ALIVE_EN *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD =
{
    "KEEP_ALIVE_EN",
#if RU_INCLUDE_DESC
    "",
    "Enables the keep alive logic which will periodically enable the clock to assure that no deadlock of clock being removed completely will occur\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: KEEP_ALIVE_INTRVL *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD =
{
    "KEEP_ALIVE_INTRVL",
#if RU_INCLUDE_DESC
    "",
    "If the KEEP alive option is enabled the field will determine for how many cycles should the clock be active\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD_SHIFT },
    2,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: KEEP_ALIVE_CYC *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD =
{
    "KEEP_ALIVE_CYC",
#if RU_INCLUDE_DESC
    "",
    "If the KEEP alive option is enabled this field will determine for how many cycles should the clock be disabled (minus the KEEP_ALIVE_INTERVAL)\n\nSo KEEP_ALIVE_CYCLE must be larger than KEEP_ALIVE_INTERVAL.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD_SHIFT },
    15,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_TIMER_VAL_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL *****/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_REG =
{
    "GENERAL_CONFIGURATION_CLK_GATE_CNTRL",
#if RU_INCLUDE_DESC
    "CLOCK_GATE_CONTROL Register",
    "Clock Gate control register including timer config and bypass control\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_REG_OFFSET },
    0,
    0,
    51,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_GENERAL_CONFIGURATION_PFCCONTROL, TYPE: Type_BBH_RX_BBHRX_GENERAL_CONFIGURATION_PFCCONTROL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RUNNERADDR *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_PFCCONTROL_RUNNERADDR_FIELD =
{
    "RUNNERADDR",
#if RU_INCLUDE_DESC
    "",
    "Defines the target address in the TM Runner, to which the PFC vector should be written.\nAddress is in 8-bytes resolution.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_PFCCONTROL_RUNNERADDR_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_PFCCONTROL_RUNNERADDR_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_PFCCONTROL_RUNNERADDR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PFCEN *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_PFCCONTROL_PFCEN_FIELD =
{
    "PFCEN",
#if RU_INCLUDE_DESC
    "",
    "Enables BBH RX PFC.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_PFCCONTROL_PFCEN_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_PFCCONTROL_PFCEN_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_PFCCONTROL_PFCEN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_PFCCONTROL_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_PFCCONTROL_RUNNERADDR_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_PFCCONTROL_PFCEN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_GENERAL_CONFIGURATION_PFCCONTROL *****/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_PFCCONTROL_REG =
{
    "GENERAL_CONFIGURATION_PFCCONTROL",
#if RU_INCLUDE_DESC
    "PFC_CONTROL Register",
    "UNIMAC handles PFC. It notifies BBH using a dedicated strobe and 8 bits vector. BBH updates a dedicated SRAM memory vector for TX-Task to monitor (target Runner and data memory address are configurable)\nThis vector (8-bits/priority) is per BBH.\nSetting/Clearing this vector is done by BBH (as per XON/XOFF PFC frames).\n\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_PFCCONTROL_REG_OFFSET },
    0,
    0,
    52,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_RX_GENERAL_CONFIGURATION_PFCCONTROL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_GENERAL_CONFIGURATION_EPONSEQDIS, TYPE: Type_BBH_RX_BBHRX_GENERAL_CONFIGURATION_EPONSEQDIS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DISABLE *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_EPONSEQDIS_DISABLE_FIELD =
{
    "DISABLE",
#if RU_INCLUDE_DESC
    "",
    "Disables EPON type sequence error fix\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_EPONSEQDIS_DISABLE_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_EPONSEQDIS_DISABLE_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_EPONSEQDIS_DISABLE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_EPONSEQDIS_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_EPONSEQDIS_DISABLE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_GENERAL_CONFIGURATION_EPONSEQDIS *****/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_EPONSEQDIS_REG =
{
    "GENERAL_CONFIGURATION_EPONSEQDIS",
#if RU_INCLUDE_DESC
    "EPON_SEQUENCE_ERROR_DIS Register",
    "Disables a fix for EPON type error in case of MAC RX FIFO overrun or in case of soft reset\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_EPONSEQDIS_REG_OFFSET },
    0,
    0,
    53,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_GENERAL_CONFIGURATION_EPONSEQDIS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_GENERAL_CONFIGURATION_MACFLOW, TYPE: Type_BBH_RX_BBHRX_GENERAL_CONFIGURATION_MACFLOW
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: MACFLOW *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_MACFLOW_MACFLOW_FIELD =
{
    "MACFLOW",
#if RU_INCLUDE_DESC
    "",
    "Configured FW value for MAC flow field in the PD (previously was BB ID)\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_MACFLOW_MACFLOW_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_MACFLOW_MACFLOW_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_MACFLOW_MACFLOW_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_MACFLOW_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_MACFLOW_MACFLOW_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_GENERAL_CONFIGURATION_MACFLOW *****/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_MACFLOW_REG =
{
    "GENERAL_CONFIGURATION_MACFLOW",
#if RU_INCLUDE_DESC
    "MAC_FLOW_CONFIG Register",
    "Configured FW value for MAC flow field in the PD (previously was BB ID)\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_MACFLOW_REG_OFFSET },
    0,
    0,
    54,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_GENERAL_CONFIGURATION_MACFLOW_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_PM_COUNTERS_INPKT, TYPE: Type_BBH_RX_BBHRX_PM_COUNTERS_INPKT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: INPKT *****/
const ru_field_rec BBH_RX_PM_COUNTERS_INPKT_INPKT_FIELD =
{
    "INPKT",
#if RU_INCLUDE_DESC
    "",
    "This counter counts the number of incoming good packets.\n",
#endif
    { BBH_RX_PM_COUNTERS_INPKT_INPKT_FIELD_MASK },
    0,
    { BBH_RX_PM_COUNTERS_INPKT_INPKT_FIELD_WIDTH },
    { BBH_RX_PM_COUNTERS_INPKT_INPKT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_PM_COUNTERS_INPKT_FIELDS[] =
{
    &BBH_RX_PM_COUNTERS_INPKT_INPKT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_PM_COUNTERS_INPKT *****/
const ru_reg_rec BBH_RX_PM_COUNTERS_INPKT_REG =
{
    "PM_COUNTERS_INPKT",
#if RU_INCLUDE_DESC
    "INCOMING_PACKETS Register",
    "This counter counts the number of incoming good packets.\nIt counts the packets from all flows together.\nThis counter is cleared when read and freezes when reaches the maximum value.\n",
#endif
    { BBH_RX_PM_COUNTERS_INPKT_REG_OFFSET },
    0,
    0,
    55,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_PM_COUNTERS_INPKT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_PM_COUNTERS_THIRDFLOW, TYPE: Type_BBH_RX_BBHRX_PM_COUNTERS_THIRDFLOW
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PMVALUE *****/
const ru_field_rec BBH_RX_PM_COUNTERS_THIRDFLOW_PMVALUE_FIELD =
{
    "PMVALUE",
#if RU_INCLUDE_DESC
    "",
    "PM counter value.\n",
#endif
    { BBH_RX_PM_COUNTERS_THIRDFLOW_PMVALUE_FIELD_MASK },
    0,
    { BBH_RX_PM_COUNTERS_THIRDFLOW_PMVALUE_FIELD_WIDTH },
    { BBH_RX_PM_COUNTERS_THIRDFLOW_PMVALUE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_PM_COUNTERS_THIRDFLOW_FIELDS[] =
{
    &BBH_RX_PM_COUNTERS_THIRDFLOW_PMVALUE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_PM_COUNTERS_THIRDFLOW *****/
const ru_reg_rec BBH_RX_PM_COUNTERS_THIRDFLOW_REG =
{
    "PM_COUNTERS_THIRDFLOW",
#if RU_INCLUDE_DESC
    "THIRD_FLOW_ERROR Register",
    "This counter counts the packets drop due to Third flow error.\nThis counter is cleared when read and freezes when reaches the maximum value.\n",
#endif
    { BBH_RX_PM_COUNTERS_THIRDFLOW_REG_OFFSET },
    0,
    0,
    56,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_PM_COUNTERS_THIRDFLOW_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_PM_COUNTERS_SOPASOP, TYPE: Type_BBH_RX_BBHRX_PM_COUNTERS_SOPASOP
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PMVALUE *****/
const ru_field_rec BBH_RX_PM_COUNTERS_SOPASOP_PMVALUE_FIELD =
{
    "PMVALUE",
#if RU_INCLUDE_DESC
    "",
    "PM counter value.\n",
#endif
    { BBH_RX_PM_COUNTERS_SOPASOP_PMVALUE_FIELD_MASK },
    0,
    { BBH_RX_PM_COUNTERS_SOPASOP_PMVALUE_FIELD_WIDTH },
    { BBH_RX_PM_COUNTERS_SOPASOP_PMVALUE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_PM_COUNTERS_SOPASOP_FIELDS[] =
{
    &BBH_RX_PM_COUNTERS_SOPASOP_PMVALUE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_PM_COUNTERS_SOPASOP *****/
const ru_reg_rec BBH_RX_PM_COUNTERS_SOPASOP_REG =
{
    "PM_COUNTERS_SOPASOP",
#if RU_INCLUDE_DESC
    "SOP_AFTER_SOP_ERROR Register",
    "This counter counts the packets drop due to SOP after SOP error.\nThis counter is cleared when read and freezes when reaches the maximum value.\n",
#endif
    { BBH_RX_PM_COUNTERS_SOPASOP_REG_OFFSET },
    0,
    0,
    57,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_PM_COUNTERS_SOPASOP_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_PM_COUNTERS_TOOSHORT, TYPE: Type_BBH_RX_BBHRX_PM_COUNTERS_TOOSHORT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PMVALUE *****/
const ru_field_rec BBH_RX_PM_COUNTERS_TOOSHORT_PMVALUE_FIELD =
{
    "PMVALUE",
#if RU_INCLUDE_DESC
    "",
    "PM counter value.\n",
#endif
    { BBH_RX_PM_COUNTERS_TOOSHORT_PMVALUE_FIELD_MASK },
    0,
    { BBH_RX_PM_COUNTERS_TOOSHORT_PMVALUE_FIELD_WIDTH },
    { BBH_RX_PM_COUNTERS_TOOSHORT_PMVALUE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_PM_COUNTERS_TOOSHORT_FIELDS[] =
{
    &BBH_RX_PM_COUNTERS_TOOSHORT_PMVALUE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_PM_COUNTERS_TOOSHORT *****/
const ru_reg_rec BBH_RX_PM_COUNTERS_TOOSHORT_REG =
{
    "PM_COUNTERS_TOOSHORT",
#if RU_INCLUDE_DESC
    "TOO_SHORT_ERROR Register",
    "This counter counts the packets drop due to Too short error.\nThis counter is cleared when read and freezes when reaches the maximum value.\n",
#endif
    { BBH_RX_PM_COUNTERS_TOOSHORT_REG_OFFSET },
    0,
    0,
    58,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_PM_COUNTERS_TOOSHORT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_PM_COUNTERS_TOOLONG, TYPE: Type_BBH_RX_BBHRX_PM_COUNTERS_TOOLONG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PMVALUE *****/
const ru_field_rec BBH_RX_PM_COUNTERS_TOOLONG_PMVALUE_FIELD =
{
    "PMVALUE",
#if RU_INCLUDE_DESC
    "",
    "PM counter value.\n",
#endif
    { BBH_RX_PM_COUNTERS_TOOLONG_PMVALUE_FIELD_MASK },
    0,
    { BBH_RX_PM_COUNTERS_TOOLONG_PMVALUE_FIELD_WIDTH },
    { BBH_RX_PM_COUNTERS_TOOLONG_PMVALUE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_PM_COUNTERS_TOOLONG_FIELDS[] =
{
    &BBH_RX_PM_COUNTERS_TOOLONG_PMVALUE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_PM_COUNTERS_TOOLONG *****/
const ru_reg_rec BBH_RX_PM_COUNTERS_TOOLONG_REG =
{
    "PM_COUNTERS_TOOLONG",
#if RU_INCLUDE_DESC
    "TOO_LONG_ERROR Register",
    "This counter counts the packets drop due to Too long error.\nThis counter is cleared when read and freezes when reaches the maximum value.\n",
#endif
    { BBH_RX_PM_COUNTERS_TOOLONG_REG_OFFSET },
    0,
    0,
    59,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_PM_COUNTERS_TOOLONG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_PM_COUNTERS_CRCERROR, TYPE: Type_BBH_RX_BBHRX_PM_COUNTERS_CRCERROR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PMVALUE *****/
const ru_field_rec BBH_RX_PM_COUNTERS_CRCERROR_PMVALUE_FIELD =
{
    "PMVALUE",
#if RU_INCLUDE_DESC
    "",
    "PM counter value.\n",
#endif
    { BBH_RX_PM_COUNTERS_CRCERROR_PMVALUE_FIELD_MASK },
    0,
    { BBH_RX_PM_COUNTERS_CRCERROR_PMVALUE_FIELD_WIDTH },
    { BBH_RX_PM_COUNTERS_CRCERROR_PMVALUE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_PM_COUNTERS_CRCERROR_FIELDS[] =
{
    &BBH_RX_PM_COUNTERS_CRCERROR_PMVALUE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_PM_COUNTERS_CRCERROR *****/
const ru_reg_rec BBH_RX_PM_COUNTERS_CRCERROR_REG =
{
    "PM_COUNTERS_CRCERROR",
#if RU_INCLUDE_DESC
    "CRC_ERROR Register",
    "This counter counts the packets drop due to CRC error.\nThis counter is cleared when read and freezes when reaches the maximum value.\n",
#endif
    { BBH_RX_PM_COUNTERS_CRCERROR_REG_OFFSET },
    0,
    0,
    60,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_PM_COUNTERS_CRCERROR_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_PM_COUNTERS_ENCRYPTERROR, TYPE: Type_BBH_RX_BBHRX_PM_COUNTERS_ENCRYPTERROR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PMVALUE *****/
const ru_field_rec BBH_RX_PM_COUNTERS_ENCRYPTERROR_PMVALUE_FIELD =
{
    "PMVALUE",
#if RU_INCLUDE_DESC
    "",
    "PM counter value.\n",
#endif
    { BBH_RX_PM_COUNTERS_ENCRYPTERROR_PMVALUE_FIELD_MASK },
    0,
    { BBH_RX_PM_COUNTERS_ENCRYPTERROR_PMVALUE_FIELD_WIDTH },
    { BBH_RX_PM_COUNTERS_ENCRYPTERROR_PMVALUE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_PM_COUNTERS_ENCRYPTERROR_FIELDS[] =
{
    &BBH_RX_PM_COUNTERS_ENCRYPTERROR_PMVALUE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_PM_COUNTERS_ENCRYPTERROR *****/
const ru_reg_rec BBH_RX_PM_COUNTERS_ENCRYPTERROR_REG =
{
    "PM_COUNTERS_ENCRYPTERROR",
#if RU_INCLUDE_DESC
    "ENCRYPTION_ERROR Register",
    "This counter counts the packets drop due to XGPON encryption error.\nThis counter is cleared when read and freezes when reaches the maximum value.\n",
#endif
    { BBH_RX_PM_COUNTERS_ENCRYPTERROR_REG_OFFSET },
    0,
    0,
    61,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_PM_COUNTERS_ENCRYPTERROR_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_PM_COUNTERS_DISPCONG, TYPE: Type_BBH_RX_BBHRX_PM_COUNTERS_DISPCONG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PMVALUE *****/
const ru_field_rec BBH_RX_PM_COUNTERS_DISPCONG_PMVALUE_FIELD =
{
    "PMVALUE",
#if RU_INCLUDE_DESC
    "",
    "PM counter value.\n",
#endif
    { BBH_RX_PM_COUNTERS_DISPCONG_PMVALUE_FIELD_MASK },
    0,
    { BBH_RX_PM_COUNTERS_DISPCONG_PMVALUE_FIELD_WIDTH },
    { BBH_RX_PM_COUNTERS_DISPCONG_PMVALUE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_PM_COUNTERS_DISPCONG_FIELDS[] =
{
    &BBH_RX_PM_COUNTERS_DISPCONG_PMVALUE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_PM_COUNTERS_DISPCONG *****/
const ru_reg_rec BBH_RX_PM_COUNTERS_DISPCONG_REG =
{
    "PM_COUNTERS_DISPCONG",
#if RU_INCLUDE_DESC
    "DISPATCHER_CONGESTION_ERROR Register",
    "This counter counts the packets drop due to Dispatcher congestion error.\nThis counter is cleared when read and freezes when reaches the maximum value.\n",
#endif
    { BBH_RX_PM_COUNTERS_DISPCONG_REG_OFFSET },
    0,
    0,
    62,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_PM_COUNTERS_DISPCONG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_PM_COUNTERS_NOSBPMSBN, TYPE: Type_BBH_RX_BBHRX_PM_COUNTERS_NOSBPMSBN
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PMVALUE *****/
const ru_field_rec BBH_RX_PM_COUNTERS_NOSBPMSBN_PMVALUE_FIELD =
{
    "PMVALUE",
#if RU_INCLUDE_DESC
    "",
    "PM counter value.\n",
#endif
    { BBH_RX_PM_COUNTERS_NOSBPMSBN_PMVALUE_FIELD_MASK },
    0,
    { BBH_RX_PM_COUNTERS_NOSBPMSBN_PMVALUE_FIELD_WIDTH },
    { BBH_RX_PM_COUNTERS_NOSBPMSBN_PMVALUE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_PM_COUNTERS_NOSBPMSBN_FIELDS[] =
{
    &BBH_RX_PM_COUNTERS_NOSBPMSBN_PMVALUE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_PM_COUNTERS_NOSBPMSBN *****/
const ru_reg_rec BBH_RX_PM_COUNTERS_NOSBPMSBN_REG =
{
    "PM_COUNTERS_NOSBPMSBN",
#if RU_INCLUDE_DESC
    "NO_SBPM_SBN_ERROR Register",
    "This counter counts the packets drop due to NO SBPM SBN error.\nThis counter is cleared when read and freezes when reaches the maximum value.\n",
#endif
    { BBH_RX_PM_COUNTERS_NOSBPMSBN_REG_OFFSET },
    0,
    0,
    63,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_PM_COUNTERS_NOSBPMSBN_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_PM_COUNTERS_NOSDMACD, TYPE: Type_BBH_RX_BBHRX_PM_COUNTERS_NOSDMACD
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PMVALUE *****/
const ru_field_rec BBH_RX_PM_COUNTERS_NOSDMACD_PMVALUE_FIELD =
{
    "PMVALUE",
#if RU_INCLUDE_DESC
    "",
    "PM counter value.\n",
#endif
    { BBH_RX_PM_COUNTERS_NOSDMACD_PMVALUE_FIELD_MASK },
    0,
    { BBH_RX_PM_COUNTERS_NOSDMACD_PMVALUE_FIELD_WIDTH },
    { BBH_RX_PM_COUNTERS_NOSDMACD_PMVALUE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_PM_COUNTERS_NOSDMACD_FIELDS[] =
{
    &BBH_RX_PM_COUNTERS_NOSDMACD_PMVALUE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_PM_COUNTERS_NOSDMACD *****/
const ru_reg_rec BBH_RX_PM_COUNTERS_NOSDMACD_REG =
{
    "PM_COUNTERS_NOSDMACD",
#if RU_INCLUDE_DESC
    "NO_SDMA_CD_ERROR Register",
    "This counter counts the packets drop due to No SDMA CD error.\nThis counter is cleared when read and freezes when reaches the maximum value.\n",
#endif
    { BBH_RX_PM_COUNTERS_NOSDMACD_REG_OFFSET },
    0,
    0,
    64,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_PM_COUNTERS_NOSDMACD_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_PM_COUNTERS_INPLOAM, TYPE: Type_BBH_RX_BBHRX_PM_COUNTERS_INPLOAM
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: INPLOAM *****/
const ru_field_rec BBH_RX_PM_COUNTERS_INPLOAM_INPLOAM_FIELD =
{
    "INPLOAM",
#if RU_INCLUDE_DESC
    "",
    "This counter counts the number of incoming PLOAMs.\n",
#endif
    { BBH_RX_PM_COUNTERS_INPLOAM_INPLOAM_FIELD_MASK },
    0,
    { BBH_RX_PM_COUNTERS_INPLOAM_INPLOAM_FIELD_WIDTH },
    { BBH_RX_PM_COUNTERS_INPLOAM_INPLOAM_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_PM_COUNTERS_INPLOAM_FIELDS[] =
{
    &BBH_RX_PM_COUNTERS_INPLOAM_INPLOAM_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_PM_COUNTERS_INPLOAM *****/
const ru_reg_rec BBH_RX_PM_COUNTERS_INPLOAM_REG =
{
    "PM_COUNTERS_INPLOAM",
#if RU_INCLUDE_DESC
    "INCOMING_PLOAM Register",
    "This counter counts the number of incoming good PLOAMs.\nThis counter is cleared when read and freezes when reaches the maximum value.\n",
#endif
    { BBH_RX_PM_COUNTERS_INPLOAM_REG_OFFSET },
    0,
    0,
    65,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_PM_COUNTERS_INPLOAM_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_PM_COUNTERS_CRCERRORPLOAM, TYPE: Type_BBH_RX_BBHRX_PM_COUNTERS_CRCERRORPLOAM
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PMVALUE *****/
const ru_field_rec BBH_RX_PM_COUNTERS_CRCERRORPLOAM_PMVALUE_FIELD =
{
    "PMVALUE",
#if RU_INCLUDE_DESC
    "",
    "PM counter value.\n",
#endif
    { BBH_RX_PM_COUNTERS_CRCERRORPLOAM_PMVALUE_FIELD_MASK },
    0,
    { BBH_RX_PM_COUNTERS_CRCERRORPLOAM_PMVALUE_FIELD_WIDTH },
    { BBH_RX_PM_COUNTERS_CRCERRORPLOAM_PMVALUE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_PM_COUNTERS_CRCERRORPLOAM_FIELDS[] =
{
    &BBH_RX_PM_COUNTERS_CRCERRORPLOAM_PMVALUE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_PM_COUNTERS_CRCERRORPLOAM *****/
const ru_reg_rec BBH_RX_PM_COUNTERS_CRCERRORPLOAM_REG =
{
    "PM_COUNTERS_CRCERRORPLOAM",
#if RU_INCLUDE_DESC
    "CRC_PLOAM_ERROR Register",
    "This counter counts the PLOAMs drop due to CRC error.\nThis counter is cleared when read and freezes when reaches the maximum value.\n",
#endif
    { BBH_RX_PM_COUNTERS_CRCERRORPLOAM_REG_OFFSET },
    0,
    0,
    66,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_PM_COUNTERS_CRCERRORPLOAM_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_PM_COUNTERS_DISPCONGPLOAM, TYPE: Type_BBH_RX_BBHRX_PM_COUNTERS_DISPCONGPLOAM
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PMVALUE *****/
const ru_field_rec BBH_RX_PM_COUNTERS_DISPCONGPLOAM_PMVALUE_FIELD =
{
    "PMVALUE",
#if RU_INCLUDE_DESC
    "",
    "PM counter value.\n",
#endif
    { BBH_RX_PM_COUNTERS_DISPCONGPLOAM_PMVALUE_FIELD_MASK },
    0,
    { BBH_RX_PM_COUNTERS_DISPCONGPLOAM_PMVALUE_FIELD_WIDTH },
    { BBH_RX_PM_COUNTERS_DISPCONGPLOAM_PMVALUE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_PM_COUNTERS_DISPCONGPLOAM_FIELDS[] =
{
    &BBH_RX_PM_COUNTERS_DISPCONGPLOAM_PMVALUE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_PM_COUNTERS_DISPCONGPLOAM *****/
const ru_reg_rec BBH_RX_PM_COUNTERS_DISPCONGPLOAM_REG =
{
    "PM_COUNTERS_DISPCONGPLOAM",
#if RU_INCLUDE_DESC
    "DISPATCHER_CONGESTION_PLOAM_ERROR Register",
    "This counter counts the packets drop due to Dispatcher congestion error for PLOAM.\nThis counter is cleared when read and freezes when reaches the maximum value.\n",
#endif
    { BBH_RX_PM_COUNTERS_DISPCONGPLOAM_REG_OFFSET },
    0,
    0,
    67,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_PM_COUNTERS_DISPCONGPLOAM_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_PM_COUNTERS_NOSBPMSBNPLOAM, TYPE: Type_BBH_RX_BBHRX_PM_COUNTERS_NOSBPMSBNPLOAM
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PMVALUE *****/
const ru_field_rec BBH_RX_PM_COUNTERS_NOSBPMSBNPLOAM_PMVALUE_FIELD =
{
    "PMVALUE",
#if RU_INCLUDE_DESC
    "",
    "PM counter value.\n",
#endif
    { BBH_RX_PM_COUNTERS_NOSBPMSBNPLOAM_PMVALUE_FIELD_MASK },
    0,
    { BBH_RX_PM_COUNTERS_NOSBPMSBNPLOAM_PMVALUE_FIELD_WIDTH },
    { BBH_RX_PM_COUNTERS_NOSBPMSBNPLOAM_PMVALUE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_PM_COUNTERS_NOSBPMSBNPLOAM_FIELDS[] =
{
    &BBH_RX_PM_COUNTERS_NOSBPMSBNPLOAM_PMVALUE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_PM_COUNTERS_NOSBPMSBNPLOAM *****/
const ru_reg_rec BBH_RX_PM_COUNTERS_NOSBPMSBNPLOAM_REG =
{
    "PM_COUNTERS_NOSBPMSBNPLOAM",
#if RU_INCLUDE_DESC
    "NO_SBPM_SBN_PLOAM_ERROR Register",
    "This counter counts the PLOAMs drop due to No SBPM SBN error.\nThis counter is cleared when read and freezes when reaches the maximum value.\n",
#endif
    { BBH_RX_PM_COUNTERS_NOSBPMSBNPLOAM_REG_OFFSET },
    0,
    0,
    68,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_PM_COUNTERS_NOSBPMSBNPLOAM_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_PM_COUNTERS_NOSDMACDPLOAM, TYPE: Type_BBH_RX_BBHRX_PM_COUNTERS_NOSDMACDPLOAM
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PMVALUE *****/
const ru_field_rec BBH_RX_PM_COUNTERS_NOSDMACDPLOAM_PMVALUE_FIELD =
{
    "PMVALUE",
#if RU_INCLUDE_DESC
    "",
    "PM counter value.\n",
#endif
    { BBH_RX_PM_COUNTERS_NOSDMACDPLOAM_PMVALUE_FIELD_MASK },
    0,
    { BBH_RX_PM_COUNTERS_NOSDMACDPLOAM_PMVALUE_FIELD_WIDTH },
    { BBH_RX_PM_COUNTERS_NOSDMACDPLOAM_PMVALUE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_PM_COUNTERS_NOSDMACDPLOAM_FIELDS[] =
{
    &BBH_RX_PM_COUNTERS_NOSDMACDPLOAM_PMVALUE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_PM_COUNTERS_NOSDMACDPLOAM *****/
const ru_reg_rec BBH_RX_PM_COUNTERS_NOSDMACDPLOAM_REG =
{
    "PM_COUNTERS_NOSDMACDPLOAM",
#if RU_INCLUDE_DESC
    "NO_SDMA_CD_PLOAM_ERROR Register",
    "This counter counts the packets drop due to No SDMA CD error for PLOAMs.\nThis counter is cleared when read and freezes when reaches the maximum value.\n",
#endif
    { BBH_RX_PM_COUNTERS_NOSDMACDPLOAM_REG_OFFSET },
    0,
    0,
    69,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_PM_COUNTERS_NOSDMACDPLOAM_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_PM_COUNTERS_EPONTYPERROR, TYPE: Type_BBH_RX_BBHRX_PM_COUNTERS_EPONTYPERROR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PMVALUE *****/
const ru_field_rec BBH_RX_PM_COUNTERS_EPONTYPERROR_PMVALUE_FIELD =
{
    "PMVALUE",
#if RU_INCLUDE_DESC
    "",
    "PM counter value.\n",
#endif
    { BBH_RX_PM_COUNTERS_EPONTYPERROR_PMVALUE_FIELD_MASK },
    0,
    { BBH_RX_PM_COUNTERS_EPONTYPERROR_PMVALUE_FIELD_WIDTH },
    { BBH_RX_PM_COUNTERS_EPONTYPERROR_PMVALUE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_PM_COUNTERS_EPONTYPERROR_FIELDS[] =
{
    &BBH_RX_PM_COUNTERS_EPONTYPERROR_PMVALUE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_PM_COUNTERS_EPONTYPERROR *****/
const ru_reg_rec BBH_RX_PM_COUNTERS_EPONTYPERROR_REG =
{
    "PM_COUNTERS_EPONTYPERROR",
#if RU_INCLUDE_DESC
    "EPON_TYPE_ERROR Register",
    "This counter counts the events of EPON type sequence which is wrong, meaning no sop after header, or sop/header in the middle of packet (before eop).\nThis counter is cleared when read and freezes when reaches the maximum value.\n",
#endif
    { BBH_RX_PM_COUNTERS_EPONTYPERROR_REG_OFFSET },
    0,
    0,
    70,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_PM_COUNTERS_EPONTYPERROR_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_PM_COUNTERS_RUNTERROR, TYPE: Type_BBH_RX_BBHRX_PM_COUNTERS_RUNTERROR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PMVALUE *****/
const ru_field_rec BBH_RX_PM_COUNTERS_RUNTERROR_PMVALUE_FIELD =
{
    "PMVALUE",
#if RU_INCLUDE_DESC
    "",
    "PM counter value.\n",
#endif
    { BBH_RX_PM_COUNTERS_RUNTERROR_PMVALUE_FIELD_MASK },
    0,
    { BBH_RX_PM_COUNTERS_RUNTERROR_PMVALUE_FIELD_WIDTH },
    { BBH_RX_PM_COUNTERS_RUNTERROR_PMVALUE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_PM_COUNTERS_RUNTERROR_FIELDS[] =
{
    &BBH_RX_PM_COUNTERS_RUNTERROR_PMVALUE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_PM_COUNTERS_RUNTERROR *****/
const ru_reg_rec BBH_RX_PM_COUNTERS_RUNTERROR_REG =
{
    "PM_COUNTERS_RUNTERROR",
#if RU_INCLUDE_DESC
    "RUNT_ERROR Register",
    "This counter counts the number of RUNT packets received from the XLMAC.\nThis counter is cleared when read and freezes when reaches the maximum value.\n",
#endif
    { BBH_RX_PM_COUNTERS_RUNTERROR_REG_OFFSET },
    0,
    0,
    71,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_PM_COUNTERS_RUNTERROR_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_PM_COUNTERS_INBYTE, TYPE: Type_BBH_RX_BBHRX_PM_COUNTERS_INBYTE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: INBYTE *****/
const ru_field_rec BBH_RX_PM_COUNTERS_INBYTE_INBYTE_FIELD =
{
    "INBYTE",
#if RU_INCLUDE_DESC
    "",
    "This counter counts the number of incoming good bytes.\n",
#endif
    { BBH_RX_PM_COUNTERS_INBYTE_INBYTE_FIELD_MASK },
    0,
    { BBH_RX_PM_COUNTERS_INBYTE_INBYTE_FIELD_WIDTH },
    { BBH_RX_PM_COUNTERS_INBYTE_INBYTE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_PM_COUNTERS_INBYTE_FIELDS[] =
{
    &BBH_RX_PM_COUNTERS_INBYTE_INBYTE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_PM_COUNTERS_INBYTE *****/
const ru_reg_rec BBH_RX_PM_COUNTERS_INBYTE_REG =
{
    "PM_COUNTERS_INBYTE",
#if RU_INCLUDE_DESC
    "INCOMING_BYTES Register",
    "This counter counts the number of incoming good bytes.\nIt counts the bytes from all flows together.\nThis counter is cleared when read and freezes when reaches the maximum value.\n",
#endif
    { BBH_RX_PM_COUNTERS_INBYTE_REG_OFFSET },
    0,
    0,
    72,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_PM_COUNTERS_INBYTE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_PM_COUNTERS_FLOWNOTCOUNTED, TYPE: Type_BBH_RX_BBHRX_PM_COUNTERS_FLOWNOTCOUNTED
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PMVALUE *****/
const ru_field_rec BBH_RX_PM_COUNTERS_FLOWNOTCOUNTED_PMVALUE_FIELD =
{
    "PMVALUE",
#if RU_INCLUDE_DESC
    "",
    "PM counter value.\n",
#endif
    { BBH_RX_PM_COUNTERS_FLOWNOTCOUNTED_PMVALUE_FIELD_MASK },
    0,
    { BBH_RX_PM_COUNTERS_FLOWNOTCOUNTED_PMVALUE_FIELD_WIDTH },
    { BBH_RX_PM_COUNTERS_FLOWNOTCOUNTED_PMVALUE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_PM_COUNTERS_FLOWNOTCOUNTED_FIELDS[] =
{
    &BBH_RX_PM_COUNTERS_FLOWNOTCOUNTED_PMVALUE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_PM_COUNTERS_FLOWNOTCOUNTED *****/
const ru_reg_rec BBH_RX_PM_COUNTERS_FLOWNOTCOUNTED_REG =
{
    "PM_COUNTERS_FLOWNOTCOUNTED",
#if RU_INCLUDE_DESC
    "FLOW_NOT_COUNTED Register",
    "This counter counts the number of WAN flow packets, which are not counted per flow, due to 2 reasons:\n1. Flow >= 130\n2. Pre counter FIFO full\nThis counter is cleared when read and freezes when reaches the maximum value.\n",
#endif
    { BBH_RX_PM_COUNTERS_FLOWNOTCOUNTED_REG_OFFSET },
    0,
    0,
    73,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_PM_COUNTERS_FLOWNOTCOUNTED_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_DEBUG_CNTXTX0LSB, TYPE: Type_BBH_RX_BBHRX_DEBUG_CNTXTX0LSB
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: INREASS *****/
const ru_field_rec BBH_RX_DEBUG_CNTXTX0LSB_INREASS_FIELD =
{
    "INREASS",
#if RU_INCLUDE_DESC
    "",
    "In reassembly.\nNot relevant for Ethernet.\n",
#endif
    { BBH_RX_DEBUG_CNTXTX0LSB_INREASS_FIELD_MASK },
    0,
    { BBH_RX_DEBUG_CNTXTX0LSB_INREASS_FIELD_WIDTH },
    { BBH_RX_DEBUG_CNTXTX0LSB_INREASS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FLOWID *****/
const ru_field_rec BBH_RX_DEBUG_CNTXTX0LSB_FLOWID_FIELD =
{
    "FLOWID",
#if RU_INCLUDE_DESC
    "",
    "Flow ID\n",
#endif
    { BBH_RX_DEBUG_CNTXTX0LSB_FLOWID_FIELD_MASK },
    0,
    { BBH_RX_DEBUG_CNTXTX0LSB_FLOWID_FIELD_WIDTH },
    { BBH_RX_DEBUG_CNTXTX0LSB_FLOWID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CUROFFSET *****/
const ru_field_rec BBH_RX_DEBUG_CNTXTX0LSB_CUROFFSET_FIELD =
{
    "CUROFFSET",
#if RU_INCLUDE_DESC
    "",
    "Current offset\n",
#endif
    { BBH_RX_DEBUG_CNTXTX0LSB_CUROFFSET_FIELD_MASK },
    0,
    { BBH_RX_DEBUG_CNTXTX0LSB_CUROFFSET_FIELD_WIDTH },
    { BBH_RX_DEBUG_CNTXTX0LSB_CUROFFSET_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_DEBUG_CNTXTX0LSB_FIELDS[] =
{
    &BBH_RX_DEBUG_CNTXTX0LSB_INREASS_FIELD,
    &BBH_RX_DEBUG_CNTXTX0LSB_FLOWID_FIELD,
    &BBH_RX_DEBUG_CNTXTX0LSB_CUROFFSET_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_DEBUG_CNTXTX0LSB *****/
const ru_reg_rec BBH_RX_DEBUG_CNTXTX0LSB_REG =
{
    "DEBUG_CNTXTX0LSB",
#if RU_INCLUDE_DESC
    "CONTEXT_0_LSB Register",
    "In the case of GPON peripheral, DS flows may arrive interleaved. The BBH supports parallel reassembly of up to two interleaved flows (out of 256). For the reassembly process the BBH stores a double flow context.\n",
#endif
    { BBH_RX_DEBUG_CNTXTX0LSB_REG_OFFSET },
    0,
    0,
    74,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    BBH_RX_DEBUG_CNTXTX0LSB_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_DEBUG_CNTXTX0MSB, TYPE: Type_BBH_RX_BBHRX_DEBUG_CNTXTX0MSB
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CURBN *****/
const ru_field_rec BBH_RX_DEBUG_CNTXTX0MSB_CURBN_FIELD =
{
    "CURBN",
#if RU_INCLUDE_DESC
    "",
    "Current BN\n",
#endif
    { BBH_RX_DEBUG_CNTXTX0MSB_CURBN_FIELD_MASK },
    0,
    { BBH_RX_DEBUG_CNTXTX0MSB_CURBN_FIELD_WIDTH },
    { BBH_RX_DEBUG_CNTXTX0MSB_CURBN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FIRSTBN *****/
const ru_field_rec BBH_RX_DEBUG_CNTXTX0MSB_FIRSTBN_FIELD =
{
    "FIRSTBN",
#if RU_INCLUDE_DESC
    "",
    "First BN\n",
#endif
    { BBH_RX_DEBUG_CNTXTX0MSB_FIRSTBN_FIELD_MASK },
    0,
    { BBH_RX_DEBUG_CNTXTX0MSB_FIRSTBN_FIELD_WIDTH },
    { BBH_RX_DEBUG_CNTXTX0MSB_FIRSTBN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_DEBUG_CNTXTX0MSB_FIELDS[] =
{
    &BBH_RX_DEBUG_CNTXTX0MSB_CURBN_FIELD,
    &BBH_RX_DEBUG_CNTXTX0MSB_FIRSTBN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_DEBUG_CNTXTX0MSB *****/
const ru_reg_rec BBH_RX_DEBUG_CNTXTX0MSB_REG =
{
    "DEBUG_CNTXTX0MSB",
#if RU_INCLUDE_DESC
    "CONTEXT_0_MSB Register",
    "In the case of GPON peripheral, DS flows may arrive interleaved. The BBH supports parallel reassembly of up to two interleaved flows (out of 256). For the reassembly process the BBH stores a double flow context.\n",
#endif
    { BBH_RX_DEBUG_CNTXTX0MSB_REG_OFFSET },
    0,
    0,
    75,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_RX_DEBUG_CNTXTX0MSB_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_DEBUG_CNTXTX1LSB, TYPE: Type_BBH_RX_BBHRX_DEBUG_CNTXTX1LSB
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: INREASS *****/
const ru_field_rec BBH_RX_DEBUG_CNTXTX1LSB_INREASS_FIELD =
{
    "INREASS",
#if RU_INCLUDE_DESC
    "",
    "In reassembly.\nNot relevant for Ethernet.\n",
#endif
    { BBH_RX_DEBUG_CNTXTX1LSB_INREASS_FIELD_MASK },
    0,
    { BBH_RX_DEBUG_CNTXTX1LSB_INREASS_FIELD_WIDTH },
    { BBH_RX_DEBUG_CNTXTX1LSB_INREASS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FLOWID *****/
const ru_field_rec BBH_RX_DEBUG_CNTXTX1LSB_FLOWID_FIELD =
{
    "FLOWID",
#if RU_INCLUDE_DESC
    "",
    "Flow ID\n",
#endif
    { BBH_RX_DEBUG_CNTXTX1LSB_FLOWID_FIELD_MASK },
    0,
    { BBH_RX_DEBUG_CNTXTX1LSB_FLOWID_FIELD_WIDTH },
    { BBH_RX_DEBUG_CNTXTX1LSB_FLOWID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CUROFFSET *****/
const ru_field_rec BBH_RX_DEBUG_CNTXTX1LSB_CUROFFSET_FIELD =
{
    "CUROFFSET",
#if RU_INCLUDE_DESC
    "",
    "Current offset\n",
#endif
    { BBH_RX_DEBUG_CNTXTX1LSB_CUROFFSET_FIELD_MASK },
    0,
    { BBH_RX_DEBUG_CNTXTX1LSB_CUROFFSET_FIELD_WIDTH },
    { BBH_RX_DEBUG_CNTXTX1LSB_CUROFFSET_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_DEBUG_CNTXTX1LSB_FIELDS[] =
{
    &BBH_RX_DEBUG_CNTXTX1LSB_INREASS_FIELD,
    &BBH_RX_DEBUG_CNTXTX1LSB_FLOWID_FIELD,
    &BBH_RX_DEBUG_CNTXTX1LSB_CUROFFSET_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_DEBUG_CNTXTX1LSB *****/
const ru_reg_rec BBH_RX_DEBUG_CNTXTX1LSB_REG =
{
    "DEBUG_CNTXTX1LSB",
#if RU_INCLUDE_DESC
    "CONTEXT_1_LSB Register",
    "In the case of GPON peripheral, DS flows may arrive interleaved. The BBH supports parallel reassembly of up to two interleaved flows (out of 256). For the reassembly process the BBH stores a double flow context.\n",
#endif
    { BBH_RX_DEBUG_CNTXTX1LSB_REG_OFFSET },
    0,
    0,
    76,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    BBH_RX_DEBUG_CNTXTX1LSB_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_DEBUG_CNTXTX1MSB, TYPE: Type_BBH_RX_BBHRX_DEBUG_CNTXTX1MSB
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CURBN *****/
const ru_field_rec BBH_RX_DEBUG_CNTXTX1MSB_CURBN_FIELD =
{
    "CURBN",
#if RU_INCLUDE_DESC
    "",
    "Current BN\n",
#endif
    { BBH_RX_DEBUG_CNTXTX1MSB_CURBN_FIELD_MASK },
    0,
    { BBH_RX_DEBUG_CNTXTX1MSB_CURBN_FIELD_WIDTH },
    { BBH_RX_DEBUG_CNTXTX1MSB_CURBN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FIRSTBN *****/
const ru_field_rec BBH_RX_DEBUG_CNTXTX1MSB_FIRSTBN_FIELD =
{
    "FIRSTBN",
#if RU_INCLUDE_DESC
    "",
    "First BN\n",
#endif
    { BBH_RX_DEBUG_CNTXTX1MSB_FIRSTBN_FIELD_MASK },
    0,
    { BBH_RX_DEBUG_CNTXTX1MSB_FIRSTBN_FIELD_WIDTH },
    { BBH_RX_DEBUG_CNTXTX1MSB_FIRSTBN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_DEBUG_CNTXTX1MSB_FIELDS[] =
{
    &BBH_RX_DEBUG_CNTXTX1MSB_CURBN_FIELD,
    &BBH_RX_DEBUG_CNTXTX1MSB_FIRSTBN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_DEBUG_CNTXTX1MSB *****/
const ru_reg_rec BBH_RX_DEBUG_CNTXTX1MSB_REG =
{
    "DEBUG_CNTXTX1MSB",
#if RU_INCLUDE_DESC
    "CONTEXT_1_MSB Register",
    "In the case of GPON peripheral, DS flows may arrive interleaved. The BBH supports parallel reassembly of up to two interleaved flows (out of 256). For the reassembly process the BBH stores a double flow context.\n",
#endif
    { BBH_RX_DEBUG_CNTXTX1MSB_REG_OFFSET },
    0,
    0,
    77,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_RX_DEBUG_CNTXTX1MSB_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_DEBUG_CNTXTX0INGRESS, TYPE: Type_BBH_RX_BBHRX_DEBUG_CNTXTX0INGRESS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: INREASS *****/
const ru_field_rec BBH_RX_DEBUG_CNTXTX0INGRESS_INREASS_FIELD =
{
    "INREASS",
#if RU_INCLUDE_DESC
    "",
    "In reassembly.\nNot relevant for Ethernet.\n",
#endif
    { BBH_RX_DEBUG_CNTXTX0INGRESS_INREASS_FIELD_MASK },
    0,
    { BBH_RX_DEBUG_CNTXTX0INGRESS_INREASS_FIELD_WIDTH },
    { BBH_RX_DEBUG_CNTXTX0INGRESS_INREASS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SOP *****/
const ru_field_rec BBH_RX_DEBUG_CNTXTX0INGRESS_SOP_FIELD =
{
    "SOP",
#if RU_INCLUDE_DESC
    "",
    "SOP\n",
#endif
    { BBH_RX_DEBUG_CNTXTX0INGRESS_SOP_FIELD_MASK },
    0,
    { BBH_RX_DEBUG_CNTXTX0INGRESS_SOP_FIELD_WIDTH },
    { BBH_RX_DEBUG_CNTXTX0INGRESS_SOP_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PRIORITY *****/
const ru_field_rec BBH_RX_DEBUG_CNTXTX0INGRESS_PRIORITY_FIELD =
{
    "PRIORITY",
#if RU_INCLUDE_DESC
    "",
    "Priority\n",
#endif
    { BBH_RX_DEBUG_CNTXTX0INGRESS_PRIORITY_FIELD_MASK },
    0,
    { BBH_RX_DEBUG_CNTXTX0INGRESS_PRIORITY_FIELD_WIDTH },
    { BBH_RX_DEBUG_CNTXTX0INGRESS_PRIORITY_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FLOWID *****/
const ru_field_rec BBH_RX_DEBUG_CNTXTX0INGRESS_FLOWID_FIELD =
{
    "FLOWID",
#if RU_INCLUDE_DESC
    "",
    "Flow ID\n",
#endif
    { BBH_RX_DEBUG_CNTXTX0INGRESS_FLOWID_FIELD_MASK },
    0,
    { BBH_RX_DEBUG_CNTXTX0INGRESS_FLOWID_FIELD_WIDTH },
    { BBH_RX_DEBUG_CNTXTX0INGRESS_FLOWID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CUROFFSET *****/
const ru_field_rec BBH_RX_DEBUG_CNTXTX0INGRESS_CUROFFSET_FIELD =
{
    "CUROFFSET",
#if RU_INCLUDE_DESC
    "",
    "Current offset\n",
#endif
    { BBH_RX_DEBUG_CNTXTX0INGRESS_CUROFFSET_FIELD_MASK },
    0,
    { BBH_RX_DEBUG_CNTXTX0INGRESS_CUROFFSET_FIELD_WIDTH },
    { BBH_RX_DEBUG_CNTXTX0INGRESS_CUROFFSET_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_DEBUG_CNTXTX0INGRESS_FIELDS[] =
{
    &BBH_RX_DEBUG_CNTXTX0INGRESS_INREASS_FIELD,
    &BBH_RX_DEBUG_CNTXTX0INGRESS_SOP_FIELD,
    &BBH_RX_DEBUG_CNTXTX0INGRESS_PRIORITY_FIELD,
    &BBH_RX_DEBUG_CNTXTX0INGRESS_FLOWID_FIELD,
    &BBH_RX_DEBUG_CNTXTX0INGRESS_CUROFFSET_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_DEBUG_CNTXTX0INGRESS *****/
const ru_reg_rec BBH_RX_DEBUG_CNTXTX0INGRESS_REG =
{
    "DEBUG_CNTXTX0INGRESS",
#if RU_INCLUDE_DESC
    "INGRESS_CONTEXT_0 Register",
    "In the case of GPON peripheral, DS flows may arrive interleaved. The BBH supports parallel reassembly of up to two interleaved flows (out of 256). For the reassembly process the BBH stores a double flow context.\n",
#endif
    { BBH_RX_DEBUG_CNTXTX0INGRESS_REG_OFFSET },
    0,
    0,
    78,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    BBH_RX_DEBUG_CNTXTX0INGRESS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_DEBUG_CNTXTX1INGRESS, TYPE: Type_BBH_RX_BBHRX_DEBUG_CNTXTX1INGRESS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: INREASS *****/
const ru_field_rec BBH_RX_DEBUG_CNTXTX1INGRESS_INREASS_FIELD =
{
    "INREASS",
#if RU_INCLUDE_DESC
    "",
    "In reassembly.\nNot relevant for Ethernet.\n",
#endif
    { BBH_RX_DEBUG_CNTXTX1INGRESS_INREASS_FIELD_MASK },
    0,
    { BBH_RX_DEBUG_CNTXTX1INGRESS_INREASS_FIELD_WIDTH },
    { BBH_RX_DEBUG_CNTXTX1INGRESS_INREASS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SOP *****/
const ru_field_rec BBH_RX_DEBUG_CNTXTX1INGRESS_SOP_FIELD =
{
    "SOP",
#if RU_INCLUDE_DESC
    "",
    "SOP\n",
#endif
    { BBH_RX_DEBUG_CNTXTX1INGRESS_SOP_FIELD_MASK },
    0,
    { BBH_RX_DEBUG_CNTXTX1INGRESS_SOP_FIELD_WIDTH },
    { BBH_RX_DEBUG_CNTXTX1INGRESS_SOP_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PRIORITY *****/
const ru_field_rec BBH_RX_DEBUG_CNTXTX1INGRESS_PRIORITY_FIELD =
{
    "PRIORITY",
#if RU_INCLUDE_DESC
    "",
    "Priority\n",
#endif
    { BBH_RX_DEBUG_CNTXTX1INGRESS_PRIORITY_FIELD_MASK },
    0,
    { BBH_RX_DEBUG_CNTXTX1INGRESS_PRIORITY_FIELD_WIDTH },
    { BBH_RX_DEBUG_CNTXTX1INGRESS_PRIORITY_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FLOWID *****/
const ru_field_rec BBH_RX_DEBUG_CNTXTX1INGRESS_FLOWID_FIELD =
{
    "FLOWID",
#if RU_INCLUDE_DESC
    "",
    "Flow ID\n",
#endif
    { BBH_RX_DEBUG_CNTXTX1INGRESS_FLOWID_FIELD_MASK },
    0,
    { BBH_RX_DEBUG_CNTXTX1INGRESS_FLOWID_FIELD_WIDTH },
    { BBH_RX_DEBUG_CNTXTX1INGRESS_FLOWID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CUROFFSET *****/
const ru_field_rec BBH_RX_DEBUG_CNTXTX1INGRESS_CUROFFSET_FIELD =
{
    "CUROFFSET",
#if RU_INCLUDE_DESC
    "",
    "Current offset\n",
#endif
    { BBH_RX_DEBUG_CNTXTX1INGRESS_CUROFFSET_FIELD_MASK },
    0,
    { BBH_RX_DEBUG_CNTXTX1INGRESS_CUROFFSET_FIELD_WIDTH },
    { BBH_RX_DEBUG_CNTXTX1INGRESS_CUROFFSET_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_DEBUG_CNTXTX1INGRESS_FIELDS[] =
{
    &BBH_RX_DEBUG_CNTXTX1INGRESS_INREASS_FIELD,
    &BBH_RX_DEBUG_CNTXTX1INGRESS_SOP_FIELD,
    &BBH_RX_DEBUG_CNTXTX1INGRESS_PRIORITY_FIELD,
    &BBH_RX_DEBUG_CNTXTX1INGRESS_FLOWID_FIELD,
    &BBH_RX_DEBUG_CNTXTX1INGRESS_CUROFFSET_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_DEBUG_CNTXTX1INGRESS *****/
const ru_reg_rec BBH_RX_DEBUG_CNTXTX1INGRESS_REG =
{
    "DEBUG_CNTXTX1INGRESS",
#if RU_INCLUDE_DESC
    "INGRESS_CONTEXT_1 Register",
    "In the case of GPON peripheral, DS flows may arrive interleaved. The BBH supports parallel reassembly of up to two interleaved flows (out of 256). For the reassembly process the BBH stores a double flow context.\n",
#endif
    { BBH_RX_DEBUG_CNTXTX1INGRESS_REG_OFFSET },
    0,
    0,
    79,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    BBH_RX_DEBUG_CNTXTX1INGRESS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_DEBUG_IBUW, TYPE: Type_BBH_RX_BBHRX_DEBUG_IBUW
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: UW *****/
const ru_field_rec BBH_RX_DEBUG_IBUW_UW_FIELD =
{
    "UW",
#if RU_INCLUDE_DESC
    "",
    "Used words\n",
#endif
    { BBH_RX_DEBUG_IBUW_UW_FIELD_MASK },
    0,
    { BBH_RX_DEBUG_IBUW_UW_FIELD_WIDTH },
    { BBH_RX_DEBUG_IBUW_UW_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_DEBUG_IBUW_FIELDS[] =
{
    &BBH_RX_DEBUG_IBUW_UW_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_DEBUG_IBUW *****/
const ru_reg_rec BBH_RX_DEBUG_IBUW_REG =
{
    "DEBUG_IBUW",
#if RU_INCLUDE_DESC
    "INPUT_BUF_USED_WORDS Register",
    "Input buf used words\n",
#endif
    { BBH_RX_DEBUG_IBUW_REG_OFFSET },
    0,
    0,
    80,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_DEBUG_IBUW_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_DEBUG_BBUW, TYPE: Type_BBH_RX_BBHRX_DEBUG_BBUW
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: UW *****/
const ru_field_rec BBH_RX_DEBUG_BBUW_UW_FIELD =
{
    "UW",
#if RU_INCLUDE_DESC
    "",
    "Used words\n",
#endif
    { BBH_RX_DEBUG_BBUW_UW_FIELD_MASK },
    0,
    { BBH_RX_DEBUG_BBUW_UW_FIELD_WIDTH },
    { BBH_RX_DEBUG_BBUW_UW_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_DEBUG_BBUW_FIELDS[] =
{
    &BBH_RX_DEBUG_BBUW_UW_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_DEBUG_BBUW *****/
const ru_reg_rec BBH_RX_DEBUG_BBUW_REG =
{
    "DEBUG_BBUW",
#if RU_INCLUDE_DESC
    "BURST_BUF_USED_WORDS Register",
    "Burst buf used words\n",
#endif
    { BBH_RX_DEBUG_BBUW_REG_OFFSET },
    0,
    0,
    81,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_DEBUG_BBUW_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_DEBUG_CFUW, TYPE: Type_BBH_RX_BBHRX_DEBUG_CFUW
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: UW *****/
const ru_field_rec BBH_RX_DEBUG_CFUW_UW_FIELD =
{
    "UW",
#if RU_INCLUDE_DESC
    "",
    "Used words\n",
#endif
    { BBH_RX_DEBUG_CFUW_UW_FIELD_MASK },
    0,
    { BBH_RX_DEBUG_CFUW_UW_FIELD_WIDTH },
    { BBH_RX_DEBUG_CFUW_UW_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_DEBUG_CFUW_FIELDS[] =
{
    &BBH_RX_DEBUG_CFUW_UW_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_DEBUG_CFUW *****/
const ru_reg_rec BBH_RX_DEBUG_CFUW_REG =
{
    "DEBUG_CFUW",
#if RU_INCLUDE_DESC
    "COHERENCY_FIFO_USED_WORDS Register",
    "Coherency FIFO used words\n",
#endif
    { BBH_RX_DEBUG_CFUW_REG_OFFSET },
    0,
    0,
    82,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_DEBUG_CFUW_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_DEBUG_ACKCNT, TYPE: Type_BBH_RX_BBHRX_DEBUG_ACKCNT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: SDMA *****/
const ru_field_rec BBH_RX_DEBUG_ACKCNT_SDMA_FIELD =
{
    "SDMA",
#if RU_INCLUDE_DESC
    "",
    "SDMA ACK counter\n",
#endif
    { BBH_RX_DEBUG_ACKCNT_SDMA_FIELD_MASK },
    0,
    { BBH_RX_DEBUG_ACKCNT_SDMA_FIELD_WIDTH },
    { BBH_RX_DEBUG_ACKCNT_SDMA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CONNECT *****/
const ru_field_rec BBH_RX_DEBUG_ACKCNT_CONNECT_FIELD =
{
    "CONNECT",
#if RU_INCLUDE_DESC
    "",
    "Connect ACK counter\n",
#endif
    { BBH_RX_DEBUG_ACKCNT_CONNECT_FIELD_MASK },
    0,
    { BBH_RX_DEBUG_ACKCNT_CONNECT_FIELD_WIDTH },
    { BBH_RX_DEBUG_ACKCNT_CONNECT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_DEBUG_ACKCNT_FIELDS[] =
{
    &BBH_RX_DEBUG_ACKCNT_SDMA_FIELD,
    &BBH_RX_DEBUG_ACKCNT_CONNECT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_DEBUG_ACKCNT *****/
const ru_reg_rec BBH_RX_DEBUG_ACKCNT_REG =
{
    "DEBUG_ACKCNT",
#if RU_INCLUDE_DESC
    "ACK_COUNTERS Register",
    "The register reflects 2 ACK counters:\nSDMA\nCONNECT\n",
#endif
    { BBH_RX_DEBUG_ACKCNT_REG_OFFSET },
    0,
    0,
    83,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_RX_DEBUG_ACKCNT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_DEBUG_COHERENCYCNT, TYPE: Type_BBH_RX_BBHRX_DEBUG_COHERENCYCNT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: NORMAL *****/
const ru_field_rec BBH_RX_DEBUG_COHERENCYCNT_NORMAL_FIELD =
{
    "NORMAL",
#if RU_INCLUDE_DESC
    "",
    "Normal\n",
#endif
    { BBH_RX_DEBUG_COHERENCYCNT_NORMAL_FIELD_MASK },
    0,
    { BBH_RX_DEBUG_COHERENCYCNT_NORMAL_FIELD_WIDTH },
    { BBH_RX_DEBUG_COHERENCYCNT_NORMAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EXCLUSIVE *****/
const ru_field_rec BBH_RX_DEBUG_COHERENCYCNT_EXCLUSIVE_FIELD =
{
    "EXCLUSIVE",
#if RU_INCLUDE_DESC
    "",
    "Exclusive\n",
#endif
    { BBH_RX_DEBUG_COHERENCYCNT_EXCLUSIVE_FIELD_MASK },
    0,
    { BBH_RX_DEBUG_COHERENCYCNT_EXCLUSIVE_FIELD_WIDTH },
    { BBH_RX_DEBUG_COHERENCYCNT_EXCLUSIVE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_DEBUG_COHERENCYCNT_FIELDS[] =
{
    &BBH_RX_DEBUG_COHERENCYCNT_NORMAL_FIELD,
    &BBH_RX_DEBUG_COHERENCYCNT_EXCLUSIVE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_DEBUG_COHERENCYCNT *****/
const ru_reg_rec BBH_RX_DEBUG_COHERENCYCNT_REG =
{
    "DEBUG_COHERENCYCNT",
#if RU_INCLUDE_DESC
    "COHERENCY_COUNTERS Register",
    "The register 2 pending coherency counters:\nNormal\nExclusive\n",
#endif
    { BBH_RX_DEBUG_COHERENCYCNT_REG_OFFSET },
    0,
    0,
    84,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_RX_DEBUG_COHERENCYCNT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_DEBUG_DBGVEC, TYPE: Type_BBH_RX_BBHRX_DEBUG_DBGVEC
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DBGVEC *****/
const ru_field_rec BBH_RX_DEBUG_DBGVEC_DBGVEC_FIELD =
{
    "DBGVEC",
#if RU_INCLUDE_DESC
    "",
    "selected debug vector\n",
#endif
    { BBH_RX_DEBUG_DBGVEC_DBGVEC_FIELD_MASK },
    0,
    { BBH_RX_DEBUG_DBGVEC_DBGVEC_FIELD_WIDTH },
    { BBH_RX_DEBUG_DBGVEC_DBGVEC_FIELD_SHIFT },
    69632,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_DEBUG_DBGVEC_FIELDS[] =
{
    &BBH_RX_DEBUG_DBGVEC_DBGVEC_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_DEBUG_DBGVEC *****/
const ru_reg_rec BBH_RX_DEBUG_DBGVEC_REG =
{
    "DEBUG_DBGVEC",
#if RU_INCLUDE_DESC
    "DEBUG_VECTOR Register",
    "selected debug vector\n",
#endif
    { BBH_RX_DEBUG_DBGVEC_REG_OFFSET },
    0,
    0,
    85,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_DEBUG_DBGVEC_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_DEBUG_UFUW, TYPE: Type_BBH_RX_BBHRX_DEBUG_UFUW
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: UW *****/
const ru_field_rec BBH_RX_DEBUG_UFUW_UW_FIELD =
{
    "UW",
#if RU_INCLUDE_DESC
    "",
    "Used words\n",
#endif
    { BBH_RX_DEBUG_UFUW_UW_FIELD_MASK },
    0,
    { BBH_RX_DEBUG_UFUW_UW_FIELD_WIDTH },
    { BBH_RX_DEBUG_UFUW_UW_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_DEBUG_UFUW_FIELDS[] =
{
    &BBH_RX_DEBUG_UFUW_UW_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_DEBUG_UFUW *****/
const ru_reg_rec BBH_RX_DEBUG_UFUW_REG =
{
    "DEBUG_UFUW",
#if RU_INCLUDE_DESC
    "UPLOAD_FIFO_USED_WORDS Register",
    "Upload FIFO used words\n",
#endif
    { BBH_RX_DEBUG_UFUW_REG_OFFSET },
    0,
    0,
    86,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_DEBUG_UFUW_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_DEBUG_CREDITCNT, TYPE: Type_BBH_RX_BBHRX_DEBUG_CREDITCNT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: NORMAL *****/
const ru_field_rec BBH_RX_DEBUG_CREDITCNT_NORMAL_FIELD =
{
    "NORMAL",
#if RU_INCLUDE_DESC
    "",
    "Normal\n",
#endif
    { BBH_RX_DEBUG_CREDITCNT_NORMAL_FIELD_MASK },
    0,
    { BBH_RX_DEBUG_CREDITCNT_NORMAL_FIELD_WIDTH },
    { BBH_RX_DEBUG_CREDITCNT_NORMAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EXCLUSIVE *****/
const ru_field_rec BBH_RX_DEBUG_CREDITCNT_EXCLUSIVE_FIELD =
{
    "EXCLUSIVE",
#if RU_INCLUDE_DESC
    "",
    "Exclusive\n",
#endif
    { BBH_RX_DEBUG_CREDITCNT_EXCLUSIVE_FIELD_MASK },
    0,
    { BBH_RX_DEBUG_CREDITCNT_EXCLUSIVE_FIELD_WIDTH },
    { BBH_RX_DEBUG_CREDITCNT_EXCLUSIVE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_DEBUG_CREDITCNT_FIELDS[] =
{
    &BBH_RX_DEBUG_CREDITCNT_NORMAL_FIELD,
    &BBH_RX_DEBUG_CREDITCNT_EXCLUSIVE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_DEBUG_CREDITCNT *****/
const ru_reg_rec BBH_RX_DEBUG_CREDITCNT_REG =
{
    "DEBUG_CREDITCNT",
#if RU_INCLUDE_DESC
    "CREDIT_COUNTERS Register",
    "This register holds 2 credit counters:\nNormal\nExclusive\n",
#endif
    { BBH_RX_DEBUG_CREDITCNT_REG_OFFSET },
    0,
    0,
    87,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_RX_DEBUG_CREDITCNT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_DEBUG_SDMACNT, TYPE: Type_BBH_RX_BBHRX_DEBUG_SDMACNT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: UCD *****/
const ru_field_rec BBH_RX_DEBUG_SDMACNT_UCD_FIELD =
{
    "UCD",
#if RU_INCLUDE_DESC
    "",
    "Used CDs\n",
#endif
    { BBH_RX_DEBUG_SDMACNT_UCD_FIELD_MASK },
    0,
    { BBH_RX_DEBUG_SDMACNT_UCD_FIELD_WIDTH },
    { BBH_RX_DEBUG_SDMACNT_UCD_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_DEBUG_SDMACNT_FIELDS[] =
{
    &BBH_RX_DEBUG_SDMACNT_UCD_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_DEBUG_SDMACNT *****/
const ru_reg_rec BBH_RX_DEBUG_SDMACNT_REG =
{
    "DEBUG_SDMACNT",
#if RU_INCLUDE_DESC
    "USED_SDMA_CD_CNT Register",
    "Number of used SDMA CDs\n",
#endif
    { BBH_RX_DEBUG_SDMACNT_REG_OFFSET },
    0,
    0,
    88,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_DEBUG_SDMACNT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_DEBUG_CMFUW, TYPE: Type_BBH_RX_BBHRX_DEBUG_CMFUW
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: UW *****/
const ru_field_rec BBH_RX_DEBUG_CMFUW_UW_FIELD =
{
    "UW",
#if RU_INCLUDE_DESC
    "",
    "Used words\n",
#endif
    { BBH_RX_DEBUG_CMFUW_UW_FIELD_MASK },
    0,
    { BBH_RX_DEBUG_CMFUW_UW_FIELD_WIDTH },
    { BBH_RX_DEBUG_CMFUW_UW_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_DEBUG_CMFUW_FIELDS[] =
{
    &BBH_RX_DEBUG_CMFUW_UW_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_DEBUG_CMFUW *****/
const ru_reg_rec BBH_RX_DEBUG_CMFUW_REG =
{
    "DEBUG_CMFUW",
#if RU_INCLUDE_DESC
    "CMD_FIFO_USED_WORDS Register",
    "CMD FIFO used words\n",
#endif
    { BBH_RX_DEBUG_CMFUW_REG_OFFSET },
    0,
    0,
    89,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_DEBUG_CMFUW_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_DEBUG_SBNFIFO, TYPE: Type_BBH_RX_BBHRX_DEBUG_SBNFIFO
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: BNENTRY *****/
const ru_field_rec BBH_RX_DEBUG_SBNFIFO_BNENTRY_FIELD =
{
    "BNENTRY",
#if RU_INCLUDE_DESC
    "",
    "BN\n",
#endif
    { BBH_RX_DEBUG_SBNFIFO_BNENTRY_FIELD_MASK },
    0,
    { BBH_RX_DEBUG_SBNFIFO_BNENTRY_FIELD_WIDTH },
    { BBH_RX_DEBUG_SBNFIFO_BNENTRY_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: VALID *****/
const ru_field_rec BBH_RX_DEBUG_SBNFIFO_VALID_FIELD =
{
    "VALID",
#if RU_INCLUDE_DESC
    "",
    "SBN is Valid\n",
#endif
    { BBH_RX_DEBUG_SBNFIFO_VALID_FIELD_MASK },
    0,
    { BBH_RX_DEBUG_SBNFIFO_VALID_FIELD_WIDTH },
    { BBH_RX_DEBUG_SBNFIFO_VALID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_DEBUG_SBNFIFO_FIELDS[] =
{
    &BBH_RX_DEBUG_SBNFIFO_BNENTRY_FIELD,
    &BBH_RX_DEBUG_SBNFIFO_VALID_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_DEBUG_SBNFIFO *****/
const ru_reg_rec BBH_RX_DEBUG_SBNFIFO_REG =
{
    "DEBUG_SBNFIFO",
#if RU_INCLUDE_DESC
    "SRAM_BN_FIFO 0..15 Register",
    "The BBH RX hold a FIFO with 16 BN.\n",
#endif
    { BBH_RX_DEBUG_SBNFIFO_REG_OFFSET },
    BBH_RX_DEBUG_SBNFIFO_REG_RAM_CNT,
    4,
    90,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_RX_DEBUG_SBNFIFO_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_DEBUG_CMDFIFO, TYPE: Type_BBH_RX_BBHRX_DEBUG_CMDFIFO
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CMDENTRY *****/
const ru_field_rec BBH_RX_DEBUG_CMDFIFO_CMDENTRY_FIELD =
{
    "CMDENTRY",
#if RU_INCLUDE_DESC
    "",
    "CMD\n",
#endif
    { BBH_RX_DEBUG_CMDFIFO_CMDENTRY_FIELD_MASK },
    0,
    { BBH_RX_DEBUG_CMDFIFO_CMDENTRY_FIELD_WIDTH },
    { BBH_RX_DEBUG_CMDFIFO_CMDENTRY_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_DEBUG_CMDFIFO_FIELDS[] =
{
    &BBH_RX_DEBUG_CMDFIFO_CMDENTRY_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_DEBUG_CMDFIFO *****/
const ru_reg_rec BBH_RX_DEBUG_CMDFIFO_REG =
{
    "DEBUG_CMDFIFO",
#if RU_INCLUDE_DESC
    "CMD_FIFO 0..3 Register",
    "The BBH RX hold a FIFO with 8 command.\n",
#endif
    { BBH_RX_DEBUG_CMDFIFO_REG_OFFSET },
    BBH_RX_DEBUG_CMDFIFO_REG_RAM_CNT,
    4,
    91,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_DEBUG_CMDFIFO_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_DEBUG_SBNRECYCLEFIFO, TYPE: Type_BBH_RX_BBHRX_DEBUG_SBNRECYCLEFIFO
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: BNENTRY *****/
const ru_field_rec BBH_RX_DEBUG_SBNRECYCLEFIFO_BNENTRY_FIELD =
{
    "BNENTRY",
#if RU_INCLUDE_DESC
    "",
    "BN\n",
#endif
    { BBH_RX_DEBUG_SBNRECYCLEFIFO_BNENTRY_FIELD_MASK },
    0,
    { BBH_RX_DEBUG_SBNRECYCLEFIFO_BNENTRY_FIELD_WIDTH },
    { BBH_RX_DEBUG_SBNRECYCLEFIFO_BNENTRY_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: VALID *****/
const ru_field_rec BBH_RX_DEBUG_SBNRECYCLEFIFO_VALID_FIELD =
{
    "VALID",
#if RU_INCLUDE_DESC
    "",
    "SBN is Valid\n",
#endif
    { BBH_RX_DEBUG_SBNRECYCLEFIFO_VALID_FIELD_MASK },
    0,
    { BBH_RX_DEBUG_SBNRECYCLEFIFO_VALID_FIELD_WIDTH },
    { BBH_RX_DEBUG_SBNRECYCLEFIFO_VALID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_DEBUG_SBNRECYCLEFIFO_FIELDS[] =
{
    &BBH_RX_DEBUG_SBNRECYCLEFIFO_BNENTRY_FIELD,
    &BBH_RX_DEBUG_SBNRECYCLEFIFO_VALID_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_DEBUG_SBNRECYCLEFIFO *****/
const ru_reg_rec BBH_RX_DEBUG_SBNRECYCLEFIFO_REG =
{
    "DEBUG_SBNRECYCLEFIFO",
#if RU_INCLUDE_DESC
    "SRAM_BN_RECYCLE_FIFO 0..1 Register",
    "The BBH RX hold a recycle FIFO with up to 2 BN.\n",
#endif
    { BBH_RX_DEBUG_SBNRECYCLEFIFO_REG_OFFSET },
    BBH_RX_DEBUG_SBNRECYCLEFIFO_REG_RAM_CNT,
    4,
    92,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_RX_DEBUG_SBNRECYCLEFIFO_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_DEBUG_COHERENCYCNT2, TYPE: Type_BBH_RX_BBHRX_DEBUG_COHERENCYCNT2
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CDSENT *****/
const ru_field_rec BBH_RX_DEBUG_COHERENCYCNT2_CDSENT_FIELD =
{
    "CDSENT",
#if RU_INCLUDE_DESC
    "",
    "CD sent\n",
#endif
    { BBH_RX_DEBUG_COHERENCYCNT2_CDSENT_FIELD_MASK },
    0,
    { BBH_RX_DEBUG_COHERENCYCNT2_CDSENT_FIELD_WIDTH },
    { BBH_RX_DEBUG_COHERENCYCNT2_CDSENT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ACKRECEIVED *****/
const ru_field_rec BBH_RX_DEBUG_COHERENCYCNT2_ACKRECEIVED_FIELD =
{
    "ACKRECEIVED",
#if RU_INCLUDE_DESC
    "",
    "EOP ACK received\n",
#endif
    { BBH_RX_DEBUG_COHERENCYCNT2_ACKRECEIVED_FIELD_MASK },
    0,
    { BBH_RX_DEBUG_COHERENCYCNT2_ACKRECEIVED_FIELD_WIDTH },
    { BBH_RX_DEBUG_COHERENCYCNT2_ACKRECEIVED_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_DEBUG_COHERENCYCNT2_FIELDS[] =
{
    &BBH_RX_DEBUG_COHERENCYCNT2_CDSENT_FIELD,
    &BBH_RX_DEBUG_COHERENCYCNT2_ACKRECEIVED_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_DEBUG_COHERENCYCNT2 *****/
const ru_reg_rec BBH_RX_DEBUG_COHERENCYCNT2_REG =
{
    "DEBUG_COHERENCYCNT2",
#if RU_INCLUDE_DESC
    "COHERENCY_COUNTERS_METHOD2 Register",
    "Read of 4 coherency counters:\nCD CMD sent (1 per flow)\nEOP ACK received (1 per flow)\n",
#endif
    { BBH_RX_DEBUG_COHERENCYCNT2_REG_OFFSET },
    0,
    0,
    93,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_RX_DEBUG_COHERENCYCNT2_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_DEBUG_DROPSTATUS, TYPE: Type_BBH_RX_BBHRX_DEBUG_DROPSTATUS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DISPSTATUS *****/
const ru_field_rec BBH_RX_DEBUG_DROPSTATUS_DISPSTATUS_FIELD =
{
    "DISPSTATUS",
#if RU_INCLUDE_DESC
    "",
    "Dispatcher drop due to coherency FIFO full.\nWriting 1 to this bit clears it\n\n",
#endif
    { BBH_RX_DEBUG_DROPSTATUS_DISPSTATUS_FIELD_MASK },
    0,
    { BBH_RX_DEBUG_DROPSTATUS_DISPSTATUS_FIELD_WIDTH },
    { BBH_RX_DEBUG_DROPSTATUS_DISPSTATUS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SDMASTATUS *****/
const ru_field_rec BBH_RX_DEBUG_DROPSTATUS_SDMASTATUS_FIELD =
{
    "SDMASTATUS",
#if RU_INCLUDE_DESC
    "",
    "SDMA drop due to coherency method 2 counters over 63 (dec).\nWriting 1 to this bit clears it\n",
#endif
    { BBH_RX_DEBUG_DROPSTATUS_SDMASTATUS_FIELD_MASK },
    0,
    { BBH_RX_DEBUG_DROPSTATUS_SDMASTATUS_FIELD_WIDTH },
    { BBH_RX_DEBUG_DROPSTATUS_SDMASTATUS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FLOWEXCEED *****/
const ru_field_rec BBH_RX_DEBUG_DROPSTATUS_FLOWEXCEED_FIELD =
{
    "FLOWEXCEED",
#if RU_INCLUDE_DESC
    "",
    "Asserted when flow >= 130\nWriting 1 to this bit clears it\n\n",
#endif
    { BBH_RX_DEBUG_DROPSTATUS_FLOWEXCEED_FIELD_MASK },
    0,
    { BBH_RX_DEBUG_DROPSTATUS_FLOWEXCEED_FIELD_WIDTH },
    { BBH_RX_DEBUG_DROPSTATUS_FLOWEXCEED_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FLOWFULL *****/
const ru_field_rec BBH_RX_DEBUG_DROPSTATUS_FLOWFULL_FIELD =
{
    "FLOWFULL",
#if RU_INCLUDE_DESC
    "",
    "Asserted when flow drop pre-fifo in full.\nWriting 1 to this bit clears it\n\n",
#endif
    { BBH_RX_DEBUG_DROPSTATUS_FLOWFULL_FIELD_MASK },
    0,
    { BBH_RX_DEBUG_DROPSTATUS_FLOWFULL_FIELD_WIDTH },
    { BBH_RX_DEBUG_DROPSTATUS_FLOWFULL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_DEBUG_DROPSTATUS_FIELDS[] =
{
    &BBH_RX_DEBUG_DROPSTATUS_DISPSTATUS_FIELD,
    &BBH_RX_DEBUG_DROPSTATUS_SDMASTATUS_FIELD,
    &BBH_RX_DEBUG_DROPSTATUS_FLOWEXCEED_FIELD,
    &BBH_RX_DEBUG_DROPSTATUS_FLOWFULL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_DEBUG_DROPSTATUS *****/
const ru_reg_rec BBH_RX_DEBUG_DROPSTATUS_REG =
{
    "DEBUG_DROPSTATUS",
#if RU_INCLUDE_DESC
    "SPECIAL_DROP_STATUS Register",
    "Information of the following:\n- Dispatcher drop due to coherency FIFO full\n- SDMA drop due to coherency method 2 counters over 63 (dec)\n\n",
#endif
    { BBH_RX_DEBUG_DROPSTATUS_REG_OFFSET },
    0,
    0,
    94,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    BBH_RX_DEBUG_DROPSTATUS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_WAN_FLOW_COUNTERS_GEMCTRINIT, TYPE: Type_BBH_RX_BBHRX_WAN_FLOW_COUNTERS_GEMCTRINIT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: INIT *****/
const ru_field_rec BBH_RX_WAN_FLOW_COUNTERS_GEMCTRINIT_INIT_FIELD =
{
    "INIT",
#if RU_INCLUDE_DESC
    "",
    "when asserted, the HW will start initializing the counters with value of 0. Should not be done during traffic.\n",
#endif
    { BBH_RX_WAN_FLOW_COUNTERS_GEMCTRINIT_INIT_FIELD_MASK },
    0,
    { BBH_RX_WAN_FLOW_COUNTERS_GEMCTRINIT_INIT_FIELD_WIDTH },
    { BBH_RX_WAN_FLOW_COUNTERS_GEMCTRINIT_INIT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: INITDONE *****/
const ru_field_rec BBH_RX_WAN_FLOW_COUNTERS_GEMCTRINIT_INITDONE_FIELD =
{
    "INITDONE",
#if RU_INCLUDE_DESC
    "",
    "Asserted by the HW, this bit indicates the HW finished initializing the counters with value of 0.\n",
#endif
    { BBH_RX_WAN_FLOW_COUNTERS_GEMCTRINIT_INITDONE_FIELD_MASK },
    0,
    { BBH_RX_WAN_FLOW_COUNTERS_GEMCTRINIT_INITDONE_FIELD_WIDTH },
    { BBH_RX_WAN_FLOW_COUNTERS_GEMCTRINIT_INITDONE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_WAN_FLOW_COUNTERS_GEMCTRINIT_FIELDS[] =
{
    &BBH_RX_WAN_FLOW_COUNTERS_GEMCTRINIT_INIT_FIELD,
    &BBH_RX_WAN_FLOW_COUNTERS_GEMCTRINIT_INITDONE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_WAN_FLOW_COUNTERS_GEMCTRINIT *****/
const ru_reg_rec BBH_RX_WAN_FLOW_COUNTERS_GEMCTRINIT_REG =
{
    "WAN_FLOW_COUNTERS_GEMCTRINIT",
#if RU_INCLUDE_DESC
    "GEM_COUNTERS_INIT Register",
    "init configuration\n",
#endif
    { BBH_RX_WAN_FLOW_COUNTERS_GEMCTRINIT_REG_OFFSET },
    0,
    0,
    95,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_RX_WAN_FLOW_COUNTERS_GEMCTRINIT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_WAN_FLOW_COUNTERS_GEMCTRRD, TYPE: Type_BBH_RX_BBHRX_WAN_FLOW_COUNTERS_GEMCTRRD
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RDADDRESS *****/
const ru_field_rec BBH_RX_WAN_FLOW_COUNTERS_GEMCTRRD_RDADDRESS_FIELD =
{
    "RDADDRESS",
#if RU_INCLUDE_DESC
    "",
    "the counter to be read\n",
#endif
    { BBH_RX_WAN_FLOW_COUNTERS_GEMCTRRD_RDADDRESS_FIELD_MASK },
    0,
    { BBH_RX_WAN_FLOW_COUNTERS_GEMCTRRD_RDADDRESS_FIELD_WIDTH },
    { BBH_RX_WAN_FLOW_COUNTERS_GEMCTRRD_RDADDRESS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: RD *****/
const ru_field_rec BBH_RX_WAN_FLOW_COUNTERS_GEMCTRRD_RD_FIELD =
{
    "RD",
#if RU_INCLUDE_DESC
    "",
    "rd\n",
#endif
    { BBH_RX_WAN_FLOW_COUNTERS_GEMCTRRD_RD_FIELD_MASK },
    0,
    { BBH_RX_WAN_FLOW_COUNTERS_GEMCTRRD_RD_FIELD_WIDTH },
    { BBH_RX_WAN_FLOW_COUNTERS_GEMCTRRD_RD_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_WAN_FLOW_COUNTERS_GEMCTRRD_FIELDS[] =
{
    &BBH_RX_WAN_FLOW_COUNTERS_GEMCTRRD_RDADDRESS_FIELD,
    &BBH_RX_WAN_FLOW_COUNTERS_GEMCTRRD_RD_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_WAN_FLOW_COUNTERS_GEMCTRRD *****/
const ru_reg_rec BBH_RX_WAN_FLOW_COUNTERS_GEMCTRRD_REG =
{
    "WAN_FLOW_COUNTERS_GEMCTRRD",
#if RU_INCLUDE_DESC
    "GEM_COUNTERS_RD Register",
    "the GEM counters are a set of 130 dual counters. each pair contains 28bit packet counter and 36 bits of bytes counter. The pair is always being read together.\nThe counters are read clear.\n",
#endif
    { BBH_RX_WAN_FLOW_COUNTERS_GEMCTRRD_REG_OFFSET },
    0,
    0,
    96,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_RX_WAN_FLOW_COUNTERS_GEMCTRRD_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_WAN_FLOW_COUNTERS_GEMCTRRD0, TYPE: Type_BBH_RX_BBHRX_WAN_FLOW_COUNTERS_GEMCTRRD0
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RDDATA *****/
const ru_field_rec BBH_RX_WAN_FLOW_COUNTERS_GEMCTRRD0_RDDATA_FIELD =
{
    "RDDATA",
#if RU_INCLUDE_DESC
    "",
    "read data:\npkts count 27:0, byte cnt 35:32\n",
#endif
    { BBH_RX_WAN_FLOW_COUNTERS_GEMCTRRD0_RDDATA_FIELD_MASK },
    0,
    { BBH_RX_WAN_FLOW_COUNTERS_GEMCTRRD0_RDDATA_FIELD_WIDTH },
    { BBH_RX_WAN_FLOW_COUNTERS_GEMCTRRD0_RDDATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_WAN_FLOW_COUNTERS_GEMCTRRD0_FIELDS[] =
{
    &BBH_RX_WAN_FLOW_COUNTERS_GEMCTRRD0_RDDATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_WAN_FLOW_COUNTERS_GEMCTRRD0 *****/
const ru_reg_rec BBH_RX_WAN_FLOW_COUNTERS_GEMCTRRD0_REG =
{
    "WAN_FLOW_COUNTERS_GEMCTRRD0",
#if RU_INCLUDE_DESC
    "GEM_COUTERS_READ_DATA_0 Register",
    "upper 32 bits of the pair:\n{pkt count [27:0], byte count [35:32]}\n",
#endif
    { BBH_RX_WAN_FLOW_COUNTERS_GEMCTRRD0_REG_OFFSET },
    0,
    0,
    97,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_WAN_FLOW_COUNTERS_GEMCTRRD0_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_WAN_FLOW_COUNTERS_GEMCTRRD1, TYPE: Type_BBH_RX_BBHRX_WAN_FLOW_COUNTERS_GEMCTRRD1
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RDDATA *****/
const ru_field_rec BBH_RX_WAN_FLOW_COUNTERS_GEMCTRRD1_RDDATA_FIELD =
{
    "RDDATA",
#if RU_INCLUDE_DESC
    "",
    "read data:\nbyte cnt 31:0\n",
#endif
    { BBH_RX_WAN_FLOW_COUNTERS_GEMCTRRD1_RDDATA_FIELD_MASK },
    0,
    { BBH_RX_WAN_FLOW_COUNTERS_GEMCTRRD1_RDDATA_FIELD_WIDTH },
    { BBH_RX_WAN_FLOW_COUNTERS_GEMCTRRD1_RDDATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_WAN_FLOW_COUNTERS_GEMCTRRD1_FIELDS[] =
{
    &BBH_RX_WAN_FLOW_COUNTERS_GEMCTRRD1_RDDATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_WAN_FLOW_COUNTERS_GEMCTRRD1 *****/
const ru_reg_rec BBH_RX_WAN_FLOW_COUNTERS_GEMCTRRD1_REG =
{
    "WAN_FLOW_COUNTERS_GEMCTRRD1",
#if RU_INCLUDE_DESC
    "GEM_COUTERS_READ_DATA_1 Register",
    "lower 32 bits of the pair:\nbyte count [31:0]\n",
#endif
    { BBH_RX_WAN_FLOW_COUNTERS_GEMCTRRD1_REG_OFFSET },
    0,
    0,
    98,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_WAN_FLOW_COUNTERS_GEMCTRRD1_FIELDS,
#endif
};

unsigned long BBH_RX_ADDRS[] =
{
    0x82898000,
    0x82898400,
    0x82898800,
    0x82898C00,
    0x82899000,
    0x82899400,
    0x82899800,
    0x82899C00,
    0x8289A000,
    0x8289A400,
    0x8289A800,
    0x8289AC00,
    0x8289B000,
    0x80182000,
};

static const ru_reg_rec *BBH_RX_REGS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_BBCFG_REG,
    &BBH_RX_GENERAL_CONFIGURATION_DISPVIQ_REG,
    &BBH_RX_GENERAL_CONFIGURATION_PATTERNDATALSB_REG,
    &BBH_RX_GENERAL_CONFIGURATION_PATTERNDATAMSB_REG,
    &BBH_RX_GENERAL_CONFIGURATION_PATTERNMASKLSB_REG,
    &BBH_RX_GENERAL_CONFIGURATION_PATTERNMASKMSB_REG,
    &BBH_RX_GENERAL_CONFIGURATION_EXCLQCFG_REG,
    &BBH_RX_GENERAL_CONFIGURATION_SDMAADDR_REG,
    &BBH_RX_GENERAL_CONFIGURATION_SDMACFG_REG,
    &BBH_RX_GENERAL_CONFIGURATION_MINPKT0_REG,
    &BBH_RX_GENERAL_CONFIGURATION_MAXPKT0_REG,
    &BBH_RX_GENERAL_CONFIGURATION_MAXPKT1_REG,
    &BBH_RX_GENERAL_CONFIGURATION_SOPOFFSET_REG,
    &BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_REG,
    &BBH_RX_GENERAL_CONFIGURATION_CRCOMITDIS_REG,
    &BBH_RX_GENERAL_CONFIGURATION_ENABLE_REG,
    &BBH_RX_GENERAL_CONFIGURATION_G9991EN_REG,
    &BBH_RX_GENERAL_CONFIGURATION_PERFLOWTH_REG,
    &BBH_RX_GENERAL_CONFIGURATION_PERFLOWSETS_REG,
    &BBH_RX_GENERAL_CONFIGURATION_MINPKTSEL0_REG,
    &BBH_RX_GENERAL_CONFIGURATION_MINPKTSEL1_REG,
    &BBH_RX_GENERAL_CONFIGURATION_MAXPKTSEL0_REG,
    &BBH_RX_GENERAL_CONFIGURATION_MAXPKTSEL1_REG,
    &BBH_RX_GENERAL_CONFIGURATION_MACMODE_REG,
    &BBH_RX_GENERAL_CONFIGURATION_SBPMCFG_REG,
    &BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_REG,
    &BBH_RX_GENERAL_CONFIGURATION_RXDBGSEL_REG,
    &BBH_RX_GENERAL_CONFIGURATION_BBHRX_RADDR_DECODER_REG,
    &BBH_RX_GENERAL_CONFIGURATION_NONETH_REG,
    &BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_REG,
    &BBH_RX_GENERAL_CONFIGURATION_PFCCONTROL_REG,
    &BBH_RX_GENERAL_CONFIGURATION_EPONSEQDIS_REG,
    &BBH_RX_GENERAL_CONFIGURATION_MACFLOW_REG,
    &BBH_RX_PM_COUNTERS_INPKT_REG,
    &BBH_RX_PM_COUNTERS_THIRDFLOW_REG,
    &BBH_RX_PM_COUNTERS_SOPASOP_REG,
    &BBH_RX_PM_COUNTERS_TOOSHORT_REG,
    &BBH_RX_PM_COUNTERS_TOOLONG_REG,
    &BBH_RX_PM_COUNTERS_CRCERROR_REG,
    &BBH_RX_PM_COUNTERS_ENCRYPTERROR_REG,
    &BBH_RX_PM_COUNTERS_DISPCONG_REG,
    &BBH_RX_PM_COUNTERS_NOSBPMSBN_REG,
    &BBH_RX_PM_COUNTERS_NOSDMACD_REG,
    &BBH_RX_PM_COUNTERS_INPLOAM_REG,
    &BBH_RX_PM_COUNTERS_CRCERRORPLOAM_REG,
    &BBH_RX_PM_COUNTERS_DISPCONGPLOAM_REG,
    &BBH_RX_PM_COUNTERS_NOSBPMSBNPLOAM_REG,
    &BBH_RX_PM_COUNTERS_NOSDMACDPLOAM_REG,
    &BBH_RX_PM_COUNTERS_EPONTYPERROR_REG,
    &BBH_RX_PM_COUNTERS_RUNTERROR_REG,
    &BBH_RX_PM_COUNTERS_INBYTE_REG,
    &BBH_RX_PM_COUNTERS_FLOWNOTCOUNTED_REG,
    &BBH_RX_DEBUG_CNTXTX0LSB_REG,
    &BBH_RX_DEBUG_CNTXTX0MSB_REG,
    &BBH_RX_DEBUG_CNTXTX1LSB_REG,
    &BBH_RX_DEBUG_CNTXTX1MSB_REG,
    &BBH_RX_DEBUG_CNTXTX0INGRESS_REG,
    &BBH_RX_DEBUG_CNTXTX1INGRESS_REG,
    &BBH_RX_DEBUG_IBUW_REG,
    &BBH_RX_DEBUG_BBUW_REG,
    &BBH_RX_DEBUG_CFUW_REG,
    &BBH_RX_DEBUG_ACKCNT_REG,
    &BBH_RX_DEBUG_COHERENCYCNT_REG,
    &BBH_RX_DEBUG_DBGVEC_REG,
    &BBH_RX_DEBUG_UFUW_REG,
    &BBH_RX_DEBUG_CREDITCNT_REG,
    &BBH_RX_DEBUG_SDMACNT_REG,
    &BBH_RX_DEBUG_CMFUW_REG,
    &BBH_RX_DEBUG_SBNFIFO_REG,
    &BBH_RX_DEBUG_CMDFIFO_REG,
    &BBH_RX_DEBUG_SBNRECYCLEFIFO_REG,
    &BBH_RX_DEBUG_COHERENCYCNT2_REG,
    &BBH_RX_DEBUG_DROPSTATUS_REG,
    &BBH_RX_WAN_FLOW_COUNTERS_GEMCTRINIT_REG,
    &BBH_RX_WAN_FLOW_COUNTERS_GEMCTRRD_REG,
    &BBH_RX_WAN_FLOW_COUNTERS_GEMCTRRD0_REG,
    &BBH_RX_WAN_FLOW_COUNTERS_GEMCTRRD1_REG,
};

const ru_block_rec BBH_RX_BLOCK =
{
    "BBH_RX",
    BBH_RX_ADDRS,
    14,
    77,
    BBH_RX_REGS,
};
