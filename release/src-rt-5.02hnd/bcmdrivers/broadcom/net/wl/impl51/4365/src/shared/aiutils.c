/*
 * Misc utility routines for accessing chip-specific features
 * of the SiliconBackplane-based Broadcom chips.
 *
 * Copyright (C) 2017, Broadcom. All Rights Reserved.
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
 * $Id: aiutils.c 559970 2015-05-29 07:19:02Z $
 */
#include <bcm_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <hndsoc.h>
#include <sbchipc.h>
#include <pcicfg.h>

#include "siutils_priv.h"

#include <bcmdevs.h>

#define BCM47162_DMP() ((CHIPID(sih->chip) == BCM47162_CHIP_ID) && \
	    (CHIPREV(sih->chiprev) == 0) && \
	    (cores_info->coreid[sii->curidx] == MIPS74K_CORE_ID))

#define BCM5357_DMP() (((CHIPID(sih->chip) == BCM5357_CHIP_ID) || \
			(CHIPID(sih->chip) == BCM4749_CHIP_ID)) && \
	    (sih->chippkg == BCM5357_PKG_ID) && \
	    (cores_info->coreid[sii->curidx] == USB20H_CORE_ID))
#define BCM4707_DMP() (BCM4707_CHIP(CHIPID(sih->chip)) && \
	    (cores_info->coreid[sii->curidx] == NS_CCB_CORE_ID))
#define PMU_DMP()  (cores_info->coreid[sii->curidx] == PMU_CORE_ID)
#define GCI_DMP()  (cores_info->coreid[sii->curidx] == GCI_CORE_ID)

#if defined(BCM_BACKPLANE_TIMEOUT)
bool ai_get_apb_bridge(si_t * sih, uint32 coreidx, uint32 *apb_id, uint32 * apb_coreuinit);
#endif /* BCM_BACKPLANE_TIMEOUT */

/* EROM parsing */

static uint32
get_erom_ent(si_t *sih, uint32 **eromptr, uint32 mask, uint32 match)
{
	uint32 ent;
	uint inv = 0, nom = 0;

	while (TRUE) {
		ent = R_REG(si_osh(sih), *eromptr);
		(*eromptr)++;

		if (mask == 0)
			break;

		if ((ent & ER_VALID) == 0) {
			inv++;
			continue;
		}

		if (ent == (ER_END | ER_VALID))
			break;

		if ((ent & mask) == match)
			break;

		nom++;
	}

	SI_VMSG(("%s: Returning ent 0x%08x\n", __FUNCTION__, ent));
	if (inv + nom) {
		SI_VMSG(("  after %d invalid and %d non-matching entries\n", inv, nom));
	}
	return ent;
}

static uint32
get_asd(si_t *sih, uint32 **eromptr, uint sp, uint ad, uint st, uint32 *addrl, uint32 *addrh,
        uint32 *sizel, uint32 *sizeh)
{
	uint32 asd, sz, szd;

	asd = get_erom_ent(sih, eromptr, ER_VALID, ER_VALID);
	if (((asd & ER_TAG1) != ER_ADD) ||
	    (((asd & AD_SP_MASK) >> AD_SP_SHIFT) != sp) ||
	    ((asd & AD_ST_MASK) != st)) {
		/* This is not what we want, "push" it back */
		(*eromptr)--;
		return 0;
	}
	*addrl = asd & AD_ADDR_MASK;
	if (asd & AD_AG32)
		*addrh = get_erom_ent(sih, eromptr, 0, 0);
	else
		*addrh = 0;
	*sizeh = 0;
	sz = asd & AD_SZ_MASK;
	if (sz == AD_SZ_SZD) {
		szd = get_erom_ent(sih, eromptr, 0, 0);
		*sizel = szd & SD_SZ_MASK;
		if (szd & SD_SG32)
			*sizeh = get_erom_ent(sih, eromptr, 0, 0);
	} else
		*sizel = AD_SZ_BASE << (sz >> AD_SZ_SHIFT);

	SI_VMSG(("  SP %d, ad %d: st = %d, 0x%08x_0x%08x @ 0x%08x_0x%08x\n",
	        sp, ad, st, *sizeh, *sizel, *addrh, *addrl));

	return asd;
}

static void
ai_hwfixup(si_info_t *sii)
{
#ifdef	_CFE_
	si_cores_info_t *cores_info = (si_cores_info_t *)sii->cores_info;
	/* Fixup the interrupts in 4716 for i2s core so that ai_flag
	 * works without having to look at the core sinking the
	 * interrupt. We should have done this as the hardware default.
	 *
	 * Future chips should allocate interrupt lines in order (meaning
	 * no line should be skipped), without regard for core index.
	 */
	if (BUSTYPE(sii->pub.bustype) == SI_BUS &&
	    ((CHIPID(sii->pub.chip) == BCM4716_CHIP_ID) ||
		(CHIPID(sii->pub.chip) == BCM4748_CHIP_ID))) {
		aidmp_t *i2s, *pcie, *cpu;

		ASSERT(cores_info->coreid[3] == MIPS74K_CORE_ID);
		cpu = REG_MAP(cores_info->wrapba[3], SI_CORE_SIZE);
		ASSERT(cores_info->coreid[5] == PCIE_CORE_ID);
		pcie = REG_MAP(cores_info->wrapba[5], SI_CORE_SIZE);
		ASSERT(cores_info->coreid[8] == I2S_CORE_ID);
		i2s = REG_MAP(cores_info->wrapba[8], SI_CORE_SIZE);
		if ((R_REG(sii->osh, &cpu->oobselina74) != 0x08060504) ||
		    (R_REG(sii->osh, &pcie->oobselina74) != 0x08060504) ||
		    (R_REG(sii->osh, &i2s->oobselouta30) != 0x88)) {
			SI_VMSG(("Unexpected oob values, not fixing i2s interrupt\n"));
		} else {
			/* Move i2s interrupt to oob line 7 instead of 8 */
			W_REG(sii->osh, &cpu->oobselina74, 0x07060504);
			W_REG(sii->osh, &pcie->oobselina74, 0x07060504);
			W_REG(sii->osh, &i2s->oobselouta30, 0x87);
			SI_VMSG(("Changed i2s interrupt to use oob line 7 instead of 8\n"));
		}
	}
#endif	/* _CFE_ */
}

struct _corerev_entry {
	uint corerev;
	uint corerev_alias;
};
static struct _corerev_entry bcm4706_corerev_cc[] = {
	{ 0x1f, CC_4706B0_CORE_REV },
	{ 0, 0 }
};
static struct _corerev_entry bcm4706_corerev_socsram[] = {
	{ 0x05, SOCRAM_4706B0_CORE_REV },
	{ 0, 0 }
};
static struct _corerev_entry bcm4706_corerev_gmac[] = {
	{ 0x00, GMAC_4706B0_CORE_REV },
	{ 0, 0 }
};

struct _coreid_entry {
	uint coreid;
	uint coreid_alias;
};
static struct _coreid_entry bcm4706_coreid_table[] = {
	{	CC_4706_CORE_ID, CC_CORE_ID },
	{	SOCRAM_4706_CORE_ID, SOCRAM_CORE_ID },
	{	GMAC_4706_CORE_ID, GMAC_CORE_ID },
	{ 0, 0 }
};

static uint
BCMATTACHFN(remap_coreid)(si_t *sih, uint coreid)
{
	struct _coreid_entry *coreid_table = NULL;

	if (CHIPID(sih->chip) == BCM4706_CHIP_ID)
		coreid_table = &bcm4706_coreid_table[0];

	if (coreid_table != NULL) {
		uint i;

		for (i = 0; coreid_table[i].coreid; i++)
			if (coreid_table[i].coreid == coreid)
				return coreid_table[i].coreid_alias;
	}

	return coreid;
}

static uint
remap_corerev(si_t *sih, uint corerev)
{
	if (CHIPID(sih->chip) == BCM4706_CHIP_ID) {
		si_info_t *sii = SI_INFO(sih);
		si_cores_info_t *cores_info = (si_cores_info_t *)sii->cores_info;
		uint i, coreid = cores_info->coreid[sii->curidx];
		struct _corerev_entry *corerev_table = NULL;

		if (coreid == CC_CORE_ID)
			corerev_table = bcm4706_corerev_cc;
		else if (coreid == GMAC_CORE_ID)
			corerev_table = bcm4706_corerev_gmac;
		else if (coreid == SOCRAM_CORE_ID)
			corerev_table = bcm4706_corerev_socsram;
		if (corerev_table != NULL) {
			for (i = 0; corerev_table[i].corerev_alias; i++)
				if (corerev_table[i].corerev == corerev)
					return corerev_table[i].corerev_alias;
		}
	}

	return corerev;
}

