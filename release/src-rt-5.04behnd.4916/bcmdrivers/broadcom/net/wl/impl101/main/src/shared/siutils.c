/*
 * Misc utility routines for accessing chip-specific features
 * of the SiliconBackplane-based Broadcom chips.
 *
 * Copyright (C) 2023, Broadcom. All Rights Reserved.
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
 * $Id: siutils.c 832034 2023-11-01 03:54:25Z $
 */

#include <bcm_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <bcmdevs.h>
#include <hndsoc.h>
#include <sbchipc.h>
#include <sbgci.h>
#include <pcie_core.h>
#if !defined(BCMDONGLEHOST)
#include <pci_core.h>
#include <nicpci.h>
#include <bcmnvram.h>
#include <bcmsrom.h>
#include <hndtcam.h>
#endif /* !defined(BCMDONGLEHOST) */
#ifdef BCMPCIEDEV
#include <pciedev.h>
#endif /* BCMPCIEDEV */
#include <pcicfg.h>
#include <sbsysmem.h>
#include <sbsocram.h>
#if defined(BCMECICOEX) || !defined(BCMDONGLEHOST)
#include <bcmotp.h>
#endif /* BCMECICOEX || !BCMDONGLEHOST */
#include <hndpmu.h>
#ifdef BCMSPI
#include <spid.h>
#endif /* BCMSPI */

#ifdef HNDGCI
#include <hndgci.h>
#endif /* HNDGCI */
#ifdef WLGCIMBHLR
#include <hnd_gci.h>
#endif /* WLGCIMBHLR */

#include <lpflags.h>

#ifdef HNDGIC
#include <hndgic.h>
#endif

#include "siutils_priv.h"
#ifdef SECI_UART
static bool force_seci_clk = 0;
#endif /* SECI_UART */

#define XTAL_FREQ_26000KHZ		26000

/**
 * A set of PMU registers is clocked in the ILP domain, which has an implication on register write
 * behavior: if such a register is written, it takes multiple ILP clocks for the PMU block to absorb
 * the write. During that time the 'SlowWritePending' bit in the PMUStatus register is set.
 */
#define PMUREGS_ILP_SENSITIVE(regoff) \
	((regoff) == OFFSETOF(pmuregs_t, pmutimer) || \
	 (regoff) == OFFSETOF(pmuregs_t, pmuwatchdog) || \
	 (regoff) == OFFSETOF(pmuregs_t, res_req_timer) || \
	 (regoff) == OFFSETOF(pmuregs_t, mac_res_req_timer) || \
	 (regoff) == OFFSETOF(pmuregs_t, mac_res_req_timer1))

#define CHIPCREGS_ILP_SENSITIVE(regoff) \
	((regoff) == OFFSETOF(chipcregs_t, pmutimer) || \
	 (regoff) == OFFSETOF(chipcregs_t, pmuwatchdog) || \
	 (regoff) == OFFSETOF(chipcregs_t, res_req_timer) || \
	 (regoff) == OFFSETOF(chipcregs_t, mac_res_req_timer) || \
	 (regoff) == OFFSETOF(chipcregs_t, mac_res_req_timer1))

#ifndef AXI_TO_VAL
#define AXI_TO_VAL 19
#endif	/* AXI_TO_VAL */

#define DUALSLICE_WLAN_DEV_A	0 /**< prefix in NVRAM file */
#define DUALSLICE_WLAN_DEV_B	1 /**< prefix in NVRAM file */

#if (defined(BCA_CPEROUTER) || defined(BCM_ROUTER)) && !defined(DONGLEBUILD) && \
	defined(CONFIG_BCM947622)
#define BCAWLAN205004
#endif /* (BCA_CPEROUTER || BCM_ROUTER) && !DONGLEBUILD && CONFIG_BCM947622 */

#ifdef BCAWLAN205004
#define SICF_MCLKE              0x0001         /**< Mac core clock Enable */
#define SICF_FCLKON             0x0002         /**< Force clocks On */
#define SICF_FASTCLKRQ          0x0020         /**< bit5, introduced in rev 130 */
#define EMBEDDED2x2AX_D11_MWR0_ADDR 0x84101000 /**< d11 master wrapper #0 backplane addr */
#define EMBEDDED2x2AX_D11_MWR1_ADDR 0x84102000 /**< d11 master wrapper #1 backplane addr */
#define EMBEDDED2x2AX_PMU_ADDR      0x84005000
#define PMU_WD_CNTR_REG_OFFSET      0x634
#define IS_47622A0_SLAVE_SLICE(devid, chiprev, enum_base) \
	(devid == EMBEDDED_2x2AX_ID && chiprev == 0 && enum_base == DUALSLICE_DEV_B_PHYS_ADDR)
#endif /* BCAWLAN205004 */

#ifndef DONGLEBUILD
#define GCI_BLOCK_BASE		0x6000
#define OFFSET_OTP_CTRL		0x314
#define OTP_PWR_DIS		(1<<15)
#endif /* DONGLEBUILD */

/* local prototypes */
static si_info_t *si_doattach(si_info_t *sii, uint devid, osl_t *osh, volatile void *regs,
                              enum bustype_e bustype, void *sdh, char **vars, uint *varsz);
static bool si_buscore_prep(si_info_t *sii, enum bustype_e bustype, uint devid, void *sdh);
static bool si_buscore_setup(si_info_t *sii, chipcregs_t *cc, enum bustype_e bustype,
                             uint32 savewin, uint *origidx, volatile void *regs);
#ifndef DONGLEBUILD
static void si_reset_otp_ctrl(si_t *sih, osl_t *osh, uint devid, volatile void *regs);
#endif /* DONGLEBUILD */

#if !defined(BCMDONGLEHOST)
static void si_nvram_process(si_info_t *sii, char *pvars);

/* dev path concatenation util */
static char *si_devpathvar(si_t *sih, char *var, int len, const char *name);
static char *si_pcie_devpathvar(si_t *sih, char *var, int len, const char *name);
static bool _si_clkctl_cc(si_info_t *sii, uint mode);
static bool si_ispcie(si_info_t *sii);
static uint BCMINITFN(sysmem_banksize)(si_info_t *sii, sysmemregs_t *r, uint8 idx);
static uint BCMINITFN(socram_banksize)(si_info_t *sii, sbsocramregs_t *r, uint8 idx, uint8 mtype);
#if (!defined(_CFE_) && !defined(_CFEZ_)) || defined(CFG_WL)
static void si_gci_get_chipctrlreg_ringidx_base4(uint32 pin, uint32 *regidx, uint32 *pos);
static uint8 si_gci_get_chipctrlreg_ringidx_base8(uint32 pin, uint32 *regidx, uint32 *pos);
static void si_gci_gpio_chipcontrol(si_t *si, uint8 gpoi, uint8 opt);
uint8 si_gci_gpio_status(si_t *sih, uint8 gci_gpio, uint8 mask, uint8 value);
static void si_gci_enable_gpioint(si_t *sih, bool enable);
#endif /* (!_CFE_ && !_CFEZ_) || CFG_WL */
#if defined(BCMECICOEX) || defined(SECI_UART)
static chipcregs_t * seci_set_core(si_t *sih, uint32 *origidx, bool *fast);
#endif
#endif /* !defined(BCMDONGLEHOST) */

static bool si_pmu_is_ilp_sensitive(uint32 idx, uint regoff);

#if defined(DONGLEBUILD)
static char * BCMATTACHFN(si_getkvars)(void);
static int BCMATTACHFN(si_getkvarsz)(void);
#endif /* DONGLEBUILD */

#if !defined(BCMDONGLEHOST)
#if (!defined(_CFE_) && !defined(_CFEZ_)) || defined(CFG_WL)
#if defined(BCMLTECOEX)
static void si_config_gcigpio(si_t *sih, uint32 gci_pos, uint8 gcigpio,
	uint8 gpioctl_mask, uint8 gpioctl_val);
#endif /* BCMLTECOEX */
#endif /* (!_CFE_ && !_CFEZ_) || CFG_WL */
#endif /* !defined(BCMDONGLEHOST) */

/* global variable to indicate reservation/release of gpio's */
static uint32 si_gpioreservation = 0;

/* global flag to prevent shared resources from being initialized multiple times in si_attach() */
static bool si_onetimeinit = FALSE;

/* global kernel resource */
static si_info_t ksii;
static si_cores_info_t ksii_cores_info;

static const char BCMATTACHDATA(rstr_rmin)[] = "rmin";
static const char BCMATTACHDATA(rstr_rmax)[] = "rmax";

/**
 * Allocate an si handle. This function is called multiple times from multiple source files.
 *
 * devid - pci device id (used to determine chip#)
 * osh - opaque OS handle
 * regs - virtual address of initial core registers
 * bustype - pci/sb/sdio/etc
 * @param sdh  opaque pointer pointing to e.g. a Linux 'struct pci_dev'
 * vars - pointer to a to-be created pointer area for "environment" variables. Some callers of this
 *        function set 'vars' to NULL, making dereferencing of this parameter undesired.
 * varsz - pointer to int to return the size of the vars
 */
si_t *
BCMATTACHFN(si_attach)(uint devid, osl_t *osh, volatile void *regs,
                       enum bustype_e bustype, void *sdh, char **vars, uint *varsz)
{
	si_info_t *sii;
	si_cores_info_t *cores_info;
	/* alloc si_info_t */
	/* freed after ucode download for firmware builds */
	if ((sii = MALLOCZ_NOPERSIST(osh, sizeof(si_info_t))) == NULL) {
		SI_ERROR(("si_attach: malloc failed! malloced %d bytes\n", MALLOCED(osh)));
		return (NULL);
	}

#ifdef _RTE_
	cores_info = (si_cores_info_t *)&ksii_cores_info;
#else
	/* alloc si_cores_info_t */
	if ((cores_info = (si_cores_info_t *)MALLOCZ(osh,
		sizeof(si_cores_info_t))) == NULL) {
		SI_ERROR(("si_attach: malloc failed! malloced %d bytes\n", MALLOCED(osh)));
		MFREE(osh, sii, sizeof(si_info_t));
		return (NULL);
	}
#endif
	sii->cores_info = cores_info;

	if (si_doattach(sii, devid, osh, regs, bustype, sdh, vars, varsz) == NULL) {
		MFREE(osh, sii, sizeof(si_info_t));
#ifndef _RTE_
		MFREE(osh, cores_info, sizeof(si_cores_info_t));
#endif
		return (NULL);
	}
	sii->vars = vars ? *vars : NULL;
	sii->varsz = varsz ? *varsz : 0;

	return (si_t *)sii;
}

static uint32	wd_msticks;		/**< watchdog timer ticks normalized to ms */

#ifdef DONGLEBUILD
/**
 * As si_kattach goes thru full srom initialisation same can be used
 * for all subsequent calls
 */
static char *
BCMATTACHFN(si_getkvars)(void)
{
	return (ksii.vars);
}

static int
BCMATTACHFN(si_getkvarsz)(void)
{
	return (ksii.varsz);
}
#endif /* DONGLEBUILD */

/**
 * Returns the backplane address of the chipcommon core for a particular chip.
 * Returns a physical, not a logical address.
 */
uint32
BCMATTACHFN(si_enum_base_pa)(uint devid)
{
#ifndef DONGLEBUILD
	/* NIC/DHD build */
	switch (devid) {
		CASE_BCM43684_CHIP:
		case BCM43684_D11AX_ID:
		case BCM43684_D11AX2G_ID:
		case BCM43684_D11AX5G_ID:
		case BCM43684_D11AX2G6G_ID:
		case BCM43684_D11AX5G6G_ID:
		case BCM43684_D11AX6G_ID:
		case BCM43684_D11AX2G5G6G_ID:
		CASE_BCM6710_CHIP:
		case BCM6710_D11AX_ID:
		case BCM6710_D11AX2G_ID:
		case BCM6710_D11AX5G_ID:
		case BCM6710_D11AX2G6G_ID:
		case BCM6710_D11AX5G6G_ID:
		case BCM6710_D11AX6G_ID:
		case BCM43692_D11AX_ID:
		case BCM43692_D11AX2G_ID:
		case BCM43692_D11AX5G_ID:
		CASE_BCM6715_CHIP:
		case BCM6715_D11AX_ID:
		case BCM6715_D11AX2G_ID:
		case BCM6715_D11AX5G_ID:
		case BCM6715_D11AX2G6G_ID:
		case BCM6715_D11AX5G6G_ID:
		case BCM6715_D11AX6G_ID:
		case BCM6715_D11AX2G5G6G_ID:
		CASE_BCM6717_CHIP:
		case BCM6717_D11BE_ID:
		case BCM6717_D11BE2G_ID:
		case BCM6717_D11BE5G_ID:
		case BCM6717_D11BE2G6G_ID:
		case BCM6717_D11BE5G6G_ID:
		case BCM6717_D11BE6G_ID:
		case BCM6717_D11BE2G5G6G_ID:
		CASE_BCM6726_CHIP:
		case BCM6726_D11BE_ID:
		case BCM6726_D11BE2G_ID:
		case BCM6726_D11BE5G_ID:
		case BCM6726_D11BE2G6G_ID:
		case BCM6726_D11BE5G6G_ID:
		case BCM6726_D11BE6G_ID:
		case BCM6726_D11BE2G5G6G_ID:
		CASE_BCM6711_CHIP:
		case BCM6711_D11BE2G_ID:
			return 0x28000000; /* chips using backplane address 0x2800_0000 */
		case BCM63178_CHIP_ID: //equals EMBEDDED_2x2AX_ID, so don't add separately
		case BCM47622_CHIP_ID:
		case BCM6756_CHIP_ID: // equals EMBEDDED_2x2AX160_ID, so don't add separately
		case EMBEDDED_2x2AX_DEV2G_ID:
		case EMBEDDED_2x2AX_DEV5G_ID:
		case EMBEDDED_2x2AX_DEV6G_ID:
		case EMBEDDED_2x2AX_DEV2G6G_ID:
		case EMBEDDED_2x2AX_DEV5G6G_ID:
		case EMBEDDED_2x2AX160_DEV2G_ID:
		case EMBEDDED_2x2AX160_DEV5G_ID:
		case EMBEDDED_2x2AX160_DEV6G_ID:
		case EMBEDDED_2x2AX160_DEV2G6G_ID:
		case EMBEDDED_2x2AX160_DEV5G6G_ID:
		case EMBEDDED_2x2AX160_DEV2G5G6G_ID:
		case BCM6765_CHIP_ID: // equals EMBEDDED_2x2BE320_ID, so don't add separately
		case EMBEDDED_2x2BE320_DEV2G_ID:
		case EMBEDDED_2x2BE320_DEV5G_ID:
		case EMBEDDED_2x2BE320_DEV6G_ID:
		case EMBEDDED_2x2BE320_DEV2G5G_ID:
		case EMBEDDED_2x2BE320_DEV5G6G_ID:
		case EMBEDDED_2x2BE320_DEV2G6G_ID:
			/* si->enum_base_pa is determined by pseudo pcie bar0 reg instead */
			ASSERT(0);
			break;
	}
#endif /* DONGLEBUILD */

	return SI_ENUM_BASE_DEFAULT;
}

/**
 * Generic kernel variant of si_attach(). Is is called by the full dongle's OS during
 * initialization, or in case of a linux 2.6.36 NIC router build by the router's OS.
 */
si_t *
BCMATTACHFN(si_kattach)(osl_t *osh)
{
	static bool ksii_attached = FALSE;
	si_cores_info_t *cores_info;

	if (!ksii_attached) {
		void *regs = NULL;
		const uint device_id = BCM4710_DEVICE_ID; // pick an arbitrary default device_id

		regs = REG_MAP(si_enum_base_pa(device_id), SI_CORE_SIZE); // map physical to virt
		cores_info = (si_cores_info_t *)&ksii_cores_info;
		ksii.cores_info = cores_info;

#if defined(BCMDONGLEHOST)
		ASSERT(osh);
#endif
		if (si_doattach(&ksii, device_id, osh, regs,
		                SI_BUS, NULL,
		                osh != SI_OSH ? &(ksii.vars) : NULL,
		                osh != SI_OSH ? &(ksii.varsz) : NULL) == NULL) {
			SI_ERROR(("si_kattach: si_doattach failed\n"));
			REG_UNMAP(regs);
			return NULL;
		}
		REG_UNMAP(regs);

		/* save ticks normalized to ms for si_watchdog_ms() */
		/* based on 32KHz ILP clock */
		wd_msticks = 32;

		ksii_attached = TRUE;
		SI_MSG(("si_kattach done. ccrev = %d, wd_msticks = %d\n",
		        CCREV(ksii.pub.ccrev), wd_msticks));
	}

	return &ksii.pub;
} /* si_kattach */

static bool
BCMATTACHFN(si_buscore_prep)(si_info_t *sii, enum bustype_e bustype, uint devid, void *sdh)
{
	BCM_REFERENCE(sdh);
	BCM_REFERENCE(devid);

#if !defined(BCMDONGLEHOST)
	/* kludge to enable the clock on the 4306 which lacks a slowclock */
	if (BUSTYPE(bustype) == PCI_BUS && !si_ispcie(sii))
		si_clkctl_xtal(&sii->pub, XTAL|PLL, ON);
#endif /* !defined(BCMDONGLEHOST) */

	return TRUE;
} /* si_buscore_prep */

uint32
si_get_pmu_reg_addr(si_t *sih, uint32 offset)
{
	si_info_t *sii = SI_INFO(sih);
	uint32 pmuaddr = INVALID_ADDR;
	uint origidx = 0;
	uint pmucoreidx;
	pmuregs_t *pmu;

	SI_MSG(("%s: pmu access, offset: %x\n", __FUNCTION__, offset));
	if (!(sii->pub.cccaps & CC_CAP_PMU)) {
		goto done;
	}

	SI_MSG(("%s: AOBENAB: %x\n", __FUNCTION__, offset));
	origidx = sii->curidx;
	pmucoreidx = si_findcoreidx(&sii->pub, PMU_CORE_ID, 0);
	pmu = si_setcoreidx(&sii->pub, pmucoreidx);
	pmuaddr = (uint32)(uintptr)((volatile uint8*)pmu + offset);
	si_setcoreidx(sih, origidx);

done:
	printf("%s: addrRET: %x\n", __FUNCTION__, pmuaddr);

	return pmuaddr;
}

static bool
BCMATTACHFN(si_buscore_setup)(si_info_t *sii, chipcregs_t *cc, enum bustype_e bustype,
                              uint32 savewin, uint *origidx, volatile void *regs)
{
	si_cores_info_t *cores_info = (si_cores_info_t *)sii->cores_info;
	bool pci, pcie, pcie_gen2 = FALSE;
	uint i;
	uint pciidx, pcieidx, pcirev, pcierev;

#if defined(BCM_BACKPLANE_TIMEOUT) || defined(AXI_TIMEOUTS)
	/* first, enable backplane timeouts */
	si_slave_wrapper_add(&sii->pub);
#endif
	sii->curidx = 0;

	cc = si_setcoreidx(&sii->pub, SI_CC_IDX);
	ASSERT((uintptr)cc);

	/* get chipcommon rev */
	sii->pub.ccrev = (int)si_corerev(&sii->pub);

	/* get chipcommon chipstatus */
	sii->pub.chipst = R_REG(sii->osh, &cc->chipstatus);

	/* get chipcommon capabilites */
	sii->pub.cccaps = R_REG(sii->osh, &cc->capabilities);

	/* get chipcommon extended capabilities */
	sii->pub.cccaps_ext = R_REG(sii->osh, &cc->capabilities_ext);

	/* get pmu rev and caps */
	if (sii->pub.cccaps & CC_CAP_PMU) {
		uint pmucoreidx;
		pmuregs_t *pmu;
		struct si_pub *sih = &sii->pub;

		pmucoreidx = si_findcoreidx(&sii->pub, PMU_CORE_ID, 0);
		pmu = si_setcoreidx(&sii->pub, pmucoreidx);
		sii->pub.pmucaps = R_REG(sii->osh, &pmu->pmucapabilities);
		si_setcoreidx(&sii->pub, SI_CC_IDX);

		sii->pub.gcirev = si_corereg(sih,
				GCI_CORE_IDX(sih),
				GCI_OFFSETOF(sih, gci_corecaps0), 0, 0) & GCI_CAP0_REV_MASK;

		sii->pub.pmurev = sii->pub.pmucaps & PCAP_REV_MASK;
	}

	SI_MSG(("Chipc: rev %d, caps 0x%x, chipst 0x%x pmurev %d, pmucaps 0x%x\n",
		CCREV(sii->pub.ccrev), sii->pub.cccaps, sii->pub.chipst, sii->pub.pmurev,
		sii->pub.pmucaps));

	/* figure out bus/orignal core idx */
	sii->pub.buscoretype = NODEV_CORE_ID;
	sii->pub.buscorerev = (uint)NOREV;
	sii->pub.buscoreidx = BADIDX;

	pci = pcie = FALSE;
	pcirev = pcierev = (uint)NOREV;
	pciidx = pcieidx = BADIDX;

	for (i = 0; i < sii->numcores; i++) {
		uint cid, crev;

		si_setcoreidx(&sii->pub, i);
		cid = si_coreid(&sii->pub);
		crev = si_corerev(&sii->pub);

		/* Display cores found */
		SI_VMSG(("CORE[%d]: id 0x%x rev %d base 0x%x size:%x regs 0x%p\n",
			i, cid, crev, cores_info->coresba_pa[i], cores_info->coresba_size[i],
			OSL_OBFUSCATE_BUF(cores_info->regs[i])));

		if (BUSTYPE(bustype) == SI_BUS) {
#ifdef BCMPCIEDEV_ENABLED
			if (cid == PCIE2_CORE_ID) {
				pcieidx = i;
				pcierev = crev;
				pcie = TRUE;
				pcie_gen2 = TRUE;
			}
#endif
			if (cid == UBUS_BRIDGE_ID) {
				sii->pub.buscorerev = crev;
				sii->pub.buscoretype = cid;
				sii->pub.buscoreidx = i;
			}
		} else if (BUSTYPE(bustype) == PCI_BUS) {
			if (cid == PCI_CORE_ID) {
				pciidx = i;
				pcirev = crev;
				pci = TRUE;
			} else if ((cid == PCIE_CORE_ID) || (cid == PCIE2_CORE_ID)) {
				pcieidx = i;
				pcierev = crev;
				pcie = TRUE;
				if (cid == PCIE2_CORE_ID)
					pcie_gen2 = TRUE;
			}
		}

		/* find the core idx before entering this func. */
		if ((savewin && (savewin == cores_info->coresba_pa[i])) ||
		    (regs == cores_info->regs[i]))
			*origidx = i;
	}

#if !defined(BCMDONGLEHOST)
	if (pci && pcie) {
		if (si_ispcie(sii))
			pci = FALSE;
		else
			pcie = FALSE;
	}
#endif /* !defined(BCMDONGLEHOST) */

#if defined(PCIE_FULL_DONGLE)
	if (pcie) {
		if (pcie_gen2)
			sii->pub.buscoretype = PCIE2_CORE_ID;
		else
			sii->pub.buscoretype = PCIE_CORE_ID;
		sii->pub.buscorerev = pcierev;
		sii->pub.buscoreidx = pcieidx;
	}
	BCM_REFERENCE(pci);
	BCM_REFERENCE(pcirev);
	BCM_REFERENCE(pciidx);
#else
	if (pci) {
		sii->pub.buscoretype = PCI_CORE_ID;
		sii->pub.buscorerev = pcirev;
		sii->pub.buscoreidx = pciidx;
	} else if (pcie) {
		if (pcie_gen2)
			sii->pub.buscoretype = PCIE2_CORE_ID;
		else
			sii->pub.buscoretype = PCIE_CORE_ID;
		sii->pub.buscorerev = pcierev;
		sii->pub.buscoreidx = pcieidx;
	}
#endif /* defined(PCIE_FULL_DONGLE) */

	SI_VMSG(("Buscore id/type/rev %d/0x%x/%d\n", sii->pub.buscoreidx, sii->pub.buscoretype,
	         sii->pub.buscorerev));

#if !defined(BCMDONGLEHOST)
	/* fixup necessary chip/core configurations */
	if (BUSTYPE(sii->pub.bustype) == PCI_BUS) {
		if (SI_FAST(sii)) {
			if (!sii->pch &&
			    ((sii->pch = (void *)(uintptr)pcicore_init(&sii->pub, sii->osh,
				(volatile void *)PCIEREGS(sii))) == NULL))
				return FALSE;
		}
		if (si_pci_fixcfg(&sii->pub)) {
			SI_ERROR(("si_doattach: si_pci_fixcfg failed\n"));
			return FALSE;
		}
	}
#endif /* !defined(BCMDONGLEHOST) */

	/* return to the original core */
	si_setcoreidx(&sii->pub, *origidx);

	return TRUE;
} /* si_buscore_setup */

#if !defined(BCMDONGLEHOST) /* if not a DHD build */

static const char BCMATTACHDATA(rstr_boardvendor)[] = "boardvendor";
static const char BCMATTACHDATA(rstr_boardtype)[] = "boardtype";
static const char BCMATTACHDATA(rstr_subvid)[] = "subvid";
static const char BCMATTACHDATA(rstr_manfid)[] = "manfid";
static const char BCMATTACHDATA(rstr_prodid)[] = "prodid";
static const char BCMATTACHDATA(rstr_boardrev)[] = "boardrev";
static const char BCMATTACHDATA(rstr_boardflags)[] = "boardflags";
static const char BCMATTACHDATA(rstr_boardflags2)[] = "boardflags2";
static const char BCMATTACHDATA(rstr_boardflags4)[] = "boardflags4";
static const char BCMATTACHDATA(rstr_xtalfreq)[] = "xtalfreq";
#ifdef WLLED
static const char BCMATTACHDATA(rstr_leddc)[] = "leddc";
#endif
static const char BCMATTACHDATA(rstr_muxenab)[] = "muxenab";
static const char BCMATTACHDATA(rstr_gpiopulldown)[] = "gpdn";
static const char BCMATTACHDATA(rstr_devid)[] = "devid";
static const char BCMATTACHDATA(rstr_wl0id)[] = "wl0id";
static const char BCMATTACHDATA(rstr_devpathD)[] = "devpath%d";
static const char BCMATTACHDATA(rstr_D_S)[] = "%d:%s";
static const char BCMATTACHDATA(rstr_lpflags)[] = "lpflags";
static const char BCMATTACHDATA(rstr_fuart_pup_rx_cts)[] = "fuart_pup_rx_cts";

static void
BCMATTACHFN(si_nvram_process)(si_info_t *sii, char *pvars)
{
	uint w = 0;

	/* get boardtype and boardrev */
	switch (BUSTYPE(sii->pub.bustype)) {
	case PCI_BUS:
		/* do a pci config read to get subsystem id and subvendor id */
		w = OSL_PCI_READ_CONFIG(sii->osh, PCI_CFG_SVID, sizeof(uint32));

		/* Let nvram variables override subsystem Vend/ID */
		if ((sii->pub.boardvendor = (uint16)si_getdevpathintvar(&sii->pub,
			rstr_boardvendor)) == 0) {
#ifdef BCMHOSTVARS
			if ((w & 0xffff) == 0)
				sii->pub.boardvendor = VENDOR_BROADCOM;
			else
#endif /* !BCMHOSTVARS */
				sii->pub.boardvendor = w & 0xffff;
		} else {
			SI_ERROR(("Overriding boardvendor: 0x%x instead of 0x%x\n",
				sii->pub.boardvendor, w & 0xffff));
		}

		if ((sii->pub.boardtype = (uint16)si_getdevpathintvar(&sii->pub, rstr_boardtype))
			== 0) {
			if ((sii->pub.boardtype = getintvar(pvars, rstr_boardtype)) == 0)
				sii->pub.boardtype = (w >> 16) & 0xffff;
		} else {
			SI_ERROR(("Overriding boardtype: 0x%x instead of 0x%x\n",
				sii->pub.boardtype, (w >> 16) & 0xffff));
		}
		break;

	case SI_BUS:
	case JTAG_BUS:
#ifdef BCMPCIEDEV_SROM_FORMAT
		if (BCMPCIEDEV_ENAB() && si_is_sprom_available(&sii->pub) && pvars &&
			getvar(pvars, rstr_subvid)) {
			sii->pub.boardvendor = getintvar(pvars, rstr_subvid);
		} else
#endif
		sii->pub.boardvendor = VENDOR_BROADCOM;
		if (pvars == NULL || ((sii->pub.boardtype = getintvar(pvars, rstr_prodid)) == 0)) {
			if ((sii->pub.boardtype = getintvar(pvars, rstr_boardtype)) == 0) {
				sii->pub.boardtype = 0xffff;
			}
		}
		break;
	default:
		ASSERT(0);
		break;
	}

	if (sii->pub.boardtype == 0) {
		SI_ERROR(("si_doattach: unknown board type\n"));
		ASSERT(sii->pub.boardtype);
	}

	sii->pub.lpflags = getintvar(pvars, rstr_lpflags);
	sii->pub.boardrev = getintvar(pvars, rstr_boardrev);
	sii->pub.boardflags = getintvar(pvars, rstr_boardflags);
#ifdef WLCX_ATLAS
	sii->pub.boardflags2 = getintvar(pvars, rstr_boardflags2);
#endif

	sii->pub.boardflags4 = getintvar(pvars, rstr_boardflags4);
} /* si_nvram_process */

#endif /* !defined(BCMDONGLEHOST) */

#if defined(CONFIG_XIP) && defined(BCMTCAM)
extern uint8 patch_pair;
#endif /* CONFIG_XIP && BCMTCAM */

#if !defined(BCMDONGLEHOST)
#if (!defined(_CFE_) && !defined(_CFEZ_)) || defined(CFG_WL)
#ifdef SECI_UART
static bool fuart_pullup_rx_cts_enab = FALSE;
static bool fast_uart_init = FALSE;
static uint32 fast_uart_tx;
static uint32 fast_uart_functionsel;
static uint32 fast_uart_pup;
static uint32 fast_uart_rx;
static uint32 fast_uart_cts_in;
#endif /* SECI_UART */

/** want to have this available all the time to switch mux for debugging */
void
BCMATTACHFN(si_muxenab)(si_t *sih, uint32 w)
{
	uint32 chipcontrol, pmu_chipcontrol;

	pmu_chipcontrol = si_pmu_chipcontrol(sih, 1, 0, 0);
	chipcontrol = si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, chipcontrol), 0, 0);

	switch (CHIPID(sih->chip)) {
	CASE_BCM43684_CHIP:
	CASE_BCM6715_CHIP:
	CASE_BCM6717_CHIP:
	CASE_BCM6726_CHIP:
		if (w & MUXENAB_UART) { /* 684 lacks 'super mux' so no si_gci_set_functionsel() */
			si_gci_chipcontrol(sih, CC_GCI_CHIPCTRL_00,
				BCM_BIT(18), BCM_BIT(18)); /* uart_mode=1 */
		}
		break;
	default:
		/* muxenab specified for an unsupported chip */
		ASSERT(0);
		break;
	}

	/* write both updated values to hw */
	si_pmu_chipcontrol(sih, 1, ~0, pmu_chipcontrol);
	si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, chipcontrol), ~0, chipcontrol);
} /* si_muxenab */

/** ltecx GCI reg access */
uint32
si_gci_direct(si_t *sih, uint offset, uint32 mask, uint32 val)
{
	/* gci direct reg access */
	return si_corereg(sih, GCI_CORE_IDX(sih), offset, mask, val);
}

uint32
si_gci_indirect(si_t *sih, uint regidx, uint offset, uint32 mask, uint32 val)
{
	/* gci indirect reg access */
	si_corereg(sih, GCI_CORE_IDX(sih), GCI_OFFSETOF(sih, gci_indirect_addr), ~0, regidx);
	return si_corereg(sih, GCI_CORE_IDX(sih), offset, mask, val);
}

uint32
si_gci_input(si_t *sih, uint reg)
{
	/* gci_input[] */
	return si_corereg(sih, GCI_CORE_IDX(sih), GCI_OFFSETOF(sih, gci_input[reg]), 0, 0);
}

uint32
si_gci_output(si_t *sih, uint reg, uint32 mask, uint32 val)
{
	/* gci_output[] */
	return si_corereg(sih, GCI_CORE_IDX(sih), GCI_OFFSETOF(sih, gci_output[reg]), mask, val);
}

uint32
si_gci_int_enable(si_t *sih, bool enable)
{
	uint offs;

	/* enable GCI interrupt */
	offs = OFFSETOF(chipcregs_t, intmask);
	return (si_corereg(sih, SI_CC_IDX, offs, CI_ECI, (enable ? CI_ECI : 0)));
}

void
si_gci_reset(si_t *sih)
{
	int i;

	/* Set ForceRegClk and ForceSeciClk */
	si_gci_direct(sih, GCI_OFFSETOF(sih, gci_corectrl),
		((1 << GCI_CCTL_FREGCLK_OFFSET)
		|(1 << GCI_CCTL_FSECICLK_OFFSET)),
		((1 << GCI_CCTL_FREGCLK_OFFSET)
		|(1 << GCI_CCTL_FSECICLK_OFFSET)));

	/* Some Delay */
	for (i = 0; i < 2; i++) {
		si_gci_direct(sih, GCI_OFFSETOF(sih, gci_corectrl), 0, 0);
	}
	/* Reset SECI block */
	si_gci_direct(sih, GCI_OFFSETOF(sih, gci_corectrl),
		((1 << GCI_CCTL_SECIRST_OFFSET)
		|(1 << GCI_CCTL_RSTSL_OFFSET)
		|(1 << GCI_CCTL_RSTOCC_OFFSET)),
		((1 << GCI_CCTL_SECIRST_OFFSET)
		|(1 << GCI_CCTL_RSTSL_OFFSET)
		|(1 << GCI_CCTL_RSTOCC_OFFSET)));
	/* Some Delay */
	for (i = 0; i < 10; i++) {
		si_gci_direct(sih, GCI_OFFSETOF(sih, gci_corectrl), 0, 0);
	}
	/* Remove SECI Reset */
	si_gci_direct(sih, GCI_OFFSETOF(sih, gci_corectrl),
		((1 << GCI_CCTL_SECIRST_OFFSET)
		|(1 << GCI_CCTL_RSTSL_OFFSET)
		|(1 << GCI_CCTL_RSTOCC_OFFSET)),
		((0 << GCI_CCTL_SECIRST_OFFSET)
		|(0 << GCI_CCTL_RSTSL_OFFSET)
		|(0 << GCI_CCTL_RSTOCC_OFFSET)));

	/* Some Delay */
	for (i = 0; i < 2; i++) {
		si_gci_direct(sih, GCI_OFFSETOF(sih, gci_corectrl), 0, 0);
	}

	/* Clear ForceRegClk and ForceSeciClk */
	si_gci_direct(sih, GCI_OFFSETOF(sih, gci_corectrl),
		((1 << GCI_CCTL_FREGCLK_OFFSET)
		|(1 << GCI_CCTL_FSECICLK_OFFSET)),
		((0 << GCI_CCTL_FREGCLK_OFFSET)
		|(0 << GCI_CCTL_FSECICLK_OFFSET)));

	/* clear events */
	for (i = 0; i < 32; i++) {
		si_gci_direct(sih, GCI_OFFSETOF(sih, gci_event[i]), ALLONES_32, 0x00);
	}
} /* si_gci_reset */

static void
si_gci_gpio_chipcontrol(si_t *sih, uint8 gci_gpio, uint8 opt)
{
	uint32 ring_idx = 0, pos = 0;

	si_gci_get_chipctrlreg_ringidx_base8(gci_gpio, &ring_idx, &pos);
	SI_MSG(("%s:rngidx is %d, pos is %d, opt is %d, mask is 0x%04x, value is 0x%04x\n",
		__FUNCTION__, ring_idx, pos, opt, GCIMASK_8B(pos), GCIPOSVAL_8B(opt, pos)));

	si_corereg(sih, GCI_CORE_IDX(sih), GCI_OFFSETOF(sih, gci_indirect_addr), ~0, ring_idx);
	si_corereg(sih, GCI_CORE_IDX(sih), GCI_OFFSETOF(sih, gci_gpioctl),
		GCIMASK_8B(pos), GCIPOSVAL_8B(opt, pos));
}

