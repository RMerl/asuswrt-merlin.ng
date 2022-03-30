/*
    Copyright (c) 2017 Broadcom
    All Rights Reserved

    <:label-BRCM:2017:DUAL/GPL:standard
    
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

#if defined(DSLCPE) && defined(CTFPOOL)

#include <typedefs.h>
#include <osl.h>

#include <linux/skbuff.h>
#include <dngl_stats.h>
#include <dhd.h>
#include <dhd_flowring.h>

#if !defined(HNDCTF)
#define RXBUFPOOLSZ             4096
#define RXBUFSZ                 DHD_FLOWRING_RX_BUFPOST_PKTSZ /* packet data buffer size */
#endif

int allocskbmode = 1;   /* pre-allocated buffer mode is enabled by default */
module_param(allocskbmode, int, 0);
uint allocskbsz = 4096; /* RXBUFPOOLSZ */
module_param(allocskbsz, uint, 0);

int dhd_prealloc_attach(void *p)
{
	int ret = BCME_OK;
        dhd_pub_t *dhdp;
        dhdp = (dhd_pub_t *)p;

#if !defined(HNDCTF) && defined(CTFPOOL)
	/* use large ctf poolsz for platforms with more memory */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0))
	if (totalram_pages >= 65535) {
#else
	if (num_physpages >= 65535) {
#endif
	        allocskbsz = RXBUFPOOLSZ * 2;
	}
#if defined(BCM_DHD_RUNNER)
	/* Disable CTFPOOL as runner will manage the rx buffers */
	if (DHD_RNR_OFFL_RXCMPL(dhdp)) {
		allocskbmode = 0;
		allocskbsz = 0;
	}
#endif /* BCM_DHD_RUNNER */
	if (allocskbsz && (osl_ctfpool_init(dhdp->osh, allocskbsz, RXBUFSZ + BCMEXTRAHDROOM) < 0)) {
		printk("%s: osl_ctfpool_init() failed\n", __FUNCTION__);
		ret = BCME_ERROR;
	}
	printk("%s: pre-allocated buffer mode is %s (allocskbsz=%d)\n", __FUNCTION__, 
		(allocskbmode == 0) ? "disabled" : "enabled", allocskbsz);
#endif /* !HNDCTF && CTFPOOL */

	return ret;
}

int dhd_prealloc_detach(void *p)
{
        dhd_pub_t *dhdp;
        dhdp = (dhd_pub_t *)p;

#if !defined(HNDCTF) && defined(CTFPOOL)
	/* free the buffers in fast pool */
	osl_ctfpool_cleanup(dhdp->osh);
#endif /* !HNDCTF && CTFPOOL */

	return BCME_OK;
}

#endif /* DSLCPE && CTFPOOL */
