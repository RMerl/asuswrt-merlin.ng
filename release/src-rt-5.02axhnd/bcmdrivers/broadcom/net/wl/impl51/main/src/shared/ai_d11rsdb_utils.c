/*
 * Misc utility routines for accessing chip-specific features
 * of the SiliconBackplane-based Broadcom rsdb chips.
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
 * $Id: ai_d11rsdb_utils.c 690155 2017-03-14 23:38:37Z $
 */
#include <bcm_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <bcmdevs.h>
#include <hndsoc.h>
#include "siutils_priv.h"

#define	SICF_PCLKE		0x0004		/* PHY clock enable */

static void
ai_d11rsdb_reset_set(const si_info_t *sii, aidmp_t *pmacai, aidmp_t *smacai)
{
	volatile uint32 dummyp;
	volatile uint32 dummys;
	volatile uint32 rstctrls;
	volatile uint32 rstctrlp;
	uint loop_counter = 10;

#ifdef BCMDBG_ERR
	si_cores_info_t *cores_info = (si_cores_info_t *)sii->cores_info;
#endif // endif
	/* ensure there are no pending backplane operations */
	SPINWAIT(((dummyp = R_REG(sii->osh, &pmacai->resetstatus)) != 0), 300);
	/* ensure there are no pending backplane operations on core1 */
	SPINWAIT(((dummys = R_REG(sii->osh, &smacai->resetstatus)) != 0), 300);

	/* put core to reset */
	W_REG(sii->osh, &pmacai->resetctrl,  AIRC_RESET);
	W_REG(sii->osh, &smacai->resetctrl,  AIRC_RESET);

	OSL_DELAY(1000);

	/* put core to reset */
	W_REG(sii->osh, &pmacai->resetctrl,  AIRC_RESET);
	W_REG(sii->osh, &smacai->resetctrl,  AIRC_RESET);
#ifdef BCMDBG_ERR
	if (dummyp != 0)
		SI_ERROR(("%s: WARN2: primary mac resetstatus=0x%0x\n", __FUNCTION__, dummyp));
	if (dummys != 0)
		SI_ERROR(("%s: WARN2: secondary mac resetstatus=0x%0x\n", __FUNCTION__, dummys));
#endif // endif
	rstctrlp = R_REG(sii->osh, &pmacai->resetctrl);
	rstctrls = R_REG(sii->osh, &smacai->resetctrl);

	/* turn off resetn */
	while (((rstctrlp !=  AIRC_RESET) || (rstctrls !=  AIRC_RESET)) && --loop_counter != 0) {
		/* ensure there are no pending backplane operations */
		SPINWAIT(((dummyp = R_REG(sii->osh, &pmacai->resetstatus)) != 0), 300);
		/* ensure there are no pending backplane operations on core1 */
		SPINWAIT(((dummys = R_REG(sii->osh, &smacai->resetstatus)) != 0), 300);

#ifdef BCMDBG_ERR
		if (dummyp != 0)
			SI_ERROR(("%s: WARN2: primary mac resetstatus=0x%0x\n",
			__FUNCTION__, dummyp));
		if (dummys != 0)
			SI_ERROR(("%s: WARN2: secondary mac resetstatus=0x%0x\n",
			__FUNCTION__, dummys));
#endif // endif
		/* put core to reset */
		if (rstctrlp == 0)
			W_REG(sii->osh, &pmacai->resetctrl,  AIRC_RESET);

		if (rstctrls == 0)
			W_REG(sii->osh, &smacai->resetctrl,  AIRC_RESET);

		/* ensure there are no pending backplane operations */
		SPINWAIT(((dummyp = R_REG(sii->osh, &pmacai->resetstatus)) != 0), 300);
		/* ensure there are no pending backplane operations on core1 */
		SPINWAIT(((dummys = R_REG(sii->osh, &smacai->resetstatus)) != 0), 300);

		rstctrlp = R_REG(sii->osh, &pmacai->resetctrl);
		rstctrls = R_REG(sii->osh, &smacai->resetctrl);

		/* ensure there are no pending backplane operations */
		SPINWAIT(((dummyp = R_REG(sii->osh, &pmacai->resetstatus)) != 0), 300);
		/* ensure there are no pending backplane operations on core1 */
		SPINWAIT(((dummys = R_REG(sii->osh, &smacai->resetstatus)) != 0), 300);
	}

#ifdef BCMDBG_ERR
	if (loop_counter == 0)
		SI_ERROR(("%s: Failed to set core reset bit 0x%x\n",
		          __FUNCTION__, cores_info->coreid[sii->curidx]));
#endif // endif
}

