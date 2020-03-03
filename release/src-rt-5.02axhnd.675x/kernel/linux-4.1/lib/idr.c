/*
 * 2002-10-18  written by Jim Houston jim.houston@ccur.com
 *	Copyright (C) 2002 by Concurrent Computer Corporation
 *	Distributed under the GNU GPL license version 2.
 *
 * Modified by George Anzinger to reuse immediately and to use
 * find bit instructions.  Also removed _irq on spinlocks.
 *
 * Modified by Nadia Derbey to make it RCU safe.
 *
 * Small id to pointer translation service.
 *
 * It uses a radix tree like structure as a sparse array indexed
 * by the id to obtain the pointer.  The bitmap makes allocating
 * a new id quick.
 *
 * You call it to allocate an id (an int) an associate with that id a
 * pointer or what ever, we treat it as a (void *).  You can pass this
 * id to a user for him to pass back at a later time.  You then pass
 * that id to this code and it returns your pointer.
 */

#ifndef TEST                        // to test in user space...
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/export.h>
#endif
#include <linux/err.h>
#include <linux/string.h>
#include <linux/idr.h>
#include <linux/spinlock.h>
#include <linux/percpu.h>

#define MAX_IDR_SHIFT		(sizeof(int) * 8 - 1)
#define MAX_IDR_BIT		(1U << MAX_IDR_SHIFT)

/* Leave the possibility of an incomplete final layer */
#define MAX_IDR_LEVEL ((MAX_IDR_SHIFT + IDR_BITS - 1) / IDR_BITS)

/* Number of id_layer structs to leave in free list */
#define MAX_IDR_FREE (MAX_IDR_LEVEL * 2)

static struct kmem_cache *idr_layer_cache;
static DEFINE_PER_CPU(struct idr_layer *, idr_preload_head);
static DEFINE_PER_CPU(int, idr_preload_cnt);
static DEFINE_SPINLOCK(simple_ida_lock);

/* the maximum ID which can be allocated given idr->layers */
static int idr_max(int layers)
{
	int bits = min_t(int, layers * IDR_BITS, MAX_IDR_SHIFT);

	return (1 << bits) - 1;
}

/*
 * Prefix mask for an idr_layer at @layer.  For layer 0, the prefix mask is
 * all bits except for the lower IDR_BITS.  For layer 1, 2 * IDR_BITS, and
 * so on.
 */
static int idr_layer_prefix_mask(int layer)
{
	return ~idr_max(layer + 1);
}

static struct idr_layer *get_from_free_list(struct idr *idp)
{
	struct idr_layer *p;
	unsigned long flags;

	spin_lock_irqsave(&idp->lock, flags);
	if ((p = idp->id_free)) {
		idp->id_free = p->ary[0];
		idp->id_free_cnt--;
		p->ary[0] = NULL;
	}
	spin_unlock_irqrestore(&idp->lock, flags);
	return(p);
}

/**
 * idr_layer_alloc - allocate a new idr_layer
 * @gfp_mask: allocation mask
 * @layer_idr: optional idr to allocate from
 *
 * If @layer_idr is %NULL, directly allocate one using @gfp_mask or fetch
 * one from the per-cpu preload buffer.  If @layer_idr is not %NULL, fetch
 * an idr_layer from @idr->id_free.
 *
 * @layer_idr is to maintain backward compatibility with the old alloc
 * interface - idr_pre_get() and idr_get_new*() - and will be removed
 * together with per-pool preload buffer.
 */
static struct idr_layer *idr_layer_alloc(gfp_t gfp_mask, struct idr *layer_idr)
{
	struct idr_layer *new;

	/* this is the old path, bypass to get_from_free_list() */
	if (layer_idr)
		return get_from_free_list(layer_idr);

	/*
	 * Try to allocate directly from kmem_cache.  We want to try this
	 * before preload buffer; otherwise, non-preloading idr_alloc()
	 * users will end up taking advantage of preloading ones.  As the
	 * following is allowed to fail for preloaded cases, suppress
	 * warning this time.
	 */
	new = kmem_cache_zalloc(idr_layer_cache, gfp_mask | __GFP_NOWARN);
	if (new)
		return new;

	/*
	 * Try to fetch one from the per-cpu preload buffer if in process
	 * context.  See idr_preload() for details.
	 */
	if (!in_interrupt()) {
		preempt_disable();
		new = __this_cpu_read(idr_preload_head);
		if (new) {
			__this_cpu_write(idr_preload_head, new->ary[0]);
			__this_cpu_dec(idr_preload_cnt);
			new->ary[0] = NULL;
		}
		preempt_enable();
		if (new)
			return new;
	}

