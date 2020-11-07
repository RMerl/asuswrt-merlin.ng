/*
 * HND RTE OS Abstraction Layer
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
 * $Id: rte_osl.c 763814 2018-05-22 09:11:06Z $
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#ifdef SBPCI
#include <hndpci.h>
#endif /* SBPCI */
#include <rte_heap.h>
#include <rte_trap.h>
#include <rte.h>

#if defined(BCMDBG_MEM) || defined(BCMDBG_MEMFAIL)
static void *osl_malloc_persist_ra(uint size, uint32 constraint, void *call_site);
static void *osl_malloc_ra(uint size, void *call_site);
#endif // endif

osl_t *
#ifdef SHARED_OSL_CMN
BCMATTACHFN(osl_attach)(void *dev, void **osl_cmn)
#else
BCMATTACHFN(osl_attach)(void *dev)
#endif /* SHARED_OSL_CMN */
{
#ifndef SHARED_OSL_CMN
	void **osl_cmn = NULL;
#endif // endif
	osl_t *osh;

	osh = (osl_t *)hnd_malloc(sizeof(osl_t));
	ASSERT(osh);
	bzero(osh, sizeof(osl_t));

	if (osl_cmn == NULL || *osl_cmn == NULL) {
		osh->cmn = (osl_cmn_t *)hnd_malloc(sizeof(osl_cmn_t));
		bzero(osh->cmn, sizeof(osl_cmn_t));
		if (osl_cmn)
			*osl_cmn = osh->cmn;
	} else
		if (osl_cmn)
			osh->cmn = *osl_cmn;

	osh->cmn->refcount++;

	ASSERT(osh->cmn);

	osh->dev = dev;
	return osh;
}

void
BCMATTACHFN(osl_detach)(osl_t *osh)
{
	if (osh == NULL)
		return;
	osh->cmn->refcount--;
	hnd_free(osh);
}

/* Mechanism to force persistent allocations */
static bool force_nopersist = FALSE;

void
osl_malloc_set_nopersist(void)
{
	force_nopersist = TRUE;
}

void
osl_malloc_clear_nopersist(void)
{
	force_nopersist = FALSE;
}

#if defined(BCMDBG_MEM) || defined(BCMDBG_MEMFAIL)
void *
osl_malloc(uint size)
{
	void * ptr;

	ptr = osl_malloc_ra(size, CALL_SITE);

	if (ptr != NULL) {
		bzero(ptr, size);
	}

	return ptr;
}

static void *
osl_malloc_ra(uint size, void *call_site)
{
	void *p;

	if (ATTACH_PART_RECLAIMED() || force_nopersist)
		p = hnd_malloc_align(size, 0, call_site);
	else
		p = hnd_malloc_persist_align(size, 0, call_site);

	return p;
}

void *
osl_mallocz(uint size)
{
	void * ptr;

	ptr = osl_malloc_ra(size, CALL_SITE);

	if (ptr != NULL) {
		bzero(ptr, size);
	}

	return ptr;
}

void *
osl_malloc_align(uint size, uint align_bits, void *call_site)
{
	void *p;

	if (call_site == NULL)
		call_site = CALL_SITE;

	if (ATTACH_PART_RECLAIMED() || force_nopersist)
		p = hnd_malloc_align(size, align_bits, call_site);
	else
		p = hnd_malloc_persist_align(size, align_bits, call_site);

	return p;
}

void *
osl_mallocz_nopersist(uint size)
{
	void *ptr;

	ptr = hnd_malloc_align(size, 0, CALL_SITE);

	if (ptr != NULL) {
		bzero(ptr, size);
	}

	return ptr;
}

void *
osl_malloc_nopersist(uint size)
{
	return hnd_malloc_align(size, 0, CALL_SITE);
}

void *
osl_malloc_persist(uint size, uint32 constraint)
{
	return osl_malloc_persist_ra(size, constraint, CALL_SITE);
}

static void *
osl_malloc_persist_ra(uint size, uint32 constraint, void *call_site)
{
	void *p;

#ifdef BCM_RECLAIM
	if (!ATTACH_PART_RECLAIMED())
		printf("MALLOC warning: allocations are persistent by default in attach phase!\n");
#endif /* BCM_RECLAIM */

	if (constraint)
		p = hnd_malloc_persist_attach_align(size, 0, call_site);
	else
		p = hnd_malloc_persist_align(size, 0, call_site);

	return p;
}

void *
osl_mallocz_persist(uint size, uint32 constraint)
{
	void *ptr;

	ptr = osl_malloc_persist_ra(size, constraint, CALL_SITE);

	if (ptr != NULL) {
		bzero(ptr, size);
	}

	return ptr;
}
#else
void *
osl_malloc(uint size)
{
	void *p;
	if (ATTACH_PART_RECLAIMED() || force_nopersist)
		p = hnd_malloc_align(size, 0);
	else
		p = hnd_malloc_persist_align(size, 0);

	if (p != NULL) {
		bzero(p, size);
	}
#ifdef BCMDBG_MEMNULL
	else
		printf("MALLOC failed: size=%d ra=0x%x\n", size, CALL_SITE);
#endif // endif

	return p;
}

