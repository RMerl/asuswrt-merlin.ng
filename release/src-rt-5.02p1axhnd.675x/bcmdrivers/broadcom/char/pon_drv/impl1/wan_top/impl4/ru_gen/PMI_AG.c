/*
   Copyright (c) 2015 Broadcom Corporation
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
 * Field: PMI_LP_0_RESERVED0
 ******************************************************************************/
const ru_field_rec PMI_LP_0_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    PMI_LP_0_RESERVED0_FIELD_MASK,
    0,
    PMI_LP_0_RESERVED0_FIELD_WIDTH,
    PMI_LP_0_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: PMI_LP_0_CR_XGWAN_TOP_WAN_PCS_PMI_LP_EN
 ******************************************************************************/
const ru_field_rec PMI_LP_0_CR_XGWAN_TOP_WAN_PCS_PMI_LP_EN_FIELD =
{
    "CR_XGWAN_TOP_WAN_PCS_PMI_LP_EN",
#if RU_INCLUDE_DESC
    "",
    "Transaction enable control from master. This is treated as"
    "asynchronous to the PCS. The bus master should wait for"
    "pcs_pmi_lp_ack to be deasserted before pcs_pmi_lp_en is asserted."
    "The bus master should then wait for pcs_pmi_lp_ack to be asserted"
    "indicating that the transaction is complete before it deasserts"
    "pcs_pmi_lp_en.",
#endif
    PMI_LP_0_CR_XGWAN_TOP_WAN_PCS_PMI_LP_EN_FIELD_MASK,
    0,
    PMI_LP_0_CR_XGWAN_TOP_WAN_PCS_PMI_LP_EN_FIELD_WIDTH,
    PMI_LP_0_CR_XGWAN_TOP_WAN_PCS_PMI_LP_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: PMI_LP_0_CR_XGWAN_TOP_WAN_MISC_PMI_LP_EN
 ******************************************************************************/
const ru_field_rec PMI_LP_0_CR_XGWAN_TOP_WAN_MISC_PMI_LP_EN_FIELD =
{
    "CR_XGWAN_TOP_WAN_MISC_PMI_LP_EN",
#if RU_INCLUDE_DESC
    "",
    "Transaction enable control from master. This is treated as"
    "asynchronous to the rmic. The bus master should wait for pmi_lp_ack"
    "to be deasserted before pmi_lp_en is asserted. The bus master should"
    "then wait for pmi_lp_ack to be asserted indicating that the"
    "transaction is complete before it deasserts pmi_lp_en.",
#endif
    PMI_LP_0_CR_XGWAN_TOP_WAN_MISC_PMI_LP_EN_FIELD_MASK,
    0,
    PMI_LP_0_CR_XGWAN_TOP_WAN_MISC_PMI_LP_EN_FIELD_WIDTH,
    PMI_LP_0_CR_XGWAN_TOP_WAN_MISC_PMI_LP_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: PMI_LP_0_CR_XGWAN_TOP_WAN_MISC_PMI_LP_WRITE
 ******************************************************************************/
const ru_field_rec PMI_LP_0_CR_XGWAN_TOP_WAN_MISC_PMI_LP_WRITE_FIELD =
{
    "CR_XGWAN_TOP_WAN_MISC_PMI_LP_WRITE",
#if RU_INCLUDE_DESC
    "",
    "Read/Write control from master. 1-write, 0-read. This should be"
    "asserted before or with the pmi_lp_en and should be driven until the"
    "next transaction.",
#endif
    PMI_LP_0_CR_XGWAN_TOP_WAN_MISC_PMI_LP_WRITE_FIELD_MASK,
    0,
    PMI_LP_0_CR_XGWAN_TOP_WAN_MISC_PMI_LP_WRITE_FIELD_WIDTH,
    PMI_LP_0_CR_XGWAN_TOP_WAN_MISC_PMI_LP_WRITE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: PMI_LP_1_CR_XGWAN_TOP_WAN_MISC_PMI_LP_ADDR
 ******************************************************************************/
const ru_field_rec PMI_LP_1_CR_XGWAN_TOP_WAN_MISC_PMI_LP_ADDR_FIELD =
{
    "CR_XGWAN_TOP_WAN_MISC_PMI_LP_ADDR",
#if RU_INCLUDE_DESC
    "",
    "32-bit address driven by master for read or write transaction. This"
    "should be asserted before or with the pmi_lp_en and should be driven"
    "until the next transaction",
#endif
    PMI_LP_1_CR_XGWAN_TOP_WAN_MISC_PMI_LP_ADDR_FIELD_MASK,
    0,
    PMI_LP_1_CR_XGWAN_TOP_WAN_MISC_PMI_LP_ADDR_FIELD_WIDTH,
    PMI_LP_1_CR_XGWAN_TOP_WAN_MISC_PMI_LP_ADDR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: PMI_LP_2_CR_XGWAN_TOP_WAN_MISC_PMI_LP_WRDATA
 ******************************************************************************/
const ru_field_rec PMI_LP_2_CR_XGWAN_TOP_WAN_MISC_PMI_LP_WRDATA_FIELD =
{
    "CR_XGWAN_TOP_WAN_MISC_PMI_LP_WRDATA",
#if RU_INCLUDE_DESC
    "",
    "16-bit data bus driven by master for write transaction. This should"
    "be driven before or with the pmi_lp_en and should be driven until"
    "the next transaction.",
#endif
    PMI_LP_2_CR_XGWAN_TOP_WAN_MISC_PMI_LP_WRDATA_FIELD_MASK,
    0,
    PMI_LP_2_CR_XGWAN_TOP_WAN_MISC_PMI_LP_WRDATA_FIELD_WIDTH,
    PMI_LP_2_CR_XGWAN_TOP_WAN_MISC_PMI_LP_WRDATA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: PMI_LP_2_CR_XGWAN_TOP_WAN_MISC_PMI_LP_MASKDATA
 ******************************************************************************/
const ru_field_rec PMI_LP_2_CR_XGWAN_TOP_WAN_MISC_PMI_LP_MASKDATA_FIELD =
{
    "CR_XGWAN_TOP_WAN_MISC_PMI_LP_MASKDATA",
#if RU_INCLUDE_DESC
    "",
    "16-bit mask bus driven by master for write transaction. 0 means no"
    "mask (wrdata bit is written to register), 1 means mask (wrdata bit"
    "is ignored). This bus has no affect during a read operation. This"
    "should be asserted before or with the pmi_lp_en and should be driven"
    "until the next transaction.",
#endif
    PMI_LP_2_CR_XGWAN_TOP_WAN_MISC_PMI_LP_MASKDATA_FIELD_MASK,
    0,
    PMI_LP_2_CR_XGWAN_TOP_WAN_MISC_PMI_LP_MASKDATA_FIELD_WIDTH,
    PMI_LP_2_CR_XGWAN_TOP_WAN_MISC_PMI_LP_MASKDATA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: PMI_LP_3_RESERVED0
 ******************************************************************************/
const ru_field_rec PMI_LP_3_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    PMI_LP_3_RESERVED0_FIELD_MASK,
    0,
    PMI_LP_3_RESERVED0_FIELD_WIDTH,
    PMI_LP_3_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: PMI_LP_3_PMI_LP_ERR
 ******************************************************************************/
const ru_field_rec PMI_LP_3_PMI_LP_ERR_FIELD =
{
    "PMI_LP_ERR",
#if RU_INCLUDE_DESC
    "",
    "Error response from RMIC slave indicating an address error which"
    "means that either the block address does not exist or that the devid"
    "did not match the strap value. The ack signal indicates that the"
    "transaction is complete and the error signal indicates that there"
    "was an address error with this transaction. This signal is asserted"
    "along with the ack signal and should be treated an asynchronous"
    "signal the same way as the ack signal.",
#endif
    PMI_LP_3_PMI_LP_ERR_FIELD_MASK,
    0,
    PMI_LP_3_PMI_LP_ERR_FIELD_WIDTH,
    PMI_LP_3_PMI_LP_ERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: PMI_LP_3_PMI_LP_ACK
 ******************************************************************************/
const ru_field_rec PMI_LP_3_PMI_LP_ACK_FIELD =
{
    "PMI_LP_ACK",
#if RU_INCLUDE_DESC
    "",
    "Ack response back from the RMIC slave indicating that the write or"
    "read transaction is complete. This signal is driven in the registers"
    "blocks clock domain and should be treated as an asynchronous input"
    "by the master.",
#endif
    PMI_LP_3_PMI_LP_ACK_FIELD_MASK,
    0,
    PMI_LP_3_PMI_LP_ACK_FIELD_WIDTH,
    PMI_LP_3_PMI_LP_ACK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: PMI_LP_3_PMI_LP_RDDATA
 ******************************************************************************/
const ru_field_rec PMI_LP_3_PMI_LP_RDDATA_FIELD =
{
    "PMI_LP_RDDATA",
#if RU_INCLUDE_DESC
    "",
    "16-bit data bus driven RMIC slave during a read transaction. This"
    "data is latched in the register clock domain but this data is"
    "guaranteed to be stable by the end of the read transaction so this"
    "does not have to metastabilized.",
#endif
    PMI_LP_3_PMI_LP_RDDATA_FIELD_MASK,
    0,
    PMI_LP_3_PMI_LP_RDDATA_FIELD_WIDTH,
    PMI_LP_3_PMI_LP_RDDATA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: PMI_LP_4_RESERVED0
 ******************************************************************************/
const ru_field_rec PMI_LP_4_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    PMI_LP_4_RESERVED0_FIELD_MASK,
    0,
    PMI_LP_4_RESERVED0_FIELD_WIDTH,
    PMI_LP_4_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: PMI_LP_4_PCS_PMI_LP_ERR
 ******************************************************************************/
const ru_field_rec PMI_LP_4_PCS_PMI_LP_ERR_FIELD =
{
    "PCS_PMI_LP_ERR",
#if RU_INCLUDE_DESC
    "",
    "Error response from PCS slave indicating an address error which"
    "means that either the block address does not exist or that the devid"
    "did not match the strap value. The ack signal indicates that the"
    "transaction is complete and the error signal indicates that there"
    "was an address error with this transaction. This signal is asserted"
    "along with the ack signal and should be treated an asynchronous"
    "signal the same way as the ack signal.",
#endif
    PMI_LP_4_PCS_PMI_LP_ERR_FIELD_MASK,
    0,
    PMI_LP_4_PCS_PMI_LP_ERR_FIELD_WIDTH,
    PMI_LP_4_PCS_PMI_LP_ERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: PMI_LP_4_PCS_PMI_LP_ACK
 ******************************************************************************/
const ru_field_rec PMI_LP_4_PCS_PMI_LP_ACK_FIELD =
{
    "PCS_PMI_LP_ACK",
#if RU_INCLUDE_DESC
    "",
    "Ack response back from the PCS slave indicating that the write or"
    "read transaction is complete. This signal is driven in the registers"
    "blocks clock domain and should be treated as an asynchronous input"
    "by the master.",
#endif
    PMI_LP_4_PCS_PMI_LP_ACK_FIELD_MASK,
    0,
    PMI_LP_4_PCS_PMI_LP_ACK_FIELD_WIDTH,
    PMI_LP_4_PCS_PMI_LP_ACK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: PMI_LP_4_PCS_PMI_LP_RDDATA
 ******************************************************************************/
const ru_field_rec PMI_LP_4_PCS_PMI_LP_RDDATA_FIELD =
{
    "PCS_PMI_LP_RDDATA",
#if RU_INCLUDE_DESC
    "",
    "16-bit data bus driven PCS slave during a read transaction. This"
    "data is latched in the register clock domain but this data is"
    "guaranteed to be stable by the end of the read transaction so this"
    "does not have to metastabilized.",
#endif
    PMI_LP_4_PCS_PMI_LP_RDDATA_FIELD_MASK,
    0,
    PMI_LP_4_PCS_PMI_LP_RDDATA_FIELD_WIDTH,
    PMI_LP_4_PCS_PMI_LP_RDDATA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: PMI_LP_0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PMI_LP_0_FIELDS[] =
{
    &PMI_LP_0_RESERVED0_FIELD,
    &PMI_LP_0_CR_XGWAN_TOP_WAN_PCS_PMI_LP_EN_FIELD,
    &PMI_LP_0_CR_XGWAN_TOP_WAN_MISC_PMI_LP_EN_FIELD,
    &PMI_LP_0_CR_XGWAN_TOP_WAN_MISC_PMI_LP_WRITE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec PMI_LP_0_REG = 
{
    "LP_0",
#if RU_INCLUDE_DESC
    "WAN_TOP_PMI_LP_0 Register",
    "Register used for low priority configuration.",
#endif
    PMI_LP_0_REG_OFFSET,
    0,
    0,
    19,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    PMI_LP_0_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: PMI_LP_1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PMI_LP_1_FIELDS[] =
{
    &PMI_LP_1_CR_XGWAN_TOP_WAN_MISC_PMI_LP_ADDR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec PMI_LP_1_REG = 
{
    "LP_1",
#if RU_INCLUDE_DESC
    "WAN_TOP_PMI_LP_1 Register",
    "Register used for low priority configuration.",
#endif
    PMI_LP_1_REG_OFFSET,
    0,
    0,
    20,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    PMI_LP_1_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: PMI_LP_2
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PMI_LP_2_FIELDS[] =
{
    &PMI_LP_2_CR_XGWAN_TOP_WAN_MISC_PMI_LP_WRDATA_FIELD,
    &PMI_LP_2_CR_XGWAN_TOP_WAN_MISC_PMI_LP_MASKDATA_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec PMI_LP_2_REG = 
{
    "LP_2",
#if RU_INCLUDE_DESC
    "WAN_TOP_PMI_LP_2 Register",
    "Register used for low priority configuration.",
#endif
    PMI_LP_2_REG_OFFSET,
    0,
    0,
    21,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    PMI_LP_2_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: PMI_LP_3
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PMI_LP_3_FIELDS[] =
{
    &PMI_LP_3_RESERVED0_FIELD,
    &PMI_LP_3_PMI_LP_ERR_FIELD,
    &PMI_LP_3_PMI_LP_ACK_FIELD,
    &PMI_LP_3_PMI_LP_RDDATA_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec PMI_LP_3_REG = 
{
    "LP_3",
#if RU_INCLUDE_DESC
    "WAN_TOP_PMI_LP_3 Register",
    "Register used for low priority read back.",
#endif
    PMI_LP_3_REG_OFFSET,
    0,
    0,
    22,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    PMI_LP_3_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: PMI_LP_4
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PMI_LP_4_FIELDS[] =
{
    &PMI_LP_4_RESERVED0_FIELD,
    &PMI_LP_4_PCS_PMI_LP_ERR_FIELD,
    &PMI_LP_4_PCS_PMI_LP_ACK_FIELD,
    &PMI_LP_4_PCS_PMI_LP_RDDATA_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec PMI_LP_4_REG = 
{
    "LP_4",
#if RU_INCLUDE_DESC
    "WAN_TOP_PMI_LP_4 Register",
    "Register used for PCS low priority read back.",
#endif
    PMI_LP_4_REG_OFFSET,
    0,
    0,
    23,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    PMI_LP_4_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Block: PMI
 ******************************************************************************/
static const ru_reg_rec *PMI_REGS[] =
{
    &PMI_LP_0_REG,
    &PMI_LP_1_REG,
    &PMI_LP_2_REG,
    &PMI_LP_3_REG,
    &PMI_LP_4_REG,
};

unsigned long PMI_ADDRS[] =
{
    0x8014404c,
};

const ru_block_rec PMI_BLOCK = 
{
    "PMI",
    PMI_ADDRS,
    1,
    5,
    PMI_REGS
};

/* End of file PMI.c */
