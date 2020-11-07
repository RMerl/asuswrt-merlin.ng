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
 * Field: UBUS_MSTR_EN_EN
 ******************************************************************************/
const ru_field_rec UBUS_MSTR_EN_EN_FIELD =
{
    "EN",
#if RU_INCLUDE_DESC
    "BRDG_ENABLE",
    "bridge enable",
#endif
    UBUS_MSTR_EN_EN_FIELD_MASK,
    0,
    UBUS_MSTR_EN_EN_FIELD_WIDTH,
    UBUS_MSTR_EN_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_MSTR_EN_RESERVED0
 ******************************************************************************/
const ru_field_rec UBUS_MSTR_EN_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UBUS_MSTR_EN_RESERVED0_FIELD_MASK,
    0,
    UBUS_MSTR_EN_RESERVED0_FIELD_WIDTH,
    UBUS_MSTR_EN_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_MSTR_REQ_CNTRL_PKT_ID
 ******************************************************************************/
const ru_field_rec UBUS_MSTR_REQ_CNTRL_PKT_ID_FIELD =
{
    "PKT_ID",
#if RU_INCLUDE_DESC
    "packet_id",
    "ID that can be added to a packet",
#endif
    UBUS_MSTR_REQ_CNTRL_PKT_ID_FIELD_MASK,
    0,
    UBUS_MSTR_REQ_CNTRL_PKT_ID_FIELD_WIDTH,
    UBUS_MSTR_REQ_CNTRL_PKT_ID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_MSTR_REQ_CNTRL_PKT_TAG
 ******************************************************************************/
const ru_field_rec UBUS_MSTR_REQ_CNTRL_PKT_TAG_FIELD =
{
    "PKT_TAG",
#if RU_INCLUDE_DESC
    "packet_tag",
    "enable packet tagging",
#endif
    UBUS_MSTR_REQ_CNTRL_PKT_TAG_FIELD_MASK,
    0,
    UBUS_MSTR_REQ_CNTRL_PKT_TAG_FIELD_WIDTH,
    UBUS_MSTR_REQ_CNTRL_PKT_TAG_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_MSTR_REQ_CNTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec UBUS_MSTR_REQ_CNTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UBUS_MSTR_REQ_CNTRL_RESERVED0_FIELD_MASK,
    0,
    UBUS_MSTR_REQ_CNTRL_RESERVED0_FIELD_WIDTH,
    UBUS_MSTR_REQ_CNTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_MSTR_REQ_CNTRL_ENDIAN_MODE
 ******************************************************************************/
const ru_field_rec UBUS_MSTR_REQ_CNTRL_ENDIAN_MODE_FIELD =
{
    "ENDIAN_MODE",
#if RU_INCLUDE_DESC
    "endian_mode",
    "endian mode of the requester",
#endif
    UBUS_MSTR_REQ_CNTRL_ENDIAN_MODE_FIELD_MASK,
    0,
    UBUS_MSTR_REQ_CNTRL_ENDIAN_MODE_FIELD_WIDTH,
    UBUS_MSTR_REQ_CNTRL_ENDIAN_MODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_MSTR_REQ_CNTRL_REPIN_ESWAP
 ******************************************************************************/
const ru_field_rec UBUS_MSTR_REQ_CNTRL_REPIN_ESWAP_FIELD =
{
    "REPIN_ESWAP",
#if RU_INCLUDE_DESC
    "repin_eswap",
    "repin endian swap",
#endif
    UBUS_MSTR_REQ_CNTRL_REPIN_ESWAP_FIELD_MASK,
    0,
    UBUS_MSTR_REQ_CNTRL_REPIN_ESWAP_FIELD_WIDTH,
    UBUS_MSTR_REQ_CNTRL_REPIN_ESWAP_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_MSTR_REQ_CNTRL_REQOUT_ESWAP
 ******************************************************************************/
const ru_field_rec UBUS_MSTR_REQ_CNTRL_REQOUT_ESWAP_FIELD =
{
    "REQOUT_ESWAP",
#if RU_INCLUDE_DESC
    "reqout_eswap",
    "reqout endian swap",
#endif
    UBUS_MSTR_REQ_CNTRL_REQOUT_ESWAP_FIELD_MASK,
    0,
    UBUS_MSTR_REQ_CNTRL_REQOUT_ESWAP_FIELD_WIDTH,
    UBUS_MSTR_REQ_CNTRL_REQOUT_ESWAP_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_MSTR_REQ_CNTRL_DEV_ERR
 ******************************************************************************/
const ru_field_rec UBUS_MSTR_REQ_CNTRL_DEV_ERR_FIELD =
{
    "DEV_ERR",
#if RU_INCLUDE_DESC
    "dev_error",
    "indicate an error on Ubus",
#endif
    UBUS_MSTR_REQ_CNTRL_DEV_ERR_FIELD_MASK,
    0,
    UBUS_MSTR_REQ_CNTRL_DEV_ERR_FIELD_WIDTH,
    UBUS_MSTR_REQ_CNTRL_DEV_ERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_MSTR_REQ_CNTRL_RESERVED1
 ******************************************************************************/
const ru_field_rec UBUS_MSTR_REQ_CNTRL_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UBUS_MSTR_REQ_CNTRL_RESERVED1_FIELD_MASK,
    0,
    UBUS_MSTR_REQ_CNTRL_RESERVED1_FIELD_WIDTH,
    UBUS_MSTR_REQ_CNTRL_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_MSTR_REQ_CNTRL_MAX_PKT_LEN
 ******************************************************************************/
const ru_field_rec UBUS_MSTR_REQ_CNTRL_MAX_PKT_LEN_FIELD =
{
    "MAX_PKT_LEN",
#if RU_INCLUDE_DESC
    "Max_Packet_len",
    "Max packet len that the bridge can support",
#endif
    UBUS_MSTR_REQ_CNTRL_MAX_PKT_LEN_FIELD_MASK,
    0,
    UBUS_MSTR_REQ_CNTRL_MAX_PKT_LEN_FIELD_WIDTH,
    UBUS_MSTR_REQ_CNTRL_MAX_PKT_LEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_MSTR_HYST_CTRL_CMD_SPACE
 ******************************************************************************/
const ru_field_rec UBUS_MSTR_HYST_CTRL_CMD_SPACE_FIELD =
{
    "CMD_SPACE",
#if RU_INCLUDE_DESC
    "CMD_SPACE",
    "command space indication that controls the ARdy signal."
    ""
    "Once the HSPACE indication is lower than CMD_SPACE the ARdy will be deasserted",
#endif
    UBUS_MSTR_HYST_CTRL_CMD_SPACE_FIELD_MASK,
    0,
    UBUS_MSTR_HYST_CTRL_CMD_SPACE_FIELD_WIDTH,
    UBUS_MSTR_HYST_CTRL_CMD_SPACE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_MSTR_HYST_CTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec UBUS_MSTR_HYST_CTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UBUS_MSTR_HYST_CTRL_RESERVED0_FIELD_MASK,
    0,
    UBUS_MSTR_HYST_CTRL_RESERVED0_FIELD_WIDTH,
    UBUS_MSTR_HYST_CTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_MSTR_HYST_CTRL_DATA_SPACE
 ******************************************************************************/
const ru_field_rec UBUS_MSTR_HYST_CTRL_DATA_SPACE_FIELD =
{
    "DATA_SPACE",
#if RU_INCLUDE_DESC
    "DATA_SPACE",
    "data space indication that controls the ARdy signal."
    ""
    "Once the DSPACE indication is lower than DATA_SPACE the ARdy will be deasserted",
#endif
    UBUS_MSTR_HYST_CTRL_DATA_SPACE_FIELD_MASK,
    0,
    UBUS_MSTR_HYST_CTRL_DATA_SPACE_FIELD_WIDTH,
    UBUS_MSTR_HYST_CTRL_DATA_SPACE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_MSTR_HYST_CTRL_RESERVED1
 ******************************************************************************/
const ru_field_rec UBUS_MSTR_HYST_CTRL_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UBUS_MSTR_HYST_CTRL_RESERVED1_FIELD_MASK,
    0,
    UBUS_MSTR_HYST_CTRL_RESERVED1_FIELD_WIDTH,
    UBUS_MSTR_HYST_CTRL_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_MSTR_HP_HP_EN
 ******************************************************************************/
const ru_field_rec UBUS_MSTR_HP_HP_EN_FIELD =
{
    "HP_EN",
#if RU_INCLUDE_DESC
    "hp_en",
    "enables the hp mechanism",
#endif
    UBUS_MSTR_HP_HP_EN_FIELD_MASK,
    0,
    UBUS_MSTR_HP_HP_EN_FIELD_WIDTH,
    UBUS_MSTR_HP_HP_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_MSTR_HP_HP_SEL
 ******************************************************************************/
const ru_field_rec UBUS_MSTR_HP_HP_SEL_FIELD =
{
    "HP_SEL",
#if RU_INCLUDE_DESC
    "hp_sel",
    "selects between external control and internal control of the HP bit",
#endif
    UBUS_MSTR_HP_HP_SEL_FIELD_MASK,
    0,
    UBUS_MSTR_HP_HP_SEL_FIELD_WIDTH,
    UBUS_MSTR_HP_HP_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_MSTR_HP_HP_COMB
 ******************************************************************************/
const ru_field_rec UBUS_MSTR_HP_HP_COMB_FIELD =
{
    "HP_COMB",
#if RU_INCLUDE_DESC
    "hp_combine",
    "combines both internal and external HP control (OR between them)",
#endif
    UBUS_MSTR_HP_HP_COMB_FIELD_MASK,
    0,
    UBUS_MSTR_HP_HP_COMB_FIELD_WIDTH,
    UBUS_MSTR_HP_HP_COMB_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_MSTR_HP_RESERVED0
 ******************************************************************************/
const ru_field_rec UBUS_MSTR_HP_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UBUS_MSTR_HP_RESERVED0_FIELD_MASK,
    0,
    UBUS_MSTR_HP_RESERVED0_FIELD_WIDTH,
    UBUS_MSTR_HP_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_MSTR_HP_HP_CNT_HIGH
 ******************************************************************************/
const ru_field_rec UBUS_MSTR_HP_HP_CNT_HIGH_FIELD =
{
    "HP_CNT_HIGH",
#if RU_INCLUDE_DESC
    "hp_cnt_high",
    "counter will count according to this setting the amount of cycles the HP will be asserted in the internal mech",
#endif
    UBUS_MSTR_HP_HP_CNT_HIGH_FIELD_MASK,
    0,
    UBUS_MSTR_HP_HP_CNT_HIGH_FIELD_WIDTH,
    UBUS_MSTR_HP_HP_CNT_HIGH_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_MSTR_HP_RESERVED1
 ******************************************************************************/
const ru_field_rec UBUS_MSTR_HP_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UBUS_MSTR_HP_RESERVED1_FIELD_MASK,
    0,
    UBUS_MSTR_HP_RESERVED1_FIELD_WIDTH,
    UBUS_MSTR_HP_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_MSTR_HP_HP_CNT_TOTAL
 ******************************************************************************/
const ru_field_rec UBUS_MSTR_HP_HP_CNT_TOTAL_FIELD =
{
    "HP_CNT_TOTAL",
#if RU_INCLUDE_DESC
    "hp_cnt_total",
    "includes both asserted and deasserted cycles of the HP counter. can control with hp_cnt_high the frequnecy of the HP assertion",
#endif
    UBUS_MSTR_HP_HP_CNT_TOTAL_FIELD_MASK,
    0,
    UBUS_MSTR_HP_HP_CNT_TOTAL_FIELD_WIDTH,
    UBUS_MSTR_HP_HP_CNT_TOTAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_MSTR_HP_RESERVED2
 ******************************************************************************/
const ru_field_rec UBUS_MSTR_HP_RESERVED2_FIELD =
{
    "RESERVED2",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UBUS_MSTR_HP_RESERVED2_FIELD_MASK,
    0,
    UBUS_MSTR_HP_RESERVED2_FIELD_WIDTH,
    UBUS_MSTR_HP_RESERVED2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_MSTR_REPLY_ADD_ADD
 ******************************************************************************/
const ru_field_rec UBUS_MSTR_REPLY_ADD_ADD_FIELD =
{
    "ADD",
#if RU_INCLUDE_DESC
    "address",
    "address value used for the read reply."
    "a read command with this address will be terminated in the bridge",
#endif
    UBUS_MSTR_REPLY_ADD_ADD_FIELD_MASK,
    0,
    UBUS_MSTR_REPLY_ADD_ADD_FIELD_WIDTH,
    UBUS_MSTR_REPLY_ADD_ADD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_MSTR_REPLY_DATA_DATA
 ******************************************************************************/
const ru_field_rec UBUS_MSTR_REPLY_DATA_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "data",
    "holds the data value for the read reply command. the data held in this register will be returned to runner",
#endif
    UBUS_MSTR_REPLY_DATA_DATA_FIELD_MASK,
    0,
    UBUS_MSTR_REPLY_DATA_DATA_FIELD_WIDTH,
    UBUS_MSTR_REPLY_DATA_DATA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: UBUS_MSTR_EN
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_MSTR_EN_FIELDS[] =
{
    &UBUS_MSTR_EN_EN_FIELD,
    &UBUS_MSTR_EN_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UBUS_MSTR_EN_REG = 
{
    "EN",
#if RU_INCLUDE_DESC
    "BRDG_EN Register",
    "bridge enable",
#endif
    UBUS_MSTR_EN_REG_OFFSET,
    0,
    0,
    694,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    UBUS_MSTR_EN_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UBUS_MSTR_REQ_CNTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_MSTR_REQ_CNTRL_FIELDS[] =
{
    &UBUS_MSTR_REQ_CNTRL_PKT_ID_FIELD,
    &UBUS_MSTR_REQ_CNTRL_PKT_TAG_FIELD,
    &UBUS_MSTR_REQ_CNTRL_RESERVED0_FIELD,
    &UBUS_MSTR_REQ_CNTRL_ENDIAN_MODE_FIELD,
    &UBUS_MSTR_REQ_CNTRL_REPIN_ESWAP_FIELD,
    &UBUS_MSTR_REQ_CNTRL_REQOUT_ESWAP_FIELD,
    &UBUS_MSTR_REQ_CNTRL_DEV_ERR_FIELD,
    &UBUS_MSTR_REQ_CNTRL_RESERVED1_FIELD,
    &UBUS_MSTR_REQ_CNTRL_MAX_PKT_LEN_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UBUS_MSTR_REQ_CNTRL_REG = 
{
    "REQ_CNTRL",
#if RU_INCLUDE_DESC
    "RQUSTOR_CTRL Register",
    "Requestor side contol. These registers are releated to ubus requestor control",
#endif
    UBUS_MSTR_REQ_CNTRL_REG_OFFSET,
    0,
    0,
    695,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    UBUS_MSTR_REQ_CNTRL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UBUS_MSTR_HYST_CTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_MSTR_HYST_CTRL_FIELDS[] =
{
    &UBUS_MSTR_HYST_CTRL_CMD_SPACE_FIELD,
    &UBUS_MSTR_HYST_CTRL_RESERVED0_FIELD,
    &UBUS_MSTR_HYST_CTRL_DATA_SPACE_FIELD,
    &UBUS_MSTR_HYST_CTRL_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UBUS_MSTR_HYST_CTRL_REG = 
{
    "HYST_CTRL",
#if RU_INCLUDE_DESC
    "HYST_CTRL Register",
    "control the command / data queue full and empty indications.",
#endif
    UBUS_MSTR_HYST_CTRL_REG_OFFSET,
    0,
    0,
    696,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    UBUS_MSTR_HYST_CTRL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UBUS_MSTR_HP
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_MSTR_HP_FIELDS[] =
{
    &UBUS_MSTR_HP_HP_EN_FIELD,
    &UBUS_MSTR_HP_HP_SEL_FIELD,
    &UBUS_MSTR_HP_HP_COMB_FIELD,
    &UBUS_MSTR_HP_RESERVED0_FIELD,
    &UBUS_MSTR_HP_HP_CNT_HIGH_FIELD,
    &UBUS_MSTR_HP_RESERVED1_FIELD,
    &UBUS_MSTR_HP_HP_CNT_TOTAL_FIELD,
    &UBUS_MSTR_HP_RESERVED2_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UBUS_MSTR_HP_REG = 
{
    "HP",
#if RU_INCLUDE_DESC
    "HIGH_PRIORITY Register",
    "controls the high priority mechanism",
#endif
    UBUS_MSTR_HP_REG_OFFSET,
    0,
    0,
    697,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    UBUS_MSTR_HP_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UBUS_MSTR_REPLY_ADD
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_MSTR_REPLY_ADD_FIELDS[] =
{
    &UBUS_MSTR_REPLY_ADD_ADD_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UBUS_MSTR_REPLY_ADD_REG = 
{
    "REPLY_ADD",
#if RU_INCLUDE_DESC
    "REPLY_ADDRESS Register",
    "holds the termination address used for the read reply command",
#endif
    UBUS_MSTR_REPLY_ADD_REG_OFFSET,
    0,
    0,
    698,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UBUS_MSTR_REPLY_ADD_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UBUS_MSTR_REPLY_DATA
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_MSTR_REPLY_DATA_FIELDS[] =
{
    &UBUS_MSTR_REPLY_DATA_DATA_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UBUS_MSTR_REPLY_DATA_REG = 
{
    "REPLY_DATA",
#if RU_INCLUDE_DESC
    "REPLY_DATA Register",
    "holds the data value for the read reply command. the data held in this register will be returned to runner",
#endif
    UBUS_MSTR_REPLY_DATA_REG_OFFSET,
    0,
    0,
    699,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UBUS_MSTR_REPLY_DATA_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Block: UBUS_MSTR
 ******************************************************************************/
static const ru_reg_rec *UBUS_MSTR_REGS[] =
{
    &UBUS_MSTR_EN_REG,
    &UBUS_MSTR_REQ_CNTRL_REG,
    &UBUS_MSTR_HYST_CTRL_REG,
    &UBUS_MSTR_HP_REG,
    &UBUS_MSTR_REPLY_ADD_REG,
    &UBUS_MSTR_REPLY_DATA_REG,
};

unsigned long UBUS_MSTR_ADDRS[] =
{
    0x82d96000,
};

const ru_block_rec UBUS_MSTR_BLOCK = 
{
    "UBUS_MSTR",
    UBUS_MSTR_ADDRS,
    1,
    6,
    UBUS_MSTR_REGS
};

/* End of file XRDP_UBUS_MSTR.c */
