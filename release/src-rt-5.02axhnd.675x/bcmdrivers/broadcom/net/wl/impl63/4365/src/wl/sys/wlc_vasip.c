/*
 * VASIP related functions
 * Broadcom 802.11abg Networking Device Driver
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
 * $Id: wlc_vasip.c 737236 2017-12-20 08:17:39Z $
 */

#include <wlc_cfg.h>

#ifdef VASIP_HW_SUPPORT
#include <typedefs.h>
#include <wlc_types.h>
#include <siutils.h>
#include <wlioctl.h>
#include <wlc_pub.h>
#include <wlioctl.h>
#include <wlc.h>
#include <wlc_dbg.h>
#include <phy_misc_api.h>
#include <wlc_hw_priv.h>
#include <d11vasip_code.h>
#include <pcicfg.h>
#include <wl_export.h>
#include <wlc_vasip.h>
#include <wlc_phy_int.h>

#define VASIP_COUNTERS_LMT		256
#define VASIP_DEFINED_COUNTER_NUM	26
#define SVMP_MEM_DUMP_LEN_MAX		4096

/* BCM4365: Ax/Bx only has M0~M4 and C0 adds M5 */
#define SVMP_MEM_OFFSET_MAX_NOT_SUPPORT   0x0
#define SVMP_MEM_OFFSET_MAX_BCM4365B1 0x28000
#define SVMP_MEM_OFFSET_MAX_BCM4365C0 0x30000

#define SVMP_ACCESS_VIA_PHYTBL
#define SVMP_ACCESS_BLOCK_SIZE 16

/* local prototypes */
#if (defined(BCMDBG) || defined(BCMDBG_DUMP)) && defined(WLC_LOW)
/* dump vasip counters from vasip program memory */
static int wlc_dump_vasip_counters(wlc_info_t *wlc, struct bcmstrbuf *b);

/* dump vasip status data from vasip program memory */
static int wlc_dump_vasip_status(wlc_info_t *wlc, struct bcmstrbuf *b);

/* clear vasip counters */
static int wlc_vasip_counters_clear(wlc_hw_info_t *wlc);

/* copy svmp memory to a buffer starting from offset of length 'len', len is count of uint16's */
static int
wlc_svmp_mem_read(wlc_hw_info_t *wlc_hw, uint16 *ret_svmp_addr, uint32 offset, uint16 len);

/* set svmp memory with a value from offset of length 'len', len is count of uint16's */
static int wlc_svmp_mem_set(wlc_hw_info_t *wlc_hw, uint32 offset, uint16 len, uint16 val);
#endif /* BCMDBG && WLC_LOW */

/* read/write svmp memory */
int wlc_svmp_read_blk(wlc_hw_info_t *wlc_hw, uint16 *val, uint32 offset, uint16 len);
int wlc_svmp_write_blk(wlc_hw_info_t *wlc_hw, uint16 *val, uint32 offset, uint16 len);

/* get max SVMP address offset  */;
int wlc_svmp_mem_offset_max(wlc_hw_info_t *wlc_hw);

/* iovar table */
enum {
	IOV_VASIP_COUNTERS_CLEAR,
	IOV_SVMP_MEM,
	IOV_MU_RATE,
	IOV_MU_GROUP,
	IOV_MU_MCS_RECMD,
	IOV_MU_MCS_CAPPING,
	IOV_MU_SGI_RECMD,
	IOV_MU_SGI_RECMD_TH,
	IOV_MU_GROUP_DELAY,
	IOV_MU_PRECODER_DELAY
};

static int
wlc_vasip_doiovar(void *context, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *params, uint p_len, void *arg, int len, int vsize, struct wlc_if *wlcif);

static const bcm_iovar_t vasip_iovars[] = {
#if defined(BCMDBG) || defined(BCMDBG_DUMP)
	{"vasip_counters_clear", IOV_VASIP_COUNTERS_CLEAR,
	(0), IOVT_VOID, 0
	},
	{"svmp_mem", IOV_SVMP_MEM,
	(0), IOVT_BUFFER, sizeof(svmp_mem_t),
	},
	{"mu_rate", IOV_MU_RATE,
	(0), IOVT_BUFFER, 0,
	},
	{"mu_group", IOV_MU_GROUP,
	(0), IOVT_BUFFER, 0,
	},
	{"mu_mcs_recmd", IOV_MU_MCS_RECMD,
	(0), IOVT_UINT16, 0
	},
	{"mu_mcs_capping", IOV_MU_MCS_CAPPING,
	(0), IOVT_UINT16, 0
	},
	{"mu_sgi_recmd", IOV_MU_SGI_RECMD,
	(0), IOVT_INT16, 0
	},
	{"mu_sgi_recmd_th", IOV_MU_SGI_RECMD_TH,
	(0), IOVT_UINT16, 0
	},
	{"mu_group_delay", IOV_MU_GROUP_DELAY,
	(0), IOVT_UINT16, 0
	},
	{"mu_precoder_delay", IOV_MU_PRECODER_DELAY,
	(0), IOVT_UINT16, 0
	},
#endif /* BCMDBG */
	{NULL, 0, 0, 0, 0}
};

void
BCMATTACHFN(wlc_vasip_detach)(wlc_info_t *wlc)
{
	wlc_module_unregister(wlc->pub, "vasip", wlc);
}

