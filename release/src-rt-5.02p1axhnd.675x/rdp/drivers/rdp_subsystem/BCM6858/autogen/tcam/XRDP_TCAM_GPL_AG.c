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
 * Register: TCAM_CONTEXT_RAM_TCAM_TCAM_CONTEXT_RAM_CONTEXT
 ******************************************************************************/
const ru_reg_rec TCAM_CONTEXT_RAM_TCAM_TCAM_CONTEXT_RAM_CONTEXT_REG = 
{
    "CONTEXT_RAM_TCAM_TCAM_CONTEXT_RAM_CONTEXT",
#if RU_INCLUDE_DESC
    "CONTEXT %i Register",
    "Each 64 bit entry in the context ram occupies two addresses:"
    "For 64bit entry number i:"
    "the 32 least significant bits of the context are in address 2*i"
    "the 32 most significant bits of the context are in address 2*i +1",
#endif
    TCAM_CONTEXT_RAM_TCAM_TCAM_CONTEXT_RAM_CONTEXT_REG_OFFSET,
    TCAM_CONTEXT_RAM_TCAM_TCAM_CONTEXT_RAM_CONTEXT_REG_RAM_CNT,
    4,
    689,
};

/******************************************************************************
 * Register: TCAM_CFG_TCAM_TCAM_CFG_BANK_EN
 ******************************************************************************/
const ru_reg_rec TCAM_CFG_TCAM_TCAM_CFG_BANK_EN_REG = 
{
    "CFG_TCAM_TCAM_CFG_BANK_EN",
#if RU_INCLUDE_DESC
    "BANK_ENABLE Register",
    "The TCAM is divided into 8 banks. banks can be disabled to save power. bit i correspond to addresses i*128:i*128+127",
#endif
    TCAM_CFG_TCAM_TCAM_CFG_BANK_EN_REG_OFFSET,
    0,
    0,
    690,
};

/******************************************************************************
 * Register: TCAM_CFG_TCAM_TCAM_CFG_GLOBAL_MASK
 ******************************************************************************/
const ru_reg_rec TCAM_CFG_TCAM_TCAM_CFG_GLOBAL_MASK_REG = 
{
    "CFG_TCAM_TCAM_CFG_GLOBAL_MASK",
#if RU_INCLUDE_DESC
    "GLOBAL_MASK %i Register",
    "Global Mask - 256bit mask for all entries. Default value enable all bits."
    "",
#endif
    TCAM_CFG_TCAM_TCAM_CFG_GLOBAL_MASK_REG_OFFSET,
    TCAM_CFG_TCAM_TCAM_CFG_GLOBAL_MASK_REG_RAM_CNT,
    4,
    691,
};

/******************************************************************************
 * Register: TCAM_COUNTERS_TCAM_TCAM_COUNTERS_SRCH_256
 ******************************************************************************/
const ru_reg_rec TCAM_COUNTERS_TCAM_TCAM_COUNTERS_SRCH_256_REG = 
{
    "COUNTERS_TCAM_TCAM_COUNTERS_SRCH_256",
#if RU_INCLUDE_DESC
    "SEARCHES_256BIT Register",
    "Number of 256bit key searches",
#endif
    TCAM_COUNTERS_TCAM_TCAM_COUNTERS_SRCH_256_REG_OFFSET,
    0,
    0,
    692,
};

/******************************************************************************
 * Register: TCAM_COUNTERS_TCAM_TCAM_COUNTERS_HIT_256
 ******************************************************************************/
const ru_reg_rec TCAM_COUNTERS_TCAM_TCAM_COUNTERS_HIT_256_REG = 
{
    "COUNTERS_TCAM_TCAM_COUNTERS_HIT_256",
#if RU_INCLUDE_DESC
    "HITS_256BIT Register",
    "Number of 256bit key hits",
#endif
    TCAM_COUNTERS_TCAM_TCAM_COUNTERS_HIT_256_REG_OFFSET,
    0,
    0,
    693,
};

/******************************************************************************
 * Register: TCAM_COUNTERS_TCAM_TCAM_COUNTERS_SRCH_512
 ******************************************************************************/
const ru_reg_rec TCAM_COUNTERS_TCAM_TCAM_COUNTERS_SRCH_512_REG = 
{
    "COUNTERS_TCAM_TCAM_COUNTERS_SRCH_512",
#if RU_INCLUDE_DESC
    "SEARCHES_512BIT Register",
    "Number of 512it key searches",
#endif
    TCAM_COUNTERS_TCAM_TCAM_COUNTERS_SRCH_512_REG_OFFSET,
    0,
    0,
    694,
};

