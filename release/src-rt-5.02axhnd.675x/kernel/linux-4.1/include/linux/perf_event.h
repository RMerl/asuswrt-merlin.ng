/*
 * Performance events:
 *
 *    Copyright (C) 2008-2009, Thomas Gleixner <tglx@linutronix.de>
 *    Copyright (C) 2008-2011, Red Hat, Inc., Ingo Molnar
 *    Copyright (C) 2008-2011, Red Hat, Inc., Peter Zijlstra
 *
 * Data type definitions, declarations, prototypes.
 *
 *    Started by: Thomas Gleixner and Ingo Molnar
 *
 * For licencing details see kernel-base/COPYING
 */
#ifndef _LINUX_PERF_EVENT_H
#define _LINUX_PERF_EVENT_H

#include <uapi/linux/perf_event.h>

/*
 * Kernel-internal data types and definitions:
 */

#ifdef CONFIG_PERF_EVENTS
# include <asm/perf_event.h>
# include <asm/local64.h>
#endif

struct perf_guest_info_callbacks {
	int				(*is_in_guest)(void);
	int				(*is_user_mode)(void);
	unsigned long			(*get_guest_ip)(void);
};

#ifdef CONFIG_HAVE_HW_BREAKPOINT
#include <asm/hw_breakpoint.h>
#endif

#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/rculist.h>
#include <linux/rcupdate.h>
#include <linux/spinlock.h>
#include <linux/hrtimer.h>
#include <linux/fs.h>
#include <linux/pid_namespace.h>
#include <linux/workqueue.h>
#include <linux/ftrace.h>
#include <linux/cpu.h>
#include <linux/irq_work.h>
#include <linux/static_key.h>
#include <linux/jump_label_ratelimit.h>
#include <linux/atomic.h>
#include <linux/sysfs.h>
#include <linux/perf_regs.h>
#include <linux/workqueue.h>
#include <linux/cgroup.h>
#include <asm/local.h>

struct perf_callchain_entry {
	__u64				nr;
	__u64				ip[PERF_MAX_STACK_DEPTH];
};

struct perf_raw_record {
	u32				size;
	void				*data;
};

/*
 * branch stack layout:
 *  nr: number of taken branches stored in entries[]
 *
 * Note that nr can vary from sample to sample
 * branches (to, from) are stored from most recent
 * to least recent, i.e., entries[0] contains the most
 * recent branch.
 */
struct perf_branch_stack {
	__u64				nr;
	struct perf_branch_entry	entries[0];
};

struct task_struct;

/*
 * extra PMU register associated with an event
 */
struct hw_perf_event_extra {
	u64		config;	/* register value */
	unsigned int	reg;	/* register address or index */
	int		alloc;	/* extra register already allocated */
	int		idx;	/* index in shared_regs->regs[] */
};

/**
 * struct hw_perf_event - performance event hardware details:
 */
struct hw_perf_event {
#ifdef CONFIG_PERF_EVENTS
	union {
		struct { /* hardware */
			u64		config;
			u64		last_tag;
			unsigned long	config_base;
			unsigned long	event_base;
			int		event_base_rdpmc;
			int		idx;
			int		last_cpu;
			int		flags;

			struct hw_perf_event_extra extra_reg;
			struct hw_perf_event_extra branch_reg;
		};
		struct { /* software */
			struct hrtimer	hrtimer;
		};
		struct { /* tracepoint */
			/* for tp_event->class */
			struct list_head	tp_list;
		};
		struct { /* intel_cqm */
			int			cqm_state;
			int			cqm_rmid;
			struct list_head	cqm_events_entry;
			struct list_head	cqm_groups_entry;
			struct list_head	cqm_group_entry;
		};
		struct { /* itrace */
			int			itrace_started;
		};
#ifdef CONFIG_HAVE_HW_BREAKPOINT
		struct { /* breakpoint */
			/*
			 * Crufty hack to avoid the chicken and egg
			 * problem hw_breakpoint has with context
			 * creation and event initalization.
			 */
			struct arch_hw_breakpoint	info;
			struct list_head		bp_list;
		};
#endif
	};
	struct task_struct		*target;
	int				state;
	local64_t			prev_count;
	u64				sample_period;
	u64				last_period;
	local64_t			period_left;
	u64                             interrupts_seq;
	u64				interrupts;