/* parse the enumeration rom to identify all cores */
void
BCMATTACHFN(ai_scan)(si_t *sih, void *regs, uint devid)
{
	si_info_t *sii = SI_INFO(sih);
	si_cores_info_t *cores_info = (si_cores_info_t *)sii->cores_info;
	chipcregs_t *cc = (chipcregs_t *)regs;
	uint32 erombase, *eromptr, *eromlim;

	erombase = R_REG(sii->osh, &cc->eromptr);

	switch (BUSTYPE(sih->bustype)) {
	case SI_BUS:
		eromptr = (uint32 *)REG_MAP(erombase, SI_CORE_SIZE);
		break;

	case PCI_BUS:
		/* Set wrappers address */
		sii->curwrap = (void *)((uintptr)regs + SI_CORE_SIZE);

		/* Now point the window at the erom */
		OSL_PCI_WRITE_CONFIG(sii->osh, PCI_BAR0_WIN, 4, erombase);
		eromptr = regs;
		break;

#ifdef BCMSDIO
	case SPI_BUS:
	case SDIO_BUS:
		eromptr = (uint32 *)(uintptr)erombase;
		break;
#endif	/* BCMSDIO */
#ifdef BCMJTAG
	case JTAG_BUS:
		eromptr = (uint32 *)(uintptr)erombase;
		break;
#endif	/* BCMJTAG */

	case PCMCIA_BUS:
	default:
		SI_ERROR(("Don't know how to do AXI enumertion on bus %d\n", sih->bustype));
		ASSERT(0);
		return;
	}
	eromlim = eromptr + (ER_REMAPCONTROL / sizeof(uint32));
	sii->axi_num_wrappers = 0;

	SI_VMSG(("ai_scan: regs = 0x%p, erombase = 0x%08x, eromptr = 0x%p, eromlim = 0x%p\n",
	         regs, erombase, eromptr, eromlim));
	while (eromptr < eromlim) {
		uint32 cia, cib, cid, mfg, crev, nmw, nsw, nmp, nsp;
		uint32 mpd, asd, addrl, addrh, sizel, sizeh;
		uint i, j, idx;
		bool br;

		br = FALSE;

		/* Grok a component */
		cia = get_erom_ent(sih, &eromptr, ER_TAG, ER_CI);
		if (cia == (ER_END | ER_VALID)) {
			SI_VMSG(("Found END of erom after %d cores\n", sii->numcores));
			ai_hwfixup(sii);
			return;
		}

		cib = get_erom_ent(sih, &eromptr, 0, 0);

		if ((cib & ER_TAG) != ER_CI) {
			SI_ERROR(("CIA not followed by CIB\n"));
			goto error;
		}

		cid = (cia & CIA_CID_MASK) >> CIA_CID_SHIFT;
		mfg = (cia & CIA_MFG_MASK) >> CIA_MFG_SHIFT;
		crev = (cib & CIB_REV_MASK) >> CIB_REV_SHIFT;
		nmw = (cib & CIB_NMW_MASK) >> CIB_NMW_SHIFT;
		nsw = (cib & CIB_NSW_MASK) >> CIB_NSW_SHIFT;
		nmp = (cib & CIB_NMP_MASK) >> CIB_NMP_SHIFT;
		nsp = (cib & CIB_NSP_MASK) >> CIB_NSP_SHIFT;

#ifdef BCMDBG_SI
		SI_VMSG(("Found component 0x%04x/0x%04x rev %d at erom addr 0x%p, with nmw = %d, "
		         "nsw = %d, nmp = %d & nsp = %d\n",
		         mfg, cid, crev, eromptr - 1, nmw, nsw, nmp, nsp));
#else
		BCM_REFERENCE(crev);
#endif

		if (nsp == 0) {
			continue;
		}

		/* Include Default slave wrapper for timeout monitoring */
#if !defined(AXI_TIMEOUTS) && !defined(BCM_BACKPLANE_TIMEOUT)
		if ((mfg == MFGID_ARM) && (cid == DEF_AI_COMP))
			continue;
#else
		if ((CHIPTYPE(sii->pub.socitype) == SOCI_NAI) &&
			(mfg == MFGID_ARM) && (cid == DEF_AI_COMP))
			continue;
#endif

		if ((nmw + nsw == 0)) {
			/* A component which is not a core */
			if (cid == OOB_ROUTER_CORE_ID) {
				asd = get_asd(sih, &eromptr, 0, 0, AD_ST_SLAVE,
					&addrl, &addrh, &sizel, &sizeh);
				if (asd != 0) {
					sii->oob_router = addrl;
				}
			}
			if (cid != GMAC_COMMON_4706_CORE_ID && cid != NS_CCB_CORE_ID &&
				cid != PMU_CORE_ID && cid != GCI_CORE_ID)
				continue;
		}

		idx = sii->numcores;

		cores_info->cia[idx] = cia;
		cores_info->cib[idx] = cib;
		cores_info->coreid[idx] = remap_coreid(sih, cid);

		for (i = 0; i < nmp; i++) {
			mpd = get_erom_ent(sih, &eromptr, ER_VALID, ER_VALID);
			if ((mpd & ER_TAG) != ER_MP) {
				SI_ERROR(("Not enough MP entries for component 0x%x\n", cid));
				goto error;
			}
			SI_VMSG(("  Master port %d, mp: %d id: %d\n", i,
			         (mpd & MPD_MP_MASK) >> MPD_MP_SHIFT,
			         (mpd & MPD_MUI_MASK) >> MPD_MUI_SHIFT));
		}

		/* First Slave Address Descriptor should be port 0:
		 * the main register space for the core
		 */
		asd = get_asd(sih, &eromptr, 0, 0, AD_ST_SLAVE, &addrl, &addrh, &sizel, &sizeh);
		if (asd == 0) {
			do {
			/* Try again to see if it is a bridge */
			asd = get_asd(sih, &eromptr, 0, 0, AD_ST_BRIDGE, &addrl, &addrh,
			              &sizel, &sizeh);
			if (asd != 0)
				br = TRUE;
			else {
					if (br == TRUE) {
						break;
					}
					else if ((addrh != 0) || (sizeh != 0) ||
						(sizel != SI_CORE_SIZE)) {
						SI_ERROR(("addrh = 0x%x\t sizeh = 0x%x\t size1 ="
							"0x%x\n", addrh, sizeh, sizel));
						SI_ERROR(("First Slave ASD for"
							"core 0x%04x malformed "
							"(0x%08x)\n", cid, asd));
						goto error;
					}
				}
			} while (1);
		}
		cores_info->coresba[idx] = addrl;
		cores_info->coresba_size[idx] = sizel;
		/* Get any more ASDs in port 0 */
		j = 1;
		do {
			asd = get_asd(sih, &eromptr, 0, j, AD_ST_SLAVE, &addrl, &addrh,
			              &sizel, &sizeh);
			if ((asd != 0) && (j == 1) && (sizel == SI_CORE_SIZE)) {
				cores_info->coresba2[idx] = addrl;
				cores_info->coresba2_size[idx] = sizel;
			}
			j++;
		} while (asd != 0);

		/* Go through the ASDs for other slave ports */
		for (i = 1; i < nsp; i++) {
			j = 0;
			do {
				asd = get_asd(sih, &eromptr, i, j, AD_ST_SLAVE, &addrl, &addrh,
				              &sizel, &sizeh);

				if (asd == 0)
					break;
				j++;
			} while (1);
			if (j == 0) {
				SI_ERROR((" SP %d has no address descriptors\n", i));
				goto error;
			}
		}

		/* Now get master wrappers */
		for (i = 0; i < nmw; i++) {
			asd = get_asd(sih, &eromptr, i, 0, AD_ST_MWRAP, &addrl, &addrh,
			              &sizel, &sizeh);
			if (asd == 0) {
				SI_ERROR(("Missing descriptor for MW %d\n", i));
				goto error;
			}
			if ((sizeh != 0) || (sizel != SI_CORE_SIZE)) {
				SI_ERROR(("Master wrapper %d is not 4KB\n", i));
				goto error;
			}
			if (i == 0)
				cores_info->wrapba[idx] = addrl;
			else if (i == 1)
				cores_info->wrapba2[idx] = addrl;

			ASSERT(sii->axi_num_wrappers < SI_MAX_AXI_WRAPPERS);
			sii->axi_wrapper[sii->axi_num_wrappers].mfg = mfg;
			sii->axi_wrapper[sii->axi_num_wrappers].cid = cid;
			sii->axi_wrapper[sii->axi_num_wrappers].rev = crev;
			sii->axi_wrapper[sii->axi_num_wrappers].wrapper_type = AI_MASTER_WRAPPER;
			sii->axi_wrapper[sii->axi_num_wrappers].wrapper_addr = addrl;
			sii->axi_num_wrappers++;
			SI_VMSG(("MASTER WRAPPER: %d, mfg:%x, cid:%x, rev:%x, addr:%x, size:%x\n",
				sii->axi_num_wrappers, mfg, cid, crev, addrl, sizel));
		}

		/* And finally slave wrappers */
		for (i = 0; i < nsw; i++) {
			uint fwp = (nsp == 1) ? 0 : 1;
			asd = get_asd(sih, &eromptr, fwp + i, 0, AD_ST_SWRAP, &addrl, &addrh,
			              &sizel, &sizeh);

			/* cache APB bridge wrapper address for set/clear timeout */
			if ((mfg == MFGID_ARM) && (cid == APB_BRIDGE_ID)) {
				ASSERT(sii->num_br < SI_MAXBR);
				sii->br_wrapba[sii->num_br++] = addrl;
			}

			if (asd == 0) {
				SI_ERROR(("Missing descriptor for SW %d\n", i));
				goto error;
			}
			if ((sizeh != 0) || (sizel != SI_CORE_SIZE)) {
				SI_ERROR(("Slave wrapper %d is not 4KB\n", i));
				goto error;
			}
			if ((nmw == 0) && (i == 0))
				cores_info->wrapba[idx] = addrl;
			else if ((nmw == 0) && (i == 1))
				cores_info->wrapba2[idx] = addrl;

			/* Include all slave wrappers to the list to
			 * enable and monitor watchdog timeouts
			 */

			ASSERT(sii->axi_num_wrappers < SI_MAX_AXI_WRAPPERS);
			sii->axi_wrapper[sii->axi_num_wrappers].mfg = mfg;
			sii->axi_wrapper[sii->axi_num_wrappers].cid = cid;
			sii->axi_wrapper[sii->axi_num_wrappers].rev = crev;
			sii->axi_wrapper[sii->axi_num_wrappers].wrapper_type = AI_SLAVE_WRAPPER;
			sii->axi_wrapper[sii->axi_num_wrappers].wrapper_addr = addrl;
			sii->axi_num_wrappers++;

			SI_VMSG(("SLAVE WRAPPER: %d,  mfg:%x, cid:%x, rev:%x, addr:%x, size:%x\n",
				sii->axi_num_wrappers,  mfg, cid, crev, addrl, sizel));
		}

		if (CHIPID(sih->chip) == BCM4706_CHIP_ID) {
			/* Check if it's a low cost package */
			i = (R_REG(sii->osh, &cc->chipid) & CID_PKG_MASK) >> CID_PKG_SHIFT;
			if (i == BCM4706L_PKG_ID) {
				/* bcm4706L: only one GMAC */
				if (cid == GMAC_4706_CORE_ID) {
					for (j = 0; j < sii->numcores; j++) {
						if (cores_info->coreid[j] == GMAC_CORE_ID)
							break;
					}
					if (j != sii->numcores) {
						/* Found one GMAC already, ignore this one */
						continue;
					}
				}
			}
		}

#ifndef BCM_BACKPLANE_TIMEOUT
		/* Don't record bridges */
		if (br)
			continue;
#endif

		/* Done with core */
		sii->numcores++;
	}

	SI_ERROR(("Reached end of erom without finding END"));

error:
	sii->numcores = 0;
	return;
}

