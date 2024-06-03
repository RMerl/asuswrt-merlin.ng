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

#include <ru.h>
#include <bdmf_interface.h>
#include <rdp_common.h>

#ifdef USE_BDMF_SHELL
#include <bdmf_shell.h>
#endif

typedef struct
{
    uint8_t fp_buf_size;
    uint32_t pool_base_address;
    uint32_t pool_base_address_pool2;
} fpm_pool_cfg;

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
    uint32_t invalid_free_token;
    uint32_t invalid_mcast_token;
    uint32_t tokens_available_low_wtmk;
} fpm_pool_stat;

typedef struct
{
    uint32_t long_aging_timer;
    uint32_t short_aging_timer;
    uint16_t recycle_timer;
} fpm_timer;

typedef struct
{
    bdmf_boolean alloc_fifo_full_msk;
    bdmf_boolean free_fifo_full_msk;
    bdmf_boolean pool_full_msk;
    bdmf_boolean free_token_no_valid_msk;
    bdmf_boolean free_token_index_out_of_range_msk;
    bdmf_boolean multi_token_no_valid_msk;
    bdmf_boolean multi_token_index_out_of_range_msk;
    bdmf_boolean pool_dis_free_multi_msk;
    bdmf_boolean memory_corrupt_msk;
    bdmf_boolean xoff_msk;
    bdmf_boolean xon_msk;
    bdmf_boolean illegal_address_access_msk;
    bdmf_boolean illegal_alloc_request_msk;
    bdmf_boolean expired_token_det_msk;
    bdmf_boolean expired_token_recov_msk;
} fpm_pool1_intr_msk;

typedef struct
{
    bdmf_boolean alloc_fifo_full_sts;
    bdmf_boolean free_fifo_full_sts;
    bdmf_boolean pool_full_sts;
    bdmf_boolean free_token_no_valid_sts;
    bdmf_boolean free_token_index_out_of_range_sts;
    bdmf_boolean multi_token_no_valid_sts;
    bdmf_boolean multi_token_index_out_of_range_sts;
    bdmf_boolean pool_dis_free_multi_sts;
    bdmf_boolean memory_corrupt_sts;
    bdmf_boolean xoff_state_sts;
    bdmf_boolean xon_state_sts;
    bdmf_boolean illegal_address_access_sts;
    bdmf_boolean illegal_alloc_request_sts;
    bdmf_boolean expired_token_det_sts;
    bdmf_boolean expired_token_recov_sts;
} fpm_pool1_intr_sts;

typedef struct
{
    bdmf_boolean free_token_no_valid_stall_msk;
    bdmf_boolean free_token_index_out_of_range_stall_msk;
    bdmf_boolean multi_token_no_valid_stall_msk;
    bdmf_boolean multi_token_index_out_of_range_stall_msk;
    bdmf_boolean memory_corrupt_stall_msk;
} fpm_pool1_stall_msk;

typedef struct
{
    bdmf_boolean token_recover_ena;
    bdmf_boolean single_pass_ena;
    bdmf_boolean token_remark_ena;
    bdmf_boolean token_reclaim_ena;
    bdmf_boolean force_token_reclaim;
    bdmf_boolean clr_expired_token_count;
    bdmf_boolean clr_recovered_token_count;
} fpm_token_recover_ctl;

typedef struct
{
    bdmf_boolean searchdata10_m;
    uint8_t searchdata11;
    uint8_t searchdata12;
    uint8_t searchdata13;
    uint8_t searchdata14;
    uint8_t searchdata15;
} fpm_search_data_1;

typedef struct
{
    uint8_t searchdata0;
    uint8_t searchdata1;
    uint8_t searchdata2;
    uint8_t searchdata3;
    uint8_t searchdata4;
    uint8_t searchdata5;
    uint8_t searchdata6;
    uint8_t searchdata7;
    uint8_t searchdata8;
    uint8_t searchdata9;
    uint8_t searchdata10_l;
} fpm_search_data_2;

typedef struct
{
    bdmf_boolean searchdata10_m;
    uint8_t searchdata11;
    uint8_t searchdata12;
    uint8_t searchdata13;
    uint8_t searchdata14;
    uint8_t searchdata15;
} fpm_search_data_3;

typedef struct
{
    bdmf_boolean fifo_empty;
    bdmf_boolean fifo_full;
    uint8_t fifo_used_words;
    uint8_t fifo_rd_cntr;
    uint8_t fifo_wr_cntr;
} fpm_fpm_bb_dbg_rxfifo_sts;

typedef struct
{
    bdmf_boolean fifo_empty;
    bdmf_boolean fifo_full;
    uint8_t fifo_used_words;
    uint8_t fifo_rd_cntr;
    uint8_t fifo_wr_cntr;
} fpm_fpm_bb_dbg_txfifo_sts;

typedef struct
{
    bdmf_boolean old_task_num;
    bdmf_boolean alc_fre_arb_rr;
    bdmf_boolean alc_fst_ack;
    uint8_t pool_0_size;
    uint8_t pool_1_size;
    uint8_t poolx_en;
} fpm_fpm_bb_misc;

typedef struct
{
    bdmf_boolean bypass_clk_gate;
    uint8_t timer_val;
    bdmf_boolean keep_alive_en;
    uint8_t keep_alive_intrvl;
    uint8_t keep_alive_cyc;
} fpm_clk_gate_cntrl;


