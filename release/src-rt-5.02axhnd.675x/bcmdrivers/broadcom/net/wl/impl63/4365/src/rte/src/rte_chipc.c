/*
 * RTE support code for chipcommon & misc. subcores
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
 * $Id: rte_chipc.c 580528 2015-08-19 11:48:07Z $
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmdevs.h>
#include <bcmutils.h>
#include <sbchipc.h>
#include <hndsoc.h>
#include <rte_dev.h>
#include <rte_chipc.h>
#include "rte_chipc_priv.h"
#include <rte_isr.h>
#include "rte_priv.h"

/* interested chipcommon interrupt source
 *  - GPIO
 *  - EXTIF
 *  - ECI
 *  - PMU
 *  - UART
 */
#define	MAX_CC_INT_SOURCE 5

typedef void (*cc_reg_cb_t)(void *cbdata, uint32 ccintstatus);

/* chipc secondary isr info */
typedef struct {
	uint intmask;		/* int mask */
	cc_reg_cb_t cb;		/* secondary isr/dpc handler */
	void *cbdata;		/* pointer to private data */
} cc_reg_ent_t;

typedef struct {
	cc_reg_ent_t cc_desc[MAX_CC_INT_SOURCE];
	uint32 cc_intstatus;
	uint32 cc_intmask;
} cc_reg_t;

static cc_reg_t cc_isr;
#ifdef THREAD_SUPPORT
static cc_reg_t cc_dpc;
#endif	/* THREAD_SUPPORT */

chipcregs_t *hnd_ccr = NULL;	/* Chipc core regs */

/*
 * ROM accessor to avoid struct in shdat
 */
static cc_reg_t *
BCMRAMFN(get_cc_isr)(void)
{
	return &cc_isr;
}

#ifdef THREAD_SUPPORT
static cc_reg_t *
BCMRAMFN(get_cc_dpc)(void)
{
	return &cc_dpc;
}
#endif /* THREAD_SUPPORT */

/* register callback to ISR registry or DPC registry */
static bool
BCMATTACHFN(cc_register_cb)(cc_reg_t *cc_h,
	si_t *sih, cc_reg_cb_t cb, uint32 ccintmask, void *cbdata)
{
	bool done = FALSE;
	chipcregs_t *regs;
	uint origidx;
	cc_reg_ent_t *cc_desc;
	uint i;

	if (cc_h == NULL)
		return FALSE;

	cc_desc = cc_h->cc_desc;

	/* Save the current core index */
	origidx = si_coreidx(sih);
	regs = si_setcoreidx(sih, SI_CC_IDX);
	ASSERT(regs);

	for (i = 0; i < MAX_CC_INT_SOURCE; i++) {
		if (cc_desc[i].cb == NULL) {
			cc_desc[i].cb = cb;
			cc_desc[i].cbdata = cbdata;
			cc_desc[i].intmask = ccintmask;
			done = TRUE;
			break;
		}
	}

	if (done) {
		cc_h->cc_intmask = R_REG(si_osh(sih), &regs->intmask);
		cc_h->cc_intmask |= ccintmask;
		W_REG(si_osh(sih), &regs->intmask, cc_h->cc_intmask);
	}

	/* restore original coreidx */
	si_setcoreidx(sih, origidx);
	return done;
}

/*
 * Interface to register chipc secondary isr
 */

bool
BCMATTACHFN(si_cc_register_isr)(si_t *sih, cc_isr_fn isr, uint32 ccintmask, void *cbdata)
{
	return cc_register_cb(get_cc_isr(), sih, (cc_reg_cb_t)isr, ccintmask, cbdata);
}

/*
 * Interface to register chipc secondary dpc
 */

#ifdef THREAD_SUPPORT
bool
BCMATTACHFN(si_cc_register_dpc)(si_t *sih, cc_dpc_fn dpc, uint32 ccintmask, void *cbdata)
{
	return cc_register_cb(get_cc_dpc(), sih, (cc_reg_cb_t)dpc, ccintmask, cbdata);
}
#endif	/* THREAD_SUPPORT */

static void
run_cb(cc_reg_ent_t *desc, uint32 ccintstatus)
{
	uint32 intstatus;
	uint32 i;

	ASSERT(desc);
	for (i = 0; i < MAX_CC_INT_SOURCE; i++, desc++) {
		if ((desc->cb != NULL) &&
		    (intstatus = (desc->intmask & ccintstatus))) {
			(desc->cb)(desc->cbdata, intstatus);
		}
	}
}

/*
 * chipc primary interrupt handler
 *
 */

