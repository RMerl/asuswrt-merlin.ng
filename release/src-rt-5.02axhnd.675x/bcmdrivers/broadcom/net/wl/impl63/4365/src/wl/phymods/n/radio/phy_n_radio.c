/*
 * NPHY RADIO contorl module implementation
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
 * $Id$
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmdevs.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include <phy_radio.h>
#include "phy_type_radio.h"
#include <phy_n.h>
#include <phy_n_radio.h>
#include "phy_utils_reg.h"

#include <wlc_phy_radio.h>

/* module private states */
struct phy_n_radio_info {
	phy_info_t *pi;
	phy_n_info_t *ni;
	phy_radio_info_t *ri;
};

/* local functions */
static void phy_n_radio_switch(phy_type_radio_ctx_t *ctx, bool on);
static void phy_n_radio_on(phy_type_radio_ctx_t *ctx);
static void phy_n_radio_off_bandx(phy_type_radio_ctx_t *ctx);
static void phy_n_radio_off_init(phy_type_radio_ctx_t *ctx);
static uint32 _phy_n_radio_query_idcode(phy_type_radio_ctx_t *ctx);
#if (defined(BCMDBG) || defined(BCMDBG_DUMP)) && defined(DBG_PHY_IOV)
static int phy_n_radio_dump(phy_type_radio_ctx_t *ctx, struct bcmstrbuf *b);
#else
#define phy_n_radio_dump NULL
#endif // endif