static uint8
si_gci_gpio_reg(si_t *sih, uint8 gci_gpio, uint8 mask, uint8 value, uint32 reg_offset)
{
	uint32 ring_idx = 0, pos = 0; /**< FunctionSel register idx and bits to use */
	uint32 val_32;

	si_gci_get_chipctrlreg_ringidx_base4(gci_gpio, &ring_idx, &pos);
	SI_MSG(("%s:rngidx is %d, pos is %d, val is %d, mask is 0x%04x, value is 0x%04x\n",
		__FUNCTION__, ring_idx, pos, value, GCIMASK_4B(pos), GCIPOSVAL_4B(value, pos)));
	si_corereg(sih, GCI_CORE_IDX(sih), GCI_OFFSETOF(sih, gci_indirect_addr), ~0, ring_idx);

	if (mask || value) {
		/* set operation */
		si_corereg(sih, GCI_CORE_IDX(sih),
			reg_offset, GCIMASK_4B(pos), GCIPOSVAL_4B(value, pos));
	}
	val_32 = si_corereg(sih, GCI_CORE_IDX(sih), reg_offset, 0, 0);

	value  = (uint8)((val_32 >> pos) & 0xFF);

	return value;
}

/**
 * In order to route a ChipCommon originated GPIO towards a package pin, both CC and GCI cores have
 * to be written to.
 * @param[in] sih
 * @param[in] gpio   chip specific package pin number. See Toplevel Arch page, GCI chipcontrol reg
 *                   section.
 * @param[in] mask   chip common gpio mask
 * @param[in] val    chip common gpio value
 */
void
si_gci_enable_gpio(si_t *sih, uint8 gpio, uint32 mask, uint32 value)
{
	uint32 ring_idx = 0, pos = 0;

	si_gci_get_chipctrlreg_ringidx_base4(gpio, &ring_idx, &pos);
	SI_MSG(("%s:rngidx is %d, pos is %d, val is %d, mask is 0x%04x, value is 0x%04x\n",
		__FUNCTION__, ring_idx, pos, value, GCIMASK_4B(pos), GCIPOSVAL_4B(value, pos)));
	si_gci_set_functionsel(sih, gpio, CC4345_FNSEL_SAMEASPIN);
	si_corereg(sih, GCI_CORE_IDX(sih), GCI_OFFSETOF(sih, gci_indirect_addr), ~0, ring_idx);

	si_gpiocontrol(sih, mask, 0, GPIO_HI_PRIORITY);
	si_gpioouten(sih, mask, mask, GPIO_HI_PRIORITY);
	si_gpioout(sih, mask, value, GPIO_HI_PRIORITY);
}

uint8
si_gci_gpio_wakemask(si_t *sih, uint8 gpio, uint8 mask, uint8 value)
{
	si_corereg(sih, GCI_CORE_IDX(sih), GCI_OFFSETOF(sih, gci_wakemask),
		GCI_WAKEMASK_GPIOWAKE, GCI_WAKEMASK_GPIOWAKE);
	return (si_gci_gpio_reg(sih, gpio, mask, value, GCI_OFFSETOF(sih, gci_gpiowakemask)));
}

uint8
si_gci_gpio_intmask(si_t *sih, uint8 gpio, uint8 mask, uint8 value)
{
	return (si_gci_gpio_reg(sih, gpio, mask, value, GCI_OFFSETOF(sih, gci_gpiointmask)));
}

uint8
si_gci_gpio_status(si_t *sih, uint8 gpio, uint8 mask, uint8 value)
{
	return (si_gci_gpio_reg(sih, gpio, mask, value, GCI_OFFSETOF(sih, gci_gpiostatus)));
}

static void
si_gci_enable_gpioint(si_t *sih, bool enable)
{
	if (enable)
		si_corereg(sih, GCI_CORE_IDX(sih), GCI_OFFSETOF(sih, gci_intmask),
			GCI_INTSTATUS_GPIOINT, GCI_INTSTATUS_GPIOINT);
	else
		si_corereg(sih, GCI_CORE_IDX(sih), GCI_OFFSETOF(sih, gci_intmask),
			GCI_INTSTATUS_GPIOINT, 0);
}

/* assumes function select is performed separately */
void
BCMINITFN(si_enable_gpio_wake)(si_t *sih, uint8 *wake_mask, uint8 *cur_status, uint8 gci_gpio,
	uint32 pmu_cc2_mask, uint32 pmu_cc2_value)
{
	si_gci_gpio_chipcontrol(sih, gci_gpio, (1 << GCI_GPIO_CHIPCTRL_ENAB_IN_BIT));

	si_gci_gpio_intmask(sih, gci_gpio, *wake_mask, *wake_mask);
	si_gci_gpio_wakemask(sih, gci_gpio, *wake_mask, *wake_mask);

	/* clear the existing status bits */
	*cur_status = si_gci_gpio_status(sih, gci_gpio, GCI_GPIO_STS_CLEAR, GCI_GPIO_STS_CLEAR);

	/* top level gci int enable */
	si_gci_enable_gpioint(sih, TRUE);

	/* enable the pmu chip control bit to enable wake */
	si_pmu_chipcontrol(sih, PMU_CHIPCTL2, pmu_cc2_mask, pmu_cc2_value);
}

void
si_gci_config_wake_pin(si_t *sih, uint8 gpio_n, uint8 wake_events, bool gci_gpio)
{
	uint8 chipcontrol = 0;
	uint32 pmu_chipcontrol2 = 0;

	if (!gci_gpio)
		chipcontrol = (1 << GCI_GPIO_CHIPCTRL_ENAB_EXT_GPIO_BIT);

	chipcontrol |= (1 << GCI_GPIO_CHIPCTRL_PULLUP_BIT);
	si_gci_gpio_chipcontrol(sih, gpio_n,
		(chipcontrol | (1 << GCI_GPIO_CHIPCTRL_ENAB_IN_BIT)));

	/* enable gci gpio int/wake events */
	si_gci_gpio_intmask(sih, gpio_n, wake_events, wake_events);
	si_gci_gpio_wakemask(sih, gpio_n, wake_events, wake_events);

	/* clear the existing status bits */
	si_gci_gpio_status(sih, gpio_n,
		GCI_GPIO_STS_CLEAR, GCI_GPIO_STS_CLEAR);

	/* Enable gci2wl_wake */
	pmu_chipcontrol2 = si_pmu_chipcontrol(sih, 2, 0, 0);
	/* The next function was removed, because it was only implemented for 4347 and asserts
	 * for any other chip. Likely this gci code can be removed as well....
	 */
	/* pmu_chipcontrol2 |= si_pmu_wake_bit_offset(sih); */
	si_pmu_chipcontrol(sih, 2, ~0, pmu_chipcontrol2);

	/* enable gci int/wake events */
	si_corereg(sih, GCI_CORE_IDX(sih), GCI_OFFSETOF(sih, gci_intmask),
		GCI_INTSTATUS_GPIOINT, GCI_INTSTATUS_GPIOINT);
	si_corereg(sih, GCI_CORE_IDX(sih), GCI_OFFSETOF(sih, gci_wakemask),
		GCI_INTSTATUS_GPIOWAKE, GCI_INTSTATUS_GPIOWAKE);
}

void
si_gci_free_wake_pin(si_t *sih, uint8 gpio_n)
{
	uint8 chipcontrol = 0;
	uint8 wake_events;

	si_gci_gpio_chipcontrol(sih, gpio_n, chipcontrol);

	/* enable gci gpio int/wake events */
	wake_events = si_gci_gpio_intmask(sih, gpio_n, 0, 0);
	si_gci_gpio_intmask(sih, gpio_n, wake_events, 0);
	wake_events = si_gci_gpio_wakemask(sih, gpio_n, 0, 0);
	si_gci_gpio_wakemask(sih, gpio_n, wake_events, 0);

	/* clear the existing status bits */
	si_gci_gpio_status(sih, gpio_n,
		GCI_GPIO_STS_CLEAR, GCI_GPIO_STS_CLEAR);
}

#if defined(BCMPCIEDEV)
static const char BCMINITDATA(rstr_device_wake_opt)[] = "device_wake_opt";
#else
static const char BCMINITDATA(rstr_device_wake_opt)[] = "sd_devwake";
#endif
#define DEVICE_WAKE_GPIO3	3

uint8
BCMATTACHFN(si_enable_perst_wake)(si_t *sih, uint8 *perst_wake_mask, uint8 *perst_cur_status)
{
	uint8  gci_perst = CC_GCI_GPIO_15;
	switch (CHIPID(sih->chip)) {
	default:
		SI_ERROR(("device wake not supported for 0x%04x yet\n", CHIPID(sih->chip)));
		break;
	}
	return gci_perst;

}

uint8
BCMINITFN(si_get_device_wake_opt)(si_t *sih)
{
	si_info_t *sii = SI_INFO(sih);

	if (getvar(NULL, rstr_device_wake_opt) == NULL)
		return CC_GCI_GPIO_INVALID;

	sii->device_wake_opt = (uint8)getintvar(NULL, rstr_device_wake_opt);
	return sii->device_wake_opt;
}

uint8
si_enable_device_wake(si_t *sih, uint8 *wake_mask, uint8 *cur_status)
{
	uint8  gci_gpio = CC_GCI_GPIO_INVALID;		/* DEVICE_WAKE GCI GPIO */
	uint32 device_wake_opt;
	si_info_t *sii = SI_INFO(sih);

	device_wake_opt = sii->device_wake_opt;

	if (device_wake_opt == CC_GCI_GPIO_INVALID) {
		/* parse the device wake opt from nvram */
		/* decode what that means for specific chip */
		/* apply the right gci config */
		/* enable the internal interrupts */
		/* assume: caller already registered handler for that GCI int */
		if (getvar(NULL, rstr_device_wake_opt) == NULL)
			return gci_gpio;

		device_wake_opt = getintvar(NULL, rstr_device_wake_opt);
	}
	switch (CHIPID(sih->chip)) {
	default:
		SI_ERROR(("device wake not supported for 0x%04x yet\n", CHIPID(sih->chip)));
		break;
	}
	return gci_gpio;
} /* si_enable_device_wake */

void
BCMATTACHFN(si_gci_gpioint_handler_unregister)(si_t *sih, void *gci_i)
{
	si_info_t *sii;
	gci_gpio_item_t *p, *n;

	sii = SI_INFO(sih);

	ASSERT(gci_i != NULL);

	sii = SI_INFO(sih);

	if (!(sih->cccaps_ext & CC_CAP_EXT_GCI_PRESENT)) {
		SI_ERROR(("%s: not GCI capable\n", __FUNCTION__));
		return;
	}
	ASSERT(sii->gci_gpio_head != NULL);

	if ((void*)sii->gci_gpio_head == gci_i) {
		sii->gci_gpio_head = sii->gci_gpio_head->next;
		MFREE(sii->osh, gci_i, sizeof(gci_gpio_item_t));
		return;
	} else {
		p = sii->gci_gpio_head;
		n = p->next;
		while (n) {
			if ((void*)n == gci_i) {
				p->next = n->next;
				MFREE(sii->osh, gci_i, sizeof(gci_gpio_item_t));
				return;
			}
			p = n;
			n = n->next;
		}
	}
}

void*
BCMINITFN(si_gci_gpioint_handler_register)(si_t *sih, uint8 gci_gpio, uint8 gpio_status,
	gci_gpio_handler_t cb, void *arg)
{
	si_info_t *sii;
	gci_gpio_item_t *gci_i;

	sii = SI_INFO(sih);

	ASSERT(cb != NULL);

	sii = SI_INFO(sih);

	if (!(sih->cccaps_ext & CC_CAP_EXT_GCI_PRESENT)) {
		SI_ERROR(("%s: not GCI capable\n", __FUNCTION__));
		return NULL;
	}

	SI_MSG(("%s: gci_gpio  is %d\n", __FUNCTION__, gci_gpio));
	if (gci_gpio >= SI_GPIO_MAX) {
		SI_ERROR(("%s: Invalid GCI GPIO NUM %d\n", __FUNCTION__, gci_gpio));
		return NULL;
	}

	gci_i = MALLOC(sii->osh, (sizeof(gci_gpio_item_t)));

	ASSERT(gci_i);
	if (gci_i == NULL) {
		SI_ERROR(("%s: GCI Item MALLOC failure\n", __FUNCTION__));
		return NULL;
	}

	if (sii->gci_gpio_head)
		gci_i->next = sii->gci_gpio_head;
	else
		gci_i->next = NULL;

	sii->gci_gpio_head = gci_i;

	gci_i->handler = cb;
	gci_i->arg = arg;
	gci_i->gci_gpio = gci_gpio;
	gci_i->status = gpio_status;

	return (void *)(gci_i);
}

static void
si_gci_gpioint_handler_process(si_t *sih)
{
	si_info_t *sii;
	uint32 gpio_status[2], status;
	gci_gpio_item_t *gci_i;

	sii = SI_INFO(sih);

	/* most probably there are going to be 1 or 2 GPIOs used this way, so do for each GPIO */

	/* go through the GPIO handlers and call them back if their intstatus is set */
	si_corereg(sih, GCI_CORE_IDX(sih), GCI_OFFSETOF(sih, gci_indirect_addr), ~0, 0);
	gpio_status[0] = si_corereg(sih, GCI_CORE_IDX(sih),
		GCI_OFFSETOF(sih, gci_gpiostatus), 0, 0);
	/* Only clear the status bits that have been read. Other bits (if present) should not
	* get cleared, so that they can be handled later.
	*/
	si_corereg(sih, GCI_CORE_IDX(sih), GCI_OFFSETOF(sih, gci_gpiostatus), ~0, gpio_status[0]);

	si_corereg(sih, GCI_CORE_IDX(sih), GCI_OFFSETOF(sih, gci_indirect_addr), ~0, 1);
	gpio_status[1] = si_corereg(sih, GCI_CORE_IDX(sih),
		GCI_OFFSETOF(sih, gci_gpiostatus), 0, 0);
	/* Only clear the status bits that have been read. Other bits (if present) should not
	* get cleared, so that they can be handled later.
	*/
	si_corereg(sih, GCI_CORE_IDX(sih), GCI_OFFSETOF(sih, gci_gpiostatus), ~0, gpio_status[1]);

	gci_i = sii->gci_gpio_head;

	SI_MSG(("%s: status 0x%04x, 0x%04x\n", __FUNCTION__, gpio_status[0], gpio_status[1]));

	while (gci_i) {
		if (gci_i->gci_gpio < 8)
			status = ((gpio_status[0] >> (gci_i->gci_gpio * 4)) & 0x0F);
		else
			status = ((gpio_status[1] >> ((gci_i->gci_gpio - 8) * 4)) & 0x0F);
		/* should we mask these */
		/* call back */
		ASSERT(gci_i->handler);
		if (gci_i->status & status)
			gci_i->handler(status, gci_i->arg);
		gci_i = gci_i->next;
	}
}

void
si_gci_handler_process(si_t *sih)
{
	uint32 gci_intstatus;

	/* check the intmask, wakemask in the interrupt routine and call the right ones */
	gci_intstatus = si_corereg(sih, GCI_CORE_IDX(sih), GCI_OFFSETOF(sih, gci_intstat), 0, 0);

	if (gci_intstatus & GCI_INTMASK_GPIOINT) {
		SI_MSG(("%s: gci_intstatus is 0x%04x\n", __FUNCTION__, gci_intstatus));
		si_gci_gpioint_handler_process(sih);
	}
	if ((gci_intstatus & ~(GCI_INTMASK_GPIOINT))) {
#ifdef	HNDGCI
		hndgci_handler_process(gci_intstatus, sih);
#endif /* HNDGCI */
	}
#ifdef WLGCIMBHLR
	if (gci_intstatus & GCI_INTSTATUS_EVENT) {
		hnd_gci_mb_handler_process(gci_intstatus, sih);
	}
#endif /* WLGCIMBHLR */
}

/*
 * SECI Init Sequence
 * The following is the basic initialization sequence for SECI:
 * 1. Set the seciClkReq bit of clkCtlStatus register
 * 2. Poll the seciClkAvail bit of clkCtlStatus register until the bit is set
 * 3. Set Enable SECI and ForceLow bit of seciConfig register
 *    GCI_CCTL_FSL_OFFSET|GCI_CCTL_SECIEN_OFFSET
 * 4. Clear the SECI Reset bit of seciConfig register
 * 5. Program the UART registers properly
 * 6. Program SOM field of seciConfig register to the SECI mode
 * 7. Clear the ForceLow bit - GCI_CCTL_FSL_OFFSET
 */
void
si_gci_seci_init(si_t *sih)
{
	uint32 i;
	si_info_t *sii = SI_INFO(sih);
	/*
	 * Use the following to calculate gci_secibauddiv, gci_baudadj, gci_secimcr:
	 *
	 * https://hwnbu-twiki.lvn.broadcom.net/pub/Mwgroup/GciRev0/FastUart_calc_v3.xlsx
	 */
	switch (CHIPID(sih->chip)) {
	case BCM6765_CHIP_ID:
		/* 6765 Configuration
		 * Wireless- SECI_OUT SoC_GPIO 30 -> GCI GPIO 5
		 * Wireless- SECI_IN  SoC_GPIO 22 -> GCI GPIO 6
		 *
		 * GCI rev 15: http://hwnbu-twiki.broadcom.net/bin/view/Mwgroup/GciRev15
		 * Registers now accessed in indirect mode:
		 * SECI_UART_BAUD_RATE_DIVISOR_REG_ADDR   (offset 0x1e0)
		 * SECI_UART_LCR_REG_ADDR (offset 0x1e8)
		 * SECI_UART_MCR_REG_ADDR (offset 0x1ec)
		 * SECI_UART_BAUD_RATE_ADJUSTMENT_REG_ADDR (offset 0x1f8)
		 *
		 * With base clock set to ~52MHz:
		 *
		 * 4mbaud settings (High Rate = 1):
		 * gci_secibauddiv: 0xf3
		 * gci_baudadj:     0x00
		 * gci_secimcr:     0x08
		 */

		/*
		 * Settings based on
		 * https://confluence.broadcom.net/display/WLAN/2x2+11be+bw320+-+PMU+Chip+Control
		 * https://confluence.broadcom.net/display/WLAN/2x2+11be+bw320+-+PMU+PLL+Control
		 */
		/* gci_clk_sel 1=clock derived from HT using divider value */
		si_pmu_chipcontrol(sih, 1, (1u << 23), (1u << 23));

		/* Set divider to attain ~52MHz GCI clock freq */
		si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG4, (0x1ff << 9), (0x70 << 9));

		/* Enable SECI, force clocks and SECI OUT low */
		si_gci_direct(sih, GCI_OFFSETOF(sih, gci_corectrl), ALLONES_32,
			((1 << GCI_CCTL_SECIEN_OFFSET)
			| (1 << GCI_CCTL_FREGCLK_OFFSET)
			| (1 << GCI_CCTL_FSECICLK_OFFSET)
			| (1 << GCI_CCTL_FSL_OFFSET))); // 0x00c

		/* Some Delay for clocks */
		for (i = 0; i < 10; i++) {
			si_gci_direct(sih, GCI_OFFSETOF(sih, gci_corectrl), 0, 0);
		}

		/* 6765 SECI mode is enabled via SoC pinmux, e.g., kernel/dts/6765/96765REF2.dts */

		/* GCI gpio 5 as output enabled (0x02) and 6 as Input enabled (0x01) */
		si_gci_indirect(sih, 1,
			GCI_OFFSETOF(sih, gci_gpioctl), ALLONES_32, 0x00010200); //0x044

		/* 4mbaud */
		si_gci_indirect(sih, 0, GCI_OFFSETOF(sih, gci_secibauddiv), ALLONES_32,
			0xf3); //0x1e0
		si_gci_indirect(sih, 0, GCI_OFFSETOF(sih, gci_baudadj), ALLONES_32, 0x00); //0x1f8
		si_gci_indirect(sih, 0, GCI_OFFSETOF(sih, gci_secimcr), ALLONES_32, 0x08); //0x1ec
		si_gci_direct(sih, GCI_OFFSETOF(sih, gci_secifcr), ALLONES_32, 0x00); //0x1e4
		si_gci_indirect(sih, 0, GCI_OFFSETOF(sih, gci_secilcr), ALLONES_32, 0x00); //0x1e8
		si_gci_direct(sih, GCI_OFFSETOF(sih, gci_seciuartescval), ALLONES_32, 0xDB); //0x1d0
		si_gci_direct(sih, GCI_OFFSETOF(sih, gci_miscctl), ALLONES_32, 0x0F); //0xc54
		/* config GPIO pins 6/5 as SECI_IN/SECI_OUT */
		si_gci_indirect(sih, 0,
			GCI_OFFSETOF(sih, gci_seciin_ctrl), ALLONES_32, 0x161); //0x218
		si_gci_indirect(sih, 0,
			GCI_OFFSETOF(sih, gci_seciout_ctrl), ALLONES_32, 0x10051); //0x21c
		break;

	CASE_BCM6726_CHIP:
		/* 6726 Configuration with 20704 BT chip -
		 * BT Chip gpio 6 - BT SECI <--> Wireless- SECI_OUT WIFI_GPIO 13 -> GCI GPIO 5
		 * BT Chip gpio 7 - BT SECO <--> Wireless- SECI_IN  WIFI_GPIO 14 -> GCI GPIO 6
		 *
		 *  43740/43720 Configuration with 20704 BT chip -
		 * BT Chip gpio 6 - BT SECI <--> Wireless- SECI_OUT WIFI_SPROM_CLK -> GCI GPIO 7
		 * BT Chip gpio 7 - BT SECO <--> Wireless- SECI_IN  WIFI_SPROM_DIN -> GCI GPIO 8
		 *
		 * GCI rev 15: http://hwnbu-twiki.broadcom.net/bin/view/Mwgroup/GciRev15
		 * Registers now accessed in indirect mode:
		 * SECI_UART_BAUD_RATE_DIVISOR_REG_ADDR   (offset 0x1e0)
		 * SECI_UART_LCR_REG_ADDR (offset 0x1e8)
		 * SECI_UART_MCR_REG_ADDR (offset 0x1ec)
		 * SECI_UART_BAUD_RATE_ADJUSTMENT_REG_ADDR (offset 0x1f8)
		 *
		 * With GCI clock set to 44MHz:
		 *
		 * 4mbaud settings (High Rate = 1):
		 * gci_secibauddiv: 0xf5
		 * gci_baudadj:     0x00
		 * gci_secimcr:     0x08
		 */

		/*
		 * Settings based on
		 * https://confluence.broadcom.net/display/WLAN/BCM6716+B0+-+PMU+Chip+Control
		 */
		/* gci_clk_sel 1=clock derived from HT using divider value */
		si_pmu_chipcontrol(sih, 1, (1u << 24), (1u << 24));

		/*
		 * Program pmu PLL control reg10[31:23] to attain 44MHz GCI clock
		 * VCO of high_end pll is 4400MHz. set i_ch3_mdiv of PLL register 10 to 0x64.
		 * VCO of low_end pll is 4800MHz. set i_ch3_mdiv of PLL register 10 to 0x6D.
		 * Settings based on
		 * https://confluence.broadcom.net/display/WLAN/BCM6716+B0+-+PMU+PLL+Control
		 */
		if ((sih->otpflag & OTPFLAG_6726_HI_VOLT_DIS_BIT) == 0) {
			/* "high performance" package */
			SI_PRINT(("%s: BCM6726_CHIP w/4400MHz VCO\n", __FUNCTION__));
			si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG10, (0x1ff << 23), (0x64 << 23));
		} else {
			SI_PRINT(("%s: BCM6726_CHIP w/4800MHz VCO\n", __FUNCTION__));
			si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG10, (0x1ff << 23), (0x6D << 23));
		}
		/* PllCtnlUpdate to commit the changes */
		si_pmu_pllupd(sih);

		/* Enable SECI, force clocks and SECI OUT low */
		si_gci_direct(sih, GCI_OFFSETOF(sih, gci_corectrl), ALLONES_32,
			((1 << GCI_CCTL_SECIEN_OFFSET)
			| (1 << GCI_CCTL_FREGCLK_OFFSET)
			| (1 << GCI_CCTL_FSECICLK_OFFSET)
			| (1 << GCI_CCTL_FSL_OFFSET))); // 0x00c

		/* Some Delay for clocks */
		for (i = 0; i < 10; i++) {
			si_gci_direct(sih, GCI_OFFSETOF(sih, gci_corectrl), 0, 0);
		}

		if (sii->chipnew == BCM43740_CHIP_ID || sii->chipnew == BCM43720_CHIP_ID) {
			SI_ERROR(("%s: 43740/43720 original chipid 0x%x\n", __FUNCTION__,
				sii->chipnew));
			/*
			 * 43740/43720:
			 * GCI chip Control register bit 19, enable bt_seci_on_gpio_mode
			 */
			si_gci_indirect(sih, 0,
				GCI_OFFSETOF(sih, gci_chipctrl), ALLONES_32, (1u << 19)); //0x200
			/* GCI gpio 7 as output enabled (0x02) and 8 as Input enabled (0x01) */
			si_gci_indirect(sih, 1,
				GCI_OFFSETOF(sih, gci_gpioctl), ALLONES_32, 0x02000000); //0x044
			si_gci_indirect(sih, 2,
				GCI_OFFSETOF(sih, gci_gpioctl), ALLONES_32, 0x00000001); //0x044
			/* config GPIO pins 8/7 as SECI_IN/SECI_OUT */
			si_gci_indirect(sih, 0,
				GCI_OFFSETOF(sih, gci_seciin_ctrl), ALLONES_32, 0x181); //0x218
			si_gci_indirect(sih, 0,
				GCI_OFFSETOF(sih, gci_seciout_ctrl), ALLONES_32, 0x10071); //0x21c
		} else {
			/*
			 * 6726:
			 * GCI chip Control register bit 20, enable bt_seci_on_gpio_mode
			 */
			si_gci_indirect(sih, 0,
				GCI_OFFSETOF(sih, gci_chipctrl), ALLONES_32, (1u << 20)); //0x200
			/* GCI gpio 5 as output enabled (0x02) and 6 as Input enabled (0x01) */
			si_gci_indirect(sih, 1,
				GCI_OFFSETOF(sih, gci_gpioctl), ALLONES_32, 0x00010200); //0x044
			/* config GPIO pins 6/5 as SECI_IN/SECI_OUT */
			si_gci_indirect(sih, 0,
				GCI_OFFSETOF(sih, gci_seciin_ctrl), ALLONES_32, 0x161); //0x218
			si_gci_indirect(sih, 0,
				GCI_OFFSETOF(sih, gci_seciout_ctrl), ALLONES_32, 0x10051); //0x21c
		}

		/* 4mbaud */
		si_gci_indirect(sih, 0, GCI_OFFSETOF(sih, gci_secibauddiv), ALLONES_32,
			0xf5); //0x1e0
		si_gci_indirect(sih, 0, GCI_OFFSETOF(sih, gci_baudadj), ALLONES_32, 0x00); //0x1f8
		si_gci_indirect(sih, 0, GCI_OFFSETOF(sih, gci_secimcr), ALLONES_32, 0x08); //0x1ec
		si_gci_direct(sih, GCI_OFFSETOF(sih, gci_secifcr), ALLONES_32, 0x00); //0x1e4
		si_gci_indirect(sih, 0, GCI_OFFSETOF(sih, gci_secilcr), ALLONES_32, 0x00); //0x1e8
		si_gci_direct(sih, GCI_OFFSETOF(sih, gci_seciuartescval), ALLONES_32, 0xDB); //0x1d0
		si_gci_direct(sih, GCI_OFFSETOF(sih, gci_miscctl), ALLONES_32, 0x0F); //0xc54
		break;

	CASE_BCM6717_CHIP:
		/* 6717 Configuration with 20704 BT chip -
		 * BT Chip gpio 6 - BT SECI <--> Wireless- SECI_OUT WIFI_GPIO 13 -> GCI GPIO 5
		 * BT Chip gpio 7 - BT SECO <--> Wireless- SECI_IN  WIFI_GPIO 14 -> GCI GPIO 6
		 *
		 * GCI rev 15: http://hwnbu-twiki.broadcom.net/bin/view/Mwgroup/GciRev15
		 * Registers now accessed in indirect mode:
		 * SECI_UART_BAUD_RATE_DIVISOR_REG_ADDR   (offset 0x1e0)
		 * SECI_UART_LCR_REG_ADDR (offset 0x1e8)
		 * SECI_UART_MCR_REG_ADDR (offset 0x1ec)
		 * SECI_UART_BAUD_RATE_ADJUSTMENT_REG_ADDR (offset 0x1f8)
		 *
		 * With GCI clock set to 44MHz:
		 *
		 * 4mbaud settings (High Rate = 1):
		 * gci_secibauddiv: 0xf5
		 * gci_baudadj:     0x00
		 * gci_secimcr:     0x08
		 */

		/*
		 * Settings based on
		 * https://confluence.broadcom.net/display/WLAN/BCM6717+A0+-+PMU+Chip+Control
		 */
		/* gci_clk_sel 1=clock derived from HT using divider value */
		si_pmu_chipcontrol(sih, 1, (1u << 24), (1u << 24));
		/* enable GCI clock divider counter */
		si_pmu_chipcontrol(sih, 5, (1u << 29), (1u << 29));
		/*
		 * set the divider value so GCI clock is 44MHz:
		 * GCI clk freq = (HT freq)/(pmu_chip_cntrl[199:192] + 1)
		 * 6717 HT is 2600, so GCI clock is 2600/(58+1) = ~44MHz
		 */
		si_pmu_chipcontrol(sih, 6, 0xff, 58);

		/* Enable SECI, force clocks and SECI OUT low */
		si_gci_direct(sih, GCI_OFFSETOF(sih, gci_corectrl), ALLONES_32,
			((1 << GCI_CCTL_SECIEN_OFFSET)
			| (1 << GCI_CCTL_FREGCLK_OFFSET)
			| (1 << GCI_CCTL_FSECICLK_OFFSET)
			| (1 << GCI_CCTL_FSL_OFFSET))); // 0x00c

		/* Some Delay for clocks */
		for (i = 0; i < 10; i++) {
			si_gci_direct(sih, GCI_OFFSETOF(sih, gci_corectrl), 0, 0);
		}

		/*
		 * GCI chip Control register bit 20, enable bt_seci_on_gpio_mode
		 */
		si_gci_indirect(sih, 0,
			GCI_OFFSETOF(sih, gci_chipctrl), ALLONES_32, (1u << 20)); //0x200

		/* GCI gpio 5 as output enabled (0x02) and 6 as Input enabled (0x01) */
		si_gci_indirect(sih, 1,
			GCI_OFFSETOF(sih, gci_gpioctl), ALLONES_32, 0x00010200); //0x044

		/* 4mbaud */
		si_gci_indirect(sih, 0, GCI_OFFSETOF(sih, gci_secibauddiv), ALLONES_32,
			0xf5); //0x1e0
		si_gci_indirect(sih, 0, GCI_OFFSETOF(sih, gci_baudadj), ALLONES_32, 0x00); //0x1f8
		si_gci_indirect(sih, 0, GCI_OFFSETOF(sih, gci_secimcr), ALLONES_32, 0x08); //0x1ec
		si_gci_direct(sih, GCI_OFFSETOF(sih, gci_secifcr), ALLONES_32, 0x00); //0x1e4
		si_gci_indirect(sih, 0, GCI_OFFSETOF(sih, gci_secilcr), ALLONES_32, 0x00); //0x1e8
		si_gci_direct(sih, GCI_OFFSETOF(sih, gci_seciuartescval), ALLONES_32, 0xDB); //0x1d0
		si_gci_direct(sih, GCI_OFFSETOF(sih, gci_miscctl), ALLONES_32, 0x0F); //0xc54

		/* config GPIO pins 6/5 as SECI_IN/SECI_OUT */
		si_gci_indirect(sih, 0,
			GCI_OFFSETOF(sih, gci_seciin_ctrl), ALLONES_32, 0x161); //0x218
		si_gci_indirect(sih, 0,
			GCI_OFFSETOF(sih, gci_seciout_ctrl), ALLONES_32, 0x10051); //0x21c
		break;

	CASE_BCM6710_CHIP:
		/* 6710 Configuration with 20704 BT chip -
		 * BT Chip gpio 6 - BT SECI <--> Wireless- SECI_OUT WIFI_GPIO 3 -> GCI GPIO 5
		 * BT Chip gpio 7 - BT SECO <--> Wireless- SECI_IN  WIFI_GPIO 4 -> GCI GPIO 6
		 *
		 * GCI rev 15: http://hwnbu-twiki.broadcom.net/bin/view/Mwgroup/GciRev15
		 * Registers now accessed in indirect mode:
		 * SECI_UART_BAUD_RATE_DIVISOR_REG_ADDR   (offset 0x1e0)
		 * SECI_UART_LCR_REG_ADDR (offset 0x1e8)
		 * SECI_UART_MCR_REG_ADDR (offset 0x1ec)
		 * SECI_UART_BAUD_RATE_ADJUSTMENT_REG_ADDR (offset 0x1f8)
		 */

		/* Enable SECI, force clocks and SECI OUT low */
		si_gci_direct(sih, GCI_OFFSETOF(sih, gci_corectrl), ALLONES_32,
			((1 << GCI_CCTL_SECIEN_OFFSET)
			| (1 << GCI_CCTL_FREGCLK_OFFSET)
			| (1 << GCI_CCTL_FSECICLK_OFFSET)
			| (1 << GCI_CCTL_FSL_OFFSET))); // 0x00c

		/* Some Delay for clocks */
		for (i = 0; i < 10; i++) {
			si_gci_direct(sih, GCI_OFFSETOF(sih, gci_corectrl), 0, 0);
		}

		/* 6710 GCI chip Control register bit 20, enable bt_seci_on_gpio_mode
		 */
		si_gci_indirect(sih, 0,
			GCI_OFFSETOF(sih, gci_chipctrl), ALLONES_32, (1u << 20)); //0x200

		/* GCI gpio 5 as output enabled (0x02) and 6 as Input enabled (0x01) */
		si_gci_indirect(sih, 1,
			GCI_OFFSETOF(sih, gci_gpioctl), ALLONES_32, 0x00010200); //0x044

		/*
		 * 6710 50MHz xtal; 20704 xtal is 48Mhz; the greatest common factor is 2000
		 */
		/* 2mbaud */
		si_gci_indirect(sih, 0, GCI_OFFSETOF(sih, gci_secibauddiv), ALLONES_32,
			0xFF); //0x1e0
		si_gci_indirect(sih, 0, GCI_OFFSETOF(sih, gci_baudadj), ALLONES_32, 0x54); //0x1f8
		si_gci_direct(sih, GCI_OFFSETOF(sih, gci_secifcr), ALLONES_32, 0x00); //0x1e4
		si_gci_indirect(sih, 0, GCI_OFFSETOF(sih, gci_secimcr), ALLONES_32, 0x80); //0x1ec
		si_gci_indirect(sih, 0, GCI_OFFSETOF(sih, gci_secilcr), ALLONES_32, 0x00); //0x1e8
		si_gci_direct(sih, GCI_OFFSETOF(sih, gci_seciuartescval), ALLONES_32, 0xDB); //0x1d0
		si_gci_direct(sih, GCI_OFFSETOF(sih, gci_miscctl), ALLONES_32, 0x0F); //0xc54

		/* config GPIO pins 6/5 as SECI_IN/SECI_OUT */
		si_gci_indirect(sih, 0,
			GCI_OFFSETOF(sih, gci_seciin_ctrl), ALLONES_32, 0x161); //0x218
		si_gci_indirect(sih, 0,
			GCI_OFFSETOF(sih, gci_seciout_ctrl), ALLONES_32, 0x10051); //0x21c
		break;

	default:
		si_gci_direct(sih, GCI_OFFSETOF(sih, gci_corectrl), ALLONES_32,
		(GCI_CCTL_SCS << GCI_CCTL_SCS_OFFSET)
		| ((GCI_MODE_SECI << GCI_CCTL_SMODE_OFFSET)
			| (1 << GCI_CCTL_SECIEN_OFFSET)));

		/* 4365 chip control settigns */
		si_gci_indirect(sih, 0,
			GCI_OFFSETOF(sih, gci_chipctrl), ALLONES_32, 0x0080000); //0x200

		si_gci_indirect(sih, 1,
		GCI_OFFSETOF(sih, gci_gpioctl), ALLONES_32, 0x00010280); //0x044

		/* baudrate:4Mbps at 40MHz xtal, escseq:0xdb, high baudrate, enable seci_tx/rx */
		si_gci_direct(sih, GCI_OFFSETOF(sih, gci_secibauddiv), //0x1e0
			ALLONES_32, 0xF6);
		si_gci_direct(sih, GCI_OFFSETOF(sih, gci_baudadj), ALLONES_32, 0xFF); //0x1f8
		si_gci_direct(sih, GCI_OFFSETOF(sih, gci_secifcr), ALLONES_32, 0x00); //0x1e4
		si_gci_direct(sih, GCI_OFFSETOF(sih, gci_secimcr), ALLONES_32, 0x08); //0x1ec
		si_gci_direct(sih, GCI_OFFSETOF(sih, gci_secilcr), ALLONES_32, 0xA8); //0x1e8
		si_gci_direct(sih, GCI_OFFSETOF(sih, gci_seciuartescval), //0x1d0
			ALLONES_32, 0xDB);
		/* Atlas/GMAC3 configuration for SECI */
		si_gci_direct(sih, GCI_OFFSETOF(sih, gci_miscctl), ALLONES_32, 0xFFFF); //0xc54

		/* config GPIO pins 6/5 as SECI_IN/SECI_OUT */
		si_gci_indirect(sih, 0,
			GCI_OFFSETOF(sih, gci_seciin_ctrl), ALLONES_32, 0x161); //0x218
		si_gci_indirect(sih, 0,
			GCI_OFFSETOF(sih, gci_seciout_ctrl), ALLONES_32, 0x10051); //0x21c
		break;
	}

	si_gci_direct(sih, GCI_OFFSETOF(sih, gci_seciout_txen_txbr), ALLONES_32, 0x01); //0x224

	/* WLAN rx offset assignment */
	/* WLCX: RX offset assignment from WLAN core to WLAN core (faked as BT TX) */
	si_gci_indirect(sih, 0,
		GCI_OFFSETOF(sih, gci_secif0rx_offset), ALLONES_32, 0x13121110); //0x1bc
	si_gci_indirect(sih, 1,
		GCI_OFFSETOF(sih, gci_secif0rx_offset), ALLONES_32, 0x17161514);
	si_gci_indirect(sih, 2,
		GCI_OFFSETOF(sih, gci_secif0rx_offset), ALLONES_32, 0x1b1a1918);

	/* first 12 bits configured for format-0 */
	/* note: we can only select 1st 12 bits of each IP for format_0 */
	si_gci_indirect(sih, 0,  GCI_OFFSETOF(sih, gci_seciusef0tx_reg), //0x1b4
	                ALLONES_32, 0xFFF); // first 12 bits

	si_gci_indirect(sih, 0,  GCI_OFFSETOF(sih, gci_secitx_datatag),
			ALLONES_32, 0x0F0); // gci_secitx_datatag(bits 4 to 7 tagged)
	si_gci_indirect(sih, 0,  GCI_OFFSETOF(sih, gci_secirx_datatag),
	                ALLONES_32, 0x0F0); // gci_secirx_datatag(bits 4 to 7 tagged)

	/* TX offset assignment (wlan to bt) */
	si_gci_indirect(sih, 0,
		GCI_OFFSETOF(sih, gci_secif0tx_offset), 0xFFFFFFFF, 0x76543210); //0x1b8
	si_gci_indirect(sih, 1,
		GCI_OFFSETOF(sih, gci_secif0tx_offset), 0xFFFFFFFF, 0x0000ba98);

	// HW ECI bus directly driven from IP
	si_gci_direct(sih, GCI_OFFSETOF(sih, gci_control_0), ALLONES_32, 0x00000000);
	si_gci_direct(sih, GCI_OFFSETOF(sih, gci_control_1), ALLONES_32, 0x00000000);

	/* Clear force-low and clock forcing bits */
	si_gci_direct(sih, GCI_OFFSETOF(sih, gci_corectrl),
		((1 << GCI_CCTL_FREGCLK_OFFSET)
		| (1 << GCI_CCTL_FSECICLK_OFFSET)
		| (1 << GCI_CCTL_FSL_OFFSET)),
		0); // 0x00c
} /* si_gci_seci_init */

