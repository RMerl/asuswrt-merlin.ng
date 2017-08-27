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

#ifndef _XRDP_DRV_NATC_ENG_AG_H_
#define _XRDP_DRV_NATC_ENG_AG_H_

#include "access_macros.h"
#include "bdmf_interface.h"
#ifdef USE_BDMF_SHELL
#include "bdmf_shell.h"
#endif
#include "rdp_common.h"


/**************************************************************************************************/
/* nat_tbl:  - NAT Table Number.0h: NAT table 0.1h: NAT Table 1.2h: NAT table 2.3h: NAT Table 3.4 */
/*          h: NAT table 4.5h: NAT Table 5.6h: NAT table 6.7h: NAT Table 7.                       */
/* multihash_count:  - Multi-hash iteration countValue of 0 is iteration 1, 1 is iteration 2, 2 i */
/*                  s iteration 3, etc.                                                           */
/* cache_hit:  - This bit is set when a LOOKUP command has a cache hit                            */
/* miss:  - This bit is set when a LOOKUP command misses                                          */
/* error:  - This bit is set for the following 2 casesFor Add command, all multi-hash entries are */
/*         occupiedFor Del command, session is not found and cannot be deleted                    */
/* busy:  - Interface BusyThis bit is set when command is issued but the command is still in proc */
/*       ess.                                                                                     */
/* unused0:  -                                                                                    */
/* command:  - Command to be executedThis command only operates on the entries in local cache; DD */
/*          R accessesThe only DDR accesses supported using this register interface is Lookup com */
/*          mand when a miss occurs.Writing to COMMAND bits causes BUSY bit to be set;Note: For a */
/*          ll commands, key consists of all 0's means unused entry in h/wand therefore cannot be */
/*           used.NOPLookupAddDelHash (debug feature)Hashes are stored in different set of COMMAN */
/*          D_STATUS register (i.e.Hashes for HASH command issued using NAT0 register are returne */
/*          dat NAT1 KEY_RESULT registers; hashes for NAT1 HASH commandare returned at NAT0 KEY_R */
/*          ESULT; hashes for NAT2 HASH command arereturned at NAT3 KEY_RESULT; hashes for NAT3 H */
/*          ASH command arereturned at NAT2 KEY_RESULT register).Internal Cache command (debug fe */
/*          ature)Do not use this command                                                         */
/**************************************************************************************************/
typedef struct
{
    uint8_t nat_tbl;
    uint8_t multihash_count;
    bdmf_boolean cache_hit;
    bdmf_boolean miss;
    bdmf_boolean error;
    bdmf_boolean busy;
    bdmf_boolean unused0;
    uint8_t command;
} natc_eng_command_status;


/**************************************************************************************************/
/* nat_key_result:  -                                                                             */
/**************************************************************************************************/
typedef struct
{
    uint32_t data[18];
} natc_eng_key_result;

bdmf_error_t ag_drv_natc_eng_command_status_set(uint8_t eng_idx, const natc_eng_command_status *command_status);
bdmf_error_t ag_drv_natc_eng_command_status_get(uint8_t eng_idx, natc_eng_command_status *command_status);
bdmf_error_t ag_drv_natc_eng_key_mask_set(uint8_t eng_idx, uint32_t key_mask);
bdmf_error_t ag_drv_natc_eng_key_mask_get(uint8_t eng_idx, uint32_t *key_mask);
bdmf_error_t ag_drv_natc_eng_hash_get(uint8_t eng_idx, uint32_t *hash);
bdmf_error_t ag_drv_natc_eng_hit_count_get(uint8_t eng_idx, uint32_t *hit_count);
bdmf_error_t ag_drv_natc_eng_byte_count_get(uint8_t eng_idx, uint32_t *byte_count);
bdmf_error_t ag_drv_natc_eng_pkt_len_set(uint8_t eng_idx, uint16_t unused, uint16_t pkt_len);
bdmf_error_t ag_drv_natc_eng_pkt_len_get(uint8_t eng_idx, uint16_t *unused, uint16_t *pkt_len);
bdmf_error_t ag_drv_natc_eng_key_result_set(uint8_t eng_idx, uint8_t zero, const natc_eng_key_result *key_result);
bdmf_error_t ag_drv_natc_eng_key_result_get(uint8_t eng_idx, uint8_t zero, natc_eng_key_result *key_result);

#ifdef USE_BDMF_SHELL
enum
{
    cli_natc_eng_command_status,
    cli_natc_eng_key_mask,
    cli_natc_eng_hash,
    cli_natc_eng_hit_count,
    cli_natc_eng_byte_count,
    cli_natc_eng_pkt_len,
    cli_natc_eng_key_result,
};

int bcm_natc_eng_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
bdmfmon_handle_t ag_drv_natc_eng_cli_init(bdmfmon_handle_t driver_dir);
#endif


#endif

