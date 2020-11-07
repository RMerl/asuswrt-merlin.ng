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

#ifndef _XRDP_DRV_FPM_AG_H_
#define _XRDP_DRV_FPM_AG_H_

#include "access_macros.h"
#include "bdmf_interface.h"
#ifdef USE_BDMF_SHELL
#include "bdmf_shell.h"
#endif
#include "rdp_common.h"


/**************************************************************************************************/
/* fpm_buf_size:  - Selects the size of the buffer to be used in the pool. All buffers must be th */
/*               e same size. 
 0 - 512 byte buffers 
 1 - 256 byte buffers 
 all other values -  */
/*               reserved 
                                                                       */
/* pool_base_address:  - Buffer base address. 7:2 must be 0x00.
                                  */
/* pool_base_address_pool2:  - Buffer base address. 7:2 must be 0x00.
                            */
/**************************************************************************************************/
typedef struct
{
    uint8_t fpm_buf_size;
    uint32_t pool_base_address;
    uint32_t pool_base_address_pool2;
} fpm_pool_cfg;


/**************************************************************************************************/
/* ovrfl:  - Free Pool overflow count                                                             */
/* undrfl:  - Free Pool underflow count                                                           */
/* pool_full:  - POOL is full
This indicates that all tokens have been allocated and there no fre */
/*            e tokens available. This bit will be active as long as all usage array is fully all */
/*            ocated. 
                                                                           */
/* free_fifo_full:  - FREE_FIFO is full. 
                                                        */
/* free_fifo_empty:  - FREE_FIFO is empty 
                                                       */
/* alloc_fifo_full:  - ALLOC_FIFO is full 
                                                       */
/* alloc_fifo_empty:  - ALLOC_FIFO is empty. 
                                                    */
/* num_of_tokens_available:  - Count of tokens available for allocation. 
This provides a count o */
/*                          f number of free tokens that available for allocation in the usage ar */
/*                          ray. This value is updated instantaneously as tokens are allocated or */
/*                           freed from the array. 
                                              */
/* num_of_not_valid_token_frees:  - Count of de-allocate token requests with invalid tokens. For  */
/*                               more information on conditions under which this counter is incre */
/*                               mented, refer to POOL1_INTR_STS register (offset 0x14) bit[3] ex */
/*                               planation in this document. 
                                    */
/* num_of_not_valid_token_multi:  - Count of multi-cast token update requests with either valid b */
/*                               it not set, For more information on conditions under which this  */
/*                               counter is incremented, refer to POOL1_INTR_STS register (offset */
/*                                0x14) bit[5] explanation in this document. 
                    */
/* mem_corrupt_sts_related_alloc_token_valid:  - This bit provides status of the token in bits[30 */
/*                                            :0] of this register 
0 = New token is not captured */
/*                                             
1 = New token is captured 
                       */
/* mem_corrupt_sts_related_alloc_token:  - Token that causes memory corrupt interrupt active. If  */
/*                                      there are multiple tokens that causes this error, only th */
/*                                      e first one is captured. To capture successive tokens tha */
/*                                      t causes the error this register should be cleared by wri */
/*                                      ting any random value, in addition, memory corrupt status */
/*                                       bit (bit[8]) in interrupt status register 0x14 should be */
/*                                       cleared. Bitmap for these bits is shown below (reserved  */
/*                                      bits are zeros) 
Bit[30]    - Reserved 
Bit[29:12] - Toke */
/*                                      n 
Bit[11:0]  - Buffer size in bytes 
                    */
/* invalid_free_token_valid:  - This bit provides status of the token in bits[30:0] of this regis */
/*                           ter 
0 = New token is not captured 
1 = New token is captured 
      */
/* invalid_free_token:  - Token that causes intr[3] or intr[4] active. If there are multiple toke */
/*                     ns that causes this error, only the first one is captured. To capture succ */
/*                     essive tokens that causes the error this register should be cleared by wri */
/*                     ting any random value. Bitmap for these bits is shown below (reserved bits */
/*                      are either zeros or can reflect the length of the packet associated with  */
/*                     the freed token) 
Bit[30]    - Reserved 
Bit[29:12] - Token 
Bit[11:0]  -  */
/*                     Reserved 
                                                                 */
/* invalid_mcast_token_valid:  - This bit provides status of the token in bits[30:0] of this regi */
/*                            ster 
0 = New token is not captured 
1 = New token is captured 
    */
