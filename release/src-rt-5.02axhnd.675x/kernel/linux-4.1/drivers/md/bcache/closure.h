#ifndef _LINUX_CLOSURE_H
#define _LINUX_CLOSURE_H

#include <linux/llist.h>
#include <linux/sched.h>
#include <linux/workqueue.h>

/*
 * Closure is perhaps the most overused and abused term in computer science, but
 * since I've been unable to come up with anything better you're stuck with it
 * again.
 *
 * What are closures?
 *
 * They embed a refcount. The basic idea is they count "things that are in
 * progress" - in flight bios, some other thread that's doing something else -
 * anything you might want to wait on.
 *
 * The refcount may be manipulated with closure_get() and closure_put().
 * closure_put() is where many of the interesting things happen, when it causes
 * the refcount to go to 0.
 *
 * Closures can be used to wait on things both synchronously and asynchronously,
 * and synchronous and asynchronous use can be mixed without restriction. To
 * wait synchronously, use closure_sync() - you will sleep until your closure's
 * refcount hits 1.
 *
 * To wait asynchronously, use
 *   continue_at(cl, next_function, workqueue);
 *
 * passing it, as you might expect, the function to run when nothing is pending
 * and the workqueue to run that function out of.
 *
 * continue_at() also, critically, is a macro that returns the calling function.
 * There's good reason for this.
 *
 * To use safely closures asynchronously, they must always have a refcount while
 * they are running owned by the thread that is running them. Otherwise, suppose
 * you submit some bios and wish to have a function run when they all complete:
 *
 * foo_endio(struct bio *bio, int error)
 * {
 *	closure_put(cl);
 * }
 *
 * closure_init(cl);
 *
 * do_stuff();
 * closure_get(cl);
 * bio1->bi_endio = foo_endio;
 * bio_submit(bio1);
 *
 * do_more_stuff();
 * closure_get(cl);
 * bio2->bi_endio = foo_endio;
 * bio_submit(bio2);
 *
 * continue_at(cl, complete_some_read, system_wq);
 *
 * If closure's refcount started at 0, complete_some_read() could run before the
 * second bio was submitted - which is almost always not what you want! More
 * importantly, it wouldn't be possible to say whether the original thread or
 * complete_some_read()'s thread owned the closure - and whatever state it was
 * associated with!
 *
 * So, closure_init() initializes a closure's refcount to 1 - and when a
 * closure_fn is run, the refcount will be reset to 1 first.
 *
 * Then, the rule is - if you got the refcount with closure_get(), release it
 * with closure_put() (i.e, in a bio->bi_endio function). If you have a refcount
 * on a closure because you called closure_init() or you were run out of a
 * closure - _always_ use continue_at(). Doing so consistently will help
 * eliminate an entire class of particularly pernicious races.
 *
 * Lastly, you might have a wait list dedicated to a specific event, and have no
 * need for specifying the condition - you just want to wait until someone runs
 * closure_wake_up() on the appropriate wait list. In that case, just use
 * closure_wait(). It will return either true or false, depending on whether the
 * closure was already on a wait list or not - a closure can only be on one wait
 * list at a time.
 *
 * Parents:
 *
 * closure_init() takes two arguments - it takes the closure to initialize, and
 * a (possibly null) parent.
 *
 * If parent is non null, the new closure will have a refcount for its lifetime;
 * a closure is considered to be "finished" when its refcount hits 0 and the
 * function to run is null. Hence
 *
 * continue_at(cl, NULL, NULL);
 *
 * returns up the (spaghetti) stack of closures, precisely like normal return
 * returns up the C stack. continue_at() with non null fn is better thought of
 * as doing a tail call.
 *
 * All this implies that a closure should typically be embedded in a particular
 * struct (which its refcount will normally control the lifetime of), and that
 * struct can very much be thought of as a stack frame.
 */

struct closure;
typedef void (closure_fn) (struct closure *);

struct closure_waitlist {
	struct llist_head	list;
};