void
si_gci_3wire_init(si_t *sih, uint8 btc_wlan_gpio, uint8 btc_bt_gpio, uint8 btc_stat_gpio)
{
	uint32 mask = 0;

	/* Bits implemented in GCI GPIO mask Registers are:
	 * GPIO_MASK is supported only for lower 5 GPIOs (GCI_GPIO[4:0]).
	 * Note the 5 GPIO_MASK can be used to form the BT_SIG Type 0 message for
	 * TX.
	 */
	/* GCI chip Control register bit 22, enable gci gpio mode
	 * (gci_chip_cntrl(22))
	 */
	si_gci_indirect(sih, 0,
			GCI_OFFSETOF(sih, gci_chipctrl), ALLONES_32, 0x0400000);

	/* Write GCI_MISC_CONTROL_REG_ADDR to set btcx_txconf only from D11
	 * overrides wl2gci_hw[0] (clear gci_misc_cntrl(4))
	 */
	si_gci_indirect(sih, 0,
			GCI_OFFSETOF(sih, gci_miscctl), ALLONES_32, 0x00ff0f);

	/* Program the input (0x01) or output (0x02) direction of the GPIOs */
	mask =	(0x02 << ((btc_wlan_gpio - 1) * 8)) |
		(0x01 << ((btc_bt_gpio - 1) * 8)) |
		(0x01 << ((btc_stat_gpio - 1) * 8));
	si_gci_indirect(sih, 0,
			GCI_OFFSETOF(sih, gci_gpioctl), ALLONES_32, mask);

	/* Map WLAN GCI HW bit 0 (TxConf) to GCI_GPIO[btc_wlan_gpio-1]
	 * Write GCI_INDIRECT_ADDRESS_REG_ADDR: upper 16 is btc_wlan_gpio # and
	 * lower 16 is reg_index = 0x00 (WLAN GCI HW bus)
	 * http://hwnbu-twiki.sj.broadcom.com/bin/view/Mwgroup/GciRev0#GCI
	 * gci_gpiomask  0x1 represents mask for bit 0 (TXCONF) of BT GCI
	 * HW bus
	 * http://hwnbu-twiki.broadcom.com/bin/view/Mwgroup/
	 * GlobalCoexInterfaceBitDefinition#From_WLAN
	 */
	mask = ((btc_wlan_gpio - 1) << 16);
	si_gci_indirect(sih, mask,
			GCI_OFFSETOF(sih, gci_gpiomask), 0x00000001, 0x00000001);

	/* Map WLAN GCI HW bit 0 (BTCX_RF_ACTIVE) to GCI_GPIO[btc_bt_gpio-1]
	 * Write GCI_INDIRECT_ADDRESS_REG_ADDR: upper 16 is btc_bt_gpio #  and
	 * lower 16 reg_index = 0x04 (BT GCI HW bus)
	 * http://hwnbu-twiki.sj.broadcom.com/bin/view/Mwgroup/GciRev0#GCI
	 * gci_gpiomask 0x1 represents mask for bit 0 (BTCX_RF_ACTIVE) of BT GCI
	 * HW bus
	 * http://hwnbu-twiki.broadcom.com/bin/view/Mwgroup/
	 * GlobalCoexInterfaceBitDefinition#From_BT
	 */
	mask = ((btc_bt_gpio - 1) << 16) | 0x4;
	si_gci_indirect(sih, mask,
			GCI_OFFSETOF(sih, gci_gpiomask), 0x00000001, 0x00000001);

	/* Map BT GCI HW bus bit 1 (BTCX_STATUS) to GCI_GPIO[btc_stat_gpio-1]
	 * Write GCI_INDIRECT_ADDRESS_REG_ADDR: upper 16 is btc_stat_gpio #  and
	 * lower 16 is is reg_index = 0x04 (BT GCI HW bus)
	 * http://hwnbu-twiki.sj.broadcom.com/bin/view/Mwgroup/GciRev0#GCI
	 * gci_gpiomask 0x2 represents mask for bit 1 (BTCX_STATUS) of BT GCI
	 * HW bus
	 * http://hwnbu-twiki.broadcom.com/bin/view/Mwgroup/
	 * GlobalCoexInterfaceBitDefinition#From_BT
	 */
	mask = ((btc_stat_gpio - 1) << 16) | 0x4;
	si_gci_indirect(sih, mask,
			GCI_OFFSETOF(sih, gci_gpiomask), 0x00000002, 0x00000002);
}

int
si_gci_seci_write_byte(si_t *sih, uint32 write_byte)
{
	int status = BCME_OK;
	uint32 value = 0;
	uint32 breakout_count = 10;

	/* before writing to aux_tx register, make sure earlier write transaction is done */
	value = si_gci_direct(sih, GCI_OFFSETOF(sih, gci_seciauxtx), 0, 0);

	/* Don't write if previous write transaction is pending */
	while (value & SECI_AUX_TX_START)
	{
		/* keep checking ... */
		value = si_gci_direct(sih, GCI_OFFSETOF(sih, gci_seciauxtx), 0, 0);
		breakout_count--;
		if (breakout_count == 0) {
			/* just break out so that we won't be here forever */
			return BCME_NOTREADY;
		}
	}

	write_byte &= 0xFF;
	/* set start_busy bit */
	write_byte |= SECI_AUX_TX_START;
	/* send byte out */
	si_gci_direct(sih, GCI_OFFSETOF(sih, gci_seciauxtx), ALLONES_32, write_byte);
	return status;
}

void
si_gci_exchange_eci_info(si_t *sih, uint32 btc_mode)
{
	si_seci_clkreq(sih, TRUE);
	/*
	 * Force GCI to transmit the info even if the bits just written are
	 * the same as what they were before. This ensures BT gets updated
	 * in case BT was reset, came up after WLAN, etc.
	 */
	/* sync the operation */
	SPINWAIT((si_gci_direct(sih, GCI_OFFSETOF(sih, gci_corectrl),
		0, 0) & SECI_UPD_SECI), 1000);
	/* WLAN to update SECI information */
	si_gci_direct(sih, GCI_OFFSETOF(sih, gci_corectrl),
		SECI_UPD_SECI, SECI_UPD_SECI);
	/* sync the operation */
	SPINWAIT((si_gci_direct(sih, GCI_OFFSETOF(sih, gci_corectrl),
		0, 0) & SECI_UPD_SECI), 1000);
	if (btc_mode) {
		OSL_DELAY(5);
		/* send out Refresh Request (0xDA) to BT */
		si_gci_seci_write_byte(sih, (uint32) 0xDA);
	}
}

#ifdef BCMLTECOEX
void
si_ercx_init(si_t *sih, uint32 ltecx_mux, uint32 ltecx_padnum,
	uint32 ltecx_fnsel, uint32 ltecx_gcigpio)
{
	uint8 fsync_padnum, lterx_padnum, ltetx_padnum, wlprio_padnum;
	uint8 fsync_fnsel, lterx_fnsel, ltetx_fnsel, wlprio_fnsel;
	uint8 fsync_gcigpio, lterx_gcigpio, ltetx_gcigpio, wlprio_gcigpio;

	/* reset GCI block */
	si_gci_reset(sih);

	/* enable ERCX (pure gpio) mode, Keep SECI in Reset Mode Only */
	/* Hopefully, keeping SECI in Reset Mode will draw lesser current */
	si_gci_direct(sih, GCI_OFFSETOF(sih, gci_corectrl),
		((GCI_MODE_MASK << GCI_CCTL_SMODE_OFFSET)
		|(1 << GCI_CCTL_SECIEN_OFFSET)
		|(1 << GCI_CCTL_RSTSL_OFFSET)
		|(1 << GCI_CCTL_SECIRST_OFFSET)),
		((GCI_MODE_GPIO << GCI_CCTL_SMODE_OFFSET)
		|(0 << GCI_CCTL_SECIEN_OFFSET)
		|(1 << GCI_CCTL_RSTSL_OFFSET)
		|(1 << GCI_CCTL_SECIRST_OFFSET)));

	/* Extract Interface Configuration */
	fsync_padnum	= LTECX_EXTRACT_PADNUM(ltecx_padnum, LTECX_NVRAM_FSYNC_IDX);
	lterx_padnum	= LTECX_EXTRACT_PADNUM(ltecx_padnum, LTECX_NVRAM_LTERX_IDX);
	ltetx_padnum	= LTECX_EXTRACT_PADNUM(ltecx_padnum, LTECX_NVRAM_LTETX_IDX);
	wlprio_padnum	= LTECX_EXTRACT_PADNUM(ltecx_padnum, LTECX_NVRAM_WLPRIO_IDX);

	fsync_fnsel	= LTECX_EXTRACT_FNSEL(ltecx_fnsel, LTECX_NVRAM_FSYNC_IDX);
	lterx_fnsel	= LTECX_EXTRACT_FNSEL(ltecx_fnsel, LTECX_NVRAM_LTERX_IDX);
	ltetx_fnsel	= LTECX_EXTRACT_FNSEL(ltecx_fnsel, LTECX_NVRAM_LTETX_IDX);
	wlprio_fnsel	= LTECX_EXTRACT_FNSEL(ltecx_fnsel, LTECX_NVRAM_WLPRIO_IDX);

	fsync_gcigpio	= LTECX_EXTRACT_GCIGPIO(ltecx_gcigpio, LTECX_NVRAM_FSYNC_IDX);
	lterx_gcigpio	= LTECX_EXTRACT_GCIGPIO(ltecx_gcigpio, LTECX_NVRAM_LTERX_IDX);
	ltetx_gcigpio	= LTECX_EXTRACT_GCIGPIO(ltecx_gcigpio, LTECX_NVRAM_LTETX_IDX);
	wlprio_gcigpio	= LTECX_EXTRACT_GCIGPIO(ltecx_gcigpio, LTECX_NVRAM_WLPRIO_IDX);

	/* Clear this Function Select for all GPIOs if programmed by default */
	si_gci_clear_functionsel(sih, fsync_fnsel);
	si_gci_clear_functionsel(sih, lterx_fnsel);
	si_gci_clear_functionsel(sih, ltetx_fnsel);
	si_gci_clear_functionsel(sih, wlprio_fnsel);

	/* Program Function select for selected GPIOs */
	si_gci_set_functionsel(sih, fsync_padnum, fsync_fnsel);
	si_gci_set_functionsel(sih, lterx_padnum, lterx_fnsel);
	si_gci_set_functionsel(sih, ltetx_padnum, ltetx_fnsel);
	si_gci_set_functionsel(sih, wlprio_padnum, wlprio_fnsel);

	/* NOTE: We are keeping Input PADs in Pull Down Mode to take care of the case
	 * when LTE Modem doesn't drive these lines for any reason.
	 * We should consider alternate ways to identify this situation and dynamically
	 * enable Pull Down PAD only when LTE Modem doesn't drive these lines.
	 */

	/* Configure Frame Sync as input */
	si_config_gcigpio(sih, GCI_LTE_FRAMESYNC_POS, fsync_gcigpio, 0xFF,
		((1 << GCI_GPIOCTL_INEN_OFFSET)|(1 << GCI_GPIOCTL_PDN_OFFSET)));

	/* Configure LTE Rx as input */
	si_config_gcigpio(sih, GCI_LTE_RX_POS, lterx_gcigpio, 0xFF,
		((1 << GCI_GPIOCTL_INEN_OFFSET)|(1 << GCI_GPIOCTL_PDN_OFFSET)));

	/* Configure LTE Tx as input */
	si_config_gcigpio(sih, GCI_LTE_TX_POS, ltetx_gcigpio, 0xFF,
		((1 << GCI_GPIOCTL_INEN_OFFSET)|(1 << GCI_GPIOCTL_PDN_OFFSET)));

	/* Configure WLAN Prio as output. BT Need to configure its ISM Prio separately
	 * NOTE: LTE chip has to enable its internal pull-down whenever WL goes down
	 */
	si_config_gcigpio(sih, GCI_WLAN_PRIO_POS, wlprio_gcigpio, 0xFF,
		(1 << GCI_GPIOCTL_OUTEN_OFFSET));

	/* Enable inbandIntMask for FrmSync only, disable LTE_Rx and LTE_Tx
	  * Note: FrameSync, LTE Rx & LTE Tx happen to share the same REGIDX
	  * Hence a single Access is sufficient
	  */
	si_gci_indirect(sih, GCI_REGIDX(GCI_LTE_FRAMESYNC_POS),
		GCI_OFFSETOF(sih, gci_inbandeventintmask),
		((1 << GCI_BITOFFSET(GCI_LTE_FRAMESYNC_POS))
		|(1 << GCI_BITOFFSET(GCI_LTE_RX_POS))
		|(1 << GCI_BITOFFSET(GCI_LTE_TX_POS))),
		((1 << GCI_BITOFFSET(GCI_LTE_FRAMESYNC_POS))
		|(0 << GCI_BITOFFSET(GCI_LTE_RX_POS))
		|(0 << GCI_BITOFFSET(GCI_LTE_TX_POS))));

	/* Enable Inband interrupt polarity for LTE_FRMSYNC */
	si_gci_indirect(sih, GCI_REGIDX(GCI_LTE_FRAMESYNC_POS),
		GCI_OFFSETOF(sih, gci_intpolreg),
		(1 << GCI_BITOFFSET(GCI_LTE_FRAMESYNC_POS)),
		(1 << GCI_BITOFFSET(GCI_LTE_FRAMESYNC_POS)));
} /* si_ercx_init */

void
si_wci2_init(si_t *sih, uint8 baudrate, uint32 ltecx_mux, uint32 ltecx_padnum,
	uint32 ltecx_fnsel, uint32 ltecx_gcigpio, uint32 xtalfreq)
{
	/* BCMLTECOEXGCI_ENAB should be checked before calling si_wci2_init() */
	uint8 baud = baudrate;
	uint8 seciin, seciout, fnselin, fnselout, gcigpioin, gcigpioout;

	/* Extract PAD GPIO number (1-byte) from "ltecx_padnum" for each LTECX pin */
	seciin =	LTECX_EXTRACT_PADNUM(ltecx_padnum, LTECX_NVRAM_WCI2IN_IDX);
	seciout =	LTECX_EXTRACT_PADNUM(ltecx_padnum, LTECX_NVRAM_WCI2OUT_IDX);
	/* Extract FunctionSel (1-nibble) from "ltecx_fnsel" for each LTECX pin */
	fnselin =	LTECX_EXTRACT_FNSEL(ltecx_fnsel, LTECX_NVRAM_WCI2IN_IDX);
	fnselout =	LTECX_EXTRACT_FNSEL(ltecx_fnsel, LTECX_NVRAM_WCI2OUT_IDX);
	/* Extract GCI-GPIO number (1-nibble) from "ltecx_gcigpio" for each LTECX pin */
	gcigpioin =	LTECX_EXTRACT_GCIGPIO(ltecx_gcigpio, LTECX_NVRAM_WCI2IN_IDX);
	gcigpioout =	LTECX_EXTRACT_GCIGPIO(ltecx_gcigpio, LTECX_NVRAM_WCI2OUT_IDX);

	/* reset GCI block */
	si_gci_reset(sih);

	/* NOTE: Writing Reserved bits of older GCI Revs is OK */
	si_gci_direct(sih, GCI_OFFSETOF(sih, gci_corectrl),
		((GCI_CCTL_SCS_MASK << GCI_CCTL_SCS_OFFSET)
		|(GCI_CCTL_LOWTOUT_MASK << GCI_CCTL_SILOWTOUT_OFFSET)
		|(1 << GCI_CCTL_BRKONSLP_OFFSET)
		|(1 << GCI_CCTL_US_OFFSET)
		|(GCI_MODE_MASK << GCI_CCTL_SMODE_OFFSET)
		|(1 << GCI_CCTL_FSL_OFFSET)
		|(1 << GCI_CCTL_SECIEN_OFFSET)),
		((GCI_CCTL_SCS_DEF << GCI_CCTL_SCS_OFFSET)
		|(GCI_CCTL_LOWTOUT_30BIT << GCI_CCTL_SILOWTOUT_OFFSET)
		|(0 << GCI_CCTL_BRKONSLP_OFFSET)
		|(0 << GCI_CCTL_US_OFFSET)
		|(GCI_MODE_BTSIG << GCI_CCTL_SMODE_OFFSET)
		|(0 << GCI_CCTL_FSL_OFFSET)
		|(1 << GCI_CCTL_SECIEN_OFFSET))); /* 19000024 */

	/* Program Function select for selected GPIOs */
	si_gci_set_functionsel(sih, seciin, fnselin);
	si_gci_set_functionsel(sih, seciout, fnselout);

	/* Enable inbandIntMask for FrmSync only; disable LTE_Rx and LTE_Tx
	  * Note: FrameSync, LTE Rx & LTE Tx happen to share the same REGIDX
	  * Hence a single Access is sufficient
	  */
	si_gci_indirect(sih,
		GCI_REGIDX(GCI_LTE_FRAMESYNC_POS),
		GCI_OFFSETOF(sih, gci_inbandeventintmask),
		((1 << GCI_BITOFFSET(GCI_LTE_FRAMESYNC_POS))
		|(1 << GCI_BITOFFSET(GCI_LTE_RX_POS))
		|(1 << GCI_BITOFFSET(GCI_LTE_TX_POS))),
		((1 << GCI_BITOFFSET(GCI_LTE_FRAMESYNC_POS))
		|(0 << GCI_BITOFFSET(GCI_LTE_RX_POS))
		|(0 << GCI_BITOFFSET(GCI_LTE_TX_POS))));

	/* enable gci event interrupt mask for WCI2 message type */
	si_gci_indirect(sih, 0x00011, GCI_OFFSETOF(sih, gci_eventintmask),
		(GCI_LTE_WCI2TYPE_MASK << GCI_BITOFFSET(GCI_LTE_WCI2TYPE_POS)),
		(GCI_LTE_WCI2TYPE_MASK << GCI_BITOFFSET(GCI_LTE_WCI2TYPE_POS)));

	if (GCIREV(sih->gcirev) >= 1) {
		/* Program inband interrupt polarity as posedge for FrameSync */
		si_gci_indirect(sih, GCI_REGIDX(GCI_LTE_FRAMESYNC_POS),
			GCI_OFFSETOF(sih, gci_intpolreg),
			(1 << GCI_BITOFFSET(GCI_LTE_FRAMESYNC_POS)),
			(1 << GCI_BITOFFSET(GCI_LTE_FRAMESYNC_POS)));
	}
	if (GCIREV(sih->gcirev) >= 4) {
		/* Program SECI_IN Control Register */
		si_gci_indirect(sih, GCI_LTECX_SECI_ID,
			GCI_OFFSETOF(sih, gci_seciin_ctrl), ALLONES_32,
			((GCI_MODE_BTSIG << GCI_SECIIN_MODE_OFFSET)
			 |(gcigpioin << GCI_SECIIN_GCIGPIO_OFFSET)
			 |(GCI_LTE_IP_ID << GCI_SECIIN_RXID2IP_OFFSET)));

		/* Program GPIO Control Register for SECI_IN GCI GPIO */
		si_gci_indirect(sih, gcigpioin/4, GCI_OFFSETOF(sih, gci_gpioctl),
			(0xFF << (gcigpioin%4)*8),
			(((1 << GCI_GPIOCTL_INEN_OFFSET)
			 |(1 << GCI_GPIOCTL_PDN_OFFSET)) << (gcigpioin%4)*8));

		/* Program SECI_OUT Control Register */
		si_gci_indirect(sih, GCI_LTECX_SECI_ID,
			GCI_OFFSETOF(sih, gci_seciout_ctrl), ALLONES_32,
			((GCI_MODE_BTSIG << GCI_SECIOUT_MODE_OFFSET)
			 |(gcigpioout << GCI_SECIOUT_GCIGPIO_OFFSET)
			 |((1 << GCI_LTECX_SECI_ID) << GCI_SECIOUT_SECIINRELATED_OFFSET)));

		/* Program GPIO Control Register for SECI_OUT GCI GPIO */
		si_gci_indirect(sih, gcigpioout/4, GCI_OFFSETOF(sih, gci_gpioctl),
			(0xFF << (gcigpioout%4)*8),
			(((1 << GCI_GPIOCTL_OUTEN_OFFSET)) << (gcigpioout%4)*8));

		/* Program SECI_IN Aux FIFO enable for LTECX SECI_IN Port */
		si_gci_direct(sih, GCI_OFFSETOF(sih, gci_seciin_auxfifo_en),
			(((1 << GCI_LTECX_SECI_ID) << GCI_SECIAUX_RXENABLE_OFFSET)
			|((1 << GCI_LTECX_SECI_ID) << GCI_SECIFIFO_RXENABLE_OFFSET)),
			(((1 << GCI_LTECX_SECI_ID) << GCI_SECIAUX_RXENABLE_OFFSET)
			|((1 << GCI_LTECX_SECI_ID) << GCI_SECIFIFO_RXENABLE_OFFSET)));

		/* Program SECI_OUT Tx Enable for LTECX SECI_OUT Port */
		si_gci_direct(sih, GCI_OFFSETOF(sih, gci_seciout_txen_txbr), ALLONES_32,
			((1 << GCI_LTECX_SECI_ID) << GCI_SECITX_ENABLE_OFFSET));
	}
	if (GCIREV(sih->gcirev) >= 5) {
		/* enable WlPrio/TxOn override from D11 */
		si_gci_direct(sih, GCI_OFFSETOF(sih, gci_miscctl),
			(1 << GCI_LTECX_TXCONF_EN_OFFSET | 1 << GCI_LTECX_PRISEL_EN_OFFSET),
			(1 << GCI_LTECX_TXCONF_EN_OFFSET | 1 << GCI_LTECX_PRISEL_EN_OFFSET));
	} else {
		si_gci_direct(sih, GCI_OFFSETOF(sih, gci_miscctl),
			(1 << GCI_LTECX_TXCONF_EN_OFFSET | 1 << GCI_LTECX_PRISEL_EN_OFFSET),
			0x0000);
	}
	/* baudrate: 1/2/3/4mbps, escseq:0xdb, high baudrate, enable seci_tx/rx */
	si_gci_direct(sih, GCI_OFFSETOF(sih, gci_secifcr), ALLONES_32, 0x00);
	if (GCIREV(sih->gcirev) >= 4) {
		si_gci_direct(sih, GCI_OFFSETOF(sih, gci_secilcr), ALLONES_32, 0x00);
	} else {
		si_gci_direct(sih, GCI_OFFSETOF(sih, gci_secilcr), ALLONES_32, 0x28);
	}
	si_gci_direct(sih, GCI_OFFSETOF(sih, gci_seciuartescval), ALLONES_32, 0xDB);

	switch (baud) {
	case 1:
		/* baudrate:1mbps */
		si_gci_direct(sih, GCI_OFFSETOF(sih, gci_secibauddiv),
			ALLONES_32, 0xFE);
		if (GCIREV(sih->gcirev) >= 4) {
			si_gci_direct(sih, GCI_OFFSETOF(sih, gci_secimcr),
				ALLONES_32, 0x80);
		} else {
			si_gci_direct(sih, GCI_OFFSETOF(sih, gci_secimcr),
				ALLONES_32, 0x81);
		}
		si_gci_direct(sih, GCI_OFFSETOF(sih, gci_baudadj),
			ALLONES_32, 0x23);
		break;

	case 2:
		/* baudrate:2mbps */
		if (xtalfreq == XTAL_FREQ_26000KHZ) {
			/* 43430 A0 uses 26 MHz crystal.
			 * Baudrate settings for crystel freq 26 MHz
			 */
			si_gci_direct(sih, GCI_OFFSETOF(sih, gci_secibauddiv),
					ALLONES_32, 0xFF);
			si_gci_direct(sih, GCI_OFFSETOF(sih, gci_secimcr),
					ALLONES_32, 0x80);
			si_gci_direct(sih, GCI_OFFSETOF(sih, gci_baudadj),
					ALLONES_32, 0x0);
		}
		else {
			si_gci_direct(sih, GCI_OFFSETOF(sih, gci_secibauddiv),
					ALLONES_32, 0xFF);
			if (GCIREV(sih->gcirev) >= 4) {
				si_gci_direct(sih, GCI_OFFSETOF(sih, gci_secimcr),
						ALLONES_32, 0x80);
			} else {
				si_gci_direct(sih, GCI_OFFSETOF(sih, gci_secimcr),
						ALLONES_32, 0x81);
			}
			si_gci_direct(sih, GCI_OFFSETOF(sih, gci_baudadj),
					ALLONES_32, 0x11);
		}
		break;

	case 4:
		/* baudrate:4mbps */
		si_gci_direct(sih, GCI_OFFSETOF(sih, gci_secibauddiv),
			ALLONES_32, 0xF7);
		if (GCIREV(sih->gcirev) >= 4) {
			si_gci_direct(sih, GCI_OFFSETOF(sih, gci_secimcr),
				ALLONES_32, 0x8);
		} else {
			si_gci_direct(sih, GCI_OFFSETOF(sih, gci_secimcr),
				ALLONES_32, 0x9);
		}
		si_gci_direct(sih, GCI_OFFSETOF(sih, gci_baudadj),
			ALLONES_32, 0x0);
		break;

	case 25:
		/* baudrate:2.5mbps */
		if (xtalfreq == XTAL_FREQ_26000KHZ) {
			/* 43430 A0 uses 26 MHz crystal.
			  * Baudrate settings for crystel freq 26 MHz
			  */
			si_gci_direct(sih, GCI_OFFSETOF(sih, gci_secibauddiv),
				ALLONES_32, 0xF6);
		} else {
			si_gci_direct(sih, GCI_OFFSETOF(sih, gci_secibauddiv),
				ALLONES_32, 0xF1);
		}
		if (GCIREV(sih->gcirev) >= 4) {
			si_gci_direct(sih, GCI_OFFSETOF(sih, gci_secimcr),
				ALLONES_32, 0x8);
		} else {
			si_gci_direct(sih, GCI_OFFSETOF(sih, gci_secimcr),
				ALLONES_32, 0x9);
		}
		si_gci_direct(sih, GCI_OFFSETOF(sih, gci_baudadj),
			ALLONES_32, 0x0);
		break;

	case 3:
	default:
		/* baudrate:3mbps */
		if (xtalfreq == XTAL_FREQ_26000KHZ) {
			/* 43430 A0 uses 26 MHz crystal.
			  * Baudrate settings for crystel freq 26 MHz
			  */
			si_gci_direct(sih, GCI_OFFSETOF(sih, gci_secibauddiv),
				ALLONES_32, 0xF7);
		} else {
			si_gci_direct(sih, GCI_OFFSETOF(sih, gci_secibauddiv),
				ALLONES_32, 0xF4);
		}
		if (GCIREV(sih->gcirev) >= 4) {
			si_gci_direct(sih, GCI_OFFSETOF(sih, gci_secimcr),
				ALLONES_32, 0x8);
		} else {
			si_gci_direct(sih, GCI_OFFSETOF(sih, gci_secimcr),
				ALLONES_32, 0x9);
		}
		si_gci_direct(sih, GCI_OFFSETOF(sih, gci_baudadj),
			ALLONES_32, 0x0);
		break;
	}
	/* GCI Rev >= 1 */
	if (GCIREV(sih->gcirev) >= 1) {
		/* Route Rx-data through AUX register */
		si_gci_direct(sih, GCI_OFFSETOF(sih, gci_rxfifo_common_ctrl),
			0xFF, 0xFF);
	} else {
		/* GPIO 3-7 as BT_SIG complaint */
		/* config GPIO pins 3-7 as input */
		si_gci_indirect(sih, 0,
			GCI_OFFSETOF(sih, gci_gpioctl), 0x20000000, 0x20000010);
		si_gci_indirect(sih, 1,
			GCI_OFFSETOF(sih, gci_gpioctl), 0x20202020, 0x20202020);
		/* gpio mapping: frmsync-gpio7, mws_rx-gpio6, mws_tx-gpio5,
		 * pat[0]-gpio4, pat[1]-gpio3
		 */
		si_gci_indirect(sih, 0x70010,
			GCI_OFFSETOF(sih, gci_gpiomask), 0x00000001, 0x00000001);
		si_gci_indirect(sih, 0x60010,
			GCI_OFFSETOF(sih, gci_gpiomask), 0x00000002, 0x00000002);
		si_gci_indirect(sih, 0x50010,
			GCI_OFFSETOF(sih, gci_gpiomask), 0x00000004, 0x00000004);
		si_gci_indirect(sih, 0x40010,
			GCI_OFFSETOF(sih, gci_gpiomask), 0x02000000, 0x00000008);
		si_gci_indirect(sih, 0x30010,
			GCI_OFFSETOF(sih, gci_gpiomask), 0x04000000, 0x04000010);
		/* gpio mapping: wlan_rx_prio-gpio5, wlan_tx_on-gpio4 */
		si_gci_indirect(sih, 0x50000,
			GCI_OFFSETOF(sih, gci_gpiomask), 0x00000010, 0x00000010);
		si_gci_indirect(sih, 0x40000,
			GCI_OFFSETOF(sih, gci_gpiomask), 0x00000020, 0x00000020);
		/* enable gpio out on gpio4(wlanrxprio), gpio5(wlantxon) */
		si_gci_direct(sih,
			GCI_OFFSETOF(sih, gci_control_0), 0x00000030, 0x00000000);
	}
} /* si_wci2_init */
#endif /* BCMLTECOEX */

void
si_gci_uart_init(si_t *sih, osl_t *osh, uint8 seci_mode)
{
#ifdef	HNDGCI
	hndgci_init(sih, osh, HND_GCI_PLAIN_UART_MODE,
		GCI_UART_BR_115200);

	/* specify rx callback */
	hndgci_uart_config_rx_complete(-1, -1, 0, NULL, NULL);
#else
	BCM_REFERENCE(sih);
	BCM_REFERENCE(osh);
	BCM_REFERENCE(seci_mode);
#endif	/* HNDGCI */
}

/**
 * A given GCI pin needs to be converted to a GCI FunctionSel register offset and the bit position
 * in this register.
 * @param[in]  input   pin number, see respective chip Toplevel Arch page, GCI chipstatus regs
 * @param[out] regidx  chipcontrol reg(ring_index base) and
 * @param[out] pos     bits to shift for pin first regbit
 *
 * eg: gpio9 will give regidx: 1 and pos 4
 */
static void
si_gci_get_chipctrlreg_ringidx_base4(uint32 pin, uint32 *regidx, uint32 *pos)
{
	*regidx = (pin / 8);
	*pos = (pin % 8) * 4; // each pin occupies 4 FunctionSel register bits

	SI_MSG(("si_gci_get_chipctrlreg_ringidx_base4:%d:%d:%d\n", pin, *regidx, *pos));
}

/* input: pin number
* output: chipcontrol reg(ring_index base) and
* bits to shift for pin first regbit.
* eg: gpio9 will give regidx: 2 and pos 16
*/
static uint8
si_gci_get_chipctrlreg_ringidx_base8(uint32 pin, uint32 *regidx, uint32 *pos)
{
	*regidx = (pin / 4);
	*pos = (pin % 4)*8;

	SI_MSG(("si_gci_get_chipctrlreg_ringidx_base8:%d:%d:%d\n", pin, *regidx, *pos));

	return 0;
}

/** setup a given pin for fnsel function */
void
si_gci_set_functionsel(si_t *sih, uint32 pin, uint8 fnsel)
{
	uint32 reg = 0, pos = 0;

	SI_MSG(("si_gci_set_functionsel:%d\n", pin));

	si_gci_get_chipctrlreg_ringidx_base4(pin, &reg, &pos);
	si_gci_chipcontrol(sih, reg, GCIMASK_4B(pos), GCIPOSVAL_4B(fnsel, pos));
}

/* Returns a given pin's fnsel value */
uint32
si_gci_get_functionsel(si_t *sih, uint32 pin)
{
	uint32 reg = 0, pos = 0, temp;

	SI_MSG(("si_gci_get_functionsel: %d\n", pin));

	si_gci_get_chipctrlreg_ringidx_base4(pin, &reg, &pos);
	temp = si_gci_chipstatus(sih, reg);
	return GCIGETNBL(temp, pos);
}

/* Sets fnsel value to IND for all the GPIO pads that have fnsel set to given argument */
void
si_gci_clear_functionsel(si_t *sih, uint8 fnsel)
{
	uint32 i;
	SI_MSG(("si_gci_clear_functionsel: %d\n", fnsel));
	for (i = 0; i <= CC4335_PIN_GPIO_LAST; i++)	{
		if (si_gci_get_functionsel(sih, i) == fnsel)
			si_gci_set_functionsel(sih, i, CC4335_FNSEL_IND);
	}
}
#endif /* (!_CFE_ && !_CFEZ_) || CFG_WL */

/** write 'val' to the gci chip control register indexed by 'reg' */
uint32
si_gci_chipcontrol(si_t *sih, uint reg, uint32 mask, uint32 val)
{
	si_corereg(sih, GCI_CORE_IDX(sih), GCI_OFFSETOF(sih, gci_indirect_addr), ~0, reg);
	return si_corereg(sih, GCI_CORE_IDX(sih), GCI_OFFSETOF(sih, gci_chipctrl), mask, val);
}

