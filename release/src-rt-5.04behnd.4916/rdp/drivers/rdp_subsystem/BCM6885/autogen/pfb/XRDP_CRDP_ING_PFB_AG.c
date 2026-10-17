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


#include "XRDP_CRDP_ING_PFB_AG.h"

/******************************************************************************
 * Register: NAME: CRDP_ING_PFB_CFG_ING, TYPE: Type_PFB_CFG_ING
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: FIFO_EN *****/
const ru_field_rec CRDP_ING_PFB_CFG_ING_FIFO_EN_FIELD =
{
    "FIFO_EN",
#if RU_INCLUDE_DESC
    "",
    "channel enabled is used as qualifier for the configuration. should be set to 0 before writing any configurations.\nset to 1 enables the channel.\nORed with INIT_ADDR_EN. see INIT_ADDR_EN for further description\n\n",
#endif
    { CRDP_ING_PFB_CFG_ING_FIFO_EN_FIELD_MASK },
    0,
    { CRDP_ING_PFB_CFG_ING_FIFO_EN_FIELD_WIDTH },
    { CRDP_ING_PFB_CFG_ING_FIFO_EN_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TO_VAL *****/
const ru_field_rec CRDP_ING_PFB_CFG_ING_TO_VAL_FIELD =
{
    "TO_VAL",
#if RU_INCLUDE_DESC
    "",
    "timeout value (power of 2)\nNote: value of 0x0 is used to disable timer feature\n\n",
#endif
    { CRDP_ING_PFB_CFG_ING_TO_VAL_FIELD_MASK },
    0,
    { CRDP_ING_PFB_CFG_ING_TO_VAL_FIELD_WIDTH },
    { CRDP_ING_PFB_CFG_ING_TO_VAL_FIELD_SHIFT },
    2,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MY_CH_ADDR *****/
const ru_field_rec CRDP_ING_PFB_CFG_ING_MY_CH_ADDR_FIELD =
{
    "MY_CH_ADDR",
#if RU_INCLUDE_DESC
    "",
    "INGRESS FIFO ID\n",
#endif
    { CRDP_ING_PFB_CFG_ING_MY_CH_ADDR_FIELD_MASK },
    0,
    { CRDP_ING_PFB_CFG_ING_MY_CH_ADDR_FIELD_WIDTH },
    { CRDP_ING_PFB_CFG_ING_MY_CH_ADDR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EGR_CH_ADDR *****/
const ru_field_rec CRDP_ING_PFB_CFG_ING_EGR_CH_ADDR_FIELD =
{
    "EGR_CH_ADDR",
#if RU_INCLUDE_DESC
    "",
    "destination egress BB target address\n",
#endif
    { CRDP_ING_PFB_CFG_ING_EGR_CH_ADDR_FIELD_MASK },
    0,
    { CRDP_ING_PFB_CFG_ING_EGR_CH_ADDR_FIELD_WIDTH },
    { CRDP_ING_PFB_CFG_ING_EGR_CH_ADDR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EGR_BB_ID *****/
const ru_field_rec CRDP_ING_PFB_CFG_ING_EGR_BB_ID_FIELD =
{
    "EGR_BB_ID",
#if RU_INCLUDE_DESC
    "",
    "destination PFB BB ID\n",
#endif
    { CRDP_ING_PFB_CFG_ING_EGR_BB_ID_FIELD_MASK },
    0,
    { CRDP_ING_PFB_CFG_ING_EGR_BB_ID_FIELD_WIDTH },
    { CRDP_ING_PFB_CFG_ING_EGR_BB_ID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: WRR_VAL *****/
const ru_field_rec CRDP_ING_PFB_CFG_ING_WRR_VAL_FIELD =
{
    "WRR_VAL",
#if RU_INCLUDE_DESC
    "",
    "if arb_scheme == 0x0 then the arbitration is done according the value here.\n",
#endif
    { CRDP_ING_PFB_CFG_ING_WRR_VAL_FIELD_MASK },
    0,
    { CRDP_ING_PFB_CFG_ING_WRR_VAL_FIELD_WIDTH },
    { CRDP_ING_PFB_CFG_ING_WRR_VAL_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: INIT_ADDR_EN *****/
const ru_field_rec CRDP_ING_PFB_CFG_ING_INIT_ADDR_EN_FIELD =
{
    "INIT_ADDR_EN",
#if RU_INCLUDE_DESC
    "",
    "Enable initialization according to incoming credit messages from egress side.\nif set to 0, then channel parameters are taken from MY_TARGET_ADDR, DEST_TARGET_ADDR, TRGT_BB_ID and WIDTH_IF (new in Lotus PFB)\nif set to 1, then the values of the above are dismissed. Configured by egress\nFIFO_EN is ORed with INIT_ADDR_EN. if set to 1 then FIFO_EN is set too.\nif any configuration of ingress FIFO should be modified then set INIT_ADDR_EN and FIFO_EN to 0 before config.\nWhen config is done set on consecutive write FIFO_EN =1 and INIT_ADDR_EN = 0/1\n\nif ING_INIT_ADDR_EN is set 1,  two following fields are hardwiried:\n1) cfg_egr_my_target_addr is set to channel index\n2) cfg_ing_my_target_addr is set to channel index\n\nand three other following fields are updated via CREDIT message from egress to ingress:\n3) cfg_ing_trgt_bb_id = target BB_ID programmmed by EGR\n4) cr_pfb_ing_cr_if_width = client width  programmed by EGR\n5)  cfg_ing_dest_target_addr = egress FIFO ID programmed by EGR\n",
#endif
    { CRDP_ING_PFB_CFG_ING_INIT_ADDR_EN_FIELD_MASK },
    0,
    { CRDP_ING_PFB_CFG_ING_INIT_ADDR_EN_FIELD_WIDTH },
    { CRDP_ING_PFB_CFG_ING_INIT_ADDR_EN_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: IF_WIDTH *****/
const ru_field_rec CRDP_ING_PFB_CFG_ING_IF_WIDTH_FIELD =
{
    "IF_WIDTH",
#if RU_INCLUDE_DESC
    "",
    "Enable different interface widths on ingress client side. Options:\n0x0: 32 bit\n0x1: 128 bit\n0x2: 256 bit\n\n",
#endif
    { CRDP_ING_PFB_CFG_ING_IF_WIDTH_FIELD_MASK },
    0,
    { CRDP_ING_PFB_CFG_ING_IF_WIDTH_FIELD_WIDTH },
    { CRDP_ING_PFB_CFG_ING_IF_WIDTH_FIELD_SHIFT },
    2,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EOP_EN *****/
const ru_field_rec CRDP_ING_PFB_CFG_ING_EOP_EN_FIELD =
{
    "EOP_EN",
#if RU_INCLUDE_DESC
    "",
    "Enable EOP detection in ingress channel\n\n",
#endif
    { CRDP_ING_PFB_CFG_ING_EOP_EN_FIELD_MASK },
    0,
    { CRDP_ING_PFB_CFG_ING_EOP_EN_FIELD_WIDTH },
    { CRDP_ING_PFB_CFG_ING_EOP_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ISO_INPUT *****/
const ru_field_rec CRDP_ING_PFB_CFG_ING_ISO_INPUT_FIELD =
{
    "ISO_INPUT",
#if RU_INCLUDE_DESC
    "",
    "Isolate (=ignore) client input (in case of ingress channel: ignore push input signal)\n",
#endif
    { CRDP_ING_PFB_CFG_ING_ISO_INPUT_FIELD_MASK },
    0,
    { CRDP_ING_PFB_CFG_ING_ISO_INPUT_FIELD_WIDTH },
    { CRDP_ING_PFB_CFG_ING_ISO_INPUT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_ING_PFB_CFG_ING_FIELDS[] =
{
    &CRDP_ING_PFB_CFG_ING_FIFO_EN_FIELD,
    &CRDP_ING_PFB_CFG_ING_TO_VAL_FIELD,
    &CRDP_ING_PFB_CFG_ING_MY_CH_ADDR_FIELD,
    &CRDP_ING_PFB_CFG_ING_EGR_CH_ADDR_FIELD,
    &CRDP_ING_PFB_CFG_ING_EGR_BB_ID_FIELD,
    &CRDP_ING_PFB_CFG_ING_WRR_VAL_FIELD,
    &CRDP_ING_PFB_CFG_ING_INIT_ADDR_EN_FIELD,
    &CRDP_ING_PFB_CFG_ING_IF_WIDTH_FIELD,
    &CRDP_ING_PFB_CFG_ING_EOP_EN_FIELD,
    &CRDP_ING_PFB_CFG_ING_ISO_INPUT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_ING_PFB_CFG_ING *****/
const ru_reg_rec CRDP_ING_PFB_CFG_ING_REG =
{
    "CFG_ING",
#if RU_INCLUDE_DESC
    "INGRESS 0..7 Register",
    "ingress FIFO cfg/ctrl\n",
#endif
    { CRDP_ING_PFB_CFG_ING_REG_OFFSET },
    CRDP_ING_PFB_CFG_ING_REG_RAM_CNT,
    4,
    967,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    10,
    CRDP_ING_PFB_CFG_ING_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_ING_PFB_CFG_EGR, TYPE: Type_PFB_CFG_EGR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: FIFO_EN *****/
const ru_field_rec CRDP_ING_PFB_CFG_EGR_FIFO_EN_FIELD =
{
    "FIFO_EN",
#if RU_INCLUDE_DESC
    "",
    "fifo enable\n",
#endif
    { CRDP_ING_PFB_CFG_EGR_FIFO_EN_FIELD_MASK },
    0,
    { CRDP_ING_PFB_CFG_EGR_FIFO_EN_FIELD_WIDTH },
    { CRDP_ING_PFB_CFG_EGR_FIFO_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CR_REP_GRAN *****/
const ru_field_rec CRDP_ING_PFB_CFG_EGR_CR_REP_GRAN_FIELD =
{
    "CR_REP_GRAN",
#if RU_INCLUDE_DESC
    "",
    "credits reply granularity 8B granulariy (power of 2)\n",
#endif
    { CRDP_ING_PFB_CFG_EGR_CR_REP_GRAN_FIELD_MASK },
    0,
    { CRDP_ING_PFB_CFG_EGR_CR_REP_GRAN_FIELD_WIDTH },
    { CRDP_ING_PFB_CFG_EGR_CR_REP_GRAN_FIELD_SHIFT },
    2,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MY_CH_ADDR *****/
const ru_field_rec CRDP_ING_PFB_CFG_EGR_MY_CH_ADDR_FIELD =
{
    "MY_CH_ADDR",
#if RU_INCLUDE_DESC
    "",
    "egress BB target address\n",
#endif
    { CRDP_ING_PFB_CFG_EGR_MY_CH_ADDR_FIELD_MASK },
    0,
    { CRDP_ING_PFB_CFG_EGR_MY_CH_ADDR_FIELD_WIDTH },
    { CRDP_ING_PFB_CFG_EGR_MY_CH_ADDR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ING_CH_ADDR *****/
const ru_field_rec CRDP_ING_PFB_CFG_EGR_ING_CH_ADDR_FIELD =
{
    "ING_CH_ADDR",
#if RU_INCLUDE_DESC
    "",
    "source ingress BB target address\n",
#endif
    { CRDP_ING_PFB_CFG_EGR_ING_CH_ADDR_FIELD_MASK },
    0,
    { CRDP_ING_PFB_CFG_EGR_ING_CH_ADDR_FIELD_WIDTH },
    { CRDP_ING_PFB_CFG_EGR_ING_CH_ADDR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: INIT_CR *****/
const ru_field_rec CRDP_ING_PFB_CFG_EGR_INIT_CR_FIELD =
{
    "INIT_CR",
#if RU_INCLUDE_DESC
    "",
    "init credits override 8B granulariy (power of 2). if 0 (default), then the initial credits are derived from FIFO depth:\ninit_credits_to_ingress = FIFO_size_in_bytes/8B\n",
#endif
    { CRDP_ING_PFB_CFG_EGR_INIT_CR_FIELD_MASK },
    0,
    { CRDP_ING_PFB_CFG_EGR_INIT_CR_FIELD_WIDTH },
    { CRDP_ING_PFB_CFG_EGR_INIT_CR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ING_BB_ID *****/
const ru_field_rec CRDP_ING_PFB_CFG_EGR_ING_BB_ID_FIELD =
{
    "ING_BB_ID",
#if RU_INCLUDE_DESC
    "",
    "source PFB BB ID\n",
#endif
    { CRDP_ING_PFB_CFG_EGR_ING_BB_ID_FIELD_MASK },
    0,
    { CRDP_ING_PFB_CFG_EGR_ING_BB_ID_FIELD_WIDTH },
    { CRDP_ING_PFB_CFG_EGR_ING_BB_ID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: INIT_ADDR_EN *****/
const ru_field_rec CRDP_ING_PFB_CFG_EGR_INIT_ADDR_EN_FIELD =
{
    "INIT_ADDR_EN",
#if RU_INCLUDE_DESC
    "",
    "if set to 1, then my_target_addr is hardwired to egress fifo sequential value: [0..7]\n\nif set to 0, then my_target_addr is set according to my_target_addr reg value\n",
#endif
    { CRDP_ING_PFB_CFG_EGR_INIT_ADDR_EN_FIELD_MASK },
    0,
    { CRDP_ING_PFB_CFG_EGR_INIT_ADDR_EN_FIELD_WIDTH },
    { CRDP_ING_PFB_CFG_EGR_INIT_ADDR_EN_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: IF_WIDTH *****/
const ru_field_rec CRDP_ING_PFB_CFG_EGR_IF_WIDTH_FIELD =
{
    "IF_WIDTH",
#if RU_INCLUDE_DESC
    "",
    "Enable different interface widths on egress client side. Options:\n0x0: 32 bit\n0x1: 128 bit\n0x2: 256 bit\n",
#endif
    { CRDP_ING_PFB_CFG_EGR_IF_WIDTH_FIELD_MASK },
    0,
    { CRDP_ING_PFB_CFG_EGR_IF_WIDTH_FIELD_WIDTH },
    { CRDP_ING_PFB_CFG_EGR_IF_WIDTH_FIELD_SHIFT },
    2,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FLUSH_EN *****/
const ru_field_rec CRDP_ING_PFB_CFG_EGR_FLUSH_EN_FIELD =
{
    "FLUSH_EN",
#if RU_INCLUDE_DESC
    "",
    "Flush egress FIFO (the assertion of this bit will cause Egress PFB channel to empty its FIFO, while the emptied data will be not pushed to egress client side)\n",
#endif
    { CRDP_ING_PFB_CFG_EGR_FLUSH_EN_FIELD_MASK },
    0,
    { CRDP_ING_PFB_CFG_EGR_FLUSH_EN_FIELD_WIDTH },
    { CRDP_ING_PFB_CFG_EGR_FLUSH_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ISO_INPUT *****/
const ru_field_rec CRDP_ING_PFB_CFG_EGR_ISO_INPUT_FIELD =
{
    "ISO_INPUT",
#if RU_INCLUDE_DESC
    "",
    "Isolate (=ignore) client input (in case of egress channel: ignore pop input signal)\n",
#endif
    { CRDP_ING_PFB_CFG_EGR_ISO_INPUT_FIELD_MASK },
    0,
    { CRDP_ING_PFB_CFG_EGR_ISO_INPUT_FIELD_WIDTH },
    { CRDP_ING_PFB_CFG_EGR_ISO_INPUT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_ING_PFB_CFG_EGR_FIELDS[] =
{
    &CRDP_ING_PFB_CFG_EGR_FIFO_EN_FIELD,
    &CRDP_ING_PFB_CFG_EGR_CR_REP_GRAN_FIELD,
    &CRDP_ING_PFB_CFG_EGR_MY_CH_ADDR_FIELD,
    &CRDP_ING_PFB_CFG_EGR_ING_CH_ADDR_FIELD,
    &CRDP_ING_PFB_CFG_EGR_INIT_CR_FIELD,
    &CRDP_ING_PFB_CFG_EGR_ING_BB_ID_FIELD,
    &CRDP_ING_PFB_CFG_EGR_INIT_ADDR_EN_FIELD,
    &CRDP_ING_PFB_CFG_EGR_IF_WIDTH_FIELD,
    &CRDP_ING_PFB_CFG_EGR_FLUSH_EN_FIELD,
    &CRDP_ING_PFB_CFG_EGR_ISO_INPUT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_ING_PFB_CFG_EGR *****/
const ru_reg_rec CRDP_ING_PFB_CFG_EGR_REG =
{
    "CFG_EGR",
#if RU_INCLUDE_DESC
    "EGRESS 0..7 Register",
    "egress FIFO cfg/ctrl\n",
#endif
    { CRDP_ING_PFB_CFG_EGR_REG_OFFSET },
    CRDP_ING_PFB_CFG_EGR_REG_RAM_CNT,
    4,
    968,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    10,
    CRDP_ING_PFB_CFG_EGR_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_ING_PFB_CFG_GBL, TYPE: Type_PFB_CFG_GBL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ARB_SCHEME *****/
const ru_field_rec CRDP_ING_PFB_CFG_GBL_ARB_SCHEME_FIELD =
{
    "ARB_SCHEME",
#if RU_INCLUDE_DESC
    "",
    "Configure the arbitration scheme:\n0. (Weighted) Round robin (default)\n1. Strict priority. FIFO with the lower interface number gets the highest priority\n\n",
#endif
    { CRDP_ING_PFB_CFG_GBL_ARB_SCHEME_FIELD_MASK },
    0,
    { CRDP_ING_PFB_CFG_GBL_ARB_SCHEME_FIELD_WIDTH },
    { CRDP_ING_PFB_CFG_GBL_ARB_SCHEME_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ROUTE_ADDR_OVERWR_EN *****/
const ru_field_rec CRDP_ING_PFB_CFG_GBL_ROUTE_ADDR_OVERWR_EN_FIELD =
{
    "ROUTE_ADDR_OVERWR_EN",
#if RU_INCLUDE_DESC
    "",
    "Enable of override of PFB BB256 route address for any outcoming transaction (data/credit/pwire) from PFB unit on broadbus. In case , the route addr will applied constantly from ROUTE_ADDR_OVERWR_VAL\n",
#endif
    { CRDP_ING_PFB_CFG_GBL_ROUTE_ADDR_OVERWR_EN_FIELD_MASK },
    0,
    { CRDP_ING_PFB_CFG_GBL_ROUTE_ADDR_OVERWR_EN_FIELD_WIDTH },
    { CRDP_ING_PFB_CFG_GBL_ROUTE_ADDR_OVERWR_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SRC_BB_ID *****/
const ru_field_rec CRDP_ING_PFB_CFG_GBL_SRC_BB_ID_FIELD =
{
    "SRC_BB_ID",
#if RU_INCLUDE_DESC
    "",
    "Configure source BB_ID of pfb_top, which can be used alternately (upon overiide_en) to wired input of BB_ID\n\n",
#endif
    { CRDP_ING_PFB_CFG_GBL_SRC_BB_ID_FIELD_MASK },
    0,
    { CRDP_ING_PFB_CFG_GBL_SRC_BB_ID_FIELD_WIDTH },
    { CRDP_ING_PFB_CFG_GBL_SRC_BB_ID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SRC_BB_ID_OVR_EN *****/
const ru_field_rec CRDP_ING_PFB_CFG_GBL_SRC_BB_ID_OVR_EN_FIELD =
{
    "SRC_BB_ID_OVR_EN",
#if RU_INCLUDE_DESC
    "",
    "Enable of override of source BB_ID of pfb_top, which can be used alternately to wired input of BB_ID\n\n",
#endif
    { CRDP_ING_PFB_CFG_GBL_SRC_BB_ID_OVR_EN_FIELD_MASK },
    0,
    { CRDP_ING_PFB_CFG_GBL_SRC_BB_ID_OVR_EN_FIELD_WIDTH },
    { CRDP_ING_PFB_CFG_GBL_SRC_BB_ID_OVR_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FLUSH_PACE_TIMEOUT *****/
const ru_field_rec CRDP_ING_PFB_CFG_GBL_FLUSH_PACE_TIMEOUT_FIELD =
{
    "FLUSH_PACE_TIMEOUT",
#if RU_INCLUDE_DESC
    "",
    "timeout value in power of 2 (in clock cycles) that used for pace pop during egress flush\n\nNote: value of 0x0 is used to disable timer feature\n\n",
#endif
    { CRDP_ING_PFB_CFG_GBL_FLUSH_PACE_TIMEOUT_FIELD_MASK },
    0,
    { CRDP_ING_PFB_CFG_GBL_FLUSH_PACE_TIMEOUT_FIELD_WIDTH },
    { CRDP_ING_PFB_CFG_GBL_FLUSH_PACE_TIMEOUT_FIELD_SHIFT },
    2,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ROUTE_ADDR_OVERWR_VAL *****/
const ru_field_rec CRDP_ING_PFB_CFG_GBL_ROUTE_ADDR_OVERWR_VAL_FIELD =
{
    "ROUTE_ADDR_OVERWR_VAL",
#if RU_INCLUDE_DESC
    "",
    "If Enabled of override of PFB BB256 route address for any outcoming transaction (data/credit/pwire) from PFB unit on broadbus. In case , the route addr will applied constantly from this ROUTE_ADDR_OVERWR_VAL[15:0]. Note that formal PFB BB addr is 17-bit, but practically only 15bit are in use , so 16bit config are enough\n",
#endif
    { CRDP_ING_PFB_CFG_GBL_ROUTE_ADDR_OVERWR_VAL_FIELD_MASK },
    0,
    { CRDP_ING_PFB_CFG_GBL_ROUTE_ADDR_OVERWR_VAL_FIELD_WIDTH },
    { CRDP_ING_PFB_CFG_GBL_ROUTE_ADDR_OVERWR_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_ING_PFB_CFG_GBL_FIELDS[] =
{
    &CRDP_ING_PFB_CFG_GBL_ARB_SCHEME_FIELD,
    &CRDP_ING_PFB_CFG_GBL_ROUTE_ADDR_OVERWR_EN_FIELD,
    &CRDP_ING_PFB_CFG_GBL_SRC_BB_ID_FIELD,
    &CRDP_ING_PFB_CFG_GBL_SRC_BB_ID_OVR_EN_FIELD,
    &CRDP_ING_PFB_CFG_GBL_FLUSH_PACE_TIMEOUT_FIELD,
    &CRDP_ING_PFB_CFG_GBL_ROUTE_ADDR_OVERWR_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_ING_PFB_CFG_GBL *****/
const ru_reg_rec CRDP_ING_PFB_CFG_GBL_REG =
{
    "CFG_GBL",
#if RU_INCLUDE_DESC
    "GLOBAL Register",
    "global config:\n1. arbitration\n",
#endif
    { CRDP_ING_PFB_CFG_GBL_REG_OFFSET },
    0,
    0,
    969,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    6,
    CRDP_ING_PFB_CFG_GBL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_ING_PFB_CFG_ING_PWIRE, TYPE: Type_PFB_CFG_ING_PWIRE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: MASK *****/
const ru_field_rec CRDP_ING_PFB_CFG_ING_PWIRE_MASK_FIELD =
{
    "MASK",
#if RU_INCLUDE_DESC
    "",
    "Config = Number of bits of mask that will be applied on Pseudo-Wire 16-bits input bus.\nNote that Mask config bit applies on the nibble of value be masked , i.e.:\nif desirable pseudo-wire width is 12 bits (need mask only 4 MSBits), mask config field should be =0x7\n",
#endif
    { CRDP_ING_PFB_CFG_ING_PWIRE_MASK_FIELD_MASK },
    0,
    { CRDP_ING_PFB_CFG_ING_PWIRE_MASK_FIELD_WIDTH },
    { CRDP_ING_PFB_CFG_ING_PWIRE_MASK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TO_VAL *****/
const ru_field_rec CRDP_ING_PFB_CFG_ING_PWIRE_TO_VAL_FIELD =
{
    "TO_VAL",
#if RU_INCLUDE_DESC
    "",
    "timeout value (power of 2)\n",
#endif
    { CRDP_ING_PFB_CFG_ING_PWIRE_TO_VAL_FIELD_MASK },
    0,
    { CRDP_ING_PFB_CFG_ING_PWIRE_TO_VAL_FIELD_WIDTH },
    { CRDP_ING_PFB_CFG_ING_PWIRE_TO_VAL_FIELD_SHIFT },
    2,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ING_SPARE_CFG *****/
const ru_field_rec CRDP_ING_PFB_CFG_ING_PWIRE_ING_SPARE_CFG_FIELD =
{
    "ING_SPARE_CFG",
#if RU_INCLUDE_DESC
    "",
    "spare config\n",
#endif
    { CRDP_ING_PFB_CFG_ING_PWIRE_ING_SPARE_CFG_FIELD_MASK },
    0,
    { CRDP_ING_PFB_CFG_ING_PWIRE_ING_SPARE_CFG_FIELD_WIDTH },
    { CRDP_ING_PFB_CFG_ING_PWIRE_ING_SPARE_CFG_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PWIRE_VAL *****/
const ru_field_rec CRDP_ING_PFB_CFG_ING_PWIRE_PWIRE_VAL_FIELD =
{
    "PWIRE_VAL",
#if RU_INCLUDE_DESC
    "",
    "The last captured pseudo-wire on ingress channel after pseudo-wire timer expired its counting (since the last change).\n",
#endif
    { CRDP_ING_PFB_CFG_ING_PWIRE_PWIRE_VAL_FIELD_MASK },
    0,
    { CRDP_ING_PFB_CFG_ING_PWIRE_PWIRE_VAL_FIELD_WIDTH },
    { CRDP_ING_PFB_CFG_ING_PWIRE_PWIRE_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_ING_PFB_CFG_ING_PWIRE_FIELDS[] =
{
    &CRDP_ING_PFB_CFG_ING_PWIRE_MASK_FIELD,
    &CRDP_ING_PFB_CFG_ING_PWIRE_TO_VAL_FIELD,
    &CRDP_ING_PFB_CFG_ING_PWIRE_ING_SPARE_CFG_FIELD,
    &CRDP_ING_PFB_CFG_ING_PWIRE_PWIRE_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_ING_PFB_CFG_ING_PWIRE *****/
const ru_reg_rec CRDP_ING_PFB_CFG_ING_PWIRE_REG =
{
    "CFG_ING_PWIRE",
#if RU_INCLUDE_DESC
    "ING_PWIRE 0..7 Register",
    "ingress Pseudo Wire cfg/ctrl\n",
#endif
    { CRDP_ING_PFB_CFG_ING_PWIRE_REG_OFFSET },
    CRDP_ING_PFB_CFG_ING_PWIRE_REG_RAM_CNT,
    4,
    970,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    CRDP_ING_PFB_CFG_ING_PWIRE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_ING_PFB_CFG_EGR_PWIRE, TYPE: Type_PFB_CFG_EGR_PWIRE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: MASK *****/
const ru_field_rec CRDP_ING_PFB_CFG_EGR_PWIRE_MASK_FIELD =
{
    "MASK",
#if RU_INCLUDE_DESC
    "",
    "Config = Number of bits of mask that will be applied on Pseudo-Wire 16-bits input bus.\nNote that Mask config bit applies on the nibble of value be masked , i.e.:\nif desirable pseudo-wire width is 12 bits (need mask only 4 MSBits), mask config field should be =0x7\n",
#endif
    { CRDP_ING_PFB_CFG_EGR_PWIRE_MASK_FIELD_MASK },
    0,
    { CRDP_ING_PFB_CFG_EGR_PWIRE_MASK_FIELD_WIDTH },
    { CRDP_ING_PFB_CFG_EGR_PWIRE_MASK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TO_VAL *****/
const ru_field_rec CRDP_ING_PFB_CFG_EGR_PWIRE_TO_VAL_FIELD =
{
    "TO_VAL",
#if RU_INCLUDE_DESC
    "",
    "timeout value (power of 2)\n",
#endif
    { CRDP_ING_PFB_CFG_EGR_PWIRE_TO_VAL_FIELD_MASK },
    0,
    { CRDP_ING_PFB_CFG_EGR_PWIRE_TO_VAL_FIELD_WIDTH },
    { CRDP_ING_PFB_CFG_EGR_PWIRE_TO_VAL_FIELD_SHIFT },
    2,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EGR_SPARE_CFG *****/
const ru_field_rec CRDP_ING_PFB_CFG_EGR_PWIRE_EGR_SPARE_CFG_FIELD =
{
    "EGR_SPARE_CFG",
#if RU_INCLUDE_DESC
    "",
    "EGR_SPARE_CFG\n",
#endif
    { CRDP_ING_PFB_CFG_EGR_PWIRE_EGR_SPARE_CFG_FIELD_MASK },
    0,
    { CRDP_ING_PFB_CFG_EGR_PWIRE_EGR_SPARE_CFG_FIELD_WIDTH },
    { CRDP_ING_PFB_CFG_EGR_PWIRE_EGR_SPARE_CFG_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PWIRE_VAL *****/
const ru_field_rec CRDP_ING_PFB_CFG_EGR_PWIRE_PWIRE_VAL_FIELD =
{
    "PWIRE_VAL",
#if RU_INCLUDE_DESC
    "",
    "The last captured pseudo-wire on egress channel after pseudo-wire timer expired its counting (since the last change).\n",
#endif
    { CRDP_ING_PFB_CFG_EGR_PWIRE_PWIRE_VAL_FIELD_MASK },
    0,
    { CRDP_ING_PFB_CFG_EGR_PWIRE_PWIRE_VAL_FIELD_WIDTH },
    { CRDP_ING_PFB_CFG_EGR_PWIRE_PWIRE_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_ING_PFB_CFG_EGR_PWIRE_FIELDS[] =
{
    &CRDP_ING_PFB_CFG_EGR_PWIRE_MASK_FIELD,
    &CRDP_ING_PFB_CFG_EGR_PWIRE_TO_VAL_FIELD,
    &CRDP_ING_PFB_CFG_EGR_PWIRE_EGR_SPARE_CFG_FIELD,
    &CRDP_ING_PFB_CFG_EGR_PWIRE_PWIRE_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_ING_PFB_CFG_EGR_PWIRE *****/
const ru_reg_rec CRDP_ING_PFB_CFG_EGR_PWIRE_REG =
{
    "CFG_EGR_PWIRE",
#if RU_INCLUDE_DESC
    "EGR_PWIRE 0..7 Register",
    "egress Pseudo Wire cfg/ctrl\n",
#endif
    { CRDP_ING_PFB_CFG_EGR_PWIRE_REG_OFFSET },
    CRDP_ING_PFB_CFG_EGR_PWIRE_REG_RAM_CNT,
    4,
    971,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    CRDP_ING_PFB_CFG_EGR_PWIRE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_ING_PFB_CFG_ING_EOP_MASK, TYPE: Type_PFB_CFG_ING_EOP_MASK
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: MASK *****/
const ru_field_rec CRDP_ING_PFB_CFG_ING_EOP_MASK_MASK_FIELD =
{
    "MASK",
#if RU_INCLUDE_DESC
    "",
    "EOP_mask 16-bit (all ones and all zeros) , example: 11h0, 5b11111\n\n",
#endif
    { CRDP_ING_PFB_CFG_ING_EOP_MASK_MASK_FIELD_MASK },
    0,
    { CRDP_ING_PFB_CFG_ING_EOP_MASK_MASK_FIELD_WIDTH },
    { CRDP_ING_PFB_CFG_ING_EOP_MASK_MASK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EOP_MSB_MASK *****/
const ru_field_rec CRDP_ING_PFB_CFG_ING_EOP_MASK_EOP_MSB_MASK_FIELD =
{
    "EOP_MSB_MASK",
#if RU_INCLUDE_DESC
    "",
    "EOP_MSB mask (16-bit) , example:  11h0, 1b1, 4b0\n\n",
#endif
    { CRDP_ING_PFB_CFG_ING_EOP_MASK_EOP_MSB_MASK_FIELD_MASK },
    0,
    { CRDP_ING_PFB_CFG_ING_EOP_MASK_EOP_MSB_MASK_FIELD_WIDTH },
    { CRDP_ING_PFB_CFG_ING_EOP_MASK_EOP_MSB_MASK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_ING_PFB_CFG_ING_EOP_MASK_FIELDS[] =
{
    &CRDP_ING_PFB_CFG_ING_EOP_MASK_MASK_FIELD,
    &CRDP_ING_PFB_CFG_ING_EOP_MASK_EOP_MSB_MASK_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_ING_PFB_CFG_ING_EOP_MASK *****/
const ru_reg_rec CRDP_ING_PFB_CFG_ING_EOP_MASK_REG =
{
    "CFG_ING_EOP_MASK",
#if RU_INCLUDE_DESC
    "ING_EOP_MASK 0..7 Register",
    "ingress EOP mask set\n",
#endif
    { CRDP_ING_PFB_CFG_ING_EOP_MASK_REG_OFFSET },
    CRDP_ING_PFB_CFG_ING_EOP_MASK_REG_RAM_CNT,
    4,
    972,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    CRDP_ING_PFB_CFG_ING_EOP_MASK_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_ING_PFB_DBG_FIFO_STATUS, TYPE: Type_PFB_DBG_FIFO_STATUS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ING_FULL *****/
const ru_field_rec CRDP_ING_PFB_DBG_FIFO_STATUS_ING_FULL_FIELD =
{
    "ING_FULL",
#if RU_INCLUDE_DESC
    "",
    "ingress FIFO full status\n",
#endif
    { CRDP_ING_PFB_DBG_FIFO_STATUS_ING_FULL_FIELD_MASK },
    0,
    { CRDP_ING_PFB_DBG_FIFO_STATUS_ING_FULL_FIELD_WIDTH },
    { CRDP_ING_PFB_DBG_FIFO_STATUS_ING_FULL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ING_EMPTY *****/
const ru_field_rec CRDP_ING_PFB_DBG_FIFO_STATUS_ING_EMPTY_FIELD =
{
    "ING_EMPTY",
#if RU_INCLUDE_DESC
    "",
    "ingress FIFO empty status\n",
#endif
    { CRDP_ING_PFB_DBG_FIFO_STATUS_ING_EMPTY_FIELD_MASK },
    0,
    { CRDP_ING_PFB_DBG_FIFO_STATUS_ING_EMPTY_FIELD_WIDTH },
    { CRDP_ING_PFB_DBG_FIFO_STATUS_ING_EMPTY_FIELD_SHIFT },
    255,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EGR_FULL *****/
const ru_field_rec CRDP_ING_PFB_DBG_FIFO_STATUS_EGR_FULL_FIELD =
{
    "EGR_FULL",
#if RU_INCLUDE_DESC
    "",
    "egress FIFO full status\n",
#endif
    { CRDP_ING_PFB_DBG_FIFO_STATUS_EGR_FULL_FIELD_MASK },
    0,
    { CRDP_ING_PFB_DBG_FIFO_STATUS_EGR_FULL_FIELD_WIDTH },
    { CRDP_ING_PFB_DBG_FIFO_STATUS_EGR_FULL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EGR_EMPTY *****/
const ru_field_rec CRDP_ING_PFB_DBG_FIFO_STATUS_EGR_EMPTY_FIELD =
{
    "EGR_EMPTY",
#if RU_INCLUDE_DESC
    "",
    "egress FIFO empty status\n",
#endif
    { CRDP_ING_PFB_DBG_FIFO_STATUS_EGR_EMPTY_FIELD_MASK },
    0,
    { CRDP_ING_PFB_DBG_FIFO_STATUS_EGR_EMPTY_FIELD_WIDTH },
    { CRDP_ING_PFB_DBG_FIFO_STATUS_EGR_EMPTY_FIELD_SHIFT },
    255,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_ING_PFB_DBG_FIFO_STATUS_FIELDS[] =
{
    &CRDP_ING_PFB_DBG_FIFO_STATUS_ING_FULL_FIELD,
    &CRDP_ING_PFB_DBG_FIFO_STATUS_ING_EMPTY_FIELD,
    &CRDP_ING_PFB_DBG_FIFO_STATUS_EGR_FULL_FIELD,
    &CRDP_ING_PFB_DBG_FIFO_STATUS_EGR_EMPTY_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_ING_PFB_DBG_FIFO_STATUS *****/
const ru_reg_rec CRDP_ING_PFB_DBG_FIFO_STATUS_REG =
{
    "DBG_FIFO_STATUS",
#if RU_INCLUDE_DESC
    "FIFO_STATUS Register",
    "bit per full/empty ingress/egress\n",
#endif
    { CRDP_ING_PFB_DBG_FIFO_STATUS_REG_OFFSET },
    0,
    0,
    973,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    CRDP_ING_PFB_DBG_FIFO_STATUS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_ING_PFB_DBG_INGRESS_USED_WORDS, TYPE: Type_PFB_DBG_INGRESS_USED_WORDS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: USED_WORDS *****/
const ru_field_rec CRDP_ING_PFB_DBG_INGRESS_USED_WORDS_USED_WORDS_FIELD =
{
    "USED_WORDS",
#if RU_INCLUDE_DESC
    "",
    "USED_WORDS\n",
#endif
    { CRDP_ING_PFB_DBG_INGRESS_USED_WORDS_USED_WORDS_FIELD_MASK },
    0,
    { CRDP_ING_PFB_DBG_INGRESS_USED_WORDS_USED_WORDS_FIELD_WIDTH },
    { CRDP_ING_PFB_DBG_INGRESS_USED_WORDS_USED_WORDS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_ING_PFB_DBG_INGRESS_USED_WORDS_FIELDS[] =
{
    &CRDP_ING_PFB_DBG_INGRESS_USED_WORDS_USED_WORDS_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_ING_PFB_DBG_INGRESS_USED_WORDS *****/
const ru_reg_rec CRDP_ING_PFB_DBG_INGRESS_USED_WORDS_REG =
{
    "DBG_INGRESS_USED_WORDS",
#if RU_INCLUDE_DESC
    "INGRESS_USED_WORDS 0..7 Register",
    "ingress used words. occupied entries in ingress FIFO\n",
#endif
    { CRDP_ING_PFB_DBG_INGRESS_USED_WORDS_REG_OFFSET },
    CRDP_ING_PFB_DBG_INGRESS_USED_WORDS_REG_RAM_CNT,
    4,
    974,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_ING_PFB_DBG_INGRESS_USED_WORDS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_ING_PFB_DBG_EGRESS_USED_WORDS, TYPE: Type_PFB_DBG_EGRESS_USED_WORDS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: USED_WORDS *****/
const ru_field_rec CRDP_ING_PFB_DBG_EGRESS_USED_WORDS_USED_WORDS_FIELD =
{
    "USED_WORDS",
#if RU_INCLUDE_DESC
    "",
    "USED_WORDS\n",
#endif
    { CRDP_ING_PFB_DBG_EGRESS_USED_WORDS_USED_WORDS_FIELD_MASK },
    0,
    { CRDP_ING_PFB_DBG_EGRESS_USED_WORDS_USED_WORDS_FIELD_WIDTH },
    { CRDP_ING_PFB_DBG_EGRESS_USED_WORDS_USED_WORDS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_ING_PFB_DBG_EGRESS_USED_WORDS_FIELDS[] =
{
    &CRDP_ING_PFB_DBG_EGRESS_USED_WORDS_USED_WORDS_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_ING_PFB_DBG_EGRESS_USED_WORDS *****/
const ru_reg_rec CRDP_ING_PFB_DBG_EGRESS_USED_WORDS_REG =
{
    "DBG_EGRESS_USED_WORDS",
#if RU_INCLUDE_DESC
    "EGRESS_USED_WORDS 0..7 Register",
    "egress used words. occupied entries in egress FIFO\n",
#endif
    { CRDP_ING_PFB_DBG_EGRESS_USED_WORDS_REG_OFFSET },
    CRDP_ING_PFB_DBG_EGRESS_USED_WORDS_REG_RAM_CNT,
    4,
    975,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_ING_PFB_DBG_EGRESS_USED_WORDS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_ING_PFB_DBG_INGRESS_BB_MSG_TRANS, TYPE: Type_PFB_DBG_INGRESS_BB_MSG_TRANS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: BB_MSG_TRANS *****/
const ru_field_rec CRDP_ING_PFB_DBG_INGRESS_BB_MSG_TRANS_BB_MSG_TRANS_FIELD =
{
    "BB_MSG_TRANS",
#if RU_INCLUDE_DESC
    "",
    "BB_MSG_TRANS\n",
#endif
    { CRDP_ING_PFB_DBG_INGRESS_BB_MSG_TRANS_BB_MSG_TRANS_FIELD_MASK },
    0,
    { CRDP_ING_PFB_DBG_INGRESS_BB_MSG_TRANS_BB_MSG_TRANS_FIELD_WIDTH },
    { CRDP_ING_PFB_DBG_INGRESS_BB_MSG_TRANS_BB_MSG_TRANS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_ING_PFB_DBG_INGRESS_BB_MSG_TRANS_FIELDS[] =
{
    &CRDP_ING_PFB_DBG_INGRESS_BB_MSG_TRANS_BB_MSG_TRANS_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_ING_PFB_DBG_INGRESS_BB_MSG_TRANS *****/
const ru_reg_rec CRDP_ING_PFB_DBG_INGRESS_BB_MSG_TRANS_REG =
{
    "DBG_INGRESS_BB_MSG_TRANS",
#if RU_INCLUDE_DESC
    "INGRESS_BB_MSG_TRANS 0..7 Register",
    "ingress bb msg transmitted.\nsaturates at 32hFFFF_FFFF_FFFF_FFFF.\ncleared on read\n",
#endif
    { CRDP_ING_PFB_DBG_INGRESS_BB_MSG_TRANS_REG_OFFSET },
    CRDP_ING_PFB_DBG_INGRESS_BB_MSG_TRANS_REG_RAM_CNT,
    4,
    976,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_ING_PFB_DBG_INGRESS_BB_MSG_TRANS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_ING_PFB_DBG_EGRESS_BB_MSG_TRANS, TYPE: Type_PFB_DBG_EGRESS_BB_MSG_TRANS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: BB_MSG_TRANS *****/
const ru_field_rec CRDP_ING_PFB_DBG_EGRESS_BB_MSG_TRANS_BB_MSG_TRANS_FIELD =
{
    "BB_MSG_TRANS",
#if RU_INCLUDE_DESC
    "",
    "BB_MSG_TRANS\n",
#endif
    { CRDP_ING_PFB_DBG_EGRESS_BB_MSG_TRANS_BB_MSG_TRANS_FIELD_MASK },
    0,
    { CRDP_ING_PFB_DBG_EGRESS_BB_MSG_TRANS_BB_MSG_TRANS_FIELD_WIDTH },
    { CRDP_ING_PFB_DBG_EGRESS_BB_MSG_TRANS_BB_MSG_TRANS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_ING_PFB_DBG_EGRESS_BB_MSG_TRANS_FIELDS[] =
{
    &CRDP_ING_PFB_DBG_EGRESS_BB_MSG_TRANS_BB_MSG_TRANS_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_ING_PFB_DBG_EGRESS_BB_MSG_TRANS *****/
const ru_reg_rec CRDP_ING_PFB_DBG_EGRESS_BB_MSG_TRANS_REG =
{
    "DBG_EGRESS_BB_MSG_TRANS",
#if RU_INCLUDE_DESC
    "EGRESS_BB_MSG_TRANS 0..7 Register",
    "egress bb messages received.\nsaturates at 32hFFFF_FFFF_FFFF_FFFF.\ncleared on read\n",
#endif
    { CRDP_ING_PFB_DBG_EGRESS_BB_MSG_TRANS_REG_OFFSET },
    CRDP_ING_PFB_DBG_EGRESS_BB_MSG_TRANS_REG_RAM_CNT,
    4,
    977,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_ING_PFB_DBG_EGRESS_BB_MSG_TRANS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_ING_PFB_DBG_INGRESS_CREDIT_CNT, TYPE: Type_PFB_DBG_INGRESS_CREDIT_CNT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: INGRESS_CREDIT_CNT *****/
const ru_field_rec CRDP_ING_PFB_DBG_INGRESS_CREDIT_CNT_INGRESS_CREDIT_CNT_FIELD =
{
    "INGRESS_CREDIT_CNT",
#if RU_INCLUDE_DESC
    "",
    "INGRESS_CREDIT_CNT\n",
#endif
    { CRDP_ING_PFB_DBG_INGRESS_CREDIT_CNT_INGRESS_CREDIT_CNT_FIELD_MASK },
    0,
    { CRDP_ING_PFB_DBG_INGRESS_CREDIT_CNT_INGRESS_CREDIT_CNT_FIELD_WIDTH },
    { CRDP_ING_PFB_DBG_INGRESS_CREDIT_CNT_INGRESS_CREDIT_CNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_ING_PFB_DBG_INGRESS_CREDIT_CNT_FIELDS[] =
{
    &CRDP_ING_PFB_DBG_INGRESS_CREDIT_CNT_INGRESS_CREDIT_CNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_ING_PFB_DBG_INGRESS_CREDIT_CNT *****/
const ru_reg_rec CRDP_ING_PFB_DBG_INGRESS_CREDIT_CNT_REG =
{
    "DBG_INGRESS_CREDIT_CNT",
#if RU_INCLUDE_DESC
    "INGRESS_CREDIT_CNT 0..7 Register",
    "number of credits in ingress FIFO.\ncredit = bb_transaction = 8B+sideband\n",
#endif
    { CRDP_ING_PFB_DBG_INGRESS_CREDIT_CNT_REG_OFFSET },
    CRDP_ING_PFB_DBG_INGRESS_CREDIT_CNT_REG_RAM_CNT,
    4,
    978,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_ING_PFB_DBG_INGRESS_CREDIT_CNT_FIELDS,
#endif
};

unsigned long CRDP_ING_PFB_ADDRS[] =
{
    0xA2150000,
    0xA2160000,
    0xA2170000,
};

static const ru_reg_rec *CRDP_ING_PFB_REGS[] =
{
    &CRDP_ING_PFB_CFG_ING_REG,
    &CRDP_ING_PFB_CFG_EGR_REG,
    &CRDP_ING_PFB_CFG_GBL_REG,
    &CRDP_ING_PFB_CFG_ING_PWIRE_REG,
    &CRDP_ING_PFB_CFG_EGR_PWIRE_REG,
    &CRDP_ING_PFB_CFG_ING_EOP_MASK_REG,
    &CRDP_ING_PFB_DBG_FIFO_STATUS_REG,
    &CRDP_ING_PFB_DBG_INGRESS_USED_WORDS_REG,
    &CRDP_ING_PFB_DBG_EGRESS_USED_WORDS_REG,
    &CRDP_ING_PFB_DBG_INGRESS_BB_MSG_TRANS_REG,
    &CRDP_ING_PFB_DBG_EGRESS_BB_MSG_TRANS_REG,
    &CRDP_ING_PFB_DBG_INGRESS_CREDIT_CNT_REG,
};

const ru_block_rec CRDP_ING_PFB_BLOCK =
{
    "CRDP_ING_PFB",
#if RU_INCLUDE_SIZE
    { 0x000006C0 },
#endif
    CRDP_ING_PFB_ADDRS,
    3,
    12,
    CRDP_ING_PFB_REGS,
};