int
BCMATTACHFN(wlc_vasip_attach)(wlc_info_t *wlc)
{
	wlc_pub_t *pub = wlc->pub;
	wlc_hw_info_t *wlc_hw = wlc->hw;
	int err = BCME_OK;
	uint32 *vasip_mem;
	int idx;
	uint32 vasipaddr;
#ifndef DONGLEBUILD
	osl_t *osh = wlc_hw->osh;
	uchar *bar_va = NULL;
	uint32 bar_size;
#endif // endif
	if ((err = wlc_module_register(pub, vasip_iovars, "vasip",
		wlc, wlc_vasip_doiovar, NULL, NULL, NULL))) {
		WL_ERROR(("wl%d: %s: wlc_module_register() failed\n",
			wlc->pub->unit, __FUNCTION__));
		return err;
	}

	if (!VASIP_PRESENT(wlc_hw->corerev)) {
		return BCME_OK;
	}

	/* save current core */
	idx = si_coreidx(wlc_hw->sih);
	if (si_setcore(wlc_hw->sih, ACPHY_CORE_ID, 0) != NULL) {
		/* get the VASIP memory base */
		vasipaddr = si_addrspace(wlc_hw->sih, 0);
		/* restore core */
		(void)si_setcoreidx(wlc_hw->sih, idx);
	} else {
		WL_ERROR(("wl%d: Failed to find ACPHY core \n", wlc_hw->unit));
		return BCME_UNSUPPORTED;
	}

#ifndef DONGLEBUILD
	bar_size = wl_pcie_bar2(wlc_hw->wlc->wl, &bar_va);
	if (bar_size) {
		WL_NONE(("wl%d: Mapping SVMP to BAR2\n", wlc_hw->unit));
		OSL_PCI_WRITE_CONFIG(osh, PCI_BAR1_CONTROL, sizeof(uint32), vasipaddr);
	} else if (0) {
		WL_NONE(("wl%d: Mapping SVMP to BAR1\n", wlc_hw->unit));
		/* Use PCIE BAR 1 as backup */
		bar_size = wl_pcie_bar1(wlc_hw->wlc->wl, &bar_va);

		if (bar_size) {
			OSL_PCI_WRITE_CONFIG(osh, PCI_BAR1_WIN, sizeof(uint32), vasipaddr);
		} else {
			WL_ERROR(("wl%d: Cannot map DSP mem BAR1 or BAR2.\n", wlc_hw->unit));
		}
	}

	BCM_REFERENCE(bar_size);
	ASSERT(bar_va != NULL && bar_size != 0);

	vasip_mem = (uint32 *)bar_va;
#else
	vasip_mem = (uint32 *)vasipaddr;
#endif /* DONGLEBUILD */

	/* save base address for accessing VASIP */
	wlc_hw->vasip_addr = (uint16 *)vasip_mem;

#if (defined(BCMDBG) || defined(BCMDBG_DUMP)) && defined(WLC_LOW)
	wlc_dump_register(pub, "vasip_counters", (dump_fn_t)wlc_dump_vasip_counters, (void *)wlc);
	wlc_dump_register(pub, "vasip_status", (dump_fn_t)wlc_dump_vasip_status, (void *)wlc);
#endif /* VASIP_HW_SUPPORT && WLC_LOW */

	return err;
}

