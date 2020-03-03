/*
 * drivers/base/power/main.c - Where the driver meets power management.
 *
 * Copyright (c) 2003 Patrick Mochel
 * Copyright (c) 2003 Open Source Development Lab
 *
 * This file is released under the GPLv2
 *
 *
 * The driver model core calls device_pm_add() when a device is registered.
 * This will initialize the embedded device_pm_info object in the device
 * and add it to the list of power-controlled devices. sysfs entries for
 * controlling device power management will also be added.
 *
 * A separate list is used for keeping track of power info, because the power
 * domain dependencies may differ from the ancestral dependencies that the
 * subsystem list maintains.
 */

#include <linux/device.h>
#include <linux/kallsyms.h>
#include <linux/export.h>
#include <linux/mutex.h>
#include <linux/pm.h>
#include <linux/pm_runtime.h>
#include <linux/pm-trace.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/async.h>
#include <linux/suspend.h>
#include <trace/events/power.h>
#include <linux/cpufreq.h>
#include <linux/cpuidle.h>
#include <linux/timer.h>

#include "../base.h"
#include "power.h"

typedef int (*pm_callback_t)(struct device *);

/*
 * The entries in the dpm_list list are in a depth first order, simply
 * because children are guaranteed to be discovered after parents, and
 * are inserted at the back of the list on discovery.
 *
 * Since device_pm_add() may be called with a device lock held,
 * we must never try to acquire a device lock while holding
 * dpm_list_mutex.
 */

LIST_HEAD(dpm_list);
static LIST_HEAD(dpm_prepared_list);
static LIST_HEAD(dpm_suspended_list);
static LIST_HEAD(dpm_late_early_list);
static LIST_HEAD(dpm_noirq_list);

struct suspend_stats suspend_stats;
static DEFINE_MUTEX(dpm_list_mtx);
static pm_message_t pm_transition;

static int async_error;

static char *pm_verb(int event)
{
	switch (event) {
	case PM_EVENT_SUSPEND:
		return "suspend";
	case PM_EVENT_RESUME:
		return "resume";
	case PM_EVENT_FREEZE:
		return "freeze";
	case PM_EVENT_QUIESCE:
		return "quiesce";
	case PM_EVENT_HIBERNATE:
		return "hibernate";
	case PM_EVENT_THAW:
		return "thaw";
	case PM_EVENT_RESTORE:
		return "restore";
	case PM_EVENT_RECOVER:
		return "recover";
	default:
		return "(unknown PM event)";
	}
}

/**
 * device_pm_sleep_init - Initialize system suspend-related device fields.
 * @dev: Device object being initialized.
 */
void device_pm_sleep_init(struct device *dev)
{
	dev->power.is_prepared = false;
	dev->power.is_suspended = false;
	dev->power.is_noirq_suspended = false;
	dev->power.is_late_suspended = false;
	init_completion(&dev->power.completion);
	complete_all(&dev->power.completion);
	dev->power.wakeup = NULL;
	INIT_LIST_HEAD(&dev->power.entry);
}

/**
 * device_pm_lock - Lock the list of active devices used by the PM core.
 */
void device_pm_lock(void)
{
	mutex_lock(&dpm_list_mtx);
}

/**
 * device_pm_unlock - Unlock the list of active devices used by the PM core.
 */
void device_pm_unlock(void)
{
	mutex_unlock(&dpm_list_mtx);
}

/**
 * device_pm_add - Add a device to the PM core's list of active devices.
 * @dev: Device to add to the list.
 */
void device_pm_add(struct device *dev)
{
	pr_debug("PM: Adding info for %s:%s\n",
		 dev->bus ? dev->bus->name : "No Bus", dev_name(dev));
	mutex_lock(&dpm_list_mtx);
	if (dev->parent && dev->parent->power.is_prepared)
		dev_warn(dev, "parent %s should not be sleeping\n",
			dev_name(dev->parent));
	list_add_tail(&dev->power.entry, &dpm_list);
	mutex_unlock(&dpm_list_mtx);
}

/**
 * device_pm_remove - Remove a device from the PM core's list of active devices.
 * @dev: Device to be removed from the list.
 */
void device_pm_remove(struct device *dev)
{
	pr_debug("PM: Removing info for %s:%s\n",
		 dev->bus ? dev->bus->name : "No Bus", dev_name(dev));
	complete_all(&dev->power.completion);
	mutex_lock(&dpm_list_mtx);
	list_del_init(&dev->power.entry);
	mutex_unlock(&dpm_list_mtx);
	device_wakeup_disable(dev);
	pm_runtime_remove(dev);
}

/**
 * device_pm_move_before - Move device in the PM core's list of active devices.
 * @deva: Device to move in dpm_list.
 * @devb: Device @deva should come before.
 */
void device_pm_move_before(struct device *deva, struct device *devb)
{
	pr_debug("PM: Moving %s:%s before %s:%s\n",
		 deva->bus ? deva->bus->name : "No Bus", dev_name(deva),
		 devb->bus ? devb->bus->name : "No Bus", dev_name(devb));
	/* Delete deva from dpm_list and reinsert before devb. */
	list_move_tail(&deva->power.entry, &devb->power.entry);
}

/**
 * device_pm_move_after - Move device in the PM core's list of active devices.
 * @deva: Device to move in dpm_list.
 * @devb: Device @deva should come after.
 */
void device_pm_move_after(struct device *deva, struct device *devb)
{
	pr_debug("PM: Moving %s:%s after %s:%s\n",
		 deva->bus ? deva->bus->name : "No Bus", dev_name(deva),
		 devb->bus ? devb->bus->name : "No Bus", dev_name(devb));
	/* Delete deva from dpm_list and reinsert after devb. */
	list_move(&deva->power.entry, &devb->power.entry);
}

/**
 * device_pm_move_last - Move device to end of the PM core's list of devices.
 * @dev: Device to move in dpm_list.
 */
void device_pm_move_last(struct device *dev)
{
	pr_debug("PM: Moving %s:%s to end of list\n",
		 dev->bus ? dev->bus->name : "No Bus", dev_name(dev));
	list_move_tail(&dev->power.entry, &dpm_list);
}

static ktime_t initcall_debug_start(struct device *dev)
{
	ktime_t calltime = ktime_set(0, 0);

	if (pm_print_times_enabled) {
		pr_info("calling  %s+ @ %i, parent: %s\n",
			dev_name(dev), task_pid_nr(current),
			dev->parent ? dev_name(dev->parent) : "none");
		calltime = ktime_get();
	}

	return calltime;
}

