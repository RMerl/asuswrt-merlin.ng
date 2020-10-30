/*
 * Misc utility routines for accessing chip-specific features
 * of the SiliconBackplane-based Broadcom chips.
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
 * $Id: siutils.c 787019 2020-05-14 03:53:09Z $
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
#if !defined(BCMDONGLEHOST)
#include <pci_core.h>
#include <pcie_core.h>
#include <nicpci.h>
#include <bcmnvram.h>
#include <bcmsrom.h>
#include <hndtcam.h>
#endif /* !defined(BCMDONGLEHOST) */
#ifdef BCMPCIEDEV
#include <pciedev.h>
#endif /* BCMPCIEDEV */
#include <pcicfg.h>
#include <sbpcmcia.h>
#include <sbsysmem.h>
#include <sbsocram.h>
#if defined(BCMECICOEX) || !defined(BCMDONGLEHOST)
#include <bcmotp.h>
#endif /* BCMECICOEX || !BCMDONGLEHOST */
#include <hndpmu.h>
#ifdef BCMSPI
#include <spid.h>
#endif /* BCMSPI */
#if !defined(BCMDONGLEHOST) && !defined(BCM_BOOTLOADER) && defined(SR_ESSENTIALS)
#include <saverestore.h>
#endif // endif

#ifdef BCM_SDRBL
#include <hndcpu.h>
#endif /* BCM_SDRBL */
#ifdef HNDGCI
#include <hndgci.h>
#endif /* HNDGCI */
#ifdef WLGCIMBHLR
#include <hnd_gci.h>
#endif /* WLGCIMBHLR */
#ifdef BCMULP
#include <ulp.h>
#endif /* BCMULP */
#include <hndlhl.h>

#include <lpflags.h>

#include "siutils_priv.h"
#ifdef SECI_UART
/* Defines the set of GPIOs to be used for SECI UART if not specified in NVRAM */
/* For further details on each ppin functionality please refer to PINMUX table in
 * Top level architecture of BCMXXXX Chip
 */
#define DEFAULT_SECI_UART_PINMUX	0x08090a0b
#define DEFAULT_SECI_UART_PINMUX_43430	0x0102
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

#define GCI_FEM_CTRL_WAR 0x11111111

#ifndef AXI_TO_VAL
#define AXI_TO_VAL 19
#endif	/* AXI_TO_VAL */

#ifndef AXI_TO_VAL_4347
/*
 * Increase BP timeout for fast clock and short PCIe timeouts
 * New timeout: 2 ** 25 cycles
 */
#define AXI_TO_VAL_4347	25
#endif /* AXI_TO_VAL_4347 */

#define BCM47622_WLAN_DEV_A	0 /**< prefix in NVRAM file */
#define BCM47622_WLAN_DEV_B	1 /**< prefix in NVRAM file */

#if (defined(BCA_CPEROUTER) || defined(BCA_HNDROUTER)) && !defined(DONGLEBUILD) && \
	defined(CONFIG_BCM947622)
#define BCAWLAN205004
#endif /* (BCA_CPEROUTER || BCA_HNDROUTER) && !DONGLEBUILD && CONFIG_BCM947622 */

#ifdef BCAWLAN205004
#define SICF_MCLKE              0x0001         /**< Mac core clock Enable */
#define SICF_FCLKON             0x0002         /**< Force clocks On */
#define SICF_FASTCLKRQ          0x0020         /**< bit5, introduced in rev 130 */
#define EMBEDDED2x2AX_D11_MWR0_ADDR 0x84101000 /**< d11 master wrapper #0 backplane addr */
#define EMBEDDED2x2AX_D11_MWR1_ADDR 0x84102000 /**< d11 master wrapper #1 backplane addr */
#define EMBEDDED2x2AX_PMU_ADDR      0x84005000
#define PMU_WD_CNTR_REG_OFFSET      0x634
#define IS_47622A0_SLAVE_SLICE(devid, chiprev, enum_base) \
	(devid == EMBEDDED_2x2AX_ID && chiprev == 0 && enum_base == BCM47622_DEV_B_PHYS_ADDR)
#endif /* BCAWLAN205004 */

/* local prototypes */
static si_info_t *si_doattach(si_info_t *sii, uint devid, osl_t *osh, volatile void *regs,
                              uint bustype, void *sdh, char **vars, uint *varsz);
static bool si_buscore_prep(si_info_t *sii, uint bustype, uint devid, void *sdh);
static bool si_buscore_setup(si_info_t *sii, chipcregs_t *cc, uint bustype, uint32 savewin,
	uint *origidx, volatile void *regs);

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
void si_gci_chipctrl_overrides(osl_t *osh, si_t *sih, char *pvars);
static void si_gci_get_chipctrlreg_ringidx_base4(uint32 pin, uint32 *regidx, uint32 *pos);
static uint8 si_gci_get_chipctrlreg_ringidx_base8(uint32 pin, uint32 *regidx, uint32 *pos);
static void si_gci_gpio_chipcontrol(si_t *si, uint8 gpoi, uint8 opt);
uint8 si_gci_gpio_status(si_t *sih, uint8 gci_gpio, uint8 mask, uint8 value);
static void si_gci_enable_gpioint(si_t *sih, bool enable);
#endif /* (!_CFE_ && !_CFEZ_) || CFG_WL */
#if defined(BCMECICOEX) || defined(SECI_UART)
static chipcregs_t * seci_set_core(si_t *sih, uint32 *origidx, bool *fast);
#endif // endif
#endif /* !defined(BCMDONGLEHOST) */

static bool si_pmu_is_ilp_sensitive(uint32 idx, uint regoff);

#if defined(DONGLEBUILD)
#if	!defined(NVSRCX)
static char * BCMATTACHFN(si_getkvars)(void);
static int BCMATTACHFN(si_getkvarsz)(void);
#endif // endif
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

#ifdef SR_DEBUG
static const uint32 si_power_island_test_array[] = {
	0x0000, 0x0001, 0x0010, 0x0011,
	0x0100, 0x0101, 0x0110, 0x0111,
	0x1000, 0x1001, 0x1010, 0x1011,
	0x1100, 0x1101, 0x1110, 0x1111
};
#endif /* SR_DEBUG */

int do_4360_pcie2_war = 0;

#ifdef BCMULP
/* Variable to store boot_type: warm_boot/cold_boot/etc. */
static int boot_type = 0;
#endif // endif

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
 * bustype - pci/pcmcia/sb/sdio/etc
 * @param sdh  opaque pointer pointing to e.g. a Linux 'struct pci_dev'
 * vars - pointer to a to-be created pointer area for "environment" variables. Some callers of this
 *        function set 'vars' to NULL, making dereferencing of this parameter undesired.
 * varsz - pointer to int to return the size of the vars
 */
si_t *
BCMATTACHFN(si_attach)(uint devid, osl_t *osh, volatile void *regs,
                       uint bustype, void *sdh, char **vars, uint *varsz)
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
#endif // endif
	sii->cores_info = cores_info;

	if (si_doattach(sii, devid, osh, regs, bustype, sdh, vars, varsz) == NULL) {
		MFREE(osh, sii, sizeof(si_info_t));
#ifndef _RTE_
		MFREE(osh, cores_info, sizeof(si_cores_info_t));
#endif // endif
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
#if	!defined(NVSRCX)
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
#endif // endif
#endif /* DONGLEBUILD */

/**
 * Returns the backplane address of the chipcommon core for a particular chip.
 * Returns a physical, not a logical address.
 */
uint32
BCMATTACHFN(si_enum_base)(uint devid)
{
#ifndef DONGLEBUILD
	/* NIC/DHD build */
	switch (devid) {
		CASE_BCM43684_CHIP:
		case BCM43684_D11AX_ID:
		case BCM43684_D11AX2G_ID:
		case BCM43684_D11AX5G_ID:
		case BCM43684_D11AX6G_ID:
		CASE_BCM6710_CHIP:
		case BCM6710_D11AX_ID:
		case BCM6710_D11AX2G_ID:
		case BCM6710_D11AX5G_ID:
		case BCM6710_D11AX6G_ID:
		case BCM43692_D11AX_ID:
		case BCM43692_D11AX2G_ID:
		case BCM43692_D11AX5G_ID:
		CASE_BCM6715_CHIP:
		case BCM6715_D11AX_ID:
		case BCM6715_D11AX2G_ID:
		case BCM6715_D11AX5G_ID:
			return 0x28000000; /* chips using backplane address 0x2800_0000 */
		case BCM7271_CHIP_ID:
		case BCM7271_D11AC_ID:
		case BCM7271_D11AC2G_ID:
		case BCM7271_D11AC5G_ID:
			return 0xF1800000;
		case BCM63178_CHIP_ID: //equals EMBEDDED_2x2AX_ID, so don't add separately
		case BCM47622_CHIP_ID:
		case EMBEDDED_2x2AX_DEV2G_ID:
		case EMBEDDED_2x2AX_DEV5G_ID:
			ASSERT(0); /* si->enum_base is determined by pseudo pcie bar0 reg instead */
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

		regs = REG_MAP(si_enum_base(device_id), SI_CORE_SIZE); // map physical to virtual
		cores_info = (si_cores_info_t *)&ksii_cores_info;
		ksii.cores_info = cores_info;

		/* XXX Use osh as the deciding factor if the memory management
		 * system has been initialized. Pass non-NULL vars & varsz only
		 * if memory management has been initialized. Otherwise MALLOC()
		 * will fail/crash.
		 */
#if defined(BCMDONGLEHOST)
		ASSERT(osh);
#endif // endif
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
		if (PMUCTL_ENAB(&ksii.pub)) {
			/* based on 32KHz ILP clock */
			wd_msticks = 32;
		} else {
#if !defined(BCMDONGLEHOST)
			if (CCREV(ksii.pub.ccrev) < 18)
				wd_msticks = si_clock(&ksii.pub) / 1000;
			else
				wd_msticks = si_alp_clock(&ksii.pub) / 1000;
#else
			wd_msticks = ALP_CLOCK / 1000;
#endif /* !defined(BCMDONGLEHOST) */
		}

		ksii_attached = TRUE;
		SI_MSG(("si_kattach done. ccrev = %d, wd_msticks = %d\n",
		        CCREV(ksii.pub.ccrev), wd_msticks));
	}

	return &ksii.pub;
} /* si_kattach */

static bool
BCMATTACHFN(si_buscore_prep)(si_info_t *sii, uint bustype, uint devid, void *sdh)
{
	BCM_REFERENCE(sdh);
	BCM_REFERENCE(devid);
	/* need to set memseg flag for CF card first before any sb registers access */
	if (BUSTYPE(bustype) == PCMCIA_BUS)
		sii->memseg = TRUE;

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

	SI_MSG(("%s: pmu access, offset: %x\n", __FUNCTION__, offset));
	if (!(sii->pub.cccaps & CC_CAP_PMU)) {
		goto done;
	}
	if (AOB_ENAB(&sii->pub)) {
		uint pmucoreidx;
		pmuregs_t *pmu;
		SI_MSG(("%s: AOBENAB: %x\n", __FUNCTION__, offset));
		origidx = sii->curidx;
		pmucoreidx = si_findcoreidx(&sii->pub, PMU_CORE_ID, 0);
		pmu = si_setcoreidx(&sii->pub, pmucoreidx);
		/* XXX: note: this function is used by dhd and possible 64 bit compilation needs
		 * a cast to (unsigned long) for avoiding a compilation error.
		 */
		pmuaddr = (uint32)(uintptr)((volatile uint8*)pmu + offset);
		si_setcoreidx(sih, origidx);
	} else
		pmuaddr = SI_ENUM_BASE(sih) + offset;

done:
	printf("%s: addrRET: %x\n", __FUNCTION__, pmuaddr);
	return pmuaddr;
}

static bool
BCMATTACHFN(si_buscore_setup)(si_info_t *sii, chipcregs_t *cc, uint bustype, uint32 savewin,
	uint *origidx, volatile void *regs)
{
	si_cores_info_t *cores_info = (si_cores_info_t *)sii->cores_info;
	bool pci, pcie, pcie_gen2 = FALSE;
	uint i;
	uint pciidx, pcieidx, pcirev, pcierev;

#if defined(BCM_BACKPLANE_TIMEOUT) || defined(AXI_TIMEOUTS)
	/* first, enable backplane timeouts */
	si_slave_wrapper_add(&sii->pub);
#endif // endif
	sii->curidx = 0;

	cc = si_setcoreidx(&sii->pub, SI_CC_IDX);
	ASSERT((uintptr)cc);

	/* get chipcommon rev */
	sii->pub.ccrev = (int)si_corerev(&sii->pub);

	/* get chipcommon chipstatus */
	if (CCREV(sii->pub.ccrev) >= 11)
		sii->pub.chipst = R_REG(sii->osh, &cc->chipstatus);

	/* get chipcommon capabilites */
	sii->pub.cccaps = R_REG(sii->osh, &cc->capabilities);
	/* get chipcommon extended capabilities */

	if (CCREV(sii->pub.ccrev) >= 35)
		sii->pub.cccaps_ext = R_REG(sii->osh, &cc->capabilities_ext);

	/* get pmu rev and caps */
	if (sii->pub.cccaps & CC_CAP_PMU) {
		if (AOB_ENAB(&sii->pub)) {
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
		} else
			sii->pub.pmucaps = R_REG(sii->osh, &cc->pmucapabilities);

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
			i, cid, crev, cores_info->coresba[i], cores_info->coresba_size[i],
			OSL_OBFUSCATE_BUF(cores_info->regs[i])));

		if (BUSTYPE(bustype) == SI_BUS) {
#ifdef BCMPCIEDEV_ENABLED
			if (cid == PCIE2_CORE_ID) {
				pcieidx = i;
				pcierev = crev;
				pcie = TRUE;
				pcie_gen2 = TRUE;
			}
#endif // endif
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
		} else if ((BUSTYPE(bustype) == PCMCIA_BUS) &&
		           (cid == PCMCIA_CORE_ID)) {
			sii->pub.buscorerev = crev;
			sii->pub.buscoretype = cid;
			sii->pub.buscoreidx = i;
		}

		/* find the core idx before entering this func. */
		if ((savewin && (savewin == cores_info->coresba[i])) ||
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

static const char BCMATTACHDATA(rstr_regwindowsz)[] = "regwindowsz";
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
#endif // endif
static const char BCMATTACHDATA(rstr_muxenab)[] = "muxenab";
static const char BCMATTACHDATA(rstr_gpiopulldown)[] = "gpdn";
static const char BCMATTACHDATA(rstr_devid)[] = "devid";
static const char BCMATTACHDATA(rstr_wl0id)[] = "wl0id";
static const char BCMATTACHDATA(rstr_devpathD)[] = "devpath%d";
static const char BCMATTACHDATA(rstr_D_S)[] = "%d:%s";
static const char BCMATTACHDATA(rstr_swdenab)[] = "swdenable";
static const char BCMATTACHDATA(rstr_spurconfig)[] = "spurconfig";
static const char BCMATTACHDATA(rstr_wowl_gpio)[] = "wowl_gpio";
static const char BCMATTACHDATA(rstr_lpflags)[] = "lpflags";
static const char BCMATTACHDATA(rstr_fuart_pup_rx_cts)[] = "fuart_pup_rx_cts";

static uint32
BCMATTACHFN(si_fixup_vid_overrides)(si_info_t *sii, char *pvars, uint32 conf_vid)
{
	BCM_REFERENCE(pvars);

	if ((sii->pub.boardvendor != VENDOR_APPLE)) {
		return conf_vid;
	}

	switch (sii->pub.boardtype)
	{
		/* Check for the SROM value */
		case BCM94360X51P2:
		case BCM94360X29C:
		case BCM94360X29CP2:
		case BCM94360X51:
		case BCM943602X87:
		case BCM943602X238D:
		case BCM94350X14:
			/* Take the PCIe configuration space subsystem ID */
			sii->pub.boardtype = (conf_vid >> 16) & 0xffff;
			break;

		default:
			/* Do nothing */
			break;
	}

	return conf_vid;
}

static void
BCMATTACHFN(si_nvram_process)(si_info_t *sii, char *pvars)
{
	uint w = 0;
	if (BUSTYPE(sii->pub.bustype) == PCMCIA_BUS) {
		w = getintvar(pvars, rstr_regwindowsz);
		sii->memseg = (w <= CFTABLE_REGWIN_2K) ? TRUE : FALSE;
	}

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

		/* Override high priority fixups */
		si_fixup_vid_overrides(sii, pvars, w);
		break;

	case PCMCIA_BUS:
		sii->pub.boardvendor = getintvar(pvars, rstr_manfid);
		sii->pub.boardtype = getintvar(pvars, rstr_prodid);
		break;

	case SI_BUS:
	case JTAG_BUS:
#ifdef BCMPCIEDEV_SROM_FORMAT
		if (BCMPCIEDEV_ENAB() && si_is_sprom_available(&sii->pub) && pvars &&
			getvar(pvars, rstr_subvid)) {
			sii->pub.boardvendor = getintvar(pvars, rstr_subvid);
		} else
#endif // endif
		sii->pub.boardvendor = VENDOR_BROADCOM;
		if (pvars == NULL || ((sii->pub.boardtype = getintvar(pvars, rstr_prodid)) == 0))
			if ((sii->pub.boardtype = getintvar(pvars, rstr_boardtype)) == 0)
				sii->pub.boardtype = 0xffff;

		if (CHIPTYPE(sii->pub.socitype) == SOCI_UBUS) {
			/* do a pci config read to get subsystem id and subvendor id */
			w = OSL_PCI_READ_CONFIG(sii->osh, PCI_CFG_SVID, sizeof(uint32));
			sii->pub.boardvendor = w & 0xffff;
			sii->pub.boardtype = (w >> 16) & 0xffff;
		}
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
#endif // endif

#ifdef BCM_SDRBL
	sii->pub.boardflags2 |= ((!CHIP_HOSTIF_USB(&(sii->pub))) ? ((si_arm_sflags(&(sii->pub))
				 & SISF_SDRENABLE) ?  BFL2_SDR_EN:0):
				 (((uint)getintvar(pvars, "boardflags2")) & BFL2_SDR_EN));
#endif /* BCM_SDRBL */
	sii->pub.boardflags4 = getintvar(pvars, rstr_boardflags4);
} /* si_nvram_process */

#endif /* !defined(BCMDONGLEHOST) */

#if defined(CONFIG_XIP) && defined(BCMTCAM)
extern uint8 patch_pair;
#endif /* CONFIG_XIP && BCMTCAM */

#if !defined(BCMDONGLEHOST)
#if (!defined(_CFE_) && !defined(_CFEZ_)) || defined(CFG_WL)
typedef struct {
	uint8 uart_tx;
	uint32 uart_rx;
} si_mux_uartopt_t;

/* note: each index corr to MUXENAB4335_UART mask >> shift - 1 */
static const si_mux_uartopt_t BCMATTACHDATA(mux4335_uartopt)[] = {
		{CC4335_PIN_GPIO_06, CC4335_PIN_GPIO_02},
		{CC4335_PIN_GPIO_12, CC4335_PIN_GPIO_13},
		{CC4335_PIN_SDIO_DATA0, CC4335_PIN_SDIO_CMD},
		{CC4335_PIN_RF_SW_CTRL_9, CC4335_PIN_RF_SW_CTRL_8}
};

/* note: each index corr to MUXENAB4335_HOSTWAKE mask > shift - 1 */
static const uint8 BCMATTACHDATA(mux4335_hostwakeopt)[] = {
		CC4335_PIN_GPIO_00,
		CC4335_PIN_GPIO_05,
		CC4335_PIN_GPIO_09
};

static const si_mux_uartopt_t BCMATTACHDATA(mux4350_uartopt)[] = {
		{CC4350_PIN_GPIO_00, CC4350_PIN_GPIO_01},
		{CC4350_PIN_GPIO_05, CC4350_PIN_GPIO_04},
		{CC4350_PIN_GPIO_15, CC4350_PIN_GPIO_14},
};

/* note: each index corr to MUXENAB4350_HOSTWAKE mask >> shift - 1 */
static const uint8 BCMATTACHDATA(mux4350_hostwakeopt)[] = {
		CC4335_PIN_GPIO_00,
};

#ifdef SECI_UART
#define NUM_SECI_UART_GPIOS	4
static const char BCMATTACHDATA(rstr_seci_uart_gpios)[] = "seci_uart_gpios";
static const char BCMATTACHDATA(rstr_seci_uart_fnsels)[] = "seci_uart_fnsels";
static bool fuart_pullup_rx_cts_enab = FALSE;
static bool fast_uart_init = FALSE;
static uint32 fast_uart_tx;
static uint32 fast_uart_functionsel;
static uint32 fast_uart_pup;
static uint32 fast_uart_rx;
static uint32 fast_uart_cts_in;
#endif /* SECI_UART */

/* note: each index corr to MUXENAB4349_HOSTWAKE mask >> shift - 1 */
static const uint8 BCMATTACHDATA(mux4349_hostwakeopt)[] = {
		CC4349_PIN_GPIO_00,
};

void
BCMATTACHFN(si_swdenable)(si_t *sih, uint32 swdflag)
{
	switch (CHIPID(sih->chip)) {
	case BCM4369_CHIP_GRPID:
		if (swdflag) {
			/* Enable ARM debug clk, which is required for the ARM debug
			 * unit to operate
			 */
			si_pmu_chipcontrol(sih, PMU_CHIPCTL5, (1 << ARMCR4_DBG_CLK_BIT),
				(1 << ARMCR4_DBG_CLK_BIT));
			/* Force HT clock in Chipcommon. The HT clock is required for backplane
			 * access via SWD
			 */
			si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, clk_ctl_st), CCS_FORCEHT,
				CCS_FORCEHT);
			/* Set TAP_SEL so that ARM is the first and the only TAP on the TAP chain.
			 * Must do a chip reset to clear this bit
			 */
			si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, jtagctrl),
				JCTRL_TAPSEL_BIT, JCTRL_TAPSEL_BIT);
			SI_MSG(("%s: set arm_dbgclk, ForceHTClock and tap_sel bit\n",
				__FUNCTION__));
		}
		break;
	default:
		/* swdenable specified for an unsupported chip */
		ASSERT(0);
		break;
	}
}