/* Read the gci chip status register indexed by 'reg' */
uint32
si_gci_chipstatus(si_t *sih, uint reg)
{
	si_corereg(sih, GCI_CORE_IDX(sih), GCI_OFFSETOF(sih, gci_indirect_addr), ~0, reg);
	/* setting mask and value to '0' to use si_corereg for read only purpose */
	return si_corereg(sih, GCI_CORE_IDX(sih), GCI_OFFSETOF(sih, gci_chipsts), 0, 0);
}
#endif /* !defined(BCMDONGLEHOST) */

#if !defined(BCMDONGLEHOST)
#if (!defined(_CFE_) && !defined(_CFEZ_)) || defined(CFG_WL)
#if defined(BCMLTECOEX)
/* Program GCI GpioMask and GCI GpioControl Registers */
static void
si_config_gcigpio(si_t *sih, uint32 gci_pos, uint8 gcigpio,
	uint8 gpioctl_mask, uint8 gpioctl_val)
{
	uint32 indirect_idx =
		GCI_REGIDX(gci_pos) | (gcigpio << GCI_GPIOIDX_OFFSET);
	si_gci_indirect(sih, indirect_idx, GCI_OFFSETOF(sih, gci_gpiomask),
		(1 << GCI_BITOFFSET(gci_pos)),
		(1 << GCI_BITOFFSET(gci_pos)));
	/* Write GPIO Configuration to GCI Registers */
	si_gci_indirect(sih, gcigpio/4, GCI_OFFSETOF(sih, gci_gpioctl),
		(gpioctl_mask << (gcigpio%4)*8), (gpioctl_val << (gcigpio%4)*8));
}
#endif /* BCMLTECOEX */
#endif /* (!_CFE_ && !_CFEZ_) || CFG_WL */
#endif /* !defined(BCMDONGLEHOST) */

uint16
BCMINITFN(si_chipid)(si_t *sih)
{
	si_info_t *sii = SI_INFO(sih);

	return (sii->chipnew) ? sii->chipnew : sih->chip;
}

/* CHIP_ID's being mapped here should not be used anywhere else in the code */
static void
BCMATTACHFN(si_chipid_fixup)(si_t *sih)
{
	si_info_t *sii = SI_INFO(sih);

	ASSERT(sii->chipnew == 0);
	switch (sih->chip) {
		case BCM43694_CHIP_ID:
			sii->chipnew = sih->chip; /* save it */
			sii->pub.chip = BCM43684_CHIP_ID; /* chip class */
		break;
		case BCM43692_CHIP_ID: /* also for BCM6705 */
		case BCM43693_CHIP_ID:
			sii->chipnew = sih->chip; /* save it */
			sii->pub.chip = BCM6710_CHIP_ID; /* chip class */
		break;
		case BCM43794_CHIP_ID:
			sii->chipnew = sih->chip; /* save it */
			sii->pub.chip = BCM6715_CHIP_ID; /* chip class */
		break;
		case BCM43741_CHIP_ID: /* fall through */
		case BCM43721_CHIP_ID:
			sii->chipnew = sih->chip; /* save it */
			sii->pub.chip = BCM6717_CHIP_ID; /* chip class */
		break;
		case BCM6716_CHIP_ID: /* fall through */
		case BCM43740_CHIP_ID: /* fall through */
		case BCM43720_CHIP_ID:
			sii->chipnew = sih->chip; /* save it */
			sii->pub.chip = BCM6726_CHIP_ID; /* chip class */
		break;
		case BCM43730_CHIP_ID: /* fall through */
		case BCM6706_CHIP_ID:
			sii->chipnew = sih->chip; /* save it */
			sii->pub.chip = BCM6711_CHIP_ID; /* chip class */
		break;
		default:
		break;
	}
} /* si_chipid_fixup */

#ifdef BCM_BACKPLANE_TIMEOUT
const si_axi_error_info_t *
si_get_axi_errlog_info(si_t *sih)
{
	if (CHIPTYPE(sih->socitype) == SOCI_AI) {
		return (const si_axi_error_info_t *)sih->err_info;
	}

	return NULL;
}

void
si_reset_axi_errlog_info(si_t *sih)
{
	if (sih->err_info) {
		sih->err_info->count = 0;
	}
}
#endif /* BCM_BACKPLANE_TIMEOUT */

#ifdef BCAWLAN205004
/** Convenience function */
static uint
si_backplane_wr32(si_t *sih, uint addr, uint val)
{
	return si_backplane_access(sih, addr, sizeof(uint32), &val, FALSE);
}

static void
bcm47622a0_war(si_t *sih)
{
	/* enable clocks so upcoming reset can propagate */
	si_backplane_wr32(sih, EMBEDDED2x2AX_D11_MWR0_ADDR + AI_IOCTRL,
	                  SICF_FASTCLKRQ | SICF_MCLKE | SICF_FCLKON);
	/* put d11 core in reset */
	si_backplane_wr32(sih, EMBEDDED2x2AX_D11_MWR0_ADDR + AI_RESETCTRL, 0x1);
	si_backplane_wr32(sih, EMBEDDED2x2AX_D11_MWR1_ADDR + AI_RESETCTRL, 0x1);
	OSL_DELAY(100); /* gives synchronous reset time to propagate through d11 core */
	/* pull d11 core out of reset */
	si_backplane_wr32(sih, EMBEDDED2x2AX_D11_MWR0_ADDR + AI_RESETCTRL, 0x0);
	si_backplane_wr32(sih, EMBEDDED2x2AX_D11_MWR1_ADDR + AI_RESETCTRL, 0x0);
	OSL_DELAY(100);
	/* PMU WD reset will assert in 1 ILP (32Khz) clocks */
	si_backplane_wr32(sih, EMBEDDED2x2AX_PMU_ADDR + PMU_WD_CNTR_REG_OFFSET, 0x1);
	OSL_DELAY(1000); /* wait for PMU WD to deassert so WLAN core accepts transactions again */
}
#endif /* BCAWLAN205004 */

#if !defined(BCMDONGLEHOST) && !defined(DONGLEBUILD)
/* radiodig pll1 cp1 register offset 0x18 */
#define RADIODIG_RFPLL1_PLL_CP1				0x28015060
#define RADIODIG_RFPLL1_PLL_CP1_MASK_rfpll_cp_pu	0x00002000

/* radiodig pll1 cfg1 register offset 0x29 */
#define RADIODIG_RFPLL1_PLL_CFG1			0x280150a4
#define RADIODIG_RFPLL1_PLL_CFG1_MASK_rfpll_mmd_pu	0x00000001
#define RADIODIG_RFPLL1_PLL_CFG1_MASK_rfpll_synth_pu	0x00000008
#define RADIODIG_RFPLL1_PLL_CFG1_MASK_rfpll_vco_pu	0x00000010

/* radiodig pll1 cfg2 register offset 0x2a */
#define RADIODIG_RFPLL1_PLL_CFG2			0x280150a8
#define RADIODIG_RFPLL1_PLL_CFG2_MASK_rfpll_pfd_pu	0x00000200
static void
bcm6711a0_war(si_t *sih)
{
	uint32 val = 0;

	/* radiodig pll1 cp1 register offset 0x18 */
	si_backplane_access(sih, RADIODIG_RFPLL1_PLL_CP1, sizeof(uint16), &val, TRUE);
//	SI_ERROR((" read 0x%x value 0x%x\n", RADIODIG_RFPLL1_PLL_CP1, val));
	val &= ~(RADIODIG_RFPLL1_PLL_CP1_MASK_rfpll_cp_pu);
	si_backplane_access(sih, RADIODIG_RFPLL1_PLL_CP1, sizeof(uint16), &val, FALSE);
//	SI_ERROR((" write 0x%x value 0x%x\n", RADIODIG_RFPLL1_PLL_CP1, val));

	/* radiodig pll1 register offset 0x29 */
	si_backplane_access(sih, RADIODIG_RFPLL1_PLL_CFG1, sizeof(uint16), &val, TRUE);
//	SI_ERROR((" read 0x%x value 0x%x\n", RADIODIG_RFPLL1_PLL_CFG1, val));
	val &= ~(RADIODIG_RFPLL1_PLL_CFG1_MASK_rfpll_mmd_pu |
	         RADIODIG_RFPLL1_PLL_CFG1_MASK_rfpll_synth_pu |
	         RADIODIG_RFPLL1_PLL_CFG1_MASK_rfpll_vco_pu);
	si_backplane_access(sih, RADIODIG_RFPLL1_PLL_CFG1, sizeof(uint16), &val, FALSE);
//	SI_ERROR((" write 0x%x value 0x%x\n", RADIODIG_RFPLL1_PLL_CFG1, val));

	/* radiodig pll1 register offset 0x2a */
	si_backplane_access(sih, RADIODIG_RFPLL1_PLL_CFG2, sizeof(uint16), &val, TRUE);
//	SI_ERROR((" read 0x%x value 0x%x\n", RADIODIG_RFPLL1_PLL_CFG2, val));
	val &= ~(RADIODIG_RFPLL1_PLL_CFG2_MASK_rfpll_pfd_pu);
	si_backplane_access(sih, RADIODIG_RFPLL1_PLL_CFG2, sizeof(uint16), &val, FALSE);
//	SI_ERROR((" write 0x%x value 0x%x\n", RADIODIG_RFPLL1_PLL_CFG2, val));
}
#endif /* !BCMDONGLEHOST && !DONGLEBUILD */

static enum sprom_bus_e
si_sprom_bus(si_info_t *sii)
{
	enum sprom_bus_e ret = SPROM_BUS_NO_SPROM;
	struct si_pub *sih = &sii->pub;

	if ((sih->cccaps & CC_CAP_SROM) != 0) {
		uint32 sromctrl;
		void *osh = si_osh(sih);
		uint origidx;
		chipcregs_t *cc;

		origidx = si_coreidx(&sii->pub);
		cc = si_setcore(sih, CC_CORE_ID, 0);
		ASSERT(cc);
		sromctrl = R_REG(osh, &cc->sromcontrol);
		if (sromctrl & UWIRE_PRESENT) {
			ret = SPROM_BUS_UWIRE;
		}

		if (CCREV(sih->ccrev) >= 132) { /* 6717a0, SPI support */
			sromctrl = R_REG(osh, &cc->sprom_spi_ctrl);
			if (sromctrl & SPROM_SPICTL_SPROM_PRESENT) {
				ret = SPROM_BUS_SPI;
			}
		}

		si_setcore(sih, origidx, 0);
	}

	return ret;
} /* si_sprom_bus */

/**
 * Allocate an si handle. This function may be called multiple times. This function is called by
 * both si_attach() and si_kattach().
 *
 * @param [in]   devid - e.g. a PCIe device id
 * @param [in]   regs  - virtual address of initial (chipcommon) core registers
 * @param [in]   sdh   - opaque pointer pointing to e.g. a Linux 'struct pci_dev'
 * @param[inout] vars - pointer to a to-be created pointer area for "environment" variables. Some
 *                      callers of this function set 'vars' to NULL.
 */
static si_info_t *
BCMATTACHFN(si_doattach)(si_info_t *sii, uint devid, osl_t *osh, volatile void *regs,
                         enum bustype_e bustype, void *sdh, char **vars, uint *varsz)
{
	struct si_pub *sih = &sii->pub;
	uint32 w, savewin_pa;
	chipcregs_t *cc = NULL;
	char *pvars = NULL;
	uint origidx;
	uint32 gpiopullup = 0, gpiopulldown = 0;

	ASSERT(GOODREGS(regs));

	savewin_pa = 0;

	sih->buscoreidx = BADIDX;
	sii->device_removed = FALSE;

	sii->curmap = regs;
	sii->sdh = sdh;
	sii->osh = osh;
	sii->second_bar0win = ~0x0;

#if defined(BCMDRIVER) && defined(LINUX_VERSION_CODE) && !defined(DONGLEBUILD) /* NIC \
	only */
	if (EMBEDDED_WLAN_CORE(devid)) {
		/* Prerequisite: the 'pseudo PCIe' config space code in the BSP has written the PCIe
		 * BAR0 register with the physical backplane address that the driver should use for
		 * this particular core.
		 */
		sih->enum_base_pa = pci_resource_start((struct pci_dev*)sdh, 0);
		sih->enum_base_bp_pov_pa = EMBEDDED_11BE_CORE(devid) ?
			SI_ENUM_BASE_BP_POV_2x2BE_PA /* == 0x9000_0000 */ :
			SI_ENUM_BASE_BP_POV_2x2AX_PA; /* == 0x8400_0000 */
		sih->enum_base_va = (uintptr)regs;
		SI_PRINT(("%s: Embedded WLAN core (devid:0x%x) pseudo-PCI, switching to SI;"
			"BAR0 enum_base_pa=0x%x / bp_pa=0x%x / va=0x%px\n", __FUNCTION__, devid,
			sih->enum_base_pa, sih->enum_base_bp_pov_pa, (void *)sih->enum_base_va));
		bustype = SI_BUS;
		cc = (chipcregs_t *) regs;
	} else
#endif /* BCMDRIVER && LINUX_VERSION_CODE && !DONGLEBUILD */
	{
		sih->enum_base_pa = sih->enum_base_bp_pov_pa = si_enum_base_pa(devid);
	}

#if defined(BCM_BACKPLANE_TIMEOUT)
	sih->err_info = MALLOCZ(osh, sizeof(si_axi_error_info_t));
	if (sih->err_info == NULL) {
		SI_ERROR(("%s: %zu bytes MALLOC FAILED",
			__FUNCTION__, sizeof(si_axi_error_info_t)));
	}
#endif /* BCM_BACKPLANE_TIMEOUT */

#if defined(BCM_BACKPLANE_TIMEOUT) && defined(linux)

	osl_set_bpt_cb(osh, (void *)si_clear_backplane_to_fast, (void *)sih);

#endif	/* BCM_BACKPLANE_TIMEOUT && (linux) */

	if (EMBEDDED_WLAN_CORE(devid)) {
		/* we are a si core mimic'ing a pci core; all set up already, nothing to do here */
	} else if (bustype == PCI_BUS) {
		if (OSL_PCI_READ_CONFIG(sii->osh, PCI_SPROM_CONTROL, sizeof(uint32)) ==
			0xffffffff) {
			SI_ERROR(("%s: incoming bus is PCI but it's a lie, switching to SI "
			          "devid:0x%x\n", __FUNCTION__, devid));
			bustype = SI_BUS;
			cc = (chipcregs_t *)REG_MAP(SI_ENUM_BASE_PA(sih), SI_CORE_SIZE);
		} else {
			/* find Chipcommon address */
#ifndef DONGLEBUILD
			if (BCM6715_DEVICE(devid) || BCM6715_CHIP(devid)) {
				si_reset_otp_ctrl(sih, sii->osh, devid, regs);
			}
#endif /* DONGLEBUILD */
			savewin_pa = OSL_PCI_READ_CONFIG(sii->osh, PCI_BAR0_WIN, sizeof(uint32));
			if (!GOODCOREADDR(savewin_pa, SI_ENUM_BASE_PA(sih)))
				savewin_pa = SI_ENUM_BASE_PA(sih);
			OSL_PCI_WRITE_CONFIG(sii->osh, PCI_BAR0_WIN, 4, SI_ENUM_BASE_PA(sih));
			if (!regs)
				return NULL;
			cc = (chipcregs_t *)regs;
		}
	} else {
		cc = (chipcregs_t *)REG_MAP(SI_ENUM_BASE_PA(sih), SI_CORE_SIZE);
	}

	sih->bustype = bustype;

#ifdef BCMBUSTYPE
	if (bustype != BUSTYPE(bustype)) {
		SI_ERROR(("si_doattach: bus type %d does not match configured bus type %d\n",
			bustype, BUSTYPE(bustype)));
		return NULL;
	}
#endif

	/* bus/core/clk setup for register access */
	if (!si_buscore_prep(sii, bustype, devid, sdh)) {
		SI_ERROR(("si_doattach: si_core_clk_prep failed %d\n", bustype));
		return NULL;
	}

	/* ChipID recognition.
	*   We assume we can read chipid at offset 0 from the regs arg.
	*   If we add other chiptypes (or if we need to support old sdio hosts w/o chipcommon),
	*   some way of recognizing them needs to be added here.
	*/
	if (cc == NULL) {
		SI_ERROR(("%s: chipcommon register space is null \n", __FUNCTION__));
		return NULL;
	}

	w = R_REG(osh, &cc->chipid);
	sih->socitype = (w & CID_TYPE_MASK) >> CID_TYPE_SHIFT;
	/* Might as wll fill in chip id rev & pkg */
	sih->chip = w & CID_ID_MASK;
	sih->chiprev = (w & CID_REV_MASK) >> CID_REV_SHIFT;
	sih->chippkg = (w & CID_PKG_MASK) >> CID_PKG_SHIFT;

	si_chipid_fixup(sih);

	sih->issim = IS_SIM(sih->chippkg);

#ifdef BCAWLAN205004
	if IS_47622A0_SLAVE_SLICE(devid, sih->chiprev, sih->enum_base_pa) {
		bcm47622a0_war(sih);
	}
#endif /* BCAWLAN205004 */

	/* scan for cores */
	if ((CHIPTYPE(sii->pub.socitype) == SOCI_AI) ||
		(CHIPTYPE(sii->pub.socitype) == SOCI_NAI) ||
		(CHIPTYPE(sii->pub.socitype) == SOCI_DVTBUS)) {

		if (CHIPTYPE(sii->pub.socitype) == SOCI_AI)
			SI_MSG(("Found chip type AI (0x%08x)\n", w));
		else if (CHIPTYPE(sii->pub.socitype) == SOCI_NAI)
			SI_MSG(("Found chip type NAI (0x%08x)\n", w));
		else
			SI_MSG(("Found chip type DVT (0x%08x)\n", w));
		/* pass chipc address instead of original core base */

		if (sii->osh) {
			sii->axi_wrapper = (axi_wrapper_t *)MALLOCZ(sii->osh,
				(sizeof(axi_wrapper_t) * SI_MAX_AXI_WRAPPERS));

			if (sii->axi_wrapper == NULL) {
				SI_ERROR(("%s: %zu  bytes MALLOC Failed", __FUNCTION__,
					(sizeof(axi_wrapper_t) * SI_MAX_AXI_WRAPPERS)));
			}
		} else {
			sii->axi_wrapper = NULL;
		}

		ai_scan(&sii->pub, (void *)(uintptr)cc, devid);
	} else {
		SI_ERROR(("Found chip of unknown type (0x%08x)\n", w));
		return NULL;
	}

	/* no cores found, bail out */
	if (sii->numcores == 0) {
		SI_ERROR(("si_doattach: could not find any cores\n"));
		return NULL;
	}

	/* bus/core/clk setup */
	origidx = SI_CC_IDX;
	if (!si_buscore_setup(sii, cc, bustype, savewin_pa, &origidx, regs)) {
		SI_ERROR(("si_doattach: si_buscore_setup failed\n"));
		goto exit;
	}

	sii->pub.sprom_bus = si_sprom_bus(sii);

#if (!defined(_CFE_) && !defined(_CFEZ_)) || defined(CFG_WL)
	if (bustype == SI_BUS && sii->pub.sprom_bus != SPROM_BUS_NO_SPROM) {
		si_srom_clk_set(sih);
	}
#endif /* (!_CFE_ && !_CFEZ_) || CFG_WL */
#ifdef SI_SPROM_PROBE
	si_sprom_init(sih);
#endif /* SI_SPROM_PROBE */

#if !defined(BCMDONGLEHOST)
	/* Init OTP */
	otp_init((struct si_pub *)&(sii->pub));

	/* Init nvram from flash if it exists */
	nvram_init((void *)&(sii->pub));

#if defined(_CFE_) && defined(BCM_DEVINFO)
	devinfo_nvram_init((void *)&(sii->pub));
#endif

	/* Init nvram from sprom/otp if they exist */
#if defined(STRICT_GCC_WARNINGS) && defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
#endif

#ifdef DONGLEBUILD
	/* Init nvram from sprom/otp if they exist and not inited */
	if (si_getkvars()) {
		*vars = si_getkvars();
		*varsz = si_getkvarsz();
	}
	else
#endif /* DONGLEBUILD */
	if (srom_var_init(&sii->pub, BUSTYPE(bustype), (void *)regs,
			sii->osh, vars, varsz)) {
		SI_ERROR(("si_doattach: srom_var_init failed: bad srom\n"));
		goto exit;
	}

#if defined(STRICT_GCC_WARNINGS) && defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
	pvars = vars ? *vars : NULL;

	si_nvram_process(sii, pvars);

	/* xtalfreq is required for programming open loop calibration support changes */
	sii->xtalfreq = getintvar(NULL, rstr_xtalfreq);
	/* === NVRAM, clock is ready === */
#else
	pvars = NULL;
	BCM_REFERENCE(pvars);
#endif /* !BCMDONGLEHOST */

	if (!si_onetimeinit) {
#if !defined(BCMDONGLEHOST)
		char *val;
		uint32 xtalfreq;

		/* Cache nvram override to min mask */
		if ((val = getvar(NULL, rstr_rmin)) != NULL) {
			sii->min_mask_valid = TRUE;
			sii->nvram_min_mask = (uint32)bcm_strtoul(val, NULL, 0);
		} else {
			sii->min_mask_valid = FALSE;
		}
		/* Cache nvram override to max mask */
		if ((val = getvar(NULL, rstr_rmax)) != NULL) {
			sii->max_mask_valid = TRUE;
			sii->nvram_max_mask = (uint32)bcm_strtoul(val, NULL, 0);
		} else {
			sii->max_mask_valid = FALSE;
		}
#endif /* !BCMDONGLEHOST */

#if defined(CONFIG_XIP) && defined(BCMTCAM)
		/* patch the ROM if there are any patch pairs from OTP/SPROM */
		if (patch_pair) {

#if defined(__ARM_ARCH_7R__)
			hnd_tcam_bootloader_load(si_setcore(sih, ARMCR4_CORE_ID, 0), pvars);
#elif defined(__ARM_ARCH_7A__) || defined(__ARM_ARCH_8A__)
			hnd_tcam_bootloader_load(si_setcore(sih, SYSMEM_CORE_ID, 0), pvars);
#else
			hnd_tcam_bootloader_load(si_setcore(sih, SOCRAM_CORE_ID, 0), pvars);
#endif
			si_setcoreidx(sih, origidx);
		}
#endif /* CONFIG_XIP && BCMTCAM */

		cc = (chipcregs_t *)si_setcore(sih, CC_CORE_ID, 0);
		ASSERT(cc != NULL);

#if !defined(BCMDONGLEHOST) /* if not a DHD build */
		if (getvar(pvars, rstr_gpiopulldown) != NULL) {
			uint32 value;
			value = getintvar(pvars, rstr_gpiopulldown);
			if (value != 0xFFFFFFFF) { /* non populated SROM fields are ffff */
				gpiopulldown |= value;
			}
		}
#endif /* !BCMDONGLEHOST */

		W_REG(osh, &cc->gpiopullup, gpiopullup);
		W_REG(osh, &cc->gpiopulldown, gpiopulldown);
		si_setcoreidx(sih, origidx);

#if !defined(BCMDONGLEHOST)
		/* PMU specific initializations */
		si_pmu_init(sih, sii->osh);
		si_pmu_chip_init(sih, sii->osh);
		xtalfreq = getintvar(pvars, rstr_xtalfreq);
		switch (CHIPID(sih->chip)) {
		CASE_BCM43684_CHIP:
			xtalfreq = 54000;
			break;
		CASE_EMBEDDED_2x2AX_CORE:
		CASE_BCM6710_CHIP:
		CASE_BCM6715_CHIP:
			xtalfreq = 50000;
			break;
		CASE_BCM6717_CHIP:
		CASE_BCM6726_CHIP:
		CASE_EMBEDDED_11BE_CORE:
		CASE_BCM6711_CHIP:
			xtalfreq = 80000;
			break;
		default:
			break;
		}

		/* If xtalfreq var not available, try to measure it */
		if (xtalfreq == 0)
			xtalfreq = si_pmu_measure_alpclk(sih, sii->osh);

		sii->xtalfreq = xtalfreq;
		si_pmu_pll_init(sih, sii->osh, xtalfreq);

		si_pmu_res_init(sih, sii->osh);
		si_pmu_swreg_init(sih, sii->osh);
#endif /* !defined(BCMDONGLEHOST) */
#ifdef _RTE_
		si_onetimeinit = TRUE;
#endif
	}

#if !defined(BCMDONGLEHOST) && ((!defined(_CFE_) && !defined(_CFEZ_)) || \
	defined(CFG_WL))
#ifdef WLLED
	/* setup the GPIO based LED powersave register */
	if ((w = getintvar(pvars, rstr_leddc)) == 0)
		w = DEFAULT_GPIOTIMERVAL;
	si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, gpiotimerval), ~0, w);
#endif

	if (PCIE(sii)) {
		ASSERT(sii->pch != NULL);
		pcicore_attach(sii->pch, pvars, SI_DOATTACH);
	}

#ifdef SECI_UART
	/* Enable pull up on fast_uart_rx and fast_uart_cts_in
	* when fast uart is disabled.
	*/
	if (getvar(pvars, rstr_fuart_pup_rx_cts) != NULL) {
		w = getintvar(pvars, rstr_fuart_pup_rx_cts);
		if (w)
			fuart_pullup_rx_cts_enab = TRUE;
	}
#endif

	/* configure default pinmux enables for the chip */
	if (getvar(pvars, rstr_muxenab) != NULL) {
		w = getintvar(pvars, rstr_muxenab);
		si_muxenab((si_t *)sii, w);
	}

	sii->device_wake_opt = CC_GCI_GPIO_INVALID;
#endif /* !BCMDONGLEHOST && ((!_CFE_ && !_CFEZ_) || CFG_WL) */

#if defined(BCMPMU_STATS) && !defined(BCMPMU_STATS_DISABLED)
	si_pmustatstimer_init(sih);
#endif /* BCMPMU_STATS */

#ifdef BOOTLOADER_CONSOLE_OUTPUT
	/* Enable console prints */
	si_muxenab(sii, 3);
#endif

#if !defined(BCMDONGLEHOST)
	if (BCM43684_CHIP(sih->chip)) {
		const uint32 default_val = 0x18000800;
		uint32 val = si_pmu_chipcontrol(sih, 0, 0, 0);
		if (val != default_val) {
			SI_ERROR(("PMU CC reg 0 reset (0x%x)\n", val));
			si_pmu_chipcontrol(sih, 0, ~0, default_val);
		}
	}
#endif /* !BCMDONGLEHOST */

#if !defined(BCMDONGLEHOST) && !defined(DONGLEBUILD)
	/* Apply 6711 WAR in main only package */
	if (BCM6711_CHIP(sih->chip) && sih->chiprev == 0 &&
	    !BCM6711_TWO_PLUS_ONE_PKG(sih->otpflag)) {
		bcm6711a0_war(sih);
	}
#endif /* !BCMDONGLEHOST && !DONGLEBUILD */

	return (sii);

exit:
#if defined(BCM_BACKPLANE_TIMEOUT)
	if (sih->err_info) {
		MFREE(sii->osh, sih->err_info, sizeof(si_axi_error_info_t));
		sii->pub.err_info = NULL;
	}
#endif /* BCM_BACKPLANE_TIMEOUT */

	if (sii->axi_wrapper) {
		MFREE(sii->osh, sii->axi_wrapper,
			(sizeof(axi_wrapper_t) * SI_MAX_AXI_WRAPPERS));
		sii->axi_wrapper = NULL;
	}

#if !defined(BCMDONGLEHOST)
	if (BUSTYPE(sih->bustype) == PCI_BUS) {
		if (sii->pch)
			pcicore_deinit(sii->pch);
		sii->pch = NULL;
	}
#endif /* !defined(BCMDONGLEHOST) */

	return NULL;
} /* si_doattach */

/** may be called with core in reset */
void
BCMATTACHFN(si_detach)(si_t *sih)
{
	si_info_t *sii = SI_INFO(sih);
	si_cores_info_t *cores_info = (si_cores_info_t *)sii->cores_info;
	uint idx;

#if !defined(BCMDONGLEHOST) && defined(STA)
	struct si_pub *si_local = NULL;
	bcopy(&sih, &si_local, sizeof(si_t*));
#endif /* !BCMDONGLEHOST & STA */

	if (BUSTYPE(sih->bustype) == SI_BUS) {
		if (EMBEDDED_WLAN_CORE(sih->chip)) {
			/* no need to unmap register spaces */
		} else for (idx = 0; idx < SI_MAXCORES; idx++) {
			if (cores_info->regs[idx]) {
				REG_UNMAP(cores_info->regs[idx]);
				cores_info->regs[idx] = NULL;
			}
		}
	}
#if !defined(BCMDONGLEHOST) && defined(STA)
	srom_var_deinit((void *)si_local);
	nvram_exit((void *)si_local); /* free up nvram buffers */
#endif /* !BCMDONGLEHOST  & STA */

#if !defined(BCMDONGLEHOST)
	if (BUSTYPE(sih->bustype) == PCI_BUS) {
		if (sii->pch)
			pcicore_deinit(sii->pch);
		sii->pch = NULL;
	}
#endif /* !defined(BCMDONGLEHOST) */
#if !defined(BCMBUSTYPE) || (BCMBUSTYPE == SI_BUS)
	if (cores_info != &ksii_cores_info)
#endif	/* !BCMBUSTYPE || (BCMBUSTYPE == SI_BUS) */
		MFREE(sii->osh, cores_info, sizeof(si_cores_info_t));

#if defined(BCM_BACKPLANE_TIMEOUT)
	if (sih->err_info) {
		MFREE(sii->osh, sih->err_info, sizeof(si_axi_error_info_t));
		sii->pub.err_info = NULL;
	}
#endif /* BCM_BACKPLANE_TIMEOUT */

	if (sii->axi_wrapper) {
		MFREE(sii->osh, sii->axi_wrapper,
			(sizeof(axi_wrapper_t) * SI_MAX_AXI_WRAPPERS));
		sii->axi_wrapper = NULL;
	}

#if !defined(BCMBUSTYPE) || (BCMBUSTYPE == SI_BUS)
	if (sii != &ksii)
#endif	/* !BCMBUSTYPE || (BCMBUSTYPE == SI_BUS) */
		MFREE(sii->osh, sii, sizeof(si_info_t));
} /* si_detach */

void *
si_osh(si_t *sih)
{
	si_info_t *sii;

	sii = SI_INFO(sih);
	return sii->osh;
}

void
si_setosh(si_t *sih, osl_t *osh)
{
	si_info_t *sii;

	sii = SI_INFO(sih);
	if (sii->osh != NULL) {
		SI_ERROR(("osh is already set....\n"));
		ASSERT(!sii->osh);
	}
	sii->osh = osh;
}

/** register driver interrupt disabling and restoring callback functions */
void
BCMATTACHFN(si_register_intr_callback)(si_t *sih, void *intrsoff_fn, void *intrsrestore_fn,
                          void *intrsenabled_fn, void *intr_arg)
{
	si_info_t *sii = SI_INFO(sih);
	si_cores_info_t *cores_info = (si_cores_info_t *)sii->cores_info;
	sii->intr_arg = intr_arg;
	sii->intrsoff_fn = (si_intrsoff_t)intrsoff_fn;
	sii->intrsrestore_fn = (si_intrsrestore_t)intrsrestore_fn;
	sii->intrsenabled_fn = (si_intrsenabled_t)intrsenabled_fn;
	/* save current core id.  when this function called, the current core
	 * must be the core which provides driver functions(il, et, wl, etc.)
	 */
	sii->dev_coreid = cores_info->coreid[sii->curidx];
}

void
BCMATTACHFN(si_deregister_intr_callback)(si_t *sih)
{
	si_info_t *sii;

	sii = SI_INFO(sih);
	sii->intrsoff_fn = NULL;
	sii->intrsrestore_fn = NULL;
	sii->intrsenabled_fn = NULL;
}

uint
si_intflag(si_t *sih)
{
	si_info_t *sii = SI_INFO(sih);

	if ((CHIPTYPE(sih->socitype) == SOCI_AI) ||
		(CHIPTYPE(sih->socitype) == SOCI_DVTBUS) ||
		(CHIPTYPE(sih->socitype) == SOCI_NAI)) {
#ifdef HNDGIC
		return R_REG(sii->osh, ((uint32 *)(uintptr)(sii->gicd_spisr)));
#else
		return R_REG(sii->osh, ((uint32 *)(uintptr)
			    (sii->oob_router_pa + OOB_STATUSA)));
#endif
	} else {
		ASSERT(0);
		return 0;
	}
}

uint
si_flag(si_t *sih)
{
	if ((CHIPTYPE(sih->socitype) == SOCI_AI) ||
		(CHIPTYPE(sih->socitype) == SOCI_DVTBUS) ||
		(CHIPTYPE(sih->socitype) == SOCI_NAI)) {
		return ai_flag(sih);
	} else {
		ASSERT(0);
		return 0;
	}
}

uint
si_flag_alt(si_t *sih)
{
	if ((CHIPTYPE(sih->socitype) == SOCI_AI) ||
	(CHIPTYPE(sih->socitype) == SOCI_DVTBUS) ||
	(CHIPTYPE(sih->socitype) == SOCI_NAI))
		return ai_flag_alt(sih);
	else {
		ASSERT(0);
		return 0;
	}
}

void
BCMATTACHFN(si_setint)(si_t *sih, int siflag)
{
	if ((CHIPTYPE(sih->socitype) == SOCI_AI) ||
		(CHIPTYPE(sih->socitype) == SOCI_DVTBUS) ||
		(CHIPTYPE(sih->socitype) == SOCI_NAI))
		ai_setint(sih, siflag);
	else
		ASSERT(0);
}

uint
si_coreid(si_t *sih)
{
	si_info_t *sii = SI_INFO(sih);
	si_cores_info_t *cores_info = (si_cores_info_t *)sii->cores_info;

	return cores_info->coreid[sii->curidx];
}

uint
si_coreidx(si_t *sih)
{
	si_info_t *sii;

	sii = SI_INFO(sih);
	return sii->curidx;
}

volatile void *
si_d11_switch_addrbase(si_t *sih, uint coreunit)
{
	return si_setcore(sih,  D11_CORE_ID, coreunit);
}

/** return the core-type instantiation # of the current core */
uint
si_coreunit(si_t *sih)
{
	si_info_t *sii = SI_INFO(sih);
	si_cores_info_t *cores_info = (si_cores_info_t *)sii->cores_info;
	uint idx;
	uint coreid;
	uint coreunit;
	uint i;

	coreunit = 0;

	idx = sii->curidx;

	ASSERT(GOODREGS(sii->curmap));
	coreid = si_coreid(sih);

	/* count the cores of our type */
	for (i = 0; i < idx; i++)
		if (cores_info->coreid[i] == coreid)
			coreunit++;

	return (coreunit);
}

uint
BCMATTACHFN(si_corevendor)(si_t *sih)
{
	if ((CHIPTYPE(sih->socitype) == SOCI_AI) ||
		(CHIPTYPE(sih->socitype) == SOCI_DVTBUS) ||
		(CHIPTYPE(sih->socitype) == SOCI_NAI)) {
		return ai_corevendor(sih);
	} else {
		ASSERT(0);
		return 0;
	}
}

bool
BCMINITFN(si_backplane64)(si_t *sih)
{
	return ((sih->cccaps & CC_CAP_BKPLN64) != 0);
}

uint
si_corerev(si_t *sih)
{
	if ((CHIPTYPE(sih->socitype) == SOCI_AI) ||
		(CHIPTYPE(sih->socitype) == SOCI_DVTBUS) ||
		(CHIPTYPE(sih->socitype) == SOCI_NAI)) {
		return ai_corerev(sih);
	} else {
		ASSERT(0);
		return 0;
	}
}

uint
si_corerev_minor(si_t *sih)
{
	if (CHIPTYPE(sih->socitype) == SOCI_AI) {
		return ai_corerev_minor(sih);
	} else {
		return 0;
	}
}

/* return index of coreid or BADIDX if not found */
uint
si_findcoreidx(si_t *sih, uint coreid, uint coreunit)
{
	si_info_t *sii = SI_INFO(sih);
	si_cores_info_t *cores_info = (si_cores_info_t *)sii->cores_info;
	uint found;
	uint i;

	ASSERT(sii->numcores <= ARRAYSIZE(cores_info->coreid));
	found = 0;

	for (i = 0; i < sii->numcores; i++)
		if (cores_info->coreid[i] == coreid) {
			if (found == coreunit)
				return (i);
			found++;
		}

	return (BADIDX);
}

/** return total coreunit of coreid or zero if not found */
uint
si_numcoreunits(si_t *sih, uint coreid)
{
	si_info_t *sii = SI_INFO(sih);
	si_cores_info_t *cores_info = (si_cores_info_t *)sii->cores_info;
	uint found = 0;
	uint i;

	for (i = 0; i < sii->numcores; i++) {
		if (cores_info->coreid[i] == coreid) {
			found++;
		}
	}

	return found;
}