/* invalid_mcast_token:  - Token that causes intr[5] or intr[6] active. If there are multiple tok */
/*                      ens that causes this error, only the first one is captured. To capture su */
/*                      ccessive tokens that causes the error this register should be cleared by  */
/*                      writing any random value. Bitmap for these bits is shown below (reserved  */
/*                      bits are zeros) 
Bit[30]    - Reserved 
Bit[29:12] - Token 
Bit[11]    -  */
/*                      Mcast update type (refer to register 0x224[11]) 
Bit[10:7]  - Reserved 
B */
/*                      it[6:0]   - Mcast value 
                                                 */
/* tokens_available_low_wtmk:  - Lowest value the NUM_OF_TOKENS_AVAIL count has reached.          */
/**************************************************************************************************/
typedef struct
{
    uint16_t ovrfl;
    uint16_t undrfl;
    bdmf_boolean pool_full;
    bdmf_boolean free_fifo_full;
    bdmf_boolean free_fifo_empty;
    bdmf_boolean alloc_fifo_full;
    bdmf_boolean alloc_fifo_empty;
    uint32_t num_of_tokens_available;
    uint32_t num_of_not_valid_token_frees;
    uint32_t num_of_not_valid_token_multi;
    bdmf_boolean mem_corrupt_sts_related_alloc_token_valid;
    uint32_t mem_corrupt_sts_related_alloc_token;
    bdmf_boolean invalid_free_token_valid;
    uint32_t invalid_free_token;
    bdmf_boolean invalid_mcast_token_valid;
    uint32_t invalid_mcast_token;
    uint32_t tokens_available_low_wtmk;
} fpm_pool_stat;


/**************************************************************************************************/
/* long_aging_timer:  - Aging timer used in token recovery
                                       */
/* short_aging_timer:  - Aging timer used in token recovery
                                      */
/* recycle_timer:  - Timer used in token recovery logic. Upon expiration of timer, one token from */
/*                 the allocate cache will be freed. Over time, all cached tokens will be recycle */
/*                d back to the freepool. This will prevent the cached tokens frm being aged out  */
/*                by the token recovery logic. This timer should be set to a value so that all to */
/*                kens can be recycled before the aging timer expires.
                           */
/**************************************************************************************************/
typedef struct
{
    uint32_t long_aging_timer;
    uint32_t short_aging_timer;
    uint16_t recycle_timer;
} fpm_timer;


/**************************************************************************************************/
/* expired_token_recov_msk:  - Expired token recovered interrupt mask. 
                          */
/* expired_token_det_msk:  - Expired token detect interrupt mask. 
                               */
/* illegal_alloc_request_msk:  - Illegal token request interrupt mask. 
                          */
/* illegal_address_access_msk:  - Illegal/un-implemented register/memory space access  interrupt  */
/*                             mask. 
                                                            */
/* xon_msk:  - XON_STATE interrupt mask. 
                                                        */
/* xoff_msk:  - XOFF_STATE interrupt mask. 
                                                      */
/* memory_corrupt_msk:  - Index Memory corrupt interrupt mask. 
                                  */
/* pool_dis_free_multi_msk:  - Free or Mcast update on disabled pool interrupt mask . 
           */
/* multi_token_index_out_of_range_msk:  - Token multi-cast value update request with index out-of */
/*                                     -range. 
                                                  */
/* multi_token_no_valid_msk:  - Token multi-cast value update request with invalid token. 
       */
/* free_token_index_out_of_range_msk:  - De-allocation token request with index out-of-range. 
   */
/* free_token_no_valid_msk:  - De-allocation token request with invalid token. 
                  */
/* pool_full_msk:  - Usage Index Pool is fully allocated interrupt mask. 
                        */
/* free_fifo_full_msk:  - De-Allocation FIFO Full Interrupt mask. 
                               */
/* alloc_fifo_full_msk:  - Allocation FIFO Full Interrupt mask. 
                                 */
/**************************************************************************************************/
typedef struct
{
    bdmf_boolean expired_token_recov_msk;
    bdmf_boolean expired_token_det_msk;
    bdmf_boolean illegal_alloc_request_msk;
    bdmf_boolean illegal_address_access_msk;
    bdmf_boolean xon_msk;
    bdmf_boolean xoff_msk;
    bdmf_boolean memory_corrupt_msk;
    bdmf_boolean pool_dis_free_multi_msk;
    bdmf_boolean multi_token_index_out_of_range_msk;
    bdmf_boolean multi_token_no_valid_msk;
    bdmf_boolean free_token_index_out_of_range_msk;
    bdmf_boolean free_token_no_valid_msk;
    bdmf_boolean pool_full_msk;
    bdmf_boolean free_fifo_full_msk;
    bdmf_boolean alloc_fifo_full_msk;
} fpm_pool2_intr_msk;


