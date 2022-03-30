// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2015 Broadcom
   All Rights Reserved

    
*/

#include "ru.h"

const ru_reg_rec UNIMAC_RDP_COMMAND_CONFIG_REG = 
{
    "COMMAND_CONFIG",
#if RU_INCLUDE_DESC
    "Command register. Used by the host processor to control and configure the core",
    "",
#endif
    UNIMAC_RDP_COMMAND_CONFIG_REG_OFFSET,
    0,
    0,
    807,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};

/******************************************************************************
 * Block: UNIMAC_RDP
 ******************************************************************************/
static const ru_reg_rec *UNIMAC_RDP_REGS[] =
{
    &UNIMAC_RDP_COMMAND_CONFIG_REG,
};

unsigned long UNIMAC_RDP_ADDRS[] =
{
    0x828a8004,
    0x828a9004,
    0x828aa004,
    0x828ab004,
    0x828ac004,
    0x828ad004,
    0x828ae004,
    0x828af004,
};

const ru_block_rec UNIMAC_RDP_BLOCK = 
{
    "UNIMAC_RDP",
    UNIMAC_RDP_ADDRS,
    8,
    177,
    UNIMAC_RDP_REGS
};

/* End of file XRDP_UNIMAC_RDP.c */
