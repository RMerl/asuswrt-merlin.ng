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
 * Chip: EPON
 ******************************************************************************/
const ru_block_rec *RU_EPON_BLOCKS[] =
{
    &EPON_TOP_BLOCK,
    &CLK_PRG_SWCH_ADDR_BLOCK,
    &EPN_ONU_MAC_ADDR_BLOCK,
    &EPN_TX_L1S_SHP_BLOCK,
    &EPN_BLOCK,
    &LIF_BLOCK,
    &NCO_ADDR_BLOCK,
    &XIF_BLOCK,
    &XPCSRX_BLOCK,
    &XPCSTX_BLOCK,
    NULL
};

/* End of file EPON.c */
