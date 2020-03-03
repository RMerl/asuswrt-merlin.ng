/*
 * Common Block IO controller cgroup interface
 *
 * Based on ideas and code from CFQ, CFS and BFQ:
 * Copyright (C) 2003 Jens Axboe <axboe@kernel.dk>
 *
 * Copyright (C) 2008 Fabio Checconi <fabio@gandalf.sssup.it>
 *		      Paolo Valente <paolo.valente@unimore.it>
 *
 * Copyright (C) 2009 Vivek Goyal <vgoyal@redhat.com>
 * 	              Nauman Rafique <nauman@google.com>
 */
#include <linux/ioprio.h>
#include <linux/kdev_t.h>
#include <linux/module.h>
#include <linux/err.h>
#include <linux/blkdev.h>
#include <linux/slab.h>
#include <linux/genhd.h>
#include <linux/delay.h>
#include <linux/atomic.h>
#include "blk-cgroup.h"
#include "blk.h"

#define MAX_KEY_LEN 100

static DEFINE_MUTEX(blkcg_pol_mutex);

struct blkcg blkcg_root = { .cfq_weight = 2 * CFQ_WEIGHT_DEFAULT,
			    .cfq_leaf_weight = 2 * CFQ_WEIGHT_DEFAULT, };
EXPORT_SYMBOL_GPL(blkcg_root);

static struct blkcg_policy *blkcg_policy[BLKCG_MAX_POLS];

static bool blkcg_policy_enabled(struct request_queue *q,
				 const struct blkcg_policy *pol)
{
	return pol && test_bit(pol->plid, q->blkcg_pols);
}

/**
 * blkg_free - free a blkg
 * @blkg: blkg to free
 *
 * Free @blkg which may be partially allocated.
 */
static void blkg_free(struct blkcg_gq *blkg)
{
	int i;

	if (!blkg)
		return;

	for (i = 0; i < BLKCG_MAX_POLS; i++)
		kfree(blkg->pd[i]);

	if (blkg->blkcg != &blkcg_root)
		blk_exit_rl(&blkg->rl);
	kfree(blkg);
}

/**
 * blkg_alloc - allocate a blkg
 * @blkcg: block cgroup the new blkg is associated with
 * @q: request_queue the new blkg is associated with
 * @gfp_mask: allocation mask to use
 *
 * Allocate a new blkg assocating @blkcg and @q.
 */
static struct blkcg_gq *blkg_alloc(struct blkcg *blkcg, struct request_queue *q,
				   gfp_t gfp_mask)
{
	struct blkcg_gq *blkg;
	int i;

	/* alloc and init base part */
	blkg = kzalloc_node(sizeof(*blkg), gfp_mask, q->node);
	if (!blkg)
		return NULL;

	blkg->q = q;
	INIT_LIST_HEAD(&blkg->q_node);
	blkg->blkcg = blkcg;
	atomic_set(&blkg->refcnt, 1);

	/* root blkg uses @q->root_rl, init rl only for !root blkgs */
	if (blkcg != &blkcg_root) {
		if (blk_init_rl(&blkg->rl, q, gfp_mask))
			goto err_free;
		blkg->rl.blkg = blkg;
	}

	for (i = 0; i < BLKCG_MAX_POLS; i++) {
		struct blkcg_policy *pol = blkcg_policy[i];
		struct blkg_policy_data *pd;

		if (!blkcg_policy_enabled(q, pol))
			continue;

		/* alloc per-policy data and attach it to blkg */
		pd = kzalloc_node(pol->pd_size, gfp_mask, q->node);
		if (!pd)
			goto err_free;

		blkg->pd[i] = pd;
		pd->blkg = blkg;
		pd->plid = i;
	}

	return blkg;

err_free:
	blkg_free(blkg);
	return NULL;
}

/**
 * __blkg_lookup - internal version of blkg_lookup()
 * @blkcg: blkcg of interest
 * @q: request_queue of interest
 * @update_hint: whether to update lookup hint with the result or not
 *
 * This is internal version and shouldn't be used by policy
 * implementations.  Looks up blkgs for the @blkcg - @q pair regardless of
 * @q's bypass state.  If @update_hint is %true, the caller should be
 * holding @q->queue_lock and lookup hint is updated on success.
 */
struct blkcg_gq *__blkg_lookup(struct blkcg *blkcg, struct request_queue *q,
			       bool update_hint)
{
	struct blkcg_gq *blkg;

	blkg = rcu_dereference(blkcg->blkg_hint);
	if (blkg && blkg->q == q)
		return blkg;

	/*
	 * Hint didn't match.  Look up from the radix tree.  Note that the
	 * hint can only be updated under queue_lock as otherwise @blkg
	 * could have already been removed from blkg_tree.  The caller is
	 * responsible for grabbing queue_lock if @update_hint.
	 */
	blkg = radix_tree_lookup(&blkcg->blkg_tree, q->id);
	if (blkg && blkg->q == q) {
		if (update_hint) {
			lockdep_assert_held(q->queue_lock);
			rcu_assign_pointer(blkcg->blkg_hint, blkg);
		}
		return blkg;
	}

	return NULL;
}

