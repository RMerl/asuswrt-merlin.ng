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

/******************************************************************************
 * Register: NATC_REGFILE_REGFILE_FIFO_START_ADDR_0
 ******************************************************************************/
const ru_reg_rec NATC_REGFILE_REGFILE_FIFO_START_ADDR_0_REG = 
{
    "REGFILE_FIFO_START_ADDR_0",
#if RU_INCLUDE_DESC
    "REGFILE FIFO Start Address register 0",
    "REGFILE FIFO Start Address register 0"
    "Actual FIFO size is 2 more than the number programmed in"
    "this register due to input and output holder registers"
    "which account for 2 additional depth."
    "The actual FIFO 0 (DDR_KEY_REQ_FIFO) size is"
    "REGFILE_FIFO_START_ADDR_1 -  REGFILE_FIFO_START_ADDR_0 + 2."
    "The actual FIFO 1 (DDR_RESULT_REQ_FIFO) size is"
    "REGFILE_FIFO_START_ADDR_2 -  REGFILE_FIFO_START_ADDR_1 + 2."
    "The actual FIFO 2 (DDR_KEY_REQ_PIPE) size is"
    "REGFILE_FIFO_START_ADDR_3 -  REGFILE_FIFO_START_ADDR_2 + 2."
    "The actual FIFO 3 (BLOCKING_PENDING_FIFO) size is"
    "REGFILE_FIFO_START_ADDR_4 -  REGFILE_FIFO_START_ADDR_3 + 2.",
#endif
    NATC_REGFILE_REGFILE_FIFO_START_ADDR_0_REG_OFFSET,
    0,
    0,
    957,
};

/******************************************************************************
 * Register: NATC_REGFILE_REGFILE_FIFO_START_ADDR_1
 ******************************************************************************/
const ru_reg_rec NATC_REGFILE_REGFILE_FIFO_START_ADDR_1_REG = 
{
    "REGFILE_FIFO_START_ADDR_1",
#if RU_INCLUDE_DESC
    "REGFILE FIFO Start Address register 1",
    "REGFILE FIFO Start Address register 1"
    "Actual FIFO size is 2 more than the number programmed in"
    "this register due to input and output holder registers"
    "which account for 2 additional depth."
    "The delta between REGFILE_FIFO_START_ADDR_4 and REGFILE_FIFO_START_ADDR_5,"
    "REGFILE_FIFO_START_ADDR_5 and REGFILE_FIFO_START_ADDR_6,"
    "REGFILE_FIFO_START_ADDR_6 and REGFILE_FIFO_START_ADDR_7"
    "need to be identical since these are used for the same wide FIFO."
    "The actual FIFO 4 (DDR_WRITE_RESULT_FIFO) size is"
    "REGFILE_FIFO_START_ADDR_5 -  REGFILE_FIFO_START_ADDR_4 + 2."
    "The actual FIFO 5 (DDR_WRITE_RESULT_FIFO) size is"
    "REGFILE_FIFO_START_ADDR_6 -  REGFILE_FIFO_START_ADDR_5 + 2."
    "The actual FIFO 6 (DDR_WRITE_RESULT_FIFO) size is"
    "REGFILE_FIFO_START_ADDR_7 -  REGFILE_FIFO_START_ADDR_6 + 2."
    "The actual FIFO 7 size is the same as FIFO 4, 5, 6",
#endif
    NATC_REGFILE_REGFILE_FIFO_START_ADDR_1_REG_OFFSET,
    0,
    0,
    958,
};

/******************************************************************************
 * Block: NATC_REGFILE
 ******************************************************************************/
static const ru_reg_rec *NATC_REGFILE_REGS[] =
{
    &NATC_REGFILE_REGFILE_FIFO_START_ADDR_0_REG,
    &NATC_REGFILE_REGFILE_FIFO_START_ADDR_1_REG,
};

unsigned long NATC_REGFILE_ADDRS[] =
{
    0x82e50424,
};

const ru_block_rec NATC_REGFILE_BLOCK = 
{
    "NATC_REGFILE",
    NATC_REGFILE_ADDRS,
    1,
    2,
    NATC_REGFILE_REGS
};

/* End of file XRDP_NATC_REGFILE.c */
