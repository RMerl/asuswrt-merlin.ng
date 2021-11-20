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
 * Field: LPORT_SRDS_DUAL_SERDES_REVISION_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_SRDS_DUAL_SERDES_REVISION_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_SRDS_DUAL_SERDES_REVISION_RESERVED0_FIELD_MASK,
    0,
    LPORT_SRDS_DUAL_SERDES_REVISION_RESERVED0_FIELD_WIDTH,
    LPORT_SRDS_DUAL_SERDES_REVISION_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_SRDS_DUAL_SERDES_REVISION_SERDES_REV
 ******************************************************************************/
const ru_field_rec LPORT_SRDS_DUAL_SERDES_REVISION_SERDES_REV_FIELD =
{
    "SERDES_REV",
#if RU_INCLUDE_DESC
    "",
    "SERDES Revision Control Register.",
#endif
    LPORT_SRDS_DUAL_SERDES_REVISION_SERDES_REV_FIELD_MASK,
    0,
    LPORT_SRDS_DUAL_SERDES_REVISION_SERDES_REV_FIELD_WIDTH,
    LPORT_SRDS_DUAL_SERDES_REVISION_SERDES_REV_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_SRDS_SERDES_0_INDIR_ACC_CNTRL_0_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_SRDS_SERDES_0_INDIR_ACC_CNTRL_0_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_SRDS_SERDES_0_INDIR_ACC_CNTRL_0_RESERVED0_FIELD_MASK,
    0,
    LPORT_SRDS_SERDES_0_INDIR_ACC_CNTRL_0_RESERVED0_FIELD_WIDTH,
    LPORT_SRDS_SERDES_0_INDIR_ACC_CNTRL_0_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_SRDS_SERDES_0_INDIR_ACC_CNTRL_0_ERR
 ******************************************************************************/
const ru_field_rec LPORT_SRDS_SERDES_0_INDIR_ACC_CNTRL_0_ERR_FIELD =
{
    "ERR",
#if RU_INCLUDE_DESC
    "",
    "Transaction Status. When transaction completes "
    "(START_BUSY = 0 after it was set to 1) and this bit is set "
    "it indicates that register transaction completed with error.",
#endif
    LPORT_SRDS_SERDES_0_INDIR_ACC_CNTRL_0_ERR_FIELD_MASK,
    0,
    LPORT_SRDS_SERDES_0_INDIR_ACC_CNTRL_0_ERR_FIELD_WIDTH,
    LPORT_SRDS_SERDES_0_INDIR_ACC_CNTRL_0_ERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_SRDS_SERDES_0_INDIR_ACC_CNTRL_0_START_BUSY
 ******************************************************************************/
const ru_field_rec LPORT_SRDS_SERDES_0_INDIR_ACC_CNTRL_0_START_BUSY_FIELD =
{
    "START_BUSY",
#if RU_INCLUDE_DESC
    "",
    "START_BUSY, Self-clearing. CPU writes this bit to 1 in order to initiate indirect register "
    "read/write transaction. When transaction completes hardware clears this bit.",
#endif
    LPORT_SRDS_SERDES_0_INDIR_ACC_CNTRL_0_START_BUSY_FIELD_MASK,
    0,
    LPORT_SRDS_SERDES_0_INDIR_ACC_CNTRL_0_START_BUSY_FIELD_WIDTH,
    LPORT_SRDS_SERDES_0_INDIR_ACC_CNTRL_0_START_BUSY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_SRDS_SERDES_0_INDIR_ACC_CNTRL_0_R_W
 ******************************************************************************/
const ru_field_rec LPORT_SRDS_SERDES_0_INDIR_ACC_CNTRL_0_R_W_FIELD =
{
    "R_W",
#if RU_INCLUDE_DESC
    "",
    "Register transaction:\n"
    "0 : Register Write.\n'"
    "1 : Register Read.\n",
#endif
    LPORT_SRDS_SERDES_0_INDIR_ACC_CNTRL_0_R_W_FIELD_MASK,
    0,
    LPORT_SRDS_SERDES_0_INDIR_ACC_CNTRL_0_R_W_FIELD_WIDTH,
    LPORT_SRDS_SERDES_0_INDIR_ACC_CNTRL_0_R_W_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_SRDS_SERDES_0_INDIR_ACC_CNTRL_0_REG_DATA
 ******************************************************************************/
const ru_field_rec LPORT_SRDS_SERDES_0_INDIR_ACC_CNTRL_0_REG_DATA_FIELD =
{
    "REG_DATA",
#if RU_INCLUDE_DESC
    "",
    "Register READ/WRITE Data.",
#endif
    LPORT_SRDS_SERDES_0_INDIR_ACC_CNTRL_0_REG_DATA_FIELD_MASK,
    0,
    LPORT_SRDS_SERDES_0_INDIR_ACC_CNTRL_0_REG_DATA_FIELD_WIDTH,
    LPORT_SRDS_SERDES_0_INDIR_ACC_CNTRL_0_REG_DATA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_SRDS_SERDES_0_INDIR_ACC_ADDR_0_REG_ADDR
 ******************************************************************************/
const ru_field_rec LPORT_SRDS_SERDES_0_INDIR_ACC_ADDR_0_REG_ADDR_FIELD =
{
    "REG_ADDR",
#if RU_INCLUDE_DESC
    "",
    "Indirect register access, address register.",
#endif
    LPORT_SRDS_SERDES_0_INDIR_ACC_ADDR_0_REG_ADDR_FIELD_MASK,
    0,
    LPORT_SRDS_SERDES_0_INDIR_ACC_ADDR_0_REG_ADDR_FIELD_WIDTH,
    LPORT_SRDS_SERDES_0_INDIR_ACC_ADDR_0_REG_ADDR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_SRDS_SERDES_0_INDIR_ACC_MASK_0_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_SRDS_SERDES_0_INDIR_ACC_MASK_0_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_SRDS_SERDES_0_INDIR_ACC_MASK_0_RESERVED0_FIELD_MASK,
    0,
    LPORT_SRDS_SERDES_0_INDIR_ACC_MASK_0_RESERVED0_FIELD_WIDTH,
    LPORT_SRDS_SERDES_0_INDIR_ACC_MASK_0_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_SRDS_SERDES_0_INDIR_ACC_MASK_0_REG_MASK
 ******************************************************************************/
const ru_field_rec LPORT_SRDS_SERDES_0_INDIR_ACC_MASK_0_REG_MASK_FIELD =
{
    "REG_MASK",
#if RU_INCLUDE_DESC
    "",
    "Indirect register access, data mask register."
    "When a bit in this mask is set (1'b1), writing to the corresponding data bit is disabled.",
#endif
    LPORT_SRDS_SERDES_0_INDIR_ACC_MASK_0_REG_MASK_FIELD_MASK,
    0,
    LPORT_SRDS_SERDES_0_INDIR_ACC_MASK_0_REG_MASK_FIELD_WIDTH,
    LPORT_SRDS_SERDES_0_INDIR_ACC_MASK_0_REG_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_SRDS_SERDES_0_INDIR_ACC_CNTRL_1_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_SRDS_SERDES_0_INDIR_ACC_CNTRL_1_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_SRDS_SERDES_0_INDIR_ACC_CNTRL_1_RESERVED0_FIELD_MASK,
    0,
    LPORT_SRDS_SERDES_0_INDIR_ACC_CNTRL_1_RESERVED0_FIELD_WIDTH,
    LPORT_SRDS_SERDES_0_INDIR_ACC_CNTRL_1_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_SRDS_SERDES_0_INDIR_ACC_CNTRL_1_ERR
 ******************************************************************************/
const ru_field_rec LPORT_SRDS_SERDES_0_INDIR_ACC_CNTRL_1_ERR_FIELD =
{
    "ERR",
#if RU_INCLUDE_DESC
    "",
    "Transaction Status. When transaction completes "
    "(START_BUSY = 0 after it was set to 1) and this bit is set "
    "it indicates that register transaction completed with error.",
#endif
    LPORT_SRDS_SERDES_0_INDIR_ACC_CNTRL_1_ERR_FIELD_MASK,
    0,
    LPORT_SRDS_SERDES_0_INDIR_ACC_CNTRL_1_ERR_FIELD_WIDTH,
    LPORT_SRDS_SERDES_0_INDIR_ACC_CNTRL_1_ERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_SRDS_SERDES_0_INDIR_ACC_CNTRL_1_START_BUSY
 ******************************************************************************/
const ru_field_rec LPORT_SRDS_SERDES_0_INDIR_ACC_CNTRL_1_START_BUSY_FIELD =
{
    "START_BUSY",
#if RU_INCLUDE_DESC
    "",
    "START_BUSY, Self-clearing. CPU writes this bit to 1 in order to initiate indirect register "
    "read/write transaction. When transaction completes hardware clears this bit.",
#endif
    LPORT_SRDS_SERDES_0_INDIR_ACC_CNTRL_1_START_BUSY_FIELD_MASK,
    0,
    LPORT_SRDS_SERDES_0_INDIR_ACC_CNTRL_1_START_BUSY_FIELD_WIDTH,
    LPORT_SRDS_SERDES_0_INDIR_ACC_CNTRL_1_START_BUSY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_SRDS_SERDES_0_INDIR_ACC_CNTRL_1_R_W
 ******************************************************************************/
const ru_field_rec LPORT_SRDS_SERDES_0_INDIR_ACC_CNTRL_1_R_W_FIELD =
{
    "R_W",
#if RU_INCLUDE_DESC
    "",
    "Register transaction:\n"
    "0 : Register Write.\n'"
    "1 : Register Read.\n",
#endif
    LPORT_SRDS_SERDES_0_INDIR_ACC_CNTRL_1_R_W_FIELD_MASK,
    0,
    LPORT_SRDS_SERDES_0_INDIR_ACC_CNTRL_1_R_W_FIELD_WIDTH,
    LPORT_SRDS_SERDES_0_INDIR_ACC_CNTRL_1_R_W_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_SRDS_SERDES_0_INDIR_ACC_CNTRL_1_REG_DATA
 ******************************************************************************/
const ru_field_rec LPORT_SRDS_SERDES_0_INDIR_ACC_CNTRL_1_REG_DATA_FIELD =
{
    "REG_DATA",
#if RU_INCLUDE_DESC
    "",
    "Register READ/WRITE Data.",
#endif
    LPORT_SRDS_SERDES_0_INDIR_ACC_CNTRL_1_REG_DATA_FIELD_MASK,
    0,
    LPORT_SRDS_SERDES_0_INDIR_ACC_CNTRL_1_REG_DATA_FIELD_WIDTH,
    LPORT_SRDS_SERDES_0_INDIR_ACC_CNTRL_1_REG_DATA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_SRDS_SERDES_0_INDIR_ACC_ADDR_1_REG_ADDR
 ******************************************************************************/
const ru_field_rec LPORT_SRDS_SERDES_0_INDIR_ACC_ADDR_1_REG_ADDR_FIELD =
{
    "REG_ADDR",
#if RU_INCLUDE_DESC
    "",
    "Indirect register access, address register.",
#endif
    LPORT_SRDS_SERDES_0_INDIR_ACC_ADDR_1_REG_ADDR_FIELD_MASK,
    0,
    LPORT_SRDS_SERDES_0_INDIR_ACC_ADDR_1_REG_ADDR_FIELD_WIDTH,
    LPORT_SRDS_SERDES_0_INDIR_ACC_ADDR_1_REG_ADDR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_SRDS_SERDES_0_INDIR_ACC_MASK_1_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_SRDS_SERDES_0_INDIR_ACC_MASK_1_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_SRDS_SERDES_0_INDIR_ACC_MASK_1_RESERVED0_FIELD_MASK,
    0,
    LPORT_SRDS_SERDES_0_INDIR_ACC_MASK_1_RESERVED0_FIELD_WIDTH,
    LPORT_SRDS_SERDES_0_INDIR_ACC_MASK_1_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_SRDS_SERDES_0_INDIR_ACC_MASK_1_REG_MASK
 ******************************************************************************/
const ru_field_rec LPORT_SRDS_SERDES_0_INDIR_ACC_MASK_1_REG_MASK_FIELD =
{
    "REG_MASK",
#if RU_INCLUDE_DESC
    "",
    "Indirect register access, data mask register."
    "When a bit in this mask is set (1'b1), writing to the corresponding data bit is disabled.",
#endif
    LPORT_SRDS_SERDES_0_INDIR_ACC_MASK_1_REG_MASK_FIELD_MASK,
    0,
    LPORT_SRDS_SERDES_0_INDIR_ACC_MASK_1_REG_MASK_FIELD_WIDTH,
    LPORT_SRDS_SERDES_0_INDIR_ACC_MASK_1_REG_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_SRDS_SERDES_1_INDIR_ACC_CNTRL_0_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_SRDS_SERDES_1_INDIR_ACC_CNTRL_0_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_SRDS_SERDES_1_INDIR_ACC_CNTRL_0_RESERVED0_FIELD_MASK,
    0,
    LPORT_SRDS_SERDES_1_INDIR_ACC_CNTRL_0_RESERVED0_FIELD_WIDTH,
    LPORT_SRDS_SERDES_1_INDIR_ACC_CNTRL_0_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_SRDS_SERDES_1_INDIR_ACC_CNTRL_0_ERR
 ******************************************************************************/
const ru_field_rec LPORT_SRDS_SERDES_1_INDIR_ACC_CNTRL_0_ERR_FIELD =
{
    "ERR",
#if RU_INCLUDE_DESC
    "",
    "Transaction Status. When transaction completes "
    "(START_BUSY = 0 after it was set to 1) and this bit is set "
    "it indicates that register transaction completed with error.",
#endif
    LPORT_SRDS_SERDES_1_INDIR_ACC_CNTRL_0_ERR_FIELD_MASK,
    0,
    LPORT_SRDS_SERDES_1_INDIR_ACC_CNTRL_0_ERR_FIELD_WIDTH,
    LPORT_SRDS_SERDES_1_INDIR_ACC_CNTRL_0_ERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_SRDS_SERDES_1_INDIR_ACC_CNTRL_0_START_BUSY
 ******************************************************************************/
const ru_field_rec LPORT_SRDS_SERDES_1_INDIR_ACC_CNTRL_0_START_BUSY_FIELD =
{
    "START_BUSY",
#if RU_INCLUDE_DESC
    "",
    "START_BUSY, Self-clearing. CPU writes this bit to 1 in order to initiate indirect register "
    "read/write transaction. When transaction completes hardware clears this bit.",
#endif
    LPORT_SRDS_SERDES_1_INDIR_ACC_CNTRL_0_START_BUSY_FIELD_MASK,
    0,
    LPORT_SRDS_SERDES_1_INDIR_ACC_CNTRL_0_START_BUSY_FIELD_WIDTH,
    LPORT_SRDS_SERDES_1_INDIR_ACC_CNTRL_0_START_BUSY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_SRDS_SERDES_1_INDIR_ACC_CNTRL_0_R_W
 ******************************************************************************/
const ru_field_rec LPORT_SRDS_SERDES_1_INDIR_ACC_CNTRL_0_R_W_FIELD =
{
    "R_W",
#if RU_INCLUDE_DESC
    "",
    "Register transaction:\n"
    "0 : Register Write.\n'"
    "1 : Register Read.\n",
#endif
    LPORT_SRDS_SERDES_1_INDIR_ACC_CNTRL_0_R_W_FIELD_MASK,
    0,
    LPORT_SRDS_SERDES_1_INDIR_ACC_CNTRL_0_R_W_FIELD_WIDTH,
    LPORT_SRDS_SERDES_1_INDIR_ACC_CNTRL_0_R_W_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_SRDS_SERDES_1_INDIR_ACC_CNTRL_0_REG_DATA
 ******************************************************************************/
const ru_field_rec LPORT_SRDS_SERDES_1_INDIR_ACC_CNTRL_0_REG_DATA_FIELD =
{
    "REG_DATA",
#if RU_INCLUDE_DESC
    "",
    "Register READ/WRITE Data.",
#endif
    LPORT_SRDS_SERDES_1_INDIR_ACC_CNTRL_0_REG_DATA_FIELD_MASK,
    0,
    LPORT_SRDS_SERDES_1_INDIR_ACC_CNTRL_0_REG_DATA_FIELD_WIDTH,
    LPORT_SRDS_SERDES_1_INDIR_ACC_CNTRL_0_REG_DATA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_SRDS_SERDES_1_INDIR_ACC_ADDR_0_REG_ADDR
 ******************************************************************************/
const ru_field_rec LPORT_SRDS_SERDES_1_INDIR_ACC_ADDR_0_REG_ADDR_FIELD =
{
    "REG_ADDR",
#if RU_INCLUDE_DESC
    "",
    "Indirect register access, address register.",
#endif
    LPORT_SRDS_SERDES_1_INDIR_ACC_ADDR_0_REG_ADDR_FIELD_MASK,
    0,
    LPORT_SRDS_SERDES_1_INDIR_ACC_ADDR_0_REG_ADDR_FIELD_WIDTH,
    LPORT_SRDS_SERDES_1_INDIR_ACC_ADDR_0_REG_ADDR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_SRDS_SERDES_1_INDIR_ACC_MASK_0_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_SRDS_SERDES_1_INDIR_ACC_MASK_0_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_SRDS_SERDES_1_INDIR_ACC_MASK_0_RESERVED0_FIELD_MASK,
    0,
    LPORT_SRDS_SERDES_1_INDIR_ACC_MASK_0_RESERVED0_FIELD_WIDTH,
    LPORT_SRDS_SERDES_1_INDIR_ACC_MASK_0_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_SRDS_SERDES_1_INDIR_ACC_MASK_0_REG_MASK
 ******************************************************************************/
const ru_field_rec LPORT_SRDS_SERDES_1_INDIR_ACC_MASK_0_REG_MASK_FIELD =
{
    "REG_MASK",
#if RU_INCLUDE_DESC
    "",
    "Indirect register access, data mask register."
    "When a bit in this mask is set (1'b1), writing to the corresponding data bit is disabled.",
#endif
    LPORT_SRDS_SERDES_1_INDIR_ACC_MASK_0_REG_MASK_FIELD_MASK,
    0,
    LPORT_SRDS_SERDES_1_INDIR_ACC_MASK_0_REG_MASK_FIELD_WIDTH,
    LPORT_SRDS_SERDES_1_INDIR_ACC_MASK_0_REG_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_SRDS_SERDES_1_INDIR_ACC_CNTRL_1_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_SRDS_SERDES_1_INDIR_ACC_CNTRL_1_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_SRDS_SERDES_1_INDIR_ACC_CNTRL_1_RESERVED0_FIELD_MASK,
    0,
    LPORT_SRDS_SERDES_1_INDIR_ACC_CNTRL_1_RESERVED0_FIELD_WIDTH,
    LPORT_SRDS_SERDES_1_INDIR_ACC_CNTRL_1_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_SRDS_SERDES_1_INDIR_ACC_CNTRL_1_ERR
 ******************************************************************************/
const ru_field_rec LPORT_SRDS_SERDES_1_INDIR_ACC_CNTRL_1_ERR_FIELD =
{
    "ERR",
#if RU_INCLUDE_DESC
    "",
    "Transaction Status. When transaction completes "
    "(START_BUSY = 0 after it was set to 1) and this bit is set "
    "it indicates that register transaction completed with error.",
#endif
    LPORT_SRDS_SERDES_1_INDIR_ACC_CNTRL_1_ERR_FIELD_MASK,
    0,
    LPORT_SRDS_SERDES_1_INDIR_ACC_CNTRL_1_ERR_FIELD_WIDTH,
    LPORT_SRDS_SERDES_1_INDIR_ACC_CNTRL_1_ERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_SRDS_SERDES_1_INDIR_ACC_CNTRL_1_START_BUSY
 ******************************************************************************/
const ru_field_rec LPORT_SRDS_SERDES_1_INDIR_ACC_CNTRL_1_START_BUSY_FIELD =
{
    "START_BUSY",
#if RU_INCLUDE_DESC
    "",
    "START_BUSY, Self-clearing. CPU writes this bit to 1 in order to initiate indirect register "
    "read/write transaction. When transaction completes hardware clears this bit.",
#endif
    LPORT_SRDS_SERDES_1_INDIR_ACC_CNTRL_1_START_BUSY_FIELD_MASK,
    0,
    LPORT_SRDS_SERDES_1_INDIR_ACC_CNTRL_1_START_BUSY_FIELD_WIDTH,
    LPORT_SRDS_SERDES_1_INDIR_ACC_CNTRL_1_START_BUSY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_SRDS_SERDES_1_INDIR_ACC_CNTRL_1_R_W
 ******************************************************************************/
const ru_field_rec LPORT_SRDS_SERDES_1_INDIR_ACC_CNTRL_1_R_W_FIELD =
{
    "R_W",
#if RU_INCLUDE_DESC
    "",
    "Register transaction:\n"
    "0 : Register Write.\n'"
    "1 : Register Read.\n",
#endif
    LPORT_SRDS_SERDES_1_INDIR_ACC_CNTRL_1_R_W_FIELD_MASK,
    0,
    LPORT_SRDS_SERDES_1_INDIR_ACC_CNTRL_1_R_W_FIELD_WIDTH,
    LPORT_SRDS_SERDES_1_INDIR_ACC_CNTRL_1_R_W_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_SRDS_SERDES_1_INDIR_ACC_CNTRL_1_REG_DATA
 ******************************************************************************/
const ru_field_rec LPORT_SRDS_SERDES_1_INDIR_ACC_CNTRL_1_REG_DATA_FIELD =
{
    "REG_DATA",
#if RU_INCLUDE_DESC
    "",
    "Register READ/WRITE Data.",
#endif
    LPORT_SRDS_SERDES_1_INDIR_ACC_CNTRL_1_REG_DATA_FIELD_MASK,
    0,
    LPORT_SRDS_SERDES_1_INDIR_ACC_CNTRL_1_REG_DATA_FIELD_WIDTH,
    LPORT_SRDS_SERDES_1_INDIR_ACC_CNTRL_1_REG_DATA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_SRDS_SERDES_1_INDIR_ACC_ADDR_1_REG_ADDR
 ******************************************************************************/
const ru_field_rec LPORT_SRDS_SERDES_1_INDIR_ACC_ADDR_1_REG_ADDR_FIELD =
{
    "REG_ADDR",
#if RU_INCLUDE_DESC
    "",
    "Indirect register access, address register.",
#endif
    LPORT_SRDS_SERDES_1_INDIR_ACC_ADDR_1_REG_ADDR_FIELD_MASK,
    0,
    LPORT_SRDS_SERDES_1_INDIR_ACC_ADDR_1_REG_ADDR_FIELD_WIDTH,
    LPORT_SRDS_SERDES_1_INDIR_ACC_ADDR_1_REG_ADDR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_SRDS_SERDES_1_INDIR_ACC_MASK_1_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_SRDS_SERDES_1_INDIR_ACC_MASK_1_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_SRDS_SERDES_1_INDIR_ACC_MASK_1_RESERVED0_FIELD_MASK,
    0,
    LPORT_SRDS_SERDES_1_INDIR_ACC_MASK_1_RESERVED0_FIELD_WIDTH,
    LPORT_SRDS_SERDES_1_INDIR_ACC_MASK_1_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_SRDS_SERDES_1_INDIR_ACC_MASK_1_REG_MASK
 ******************************************************************************/
const ru_field_rec LPORT_SRDS_SERDES_1_INDIR_ACC_MASK_1_REG_MASK_FIELD =
{
    "REG_MASK",
#if RU_INCLUDE_DESC
    "",
    "Indirect register access, data mask register."
    "When a bit in this mask is set (1'b1), writing to the corresponding data bit is disabled.",
#endif
    LPORT_SRDS_SERDES_1_INDIR_ACC_MASK_1_REG_MASK_FIELD_MASK,
    0,
    LPORT_SRDS_SERDES_1_INDIR_ACC_MASK_1_REG_MASK_FIELD_WIDTH,
    LPORT_SRDS_SERDES_1_INDIR_ACC_MASK_1_REG_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_SRDS_DUAL_SERDES_0_CNTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_SRDS_DUAL_SERDES_0_CNTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_SRDS_DUAL_SERDES_0_CNTRL_RESERVED0_FIELD_MASK,
    0,
    LPORT_SRDS_DUAL_SERDES_0_CNTRL_RESERVED0_FIELD_WIDTH,
    LPORT_SRDS_DUAL_SERDES_0_CNTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_SRDS_DUAL_SERDES_0_CNTRL_SERDES_TEST_EN
 ******************************************************************************/
const ru_field_rec LPORT_SRDS_DUAL_SERDES_0_CNTRL_SERDES_TEST_EN_FIELD =
{
    "SERDES_TEST_EN",
#if RU_INCLUDE_DESC
    "",
    "When set single SERDES MDIO is controlled by a MDIO master connected to chip pins. Debug only function.",
#endif
    LPORT_SRDS_DUAL_SERDES_0_CNTRL_SERDES_TEST_EN_FIELD_MASK,
    0,
    LPORT_SRDS_DUAL_SERDES_0_CNTRL_SERDES_TEST_EN_FIELD_WIDTH,
    LPORT_SRDS_DUAL_SERDES_0_CNTRL_SERDES_TEST_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_SRDS_DUAL_SERDES_0_CNTRL_SERDES_LN_OFFSET
 ******************************************************************************/
const ru_field_rec LPORT_SRDS_DUAL_SERDES_0_CNTRL_SERDES_LN_OFFSET_FIELD =
{
    "SERDES_LN_OFFSET",
#if RU_INCLUDE_DESC
    "",
    "SERDES Lane Offset device address for Clause 45.",
#endif
    LPORT_SRDS_DUAL_SERDES_0_CNTRL_SERDES_LN_OFFSET_FIELD_MASK,
    0,
    LPORT_SRDS_DUAL_SERDES_0_CNTRL_SERDES_LN_OFFSET_FIELD_WIDTH,
    LPORT_SRDS_DUAL_SERDES_0_CNTRL_SERDES_LN_OFFSET_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_SRDS_DUAL_SERDES_0_CNTRL_SERDES_PRTAD
 ******************************************************************************/
const ru_field_rec LPORT_SRDS_DUAL_SERDES_0_CNTRL_SERDES_PRTAD_FIELD =
{
    "SERDES_PRTAD",
#if RU_INCLUDE_DESC
    "",
    "SERDES port address for Clause 45",
#endif
    LPORT_SRDS_DUAL_SERDES_0_CNTRL_SERDES_PRTAD_FIELD_MASK,
    0,
    LPORT_SRDS_DUAL_SERDES_0_CNTRL_SERDES_PRTAD_FIELD_WIDTH,
    LPORT_SRDS_DUAL_SERDES_0_CNTRL_SERDES_PRTAD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_SRDS_DUAL_SERDES_0_CNTRL_RESERVED1
 ******************************************************************************/
const ru_field_rec LPORT_SRDS_DUAL_SERDES_0_CNTRL_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_SRDS_DUAL_SERDES_0_CNTRL_RESERVED1_FIELD_MASK,
    0,
    LPORT_SRDS_DUAL_SERDES_0_CNTRL_RESERVED1_FIELD_WIDTH,
    LPORT_SRDS_DUAL_SERDES_0_CNTRL_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_SRDS_DUAL_SERDES_0_CNTRL_RESERVED2
 ******************************************************************************/
const ru_field_rec LPORT_SRDS_DUAL_SERDES_0_CNTRL_RESERVED2_FIELD =
{
    "RESERVED2",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_SRDS_DUAL_SERDES_0_CNTRL_RESERVED2_FIELD_MASK,
    0,
    LPORT_SRDS_DUAL_SERDES_0_CNTRL_RESERVED2_FIELD_WIDTH,
    LPORT_SRDS_DUAL_SERDES_0_CNTRL_RESERVED2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_SRDS_DUAL_SERDES_0_CNTRL_SERDES_RESET
 ******************************************************************************/
const ru_field_rec LPORT_SRDS_DUAL_SERDES_0_CNTRL_SERDES_RESET_FIELD =
{
    "SERDES_RESET",
#if RU_INCLUDE_DESC
    "",
    "Active high SERDES system reset. Resets whole SERDES core. "
    " Must be held high for at least 150ns after IDDQ is de-asserted.",
#endif
    LPORT_SRDS_DUAL_SERDES_0_CNTRL_SERDES_RESET_FIELD_MASK,
    0,
    LPORT_SRDS_DUAL_SERDES_0_CNTRL_SERDES_RESET_FIELD_WIDTH,
    LPORT_SRDS_DUAL_SERDES_0_CNTRL_SERDES_RESET_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_SRDS_DUAL_SERDES_0_CNTRL_REFCLK_RESET
 ******************************************************************************/
const ru_field_rec LPORT_SRDS_DUAL_SERDES_0_CNTRL_REFCLK_RESET_FIELD =
{
    "REFCLK_RESET",
#if RU_INCLUDE_DESC
    "",
    "Active high SERDES reference clock logic reset. Resets logic operating in SERDES reference clock domain.",
#endif
    LPORT_SRDS_DUAL_SERDES_0_CNTRL_REFCLK_RESET_FIELD_MASK,
    0,
    LPORT_SRDS_DUAL_SERDES_0_CNTRL_REFCLK_RESET_FIELD_WIDTH,
    LPORT_SRDS_DUAL_SERDES_0_CNTRL_REFCLK_RESET_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_SRDS_DUAL_SERDES_0_CNTRL_IDDQ
 ******************************************************************************/
const ru_field_rec LPORT_SRDS_DUAL_SERDES_0_CNTRL_IDDQ_FIELD =
{
    "IDDQ",
#if RU_INCLUDE_DESC
    "",
    "IDDQ Enable. Powers down SERDES analog front end and turn off all clocks. MDIO is not operational.",
#endif
    LPORT_SRDS_DUAL_SERDES_0_CNTRL_IDDQ_FIELD_MASK,
    0,
    LPORT_SRDS_DUAL_SERDES_0_CNTRL_IDDQ_FIELD_WIDTH,
    LPORT_SRDS_DUAL_SERDES_0_CNTRL_IDDQ_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_SRDS_DUAL_SERDES_0_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_SRDS_DUAL_SERDES_0_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_SRDS_DUAL_SERDES_0_STATUS_RESERVED0_FIELD_MASK,
    0,
    LPORT_SRDS_DUAL_SERDES_0_STATUS_RESERVED0_FIELD_WIDTH,
    LPORT_SRDS_DUAL_SERDES_0_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_SRDS_DUAL_SERDES_0_STATUS_MOD_DEF0
 ******************************************************************************/
const ru_field_rec LPORT_SRDS_DUAL_SERDES_0_STATUS_MOD_DEF0_FIELD =
{
    "MOD_DEF0",
#if RU_INCLUDE_DESC
    "",
    "When 0 indicates presence of the optical module.",
#endif
    LPORT_SRDS_DUAL_SERDES_0_STATUS_MOD_DEF0_FIELD_MASK,
    0,
    LPORT_SRDS_DUAL_SERDES_0_STATUS_MOD_DEF0_FIELD_WIDTH,
    LPORT_SRDS_DUAL_SERDES_0_STATUS_MOD_DEF0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_SRDS_DUAL_SERDES_0_STATUS_EXT_SIG_DET
 ******************************************************************************/
const ru_field_rec LPORT_SRDS_DUAL_SERDES_0_STATUS_EXT_SIG_DET_FIELD =
{
    "EXT_SIG_DET",
#if RU_INCLUDE_DESC
    "",
    "Non-filtered signal detect (or loss of signal) from the pin as provided by the external optical module. "
    "Please consult used optical module datasheet for polarity. "
    "NVRAM bit that indicates expected polarity is recommended.",
#endif
    LPORT_SRDS_DUAL_SERDES_0_STATUS_EXT_SIG_DET_FIELD_MASK,
    0,
    LPORT_SRDS_DUAL_SERDES_0_STATUS_EXT_SIG_DET_FIELD_WIDTH,
    LPORT_SRDS_DUAL_SERDES_0_STATUS_EXT_SIG_DET_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_SRDS_DUAL_SERDES_0_STATUS_PLL_LOCK
 ******************************************************************************/
const ru_field_rec LPORT_SRDS_DUAL_SERDES_0_STATUS_PLL_LOCK_FIELD =
{
    "PLL_LOCK",
#if RU_INCLUDE_DESC
    "",
    "PLL Lock. When 1'b1, indicates that single SERDES PLL is locked.",
#endif
    LPORT_SRDS_DUAL_SERDES_0_STATUS_PLL_LOCK_FIELD_MASK,
    0,
    LPORT_SRDS_DUAL_SERDES_0_STATUS_PLL_LOCK_FIELD_WIDTH,
    LPORT_SRDS_DUAL_SERDES_0_STATUS_PLL_LOCK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_SRDS_DUAL_SERDES_0_STATUS_LINK_STATUS
 ******************************************************************************/
const ru_field_rec LPORT_SRDS_DUAL_SERDES_0_STATUS_LINK_STATUS_FIELD =
{
    "LINK_STATUS",
#if RU_INCLUDE_DESC
    "",
    "Link Status. When 1'b1, indicates that link is up for the respective SERDES core.",
#endif
    LPORT_SRDS_DUAL_SERDES_0_STATUS_LINK_STATUS_FIELD_MASK,
    0,
    LPORT_SRDS_DUAL_SERDES_0_STATUS_LINK_STATUS_FIELD_WIDTH,
    LPORT_SRDS_DUAL_SERDES_0_STATUS_LINK_STATUS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_SRDS_DUAL_SERDES_0_STATUS_CDR_LOCK
 ******************************************************************************/
const ru_field_rec LPORT_SRDS_DUAL_SERDES_0_STATUS_CDR_LOCK_FIELD =
{
    "CDR_LOCK",
#if RU_INCLUDE_DESC
    "",
    "CDR Lock. When 1'b1, indicates that CDR is locked for the respective SERDES core.",
#endif
    LPORT_SRDS_DUAL_SERDES_0_STATUS_CDR_LOCK_FIELD_MASK,
    0,
    LPORT_SRDS_DUAL_SERDES_0_STATUS_CDR_LOCK_FIELD_WIDTH,
    LPORT_SRDS_DUAL_SERDES_0_STATUS_CDR_LOCK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_SRDS_DUAL_SERDES_0_STATUS_RX_SIGDET
 ******************************************************************************/
const ru_field_rec LPORT_SRDS_DUAL_SERDES_0_STATUS_RX_SIGDET_FIELD =
{
    "RX_SIGDET",
#if RU_INCLUDE_DESC
    "",
    "Filtered Rx Signal Detect. When 1'b1 indicates presence of the signal on Rx pins for the respective SERDES core.",
#endif
    LPORT_SRDS_DUAL_SERDES_0_STATUS_RX_SIGDET_FIELD_MASK,
    0,
    LPORT_SRDS_DUAL_SERDES_0_STATUS_RX_SIGDET_FIELD_WIDTH,
    LPORT_SRDS_DUAL_SERDES_0_STATUS_RX_SIGDET_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_SRDS_DUAL_SERDES_1_CNTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_SRDS_DUAL_SERDES_1_CNTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_SRDS_DUAL_SERDES_1_CNTRL_RESERVED0_FIELD_MASK,
    0,
    LPORT_SRDS_DUAL_SERDES_1_CNTRL_RESERVED0_FIELD_WIDTH,
    LPORT_SRDS_DUAL_SERDES_1_CNTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_SRDS_DUAL_SERDES_1_CNTRL_SERDES_TEST_EN
 ******************************************************************************/
const ru_field_rec LPORT_SRDS_DUAL_SERDES_1_CNTRL_SERDES_TEST_EN_FIELD =
{
    "SERDES_TEST_EN",
#if RU_INCLUDE_DESC
    "",
    "When set single SERDES MDIO is controlled by a MDIO master connected to chip pins. Debug only function.",
#endif
    LPORT_SRDS_DUAL_SERDES_1_CNTRL_SERDES_TEST_EN_FIELD_MASK,
    0,
    LPORT_SRDS_DUAL_SERDES_1_CNTRL_SERDES_TEST_EN_FIELD_WIDTH,
    LPORT_SRDS_DUAL_SERDES_1_CNTRL_SERDES_TEST_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_SRDS_DUAL_SERDES_1_CNTRL_SERDES_LN_OFFSET
 ******************************************************************************/
const ru_field_rec LPORT_SRDS_DUAL_SERDES_1_CNTRL_SERDES_LN_OFFSET_FIELD =
{
    "SERDES_LN_OFFSET",
#if RU_INCLUDE_DESC
    "",
    "SERDES Lane Offset device address for Clause 45.",
#endif
    LPORT_SRDS_DUAL_SERDES_1_CNTRL_SERDES_LN_OFFSET_FIELD_MASK,
    0,
    LPORT_SRDS_DUAL_SERDES_1_CNTRL_SERDES_LN_OFFSET_FIELD_WIDTH,
    LPORT_SRDS_DUAL_SERDES_1_CNTRL_SERDES_LN_OFFSET_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_SRDS_DUAL_SERDES_1_CNTRL_SERDES_PRTAD
 ******************************************************************************/
const ru_field_rec LPORT_SRDS_DUAL_SERDES_1_CNTRL_SERDES_PRTAD_FIELD =
{
    "SERDES_PRTAD",
#if RU_INCLUDE_DESC
    "",
    "SERDES port address for Clause 45",
#endif
    LPORT_SRDS_DUAL_SERDES_1_CNTRL_SERDES_PRTAD_FIELD_MASK,
    0,
    LPORT_SRDS_DUAL_SERDES_1_CNTRL_SERDES_PRTAD_FIELD_WIDTH,
    LPORT_SRDS_DUAL_SERDES_1_CNTRL_SERDES_PRTAD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_SRDS_DUAL_SERDES_1_CNTRL_RESERVED1
 ******************************************************************************/
const ru_field_rec LPORT_SRDS_DUAL_SERDES_1_CNTRL_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_SRDS_DUAL_SERDES_1_CNTRL_RESERVED1_FIELD_MASK,
    0,
    LPORT_SRDS_DUAL_SERDES_1_CNTRL_RESERVED1_FIELD_WIDTH,
    LPORT_SRDS_DUAL_SERDES_1_CNTRL_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_SRDS_DUAL_SERDES_1_CNTRL_RESERVED2
 ******************************************************************************/
const ru_field_rec LPORT_SRDS_DUAL_SERDES_1_CNTRL_RESERVED2_FIELD =
{
    "RESERVED2",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_SRDS_DUAL_SERDES_1_CNTRL_RESERVED2_FIELD_MASK,
    0,
    LPORT_SRDS_DUAL_SERDES_1_CNTRL_RESERVED2_FIELD_WIDTH,
    LPORT_SRDS_DUAL_SERDES_1_CNTRL_RESERVED2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_SRDS_DUAL_SERDES_1_CNTRL_SERDES_RESET
 ******************************************************************************/
const ru_field_rec LPORT_SRDS_DUAL_SERDES_1_CNTRL_SERDES_RESET_FIELD =
{
    "SERDES_RESET",
#if RU_INCLUDE_DESC
    "",
    "Active high SERDES system reset. Resets whole SERDES core. "
    " Must be held high for at least 150ns after IDDQ is de-asserted.",
#endif
    LPORT_SRDS_DUAL_SERDES_1_CNTRL_SERDES_RESET_FIELD_MASK,
    0,
    LPORT_SRDS_DUAL_SERDES_1_CNTRL_SERDES_RESET_FIELD_WIDTH,
    LPORT_SRDS_DUAL_SERDES_1_CNTRL_SERDES_RESET_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_SRDS_DUAL_SERDES_1_CNTRL_REFCLK_RESET
 ******************************************************************************/
const ru_field_rec LPORT_SRDS_DUAL_SERDES_1_CNTRL_REFCLK_RESET_FIELD =
{
    "REFCLK_RESET",
#if RU_INCLUDE_DESC
    "",
    "Active high SERDES reference clock logic reset. Resets logic operating in SERDES reference clock domain.",
#endif
    LPORT_SRDS_DUAL_SERDES_1_CNTRL_REFCLK_RESET_FIELD_MASK,
    0,
    LPORT_SRDS_DUAL_SERDES_1_CNTRL_REFCLK_RESET_FIELD_WIDTH,
    LPORT_SRDS_DUAL_SERDES_1_CNTRL_REFCLK_RESET_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_SRDS_DUAL_SERDES_1_CNTRL_IDDQ
 ******************************************************************************/
const ru_field_rec LPORT_SRDS_DUAL_SERDES_1_CNTRL_IDDQ_FIELD =
{
    "IDDQ",
#if RU_INCLUDE_DESC
    "",
    "IDDQ Enable. Powers down SERDES analog front end and turn off all clocks. MDIO is not operational.",
#endif
    LPORT_SRDS_DUAL_SERDES_1_CNTRL_IDDQ_FIELD_MASK,
    0,
    LPORT_SRDS_DUAL_SERDES_1_CNTRL_IDDQ_FIELD_WIDTH,
    LPORT_SRDS_DUAL_SERDES_1_CNTRL_IDDQ_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_SRDS_DUAL_SERDES_1_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_SRDS_DUAL_SERDES_1_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_SRDS_DUAL_SERDES_1_STATUS_RESERVED0_FIELD_MASK,
    0,
    LPORT_SRDS_DUAL_SERDES_1_STATUS_RESERVED0_FIELD_WIDTH,
    LPORT_SRDS_DUAL_SERDES_1_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_SRDS_DUAL_SERDES_1_STATUS_MOD_DEF0
 ******************************************************************************/
const ru_field_rec LPORT_SRDS_DUAL_SERDES_1_STATUS_MOD_DEF0_FIELD =
{
    "MOD_DEF0",
#if RU_INCLUDE_DESC
    "",
    "When 0 indicates presence of the optical module.",
#endif
    LPORT_SRDS_DUAL_SERDES_1_STATUS_MOD_DEF0_FIELD_MASK,
    0,
    LPORT_SRDS_DUAL_SERDES_1_STATUS_MOD_DEF0_FIELD_WIDTH,
    LPORT_SRDS_DUAL_SERDES_1_STATUS_MOD_DEF0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_SRDS_DUAL_SERDES_1_STATUS_EXT_SIG_DET
 ******************************************************************************/
const ru_field_rec LPORT_SRDS_DUAL_SERDES_1_STATUS_EXT_SIG_DET_FIELD =
{
    "EXT_SIG_DET",
#if RU_INCLUDE_DESC
    "",
    "Non-filtered signal detect (or loss of signal) from the pin as provided by the external optical module. "
    "Please consult used optical module datasheet for polarity. "
    "NVRAM bit that indicates expected polarity is recommended.",
#endif
    LPORT_SRDS_DUAL_SERDES_1_STATUS_EXT_SIG_DET_FIELD_MASK,
    0,
    LPORT_SRDS_DUAL_SERDES_1_STATUS_EXT_SIG_DET_FIELD_WIDTH,
    LPORT_SRDS_DUAL_SERDES_1_STATUS_EXT_SIG_DET_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_SRDS_DUAL_SERDES_1_STATUS_PLL_LOCK
 ******************************************************************************/
const ru_field_rec LPORT_SRDS_DUAL_SERDES_1_STATUS_PLL_LOCK_FIELD =
{
    "PLL_LOCK",
#if RU_INCLUDE_DESC
    "",
    "PLL Lock. When 1'b1, indicates that single SERDES PLL is locked.",
#endif
    LPORT_SRDS_DUAL_SERDES_1_STATUS_PLL_LOCK_FIELD_MASK,
    0,
    LPORT_SRDS_DUAL_SERDES_1_STATUS_PLL_LOCK_FIELD_WIDTH,
    LPORT_SRDS_DUAL_SERDES_1_STATUS_PLL_LOCK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_SRDS_DUAL_SERDES_1_STATUS_LINK_STATUS
 ******************************************************************************/
const ru_field_rec LPORT_SRDS_DUAL_SERDES_1_STATUS_LINK_STATUS_FIELD =
{
    "LINK_STATUS",
#if RU_INCLUDE_DESC
    "",
    "Link Status. When 1'b1, indicates that link is up for the respective SERDES core.",
#endif
    LPORT_SRDS_DUAL_SERDES_1_STATUS_LINK_STATUS_FIELD_MASK,
    0,
    LPORT_SRDS_DUAL_SERDES_1_STATUS_LINK_STATUS_FIELD_WIDTH,
    LPORT_SRDS_DUAL_SERDES_1_STATUS_LINK_STATUS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_SRDS_DUAL_SERDES_1_STATUS_CDR_LOCK
 ******************************************************************************/
const ru_field_rec LPORT_SRDS_DUAL_SERDES_1_STATUS_CDR_LOCK_FIELD =
{
    "CDR_LOCK",
#if RU_INCLUDE_DESC
    "",
    "CDR Lock. When 1'b1, indicates that CDR is locked for the respective SERDES core.",
#endif
    LPORT_SRDS_DUAL_SERDES_1_STATUS_CDR_LOCK_FIELD_MASK,
    0,
    LPORT_SRDS_DUAL_SERDES_1_STATUS_CDR_LOCK_FIELD_WIDTH,
    LPORT_SRDS_DUAL_SERDES_1_STATUS_CDR_LOCK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_SRDS_DUAL_SERDES_1_STATUS_RX_SIGDET
 ******************************************************************************/
const ru_field_rec LPORT_SRDS_DUAL_SERDES_1_STATUS_RX_SIGDET_FIELD =
{
    "RX_SIGDET",
#if RU_INCLUDE_DESC
    "",
    "Filtered Rx Signal Detect. When 1'b1 indicates presence of the signal on Rx pins for the respective SERDES core.",
#endif
    LPORT_SRDS_DUAL_SERDES_1_STATUS_RX_SIGDET_FIELD_MASK,
    0,
    LPORT_SRDS_DUAL_SERDES_1_STATUS_RX_SIGDET_FIELD_WIDTH,
    LPORT_SRDS_DUAL_SERDES_1_STATUS_RX_SIGDET_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: LPORT_SRDS_DUAL_SERDES_REVISION
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_SRDS_DUAL_SERDES_REVISION_FIELDS[] =
{
    &LPORT_SRDS_DUAL_SERDES_REVISION_RESERVED0_FIELD,
    &LPORT_SRDS_DUAL_SERDES_REVISION_SERDES_REV_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_SRDS_DUAL_SERDES_REVISION_REG = 
{
    "DUAL_SERDES_REVISION",
#if RU_INCLUDE_DESC
    "Dual SERDES Revision Control Register",
    "",
#endif
    LPORT_SRDS_DUAL_SERDES_REVISION_REG_OFFSET,
    0,
    0,
    183,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    LPORT_SRDS_DUAL_SERDES_REVISION_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LPORT_SRDS_SERDES_0_INDIR_ACC_CNTRL_0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_SRDS_SERDES_0_INDIR_ACC_CNTRL_0_FIELDS[] =
{
    &LPORT_SRDS_SERDES_0_INDIR_ACC_CNTRL_0_RESERVED0_FIELD,
    &LPORT_SRDS_SERDES_0_INDIR_ACC_CNTRL_0_ERR_FIELD,
    &LPORT_SRDS_SERDES_0_INDIR_ACC_CNTRL_0_START_BUSY_FIELD,
    &LPORT_SRDS_SERDES_0_INDIR_ACC_CNTRL_0_R_W_FIELD,
    &LPORT_SRDS_SERDES_0_INDIR_ACC_CNTRL_0_REG_DATA_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_SRDS_SERDES_0_INDIR_ACC_CNTRL_0_REG = 
{
    "SERDES_0_INDIR_ACC_CNTRL_0",
#if RU_INCLUDE_DESC
    "SERDES 1 Indirect Access Control Register 1",
    "",
#endif
    LPORT_SRDS_SERDES_0_INDIR_ACC_CNTRL_0_REG_OFFSET,
    0,
    0,
    184,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    LPORT_SRDS_SERDES_0_INDIR_ACC_CNTRL_0_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LPORT_SRDS_SERDES_0_INDIR_ACC_ADDR_0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_SRDS_SERDES_0_INDIR_ACC_ADDR_0_FIELDS[] =
{
    &LPORT_SRDS_SERDES_0_INDIR_ACC_ADDR_0_REG_ADDR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_SRDS_SERDES_0_INDIR_ACC_ADDR_0_REG = 
{
    "SERDES_0_INDIR_ACC_ADDR_0",
#if RU_INCLUDE_DESC
    "SERDES 1 Indirect Access Address Register 1",
    "",
#endif
    LPORT_SRDS_SERDES_0_INDIR_ACC_ADDR_0_REG_OFFSET,
    0,
    0,
    185,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LPORT_SRDS_SERDES_0_INDIR_ACC_ADDR_0_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LPORT_SRDS_SERDES_0_INDIR_ACC_MASK_0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_SRDS_SERDES_0_INDIR_ACC_MASK_0_FIELDS[] =
{
    &LPORT_SRDS_SERDES_0_INDIR_ACC_MASK_0_RESERVED0_FIELD,
    &LPORT_SRDS_SERDES_0_INDIR_ACC_MASK_0_REG_MASK_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_SRDS_SERDES_0_INDIR_ACC_MASK_0_REG = 
{
    "SERDES_0_INDIR_ACC_MASK_0",
#if RU_INCLUDE_DESC
    "SERDES 1 Indirect Access Mask Register 1",
    "",
#endif
    LPORT_SRDS_SERDES_0_INDIR_ACC_MASK_0_REG_OFFSET,
    0,
    0,
    186,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    LPORT_SRDS_SERDES_0_INDIR_ACC_MASK_0_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LPORT_SRDS_SERDES_0_INDIR_ACC_CNTRL_1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_SRDS_SERDES_0_INDIR_ACC_CNTRL_1_FIELDS[] =
{
    &LPORT_SRDS_SERDES_0_INDIR_ACC_CNTRL_1_RESERVED0_FIELD,
    &LPORT_SRDS_SERDES_0_INDIR_ACC_CNTRL_1_ERR_FIELD,
    &LPORT_SRDS_SERDES_0_INDIR_ACC_CNTRL_1_START_BUSY_FIELD,
    &LPORT_SRDS_SERDES_0_INDIR_ACC_CNTRL_1_R_W_FIELD,
    &LPORT_SRDS_SERDES_0_INDIR_ACC_CNTRL_1_REG_DATA_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_SRDS_SERDES_0_INDIR_ACC_CNTRL_1_REG = 
{
    "SERDES_0_INDIR_ACC_CNTRL_1",
#if RU_INCLUDE_DESC
    "SERDES 1 Indirect Access Control Register 1",
    "",
#endif
    LPORT_SRDS_SERDES_0_INDIR_ACC_CNTRL_1_REG_OFFSET,
    0,
    0,
    187,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    LPORT_SRDS_SERDES_0_INDIR_ACC_CNTRL_1_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LPORT_SRDS_SERDES_0_INDIR_ACC_ADDR_1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_SRDS_SERDES_0_INDIR_ACC_ADDR_1_FIELDS[] =
{
    &LPORT_SRDS_SERDES_0_INDIR_ACC_ADDR_1_REG_ADDR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_SRDS_SERDES_0_INDIR_ACC_ADDR_1_REG = 
{
    "SERDES_0_INDIR_ACC_ADDR_1",
#if RU_INCLUDE_DESC
    "SERDES 1 Indirect Access Address Register 1",
    "",
#endif
    LPORT_SRDS_SERDES_0_INDIR_ACC_ADDR_1_REG_OFFSET,
    0,
    0,
    188,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LPORT_SRDS_SERDES_0_INDIR_ACC_ADDR_1_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LPORT_SRDS_SERDES_0_INDIR_ACC_MASK_1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_SRDS_SERDES_0_INDIR_ACC_MASK_1_FIELDS[] =
{
    &LPORT_SRDS_SERDES_0_INDIR_ACC_MASK_1_RESERVED0_FIELD,
    &LPORT_SRDS_SERDES_0_INDIR_ACC_MASK_1_REG_MASK_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_SRDS_SERDES_0_INDIR_ACC_MASK_1_REG = 
{
    "SERDES_0_INDIR_ACC_MASK_1",
#if RU_INCLUDE_DESC
    "SERDES 1 Indirect Access Mask Register 1",
    "",
#endif
    LPORT_SRDS_SERDES_0_INDIR_ACC_MASK_1_REG_OFFSET,
    0,
    0,
    189,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    LPORT_SRDS_SERDES_0_INDIR_ACC_MASK_1_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LPORT_SRDS_SERDES_1_INDIR_ACC_CNTRL_0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_SRDS_SERDES_1_INDIR_ACC_CNTRL_0_FIELDS[] =
{
    &LPORT_SRDS_SERDES_1_INDIR_ACC_CNTRL_0_RESERVED0_FIELD,
    &LPORT_SRDS_SERDES_1_INDIR_ACC_CNTRL_0_ERR_FIELD,
    &LPORT_SRDS_SERDES_1_INDIR_ACC_CNTRL_0_START_BUSY_FIELD,
    &LPORT_SRDS_SERDES_1_INDIR_ACC_CNTRL_0_R_W_FIELD,
    &LPORT_SRDS_SERDES_1_INDIR_ACC_CNTRL_0_REG_DATA_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_SRDS_SERDES_1_INDIR_ACC_CNTRL_0_REG = 
{
    "SERDES_1_INDIR_ACC_CNTRL_0",
#if RU_INCLUDE_DESC
    "SERDES 1 Indirect Access Control Register 1",
    "",
#endif
    LPORT_SRDS_SERDES_1_INDIR_ACC_CNTRL_0_REG_OFFSET,
    0,
    0,
    190,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    LPORT_SRDS_SERDES_1_INDIR_ACC_CNTRL_0_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LPORT_SRDS_SERDES_1_INDIR_ACC_ADDR_0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_SRDS_SERDES_1_INDIR_ACC_ADDR_0_FIELDS[] =
{
    &LPORT_SRDS_SERDES_1_INDIR_ACC_ADDR_0_REG_ADDR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_SRDS_SERDES_1_INDIR_ACC_ADDR_0_REG = 
{
    "SERDES_1_INDIR_ACC_ADDR_0",
#if RU_INCLUDE_DESC
    "SERDES 1 Indirect Access Address Register 1",
    "",
#endif
    LPORT_SRDS_SERDES_1_INDIR_ACC_ADDR_0_REG_OFFSET,
    0,
    0,
    191,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LPORT_SRDS_SERDES_1_INDIR_ACC_ADDR_0_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LPORT_SRDS_SERDES_1_INDIR_ACC_MASK_0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_SRDS_SERDES_1_INDIR_ACC_MASK_0_FIELDS[] =
{
    &LPORT_SRDS_SERDES_1_INDIR_ACC_MASK_0_RESERVED0_FIELD,
    &LPORT_SRDS_SERDES_1_INDIR_ACC_MASK_0_REG_MASK_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_SRDS_SERDES_1_INDIR_ACC_MASK_0_REG = 
{
    "SERDES_1_INDIR_ACC_MASK_0",
#if RU_INCLUDE_DESC
    "SERDES 1 Indirect Access Mask Register 1",
    "",
#endif
    LPORT_SRDS_SERDES_1_INDIR_ACC_MASK_0_REG_OFFSET,
    0,
    0,
    192,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    LPORT_SRDS_SERDES_1_INDIR_ACC_MASK_0_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LPORT_SRDS_SERDES_1_INDIR_ACC_CNTRL_1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_SRDS_SERDES_1_INDIR_ACC_CNTRL_1_FIELDS[] =
{
    &LPORT_SRDS_SERDES_1_INDIR_ACC_CNTRL_1_RESERVED0_FIELD,
    &LPORT_SRDS_SERDES_1_INDIR_ACC_CNTRL_1_ERR_FIELD,
    &LPORT_SRDS_SERDES_1_INDIR_ACC_CNTRL_1_START_BUSY_FIELD,
    &LPORT_SRDS_SERDES_1_INDIR_ACC_CNTRL_1_R_W_FIELD,
    &LPORT_SRDS_SERDES_1_INDIR_ACC_CNTRL_1_REG_DATA_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_SRDS_SERDES_1_INDIR_ACC_CNTRL_1_REG = 
{
    "SERDES_1_INDIR_ACC_CNTRL_1",
#if RU_INCLUDE_DESC
    "SERDES 1 Indirect Access Control Register 1",
    "",
#endif
    LPORT_SRDS_SERDES_1_INDIR_ACC_CNTRL_1_REG_OFFSET,
    0,
    0,
    193,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    LPORT_SRDS_SERDES_1_INDIR_ACC_CNTRL_1_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LPORT_SRDS_SERDES_1_INDIR_ACC_ADDR_1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_SRDS_SERDES_1_INDIR_ACC_ADDR_1_FIELDS[] =
{
    &LPORT_SRDS_SERDES_1_INDIR_ACC_ADDR_1_REG_ADDR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_SRDS_SERDES_1_INDIR_ACC_ADDR_1_REG = 
{
    "SERDES_1_INDIR_ACC_ADDR_1",
#if RU_INCLUDE_DESC
    "SERDES 1 Indirect Access Address Register 1",
    "",
#endif
    LPORT_SRDS_SERDES_1_INDIR_ACC_ADDR_1_REG_OFFSET,
    0,
    0,
    194,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LPORT_SRDS_SERDES_1_INDIR_ACC_ADDR_1_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LPORT_SRDS_SERDES_1_INDIR_ACC_MASK_1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_SRDS_SERDES_1_INDIR_ACC_MASK_1_FIELDS[] =
{
    &LPORT_SRDS_SERDES_1_INDIR_ACC_MASK_1_RESERVED0_FIELD,
    &LPORT_SRDS_SERDES_1_INDIR_ACC_MASK_1_REG_MASK_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_SRDS_SERDES_1_INDIR_ACC_MASK_1_REG = 
{
    "SERDES_1_INDIR_ACC_MASK_1",
#if RU_INCLUDE_DESC
    "SERDES 1 Indirect Access Mask Register 1",
    "",
#endif
    LPORT_SRDS_SERDES_1_INDIR_ACC_MASK_1_REG_OFFSET,
    0,
    0,
    195,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    LPORT_SRDS_SERDES_1_INDIR_ACC_MASK_1_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LPORT_SRDS_DUAL_SERDES_0_CNTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_SRDS_DUAL_SERDES_0_CNTRL_FIELDS[] =
{
    &LPORT_SRDS_DUAL_SERDES_0_CNTRL_RESERVED0_FIELD,
    &LPORT_SRDS_DUAL_SERDES_0_CNTRL_SERDES_TEST_EN_FIELD,
    &LPORT_SRDS_DUAL_SERDES_0_CNTRL_SERDES_LN_OFFSET_FIELD,
    &LPORT_SRDS_DUAL_SERDES_0_CNTRL_SERDES_PRTAD_FIELD,
    &LPORT_SRDS_DUAL_SERDES_0_CNTRL_RESERVED1_FIELD,
    &LPORT_SRDS_DUAL_SERDES_0_CNTRL_RESERVED2_FIELD,
    &LPORT_SRDS_DUAL_SERDES_0_CNTRL_SERDES_RESET_FIELD,
    &LPORT_SRDS_DUAL_SERDES_0_CNTRL_REFCLK_RESET_FIELD,
    &LPORT_SRDS_DUAL_SERDES_0_CNTRL_IDDQ_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_SRDS_DUAL_SERDES_0_CNTRL_REG = 
{
    "DUAL_SERDES_0_CNTRL",
#if RU_INCLUDE_DESC
    "Dual SERDES 0 Control Register",
    "",
#endif
    LPORT_SRDS_DUAL_SERDES_0_CNTRL_REG_OFFSET,
    0,
    0,
    196,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    LPORT_SRDS_DUAL_SERDES_0_CNTRL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LPORT_SRDS_DUAL_SERDES_0_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_SRDS_DUAL_SERDES_0_STATUS_FIELDS[] =
{
    &LPORT_SRDS_DUAL_SERDES_0_STATUS_RESERVED0_FIELD,
    &LPORT_SRDS_DUAL_SERDES_0_STATUS_MOD_DEF0_FIELD,
    &LPORT_SRDS_DUAL_SERDES_0_STATUS_EXT_SIG_DET_FIELD,
    &LPORT_SRDS_DUAL_SERDES_0_STATUS_PLL_LOCK_FIELD,
    &LPORT_SRDS_DUAL_SERDES_0_STATUS_LINK_STATUS_FIELD,
    &LPORT_SRDS_DUAL_SERDES_0_STATUS_CDR_LOCK_FIELD,
    &LPORT_SRDS_DUAL_SERDES_0_STATUS_RX_SIGDET_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_SRDS_DUAL_SERDES_0_STATUS_REG = 
{
    "DUAL_SERDES_0_STATUS",
#if RU_INCLUDE_DESC
    "Dual SERDES 1 Status Register",
    "",
#endif
    LPORT_SRDS_DUAL_SERDES_0_STATUS_REG_OFFSET,
    0,
    0,
    197,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    7,
    LPORT_SRDS_DUAL_SERDES_0_STATUS_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LPORT_SRDS_DUAL_SERDES_1_CNTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_SRDS_DUAL_SERDES_1_CNTRL_FIELDS[] =
{
    &LPORT_SRDS_DUAL_SERDES_1_CNTRL_RESERVED0_FIELD,
    &LPORT_SRDS_DUAL_SERDES_1_CNTRL_SERDES_TEST_EN_FIELD,
    &LPORT_SRDS_DUAL_SERDES_1_CNTRL_SERDES_LN_OFFSET_FIELD,
    &LPORT_SRDS_DUAL_SERDES_1_CNTRL_SERDES_PRTAD_FIELD,
    &LPORT_SRDS_DUAL_SERDES_1_CNTRL_RESERVED1_FIELD,
    &LPORT_SRDS_DUAL_SERDES_1_CNTRL_RESERVED2_FIELD,
    &LPORT_SRDS_DUAL_SERDES_1_CNTRL_SERDES_RESET_FIELD,
    &LPORT_SRDS_DUAL_SERDES_1_CNTRL_REFCLK_RESET_FIELD,
    &LPORT_SRDS_DUAL_SERDES_1_CNTRL_IDDQ_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_SRDS_DUAL_SERDES_1_CNTRL_REG = 
{
    "DUAL_SERDES_1_CNTRL",
#if RU_INCLUDE_DESC
    "Dual SERDES 1 Control Register",
    "",
#endif
    LPORT_SRDS_DUAL_SERDES_1_CNTRL_REG_OFFSET,
    0,
    0,
    198,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    LPORT_SRDS_DUAL_SERDES_1_CNTRL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LPORT_SRDS_DUAL_SERDES_1_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_SRDS_DUAL_SERDES_1_STATUS_FIELDS[] =
{
    &LPORT_SRDS_DUAL_SERDES_1_STATUS_RESERVED0_FIELD,
    &LPORT_SRDS_DUAL_SERDES_1_STATUS_MOD_DEF0_FIELD,
    &LPORT_SRDS_DUAL_SERDES_1_STATUS_EXT_SIG_DET_FIELD,
    &LPORT_SRDS_DUAL_SERDES_1_STATUS_PLL_LOCK_FIELD,
    &LPORT_SRDS_DUAL_SERDES_1_STATUS_LINK_STATUS_FIELD,
    &LPORT_SRDS_DUAL_SERDES_1_STATUS_CDR_LOCK_FIELD,
    &LPORT_SRDS_DUAL_SERDES_1_STATUS_RX_SIGDET_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_SRDS_DUAL_SERDES_1_STATUS_REG = 
{
    "DUAL_SERDES_1_STATUS",
#if RU_INCLUDE_DESC
    "Dual SERDES 1 Status Register",
    "",
#endif
    LPORT_SRDS_DUAL_SERDES_1_STATUS_REG_OFFSET,
    0,
    0,
    199,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    7,
    LPORT_SRDS_DUAL_SERDES_1_STATUS_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Block: LPORT_SRDS
 ******************************************************************************/
static const ru_reg_rec *LPORT_SRDS_REGS[] =
{
    &LPORT_SRDS_DUAL_SERDES_REVISION_REG,
    &LPORT_SRDS_SERDES_0_INDIR_ACC_CNTRL_0_REG,
    &LPORT_SRDS_SERDES_0_INDIR_ACC_ADDR_0_REG,
    &LPORT_SRDS_SERDES_0_INDIR_ACC_MASK_0_REG,
    &LPORT_SRDS_SERDES_0_INDIR_ACC_CNTRL_1_REG,
    &LPORT_SRDS_SERDES_0_INDIR_ACC_ADDR_1_REG,
    &LPORT_SRDS_SERDES_0_INDIR_ACC_MASK_1_REG,
    &LPORT_SRDS_SERDES_1_INDIR_ACC_CNTRL_0_REG,
    &LPORT_SRDS_SERDES_1_INDIR_ACC_ADDR_0_REG,
    &LPORT_SRDS_SERDES_1_INDIR_ACC_MASK_0_REG,
    &LPORT_SRDS_SERDES_1_INDIR_ACC_CNTRL_1_REG,
    &LPORT_SRDS_SERDES_1_INDIR_ACC_ADDR_1_REG,
    &LPORT_SRDS_SERDES_1_INDIR_ACC_MASK_1_REG,
    &LPORT_SRDS_DUAL_SERDES_0_CNTRL_REG,
    &LPORT_SRDS_DUAL_SERDES_0_STATUS_REG,
    &LPORT_SRDS_DUAL_SERDES_1_CNTRL_REG,
    &LPORT_SRDS_DUAL_SERDES_1_STATUS_REG,
};

unsigned long LPORT_SRDS_ADDRS[] =
{
    0x8013c00c,
};

const ru_block_rec LPORT_SRDS_BLOCK = 
{
    "LPORT_SRDS",
    LPORT_SRDS_ADDRS,
    1,
    17,
    LPORT_SRDS_REGS
};

/* End of file BCM6858_A0LPORT_SRDS.c */