static void initcall_debug_report(struct device *dev, ktime_t calltime,
				  int error, pm_message_t state, char *info)
{
	ktime_t rettime;
	s64 nsecs;

	rettime = ktime_get();
	nsecs = (s64) ktime_to_ns(ktime_sub(rettime, calltime));

	if (pm_print_times_enabled) {
		pr_info("call %s+ returned %d after %Ld usecs\n", dev_name(dev),
			error, (unsigned long long)nsecs >> 10);
	}
}

/**
 * dpm_wait - Wait for a PM operation to complete.
 * @dev: Device to wait for.
 * @async: If unset, wait only if the device's power.async_suspend flag is set.
 */
static void dpm_wait(struct device *dev, bool async)
{
	if (!dev)
		return;

	if (async || (pm_async_enabled && dev->power.async_suspend))
		wait_for_completion(&dev->power.completion);
}

static int dpm_wait_fn(struct device *dev, void *async_ptr)
{
	dpm_wait(dev, *((bool *)async_ptr));
	return 0;
}

static void dpm_wait_for_children(struct device *dev, bool async)
{
       device_for_each_child(dev, &async, dpm_wait_fn);
}

/**
 * pm_op - Return the PM operation appropriate for given PM event.
 * @ops: PM operations to choose from.
 * @state: PM transition of the system being carried out.
 */
static pm_callback_t pm_op(const struct dev_pm_ops *ops, pm_message_t state)
{
	switch (state.event) {
#ifdef CONFIG_SUSPEND
	case PM_EVENT_SUSPEND:
		return ops->suspend;
	case PM_EVENT_RESUME:
		return ops->resume;
#endif /* CONFIG_SUSPEND */
#ifdef CONFIG_HIBERNATE_CALLBACKS
	case PM_EVENT_FREEZE:
	case PM_EVENT_QUIESCE:
		return ops->freeze;
	case PM_EVENT_HIBERNATE:
		return ops->poweroff;
	case PM_EVENT_THAW:
	case PM_EVENT_RECOVER:
		return ops->thaw;
		break;
	case PM_EVENT_RESTORE:
		return ops->restore;
#endif /* CONFIG_HIBERNATE_CALLBACKS */
	}

	return NULL;
}

/**
 * pm_late_early_op - Return the PM operation appropriate for given PM event.
 * @ops: PM operations to choose from.
 * @state: PM transition of the system being carried out.
 *
 * Runtime PM is disabled for @dev while this function is being executed.
 */
static pm_callback_t pm_late_early_op(const struct dev_pm_ops *ops,
				      pm_message_t state)
{
	switch (state.event) {
#ifdef CONFIG_SUSPEND
	case PM_EVENT_SUSPEND:
		return ops->suspend_late;
	case PM_EVENT_RESUME:
		return ops->resume_early;
#endif /* CONFIG_SUSPEND */
#ifdef CONFIG_HIBERNATE_CALLBACKS
	case PM_EVENT_FREEZE:
	case PM_EVENT_QUIESCE:
		return ops->freeze_late;
	case PM_EVENT_HIBERNATE:
		return ops->poweroff_late;
	case PM_EVENT_THAW:
	case PM_EVENT_RECOVER:
		return ops->thaw_early;
	case PM_EVENT_RESTORE:
		return ops->restore_early;
#endif /* CONFIG_HIBERNATE_CALLBACKS */
	}

	return NULL;
}

/**
 * pm_noirq_op - Return the PM operation appropriate for given PM event.
 * @ops: PM operations to choose from.
 * @state: PM transition of the system being carried out.
 *
 * The driver of @dev will not receive interrupts while this function is being
 * executed.
 */
static pm_callback_t pm_noirq_op(const struct dev_pm_ops *ops, pm_message_t state)
{
	switch (state.event) {
#ifdef CONFIG_SUSPEND
	case PM_EVENT_SUSPEND:
		return ops->suspend_noirq;
	case PM_EVENT_RESUME:
		return ops->resume_noirq;
#endif /* CONFIG_SUSPEND */
#ifdef CONFIG_HIBERNATE_CALLBACKS
	case PM_EVENT_FREEZE:
	case PM_EVENT_QUIESCE:
		return ops->freeze_noirq;
	case PM_EVENT_HIBERNATE:
		return ops->poweroff_noirq;
	case PM_EVENT_THAW:
	case PM_EVENT_RECOVER:
		return ops->thaw_noirq;
	case PM_EVENT_RESTORE:
		return ops->restore_noirq;
#endif /* CONFIG_HIBERNATE_CALLBACKS */
	}

	return NULL;
}

static void pm_dev_dbg(struct device *dev, pm_message_t state, char *info)
{
	dev_dbg(dev, "%s%s%s\n", info, pm_verb(state.event),
		((state.event & PM_EVENT_SLEEP) && device_may_wakeup(dev)) ?
		", may wakeup" : "");
}

static void pm_dev_err(struct device *dev, pm_message_t state, char *info,
			int error)
{
	printk(KERN_ERR "PM: Device %s failed to %s%s: error %d\n",
		dev_name(dev), pm_verb(state.event), info, error);
}

static void dpm_show_time(ktime_t starttime, pm_message_t state, char *info)
{
	ktime_t calltime;
	u64 usecs64;
	int usecs;

	calltime = ktime_get();
	usecs64 = ktime_to_ns(ktime_sub(calltime, starttime));
	do_div(usecs64, NSEC_PER_USEC);
	usecs = usecs64;
	if (usecs == 0)
		usecs = 1;
	pr_info("PM: %s%s%s of devices complete after %ld.%03ld msecs\n",
		info ?: "", info ? " " : "", pm_verb(state.event),
		usecs / USEC_PER_MSEC, usecs % USEC_PER_MSEC);
}

static int dpm_run_callback(pm_callback_t cb, struct device *dev,
			    pm_message_t state, char *info)
{
	ktime_t calltime;
	int error;

	if (!cb)
		return 0;

	calltime = initcall_debug_start(dev);

	pm_dev_dbg(dev, state, info);
	trace_device_pm_callback_start(dev, info, state.event);
	error = cb(dev);
	trace_device_pm_callback_end(dev, error);
	suspend_report_result(cb, error);

	initcall_debug_report(dev, calltime, error, state, info);

	return error;
}