	/*
	 * Both failed.  Try kmem_cache again w/o adding __GFP_NOWARN so
	 * that memory allocation failure warning is printed as intended.
	 */
	return kmem_cache_zalloc(idr_layer_cache, gfp_mask);
}

static void idr_layer_rcu_free(struct rcu_head *head)
{
	struct idr_layer *layer;

	layer = container_of(head, struct idr_layer, rcu_head);
	kmem_cache_free(idr_layer_cache, layer);
}

static inline void free_layer(struct idr *idr, struct idr_layer *p)
{
	if (idr->hint == p)
		RCU_INIT_POINTER(idr->hint, NULL);
	call_rcu(&p->rcu_head, idr_layer_rcu_free);
}

/* only called when idp->lock is held */
static void __move_to_free_list(struct idr *idp, struct idr_layer *p)
{
	p->ary[0] = idp->id_free;
	idp->id_free = p;
	idp->id_free_cnt++;
}

static void move_to_free_list(struct idr *idp, struct idr_layer *p)
{
	unsigned long flags;

	/*
	 * Depends on the return element being zeroed.
	 */
	spin_lock_irqsave(&idp->lock, flags);
	__move_to_free_list(idp, p);
	spin_unlock_irqrestore(&idp->lock, flags);
}

static void idr_mark_full(struct idr_layer **pa, int id)
{
	struct idr_layer *p = pa[0];
	int l = 0;

	__set_bit(id & IDR_MASK, p->bitmap);
	/*
	 * If this layer is full mark the bit in the layer above to
	 * show that this part of the radix tree is full.  This may
	 * complete the layer above and require walking up the radix
	 * tree.
	 */
	while (bitmap_full(p->bitmap, IDR_SIZE)) {
		if (!(p = pa[++l]))
			break;
		id = id >> IDR_BITS;
		__set_bit((id & IDR_MASK), p->bitmap);
	}
}

static int __idr_pre_get(struct idr *idp, gfp_t gfp_mask)
{
	while (idp->id_free_cnt < MAX_IDR_FREE) {
		struct idr_layer *new;
		new = kmem_cache_zalloc(idr_layer_cache, gfp_mask);
		if (new == NULL)
			return (0);
		move_to_free_list(idp, new);
	}
	return 1;
}

/**
 * sub_alloc - try to allocate an id without growing the tree depth
 * @idp: idr handle
 * @starting_id: id to start search at
 * @pa: idr_layer[MAX_IDR_LEVEL] used as backtrack buffer
 * @gfp_mask: allocation mask for idr_layer_alloc()
 * @layer_idr: optional idr passed to idr_layer_alloc()
 *
 * Allocate an id in range [@starting_id, INT_MAX] from @idp without
 * growing its depth.  Returns
 *
 *  the allocated id >= 0 if successful,
 *  -EAGAIN if the tree needs to grow for allocation to succeed,
 *  -ENOSPC if the id space is exhausted,
 *  -ENOMEM if more idr_layers need to be allocated.
 */
static int sub_alloc(struct idr *idp, int *starting_id, struct idr_layer **pa,
		     gfp_t gfp_mask, struct idr *layer_idr)
{
	int n, m, sh;
	struct idr_layer *p, *new;
	int l, id, oid;

	id = *starting_id;
 restart:
	p = idp->top;
	l = idp->layers;
	pa[l--] = NULL;
	while (1) {
		/*
		 * We run around this while until we reach the leaf node...
		 */
		n = (id >> (IDR_BITS*l)) & IDR_MASK;
		m = find_next_zero_bit(p->bitmap, IDR_SIZE, n);
		if (m == IDR_SIZE) {
			/* no space available go back to previous layer. */
			l++;
			oid = id;
			id = (id | ((1 << (IDR_BITS * l)) - 1)) + 1;

			/* if already at the top layer, we need to grow */
			if (id > idr_max(idp->layers)) {
				*starting_id = id;
				return -EAGAIN;
			}
			p = pa[l];
			BUG_ON(!p);

			/* If we need to go up one layer, continue the
			 * loop; otherwise, restart from the top.
			 */
			sh = IDR_BITS * (l + 1);
			if (oid >> sh == id >> sh)
				continue;
			else
				goto restart;
		}
		if (m != n) {
			sh = IDR_BITS*l;
			id = ((id >> sh) ^ n ^ m) << sh;
		}
		if ((id >= MAX_IDR_BIT) || (id < 0))
			return -ENOSPC;
		if (l == 0)
			break;
		/*
		 * Create the layer below if it is missing.
		 */
		if (!p->ary[m]) {
			new = idr_layer_alloc(gfp_mask, layer_idr);
			if (!new)
				return -ENOMEM;
			new->layer = l-1;
			new->prefix = id & idr_layer_prefix_mask(new->layer);
			rcu_assign_pointer(p->ary[m], new);
			p->count++;
		}
		pa[l--] = p;
		p = p->ary[m];
	}

	pa[l] = p;
	return id;
}

