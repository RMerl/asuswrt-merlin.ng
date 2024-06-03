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

#ifndef DRV_MPM_H_INCLUDED
#define DRV_MPM_H_INCLUDED

#ifdef __cplusplus
extern "C"
{
#endif


void drv_mpm_init(void *virt_base, uintptr_t phys_base, unsigned int buf_size, uint32_t pool_memory_size, uint32_t num_of_token);

bdmf_error_t drv_mpm_alloc_buffer(uint32_t packet_len, uint32_t *buff_num);
bdmf_error_t drv_mpm_free_buffer(uint32_t packet_len, uint32_t buff_num, void *info);
bdmf_error_t drv_mpm_check_xoff(uint32_t num_of_token);

#ifdef __cplusplus
}
#endif

#endif