/* Register/unregister NPHY specific implementation to common layer */
phy_n_radio_info_t *
BCMATTACHFN(phy_n_radio_register_impl)(phy_info_t *pi, phy_n_info_t *ni,
	phy_radio_info_t *ri)
{
	phy_n_radio_info_t *info;
	phy_type_radio_fns_t fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate all storage in once */
	if ((info = phy_malloc(pi, sizeof(phy_n_radio_info_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	bzero(info, sizeof(phy_n_radio_info_t));
	info->pi = pi;
	info->ni = ni;
	info->ri = ri;

	if (NREV_GE(pi->pubpi.phy_rev, 7)) {
		if (NREV_IS(pi->pubpi.phy_rev, 19))
			pi->pubpi.radiooffset = RADIO_20671_READ_OFF;
		else
			pi->pubpi.radiooffset = RADIO_2057_READ_OFF;
	} else
		pi->pubpi.radiooffset = RADIO_2055_READ_OFF;  /* works for 2056 too */

#ifndef BCM_OL_DEV
	/* make sure the radio is off until we do an "up" */
	phy_n_radio_switch(info, OFF);
#endif // endif

	/* Register PHY type specific implementation */
	bzero(&fns, sizeof(fns));
	fns.ctrl = phy_n_radio_switch;
	fns.on = phy_n_radio_on;
	fns.bandx = phy_n_radio_off_bandx;
	fns.init = phy_n_radio_off_init;
	fns.id = _phy_n_radio_query_idcode;
	fns.dump = phy_n_radio_dump;
	fns.ctx = info;

	phy_radio_register_impl(ri, &fns);

	return info;

fail:
	if (info != NULL)
		phy_mfree(pi, info, sizeof(phy_n_radio_info_t));
	return NULL;
}

void
BCMATTACHFN(phy_n_radio_unregister_impl)(phy_n_radio_info_t *info)
{
	phy_info_t *pi = info->pi;
	phy_radio_info_t *ri = info->ri;

	PHY_TRACE(("%s\n", __FUNCTION__));

	phy_radio_unregister_impl(ri);

	phy_mfree(pi, info, sizeof(phy_n_radio_info_t));
}

/* switch radio on/off */
static void
phy_n_radio_switch(phy_type_radio_ctx_t *ctx, bool on)
{
	phy_n_radio_info_t *info = (phy_n_radio_info_t *)ctx;
	phy_info_t *pi = info->pi;

	PHY_TRACE(("wl%d: %s %d\n", pi->sh->unit, __FUNCTION__, on));

	wlc_phy_switch_radio_nphy(pi, on);
}

/* turn radio on */
static void
WLBANDINITFN(phy_n_radio_on)(phy_type_radio_ctx_t *ctx)
{
	phy_n_radio_switch(ctx, ON);
}

/* switch radio off when switching band */
static void
phy_n_radio_off_bandx(phy_type_radio_ctx_t *ctx)
{
	/* the radio is left off when switching band if we do it here */
	/* temporarily turn it off until the problem is understood and fixed */
	phy_n_radio_info_t *info = (phy_n_radio_info_t *)ctx;
	phy_info_t *pi = info->pi;

	(void)pi;

	if (NREV_GE(pi->pubpi.phy_rev, 3))
		return;

	phy_n_radio_switch(ctx, OFF);
}

/* switch radio off when initializing */
static void
phy_n_radio_off_init(phy_type_radio_ctx_t *ctx)
{
	phy_n_radio_switch(ctx, OFF);
}

/* query radio idcode */
static uint32
_phy_n_radio_query_idcode(phy_type_radio_ctx_t *ctx)
{
	phy_n_radio_info_t *info = (phy_n_radio_info_t *)ctx;
	phy_info_t *pi = info->pi;

	return phy_n_radio_query_idcode(pi);
}

uint32
phy_n_radio_query_idcode(phy_info_t *pi)
{
	uint32 idcode;

	/* XXX: Router build failures using this:
	 * XXX:   D11REV_IS(pi->sh->corerev, 33)
	 * XXX: Temp fix for now.
	 */
	if (NREV_GE(pi->pubpi.phy_rev, LCNXN_BASEREV + 3)) {
		uint32 rnum;

		W_REG(pi->sh->osh, &pi->regs->radioregaddr, 0);
		rnum = (uint32)R_REG(pi->sh->osh, &pi->regs->radioregdata);
		if (CHIP_4324_B3(pi) || CHIP_4324_B4(pi) || CHIP_4324_B5(pi)) {
			rnum = (rnum >> 4) & 0xF;
		} else {
			rnum = (rnum >> 0) & 0xF;
		}
		if (CHIPID_4324X_MEDIA_A1(pi)) {
			ASSERT(rnum == 1);
			/* patch radiorev to 2 to differentiate from 4324B4 */
			rnum = 2;
		}

		W_REG(pi->sh->osh, &pi->regs->radioregaddr, 1);
		idcode = (R_REG(pi->sh->osh, &pi->regs->radioregdata) <<
			IDCODE_ID_SHIFT) | (rnum << IDCODE_REV_SHIFT);
	}
	else if (D11REV_GE(pi->sh->corerev, 24)) {
		uint32 b0, b1, b2;

		W_REG(pi->sh->osh, &pi->regs->radioregaddr, 0);
#ifdef __mips__
		(void)R_REG(pi->sh->osh, &pi->regs->radioregaddr);
#endif // endif
		b0 = (uint32)R_REG(pi->sh->osh, &pi->regs->radioregdata);
		W_REG(pi->sh->osh, &pi->regs->radioregaddr, 1);
#ifdef __mips__
		(void)R_REG(pi->sh->osh, &pi->regs->radioregaddr);
#endif // endif
		b1 = (uint32)R_REG(pi->sh->osh, &pi->regs->radioregdata);

		W_REG(pi->sh->osh, &pi->regs->radioregaddr, 2);
#ifdef __mips__
		(void)R_REG(pi->sh->osh, &pi->regs->radioregaddr);
#endif // endif
		b2 = (uint32)R_REG(pi->sh->osh, &pi->regs->radioregdata);
		idcode = ((b0  & 0xf) << 28) | (((b2 << 8) | b1) << 12) | ((b0 >> 4) & 0xf);
	}
	else {
		W_REG(pi->sh->osh, &pi->regs->phy4waddr, RADIO_IDCODE);
#ifdef __mips__
		(void)R_REG(pi->sh->osh, &pi->regs->phy4waddr);
#endif // endif
		idcode = (uint32)R_REG(pi->sh->osh, &pi->regs->phy4wdatalo);
		idcode |= (uint32)R_REG(pi->sh->osh, &pi->regs->phy4wdatahi) << 16;
	}

	return idcode;
}

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
#if defined(DBG_PHY_IOV)
static int
phy_n_radio_dump(phy_type_radio_ctx_t *ctx, struct bcmstrbuf *b)
{
	phy_n_radio_info_t *gi = (phy_n_radio_info_t *)ctx;
	phy_info_t *pi = gi->pi;
	const char *name = NULL;
	int i, core_cnt, jtag_core;
	uint16 addr = 0;
	radio_regs_t *radioregs = NULL;
	radio_regs_t *radioregs_tx = NULL;
	radio_regs_t *radioregs_rx = NULL;
	radio_20xx_regs_t *radio20xxregs = NULL;
	radio_20671_regs_t *radio20671regs = NULL;

	if (RADIOID(pi->pubpi.radioid) == BCM2055_ID) {
		radioregs = regs_2055;
		name = "2055";
	} else if (RADIOID(pi->pubpi.radioid) == BCM2056_ID) {
		if (NREV_LE(pi->pubpi.phy_rev, 4)) {
			radioregs = regs_SYN_2056;
			radioregs_tx = regs_TX_2056;
			radioregs_rx = regs_RX_2056;
		} else {
			switch (RADIOREV(pi->pubpi.radiorev)) {
			case 5:
				radioregs = regs_SYN_2056_rev5;
				radioregs_tx = regs_TX_2056_rev5;
				radioregs_rx = regs_RX_2056_rev5;
				break;

			case 6:
				radioregs = regs_SYN_2056_rev6;
				radioregs_tx = regs_TX_2056_rev6;
				radioregs_rx = regs_RX_2056_rev6;
				break;

			case 7:
			case 9: /* Radio id rev7 and rev9 have the same register settings */
				radioregs = regs_SYN_2056_rev7;
				radioregs_tx = regs_TX_2056_rev7;
				radioregs_rx = regs_RX_2056_rev7;
				break;

			case 8:
			case 11:
				radioregs = regs_SYN_2056_rev8;
				radioregs_tx = regs_TX_2056_rev8;
				radioregs_rx = regs_RX_2056_rev8;
				break;

			default:
				PHY_ERROR(("Unsupported radio rev %d\n",
					RADIOREV(pi->pubpi.radiorev)));
				ASSERT(0);
				break;
			}
		}
		name = "2056";
	} else if (RADIOID(pi->pubpi.radioid) == BCM2057_ID) {
		switch (RADIOREV(pi->pubpi.radiorev)) {
		case 3:
			radio20xxregs = regs_2057_rev4;
			break;
		case 4:
			radio20xxregs = regs_2057_rev4;
			break;
		case 5:
			if (RADIOVER(pi->pubpi.radiover) == 0x0) {
				/* A0 */
				radio20xxregs = regs_2057_rev5;
			} else if (RADIOVER(pi->pubpi.radiover) == 0x1) {
				/* B0 */
				radio20xxregs = regs_2057_rev5v1;
			}
			break;
		case 6:
			radio20xxregs = regs_2057_rev4;
			break;
		case 7:
			radio20xxregs = regs_2057_rev7;
			break;
		case 8:
			radio20xxregs = regs_2057_rev8;
			break;
		case 9:
			radio20xxregs = regs_2057_rev9;
			break;
		case 10:
			radio20xxregs = regs_2057_rev10;
			break;
		case 12:
			radio20xxregs = regs_2057_rev12;
			break;
		case 13:
			radio20xxregs = regs_2057_rev13;
			break;
		case 14:
			if (RADIOVER(pi->pubpi.radiover) == 1)
				radio20xxregs = regs_2057_rev14v1;
			else
				radio20xxregs = regs_2057_rev14;
			break;
		default:
			PHY_ERROR(("Unsupported radio rev %d\n", RADIOREV(pi->pubpi.radiorev)));
			ASSERT(0);
			break;
		}
		name = "2057";
	} else if (RADIOID(pi->pubpi.radioid) == BCM20671_ID) {
		switch (RADIOREV(pi->pubpi.radiorev)) {
		case 0:
			radio20671regs = regs_20671_rev0;
			break;
		case 1:
			if (RADIOVER(pi->pubpi.radiover) == 0)
				radio20671regs = regs_20671_rev1;
			else if (RADIOVER(pi->pubpi.radiover) == 1)
				radio20671regs = regs_20671_rev1_ver1;
			break;
		default:
			PHY_ERROR(("Unsupported radio rev %d\n", RADIOREV(pi->pubpi.radiorev)));
			ASSERT(0);
			break;
		}
		BCM_REFERENCE(radio20671regs);
		name = "20671";
	}

	if (name == NULL)
		return BCME_ERROR;

	bcm_bprintf(b, "----- %08s -----\n", name);
	bcm_bprintf(b, "Add Value\n");

	i = 0;
	while (TRUE) {
		if (radioregs) {
			/* omit spare and radio ID */
			addr = radioregs[i].address;
		} else if (radio20xxregs) {
			addr = radio20xxregs[i].address;
		} else {
			addr = 0xffff;
		}

		if (addr == 0xffff)
			break;

		if (RADIOID(pi->pubpi.radioid) == BCM2056_ID) {
			jtag_core = RADIO_2056_SYN;
			name = "2056 SYN";
		} else {
			jtag_core = 0;
		}

		bcm_bprintf(b, "%03x %04x\n", addr, phy_utils_read_radioreg(pi, addr | jtag_core));
		i++;
	}

	if (RADIOID(pi->pubpi.radioid) == BCM2056_ID) {
		for (core_cnt = 0; core_cnt <= 1; core_cnt++) {
			if (core_cnt == 0) {
				jtag_core = RADIO_2056_TX0;
				name = "2056 TX0";
			} else {
				jtag_core = RADIO_2056_TX1;
				name = "2056 TX1";
			}

			i = 0;
			while (TRUE) {
				if (radioregs_tx) {
					/* omit spare and radio ID */
					addr = radioregs_tx[i].address;
				} else {
					addr = 0xffff;
				}

				if (addr == 0xffff)
					break;

				bcm_bprintf(b, "%03x %04x\n", addr,
				            phy_utils_read_radioreg(pi, addr | jtag_core));
				i++;
			}
		}
	}

	if (RADIOID(pi->pubpi.radioid) == BCM2056_ID) {
		for (core_cnt = 0; core_cnt <= 1; core_cnt++) {
			if (core_cnt == 0) {
				jtag_core = RADIO_2056_RX0;
				name = "2056 RX0";
			} else {
				jtag_core = RADIO_2056_RX1;
				name = "2056 RX1";
			}

			i = 0;
			while (TRUE) {
				if (radioregs_rx) {
					/* omit spare and radio ID */
					addr = radioregs_rx[i].address;
				} else {
					addr = 0xffff;
				}

				if (addr == 0xffff)
					break;

				bcm_bprintf(b, "%03x %04x\n", addr,
				            phy_utils_read_radioreg(pi, addr | jtag_core));
				i++;
			}
		}
	}

	return BCME_OK;
}
#endif // endif
#endif /* BCMDBG || BCMDBG_DUMP */
