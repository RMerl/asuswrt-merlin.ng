/** @file pcie_core.c
 *
 * Contains PCIe related functions that are shared between different driver models (e.g. firmware
 * builds, NIC builds, DHD builds), in order to avoid code duplication.
 *
 * Copyright (C) 2025, Broadcom. All Rights Reserved.
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
 * $Id: pcie_core.c 846943 2024-10-25 14:35:51Z $
 */

#include <bcm_cfg.h>
#include <typedefs.h>
#include <bcmutils.h>
#include <bcmdefs.h>
#include <osl.h>
#include <siutils.h>
#include <hndsoc.h>
#include <sbchipc.h>
#include <bcmdevs.h>
#include <hndpmu.h>

#if defined(LINUX) || defined(linux)
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/pci.h>
#endif /* LINUX || linux */

#if defined(BCA_CPEROUTER)			/* means: perforce source files present */
#include <bcm_pcie.h>				/* defines #BCM_PCIE_MUTEX_FLAG_MASK */
#endif /* BCA_CPEROUTER */

#include <pcicfg.h>
#include <pcie_core.h>

/* local prototypes */
struct pcie_reg_to_str {
	int32 minrev;				/** oldest core rev containing this register */
	const char *name;			/** register name */
	uint32 offset;
};

/* local variables */
static struct pcie_reg_to_str pcie_ep_cmn_regs[] = { /** PCIe EP CORE enumeration space registers */
	{  1, "control",	PCIE_CORE_REG_CONTROL},
	{  1, "iostatus",	PCIE_CORE_REG_IOSTATUS},
	{  1, "bitstatus",	PCIE_CORE_REG_BITSTATUS},
	{  1, "goiosel",	PCIE_CORE_REG_GPIO_SEL},
	{  1, "gpioout",	PCIE_CORE_REG_GPIO_OUT_EN},
	{  1, "intstatus",	PCIE_CORE_REG_INT_STATUS},
	{  1, "intmask",	PCIE_CORE_REG_INT_MASK},
	{  1, "sbpciemb",	PCIE_CORE_REG_SB_PCIE_MB},
	{  1, "errlog",		PCIE_CORE_REG_ERRLOG},
	{  1, "erraddr",	PCIE_CORE_REG_ERR_ADDR},
	{  1, "mb_intr",	PCIE_CORE_REG_MB_INTR},

	{133, "lock_addr_lo",	PCIE_CORE_REG_LOCK_ADDR_LO},
	{133, "lock_addr_hi",	PCIE_CORE_REG_LOCK_ADDR_HI},
	{133, "lock_mode",	PCIE_CORE_REG_LOCK_MODE},
	{133, "lock_polling",	PCIE_CORE_REG_LOCK_POLLING},

	{  1, "s2ptrans2",	PCIE_CORE_REG_S2P_TRANS2},
	{  1, "s2ptrans2u",	PCIE_CORE_REG_S2P_TRANS2_UPPER},
	{  1, "s2ptrans3",	PCIE_CORE_REG_S2P_TRANS3},
	{  1, "s2ptrans3u",	PCIE_CORE_REG_S2P_TRANS3_UPPER},
	{  1, "s2ptrans0",	PCIE_CORE_REG_S2P_TRANS0},
	{  1, "s2ptrans1",	PCIE_CORE_REG_S2P_TRANS1},
	{  1, "s2ptransbig",	PCIE_CORE_REG_S2P_TRANS_BIG},
	{  1, "s2ptrans0u",	PCIE_CORE_REG_S2P_TRANS0_UPPER},
	{  1, "s2ptrans1u",	PCIE_CORE_REG_S2P_TRANS1_UPPER},
	{  1, "s2ptrans4",	PCIE_CORE_REG_S2P_TRANS4},
	{  1, "s2ptrans4u",	PCIE_CORE_REG_S2P_TRANS4_UPPER},
	{  1, "errheaderlog1",	PCIE_CORE_REG_ERRORHEADERLOG1},
	{  1, "errheaderlog2",	PCIE_CORE_REG_ERRORHEADERLOG2},
	{  1, "errheaderlog3",	PCIE_CORE_REG_ERRORHEADERLOG3},
	{  1, "errheaderlog4",	PCIE_CORE_REG_ERRORHEADERLOG4},
	{  1, "hmapvioaddrlo",	PCIE_CORE_REG_HMAPVIOLATIONADDRLOWER},
	{  1, "hmapvioaddrhi",	PCIE_CORE_REG_HMAPVIOLATIONADDRUPPER},
	{  1, "hmapvioinfo",	PCIE_CORE_REG_HMAPVIOLATIONINFO},
	{  0}
};