#ifdef CONFIG_DPM_WATCHDOG
struct dpm_watchdog {
	struct device		*dev;
	struct task_struct	*tsk;
	struct timer_list	timer;
};

#define DECLARE_DPM_WATCHDOG_ON_STACK(wd) \
	struct dpm_watchdog wd

/**
 * dpm_watchdog_handler - Driver suspend / resume watchdog handler.
 * @data: Watchdog object address.
 *
 * Called when a driver has timed out suspending or resuming.
 * There's not much we can do here to recover so panic() to
 * capture a crash-dump in pstore.
 */
static void dpm_watchdog_handler(unsigned long data)
{
	struct dpm_watchdog *wd = (void *)data;

	dev_emerg(wd->dev, "**** DPM device timeout ****\n");
	show_stack(wd->tsk, NULL);
	panic("%s %s: unrecoverable failure\n",
		dev_driver_string(wd->dev), dev_name(wd->dev));
}

/**
 * dpm_watchdog_set - Enable pm watchdog for given device.
 * @wd: Watchdog. Must be allocated on the stack.
 * @dev: Device to handle.
 */
static void dpm_watchdog_set(struct dpm_watchdog *wd, struct device *dev)
{
	struct timer_list *timer = &wd->timer;

	wd->dev = dev;
	wd->tsk = current;

	init_timer_on_stack(timer);
	/* use same timeout value for both suspend and resume */
	timer->expires = jiffies + HZ * CONFIG_DPM_WATCHDOG_TIMEOUT;
	timer->function = dpm_watchdog_handler;
	timer->data = (unsigned long)wd;
	add_timer(timer);
}

/**
 * dpm_watchdog_clear - Disable suspend/resume watchdog.
 * @wd: Watchdog to disable.
 */
static void dpm_watchdog_clear(struct dpm_watchdog *wd)
{
	struct timer_list *timer = &wd->timer;

	del_timer_sync(timer);
	destroy_timer_on_stack(timer);
}
#else
#define DECLARE_DPM_WATCHDOG_ON_STACK(wd)
#define dpm_watchdog_set(x, y)
#define dpm_watchdog_clear(x)
#endif

/*------------------------- Resume routines -------------------------*/

/**
 * device_resume_noirq - Execute an "early resume" callback for given device.
 * @dev: Device to handle.
 * @state: PM transition of the system being carried out.
 * @async: If true, the device is being resumed asynchronously.
 *
 * The driver of @dev will not receive interrupts while this function is being
 * executed.
 */
static int device_resume_noirq(struct device *dev, pm_message_t state, bool async)
{
	pm_callback_t callback = NULL;
	char *info = NULL;
	int error = 0;

	TRACE_DEVICE(dev);
	TRACE_RESUME(0);

	if (dev->power.syscore || dev->power.direct_complete)
		goto Out;

	if (!dev->power.is_noirq_suspended)
		goto Out;

	dpm_wait(dev->parent, async);

	if (dev->pm_domain) {
		info = "noirq power domain ";
		callback = pm_noirq_op(&dev->pm_domain->ops, state);
	} else if (dev->type && dev->type->pm) {
		info = "noirq type ";
		callback = pm_noirq_op(dev->type->pm, state);
	} else if (dev->class && dev->class->pm) {
		info = "noirq class ";
		callback = pm_noirq_op(dev->class->pm, state);
	} else if (dev->bus && dev->bus->pm) {
		info = "noirq bus ";
		callback = pm_noirq_op(dev->bus->pm, state);
	}

	if (!callback && dev->driver && dev->driver->pm) {
		info = "noirq driver ";
		callback = pm_noirq_op(dev->driver->pm, state);
	}

	error = dpm_run_callback(callback, dev, state, info);
	dev->power.is_noirq_suspended = false;

 Out:
	complete_all(&dev->power.completion);
	TRACE_RESUME(error);
	return error;
}

static bool is_async(struct device *dev)
{
	return dev->power.async_suspend && pm_async_enabled
		&& !pm_trace_is_enabled();
}

static void async_resume_noirq(void *data, async_cookie_t cookie)
{
	struct device *dev = (struct device *)data;
	int error;

	error = device_resume_noirq(dev, pm_transition, true);
	if (error)
		pm_dev_err(dev, pm_transition, " async", error);

	put_device(dev);
}

/**
 * dpm_resume_noirq - Execute "noirq resume" callbacks for all devices.
 * @state: PM transition of the system being carried out.
 *
 * Call the "noirq" resume handlers for all devices in dpm_noirq_list and
 * enable device drivers to receive interrupts.
 */
void dpm_resume_noirq(pm_message_t state)
{
	struct device *dev;
	ktime_t starttime = ktime_get();

	trace_suspend_resume(TPS("dpm_resume_noirq"), state.event, true);
	mutex_lock(&dpm_list_mtx);
	pm_transition = state;

	/*
	 * Advanced the async threads upfront,
	 * in case the starting of async threads is
	 * delayed by non-async resuming devices.
	 */
	list_for_each_entry(dev, &dpm_noirq_list, power.entry) {
		reinit_completion(&dev->power.completion);
		if (is_async(dev)) {
			get_device(dev);
			async_schedule(async_resume_noirq, dev);
		}
	}

	while (!list_empty(&dpm_noirq_list)) {
		dev = to_device(dpm_noirq_list.next);
		get_device(dev);
		list_move_tail(&dev->power.entry, &dpm_late_early_list);
		mutex_unlock(&dpm_list_mtx);

		if (!is_async(dev)) {
			int error;

			error = device_resume_noirq(dev, state, false);
			if (error) {
				suspend_stats.failed_resume_noirq++;
				dpm_save_failed_step(SUSPEND_RESUME_NOIRQ);
				dpm_save_failed_dev(dev_name(dev));
				pm_dev_err(dev, state, " noirq", error);
			}
		}

		mutex_lock(&dpm_list_mtx);
		put_device(dev);
	}
	mutex_unlock(&dpm_list_mtx);
	async_synchronize_full();
	dpm_show_time(starttime, state, "noirq");
	resume_device_irqs();
	cpuidle_resume();
	trace_suspend_resume(TPS("dpm_resume_noirq"), state.event, false);
}

/**
 * device_resume_early - Execute an "early resume" callback for given device.
 * @dev: Device to handle.
 * @state: PM transition of the system being carried out.
 * @async: If true, the device is being resumed asynchronously.
 *
 * Runtime PM is disabled for @dev while this function is being executed.
 */