/**
 * blkg_lookup - lookup blkg for the specified blkcg - q pair
 * @blkcg: blkcg of interest
 * @q: request_queue of interest
 *
 * Lookup blkg for the @blkcg - @q pair.  This function should be called
 * under RCU read lock and is guaranteed to return %NULL if @q is bypassing
 * - see blk_queue_bypass_start() for details.
 */
struct blkcg_gq *blkg_lookup(struct blkcg *blkcg, struct request_queue *q)
{
	WARN_ON_ONCE(!rcu_read_lock_held());

	if (unlikely(blk_queue_bypass(q)))
		return NULL;
	return __blkg_lookup(blkcg, q, false);
}
EXPORT_SYMBOL_GPL(blkg_lookup);

/*
 * If @new_blkg is %NULL, this function tries to allocate a new one as
 * necessary using %GFP_ATOMIC.  @new_blkg is always consumed on return.
 */
static struct blkcg_gq *blkg_create(struct blkcg *blkcg,
				    struct request_queue *q,
				    struct blkcg_gq *new_blkg)
{
	struct blkcg_gq *blkg;
	int i, ret;

	WARN_ON_ONCE(!rcu_read_lock_held());
	lockdep_assert_held(q->queue_lock);

	/* blkg holds a reference to blkcg */
	if (!css_tryget_online(&blkcg->css)) {
		ret = -EINVAL;
		goto err_free_blkg;
	}

	/* allocate */
	if (!new_blkg) {
		new_blkg = blkg_alloc(blkcg, q, GFP_ATOMIC);
		if (unlikely(!new_blkg)) {
			ret = -ENOMEM;
			goto err_put_css;
		}
	}
	blkg = new_blkg;

	/* link parent */
	if (blkcg_parent(blkcg)) {
		blkg->parent = __blkg_lookup(blkcg_parent(blkcg), q, false);
		if (WARN_ON_ONCE(!blkg->parent)) {
			ret = -EINVAL;
			goto err_put_css;
		}
		blkg_get(blkg->parent);
	}

	/* invoke per-policy init */
	for (i = 0; i < BLKCG_MAX_POLS; i++) {
		struct blkcg_policy *pol = blkcg_policy[i];

		if (blkg->pd[i] && pol->pd_init_fn)
			pol->pd_init_fn(blkg);
	}

	/* insert */
	spin_lock(&blkcg->lock);
	ret = radix_tree_insert(&blkcg->blkg_tree, q->id, blkg);
	if (likely(!ret)) {
		hlist_add_head_rcu(&blkg->blkcg_node, &blkcg->blkg_list);
		list_add(&blkg->q_node, &q->blkg_list);

		for (i = 0; i < BLKCG_MAX_POLS; i++) {
			struct blkcg_policy *pol = blkcg_policy[i];

			if (blkg->pd[i] && pol->pd_online_fn)
				pol->pd_online_fn(blkg);
		}
	}
	blkg->online = true;
	spin_unlock(&blkcg->lock);

	if (!ret)
		return blkg;

	/* @blkg failed fully initialized, use the usual release path */
	blkg_put(blkg);
	return ERR_PTR(ret);

err_put_css:
	css_put(&blkcg->css);
err_free_blkg:
	blkg_free(new_blkg);
	return ERR_PTR(ret);
}

/**
 * blkg_lookup_create - lookup blkg, try to create one if not there
 * @blkcg: blkcg of interest
 * @q: request_queue of interest
 *
 * Lookup blkg for the @blkcg - @q pair.  If it doesn't exist, try to
 * create one.  blkg creation is performed recursively from blkcg_root such
 * that all non-root blkg's have access to the parent blkg.  This function
 * should be called under RCU read lock and @q->queue_lock.
 *
 * Returns pointer to the looked up or created blkg on success, ERR_PTR()
 * value on error.  If @q is dead, returns ERR_PTR(-EINVAL).  If @q is not
 * dead and bypassing, returns ERR_PTR(-EBUSY).
 */
struct blkcg_gq *blkg_lookup_create(struct blkcg *blkcg,
				    struct request_queue *q)
{
	struct blkcg_gq *blkg;

	WARN_ON_ONCE(!rcu_read_lock_held());
	lockdep_assert_held(q->queue_lock);

	/*
	 * This could be the first entry point of blkcg implementation and
	 * we shouldn't allow anything to go through for a bypassing queue.
	 */
	if (unlikely(blk_queue_bypass(q)))
		return ERR_PTR(blk_queue_dying(q) ? -EINVAL : -EBUSY);

	blkg = __blkg_lookup(blkcg, q, true);
	if (blkg)
		return blkg;

