// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2015 Broadcom
   All Rights Reserved

    
*/

#include "ru.h"

/******************************************************************************
 * Register: SBPM_REGS_BN_ALLOC
 ******************************************************************************/
const ru_reg_rec SBPM_REGS_BN_ALLOC_REG = 
{
    "REGS_BN_ALLOC",
#if RU_INCLUDE_DESC
    "BN_ALLOC Register",
    "request for a new buffer",
#endif
    SBPM_REGS_BN_ALLOC_REG_OFFSET,
    0,
    0,
    682,
};

/******************************************************************************
 * Register: SBPM_REGS_BN_ALLOC_RPLY
 ******************************************************************************/
const ru_reg_rec SBPM_REGS_BN_ALLOC_RPLY_REG = 
{
    "REGS_BN_ALLOC_RPLY",
#if RU_INCLUDE_DESC
    "BN_ALLOC_RPLY Register",
    "reply for a new buffer alloc",
#endif
    SBPM_REGS_BN_ALLOC_RPLY_REG_OFFSET,
    0,
    0,
    683,
};

/******************************************************************************
 * Register: SBPM_REGS_BN_CONNECT
 ******************************************************************************/
const ru_reg_rec SBPM_REGS_BN_CONNECT_REG = 
{
    "REGS_BN_CONNECT",
#if RU_INCLUDE_DESC
    "BN_CONNECT Register",
    "request for connection between two buffers in a linked list. The connection request may be replied with ACK message if the ACK request bit is asserted."
    "This command is used as write command.",
#endif
    SBPM_REGS_BN_CONNECT_REG_OFFSET,
    0,
    0,
    688,
};

/******************************************************************************
 * Register: SBPM_REGS_BN_CONNECT_RPLY
 ******************************************************************************/
const ru_reg_rec SBPM_REGS_BN_CONNECT_RPLY_REG = 
{
    "REGS_BN_CONNECT_RPLY",
#if RU_INCLUDE_DESC
    "BN_CONNECT_RPLY Register",
    "bn_connect_rply",
#endif
    SBPM_REGS_BN_CONNECT_RPLY_REG_OFFSET,
    0,
    0,
    689,
};

/******************************************************************************
 * Register: SBPM_REGS_GET_NEXT
 ******************************************************************************/
const ru_reg_rec SBPM_REGS_GET_NEXT_REG = 
{
    "REGS_GET_NEXT",
#if RU_INCLUDE_DESC
    "GET_NEXT Register",
    "a pointer to a buffer in a packet linked list and request for the next buffer in the list"
    "this command is used as read command.",
#endif
    SBPM_REGS_GET_NEXT_REG_OFFSET,
    0,
    0,
    690,
};

/******************************************************************************
 * Register: SBPM_REGS_GET_NEXT_RPLY
 ******************************************************************************/
const ru_reg_rec SBPM_REGS_GET_NEXT_RPLY_REG = 
{
    "REGS_GET_NEXT_RPLY",
#if RU_INCLUDE_DESC
    "GET_NEXT_RPLY Register",
    "get_next_rply",
#endif
    SBPM_REGS_GET_NEXT_RPLY_REG_OFFSET,
    0,
    0,
    691,
};

/******************************************************************************
 * Register: SBPM_REGS_BN_FREE_WITHOUT_CONTXT
 ******************************************************************************/
const ru_reg_rec SBPM_REGS_BN_FREE_WITHOUT_CONTXT_REG = 
{
    "REGS_BN_FREE_WITHOUT_CONTXT",
#if RU_INCLUDE_DESC
    "BN_FREE_WITHOUT_CONTXT Register",
    "bn_free_without_contxt",
#endif
    SBPM_REGS_BN_FREE_WITHOUT_CONTXT_REG_OFFSET,
    0,
    0,
    693,
};

/******************************************************************************
 * Register: SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY
 ******************************************************************************/
const ru_reg_rec SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_REG = 
{
    "REGS_BN_FREE_WITHOUT_CONTXT_RPLY",
#if RU_INCLUDE_DESC
    "BN_FREE_WITHOUT_CONTXT_RPLY Register",
    "bn_free_without_contxt_rply",
#endif
    SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_REG_OFFSET,
    0,
    0,
    694,
};


/******************************************************************************
 * Block: SBPM
 ******************************************************************************/
static const ru_reg_rec *SBPM_REGS[] =
{
    &SBPM_REGS_BN_ALLOC_REG,
    &SBPM_REGS_BN_ALLOC_RPLY_REG,
    &SBPM_REGS_BN_CONNECT_REG,
    &SBPM_REGS_BN_CONNECT_RPLY_REG,
    &SBPM_REGS_GET_NEXT_REG,
    &SBPM_REGS_GET_NEXT_RPLY_REG,
    &SBPM_REGS_BN_FREE_WITHOUT_CONTXT_REG,
    &SBPM_REGS_BN_FREE_WITHOUT_CONTXT_RPLY_REG,
};

unsigned long SBPM_ADDRS[] =
{
    0x828a1000,
};

const ru_block_rec SBPM_BLOCK = 
{
    "SBPM",
    SBPM_ADDRS,
    1,
    53,
    SBPM_REGS
};

/* End of file XRDP_SBPM.c */
