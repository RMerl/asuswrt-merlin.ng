/*
 * Expose some of the kernel scheduler routines
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
 * $Id: dhd_linux_sched.c 834170 2023-12-13 07:05:58Z $
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <typedefs.h>
#include <linuxver.h>

int setScheduler(struct task_struct *p, int policy, struct sched_param *param)
{
	int rc = 0;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
	switch (policy) {
	case SCHED_FIFO:
		sched_set_fifo(p);
		break;
	case SCHED_NORMAL:
		sched_set_normal(p, 0);
		break;
	default:
		rc = -EINVAL;
		break;
	}
#else
	rc = sched_setscheduler(p, policy, param);
#endif /* LinuxVer */
	return rc;
}

int get_scheduler_policy(struct task_struct *p)
{
	return p->policy;
}