	/*
	 * Create blkgs walking down from blkcg_root to @blkcg, so that all
	 * non-root blkgs have access to their parents.
	 */
	while (true) {
		struct blkcg *pos = blkcg;
		struct blkcg *parent = blkcg_parent(blkcg);

		while (parent && !__blkg_lookup(parent, q, false)) {
			pos = parent;
			parent = blkcg_parent(parent);
		}

		blkg = blkg_create(pos, q, NULL);
		if (pos == blkcg || IS_ERR(blkg))
			return blkg;
	}
}
EXPORT_SYMBOL_GPL(blkg_lookup_create);

static void blkg_destroy(struct blkcg_gq *blkg)
{
	struct blkcg *blkcg = blkg->blkcg;
	int i;

	lockdep_assert_held(blkg->q->queue_lock);
	lockdep_assert_held(&blkcg->lock);

	/* Something wrong if we are trying to remove same group twice */
	WARN_ON_ONCE(list_empty(&blkg->q_node));
	WARN_ON_ONCE(hlist_unhashed(&blkg->blkcg_node));

	for (i = 0; i < BLKCG_MAX_POLS; i++) {
		struct blkcg_policy *pol = blkcg_policy[i];

		if (blkg->pd[i] && pol->pd_offline_fn)
			pol->pd_offline_fn(blkg);
	}
	blkg->online = false;

	radix_tree_delete(&blkcg->blkg_tree, blkg->q->id);
	list_del_init(&blkg->q_node);
	hlist_del_init_rcu(&blkg->blkcg_node);

	/*
	 * Both setting lookup hint to and clearing it from @blkg are done
	 * under queue_lock.  If it's not pointing to @blkg now, it never
	 * will.  Hint assignment itself can race safely.
	 */
	if (rcu_access_pointer(blkcg->blkg_hint) == blkg)
		rcu_assign_pointer(blkcg->blkg_hint, NULL);

	/*
	 * Put the reference taken at the time of creation so that when all
	 * queues are gone, group can be destroyed.
	 */
	blkg_put(blkg);
}

/**
 * blkg_destroy_all - destroy all blkgs associated with a request_queue
 * @q: request_queue of interest
 *
 * Destroy all blkgs associated with @q.
 */
static void blkg_destroy_all(struct request_queue *q)
{
	struct blkcg_gq *blkg, *n;

	lockdep_assert_held(q->queue_lock);

	list_for_each_entry_safe(blkg, n, &q->blkg_list, q_node) {
		struct blkcg *blkcg = blkg->blkcg;

		spin_lock(&blkcg->lock);
		blkg_destroy(blkg);
		spin_unlock(&blkcg->lock);
	}
}

/*
 * A group is RCU protected, but having an rcu lock does not mean that one
 * can access all the fields of blkg and assume these are valid.  For
 * example, don't try to follow throtl_data and request queue links.
 *
 * Having a reference to blkg under an rcu allows accesses to only values
 * local to groups like group stats and group rate limits.
 */
void __blkg_release_rcu(struct rcu_head *rcu_head)
{
	struct blkcg_gq *blkg = container_of(rcu_head, struct blkcg_gq, rcu_head);
	int i;

	/* tell policies that this one is being freed */
	for (i = 0; i < BLKCG_MAX_POLS; i++) {
		struct blkcg_policy *pol = blkcg_policy[i];

		if (blkg->pd[i] && pol->pd_exit_fn)
			pol->pd_exit_fn(blkg);
	}

	/* release the blkcg and parent blkg refs this blkg has been holding */
	css_put(&blkg->blkcg->css);
	if (blkg->parent)
		blkg_put(blkg->parent);

	blkg_free(blkg);
}
EXPORT_SYMBOL_GPL(__blkg_release_rcu);

/*
 * The next function used by blk_queue_for_each_rl().  It's a bit tricky
 * because the root blkg uses @q->root_rl instead of its own rl.
 */
struct request_list *__blk_queue_next_rl(struct request_list *rl,
					 struct request_queue *q)
{
	struct list_head *ent;
	struct blkcg_gq *blkg;

	/*
	 * Determine the current blkg list_head.  The first entry is
	 * root_rl which is off @q->blkg_list and mapped to the head.
	 */
	if (rl == &q->root_rl) {
		ent = &q->blkg_list;
		/* There are no more block groups, hence no request lists */
		if (list_empty(ent))
			return NULL;
	} else {
		blkg = container_of(rl, struct blkcg_gq, rl);
		ent = &blkg->q_node;
	}

	/* walk to the next list_head, skip root blkcg */
	ent = ent->next;
	if (ent == &q->root_blkg->q_node)
		ent = ent->next;
	if (ent == &q->blkg_list)
		return NULL;

	blkg = container_of(ent, struct blkcg_gq, q_node);
	return &blkg->rl;
}

static int blkcg_reset_stats(struct cgroup_subsys_state *css,
			     struct cftype *cftype, u64 val)
{
	struct blkcg *blkcg = css_to_blkcg(css);
	struct blkcg_gq *blkg;
	int i;