static int idr_get_empty_slot(struct idr *idp, int starting_id,
			      struct idr_layer **pa, gfp_t gfp_mask,
			      struct idr *layer_idr)
{
	struct idr_layer *p, *new;
	int layers, v, id;
	unsigned long flags;

	id = starting_id;
build_up:
	p = idp->top;
	layers = idp->layers;
	if (unlikely(!p)) {
		if (!(p = idr_layer_alloc(gfp_mask, layer_idr)))
			return -ENOMEM;
		p->layer = 0;
		layers = 1;
	}
	/*
	 * Add a new layer to the top of the tree if the requested
	 * id is larger than the currently allocated space.
	 */
	while (id > idr_max(layers)) {
		layers++;
		if (!p->count) {
			/* special case: if the tree is currently empty,
			 * then we grow the tree by moving the top node
			 * upwards.
			 */
			p->layer++;
			WARN_ON_ONCE(p->prefix);
			continue;
		}
		if (!(new = idr_layer_alloc(gfp_mask, layer_idr))) {
			/*
			 * The allocation failed.  If we built part of
			 * the structure tear it down.
			 */
			spin_lock_irqsave(&idp->lock, flags);
			for (new = p; p && p != idp->top; new = p) {
				p = p->ary[0];
				new->ary[0] = NULL;
				new->count = 0;
				bitmap_clear(new->bitmap, 0, IDR_SIZE);
				__move_to_free_list(idp, new);
			}
			spin_unlock_irqrestore(&idp->lock, flags);
			return -ENOMEM;
		}
		new->ary[0] = p;
		new->count = 1;
		new->layer = layers-1;
		new->prefix = id & idr_layer_prefix_mask(new->layer);
		if (bitmap_full(p->bitmap, IDR_SIZE))
			__set_bit(0, new->bitmap);
		p = new;
	}
	rcu_assign_pointer(idp->top, p);
	idp->layers = layers;
	v = sub_alloc(idp, &id, pa, gfp_mask, layer_idr);
	if (v == -EAGAIN)
		goto build_up;
	return(v);
}

/*
 * @id and @pa are from a successful allocation from idr_get_empty_slot().
 * Install the user pointer @ptr and mark the slot full.
 */
static void idr_fill_slot(struct idr *idr, void *ptr, int id,
			  struct idr_layer **pa)
{
	/* update hint used for lookup, cleared from free_layer() */
	rcu_assign_pointer(idr->hint, pa[0]);

	rcu_assign_pointer(pa[0]->ary[id & IDR_MASK], (struct idr_layer *)ptr);
	pa[0]->count++;
	idr_mark_full(pa, id);
}


/**
 * idr_preload - preload for idr_alloc()
 * @gfp_mask: allocation mask to use for preloading
 *
 * Preload per-cpu layer buffer for idr_alloc().  Can only be used from
 * process context and each idr_preload() invocation should be matched with
 * idr_preload_end().  Note that preemption is disabled while preloaded.
 *
 * The first idr_alloc() in the preloaded section can be treated as if it
 * were invoked with @gfp_mask used for preloading.  This allows using more
 * permissive allocation masks for idrs protected by spinlocks.
 *
 * For example, if idr_alloc() below fails, the failure can be treated as
 * if idr_alloc() were called with GFP_KERNEL rather than GFP_NOWAIT.
 *
 *	idr_preload(GFP_KERNEL);
 *	spin_lock(lock);
 *
 *	id = idr_alloc(idr, ptr, start, end, GFP_NOWAIT);
 *
 *	spin_unlock(lock);
 *	idr_preload_end();
 *	if (id < 0)
 *		error;
 */