	u64				freq_time_stamp;
	u64				freq_count_stamp;
#endif
};

/*
 * hw_perf_event::state flags
 */
#define PERF_HES_STOPPED	0x01 /* the counter is stopped */
#define PERF_HES_UPTODATE	0x02 /* event->count up-to-date */
#define PERF_HES_ARCH		0x04

struct perf_event;

/*
 * Common implementation detail of pmu::{start,commit,cancel}_txn
 */
#define PERF_EVENT_TXN 0x1

/**
 * pmu::capabilities flags
 */
#define PERF_PMU_CAP_NO_INTERRUPT		0x01
#define PERF_PMU_CAP_NO_NMI			0x02
#define PERF_PMU_CAP_AUX_NO_SG			0x04
#define PERF_PMU_CAP_AUX_SW_DOUBLEBUF		0x08
#define PERF_PMU_CAP_EXCLUSIVE			0x10
#define PERF_PMU_CAP_ITRACE			0x20

/**
 * struct pmu - generic performance monitoring unit
 */
struct pmu {
	struct list_head		entry;

	struct module			*module;
	struct device			*dev;
	const struct attribute_group	**attr_groups;
	const char			*name;
	int				type;

	/*
	 * various common per-pmu feature flags
	 */
	int				capabilities;

	int * __percpu			pmu_disable_count;
	struct perf_cpu_context * __percpu pmu_cpu_context;
	atomic_t			exclusive_cnt; /* < 0: cpu; > 0: tsk */
	int				task_ctx_nr;
	int				hrtimer_interval_ms;

	/*
	 * Fully disable/enable this PMU, can be used to protect from the PMI
	 * as well as for lazy/batch writing of the MSRs.
	 */
	void (*pmu_enable)		(struct pmu *pmu); /* optional */
	void (*pmu_disable)		(struct pmu *pmu); /* optional */

	/*
	 * Try and initialize the event for this PMU.
	 * Should return -ENOENT when the @event doesn't match this PMU.
	 */
	int (*event_init)		(struct perf_event *event);

	/*
	 * Notification that the event was mapped or unmapped.  Called
	 * in the context of the mapping task.
	 */
	void (*event_mapped)		(struct perf_event *event); /*optional*/
	void (*event_unmapped)		(struct perf_event *event); /*optional*/

#define PERF_EF_START	0x01		/* start the counter when adding    */
#define PERF_EF_RELOAD	0x02		/* reload the counter when starting */
#define PERF_EF_UPDATE	0x04		/* update the counter when stopping */

	/*
	 * Adds/Removes a counter to/from the PMU, can be done inside
	 * a transaction, see the ->*_txn() methods.
	 */
	int  (*add)			(struct perf_event *event, int flags);
	void (*del)			(struct perf_event *event, int flags);

	/*
	 * Starts/Stops a counter present on the PMU. The PMI handler
	 * should stop the counter when perf_event_overflow() returns
	 * !0. ->start() will be used to continue.
	 */
	void (*start)			(struct perf_event *event, int flags);
	void (*stop)			(struct perf_event *event, int flags);

	/*
	 * Updates the counter value of the event.
	 */
	void (*read)			(struct perf_event *event);

	/*
	 * Group events scheduling is treated as a transaction, add
	 * group events as a whole and perform one schedulability test.
	 * If the test fails, roll back the whole group
	 *
	 * Start the transaction, after this ->add() doesn't need to
	 * do schedulability tests.
	 */
	void (*start_txn)		(struct pmu *pmu); /* optional */
	/*
	 * If ->start_txn() disabled the ->add() schedulability test
	 * then ->commit_txn() is required to perform one. On success
	 * the transaction is closed. On error the transaction is kept
	 * open until ->cancel_txn() is called.
	 */
	int  (*commit_txn)		(struct pmu *pmu); /* optional */
	/*
	 * Will cancel the transaction, assumes ->del() is called
	 * for each successful ->add() during the transaction.
	 */
	void (*cancel_txn)		(struct pmu *pmu); /* optional */

	/*
	 * Will return the value for perf_event_mmap_page::index for this event,
	 * if no implementation is provided it will default to: event->hw.idx + 1.
	 */
	int (*event_idx)		(struct perf_event *event); /*optional */

	/*
	 * context-switches callback
	 */
	void (*sched_task)		(struct perf_event_context *ctx,
					bool sched_in);
	/*
	 * PMU specific data size
	 */
	size_t				task_ctx_size;


