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

#ifdef RUNNER_MPM_SUPPORT

#include "rdp_drv_xpm.h"
#ifndef RDP_SIM
#include "mpm.h"
#endif

static bdmf_fastlock mpm_alloc_lock;
static bdmf_fastlock mpm_free_lock;

#if !defined(RDP_SIM)
#define RDP_MPM_ALLOC_RING_SIZE_LOG2  5 // 32
#define RDP_MPM_FREE_RING_SIZE_LOG2   5 // 32
static mpm_ring_index_t mpm_alloc_ring_index = 0;
static mpm_ring_index_t mpm_free_ring_index = 0;
#endif


void drv_mpm_init(void *virt_base, uintptr_t phys_base, unsigned int buf_size,
                  uint32_t pool_memory_size, uint32_t num_of_token)
{
#ifndef RDP_SIM
    int rc;
#endif

    drv_xpm_common_init(virt_base, phys_base, buf_size, num_of_token);
    update_rdp_fpm_resources(pool_memory_size, buf_size,
                             TOTAL_FPM_TOKENS, num_of_token, xpm_common_cfg.pool_size);
    configured_total_fpm_tokens = num_of_token;

#ifndef RDP_SIM
    /* For MPM support running in Linux, we need to create alloc and free ring */
    rc = mpm_alloc_ring_alloc(MPM_BUF_MODE_FKB, RDP_MPM_ALLOC_RING_SIZE_LOG2,
                              &mpm_alloc_ring_index);
    if (rc)
    {
        bdmf_trace("fail to alloc MPM alloc ring\n");
        return;
    }

    rc = mpm_free_ring_alloc(RDP_MPM_FREE_RING_SIZE_LOG2, &mpm_free_ring_index);
    if (rc)
    {
        /* TODO! there is no API to deallocate a previously allocated ring.
         * Do it here when there is such API */
        bdmf_trace("fail to alloc MPM free ring\n");
        return;
    }

    bdmf_trace("\talloc_ring#%d, free_ring#%d\n", mpm_alloc_ring_index,
               mpm_free_ring_index);
#endif

    bdmf_fastlock_init(&mpm_alloc_lock);
    bdmf_fastlock_init(&mpm_free_lock);
}

bdmf_error_t drv_mpm_alloc_buffer(uint32_t packet_len, uint32_t *buff_num,
                                  uint8_t no_lock)
{
#ifndef RDP_SIM
#define RX_POST_INIT_WAIT_CYCLES 200 // 1 cycle = 10usec

    uint8_t *buff_ptr;
    int wait_cycles;
    wait_cycles = RX_POST_INIT_WAIT_CYCLES;

    if (unlikely(no_lock))
    {
        buff_ptr = (uint8_t *)mpm_alloc_fkb(mpm_alloc_ring_index);
        if (buff_ptr == NULL)
            return BDMF_ERR_NOMEM;

        *buff_num = mpm_virt_to_idx(buff_ptr);
        if (*buff_num == 0xffffffff)
        {
            mpm_free_fkb(mpm_free_ring_index, (FkBuff_t *)buff_ptr);
            return BDMF_ERR_NOMEM;
        }
        return 0;
    }

    do {
    /* we assume all the packet_len should be handled/covered by PDATA type */
        bdmf_fastlock_lock(&mpm_alloc_lock);
        buff_ptr = (uint8_t *)mpm_alloc_fkb(mpm_alloc_ring_index);
        bdmf_fastlock_unlock(&mpm_alloc_lock);
        if (buff_ptr) {
            break;
        }
        udelay(10);
    } while (--wait_cycles);

    if (wait_cycles != RX_POST_INIT_WAIT_CYCLES) {
        bdmf_print("ERROR: %s: wait_cycles %d\n", __FUNCTION__, wait_cycles);
    }

    if (buff_ptr == NULL)
        return BDMF_ERR_NOMEM;

    *buff_num = mpm_virt_to_idx(buff_ptr);
    if (*buff_num == 0xffffffff)
    {
        bdmf_fastlock_lock(&mpm_free_lock);
        mpm_free_fkb(mpm_free_ring_index, (FkBuff_t *)buff_ptr);
        bdmf_fastlock_unlock(&mpm_free_lock);
        return BDMF_ERR_NOMEM;
    }

    return 0;
#else
    bdmf_error_t rc;
    if (likely(no_lock == 0))
        bdmf_fastlock_lock(&mpm_alloc_lock);
    rc = rdp_cpu_fpm_alloc(packet_len, buff_num, xpm_common_cfg.buf_size);
    if (likely(no_lock == 0))
        bdmf_fastlock_unlock(&mpm_alloc_lock);
    return rc;
#endif
}