	/*
	 * XXX: We invoke cgroup_add/rm_cftypes() under blkcg_pol_mutex
	 * which ends up putting cgroup's internal cgroup_tree_mutex under
	 * it; however, cgroup_tree_mutex is nested above cgroup file
	 * active protection and grabbing blkcg_pol_mutex from a cgroup
	 * file operation creates a possible circular dependency.  cgroup
	 * internal locking is planned to go through further simplification
	 * and this issue should go away soon.  For now, let's trylock
	 * blkcg_pol_mutex and restart the write on failure.
	 *
	 * http://lkml.kernel.org/g/5363C04B.4010400@oracle.com
	 */
	if (!mutex_trylock(&blkcg_pol_mutex))
		return restart_syscall();
	spin_lock_irq(&blkcg->lock);

	/*
	 * Note that stat reset is racy - it doesn't synchronize against
	 * stat updates.  This is a debug feature which shouldn't exist
	 * anyway.  If you get hit by a race, retry.
	 */
	hlist_for_each_entry(blkg, &blkcg->blkg_list, blkcg_node) {
		for (i = 0; i < BLKCG_MAX_POLS; i++) {
			struct blkcg_policy *pol = blkcg_policy[i];

			if (blkcg_policy_enabled(blkg->q, pol) &&
			    pol->pd_reset_stats_fn)
				pol->pd_reset_stats_fn(blkg);
		}
	}

	spin_unlock_irq(&blkcg->lock);
	mutex_unlock(&blkcg_pol_mutex);
	return 0;
}

static const char *blkg_dev_name(struct blkcg_gq *blkg)
{
	/* some drivers (floppy) instantiate a queue w/o disk registered */
	if (blkg->q->backing_dev_info.dev)
		return dev_name(blkg->q->backing_dev_info.dev);
	return NULL;
}

/**
 * blkcg_print_blkgs - helper for printing per-blkg data
 * @sf: seq_file to print to
 * @blkcg: blkcg of interest
 * @prfill: fill function to print out a blkg
 * @pol: policy in question
 * @data: data to be passed to @prfill
 * @show_total: to print out sum of prfill return values or not
 *
 * This function invokes @prfill on each blkg of @blkcg if pd for the
 * policy specified by @pol exists.  @prfill is invoked with @sf, the
 * policy data and @data and the matching queue lock held.  If @show_total
 * is %true, the sum of the return values from @prfill is printed with
 * "Total" label at the end.
 *
 * This is to be used to construct print functions for
 * cftype->read_seq_string method.
 */
void blkcg_print_blkgs(struct seq_file *sf, struct blkcg *blkcg,
		       u64 (*prfill)(struct seq_file *,
				     struct blkg_policy_data *, int),
		       const struct blkcg_policy *pol, int data,
		       bool show_total)
{
	struct blkcg_gq *blkg;
	u64 total = 0;

	rcu_read_lock();
	hlist_for_each_entry_rcu(blkg, &blkcg->blkg_list, blkcg_node) {
		spin_lock_irq(blkg->q->queue_lock);
		if (blkcg_policy_enabled(blkg->q, pol))
			total += prfill(sf, blkg->pd[pol->plid], data);
		spin_unlock_irq(blkg->q->queue_lock);
	}
	rcu_read_unlock();

	if (show_total)
		seq_printf(sf, "Total %llu\n", (unsigned long long)total);
}
EXPORT_SYMBOL_GPL(blkcg_print_blkgs);

/**
 * __blkg_prfill_u64 - prfill helper for a single u64 value
 * @sf: seq_file to print to
 * @pd: policy private data of interest
 * @v: value to print
 *
 * Print @v to @sf for the device assocaited with @pd.
 */
u64 __blkg_prfill_u64(struct seq_file *sf, struct blkg_policy_data *pd, u64 v)
{
	const char *dname = blkg_dev_name(pd->blkg);

	if (!dname)
		return 0;

	seq_printf(sf, "%s %llu\n", dname, (unsigned long long)v);
	return v;
}
EXPORT_SYMBOL_GPL(__blkg_prfill_u64);

/**
 * __blkg_prfill_rwstat - prfill helper for a blkg_rwstat
 * @sf: seq_file to print to
 * @pd: policy private data of interest
 * @rwstat: rwstat to print
 *
 * Print @rwstat to @sf for the device assocaited with @pd.
 */
u64 __blkg_prfill_rwstat(struct seq_file *sf, struct blkg_policy_data *pd,
			 const struct blkg_rwstat *rwstat)
{
	static const char *rwstr[] = {
		[BLKG_RWSTAT_READ]	= "Read",
		[BLKG_RWSTAT_WRITE]	= "Write",
		[BLKG_RWSTAT_SYNC]	= "Sync",
		[BLKG_RWSTAT_ASYNC]	= "Async",
	};
	const char *dname = blkg_dev_name(pd->blkg);
	u64 v;
	int i;

	if (!dname)
		return 0;

	for (i = 0; i < BLKG_RWSTAT_NR; i++)
		seq_printf(sf, "%s %s %llu\n", dname, rwstr[i],
			   (unsigned long long)rwstat->cnt[i]);

	v = rwstat->cnt[BLKG_RWSTAT_READ] + rwstat->cnt[BLKG_RWSTAT_WRITE];
	seq_printf(sf, "%s Total %llu\n", dname, (unsigned long long)v);
	return v;
}
EXPORT_SYMBOL_GPL(__blkg_prfill_rwstat);