/** want to have this available all the time to switch mux for debugging */
void
BCMATTACHFN(si_muxenab)(si_t *sih, uint32 w)
{
	uint32 chipcontrol, pmu_chipcontrol;

	pmu_chipcontrol = si_pmu_chipcontrol(sih, 1, 0, 0);
	chipcontrol = si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, chipcontrol),
	                         0, 0);

	switch (CHIPID(sih->chip)) {
	case BCM4360_CHIP_ID:
	case BCM43460_CHIP_ID:
	case BCM4352_CHIP_ID:
	case BCM43526_CHIP_ID:
	CASE_BCM43602_CHIP:
		if (w & MUXENAB_UART)
			chipcontrol |= CCTRL4360_UART_MODE;
		break;
	CASE_BCM43684_CHIP:
	CASE_BCM6715_CHIP:
		if (w & MUXENAB_UART) { /* 684 lacks 'super mux' so no si_gci_set_functionsel() */
			si_gci_chipcontrol(sih, CC_GCI_CHIPCTRL_00,
				BCM_BIT(18), BCM_BIT(18)); /* uart_mode=1 */
		}
		break;
	case BCM53573_CHIP_GRPID:
		/* Just place holder for now */
	case BCM4347_CHIP_GRPID:
	case BCM43570_CHIP_ID:
		if (w & MUXENAB4350_UART_MASK) {
			uint32 uart_rx = 0, uart_tx = 0;
			uint8 uartopt_idx = (w & MUXENAB4350_UART_MASK) - 1;
			uint8 uartopt_size = sizeof(mux4350_uartopt)/sizeof(mux4350_uartopt[0]);

			if (uartopt_idx < uartopt_size) {
				uart_rx = mux4350_uartopt[uartopt_idx].uart_rx;
				uart_tx = mux4350_uartopt[uartopt_idx].uart_tx;
#ifdef BOOTLOADER_CONSOLE_OUTPUT
				uart_rx = 0;
				uart_tx = 1;
#endif // endif
				if ((CHIPREV(sih->chiprev) >= 3) ||
				    (CHIPID(sih->chip) == BCM43570_CHIP_ID)) {
					si_gci_set_functionsel(sih, uart_rx, CC4350C_FNSEL_UART);
					si_gci_set_functionsel(sih, uart_tx, CC4350C_FNSEL_UART);
				} else {
					si_gci_set_functionsel(sih, uart_rx, CC4350_FNSEL_UART);
					si_gci_set_functionsel(sih, uart_tx, CC4350_FNSEL_UART);
				}
			} else {
				SI_MSG(("si_muxenab: Invalid uart OTP setting\n"));
			}
		}
		if (w & MUXENAB4350_HOSTWAKE_MASK) {
			uint8 hostwake = 0;
			switch (CHIPID(sih->chip)) {
			case BCM43570_CHIP_ID: {
				/*
				* PCIE, USB
				* 0xF0 : use WOWL GPIO as host wake up pin
				*/
#ifdef WOWL_GPIO
				hostwake = WOWL_GPIO;
#else
				hostwake = CC4350_GPIO_COUNT;
#endif /* WOWL_GPIO */
				if (getvar(NULL, rstr_wowl_gpio) != NULL)
					hostwake =  (uint8)getintvar(NULL, rstr_wowl_gpio);
				if (hostwake < CC4350_GPIO_COUNT) {
					si_gci_set_functionsel(sih, hostwake,
						CC4350_FNSEL_SAMEASPIN);
				}
			}
			break;

			default: {
				/*
				* SDIO
				* 0x10 : use GPIO0 as host wake up pin
				*/
				uint8 hostwake_ix = ((w & MUXENAB4350_HOSTWAKE_MASK) >>
					MUXENAB4350_HOSTWAKE_SHIFT) - 1;

				if (hostwake_ix > (sizeof(mux4350_hostwakeopt) /
					sizeof(mux4350_hostwakeopt[0]) - 1)) {
					SI_ERROR(("%s: wrong index %d for hostwake\n",
						__FUNCTION__, hostwake_ix));
					break;
				}

				hostwake = mux4350_hostwakeopt[hostwake_ix];
				si_gci_set_functionsel(sih, hostwake, CC4350_FNSEL_MISC1);
			}
			break;

			}
		}

		break;

	case BCM4369_CHIP_GRPID:
		/* TBD fill */
		break;

	default:
		/* muxenab specified for an unsupported chip */
		ASSERT(0);
		break;
	}

	/* write both updated values to hw */
	si_pmu_chipcontrol(sih, 1, ~0, pmu_chipcontrol);
	si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, chipcontrol),
	           ~0, chipcontrol);
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

static const char BCMATTACHDATA(rstr_host_wake_opt)[] = "host_wake_opt";
uint8
BCMATTACHFN(si_gci_host_wake_gpio_init)(si_t *sih)
{
	uint8  host_wake_gpio = CC_GCI_GPIO_INVALID;
	uint32 host_wake_opt;

	/* parse the device wake opt from nvram */
	/* decode what that means for specific chip */
	if (getvar(NULL, rstr_host_wake_opt) == NULL)
		return host_wake_gpio;

	host_wake_opt = getintvar(NULL, rstr_host_wake_opt);
	switch (CHIPID(sih->chip)) {
	case BCM53573_CHIP_GRPID:
		host_wake_gpio = host_wake_opt & 0xff;
		si_gci_enable_gpio(sih, host_wake_gpio,
			1 << host_wake_gpio, 0x00);
		break;
	default:
		SI_ERROR(("host wake not supported for 0x%04x yet\n", CHIPID(sih->chip)));
		break;
	}
	return host_wake_gpio;
} /* si_gci_host_wake_gpio_init */

void
si_gci_host_wake_gpio_enable(si_t *sih, uint8 gpio, bool state)
{
	switch (CHIPID(sih->chip)) {
	case BCM53573_CHIP_GRPID:
		si_gci_enable_gpio(sih, gpio, 1 << gpio,
			state ? 1 << gpio : 0x00);
		break;
	default:
		SI_ERROR(("host wake not supported for 0x%04x yet\n", CHIPID(sih->chip)));
		break;
	}
}

void
si_gci_time_sync_gpio_enable(si_t *sih, uint8 gpio, bool state)
{
	switch (CHIPID(sih->chip)) {
	case BCM4369_CHIP_GRPID:
	case BCM4347_CHIP_GRPID:
		si_gci_enable_gpio(sih, gpio, 1 << gpio,
			state ? 1 << gpio : 0x00);
		break;
	default:
		SI_ERROR(("Time sync not supported for 0x%04x yet\n", CHIPID(sih->chip)));
		break;
	}
}

#define	TIMESYNC_GPIO_NUM	12
static const char BCMATTACHDATA(rstr_time_sync_opt)[] = "time_sync_opt";
uint8
BCMATTACHFN(si_gci_time_sync_gpio_init)(si_t *sih)
{
	uint8  time_sync_gpio = TIMESYNC_GPIO_NUM;
	uint32 time_sync_opt;

	/* parse the device wake opt from nvram */
	/* decode what that means for specific chip */
	if (getvar(NULL, rstr_time_sync_opt) == NULL) {
		time_sync_opt = TIMESYNC_GPIO_NUM;
	} else {
		time_sync_opt = getintvar(NULL, rstr_time_sync_opt);
	}
	switch (CHIPID(sih->chip)) {
	case BCM4369_CHIP_GRPID:
	case BCM4347_CHIP_GRPID:
		time_sync_gpio = time_sync_opt & 0xff;
		si_gci_enable_gpio(sih, time_sync_gpio,
			1 << time_sync_gpio, 0x00);
		break;
	default:
		SI_ERROR(("time sync not supported for 0x%04x yet\n", CHIPID(sih->chip)));
		break;
	}

	return time_sync_gpio;
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
	si_gci_gpio_chipcontrol(sih, gci_gpio,
	                        (1 << GCI_GPIO_CHIPCTRL_ENAB_IN_BIT));

	si_gci_gpio_intmask(sih, gci_gpio, *wake_mask, *wake_mask);
	si_gci_gpio_wakemask(sih, gci_gpio, *wake_mask, *wake_mask);

	/* clear the existing status bits */
	*cur_status = si_gci_gpio_status(sih, gci_gpio,
	                                 GCI_GPIO_STS_CLEAR, GCI_GPIO_STS_CLEAR);

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
	pmu_chipcontrol2 |= si_pmu_wake_bit_offset(sih);
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
#endif // endif
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
	case BCM53573_CHIP_GRPID:
	case BCM4350_CHIP_ID:
		/* device_wake op 1:
		 * XXX: gpio 1, func sel 4,
		 * gcigpioctrl: input pin, exra gpio
		 * since GCI_GPIO_CHIPCTRL_ENAB_EXT_GPIO_BIT is used, gci gpio is same as GPIO num
		 * GCI GPIO 1,wakemask/intmask: any edge, both positive negative
		 * enable the wake mask, intmask in GCI top level
		 * enable the chip common to get the G/ECI interrupt
		 * enable the PMU ctrl to wake the chip on wakemask set
		 */
		if (device_wake_opt == 1) {
			gci_gpio = CC_GCI_GPIO_1;
			*wake_mask = (1 << GCI_GPIO_STS_VALUE_BIT) |
				(1 << GCI_GPIO_STS_POS_EDGE_BIT) |
				(1 << GCI_GPIO_STS_NEG_EDGE_BIT);
			si_gci_set_functionsel(sih, gci_gpio, CC4345_FNSEL_GCI0);
			si_enable_gpio_wake(sih, wake_mask, cur_status, gci_gpio,
				PMU_CC2_GCI2_WAKE | PMU_CC2_MASK_WL_DEV_WAKE,
				PMU_CC2_GCI2_WAKE | PMU_CC2_MASK_WL_DEV_WAKE);
			si_gci_gpio_chipcontrol(sih, 0,
					(1 << GCI_GPIO_CHIPCTRL_PULLDN_BIT));

			/* Enable wake on GciWake */
			si_gci_indirect(sih, 0,
				GCI_OFFSETOF(sih, gci_wakemask),
				(GCI_INTSTATUS_GPIOWAKE | GCI_INTSTATUS_GPIOINT),
				(GCI_INTSTATUS_GPIOWAKE | GCI_INTSTATUS_GPIOINT));

		} else {
			SI_ERROR(("0x%04x: don't know about device_wake_opt %d\n",
				CHIPID(sih->chip), device_wake_opt));
		}
		break;

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

void
si_gci_seci_init(si_t *sih)
{
	/* For WL-Coex (#ifdef WLCX_ATLAS) and BT-Coex (BFL4_BTCOEX_OVER_SECI) */
	if (CHIPID(sih->chip) == BCM7271_CHIP_ID) {
		/* 7271 Configuration
		 * BT Chip Pin 27 -gpio 7 - BT SECO <--> Wireless- SECI  WIFI_GPIO 9 -> gci gpio 6
		 * BT Chip Pin 20 -gpio 6 - BT SECI  <--> Wireless- SECO WIFI_GPIO 8   > gci gpio5
		 */
		/* bit 2: Enable Seci  bit 4 to 6: Seci Mode   */
		si_gci_direct(sih, GCI_OFFSETOF(sih, gci_corectrl), ALLONES_32,
			((GCI_MODE_SECI << GCI_CCTL_SMODE_OFFSET)
				| (1 << GCI_CCTL_SECIEN_OFFSET))); // 0x00c

		/* 7271 GCI chip Control register bit 20, enable bt_seci_on_gpio_mode
		 * 4365 uses  bit 19 enable bt_seci_on_sprom_mode instead
		 */
		si_gci_indirect(sih, 0,
			GCI_OFFSETOF(sih, gci_chipctrl), ALLONES_32, 0x0100000); //0x200

		/*  gpio 5 as output enabled and 6 as Input enabled */
		si_gci_indirect(sih, 1,
			GCI_OFFSETOF(sih, gci_gpioctl), ALLONES_32, 0x00010200); //0x044

		/* baudrate: 3Mbps escseq:0xdb, low baudrate, enable seci_tx/rx.
		 * 7271 54MHz xtal 20704 xtal is 48Mhz, 3M is the highest common under 4M
		 * With 3-4Mbps baudrate the seci delay is ~5us. Legacy coex programs have used 3M,
		 * newer gci progams use 4M.
		 */
		si_gci_direct(sih, GCI_OFFSETOF(sih, gci_secibauddiv), ALLONES_32, 0xFF); //0x1e0
		si_gci_direct(sih, GCI_OFFSETOF(sih, gci_baudadj), ALLONES_32, 0x11); //0x1f8
		si_gci_direct(sih, GCI_OFFSETOF(sih, gci_secifcr), ALLONES_32, 0x00); //0x1e4
		si_gci_direct(sih, GCI_OFFSETOF(sih, gci_secimcr), ALLONES_32, 0x80); //0x1ec
		si_gci_direct(sih, GCI_OFFSETOF(sih, gci_secilcr), ALLONES_32, 0x00); //0x1e8
		si_gci_direct(sih, GCI_OFFSETOF(sih, gci_seciuartescval), ALLONES_32, 0xDB); //0x1d0
	}
	else {
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
	}

	/* config GPIO pins 5/6 as SECI_IN/SECI_OUT */
	si_gci_indirect(sih, 0,
		GCI_OFFSETOF(sih, gci_seciin_ctrl), ALLONES_32, 0x161); //0x218
	si_gci_indirect(sih, 0,
		GCI_OFFSETOF(sih, gci_seciout_ctrl), ALLONES_32, 0x10051); //0x21c

	si_gci_direct(sih, GCI_OFFSETOF(sih, gci_seciout_txen_txbr), ALLONES_32, 0x01); //0x224

	/* WLAN rx offset assignment */
	/* WLCX: RX offset assignment from WLAN core to WLAN core (faked as BT TX) */
	si_gci_indirect(sih, 0,
		GCI_OFFSETOF(sih, gci_secif0rx_offset), ALLONES_32, 0x13121110); //0x1bc
	si_gci_indirect(sih, 1,
		GCI_OFFSETOF(sih, gci_secif0rx_offset), ALLONES_32, 0x17161514);
	si_gci_indirect(sih, 2,
		GCI_OFFSETOF(sih, gci_secif0rx_offset), ALLONES_32, 0x1b1a1918);

	/* first 12 nibbles configured for format-0 */
	/* note: we can only select 1st 12 nibbles of each IP for format_0 */
	si_gci_indirect(sih, 0,  GCI_OFFSETOF(sih, gci_seciusef0tx_reg), //0x1b4
	                ALLONES_32, 0xFFF); // first 12 nibbles

	si_gci_indirect(sih, 0,  GCI_OFFSETOF(sih, gci_secitx_datatag),
			ALLONES_32, 0x0F0); // gci_secitx_datatag(nibbles 4 to 7 tagged)
	si_gci_indirect(sih, 0,  GCI_OFFSETOF(sih, gci_secirx_datatag),
	                ALLONES_32, 0x0F0); // gci_secirx_datatag(nibbles 4 to 7 tagged)

	/* TX offset assignment (wlan to bt) */
	si_gci_indirect(sih, 0,
		GCI_OFFSETOF(sih, gci_secif0tx_offset), 0xFFFFFFFF, 0x76543210); //0x1b8
	si_gci_indirect(sih, 1,
		GCI_OFFSETOF(sih, gci_secif0tx_offset), 0xFFFFFFFF, 0x0000ba98);
	if (CHIPID(sih->chip) == BCM43602_CHIP_ID) {
		/* Request	BT side to update SECI information */
		si_gci_direct(sih, OFFSETOF(chipcregs_t, gci_seciauxtx),
			(SECI_AUX_TX_START | SECI_REFRESH_REQ),
			(SECI_AUX_TX_START | SECI_REFRESH_REQ));
		/* WLAN to update SECI information */
		si_gci_direct(sih, OFFSETOF(chipcregs_t, gci_corectrl),
			SECI_UPD_SECI, SECI_UPD_SECI);
	}

	// HW ECI bus directly driven from IP
	si_gci_direct(sih, GCI_OFFSETOF(sih, gci_control_0), ALLONES_32, 0x00000000);
	si_gci_direct(sih, GCI_OFFSETOF(sih, gci_control_1), ALLONES_32, 0x00000000);
} /* si_gci_seci_init */

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

void
BCMATTACHFN(si_gci_chipctrl_overrides)(osl_t *osh, si_t *sih, char *pvars)
{
	uint8 num_cc = 0;
	char gciccstr[16];
	const char *otp_val;
	uint32 gciccval = 0, cap1 = 0;
	int i = 0;

	BCM_REFERENCE(osh);
	BCM_REFERENCE(sih);

	/* because NFLASH and GCI clashes in 0xC00 */
	if ((CCREV(sih->ccrev) == 38) && ((sih->chipst & (1 << 4)) != 0)) {
		/* CC NFLASH exist, prohibit to manipulate gci register */
		ASSERT(0);
	}

	cap1 = si_corereg(sih, GCI_CORE_IDX(sih), GCI_OFFSETOF(sih, gci_corecaps1), 0, 0);
	num_cc = CC_GCI_NUMCHIPCTRLREGS(cap1);

	for (i = 0; i < num_cc; i++) {
		snprintf(gciccstr, sizeof(gciccstr), "gcr%d", i);

		if ((otp_val = getvar(NULL, gciccstr)) == NULL)
			continue;

		gciccval = (uint32) getintvar(pvars, gciccstr);
		si_gci_chipcontrol(sih, i, ~0, gciccval);
	}
}
#endif /* (!_CFE_ && !_CFEZ_) || CFG_WL */

/** write 'val' to the gci chip control register indexed by 'reg' */
uint32
si_gci_chipcontrol(si_t *sih, uint reg, uint32 mask, uint32 val)
{
	/* because NFLASH and GCI clashes in 0xC00 */
	if ((CCREV(sih->ccrev) == 38) && ((sih->chipst & (1 << 4)) != 0)) {
		/* CC NFLASH exist, prohibit to manipulate gci register */
		ASSERT(0);
		return ALLONES_32;
	}

	si_corereg(sih, GCI_CORE_IDX(sih), GCI_OFFSETOF(sih, gci_indirect_addr), ~0, reg);
	return si_corereg(sih, GCI_CORE_IDX(sih), GCI_OFFSETOF(sih, gci_chipctrl), mask, val);
}

/* Read the gci chip status register indexed by 'reg' */
uint32
si_gci_chipstatus(si_t *sih, uint reg)
{
	/* because NFLASH and GCI clashes in 0xC00 */
	if ((CCREV(sih->ccrev) == 38) && ((sih->chipst & (1 << 4)) != 0)) {
		/* CC NFLASH exist, prohibit to manipulate gci register */
		ASSERT(0);
		return ALLONES_32;
	}

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
		case BCM4357_CHIP_ID:
		case BCM4361_CHIP_ID:
			sii->chipnew = sih->chip; /* save it */
			sii->pub.chip = BCM4347_CHIP_ID; /* chip class */
		break;
		case BCM4377_CHIP_ID:
			sii->chipnew = sih->chip; /* save it */
			sii->pub.chip = BCM4369_CHIP_ID; /* chip class */
		break;
		case BCM43694_CHIP_ID:
			sii->chipnew = sih->chip; /* save it */
			sii->pub.chip = BCM43684_CHIP_ID; /* chip class */
		break;
		case BCM43465_CHIP_ID:
			sii->chipnew = sih->chip; /* save it */
			sii->pub.chip = BCM4366_CHIP_ID; /* chip class */
		break;
		case BCM43525_CHIP_ID:
			sii->chipnew = sih->chip; /* save it */
			sii->pub.chip = BCM4365_CHIP_ID; /* chip class */
		break;
		case BCM43460_CHIP_ID:
			sii->chipnew = sih->chip; /* save it */
			sii->pub.chip = BCM4360_CHIP_ID; /* chip class */
		break;
		case BCM43462_CHIP_ID:
			sii->chipnew = sih->chip; /* save it */
			sii->pub.chip = BCM43602_CHIP_ID; /* chip class */
		break;
		case BCM43692_CHIP_ID: /* also for BCM6705 */
		case BCM43693_CHIP_ID:
			sii->chipnew = sih->chip; /* save it */
			sii->pub.chip = BCM6710_CHIP_ID; /* chip class */
		break;
		default:
		break;
	}
} /* si_chipid_fixup */

