/*
 * Packet ID to pointer mapping source
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
 * $Id: hnd_pktid.c 690155 2017-03-14 23:38:37Z $
 */

#if defined(BCMPKTIDMAP)

#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <hnd_pktid.h>
#include <rte_cons.h>

#ifdef BCMDBG
#define PKTID_MSG(x) printf x
#else
#define PKTID_MSG(x)
#endif // endif

/* Hierarchical multiword bit map for unique 16bit id allocator */
static struct bcm_mwbmap * hnd_pktid_map = (struct bcm_mwbmap *)NULL;
static uint32 hnd_pktid_max = 0U; /* mwbmap allocator dimension */
static uint32 hnd_pktid_failure_count = 0U; /* pktid alloc failure count */

#ifdef RTE_CONS
static void hnd_print_pktid(void *arg, int argc, char *argv[]);
#endif // endif

/* Associative array of pktid to pktptr */
struct lbuf **hnd_pktptr_map = NULL;

/* Support for 32bit pktptr to 16bit pktid for memory conservation */
/*
 * Prior to constructing packet pools, a packet ID to packet PTR mapping service
 * must be initialized.
 * Instantiate a hierarchical multiword bit map for 16bit unique pktid allocator
 * Instantiate a reverse pktid to pktptr map.
 */
void
BCMATTACHFN(hnd_pktid_init)(osl_t * osh, uint32 pktids_total)
{
	uint32 mapsz;
	pktids_total += 1; /* pktid 0 is reserved */

	ASSERT(PKT_MAXIMUM_ID < 0xFFFF);
	ASSERT(pktids_total <= PKT_MAXIMUM_ID);
	ASSERT((uint16)(BCM_MWBMAP_INVALID_IDX) == PKT_INVALID_ID);

	/* Instantiate a hierarchical multiword bitmap for unique pktid allocator */
	hnd_pktid_map = bcm_mwbmap_init(osh, pktids_total);
	if (hnd_pktid_map == (struct bcm_mwbmap *)NULL) {
		ASSERT(0);
		return;
	}

	/* Instantiate a pktid to pointer associative array */
	mapsz = sizeof(struct lbuf *) * pktids_total;
	if ((hnd_pktptr_map = MALLOCZ(osh, mapsz)) == NULL) {
		ASSERT(0);
		goto error;
	}

	/* reserve pktid #0 and setup mapping of pktid#0 to NULL pktptr */
	ASSERT(PKT_NULL_ID == (uint16)0);
	bcm_mwbmap_force(hnd_pktid_map, PKT_NULL_ID);

	ASSERT(!bcm_mwbmap_isfree(hnd_pktid_map, PKT_NULL_ID));
	hnd_pktptr_map[PKT_NULL_ID] = (struct lbuf *)(NULL);

	/* pktid to pktptr mapping successfully setup, with pktid#0 reserved */
	hnd_pktid_max = pktids_total;

#ifdef RTE_CONS
	if (!hnd_cons_add_cmd("pktid", hnd_print_pktid, 0))
		goto error;
#endif // endif

	return;

error:
	bcm_mwbmap_fini(osh, hnd_pktid_map);
	hnd_pktid_max = 0U;
	hnd_pktid_map = (struct bcm_mwbmap *)NULL;
	return;
}

void /* Increment pktid alloc failure count */
hnd_pktid_inc_fail_cnt(void)
{
	hnd_pktid_failure_count++;
}

uint32 /* Fetch total number of pktid alloc failures */
hnd_pktid_fail_cnt(void)
{
	return hnd_pktid_failure_count;
}

uint32 /* Fetch total number of free pktids */
hnd_pktid_free_cnt(void)
{
	return bcm_mwbmap_free_cnt(hnd_pktid_map);
}

/* Allocate a unique pktid and associate the pktptr to it */
uint16
hnd_pktid_allocate(struct lbuf * pktptr)
{
	uint32 pktid;

	pktid = bcm_mwbmap_alloc(hnd_pktid_map); /* allocate unique id */

	ASSERT(pktid < hnd_pktid_max);
	if (pktid < hnd_pktid_max) { /* valid unique id allocated */
		ASSERT(pktid != 0U);
		/* map pktptr @ pktid */
		hnd_pktptr_map[pktid] = pktptr;
	}

	return (uint16)(pktid);
}

/* Release a previously allocated unique pktid */
void
hnd_pktid_release(struct lbuf * pktptr, const uint16 pktid)
{
	ASSERT(pktid != 0U);
	ASSERT(pktid < hnd_pktid_max);
	ASSERT(hnd_pktptr_map[pktid] == pktptr);

	hnd_pktptr_map[pktid] = (struct lbuf *)NULL; /* unmap pktptr @ pktid */
	/* BCMDBG:
	 * hnd_pktptr_map[pktid] = (struct lbuf *) (0xdead0000 | pktid);
	 */
	bcm_mwbmap_free(hnd_pktid_map, (uint32)pktid);
}

bool
hnd_pktid_sane(struct lbuf * pktptr)
{
	int insane = 0;
	uint16 pktid = 0;
	struct lbuf * lb = pktptr;

	pktid = PKTID(lb);

	insane |= pktid >= hnd_pktid_max;
	insane |= (hnd_pktptr_map[pktid] != lb);

	if (insane) {
		ASSERT(pktid < hnd_pktid_max);
		ASSERT(hnd_pktptr_map[pktid] == lb);
		PKTID_MSG(("hnd_pktid_sane pktptr<%p> pktid<%u>\n", pktptr, pktid));
	}

	return (!insane);
}

#ifdef RTE_CONS
static void
hnd_print_pktid(void *arg, int argc, char *argv[])
{
	printf("\tPktId Total: %d, Free: %d, Failed: %d\n",
		hnd_pktid_max, hnd_pktid_free_cnt(), hnd_pktid_fail_cnt());
}
#endif /* RTE_CONS */
#endif /* BCMPKTIDMAP */