static int
wlc_vasip_doiovar(void *context, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *params, uint p_len, void *arg, int len, int vsize, struct wlc_if *wlcif)
{
#if (defined(BCMDBG) || defined(BCMDBG_DUMP)) && defined(WLC_LOW)
	wlc_info_t *wlc = (wlc_info_t *)context;
#endif /* BCMDBG && WLC_LOW */

	int err = BCME_OK;
	switch (actionid) {

#if (defined(BCMDBG) || defined(BCMDBG_DUMP)) && defined(WLC_LOW)
	case IOV_SVAL(IOV_VASIP_COUNTERS_CLEAR):
		err = wlc_vasip_counters_clear(wlc->hw);
		break;

	case IOV_GVAL(IOV_SVMP_MEM): {
		svmp_mem_t *mem = (svmp_mem_t *)params;
		uint32 mem_addr;
		uint16 mem_len;

		mem_addr = mem->addr;
		mem_len = mem->len;

		if (len < mem_len) {
			err = BCME_BUFTOOSHORT;
			break;
		}

		err = wlc_svmp_mem_read(wlc->hw, (uint16 *)arg, mem_addr, mem_len);
		break;
	}

	case IOV_SVAL(IOV_SVMP_MEM): {
		svmp_mem_t *mem = (svmp_mem_t *)params;
		uint32 mem_addr;
		uint16 mem_len;

		mem_addr = mem->addr;
		mem_len = mem->len;

		err = wlc_svmp_mem_set(wlc->hw, mem_addr, mem_len, mem->val);
		break;
	}

	case IOV_GVAL(IOV_MU_RATE): {
		mu_rate_t *mu = (mu_rate_t *)arg;
		uint32 mem_addr;
		uint16 mem_len;
		bool fix_rate;
		uint16 grouping_forced, grouping_forced_mcs;

		if (len < sizeof(mu_rate_t)) {
			err = BCME_BUFTOOSHORT;
			break;
		}

		mem_addr = VASIP_OVERWRITE_MCS_FLAG_ADDR_OFFSET;
		mem_len = 1;
		err = wlc_svmp_mem_read(wlc->hw, (uint16 *) &fix_rate, mem_addr, mem_len);

		mem_addr = VASIP_GROUP_FORCED_ADDR_OFFSET;
		mem_len = 1;
		err = wlc_svmp_mem_read(wlc->hw,
			(uint16 *) &grouping_forced, mem_addr, mem_len);

		mem_addr = VASIP_GROUP_FORCED_MCS_ADDR_OFFSET;
		mem_len = 1;
		err = wlc_svmp_mem_read(wlc->hw,
			(uint16 *) &grouping_forced_mcs, mem_addr, mem_len);

		fix_rate |= ((grouping_forced > 0) && (grouping_forced_mcs == 1));

		if (fix_rate == 0) {
			//mem_addr = VASIP_STEERING_MCS_BUF_ADDR_OFFSET;
			mu->auto_rate = 1;
		} else {
			//mem_addr = VASIP_OVERWRITE_MCS_BUF_ADDR_OFFSET;
			mu->auto_rate = 0;
		}

		mem_len = 4;
		mem_addr = VASIP_STEERING_MCS_BUF_ADDR_OFFSET;
		err = wlc_svmp_mem_read(wlc->hw, mu->rate_user, mem_addr, mem_len);
		break;
	}

	case IOV_SVAL(IOV_MU_RATE): {
		mu_rate_t *mu = (mu_rate_t *)params;
		uint16 forced_group, group_method;
		uint32 mem_addr;
		uint16 mem_len;
		bool   fix_rate;

		/* get forced_group */
		mem_addr = VASIP_GROUP_FORCED_ADDR_OFFSET;
		mem_len = 1;
		err = wlc_svmp_mem_read(wlc->hw, &forced_group, mem_addr, mem_len);
		/* get group_method */
		mem_addr = VASIP_GROUP_METHOD_ADDR_OFFSET;
		mem_len = 1;
		err = wlc_svmp_mem_read(wlc->hw, &group_method, mem_addr, mem_len);
		/* only allow fix mu_rate when */
		/* either "forced only 1 group" or "auto-groupping with old method (M=0)" */
		fix_rate = (!mu->auto_rate) &&
			((forced_group == WL_MU_GROUP_FORCED_1GROUP) ||
				((forced_group == WL_MU_GROUP_MODE_AUTO) &&
				(group_method == WL_MU_GROUP_METHOD_OLD)));
		mem_addr = VASIP_OVERWRITE_MCS_FLAG_ADDR_OFFSET;
		mem_len = 1;
		if ((err = wlc_svmp_mem_set(wlc->hw,
				mem_addr, mem_len, fix_rate)) != BCME_OK) {
			break;
		}
		if (fix_rate) {
			mem_addr = VASIP_OVERWRITE_MCS_BUF_ADDR_OFFSET;
			mem_len = 4;
			err = wlc_svmp_mem_blk_set(wlc->hw, mem_addr, mem_len, mu->rate_user);
		}
		/* barf if set mu_rate but blocked by mu_group */
		if (mu->auto_rate == fix_rate) {
			err = BCME_USAGE_ERROR;
		}
		break;
	}

	case IOV_GVAL(IOV_MU_GROUP): {
		mu_group_t *mugrp = (mu_group_t *)arg;
		uint16 forced_group;
		uint32 mem_addr;
		uint16 mem_len;
		uint16 v2m_grp[14];
		uint16 m, n;

		/* set WL_MU_GROUP_PARAMS_VERSION */
		mugrp->version = WL_MU_GROUP_PARAMS_VERSION;

		/* check if forced */
		mem_addr = VASIP_GROUP_FORCED_ADDR_OFFSET;
		mem_len = 1;
		err = wlc_svmp_mem_read(wlc->hw, &forced_group, mem_addr, mem_len);

		if (forced_group > 0) {
			mugrp->forced = 1;
			mugrp->forced_group_num = forced_group;
			/* get forced_group_mcs */
			mem_addr = VASIP_GROUP_FORCED_MCS_ADDR_OFFSET;
			mem_len = 1;
			err = wlc_svmp_mem_read(wlc->hw,
				(uint16*)(&(mugrp->forced_group_mcs)), mem_addr, mem_len);
			/* get forced_group_option */
			mem_addr = VASIP_GROUP_FORCED_OPTION_ADDR_OFFSET;
			mem_len = forced_group*WL_MU_GROUP_NUSER_MAX;
			err = wlc_svmp_mem_read(wlc->hw,
				(uint16*)(&(mugrp->group_option[0][0])), mem_addr, mem_len);
		} else {
			mugrp->forced = 0;
			mugrp->forced_group_num = 0;
			/* get group_method */
			mem_addr = VASIP_GROUP_METHOD_ADDR_OFFSET;
			mem_len = 1;
			err = wlc_svmp_mem_read(wlc->hw,
				(uint16*)&(mugrp->group_method), mem_addr, mem_len);
			/* get group_number */
			mem_addr = VASIP_GROUP_NUMBER_ADDR_OFFSET;
			mem_len = 1;
			err = wlc_svmp_mem_read(wlc->hw,
				(uint16*)(&(mugrp->group_number)), mem_addr, mem_len);
			/* method name */
			if (mugrp->group_method == 0) {
				snprintf(mugrp->group_method_name,
					sizeof(mugrp->group_method_name),
					"1 group for all admitted users");
			} else if (mugrp->group_method == 1) {
				snprintf(mugrp->group_method_name,
					sizeof(mugrp->group_method_name),
					"N best THPT groups and equally distributed across all BW");
			} else if (mugrp->group_method == 2) {
				snprintf(mugrp->group_method_name,
					sizeof(mugrp->group_method_name),
					"greedy-cover non-disjoint grouping");
			} else if (mugrp->group_method == 3) {
				snprintf(mugrp->group_method_name,
					sizeof(mugrp->group_method_name),
					"disjoint grouping");
			} else if (mugrp->group_method == 4) {
				snprintf(mugrp->group_method_name,
					sizeof(mugrp->group_method_name),
					"greedy-cover-extension grouping");
			} else if (mugrp->group_method == 5) {
				snprintf(mugrp->group_method_name,
					sizeof(mugrp->group_method_name),
					"greedy-cover-extension grouping with max-phyrate group");
			} else {
				snprintf(mugrp->group_method_name,
					sizeof(mugrp->group_method_name),
					"not support yet");
			}
			/* read out v2m_buf_grp[] to get latest recommend grouping */
			mem_addr = VASIP_V2M_GROUP_OFFSET;
			mem_len = 2;
			err = wlc_svmp_mem_read(wlc->hw, v2m_grp, mem_addr, mem_len);
			n = (v2m_grp[0] - 2) / 28;
			if (((n > mugrp->group_number) && (mugrp->group_method == 1)) ||
					((v2m_grp[0] != 0) && (v2m_grp[0] != (n * 28 + 2)))) {
				mugrp->auto_group_num = 0;
				WL_ERROR(("%s: unexpected auto_group_num=%d "
					"or v2m_len=%d (should be 2+28*N)\n",
					__FUNCTION__, n, v2m_grp[0]));
				//err = BCME_ERROR;
			} else {
				mugrp->auto_group_num = n;
				mem_addr = VASIP_V2M_GROUP_OFFSET + 2;
				mem_len = 14;
				for (m = 0; m < mugrp->auto_group_num; m++) {
					err = wlc_svmp_mem_read(wlc->hw, v2m_grp,
							mem_addr, mem_len);
					mugrp->group_GID[m] = v2m_grp[1] & 0x003f;
					for (n = 0; n < 4; n++) {
						mugrp->group_option[m][n] =
							((v2m_grp[2+3*n] & 0x00ff) << 8) +
								(v2m_grp[3+3*n] & 0x00ff);
					}
					mem_addr += mem_len;
				}
			}
		}
		break;
	}

	case IOV_SVAL(IOV_MU_GROUP): {
		mu_group_t *mugrp = (mu_group_t *)arg;
		uint16 forced_group;
		uint32 mem_addr;
		uint16 mem_len;

		/* check WL_MU_GROUP_PARAMS_VERSION */
		if (mugrp->version != WL_MU_GROUP_PARAMS_VERSION) {
			err = BCME_BADARG;
			break;
		}

		/* forced grouping */
		if (mugrp->forced == WL_MU_GROUP_MODE_FORCED) {
			/* set forced_group with forced_group_num */
			mem_addr = VASIP_GROUP_FORCED_ADDR_OFFSET;
			mem_len = 1;
			if ((err = wlc_svmp_mem_set(wlc->hw, mem_addr, mem_len,
					mugrp->forced_group_num)) != BCME_OK) {
				break;
			}
			/* set forced_group mcs */
			mem_addr = VASIP_GROUP_FORCED_MCS_ADDR_OFFSET;
			mem_len = 1;
			if ((err = wlc_svmp_mem_set(wlc->hw, mem_addr, mem_len,
					mugrp->forced_group_mcs)) != BCME_OK) {
				break;
			}
			/* store forced grouping options into SVMP */
			mem_addr = VASIP_GROUP_FORCED_OPTION_ADDR_OFFSET;
			mem_len = mugrp->forced_group_num*WL_MU_GROUP_NUSER_MAX;
			if ((err = wlc_svmp_write_blk(wlc->hw,
					(uint16*)(&(mugrp->group_option[0][0])),
					mem_addr, mem_len)) != BCME_OK) {
				break;
			}
			/* set fix_rate=0 for forced_group==1 && forced_group_num!=1 */
			if (mugrp->forced_group_num != 1) {
				mem_addr = VASIP_OVERWRITE_MCS_FLAG_ADDR_OFFSET;
				mem_len = 1;
				if ((err = wlc_svmp_mem_set(wlc->hw,
						mem_addr, mem_len, 0)) != BCME_OK) {
					break;
				}
			}
		} else if (mugrp->forced == WL_MU_GROUP_MODE_AUTO) {
			/* disable forced_group */
			mem_addr = VASIP_GROUP_FORCED_ADDR_OFFSET;
			mem_len = 1;
			if ((err = wlc_svmp_mem_set(wlc->hw,
					mem_addr, mem_len, 0)) != BCME_OK) {
				break;
			}
			/* clean first forced grouping option in SVMP */
			mem_addr = VASIP_GROUP_FORCED_OPTION_ADDR_OFFSET;
			mem_len = WL_MU_GROUP_NGROUP_MAX*WL_MU_GROUP_NUSER_MAX;
			if ((err = wlc_svmp_mem_set(wlc->hw,
					mem_addr, mem_len, 0xffff)) != BCME_OK) {
				break;
			}
		} /* mugrp->forced can be WL_MU_GROUP_ENTRY_EMPTY for no '-g' option */

		/* auto grouping parameters only when not-forced-group */
		mem_addr = VASIP_GROUP_FORCED_ADDR_OFFSET;
		mem_len = 1;
		err = wlc_svmp_mem_read(wlc->hw, &forced_group, mem_addr, mem_len);
		if (forced_group == WL_MU_GROUP_MODE_AUTO) {
			if (mugrp->group_number != WL_MU_GROUP_ENTRY_EMPTY) {
				/* update group_number */
				mem_addr = VASIP_GROUP_NUMBER_ADDR_OFFSET;
				mem_len = 1;
				if ((err = wlc_svmp_mem_set(wlc->hw, mem_addr, mem_len,
						mugrp->group_number)) != BCME_OK) {
					break;
				}
			}
			if (mugrp->group_method != WL_MU_GROUP_ENTRY_EMPTY) {
				/* update group method */
				mem_addr = VASIP_GROUP_METHOD_ADDR_OFFSET;
				mem_len = 1;
				if ((err = wlc_svmp_mem_set(wlc->hw, mem_addr, mem_len,
						mugrp->group_method)) != BCME_OK) {
					break;
				}
				/* set group number */
				if (mugrp->group_method == WL_MU_GROUP_METHOD_OLD) {
					/*  old method: it should be 1 */
					mem_addr = VASIP_GROUP_NUMBER_ADDR_OFFSET;
					mem_len = 1;
					if ((err = wlc_svmp_mem_set(wlc->hw, mem_addr, mem_len,
							1)) != BCME_OK) {
						break;
					}
				} else if (mugrp->group_number == WL_MU_GROUP_ENTRY_EMPTY) {
					/* set if not specified group_number
					 *   method 1: set 1 if not specified group_number
					 *   method 2&3: don't-care (set to 4)
					 */
					mem_addr = VASIP_GROUP_NUMBER_ADDR_OFFSET;
					mem_len = 1;
					if (mugrp->group_method == 1) {
						mugrp->group_number = 1;
					} else if ((mugrp->group_method == 2) ||
							(mugrp->group_method == 3)) {
						mugrp->group_number = 4;
					}
					if ((err = wlc_svmp_mem_set(wlc->hw,
							mem_addr, mem_len,
							mugrp->group_number)) != BCME_OK) {
						break;
					}
				}
				/* set fix_rate=0 for forced_group==0 && old mathod */
				if (mugrp->group_method != WL_MU_GROUP_METHOD_OLD) {
					mem_addr = VASIP_OVERWRITE_MCS_FLAG_ADDR_OFFSET;
					mem_len = 1;
					if ((err = wlc_svmp_mem_set(wlc->hw,
							mem_addr, mem_len, 0)) != BCME_OK) {
						break;
					}
				}
			}
		}
		break;
	}

	/* VASIP FW knobs: unsigned int16 */
	case IOV_GVAL(IOV_MU_MCS_RECMD):
	case IOV_GVAL(IOV_MU_MCS_CAPPING):
	case IOV_GVAL(IOV_MU_SGI_RECMD_TH):
	case IOV_GVAL(IOV_MU_GROUP_DELAY):
	case IOV_GVAL(IOV_MU_PRECODER_DELAY): {
		uint16 value;
		uint16 mem_len = 1;
		uint32 mem_addr = VASIP_MCS_RECMND_MI_ENA_OFFSET;

		if (len < mem_len) {
			err = BCME_BUFTOOSHORT;
			break;
		}

		if (IOV_ID(actionid) == IOV_MU_MCS_RECMD) {
			mem_addr = VASIP_MCS_RECMND_MI_ENA_OFFSET;
		} else if (IOV_ID(actionid) == IOV_MU_MCS_CAPPING) {
			mem_addr = VASIP_MCS_CAPPING_ENA_OFFSET;
		} else if (IOV_ID(actionid) == IOV_MU_SGI_RECMD_TH) {
			mem_addr = VASIP_SGI_RECMND_THRES_OFFSET;
		} else if (IOV_ID(actionid) == IOV_MU_GROUP_DELAY) {
			mem_addr = VASIP_DELAY_GROUP_TIME_OFFSET;
		} else if (IOV_ID(actionid) == IOV_MU_PRECODER_DELAY) {
			mem_addr = VASIP_DELAY_PRECODER_TIME_OFFSET;
		}

		err = wlc_svmp_mem_read(wlc->hw, &value, mem_addr, mem_len);
		*((uint32*)arg) = value;
		break;
	}

	case IOV_SVAL(IOV_MU_MCS_RECMD):
	case IOV_SVAL(IOV_MU_MCS_CAPPING):
	case IOV_SVAL(IOV_MU_SGI_RECMD_TH):
	case IOV_SVAL(IOV_MU_GROUP_DELAY):
	case IOV_SVAL(IOV_MU_PRECODER_DELAY): {
		uint16 value;
		uint16 mem_len = 1;
		uint32 mem_addr = VASIP_MCS_RECMND_MI_ENA_OFFSET;

		if (len < mem_len) {
			err = BCME_BUFTOOSHORT;
			break;
		}

		value = *((uint16*)params);

		if (IOV_ID(actionid) == IOV_MU_MCS_RECMD) {
			mem_addr = VASIP_MCS_RECMND_MI_ENA_OFFSET;
			if ((value != 0) && (value != 1)) {
				return BCME_USAGE_ERROR;
			}
		} else if (IOV_ID(actionid) == IOV_MU_MCS_CAPPING) {
			mem_addr = VASIP_MCS_CAPPING_ENA_OFFSET;
			if ((value != 0) && (value != 1)) {
				return BCME_USAGE_ERROR;
			}
		} else if (IOV_ID(actionid) == IOV_MU_SGI_RECMD_TH) {
			mem_addr = VASIP_SGI_RECMND_THRES_OFFSET;
		} else if (IOV_ID(actionid) == IOV_MU_GROUP_DELAY) {
			mem_addr = VASIP_DELAY_GROUP_TIME_OFFSET;
		} else if (IOV_ID(actionid) == IOV_MU_PRECODER_DELAY) {
			mem_addr = VASIP_DELAY_PRECODER_TIME_OFFSET;
		}

		err = wlc_svmp_mem_set(wlc->hw, mem_addr, mem_len, value);
		break;
	}

	/* VASIP FW knobs: signed int16 */
	case IOV_GVAL(IOV_MU_SGI_RECMD): {
		int16 value;
		uint16 mem_len = 1;
		uint32 mem_addr = VASIP_SGI_RECMND_METHOD_OFFSET;

		if (len < mem_len) {
			err = BCME_BUFTOOSHORT;
			break;
		}

		if (IOV_ID(actionid) == IOV_MU_SGI_RECMD) {
			mem_addr = VASIP_SGI_RECMND_METHOD_OFFSET;
		}

		err = wlc_svmp_mem_read(wlc->hw, (uint16*)&value, mem_addr, mem_len);
		*((int32*)arg) = (int16)value;
		break;
	}
	case IOV_SVAL(IOV_MU_SGI_RECMD): {
		int16 value;
		uint16 mem_len = 1;
		uint32 mem_addr = VASIP_SGI_RECMND_METHOD_OFFSET;

		if (len < mem_len) {
			err = BCME_BUFTOOSHORT;
			break;
		}

		value = *((int16*)params);

		if (IOV_ID(actionid) == IOV_MU_SGI_RECMD) {
			mem_addr = VASIP_SGI_RECMND_METHOD_OFFSET;
			if ((value != 0) && (value != 1) && (value != -1) && (value != 2)) {
				return BCME_USAGE_ERROR;
			}
		}

		err = wlc_svmp_mem_set(wlc->hw, mem_addr, mem_len, (uint16)value);
		break;
	}
#endif /* BCMDBG && WLC_LOW */
	}
	return err;
}