struct pcie_reg_to_str pcie_ep_cfg_cmn_regs[] = {	/** PCIe EP configuration space registers */
	{  1, "status_cmd",		PCIECFGREG_STATUS_CMD},
	{  1, "pm_csr",			PCIECFGREG_PM_CSR},
	{  1, "msi_cap",		PCIECFGREG_MSI_CAP},
	{  1, "msi_addr_l",		PCIECFGREG_MSI_ADDR_L},
	{  1, "msi_addr_h",		PCIECFGREG_MSI_ADDR_H},
	{  1, "msi_data",		PCIECFGREG_MSI_DATA},
	{  1, "sprom_ctrl",		PCIECFGREG_SPROM_CTRL},
	{  1, "link_cap",		PCIECFGREG_LINKCAP},
	{  1, "link_status_ctrl",	PCIECFGREG_LINK_STATUS_CTRL},
	{  1, "link_status_ctrl2",	PCIECFGREG_LINK_STATUS_CTRL2},
	{  1, "dev_ctrl",		PCIECFGREG_DEV_STATUS_CTRL},
	{  1, "device_status_control_2", PCIECFGREG_DEV_STATUS_CTRL2},
	{  1, "adv_err_cap",		PCIE_CFG_ADV_ERR_CAP},
	{  1, "uc_err_status",		PCIE_CFG_UC_ERR_STS},
	{  1, "ucorr_err_mask",		PCIE_CFG_UC_ERR_MASK},
	{  1, "ucorr_err_sevr",		PCIE_CFG_UNCOR_ERR_SERV},
	{  1, "corr_err_status",	PCIE_CFG_CORR_ERR_STS},
	{  1, "corr_err_mask",		PCIE_CFG_CORR_ERR_MASK},
	{  1, "adv_err_cap_control",	PCIE_CFG_ADV_ERR_CTRL},
	{  1, "header_log1",		PCIE_CFG_HDR_LOG1},
	{  1, "header_log2",		PCIE_CFG_HDR_LOG2},
	{  1, "header_log3",		PCIE_CFG_HDR_LOG3},
	{  1, "header_log4",		PCIE_CFG_HDR_LOG4},
	{  1, "rbar_ctrl",		PCIECFGREG_RBAR_CTRL},
	{  1, "PML1_sub_capID",		PCIE_CFG_PML1_SUB_CAP_ID},
	{  1, "PML1_sub_Cap_reg",	PCIE_CFG_PML1_SUB_CAP_REG},
	{  1, "PML1_sub_control1",	PCIECFGREG_PML1_SUB_CTRL1},
	{  1, "PML1_sub_control2",	PCIECFGREG_PML1_SUB_CTRL2},
	{  1, "reg_bar2_config",	PCIECFGREG_REG_BAR2_CONFIG},
	{  1, "reg_bar3_config",	PCIECFGREG_REG_BAR3_CONFIG},
	{  1, "tl_control_0",		PCIECFGREG_TL_CTRL0},
	{  1, "tl_control_5",		PCIE_CFG_TL_CTRL_5},
	{  1, "pdl_ctrl1",		PCIECFGREG_PDL_CTRL1},
	{  1, "pdl_iddq",		PCIECFGREG_PDL_IDDQ},
	{  1, "reg_phy_ctrl7",		PCIECFGREG_REG_PHY_CTL7},
	{  0}
};

/* PCIe EXT Config registers */
struct pcie_reg_to_str pcie_ep_cfg_ext_regs[] = {
	{  1, "phy_err_attn_vec",	PCIE_CFG_PHY_ERR_ATT_VEC},
	{  1, "phy_err_attn_mask",	PCIE_CFG_PHY_ERR_ATT_MASK},
	{ 0}
};