bdmf_error_t drv_mpm_free_buffer(uint32_t packet_len, uint32_t buff_num, void *info_in)
{
    uint32_t token_idx = buff_num & FPM_INDX_MASK;
#ifndef RDP_SIM
    uint8_t *buff_ptr;
#endif
#if defined(CONFIG_CPU_RX_FROM_XPM) || defined(CONFIG_CPU_TX_FROM_XPM)
    ddr_token_info_t *info;
#endif

#if defined(CONFIG_CPU_RX_FROM_XPM) || defined(CONFIG_CPU_TX_FROM_XPM)
    if (!info_in)
        info = drv_xpm_ddr_token_info_get(token_idx);
    else
        info = (ddr_token_info_t *)info_in;

    if (info->bufmng_cnt_id != BDMF_INDEX_UNASSIGNED)
        drv_xpm_buffer_update_cpu_free_cnt(token_idx, info->bufmng_cnt_id, info->pool_id);
#endif

    drv_xpm_ddr_token_info_clear(token_idx);

    bdmf_fastlock_lock(&mpm_free_lock);
#ifndef RDP_SIM
    buff_ptr = mpm_idx_to_virt(buff_num);

    mpm_free_fkb(mpm_free_ring_index, (FkBuff_t *)buff_ptr);
#else
    rdp_cpu_fpm_free(buff_num & FPM_INDX_MASK);
#endif
    bdmf_fastlock_unlock(&mpm_free_lock);


    return 0;
}

bdmf_error_t drv_mpm_check_xoff(uint32_t num_of_token)
{
    /* TODO! implement me. check xoff threshold */
#ifndef RDP_SIM
#endif

    return 0;
}

int drv_fpm_data_path_mpm_init(dpi_params_t *p_dpi_cfg)
{
    bdmf_error_t rc;
    int token_total;

#ifndef RDP_SIM
    int token_size, pool_size[4], temp;

    rc = mpm_ebuf_info_get(&token_size, &token_total, pool_size);
    if (rc) 
        goto exit;
    p_dpi_cfg->fpm_buf_size = token_size;
#else
    token_total = TOTAL_FPM_TOKENS;
    rc = 0;
#endif
    drv_mpm_init(p_dpi_cfg->rdp_ddr_pkt_base_virt,
                 p_dpi_cfg->rdp_ddr_pkt_base_phys, p_dpi_cfg->fpm_buf_size,
                 p_dpi_cfg->fpm_pool_memory_size, token_total);

#ifndef RDP_SIM
    mpm_runner_xon_xoff_thld_set(drv_xpm_xon_thld(p_dpi_cfg->fpm_buf_size),
                                 drv_xpm_xoff_thld(p_dpi_cfg->fpm_buf_size));

    /* pool_size array from mpm is in ascending order, but in XRDP it has to be
     * descending order */
    temp = pool_size[3];
    pool_size[3] = pool_size[0];
    pool_size[0] = temp;
    temp = pool_size[2];
    pool_size[2] = pool_size[1];
    pool_size[1] = temp;
    drv_xpm_common_update_pool_size(pool_size);
    update_rdp_fpm_resources(p_dpi_cfg->fpm_pool_memory_size, p_dpi_cfg->fpm_buf_size,
                                 TOTAL_FPM_TOKENS, token_total, pool_size);

exit:
#endif
    return rc;
}

#endif /* RUNNER_MPM_SUPPORT */