/**
 * blkg_prfill_stat - prfill callback for blkg_stat
 * @sf: seq_file to print to
 * @pd: policy private data of interest
 * @off: offset to the blkg_stat in @pd
 *
 * prfill callback for printing a blkg_stat.
 */
u64 blkg_prfill_stat(struct seq_file *sf, struct blkg_policy_data *pd, int off)
{
	return __blkg_prfill_u64(sf, pd, blkg_stat_read((void *)pd + off));
}
EXPORT_SYMBOL_GPL(blkg_prfill_stat);

/**
 * blkg_prfill_rwstat - prfill callback for blkg_rwstat
 * @sf: seq_file to print to
 * @pd: policy private data of interest
 * @off: offset to the blkg_rwstat in @pd
 *
 * prfill callback for printing a blkg_rwstat.
 */
u64 blkg_prfill_rwstat(struct seq_file *sf, struct blkg_policy_data *pd,
		       int off)
{
	struct blkg_rwstat rwstat = blkg_rwstat_read((void *)pd + off);

	return __blkg_prfill_rwstat(sf, pd, &rwstat);
}
EXPORT_SYMBOL_GPL(blkg_prfill_rwstat);

/**
 * blkg_stat_recursive_sum - collect hierarchical blkg_stat
 * @pd: policy private data of interest
 * @off: offset to the blkg_stat in @pd
 *
 * Collect the blkg_stat specified by @off from @pd and all its online
 * descendants and return the sum.  The caller must be holding the queue
 * lock for online tests.
 */
u64 blkg_stat_recursive_sum(struct blkg_policy_data *pd, int off)
{
	struct blkcg_policy *pol = blkcg_policy[pd->plid];
	struct blkcg_gq *pos_blkg;
	struct cgroup_subsys_state *pos_css;
	u64 sum = 0;

	lockdep_assert_held(pd->blkg->q->queue_lock);

	rcu_read_lock();
	blkg_for_each_descendant_pre(pos_blkg, pos_css, pd_to_blkg(pd)) {
		struct blkg_policy_data *pos_pd = blkg_to_pd(pos_blkg, pol);
		struct blkg_stat *stat = (void *)pos_pd + off;

		if (pos_blkg->online)
			sum += blkg_stat_read(stat);
	}
	rcu_read_unlock();

	return sum;
}
EXPORT_SYMBOL_GPL(blkg_stat_recursive_sum);

/**
 * blkg_rwstat_recursive_sum - collect hierarchical blkg_rwstat
 * @pd: policy private data of interest
 * @off: offset to the blkg_stat in @pd
 *
 * Collect the blkg_rwstat specified by @off from @pd and all its online
 * descendants and return the sum.  The caller must be holding the queue
 * lock for online tests.
 */
struct blkg_rwstat blkg_rwstat_recursive_sum(struct blkg_policy_data *pd,
					     int off)
{
	struct blkcg_policy *pol = blkcg_policy[pd->plid];
	struct blkcg_gq *pos_blkg;
	struct cgroup_subsys_state *pos_css;
	struct blkg_rwstat sum = { };
	int i;

	lockdep_assert_held(pd->blkg->q->queue_lock);

	rcu_read_lock();
	blkg_for_each_descendant_pre(pos_blkg, pos_css, pd_to_blkg(pd)) {
		struct blkg_policy_data *pos_pd = blkg_to_pd(pos_blkg, pol);
		struct blkg_rwstat *rwstat = (void *)pos_pd + off;
		struct blkg_rwstat tmp;

		if (!pos_blkg->online)
			continue;

		tmp = blkg_rwstat_read(rwstat);

		for (i = 0; i < BLKG_RWSTAT_NR; i++)
			sum.cnt[i] += tmp.cnt[i];
	}
	rcu_read_unlock();

	return sum;
}
EXPORT_SYMBOL_GPL(blkg_rwstat_recursive_sum);

/**
 * blkg_conf_prep - parse and prepare for per-blkg config update
 * @blkcg: target block cgroup
 * @pol: target policy
 * @input: input string
 * @ctx: blkg_conf_ctx to be filled
 *
 * Parse per-blkg config update from @input and initialize @ctx with the
 * result.  @ctx->blkg points to the blkg to be updated and @ctx->v the new
 * value.  This function returns with RCU read lock and queue lock held and
 * must be paired with blkg_conf_finish().
 */
