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
/* add_cmd_ddr_miss:  - This filed specifies the DDR MISS flag to be written to cache for ADD com */
/*                   mandwhen ADD_CMD_MODE is set to 1.                                           */
/* add_cmd_ddr_bin:  - This filed specifies the DDR BIN number to be written to cache for ADD com */
/*                  mandwhen ADD_CMD_MODE is set to 1.                                            */
/* del_cmd_ddr_bin:  - This filed specifies the DDR BIN number to be compared for DEL commandwhen */
/*                   DEL_CMD_MODE is set to 1                                                     */
/* add_cmd_mode:  - ADD Command mode0h: ADD command writes 0 (DDR bin number and DDR miss flag) t */
/*               o cache.1h: ADD command writes DDR bin number and DDR miss flag to cachespecifie */
/*               d in ADD_CMD_DDR_BIN and ADD_CMD_DDR_MISS fields.                                */
/* del_cmd_mode:  - DEL Command DDR-bin matching mode enable0h: DEL command deletes the cache ent */
/*               ry with matching key1h: DEL command deletes the cache entry with matching key an */
/*               d matching DDR binnumber specified in DEL_CMD_DDR_BIN field                      */
/* cache_flush:  - Cache Flush enableWhen set, LOOKUP command is used to flush counters from cach */
/*              e into DDR.This command does not use key to lookup the cache entry.  Instead it u */
/*              sescache index number located in 10-MSB bits of key specified in NAT_KEY_RESULT r */
/*              egister.For 16 bytes key, the cache index will be located in{NAT_KEY_RESULT[15],  */
/*              NAT_KEY_RESULT[14][7:6]} (15th byte of NAT_KEY_RESULT register andbits 7:6 of 14t */
/*              h byte of NAT_KEY_RESULT register).For 32 bytes key, the cache index will be loca */
/*              ted in{NAT_KEY_RESULT[31], NAT_KEY_RESULT[30][7:6]} (31th byte of NAT_KEY_RESULT  */
/*              register andbits 7:6 of 30th byte of NAT_KEY_RESULT register).0h: LOOKUP command  */
/*              is used as normal lookup command.1h: LOOKUP command is used as cache flush comman */
/*              d.                                                                                */
/* decr_count:  - Decrement-counter mode enableWhen set, LOOKUP command will decrement hit counte */
/*             r by 1 and decrementbyte counter by the value specified in PKT_LEN, on a successfu */
/*             l lookup.NATC_SMEM_INCREMENT_ON_REG_LOOKUP must be set to 1 for it to be effective */
/*             0h: LOOKUP command will increment hit counter and byte counter1h: LOOKUP command w */
/*             ill decrement hit counter and byte counter                                         */
/* nat_tbl:  - Select the DDR Table on which the command will operate0h: DDR table 01h: DDR table */
/*           12h: DDR table 23h: DDR table 34h: DDR table 45h: DDR table 56h: DDR table 67h: DDR  */
/*          table 7                                                                               */
/* multihash_count:  - Cache multi-hash iteration count statusValue of 0 is iteration 1, 1 is ite */
/*                  ration 2, 2 is iteration 3, etc.cache miss returns 0 count.                   */
/* cache_hit:  - This bit is set when a LOOKUP command has a cache hit                            */
/* miss:  - This bit is set when a LOOKUP command has a miss                                      */
/* error:  - This bit is set for the following 2 casesFor ADD command all multi-hash entries are  */
/*        occupied (i.e, no room to ADD)For DEL command entry is not found and cannot be deleted  */
/* busy:  - Interface BusyThis bit is set when command is issued but still in progress been proce */
/*       ssed.When command completes this bit will be cleared.                                    */
/* unused0:  -                                                                                    */
/* command:  - Command to be executedThis command only operates on the entries in cache except LO */
/*          OKUP command whereentry can be fetched from DDR.Writing to this field causes BUSY bit */
/*           to be set.Note: For all commands, key consists of all 0's indicates unused entry in  */
/*          h/wand therefore cannot be used.No-OperationLookupAdd (to cache only)Del (from cache  */
/*          only)Hash (debug command)Internal Cache command (debug command)Do not use this comman */
/*          d                                                                                     */
/**************************************************************************************************/
typedef struct
{
    bdmf_boolean add_cmd_ddr_miss;
    uint8_t add_cmd_ddr_bin;
    uint8_t del_cmd_ddr_bin;
    bdmf_boolean add_cmd_mode;
    bdmf_boolean del_cmd_mode;
    bdmf_boolean cache_flush;
    bdmf_boolean decr_count;
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