/** return total D11 coreunits */
uint
BCMRAMFN(si_numd11coreunits)(si_t *sih)
{
	return !!(si_numcoreunits(sih, D11_CORE_ID));
}

/** return list of found cores */
uint
si_corelist(si_t *sih, uint coreid[])
{
	si_info_t *sii = SI_INFO(sih);
	si_cores_info_t *cores_info = (si_cores_info_t *)sii->cores_info;

	bcopy((uchar*)cores_info->coreid, (uchar*)coreid, (sii->numcores * sizeof(uint)));
	return (sii->numcores);
}

/** return current wrapper mapping */
void *
si_wrapperregs(si_t *sih)
{
	si_info_t *sii;

	sii = SI_INFO(sih);
	ASSERT(GOODREGS(sii->curwrap));

	return (sii->curwrap);
}

/** return current register mapping */
volatile void *
si_coreregs(si_t *sih)
{
	si_info_t *sii;

	sii = SI_INFO(sih);
	ASSERT(GOODREGS(sii->curmap));

	return (sii->curmap);
}

/**
 * This function changes logical "focus" to the indicated core;
 * must be called with interrupts off.
 * Moreover, callers should keep interrupts off during switching out of and back to d11 core
 */
volatile void *
si_setcore(si_t *sih, uint coreid, uint coreunit)
{
	uint idx;

	idx = si_findcoreidx(sih, coreid, coreunit);
	if (!GOODIDX(idx)) {
		return (NULL);
	}

	if ((CHIPTYPE(sih->socitype) == SOCI_AI) ||
		(CHIPTYPE(sih->socitype) == SOCI_DVTBUS) ||
		(CHIPTYPE(sih->socitype) == SOCI_NAI)) {
		return ai_setcoreidx(sih, idx);
	} else {
		ASSERT(0);
		return NULL;
	}
}

volatile void *
si_setcoreidx(si_t *sih, uint coreidx)
{
	if ((CHIPTYPE(sih->socitype) == SOCI_AI) ||
		(CHIPTYPE(sih->socitype) == SOCI_DVTBUS) ||
		(CHIPTYPE(sih->socitype) == SOCI_NAI)) {
		return ai_setcoreidx(sih, coreidx);
	} else {
		ASSERT(0);
		return NULL;
	}
}

/** Turn off interrupt as required by sb_setcore, before switch core */
volatile void *
si_switch_core(si_t *sih, uint coreid, uint *origidx, uint *intr_val)
{
	volatile void *cc;
	si_info_t *sii = SI_INFO(sih);

	if (SI_FAST(sii)) {
		/* Overloading the origidx variable to remember the coreid,
		 * this works because the core ids cannot be confused with
		 * core indices.
		 */
		*origidx = coreid;
		if (coreid == CC_CORE_ID)
			return (volatile void *)CCREGS_FAST(sii);
		else if (coreid == BUSCORETYPE(sih->buscoretype))
			return (volatile void *)PCIEREGS(sii);
	}
	INTR_OFF(sii, *intr_val);
	*origidx = sii->curidx;
	cc = si_setcore(sih, coreid, 0);
	ASSERT(cc != NULL);

	return cc;
}

/* restore coreidx and restore interrupt */
void
si_restore_core(si_t *sih, uint coreid, uint intr_val)
{
	si_info_t *sii = SI_INFO(sih);

	if (SI_FAST(sii) && ((coreid == CC_CORE_ID) || (coreid == BUSCORETYPE(sih->buscoretype))))
		return;

	si_setcoreidx(sih, coreid);
	INTR_RESTORE(sii, intr_val);
}

int
BCMATTACHFN(si_numaddrspaces)(si_t *sih)
{
	if ((CHIPTYPE(sih->socitype) == SOCI_AI) ||
		(CHIPTYPE(sih->socitype) == SOCI_DVTBUS) ||
		(CHIPTYPE(sih->socitype) == SOCI_NAI)) {
		return ai_numaddrspaces(sih);
	} else {
		ASSERT(0);
		return 0;
	}
}

/**
 * Return the physical address of the nth address space in the current core
 * Arguments:
 * @param sih     Pointer to struct si_t
 * @param spidx   Slave port index
 * @param baidx   Base address index
 */
uint32
si_addrspace(si_t *sih, uint spidx, uint baidx)
{
	if ((CHIPTYPE(sih->socitype) == SOCI_AI) ||
		(CHIPTYPE(sih->socitype) == SOCI_DVTBUS) ||
		(CHIPTYPE(sih->socitype) == SOCI_NAI)) {
		return ai_addrspace(sih, spidx, baidx);
	} else {
		ASSERT(0);
		return 0;
	}
}

/* Return the size of the nth address space in the current core
 * Arguments:
 * sih : Pointer to struct si_t
 * spidx : slave port index
 * baidx : base address index
 */
uint32
si_addrspacesize(si_t *sih, uint spidx, uint baidx)
{
	if ((CHIPTYPE(sih->socitype) == SOCI_AI) ||
		(CHIPTYPE(sih->socitype) == SOCI_DVTBUS) ||
		(CHIPTYPE(sih->socitype) == SOCI_NAI)) {
		return ai_addrspacesize(sih, spidx, baidx);
	} else {
		ASSERT(0);
		return 0;
	}
}

void
si_coreaddrspaceX(si_t *sih, uint asidx, uint32 *addr, uint32 *size)
{
	/* Only supported for SOCI_AI */
	if ((CHIPTYPE(sih->socitype) == SOCI_AI) ||
		(CHIPTYPE(sih->socitype) == SOCI_DVTBUS) ||
		(CHIPTYPE(sih->socitype) == SOCI_NAI))
		ai_coreaddrspaceX(sih, asidx, addr, size);
	else
		*size = 0;
}

uint32
si_core_cflags(si_t *sih, uint32 mask, uint32 val)
{
	if ((CHIPTYPE(sih->socitype) == SOCI_AI) ||
		(CHIPTYPE(sih->socitype) == SOCI_DVTBUS) ||
		(CHIPTYPE(sih->socitype) == SOCI_NAI)) {
		return ai_core_cflags(sih, mask, val);
	} else {
		ASSERT(0);
		return 0;
	}
}

void
si_core_cflags_wo(si_t *sih, uint32 mask, uint32 val)
{
	if ((CHIPTYPE(sih->socitype) == SOCI_AI) ||
		(CHIPTYPE(sih->socitype) == SOCI_DVTBUS) ||
		(CHIPTYPE(sih->socitype) == SOCI_NAI))
		ai_core_cflags_wo(sih, mask, val);
	else
		ASSERT(0);
}

uint32
si_core_sflags(si_t *sih, uint32 mask, uint32 val)
{
	if ((CHIPTYPE(sih->socitype) == SOCI_AI) ||
		(CHIPTYPE(sih->socitype) == SOCI_DVTBUS) ||
		(CHIPTYPE(sih->socitype) == SOCI_NAI)) {
		return ai_core_sflags(sih, mask, val);
	} else {
		ASSERT(0);
		return 0;
	}
}

void
si_commit(si_t *sih)
{
	if ((CHIPTYPE(sih->socitype) == SOCI_AI) ||
		(CHIPTYPE(sih->socitype) == SOCI_DVTBUS) ||
		(CHIPTYPE(sih->socitype) == SOCI_NAI)) {
		;
	} else {
		ASSERT(0);
	}
}

bool
si_iscoreup(si_t *sih)
{
	bool ret = FALSE;

	if ((CHIPTYPE(sih->socitype) == SOCI_AI) ||
		(CHIPTYPE(sih->socitype) == SOCI_DVTBUS) ||
		(CHIPTYPE(sih->socitype) == SOCI_NAI)) {
		ret = ai_iscoreup(sih);
	} else {
		ASSERT(0);
	}

	return ret;
}

uint
si_wrapperreg(si_t *sih, uint32 offset, uint32 mask, uint32 val)
{
	/* only for AI back plane chips */
	if ((CHIPTYPE(sih->socitype) == SOCI_AI) ||
		(CHIPTYPE(sih->socitype) == SOCI_DVTBUS) ||
		(CHIPTYPE(sih->socitype) == SOCI_NAI))
		return (ai_wrap_reg(sih, offset, mask, val));
	return 0;
}

static int
si_backplane_addr_sane(uint addr, uint size)
{
	int bcmerror = BCME_OK;

	/* For 2 byte access, address has to be 2 byte aligned */
	if (size == 2) {
		if (addr & 0x1) {
			bcmerror = BCME_ERROR;
		}
	}
	/* For 4 byte access, address has to be 4 byte aligned */
	if (size == 4) {
		if (addr & 0x3) {
			bcmerror = BCME_ERROR;
		}
	}
	return bcmerror;
}

void
si_invalidate_second_bar0win(si_t *sih)
{
	si_info_t *sii = SI_INFO(sih);
	sii->second_bar0win = ~0x0;
}

/**
 * read or write to a given physical backplane address, SI_BUS variant.
 *
 * @param[in] addr_pa     Physical address from the perspective of the AXI backplane
 */
static uint
si_backplane_access_si(si_t *sih, uint addr_pa, uint size, uint *val, bool read)
{
	void *va;  /** virtual address */

#ifdef DONGLEBUILD /* For dongle builds, virtual are the same as physical addresses */
	va = (void *)addr_pa;
#else /* embedded WLAN cores */
	if (addr_pa != 0) {
		addr_pa -= SI_ENUM_BASE_BP_POV_PA(sih);
		addr_pa += SI_ENUM_BASE_PA(sih); /* becomes pa from CPU's perspective */
	}

	va = REG_MAP(addr_pa, sizeof(uint32));
#endif /* DONGLEBUILD */

	switch (size) {
		case sizeof(uint8) :
			if (read)
				*val = *(volatile uint8*)va;
			else
				*(volatile uint8*)va = *val;
			break;
		case sizeof(uint16) :
			if (read)
				*val = *(volatile uint16*)va;
			else
				*(volatile uint16*)va = *val;
			break;
		case sizeof(uint32) :
			if (read)
				*val = *(volatile uint32*)va;
			else
				*(volatile uint32*)va = *val;
			break;
		default :
			SI_ERROR(("Invalid  size %d \n", size));
			return (BCME_ERROR);
			break;
	}

#if !defined(DONGLEBUILD)
	REG_UNMAP(va);
#endif /* !DONGLEBUILD */

	return (BCME_OK);
} /* si_backplane_access_si */

/* Configure PCIE2_BAR0_CORE2_WIN2 register with the required address so the
 * sixth 4KB region of BAR 0 space points to the given address region.
 * The register PCIE2_BAR0_CORE2_WIN2 is used to access M2M or HWA core register base in NIC mode.
 * But only one of these two (HWA or M2M) features are enabled so there is no conflict.
 */
uint
si_backplane_pci_config_bar0_core2_win2(si_t *sih, uint32 addr_pa)
{
	si_info_t *sii = SI_INFO(sih);
	uint32 region = 0;
	region = (addr_pa & (0xFFFFF << 12));

	ASSERT(BUSTYPE(sih->bustype) == PCI_BUS);
	OSL_PCI_WRITE_CONFIG(sii->osh, PCIE2_BAR0_CORE2_WIN2, 4, region);

	return BCME_OK;
}

/* Configure PCIE_ENUM_BAR0_WIN_EXTx register to map tenth (onwards)
 * 4KB region of BAR0 space to the given address
 */
uint
si_backplane_pci_config_bar0_ext_win(si_t *sih, uint32 addr_pa, uint32 ext_idx)
{
	uint32 region = 0;
	region = (addr_pa & (0xFFFFF << 12));

	/* Ext window is supported from pcie rev 132 */
	ASSERT(sih->buscorerev >= 132);
	ASSERT(BUSTYPE(sih->bustype) == PCI_BUS);

	/* Extended window is configured through enummeration space registers,
	 * 0x6D0-0x6E8. Enumeration space needs to be mapped through BAR
	 * before accessing it
	 */
	switch (ext_idx) {
		case 0:
			si_corereg(sih, sih->buscoreidx, PCIE_ENUM_BAR0_WIN_EXT0, ~0, region);
			break;
		case 1:
			si_corereg(sih, sih->buscoreidx, PCIE_ENUM_BAR0_WIN_EXT1, ~0, region);
			break;
		case 2:
			si_corereg(sih, sih->buscoreidx, PCIE_ENUM_BAR0_WIN_EXT2, ~0, region);
			break;
		case 3:
			si_corereg(sih, sih->buscoreidx, PCIE_ENUM_BAR0_WIN_EXT3, ~0, region);
			break;
		case 4:
			si_corereg(sih, sih->buscoreidx, PCIE_ENUM_BAR0_WIN_EXT4, ~0, region);
			break;
		case 5:
			si_corereg(sih, sih->buscoreidx, PCIE_ENUM_BAR0_WIN_EXT5, ~0, region);
			break;
		case 6:
			si_corereg(sih, sih->buscoreidx, PCIE_ENUM_BAR0_WIN_EXT6, ~0, region);
			break;
		default:
			ASSERT(0);
			break;
	}

	return BCME_OK;
} // si_backplane_pci_config_bar0_ext_win

/**
 * si_backplane_access_pci is used to read full backplane address from host for PCIE FD
 * it uses secondary bar-0 window which lies at an offset of 16K from primary bar-0
 * Provides support for read/write of 1/2/4 bytes of backplane address
 * Can be used to read/write
 *	1. core regs
 *	2. Wrapper regs
 *	3. memory
 *	4. BT area
 * For accessing any 32 bit backplane address, [31 : 12] of backplane should be given in "region"
 * [11 : 0] should be the "regoff"
 */

/** read or write to a given physical backplane address, PCI_BUS variant */
static uint
si_backplane_access_pci(si_info_t *sii, uint addr_pa, uint size, uint *val, bool read)
{
	volatile uint32 *r = NULL;
	uint32 region = 0;
	/* Split adrr into region and address offset */
	region = (addr_pa & (0xFFFFF << 12));
	addr_pa = addr_pa & 0xFFF;

	/* check for address and size sanity */
	if (si_backplane_addr_sane(addr_pa, size) != BCME_OK)
		return BCME_ERROR;

	/* Update window if required */
	if (sii->second_bar0win != region) {
		OSL_PCI_WRITE_CONFIG(sii->osh, PCIE2_BAR0_CORE2_WIN, 4, region);
		sii->second_bar0win = region;
	}

	/* Estimate effective address
	 * sii->curmap   : bar-0 virtual address
	 * PCI_SECOND_BAR0_OFFSET  : secondar bar-0 offset
	 * regoff : actual reg offset
	 */
	r = (volatile uint32 *)((volatile char *)sii->curmap + PCI_SECOND_BAR0_OFFSET + addr_pa);

	SI_VMSG(("si curmap %p  region %x regaddr %x effective addr %p READ %d\n",
		(volatile char*)sii->curmap, region, addr_pa, r, read));

	switch (size) {
		case sizeof(uint8) :
			if (read)
				*val = R_REG(sii->osh, (volatile uint8*)r);
			else
				W_REG(sii->osh, (volatile uint8*)r, (uint8)*val);
			break;
		case sizeof(uint16) :
			if (read)
				*val = R_REG(sii->osh, (volatile uint16*)r);
			else
				W_REG(sii->osh, (volatile uint16*)r, (uint16)*val);
			break;
		case sizeof(uint32) :
			if (read)
				*val = R_REG(sii->osh, (volatile uint32*)r);
			else
				W_REG(sii->osh, (volatile uint32*)r, (uint32)*val);
			break;
		default :
			SI_ERROR(("Invalid  size %d \n", size));
			return (BCME_ERROR);
			break;
	}

	return (BCME_OK);
} /* si_backplane_access_pci */

/**
 * @param[in] addr_pa  Physical address on the backplane, from the backplane's point of view.
 * @return             BCME_OK if succesfull.
 */
uint
si_backplane_access(si_t *sih, uint addr_pa, uint size, uint *val, bool read)
{
	si_info_t *sii = SI_INFO(sih);

	if (BUSTYPE(sih->bustype) == SI_BUS) {
		ASSERT(CHIPTYPE(sii->pub.socitype) == SOCI_AI);
		return si_backplane_access_si(sih, addr_pa, size, val, read);
	} else if (BUSTYPE(sih->bustype) == PCI_BUS) {
		return si_backplane_access_pci(sii, addr_pa, size, val, read);
	} else {
		SI_ERROR(("Valid only for si and pcie bus\n"));
	}

	return BCME_ERROR;
}

uint
si_corereg(si_t *sih, uint coreidx, uint regoff, uint mask, uint val)
{
	if ((CHIPTYPE(sih->socitype) == SOCI_AI) ||
		(CHIPTYPE(sih->socitype) == SOCI_DVTBUS) ||
		(CHIPTYPE(sih->socitype) == SOCI_NAI)) {
		return ai_corereg(sih, coreidx, regoff, mask, val);
	} else {
		ASSERT(0);
		return 0;
	}
}

/** ILP sensitive register access needs special treatment to avoid backplane stalls */
bool si_pmu_is_ilp_sensitive(uint32 idx, uint regoff)
{
	if (idx == SI_CC_IDX) {
		if (CHIPCREGS_ILP_SENSITIVE(regoff))
			return TRUE;
	} else if (PMUREGS_ILP_SENSITIVE(regoff)) {
		return TRUE;
	}

	return FALSE;
}

/** 'idx' should refer either to the chipcommon core or the PMU core */
uint
si_pmu_corereg(si_t *sih, uint32 idx, uint regoff, uint mask, uint val)
{
	int pmustatus_offset;

	/* prevent backplane stall on double write to 'ILP domain' registers in the PMU */
	if (mask != 0 && PMUREV(sih->pmurev) >= 22 &&
	    si_pmu_is_ilp_sensitive(idx, regoff)) {
		pmustatus_offset = OFFSETOF(pmuregs_t, pmustatus);

		while (si_corereg(sih, idx, pmustatus_offset, 0, 0) & PST_SLOW_WR_PENDING)
			{};
	}

	return si_corereg(sih, idx, regoff, mask, val);
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
volatile uint32 *
si_corereg_addr(si_t *sih, uint coreidx, uint regoff)
{
	if ((CHIPTYPE(sih->socitype) == SOCI_AI) ||
		(CHIPTYPE(sih->socitype) == SOCI_DVTBUS) ||
		(CHIPTYPE(sih->socitype) == SOCI_NAI))
		return ai_corereg_addr(sih, coreidx, regoff);
	else {
		return 0;
	}
}

void
si_core_disable(si_t *sih, uint32 bits)
{
	if ((CHIPTYPE(sih->socitype) == SOCI_AI) ||
		(CHIPTYPE(sih->socitype) == SOCI_DVTBUS) ||
		(CHIPTYPE(sih->socitype) == SOCI_NAI))
		ai_core_disable(sih, bits);
}

/**
 * Resets the currently selected core by resetting one or more wrappers. Reset is lifted when this
 * function returns.
 */
void
si_core_reset(si_t *sih, uint32 bits, uint32 resetbits)
{
	if ((CHIPTYPE(sih->socitype) == SOCI_AI) ||
		(CHIPTYPE(sih->socitype) == SOCI_DVTBUS) ||
		(CHIPTYPE(sih->socitype) == SOCI_NAI))
		ai_core_reset(sih, bits, resetbits);
}

/** Run bist on current core. Caller needs to take care of core-specific bist hazards */
int
si_corebist(si_t *sih)
{
	uint32 cflags;
	int result = 0;

	/* Read core control flags */
	cflags = si_core_cflags(sih, 0, 0);

	/* Set bist & fgc */
	si_core_cflags(sih, ~0, (SICF_BIST_EN | SICF_FGC));

	/* Wait for bist done */
	SPINWAIT(((si_core_sflags(sih, 0, 0) & SISF_BIST_DONE) == 0), 100000);

	if (si_core_sflags(sih, 0, 0) & SISF_BIST_ERROR)
		result = BCME_ERROR;

	/* Reset core control flags */
	si_core_cflags(sih, 0xffff, cflags);

	return result;
}

uint
si_num_slaveports(si_t *sih, uint coreid)
{
	uint idx = si_findcoreidx(sih, coreid, 0);
	uint num = 0;

	if (CHIPTYPE(sih->socitype) == SOCI_AI) {
		num = ai_num_slaveports(sih, idx);
	}

	return num;
}

uint32
si_get_slaveport_addr(si_t *sih, uint spidx, uint baidx, uint core_id, uint coreunit)
{
	si_info_t *sii = SI_INFO(sih);
	uint origidx = sii->curidx;
	uint32 addr_pa = 0x0;

	if (!((CHIPTYPE(sih->socitype) == SOCI_AI) ||
		(CHIPTYPE(sih->socitype) == SOCI_DVTBUS) ||
		(CHIPTYPE(sih->socitype) == SOCI_NAI)))
		goto done;

	si_setcore(sih, core_id, coreunit);

	addr_pa = ai_addrspace(sih, spidx, baidx);

	si_setcoreidx(sih, origidx);

done:
	return addr_pa;
}

uint32
si_get_d11_slaveport_addr(si_t *sih, uint spidx, uint baidx, uint coreunit)
{
	si_info_t *sii = SI_INFO(sih);
	uint origidx = sii->curidx;
	uint32 addr_pa = 0x0;

	if (!((CHIPTYPE(sih->socitype) == SOCI_AI) ||
		(CHIPTYPE(sih->socitype) == SOCI_DVTBUS) ||
		(CHIPTYPE(sih->socitype) == SOCI_NAI)))
		goto done;

	si_setcore(sih, D11_CORE_ID, coreunit);

	addr_pa = ai_addrspace(sih, spidx, baidx);

	si_setcoreidx(sih, origidx);

done:
	return addr_pa;
}

static uint32
BCMINITFN(factor6)(uint32 x)
{
	switch (x) {
	case CC_F6_2:	return 2;
	case CC_F6_3:	return 3;
	case CC_F6_4:	return 4;
	case CC_F6_5:	return 5;
	case CC_F6_6:	return 6;
	case CC_F6_7:	return 7;
	default:	return 0;
	}
}

/*
 * Divide the clock by the divisor with protection for
 * a zero divisor.
 */
static uint32
divide_clock(uint32 clock, uint32 div)
{
	return div ? clock / div : 0;
}

/** calculate the speed the SI would run at given a set of clockcontrol values */
uint32
BCMINITFN(si_clock_rate)(uint32 pll_type, uint32 n, uint32 m)
{
	uint32 n1, n2, clock, m1, m2, m3, mc;

	n1 = n & CN_N1_MASK;
	n2 = (n & CN_N2_MASK) >> CN_N2_SHIFT;

	if (pll_type == PLL_TYPE6) {
		if (m & CC_T6_MMASK)
			return CC_T6_M1;
		else
			return CC_T6_M0;
	} else if ((pll_type == PLL_TYPE1) ||
	           (pll_type == PLL_TYPE3) ||
	           (pll_type == PLL_TYPE4) ||
	           (pll_type == PLL_TYPE7)) {
		n1 = factor6(n1);
		n2 += CC_F5_BIAS;
	} else if (pll_type == PLL_TYPE2) {
		n1 += CC_T2_BIAS;
		n2 += CC_T2_BIAS;
		ASSERT((n1 >= 2) && (n1 <= 7));
		ASSERT((n2 >= 5) && (n2 <= 23));
	} else if (pll_type == PLL_TYPE5) {
		return (100000000);
	} else
		ASSERT(0);
	/* PLL types 3 and 7 use BASE2 (25Mhz) */
	if ((pll_type == PLL_TYPE3) ||
	    (pll_type == PLL_TYPE7)) {
		clock = CC_CLOCK_BASE2 * n1 * n2;
	} else
		clock = CC_CLOCK_BASE1 * n1 * n2;

	if (clock == 0)
		return 0;

	m1 = m & CC_M1_MASK;
	m2 = (m & CC_M2_MASK) >> CC_M2_SHIFT;
	m3 = (m & CC_M3_MASK) >> CC_M3_SHIFT;
	mc = (m & CC_MC_MASK) >> CC_MC_SHIFT;

	if ((pll_type == PLL_TYPE1) ||
	    (pll_type == PLL_TYPE3) ||
	    (pll_type == PLL_TYPE4) ||
	    (pll_type == PLL_TYPE7)) {
		m1 = factor6(m1);
		if ((pll_type == PLL_TYPE1) || (pll_type == PLL_TYPE3))
			m2 += CC_F5_BIAS;
		else
			m2 = factor6(m2);
		m3 = factor6(m3);

		switch (mc) {
		case CC_MC_BYPASS:	return (clock);
		case CC_MC_M1:		return divide_clock(clock, m1);
		case CC_MC_M1M2:	return divide_clock(clock, m1 * m2);
		case CC_MC_M1M2M3:	return divide_clock(clock, m1 * m2 * m3);
		case CC_MC_M1M3:	return divide_clock(clock, m1 * m3);
		default:		return (0);
		}
	} else {
		ASSERT(pll_type == PLL_TYPE2);

		m1 += CC_T2_BIAS;
		m2 += CC_T2M2_BIAS;
		m3 += CC_T2_BIAS;
		ASSERT((m1 >= 2) && (m1 <= 7));
		ASSERT((m2 >= 3) && (m2 <= 10));
		ASSERT((m3 >= 2) && (m3 <= 7));

		if ((mc & CC_T2MC_M1BYP) == 0)
			clock /= m1;
		if ((mc & CC_T2MC_M2BYP) == 0)
			clock /= m2;
		if ((mc & CC_T2MC_M3BYP) == 0)
			clock /= m3;

		return (clock);
	}
} /* si_clock_rate */

/**
 * Some chips could have multiple host interfaces, however only one will be active.
 * For a given chip. Depending pkgopt and cc_chipst return the active host interface.
 */
uint
si_chip_hostif(si_t *sih)
{
	uint hosti = 0;

	return hosti;
} /* si_chip_hostif */

uint32
BCMINITFN(si_clock)(si_t *sih)
{
	si_info_t *sii = SI_INFO(sih);
	uint32 rate;
	uint intr_val = 0;

	INTR_OFF(sii, intr_val);
	rate = si_pmu_get_clock(sih, sii->osh, SI_PMU_CLK_BACKPLANE);
	INTR_RESTORE(sii, intr_val);

	return rate;
} /* si_clock */

/** returns value in [Hz] units */
uint32
BCMINITFN(si_alp_clock)(si_t *sih)
{
	uint32 alp_hz;

	alp_hz = si_pmu_get_clock(sih, si_osh(sih), SI_PMU_CLK_ALP);

	return alp_hz;
}

uint32
BCMINITFN(si_xtal_freq)(si_t *sih)
{
	uint32 xtal_hz;

	xtal_hz = si_pmu_get_clock(sih, si_osh(sih), SI_PMU_CLK_XTAL);

	return xtal_hz;
}

/** returns value in [Hz] units */
uint32
BCMINITFN(si_ilp_clock)(si_t *sih)
{
	return si_pmu_get_clock(sih, si_osh(sih), SI_PMU_CLK_ILP);
}

/** set chip watchdog reset timer to fire in 'ticks' */
void
si_watchdog(si_t *sih, uint ticks)
{
	uint maxt;
	uint pmu_wdt = 1;

	if (BCM43684_CHIP(sih->chip)) {
		/* Resetting AVS requires the use of the CC watchdog */
		pmu_wdt = 0;
	}

	if (pmu_wdt) {
		if (ticks == 1)
			ticks = 2;

		pmu_corereg(sih, SI_CC_IDX, pmuwatchdog, ~0, ticks);
	} else {
#if !defined(BCMDONGLEHOST)
		if (!BCM43684_CHIP(sih->chip) && !EMBEDDED_2x2AX_CORE(sih->chip)) {
			/* make sure we come up in fast clock mode; or if clearing, clear clock */
			si_clkctl_cc(sih, ticks ? CLK_FAST : CLK_DYNAMIC);
		}
#endif /* !defined(BCMDONGLEHOST) */
		maxt = (1 << 28) - 1;
		if (ticks > maxt)
			ticks = maxt;

		si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, watchdog), ~0, ticks);
	}
}

/** trigger watchdog reset after ms milliseconds */
void
si_watchdog_ms(si_t *sih, uint32 ms)
{
	si_watchdog(sih, wd_msticks * ms);
}

uint32 si_watchdog_msticks(void)
{
	return wd_msticks;
}

#if !defined(BCMDONGLEHOST)
uint16
BCMATTACHFN(si_d11_devid)(si_t *sih)
{
	si_info_t *sii = SI_INFO(sih);
	uint16 device;

	/* normal case: nvram variable with devpath->devid->wl0id */
	if ((device = (uint16)si_getdevpathintvar(sih, rstr_devid)) != 0)
		;
	/* Get devid from OTP/SPROM depending on where the SROM is read */
	else if ((device = (uint16)getintvar(sii->vars, rstr_devid)) != 0)
		;
	/* no longer support wl0id, but keep the code here for backward compatibility. */
	else if ((device = (uint16)getintvar(sii->vars, rstr_wl0id)) != 0)
		;
	else {
		/* ignore it */
		device = 0xffff;
	}
	return device;
}

int
BCMATTACHFN(si_corepciid)(si_t *sih, uint func, uint16 *pcivendor, uint16 *pcidevice,
                          uint8 *pciclass, uint8 *pcisubclass, uint8 *pciprogif,
                          uint8 *pciheader)
{
	uint16 vendor = 0xffff, device = 0xffff;
	uint8 class, subclass, progif = 0;
	uint8 header = PCI_HEADER_NORMAL;
	uint32 core = si_coreid(sih);

	/* Verify whether the function exists for the core */
	if (func >= (uint)((core == USB20H_CORE_ID) || (core == NS_USB20_CORE_ID) ? 2 : 1))
		return BCME_ERROR;

	/* Known vendor translations */
	switch (si_corevendor(sih)) {
	case SB_VEND_BCM:
	case MFGID_BRCM:
		vendor = VENDOR_BROADCOM;
		break;
	default:
		return BCME_ERROR;
	}

	/* Determine class based on known core codes */
	switch (core) {
	case ENET_CORE_ID:
		class = PCI_CLASS_NET;
		subclass = PCI_NET_ETHER;
		device = BCM47XX_ENET_ID;
		break;
	case GIGETH_CORE_ID:
		class = PCI_CLASS_NET;
		subclass = PCI_NET_ETHER;
		device = BCM47XX_GIGETH_ID;
		break;
	case GMAC_CORE_ID:
		class = PCI_CLASS_NET;
		subclass = PCI_NET_ETHER;
		device = BCM47XX_GMAC_ID;
		break;
	case SDRAM_CORE_ID:
	case MEMC_CORE_ID:
	case DMEMC_CORE_ID:
	case SOCRAM_CORE_ID:
		class = PCI_CLASS_MEMORY;
		subclass = PCI_MEMORY_RAM;
		device = (uint16)core;
		break;
	case PCI_CORE_ID:
	case PCIE_CORE_ID:
	case PCIE2_CORE_ID:
		class = PCI_CLASS_BRIDGE;
		subclass = PCI_BRIDGE_PCI;
		device = (uint16)core;
		header = PCI_HEADER_BRIDGE;
		break;
	case MIPS33_CORE_ID:
	case MIPS74K_CORE_ID:
		class = PCI_CLASS_CPU;
		subclass = PCI_CPU_MIPS;
		device = (uint16)core;
		break;
	case CODEC_CORE_ID:
		class = PCI_CLASS_COMM;
		subclass = PCI_COMM_MODEM;
		device = BCM47XX_V90_ID;
		break;
	case I2S_CORE_ID:
		class = PCI_CLASS_MMEDIA;
		subclass = PCI_MMEDIA_AUDIO;
		device = BCM47XX_AUDIO_ID;
		break;
	case USB_CORE_ID:
	case USB11H_CORE_ID:
		class = PCI_CLASS_SERIAL;
		subclass = PCI_SERIAL_USB;
		progif = 0x10; /* OHCI */
		device = BCM47XX_USBH_ID;
		break;
	case USB20H_CORE_ID:
	case NS_USB20_CORE_ID:
		class = PCI_CLASS_SERIAL;
		subclass = PCI_SERIAL_USB;
		progif = func == 0 ? 0x10 : 0x20; /* OHCI/EHCI value defined in spec */
		device = BCM47XX_USB20H_ID;
		header = PCI_HEADER_MULTI; /* multifunction */
		break;
	case IPSEC_CORE_ID:
		class = PCI_CLASS_CRYPT;
		subclass = PCI_CRYPT_NETWORK;
		device = BCM47XX_IPSEC_ID;
		break;
	case NS_USB30_CORE_ID:
		class = PCI_CLASS_SERIAL;
		subclass = PCI_SERIAL_USB;
		progif = 0x30; /* XHCI */
		device = BCM47XX_USB30H_ID;
		break;
	case ROBO_CORE_ID:
		/* Don't use class NETWORK, so wl/et won't attempt to recognize it */
		class = PCI_CLASS_COMM;
		subclass = PCI_COMM_OTHER;
		device = BCM47XX_ROBO_ID;
		break;
	case CC_CORE_ID:
		class = PCI_CLASS_MEMORY;
		subclass = PCI_MEMORY_FLASH;
		device = (uint16)core;
		break;
	case SATAXOR_CORE_ID:
		class = PCI_CLASS_XOR;
		subclass = PCI_XOR_QDMA;
		device = BCM47XX_SATAXOR_ID;
		break;
	case ATA100_CORE_ID:
		class = PCI_CLASS_DASDI;
		subclass = PCI_DASDI_IDE;
		device = BCM47XX_ATA100_ID;
		break;
	case USB11D_CORE_ID:
		class = PCI_CLASS_SERIAL;
		subclass = PCI_SERIAL_USB;
		device = BCM47XX_USBD_ID;
		break;
	case USB20D_CORE_ID:
		class = PCI_CLASS_SERIAL;
		subclass = PCI_SERIAL_USB;
		device = BCM47XX_USB20D_ID;
		break;
	case D11_CORE_ID:
		class = PCI_CLASS_NET;
		subclass = PCI_NET_OTHER;
		device = si_d11_devid(sih);
		break;

	default:
		class = subclass = progif = 0xff;
		device = (uint16)core;
		break;
	}

	*pcivendor = vendor;
	*pcidevice = device;
	*pciclass = class;
	*pcisubclass = subclass;
	*pciprogif = progif;
	*pciheader = header;

	return 0;
} /* si_corepciid */

#if defined(BCMDBG) || defined(BCMDBG_PHYDUMP)
/** print interesting sbconfig registers */
void
si_dumpregs(si_t *sih, struct bcmstrbuf *b)
{
	si_info_t *sii = SI_INFO(sih);
	uint origidx, intr_val = 0;

	origidx = sii->curidx;

	INTR_OFF(sii, intr_val);
	if ((CHIPTYPE(sih->socitype) == SOCI_AI) ||
		(CHIPTYPE(sih->socitype) == SOCI_DVTBUS) ||
		(CHIPTYPE(sih->socitype) == SOCI_NAI))
		ai_dumpregs(sih, b);
	else
		ASSERT(0);

	si_setcoreidx(sih, origidx);
	INTR_RESTORE(sii, intr_val);
}
#endif
#endif /* !defined(BCMDONGLEHOST) */

#ifdef BCMDBG
void
si_view(si_t *sih, bool verbose)
{
	if ((CHIPTYPE(sih->socitype) == SOCI_AI) ||
		(CHIPTYPE(sih->socitype) == SOCI_DVTBUS) ||
		(CHIPTYPE(sih->socitype) == SOCI_NAI))
		ai_view(sih, verbose);
	else
		ASSERT(0);
}

void
si_viewall(si_t *sih, bool verbose)
{
	si_info_t *sii = SI_INFO(sih);
	uint curidx, i;
	uint intr_val = 0;

	curidx = sii->curidx;

	INTR_OFF(sii, intr_val);
	if ((CHIPTYPE(sih->socitype) == SOCI_AI) ||
		(CHIPTYPE(sih->socitype) == SOCI_DVTBUS) ||
		(CHIPTYPE(sih->socitype) == SOCI_NAI)) {
		ai_viewall(sih, verbose);
	} else {
		SI_ERROR(("si_viewall: num_cores %d\n", sii->numcores));
		for (i = 0; i < sii->numcores; i++) {
			si_setcoreidx(sih, i);
			si_view(sih, verbose);
		}
	}
	si_setcoreidx(sih, curidx);
	INTR_RESTORE(sii, intr_val);
}
#endif	/* BCMDBG */

