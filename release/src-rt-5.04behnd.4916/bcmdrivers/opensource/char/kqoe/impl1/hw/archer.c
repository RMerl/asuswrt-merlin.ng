/*
 * <:copyright-BRCM:2024:DUAL/GPL:standard
 *
 *    Copyright (c) 2024 Broadcom
 *    All Rights Reserved
 *
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 *
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 *
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
 *
 * :>
 */
#include <linux/err.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/types.h>
#include <bcm_archer_dpi.h>
#include <linux/kqoe.h>
#include "../kqoe_local.h"

#define DEFAULT_MBS	2000

struct __aq {
	u64	bw_min;
	u64	bw_max;
};

struct __acfg {
	u64		bw_max;
	struct __aq	ds[ARCHER_DPI_DS_QUEUE_MAX];
};

static struct __acfg acfg = {};

/* ----- functions ----- */
static u64 aq_bw_to_bw(u64 val)
{
	return val * 1000;
}

static u64 bw_to_aq_bw(u64 val)
{
	u64 tmp = val;

	do_div(tmp, 1000);
	if (val)
		tmp = max_t(u64, tmp, ARCHER_DPI_BW_MIN_LIMIT);

	return tmp;
}

static struct __aq *kq2archer_queue_get(struct kq_queue *q, u8 dir)
{
	if (dir != KQ_DIR_DS)
		return ERR_PTR(-EINVAL);
	if (q->id >= ARRAY_SIZE(acfg.ds))
		return ERR_PTR(-ENOENT);
	return &acfg.ds[q->id];
}

int kq_hw_queue_fill(struct kq_queue *q, u8 dir)
{
	struct __aq *aq;

	memset(&q->bw, 0, sizeof(q->bw));

	aq = kq2archer_queue_get(q, dir);
	if (IS_ERR(aq))
		return PTR_ERR(aq);
	if (aq) {
		q->bw.min = aq_bw_to_bw(aq->bw_min);
		q->bw.max = aq_bw_to_bw(aq->bw_max);
	}

	return 0;
}

int kq_hw_queue_set(struct kq_queue *q, u8 dir)
{
	struct __aq *aq;

	aq = kq2archer_queue_get(q, dir);
	if (IS_ERR(aq))
		return PTR_ERR(aq);
	if (!aq)
		return -EPERM;

	aq->bw_min = bw_to_aq_bw(q->bw.min);
	aq->bw_max = bw_to_aq_bw(q->bw.max);

	archer_dpi_ds_queue_config(q->id, aq->bw_min, DEFAULT_MBS, aq->bw_max,
				   DEFAULT_MBS);
	return 0;
}

int kq_hw_config_fill(struct kq_cfg_dir *c, u8 dir)
{
	memset(&c->bw, 0, sizeof(c->bw));

	if (dir == KQ_DIR_DS) {
		c->bw.min = 0;
		c->bw.max = aq_bw_to_bw(acfg.bw_max);
	}
	return 0;
}

int kq_hw_config_set(struct kq_cfg_dir *c, u8 dir)
{
	if (dir != KQ_DIR_DS)
		return -EPERM;

	acfg.bw_max = bw_to_aq_bw(c->bw.max);

	archer_dpi_ds_port_config(acfg.bw_max, DEFAULT_MBS);

	return 0;
}

int __init kq_hw_init(void)
{
	kqcfg.ds.num_queues	= ARCHER_DPI_DS_QUEUE_MAX;
	kqcfg.ds.bypass		= ARCHER_DPI_BYPASS_QUEUE;
	kqcfg.us.num_queues	= ARCHER_DPI_US_QUEUE_MAX;

	if (IS_ENABLED(CONFIG_BCM_CROSSBOW))
		kqcfg.us.bypass = ARCHER_DPI_BYPASS_QUEUE;
	else
		kqcfg.us.bypass = EG_PRIO_NORMAL;

	archer_dpi_mode_set(ARCHER_DPI_MODE_DISABLE);
	archer_dpi_mode_set(ARCHER_DPI_MODE_ENABLE);
	return 0;
}

void __exit kq_hw_exit(void)
{
	archer_dpi_mode_set(ARCHER_DPI_MODE_DISABLE);
}