/**********************************************************************************************************************
 * init_mem: 
 *     Clear memory - Initialize all bits of the usage index array memory to zero's
 *     This is a self clearing bit. Once software writes a 1'b1 to enable, hardware initializes the memory and resets
 *     this bit back to 1'b0 at completion of initialization. Software can poll this bit and check for a value a zero
 *     that indicates initialization completion status
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_fpm_init_mem_set(bdmf_boolean init_mem);
bdmf_error_t ag_drv_fpm_init_mem_get(bdmf_boolean *init_mem);

/**********************************************************************************************************************
 * pool1_enable: 
 *     Enable POOL1 token allocation / deallocation
 *     0 = Disabled
 *     1 = Enabled
 *     
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_fpm_pool1_en_set(bdmf_boolean pool1_enable);
bdmf_error_t ag_drv_fpm_pool1_en_get(bdmf_boolean *pool1_enable);

/**********************************************************************************************************************
 * fpm_bb_soft_reset: 
 *     Set to 1 to hold the FPM Broadbus interface in reset. This is useful for maintaining a known state on that
 *     interface when Runner is powered down.
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_fpm_bb_reset_set(bdmf_boolean fpm_bb_soft_reset);
bdmf_error_t ag_drv_fpm_bb_reset_get(bdmf_boolean *fpm_bb_soft_reset);

/**********************************************************************************************************************
 * ddr0_alloc_weight: 
 *     Weight assigned to each alloc from pool for DDR0
 *     
 * ddr0_free_weight: 
 *     Weight assigned to each free to pool for DDR0
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_fpm_ddr0_weight_set(uint8_t ddr0_alloc_weight, uint8_t ddr0_free_weight);
bdmf_error_t ag_drv_fpm_ddr0_weight_get(uint8_t *ddr0_alloc_weight, uint8_t *ddr0_free_weight);

/**********************************************************************************************************************
 * ddr1_alloc_weight: 
 *     Weight assigned to each alloc from pool for DDR1
 *     
 * ddr1_free_weight: 
 *     Weight assigned to each free to pool for DDR1
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_fpm_ddr1_weight_set(uint8_t ddr1_alloc_weight, uint8_t ddr1_free_weight);
bdmf_error_t ag_drv_fpm_ddr1_weight_get(uint8_t *ddr1_alloc_weight, uint8_t *ddr1_free_weight);

/**********************************************************************************************************************
 * fp_buf_size: 
 *     Selects the size of the buffer to be used in the pool. All buffers must be the same size.
 *     0 - 512 byte buffers
 *     1 - 256 byte buffers
 *     all other values - reserved
 *     
 * pool_base_address: 
 *     Buffer base address. 7:2 must be 0x00.
 *     
 * pool_base_address_pool2: 
 *     Buffer base address. 7:2 must be 0x00.
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_fpm_pool_cfg_set(const fpm_pool_cfg *pool_cfg);
bdmf_error_t ag_drv_fpm_pool_cfg_get(fpm_pool_cfg *pool_cfg);

/**********************************************************************************************************************
 * ovrfl: 
 *     Free Pool overflow count
 *     
 * undrfl: 
 *     Free Pool underflow count
 *     
 * pool_full: 
 *     POOL is full
 *     This indicates that all tokens have been allocated and there no free tokens available. This bit will be active
 *     as long as all usage array is fully allocated.
 *     
 * free_fifo_full: 
 *     FREE_FIFO is full.
 *     
 * free_fifo_empty: 
 *     FREE_FIFO is empty
 *     
 * alloc_fifo_full: 
 *     ALLOC_FIFO is full
 *     
 * alloc_fifo_empty: 
 *     ALLOC_FIFO is empty.
 *     
 * num_of_tokens_available: 
 *     Count of tokens available for allocation.
 *     This provides a count of number of free tokens that available for allocation in the usage array. This value is
 *     updated instantaneously as tokens are allocated or freed from the array.
 *     
 * num_of_not_valid_token_frees: 
 *     Count of de-allocate token requests with invalid tokens. For more information on conditions under which this
 *     counter is incremented, refer to POOL1_INTR_STS register (offset 0x14) bit[3] explanation in this document.
 *     
 * num_of_not_valid_token_multi: 
 *     Count of multi-cast token update requests with either valid bit not set, For more information on conditions
 *     under which this counter is incremented, refer to POOL1_INTR_STS register (offset 0x14) bit[5] explanation in
 *     this document.
 *     
 * invalid_free_token: 
 *     Token that causes intr[3] or intr[4] active. If there are multiple tokens that causes this error, only the
 *     first one is captured. To capture successive tokens that causes the error this register should be cleared by
 *     writing any random value. Bitmap for these bits is shown below (reserved bits are either zeros or can reflect
 *     the length of the packet associated with the freed token)
 *     Bit[30]    - Reserved
 *     Bit[29:12] - Token
 *     Bit[11:0]  - Reserved
 *     
 * invalid_mcast_token: 
 *     Token that causes intr[5] or intr[6] active. If there are multiple tokens that causes this error, only the
 *     first one is captured. To capture successive tokens that causes the error this register should be cleared by
 *     writing any random value. Bitmap for these bits is shown below (reserved bits are zeros)
 *     Bit[30]    - Reserved
 *     Bit[29:12] - Token
 *     Bit[11]    - Mcast update type (refer to register 0x224[11])
 *     Bit[10:7]  - Reserved
 *     Bit[6:0]   - Mcast value
 *     
 * tokens_available_low_wtmk: 
 *     Lowest value the NUM_OF_TOKENS_AVAIL count has reached.
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_fpm_pool_stat_set(const fpm_pool_stat *pool_stat);
bdmf_error_t ag_drv_fpm_pool_stat_get(fpm_pool_stat *pool_stat);

/**********************************************************************************************************************
 * token_valid: 
 *     Valid Token Indicator
 *     0: No buffers available
 *     1: A valid token index is provided. If a token is de-allocated/freed without this bit set that causes an error
 *     and the token will be ignored, error counter in register offset 0xB8 will be incremented.
 *     
 * ddr: 
 *     DDR Identifier
 *     0: DDR0
 *     1: DDR1
 *     
 * token_index: 
 *     Buffer Index Pointer
 *     
 * token_size: 
 *     Buffer length or packet size in bytes
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_fpm_pool4_alloc_dealloc_set(bdmf_boolean token_valid, bdmf_boolean ddr, uint32_t token_index, uint16_t token_size);
bdmf_error_t ag_drv_fpm_pool4_alloc_dealloc_get(bdmf_boolean *token_valid, bdmf_boolean *ddr, uint32_t *token_index, uint16_t *token_size);

/**********************************************************************************************************************
 * token_valid: 
 *     Valid Token Indicator
 *     0: No buffers available
 *     1: A valid token index is provided. If a token is de-allocated/freed without this bit set that causes an error
 *     and the token will be ignored, error counter in register offset 0xB8 will be incremented.
 *     
 * ddr: 
 *     DDR Identifier
 *     0: DDR0
 *     1: DDR1
 *     
 * token_index: 
 *     Buffer Index Pointer
 *     
 * token_size: 
 *     Buffer length or packet size in bytes
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_fpm_pool3_alloc_dealloc_set(bdmf_boolean token_valid, bdmf_boolean ddr, uint32_t token_index, uint16_t token_size);
bdmf_error_t ag_drv_fpm_pool3_alloc_dealloc_get(bdmf_boolean *token_valid, bdmf_boolean *ddr, uint32_t *token_index, uint16_t *token_size);

/**********************************************************************************************************************
 * token_valid: 
 *     Valid Token Indicator
 *     0: No buffers available
 *     1: A valid token index is provided. If a token is de-allocated/freed without this bit set that causes an error
 *     and the token will be ignored, error counter in register offset 0xB8 will be incremented.
 *     
 * ddr: 
 *     DDR Identifier
 *     0: DDR0
 *     1: DDR1
 *     
 * token_index: 
 *     Buffer Index Pointer
 *     
 * token_size: 
 *     Buffer length or packet size in bytes
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_fpm_pool2_alloc_dealloc_set(bdmf_boolean token_valid, bdmf_boolean ddr, uint32_t token_index, uint16_t token_size);
bdmf_error_t ag_drv_fpm_pool2_alloc_dealloc_get(bdmf_boolean *token_valid, bdmf_boolean *ddr, uint32_t *token_index, uint16_t *token_size);

/**********************************************************************************************************************
 * token_valid: 
 *     Valid Token Indicator
 *     0: No buffers available
 *     1: A valid token index is provided. If a token is de-allocated/freed without this bit set that causes an error
 *     and the token will be ignored, error counter in register offset 0xB8 will be incremented.
 *     
 * ddr: 
 *     DDR Identifier
 *     0: DDR0
 *     1: DDR1
 *     
 * token_index: 
 *     Buffer Index Pointer
 *     
 * token_size: 
 *     Buffer length or packet size in bytes
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_fpm_pool1_alloc_dealloc_set(bdmf_boolean token_valid, bdmf_boolean ddr, uint32_t token_index, uint16_t token_size);
bdmf_error_t ag_drv_fpm_pool1_alloc_dealloc_get(bdmf_boolean *token_valid, bdmf_boolean *ddr, uint32_t *token_index, uint16_t *token_size);

/**********************************************************************************************************************
 * mem_data1: 
 *     Memory Data 1
 *     This contains the lower 32 bits (bits[31:0]) of 32/64 bit data
 *     
 * mem_data2: 
 *     Memory Data 2
 *     This contains the upper 32 bits (bits[63:32]) of 64 bit data. The value in this register should be ignored
 *     during 32 bit access
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_fpm_back_door_mem_set(uint32_t mem_data1, uint32_t mem_data2);
bdmf_error_t ag_drv_fpm_back_door_mem_get(uint32_t *mem_data1, uint32_t *mem_data2);

/**********************************************************************************************************************
 * expired_count: 
 *     Cumulative count of the number of expired tokens detected in the token recovery process. The count can be
 *     cleared by setting the CLR_EXPIRED_TOKEN_COUNT in the TOKEN_RECOVER_CTL register
 *     
 * recovered_count: 
 *     Cumulative count of the number of expired tokens that were freed in the token recovery process. The count can
 *     be cleared by setting the CLR_RECOVERED_TOKEN_COUNT in the TOKEN_RECOVER_CTL register
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_fpm_pool1_count_get(uint32_t *expired_count, uint32_t *recovered_count);

/**********************************************************************************************************************
 * long_aging_timer: 
 *     Aging timer used in token recovery
 *     
 * short_aging_timer: 
 *     Aging timer used in token recovery
 *     
 * recycle_timer: 
 *     Timer used in token recovery logic. Upon expiration of timer, one token from the allocate cache will be freed.
 *     Over time, all cached tokens will be recycled back to the freepool. This will prevent the cached tokens frm
 *     being aged out by the token recovery logic. This timer should be set to a value so that all tokens can be
 *     recycled before the aging timer expires.
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_fpm_timer_set(const fpm_timer *timer);
bdmf_error_t ag_drv_fpm_timer_get(fpm_timer *timer);

/**********************************************************************************************************************
 * pool1_search_mode: 
 *     Index memory search method
 *     (For more info refer to FPM architecture wiki page)
 *     0 = Method 1
 *     1 = Method 2
 *     
 *     
 * enable_jumbo_support: 
 *     Enable Jumbo Support0 = Disabled 1
 *     1 = Enabled 2
 *     
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_fpm_fpm_cfg1_set(bdmf_boolean pool1_search_mode, bdmf_boolean enable_jumbo_support);
bdmf_error_t ag_drv_fpm_fpm_cfg1_get(bdmf_boolean *pool1_search_mode, bdmf_boolean *enable_jumbo_support);

/**********************************************************************************************************************
 * bb_ddr_sel: 
 *     Select pool/DDR to be used when FPM_BB allocates tokens
 *     11 = reserved
 *     10 = allocate from both pools
 *     01 = pool1/DDR1
 *     00 = pool0/DDR0
 *     
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_fpm_fpm_bb_cfg_set(uint8_t bb_ddr_sel);
bdmf_error_t ag_drv_fpm_fpm_bb_cfg_get(uint8_t *bb_ddr_sel);

/**********************************************************************************************************************
 * alloc_fifo_full_msk: 
 *     Allocation FIFO Full Interrupt mask.
 *     
 * free_fifo_full_msk: 
 *     De-Allocation FIFO Full Interrupt mask.
 *     
 * pool_full_msk: 
 *     Usage Index Pool is fully allocated interrupt mask.
 *     
 * free_token_no_valid_msk: 
 *     De-allocation token request with invalid token.
 *     
 * free_token_index_out_of_range_msk: 
 *     De-allocation token request with index out-of-range.
 *     
 * multi_token_no_valid_msk: 
 *     Token multi-cast value update request with invalid token.
 *     
 * multi_token_index_out_of_range_msk: 
 *     Token multi-cast value update request with index out-of-range.
 *     
 * pool_dis_free_multi_msk: 
 *     Free or Mcast update on disabled pool interrupt mask .
 *     
 * memory_corrupt_msk: 
 *     Index Memory corrupt interrupt mask.
 *     
 * xoff_msk: 
 *     XOFF_STATE interrupt mask.
 *     
 * xon_msk: 
 *     XON_STATE interrupt mask.
 *     
 * illegal_address_access_msk: 
 *     Illegal/un-implemented register/memory space access  interrupt mask.
 *     
 * illegal_alloc_request_msk: 
 *     Illegal token request interrupt mask.
 *     
 * expired_token_det_msk: 
 *     Expired token detect interrupt mask.
 *     
 * expired_token_recov_msk: 
 *     Expired token recovered interrupt mask.
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_fpm_pool1_intr_msk_set(const fpm_pool1_intr_msk *pool1_intr_msk);
bdmf_error_t ag_drv_fpm_pool1_intr_msk_get(fpm_pool1_intr_msk *pool1_intr_msk);

/**********************************************************************************************************************
 * alloc_fifo_full_sts: 
 *     Allocation FIFO Full Interrupt. This is a functional status bit, not an error status bit. This indicates that
 *     allocation FIFO is full with new tokens to be allocated and will be active (high) as long as FIFO is full. This
 *     status is intended to be used for debug purpose only.
 *     
 * free_fifo_full_sts: 
 *     De-Allocation FIFO Full Interrupt. This is a functional status bit, not an error status bit. This indicates
 *     that de-allocation FIFO is full with tokens needs to be freed and will be active (high) as long as FIFO is
 *     full. This status is intended to be used for debug purpose only.
 *     
 * pool_full_sts: 
 *     Usage Index Pool is fully allocated interrupt. This is a functional status bit, not an error status bit. This
 *     indicates that token pool is fully allocated and there are no free tokens available. This bit will be active
 *     (high) as long as there no free tokens available to allocate. This bit is intended to be used for debug purpose
 *     only.
 *     
 * free_token_no_valid_sts: 
 *     De-allocation token request with invalid token Interrupt.
 *     Invalid free token is determined when one or more the following conditions are met -
 *     1. Incoming multi-cast request token is not valid ( 0xffffffff )
 *     2. Incoming free request token entry in the usage array indicates it is not an allocated token, i.e.,
 *     associated use count value for this count in the usage array is zero
 *     Note: item 2 is not checked if auto_pool_en is set. The auto_pool_en bit is always set when using the new token
 *     format without a pool select field.
 *     
 * free_token_index_out_of_range_sts: 
 *     Not applicable for BCM3391.  All token values are valid.
 *     De-allocation token request with index out-of-range Interrupt.
 *     Free token index out of range is determined when one or more of the following conditions are met -
 *     1. Incoming free request token index is not aligned to the pool size indicated by the pool select field
 *     (bits[29:28])
 *     2. The buffer size indicated by the size field (bits[11:0]) is greater than the size of the allocated token.
 *     There is no associated count for this error. Note: item 1 is not checked if auto_pool_en is set. The
 *     auto_pool_en bit is always set when using the new token format without a pool select field.
 *     
 * multi_token_no_valid_sts: 
 *     Token multi-cast value update request with invalid token Interrupt.
 *     Invalid multi-cast token is determined when one or more the following conditions are met -
 *     1. Incoming multi-cast request token is not valid ( 0xffffffff )
 *     2. Incoming multi-cast request token has use count field set to zero
 *     3. Incoming multi-cast request token entry in the usage array indicates it is not an allocated token, i.e.,
 *     associated use count value for this count in the usage array is zero
 *     4. After updating the use count value, the new use count value exceeds 0x7E
 *     Note: item 2 is not checked if auto_pool_en is set. The auto_pool_en bit is always set when using the new token
 *     format without a pool select field.
 *     
 * multi_token_index_out_of_range_sts: 
 *     Token multi-cast value update request with index out-of-range Interrupt.
 *     This is set when the token index is not aligned to the pool size. This is determined by examining the pool
 *     select field (bits[29:28]) and the 3 lsbs of the token index (bits[14:12]). There is no associated count for
 *     this error. Note: this error is not checked if auto_pool_en is set. The auto_pool_en bit is always set when
 *     using the new token format without a pool select field.
 *     
 * pool_dis_free_multi_sts: 
 *     Free or Mcast update on disabled pool interrupt.
 *     This bit goes active when a free or multi-cast request is received and FPM is not enabled, i.e., pool enable
 *     bit in FPM control register is not set to 1'b1.
 *     
 * memory_corrupt_sts: 
 *     Index Memory is corrupted.
 *     During updates of the usage array, token manager checks if the use count and search tree value in the array has
 *     a legal value. If the use count or search tree value is not correct before updating, logic generates an error
 *     and interrupt. As long as the interrupt is active no more valid tokens will be allocated because this is a
 *     catastrophic error. Following are the two error conditions that are checked -
 *     1.    During search for a free token, a particular token use count value indicates it is allocated (use count
 *     is greater than 0), but corresponding upper level search tree value indicates the token is still available
 *     (with bit value of 1'b0, instead of 1'b1). This is an error.
 *     2.    During search for a free token, a particular token use count value indicates that it is free (use count
 *     is 0), but corresponding upper level search tree value indicates the token is not available (with bit value of
 *     1'b1, instead of 1'b0). This is an error.
 *     
 * xoff_state_sts: 
 *     Number of available tokens is less than or equal to XOFF_THRESHOLD value in XON/XOFF Threshold configuration
 *     register. This is a functional status bit, not an error status bit. Using this information FPM generates
 *     backpressure output signal that is used by other UBUS client logics to throttle its operation. For example,
 *     UNIMAC logic can use backpressure signal to transfer PAUSE Ethernet flow control packets to throttle incoming
 *     frames on Ethernet interface.
 *     
 * xon_state_sts: 
 *     Number of available tokens is greater than or equal to XON_THRESHOLD value in XON/XOFF Threshold configuration
 *     register. This is a functional status bit, not an error status bit. Using this information FPM generates
 *     backpressure output signal that is used by other UBUS client logics to throttle its operation. For example,
 *     UNIMAC logic can use backpressure signal to transfer PAUSE Ethernet flow control packets to throttle incoming
 *     frames on Ethernet interface.
 *     
 * illegal_address_access_sts: 
 *     Illegal/un-implemented register/memory space access interrupt. This will be active when there is an attempt to
 *     read from an unimplemented register or memory space. Along with interrupt being sent an error reply packet will
 *     be sent out with o_ubus_error_out asserted.
 *     
 * illegal_alloc_request_sts: 
 *     Illegal token request interrupt. This will be active when the pool is disabled, there is a request for a new
 *     token and the alloc fifo for the selected token size is empty. Along with interrupt being sent an error reply
 *     packet will be sent out with o_ubus_error_out asserted.
 *     
 * expired_token_det_sts: 
 *     Expired token detect interrupt. This is set when the token recovery logic detects a token that has been held
 *     for the entire duration of the aging timer.
 *     
 * expired_token_recov_sts: 
 *     Expired token recovered interrupt. This is set when an expired token has been recoveredand returned to pool as
 *     an available token.
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_fpm_pool1_intr_sts_set(const fpm_pool1_intr_sts *pool1_intr_sts);
bdmf_error_t ag_drv_fpm_pool1_intr_sts_get(fpm_pool1_intr_sts *pool1_intr_sts);

/**********************************************************************************************************************
 * free_token_no_valid_stall_msk: 
 *     Stall FPM on De-allocation token request with invalid token interrupt status.
 *     
 * free_token_index_out_of_range_stall_msk: 
 *     Stall FPM on De-allocation token request with index out-of-range interrupt status.
 *     
 * multi_token_no_valid_stall_msk: 
 *     Stall FPM on Token multi-cast value update request with invalid token interrupt status.
 *     
 * multi_token_index_out_of_range_stall_msk: 
 *     Stall FPM on Token multi-cast value update request with index out-of-range interrupt status.
 *     
 * memory_corrupt_stall_msk: 
 *     Stall FPM on Index Memory corrupt interrupt status.
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_fpm_pool1_stall_msk_set(const fpm_pool1_stall_msk *pool1_stall_msk);
bdmf_error_t ag_drv_fpm_pool1_stall_msk_get(fpm_pool1_stall_msk *pool1_stall_msk);

/**********************************************************************************************************************
 * pool_base_address_pool3: 
 *     Buffer base address. 7:2 must be 0x00.
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_fpm_pool1_cfg4_set(uint32_t pool_base_address_pool3);
bdmf_error_t ag_drv_fpm_pool1_cfg4_get(uint32_t *pool_base_address_pool3);

/**********************************************************************************************************************
 * mem_corrupt_sts_related_alloc_token: 
 *     Token that causes memory corrupt interrupt active. If there are multiple tokens that causes this error, only
 *     the first one is captured. To capture successive tokens that causes the error this register should be cleared
 *     by writing any random value, in addition, memory corrupt status bit (bit[8]) in interrupt status register 0x14
 *     should be cleared. Bitmap for these bits is shown below (reserved bits are zeros)
 *     Bit[30]    - Reserved
 *     Bit[29:12] - Token
 *     Bit[11:0]  - Buffer size in bytes
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_fpm_pool1_stat5_set(uint32_t mem_corrupt_sts_related_alloc_token);
bdmf_error_t ag_drv_fpm_pool1_stat5_get(uint32_t *mem_corrupt_sts_related_alloc_token);

/**********************************************************************************************************************
 * xoff_threshold: 
 *     XOFF Threshold value
 *     Indicates the lower threshold of available tokens at which XOFF condition is set.
 *     Each threshold value represents 256 buffers
 *     
 * xon_threshold: 
 *     XON Threshold value
 *     Indicates the upper threshold of available tokens at which XON condition is set.
 *     Each threshold value represents 256 buffers
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_fpm_pool1_xon_xoff_cfg_set(uint16_t xoff_threshold, uint16_t xon_threshold);
bdmf_error_t ag_drv_fpm_pool1_xon_xoff_cfg_get(uint16_t *xoff_threshold, uint16_t *xon_threshold);

/**********************************************************************************************************************
 * not_empty_threshold: 
 *     Threshold value for reasserting pool_not_empty to FPM_BB
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_fpm_fpm_not_empty_cfg_set(uint8_t not_empty_threshold);
bdmf_error_t ag_drv_fpm_fpm_not_empty_cfg_get(uint8_t *not_empty_threshold);

/**********************************************************************************************************************
 * mem_addr: 
 *     Memory address for write/read location
 *     This is DWord aligned address
 *     
 * mem_sel: 
 *     2'b00 = Reserved
 *     2'b01 = FPM Memory
 *     2'b10 = Reserved
 *     2'b11 = When memory is enabled, bit[31]=1, this value will allow a write to NUM_OF_TOKENS_AVAILABLE field
 *     [21:0] in POOL1_STAT2 register (offset 0x54). This should be used for debug purposes only
 *     
 * mem_rd: 
 *     Read control bit for Usage index array memory. This is a self clearing bit, cleared by hardware to zero once
 *     memory read is  complete. Software can read more locations if the bit value is zero
 *     
 * mem_wr: 
 *     Write control bit for Usage index array memory. This is a self clearing bit, cleared by hardware to zero once
 *     memory write is  complete. Software can write more locations if the bit value is zero
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_fpm_mem_ctl_set(uint16_t mem_addr, uint8_t mem_sel, bdmf_boolean mem_rd, bdmf_boolean mem_wr);
bdmf_error_t ag_drv_fpm_mem_ctl_get(uint16_t *mem_addr, uint8_t *mem_sel, bdmf_boolean *mem_rd, bdmf_boolean *mem_wr);

/**********************************************************************************************************************
 * token_recover_ena: 
 *     Token recovery enable
 *     1 = Enabled
 *     0 = Disabled
 *     
 *     
 * single_pass_ena: 
 *     If token recovery is enabled, the single-pass control will indicate whether the hardware should perform just
 *     one iteration of the token recovery process or will continuously loop through the token recovery process.
 *     1 = Single pass
 *     0 = Auto repeat
 *     
 *     
 * token_remark_ena: 
 *     Enable remarking of tokens for multiple passes through the token recovery process. The mark bit is set on all
 *     tokens on the first pass through the loop. When this bit is set, the mark bits will be set again on all
 *     subsequent passes through the loop. It is anticipated that this bit will always be set when token recovery is
 *     enabled. It is provided as a potential debug tool.
 *     1 = Enabled
 *     0 = Disabled
 *     
 *     
 * token_reclaim_ena: 
 *     Enable automatic return of marked tokens to the freepool
 *     1 = Enabled
 *     0 = Disabled
 *     
 *     
 * force_token_reclaim: 
 *     Non-automated token recovery.
 *     This bit can be used when automatic token return is not enabled. When software gets an interrupt indicating
 *     that the token recovery process has detected expired tokens, it can set this bit to force the expired tokens to
 *     be reclaimed.
 *     1 = Enabled
 *     0 = Disabled
 *     
 *     
 * clr_expired_token_count: 
 *     This is a self-clearing bit. Write a 1 to the bit to reset  the EXPIRED_TOKEN_COUNT to 0.
 *     
 * clr_recovered_token_count: 
 *     This is a self-clearing bit. Write a 1 to the bit to reset  the RECOVERED_TOKEN_COUNT to 0.
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_fpm_token_recover_ctl_set(const fpm_token_recover_ctl *token_recover_ctl);
bdmf_error_t ag_drv_fpm_token_recover_ctl_get(fpm_token_recover_ctl *token_recover_ctl);

/**********************************************************************************************************************
 * end_index: 
 *     End of token index range to be used when performing token recovery.
 *     
 * start_index: 
 *     Start of token index range to be used when performing token recovery.
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_fpm_token_recover_start_end_pool1_set(uint16_t end_index, uint16_t start_index);
bdmf_error_t ag_drv_fpm_token_recover_start_end_pool1_get(uint16_t *end_index, uint16_t *start_index);

/**********************************************************************************************************************
 * mask: 
 *     ORing bit mask for output PRBS signal.
 *     
 * enable: 
 *     Enable PRBS invalid token generation.
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_fpm_prbs_invalid_gen_set(uint32_t mask, bdmf_boolean enable);
bdmf_error_t ag_drv_fpm_prbs_invalid_gen_get(uint32_t *mask, bdmf_boolean *enable);

/**********************************************************************************************************************
 * searchdata10_m: 
 *     Search Tree.
 *     msb of data10
 *     
 * searchdata11: 
 *     Search Tree
 *     
 * searchdata12: 
 *     Search Tree
 *     
 * searchdata13: 
 *     Search Tree
 *     
 * searchdata14: 
 *     Search Tree
 *     
 * searchdata15: 
 *     Search Tree
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_fpm_search_data_1_set(uint32_t search1, const fpm_search_data_1 *search_data_1);
bdmf_error_t ag_drv_fpm_search_data_1_get(uint32_t search1, fpm_search_data_1 *search_data_1);

/**********************************************************************************************************************
 * searchdata0: 
 *     Search Tree
 *     
 * searchdata1: 
 *     Search Tree
 *     
 * searchdata2: 
 *     Search Tree
 *     
 * searchdata3: 
 *     Search Tree
 *     
 * searchdata4: 
 *     Search Tree
 *     
 * searchdata5: 
 *     Search Tree
 *     
 * searchdata6: 
 *     Search Tree
 *     
 * searchdata7: 
 *     Search Tree
 *     
 * searchdata8: 
 *     Search Tree
 *     
 * searchdata9: 
 *     Search Tree
 *     
 * searchdata10_l: 
 *     Search Tree.
 *     2 lsbs of data10
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_fpm_search_data_2_set(uint32_t search1, const fpm_search_data_2 *search_data_2);
bdmf_error_t ag_drv_fpm_search_data_2_get(uint32_t search1, fpm_search_data_2 *search_data_2);

/**********************************************************************************************************************
 * searchdata10_m: 
 *     Search Tree.
 *     msb of data10
 *     
 * searchdata11: 
 *     Search Tree
 *     
 * searchdata12: 
 *     Search Tree
 *     
 * searchdata13: 
 *     Search Tree
 *     
 * searchdata14: 
 *     Search Tree
 *     
 * searchdata15: 
 *     Search Tree
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_fpm_search_data_3_set(uint32_t search3, const fpm_search_data_3 *search_data_3);
bdmf_error_t ag_drv_fpm_search_data_3_get(uint32_t search3, fpm_search_data_3 *search_data_3);

/**********************************************************************************************************************
 * multicast: 
 *     Multicast Value:
 *     32b for 32 tokens
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_fpm_multicast_data_set(uint32_t multicast_data, uint32_t multicast);
bdmf_error_t ag_drv_fpm_multicast_data_get(uint32_t multicast_data, uint32_t *multicast);

/**********************************************************************************************************************
 * poolid: 
 *     Computed pool
 *     0 - pool id 0
 *     1 - pool id 1
 *     2 - pool id 2
 *     3 - pool id 3
 *     0xff - token not allocated
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_fpm_compute_pool_data_get(uint32_t compute_pool_data, uint8_t *poolid);

/**********************************************************************************************************************
 * force: 
 *     Write 1 to force BB transaction
 **********************************************************************************************************************/
