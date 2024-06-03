// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2015 Broadcom
   All Rights Reserved

*/

#include "ru.h"

/******************************************************************************
 * Chip: 
 ******************************************************************************/
const ru_block_rec *RU_XPORT_BLOCKS[] =
{
    &XPORT_XLMAC_CORE_BLOCK,
    &XPORT_MIB_CORE_BLOCK,
    &XPORT_TOP_BLOCK,
    &XPORT_XLMAC_REG_BLOCK,
    &XPORT_MIB_REG_BLOCK,
    &XPORT_INTR_BLOCK,
    &XPORT_MAB_BLOCK,
    &XPORT_PORTRESET_BLOCK,
    &XPORT_WOL_MPD_BLOCK,
    &XPORT_WOL_ARD_BLOCK,
    NULL
};

/* End of file .c */