/**************************************************************************************************/
/* expired_token_recov_sts:  - Expired token recovered interrupt. This is set when an expired tok */
/*                          en has been recoveredand returned to pool as an available token. 
    */
/* expired_token_det_sts:  - Expired token detect interrupt. This is set when the token recovery  */
/*                        logic detects a token that has been held for the entire duration of the */
/*                         aging timer. 
                                                         */
/* illegal_alloc_request_sts:  - Illegal token request interrupt. This will be active when the po */
/*                            ol is disabled, there is a request for a new token and the alloc fi */
/*                            fo for the selected token size is empty. Along with interrupt being */
/*                             sent an error reply packet will be sent out with o_ubus_error_out  */
/*                            asserted. 
                                                         */
/* illegal_address_access_sts:  - Illegal/un-implemented register/memory space access interrupt.  */
/*                             This will be active when there is an attempt to read from an unimp */
/*                             lemented register or memory space. Along with interrupt being sent */
/*                              an error reply packet will be sent out with o_ubus_error_out asse */
/*                             rted. 
                                                            */
/* xon_state_sts:  - Number of available tokens is greater than or equal to XON_THRESHOLD value i */
/*                n XON/XOFF Threshold configuration register. This is a functional status bit, n */
/*                ot an error status bit. Using this information FPM generates "backpressure" out */
/*                put signal that is used by other UBUS client logics to throttle its operation.  */
/*                For example, UNIMAC logic can use "backpressure" signal to transfer "PAUSE" Eth */
/*                ernet flow control packets to throttle incoming frames on Ethernet interface. 
 */
/* xoff_state_sts:  - Number of available tokens is less than or equal to XOFF_THRESHOLD value in */
/*                  XON/XOFF Threshold configuration register. This is a functional status bit, n */
/*                 ot an error status bit. Using this information FPM generates "backpressure" ou */
/*                 tput signal that is used by other UBUS client logics to throttle its operation */
/*                 . For example, UNIMAC logic can use "backpressure" signal to transfer "PAUSE"  */
/*                 Ethernet flow control packets to throttle incoming frames on Ethernet interfac */
/*                 e. 
                                                                           */
/* memory_corrupt_sts:  - Index Memory is corrupted. 
During updates of the usage array, token ma */
/*                     nager checks if the use count and search tree value in the array has a leg */
/*                     al value. If the use count or search tree value is not correct before upda */
/*                     ting, logic generates an error and interrupt. As long as the interrupt is  */
/*                     active no more valid tokens will be allocated because this is a catastroph */
/*                     ic error. Following are the two error conditions that are checked - 
1. Du */
/*                     ring search for a free token, a particular token use count value indicates */
/*                      it is allocated (use count is greater than 0), but corresponding upper le */
/*                     vel search tree value indicates the token is still available (with bit val */
/*                     ue of 1'b0, instead of 1'b1). This is an error. 
2. During search for a fr */
/*                     ee token, a particular token use count value indicates that it is free (us */
/*                     e count is 0), but corresponding upper level search tree value indicates t */
/*                     he token is not available (with bit value of 1'b1, instead of 1'b0). This  */
/*                     is an error. 
                                                             */
/* pool_dis_free_multi_sts:  - Free or Mcast update on disabled pool interrupt. 
This bit goes ac */
/*                          tive when a free or multi-cast request is received and FPM is not ena */
/*                          bled, i.e., pool enable bit in FPM control register is not set to 1'b */
/*                          1. 
                                                                  */
/* multi_token_index_out_of_range_sts:  - Token multi-cast value update request with index out-of */
/*                                     -range Interrupt. 
This is set when the token index is not */
/*                                      aligned to the pool size. This is determined by examining */
/*                                      the pool select field (bits[29:28]) and the 3 lsbs of the */
/*                                      token index (bits[14:12]). There is no associated count f */
/*                                     or this error. Note: this error is not checked if auto_poo */
/*                                     l_en is set. The auto_pool_en bit is always set when using */
/*                                      the new token format without a pool select field. 
       */
/* multi_token_no_valid_sts:  - Token multi-cast value update request with invalid token Interrup */
/*                           t. 
Invalid multi-cast token is determined when one or more the foll */
/*                           owing conditions are met - 
1. Incoming multi-cast request token has */
/*                            valid bit (bit[31]) set to 1'b0 
2. Incoming multi-cast request tok */
/*                           en index is not aligned to the pool size indicated by the pool selec */
/*                           t field (bits[29:28]) 
3. Incoming multi-cast request token has use  */
/*                           count field (bit[6:0]) set to zero 
4. Incoming multi-cast request t */
/*                           oken entry in the usage array indicates it is not an allocated token */
/*                           , i.e., associated use count value for this count in the usage array */
/*                            is zero 
5. After updating the use count value, the new use count v */
/*                           alue exceeds 0x7E 
Note: item 2 is not checked if auto_pool_en is se */
/*                           t. The auto_pool_en bit is always set when using the new token forma */
/*                           t without a pool select field. 
                                     */