#define AI_SETCOREIDX_MAPSIZE(coreid) \
	(((coreid) == NS_CCB_CORE_ID) ? 15 * SI_CORE_SIZE : SI_CORE_SIZE)

/* This function changes the logical "focus" to the indicated core.
 * Return the current core's virtual address.
 */
static void *
_ai_setcoreidx(si_t *sih, uint coreidx, uint use_wrap2)
{
	si_info_t *sii = SI_INFO(sih);
	si_cores_info_t *cores_info = (si_cores_info_t *)sii->cores_info;
	uint32 addr, wrap, wrap2;
	void *regs;

	if (coreidx >= MIN(sii->numcores, SI_MAXCORES))
		return (NULL);

	addr = cores_info->coresba[coreidx];
	wrap = cores_info->wrapba[coreidx];
	wrap2 = cores_info->wrapba2[coreidx];

#ifdef BCM_BACKPLANE_TIMEOUT
	/* No need to disable interrupts while entering/exiting APB bridge core */
	if ((cores_info->coreid[coreidx] != APB_BRIDGE_CORE_ID) &&
		(cores_info->coreid[sii->curidx] != APB_BRIDGE_CORE_ID))
#endif /* BCM_BACKPLANE_TIMEOUT */
	{
		/*
		 * If the user has provided an interrupt mask enabled function,
		 * then assert interrupts are disabled before switching the core.
		 */
		ASSERT((sii->intrsenabled_fn == NULL) ||
			!(*(sii)->intrsenabled_fn)((sii)->intr_arg));
	}

	switch (BUSTYPE(sih->bustype)) {
	case SI_BUS:
		/* map new one */
		if (!cores_info->regs[coreidx]) {
			cores_info->regs[coreidx] = REG_MAP(addr,
				AI_SETCOREIDX_MAPSIZE(cores_info->coreid[coreidx]));
			ASSERT(GOODREGS(cores_info->regs[coreidx]));
		}
		sii->curmap = regs = cores_info->regs[coreidx];
		if (!cores_info->wrappers[coreidx] && (wrap != 0)) {
			cores_info->wrappers[coreidx] = REG_MAP(wrap, SI_CORE_SIZE);
			ASSERT(GOODREGS(cores_info->wrappers[coreidx]));
		}
		if (!cores_info->wrappers2[coreidx] && (wrap2 != 0)) {
			cores_info->wrappers2[coreidx] = REG_MAP(wrap2, SI_CORE_SIZE);
			ASSERT(GOODREGS(cores_info->wrappers2[coreidx]));
		}
		if (use_wrap2)
			sii->curwrap = cores_info->wrappers2[coreidx];
		else
			sii->curwrap = cores_info->wrappers[coreidx];
		break;

	case PCI_BUS:
#ifdef BCM_BACKPLANE_TIMEOUT
		/* No need to set the BAR0 if core is APB Bridge.
		 * This is to reduce 2 PCI writes while checkng for errlog
		 */
		if (cores_info->coreid[coreidx] != APB_BRIDGE_CORE_ID)
#endif /* BCM_BACKPLANE_TIMEOUT */
		{
			/* point bar0 window */
			OSL_PCI_WRITE_CONFIG(sii->osh, PCI_BAR0_WIN, 4, addr);
		}

		regs = sii->curmap;
		/* point bar0 2nd 4KB window to the primary wrapper */
		if (use_wrap2)
			wrap = wrap2;
		if (PCIE_GEN2(sii))
			OSL_PCI_WRITE_CONFIG(sii->osh, PCIE2_BAR0_WIN2, 4, wrap);
		else
			OSL_PCI_WRITE_CONFIG(sii->osh, PCI_BAR0_WIN2, 4, wrap);
		break;

#ifdef BCMSDIO
	case SPI_BUS:
	case SDIO_BUS:
		sii->curmap = regs = (void *)((uintptr)addr);
		if (use_wrap2)
			sii->curwrap = (void *)((uintptr)wrap2);
		else
			sii->curwrap = (void *)((uintptr)wrap);
		break;
#endif	/* BCMSDIO */
#ifdef BCMJTAG
	case JTAG_BUS:
		sii->curmap = regs = (void *)((uintptr)addr);
		if (use_wrap2)
			sii->curwrap = (void *)((uintptr)wrap2);
		else
			sii->curwrap = (void *)((uintptr)wrap);
		break;
#endif	/* BCMJTAG */

	case PCMCIA_BUS:
	default:
		ASSERT(0);
		regs = NULL;
		break;
	}

	sii->curmap = regs;
	sii->curidx = coreidx;

	return regs;
}

void *
ai_setcoreidx(si_t *sih, uint coreidx)
{
	return _ai_setcoreidx(sih, coreidx, 0);
}

void *
ai_setcoreidx_2ndwrap(si_t *sih, uint coreidx)
{
	return _ai_setcoreidx(sih, coreidx, 1);
}

void
ai_coreaddrspaceX(si_t *sih, uint asidx, uint32 *addr, uint32 *size)
{
	si_info_t *sii = SI_INFO(sih);
	si_cores_info_t *cores_info = (si_cores_info_t *)sii->cores_info;
	chipcregs_t *cc = NULL;
	uint32 erombase, *eromptr, *eromlim;
	uint i, j, cidx;
	uint32 cia, cib, nmp, nsp;
	uint32 asd, addrl, addrh, sizel, sizeh;

	for (i = 0; i < sii->numcores; i++) {
		if (cores_info->coreid[i] == CC_CORE_ID) {
			cc = (chipcregs_t *)cores_info->regs[i];
			break;
		}
	}
	if (cc == NULL)
		goto error;

	erombase = R_REG(sii->osh, &cc->eromptr);
	eromptr = (uint32 *)REG_MAP(erombase, SI_CORE_SIZE);
	eromlim = eromptr + (ER_REMAPCONTROL / sizeof(uint32));

	cidx = sii->curidx;
	cia = cores_info->cia[cidx];
	cib = cores_info->cib[cidx];

	nmp = (cib & CIB_NMP_MASK) >> CIB_NMP_SHIFT;
	nsp = (cib & CIB_NSP_MASK) >> CIB_NSP_SHIFT;

	/* scan for cores */
	while (eromptr < eromlim) {
		if ((get_erom_ent(sih, &eromptr, ER_TAG, ER_CI) == cia) &&
			(get_erom_ent(sih, &eromptr, 0, 0) == cib)) {
			break;
		}
	}

	/* skip master ports */
	for (i = 0; i < nmp; i++)
		get_erom_ent(sih, &eromptr, ER_VALID, ER_VALID);

	/* Skip ASDs in port 0 */
	asd = get_asd(sih, &eromptr, 0, 0, AD_ST_SLAVE, &addrl, &addrh, &sizel, &sizeh);
	if (asd == 0) {
		/* Try again to see if it is a bridge */
		asd = get_asd(sih, &eromptr, 0, 0, AD_ST_BRIDGE, &addrl, &addrh,
		              &sizel, &sizeh);
	}

	j = 1;
	do {
		asd = get_asd(sih, &eromptr, 0, j, AD_ST_SLAVE, &addrl, &addrh,
		              &sizel, &sizeh);
		j++;
	} while (asd != 0);

	/* Go through the ASDs for other slave ports */
	for (i = 1; i < nsp; i++) {
		j = 0;
		do {
			asd = get_asd(sih, &eromptr, i, j, AD_ST_SLAVE, &addrl, &addrh,
				&sizel, &sizeh);
			if (asd == 0)
				break;

			if (!asidx--) {
				*addr = addrl;
				*size = sizel;
				return;
			}
			j++;
		} while (1);

		if (j == 0) {
			SI_ERROR((" SP %d has no address descriptors\n", i));
			break;
		}
	}

error:
	*size = 0;
	return;
}

/* Return the number of address spaces in current core */
int
ai_numaddrspaces(si_t *sih)
{
	return 2;
}

/* Return the address of the nth address space in the current core */
uint32
ai_addrspace(si_t *sih, uint asidx)
{
	si_info_t *sii = SI_INFO(sih);
	si_cores_info_t *cores_info = (si_cores_info_t *)sii->cores_info;
	uint cidx;

	cidx = sii->curidx;

	if (asidx == 0)
		return cores_info->coresba[cidx];
	else if (asidx == 1)
		return cores_info->coresba2[cidx];
	else {
		SI_ERROR(("%s: Need to parse the erom again to find addr space %d\n",
		          __FUNCTION__, asidx));
		return 0;
	}
}

/* Return the size of the nth address space in the current core */
uint32
ai_addrspacesize(si_t *sih, uint asidx)
{
	si_info_t *sii = SI_INFO(sih);
	si_cores_info_t *cores_info = (si_cores_info_t *)sii->cores_info;
	uint cidx;

	cidx = sii->curidx;

	if (asidx == 0)
		return cores_info->coresba_size[cidx];
	else if (asidx == 1)
		return cores_info->coresba2_size[cidx];
	else {
		SI_ERROR(("%s: Need to parse the erom again to find addr space %d\n",
		          __FUNCTION__, asidx));
		return 0;
	}
}