/* write vasip code to vasip program memory */
#ifndef SVMP_ACCESS_VIA_PHYTBL
static void
wlc_vasip_write(wlc_hw_info_t *wlc_hw, const uint32 vasip_code[], const uint nbytes)
{
	uint32 *vasip_mem = (uint32*)wlc_hw->vasip_addr;
#if defined(WL_MU_TX) || defined(WL_AIR_IQ)
	int i, count;
	count = (nbytes/sizeof(uint32));
#endif // endif

	WL_TRACE(("wl%d: %s\n", wlc_hw->unit, __FUNCTION__));

	ASSERT(ISALIGNED(nbytes, sizeof(uint32)));

	/* write vasip code to program memory */
	BCM_REFERENCE(vasip_code);
	BCM_REFERENCE(nbytes);

#if defined(WL_MU_TX) || defined(WL_AIR_IQ)
	for (i = 0; i < count; i++) {
		vasip_mem[i] = vasip_code[i];
	}
#endif // endif
}

#ifdef VASIP_SPECTRUM_ANALYSIS
static void
wlc_vasip_spectrum_tbl_write(wlc_hw_info_t *wlc_hw,
        const uint32 vasip_spectrum_tbl[], const uint nbytes_tbl)
{
	uint32 *vasip_mem = (uint32 *)wlc_hw->vasip_addr;
	int idx;

	int n, tbl_count;
	tbl_count = (nbytes_tbl/sizeof(uint32));

	WL_TRACE(("wl%d: %s\n", wlc_hw->unit, __FUNCTION__));

	ASSERT(ISALIGNED(nbytes, sizeof(uint32)));

	/* write spactrum analysis tables to VASIP_SPECTRUM_TBL_ADDR_OFFSET */
	BCM_REFERENCE(vasip_spectrum_tbl);
	BCM_REFERENCE(nbytes_tbl);
	for (n = 0; n < tbl_count; n++) {
		vasip_mem[VASIP_SPECTRUM_TBL_ADDR_OFFSET + n] = vasip_spectrum_tbl[i];
	}
}
#endif /* VASIP_SPECTRUM_ANALYSIS */
#endif /* SVMP_ACCESS_VIA_PHYTBL */

