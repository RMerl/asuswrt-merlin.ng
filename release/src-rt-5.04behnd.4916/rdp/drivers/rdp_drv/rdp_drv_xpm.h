/*
    <:copyright-BRCM:2021:DUAL/GPL:standard

       Copyright (c) 2021 Broadcom
       All Rights Reserved

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

#ifndef DRV_XPM_COMMON_H_INCLUDED
#define DRV_XPM_COMMON_H_INCLUDED

#ifdef __cplusplus
extern "C"
{
#endif

#include "rdp_subsystem_common.h"
#include "rdp_common.h"
#ifndef RUNNER_MPM_SUPPORT
/* FPM */
#include "rdp_drv_fpm.h"

#define drv_xpm_free_buffer(_plen, _buffnum, info)   drv_fpm_free_buffer(_plen, _buffnum, info)
#define drv_xpm_check_xoff_plat(_num_of_token) drv_fpm_check_xoff(_num_of_token)
#else
/* MPM */
#include "rdp_drv_mpm.h"

#define FPM_INTERRUPT_TIMER_DELAY           1000  /* msec, dummy for rdpa_system_ex */

#define drv_xpm_free_buffer(_plen, _buffnum, info)   drv_mpm_free_buffer(_plen, _buffnum, info)
#define drv_xpm_check_xoff_plat(_num_of_token) drv_mpm_check_xoff(_num_of_token)
#endif

#if defined(__KERNEL__)
#define ATOMIC_LEVEL_ADD(a,b) atomic_add(a,b)
#define ATOMIC_LEVEL64_ADD(a,b) atomic64_add(a,b)
#else
#define ATOMIC_LEVEL_ADD(a,b) (*(b) = *(b) + (a))
#define ATOMIC_LEVEL64_ADD(a,b) (*(b) = *(b) + (a))
#endif
extern atomic64_t *bufmng_free_req_cnt;
/* common definitions between FPM and MPM, keep them in FPM naming */
/* token size definitions */
#define FPM_BUF_SIZE_256                    256
#define FPM_BUF_SIZE_512                    512
#define FPM_BUF_SIZE_1K                     1024
#define FPM_BUF_SIZE_2K                     2048

#define FPM_BASIC_TOKEN_SIZE_0              FPM_BUF_SIZE_256
#define FPM_BASIC_TOKEN_SIZE_1              FPM_BUF_SIZE_512
#define FPM_BASIC_TOKEN_SIZE_2              FPM_BUF_SIZE_1K
#define FPM_BASIC_TOKEN_SIZE_3              FPM_BUF_SIZE_2K

#define FPM_BUF_SIZE_DEFAULT                FPM_BASIC_TOKEN_SIZE_1

#define RDD_BUF_SIZE_0                      BUF_256
#define RDD_BUF_SIZE_1                      BUF_512
#define RDD_BUF_SIZE_2                      BUF_1024
#define RDD_BUF_SIZE_3                      BUF_2048

#define RDD_BUF_SIZE_DEFAULT                RDD_BUF_SIZE_1

#define FPM_POOL_SET_0                      0x01020408    /* x1,x2,x4,x8 . Default configuration for all chips.
                                                                           Can be changed for CHIP_VER >= RDP_GEN_62 only. */
#if CHIP_VER >= RDP_GEN_62
/*
GEN 62 chips (6888, 6837) with TOTAL_FPM_TOKENS==CONST_INT_256K, support the followin FPM pool settings:
 # Pool 3: x1
 # Pool 2: x2
 # Pool 1: X4 or X5 or X6 or X8
 # Pool 0: X6 or X7 or X8 or x20
*/
#define FPM_POOL_SET_1                      0x01020814    /* x1,x2,x8,x20 */
#define FPM_POOL_SET_2                      0x01020414    /* x1,x2,x4,x20 */
#else
#define FPM_POOL_SET_1                      0xFFFFFFFF    /* Setting FPM_POOL_SET is not supported for CHIP_VER < RDP_GEN_62. */
#define FPM_POOL_SET_2                      0xFFFFFFFF    /* FPM_POOL_SET_1/2 are defined to overcome some compilation problems. */
#endif

/* number of token definitions */
#if !defined(SUPPORTED_NUM_OF_FPM_TOKENS)