uint
ai_flag(si_t *sih)
{
	si_info_t *sii = SI_INFO(sih);
	si_cores_info_t *cores_info = (si_cores_info_t *)sii->cores_info;
	aidmp_t *ai;

	if (BCM47162_DMP()) {
		SI_ERROR(("%s: Attempting to read MIPS DMP registers on 47162a0", __FUNCTION__));
		return sii->curidx;
	}
	if (BCM5357_DMP()) {
		SI_ERROR(("%s: Attempting to read USB20H DMP registers on 5357b0\n", __FUNCTION__));
		return sii->curidx;
	}
	if (BCM4707_DMP()) {
		SI_ERROR(("%s: Attempting to read CHIPCOMMONB DMP registers on 4707\n",
			__FUNCTION__));
		return sii->curidx;
	}

#ifdef REROUTE_OOBINT
	if (PMU_DMP()) {
		SI_ERROR(("%s: Attempting to read PMU DMP registers\n",
			__FUNCTION__));
		return PMU_OOB_BIT;
	}
#else
	if (PMU_DMP()) {
		uint idx, flag;
		idx = sii->curidx;
		ai_setcoreidx(sih, SI_CC_IDX);
		flag = ai_flag_alt(sih);
		ai_setcoreidx(sih, idx);
		return flag;
	}
#endif /* REROUTE_OOBINT */

	ai = sii->curwrap;
	ASSERT(ai != NULL);

	return (R_REG(sii->osh, &ai->oobselouta30) & 0x1f);
}

uint
ai_flag_alt(si_t *sih)
{
	si_info_t *sii = SI_INFO(sih);
	si_cores_info_t *cores_info = (si_cores_info_t *)sii->cores_info;
	aidmp_t *ai;

	if (BCM47162_DMP()) {
		SI_ERROR(("%s: Attempting to read MIPS DMP registers on 47162a0", __FUNCTION__));
		return sii->curidx;
	}
	if (BCM5357_DMP()) {
		SI_ERROR(("%s: Attempting to read USB20H DMP registers on 5357b0\n", __FUNCTION__));
		return sii->curidx;
	}
	if (BCM4707_DMP()) {
		SI_ERROR(("%s: Attempting to read CHIPCOMMONB DMP registers on 4707\n",
			__FUNCTION__));
		return sii->curidx;
	}
#ifdef REROUTE_OOBINT
	if (PMU_DMP()) {
		SI_ERROR(("%s: Attempting to read PMU DMP registers\n",
			__FUNCTION__));
		return PMU_OOB_BIT;
	}
#endif /* REROUTE_OOBINT */

	ai = sii->curwrap;

	return ((R_REG(sii->osh, &ai->oobselouta30) >> AI_OOBSEL_1_SHIFT) & AI_OOBSEL_MASK);
}

void
ai_setint(si_t *sih, int siflag)
{
}

uint
ai_wrap_reg(si_t *sih, uint32 offset, uint32 mask, uint32 val)
{
	si_info_t *sii = SI_INFO(sih);
	uint32 *map = (uint32 *) sii->curwrap;

	if (mask || val) {
		uint32 w = R_REG(sii->osh, map+(offset/4));
		w &= ~mask;
		w |= val;
		W_REG(sii->osh, map+(offset/4), w);
	}

	return (R_REG(sii->osh, map+(offset/4)));
}

uint
ai_corevendor(si_t *sih)
{
	si_info_t *sii = SI_INFO(sih);
	si_cores_info_t *cores_info = (si_cores_info_t *)sii->cores_info;
	uint32 cia;

	cia = cores_info->cia[sii->curidx];
	return ((cia & CIA_MFG_MASK) >> CIA_MFG_SHIFT);
}

uint
ai_corerev(si_t *sih)
{
	si_info_t *sii = SI_INFO(sih);
	si_cores_info_t *cores_info = (si_cores_info_t *)sii->cores_info;
	uint32 cib;


	cib = cores_info->cib[sii->curidx];
	return remap_corerev(sih, (cib & CIB_REV_MASK) >> CIB_REV_SHIFT);
}

bool
ai_iscoreup(si_t *sih)
{
	si_info_t *sii = SI_INFO(sih);
	aidmp_t *ai;

	ai = sii->curwrap;

	return (((R_REG(sii->osh, &ai->ioctrl) & (SICF_FGC | SICF_CLOCK_EN)) == SICF_CLOCK_EN) &&
	        ((R_REG(sii->osh, &ai->resetctrl) & AIRC_RESET) == 0));
}

/*
 * Switch to 'coreidx', issue a single arbitrary 32bit register mask&set operation,
 * switch back to the original core, and return the new value.
 *
 * When using the silicon backplane, no fiddling with interrupts or core switches is needed.
 *
 * Also, when using pci/pcie, we can optimize away the core switching for pci registers
 * and (on newer pci cores) chipcommon registers.
 */
uint
ai_corereg(si_t *sih, uint coreidx, uint regoff, uint mask, uint val)
{
	uint origidx = 0;
	uint32 *r = NULL;
	uint w;
	uint intr_val = 0;
	bool fast = FALSE;
	si_info_t *sii = SI_INFO(sih);
	si_cores_info_t *cores_info = (si_cores_info_t *)sii->cores_info;


	ASSERT(GOODIDX(coreidx));
	ASSERT(regoff < SI_CORE_SIZE);
	ASSERT((val & ~mask) == 0);

	if (coreidx >= SI_MAXCORES)
		return 0;

	if (BUSTYPE(sih->bustype) == SI_BUS) {
		/* If internal bus, we can always get at everything */
		fast = TRUE;
		/* map if does not exist */
		if (!cores_info->regs[coreidx]) {
			cores_info->regs[coreidx] = REG_MAP(cores_info->coresba[coreidx],
			                            SI_CORE_SIZE);
			ASSERT(GOODREGS(cores_info->regs[coreidx]));
		}
		r = (uint32 *)((uchar *)cores_info->regs[coreidx] + regoff);
	} else if (BUSTYPE(sih->bustype) == PCI_BUS) {
		/* If pci/pcie, we can get at pci/pcie regs and on newer cores to chipc */

		if ((cores_info->coreid[coreidx] == CC_CORE_ID) && SI_FAST(sii)) {
			/* Chipc registers are mapped at 12KB */

			fast = TRUE;
			r = (uint32 *)((char *)sii->curmap + PCI_16KB0_CCREGS_OFFSET + regoff);
		} else if (sii->pub.buscoreidx == coreidx) {
			/* pci registers are at either in the last 2KB of an 8KB window
			 * or, in pcie and pci rev 13 at 8KB
			 */
			fast = TRUE;
			if (SI_FAST(sii))
				r = (uint32 *)((char *)sii->curmap +
				               PCI_16KB0_PCIREGS_OFFSET + regoff);
			else
				r = (uint32 *)((char *)sii->curmap +
				               ((regoff >= SBCONFIGOFF) ?
				                PCI_BAR0_PCISBR_OFFSET : PCI_BAR0_PCIREGS_OFFSET) +
				               regoff);
		}
	}

	if (!fast) {
		INTR_OFF(sii, intr_val);

		/* save current core index */
		origidx = si_coreidx(&sii->pub);

		/* switch core */
		r = (uint32*) ((uchar*) ai_setcoreidx(&sii->pub, coreidx) + regoff);
	}
	ASSERT(r != NULL);

	/* mask and set */
	if (mask || val) {
		w = (R_REG(sii->osh, r) & ~mask) | val;
		W_REG(sii->osh, r, w);
	}

	/* readback */
	w = R_REG(sii->osh, r);

	if (!fast) {
		/* restore core index */
		if (origidx != coreidx)
			ai_setcoreidx(&sii->pub, origidx);

		INTR_RESTORE(sii, intr_val);
	}

	return (w);
}

/*
 * If there is no need for fiddling with interrupts or core switches (typically silicon
 * back plane registers, pci registers and chipcommon registers), this function
 * returns the register offset on this core to a mapped address. This address can
 * be used for W_REG/R_REG directly.
 *
 * For accessing registers that would need a core switch, this function will return
 * NULL.
 */
uint32 *
ai_corereg_addr(si_t *sih, uint coreidx, uint regoff)
{
	uint32 *r = NULL;
	bool fast = FALSE;
	si_info_t *sii = SI_INFO(sih);
	si_cores_info_t *cores_info = (si_cores_info_t *)sii->cores_info;


	ASSERT(GOODIDX(coreidx));
	ASSERT(regoff < SI_CORE_SIZE);

	if (coreidx >= SI_MAXCORES)
		return 0;

	if (BUSTYPE(sih->bustype) == SI_BUS) {
		/* If internal bus, we can always get at everything */
		fast = TRUE;
		/* map if does not exist */
		if (!cores_info->regs[coreidx]) {
			cores_info->regs[coreidx] = REG_MAP(cores_info->coresba[coreidx],
			                            SI_CORE_SIZE);
			ASSERT(GOODREGS(cores_info->regs[coreidx]));
		}
		r = (uint32 *)((uchar *)cores_info->regs[coreidx] + regoff);
	} else if (BUSTYPE(sih->bustype) == PCI_BUS) {
		/* If pci/pcie, we can get at pci/pcie regs and on newer cores to chipc */

		if ((cores_info->coreid[coreidx] == CC_CORE_ID) && SI_FAST(sii)) {
			/* Chipc registers are mapped at 12KB */

			fast = TRUE;
			r = (uint32 *)((char *)sii->curmap + PCI_16KB0_CCREGS_OFFSET + regoff);
		} else if (sii->pub.buscoreidx == coreidx) {
			/* pci registers are at either in the last 2KB of an 8KB window
			 * or, in pcie and pci rev 13 at 8KB
			 */
			fast = TRUE;
			if (SI_FAST(sii))
				r = (uint32 *)((char *)sii->curmap +
				               PCI_16KB0_PCIREGS_OFFSET + regoff);
			else
				r = (uint32 *)((char *)sii->curmap +
				               ((regoff >= SBCONFIGOFF) ?
				                PCI_BAR0_PCISBR_OFFSET : PCI_BAR0_PCIREGS_OFFSET) +
				               regoff);
		}
	}

	if (!fast) {
		ASSERT(sii->curidx == coreidx);
		r = (uint32*) ((uchar*)sii->curmap + regoff);
	}

	return (r);
}