static int device_resume_early(struct device *dev, pm_message_t state, bool async)
{
	pm_callback_t callback = NULL;
	char *info = NULL;
	int error = 0;

	TRACE_DEVICE(dev);
	TRACE_RESUME(0);

	if (dev->power.syscore || dev->power.direct_complete)
		goto Out;

	if (!dev->power.is_late_suspended)
		goto Out;

	dpm_wait(dev->parent, async);

	if (dev->pm_domain) {
		info = "early power domain ";
		callback = pm_late_early_op(&dev->pm_domain->ops, state);
	} else if (dev->type && dev->type->pm) {
		info = "early type ";
		callback = pm_late_early_op(dev->type->pm, state);
	} else if (dev->class && dev->class->pm) {
		info = "early class ";
		callback = pm_late_early_op(dev->class->pm, state);
	} else if (dev->bus && dev->bus->pm) {
		info = "early bus ";
		callback = pm_late_early_op(dev->bus->pm, state);
	}

	if (!callback && dev->driver && dev->driver->pm) {
		info = "early driver ";
		callback = pm_late_early_op(dev->driver->pm, state);
	}

	error = dpm_run_callback(callback, dev, state, info);
	dev->power.is_late_suspended = false;

 Out:
	TRACE_RESUME(error);

	pm_runtime_enable(dev);
	complete_all(&dev->power.completion);
	return error;
}

static void async_resume_early(void *data, async_cookie_t cookie)
{
	struct device *dev = (struct device *)data;
	int error;

	error = device_resume_early(dev, pm_transition, true);
	if (error)
		pm_dev_err(dev, pm_transition, " async", error);

	put_device(dev);
}

/**
 * dpm_resume_early - Execute "early resume" callbacks for all devices.
 * @state: PM transition of the system being carried out.
 */
void dpm_resume_early(pm_message_t state)
{
	struct device *dev;
	ktime_t starttime = ktime_get();

	trace_suspend_resume(TPS("dpm_resume_early"), state.event, true);
	mutex_lock(&dpm_list_mtx);
	pm_transition = state;

	/*
	 * Advanced the async threads upfront,
	 * in case the starting of async threads is
	 * delayed by non-async resuming devices.
	 */
	list_for_each_entry(dev, &dpm_late_early_list, power.entry) {
		reinit_completion(&dev->power.completion);
		if (is_async(dev)) {
			get_device(dev);
			async_schedule(async_resume_early, dev);
		}
	}

	while (!list_empty(&dpm_late_early_list)) {
		dev = to_device(dpm_late_early_list.next);
		get_device(dev);
		list_move_tail(&dev->power.entry, &dpm_suspended_list);
		mutex_unlock(&dpm_list_mtx);

		if (!is_async(dev)) {
			int error;

			error = device_resume_early(dev, state, false);
			if (error) {
				suspend_stats.failed_resume_early++;
				dpm_save_failed_step(SUSPEND_RESUME_EARLY);
				dpm_save_failed_dev(dev_name(dev));
				pm_dev_err(dev, state, " early", error);
			}
		}
		mutex_lock(&dpm_list_mtx);
		put_device(dev);
	}
	mutex_unlock(&dpm_list_mtx);
	async_synchronize_full();
	dpm_show_time(starttime, state, "early");
	trace_suspend_resume(TPS("dpm_resume_early"), state.event, false);
}

/**
 * dpm_resume_start - Execute "noirq" and "early" device callbacks.
 * @state: PM transition of the system being carried out.
 */
void dpm_resume_start(pm_message_t state)
{
	dpm_resume_noirq(state);
	dpm_resume_early(state);
}
EXPORT_SYMBOL_GPL(dpm_resume_start);

/**
 * device_resume - Execute "resume" callbacks for given device.
 * @dev: Device to handle.
 * @state: PM transition of the system being carried out.
 * @async: If true, the device is being resumed asynchronously.
 */
static int device_resume(struct device *dev, pm_message_t state, bool async)
{
	pm_callback_t callback = NULL;
	char *info = NULL;
	int error = 0;
	DECLARE_DPM_WATCHDOG_ON_STACK(wd);

	TRACE_DEVICE(dev);
	TRACE_RESUME(0);

	if (dev->power.syscore)
		goto Complete;

	if (dev->power.direct_complete) {
		/* Match the pm_runtime_disable() in __device_suspend(). */
		pm_runtime_enable(dev);
		goto Complete;
	}

	dpm_wait(dev->parent, async);
	dpm_watchdog_set(&wd, dev);
	device_lock(dev);

	/*
	 * This is a fib.  But we'll allow new children to be added below
	 * a resumed device, even if the device hasn't been completed yet.
	 */
	dev->power.is_prepared = false;

	if (!dev->power.is_suspended)
		goto Unlock;

	if (dev->pm_domain) {
		info = "power domain ";
		callback = pm_op(&dev->pm_domain->ops, state);
		goto Driver;
	}

	if (dev->type && dev->type->pm) {
		info = "type ";
		callback = pm_op(dev->type->pm, state);
		goto Driver;
	}

	if (dev->class) {
		if (dev->class->pm) {
			info = "class ";
			callback = pm_op(dev->class->pm, state);
			goto Driver;
		} else if (dev->class->resume) {
			info = "legacy class ";
			callback = dev->class->resume;
			goto End;
		}
	}

	if (dev->bus) {
		if (dev->bus->pm) {
			info = "bus ";
			callback = pm_op(dev->bus->pm, state);
		} else if (dev->bus->resume) {
			info = "legacy bus ";
			callback = dev->bus->resume;
			goto End;
		}
	}

 Driver:
	if (!callback && dev->driver && dev->driver->pm) {
		info = "driver ";
		callback = pm_op(dev->driver->pm, state);
	}

 End:
	error = dpm_run_callback(callback, dev, state, info);
	dev->power.is_suspended = false;

 Unlock:
	device_unlock(dev);
	dpm_watchdog_clear(&wd);

 Complete:
	complete_all(&dev->power.completion);

	TRACE_RESUME(error);

	return error;
}

static void async_resume(void *data, async_cookie_t cookie)
{
	struct device *dev = (struct device *)data;
	int error;

	error = device_resume(dev, pm_transition, true);
	if (error)
		pm_dev_err(dev, pm_transition, " async", error);
	put_device(dev);
}

