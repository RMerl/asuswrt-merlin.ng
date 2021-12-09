/*
    Copyright (c) 2017 Broadcom
    All Rights Reserved

    <:label-BRCM:2017:DUAL/GPL:standard

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
