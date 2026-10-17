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
#include <linux/types.h>
#include <linux/kqoe.h>
#include <rdpa_types.h>
#include <rdpa_api.h>
#include "../kqoe_local.h"

struct kq_runner_queue_cfg {
	bdmf_object_handle	handle;
	u32			status;
};

struct kq_runner_cfg {
	rdpa_traffic_dir		dir;
	rdpa_tm_level_type		level;
	bdmf_object_handle		sched;
	bdmf_object_handle		rl;
	int				num_queues;
	struct kq_runner_queue_cfg	*queues;
};

struct __rq {
	bdmf_object_handle	queue;
	bdmf_object_handle	rl;
	int			queue_index;
	rdpa_tm_queue_cfg_t	queue_cfg;
};

/* --- variables --- */
static int drop_threshold;
static struct kq_runner_cfg ds = {
	.dir	= rdpa_dir_ds,
	.level	= rdpa_tm_level_queue,
};

/* --- functions --- */
static inline struct kq_runner_cfg *get_cfg_from_dir(u32 dir)
{
	struct kq_runner_cfg *cfg = NULL;
	int ret;

	if (dir == KQ_DIR_DS)
		cfg = &ds;
	if (!cfg)
		return NULL;

	ret = rdpa_egress_tm_rate_limit_get(cfg->sched, &cfg->rl);
	if (ret)
		cfg->rl = NULL;

	return cfg;
}

static int queue_create(struct kq_runner_cfg *cfg, int id)
{
	struct kq_runner_queue_cfg *qcfg;
	rdpa_tm_queue_cfg_t queue_cfg = {};

	if (!cfg->sched)
		goto err;
	qcfg = &cfg->queues[id];

	qcfg->handle			= cfg->sched;
	queue_cfg.weight		= 1;
	queue_cfg.queue_id		= id;
	queue_cfg.drop_threshold	= drop_threshold;
	queue_cfg.drop_alg		= rdpa_tm_drop_alg_dt;

	if (rdpa_egress_tm_queue_cfg_set(qcfg->handle, id, &queue_cfg))
		goto err;

	return 0;

err:
	return -EINVAL;
}

static int __init runner_cfg_init(struct kq_runner_cfg *cfg)
{
	BDMF_MATTR_ALLOC(attrs, rdpa_egress_tm_drv());
	rdpa_tm_service_queue_t service_queue = { .enable = 1 };
	int i, ret = -EINVAL;

	/* create scheduler */
	rdpa_egress_tm_level_set(attrs, cfg->level);
	rdpa_egress_tm_num_queues_set(attrs, cfg->num_queues);
	rdpa_egress_tm_mode_set(attrs, rdpa_tm_sched_sp_wrr);
	rdpa_egress_tm_num_sp_elements_set(attrs, 0);
	rdpa_egress_tm_rl_rate_mode_set(attrs, rdpa_tm_rl_dual_rate);
	rdpa_egress_tm_service_queue_set(attrs, &service_queue);

	if (bdmf_new_and_set(rdpa_egress_tm_drv(), NULL, attrs, &cfg->sched))
		goto err;

	/* create queues */
	for (i = 0; i < cfg->num_queues; i++) {
		ret = queue_create(cfg, i);
		if (ret)
			goto err_cleanup_queues;
	}
	BDMF_MATTR_FREE(attrs);
	return 0;

err_cleanup_queues:
	bdmf_destroy(cfg->sched);
err:
	BDMF_MATTR_FREE(attrs);
	return ret;
}

static void __exit runner_cfg_deinit(struct kq_runner_cfg *cfg)
{
	bdmf_destroy(cfg->sched);
}

static int kq2runner_queue_get(struct kq_queue *q, u8 dir, struct __rq *rq)
{
	struct kq_runner_cfg *cfg;

	cfg = get_cfg_from_dir(dir);
	if (!cfg || q->id >= cfg->num_queues)
		return -ENOENT;
	if (!cfg->queues[q->id].handle)
		return -EINVAL;

	rq->queue	= cfg->queues[q->id].handle;
	rq->queue_index	= q->id;

	if (rdpa_egress_tm_queue_cfg_get(rq->queue, rq->queue_index,
					 &rq->queue_cfg))
		return -EINVAL;

	rq->rl = rq->queue_cfg.rl_cfg;

	return 0;
}

int kq_hw_queue_fill(struct kq_queue *q, u8 dir)
{
	rdpa_rl_cfg_t rl_cfg = {0};
	struct __rq rq = {};
	int ret;

	memset(&q->bw, 0, sizeof(q->bw));

	ret = kq2runner_queue_get(q, dir, &rq);
	if (ret)
		return ret;

	if (!rq.queue_cfg.rl_cfg)
		goto out;

	ret = rdpa_rate_limit_cfg_get(rq.rl, &rl_cfg);
	if (ret) {
		ret = 0;
		goto out;
	}

	q->bw.max = rl_cfg.af_rate;
	q->bw.min = rl_cfg.be_rate;

out:
	return ret;
}