#if (CHIP_VER >= RDP_GEN_60)
#error "SUPPORTED_NUM_OF_FPM_TOKENS is not defined"
#endif

/* Unless explicitly specified in rdp_platform.h, assume max num of FPM tokens 64K */
#define FPM_POOL_ID_SHIFT                   (16)
/* All following definitions must be 100*k since they are percent-wise divided */
#define TOTAL_DYNAMIC_FPM                   61000 /* ( 2 (FPM per DQM) * ( 287 (no. of queues) + 16 (prefetch) ) +
                                                     ( 64K (maximal number of PDs in Queue) / (2K / 16) number of PDs in FPM) ) * 4(2K allocation )
                                                     = 4472 FPMs (basic size of 512). Worst case DQM FPM allocation.  */
#else /* SUPPORTED_NUM_OF_FPM_TOKENS */
#if (SUPPORTED_NUM_OF_FPM_TOKENS == CONST_INT_128K)
#if (CHIP_VER >= RDP_GEN_62)
#define FPM_POOL_ID_SHIFT                   (18)
#else
#define FPM_POOL_ID_SHIFT                   (17)
#endif
#define TOTAL_DYNAMIC_FPM                   120000
#elif (SUPPORTED_NUM_OF_FPM_TOKENS == CONST_INT_256K)
#define FPM_POOL_ID_SHIFT                   (18)
#define TOTAL_DYNAMIC_FPM                   250000
#endif
#endif /* SUPPORTED_NUM_OF_FPM_TOKENS */
#define TOTAL_FPM_TOKENS                    (1 << FPM_POOL_ID_SHIFT)    
#define FPM_INDX_MASK                       (TOTAL_FPM_TOKENS - 1)
/* common data structure */
typedef struct {
    void *virt_base;
    void *virt_end;
    uintptr_t phys_base;
    unsigned int buf_size;
    unsigned int buf_size_log2;
    unsigned int num_of_token;
    int pool_size[4];
} xpm_common_cfg_t;



extern xpm_common_cfg_t xpm_common_cfg;
extern uint32_t configured_total_fpm_tokens;
extern ddr_token_info_t *ddr_token_info;
extern bdmf_phys_addr_t ddr_token_info_phy_addr;

#if CHIP_VER >= RDP_GEN_60
/* XOFF threshold calculation is based on the sum of the following 4 reservations:
 * MAX_TX_QUEUES__NUM_OF (in 63146/4912, it is 128 queues; in 6813, it is 160 queues)
 * 1. MAX_TX_QUEUES__NUM_OF x 2K bytes.  // all queue prefetch upon power up
 * 2. MAX_TX_QUEUES__NUM_OF x 2K bytes.  // assume all queues are active for additional prefetch
 * 3. (128K - XOFF) * 16 bytes  = simplify => 128K / 128 * 2K bytes = 1K * 2K Bytes.
 *                            // PDs required when all tokens are used as buffer
 * 4. 960 QM Prefetch buffers
 * => 2K bytes is converted to number of tokens, so for 320 bytes token size,
 *    it requires 8 tokens for 2K Bytes.  4 tokens for 640 bytes token size, etc.
 * real simplified calculation => (MAX_TX_QUEUES__NUM_OF + MAX_TX_QUEUES__NUM_OF + 1K) * token_for_2K + 960 */
static inline uint16_t drv_xpm_xoff_thld(int token_size)
{
    int token_used = FPM_BASIC_TOKEN_SIZE_3 / token_size;
    return (((MAX_TX_QUEUES__NUM_OF + MAX_TX_QUEUES__NUM_OF + (SUPPORTED_NUM_OF_FPM_TOKENS >> 7)) * token_used) + 960);
}

/* xon threshold is (1 + 1/8) xoff threshold */
static inline uint16_t drv_xpm_xon_thld(int token_size)
{
    uint16_t xoff_thld = drv_xpm_xoff_thld(token_size);
    return (xoff_thld + (xoff_thld >> 3));
}
#endif
static inline int32_t drv_xmp_is_valid(void *virt_ptr)
{
    return ((uintptr_t)virt_ptr >= (uintptr_t)xpm_common_cfg.virt_base) && ((uintptr_t)virt_ptr <= (uintptr_t)xpm_common_cfg.virt_end);
}


