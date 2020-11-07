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
/* ddr_64bit_in_128bit_swap_control:  - Swap 64-bit word within 128-bit word for DDR memory read/ */
/*                                   write accesses(i.e., [127:0] becomes {[63:0], [127:64]}).Thi */
/*                                   s bit should be set to 1 in Little Endian mode.              */
/* smem_32bit_in_64bit_swap_control:  - Swap 32-bit word within 64-bit word for statistics (count */
/*                                   er) memory accesses(i.e., [63:0] becomes {[31:0], [63:32]})  */
/* smem_8bit_in_32bit_swap_control:  - Reverse bytes within 32-bit word for statistics (counter)  */
/*                                  memory accesses(i.e., [31:0] becomes {[7:0], [15,8], [23,16], */
/*                                   [31,24]})                                                    */
/* ddr_swap_all_control:  - Swap all bytes on DDR interface.This bit should be set to 1 in Little */
/*                        Endian mode.                                                            */
/* unused3:  -                                                                                    */
/* reg_32bit_in_64bit_swap_control:  - Swap 32-bit word within 64-bit word for key_result registe */
/*                                  r accesses(i.e., [63:0] becomes {[31:0], [63:32]})            */
/* reg_8bit_in_32bit_swap_control:  - Reverse bytes within 32-bit word for key_result register ac */
/*                                 cesses(i.e., [31:0] becomes {[7:0], [15,8], [23,16], [31,24]}) */
/* unused2:  -                                                                                    */
/* pending_fifo_entry_check_enable:  - This bit is only valid when CACHE_UPDATE_ON_DDR_MISS bit i */
/*                                  s set to 1.This bit determines whether pending FIFO entry wil */
/*                                  l be checked todetermine whether cache update on DDR miss wil */
/*                                  l happen or not.1h: Enable; DDR miss entry fetched from DDR w */
/*                                  ill be cached if pending FIFOcontains entries which have the  */
/*                                  same hash value as DDR miss entry.0h: Disable; DDR miss entry */
/*                                   fetched from DDR will always be cached.                      */
/* cache_update_on_ddr_miss:  - This bit determines whether DDR lookup will cache the entry for D */
/*                           DR miss entry.1h: Enable; DDR miss entry fetched from DDR will be ca */
/*                           ched.0h: Disable; DDR miss entry fetched from DDR will not be cached */
/*                           .                                                                    */
/* ddr_disable_on_reg_lookup:  - 0h: Enable DDR lookup when cache misses using register interface */
/*                             lookup1h: Disable DDR lookup when cache misses using register inte */
/*                            rface lookup                                                        */
/* regfile_fifo_reset:  - Reset regfile_FIFO.                                                     */
/* nat_hash_mode:  - Hash algorithm used for internal caching0h: 32-bit rolling XOR hash is used  */
/*                as cache hash function.1h: CRC32 hash is used as cache hash function. CRC32 is  */
/*                reduced to 10-bit usingthe same method as in 32-bit rolling XOR hash.2h: CRC32  */
/*                hash is used as cache hash function. CRC32[9:0] is used as hash value3h: CRC32  */
/*                hash is used as cache hash function. CRC32[25:16] is used as hash value.        */
/* multi_hash_limit:  - Maximum number of multi-hash iterationsValue of 0 is 1 iteration, 1 is 2  */
/*                   iterations, 2 is 3 iterations, etc.                                          */
/* decr_count_wraparound_enable:  - Decrement Count Wraparound Enable0h: Do not decrement counter */
/*                               s for decrement command when counters reach 01h: Always decremen */
/*                               t counters for decrement command; will wrap around from 0 to all */
/*                                1's                                                             */
/* nat_arb_st:  - NAT Arbitration MechanismRound-robin arbitrationStrict priority arbitrationlist */
/*             ed from highest to lowest priority --  NAT0, NAT1, NAT2, NAT3, RunnerStrict priori */
/*             ty arbitration (priority reversed from above)listed from highest to lowest priorit */
/*             y --  Runner, NAT3, NAT2, NAT1, NAT0                                               */
/* natc_smem_increment_on_reg_lookup:  - Enables incrementing or decrementing hit counter by 1 an */
/*                                    d byte counter by PKT_LENon successful lookups using regist */
/*                                    er interfaceBY default, counters only increment on successf */
/*                                    ul lookups on Runner interface                              */
/* natc_smem_clear_by_update_disable:  - Disables clearing counters when an existing entry is rep */
/*                                    laced by ADD command                                        */
/* natc_smem_disable:  - Disables counters from incrementing when hit                             */
/* natc_enable:  - Enables all NATC state machines and input FIFO;Clearing this bit will halt all */
/*               state machines gracefully to idle states,all outstanding transactions in the FIF */
/*              O will remain in the FIFO and NATCwill stop accepting new commands;  All configur */
/*              ation registers should beconfigured before enabling this bit.                     */
/* natc_reset:  - Self Clearing Block Reset (including resetting all registers to default values) */
/* ddr_hash_swap:  - Reverse bytes within 18-bit DDR hash value                                   */
/* ddr_hash_mode:  - Hash algorithm used for DDR lookupHash value is DDR table size dependent.0h: */
/*                 32-bit rolling XOR hash is used as DDR hash function. It is reduced to N-bitDD */
/*                R table size is 8K,   N = 13.DDR table size is 16K,  N = 14.DDR table size is 3 */
/*                2K,  N = 15.DDR table size is 64K,  N = 16.DDR table size is 128K, N = 17.DDR t */
/*                able size is 256K, N = 18.1h: CRC32 hash is used as DDR hash function. CRC32 is */
/*                 reduced to N-bit usingthe same method as in 32-bit rolling XOR hash.DDR table  */
/*                size is 8K,   N = 13.DDR table size is 16K,  N = 14.DDR table size is 32K,  N = */
/*                 15.DDR table size is 64K,  N = 16.DDR table size is 128K, N = 17.DDR table siz */
/*                e is 256K, N = 18.2h: CRC32 hash is used as DDR hash function. CRC32[N:0] is us */
/*                ed as hash valueDDR table size is 8K,   N = 12.DDR table size is 16K,  N = 13.D */
/*                DR table size is 32K,  N = 14.DDR table size is 64K,  N = 15.DDR table size is  */
/*                128K, N = 16.DDR table size is 256K, N = 17.3h: CRC32 hash is used as DDR hash  */
/*                function. CRC32[31:N] is used as hash valueDDR table size is 8K,   N = 19.DDR t */
/*                able size is 16K,  N = 18.DDR table size is 32K,  N = 17.DDR table size is 64K, */
/*                  N = 16.DDR table size is 128K, N = 15.DDR table size is 256K, N = 14.         */
/* ddr_32bit_in_64bit_swap_control:  - Swap 32-bit word within 64-bit word for DDR memory read/wr */
/*                                  ite accesses(i.e., [63:0] becomes {[31:0], [63:32]}).This bit */
/*                                   should be set to 1 in Little Endian mode.                    */
/* ddr_8bit_in_32bit_swap_control:  - Reverse bytes within 32-bit word for DDR memory read/write  */
/*                                 accesses(i.e., [31:0] becomes {[7:0], [15,8], [23,16], [31,24] */
/*                                 }).This bit should be set to 1 in Little Endian mode.          */
/* cache_lookup_blocking_mode:  - (debug command) Do not set this bit to 1                        */
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
/*             count while the entry is in the cache.When the entry is evicted hit count for that */
/*              entry is lost.Replacement algorithm prioritizes pseudo-LRU over lowest-hit-countR */
/*             eplacement algorithm prioritizes lowest-hit-count over pseudo-LRUReplacement algor */
/*             ithm uses pseudo-LRUReplacement algorithm uses least-hit-countReplacement algorith */
/*             m prioritizes pseudo-LRU over pseudo-randomReplacement algorithm prioritizes lowes */
/*             t-hit-count over pseudo-randomReplacement algorithm uses pseudo-random algorithmRe */
/*             placement algorithm prioritizes highest-hit-count overmost-recently-useReplacement */
/*              algorithm prioritizes pseudo-LRU over lowest-byte-countReplacement algorithm prio */
/*             ritizes lowest-byte-count over pseudo-LRUReplacement algorithm uses least-byte-cou */
/*             ntReplacement algorithm prioritizes lowest-byte-count over pseudo-randomReplacemen */
/*             t algorithm prioritizes highest-byte-count overmost-recently-use                   */
/* unused1:  -                                                                                    */
/* unused0:  -                                                                                    */
/* cache_update_on_reg_ddr_lookup:  - This bit determines whether register interface lookup will  */
/*                                 cache the entry from DDR1h: Enable; entry fetched from DDR wil */
/*                                 l be cached using register interface lookup command0h: Disable */
/*                                 ; entry fetched from DDR will not be cached using register int */
/*                                 erface lookup command                                          */
/* ddr_counter_8bit_in_32bit_swap_control:  - Reverse bytes within 32-bit word for DDR counters o */
/*                                         n read/write accesses.(i.e., [31:0] becomes {[7:0], [1 */
/*                                         5,8], [23,16], [31,24]})                               */
/* unused5:  -                                                                                    */
/* ddr_replace_duplicated_cached_entry_enable:  - (debug command) Do not set this bit to 1Enable  */
/*                                             replacing existing cache counters with DDR fetched */
/*                                              entry                                             */
/* ddr_lookup_pending_fifo_mode_disable:  - (debug command) Do not set this bit to 1              */
/* eviction_disable:  - Disable counter eviction to DDR; this bit is effective when CACHE_DISABLE */
/*                    is 0Set this bit when counters are not used; NATC performance will improve  */
/*                   dueto reduced DDR accesses; CACHE_ALGO should not use HIT_COUNT and BYTE_COU */
/*                   NT                                                                           */
/**************************************************************************************************/
typedef struct
{
    bdmf_boolean ddr_enable;
    bdmf_boolean natc_add_command_speedup_mode;
    uint8_t unused4;
    bdmf_boolean ddr_64bit_in_128bit_swap_control;
    bdmf_boolean smem_32bit_in_64bit_swap_control;
    bdmf_boolean smem_8bit_in_32bit_swap_control;
    bdmf_boolean ddr_swap_all_control;
    bdmf_boolean unused3;
    bdmf_boolean reg_32bit_in_64bit_swap_control;
    bdmf_boolean reg_8bit_in_32bit_swap_control;
    uint8_t unused2;
    bdmf_boolean pending_fifo_entry_check_enable;
    bdmf_boolean cache_update_on_ddr_miss;
    bdmf_boolean ddr_disable_on_reg_lookup;
    bdmf_boolean regfile_fifo_reset;
    uint8_t nat_hash_mode;
    uint8_t multi_hash_limit;
    bdmf_boolean decr_count_wraparound_enable;
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
    uint8_t unused1;
    uint8_t unused0;
    bdmf_boolean cache_update_on_reg_ddr_lookup;
    bdmf_boolean ddr_counter_8bit_in_32bit_swap_control;
    bdmf_boolean unused5;
    bdmf_boolean ddr_replace_duplicated_cached_entry_enable;
    bdmf_boolean ddr_lookup_pending_fifo_mode_disable;
    bdmf_boolean eviction_disable;
} natc_ctrl_status;