void
ai_core_disable(si_t *sih, uint32 bits)
{
	si_info_t *sii = SI_INFO(sih);
	volatile uint32 dummy;
	uint32 status;
	aidmp_t *ai;


	ASSERT(GOODREGS(sii->curwrap));
	ai = sii->curwrap;

	/* if core is already in reset, just return */
	if (R_REG(sii->osh, &ai->resetctrl) & AIRC_RESET)
		return;

	/* ensure there are no pending backplane operations */
	SPINWAIT(((status = R_REG(sii->osh, &ai->resetstatus)) != 0), 300);

	/* if pending backplane ops still, try waiting longer */
	if (status != 0) {
		/* 300usecs was sufficient to allow backplane ops to clear for big hammer */
		/* during driver load we may need more time */
		SPINWAIT(((status = R_REG(sii->osh, &ai->resetstatus)) != 0), 10000);
		/* if still pending ops, continue on and try disable anyway */
		/* this is in big hammer path, so don't call wl_reinit in this case... */
#ifdef BCMDBG
		if (status != 0) {
			SI_PRINT(("%s: WARN: resetstatus=%0x on core disable\n",
				__FUNCTION__, status));
		}
#endif
	}

	W_REG(sii->osh, &ai->resetctrl, AIRC_RESET);
	dummy = R_REG(sii->osh, &ai->resetctrl);
	BCM_REFERENCE(dummy);
	OSL_DELAY(1);

	W_REG(sii->osh, &ai->ioctrl, bits);
	dummy = R_REG(sii->osh, &ai->ioctrl);
	BCM_REFERENCE(dummy);
	OSL_DELAY(10);
}

/* reset and re-enable a core
 * inputs:
 * bits - core specific bits that are set during and after reset sequence
 * resetbits - core specific bits that are set only during reset sequence
 */
static void
_ai_core_reset(si_t *sih, uint32 bits, uint32 resetbits)
{
	si_info_t *sii = SI_INFO(sih);
#ifdef BCMDBG
	si_cores_info_t *cores_info = (si_cores_info_t *)sii->cores_info;
#endif
	aidmp_t *ai;
	volatile uint32 dummy;
	uint loop_counter = 10;

	ASSERT(GOODREGS(sii->curwrap));
	ai = sii->curwrap;

	/* ensure there are no pending backplane operations */
	SPINWAIT(((dummy = R_REG(sii->osh, &ai->resetstatus)) != 0), 300);

#ifdef BCMDBG
	if (dummy != 0)
		SI_ERROR(("%s: WARN1: resetstatus=0x%0x\n", __FUNCTION__, dummy));
#endif

	/* put core into reset state */
	W_REG(sii->osh, &ai->resetctrl, AIRC_RESET);
	OSL_DELAY(10);

	/* ensure there are no pending backplane operations */
	SPINWAIT((R_REG(sii->osh, &ai->resetstatus) != 0), 300);

	W_REG(sii->osh, &ai->ioctrl, (bits | resetbits | SICF_FGC | SICF_CLOCK_EN));
	dummy = R_REG(sii->osh, &ai->ioctrl);
	BCM_REFERENCE(dummy);

	/* ensure there are no pending backplane operations */
	SPINWAIT(((dummy = R_REG(sii->osh, &ai->resetstatus)) != 0), 300);

#ifdef BCMDBG
	if (dummy != 0)
		SI_ERROR(("%s: WARN2: resetstatus=0x%0x\n", __FUNCTION__, dummy));
#endif

	while (R_REG(sii->osh, &ai->resetctrl) != 0 && --loop_counter != 0) {
		/* ensure there are no pending backplane operations */
		SPINWAIT(((dummy = R_REG(sii->osh, &ai->resetstatus)) != 0), 300);

#ifdef BCMDBG
		if (dummy != 0)
			SI_ERROR(("%s: WARN3 resetstatus=0x%0x\n", __FUNCTION__, dummy));
#endif

		/* take core out of reset */
		W_REG(sii->osh, &ai->resetctrl, 0);

		/* ensure there are no pending backplane operations */
		SPINWAIT((R_REG(sii->osh, &ai->resetstatus) != 0), 300);
	}

#ifdef BCMDBG
	if (loop_counter == 0)
		SI_ERROR(("%s: Failed to take core 0x%x out of reset\n",
		          __FUNCTION__, cores_info->coreid[sii->curidx]));
#endif

	W_REG(sii->osh, &ai->ioctrl, (bits | SICF_CLOCK_EN));
	dummy = R_REG(sii->osh, &ai->ioctrl);
	BCM_REFERENCE(dummy);
	OSL_DELAY(1);
}

void
ai_core_reset(si_t *sih, uint32 bits, uint32 resetbits)
{
	si_info_t *sii = SI_INFO(sih);
	si_cores_info_t *cores_info = (si_cores_info_t *)sii->cores_info;
	uint idx = sii->curidx;

	if (cores_info->wrapba2[idx] != 0) {
		ai_setcoreidx_2ndwrap(sih, idx);
		_ai_core_reset(sih, bits, resetbits);
		ai_setcoreidx(sih, idx);
	}

	_ai_core_reset(sih, bits, resetbits);
}

void
ai_core_cflags_wo(si_t *sih, uint32 mask, uint32 val)
{
	si_info_t *sii = SI_INFO(sih);
	si_cores_info_t *cores_info = (si_cores_info_t *)sii->cores_info;
	aidmp_t *ai;
	uint32 w;


	if (BCM47162_DMP()) {
		SI_ERROR(("%s: Accessing MIPS DMP register (ioctrl) on 47162a0",
		          __FUNCTION__));
		return;
	}
	if (BCM5357_DMP()) {
		SI_ERROR(("%s: Accessing USB20H DMP register (ioctrl) on 5357\n",
		          __FUNCTION__));
		return;
	}
	if (BCM4707_DMP()) {
		SI_ERROR(("%s: Accessing CHIPCOMMONB DMP register (ioctrl) on 4707\n",
			__FUNCTION__));
		return;
	}
	if (PMU_DMP()) {
		SI_ERROR(("%s: Accessing PMU DMP register (ioctrl)\n",
			__FUNCTION__));
		return;
	}

	ASSERT(GOODREGS(sii->curwrap));
	ai = sii->curwrap;

	ASSERT((val & ~mask) == 0);

	if (mask || val) {
		w = ((R_REG(sii->osh, &ai->ioctrl) & ~mask) | val);
		W_REG(sii->osh, &ai->ioctrl, w);
	}
}

uint32
ai_core_cflags(si_t *sih, uint32 mask, uint32 val)
{
	si_info_t *sii = SI_INFO(sih);
	si_cores_info_t *cores_info = (si_cores_info_t *)sii->cores_info;
	aidmp_t *ai;
	uint32 w;

	if (BCM47162_DMP()) {
		SI_ERROR(("%s: Accessing MIPS DMP register (ioctrl) on 47162a0",
		          __FUNCTION__));
		return 0;
	}
	if (BCM5357_DMP()) {
		SI_ERROR(("%s: Accessing USB20H DMP register (ioctrl) on 5357\n",
		          __FUNCTION__));
		return 0;
	}
	if (BCM4707_DMP()) {
		SI_ERROR(("%s: Accessing CHIPCOMMONB DMP register (ioctrl) on 4707\n",
			__FUNCTION__));
		return 0;
	}

	if (PMU_DMP()) {
		SI_ERROR(("%s: Accessing PMU DMP register (ioctrl)\n",
			__FUNCTION__));
		return 0;
	}
	ASSERT(GOODREGS(sii->curwrap));
	ai = sii->curwrap;

	ASSERT((val & ~mask) == 0);

	if (mask || val) {
		w = ((R_REG(sii->osh, &ai->ioctrl) & ~mask) | val);
		W_REG(sii->osh, &ai->ioctrl, w);
	}

	return R_REG(sii->osh, &ai->ioctrl);
}

uint32
ai_core_sflags(si_t *sih, uint32 mask, uint32 val)
{
	si_info_t *sii = SI_INFO(sih);
	si_cores_info_t *cores_info = (si_cores_info_t *)sii->cores_info;
	aidmp_t *ai;
	uint32 w;

	if (BCM47162_DMP()) {
		SI_ERROR(("%s: Accessing MIPS DMP register (iostatus) on 47162a0",
		          __FUNCTION__));
		return 0;
	}
	if (BCM5357_DMP()) {
		SI_ERROR(("%s: Accessing USB20H DMP register (iostatus) on 5357\n",
		          __FUNCTION__));
		return 0;
	}
	if (BCM4707_DMP()) {
		SI_ERROR(("%s: Accessing CHIPCOMMONB DMP register (ioctrl) on 4707\n",
			__FUNCTION__));
		return 0;
	}
	if (PMU_DMP()) {
		SI_ERROR(("%s: Accessing PMU DMP register (ioctrl)\n",
			__FUNCTION__));
		return 0;
	}

	ASSERT(GOODREGS(sii->curwrap));
	ai = sii->curwrap;

	ASSERT((val & ~mask) == 0);
	ASSERT((mask & ~SISF_CORE_BITS) == 0);

	if (mask || val) {
		w = ((R_REG(sii->osh, &ai->iostatus) & ~mask) | val);
		W_REG(sii->osh, &ai->iostatus, w);
	}

	return R_REG(sii->osh, &ai->iostatus);
}