/* free_token_index_out_of_range_sts:  - De-allocation token request with index out-of-range Inte */
/*                                    rrupt. 
Free token index out of range is determined when on */
/*                                    e or more of the following conditions are met - 
1. Incomin */
/*                                    g free request token index is not aligned to the pool size  */
/*                                    indicated by the pool select field (bits[29:28]) 
2. The bu */
/*                                    ffer size indicated by the size field (bits[11:0]) is great */
/*                                    er than the size of the allocated token. 
There is no assoc */
/*                                    iated count for this error. Note: item 1 is not checked if  */
/*                                    auto_pool_en is set. The auto_pool_en bit is always set whe */
/*                                    n using the new token format without a pool select field. 
 */
/* free_token_no_valid_sts:  - De-allocation token request with invalid token Interrupt. 
Invalid */
/*                           free token is determined when one or more the following conditions a */
/*                          re met - 
1. Incoming free request token has valid bit (bit[31]) set  */
/*                          to 1'b0 
2. Incoming free request token index is not aligned to the p */
/*                          ool size indicated by the pool select field (bits[29:28]) 
3. Incomin */
/*                          g free request token entry in the usage array indicates it is not an  */
/*                          allocated token, i.e., associated use count value for this count in t */
/*                          he usage array is zero 
Note: item 2 is not checked if auto_pool_en i */
/*                          s set. The auto_pool_en bit is always set when using the new token fo */
/*                          rmat without a pool select field. 
                                   */
/* pool_full_sts:  - Usage Index Pool is fully allocated interrupt. This is a functional status b */
/*                it, not an error status bit. This indicates that token pool is fully allocated  */
/*                and there are no free tokens available. This bit will be active (high) as long  */
/*                as there no free tokens available to allocate. This bit is intended to be used  */
/*                for debug purpose only. 
                                                       */
/* free_fifo_full_sts:  - De-Allocation FIFO Full Interrupt. This is a functional status bit, not */
/*                      an error status bit. This indicates that de-allocation FIFO is full with  */
/*                     tokens needs to be freed and will be active (high) as long as FIFO is full */
/*                     . This status is intended to be used for debug purpose only. 
             */
/* alloc_fifo_full_sts:  - Allocation FIFO Full Interrupt. This is a functional status bit, not a */
/*                      n error status bit. This indicates that allocation FIFO is full with new  */
/*                      tokens to be allocated and will be active (high) as long as FIFO is full. */
/*                       This status is intended to be used for debug purpose only. 
             */
/**************************************************************************************************/
typedef struct
{
    bdmf_boolean expired_token_recov_sts;
    bdmf_boolean expired_token_det_sts;
    bdmf_boolean illegal_alloc_request_sts;
    bdmf_boolean illegal_address_access_sts;
    bdmf_boolean xon_state_sts;
    bdmf_boolean xoff_state_sts;
    bdmf_boolean memory_corrupt_sts;
    bdmf_boolean pool_dis_free_multi_sts;
    bdmf_boolean multi_token_index_out_of_range_sts;
    bdmf_boolean multi_token_no_valid_sts;
    bdmf_boolean free_token_index_out_of_range_sts;
    bdmf_boolean free_token_no_valid_sts;
    bdmf_boolean pool_full_sts;
    bdmf_boolean free_fifo_full_sts;
    bdmf_boolean alloc_fifo_full_sts;
} fpm_pool2_intr_sts;


/**************************************************************************************************/
/* memory_corrupt_stall_msk:  - Stall FPM on Index Memory corrupt interrupt status. 
             */
/* multi_token_index_out_of_range_stall_msk:  - Stall FPM on Token multi-cast value update reques */
/*                                           t with index out-of-range interrupt status. 
        */
/* multi_token_no_valid_stall_msk:  - Stall FPM on Token multi-cast value update request with inv */
/*                                 alid token interrupt status. 
                                 */
/* free_token_index_out_of_range_stall_msk:  - Stall FPM on De-allocation token request with inde */
/*                                          x out-of-range interrupt status. 
                    */
/* free_token_no_valid_stall_msk:  - Stall FPM on De-allocation token request with invalid token  */
/*                                interrupt status. 
                                             */