/**
 * dpm_resume - Execute "resume" callbacks for non-sysdev devices.
 * @state: PM transition of the system being carried out.
 *
 * Execute the appropriate "resume" callback for all devices whose status
 * indicates that they are suspended.
 */
void dpm_resume(pm_message_t state)
{
	struct device *dev;
	ktime_t starttime = ktime_get();

	trace_suspend_resume(TPS("dpm_resume"), state.event, true);
	might_sleep();

	mutex_lock(&dpm_list_mtx);
	pm_transition = state;
	async_error = 0;

	list_for_each_entry(dev, &dpm_suspended_list, power.entry) {
		reinit_completion(&dev->power.completion);
		if (is_async(dev)) {
			get_device(dev);
			async_schedule(async_resume, dev);
		}
	}

	while (!list_empty(&dpm_suspended_list)) {
		dev = to_device(dpm_suspended_list.next);
		get_device(dev);
		if (!is_async(dev)) {
			int error;

			mutex_unlock(&dpm_list_mtx);

			error = device_resume(dev, state, false);
			if (error) {
				suspend_stats.failed_resume++;
				dpm_save_failed_step(SUSPEND_RESUME);
				dpm_save_failed_dev(dev_name(dev));
				pm_dev_err(dev, state, "", error);
			}

			mutex_lock(&dpm_list_mtx);
		}
		if (!list_empty(&dev->power.entry))
			list_move_tail(&dev->power.entry, &dpm_prepared_list);
		put_device(dev);
	}
	mutex_unlock(&dpm_list_mtx);
	async_synchronize_full();
	dpm_show_time(starttime, state, NULL);

	cpufreq_resume();
	trace_suspend_resume(TPS("dpm_resume"), state.event, false);
}

/**
 * device_complete - Complete a PM transition for given device.
 * @dev: Device to handle.
 * @state: PM transition of the system being carried out.
 */
static void device_complete(struct device *dev, pm_message_t state)
{
	void (*callback)(struct device *) = NULL;
	char *info = NULL;

	if (dev->power.syscore)
		return;

	device_lock(dev);

	if (dev->pm_domain) {
		info = "completing power domain ";
		callback = dev->pm_domain->ops.complete;
	} else if (dev->type && dev->type->pm) {
		info = "completing type ";
		callback = dev->type->pm->complete;
	} else if (dev->class && dev->class->pm) {
		info = "completing class ";
		callback = dev->class->pm->complete;
	} else if (dev->bus && dev->bus->pm) {
		info = "completing bus ";
		callback = dev->bus->pm->complete;
	}

	if (!callback && dev->driver && dev->driver->pm) {
		info = "completing driver ";
		callback = dev->driver->pm->complete;
	}

	if (callback) {
		pm_dev_dbg(dev, state, info);
		trace_device_pm_callback_start(dev, info, state.event);
		callback(dev);
		trace_device_pm_callback_end(dev, 0);
	}

	device_unlock(dev);

	pm_runtime_put(dev);
}

/**
 * dpm_complete - Complete a PM transition for all non-sysdev devices.
 * @state: PM transition of the system being carried out.
 *
 * Execute the ->complete() callbacks for all devices whose PM status is not
 * DPM_ON (this allows new devices to be registered).
 */
void dpm_complete(pm_message_t state)
{
	struct list_head list;

	trace_suspend_resume(TPS("dpm_complete"), state.event, true);
	might_sleep();

	INIT_LIST_HEAD(&list);
	mutex_lock(&dpm_list_mtx);
	while (!list_empty(&dpm_prepared_list)) {
		struct device *dev = to_device(dpm_prepared_list.prev);

		get_device(dev);
		dev->power.is_prepared = false;
		list_move(&dev->power.entry, &list);
		mutex_unlock(&dpm_list_mtx);

		device_complete(dev, state);

		mutex_lock(&dpm_list_mtx);
		put_device(dev);
	}
	list_splice(&list, &dpm_list);
	mutex_unlock(&dpm_list_mtx);
	trace_suspend_resume(TPS("dpm_complete"), state.event, false);
}

/**
 * dpm_resume_end - Execute "resume" callbacks and complete system transition.
 * @state: PM transition of the system being carried out.
 *
 * Execute "resume" callbacks for all devices and complete the PM transition of
 * the system.
 */
void dpm_resume_end(pm_message_t state)
{
	dpm_resume(state);
	dpm_complete(state);
}
EXPORT_SYMBOL_GPL(dpm_resume_end);


/*------------------------- Suspend routines -------------------------*/

/**
 * resume_event - Return a "resume" message for given "suspend" sleep state.
 * @sleep_state: PM message representing a sleep state.
 *
 * Return a PM message representing the resume event corresponding to given
 * sleep state.
 */
static pm_message_t resume_event(pm_message_t sleep_state)
{
	switch (sleep_state.event) {
	case PM_EVENT_SUSPEND:
		return PMSG_RESUME;
	case PM_EVENT_FREEZE:
	case PM_EVENT_QUIESCE:
		return PMSG_RECOVER;
	case PM_EVENT_HIBERNATE:
		return PMSG_RESTORE;
	}
	return PMSG_ON;
}

/**
 * device_suspend_noirq - Execute a "late suspend" callback for given device.
 * @dev: Device to handle.
 * @state: PM transition of the system being carried out.
 * @async: If true, the device is being suspended asynchronously.
 *
 * The driver of @dev will not receive interrupts while this function is being
 * executed.
 */
static int __device_suspend_noirq(struct device *dev, pm_message_t state, bool async)
{
	pm_callback_t callback = NULL;
	char *info = NULL;
	int error = 0;

	TRACE_DEVICE(dev);
	TRACE_SUSPEND(0);

	if (async_error)
		goto Complete;

	if (pm_wakeup_pending()) {
		async_error = -EBUSY;
		goto Complete;
	}

	if (dev->power.syscore || dev->power.direct_complete)
		goto Complete;

	dpm_wait_for_children(dev, async);

	if (dev->pm_domain) {
		info = "noirq power domain ";
		callback = pm_noirq_op(&dev->pm_domain->ops, state);
	} else if (dev->type && dev->type->pm) {
		info = "noirq type ";
		callback = pm_noirq_op(dev->type->pm, state);
	} else if (dev->class && dev->class->pm) {
		info = "noirq class ";
		callback = pm_noirq_op(dev->class->pm, state);
	} else if (dev->bus && dev->bus->pm) {
		info = "noirq bus ";
		callback = pm_noirq_op(dev->bus->pm, state);
	}

	if (!callback && dev->driver && dev->driver->pm) {
		info = "noirq driver ";
		callback = pm_noirq_op(dev->driver->pm, state);
	}

	error = dpm_run_callback(callback, dev, state, info);
	if (!error)
		dev->power.is_noirq_suspended = true;
	else
		async_error = error;

Complete:
	complete_all(&dev->power.completion);
	TRACE_SUSPEND(error);
	return error;
}

