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
    26,
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
    38,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_BBCFG_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_BBCFG_DISPBBID_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_BBCFG_SBPMBBID_FIELD,
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
    208,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_RX_GENERAL_CONFIGURATION_BBCFG_FIELDS,
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
    "When de-asserted, the BBH will read and discard new fragment/packet from the MAC.\nThe BBH will Gracefully enable/disable (on fragment boundary for N/X/GPON/2 and on packet boundary for the rest).\nBit n refers to PFB channel n.\nTRIGGERS_HW\n\n",
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
    "When de-asserted, the BBH will not pre-fetch SBPM buffers\nTRIGGERS_HW\n\n",
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
/***** Field: GEMINIT *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_ENABLE_GEMINIT_FIELD =
{
    "GEMINIT",
#if RU_INCLUDE_DESC
    "",
    "Initialize the GEM mode memory to all zero.\nSW may check the done bit (separated register) to verify that the init operation is complete.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_ENABLE_GEMINIT_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_ENABLE_GEMINIT_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_ENABLE_GEMINIT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: GRACE_DIS *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_ENABLE_GRACE_DIS_FIELD =
{
    "GRACE_DIS",
#if RU_INCLUDE_DESC
    "",
    "When Packet_enable (PKTEN) is de-asserted (disable BBH RX), according to GRACE_DIS:\n0: complete receiving the current packet and drop all the next ones.\n1: Truncate the correct packet with error and drop all the next ones.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_ENABLE_GRACE_DIS_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_ENABLE_GRACE_DIS_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_ENABLE_GRACE_DIS_FIELD_SHIFT },
    1,
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
    &BBH_RX_GENERAL_CONFIGURATION_ENABLE_GEMINIT_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_ENABLE_GRACE_DIS_FIELD,
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
    209,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    BBH_RX_GENERAL_CONFIGURATION_ENABLE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_GENERAL_CONFIGURATION_MINPKT, TYPE: Type_BBH_RX_BBHRX_GENERAL_CONFIGURATION_MINPKT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: MINPKT_NORMAL *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_MINPKT_MINPKT_NORMAL_FIELD =
{
    "MINPKT_NORMAL",
#if RU_INCLUDE_DESC
    "",
    "Normal packets shorter than this threshold will be discarded.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_MINPKT_MINPKT_NORMAL_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_MINPKT_MINPKT_NORMAL_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_MINPKT_MINPKT_NORMAL_FIELD_SHIFT },
    64,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MINPKT_OMCI *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_MINPKT_MINPKT_OMCI_FIELD =
{
    "MINPKT_OMCI",
#if RU_INCLUDE_DESC
    "",
    "OMCI packets shorter than this threshold will be discarded.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_MINPKT_MINPKT_OMCI_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_MINPKT_MINPKT_OMCI_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_MINPKT_MINPKT_OMCI_FIELD_SHIFT },
    14,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_MINPKT_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_MINPKT_MINPKT_NORMAL_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_MINPKT_MINPKT_OMCI_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_GENERAL_CONFIGURATION_MINPKT *****/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_MINPKT_REG =
{
    "GENERAL_CONFIGURATION_MINPKT",
#if RU_INCLUDE_DESC
    "MINIMUM_PACKET_SIZE Register",
    "Packets shorter than this threshold will be discarded.\n2 global threshold values (not per port) - one for OMCI and one for the rest\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_MINPKT_REG_OFFSET },
    0,
    0,
    210,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_RX_GENERAL_CONFIGURATION_MINPKT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_GENERAL_CONFIGURATION_MAXPKT, TYPE: Type_BBH_RX_BBHRX_GENERAL_CONFIGURATION_MAXPKT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: MAXPKT *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_MAXPKT_MAXPKT_FIELD =
{
    "MAXPKT",
#if RU_INCLUDE_DESC
    "",
    "Packets longer than this threshold will be discarded.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_MAXPKT_MAXPKT_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_MAXPKT_MAXPKT_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_MAXPKT_MAXPKT_FIELD_SHIFT },
    16383,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_MAXPKT_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_MAXPKT_MAXPKT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_GENERAL_CONFIGURATION_MAXPKT *****/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_MAXPKT_REG =
{
    "GENERAL_CONFIGURATION_MAXPKT",
#if RU_INCLUDE_DESC
    "MAXIMUM_PACKET_SIZE Register",
    "Packets longer than this threshold will be discarded.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_MAXPKT_REG_OFFSET },
    0,
    0,
    211,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_GENERAL_CONFIGURATION_MAXPKT_FIELDS,
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
    "If set to 1b1 will disable the clock gate logic such to always enable the clock\nTRIGGERS_HW\n\n",
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
    "For how long should the clock stay active once all conditions for clock disable are met.\nTRIGGERS_HW\n\n",
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
    "Enables the keep alive logic which will periodically enable the clock to assure that no deadlock of clock being removed completely will occur\nTRIGGERS_HW\n\n",
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
    "If the KEEP alive option is enabled the field will determine for how many cycles should the clock be active\nTRIGGERS_HW\n\n",
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
    "If the KEEP alive option is enabled this field will determine for how many cycles should the clock be disabled (minus the KEEP_ALIVE_INTERVAL)\n\nSo KEEP_ALIVE_CYCLE must be larger than KEEP_ALIVE_INTERVAL.\nTRIGGERS_HW\n\n",
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
    212,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_FIELDS,
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
    "Selects one out of the debug vectors\n",
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
    213,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_GENERAL_CONFIGURATION_RXDBGSEL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_GENERAL_CONFIGURATION_RXRSTRST, TYPE: Type_BBH_RX_BBHRX_GENERAL_CONFIGURATION_RXRSTRST
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PORTRST *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_PORTRST_FIELD =
{
    "PORTRST",
#if RU_INCLUDE_DESC
    "",
    "Writing 1 to this register will reset the port module according to PFBCH.\nFor a reset operation the SW should assert and then de-assert this bit.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_PORTRST_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_PORTRST_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_PORTRST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PSRAMRST *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_PSRAMRST_FIELD =
{
    "PSRAMRST",
#if RU_INCLUDE_DESC
    "",
    "Writing 1 to this register will reset the psram module.\nFor a reset operation the SW should assert and then de-assert this bit.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_PSRAMRST_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_PSRAMRST_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_PSRAMRST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PDFIFORST *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_PDFIFORST_FIELD =
{
    "PDFIFORST",
#if RU_INCLUDE_DESC
    "",
    "Writing 1 to this register will reset the PD FIFO module.\nFor a reset operation the SW should assert and then de-assert this bit.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_PDFIFORST_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_PDFIFORST_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_PDFIFORST_FIELD_SHIFT },
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
/***** Field: DISPCREDITRST *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_DISPCREDITRST_FIELD =
{
    "DISPCREDITRST",
#if RU_INCLUDE_DESC
    "",
    "Writing 1 to this register will reset the dispatcher credits of a configured VIQ (DISPVIQ) to a configurable value DISPCREDIT.\nFor a reset operation the SW should assert and then de-assert this bit.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_DISPCREDITRST_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_DISPCREDITRST_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_DISPCREDITRST_FIELD_SHIFT },
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
/***** Field: PFBCH *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_PFBCH_FIELD =
{
    "PFBCH",
#if RU_INCLUDE_DESC
    "",
    "Setting PORTRST will reset the relevant module according to this field.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_PFBCH_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_PFBCH_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_PFBCH_FIELD_SHIFT },
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
    "Setting DISPCREDITRST will init the relevant credit counter according to this field.\n",
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
/***** Field: DISPVIQ *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_DISPVIQ_FIELD =
{
    "DISPVIQ",
#if RU_INCLUDE_DESC
    "",
    "Setting DISPCREDITRST will init the relevant credit counter according to this field.\nThis is a one hot field, which selects one out of 9 VIQ, e.g. a value of 0x4 will init VIQ2.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_DISPVIQ_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_DISPVIQ_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_DISPVIQ_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_PORTRST_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_PSRAMRST_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_PDFIFORST_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_SBPMFIFORST_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_DISPCREDITRST_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_UPLDFIFORST_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_PFBCH_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_DISPCREDIT_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_DISPVIQ_FIELD,
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
    214,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_FIELDS,
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
    32,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_SBPMCFG_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_SBPMCFG_MAXREQ_FIELD,
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
    215,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_GENERAL_CONFIGURATION_SBPMCFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_GENERAL_CONFIGURATION_PSRAMBASE, TYPE: Type_BBH_RX_BBHRX_GENERAL_CONFIGURATION_PSRAMBASE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PSRAMBASE *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_PSRAMBASE_PSRAMBASE_FIELD =
{
    "PSRAMBASE",
#if RU_INCLUDE_DESC
    "",
    "PSRAM UBUS base address.\nThe address is in 1MByte resolution.\nReal address: {PSRAMBASE, 20h0}\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_PSRAMBASE_PSRAMBASE_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_PSRAMBASE_PSRAMBASE_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_PSRAMBASE_PSRAMBASE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_PSRAMBASE_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_PSRAMBASE_PSRAMBASE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_GENERAL_CONFIGURATION_PSRAMBASE *****/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_PSRAMBASE_REG =
{
    "GENERAL_CONFIGURATION_PSRAMBASE",
#if RU_INCLUDE_DESC
    "PSRAM_BASE Register",
    "Each ingress PSRAM has 2 UBUS responders.\nThis register defines the base address of the PSRAM, which actually defines which responder will be accessed.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_PSRAMBASE_REG_OFFSET },
    0,
    0,
    216,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_GENERAL_CONFIGURATION_PSRAMBASE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_GENERAL_CONFIGURATION_INGRESSCHANNEL, TYPE: Type_BBH_RX_BBHRX_GENERAL_CONFIGURATION_INGRESSCHANNEL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: INGRESSCHANNEL *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_INGRESSCHANNEL_INGRESSCHANNEL_FIELD =
{
    "INGRESSCHANNEL",
#if RU_INCLUDE_DESC
    "",
    "BBH RX ingress channel\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_INGRESSCHANNEL_INGRESSCHANNEL_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_INGRESSCHANNEL_INGRESSCHANNEL_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_INGRESSCHANNEL_INGRESSCHANNEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_INGRESSCHANNEL_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_INGRESSCHANNEL_INGRESSCHANNEL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_GENERAL_CONFIGURATION_INGRESSCHANNEL *****/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_INGRESSCHANNEL_REG =
{
    "GENERAL_CONFIGURATION_INGRESSCHANNEL",
#if RU_INCLUDE_DESC
    "INGRESS_CHANNEL Register",
    "There are 9 BBHs per CRDP DS/US pipe.\nThis register defines the channel number of the BBH.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_INGRESSCHANNEL_REG_OFFSET },
    0,
    0,
    217,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_GENERAL_CONFIGURATION_INGRESSCHANNEL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_GENERAL_CONFIGURATION_BBOVRDROUTE, TYPE: Type_BBH_RX_BBHRX_GENERAL_CONFIGURATION_BBOVRDROUTE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ROUTE *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_BBOVRDROUTE_ROUTE_FIELD =
{
    "ROUTE",
#if RU_INCLUDE_DESC
    "",
    "route address\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_BBOVRDROUTE_ROUTE_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_BBOVRDROUTE_ROUTE_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_BBOVRDROUTE_ROUTE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_BBOVRDROUTE_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_BBOVRDROUTE_ROUTE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_GENERAL_CONFIGURATION_BBOVRDROUTE *****/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_BBOVRDROUTE_REG =
{
    "GENERAL_CONFIGURATION_BBOVRDROUTE",
#if RU_INCLUDE_DESC
    "BB_OVERRIDE_ROUTE Register",
    "override configuration for the route of one of the peripherals\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_BBOVRDROUTE_REG_OFFSET },
    0,
    0,
    218,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_GENERAL_CONFIGURATION_BBOVRDROUTE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_GENERAL_CONFIGURATION_BBOVRDCTRL, TYPE: Type_BBH_RX_BBHRX_GENERAL_CONFIGURATION_BBOVRDCTRL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DEST *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_BBOVRDCTRL_DEST_FIELD =
{
    "DEST",
#if RU_INCLUDE_DESC
    "",
    "destination source id\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_BBOVRDCTRL_DEST_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_BBOVRDCTRL_DEST_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_BBOVRDCTRL_DEST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EN *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_BBOVRDCTRL_EN_FIELD =
{
    "EN",
#if RU_INCLUDE_DESC
    "",
    "enable\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_BBOVRDCTRL_EN_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_BBOVRDCTRL_EN_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_BBOVRDCTRL_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_BBOVRDCTRL_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_BBOVRDCTRL_DEST_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_BBOVRDCTRL_EN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_GENERAL_CONFIGURATION_BBOVRDCTRL *****/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_BBOVRDCTRL_REG =
{
    "GENERAL_CONFIGURATION_BBOVRDCTRL",
#if RU_INCLUDE_DESC
    "BB_OVERRIDE_CTRL Register",
    "override configuration for the route of one of the peripherals\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_BBOVRDCTRL_REG_OFFSET },
    0,
    0,
    219,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_RX_GENERAL_CONFIGURATION_BBOVRDCTRL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_GENERAL_CONFIGURATION_GEMDONE, TYPE: Type_BBH_RX_BBHRX_GENERAL_CONFIGURATION_GEMDONE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: GEMDONE *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_GEMDONE_GEMDONE_FIELD =
{
    "GEMDONE",
#if RU_INCLUDE_DESC
    "",
    "GEM mode init done\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_GEMDONE_GEMDONE_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_GEMDONE_GEMDONE_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_GEMDONE_GEMDONE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_GEMDONE_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_GEMDONE_GEMDONE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_GENERAL_CONFIGURATION_GEMDONE *****/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_GEMDONE_REG =
{
    "GENERAL_CONFIGURATION_GEMDONE",
#if RU_INCLUDE_DESC
    "GEM_MODE_INIT_DONE Register",
    "GEM MODE memory init is done\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_GEMDONE_REG_OFFSET },
    0,
    0,
    220,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_GENERAL_CONFIGURATION_GEMDONE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_GENERAL_CONFIGURATION_MACTYPE, TYPE: Type_BBH_RX_BBHRX_GENERAL_CONFIGURATION_MACTYPE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: MACTYPE *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_MACTYPE_MACTYPE_FIELD =
{
    "MACTYPE",
#if RU_INCLUDE_DESC
    "",
    "Configuration of the MAC type:\n4d0   : OLT 1G EPON\n4d1   : OLT 10G EPON\n4d2   : OLT 25G EPON / ONU 25G EPON\n4d3   : OLT Optical AE\n4d4   : OLT GPON\n4d5   : OLT XGPON/XGS\n4d6   : OLT HSP\n4d7   : NNI\n4d8   : Reserved\n4d9   : Reserved\n4d10  : ONU HSP\n4d11  : Reserved\n4d12  : Reserved\n4d13  : Reserved\n4d14  : Reserved\n4d15  : Reserved\n\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_MACTYPE_MACTYPE_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_MACTYPE_MACTYPE_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_MACTYPE_MACTYPE_FIELD_SHIFT },
    7,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_MACTYPE_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_MACTYPE_MACTYPE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_GENERAL_CONFIGURATION_MACTYPE *****/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_MACTYPE_REG =
{
    "GENERAL_CONFIGURATION_MACTYPE",
#if RU_INCLUDE_DESC
    "MAC_TYPE 0..7 Register",
    "Defines the MAC type\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_MACTYPE_REG_OFFSET },
    BBH_RX_GENERAL_CONFIGURATION_MACTYPE_REG_RAM_CNT,
    4,
    221,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_GENERAL_CONFIGURATION_MACTYPE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_GENERAL_CONFIGURATION_PORTTH, TYPE: Type_BBH_RX_BBHRX_GENERAL_CONFIGURATION_PORTTH
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PORTTHR *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_PORTTH_PORTTHR_FIELD =
{
    "PORTTHR",
#if RU_INCLUDE_DESC
    "",
    "Threshold configuration in 32bytes resolution.\nThe FIFO size is 1KBytes - 32 x 256b.\nMax value is 0x20.\n0x3f should be used to disable the effect of this threshold.\n\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_PORTTH_PORTTHR_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_PORTTH_PORTTHR_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_PORTTH_PORTTHR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_PORTTH_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_PORTTH_PORTTHR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_GENERAL_CONFIGURATION_PORTTH *****/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_PORTTH_REG =
{
    "GENERAL_CONFIGURATION_PORTTH",
#if RU_INCLUDE_DESC
    "PORT_THRESHOLD 0..7 Register",
    "The BBH RX has a FIFO per port.\nEach FIFO has a configurable threshold; when crossed, non-exclusive priority packets may be dropped (see flow control configuration).\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_PORTTH_REG_OFFSET },
    BBH_RX_GENERAL_CONFIGURATION_PORTTH_REG_RAM_CNT,
    4,
    222,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_GENERAL_CONFIGURATION_PORTTH_FIELDS,
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
/***** Field: DROPNONEXC *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_DROPNONEXC_FIELD =
{
    "DROPNONEXC",
#if RU_INCLUDE_DESC
    "",
    "The BBH may receive a flow control indication from the Dispatcher.\nWhen in flow control state, this register defines either of the following:\n00: Back-pressure any new packet\n01: Pass this flow control notification to the MAC (e.g. PMA) - BBHRX2MAC_FC\n10: Receive Exclusive priority packets only (drop any other packet). This will be done by a simple pattern match logic in the BBH RX\n11: reserved\n\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_DROPNONEXC_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_DROPNONEXC_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_DROPNONEXC_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BPNOTFC *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_BPNOTFC_FIELD =
{
    "BPNOTFC",
#if RU_INCLUDE_DESC
    "",
    "When flow control is not active (according to Dispatcher messages to the BBH RX), the BBH RX will drop packets when it reaching full condition.\nWhen this bit is asserted, the BBH will back-pressure when reaching full condition (and flow control is not active).\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_BPNOTFC_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_BPNOTFC_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_BPNOTFC_FIELD_SHIFT },
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
    &BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_FCFORCE_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_FCRNREN_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_FCQMEN_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_DROPNONEXC_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_BPNOTFC_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL *****/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_REG =
{
    "GENERAL_CONFIGURATION_FLOWCTRL",
#if RU_INCLUDE_DESC
    "FLOW_CONTROL_CONFIGURATION 0..7 Register",
    "The BBH RX may assert the flow control indication towards the Ethernet MAC, signaling that the system is congested. As long as this signal is asserted, the MAC will keep the link-partner in Xoff state.\nBoth Runner and QM may send a BB message to control the flow control. A static configuration determines if the flow control is managed by FW, QM or both.\nFW message triggers a timer. When the timer expires, the BBH will de-assert the flow control indication.\nQM may send Xoff and Xon messages.\nThis register also disable BBH packet drop due to no space in the SDMA, SBPM or Dispatcher.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_REG_OFFSET },
    BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_REG_RAM_CNT,
    4,
    223,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    6,
    BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_FIELDS,
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
    "SOP_OFFSET 0..7 Register",
    "The BBH writes the packets into the PSRAM. The start of data offset is configurable. This register defines the SOP (start of packet) offset.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_SOPOFFSET_REG_OFFSET },
    BBH_RX_GENERAL_CONFIGURATION_SOPOFFSET_REG_RAM_CNT,
    4,
    224,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_GENERAL_CONFIGURATION_SOPOFFSET_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_GENERAL_CONFIGURATION_INGRESSPID, TYPE: Type_BBH_RX_BBHRX_GENERAL_CONFIGURATION_INGRESSPID
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: INGRESSPID *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_INGRESSPID_INGRESSPID_FIELD =
{
    "INGRESSPID",
#if RU_INCLUDE_DESC
    "",
    "Ingress PID configuration (will be sent in the PD)\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_INGRESSPID_INGRESSPID_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_INGRESSPID_INGRESSPID_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_INGRESSPID_INGRESSPID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_INGRESSPID_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_INGRESSPID_INGRESSPID_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_GENERAL_CONFIGURATION_INGRESSPID *****/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_INGRESSPID_REG =
{
    "GENERAL_CONFIGURATION_INGRESSPID",
#if RU_INCLUDE_DESC
    "INGRESS_PID 0..7 Register",
    "Each BBH is connected to 8 ports. This register configures the port IDs, which will be sent in the PD.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_INGRESSPID_REG_OFFSET },
    BBH_RX_GENERAL_CONFIGURATION_INGRESSPID_REG_RAM_CNT,
    4,
    225,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_GENERAL_CONFIGURATION_INGRESSPID_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_GENERAL_CONFIGURATION_PORTWEIGHT, TYPE: Type_BBH_RX_BBHRX_GENERAL_CONFIGURATION_PORTWEIGHT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PORTWEIGHT *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_PORTWEIGHT_PORTWEIGHT_FIELD =
{
    "PORTWEIGHT",
#if RU_INCLUDE_DESC
    "",
    "Ports weight for WRR arbitration\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_PORTWEIGHT_PORTWEIGHT_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_PORTWEIGHT_PORTWEIGHT_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_PORTWEIGHT_PORTWEIGHT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_PORTWEIGHT_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_PORTWEIGHT_PORTWEIGHT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_GENERAL_CONFIGURATION_PORTWEIGHT *****/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_PORTWEIGHT_REG =
{
    "GENERAL_CONFIGURATION_PORTWEIGHT",
#if RU_INCLUDE_DESC
    "PORT_WEIGHT 0..7 Register",
    "The BBH write the ports  data into the PSRAM.\nIt arbitrates between 8 ports using Weighted Round-Robin algorithm.\nThe register defines the weight per port\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_PORTWEIGHT_REG_OFFSET },
    BBH_RX_GENERAL_CONFIGURATION_PORTWEIGHT_REG_RAM_CNT,
    4,
    226,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_GENERAL_CONFIGURATION_PORTWEIGHT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_GENERAL_CONFIGURATION_PORTVIQMASK, TYPE: Type_BBH_RX_BBHRX_GENERAL_CONFIGURATION_PORTVIQMASK
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PORTVIQMASK *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_PORTVIQMASK_PORTVIQMASK_FIELD =
{
    "PORTVIQMASK",
#if RU_INCLUDE_DESC
    "",
    "Ports VIQ mask - one hot vector\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_PORTVIQMASK_PORTVIQMASK_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_PORTVIQMASK_PORTVIQMASK_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_PORTVIQMASK_PORTVIQMASK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_PORTVIQMASK_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_PORTVIQMASK_PORTVIQMASK_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_GENERAL_CONFIGURATION_PORTVIQMASK *****/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_PORTVIQMASK_REG =
{
    "GENERAL_CONFIGURATION_PORTVIQMASK",
#if RU_INCLUDE_DESC
    "PORT_VIQ_MASK 0..7 Register",
    "The BBH has 8 ports and can manage 9 Dispatcher VIQs.\nEach port may be configured to either one of the VIQs and to the exclusive VIQ.\nThis register determines the VIQ using one hot vector (9 bits) - bit per VIQ.\nFor example:\nIf port 0 is connected to DISPVIQ index 3, then this register should be configured to 9h8.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_PORTVIQMASK_REG_OFFSET },
    BBH_RX_GENERAL_CONFIGURATION_PORTVIQMASK_REG_RAM_CNT,
    4,
    227,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_GENERAL_CONFIGURATION_PORTVIQMASK_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_GENERAL_CONFIGURATION_EXCVIQMASK, TYPE: Type_BBH_RX_BBHRX_GENERAL_CONFIGURATION_EXCVIQMASK
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: EXCVIQMASK *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_EXCVIQMASK_EXCVIQMASK_FIELD =
{
    "EXCVIQMASK",
#if RU_INCLUDE_DESC
    "",
    "Exclusive VIQ mask - one hot vector\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_EXCVIQMASK_EXCVIQMASK_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_EXCVIQMASK_EXCVIQMASK_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_EXCVIQMASK_EXCVIQMASK_FIELD_SHIFT },
    256,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EXCVIQEN *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_EXCVIQMASK_EXCVIQEN_FIELD =
{
    "EXCVIQEN",
#if RU_INCLUDE_DESC
    "",
    "Exclusive VIQ enable.\nEach port may enable/disable this VIQ.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_EXCVIQMASK_EXCVIQEN_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_EXCVIQMASK_EXCVIQEN_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_EXCVIQMASK_EXCVIQEN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EXCVIQOAMEN *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_EXCVIQMASK_EXCVIQOAMEN_FIELD =
{
    "EXCVIQOAMEN",
#if RU_INCLUDE_DESC
    "",
    "Exclusive VIQ enable due to OAM indication.\nEach port may enable/disable this VIQ.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_EXCVIQMASK_EXCVIQOAMEN_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_EXCVIQMASK_EXCVIQOAMEN_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_EXCVIQMASK_EXCVIQOAMEN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_EXCVIQMASK_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_EXCVIQMASK_EXCVIQMASK_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_EXCVIQMASK_EXCVIQEN_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_EXCVIQMASK_EXCVIQOAMEN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_GENERAL_CONFIGURATION_EXCVIQMASK *****/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_EXCVIQMASK_REG =
{
    "GENERAL_CONFIGURATION_EXCVIQMASK",
#if RU_INCLUDE_DESC
    "EXC_VIQ_MASK 0..7 Register",
    "The BBH has 8 ports and can manage 9 Dispatcher VIQs.\nEach port may be configured to either one of the VIQs and to the exclusive VIQ.\nThis register determines the Exclusive VIQ using one hot vector (9 bits) - bit per VIQ.\nFor example:\nIf port 0 is connected to DISPVIQ index 3, then this register should be configured to 9h8.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_EXCVIQMASK_REG_OFFSET },
    BBH_RX_GENERAL_CONFIGURATION_EXCVIQMASK_REG_RAM_CNT,
    4,
    228,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    BBH_RX_GENERAL_CONFIGURATION_EXCVIQMASK_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_GENERAL_CONFIGURATION_DISPVIQ, TYPE: Type_BBH_RX_BBHRX_GENERAL_CONFIGURATION_DISPVIQ
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VIQ *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_DISPVIQ_VIQ_FIELD =
{
    "VIQ",
#if RU_INCLUDE_DESC
    "",
    "Defines the Dispatchers VIQs for PFB channels 0-7\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_DISPVIQ_VIQ_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_DISPVIQ_VIQ_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_DISPVIQ_VIQ_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_DISPVIQ_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_DISPVIQ_VIQ_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_GENERAL_CONFIGURATION_DISPVIQ *****/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_DISPVIQ_REG =
{
    "GENERAL_CONFIGURATION_DISPVIQ",
#if RU_INCLUDE_DESC
    "DISPATCHER_VIQ 0..8 Register",
    "For every reassembled packet in the PSRAM the BBH writes a packet descriptor (PD) into the Dispatcher. The PDs are arranged using a link list in the Dispatcher. The Dispatcher manages multiple virtual queues (VIQ) and the BBH may be assigned to each of the VIQs of the Dispatcher\nThis register defines virtual queues.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_DISPVIQ_REG_OFFSET },
    BBH_RX_GENERAL_CONFIGURATION_DISPVIQ_REG_RAM_CNT,
    4,
    229,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_GENERAL_CONFIGURATION_DISPVIQ_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_GENERAL_CONFIGURATION_VIQWEIGHT, TYPE: Type_BBH_RX_BBHRX_GENERAL_CONFIGURATION_VIQWEIGHT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VIQWEIGHT *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_VIQWEIGHT_VIQWEIGHT_FIELD =
{
    "VIQWEIGHT",
#if RU_INCLUDE_DESC
    "",
    "VIQ weight for WRR arbitration\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_VIQWEIGHT_VIQWEIGHT_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_VIQWEIGHT_VIQWEIGHT_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_VIQWEIGHT_VIQWEIGHT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_VIQWEIGHT_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_VIQWEIGHT_VIQWEIGHT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_GENERAL_CONFIGURATION_VIQWEIGHT *****/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_VIQWEIGHT_REG =
{
    "GENERAL_CONFIGURATION_VIQWEIGHT",
#if RU_INCLUDE_DESC
    "VIQ_WEIGHT 0..8 Register",
    "The BBH write the PDs into the Dispatcher.\nIt arbitrates between 9 VIQs using Weighted Round-Robin algorithm.\nThe register defines the weight per port\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_VIQWEIGHT_REG_OFFSET },
    BBH_RX_GENERAL_CONFIGURATION_VIQWEIGHT_REG_RAM_CNT,
    4,
    230,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_GENERAL_CONFIGURATION_VIQWEIGHT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_GENERAL_CONFIGURATION_PATTERNCFG, TYPE: Type_BBH_RX_BBHRX_GENERAL_CONFIGURATION_PATTERNCFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: LOCATION *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_PATTERNCFG_LOCATION_FIELD =
{
    "LOCATION",
#if RU_INCLUDE_DESC
    "",
    "The pattern is compared against the 32 first bytes of each packet. This field defines the start offset for matching (1-byte resolution) within these 32 bytes.\nLocation max value is: 32 - (pattern size in bytes)\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_PATTERNCFG_LOCATION_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_PATTERNCFG_LOCATION_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_PATTERNCFG_LOCATION_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: NO_DROP_EN *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_PATTERNCFG_NO_DROP_EN_FIELD =
{
    "NO_DROP_EN",
#if RU_INCLUDE_DESC
    "",
    "1 means this pattern match is used for not dropping high priority (if drop is enabled).\n0 means this pattern is not used for not dropping high priority (if drop is enabled).\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_PATTERNCFG_NO_DROP_EN_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_PATTERNCFG_NO_DROP_EN_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_PATTERNCFG_NO_DROP_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: VIQCLASSEN *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_PATTERNCFG_VIQCLASSEN_FIELD =
{
    "VIQCLASSEN",
#if RU_INCLUDE_DESC
    "",
    "1 means this pattern match is used for VIQ classification.\n0 means this pattern is not used for VIQ classification.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_PATTERNCFG_VIQCLASSEN_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_PATTERNCFG_VIQCLASSEN_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_PATTERNCFG_VIQCLASSEN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: VIQ *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_PATTERNCFG_VIQ_FIELD =
{
    "VIQ",
#if RU_INCLUDE_DESC
    "",
    "9 bits (one hot - out of 9 VIQs) \" relevant only if NO_DROP = 0\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_PATTERNCFG_VIQ_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_PATTERNCFG_VIQ_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_PATTERNCFG_VIQ_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_PATTERNCFG_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_PATTERNCFG_LOCATION_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_PATTERNCFG_NO_DROP_EN_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_PATTERNCFG_VIQCLASSEN_FIELD,
    &BBH_RX_GENERAL_CONFIGURATION_PATTERNCFG_VIQ_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_GENERAL_CONFIGURATION_PATTERNCFG *****/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_PATTERNCFG_REG =
{
    "GENERAL_CONFIGURATION_PATTERNCFG",
#if RU_INCLUDE_DESC
    "PATTERN_MATCH_CFG 0..7 Register",
    "This set of 8 registers refers to 8 pattern match:\n0-5 refers to 6 patterns of 4 bytes\n6-7 refers to 2 patterns of 8 bytes\n\n6 global patterns of 4 bytes:\nPattern                                 4 bytes\nMask                                    4 bytes\nLocation within the first 32B\t        5 bits (1-bytes resolution)\nPriority no Drop enable          \t1 bit - used for not dropping high priority\nVIQ classification enable       \t1 bit - used VIQ classification\nVIQ                                     9 bits (one hot - out of 9 VIQs)\n\n2 global patterns of 8 bytes:\nPattern                                 8 bytes\nMask                                    8 bytes\nLocation within the first 32B\t        5 bits (1-bytes resolution)\nPriority no Drop enable          \t1 bit - used for not dropping high priority\nVIQ classification enable       \t1 bit - used VIQ classification\nVIQ                                     9 bits (one hot - out of 9 VIQs)\n\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_PATTERNCFG_REG_OFFSET },
    BBH_RX_GENERAL_CONFIGURATION_PATTERNCFG_REG_RAM_CNT,
    4,
    231,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    BBH_RX_GENERAL_CONFIGURATION_PATTERNCFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_GENERAL_CONFIGURATION_PATTERNDATA, TYPE: Type_BBH_RX_BBHRX_GENERAL_CONFIGURATION_PATTERNDATA
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PATTERNDATA *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_PATTERNDATA_PATTERNDATA_FIELD =
{
    "PATTERNDATA",
#if RU_INCLUDE_DESC
    "",
    "Pattern Data (reg index 0 refers to 32 LSBits)\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_PATTERNDATA_PATTERNDATA_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_PATTERNDATA_PATTERNDATA_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_PATTERNDATA_PATTERNDATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_PATTERNDATA_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_PATTERNDATA_PATTERNDATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_GENERAL_CONFIGURATION_PATTERNDATA *****/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_PATTERNDATA_REG =
{
    "GENERAL_CONFIGURATION_PATTERNDATA",
#if RU_INCLUDE_DESC
    "PATTERN_MATCH_DATA 0..5 Register",
    "The BBH may direct or drop a packet into the Dispatchers exclusive VIQ (Virtual Ingress Queue) according to a match in the pattern match.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_PATTERNDATA_REG_OFFSET },
    BBH_RX_GENERAL_CONFIGURATION_PATTERNDATA_REG_RAM_CNT,
    4,
    232,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_GENERAL_CONFIGURATION_PATTERNDATA_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_GENERAL_CONFIGURATION_PATTERNMASK, TYPE: Type_BBH_RX_BBHRX_GENERAL_CONFIGURATION_PATTERNMASK
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PATTERNMASK *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_PATTERNMASK_PATTERNMASK_FIELD =
{
    "PATTERNMASK",
#if RU_INCLUDE_DESC
    "",
    "Pattern mask. Reg index 0 refers to 32 LSBits\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_PATTERNMASK_PATTERNMASK_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_PATTERNMASK_PATTERNMASK_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_PATTERNMASK_PATTERNMASK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_PATTERNMASK_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_PATTERNMASK_PATTERNMASK_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_GENERAL_CONFIGURATION_PATTERNMASK *****/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_PATTERNMASK_REG =
{
    "GENERAL_CONFIGURATION_PATTERNMASK",
#if RU_INCLUDE_DESC
    "PATTERN_MATCH_MASK 0..5 Register",
    "The BBH may drop or direct a packet into the Dispatchers exclusive VIQ (Virtual Ingress Queue) according to a match in the pattern match.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_PATTERNMASK_REG_OFFSET },
    BBH_RX_GENERAL_CONFIGURATION_PATTERNMASK_REG_RAM_CNT,
    4,
    233,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_GENERAL_CONFIGURATION_PATTERNMASK_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_GENERAL_CONFIGURATION_PATTERNDATA8B, TYPE: Type_BBH_RX_BBHRX_GENERAL_CONFIGURATION_PATTERNDATA8B
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PATTERNDATA *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_PATTERNDATA8B_PATTERNDATA_FIELD =
{
    "PATTERNDATA",
#if RU_INCLUDE_DESC
    "",
    "Pattern Data (reg index 0 refers to 32 LSBits)\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_PATTERNDATA8B_PATTERNDATA_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_PATTERNDATA8B_PATTERNDATA_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_PATTERNDATA8B_PATTERNDATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_PATTERNDATA8B_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_PATTERNDATA8B_PATTERNDATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_GENERAL_CONFIGURATION_PATTERNDATA8B *****/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_PATTERNDATA8B_REG =
{
    "GENERAL_CONFIGURATION_PATTERNDATA8B",
#if RU_INCLUDE_DESC
    "PATTERN_MATCH_DATA_8B 0..3 Register",
    "The BBH may direct or drop a packet into the Dispatchers exclusive VIQ (Virtual Ingress Queue) according to a match in the pattern match.\nAn array of 4 registers is used for 2 patterns:\nPattern 0: {reg0, reg1}\nPattern 1: {reg2, reg3}\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_PATTERNDATA8B_REG_OFFSET },
    BBH_RX_GENERAL_CONFIGURATION_PATTERNDATA8B_REG_RAM_CNT,
    4,
    234,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_GENERAL_CONFIGURATION_PATTERNDATA8B_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_GENERAL_CONFIGURATION_PATTERNMASK8B, TYPE: Type_BBH_RX_BBHRX_GENERAL_CONFIGURATION_PATTERNMASK8B
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PATTERNMASK *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_PATTERNMASK8B_PATTERNMASK_FIELD =
{
    "PATTERNMASK",
#if RU_INCLUDE_DESC
    "",
    "Pattern mask. Reg index 0 refers to 32 LSBits\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_PATTERNMASK8B_PATTERNMASK_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_PATTERNMASK8B_PATTERNMASK_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_PATTERNMASK8B_PATTERNMASK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_PATTERNMASK8B_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_PATTERNMASK8B_PATTERNMASK_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_GENERAL_CONFIGURATION_PATTERNMASK8B *****/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_PATTERNMASK8B_REG =
{
    "GENERAL_CONFIGURATION_PATTERNMASK8B",
#if RU_INCLUDE_DESC
    "PATTERN_MATCH_MASK_8B 0..3 Register",
    "The BBH may drop or direct a packet into the Dispatchers exclusive VIQ (Virtual Ingress Queue) according to a match in the pattern match.\nAn array of 4 registers is used for 2 patterns:\nPattern 0: {reg0, reg1}\nPattern 1: {reg2, reg3}\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_PATTERNMASK8B_REG_OFFSET },
    BBH_RX_GENERAL_CONFIGURATION_PATTERNMASK8B_REG_RAM_CNT,
    4,
    235,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_GENERAL_CONFIGURATION_PATTERNMASK8B_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_GENERAL_CONFIGURATION_PATTERNEN, TYPE: Type_BBH_RX_BBHRX_GENERAL_CONFIGURATION_PATTERNEN
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PATTERNEN *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_PATTERNEN_PATTERNEN_FIELD =
{
    "PATTERNEN",
#if RU_INCLUDE_DESC
    "",
    "Each of the global 8 patterns can be enables per port according to this vector (bit per pattern match).\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_PATTERNEN_PATTERNEN_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_PATTERNEN_PATTERNEN_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_PATTERNEN_PATTERNEN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_PATTERNEN_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_PATTERNEN_PATTERNEN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_GENERAL_CONFIGURATION_PATTERNEN *****/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_PATTERNEN_REG =
{
    "GENERAL_CONFIGURATION_PATTERNEN",
#if RU_INCLUDE_DESC
    "PER_PORT_PATTERN_MATCH_EN 0..7 Register",
    "Each BBH is connected to 8 ports. This register configures the pattern match enable vector per port.\nEach of the global 8 patterns can be enables per port according to this vector (bit per pattern match).\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_PATTERNEN_REG_OFFSET },
    BBH_RX_GENERAL_CONFIGURATION_PATTERNEN_REG_RAM_CNT,
    4,
    236,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_GENERAL_CONFIGURATION_PATTERNEN_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_GENERAL_CONFIGURATION_VIQFCEN, TYPE: Type_BBH_RX_BBHRX_GENERAL_CONFIGURATION_VIQFCEN
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VIQFCEN *****/
const ru_field_rec BBH_RX_GENERAL_CONFIGURATION_VIQFCEN_VIQFCEN_FIELD =
{
    "VIQFCEN",
#if RU_INCLUDE_DESC
    "",
    "VIQ flow control enable vector\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_VIQFCEN_VIQFCEN_FIELD_MASK },
    0,
    { BBH_RX_GENERAL_CONFIGURATION_VIQFCEN_VIQFCEN_FIELD_WIDTH },
    { BBH_RX_GENERAL_CONFIGURATION_VIQFCEN_VIQFCEN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GENERAL_CONFIGURATION_VIQFCEN_FIELDS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_VIQFCEN_VIQFCEN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_GENERAL_CONFIGURATION_VIQFCEN *****/
const ru_reg_rec BBH_RX_GENERAL_CONFIGURATION_VIQFCEN_REG =
{
    "GENERAL_CONFIGURATION_VIQFCEN",
#if RU_INCLUDE_DESC
    "VIQ_FLOW_CONTROL_ENABLE_VECTOR 0..7 Register",
    "Each BBH is connected to 8 ports. This vector (per port) has 1 bit per VIQ and maps all the relevant VIQ to a specific port.\nE.g. if VIQs 0, 4 and 5 (indexes according to DISPVIQ register) are valid for a specific port, then the relevant vector value is: 9h31\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_VIQFCEN_REG_OFFSET },
    BBH_RX_GENERAL_CONFIGURATION_VIQFCEN_REG_RAM_CNT,
    4,
    237,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_GENERAL_CONFIGURATION_VIQFCEN_FIELDS,
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
    "NON_ETHERNET_FLOW 0..7 Register",
    "There an option to disable CRC error counting for this flow.\n",
#endif
    { BBH_RX_GENERAL_CONFIGURATION_NONETH_REG_OFFSET },
    BBH_RX_GENERAL_CONFIGURATION_NONETH_REG_RAM_CNT,
    4,
    238,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_RX_GENERAL_CONFIGURATION_NONETH_FIELDS,
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
    "INCOMING_PACKETS 0..7 Register",
    "This counter counts the number of incoming good packets.\nIt counts the packets from all flows together.\nThis counter is cleared when read and freezes when reaches the maximum value.\n",
#endif
    { BBH_RX_PM_COUNTERS_INPKT_REG_OFFSET },
    BBH_RX_PM_COUNTERS_INPKT_REG_RAM_CNT,
    4,
    239,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_PM_COUNTERS_INPKT_FIELDS,
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
    "INCOMING_BYTES 0..7 Register",
    "This counter counts the number of incoming good bytes.\nIt counts the bytes from all flows together.\nThis counter is cleared when read and freezes when reaches the maximum value.\n",
#endif
    { BBH_RX_PM_COUNTERS_INBYTE_REG_OFFSET },
    BBH_RX_PM_COUNTERS_INBYTE_REG_RAM_CNT,
    4,
    240,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_PM_COUNTERS_INBYTE_FIELDS,
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
    "TOO_SHORT_ERROR 0..7 Register",
    "This counter counts the packets drop due to Too short error.\nThis counter is cleared when read and freezes when reaches the maximum value.\n",
#endif
    { BBH_RX_PM_COUNTERS_TOOSHORT_REG_OFFSET },
    BBH_RX_PM_COUNTERS_TOOSHORT_REG_RAM_CNT,
    4,
    241,
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
    "TOO_LONG_ERROR 0..7 Register",
    "This counter counts the packets drop due to Too long error.\nThis counter is cleared when read and freezes when reaches the maximum value.\n",
#endif
    { BBH_RX_PM_COUNTERS_TOOLONG_REG_OFFSET },
    BBH_RX_PM_COUNTERS_TOOLONG_REG_RAM_CNT,
    4,
    242,
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
    "CRC_ERROR 0..7 Register",
    "This counter counts the packets drop due to CRC error.\nThis counter is cleared when read and freezes when reaches the maximum value.\n",
#endif
    { BBH_RX_PM_COUNTERS_CRCERROR_REG_OFFSET },
    BBH_RX_PM_COUNTERS_CRCERROR_REG_RAM_CNT,
    4,
    243,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_PM_COUNTERS_CRCERROR_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_PM_COUNTERS_FCDROP, TYPE: Type_BBH_RX_BBHRX_PM_COUNTERS_FCDROP
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PMVALUE *****/
const ru_field_rec BBH_RX_PM_COUNTERS_FCDROP_PMVALUE_FIELD =
{
    "PMVALUE",
#if RU_INCLUDE_DESC
    "",
    "PM counter value.\n",
#endif
    { BBH_RX_PM_COUNTERS_FCDROP_PMVALUE_FIELD_MASK },
    0,
    { BBH_RX_PM_COUNTERS_FCDROP_PMVALUE_FIELD_WIDTH },
    { BBH_RX_PM_COUNTERS_FCDROP_PMVALUE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_PM_COUNTERS_FCDROP_FIELDS[] =
{
    &BBH_RX_PM_COUNTERS_FCDROP_PMVALUE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_PM_COUNTERS_FCDROP *****/
const ru_reg_rec BBH_RX_PM_COUNTERS_FCDROP_REG =
{
    "PM_COUNTERS_FCDROP",
#if RU_INCLUDE_DESC
    "FC_DROP 0..7 Register",
    "This counter counts the packets drop due to flow control.\nThis counter is cleared when read and freezes when reaches the maximum value.\n",
#endif
    { BBH_RX_PM_COUNTERS_FCDROP_REG_OFFSET },
    BBH_RX_PM_COUNTERS_FCDROP_REG_RAM_CNT,
    4,
    244,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_PM_COUNTERS_FCDROP_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_PM_COUNTERS_TRUNCATE, TYPE: Type_BBH_RX_BBHRX_PM_COUNTERS_TRUNCATE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PMVALUE *****/
const ru_field_rec BBH_RX_PM_COUNTERS_TRUNCATE_PMVALUE_FIELD =
{
    "PMVALUE",
#if RU_INCLUDE_DESC
    "",
    "PM counter value.\n",
#endif
    { BBH_RX_PM_COUNTERS_TRUNCATE_PMVALUE_FIELD_MASK },
    0,
    { BBH_RX_PM_COUNTERS_TRUNCATE_PMVALUE_FIELD_WIDTH },
    { BBH_RX_PM_COUNTERS_TRUNCATE_PMVALUE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_PM_COUNTERS_TRUNCATE_FIELDS[] =
{
    &BBH_RX_PM_COUNTERS_TRUNCATE_PMVALUE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_PM_COUNTERS_TRUNCATE *****/
const ru_reg_rec BBH_RX_PM_COUNTERS_TRUNCATE_REG =
{
    "PM_COUNTERS_TRUNCATE",
#if RU_INCLUDE_DESC
    "TRUNCATE 0..7 Register",
    "This counter counts the packets drop due to flow control and truncated.\nThis counter is cleared when read and freezes when reaches the maximum value.\n",
#endif
    { BBH_RX_PM_COUNTERS_TRUNCATE_REG_OFFSET },
    BBH_RX_PM_COUNTERS_TRUNCATE_REG_RAM_CNT,
    4,
    245,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_PM_COUNTERS_TRUNCATE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_PM_COUNTERS_TOTALDROP, TYPE: Type_BBH_RX_BBHRX_PM_COUNTERS_TOTALDROP
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PMVALUE *****/
const ru_field_rec BBH_RX_PM_COUNTERS_TOTALDROP_PMVALUE_FIELD =
{
    "PMVALUE",
#if RU_INCLUDE_DESC
    "",
    "PM counter value.\n",
#endif
    { BBH_RX_PM_COUNTERS_TOTALDROP_PMVALUE_FIELD_MASK },
    0,
    { BBH_RX_PM_COUNTERS_TOTALDROP_PMVALUE_FIELD_WIDTH },
    { BBH_RX_PM_COUNTERS_TOTALDROP_PMVALUE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_PM_COUNTERS_TOTALDROP_FIELDS[] =
{
    &BBH_RX_PM_COUNTERS_TOTALDROP_PMVALUE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_PM_COUNTERS_TOTALDROP *****/
const ru_reg_rec BBH_RX_PM_COUNTERS_TOTALDROP_REG =
{
    "PM_COUNTERS_TOTALDROP",
#if RU_INCLUDE_DESC
    "TOTAL_DROP 0..7 Register",
    "This counter counts the total number of dropped packets.\nThis counter is cleared when read and freezes when reaches the maximum value.\n",
#endif
    { BBH_RX_PM_COUNTERS_TOTALDROP_REG_OFFSET },
    BBH_RX_PM_COUNTERS_TOTALDROP_REG_RAM_CNT,
    4,
    246,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_PM_COUNTERS_TOTALDROP_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_PM_COUNTERS_DUMMYPDCNT, TYPE: Type_BBH_RX_BBHRX_PM_COUNTERS_DUMMYPDCNT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PMVALUE *****/
const ru_field_rec BBH_RX_PM_COUNTERS_DUMMYPDCNT_PMVALUE_FIELD =
{
    "PMVALUE",
#if RU_INCLUDE_DESC
    "",
    "PM counter value.\n",
#endif
    { BBH_RX_PM_COUNTERS_DUMMYPDCNT_PMVALUE_FIELD_MASK },
    0,
    { BBH_RX_PM_COUNTERS_DUMMYPDCNT_PMVALUE_FIELD_WIDTH },
    { BBH_RX_PM_COUNTERS_DUMMYPDCNT_PMVALUE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_PM_COUNTERS_DUMMYPDCNT_FIELDS[] =
{
    &BBH_RX_PM_COUNTERS_DUMMYPDCNT_PMVALUE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_PM_COUNTERS_DUMMYPDCNT *****/
const ru_reg_rec BBH_RX_PM_COUNTERS_DUMMYPDCNT_REG =
{
    "PM_COUNTERS_DUMMYPDCNT",
#if RU_INCLUDE_DESC
    "DUMMY_PD Register",
    "This counter counts the number of Dummy PDs.\nGlobal counter per BBH RX instance (not per port).\nThis counter is cleared when read and freezes when reaches the maximum value.\n",
#endif
    { BBH_RX_PM_COUNTERS_DUMMYPDCNT_REG_OFFSET },
    0,
    0,
    247,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_PM_COUNTERS_DUMMYPDCNT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_PON_RMON_COUNTERS_R64, TYPE: Type_BBH_RX_BBHRX_PON_RMON_COUNTERS_R64
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: R64 *****/
const ru_field_rec BBH_RX_PON_RMON_COUNTERS_R64_R64_FIELD =
{
    "R64",
#if RU_INCLUDE_DESC
    "",
    "This counter counts the number of R64 packets.\n",
#endif
    { BBH_RX_PON_RMON_COUNTERS_R64_R64_FIELD_MASK },
    0,
    { BBH_RX_PON_RMON_COUNTERS_R64_R64_FIELD_WIDTH },
    { BBH_RX_PON_RMON_COUNTERS_R64_R64_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_PON_RMON_COUNTERS_R64_FIELDS[] =
{
    &BBH_RX_PON_RMON_COUNTERS_R64_R64_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_PON_RMON_COUNTERS_R64 *****/
const ru_reg_rec BBH_RX_PON_RMON_COUNTERS_R64_REG =
{
    "PON_RMON_COUNTERS_R64",
#if RU_INCLUDE_DESC
    "R64 Register",
    "This counter counts the number of packets in the range of 64 Bytes.\nThis counter is cleared when read and freezes when reaches the maximum value.\n",
#endif
    { BBH_RX_PON_RMON_COUNTERS_R64_REG_OFFSET },
    0,
    0,
    248,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_PON_RMON_COUNTERS_R64_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_PON_RMON_COUNTERS_R127, TYPE: Type_BBH_RX_BBHRX_PON_RMON_COUNTERS_R127
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: R127 *****/
const ru_field_rec BBH_RX_PON_RMON_COUNTERS_R127_R127_FIELD =
{
    "R127",
#if RU_INCLUDE_DESC
    "",
    "This counter counts the number of R127 packets.\n",
#endif
    { BBH_RX_PON_RMON_COUNTERS_R127_R127_FIELD_MASK },
    0,
    { BBH_RX_PON_RMON_COUNTERS_R127_R127_FIELD_WIDTH },
    { BBH_RX_PON_RMON_COUNTERS_R127_R127_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_PON_RMON_COUNTERS_R127_FIELDS[] =
{
    &BBH_RX_PON_RMON_COUNTERS_R127_R127_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_PON_RMON_COUNTERS_R127 *****/
const ru_reg_rec BBH_RX_PON_RMON_COUNTERS_R127_REG =
{
    "PON_RMON_COUNTERS_R127",
#if RU_INCLUDE_DESC
    "R127 Register",
    "This counter counts the number of packets in the range of 65-127 Bytes.\nThis counter is cleared when read and freezes when reaches the maximum value.\n",
#endif
    { BBH_RX_PON_RMON_COUNTERS_R127_REG_OFFSET },
    0,
    0,
    249,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_PON_RMON_COUNTERS_R127_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_PON_RMON_COUNTERS_R255, TYPE: Type_BBH_RX_BBHRX_PON_RMON_COUNTERS_R255
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: R255 *****/
const ru_field_rec BBH_RX_PON_RMON_COUNTERS_R255_R255_FIELD =
{
    "R255",
#if RU_INCLUDE_DESC
    "",
    "This counter counts the number of R255 packets.\n",
#endif
    { BBH_RX_PON_RMON_COUNTERS_R255_R255_FIELD_MASK },
    0,
    { BBH_RX_PON_RMON_COUNTERS_R255_R255_FIELD_WIDTH },
    { BBH_RX_PON_RMON_COUNTERS_R255_R255_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_PON_RMON_COUNTERS_R255_FIELDS[] =
{
    &BBH_RX_PON_RMON_COUNTERS_R255_R255_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_PON_RMON_COUNTERS_R255 *****/
const ru_reg_rec BBH_RX_PON_RMON_COUNTERS_R255_REG =
{
    "PON_RMON_COUNTERS_R255",
#if RU_INCLUDE_DESC
    "R255 Register",
    "This counter counts the number of packets in the range of 128-255 Bytes.\nThis counter is cleared when read and freezes when reaches the maximum value.\n",
#endif
    { BBH_RX_PON_RMON_COUNTERS_R255_REG_OFFSET },
    0,
    0,
    250,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_PON_RMON_COUNTERS_R255_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_PON_RMON_COUNTERS_R511, TYPE: Type_BBH_RX_BBHRX_PON_RMON_COUNTERS_R511
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: R511 *****/
const ru_field_rec BBH_RX_PON_RMON_COUNTERS_R511_R511_FIELD =
{
    "R511",
#if RU_INCLUDE_DESC
    "",
    "This counter counts the number of R511 packets.\n",
#endif
    { BBH_RX_PON_RMON_COUNTERS_R511_R511_FIELD_MASK },
    0,
    { BBH_RX_PON_RMON_COUNTERS_R511_R511_FIELD_WIDTH },
    { BBH_RX_PON_RMON_COUNTERS_R511_R511_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_PON_RMON_COUNTERS_R511_FIELDS[] =
{
    &BBH_RX_PON_RMON_COUNTERS_R511_R511_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_PON_RMON_COUNTERS_R511 *****/
const ru_reg_rec BBH_RX_PON_RMON_COUNTERS_R511_REG =
{
    "PON_RMON_COUNTERS_R511",
#if RU_INCLUDE_DESC
    "R511 Register",
    "This counter counts the number of packets in the range of 256-511 Bytes.\nThis counter is cleared when read and freezes when reaches the maximum value.\n",
#endif
    { BBH_RX_PON_RMON_COUNTERS_R511_REG_OFFSET },
    0,
    0,
    251,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_PON_RMON_COUNTERS_R511_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_PON_RMON_COUNTERS_R1023, TYPE: Type_BBH_RX_BBHRX_PON_RMON_COUNTERS_R1023
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: R1023 *****/
const ru_field_rec BBH_RX_PON_RMON_COUNTERS_R1023_R1023_FIELD =
{
    "R1023",
#if RU_INCLUDE_DESC
    "",
    "This counter counts the number of R1023 packets.\n",
#endif
    { BBH_RX_PON_RMON_COUNTERS_R1023_R1023_FIELD_MASK },
    0,
    { BBH_RX_PON_RMON_COUNTERS_R1023_R1023_FIELD_WIDTH },
    { BBH_RX_PON_RMON_COUNTERS_R1023_R1023_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_PON_RMON_COUNTERS_R1023_FIELDS[] =
{
    &BBH_RX_PON_RMON_COUNTERS_R1023_R1023_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_PON_RMON_COUNTERS_R1023 *****/
const ru_reg_rec BBH_RX_PON_RMON_COUNTERS_R1023_REG =
{
    "PON_RMON_COUNTERS_R1023",
#if RU_INCLUDE_DESC
    "R1023 Register",
    "This counter counts the number of packets in the range of 512-1023 Bytes.\nThis counter is cleared when read and freezes when reaches the maximum value.\n",
#endif
    { BBH_RX_PON_RMON_COUNTERS_R1023_REG_OFFSET },
    0,
    0,
    252,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_PON_RMON_COUNTERS_R1023_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_PON_RMON_COUNTERS_R1518, TYPE: Type_BBH_RX_BBHRX_PON_RMON_COUNTERS_R1518
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: R1518 *****/
const ru_field_rec BBH_RX_PON_RMON_COUNTERS_R1518_R1518_FIELD =
{
    "R1518",
#if RU_INCLUDE_DESC
    "",
    "This counter counts the number of R1518 packets.\n",
#endif
    { BBH_RX_PON_RMON_COUNTERS_R1518_R1518_FIELD_MASK },
    0,
    { BBH_RX_PON_RMON_COUNTERS_R1518_R1518_FIELD_WIDTH },
    { BBH_RX_PON_RMON_COUNTERS_R1518_R1518_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_PON_RMON_COUNTERS_R1518_FIELDS[] =
{
    &BBH_RX_PON_RMON_COUNTERS_R1518_R1518_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_PON_RMON_COUNTERS_R1518 *****/
const ru_reg_rec BBH_RX_PON_RMON_COUNTERS_R1518_REG =
{
    "PON_RMON_COUNTERS_R1518",
#if RU_INCLUDE_DESC
    "R1518 Register",
    "This counter counts the number of packets in the range of 1024-1518 Bytes.\nThis counter is cleared when read and freezes when reaches the maximum value.\n",
#endif
    { BBH_RX_PON_RMON_COUNTERS_R1518_REG_OFFSET },
    0,
    0,
    253,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_PON_RMON_COUNTERS_R1518_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_PON_RMON_COUNTERS_R2047, TYPE: Type_BBH_RX_BBHRX_PON_RMON_COUNTERS_R2047
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: R1518 *****/
const ru_field_rec BBH_RX_PON_RMON_COUNTERS_R2047_R1518_FIELD =
{
    "R1518",
#if RU_INCLUDE_DESC
    "",
    "This counter counts the number of R1518 packets.\n",
#endif
    { BBH_RX_PON_RMON_COUNTERS_R2047_R1518_FIELD_MASK },
    0,
    { BBH_RX_PON_RMON_COUNTERS_R2047_R1518_FIELD_WIDTH },
    { BBH_RX_PON_RMON_COUNTERS_R2047_R1518_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_PON_RMON_COUNTERS_R2047_FIELDS[] =
{
    &BBH_RX_PON_RMON_COUNTERS_R2047_R1518_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_PON_RMON_COUNTERS_R2047 *****/
const ru_reg_rec BBH_RX_PON_RMON_COUNTERS_R2047_REG =
{
    "PON_RMON_COUNTERS_R2047",
#if RU_INCLUDE_DESC
    "R2047 Register",
    "This counter counts the number of packets in the range of 1519-2047 Bytes.\nThis counter is cleared when read and freezes when reaches the maximum value.\n",
#endif
    { BBH_RX_PON_RMON_COUNTERS_R2047_REG_OFFSET },
    0,
    0,
    254,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_PON_RMON_COUNTERS_R2047_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_PON_RMON_COUNTERS_R4095, TYPE: Type_BBH_RX_BBHRX_PON_RMON_COUNTERS_R4095
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: R4095 *****/
const ru_field_rec BBH_RX_PON_RMON_COUNTERS_R4095_R4095_FIELD =
{
    "R4095",
#if RU_INCLUDE_DESC
    "",
    "This counter counts the number of R4095 packets.\n",
#endif
    { BBH_RX_PON_RMON_COUNTERS_R4095_R4095_FIELD_MASK },
    0,
    { BBH_RX_PON_RMON_COUNTERS_R4095_R4095_FIELD_WIDTH },
    { BBH_RX_PON_RMON_COUNTERS_R4095_R4095_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_PON_RMON_COUNTERS_R4095_FIELDS[] =
{
    &BBH_RX_PON_RMON_COUNTERS_R4095_R4095_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_PON_RMON_COUNTERS_R4095 *****/
const ru_reg_rec BBH_RX_PON_RMON_COUNTERS_R4095_REG =
{
    "PON_RMON_COUNTERS_R4095",
#if RU_INCLUDE_DESC
    "R4095 Register",
    "This counter counts the number of packets in the range of 2048-4095 Bytes.\nThis counter is cleared when read and freezes when reaches the maximum value.\n",
#endif
    { BBH_RX_PON_RMON_COUNTERS_R4095_REG_OFFSET },
    0,
    0,
    255,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_PON_RMON_COUNTERS_R4095_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_PON_RMON_COUNTERS_R9216, TYPE: Type_BBH_RX_BBHRX_PON_RMON_COUNTERS_R9216
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: R9216 *****/
const ru_field_rec BBH_RX_PON_RMON_COUNTERS_R9216_R9216_FIELD =
{
    "R9216",
#if RU_INCLUDE_DESC
    "",
    "This counter counts the number of R9216 packets.\n",
#endif
    { BBH_RX_PON_RMON_COUNTERS_R9216_R9216_FIELD_MASK },
    0,
    { BBH_RX_PON_RMON_COUNTERS_R9216_R9216_FIELD_WIDTH },
    { BBH_RX_PON_RMON_COUNTERS_R9216_R9216_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_PON_RMON_COUNTERS_R9216_FIELDS[] =
{
    &BBH_RX_PON_RMON_COUNTERS_R9216_R9216_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_PON_RMON_COUNTERS_R9216 *****/
const ru_reg_rec BBH_RX_PON_RMON_COUNTERS_R9216_REG =
{
    "PON_RMON_COUNTERS_R9216",
#if RU_INCLUDE_DESC
    "R9216 Register",
    "This counter counts the number of packets in the range of 4096-9216 Bytes.\nThis counter is cleared when read and freezes when reaches the maximum value.\n",
#endif
    { BBH_RX_PON_RMON_COUNTERS_R9216_REG_OFFSET },
    0,
    0,
    256,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_PON_RMON_COUNTERS_R9216_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_PON_RMON_COUNTERS_R16383, TYPE: Type_BBH_RX_BBHRX_PON_RMON_COUNTERS_R16383
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: R16383 *****/
const ru_field_rec BBH_RX_PON_RMON_COUNTERS_R16383_R16383_FIELD =
{
    "R16383",
#if RU_INCLUDE_DESC
    "",
    "This counter counts the number of R18383 packets.\n",
#endif
    { BBH_RX_PON_RMON_COUNTERS_R16383_R16383_FIELD_MASK },
    0,
    { BBH_RX_PON_RMON_COUNTERS_R16383_R16383_FIELD_WIDTH },
    { BBH_RX_PON_RMON_COUNTERS_R16383_R16383_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_PON_RMON_COUNTERS_R16383_FIELDS[] =
{
    &BBH_RX_PON_RMON_COUNTERS_R16383_R16383_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_PON_RMON_COUNTERS_R16383 *****/
const ru_reg_rec BBH_RX_PON_RMON_COUNTERS_R16383_REG =
{
    "PON_RMON_COUNTERS_R16383",
#if RU_INCLUDE_DESC
    "R16383 Register",
    "This counter counts the number of packets in the range of 9217-16383 Bytes.\nThis counter is cleared when read and freezes when reaches the maximum value.\n",
#endif
    { BBH_RX_PON_RMON_COUNTERS_R16383_REG_OFFSET },
    0,
    0,
    257,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_PON_RMON_COUNTERS_R16383_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_DEBUG_PFBUW, TYPE: Type_BBH_RX_BBHRX_DEBUG_PFBUW
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: UW *****/
const ru_field_rec BBH_RX_DEBUG_PFBUW_UW_FIELD =
{
    "UW",
#if RU_INCLUDE_DESC
    "",
    "Used words\n",
#endif
    { BBH_RX_DEBUG_PFBUW_UW_FIELD_MASK },
    0,
    { BBH_RX_DEBUG_PFBUW_UW_FIELD_WIDTH },
    { BBH_RX_DEBUG_PFBUW_UW_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_DEBUG_PFBUW_FIELDS[] =
{
    &BBH_RX_DEBUG_PFBUW_UW_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_DEBUG_PFBUW *****/
const ru_reg_rec BBH_RX_DEBUG_PFBUW_REG =
{
    "DEBUG_PFBUW",
#if RU_INCLUDE_DESC
    "PFB_FIFO_USED_WORDS 0..7 Register",
    "PFB FIFO used words\n",
#endif
    { BBH_RX_DEBUG_PFBUW_REG_OFFSET },
    BBH_RX_DEBUG_PFBUW_REG_RAM_CNT,
    4,
    258,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_DEBUG_PFBUW_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_DEBUG_PATTERNUW, TYPE: Type_BBH_RX_BBHRX_DEBUG_PATTERNUW
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: UW *****/
const ru_field_rec BBH_RX_DEBUG_PATTERNUW_UW_FIELD =
{
    "UW",
#if RU_INCLUDE_DESC
    "",
    "Used words\n",
#endif
    { BBH_RX_DEBUG_PATTERNUW_UW_FIELD_MASK },
    0,
    { BBH_RX_DEBUG_PATTERNUW_UW_FIELD_WIDTH },
    { BBH_RX_DEBUG_PATTERNUW_UW_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_DEBUG_PATTERNUW_FIELDS[] =
{
    &BBH_RX_DEBUG_PATTERNUW_UW_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_DEBUG_PATTERNUW *****/
const ru_reg_rec BBH_RX_DEBUG_PATTERNUW_REG =
{
    "DEBUG_PATTERNUW",
#if RU_INCLUDE_DESC
    "PATTERN_MATCH_FIFO_USED_WORDS 0..7 Register",
    "Pattern Match FIFO used words\n",
#endif
    { BBH_RX_DEBUG_PATTERNUW_REG_OFFSET },
    BBH_RX_DEBUG_PATTERNUW_REG_RAM_CNT,
    4,
    259,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_DEBUG_PATTERNUW_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_DEBUG_EXCUW, TYPE: Type_BBH_RX_BBHRX_DEBUG_EXCUW
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: UW *****/
const ru_field_rec BBH_RX_DEBUG_EXCUW_UW_FIELD =
{
    "UW",
#if RU_INCLUDE_DESC
    "",
    "Used words\n",
#endif
    { BBH_RX_DEBUG_EXCUW_UW_FIELD_MASK },
    0,
    { BBH_RX_DEBUG_EXCUW_UW_FIELD_WIDTH },
    { BBH_RX_DEBUG_EXCUW_UW_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_DEBUG_EXCUW_FIELDS[] =
{
    &BBH_RX_DEBUG_EXCUW_UW_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_DEBUG_EXCUW *****/
const ru_reg_rec BBH_RX_DEBUG_EXCUW_REG =
{
    "DEBUG_EXCUW",
#if RU_INCLUDE_DESC
    "EXC_RESULT_FIFO_USED_WORDS 0..7 Register",
    "Exc result FIFO used words\n",
#endif
    { BBH_RX_DEBUG_EXCUW_REG_OFFSET },
    BBH_RX_DEBUG_EXCUW_REG_RAM_CNT,
    4,
    260,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_DEBUG_EXCUW_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_DEBUG_PORTUW, TYPE: Type_BBH_RX_BBHRX_DEBUG_PORTUW
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: UW *****/
const ru_field_rec BBH_RX_DEBUG_PORTUW_UW_FIELD =
{
    "UW",
#if RU_INCLUDE_DESC
    "",
    "Used words\n",
#endif
    { BBH_RX_DEBUG_PORTUW_UW_FIELD_MASK },
    0,
    { BBH_RX_DEBUG_PORTUW_UW_FIELD_WIDTH },
    { BBH_RX_DEBUG_PORTUW_UW_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_DEBUG_PORTUW_FIELDS[] =
{
    &BBH_RX_DEBUG_PORTUW_UW_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_DEBUG_PORTUW *****/
const ru_reg_rec BBH_RX_DEBUG_PORTUW_REG =
{
    "DEBUG_PORTUW",
#if RU_INCLUDE_DESC
    "PORT_FIFO_USED_WORDS 0..7 Register",
    "Port FIFO used words\n",
#endif
    { BBH_RX_DEBUG_PORTUW_REG_OFFSET },
    BBH_RX_DEBUG_PORTUW_REG_RAM_CNT,
    4,
    261,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_DEBUG_PORTUW_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_DEBUG_FIRSTSBN, TYPE: Type_BBH_RX_BBHRX_DEBUG_FIRSTSBN
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CONTEXT *****/
const ru_field_rec BBH_RX_DEBUG_FIRSTSBN_CONTEXT_FIELD =
{
    "CONTEXT",
#if RU_INCLUDE_DESC
    "",
    "Context value\n",
#endif
    { BBH_RX_DEBUG_FIRSTSBN_CONTEXT_FIELD_MASK },
    0,
    { BBH_RX_DEBUG_FIRSTSBN_CONTEXT_FIELD_WIDTH },
    { BBH_RX_DEBUG_FIRSTSBN_CONTEXT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_DEBUG_FIRSTSBN_FIELDS[] =
{
    &BBH_RX_DEBUG_FIRSTSBN_CONTEXT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_DEBUG_FIRSTSBN *****/
const ru_reg_rec BBH_RX_DEBUG_FIRSTSBN_REG =
{
    "DEBUG_FIRSTSBN",
#if RU_INCLUDE_DESC
    "FIRST_SBN 0..7 Register",
    "First SBN context per port\n",
#endif
    { BBH_RX_DEBUG_FIRSTSBN_REG_OFFSET },
    BBH_RX_DEBUG_FIRSTSBN_REG_RAM_CNT,
    4,
    262,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_DEBUG_FIRSTSBN_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_DEBUG_CURRENTSBN, TYPE: Type_BBH_RX_BBHRX_DEBUG_CURRENTSBN
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CONTEXT *****/
const ru_field_rec BBH_RX_DEBUG_CURRENTSBN_CONTEXT_FIELD =
{
    "CONTEXT",
#if RU_INCLUDE_DESC
    "",
    "Context value\n",
#endif
    { BBH_RX_DEBUG_CURRENTSBN_CONTEXT_FIELD_MASK },
    0,
    { BBH_RX_DEBUG_CURRENTSBN_CONTEXT_FIELD_WIDTH },
    { BBH_RX_DEBUG_CURRENTSBN_CONTEXT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_DEBUG_CURRENTSBN_FIELDS[] =
{
    &BBH_RX_DEBUG_CURRENTSBN_CONTEXT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_DEBUG_CURRENTSBN *****/
const ru_reg_rec BBH_RX_DEBUG_CURRENTSBN_REG =
{
    "DEBUG_CURRENTSBN",
#if RU_INCLUDE_DESC
    "CURRENT_SBN 0..7 Register",
    "Current SBN context per port\n",
#endif
    { BBH_RX_DEBUG_CURRENTSBN_REG_OFFSET },
    BBH_RX_DEBUG_CURRENTSBN_REG_RAM_CNT,
    4,
    263,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_DEBUG_CURRENTSBN_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_DEBUG_SN, TYPE: Type_BBH_RX_BBHRX_DEBUG_SN
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CONTEXT *****/
const ru_field_rec BBH_RX_DEBUG_SN_CONTEXT_FIELD =
{
    "CONTEXT",
#if RU_INCLUDE_DESC
    "",
    "Context value\n",
#endif
    { BBH_RX_DEBUG_SN_CONTEXT_FIELD_MASK },
    0,
    { BBH_RX_DEBUG_SN_CONTEXT_FIELD_WIDTH },
    { BBH_RX_DEBUG_SN_CONTEXT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_DEBUG_SN_FIELDS[] =
{
    &BBH_RX_DEBUG_SN_CONTEXT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_DEBUG_SN *****/
const ru_reg_rec BBH_RX_DEBUG_SN_REG =
{
    "DEBUG_SN",
#if RU_INCLUDE_DESC
    "SN 0..7 Register",
    "SN (Serial Number) context per port\n",
#endif
    { BBH_RX_DEBUG_SN_REG_OFFSET },
    BBH_RX_DEBUG_SN_REG_RAM_CNT,
    4,
    264,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_DEBUG_SN_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_DEBUG_VIQMASK, TYPE: Type_BBH_RX_BBHRX_DEBUG_VIQMASK
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CONTEXT *****/
const ru_field_rec BBH_RX_DEBUG_VIQMASK_CONTEXT_FIELD =
{
    "CONTEXT",
#if RU_INCLUDE_DESC
    "",
    "Context value\n",
#endif
    { BBH_RX_DEBUG_VIQMASK_CONTEXT_FIELD_MASK },
    0,
    { BBH_RX_DEBUG_VIQMASK_CONTEXT_FIELD_WIDTH },
    { BBH_RX_DEBUG_VIQMASK_CONTEXT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_DEBUG_VIQMASK_FIELDS[] =
{
    &BBH_RX_DEBUG_VIQMASK_CONTEXT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_DEBUG_VIQMASK *****/
const ru_reg_rec BBH_RX_DEBUG_VIQMASK_REG =
{
    "DEBUG_VIQMASK",
#if RU_INCLUDE_DESC
    "VIQ_MASK 0..7 Register",
    "VIQ mask context per port\n",
#endif
    { BBH_RX_DEBUG_VIQMASK_REG_OFFSET },
    BBH_RX_DEBUG_VIQMASK_REG_RAM_CNT,
    4,
    265,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_DEBUG_VIQMASK_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_DEBUG_PLEN, TYPE: Type_BBH_RX_BBHRX_DEBUG_PLEN
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CONTEXT *****/
const ru_field_rec BBH_RX_DEBUG_PLEN_CONTEXT_FIELD =
{
    "CONTEXT",
#if RU_INCLUDE_DESC
    "",
    "Context value\n",
#endif
    { BBH_RX_DEBUG_PLEN_CONTEXT_FIELD_MASK },
    0,
    { BBH_RX_DEBUG_PLEN_CONTEXT_FIELD_WIDTH },
    { BBH_RX_DEBUG_PLEN_CONTEXT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_DEBUG_PLEN_FIELDS[] =
{
    &BBH_RX_DEBUG_PLEN_CONTEXT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_DEBUG_PLEN *****/
const ru_reg_rec BBH_RX_DEBUG_PLEN_REG =
{
    "DEBUG_PLEN",
#if RU_INCLUDE_DESC
    "PLEN 0..7 Register",
    "PLEN context per port\n",
#endif
    { BBH_RX_DEBUG_PLEN_REG_OFFSET },
    BBH_RX_DEBUG_PLEN_REG_RAM_CNT,
    4,
    266,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_DEBUG_PLEN_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_DEBUG_EOPOK, TYPE: Type_BBH_RX_BBHRX_DEBUG_EOPOK
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CONTEXT *****/
const ru_field_rec BBH_RX_DEBUG_EOPOK_CONTEXT_FIELD =
{
    "CONTEXT",
#if RU_INCLUDE_DESC
    "",
    "Context value - bit per port.\nbit 0 for Port 0\n...\nBit 7 for port 7\n",
#endif
    { BBH_RX_DEBUG_EOPOK_CONTEXT_FIELD_MASK },
    0,
    { BBH_RX_DEBUG_EOPOK_CONTEXT_FIELD_WIDTH },
    { BBH_RX_DEBUG_EOPOK_CONTEXT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_DEBUG_EOPOK_FIELDS[] =
{
    &BBH_RX_DEBUG_EOPOK_CONTEXT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_DEBUG_EOPOK *****/
const ru_reg_rec BBH_RX_DEBUG_EOPOK_REG =
{
    "DEBUG_EOPOK",
#if RU_INCLUDE_DESC
    "EOP_OK Register",
    "EOP ok per port\n",
#endif
    { BBH_RX_DEBUG_EOPOK_REG_OFFSET },
    0,
    0,
    267,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_DEBUG_EOPOK_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_DEBUG_VIQFULL, TYPE: Type_BBH_RX_BBHRX_DEBUG_VIQFULL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CONTEXT *****/
const ru_field_rec BBH_RX_DEBUG_VIQFULL_CONTEXT_FIELD =
{
    "CONTEXT",
#if RU_INCLUDE_DESC
    "",
    "Context value - bit per port.\nbit 0 for Port 0\n...\nBit 7 for port 7\n",
#endif
    { BBH_RX_DEBUG_VIQFULL_CONTEXT_FIELD_MASK },
    0,
    { BBH_RX_DEBUG_VIQFULL_CONTEXT_FIELD_WIDTH },
    { BBH_RX_DEBUG_VIQFULL_CONTEXT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_DEBUG_VIQFULL_FIELDS[] =
{
    &BBH_RX_DEBUG_VIQFULL_CONTEXT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_DEBUG_VIQFULL *****/
const ru_reg_rec BBH_RX_DEBUG_VIQFULL_REG =
{
    "DEBUG_VIQFULL",
#if RU_INCLUDE_DESC
    "VIQ_FULL Register",
    "VIQ full per port\n",
#endif
    { BBH_RX_DEBUG_VIQFULL_REG_OFFSET },
    0,
    0,
    268,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_DEBUG_VIQFULL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_DEBUG_PORTREQ, TYPE: Type_BBH_RX_BBHRX_DEBUG_PORTREQ
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CONTEXT *****/
const ru_field_rec BBH_RX_DEBUG_PORTREQ_CONTEXT_FIELD =
{
    "CONTEXT",
#if RU_INCLUDE_DESC
    "",
    "Context value - bit per port.\nbit 0 for Port 0\n...\nBit 7 for port 7\n",
#endif
    { BBH_RX_DEBUG_PORTREQ_CONTEXT_FIELD_MASK },
    0,
    { BBH_RX_DEBUG_PORTREQ_CONTEXT_FIELD_WIDTH },
    { BBH_RX_DEBUG_PORTREQ_CONTEXT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_DEBUG_PORTREQ_FIELDS[] =
{
    &BBH_RX_DEBUG_PORTREQ_CONTEXT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_DEBUG_PORTREQ *****/
const ru_reg_rec BBH_RX_DEBUG_PORTREQ_REG =
{
    "DEBUG_PORTREQ",
#if RU_INCLUDE_DESC
    "PORT_FIFO_REQ Register",
    "Port FIFO req per port\n",
#endif
    { BBH_RX_DEBUG_PORTREQ_REG_OFFSET },
    0,
    0,
    269,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_DEBUG_PORTREQ_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_DEBUG_DUMMYREQ, TYPE: Type_BBH_RX_BBHRX_DEBUG_DUMMYREQ
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CONTEXT *****/
const ru_field_rec BBH_RX_DEBUG_DUMMYREQ_CONTEXT_FIELD =
{
    "CONTEXT",
#if RU_INCLUDE_DESC
    "",
    "Context value - bit per port.\nbit 0 for Port 0\n...\nBit 7 for port 7\n",
#endif
    { BBH_RX_DEBUG_DUMMYREQ_CONTEXT_FIELD_MASK },
    0,
    { BBH_RX_DEBUG_DUMMYREQ_CONTEXT_FIELD_WIDTH },
    { BBH_RX_DEBUG_DUMMYREQ_CONTEXT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_DEBUG_DUMMYREQ_FIELDS[] =
{
    &BBH_RX_DEBUG_DUMMYREQ_CONTEXT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_DEBUG_DUMMYREQ *****/
const ru_reg_rec BBH_RX_DEBUG_DUMMYREQ_REG =
{
    "DEBUG_DUMMYREQ",
#if RU_INCLUDE_DESC
    "PORT_DUMMY_REQ Register",
    "Port dummy req per port\n",
#endif
    { BBH_RX_DEBUG_DUMMYREQ_REG_OFFSET },
    0,
    0,
    270,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_DEBUG_DUMMYREQ_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_DEBUG_INREASSEMBLY, TYPE: Type_BBH_RX_BBHRX_DEBUG_INREASSEMBLY
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CONTEXT *****/
const ru_field_rec BBH_RX_DEBUG_INREASSEMBLY_CONTEXT_FIELD =
{
    "CONTEXT",
#if RU_INCLUDE_DESC
    "",
    "Context value - bit per port.\nbit 0 for Port 0\n...\nBit 7 for port 7\n",
#endif
    { BBH_RX_DEBUG_INREASSEMBLY_CONTEXT_FIELD_MASK },
    0,
    { BBH_RX_DEBUG_INREASSEMBLY_CONTEXT_FIELD_WIDTH },
    { BBH_RX_DEBUG_INREASSEMBLY_CONTEXT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_DEBUG_INREASSEMBLY_FIELDS[] =
{
    &BBH_RX_DEBUG_INREASSEMBLY_CONTEXT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_DEBUG_INREASSEMBLY *****/
const ru_reg_rec BBH_RX_DEBUG_INREASSEMBLY_REG =
{
    "DEBUG_INREASSEMBLY",
#if RU_INCLUDE_DESC
    "IN_REASSMBLY Register",
    "In reassembly per port\n",
#endif
    { BBH_RX_DEBUG_INREASSEMBLY_REG_OFFSET },
    0,
    0,
    271,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_DEBUG_INREASSEMBLY_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_DEBUG_DUMMYPD, TYPE: Type_BBH_RX_BBHRX_DEBUG_DUMMYPD
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CONTEXT *****/
const ru_field_rec BBH_RX_DEBUG_DUMMYPD_CONTEXT_FIELD =
{
    "CONTEXT",
#if RU_INCLUDE_DESC
    "",
    "Context value - bit per port.\nbit 0 for Port 0\n...\nBit 7 for port 7\n",
#endif
    { BBH_RX_DEBUG_DUMMYPD_CONTEXT_FIELD_MASK },
    0,
    { BBH_RX_DEBUG_DUMMYPD_CONTEXT_FIELD_WIDTH },
    { BBH_RX_DEBUG_DUMMYPD_CONTEXT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_DEBUG_DUMMYPD_FIELDS[] =
{
    &BBH_RX_DEBUG_DUMMYPD_CONTEXT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_DEBUG_DUMMYPD *****/
const ru_reg_rec BBH_RX_DEBUG_DUMMYPD_REG =
{
    "DEBUG_DUMMYPD",
#if RU_INCLUDE_DESC
    "DUMMY_PD Register",
    "Dummy PD per port\n",
#endif
    { BBH_RX_DEBUG_DUMMYPD_REG_OFFSET },
    0,
    0,
    272,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_DEBUG_DUMMYPD_FIELDS,
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
    273,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_DEBUG_UFUW_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_DEBUG_PENDCONNECTUW, TYPE: Type_BBH_RX_BBHRX_DEBUG_PENDCONNECTUW
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: UW *****/
const ru_field_rec BBH_RX_DEBUG_PENDCONNECTUW_UW_FIELD =
{
    "UW",
#if RU_INCLUDE_DESC
    "",
    "Used words\n",
#endif
    { BBH_RX_DEBUG_PENDCONNECTUW_UW_FIELD_MASK },
    0,
    { BBH_RX_DEBUG_PENDCONNECTUW_UW_FIELD_WIDTH },
    { BBH_RX_DEBUG_PENDCONNECTUW_UW_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_DEBUG_PENDCONNECTUW_FIELDS[] =
{
    &BBH_RX_DEBUG_PENDCONNECTUW_UW_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_DEBUG_PENDCONNECTUW *****/
const ru_reg_rec BBH_RX_DEBUG_PENDCONNECTUW_REG =
{
    "DEBUG_PENDCONNECTUW",
#if RU_INCLUDE_DESC
    "PENDING_CONNECT Register",
    "Pending connect FIFO used words\n",
#endif
    { BBH_RX_DEBUG_PENDCONNECTUW_REG_OFFSET },
    0,
    0,
    274,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_DEBUG_PENDCONNECTUW_FIELDS,
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
    "DEBUG vector.\nCHANGED_BY_HW\n\n",
#endif
    { BBH_RX_DEBUG_DBGVEC_DBGVEC_FIELD_MASK },
    0,
    { BBH_RX_DEBUG_DBGVEC_DBGVEC_FIELD_WIDTH },
    { BBH_RX_DEBUG_DBGVEC_DBGVEC_FIELD_SHIFT },
    0,
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
    "DEBUG_VECTOR 0..27 Register",
    "Debug vectors\n",
#endif
    { BBH_RX_DEBUG_DBGVEC_REG_OFFSET },
    BBH_RX_DEBUG_DBGVEC_REG_RAM_CNT,
    4,
    275,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_DEBUG_DBGVEC_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_DEBUG_PDUW, TYPE: Type_BBH_RX_BBHRX_DEBUG_PDUW
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: UW *****/
const ru_field_rec BBH_RX_DEBUG_PDUW_UW_FIELD =
{
    "UW",
#if RU_INCLUDE_DESC
    "",
    "Used words\n",
#endif
    { BBH_RX_DEBUG_PDUW_UW_FIELD_MASK },
    0,
    { BBH_RX_DEBUG_PDUW_UW_FIELD_WIDTH },
    { BBH_RX_DEBUG_PDUW_UW_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_DEBUG_PDUW_FIELDS[] =
{
    &BBH_RX_DEBUG_PDUW_UW_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_DEBUG_PDUW *****/
const ru_reg_rec BBH_RX_DEBUG_PDUW_REG =
{
    "DEBUG_PDUW",
#if RU_INCLUDE_DESC
    "PD_FIFO_USED_WORDS 0..8 Register",
    "PD FIFO used words\n",
#endif
    { BBH_RX_DEBUG_PDUW_REG_OFFSET },
    BBH_RX_DEBUG_PDUW_REG_RAM_CNT,
    4,
    276,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_DEBUG_PDUW_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_DEBUG_SBPMACK, TYPE: Type_BBH_RX_BBHRX_DEBUG_SBPMACK
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ACKCNT *****/
const ru_field_rec BBH_RX_DEBUG_SBPMACK_ACKCNT_FIELD =
{
    "ACKCNT",
#if RU_INCLUDE_DESC
    "",
    "ACK counter\n",
#endif
    { BBH_RX_DEBUG_SBPMACK_ACKCNT_FIELD_MASK },
    0,
    { BBH_RX_DEBUG_SBPMACK_ACKCNT_FIELD_WIDTH },
    { BBH_RX_DEBUG_SBPMACK_ACKCNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_DEBUG_SBPMACK_FIELDS[] =
{
    &BBH_RX_DEBUG_SBPMACK_ACKCNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_DEBUG_SBPMACK *****/
const ru_reg_rec BBH_RX_DEBUG_SBPMACK_REG =
{
    "DEBUG_SBPMACK",
#if RU_INCLUDE_DESC
    "SBPM_CONNECT_ACK 0..8 Register",
    "SBPM connect ACK counter per VIQ\n",
#endif
    { BBH_RX_DEBUG_SBPMACK_REG_OFFSET },
    BBH_RX_DEBUG_SBPMACK_REG_RAM_CNT,
    4,
    277,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_DEBUG_SBPMACK_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_DEBUG_WRITEREPLYACK, TYPE: Type_BBH_RX_BBHRX_DEBUG_WRITEREPLYACK
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ACKCNT *****/
const ru_field_rec BBH_RX_DEBUG_WRITEREPLYACK_ACKCNT_FIELD =
{
    "ACKCNT",
#if RU_INCLUDE_DESC
    "",
    "ACK counter\n",
#endif
    { BBH_RX_DEBUG_WRITEREPLYACK_ACKCNT_FIELD_MASK },
    0,
    { BBH_RX_DEBUG_WRITEREPLYACK_ACKCNT_FIELD_WIDTH },
    { BBH_RX_DEBUG_WRITEREPLYACK_ACKCNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_DEBUG_WRITEREPLYACK_FIELDS[] =
{
    &BBH_RX_DEBUG_WRITEREPLYACK_ACKCNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_DEBUG_WRITEREPLYACK *****/
const ru_reg_rec BBH_RX_DEBUG_WRITEREPLYACK_REG =
{
    "DEBUG_WRITEREPLYACK",
#if RU_INCLUDE_DESC
    "WRITE_REPLY_ACK 0..8 Register",
    "Write reply ACK counter per VIQ\n",
#endif
    { BBH_RX_DEBUG_WRITEREPLYACK_REG_OFFSET },
    BBH_RX_DEBUG_WRITEREPLYACK_REG_RAM_CNT,
    4,
    278,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_DEBUG_WRITEREPLYACK_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_DEBUG_DISPCREDITACK, TYPE: Type_BBH_RX_BBHRX_DEBUG_DISPCREDITACK
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ACKCNT *****/
const ru_field_rec BBH_RX_DEBUG_DISPCREDITACK_ACKCNT_FIELD =
{
    "ACKCNT",
#if RU_INCLUDE_DESC
    "",
    "ACK counter\n",
#endif
    { BBH_RX_DEBUG_DISPCREDITACK_ACKCNT_FIELD_MASK },
    0,
    { BBH_RX_DEBUG_DISPCREDITACK_ACKCNT_FIELD_WIDTH },
    { BBH_RX_DEBUG_DISPCREDITACK_ACKCNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_DEBUG_DISPCREDITACK_FIELDS[] =
{
    &BBH_RX_DEBUG_DISPCREDITACK_ACKCNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_DEBUG_DISPCREDITACK *****/
const ru_reg_rec BBH_RX_DEBUG_DISPCREDITACK_REG =
{
    "DEBUG_DISPCREDITACK",
#if RU_INCLUDE_DESC
    "DISP_CREDIT_ACK 0..8 Register",
    "Dispatcher credits ACK counter per VIQ\n",
#endif
    { BBH_RX_DEBUG_DISPCREDITACK_REG_OFFSET },
    BBH_RX_DEBUG_DISPCREDITACK_REG_RAM_CNT,
    4,
    279,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_DEBUG_DISPCREDITACK_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_DEBUG_PDRDDATA, TYPE: Type_BBH_RX_BBHRX_DEBUG_PDRDDATA
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RDDATA *****/
const ru_field_rec BBH_RX_DEBUG_PDRDDATA_RDDATA_FIELD =
{
    "RDDATA",
#if RU_INCLUDE_DESC
    "",
    "Read Data\n",
#endif
    { BBH_RX_DEBUG_PDRDDATA_RDDATA_FIELD_MASK },
    0,
    { BBH_RX_DEBUG_PDRDDATA_RDDATA_FIELD_WIDTH },
    { BBH_RX_DEBUG_PDRDDATA_RDDATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_DEBUG_PDRDDATA_FIELDS[] =
{
    &BBH_RX_DEBUG_PDRDDATA_RDDATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_DEBUG_PDRDDATA *****/
const ru_reg_rec BBH_RX_DEBUG_PDRDDATA_REG =
{
    "DEBUG_PDRDDATA",
#if RU_INCLUDE_DESC
    "PD_FIFO_RD_DATA 0..35 Register",
    "PD FIFO read data per VIQ.\nReflects the PD at the head of the PD FIFO.\nPD size is 128 bits, therefore there are 4 indexes pre VIQ FIFO:\nfor n in [0..8]\nVIQ n: [n*4..(n+1)*4-1]\nwhere:\nn*4 reflects the 32 LSBits\n(n+1)*4-1 reflects the 32 MSBits\n\n",
#endif
    { BBH_RX_DEBUG_PDRDDATA_REG_OFFSET },
    BBH_RX_DEBUG_PDRDDATA_REG_RAM_CNT,
    4,
    280,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_DEBUG_PDRDDATA_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_DEBUG_SBNFIFO, TYPE: Type_BBH_RX_BBHRX_DEBUG_SBNFIFO
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ODDBNENTRY *****/
const ru_field_rec BBH_RX_DEBUG_SBNFIFO_ODDBNENTRY_FIELD =
{
    "ODDBNENTRY",
#if RU_INCLUDE_DESC
    "",
    "BN\n",
#endif
    { BBH_RX_DEBUG_SBNFIFO_ODDBNENTRY_FIELD_MASK },
    0,
    { BBH_RX_DEBUG_SBNFIFO_ODDBNENTRY_FIELD_WIDTH },
    { BBH_RX_DEBUG_SBNFIFO_ODDBNENTRY_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ODDVALID *****/
const ru_field_rec BBH_RX_DEBUG_SBNFIFO_ODDVALID_FIELD =
{
    "ODDVALID",
#if RU_INCLUDE_DESC
    "",
    "SBN is Valid\n",
#endif
    { BBH_RX_DEBUG_SBNFIFO_ODDVALID_FIELD_MASK },
    0,
    { BBH_RX_DEBUG_SBNFIFO_ODDVALID_FIELD_WIDTH },
    { BBH_RX_DEBUG_SBNFIFO_ODDVALID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EVENBNENTRY *****/
const ru_field_rec BBH_RX_DEBUG_SBNFIFO_EVENBNENTRY_FIELD =
{
    "EVENBNENTRY",
#if RU_INCLUDE_DESC
    "",
    "BN\n",
#endif
    { BBH_RX_DEBUG_SBNFIFO_EVENBNENTRY_FIELD_MASK },
    0,
    { BBH_RX_DEBUG_SBNFIFO_EVENBNENTRY_FIELD_WIDTH },
    { BBH_RX_DEBUG_SBNFIFO_EVENBNENTRY_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EVENVALID *****/
const ru_field_rec BBH_RX_DEBUG_SBNFIFO_EVENVALID_FIELD =
{
    "EVENVALID",
#if RU_INCLUDE_DESC
    "",
    "SBN is Valid\n",
#endif
    { BBH_RX_DEBUG_SBNFIFO_EVENVALID_FIELD_MASK },
    0,
    { BBH_RX_DEBUG_SBNFIFO_EVENVALID_FIELD_WIDTH },
    { BBH_RX_DEBUG_SBNFIFO_EVENVALID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_DEBUG_SBNFIFO_FIELDS[] =
{
    &BBH_RX_DEBUG_SBNFIFO_ODDBNENTRY_FIELD,
    &BBH_RX_DEBUG_SBNFIFO_ODDVALID_FIELD,
    &BBH_RX_DEBUG_SBNFIFO_EVENBNENTRY_FIELD,
    &BBH_RX_DEBUG_SBNFIFO_EVENVALID_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_DEBUG_SBNFIFO *****/
const ru_reg_rec BBH_RX_DEBUG_SBNFIFO_REG =
{
    "DEBUG_SBNFIFO",
#if RU_INCLUDE_DESC
    "SRAM_BN_FIFO 0..31 Register",
    "The BBH RX hold a FIFO with 64 BN.\n",
#endif
    { BBH_RX_DEBUG_SBNFIFO_REG_OFFSET },
    BBH_RX_DEBUG_SBNFIFO_REG_RAM_CNT,
    4,
    281,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    BBH_RX_DEBUG_SBNFIFO_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_DEBUG_SBNPREFETCH, TYPE: Type_BBH_RX_BBHRX_DEBUG_SBNPREFETCH
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ODDBNENTRY *****/
const ru_field_rec BBH_RX_DEBUG_SBNPREFETCH_ODDBNENTRY_FIELD =
{
    "ODDBNENTRY",
#if RU_INCLUDE_DESC
    "",
    "BN\n",
#endif
    { BBH_RX_DEBUG_SBNPREFETCH_ODDBNENTRY_FIELD_MASK },
    0,
    { BBH_RX_DEBUG_SBNPREFETCH_ODDBNENTRY_FIELD_WIDTH },
    { BBH_RX_DEBUG_SBNPREFETCH_ODDBNENTRY_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ODDVALID *****/
const ru_field_rec BBH_RX_DEBUG_SBNPREFETCH_ODDVALID_FIELD =
{
    "ODDVALID",
#if RU_INCLUDE_DESC
    "",
    "SBN is Valid\n",
#endif
    { BBH_RX_DEBUG_SBNPREFETCH_ODDVALID_FIELD_MASK },
    0,
    { BBH_RX_DEBUG_SBNPREFETCH_ODDVALID_FIELD_WIDTH },
    { BBH_RX_DEBUG_SBNPREFETCH_ODDVALID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EVENBNENTRY *****/
const ru_field_rec BBH_RX_DEBUG_SBNPREFETCH_EVENBNENTRY_FIELD =
{
    "EVENBNENTRY",
#if RU_INCLUDE_DESC
    "",
    "BN\n",
#endif
    { BBH_RX_DEBUG_SBNPREFETCH_EVENBNENTRY_FIELD_MASK },
    0,
    { BBH_RX_DEBUG_SBNPREFETCH_EVENBNENTRY_FIELD_WIDTH },
    { BBH_RX_DEBUG_SBNPREFETCH_EVENBNENTRY_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EVENVALID *****/
const ru_field_rec BBH_RX_DEBUG_SBNPREFETCH_EVENVALID_FIELD =
{
    "EVENVALID",
#if RU_INCLUDE_DESC
    "",
    "SBN is Valid\n",
#endif
    { BBH_RX_DEBUG_SBNPREFETCH_EVENVALID_FIELD_MASK },
    0,
    { BBH_RX_DEBUG_SBNPREFETCH_EVENVALID_FIELD_WIDTH },
    { BBH_RX_DEBUG_SBNPREFETCH_EVENVALID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_DEBUG_SBNPREFETCH_FIELDS[] =
{
    &BBH_RX_DEBUG_SBNPREFETCH_ODDBNENTRY_FIELD,
    &BBH_RX_DEBUG_SBNPREFETCH_ODDVALID_FIELD,
    &BBH_RX_DEBUG_SBNPREFETCH_EVENBNENTRY_FIELD,
    &BBH_RX_DEBUG_SBNPREFETCH_EVENVALID_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_DEBUG_SBNPREFETCH *****/
const ru_reg_rec BBH_RX_DEBUG_SBNPREFETCH_REG =
{
    "DEBUG_SBNPREFETCH",
#if RU_INCLUDE_DESC
    "SRAM_BN_PREFETCH Register",
    "The BBH RX hold a FIFO with 64 BN.\n",
#endif
    { BBH_RX_DEBUG_SBNPREFETCH_REG_OFFSET },
    0,
    0,
    282,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    BBH_RX_DEBUG_SBNPREFETCH_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_DEBUG_EXCFIFO, TYPE: Type_BBH_RX_BBHRX_DEBUG_EXCFIFO
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: BNENTRY *****/
const ru_field_rec BBH_RX_DEBUG_EXCFIFO_BNENTRY_FIELD =
{
    "BNENTRY",
#if RU_INCLUDE_DESC
    "",
    "BN\n",
#endif
    { BBH_RX_DEBUG_EXCFIFO_BNENTRY_FIELD_MASK },
    0,
    { BBH_RX_DEBUG_EXCFIFO_BNENTRY_FIELD_WIDTH },
    { BBH_RX_DEBUG_EXCFIFO_BNENTRY_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: VALID *****/
const ru_field_rec BBH_RX_DEBUG_EXCFIFO_VALID_FIELD =
{
    "VALID",
#if RU_INCLUDE_DESC
    "",
    "SBN is Valid\n",
#endif
    { BBH_RX_DEBUG_EXCFIFO_VALID_FIELD_MASK },
    0,
    { BBH_RX_DEBUG_EXCFIFO_VALID_FIELD_WIDTH },
    { BBH_RX_DEBUG_EXCFIFO_VALID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_DEBUG_EXCFIFO_FIELDS[] =
{
    &BBH_RX_DEBUG_EXCFIFO_BNENTRY_FIELD,
    &BBH_RX_DEBUG_EXCFIFO_VALID_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_DEBUG_EXCFIFO *****/
const ru_reg_rec BBH_RX_DEBUG_EXCFIFO_REG =
{
    "DEBUG_EXCFIFO",
#if RU_INCLUDE_DESC
    "EXC_RESULT_FIFO 0..127 Register",
    "The BBH RX holds a 32 entries FIFO per port.\nFor n in [0..7]\nPort n: index[n*32..(n+1)*32-1]\n",
#endif
    { BBH_RX_DEBUG_EXCFIFO_REG_OFFSET },
    BBH_RX_DEBUG_EXCFIFO_REG_RAM_CNT,
    4,
    283,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    BBH_RX_DEBUG_EXCFIFO_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_BBH_RX_PON_CNPL_DEBUG_GEM_OUT_OF_RANGE, TYPE: Type_BBH_RX_PON_CNPL_DEBUG_GEM_OUT_OF_RANGE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PMVALUE *****/
const ru_field_rec BBH_RX_BBH_RX_PON_CNPL_DEBUG_GEM_OUT_OF_RANGE_PMVALUE_FIELD =
{
    "PMVALUE",
#if RU_INCLUDE_DESC
    "",
    "PM counter value.\n",
#endif
    { BBH_RX_BBH_RX_PON_CNPL_DEBUG_GEM_OUT_OF_RANGE_PMVALUE_FIELD_MASK },
    0,
    { BBH_RX_BBH_RX_PON_CNPL_DEBUG_GEM_OUT_OF_RANGE_PMVALUE_FIELD_WIDTH },
    { BBH_RX_BBH_RX_PON_CNPL_DEBUG_GEM_OUT_OF_RANGE_PMVALUE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_BBH_RX_PON_CNPL_DEBUG_GEM_OUT_OF_RANGE_FIELDS[] =
{
    &BBH_RX_BBH_RX_PON_CNPL_DEBUG_GEM_OUT_OF_RANGE_PMVALUE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_BBH_RX_PON_CNPL_DEBUG_GEM_OUT_OF_RANGE *****/
const ru_reg_rec BBH_RX_BBH_RX_PON_CNPL_DEBUG_GEM_OUT_OF_RANGE_REG =
{
    "BBH_RX_PON_CNPL_DEBUG_GEM_OUT_OF_RANGE",
#if RU_INCLUDE_DESC
    "PON_GEM_OUT_OF_RANGE Register",
    "This counter counts the number PON GEMs out of range - in case GEM ID > 255.\nThis counter is cleared when read and freezes when reaches the maximum value.\n",
#endif
    { BBH_RX_BBH_RX_PON_CNPL_DEBUG_GEM_OUT_OF_RANGE_REG_OFFSET },
    0,
    0,
    284,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_BBH_RX_PON_CNPL_DEBUG_GEM_OUT_OF_RANGE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_BBH_RX_PON_CNPL_DEBUG_GEM_OUT_OF_RANGE_FLOW, TYPE: Type_BBH_RX_PON_CNPL_DEBUG_GEM_OUT_OF_RANGE_FLOW
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: FLOW *****/
const ru_field_rec BBH_RX_BBH_RX_PON_CNPL_DEBUG_GEM_OUT_OF_RANGE_FLOW_FLOW_FIELD =
{
    "FLOW",
#if RU_INCLUDE_DESC
    "",
    "Out of range flow.\n",
#endif
    { BBH_RX_BBH_RX_PON_CNPL_DEBUG_GEM_OUT_OF_RANGE_FLOW_FLOW_FIELD_MASK },
    0,
    { BBH_RX_BBH_RX_PON_CNPL_DEBUG_GEM_OUT_OF_RANGE_FLOW_FLOW_FIELD_WIDTH },
    { BBH_RX_BBH_RX_PON_CNPL_DEBUG_GEM_OUT_OF_RANGE_FLOW_FLOW_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_BBH_RX_PON_CNPL_DEBUG_GEM_OUT_OF_RANGE_FLOW_FIELDS[] =
{
    &BBH_RX_BBH_RX_PON_CNPL_DEBUG_GEM_OUT_OF_RANGE_FLOW_FLOW_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_BBH_RX_PON_CNPL_DEBUG_GEM_OUT_OF_RANGE_FLOW *****/
const ru_reg_rec BBH_RX_BBH_RX_PON_CNPL_DEBUG_GEM_OUT_OF_RANGE_FLOW_REG =
{
    "BBH_RX_PON_CNPL_DEBUG_GEM_OUT_OF_RANGE_FLOW",
#if RU_INCLUDE_DESC
    "PON_GEM_OUT_OF_RANGE_FLOW Register",
    "This register holds the last GEM flow in the case of PON GEMs out of range - in case GEM ID > 255.\n",
#endif
    { BBH_RX_BBH_RX_PON_CNPL_DEBUG_GEM_OUT_OF_RANGE_FLOW_REG_OFFSET },
    0,
    0,
    285,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_BBH_RX_PON_CNPL_DEBUG_GEM_OUT_OF_RANGE_FLOW_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_BBH_RX_PON_CNPL_DEBUG_USEDWORDS, TYPE: Type_BBH_RX_PON_CNPL_DEBUG_USEDWORDS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: USEDWORDS *****/
const ru_field_rec BBH_RX_BBH_RX_PON_CNPL_DEBUG_USEDWORDS_USEDWORDS_FIELD =
{
    "USEDWORDS",
#if RU_INCLUDE_DESC
    "",
    "Used words\n",
#endif
    { BBH_RX_BBH_RX_PON_CNPL_DEBUG_USEDWORDS_USEDWORDS_FIELD_MASK },
    0,
    { BBH_RX_BBH_RX_PON_CNPL_DEBUG_USEDWORDS_USEDWORDS_FIELD_WIDTH },
    { BBH_RX_BBH_RX_PON_CNPL_DEBUG_USEDWORDS_USEDWORDS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_BBH_RX_PON_CNPL_DEBUG_USEDWORDS_FIELDS[] =
{
    &BBH_RX_BBH_RX_PON_CNPL_DEBUG_USEDWORDS_USEDWORDS_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_BBH_RX_PON_CNPL_DEBUG_USEDWORDS *****/
const ru_reg_rec BBH_RX_BBH_RX_PON_CNPL_DEBUG_USEDWORDS_REG =
{
    "BBH_RX_PON_CNPL_DEBUG_USEDWORDS",
#if RU_INCLUDE_DESC
    "INGRESS_FIFO_USED_WORDS Register",
    "CNPL ingress FIFO used words\n",
#endif
    { BBH_RX_BBH_RX_PON_CNPL_DEBUG_USEDWORDS_REG_OFFSET },
    0,
    0,
    286,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_BBH_RX_PON_CNPL_DEBUG_USEDWORDS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: BBH_RX_GEM_MODE_GEMMODE, TYPE: Type_BBH_RX_BBHRX_GEM_MODE_GEMMODE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: GEMMODE *****/
const ru_field_rec BBH_RX_GEM_MODE_GEMMODE_GEMMODE_FIELD =
{
    "GEMMODE",
#if RU_INCLUDE_DESC
    "",
    "GEM mode entry\n",
#endif
    { BBH_RX_GEM_MODE_GEMMODE_GEMMODE_FIELD_MASK },
    0,
    { BBH_RX_GEM_MODE_GEMMODE_GEMMODE_FIELD_WIDTH },
    { BBH_RX_GEM_MODE_GEMMODE_GEMMODE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *BBH_RX_GEM_MODE_GEMMODE_FIELDS[] =
{
    &BBH_RX_GEM_MODE_GEMMODE_GEMMODE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: BBH_RX_GEM_MODE_GEMMODE *****/
const ru_reg_rec BBH_RX_GEM_MODE_GEMMODE_REG =
{
    "GEM_MODE_GEMMODE",
#if RU_INCLUDE_DESC
    "GEM_MODE_ENTRY 0..2047 Register",
    "Total of 8KBytes - 1024 x 64\nport 0 refers to the 1st KByte (8Kbit).\nport 1 refers to the 2nd KByte (8Kbit).\n...\nport 7 refers to the 8th KByte (8Kbit).\n\nThe order of GEM flows (per port) within each entry of the table (i is the entry index - starting from 0) is as follows:\n{gem_flow[i*64+63], gem_flow[i*64+62], ....., gem_flow[i*64+1], gem_flow[i*64+0]}\n\nSince the memory width is 64 bits, Write is done using 2 accesses:\nwrite into an 8-bytes aligned address will be sampled by the HW.\nWrite to a non 8-bytes aligned address will write into the memory {sampled wr data, current wr_data}.\n\nRead is also done using 2 accesses:\nRead from an 8-bytes aligned address will sampled 64 bits of the memory  and reply the 32 MSBits.\nRead to a non 8-bytes aligned address will read the 32 LSBits.\n\n64 bits GEM entry is a concatenation of { data[address=8X] , data[ address=8X+4] }\n- address 8x   will contain { gem_flow[63], gem_flow[62], ..., gem_flow[33], gem_flow[32] }\n- address 8X+4 will contain { gem_flow[31], gem_flow[30], ..., gem_flow[1], gem_flow[0] }\n\n\n",
#endif
    { BBH_RX_GEM_MODE_GEMMODE_REG_OFFSET },
    BBH_RX_GEM_MODE_GEMMODE_REG_RAM_CNT,
    4,
    287,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    BBH_RX_GEM_MODE_GEMMODE_FIELDS,
#endif
};

unsigned long BBH_RX_ADDRS[] =
{
    0xA2050000,
    0xA2060000,
    0xA2070000,
};

static const ru_reg_rec *BBH_RX_REGS[] =
{
    &BBH_RX_GENERAL_CONFIGURATION_BBCFG_REG,
    &BBH_RX_GENERAL_CONFIGURATION_ENABLE_REG,
    &BBH_RX_GENERAL_CONFIGURATION_MINPKT_REG,
    &BBH_RX_GENERAL_CONFIGURATION_MAXPKT_REG,
    &BBH_RX_GENERAL_CONFIGURATION_CLK_GATE_CNTRL_REG,
    &BBH_RX_GENERAL_CONFIGURATION_RXDBGSEL_REG,
    &BBH_RX_GENERAL_CONFIGURATION_RXRSTRST_REG,
    &BBH_RX_GENERAL_CONFIGURATION_SBPMCFG_REG,
    &BBH_RX_GENERAL_CONFIGURATION_PSRAMBASE_REG,
    &BBH_RX_GENERAL_CONFIGURATION_INGRESSCHANNEL_REG,
    &BBH_RX_GENERAL_CONFIGURATION_BBOVRDROUTE_REG,
    &BBH_RX_GENERAL_CONFIGURATION_BBOVRDCTRL_REG,
    &BBH_RX_GENERAL_CONFIGURATION_GEMDONE_REG,
    &BBH_RX_GENERAL_CONFIGURATION_MACTYPE_REG,
    &BBH_RX_GENERAL_CONFIGURATION_PORTTH_REG,
    &BBH_RX_GENERAL_CONFIGURATION_FLOWCTRL_REG,
    &BBH_RX_GENERAL_CONFIGURATION_SOPOFFSET_REG,
    &BBH_RX_GENERAL_CONFIGURATION_INGRESSPID_REG,
    &BBH_RX_GENERAL_CONFIGURATION_PORTWEIGHT_REG,
    &BBH_RX_GENERAL_CONFIGURATION_PORTVIQMASK_REG,
    &BBH_RX_GENERAL_CONFIGURATION_EXCVIQMASK_REG,
    &BBH_RX_GENERAL_CONFIGURATION_DISPVIQ_REG,
    &BBH_RX_GENERAL_CONFIGURATION_VIQWEIGHT_REG,
    &BBH_RX_GENERAL_CONFIGURATION_PATTERNCFG_REG,
    &BBH_RX_GENERAL_CONFIGURATION_PATTERNDATA_REG,
    &BBH_RX_GENERAL_CONFIGURATION_PATTERNMASK_REG,
    &BBH_RX_GENERAL_CONFIGURATION_PATTERNDATA8B_REG,
    &BBH_RX_GENERAL_CONFIGURATION_PATTERNMASK8B_REG,
    &BBH_RX_GENERAL_CONFIGURATION_PATTERNEN_REG,
    &BBH_RX_GENERAL_CONFIGURATION_VIQFCEN_REG,
    &BBH_RX_GENERAL_CONFIGURATION_NONETH_REG,
    &BBH_RX_PM_COUNTERS_INPKT_REG,
    &BBH_RX_PM_COUNTERS_INBYTE_REG,
    &BBH_RX_PM_COUNTERS_TOOSHORT_REG,
    &BBH_RX_PM_COUNTERS_TOOLONG_REG,
    &BBH_RX_PM_COUNTERS_CRCERROR_REG,
    &BBH_RX_PM_COUNTERS_FCDROP_REG,
    &BBH_RX_PM_COUNTERS_TRUNCATE_REG,
    &BBH_RX_PM_COUNTERS_TOTALDROP_REG,
    &BBH_RX_PM_COUNTERS_DUMMYPDCNT_REG,
    &BBH_RX_PON_RMON_COUNTERS_R64_REG,
    &BBH_RX_PON_RMON_COUNTERS_R127_REG,
    &BBH_RX_PON_RMON_COUNTERS_R255_REG,
    &BBH_RX_PON_RMON_COUNTERS_R511_REG,
    &BBH_RX_PON_RMON_COUNTERS_R1023_REG,
    &BBH_RX_PON_RMON_COUNTERS_R1518_REG,
    &BBH_RX_PON_RMON_COUNTERS_R2047_REG,
    &BBH_RX_PON_RMON_COUNTERS_R4095_REG,
    &BBH_RX_PON_RMON_COUNTERS_R9216_REG,
    &BBH_RX_PON_RMON_COUNTERS_R16383_REG,
    &BBH_RX_DEBUG_PFBUW_REG,
    &BBH_RX_DEBUG_PATTERNUW_REG,
    &BBH_RX_DEBUG_EXCUW_REG,
    &BBH_RX_DEBUG_PORTUW_REG,
    &BBH_RX_DEBUG_FIRSTSBN_REG,
    &BBH_RX_DEBUG_CURRENTSBN_REG,
    &BBH_RX_DEBUG_SN_REG,
    &BBH_RX_DEBUG_VIQMASK_REG,
    &BBH_RX_DEBUG_PLEN_REG,
    &BBH_RX_DEBUG_EOPOK_REG,
    &BBH_RX_DEBUG_VIQFULL_REG,
    &BBH_RX_DEBUG_PORTREQ_REG,
    &BBH_RX_DEBUG_DUMMYREQ_REG,
    &BBH_RX_DEBUG_INREASSEMBLY_REG,
    &BBH_RX_DEBUG_DUMMYPD_REG,
    &BBH_RX_DEBUG_UFUW_REG,
    &BBH_RX_DEBUG_PENDCONNECTUW_REG,
    &BBH_RX_DEBUG_DBGVEC_REG,
    &BBH_RX_DEBUG_PDUW_REG,
    &BBH_RX_DEBUG_SBPMACK_REG,
    &BBH_RX_DEBUG_WRITEREPLYACK_REG,
    &BBH_RX_DEBUG_DISPCREDITACK_REG,
    &BBH_RX_DEBUG_PDRDDATA_REG,
    &BBH_RX_DEBUG_SBNFIFO_REG,
    &BBH_RX_DEBUG_SBNPREFETCH_REG,
    &BBH_RX_DEBUG_EXCFIFO_REG,
    &BBH_RX_BBH_RX_PON_CNPL_DEBUG_GEM_OUT_OF_RANGE_REG,
    &BBH_RX_BBH_RX_PON_CNPL_DEBUG_GEM_OUT_OF_RANGE_FLOW_REG,
    &BBH_RX_BBH_RX_PON_CNPL_DEBUG_USEDWORDS_REG,
    &BBH_RX_GEM_MODE_GEMMODE_REG,
};

const ru_block_rec BBH_RX_BLOCK =
{
    "BBH_RX",
#if RU_INCLUDE_SIZE
    { 0x00004000 },
#endif
    BBH_RX_ADDRS,
    3,
    80,
    BBH_RX_REGS,
};