#ifdef BCMULP
static void
si_check_boot_type(si_t *sih, osl_t *osh)
{
#ifdef _RTE_
	if (!si_onetimeinit) {
#endif // endif
	if (sih->pmurev >= 30) {
		boot_type = PMU_REG_NEW(sih, swscratch, 0, 0);
	} else {
		boot_type = CHIPC_REG(sih, flashdata, 0, 0);
	}

	SI_ERROR(("%s: boot_type: 0x%08x\n", __func__, boot_type));
#ifdef _RTE_
	}
#endif // endif
}
#endif /* BCMULP */

#ifdef BCM_BACKPLANE_TIMEOUT
uint32
si_clear_backplane_to_fast(void *sih, void *addr)
{
	si_t *_sih = DISCARD_QUAL(sih, si_t);

	if (CHIPTYPE(_sih->socitype) == SOCI_AI) {
		return ai_clear_backplane_to_fast(_sih, addr);
	}

	return 0;
}

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

/**
 * Allocate an si handle. This function may be called multiple times. This function is called by
 * both si_attach() and si_kattach().
 *
 * vars - pointer to a to-be created pointer area for "environment" variables. Some callers of this
 *        function set 'vars' to NULL.
 *
 * @param [in]  devid - e.g. a PCIe device id
 * @param [in]  regs  - virtual address of initial (chipcommon) core registers
 * @param [in]  sdh   - opaque pointer pointing to e.g. a Linux 'struct pci_dev'
 */
static si_info_t *
BCMATTACHFN(si_doattach)(si_info_t *sii, uint devid, osl_t *osh, volatile void *regs,
                       uint bustype, void *sdh, char **vars, uint *varsz)
{
	struct si_pub *sih = &sii->pub;
	uint32 w, savewin;
	chipcregs_t *cc;
	char *pvars = NULL;
	uint origidx;
#ifdef NVSRCX
	char *sromvars;
#endif // endif

	ASSERT(GOODREGS(regs));

	savewin = 0;

	sih->buscoreidx = BADIDX;
	sii->device_removed = FALSE;

	sii->curmap = regs;
	sii->sdh = sdh;
	sii->osh = osh;
	sii->second_bar0win = ~0x0;

#if defined(BCMDRIVER) && defined(LINUX_VERSION_CODE) && !defined(DONGLEBUILD) /* NIC \
	only */
	if (EMBEDDED_2x2AX_CORE(devid) || BCM6878_CHIP(devid)) {
		/* Prerequisite: the 'pseudo PCIe' config space code in the BSP has written the PCIe
		 * BAR0 register with the physical backplane address that the driver should use for
		 * this particular core.
		 */
		sih->enum_base = pci_resource_start((struct pci_dev*)sdh, 0);
		if (sih->enum_base == BCM47622_DEV_B_PHYS_ADDR) {
			sih->nvram_fname = "nvram2.txt"; /* each WLAN core has its own nvram file */
		}
		sih->enum_base_bp = SI_ENUM_BASE_BP_2x2AX; /* == 0x8400_0000 */
		SI_PRINT(("%s: using BAR0 value: enum_base=%x\n", __FUNCTION__, sih->enum_base));
	} else
#endif /* BCMDRIVER && LINUX_VERSION_CODE && !DONGLEBUILD */
		sih->enum_base = sih->enum_base_bp = si_enum_base(devid);

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

	/* check to see if we are a si core mimic'ing a pci core */
	if ((bustype == PCI_BUS) &&
	    ((OSL_PCI_READ_CONFIG(sii->osh, PCI_SPROM_CONTROL, sizeof(uint32)) == 0xffffffff) ||
	      (devid == EMBEDDED_2x2AX_ID) || (BCM6878_CHIP(devid)) ||
	      FALSE)) {
		SI_ERROR(("%s: incoming bus is PCI but it's a lie, switching to SI "
		          "devid:0x%x\n", __FUNCTION__, devid));
		bustype = SI_BUS;
	}

	/* find Chipcommon address */
	if (bustype == PCI_BUS) {
		savewin = OSL_PCI_READ_CONFIG(sii->osh, PCI_BAR0_WIN, sizeof(uint32));
		if (!GOODCOREADDR(savewin, SI_ENUM_BASE(sih)))
			savewin = SI_ENUM_BASE(sih);
		OSL_PCI_WRITE_CONFIG(sii->osh, PCI_BAR0_WIN, 4, SI_ENUM_BASE(sih));
		if (!regs)
			return NULL;
		cc = (chipcregs_t *)regs;
	} else {
		cc = (chipcregs_t *)REG_MAP(SI_ENUM_BASE(sih), SI_CORE_SIZE);
	}

	sih->bustype = bustype;
#ifndef DONGLEBUILD
	if (bustype == SI_BUS) { /* For NIC build with SI_BUS, e.g. 63178/47622 */
		sih->enum_base_va = (uintptr)regs;
	}
#endif /* DONGLEBUILD */

#ifdef BCMBUSTYPE
	if (bustype != BUSTYPE(bustype)) {
		SI_ERROR(("si_doattach: bus type %d does not match configured bus type %d\n",
			bustype, BUSTYPE(bustype)));
		return NULL;
	}
#endif // endif

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
	if (!cc) {
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
	if IS_47622A0_SLAVE_SLICE(devid, sih->chiprev, sih->enum_base) {
		bcm47622a0_war(sih);
	}
#endif /* BCAWLAN205004 */

	/* scan for cores */
	if (CHIPTYPE(sii->pub.socitype) == SOCI_SB) {
		SI_MSG(("Found chip type SB (0x%08x)\n", w));
		sb_scan(&sii->pub, regs, devid);
	} else if ((CHIPTYPE(sii->pub.socitype) == SOCI_AI) ||
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
	} else if (CHIPTYPE(sii->pub.socitype) == SOCI_UBUS) {
		SI_MSG(("Found chip type UBUS (0x%08x), chip id = 0x%4x\n", w, sih->chip));
		/* pass chipc address instead of original core base */
		ub_scan(&sii->pub, (void *)(uintptr)cc, devid);
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
	if (!si_buscore_setup(sii, cc, bustype, savewin, &origidx, regs)) {
		SI_ERROR(("si_doattach: si_buscore_setup failed\n"));
		goto exit;
	}

#ifdef BCMULP
	if (BCMULP_ENAB()) {
		si_check_boot_type(sih, osh);
		if (ulp_module_init(osh, sih) != BCME_OK) {
			ULP_ERR(("%s: err in ulp_module_init\n", __FUNCTION__));
			goto exit;
		}
	}
#endif /* BCMULP */

#if (!defined(_CFE_) && !defined(_CFEZ_)) || defined(CFG_WL)
	/* Set the clkdiv2 divisor bits (2:0) to 0x4 if srom is present */
	if (bustype == SI_BUS) {
		uint32 clkdiv2, sromprsnt, capabilities, srom_supported;
		capabilities =	R_REG(osh, &cc->capabilities);
		srom_supported = capabilities & SROM_SUPPORTED;
		if (srom_supported)
		{
			sromprsnt = R_REG(osh, &cc->sromcontrol);
			sromprsnt = sromprsnt & SROM_PRSNT_MASK;
			if (sromprsnt) {
				/* SROM clock come from backplane clock/div2. Must <= 1Mhz */
				clkdiv2 = (R_REG(osh, &cc->clkdiv2) & ~CLKD2_SROM);
				if (BCM43684_CHIP(sih->chip))
					clkdiv2 |= CLKD2_SROMDIV_256;
				else
					clkdiv2 |= CLKD2_SROMDIV_192;
				W_REG(osh, &cc->clkdiv2, clkdiv2);
			}
		}
	}

	if (bustype == PCI_BUS) {
#if !defined(BCMDONGLEHOST)
		if (((CHIPID(sih->chip) == BCM4360_CHIP_ID) ||
		     (CHIPID(sih->chip) == BCM43460_CHIP_ID) ||
		     (CHIPID(sih->chip) == BCM4352_CHIP_ID)) &&
		    (CHIPREV(sih->chiprev) <= 2)) {
			pcie_disable_TL_clk_gating(sii->pch);
			pcie_set_L1_entry_time(sii->pch, 0x40);
		}
#endif /* BCMDONGLEHOST */

#ifdef BCMQT
		/* Set OTPClkDiv to smaller value otherwise OTP always reads 0xFFFF.
		 * For real-chip we shouldn't set OTPClkDiv to 2 because 20/2 = 10 > 9Mhz
		 */
		{
			uint otpclkdiv = 0;

			if (CHIPID(sih->chip) == BCM43217_CHIP_ID) {
				otpclkdiv = 4;
			}

			if (otpclkdiv != 0) {
				uint clkdiv, savecore;
				savecore = si_coreidx(sih);
				si_setcore(sih, CC_CORE_ID, 0);

				clkdiv = R_REG(osh, &cc->clkdiv);
				clkdiv = (clkdiv & ~CLKD_OTP) | (otpclkdiv << CLKD_OTP_SHIFT);
				W_REG(osh, &cc->clkdiv, clkdiv);

				SI_ERROR(("%s: set clkdiv to 0x%x for QT\n", __FUNCTION__, clkdiv));
				si_setcoreidx(sih, savecore);
			}
		}
#endif /* BCMQT */
	}
#endif /* (!_CFE_ && !_CFEZ_) || CFG_WL */
#ifdef BCM_SDRBL
	/* 4360 rom bootloader in PCIE case, if the SDR is enabled, But preotection is
	 * not turned on, then we want to hold arm in reset.
	 * Bottomline: In sdrenable case, we allow arm to boot only when protection is
	 * turned on.
	 */
	if (CHIP_HOSTIF_PCIE(&(sii->pub))) {
		uint32 sflags = si_arm_sflags(&(sii->pub));

		/* If SDR is enabled but protection is not turned on
		* then we want to force arm to WFI.
		*/
		if ((sflags & (SISF_SDRENABLE | SISF_TCMPROT)) == SISF_SDRENABLE) {
			disable_arm_irq();
			while (1) {
				hnd_cpu_wait(sih);
			}
		}
	}
#endif /* BCM_SDRBL */
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
#endif // endif

	/* Init nvram from sprom/otp if they exist */
#if defined(STRICT_GCC_WARNINGS) && defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
#endif // endif

#ifdef DONGLEBUILD
#if	!defined(NVSRCX)
	/* Init nvram from sprom/otp if they exist and not inited */
	if (si_getkvars()) {
		*vars = si_getkvars();
		*varsz = si_getkvarsz();
	}
	else
#endif // endif
#endif /* DONGLEBUILD */
	{
#ifdef NVSRCX
	sromvars = srom_get_sromvars();
	if (sromvars == NULL) {
		if (srom_var_init(&sii->pub, BUSTYPE(bustype), (void *)regs,
				sii->osh, &sromvars, varsz)) {
			SI_ERROR(("si_doattach: srom_var_init failed: bad srom\n"));
			goto exit;
		}
	}
#else
	if (srom_var_init(&sii->pub, BUSTYPE(bustype), (void *)regs,
			sii->osh, vars, varsz)) {
		SI_ERROR(("si_doattach: srom_var_init failed: bad srom\n"));
		goto exit;
	}
#endif // endif
	}

#if defined(STRICT_GCC_WARNINGS) && defined(__GNUC__)
#pragma GCC diagnostic pop
#endif // endif
	pvars = vars ? *vars : NULL;

	si_nvram_process(sii, pvars);

	/* xtalfreq is required for programming open loop calibration support changes */
	sii->xtalfreq = getintvar(NULL, rstr_xtalfreq);
	/* === NVRAM, clock is ready === */
#else
	pvars = NULL;
	BCM_REFERENCE(pvars);
#endif /* !BCMDONGLEHOST */

#if !defined(BCMDONGLEHOST)
	if (HIB_EXT_WAKEUP_CAP(sih)) {
		/* Turn off PS1 mode by default now */
		sii->lhl_ps_mode = LHL_PS_MODE_0;
	} else
#endif // endif
	{
		sii->lhl_ps_mode = LHL_PS_MODE_0;
	}

	if (!si_onetimeinit) {
#if !defined(BCMDONGLEHOST)
		char *val;

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
#elif defined(__ARM_ARCH_7A__)
			hnd_tcam_bootloader_load(si_setcore(sih, SYSMEM_CORE_ID, 0), pvars);
#else
			hnd_tcam_bootloader_load(si_setcore(sih, SOCRAM_CORE_ID, 0), pvars);
#endif // endif
			si_setcoreidx(sih, origidx);
		}
#endif /* CONFIG_XIP && BCMTCAM */

		if (CCREV(sii->pub.ccrev) >= 20) {
			uint32 gpiopullup = 0, gpiopulldown = 0;
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
		}

		/* XXX Skip PMU initialization from the Dongle Host.
		 * Firmware will take care of it when it comes up.
		 */
#if !defined(BCMDONGLEHOST)
		/* PMU specific initializations */
		if (PMUCTL_ENAB(sih)) {
			uint32 xtalfreq, mode;
			si_pmu_init(sih, sii->osh);
			si_pmu_chip_init(sih, sii->osh);
			xtalfreq = getintvar(pvars, rstr_xtalfreq);
			/*
			 * workaround for chips that don't support external LPO, thus ALP clock
			 * can not be measured accurately:
			 */
			switch (CHIPID(sih->chip)) {
			CASE_BCM43602_CHIP:
			case BCM4365_CHIP_ID:
			case BCM4366_CHIP_ID:
			case BCM43664_CHIP_ID:
			case BCM43666_CHIP_ID:
				xtalfreq = 40000;
				break;
			case BCM7271_CHIP_ID:
				xtalfreq = 54000;
				break;
			CASE_BCM43684_CHIP:
				xtalfreq = 54000;
				break;
			CASE_EMBEDDED_2x2AX_CORE:
			CASE_BCM6710_CHIP:
			CASE_BCM6715_CHIP:
				xtalfreq = 50000;
				break;
			case BCM43570_CHIP_ID:
				if (xtalfreq != 0)
					break;
				mode = CST4350_IFC_MODE(sih->chipst);
				if ((mode == CST4350_IFC_MODE_USB20D) ||
				    (mode == CST4350_IFC_MODE_USB30D) ||
				    (mode == CST4350_IFC_MODE_USB30D_WL))
					xtalfreq = 40000;
				else {
					xtalfreq = 37400;
					if ((mode == CST4350_IFC_MODE_HSIC20D ||
					     mode == CST4350_IFC_MODE_HSIC30D) &&
					    /* HSIC sprom_present_strap=1:40 mHz xtal */
					    (CHIPREV(sih->chiprev) >= 3 ||
					     CHIPID(sih->chip) == BCM43570_CHIP_ID) &&
					    CST4350_PKG_USB_40M(sih->chipst) &&
					    CST4350_PKG_USB(sih->chipst)) {
						xtalfreq = 40000;
					}
				}
				break;
			default:
				break;
			}

			/* If xtalfreq var not available, try to measure it */
			if (xtalfreq == 0)
				xtalfreq = si_pmu_measure_alpclk(sih, sii->osh);

			sii->xtalfreq = xtalfreq;
			si_pmu_pll_init(sih, sii->osh, xtalfreq);

			/* configure default spurmode  */
			sii->spurmode = getintvar(pvars, rstr_spurconfig) & 0xf;

#if defined(SAVERESTORE)
			/* Only needs to be done once.
			 * Needs this before si_pmu_res_init() to use sr_isenab()
			 */
			if (SR_ENAB()) {
				sr_save_restore_init(sih);
			}
#endif // endif

			if (SRPWR_CAP(sih)) {
				si_srpwr_request(sih, SRPWR_DMN_ALL_MASK, SRPWR_DMN_ALL_MASK);
			}

			si_pmu_res_init(sih, sii->osh);
			si_pmu_swreg_init(sih, sii->osh);
#ifdef BCMULP
			/* for pmu, register for ulp */
			if (BCMULP_ENAB())
			{
				int err = BCME_OK;
				err = si_pmu_ulp_register((si_t *)sii);
				 if (err != BCME_OK)
				 {
					ULP_DBG(("si_doattach: si_pmu_ulp_register failed\n"));
					goto exit;
				 }
			}
#endif /* BCMULP */
		}
#endif /* !defined(BCMDONGLEHOST) */
#ifdef _RTE_
		si_onetimeinit = TRUE;
#endif // endif
	}

#if !defined(BCMDONGLEHOST) && ((!defined(_CFE_) && !defined(_CFEZ_)) || \
	defined(CFG_WL))
	si_lowpwr_opt(sih);
#ifdef WLLED
	/* setup the GPIO based LED powersave register */
	if (CCREV(sii->pub.ccrev) >= 16) {
		if ((w = getintvar(pvars, rstr_leddc)) == 0)
			w = DEFAULT_GPIOTIMERVAL;
		si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, gpiotimerval), ~0, w);
	}
#endif // endif

	if (PCIE(sii)) {
		ASSERT(sii->pch != NULL);
		pcicore_attach(sii->pch, pvars, SI_DOATTACH);
	}

	if (BCM4347_CHIP(sih->chip)) {
		/* Clear SFlash clock request */
		CHIPC_REG(sih, clk_ctl_st, CCS_SFLASH_CLKREQ, 0);
	}

	/* Set up LED and WPS pins */
	if (BCM4350_CHIP(sih->chip) && CHIP_HOSTIF_USB(sih)) {
		si_gci_set_functionsel(sih, CC4350_PIN_GPIO_13, CC4350_FNSEL_SAMEASPIN);
		si_gci_set_functionsel(sih, CC4350_PIN_GPIO_14, CC4350_FNSEL_SAMEASPIN);
		si_gci_set_functionsel(sih, CC4350_PIN_GPIO_15, CC4350_FNSEL_SAMEASPIN);
	}

	/* Set up GPIO radio disable */
	if (sii->pub.boardflags & BFL_AIRLINEMODE) {
		if (BCM4350_CHIP(sih->chip)) {
			si_gci_set_functionsel(sih, CC4350_PIN_GPIO_01, CC4350_FNSEL_MISC0);
		}
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
#endif // endif

	/* configure default pinmux enables for the chip */
	if (getvar(pvars, rstr_muxenab) != NULL) {
		w = getintvar(pvars, rstr_muxenab);
		si_muxenab((si_t *)sii, w);
	}

	/* configure default swd enables for the chip */
	if (getvar(pvars, rstr_swdenab) != NULL) {
		w = getintvar(pvars, rstr_swdenab);
		si_swdenable((si_t *)sii, w);
	}

	sii->device_wake_opt = CC_GCI_GPIO_INVALID;
	if (BCM53573_CHIP(sih->chip)) {
		/* FEM Ctrl WAR for 53573 - Enable the rf_sw control lines drive the pad mux
		   There are 4 bits for every control line and the lsb needs to be made 1 to
		   enable the o/p
		 */
		si_gci_chipcontrol(sih, CC_GCI_CHIPCTRL_04, ~0, GCI_FEM_CTRL_WAR);
		si_gci_chipcontrol(sih, CC_GCI_CHIPCTRL_05, ~0, GCI_FEM_CTRL_WAR);
	}
#endif /* !BCMDONGLEHOST && ((!_CFE_ && !_CFEZ_) || CFG_WL) */
	/* clear any previous epidiag-induced target abort */
	ASSERT(!si_taclear(sih, FALSE));

#if !defined(BCMDONGLEHOST) && ((!defined(_CFE_) && !defined(_CFEZ_)) || \
	defined(CFG_WL))
	if (BCM4350_CHIP(sih->chip)) {
#ifndef BCM4350_FPGA
		si_gci_chipctrl_overrides(osh, sih, pvars);
#endif // endif
	}
#endif /* !BCMDONGLEHOST && ((!_CFE_ && !_CFEZ_) || CFG_WL) */

#if defined(BCMPMU_STATS) && !defined(BCMPMU_STATS_DISABLED)
	si_pmustatstimer_init(sih);