/**************************************************************************************************/
/* var_context_len_en_tbl7:  - Controls the amount of context to fetch from DDR in unit of 8 byte */
/*                          s for DDR table 7lowest 4 bits of key[3:0] is used to indicate the co */
/*                          ntext length0=8 byte, 1=16 bytes, 2=24 bytes, .... 15=128 bytesNote t */
/*                          hat key length is reduced by 4 bit0h: Disable variable context length */
/*                          1h: Enable variable context length                                    */
/* var_context_len_en_tbl6:  - Controls the amount of context to fetch from DDR in unit of 8 byte */
/*                          s for DDR table 6lowest 4 bits of key[3:0] is used to indicate the co */
/*                          ntext length0=8 byte, 1=16 bytes, 2=24 bytes, .... 15=128 bytesNote t */
/*                          hat key length is reduced by 4 bit0h: Disable variable context length */
/*                          1h: Enable variable context length                                    */
/* var_context_len_en_tbl5:  - Controls the amount of context to fetch from DDR in unit of 8 byte */
/*                          s for DDR table 5lowest 4 bits of key[3:0] is used to indicate the co */
/*                          ntext length0=8 byte, 1=16 bytes, 2=24 bytes, .... 15=128 bytesNote t */
/*                          hat key length is reduced by 4 bit0h: Disable variable context length */
/*                          1h: Enable variable context length                                    */
/* var_context_len_en_tbl4:  - Controls the amount of context to fetch from DDR in unit of 8 byte */
/*                          s for DDR table 4lowest 4 bits of key[3:0] is used to indicate the co */
/*                          ntext length0=8 byte, 1=16 bytes, 2=24 bytes, .... 15=128 bytesNote t */
/*                          hat key length is reduced by 4 bit0h: Disable variable context length */
/*                          1h: Enable variable context length                                    */
/* var_context_len_en_tbl3:  - Controls the amount of context to fetch from DDR in unit of 8 byte */
/*                          s for DDR table 3lowest 4 bits of key[3:0] is used to indicate the co */
/*                          ntext length0=8 byte, 1=16 bytes, 2=24 bytes, .... 15=128 bytesNote t */
/*                          hat key length is reduced by 4 bit0h: Disable variable context length */
/*                          1h: Enable variable context length                                    */
/* var_context_len_en_tbl2:  - Controls the amount of context to fetch from DDR in unit of 8 byte */
/*                          s for DDR table 2lowest 4 bits of key[3:0] is used to indicate the co */
/*                          ntext length0=8 byte, 1=16 bytes, 2=24 bytes, .... 15=128 bytesNote t */
/*                          hat key length is reduced by 4 bit0h: Disable variable context length */
/*                          1h: Enable variable context length                                    */
/* var_context_len_en_tbl1:  - Controls the amount of context to fetch from DDR in unit of 8 byte */
/*                          s for DDR table 1lowest 4 bits of key[3:0] is used to indicate the co */
/*                          ntext length0=8 byte, 1=16 bytes, 2=24 bytes, .... 15=128 bytesNote t */
/*                          hat key length is reduced by 4 bit0h: Disable variable context length */
/*                          1h: Enable variable context length                                    */
/* var_context_len_en_tbl0:  - Controls the amount of context to fetch from DDR in unit of 8 byte */
/*                          s for DDR table 0lowest 4 bits of key[3:0] is used to indicate the co */
/*                          ntext length0=8 byte, 1=16 bytes, 2=24 bytes, .... 15=128 bytesNote t */
/*                          hat key length is reduced by 4 bit0h: Disable variable context length */
/*                          1h: Enable variable context length                                    */
/* key_len_tbl7:  - Length of the key for DDR table 70h: 16-byte key1h: 32-byte key               */
/* non_cacheable_tbl7:  - DDR table 7 non-cacheable control0h: DDR table is cached1h: DDR table i */
/*                     s not cached; counters are updated in DDR directly                         */
/* key_len_tbl6:  - Length of the key for DDR table 60h: 16-byte key1h: 32-byte key               */
/* non_cacheable_tbl6:  - DDR table 6 non-cacheable control0h: DDR table is cached1h: DDR table i */
/*                     s not cached; counters are updated in DDR directly                         */
/* key_len_tbl5:  - Length of the key for DDR table 50h: 16-byte key1h: 32-byte key               */
/* non_cacheable_tbl5:  - DDR table 5 non-cacheable control0h: DDR table is cached1h: DDR table i */
/*                     s not cached; counters are updated in DDR directly                         */
/* key_len_tbl4:  - Length of the key for DDR table 40h: 16-byte key1h: 32-byte key               */
/* non_cacheable_tbl4:  - DDR table 4 non-cacheable control0h: DDR table is cached1h: DDR table i */
/*                     s not cached; counters are updated in DDR directly                         */
/* key_len_tbl3:  - Length of the key for DDR table 30h: 16-byte key1h: 32-byte key               */
/* non_cacheable_tbl3:  - DDR table 3 non-cacheable control0h: DDR table is cached1h: DDR table i */
/*                     s not cached; counters are updated in DDR directly                         */
/* key_len_tbl2:  - Length of the key for DDR table 20h: 16-byte key1h: 32-byte key               */
/* non_cacheable_tbl2:  - DDR table 2 non-cacheable control0h: DDR table is cached1h: DDR table i */
/*                     s not cached; counters are updated in DDR directly                         */
/* key_len_tbl1:  - Length of the key for DDR table 10h: 16-byte key1h: 32-byte key               */
/* non_cacheable_tbl1:  - DDR table 1 non-cacheable control0h: DDR table is cached1h: DDR table i */
/*                     s not cached; counters are updated in DDR directly                         */
/* key_len_tbl0:  - Length of the key for DDR table 00h: 16-byte key1h: 32-byte key               */
/* non_cacheable_tbl0:  - DDR table 0 non-cacheable control0h: DDR table is cached1h: DDR table i */
/*                     s not cached; counters are updated in DDR directly                         */
/**************************************************************************************************/
typedef struct
{
    bdmf_boolean var_context_len_en_tbl7;
    bdmf_boolean var_context_len_en_tbl6;
    bdmf_boolean var_context_len_en_tbl5;
    bdmf_boolean var_context_len_en_tbl4;
    bdmf_boolean var_context_len_en_tbl3;
    bdmf_boolean var_context_len_en_tbl2;
    bdmf_boolean var_context_len_en_tbl1;
    bdmf_boolean var_context_len_en_tbl0;
    bdmf_boolean key_len_tbl7;
    bdmf_boolean non_cacheable_tbl7;
    bdmf_boolean key_len_tbl6;
    bdmf_boolean non_cacheable_tbl6;
    bdmf_boolean key_len_tbl5;
    bdmf_boolean non_cacheable_tbl5;
    bdmf_boolean key_len_tbl4;
    bdmf_boolean non_cacheable_tbl4;
    bdmf_boolean key_len_tbl3;
    bdmf_boolean non_cacheable_tbl3;
    bdmf_boolean key_len_tbl2;
    bdmf_boolean non_cacheable_tbl2;
    bdmf_boolean key_len_tbl1;
    bdmf_boolean non_cacheable_tbl1;
    bdmf_boolean key_len_tbl0;
    bdmf_boolean non_cacheable_tbl0;
} natc_table_control;

bdmf_error_t ag_drv_natc_ctrl_status_set(const natc_ctrl_status *ctrl_status);
bdmf_error_t ag_drv_natc_ctrl_status_get(natc_ctrl_status *ctrl_status);
bdmf_error_t ag_drv_natc_table_control_set(const natc_table_control *table_control);
bdmf_error_t ag_drv_natc_table_control_get(natc_table_control *table_control);

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