	/*
	 * Return the count value for a counter.
	 */
	u64 (*count)			(struct perf_event *event); /*optional*/

	/*
	 * Set up pmu-private data structures for an AUX area
	 */
	void *(*setup_aux)		(int cpu, void **pages,
					 int nr_pages, bool overwrite);
					/* optional */

	/*
	 * Free pmu-private AUX data structures
	 */
	void (*free_aux)		(void *aux); /* optional */
};

/**
 * enum perf_event_active_state - the states of a event
 */
enum perf_event_active_state {
	PERF_EVENT_STATE_EXIT		= -3,
	PERF_EVENT_STATE_ERROR		= -2,
	PERF_EVENT_STATE_OFF		= -1,
	PERF_EVENT_STATE_INACTIVE	=  0,
	PERF_EVENT_STATE_ACTIVE		=  1,
};

struct file;
struct perf_sample_data;

typedef void (*perf_overflow_handler_t)(struct perf_event *,
					struct perf_sample_data *,
					struct pt_regs *regs);

enum perf_group_flag {
	PERF_GROUP_SOFTWARE		= 0x1,
};

#define SWEVENT_HLIST_BITS		8
#define SWEVENT_HLIST_SIZE		(1 << SWEVENT_HLIST_BITS)

struct swevent_hlist {
	struct hlist_head		heads[SWEVENT_HLIST_SIZE];
	struct rcu_head			rcu_head;
};

#define PERF_ATTACH_CONTEXT	0x01
#define PERF_ATTACH_GROUP	0x02
#define PERF_ATTACH_TASK	0x04
#define PERF_ATTACH_TASK_DATA	0x08

struct perf_cgroup;
struct ring_buffer;

/**
 * struct perf_event - performance event kernel representation:
 */
struct perf_event {
#ifdef CONFIG_PERF_EVENTS
	/*
	 * entry onto perf_event_context::event_list;
	 *   modifications require ctx->lock
	 *   RCU safe iterations.
	 */
	struct list_head		event_entry;

	/*
	 * XXX: group_entry and sibling_list should be mutually exclusive;
	 * either you're a sibling on a group, or you're the group leader.
	 * Rework the code to always use the same list element.
	 *
	 * Locked for modification by both ctx->mutex and ctx->lock; holding
	 * either sufficies for read.
	 */
	struct list_head		group_entry;
	struct list_head		sibling_list;

	/*
	 * We need storage to track the entries in perf_pmu_migrate_context; we
	 * cannot use the event_entry because of RCU and we want to keep the
	 * group in tact which avoids us using the other two entries.
	 */
	struct list_head		migrate_entry;

	struct hlist_node		hlist_entry;
	struct list_head		active_entry;
	int				nr_siblings;
	int				group_flags;
	struct perf_event		*group_leader;
	struct pmu			*pmu;

	enum perf_event_active_state	state;
	unsigned int			attach_state;
	local64_t			count;
	atomic64_t			child_count;

	/*
	 * These are the total time in nanoseconds that the event
	 * has been enabled (i.e. eligible to run, and the task has
	 * been scheduled in, if this is a per-task event)
	 * and running (scheduled onto the CPU), respectively.
	 *
	 * They are computed from tstamp_enabled, tstamp_running and
	 * tstamp_stopped when the event is in INACTIVE or ACTIVE state.
	 */
	u64				total_time_enabled;
	u64				total_time_running;

	/*
	 * These are timestamps used for computing total_time_enabled
	 * and total_time_running when the event is in INACTIVE or
	 * ACTIVE state, measured in nanoseconds from an arbitrary point
	 * in time.
	 * tstamp_enabled: the notional time when the event was enabled
	 * tstamp_running: the notional time when the event was scheduled on
	 * tstamp_stopped: in INACTIVE state, the notional time when the
	 *	event was scheduled off.
	 */
	u64				tstamp_enabled;
	u64				tstamp_running;
	u64				tstamp_stopped;

	/*
	 * timestamp shadows the actual context timing but it can
	 * be safely used in NMI interrupt context. It reflects the
	 * context time as it was when the event was last scheduled in.
	 *
	 * ctx_time already accounts for ctx->timestamp. Therefore to
	 * compute ctx_time for a sample, simply add perf_clock().
	 */
	u64				shadow_ctx_time;