#if defined(BCMDBG) || defined(BCMDBG_DUMP) || defined(BCMDBG_PHYDUMP)
/* print interesting aidmp registers */
void
ai_dumpregs(si_t *sih, struct bcmstrbuf *b)
{
	si_info_t *sii = SI_INFO(sih);
	si_cores_info_t *cores_info = (si_cores_info_t *)sii->cores_info;
	osl_t *osh;
	aidmp_t *ai;
	uint i;
	uint32 prev_value = 0;
	uint32 cfg_reg = 0, offset = 0;

	osh = sii->osh;

	/* Save and restore wrapper access window */
	if (BUSTYPE(sii->pub.bustype) == PCI_BUS) {
		if (PCIE_GEN2(sii)) {
			cfg_reg = PCIE2_BAR0_CORE2_WIN2;
			offset = PCIE2_BAR0_CORE2_WIN2_OFFSET;
		} else {
			cfg_reg = PCI_BAR0_WIN2;
			offset = PCI_16KB0_WIN2_OFFSET;
		}
		prev_value = OSL_PCI_READ_CONFIG(osh, cfg_reg, 4);

		if (prev_value == 0xFFFFFFFF) {
			SI_PRINT(("%s, PCI_BAR0_WIN2 - %x\n", __FUNCTION__, prev_value));
			return;
		}
	}

	bcm_bprintf(b, "ChipNum:%x, ChipRev;%x, BusType:%x, BoardType:%x, BoardVendor:%x\n\n",
		sih->chip, sih->chiprev, sih->bustype, sih->boardtype, sih->boardvendor);

	for (i = 0; i < sii->axi_num_wrappers; i++) {

		if (BUSTYPE(sii->pub.bustype) == PCI_BUS) {
			OSL_PCI_WRITE_CONFIG(osh, cfg_reg, 4, sii->axi_wrapper[i].wrapper_addr);
			ai = (aidmp_t *)((uint8*)sii->curmap + offset);
		} else {
			ai = (aidmp_t *)(uintptr)sii->axi_wrapper[i].wrapper_addr;
		}

		bcm_bprintf(b, "core 0x%x: core_rev:%d, %s_WR ADDR:%x \n", sii->axi_wrapper[i].cid,
			sii->axi_wrapper[i].rev,
			sii->axi_wrapper[i].wrapper_type == AI_SLAVE_WRAPPER ? "SLAVE" : "MASTER",
			sii->axi_wrapper[i].wrapper_addr);

		if (BCM47162_DMP()) {
			bcm_bprintf(b, "Skipping mips74k in 47162a0\n");
			continue;
		}
		if (BCM5357_DMP()) {
			bcm_bprintf(b, "Skipping usb20h in 5357\n");
			continue;
		}
		if (BCM4707_DMP()) {
			bcm_bprintf(b, "Skipping chipcommonb in 4707\n");
			continue;
		}

		if (PMU_DMP()) {
			bcm_bprintf(b, "Skipping pmu core\n");
			continue;
		}

		bcm_bprintf(b, "ioctrlset 0x%x ioctrlclear 0x%x ioctrl 0x%x iostatus 0x%x "
			    "ioctrlwidth 0x%x iostatuswidth 0x%x\n"
			    "resetctrl 0x%x resetstatus 0x%x resetreadid 0x%x resetwriteid 0x%x\n"
			    "errlogctrl 0x%x errlogdone 0x%x errlogstatus 0x%x "
			    "errlogaddrlo 0x%x errlogaddrhi 0x%x\n"
			    "errlogid 0x%x errloguser 0x%x errlogflags 0x%x\n"
			    "intstatus 0x%x config 0x%x itcr 0x%x\n\n",
			    R_REG(osh, &ai->ioctrlset),
			    R_REG(osh, &ai->ioctrlclear),
			    R_REG(osh, &ai->ioctrl),
			    R_REG(osh, &ai->iostatus),
			    R_REG(osh, &ai->ioctrlwidth),
			    R_REG(osh, &ai->iostatuswidth),
			    R_REG(osh, &ai->resetctrl),
			    R_REG(osh, &ai->resetstatus),
			    R_REG(osh, &ai->resetreadid),
			    R_REG(osh, &ai->resetwriteid),
			    R_REG(osh, &ai->errlogctrl),
			    R_REG(osh, &ai->errlogdone),
			    R_REG(osh, &ai->errlogstatus),
			    R_REG(osh, &ai->errlogaddrlo),
			    R_REG(osh, &ai->errlogaddrhi),
			    R_REG(osh, &ai->errlogid),
			    R_REG(osh, &ai->errloguser),
			    R_REG(osh, &ai->errlogflags),
			    R_REG(osh, &ai->intstatus),
			    R_REG(osh, &ai->config),
			    R_REG(osh, &ai->itcr));
	}

	/* Restore the initial wrapper space */
	if (BUSTYPE(sii->pub.bustype) == PCI_BUS) {
		if (prev_value) {
			OSL_PCI_WRITE_CONFIG(osh, cfg_reg, 4, prev_value);
		}
	}
}
#endif	/* BCMDBG || BCMDBG_DUMP || BCMDBG_PHYDUMP */

#ifdef BCMDBG
static void
_ai_view(osl_t *osh, aidmp_t *ai, uint32 cid, uint32 addr, bool verbose)
{
	uint32 config;

	config = R_REG(osh, &ai->config);
	SI_ERROR(("\nCore ID: 0x%x, addr 0x%x, config 0x%x\n", cid, addr, config));

	if (config & AICFG_RST)
		SI_ERROR(("resetctrl 0x%x, resetstatus 0x%x, resetreadid 0x%x, resetwriteid 0x%x\n",
		          R_REG(osh, &ai->resetctrl), R_REG(osh, &ai->resetstatus),
		          R_REG(osh, &ai->resetreadid), R_REG(osh, &ai->resetwriteid)));

	if (config & AICFG_IOC)
		SI_ERROR(("ioctrl 0x%x, width %d\n", R_REG(osh, &ai->ioctrl),
		          R_REG(osh, &ai->ioctrlwidth)));

	if (config & AICFG_IOS)
		SI_ERROR(("iostatus 0x%x, width %d\n", R_REG(osh, &ai->iostatus),
		          R_REG(osh, &ai->iostatuswidth)));

	if (config & AICFG_ERRL) {
		SI_ERROR(("errlogctrl 0x%x, errlogdone 0x%x, errlogstatus 0x%x, intstatus 0x%x\n",
		          R_REG(osh, &ai->errlogctrl), R_REG(osh, &ai->errlogdone),
		          R_REG(osh, &ai->errlogstatus), R_REG(osh, &ai->intstatus)));
		SI_ERROR(("errlogid 0x%x, errloguser 0x%x, errlogflags 0x%x, errlogaddr "
		          "0x%x/0x%x\n",
		          R_REG(osh, &ai->errlogid), R_REG(osh, &ai->errloguser),
		          R_REG(osh, &ai->errlogflags), R_REG(osh, &ai->errlogaddrhi),
		          R_REG(osh, &ai->errlogaddrlo)));
	}

	if (verbose && (config & AICFG_OOB)) {
		SI_ERROR(("oobselina30 0x%x, oobselina74 0x%x\n",
		          R_REG(osh, &ai->oobselina30), R_REG(osh, &ai->oobselina74)));
		SI_ERROR(("oobselinb30 0x%x, oobselinb74 0x%x\n",
		          R_REG(osh, &ai->oobselinb30), R_REG(osh, &ai->oobselinb74)));
		SI_ERROR(("oobselinc30 0x%x, oobselinc74 0x%x\n",
		          R_REG(osh, &ai->oobselinc30), R_REG(osh, &ai->oobselinc74)));
		SI_ERROR(("oobselind30 0x%x, oobselind74 0x%x\n",
		          R_REG(osh, &ai->oobselind30), R_REG(osh, &ai->oobselind74)));
		SI_ERROR(("oobselouta30 0x%x, oobselouta74 0x%x\n",
		          R_REG(osh, &ai->oobselouta30), R_REG(osh, &ai->oobselouta74)));
		SI_ERROR(("oobseloutb30 0x%x, oobseloutb74 0x%x\n",
		          R_REG(osh, &ai->oobseloutb30), R_REG(osh, &ai->oobseloutb74)));
		SI_ERROR(("oobseloutc30 0x%x, oobseloutc74 0x%x\n",
		          R_REG(osh, &ai->oobseloutc30), R_REG(osh, &ai->oobseloutc74)));
		SI_ERROR(("oobseloutd30 0x%x, oobseloutd74 0x%x\n",
		          R_REG(osh, &ai->oobseloutd30), R_REG(osh, &ai->oobseloutd74)));
		SI_ERROR(("oobsynca 0x%x, oobseloutaen 0x%x\n",
		          R_REG(osh, &ai->oobsynca), R_REG(osh, &ai->oobseloutaen)));
		SI_ERROR(("oobsyncb 0x%x, oobseloutben 0x%x\n",
		          R_REG(osh, &ai->oobsyncb), R_REG(osh, &ai->oobseloutben)));
		SI_ERROR(("oobsyncc 0x%x, oobseloutcen 0x%x\n",
		          R_REG(osh, &ai->oobsyncc), R_REG(osh, &ai->oobseloutcen)));
		SI_ERROR(("oobsyncd 0x%x, oobseloutden 0x%x\n",
		          R_REG(osh, &ai->oobsyncd), R_REG(osh, &ai->oobseloutden)));
		SI_ERROR(("oobaextwidth 0x%x, oobainwidth 0x%x, oobaoutwidth 0x%x\n",
		          R_REG(osh, &ai->oobaextwidth), R_REG(osh, &ai->oobainwidth),
		          R_REG(osh, &ai->oobaoutwidth)));
		SI_ERROR(("oobbextwidth 0x%x, oobbinwidth 0x%x, oobboutwidth 0x%x\n",
		          R_REG(osh, &ai->oobbextwidth), R_REG(osh, &ai->oobbinwidth),
		          R_REG(osh, &ai->oobboutwidth)));
		SI_ERROR(("oobcextwidth 0x%x, oobcinwidth 0x%x, oobcoutwidth 0x%x\n",
		          R_REG(osh, &ai->oobcextwidth), R_REG(osh, &ai->oobcinwidth),
		          R_REG(osh, &ai->oobcoutwidth)));
		SI_ERROR(("oobdextwidth 0x%x, oobdinwidth 0x%x, oobdoutwidth 0x%x\n",
		          R_REG(osh, &ai->oobdextwidth), R_REG(osh, &ai->oobdinwidth),
		          R_REG(osh, &ai->oobdoutwidth)));
	}
}