int kq_hw_queue_set(struct kq_queue *q, u8 dir)
{
	struct kq_runner_cfg *cfg = get_cfg_from_dir(dir);
	rdpa_rl_cfg_t rl_cfg = {0};
	struct __rq rq = {};
	int err;

	err = kq2runner_queue_get(q, dir, &rq);
	if (err)
		goto out;

	cfg->queues[q->id].status = q->status;
	if (!rq.rl) {
		/* need to create new rate limit object */
		bdmf_mattr_handle mattr;

		mattr = bdmf_mattr_alloc(rdpa_rate_limit_drv());
		if (!mattr) {
			err = -ENOMEM;
			goto out;
		}

		err = bdmf_new_and_set(rdpa_rate_limit_drv(), NULL, mattr,
				       &rq.rl);
		bdmf_mattr_free(mattr);

		if (err)
			goto out;

		rq.queue_cfg.rl_cfg = rq.rl;
	}

	rl_cfg.af_rate = q->bw.min;
	rl_cfg.be_rate = q->bw.max - q->bw.min;
	err = rdpa_rate_limit_cfg_set(rq.rl, &rl_cfg);

	rq.queue_cfg.drop_threshold = drop_threshold;
	err = rdpa_egress_tm_queue_cfg_set(rq.queue, rq.queue_index,
					   &rq.queue_cfg);

out:
	return err;
}

int kq_hw_config_fill(struct kq_cfg_dir *c, u8 dir)
{
	rdpa_rl_cfg_t rl_cfg = {0};
	struct kq_runner_cfg *rcfg;
	int ret = 0;

	memset(&c->bw, 0, sizeof(c->bw));
	rcfg = get_cfg_from_dir(dir);
	if (!rcfg)
		return 0;

	c->bw.max = 0;

	if (!rcfg->rl)
		goto out;
	ret = rdpa_rate_limit_cfg_get(rcfg->rl, &rl_cfg);
	if (ret)
		goto out;

	c->bw.max = rl_cfg.af_rate;

out:
	return ret;
}

int kq_hw_config_set(struct kq_cfg_dir *c, u8 dir)
{
	struct kq_runner_cfg *rcfg;
	rdpa_rl_cfg_t rl_cfg = {};
	int ret;

	rcfg = get_cfg_from_dir(dir);
	if (!rcfg)
		return -EINVAL;

	rl_cfg.af_rate = c->bw.max;
	rl_cfg.be_rate = 0;

	if (!rcfg->rl) {
		/* need to create new rate limit object */
		bdmf_mattr_handle mattr;

		mattr = bdmf_mattr_alloc(rdpa_rate_limit_drv());
		if (!mattr) {
			ret = -ENOMEM;
			goto out;
		}

		ret = bdmf_new_and_set(rdpa_rate_limit_drv(), NULL, mattr,
				       &rcfg->rl);
		bdmf_mattr_free(mattr);

		if (ret)
			goto out;
	}

	rdpa_rate_limit_cfg_set(rcfg->rl, &rl_cfg);
	ret = rdpa_egress_tm_rate_limit_set(rcfg->sched, rcfg->rl);

out:
	return ret;
}

int __init kq_hw_init(void)
{
	bdmf_object_handle system_obj = NULL;
	rdpa_qm_cfg_t qm_cfg;
	int ret;

	if (rdpa_system_get(&system_obj))
		return -EINVAL;
	rdpa_system_qm_cfg_get(system_obj, &qm_cfg);
	bdmf_put(system_obj);

	drop_threshold		= 339360;
	kqcfg.ds.num_queues	= qm_cfg.number_of_service_queues;
	kqcfg.ds.bypass		= -1;
	kqcfg.us.num_queues	= EG_PRIO_MAX + 1;
	kqcfg.us.bypass		= -1;

	ds.num_queues = kqcfg.ds.num_queues;
	if (!ds.num_queues)
		return 0;
	ds.queues = kcalloc(ds.num_queues, sizeof(*ds.queues), GFP_KERNEL);
	if (!ds.queues)
		return -ENOMEM;

	ret = runner_cfg_init(&ds);
	if (ret)
		goto err_free_ds_queues;

	return 0;

err_free_ds_queues:
	kfree(ds.queues);
	return ret;
}

void kq_hw_exit(void)
{
	runner_cfg_deinit(&ds);

	kfree(ds.queues);
	ds.queues = NULL;
}