static void
si_cc_isr(si_t *sih, chipcregs_t *regs)
{
	uint32 ccintstatus;
	cc_reg_t *cc_isr_h = get_cc_isr();
	cc_reg_ent_t *desc = cc_isr_h->cc_desc;

	/* prior to rev 21 chipc interrupt means uart and gpio */
	if (sih->ccrev >= 21)
		ccintstatus = R_REG(si_osh(sih),
			&regs->intstatus) & cc_isr_h->cc_intmask;
	else
		ccintstatus = (CI_UART | CI_GPIO);

#ifdef THREAD_SUPPORT
	cc_isr_h->cc_intstatus = ccintstatus;

	/* disable interrupts */
	W_REG(si_osh(sih), &regs->intmask, 0);
#endif	/* THREAD_SUPPORT */

	run_cb(desc, ccintstatus);
}

#ifndef RTE_POLL
#ifdef THREAD_SUPPORT
static void
si_cc_dpc(si_t *sih, chipcregs_t *regs)
{
	uint32 ccintstatus;
	cc_reg_t *cc_isr_h = get_cc_isr();
	cc_reg_t *cc_dpc_h = get_cc_dpc();
	cc_reg_ent_t *desc = cc_dpc_h->cc_desc;

	ccintstatus = cc_isr_h->cc_intstatus;

	run_cb(desc, ccintstatus);

	/* reenable interrupts */
	W_REG(si_osh(sih), &regs->intmask, cc_isr_h->cc_intmask);
}
#endif	/* THREAD_SUPPORT */
#endif /* !RTE_POLL */

/* ======HND====== misc
 *     chipc init
 */

static void
hnd_chipc_isr(void *cbdata)
{
	si_cc_isr(get_hnd_sih(), hnd_ccr);
}

#ifndef RTE_POLL
#ifdef THREAD_SUPPORT
static void
hnd_chipc_dpc(void *cbdata)
{
	si_cc_dpc(get_hnd_sih(), hnd_ccr);
}
#endif	/* THREAD_SUPPORT */
#endif /* !RTE_POLL */

#ifdef RTE_POLL
static void *
BCMATTACHFN(hnd_chipc_probe)(hnd_dev_t *dev, void *regs, uint bus,
                             uint16 device, uint coreid, uint unit)
{
	return regs;
}

static void
hnd_chipc_poll(hnd_dev_t *dev)
{
	hnd_chipc_isr(dev);
}

static hnd_dev_ops_t chipc_funcs = {
	probe:		hnd_chipc_probe,
	poll:		hnd_chipc_poll
};

static hnd_dev_t chipc_dev = {
	name:		"cc",
	ops:		&chipc_funcs
};
#endif	/* RTE_POLL */

void
BCMATTACHFN(hnd_chipc_init)(si_t *sih)
{
	bzero(&cc_isr, sizeof(cc_isr));
#ifdef THREAD_SUPPORT
	bzero(&cc_dpc, sizeof(cc_dpc));
#endif // endif

	/* get chipcommon and its sbconfig addr */
	hnd_ccr = si_setcoreidx(sih, SI_CC_IDX);

	/* only support chips that have chipcommon */
	ASSERT(hnd_ccr);

	/* register polling dev or isr */
#ifdef RTE_POLL
	hnd_add_device(sih, &chipc_dev, CC_CORE_ID, BCM4710_DEVICE_ID);
#else
	if (hnd_isr_register(0, CC_CORE_ID, 0, hnd_chipc_isr, NULL, SI_BUS) != BCME_OK ||
#ifdef THREAD_SUPPORT
	    hnd_dpc_register(0, CC_CORE_ID, 0, hnd_chipc_dpc, NULL, SI_BUS) != BCME_OK ||
#endif // endif
	    FALSE)
		hnd_die();
#endif	/* !RTE_POLL */
}

/* ======HND====== misc
 *     gci init
 *     eci init
 */

static void
hnd_gci_isr(void* cbdata, uint32 ccintst)
{
#ifndef THREAD_SUPPORT
	si_t *sih = (si_t *)cbdata;
	si_gci_handler_process(sih);
#endif	/* THREAD_SUPPORT */
}

#ifdef THREAD_SUPPORT
static void
hnd_gci_dpc(void* cbdata, uint32 ccintst)
{
	si_t *sih = (si_t *)cbdata;
	si_gci_handler_process(sih);
}
#endif	/* THREAD_SUPPORT */

void
BCMATTACHFN(hnd_gci_init)(si_t *sih)
{
	if (!(sih->cccaps_ext & CC_CAP_EXT_GCI_PRESENT))
		return;

	si_cc_register_isr(sih, hnd_gci_isr, CI_ECI, (void *)sih);
#ifdef THREAD_SUPPORT
	si_cc_register_dpc(sih, hnd_gci_dpc, CI_ECI, (void *)sih);
#endif	/* THREAD_SUPPORT */
}

#ifdef BCMECICOEX
void
BCMATTACHFN(hnd_eci_init)(si_t *sih)
{
	if (sih->ccrev < 21)
		return;
	si_eci_init(sih);
}
#endif	/* BCMECICOEX */