/* initialize vasip */
void
BCMINITFN(wlc_vasip_init)(wlc_hw_info_t *wlc_hw)
{
	const uint32 *vasip_code = NULL;
	uint nbytes = 0;
	uint8 vasipver;
	uint8 val_rxchain, val_txchain;

#ifdef VASIP_SPECTRUM_ANALYSIS
	const uint32 *vasip_spectrum_tbl = NULL;
	uint nbytes_tbl = 0;
#endif // endif

	WL_TRACE(("wl%d: %s\n", wlc_hw->unit, __FUNCTION__));

	vasipver = phy_misc_get_vasip_ver((phy_info_t *)wlc_hw->band->pi);
	if (vasipver == VASIP_NOVERSION) {
		return;
	}

	phy_misc_vasip_clk_set((phy_info_t *)wlc_hw->band->pi, 1);

	/* Determine 3x3 or 4x4 VASIP FW image to use */
	wlc_phy_stf_chain_get_valid((phy_info_t *)wlc_hw->band->pi,
		&val_txchain, &val_rxchain);
	ASSERT(val_txchain >= 3);

	if (vasipver == 0 || vasipver == 2 || vasipver == 3) {
#ifdef WL_AIR_IQ
		vasip_code = d11vasip_airiq;
		nbytes = d11vasip_airiqsz;
#ifdef VASIP_SPECTRUM_ANALYSIS
		vasip_spectrum_tbl = d11vasip_airiq_tbl;
		nbytes_tbl = d11vasip_airiq_tbl_sz;
#endif // endif
#else
		if (WLC_BITSCNT(val_txchain) == 3) {
			vasip_code = d11vasip3x3;
			nbytes = d11vasip3x3sz;
#ifdef VASIP_SPECTRUM_ANALYSIS
			vasip_spectrum_tbl = d11vasip3x3_tbl;
			nbytes_tbl = d11vasip3x3_tbl_sz;
#endif // endif
		} else {
			vasip_code = d11vasip0;
			nbytes = d11vasip0sz;
#ifdef VASIP_SPECTRUM_ANALYSIS
			vasip_spectrum_tbl = d11vasip_tbl;
			nbytes_tbl = d11vasip_tbl_sz;
#endif // endif
		}
#endif /* WL_AIR_IQ */
	} else {
		WL_ERROR(("%s: wl%d: unsupported vasipver %d \n",
			__FUNCTION__, wlc_hw->unit, vasipver));
		ASSERT(0);
		return;
	}

	if (vasip_code != NULL) {
		/* stop the vasip processor */
		phy_misc_vasip_proc_reset((phy_info_t *)wlc_hw->band->pi, 1);

		if (!wlc_hw->vasip_loaded) {
			/* write binary to the vasip program memory */
#ifdef SVMP_ACCESS_VIA_PHYTBL
			phy_misc_vasip_bin_write((phy_info_t *)wlc_hw->band->pi,
				vasip_code, nbytes);
#else
			wlc_vasip_write(wlc_hw, vasip_code, nbytes);
#endif /* SVMP_ACCESS_VIA_PHYTBL */

			/* write spectrum tables to the vasip SVMP M4 0x26800 */
#ifdef VASIP_SPECTRUM_ANALYSIS
#ifdef SVMP_ACCESS_VIA_PHYTBL
			phy_misc_vasip_spectrum_tbl_write((phy_info_t *)wlc_hw->band->pi,
				vasip_spectrum_tbl, nbytes_tbl);
#else
			wlc_vasip_spectrum_tbl_write(wlc_hw, vasip_spectrum_tbl,
				nbytes_spectrum_tbl);
#endif /* SVMP_ACCESS_VIA_PHYTBL */
#endif /* VASIP_SPECTRUM_ANALYSIS */
			wlc_hw->vasip_loaded = TRUE;
		}
		/* reset and start the vasip processor */
#if defined(WL_MU_TX) || defined(WL_AIR_IQ)
		phy_misc_vasip_proc_reset((phy_info_t *)wlc_hw->band->pi, 0);
#endif // endif
	}
}

