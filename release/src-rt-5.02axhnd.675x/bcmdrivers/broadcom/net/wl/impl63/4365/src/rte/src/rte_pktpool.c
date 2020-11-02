/*
 * RTE support for pktpool
 *
 * Copyright 2020 Broadcom
 *
 * This program is the proprietary software of Broadcom and/or
 * its licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 * WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 * THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof, and to
 * use this information only in connection with your use of Broadcom
 * integrated circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
 * OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
 * SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
 * IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 * IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
 * ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
 * OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
 * NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * $Id: rte_pktpool.c 696791 2017-04-28 07:03:44Z $
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <hnd_pktpool.h>
#include <rte_cons.h>
#include "rte_heap_priv.h"
#include "rte_pktpool_priv.h"

#ifdef BCMPKTPOOL
#if defined(RTE_CONS) && defined(BCMDBG_POOL)
static void
hnd_pool_dump(void *arg, int argc, char *argv[])
{
	pktpool_dbg_dump(pktpool_shared);
#ifdef BCMFRAGPOOL
	pktpool_dbg_dump(pktpool_shared_lfrag);
#endif /* BCMFRAGPOOL */
#ifdef BCMRXFRAGPOOL
	pktpool_dbg_dump(pktpool_shared_rxlfrag);
#endif /* BCMRXFRAGPOOL */
}

static void
hnd_pool_notify(void *arg, int argc, char *argv[])
{
	pktpool_dbg_notify(pktpool_shared);
#ifdef BCMFRAGPOOL
	pktpool_dbg_notify(pktpool_shared_lfrag);
#endif /* BCMFRAGPOOL */
#ifdef BCMRXFRAGPOOL
	pktpool_dbg_notify(pktpool_shared_rxlfrag);
#endif /* BCMRXFRAGPOOL */
}
#endif /* RTE_CONS && BCMDBG_POOL */

#define KB(bytes)	(((bytes) + 1023) / 1024)