/** read/writes a register in chipcommon or PMU core */
uint32
si_ccreg(si_t *sih, uint32 offset, uint32 mask, uint32 val)
{
	si_info_t *sii;
	uint32 reg_val = 0;
	int coreidx = SI_CC_IDX;

	sii = SI_INFO(sih);

	if (offset > sizeof(chipcregs_t) || offset >= PMU_CTL) {
		if (offset < sizeof(pmuregs_t)) {
			coreidx = si_findcoreidx(sih, PMU_CORE_ID, 0);
		} else {
			return 0; /* invalid register offset */
		}
	}

	reg_val = si_corereg(&sii->pub, coreidx, offset, mask, val);

	return reg_val;
}

/** return the ILP (slowclock) min or max frequency */
static uint
si_slowclk_freq(si_info_t *sii, chipcregs_t *cc)
{
	uint div;

	ASSERT(SI_FAST(sii) || si_coreid(&sii->pub) == CC_CORE_ID);

	/* shouldn't be here unless we've established the chip has dynamic clk control */
	ASSERT(R_REG(sii->osh, &cc->capabilities) & CC_CAP_PWR_CTL);

	/* Chipc rev 10 is InstaClock */
	div = R_REG(sii->osh, &cc->system_clk_ctl) >> SYCC_CD_SHIFT;
	div = 4 * (div + 1);

	return (XTALMINFREQ / div);
}

static void
BCMINITFN(si_clkctl_setdelay)(si_info_t *sii, void *chipcregs)
{
	chipcregs_t *cc = (chipcregs_t *)chipcregs;
	uint slowmaxfreq, pll_delay;
	uint pll_on_delay, fref_sel_delay;

	pll_delay = PLL_DELAY;

	/* Starting with 4318 it is ILP that is used for the delays */
	slowmaxfreq = si_slowclk_freq(sii, cc);

	pll_on_delay = ((slowmaxfreq * pll_delay) + 999999) / 1000000;
	fref_sel_delay = ((slowmaxfreq * FREF_DELAY) + 999999) / 1000000;

	W_REG(sii->osh, &cc->pll_on_delay, pll_on_delay);
	W_REG(sii->osh, &cc->fref_sel_delay, fref_sel_delay);
}

/** initialize power control delay registers */
void
BCMINITFN(si_clkctl_init)(si_t *sih)
{
	si_info_t *sii;
	uint origidx = 0;
	chipcregs_t *cc;
	bool fast;

	if (!CCCTL_ENAB(sih))
		return;

	sii = SI_INFO(sih);
	fast = SI_FAST(sii);
	if (!fast) {
		origidx = sii->curidx;
		if ((cc = (chipcregs_t *)si_setcore(sih, CC_CORE_ID, 0)) == NULL)
			return;
	} else if ((cc = (chipcregs_t *)CCREGS_FAST(sii)) == NULL)
		return;
	ASSERT(cc != NULL);

	/* set all Instaclk chip ILP to 1 MHz */
	SET_REG(sii->osh, &cc->system_clk_ctl, SYCC_CD_MASK, (ILP_DIV_1MHZ << SYCC_CD_SHIFT));

	si_clkctl_setdelay(sii, (void *)(uintptr)cc);

	OSL_DELAY(20000);

	if (!fast)
		si_setcoreidx(sih, origidx);
}

#if !defined(BCMDONGLEHOST)
/**
 * d11 core has a 'fastpwrup_dly' register that must be written to.
 * This function returns d11 slow to fast clock transition time in [us] units.
 * It does not write to the d11 core.
 */
uint16
BCMINITFN(si_clkctl_fast_pwrup_delay)(si_t *sih)
{
	si_info_t *sii = SI_INFO(sih);
	uint16 fpdelay_us;
	uint intr_val = 0;

	INTR_OFF(sii, intr_val);
	fpdelay_us = si_pmu_fast_pwrup_delay(sih, sii->osh);
	INTR_RESTORE(sii, intr_val);

	return fpdelay_us;
}

/** turn primary xtal and/or pll off/on */
int
si_clkctl_xtal(si_t *sih, uint what, bool on)
{
	si_info_t *sii;
	uint32 in, out, outen;

	sii = SI_INFO(sih);

	switch (BUSTYPE(sih->bustype)) {

	case PCI_BUS:
		/* pcie core doesn't have any mapping to control the xtal pu */
		if (PCIE(sii) || PCIE_GEN2(sii))
			return -1;

		in = OSL_PCI_READ_CONFIG(sii->osh, PCI_GPIO_IN, sizeof(uint32));
		out = OSL_PCI_READ_CONFIG(sii->osh, PCI_GPIO_OUT, sizeof(uint32));
		outen = OSL_PCI_READ_CONFIG(sii->osh, PCI_GPIO_OUTEN, sizeof(uint32));

		/*
		 * Avoid glitching the clock if GPRS is already using it.
		 * We can't actually read the state of the PLLPD so we infer it
		 * by the value of XTAL_PU which *is* readable via gpioin.
		 */
		if (on && (in & PCI_CFG_GPIO_XTAL))
			return (0);

		if (what & XTAL)
			outen |= PCI_CFG_GPIO_XTAL;
		if (what & PLL)
			outen |= PCI_CFG_GPIO_PLL;

		if (on) {
			/* turn primary xtal on */
			if (what & XTAL) {
				out |= PCI_CFG_GPIO_XTAL;
				if (what & PLL)
					out |= PCI_CFG_GPIO_PLL;
				OSL_PCI_WRITE_CONFIG(sii->osh, PCI_GPIO_OUT,
				                     sizeof(uint32), out);
				OSL_PCI_WRITE_CONFIG(sii->osh, PCI_GPIO_OUTEN,
				                     sizeof(uint32), outen);
				OSL_DELAY(XTAL_ON_DELAY);
			}

			/* turn pll on */
			if (what & PLL) {
				out &= ~PCI_CFG_GPIO_PLL;
				OSL_PCI_WRITE_CONFIG(sii->osh, PCI_GPIO_OUT,
				                     sizeof(uint32), out);
				OSL_DELAY(2000);
			}
		} else {
			if (what & XTAL)
				out &= ~PCI_CFG_GPIO_XTAL;
			if (what & PLL)
				out |= PCI_CFG_GPIO_PLL;
			OSL_PCI_WRITE_CONFIG(sii->osh, PCI_GPIO_OUT, sizeof(uint32), out);
			OSL_PCI_WRITE_CONFIG(sii->osh, PCI_GPIO_OUTEN, sizeof(uint32),
			                     outen);
		}
		return 0;

	default:
		return (-1);
	}

	return (0);
} /* si_clkctl_xtal */

/**
 *  clock control policy function throught chipcommon
 *
 *    set dynamic clk control mode (forceslow, forcefast, dynamic)
 *    returns true if we are forcing fast clock
 *    this is a wrapper over the next internal function
 *      to allow flexible policy settings for outside caller
 */
bool
si_clkctl_cc(si_t *sih, uint mode)
{
	si_info_t *sii;

	sii = SI_INFO(sih);

	return _si_clkctl_cc(sii, mode);
}

/* clk control mechanism through chipcommon, no policy checking */
static bool
_si_clkctl_cc(si_info_t *sii, uint mode)
{
	uint origidx = 0;
	chipcregs_t *cc;
	uint intr_val = 0;
	bool fast = SI_FAST(sii);

	if (!fast) {
		INTR_OFF(sii, intr_val);
		origidx = sii->curidx;
		cc = (chipcregs_t *) si_setcore(&sii->pub, CC_CORE_ID, 0);
	} else if ((cc = (chipcregs_t *) CCREGS_FAST(sii)) == NULL)
		goto done;
	ASSERT(cc != NULL);

	switch (mode) {
	case CLK_FAST:	/* FORCEHT, fast (pll) clock */
		OR_REG(sii->osh, &cc->clk_ctl_st, CCS_FORCEHT);
		/* wait for the PLL */
		{
			uint32 htavail = CCS_HTAVAIL;
			SPINWAIT(((R_REG(sii->osh, &cc->clk_ctl_st) & htavail) == 0),
			         PMU_MAX_TRANSITION_DLY);
			ASSERT(R_REG(sii->osh, &cc->clk_ctl_st) & htavail);
		}
		break;

	case CLK_DYNAMIC:	/* enable dynamic clock control */
		AND_REG(sii->osh, &cc->clk_ctl_st, ~CCS_FORCEHT);

		/* wait for the PLL */
		{
			uint32 htavail = CCS_HTAVAIL;
			SPINWAIT(((R_REG(sii->osh, &cc->clk_ctl_st) & htavail) != 0),
			         PMU_MAX_TRANSITION_DLY);
			ASSERT(!(R_REG(sii->osh, &cc->clk_ctl_st) & htavail));
		}

		break;

	default:
		ASSERT(0);
	}

done:
	if (!fast) {
		si_setcoreidx(&sii->pub, origidx);
		INTR_RESTORE(sii, intr_val);
	}
	return (mode == CLK_FAST);
} /* _si_clkctl_cc */

/**
 * A NIC driver may support multiple device instances. Each device instance may have its own set of
 * nvram variables. To support this, the nvram file has a special format: each nvram variable is
 * prepended with a prefix. This function returns that prefix for a particular device instance,
 * making it possible to filter the nvram variables intended for this device out of the nvram file.
 *
 * Supports SI, PCI, and JTAG for now.
 *
 * Prerequisites:
 * - The CC core has been selected
 */
int
BCMNMIATTACHFN(si_devpath)(si_t *sih, char *path, int size)
{
	int slen;
	uint idx;

	ASSERT(path != NULL);
	ASSERT(size >= SI_DEVPATH_BUFSZ);

	if (!sih || !path || (size < SI_DEVPATH_BUFSZ)) {
		return -1;
	}

	switch (BUSTYPE(sih->bustype)) {
	case SI_BUS:
	case JTAG_BUS:
		if (EMBEDDED_11BE_CORE(sih->chip)) {
			idx = (sih->enum_base_pa == DUALSLICE_11BEDEV_B_PHYS_ADDR) ?
				DUALSLICE_WLAN_DEV_B : DUALSLICE_WLAN_DEV_A;
		} else if (EMBEDDED_2x2AX_CORE(sih->chip)) {
			idx = (sih->enum_base_pa == DUALSLICE_DEV_B_PHYS_ADDR) ?
				DUALSLICE_WLAN_DEV_B : DUALSLICE_WLAN_DEV_A;
		} else {
			idx = si_coreidx(sih);
		}
		slen = snprintf(path, (size_t)size, "sb/%u/", idx);
		/* returns e.g. 'sb/1/' */
		break;
	case PCI_BUS:
		ASSERT((SI_INFO(sih))->osh != NULL);
		slen = snprintf(path, (size_t)size, "pci/%u/%u/",
		                OSL_PCI_BUS((SI_INFO(sih))->osh),
		                OSL_PCI_SLOT((SI_INFO(sih))->osh));
		break;
	default:
		slen = -1;
		ASSERT(0);
		break;
	}

	if (slen < 0 || slen >= size) {
		path[0] = '\0';
		return -1;
	}

	return 0;
} /* si_devpath */

int
BCMNMIATTACHFN(si_devpath_pcie)(si_t *sih, char *path, int size)
{
	ASSERT(path != NULL);
	ASSERT(size >= SI_DEVPATH_BUFSZ);

	if (!path || size <= 0)
		return -1;

	ASSERT((SI_INFO(sih))->osh != NULL);
	snprintf(path, (size_t)size, "pcie/%u/%u/",
		OSL_PCIE_DOMAIN((SI_INFO(sih))->osh),
		OSL_PCIE_BUS((SI_INFO(sih))->osh));

	return 0;
}

/**
 * Nvram files can contain prefixes to address different WLAN devices/slices. A prefixed varname is
 * e.g. 'sb/0/myvariable'. There is an optional feature to reduce nvram file size by coding repeated
 * prefixes into a shorter prefix, known as a 'compact devpath'. Example nvram file using this:
 *     devpath0=sb/0/               <--- only one time, the prefix is defined
 *     0:venid=0x14e4               <--- now the '0' refers to 'sb/0/' (the contents of devpath0)
 *
 * When the firmware is looking for an nvram variable for a specific device/slice, it needs to know
 * the prefix to use. This function will only return 'success' if the 'compact devpath' mechanism is
 * used in the nvram file.
 *
 * @param name[in]      A variable name, without prefix.
 * @param varname[out]  A caller allocated buffer that is written to by this function on success.
 * @return              The caller allocated buffer on success, containing a prefixed variable name
 *
 * Prerequisites:
 * - The CC core has been selected
 */
char *
BCMATTACHFN(si_compact_devpathvar)(si_t *sih, char *varname, int var_len, const char *name)
{
	char pathname[SI_DEVPATH_BUFSZ + 32];
	char full_devpath[SI_DEVPATH_BUFSZ + 32];
	char devpath_pcie[SI_DEVPATH_BUFSZ + 32];
	char *p;
	int idx;
	int len1;
	int len2;
	int len3 = 0;

	if (BUSTYPE(sih->bustype) == PCI_BUS) {
		snprintf(devpath_pcie, SI_DEVPATH_BUFSZ, "pcie/%u/%u",
			OSL_PCIE_DOMAIN((SI_INFO(sih))->osh),
			OSL_PCIE_BUS((SI_INFO(sih))->osh));
		len3 = strlen(devpath_pcie);
	}

	/* try to get compact devpath if it exist */
	if (si_devpath(sih, full_devpath, SI_DEVPATH_BUFSZ) == 0) {
		len1 = strlen(full_devpath);
		if (full_devpath[len1 - 1] == '/')
			len1--;

		for (idx = 0; idx < SI_MAXCORES; idx++) {
			snprintf(pathname, SI_DEVPATH_BUFSZ, rstr_devpathD, idx);
			if ((p = getvar(NULL, pathname)) == NULL) {
				/* no line with eg 'devpath0=sb/0/' present in nvram file */
				continue;
			}

			/* eliminate ending '/' (if present) from contents of devpathX variable */
			len2 = strlen(p);
			if (p[len2 - 1] == '/')
				len2--;

			/* check that both lengths match and if so compare */
			/* the strings (minus trailing '/'s if present */
			if ((len1 == len2) && (memcmp(p, full_devpath, len1) == 0)) {
				snprintf(varname, var_len, rstr_D_S, idx, name);
				return varname;
			}

			/* try the new PCIe devpath format if it exists */
			if (len3 && (len3 == len2) && (memcmp(p, devpath_pcie, len3) == 0)) {
				snprintf(varname, var_len, rstr_D_S, idx, name);
				return varname;
			}
		}
	}

	return NULL;
} /* si_compact_devpathvar */

/**
 * Get a variable, but only if it has a devpath prefix
 *
 * Prerequisites:
 * - The CC core has been selected
 */
char *
BCMATTACHFN(si_getdevpathvar)(si_t *sih, const char *name)
{
	char varname[SI_DEVPATH_BUFSZ + 32];
	char *val;

	si_devpathvar(sih, varname, sizeof(varname), name);

	if ((val = getvar(NULL, varname)) != NULL)
		return val;

	if (BUSTYPE(sih->bustype) == PCI_BUS) {
		si_pcie_devpathvar(sih, varname, sizeof(varname), name);
		if ((val = getvar(NULL, varname)) != NULL)
			return val;
	}

	/* try to get compact devpath if used in nvram file */
	if (si_compact_devpathvar(sih, varname, sizeof(varname), name) == NULL)
		return NULL;

	return (getvar(NULL, varname));
}

/**
 * Gets the value of an nvram variable that uses a devpath prefix.
 *
 * Prerequisites:
 * - The CC core has been selected
 */
int
BCMATTACHFN(si_getdevpathintvar)(si_t *sih, const char *name)
{
#if defined(BCMBUSTYPE) && (BCMBUSTYPE == SI_BUS)
	BCM_REFERENCE(sih);
	return (getintvar(NULL, name));
#else
	char varname[SI_DEVPATH_BUFSZ + 32];
	int val;

	si_devpathvar(sih, varname, sizeof(varname), name);

	if ((val = getintvar(NULL, varname)) != 0)
		return val;

	if (BUSTYPE(sih->bustype) == PCI_BUS) {
		si_pcie_devpathvar(sih, varname, sizeof(varname), name);
		if ((val = getintvar(NULL, varname)) != 0)
			return val;
	}

	/* try to get compact devpath if used in nvram file */
	if (si_compact_devpathvar(sih, varname, sizeof(varname), name) == NULL)
		return 0;

	return (getintvar(NULL, varname));
#endif /* BCMBUSTYPE && BCMBUSTYPE == SI_BUS */
}

/**
 * Concatenate the dev path with a varname into the given 'var' buffer
 * and return the 'var' pointer.
 * Nothing is done to the arguments if len == 0 or var is NULL, var is still returned.
 * On overflow, the first char will be set to '\0'.
 *
 * Prerequisites:
 * - The CC core has been selected
 */
static char *
BCMATTACHFN(si_devpathvar)(si_t *sih, char *var, int len, const char *name)
{
	uint path_len;

	if (!var || len <= 0)
		return var;

	if (si_devpath(sih, var, len) == 0) {
		path_len = strlen(var);

		if (strlen(name) + 1 > (uint)(len - path_len))
			var[0] = '\0';
		else
			strncpy(var + path_len, name, len - path_len - 1);
	}

	return var;
}

static char *
BCMATTACHFN(si_pcie_devpathvar)(si_t *sih, char *var, int len, const char *name)
{
	uint path_len;

	if (!var || len <= 0)
		return var;

	if (si_devpath_pcie(sih, var, len) == 0) {
		path_len = strlen(var);

		if (strlen(name) + 1 > (uint)(len - path_len))
			var[0] = '\0';
		else
			strncpy(var + path_len, name, len - path_len - 1);
	}

	return var;
}

uint32
si_pciereg(si_t *sih, uint32 offset, uint32 mask, uint32 val, uint type)
{
	si_info_t *sii;

	sii = SI_INFO(sih);

	if (!PCIE(sii)) {
		SI_ERROR(("%s: Not a PCIE device\n", __FUNCTION__));
		return 0;
	}

	return pcicore_pciereg(sii->pch, offset, mask, val, type);
}

uint32
si_pcieserdesreg(si_t *sih, uint32 mdioslave, uint32 offset, uint32 mask, uint32 val)
{
	si_info_t *sii;

	sii = SI_INFO(sih);

	if (!PCIE(sii)) {
		SI_ERROR(("%s: Not a PCIE device\n", __FUNCTION__));
		return 0;
	}

	return pcicore_pcieserdesreg(sii->pch, mdioslave, offset, mask, val);

}

/** return TRUE if PCIE capability exists in the pci config space */
static bool
BCMATTACHFN(si_ispcie)(si_info_t *sii)
{
	uint8 cap_ptr;

	if (BUSTYPE(sii->pub.bustype) != PCI_BUS)
		return FALSE;

	cap_ptr = pcicore_find_pci_capability(sii->osh, PCI_CAP_PCIECAP_ID, NULL, NULL);
	if (!cap_ptr)
		return FALSE;

	return TRUE;
}

void
si_pcie_set_maxpayload_size(si_t *sih, uint16 size)
{
	si_info_t *sii = SI_INFO(sih);

	if (!PCIE(sii))
		return;

	pcie_set_maxpayload_size(sii->pch, size);
}

uint16
si_pcie_get_maxpayload_size(si_t *sih)
{
	si_info_t *sii = SI_INFO(sih);

	if (!PCIE(sii))
		return (0);

	return pcie_get_maxpayload_size(sii->pch);
}

void
si_pcie_set_request_size(si_t *sih, uint16 size)
{
	si_info_t *sii = SI_INFO(sih);

	if (!PCIE(sii))
		return;

	pcie_set_request_size(sii->pch, size);
}

uint16
BCMATTACHFN(si_pcie_get_request_size)(si_t *sih)
{
	si_info_t *sii = SI_INFO(sih);

	if (!PCIE_GEN1(sii))
		return (0);

	return pcie_get_request_size(sii->pch);
}

uint16
si_pcie_get_ssid(si_t *sih)
{
	si_info_t *sii = SI_INFO(sih);

	if (!PCIE_GEN1(sii))
		return (0);

	return pcie_get_ssid(sii->pch);
}

uint32
si_pcie_get_bar0(si_t *sih)
{
	si_info_t *sii = SI_INFO(sih);

	if (!PCIE(sii))
		return (0);

	return pcie_get_bar0(sii->pch);
}

/** back door for other module to override chippkg */
void
si_chippkg_set(si_t *sih, uint val)
{
	si_info_t *sii = SI_INFO(sih);

	sii->pub.chippkg = val;
}

void
BCMINITFN(si_pci_up)(si_t *sih)
{
	si_info_t *sii;

	/* if not pci bus, we're done */
	if (BUSTYPE(sih->bustype) != PCI_BUS)
		return;

	sii = SI_INFO(sih);

	if (PCIE(sii)) {
		pcicore_up(sii->pch, SI_PCIUP);
	}
}

/** Unconfigure and/or apply various WARs when system is going to sleep mode */
void
BCMUNINITFN(si_pci_sleep)(si_t *sih)
{
}

/**
 * Configure the pci core for pci client (NIC) action
 * coremask is the bitvec of cores by index to be enabled.
 */
void
BCMATTACHFN(si_pci_setup)(si_t *sih, uint coremask)
{
	si_info_t *sii = SI_INFO(sih);
	sbpciregs_t *pciregs = NULL;
	uint32 siflag = 0, w;
	uint idx = 0;

	if (BUSTYPE(sii->pub.bustype) != PCI_BUS)
		return;

	ASSERT(PCI(sii) || PCIE(sii));
	ASSERT(sii->pub.buscoreidx != BADIDX);

	if (PCI(sii)) {
		/* get current core index */
		idx = sii->curidx;

		/* we interrupt on this backplane flag number */
		siflag = si_flag(sih);

		/* switch over to pci core */
		pciregs = (sbpciregs_t *)si_setcoreidx(sih, sii->pub.buscoreidx);
	}

	/*
	 * Enable sb->pci interrupts.  Assume
	 * PCI rev 2.3 support was added in pci core rev 6 and things changed..
	 */
	if (PCIE(sii) || (PCI(sii) && ((sii->pub.buscorerev) >= 6))) {
		/* pci config write to set this core bit in PCIIntMask */
		w = OSL_PCI_READ_CONFIG(sii->osh, PCI_INT_MASK, sizeof(uint32));
		w |= ((coremask << PCI_SBIM_SHIFT) & PCI_SBIM_MASK);
		coremask >>= 8; /* keep upper coremask bits only */
		w |= ((coremask << PCI_SBIM_UPR_SHIFT) & PCI_SBIM_UPR_MASK);
		OSL_PCI_WRITE_CONFIG(sii->osh, (PCI_INT_MASK), sizeof(uint32), w);
	} else {
		/* set sbintvec bit for our flag number */
		si_setint(sih, siflag);
	}

	if (PCI(sii)) {
		OR_REG(sii->osh, &pciregs->sbtopci2, (SBTOPCI_PREF | SBTOPCI_BURST));
		if (sii->pub.buscorerev >= 11) {
			OR_REG(sii->osh, &pciregs->sbtopci2, SBTOPCI_RC_READMULTI);
			w = R_REG(sii->osh, &pciregs->clkrun);
			W_REG(sii->osh, &pciregs->clkrun, (w | PCI_CLKRUN_DSBL));
			w = R_REG(sii->osh, &pciregs->clkrun);
		}

		/* switch back to previous core */
		si_setcoreidx(sih, idx);
	}
} /* si_pci_setup */

uint8
si_pcieclkreq(si_t *sih, uint32 mask, uint32 val)
{
	si_info_t *sii = SI_INFO(sih);

	if (!PCIE(sii))
		return 0;

	return pcie_clkreq(sii->pch, mask, val);
}

uint32
si_pcielcreg(si_t *sih, uint32 mask, uint32 val)
{
	si_info_t *sii = SI_INFO(sih);

	if (!PCIE(sii))
		return 0;

	return pcie_lcreg(sii->pch, mask, val);
}

uint8
si_pcieltrenable(si_t *sih, uint32 mask, uint32 val)
{
	si_info_t *sii = SI_INFO(sih);

	if (!(PCIE(sii)))
		return 0;

	return pcie_ltrenable(sii->pch, mask, val);
}

uint8
BCMATTACHFN(si_pcieobffenable)(si_t *sih, uint32 mask, uint32 val)
{
	si_info_t *sii;

	sii = SI_INFO(sih);

	if (!(PCIE(sii)))
		return 0;

	return pcie_obffenable(sii->pch, mask, val);
}

uint32
si_pcieltr_reg(si_t *sih, uint32 reg, uint32 mask, uint32 val)
{
	si_info_t *sii = SI_INFO(sih);

	if (!(PCIE(sii)))
		return 0;

	return pcie_ltr_reg(sii->pch, reg, mask, val);
}

void
si_pcie_set_error_injection(si_t *sih, uint32 mode)
{
	si_info_t *sii = SI_INFO(sih);

	if (!PCIE(sii))
		return;

	pcie_set_error_injection(sii->pch, mode);
}

/** indirect way to read pcie config regs */
uint
si_pcie_readreg(void *sih, uint addrtype, uint offset)
{
	return pcie_readreg(sih, (sbpcieregs_t *)PCIEREGS(((si_info_t *)sih)),
	                    addrtype, offset);
}

/* indirect way to write pcie config regs */
uint
si_pcie_writereg(void *sih, uint addrtype, uint offset, uint val)
{
	return pcie_writereg(sih, (sbpcieregs_t *)PCIEREGS(((si_info_t *)sih)),
	                    addrtype, offset, val);
}

/**
 * PCI(e) core requires additional software initialization in an SROMless system. In such a system,
 * the PCIe core will assume POR defaults, which are mostly ok, with the exception of the mapping of
 * two address subwindows within the BAR0 window.
 * Note: the current core may be changed upon return.
 */
int
si_pci_fixcfg(si_t *sih)
{
	uint origidx, pciidx;
	sbpciregs_t *pciregs = NULL;
	sbpcieregs_t *pcieregs = NULL;
	uint16 val16;
	volatile uint16 *reg16 = NULL;

	si_info_t *sii = SI_INFO(sih);

	ASSERT(BUSTYPE(sii->pub.bustype) == PCI_BUS);

	/* Fixup PI in SROM shadow area to enable the correct PCI core access */
	origidx = si_coreidx(&sii->pub);

	if (BUSCORETYPE(sii->pub.buscoretype) == PCIE2_CORE_ID) {
		pcieregs = (sbpcieregs_t *)si_setcore(&sii->pub, PCIE2_CORE_ID, 0);
		ASSERT(pcieregs != NULL);
		reg16 = &pcieregs->sprom[SRSH_PI_OFFSET];
	} else if (BUSCORETYPE(sii->pub.buscoretype) == PCIE_CORE_ID) {
		pcieregs = (sbpcieregs_t *)si_setcore(&sii->pub, PCIE_CORE_ID, 0);
		ASSERT(pcieregs != NULL);
		reg16 = &pcieregs->sprom[SRSH_PI_OFFSET];
	} else if (BUSCORETYPE(sii->pub.buscoretype) == PCI_CORE_ID) {
		pciregs = (sbpciregs_t *)si_setcore(&sii->pub, PCI_CORE_ID, 0);
		ASSERT(pciregs != NULL);
		reg16 = &pciregs->sprom[SRSH_PI_OFFSET];
	}
	pciidx = si_coreidx(&sii->pub);

	if (!reg16) return -1;

	val16 = R_REG(sii->osh, reg16);
	if (((val16 & SRSH_PI_MASK) >> SRSH_PI_SHIFT) != (uint16)pciidx) {
		/* write bitfield used to translate 3rd and 7th 4K chunk in the Bar0 space. */
		val16 = (uint16)(pciidx << SRSH_PI_SHIFT) | (val16 & ~SRSH_PI_MASK);
		W_REG(sii->osh, reg16, val16);
	}

	/* restore the original index */
	si_setcoreidx(&sii->pub, origidx);

	return 0;
} /* si_pci_fixcfg */

#if defined(BCMDBG) || defined(WLTEST)
int
si_dump_pcieinfo(si_t *sih, struct bcmstrbuf *b)
{
	si_info_t *sii = SI_INFO(sih);

	if (!PCIE_GEN1(sii) && !PCIE_GEN2(sii))
		return BCME_ERROR;

	return pcicore_dump_pcieinfo(sii->pch, b);
}

int
si_dump_pcieregs(si_t *sih, struct bcmstrbuf *b)
{
	si_info_t *sii = SI_INFO(sih);

	if (!PCIE_GEN1(sii) && !PCIE_GEN2(sii))
		return BCME_ERROR;

	return pcicore_dump_pcieregs(sii->pch, b);
}

int
si_gpiodump(si_t *sih, struct bcmstrbuf *b)
{
	si_info_t *sii = SI_INFO(sih);
	uint origidx;
	uint intr_val = 0;
	chipcregs_t *cc;

	INTR_OFF(sii, intr_val);

	origidx = si_coreidx(sih);

	cc = (chipcregs_t *)si_setcore(sih, CC_CORE_ID, 0);
	ASSERT(cc);

	bcm_bprintf(b, "GPIOregs\t");

	bcm_bprintf(b, "gpioin 0x%x ", R_REG(sii->osh, &cc->gpioin));
	bcm_bprintf(b, "gpioout 0x%x ", R_REG(sii->osh, &cc->gpioout));
	bcm_bprintf(b, "gpioouten 0x%x ", R_REG(sii->osh, &cc->gpioouten));
	bcm_bprintf(b, "gpiocontrol 0x%x ", R_REG(sii->osh, &cc->gpiocontrol));
	bcm_bprintf(b, "gpiointpolarity 0x%x ", R_REG(sii->osh, &cc->gpiointpolarity));
	bcm_bprintf(b, "gpiointmask 0x%x ", R_REG(sii->osh, &cc->gpiointmask));
	bcm_bprintf(b, "gpiotimerval 0x%x ", R_REG(sii->osh, &cc->gpiotimerval));
	bcm_bprintf(b, "gpiotimeroutmask 0x%x", R_REG(sii->osh, &cc->gpiotimeroutmask));
	bcm_bprintf(b, "\n");

	/* restore the original index */
	si_setcoreidx(sih, origidx);

	INTR_RESTORE(sii, intr_val);
	return 0;

}
#endif

#if defined(BCMDBG) || defined(BCMDBG_PHYDUMP)
#endif

#endif /* !defined(BCMDONGLEHOST) */

#if defined(BCMDBG) || defined(WLTEST)
void
si_dump_pmuregs(si_t *sih, struct bcmstrbuf *b)
{
#ifndef BCMDONGLEHOST
	uint i;
	uint32 pmu_chip_reg;
#endif
	uint32 pmu_cap;

	bcm_bprintf(b, "===pmu(rev %d)===\n", sih->pmurev);
	pmu_cap = si_ccreg(sih, PMU_CAP, 0, 0);
	bcm_bprintf(b, "pmu_control 0x%x\n", si_ccreg(sih, PMU_CTL, 0, 0));
	bcm_bprintf(b, "pmu_capabilities 0x%x\n", pmu_cap);
	bcm_bprintf(b, "pmu_status 0x%x\n", si_ccreg(sih, PMU_ST, 0, 0));
	bcm_bprintf(b, "res_state 0x%x\n", si_ccreg(sih, PMU_RES_STATE, 0, 0));
	bcm_bprintf(b, "res_pending 0x%x\n", si_ccreg(sih, PMU_RES_PENDING, 0, 0));
	bcm_bprintf(b, "min_res_mask 0x%x\n", si_ccreg(sih, MINRESMASKREG, 0, 0));
	bcm_bprintf(b, "max_res_mask 0x%x\n", si_ccreg(sih, MAXRESMASKREG, 0, 0));
#ifndef BCMDONGLEHOST
	bcm_bprintf(b, "pmu_timer1 %d\n", si_ccreg(sih, PMU_TIMER, 0, 0));

	pmu_chip_reg = (pmu_cap & 0xf8000000);
	pmu_chip_reg = pmu_chip_reg >> 27;
	bcm_bprintf(b, "si_pmu_chipcontrol: ");
	for (i = 0; i < pmu_chip_reg; i++) {
		bcm_bprintf(b, "[%d]=0x%x ", i, si_pmu_chipcontrol(sih, i, 0, 0));
	}

	pmu_chip_reg = (pmu_cap & 0x07c00000);
	pmu_chip_reg = pmu_chip_reg >> 22;
	bcm_bprintf(b, "\nsi_pmu_vregcontrol: ");
	for (i = 0; i < pmu_chip_reg; i++) {
		bcm_bprintf(b, "[%d]=0x%x ", i, si_pmu_vreg_control(sih, i, 0, 0));
	}
	pmu_chip_reg = (pmu_cap & 0x003e0000);
	pmu_chip_reg = pmu_chip_reg >> 17;
	bcm_bprintf(b, "\nsi_pmu_pllcontrol: ");
	for (i = 0; i < pmu_chip_reg; i++) {
		bcm_bprintf(b, "[%d]=0x%x ", i, si_pmu_pllcontrol(sih, i, 0, 0));
	}
	pmu_chip_reg = (pmu_cap & 0x0001e000);
	pmu_chip_reg = pmu_chip_reg >> 13;
	bcm_bprintf(b, "\nsi_pmu_res u/d timer: ");
	for (i = 0; i < pmu_chip_reg; i++) {
		si_corereg(sih, SI_CC_IDX, RSRCTABLEADDR, ~0, i);
		bcm_bprintf(b, "[%d]=0x%x ", i, si_corereg(sih, SI_CC_IDX, RSRCUPDWNTIME, 0, 0));
	}
	pmu_chip_reg = (pmu_cap & 0x00001f00);
	pmu_chip_reg = pmu_chip_reg >> 8;
	bcm_bprintf(b, "\nsi_pmu_res dep_mask: ");
	for (i = 0; i < pmu_chip_reg; i++) {
		si_corereg(sih, SI_CC_IDX, RSRCTABLEADDR, ~0, i);
		bcm_bprintf(b, "[%d]=0x%x ", i, si_corereg(sih, SI_CC_IDX, PMU_RES_DEP_MASK, 0, 0));
	}
	bcm_bprintf(b, "\n");
#endif /* !BCMDONGLEHOST */
}
#endif

#if defined(BCMDBG) || defined(BCMDBG_PHYDUMP)
#endif

/** change logical "focus" to the gpio core for optimized access */
volatile void *
si_gpiosetcore(si_t *sih)
{
	return (si_setcoreidx(sih, SI_CC_IDX));
}

/**
 * mask & set gpiocontrol bits.
 * If a gpiocontrol bit is set to 0, chipcommon controls the corresponding GPIO pin.
 * If a gpiocontrol bit is set to 1, the GPIO pin is no longer a GPIO and becomes dedicated
 *   to some chip-specific purpose.
 */
uint32
si_gpiocontrol(si_t *sih, uint32 mask, uint32 val, uint8 priority)
{
	uint regoff;

	regoff = 0;

	/* gpios could be shared on router platforms
	 * ignore reservation if it's high priority (e.g., test apps)
	 */
	if ((priority != GPIO_HI_PRIORITY) &&
	    (BUSTYPE(sih->bustype) == SI_BUS) && (val || mask)) {
		mask = priority ? (si_gpioreservation & mask) :
			((si_gpioreservation | mask) & ~(si_gpioreservation));
		val &= mask;
	}

	regoff = OFFSETOF(chipcregs_t, gpiocontrol);
	return (si_corereg(sih, SI_CC_IDX, regoff, mask, val));
}

/** mask&set gpio output enable bits */
uint32
si_gpioouten(si_t *sih, uint32 mask, uint32 val, uint8 priority)
{
	uint regoff;

	regoff = 0;

	/* gpios could be shared on router platforms
	 * ignore reservation if it's high priority (e.g., test apps)
	 */
	if ((priority != GPIO_HI_PRIORITY) &&
	    (BUSTYPE(sih->bustype) == SI_BUS) && (val || mask)) {
		mask = priority ? (si_gpioreservation & mask) :
			((si_gpioreservation | mask) & ~(si_gpioreservation));
		val &= mask;
	}

	regoff = OFFSETOF(chipcregs_t, gpioouten);
	return (si_corereg(sih, SI_CC_IDX, regoff, mask, val));
}

/** mask&set gpio output bits */
uint32
si_gpioout(si_t *sih, uint32 mask, uint32 val, uint8 priority)
{
	uint regoff;

	regoff = 0;

	/* gpios could be shared on router platforms
	 * ignore reservation if it's high priority (e.g., test apps)
	 */
	if ((priority != GPIO_HI_PRIORITY) &&
	    (BUSTYPE(sih->bustype) == SI_BUS) && (val || mask)) {
		mask = priority ? (si_gpioreservation & mask) :
			((si_gpioreservation | mask) & ~(si_gpioreservation));
		val &= mask;
	}

	regoff = OFFSETOF(chipcregs_t, gpioout);
	return (si_corereg(sih, SI_CC_IDX, regoff, mask, val));
}