#if defined(BCMDBG) || defined(BCMDBG_DUMP)

/* dump vasip status data from vasip program memory */
int
wlc_dump_vasip_status(wlc_info_t *wlc, struct bcmstrbuf *b)
{
#ifdef SVMP_ACCESS_VIA_PHYTBL
	uint16 status[VASIP_COUNTERS_LMT];
#else
	uint16 * status;
#endif // endif
	uint32 offset = VASIP_COUNTERS_ADDR_OFFSET;
	wlc_hw_info_t *wlc_hw = wlc->hw;
	int ret = BCME_OK, i;

	if (!VASIP_PRESENT(wlc_hw->corerev)) {
		bcm_bprintf(b, "VASIP is not present!\n");
		return ret;
	}

	if (!wlc->clk) {
		return BCME_NOCLK;
	}

#ifndef SVMP_ACCESS_VIA_PHYTBL
	status = (uint16 *)(wlc_hw->vasip_addr + offset);
#endif // endif

	for (i = 0; i < VASIP_COUNTERS_LMT; i++) {
#ifdef SVMP_ACCESS_VIA_PHYTBL
		status[i] = phy_misc_vasip_svmp_read((phy_info_t *)wlc_hw->band->pi, offset+i);
#endif // endif
		bcm_bprintf(b, "status[%d] %u\n", i, status[i]);
	}

	return ret;
}

/* dump vasip counters from vasip program memory */
int
wlc_dump_vasip_counters(wlc_info_t *wlc, struct bcmstrbuf *b)
{
#ifdef SVMP_ACCESS_VIA_PHYTBL
	uint16 counter[VASIP_COUNTERS_LMT];
	uint16 mcs[16], mcs1[16], c[4], s[4], c1[4], s1[4], N_user;
#else
	uint16 * counter;
#endif // endif
	uint32 offset = VASIP_COUNTERS_ADDR_OFFSET;
	uint32 offset_steered_mcs = VASIP_STEER_MCS_ADDR_OFFSET;
	uint32 offset_recommended_mcs = VASIP_RECOMMEND_MCS_ADDR_OFFSET;

	wlc_hw_info_t *wlc_hw = wlc->hw;
	int ret = BCME_OK, i;

	if (!VASIP_PRESENT(wlc_hw->corerev)) {
		bcm_bprintf(b, "VASIP is not present!\n");
		return ret;
	}

	if (!wlc->clk) {
		return BCME_NOCLK;
	}

#ifndef SVMP_ACCESS_VIA_PHYTBL
	counter = (uint16 *)(wlc_hw->vasip_addr + offset);
#endif // endif

#ifdef SVMP_ACCESS_VIA_PHYTBL
	/* print for any non-zero values */
	for (i = 0; i < VASIP_COUNTERS_LMT; i++) {
		counter[i] = phy_misc_vasip_svmp_read((phy_info_t *)wlc_hw->band->pi, offset+i);
	}

	for (i = 0; i < 16; i++) {
		mcs[i] = phy_misc_vasip_svmp_read((phy_info_t *)wlc_hw->band->pi,
				offset_steered_mcs+i);
	}

	for (i = 0; i < 4; i++) {
		s[i] = ((mcs[i] & 0xf0) >> 4) + 1;
		c[i] = mcs[i] & 0xf;
	}

	for (i = 0; i < 4; i++) {
		mcs1[i] = phy_misc_vasip_svmp_read((phy_info_t *)wlc_hw->band->pi,
				offset_recommended_mcs+i);
	}

	for (i = 0; i < 4; i++) {
		s1[i] = ((mcs1[i] & 0xf0) >> 4) + 1;
		c1[i] = mcs1[i] & 0xf;
	}

#endif /* SVMP_ACCESS_VIA_PHYTBL */

	bcm_bprintf(b, "Received Interrupts:\n"
		"      bfr_module_done:0x%x     bfe_module_done:0x%x     bfe_imp_done:0x%x\n"
			"      m2v_transfer_done:0x%x   v2m_transfder_done:0x%x\n\n",
			counter[2], counter[3], counter[4], counter[8], counter[9]);

	N_user = (mcs[9] > 4) ? 0 : mcs[9];
	for (i = 0; i < N_user; i++) {
		bcm_bprintf(b, "user%d: bfidx %d, steered rate c%ds%d, "
				"recommended rate c%ds%d, precoderSNR %ddB\n",
				i, mcs[10+i], c[i], s[i], c1[i], s1[i], mcs[5+i]/4);
	}

	bcm_bprintf(b, "\nImportant SVMP address:\n"
			"      PPR table address:           0x%x\n"
			"      MCS threshold table address: 0x%x\n"
			"      M2V address0:                0x%x\n"
			"      M2V address1:                0x%x\n"
			"      grouping V2M address:        0x%x\n"
			"      precoder V2M address:        0x%x\n"
			"      driver interface address:    0x%x\n",
			VASIP_PPR_TABLE_OFFSET, VASIP_MCS_THRESHOLD_OFFSET, VASIP_M2V0_OFFSET,
			VASIP_M2V1_OFFSET, VASIP_V2M_GROUP_OFFSET, VASIP_V2M_PRECODER_OFFSET,
			VASIP_RATECAP_BLK);

	/*
	for (i = VASIP_DEFINED_COUNTER_NUM; i < VASIP_COUNTERS_LMT; i++) {
		if (counter[i] != 0) {
			bcm_bprintf(b, "counter[%d] 0x%x ", i, counter[i]);
		}
	}
	*/

	bcm_bprintf(b, "%d mac2vasip requests include bad reports.\n", counter[129]);

	if (counter[128] & 0x1)
		bcm_bprintf(b, "ERROR: more than 4 users in group selecton.\n");

	if (counter[128] & 0x2)
		bcm_bprintf(b, "ERROR: user BW mismatch in group selecton.\n");

	if (counter[128] & 0x4)
		bcm_bprintf(b, "ERROR: user BW is beyond 80MHz in group selecton.\n");

	if (counter[128] & 0x8)
		bcm_bprintf(b, "ERROR: BFM index is out of range in group selecton.\n");

	if (counter[128] & 0x10)
		bcm_bprintf(b, "ERROR: BFM index is repeated in group selecton.\n");

	if (counter[128] & 0x20)
		bcm_bprintf(b, "ERROR: output address is out of range in precoder.\n");

	if (counter[128] & 0x40)
		bcm_bprintf(b, "ERROR: BFM index is out of range in precoder.\n");

	if (counter[128] & 0x80)
		bcm_bprintf(b, "ERROR: one user has more than 2 streams in precoder.\n");

	if (counter[128] & 0x100)
		bcm_bprintf(b, "ERROR: more than 4 streams in precoder.\n");

	if (counter[128] & 0x200)
		bcm_bprintf(b, "ERROR: user BW mismatch in precoder.\n");

	if (counter[128] & 0x400)
		bcm_bprintf(b, "ERROR: user BW is beyond 80MHz in precoder.\n");

	if (counter[128] & 0x800)
		bcm_bprintf(b, "ERROR: number of TX antenna is not 4.\n");

	if (counter[128] & 0x1000)
		bcm_bprintf(b, "ERROR: sounding report type is not correct.\n");

	bcm_bprintf(b, "\n");
	return ret;
}

