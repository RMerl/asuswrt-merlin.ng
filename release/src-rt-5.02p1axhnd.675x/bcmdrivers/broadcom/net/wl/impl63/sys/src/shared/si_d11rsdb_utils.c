/*
 * Misc utility routines for accessing SOC Interconnects
 * of Broadcom rsdb chips.
 *
 * Copyright (C) 2020, Broadcom. All Rights Reserved.
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
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: si_d11rsdb_utils.c 690155 2017-03-14 23:38:37Z $
 */
#include <bcm_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <bcmdevs.h>
#include <hndsoc.h>
#include <pcicfg.h>
#include "siutils_priv.h"

void
set_secondary_d11_core(si_t *sih, volatile void **secmap, volatile void **secwrap)
{
	si_info_t *sii = SI_INFO(sih);
	si_cores_info_t *cores_info = (si_cores_info_t *)sii->cores_info;
	uint32 addr, wrap;
	uint coreidx;

	coreidx = si_findcoreidx(sih, D11_CORE_ID, 1);
	addr = cores_info->coresba[coreidx];
	wrap = cores_info->wrapba[coreidx];

	OSL_PCI_WRITE_CONFIG(sii->osh, PCIE2_BAR0_CORE2_WIN, 4, addr);
	OSL_PCI_WRITE_CONFIG(sii->osh, PCIE2_BAR0_CORE2_WIN2, 4, wrap);
	*secmap = ((volatile char *)sii->curmap) + (4 * SI_CORE_SIZE);
	*secwrap = ((volatile char *)*secmap) + SI_CORE_SIZE;
}

void
si_d11rsdb_core_disable(si_t *sih, uint32 bits)
{
	si_info_t *sii = SI_INFO(sih);
	void *pmacai = NULL;
	volatile void *smacai = NULL;
	volatile void *secmap = NULL, *secwrap = NULL;
	uint origidx = 0;

	/* save current core index */
	origidx = si_coreidx(&sii->pub);

	if (BUSTYPE(sih->bustype) == SI_BUS) {
		si_setcore(sih, D11_CORE_ID, 1);
		smacai = sii->curwrap;
	} else {
		set_secondary_d11_core(sih, &secmap, &secwrap);
		smacai = secwrap;
	}

	si_setcore(sih, D11_CORE_ID, 0);
	pmacai = sii->curwrap;

	ai_d11rsdb_core_disable(sii, bits, pmacai, smacai);

	si_setcoreidx(sih, origidx);
}

void
si_d11rsdb_core_reset(si_t *sih, uint32 bits, uint32 resetbits)
{
	si_info_t *sii = SI_INFO(sih);
	void *pmacai = NULL;
	volatile void *smacai = NULL;
	volatile void *secmap = NULL, *secwrap = NULL;
	uint origidx = 0;

	/* save current core index */
	origidx = si_coreidx(&sii->pub);

	if (BUSTYPE(sih->bustype) == SI_BUS) {
		si_setcore(sih, D11_CORE_ID, 1);
		smacai = sii->curwrap;
	} else {
		set_secondary_d11_core(sih, &secmap, &secwrap);
		smacai = secwrap;
	}

	si_setcore(sih, D11_CORE_ID, 0);
	pmacai = sii->curwrap;

	ai_d11rsdb_core_reset(sih, bits, resetbits, pmacai, smacai);

	si_setcoreidx(sih, origidx);
}
