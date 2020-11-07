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
 * Chip: 
 ******************************************************************************/
const ru_block_rec *WAN_TOP_BLOCKS[] =
{
    &TOP_SCRATCH_BLOCK,
    &GPON_BLOCK,
    &EARLY_TXEN_BLOCK,
    &RESCAL_BLOCK,
    &MISC_BLOCK,
    &PMI_BLOCK,
    &TOD_BLOCK,
    &SERDES_STATUS_BLOCK,
    &GPON_GEARBOX_STATUS_BLOCK,
    &FORCE_LBE_CONTROL_BLOCK,
    &WAN_SERDES_BLOCK,
    &TOP_OSR_BLOCK,
    NULL
};

/* End of file .c */