int blkg_conf_prep(struct blkcg *blkcg, const struct blkcg_policy *pol,
		   const char *input, struct blkg_conf_ctx *ctx)
	__acquires(rcu) __acquires(disk->queue->queue_lock)
{
	struct gendisk *disk;
	struct blkcg_gq *blkg;
	unsigned int major, minor;
	unsigned long long v;
	int part, ret;

	if (sscanf(input, "%u:%u %llu", &major, &minor, &v) != 3)
		return -EINVAL;

	disk = get_gendisk(MKDEV(major, minor), &part);
	if (!disk)
		return -EINVAL;
	if (part) {
		put_disk(disk);
		return -EINVAL;
	}

	rcu_read_lock();
	spin_lock_irq(disk->queue->queue_lock);

	if (blkcg_policy_enabled(disk->queue, pol))
		blkg = blkg_lookup_create(blkcg, disk->queue);
	else
		blkg = ERR_PTR(-EINVAL);

	if (IS_ERR(blkg)) {
		ret = PTR_ERR(blkg);
		rcu_read_unlock();
		spin_unlock_irq(disk->queue->queue_lock);
		put_disk(disk);
		/*
		 * If queue was bypassing, we should retry.  Do so after a
		 * short msleep().  It isn't strictly necessary but queue
		 * can be bypassing for some time and it's always nice to
		 * avoid busy looping.
		 */
		if (ret == -EBUSY) {
			msleep(10);
			ret = restart_syscall();
		}
		return ret;
	}

	ctx->disk = disk;
	ctx->blkg = blkg;
	ctx->v = v;
	return 0;
}
EXPORT_SYMBOL_GPL(blkg_conf_prep);

/**
 * blkg_conf_finish - finish up per-blkg config update
 * @ctx: blkg_conf_ctx intiailized by blkg_conf_prep()
 *
 * Finish up after per-blkg config update.  This function must be paired
 * with blkg_conf_prep().
 */
void blkg_conf_finish(struct blkg_conf_ctx *ctx)
	__releases(ctx->disk->queue->queue_lock) __releases(rcu)
{
	spin_unlock_irq(ctx->disk->queue->queue_lock);
	rcu_read_unlock();
	put_disk(ctx->disk);
}
EXPORT_SYMBOL_GPL(blkg_conf_finish);

struct cftype blkcg_files[] = {
	{
		.name = "reset_stats",
		.write_u64 = blkcg_reset_stats,
	},
	{ }	/* terminate */
};

/**
 * blkcg_css_offline - cgroup css_offline callback
 * @css: css of interest
 *
 * This function is called when @css is about to go away and responsible
 * for shooting down all blkgs associated with @css.  blkgs should be
 * removed while holding both q and blkcg locks.  As blkcg lock is nested
 * inside q lock, this function performs reverse double lock dancing.
 *
 * This is the blkcg counterpart of ioc_release_fn().
 */
static void blkcg_css_offline(struct cgroup_subsys_state *css)
{
	struct blkcg *blkcg = css_to_blkcg(css);

	spin_lock_irq(&blkcg->lock);

	while (!hlist_empty(&blkcg->blkg_list)) {
		struct blkcg_gq *blkg = hlist_entry(blkcg->blkg_list.first,
						struct blkcg_gq, blkcg_node);
		struct request_queue *q = blkg->q;

		if (spin_trylock(q->queue_lock)) {
			blkg_destroy(blkg);
			spin_unlock(q->queue_lock);
		} else {
			spin_unlock_irq(&blkcg->lock);
			cpu_relax();
			spin_lock_irq(&blkcg->lock);
		}
	}

	spin_unlock_irq(&blkcg->lock);
}

static void blkcg_css_free(struct cgroup_subsys_state *css)
{
	struct blkcg *blkcg = css_to_blkcg(css);

	if (blkcg != &blkcg_root)
		kfree(blkcg);
}

static struct cgroup_subsys_state *
blkcg_css_alloc(struct cgroup_subsys_state *parent_css)
{
	struct blkcg *blkcg;

	if (!parent_css) {
		blkcg = &blkcg_root;
		goto done;
	}

	blkcg = kzalloc(sizeof(*blkcg), GFP_KERNEL);
	if (!blkcg)
		return ERR_PTR(-ENOMEM);

	blkcg->cfq_weight = CFQ_WEIGHT_DEFAULT;
	blkcg->cfq_leaf_weight = CFQ_WEIGHT_DEFAULT;
done:
	spin_lock_init(&blkcg->lock);
	INIT_RADIX_TREE(&blkcg->blkg_tree, GFP_ATOMIC);
	INIT_HLIST_HEAD(&blkcg->blkg_list);

	return &blkcg->css;
}

/**
 * blkcg_init_queue - initialize blkcg part of request queue
 * @q: request_queue to initialize
 *
 * Called from blk_alloc_queue_node(). Responsible for initializing blkcg
 * part of new request_queue @q.
 *
 * RETURNS:
 * 0 on success, -errno on failure.
 */