static void
ai_d11rsdb_reset_clear(const si_info_t *sii, aidmp_t *pmacai, aidmp_t *smacai)
{
	volatile uint32 dummyp;
	volatile uint32 dummys;
	volatile uint32 rstctrls;
	volatile uint32 rstctrlp;
	uint loop_counter = 10;

#ifdef BCMDBG_ERR
	si_cores_info_t *cores_info = (si_cores_info_t *)sii->cores_info;
#endif // endif
	/* ensure there are no pending backplane operations */
	SPINWAIT(((dummyp = R_REG(sii->osh, &pmacai->resetstatus)) != 0), 300);
	/* ensure there are no pending backplane operations on core 1 */
	SPINWAIT(((dummys = R_REG(sii->osh, &smacai->resetstatus)) != 0), 300);

	/* take core out of reset */
	W_REG(sii->osh, &pmacai->resetctrl, 0);
	W_REG(sii->osh, &smacai->resetctrl, 0);

#ifdef BCMDBG_ERR
	if (dummyp != 0)
		SI_ERROR(("%s: WARN2: primary mac resetstatus=0x%0x\n", __FUNCTION__, dummyp));
	if (dummys != 0)
		SI_ERROR(("%s: WARN2: secondary mac resetstatus=0x%0x\n", __FUNCTION__, dummys));
#endif // endif
	rstctrlp = R_REG(sii->osh, &pmacai->resetctrl);
	rstctrls = R_REG(sii->osh, &smacai->resetctrl);

	/* turn off resetn */
	while (((rstctrlp == AIRC_RESET) || (rstctrls == AIRC_RESET)) && --loop_counter != 0) {
		/* ensure there are no pending backplane operations */
		SPINWAIT(((dummyp = R_REG(sii->osh, &pmacai->resetstatus)) != 0), 300);
		/* ensure there are no pending backplane operations on core 1 */
		SPINWAIT(((dummys = R_REG(sii->osh, &smacai->resetstatus)) != 0), 300);

#ifdef BCMDBG_ERR
		if (dummyp != 0)
			SI_ERROR(("%s: WARN2: primary mac resetstatus=0x%0x\n",
			__FUNCTION__, dummyp));
		if (dummys != 0)
			SI_ERROR(("%s: WARN2: secondary mac resetstatus=0x%0x\n",
			__FUNCTION__, dummys));
#endif // endif

		/* take core out of reset */
		if (rstctrlp)
			W_REG(sii->osh, &pmacai->resetctrl, 0);

		if (rstctrls)
			W_REG(sii->osh, &smacai->resetctrl, 0);

		/* ensure there are no pending backplane operations */
		SPINWAIT(((dummyp = R_REG(sii->osh, &pmacai->resetstatus)) != 0), 300);
		/* ensure there are no pending backplane operations on core 1 */
		SPINWAIT(((dummys = R_REG(sii->osh, &smacai->resetstatus)) != 0), 300);

		rstctrlp = R_REG(sii->osh, &pmacai->resetctrl);
		rstctrls = R_REG(sii->osh, &smacai->resetctrl);

		/* ensure there are no pending backplane operations */
		SPINWAIT(((dummyp = R_REG(sii->osh, &pmacai->resetstatus)) != 0), 300);
		/* ensure there are no pending backplane operations on core 1 */
		SPINWAIT(((dummys = R_REG(sii->osh, &smacai->resetstatus)) != 0), 300);
	}

#ifdef BCMDBG_ERR
	if (loop_counter == 0)
		SI_ERROR(("%s: Failed to take core 0x%x out of reset\n",
		          __FUNCTION__, cores_info->coreid[sii->curidx]));
#endif // endif
}

void
ai_d11rsdb_core_disable(const si_info_t *sii, uint32 bits,
	aidmp_t *pmacai, aidmp_t *smacai)
{
	if ((R_REG(sii->osh, &pmacai->resetctrl)
		& AIRC_RESET) ==  AIRC_RESET)
			return;

	W_REG(sii->osh, &pmacai->ioctrl, bits);
	W_REG(sii->osh, &smacai->ioctrl, bits);

	ai_d11rsdb_reset_set(sii, pmacai, smacai);
}

/* Steps followed here is same to both mac cores.
 * Note:
 * turn off force clock and phy clocks, core1, bit16s a dont-care
 * turn on phy clocks to resync them to macphy clock, core1, bit16s dont-care
 */
static void
ai_d11rsdb_clock_resync(const si_info_t *sii, uint32 bits, aidmp_t *ai)
{
	ASSERT(GOODREGS(ai));

	/* turn on phy clocks to resync them to macphy clock */
	W_REG(sii->osh, &ai->ioctrl, (bits | (SICF_PCLKE | SICF_CLOCK_EN)));
	R_REG(sii->osh, &ai->ioctrl);
	OSL_DELAY(1);
}

/* reset and re-enable RSDB d11 core
 * inputs:
 * bits - core specific bits that are set during and after reset sequence
 * resetbits - core specific bits that are set only during reset sequence
 */
void
ai_d11rsdb_core_reset(si_t *sih, uint32 bits,
	uint32 resetbits, void *p, volatile void *s)
{
	si_info_t *sii = SI_INFO(sih);
	aidmp_t *pmacai = p;
	aidmp_t *smacai = s;
	volatile uint32 dummy;

	ASSERT(GOODREGS(pmacai));
	ASSERT(GOODREGS(smacai));

	ASSERT((pmacai) == sii->curwrap);

	/* ensure there are no pending backplane operations */
	SPINWAIT(((dummy = R_REG(sii->osh, &pmacai->resetstatus)) != 0), 300);

#ifdef BCMDBG_ERR
	if (dummy != 0)
		SI_ERROR(("%s: WARN1: resetstatus=0x%0x\n", __FUNCTION__, dummy));
#endif // endif

	ai_d11rsdb_core_disable(sii, bits, pmacai, smacai);

	/* Turn on clock core0 */
	W_REG(sii->osh, &pmacai->ioctrl, (bits | resetbits
				| SICF_FGC | SICF_CLOCK_EN));
	dummy = R_REG(sii->osh, &pmacai->ioctrl);
	BCM_REFERENCE(dummy);

	/* Turn on clock core1 */
	W_REG(sii->osh, &smacai->ioctrl, (bits | resetbits
				| SICF_FGC | SICF_CLOCK_EN));
	dummy = R_REG(sii->osh, &smacai->ioctrl);
	BCM_REFERENCE(dummy);

	/* Turn on resetn */
	ai_d11rsdb_reset_clear(sii, pmacai, smacai);

	ai_d11rsdb_clock_resync(sii, bits, pmacai);

	ai_d11rsdb_clock_resync(sii, bits, smacai);
}