/* PCIe Root Complex AER and TL (error logging) registers */
struct pcie_reg_to_str pcie_rc_aer_tl_regs[] = {
	{  1, "status_cmd",		PCIECFGREG_STATUS_CMD},
	{  1, "adv_err_cap",		PCIE_CFG_ADV_ERR_CAP},
	{  1, "uc_err_status",		PCIE_CFG_UC_ERR_STS},
	{  1, "ucorr_err_mask",		PCIE_CFG_UC_ERR_MASK},
	{  1, "ucorr_err_sevr",		PCIE_CFG_UNCOR_ERR_SERV},
	{  1, "corr_err_status",	PCIE_CFG_CORR_ERR_STS},
	{  1, "corr_err_mask",		PCIE_CFG_CORR_ERR_MASK},
	{  1, "adv_err_cap_control",	PCIE_CFG_ADV_ERR_CTRL},
	{  1, "header_log1",		PCIE_CFG_HDR_LOG1},
	{  1, "header_log2",		PCIE_CFG_HDR_LOG2},
	{  1, "header_log3",		PCIE_CFG_HDR_LOG3},
	{  1, "header_log4",		PCIE_CFG_HDR_LOG4},
	{  1, "root_error_command",	PCIE_CFG_ROOT_ERROR_COMMAND},
	{  1, "root_error_status",	PCIE_CFG_ROOT_ERROR_STATUS},
	{  1, "root_err_id",		PCIE_CFG_ROOT_ERROR_ID},
	{  1, "tl_control_5",		PCIE_CFG_TL_CTRL_5},
	{  1, "tl_hdr_fc_st",		PCIE_CFG_TL_HDR_FC_ST},
	{  1, "tl_tgt_ctdt_st",		PCIE_CFG_TL_TGT_CTDT_ST},
	{  1, "dl_dlattn_vec",		PCIE_CFG_DL_DLATTN_VEC},
	{  1, "misc_ctrl",		PCIE_CFG_MISC_CTRL},
	{  1, "rc_bad_addr_low",	PCIE_CFG_RC_BAD_ADDRESS_LOW},
	{  1, "rc_bad_addr_high",	PCIE_CFG_RC_BAD_ADDRESS_HIGH},
	{  1, "rc_bad_data",		PCIE_CFG_RC_RC_BAD_DATA},
	{  1, "pcie_status",		PCIE_CFG_RC_PCIE_STATUS},
	{  1, "ubus_ctrl",		PCIE_CFG_RC_UBUS_CTRL},
	{  1, "ubus_status",		PCIE_CFG_RC_UBUS_STATUS},
	{  1, "ubus_status2",		PCIE_CFG_RC_UBUS_STATUS2},
	{  1, "intr2_cpu_status",	PCIE_CFG_RC_INTR2_CPU_STATUS},
	{  1, "intr2_pci_status",	PCIE_CFG_RC_INTR2_PCI_STATUS},
	{  1, "ubus2_cpu_status",	PCIE_CFG_RC_UBUS2_CPU_STATUS},
	{  1, "ubus2_pci_status",	PCIE_CFG_RC_UBUS2_PCI_STATUS},
	{ 0}
};

/* function definitions */

#ifdef BCMDRIVER

void pcie_watchdog_reset(osl_t *osh, si_t *sih, sbpcieregs_t *sbpcieregs)
{
	uint32 val, i, lsc;
	uint32 cfg_offset[] = {PCIECFGREG_STATUS_CMD, PCIECFGREG_PM_CSR,
		PCIECFGREG_MSI_CAP, PCIECFGREG_MSI_ADDR_L,
		PCIECFGREG_MSI_ADDR_H, PCIECFGREG_MSI_DATA,
		PCIECFGREG_LINK_STATUS_CTRL2, PCIECFGREG_RBAR_CTRL,
		PCIECFGREG_PML1_SUB_CTRL1, PCIECFGREG_REG_BAR2_CONFIG,
		PCIECFGREG_REG_BAR3_CONFIG};
	sbpcieregs_t *pcie = NULL;
	uint32 origidx;
#ifdef BCM_ROUTER
	uint32 pmuresetctl = 0;
	/* To retain NIC mode PCIE integrity through a "rmmod wl.ko",
	 * ensure PMU-reset is excluded from the chip reset below.
	 */
	switch (CHIPID(sih->chip)) {
	CASE_BCM6710_CHIP:
	CASE_BCM43684_CHIP:
		pmuresetctl = si_pmu_get_resetcontrol(sih);
		si_pmu_set_resetcontrol(sih, PCTL_RESETCTL_BP_ONLY);
		break;
	default:
		/* Do nothing */
		break;
	}
#endif /* BCM_ROUTER */

	origidx = si_coreidx(sih);
	/* Switch to PCIE2 core */
	pcie = (sbpcieregs_t *)si_setcore(sih, PCIE2_CORE_ID, 0);
	BCM_REFERENCE(pcie);
	ASSERT(pcie != NULL);

	/* Disable/restore ASPM Control to protect the watchdog reset */
	W_REG(osh, &sbpcieregs->configaddr, PCIECFGREG_LINK_STATUS_CTRL);
	lsc = R_REG(osh, &sbpcieregs->configdata);
	val = lsc & (~PCIE_ASPM_ENAB);
	W_REG(osh, &sbpcieregs->configdata, val);

	si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, watchdog), ~0, 4);
#ifdef BCMQT
	OSL_DELAY(1000000);
#else
	OSL_DELAY(100000);
#endif /* BCMQT */

	W_REG(osh, &sbpcieregs->configaddr, PCIECFGREG_LINK_STATUS_CTRL);
	W_REG(osh, &sbpcieregs->configdata, lsc);

	if (sih->buscorerev <= 13) {
		/* Write configuration registers back to the shadow registers
		 * cause shadow registers are cleared out after watchdog reset.
		 */
		for (i = 0; i < ARRAYSIZE(cfg_offset); i++) {
			W_REG(osh, &sbpcieregs->configaddr, cfg_offset[i]);
			val = R_REG(osh, &sbpcieregs->configdata);
			W_REG(osh, &sbpcieregs->configdata, val);
		}
	}
	si_setcoreidx(sih, origidx);