/******************************************************************************
 * Register: TCAM_COUNTERS_TCAM_TCAM_COUNTERS_HIT_512
 ******************************************************************************/
const ru_reg_rec TCAM_COUNTERS_TCAM_TCAM_COUNTERS_HIT_512_REG = 
{
    "COUNTERS_TCAM_TCAM_COUNTERS_HIT_512",
#if RU_INCLUDE_DESC
    "HITS_512BIT Register",
    "Number of 512bit key hits",
#endif
    TCAM_COUNTERS_TCAM_TCAM_COUNTERS_HIT_512_REG_OFFSET,
    0,
    0,
    695,
};

/******************************************************************************
 * Register: TCAM_INDIRECT_TCAM_TCAM_INDIRECT_OP
 ******************************************************************************/
const ru_reg_rec TCAM_INDIRECT_TCAM_TCAM_INDIRECT_OP_REG = 
{
    "INDIRECT_TCAM_TCAM_INDIRECT_OP",
#if RU_INCLUDE_DESC
    "OPERATION Register",
    "TCAM Operation:"
    "0 - TCAM READ"
    "1 - TCAM Write"
    "2 - TCAM Compare"
    "3 - TCAM valid bit reset"
    "Writing to this register triggers the operation. All other relevant register should be ready before SW writes to this register.",
#endif
    TCAM_INDIRECT_TCAM_TCAM_INDIRECT_OP_REG_OFFSET,
    0,
    0,
    696,
};

/******************************************************************************
 * Register: TCAM_INDIRECT_TCAM_TCAM_INDIRECT_OP_DONE
 ******************************************************************************/
const ru_reg_rec TCAM_INDIRECT_TCAM_TCAM_INDIRECT_OP_DONE_REG = 
{
    "INDIRECT_TCAM_TCAM_INDIRECT_OP_DONE",
#if RU_INCLUDE_DESC
    "OPERATION_DONE Register",
    "Raised when the TCAM operation is completed (cleared by HW on write to the OPERATION regiser)",
#endif
    TCAM_INDIRECT_TCAM_TCAM_INDIRECT_OP_DONE_REG_OFFSET,
    0,
    0,
    697,
};

/******************************************************************************
 * Register: TCAM_INDIRECT_TCAM_TCAM_INDIRECT_ADDR
 ******************************************************************************/
const ru_reg_rec TCAM_INDIRECT_TCAM_TCAM_INDIRECT_ADDR_REG = 
{
    "INDIRECT_TCAM_TCAM_INDIRECT_ADDR",
#if RU_INCLUDE_DESC
    "ADDRESS Register",
    "Key Address to be used in RD/WR opoerations.",
#endif
    TCAM_INDIRECT_TCAM_TCAM_INDIRECT_ADDR_REG_OFFSET,
    0,
    0,
    698,
};

/******************************************************************************
 * Register: TCAM_INDIRECT_TCAM_TCAM_INDIRECT_VLID_IN
 ******************************************************************************/
const ru_reg_rec TCAM_INDIRECT_TCAM_TCAM_INDIRECT_VLID_IN_REG = 
{
    "INDIRECT_TCAM_TCAM_INDIRECT_VLID_IN",
#if RU_INCLUDE_DESC
    "VALID_IN Register",
    "Valid value to be written - this value is relevant during write operation on key0.",
#endif
    TCAM_INDIRECT_TCAM_TCAM_INDIRECT_VLID_IN_REG_OFFSET,
    0,
    0,
    699,
};

/******************************************************************************
 * Register: TCAM_INDIRECT_TCAM_TCAM_INDIRECT_VLID_OUT
 ******************************************************************************/
const ru_reg_rec TCAM_INDIRECT_TCAM_TCAM_INDIRECT_VLID_OUT_REG = 
{
    "INDIRECT_TCAM_TCAM_INDIRECT_VLID_OUT",
#if RU_INCLUDE_DESC
    "VALID_OUT Register",
    "Valid value read from the TCAM - this value is relevant during read operation on key0.",
#endif
    TCAM_INDIRECT_TCAM_TCAM_INDIRECT_VLID_OUT_REG_OFFSET,
    0,
    0,
    700,
};

/******************************************************************************
 * Register: TCAM_INDIRECT_TCAM_TCAM_INDIRECT_RSLT
 ******************************************************************************/
const ru_reg_rec TCAM_INDIRECT_TCAM_TCAM_INDIRECT_RSLT_REG = 
{
    "INDIRECT_TCAM_TCAM_INDIRECT_RSLT",
#if RU_INCLUDE_DESC
    "SEARCH_RESULT Register",
    "The result of a search operation",
#endif
    TCAM_INDIRECT_TCAM_TCAM_INDIRECT_RSLT_REG_OFFSET,
    0,
    0,
    701,
};