void idr_preload(gfp_t gfp_mask)
{
	/*
	 * Consuming preload buffer from non-process context breaks preload
	 * allocation guarantee.  Disallow usage from those contexts.
	 */
	WARN_ON_ONCE(in_interrupt());
	might_sleep_if(gfp_mask & __GFP_WAIT);

	preempt_disable();

	/*
	 * idr_alloc() is likely to succeed w/o full idr_layer buffer and
	 * return value from idr_alloc() needs to be checked for failure
	 * anyway.  Silently give up if allocation fails.  The caller can
	 * treat failures from idr_alloc() as if idr_alloc() were called
	 * with @gfp_mask which should be enough.
	 */
	while (__this_cpu_read(idr_preload_cnt) < MAX_IDR_FREE) {
		struct idr_layer *new;

		preempt_enable();
		new = kmem_cache_zalloc(idr_layer_cache, gfp_mask);
		preempt_disable();
		if (!new)
			break;

		/* link the new one to per-cpu preload list */
		new->ary[0] = __this_cpu_read(idr_preload_head);
		__this_cpu_write(idr_preload_head, new);
		__this_cpu_inc(idr_preload_cnt);
	}
}
EXPORT_SYMBOL(idr_preload);

/**
 * idr_alloc - allocate new idr entry
 * @idr: the (initialized) idr
 * @ptr: pointer to be associated with the new id
 * @start: the minimum id (inclusive)
 * @end: the maximum id (exclusive, <= 0 for max)
 * @gfp_mask: memory allocation flags
 *
 * Allocate an id in [start, end) and associate it with @ptr.  If no ID is
 * available in the specified range, returns -ENOSPC.  On memory allocation
 * failure, returns -ENOMEM.
 *
 * Note that @end is treated as max when <= 0.  This is to always allow
 * using @start + N as @end as long as N is inside integer range.
 *
 * The user is responsible for exclusively synchronizing all operations
 * which may modify @idr.  However, read-only accesses such as idr_find()
 * or iteration can be performed under RCU read lock provided the user
 * destroys @ptr in RCU-safe way after removal from idr.
 */
int idr_alloc(struct idr *idr, void *ptr, int start, int end, gfp_t gfp_mask)
{
	int max = end > 0 ? end - 1 : INT_MAX;	/* inclusive upper limit */
	struct idr_layer *pa[MAX_IDR_LEVEL + 1];
	int id;

	might_sleep_if(gfp_mask & __GFP_WAIT);

	/* sanity checks */
	if (WARN_ON_ONCE(start < 0))
		return -EINVAL;
	if (unlikely(max < start))
		return -ENOSPC;

	/* allocate id */
	id = idr_get_empty_slot(idr, start, pa, gfp_mask, NULL);
	if (unlikely(id < 0))
		return id;
	if (unlikely(id > max))
		return -ENOSPC;

	idr_fill_slot(idr, ptr, id, pa);
	return id;
}
EXPORT_SYMBOL_GPL(idr_alloc);

/**
 * idr_alloc_cyclic - allocate new idr entry in a cyclical fashion
 * @idr: the (initialized) idr
 * @ptr: pointer to be associated with the new id
 * @start: the minimum id (inclusive)
 * @end: the maximum id (exclusive, <= 0 for max)
 * @gfp_mask: memory allocation flags
 *
 * Essentially the same as idr_alloc, but prefers to allocate progressively
 * higher ids if it can. If the "cur" counter wraps, then it will start again
 * at the "start" end of the range and allocate one that has already been used.
 */
int idr_alloc_cyclic(struct idr *idr, void *ptr, int start, int end,
			gfp_t gfp_mask)
{
	int id;

	id = idr_alloc(idr, ptr, max(start, idr->cur), end, gfp_mask);
	if (id == -ENOSPC)
		id = idr_alloc(idr, ptr, start, end, gfp_mask);

	if (likely(id >= 0))
		idr->cur = id + 1;
	return id;
}
EXPORT_SYMBOL(idr_alloc_cyclic);

static void idr_remove_warning(int id)
{
	WARN(1, "idr_remove called for id=%d which is not allocated.\n", id);
}

static void sub_remove(struct idr *idp, int shift, int id)
{
	struct idr_layer *p = idp->top;
	struct idr_layer **pa[MAX_IDR_LEVEL + 1];
	struct idr_layer ***paa = &pa[0];
	struct idr_layer *to_free;
	int n;

	*paa = NULL;
	*++paa = &idp->top;

	while ((shift > 0) && p) {
		n = (id >> shift) & IDR_MASK;
		__clear_bit(n, p->bitmap);
		*++paa = &p->ary[n];
		p = p->ary[n];
		shift -= IDR_BITS;
	}
	n = id & IDR_MASK;
	if (likely(p != NULL && test_bit(n, p->bitmap))) {
		__clear_bit(n, p->bitmap);
		RCU_INIT_POINTER(p->ary[n], NULL);
		to_free = NULL;
		while(*paa && ! --((**paa)->count)){
			if (to_free)
				free_layer(idp, to_free);
			to_free = **paa;
			**paa-- = NULL;
		}
		if (!*paa)
			idp->layers = 0;
		if (to_free)
			free_layer(idp, to_free);
	} else
		idr_remove_warning(id);
}