#ifdef BCM_ROUTER
	/* Restore PMU reset-control to prior value */
	switch (CHIPID(sih->chip)) {
	CASE_BCM6710_CHIP:
	CASE_BCM43684_CHIP:
		si_pmu_set_resetcontrol(sih, pmuresetctl);
		break;
	default:
		/* Do nothing */
		break;
	}
#endif /* BCM_ROUTER */
}

/* CRWLPCIEGEN2-117 pcie_pipe_Iddq should be controlled
 * by the L12 state from MAC to save power by putting the
 * SerDes analog in IDDQ mode
 */
void  pcie_serdes_iddqdisable(osl_t *osh, si_t *sih, sbpcieregs_t *sbpcieregs)
{
	sbpcieregs_t *pcie = NULL;
	uint crwlpciegen2_117_disable = 0;
	uint32 origidx = si_coreidx(sih);

	crwlpciegen2_117_disable = PCIE_PipeIddqDisable0 | PCIE_PipeIddqDisable1;
	/* Switch to PCIE2 core */
	pcie = (sbpcieregs_t *)si_setcore(sih, PCIE2_CORE_ID, 0);
	BCM_REFERENCE(pcie);
	ASSERT(pcie != NULL);

	OR_REG(osh, &sbpcieregs->control,
		crwlpciegen2_117_disable);

	si_setcoreidx(sih, origidx);
}

void pcie_coherent_accenable(osl_t *osh, si_t *sih)
{
	sbpcieregs_t *pcie = NULL;
	uint32 val;
	uint32 origidx = si_coreidx(sih);

	if ((pcie = si_setcore(sih, PCIE2_CORE_ID, 0)) != NULL) {
		/* PCIe BAR1 and BAR2 coherent access enabled */
		W_REG(osh, &pcie->configaddr, PCIECFGREG_SPROM_CTRL);
		val = R_REG(osh, &pcie->configdata);
		val |= (PCIE_BAR1COHERENTACCEN | PCIE_BAR2COHERENTACCEN);
		W_REG(osh, &pcie->configdata, val);
	}

	si_setcoreidx(sih, origidx);
}

void pcie_getdevctl(osl_t *osh, si_t *sih, uint32 *devctl)
{
	sbpcieregs_t *pcie = NULL;
	uint32 origidx = si_coreidx(sih);

	*devctl = 0;
	if ((pcie = si_setcore(sih, PCIE2_CORE_ID, 0)) != NULL) {
		W_REG(osh, &pcie->configaddr, PCIECFGREG_DEVCONTROL);
		*devctl = R_REG(osh, &pcie->configdata);
	}

	si_setcoreidx(sih, origidx);
}

uint32 pcie_dmatxblen(osl_t *osh, si_t *sih)
{
	uint32 devctl;
	uint32 mrrs, burstlen;

	pcie_getdevctl(osh, sih, &devctl);
	mrrs = (devctl & PCIE_CAP_DEVCTRL_MRRS_MASK) >> PCIE_CAP_DEVCTRL_MRRS_SHIFT;
	switch (mrrs)
	{
		case PCIE_CAP_DEVCTRL_MRRS_128B:
			burstlen = DMA_BL_128;
			break;
		case PCIE_CAP_DEVCTRL_MRRS_256B:
			burstlen = DMA_BL_256;
			break;
		case PCIE_CAP_DEVCTRL_MRRS_512B:
			burstlen = DMA_BL_512;
			break;
		case PCIE_CAP_DEVCTRL_MRRS_1024B:
			burstlen = DMA_BL_1024;
			break;
		default:
			burstlen = DMA_BL_128;
			break;
	}

	if (ISSIM_ENAB(sih) && burstlen > DMA_BL_256) {
		// printf("=== CAUTION: Tx DMA BurstLen in pcie_dmatxblen is downsized "
		//	"from %d to %d per PCIE MRRS ===\n", burstlen, DMA_BL_256);
		burstlen = DMA_BL_256;
	}

	return burstlen;
}

uint32 pcie_dmarxblen(osl_t *osh, si_t *sih)
{
	uint32 devctl;
	uint32 mps, burstlen;

	pcie_getdevctl(osh, sih, &devctl);
	mps = (devctl & PCIE_CAP_DEVCTRL_MPS_MASK) >> PCIE_CAP_DEVCTRL_MPS_SHIFT;
	switch (mps)
	{
		case PCIE_CAP_DEVCTRL_MPS_128B:
			burstlen = DMA_BL_128;
			break;
		case PCIE_CAP_DEVCTRL_MPS_256B:
			burstlen = DMA_BL_256;
			break;
		case PCIE_CAP_DEVCTRL_MPS_512B:
			burstlen = DMA_BL_512;
			break;
		case PCIE_CAP_DEVCTRL_MPS_1024B:
			burstlen = DMA_BL_1024;
			break;
		default:
			burstlen = DMA_BL_128;
			break;
	}

	if (ISSIM_ENAB(sih) && burstlen > DMA_BL_256) {
		// printf("=== CAUTION: Rx DMA BurstLen in pcie_dmarxblen is downsized "
		//	"from %d to %d per PCIE MPS ===\n", burstlen, DMA_BL_256);
		burstlen = DMA_BL_256;
	}

	return burstlen;
}

