// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2015 Broadcom Corporation
   All Rights Reserved

    
*/

#include "ru.h"

/******************************************************************************
 * Chip: BCM6858_A0
 ******************************************************************************/
const ru_block_rec *RU_LPORT_BLOCKS[] =
{
    &LPORT_XLMAC_BLOCK,
    &LPORT_MIB_BLOCK,
    &LPORT_SRDS_BLOCK,
    &LPORT_LED_BLOCK,
    &LPORT_CTRL_BLOCK,
    &LPORT_RGMII_BLOCK,
    &XLMAC_CONF_BLOCK,
    &MIB_CONF_BLOCK,
    &LPORT_MDIO_BLOCK,
    &LPORT_INTR_BLOCK,
    &LPORT_MAB_BLOCK,
    NULL
};

/* End of file BCM6858_A0.c */