/**
 * idr_remove - remove the given id and free its slot
 * @idp: idr handle
 * @id: unique key
 */
void idr_remove(struct idr *idp, int id)
{
	struct idr_layer *p;
	struct idr_layer *to_free;

	if (id < 0)
		return;

	if (id > idr_max(idp->layers)) {
		idr_remove_warning(id);
		return;
	}

	sub_remove(idp, (idp->layers - 1) * IDR_BITS, id);
	if (idp->top && idp->top->count == 1 && (idp->layers > 1) &&
	    idp->top->ary[0]) {
		/*
		 * Single child at leftmost slot: we can shrink the tree.
		 * This level is not needed anymore since when layers are
		 * inserted, they are inserted at the top of the existing
		 * tree.
		 */
		to_free = idp->top;
		p = idp->top->ary[0];
		rcu_assign_pointer(idp->top, p);
		--idp->layers;
		to_free->count = 0;
		bitmap_clear(to_free->bitmap, 0, IDR_SIZE);
		free_layer(idp, to_free);
	}
}
EXPORT_SYMBOL(idr_remove);

static void __idr_remove_all(struct idr *idp)
{
	int n, id, max;
	int bt_mask;
	struct idr_layer *p;
	struct idr_layer *pa[MAX_IDR_LEVEL + 1];
	struct idr_layer **paa = &pa[0];

	n = idp->layers * IDR_BITS;
	*paa = idp->top;
	RCU_INIT_POINTER(idp->top, NULL);
	max = idr_max(idp->layers);

	id = 0;
	while (id >= 0 && id <= max) {
		p = *paa;
		while (n > IDR_BITS && p) {
			n -= IDR_BITS;
			p = p->ary[(id >> n) & IDR_MASK];
			*++paa = p;
		}

		bt_mask = id;
		id += 1 << n;
		/* Get the highest bit that the above add changed from 0->1. */
		while (n < fls(id ^ bt_mask)) {
			if (*paa)
				free_layer(idp, *paa);
			n += IDR_BITS;
			--paa;
		}
	}
	idp->layers = 0;
}

/**
 * idr_destroy - release all cached layers within an idr tree
 * @idp: idr handle
 *
 * Free all id mappings and all idp_layers.  After this function, @idp is
 * completely unused and can be freed / recycled.  The caller is
 * responsible for ensuring that no one else accesses @idp during or after
 * idr_destroy().
 *
 * A typical clean-up sequence for objects stored in an idr tree will use
 * idr_for_each() to free all objects, if necessary, then idr_destroy() to
 * free up the id mappings and cached idr_layers.
 */
void idr_destroy(struct idr *idp)
{
	__idr_remove_all(idp);

	while (idp->id_free_cnt) {
		struct idr_layer *p = get_from_free_list(idp);
		kmem_cache_free(idr_layer_cache, p);
	}
}
EXPORT_SYMBOL(idr_destroy);

void *idr_find_slowpath(struct idr *idp, int id)
{
	int n;
	struct idr_layer *p;

	if (id < 0)
		return NULL;

	p = rcu_dereference_raw(idp->top);
	if (!p)
		return NULL;
	n = (p->layer+1) * IDR_BITS;

	if (id > idr_max(p->layer + 1))
		return NULL;
	BUG_ON(n == 0);

	while (n > 0 && p) {
		n -= IDR_BITS;
		BUG_ON(n != p->layer*IDR_BITS);
		p = rcu_dereference_raw(p->ary[(id >> n) & IDR_MASK]);
	}
	return((void *)p);
}
EXPORT_SYMBOL(idr_find_slowpath);

/**
 * idr_for_each - iterate through all stored pointers
 * @idp: idr handle
 * @fn: function to be called for each pointer
 * @data: data passed back to callback function
 *
 * Iterate over the pointers registered with the given idr.  The
 * callback function will be called for each pointer currently
 * registered, passing the id, the pointer and the data pointer passed
 * to this function.  It is not safe to modify the idr tree while in
 * the callback, so functions such as idr_get_new and idr_remove are
 * not allowed.
 *
 * We check the return of @fn each time. If it returns anything other
 * than %0, we break out and return that value.
 *
 * The caller must serialize idr_for_each() vs idr_get_new() and idr_remove().
 */
