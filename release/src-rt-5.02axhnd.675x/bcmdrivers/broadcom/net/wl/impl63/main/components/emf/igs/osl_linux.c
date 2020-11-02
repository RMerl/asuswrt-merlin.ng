/*
 * Timer functions used by EMFL. These Functions can be moved to
 * shared/linux_osl.c, include/linux_osl.h
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
 * $Id: osl_linux.c 758520 2018-04-19 06:01:39Z $
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include "osl_linux.h"

static void
igs_osl_timer(ulong data)
{
	igs_osl_timer_t *t;

	t = (igs_osl_timer_t *)data;

	ASSERT(t->set);

	if (t->periodic) {
#if defined(BCMJTAG) || defined(BCMSLTGT)
		t->timer.expires = jiffies + t->ms*HZ/1000*htclkratio;
#else
		t->timer.expires = jiffies + t->ms*HZ/1000;
#endif /* defined(BCMJTAG) || defined(BCMSLTGT) */
		add_timer(&t->timer);
		t->set = TRUE;
		t->fn(t->arg);
	} else {
		t->set = FALSE;
		t->fn(t->arg);
#ifdef BCMDBG
		if (t->name) {
			MFREE(NULL, t->name, strlen(t->name) + 1);
		}
#endif // endif
		MFREE(NULL, t, sizeof(igs_osl_timer_t));
	}

	return;
}

igs_osl_timer_t *
igs_osl_timer_init(const char *name, void (*fn)(void *arg), void *arg)
{
	igs_osl_timer_t *t;

	if ((t = MALLOC(NULL, sizeof(igs_osl_timer_t))) == NULL) {
		printk(KERN_ERR "igs_osl_timer_init: out of memory, malloced %zu bytes\n",
		       sizeof(igs_osl_timer_t));
		return (NULL);
	}

	bzero(t, sizeof(igs_osl_timer_t));

	t->fn = fn;
	t->arg = arg;
	t->timer.data = (ulong)t;
	t->timer.function = igs_osl_timer;
#ifdef BCMDBG
	if ((t->name = MALLOC(NULL, strlen(name) + 1)) != NULL) {
		strcpy(t->name, name);
	}
#endif // endif

	init_timer(&t->timer);

	return (t);
}

void
igs_osl_timer_add(igs_osl_timer_t *t, uint32 ms, bool periodic)
{
	ASSERT(!t->set);

	t->set = TRUE;
	t->ms = ms;
	t->periodic = periodic;
#if defined(BCMJTAG) || defined(BCMSLTGT)
	t->timer.expires = jiffies + ms*HZ/1000*htclkratio;
#else
	t->timer.expires = jiffies + ms*HZ/1000;
#endif /* defined(BCMJTAG) || defined(BCMSLTGT) */

	add_timer(&t->timer);

	return;
}

void
igs_osl_timer_update(igs_osl_timer_t *t, uint32 ms, bool periodic)
{
	ASSERT(t->set);

	t->ms = ms;
	t->periodic = periodic;
	t->set = TRUE;
#if defined(BCMJTAG) || defined(BCMSLTGT)
	t->timer.expires = jiffies + ms*HZ/1000*htclkratio;
#else
	t->timer.expires = jiffies + ms*HZ/1000;
#endif /* defined(BCMJTAG) || defined(BCMSLTGT) */

	mod_timer(&t->timer, t->timer.expires);

	return;
}

/*
 * Return TRUE if timer successfully deleted, FALSE if still pending
 */
bool
igs_osl_timer_del(igs_osl_timer_t *t)
{
	if (t->set) {
		t->set = FALSE;
		if (!del_timer(&t->timer)) {
			printk(KERN_INFO "igs_osl_timer_del: Failed to delete timer\n");
			return (FALSE);
		}
#ifdef BCMDBG
		if (t->name) {
			MFREE(NULL, t->name, strlen(t->name) + 1);
		}
#endif // endif
		MFREE(NULL, t, sizeof(igs_osl_timer_t));
	}

	return (TRUE);
}
