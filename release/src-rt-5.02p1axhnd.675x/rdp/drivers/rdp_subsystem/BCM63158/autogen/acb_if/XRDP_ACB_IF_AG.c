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
 * Field: ACB_IF_ACBIF_BLOCK_ACBIF_CONFIG_CONF0_CRC_ADD
 ******************************************************************************/
const ru_field_rec ACB_IF_ACBIF_BLOCK_ACBIF_CONFIG_CONF0_CRC_ADD_FIELD =
{
    "CRC_ADD",
#if RU_INCLUDE_DESC
    "crc_add",
    "add to the len of each packet 4B of crc",
#endif
    ACB_IF_ACBIF_BLOCK_ACBIF_CONFIG_CONF0_CRC_ADD_FIELD_MASK,
    0,
    ACB_IF_ACBIF_BLOCK_ACBIF_CONFIG_CONF0_CRC_ADD_FIELD_WIDTH,
    ACB_IF_ACBIF_BLOCK_ACBIF_CONFIG_CONF0_CRC_ADD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: ACB_IF_ACBIF_BLOCK_ACBIF_CONFIG_CONF0_RESERVED0
 ******************************************************************************/
const ru_field_rec ACB_IF_ACBIF_BLOCK_ACBIF_CONFIG_CONF0_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    ACB_IF_ACBIF_BLOCK_ACBIF_CONFIG_CONF0_RESERVED0_FIELD_MASK,
    0,
    ACB_IF_ACBIF_BLOCK_ACBIF_CONFIG_CONF0_RESERVED0_FIELD_WIDTH,
    ACB_IF_ACBIF_BLOCK_ACBIF_CONFIG_CONF0_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: ACB_IF_ACBIF_BLOCK_ACBIF_CONFIG_CONF0_VAL_LOC
 ******************************************************************************/
const ru_field_rec ACB_IF_ACBIF_BLOCK_ACBIF_CONFIG_CONF0_VAL_LOC_FIELD =
{
    "VAL_LOC",
#if RU_INCLUDE_DESC
    "valid_location",
    "location byte for the valid bit in the result(last bit in that byte):"
    "0: bit 7"
    "..."
    "7: bit 63",
#endif
    ACB_IF_ACBIF_BLOCK_ACBIF_CONFIG_CONF0_VAL_LOC_FIELD_MASK,
    0,
    ACB_IF_ACBIF_BLOCK_ACBIF_CONFIG_CONF0_VAL_LOC_FIELD_WIDTH,
    ACB_IF_ACBIF_BLOCK_ACBIF_CONFIG_CONF0_VAL_LOC_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: ACB_IF_ACBIF_BLOCK_ACBIF_CONFIG_CONF0_RESERVED1
 ******************************************************************************/
const ru_field_rec ACB_IF_ACBIF_BLOCK_ACBIF_CONFIG_CONF0_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    ACB_IF_ACBIF_BLOCK_ACBIF_CONFIG_CONF0_RESERVED1_FIELD_MASK,
    0,
    ACB_IF_ACBIF_BLOCK_ACBIF_CONFIG_CONF0_RESERVED1_FIELD_WIDTH,
    ACB_IF_ACBIF_BLOCK_ACBIF_CONFIG_CONF0_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_CMD_TYPE_VAL
 ******************************************************************************/
const ru_field_rec ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_CMD_TYPE_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "value",
    "value",
#endif
    ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_CMD_TYPE_VAL_FIELD_MASK,
    0,
    ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_CMD_TYPE_VAL_FIELD_WIDTH,
    ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_CMD_TYPE_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_CMD_IMP_VAL
 ******************************************************************************/
const ru_field_rec ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_CMD_IMP_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "value",
    "value",
#endif
    ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_CMD_IMP_VAL_FIELD_MASK,
    0,
    ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_CMD_IMP_VAL_FIELD_WIDTH,
    ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_CMD_IMP_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_AGG_VAL
 ******************************************************************************/
const ru_field_rec ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_AGG_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "value",
    "value",
#endif
    ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_AGG_VAL_FIELD_MASK,
    0,
    ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_AGG_VAL_FIELD_WIDTH,
    ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_AGG_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_BUFFS_VAL
 ******************************************************************************/
const ru_field_rec ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_BUFFS_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "value",
    "value",
#endif
    ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_BUFFS_VAL_FIELD_MASK,
    0,
    ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_BUFFS_VAL_FIELD_WIDTH,
    ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_BUFFS_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_GEN_CFG_RD_CLR
 ******************************************************************************/
const ru_field_rec ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_GEN_CFG_RD_CLR_FIELD =
{
    "RD_CLR",
#if RU_INCLUDE_DESC
    "rd_clr",
    "read clear bit",
#endif
    ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_GEN_CFG_RD_CLR_FIELD_MASK,
    0,
    ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_GEN_CFG_RD_CLR_FIELD_WIDTH,
    ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_GEN_CFG_RD_CLR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_GEN_CFG_WRAP
 ******************************************************************************/
const ru_field_rec ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_GEN_CFG_WRAP_FIELD =
{
    "WRAP",
#if RU_INCLUDE_DESC
    "wrap",
    "read clear bit",
#endif
    ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_GEN_CFG_WRAP_FIELD_MASK,
    0,
    ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_GEN_CFG_WRAP_FIELD_WIDTH,
    ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_GEN_CFG_WRAP_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_GEN_CFG_RESERVED0
 ******************************************************************************/
const ru_field_rec ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_GEN_CFG_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_GEN_CFG_RESERVED0_FIELD_MASK,
    0,
    ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_GEN_CFG_RESERVED0_FIELD_WIDTH,
    ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_GEN_CFG_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: ACB_IF_ACBIF_BLOCK_ACBIF_DEBUG_DBGSEL_VS
 ******************************************************************************/
const ru_field_rec ACB_IF_ACBIF_BLOCK_ACBIF_DEBUG_DBGSEL_VS_FIELD =
{
    "VS",
#if RU_INCLUDE_DESC
    "vector_select",
    "selects th debug vector",
#endif
    ACB_IF_ACBIF_BLOCK_ACBIF_DEBUG_DBGSEL_VS_FIELD_MASK,
    0,
    ACB_IF_ACBIF_BLOCK_ACBIF_DEBUG_DBGSEL_VS_FIELD_WIDTH,
    ACB_IF_ACBIF_BLOCK_ACBIF_DEBUG_DBGSEL_VS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: ACB_IF_ACBIF_BLOCK_ACBIF_DEBUG_DBGSEL_RESERVED0
 ******************************************************************************/
const ru_field_rec ACB_IF_ACBIF_BLOCK_ACBIF_DEBUG_DBGSEL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    ACB_IF_ACBIF_BLOCK_ACBIF_DEBUG_DBGSEL_RESERVED0_FIELD_MASK,
    0,
    ACB_IF_ACBIF_BLOCK_ACBIF_DEBUG_DBGSEL_RESERVED0_FIELD_WIDTH,
    ACB_IF_ACBIF_BLOCK_ACBIF_DEBUG_DBGSEL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: ACB_IF_ACBIF_BLOCK_ACBIF_DEBUG_DBGBUS_VAL
 ******************************************************************************/
const ru_field_rec ACB_IF_ACBIF_BLOCK_ACBIF_DEBUG_DBGBUS_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "value",
    "value",
#endif
    ACB_IF_ACBIF_BLOCK_ACBIF_DEBUG_DBGBUS_VAL_FIELD_MASK,
    0,
    ACB_IF_ACBIF_BLOCK_ACBIF_DEBUG_DBGBUS_VAL_FIELD_WIDTH,
    ACB_IF_ACBIF_BLOCK_ACBIF_DEBUG_DBGBUS_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: ACB_IF_ACBIF_BLOCK_ACBIF_DEBUG_DBGBUS_RESERVED0
 ******************************************************************************/
const ru_field_rec ACB_IF_ACBIF_BLOCK_ACBIF_DEBUG_DBGBUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    ACB_IF_ACBIF_BLOCK_ACBIF_DEBUG_DBGBUS_RESERVED0_FIELD_MASK,
    0,
    ACB_IF_ACBIF_BLOCK_ACBIF_DEBUG_DBGBUS_RESERVED0_FIELD_WIDTH,
    ACB_IF_ACBIF_BLOCK_ACBIF_DEBUG_DBGBUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: ACB_IF_ACBIF_BLOCK_ACBIF_DEBUG_STAT_VAL
 ******************************************************************************/
const ru_field_rec ACB_IF_ACBIF_BLOCK_ACBIF_DEBUG_STAT_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "value",
    "value",
#endif
    ACB_IF_ACBIF_BLOCK_ACBIF_DEBUG_STAT_VAL_FIELD_MASK,
    0,
    ACB_IF_ACBIF_BLOCK_ACBIF_DEBUG_STAT_VAL_FIELD_WIDTH,
    ACB_IF_ACBIF_BLOCK_ACBIF_DEBUG_STAT_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: ACB_IF_ACBIF_BLOCK_ACBIF_CONFIG_CONF0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *ACB_IF_ACBIF_BLOCK_ACBIF_CONFIG_CONF0_FIELDS[] =
{
    &ACB_IF_ACBIF_BLOCK_ACBIF_CONFIG_CONF0_CRC_ADD_FIELD,
    &ACB_IF_ACBIF_BLOCK_ACBIF_CONFIG_CONF0_RESERVED0_FIELD,
    &ACB_IF_ACBIF_BLOCK_ACBIF_CONFIG_CONF0_VAL_LOC_FIELD,
    &ACB_IF_ACBIF_BLOCK_ACBIF_CONFIG_CONF0_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec ACB_IF_ACBIF_BLOCK_ACBIF_CONFIG_CONF0_REG = 
{
    "ACBIF_BLOCK_ACBIF_CONFIG_CONF0",
#if RU_INCLUDE_DESC
    "CONFIG0 Register",
    "misc configs 0",
#endif
    ACB_IF_ACBIF_BLOCK_ACBIF_CONFIG_CONF0_REG_OFFSET,
    0,
    0,
    961,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    ACB_IF_ACBIF_BLOCK_ACBIF_CONFIG_CONF0_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_CMD_TYPE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_CMD_TYPE_FIELDS[] =
{
    &ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_CMD_TYPE_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_CMD_TYPE_REG = 
{
    "ACBIF_BLOCK_ACBIF_PM_COUNTERS_CMD_TYPE",
#if RU_INCLUDE_DESC
    "CMD_TYPE_CNTR %i Register",
    "Number of commands that were processed for each command type (order - intend, sent, stat).",
#endif
    ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_CMD_TYPE_REG_OFFSET,
    ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_CMD_TYPE_REG_RAM_CNT,
    4,
    962,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_CMD_TYPE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_CMD_IMP
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_CMD_IMP_FIELDS[] =
{
    &ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_CMD_IMP_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_CMD_IMP_REG = 
{
    "ACBIF_BLOCK_ACBIF_PM_COUNTERS_CMD_IMP",
#if RU_INCLUDE_DESC
    "CMD_IMP_CNTR %i Register",
    "Number of commands that were processed for each IMP(0,1,2).e.",
#endif
    ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_CMD_IMP_REG_OFFSET,
    ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_CMD_IMP_REG_RAM_CNT,
    4,
    963,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_CMD_IMP_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_AGG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_AGG_FIELDS[] =
{
    &ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_AGG_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_AGG_REG = 
{
    "ACBIF_BLOCK_ACBIF_PM_COUNTERS_AGG",
#if RU_INCLUDE_DESC
    "AGG_CNTR %i Register",
    "Number of commands (for each of - intend, sent) that were for aggregated packets.",
#endif
    ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_AGG_REG_OFFSET,
    ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_AGG_REG_RAM_CNT,
    4,
    964,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_AGG_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_BUFFS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_BUFFS_FIELDS[] =
{
    &ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_BUFFS_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_BUFFS_REG = 
{
    "ACBIF_BLOCK_ACBIF_PM_COUNTERS_BUFFS",
#if RU_INCLUDE_DESC
    "BUFS_NUM_CNTR %i Register",
    "Number of buffers that were counted for each command(order - intend, sent).",
#endif
    ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_BUFFS_REG_OFFSET,
    ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_BUFFS_REG_RAM_CNT,
    4,
    965,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_BUFFS_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_GEN_CFG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_GEN_CFG_FIELDS[] =
{
    &ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_GEN_CFG_RD_CLR_FIELD,
    &ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_GEN_CFG_WRAP_FIELD,
    &ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_GEN_CFG_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_GEN_CFG_REG = 
{
    "ACBIF_BLOCK_ACBIF_PM_COUNTERS_GEN_CFG",
#if RU_INCLUDE_DESC
    "GENERAL_CONFIG Register",
    "bits rd_clr and wrap for the pm counters(above)",
#endif
    ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_GEN_CFG_REG_OFFSET,
    0,
    0,
    966,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_GEN_CFG_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: ACB_IF_ACBIF_BLOCK_ACBIF_DEBUG_DBGSEL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *ACB_IF_ACBIF_BLOCK_ACBIF_DEBUG_DBGSEL_FIELDS[] =
{
    &ACB_IF_ACBIF_BLOCK_ACBIF_DEBUG_DBGSEL_VS_FIELD,
    &ACB_IF_ACBIF_BLOCK_ACBIF_DEBUG_DBGSEL_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec ACB_IF_ACBIF_BLOCK_ACBIF_DEBUG_DBGSEL_REG = 
{
    "ACBIF_BLOCK_ACBIF_DEBUG_DBGSEL",
#if RU_INCLUDE_DESC
    "DBG_MUX_SEL Register",
    "selects the debug vecore",
#endif
    ACB_IF_ACBIF_BLOCK_ACBIF_DEBUG_DBGSEL_REG_OFFSET,
    0,
    0,
    967,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    ACB_IF_ACBIF_BLOCK_ACBIF_DEBUG_DBGSEL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: ACB_IF_ACBIF_BLOCK_ACBIF_DEBUG_DBGBUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *ACB_IF_ACBIF_BLOCK_ACBIF_DEBUG_DBGBUS_FIELDS[] =
{
    &ACB_IF_ACBIF_BLOCK_ACBIF_DEBUG_DBGBUS_VAL_FIELD,
    &ACB_IF_ACBIF_BLOCK_ACBIF_DEBUG_DBGBUS_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec ACB_IF_ACBIF_BLOCK_ACBIF_DEBUG_DBGBUS_REG = 
{
    "ACBIF_BLOCK_ACBIF_DEBUG_DBGBUS",
#if RU_INCLUDE_DESC
    "DBG_BUS Register",
    "the debug bus",
#endif
    ACB_IF_ACBIF_BLOCK_ACBIF_DEBUG_DBGBUS_REG_OFFSET,
    0,
    0,
    968,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    ACB_IF_ACBIF_BLOCK_ACBIF_DEBUG_DBGBUS_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: ACB_IF_ACBIF_BLOCK_ACBIF_DEBUG_STAT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *ACB_IF_ACBIF_BLOCK_ACBIF_DEBUG_STAT_FIELDS[] =
{
    &ACB_IF_ACBIF_BLOCK_ACBIF_DEBUG_STAT_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec ACB_IF_ACBIF_BLOCK_ACBIF_DEBUG_STAT_REG = 
{
    "ACBIF_BLOCK_ACBIF_DEBUG_STAT",
#if RU_INCLUDE_DESC
    "STATUS %i Register",
    "status register (msb, lsb)",
#endif
    ACB_IF_ACBIF_BLOCK_ACBIF_DEBUG_STAT_REG_OFFSET,
    ACB_IF_ACBIF_BLOCK_ACBIF_DEBUG_STAT_REG_RAM_CNT,
    4,
    969,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    ACB_IF_ACBIF_BLOCK_ACBIF_DEBUG_STAT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Block: ACB_IF
 ******************************************************************************/
static const ru_reg_rec *ACB_IF_REGS[] =
{
    &ACB_IF_ACBIF_BLOCK_ACBIF_CONFIG_CONF0_REG,
    &ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_CMD_TYPE_REG,
    &ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_CMD_IMP_REG,
    &ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_AGG_REG,
    &ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_BUFFS_REG,
    &ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_GEN_CFG_REG,
    &ACB_IF_ACBIF_BLOCK_ACBIF_DEBUG_DBGSEL_REG,
    &ACB_IF_ACBIF_BLOCK_ACBIF_DEBUG_DBGBUS_REG,
    &ACB_IF_ACBIF_BLOCK_ACBIF_DEBUG_STAT_REG,
};

unsigned long ACB_IF_ADDRS[] =
{
    0x82e50800,
};

const ru_block_rec ACB_IF_BLOCK = 
{
    "ACB_IF",
    ACB_IF_ADDRS,
    1,
    9,
    ACB_IF_REGS
};

/* End of file XRDP_ACB_IF.c */