static void async_suspend_noirq(void *data, async_cookie_t cookie)
{
	struct device *dev = (struct device *)data;
	int error;

	error = __device_suspend_noirq(dev, pm_transition, true);
	if (error) {
		dpm_save_failed_dev(dev_name(dev));
		pm_dev_err(dev, pm_transition, " async", error);
	}

	put_device(dev);
}

static int device_suspend_noirq(struct device *dev)
{
	reinit_completion(&dev->power.completion);

	if (is_async(dev)) {
		get_device(dev);
		async_schedule(async_suspend_noirq, dev);
		return 0;
	}
	return __device_suspend_noirq(dev, pm_transition, false);
}

/**
 * dpm_suspend_noirq - Execute "noirq suspend" callbacks for all devices.
 * @state: PM transition of the system being carried out.
 *
 * Prevent device drivers from receiving interrupts and call the "noirq" suspend
 * handlers for all non-sysdev devices.
 */
int dpm_suspend_noirq(pm_message_t state)
{
	ktime_t starttime = ktime_get();
	int error = 0;

	trace_suspend_resume(TPS("dpm_suspend_noirq"), state.event, true);
	cpuidle_pause();
	suspend_device_irqs();
	mutex_lock(&dpm_list_mtx);
	pm_transition = state;
	async_error = 0;

	while (!list_empty(&dpm_late_early_list)) {
		struct device *dev = to_device(dpm_late_early_list.prev);

		get_device(dev);
		mutex_unlock(&dpm_list_mtx);

		error = device_suspend_noirq(dev);

		mutex_lock(&dpm_list_mtx);
		if (error) {
			pm_dev_err(dev, state, " noirq", error);
			dpm_save_failed_dev(dev_name(dev));
			put_device(dev);
			break;
		}
		if (!list_empty(&dev->power.entry))
			list_move(&dev->power.entry, &dpm_noirq_list);
		put_device(dev);

		if (async_error)
			break;
	}
	mutex_unlock(&dpm_list_mtx);
	async_synchronize_full();
	if (!error)
		error = async_error;

	if (error) {
		suspend_stats.failed_suspend_noirq++;
		dpm_save_failed_step(SUSPEND_SUSPEND_NOIRQ);
		dpm_resume_noirq(resume_event(state));
	} else {
		dpm_show_time(starttime, state, "noirq");
	}
	trace_suspend_resume(TPS("dpm_suspend_noirq"), state.event, false);
	return error;
}

/**
 * device_suspend_late - Execute a "late suspend" callback for given device.
 * @dev: Device to handle.
 * @state: PM transition of the system being carried out.
 * @async: If true, the device is being suspended asynchronously.
 *
 * Runtime PM is disabled for @dev while this function is being executed.
 */
static int __device_suspend_late(struct device *dev, pm_message_t state, bool async)
{
	pm_callback_t callback = NULL;
	char *info = NULL;
	int error = 0;

	TRACE_DEVICE(dev);
	TRACE_SUSPEND(0);

	__pm_runtime_disable(dev, false);

	if (async_error)
		goto Complete;

	if (pm_wakeup_pending()) {
		async_error = -EBUSY;
		goto Complete;
	}

	if (dev->power.syscore || dev->power.direct_complete)
		goto Complete;

	dpm_wait_for_children(dev, async);

	if (dev->pm_domain) {
		info = "late power domain ";
		callback = pm_late_early_op(&dev->pm_domain->ops, state);
	} else if (dev->type && dev->type->pm) {
		info = "late type ";
		callback = pm_late_early_op(dev->type->pm, state);
	} else if (dev->class && dev->class->pm) {
		info = "late class ";
		callback = pm_late_early_op(dev->class->pm, state);
	} else if (dev->bus && dev->bus->pm) {
		info = "late bus ";
		callback = pm_late_early_op(dev->bus->pm, state);
	}

	if (!callback && dev->driver && dev->driver->pm) {
		info = "late driver ";
		callback = pm_late_early_op(dev->driver->pm, state);
	}

	error = dpm_run_callback(callback, dev, state, info);
	if (!error)
		dev->power.is_late_suspended = true;
	else
		async_error = error;

Complete:
	TRACE_SUSPEND(error);
	complete_all(&dev->power.completion);
	return error;
}

static void async_suspend_late(void *data, async_cookie_t cookie)
{
	struct device *dev = (struct device *)data;
	int error;

	error = __device_suspend_late(dev, pm_transition, true);
	if (error) {
		dpm_save_failed_dev(dev_name(dev));
		pm_dev_err(dev, pm_transition, " async", error);
	}
	put_device(dev);
}

static int device_suspend_late(struct device *dev)
{
	reinit_completion(&dev->power.completion);

	if (is_async(dev)) {
		get_device(dev);
		async_schedule(async_suspend_late, dev);
		return 0;
	}

	return __device_suspend_late(dev, pm_transition, false);
}

/**
 * dpm_suspend_late - Execute "late suspend" callbacks for all devices.
 * @state: PM transition of the system being carried out.
 */
int dpm_suspend_late(pm_message_t state)
{
	ktime_t starttime = ktime_get();
	int error = 0;

	trace_suspend_resume(TPS("dpm_suspend_late"), state.event, true);
	mutex_lock(&dpm_list_mtx);
	pm_transition = state;
	async_error = 0;

	while (!list_empty(&dpm_suspended_list)) {
		struct device *dev = to_device(dpm_suspended_list.prev);

		get_device(dev);
		mutex_unlock(&dpm_list_mtx);

		error = device_suspend_late(dev);

		mutex_lock(&dpm_list_mtx);
		if (!list_empty(&dev->power.entry))
			list_move(&dev->power.entry, &dpm_late_early_list);

		if (error) {
			pm_dev_err(dev, state, " late", error);
			dpm_save_failed_dev(dev_name(dev));
			put_device(dev);
			break;
		}
		put_device(dev);

		if (async_error)
			break;
	}
	mutex_unlock(&dpm_list_mtx);
	async_synchronize_full();
	if (!error)
		error = async_error;
	if (error) {
		suspend_stats.failed_suspend_late++;
		dpm_save_failed_step(SUSPEND_SUSPEND_LATE);
		dpm_resume_early(resume_event(state));
	} else {
		dpm_show_time(starttime, state, "late");
	}
	trace_suspend_resume(TPS("dpm_suspend_late"), state.event, false);
	return error;
}

