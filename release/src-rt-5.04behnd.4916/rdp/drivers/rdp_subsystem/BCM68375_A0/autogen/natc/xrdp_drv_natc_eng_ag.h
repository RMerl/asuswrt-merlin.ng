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

#include <ru.h>
#include <bdmf_interface.h>
#include <rdp_common.h>

#ifdef USE_BDMF_SHELL
#include <bdmf_shell.h>
#endif

typedef struct
{
    uint8_t command;
    bdmf_boolean unused5;
    bdmf_boolean busy;
    bdmf_boolean error;
    bdmf_boolean miss;
    bdmf_boolean cache_hit;
    uint8_t multihash_count;
    uint8_t nat_tbl;
    bdmf_boolean decr_count;
    bdmf_boolean cache_flush;
    bdmf_boolean del_cmd_mode;
    bdmf_boolean add_cmd_mode;
    uint8_t del_cmd_ddr_bin;
    uint8_t add_cmd_ddr_bin;
    bdmf_boolean add_cmd_ddr_miss;
} natc_eng_command_status;

typedef struct
{
    uint32_t data[18];
} natc_eng_key_result;


/**********************************************************************************************************************
 * command: 
 *     Command to be executed
 *     This command only operates on the entries in cache except LOOKUP command where
 *     entry can be fetched from DDR.
 *     Writing to this field causes BUSY bit to be set.
 *     Note: For all commands, key consists of all 0's indicates unused entry in h/w
 *     and therefore cannot be used.
 * unused5: 
 * busy: 
 *     Interface Busy
 *     This bit is set when command is issued but still in progress been processed.
 *     When command completes this bit will be cleared.
 * error: 
 *     This bit is set for the following 2 cases
 *     For ADD command all multi-hash entries are occupied (i.e, no room to ADD)
 *     For DEL command entry is not found and cannot be deleted
 * miss: 
 *     This bit is set when a LOOKUP command has a miss
 * cache_hit: 
 *     This bit is set when a LOOKUP command has a cache hit
 * multihash_count: 
 *     Cache multi-hash iteration count status
 *     Value of 0 is iteration 1, 1 is iteration 2, 2 is iteration 3, etc.
 *     cache miss returns 0 count.
 * nat_tbl: 
 *     Select the DDR Table on which the command will operate
 *     0h: DDR table 0
 *     1h: DDR table 1
 *     2h: DDR table 2
 *     3h: DDR table 3
 *     4h: DDR table 4
 *     5h: DDR table 5
 *     6h: DDR table 6
 *     7h: DDR table 7
 * decr_count: 
 *     Decrement-counter mode enable
 *     When set, LOOKUP command will decrement hit counter by 1 and decrement
 *     byte counter by the value specified in PKT_LEN, on a successful lookup.
 *     NATC_SMEM_INCREMENT_ON_REG_LOOKUP must be set to 1 for it to be effective
 *     0h: LOOKUP command will increment hit counter and byte counter
 *     1h: LOOKUP command will decrement hit counter and byte counter
 * cache_flush: 
 *     Cache Flush enable
 *     When set, LOOKUP command is used to flush counters from cache into DDR.
 *     This command does not use key to lookup the cache entry.  Instead it uses
 *     cache index number located in 10-MSB bits of key specified in NAT_KEY_RESULT register.
 *     For 16 bytes key, the cache index will be located in
 *     {NAT_KEY_RESULT[15], NAT_KEY_RESULT[14][7:6]} (15th byte of NAT_KEY_RESULT register and
 *     bits 7:6 of 14th byte of NAT_KEY_RESULT register).
 *     For 32 bytes key, the cache index will be located in
 *     {NAT_KEY_RESULT[31], NAT_KEY_RESULT[30][7:6]} (31th byte of NAT_KEY_RESULT register and
 *     bits 7:6 of 30th byte of NAT_KEY_RESULT register).
 *     0h: LOOKUP command is used as normal lookup command.
 *     1h: LOOKUP command is used as cache flush command.
 * del_cmd_mode: 
 *     DEL Command DDR-bin matching mode enable
 *     0h: DEL command deletes the cache entry with matching key
 *     1h: DEL command deletes the cache entry with matching key and matching DDR bin
 *     number specified in DEL_CMD_DDR_BIN field
 * add_cmd_mode: 
 *     ADD Command mode
 *     0h: ADD command writes 0 (DDR bin number and DDR miss flag) to cache.
 *     1h: ADD command writes DDR bin number and DDR miss flag to cache
 *     specified in ADD_CMD_DDR_BIN and ADD_CMD_DDR_MISS fields.
 * del_cmd_ddr_bin: 
 *     This filed specifies the DDR BIN number to be compared for DEL command
 *     when DEL_CMD_MODE is set to 1
 * add_cmd_ddr_bin: 
 *     This filed specifies the DDR BIN number to be written to cache for ADD command
 *     when ADD_CMD_MODE is set to 1.
 * add_cmd_ddr_miss: 
 *     This filed specifies the DDR MISS flag to be written to cache for ADD command
 *     when ADD_CMD_MODE is set to 1.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_natc_eng_command_status_set(uint8_t eng_idx, const natc_eng_command_status *command_status);
bdmf_error_t ag_drv_natc_eng_command_status_get(uint8_t eng_idx, natc_eng_command_status *command_status);

/**********************************************************************************************************************
 * hash: 
 *     hash value; only valid on a successful lookup/add/del command
 *     For cache hit 10-bit hash value is returned.
 *     For cache miss and DDR_ENABLE is 0, first hash value (10-bit) is returned.
 *     For cache miss, DDR_ENABLE is 1 and DDR is a hit, 18-bit DDR hash value + DDR bin count is returned.
 *     For cache miss, DDR_ENABLE is 1 and DDR is a miss, 18-bit DDR hash value is returned.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_natc_eng_hash_get(uint8_t eng_idx, uint32_t *hash);

/**********************************************************************************************************************
 * hit_count: 
 *     bits 27:0 are 28-bit hit count value.
 *     bits 31:28 are 4 lsb of 36-bit byte count value.
 *     only valid on a successful lookup or delete command.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_natc_eng_hit_count_get(uint8_t eng_idx, uint32_t *hit_count);

/**********************************************************************************************************************
 * byte_count: 
 *     32-bit msb of 36-bit byte count value.
 *     {BYTE_COUNT, HIT_COUNT[31:28]} is the 36-bit byte count value.
 *     only valid on a successful lookup or delete command
 **********************************************************************************************************************/
bdmf_error_t ag_drv_natc_eng_byte_count_get(uint8_t eng_idx, uint32_t *byte_count);

/**********************************************************************************************************************
 * pkt_len: 
 *     16-bit packet length value used to increment or decrement byte counter
 * unused: 
 **********************************************************************************************************************/
bdmf_error_t ag_drv_natc_eng_pkt_len_set(uint8_t eng_idx, uint16_t pkt_len, uint16_t unused);
bdmf_error_t ag_drv_natc_eng_pkt_len_get(uint8_t eng_idx, uint16_t *pkt_len, uint16_t *unused);

/**********************************************************************************************************************
 * nat_key_result: 
 **********************************************************************************************************************/
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
bdmfmon_handle_t ag_drv_natc_eng_cli_init(bdmfmon_handle_t root_dir);

#endif
#endif