int blkcg_init_queue(struct request_queue *q)
{
	struct blkcg_gq *new_blkg, *blkg;
	bool preloaded;
	int ret;

	new_blkg = blkg_alloc(&blkcg_root, q, GFP_KERNEL);
	if (!new_blkg)
		return -ENOMEM;

	preloaded = !radix_tree_preload(GFP_KERNEL);

	/*
	 * Make sure the root blkg exists and count the existing blkgs.  As
	 * @q is bypassing at this point, blkg_lookup_create() can't be
	 * used.  Open code insertion.
	 */
	rcu_read_lock();
	spin_lock_irq(q->queue_lock);
	blkg = blkg_create(&blkcg_root, q, new_blkg);
	spin_unlock_irq(q->queue_lock);
	rcu_read_unlock();

	if (preloaded)
		radix_tree_preload_end();

	if (IS_ERR(blkg))
		return PTR_ERR(blkg);

	q->root_blkg = blkg;
	q->root_rl.blkg = blkg;

	ret = blk_throtl_init(q);
	if (ret) {
		spin_lock_irq(q->queue_lock);
		blkg_destroy_all(q);
		spin_unlock_irq(q->queue_lock);
	}
	return ret;
}

/**
 * blkcg_drain_queue - drain blkcg part of request_queue
 * @q: request_queue to drain
 *
 * Called from blk_drain_queue().  Responsible for draining blkcg part.
 */
void blkcg_drain_queue(struct request_queue *q)
{
	lockdep_assert_held(q->queue_lock);

	/*
	 * @q could be exiting and already have destroyed all blkgs as
	 * indicated by NULL root_blkg.  If so, don't confuse policies.
	 */
	if (!q->root_blkg)
		return;

	blk_throtl_drain(q);
}

/**
 * blkcg_exit_queue - exit and release blkcg part of request_queue
 * @q: request_queue being released
 *
 * Called from blk_release_queue().  Responsible for exiting blkcg part.
 */
void blkcg_exit_queue(struct request_queue *q)
{
	spin_lock_irq(q->queue_lock);
	blkg_destroy_all(q);
	spin_unlock_irq(q->queue_lock);

	blk_throtl_exit(q);
}

/*
 * We cannot support shared io contexts, as we have no mean to support
 * two tasks with the same ioc in two different groups without major rework
 * of the main cic data structures.  For now we allow a task to change
 * its cgroup only if it's the only owner of its ioc.
 */
static int blkcg_can_attach(struct cgroup_subsys_state *css,
			    struct cgroup_taskset *tset)
{
	struct task_struct *task;
	struct io_context *ioc;
	int ret = 0;

	/* task_lock() is needed to avoid races with exit_io_context() */
	cgroup_taskset_for_each(task, tset) {
		task_lock(task);
		ioc = task->io_context;
		if (ioc && atomic_read(&ioc->nr_tasks) > 1)
			ret = -EINVAL;
		task_unlock(task);
		if (ret)
			break;
	}
	return ret;
}

struct cgroup_subsys blkio_cgrp_subsys = {
	.css_alloc = blkcg_css_alloc,
	.css_offline = blkcg_css_offline,
	.css_free = blkcg_css_free,
	.can_attach = blkcg_can_attach,
	.legacy_cftypes = blkcg_files,
#ifdef CONFIG_MEMCG
	/*
	 * This ensures that, if available, memcg is automatically enabled
	 * together on the default hierarchy so that the owner cgroup can
	 * be retrieved from writeback pages.
	 */
	.depends_on = 1 << memory_cgrp_id,
#endif
};
EXPORT_SYMBOL_GPL(blkio_cgrp_subsys);

/**
 * blkcg_activate_policy - activate a blkcg policy on a request_queue
 * @q: request_queue of interest
 * @pol: blkcg policy to activate
 *
 * Activate @pol on @q.  Requires %GFP_KERNEL context.  @q goes through
 * bypass mode to populate its blkgs with policy_data for @pol.
 *
 * Activation happens with @q bypassed, so nobody would be accessing blkgs
 * from IO path.  Update of each blkg is protected by both queue and blkcg
 * locks so that holding either lock and testing blkcg_policy_enabled() is
 * always enough for dereferencing policy data.
 *
 * The caller is responsible for synchronizing [de]activations and policy
 * [un]registerations.  Returns 0 on success, -errno on failure.
 */