void pcie_tl_control0(osl_t *osh, si_t *sih)
{
	uint32 val;
	sbpcieregs_t *pcie = NULL;
	uint32 origidx = si_coreidx(sih);

	if ((pcie = si_setcore(sih, PCIE2_CORE_ID, 0)) != NULL) {

		/* Setup MemRd & MemWr burst sizes */
		W_REG(osh, &pcie->configaddr, PCIECFGREG_TL_CTRL0);
		val = R_REG(osh, &pcie->configdata);

		/* Mask away MEMRD/WR_DW_CHK */
		val = val & 0xfffffc01;

		/* BAR0: max 1DW */
		val |= (0x1 << 3) | (0x1 << 1);
		/* BAR1 & BAR2: do not restrict 32DW/128B */
		val &= ~((0x1 << 9) | (0x1 << 8));

		W_REG(osh, &pcie->configdata, val);
	}

	si_setcoreidx(sih, origidx);
}

#if defined(BCM_PCIE_MUTEX_FLAG_MASK)
/**
 * Configures the client hwmutex for NIC or DHD. The mutex client hardware is part of the PCIe EP
 * core.
 *
 * @param[in] bc_out_addr  ICC broadcast out physical address. 0 means: ICC bcast not enabled.
 * @param[in] bc_in_addr   ICC broadcast in physical address
 */
static bool
pcie_config_hwmutex_nic_or_dhd(si_t *sih, osl_t *osh, u64 mutex_svr_addr,
                               u64 bc_out_addr, u64 bc_in_addr)
{
	sbpcieregs_t *regs;			/** PCIe EP enum space virtual address */
	uint savecore;
	const bool bc_en = (bc_out_addr != 0 || bc_in_addr != 0);

	if (PCIECOREREV(sih->buscorerev) < 133) {
		return FALSE;
	}

	savecore = si_coreidx(sih);
	regs = si_setcore(sih, PCIE2_CORE_ID, 0);
	ASSERT(regs != NULL);
	ASSERT((mutex_svr_addr & 0x1ULL) == 0); /* reserved regbits must be zero */
	W_REG(osh, &regs->u.pcie2.lock_addr_lo, (uint32)(mutex_svr_addr & 0xFFFFFFFF));
	W_REG(osh, &regs->u.pcie2.lock_addr_hi, (uint32)(mutex_svr_addr >> 32));
	W_REG(osh, &regs->u.pcie2.lock_mode,	/* ucode requires read-only lock, 'owner' support */
	      (3 << EPLOCK_MODE_TCLASS_SHIFT) | EPLOCK_MODE_OWNER_SUP);
	W_REG(osh, &regs->u.pcie2.lock_polling, 0); /* ucode does not use 'lock polling' */
	OR_REG(osh, &regs->u.pcie2.lock_addr_lo, EPLOCK_ADDR_LO_EN);

	if (bc_en) {
		/* addr_lo: Base[31:2], Enable [0] */
		bc_out_addr &= 0xFFFFFFFC;
		bc_out_addr |= (1 << 0);
		W_REG(osh, &regs->u.pcie2.bc_out_addr_lo, (uint32)(bc_out_addr & 0xFFFFFFFF));

		/* addr_hi: Base[39:32] */
		W_REG(osh, &regs->u.pcie2.bc_out_addr_hi, (uint32)((bc_out_addr >> 32) & 0xFF));

		/* mode: flow=3 */
		W_REG(osh, &regs->u.pcie2.bc_out_mode, (3 << 0));

		/* addr_lo: Bar[31:30] Base[26:2], Enable [0] */
		bc_in_addr &= 0xC7FFFFFC;
		bc_in_addr |= (1 << 0);
		W_REG(osh, &regs->u.pcie2.bc_in_addr, (uint32)bc_in_addr);
	}

	si_setcoreidx(sih, savecore);

	return TRUE;
} /* pcie_config_hwmutex_nic_or_dhd */

#if !defined(BCMDONGLEHOST)
/**
 * Configures the client hwmutex for vNIC (embedded WLAN cores in a SoC). The mutex client hardware
 * is part of the WLAN wrapper.
 *
 * @param[in] bc_out_addr  ICC broadcast out physical address. 0 means: ICC bcast not enabled.
 * @param[in] bc_in_addr   ICC broadcast in physical address
 */