enum closure_state {
	/*
	 * CLOSURE_WAITING: Set iff the closure is on a waitlist. Must be set by
	 * the thread that owns the closure, and cleared by the thread that's
	 * waking up the closure.
	 *
	 * CLOSURE_SLEEPING: Must be set before a thread uses a closure to sleep
	 * - indicates that cl->task is valid and closure_put() may wake it up.
	 * Only set or cleared by the thread that owns the closure.
	 *
	 * The rest are for debugging and don't affect behaviour:
	 *
	 * CLOSURE_RUNNING: Set when a closure is running (i.e. by
	 * closure_init() and when closure_put() runs then next function), and
	 * must be cleared before remaining hits 0. Primarily to help guard
	 * against incorrect usage and accidentally transferring references.
	 * continue_at() and closure_return() clear it for you, if you're doing
	 * something unusual you can use closure_set_dead() which also helps
	 * annotate where references are being transferred.
	 *
	 * CLOSURE_STACK: Sanity check - remaining should never hit 0 on a
	 * closure with this flag set
	 */

	CLOSURE_BITS_START	= (1 << 23),
	CLOSURE_DESTRUCTOR	= (1 << 23),
	CLOSURE_WAITING		= (1 << 25),
	CLOSURE_SLEEPING	= (1 << 27),
	CLOSURE_RUNNING		= (1 << 29),
	CLOSURE_STACK		= (1 << 31),
};

#define CLOSURE_GUARD_MASK					\
	((CLOSURE_DESTRUCTOR|CLOSURE_WAITING|CLOSURE_SLEEPING|	\
	  CLOSURE_RUNNING|CLOSURE_STACK) << 1)

#define CLOSURE_REMAINING_MASK		(CLOSURE_BITS_START - 1)
#define CLOSURE_REMAINING_INITIALIZER	(1|CLOSURE_RUNNING)

struct closure {
	union {
		struct {
			struct workqueue_struct *wq;
			struct task_struct	*task;
			struct llist_node	list;
			closure_fn		*fn;
		};
		struct work_struct	work;
	};

	struct closure		*parent;

	atomic_t		remaining;

#ifdef CONFIG_BCACHE_CLOSURES_DEBUG
#define CLOSURE_MAGIC_DEAD	0xc054dead
#define CLOSURE_MAGIC_ALIVE	0xc054a11e

	unsigned		magic;
	struct list_head	all;
	unsigned long		ip;
	unsigned long		waiting_on;
#endif
};

void closure_sub(struct closure *cl, int v);
void closure_put(struct closure *cl);
void __closure_wake_up(struct closure_waitlist *list);
bool closure_wait(struct closure_waitlist *list, struct closure *cl);
void closure_sync(struct closure *cl);

#ifdef CONFIG_BCACHE_CLOSURES_DEBUG

void closure_debug_init(void);
void closure_debug_create(struct closure *cl);
void closure_debug_destroy(struct closure *cl);

#else

static inline void closure_debug_init(void) {}
static inline void closure_debug_create(struct closure *cl) {}
static inline void closure_debug_destroy(struct closure *cl) {}

#endif

static inline void closure_set_ip(struct closure *cl)
{
#ifdef CONFIG_BCACHE_CLOSURES_DEBUG
	cl->ip = _THIS_IP_;
#endif
}

static inline void closure_set_ret_ip(struct closure *cl)
{
#ifdef CONFIG_BCACHE_CLOSURES_DEBUG
	cl->ip = _RET_IP_;
#endif
}

static inline void closure_set_waiting(struct closure *cl, unsigned long f)
{
#ifdef CONFIG_BCACHE_CLOSURES_DEBUG
	cl->waiting_on = f;
#endif
}

static inline void __closure_end_sleep(struct closure *cl)
{
	__set_current_state(TASK_RUNNING);

	if (atomic_read(&cl->remaining) & CLOSURE_SLEEPING)
		atomic_sub(CLOSURE_SLEEPING, &cl->remaining);
}

static inline void __closure_start_sleep(struct closure *cl)
{
	closure_set_ip(cl);
	cl->task = current;
	set_current_state(TASK_UNINTERRUPTIBLE);

	if (!(atomic_read(&cl->remaining) & CLOSURE_SLEEPING))
		atomic_add(CLOSURE_SLEEPING, &cl->remaining);
}

static inline void closure_set_stopped(struct closure *cl)
{
	atomic_sub(CLOSURE_RUNNING, &cl->remaining);
}

static inline void set_closure_fn(struct closure *cl, closure_fn *fn,
				  struct workqueue_struct *wq)
{
	BUG_ON(object_is_on_stack(cl));
	closure_set_ip(cl);
	cl->fn = fn;
	cl->wq = wq;
	/* between atomic_dec() in closure_put() */
	smp_mb__before_atomic();
}

static inline void closure_queue(struct closure *cl)
{
	struct workqueue_struct *wq = cl->wq;
	if (wq) {
		INIT_WORK(&cl->work, cl->work.func);
		BUG_ON(!queue_work(wq, &cl->work));
	} else
		cl->fn(cl);
}

