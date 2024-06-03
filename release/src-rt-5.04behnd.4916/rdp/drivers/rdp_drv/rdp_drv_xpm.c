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

/*****************************************************************************/
/*                                                                           */
/* Include files                                                             */
/*                                                                           */
/*****************************************************************************/

#include "rdp_drv_xpm.h"
#include "rdp_drv_qm.h"
#include "rdp_mm.h"
#ifndef RDP_SIM
#include "bcm_mm.h"
#endif
#include "rdd_data_structures_auto.h"
#include "rdp_cpu_ring.h"

#undef FLUSH_RANGE
#undef INV_RANGE

#define FLUSH_RANGE(s,l)                    bdmf_dcache_flush(s,l);
#define FLUSH_FPM_RANGE(s,l)                bdmf_fpm_dcache_flush(s,l);
#define INV_RANGE(s,l)                      bdmf_dcache_inv(s,l);

xpm_common_cfg_t xpm_common_cfg;
rdp_fpm_resources_t rdp_fpm_resources;
ddr_token_info_t *ddr_token_info;
uint32_t configured_total_fpm_tokens = TOTAL_FPM_TOKENS;
bdmf_phys_addr_t ddr_token_info_phy_addr;
extern dpi_params_t *p_dpi_cfg;

#if defined(CONFIG_CPU_RX_FROM_XPM) || defined(CONFIG_CPU_TX_FROM_XPM)
atomic64_t *bufmng_free_req_cnt;
void drv_xpm_buf_cnt_req_init_fw(void);
#endif
static ddr_token_info_t zeroed_token_info = {.is_xpm_valid = 0, 
    .bufmng_cnt_id = BDMF_INDEX_UNASSIGNED,
    .pool_id = BDMF_INDEX_UNASSIGNED};

uint32_t drv_xrdp_get_num_of_tokens(void)
{
    return xpm_common_cfg.num_of_token;
}

void drv_xpm_common_init(void *virt_base, uintptr_t phys_base, unsigned int buf_size, unsigned int num_of_token)
{
    bdmf_phys_addr_t phy_addr;
    uint32_t i;

    xpm_common_cfg.virt_base = virt_base;
    xpm_common_cfg.phys_base = phys_base;
    xpm_common_cfg.buf_size = buf_size;
    xpm_common_cfg.num_of_token = num_of_token;
    xpm_common_cfg.virt_end = (void *)((uint8_t *)virt_base + num_of_token * buf_size - 1);
    xpm_common_cfg.buf_size_log2 = 0;

    xpm_common_cfg.pool_size[0] = 8;
    xpm_common_cfg.pool_size[1] = 4;
    xpm_common_cfg.pool_size[2] = 2;
    xpm_common_cfg.pool_size[3] = 1;

    ddr_token_info = (ddr_token_info_t *)rdp_mm_aligned_alloc(sizeof(ddr_token_info_t) * num_of_token, &phy_addr);
    BUG_ON(!ddr_token_info);

    ddr_token_info_phy_addr = RDD_VIRT_TO_PHYS((uintptr_t)ddr_token_info);

    for (i = 0; i < num_of_token; i++)
    {
        drv_xpm_ddr_token_info_clear(i);
    }

    while (buf_size >>= 1)
    {
        ++xpm_common_cfg.buf_size_log2;
    }

    bdmf_trace("XPM driver: virt base 0x%px; phys base 0x%llx, buffer size %d, "
              "token_info@ virt:0x%px, phy:0x%x\n", virt_base, (unsigned long long)phys_base,
             buf_size, ddr_token_info, (uint32_t)ddr_token_info_phy_addr);
    bdmf_trace("XPM driver: buf_size_log2 %d, num_of_token %d virt_end 0x%px\n", xpm_common_cfg.buf_size_log2, num_of_token, xpm_common_cfg.virt_end);
#if defined(CONFIG_CPU_RX_FROM_XPM) || defined(CONFIG_CPU_TX_FROM_XPM)
    {
        bdmf_phys_addr_t phy_addr;

        /* First object created */
        bufmng_free_req_cnt = (atomic64_t *)rdp_mm_aligned_alloc((FPM_MAX_NUM_OF_CPU_BUFMGT_GROUPS * sizeof(atomic64_t)), &phy_addr);
        if (!bufmng_free_req_cnt)
        {
            BDMF_TRACE_ERR("Failed to allocate bufmgt array for ring\n");
            return;
        }
        memset(bufmng_free_req_cnt, 0, sizeof(atomic64_t) * FPM_MAX_NUM_OF_CPU_BUFMGT_GROUPS);
    }
#endif
}

