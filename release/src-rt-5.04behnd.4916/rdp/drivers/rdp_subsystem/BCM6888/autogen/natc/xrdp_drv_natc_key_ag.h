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


#ifndef _XRDP_DRV_NATC_KEY_AG_H_
#define _XRDP_DRV_NATC_KEY_AG_H_

#include <ru.h>
#include <bdmf_interface.h>
#include <rdp_common.h>

#ifdef USE_BDMF_SHELL
#include <bdmf_shell.h>
#endif

typedef struct
{
    uint32_t data[8];
} natc_key_mask;


/**********************************************************************************************************************
 * key_mask: 
 *     Specifies the key mask for each bit in the key.
 *     For 16-byte key, there are corresponding 4 KEY_MASK registers.
 *     For 32-byte key, there are corresponding 8 KEY_MASK registers.
 *     Each bit in each KEY_MASK register corresponds to one bit in the key.
 *     0 enables the compare and 1 disables the compare.
 *     bit 0 of KEY_MASK register[0] corresponds to key bit 0.
 *     bit 1 of KEY_MASK register[0] corresponds to key bit 1.
 *     bit 2 of KEY_MASK register[0] corresponds to key bit 2.
 *     ......................
 *     bit 31 of KEY_MASK register [0] corresponds to key bit 31.
 *     bit 0 of KEY_MASK register[1] corresponds to key bit 32.
 *     bit 1 of KEY_MASK register[1] corresponds to key bit 33.
 *     bit 2 of KEY_MASK register[1] corresponds to key bit 34.
 *     ......................
 *     bit 31 of KEY_MASK register [1] corresponds to key bit 63.
 *     ......................
 *     bit 0 of KEY_MASK register[7] corresponds to key bit 224.
 *     bit 1 of KEY_MASK register[7] corresponds to key bit 225.
 *     bit 2 of KEY_MASK register[7] corresponds to key bit 226.
 *     ......................
 *     bit 31 of KEY_MASK register [7] corresponds to key bit 255.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_natc_key_mask_set(uint8_t tbl_idx, uint8_t zero, const natc_key_mask *mask);
bdmf_error_t ag_drv_natc_key_mask_get(uint8_t tbl_idx, uint8_t zero, natc_key_mask *mask);

#ifdef USE_BDMF_SHELL
enum
{
    cli_natc_key_mask,
};

int bcm_natc_key_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
bdmfmon_handle_t ag_drv_natc_key_cli_init(bdmfmon_handle_t root_dir);

#endif
#endif