/**
 * dpm_suspend_end - Execute "late" and "noirq" device suspend callbacks.
 * @state: PM transition of the system being carried out.
 */
int dpm_suspend_end(pm_message_t state)
{
	int error = dpm_suspend_late(state);
	if (error)
		return error;

	error = dpm_suspend_noirq(state);
	if (error) {
		dpm_resume_early(resume_event(state));
		return error;
	}

	return 0;
}
EXPORT_SYMBOL_GPL(dpm_suspend_end);

/**
 * legacy_suspend - Execute a legacy (bus or class) suspend callback for device.
 * @dev: Device to suspend.
 * @state: PM transition of the system being carried out.
 * @cb: Suspend callback to execute.
 * @info: string description of caller.
 */
static int legacy_suspend(struct device *dev, pm_message_t state,
			  int (*cb)(struct device *dev, pm_message_t state),
			  char *info)
{
	int error;
	ktime_t calltime;

	calltime = initcall_debug_start(dev);

	trace_device_pm_callback_start(dev, info, state.event);
	error = cb(dev, state);
	trace_device_pm_callback_end(dev, error);
	suspend_report_result(cb, error);

	initcall_debug_report(dev, calltime, error, state, info);

	return error;
}

/**
 * device_suspend - Execute "suspend" callbacks for given device.
 * @dev: Device to handle.
 * @state: PM transition of the system being carried out.
 * @async: If true, the device is being suspended asynchronously.
 */
static int __device_suspend(struct device *dev, pm_message_t state, bool async)
{
	pm_callback_t callback = NULL;
	char *info = NULL;
	int error = 0;
	DECLARE_DPM_WATCHDOG_ON_STACK(wd);

	TRACE_DEVICE(dev);
	TRACE_SUSPEND(0);

	dpm_wait_for_children(dev, async);

	if (async_error)
		goto Complete;

	/*
	 * If a device configured to wake up the system from sleep states
	 * has been suspended at run time and there's a resume request pending
	 * for it, this is equivalent to the device signaling wakeup, so the
	 * system suspend operation should be aborted.
	 */
	if (pm_runtime_barrier(dev) && device_may_wakeup(dev))
		pm_wakeup_event(dev, 0);

	if (pm_wakeup_pending()) {
		async_error = -EBUSY;
		goto Complete;
	}

	if (dev->power.syscore)
		goto Complete;

	if (dev->power.direct_complete) {
		if (pm_runtime_status_suspended(dev)) {
			pm_runtime_disable(dev);
			if (pm_runtime_suspended_if_enabled(dev))
				goto Complete;

			pm_runtime_enable(dev);
		}
		dev->power.direct_complete = false;
	}

	dpm_watchdog_set(&wd, dev);
	device_lock(dev);

	if (dev->pm_domain) {
		info = "power domain ";
		callback = pm_op(&dev->pm_domain->ops, state);
		goto Run;
	}

	if (dev->type && dev->type->pm) {
		info = "type ";
		callback = pm_op(dev->type->pm, state);
		goto Run;
	}

	if (dev->class) {
		if (dev->class->pm) {
			info = "class ";
			callback = pm_op(dev->class->pm, state);
			goto Run;
		} else if (dev->class->suspend) {
			pm_dev_dbg(dev, state, "legacy class ");
			error = legacy_suspend(dev, state, dev->class->suspend,
						"legacy class ");
			goto End;
		}
	}

	if (dev->bus) {
		if (dev->bus->pm) {
			info = "bus ";
			callback = pm_op(dev->bus->pm, state);
		} else if (dev->bus->suspend) {
			pm_dev_dbg(dev, state, "legacy bus ");
			error = legacy_suspend(dev, state, dev->bus->suspend,
						"legacy bus ");
			goto End;
		}
	}

 Run:
	if (!callback && dev->driver && dev->driver->pm) {
		info = "driver ";
		callback = pm_op(dev->driver->pm, state);
	}

	error = dpm_run_callback(callback, dev, state, info);

 End:
	if (!error) {
		struct device *parent = dev->parent;

		dev->power.is_suspended = true;
		if (parent) {
			spin_lock_irq(&parent->power.lock);

			dev->parent->power.direct_complete = false;
			if (dev->power.wakeup_path
			    && !dev->parent->power.ignore_children)
				dev->parent->power.wakeup_path = true;

			spin_unlock_irq(&parent->power.lock);
		}
	}

	device_unlock(dev);
	dpm_watchdog_clear(&wd);

 Complete:
	complete_all(&dev->power.completion);
	if (error)
		async_error = error;

	TRACE_SUSPEND(error);
	return error;
}

static void async_suspend(void *data, async_cookie_t cookie)
{
	struct device *dev = (struct device *)data;
	int error;

	error = __device_suspend(dev, pm_transition, true);
	if (error) {
		dpm_save_failed_dev(dev_name(dev));
		pm_dev_err(dev, pm_transition, " async", error);
	}

	put_device(dev);
}

static int device_suspend(struct device *dev)
{
	reinit_completion(&dev->power.completion);

	if (is_async(dev)) {
		get_device(dev);
		async_schedule(async_suspend, dev);
		return 0;
	}

	return __device_suspend(dev, pm_transition, false);
}

/**
 * dpm_suspend - Execute "suspend" callbacks for all non-sysdev devices.
 * @state: PM transition of the system being carried out.
 */