bdmf_error_t ag_drv_fpm_fpm_bb_force_set(bdmf_boolean force);
bdmf_error_t ag_drv_fpm_fpm_bb_force_get(bdmf_boolean *force);

/**********************************************************************************************************************
 * ctrl: 
 *     Forced control
 **********************************************************************************************************************/
bdmf_error_t ag_drv_fpm_fpm_bb_forced_ctrl_set(uint16_t ctrl);
bdmf_error_t ag_drv_fpm_fpm_bb_forced_ctrl_get(uint16_t *ctrl);

/**********************************************************************************************************************
 * ta_addr: 
 *     Forced TA address
 * dest_addr: 
 *     Forced destination address
 **********************************************************************************************************************/
bdmf_error_t ag_drv_fpm_fpm_bb_forced_addr_set(uint16_t ta_addr, uint8_t dest_addr);
bdmf_error_t ag_drv_fpm_fpm_bb_forced_addr_get(uint16_t *ta_addr, uint8_t *dest_addr);

/**********************************************************************************************************************
 * data: 
 *     Forced data
 **********************************************************************************************************************/
bdmf_error_t ag_drv_fpm_fpm_bb_forced_data_set(uint32_t data);
bdmf_error_t ag_drv_fpm_fpm_bb_forced_data_get(uint32_t *data);