void drv_xpm_common_exit(void)
{
    rdp_mm_aligned_free((void *)NONCACHE_TO_CACHE(ddr_token_info), sizeof(ddr_token_info_t) * xpm_common_cfg.num_of_token);

#if defined(CONFIG_CPU_RX_FROM_XPM) || defined(CONFIG_CPU_TX_FROM_XPM)
    if (bufmng_free_req_cnt)
    {
        rdp_mm_aligned_free((void *)NONCACHE_TO_CACHE(bufmng_free_req_cnt),
            FPM_MAX_NUM_OF_CPU_BUFMGT_GROUPS * sizeof(atomic64_t));
        bufmng_free_req_cnt = NULL;
    }
#endif
}

int drv_xpm_common_update_pool_size(int *pool_size_array)
{
    if (pool_size_array == NULL)
        return -1;

    /* TODO, check the pool_size_array is in descending order? */

    xpm_common_cfg.pool_size[0] = pool_size_array[0];
    xpm_common_cfg.pool_size[1] = pool_size_array[1];
    xpm_common_cfg.pool_size[2] = pool_size_array[2];
    xpm_common_cfg.pool_size[3] = pool_size_array[3];

    return 0;
}

/* buffer virt/phys address <-> ID conversion APIs */
void *drv_xpm_buffer_id_to_virt(uint32_t xpm_bn)
{
    uint32_t bn = xpm_bn & FPM_INDX_MASK;

    return (void *)((uint8_t *)xpm_common_cfg.virt_base + (bn << xpm_common_cfg.buf_size_log2));
}

bdmf_error_t drv_xpm_alloc_buffer(uint32_t packet_len, uint32_t *buff_num, uint8_t *pool_num)
{
    ddr_token_info_t info = {};
    bdmf_error_t rc = BDMF_ERR_OK;

#ifndef RUNNER_MPM_SUPPORT
    rc = drv_fpm_alloc_buffer(packet_len, buff_num);
#else
    rc = drv_mpm_alloc_buffer(packet_len, buff_num);
#endif

    info.pool_id = *buff_num >> FPM_POOL_ID_SHIFT;
    info.is_xpm_valid = 1;
    info.bufmng_cnt_id = BDMF_INDEX_UNASSIGNED;
    *buff_num &= FPM_INDX_MASK;
    if (likely(pool_num))
    {
        *pool_num = info.pool_id;
    }
    rc = rc ? rc : drv_xpm_ddr_token_info_set(*buff_num, &info);

    return rc;
}

void drv_xpm_copy_from_host_buffer(void *data, uint32_t xpm_bn, uint32_t packet_len, uint16_t offset)
{
    void *xpm_buffer_ptr;

    xpm_buffer_ptr = ((uint8_t *)drv_xpm_buffer_id_to_virt(xpm_bn)) + offset;
    MWRITE_BLK_8(xpm_buffer_ptr, data, packet_len);

#ifndef RDP_SIM
    /* Be sure that all memory access done before flush */
    __asm__ __volatile__ ("dsb    sy");

    FLUSH_FPM_RANGE((unsigned long)xpm_buffer_ptr, packet_len);
#endif
}

int drv_xpm_ddr_token_info_set(uint32_t token_idx, ddr_token_info_t *info)
{
    if (unlikely(token_idx >= TOTAL_FPM_TOKENS))
        return BDMF_ERR_RANGE;

    memcpy(&ddr_token_info[token_idx], info, sizeof(ddr_token_info_t));
    return 0;
}

int drv_xpm_ddr_token_info_clear(uint32_t token_idx)
{
    if (unlikely(token_idx >= TOTAL_FPM_TOKENS))
        return BDMF_ERR_RANGE;

    memcpy(&ddr_token_info[token_idx], &zeroed_token_info, sizeof(zeroed_token_info));

    return 0;
}

int drv_xpm_check_threshold(uint32_t packet_len, int prio)
{
    int rc;
    uint32_t num_of_token = 0;

    /* TODO! convert packet_len to num_of_token, and check the software
     * maintained threshold counter */

    rc = drv_xpm_check_xoff_plat(num_of_token);

    return rc;
}

