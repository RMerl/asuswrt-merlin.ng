/*
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
 *
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id: osl_linux.h 679290 2017-01-13 07:39:40Z $
 */

#ifndef _OSL_LINUX_H_
#define _OSL_LINUX_H_

#include <linux/types.h>
#include <linux/timer.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>

struct osl_lock
{
	spinlock_t slock;       /* Spin lock */
	uint8      name[16];    /* Name of the lock */
};

typedef struct osl_lock *osl_lock_t;

static inline osl_lock_t
OSL_LOCK_CREATE(uint8 *name)
{
	osl_lock_t lock;

	lock = MALLOC(NULL, sizeof(struct osl_lock));

	if (lock == NULL)
	{
		printf("Memory alloc for lock object failed\n");
		return (NULL);
	}

	strncpy(lock->name, name, sizeof(lock->name)-1);
	lock->name[ sizeof(lock->name)-1 ] = '\0';
	spin_lock_init(&lock->slock);

	return (lock);
}

static inline void
OSL_LOCK_DESTROY(osl_lock_t lock)
{
	MFREE(NULL, lock, sizeof(struct osl_lock));
	return;
}

#define OSL_LOCK(lock)          spin_lock_bh(&((lock)->slock))
#define OSL_UNLOCK(lock)        spin_unlock_bh(&((lock)->slock))

#define DEV_IFNAME(dev)         (((struct net_device *)dev)->name)

typedef struct igs_osl_timer {
	struct timer_list timer;
	void   (*fn)(void *);
	void   *arg;
	uint   ms;
	bool   periodic;
	bool   set;
#ifdef BCMDBG
	char    *name;          /* Desription of the timer */
#endif // endif
} igs_osl_timer_t;

extern igs_osl_timer_t *igs_osl_timer_init(const char *name, void (*fn)(void *arg), void *arg);
extern void igs_osl_timer_add(igs_osl_timer_t *t, uint32 ms, bool periodic);
extern void igs_osl_timer_update(igs_osl_timer_t *t, uint32 ms, bool periodic);
extern bool igs_osl_timer_del(igs_osl_timer_t *t);
extern osl_lock_t OSL_LOCK_CREATE(uint8 *name);
extern void OSL_LOCK_DESTROY(osl_lock_t lock);
#endif /* _OSL_LINUX_H_ */