#endif /* BCMPMU_STATS */

#ifdef BOOTLOADER_CONSOLE_OUTPUT
	/* Enable console prints */
	si_muxenab(sii, 3);
#endif // endif

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

	if (BUSTYPE(sih->bustype) == SI_BUS)
		for (idx = 0; idx < SI_MAXCORES; idx++)
			if (cores_info->regs[idx]) {
				REG_UNMAP(cores_info->regs[idx]);
				cores_info->regs[idx] = NULL;
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

	if (CHIPTYPE(sih->socitype) == SOCI_SB)
		return sb_intflag(sih);
	else if ((CHIPTYPE(sih->socitype) == SOCI_AI) ||
		(CHIPTYPE(sih->socitype) == SOCI_DVTBUS) ||
		(CHIPTYPE(sih->socitype) == SOCI_NAI))
		return R_REG(sii->osh, ((uint32 *)(uintptr)
			    (sii->oob_router + OOB_STATUSA)));
	else {
		ASSERT(0);
		return 0;
	}
}

uint
si_flag(si_t *sih)
{
	if (CHIPTYPE(sih->socitype) == SOCI_SB)
		return sb_flag(sih);
	else if ((CHIPTYPE(sih->socitype) == SOCI_AI) ||
		(CHIPTYPE(sih->socitype) == SOCI_DVTBUS) ||
		(CHIPTYPE(sih->socitype) == SOCI_NAI))
		return ai_flag(sih);
	else if (CHIPTYPE(sih->socitype) == SOCI_UBUS)
		return ub_flag(sih);
	else {
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
	if (CHIPTYPE(sih->socitype) == SOCI_SB)
		sb_setint(sih, siflag);
	else if ((CHIPTYPE(sih->socitype) == SOCI_AI) ||
		(CHIPTYPE(sih->socitype) == SOCI_DVTBUS) ||
		(CHIPTYPE(sih->socitype) == SOCI_NAI))
		ai_setint(sih, siflag);
	else if (CHIPTYPE(sih->socitype) == SOCI_UBUS)
		ub_setint(sih, siflag);
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
	if (CHIPTYPE(sih->socitype) == SOCI_SB)
		return sb_corevendor(sih);
		else if ((CHIPTYPE(sih->socitype) == SOCI_AI) ||
		(CHIPTYPE(sih->socitype) == SOCI_DVTBUS) ||
		(CHIPTYPE(sih->socitype) == SOCI_NAI))
		return ai_corevendor(sih);
	else if (CHIPTYPE(sih->socitype) == SOCI_UBUS)
		return ub_corevendor(sih);
	else {
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
	if (CHIPTYPE(sih->socitype) == SOCI_SB)
		return sb_corerev(sih);
		else if ((CHIPTYPE(sih->socitype) == SOCI_AI) ||
		(CHIPTYPE(sih->socitype) == SOCI_DVTBUS) ||
		(CHIPTYPE(sih->socitype) == SOCI_NAI))
		return ai_corerev(sih);
	else if (CHIPTYPE(sih->socitype) == SOCI_UBUS)
		return ub_corerev(sih);
	else {
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
	uint found = 0;

	found = si_numcoreunits(sih, D11_CORE_ID);

#if defined(WLRSDB) && defined(WLRSDB_DISABLED)
	/* If RSDB functionality is compiled out,
	 * then ignore any D11 cores beyond the first
	 * Used in norsdb dongle build variants for rsdb chip.
	 */
	found = 1;
#endif /* defined(WLRSDB) && !defined(WLRSDB_DISABLED) */

	return found;
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
	if (!GOODIDX(idx))
		return (NULL);

	if (CHIPTYPE(sih->socitype) == SOCI_SB)
		return sb_setcoreidx(sih, idx);
	else if ((CHIPTYPE(sih->socitype) == SOCI_AI) ||
		(CHIPTYPE(sih->socitype) == SOCI_DVTBUS) ||
		(CHIPTYPE(sih->socitype) == SOCI_NAI))
		return ai_setcoreidx(sih, idx);
	else if (CHIPTYPE(sih->socitype) == SOCI_UBUS)
		return ub_setcoreidx(sih, idx);
	else {
		ASSERT(0);
		return NULL;
	}
}

volatile void *
si_setcoreidx(si_t *sih, uint coreidx)
{
	if (CHIPTYPE(sih->socitype) == SOCI_SB)
		return sb_setcoreidx(sih, coreidx);
	else if ((CHIPTYPE(sih->socitype) == SOCI_AI) ||
		(CHIPTYPE(sih->socitype) == SOCI_DVTBUS) ||
		(CHIPTYPE(sih->socitype) == SOCI_NAI))
		return ai_setcoreidx(sih, coreidx);
	else if (CHIPTYPE(sih->socitype) == SOCI_UBUS)
		return ub_setcoreidx(sih, coreidx);
	else {
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
	if (CHIPTYPE(sih->socitype) == SOCI_SB)
		return sb_numaddrspaces(sih);
	else if ((CHIPTYPE(sih->socitype) == SOCI_AI) ||
		(CHIPTYPE(sih->socitype) == SOCI_DVTBUS) ||
		(CHIPTYPE(sih->socitype) == SOCI_NAI))
		return ai_numaddrspaces(sih);
	else if (CHIPTYPE(sih->socitype) == SOCI_UBUS)
		return ub_numaddrspaces(sih);
	else {
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
	if (CHIPTYPE(sih->socitype) == SOCI_SB)
		return sb_addrspace(sih, baidx);
	else if ((CHIPTYPE(sih->socitype) == SOCI_AI) ||
		(CHIPTYPE(sih->socitype) == SOCI_DVTBUS) ||
		(CHIPTYPE(sih->socitype) == SOCI_NAI))
		return ai_addrspace(sih, spidx, baidx);
	else if (CHIPTYPE(sih->socitype) == SOCI_UBUS)
		return ub_addrspace(sih, baidx);
	else {
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
BCMATTACHFN(si_addrspacesize)(si_t *sih, uint spidx, uint baidx)
{
	if (CHIPTYPE(sih->socitype) == SOCI_SB)
		return sb_addrspacesize(sih, baidx);
	else if ((CHIPTYPE(sih->socitype) == SOCI_AI) ||
		(CHIPTYPE(sih->socitype) == SOCI_DVTBUS) ||
		(CHIPTYPE(sih->socitype) == SOCI_NAI))
		return ai_addrspacesize(sih, spidx, baidx);
	else if (CHIPTYPE(sih->socitype) == SOCI_UBUS)
		return ub_addrspacesize(sih, baidx);
	else {
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
	if (CHIPTYPE(sih->socitype) == SOCI_SB)
		return sb_core_cflags(sih, mask, val);
	else if ((CHIPTYPE(sih->socitype) == SOCI_AI) ||
		(CHIPTYPE(sih->socitype) == SOCI_DVTBUS) ||
		(CHIPTYPE(sih->socitype) == SOCI_NAI))
		return ai_core_cflags(sih, mask, val);
	else if (CHIPTYPE(sih->socitype) == SOCI_UBUS)
		return ub_core_cflags(sih, mask, val);
	else {
		ASSERT(0);
		return 0;
	}
}

void
si_core_cflags_wo(si_t *sih, uint32 mask, uint32 val)
{
	if (CHIPTYPE(sih->socitype) == SOCI_SB)
		sb_core_cflags_wo(sih, mask, val);
	else if ((CHIPTYPE(sih->socitype) == SOCI_AI) ||
		(CHIPTYPE(sih->socitype) == SOCI_DVTBUS) ||
		(CHIPTYPE(sih->socitype) == SOCI_NAI))
		ai_core_cflags_wo(sih, mask, val);
	else if (CHIPTYPE(sih->socitype) == SOCI_UBUS)
		ub_core_cflags_wo(sih, mask, val);
	else
		ASSERT(0);
}

uint32
si_core_sflags(si_t *sih, uint32 mask, uint32 val)
{
	if (CHIPTYPE(sih->socitype) == SOCI_SB)
		return sb_core_sflags(sih, mask, val);
	else if ((CHIPTYPE(sih->socitype) == SOCI_AI) ||
		(CHIPTYPE(sih->socitype) == SOCI_DVTBUS) ||
		(CHIPTYPE(sih->socitype) == SOCI_NAI))
		return ai_core_sflags(sih, mask, val);
	else if (CHIPTYPE(sih->socitype) == SOCI_UBUS)
		return ub_core_sflags(sih, mask, val);
	else {
		ASSERT(0);
		return 0;
	}
}

void
si_commit(si_t *sih)
{
	if (CHIPTYPE(sih->socitype) == SOCI_SB)
		sb_commit(sih);
	else if ((CHIPTYPE(sih->socitype) == SOCI_AI) ||
		(CHIPTYPE(sih->socitype) == SOCI_DVTBUS) ||
		(CHIPTYPE(sih->socitype) == SOCI_NAI))
		;
	else if (CHIPTYPE(sih->socitype) == SOCI_UBUS)
		;
	else {
		ASSERT(0);
	}
}

bool
si_iscoreup(si_t *sih)
{
	if (CHIPTYPE(sih->socitype) == SOCI_SB)
		return sb_iscoreup(sih);
	else if ((CHIPTYPE(sih->socitype) == SOCI_AI) ||
		(CHIPTYPE(sih->socitype) == SOCI_DVTBUS) ||
		(CHIPTYPE(sih->socitype) == SOCI_NAI))
		return ai_iscoreup(sih);
	else if (CHIPTYPE(sih->socitype) == SOCI_UBUS)
		return ub_iscoreup(sih);
	else {
		ASSERT(0);
		return FALSE;
	}
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

/* si_backplane_access is used to read full backplane address from host for PCIE FD
 * it uses secondary bar-0 window which lies at an offset of 16K from primary bar-0
 * Provides support for read/write of 1/2/4 bytes of backplane address
 * Can be used to read/write
 *	1. core regs
 *	2. Wrapper regs
 *	3. memory
 *	4. BT area
 * For accessing any 32 bit backplane address, [31 : 12] of backplane should be given in "region"
 * [11 : 0] should be the "regoff"
 * for reading  4 bytes from reg 0x200 of d11 core use it like below
 * : si_backplane_access(sih, 0x18001000, 0x200, 4, 0, TRUE)
 */
static int si_backplane_addr_sane(uint addr, uint size)
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

/** read or write to a given physical backplane address, SI_BUS variant */
static uint
si_backplane_access_si(si_t *sih, uint addr, uint size, uint *val, bool read)
{
	uintptr va; /** virtual address */

#ifdef DONGLEBUILD /* For dongle builds, virtual are the same as physical addresses */
	va = addr;
#else
	if (addr != 0) {
		addr -= SI_ENUM_BASE_BP(sih); /* becomes offset relative to cc core */
	}

	va = sih->enum_base_va;
	va += addr;
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

	return (BCME_OK);
} /* si_backplane_access_si */

/** read or write to a given physical backplane address, PCI_BUS variant */
static uint
si_backplane_access_pci(si_info_t *sii, uint addr, uint size, uint *val, bool read)
{
	volatile uint32 *r = NULL;
	uint32 region = 0;
	/* Split adrr into region and address offset */
	region = (addr & (0xFFFFF << 12));
	addr = addr & 0xFFF;

	/* check for address and size sanity */
	if (si_backplane_addr_sane(addr, size) != BCME_OK)
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
	r = (volatile uint32 *)((volatile char *)sii->curmap + PCI_SECOND_BAR0_OFFSET + addr);

	SI_VMSG(("si curmap %p  region %x regaddr %x effective addr %p READ %d\n",
		(volatile char*)sii->curmap, region, addr, r, read));

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
 * @param[in] addr   Address on the backplane, from the backplane's point of view.
 */
uint
si_backplane_access(si_t *sih, uint addr, uint size, uint *val, bool read)
{
	si_info_t *sii = SI_INFO(sih);

	if (BUSTYPE(sih->bustype) == SI_BUS) {
		ASSERT(CHIPTYPE(sii->pub.socitype) == SOCI_AI);
		return si_backplane_access_si(sih, addr, size, val, read);
	} else if (BUSTYPE(sih->bustype) == PCI_BUS) {
		return si_backplane_access_pci(sii, addr, size, val, read);
	} else {
		SI_ERROR(("Valid only for si and pcie bus\n"));
	}

	return BCME_ERROR;
}

uint
si_corereg(si_t *sih, uint coreidx, uint regoff, uint mask, uint val)
{
	if (CHIPTYPE(sih->socitype) == SOCI_SB)
		return sb_corereg(sih, coreidx, regoff, mask, val);
	else if ((CHIPTYPE(sih->socitype) == SOCI_AI) ||
		(CHIPTYPE(sih->socitype) == SOCI_DVTBUS) ||
		(CHIPTYPE(sih->socitype) == SOCI_NAI))
		return ai_corereg(sih, coreidx, regoff, mask, val);
	else if (CHIPTYPE(sih->socitype) == SOCI_UBUS)
		return ub_corereg(sih, coreidx, regoff, mask, val);
	else {
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
		pmustatus_offset = AOB_ENAB(sih) ? OFFSETOF(pmuregs_t, pmustatus) :
			OFFSETOF(chipcregs_t, pmustatus);

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
	if (CHIPTYPE(sih->socitype) == SOCI_SB)
		return sb_corereg_addr(sih, coreidx, regoff);
	else if ((CHIPTYPE(sih->socitype) == SOCI_AI) ||
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
	if (CHIPTYPE(sih->socitype) == SOCI_SB)
		sb_core_disable(sih, bits);
	else if ((CHIPTYPE(sih->socitype) == SOCI_AI) ||
		(CHIPTYPE(sih->socitype) == SOCI_DVTBUS) ||
		(CHIPTYPE(sih->socitype) == SOCI_NAI))
		ai_core_disable(sih, bits);
	else if (CHIPTYPE(sih->socitype) == SOCI_UBUS)
		ub_core_disable(sih, bits);
}

void
si_core_reset(si_t *sih, uint32 bits, uint32 resetbits)
{
	if (CHIPTYPE(sih->socitype) == SOCI_SB)
		sb_core_reset(sih, bits, resetbits);
	else if ((CHIPTYPE(sih->socitype) == SOCI_AI) ||
		(CHIPTYPE(sih->socitype) == SOCI_DVTBUS) ||
		(CHIPTYPE(sih->socitype) == SOCI_NAI))
		ai_core_reset(sih, bits, resetbits);
	else if (CHIPTYPE(sih->socitype) == SOCI_UBUS)
		ub_core_reset(sih, bits, resetbits);
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
	uint32 addr = 0x0;

	if (!((CHIPTYPE(sih->socitype) == SOCI_AI) ||
		(CHIPTYPE(sih->socitype) == SOCI_DVTBUS) ||
		(CHIPTYPE(sih->socitype) == SOCI_NAI)))
		goto done;

	si_setcore(sih, core_id, coreunit);

	addr = ai_addrspace(sih, spidx, baidx);

	si_setcoreidx(sih, origidx);

done:
	return addr;
}

uint32
si_get_d11_slaveport_addr(si_t *sih, uint spidx, uint baidx, uint coreunit)
{
	si_info_t *sii = SI_INFO(sih);
	uint origidx = sii->curidx;
	uint32 addr = 0x0;

	if (!((CHIPTYPE(sih->socitype) == SOCI_AI) ||
		(CHIPTYPE(sih->socitype) == SOCI_DVTBUS) ||
		(CHIPTYPE(sih->socitype) == SOCI_NAI)))
		goto done;

	si_setcore(sih, D11_CORE_ID, coreunit);

	addr = ai_addrspace(sih, spidx, baidx);

	si_setcoreidx(sih, origidx);

done:
	return addr;
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

	switch (CHIPID(sih->chip)) {
	CASE_BCM43602_CHIP:
		hosti = CHIP_HOSTIF_PCIEMODE;
		break;
	case BCM4360_CHIP_ID:
		/* chippkg bit-0 == 0 is PCIE only pkgs
		 * chippkg bit-0 == 1 has both PCIE and USB cores enabled
		 */
		if ((sih->chippkg & 0x1) && (sih->chipst & CST4360_MODE_USB))
			hosti = CHIP_HOSTIF_USBMODE;
		else
			hosti = CHIP_HOSTIF_PCIEMODE;

		break;
	case BCM53573_CHIP_GRPID:
		if (CST4349_CHIPMODE_SDIOD(sih->chipst))
			hosti = CHIP_HOSTIF_SDIOMODE;
		else if (CST4349_CHIPMODE_PCIE(sih->chipst))
			hosti = CHIP_HOSTIF_PCIEMODE;
		break;
	case BCM4347_CHIP_GRPID:
		 if (CST4347_CHIPMODE_SDIOD(sih->chipst))
			 hosti = CHIP_HOSTIF_SDIOMODE;
		 else if (CST4347_CHIPMODE_PCIE(sih->chipst))
			 hosti = CHIP_HOSTIF_PCIEMODE;
		 break;
	case BCM4369_CHIP_GRPID:
		 if (CST4369_CHIPMODE_SDIOD(sih->chipst))
			 hosti = CHIP_HOSTIF_SDIOMODE;
		 else if (CST4369_CHIPMODE_PCIE(sih->chipst))
			 hosti = CHIP_HOSTIF_PCIEMODE;
		 break;

	case BCM43570_CHIP_ID:
		if (CST4350_CHIPMODE_USB20D(sih->chipst) ||
		    CST4350_CHIPMODE_HSIC20D(sih->chipst) ||
		    CST4350_CHIPMODE_USB30D(sih->chipst) ||
		    CST4350_CHIPMODE_USB30D_WL(sih->chipst) ||
		    CST4350_CHIPMODE_HSIC30D(sih->chipst))
			hosti = CHIP_HOSTIF_USBMODE;
		else if (CST4350_CHIPMODE_SDIOD(sih->chipst))
			hosti = CHIP_HOSTIF_SDIOMODE;
		else if (CST4350_CHIPMODE_PCIE(sih->chipst))
			hosti = CHIP_HOSTIF_PCIEMODE;
		break;

	default:
		break;
	}

	return hosti;
} /* si_chip_hostif */

#if !defined(BCMDONGLEHOST)
uint32
BCMINITFN(si_clock)(si_t *sih)
{
	si_info_t *sii = SI_INFO(sih);
	chipcregs_t *cc;
	uint32 n, m;
	uint idx;
	uint32 pll_type, rate;
	uint intr_val = 0;

	if (BCM4707_CHIP(CHIPID(sih->chip))) {
		if (sih->chippkg == BCM4709_PKG_ID) {
			return NS_SI_CLOCK;
		} else
			return NS_SLOW_SI_CLOCK;
	}

	INTR_OFF(sii, intr_val);
	if (PMUCTL_ENAB(sih)) {
		rate = si_pmu_si_clock(sih, sii->osh);
		goto exit;
	}

	idx = sii->curidx;
	cc = (chipcregs_t *)si_setcore(sih, CC_CORE_ID, 0);
	ASSERT(cc != NULL);

	n = R_REG(sii->osh, &cc->clockcontrol_n);
	pll_type = sih->cccaps & CC_CAP_PLL_MASK;
	if (pll_type == PLL_TYPE6)
		m = R_REG(sii->osh, &cc->clockcontrol_m3);
	else if (pll_type == PLL_TYPE3)
		m = R_REG(sii->osh, &cc->clockcontrol_m2);
	else
		m = R_REG(sii->osh, &cc->clockcontrol_sb);

	/* calculate rate */
	rate = si_clock_rate(pll_type, n, m);

	if (pll_type == PLL_TYPE3)
		rate = rate / 2;

	/* switch back to previous core */
	si_setcoreidx(sih, idx);
exit:
	INTR_RESTORE(sii, intr_val);

	return rate;
} /* si_clock */

/** returns value in [Hz] units */
static uint32
BCMINITFN(si_ns_alp_clock)(si_t *sih)
{
	osl_t *osh = si_osh(sih);
	uint32 *genpll_base;
	uint32 val;
	uint32 pdiv, ndiv_int, mdiv, clkrate;

	/* reg map for genpll base address */
	genpll_base = (uint32 *)REG_MAP(SI_ENUM_BASE(sih) | (0x1800C140 & 0x00FFFFFF), 4096);

	/* get divider integer from the cru_genpll_control5 */
	val = R_REG(osh, (genpll_base + 0x5));
	ndiv_int = (val >> 20) & 0x3ff;
	if (ndiv_int == 0)
		ndiv_int = 1 << 10;

	/* get pdiv from the cru_genpll_control6 */
	val = R_REG(osh, (genpll_base + 0x6));
	pdiv = (val >> 24) & 0x7;
	if (pdiv == 0)
		pdiv = 1 << 3;

	/* get mdiv from the cru_genpll_control7 */
	val = R_REG(osh, (genpll_base + 0x7));
	mdiv = val & 0xff;
	if (mdiv == 0)
		mdiv = 1 << 8;

	/* caculate clock rate based on 25MHz reference clock */
	clkrate = (25000000 / (pdiv * mdiv)) * ndiv_int;

	/* round to the nearest Hz */
	clkrate = ((clkrate + 500000) / 1000000) * 1000000;

	/* reg unmap */
	REG_UNMAP((void *)genpll_base);

	return clkrate;
}

/** returns value in [Hz] units */
uint32
BCMINITFN(si_alp_clock)(si_t *sih)
{
	if (PMUCTL_ENAB(sih))
		return si_pmu_alp_clock(sih, si_osh(sih));
	else if (BCM4707_CHIP(CHIPID(sih->chip))) {
		return si_ns_alp_clock(sih);
	}
	else if (BCM53573_CHIP(CHIPID(sih->chip))) {
		return ALP_CLOCK_53573;
	}

	return ALP_CLOCK;
}

/** returns value in [Hz] units */
uint32
BCMINITFN(si_ilp_clock)(si_t *sih)
{
	if (PMUCTL_ENAB(sih))
		return si_pmu_ilp_clock(sih, si_osh(sih));

	return ILP_CLOCK;
}
#endif /* !defined(BCMDONGLEHOST) */

/** set chip watchdog reset timer to fire in 'ticks' */
void
si_watchdog(si_t *sih, uint ticks)
{
	uint nb, maxt;
	uint pmu_wdt = 1;

	if (BCM53573_CHIP(sih->chip)) {
		/* 53573 B0/B1 pmu_wdt HW bug.  Instead to use cc_wdt */
		pmu_wdt = 0;
	}

	if (BCM43684_CHIP(sih->chip)) {
		/* Resetting AVS requires the use of the CC watchdog */
		pmu_wdt = 0;
	}

	if (PMUCTL_ENAB(sih) && pmu_wdt) {
		nb = (CCREV(sih->ccrev) < 26) ? 16 : ((CCREV(sih->ccrev) >= 37) ? 32 : 24);
		/* The mips compiler uses the sllv instruction,
		 * so we specially handle the 32-bit case.
		 */
		if (nb == 32)
			maxt = 0xffffffff;
		else
			maxt = ((1 << nb) - 1);

		if (ticks == 1)
			ticks = 2;
		else if (ticks > maxt)
			ticks = maxt;

		pmu_corereg(sih, SI_CC_IDX, pmuwatchdog, ~0, ticks);
	} else {
#if !defined(BCMDONGLEHOST)
		if (!BCM4707_CHIP(CHIPID(sih->chip)) && !BCM43684_CHIP(sih->chip) &&
		    !EMBEDDED_2x2AX_CORE(sih->chip) &&
		    !BCM6878_CHIP(sih->chip) &&
		    TRUE) {
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

bool
si_taclear(si_t *sih, bool details)
{
#if defined(BCMDBG_ERR) || defined(BCMASSERT_SUPPORT)
	if (CHIPTYPE(sih->socitype) == SOCI_SB)
		return sb_taclear(sih, details);
	else if ((CHIPTYPE(sih->socitype) == SOCI_AI) ||
		(CHIPTYPE(sih->socitype) == SOCI_DVTBUS) ||
		(CHIPTYPE(sih->socitype) == SOCI_NAI))
		return FALSE;
	else if (CHIPTYPE(sih->socitype) == SOCI_UBUS)
		return FALSE;
	else {
		ASSERT(0);
		return FALSE;
	}
#else
	return FALSE;
#endif // endif
}

#if !defined(BCMDONGLEHOST)
/**
 * Map sb core id to pci device id.
 *
 * XXX These function should be compiled in conditionally when driver builds
 * include files such as bcmjtag.c that make calls to this function. Also
 * hndpci.c is the ideal place to house this function but putting it there
 * would require other files such as hndmips.c to be built in as well (some
 * functions in hndpci.c make calls into hndmips.c).
 */
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
	if (CHIPTYPE(sih->socitype) == SOCI_SB)
		sb_dumpregs(sih, b);
	else if ((CHIPTYPE(sih->socitype) == SOCI_AI) ||
		(CHIPTYPE(sih->socitype) == SOCI_DVTBUS) ||
		(CHIPTYPE(sih->socitype) == SOCI_NAI))
		ai_dumpregs(sih, b);
	else if (CHIPTYPE(sih->socitype) == SOCI_UBUS)
		ub_dumpregs(sih, b);
	else
		ASSERT(0);

	si_setcoreidx(sih, origidx);
	INTR_RESTORE(sii, intr_val);
}
#endif // endif
#endif /* !defined(BCMDONGLEHOST) */

#ifdef BCMDBG
void
si_view(si_t *sih, bool verbose)
{
	if (CHIPTYPE(sih->socitype) == SOCI_SB)
		sb_view(sih, verbose);
	else if ((CHIPTYPE(sih->socitype) == SOCI_AI) ||
		(CHIPTYPE(sih->socitype) == SOCI_DVTBUS) ||
		(CHIPTYPE(sih->socitype) == SOCI_NAI))
		ai_view(sih, verbose);
	else if (CHIPTYPE(sih->socitype) == SOCI_UBUS)
		ub_view(sih, verbose);
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
		(CHIPTYPE(sih->socitype) == SOCI_NAI))
		ai_viewall(sih, verbose);
	else {
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

/** return the slow clock source - LPO, XTAL, or PCI */
static uint
si_slowclk_src(si_info_t *sii)
{
	chipcregs_t *cc;

	ASSERT(SI_FAST(sii) || si_coreid(&sii->pub) == CC_CORE_ID);

	if (CCREV(sii->pub.ccrev) < 6) {
		if ((BUSTYPE(sii->pub.bustype) == PCI_BUS) &&
		    (OSL_PCI_READ_CONFIG(sii->osh, PCI_GPIO_OUT, sizeof(uint32)) &
		     PCI_CFG_GPIO_SCS))
			return (SCC_SS_PCI);
		else
			return (SCC_SS_XTAL);
	} else if (CCREV(sii->pub.ccrev) < 10) {
		cc = (chipcregs_t *)si_setcoreidx(&sii->pub, sii->curidx);
		ASSERT(cc);
		return (R_REG(sii->osh, &cc->slow_clk_ctl) & SCC_SS_MASK);
	} else	/* Insta-clock */
		return (SCC_SS_XTAL);
}

/** return the ILP (slowclock) min or max frequency */
static uint
si_slowclk_freq(si_info_t *sii, bool max_freq, chipcregs_t *cc)
{
	uint32 slowclk;
	uint div;

	ASSERT(SI_FAST(sii) || si_coreid(&sii->pub) == CC_CORE_ID);

	/* shouldn't be here unless we've established the chip has dynamic clk control */
	ASSERT(R_REG(sii->osh, &cc->capabilities) & CC_CAP_PWR_CTL);

	slowclk = si_slowclk_src(sii);
	if (CCREV(sii->pub.ccrev) < 6) {
		if (slowclk == SCC_SS_PCI)
			return (max_freq ? (PCIMAXFREQ / 64) : (PCIMINFREQ / 64));
		else
			return (max_freq ? (XTALMAXFREQ / 32) : (XTALMINFREQ / 32));
	} else if (CCREV(sii->pub.ccrev) < 10) {
		div = 4 *
		        (((R_REG(sii->osh, &cc->slow_clk_ctl) & SCC_CD_MASK) >> SCC_CD_SHIFT) + 1);
		if (slowclk == SCC_SS_LPO)
			return (max_freq ? LPOMAXFREQ : LPOMINFREQ);
		else if (slowclk == SCC_SS_XTAL)
			return (max_freq ? (XTALMAXFREQ / div) : (XTALMINFREQ / div));
		else if (slowclk == SCC_SS_PCI)
			return (max_freq ? (PCIMAXFREQ / div) : (PCIMINFREQ / div));
		else
			ASSERT(0);
	} else {
		/* Chipc rev 10 is InstaClock */
		div = R_REG(sii->osh, &cc->system_clk_ctl) >> SYCC_CD_SHIFT;
		div = 4 * (div + 1);
		return (max_freq ? XTALMAXFREQ : (XTALMINFREQ / div));
	}
	return (0);
}

static void
BCMINITFN(si_clkctl_setdelay)(si_info_t *sii, void *chipcregs)
{
	chipcregs_t *cc = (chipcregs_t *)chipcregs;
	uint slowmaxfreq, pll_delay, slowclk;
	uint pll_on_delay, fref_sel_delay;

	pll_delay = PLL_DELAY;

	/* If the slow clock is not sourced by the xtal then add the xtal_on_delay
	 * since the xtal will also be powered down by dynamic clk control logic.
	 */

	slowclk = si_slowclk_src(sii);
	if (slowclk != SCC_SS_XTAL)
		pll_delay += XTAL_ON_DELAY;

	/* Starting with 4318 it is ILP that is used for the delays */
	slowmaxfreq = si_slowclk_freq(sii, (CCREV(sii->pub.ccrev) >= 10) ? FALSE : TRUE, cc);

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
	if (CCREV(sih->ccrev) >= 10)
		SET_REG(sii->osh, &cc->system_clk_ctl, SYCC_CD_MASK,
		        (ILP_DIV_1MHZ << SYCC_CD_SHIFT));

	si_clkctl_setdelay(sii, (void *)(uintptr)cc);

	OSL_DELAY(20000);

	if (!fast)
		si_setcoreidx(sih, origidx);
}

#if !defined(BCMDONGLEHOST)
/** return the value suitable for writing to the dot11 core FAST_PWRUP_DELAY register */
uint16
BCMINITFN(si_clkctl_fast_pwrup_delay)(si_t *sih)
{
	si_info_t *sii = SI_INFO(sih);
	uint origidx = 0;
	chipcregs_t *cc;
	uint slowminfreq;
	uint16 fpdelay;
	uint intr_val = 0;
	bool fast;

	if (PMUCTL_ENAB(sih)) {
		INTR_OFF(sii, intr_val);
		fpdelay = si_pmu_fast_pwrup_delay(sih, sii->osh);
		INTR_RESTORE(sii, intr_val);
		return fpdelay;
	}

	if (!CCCTL_ENAB(sih))
		return 0;

	fast = SI_FAST(sii);
	fpdelay = 0;
	if (!fast) {
		origidx = sii->curidx;
		INTR_OFF(sii, intr_val);
		if ((cc = (chipcregs_t *)si_setcore(sih, CC_CORE_ID, 0)) == NULL)
			goto done;
	} else if ((cc = (chipcregs_t *)CCREGS_FAST(sii)) == NULL) {
		goto done;
	}

	ASSERT(cc != NULL);

	slowminfreq = si_slowclk_freq(sii, FALSE, cc);
	if (slowminfreq > 0)
		fpdelay = (((R_REG(sii->osh, &cc->pll_on_delay) + 2) * 1000000) +
		(slowminfreq - 1)) / slowminfreq;

done:
	if (!fast) {
		si_setcoreidx(sih, origidx);
		INTR_RESTORE(sii, intr_val);
	}
	return fpdelay;
}

/** turn primary xtal and/or pll off/on */
int
si_clkctl_xtal(si_t *sih, uint what, bool on)
{
	si_info_t *sii;
	uint32 in, out, outen;

	sii = SI_INFO(sih);

	switch (BUSTYPE(sih->bustype)) {

	case PCMCIA_BUS:
		return (0);

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

	/* chipcommon cores prior to rev6 don't support dynamic clock control */
	if (CCREV(sih->ccrev) < 6)
		return FALSE;

	return _si_clkctl_cc(sii, mode);
}

/* clk control mechanism through chipcommon, no policy checking */
static bool
_si_clkctl_cc(si_info_t *sii, uint mode)
{
	uint origidx = 0;
	chipcregs_t *cc;
	uint32 scc;
	uint intr_val = 0;
	bool fast = SI_FAST(sii);

	/* chipcommon cores prior to rev6 don't support dynamic clock control */
	if (CCREV(sii->pub.ccrev) < 6)
		return (FALSE);

	/* Chips with ccrev 10 are EOL and they don't have SYCC_HR which we use below */
	ASSERT(CCREV(sii->pub.ccrev) != 10);

	if (!fast) {
		INTR_OFF(sii, intr_val);
		origidx = sii->curidx;
		cc = (chipcregs_t *) si_setcore(&sii->pub, CC_CORE_ID, 0);
	} else if ((cc = (chipcregs_t *) CCREGS_FAST(sii)) == NULL)
		goto done;
	ASSERT(cc != NULL);

	if (!CCCTL_ENAB(&sii->pub) && (CCREV(sii->pub.ccrev) < 20))
		goto done;

	switch (mode) {
	case CLK_FAST:	/* FORCEHT, fast (pll) clock */
		if (CCREV(sii->pub.ccrev) < 10) {
			/* don't forget to force xtal back on before we clear SCC_DYN_XTAL.. */
			si_clkctl_xtal(&sii->pub, XTAL, ON);
			SET_REG(sii->osh, &cc->slow_clk_ctl, (SCC_XC | SCC_FS | SCC_IP), SCC_IP);
		} else if (CCREV(sii->pub.ccrev) < 20) {
			OR_REG(sii->osh, &cc->system_clk_ctl, SYCC_HR);
		} else {
			OR_REG(sii->osh, &cc->clk_ctl_st, CCS_FORCEHT);
		}

		/* wait for the PLL */
		if (PMUCTL_ENAB(&sii->pub)) {
			uint32 htavail = CCS_HTAVAIL;
			SPINWAIT(((R_REG(sii->osh, &cc->clk_ctl_st) & htavail) == 0),
			         PMU_MAX_TRANSITION_DLY);
			ASSERT(R_REG(sii->osh, &cc->clk_ctl_st) & htavail);
		} else {
			OSL_DELAY(PLL_DELAY);
		}
		break;

	case CLK_DYNAMIC:	/* enable dynamic clock control */
		if (CCREV(sii->pub.ccrev) < 10) {
			scc = R_REG(sii->osh, &cc->slow_clk_ctl);
			scc &= ~(SCC_FS | SCC_IP | SCC_XC);
			if ((scc & SCC_SS_MASK) != SCC_SS_XTAL)
				scc |= SCC_XC;
			W_REG(sii->osh, &cc->slow_clk_ctl, scc);

			/* for dynamic control, we have to release our xtal_pu "force on" */
			if (scc & SCC_XC)
				si_clkctl_xtal(&sii->pub, XTAL, OFF);
		} else if (CCREV(sii->pub.ccrev) < 20) {
			/* Instaclock */
			AND_REG(sii->osh, &cc->system_clk_ctl, ~SYCC_HR);
		} else {
			AND_REG(sii->osh, &cc->clk_ctl_st, ~CCS_FORCEHT);
		}

		/* wait for the PLL */
		if (PMUCTL_ENAB(&sii->pub)) {
			uint32 htavail = CCS_HTAVAIL;
			SPINWAIT(((R_REG(sii->osh, &cc->clk_ctl_st) & htavail) != 0),
			         PMU_MAX_TRANSITION_DLY);
			ASSERT(!(R_REG(sii->osh, &cc->clk_ctl_st) & htavail));
		} else {
			OSL_DELAY(PLL_DELAY);
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
		/*
		 * XXX JTAG is not a bus/device type. It is the interface we use
		 * to get to the core connected to the Sonic Backplane.
		 */
		if (BCM53573_CHIP(sih->chip)) {
			idx = si_findcoreidx(sih, D11_CORE_ID, 0);
		} else if (EMBEDDED_2x2AX_CORE(sih->chip)) {
			idx = (sih->enum_base == BCM47622_DEV_B_PHYS_ADDR) ? BCM47622_WLAN_DEV_B :
			                                                     BCM47622_WLAN_DEV_A;
		} else if (BCM6878_CHIP(sih->chip)) {
			/* Single core SoC. Index should be 0 here */
			idx = 0;
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
	case PCMCIA_BUS:
		SI_ERROR(("si_devpath: OSL_PCMCIA_BUS() not implemented, bus 1 assumed\n"));
		SI_ERROR(("si_devpath: OSL_PCMCIA_SLOT() not implemented, slot 1 assumed\n"));
		slen = snprintf(path, (size_t)size, "pc/1/1/");
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
		/* Example of devpath: 'sb/1/'. devpath now is 'xxx/xx/', adjust length to eliminate
		 * ending '/' (if present).
		 */
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

/** read/writes a register in chipcommon or PMU core */
uint32
si_ccreg(si_t *sih, uint32 offset, uint32 mask, uint32 val)
{
	si_info_t *sii;
	uint32 reg_val = 0;
	int coreidx = SI_CC_IDX;

	sii = SI_INFO(sih);

	if (offset > sizeof(chipcregs_t) || offset >= PMU_CTL) {
		if (AOB_ENAB(sih) && offset < sizeof(pmuregs_t)) {
			coreidx = si_findcoreidx(sih, PMU_CORE_ID, 0);
		} else {
			return 0; /* invalid register offset */
		}
	}

	reg_val = si_corereg(&sii->pub, coreidx, offset, mask, val);

	return reg_val;
}

#ifdef SR_DEBUG
void
si_dump_pmu(si_t *sih, void *arg)
{
	uint i;
	uint32 pmu_chip_ctl_reg;
	uint32 pmu_chip_reg_reg;
	uint32 pmu_chip_pll_reg;
	uint32 pmu_chip_res_reg;
	pmu_reg_t *pmu_var = (pmu_reg_t*)arg;
	pmu_var->pmu_control = si_ccreg(sih, PMU_CTL, 0, 0);
	pmu_var->pmu_capabilities = si_ccreg(sih, PMU_CAP, 0, 0);
	pmu_var->pmu_status = si_ccreg(sih, PMU_ST, 0, 0);
	pmu_var->res_state = si_ccreg(sih, PMU_RES_STATE, 0, 0);
	pmu_var->res_pending = si_ccreg(sih, PMU_RES_PENDING, 0, 0);
	pmu_var->pmu_timer1 = si_ccreg(sih, PMU_TIMER, 0, 0);
	pmu_var->min_res_mask = si_ccreg(sih, MINRESMASKREG, 0, 0);
	pmu_var->max_res_mask = si_ccreg(sih, MAXRESMASKREG, 0, 0);
	pmu_chip_ctl_reg = (pmu_var->pmu_capabilities & 0xf8000000);
	pmu_chip_ctl_reg = pmu_chip_ctl_reg >> 27;
	for (i = 0; i < pmu_chip_ctl_reg; i++) {
		pmu_var->pmu_chipcontrol1[i] = si_pmu_chipcontrol(sih, i, 0, 0);
	}
	pmu_chip_reg_reg = (pmu_var->pmu_capabilities & 0x07c00000);
	pmu_chip_reg_reg = pmu_chip_reg_reg >> 22;
	for (i = 0; i < pmu_chip_reg_reg; i++) {
		pmu_var->pmu_regcontrol[i] = si_pmu_vreg_control(sih, i, 0, 0);
	}
	pmu_chip_pll_reg = (pmu_var->pmu_capabilities & 0x003e0000);
	pmu_chip_pll_reg = pmu_chip_pll_reg >> 17;
	for (i = 0; i < pmu_chip_pll_reg; i++) {
		pmu_var->pmu_pllcontrol[i] = si_pmu_pllcontrol(sih, i, 0, 0);
	}
	pmu_chip_res_reg = (pmu_var->pmu_capabilities & 0x00001f00);
	pmu_chip_res_reg = pmu_chip_res_reg >> 8;
	for (i = 0; i < pmu_chip_res_reg; i++) {
		si_corereg(sih, SI_CC_IDX, RSRCTABLEADDR, ~0, i);
		pmu_var->pmu_rsrc_up_down_timer[i] = si_corereg(sih, SI_CC_IDX,
			RSRCUPDWNTIME, 0, 0);
	}
	pmu_chip_res_reg = (pmu_var->pmu_capabilities & 0x00001f00);
	pmu_chip_res_reg = pmu_chip_res_reg >> 8;
	for (i = 0; i < pmu_chip_res_reg; i++) {
		si_corereg(sih, SI_CC_IDX, RSRCTABLEADDR, ~0, i);
		pmu_var->rsrc_dep_mask[i] = si_corereg(sih, SI_CC_IDX, PMU_RES_DEP_MASK, 0, 0);
	}
}

void
si_pmu_keep_on(si_t *sih, int32 int_val)
{
	si_info_t *sii;
	chipcregs_t *cc;
	sii = SI_INFO(sih);
	cc = (chipcregs_t *)si_setcore(sih, CC_CORE_ID, 0);
	uint32 res_dep_mask;
	uint32 min_res_mask;
	uint32 max_res_mask;
	/* Corresponding Resource Dependancy Mask */
	W_REG(sii->osh, &cc->res_table_sel, int_val);
	res_dep_mask = R_REG(sii->osh, &cc->res_dep_mask);
	/* Local change of minimum resource mask */
	min_res_mask = res_dep_mask | 1 << int_val;
	/* Corresponding change of Maximum Resource Mask */
	max_res_mask = R_REG(sii->osh, &cc->max_res_mask);
	max_res_mask  = max_res_mask | min_res_mask;
	W_REG(sii->osh, &cc->max_res_mask, max_res_mask);
	/* Corresponding change of Minimum Resource Mask */
	W_REG(sii->osh, &cc->min_res_mask, min_res_mask);
}

uint32
si_pmu_keep_on_get(si_t *sih)
{
	uint i;
	si_info_t *sii;
	chipcregs_t *cc;
	sii = SI_INFO(sih);
	cc = (chipcregs_t *)si_setcore(sih, CC_CORE_ID, 0);
	uint32 res_dep_mask;
	uint32 min_res_mask;
	/* Read min res mask */
	min_res_mask = R_REG(sii->osh, &cc->min_res_mask);
	/* Get corresponding Resource Dependancy Mask  */
	for (i = 0; i < PMU_RES; i++) {
		W_REG(sii->osh, &cc->res_table_sel, i);
		res_dep_mask = R_REG(sii->osh, &cc->res_dep_mask);
		res_dep_mask = res_dep_mask | 1 << i;
		/* Compare with the present min res mask */
		if (res_dep_mask == min_res_mask) {
			return i;
		}
	}
	return 0;
}

uint32
si_power_island_set(si_t *sih, uint32 int_val)
{
	uint32 i = 0x0;
	uint32 j;
	uint32 k;
	int cnt = 0;
	for (k = 0; k < ARRAYSIZE(si_power_island_test_array); k++) {
		if (int_val == si_power_island_test_array[k]) {
			cnt = cnt + 1;
		}
	}
	if (cnt > 0) {
		if (int_val & SUBCORE_POWER_ON) {
			i = i | 0x1;
		}
		if (int_val & PHY_POWER_ON) {
			i = i | 0x2;
		}
		if (int_val & VDDM_POWER_ON) {
			i = i | 0x4;
		}
		if (int_val & MEMLPLDO_POWER_ON) {
			i = i | 0x8;
		}
		j = (i << 18) & 0x003c0000;
		si_pmu_chipcontrol(sih, CHIPCTRLREG2, 0x003c0000, j);
	} else {
		return 0;
	}

	return 1;
}

uint32
si_power_island_get(si_t *sih)
{
	uint32 sc_on = 0x0;
	uint32 phy_on = 0x0;
	uint32 vddm_on = 0x0;
	uint32 memlpldo_on = 0x0;
	uint32 res;
	uint32 reg_val;
	reg_val = si_pmu_chipcontrol(sih, CHIPCTRLREG2, 0, 0);
	if (reg_val & SUBCORE_POWER_ON_CHK) {
		sc_on = SUBCORE_POWER_ON;
	}
	if (reg_val & PHY_POWER_ON_CHK) {
		phy_on = PHY_POWER_ON;
	}
	if (reg_val & VDDM_POWER_ON_CHK) {
		vddm_on = VDDM_POWER_ON;
	}
	if (reg_val & MEMLPLDO_POWER_ON_CHK) {
		memlpldo_on = MEMLPLDO_POWER_ON;
	}
	res = (sc_on | phy_on | vddm_on | memlpldo_on);
	return res;
}
#endif /* SR_DEBUG */

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

/* Wake-on-wireless-LAN (WOWL) support functions */
/** Enable PME generation and disable clkreq */
void
si_pci_pmeen(si_t *sih)
{
	pcicore_pmeen(SI_INFO(sih)->pch);
}

/** Return TRUE if PME status is set */
bool
si_pci_pmestat(si_t *sih)
{
	return pcicore_pmestat(SI_INFO(sih)->pch);
}

/** Disable PME generation, clear the PME status bit if set */
void
si_pci_pmeclr(si_t *sih)
{
	pcicore_pmeclr(SI_INFO(sih)->pch);
}

void
si_pci_pmestatclr(si_t *sih)
{
	pcicore_pmestatclr(SI_INFO(sih)->pch);
}

/** initialize the pcmcia core */
void
si_pcmcia_init(si_t *sih)
{
	si_info_t *sii = SI_INFO(sih);
	uint8 cor = 0;

	/* enable d11 mac interrupts */
	OSL_PCMCIA_READ_ATTR(sii->osh, PCMCIA_FCR0 + PCMCIA_COR, &cor, 1);
	cor |= COR_IRQEN | COR_FUNEN;
	OSL_PCMCIA_WRITE_ATTR(sii->osh, PCMCIA_FCR0 + PCMCIA_COR, &cor, 1);

}

/**
 * Disable pcie_war_ovr for some platforms (sigh!)
 * This is for boards that have BFL2_PCIEWAR_OVR set
 * but are in systems that still want the benefits of ASPM
 * Note that this should be done AFTER si_doattach
 */
void
si_pcie_war_ovr_update(si_t *sih, uint8 aspm)
{
	si_info_t *sii = SI_INFO(sih);

	if (!PCIE_GEN1(sii))
		return;

	pcie_war_ovr_aspm_update(sii->pch, aspm);
}

void
si_pcie_power_save_enable(si_t *sih, bool enable)
{
	si_info_t *sii = SI_INFO(sih);

	if (!PCIE_GEN1(sii))
		return;

	pcie_power_save_enable(sii->pch, enable);
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

int
si_pcie_configspace_cache(si_t *sih)
{
	si_info_t *sii = SI_INFO(sih);

	if (!PCIE(sii))
		return BCME_UNSUPPORTED;

	return pcie_configspace_cache(sii->pch);
}

int
si_pcie_configspace_restore(si_t *sih)
{
	si_info_t *sii = SI_INFO(sih);

	if (!PCIE(sii))
		return BCME_UNSUPPORTED;

	return pcie_configspace_restore(sii->pch);
}

int
si_pcie_configspace_get(si_t *sih, uint8 *buf, uint size)
{
	si_info_t *sii = SI_INFO(sih);

	if (!PCIE(sii) || size > PCI_CONFIG_SPACE_SIZE)
		return -1;

	return pcie_configspace_get(sii->pch, buf, size);
}

/** back door for other module to override chippkg */
void
si_chippkg_set(si_t *sih, uint val)
{
	si_info_t *sii = SI_INFO(sih);

	sii->pub.chippkg = val;
}

void
si_pcie_hw_L1SS_war(si_t *sih)
{
	si_info_t *sii = SI_INFO(sih);

	/* XXX SWWLAN-41753: WAR intermittent issue with D3Cold and L1.2 exit,
	 * need to update PMU rsrc dependency
	 */
	if (PCIE_GEN2(sii))
		pcie_hw_L1SS_war(sii->pch);
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
	do_4360_pcie2_war = 0;

	pcicore_sleep(SI_INFO(sih)->pch);
}

/** Unconfigure and/or apply various WARs when the wireless interface is going down */
void
BCMINITFN(si_pci_down)(si_t *sih)
{
	si_info_t *sii = SI_INFO(sih);
	BCM_REFERENCE(sii);

	/* if not pci bus, we're done */
	if (BUSTYPE(sih->bustype) != PCI_BUS)
		return;

	pcicore_down(sii->pch, SI_PCIDOWN);
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
		w |= (coremask << PCI_SBIM_SHIFT);
#ifdef USER_MODE
		/* User mode operate with interrupt disabled */
		w &= !(coremask << PCI_SBIM_SHIFT);
#endif // endif
		OSL_PCI_WRITE_CONFIG(sii->osh, PCI_INT_MASK, sizeof(uint32), w);
	} else {
		/* set sbintvec bit for our flag number */
		si_setint(sih, siflag);
	}

	/*
	 * enable prefetch and bursts for dma big window
	 * enable read multiple for dma big window corerev >= 11
	 * PR 9962/4708: Set initiator timeouts. corerev < 5
	 */
	if (PCI(sii)) {
		OR_REG(sii->osh, &pciregs->sbtopci2, (SBTOPCI_PREF | SBTOPCI_BURST));
		if (sii->pub.buscorerev >= 11) {
			OR_REG(sii->osh, &pciregs->sbtopci2, SBTOPCI_RC_READMULTI);
			/* XXX PR50531: On some Laptops, the 4321 CB shows bad
			 * UDP performance on one direction
			 */
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

uint32
si_pcieltrspacing_reg(si_t *sih, uint32 mask, uint32 val)
{
	si_info_t *sii = SI_INFO(sih);

	if (!(PCIE(sii)))
		return 0;

	return pcieltrspacing_reg(sii->pch, mask, val);
}

uint32
si_pcieltrhysteresiscnt_reg(si_t *sih, uint32 mask, uint32 val)
{
	si_info_t *sii = SI_INFO(sih);

	if (!(PCIE(sii)))
		return 0;

	return pcieltrhysteresiscnt_reg(sii->pch, mask, val);
}

void
si_pcie_set_error_injection(si_t *sih, uint32 mode)
{
	si_info_t *sii = SI_INFO(sih);

	if (!PCIE(sii))
		return;

	pcie_set_error_injection(sii->pch, mode);
}

void
si_pcie_set_L1substate(si_t *sih, uint32 substate)
{
	si_info_t *sii;

	sii = SI_INFO(sih);

	if (PCIE_GEN2(sii))
		pcie_set_L1substate(sii->pch, substate);
}
#ifndef BCM_BOOTLOADER
uint32
si_pcie_get_L1substate(si_t *sih)
{
	si_info_t *sii;

	sii = SI_INFO(sih);

	if (PCIE_GEN2(sii))
		return pcie_get_L1substate(sii->pch);

	return 0;
}
#endif /* BCM_BOOTLOADER */
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

	pcicore_hwup(sii->pch);
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

void
si_dump_pmuregs(si_t *sih, struct bcmstrbuf *b)
{
	uint i;
	uint32 pmu_cap;
	uint32 pmu_chip_reg;

	bcm_bprintf(b, "===pmu(rev %d)===\n", sih->pmurev);
	if (!(sih->pmurev == 0x11 || (sih->pmurev >= 0x15 && sih->pmurev <= 0x19))) {
		bcm_bprintf(b, "PMU dump not supported\n");
		return;
	}
	pmu_cap = si_ccreg(sih, PMU_CAP, 0, 0);
	bcm_bprintf(b, "pmu_control 0x%x\n", si_ccreg(sih, PMU_CTL, 0, 0));
	bcm_bprintf(b, "pmu_capabilities 0x%x\n", pmu_cap);
	bcm_bprintf(b, "pmu_status 0x%x\n", si_ccreg(sih, PMU_ST, 0, 0));
	bcm_bprintf(b, "res_state 0x%x\n", si_ccreg(sih, PMU_RES_STATE, 0, 0));
	bcm_bprintf(b, "res_pending 0x%x\n", si_ccreg(sih, PMU_RES_PENDING, 0, 0));
	bcm_bprintf(b, "pmu_timer1 %d\n", si_ccreg(sih, PMU_TIMER, 0, 0));
	bcm_bprintf(b, "min_res_mask 0x%x\n", si_ccreg(sih, MINRESMASKREG, 0, 0));
	bcm_bprintf(b, "max_res_mask 0x%x\n", si_ccreg(sih, MAXRESMASKREG, 0, 0));

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
}

int
si_dump_pcieregs(si_t *sih, struct bcmstrbuf *b)
{
	si_info_t *sii = SI_INFO(sih);

	if (!PCIE_GEN1(sii) && !PCIE_GEN2(sii))
		return BCME_ERROR;

	return pcicore_dump_pcieregs(sii->pch, b);
}

#endif // endif

#if defined(BCMDBG) || defined(BCMDBG_PHYDUMP)
#endif // endif

#endif /* !defined(BCMDONGLEHOST) */

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
	if (CCREV(sih->ccrev) < 16)
		return 0xffffffff;

	/* gpio led powersave reg */
	return (si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, gpiotimeroutmask), mask, val));
}

/* mask&set gpio timer val */
uint32
si_gpiotimerval(si_t *sih, uint32 mask, uint32 gpiotimerval)
{
	if (CCREV(sih->ccrev) < 16)
		return 0xffffffff;

	return (si_corereg(sih, SI_CC_IDX,
		OFFSETOF(chipcregs_t, gpiotimerval), mask, gpiotimerval));
}

uint32
si_gpiopull(si_t *sih, bool updown, uint32 mask, uint32 val)
{
	uint offs;

	if (CCREV(sih->ccrev) < 20)
		return 0xffffffff;

	offs = (updown ? OFFSETOF(chipcregs_t, gpiopulldown) : OFFSETOF(chipcregs_t, gpiopullup));
	return (si_corereg(sih, SI_CC_IDX, offs, mask, val));
}

uint32
si_gpioevent(si_t *sih, uint regtype, uint32 mask, uint32 val)
{
	uint offs;

	if (CCREV(sih->ccrev) < 11)
		return 0xffffffff;

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

	if (CCREV(sih->ccrev) < 11)
		return 0xffffffff;

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
static void
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
#endif // endif

	if (AOB_ENAB(sih)) {
		pmu = si_setcore(sih, PMU_CORE_ID, 0);
	} else {
		pmu = si_setcoreidx(sih, SI_CC_IDX);
	}
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
			SI_ERROR(("SECI clock is still not available\n"));
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

	if (sih->ccrev < 35)
		return NULL;

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
		if ((CHIPID(sih->chip) == BCM4360_CHIP_ID) ||
			(CHIPID(sih->chip) == BCM43460_CHIP_ID) ||
			(CHIPID(sih->chip) == BCM43526_CHIP_ID) ||
			(CHIPID(sih->chip) == BCM4352_CHIP_ID)) {
			/* MAC clk is 160MHz */
			offset = OFFSETOF(chipcregs_t, seci_uart_bauddiv);
			si_corereg(sih, SI_CC_IDX, offset, 0xFF, 0xFE);
			offset = OFFSETOF(chipcregs_t, seci_uart_baudadj);
			si_corereg(sih, SI_CC_IDX, offset, 0xFF, 0x44);
			offset = OFFSETOF(chipcregs_t, seci_uart_mcr);
			si_corereg(sih, SI_CC_IDX, offset,
				0xFF, SECI_UART_MCR_BAUD_ADJ_EN); /* 0x81 */
		}
#ifdef SECI_UART
		} else if (BCM4347_CHIP(sih->chip)) {
			/* rx FIFO level at which an interrupt is generated */
			offset = OFFSETOF(chipcregs_t, eci.ge35.eci_uartfifolevel);
			si_corereg(sih, SI_CC_IDX, offset, 0xff, 0x01);
			offset = OFFSETOF(chipcregs_t, seci_uart_mcr);
			si_corereg(sih, SI_CC_IDX, offset, SECI_UART_MCR_AUTO_RTS,
				SECI_UART_MCR_AUTO_RTS);
		}
#endif /* SECI_UART */
		else {
			/* 4336 MAC clk is 80MHz */
			offset = OFFSETOF(chipcregs_t, seci_uart_baudadj);
			si_corereg(sih, SI_CC_IDX, offset, 0xFF, 0x22);
			offset = OFFSETOF(chipcregs_t, seci_uart_mcr);
			si_corereg(sih, SI_CC_IDX, offset,
				0xFF, SECI_UART_MCR_BAUD_ADJ_EN); /* 0x80 */
		}

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
	si_eci_notify_bt((sih), ECI_OUT_FM_DISABLE_MASK(CCREV(sih->ccrev)), \
			 ((val) << ECI_OUT_FM_DISABLE_SHIFT(CCREV(sih->ccrev))), FALSE)

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
	if (CCREV(sih->ccrev) < 35) {
		W_REG(sii->osh, &cc->eci.lt35.eci_intmaskhi, 0x0);
		W_REG(sii->osh, &cc->eci.lt35.eci_intmaskmi, 0x0);
		W_REG(sii->osh, &cc->eci.lt35.eci_intmasklo, 0x0);
	} else {
		W_REG(sii->osh, &cc->eci.ge35.eci_intmaskhi, 0x0);
		W_REG(sii->osh, &cc->eci.ge35.eci_intmasklo, 0x0);
	}

	/* Assign eci_output bits between 'wl' and dot11mac */
	if (CCREV(sih->ccrev) < 35) {
		W_REG(sii->osh, &cc->eci.lt35.eci_control, ECI_MACCTRL_BITS);
	} else {
		W_REG(sii->osh, &cc->eci.ge35.eci_controllo, ECI_MACCTRLLO_BITS);
		W_REG(sii->osh, &cc->eci.ge35.eci_controlhi, ECI_MACCTRLHI_BITS);
	}

	/* enable only edge based interrupts
	 * only toggle on bit 62 triggers an interrupt
	 */
	if (CCREV(sih->ccrev) < 35) {
		W_REG(sii->osh, &cc->eci.lt35.eci_eventmaskhi, 0x0);
		W_REG(sii->osh, &cc->eci.lt35.eci_eventmaskmi, 0x0);
		W_REG(sii->osh, &cc->eci.lt35.eci_eventmasklo, 0x0);
	} else {
		W_REG(sii->osh, &cc->eci.ge35.eci_eventmaskhi, 0x0);
		W_REG(sii->osh, &cc->eci.ge35.eci_eventmasklo, 0x0);
	}

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

	if ((sih->cccaps & CC_CAP_ECI) ||
		(si_seci(sih)))
	{
		/* ECI or SECI mode */
		/* Clear interrupt bit by default */
		if (is_interrupt) {
			si_corereg(sih, SI_CC_IDX,
			   (CCREV(sih->ccrev) < 35 ?
			    OFFSETOF(chipcregs_t, eci.lt35.eci_output) :
			    OFFSETOF(chipcregs_t, eci.ge35.eci_outputlo)),
			   (1 << 30), 0);
		}

		if (CCREV(sih->ccrev) >= 35) {
			if ((mask & 0xFFFF0000) == ECI48_OUT_MASKMAGIC_HIWORD) {
				offset = OFFSETOF(chipcregs_t, eci.ge35.eci_outputhi);
				mask = mask & ~0xFFFF0000;
			} else {
				offset = OFFSETOF(chipcregs_t, eci.ge35.eci_outputlo);
				mask = mask | (1<<30);
				val = val & ~(1 << 30);
			}
		} else {
			offset = OFFSETOF(chipcregs_t, eci.lt35.eci_output);
			val = val & ~(1 << 30);
		}

		si_corereg(sih, SI_CC_IDX, offset, mask, val);

		/* Set interrupt bit if needed */
		if (is_interrupt) {
			si_corereg(sih, SI_CC_IDX,
			   (CCREV(sih->ccrev) < 35 ?
			    OFFSETOF(chipcregs_t, eci.lt35.eci_output) :
			    OFFSETOF(chipcregs_t, eci.ge35.eci_outputlo)),
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

void
si_seci_upd(si_t *sih, bool enable)
{
	uint32 origidx = 0;
	si_info_t *sii = SI_INFO(sih);
	chipcregs_t *cc;
	bool fast;
	uint32 regval, seci_ctrl;
	uint intr_val = 0;

	if (!si_seci(sih))
		return;

	fast = SI_FAST(sii);
	INTR_OFF(sii, intr_val);
	if (!fast) {
		origidx = sii->curidx;
		if ((cc = (chipcregs_t *)si_setcore(sih, CC_CORE_ID, 0)) == NULL)
			goto exit;
	} else if ((cc = (chipcregs_t *)CCREGS_FAST(sii)) == NULL)
		goto exit;

	ASSERT(cc);

	/* Select SECI based on enable input */
	if ((CHIPID(sih->chip) == BCM4352_CHIP_ID) || (CHIPID(sih->chip) == BCM4360_CHIP_ID)) {
		regval = R_REG(sii->osh, &cc->chipcontrol);

		seci_ctrl = CCTRL4360_SECI_ON_GPIO01;

		if (enable) {
			regval |= seci_ctrl;
		} else {
			regval &= ~seci_ctrl;
		}
		W_REG(sii->osh, &cc->chipcontrol, regval);

		if (enable) {
			/* Send ECI update to BT */
			regval = R_REG(sii->osh, &cc->SECI_config);
			regval |= SECI_UPD_SECI;
			W_REG(sii->osh, &cc->SECI_config, regval);
			SPINWAIT((R_REG(sii->osh, &cc->SECI_config) & SECI_UPD_SECI), 1000);
			/* Request ECI update from BT */
			W_REG(sii->osh, &cc->seci_uart_data, SECI_SLIP_ESC_CHAR);
			W_REG(sii->osh, &cc->seci_uart_data, SECI_REFRESH_REQ);
		}
	}

exit:
	/* restore previous core */
	if (!fast)
		si_setcoreidx(sih, origidx);

	INTR_RESTORE(sii, intr_val);
} /* si_seci_upd */

void *
BCMINITFN(si_gci_init)(si_t *sih)
{
#ifdef HNDGCI
	si_info_t *sii = SI_INFO(sih);
#endif /* HNDGCI */

	if (sih->cccaps_ext & CC_CAP_EXT_GCI_PRESENT)
	{
		si_gci_reset(sih);

		if (!(sih->boardflags2 & BFL2_BTCLEGACY)) {
			si_gci_seci_init(sih);
		}

		/* Set GCI Control bits 40 - 47 to be SW Controlled. These bits
		contain WL channel info and are sent to BT.
		*/
		si_gci_direct(sih, GCI_OFFSETOF(sih, gci_control_1),
			GCI_WL_CHN_INFO_MASK, GCI_WL_CHN_INFO_MASK);
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
/**
 * For boards that use GPIO(8) is used for Bluetooth Coex TX_WLAN pin,
 * when GPIOControl for Pin 8 is with ChipCommon core,
 * if UART_TX_1 (bit 5: Chipc capabilities) strapping option is set, then
 * GPIO pin 8 is driven by Uart0MCR:2 rather than GPIOOut:8. To drive this pin
 * low, one has to set Uart0MCR:2 to 1. This is required when the BTC is disabled,
 * or the driver goes down. Refer to PR35488.
 */
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

/** switch muxed pins, on: SROM, off: FEMCTRL. Called for a family of ac chips, not just 4360. */
void
si_chipcontrl_srom4360(si_t *sih, bool on)
{
	si_info_t *sii = SI_INFO(sih);
	chipcregs_t *cc;
	uint origidx = si_coreidx(sih);
	uint32 val;

	if ((cc = (chipcregs_t *)si_setcore(sih, CC_CORE_ID, 0)) == NULL) {
		SI_ERROR(("%s: Failed to find CORE ID!\n", __FUNCTION__));
		return;
	}
	val = R_REG(sii->osh, &cc->chipcontrol);

	if (on) {
		val &= ~(CCTRL4360_SECI_MODE |
			CCTRL4360_BTSWCTRL_MODE |
			CCTRL4360_EXTRA_FEMCTRL_MODE |
			CCTRL4360_BT_LGCY_MODE |
			CCTRL4360_CORE2FEMCTRL4_ON);

		W_REG(sii->osh, &cc->chipcontrol, val);
	} else {
	}

	si_setcoreidx(sih, origidx);
}

/**
 * The SROM clock is derived from the backplane clock. 4365 (200Mhz) and 43684/6710 (242Mhz)
 * have a fast backplane clock that requires a higher-than-POR-default clock divisor ratio
 * for the SROM clock.
 */
void
si_srom_clk_set(si_t *sih)
{
	si_info_t *sii = SI_INFO(sih);
	chipcregs_t *cc;
	uint origidx = si_coreidx(sih);
	uint32 val;
	uint32 divisor = 1;

	if ((cc = (chipcregs_t *)si_setcore(sih, CC_CORE_ID, 0)) == NULL) {
		SI_ERROR(("%s: Failed to find CORE ID!\n", __FUNCTION__));
		return;
	}

	val = R_REG(sii->osh, &cc->clkdiv2);
	if (BCM4365_CHIP(sih->chip)) {
		divisor = CLKD2_SROMDIV_192; /* divide 200 by 192 -> SPROM clock ~ 1.04Mhz */
	} else if (BCM43684_CHIP(sih->chip) || BCM6710_CHIP(sih->chip)) {
		divisor = CLKD2_SROMDIV_256; /* divide 242 by 256 -> SPROM clock ~ 0.95Mhz */
	} else {
		ASSERT(0);
	}

	W_REG(sii->osh, &cc->clkdiv2, ((val & ~CLKD2_SROM) | divisor));
	si_setcoreidx(sih, origidx);
}
#endif /* (!_CFE_ && !_CFEZ_) || CFG_WL */

void
si_pmu_avb_clk_set(si_t *sih, osl_t *osh, bool set_flag)
{
#if !defined(BCMDONGLEHOST)
	switch (CHIPID(sih->chip)) {
		case BCM43460_CHIP_ID:
		case BCM4360_CHIP_ID:
		case BCM4366_CHIP_ID:
		case BCM43684_CHIP_ID:
			si_pmu_avbtimer_enable(sih, osh, set_flag);
			break;
		default:
			break;
	}
#endif // endif
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
	}
	return FALSE;
}

bool
si_is_warmboot(void)
{

#ifdef BCMULP
	return (boot_type == WARM_BOOT);
#else
	return FALSE;
#endif // endif
}

bool
si_is_sprom_available(si_t *sih)
{
	if (CCREV(sih->ccrev) >= 31) {
		si_info_t *sii;
		uint origidx;
		chipcregs_t *cc;
		uint32 sromctrl;

		if ((sih->cccaps & CC_CAP_SROM) == 0)
			return FALSE;

		sii = SI_INFO(sih);
		origidx = sii->curidx;
		cc = si_setcoreidx(sih, SI_CC_IDX);
		ASSERT(cc);
		sromctrl = R_REG(sii->osh, &cc->sromcontrol);
		si_setcoreidx(sih, origidx);
		return (sromctrl & SRC_PRESENT);
	}

	switch (CHIPID(sih->chip)) {
	case BCM53573_CHIP_GRPID:
		return FALSE; /* SPROM PRESENT is not defined for 53573 as of now */
	case BCM4369_CHIP_GRPID:
		if (CHIPREV(sih->chiprev) == 0) {
			/* WAR for 4369a0: HW4369-1729. no sprom, default to otp always. */
			return 0;
		} else {
			return (sih->chipst & CST4369_SPROM_PRESENT) != 0;
		}
	case BCM4347_CHIP_GRPID:
		return (sih->chipst & CST4347_SPROM_PRESENT) != 0;
		break;
	case BCM43570_CHIP_ID:
		return (sih->chipst & CST4350_SPROM_PRESENT) != 0;
	CASE_BCM43602_CHIP:
		return (sih->chipst & CST43602_SPROM_PRESENT) != 0;
	case BCM43217_CHIP_ID:
	case BCM43428_CHIP_ID:
		return (sih->chipst & CST43228_OTP_PRESENT) != CST43228_OTP_PRESENT;
	CASE_EMBEDDED_2x2AX_CORE:
	default:
		return TRUE;
	}
} /* si_is_sprom_available */

#if !defined(BCMDONGLEHOST)
bool
si_is_otp_disabled(si_t *sih)
{
	switch (CHIPID(sih->chip)) {
	case BCM6878_CHIP_ID:
	CASE_EMBEDDED_2x2AX_CORE:
		return TRUE;
	case BCM53573_CHIP_GRPID:
	case BCM4360_CHIP_ID:
	case BCM43526_CHIP_ID:
	case BCM43460_CHIP_ID:
	case BCM4352_CHIP_ID:
	case BCM43602_CHIP_ID:
		/* 4360 OTP is always powered and enabled */
		return FALSE;
	/* These chips always have their OTP on */
	case BCM43217_CHIP_ID:
	case BCM43428_CHIP_ID:
	case BCM43570_CHIP_ID:
	case BCM4347_CHIP_GRPID:
	case BCM4369_CHIP_GRPID:
	default:
		return FALSE;
	}
}

bool
si_is_otp_powered(si_t *sih)
{
	if (PMUCTL_ENAB(sih))
		return si_pmu_is_otp_powered(sih, si_osh(sih));
	return TRUE;
}

void
si_otp_power(si_t *sih, bool on, uint32* min_res_mask)
{
	if (PMUCTL_ENAB(sih))
		si_pmu_otp_power(sih, si_osh(sih), on, min_res_mask);
	OSL_DELAY(1000);
}

/* Return BCME_NOTFOUND if the card doesn't have CIS format nvram */
int
si_cis_source(si_t *sih)
{
	/* Most PCI chips use SROM format instead of CIS */
	if (BUSTYPE(sih->bustype) == PCI_BUS && (!BCM4350_CHIP(sih->chip) &&
		!BCM43684_CHIP(sih->chip) &&
		!BCM6710_CHIP(sih->chip) &&
		!BCM6715_CHIP(sih->chip) &&
		!BCM4365_CHIP(sih->chip))) {
		return BCME_NOTFOUND;
	}

	switch (CHIPID(sih->chip)) {
	case BCM43570_CHIP_ID:
	{
		if (sih->chipst & CST4350_SPROM_PRESENT)
			return CIS_SROM;
		return CIS_OTP;
	}
	case BCM4360_CHIP_ID:
	case BCM43460_CHIP_ID:
	case BCM4352_CHIP_ID:
	case BCM43526_CHIP_ID: {
		if ((sih->chipst & CST4360_OTP_ENABLED))
			return CIS_OTP;
		return CIS_DEFAULT;
	}
	CASE_BCM43602_CHIP:
		if (sih->chipst & CST43602_SPROM_PRESENT) {
			/* Don't support CIS formatted SROM, use 'real' SROM format instead */
			return BCME_NOTFOUND;
		}
		return CIS_OTP;
	case BCM4365_CHIP_ID:
	case BCM4366_CHIP_ID:
	case BCM43664_CHIP_ID:
	case BCM43666_CHIP_ID:
	CASE_BCM43684_CHIP:
	CASE_BCM6710_CHIP:
	CASE_BCM6715_CHIP:
	{
		if (si_is_sprom_available(sih)) {
			return BCME_NOTFOUND;
		}
		return CIS_OTP;
	}
	case BCM4347_CHIP_GRPID:
		if (sih->chipst & CST4347_SPROM_PRESENT)
			return CIS_SROM;
		return CIS_OTP;
	case BCM4369_CHIP_GRPID:
		if (CHIPREV(sih->chiprev) == 0) {
			/* WAR for 4369a0: HW4369-1729 */
			return CIS_OTP;
		} else if (sih->chipst & CST4369_SPROM_PRESENT) {
			return CIS_SROM;
		}
		return CIS_OTP;
	CASE_EMBEDDED_2x2AX_CORE:
		return BCME_NOTFOUND;
	default:
		return CIS_DEFAULT;
	}
} /* si_cis_source */

uint16 BCMATTACHFN(si_fabid)(si_t *sih)
{
	uint32 data;
	uint16 fabid = 0;

	switch (CHIPID(sih->chip)) {
		CASE_BCM43602_CHIP:
		case BCM53573_CHIP_GRPID:
		case BCM4347_CHIP_GRPID:
		case BCM43570_CHIP_ID:
		case BCM4369_CHIP_GRPID:
			/* intentional fallthrough */
			data = si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, fabid),	0, 0);
			fabid = data & 0xf;
			break;

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
		if ((R_REG(osh, &cc->capabilities) & CC_CAP_SROM) != 0 &&
		     (R_REG(osh, &cc->sromcontrol) & SRC_PRESENT)) {
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

#if !defined(BCMDONGLEHOST)
/**
 * To make sure that, res mask is minimal to save power and also, to indicate
 * specifically for 4335 host about the SR logic.
 */
void
si_update_masks(si_t *sih)
{
	si_info_t *sii = SI_INFO(sih);

	switch (CHIPID(sih->chip)) {
	case BCM4347_CHIP_GRPID:
	case BCM4369_CHIP_GRPID:
	CASE_BCM43602_CHIP:
		if (PMUCTL_ENAB(sih))
			si_pmu_res_minmax_update(sih, sii->osh);
		break;

	case BCM43570_CHIP_ID:
		if (PMUCTL_ENAB(sih))
			si_pmu_res_minmax_update(sih, sii->osh);

		/* Enable BBPLL power down */
		si_pmu_chipcontrol(sih, CHIPCTRLREG1, 0x10, 0x10);

		/* Disable BBPLL open-loop mode */
		si_pmu_chipcontrol(sih, CHIPCTRLREG1, 0x80, 0);

		/* BBPLL closed loop lock time = 1 (default=4) */
		si_pmu_chipcontrol(sih, CHIPCTRLREG1, 0xf0000, 0x10000);
	    break;

	default:
		ASSERT(0);
	break;
	}
} /* si_update_masks */

void
si_force_islanding(si_t *sih, bool enable)
{
	switch (CHIPID(sih->chip)) {
	case BCM4365_CHIP_ID:
	case BCM4366_CHIP_ID:
	case BCM7271_CHIP_ID:
	case BCM43664_CHIP_ID:
	case BCM43666_CHIP_ID:
		if (enable) {
			/* Turn on the islands */
			si_pmu_chipcontrol(sih, CHIPCTRLREG2, 0x1c0000, 0x0);
#if !defined(USE_MEMLPLDO)
			/* Force vddm pwrsw always on */
			si_pmu_chipcontrol(sih, CHIPCTRLREG2, 0x100000, 0x100000);
#endif // endif
		} else {
			/* Turn off the islands */
			si_pmu_chipcontrol(sih, CHIPCTRLREG2, 0x3c0000, 0x3c0000);
		}
		break;

	default:
		ASSERT(0);
		break;
	}
} /* si_force_islanding */

#endif /* !defined(BCMDONGLEHOST) */

/* cleanup the timer from the host when ARM is been halted
 * without a chance for ARM cleanup its resources
 * If left not cleanup, Intr from a software timer can still
 * request HT clk when ARM is halted.
 */
uint32
si_pmu_res_req_timer_clr(si_t *sih)
{
	uint32 mask;

	mask = PRRT_REQ_ACTIVE | PRRT_INTEN | PRRT_HT_REQ;
	mask <<= 14;
	/* clear mask bits */
	pmu_corereg(sih, SI_CC_IDX, res_req_timer, mask, 0);
	/* readback to ensure write completes */
	return pmu_corereg(sih, SI_CC_IDX, res_req_timer, 0, 0);
}

/** turn on/off rfldo */
void
si_pmu_rfldo(si_t *sih, bool on)
{
#if !defined(BCMDONGLEHOST)
	switch (CHIPID(sih->chip)) {
	case BCM4360_CHIP_ID:
	case BCM4352_CHIP_ID:
	case BCM43526_CHIP_ID: {
	CASE_BCM43602_CHIP:
		si_pmu_vreg_control(sih, PMU_VREG_0, RCTRL4360_RFLDO_PWR_DOWN,
			on ? 0 : RCTRL4360_RFLDO_PWR_DOWN);
		break;
	}
	default:
		ASSERT(0);
	break;
	}
#endif // endif
}

#if !defined(BCMDONGLEHOST)
uint32
si_pcie_set_ctrlreg(si_t *sih, uint32 mask, uint32 val)
{
#if !defined(BCMDONGLEHOST)
	si_info_t *sii = SI_INFO(sih);

	if (!PCIE(sii))
		return 0;

	if (BUSTYPE(sih->bustype) != PCI_BUS)
		return 0;

	return pcie_set_ctrlreg(sii->pch, mask, val);
#endif // endif
	return 0;
}
#endif /* !defined(BCMDONGLEHOST) */

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
si_pcie_ltr_war(si_t *sih)
{
#if !defined(BCMDONGLEHOST)
	si_info_t *sii = SI_INFO(sih);

	if (PCIE_GEN2(sii))
		pcie_ltr_war(sii->pch, si_pcieltrenable(sih, 0, 0));
#endif /* !defined(BCMDONGLEHOST */
}

void
si_pcie_hw_LTR_war(si_t *sih)
{
#if !defined(BCMDONGLEHOST)
	si_info_t *sii = SI_INFO(sih);

	if (PCIE_GEN2(sii))
		pcie_hw_LTR_war(sii->pch);
#endif /* !defined(BCMDONGLEHOST */
}

void
si_pciedev_reg_pm_clk_period(si_t *sih)
{
#if !defined(BCMDONGLEHOST)
	si_info_t *sii = SI_INFO(sih);

	if (PCIE_GEN2(sii))
		pciedev_reg_pm_clk_period(sii->pch);
#endif /* !defined(BCMDONGLEHOST */
}

void
si_pciedev_crwlpciegen2(si_t *sih)
{
#if !defined(BCMDONGLEHOST)
	si_info_t *sii = SI_INFO(sih);

	if (PCIE_GEN2(sii))
		pciedev_crwlpciegen2(sii->pch);
#endif /* !defined(BCMDONGLEHOST */
}

void
si_pcie_prep_D3(si_t *sih, bool enter_D3)
{
#if !defined(BCMDONGLEHOST)
	si_info_t *sii = SI_INFO(sih);

	if (PCIE_GEN2(sii))
		pciedev_prep_D3(sii->pch, enter_D3);
#endif /* !defined(BCMDONGLEHOST */
}

#if !defined(BCMDONGLEHOST)
uint
si_corereg_ifup(si_t *sih, uint core_id, uint regoff, uint mask, uint val)
{
	bool isup;
	volatile void *regs;
	uint origidx, ret_val, coreidx;

	/* Remember original core before switch to chipc */
	origidx = si_coreidx(sih);
	regs = si_setcore(sih, core_id, 0);
	BCM_REFERENCE(regs);
	ASSERT(regs != NULL);

	coreidx = si_coreidx(sih);

	isup = si_iscoreup(sih);
	if (isup == TRUE) {
		ret_val = si_corereg(sih, coreidx, regoff, mask, val);
	} else {
		ret_val = 0;
	}

	/* Return to original core */
	si_setcoreidx(sih, origidx);
	return ret_val;
}

/** this function is called from the BMAC during (re) initialisation */
void
si_lowpwr_opt(si_t *sih)
{
	uint mask, val;

	/* 4350 chip (all revision) related changes */
	if (BCM4350_CHIP(sih->chip) || BCM43602_CHIP(sih->chip)) {
		uint hosti = si_chip_hostif(sih);
		uint origidx = si_coreidx(sih);
		volatile void *regs;

		regs = si_setcore(sih, CC_CORE_ID, 0);
		BCM_REFERENCE(regs);
		ASSERT(regs != NULL);

		/* disable usb app clk */
		/* Can be done any time. If it is not USB, then do it. In case */
		/* of USB, do not write it */
		if (hosti != CHIP_HOSTIF_USBMODE && !BCM43602_CHIP(sih->chip)) {
			si_pmu_chipcontrol(sih, PMU_CHIPCTL5, (1 << USBAPP_CLK_BIT), 0);
		}
		/* disable pcie clks */
		if (hosti != CHIP_HOSTIF_PCIEMODE) {
			si_pmu_chipcontrol(sih, PMU_CHIPCTL5, (1 << PCIE_CLK_BIT), 0);
		}

		/* for 4350 derivatives enable armcr4 debug clock */
		/* if tap sel bit set */
		if (BCM4350_CHIP(sih->chip)) {
			uint32 tapsel = si_corereg(sih, SI_CC_IDX,
				OFFSETOF(chipcregs_t, jtagctrl), 0, 0)
				& JCTRL_TAPSEL_BIT;
			/* SWD/JTAG: if tap sel bit set, enable armcr4 debug clock */
			si_pmu_chipcontrol(sih, PMU_CHIPCTL5,
				(1 << ARMCR4_DBG_CLK_BIT),
				tapsel?(1 << ARMCR4_DBG_CLK_BIT):0);
		} else {
			/* disable armcr4 debug clk */
			/* Can be done anytime as long as driver is functional. */
			/* In TCL, dhalt commands needs to change to undo this */
			switch (CHIPID(sih->chip)) {
				CASE_BCM43602_CHIP:
					si_pmu_chipcontrol(sih, PMU_CHIPCTL3,
						PMU43602_CC3_ARMCR4_DBG_CLK, 0);
					break;
				case BCM4369_CHIP_GRPID:
					{
						uint32 tapsel =	si_corereg(sih, SI_CC_IDX,
							OFFSETOF(chipcregs_t, jtagctrl), 0, 0)
							& JCTRL_TAPSEL_BIT;
						/* SWD: if tap sel bit set, */
						/* enable armcr4 debug clock */
						si_pmu_chipcontrol(sih, PMU_CHIPCTL5,
							(1 << ARMCR4_DBG_CLK_BIT),
							tapsel?(1 << ARMCR4_DBG_CLK_BIT):0);
					}
					break;
				default:
					si_pmu_chipcontrol(sih, PMU_CHIPCTL5,
						(1 << ARMCR4_DBG_CLK_BIT), 0);
					break;
			}
		}

		if (BCM4350_CHIP(sih->chip)) {
			/* For 4350, shut off ch-6(pcie_tl_clk) only. */
			/* NOTE: There is a backup option to use tl_clk for PCIE, but not used. */

			/* BBPLL ch-5(sample-sync-clk) is needed for: */
			/*     - high speed ARM mode in 4350C0 */
			/*     - offload driver running in PCIe mode */
			/* For now, leave it enabled in all modes */
			mask = (0x1 << PCIE_TL_CLK_BIT);
			val = (0x1 << PCIE_TL_CLK_BIT);
			si_pmu_pllcontrol(sih, PMU1_PLL0_PLLCTL0, mask, val);
			si_pmu_pllupd(sih);
		} else if (BCM43602_CHIP(sih->chip)) {
			/* configure open loop PLL parameters, open loop is used during S/R */
			val = (3 << PMU1_PLL0_PC1_M1DIV_SHIFT) | (6 << PMU1_PLL0_PC1_M2DIV_SHIFT) |
			      (6 << PMU1_PLL0_PC1_M3DIV_SHIFT) | (8 << PMU1_PLL0_PC1_M4DIV_SHIFT);
			si_pmu_pllcontrol(sih, PMU1_PLL0_PLLCTL4, ~0, val);
			si_pmu_pllupd(sih);
			si_pmu_chipcontrol(sih, PMU_CHIPCTL2,
			  PMU43602_CC2_PCIE_CLKREQ_L_WAKE_EN |  PMU43602_CC2_PMU_WAKE_ALP_AVAIL_EN,
			  PMU43602_CC2_PCIE_CLKREQ_L_WAKE_EN |  PMU43602_CC2_PMU_WAKE_ALP_AVAIL_EN);
		}

		/* Return to original core */
		si_setcoreidx(sih, origidx);
	}
} /* si_lowpwr_opt */
#endif /* !defined(BCMDONGLEHOST) */

#if defined(AXI_TIMEOUTS) || defined(BCM_BACKPLANE_TIMEOUT)
uint32
si_clear_backplane_to_per_core(si_t *sih, uint coreid, uint coreunit, void * wrap)
{
	if ((CHIPTYPE(sih->socitype) == SOCI_AI) ||
		(CHIPTYPE(sih->socitype) == SOCI_DVTBUS)) {
		return ai_clear_backplane_to_per_core(sih, coreid, coreunit, wrap);
	}

	return AXI_WRAP_STS_NONE;
}
#endif /* AXI_TIMEOUTS || BCM_BACKPLANE_TIMEOUT */

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
BCMATTACHFN(si_update_backplane_timeouts)(si_t *sih, bool enable, uint32 timeout_exp, uint32 cid)
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
	uint32 axi_to = 0;

	/* Enable only for AXI */
	if ((CHIPTYPE(sih->socitype) != SOCI_AI) &&
		(CHIPTYPE(sih->socitype) != SOCI_DVTBUS)) {
		return;
	}

	if (BCM4347_CHIP(sih->chip)) {
		axi_to = AXI_TO_VAL_4347;
	}
	else {
		axi_to = AXI_TO_VAL;
	}

	/* All required slave wrappers are added in ai_scan */
	ai_update_backplane_timeouts(sih, TRUE, axi_to, 0);

#ifdef DISABLE_PCIE2_AXI_TIMEOUT
	ai_update_backplane_timeouts(sih, FALSE, 0, PCIE_CORE_ID);
	ai_update_backplane_timeouts(sih, FALSE, 0, PCIE2_CORE_ID);
#endif // endif

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

void
si_pll_sr_reinit(si_t *sih)
{
#if !defined(BCMDONGLEHOST) && !defined(DONGLEBUILD)
	osl_t *osh = si_osh(sih);
	si_info_t *sii = SI_INFO(sih);
	uint32 data;

	/* disable PLL open loop operation */
	switch (CHIPID(sih->chip)) {
		case BCM43602_CHIP_ID:
			/* read back the pll openloop state */
			data = si_pmu_pllcontrol(sih, PMU1_PLL0_PLLCTL8, 0, 0);
			/* check current pll mode */
			if ((data & PMU1_PLLCTL8_OPENLOOP_MASK) == 0) {
				/* no POR; don't required pll and saverestore init */
				return;
			}
			si_pmu_pll_init(sih, osh, sii->xtalfreq);
			si_pmu_pllcontrol(sih, PMU1_PLL0_PLLCTL8, PMU1_PLLCTL8_OPENLOOP_MASK, 0);
			si_pmu_pllupd(sih);
			/* allow PLL to settle after config PLL for closeloop operation */
			OSL_DELAY(100);
			break;
		case BCM4350_CHIP_ID:
			/* read back the pll openloop state */
			data = si_pmu_chipcontrol(sih, CHIPCTRLREG1, 0, 0);
			/* current mode is openloop (possible POR) */
			if ((data & PMU_PLL4350_OPENLOOP_MASK) == 0) {
				/* no POR; don't required pll and saverestore init */
				return;
			}
			si_pmu_pll_init(sih, osh, sii->xtalfreq);
			/* Enable BBPLL power down */
			si_pmu_chipcontrol(sih, CHIPCTRLREG1, 0x10, 0x10);

			/* Disable BBPLL open-loop mode */
			si_pmu_chipcontrol(sih, CHIPCTRLREG1, PMU_PLL4350_OPENLOOP_MASK, 0);

			/* BBPLL closed loop lock time = 1 (default=4) */
			si_pmu_chipcontrol(sih, CHIPCTRLREG1, 0xf0000, 0x10000);
			break;
		default:
			/* any unsupported chip bail */
			return;
	}
	si_pmu_init(sih, osh);
	si_pmu_chip_init(sih, osh);
#if defined(BCMPMU_STATS)
	if (PMU_STATS_ENAB()) {
		si_pmustatstimer_init(sih);
	}
#endif /* BCMPMU_STATS */
#if defined(SR_ESSENTIALS)
	/* Module can be power down during D3 state, thus
	 * needs this before si_pmu_res_init() to use sr_isenab()
	 * Full dongle may not need to reinit saverestore
	 */
	if (SR_ESSENTIALS_ENAB()) {
		sr_save_restore_init(sih);
	}
#endif /* SR_ESSENTIALS */
	si_pmu_res_init(sih, sii->osh);
	si_pmu_swreg_init(sih, osh);
	si_lowpwr_opt(sih);
#endif /* !BCMDONGLEHOST && !DONGLEBUILD */
} /* si_pll_sr_reinit */

#if !defined(BCMDONGLEHOST)
#if (!defined(_CFE_) && !defined(_CFEZ_)) || defined(CFG_WL)
void
si_config_53573_d11_oob(si_t *sih, uint coreid)
{
	uint save_idx;

	ASSERT(coreid == D11_CORE_ID);
	save_idx = si_coreidx(sih);

	// Disable DDR interrupts
	si_setcore(sih, CMEM_CORE_ID, 0);
	si_wrapperreg(sih, AI_OOBSELOUTA30, 0x000000ff, 0x0);

	// Route d11-core1 interrupts through core 7(DDR)
	si_setcore(sih, coreid, 1);
	si_wrapperreg(sih, AI_OOBSELOUTA30, 0x0000000f, 0x7);
	si_setcoreidx(sih, save_idx);
}
#endif /* (!_CFE_ && !_CFEZ_) || CFG_WL */
#endif /* BCMDONGLEHOST */

/* Programming d11 core oob  settings for 4364
 * WARs for HW4364-237 and HW4364-166
*/
void
si_config_4364_d11_oob(si_t *sih, uint coreid)
{
	uint save_idx;

	save_idx = si_coreidx(sih);
	si_setcore(sih, coreid, 0);
	si_wrapperreg(sih, AI_OOBSELINC30, ~0, 0x81828180);
	si_wrapperreg(sih, AI_OOBSELINC74, ~0, 0x87868183);
	si_wrapperreg(sih, AI_OOBSELOUTB74, ~0, 0x84858484);
	si_setcore(sih, coreid, 1);
	si_wrapperreg(sih, AI_OOBSELINC30, ~0, 0x81828180);
	si_wrapperreg(sih, AI_OOBSELINC74, ~0, 0x87868184);
	si_wrapperreg(sih, AI_OOBSELOUTB74, ~0, 0x84868484);
	si_setcoreidx(sih, save_idx);
}

void
si_pll_closeloop(si_t *sih)
{
#if !defined(BCMDONGLEHOST) && !defined(DONGLEBUILD) || defined(SAVERESTORE)
	uint32 data;

	/* disable PLL open loop operation */
	switch (CHIPID(sih->chip)) {
#if !defined(BCMDONGLEHOST) && !defined(DONGLEBUILD)
		/* XXX don't apply those changes to FULL DONGLE mode since the
		 * behaviour was not verified
		 */
		case BCM43602_CHIP_ID:
			/* read back the pll openloop state */
			data = si_pmu_pllcontrol(sih, PMU1_PLL0_PLLCTL8, 0, 0);
			/* current mode is openloop (possible POR) */
			if ((data & PMU1_PLLCTL8_OPENLOOP_MASK) != 0) {
				si_pmu_pllcontrol(sih, PMU1_PLL0_PLLCTL8,
					PMU1_PLLCTL8_OPENLOOP_MASK, 0);
				si_pmu_pllupd(sih);
				/* allow PLL to settle after config PLL for closeloop operation */
				OSL_DELAY(100);
			}
			break;
		case BCM4350_CHIP_ID:
			/* read back the pll openloop state */
			data = si_pmu_chipcontrol(sih, CHIPCTRLREG1, 0, 0);
			/* current mode is openloop (possible POR) */
			if ((data & PMU_PLL4350_OPENLOOP_MASK) != 0) {
				/* Enable BBPLL power down */
				si_pmu_chipcontrol(sih, CHIPCTRLREG1, 0x10, 0x10);

				/* Disable BBPLL open-loop mode */
				si_pmu_chipcontrol(sih, CHIPCTRLREG1, PMU_PLL4350_OPENLOOP_MASK, 0);

				/* BBPLL closed loop lock time = 1 (default=4) */
				si_pmu_chipcontrol(sih, CHIPCTRLREG1, 0xf0000, 0x10000);
			}
			break;
#endif /* !BCMDONGLEHOST && !DONGLEBUILD */
		case BCM4347_CHIP_GRPID:
		case BCM4369_CHIP_GRPID:
			si_pmu_chipcontrol(sih, PMU_CHIPCTL1,
				PMU_CC1_ENABLE_CLOSED_LOOP_MASK, PMU_CC1_ENABLE_CLOSED_LOOP);
			break;
		default:
			/* any unsupported chip bail */
			return;
	}
#endif /* !BCMDONGLEHOST && !DONGLEBUILD || SAVERESTORE */
} /* si_pll_closeloop */

#if !defined(BCMDONGLEHOST)
/*
* Utility function that counts number of times the chip
* went to deepsleep and the total duration of the deepsleep.
* This function gets called everytime
* after ARM wakes up and also everytime before ARM
* goes to WFI.
*
* 4350: Right after wakeup, arm_wakeup=TRUE
*   Following register is checked and cleared
*
*   PMU register RetentionControl (Chipcommon Offset 0x670)
*   Name    Bits         Description
*   MemRetentionSleepLog              29
*   This field is set to 1 when scan retention memory's sleep/pm signal
*   is asserted. Writing 1 to this field clears the bit.
*
* Right before WFI, arm_wakeup=FALSE
*  Sleep duration is reset at this time
*/
void
si_deepsleep_count(si_t *sih, bool arm_wakeup)
{
#ifdef BCMPCIEDEV
	si_info_t *sii = SI_INFO(sih);
#endif /* BCMPCIEDEV */

	switch (CHIPID(sih->chip)) {
	case BCM4350_CHIP_ID:
		if (arm_wakeup) {
			if (si_corereg(sih,
				SI_CC_IDX,
				OFFSETOF(chipcregs_t, retention_ctl),
					0, 0) & RCTL_MEM_RET_SLEEP_LOG_MASK) {
				si_corereg(sih,
					SI_CC_IDX,
					OFFSETOF(chipcregs_t, retention_ctl),
					RCTL_MEM_RET_SLEEP_LOG_MASK, RCTL_MEM_RET_SLEEP_LOG_MASK);
			}
		}
#ifdef BCMPCIEDEV
		if (PCIE_GEN2(sii))
			pciedev_sr_stats(sii->pch, arm_wakeup);
#endif /* BCMPCIEDEV */
		break;
	default:
		SI_ERROR(("deepsleep count not supported for 0x%04x yet\n", CHIPID(sih->chip)));
		ASSERT(0);
		break;
	}
}

/*
* API used to configure the GCI registers for wlan femctrl .
*/

void
si_gci_set_femctrl(si_t *sih, osl_t *osh, bool set)
{
}

void
si_gci_set_femctrl_mask_ant01(si_t *sih, osl_t *osh, bool set)
{
}

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

uint8
si_getspurmode(si_t *sih)
{
	si_info_t *sii = SI_INFO(sih);
	return sii->spurmode;
}

uint32
si_xtalfreq(si_t *sih)
{
	si_info_t *sii = SI_INFO(sih);
	return sii->xtalfreq;
}

uint32
si_get_openloop_dco_code(si_t *sih)
{
	si_info_t *sii = SI_INFO(sih);
	return sii->openloop_dco_code;
}

void
si_set_openloop_dco_code(si_t *sih, uint32 _openloop_dco_code)
{
	si_info_t *sii = SI_INFO(sih);
	sii->openloop_dco_code = _openloop_dco_code;
}

#ifdef DONGLEBUILD
uint32
BCMATTACHFN(si_wrapper_dump_buf_size)(si_t *sih)
{
	if (CHIPTYPE(sih->socitype) == SOCI_AI)
		return ai_wrapper_dump_buf_size(sih);
	return 0;
}

uint32
si_wrapper_dump_binary(si_t *sih, uchar *p)
{
	if (CHIPTYPE(sih->socitype) == SOCI_AI)
		return ai_wrapper_dump_binary(sih, p);
	return 0;
}

#if defined(AWD_EXT_TRAP)
uint32
si_wrapper_dump_last_timeout(si_t *sih, uint32 *error, uint32 *core, uint32 *ba, uchar *p)
{
	if (CHIPTYPE(sih->socitype) == SOCI_AI)
		return ai_wrapper_dump_last_timeout(sih, error, core, ba, p);
	return 0;
}
#endif /* EXT_TRAP */
#endif /* DONGLEBUILD */

#endif /* !BCMDONGLEHOST */

#if defined(BCMSRPWR) && !defined(BCMSRPWR_DISABLED)
bool _bcmsrpwr = TRUE;
#else
bool _bcmsrpwr = FALSE;
#endif // endif

uint32
si_srpwr_request(si_t *sih, uint32 mask, uint32 val)
{
	uint32 r, offset = OFFSETOF(chipcregs_t, powerctl); /* Same 0x1e8 per core */
	uint cidx = (BUSTYPE(sih->bustype) == SI_BUS) ? SI_CC_IDX : sih->buscoreidx;
	uint32 mask2 = mask;
	uint32 val2 = val;
	volatile uint32 *fast_srpwr_addr = (volatile uint32 *)((uintptr)SI_ENUM_BASE(sih)
					 + (uintptr)offset);
	if (mask || val) {
		mask <<= SRPWR_REQON_SHIFT;
		val  <<= SRPWR_REQON_SHIFT;

		/* Return if requested power request is already set */
		if (BUSTYPE(sih->bustype) == SI_BUS) {
			r = R_REG(OSH_NULL, fast_srpwr_addr);
		} else {
			r = si_corereg(sih, cidx, offset, 0, 0);
		}

		if ((r & mask) == val) {
			return r;
		}

		r = (r & ~mask) | val;

		if (BUSTYPE(sih->bustype) == SI_BUS) {
			W_REG(OSH_NULL, fast_srpwr_addr, r);
			r = R_REG(OSH_NULL, fast_srpwr_addr);
		} else {
			r = si_corereg(sih, cidx, offset, ~0, r);
		}

		if (val2) {
			if ((r & (mask2 << SRPWR_STATUS_SHIFT)) ==
			(val2 << SRPWR_STATUS_SHIFT)) {
				return r;
			}
			si_srpwr_stat_spinwait(sih, mask2, val2);
		}
	} else {
		if (BUSTYPE(sih->bustype) == SI_BUS) {
			r = R_REG(OSH_NULL, fast_srpwr_addr);
		} else {
			r = si_corereg(sih, cidx, offset, 0, 0);
		}
	}

	return r;
}

uint32
si_srpwr_stat_spinwait(si_t *sih, uint32 mask, uint32 val)
{
	uint32 r, offset = OFFSETOF(chipcregs_t, powerctl); /* Same 0x1e8 per core */
	uint cidx = (BUSTYPE(sih->bustype) == SI_BUS) ? SI_CC_IDX : sih->buscoreidx;
	volatile uint32 *fast_srpwr_addr = (volatile uint32 *)((uintptr)SI_ENUM_BASE(sih)
					 + (uintptr)offset);

	ASSERT(mask);
	ASSERT(val);

	/* spinwait on pwrstatus */
	mask <<= SRPWR_STATUS_SHIFT;
	val <<= SRPWR_STATUS_SHIFT;

	if (BUSTYPE(sih->bustype) == SI_BUS) {
		SPINWAIT(((R_REG(OSH_NULL, fast_srpwr_addr) & mask) != val),
			PMU_MAX_TRANSITION_DLY);
		r = R_REG(OSH_NULL, fast_srpwr_addr) & mask;
		ASSERT(r == val);
	} else {
		SPINWAIT(((si_corereg(sih, cidx, offset, 0, 0) & mask) != val),
			PMU_MAX_TRANSITION_DLY);
		r = si_corereg(sih, cidx, offset, 0, 0) & mask;
		ASSERT(r == val);
	}

	r = (r >> SRPWR_STATUS_SHIFT) & SRPWR_DMN_ALL_MASK;

	return r;
}

uint32
si_srpwr_stat(si_t *sih)
{
	uint32 r, offset = OFFSETOF(chipcregs_t, powerctl); /* Same 0x1e8 per core */
	uint cidx = (BUSTYPE(sih->bustype) == SI_BUS) ? SI_CC_IDX : sih->buscoreidx;

	r = si_corereg(sih, cidx, offset, 0, 0);
	r = (r >> SRPWR_STATUS_SHIFT) & SRPWR_DMN_ALL_MASK;

	return r;
}

uint32
si_srpwr_domain(si_t *sih)
{
	uint32 r, offset = OFFSETOF(chipcregs_t, powerctl); /* Same 0x1e8 per core */
	uint cidx = (BUSTYPE(sih->bustype) == SI_BUS) ? SI_CC_IDX : sih->buscoreidx;

	r = si_corereg(sih, cidx, offset, 0, 0);
	r = (r >> SRPWR_DMN_SHIFT) & SRPWR_DMN_ALL_MASK;

	return r;
}

/* Utility API to read/write the raw registers with absolute address.
 * This function can be invoked from either FW or host driver.
 */
uint32
si_raw_reg(si_t *sih, uint32 reg, uint32 val, uint32 wrire_req)
{
	si_info_t *sii = SI_INFO(sih);
	uint32 address_space = reg & ~0xFFF;
	volatile uint32 * addr = (void*)(uintptr)(reg);
	uint32 prev_value = 0;
	uint32 cfg_reg = 0;

	if (sii == NULL) {
		return 0;
	}

	/* No need to translate the absolute address on SI bus */
	if (BUSTYPE(sih->bustype) == SI_BUS) {
		goto skip_cfg;
	}

	/* This API supports only the PCI host interface */
	if (BUSTYPE(sih->bustype) != PCI_BUS) {
		return ID32_INVALID;
	}

	if (PCIE_GEN2(sii)) {
		/* Use BAR0 Secondary window is PCIe Gen2.
		 * Set the secondary BAR0 Window to current register of interest
		 */
		addr = (volatile uint32*)(((volatile uint8*)sii->curmap) +
			PCI_SEC_BAR0_WIN_OFFSET + (reg & 0xfff));
		cfg_reg = PCIE2_BAR0_CORE2_WIN;

	} else {
		/* PCIe Gen1 do not have secondary BAR0 window.
		 * reuse the BAR0 WIN2
		 */
		addr = (volatile uint32*)(((volatile uint8*)sii->curmap) +
			PCI_BAR0_WIN2_OFFSET + (reg & 0xfff));
		cfg_reg = PCI_BAR0_WIN2;
	}

	prev_value = OSL_PCI_READ_CONFIG(sii->osh, cfg_reg, 4);

	if (prev_value != address_space) {
		OSL_PCI_WRITE_CONFIG(sii->osh, cfg_reg,
			sizeof(uint32), address_space);
	} else {
		prev_value = 0;
	}

skip_cfg:
	if (wrire_req) {
		W_REG(sii->osh, addr, val);
	} else {
		val = R_REG(sii->osh, addr);
	}

	if (prev_value) {
		/* Restore BAR0 WIN2 for PCIE GEN1 devices */
		OSL_PCI_WRITE_CONFIG(sii->osh,
			cfg_reg, sizeof(uint32), prev_value);
	}

	return val;
} /* si_raw_reg */

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

uint8
si_lhl_ps_mode(si_t *sih)
{
	si_info_t *sii = SI_INFO(sih);
	return sii->lhl_ps_mode;
}
