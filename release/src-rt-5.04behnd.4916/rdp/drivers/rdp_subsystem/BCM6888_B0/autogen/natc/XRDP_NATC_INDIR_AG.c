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


#include "XRDP_NATC_INDIR_AG.h"

/******************************************************************************
 * Register: NAME: NATC_INDIR_ADDR_REG, TYPE: Type_NATC_INDIR_ADDR_REG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: NATC_ENTRY *****/
const ru_field_rec NATC_INDIR_ADDR_REG_NATC_ENTRY_FIELD =
{
    "NATC_ENTRY",
#if RU_INCLUDE_DESC
    "",
    "NAT Cache Entry number.\n",
#endif
    { NATC_INDIR_ADDR_REG_NATC_ENTRY_FIELD_MASK },
    0,
    { NATC_INDIR_ADDR_REG_NATC_ENTRY_FIELD_WIDTH },
    { NATC_INDIR_ADDR_REG_NATC_ENTRY_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: W_R *****/
const ru_field_rec NATC_INDIR_ADDR_REG_W_R_FIELD =
{
    "W_R",
#if RU_INCLUDE_DESC
    "",
    "NAT Cache Memory and Statics Memory Transaction.1 : NAT Cache Memory and Statics Memory Write.0 : NAT Cache Memory and Statics Memory Read.\n",
#endif
    { NATC_INDIR_ADDR_REG_W_R_FIELD_MASK },
    0,
    { NATC_INDIR_ADDR_REG_W_R_FIELD_WIDTH },
    { NATC_INDIR_ADDR_REG_W_R_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NATC_INDIR_ADDR_REG_FIELDS[] =
{
    &NATC_INDIR_ADDR_REG_NATC_ENTRY_FIELD,
    &NATC_INDIR_ADDR_REG_W_R_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NATC_INDIR_ADDR_REG *****/
const ru_reg_rec NATC_INDIR_ADDR_REG_REG =
{
    "ADDR_REG",
#if RU_INCLUDE_DESC
    "NATC Indirect Address Register",
    "",
#endif
    { NATC_INDIR_ADDR_REG_REG_OFFSET },
    0,
    0,
    622,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    NATC_INDIR_ADDR_REG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NATC_INDIR_DATA_REG, TYPE: Type_NATC_INDIR_DATA_REG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec NATC_INDIR_DATA_REG_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "Indirect register access data register, bits[31:0].-------------------------------------------------------For NAT Cache nd Statics Memory write operation,first, write all the data to Indirect Data Registers[N-1:0],N is number of words including key, result, hit count and byte count.Indirect Data Register[1:0] are for Statics Memory.Indirect Data register[0] is for hit count and 4 lsb of byte count.Indirect Data register[0] bits 27:0 are 28-bit hit count.Indirect Data register[0] bits 31:28 are 4 lsb of 36-bit byte count.Indirect Data Register[1] is for 32 msb of 36-bit byte count.{Indirect Data Register[1], Indirect Data register[0][31:28]} is the 36-bit byte count.indirect Data register [N-1:2] are for NAT Cache Memory (key and result), key is first,followed by result, followed by {ddr_miss, nat_ddr_bin, nat_tbl}then followed by a write to Indirect Address Register to set upNAT Cache Entry Number and W_R bit to 1, this will initiate the write operation.--------------------------------------------------------For NAT Cache Memory and statics Memory read operation,first, write to Indirect Address Register to set upNAT Cache Entry Number and W_R bit to 0, this will initiate the read operation.the read data from NAT Cache Memory and statics Memory will be loaded into Indirect Data Registers[N-1:0].then followed by read from Indirect Data Registers[N-1:0] for all data.\n",
#endif
    { NATC_INDIR_DATA_REG_DATA_FIELD_MASK },
    0,
    { NATC_INDIR_DATA_REG_DATA_FIELD_WIDTH },
    { NATC_INDIR_DATA_REG_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NATC_INDIR_DATA_REG_FIELDS[] =
{
    &NATC_INDIR_DATA_REG_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NATC_INDIR_DATA_REG *****/
const ru_reg_rec NATC_INDIR_DATA_REG_REG =
{
    "DATA_REG",
#if RU_INCLUDE_DESC
    "MATC Indirect Data Register",
    "",
#endif
    { NATC_INDIR_DATA_REG_REG_OFFSET },
    NATC_INDIR_DATA_REG_REG_RAM_CNT,
    4,
    623,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    NATC_INDIR_DATA_REG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NATC_INDIR_NATC_FLOW_CNTR_INDIR_ADDR_REG, TYPE: Type_NATC_FLOW_CNTR_INDIR_ADDR_REG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: FLOW_CNTR_ENTRY *****/
const ru_field_rec NATC_INDIR_NATC_FLOW_CNTR_INDIR_ADDR_REG_FLOW_CNTR_ENTRY_FIELD =
{
    "FLOW_CNTR_ENTRY",
#if RU_INCLUDE_DESC
    "",
    "Flow counter address.\n",
#endif
    { NATC_INDIR_NATC_FLOW_CNTR_INDIR_ADDR_REG_FLOW_CNTR_ENTRY_FIELD_MASK },
    0,
    { NATC_INDIR_NATC_FLOW_CNTR_INDIR_ADDR_REG_FLOW_CNTR_ENTRY_FIELD_WIDTH },
    { NATC_INDIR_NATC_FLOW_CNTR_INDIR_ADDR_REG_FLOW_CNTR_ENTRY_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: W_R *****/
const ru_field_rec NATC_INDIR_NATC_FLOW_CNTR_INDIR_ADDR_REG_W_R_FIELD =
{
    "W_R",
#if RU_INCLUDE_DESC
    "",
    "Flow counter Transaction.1 : Flow counter write.0 : Flow counter read.\n",
#endif
    { NATC_INDIR_NATC_FLOW_CNTR_INDIR_ADDR_REG_W_R_FIELD_MASK },
    0,
    { NATC_INDIR_NATC_FLOW_CNTR_INDIR_ADDR_REG_W_R_FIELD_WIDTH },
    { NATC_INDIR_NATC_FLOW_CNTR_INDIR_ADDR_REG_W_R_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NATC_INDIR_NATC_FLOW_CNTR_INDIR_ADDR_REG_FIELDS[] =
{
    &NATC_INDIR_NATC_FLOW_CNTR_INDIR_ADDR_REG_FLOW_CNTR_ENTRY_FIELD,
    &NATC_INDIR_NATC_FLOW_CNTR_INDIR_ADDR_REG_W_R_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NATC_INDIR_NATC_FLOW_CNTR_INDIR_ADDR_REG *****/
const ru_reg_rec NATC_INDIR_NATC_FLOW_CNTR_INDIR_ADDR_REG_REG =
{
    "NATC_FLOW_CNTR_INDIR_ADDR_REG",
#if RU_INCLUDE_DESC
    "NATC Flow Counter Indirect Address Register",
    "",
#endif
    { NATC_INDIR_NATC_FLOW_CNTR_INDIR_ADDR_REG_REG_OFFSET },
    0,
    0,
    624,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    NATC_INDIR_NATC_FLOW_CNTR_INDIR_ADDR_REG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NATC_INDIR_NATC_FLOW_CNTR_INDIR_DATA_REG, TYPE: Type_NATC_FLOW_CNTR_INDIR_DATA_REG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec NATC_INDIR_NATC_FLOW_CNTR_INDIR_DATA_REG_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "Flow Counter indirect register access data register, bits[31:0].-------------------------------------------------------For Flow Counter write operation,first, write all the data to Flow Counter Indirect Data Registers[1:0],Flow Counter Indirect Data register[0] is for flow Counter hit count and 4 lsb of Flow Counter byte count.Flow Counter Indirect Data register[0] bits 27:0 are 28-bit Flow Counter hit count.Flow Counter Indirect Data register[0] bits 31:28 are 4 lsb of 36-bit Flow Counter byte count.Flow Counter Indirect Data Register[1] is for 32 msb of 36-bit Flow Counter byte count.{Flow Counter Indirect Data Register[1], Flow Counter Indirect Data register[0][31:28]} is the 36-bit Flow Counter byte count.then followed by a write to Flow Counter Indirect Address Register to set upFlow Counter adress and W_R bit to 1, this will initiate the write operation.--------------------------------------------------------For Flow Counter read operation,first, write to Flow Counter Indirect Address Register to set upFlow Counter address and W_R bit to 0, this will initiate the read operation.the read data from Flow Counter will be loaded into Flow Counter Indirect Data Registers[1:0].then followed by read from Flow Counter Indirect Data Registers[1:0] for all data.\n",
#endif
    { NATC_INDIR_NATC_FLOW_CNTR_INDIR_DATA_REG_DATA_FIELD_MASK },
    0,
    { NATC_INDIR_NATC_FLOW_CNTR_INDIR_DATA_REG_DATA_FIELD_WIDTH },
    { NATC_INDIR_NATC_FLOW_CNTR_INDIR_DATA_REG_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NATC_INDIR_NATC_FLOW_CNTR_INDIR_DATA_REG_FIELDS[] =
{
    &NATC_INDIR_NATC_FLOW_CNTR_INDIR_DATA_REG_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NATC_INDIR_NATC_FLOW_CNTR_INDIR_DATA_REG *****/
const ru_reg_rec NATC_INDIR_NATC_FLOW_CNTR_INDIR_DATA_REG_REG =
{
    "NATC_FLOW_CNTR_INDIR_DATA_REG",
#if RU_INCLUDE_DESC
    "MATC Flow Counter Indirect Data Register",
    "",
#endif
    { NATC_INDIR_NATC_FLOW_CNTR_INDIR_DATA_REG_REG_OFFSET },
    NATC_INDIR_NATC_FLOW_CNTR_INDIR_DATA_REG_REG_RAM_CNT,
    4,
    625,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    NATC_INDIR_NATC_FLOW_CNTR_INDIR_DATA_REG_FIELDS,
#endif
};

unsigned long NATC_INDIR_ADDRS[] =
{
    0x82950700,
};

static const ru_reg_rec *NATC_INDIR_REGS[] =
{
    &NATC_INDIR_ADDR_REG_REG,
    &NATC_INDIR_DATA_REG_REG,
    &NATC_INDIR_NATC_FLOW_CNTR_INDIR_ADDR_REG_REG,
    &NATC_INDIR_NATC_FLOW_CNTR_INDIR_DATA_REG_REG,
};

const ru_block_rec NATC_INDIR_BLOCK =
{
    "NATC_INDIR",
    NATC_INDIR_ADDRS,
    1,
    4,
    NATC_INDIR_REGS,
};