int blkcg_activate_policy(struct request_queue *q,
			  const struct blkcg_policy *pol)
{
	LIST_HEAD(pds);
	struct blkcg_gq *blkg;
	struct blkg_policy_data *pd, *n;
	int cnt = 0, ret;

	if (blkcg_policy_enabled(q, pol))
		return 0;

	/* count and allocate policy_data for all existing blkgs */
	blk_queue_bypass_start(q);
	spin_lock_irq(q->queue_lock);
	list_for_each_entry(blkg, &q->blkg_list, q_node)
		cnt++;
	spin_unlock_irq(q->queue_lock);

	while (cnt--) {
		pd = kzalloc_node(pol->pd_size, GFP_KERNEL, q->node);
		if (!pd) {
			ret = -ENOMEM;
			goto out_free;
		}
		list_add_tail(&pd->alloc_node, &pds);
	}

	/*
	 * Install the allocated pds.  With @q bypassing, no new blkg
	 * should have been created while the queue lock was dropped.
	 */
	spin_lock_irq(q->queue_lock);

	list_for_each_entry(blkg, &q->blkg_list, q_node) {
		if (WARN_ON(list_empty(&pds))) {
			/* umm... this shouldn't happen, just abort */
			ret = -ENOMEM;
			goto out_unlock;
		}
		pd = list_first_entry(&pds, struct blkg_policy_data, alloc_node);
		list_del_init(&pd->alloc_node);

		/* grab blkcg lock too while installing @pd on @blkg */
		spin_lock(&blkg->blkcg->lock);

		blkg->pd[pol->plid] = pd;
		pd->blkg = blkg;
		pd->plid = pol->plid;
		pol->pd_init_fn(blkg);

		spin_unlock(&blkg->blkcg->lock);
	}

	__set_bit(pol->plid, q->blkcg_pols);
	ret = 0;
out_unlock:
	spin_unlock_irq(q->queue_lock);
out_free:
	blk_queue_bypass_end(q);
	list_for_each_entry_safe(pd, n, &pds, alloc_node)
		kfree(pd);
	return ret;
}
EXPORT_SYMBOL_GPL(blkcg_activate_policy);

/**
 * blkcg_deactivate_policy - deactivate a blkcg policy on a request_queue
 * @q: request_queue of interest
 * @pol: blkcg policy to deactivate
 *
 * Deactivate @pol on @q.  Follows the same synchronization rules as
 * blkcg_activate_policy().
 */
void blkcg_deactivate_policy(struct request_queue *q,
			     const struct blkcg_policy *pol)
{
	struct blkcg_gq *blkg;

	if (!blkcg_policy_enabled(q, pol))
		return;

	blk_queue_bypass_start(q);
	spin_lock_irq(q->queue_lock);

	__clear_bit(pol->plid, q->blkcg_pols);

	list_for_each_entry(blkg, &q->blkg_list, q_node) {
		/* grab blkcg lock too while removing @pd from @blkg */
		spin_lock(&blkg->blkcg->lock);

		if (pol->pd_offline_fn)
			pol->pd_offline_fn(blkg);
		if (pol->pd_exit_fn)
			pol->pd_exit_fn(blkg);

		kfree(blkg->pd[pol->plid]);
		blkg->pd[pol->plid] = NULL;

		spin_unlock(&blkg->blkcg->lock);
	}

	spin_unlock_irq(q->queue_lock);
	blk_queue_bypass_end(q);
}
EXPORT_SYMBOL_GPL(blkcg_deactivate_policy);

/**
 * blkcg_policy_register - register a blkcg policy
 * @pol: blkcg policy to register
 *
 * Register @pol with blkcg core.  Might sleep and @pol may be modified on
 * successful registration.  Returns 0 on success and -errno on failure.
 */
int blkcg_policy_register(struct blkcg_policy *pol)
{
	int i, ret;

	if (WARN_ON(pol->pd_size < sizeof(struct blkg_policy_data)))
		return -EINVAL;

	mutex_lock(&blkcg_pol_mutex);

	/* find an empty slot */
	ret = -ENOSPC;
	for (i = 0; i < BLKCG_MAX_POLS; i++)
		if (!blkcg_policy[i])
			break;
	if (i >= BLKCG_MAX_POLS)
		goto out_unlock;

	/* register and update blkgs */
	pol->plid = i;
	blkcg_policy[i] = pol;

	/* everything is in place, add intf files for the new policy */
	if (pol->cftypes)
		WARN_ON(cgroup_add_legacy_cftypes(&blkio_cgrp_subsys,
						  pol->cftypes));
	ret = 0;
out_unlock:
	mutex_unlock(&blkcg_pol_mutex);
	return ret;
}
EXPORT_SYMBOL_GPL(blkcg_policy_register);

/**
 * blkcg_policy_unregister - unregister a blkcg policy
 * @pol: blkcg policy to unregister
 *
 * Undo blkcg_policy_register(@pol).  Might sleep.
 */
void blkcg_policy_unregister(struct blkcg_policy *pol)
{
	mutex_lock(&blkcg_pol_mutex);

	if (WARN_ON(blkcg_policy[pol->plid] != pol))
		goto out_unlock;

	/* kill the intf files first */
	if (pol->cftypes)
		cgroup_rm_cftypes(pol->cftypes);

	/* unregister and update blkgs */
	blkcg_policy[pol->plid] = NULL;
out_unlock:
	mutex_unlock(&blkcg_pol_mutex);
}
EXPORT_SYMBOL_GPL(blkcg_policy_unregister);