	struct perf_event_attr		attr;
	u16				header_size;
	u16				id_header_size;
	u16				read_size;
	struct hw_perf_event		hw;

	struct perf_event_context	*ctx;
	atomic_long_t			refcount;

	/*
	 * These accumulate total time (in nanoseconds) that children
	 * events have been enabled and running, respectively.
	 */
	atomic64_t			child_total_time_enabled;
	atomic64_t			child_total_time_running;

	/*
	 * Protect attach/detach and child_list:
	 */
	struct mutex			child_mutex;
	struct list_head		child_list;
	struct perf_event		*parent;

	int				oncpu;
	int				cpu;

	struct list_head		owner_entry;
	struct task_struct		*owner;

	/* mmap bits */
	struct mutex			mmap_mutex;
	atomic_t			mmap_count;

	struct ring_buffer		*rb;
	struct list_head		rb_entry;
	unsigned long			rcu_batches;
	int				rcu_pending;

	/* poll related */
	wait_queue_head_t		waitq;
	struct fasync_struct		*fasync;

	/* delayed work for NMIs and such */
	int				pending_wakeup;
	int				pending_kill;
	int				pending_disable;
	struct irq_work			pending;

	atomic_t			event_limit;

	void (*destroy)(struct perf_event *);
	struct rcu_head			rcu_head;

	struct pid_namespace		*ns;
	u64				id;

	u64				(*clock)(void);
	perf_overflow_handler_t		overflow_handler;
	void				*overflow_handler_context;

#ifdef CONFIG_EVENT_TRACING
	struct ftrace_event_call	*tp_event;
	struct event_filter		*filter;
#ifdef CONFIG_FUNCTION_TRACER
	struct ftrace_ops               ftrace_ops;
#endif
#endif

#ifdef CONFIG_CGROUP_PERF
	struct perf_cgroup		*cgrp; /* cgroup event is attach to */
	int				cgrp_defer_enabled;
#endif

#endif /* CONFIG_PERF_EVENTS */
};

/**
 * struct perf_event_context - event context structure
 *
 * Used as a container for task events and CPU events as well:
 */
struct perf_event_context {
	struct pmu			*pmu;
	/*
	 * Protect the states of the events in the list,
	 * nr_active, and the list:
	 */
	raw_spinlock_t			lock;
	/*
	 * Protect the list of events.  Locking either mutex or lock
	 * is sufficient to ensure the list doesn't change; to change
	 * the list you need to lock both the mutex and the spinlock.
	 */
	struct mutex			mutex;

	struct list_head		active_ctx_list;
	struct list_head		pinned_groups;
	struct list_head		flexible_groups;
	struct list_head		event_list;
	int				nr_events;
	int				nr_active;
	int				is_active;
	int				nr_stat;
	int				nr_freq;
	int				rotate_disable;
	atomic_t			refcount;
	struct task_struct		*task;

	/*
	 * Context clock, runs when context enabled.
	 */
	u64				time;
	u64				timestamp;

	/*
	 * These fields let us detect when two contexts have both
	 * been cloned (inherited) from a common ancestor.
	 */
	struct perf_event_context	*parent_ctx;
	u64				parent_gen;
	u64				generation;
	int				pin_count;
	int				nr_cgroups;	 /* cgroup evts */
	void				*task_ctx_data; /* pmu specific data */
	struct rcu_head			rcu_head;

	struct delayed_work		orphans_remove;
	bool				orphans_remove_sched;
};

/*
 * Number of contexts where an event can trigger:
 *	task, softirq, hardirq, nmi.
 */
#define PERF_NR_CONTEXTS	4

/**
 * struct perf_event_cpu_context - per cpu event context structure
 */
struct perf_cpu_context {
	struct perf_event_context	ctx;
	struct perf_event_context	*task_ctx;
	int				active_oncpu;
	int				exclusive;
	struct hrtimer			hrtimer;
	ktime_t				hrtimer_interval;
	struct pmu			*unique_pmu;
	struct perf_cgroup		*cgrp;
};

struct perf_output_handle {
	struct perf_event		*event;
	struct ring_buffer		*rb;
	unsigned long			wakeup;
	unsigned long			size;
	union {
		void			*addr;
		unsigned long		head;
	};
	int				page;
};

#ifdef CONFIG_CGROUP_PERF

