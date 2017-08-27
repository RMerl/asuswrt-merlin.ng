/*
 * Broadcom Dongle Host Driver (DHD), Linux-specific network interface
 *
 * Copyright (C) 2016, Broadcom. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * $Id: dhd_prealloc.c $
 */

#if defined(DSLCPE) && defined(CTFPOOL)

#include <typedefs.h>
#include <osl.h>

#include <linux/skbuff.h>
#include <dngl_stats.h>
#include <dhd.h>
#include <dhd_flowring.h>

#if !defined(HNDCTF)
#define RXBUFPOOLSZ             2048
#define RXBUFSZ                 DHD_FLOWRING_RX_BUFPOST_PKTSZ /* packet data buffer size */
#endif

int allocskbmode = 1;   /* pre-allocated buffer mode is enabled by default */
module_param(allocskbmode, int, 0);
uint allocskbsz = 2048; /* RXBUFPOOLSZ */
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
