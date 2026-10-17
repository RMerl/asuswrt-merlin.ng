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


#include "XRDP_GSU_MEM_AG.h"

/******************************************************************************
 * Register: NAME: GSU_MEM_INPUT_INFO_ENTRY, TYPE: Type_MEMORIES_GSU_INPUT_INFO_ENTRY
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec GSU_MEM_INPUT_INFO_ENTRY_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "value\n",
#endif
    { GSU_MEM_INPUT_INFO_ENTRY_VAL_FIELD_MASK },
    0,
    { GSU_MEM_INPUT_INFO_ENTRY_VAL_FIELD_WIDTH },
    { GSU_MEM_INPUT_INFO_ENTRY_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *GSU_MEM_INPUT_INFO_ENTRY_FIELDS[] =
{
    &GSU_MEM_INPUT_INFO_ENTRY_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: GSU_MEM_INPUT_INFO_ENTRY *****/
const ru_reg_rec GSU_MEM_INPUT_INFO_ENTRY_REG =
{
    "INPUT_INFO_ENTRY",
#if RU_INCLUDE_DESC
    "ENTRY 0..5119 Register",
    "Each InputInfo entry is 48-bit wide\n",
#endif
    { GSU_MEM_INPUT_INFO_ENTRY_REG_OFFSET },
    GSU_MEM_INPUT_INFO_ENTRY_REG_RAM_CNT,
    4,
    1505,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    GSU_MEM_INPUT_INFO_ENTRY_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: GSU_MEM_QUEUE_DATA_ENTRY, TYPE: Type_MEMORIES_GSU_QUEUE_DATA_ENTRY
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec GSU_MEM_QUEUE_DATA_ENTRY_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "value\n",
#endif
    { GSU_MEM_QUEUE_DATA_ENTRY_VAL_FIELD_MASK },
    0,
    { GSU_MEM_QUEUE_DATA_ENTRY_VAL_FIELD_WIDTH },
    { GSU_MEM_QUEUE_DATA_ENTRY_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *GSU_MEM_QUEUE_DATA_ENTRY_FIELDS[] =
{
    &GSU_MEM_QUEUE_DATA_ENTRY_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: GSU_MEM_QUEUE_DATA_ENTRY *****/
const ru_reg_rec GSU_MEM_QUEUE_DATA_ENTRY_REG =
{
    "QUEUE_DATA_ENTRY",
#if RU_INCLUDE_DESC
    "ENTRY 0..5119 Register",
    "Each QueueData entry is 128-bit wide.\nEach GSU is allocated space of 256 queues. For GSUs 0,2 - can use ether 129 queues in single mode or 256 queues in sharing mode.\nFor GSUs 1,3,4 - can use only 129 queues.\n",
#endif
    { GSU_MEM_QUEUE_DATA_ENTRY_REG_OFFSET },
    GSU_MEM_QUEUE_DATA_ENTRY_REG_RAM_CNT,
    4,
    1506,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    GSU_MEM_QUEUE_DATA_ENTRY_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: GSU_MEM_CONTEXT_DATA_ENTRY, TYPE: Type_MEMORIES_GSU_CONTEXT_DATA_ENTRY
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec GSU_MEM_CONTEXT_DATA_ENTRY_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "value\n",
#endif
    { GSU_MEM_CONTEXT_DATA_ENTRY_VAL_FIELD_MASK },
    0,
    { GSU_MEM_CONTEXT_DATA_ENTRY_VAL_FIELD_WIDTH },
    { GSU_MEM_CONTEXT_DATA_ENTRY_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *GSU_MEM_CONTEXT_DATA_ENTRY_FIELDS[] =
{
    &GSU_MEM_CONTEXT_DATA_ENTRY_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: GSU_MEM_CONTEXT_DATA_ENTRY *****/
const ru_reg_rec GSU_MEM_CONTEXT_DATA_ENTRY_REG =
{
    "CONTEXT_DATA_ENTRY",
#if RU_INCLUDE_DESC
    "ENTRY 0..8959 Register",
    "Each Context entry is 128-bit wide\n",
#endif
    { GSU_MEM_CONTEXT_DATA_ENTRY_REG_OFFSET },
    GSU_MEM_CONTEXT_DATA_ENTRY_REG_RAM_CNT,
    4,
    1507,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    GSU_MEM_CONTEXT_DATA_ENTRY_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: GSU_MEM_NODE_MASK_VECTOR_ENTRY, TYPE: Type_MEMORIES_NODE_MASK_VECTOR_ENTRY
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ENTRY_0_VAL *****/
const ru_field_rec GSU_MEM_NODE_MASK_VECTOR_ENTRY_ENTRY_0_VAL_FIELD =
{
    "ENTRY_0_VAL",
#if RU_INCLUDE_DESC
    "",
    "value\n",
#endif
    { GSU_MEM_NODE_MASK_VECTOR_ENTRY_ENTRY_0_VAL_FIELD_MASK },
    0,
    { GSU_MEM_NODE_MASK_VECTOR_ENTRY_ENTRY_0_VAL_FIELD_WIDTH },
    { GSU_MEM_NODE_MASK_VECTOR_ENTRY_ENTRY_0_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ENTRY_1_VAL *****/
const ru_field_rec GSU_MEM_NODE_MASK_VECTOR_ENTRY_ENTRY_1_VAL_FIELD =
{
    "ENTRY_1_VAL",
#if RU_INCLUDE_DESC
    "",
    "value\n",
#endif
    { GSU_MEM_NODE_MASK_VECTOR_ENTRY_ENTRY_1_VAL_FIELD_MASK },
    0,
    { GSU_MEM_NODE_MASK_VECTOR_ENTRY_ENTRY_1_VAL_FIELD_WIDTH },
    { GSU_MEM_NODE_MASK_VECTOR_ENTRY_ENTRY_1_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ENTRY_2_VAL *****/
const ru_field_rec GSU_MEM_NODE_MASK_VECTOR_ENTRY_ENTRY_2_VAL_FIELD =
{
    "ENTRY_2_VAL",
#if RU_INCLUDE_DESC
    "",
    "value\n",
#endif
    { GSU_MEM_NODE_MASK_VECTOR_ENTRY_ENTRY_2_VAL_FIELD_MASK },
    0,
    { GSU_MEM_NODE_MASK_VECTOR_ENTRY_ENTRY_2_VAL_FIELD_WIDTH },
    { GSU_MEM_NODE_MASK_VECTOR_ENTRY_ENTRY_2_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ENTRY_3_VAL *****/
const ru_field_rec GSU_MEM_NODE_MASK_VECTOR_ENTRY_ENTRY_3_VAL_FIELD =
{
    "ENTRY_3_VAL",
#if RU_INCLUDE_DESC
    "",
    "value\n",
#endif
    { GSU_MEM_NODE_MASK_VECTOR_ENTRY_ENTRY_3_VAL_FIELD_MASK },
    0,
    { GSU_MEM_NODE_MASK_VECTOR_ENTRY_ENTRY_3_VAL_FIELD_WIDTH },
    { GSU_MEM_NODE_MASK_VECTOR_ENTRY_ENTRY_3_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *GSU_MEM_NODE_MASK_VECTOR_ENTRY_FIELDS[] =
{
    &GSU_MEM_NODE_MASK_VECTOR_ENTRY_ENTRY_0_VAL_FIELD,
    &GSU_MEM_NODE_MASK_VECTOR_ENTRY_ENTRY_1_VAL_FIELD,
    &GSU_MEM_NODE_MASK_VECTOR_ENTRY_ENTRY_2_VAL_FIELD,
    &GSU_MEM_NODE_MASK_VECTOR_ENTRY_ENTRY_3_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: GSU_MEM_NODE_MASK_VECTOR_ENTRY *****/
const ru_reg_rec GSU_MEM_NODE_MASK_VECTOR_ENTRY_REG =
{
    "NODE_MASK_VECTOR_ENTRY",
#if RU_INCLUDE_DESC
    "ENTRY 0..4863 Register",
    "Each Node Mask vecitor is 32-bit wide\n",
#endif
    { GSU_MEM_NODE_MASK_VECTOR_ENTRY_REG_OFFSET },
    GSU_MEM_NODE_MASK_VECTOR_ENTRY_REG_RAM_CNT,
    4,
    1508,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    GSU_MEM_NODE_MASK_VECTOR_ENTRY_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: GSU_MEM_PD_FIFO_ENTRY, TYPE: Type_MEMORIES_PD_FIFO_ENTRY
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec GSU_MEM_PD_FIFO_ENTRY_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "value\n",
#endif
    { GSU_MEM_PD_FIFO_ENTRY_VAL_FIELD_MASK },
    0,
    { GSU_MEM_PD_FIFO_ENTRY_VAL_FIELD_WIDTH },
    { GSU_MEM_PD_FIFO_ENTRY_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *GSU_MEM_PD_FIFO_ENTRY_FIELDS[] =
{
    &GSU_MEM_PD_FIFO_ENTRY_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: GSU_MEM_PD_FIFO_ENTRY *****/
const ru_reg_rec GSU_MEM_PD_FIFO_ENTRY_REG =
{
    "PD_FIFO_ENTRY",
#if RU_INCLUDE_DESC
    "ENTRY 0..10239 Register",
    "Each PD FIFO entry is 64-bit wide.\nEach GSU is allocated space of 1024 PDs. For GSUs 0,2 - can use ether 516 PDs in single mode or 1024 PDs in sharing mode.\nFor GSUs 1,3,4 - can use only 516 PDs.\n",
#endif
    { GSU_MEM_PD_FIFO_ENTRY_REG_OFFSET },
    GSU_MEM_PD_FIFO_ENTRY_REG_RAM_CNT,
    4,
    1509,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    GSU_MEM_PD_FIFO_ENTRY_FIELDS,
#endif
};

unsigned long GSU_MEM_ADDRS[] =
{
    0xB2200000,
    0xB2400000,
};

static const ru_reg_rec *GSU_MEM_REGS[] =
{
    &GSU_MEM_PD_FIFO_ENTRY_REG,
    &GSU_MEM_QUEUE_DATA_ENTRY_REG,
    &GSU_MEM_NODE_MASK_VECTOR_ENTRY_REG,
    &GSU_MEM_CONTEXT_DATA_ENTRY_REG,
    &GSU_MEM_INPUT_INFO_ENTRY_REG,
};

const ru_block_rec GSU_MEM_BLOCK =
{
    "GSU_MEM",
#if RU_INCLUDE_SIZE
    { 0x00105000 },
#endif
    GSU_MEM_ADDRS,
    2,
    5,
    GSU_MEM_REGS,
};
