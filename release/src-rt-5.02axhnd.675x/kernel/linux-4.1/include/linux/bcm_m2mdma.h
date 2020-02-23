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
#if !defined(__BCM_M2M_DMA_H__) && defined(CONFIG_BCM_KF_M2M_DMA) && defined(CONFIG_BCM_M2M_DMA)
#define __BCM_M2M_DMA_H__


extern uint32_t bcm_m2m_dma_memcpy_async_uncached(uint32_t phys_dest, uint32_t phys_src, uint16_t len);

extern uint32_t bcm_m2m_dma_memcpy_async(void *dest, void *src, uint16_t len);
extern uint32_t bcm_m2m_dma_memcpy_async_no_flush(void *dest, void *src, uint16_t len);

extern uint32_t bcm_m2m_dma_memcpy_async_no_flush_inv(void *dest, void *src, uint16_t len);
extern int bcm_m2m_wait_for_complete(uint32_t desc_id);

#endif /* __BCM_M2M_DMA_H__ */