/*
 * perf_cgroup_info keeps track of time_enabled for a cgroup.
 * This is a per-cpu dynamically allocated data structure.
 */
struct perf_cgroup_info {
	u64				time;
	u64				timestamp;
};

struct perf_cgroup {
	struct cgroup_subsys_state	css;
	struct perf_cgroup_info	__percpu *info;
};

/*
 * Must ensure cgroup is pinned (css_get) before calling
 * this function. In other words, we cannot call this function
 * if there is no cgroup event for the current CPU context.
 */
static inline struct perf_cgroup *
perf_cgroup_from_task(struct task_struct *task)
{
	return container_of(task_css(task, perf_event_cgrp_id),
			    struct perf_cgroup, css);
}
#endif /* CONFIG_CGROUP_PERF */

#ifdef CONFIG_PERF_EVENTS

extern void *perf_aux_output_begin(struct perf_output_handle *handle,
				   struct perf_event *event);
extern void perf_aux_output_end(struct perf_output_handle *handle,
				unsigned long size, bool truncated);
extern int perf_aux_output_skip(struct perf_output_handle *handle,
				unsigned long size);
extern void *perf_get_aux(struct perf_output_handle *handle);

extern int perf_pmu_register(struct pmu *pmu, const char *name, int type);
extern void perf_pmu_unregister(struct pmu *pmu);

extern int perf_num_counters(void);
extern const char *perf_pmu_name(void);
extern void __perf_event_task_sched_in(struct task_struct *prev,
				       struct task_struct *task);
extern void __perf_event_task_sched_out(struct task_struct *prev,
					struct task_struct *next);
extern int perf_event_init_task(struct task_struct *child);
extern void perf_event_exit_task(struct task_struct *child);
extern void perf_event_free_task(struct task_struct *task);
extern void perf_event_delayed_put(struct task_struct *task);
extern void perf_event_print_debug(void);
extern void perf_pmu_disable(struct pmu *pmu);
extern void perf_pmu_enable(struct pmu *pmu);
extern void perf_sched_cb_dec(struct pmu *pmu);
extern void perf_sched_cb_inc(struct pmu *pmu);
extern int perf_event_task_disable(void);
extern int perf_event_task_enable(void);
extern int perf_event_refresh(struct perf_event *event, int refresh);
extern void perf_event_update_userpage(struct perf_event *event);
extern int perf_event_release_kernel(struct perf_event *event);
extern struct perf_event *
perf_event_create_kernel_counter(struct perf_event_attr *attr,
				int cpu,
				struct task_struct *task,
				perf_overflow_handler_t callback,
				void *context);
extern void perf_pmu_migrate_context(struct pmu *pmu,
				int src_cpu, int dst_cpu);
extern u64 perf_event_read_value(struct perf_event *event,
				 u64 *enabled, u64 *running);


struct perf_sample_data {
	/*
	 * Fields set by perf_sample_data_init(), group so as to
	 * minimize the cachelines touched.
	 */
	u64				addr;
	struct perf_raw_record		*raw;
	struct perf_branch_stack	*br_stack;
	u64				period;
	u64				weight;
	u64				txn;
	union  perf_mem_data_src	data_src;

	/*
	 * The other fields, optionally {set,used} by
	 * perf_{prepare,output}_sample().
	 */
	u64				type;
	u64				ip;
	struct {
		u32	pid;
		u32	tid;
	}				tid_entry;
	u64				time;
	u64				id;
	u64				stream_id;
	struct {
		u32	cpu;
		u32	reserved;
	}				cpu_entry;
	struct perf_callchain_entry	*callchain;

	/*
	 * regs_user may point to task_pt_regs or to regs_user_copy, depending
	 * on arch details.
	 */
	struct perf_regs		regs_user;
	struct pt_regs			regs_user_copy;

	struct perf_regs		regs_intr;
	u64				stack_user_size;
} ____cacheline_aligned;

/* default value for data source */
#define PERF_MEM_NA (PERF_MEM_S(OP, NA)   |\
		    PERF_MEM_S(LVL, NA)   |\
		    PERF_MEM_S(SNOOP, NA) |\
		    PERF_MEM_S(LOCK, NA)  |\
		    PERF_MEM_S(TLB, NA))