#if defined(RTE_CONS) && !defined(BCM_BOOTLOADER)
static void
hnd_print_pooluse(void *arg, int argc, char *argv[])
{
	int tot_plen;
	int tot_overhead;
	int n, m;
	int plen = 0;
	int overhead = 0;
	uint inuse_size, inuse_overhead;

	tot_plen = 0;
	tot_overhead = 0;

	hnd_meminuse(&inuse_size, &inuse_overhead);

	if (POOL_ENAB(pktpool_shared)) {

		n = pktpool_len(pktpool_shared);
		m = pktpool_avail(pktpool_shared);
		plen = pktpool_plen(pktpool_shared) * n;
		overhead = (pktpool_plen(pktpool_shared) + LBUFSZ) * n;

		tot_plen = tot_plen + plen;
		tot_overhead = tot_overhead + overhead;

		printf("\tIn use pool %d(%d): %d(%dK), w/oh: %d(%dK)\n",
		       n, m,
		       plen, KB(plen),
		       overhead, KB(overhead));
#if !defined(BCMFRAGPOOL) && !defined(BCMRXFRAGPOOL)
		printf("\tIn use - pool: %d(%dK), w/oh: %d(%dK)\n",
		       inuse_size - tot_plen, KB(inuse_size - tot_plen),
		       (inuse_size + inuse_overhead) - tot_overhead,
		       KB((inuse_size + inuse_overhead) - tot_overhead));
#endif // endif
	}
#ifdef BCMFRAGPOOL
	if (POOL_ENAB(pktpool_shared_lfrag)) {
		int n, m;
		int plen_lf, overhead_lf;

		n = pktpool_len(pktpool_shared_lfrag);
		m = pktpool_avail(pktpool_shared_lfrag);
		plen_lf = pktpool_plen(pktpool_shared_lfrag) * n;
		overhead_lf = (pktpool_plen(pktpool_shared_lfrag) + LBUFFRAGSZ) * n;

		tot_plen = tot_plen + plen_lf;
		tot_overhead = tot_overhead + overhead_lf;
		printf("\tIn use Frag pool %d(%d): %d(%dK), w/oh: %d(%dK)\n",
		       n, m,
		       plen_lf, KB(plen_lf),
		       overhead_lf, KB(overhead_lf));
#ifndef BCMRXFRAGPOOL
		printf("\tIn use - pools : %d(%dK), w/oh: %d(%dK)\n",
		       inuse_size - (tot_plen),
			KB(inuse_size - (tot_plen)),
		       (inuse_size + inuse_overhead) - (tot_overhead),
		       KB((inuse_size + inuse_overhead) - (tot_overhead)));
#endif // endif
	}

#ifdef BCM_DHDHDR
	if (BCMDHDHDR_ENAB() && d3_lfrag_buf_pool->inited) {
		int n, m;
		int buflen_lf, overhead_lf;

		/* D3_BUFFER */
		n = d3_lfrag_buf_pool->len;
		m = d3_lfrag_buf_pool->avail;
		buflen_lf = d3_lfrag_buf_pool->buflen * n;
		overhead_lf = (d3_lfrag_buf_pool->buflen + LFBUFSZ) * n;
		printf("\tIn use D3_BUF pool %d(%d): %d(%dK), w/oh: %d(%dK)\n",
			n, m,
			buflen_lf, KB(buflen_lf),
			overhead_lf, KB(overhead_lf));

		/* D11_BUFFER */
		n = d11_lfrag_buf_pool->len;
		m = d11_lfrag_buf_pool->avail;
		buflen_lf = d11_lfrag_buf_pool->buflen * n;
		overhead_lf = (d11_lfrag_buf_pool->buflen + LFBUFSZ) * n;
		printf("\tIn use D11_BUF pool %d(%d): %d(%dK), w/oh: %d(%dK)\n",
			n, m,
			buflen_lf, KB(buflen_lf),
			overhead_lf, KB(overhead_lf));

#ifdef HOST_HDR_FETCH
		/* Allocate D11_EXT_BUFFER pool only if htxhdr is enabled at boot. */
		if (HOST_HDR_FETCH_ENAB()) {
			/* D11_EXT_BUFFER */
			n = d11_ext_lfrag_buf_pool->len;
			m = d11_ext_lfrag_buf_pool->avail;
			buflen_lf = d11_ext_lfrag_buf_pool->buflen * n;
			overhead_lf = (d11_ext_lfrag_buf_pool->buflen + LFBUFSZ) * n;
			printf("\tIn use D11_EXT_BUF pool %d(%d): %d(%dK), w/oh: %d(%dK)\n",
				n, m,
				buflen_lf, KB(buflen_lf),
				overhead_lf, KB(overhead_lf));
		}
#endif /* HOST_HDR_FETCH */
	}
#endif /* BCM_DHDHDR */
#endif /* BCMFRAGPOOL */

#ifdef BCMRXFRAGPOOL
	if (POOL_ENAB(pktpool_shared_rxlfrag)) {
		int n, m;
		int plen_rxlf, overhead_rxlf;

		n = pktpool_len(pktpool_shared_rxlfrag);
		m = pktpool_avail(pktpool_shared_rxlfrag);
		plen_rxlf = pktpool_plen(pktpool_shared_rxlfrag) * n;
		overhead_rxlf = (pktpool_plen(pktpool_shared_rxlfrag) + LBUFFRAGSZ) * n;

		tot_plen = tot_plen + plen_rxlf;
		tot_overhead = tot_overhead + overhead_rxlf;

		printf("\tIn use RX Frag pool %d(%d): %d(%dK), w/oh: %d(%dK)\n",
		       n, m,
		       plen_rxlf, KB(plen_rxlf),
		       overhead_rxlf, KB(overhead_rxlf));
		printf("\tIn use - pools : %d(%dK), w/oh: %d(%dK)\n",
		       inuse_size - (tot_plen),
			KB(inuse_size - (tot_plen)),
		       (inuse_size + inuse_overhead) - (tot_overhead),
		       KB((inuse_size + inuse_overhead) - (tot_overhead)));
	}
#endif /* BCMRXFRAGPOOL */
}
#endif /* RTE_CONS && !BCM_BOOTLOADER */

void
BCMATTACHFN(rte_pktpool_init)(osl_t *osh)
{
	hnd_pktpool_init(osh);

#if defined(RTE_CONS) && !defined(BCM_BOOTLOADER)
#ifdef BCMDBG_POOL
	hnd_cons_add_cmd("d", hnd_pool_notify, 0);
	hnd_cons_add_cmd("p", hnd_pool_dump, 0);
#endif // endif
	hnd_cons_add_cmd("pu", hnd_print_pooluse, 0);
#endif /* RTE_CONS && !BCM_BOOTLOADER */
}
#endif /* BCMPKTPOOL */