#if CHIP_VER < RDP_GEN_60
void lookup_bbh_tx_bufsz_by_fpm_bufsz(uint32_t *fpm_bufsize, uint8_t *bbh_tx_bufsize)
{
    switch(*fpm_bufsize)
    {
    case FPM_BUF_SIZE_256:
        *bbh_tx_bufsize = BUF_256;
        break;
    case FPM_BUF_SIZE_512:
        *bbh_tx_bufsize = BUF_512;
        break;
    case FPM_BUF_SIZE_1K:
        *bbh_tx_bufsize = BUF_1K;
        break;
    case FPM_BUF_SIZE_2K:
        *bbh_tx_bufsize = BUF_2K;
        break;
    default:
        *bbh_tx_bufsize = BUF_512;
    }

    return;
}

void lookup_dma_bufsz_by_fpm_bufsz(uint32_t *fpm_bufsz, uint8_t* dma_bufsz)
{
    switch (*fpm_bufsz)
    {
    case FPM_BUF_SIZE_256:
        *dma_bufsz = DMA_BUFSIZE_256;
        break;
    case FPM_BUF_SIZE_1K:
        *dma_bufsz = DMA_BUFSIZE_1024;
        break;
    case FPM_BUF_SIZE_2K:
        *dma_bufsz = DMA_BUFSIZE_2048;
        break;
    default:
        *dma_bufsz = DMA_BUFSIZE_512;
    }
}
#else
void lookup_bbh_tx_bufsz_by_fpm_bufsz(uint32_t *fpm_bufsize, uint8_t *bbh_tx_bufsize)
{
    switch (*fpm_bufsize)
    {
    case FPM_BASIC_TOKEN_SIZE_0:
        *bbh_tx_bufsize = RDD_BUF_SIZE_0;
        break;
    case FPM_BASIC_TOKEN_SIZE_1:
        *bbh_tx_bufsize = RDD_BUF_SIZE_1;
        break;
    case FPM_BASIC_TOKEN_SIZE_2:
        *bbh_tx_bufsize = RDD_BUF_SIZE_2;
        break;
    case FPM_BASIC_TOKEN_SIZE_3:
        *bbh_tx_bufsize = RDD_BUF_SIZE_3;
        break;
    default:
        *bbh_tx_bufsize = RDD_BUF_SIZE_DEFAULT;
    }

    return;
}

void lookup_dma_bufsz_by_fpm_bufsz(uint32_t *fpm_bufsz, uint8_t* dma_bufsz)
{
    switch (*fpm_bufsz)
    {
    case FPM_BASIC_TOKEN_SIZE_0:
        /* note: DMA_BUFSIZE_256 == DMA_BUFSIZE_320 */
        *dma_bufsz = DMA_BUFSIZE_320;
        break;
    case FPM_BASIC_TOKEN_SIZE_2:
        /* note: DMA_BUFSIZE_1024 == DMA_BUFSIZE_1280 */
        *dma_bufsz = DMA_BUFSIZE_1280;
        break;
    case FPM_BASIC_TOKEN_SIZE_3:
        /* note: DMA_BUFSIZE_2048 == DMA_BUFSIZE_2560 */
        *dma_bufsz = DMA_BUFSIZE_2560;
        break;
    default:
        /* note: DMA_BUFSIZE_512 == DMA_BUFSIZE_640 */
        *dma_bufsz = DMA_BUFSIZE_640;
    }
}
#endif

int calculate_total_available_dynamic_fpm_tokens(void)
{
    int fpm_tokens_remove_for_mbr = drv_qm_get_number_of_extra_mbr();
    int total_available_tokens;
#if CHIP_VER < RDP_GEN_60
    /* we need 2 PDs for each QM queue plus 16 PDs for prefetch, for each we need 2K bytes. */
    int num_of_bytes_for_dqm_prefetch = 2 * MAX_TX_QUEUES__NUM_OF + 16 * CONST_INT_2K + (p_dpi_cfg->fpm_buf_size-1);
    int num_of_fpms_for_dqm_prefetch = num_of_bytes_for_dqm_prefetch / p_dpi_cfg->fpm_buf_size;

    /* we need 16 bytes (128 bits) for each PD. */
    int num_of_bytes_for_max_num_of_pds_in_qm = CONST_INT_64K * 16 + (p_dpi_cfg->fpm_buf_size-1);
    int num_of_fpms_for_max_num_of_pds_in_qm = num_of_bytes_for_max_num_of_pds_in_qm / p_dpi_cfg->fpm_buf_size;
    int num_of_fpms_for_qm = num_of_fpms_for_dqm_prefetch + num_of_fpms_for_max_num_of_pds_in_qm;
    int total_dynamic_fpm = configured_total_fpm_tokens - num_of_fpms_for_qm;
    total_available_tokens = total_dynamic_fpm - drv_fpm_dqm_extra_fpm_tokens_get() - fpm_tokens_remove_for_mbr;
#else

#if CHIP_VER < RDP_GEN_62
#ifdef RUNNER_MPM_SUPPORT
    uint16_t xon_thld = drv_xpm_xon_thld(p_dpi_cfg->fpm_buf_size);
#else
    uint16_t xon_thld = 0, xoff_thld = 0;

    ag_drv_fpm_pool1_xon_xoff_cfg_get(&xon_thld, &xoff_thld);
    xon_thld = xon_thld * 2;
#endif

    total_available_tokens = configured_total_fpm_tokens - xon_thld - fpm_tokens_remove_for_mbr;
#else /* #if CHIP_VER < RDP_GEN_62 */
    total_available_tokens = configured_total_fpm_tokens - TOTAL_RESERVED_FPM_TOKENS - fpm_tokens_remove_for_mbr;
#endif

#endif

    return total_available_tokens;
}