/* common init/exit API for FPM and MPM driver to call */
void drv_xpm_common_init(void *virt_base, uintptr_t phys_base, unsigned int xpm_buf_size, unsigned int num_of_token);
void drv_xpm_common_exit(void);
int drv_xpm_common_update_pool_size(int *pool_size_array);

void update_rdp_fpm_resources(uint32_t pool_memory_size, uint32_t buf_size, uint32_t hw_supported_total_tokens, uint32_t configured_total_tokens, int *pool_size);

/* common external APIs */
void drv_xpm_copy_from_host_buffer(void *data, uint32_t xpm_bn, uint32_t packet_len, uint16_t offset);
bdmf_error_t drv_xpm_alloc_buffer(uint32_t packet_len, uint32_t *buff_num, uint8_t *pool_num);

/* buffer_id <-> virt/phys address translation APIs */
void *drv_xpm_buffer_id_to_virt(uint32_t xpm_bn);
uintptr_t drv_xpm_buffer_id_to_phys(uint32_t xpm_bn);
int32_t drv_xpm_buffer_phys_to_id(uintptr_t phys_addr);


int drv_xpm_ddr_token_info_set(uint32_t token_idx, ddr_token_info_t *info);
extern ddr_token_info_t *ddr_token_info;
/* return the token info in case of error returns NULL */
static inline ddr_token_info_t *drv_xpm_ddr_token_info_get(uint32_t token_id)
{
    if (unlikely(token_id >= TOTAL_FPM_TOKENS))
        return NULL;
    return &ddr_token_info[token_id];
}
int drv_xpm_ddr_token_info_clear(uint32_t token_idx);


/* threshold / buffer usage APIs */
int drv_xpm_check_threshold(uint32_t packet_len, int prio);
static inline void lookup_num_of_tokens_by_pool_id(uint8_t *num_of_tokens, uint8_t pool_id)
{
    *num_of_tokens = xpm_common_cfg.pool_size[pool_id];
}
void rdp_drv_xpm_bufmgmt_db_reset(void);
int calculate_total_available_dynamic_fpm_tokens(void);
static inline int32_t drv_xpm_buffer_virt_to_id(void *virt_ptr)
{
    int32_t buffer_id;

    if ((!drv_xmp_is_valid(virt_ptr)))
        return -1;

    buffer_id = ((uintptr_t)virt_ptr - (uintptr_t)xpm_common_cfg.virt_base);

    return (buffer_id >> xpm_common_cfg.buf_size_log2);
}
static inline int32_t drv_xpm_buffer_virt_to_id_ex(void *virt_ptr, void *data_ptr, int32_t *offset)
{
    int32_t buffer_id = drv_xpm_buffer_virt_to_id(virt_ptr);

    if ((buffer_id >= 0)) 
    {
        int32_t related_address = ((uintptr_t)data_ptr - (uintptr_t)xpm_common_cfg.virt_base);
        *offset = (related_address - ((buffer_id) << xpm_common_cfg.buf_size_log2));
    }

    return buffer_id;
}
#if defined(CONFIG_CPU_RX_FROM_XPM) || defined(CONFIG_CPU_TX_FROM_XPM)
static inline int drv_xpm_buffer_update_cpu_free_cnt(uint32_t token_idx, int32_t bufmng_cnt_id, uint32_t pool_id)
{
    uint8_t num_of_tokens;
    uint64_t num_of_tokens_64;

    BUG_ON(bufmng_cnt_id >= FPM_MAX_NUM_OF_CPU_BUFMGT_GROUPS);

    lookup_num_of_tokens_by_pool_id(&num_of_tokens, pool_id);
    num_of_tokens_64 = (uint64_t)num_of_tokens;
    
    ATOMIC_LEVEL64_ADD(num_of_tokens_64, &bufmng_free_req_cnt[bufmng_cnt_id]);
     
    bdmf_dcache_flush((unsigned long)&bufmng_free_req_cnt[bufmng_cnt_id], sizeof(bufmng_free_req_cnt[bufmng_cnt_id]));
    /* TODO: check if needed dma_wmb(); */
    return 0;
}
#endif
uint32_t drv_xrdp_get_num_of_tokens(void);
#ifdef __cplusplus
}
#endif

#endif
