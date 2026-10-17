/*
   Copyright (c) 2024 Broadcom 
   All Rights Reserved
   <:label-BRCM:2024:DUAL/GPL:standard
   
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

#ifndef _wlan_rsvmem_h_
#define _wlan_rsvmem_h_

/*
 * Exported function declarations
 */
/* Allocate contiguous memory from WiFi/System CMA Reserved memory */
void *wlan_rsvmem_alloc(void *ctxt, int unit, uint size, dma_addr_t *pap);

/* Free contiguous memory from WiFi/System CMA Reserved memory */
int wlan_rsvmem_free(void *ctxt, int unit, uint size, void * va,
	dma_addr_t pa);

static inline void*
wlan_cma_rsvmem_alloc(osl_t *osh, uint size, dma_addr_t *pap)
{
	return wlan_rsvmem_alloc(NULL, osl_unit(osh), size, pap);
}

static inline int
wlan_cma_rsvmem_free(osl_t *osh, uint size, void * va, dma_addr_t pa)
{
	return wlan_rsvmem_free(NULL, osl_unit(osh), size, va, pa);
}
#endif /* _wlan_rsvmem_h_ */