int idr_for_each(struct idr *idp,
		 int (*fn)(int id, void *p, void *data), void *data)
{
	int n, id, max, error = 0;
	struct idr_layer *p;
	struct idr_layer *pa[MAX_IDR_LEVEL + 1];
	struct idr_layer **paa = &pa[0];

	n = idp->layers * IDR_BITS;
	*paa = rcu_dereference_raw(idp->top);
	max = idr_max(idp->layers);

	id = 0;
	while (id >= 0 && id <= max) {
		p = *paa;
		while (n > 0 && p) {
			n -= IDR_BITS;
			p = rcu_dereference_raw(p->ary[(id >> n) & IDR_MASK]);
			*++paa = p;
		}

		if (p) {
			error = fn(id, (void *)p, data);
			if (error)
				break;
		}

		id += 1 << n;
		while (n < fls(id)) {
			n += IDR_BITS;
			--paa;
		}
	}

	return error;
}
EXPORT_SYMBOL(idr_for_each);

/**
 * idr_get_next - lookup next object of id to given id.
 * @idp: idr handle
 * @nextidp:  pointer to lookup key
 *
 * Returns pointer to registered object with id, which is next number to
 * given id. After being looked up, *@nextidp will be updated for the next
 * iteration.
 *
 * This function can be called under rcu_read_lock(), given that the leaf
 * pointers lifetimes are correctly managed.
 */
void *idr_get_next(struct idr *idp, int *nextidp)
{
	struct idr_layer *p, *pa[MAX_IDR_LEVEL + 1];
	struct idr_layer **paa = &pa[0];
	int id = *nextidp;
	int n, max;

	/* find first ent */
	p = *paa = rcu_dereference_raw(idp->top);
	if (!p)
		return NULL;
	n = (p->layer + 1) * IDR_BITS;
	max = idr_max(p->layer + 1);

	while (id >= 0 && id <= max) {
		p = *paa;
		while (n > 0 && p) {
			n -= IDR_BITS;
			p = rcu_dereference_raw(p->ary[(id >> n) & IDR_MASK]);
			*++paa = p;
		}

		if (p) {
			*nextidp = id;
			return p;
		}

		/*
		 * Proceed to the next layer at the current level.  Unlike
		 * idr_for_each(), @id isn't guaranteed to be aligned to
		 * layer boundary at this point and adding 1 << n may
		 * incorrectly skip IDs.  Make sure we jump to the
		 * beginning of the next layer using round_up().
		 */
		id = round_up(id + 1, 1 << n);
		while (n < fls(id)) {
			n += IDR_BITS;
			--paa;
		}
	}
	return NULL;
}
EXPORT_SYMBOL(idr_get_next);


/**
 * idr_replace - replace pointer for given id
 * @idp: idr handle
 * @ptr: pointer you want associated with the id
 * @id: lookup key
 *
 * Replace the pointer registered with an id and return the old value.
 * A %-ENOENT return indicates that @id was not found.
 * A %-EINVAL return indicates that @id was not within valid constraints.
 *
 * The caller must serialize with writers.
 */
void *idr_replace(struct idr *idp, void *ptr, int id)
{
	int n;
	struct idr_layer *p, *old_p;

	if (id < 0)
		return ERR_PTR(-EINVAL);

	p = idp->top;
	if (!p)
		return ERR_PTR(-ENOENT);

	if (id > idr_max(p->layer + 1))
		return ERR_PTR(-ENOENT);

	n = p->layer * IDR_BITS;
	while ((n > 0) && p) {
		p = p->ary[(id >> n) & IDR_MASK];
		n -= IDR_BITS;
	}

	n = id & IDR_MASK;
	if (unlikely(p == NULL || !test_bit(n, p->bitmap)))
		return ERR_PTR(-ENOENT);

	old_p = p->ary[n];
	rcu_assign_pointer(p->ary[n], ptr);

	return old_p;
}
EXPORT_SYMBOL(idr_replace);

void __init idr_init_cache(void)
{
	idr_layer_cache = kmem_cache_create("idr_layer_cache",
				sizeof(struct idr_layer), 0, SLAB_PANIC, NULL);
}

/**
 * idr_init - initialize idr handle
 * @idp:	idr handle
 *
 * This function is use to set up the handle (@idp) that you will pass
 * to the rest of the functions.
 */
void idr_init(struct idr *idp)
{
	memset(idp, 0, sizeof(struct idr));
	spin_lock_init(&idp->lock);
}
EXPORT_SYMBOL(idr_init);

static int idr_has_entry(int id, void *p, void *data)
{
	return 1;
}