void update_rdp_fpm_resources(uint32_t pool_memory_size, uint32_t buf_size,
                              uint32_t hw_supported_total_tokens,
                              uint32_t configured_total_tokens,
                              int *pool_size)
{
    rdp_fpm_resources.fpm_pool_memory_size = pool_memory_size;
    rdp_fpm_resources.fpm_buf_size = buf_size;
    rdp_fpm_resources.hardware_supported_total_fpm_tokens = hw_supported_total_tokens;
    rdp_fpm_resources.configured_total_fpm_tokens = configured_total_tokens;
    rdp_fpm_resources.fpm_pool_configuration = (pool_size[3] << 24) | (pool_size[2] << 16) | (pool_size[1] << 8) | pool_size[0];
}

const rdp_fpm_resources_t *get_rdp_fpm_resources(void)
{
    return &rdp_fpm_resources;
}

#if defined(CONFIG_CPU_RX_FROM_XPM) || defined(CONFIG_CPU_TX_FROM_XPM)

void drv_xpm_buf_cnt_req_init_fw()
{
    uint32_t i, addr_l, addr_h;

    BUG_ON(!bufmng_free_req_cnt);

    __rdp_prepare_ptr_address((void *)(bufmng_free_req_cnt), &addr_l, &addr_h);

    RDD_FPM_RING_BUFMNG_CNT_FREE_CNT_REQ_ADDR_LOW_WRITE_G(addr_l, RDD_FPM_RING_BUFMGT_CNT_TABLE_ADDRESS_ARR, 0);
    RDD_FPM_RING_BUFMNG_CNT_FREE_CNT_REQ_ADDR_HIGH_WRITE_G(addr_h, RDD_FPM_RING_BUFMGT_CNT_TABLE_ADDRESS_ARR, 0);


    for (i = 0; i < RDD_FPM_RING_BUFMNG_CNT_FREE_CNT_REQ_NUMBER; i++)
    {
        RDD_FPM_RING_BUFMNG_CNT_FREE_CNT_REQ_WRITE_G(0, RDD_FPM_RING_BUFMGT_CNT_TABLE_ADDRESS_ARR, 0, i);
        RDD_FPM_RING_BUFMNG_CNT_FREE_CNT_DONE_WRITE_G(0, RDD_FPM_RING_BUFMGT_CNT_TABLE_ADDRESS_ARR, 0, i);
    }

}
#endif

void rdp_drv_xpm_bufmgmt_db_reset(void)
{
#if defined(CONFIG_CPU_RX_FROM_XPM) || defined(CONFIG_CPU_TX_FROM_XPM)
    int i;
    uint8_t init_value = (uint8_t)BDMF_INDEX_UNASSIGNED;

    drv_xpm_buf_cnt_req_init_fw(); /*init FPM address table in SRAM */
    for (i = 0; i < BUF_MNG_CPU_VPORTS_LOW_HIGH_PRIO_NUM; i++)
    {
        RDD_BUFMNG_DESCRIPTOR_ENTRY_BUFMNG_DROP_CNTR_IDX_WRITE_G(init_value, RDD_BUFMNG_DESCRIPTOR_TABLE_ADDRESS_ARR, i);
        RDD_BUFMNG_DESCRIPTOR_ENTRY_BUFMNG_IDX_WRITE_G(init_value, RDD_BUFMNG_DESCRIPTOR_TABLE_ADDRESS_ARR, i);
    }
#endif
}