static void
pcie_config_hwmutex_vnic(si_t *sih, osl_t *osh, u64 mutex_svr_addr,
                         u64 bc_out_addr, u64 bc_in_addr)
{
	u64 wlan_mlo_misc_wifi_pa;	/** physical address of mutex client in WLAN wrapper */
	struct wlan_mlo_misc_wifi_regs_s *regs;	/** virtual address of mutex client */
	const bool bc_en = (bc_out_addr != 0 || bc_in_addr != 0);

	wlan_mlo_misc_wifi_pa = OSL_PCI_READ_CONFIG(osh, PCI_BASE_ADDRESS_3, sizeof(u32));
	wlan_mlo_misc_wifi_pa <<= 32;
	wlan_mlo_misc_wifi_pa |= OSL_PCI_READ_CONFIG(osh, PCI_BASE_ADDRESS_2, sizeof(u32));
	wlan_mlo_misc_wifi_pa &= ~0xFULL;

	ASSERT(wlan_mlo_misc_wifi_pa != 0);
	regs = REG_MAP(wlan_mlo_misc_wifi_pa,
	               sizeof(struct wlan_mlo_misc_wifi_regs_s));
	ASSERT((mutex_svr_addr & 0xFFFFFF0000000001ULL) == 0); /* reserved regbits must be zero */
	W_REG(osh, &regs->lock_addr_lo, (uint32)(mutex_svr_addr & 0xFFFFFFFF));
	W_REG(osh, &regs->lock_addr_hi, (uint32)(mutex_svr_addr >> 32));
	/* ucode requires read-only lock, 'owner' support */
	W_REG(osh, &regs->lock_mode, LOCK_MODE_UBUS_FLOW_MASK | LOCK_MODE_UBUS_PRIORITY |
	                             LOCK_MODE_OWNER_SUPP);
	W_REG(osh, &regs->lock_polling, 0);		/* ucode does not use 'lock polling' */
	OR_REG(osh, &regs->lock_addr_lo, LOCK_ADDR_LO_EN);

	if (bc_en) {
		/* addr_lo: Base[31:2], Enable [0] */
		bc_out_addr &= 0xFFFFFFFC;
		bc_out_addr |= (1 << 0);
		W_REG(osh, &regs->bc_out_addr_lo, (uint32)(bc_out_addr & 0xFFFFFFFF));

		/* addr_hi: Base[39:32] */
		W_REG(osh, &regs->bc_out_addr_hi, (uint32)((bc_out_addr >> 32) & 0xFF));

		/* mode: priority=1, flow=3 */
		W_REG(osh, &regs->bc_out_mode, ((1 << 2) | (3 << 0)));

		/* addr_lo: Base[7:2], Enable [0] */
		bc_in_addr &= 0xFC;
		bc_in_addr |= (1 << 0);
		W_REG(osh, &regs->bc_in_addr, (uint32)bc_in_addr);
	}

	REG_UNMAP(regs);
} /* pcie_config_hwmutex_vnic */
#endif /* BCMDONGLEHOST */
#endif /* BCM_PCIE_MUTEX_FLAG_MASK */

/**
 * In new(er) chips, hardware offers a fast mutex mechanism to ucode, that needs to be configured
 * by the driver before ucode can start using it. This function configures the hardware 'client'
 * part of that hwmutex mechanism. The 'server' part is configured by the SDK.
 *
 * @param[in] en_icc_bcast_hwacc  TRUE if hardware acceleration for ICC broadcast has to be enabled
 *
 * @return <0:  the hwmutex mechanism was not available, or it could not be configured.
 *         >=0: the request id, also known as 'mutex id', to be used by ucode to verify the locking.
 */
int32
pcie_config_hwmutex(si_t *sih, int nv)
{
#if !defined(BCM_PCIE_MUTEX_FLAG_MASK)
	return -1;
#else
	osl_t *osh = si_osh(sih);
	u64 mutex_svr_addr;		/** ubus physical address of mutex server in PCIe RC */
	struct pci_dev *dev = osl_pci_device(osh);
	int32 hwmutex_id = -1;		/** known as a 'request id' in the SDK */
	u64    bc_out_addr = 0, bc_in_addr = 0;

	mutex_svr_addr = bcm_pcie_get_mutex_addr(dev);
	if (mutex_svr_addr == BCM_PCIE_MUTEX_ADDR_INVALID) {
		return -1;
	}

#if defined(BCM_PCIE_OUT_BCAST_ADDR_MASK)
	if (nv & BCM_BIT(1)) {
		bc_out_addr = bcm_pcie_get_bcast_out_addr(dev);
		bc_in_addr = bcm_pcie_get_bcast_in_addr(dev);
	}
#endif /* BCM_PCIE_OUT_BCAST_ADDR_MASK */

#if defined(BCMDONGLEHOST)
	if (!pcie_config_hwmutex_nic_or_dhd(sih, osh, mutex_svr_addr,
	                                    bc_out_addr, bc_in_addr)) {		/* DHD */
		return -1;
	}
#else /* NIC and vNIC (=embedded WLAN NIC driver, vPCIe) */
	if (EMBEDDED_2x2AX_CORE(sih->chip)) {
		return -1;
	}

	if (EMBEDDED_11BE_CORE(sih->chip)) {
		pcie_config_hwmutex_vnic(sih, osh, mutex_svr_addr,
		                         bc_out_addr, bc_in_addr);		/* vNIC */
	} else if (!pcie_config_hwmutex_nic_or_dhd(sih, osh, mutex_svr_addr,
	                                           bc_out_addr, bc_in_addr)) {	/* NIC */
		return -1;
	}
#endif /* BCMDONGLEHOST */
	if (nv & BCM_BIT(0)) {
		hwmutex_id = (bcm_pcie_get_mutex_config(dev) >> 4);
	}

	return hwmutex_id;
#endif /* BCM_PCIE_MUTEX_FLAG_MASK */
} /* pcie_config_hwmutex */