/* clear vasip counters */
int
wlc_vasip_counters_clear(wlc_hw_info_t *wlc_hw)
{
	int i;
#ifdef SVMP_ACCESS_VIA_PHYTBL
	uint32 offset = VASIP_COUNTERS_ADDR_OFFSET;
#else
	uint16 * counter;
#endif // endif

	if (!VASIP_PRESENT(wlc_hw->corerev)) {
		return BCME_UNSUPPORTED;
	}
	if (!wlc_hw->clk) {
		return BCME_NOCLK;
	}

#ifndef SVMP_ACCESS_VIA_PHYTBL
	counter = (uint16 *)(wlc_hw->vasip_addr + VASIP_COUNTERS_ADDR_OFFSET);
#endif // endif
	for (i = 0; i < VASIP_COUNTERS_LMT; i++) {
#ifdef SVMP_ACCESS_VIA_PHYTBL
		phy_misc_vasip_svmp_write((phy_info_t *)wlc_hw->band->pi, offset+i, 0);
#else
		counter[i] = 0;
#endif // endif
	}
	return BCME_OK;
}

/* copy svmp memory to a buffer starting from offset of length 'len', len is count of uint16's */
static int
wlc_svmp_mem_read(wlc_hw_info_t *wlc_hw, uint16 *ret_svmp_addr, uint32 offset, uint16 len)
{
#ifndef SVMP_ACCESS_VIA_PHYTBL
	uint16 * svmp_addr;
#endif // endif
	uint16 i;
	uint32 svmp_mem_offset_max;

	if (!VASIP_PRESENT(wlc_hw->corerev)) {
		return BCME_UNSUPPORTED;
	}

	if (!wlc_hw->clk) {
		return BCME_NOCLK;
	}

	svmp_mem_offset_max = wlc_svmp_mem_offset_max(wlc_hw);
	if ((offset + len) > svmp_mem_offset_max) {
		return BCME_RANGE;
	}

#ifndef SVMP_ACCESS_VIA_PHYTBL
	svmp_addr = (uint16 *)(wlc_hw->vasip_addr + offset);
#endif // endif

	for (i = 0; i < len; i++) {
#ifdef SVMP_ACCESS_VIA_PHYTBL
		ret_svmp_addr[i] =
			phy_misc_vasip_svmp_read((phy_info_t *)wlc_hw->band->pi, offset+i);
#else
		ret_svmp_addr[i] = svmp_addr[i];
#endif // endif
	}
	return BCME_OK;
}

/* set svmp memory with a value from offset of length 'len', len is count of uint16's */
static int
wlc_svmp_mem_set(wlc_hw_info_t *wlc_hw, uint32 offset, uint16 len, uint16 val)
{
#ifndef SVMP_ACCESS_VIA_PHYTBL
	volatile uint16 * svmp_addr;
#endif // endif
	uint16 i;
	uint32 svmp_mem_offset_max;

	if (!VASIP_PRESENT(wlc_hw->corerev)) {
		return BCME_UNSUPPORTED;
	}

	if (!wlc_hw->clk) {
		return BCME_NOCLK;
	}

	svmp_mem_offset_max = wlc_svmp_mem_offset_max(wlc_hw);
	if ((offset + len) > svmp_mem_offset_max) {
		return BCME_RANGE;
	}

#ifndef SVMP_ACCESS_VIA_PHYTBL
	svmp_addr = (volatile uint16 *)(wlc_hw->vasip_addr + offset);
#endif // endif

	for (i = 0; i < len; i++) {
#ifdef SVMP_ACCESS_VIA_PHYTBL
		phy_misc_vasip_svmp_write((phy_info_t *)wlc_hw->band->pi, offset+i, val);
#else
		svmp_addr[i] = val;
#endif // endif
	}
	return BCME_OK;
}
#endif /* BCMDBG */

/* set svmp memory with a value from offset of length 'len', len is count of uint16's */
int
wlc_svmp_mem_blk_set(wlc_hw_info_t *wlc_hw, uint32 offset, uint16 len, uint16 *val)
{
#ifndef SVMP_ACCESS_VIA_PHYTBL
	volatile uint16 * svmp_addr;
#endif // endif
	uint16 i;
	uint32 svmp_mem_offset_max;

	if (!VASIP_PRESENT(wlc_hw->corerev)) {
		return BCME_UNSUPPORTED;
	}

	if (!wlc_hw->clk) {
		return BCME_NOCLK;
	}

	svmp_mem_offset_max = wlc_svmp_mem_offset_max(wlc_hw);
	if ((offset + len) > svmp_mem_offset_max) {
		return BCME_RANGE;
	}

#ifndef SVMP_ACCESS_VIA_PHYTBL
	svmp_addr = (volatile uint16 *)(wlc_hw->vasip_addr + offset);
#endif // endif

	for (i = 0; i < len; i++) {
		if (val[i] != 0xffff) {
#ifdef SVMP_ACCESS_VIA_PHYTBL
			phy_misc_vasip_svmp_write((phy_info_t *)wlc_hw->band->pi, offset+i, val[i]);
#else
			svmp_addr[i] = val[i];
#endif // endif
		}
	}
	return BCME_OK;
}