/**********************************************************************************************************************
 * dest_id: 
 *     destination id
 * override_en: 
 *     Enable override
 * route_addr: 
 *     route address
 **********************************************************************************************************************/
bdmf_error_t ag_drv_fpm_fpm_bb_decode_cfg_set(uint8_t dest_id, bdmf_boolean override_en, uint16_t route_addr);
bdmf_error_t ag_drv_fpm_fpm_bb_decode_cfg_get(uint8_t *dest_id, bdmf_boolean *override_en, uint16_t *route_addr);

/**********************************************************************************************************************
 * rxfifo_sw_addr: 
 *     SW address for reading FPM BB RXFIFO
 * txfifo_sw_addr: 
 *     SW address for reading FPM BB TXFIFO
 * rxfifo_sw_rst: 
 *     SW reset for FPM BB RXFIFO
 * txfifo_sw_rst: 
 *     SW reset for FPM BB TXFIFO
 **********************************************************************************************************************/
bdmf_error_t ag_drv_fpm_fpm_bb_dbg_cfg_set(uint8_t rxfifo_sw_addr, uint8_t txfifo_sw_addr, bdmf_boolean rxfifo_sw_rst, bdmf_boolean txfifo_sw_rst);
bdmf_error_t ag_drv_fpm_fpm_bb_dbg_cfg_get(uint8_t *rxfifo_sw_addr, uint8_t *txfifo_sw_addr, bdmf_boolean *rxfifo_sw_rst, bdmf_boolean *txfifo_sw_rst);