/** PCIe EP configuration space */
int
pcie_set_cfg_reg(si_t *sih, uint32 regoffset, uint32 regval)
{
	si_corereg(sih, sih->buscoreidx, OFFSETOF(sbpcieregs_t, configaddr), ~0, regoffset);
	si_corereg(sih, sih->buscoreidx, OFFSETOF(sbpcieregs_t, configdata), ~0, regval);
	return BCME_OK;
} /* pcie_set_cfg_reg */

/** PCIe EP configuration space */
uint32
pcie_get_cfg_reg(si_t *sih, uint32 regoffset)
{
	uint32 regval = 0;

	si_corereg(sih, sih->buscoreidx, OFFSETOF(sbpcieregs_t, configaddr), ~0, regoffset);
	regval = si_corereg(sih, sih->buscoreidx, OFFSETOF(sbpcieregs_t, configdata), 0, 0);
	return regval;
} /* pcie_get_cfg_reg */

/** PCIe EP enumeration space */
int32
pcie_get_core_regval(si_t *sih, uint regoffset)
{
	/* dhd iovar uses si_corereg() to read values from pcie core registers */
	return si_corereg(sih, sih->buscoreidx, regoffset, 0, 0);
}

#if defined(LINUX) || defined(linux)
uint32
pcie_rc_get_cfg_reg(struct pci_bus *root_bus, uint32 regoffset)
{
	uint32 regval = 0;

	pci_bus_read_config_dword(root_bus, 0, regoffset, &regval);
	return regval;
} /* pcie_rc_get_cfg_reg */

/**
 * dumps a subset of PCIe root complex configuration space registers
 *
 * @param[inout]  bstrbuf  If NULL, info will be dumped to the console. Otherwise into bstrbuf.
 */
void
pcie_rc_dump(struct pci_dev *pci_dev, struct bcmstrbuf *bstrbuf)
{
	int i = 0;
	struct pci_bus *root_bus = pci_dev->bus;

	bcm_bprintf_ex(bstrbuf, "\nPCIe RC AER/TL Registers\n");

	if (root_bus == NULL) {
		bcm_bprintf_ex(bstrbuf, "PCIe RC root_bus NULL\n");
		return;
	}

	/* Get the handle to the Root Bus of the PCIe device */
	while (NULL != root_bus->parent && ++i < 10) {
		root_bus = root_bus->parent;
	}

	if (i == 10) {
		bcm_bprintf_ex(bstrbuf, "PCIe RC could not be found. %px->parent=%px\n",
		               root_bus, root_bus->parent);
		return;
	}

	i = 0;
	while (pcie_rc_aer_tl_regs[i].minrev != 0) {
		bcm_bprintf_ex(bstrbuf, "%s: %08X\n", pcie_rc_aer_tl_regs[i].name,
		               pcie_rc_get_cfg_reg(root_bus, pcie_rc_aer_tl_regs[i].offset));
		i++;
	}
} /* pcie_rc_dump */

#endif /* LINUX || linux */

/**
 * dumps PCIe endpoint registers, both enumeration and configuration register space
 *
 * @param[inout]  bstrbuf  If NULL, info will be dumped to the console. Otherwise into bstrbuf.
 */