/** reserve one gpio */
uint32
si_gpioreserve(si_t *sih, uint32 gpio_bitmask, uint8 priority)
{
	/* only cores on SI_BUS share GPIO's and only applcation users need to
	 * reserve/release GPIO
	 */
	if ((BUSTYPE(sih->bustype) != SI_BUS) || (!priority)) {
		ASSERT((BUSTYPE(sih->bustype) == SI_BUS) && (priority));
		return 0xffffffff;
	}
	/* make sure only one bit is set */
	if ((!gpio_bitmask) || ((gpio_bitmask) & (gpio_bitmask - 1))) {
		ASSERT((gpio_bitmask) && !((gpio_bitmask) & (gpio_bitmask - 1)));
		return 0xffffffff;
	}

	/* already reserved */
	if (si_gpioreservation & gpio_bitmask)
		return 0xffffffff;
	/* set reservation */
	si_gpioreservation |= gpio_bitmask;

	return si_gpioreservation;
}

/**
 * release one gpio.
 *
 * releasing the gpio doesn't change the current value on the GPIO last write value
 * persists till someone overwrites it.
 */
uint32
si_gpiorelease(si_t *sih, uint32 gpio_bitmask, uint8 priority)
{
	/* only cores on SI_BUS share GPIO's and only applcation users need to
	 * reserve/release GPIO
	 */
	if ((BUSTYPE(sih->bustype) != SI_BUS) || (!priority)) {
		ASSERT((BUSTYPE(sih->bustype) == SI_BUS) && (priority));
		return 0xffffffff;
	}
	/* make sure only one bit is set */
	if ((!gpio_bitmask) || ((gpio_bitmask) & (gpio_bitmask - 1))) {
		ASSERT((gpio_bitmask) && !((gpio_bitmask) & (gpio_bitmask - 1)));
		return 0xffffffff;
	}

	/* already released */
	if (!(si_gpioreservation & gpio_bitmask))
		return 0xffffffff;

	/* clear reservation */
	si_gpioreservation &= ~gpio_bitmask;

	return si_gpioreservation;
}

/* return the current gpioin register value */
uint32
si_gpioin(si_t *sih)
{
	uint regoff;

	regoff = OFFSETOF(chipcregs_t, gpioin);
	return (si_corereg(sih, SI_CC_IDX, regoff, 0, 0));
}

/* mask&set gpio interrupt polarity bits */
uint32
si_gpiointpolarity(si_t *sih, uint32 mask, uint32 val, uint8 priority)
{
	uint regoff;

	/* gpios could be shared on router platforms */
	if ((BUSTYPE(sih->bustype) == SI_BUS) && (val || mask)) {
		mask = priority ? (si_gpioreservation & mask) :
			((si_gpioreservation | mask) & ~(si_gpioreservation));
		val &= mask;
	}

	regoff = OFFSETOF(chipcregs_t, gpiointpolarity);
	return (si_corereg(sih, SI_CC_IDX, regoff, mask, val));
}

/* mask&set gpio interrupt mask bits */
uint32
si_gpiointmask(si_t *sih, uint32 mask, uint32 val, uint8 priority)
{
	uint regoff;

	/* gpios could be shared on router platforms */
	if ((BUSTYPE(sih->bustype) == SI_BUS) && (val || mask)) {
		mask = priority ? (si_gpioreservation & mask) :
			((si_gpioreservation | mask) & ~(si_gpioreservation));
		val &= mask;
	}

	regoff = OFFSETOF(chipcregs_t, gpiointmask);
	return (si_corereg(sih, SI_CC_IDX, regoff, mask, val));
}

uint32
si_gpioeventintmask(si_t *sih, uint32 mask, uint32 val, uint8 priority)
{
	uint regoff;
	/* gpios could be shared on router platforms */
	if ((BUSTYPE(sih->bustype) == SI_BUS) && (val || mask)) {
		mask = priority ? (si_gpioreservation & mask) :
			((si_gpioreservation | mask) & ~(si_gpioreservation));
		val &= mask;
	}
	regoff = OFFSETOF(chipcregs_t, gpioeventintmask);
	return (si_corereg(sih, SI_CC_IDX, regoff, mask, val));
}

/* assign the gpio to an led */
uint32
si_gpioled(si_t *sih, uint32 mask, uint32 val)
{
	/* gpio led powersave reg */
	return (si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, gpiotimeroutmask), mask, val));
}

/* mask&set gpio timer val */
uint32
si_gpiotimerval(si_t *sih, uint32 mask, uint32 gpiotimerval)
{
	return (si_corereg(sih, SI_CC_IDX,
		OFFSETOF(chipcregs_t, gpiotimerval), mask, gpiotimerval));
}

uint32
si_gpiopull(si_t *sih, bool updown, uint32 mask, uint32 val)
{
	uint offs;

	offs = (updown ? OFFSETOF(chipcregs_t, gpiopulldown) : OFFSETOF(chipcregs_t, gpiopullup));
	return (si_corereg(sih, SI_CC_IDX, offs, mask, val));
}

uint32
si_gpioevent(si_t *sih, uint regtype, uint32 mask, uint32 val)
{
	uint offs;

	if (regtype == GPIO_REGEVT)
		offs = OFFSETOF(chipcregs_t, gpioevent);
	else if (regtype == GPIO_REGEVT_INTMSK)
		offs = OFFSETOF(chipcregs_t, gpioeventintmask);
	else if (regtype == GPIO_REGEVT_INTPOL)
		offs = OFFSETOF(chipcregs_t, gpioeventintpolarity);
	else
		return 0xffffffff;

	return (si_corereg(sih, SI_CC_IDX, offs, mask, val));
}

uint32
BCMATTACHFN(si_gpio_int_enable)(si_t *sih, bool enable)
{
	uint offs;

	offs = OFFSETOF(chipcregs_t, intmask);
	return (si_corereg(sih, SI_CC_IDX, offs, CI_GPIO, (enable ? CI_GPIO : 0)));
}

#if !defined(BCMDONGLEHOST)
#if (!defined(_CFE_) && !defined(_CFEZ_)) || defined(CFG_WL)
void
si_gci_shif_config_wake_pin(si_t *sih, uint8 gpio_n, uint8 wake_events,
		bool gci_gpio)
{
	uint8 chipcontrol = 0;
	uint32 gci_wakset;

	switch (CHIPID(sih->chip)) {
		case BCM4347_CHIP_GRPID :
			{
				if (!gci_gpio) {
					chipcontrol = (1 << GCI_GPIO_CHIPCTRL_ENAB_EXT_GPIO_BIT);
				}
				chipcontrol |= (1 << GCI_GPIO_CHIPCTRL_PULLUP_BIT);
				chipcontrol |= (1 << GCI_GPIO_CHIPCTRL_INVERT_BIT);
				si_gci_gpio_chipcontrol(sih, gpio_n,
					(chipcontrol | (1 << GCI_GPIO_CHIPCTRL_ENAB_IN_BIT)));

				/* enable gci gpio int/wake events */
				si_gci_gpio_intmask(sih, gpio_n, wake_events, wake_events);
				si_gci_gpio_wakemask(sih, gpio_n, wake_events, wake_events);

				/* clear the existing status bits */
				si_gci_gpio_status(sih, gpio_n,
						GCI_GPIO_STS_CLEAR, GCI_GPIO_STS_CLEAR);

				/* Enable gci2wl_wake for 4347 */
				si_pmu_chipcontrol(sih, PMU_CHIPCTL2,
						CC2_4347_GCI2WAKE_MASK, CC2_4347_GCI2WAKE_MASK);

				/* enable gci int/wake events */
				gci_wakset = (GCI_INTSTATUS_GPIOWAKE) | (GCI_INTSTATUS_GPIOINT);

				si_gci_indirect(sih, 0,	GCI_OFFSETOF(sih, gci_intmask),
						gci_wakset, gci_wakset);
				/* Enable wake on GciWake */
				si_gci_indirect(sih, 0,	GCI_OFFSETOF(sih, gci_wakemask),
						gci_wakset, gci_wakset);
				break;
			}
		default:;
	}
}

void
si_shif_int_enable(si_t *sih, uint8 gpio_n, uint8 wake_events, bool enable)
{
	if (enable) {
		si_gci_gpio_intmask(sih, gpio_n, wake_events, wake_events);
		si_gci_gpio_wakemask(sih, gpio_n, wake_events, wake_events);
	} else {
		si_gci_gpio_intmask(sih, gpio_n, wake_events, 0);
		si_gci_gpio_wakemask(sih, gpio_n, wake_events, 0);
	}
}
#endif /* (!_CFE_ && !_CFEZ_) || CFG_WL */
#endif /* !defined(BCMDONGLEHOST) */

/** Return the size of the specified SYSMEM bank */
static uint
sysmem_banksize(si_info_t *sii, sysmemregs_t *regs, uint8 idx)
{
	uint banksize, bankinfo;
	uint bankidx = idx;

	W_REG(sii->osh, &regs->bankidx, bankidx);
	bankinfo = R_REG(sii->osh, &regs->bankinfo);
	banksize = SYSMEM_BANKINFO_SZBASE * ((bankinfo & SYSMEM_BANKINFO_SZMASK) + 1);
	return banksize;
}

/** Return the RAM size of the SYSMEM core */
uint32
si_sysmem_size(si_t *sih)
{
	si_info_t *sii = SI_INFO(sih);
	uint origidx;
	uint intr_val = 0;

	sysmemregs_t *regs;
	bool wasup;
	uint32 coreinfo;
	uint memsize = 0;
	uint8 i;
	uint nb, nrb;
	uint32 bar0_win = 0;

	/* Block ints and save current core */
	INTR_OFF(sii, intr_val);
	origidx = si_coreidx(sih);

	/* Switch to SYSMEM core */
	if (!(regs = si_setcore(sih, SYSMEM_CORE_ID, 0)))
		goto done;

	/* Get info for determining size */
	if (!(wasup = si_iscoreup(sih)))
		si_core_reset(sih, 0, 0);
	coreinfo = R_REG(sii->osh, &regs->coreinfo);

	/* Number of ROM banks, SW need to skip the ROM banks. */
	nrb = (coreinfo & SYSMEM_SRCI_ROMNB_MASK) >> SYSMEM_SRCI_ROMNB_SHIFT;

	nb = (coreinfo & SYSMEM_SRCI_SRNB_MASK) >> SYSMEM_SRCI_SRNB_SHIFT;
	for (i = 0; i < nb; i++)
		memsize += sysmem_banksize(sii, regs, i + nrb);

	if ((si_addrspacesize(sih, CORE_SLAVE_PORT_0, CORE_BASE_ADDR_0) > SI_CORE_SIZE)) {
		if (BUSTYPE(sih->bustype) == PCI_BUS) {
			bar0_win = OSL_PCI_READ_CONFIG(sii->osh, PCI_BAR0_WIN, sizeof(uint32));
			OSL_PCI_WRITE_CONFIG(sii->osh, PCI_BAR0_WIN, 4, bar0_win + SI_CORE_SIZE);
#if defined(DONGLEBUILD)
		} else {
			ASSERT(BUSTYPE(sih->bustype) == SI_BUS);
			regs = (sysmemregs_t *)((uint32)regs + SI_CORE_SIZE);
#endif /* DONGLEBUILD */
		}

		coreinfo = R_REG(sii->osh, &regs->coreinfo);
		nrb = (coreinfo & SYSMEM_SRCI_ROMNB_MASK) >> SYSMEM_SRCI_ROMNB_SHIFT;
		nb = (coreinfo & SYSMEM_SRCI_SRNB_MASK) >> SYSMEM_SRCI_SRNB_SHIFT;
		for (i = 0; i < nb; i++)
			memsize += sysmem_banksize(sii, regs, i + nrb);
		if (BUSTYPE(sih->bustype) == PCI_BUS) {
			/* Restore PCI_BAR0_WIN */
			OSL_PCI_WRITE_CONFIG(sii->osh, PCI_BAR0_WIN, 4, bar0_win);
		}
	}

	si_setcoreidx(sih, origidx);

done:
	INTR_RESTORE(sii, intr_val);

	return memsize;
}

/** Return the size of the specified SOCRAM bank */
static uint
socram_banksize(si_info_t *sii, sbsocramregs_t *regs, uint8 idx, uint8 mem_type)
{
	uint banksize, bankinfo;
	uint bankidx = idx | (mem_type << SOCRAM_BANKIDX_MEMTYPE_SHIFT);

	ASSERT(mem_type <= SOCRAM_MEMTYPE_DEVRAM);

	W_REG(sii->osh, &regs->bankidx, (uint32)bankidx);
	bankinfo = R_REG(sii->osh, &regs->bankinfo);
	banksize = SOCRAM_BANKINFO_SZBASE * ((bankinfo & SOCRAM_BANKINFO_SZMASK) + 1);
	return banksize;
}

void si_socram_set_bankpda(si_t *sih, uint32 bankidx, uint32 bankpda)
{
	si_info_t *sii = SI_INFO(sih);
	uint origidx;
	uint intr_val = 0;
	sbsocramregs_t *regs;
	bool wasup;
	uint corerev;

	/* Block ints and save current core */
	INTR_OFF(sii, intr_val);
	origidx = si_coreidx(sih);

	/* Switch to SOCRAM core */
	if (!(regs = si_setcore(sih, SOCRAM_CORE_ID, 0)))
		goto done;

	if (!(wasup = si_iscoreup(sih)))
		si_core_reset(sih, 0, 0);

	corerev = si_corerev(sih);
	if (corerev >= 16) {
		W_REG(sii->osh, &regs->bankidx, bankidx);
		W_REG(sii->osh, &regs->bankpda, bankpda);
	}

	/* Return to previous state and core */
	if (!wasup)
		si_core_disable(sih, 0);
	si_setcoreidx(sih, origidx);

done:
	INTR_RESTORE(sii, intr_val);
}

void
si_socdevram(si_t *sih, bool set, uint8 *enable, uint8 *protect, uint8 *remap)
{
	si_info_t *sii = SI_INFO(sih);
	uint origidx;
	uint intr_val = 0;
	sbsocramregs_t *regs;
	bool wasup;
	uint corerev;

	/* Block ints and save current core */
	INTR_OFF(sii, intr_val);
	origidx = si_coreidx(sih);

	if (!set)
		*enable = *protect = *remap = 0;

	/* Switch to SOCRAM core */
	if (!(regs = si_setcore(sih, SOCRAM_CORE_ID, 0)))
		goto done;

	/* Get info for determining size */
	if (!(wasup = si_iscoreup(sih)))
		si_core_reset(sih, 0, 0);

	corerev = si_corerev(sih);
	if (corerev >= 10) {
		uint32 extcinfo;
		uint8 nb;
		uint8 i;
		uint32 bankidx, bankinfo;

		extcinfo = R_REG(sii->osh, &regs->extracoreinfo);
		nb = ((extcinfo & SOCRAM_DEVRAMBANK_MASK) >> SOCRAM_DEVRAMBANK_SHIFT);
		for (i = 0; i < nb; i++) {
			bankidx = i | (SOCRAM_MEMTYPE_DEVRAM << SOCRAM_BANKIDX_MEMTYPE_SHIFT);
			W_REG(sii->osh, &regs->bankidx, bankidx);
			bankinfo = R_REG(sii->osh, &regs->bankinfo);
			if (set) {
				bankinfo &= ~SOCRAM_BANKINFO_DEVRAMSEL_MASK;
				bankinfo &= ~SOCRAM_BANKINFO_DEVRAMPRO_MASK;
				bankinfo &= ~SOCRAM_BANKINFO_DEVRAMREMAP_MASK;
				if (*enable) {
					bankinfo |= (1 << SOCRAM_BANKINFO_DEVRAMSEL_SHIFT);
					if (*protect)
						bankinfo |= (1 << SOCRAM_BANKINFO_DEVRAMPRO_SHIFT);
					if ((corerev >= 16) && *remap)
						bankinfo |=
							(1 << SOCRAM_BANKINFO_DEVRAMREMAP_SHIFT);
				}
				W_REG(sii->osh, &regs->bankinfo, bankinfo);
			} else if (i == 0) {
				if (bankinfo & SOCRAM_BANKINFO_DEVRAMSEL_MASK) {
					*enable = 1;
					if (bankinfo & SOCRAM_BANKINFO_DEVRAMPRO_MASK)
						*protect = 1;
					if (bankinfo & SOCRAM_BANKINFO_DEVRAMREMAP_MASK)
						*remap = 1;
				}
			}
		}
	}

	/* Return to previous state and core */
	if (!wasup)
		si_core_disable(sih, 0);
	si_setcoreidx(sih, origidx);

done:
	INTR_RESTORE(sii, intr_val);
} /* si_socdevram */

bool
si_socdevram_remap_isenb(si_t *sih)
{
	si_info_t *sii = SI_INFO(sih);
	uint origidx;
	uint intr_val = 0;
	sbsocramregs_t *regs;
	bool wasup, remap = FALSE;
	uint corerev;
	uint32 extcinfo;
	uint8 nb;
	uint8 i;
	uint32 bankidx, bankinfo;

	/* Block ints and save current core */
	INTR_OFF(sii, intr_val);
	origidx = si_coreidx(sih);

	/* Switch to SOCRAM core */
	if (!(regs = si_setcore(sih, SOCRAM_CORE_ID, 0)))
		goto done;

	/* Get info for determining size */
	if (!(wasup = si_iscoreup(sih)))
		si_core_reset(sih, 0, 0);

	corerev = si_corerev(sih);
	if (corerev >= 16) {
		extcinfo = R_REG(sii->osh, &regs->extracoreinfo);
		nb = ((extcinfo & SOCRAM_DEVRAMBANK_MASK) >> SOCRAM_DEVRAMBANK_SHIFT);
		for (i = 0; i < nb; i++) {
			bankidx = i | (SOCRAM_MEMTYPE_DEVRAM << SOCRAM_BANKIDX_MEMTYPE_SHIFT);
			W_REG(sii->osh, &regs->bankidx, bankidx);
			bankinfo = R_REG(sii->osh, &regs->bankinfo);
			if (bankinfo & SOCRAM_BANKINFO_DEVRAMREMAP_MASK) {
				remap = TRUE;
				break;
			}
		}
	}

	/* Return to previous state and core */
	if (!wasup)
		si_core_disable(sih, 0);
	si_setcoreidx(sih, origidx);

done:
	INTR_RESTORE(sii, intr_val);
	return remap;
}

bool
si_socdevram_pkg(si_t *sih)
{
	if (si_socdevram_size(sih) > 0)
		return TRUE;
	else
		return FALSE;
}

uint32
si_socdevram_size(si_t *sih)
{
	si_info_t *sii = SI_INFO(sih);
	uint origidx;
	uint intr_val = 0;
	uint32 memsize = 0;
	sbsocramregs_t *regs;
	bool wasup;
	uint corerev;

	/* Block ints and save current core */
	INTR_OFF(sii, intr_val);
	origidx = si_coreidx(sih);

	/* Switch to SOCRAM core */
	if (!(regs = si_setcore(sih, SOCRAM_CORE_ID, 0)))
		goto done;

	/* Get info for determining size */
	if (!(wasup = si_iscoreup(sih)))
		si_core_reset(sih, 0, 0);

	corerev = si_corerev(sih);
	if (corerev >= 10) {
		uint32 extcinfo;
		uint8 nb;
		uint8 i;

		extcinfo = R_REG(sii->osh, &regs->extracoreinfo);
		nb = (((extcinfo & SOCRAM_DEVRAMBANK_MASK) >> SOCRAM_DEVRAMBANK_SHIFT));
		for (i = 0; i < nb; i++)
			memsize += socram_banksize(sii, regs, i, SOCRAM_MEMTYPE_DEVRAM);
	}

	/* Return to previous state and core */
	if (!wasup)
		si_core_disable(sih, 0);
	si_setcoreidx(sih, origidx);

done:
	INTR_RESTORE(sii, intr_val);

	return memsize;
} /* si_socdevram_size */

uint32
si_socdevram_remap_size(si_t *sih)
{
	si_info_t *sii = SI_INFO(sih);
	uint origidx;
	uint intr_val = 0;
	uint32 memsize = 0, banksz;
	sbsocramregs_t *regs;
	bool wasup;
	uint corerev;
	uint32 extcinfo;
	uint8 nb;
	uint8 i;
	uint32 bankidx, bankinfo;

	/* Block ints and save current core */
	INTR_OFF(sii, intr_val);
	origidx = si_coreidx(sih);

	/* Switch to SOCRAM core */
	if (!(regs = si_setcore(sih, SOCRAM_CORE_ID, 0)))
		goto done;

	/* Get info for determining size */
	if (!(wasup = si_iscoreup(sih)))
		si_core_reset(sih, 0, 0);

	corerev = si_corerev(sih);
	if (corerev >= 16) {
		extcinfo = R_REG(sii->osh, &regs->extracoreinfo);
		nb = (((extcinfo & SOCRAM_DEVRAMBANK_MASK) >> SOCRAM_DEVRAMBANK_SHIFT));

		/*
		 * FIX: A0 Issue: Max addressable is 512KB, instead 640KB
		 * Only four banks are accessible to ARM
		 */
		if ((corerev == 16) && (nb == 5))
			nb = 4;

		for (i = 0; i < nb; i++) {
			bankidx = i | (SOCRAM_MEMTYPE_DEVRAM << SOCRAM_BANKIDX_MEMTYPE_SHIFT);
			W_REG(sii->osh, &regs->bankidx, bankidx);
			bankinfo = R_REG(sii->osh, &regs->bankinfo);
			if (bankinfo & SOCRAM_BANKINFO_DEVRAMREMAP_MASK) {
				banksz = socram_banksize(sii, regs, i, SOCRAM_MEMTYPE_DEVRAM);
				memsize += banksz;
			} else {
				/* Account only consecutive banks for now */
				break;
			}
		}
	}

	/* Return to previous state and core */
	if (!wasup)
		si_core_disable(sih, 0);
	si_setcoreidx(sih, origidx);

done:
	INTR_RESTORE(sii, intr_val);

	return memsize;
} /* si_socdevram_remap_size */

/** Return the RAM size of the SOCRAM core */
uint32
si_socram_size(si_t *sih)
{
	si_info_t *sii = SI_INFO(sih);
	uint origidx;
	uint intr_val = 0;

	sbsocramregs_t *regs;
	bool wasup;
	uint corerev;
	uint32 coreinfo;
	uint memsize = 0;

	/* Block ints and save current core */
	INTR_OFF(sii, intr_val);
	origidx = si_coreidx(sih);

	/* Switch to SOCRAM core */
	if (!(regs = si_setcore(sih, SOCRAM_CORE_ID, 0)))
		goto done;

	/* Get info for determining size */
	if (!(wasup = si_iscoreup(sih)))
		si_core_reset(sih, 0, 0);
	corerev = si_corerev(sih);
	coreinfo = R_REG(sii->osh, &regs->coreinfo);

	/* Calculate size from coreinfo based on rev */
	if (corerev == 0)
		memsize = 1 << (16 + (coreinfo & SRCI_MS0_MASK));
	else if (corerev < 3) {
		memsize = 1 << (SR_BSZ_BASE + (coreinfo & SRCI_SRBSZ_MASK));
		memsize *= (coreinfo & SRCI_SRNB_MASK) >> SRCI_SRNB_SHIFT;
	} else if ((corerev <= 7) || (corerev == 12)) {
		uint nb = (coreinfo & SRCI_SRNB_MASK) >> SRCI_SRNB_SHIFT;
		uint bsz = (coreinfo & SRCI_SRBSZ_MASK);
		uint lss = (coreinfo & SRCI_LSS_MASK) >> SRCI_LSS_SHIFT;
		if (lss != 0)
			nb --;
		memsize = nb * (1 << (bsz + SR_BSZ_BASE));
		if (lss != 0)
			memsize += (1 << ((lss - 1) + SR_BSZ_BASE));
	} else {
		uint8 i;
		uint nb;
		/* length of SRAM Banks increased for corerev greater than 23 */
		if (corerev >= 23) {
			nb = (coreinfo & (SRCI_SRNB_MASK | SRCI_SRNB_MASK_EXT)) >> SRCI_SRNB_SHIFT;
		} else {
			nb = (coreinfo & SRCI_SRNB_MASK) >> SRCI_SRNB_SHIFT;
		}
		for (i = 0; i < nb; i++)
			memsize += socram_banksize(sii, regs, i, SOCRAM_MEMTYPE_RAM);
	}

	/* Return to previous state and core */
	if (!wasup)
		si_core_disable(sih, 0);
	si_setcoreidx(sih, origidx);

done:
	INTR_RESTORE(sii, intr_val);

	return memsize;
} /* si_socram_size */

#if defined(BCMDONGLEHOST)

/** Return the TCM-RAM size of the ARMCR4 core. */
uint32
si_tcm_size(si_t *sih)
{
	si_info_t *sii = SI_INFO(sih);
	uint origidx;
	uint intr_val = 0;
	volatile uint8 *regs;
	bool wasup;
	uint32 corecap;
	uint memsize = 0;
	uint32 nab = 0;
	uint32 nbb = 0;
	uint32 totb = 0;
	uint32 bxinfo = 0;
	uint32 idx = 0;
	volatile uint32 *arm_cap_reg;
	volatile uint32 *arm_bidx;
	volatile uint32 *arm_binfo;

	/* Block ints and save current core */
	INTR_OFF(sii, intr_val);
	origidx = si_coreidx(sih);

	/* Switch to CR4 core */
	if (!(regs = si_setcore(sih, ARMCR4_CORE_ID, 0)))
		goto done;

	/* Get info for determining size. If in reset, come out of reset,
	 * but remain in halt
	 */
	if (!(wasup = si_iscoreup(sih)))
		si_core_reset(sih, SICF_CPUHALT, SICF_CPUHALT);

	arm_cap_reg = (volatile uint32 *)(regs + SI_CR4_CAP);
	corecap = R_REG(sii->osh, arm_cap_reg);

	nab = (corecap & ARMCR4_TCBANB_MASK) >> ARMCR4_TCBANB_SHIFT;
	nbb = (corecap & ARMCR4_TCBBNB_MASK) >> ARMCR4_TCBBNB_SHIFT;
	totb = nab + nbb;

	arm_bidx = (volatile uint32 *)(regs + SI_CR4_BANKIDX);
	arm_binfo = (volatile uint32 *)(regs + SI_CR4_BANKINFO);
	for (idx = 0; idx < totb; idx++) {
		W_REG(sii->osh, arm_bidx, idx);

		bxinfo = R_REG(sii->osh, arm_binfo);
		memsize += ((bxinfo & ARMCR4_BSZ_MASK) + 1) * ARMCR4_BSZ_MULT;
	}

	/* Return to previous state and core */
	if (!wasup)
		si_core_disable(sih, 0);
	si_setcoreidx(sih, origidx);

done:
	INTR_RESTORE(sii, intr_val);

	return memsize;
} /* si_tcm_size */

bool
si_has_flops(si_t *sih)
{
	uint origidx, cr4_rev;

	/* Find out CR4 core revision */
	origidx = si_coreidx(sih);
	if (si_setcore(sih, ARMCR4_CORE_ID, 0)) {
		cr4_rev = si_corerev(sih);
		si_setcoreidx(sih, origidx);

		if (cr4_rev == 1 || cr4_rev >= 3)
			return TRUE;
	}
	return FALSE;
}
#endif /* BCMDONGLEHOST */

uint32
si_socram_srmem_size(si_t *sih)
{
	si_info_t *sii = SI_INFO(sih);
	uint origidx;
	uint intr_val = 0;

	sbsocramregs_t *regs;
	bool wasup;
	uint corerev;
	uint32 coreinfo;
	uint memsize = 0;

	/* Block ints and save current core */
	INTR_OFF(sii, intr_val);
	origidx = si_coreidx(sih);

	/* Switch to SOCRAM core */
	if (!(regs = si_setcore(sih, SOCRAM_CORE_ID, 0)))
		goto done;

	/* Get info for determining size */
	if (!(wasup = si_iscoreup(sih)))
		si_core_reset(sih, 0, 0);
	corerev = si_corerev(sih);
	coreinfo = R_REG(sii->osh, &regs->coreinfo);

	/* Calculate size from coreinfo based on rev */
	if (corerev >= 16) {
		uint32 i;
		uint32 nb = (coreinfo & SRCI_SRNB_MASK) >> SRCI_SRNB_SHIFT;
		for (i = 0; i < nb; i++) {
			W_REG(sii->osh, &regs->bankidx, i);
			if (R_REG(sii->osh, &regs->bankinfo) & SOCRAM_BANKINFO_RETNTRAM_MASK)
				memsize += socram_banksize(sii, regs, i, SOCRAM_MEMTYPE_RAM);
		}
	}

	/* Return to previous state and core */
	if (!wasup)
		si_core_disable(sih, 0);
	si_setcoreidx(sih, origidx);

done:
	INTR_RESTORE(sii, intr_val);

	return memsize;
} /* si_socram_srmem_size */

#if !defined(BCMDONGLEHOST)
static bool
si_seci_uart(si_t *sih)
{
	return (sih->cccaps_ext & CC_CAP_EXT_SECI_PUART_PRESENT);
}

/** seci clock enable/disable */
void
si_seci_clkreq(si_t *sih, bool enable)
{
	uint32 clk_ctl_st;
	uint32 offset;
	uint32 val;
	pmuregs_t *pmu;
	uint32 origidx = 0;
	si_info_t *sii = SI_INFO(sih);
#ifdef SECI_UART
	bool fast;
	chipcregs_t *cc = seci_set_core(sih, &origidx, &fast);
	ASSERT(cc);
#endif /* SECI_UART */
	if (!si_seci(sih) && !si_seci_uart(sih))
		return;
	offset = OFFSETOF(chipcregs_t, clk_ctl_st);
	clk_ctl_st = si_corereg(sih, 0, offset, 0, 0);

	if (enable && !(clk_ctl_st & CLKCTL_STS_SECI_CLK_REQ)) {
		val = CLKCTL_STS_SECI_CLK_REQ | CLKCTL_STS_HT_AVAIL_REQ;
#ifdef SECI_UART
		/* Restore the fast UART function select when enabling */
		if (fast_uart_init) {
			si_gci_set_functionsel(sih, fast_uart_tx, fast_uart_functionsel);
			if (fuart_pullup_rx_cts_enab) {
				si_gci_set_functionsel(sih, fast_uart_rx, fast_uart_functionsel);
				si_gci_set_functionsel(sih, fast_uart_cts_in,
					fast_uart_functionsel);
			}
		}
#endif /* SECI_UART */
	} else if (!enable && (clk_ctl_st & CLKCTL_STS_SECI_CLK_REQ)) {
		val = 0;
#ifdef SECI_UART
		if (force_seci_clk) {
			return;
		}
#endif /* SECI_UART */
	} else {
		return;
	}
#ifdef SECI_UART
	/* park the fast UART as PULL UP when disabling the clocks to avoid sending
	 * breaks to the host
	 */
	if (!enable && fast_uart_init) {
		si_gci_set_functionsel(sih, fast_uart_tx, fast_uart_pup);
		if (fuart_pullup_rx_cts_enab) {
			W_REG(sii->osh, &cc->SECI_status, SECI_STAT_BI);
			si_gci_set_functionsel(sih, fast_uart_rx, fast_uart_pup);
			si_gci_set_functionsel(sih, fast_uart_cts_in, fast_uart_pup);
			SPINWAIT(!(R_REG(sii->osh, &cc->SECI_status) & SECI_STAT_BI), 1000);
		}
	}
#endif /* SECI_UART */

	/* Setting/clearing bit 4 along with bit 8 of 0x1e0 block. the core requests that
	  * the PMU set the device state such that the HT clock will be available on short notice.
	  */
	si_corereg(sih, SI_CC_IDX, offset,
		CLKCTL_STS_SECI_CLK_REQ | CLKCTL_STS_HT_AVAIL_REQ, val);

	if (!enable)
		return;

#ifndef SECI_UART
	/* Remember original core before switch to chipc/pmu */
	origidx = si_coreidx(sih);
#endif

	pmu = si_setcore(sih, PMU_CORE_ID, 0);
	ASSERT(pmu != NULL);
	if (si_pmu_wait_for_steady_state(sih, sii->osh, pmu)) {
#if defined(DONGLEBUILD)
		OSL_SYS_HALT();
#endif /* DONGLEBUILD */
	}
	/* Return to original core */
	si_setcoreidx(sih, origidx);

	SPINWAIT(!(si_corereg(sih, 0, offset, 0, 0) & CLKCTL_STS_SECI_CLK_AVAIL),
	        PMU_MAX_TRANSITION_DLY);

	clk_ctl_st = si_corereg(sih, 0, offset, 0, 0);
	if (enable) {
		if (!(clk_ctl_st & CLKCTL_STS_SECI_CLK_AVAIL)) {
			SI_ERROR(("SECI clock is still not available: 0x%x\n", clk_ctl_st));
			return;
		}
	}
} /* si_seci_clkreq */

#if defined(BCMECICOEX) || defined(SECI_UART)
static chipcregs_t *
seci_set_core(si_t *sih, uint32 *origidx, bool *fast)
{
	chipcregs_t *cc;
	si_info_t *sii = SI_INFO(sih);
	*fast = SI_FAST(sii);

	if (!*fast) {
		*origidx = sii->curidx;
		cc = (chipcregs_t *)si_setcore(sih, CC_CORE_ID, 0);
	} else {
		*origidx = 0;
		cc = (chipcregs_t *)CCREGS_FAST(sii);
	}
	return cc;
}

static chipcregs_t *
si_seci_access_preamble(si_t *sih, si_info_t *sii, uint32 *origidx, bool *fast)
{
	chipcregs_t *cc;
	cc = seci_set_core(sih, origidx, fast);

	if (cc) {
		if (((R_REG(sii->osh, &cc->clk_ctl_st) & CCS_SECICLKREQ) != CCS_SECICLKREQ)) {
			/* enable SECI clock */
			si_seci_clkreq(sih, TRUE);
		}
	}
	return cc;
}
#endif /* BCMECICOEX||SECI_UART */

#ifdef SECI_UART

uint32
si_seci_access(si_t *sih, uint32 val, int access)
{
	uint32 origidx;
	bool fast;
	si_info_t *sii = SI_INFO(sih);
	chipcregs_t *cc;
	uint intr_val = 0;
	uint32 offset, retval = 1;

	if (!si_seci_uart(sih))
		return 0;

	INTR_OFF(sii, intr_val);
	if (!(cc = si_seci_access_preamble(sih, sii, &origidx, &fast)))
		goto exit;

	switch (access) {
	case SECI_ACCESS_STATUSMASK_SET:
		offset = OFFSETOF(chipcregs_t, SECI_statusmask);
		retval = si_corereg(sih, SI_CC_IDX, offset, ALLONES_32, val);
		break;
	case SECI_ACCESS_STATUSMASK_GET:
		offset = OFFSETOF(chipcregs_t, SECI_statusmask);
		retval = si_corereg(sih, SI_CC_IDX, offset, 0, 0);
		break;
	case SECI_ACCESS_INTRS:
		offset = OFFSETOF(chipcregs_t, SECI_status);
		retval = si_corereg(sih, SI_CC_IDX, offset,
		                    ALLONES_32, ALLONES_32);
		break;
	case SECI_ACCESS_UART_CTS:
		offset = OFFSETOF(chipcregs_t, seci_uart_msr);
		retval = si_corereg(sih, SI_CC_IDX, offset, 0, 0);
		retval = retval & SECI_UART_MSR_CTS_STATE;
		break;
	case SECI_ACCESS_UART_RTS:
		offset = OFFSETOF(chipcregs_t, seci_uart_mcr);
		if (val) {
			/* clear forced flow control; enable auto rts */
			retval = si_corereg(sih, SI_CC_IDX, offset,
			           SECI_UART_MCR_PRTS |  SECI_UART_MCR_AUTO_RTS,
			           SECI_UART_MCR_AUTO_RTS);
		} else {
			/* set forced flow control; clear auto rts */
			retval = si_corereg(sih, SI_CC_IDX, offset,
			           SECI_UART_MCR_PRTS |  SECI_UART_MCR_AUTO_RTS,
			           SECI_UART_MCR_PRTS);
		}
		break;
	case SECI_ACCESS_UART_RXEMPTY:
		offset = OFFSETOF(chipcregs_t, SECI_status);
		retval = si_corereg(sih, SI_CC_IDX, offset, 0, 0);
		retval = (retval & SECI_STAT_SRFE) == SECI_STAT_SRFE;
		break;
	case SECI_ACCESS_UART_GETC:
		/* assumes caller checked for nonempty rx FIFO */
		offset = OFFSETOF(chipcregs_t, seci_uart_data);
		retval = si_corereg(sih, SI_CC_IDX, offset, 0, 0) & 0xff;
		break;
	case SECI_ACCESS_UART_TXFULL:
		offset = OFFSETOF(chipcregs_t, SECI_status);
		retval = si_corereg(sih, SI_CC_IDX, offset, 0, 0);
		retval = (retval & SECI_STAT_STFF) == SECI_STAT_STFF;
		break;
	case SECI_ACCESS_UART_PUTC:
		/* This register must not do a RMW otherwise it will affect the RX FIFO */
		W_REG(sii->osh, &cc->seci_uart_data, (uint32)(val & 0xff));
		retval = 0;
		break;
	default:
		ASSERT(0);
	}

exit:
	/* restore previous core */
	if (!fast)
		si_setcoreidx(sih, origidx);

	INTR_RESTORE(sii, intr_val);

	return retval;
} /* si_seci_access */