int dpm_suspend(pm_message_t state)
{
	ktime_t starttime = ktime_get();
	int error = 0;

	trace_suspend_resume(TPS("dpm_suspend"), state.event, true);
	might_sleep();

	cpufreq_suspend();

	mutex_lock(&dpm_list_mtx);
	pm_transition = state;
	async_error = 0;
	while (!list_empty(&dpm_prepared_list)) {
		struct device *dev = to_device(dpm_prepared_list.prev);

		get_device(dev);
		mutex_unlock(&dpm_list_mtx);

		error = device_suspend(dev);

		mutex_lock(&dpm_list_mtx);
		if (error) {
			pm_dev_err(dev, state, "", error);
			dpm_save_failed_dev(dev_name(dev));
			put_device(dev);
			break;
		}
		if (!list_empty(&dev->power.entry))
			list_move(&dev->power.entry, &dpm_suspended_list);
		put_device(dev);
		if (async_error)
			break;
	}
	mutex_unlock(&dpm_list_mtx);
	async_synchronize_full();
	if (!error)
		error = async_error;
	if (error) {
		suspend_stats.failed_suspend++;
		dpm_save_failed_step(SUSPEND_SUSPEND);
	} else
		dpm_show_time(starttime, state, NULL);
	trace_suspend_resume(TPS("dpm_suspend"), state.event, false);
	return error;
}

/**
 * device_prepare - Prepare a device for system power transition.
 * @dev: Device to handle.
 * @state: PM transition of the system being carried out.
 *
 * Execute the ->prepare() callback(s) for given device.  No new children of the
 * device may be registered after this function has returned.
 */
static int device_prepare(struct device *dev, pm_message_t state)
{
	int (*callback)(struct device *) = NULL;
	char *info = NULL;
	int ret = 0;

	if (dev->power.syscore)
		return 0;

	/*
	 * If a device's parent goes into runtime suspend at the wrong time,
	 * it won't be possible to resume the device.  To prevent this we
	 * block runtime suspend here, during the prepare phase, and allow
	 * it again during the complete phase.
	 */
	pm_runtime_get_noresume(dev);

	device_lock(dev);

	dev->power.wakeup_path = device_may_wakeup(dev);

	if (dev->pm_domain) {
		info = "preparing power domain ";
		callback = dev->pm_domain->ops.prepare;
	} else if (dev->type && dev->type->pm) {
		info = "preparing type ";
		callback = dev->type->pm->prepare;
	} else if (dev->class && dev->class->pm) {
		info = "preparing class ";
		callback = dev->class->pm->prepare;
	} else if (dev->bus && dev->bus->pm) {
		info = "preparing bus ";
		callback = dev->bus->pm->prepare;
	}

	if (!callback && dev->driver && dev->driver->pm) {
		info = "preparing driver ";
		callback = dev->driver->pm->prepare;
	}

	if (callback) {
		trace_device_pm_callback_start(dev, info, state.event);
		ret = callback(dev);
		trace_device_pm_callback_end(dev, ret);
	}

	device_unlock(dev);

	if (ret < 0) {
		suspend_report_result(callback, ret);
		pm_runtime_put(dev);
		return ret;
	}
	/*
	 * A positive return value from ->prepare() means "this device appears
	 * to be runtime-suspended and its state is fine, so if it really is
	 * runtime-suspended, you can leave it in that state provided that you
	 * will do the same thing with all of its descendants".  This only
	 * applies to suspend transitions, however.
	 */
	spin_lock_irq(&dev->power.lock);
	dev->power.direct_complete = ret > 0 && state.event == PM_EVENT_SUSPEND;
	spin_unlock_irq(&dev->power.lock);
	return 0;
}

/**
 * dpm_prepare - Prepare all non-sysdev devices for a system PM transition.
 * @state: PM transition of the system being carried out.
 *
 * Execute the ->prepare() callback(s) for all devices.
 */
int dpm_prepare(pm_message_t state)
{
	int error = 0;

	trace_suspend_resume(TPS("dpm_prepare"), state.event, true);
	might_sleep();

	mutex_lock(&dpm_list_mtx);
	while (!list_empty(&dpm_list)) {
		struct device *dev = to_device(dpm_list.next);

		get_device(dev);
		mutex_unlock(&dpm_list_mtx);

		error = device_prepare(dev, state);

		mutex_lock(&dpm_list_mtx);
		if (error) {
			if (error == -EAGAIN) {
				put_device(dev);
				error = 0;
				continue;
			}
			printk(KERN_INFO "PM: Device %s not prepared "
				"for power transition: code %d\n",
				dev_name(dev), error);
			put_device(dev);
			break;
		}
		dev->power.is_prepared = true;
		if (!list_empty(&dev->power.entry))
			list_move_tail(&dev->power.entry, &dpm_prepared_list);
		put_device(dev);
	}
	mutex_unlock(&dpm_list_mtx);
	trace_suspend_resume(TPS("dpm_prepare"), state.event, false);
	return error;
}

/**
 * dpm_suspend_start - Prepare devices for PM transition and suspend them.
 * @state: PM transition of the system being carried out.
 *
 * Prepare all non-sysdev devices for system PM transition and execute "suspend"
 * callbacks for them.
 */
int dpm_suspend_start(pm_message_t state)
{
	int error;

	error = dpm_prepare(state);
	if (error) {
		suspend_stats.failed_prepare++;
		dpm_save_failed_step(SUSPEND_PREPARE);
	} else
		error = dpm_suspend(state);
	return error;
}
EXPORT_SYMBOL_GPL(dpm_suspend_start);

void __suspend_report_result(const char *function, void *fn, int ret)
{
	if (ret)
		printk(KERN_ERR "%s(): %pF returns %d\n", function, fn, ret);
}
EXPORT_SYMBOL_GPL(__suspend_report_result);

/**
 * device_pm_wait_for_dev - Wait for suspend/resume of a device to complete.
 * @dev: Device to wait for.
 * @subordinate: Device that needs to wait for @dev.
 */
int device_pm_wait_for_dev(struct device *subordinate, struct device *dev)
{
	dpm_wait(dev, subordinate->power.async_suspend);
	return async_error;
}
EXPORT_SYMBOL_GPL(device_pm_wait_for_dev);

/**
 * dpm_for_each_dev - device iterator.
 * @data: data for the callback.
 * @fn: function to be called for each device.
 *
 * Iterate over devices in dpm_list, and call @fn for each device,
 * passing it @data.
 */
void dpm_for_each_dev(void *data, void (*fn)(struct device *, void *))
{
	struct device *dev;

	if (!fn)
		return;

	device_pm_lock();
	list_for_each_entry(dev, &dpm_list, power.entry)
		fn(dev, data);
	device_pm_unlock();
}
EXPORT_SYMBOL_GPL(dpm_for_each_dev);