bool idr_is_empty(struct idr *idp)
{
	return !idr_for_each(idp, idr_has_entry, NULL);
}
EXPORT_SYMBOL(idr_is_empty);

/**
 * DOC: IDA description
 * IDA - IDR based ID allocator
 *
 * This is id allocator without id -> pointer translation.  Memory
 * usage is much lower than full blown idr because each id only
 * occupies a bit.  ida uses a custom leaf node which contains
 * IDA_BITMAP_BITS slots.
 *
 * 2007-04-25  written by Tejun Heo <htejun@gmail.com>
 */

static void free_bitmap(struct ida *ida, struct ida_bitmap *bitmap)
{
	unsigned long flags;

	if (!ida->free_bitmap) {
		spin_lock_irqsave(&ida->idr.lock, flags);
		if (!ida->free_bitmap) {
			ida->free_bitmap = bitmap;
			bitmap = NULL;
		}
		spin_unlock_irqrestore(&ida->idr.lock, flags);
	}

	kfree(bitmap);
}

/**
 * ida_pre_get - reserve resources for ida allocation
 * @ida:	ida handle
 * @gfp_mask:	memory allocation flag
 *
 * This function should be called prior to locking and calling the
 * following function.  It preallocates enough memory to satisfy the
 * worst possible allocation.
 *
 * If the system is REALLY out of memory this function returns %0,
 * otherwise %1.
 */
int ida_pre_get(struct ida *ida, gfp_t gfp_mask)
{
	/* allocate idr_layers */
	if (!__idr_pre_get(&ida->idr, gfp_mask))
		return 0;

	/* allocate free_bitmap */
	if (!ida->free_bitmap) {
		struct ida_bitmap *bitmap;

		bitmap = kmalloc(sizeof(struct ida_bitmap), gfp_mask);
		if (!bitmap)
			return 0;

		free_bitmap(ida, bitmap);
	}

	return 1;
}
EXPORT_SYMBOL(ida_pre_get);

/**
 * ida_get_new_above - allocate new ID above or equal to a start id
 * @ida:	ida handle
 * @starting_id: id to start search at
 * @p_id:	pointer to the allocated handle
 *
 * Allocate new ID above or equal to @starting_id.  It should be called
 * with any required locks.
 *
 * If memory is required, it will return %-EAGAIN, you should unlock
 * and go back to the ida_pre_get() call.  If the ida is full, it will
 * return %-ENOSPC.
 *
 * @p_id returns a value in the range @starting_id ... %0x7fffffff.
 */
int ida_get_new_above(struct ida *ida, int starting_id, int *p_id)
{
	struct idr_layer *pa[MAX_IDR_LEVEL + 1];
	struct ida_bitmap *bitmap;
	unsigned long flags;
	int idr_id = starting_id / IDA_BITMAP_BITS;
	int offset = starting_id % IDA_BITMAP_BITS;
	int t, id;

 restart:
	/* get vacant slot */
	t = idr_get_empty_slot(&ida->idr, idr_id, pa, 0, &ida->idr);
	if (t < 0)
		return t == -ENOMEM ? -EAGAIN : t;

	if (t * IDA_BITMAP_BITS >= MAX_IDR_BIT)
		return -ENOSPC;

	if (t != idr_id)
		offset = 0;
	idr_id = t;

	/* if bitmap isn't there, create a new one */
	bitmap = (void *)pa[0]->ary[idr_id & IDR_MASK];
	if (!bitmap) {
		spin_lock_irqsave(&ida->idr.lock, flags);
		bitmap = ida->free_bitmap;
		ida->free_bitmap = NULL;
		spin_unlock_irqrestore(&ida->idr.lock, flags);

		if (!bitmap)
			return -EAGAIN;

		memset(bitmap, 0, sizeof(struct ida_bitmap));
		rcu_assign_pointer(pa[0]->ary[idr_id & IDR_MASK],
				(void *)bitmap);
		pa[0]->count++;
	}

	/* lookup for empty slot */
	t = find_next_zero_bit(bitmap->bitmap, IDA_BITMAP_BITS, offset);
	if (t == IDA_BITMAP_BITS) {
		/* no empty slot after offset, continue to the next chunk */
		idr_id++;
		offset = 0;
		goto restart;
	}

	id = idr_id * IDA_BITMAP_BITS + t;
	if (id >= MAX_IDR_BIT)
		return -ENOSPC;

	__set_bit(t, bitmap->bitmap);
	if (++bitmap->nr_busy == IDA_BITMAP_BITS)
		idr_mark_full(pa, idr_id);

	*p_id = id;

	/* Each leaf node can handle nearly a thousand slots and the
	 * whole idea of ida is to have small memory foot print.
	 * Throw away extra resources one by one after each successful
	 * allocation.
	 */
	if (ida->idr.id_free_cnt || ida->free_bitmap) {
		struct idr_layer *p = get_from_free_list(&ida->idr);
		if (p)
			kmem_cache_free(idr_layer_cache, p);
	}

	return 0;
}
EXPORT_SYMBOL(ida_get_new_above);