void
ai_view(si_t *sih, bool verbose)
{
	si_info_t *sii = SI_INFO(sih);
	si_cores_info_t *cores_info = (si_cores_info_t *)sii->cores_info;
	osl_t *osh;
	aidmp_t *ai;
	uint32 cid, addr;

	ai = sii->curwrap;
	osh = sii->osh;
	if (BCM47162_DMP()) {
		SI_ERROR(("Cannot access mips74k DMP in 47162a0\n"));
		return;
	}
	if (BCM5357_DMP()) {
		SI_ERROR(("Cannot access usb20h DMP in 5357\n"));
		return;
	}
	if (BCM4707_DMP()) {
		SI_ERROR(("Cannot access chipcommonb DMP in 4707\n"));
		return;
	}
	if (PMU_DMP()) {
		SI_ERROR(("Cannot access pmu DMP\n"));
		return;
	}
	cid = cores_info->coreid[sii->curidx];
	addr = cores_info->wrapba[sii->curidx];
	_ai_view(osh, ai, cid, addr, verbose);
}

void
ai_viewall(si_t *sih, bool verbose)
{
	si_info_t *sii = SI_INFO(sih);
	si_cores_info_t *cores_info = (si_cores_info_t *)sii->cores_info;
	osl_t *osh;
	aidmp_t *ai;
	uint32 cid, addr;
	uint i;

	osh = sii->osh;
	for (i = 0; i < sii->numcores; i++) {
		si_setcoreidx(sih, i);
		if (BCM47162_DMP()) {
			SI_ERROR(("Skipping mips74k DMP in 47162a0\n"));
			continue;
		}
		if (BCM5357_DMP()) {
			SI_ERROR(("Skipping usb20h DMP in 5357\n"));
			continue;
		}
		if (BCM4707_DMP()) {
			SI_ERROR(("Skipping chipcommonb DMP in 4707\n"));
			continue;
		}
		if (PMU_DMP()) {
			SI_ERROR(("Skipping pmu DMP\n"));
			continue;
		}
		ai = sii->curwrap;
		cid = cores_info->coreid[sii->curidx];
		addr = cores_info->wrapba[sii->curidx];
		_ai_view(osh, ai, cid, addr, verbose);
		if ((sih->chip == BCM4331_CHIP_ID) && (cores_info->coreid[i] == PCIE_CORE_ID)) {
			/* point bar0 2nd 4KB window */
			OSL_PCI_WRITE_CONFIG(sii->osh, PCI_BAR0_WIN2, 4, 0x18103000);
			_ai_view(osh, ai, cid, 0x18103000, verbose);
			OSL_PCI_WRITE_CONFIG(sii->osh, PCI_BAR0_WIN2, 4, 0x18104000);
			_ai_view(osh, ai, 0x135, 0x18104000, verbose);
			OSL_PCI_WRITE_CONFIG(sii->osh, PCI_BAR0_WIN2, 4, 0x18105000);
			_ai_view(osh, ai, 0x135, 0x18105000, verbose);
			/* bar0 2nd 4KB window will be fixed in the next setcore */
		}
	}
}
#endif	/* BCMDBG */

void
BCMATTACHFN(ai_enable_backplane_timeouts)(si_t *sih)
{
#if defined(AXI_TIMEOUTS) || defined(BCM_BACKPLANE_TIMEOUT)
	si_info_t *sii = SI_INFO(sih);
	aidmp_t *ai;
	int i;

#ifdef BCM_BACKPLANE_TIMEOUT
	uint32 prev_value = 0;
	osl_t *osh = sii->osh;
	uint32 cfg_reg = 0;
	uint32 offset = 0;


	if ((sii->axi_num_wrappers == 0) || (!PCIE(sii)))
#else
	if (sii->axi_num_wrappers == 0)
#endif
	{
		SI_VMSG((" %s, axi_num_wrappers:%d, Is_PCIE:%d, BUS_TYPE:%d, ID:%x\n",
			__FUNCTION__, sii->axi_num_wrappers, PCIE(sii),
			BUSTYPE(sii->pub.bustype), sii->pub.buscoretype));
		return;
	}

#ifdef BCM_BACKPLANE_TIMEOUT
	/* Save and restore the wrapper access window */
	if (BUSTYPE(sii->pub.bustype) == PCI_BUS) {
		if (PCIE_GEN1(sii)) {
			cfg_reg = PCI_BAR0_WIN2;
			offset = PCI_16KB0_WIN2_OFFSET;
		} else if (PCIE_GEN2(sii)) {
			cfg_reg = PCIE2_BAR0_CORE2_WIN2;
			offset = PCIE2_BAR0_CORE2_WIN2_OFFSET;
		}
		else {
			ASSERT(!"!PCIE_GEN1 && !PCIE_GEN2");
		}

		prev_value = OSL_PCI_READ_CONFIG(osh, cfg_reg, 4);
		if (prev_value == 0xFFFFFFFF) {
			SI_PRINT(("%s, PCI_BAR0_WIN2 - %x\n", __FUNCTION__, prev_value));
			return;
		}
	}

#endif /* BCM_BACKPLANE_TIMEOUT */

	for (i = 0; i < sii->axi_num_wrappers; ++i) {

		if (sii->axi_wrapper[i].wrapper_type != AI_SLAVE_WRAPPER) {
			SI_VMSG(("SKIP ENABLE BPT: MFG:%x, CID:%x, ADDR:%x\n",
				sii->axi_wrapper[i].mfg,
				sii->axi_wrapper[i].cid,
				sii->axi_wrapper[i].wrapper_addr));
			continue;
		}

#ifdef BCM_BACKPLANE_TIMEOUT
		if (BUSTYPE(sii->pub.bustype) == PCI_BUS) {
			/* Set BAR0_CORE2_WIN2 to bridge wapper base address */
			OSL_PCI_WRITE_CONFIG(osh,
				cfg_reg, 4, sii->axi_wrapper[i].wrapper_addr);

			/* set AI to BAR0 + Offset corresponding to Gen1 or gen2 */
			ai = (aidmp_t *) ((uint8*)sii->curmap + offset);
		}
		else
#endif /* BCM_BACKPLANE_TIMEOUT */
		{
			ai = (aidmp_t *)(uintptr) sii->axi_wrapper[i].wrapper_addr;
		}

		W_REG(sii->osh, &ai->errlogctrl, (1 << AIELC_TO_ENAB_SHIFT) |
		      ((AXI_TO_VAL << AIELC_TO_EXP_SHIFT) & AIELC_TO_EXP_MASK));

		SI_VMSG(("ENABLED BPT: MFG:%x, CID:%x, ADDR:%x, ERR_CTRL:%x\n",
			sii->axi_wrapper[i].mfg,
			sii->axi_wrapper[i].cid,
			sii->axi_wrapper[i].wrapper_addr,
			R_REG(sii->osh, &ai->errlogctrl)));
	}

#ifdef BCM_BACKPLANE_TIMEOUT
	/* Restore the initial wrapper space */
	if (prev_value) {
		OSL_PCI_WRITE_CONFIG(osh, cfg_reg, 4, prev_value);
	}
#endif /* BCM_BACKPLANE_TIMEOUT */

#endif /* AXI_TIMEOUTS || BCM_BACKPLANE_TIMEOUT */
}

#if defined(AXI_TIMEOUTS) || defined(BCM_BACKPLANE_TIMEOUT)

/* slave error is ignored, so account for those cases */
static uint32 si_ignore_errlog_cnt = 0;

static bool
ai_ignore_errlog(si_info_t *sii, uint32 lo_addr, uint32 hi_addr, uint32 err_axi_id, uint32 errsts)
{
	return FALSE;
}
#endif /* defined (AXI_TIMEOUTS) || defined (BCM_BACKPLANE_TIMEOUT) */

#ifdef BCM_BACKPLANE_TIMEOUT

/* Function to return the APB bridge details corresponding to the core */
bool
ai_get_apb_bridge(si_t * sih, uint32 coreidx, uint32 *apb_id, uint32 * apb_coreuinit)
{
	uint i;
	uint32 core_base, core_end;
	si_info_t *sii = SI_INFO(sih);
	si_cores_info_t *cores_info = (si_cores_info_t *)sii->cores_info;
	static uint32 coreidx_cached = 0, apb_id_cached = 0, apb_coreunit_cached = 0;
	uint32 tmp_coreunit = 0;

	if (coreidx >= MIN(sii->numcores, SI_MAXCORES))
		return FALSE;

	/* Most of the time apb bridge query will be for d11 core.
	 * Maintain the last cache and return if found rather than iterating the table
	 */
	if (coreidx_cached == coreidx) {
		*apb_id = apb_id_cached;
		*apb_coreuinit = apb_coreunit_cached;
		return TRUE;
	}

	core_base = cores_info->coresba[coreidx];
	core_end = core_base + cores_info->coresba_size[coreidx];

	for (i = 0; i < sii->numcores; i++) {
		if (cores_info->coreid[i] == APB_BRIDGE_ID) {
			uint32 apb_base;
			uint32 apb_end;

			apb_base = cores_info->coresba[i];
			apb_end = apb_base + cores_info->coresba_size[i];

			if ((core_base >= apb_base) &&
				(core_end <= apb_end)) {
				/* Current core is attached to this APB bridge */
				*apb_id = apb_id_cached = APB_BRIDGE_ID;
				*apb_coreuinit = apb_coreunit_cached = tmp_coreunit;
				coreidx_cached = coreidx;
				return TRUE;
			}
			/* Increment the coreunit */
			tmp_coreunit++;
		}
	}

	return FALSE;
}

