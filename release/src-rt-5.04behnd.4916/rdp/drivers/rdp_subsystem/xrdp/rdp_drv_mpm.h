/*
    <:copyright-BRCM:2021:DUAL/GPL:standard
    
       Copyright (c) 2021 Broadcom 
       All Rights Reserved
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License, version 2, as published by
    the Free Software Foundation (the "GPL").
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    
    A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
    writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
    
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