/**
 * ida_remove - remove the given ID
 * @ida:	ida handle
 * @id:		ID to free
 */
void ida_remove(struct ida *ida, int id)
{
	struct idr_layer *p = ida->idr.top;
	int shift = (ida->idr.layers - 1) * IDR_BITS;
	int idr_id = id / IDA_BITMAP_BITS;
	int offset = id % IDA_BITMAP_BITS;
	int n;
	struct ida_bitmap *bitmap;

	if (idr_id > idr_max(ida->idr.layers))
		goto err;

	/* clear full bits while looking up the leaf idr_layer */
	while ((shift > 0) && p) {
		n = (idr_id >> shift) & IDR_MASK;
		__clear_bit(n, p->bitmap);
		p = p->ary[n];
		shift -= IDR_BITS;
	}

	if (p == NULL)
		goto err;

	n = idr_id & IDR_MASK;
	__clear_bit(n, p->bitmap);

	bitmap = (void *)p->ary[n];
	if (!bitmap || !test_bit(offset, bitmap->bitmap))
		goto err;

	/* update bitmap and remove it if empty */
	__clear_bit(offset, bitmap->bitmap);
	if (--bitmap->nr_busy == 0) {
		__set_bit(n, p->bitmap);	/* to please idr_remove() */
		idr_remove(&ida->idr, idr_id);
		free_bitmap(ida, bitmap);
	}

	return;

 err:
	WARN(1, "ida_remove called for id=%d which is not allocated.\n", id);
}
EXPORT_SYMBOL(ida_remove);

/**
 * ida_destroy - release all cached layers within an ida tree
 * @ida:		ida handle
 */
void ida_destroy(struct ida *ida)
{
	idr_destroy(&ida->idr);
	kfree(ida->free_bitmap);
}
EXPORT_SYMBOL(ida_destroy);

/**
 * ida_simple_get - get a new id.
 * @ida: the (initialized) ida.
 * @start: the minimum id (inclusive, < 0x8000000)
 * @end: the maximum id (exclusive, < 0x8000000 or 0)
 * @gfp_mask: memory allocation flags
 *
 * Allocates an id in the range start <= id < end, or returns -ENOSPC.
 * On memory allocation failure, returns -ENOMEM.
 *
 * Use ida_simple_remove() to get rid of an id.
 */
int ida_simple_get(struct ida *ida, unsigned int start, unsigned int end,
		   gfp_t gfp_mask)
{
	int ret, id;
	unsigned int max;
	unsigned long flags;

	BUG_ON((int)start < 0);
	BUG_ON((int)end < 0);

	if (end == 0)
		max = 0x80000000;
	else {
		BUG_ON(end < start);
		max = end - 1;
	}

again:
	if (!ida_pre_get(ida, gfp_mask))
		return -ENOMEM;

	spin_lock_irqsave(&simple_ida_lock, flags);
	ret = ida_get_new_above(ida, start, &id);
	if (!ret) {
		if (id > max) {
			ida_remove(ida, id);
			ret = -ENOSPC;
		} else {
			ret = id;
		}
	}
	spin_unlock_irqrestore(&simple_ida_lock, flags);

	if (unlikely(ret == -EAGAIN))
		goto again;

	return ret;
}
EXPORT_SYMBOL(ida_simple_get);

/**
 * ida_simple_remove - remove an allocated id.
 * @ida: the (initialized) ida.
 * @id: the id returned by ida_simple_get.
 */
void ida_simple_remove(struct ida *ida, unsigned int id)
{
	unsigned long flags;

	BUG_ON((int)id < 0);
	spin_lock_irqsave(&simple_ida_lock, flags);
	ida_remove(ida, id);
	spin_unlock_irqrestore(&simple_ida_lock, flags);
}
EXPORT_SYMBOL(ida_simple_remove);

/**
 * ida_init - initialize ida handle
 * @ida:	ida handle
 *
 * This function is use to set up the handle (@ida) that you will pass
 * to the rest of the functions.
 */
void ida_init(struct ida *ida)
{
	memset(ida, 0, sizeof(struct ida));
	idr_init(&ida->idr);

}
EXPORT_SYMBOL(ida_init);