uint32
ai_clear_backplane_to_fast(si_t *sih, void * addr)
{
	si_info_t *sii = SI_INFO(sih);
	void * curmap = sii->curmap;
	bool core_reg = FALSE;

	/* Use fast path only for core register access */
	if ((addr >= curmap) && (addr < (curmap + SI_CORE_SIZE))) {
		/* address being accessed is within current core reg map */
		core_reg = TRUE;
	}

	if (core_reg) {
		uint32 apb_id, apb_coreuinit;

		if (ai_get_apb_bridge(sih, si_coreidx(&sii->pub),
			&apb_id, &apb_coreuinit) == TRUE) {
			/* Found the APB bridge corresponding to current core,
			 * Check for bus errors in APB wrapper
			 */
			return ai_clear_backplane_to_per_core(sih,
				apb_id, apb_coreuinit, NULL);
		}
	}

	/* Default is to poll for errors on all slave wrappers */
	return si_clear_backplane_to(sih);
}
#endif /* BCM_BACKPLANE_TIMEOUT */

#if defined(AXI_TIMEOUTS) || defined(BCM_BACKPLANE_TIMEOUT)
/*
 * API to clear the back plane timeout per core.
 * Caller may passs optional wrapper address. If present this will be used as
 * the wrapper base address. If wrapper base address if provided then caller
 * must provide the coreid also.
 * If both coreid and wrapper is zero, then err status of current bridge
 * will be verified.
 */
uint32
ai_clear_backplane_to_per_core(si_t *sih, uint coreid, uint coreunit, void * wrap)
{
	int ret = AXI_WRAP_STS_NONE;
	aidmp_t *ai = NULL;
	uint32 errlog_status = 0;
	si_info_t *sii = SI_INFO(sih);
	uint32 errlog_lo = 0, errlog_hi = 0, errlog_id = 0, errlog_flags = 0;
	uint32 current_coreidx = si_coreidx(sih);
	uint32 target_coreidx = si_findcoreidx(sih, coreid, coreunit);

	bool restore_core = FALSE;

#ifdef BCM_BACKPLANE_TIMEOUT
	if ((sii->axi_num_wrappers == 0) || (!PCIE(sii)))
#else
	if (sii->axi_num_wrappers == 0)
#endif
	{
		SI_VMSG((" %s, axi_num_wrappers:%d, Is_PCIE:%d, BUS_TYPE:%d, ID:%x\n",
			__FUNCTION__, sii->axi_num_wrappers, PCIE(sii),
			BUSTYPE(sii->pub.bustype), sii->pub.buscoretype));
		return AXI_WRAP_STS_NONE;
	}

	if (wrap != NULL) {
		ai = (aidmp_t *)wrap;
	} else if (coreid && (target_coreidx != current_coreidx)) {
		restore_core = TRUE;

		if (ai_setcoreidx(sih, target_coreidx) == NULL) {
			/* Unable to set the core */
			return ID32_INVALID;
		}

		ai = (aidmp_t *)si_wrapperregs(sih);
	} else {
		/* Read error status of current wrapper */
		ai = (aidmp_t *)si_wrapperregs(sih);
	}

	/* read error log status */
	errlog_status = R_REG(sii->osh, &ai->errlogstatus);

	if (errlog_status == 0xFFFFFFFF) {
		/* Do not try to peek further */
		SI_PRINT(("%s, errlogstatus:%x - Slave Wrapper:%x\n",
			__FUNCTION__, errlog_status, coreid));
		return AXI_WRAP_STS_WRAP_RD_ERR;
	}

	if ((errlog_status & AIELS_TIMEOUT_MASK) != 0) {
		uint32 tmp;
		/* set ErrDone to clear the condition */
		W_REG(sii->osh, &ai->errlogdone, AIELD_ERRDONE_MASK);
		/* SPINWAIT on errlogstatus timeout status bits */
		while ((tmp = R_REG(sii->osh, &ai->errlogstatus)) & AIELS_TIMEOUT_MASK) {
			if (tmp == 0xFFFFFFFF) {
				SI_PRINT(("%s:%d, prev errlogstatus:%x, errlogstatus:%x\n",
					__FUNCTION__, __LINE__, errlog_status, tmp));
				return AXI_WRAP_STS_WRAP_RD_ERR;
			}
		}
		errlog_lo = R_REG(sii->osh, &ai->errlogaddrlo);
		errlog_hi = R_REG(sii->osh, &ai->errlogaddrhi);
		errlog_id = R_REG(sii->osh, &ai->errlogid);
		errlog_flags = R_REG(sii->osh, &ai->errlogflags);

		/* we are already in the error path, so OK to check for the  slave error */
		if (ai_ignore_errlog(sii, errlog_lo, errlog_hi, errlog_id,
			errlog_status)) {
			si_ignore_errlog_cnt++;
			goto end;
		}

		/* only reset APB Bridge on timeout (not slave error, or dec error) */
		switch (errlog_status & AIELS_TIMEOUT_MASK) {
			case AIELS_SLAVE_ERR:
				SI_PRINT(("AXI slave error"));
				ret = AXI_WRAP_STS_SLAVE_ERR;
				break;

			case AIELS_TIMEOUT:
				/* reset APB Bridge */
				OR_REG(sii->osh, &ai->resetctrl, AIRC_RESET);
				/* sync write */
				(void)R_REG(sii->osh, &ai->resetctrl);
				/* clear Reset bit */
				AND_REG(sii->osh, &ai->resetctrl, ~(AIRC_RESET));
				/* sync write */
				(void)R_REG(sii->osh, &ai->resetctrl);
				SI_PRINT(("AXI timeout"));
				ret = AXI_WRAP_STS_TIMEOUT;
				break;

			case AIELS_DECODE:
				SI_PRINT(("AXI decode error"));
				ret = AXI_WRAP_STS_DECODE_ERR;
				break;
			default:
				ASSERT(0);	/* should be impossible */
		}

		SI_PRINT(("\tCoreID: %x\n", coreid));
		SI_PRINT(("\t errlog: lo 0x%08x, hi 0x%08x, id 0x%08x, flags 0x%08x"
			", status 0x%08x\n",
			errlog_lo, errlog_hi, errlog_id, errlog_flags,
			errlog_status));
	}

end:
	if (restore_core) {
		if (ai_setcoreidx(sih, current_coreidx) == NULL) {
			/* Unable to set the core */
			return ID32_INVALID;
		}
	}

	return ret;
}
#endif /* AXI_TIMEOUTS || BCM_BACKPLANE_TIMEOUT */

uint32
ai_clear_backplane_to(si_t *sih)
{
	uint32 ret = 0;
#if defined(AXI_TIMEOUTS) || defined(BCM_BACKPLANE_TIMEOUT)

	si_info_t *sii = SI_INFO(sih);
	aidmp_t *ai;
	uint32 i;

#ifdef BCM_BACKPLANE_TIMEOUT
	uint32 prev_value = 0;
	osl_t *osh = sii->osh;
	uint32 cfg_reg = 0;
	uint32 offset = 0;

	if ((sii->axi_num_wrappers == 0) || (!PCIE(sii)))
#else
	if (sii->axi_num_wrappers == 0)
#endif
	{
		SI_VMSG((" %s, axi_num_wrappers:%d, Is_PCIE:%d, BUS_TYPE:%d, ID:%x\n",
			__FUNCTION__, sii->axi_num_wrappers, PCIE(sii),
			BUSTYPE(sii->pub.bustype), sii->pub.buscoretype));
		return FALSE;
	}

#ifdef BCM_BACKPLANE_TIMEOUT
	/* Save and restore wrapper access window */
	if (BUSTYPE(sii->pub.bustype) == PCI_BUS) {
		if (PCIE_GEN1(sii)) {
			cfg_reg = PCI_BAR0_WIN2;
			offset = PCI_16KB0_WIN2_OFFSET;
		} else if (PCIE_GEN2(sii)) {
			cfg_reg = PCIE2_BAR0_CORE2_WIN2;
			offset = PCIE2_BAR0_CORE2_WIN2_OFFSET;
		}
		else {
			ASSERT(!"!PCIE_GEN1 && !PCIE_GEN2");
		}

		prev_value = OSL_PCI_READ_CONFIG(osh, cfg_reg, 4);

		if (prev_value == 0xFFFFFFFF) {
			SI_PRINT(("%s, PCI_BAR0_WIN2 - %x\n", __FUNCTION__, prev_value));
			return TRUE;
		}
	}
#endif /* BCM_BACKPLANE_TIMEOUT */

	for (i = 0; i < sii->axi_num_wrappers; ++i) {
		uint32 tmp;

		if (sii->axi_wrapper[i].wrapper_type != AI_SLAVE_WRAPPER) {
			continue;
		}

#ifdef BCM_BACKPLANE_TIMEOUT

		if (BUSTYPE(sii->pub.bustype) == PCI_BUS) {
			/* Set BAR0_CORE2_WIN2 to bridge wapper base address */
			OSL_PCI_WRITE_CONFIG(osh,
				cfg_reg, 4, sii->axi_wrapper[i].wrapper_addr);

			/* set AI to BAR0 + Offset corresponding to Gen1 or gen2 */
			ai = (aidmp_t *) ((uint8*)sii->curmap + offset);
		}
		else
#endif /* BCM_BACKPLANE_TIMEOUT */
		{
			ai = (aidmp_t *)(uintptr) sii->axi_wrapper[i].wrapper_addr;
		}

		tmp = ai_clear_backplane_to_per_core(sih, sii->axi_wrapper[i].cid, 0, (void*)ai);
		if (tmp != ID32_INVALID) {
			ret |= tmp;
		} else {
			SI_PRINT(("ERROR: Set Core failed: CID:%x, Unit:0, ai:%p\n",
				sii->axi_wrapper[i].cid, ai));
		}
	}

#ifdef BCM_BACKPLANE_TIMEOUT
	/* Restore the initial wrapper space */
	if (prev_value) {
		OSL_PCI_WRITE_CONFIG(osh, cfg_reg, 4, prev_value);
	}
#endif /* BCM_BACKPLANE_TIMEOUT */

#endif /* AXI_TIMEOUTS || BCM_BACKPLANE_TIMEOUT */

	return ret;
}