/**************************************************************************************************/
typedef struct
{
    bdmf_boolean memory_corrupt_stall_msk;
    bdmf_boolean multi_token_index_out_of_range_stall_msk;
    bdmf_boolean multi_token_no_valid_stall_msk;
    bdmf_boolean free_token_index_out_of_range_stall_msk;
    bdmf_boolean free_token_no_valid_stall_msk;
} fpm_pool2_stall_msk;


/**************************************************************************************************/
/* clr_recovered_token_count:  - This is a self-clearing bit. Write a 1 to the bit to reset  the  */
/*                            RECOVERED_TOKEN_COUNT to 0.
                                        */
/* clr_expired_token_count:  - This is a self-clearing bit. Write a 1 to the bit to reset  the EX */
/*                          PIRED_TOKEN_COUNT to 0.
                                              */
/* force_token_reclaim:  - Non-automated token recovery.
This bit can be used when automatic toke */
/*                      n return is not enabled. When software gets an interrupt indicating that  */
/*                      the token recovery process has detected expired tokens, it can set this b */
/*                      it to force the expired tokens to be reclaimed.
1 = Enabled
 0 = Disabled */
/*                      
                                                                         */
/* token_reclaim_ena:  - Enable automatic return of marked tokens to the freepool
1 = Enabled
 0  */
/*                    = Disabled
                                                                 */
/* token_remark_ena:  - Enable remarking of tokens for multiple passes through the token recovery */
/*                    process. The mark bit is set on all tokens on the first pass through the lo */
/*                   op. When this bit is set, the mark bits will be set again on all subsequent  */
/*                   passes through the loop. It is anticipated that this bit will always be set  */
/*                   when token recovery is enabled. It is provided as a potential debug tool.
1  */
/*                   = Enabled
 0 = Disabled
                                                     */
/* single_pass_ena:  - If token recovery is enabled, the single-pass control will indicate whethe */
/*                  r the hardware should perform just one iteration of the token recovery proces */
/*                  s or will continuously loop through the token recovery process.
1 = Single pa */
/*                  ss
 0 = Auto repeat
                                                          */
/* token_recover_ena:  - Token recovery enable
1 = Enabled
 0 = Disabled
                         */
/**************************************************************************************************/
typedef struct
{
    bdmf_boolean clr_recovered_token_count;
    bdmf_boolean clr_expired_token_count;
    bdmf_boolean force_token_reclaim;
    bdmf_boolean token_reclaim_ena;
    bdmf_boolean token_remark_ena;
    bdmf_boolean single_pass_ena;
    bdmf_boolean token_recover_ena;
} fpm_token_recover_ctl;


/**************************************************************************************************/
/* token_valid:  - Valid Token Indicator
0: No buffers available
1: A valid token index is provid */
/*              ed. If a token multi-cast value is updated without this bit set, that causes an e */
/*              rror and the token will be ignored, error counter in register offset 0xBC will be */
/*               incremented.                                                                     */
/* ddr:  - DDR Identifier
0: DDR0
1: DDR1
                                                        */
/* token_index:  - Buffer Index Pointer
                                                          */
/* update_type:  - 1'b0 - Count value is replaced with new value in bits[6:0]
1'b1 - Count value  */
/*              is incremented by value in bits[6:0]
                                             */
/* token_multi:  - New Multi-cast Value                                                           */
/**************************************************************************************************/
typedef struct
{
    bdmf_boolean token_valid;
    bdmf_boolean ddr;
    uint32_t token_index;
    bdmf_boolean update_type;
    uint8_t token_multi;
} fpm_pool_multi;


/**************************************************************************************************/
/* fifo_empty: FIFO_EMPTY - FIFO is empty                                                         */
/* fifo_full: FIFO_FULL - FIFO is full                                                            */
/* fifo_used_words: FIFO_USED_WORDS - Used words                                                  */
/* fifo_rd_cntr: FIFO_RD_CNTR - Write counter                                                     */
/* fifo_wr_cntr: FIFO_WR_CNTR - Write counter                                                     */
/**************************************************************************************************/
typedef struct
{
    bdmf_boolean fifo_empty;
    bdmf_boolean fifo_full;
    uint8_t fifo_used_words;
    uint8_t fifo_rd_cntr;
    uint8_t fifo_wr_cntr;
} fpm_fpm_bb_dbg_rxfifo_sts;