/**********************************************************************************************************************
 * fifo_empty: 
 *     FIFO is empty
 * fifo_full: 
 *     FIFO is full
 * fifo_used_words: 
 *     Used words
 * fifo_rd_cntr: 
 *     Write counter
 * fifo_wr_cntr: 
 *     Write counter
 **********************************************************************************************************************/
bdmf_error_t ag_drv_fpm_fpm_bb_dbg_rxfifo_sts_get(fpm_fpm_bb_dbg_rxfifo_sts *fpm_bb_dbg_rxfifo_sts);

/**********************************************************************************************************************
 * fifo_empty: 
 *     FIFO is empty
 * fifo_full: 
 *     FIFO is full
 * fifo_used_words: 
 *     Used words
 * fifo_rd_cntr: 
 *     Write counter
 * fifo_wr_cntr: 
 *     Write counter
 **********************************************************************************************************************/
bdmf_error_t ag_drv_fpm_fpm_bb_dbg_txfifo_sts_get(fpm_fpm_bb_dbg_txfifo_sts *fpm_bb_dbg_txfifo_sts);

/**********************************************************************************************************************
 * data: 
 *     data
 **********************************************************************************************************************/
bdmf_error_t ag_drv_fpm_fpm_bb_dbg_rxfifo_data1_get(uint32_t *data);