static inline void perf_sample_data_init(struct perf_sample_data *data,
					 u64 addr, u64 period)
{
	/* remaining struct members initialized in perf_prepare_sample() */
	data->addr = addr;
	data->raw  = NULL;
	data->br_stack = NULL;
	data->period = period;
	data->weight = 0;
	data->data_src.val = PERF_MEM_NA;
	data->txn = 0;
}

extern void perf_output_sample(struct perf_output_handle *handle,
			       struct perf_event_header *header,
			       struct perf_sample_data *data,
			       struct perf_event *event);
extern void perf_prepare_sample(struct perf_event_header *header,
				struct perf_sample_data *data,
				struct perf_event *event,
				struct pt_regs *regs);

extern int perf_event_overflow(struct perf_event *event,
				 struct perf_sample_data *data,
				 struct pt_regs *regs);

static inline bool is_sampling_event(struct perf_event *event)
{
	return event->attr.sample_period != 0;
}

/*
 * Return 1 for a software event, 0 for a hardware event
 */
static inline int is_software_event(struct perf_event *event)
{
	return event->pmu->task_ctx_nr == perf_sw_context;
}

extern struct static_key perf_swevent_enabled[PERF_COUNT_SW_MAX];

extern void ___perf_sw_event(u32, u64, struct pt_regs *, u64);
extern void __perf_sw_event(u32, u64, struct pt_regs *, u64);

#ifndef perf_arch_fetch_caller_regs
static inline void perf_arch_fetch_caller_regs(struct pt_regs *regs, unsigned long ip) { }
#endif

/*
 * Take a snapshot of the regs. Skip ip and frame pointer to
 * the nth caller. We only need a few of the regs:
 * - ip for PERF_SAMPLE_IP
 * - cs for user_mode() tests
 * - bp for callchains
 * - eflags, for future purposes, just in case
 */
static inline void perf_fetch_caller_regs(struct pt_regs *regs)
{
	memset(regs, 0, sizeof(*regs));

	perf_arch_fetch_caller_regs(regs, CALLER_ADDR0);
}

static __always_inline void
perf_sw_event(u32 event_id, u64 nr, struct pt_regs *regs, u64 addr)
{
	if (static_key_false(&perf_swevent_enabled[event_id]))
		__perf_sw_event(event_id, nr, regs, addr);
}

DECLARE_PER_CPU(struct pt_regs, __perf_regs[4]);

/*
 * 'Special' version for the scheduler, it hard assumes no recursion,
 * which is guaranteed by us not actually scheduling inside other swevents
 * because those disable preemption.
 */
static __always_inline void
perf_sw_event_sched(u32 event_id, u64 nr, u64 addr)
{
	if (static_key_false(&perf_swevent_enabled[event_id])) {
		struct pt_regs *regs = this_cpu_ptr(&__perf_regs[0]);

		perf_fetch_caller_regs(regs);
		___perf_sw_event(event_id, nr, regs, addr);
	}
}

extern struct static_key_deferred perf_sched_events;

static inline void perf_event_task_sched_in(struct task_struct *prev,
					    struct task_struct *task)
{
	if (static_key_false(&perf_sched_events.key))
		__perf_event_task_sched_in(prev, task);
}

static inline void perf_event_task_sched_out(struct task_struct *prev,
					     struct task_struct *next)
{
	perf_sw_event_sched(PERF_COUNT_SW_CONTEXT_SWITCHES, 1, 0);

	if (static_key_false(&perf_sched_events.key))
		__perf_event_task_sched_out(prev, next);
}

static inline u64 __perf_event_count(struct perf_event *event)
{
	return local64_read(&event->count) + atomic64_read(&event->child_count);
}

extern void perf_event_mmap(struct vm_area_struct *vma);
extern struct perf_guest_info_callbacks *perf_guest_cbs;
extern int perf_register_guest_info_callbacks(struct perf_guest_info_callbacks *callbacks);
extern int perf_unregister_guest_info_callbacks(struct perf_guest_info_callbacks *callbacks);

extern void perf_event_exec(void);
extern void perf_event_comm(struct task_struct *tsk, bool exec);
extern void perf_event_fork(struct task_struct *tsk);

/* Callchains */
DECLARE_PER_CPU(struct perf_callchain_entry, perf_callchain_entry);

extern void perf_callchain_user(struct perf_callchain_entry *entry, struct pt_regs *regs);
extern void perf_callchain_kernel(struct perf_callchain_entry *entry, struct pt_regs *regs);