/**************************************************************************************************/
/* fifo_empty: FIFO_EMPTY - FIFO is empty                                                         */
/* fifo_full: FIFO_FULL - FIFO is full                                                            */
/* fifo_used_words: FIFO_USED_WORDS - Used words                                                  */
/* fifo_rd_cntr: FIFO_RD_CNTR - Write counter                                                     */
/* fifo_wr_cntr: FIFO_WR_CNTR - Write counter                                                     */
/**************************************************************************************************/
typedef struct
{
    bdmf_boolean fifo_empty;
    bdmf_boolean fifo_full;
    uint8_t fifo_used_words;
    uint8_t fifo_rd_cntr;
    uint8_t fifo_wr_cntr;
} fpm_fpm_bb_dbg_txfifo_sts;

bdmf_error_t ag_drv_fpm_init_mem_set(bdmf_boolean init_mem);
bdmf_error_t ag_drv_fpm_init_mem_get(bdmf_boolean *init_mem);
bdmf_error_t ag_drv_fpm_pool1_en_set(bdmf_boolean pool1_enable);
bdmf_error_t ag_drv_fpm_pool1_en_get(bdmf_boolean *pool1_enable);
bdmf_error_t ag_drv_fpm_bb_reset_set(bdmf_boolean fpm_bb_soft_reset);
bdmf_error_t ag_drv_fpm_bb_reset_get(bdmf_boolean *fpm_bb_soft_reset);
bdmf_error_t ag_drv_fpm_ddr0_weight_set(uint8_t ddr0_alloc_weight, uint8_t ddr0_free_weight);
bdmf_error_t ag_drv_fpm_ddr0_weight_get(uint8_t *ddr0_alloc_weight, uint8_t *ddr0_free_weight);
bdmf_error_t ag_drv_fpm_ddr1_weight_set(uint8_t ddr1_alloc_weight, uint8_t ddr1_free_weight);
bdmf_error_t ag_drv_fpm_ddr1_weight_get(uint8_t *ddr1_alloc_weight, uint8_t *ddr1_free_weight);
bdmf_error_t ag_drv_fpm_pool_cfg_set(const fpm_pool_cfg *pool_cfg);
bdmf_error_t ag_drv_fpm_pool_cfg_get(fpm_pool_cfg *pool_cfg);
bdmf_error_t ag_drv_fpm_pool_stat_get(fpm_pool_stat *pool_stat);
bdmf_error_t ag_drv_fpm_pool2_stat_get(fpm_pool_stat *pool_stat);
bdmf_error_t ag_drv_fpm_back_door_mem_set(uint32_t mem_data1, uint32_t mem_data2);
bdmf_error_t ag_drv_fpm_back_door_mem_get(uint32_t *mem_data1, uint32_t *mem_data2);
bdmf_error_t ag_drv_fpm_pool1_count_get(uint32_t *expired_count, uint32_t *recovered_count);
bdmf_error_t ag_drv_fpm_pool2_count_get(uint32_t *expired_count, uint32_t *recovered_count);
bdmf_error_t ag_drv_fpm_timer_set(const fpm_timer *timer);
bdmf_error_t ag_drv_fpm_timer_get(fpm_timer *timer);
bdmf_error_t ag_drv_fpm_fpm_cfg1_set(bdmf_boolean pool1_search_mode);
bdmf_error_t ag_drv_fpm_fpm_cfg1_get(bdmf_boolean *pool1_search_mode);
bdmf_error_t ag_drv_fpm_fpm_bb_cfg_set(uint8_t bb_ddr_sel);
bdmf_error_t ag_drv_fpm_fpm_bb_cfg_get(uint8_t *bb_ddr_sel);
bdmf_error_t ag_drv_fpm_pool1_intr_msk_set(const fpm_pool2_intr_msk *pool2_intr_msk);
bdmf_error_t ag_drv_fpm_pool1_intr_msk_get(fpm_pool2_intr_msk *pool2_intr_msk);
bdmf_error_t ag_drv_fpm_pool1_intr_sts_set(const fpm_pool2_intr_sts *pool2_intr_sts);
bdmf_error_t ag_drv_fpm_pool1_intr_sts_get(fpm_pool2_intr_sts *pool2_intr_sts);
bdmf_error_t ag_drv_fpm_pool1_stall_msk_set(const fpm_pool2_stall_msk *pool2_stall_msk);
bdmf_error_t ag_drv_fpm_pool1_stall_msk_get(fpm_pool2_stall_msk *pool2_stall_msk);
bdmf_error_t ag_drv_fpm_pool2_intr_msk_set(const fpm_pool2_intr_msk *pool2_intr_msk);
bdmf_error_t ag_drv_fpm_pool2_intr_msk_get(fpm_pool2_intr_msk *pool2_intr_msk);
bdmf_error_t ag_drv_fpm_pool2_intr_sts_set(const fpm_pool2_intr_sts *pool2_intr_sts);
bdmf_error_t ag_drv_fpm_pool2_intr_sts_get(fpm_pool2_intr_sts *pool2_intr_sts);
bdmf_error_t ag_drv_fpm_pool2_stall_msk_set(const fpm_pool2_stall_msk *pool2_stall_msk);
bdmf_error_t ag_drv_fpm_pool2_stall_msk_get(fpm_pool2_stall_msk *pool2_stall_msk);
bdmf_error_t ag_drv_fpm_pool1_xon_xoff_cfg_set(uint16_t xon_threshold, uint16_t xoff_threshold);
bdmf_error_t ag_drv_fpm_pool1_xon_xoff_cfg_get(uint16_t *xon_threshold, uint16_t *xoff_threshold);
bdmf_error_t ag_drv_fpm_fpm_not_empty_cfg_set(uint8_t not_empty_threshold);
bdmf_error_t ag_drv_fpm_fpm_not_empty_cfg_get(uint8_t *not_empty_threshold);
bdmf_error_t ag_drv_fpm_mem_ctl_set(bdmf_boolean mem_wr, bdmf_boolean mem_rd, uint8_t mem_sel, uint16_t mem_addr);
bdmf_error_t ag_drv_fpm_mem_ctl_get(bdmf_boolean *mem_wr, bdmf_boolean *mem_rd, uint8_t *mem_sel, uint16_t *mem_addr);
bdmf_error_t ag_drv_fpm_token_recover_ctl_set(const fpm_token_recover_ctl *token_recover_ctl);
bdmf_error_t ag_drv_fpm_token_recover_ctl_get(fpm_token_recover_ctl *token_recover_ctl);
bdmf_error_t ag_drv_fpm_token_recover_start_end_pool1_set(uint16_t start_index, uint16_t end_index);
bdmf_error_t ag_drv_fpm_token_recover_start_end_pool1_get(uint16_t *start_index, uint16_t *end_index);
bdmf_error_t ag_drv_fpm_token_recover_start_end_pool2_set(uint16_t start_index, uint16_t end_index);
bdmf_error_t ag_drv_fpm_token_recover_start_end_pool2_get(uint16_t *start_index, uint16_t *end_index);
bdmf_error_t ag_drv_fpm_pool1_alloc_dealloc_set(bdmf_boolean token_valid, bdmf_boolean ddr, uint32_t token_index, uint16_t token_size);
bdmf_error_t ag_drv_fpm_pool1_alloc_dealloc_get(bdmf_boolean *token_valid, bdmf_boolean *ddr, uint32_t *token_index, uint16_t *token_size);
bdmf_error_t ag_drv_fpm_pool2_alloc_dealloc_set(bdmf_boolean token_valid, bdmf_boolean ddr, uint32_t token_index, uint16_t token_size);
bdmf_error_t ag_drv_fpm_pool2_alloc_dealloc_get(bdmf_boolean *token_valid, bdmf_boolean *ddr, uint32_t *token_index, uint16_t *token_size);
bdmf_error_t ag_drv_fpm_pool3_alloc_dealloc_set(bdmf_boolean token_valid, bdmf_boolean ddr, uint32_t token_index, uint16_t token_size);
bdmf_error_t ag_drv_fpm_pool3_alloc_dealloc_get(bdmf_boolean *token_valid, bdmf_boolean *ddr, uint32_t *token_index, uint16_t *token_size);
bdmf_error_t ag_drv_fpm_pool4_alloc_dealloc_set(bdmf_boolean token_valid, bdmf_boolean ddr, uint32_t token_index, uint16_t token_size);
bdmf_error_t ag_drv_fpm_pool4_alloc_dealloc_get(bdmf_boolean *token_valid, bdmf_boolean *ddr, uint32_t *token_index, uint16_t *token_size);
bdmf_error_t ag_drv_fpm_pool_multi_set(const fpm_pool_multi *pool_multi);
bdmf_error_t ag_drv_fpm_pool_multi_get(fpm_pool_multi *pool_multi);
bdmf_error_t ag_drv_fpm_fpm_bb_force_set(bdmf_boolean force);
bdmf_error_t ag_drv_fpm_fpm_bb_force_get(bdmf_boolean *force);
bdmf_error_t ag_drv_fpm_fpm_bb_forced_ctrl_set(uint16_t ctrl);
bdmf_error_t ag_drv_fpm_fpm_bb_forced_ctrl_get(uint16_t *ctrl);
bdmf_error_t ag_drv_fpm_fpm_bb_forced_addr_set(uint16_t ta_addr, uint8_t dest_addr);
bdmf_error_t ag_drv_fpm_fpm_bb_forced_addr_get(uint16_t *ta_addr, uint8_t *dest_addr);
bdmf_error_t ag_drv_fpm_fpm_bb_forced_data_set(uint32_t data);
bdmf_error_t ag_drv_fpm_fpm_bb_forced_data_get(uint32_t *data);
bdmf_error_t ag_drv_fpm_fpm_bb_decode_cfg_set(uint8_t dest_id, bdmf_boolean override_en, uint16_t route_addr);
bdmf_error_t ag_drv_fpm_fpm_bb_decode_cfg_get(uint8_t *dest_id, bdmf_boolean *override_en, uint16_t *route_addr);
bdmf_error_t ag_drv_fpm_fpm_bb_dbg_cfg_set(uint8_t rxfifo_sw_addr, uint8_t txfifo_sw_addr, bdmf_boolean rxfifo_sw_rst, bdmf_boolean txfifo_sw_rst);
bdmf_error_t ag_drv_fpm_fpm_bb_dbg_cfg_get(uint8_t *rxfifo_sw_addr, uint8_t *txfifo_sw_addr, bdmf_boolean *rxfifo_sw_rst, bdmf_boolean *txfifo_sw_rst);
bdmf_error_t ag_drv_fpm_fpm_bb_dbg_rxfifo_sts_get(fpm_fpm_bb_dbg_rxfifo_sts *fpm_bb_dbg_rxfifo_sts);
bdmf_error_t ag_drv_fpm_fpm_bb_dbg_txfifo_sts_get(fpm_fpm_bb_dbg_txfifo_sts *fpm_bb_dbg_txfifo_sts);
bdmf_error_t ag_drv_fpm_fpm_bb_dbg_rxfifo_data1_get(uint32_t *data);
bdmf_error_t ag_drv_fpm_fpm_bb_dbg_rxfifo_data2_get(uint32_t *data);
bdmf_error_t ag_drv_fpm_fpm_bb_dbg_txfifo_data1_get(uint32_t *data);
bdmf_error_t ag_drv_fpm_fpm_bb_dbg_txfifo_data2_get(uint32_t *data);
bdmf_error_t ag_drv_fpm_fpm_bb_dbg_txfifo_data3_get(uint32_t *data);

