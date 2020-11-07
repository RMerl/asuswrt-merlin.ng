/*
    <:copyright-BRCM:2015:DUAL/GPL:standard
    
       Copyright (c) 2015 Broadcom 
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

#include "rdpa_gpl_fpm.h"
#include "rdp_common.h"
#include "rdp_subsystem_common.h"
#include "XRDP_AG.h"
#ifndef RDP_SIM
#include "bcm_mm.h"
#endif
#include "xrdp_drv_fpm_ag.h"

uint32_t fpm_ddr_address;
void *drv_fpm_buffer_get_address(uint32_t fpm_bn)
{
    return (void *)((uint8_t *)fpm_ddr_address + fpm_bn * FPM_DDR_BUFFER_SIZE);
}

bdmf_error_t drv_fpm_free_buffer(uint32_t packet_len, uint32_t buff_num)
{

    bdmf_boolean token_valid = 1;
    bdmf_boolean ddr_pool = (buff_num >> 16) & 1;
    uint32_t token_index = buff_num & 0xFFFF;
    bdmf_error_t rc = 0;

    if (packet_len <= FPM_DDR_BUFFER_SIZE)
        rc = ag_drv_fpm_pool4_alloc_dealloc_set(token_valid, ddr_pool, token_index, packet_len);
    else if (packet_len <= (FPM_DDR_BUFFER_SIZE<<1))
        rc = ag_drv_fpm_pool3_alloc_dealloc_set(token_valid, ddr_pool, token_index, packet_len);
    else if (packet_len <= (FPM_DDR_BUFFER_SIZE<<2))
        rc = ag_drv_fpm_pool2_alloc_dealloc_set(token_valid, ddr_pool, token_index, packet_len);
    else if (packet_len <= (FPM_DDR_BUFFER_SIZE<<3))
        rc = ag_drv_fpm_pool1_alloc_dealloc_set(token_valid, ddr_pool, token_index, packet_len);

    return rc;
}

bdmf_error_t drv_fpm_alloc_buffer(uint32_t packet_len, uint32_t *buff_num)
{
    bdmf_boolean token_valid = 0;
    bdmf_boolean ddr_pool = 0;
    uint32_t fpm_pool = 0;
    uint32_t token_index = 0;
    uint16_t token_size;
    bdmf_error_t rc = 0;

    if (packet_len <= FPM_DDR_BUFFER_SIZE)
    {
        rc = ag_drv_fpm_pool4_alloc_dealloc_get(&token_valid, &ddr_pool, &token_index, &token_size);
        fpm_pool = 3;
    }
    else if (packet_len <= (FPM_DDR_BUFFER_SIZE<<1))
    {
        rc = ag_drv_fpm_pool3_alloc_dealloc_get(&token_valid, &ddr_pool, &token_index, &token_size);
        fpm_pool = 2;
    }
    else if (packet_len <= (FPM_DDR_BUFFER_SIZE<<2))
    {
        rc = ag_drv_fpm_pool2_alloc_dealloc_get(&token_valid, &ddr_pool, &token_index, &token_size);
        fpm_pool = 1;
    }
    else if (packet_len <= (FPM_DDR_BUFFER_SIZE<<3))
    {
        rc = ag_drv_fpm_pool1_alloc_dealloc_get(&token_valid, &ddr_pool, &token_index, &token_size);
        fpm_pool = 0;
    }

    if (!token_valid)
        return BDMF_ERR_NOMEM;

#if defined (BCM6858)
    *buff_num = (ddr_pool << 16) | token_index;
#else
    *buff_num = (fpm_pool << 16) | token_index;
#endif

    return rc;
}


void drv_fpm_copy_from_host_buffer(void *data, uint32_t fpm_bn, uint32_t packet_len, uint16_t offset)
{
    void *fpm_buffer_ptr;

    fpm_buffer_ptr = ((uint8_t *)drv_fpm_buffer_get_address(fpm_bn)) + offset;
#ifndef XRDP_EMULATION
    MWRITE_BLK_8(fpm_buffer_ptr, data, packet_len);
#else
    MWRITE_BLK_8((uint32_t)fpm_buffer_ptr, (uint8_t *)data, packet_len);
#endif

#ifndef RDP_SIM
    /* Be sure that all memory access done before flush */
    __asm__ __volatile__ ("dsb    sy");

    _cfe_flushcache(CFE_CACHE_FLUSH_RANGE,(uint8_t *)fpm_buffer_ptr, (uint8_t *)fpm_buffer_ptr + packet_len);
#endif
}