static inline void perf_callchain_store(struct perf_callchain_entry *entry, u64 ip)
{
	if (entry->nr < PERF_MAX_STACK_DEPTH)
		entry->ip[entry->nr++] = ip;
}

extern int sysctl_perf_event_paranoid;
extern int sysctl_perf_event_mlock;
extern int sysctl_perf_event_sample_rate;
extern int sysctl_perf_cpu_time_max_percent;

extern void perf_sample_event_took(u64 sample_len_ns);

extern int perf_proc_update_handler(struct ctl_table *table, int write,
		void __user *buffer, size_t *lenp,
		loff_t *ppos);
extern int perf_cpu_time_max_percent_handler(struct ctl_table *table, int write,
		void __user *buffer, size_t *lenp,
		loff_t *ppos);


static inline bool perf_paranoid_tracepoint_raw(void)
{
	return sysctl_perf_event_paranoid > -1;
}

static inline bool perf_paranoid_cpu(void)
{
	return sysctl_perf_event_paranoid > 0;
}

static inline bool perf_paranoid_kernel(void)
{
	return sysctl_perf_event_paranoid > 1;
}

extern void perf_event_init(void);
extern void perf_tp_event(u64 addr, u64 count, void *record,
			  int entry_size, struct pt_regs *regs,
			  struct hlist_head *head, int rctx,
			  struct task_struct *task);
extern void perf_bp_event(struct perf_event *event, void *data);

#ifndef perf_misc_flags
# define perf_misc_flags(regs) \
		(user_mode(regs) ? PERF_RECORD_MISC_USER : PERF_RECORD_MISC_KERNEL)
# define perf_instruction_pointer(regs)	instruction_pointer(regs)
#endif

static inline bool has_branch_stack(struct perf_event *event)
{
	return event->attr.sample_type & PERF_SAMPLE_BRANCH_STACK;
}

static inline bool needs_branch_stack(struct perf_event *event)
{
	return event->attr.branch_sample_type != 0;
}

static inline bool has_aux(struct perf_event *event)
{
	return event->pmu->setup_aux;
}

extern int perf_output_begin(struct perf_output_handle *handle,
			     struct perf_event *event, unsigned int size);
extern void perf_output_end(struct perf_output_handle *handle);
extern unsigned int perf_output_copy(struct perf_output_handle *handle,
			     const void *buf, unsigned int len);
extern unsigned int perf_output_skip(struct perf_output_handle *handle,
				     unsigned int len);
extern int perf_swevent_get_recursion_context(void);
extern void perf_swevent_put_recursion_context(int rctx);
extern u64 perf_swevent_set_period(struct perf_event *event);
extern void perf_event_enable(struct perf_event *event);
extern void perf_event_disable(struct perf_event *event);
extern int __perf_event_disable(void *info);
extern void perf_event_task_tick(void);
#else /* !CONFIG_PERF_EVENTS: */
static inline void *
perf_aux_output_begin(struct perf_output_handle *handle,
		      struct perf_event *event)				{ return NULL; }
static inline void
perf_aux_output_end(struct perf_output_handle *handle, unsigned long size,
		    bool truncated)					{ }
static inline int
perf_aux_output_skip(struct perf_output_handle *handle,
		     unsigned long size)				{ return -EINVAL; }
static inline void *
perf_get_aux(struct perf_output_handle *handle)				{ return NULL; }
static inline void
perf_event_task_sched_in(struct task_struct *prev,
			 struct task_struct *task)			{ }
static inline void
perf_event_task_sched_out(struct task_struct *prev,
			  struct task_struct *next)			{ }
static inline int perf_event_init_task(struct task_struct *child)	{ return 0; }
static inline void perf_event_exit_task(struct task_struct *child)	{ }
static inline void perf_event_free_task(struct task_struct *task)	{ }
static inline void perf_event_delayed_put(struct task_struct *task)	{ }
static inline void perf_event_print_debug(void)				{ }
static inline int perf_event_task_disable(void)				{ return -EINVAL; }
static inline int perf_event_task_enable(void)				{ return -EINVAL; }
static inline int perf_event_refresh(struct perf_event *event, int refresh)
{
	return -EINVAL;
}

static inline void
perf_sw_event(u32 event_id, u64 nr, struct pt_regs *regs, u64 addr)	{ }
static inline void
perf_sw_event_sched(u32 event_id, u64 nr, u64 addr)			{ }
static inline void
perf_bp_event(struct perf_event *event, void *data)			{ }

