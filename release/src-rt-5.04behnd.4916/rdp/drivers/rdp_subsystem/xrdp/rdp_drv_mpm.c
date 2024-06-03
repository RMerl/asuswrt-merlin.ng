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
}

bdmf_error_t drv_mpm_alloc_buffer(uint32_t packet_len, uint32_t *buff_num)
{
#ifndef RDP_SIM
#define RX_POST_INIT_WAIT_CYCLES 200 // 1 cycle = 10usec

    uint8_t *buff_ptr;
    int wait_cycles;

    wait_cycles = RX_POST_INIT_WAIT_CYCLES;
    do {
        /* we assume all the packet_len should be handled/covered by PDATA type */
        buff_ptr = (uint8_t *)mpm_alloc_fkb(mpm_alloc_ring_index);
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
        mpm_free_fkb(mpm_free_ring_index, (FkBuff_t *)buff_ptr);
        return BDMF_ERR_NOMEM;
    }

    return 0;
#else
    return rdp_cpu_fpm_alloc(packet_len, buff_num, xpm_common_cfg.buf_size);
#endif
}

bdmf_error_t drv_mpm_free_buffer(uint32_t packet_len, uint32_t buff_num, void *info)
{
#ifndef RDP_SIM
    uint8_t *buff_ptr;

    buff_ptr = mpm_idx_to_virt(buff_num);
    mpm_free_fkb(mpm_free_ring_index, (FkBuff_t *)buff_ptr);

    return 0;
#else
    rdp_cpu_fpm_free(buff_num & FPM_INDX_MASK);
    return 0;
#endif
}

bdmf_error_t drv_mpm_check_xoff(uint32_t num_of_token)
{
    /* TODO! implement me. check xoff threshold */
#ifndef RDP_SIM
#endif

    return 0;
}

#endif /* RUNNER_MPM_SUPPORT */