/******************************************************************************
 * Register: TCAM_INDIRECT_TCAM_TCAM_INDIRECT_KEY_IN
 ******************************************************************************/
const ru_reg_rec TCAM_INDIRECT_TCAM_TCAM_INDIRECT_KEY_IN_REG = 
{
    "INDIRECT_TCAM_TCAM_INDIRECT_KEY_IN",
#if RU_INCLUDE_DESC
    "KEY_IN %i Register",
    "Key to be used in Write/Compare operations."
    "The Key is 256bit long and is represented by 8 registers. The lower address register correspond to the least significant bits of the key.",
#endif
    TCAM_INDIRECT_TCAM_TCAM_INDIRECT_KEY_IN_REG_OFFSET,
    TCAM_INDIRECT_TCAM_TCAM_INDIRECT_KEY_IN_REG_RAM_CNT,
    4,
    702,
};

/******************************************************************************
 * Register: TCAM_INDIRECT_TCAM_TCAM_INDIRECT_KEY_OUT
 ******************************************************************************/
const ru_reg_rec TCAM_INDIRECT_TCAM_TCAM_INDIRECT_KEY_OUT_REG = 
{
    "INDIRECT_TCAM_TCAM_INDIRECT_KEY_OUT",
#if RU_INCLUDE_DESC
    "KEY_OUT %i Register",
    "Key returned from the CAM in a read operation. The Key is 256bit long and is represented by 8 registers. The lower address register correspond to the least significant bits of the key.",
#endif
    TCAM_INDIRECT_TCAM_TCAM_INDIRECT_KEY_OUT_REG_OFFSET,
    TCAM_INDIRECT_TCAM_TCAM_INDIRECT_KEY_OUT_REG_RAM_CNT,
    4,
    703,
};

/******************************************************************************
 * Register: TCAM_DEBUG_BUS_TCAM_DEBUG_BUS_SELECT
 ******************************************************************************/
const ru_reg_rec TCAM_DEBUG_BUS_TCAM_DEBUG_BUS_SELECT_REG = 
{
    "DEBUG_BUS_TCAM_DEBUG_BUS_SELECT",
#if RU_INCLUDE_DESC
    "SELECT Register",
    "Select",
#endif
    TCAM_DEBUG_BUS_TCAM_DEBUG_BUS_SELECT_REG_OFFSET,
    0,
    0,
    704,
};

/******************************************************************************
 * Block: TCAM
 ******************************************************************************/
static const ru_reg_rec *TCAM_REGS[] =
{
    &TCAM_CONTEXT_RAM_TCAM_TCAM_CONTEXT_RAM_CONTEXT_REG,
    &TCAM_CFG_TCAM_TCAM_CFG_BANK_EN_REG,
    &TCAM_CFG_TCAM_TCAM_CFG_GLOBAL_MASK_REG,
    &TCAM_COUNTERS_TCAM_TCAM_COUNTERS_SRCH_256_REG,
    &TCAM_COUNTERS_TCAM_TCAM_COUNTERS_HIT_256_REG,
    &TCAM_COUNTERS_TCAM_TCAM_COUNTERS_SRCH_512_REG,
    &TCAM_COUNTERS_TCAM_TCAM_COUNTERS_HIT_512_REG,
    &TCAM_INDIRECT_TCAM_TCAM_INDIRECT_OP_REG,
    &TCAM_INDIRECT_TCAM_TCAM_INDIRECT_OP_DONE_REG,
    &TCAM_INDIRECT_TCAM_TCAM_INDIRECT_ADDR_REG,
    &TCAM_INDIRECT_TCAM_TCAM_INDIRECT_VLID_IN_REG,
    &TCAM_INDIRECT_TCAM_TCAM_INDIRECT_VLID_OUT_REG,
    &TCAM_INDIRECT_TCAM_TCAM_INDIRECT_RSLT_REG,
    &TCAM_INDIRECT_TCAM_TCAM_INDIRECT_KEY_IN_REG,
    &TCAM_INDIRECT_TCAM_TCAM_INDIRECT_KEY_OUT_REG,
    &TCAM_DEBUG_BUS_TCAM_DEBUG_BUS_SELECT_REG,
};

unsigned long TCAM_ADDRS[] =
{
    0x82e00000,
};

const ru_block_rec TCAM_BLOCK = 
{
    "TCAM",
    TCAM_ADDRS,
    1,
    16,
    TCAM_REGS
};

/* End of file XRDP_TCAM.c */
