/*
 * HND GPIO control interface
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
 * Implementation of GPIO access functions
 *
 * $Id: $
 */
#ifdef WLGPIOHLR

#include <bcm_cfg.h>
#include <typedefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <rte_chipc.h>
#include <rte_gpio.h>
#include "rte_priv.h"
#include "rte_chipc_priv.h"

#ifdef ATE_BUILD
#include "wl_ate.h"
#endif // endif

typedef struct rte_gpioh {
	void			*arg;
	bool			level;
	gpio_handler_t		handler;
	uint32			event;
	struct rte_gpioh	*next;
} rte_gpioh_t;

typedef struct rte_gpio {
	si_t	*sih;
	rte_gpioh_t *gpioh_head;       /* GPIO event handlers list */
} rte_gpio_t;
static rte_gpio_t *gpio_h = NULL;

static rte_gpio_t * rte_gpio_get_handle(void);
static void rte_gpio_set_handle(rte_gpio_t *gph);

rte_gpioh_t *
BCMATTACHFN(rte_gpio_handler_register)(uint32 event,
	bool level, gpio_handler_t cb, void *arg)
{
	si_t *sih;
	rte_gpioh_t *gi;

	ASSERT(gpio_h);
	ASSERT(event);
	ASSERT(cb != NULL);

	sih = gpio_h->sih;

	if ((gi = MALLOCZ(si_osh(sih), sizeof(rte_gpioh_t))) == NULL)
		return NULL;

	bzero(gi, sizeof(rte_gpioh_t));
	gi->event = event;
	gi->handler = cb;
	gi->arg = arg;
	gi->level = level;

	gi->next = gpio_h->gpioh_head;
	gpio_h->gpioh_head = gi;

#ifdef BCMDBG_ERR
	{
		rte_gpioh_t *h = gpio_h->gpioh_head;
		int cnt = 0;

		for (; h; h = h->next) {
			cnt++;
			printf("gpiohdler=%p cb=%p event=0x%x\n",
				h, h->handler, h->event);
		}
		printf("gpiohdler total=%d\n", cnt);
	}
#endif // endif
	return (void *)(gi);
}

void
BCMATTACHFN(rte_gpio_handler_unregister)(rte_gpioh_t *gpioh)
{
	rte_gpioh_t *p, *n;

	ASSERT(gpio_h);
	ASSERT(gpio_h->gpioh_head != NULL);

	if ((void*)gpio_h->gpioh_head == gpioh) {
		gpio_h->gpioh_head = gpio_h->gpioh_head->next;
		MFREE(si_osh(gpio_h->sih), gpioh, sizeof(rte_gpioh_t));
		return;
	} else {
		p = gpio_h->gpioh_head;
		n = p->next;
		while (n) {
			if ((void*)n == gpioh) {
				p->next = n->next;
				MFREE(si_osh(gpio_h->sih), gpioh, sizeof(rte_gpioh_t));
				return;
			}
			p = n;
			n = n->next;
		}
	}

#ifdef BCMDBG_ERR
	{
		rte_gpioh_t *h = gpio_h->gpioh_head;
		int cnt = 0;

		for (; h; h = h->next) {
			cnt++;
			printf("gpiohdler=%p cb=%p event=0x%x\n",
				h, h->handler, h->event);
		}
		printf("gpiohdler total=%d\n", cnt);
	}
#endif // endif
	ASSERT(0); /* Not found in list */
}
#ifndef ATE_BUILD
static void
rte_gpio_handler_process(void *gph)
{
	rte_gpio_t *gpio_p = gph;
	si_t *sih = gpio_p->sih;
	rte_gpioh_t *h;
	uint32 level = si_gpioin(sih);
	uint32 levelp = si_gpiointpolarity(sih, 0, 0, 0);
	uint32 edge = si_gpioevent(sih, GPIO_REGEVT, 0, 0);
	uint32 edgep = si_gpioevent(sih, GPIO_REGEVT_INTPOL, 0, 0);

	for (h = gpio_p->gpioh_head; h != NULL; h = h->next) {
		if (h->handler) {
			uint32 status = (h->level ? level : edge) & h->event;
			uint32 polarity = (h->level ? levelp : edgep) & h->event;

			/* polarity bitval is opposite of status bitval */
			if ((h->level && (status ^ polarity)) || (!h->level && status))
				h->handler(status, h->arg);
		}
	}

	si_gpioevent(sih, GPIO_REGEVT, edge, edge); /* clear edge-trigger status */
}
#endif /* !ATE_BUILD */
static void
rte_gpio_run(void* cbdata, uint32 ccintst)
{
#ifdef ATE_BUILD
	ate_params.cmd_proceed = TRUE;
#else
	rte_gpio_handler_process(cbdata);
#endif // endif
}

static void
rte_gpio_isr(void* cbdata, uint32 ccintst)
{
#ifndef THREAD_SUPPORT
	rte_gpio_run(cbdata, ccintst);
#endif	/* THREAD_SUPPORT */
}

#ifdef THREAD_SUPPORT
static void
rte_gpio_dpc(void* cbdata, uint32 ccintst)
{
	rte_gpio_run(cbdata, ccintst);
}
#endif	/* THREAD_SUPPORT */

int
BCMATTACHFN(rte_gpio_init)(si_t *sih)
{
	osl_t *osh = si_osh(sih);

	if (sih->ccrev < 11)
		return BCME_ERROR;

	if (!rte_gpio_get_handle()) {
		gpio_h = MALLOCZ(osh, sizeof(rte_gpio_t));
		if (!gpio_h)
			return BCME_ERROR;

		gpio_h->sih = sih;

		si_cc_register_isr(sih, rte_gpio_isr, CI_GPIO, (void *)gpio_h);
#ifdef THREAD_SUPPORT
		si_cc_register_dpc(sih, rte_gpio_dpc, CI_GPIO, (void *)sih);
#endif	/* THREAD_SUPPORT */
		si_gpio_int_enable(sih, TRUE);
		rte_gpio_set_handle(gpio_h);
	}

	return BCME_OK;
}

/*
 * accessor functions for "gpio_h"
 * By making the function (BCMRAMFN), prevents "gpio_h" going
 * into ROM/RAM shared memory.
 */
static rte_gpio_t *
BCMRAMFN(rte_gpio_get_handle)(void)
{
	return gpio_h;
}

static void
BCMRAMFN(rte_gpio_set_handle)(rte_gpio_t *gph)
{
	gpio_h = gph;
}
#endif /* WLGPIOHLR */