void *
osl_malloc_persist(uint size, uint32 constraint)
{

#ifdef BCM_RECLAIM
	if (!ATTACH_PART_RECLAIMED())
		printf("MALLOC warning: allocations are persistent by default in attach phase!\n");
#endif /* BCM_RECLAIM */

	void *p;
	if (constraint)
		p = hnd_malloc_persist_attach_align(size, 0);
	else
		p = hnd_malloc_persist_align(size, 0);

#ifdef BCMDBG_MEMNULL
	if (p == NULL)
		printf("MALLOC failed: size=%d ra=%p\n", size, CALL_SITE);
#endif // endif
	return p;
}

void *
osl_mallocz(uint size)
{
	return osl_malloc(size);
}

void *
osl_mallocz_persist(uint size, uint32 constraint)
{
	void * ptr;

	ptr = osl_malloc_persist(size, constraint);

	if (ptr != NULL) {
		bzero(ptr, size);
	}

	return ptr;
}

void *
osl_malloc_align(uint size, uint align_bits)
{
	void *p;

	if (ATTACH_PART_RECLAIMED() || force_nopersist)
		p = hnd_malloc_align(size, align_bits);
	else
		p = hnd_malloc_persist_align(size, align_bits);

#ifdef BCMDBG_MEMNULL
	if (p == NULL)
		printf("MALLOC failed: size=%d ra=0x%x\n", size, CALL_SITE);
#endif // endif

	return p;
}

void *
osl_malloc_nopersist(uint size)
{
	void *p;

	p = hnd_malloc_align(size, 0);

#ifdef BCMDBG_MEMNULL
	if (p == NULL)
		printf("MALLOC failed: size=%d ra=0x%x\n", size, CALL_SITE);
#endif // endif

	return p;
}

void *
osl_mallocz_nopersist(uint size)
{
	void *ptr;

	ptr = osl_malloc_nopersist(size);

	if (ptr != NULL) {
		bzero(ptr, size);
	}

	return ptr;
}
#endif /* BCMDBG_MEM */

int
osl_mfree(void *addr)
{
	return hnd_free(addr);
}

uint
osl_malloced(osl_t *osh)
{
	return 0;
}

uint
osl_malloc_failed(osl_t *osh)
{
	return 0;
}

int
osl_busprobe(uint32 *val, uint32 addr)
{
	*val = *(uint32 *)addr;
	return 0;
}

/* translate these erros into hnd specific errors */
int
osl_error(int bcmerror)
{
	return bcmerror;
}

#ifdef SBPCI
#include <typedefs.h>
#include <rte_dev.h>

uint
osl_pci_bus(osl_t *osh)
{
	hnd_dev_t *dev = (hnd_dev_t *)osh->dev;
	pdev_t *pdev = (pdev_t *)dev->pdev;

	return pdev->bus;
}

uint
osl_pci_slot(osl_t *osh)
{
	hnd_dev_t *dev = (hnd_dev_t *)osh->dev;
	pdev_t *pdev = (pdev_t *)dev->pdev;

	return pdev->slot;
}

uint32
osl_pci_read_config(osl_t *osh, uint offset, uint size)
{
	hnd_dev_t *dev = (hnd_dev_t *)osh->dev;
	pdev_t *pdev = (pdev_t *)dev->pdev;
	uint32 data;

	if (extpci_read_config(pdev->sih, pdev->bus, pdev->slot, pdev->func, offset,
	                       &data, size) != 0)
		data = 0xffffffff;

	return data;
}

void
osl_pci_write_config(osl_t *osh, uint offset, uint size, uint val)
{
	hnd_dev_t *dev = (hnd_dev_t *)osh->dev;
	pdev_t *pdev = dev->pdev;

	extpci_write_config(pdev->sih, pdev->bus, pdev->slot, pdev->func, offset, &val, size);
}
#endif /* SBPCI */

uint32
osl_rand(void)
{
	uint32 x, hi, lo, t;

	x = OSL_SYSUPTIME();
	hi = x / 127773;
	lo = x % 127773;
	t = 16807 * lo - 2836 * hi;
	if (t <= 0) t += 0x7fffffff;
	return t;
}

extern void*
osl_get_fatal_logbuf(osl_t *osh, uint32 size, uint32 *alloced)
{
	return (hnd_get_fatal_logbuf(size, alloced));
}

#if defined(BCMDBG_MEM) || defined(BCMDBG_HEAPCHECK)
extern int
osl_memcheck(const char *file, int line)
{
	return (hnd_memcheck(file, line));
}
#endif // endif