/**
 * closure_get - increment a closure's refcount
 */
static inline void closure_get(struct closure *cl)
{
#ifdef CONFIG_BCACHE_CLOSURES_DEBUG
	BUG_ON((atomic_inc_return(&cl->remaining) &
		CLOSURE_REMAINING_MASK) <= 1);
#else
	atomic_inc(&cl->remaining);
#endif
}

/**
 * closure_init - Initialize a closure, setting the refcount to 1
 * @cl:		closure to initialize
 * @parent:	parent of the new closure. cl will take a refcount on it for its
 *		lifetime; may be NULL.
 */
static inline void closure_init(struct closure *cl, struct closure *parent)
{
	memset(cl, 0, sizeof(struct closure));
	cl->parent = parent;
	if (parent)
		closure_get(parent);

	atomic_set(&cl->remaining, CLOSURE_REMAINING_INITIALIZER);

	closure_debug_create(cl);
	closure_set_ip(cl);
}

static inline void closure_init_stack(struct closure *cl)
{
	memset(cl, 0, sizeof(struct closure));
	atomic_set(&cl->remaining, CLOSURE_REMAINING_INITIALIZER|CLOSURE_STACK);
}

/**
 * closure_wake_up - wake up all closures on a wait list.
 */
static inline void closure_wake_up(struct closure_waitlist *list)
{
	smp_mb();
	__closure_wake_up(list);
}

/**
 * continue_at - jump to another function with barrier
 *
 * After @cl is no longer waiting on anything (i.e. all outstanding refs have
 * been dropped with closure_put()), it will resume execution at @fn running out
 * of @wq (or, if @wq is NULL, @fn will be called by closure_put() directly).
 *
 * NOTE: This macro expands to a return in the calling function!
 *
 * This is because after calling continue_at() you no longer have a ref on @cl,
 * and whatever @cl owns may be freed out from under you - a running closure fn
 * has a ref on its own closure which continue_at() drops.
 */
#define continue_at(_cl, _fn, _wq)					\
do {									\
	set_closure_fn(_cl, _fn, _wq);					\
	closure_sub(_cl, CLOSURE_RUNNING + 1);				\
	return;								\
} while (0)

/**
 * closure_return - finish execution of a closure
 *
 * This is used to indicate that @cl is finished: when all outstanding refs on
 * @cl have been dropped @cl's ref on its parent closure (as passed to
 * closure_init()) will be dropped, if one was specified - thus this can be
 * thought of as returning to the parent closure.
 */
#define closure_return(_cl)	continue_at((_cl), NULL, NULL)

/**
 * continue_at_nobarrier - jump to another function without barrier
 *
 * Causes @fn to be executed out of @cl, in @wq context (or called directly if
 * @wq is NULL).
 *
 * NOTE: like continue_at(), this macro expands to a return in the caller!
 *
 * The ref the caller of continue_at_nobarrier() had on @cl is now owned by @fn,
 * thus it's not safe to touch anything protected by @cl after a
 * continue_at_nobarrier().
 */
#define continue_at_nobarrier(_cl, _fn, _wq)				\
do {									\
	set_closure_fn(_cl, _fn, _wq);					\
	closure_queue(_cl);						\
	return;								\
} while (0)

/**
 * closure_return - finish execution of a closure, with destructor
 *
 * Works like closure_return(), except @destructor will be called when all
 * outstanding refs on @cl have been dropped; @destructor may be used to safely
 * free the memory occupied by @cl, and it is called with the ref on the parent
 * closure still held - so @destructor could safely return an item to a
 * freelist protected by @cl's parent.
 */
#define closure_return_with_destructor(_cl, _destructor)		\
do {									\
	set_closure_fn(_cl, _destructor, NULL);				\
	closure_sub(_cl, CLOSURE_RUNNING - CLOSURE_DESTRUCTOR + 1);	\
	return;								\
} while (0)

/**
 * closure_call - execute @fn out of a new, uninitialized closure
 *
 * Typically used when running out of one closure, and we want to run @fn
 * asynchronously out of a new closure - @parent will then wait for @cl to
 * finish.
 */
static inline void closure_call(struct closure *cl, closure_fn fn,
				struct workqueue_struct *wq,
				struct closure *parent)
{
	closure_init(cl, parent);
	continue_at_nobarrier(cl, fn, wq);
}

#endif /* _LINUX_CLOSURE_H */
