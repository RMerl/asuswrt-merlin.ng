/*
<:copyright-BRCM:2015:DUAL/GPL:standard

   Copyright (c) 2015 Broadcom 
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
#if !defined(__BCM_M2M_DMA_H__) && defined(CONFIG_BCM_KF_M2M_DMA) && defined(CONFIG_BCM_M2M_DMA)
#define __BCM_M2M_DMA_H__


extern uint32_t bcm_m2m_dma_memcpy_async_uncached(uint32_t phys_dest, uint32_t phys_src, uint16_t len);

extern uint32_t bcm_m2m_dma_memcpy_async(void *dest, void *src, uint16_t len);
extern uint32_t bcm_m2m_dma_memcpy_async_no_flush(void *dest, void *src, uint16_t len);

extern uint32_t bcm_m2m_dma_memcpy_async_no_flush_inv(void *dest, void *src, uint16_t len);
extern int bcm_m2m_wait_for_complete(uint32_t desc_id);

#endif /* __BCM_M2M_DMA_H__ */
