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