#ifdef WL_AIR_IQ
/* copy svmp memory to a buffer starting from offset of length 'len', len is
 * count of uint64's
 */
int
wlc_svmp_mem_read64(wlc_hw_info_t *wlc_hw, uint64 *ret_svmp_addr, uint32 offset, uint16 len)
{
	uint64 * svmp_addr;
	uint16 i;

	if (((offset + (len * sizeof(*ret_svmp_addr))) > SVMP_MEM_OFFSET_MAX_BCM4365C0) ||
		(len * sizeof(*ret_svmp_addr)) > SVMP_MEM_DUMP_LEN_MAX) {
		return BCME_RANGE;
	}
	offset = offset >> 2;
	svmp_addr = (uint64 *)((uint64 *)wlc_hw->vasip_addr + offset);

	for (i = 0; i < len; i++) {
		ret_svmp_addr[i] = svmp_addr[i];
	}

	return BCME_OK;
}
/* set svmp memory with a value from offset of length 'len', len is count of uint16's */
int
wlc_svmp_mem_set_axi(wlc_hw_info_t *wlc_hw, uint32 offset, uint16 len, uint16 val)
{
	volatile uint16 * svmp_addr;
	uint16 i;

	svmp_addr = (volatile uint16 *)(wlc_hw->vasip_addr + offset);

	for (i = 0; i < len; i++) {
		svmp_addr[i] = val;
	}
	return BCME_OK;
}

int
wlc_svmp_mem_read_axi(wlc_hw_info_t *wlc_hw, uint16 *ret_svmp_addr, uint32 offset, uint16 len)
{
	uint16 * svmp_addr;
	uint16 i;

	svmp_addr = (uint16 *)(wlc_hw->vasip_addr + offset);

	for (i = 0; i < len; i++) {
		ret_svmp_addr[i] = svmp_addr[i];
	}
	return BCME_OK;
}
#endif /* WL_AIR_IQ */

int
wlc_svmp_read_blk(wlc_hw_info_t *wlc_hw, uint16 *val, uint32 offset, uint16 len)
{
#ifndef SVMP_ACCESS_VIA_PHYTBL
	uint16 *svmp_addr;
#endif // endif
	uint16 i;
	uint32 svmp_mem_offset_max;

	if (!VASIP_PRESENT(wlc_hw->corerev)) {
		return BCME_UNSUPPORTED;
	}

	if (!wlc_hw->clk) {
		return BCME_NOCLK;
	}

	svmp_mem_offset_max = wlc_svmp_mem_offset_max(wlc_hw);
	if ((offset + len) > svmp_mem_offset_max) {
		return BCME_RANGE;
	}

#ifndef SVMP_ACCESS_VIA_PHYTBL
	svmp_addr = (uint16 *)(wlc_hw->vasip_addr + offset);
	for (i = 0; i < len; i++) {
		val[i] = svmp_addr[i];
	}
#else
	//phy_misc_vasip_svmp_read_blk((phy_info_t *)wlc_hw->band->pi, offset, len, val);
	for (i = 0; i < (len/SVMP_ACCESS_BLOCK_SIZE); i++) {
		phy_misc_vasip_svmp_read_blk((phy_info_t *)wlc_hw->band->pi,
		        offset, SVMP_ACCESS_BLOCK_SIZE, val);
		offset += SVMP_ACCESS_BLOCK_SIZE;
		val += SVMP_ACCESS_BLOCK_SIZE;
		len -= SVMP_ACCESS_BLOCK_SIZE;
	}
	if (len > 0) {
		phy_misc_vasip_svmp_read_blk((phy_info_t *)wlc_hw->band->pi, offset, len, val);
	}
#endif /* SVMP_ACCESS_VIA_PHYTBL */

	return BCME_OK;
}

int
wlc_svmp_write_blk(wlc_hw_info_t *wlc_hw, uint16 *val, uint32 offset, uint16 len)
{
#ifndef SVMP_ACCESS_VIA_PHYTBL
	volatile uint16 * svmp_addr;
#endif // endif
	uint16 i;
	uint32 svmp_mem_offset_max;

	if (!VASIP_PRESENT(wlc_hw->corerev)) {
		return BCME_UNSUPPORTED;
	}

	if (!wlc_hw->clk) {
		return BCME_NOCLK;
	}

	svmp_mem_offset_max = wlc_svmp_mem_offset_max(wlc_hw);
	if ((offset + len) > svmp_mem_offset_max) {
		return BCME_RANGE;
	}

#ifndef SVMP_ACCESS_VIA_PHYTBL
	svmp_addr = (volatile uint16 *)(wlc_hw->vasip_addr + offset);
	for (i = 0; i < len; i++) {
		if (val[i] != 0xffff) {
			svmp_addr[i] = val[i];
		}
	}
#else
	//phy_misc_vasip_svmp_write_blk((phy_info_t *)wlc_hw->band->pi, offset, len, val);
	for (i = 0; i < (len/SVMP_ACCESS_BLOCK_SIZE); i++) {
		phy_misc_vasip_svmp_write_blk((phy_info_t *)wlc_hw->band->pi,
		        offset, SVMP_ACCESS_BLOCK_SIZE, val);
		offset += SVMP_ACCESS_BLOCK_SIZE;
		val += SVMP_ACCESS_BLOCK_SIZE;
		len -= SVMP_ACCESS_BLOCK_SIZE;
	}
	if (len > 0) {
		phy_misc_vasip_svmp_write_blk((phy_info_t *)wlc_hw->band->pi, offset, len, val);
	}
#endif /* SVMP_ACCESS_VIA_PHYTBL */

	return BCME_OK;
}

int
wlc_svmp_mem_offset_max(wlc_hw_info_t *wlc_hw)
{
	uint32 svmp_mem_offset_max;

	if (D11REV_IS(wlc_hw->corerev, 64)) {
		svmp_mem_offset_max = SVMP_MEM_OFFSET_MAX_BCM4365B1;
	} else if (D11REV_IS(wlc_hw->corerev, 65)) {
		svmp_mem_offset_max = SVMP_MEM_OFFSET_MAX_BCM4365C0;
	} else {
		svmp_mem_offset_max = SVMP_MEM_OFFSET_MAX_NOT_SUPPORT;
	}

	return svmp_mem_offset_max;
}
#endif /* VASIP_HW_SUPPORT */