#ifdef USE_BDMF_SHELL
enum
{
    cli_fpm_init_mem,
    cli_fpm_pool1_en,
    cli_fpm_bb_reset,
    cli_fpm_ddr0_weight,
    cli_fpm_ddr1_weight,
    cli_fpm_pool_cfg,
    cli_fpm_pool_stat,
    cli_fpm_pool2_stat,
    cli_fpm_back_door_mem,
    cli_fpm_pool1_count,
    cli_fpm_pool2_count,
    cli_fpm_timer,
    cli_fpm_fpm_cfg1,
    cli_fpm_fpm_bb_cfg,
    cli_fpm_pool1_intr_msk,
    cli_fpm_pool1_intr_sts,
    cli_fpm_pool1_stall_msk,
    cli_fpm_pool2_intr_msk,
    cli_fpm_pool2_intr_sts,
    cli_fpm_pool2_stall_msk,
    cli_fpm_pool1_xon_xoff_cfg,
    cli_fpm_fpm_not_empty_cfg,
    cli_fpm_mem_ctl,
    cli_fpm_token_recover_ctl,
    cli_fpm_token_recover_start_end_pool1,
    cli_fpm_token_recover_start_end_pool2,
    cli_fpm_pool1_alloc_dealloc,
    cli_fpm_pool2_alloc_dealloc,
    cli_fpm_pool3_alloc_dealloc,
    cli_fpm_pool4_alloc_dealloc,
    cli_fpm_pool_multi,
    cli_fpm_fpm_bb_force,
    cli_fpm_fpm_bb_forced_ctrl,
    cli_fpm_fpm_bb_forced_addr,
    cli_fpm_fpm_bb_forced_data,
    cli_fpm_fpm_bb_decode_cfg,
    cli_fpm_fpm_bb_dbg_cfg,
    cli_fpm_fpm_bb_dbg_rxfifo_sts,
    cli_fpm_fpm_bb_dbg_txfifo_sts,
    cli_fpm_fpm_bb_dbg_rxfifo_data1,
    cli_fpm_fpm_bb_dbg_rxfifo_data2,
    cli_fpm_fpm_bb_dbg_txfifo_data1,
    cli_fpm_fpm_bb_dbg_txfifo_data2,
    cli_fpm_fpm_bb_dbg_txfifo_data3,
};

int bcm_fpm_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
bdmfmon_handle_t ag_drv_fpm_cli_init(bdmfmon_handle_t driver_dir);
#endif


#endif