void
pcie_ep_dump(si_t *sih, struct bcmstrbuf *bstrbuf)
{
	int i = 0;

	bcm_bprintf_ex(bstrbuf, "PCIe endpoint CORE Registers\n");
	if (sih == NULL) {
		bcm_bprintf_ex(bstrbuf, "   cannot dump because sih==NULL\n");
		return;
	}

	while (pcie_ep_cmn_regs[i].minrev != 0) {
		if (!(BCM6726_CHIP(si_chipid(sih))) &&
			pcie_ep_cmn_regs[i].offset == PCIE_CORE_REG_S2P_TRANS4) {
			break;
		}

		if (PCIECOREREV(sih->buscorerev) >= pcie_ep_cmn_regs[i].minrev) {
			bcm_bprintf_ex(bstrbuf, "%23s@0x%04x: %08X %08X\n",
			               pcie_ep_cmn_regs[i].name,
			               pcie_ep_cmn_regs[i].offset,
			               pcie_get_core_regval(sih, (uint)pcie_ep_cmn_regs[i].offset),
			               pcie_get_core_regval(sih, (uint)pcie_ep_cmn_regs[i].offset));
		}

		i++;
	}

	bcm_bprintf_ex(bstrbuf, "\nPCIe endpoint CFG Registers\n");
	i = 0;
	while (pcie_ep_cfg_cmn_regs[i].minrev != 0) {
		bcm_bprintf_ex(bstrbuf, "%23s@0x%04X: %08X %08X\n",
		               pcie_ep_cfg_cmn_regs[i].name,
		               pcie_ep_cfg_cmn_regs[i].offset,
		               pcie_get_cfg_reg(sih, pcie_ep_cfg_cmn_regs[i].offset),
		               pcie_get_cfg_reg(sih, pcie_ep_cfg_cmn_regs[i].offset));
		i++;
	}

	i = 0;
	while (pcie_ep_cfg_ext_regs[i].minrev != 0) {
		bcm_bprintf_ex(bstrbuf, "%23s@0x%04X: %08X %08X\n",
		               pcie_ep_cfg_ext_regs[i].name,
		               pcie_ep_cfg_ext_regs[i].offset,
		               pcie_get_cfg_reg(sih, pcie_ep_cfg_ext_regs[i].offset),
		               pcie_get_cfg_reg(sih, pcie_ep_cfg_ext_regs[i].offset));
		i++;
	}

	bcm_bprintf_ex(bstrbuf, "\nPCIe endpoint CFG Registers raw dump\n");
	bcm_bprintf_ex(bstrbuf, "[regoffset] : value\n");
	for (i = 0; i <= 0x200; i += 4) {
		bcm_bprintf_ex(bstrbuf, "[0x%04X] : %08X %08X\n", i,
		               pcie_get_cfg_reg(sih, i), pcie_get_cfg_reg(sih, i));
	}
} /* pcie_ep_dump */

#if !defined(BCMDONGLEHOST) && !defined(DONGLEBUILD)
/**
 * vPCIe/vNIC does not have PCIe endpoint hardware, but the embedded WLAN wrapper contains certain
 * registers that have counterparts in 'real' PCIe EP hardware.
 *
 * @param[inout]  bstrbuf  If NULL, info will be dumped to the console. Otherwise into bstrbuf.
 */
void
vpcie_ep_dump(si_t *sih, struct bcmstrbuf *bstrbuf)
{
	u64 wlan_mlo_misc_wifi_pa;	/** physical address of mutex client in WLAN wrapper */
	struct wlan_mlo_misc_wifi_regs_s *regs;	/** PCIe EP enum space virtual address */
	osl_t *osh = si_osh(sih);

	if (!EMBEDDED_11BE_CORE(sih->chip)) {
		return;
	}

	wlan_mlo_misc_wifi_pa = OSL_PCI_READ_CONFIG(osh, PCI_BASE_ADDRESS_3, sizeof(u32));
	wlan_mlo_misc_wifi_pa <<= 32;
	wlan_mlo_misc_wifi_pa |= OSL_PCI_READ_CONFIG(osh, PCI_BASE_ADDRESS_2, sizeof(u32));
	wlan_mlo_misc_wifi_pa &= ~0xFULL;

	bcm_bprintf_ex(bstrbuf, "vPCIe WLAN wrapper registers\n");

	if (wlan_mlo_misc_wifi_pa == 0) {
		return;
	}

	regs = REG_MAP(wlan_mlo_misc_wifi_pa,
	               sizeof(struct wlan_mlo_misc_wifi_regs_s));
	bcm_bprintf_ex(bstrbuf, "[0x%08X] : lock_addr_lo: %08X\n",
	               (uint32)wlan_mlo_misc_wifi_pa, R_REG(osh, &regs->lock_addr_lo));
	bcm_bprintf_ex(bstrbuf, "[0x%08X] : lock_addr_hi: %08X\n",
	               (uint32)wlan_mlo_misc_wifi_pa + 0x4, R_REG(osh, &regs->lock_addr_hi));
	bcm_bprintf_ex(bstrbuf, "[0x%08X] : lock_mode:    %08X\n",
	               (uint32)wlan_mlo_misc_wifi_pa + 0x8, R_REG(osh, &regs->lock_mode));
	bcm_bprintf_ex(bstrbuf, "[0x%08X] : lock_polling: %08X\n",
	               (uint32)wlan_mlo_misc_wifi_pa + 0xC, R_REG(osh, &regs->lock_polling));

	REG_UNMAP(regs);
}
#endif /* !BCMDONGLEHOST && !DONGLEBUILD */

#endif /* BCMDRIVER */
