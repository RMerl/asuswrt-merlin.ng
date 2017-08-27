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

#ifndef _XRDP_DRV_NATC_AG_H_
#define _XRDP_DRV_NATC_AG_H_

#include "access_macros.h"
#include "bdmf_interface.h"
#ifdef USE_BDMF_SHELL
#include "bdmf_shell.h"
#endif
#include "rdp_common.h"


/**************************************************************************************************/
/* ddr_enable:  - Enables NAT table offload to DDR functionality.NATC_CONTROL_STATUS2 register sh */
/*             ould be configured before enabling this feature.                                   */
/* natc_add_command_speedup_mode:  - Default behavior for an ADD command is to do a LOOKUP first  */
/*                                to see if the entrywith the same key already exists and replace */
/*                                 it; this is to avoid having duplicatedentries in the table for */
/*                                 ADD command.  When this bit is set an ADD command willeither r */
/*                                eplace the entry with the matched key or add an entry to an emp */
/*                                ty entrydepending on whichever one is encountered first during  */
/*                                multi-hash.  Enablingthis bit speeds up the ADD command.        */
/* unused4:  -                                                                                    */
/* smem_32bit_in_64bit_swap_control:  - Swap 32-bit word within 64-bit word for NAT statistics (c */
/*                                   ounter) memory accesses(i.e., [63:0] becomes {[31:0], [63:32 */
/*                                   ]})                                                          */
/* smem_8bit_in_32bit_swap_control:  - Reverse bytes within 32-bit word for NAT statistics (count */
/*                                  er) memory accesses(i.e., [31:0] becomes {[7:0], [15,8], [23, */
/*                                  16], [31,24]})                                                */
/* ddr_swap_all_control:  - Swap all bytes on DDR interface.This bit should be set to 1 in Little */
/*                        Endian mode.                                                            */
/* unused3:  -                                                                                    */
/* reg_32bit_in_64bit_swap_control:  - Swap 32-bit word within 64-bit word for key_result registe */
/*                                  r accesses(i.e., [63:0] becomes {[31:0], [63:32]})            */
/* reg_8bit_in_32bit_swap_control:  - Reverse bytes within 32-bit word for key_result register ac */
/*                                 cesses(i.e., [31:0] becomes {[7:0], [15,8], [23,16], [31,24]}) */
/* unused2:  -                                                                                    */
/* total_len:  - Total length of the lookup key + result (including 4-byte byte count and 4-byte  */
/*            hit count).The result length  (including 4-byte byte count and 4-byte hit count)sho */
/*            uld be TOTAL_LEN - KEY_LEN.The default value of this register will be from harware  */
/*            DEFINE which defines thetotal length of the nat cache memory and Statitics Memory ( */
/*            4-byte byte count and 4-byte hit count).0h: 48-byte1h: 64-byte2h: 80-byte3h: 96-byt */
/*            e4h: 112-byte5h: 128-byte6h: Not used7h: Not used                                   */
/* ddr_disable_on_reg_lookup:  - 0h: Enable DDR lookup on register lookup miss1h: Disable DDR loo */
/*                            kup on register lookup cache miss                                   */
/* unused1:  -                                                                                    */
/* nat_hash_mode:  - 0h: 32-bit rolling XOR hash is used as nat hash function.1h: CRC32 hash is u */
/*                sed as NAT hash function. CRC32 is reduced to 16-bit usingthe same method as th */
/*                e one used in 32-bit rolling XOR hash.2h: CRC32 hash is used as NAT hash functi */
/*                on. CRC32[15:0] is used as hash value3h: CRC32 hash is used as NAT hash functio */
/*                n. CRC32[31:16] is used as hash value.                                          */
/* multi_hash_limit:  - Maximum number of multi-hash iterationsValue of 0 is 1 time, 1 is 2 times */
/*                   , 2 is 3 times, etc.                                                         */
/* unused0:  -                                                                                    */
/* nat_arb_st:  - NAT Arbitration MechanismRound-robin arbitrationStrict priority arbitrationlist */
/*             ed from highest to lowest priority --  NAT0, NAT1, NAT2, NAT3, RunnerStrict priori */
/*             ty arbitration (priority reversed from above)listed from highest to lowest priorit */
/*             y --  Runner, NAT3, NAT2, NAT1, NAT0                                               */
/* natc_smem_increment_on_reg_lookup:  - Enables incrementing NAT hit counter by 1 andNAT byte co */
/*                                    unter by PKT_LEN defined in NAT_PKT_LEN registeron successf */
/*                                    ul lookups using register interfaceBY default, NAT counter  */
/*                                    memory only increments on successful lookups by Runner.     */
/* natc_smem_clear_by_update_disable:  - Disables clearing NAT counter (hit count and byte count) */
/*                                     memory when an existing entry is replaced by ADD command   */
/* natc_smem_disable:  - Disables NAT counter memory from incrementing for a lookup hit.          */
/* natc_enable:  - Enables all NATC state machines and input fifo;Clearing this bit from a 1 will */
/*               halt all state machines gracefully to idle states,all outstanding transactions i */
/*              n the fifo will remain in the fifo and inputfifo will also be disable;  All NATC  */
/*              configuration registers should be configuredbefore enabling this bit.             */
/* natc_reset:  - Block Reset (including resetting all registers to default values)Self clear.    */
/* ddr_hash_swap:  - Reverse bytes within 16-bit DDR hash value                                   */
/* ddr_hash_mode:  - 0h: 32-bit rolling XOR hash is used as DDR hash function.1h: CRC32 hash is u */
/*                sed as DDR hash function. CRC32 is reduced to 16-bit usingthe same method as th */
/*                e one used in 32-bit rolling XOR hash.2h: CRC32 hash is used as DDR hash functi */
/*                on. CRC32[15:0] is used as hash value3h: CRC32 hash is used as DDR hash functio */
/*                n. CRC32[31:16] is used as hash value.                                          */
/* ddr_32bit_in_64bit_swap_control:  - Swap 32-bit word within 64-bit word for NAT DDR memory rea */
/*                                  d/write accesses(i.e., [63:0] becomes {[31:0], [63:32]}).This */
/*                                   bit should be set to 1 in Little Endian mode.                */
/* ddr_8bit_in_32bit_swap_control:  - Reverse bytes within 32-bit word for NAT DDR memory read/wr */
/*                                 ite accesses(i.e., [31:0] becomes {[7:0], [15,8], [23,16], [31 */
/*                                 ,24]}).This bit should be set to 1 in Little Endian mode.      */
/* cache_lookup_blocking_mode:  - 1h: Local cache lookup will be blocked if the lookup keys have  */
/*                             the same hash valueas the oustanding DDR searches. This is used wh */
/*                             en the ordering whthin the sameflow is to be preserved.0h: Local c */
/*                             ache will not be blocked during multiple outstanding DDR searches. */
/*                             This is used where search ordering preservation is not required.Th */
/*                             e hit count and byte count in local cache are not accurate in this */
/*                              mode.The scenario is as follows when the same key is used.1. Firs */
/*                             t key lookup, it is not in nat cache, it proceeds with ddr lookup. */
/*                             2. Second key lookup, it is still not in nat cache. It proceeds wi */
/*                             th another ddr lookup.3. The key/result/hit count/byte count from  */
/*                             #1 ddr lookup returns and writes to local cache.4. Other lookup ca */
/*                             uses local cache enrty from #3 to be evicted.5. The key/results/hi */
/*                             t count/byte count (hit count and byte count value in ddr before o */
/*                             verridden by eviction) from #2 ddr lookup returnsand it writes the */
/*                              hit count/byte count into local cache.This causes the hit count i */
/*                             n local cache to be one less than correct hit count.This also caus */
/*                             es the byte count in local cache to be one packet length less than */
/*                              correct byte count.                                               */
/* age_timer_tick:  - Timer tick for pseudo-LRUTimer is incremented on every system clock cycleTi */
/*                 mer is incremented on every packet arrival to NAT block                        */
/* age_timer:  - Timer value used for pseudo-LRU;When timer fires the 8-bit age value of every en */
/*            try in the cache isdecremented (cap at 0).  The entry with lower value isthe older  */
/*            entry.  The default setting keeps track of 2s age at~7ms resolution.0: 1 tick1: 2 t */
/*            icks2: 4 ticks3: 8 ticks4: 16 ticks....31: 2^31 TICKS                               */
/* cache_algo:  - Replacement algorithm for cachingLowest-multi-hash-iteration number is used to  */
/*             select the final replacemententry if multiple entries were chosen by the selected  */
/*             algorithm.  Forinstance, if HIT_COUNT algorithm were selected, and 2nd, 3rd and 7t */
/*             hentry all have the same hit_count values, 2nd entry will be evicted.If CACHE_DISA */
/*             BLE or EVICTION_DISABLE is set, HIT_COUNT algorithmcan only keep track of the hit  */
/*             count while the session is in the cache.When the session is evicted hit count for  */
/*             that session is lost.Replacement algorithm prioritizes pseudo-LRU over lowest-hit- */
/*             countReplacement algorithm prioritizes lowest-hit-count over pseudo-LRUReplacement */
/*              algorithm uses pseudo-LRUReplacement algorithm uses least-hit-countReplacement al */
/*             gorithm prioritizes pseudo-LRU over pseudo-randomReplacement algorithm prioritizes */
/*              lowest-hit-count over pseudo-randomReplacement algorithm uses pseudo-random algor */
/*             ithmReplacement algorithm prioritizes highest-hit-count overmost-recently-useRepla */
/*             cement algorithm prioritizes pseudo-LRU over lowest-byte-countReplacement algorith */
/*             m prioritizes lowest-byte-count over pseudo-LRUReplacement algorithm uses least-by */
/*             te-countReplacement algorithm prioritizes lowest-byte-count over pseudo-randomRepl */
/*             acement algorithm prioritizes highest-byte-count overmost-recently-use             */
/* ddr_bins_per_bucket:  - Number of entries per bucket - 1.  Number of entries within a bucket i */
/*                      slimited by bus max burst size.  For instance, if UBUS supports max burst */
/*                      size of 512 bytes, key length is 16 bytes, maximum DDR_BINS_PER_BUCKET th */
/*                      at can be programmedis 512 bytes / 16-bytes (bytes per bin) = 32 entries. */
/*                      0h: 1 entry1h: 2 entries2h: 3 entries3h: 4 entries4h: 5 entries.......... */
/*                      ...                                                                       */
/* ddr_size:  - Number of NAT entries in DDR NAT table.Add DDR_BINS_PER_BUCKET field to the table */
/*            size selection below to computethe actual size of the table.For instance, if DDR_BI */
/*           NS_PER_BUCKET is 3 (4 bins per bucket)and DDR_size is 3 (64k entries), the size of t */
/*           he table in DDR is(64*1024+3) multiply by total length of key and results in bytes(d */
/*           efined by TOTAL_LEN in NATC_CONTROL_STATUS register).Extra 3 entries are used to sto */
/*           recollided entries of the last entry of the desire table size.64k entries32k entries */
/*           16k entries8k entries                                                                */
/* cache_update_on_reg_ddr_lookup:  - 1h: On a register interface Lookup command,if it is a cache */
/*                                  miss, NAT local cache will be updated with valuefetched from  */
/*                                 a successful DDR lookup.The hit count and byte count returned  */
/*                                 back to the register interface will bethe hit count and byte c */
/*                                 ount in SMEM (counter memory) after the update.0h: On a regist */
/*                                 er interface Lookup command,if it is a cache miss, NAT local c */
/*                                 ache will not be updated with valuefetched from a successful D */
/*                                 DR lookup.If it is a cache hit, the hit count and byte count r */
/*                                 eturned back to the register interfacewill be the hit count an */
/*                                 d byte count in SMEM (counter memory) after the update.If it i */
/*                                 s a cache miss, the hit count and byte count returned back to  */
/*                                 the register interfacewill be the hit count and byte count ret */
/*                                 urned from DDR.                                                */
/* ddr_counter_8bit_in_32bit_swap_control:  - Reverse bytes within 32-bit word for NAT DDR counte */
/*                                         rs (hit count and byte count) read/write accesses.(i.e */
/*                                         ., [31:0] becomes {[7:0], [15,8], [23,16], [31,24]})   */
/* ddr_write_w_ack_enable:  - Enable write w/ack for DDR accessesDefault is to use write w/o ack  */
/* ddr_replace_duplicated_cached_entry_enable:  - Enable replacing local hit count and byte count */
/*                                              with DDR fetched entry when the entryalready exis */
/*                                             ted in cacheThis is a debug feature, do not set th */
/*                                             is bit                                             */
/* ddr_lookup_pending_fifo_mode_disable:  - 0h: If DDR lookup keys have the same hash value as an */
/*                                        outstanding DDR searches,DDR search request will be for */
/*                                       dwarded to a pending lookup FIFO(instead of forwarding t */
/*                                       o DDR control). The requestin pending lookup FIFO will b */
/*                                       e read to perform local cache lookup after thepending DD */
/*                                       R search with the same key hash value has returned from  */
/*                                       DDR controland completes the local cache lookup.1h: DDR  */
/*                                       lookup pending FIFO mode is disabled. All DDR lookup sea */
/*                                       rch requestsare forwarded to DDR control regardless of t */
/*                                       he key hash value.                                       */
/* eviction_disable:  - Disable session hit count and byte count eviction back to DDR.Set this bi */
/*                   t when session hit count and byte count values are not used and the accesses */
/*                   to DDR will be reduced by 1/2 to speed up overall NAT operation; this bitis  */
/*                   effective when CACHE_DISABLE is 0; this bit also affects CACHE_ALGOif HIT_CO */
/*                   UNT and BYTE_COUNT are used.                                                 */
/**************************************************************************************************/
typedef struct
{
    bdmf_boolean ddr_enable;
    bdmf_boolean natc_add_command_speedup_mode;
    uint8_t unused4;
    bdmf_boolean smem_32bit_in_64bit_swap_control;
    bdmf_boolean smem_8bit_in_32bit_swap_control;
    bdmf_boolean ddr_swap_all_control;
    bdmf_boolean unused3;
    bdmf_boolean reg_32bit_in_64bit_swap_control;
    bdmf_boolean reg_8bit_in_32bit_swap_control;
    uint8_t unused2;
    uint8_t total_len;
    bdmf_boolean ddr_disable_on_reg_lookup;
    bdmf_boolean unused1;
    uint8_t nat_hash_mode;
    uint8_t multi_hash_limit;
    bdmf_boolean unused0;
    uint8_t nat_arb_st;
    bdmf_boolean natc_smem_increment_on_reg_lookup;
    bdmf_boolean natc_smem_clear_by_update_disable;
    bdmf_boolean natc_smem_disable;
    bdmf_boolean natc_enable;
    bdmf_boolean natc_reset;
    bdmf_boolean ddr_hash_swap;
    uint8_t ddr_hash_mode;
    bdmf_boolean ddr_32bit_in_64bit_swap_control;
    bdmf_boolean ddr_8bit_in_32bit_swap_control;
    bdmf_boolean cache_lookup_blocking_mode;
    bdmf_boolean age_timer_tick;
    uint8_t age_timer;
    uint8_t cache_algo;
    uint8_t ddr_bins_per_bucket;
    uint8_t ddr_size;
    bdmf_boolean cache_update_on_reg_ddr_lookup;
    bdmf_boolean ddr_counter_8bit_in_32bit_swap_control;
    bdmf_boolean ddr_write_w_ack_enable;
    bdmf_boolean ddr_replace_duplicated_cached_entry_enable;
    bdmf_boolean ddr_lookup_pending_fifo_mode_disable;
    bdmf_boolean eviction_disable;
} natc_ctrl_status;

bdmf_error_t ag_drv_natc_ctrl_status_set(const natc_ctrl_status *ctrl_status);
bdmf_error_t ag_drv_natc_ctrl_status_get(natc_ctrl_status *ctrl_status);
bdmf_error_t ag_drv_natc_table_control_set(uint8_t tbl_idx, uint8_t data);
bdmf_error_t ag_drv_natc_table_control_get(uint8_t tbl_idx, uint8_t *data);

#ifdef USE_BDMF_SHELL
enum
{
    cli_natc_ctrl_status,
    cli_natc_table_control,
};

int bcm_natc_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
bdmfmon_handle_t ag_drv_natc_cli_init(bdmfmon_handle_t driver_dir);
#endif


#endif