void si_seci_clk_force(si_t *sih, bool val)
{
	force_seci_clk = val;
	if (force_seci_clk) {
		si_seci_clkreq(sih, TRUE);
	} else {
		si_seci_down(sih);
	}
}

bool si_seci_clk_force_status(si_t *sih)
{
	return force_seci_clk;
}
#endif /* SECI_UART */

/** SECI Init routine, pass in seci_mode */
volatile void *
BCMINITFN(si_seci_init)(si_t *sih, uint8  seci_mode)
{
	uint32 origidx = 0;
	uint32 offset;
	si_info_t *sii;
	volatile void *ptr;
	chipcregs_t *cc;
	bool fast;
	uint32 seci_conf;

#ifdef SECI_UART
	if (seci_mode == SECI_MODE_UART) {
		if (!si_seci_uart(sih))
			return NULL;
	}
	else {
#endif /* SECI_UART */
	if (!si_seci(sih))
		return NULL;
#ifdef SECI_UART
	}
#endif /* SECI_UART */

	if (seci_mode > SECI_MODE_MASK)
		return NULL;

	sii = SI_INFO(sih);
	fast = SI_FAST(sii);
	if (!fast) {
		origidx = sii->curidx;
		if ((ptr = si_setcore(sih, CC_CORE_ID, 0)) == NULL)
			return NULL;
	} else if ((ptr = CCREGS_FAST(sii)) == NULL)
		return NULL;
	cc = (chipcregs_t *)ptr;
	ASSERT(cc);

	/* enable SECI clock */
	if (seci_mode != SECI_MODE_LEGACY_3WIRE_WLAN)
		si_seci_clkreq(sih, TRUE);

	/* put the SECI in reset */
	seci_conf = R_REG(sii->osh, &cc->SECI_config);
	seci_conf &= ~SECI_ENAB_SECI_ECI;
	W_REG(sii->osh, &cc->SECI_config, seci_conf);
	seci_conf = SECI_RESET;
	W_REG(sii->osh, &cc->SECI_config, seci_conf);

	/* set force-low, and set EN_SECI for all non-legacy modes */
	seci_conf |= SECI_ENAB_SECIOUT_DIS;
	if ((seci_mode == SECI_MODE_UART) || (seci_mode == SECI_MODE_SECI) ||
	    (seci_mode == SECI_MODE_HALF_SECI))
	{
		seci_conf |= SECI_ENAB_SECI_ECI;
	}
	W_REG(sii->osh, &cc->SECI_config, seci_conf);

	if (seci_mode != SECI_MODE_LEGACY_3WIRE_WLAN) {
		/* take seci out of reset */
		seci_conf = R_REG(sii->osh, &cc->SECI_config);
		seci_conf &= ~(SECI_RESET);
		W_REG(sii->osh, &cc->SECI_config, seci_conf);
	}

	/* set UART/SECI baud rate */
	/* hard-coded at 4MBaud for now */
	if ((seci_mode == SECI_MODE_UART) || (seci_mode == SECI_MODE_SECI) ||
	    (seci_mode == SECI_MODE_HALF_SECI)) {
		offset = OFFSETOF(chipcregs_t, seci_uart_bauddiv);
		si_corereg(sih, SI_CC_IDX, offset, 0xFF, 0xFF); /* 4MBaud */
		/* 4336 MAC clk is 80MHz */
		offset = OFFSETOF(chipcregs_t, seci_uart_baudadj);
		si_corereg(sih, SI_CC_IDX, offset, 0xFF, 0x22);
		offset = OFFSETOF(chipcregs_t, seci_uart_mcr);
		si_corereg(sih, SI_CC_IDX, offset,
			0xFF, SECI_UART_MCR_BAUD_ADJ_EN); /* 0x80 */

		/* LCR/MCR settings */
		offset = OFFSETOF(chipcregs_t, seci_uart_lcr);
		si_corereg(sih, SI_CC_IDX, offset, 0xFF,
			(SECI_UART_LCR_RX_EN | SECI_UART_LCR_TXO_EN)); /* 0x28 */
		offset = OFFSETOF(chipcregs_t, seci_uart_mcr);
			si_corereg(sih, SI_CC_IDX, offset,
			SECI_UART_MCR_TX_EN, SECI_UART_MCR_TX_EN); /* 0x01 */

#ifndef SECI_UART
		/* Give control of ECI output regs to MAC core */
		if (sih->boardflags2 & BFL2_WLCX_ATLAS) {
			offset = OFFSETOF(chipcregs_t, eci.ge35.eci_controllo);
			// enable nibbles 4 to 7
			si_corereg(sih, SI_CC_IDX, offset, 0xFFFFFFFF, 0xFFFF0000);
			offset = OFFSETOF(chipcregs_t, eci.ge35.eci_controlhi);
			// enable nibble 8
			si_corereg(sih, SI_CC_IDX, offset, 0xFFFFFFFF, 0x0000000F);
			// Tag nibbles 4 to 7
			offset = OFFSETOF(chipcregs_t, eci.ge35.eci_datatag);
			// bits 8 to 11 as TX, 24-27 as RX
			si_corereg(sih, SI_CC_IDX, offset, 0xFFFFFFFF, 0x00F000F0);
		} else {
			offset = OFFSETOF(chipcregs_t, eci.ge35.eci_controllo);
			si_corereg(sih, SI_CC_IDX, offset, ALLONES_32, ECI_MACCTRLLO_BITS);
			offset = OFFSETOF(chipcregs_t, eci.ge35.eci_controlhi);
			si_corereg(sih, SI_CC_IDX, offset, 0xFFFF, ECI_MACCTRLHI_BITS);
		}
#endif /* SECI_UART */
	}

	/* set the seci mode in seci conf register */
	seci_conf = R_REG(sii->osh, &cc->SECI_config);
	seci_conf &= ~(SECI_MODE_MASK << SECI_MODE_SHIFT);
	seci_conf |= (seci_mode << SECI_MODE_SHIFT);
	W_REG(sii->osh, &cc->SECI_config, seci_conf);

	/* Clear force-low bit */
	seci_conf = R_REG(sii->osh, &cc->SECI_config);
	seci_conf &= ~SECI_ENAB_SECIOUT_DIS;
	W_REG(sii->osh, &cc->SECI_config, seci_conf);

	/* restore previous core */
	if (!fast)
		si_setcoreidx(sih, origidx);

	return ptr;
} /* si_seci_init */

#ifdef BCMECICOEX
#define NOTIFY_BT_FM_DISABLE(sih, val) \
	si_eci_notify_bt((sih), ECI_OUT_FM_DISABLE_MASK, (val) << ECI_OUT_FM_DISABLE_SHIFT, FALSE)

/** Query OTP to see if FM is disabled */
static int
BCMINITFN(si_query_FMDisabled_from_OTP)(si_t *sih, uint16 *FMDisabled)
{
	int error = BCME_OK;
	uint bitoff = 0;
	bool wasup;
	void *oh;
	uint32 min_res_mask = 0;

	/* If there is a bit for this chip, check it */
	if (bitoff) {
		if (!(wasup = si_is_otp_powered(sih))) {
			si_otp_power(sih, TRUE, &min_res_mask);
		}

		if ((oh = otp_init(sih)) != NULL)
			*FMDisabled = !otp_read_bit(oh, OTP4325_FM_DISABLED_OFFSET);
		else
			error = BCME_NOTFOUND;

		if (!wasup) {
			si_otp_power(sih, FALSE, &min_res_mask);
		}
	}

	return error;
}

bool
si_eci(si_t *sih)
{
	return (!!(sih->cccaps & CC_CAP_ECI));
}

bool
si_seci(si_t *sih)
{
	return (sih->cccaps_ext & CC_CAP_EXT_SECI_PRESENT);
}

bool
si_gci(si_t *sih)
{
	return (sih->cccaps_ext & CC_CAP_EXT_GCI_PRESENT);
}

/** ECI Init routine */
int
BCMINITFN(si_eci_init)(si_t *sih)
{
	uint32 origidx = 0;
	si_info_t *sii;
	chipcregs_t *cc;
	bool fast;
	uint16 FMDisabled = FALSE;

	/* check for ECI capability */
	if (!(sih->cccaps & CC_CAP_ECI))
		return BCME_ERROR;

	sii = SI_INFO(sih);
	fast = SI_FAST(sii);
	if (!fast) {
		origidx = sii->curidx;
		if ((cc = (chipcregs_t *)si_setcore(sih, CC_CORE_ID, 0)) == NULL)
			return BCME_ERROR;
	} else if ((cc = (chipcregs_t *)CCREGS_FAST(sii)) == NULL)
		return BCME_ERROR;
	ASSERT(cc);

	/* disable level based interrupts */
	W_REG(sii->osh, &cc->eci.ge35.eci_intmaskhi, 0x0);
	W_REG(sii->osh, &cc->eci.ge35.eci_intmasklo, 0x0);

	/* Assign eci_output bits between 'wl' and dot11mac */
	W_REG(sii->osh, &cc->eci.ge35.eci_controllo, ECI_MACCTRLLO_BITS);
	W_REG(sii->osh, &cc->eci.ge35.eci_controlhi, ECI_MACCTRLHI_BITS);

	/* enable only edge based interrupts
	 * only toggle on bit 62 triggers an interrupt
	 */
	W_REG(sii->osh, &cc->eci.ge35.eci_eventmaskhi, 0x0);
	W_REG(sii->osh, &cc->eci.ge35.eci_eventmasklo, 0x0);

	/* restore previous core */
	if (!fast)
		si_setcoreidx(sih, origidx);

	/* if FM disabled in OTP, let BT know */
	if (!si_query_FMDisabled_from_OTP(sih, &FMDisabled)) {
		if (FMDisabled) {
			NOTIFY_BT_FM_DISABLE(sih, 1);
		}
	}

	return 0;
} /* si_eci_init */

/** Write values to BT on eci_output. */
void
si_eci_notify_bt(si_t *sih, uint32 mask, uint32 val, bool is_interrupt)
{
	uint32 offset;

	if ((sih->cccaps & CC_CAP_ECI) || (si_seci(sih))) {
		/* ECI or SECI mode */
		/* Clear interrupt bit by default */
		if (is_interrupt) {
			si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, eci.ge35.eci_outputlo),
				(1 << 30), 0);
		}

		if ((mask & 0xFFFF0000) == ECI48_OUT_MASKMAGIC_HIWORD) {
			offset = OFFSETOF(chipcregs_t, eci.ge35.eci_outputhi);
			mask = mask & ~0xFFFF0000;
		} else {
			offset = OFFSETOF(chipcregs_t, eci.ge35.eci_outputlo);
			mask = mask | (1<<30);
			val = val & ~(1 << 30);
		}

		si_corereg(sih, SI_CC_IDX, offset, mask, val);

		/* Set interrupt bit if needed */
		if (is_interrupt) {
			si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, eci.ge35.eci_outputlo),
				(1 << 30), (1 << 30));
		}
	} else if (sih->cccaps_ext & CC_CAP_EXT_GCI_PRESENT) {
		/* GCI Mode */
		if ((mask & 0xFFFF0000) == ECI48_OUT_MASKMAGIC_HIWORD) {
			mask = mask & ~0xFFFF0000;
			si_gci_direct(sih, GCI_OFFSETOF(sih, gci_output[1]), mask, val);
		}
	}
}

static void
seci_restore_coreidx(si_t *sih, uint32 origidx, bool fast)
{
	if (!fast)
		si_setcoreidx(sih, origidx);
} /* si_eci_notify_bt */

void
BCMINITFN(si_seci_down)(si_t *sih)
{
	uint32 origidx;
	bool fast;
	si_info_t *sii = SI_INFO(sih);
	chipcregs_t *cc;
	uint32 offset;

	if (!si_seci(sih) && !si_seci_uart(sih))
		return;
	/* Don't proceed if request is already made to bring down the clock */
	offset = OFFSETOF(chipcregs_t, clk_ctl_st);
	if (!(si_corereg(sih, 0, offset, 0, 0) & CLKCTL_STS_SECI_CLK_REQ))
		return;
	if (!(cc = si_seci_access_preamble(sih, sii, &origidx, &fast)))
	    goto exit;

exit:
	/* bring down the clock if up */
	si_seci_clkreq(sih, FALSE);

	/* restore previous core */
	seci_restore_coreidx(sih, origidx, fast);
}

void *
BCMINITFN(si_gci_init)(si_t *sih)
{
#ifdef HNDGCI
	si_info_t *sii = SI_INFO(sih);
#endif /* HNDGCI */

	if (sih->cccaps_ext & CC_CAP_EXT_GCI_PRESENT)
	{
		si_gci_reset(sih);

		if (!(sih->boardflags2 & (BFL2_BTCLEGACY | BFL2_BTC3WIREONLY))) {
			si_seci_clkreq(sih, TRUE);
			si_gci_seci_init(sih);
		}

		/*
		 * Set GCI Control bits 40 - 47 to be SW Controlled. These bits
		 * contain WL channel info and are sent to BT. Also allow for ECI
		 * btc_mode updates.
		 */
		si_gci_direct(sih, GCI_OFFSETOF(sih, gci_control_1),
			(GCI_WL_CHN_INFO_MASK | GCI_WL_BTC_MODE_MASK),
			(GCI_WL_CHN_INFO_MASK | GCI_WL_BTC_MODE_MASK));
	}
#ifdef HNDGCI
	hndgci_init(sih, sii->osh, HND_GCI_PLAIN_UART_MODE,
		GCI_UART_BR_115200);
#endif /* HNDGCI */

	return (NULL);
}
#endif /* BCMECICOEX */
#endif /* !(BCMDONGLEHOST) */

#if (!defined(_CFE_) && !defined(_CFEZ_)) || defined(CFG_WL)
void
si_btcgpiowar(si_t *sih)
{
	si_info_t *sii = SI_INFO(sih);
	uint origidx;
	uint intr_val = 0;
	chipcregs_t *cc;

	/* Make sure that there is ChipCommon core present &&
	 * UART_TX is strapped to 1
	 */
	if (!(sih->cccaps & CC_CAP_UARTGPIO))
		return;

	/* si_corereg cannot be used as we have to guarantee 8-bit read/writes */
	INTR_OFF(sii, intr_val);

	origidx = si_coreidx(sih);

	cc = (chipcregs_t *)si_setcore(sih, CC_CORE_ID, 0);
	ASSERT(cc != NULL);

	W_REG(sii->osh, &cc->uart0mcr, (uint8)(R_REG(sii->osh, &cc->uart0mcr) | 0x04));

	/* restore the original index */
	si_setcoreidx(sih, origidx);

	INTR_RESTORE(sii, intr_val);
}

void
si_chipcontrl_restore(si_t *sih, uint32 val)
{
	si_info_t *sii = SI_INFO(sih);
	chipcregs_t *cc;
	uint origidx = si_coreidx(sih);

	if ((cc = (chipcregs_t *)si_setcore(sih, CC_CORE_ID, 0)) == NULL) {
		SI_ERROR(("%s: Failed to find CORE ID!\n", __FUNCTION__));
		return;
	}
	W_REG(sii->osh, &cc->chipcontrol, val);
	si_setcoreidx(sih, origidx);
}

uint32
si_chipcontrl_read(si_t *sih)
{
	si_info_t *sii = SI_INFO(sih);
	chipcregs_t *cc;
	uint origidx = si_coreidx(sih);
	uint32 val;

	if ((cc = (chipcregs_t *)si_setcore(sih, CC_CORE_ID, 0)) == NULL) {
		SI_ERROR(("%s: Failed to find CORE ID!\n", __FUNCTION__));
		return -1;
	}
	val = R_REG(sii->osh, &cc->chipcontrol);
	si_setcoreidx(sih, origidx);
	return val;
}

/**
 * The SROM clock is derived from the backplane clock. 4365 (200Mhz) and 43684/6710 (242Mhz)
 * have a fast backplane clock that requires a higher-than-POR-default clock divisor ratio
 * for the SROM clock.
 */
void
si_srom_clk_set(si_t *sih)
{
#if !defined(BCMDONGLEHOST)
	si_info_t *sii = SI_INFO(sih);
	chipcregs_t *cc;
	uint origidx = si_coreidx(sih);
	uint32 val;
	uint32 divisor = 1;
	bool ht_clock_active;

	if ((cc = (chipcregs_t *)si_setcore(sih, CC_CORE_ID, 0)) == NULL) {
		SI_ERROR(("%s: Failed to find CORE ID!\n", __FUNCTION__));
		return;
	}

	val = R_REG(sii->osh, &cc->clkdiv2);
	ht_clock_active = (R_REG(sii->osh, &cc->clk_ctl_st) & CCS_HTAVAIL) ? TRUE : FALSE;
	BCM_REFERENCE(ht_clock_active);

	if (BCM4365_CHIP(sih->chip)) {
		divisor = CLKD2_SROMDIV_192; /* divide 200 by 192 -> SPROM clock ~ 1.04Mhz */
	} else if (BCM43684_CHIP(sih->chip) ||
			BCM6710_CHIP(sih->chip)) {
		divisor = CLKD2_SROMDIV_256; /* divide 242 by 256 -> SPROM clock ~ 0.95Mhz */
	} else if (BCM6715_CHIP(sih->chip)) {
		if (ht_clock_active) {
			divisor = CLKD2_SROMDIV_384; /* 320MHz/384 -> SPROM clock ~ 0.83Mhz */
		} else {
			divisor = CLKD2_SROMDIV_64; /* 50MHz by 64 -> SPROM clock ~ 0.781Mhz */
		}
	} else if (BCM6717_CHIP(sih->chip)) {
		if (sii->pub.sprom_bus == SPROM_BUS_SPI) { /* SPI sprom max clk freq is 10MHz */
			if (ht_clock_active) {
				divisor = CLKD2_SROMDIV_64; /* 582MHz/64 -> SPROM clock ~ 9.1Mhz */
			} else {
				divisor = CLKD2_SROMDIV_32; /* 40MHz/32 -> SPROM clock ~ 1.25Mhz */
			}
		} else {
			if (ht_clock_active) {
				divisor = CLKD2_SROMDIV_512; /* 582Mhz/512 -> clock ~ 1.13Mhz */
			} else {
				divisor = CLKD2_SROMDIV_64;  /* 40MHz/64 -> clock ~ 0.625Mhz */
			}
		}
	} else if (BCM6726_CHIP(sih->chip)) {
		if (!ht_clock_active) {
			/* BP on ALP */
			if (sii->pub.sprom_bus == SPROM_BUS_SPI) {
				/* SPI sprom max clk freq is 10MHz */
				divisor = CLKD2_SROMDIV_32; /* 40MHz/32 -> SPROM clock ~1.25Mhz */
			} else {
				/* max clk freq is ~1MHz */
				divisor = CLKD2_SROMDIV_64;  /* 40MHz/64 -> clock ~0.625Mhz */
			}
		} else if ((sih->otpflag & OTPFLAG_6726_HI_VOLT_DIS_BIT) == 0) {
			/* high performance package: BP on 584MHz */
			if (sii->pub.sprom_bus == SPROM_BUS_SPI) {
				/* SPI sprom max clk freq is 10MHz */
				divisor = CLKD2_SROMDIV_64; /* 584MHz/64 -> SPROM clock ~9.13Mhz */
			} else {
				/* max clk freq is ~1MHz */
				divisor = CLKD2_SROMDIV_512; /* 584Mhz/512 -> clock ~1.14Mhz */
			}
		} else {
			/* low performance package: BP on 324.45MHz */
			if (sii->pub.sprom_bus == SPROM_BUS_SPI) {
				/* SPI sprom max clk freq is 10MHz */
				divisor = CLKD2_SROMDIV_32; /* 324MHz/32 -> SPROM clock ~10.14Mhz */
			} else {
				/* max clk freq is ~1MHz */
				divisor = CLKD2_SROMDIV_512; /* 324MHz/512 -> clock ~0.633Mhz */
			}
		}
	} else if (BCM6711_CHIP(sih->chip)) {
		if (sii->pub.sprom_bus == SPROM_BUS_SPI) {
			/* SPI sprom max clk freq is 10MHz.
			 *
			 * Use SROMDIV_32 for both HT and ALP due to the minumal of SROMDIV is 32.
			 *   HT : 162MHz/32 -> SPROM clock ~ 5.06Mhz
			 *   ALP : 40MHz/32 -> SPROM clock ~ 1.25Mhz
			 */
			divisor = CLKD2_SROMDIV_32;
		} else {
			/* max clk freq is ~1MHz */
			if (ht_clock_active) {
				divisor = CLKD2_SROMDIV_192; /* 162Mhz/192 -> clock ~ 0.84Mhz */
			} else {
				divisor = CLKD2_SROMDIV_64;  /* 40MHz/64 -> clock ~ 0.625Mhz */
			}
		}
	} else {
		ASSERT(0);
	}

	W_REG(sii->osh, &cc->clkdiv2, ((val & ~CLKD2_SROM) | divisor));
	si_setcoreidx(sih, origidx);
#endif /* !defined(BCMDONGLEHOST) */
}
#endif /* (!_CFE_ && !_CFEZ_) || CFG_WL */

void
si_pmu_avb_clk_set(si_t *sih, osl_t *osh, bool set_flag)
{
#if !defined(BCMDONGLEHOST)
	switch (CHIPID(sih->chip)) {
		case BCM43684_CHIP_ID:
			si_pmu_avbtimer_enable(sih, osh, set_flag);
			break;
		default:
			break;
	}
#endif
}

void
si_btc_enable_chipcontrol(si_t *sih)
{
	si_info_t *sii = SI_INFO(sih);
	chipcregs_t *cc;
	uint origidx = si_coreidx(sih);

	if ((cc = (chipcregs_t *)si_setcore(sih, CC_CORE_ID, 0)) == NULL) {
		SI_ERROR(("%s: Failed to find CORE ID!\n", __FUNCTION__));
		return;
	}

	/* BT fix */
	W_REG(sii->osh, &cc->chipcontrol,
		R_REG(sii->osh, &cc->chipcontrol) | CC_BTCOEX_EN_MASK);

	si_setcoreidx(sih, origidx);
}

/** cache device removed state */
void si_set_device_removed(si_t *sih, bool status)
{
	si_info_t *sii = SI_INFO(sih);

	sii->device_removed = status;
}

/** check if the device is removed */
bool
si_deviceremoved(si_t *sih)
{
	uint32 w;
	si_info_t *sii = SI_INFO(sih);

	if (sii->device_removed) {
		return TRUE;
	}

	switch (BUSTYPE(sih->bustype)) {
	case PCI_BUS:
		ASSERT(SI_INFO(sih)->osh != NULL);
		w = OSL_PCI_READ_CONFIG(SI_INFO(sih)->osh, PCI_CFG_VID, sizeof(uint32));
		if ((w & 0xFFFF) != VENDOR_BROADCOM)
			return TRUE;
		break;
	default:
		break;
	}

	return FALSE;
}

bool
si_is_sprom_available(si_t *sih)
{
	return sih->sprom_bus != SPROM_BUS_NO_SPROM;
} /* si_is_sprom_available */

#if !defined(BCMDONGLEHOST)
bool
si_is_otp_disabled(si_t *sih)
{
	switch (CHIPID(sih->chip)) {
	CASE_EMBEDDED_2x2AX_CORE:
	CASE_EMBEDDED_11BE_CORE:
		return TRUE;
	default:
		return FALSE;
	}
}

bool
si_is_otp_powered(si_t *sih)
{
	return si_pmu_is_otp_powered(sih, si_osh(sih));
}

void
si_otp_power(si_t *sih, bool on, uint32* min_res_mask)
{
	si_pmu_otp_power(sih, si_osh(sih), on, min_res_mask);
	OSL_DELAY(1000);
}

/* Return BCME_NOTFOUND if the card doesn't have CIS format nvram */
int
si_cis_source(si_t *sih)
{
	/* Most PCI chips use SROM format instead of CIS */
	if (BUSTYPE(sih->bustype) == PCI_BUS && (
		!BCM43684_CHIP(sih->chip) &&
		!BCM6710_CHIP(sih->chip) &&
		!BCM6715_CHIP(sih->chip) &&
		!BCM6717_CHIP(sih->chip) &&
		!BCM6726_CHIP(sih->chip) &&
		!BCM6711_CHIP(sih->chip))) {
		return BCME_NOTFOUND;
	}

	switch (CHIPID(sih->chip)) {
	CASE_BCM43684_CHIP:
	CASE_BCM6710_CHIP:
	CASE_BCM6715_CHIP:
	CASE_BCM6717_CHIP:
	CASE_BCM6726_CHIP:
	CASE_BCM6711_CHIP:
		if (si_is_sprom_available(sih)) {
			return BCME_NOTFOUND;
		}
		return CIS_OTP;
	CASE_EMBEDDED_2x2AX_CORE:
	CASE_EMBEDDED_11BE_CORE:
		return BCME_NOTFOUND;
	default:
		return CIS_DEFAULT;
	}
} /* si_cis_source */

uint16 BCMATTACHFN(si_fabid)(si_t *sih)
{
	uint16 fabid = 0;

	switch (CHIPID(sih->chip)) {
		default:
			break;
	}

	return fabid;
}
#endif /* !defined(BCMDONGLEHOST) */

uint32 BCMATTACHFN(si_get_sromctl)(si_t *sih)
{
	chipcregs_t *cc;
	uint origidx = si_coreidx(sih);
	uint32 sromctl;
	osl_t *osh = si_osh(sih);

	cc = si_setcoreidx(sih, SI_CC_IDX);
	ASSERT((uintptr)cc);

	sromctl = R_REG(osh, &cc->sromcontrol);

	/* return to the original core */
	si_setcoreidx(sih, origidx);
	return sromctl;
}

int BCMATTACHFN(si_set_sromctl)(si_t *sih, uint32 value)
{
	chipcregs_t *cc;
	uint origidx = si_coreidx(sih);
	osl_t *osh = si_osh(sih);
	int ret = BCME_OK;

	cc = si_setcoreidx(sih, SI_CC_IDX);
	ASSERT((uintptr)cc);

	/* get chipcommon rev */
	if (si_corerev(sih) >= 32) {
		/* SpromCtrl is only accessible if CoreCapabilities.SpromSupported and
		 * SpromPresent is 1.
		 */
		if (sih->sprom_bus == SPROM_BUS_UWIRE) {
			W_REG(osh, &cc->sromcontrol, value);
		} else {
			ret = BCME_NODEVICE;
		}
	} else {
		ret = BCME_UNSUPPORTED;
	}

	/* return to the original core */
	si_setcoreidx(sih, origidx);

	return ret;
}

uint
si_core_wrapperreg(si_t *sih, uint32 coreidx, uint32 offset, uint32 mask, uint32 val)
{
	uint origidx, intr_val = 0;
	uint ret_val;
	si_info_t *sii = SI_INFO(sih);

	origidx = si_coreidx(sih);

	INTR_OFF(sii, intr_val);
	si_setcoreidx(sih, coreidx);

	ret_val = si_wrapperreg(sih, offset, mask, val);

	/* return to the original core */
	si_setcoreidx(sih, origidx);
	INTR_RESTORE(sii, intr_val);
	return ret_val;
}

/* Caller of this function should make sure is on PCIE core
 * Used in pciedev.c.
 */
void
si_pcie_disable_oobselltr(si_t *sih)
{
	ASSERT(si_coreid(sih) == PCIE2_CORE_ID);
	 if (PCIECOREREV(sih->buscorerev) >= 23)
		si_wrapperreg(sih, AI_OOBSELIND74, ~0, 0);
	 else
		si_wrapperreg(sih, AI_OOBSELIND30, ~0, 0);
}

void
si_pcie_prep_D3(si_t *sih, bool enter_D3)
{
}

uint32
si_clear_backplane_to(si_t *sih)
{
	if ((CHIPTYPE(sih->socitype) == SOCI_AI) ||
		(CHIPTYPE(sih->socitype) == SOCI_DVTBUS)) {
		return ai_clear_backplane_to(sih);
	}

	return 0;
}

void
si_update_backplane_timeouts(si_t *sih, bool enable, uint32 timeout_exp, uint32 cid)
{
#if defined(AXI_TIMEOUTS) || defined(BCM_BACKPLANE_TIMEOUT)
	/* Enable only for AXI */
	if (CHIPTYPE(sih->socitype) != SOCI_AI) {
		return;
	}

	ai_update_backplane_timeouts(sih, enable, timeout_exp, cid);
#endif /* AXI_TIMEOUTS  || BCM_BACKPLANE_TIMEOUT */
}

/*
 * This routine adds the AXI timeouts for
 * chipcommon, pcie and ARM slave wrappers
 */
void
si_slave_wrapper_add(si_t *sih)
{
#if defined(AXI_TIMEOUTS) || defined(BCM_BACKPLANE_TIMEOUT)
	/* Enable only for AXI */
	if ((CHIPTYPE(sih->socitype) != SOCI_AI) &&
		(CHIPTYPE(sih->socitype) != SOCI_DVTBUS)) {
		return;
	}

	/* All required slave wrappers are added in ai_scan */
	ai_update_backplane_timeouts(sih, TRUE, AXI_TO_VAL, 0);

#ifdef DISABLE_PCIE2_AXI_TIMEOUT
	ai_update_backplane_timeouts(sih, FALSE, 0, PCIE_CORE_ID);
	ai_update_backplane_timeouts(sih, FALSE, 0, PCIE2_CORE_ID);
#endif

#endif /* AXI_TIMEOUTS  || BCM_BACKPLANE_TIMEOUT */

}

#ifndef BCMDONGLEHOST
/* read from pcie space using back plane  indirect access */
/* Set Below mask for reading 1, 2, 4 bytes in single read */
/* #define	SI_BPIND_1BYTE		0x1 */
/* #define	SI_BPIND_2BYTE		0x3 */
/* #define	SI_BPIND_4BYTE		0xF */
int
si_bpind_access(si_t *sih, uint32 addr_high, uint32 addr_low, int32 * data, bool read)
{

	uint32 status = 0;
	uint8 mask = SI_BPIND_4BYTE;
	int ret_val = BCME_OK;

	/* Program Address low and high fields */
	si_ccreg(sih, OFFSETOF(chipcregs_t, bp_addrlow), ~0, addr_low);
	si_ccreg(sih, OFFSETOF(chipcregs_t, bp_addrhigh), ~0, addr_high);

	if (read) {
		/* Start the read */
		si_ccreg(sih, OFFSETOF(chipcregs_t, bp_indaccess), ~0,
			CC_BP_IND_ACCESS_START_MASK | mask);
	} else {
		/* Write the data and force the trigger */
		si_ccreg(sih, OFFSETOF(chipcregs_t, bp_data), ~0, *data);
		si_ccreg(sih, OFFSETOF(chipcregs_t, bp_indaccess), ~0,
			CC_BP_IND_ACCESS_START_MASK |
			CC_BP_IND_ACCESS_RDWR_MASK | mask);

	}

	/* Wait for status to be cleared */
	SPINWAIT(((status = si_ccreg(sih, OFFSETOF(chipcregs_t, bp_indaccess), 0, 0)) &
		CC_BP_IND_ACCESS_START_MASK), 1000);

	if (status & (CC_BP_IND_ACCESS_START_MASK | CC_BP_IND_ACCESS_ERROR_MASK)) {
		ret_val = BCME_ERROR;
		SI_ERROR(("Action Failed for address 0x%08x:0x%08x \t status: 0x%x\n",
			addr_high, addr_low, status));
	} else {
		/* read data */
		if (read)
			*data = si_ccreg(sih, OFFSETOF(chipcregs_t, bp_data), 0, 0);
	}
	return ret_val;
}
#endif /* !BCMDONGLEHOST */

uint
si_introff(si_t *sih)
{
	si_info_t *sii = SI_INFO(sih);
	uint intr_val = 0;
	INTR_OFF(sii, intr_val);
	return intr_val;
}

void
si_intrrestore(si_t *sih, uint intr_val)
{
	si_info_t *sii = SI_INFO(sih);
	INTR_RESTORE(sii, intr_val);
}

void
si_nvram_res_masks(si_t *sih, uint32 *min_mask, uint32 *max_mask)
{
	si_info_t *sii = SI_INFO(sih);
	/* Apply nvram override to min mask */
	if (sii->min_mask_valid == TRUE) {
		SI_MSG(("Applying rmin=%d to min_mask\n", sii->nvram_min_mask));
		*min_mask = sii->nvram_min_mask;
	}
	/* Apply nvram override to max mask */
	if (sii->max_mask_valid == TRUE) {
		SI_MSG(("Applying rmax=%d to max_mask\n", sii->nvram_max_mask));
		*max_mask = sii->nvram_max_mask;
	}
}

#ifdef DONGLEBUILD

/* Set the SDIO drive strength */
int
si_set_sdio_drive_strength(si_t *sih, uint32 ma)
{
	int ret;

	ret = BCME_UNSUPPORTED;

	return ret;
}

/* Get the SDIO drive strength */
int
si_get_sdio_drive_strength(si_t *sih, uint32 *ma)
{
	int ret;

	ret = BCME_UNSUPPORTED;

	return ret;
}

/* if the logs could be gathered, host could be notified with to take logs or not  */
bool
si_check_enable_backplane_log(si_t *sih)
{
	if (CHIPTYPE(sih->socitype) == SOCI_AI) {
		return ai_check_enable_backplane_log(sih);
	}
	return TRUE;
}
#endif /* DONGLEBUILD */

#ifndef DONGLEBUILD
static void
si_reset_otp_ctrl(si_t *sih, osl_t *osh, uint devid, volatile void *regs)
{
	uint32 savewin_pa;
	uint32 regval;
	/* save BAR0 */
	savewin_pa = OSL_PCI_READ_CONFIG(osh, PCI_BAR0_WIN, sizeof(uint32));

	/* toggle OTP power */
	OSL_PCI_WRITE_CONFIG(osh, PCI_BAR0_WIN, 4,
			si_enum_base_pa(devid) + GCI_BLOCK_BASE);
	regval = R_REG(osh, ((volatile uint32*)((volatile uint8*)regs+OFFSET_OTP_CTRL)));
	W_REG(osh, ((volatile uint32*) ((volatile uint8*)regs+OFFSET_OTP_CTRL)),
		regval+OTP_PWR_DIS);
	(void)R_REG(osh, ((volatile uint32*)((volatile uint8*)regs+OFFSET_OTP_CTRL)));
	/* delay before clearing to avoid any board/xtal glitch */
	OSL_DELAY(100);
	W_REG(osh, ((volatile uint32*) ((volatile uint8*)regs+OFFSET_OTP_CTRL)), regval);
	(void)R_REG(osh, ((volatile uint32*)((volatile uint8*)regs+OFFSET_OTP_CTRL)));

	/* restore BAR0 */
	OSL_PCI_WRITE_CONFIG(osh, PCI_BAR0_WIN, 4, savewin_pa);
	OSL_DELAY(1000);
	return;
}
#endif /* DONGLEBUILD */

#if !defined(BCMDONGLEHOST) && !defined(DONGLEBUILD)
void
si_gci_hw_semaphore_request(si_t *sih)
{
	/* Set the request bit */
	si_gci_direct(sih, GCI_OFFSETOF(sih, gci_hw_sema_func0_req),
		GCI_HW_SEMA_FUN0_REQ_MASK, 1);

	/* Wait for the grant bit */
	SPINWAIT(!(si_gci_direct(sih, GCI_OFFSETOF(sih, gci_hw_sema_func_sts),
		0, 0) & GCI_HW_SEMA_FSTS_GRANT_FUN0_MASK), 1000);
}

void
si_gci_hw_semaphore_finish(si_t *sih)
{
	/* Clear the request bit */
	si_gci_direct(sih, GCI_OFFSETOF(sih, gci_hw_sema_func0_req),
			GCI_HW_SEMA_FUN0_REQ_MASK, 0);
}
#endif /* !(BCMDONGLEHOST) && !defined(DONGLEBUILD) */