/**********************************************************************************************************************
 * data: 
 *     data
 **********************************************************************************************************************/
bdmf_error_t ag_drv_fpm_fpm_bb_dbg_rxfifo_data2_get(uint32_t *data);

/**********************************************************************************************************************
 * data: 
 *     data
 **********************************************************************************************************************/
bdmf_error_t ag_drv_fpm_fpm_bb_dbg_txfifo_data1_get(uint32_t *data);

/**********************************************************************************************************************
 * data: 
 *     data
 **********************************************************************************************************************/
bdmf_error_t ag_drv_fpm_fpm_bb_dbg_txfifo_data2_get(uint32_t *data);

/**********************************************************************************************************************
 * data: 
 *     data
 **********************************************************************************************************************/
bdmf_error_t ag_drv_fpm_fpm_bb_dbg_txfifo_data3_get(uint32_t *data);

/**********************************************************************************************************************
 * old_task_num: 
 *     old_task_num_format.
 *     1: default. task_num in legacy bits (according to bb_message excel)
 *     0: new. task_num in bits 59:56 of bb_fpm_data_in
 * alc_fre_arb_rr: 
 *     alloc_free_arb_rr
 *     0: default(legacy) - no rr, free has priority over alloc.
 *     1: new option. rr between free and alloc
 * alc_fst_ack: 
 *     alloc_free_rr
 *     0: default(legacy) - no fast ack. wait until all free commands were popped, and array was searched.
 *     1: new option. ack for alloc is returned immediately according to emptiness of alloc prefetch fifo only.
 * pool_0_size: 
 *     Select how many token are related to pool0:
 *     
 *     0: 6 tkns,
 *     1: 7 tkns,
 *     2: 8 tkns(default),
 *     3: 20 tkns,
 *     
 * pool_1_size: 
 *     Select how many token are related to pool1:
 *     0: 4 tkns(default),
 *     1: 5 tkns,
 *     2: 6 tkns,
 *     3: 8 tkns
 *     
 *     Note: No corresponding configuration for pools_2/3_size, since they always get default: 2 tkns for pool2, 1 tkn
 *     for pool3
 *     
 *     Note: No corresponding configuration for pools_2/3_size, since they always get default: 2 tkns for pool2, 1 tkn
 *     for pool3
 *     
 * poolx_en: 
 *     en bit per pool.
 *     If the bit is 1, the corresponding pool is enabled, if the bit is not 1, the corresponding pool is not enabled.
 *     12: pool0 en,
 *     13: pool1 en,
 *     14: pool2 en,
 *     15: pool3 en,
 *     
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_fpm_fpm_bb_misc_set(const fpm_fpm_bb_misc *fpm_bb_misc);
bdmf_error_t ag_drv_fpm_fpm_bb_misc_get(fpm_fpm_bb_misc *fpm_bb_misc);

/**********************************************************************************************************************
 * bypass_clk_gate: 
 *     If set to 1b1 will disable the clock gate logic such to always enable the clock
 * timer_val: 
 *     For how long should the clock stay active once all conditions for clock disable are met.
 *     
 *     
 * keep_alive_en: 
 *     Enables the keep alive logic which will periodically enable the clock to assure that no deadlock of clock being
 *     removed completely will occur
 * keep_alive_intrvl: 
 *     If the KEEP alive option is enabled the field will determine for how many cycles should the clock be active
 * keep_alive_cyc: 
 *     If the KEEP alive option is enabled this field will determine for how many cycles should the clock be disabled
 *     (minus the KEEP_ALIVE_INTERVAL)
 *     
 *     So KEEP_ALIVE_CYCLE must be larger than KEEP_ALIVE_INTERVAL.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_fpm_clk_gate_cntrl_set(const fpm_clk_gate_cntrl *clk_gate_cntrl);
bdmf_error_t ag_drv_fpm_clk_gate_cntrl_get(fpm_clk_gate_cntrl *clk_gate_cntrl);

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
    cli_fpm_pool4_alloc_dealloc,
    cli_fpm_pool3_alloc_dealloc,
    cli_fpm_pool2_alloc_dealloc,
    cli_fpm_pool1_alloc_dealloc,
    cli_fpm_back_door_mem,
    cli_fpm_pool1_count,
    cli_fpm_timer,
    cli_fpm_fpm_cfg1,
    cli_fpm_fpm_bb_cfg,
    cli_fpm_pool1_intr_msk,
    cli_fpm_pool1_intr_sts,
    cli_fpm_pool1_stall_msk,
    cli_fpm_pool1_cfg4,
    cli_fpm_pool1_stat5,
    cli_fpm_pool1_xon_xoff_cfg,
    cli_fpm_fpm_not_empty_cfg,
    cli_fpm_mem_ctl,
    cli_fpm_token_recover_ctl,
    cli_fpm_token_recover_start_end_pool1,
    cli_fpm_prbs_invalid_gen,
    cli_fpm_search_data_1,
    cli_fpm_search_data_2,
    cli_fpm_search_data_3,
    cli_fpm_multicast_data,
    cli_fpm_compute_pool_data,
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
    cli_fpm_fpm_bb_misc,
    cli_fpm_clk_gate_cntrl,
};

int bcm_fpm_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
bdmfmon_handle_t ag_drv_fpm_cli_init(bdmfmon_handle_t root_dir);

#endif
#endif