static inline int perf_register_guest_info_callbacks
(struct perf_guest_info_callbacks *callbacks)				{ return 0; }
static inline int perf_unregister_guest_info_callbacks
(struct perf_guest_info_callbacks *callbacks)				{ return 0; }

static inline void perf_event_mmap(struct vm_area_struct *vma)		{ }
static inline void perf_event_exec(void)				{ }
static inline void perf_event_comm(struct task_struct *tsk, bool exec)	{ }
static inline void perf_event_fork(struct task_struct *tsk)		{ }
static inline void perf_event_init(void)				{ }
static inline int  perf_swevent_get_recursion_context(void)		{ return -1; }
static inline void perf_swevent_put_recursion_context(int rctx)		{ }
static inline u64 perf_swevent_set_period(struct perf_event *event)	{ return 0; }
static inline void perf_event_enable(struct perf_event *event)		{ }
static inline void perf_event_disable(struct perf_event *event)		{ }
static inline int __perf_event_disable(void *info)			{ return -1; }
static inline void perf_event_task_tick(void)				{ }
#endif

#if defined(CONFIG_PERF_EVENTS) && defined(CONFIG_NO_HZ_FULL)
extern bool perf_event_can_stop_tick(void);
#else
static inline bool perf_event_can_stop_tick(void)			{ return true; }
#endif

#if defined(CONFIG_PERF_EVENTS) && defined(CONFIG_CPU_SUP_INTEL)
extern void perf_restore_debug_store(void);
#else
static inline void perf_restore_debug_store(void)			{ }
#endif

#define perf_output_put(handle, x) perf_output_copy((handle), &(x), sizeof(x))

/*
 * This has to have a higher priority than migration_notifier in sched/core.c.
 */
#define perf_cpu_notifier(fn)						\
do {									\
	static struct notifier_block fn##_nb =				\
		{ .notifier_call = fn, .priority = CPU_PRI_PERF };	\
	unsigned long cpu = smp_processor_id();				\
	unsigned long flags;						\
									\
	cpu_notifier_register_begin();					\
	fn(&fn##_nb, (unsigned long)CPU_UP_PREPARE,			\
		(void *)(unsigned long)cpu);				\
	local_irq_save(flags);						\
	fn(&fn##_nb, (unsigned long)CPU_STARTING,			\
		(void *)(unsigned long)cpu);				\
	local_irq_restore(flags);					\
	fn(&fn##_nb, (unsigned long)CPU_ONLINE,				\
		(void *)(unsigned long)cpu);				\
	__register_cpu_notifier(&fn##_nb);				\
	cpu_notifier_register_done();					\
} while (0)

/*
 * Bare-bones version of perf_cpu_notifier(), which doesn't invoke the
 * callback for already online CPUs.
 */
#define __perf_cpu_notifier(fn)						\
do {									\
	static struct notifier_block fn##_nb =				\
		{ .notifier_call = fn, .priority = CPU_PRI_PERF };	\
									\
	__register_cpu_notifier(&fn##_nb);				\
} while (0)

struct perf_pmu_events_attr {
	struct device_attribute attr;
	u64 id;
	const char *event_str;
};

ssize_t perf_event_sysfs_show(struct device *dev, struct device_attribute *attr,
			      char *page);

#define PMU_EVENT_ATTR(_name, _var, _id, _show)				\
static struct perf_pmu_events_attr _var = {				\
	.attr = __ATTR(_name, 0444, _show, NULL),			\
	.id   =  _id,							\
};

#define PMU_EVENT_ATTR_STRING(_name, _var, _str)			    \
static struct perf_pmu_events_attr _var = {				    \
	.attr		= __ATTR(_name, 0444, perf_event_sysfs_show, NULL), \
	.id		= 0,						    \
	.event_str	= _str,						    \
};

#define PMU_FORMAT_ATTR(_name, _format)					\
static ssize_t								\
_name##_show(struct device *dev,					\
			       struct device_attribute *attr,		\
			       char *page)				\
{									\
	BUILD_BUG_ON(sizeof(_format) >= PAGE_SIZE);			\
	return sprintf(page, _format "\n");				\
}									\
									\
static struct device_attribute format_attr_##_name = __ATTR_RO(_name)

#endif /* _LINUX_PERF_EVENT_H */
