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


#include "XRDP_CRDP_EGR_BBH_TX_AG.h"

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MACTYPE, TYPE: Type_BBH_TX_BBHTX_COMMON_CONFIGURATIONS_MACTYPE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: TYPE *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MACTYPE_TYPE_FIELD =
{
    "TYPE",
#if RU_INCLUDE_DESC
    "",
    "MAC type\n\nNote:\nxgpon - refers also to xgs, ngpon, hsp olt\nnni - refers also to nni and xfi\nxepon - refers also to 25g epon\n\n\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MACTYPE_TYPE_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MACTYPE_TYPE_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MACTYPE_TYPE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: REMEH *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MACTYPE_REMEH_FIELD =
{
    "REMEH",
#if RU_INCLUDE_DESC
    "",
    "remove_eh_dis.\nRelevant only for reprocessing mac_type.\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MACTYPE_REMEH_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MACTYPE_REMEH_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MACTYPE_REMEH_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MACTYPE_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MACTYPE_TYPE_FIELD,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MACTYPE_REMEH_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MACTYPE *****/
const ru_reg_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MACTYPE_REG =
{
    "COMMON_CONFIGURATIONS_MACTYPE",
#if RU_INCLUDE_DESC
    "MAC_TYPE 0..7 Register",
    "The BBH supports working with different MAC types. Each MAC requires different interface and features. This register defines the type of MAC the BBH works with.\nFor HSP MACs, please configure as XGPON\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MACTYPE_REG_OFFSET },
    CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MACTYPE_REG_RAM_CNT,
    4,
    593,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MACTYPE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_BBCFG_TX, TYPE: Type_BBH_TX_BBHTX_COMMON_CONFIGURATIONS_BBCFG_TX
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: SBPMSRC *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_BBCFG_TX_SBPMSRC_FIELD =
{
    "SBPMSRC",
#if RU_INCLUDE_DESC
    "",
    "source id. This id is used to determine the route to the module.\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_BBCFG_TX_SBPMSRC_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_BBCFG_TX_SBPMSRC_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_BBCFG_TX_SBPMSRC_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FPMSRC *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_BBCFG_TX_FPMSRC_FIELD =
{
    "FPMSRC",
#if RU_INCLUDE_DESC
    "",
    "source id. This id is used to determine the route to the module.\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_BBCFG_TX_FPMSRC_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_BBCFG_TX_FPMSRC_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_BBCFG_TX_FPMSRC_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_BBCFG_TX_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_BBCFG_TX_SBPMSRC_FIELD,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_BBCFG_TX_FPMSRC_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_BBCFG_TX *****/
const ru_reg_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_BBCFG_TX_REG =
{
    "COMMON_CONFIGURATIONS_BBCFG_TX",
#if RU_INCLUDE_DESC
    "BB_CFG Register",
    "Each BBH unit has its own position on the BB tree. This position defines the Route address when approaching the FPM, TM or SBPM. The route is determined by a dedicated generic logic which uses the source id of the destination.\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_BBCFG_TX_REG_OFFSET },
    0,
    0,
    594,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_BBCFG_TX_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG, TYPE: Type_BBH_TX_BBHTX_COMMON_CONFIGURATIONS_SBPMCFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: FREENOCNTXT *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_FREENOCNTXT_FIELD =
{
    "FREENOCNTXT",
#if RU_INCLUDE_DESC
    "",
    "When this bit is enabled, the BBH will use free without context command.\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_FREENOCNTXT_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_FREENOCNTXT_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_FREENOCNTXT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MC_TYPE_3 *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_MC_TYPE_3_FIELD =
{
    "MC_TYPE_3",
#if RU_INCLUDE_DESC
    "",
    "When this bit is enabled, when freeing a multicast packet, the bbh will use free type 3, meaning the sbpm should check the counter of both the first and the second buffers.\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_MC_TYPE_3_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_MC_TYPE_3_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_MC_TYPE_3_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MC_TYPE_2 *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_MC_TYPE_2_FIELD =
{
    "MC_TYPE_2",
#if RU_INCLUDE_DESC
    "",
    "Same as above but with type 2, meaning the sbpm should check only the second buffer\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_MC_TYPE_2_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_MC_TYPE_2_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_MC_TYPE_2_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SECOND_BN_DIS *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_SECOND_BN_DIS_FIELD =
{
    "SECOND_BN_DIS",
#if RU_INCLUDE_DESC
    "",
    "The bbh tx uses the second BN from the PD if the packet is 2 buffers long. This bit disables this optimization, so the BBH tx will always use get next command to understand what is the next BN\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_SECOND_BN_DIS_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_SECOND_BN_DIS_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_SECOND_BN_DIS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BUFSIZE *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_BUFSIZE_FIELD =
{
    "BUFSIZE",
#if RU_INCLUDE_DESC
    "",
    "buffer size - relevant only for epsram\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_BUFSIZE_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_BUFSIZE_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_BUFSIZE_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ENRRSPREAD *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_ENRRSPREAD_FIELD =
{
    "ENRRSPREAD",
#if RU_INCLUDE_DESC
    "",
    "enable rr spread option in the address spreader\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_ENRRSPREAD_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_ENRRSPREAD_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_ENRRSPREAD_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MAXGN *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_MAXGN_FIELD =
{
    "MAXGN",
#if RU_INCLUDE_DESC
    "",
    "maximum number of pending on the fly get next commands\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_MAXGN_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_MAXGN_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_MAXGN_FIELD_SHIFT },
    12,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: LISTFREETIMER *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_LISTFREETIMER_FIELD =
{
    "LISTFREETIMER",
#if RU_INCLUDE_DESC
    "",
    "timer value in 1024 clock cycles resolution for the case of free of list of buffers. If the list does not reach 8 buffers for a timer time, then the free will occur anyway.\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_LISTFREETIMER_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_LISTFREETIMER_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_LISTFREETIMER_FIELD_SHIFT },
    256,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: LISTFREETHR *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_LISTFREETHR_FIELD =
{
    "LISTFREETHR",
#if RU_INCLUDE_DESC
    "",
    "threshold value for the case of free of list of buffers. When the number of bns in the list reaches thr value + 1, the free command will be sent.\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_LISTFREETHR_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_LISTFREETHR_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_LISTFREETHR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SMID *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_SMID_FIELD =
{
    "SMID",
#if RU_INCLUDE_DESC
    "",
    "id of the sm in the sbpm for get next\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_SMID_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_SMID_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_SMID_FIELD_SHIFT },
    170,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_FREENOCNTXT_FIELD,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_MC_TYPE_3_FIELD,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_MC_TYPE_2_FIELD,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_SECOND_BN_DIS_FIELD,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_BUFSIZE_FIELD,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_ENRRSPREAD_FIELD,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_MAXGN_FIELD,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_LISTFREETIMER_FIELD,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_LISTFREETHR_FIELD,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_SMID_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG *****/
const ru_reg_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_REG =
{
    "COMMON_CONFIGURATIONS_SBPMCFG",
#if RU_INCLUDE_DESC
    "SBPM_CFG Register",
    "When packet transmission is done, the BBH releases the SBPM buffers.\nThis register defines which release command is used:\n1. Normal free with context\n2. Special free with context\n3. free without context\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_REG_OFFSET },
    0,
    0,
    595,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    10,
    CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_BBSRCID, TYPE: Type_BBH_TX_BBHTX_COMMON_CONFIGURATIONS_BBSRCID
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: BBSRCID *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_BBSRCID_BBSRCID_FIELD =
{
    "BBSRCID",
#if RU_INCLUDE_DESC
    "",
    "bb_id\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_BBSRCID_BBSRCID_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_BBSRCID_BBSRCID_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_BBSRCID_BBSRCID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BYP *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_BBSRCID_BYP_FIELD =
{
    "BYP",
#if RU_INCLUDE_DESC
    "",
    "if set, the value will be taken from this register, else it will be taken from an input with the same name\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_BBSRCID_BYP_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_BBSRCID_BYP_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_BBSRCID_BYP_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_BBSRCID_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_BBSRCID_BBSRCID_FIELD,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_BBSRCID_BYP_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_BBSRCID *****/
const ru_reg_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_BBSRCID_REG =
{
    "COMMON_CONFIGURATIONS_BBSRCID",
#if RU_INCLUDE_DESC
    "BB_SRC_ID Register",
    "The BB ID can be taken from an input to the module or from this register\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_BBSRCID_REG_OFFSET },
    0,
    0,
    596,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_BBSRCID_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX, TYPE: Type_BBH_TX_BBHTX_COMMON_CONFIGURATIONS_DDRCFG_TX
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: BUFSIZE0 *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_BUFSIZE0_FIELD =
{
    "BUFSIZE0",
#if RU_INCLUDE_DESC
    "",
    "The data is arranged in the DDR in a fixed size FPM buffers.\n\nThis register determines the FPM0 buffer size.\nFor the FPMini (multicast headers), the size is fixed - 4 Kbytes.\n\nThe\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_BUFSIZE0_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_BUFSIZE0_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_BUFSIZE0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BUFSIZE1 *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_BUFSIZE1_FIELD =
{
    "BUFSIZE1",
#if RU_INCLUDE_DESC
    "",
    "The data is arranged in the DDR in a fixed size FPM1 buffers.\n\nThis register determines the FPM buffer size.\nFor the FPMini (multicast headers), the size is fixed - 4 Kbytes.\n\nThe\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_BUFSIZE1_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_BUFSIZE1_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_BUFSIZE1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: HNSIZE *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_HNSIZE_FIELD =
{
    "HNSIZE",
#if RU_INCLUDE_DESC
    "",
    "The size of the HN (Header number) in bytes.\nIn multicast, several headers are located in the same DDR buffer. the offset of header N within a buffer is (N-1)*64bytes, regardless of the HN actual size.\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_HNSIZE_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_HNSIZE_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_HNSIZE_FIELD_SHIFT },
    64,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_BUFSIZE0_FIELD,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_BUFSIZE1_FIELD,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_HNSIZE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX *****/
const ru_reg_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_REG =
{
    "COMMON_CONFIGURATIONS_DDRCFG_TX",
#if RU_INCLUDE_DESC
    "DDR_ADDR_CFG Register",
    "Configurations for determining the address to read from the DDR/PSRAm\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_REG_OFFSET },
    0,
    0,
    597,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_PDRNRCFG, TYPE: Type_BBH_TX_BBHTX_COMMON_CONFIGURATIONS_PDRNRCFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: WKPADDR *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_PDRNRCFG_WKPADDR_FIELD =
{
    "WKPADDR",
#if RU_INCLUDE_DESC
    "",
    "the address in which to write the wakeup info\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_PDRNRCFG_WKPADDR_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_PDRNRCFG_WKPADDR_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_PDRNRCFG_WKPADDR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TASK *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_PDRNRCFG_TASK_FIELD =
{
    "TASK",
#if RU_INCLUDE_DESC
    "",
    "defines the tx task to wakeup\n\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_PDRNRCFG_TASK_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_PDRNRCFG_TASK_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_PDRNRCFG_TASK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BBSRCID *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_PDRNRCFG_BBSRCID_FIELD =
{
    "BBSRCID",
#if RU_INCLUDE_DESC
    "",
    "defines the bb source id of the TM core to wakeup\n\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_PDRNRCFG_BBSRCID_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_PDRNRCFG_BBSRCID_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_PDRNRCFG_BBSRCID_FIELD_SHIFT },
    64,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_PDRNRCFG_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_PDRNRCFG_WKPADDR_FIELD,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_PDRNRCFG_TASK_FIELD,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_PDRNRCFG_BBSRCID_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_PDRNRCFG *****/
const ru_reg_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_PDRNRCFG_REG =
{
    "COMMON_CONFIGURATIONS_PDRNRCFG",
#if RU_INCLUDE_DESC
    "PD_RNR_CFG 0..7 Register",
    "This array holds the configurations related to the TM core that each BBH port is using.\nIn case of copy BBH, these configurations are  related to the QM Copy.\n\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_PDRNRCFG_REG_OFFSET },
    CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_PDRNRCFG_REG_RAM_CNT,
    4,
    598,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_PDRNRCFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD, TYPE: Type_BBH_TX_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CNTXTRST *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_CNTXTRST_FIELD =
{
    "CNTXTRST",
#if RU_INCLUDE_DESC
    "",
    "Writing 1 to this register will reset the segmentation context table.\nThe reset is done immediately. Reading this register will always return 0.\nTRIGGERS_HW\n\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_CNTXTRST_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_CNTXTRST_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_CNTXTRST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PDFIFORST *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_PDFIFORST_FIELD =
{
    "PDFIFORST",
#if RU_INCLUDE_DESC
    "",
    "Writing 1 to this register will reset the PDs FIFOs.\nThe reset is done immediately. Reading this register will always return 0.\nTRIGGERS_HW\n\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_PDFIFORST_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_PDFIFORST_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_PDFIFORST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BPMFIFORST *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_BPMFIFORST_FIELD =
{
    "BPMFIFORST",
#if RU_INCLUDE_DESC
    "",
    "Writing 1 to this register will reset the BPM FIFO.\nThe reset is done immediately. Reading this register will always return 0.\nTRIGGERS_HW\n\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_BPMFIFORST_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_BPMFIFORST_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_BPMFIFORST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SBPMFIFORST *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_SBPMFIFORST_FIELD =
{
    "SBPMFIFORST",
#if RU_INCLUDE_DESC
    "",
    "Writing 1 to this register will reset the SBPM FIFO.\nThe reset is done immediately. Reading this register will always return 0.\nThis register is relevalt only for Ethernet.\nTRIGGERS_HW\n\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_SBPMFIFORST_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_SBPMFIFORST_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_SBPMFIFORST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDRFIFORST *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_DDRFIFORST_FIELD =
{
    "DDRFIFORST",
#if RU_INCLUDE_DESC
    "",
    "Writing 1 to this register will reset the DDR data FIFO.\nThe reset is done immediately. Reading this register will always return 0.\nThis register is relevalt only for Ethernet.\nTRIGGERS_HW\n\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_DDRFIFORST_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_DDRFIFORST_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_DDRFIFORST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SRAMFIFORST *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_SRAMFIFORST_FIELD =
{
    "SRAMFIFORST",
#if RU_INCLUDE_DESC
    "",
    "Writing 1 to this register will reset the SRAM data FIFO.\nThe reset is done immediately. Reading this register will always return 0.\nThis register is relevalt only for Ethernet.\nTRIGGERS_HW\n\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_SRAMFIFORST_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_SRAMFIFORST_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_SRAMFIFORST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SKBPTRRST *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_SKBPTRRST_FIELD =
{
    "SKBPTRRST",
#if RU_INCLUDE_DESC
    "",
    "Writing 1 to this register will reset the SKB pointers.\nThe reset is done immediately. Reading this register will always return 0.\nTRIGGERS_HW\n\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_SKBPTRRST_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_SKBPTRRST_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_SKBPTRRST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: GNXTFIFORST *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_GNXTFIFORST_FIELD =
{
    "GNXTFIFORST",
#if RU_INCLUDE_DESC
    "",
    "Writing 1 to this register will reset the GET NEXT FIFOs\nThe reset is done immediately. Reading this register will always return 0.\nTRIGGERS_HW\n\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_GNXTFIFORST_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_GNXTFIFORST_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_GNXTFIFORST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FBNFIFORST *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_FBNFIFORST_FIELD =
{
    "FBNFIFORST",
#if RU_INCLUDE_DESC
    "",
    "Writing 1 to this register will reset the FIRST BN FIFOs\nThe reset is done immediately. Reading this register will always return 0.\nTRIGGERS_HW\n\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_FBNFIFORST_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_FBNFIFORST_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_FBNFIFORST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PORT *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_PORT_FIELD =
{
    "PORT",
#if RU_INCLUDE_DESC
    "",
    "relevant for resetting specific fifo, for example, PD FIFO of a certain port.\n\nPD FIFO\nGET NEXT FIFO\nFIRST_BN FIFO\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_PORT_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_PORT_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_PORT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_CNTXTRST_FIELD,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_PDFIFORST_FIELD,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_BPMFIFORST_FIELD,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_SBPMFIFORST_FIELD,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_DDRFIFORST_FIELD,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_SRAMFIFORST_FIELD,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_SKBPTRRST_FIELD,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_GNXTFIFORST_FIELD,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_FBNFIFORST_FIELD,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_PORT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD *****/
const ru_reg_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_REG =
{
    "COMMON_CONFIGURATIONS_TXRSTCMD",
#if RU_INCLUDE_DESC
    "TX_RESET_COMMAND Register",
    "This register enables reset of internal units (for possible WA purposes).\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_REG_OFFSET },
    0,
    0,
    599,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    10,
    CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_DBGSEL, TYPE: Type_BBH_TX_BBHTX_COMMON_CONFIGURATIONS_DBGSEL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DBGSEL *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_DBGSEL_DBGSEL_FIELD =
{
    "DBGSEL",
#if RU_INCLUDE_DESC
    "",
    "This register selects 1 of 8 debug vectors.\nThe selected vector is reflected to DBGOUTREG.\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_DBGSEL_DBGSEL_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_DBGSEL_DBGSEL_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_DBGSEL_DBGSEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_DBGSEL_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_DBGSEL_DBGSEL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_DBGSEL *****/
const ru_reg_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_DBGSEL_REG =
{
    "COMMON_CONFIGURATIONS_DBGSEL",
#if RU_INCLUDE_DESC
    "DEBUG_SELECT Register",
    "This register selects 1 of 8 debug vectors.\nThe selected vector is reflected to DBGOUTREG.\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_DBGSEL_REG_OFFSET },
    0,
    0,
    600,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_DBGSEL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL, TYPE: Type_BBH_TX_BBHTX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: BYPASS_CLK_GATE *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD =
{
    "BYPASS_CLK_GATE",
#if RU_INCLUDE_DESC
    "",
    "If set to 1b1 will disable the clock gate logic such to always enable the clock\nTRIGGERS_HW\n\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TIMER_VAL *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_TIMER_VAL_FIELD =
{
    "TIMER_VAL",
#if RU_INCLUDE_DESC
    "",
    "For how long should the clock stay active once all conditions for clock disable are met.\nTRIGGERS_HW\n\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_TIMER_VAL_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_TIMER_VAL_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_TIMER_VAL_FIELD_SHIFT },
    15,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: KEEP_ALIVE_EN *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD =
{
    "KEEP_ALIVE_EN",
#if RU_INCLUDE_DESC
    "",
    "Enables the keep alive logic which will periodically enable the clock to assure that no deadlock of clock being removed completely will occur\nTRIGGERS_HW\n\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: KEEP_ALIVE_INTRVL *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD =
{
    "KEEP_ALIVE_INTRVL",
#if RU_INCLUDE_DESC
    "",
    "If the KEEP alive option is enabled the field will determine for how many cycles should the clock be active\nTRIGGERS_HW\n\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD_SHIFT },
    2,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: KEEP_ALIVE_CYC *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD =
{
    "KEEP_ALIVE_CYC",
#if RU_INCLUDE_DESC
    "",
    "If the KEEP alive option is enabled this field will determine for how many cycles should the clock be disabled (minus the KEEP_ALIVE_INTERVAL)\n\nSo KEEP_ALIVE_CYCLE must be larger than KEEP_ALIVE_INTERVAL.\nTRIGGERS_HW\n\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD_SHIFT },
    15,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_TIMER_VAL_FIELD,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL *****/
const ru_reg_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_REG =
{
    "COMMON_CONFIGURATIONS_CLK_GATE_CNTRL",
#if RU_INCLUDE_DESC
    "CLOCK_GATE_CONTROL Register",
    "Clock Gate control register including timer config and bypass control\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_REG_OFFSET },
    0,
    0,
    601,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_GPR, TYPE: Type_BBH_TX_BBHTX_COMMON_CONFIGURATIONS_GPR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: GPR *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_GPR_GPR_FIELD =
{
    "GPR",
#if RU_INCLUDE_DESC
    "",
    "general purpose register\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_GPR_GPR_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_GPR_GPR_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_GPR_GPR_FIELD_SHIFT },
    3,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_GPR_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_GPR_GPR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_GPR *****/
const ru_reg_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_GPR_REG =
{
    "COMMON_CONFIGURATIONS_GPR",
#if RU_INCLUDE_DESC
    "GENERAL_PURPOSE_REGISTER Register",
    "general purpose register\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_GPR_REG_OFFSET },
    0,
    0,
    602,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_GPR_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG, TYPE: Type_BBH_TX_BBHTX_COMMON_CONFIGURATIONS_GENERAL_CFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: MACDROP *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_MACDROP_FIELD =
{
    "MACDROP",
#if RU_INCLUDE_DESC
    "",
    "when asserted, the BBH TX will drop the packet data of an invalid MAC flow\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_MACDROP_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_MACDROP_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_MACDROP_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DISCOPYCRDT *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_DISCOPYCRDT_FIELD =
{
    "DISCOPYCRDT",
#if RU_INCLUDE_DESC
    "",
    "For the copy BBH TX, there is no need to check for credits in the FE buffer. When this bit is asserted, the segmentation SM will not check for credits, and the unified IF will not read from the data FIFO is the FE buffers are full.\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_DISCOPYCRDT_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_DISCOPYCRDT_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_DISCOPYCRDT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ECNRMRK *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_ECNRMRK_FIELD =
{
    "ECNRMRK",
#if RU_INCLUDE_DESC
    "",
    "when asserted, the BBH TX will change the ecn bit according to the information in the PD + embedded header\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_ECNRMRK_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_ECNRMRK_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_ECNRMRK_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CNTRADDCRC *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_CNTRADDCRC_FIELD =
{
    "CNTRADDCRC",
#if RU_INCLUDE_DESC
    "",
    "when asserted, the BBH TX will add 4 bytes per packet to all the byte counters, compensating the crc bytes\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_CNTRADDCRC_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_CNTRADDCRC_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_CNTRADDCRC_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CRCFORWARD *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_CRCFORWARD_FIELD =
{
    "CRCFORWARD",
#if RU_INCLUDE_DESC
    "",
    "If asserted the BBH will mark the forward CRC (meaning not append or replace) towards the NNI\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_CRCFORWARD_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_CRCFORWARD_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_CRCFORWARD_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MULTI_TCONT *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_MULTI_TCONT_FIELD =
{
    "MULTI_TCONT",
#if RU_INCLUDE_DESC
    "",
    "when asserted, the bbh will work in a multi tcont mode (affects segmentation SM and runner SM).\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_MULTI_TCONT_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_MULTI_TCONT_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_MULTI_TCONT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DISTARGETPERPORT *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_DISTARGETPERPORT_FIELD =
{
    "DISTARGETPERPORT",
#if RU_INCLUDE_DESC
    "",
    "the bbh manages credits per target memory per port. When this bit is asserted, the credits will be managed globally (per target mem, not per port). The credits configurations will be done in the configuration set of port 0.\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_DISTARGETPERPORT_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_DISTARGETPERPORT_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_DISTARGETPERPORT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DISCRDTCHK *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_DISCRDTCHK_FIELD =
{
    "DISCRDTCHK",
#if RU_INCLUDE_DESC
    "",
    "when asserted the segmentation sm will not check for credits for preventing HOL before issuing request. It will only check for physical space in order to verify no overrun occurs.\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_DISCRDTCHK_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_DISCRDTCHK_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_DISCRDTCHK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_MACDROP_FIELD,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_DISCOPYCRDT_FIELD,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_ECNRMRK_FIELD,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_CNTRADDCRC_FIELD,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_CRCFORWARD_FIELD,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_MULTI_TCONT_FIELD,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_DISTARGETPERPORT_FIELD,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_DISCRDTCHK_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG *****/
const ru_reg_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_REG =
{
    "COMMON_CONFIGURATIONS_GENERAL_CFG",
#if RU_INCLUDE_DESC
    "GENERAL_CONFIG Register",
    "1. configuration whether the BBH works with single DMA for both DDR and SRAM accesses or not\n2. configuration whether to support 640 bytes aggregation\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_REG_OFFSET },
    0,
    0,
    603,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ECNCFG, TYPE: Type_BBH_TX_BBHTX_COMMON_CONFIGURATIONS_ECNCFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ECNBYTEIPV4 *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ECNCFG_ECNBYTEIPV4_FIELD =
{
    "ECNBYTEIPV4",
#if RU_INCLUDE_DESC
    "",
    "the number of byte in which the ecn bits are located in ipv4 packets, relative to the ip header offset\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ECNCFG_ECNBYTEIPV4_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ECNCFG_ECNBYTEIPV4_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ECNCFG_ECNBYTEIPV4_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ECNBITIPV4 *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ECNCFG_ECNBITIPV4_FIELD =
{
    "ECNBITIPV4",
#if RU_INCLUDE_DESC
    "",
    "the bit offset of the ECN bits within the byte. 0 means the ecn bits are [1:0], 6 means the ecn bits are [7:6]\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ECNCFG_ECNBITIPV4_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ECNCFG_ECNBITIPV4_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ECNCFG_ECNBITIPV4_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ECNBYTEIPV6 *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ECNCFG_ECNBYTEIPV6_FIELD =
{
    "ECNBYTEIPV6",
#if RU_INCLUDE_DESC
    "",
    "the number of byte in which the ecn bits are located in ipv6 packets, relative to the ip header offset\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ECNCFG_ECNBYTEIPV6_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ECNCFG_ECNBYTEIPV6_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ECNCFG_ECNBYTEIPV6_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ECNBITIPV6 *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ECNCFG_ECNBITIPV6_FIELD =
{
    "ECNBITIPV6",
#if RU_INCLUDE_DESC
    "",
    "the bit offset of the ECN bits within the byte. 0 means the ecn bits are [1:0], 6 means the ecn bits are [7:6]\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ECNCFG_ECNBITIPV6_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ECNCFG_ECNBITIPV6_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ECNCFG_ECNBITIPV6_FIELD_SHIFT },
    4,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CHKSUMBYTEIPV4 *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ECNCFG_CHKSUMBYTEIPV4_FIELD =
{
    "CHKSUMBYTEIPV4",
#if RU_INCLUDE_DESC
    "",
    "the offset of the checksum field related to ip header offset in ipv4 packets. The offset must be multiplication of 2\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ECNCFG_CHKSUMBYTEIPV4_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ECNCFG_CHKSUMBYTEIPV4_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ECNCFG_CHKSUMBYTEIPV4_FIELD_SHIFT },
    10,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ASSIGNEDQCNT *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ECNCFG_ASSIGNEDQCNT_FIELD =
{
    "ASSIGNEDQCNT",
#if RU_INCLUDE_DESC
    "",
    "to which port to assign the one set of ecn marked packets and bytes counters\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ECNCFG_ASSIGNEDQCNT_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ECNCFG_ASSIGNEDQCNT_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ECNCFG_ASSIGNEDQCNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ECNCFG_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ECNCFG_ECNBYTEIPV4_FIELD,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ECNCFG_ECNBITIPV4_FIELD,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ECNCFG_ECNBYTEIPV6_FIELD,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ECNCFG_ECNBITIPV6_FIELD,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ECNCFG_CHKSUMBYTEIPV4_FIELD,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ECNCFG_ASSIGNEDQCNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ECNCFG *****/
const ru_reg_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ECNCFG_REG =
{
    "COMMON_CONFIGURATIONS_ECNCFG",
#if RU_INCLUDE_DESC
    "ECN_CONFIGS Register",
    "configurations related to ecn\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ECNCFG_REG_OFFSET },
    0,
    0,
    604,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    6,
    CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ECNCFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_DDRFPMINIBASEL, TYPE: Type_BBH_TX_BBHTX_COMMON_CONFIGURATIONS_DDRFPMINIBASEL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DDRFPMINIBASE *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_DDRFPMINIBASEL_DDRFPMINIBASE_FIELD =
{
    "DDRFPMINIBASE",
#if RU_INCLUDE_DESC
    "",
    "DDR FPMINI base.\nThe address is in bytes resolution.\nThe address should be aligned to 128 bytes.\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_DDRFPMINIBASEL_DDRFPMINIBASE_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_DDRFPMINIBASEL_DDRFPMINIBASE_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_DDRFPMINIBASEL_DDRFPMINIBASE_FIELD_SHIFT },
    8388608,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_DDRFPMINIBASEL_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_DDRFPMINIBASEL_DDRFPMINIBASE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_DDRFPMINIBASEL *****/
const ru_reg_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_DDRFPMINIBASEL_REG =
{
    "COMMON_CONFIGURATIONS_DDRFPMINIBASEL",
#if RU_INCLUDE_DESC
    "DDR_TM_BASE_LOW_FPMINI 0..1 Register",
    "The BBH calculate the DDR physical address according to the Buffer number and buffer size and then adds the DDR TM base.\n\nThe value of this register should match the relevant registers value in the QM and the Runner.\n\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_DDRFPMINIBASEL_REG_OFFSET },
    CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_DDRFPMINIBASEL_REG_RAM_CNT,
    4,
    605,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_DDRFPMINIBASEL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_DDRFPMINIBASEH, TYPE: Type_BBH_TX_BBHTX_COMMON_CONFIGURATIONS_DDRFPMINIBASEH
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DDRFPMINIBASE *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_DDRFPMINIBASEH_DDRFPMINIBASE_FIELD =
{
    "DDRFPMINIBASE",
#if RU_INCLUDE_DESC
    "",
    "DDR FPMINI base.\nThe address is in bytes resolution.\nThe address should be aligned to 128 bytes.\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_DDRFPMINIBASEH_DDRFPMINIBASE_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_DDRFPMINIBASEH_DDRFPMINIBASE_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_DDRFPMINIBASEH_DDRFPMINIBASE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_DDRFPMINIBASEH_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_DDRFPMINIBASEH_DDRFPMINIBASE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_DDRFPMINIBASEH *****/
const ru_reg_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_DDRFPMINIBASEH_REG =
{
    "COMMON_CONFIGURATIONS_DDRFPMINIBASEH",
#if RU_INCLUDE_DESC
    "DDR_TM_BASE_HIGH_FPMINI 0..1 Register",
    "The BBH calculate the DDR physical address according to the Buffer number and buffer size and then adds the DDR TM base.\n\nThe value of this register should match the relevant registers value in the QM and the Runner.\n\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_DDRFPMINIBASEH_REG_OFFSET },
    CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_DDRFPMINIBASEH_REG_RAM_CNT,
    4,
    606,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_DDRFPMINIBASEH_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_BBOVRDROUTE, TYPE: Type_BBH_TX_BBHTX_COMMON_CONFIGURATIONS_BBOVRDROUTE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ROUTE *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_BBOVRDROUTE_ROUTE_FIELD =
{
    "ROUTE",
#if RU_INCLUDE_DESC
    "",
    "route address\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_BBOVRDROUTE_ROUTE_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_BBOVRDROUTE_ROUTE_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_BBOVRDROUTE_ROUTE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_BBOVRDROUTE_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_BBOVRDROUTE_ROUTE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_BBOVRDROUTE *****/
const ru_reg_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_BBOVRDROUTE_REG =
{
    "COMMON_CONFIGURATIONS_BBOVRDROUTE",
#if RU_INCLUDE_DESC
    "BB_OVERRIDE_ROUTE Register",
    "override configuration for the route of one of the peripherals\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_BBOVRDROUTE_REG_OFFSET },
    0,
    0,
    607,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_BBOVRDROUTE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_BBOVRDCTRL, TYPE: Type_BBH_TX_BBHTX_COMMON_CONFIGURATIONS_BBOVRDCTRL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DEST *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_BBOVRDCTRL_DEST_FIELD =
{
    "DEST",
#if RU_INCLUDE_DESC
    "",
    "destination source id\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_BBOVRDCTRL_DEST_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_BBOVRDCTRL_DEST_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_BBOVRDCTRL_DEST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EN *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_BBOVRDCTRL_EN_FIELD =
{
    "EN",
#if RU_INCLUDE_DESC
    "",
    "enable\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_BBOVRDCTRL_EN_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_BBOVRDCTRL_EN_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_BBOVRDCTRL_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_BBOVRDCTRL_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_BBOVRDCTRL_DEST_FIELD,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_BBOVRDCTRL_EN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_BBOVRDCTRL *****/
const ru_reg_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_BBOVRDCTRL_REG =
{
    "COMMON_CONFIGURATIONS_BBOVRDCTRL",
#if RU_INCLUDE_DESC
    "BB_OVERRIDE_CTRL Register",
    "override configuration for the route of one of the peripherals (DMA/SDMMA/FPM/SBPM?Runners)\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_BBOVRDCTRL_REG_OFFSET },
    0,
    0,
    608,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_BBOVRDCTRL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_TMBASEL, TYPE: Type_BBH_TX_BBHTX_COMMON_CONFIGURATIONS_TMBASEL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DDRTMBASE *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_TMBASEL_DDRTMBASE_FIELD =
{
    "DDRTMBASE",
#if RU_INCLUDE_DESC
    "",
    "DDR TM base.\nThe address is in bytes resolution.\nThe address should be aligned to 128 bytes.\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_TMBASEL_DDRTMBASE_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_TMBASEL_DDRTMBASE_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_TMBASEL_DDRTMBASE_FIELD_SHIFT },
    8388608,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_TMBASEL_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_TMBASEL_DDRTMBASE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_TMBASEL *****/
const ru_reg_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_TMBASEL_REG =
{
    "COMMON_CONFIGURATIONS_TMBASEL",
#if RU_INCLUDE_DESC
    "TM_BASE_LOW 0..3 Register",
    "Array of 32 registers. first 8 is for DDR responders FPM0, next 8 is for FPM1, next 8 is for EPSRAM0 responders and the last 8 are for EPSRAM1\n\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_TMBASEL_REG_OFFSET },
    CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_TMBASEL_REG_RAM_CNT,
    4,
    609,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_TMBASEL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_TMBASEH, TYPE: Type_BBH_TX_BBHTX_COMMON_CONFIGURATIONS_TMBASEH
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DDRTMBASE *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_TMBASEH_DDRTMBASE_FIELD =
{
    "DDRTMBASE",
#if RU_INCLUDE_DESC
    "",
    "MSB of DDR TM base.\n\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_TMBASEH_DDRTMBASE_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_TMBASEH_DDRTMBASE_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_TMBASEH_DDRTMBASE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_TMBASEH_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_TMBASEH_DDRTMBASE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_TMBASEH *****/
const ru_reg_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_TMBASEH_REG =
{
    "COMMON_CONFIGURATIONS_TMBASEH",
#if RU_INCLUDE_DESC
    "TM_BASE_HIGH 0..3 Register",
    "Array of 32 registers. first 8 is for FPM0 DDR responders, next 8 is for FPM1, next 8 is for EPSRAM0 responders and the last 8 are for EPSRAM1\n\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_TMBASEH_REG_OFFSET },
    CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_TMBASEH_REG_RAM_CNT,
    4,
    610,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_TMBASEH_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_REORDERFIFOSIZE, TYPE: Type_BBH_TX_BBHTX_COMMON_CONFIGURATIONS_REORDERFIFOSIZE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: SIZE *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_REORDERFIFOSIZE_SIZE_FIELD =
{
    "SIZE",
#if RU_INCLUDE_DESC
    "",
    "FIFO size in 32 bytes resolution\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_REORDERFIFOSIZE_SIZE_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_REORDERFIFOSIZE_SIZE_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_REORDERFIFOSIZE_SIZE_FIELD_SHIFT },
    64,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_REORDERFIFOSIZE_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_REORDERFIFOSIZE_SIZE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_REORDERFIFOSIZE *****/
const ru_reg_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_REORDERFIFOSIZE_REG =
{
    "COMMON_CONFIGURATIONS_REORDERFIFOSIZE",
#if RU_INCLUDE_DESC
    "REORDER_RESP_DATA_FIFO_SIZE 0..23 Register",
    "Array of 24 registers. first 8 is for DDR responders, next 8 is for EPSRAM0 responders and the last 8 are for EPSRAM1\n\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_REORDERFIFOSIZE_REG_OFFSET },
    CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_REORDERFIFOSIZE_REG_RAM_CNT,
    4,
    611,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_REORDERFIFOSIZE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_UBUSCREDITS, TYPE: Type_BBH_TX_BBHTX_COMMON_CONFIGURATIONS_UBUSCREDITS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CREDITS *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_UBUSCREDITS_CREDITS_FIELD =
{
    "CREDITS",
#if RU_INCLUDE_DESC
    "",
    "credits\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_UBUSCREDITS_CREDITS_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_UBUSCREDITS_CREDITS_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_UBUSCREDITS_CREDITS_FIELD_SHIFT },
    8,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_UBUSCREDITS_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_UBUSCREDITS_CREDITS_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_UBUSCREDITS *****/
const ru_reg_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_UBUSCREDITS_REG =
{
    "COMMON_CONFIGURATIONS_UBUSCREDITS",
#if RU_INCLUDE_DESC
    "UBUS_RESP_CREDITS 0..23 Register",
    "Array of 24 registers. first 8 is for DDR responders, next 8 is for EPSRAM0 responders and the last 8 are for EPSRAM1\n\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_UBUSCREDITS_REG_OFFSET },
    CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_UBUSCREDITS_REG_RAM_CNT,
    4,
    612,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_UBUSCREDITS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_CREDITINIT, TYPE: Type_BBH_TX_BBHTX_COMMON_CONFIGURATIONS_CREDITINIT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: INIT *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_CREDITINIT_INIT_FIELD =
{
    "INIT",
#if RU_INCLUDE_DESC
    "",
    "per esponder\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_CREDITINIT_INIT_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_CREDITINIT_INIT_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_CREDITINIT_INIT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_CREDITINIT_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_CREDITINIT_INIT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_CREDITINIT *****/
const ru_reg_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_CREDITINIT_REG =
{
    "COMMON_CONFIGURATIONS_CREDITINIT",
#if RU_INCLUDE_DESC
    "UBUS_RESP_CREDITS_INIT Register",
    "init pulse for initializing the credit counter per responder\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_CREDITINIT_REG_OFFSET },
    0,
    0,
    613,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_CREDITINIT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MIRRORRNRCFG, TYPE: Type_BBH_TX_BBHTX_COMMON_CONFIGURATIONS_MIRRORRNRCFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: WKPADDR *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MIRRORRNRCFG_WKPADDR_FIELD =
{
    "WKPADDR",
#if RU_INCLUDE_DESC
    "",
    "the address in which to write the wakeup info\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MIRRORRNRCFG_WKPADDR_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MIRRORRNRCFG_WKPADDR_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MIRRORRNRCFG_WKPADDR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TASK *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MIRRORRNRCFG_TASK_FIELD =
{
    "TASK",
#if RU_INCLUDE_DESC
    "",
    "defines the tx task to wakeup\n\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MIRRORRNRCFG_TASK_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MIRRORRNRCFG_TASK_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MIRRORRNRCFG_TASK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BBSRCID *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MIRRORRNRCFG_BBSRCID_FIELD =
{
    "BBSRCID",
#if RU_INCLUDE_DESC
    "",
    "defines the bb source id of the TM core to wakeup\n\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MIRRORRNRCFG_BBSRCID_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MIRRORRNRCFG_BBSRCID_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MIRRORRNRCFG_BBSRCID_FIELD_SHIFT },
    64,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MIRRORRNRCFG_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MIRRORRNRCFG_WKPADDR_FIELD,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MIRRORRNRCFG_TASK_FIELD,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MIRRORRNRCFG_BBSRCID_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MIRRORRNRCFG *****/
const ru_reg_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MIRRORRNRCFG_REG =
{
    "COMMON_CONFIGURATIONS_MIRRORRNRCFG",
#if RU_INCLUDE_DESC
    "MIRROR_RNR_CFG 0..7 Register",
    "This register holds the configurations related to the TM core that is responsible for port mirroring\n\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MIRRORRNRCFG_REG_OFFSET },
    CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MIRRORRNRCFG_REG_RAM_CNT,
    4,
    614,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MIRRORRNRCFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MIRRORCFG, TYPE: Type_BBH_TX_BBHTX_COMMON_CONFIGURATIONS_MIRRORCFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: WKPGRAN *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MIRRORCFG_WKPGRAN_FIELD =
{
    "WKPGRAN",
#if RU_INCLUDE_DESC
    "",
    "wakeup granularity - upon how many pd pops to wakeup the runner.\nOne hot vector - possible options:\n1,2,4,8,16,32,64,128\nIf configured to 0, there will not be a wakeup.\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MIRRORCFG_WKPGRAN_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MIRRORCFG_WKPGRAN_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MIRRORCFG_WKPGRAN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PTRRST *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MIRRORCFG_PTRRST_FIELD =
{
    "PTRRST",
#if RU_INCLUDE_DESC
    "",
    "resetting the pd wr pointer\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MIRRORCFG_PTRRST_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MIRRORCFG_PTRRST_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MIRRORCFG_PTRRST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ENABLE *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MIRRORCFG_ENABLE_FIELD =
{
    "ENABLE",
#if RU_INCLUDE_DESC
    "",
    "enable mirroring.\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MIRRORCFG_ENABLE_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MIRRORCFG_ENABLE_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MIRRORCFG_ENABLE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TIMER *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MIRRORCFG_TIMER_FIELD =
{
    "TIMER",
#if RU_INCLUDE_DESC
    "",
    "timer value in 1024 cycles resolution. In cases in which the granularity is larger than 1, the bbh tx will send wakeup to the runner if the granularity is achieved or if the timer expired. The timer starts ticking when the bbh finished handling the first pd after wakeup.\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MIRRORCFG_TIMER_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MIRRORCFG_TIMER_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MIRRORCFG_TIMER_FIELD_SHIFT },
    16,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MIRRORCFG_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MIRRORCFG_WKPGRAN_FIELD,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MIRRORCFG_PTRRST_FIELD,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MIRRORCFG_ENABLE_FIELD,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MIRRORCFG_TIMER_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MIRRORCFG *****/
const ru_reg_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MIRRORCFG_REG =
{
    "COMMON_CONFIGURATIONS_MIRRORCFG",
#if RU_INCLUDE_DESC
    "MIRROR_CFG 0..7 Register",
    "This register holds the configurations related to the port mirroring\n\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MIRRORCFG_REG_OFFSET },
    CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MIRRORCFG_REG_RAM_CNT,
    4,
    615,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MIRRORCFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MULTIBBHCFG, TYPE: Type_BBH_TX_BBHTX_COMMON_CONFIGURATIONS_MULTIBBHCFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: SNINITVAL *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MULTIBBHCFG_SNINITVAL_FIELD =
{
    "SNINITVAL",
#if RU_INCLUDE_DESC
    "",
    "initial value of the serial number. The BBH increments it with the number of packets every time a PD arrives. When sending it to the PMA, the BBH will increment each packet one by one.\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MULTIBBHCFG_SNINITVAL_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MULTIBBHCFG_SNINITVAL_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MULTIBBHCFG_SNINITVAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SNINIT *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MULTIBBHCFG_SNINIT_FIELD =
{
    "SNINIT",
#if RU_INCLUDE_DESC
    "",
    "init the serial number\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MULTIBBHCFG_SNINIT_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MULTIBBHCFG_SNINIT_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MULTIBBHCFG_SNINIT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: LBCTRRST *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MULTIBBHCFG_LBCTRRST_FIELD =
{
    "LBCTRRST",
#if RU_INCLUDE_DESC
    "",
    "option to reset the bytes counter per BBH. Total of 4 bytes counters (one per BBH). Used for load balancing\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MULTIBBHCFG_LBCTRRST_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MULTIBBHCFG_LBCTRRST_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MULTIBBHCFG_LBCTRRST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MODE *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MULTIBBHCFG_MODE_FIELD =
{
    "MODE",
#if RU_INCLUDE_DESC
    "",
    "200 or 400G mode\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MULTIBBHCFG_MODE_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MULTIBBHCFG_MODE_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MULTIBBHCFG_MODE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MULTIBBHCFG_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MULTIBBHCFG_SNINITVAL_FIELD,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MULTIBBHCFG_SNINIT_FIELD,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MULTIBBHCFG_LBCTRRST_FIELD,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MULTIBBHCFG_MODE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MULTIBBHCFG *****/
const ru_reg_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MULTIBBHCFG_REG =
{
    "COMMON_CONFIGURATIONS_MULTIBBHCFG",
#if RU_INCLUDE_DESC
    "MULTI_BBH_MODE Register",
    "for 200 or 400G support, 2 or 4 BBH TX are combined. One of them is defined as manager. It gets the PDs for all, mark them with serial number and send them to the rest of the BBHs with load balancing between them\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MULTIBBHCFG_REG_OFFSET },
    0,
    0,
    616,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MULTIBBHCFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MULTIBBSRC, TYPE: Type_BBH_TX_BBHTX_COMMON_CONFIGURATIONS_MULTIBBSRC
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: BBSRCID *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MULTIBBSRC_BBSRCID_FIELD =
{
    "BBSRCID",
#if RU_INCLUDE_DESC
    "",
    "bb source id of the other BBHs\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MULTIBBSRC_BBSRCID_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MULTIBBSRC_BBSRCID_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MULTIBBSRC_BBSRCID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MULTIBBSRC_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MULTIBBSRC_BBSRCID_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MULTIBBSRC *****/
const ru_reg_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MULTIBBSRC_REG =
{
    "COMMON_CONFIGURATIONS_MULTIBBSRC",
#if RU_INCLUDE_DESC
    "MULTI_BBH_BB_IDS 0..2 Register",
    "The BB IDs of the BBHs that are part of the 200/400 group\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MULTIBBSRC_REG_OFFSET },
    CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MULTIBBSRC_REG_RAM_CNT,
    4,
    617,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MULTIBBSRC_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ADDR_SPREADER, TYPE: Type_BBH_TX_BBHTX_COMMON_CONFIGURATIONS_ADDR_SPREADER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: NUM_ACTIVE_DRAM *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ADDR_SPREADER_NUM_ACTIVE_DRAM_FIELD =
{
    "NUM_ACTIVE_DRAM",
#if RU_INCLUDE_DESC
    "",
    "num_of_active_drams\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ADDR_SPREADER_NUM_ACTIVE_DRAM_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ADDR_SPREADER_NUM_ACTIVE_DRAM_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ADDR_SPREADER_NUM_ACTIVE_DRAM_FIELD_SHIFT },
    2,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EN_DDR *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ADDR_SPREADER_EN_DDR_FIELD =
{
    "EN_DDR",
#if RU_INCLUDE_DESC
    "",
    "enable_ddr\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ADDR_SPREADER_EN_DDR_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ADDR_SPREADER_EN_DDR_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ADDR_SPREADER_EN_DDR_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EN_EPSR *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ADDR_SPREADER_EN_EPSR_FIELD =
{
    "EN_EPSR",
#if RU_INCLUDE_DESC
    "",
    "enable_epsram\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ADDR_SPREADER_EN_EPSR_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ADDR_SPREADER_EN_EPSR_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ADDR_SPREADER_EN_EPSR_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SIMPLE_SPREAD *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ADDR_SPREADER_SIMPLE_SPREAD_FIELD =
{
    "SIMPLE_SPREAD",
#if RU_INCLUDE_DESC
    "",
    "simple_spread\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ADDR_SPREADER_SIMPLE_SPREAD_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ADDR_SPREADER_SIMPLE_SPREAD_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ADDR_SPREADER_SIMPLE_SPREAD_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ACTIVE_DRAM_0 *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ADDR_SPREADER_ACTIVE_DRAM_0_FIELD =
{
    "ACTIVE_DRAM_0",
#if RU_INCLUDE_DESC
    "",
    "active_dram_0\nin case only 2 drams are active we get their selection\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ADDR_SPREADER_ACTIVE_DRAM_0_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ADDR_SPREADER_ACTIVE_DRAM_0_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ADDR_SPREADER_ACTIVE_DRAM_0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ACTIVE_DRAM_1 *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ADDR_SPREADER_ACTIVE_DRAM_1_FIELD =
{
    "ACTIVE_DRAM_1",
#if RU_INCLUDE_DESC
    "",
    "active_dram_1\nin case only 2 drams are active we get their selection\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ADDR_SPREADER_ACTIVE_DRAM_1_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ADDR_SPREADER_ACTIVE_DRAM_1_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ADDR_SPREADER_ACTIVE_DRAM_1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BLOCK_SPREAD_ZONE_EN *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ADDR_SPREADER_BLOCK_SPREAD_ZONE_EN_FIELD =
{
    "BLOCK_SPREAD_ZONE_EN",
#if RU_INCLUDE_DESC
    "",
    "block_spread_zone_en\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ADDR_SPREADER_BLOCK_SPREAD_ZONE_EN_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ADDR_SPREADER_BLOCK_SPREAD_ZONE_EN_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ADDR_SPREADER_BLOCK_SPREAD_ZONE_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BLOCK_SPREAD_ZONE *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ADDR_SPREADER_BLOCK_SPREAD_ZONE_FIELD =
{
    "BLOCK_SPREAD_ZONE",
#if RU_INCLUDE_DESC
    "",
    "block_spread_zone\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ADDR_SPREADER_BLOCK_SPREAD_ZONE_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ADDR_SPREADER_BLOCK_SPREAD_ZONE_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ADDR_SPREADER_BLOCK_SPREAD_ZONE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SPARE *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ADDR_SPREADER_SPARE_FIELD =
{
    "SPARE",
#if RU_INCLUDE_DESC
    "",
    "spare\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ADDR_SPREADER_SPARE_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ADDR_SPREADER_SPARE_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ADDR_SPREADER_SPARE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ADDR_SPREADER_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ADDR_SPREADER_NUM_ACTIVE_DRAM_FIELD,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ADDR_SPREADER_EN_DDR_FIELD,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ADDR_SPREADER_EN_EPSR_FIELD,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ADDR_SPREADER_SIMPLE_SPREAD_FIELD,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ADDR_SPREADER_ACTIVE_DRAM_0_FIELD,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ADDR_SPREADER_ACTIVE_DRAM_1_FIELD,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ADDR_SPREADER_BLOCK_SPREAD_ZONE_EN_FIELD,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ADDR_SPREADER_BLOCK_SPREAD_ZONE_FIELD,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ADDR_SPREADER_SPARE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ADDR_SPREADER *****/
const ru_reg_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ADDR_SPREADER_REG =
{
    "COMMON_CONFIGURATIONS_ADDR_SPREADER",
#if RU_INCLUDE_DESC
    "ADDR_SPREADER Register",
    "UBUS Address spreading configuration\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ADDR_SPREADER_REG_OFFSET },
    0,
    0,
    618,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ADDR_SPREADER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_FLOWCTRCFG, TYPE: Type_BBH_TX_BBHTX_COMMON_CONFIGURATIONS_FLOWCTRCFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: NUMPORTS *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_FLOWCTRCFG_NUMPORTS_FIELD =
{
    "NUMPORTS",
#if RU_INCLUDE_DESC
    "",
    "the number of ports that share the pool of counters\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_FLOWCTRCFG_NUMPORTS_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_FLOWCTRCFG_NUMPORTS_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_FLOWCTRCFG_NUMPORTS_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: NUMCTRS *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_FLOWCTRCFG_NUMCTRS_FIELD =
{
    "NUMCTRS",
#if RU_INCLUDE_DESC
    "",
    "the size of the share of counters each port is using from the general pool\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_FLOWCTRCFG_NUMCTRS_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_FLOWCTRCFG_NUMCTRS_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_FLOWCTRCFG_NUMCTRS_FIELD_SHIFT },
    2,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MACFLOWCNTEN *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_FLOWCTRCFG_MACFLOWCNTEN_FIELD =
{
    "MACFLOWCNTEN",
#if RU_INCLUDE_DESC
    "",
    "enable counting according to mac flow field in the EH\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_FLOWCTRCFG_MACFLOWCNTEN_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_FLOWCTRCFG_MACFLOWCNTEN_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_FLOWCTRCFG_MACFLOWCNTEN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CTRIDXCNTEN *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_FLOWCTRCFG_CTRIDXCNTEN_FIELD =
{
    "CTRIDXCNTEN",
#if RU_INCLUDE_DESC
    "",
    "enable counting according to counter index field in the EH\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_FLOWCTRCFG_CTRIDXCNTEN_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_FLOWCTRCFG_CTRIDXCNTEN_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_FLOWCTRCFG_CTRIDXCNTEN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_FLOWCTRCFG_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_FLOWCTRCFG_NUMPORTS_FIELD,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_FLOWCTRCFG_NUMCTRS_FIELD,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_FLOWCTRCFG_MACFLOWCNTEN_FIELD,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_FLOWCTRCFG_CTRIDXCNTEN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_FLOWCTRCFG *****/
const ru_reg_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_FLOWCTRCFG_REG =
{
    "COMMON_CONFIGURATIONS_FLOWCTRCFG",
#if RU_INCLUDE_DESC
    "FLOW_COUNTER_CONFIG Register",
    "configurations related to the flow counters.\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_FLOWCTRCFG_REG_OFFSET },
    0,
    0,
    619,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_FLOWCTRCFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_FCRNRCFG, TYPE: Type_BBH_TX_BBHTX_COMMON_CONFIGURATIONS_FCRNRCFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: WKPADDR *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_FCRNRCFG_WKPADDR_FIELD =
{
    "WKPADDR",
#if RU_INCLUDE_DESC
    "",
    "the address in which to write the wakeup info\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_FCRNRCFG_WKPADDR_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_FCRNRCFG_WKPADDR_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_FCRNRCFG_WKPADDR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TASK *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_FCRNRCFG_TASK_FIELD =
{
    "TASK",
#if RU_INCLUDE_DESC
    "",
    "defines the tx task to wakeup\n\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_FCRNRCFG_TASK_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_FCRNRCFG_TASK_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_FCRNRCFG_TASK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BBSRCID *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_FCRNRCFG_BBSRCID_FIELD =
{
    "BBSRCID",
#if RU_INCLUDE_DESC
    "",
    "defines the bb source id of the TM core to wakeup\n\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_FCRNRCFG_BBSRCID_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_FCRNRCFG_BBSRCID_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_FCRNRCFG_BBSRCID_FIELD_SHIFT },
    64,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_FCRNRCFG_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_FCRNRCFG_WKPADDR_FIELD,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_FCRNRCFG_TASK_FIELD,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_FCRNRCFG_BBSRCID_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_FCRNRCFG *****/
const ru_reg_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_FCRNRCFG_REG =
{
    "COMMON_CONFIGURATIONS_FCRNRCFG",
#if RU_INCLUDE_DESC
    "FLOW_CONTROL_RNR_CFG 0..7 Register",
    "This register holds the configurations related to the TM core that is responsible for flow control management\n\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_FCRNRCFG_REG_OFFSET },
    CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_FCRNRCFG_REG_RAM_CNT,
    4,
    620,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_FCRNRCFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_FCCFG, TYPE: Type_BBH_TX_BBHTX_COMMON_CONFIGURATIONS_FCCFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: FC_STBLFILTER *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_FCCFG_FC_STBLFILTER_FIELD =
{
    "FC_STBLFILTER",
#if RU_INCLUDE_DESC
    "",
    "change stability filter - If there is a change in the flow control bits, for how long it should be stable for the BBH to send wake up the TM. This is used for the filtering frequent changes.\nValue is in clock cycles\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_FCCFG_FC_STBLFILTER_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_FCCFG_FC_STBLFILTER_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_FCCFG_FC_STBLFILTER_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FC_EN *****/
const ru_field_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_FCCFG_FC_EN_FIELD =
{
    "FC_EN",
#if RU_INCLUDE_DESC
    "",
    "when disabled, the BBH will not wakeup the TM\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_FCCFG_FC_EN_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_FCCFG_FC_EN_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_FCCFG_FC_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_FCCFG_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_FCCFG_FC_STBLFILTER_FIELD,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_FCCFG_FC_EN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_FCCFG *****/
const ru_reg_rec CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_FCCFG_REG =
{
    "COMMON_CONFIGURATIONS_FCCFG",
#if RU_INCLUDE_DESC
    "FLOW_CONTROL_CFG Register",
    "This register holds the configurations related to the flow control\n\n",
#endif
    { CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_FCCFG_REG_OFFSET },
    0,
    0,
    621,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_FCCFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PDACK, TYPE: Type_BBH_TX_BBHTX_PER_PORT_CONFIGURATIONS_PDACK
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: Q *****/
const ru_field_rec CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PDACK_Q_FIELD =
{
    "Q",
#if RU_INCLUDE_DESC
    "",
    "vector of a bit per q\n",
#endif
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PDACK_Q_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PDACK_Q_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PDACK_Q_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PDACK_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PDACK_Q_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PDACK *****/
const ru_reg_rec CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PDACK_REG =
{
    "PER_PORT_CONFIGURATIONS_PDACK",
#if RU_INCLUDE_DESC
    "PD_ACK Register",
    "option to move a BBH q to ACK state using configuration interface instead of sending ACK message.\nvector of bit per q. The HW will identify assertion of a bit as an ACK message.\n",
#endif
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PDACK_REG_OFFSET },
    0,
    0,
    622,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PDACK_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PDSIZE, TYPE: Type_BBH_TX_BBHTX_PER_PORT_CONFIGURATIONS_PDSIZE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: FIFOSIZE *****/
const ru_field_rec CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PDSIZE_FIFOSIZE_FIELD =
{
    "FIFOSIZE",
#if RU_INCLUDE_DESC
    "",
    "PD fifo size.\nNote that value of X means X+1.\n",
#endif
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PDSIZE_FIFOSIZE_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PDSIZE_FIFOSIZE_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PDSIZE_FIFOSIZE_FIELD_SHIFT },
    31,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PDSIZE_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PDSIZE_FIFOSIZE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PDSIZE *****/
const ru_reg_rec CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PDSIZE_REG =
{
    "PER_PORT_CONFIGURATIONS_PDSIZE",
#if RU_INCLUDE_DESC
    "PD_FIFO_SIZE 0..7 Register",
    "PD FIFO size.\nA total of 256 PDs are divided between all active ports.\nNote that configuration value of X means allocating X+1 entries in the FIFO.\n",
#endif
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PDSIZE_REG_OFFSET },
    CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PDSIZE_REG_RAM_CNT,
    4,
    623,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PDSIZE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PDWKUPH, TYPE: Type_BBH_TX_BBHTX_PER_PORT_CONFIGURATIONS_PDWKUPH
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: WKUPTHRESH *****/
const ru_field_rec CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PDWKUPH_WKUPTHRESH_FIELD =
{
    "WKUPTHRESH",
#if RU_INCLUDE_DESC
    "",
    "The wakeup threshold\n",
#endif
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PDWKUPH_WKUPTHRESH_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PDWKUPH_WKUPTHRESH_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PDWKUPH_WKUPTHRESH_FIELD_SHIFT },
    24,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PDWKUPH_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PDWKUPH_WKUPTHRESH_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PDWKUPH *****/
const ru_reg_rec CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PDWKUPH_REG =
{
    "PER_PORT_CONFIGURATIONS_PDWKUPH",
#if RU_INCLUDE_DESC
    "PD_WKUP_THRESH 0..7 Register",
    "When a FIFO occupancy is above this wakeup threshold, the BBH will not wake-up the Runner for sending a new PD. This threshold does not represent the actual size of the FIFO. If a PD will arrive from the Runner when the FIFO is above the threshold, it will not be dropped unless the FIFO is actually full.\n\n\n",
#endif
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PDWKUPH_REG_OFFSET },
    CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PDWKUPH_REG_RAM_CNT,
    4,
    624,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PDWKUPH_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PDEMPTY, TYPE: Type_BBH_TX_BBHTX_PER_PORT_CONFIGURATIONS_PDEMPTY
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: EMPTY *****/
const ru_field_rec CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PDEMPTY_EMPTY_FIELD =
{
    "EMPTY",
#if RU_INCLUDE_DESC
    "",
    "EPON PD FIFO empty threshold.\nA queue which its PD FIFO occupancy is below this threshold will have high priority in PD ordering arbitration.\n",
#endif
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PDEMPTY_EMPTY_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PDEMPTY_EMPTY_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PDEMPTY_EMPTY_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PDEMPTY_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PDEMPTY_EMPTY_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PDEMPTY *****/
const ru_reg_rec CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PDEMPTY_REG =
{
    "PER_PORT_CONFIGURATIONS_PDEMPTY",
#if RU_INCLUDE_DESC
    "PD_EMPTY_THRESHOLD 0..7 Register",
    "The BBH manages 32 queues for EPON (FIFO per LLID). For each queue it manages a PD FIFO. Usually, the BBH orders PDs from the Runner in RR between all queues. In EPON BBH, if a FIFO occupancy is below this threshold, the queue will have higher priority in PD ordering arbitration (with RR between all the empty queues).\nThis configuration is global for all queues.\nRelevant only for EPON BBH.\n",
#endif
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PDEMPTY_REG_OFFSET },
    CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PDEMPTY_REG_RAM_CNT,
    4,
    625,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PDEMPTY_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PD_BYTE_TH, TYPE: Type_BBH_TX_BBHTX_PER_PORT_CONFIGURATIONS_PD_BYTE_TH
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: BYTELIMIT *****/
const ru_field_rec CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PD_BYTE_TH_BYTELIMIT_FIELD =
{
    "BYTELIMIT",
#if RU_INCLUDE_DESC
    "",
    "Defines the number of bytes for PDs pre fetch limited according to the total number of bytes.\nThe value is in 8-bytes resolution.\n\n",
#endif
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PD_BYTE_TH_BYTELIMIT_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PD_BYTE_TH_BYTELIMIT_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PD_BYTE_TH_BYTELIMIT_FIELD_SHIFT },
    65535,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PD_BYTE_TH_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PD_BYTE_TH_BYTELIMIT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PD_BYTE_TH *****/
const ru_reg_rec CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PD_BYTE_TH_REG =
{
    "PER_PORT_CONFIGURATIONS_PD_BYTE_TH",
#if RU_INCLUDE_DESC
    "PD_BYTES_THRESHOLD 0..7 Register",
    "The BBH requests PDs from the Runner and maintains a pre-fetch PDs FIFO.\nThe PDs pre fetch is limited either by the PD FIFO configurable size or according to the total number of bytes (deducting bytes already requested/transmitted) for preventing HOL.\nWhen the runner is woken up by the BBH, it gets an indication from the BBH for the allowed amount of bytes the runner may send. This amount is calculated as follows:\n\nrnr_wakeup_msg.num_bytes = MIN( (byte_th_enabled ? (byte_th*8-num_bytes)  :  byte_th*8 ),0xffff) ;\n",
#endif
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PD_BYTE_TH_REG_OFFSET },
    CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PD_BYTE_TH_REG_RAM_CNT,
    4,
    626,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PD_BYTE_TH_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_MAX_PD_TH, TYPE: Type_BBH_TX_BBHTX_PER_PORT_CONFIGURATIONS_MAX_PD_TH
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PDLIMIT *****/
const ru_field_rec CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_MAX_PD_TH_PDLIMIT_FIELD =
{
    "PDLIMIT",
#if RU_INCLUDE_DESC
    "",
    "Defines the max number of PDs to pre fetch in a single order from the TM runner\n\n\n",
#endif
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_MAX_PD_TH_PDLIMIT_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_MAX_PD_TH_PDLIMIT_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_MAX_PD_TH_PDLIMIT_FIELD_SHIFT },
    8,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_MAX_PD_TH_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_MAX_PD_TH_PDLIMIT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_MAX_PD_TH *****/
const ru_reg_rec CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_MAX_PD_TH_REG =
{
    "PER_PORT_CONFIGURATIONS_MAX_PD_TH",
#if RU_INCLUDE_DESC
    "MAX_PD_THRESHOLD 0..7 Register",
    "The BBH requests PDs from the TM and maintains a pre-fetch PDs FIFO.\nThe PDs pre fetch is limited either by the PD FIFO configurable size or according to the total number of bytes (deducting bytes already requested/transmitted) for preventing HOL. For TM use, this register can limit the actual number of PDs ordered by the BBH TX in a single command.\n",
#endif
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_MAX_PD_TH_REG_OFFSET },
    CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_MAX_PD_TH_REG_RAM_CNT,
    4,
    627,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_MAX_PD_TH_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PD_BYTE_TH_EN, TYPE: Type_BBH_TX_BBHTX_PER_PORT_CONFIGURATIONS_PD_BYTE_TH_EN
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PDBYTELIMITEN *****/
const ru_field_rec CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PD_BYTE_TH_EN_PDBYTELIMITEN_FIELD =
{
    "PDBYTELIMITEN",
#if RU_INCLUDE_DESC
    "",
    "This 8 bit vector (bit per port) enables the above feature (PDs pre fetch limited according to the total number of bytes).\n",
#endif
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PD_BYTE_TH_EN_PDBYTELIMITEN_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PD_BYTE_TH_EN_PDBYTELIMITEN_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PD_BYTE_TH_EN_PDBYTELIMITEN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PDMAXLIMITEN *****/
const ru_field_rec CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PD_BYTE_TH_EN_PDMAXLIMITEN_FIELD =
{
    "PDMAXLIMITEN",
#if RU_INCLUDE_DESC
    "",
    "This bit vector (bit per port) enables the feature of PDs pre fetch limited according to max number of pds in a single batch.\n",
#endif
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PD_BYTE_TH_EN_PDMAXLIMITEN_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PD_BYTE_TH_EN_PDMAXLIMITEN_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PD_BYTE_TH_EN_PDMAXLIMITEN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DISFESF *****/
const ru_field_rec CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PD_BYTE_TH_EN_DISFESF_FIELD =
{
    "DISFESF",
#if RU_INCLUDE_DESC
    "",
    "Disable the store and forward mechanism in the FE buffer. When disable the BBH will transmit the data as soon as it gets to the FE buffer.\n",
#endif
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PD_BYTE_TH_EN_DISFESF_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PD_BYTE_TH_EN_DISFESF_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PD_BYTE_TH_EN_DISFESF_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PD_BYTE_TH_EN_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PD_BYTE_TH_EN_PDBYTELIMITEN_FIELD,
    &CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PD_BYTE_TH_EN_PDMAXLIMITEN_FIELD,
    &CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PD_BYTE_TH_EN_DISFESF_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PD_BYTE_TH_EN *****/
const ru_reg_rec CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PD_BYTE_TH_EN_REG =
{
    "PER_PORT_CONFIGURATIONS_PD_BYTE_TH_EN",
#if RU_INCLUDE_DESC
    "THRESHOLDS_EN Register",
    "PD byte threshold and PD max threshold. Used for PD request limitation.\n",
#endif
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PD_BYTE_TH_EN_REG_OFFSET },
    0,
    0,
    628,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PD_BYTE_TH_EN_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FLOWTOGEMOFFSET, TYPE: Type_BBH_TX_BBHTX_PER_PORT_CONFIGURATIONS_FLOWTOGEMOFFSET
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: OFFSET0 *****/
const ru_field_rec CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FLOWTOGEMOFFSET_OFFSET0_FIELD =
{
    "OFFSET0",
#if RU_INCLUDE_DESC
    "",
    "offset0\n",
#endif
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FLOWTOGEMOFFSET_OFFSET0_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FLOWTOGEMOFFSET_OFFSET0_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FLOWTOGEMOFFSET_OFFSET0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: OFFSET1 *****/
const ru_field_rec CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FLOWTOGEMOFFSET_OFFSET1_FIELD =
{
    "OFFSET1",
#if RU_INCLUDE_DESC
    "",
    "offset1\n",
#endif
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FLOWTOGEMOFFSET_OFFSET1_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FLOWTOGEMOFFSET_OFFSET1_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FLOWTOGEMOFFSET_OFFSET1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FLOWTOGEMOFFSET_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FLOWTOGEMOFFSET_OFFSET0_FIELD,
    &CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FLOWTOGEMOFFSET_OFFSET1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FLOWTOGEMOFFSET *****/
const ru_reg_rec CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FLOWTOGEMOFFSET_REG =
{
    "PER_PORT_CONFIGURATIONS_FLOWTOGEMOFFSET",
#if RU_INCLUDE_DESC
    "FLOW_INDEX_TO_GEM_OFFSETS 0..7 Register",
    "gem = index >= thr ? flow + offset1 : flow + offset 0\n",
#endif
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FLOWTOGEMOFFSET_REG_OFFSET },
    CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FLOWTOGEMOFFSET_REG_RAM_CNT,
    4,
    629,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FLOWTOGEMOFFSET_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FLOWTOGEMTHR, TYPE: Type_BBH_TX_BBHTX_PER_PORT_CONFIGURATIONS_FLOWTOGEMTHR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: THR *****/
const ru_field_rec CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FLOWTOGEMTHR_THR_FIELD =
{
    "THR",
#if RU_INCLUDE_DESC
    "",
    "threshold\n",
#endif
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FLOWTOGEMTHR_THR_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FLOWTOGEMTHR_THR_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FLOWTOGEMTHR_THR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FLOWTOGEMTHR_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FLOWTOGEMTHR_THR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FLOWTOGEMTHR *****/
const ru_reg_rec CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FLOWTOGEMTHR_REG =
{
    "PER_PORT_CONFIGURATIONS_FLOWTOGEMTHR",
#if RU_INCLUDE_DESC
    "FLOW_INDEX_TO_GEM_THR 0..7 Register",
    "gem = index > thr ? flow + offset1 : flow + offset 0\n",
#endif
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FLOWTOGEMTHR_REG_OFFSET },
    CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FLOWTOGEMTHR_REG_RAM_CNT,
    4,
    630,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FLOWTOGEMTHR_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FEBASE, TYPE: Type_BBH_TX_BBHTX_PER_PORT_CONFIGURATIONS_FEBASE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: FIFOBASE0 *****/
const ru_field_rec CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FEBASE_FIFOBASE0_FIELD =
{
    "FIFOBASE0",
#if RU_INCLUDE_DESC
    "",
    "The base of FE FIFO in 32 bytes resolution\n",
#endif
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FEBASE_FIFOBASE0_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FEBASE_FIFOBASE0_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FEBASE_FIFOBASE0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FEBASE_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FEBASE_FIFOBASE0_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FEBASE *****/
const ru_reg_rec CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FEBASE_REG =
{
    "PER_PORT_CONFIGURATIONS_FEBASE",
#if RU_INCLUDE_DESC
    "FE_FIFO_BASE 0..7 Register",
    "The BBH manages 8 queues. For each queue it manages a Front end buffer of data and PDs.\nThis registers array determine the front end data fifo base per BBH queue.\n\n",
#endif
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FEBASE_REG_OFFSET },
    CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FEBASE_REG_RAM_CNT,
    4,
    631,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FEBASE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FESIZE, TYPE: Type_BBH_TX_BBHTX_PER_PORT_CONFIGURATIONS_FESIZE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: FIFOSIZE *****/
const ru_field_rec CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FESIZE_FIFOSIZE_FIELD =
{
    "FIFOSIZE",
#if RU_INCLUDE_DESC
    "",
    "The size of PD FIFO in 32 bytes resolution\n",
#endif
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FESIZE_FIFOSIZE_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FESIZE_FIFOSIZE_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FESIZE_FIFOSIZE_FIELD_SHIFT },
    512,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FESIZE_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FESIZE_FIFOSIZE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FESIZE *****/
const ru_reg_rec CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FESIZE_REG =
{
    "PER_PORT_CONFIGURATIONS_FESIZE",
#if RU_INCLUDE_DESC
    "FE_FIFO_SIZE 0..7 Register",
    "The BBH manages 8 queues. For each queue it manages a Front end buffer of data and PDs.\nThis registers array determine the front end data fifo size per BBH queue.\n\n\n",
#endif
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FESIZE_REG_OFFSET },
    CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FESIZE_REG_RAM_CNT,
    4,
    632,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FESIZE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FEPDBASE, TYPE: Type_BBH_TX_BBHTX_PER_PORT_CONFIGURATIONS_FEPDBASE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: FIFOBASE0 *****/
const ru_field_rec CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FEPDBASE_FIFOBASE0_FIELD =
{
    "FIFOBASE0",
#if RU_INCLUDE_DESC
    "",
    "The base of FE PD FIFO\n",
#endif
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FEPDBASE_FIFOBASE0_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FEPDBASE_FIFOBASE0_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FEPDBASE_FIFOBASE0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FEPDBASE_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FEPDBASE_FIFOBASE0_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FEPDBASE *****/
const ru_reg_rec CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FEPDBASE_REG =
{
    "PER_PORT_CONFIGURATIONS_FEPDBASE",
#if RU_INCLUDE_DESC
    "FE_PD_FIFO_BASE 0..7 Register",
    "The BBH manages 8 queues. For each queue it manages a Front end buffer of data and PDs.\nThis registers array determine the front end PD fifo base per BBH queue.\n\n",
#endif
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FEPDBASE_REG_OFFSET },
    CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FEPDBASE_REG_RAM_CNT,
    4,
    633,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FEPDBASE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FEPDSIZE, TYPE: Type_BBH_TX_BBHTX_PER_PORT_CONFIGURATIONS_FEPDSIZE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: FIFOSIZE0 *****/
const ru_field_rec CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FEPDSIZE_FIFOSIZE0_FIELD =
{
    "FIFOSIZE0",
#if RU_INCLUDE_DESC
    "",
    "The size of FE PD FIFO\n",
#endif
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FEPDSIZE_FIFOSIZE0_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FEPDSIZE_FIFOSIZE0_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FEPDSIZE_FIFOSIZE0_FIELD_SHIFT },
    256,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FEPDSIZE_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FEPDSIZE_FIFOSIZE0_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FEPDSIZE *****/
const ru_reg_rec CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FEPDSIZE_REG =
{
    "PER_PORT_CONFIGURATIONS_FEPDSIZE",
#if RU_INCLUDE_DESC
    "FE_PD_FIFO_SIZE 0..7 Register",
    "The BBH manages 8 queues. For each queue it manages a Front end buffer of data and PDs.\nThis registers array determine the front end PD fifo base per BBH queue.\n\n",
#endif
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FEPDSIZE_REG_OFFSET },
    CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FEPDSIZE_REG_RAM_CNT,
    4,
    634,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FEPDSIZE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_SGMTWRR, TYPE: Type_BBH_TX_BBHTX_PER_PORT_CONFIGURATIONS_SGMTWRR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: W0 *****/
const ru_field_rec CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_SGMTWRR_W0_FIELD =
{
    "W0",
#if RU_INCLUDE_DESC
    "",
    "weight\n",
#endif
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_SGMTWRR_W0_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_SGMTWRR_W0_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_SGMTWRR_W0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_SGMTWRR_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_SGMTWRR_W0_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_SGMTWRR *****/
const ru_reg_rec CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_SGMTWRR_REG =
{
    "PER_PORT_CONFIGURATIONS_SGMTWRR",
#if RU_INCLUDE_DESC
    "SGMT_RR_WEIGHT 0..7 Register",
    "The unified BBH TX serves multiple MACs.\n\nThe TX arbitration between these MACs is WRR.\n\nThis register array determines the weight of each MAC.\n",
#endif
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_SGMTWRR_REG_OFFSET },
    CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_SGMTWRR_REG_RAM_CNT,
    4,
    635,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_SGMTWRR_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FE_CREDITS, TYPE: Type_BBH_TX_BBHTX_PER_PORT_CONFIGURATIONS_FE_CREDITS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: INIT *****/
const ru_field_rec CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FE_CREDITS_INIT_FIELD =
{
    "INIT",
#if RU_INCLUDE_DESC
    "",
    "initialization of the credits counter with the size of the FE FIFOS (pd and data)\nTRIGGERS_HW\n\n",
#endif
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FE_CREDITS_INIT_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FE_CREDITS_INIT_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FE_CREDITS_INIT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MIN_PD *****/
const ru_field_rec CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FE_CREDITS_MIN_PD_FIELD =
{
    "MIN_PD",
#if RU_INCLUDE_DESC
    "",
    "minimum number of credits for the FE PD FIFO in order to perform an ordering of chuck of data from the SRAM/DDR\nTRIGGERS_HW\n\n",
#endif
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FE_CREDITS_MIN_PD_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FE_CREDITS_MIN_PD_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FE_CREDITS_MIN_PD_FIELD_SHIFT },
    4,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MIN_DATA *****/
const ru_field_rec CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FE_CREDITS_MIN_DATA_FIELD =
{
    "MIN_DATA",
#if RU_INCLUDE_DESC
    "",
    "minimum number of credits for the FE TX FIFO in order to perform an ordering of chuck of data from the SRAM/DDR\nTRIGGERS_HW\n\n",
#endif
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FE_CREDITS_MIN_DATA_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FE_CREDITS_MIN_DATA_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FE_CREDITS_MIN_DATA_FIELD_SHIFT },
    17,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FE_CREDITS_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FE_CREDITS_INIT_FIELD,
    &CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FE_CREDITS_MIN_PD_FIELD,
    &CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FE_CREDITS_MIN_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FE_CREDITS *****/
const ru_reg_rec CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FE_CREDITS_REG =
{
    "PER_PORT_CONFIGURATIONS_FE_CREDITS",
#if RU_INCLUDE_DESC
    "FE_CREDITS Register",
    "between the FE and segmentation, the indication whether there is a free space in the FE FIFOs or not can be done with either buf_rdy indication as in GPON or with credits mechanism. This register configures the credits mechanism.\n",
#endif
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FE_CREDITS_REG_OFFSET },
    0,
    0,
    636,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FE_CREDITS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_GEM_EN_INIT, TYPE: Type_BBH_TX_BBHTX_PER_PORT_CONFIGURATIONS_GEM_EN_INIT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: INIT *****/
const ru_field_rec CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_GEM_EN_INIT_INIT_FIELD =
{
    "INIT",
#if RU_INCLUDE_DESC
    "",
    "init per port, each have 2b:\n00: reset value, init done.\n01: reset the port memory to value 0, meaning disable all gems (functional mode).\n10: reset the port memory to value 1, meaning enable all gems(verification mode).\n",
#endif
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_GEM_EN_INIT_INIT_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_GEM_EN_INIT_INIT_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_GEM_EN_INIT_INIT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_GEM_EN_INIT_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_GEM_EN_INIT_INIT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_GEM_EN_INIT *****/
const ru_reg_rec CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_GEM_EN_INIT_REG =
{
    "PER_PORT_CONFIGURATIONS_GEM_EN_INIT",
#if RU_INCLUDE_DESC
    "GEM_EN_TABLE_INIT Register",
    "init per port, each have 2b:\n00: reset value, init done.\n01: reset the port memory to value 0, meaning disable all gems (functional mode).\n10: reset the port memory to value 1, meaning enable all gems(verification mode).\n",
#endif
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_GEM_EN_INIT_REG_OFFSET },
    0,
    0,
    637,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_GEM_EN_INIT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_GEM_TRNS_INIT, TYPE: Type_BBH_TX_BBHTX_PER_PORT_CONFIGURATIONS_GEM_TRNS_INIT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: INIT *****/
const ru_field_rec CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_GEM_TRNS_INIT_INIT_FIELD =
{
    "INIT",
#if RU_INCLUDE_DESC
    "",
    "2b init for table:\n00: reset value, init done.\n01: reset the port memory to value 0, meaning disable all gems (functional mode).\n10: reset the port memory to value 1, meaning enable all gems(verification mode).\n",
#endif
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_GEM_TRNS_INIT_INIT_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_GEM_TRNS_INIT_INIT_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_GEM_TRNS_INIT_INIT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_GEM_TRNS_INIT_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_GEM_TRNS_INIT_INIT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_GEM_TRNS_INIT *****/
const ru_reg_rec CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_GEM_TRNS_INIT_REG =
{
    "PER_PORT_CONFIGURATIONS_GEM_TRNS_INIT",
#if RU_INCLUDE_DESC
    "GEM_TRANS_TABLE_INIT Register",
    "2b init for gem translation table(256 entries):\n00: reset value, init done.\n01: reset the port memory to value 0, meaning disable all gems (functional mode).\n10: reset the port memory to value 1, meaning enable all gems(verification mode).\n",
#endif
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_GEM_TRNS_INIT_REG_OFFSET },
    0,
    0,
    638,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_GEM_TRNS_INIT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_OTFBYTES, TYPE: Type_BBH_TX_BBHTX_PER_PORT_CONFIGURATIONS_OTFBYTES
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR *****/
const ru_field_rec CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_OTFBYTES_DDR_FIELD =
{
    "DDR",
#if RU_INCLUDE_DESC
    "",
    "OTF commands for DDR responders\n",
#endif
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_OTFBYTES_DDR_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_OTFBYTES_DDR_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_OTFBYTES_DDR_FIELD_SHIFT },
    64,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PSRAM0 *****/
const ru_field_rec CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_OTFBYTES_PSRAM0_FIELD =
{
    "PSRAM0",
#if RU_INCLUDE_DESC
    "",
    "OTF commands for PSRAM0 responders\n",
#endif
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_OTFBYTES_PSRAM0_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_OTFBYTES_PSRAM0_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_OTFBYTES_PSRAM0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PSRAM1 *****/
const ru_field_rec CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_OTFBYTES_PSRAM1_FIELD =
{
    "PSRAM1",
#if RU_INCLUDE_DESC
    "",
    "OTF commands for PSRAM1 responders\n",
#endif
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_OTFBYTES_PSRAM1_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_OTFBYTES_PSRAM1_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_OTFBYTES_PSRAM1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_OTFBYTES_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_OTFBYTES_DDR_FIELD,
    &CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_OTFBYTES_PSRAM0_FIELD,
    &CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_OTFBYTES_PSRAM1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_OTFBYTES *****/
const ru_reg_rec CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_OTFBYTES_REG =
{
    "PER_PORT_CONFIGURATIONS_OTFBYTES",
#if RU_INCLUDE_DESC
    "TARGET_MEM_MAX_OTF_BYTES 0..7 Register",
    "target mems are DDR, PSRAM0 and PSRAM1\n",
#endif
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_OTFBYTES_REG_OFFSET },
    CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_OTFBYTES_REG_RAM_CNT,
    4,
    639,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_OTFBYTES_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_OTFCMD, TYPE: Type_BBH_TX_BBHTX_PER_PORT_CONFIGURATIONS_OTFCMD
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR *****/
const ru_field_rec CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_OTFCMD_DDR_FIELD =
{
    "DDR",
#if RU_INCLUDE_DESC
    "",
    "OTF commands for DDR responders\n",
#endif
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_OTFCMD_DDR_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_OTFCMD_DDR_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_OTFCMD_DDR_FIELD_SHIFT },
    32,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PSRAM0 *****/
const ru_field_rec CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_OTFCMD_PSRAM0_FIELD =
{
    "PSRAM0",
#if RU_INCLUDE_DESC
    "",
    "OTF commands for PSRAM0 responders\n",
#endif
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_OTFCMD_PSRAM0_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_OTFCMD_PSRAM0_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_OTFCMD_PSRAM0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PSRAM1 *****/
const ru_field_rec CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_OTFCMD_PSRAM1_FIELD =
{
    "PSRAM1",
#if RU_INCLUDE_DESC
    "",
    "OTF commands for PSRAM1 responders\n",
#endif
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_OTFCMD_PSRAM1_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_OTFCMD_PSRAM1_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_OTFCMD_PSRAM1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_OTFCMD_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_OTFCMD_DDR_FIELD,
    &CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_OTFCMD_PSRAM0_FIELD,
    &CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_OTFCMD_PSRAM1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_OTFCMD *****/
const ru_reg_rec CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_OTFCMD_REG =
{
    "PER_PORT_CONFIGURATIONS_OTFCMD",
#if RU_INCLUDE_DESC
    "TARGET_MEM_MAX_OTF_CMDS 0..7 Register",
    "target mems are DDR, PSRAM0 and PSRAM1\n",
#endif
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_OTFCMD_REG_OFFSET },
    CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_OTFCMD_REG_RAM_CNT,
    4,
    640,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_OTFCMD_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_AUTOOTFCREDITS, TYPE: Type_BBH_TX_BBHTX_PER_PORT_CONFIGURATIONS_AUTOOTFCREDITS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: NUMPORTS *****/
const ru_field_rec CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_AUTOOTFCREDITS_NUMPORTS_FIELD =
{
    "NUMPORTS",
#if RU_INCLUDE_DESC
    "",
    "number of active ports\n",
#endif
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_AUTOOTFCREDITS_NUMPORTS_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_AUTOOTFCREDITS_NUMPORTS_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_AUTOOTFCREDITS_NUMPORTS_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_AUTOOTFCREDITS_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_AUTOOTFCREDITS_NUMPORTS_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_AUTOOTFCREDITS *****/
const ru_reg_rec CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_AUTOOTFCREDITS_REG =
{
    "PER_PORT_CONFIGURATIONS_AUTOOTFCREDITS",
#if RU_INCLUDE_DESC
    "TARGET_MEM_AUTO_CREDITS_DIVIDE Register",
    "when enabled, the BBH will divide the target mem credits automatically according to number of active ports. The default configuration includes limiting only the DDR traffic.\n",
#endif
    { CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_AUTOOTFCREDITS_REG_OFFSET },
    0,
    0,
    641,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_AUTOOTFCREDITS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_TXSRAM0PD, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_TXSRAM0PD
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: SRAMPD *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_TXSRAM0PD_SRAMPD_FIELD =
{
    "SRAMPD",
#if RU_INCLUDE_DESC
    "",
    "This counter counts the number of packets which were transmitted from the SRAM.\nCHANGED_BY_HW\n\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_TXSRAM0PD_SRAMPD_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_TXSRAM0PD_SRAMPD_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_TXSRAM0PD_SRAMPD_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_DEBUG_COUNTERS_TXSRAM0PD_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_TXSRAM0PD_SRAMPD_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_TXSRAM0PD *****/
const ru_reg_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_TXSRAM0PD_REG =
{
    "DEBUG_COUNTERS_TXSRAM0PD",
#if RU_INCLUDE_DESC
    "TX_SRAM0_PD_COUNTER Register",
    "This counter counts the number of received PD for packets to be transmitted from the SRAM0.\nIt counts the packets for all queues together.\nThis counter is cleared when read and freezes when maximum value is reached.\n\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_TXSRAM0PD_REG_OFFSET },
    0,
    0,
    642,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_EGR_BBH_TX_DEBUG_COUNTERS_TXSRAM0PD_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_TXSRAM1PD, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_TXSRAM1PD
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: SRAMPD *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_TXSRAM1PD_SRAMPD_FIELD =
{
    "SRAMPD",
#if RU_INCLUDE_DESC
    "",
    "This counter counts the number of packets which were transmitted from the SRAM.\nCHANGED_BY_HW\n\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_TXSRAM1PD_SRAMPD_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_TXSRAM1PD_SRAMPD_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_TXSRAM1PD_SRAMPD_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_DEBUG_COUNTERS_TXSRAM1PD_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_TXSRAM1PD_SRAMPD_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_TXSRAM1PD *****/
const ru_reg_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_TXSRAM1PD_REG =
{
    "DEBUG_COUNTERS_TXSRAM1PD",
#if RU_INCLUDE_DESC
    "TX_SRAM1_PD_COUNTER Register",
    "This counter counts the number of received PD for packets to be transmitted from the SRAM0.\nIt counts the packets for all queues together.\nThis counter is cleared when read and freezes when maximum value is reached.\n\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_TXSRAM1PD_REG_OFFSET },
    0,
    0,
    643,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_EGR_BBH_TX_DEBUG_COUNTERS_TXSRAM1PD_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_TXDDRPD, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_TXDDRPD
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DDRPD *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_TXDDRPD_DDRPD_FIELD =
{
    "DDRPD",
#if RU_INCLUDE_DESC
    "",
    "This counter counts the number of packets which were transmitted from the DDR.\nCHANGED_BY_HW\n\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_TXDDRPD_DDRPD_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_TXDDRPD_DDRPD_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_TXDDRPD_DDRPD_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_DEBUG_COUNTERS_TXDDRPD_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_TXDDRPD_DDRPD_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_TXDDRPD *****/
const ru_reg_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_TXDDRPD_REG =
{
    "DEBUG_COUNTERS_TXDDRPD",
#if RU_INCLUDE_DESC
    "TX_DDR_PD_COUNTER Register",
    "This counter counts the number of received PDs for packets to be transmitted from the DDR.\nIt counts the packets for all queues together.\nThis counter is cleared when read and freezes when maximum value is reached.\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_TXDDRPD_REG_OFFSET },
    0,
    0,
    644,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_EGR_BBH_TX_DEBUG_COUNTERS_TXDDRPD_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_PDDROP, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_PDDROP
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PDDROP *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_PDDROP_PDDROP_FIELD =
{
    "PDDROP",
#if RU_INCLUDE_DESC
    "",
    "This counter counts the number of PDs which were dropped due to PD FIFO full.\nCHANGED_BY_HW\n\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_PDDROP_PDDROP_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_PDDROP_PDDROP_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_PDDROP_PDDROP_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_DEBUG_COUNTERS_PDDROP_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_PDDROP_PDDROP_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_PDDROP *****/
const ru_reg_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_PDDROP_REG =
{
    "DEBUG_COUNTERS_PDDROP",
#if RU_INCLUDE_DESC
    "PD_DROP_COUNTER Register",
    "This counter counts the number of PDs which were dropped due to PD FIFO full.\nIt counts the packets for all queues together.\nThis counter is cleared when read and freezes when maximum value is reached.\n\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_PDDROP_REG_OFFSET },
    0,
    0,
    645,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_EGR_BBH_TX_DEBUG_COUNTERS_PDDROP_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_GETNEXTNULL, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_GETNEXTNULL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: GETNEXTNULL *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_GETNEXTNULL_GETNEXTNULL_FIELD =
{
    "GETNEXTNULL",
#if RU_INCLUDE_DESC
    "",
    "This counter counts the number Get next responses with a null BN.\nCHANGED_BY_HW\n\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_GETNEXTNULL_GETNEXTNULL_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_GETNEXTNULL_GETNEXTNULL_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_GETNEXTNULL_GETNEXTNULL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_DEBUG_COUNTERS_GETNEXTNULL_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_GETNEXTNULL_GETNEXTNULL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_GETNEXTNULL *****/
const ru_reg_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_GETNEXTNULL_REG =
{
    "DEBUG_COUNTERS_GETNEXTNULL",
#if RU_INCLUDE_DESC
    "GET_NEXT_IS_NULL_COUNTER Register",
    "This counter counts the number Get next responses with a null BN.\nIt counts the packets for all TCONTs together.\nThis counter is cleared when read and freezes when maximum value is reached.\nThis counter is relevant for Ethernet only.\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_GETNEXTNULL_REG_OFFSET },
    0,
    0,
    646,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_EGR_BBH_TX_DEBUG_COUNTERS_GETNEXTNULL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_GETNEXTDROP, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_GETNEXTDROP
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: GETNEXTDROP *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_GETNEXTDROP_GETNEXTDROP_FIELD =
{
    "GETNEXTDROP",
#if RU_INCLUDE_DESC
    "",
    "This counter counts the number Get next responses that are dropped.\nCHANGED_BY_HW\n\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_GETNEXTDROP_GETNEXTDROP_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_GETNEXTDROP_GETNEXTDROP_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_GETNEXTDROP_GETNEXTDROP_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_DEBUG_COUNTERS_GETNEXTDROP_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_GETNEXTDROP_GETNEXTDROP_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_GETNEXTDROP *****/
const ru_reg_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_GETNEXTDROP_REG =
{
    "DEBUG_COUNTERS_GETNEXTDROP",
#if RU_INCLUDE_DESC
    "GET_NEXT_DROP_COUNTER Register",
    "This counter counts the number Get next responses that are dropped due to fifo full.\n\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_GETNEXTDROP_REG_OFFSET },
    0,
    0,
    647,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_EGR_BBH_TX_DEBUG_COUNTERS_GETNEXTDROP_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_INPUTFIFODROP, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_INPUTFIFODROP
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: INPUTFIFODROP *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_INPUTFIFODROP_INPUTFIFODROP_FIELD =
{
    "INPUTFIFODROP",
#if RU_INCLUDE_DESC
    "",
    "This counter counts the number of PDs which were dropped due to input PD FIFO full.\nCHANGED_BY_HW\n\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_INPUTFIFODROP_INPUTFIFODROP_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_INPUTFIFODROP_INPUTFIFODROP_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_INPUTFIFODROP_INPUTFIFODROP_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_DEBUG_COUNTERS_INPUTFIFODROP_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_INPUTFIFODROP_INPUTFIFODROP_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_INPUTFIFODROP *****/
const ru_reg_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_INPUTFIFODROP_REG =
{
    "DEBUG_COUNTERS_INPUTFIFODROP",
#if RU_INCLUDE_DESC
    "INPUT_PD_DROP_COUNTER Register",
    "This counter counts the number of PDs which were dropped due to input PD FIFO full.\nIt counts the packets for all queues together.\nThis counter is cleared when read and freezes when maximum value is reached.\n\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_INPUTFIFODROP_REG_OFFSET },
    0,
    0,
    648,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_EGR_BBH_TX_DEBUG_COUNTERS_INPUTFIFODROP_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SRAM0PKT, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_SRAM0PKT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: SRAMPKT *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SRAM0PKT_SRAMPKT_FIELD =
{
    "SRAMPKT",
#if RU_INCLUDE_DESC
    "",
    "This counter counts the number of packets which were transmitted from the SRAM.\nCHANGED_BY_HW\n\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SRAM0PKT_SRAMPKT_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SRAM0PKT_SRAMPKT_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SRAM0PKT_SRAMPKT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SRAM0PKT_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SRAM0PKT_SRAMPKT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SRAM0PKT *****/
const ru_reg_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SRAM0PKT_REG =
{
    "DEBUG_COUNTERS_SRAM0PKT",
#if RU_INCLUDE_DESC
    "SRAM0_PKT_COUNTER Register",
    "This counter counts the number of received packets to be transmitted from the SRAM.\nIt counts the packets for all queues together.\nThis counter is cleared when read and freezes when maximum value is reached.\n\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SRAM0PKT_REG_OFFSET },
    0,
    0,
    649,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SRAM0PKT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SRAM1PKT, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_SRAM1PKT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: SRAMPKT *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SRAM1PKT_SRAMPKT_FIELD =
{
    "SRAMPKT",
#if RU_INCLUDE_DESC
    "",
    "This counter counts the number of packets which were transmitted from the SRAM.\nCHANGED_BY_HW\n\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SRAM1PKT_SRAMPKT_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SRAM1PKT_SRAMPKT_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SRAM1PKT_SRAMPKT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SRAM1PKT_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SRAM1PKT_SRAMPKT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SRAM1PKT *****/
const ru_reg_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SRAM1PKT_REG =
{
    "DEBUG_COUNTERS_SRAM1PKT",
#if RU_INCLUDE_DESC
    "SRAM1_PKT_COUNTER Register",
    "This counter counts the number of received packets to be transmitted from the SRAM.\nIt counts the packets for all queues together.\nThis counter is cleared when read and freezes when maximum value is reached.\n\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SRAM1PKT_REG_OFFSET },
    0,
    0,
    650,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SRAM1PKT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_DDRPKT, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_DDRPKT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DDRPKT *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_DDRPKT_DDRPKT_FIELD =
{
    "DDRPKT",
#if RU_INCLUDE_DESC
    "",
    "This counter counts the number of packets which were transmitted from the DDR.\nCHANGED_BY_HW\n\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_DDRPKT_DDRPKT_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_DDRPKT_DDRPKT_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_DDRPKT_DDRPKT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_DEBUG_COUNTERS_DDRPKT_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_DDRPKT_DDRPKT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_DDRPKT *****/
const ru_reg_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_DDRPKT_REG =
{
    "DEBUG_COUNTERS_DDRPKT",
#if RU_INCLUDE_DESC
    "DDR_PKT_COUNTER Register",
    "This counter counts the number of received packets to be transmitted from the DDR.\nIt counts the packets for all queues together.\nThis counter is cleared when read and freezes when maximum value is reached.\n\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_DDRPKT_REG_OFFSET },
    0,
    0,
    651,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_EGR_BBH_TX_DEBUG_COUNTERS_DDRPKT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_TXSRAM0BYTE, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_TXSRAM0BYTE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: SRAMBYTE *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_TXSRAM0BYTE_SRAMBYTE_FIELD =
{
    "SRAMBYTE",
#if RU_INCLUDE_DESC
    "",
    "This counter counts the number of transmitted bytes from the SRAM.\nCHANGED_BY_HW\n\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_TXSRAM0BYTE_SRAMBYTE_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_TXSRAM0BYTE_SRAMBYTE_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_TXSRAM0BYTE_SRAMBYTE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_DEBUG_COUNTERS_TXSRAM0BYTE_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_TXSRAM0BYTE_SRAMBYTE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_TXSRAM0BYTE *****/
const ru_reg_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_TXSRAM0BYTE_REG =
{
    "DEBUG_COUNTERS_TXSRAM0BYTE",
#if RU_INCLUDE_DESC
    "TX_SRAM0_BYTE_COUNTER Register",
    "This counter counts the number of transmitted bytes from the SRAM.\nIt counts the bytes for all queues together.\nThis counter is cleared when read and freezes when maximum value is reached.\n\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_TXSRAM0BYTE_REG_OFFSET },
    0,
    0,
    652,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_EGR_BBH_TX_DEBUG_COUNTERS_TXSRAM0BYTE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_TXSRAM1BYTE, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_TXSRAM1BYTE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: SRAMBYTE *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_TXSRAM1BYTE_SRAMBYTE_FIELD =
{
    "SRAMBYTE",
#if RU_INCLUDE_DESC
    "",
    "This counter counts the number of transmitted bytes from the SRAM.\nCHANGED_BY_HW\n\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_TXSRAM1BYTE_SRAMBYTE_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_TXSRAM1BYTE_SRAMBYTE_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_TXSRAM1BYTE_SRAMBYTE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_DEBUG_COUNTERS_TXSRAM1BYTE_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_TXSRAM1BYTE_SRAMBYTE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_TXSRAM1BYTE *****/
const ru_reg_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_TXSRAM1BYTE_REG =
{
    "DEBUG_COUNTERS_TXSRAM1BYTE",
#if RU_INCLUDE_DESC
    "TX_SRAM1_BYTE_COUNTER Register",
    "This counter counts the number of transmitted bytes from the SRAM.\nIt counts the bytes for all queues together.\nThis counter is cleared when read and freezes when maximum value is reached.\n\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_TXSRAM1BYTE_REG_OFFSET },
    0,
    0,
    653,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_EGR_BBH_TX_DEBUG_COUNTERS_TXSRAM1BYTE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_TXDDRBYTE, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_TXDDRBYTE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DDRBYTE *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_TXDDRBYTE_DDRBYTE_FIELD =
{
    "DDRBYTE",
#if RU_INCLUDE_DESC
    "",
    "This counter counts the number of transmitted bytes from the DDr.\nCHANGED_BY_HW\n\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_TXDDRBYTE_DDRBYTE_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_TXDDRBYTE_DDRBYTE_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_TXDDRBYTE_DDRBYTE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_DEBUG_COUNTERS_TXDDRBYTE_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_TXDDRBYTE_DDRBYTE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_TXDDRBYTE *****/
const ru_reg_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_TXDDRBYTE_REG =
{
    "DEBUG_COUNTERS_TXDDRBYTE",
#if RU_INCLUDE_DESC
    "TX_DDR_BYTE_COUNTER Register",
    "This counter counts the number of transmitted bytes from the DDR.\nIt counts the bytes for all queues together.\nThis counter is cleared when read and freezes when maximum value is reached.\n\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_TXDDRBYTE_REG_OFFSET },
    0,
    0,
    654,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_EGR_BBH_TX_DEBUG_COUNTERS_TXDDRBYTE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_SWRDEN
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PDSEL *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_PDSEL_FIELD =
{
    "PDSEL",
#if RU_INCLUDE_DESC
    "",
    "rd from the PD FIFO\nTRIGGERS_HW\n\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_PDSEL_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_PDSEL_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_PDSEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PDVSEL *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_PDVSEL_FIELD =
{
    "PDVSEL",
#if RU_INCLUDE_DESC
    "",
    "rd from the PD valid array\nTRIGGERS_HW\n\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_PDVSEL_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_PDVSEL_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_PDVSEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PDEMPTYSEL *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_PDEMPTYSEL_FIELD =
{
    "PDEMPTYSEL",
#if RU_INCLUDE_DESC
    "",
    "rd from the PD empty array\nTRIGGERS_HW\n\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_PDEMPTYSEL_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_PDEMPTYSEL_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_PDEMPTYSEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PDFULLSEL *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_PDFULLSEL_FIELD =
{
    "PDFULLSEL",
#if RU_INCLUDE_DESC
    "",
    "rd from the PD Full array\nTRIGGERS_HW\n\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_PDFULLSEL_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_PDFULLSEL_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_PDFULLSEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PDBEMPTYSEL *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_PDBEMPTYSEL_FIELD =
{
    "PDBEMPTYSEL",
#if RU_INCLUDE_DESC
    "",
    "rd from the PD beliow empty array\nTRIGGERS_HW\n\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_PDBEMPTYSEL_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_PDBEMPTYSEL_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_PDBEMPTYSEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PDFFWKPSEL *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_PDFFWKPSEL_FIELD =
{
    "PDFFWKPSEL",
#if RU_INCLUDE_DESC
    "",
    "rd from the PD full for wakeup empty array\nTRIGGERS_HW\n\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_PDFFWKPSEL_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_PDFFWKPSEL_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_PDFFWKPSEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FBN_0_SEL *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_FBN_0_SEL_FIELD =
{
    "FBN_0_SEL",
#if RU_INCLUDE_DESC
    "",
    "rd from the first BN 0 array\nTRIGGERS_HW\n\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_FBN_0_SEL_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_FBN_0_SEL_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_FBN_0_SEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FBNV_0_SEL *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_FBNV_0_SEL_FIELD =
{
    "FBNV_0_SEL",
#if RU_INCLUDE_DESC
    "",
    "rd from the first BN 0 valid array\nTRIGGERS_HW\n\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_FBNV_0_SEL_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_FBNV_0_SEL_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_FBNV_0_SEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FBNEMPTY_0_SEL *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_FBNEMPTY_0_SEL_FIELD =
{
    "FBNEMPTY_0_SEL",
#if RU_INCLUDE_DESC
    "",
    "rd from the first BN 0 empty array\nTRIGGERS_HW\n\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_FBNEMPTY_0_SEL_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_FBNEMPTY_0_SEL_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_FBNEMPTY_0_SEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FBNFULL_0_SEL *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_FBNFULL_0_SEL_FIELD =
{
    "FBNFULL_0_SEL",
#if RU_INCLUDE_DESC
    "",
    "rd from the first BN 0 full array\nTRIGGERS_HW\n\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_FBNFULL_0_SEL_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_FBNFULL_0_SEL_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_FBNFULL_0_SEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: GETNEXT_0_SEL *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_GETNEXT_0_SEL_FIELD =
{
    "GETNEXT_0_SEL",
#if RU_INCLUDE_DESC
    "",
    "rd from the first Get Next 0 array\nTRIGGERS_HW\n\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_GETNEXT_0_SEL_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_GETNEXT_0_SEL_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_GETNEXT_0_SEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: GETNEXTV_0_SEL *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_GETNEXTV_0_SEL_FIELD =
{
    "GETNEXTV_0_SEL",
#if RU_INCLUDE_DESC
    "",
    "rd from the get_next valid 0 array\nTRIGGERS_HW\n\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_GETNEXTV_0_SEL_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_GETNEXTV_0_SEL_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_GETNEXTV_0_SEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: GETNEXTEMPTY_0_SEL *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_GETNEXTEMPTY_0_SEL_FIELD =
{
    "GETNEXTEMPTY_0_SEL",
#if RU_INCLUDE_DESC
    "",
    "rd from the get next 0 empty array\nTRIGGERS_HW\n\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_GETNEXTEMPTY_0_SEL_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_GETNEXTEMPTY_0_SEL_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_GETNEXTEMPTY_0_SEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: GETNEXTFULL_0_SEL *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_GETNEXTFULL_0_SEL_FIELD =
{
    "GETNEXTFULL_0_SEL",
#if RU_INCLUDE_DESC
    "",
    "rd from the get next 0 full array\nTRIGGERS_HW\n\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_GETNEXTFULL_0_SEL_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_GETNEXTFULL_0_SEL_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_GETNEXTFULL_0_SEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FPM_0_SEL *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_FPM_0_SEL_FIELD =
{
    "FPM_0_SEL",
#if RU_INCLUDE_DESC
    "",
    "rd from the FPM FIFO\nTRIGGERS_HW\n\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_FPM_0_SEL_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_FPM_0_SEL_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_FPM_0_SEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SBPM_0_SEL *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_SBPM_0_SEL_FIELD =
{
    "SBPM_0_SEL",
#if RU_INCLUDE_DESC
    "",
    "rd from the SBPM 0 FIFO\nTRIGGERS_HW\n\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_SBPM_0_SEL_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_SBPM_0_SEL_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_SBPM_0_SEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FBNV_1_SEL *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_FBNV_1_SEL_FIELD =
{
    "FBNV_1_SEL",
#if RU_INCLUDE_DESC
    "",
    "rd from the first BN 1 valid array\nTRIGGERS_HW\n\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_FBNV_1_SEL_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_FBNV_1_SEL_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_FBNV_1_SEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FBNEMPTY_1_SEL *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_FBNEMPTY_1_SEL_FIELD =
{
    "FBNEMPTY_1_SEL",
#if RU_INCLUDE_DESC
    "",
    "rd from the first BN 1 empty array\nTRIGGERS_HW\n\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_FBNEMPTY_1_SEL_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_FBNEMPTY_1_SEL_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_FBNEMPTY_1_SEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FBNFULL_1_SEL *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_FBNFULL_1_SEL_FIELD =
{
    "FBNFULL_1_SEL",
#if RU_INCLUDE_DESC
    "",
    "rd from the first BN 1 full array\nTRIGGERS_HW\n\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_FBNFULL_1_SEL_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_FBNFULL_1_SEL_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_FBNFULL_1_SEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: GETNEXT_1_SEL *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_GETNEXT_1_SEL_FIELD =
{
    "GETNEXT_1_SEL",
#if RU_INCLUDE_DESC
    "",
    "rd from the first Get Next 1 array\nTRIGGERS_HW\n\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_GETNEXT_1_SEL_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_GETNEXT_1_SEL_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_GETNEXT_1_SEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: GETNEXTV_1_SEL *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_GETNEXTV_1_SEL_FIELD =
{
    "GETNEXTV_1_SEL",
#if RU_INCLUDE_DESC
    "",
    "rd from the get_next valid 1 array\nTRIGGERS_HW\n\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_GETNEXTV_1_SEL_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_GETNEXTV_1_SEL_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_GETNEXTV_1_SEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: GETNEXTEMPTY_1_SEL *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_GETNEXTEMPTY_1_SEL_FIELD =
{
    "GETNEXTEMPTY_1_SEL",
#if RU_INCLUDE_DESC
    "",
    "rd from the get next 1 empty array\nTRIGGERS_HW\n\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_GETNEXTEMPTY_1_SEL_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_GETNEXTEMPTY_1_SEL_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_GETNEXTEMPTY_1_SEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: GETNEXTFULL_1_SEL *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_GETNEXTFULL_1_SEL_FIELD =
{
    "GETNEXTFULL_1_SEL",
#if RU_INCLUDE_DESC
    "",
    "rd from the get next 1 full array\nTRIGGERS_HW\n\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_GETNEXTFULL_1_SEL_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_GETNEXTFULL_1_SEL_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_GETNEXTFULL_1_SEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FPM_1_SEL *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_FPM_1_SEL_FIELD =
{
    "FPM_1_SEL",
#if RU_INCLUDE_DESC
    "",
    "rd from the FPM 1 FIFO\nTRIGGERS_HW\n\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_FPM_1_SEL_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_FPM_1_SEL_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_FPM_1_SEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SBPM_1_SEL *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_SBPM_1_SEL_FIELD =
{
    "SBPM_1_SEL",
#if RU_INCLUDE_DESC
    "",
    "rd from the SBPM 1 FIFO\nTRIGGERS_HW\n\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_SBPM_1_SEL_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_SBPM_1_SEL_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_SBPM_1_SEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FBN_1_SEL *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_FBN_1_SEL_FIELD =
{
    "FBN_1_SEL",
#if RU_INCLUDE_DESC
    "",
    "first bn 1 array\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_FBN_1_SEL_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_FBN_1_SEL_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_FBN_1_SEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_PDSEL_FIELD,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_PDVSEL_FIELD,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_PDEMPTYSEL_FIELD,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_PDFULLSEL_FIELD,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_PDBEMPTYSEL_FIELD,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_PDFFWKPSEL_FIELD,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_FBN_0_SEL_FIELD,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_FBNV_0_SEL_FIELD,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_FBNEMPTY_0_SEL_FIELD,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_FBNFULL_0_SEL_FIELD,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_GETNEXT_0_SEL_FIELD,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_GETNEXTV_0_SEL_FIELD,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_GETNEXTEMPTY_0_SEL_FIELD,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_GETNEXTFULL_0_SEL_FIELD,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_FPM_0_SEL_FIELD,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_SBPM_0_SEL_FIELD,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_FBNV_1_SEL_FIELD,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_FBNEMPTY_1_SEL_FIELD,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_FBNFULL_1_SEL_FIELD,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_GETNEXT_1_SEL_FIELD,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_GETNEXTV_1_SEL_FIELD,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_GETNEXTEMPTY_1_SEL_FIELD,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_GETNEXTFULL_1_SEL_FIELD,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_FPM_1_SEL_FIELD,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_SBPM_1_SEL_FIELD,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_FBN_1_SEL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN *****/
const ru_reg_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_REG =
{
    "DEBUG_COUNTERS_SWRDEN",
#if RU_INCLUDE_DESC
    "SW_RD_EN Register",
    "writing to this register creates a rd_en pulse to the selected array the SW wants to access.\n\nEach bit in the register represents one of the arrays the SW can access.\n\nThe address inside the array is determined in the previous register (sw_rd_address).\n\nWhen writing to this register the SW should assert only one bit. If more than one is asserted, The HW will return the value read from the lsb selected array.\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_REG_OFFSET },
    0,
    0,
    655,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    26,
    CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDADDR, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_SWRDADDR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RDADDR *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDADDR_RDADDR_FIELD =
{
    "RDADDR",
#if RU_INCLUDE_DESC
    "",
    "The address inside the array the sw wants to read\nCHANGED_BY_HW\n\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDADDR_RDADDR_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDADDR_RDADDR_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDADDR_RDADDR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDADDR_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDADDR_RDADDR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDADDR *****/
const ru_reg_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDADDR_REG =
{
    "DEBUG_COUNTERS_SWRDADDR",
#if RU_INCLUDE_DESC
    "SW_RD_ADDR Register",
    "the address inside the array the SW wants to read\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDADDR_REG_OFFSET },
    0,
    0,
    656,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDADDR_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDDATA, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_SWRDDATA
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDDATA_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "data\nCHANGED_BY_HW\n\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDDATA_DATA_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDDATA_DATA_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDDATA_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDDATA_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDDATA_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDDATA *****/
const ru_reg_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDDATA_REG =
{
    "DEBUG_COUNTERS_SWRDDATA",
#if RU_INCLUDE_DESC
    "SW_RD_DATA Register",
    "indirect memories and arrays read data\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDDATA_REG_OFFSET },
    0,
    0,
    657,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDDATA_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_UNIFIEDPKT, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_UNIFIEDPKT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: UNIFIED_PKT *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_UNIFIEDPKT_UNIFIED_PKT_FIELD =
{
    "UNIFIED_PKT",
#if RU_INCLUDE_DESC
    "",
    "This counter counts the number of packets transmitted per unified port.\nCHANGED_BY_HW\n\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_UNIFIEDPKT_UNIFIED_PKT_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_UNIFIEDPKT_UNIFIED_PKT_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_UNIFIEDPKT_UNIFIED_PKT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_DEBUG_COUNTERS_UNIFIEDPKT_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_UNIFIEDPKT_UNIFIED_PKT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_UNIFIEDPKT *****/
const ru_reg_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_UNIFIEDPKT_REG =
{
    "DEBUG_COUNTERS_UNIFIEDPKT",
#if RU_INCLUDE_DESC
    "UNIFIED_PKT_COUNTER 0..7 Register",
    "This counter array counts the number of transmitted packets through each interface in the unified BBH.\n\nThis counter is cleared when read and freezes when maximum value is reached.\n\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_UNIFIEDPKT_REG_OFFSET },
    CRDP_EGR_BBH_TX_DEBUG_COUNTERS_UNIFIEDPKT_REG_RAM_CNT,
    4,
    658,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_EGR_BBH_TX_DEBUG_COUNTERS_UNIFIEDPKT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_UNIFIEDBYTE, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_UNIFIEDBYTE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DDRBYTE *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_UNIFIEDBYTE_DDRBYTE_FIELD =
{
    "DDRBYTE",
#if RU_INCLUDE_DESC
    "",
    "This counter counts the number of transmitted bytes from the DDr.\nCHANGED_BY_HW\n\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_UNIFIEDBYTE_DDRBYTE_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_UNIFIEDBYTE_DDRBYTE_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_UNIFIEDBYTE_DDRBYTE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_DEBUG_COUNTERS_UNIFIEDBYTE_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_UNIFIEDBYTE_DDRBYTE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_UNIFIEDBYTE *****/
const ru_reg_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_UNIFIEDBYTE_REG =
{
    "DEBUG_COUNTERS_UNIFIEDBYTE",
#if RU_INCLUDE_DESC
    "UNIFIED_BYTE_COUNTER 0..7 Register",
    "This counter array counts the number of transmitted bytes through each interface in the unified BBH.\n\nThis counter is cleared when read and freezes when maximum value is reached.\n\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_UNIFIEDBYTE_REG_OFFSET },
    CRDP_EGR_BBH_TX_DEBUG_COUNTERS_UNIFIEDBYTE_REG_RAM_CNT,
    4,
    659,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_EGR_BBH_TX_DEBUG_COUNTERS_UNIFIEDBYTE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_DBGOUTREG, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_DBGOUTREG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DBGVEC *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_DBGOUTREG_DBGVEC_FIELD =
{
    "DBGVEC",
#if RU_INCLUDE_DESC
    "",
    "Selected debug vector.\nCHANGED_BY_HW\n\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_DBGOUTREG_DBGVEC_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_DBGOUTREG_DBGVEC_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_DBGOUTREG_DBGVEC_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_DEBUG_COUNTERS_DBGOUTREG_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_DBGOUTREG_DBGVEC_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_DBGOUTREG *****/
const ru_reg_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_DBGOUTREG_REG =
{
    "DEBUG_COUNTERS_DBGOUTREG",
#if RU_INCLUDE_DESC
    "DEBUG_OUT_REG 0..31 Register",
    "an array including all the debug vectors of the BBH TX.\nentries 30 and 31 are DSL debug.\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_DBGOUTREG_REG_OFFSET },
    CRDP_EGR_BBH_TX_DEBUG_COUNTERS_DBGOUTREG_REG_RAM_CNT,
    4,
    660,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_EGR_BBH_TX_DEBUG_COUNTERS_DBGOUTREG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_IN_SEGMENTATION, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_IN_SEGMENTATION
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: IN_SEGMENTATION *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_IN_SEGMENTATION_IN_SEGMENTATION_FIELD =
{
    "IN_SEGMENTATION",
#if RU_INCLUDE_DESC
    "",
    "in_segmentation indication\nCHANGED_BY_HW\n\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_IN_SEGMENTATION_IN_SEGMENTATION_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_IN_SEGMENTATION_IN_SEGMENTATION_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_IN_SEGMENTATION_IN_SEGMENTATION_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_DEBUG_COUNTERS_IN_SEGMENTATION_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_IN_SEGMENTATION_IN_SEGMENTATION_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_IN_SEGMENTATION *****/
const ru_reg_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_IN_SEGMENTATION_REG =
{
    "DEBUG_COUNTERS_IN_SEGMENTATION",
#if RU_INCLUDE_DESC
    "IN_SEGMENTATION Register",
    "8 bit vector in which each bit represents if the segmentation SM is currently handling a PD of a certain TCONT.\n\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_IN_SEGMENTATION_REG_OFFSET },
    0,
    0,
    661,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_EGR_BBH_TX_DEBUG_COUNTERS_IN_SEGMENTATION_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_UNIFIED_DATA_CREDITS, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_UNIFIED_DATA_CREDITS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CREDITS *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_UNIFIED_DATA_CREDITS_CREDITS_FIELD =
{
    "CREDITS",
#if RU_INCLUDE_DESC
    "",
    "credits\nCHANGED_BY_HW\n\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_UNIFIED_DATA_CREDITS_CREDITS_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_UNIFIED_DATA_CREDITS_CREDITS_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_UNIFIED_DATA_CREDITS_CREDITS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_DEBUG_COUNTERS_UNIFIED_DATA_CREDITS_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_UNIFIED_DATA_CREDITS_CREDITS_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_UNIFIED_DATA_CREDITS *****/
const ru_reg_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_UNIFIED_DATA_CREDITS_REG =
{
    "DEBUG_COUNTERS_UNIFIED_DATA_CREDITS",
#if RU_INCLUDE_DESC
    "UNIFIED_DATA_CREDITS 0..7 Register",
    "per q status of the data credit counter that indicate the number of free entries in the FE TX buffer.\nThe BBH decrements the counter upon requesting data from the DMA. The counter is incremented when data is popped from the TX buffer and is sent to the MAC.\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_UNIFIED_DATA_CREDITS_REG_OFFSET },
    CRDP_EGR_BBH_TX_DEBUG_COUNTERS_UNIFIED_DATA_CREDITS_REG_RAM_CNT,
    4,
    662,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_EGR_BBH_TX_DEBUG_COUNTERS_UNIFIED_DATA_CREDITS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_UNIFIED_PD_CREDITS, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_UNIFIED_PD_CREDITS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CREDITS *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_UNIFIED_PD_CREDITS_CREDITS_FIELD =
{
    "CREDITS",
#if RU_INCLUDE_DESC
    "",
    "credits\nCHANGED_BY_HW\n\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_UNIFIED_PD_CREDITS_CREDITS_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_UNIFIED_PD_CREDITS_CREDITS_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_UNIFIED_PD_CREDITS_CREDITS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_DEBUG_COUNTERS_UNIFIED_PD_CREDITS_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_UNIFIED_PD_CREDITS_CREDITS_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_UNIFIED_PD_CREDITS *****/
const ru_reg_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_UNIFIED_PD_CREDITS_REG =
{
    "DEBUG_COUNTERS_UNIFIED_PD_CREDITS",
#if RU_INCLUDE_DESC
    "UNIFIED_PD_CREDITS 0..7 Register",
    "per q status of the pd credit counter that indicate the number of free entries in the FE PD buffer.\nThe BBH decrements the counter upon requesting data from the DMA. The counter is incremented when data is popped from the PD buffer and is sent to the MAC.\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_UNIFIED_PD_CREDITS_REG_OFFSET },
    CRDP_EGR_BBH_TX_DEBUG_COUNTERS_UNIFIED_PD_CREDITS_REG_RAM_CNT,
    4,
    663,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_EGR_BBH_TX_DEBUG_COUNTERS_UNIFIED_PD_CREDITS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_DISFLOWDROPPKT, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_DISFLOWDROPPKT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DROPPKT *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_DISFLOWDROPPKT_DROPPKT_FIELD =
{
    "DROPPKT",
#if RU_INCLUDE_DESC
    "",
    "This counter counts the number of dropped pkts due to GEM disable\nCHANGED_BY_HW\n\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_DISFLOWDROPPKT_DROPPKT_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_DISFLOWDROPPKT_DROPPKT_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_DISFLOWDROPPKT_DROPPKT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_DEBUG_COUNTERS_DISFLOWDROPPKT_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_DISFLOWDROPPKT_DROPPKT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_DISFLOWDROPPKT *****/
const ru_reg_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_DISFLOWDROPPKT_REG =
{
    "DEBUG_COUNTERS_DISFLOWDROPPKT",
#if RU_INCLUDE_DESC
    "DISABLED_FLOW_DROPPED_PACKETS_COUNTER Register",
    "was Disabled flow dropped packets counter but is not functional\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_DISFLOWDROPPKT_REG_OFFSET },
    0,
    0,
    664,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_EGR_BBH_TX_DEBUG_COUNTERS_DISFLOWDROPPKT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_DISFLOWDROPBYTE, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_DISFLOWDROPBYTE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DROPBYTE *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_DISFLOWDROPBYTE_DROPBYTE_FIELD =
{
    "DROPBYTE",
#if RU_INCLUDE_DESC
    "",
    "This counter counts the number of dropped bytes due to GEM disable\nCHANGED_BY_HW\n\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_DISFLOWDROPBYTE_DROPBYTE_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_DISFLOWDROPBYTE_DROPBYTE_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_DISFLOWDROPBYTE_DROPBYTE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_DEBUG_COUNTERS_DISFLOWDROPBYTE_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_DISFLOWDROPBYTE_DROPBYTE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_DISFLOWDROPBYTE *****/
const ru_reg_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_DISFLOWDROPBYTE_REG =
{
    "DEBUG_COUNTERS_DISFLOWDROPBYTE",
#if RU_INCLUDE_DESC
    "DISABLED_FLOW_DROPPED_BYTES_COUNTER Register",
    "This counter counts the number of bytes that were dropped due to the fact that the GEM is disabled (relevant only to the non WAN BBH)\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_DISFLOWDROPBYTE_REG_OFFSET },
    0,
    0,
    665,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_EGR_BBH_TX_DEBUG_COUNTERS_DISFLOWDROPBYTE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_ECNPKT, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_ECNPKT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ECNPKT *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_ECNPKT_ECNPKT_FIELD =
{
    "ECNPKT",
#if RU_INCLUDE_DESC
    "",
    "This counter counts the number of packets that were ecn remarked\nCHANGED_BY_HW\n\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_ECNPKT_ECNPKT_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_ECNPKT_ECNPKT_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_ECNPKT_ECNPKT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_DEBUG_COUNTERS_ECNPKT_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_ECNPKT_ECNPKT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_ECNPKT *****/
const ru_reg_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_ECNPKT_REG =
{
    "DEBUG_COUNTERS_ECNPKT",
#if RU_INCLUDE_DESC
    "ECN_PKT_COUNTER Register",
    "This counter counts the number of packets that their ECN bits were remarked\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_ECNPKT_REG_OFFSET },
    0,
    0,
    666,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_EGR_BBH_TX_DEBUG_COUNTERS_ECNPKT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_ECNBYTES, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_ECNBYTES
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ECNBYTES *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_ECNBYTES_ECNBYTES_FIELD =
{
    "ECNBYTES",
#if RU_INCLUDE_DESC
    "",
    "This counter counts the number of bytes of ecn marked packets.\nCHANGED_BY_HW\n\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_ECNBYTES_ECNBYTES_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_ECNBYTES_ECNBYTES_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_ECNBYTES_ECNBYTES_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_DEBUG_COUNTERS_ECNBYTES_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_ECNBYTES_ECNBYTES_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_ECNBYTES *****/
const ru_reg_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_ECNBYTES_REG =
{
    "DEBUG_COUNTERS_ECNBYTES",
#if RU_INCLUDE_DESC
    "ECN_BYTES_COUNTER Register",
    "This counter counts the number of bytes of packets that their ECN bits were remarked\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_ECNBYTES_REG_OFFSET },
    0,
    0,
    667,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_EGR_BBH_TX_DEBUG_COUNTERS_ECNBYTES_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_ECNMARKERR, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_ECNMARKERR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: V1 *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_ECNMARKERR_V1_FIELD =
{
    "V1",
#if RU_INCLUDE_DESC
    "",
    "value1\nCHANGED_BY_HW\n\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_ECNMARKERR_V1_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_ECNMARKERR_V1_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_ECNMARKERR_V1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: V2 *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_ECNMARKERR_V2_FIELD =
{
    "V2",
#if RU_INCLUDE_DESC
    "",
    "value2\nCHANGED_BY_HW\n\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_ECNMARKERR_V2_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_ECNMARKERR_V2_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_ECNMARKERR_V2_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_DEBUG_COUNTERS_ECNMARKERR_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_ECNMARKERR_V1_FIELD,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_ECNMARKERR_V2_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_ECNMARKERR *****/
const ru_reg_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_ECNMARKERR_REG =
{
    "DEBUG_COUNTERS_ECNMARKERR",
#if RU_INCLUDE_DESC
    "ECN_MARK_ERR Register",
    "this counter counts two types of ecn mark error:\n1. the ECN capable bits were asserted in both PD and EH, but the ECN bits in the packet were zero.\n2. the ECN capable bits were asserted in both PD and EH, but the ECN bits in the packet were already asserted.\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_ECNMARKERR_REG_OFFSET },
    0,
    0,
    668,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    CRDP_EGR_BBH_TX_DEBUG_COUNTERS_ECNMARKERR_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_ECNLENERR, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_ECNLENERR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: V1 *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_ECNLENERR_V1_FIELD =
{
    "V1",
#if RU_INCLUDE_DESC
    "",
    "value1\nCHANGED_BY_HW\n\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_ECNLENERR_V1_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_ECNLENERR_V1_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_ECNLENERR_V1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: V2 *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_ECNLENERR_V2_FIELD =
{
    "V2",
#if RU_INCLUDE_DESC
    "",
    "value2\nCHANGED_BY_HW\n\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_ECNLENERR_V2_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_ECNLENERR_V2_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_ECNLENERR_V2_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_DEBUG_COUNTERS_ECNLENERR_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_ECNLENERR_V1_FIELD,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_ECNLENERR_V2_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_ECNLENERR *****/
const ru_reg_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_ECNLENERR_REG =
{
    "DEBUG_COUNTERS_ECNLENERR",
#if RU_INCLUDE_DESC
    "ECN_PLEN_ERR Register",
    "These counters count plen error events related to ECN.\nevent 1 - packet is marked as ECN capable on both PD and EH, but the PLEN is shorter than the ECN offset\nevent 2 - packet is marked as ECN capable on both PD and EH and the packet is IPV, but the PLEN is shorter than the checksum offset.\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_ECNLENERR_REG_OFFSET },
    0,
    0,
    669,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    CRDP_EGR_BBH_TX_DEBUG_COUNTERS_ECNLENERR_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_FLUSH_INFO, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_FLUSH_INFO
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: SN *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_FLUSH_INFO_SN_FIELD =
{
    "SN",
#if RU_INCLUDE_DESC
    "",
    "bbh 200/400 serial number\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_FLUSH_INFO_SN_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_FLUSH_INFO_SN_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_FLUSH_INFO_SN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PDFIFOEMPTY *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_FLUSH_INFO_PDFIFOEMPTY_FIELD =
{
    "PDFIFOEMPTY",
#if RU_INCLUDE_DESC
    "",
    "pd_fifo_empty - bit per port\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_FLUSH_INFO_PDFIFOEMPTY_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_FLUSH_INFO_PDFIFOEMPTY_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_FLUSH_INFO_PDFIFOEMPTY_FIELD_SHIFT },
    255,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PDARRAYEMPTY *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_FLUSH_INFO_PDARRAYEMPTY_FIELD =
{
    "PDARRAYEMPTY",
#if RU_INCLUDE_DESC
    "",
    "pd_array_empty - bit per port\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_FLUSH_INFO_PDARRAYEMPTY_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_FLUSH_INFO_PDARRAYEMPTY_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_FLUSH_INFO_PDARRAYEMPTY_FIELD_SHIFT },
    255,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_DEBUG_COUNTERS_FLUSH_INFO_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_FLUSH_INFO_SN_FIELD,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_FLUSH_INFO_PDFIFOEMPTY_FIELD,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_FLUSH_INFO_PDARRAYEMPTY_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_FLUSH_INFO *****/
const ru_reg_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_FLUSH_INFO_REG =
{
    "DEBUG_COUNTERS_FLUSH_INFO",
#if RU_INCLUDE_DESC
    "FLUSH_INFO Register",
    "indications whether the BBH is empty or not\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_FLUSH_INFO_REG_OFFSET },
    0,
    0,
    670,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    CRDP_EGR_BBH_TX_DEBUG_COUNTERS_FLUSH_INFO_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_FLUSH_INFO2, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_FLUSH_INFO2
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PFBINGEMPTY *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_FLUSH_INFO2_PFBINGEMPTY_FIELD =
{
    "PFBINGEMPTY",
#if RU_INCLUDE_DESC
    "",
    "pfb ingress fifo empty - bit per port\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_FLUSH_INFO2_PFBINGEMPTY_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_FLUSH_INFO2_PFBINGEMPTY_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_FLUSH_INFO2_PFBINGEMPTY_FIELD_SHIFT },
    255,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FEPDEMPTY *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_FLUSH_INFO2_FEPDEMPTY_FIELD =
{
    "FEPDEMPTY",
#if RU_INCLUDE_DESC
    "",
    "FE PD FIFO empty - bit per port\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_FLUSH_INFO2_FEPDEMPTY_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_FLUSH_INFO2_FEPDEMPTY_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_FLUSH_INFO2_FEPDEMPTY_FIELD_SHIFT },
    255,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FEDATAEMPTY *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_FLUSH_INFO2_FEDATAEMPTY_FIELD =
{
    "FEDATAEMPTY",
#if RU_INCLUDE_DESC
    "",
    "FE data FIFO empty - bit per port\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_FLUSH_INFO2_FEDATAEMPTY_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_FLUSH_INFO2_FEDATAEMPTY_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_FLUSH_INFO2_FEDATAEMPTY_FIELD_SHIFT },
    255,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PORTVLD *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_FLUSH_INFO2_PORTVLD_FIELD =
{
    "PORTVLD",
#if RU_INCLUDE_DESC
    "",
    "indication that the port has credits and a pending pd\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_FLUSH_INFO2_PORTVLD_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_FLUSH_INFO2_PORTVLD_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_FLUSH_INFO2_PORTVLD_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_DEBUG_COUNTERS_FLUSH_INFO2_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_FLUSH_INFO2_PFBINGEMPTY_FIELD,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_FLUSH_INFO2_FEPDEMPTY_FIELD,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_FLUSH_INFO2_FEDATAEMPTY_FIELD,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_FLUSH_INFO2_PORTVLD_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_FLUSH_INFO2 *****/
const ru_reg_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_FLUSH_INFO2_REG =
{
    "DEBUG_COUNTERS_FLUSH_INFO2",
#if RU_INCLUDE_DESC
    "FLUSH_INFO_2 Register",
    "indications whether the BBH is empty or not\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_FLUSH_INFO2_REG_OFFSET },
    0,
    0,
    671,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    CRDP_EGR_BBH_TX_DEBUG_COUNTERS_FLUSH_INFO2_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_PD_EH_MISMATCH, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_PD_EH_MISMATCH
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CNT *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_PD_EH_MISMATCH_CNT_FIELD =
{
    "CNT",
#if RU_INCLUDE_DESC
    "",
    "cnt\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_PD_EH_MISMATCH_CNT_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_PD_EH_MISMATCH_CNT_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_PD_EH_MISMATCH_CNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_DEBUG_COUNTERS_PD_EH_MISMATCH_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_PD_EH_MISMATCH_CNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_PD_EH_MISMATCH *****/
const ru_reg_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_PD_EH_MISMATCH_REG =
{
    "DEBUG_COUNTERS_PD_EH_MISMATCH",
#if RU_INCLUDE_DESC
    "PD_EH_MISMATCH_CNT Register",
    "counts mismatch events between the plen in pd and the total eh plens\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_PD_EH_MISMATCH_REG_OFFSET },
    0,
    0,
    672,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_EGR_BBH_TX_DEBUG_COUNTERS_PD_EH_MISMATCH_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_PD_EH_MISMATCH_DATA, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_PD_EH_MISMATCH_DATA
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PORT *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_PD_EH_MISMATCH_DATA_PORT_FIELD =
{
    "PORT",
#if RU_INCLUDE_DESC
    "",
    "port\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_PD_EH_MISMATCH_DATA_PORT_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_PD_EH_MISMATCH_DATA_PORT_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_PD_EH_MISMATCH_DATA_PORT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PLEN *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_PD_EH_MISMATCH_DATA_PLEN_FIELD =
{
    "PLEN",
#if RU_INCLUDE_DESC
    "",
    "plen\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_PD_EH_MISMATCH_DATA_PLEN_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_PD_EH_MISMATCH_DATA_PLEN_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_PD_EH_MISMATCH_DATA_PLEN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PKTNUM *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_PD_EH_MISMATCH_DATA_PKTNUM_FIELD =
{
    "PKTNUM",
#if RU_INCLUDE_DESC
    "",
    "number of aggregated packets\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_PD_EH_MISMATCH_DATA_PKTNUM_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_PD_EH_MISMATCH_DATA_PKTNUM_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_PD_EH_MISMATCH_DATA_PKTNUM_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_DEBUG_COUNTERS_PD_EH_MISMATCH_DATA_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_PD_EH_MISMATCH_DATA_PORT_FIELD,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_PD_EH_MISMATCH_DATA_PLEN_FIELD,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_PD_EH_MISMATCH_DATA_PKTNUM_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_PD_EH_MISMATCH_DATA *****/
const ru_reg_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_PD_EH_MISMATCH_DATA_REG =
{
    "DEBUG_COUNTERS_PD_EH_MISMATCH_DATA",
#if RU_INCLUDE_DESC
    "PD_EH_MISMATCH_DATA Register",
    "data of the first mismatch event between the plen in pd and the total eh plens.\nThe next event will be sampled again on the next event after read of the current data\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_PD_EH_MISMATCH_DATA_REG_OFFSET },
    0,
    0,
    673,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    CRDP_EGR_BBH_TX_DEBUG_COUNTERS_PD_EH_MISMATCH_DATA_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_OUTOFRANGE_ERRORS, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_OUTOFRANGE_ERRORS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: MACFLOW *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_OUTOFRANGE_ERRORS_MACFLOW_FIELD =
{
    "MACFLOW",
#if RU_INCLUDE_DESC
    "",
    "mac flow field in eh is out of range\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_OUTOFRANGE_ERRORS_MACFLOW_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_OUTOFRANGE_ERRORS_MACFLOW_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_OUTOFRANGE_ERRORS_MACFLOW_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CTRIDX *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_OUTOFRANGE_ERRORS_CTRIDX_FIELD =
{
    "CTRIDX",
#if RU_INCLUDE_DESC
    "",
    "counter index field in eh is out of range\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_OUTOFRANGE_ERRORS_CTRIDX_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_OUTOFRANGE_ERRORS_CTRIDX_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_OUTOFRANGE_ERRORS_CTRIDX_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PORT *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_OUTOFRANGE_ERRORS_PORT_FIELD =
{
    "PORT",
#if RU_INCLUDE_DESC
    "",
    "the port in which the error occurred\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_OUTOFRANGE_ERRORS_PORT_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_OUTOFRANGE_ERRORS_PORT_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_OUTOFRANGE_ERRORS_PORT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_DEBUG_COUNTERS_OUTOFRANGE_ERRORS_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_OUTOFRANGE_ERRORS_MACFLOW_FIELD,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_OUTOFRANGE_ERRORS_CTRIDX_FIELD,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_OUTOFRANGE_ERRORS_PORT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_OUTOFRANGE_ERRORS *****/
const ru_reg_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_OUTOFRANGE_ERRORS_REG =
{
    "DEBUG_COUNTERS_OUTOFRANGE_ERRORS",
#if RU_INCLUDE_DESC
    "FLOW_COUNTERS_ERRORS Register",
    "sticky out of range errors status bits\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_OUTOFRANGE_ERRORS_REG_OFFSET },
    0,
    0,
    674,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    CRDP_EGR_BBH_TX_DEBUG_COUNTERS_OUTOFRANGE_ERRORS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_GEMDROPPKT, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_GEMDROPPKT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PKT *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_GEMDROPPKT_PKT_FIELD =
{
    "PKT",
#if RU_INCLUDE_DESC
    "",
    "gem dropped pkt cnt\nCHANGED_BY_HW\n\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_GEMDROPPKT_PKT_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_GEMDROPPKT_PKT_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_GEMDROPPKT_PKT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_DEBUG_COUNTERS_GEMDROPPKT_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_GEMDROPPKT_PKT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_GEMDROPPKT *****/
const ru_reg_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_GEMDROPPKT_REG =
{
    "DEBUG_COUNTERS_GEMDROPPKT",
#if RU_INCLUDE_DESC
    "GEM_DIS_DROPPED_PKT_COUNTER 0..7 Register",
    "This counter array counts the number of gem disabled dropped packets through each channel of the BBH.\n\nThis counter is cleared when read and freezes when maximum value is reached.\n\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_GEMDROPPKT_REG_OFFSET },
    CRDP_EGR_BBH_TX_DEBUG_COUNTERS_GEMDROPPKT_REG_RAM_CNT,
    4,
    675,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_EGR_BBH_TX_DEBUG_COUNTERS_GEMDROPPKT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_GEMDROPBYTES, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_GEMDROPBYTES
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: BYTES *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_GEMDROPBYTES_BYTES_FIELD =
{
    "BYTES",
#if RU_INCLUDE_DESC
    "",
    "gem dropped bytes cnt\nCHANGED_BY_HW\n\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_GEMDROPBYTES_BYTES_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_GEMDROPBYTES_BYTES_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_GEMDROPBYTES_BYTES_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_DEBUG_COUNTERS_GEMDROPBYTES_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_GEMDROPBYTES_BYTES_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_GEMDROPBYTES *****/
const ru_reg_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_GEMDROPBYTES_REG =
{
    "DEBUG_COUNTERS_GEMDROPBYTES",
#if RU_INCLUDE_DESC
    "GEM_DIS_DROPPED_BYTES_COUNTER 0..7 Register",
    "This counter array counts the number of gem disabled dropped bytes through each channel of the BBH.\n\nThis counter is cleared when read and freezes when maximum value is reached.\n\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_GEMDROPBYTES_REG_OFFSET },
    CRDP_EGR_BBH_TX_DEBUG_COUNTERS_GEMDROPBYTES_REG_RAM_CNT,
    4,
    676,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_EGR_BBH_TX_DEBUG_COUNTERS_GEMDROPBYTES_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_CMDOTFDDR, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_CMDOTFDDR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VALUE *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_CMDOTFDDR_VALUE_FIELD =
{
    "VALUE",
#if RU_INCLUDE_DESC
    "",
    "value\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_CMDOTFDDR_VALUE_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_CMDOTFDDR_VALUE_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_CMDOTFDDR_VALUE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_DEBUG_COUNTERS_CMDOTFDDR_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_CMDOTFDDR_VALUE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_CMDOTFDDR *****/
const ru_reg_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_CMDOTFDDR_REG =
{
    "DEBUG_COUNTERS_CMDOTFDDR",
#if RU_INCLUDE_DESC
    "COMMANSD_ON_THE_FLY_DDR 0..7 Register",
    "commansd_on_the_fly_ddr\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_CMDOTFDDR_REG_OFFSET },
    CRDP_EGR_BBH_TX_DEBUG_COUNTERS_CMDOTFDDR_REG_RAM_CNT,
    4,
    677,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_EGR_BBH_TX_DEBUG_COUNTERS_CMDOTFDDR_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_BYTESOTFDDR, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_BYTESOTFDDR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VALUE *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_BYTESOTFDDR_VALUE_FIELD =
{
    "VALUE",
#if RU_INCLUDE_DESC
    "",
    "value\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_BYTESOTFDDR_VALUE_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_BYTESOTFDDR_VALUE_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_BYTESOTFDDR_VALUE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_DEBUG_COUNTERS_BYTESOTFDDR_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_BYTESOTFDDR_VALUE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_BYTESOTFDDR *****/
const ru_reg_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_BYTESOTFDDR_REG =
{
    "DEBUG_COUNTERS_BYTESOTFDDR",
#if RU_INCLUDE_DESC
    "BYTES_ON_THE_FLY_DDR 0..7 Register",
    "bytes_on_the_fly_ddr in 32 bytes resolution\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_BYTESOTFDDR_REG_OFFSET },
    CRDP_EGR_BBH_TX_DEBUG_COUNTERS_BYTESOTFDDR_REG_RAM_CNT,
    4,
    678,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_EGR_BBH_TX_DEBUG_COUNTERS_BYTESOTFDDR_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_CMDOTFEPSRAM, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_CMDOTFEPSRAM
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VALUE *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_CMDOTFEPSRAM_VALUE_FIELD =
{
    "VALUE",
#if RU_INCLUDE_DESC
    "",
    "value\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_CMDOTFEPSRAM_VALUE_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_CMDOTFEPSRAM_VALUE_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_CMDOTFEPSRAM_VALUE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_DEBUG_COUNTERS_CMDOTFEPSRAM_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_CMDOTFEPSRAM_VALUE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_CMDOTFEPSRAM *****/
const ru_reg_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_CMDOTFEPSRAM_REG =
{
    "DEBUG_COUNTERS_CMDOTFEPSRAM",
#if RU_INCLUDE_DESC
    "COMMANSD_ON_THE_FLY_EPSRAM 0..15 Register",
    "commansd_on_the_fly_epsrams\n\nfirst 8 entries are for epsram0\nlast 8 entries are for epsram1\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_CMDOTFEPSRAM_REG_OFFSET },
    CRDP_EGR_BBH_TX_DEBUG_COUNTERS_CMDOTFEPSRAM_REG_RAM_CNT,
    4,
    679,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_EGR_BBH_TX_DEBUG_COUNTERS_CMDOTFEPSRAM_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_BYTESOTFEPSRAM, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_BYTESOTFEPSRAM
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VALUE *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_BYTESOTFEPSRAM_VALUE_FIELD =
{
    "VALUE",
#if RU_INCLUDE_DESC
    "",
    "value\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_BYTESOTFEPSRAM_VALUE_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_BYTESOTFEPSRAM_VALUE_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_BYTESOTFEPSRAM_VALUE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_DEBUG_COUNTERS_BYTESOTFEPSRAM_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_BYTESOTFEPSRAM_VALUE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_BYTESOTFEPSRAM *****/
const ru_reg_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_BYTESOTFEPSRAM_REG =
{
    "DEBUG_COUNTERS_BYTESOTFEPSRAM",
#if RU_INCLUDE_DESC
    "BYTES_ON_THE_FLY_EPSRAM 0..15 Register",
    "bytes_on_the_fly_epsram0 in 32 bytes resolution\n\nfirst 8 entries are for epsram0\nlast 8 entries are for epsram1\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_BYTESOTFEPSRAM_REG_OFFSET },
    CRDP_EGR_BBH_TX_DEBUG_COUNTERS_BYTESOTFEPSRAM_REG_RAM_CNT,
    4,
    680,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_EGR_BBH_TX_DEBUG_COUNTERS_BYTESOTFEPSRAM_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_GNEXT, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_GNEXT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ENTRY *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_GNEXT_ENTRY_FIELD =
{
    "ENTRY",
#if RU_INCLUDE_DESC
    "",
    "entry\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_GNEXT_ENTRY_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_GNEXT_ENTRY_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_GNEXT_ENTRY_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_DEBUG_COUNTERS_GNEXT_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_GNEXT_ENTRY_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_GNEXT *****/
const ru_reg_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_GNEXT_REG =
{
    "DEBUG_COUNTERS_GNEXT",
#if RU_INCLUDE_DESC
    "GET_NEXT_ARRAYS 0..63 Register",
    "get_next_array.\n32 entries (4x16). each 4 entries represent the get next array of a port.\nentry 0-3 are for port0, entry 28-31 are for port7\n\nfirst 32 entries are for epsram0\nlast 32 entries are for epsram1\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_GNEXT_REG_OFFSET },
    CRDP_EGR_BBH_TX_DEBUG_COUNTERS_GNEXT_REG_RAM_CNT,
    4,
    681,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_EGR_BBH_TX_DEBUG_COUNTERS_GNEXT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_PDARRAY, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_PDARRAY
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ENTRY *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_PDARRAY_ENTRY_FIELD =
{
    "ENTRY",
#if RU_INCLUDE_DESC
    "",
    "entry\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_PDARRAY_ENTRY_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_PDARRAY_ENTRY_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_PDARRAY_ENTRY_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_DEBUG_COUNTERS_PDARRAY_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_PDARRAY_ENTRY_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_PDARRAY *****/
const ru_reg_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_PDARRAY_REG =
{
    "DEBUG_COUNTERS_PDARRAY",
#if RU_INCLUDE_DESC
    "PD_ARRAY 0..31 Register",
    "pd_array.\n32 entries (4x8). each 4 entries represent the get next array of a port.\nentry 0-3 are for port0, entry 28-31 are for port7\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_PDARRAY_REG_OFFSET },
    CRDP_EGR_BBH_TX_DEBUG_COUNTERS_PDARRAY_REG_RAM_CNT,
    4,
    682,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_EGR_BBH_TX_DEBUG_COUNTERS_PDARRAY_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_CURPD, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_CURPD
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ENTRY *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_CURPD_ENTRY_FIELD =
{
    "ENTRY",
#if RU_INCLUDE_DESC
    "",
    "entry\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_CURPD_ENTRY_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_CURPD_ENTRY_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_CURPD_ENTRY_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_DEBUG_COUNTERS_CURPD_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_CURPD_ENTRY_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_CURPD *****/
const ru_reg_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_CURPD_REG =
{
    "DEBUG_COUNTERS_CURPD",
#if RU_INCLUDE_DESC
    "CURRENT_PD 0..3 Register",
    "current pd. 4 entries. entry 0 is 32 lsb of the pd\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_CURPD_REG_OFFSET },
    CRDP_EGR_BBH_TX_DEBUG_COUNTERS_CURPD_REG_RAM_CNT,
    4,
    683,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_EGR_BBH_TX_DEBUG_COUNTERS_CURPD_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SPACE4REQ, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_SPACE4REQ
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RESPS *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SPACE4REQ_RESPS_FIELD =
{
    "RESPS",
#if RU_INCLUDE_DESC
    "",
    "there are enough credits in the responders reorder fifos.\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SPACE4REQ_RESPS_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SPACE4REQ_RESPS_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SPACE4REQ_RESPS_FIELD_SHIFT },
    255,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TARGET_MEM *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SPACE4REQ_TARGET_MEM_FIELD =
{
    "TARGET_MEM",
#if RU_INCLUDE_DESC
    "",
    "the on the fly commands and bytes counters per port per target memory did not cross the thresholds.\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SPACE4REQ_TARGET_MEM_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SPACE4REQ_TARGET_MEM_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SPACE4REQ_TARGET_MEM_FIELD_SHIFT },
    255,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FE *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SPACE4REQ_FE_FIELD =
{
    "FE",
#if RU_INCLUDE_DESC
    "",
    "there are enough credits in the front_end pd and data buffers.\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SPACE4REQ_FE_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SPACE4REQ_FE_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SPACE4REQ_FE_FIELD_SHIFT },
    255,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DP *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SPACE4REQ_DP_FIELD =
{
    "DP",
#if RU_INCLUDE_DESC
    "",
    "there is space in the per port pd,pc,reorder fifos in the BBH TX data path.\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SPACE4REQ_DP_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SPACE4REQ_DP_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SPACE4REQ_DP_FIELD_SHIFT },
    255,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SPACE4REQ_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SPACE4REQ_RESPS_FIELD,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SPACE4REQ_TARGET_MEM_FIELD,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SPACE4REQ_FE_FIELD,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SPACE4REQ_DP_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SPACE4REQ *****/
const ru_reg_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SPACE4REQ_REG =
{
    "DEBUG_COUNTERS_SPACE4REQ",
#if RU_INCLUDE_DESC
    "SPACE4_REQ_INFO Register",
    "space4_req_info. indications for the segmentation sm which q has space for next command in its fifos\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SPACE4REQ_REG_OFFSET },
    0,
    0,
    684,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SPACE4REQ_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SEGINFO, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_SEGINFO
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PDEMPTY *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SEGINFO_PDEMPTY_FIELD =
{
    "PDEMPTY",
#if RU_INCLUDE_DESC
    "",
    "pd_array emtpy\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SEGINFO_PDEMPTY_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SEGINFO_PDEMPTY_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SEGINFO_PDEMPTY_FIELD_SHIFT },
    255,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: GNRDY0 *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SEGINFO_GNRDY0_FIELD =
{
    "GNRDY0",
#if RU_INCLUDE_DESC
    "",
    "get_next_ready0\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SEGINFO_GNRDY0_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SEGINFO_GNRDY0_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SEGINFO_GNRDY0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: GNRDY1 *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SEGINFO_GNRDY1_FIELD =
{
    "GNRDY1",
#if RU_INCLUDE_DESC
    "",
    "get_next_ready1\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SEGINFO_GNRDY1_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SEGINFO_GNRDY1_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SEGINFO_GNRDY1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: QRDY *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SEGINFO_QRDY_FIELD =
{
    "QRDY",
#if RU_INCLUDE_DESC
    "",
    "q_is_ready\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SEGINFO_QRDY_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SEGINFO_QRDY_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SEGINFO_QRDY_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SEGINFO_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SEGINFO_PDEMPTY_FIELD,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SEGINFO_GNRDY0_FIELD,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SEGINFO_GNRDY1_FIELD,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SEGINFO_QRDY_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SEGINFO *****/
const ru_reg_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SEGINFO_REG =
{
    "DEBUG_COUNTERS_SEGINFO",
#if RU_INCLUDE_DESC
    "SEGMENTATION_SM_INFO Register",
    "info used by the sm to determine which q is valid for next command\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SEGINFO_REG_OFFSET },
    0,
    0,
    685,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SEGINFO_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_EHDATVAL, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_EHDATVAL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_EHDATVAL_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "valid\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_EHDATVAL_VAL_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_EHDATVAL_VAL_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_EHDATVAL_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_DEBUG_COUNTERS_EHDATVAL_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_EHDATVAL_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_EHDATVAL *****/
const ru_reg_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_EHDATVAL_REG =
{
    "DEBUG_COUNTERS_EHDATVAL",
#if RU_INCLUDE_DESC
    "EH_DATA_VLD Register",
    "last eh data per port valid\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_EHDATVAL_REG_OFFSET },
    0,
    0,
    686,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_EGR_BBH_TX_DEBUG_COUNTERS_EHDATVAL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_EHDAT, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_EHDAT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_EHDAT_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "data\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_EHDAT_DATA_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_EHDAT_DATA_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_EHDAT_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_DEBUG_COUNTERS_EHDAT_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_EHDAT_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_EHDAT *****/
const ru_reg_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_EHDAT_REG =
{
    "DEBUG_COUNTERS_EHDAT",
#if RU_INCLUDE_DESC
    "EH_DATA 0..7 Register",
    "last eh data per port\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_EHDAT_REG_OFFSET },
    CRDP_EGR_BBH_TX_DEBUG_COUNTERS_EHDAT_REG_RAM_CNT,
    4,
    687,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_EGR_BBH_TX_DEBUG_COUNTERS_EHDAT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RMTPDOCC, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_RMTPDOCC
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: OCC1 *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RMTPDOCC_OCC1_FIELD =
{
    "OCC1",
#if RU_INCLUDE_DESC
    "",
    "occupancy1\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RMTPDOCC_OCC1_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RMTPDOCC_OCC1_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RMTPDOCC_OCC1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: OCC2 *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RMTPDOCC_OCC2_FIELD =
{
    "OCC2",
#if RU_INCLUDE_DESC
    "",
    "occupancy2\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RMTPDOCC_OCC2_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RMTPDOCC_OCC2_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RMTPDOCC_OCC2_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: OCC3 *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RMTPDOCC_OCC3_FIELD =
{
    "OCC3",
#if RU_INCLUDE_DESC
    "",
    "occupancy3\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RMTPDOCC_OCC3_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RMTPDOCC_OCC3_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RMTPDOCC_OCC3_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RMTPDOCC_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RMTPDOCC_OCC1_FIELD,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RMTPDOCC_OCC2_FIELD,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RMTPDOCC_OCC3_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RMTPDOCC *****/
const ru_reg_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RMTPDOCC_REG =
{
    "DEBUG_COUNTERS_RMTPDOCC",
#if RU_INCLUDE_DESC
    "BBH_200_400_REMOTE_PD_FIFO_OCCUPANCY Register",
    "The remote PD fifos occupancies\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RMTPDOCC_REG_OFFSET },
    0,
    0,
    688,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RMTPDOCC_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RMTBYTES, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_RMTBYTES
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RMTBYTES_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "value\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RMTBYTES_VAL_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RMTBYTES_VAL_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RMTBYTES_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RMTBYTES_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RMTBYTES_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RMTBYTES *****/
const ru_reg_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RMTBYTES_REG =
{
    "DEBUG_COUNTERS_RMTBYTES",
#if RU_INCLUDE_DESC
    "BBH_200_400_REMOTE_OVERALL_BYTES_CTR Register",
    "BBH_200_400_remote overall bytes counter\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RMTBYTES_REG_OFFSET },
    0,
    0,
    689,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RMTBYTES_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RMTLBCNT, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_RMTLBCNT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RMTLBCNT_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "value\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RMTLBCNT_VAL_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RMTLBCNT_VAL_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RMTLBCNT_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RMTLBCNT_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RMTLBCNT_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RMTLBCNT *****/
const ru_reg_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RMTLBCNT_REG =
{
    "DEBUG_COUNTERS_RMTLBCNT",
#if RU_INCLUDE_DESC
    "BBH_200_400_LOAD_BALANCING_COUNT 0..3 Register",
    "BBH_200_400_load_balancing_count\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RMTLBCNT_REG_OFFSET },
    CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RMTLBCNT_REG_RAM_CNT,
    4,
    690,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RMTLBCNT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RESPSPD, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_RESPSPD
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CREDIT *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RESPSPD_CREDIT_FIELD =
{
    "CREDIT",
#if RU_INCLUDE_DESC
    "",
    "credit\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RESPSPD_CREDIT_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RESPSPD_CREDIT_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RESPSPD_CREDIT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RESPSPD_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RESPSPD_CREDIT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RESPSPD *****/
const ru_reg_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RESPSPD_REG =
{
    "DEBUG_COUNTERS_RESPSPD",
#if RU_INCLUDE_DESC
    "RESPS_PD_CREDITS 0..23 Register",
    "resps_pd_credits\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RESPSPD_REG_OFFSET },
    CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RESPSPD_REG_RAM_CNT,
    4,
    691,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RESPSPD_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RESPSDATA, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_RESPSDATA
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CREDIT *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RESPSDATA_CREDIT_FIELD =
{
    "CREDIT",
#if RU_INCLUDE_DESC
    "",
    "credit\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RESPSDATA_CREDIT_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RESPSDATA_CREDIT_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RESPSDATA_CREDIT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RESPSDATA_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RESPSDATA_CREDIT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RESPSDATA *****/
const ru_reg_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RESPSDATA_REG =
{
    "DEBUG_COUNTERS_RESPSDATA",
#if RU_INCLUDE_DESC
    "RESPS_DATA_CREDITS 0..23 Register",
    "resps_data_credits\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RESPSDATA_REG_OFFSET },
    CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RESPSDATA_REG_RAM_CNT,
    4,
    692,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RESPSDATA_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RNRSMPDCNT, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_RNRSMPDCNT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VALUE *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RNRSMPDCNT_VALUE_FIELD =
{
    "VALUE",
#if RU_INCLUDE_DESC
    "",
    "value\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RNRSMPDCNT_VALUE_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RNRSMPDCNT_VALUE_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RNRSMPDCNT_VALUE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RNRSMPDCNT_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RNRSMPDCNT_VALUE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RNRSMPDCNT *****/
const ru_reg_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RNRSMPDCNT_REG =
{
    "DEBUG_COUNTERS_RNRSMPDCNT",
#if RU_INCLUDE_DESC
    "RUNNER_SM_PD_CTRS 0..7 Register",
    "runner_sm_pd_ctrs\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RNRSMPDCNT_REG_OFFSET },
    CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RNRSMPDCNT_REG_RAM_CNT,
    4,
    693,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RNRSMPDCNT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RNRSMBYTECNT, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_RNRSMBYTECNT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VALUE *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RNRSMBYTECNT_VALUE_FIELD =
{
    "VALUE",
#if RU_INCLUDE_DESC
    "",
    "value\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RNRSMBYTECNT_VALUE_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RNRSMBYTECNT_VALUE_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RNRSMBYTECNT_VALUE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RNRSMBYTECNT_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RNRSMBYTECNT_VALUE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RNRSMBYTECNT *****/
const ru_reg_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RNRSMBYTECNT_REG =
{
    "DEBUG_COUNTERS_RNRSMBYTECNT",
#if RU_INCLUDE_DESC
    "RUNNER_SM_BYTE_CTRS 0..7 Register",
    "runner_sm_byte_ctrs\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RNRSMBYTECNT_REG_OFFSET },
    CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RNRSMBYTECNT_REG_RAM_CNT,
    4,
    694,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RNRSMBYTECNT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_PORTDBG, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_PORTDBG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_PORTDBG_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "data\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_PORTDBG_DATA_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_PORTDBG_DATA_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_PORTDBG_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_DEBUG_COUNTERS_PORTDBG_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_PORTDBG_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_PORTDBG *****/
const ru_reg_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_PORTDBG_REG =
{
    "DEBUG_COUNTERS_PORTDBG",
#if RU_INCLUDE_DESC
    "PER_PORT_DBG 0..23 Register",
    "96 bit debug info per port\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_PORTDBG_REG_OFFSET },
    CRDP_EGR_BBH_TX_DEBUG_COUNTERS_PORTDBG_REG_RAM_CNT,
    4,
    695,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_EGR_BBH_TX_DEBUG_COUNTERS_PORTDBG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RESPDBG, TYPE: Type_BBH_TX_BBHTX_DEBUG_COUNTERS_RESPDBG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RESPDBG_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "data\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RESPDBG_DATA_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RESPDBG_DATA_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RESPDBG_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RESPDBG_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RESPDBG_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RESPDBG *****/
const ru_reg_rec CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RESPDBG_REG =
{
    "DEBUG_COUNTERS_RESPDBG",
#if RU_INCLUDE_DESC
    "PER_RESP_DBG 0..23 Register",
    "32 bit debug info per responder\n",
#endif
    { CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RESPDBG_REG_OFFSET },
    CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RESPDBG_REG_RAM_CNT,
    4,
    696,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RESPDBG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_0, TYPE: Type_BBH_TX_BBHTX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: FLOW *****/
const ru_field_rec CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_0_FLOW_FIELD =
{
    "FLOW",
#if RU_INCLUDE_DESC
    "",
    "flow_index\n",
#endif
    { CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_0_FLOW_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_0_FLOW_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_0_FLOW_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: GEM *****/
const ru_field_rec CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_0_GEM_FIELD =
{
    "GEM",
#if RU_INCLUDE_DESC
    "",
    "gem port-id\n",
#endif
    { CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_0_GEM_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_0_GEM_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_0_GEM_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: VALID *****/
const ru_field_rec CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_0_VALID_FIELD =
{
    "VALID",
#if RU_INCLUDE_DESC
    "",
    "valid\n",
#endif
    { CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_0_VALID_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_0_VALID_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_0_VALID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_0_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_0_FLOW_FIELD,
    &CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_0_GEM_FIELD,
    &CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_0_VALID_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_0 *****/
const ru_reg_rec CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_0_REG =
{
    "IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_0",
#if RU_INCLUDE_DESC
    "TRANSLATION_TBL 0..7 Register",
    "entry is comprised of:\n13 bit flow\n16 bit gem\n1  bit valid\n",
#endif
    { CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_0_REG_OFFSET },
    CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_0_REG_RAM_CNT,
    32,
    697,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_0_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_1, TYPE: Type_BBH_TX_BBHTX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: FLOW *****/
const ru_field_rec CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_1_FLOW_FIELD =
{
    "FLOW",
#if RU_INCLUDE_DESC
    "",
    "flow_index\n",
#endif
    { CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_1_FLOW_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_1_FLOW_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_1_FLOW_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: GEM *****/
const ru_field_rec CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_1_GEM_FIELD =
{
    "GEM",
#if RU_INCLUDE_DESC
    "",
    "gem port-id\n",
#endif
    { CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_1_GEM_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_1_GEM_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_1_GEM_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: VALID *****/
const ru_field_rec CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_1_VALID_FIELD =
{
    "VALID",
#if RU_INCLUDE_DESC
    "",
    "valid\n",
#endif
    { CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_1_VALID_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_1_VALID_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_1_VALID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_1_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_1_FLOW_FIELD,
    &CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_1_GEM_FIELD,
    &CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_1_VALID_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_1 *****/
const ru_reg_rec CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_1_REG =
{
    "IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_1",
#if RU_INCLUDE_DESC
    "TRANSLATION_TBL 0..7 Register",
    "entry is comprised of:\n13 bit flow\n16 bit gem\n1  bit valid\n",
#endif
    { CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_1_REG_OFFSET },
    CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_1_REG_RAM_CNT,
    32,
    698,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_1_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_2, TYPE: Type_BBH_TX_BBHTX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: FLOW *****/
const ru_field_rec CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_2_FLOW_FIELD =
{
    "FLOW",
#if RU_INCLUDE_DESC
    "",
    "flow_index\n",
#endif
    { CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_2_FLOW_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_2_FLOW_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_2_FLOW_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: GEM *****/
const ru_field_rec CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_2_GEM_FIELD =
{
    "GEM",
#if RU_INCLUDE_DESC
    "",
    "gem port-id\n",
#endif
    { CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_2_GEM_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_2_GEM_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_2_GEM_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: VALID *****/
const ru_field_rec CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_2_VALID_FIELD =
{
    "VALID",
#if RU_INCLUDE_DESC
    "",
    "valid\n",
#endif
    { CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_2_VALID_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_2_VALID_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_2_VALID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_2_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_2_FLOW_FIELD,
    &CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_2_GEM_FIELD,
    &CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_2_VALID_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_2 *****/
const ru_reg_rec CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_2_REG =
{
    "IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_2",
#if RU_INCLUDE_DESC
    "TRANSLATION_TBL 0..7 Register",
    "entry is comprised of:\n13 bit flow\n16 bit gem\n1  bit valid\n",
#endif
    { CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_2_REG_OFFSET },
    CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_2_REG_RAM_CNT,
    32,
    699,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_2_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_3, TYPE: Type_BBH_TX_BBHTX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: FLOW *****/
const ru_field_rec CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_3_FLOW_FIELD =
{
    "FLOW",
#if RU_INCLUDE_DESC
    "",
    "flow_index\n",
#endif
    { CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_3_FLOW_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_3_FLOW_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_3_FLOW_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: GEM *****/
const ru_field_rec CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_3_GEM_FIELD =
{
    "GEM",
#if RU_INCLUDE_DESC
    "",
    "gem port-id\n",
#endif
    { CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_3_GEM_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_3_GEM_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_3_GEM_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: VALID *****/
const ru_field_rec CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_3_VALID_FIELD =
{
    "VALID",
#if RU_INCLUDE_DESC
    "",
    "valid\n",
#endif
    { CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_3_VALID_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_3_VALID_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_3_VALID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_3_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_3_FLOW_FIELD,
    &CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_3_GEM_FIELD,
    &CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_3_VALID_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_3 *****/
const ru_reg_rec CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_3_REG =
{
    "IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_3",
#if RU_INCLUDE_DESC
    "TRANSLATION_TBL 0..7 Register",
    "entry is comprised of:\n13 bit flow\n16 bit gem\n1  bit valid\n",
#endif
    { CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_3_REG_OFFSET },
    CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_3_REG_RAM_CNT,
    32,
    700,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_3_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_4, TYPE: Type_BBH_TX_BBHTX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: FLOW *****/
const ru_field_rec CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_4_FLOW_FIELD =
{
    "FLOW",
#if RU_INCLUDE_DESC
    "",
    "flow_index\n",
#endif
    { CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_4_FLOW_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_4_FLOW_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_4_FLOW_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: GEM *****/
const ru_field_rec CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_4_GEM_FIELD =
{
    "GEM",
#if RU_INCLUDE_DESC
    "",
    "gem port-id\n",
#endif
    { CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_4_GEM_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_4_GEM_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_4_GEM_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: VALID *****/
const ru_field_rec CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_4_VALID_FIELD =
{
    "VALID",
#if RU_INCLUDE_DESC
    "",
    "valid\n",
#endif
    { CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_4_VALID_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_4_VALID_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_4_VALID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_4_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_4_FLOW_FIELD,
    &CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_4_GEM_FIELD,
    &CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_4_VALID_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_4 *****/
const ru_reg_rec CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_4_REG =
{
    "IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_4",
#if RU_INCLUDE_DESC
    "TRANSLATION_TBL 0..7 Register",
    "entry is comprised of:\n13 bit flow\n16 bit gem\n1  bit valid\n",
#endif
    { CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_4_REG_OFFSET },
    CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_4_REG_RAM_CNT,
    32,
    701,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_4_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_5, TYPE: Type_BBH_TX_BBHTX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: FLOW *****/
const ru_field_rec CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_5_FLOW_FIELD =
{
    "FLOW",
#if RU_INCLUDE_DESC
    "",
    "flow_index\n",
#endif
    { CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_5_FLOW_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_5_FLOW_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_5_FLOW_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: GEM *****/
const ru_field_rec CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_5_GEM_FIELD =
{
    "GEM",
#if RU_INCLUDE_DESC
    "",
    "gem port-id\n",
#endif
    { CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_5_GEM_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_5_GEM_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_5_GEM_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: VALID *****/
const ru_field_rec CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_5_VALID_FIELD =
{
    "VALID",
#if RU_INCLUDE_DESC
    "",
    "valid\n",
#endif
    { CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_5_VALID_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_5_VALID_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_5_VALID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_5_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_5_FLOW_FIELD,
    &CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_5_GEM_FIELD,
    &CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_5_VALID_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_5 *****/
const ru_reg_rec CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_5_REG =
{
    "IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_5",
#if RU_INCLUDE_DESC
    "TRANSLATION_TBL 0..7 Register",
    "entry is comprised of:\n13 bit flow\n16 bit gem\n1  bit valid\n",
#endif
    { CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_5_REG_OFFSET },
    CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_5_REG_RAM_CNT,
    32,
    702,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_5_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_6, TYPE: Type_BBH_TX_BBHTX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: FLOW *****/
const ru_field_rec CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_6_FLOW_FIELD =
{
    "FLOW",
#if RU_INCLUDE_DESC
    "",
    "flow_index\n",
#endif
    { CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_6_FLOW_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_6_FLOW_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_6_FLOW_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: GEM *****/
const ru_field_rec CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_6_GEM_FIELD =
{
    "GEM",
#if RU_INCLUDE_DESC
    "",
    "gem port-id\n",
#endif
    { CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_6_GEM_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_6_GEM_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_6_GEM_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: VALID *****/
const ru_field_rec CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_6_VALID_FIELD =
{
    "VALID",
#if RU_INCLUDE_DESC
    "",
    "valid\n",
#endif
    { CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_6_VALID_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_6_VALID_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_6_VALID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_6_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_6_FLOW_FIELD,
    &CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_6_GEM_FIELD,
    &CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_6_VALID_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_6 *****/
const ru_reg_rec CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_6_REG =
{
    "IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_6",
#if RU_INCLUDE_DESC
    "TRANSLATION_TBL 0..7 Register",
    "entry is comprised of:\n13 bit flow\n16 bit gem\n1  bit valid\n",
#endif
    { CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_6_REG_OFFSET },
    CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_6_REG_RAM_CNT,
    32,
    703,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_6_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_7, TYPE: Type_BBH_TX_BBHTX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: FLOW *****/
const ru_field_rec CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_7_FLOW_FIELD =
{
    "FLOW",
#if RU_INCLUDE_DESC
    "",
    "flow_index\n",
#endif
    { CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_7_FLOW_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_7_FLOW_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_7_FLOW_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: GEM *****/
const ru_field_rec CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_7_GEM_FIELD =
{
    "GEM",
#if RU_INCLUDE_DESC
    "",
    "gem port-id\n",
#endif
    { CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_7_GEM_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_7_GEM_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_7_GEM_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: VALID *****/
const ru_field_rec CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_7_VALID_FIELD =
{
    "VALID",
#if RU_INCLUDE_DESC
    "",
    "valid\n",
#endif
    { CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_7_VALID_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_7_VALID_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_7_VALID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_7_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_7_FLOW_FIELD,
    &CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_7_GEM_FIELD,
    &CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_7_VALID_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_7 *****/
const ru_reg_rec CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_7_REG =
{
    "IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_7",
#if RU_INCLUDE_DESC
    "TRANSLATION_TBL 0..7 Register",
    "entry is comprised of:\n13 bit flow\n16 bit gem\n1  bit valid\n",
#endif
    { CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_7_REG_OFFSET },
    CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_7_REG_RAM_CNT,
    32,
    704,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_7_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_GEM_TRANS_TBL_GEM_TRNS_VAL, TYPE: Type_BBH_TX_BBHTX_GEM_TRANS_TBL_GEM_TRNS_VAL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec CRDP_EGR_BBH_TX_GEM_TRANS_TBL_GEM_TRNS_VAL_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "val\n",
#endif
    { CRDP_EGR_BBH_TX_GEM_TRANS_TBL_GEM_TRNS_VAL_VAL_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_GEM_TRANS_TBL_GEM_TRNS_VAL_VAL_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_GEM_TRANS_TBL_GEM_TRNS_VAL_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_GEM_TRANS_TBL_GEM_TRNS_VAL_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_GEM_TRANS_TBL_GEM_TRNS_VAL_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_GEM_TRANS_TBL_GEM_TRNS_VAL *****/
const ru_reg_rec CRDP_EGR_BBH_TX_GEM_TRANS_TBL_GEM_TRNS_VAL_REG =
{
    "GEM_TRANS_TBL_GEM_TRNS_VAL",
#if RU_INCLUDE_DESC
    "GEM_TRANS_VAL 0..127 Register",
    "translation table for all 256 gems\n\nmemory has 64 entries, each has 64b for 4 gems per entry.\n\naddr=0: mem_idx=0,   [31:16]:gem1, [15:0]:gem0\naddr=4: mem_idx=0, : [63:48]:gem3, [47:32]:gem2\n\n...\n\naddr=4*126: mem_idx=63, [31:16]:gem253, [15:0]:gem252\naddr=4*127: mem_idx=63, [63:48]:gem255, [47:32]:gem254\n\n",
#endif
    { CRDP_EGR_BBH_TX_GEM_TRANS_TBL_GEM_TRNS_VAL_REG_OFFSET },
    CRDP_EGR_BBH_TX_GEM_TRANS_TBL_GEM_TRNS_VAL_REG_RAM_CNT,
    4,
    705,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_EGR_BBH_TX_GEM_TRANS_TBL_GEM_TRNS_VAL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CRDP_EGR_BBH_TX_MAC_FLOW_EN_TABLE_GEM_EN, TYPE: Type_BBH_TX_BBHTX_MAC_FLOW_EN_TABLE_GEM_EN
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ENABLE *****/
const ru_field_rec CRDP_EGR_BBH_TX_MAC_FLOW_EN_TABLE_GEM_EN_ENABLE_FIELD =
{
    "ENABLE",
#if RU_INCLUDE_DESC
    "",
    "enable\n",
#endif
    { CRDP_EGR_BBH_TX_MAC_FLOW_EN_TABLE_GEM_EN_ENABLE_FIELD_MASK },
    0,
    { CRDP_EGR_BBH_TX_MAC_FLOW_EN_TABLE_GEM_EN_ENABLE_FIELD_WIDTH },
    { CRDP_EGR_BBH_TX_MAC_FLOW_EN_TABLE_GEM_EN_ENABLE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_BBH_TX_MAC_FLOW_EN_TABLE_GEM_EN_FIELDS[] =
{
    &CRDP_EGR_BBH_TX_MAC_FLOW_EN_TABLE_GEM_EN_ENABLE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_BBH_TX_MAC_FLOW_EN_TABLE_GEM_EN *****/
const ru_reg_rec CRDP_EGR_BBH_TX_MAC_FLOW_EN_TABLE_GEM_EN_REG =
{
    "MAC_FLOW_EN_TABLE_GEM_EN",
#if RU_INCLUDE_DESC
    "GEM_EN 0..2047 Register",
    "enable bits for flows x to x+31\n\nPON has 2048 entries(only 8 are used in redwood)\n\nNNI has 512 entries.\n\nCopy : 0 entries\n",
#endif
    { CRDP_EGR_BBH_TX_MAC_FLOW_EN_TABLE_GEM_EN_REG_OFFSET },
    CRDP_EGR_BBH_TX_MAC_FLOW_EN_TABLE_GEM_EN_REG_RAM_CNT,
    4,
    706,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_EGR_BBH_TX_MAC_FLOW_EN_TABLE_GEM_EN_FIELDS,
#endif
};

unsigned long CRDP_EGR_BBH_TX_ADDRS[] =
{
    0xB2700000,
    0xB2710000,
    0xB2720000,
};

static const ru_reg_rec *CRDP_EGR_BBH_TX_REGS[] =
{
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MACTYPE_REG,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_BBCFG_TX_REG,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_REG,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_BBSRCID_REG,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_REG,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_PDRNRCFG_REG,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_REG,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_DBGSEL_REG,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_REG,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_GPR_REG,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_GENERAL_CFG_REG,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ECNCFG_REG,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_DDRFPMINIBASEL_REG,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_DDRFPMINIBASEH_REG,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_BBOVRDROUTE_REG,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_BBOVRDCTRL_REG,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_TMBASEL_REG,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_TMBASEH_REG,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_REORDERFIFOSIZE_REG,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_UBUSCREDITS_REG,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_CREDITINIT_REG,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MIRRORRNRCFG_REG,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MIRRORCFG_REG,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MULTIBBHCFG_REG,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_MULTIBBSRC_REG,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_ADDR_SPREADER_REG,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_FLOWCTRCFG_REG,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_FCRNRCFG_REG,
    &CRDP_EGR_BBH_TX_COMMON_CONFIGURATIONS_FCCFG_REG,
    &CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PDACK_REG,
    &CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PDSIZE_REG,
    &CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PDWKUPH_REG,
    &CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PDEMPTY_REG,
    &CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PD_BYTE_TH_REG,
    &CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_MAX_PD_TH_REG,
    &CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_PD_BYTE_TH_EN_REG,
    &CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FLOWTOGEMOFFSET_REG,
    &CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FLOWTOGEMTHR_REG,
    &CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FEBASE_REG,
    &CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FESIZE_REG,
    &CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FEPDBASE_REG,
    &CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FEPDSIZE_REG,
    &CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_SGMTWRR_REG,
    &CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_FE_CREDITS_REG,
    &CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_GEM_EN_INIT_REG,
    &CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_GEM_TRNS_INIT_REG,
    &CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_OTFBYTES_REG,
    &CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_OTFCMD_REG,
    &CRDP_EGR_BBH_TX_PER_PORT_CONFIGURATIONS_AUTOOTFCREDITS_REG,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_TXSRAM0PD_REG,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_TXSRAM1PD_REG,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_TXDDRPD_REG,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_PDDROP_REG,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_GETNEXTNULL_REG,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_GETNEXTDROP_REG,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_INPUTFIFODROP_REG,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SRAM0PKT_REG,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SRAM1PKT_REG,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_DDRPKT_REG,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_TXSRAM0BYTE_REG,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_TXSRAM1BYTE_REG,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_TXDDRBYTE_REG,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDEN_REG,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDADDR_REG,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SWRDDATA_REG,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_UNIFIEDPKT_REG,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_UNIFIEDBYTE_REG,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_DBGOUTREG_REG,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_IN_SEGMENTATION_REG,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_UNIFIED_DATA_CREDITS_REG,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_UNIFIED_PD_CREDITS_REG,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_DISFLOWDROPPKT_REG,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_DISFLOWDROPBYTE_REG,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_ECNPKT_REG,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_ECNBYTES_REG,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_ECNMARKERR_REG,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_ECNLENERR_REG,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_FLUSH_INFO_REG,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_FLUSH_INFO2_REG,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_PD_EH_MISMATCH_REG,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_PD_EH_MISMATCH_DATA_REG,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_OUTOFRANGE_ERRORS_REG,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_GEMDROPPKT_REG,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_GEMDROPBYTES_REG,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_CMDOTFDDR_REG,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_BYTESOTFDDR_REG,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_CMDOTFEPSRAM_REG,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_BYTESOTFEPSRAM_REG,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_GNEXT_REG,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_PDARRAY_REG,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_CURPD_REG,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SPACE4REQ_REG,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_SEGINFO_REG,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_EHDATVAL_REG,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_EHDAT_REG,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RMTPDOCC_REG,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RMTBYTES_REG,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RMTLBCNT_REG,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RESPSPD_REG,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RESPSDATA_REG,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RNRSMPDCNT_REG,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RNRSMBYTECNT_REG,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_PORTDBG_REG,
    &CRDP_EGR_BBH_TX_DEBUG_COUNTERS_RESPDBG_REG,
    &CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_0_REG,
    &CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_1_REG,
    &CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_2_REG,
    &CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_3_REG,
    &CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_4_REG,
    &CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_5_REG,
    &CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_6_REG,
    &CRDP_EGR_BBH_TX_IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_7_REG,
    &CRDP_EGR_BBH_TX_GEM_TRANS_TBL_GEM_TRNS_VAL_REG,
    &CRDP_EGR_BBH_TX_MAC_FLOW_EN_TABLE_GEM_EN_REG,
};

const ru_block_rec CRDP_EGR_BBH_TX_BLOCK =
{
    "CRDP_EGR_BBH_TX",
#if RU_INCLUDE_SIZE
    { 0x00004000 },
#endif
    CRDP_EGR_BBH_TX_ADDRS,
    3,
    114,
    CRDP_EGR_BBH_TX_REGS,
};
