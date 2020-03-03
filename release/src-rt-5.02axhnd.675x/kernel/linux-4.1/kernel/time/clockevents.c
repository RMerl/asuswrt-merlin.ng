/*
 * linux/kernel/time/clockevents.c
 *
 * This file contains functions which manage clock event devices.
 *
 * Copyright(C) 2005-2006, Thomas Gleixner <tglx@linutronix.de>
 * Copyright(C) 2005-2007, Red Hat, Inc., Ingo Molnar
 * Copyright(C) 2006-2007, Timesys Corp., Thomas Gleixner
 *
 * This code is licenced under the GPL version 2. For details see
 * kernel-base/COPYING.
 */

#include <linux/clockchips.h>
#include <linux/hrtimer.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/smp.h>
#include <linux/device.h>

#include "tick-internal.h"

/* The registered clock event devices */
static LIST_HEAD(clockevent_devices);
static LIST_HEAD(clockevents_released);
/* Protection for the above */
static DEFINE_RAW_SPINLOCK(clockevents_lock);
/* Protection for unbind operations */
static DEFINE_MUTEX(clockevents_mutex);

struct ce_unbind {
	struct clock_event_device *ce;
	int res;
};

static u64 cev_delta2ns(unsigned long latch, struct clock_event_device *evt,
			bool ismax)
{
	u64 clc = (u64) latch << evt->shift;
	u64 rnd;

	if (unlikely(!evt->mult)) {
		evt->mult = 1;
		WARN_ON(1);
	}
	rnd = (u64) evt->mult - 1;

	/*
	 * Upper bound sanity check. If the backwards conversion is
	 * not equal latch, we know that the above shift overflowed.
	 */
	if ((clc >> evt->shift) != (u64)latch)
		clc = ~0ULL;

	/*
	 * Scaled math oddities:
	 *
	 * For mult <= (1 << shift) we can safely add mult - 1 to
	 * prevent integer rounding loss. So the backwards conversion
	 * from nsec to device ticks will be correct.
	 *
	 * For mult > (1 << shift), i.e. device frequency is > 1GHz we
	 * need to be careful. Adding mult - 1 will result in a value
	 * which when converted back to device ticks can be larger
	 * than latch by up to (mult - 1) >> shift. For the min_delta
	 * calculation we still want to apply this in order to stay
	 * above the minimum device ticks limit. For the upper limit
	 * we would end up with a latch value larger than the upper
	 * limit of the device, so we omit the add to stay below the
	 * device upper boundary.
	 *
	 * Also omit the add if it would overflow the u64 boundary.
	 */
	if ((~0ULL - clc > rnd) &&
	    (!ismax || evt->mult <= (1ULL << evt->shift)))
		clc += rnd;

	do_div(clc, evt->mult);

	/* Deltas less than 1usec are pointless noise */
	return clc > 1000 ? clc : 1000;
}

/**
 * clockevents_delta2ns - Convert a latch value (device ticks) to nanoseconds
 * @latch:	value to convert
 * @evt:	pointer to clock event device descriptor
 *
 * Math helper, returns latch value converted to nanoseconds (bound checked)
 */
u64 clockevent_delta2ns(unsigned long latch, struct clock_event_device *evt)
{
	return cev_delta2ns(latch, evt, false);
}
EXPORT_SYMBOL_GPL(clockevent_delta2ns);

static int __clockevents_set_state(struct clock_event_device *dev,
				   enum clock_event_state state)
{
	/* Transition with legacy set_mode() callback */
	if (dev->set_mode) {
		/* Legacy callback doesn't support new modes */
		if (state > CLOCK_EVT_STATE_ONESHOT)
			return -ENOSYS;
		/*
		 * 'clock_event_state' and 'clock_event_mode' have 1-to-1
		 * mapping until *_ONESHOT, and so a simple cast will work.
		 */
		dev->set_mode((enum clock_event_mode)state, dev);
		dev->mode = (enum clock_event_mode)state;
		return 0;
	}

	if (dev->features & CLOCK_EVT_FEAT_DUMMY)
		return 0;

	/* Transition with new state-specific callbacks */
	switch (state) {
	case CLOCK_EVT_STATE_DETACHED:
		/* The clockevent device is getting replaced. Shut it down. */

	case CLOCK_EVT_STATE_SHUTDOWN:
		return dev->set_state_shutdown(dev);

	case CLOCK_EVT_STATE_PERIODIC:
		/* Core internal bug */
		if (!(dev->features & CLOCK_EVT_FEAT_PERIODIC))
			return -ENOSYS;
		return dev->set_state_periodic(dev);

	case CLOCK_EVT_STATE_ONESHOT:
		/* Core internal bug */
		if (!(dev->features & CLOCK_EVT_FEAT_ONESHOT))
			return -ENOSYS;
		return dev->set_state_oneshot(dev);

	default:
		return -ENOSYS;
	}
}

/**
 * clockevents_set_state - set the operating state of a clock event device
 * @dev:	device to modify
 * @state:	new state
 *
 * Must be called with interrupts disabled !
 */
void clockevents_set_state(struct clock_event_device *dev,
			   enum clock_event_state state)
{
	if (dev->state != state) {
		if (__clockevents_set_state(dev, state))
			return;

		dev->state = state;

		/*
		 * A nsec2cyc multiplicator of 0 is invalid and we'd crash
		 * on it, so fix it up and emit a warning:
		 */
		if (state == CLOCK_EVT_STATE_ONESHOT) {
			if (unlikely(!dev->mult)) {
				dev->mult = 1;
				WARN_ON(1);
			}
		}
	}
}

/**
 * clockevents_shutdown - shutdown the device and clear next_event
 * @dev:	device to shutdown
 */
void clockevents_shutdown(struct clock_event_device *dev)
{
	clockevents_set_state(dev, CLOCK_EVT_STATE_SHUTDOWN);
	dev->next_event.tv64 = KTIME_MAX;
}

/**
 * clockevents_tick_resume -	Resume the tick device before using it again
 * @dev:			device to resume
 */
int clockevents_tick_resume(struct clock_event_device *dev)
{
	int ret = 0;

	if (dev->set_mode) {
		dev->set_mode(CLOCK_EVT_MODE_RESUME, dev);
		dev->mode = CLOCK_EVT_MODE_RESUME;
	} else if (dev->tick_resume) {
		ret = dev->tick_resume(dev);
	}

	return ret;
}

#ifdef CONFIG_GENERIC_CLOCKEVENTS_MIN_ADJUST

/* Limit min_delta to a jiffie */
#define MIN_DELTA_LIMIT		(NSEC_PER_SEC / HZ)

/**
 * clockevents_increase_min_delta - raise minimum delta of a clock event device
 * @dev:       device to increase the minimum delta
 *
 * Returns 0 on success, -ETIME when the minimum delta reached the limit.
 */
static int clockevents_increase_min_delta(struct clock_event_device *dev)
{
	/* Nothing to do if we already reached the limit */
	if (dev->min_delta_ns >= MIN_DELTA_LIMIT) {
		printk_deferred(KERN_WARNING
				"CE: Reprogramming failure. Giving up\n");
		dev->next_event.tv64 = KTIME_MAX;
		return -ETIME;
	}

	if (dev->min_delta_ns < 5000)
		dev->min_delta_ns = 5000;
	else
		dev->min_delta_ns += dev->min_delta_ns >> 1;

	if (dev->min_delta_ns > MIN_DELTA_LIMIT)
		dev->min_delta_ns = MIN_DELTA_LIMIT;

	printk_deferred(KERN_WARNING
			"CE: %s increased min_delta_ns to %llu nsec\n",
			dev->name ? dev->name : "?",
			(unsigned long long) dev->min_delta_ns);
	return 0;
}

/**
 * clockevents_program_min_delta - Set clock event device to the minimum delay.
 * @dev:	device to program
 *
 * Returns 0 on success, -ETIME when the retry loop failed.
 */
static int clockevents_program_min_delta(struct clock_event_device *dev)
{
	unsigned long long clc;
	int64_t delta;
	int i;

	for (i = 0;;) {
		delta = dev->min_delta_ns;
		dev->next_event = ktime_add_ns(ktime_get(), delta);

		if (dev->state == CLOCK_EVT_STATE_SHUTDOWN)
			return 0;

		dev->retries++;
		clc = ((unsigned long long) delta * dev->mult) >> dev->shift;
		if (dev->set_next_event((unsigned long) clc, dev) == 0)
			return 0;

		if (++i > 2) {
			/*
			 * We tried 3 times to program the device with the
			 * given min_delta_ns. Try to increase the minimum
			 * delta, if that fails as well get out of here.
			 */
			if (clockevents_increase_min_delta(dev))
				return -ETIME;
			i = 0;
		}
	}
}

#else  /* CONFIG_GENERIC_CLOCKEVENTS_MIN_ADJUST */

/**
 * clockevents_program_min_delta - Set clock event device to the minimum delay.
 * @dev:	device to program
 *
 * Returns 0 on success, -ETIME when the retry loop failed.
 */
static int clockevents_program_min_delta(struct clock_event_device *dev)
{
	unsigned long long clc;
	int64_t delta;

	delta = dev->min_delta_ns;
	dev->next_event = ktime_add_ns(ktime_get(), delta);

	if (dev->state == CLOCK_EVT_STATE_SHUTDOWN)
		return 0;

	dev->retries++;
	clc = ((unsigned long long) delta * dev->mult) >> dev->shift;
	return dev->set_next_event((unsigned long) clc, dev);
}

#endif /* CONFIG_GENERIC_CLOCKEVENTS_MIN_ADJUST */

/**
 * clockevents_program_event - Reprogram the clock event device.
 * @dev:	device to program
 * @expires:	absolute expiry time (monotonic clock)
 * @force:	program minimum delay if expires can not be set
 *
 * Returns 0 on success, -ETIME when the event is in the past.
 */
int clockevents_program_event(struct clock_event_device *dev, ktime_t expires,
			      bool force)
{
	unsigned long long clc;
	int64_t delta;
	int rc;

	if (unlikely(expires.tv64 < 0)) {
		WARN_ON_ONCE(1);
		return -ETIME;
	}

	dev->next_event = expires;

	if (dev->state == CLOCK_EVT_STATE_SHUTDOWN)
		return 0;

	/* Shortcut for clockevent devices that can deal with ktime. */
	if (dev->features & CLOCK_EVT_FEAT_KTIME)
		return dev->set_next_ktime(expires, dev);

	delta = ktime_to_ns(ktime_sub(expires, ktime_get()));
	if (delta <= 0)
		return force ? clockevents_program_min_delta(dev) : -ETIME;

	delta = min(delta, (int64_t) dev->max_delta_ns);
	delta = max(delta, (int64_t) dev->min_delta_ns);

	clc = ((unsigned long long) delta * dev->mult) >> dev->shift;
	rc = dev->set_next_event((unsigned long) clc, dev);

	return (rc && force) ? clockevents_program_min_delta(dev) : rc;
}

/*
 * Called after a notify add to make devices available which were
 * released from the notifier call.
 */
static void clockevents_notify_released(void)
{
	struct clock_event_device *dev;

	while (!list_empty(&clockevents_released)) {
		dev = list_entry(clockevents_released.next,
				 struct clock_event_device, list);
		list_del(&dev->list);
		list_add(&dev->list, &clockevent_devices);
		tick_check_new_device(dev);
	}
}

/*
 * Try to install a replacement clock event device
 */
static int clockevents_replace(struct clock_event_device *ced)
{
	struct clock_event_device *dev, *newdev = NULL;

	list_for_each_entry(dev, &clockevent_devices, list) {
		if (dev == ced || dev->state != CLOCK_EVT_STATE_DETACHED)
			continue;

		if (!tick_check_replacement(newdev, dev))
			continue;

		if (!try_module_get(dev->owner))
			continue;

		if (newdev)
			module_put(newdev->owner);
		newdev = dev;
	}
	if (newdev) {
		tick_install_replacement(newdev);
		list_del_init(&ced->list);
	}
	return newdev ? 0 : -EBUSY;
}

/*
 * Called with clockevents_mutex and clockevents_lock held
 */
static int __clockevents_try_unbind(struct clock_event_device *ced, int cpu)
{
	/* Fast track. Device is unused */
	if (ced->state == CLOCK_EVT_STATE_DETACHED) {
		list_del_init(&ced->list);
		return 0;
	}

	return ced == per_cpu(tick_cpu_device, cpu).evtdev ? -EAGAIN : -EBUSY;
}

/*
 * SMP function call to unbind a device
 */
static void __clockevents_unbind(void *arg)
{
	struct ce_unbind *cu = arg;
	int res;

	raw_spin_lock(&clockevents_lock);
	res = __clockevents_try_unbind(cu->ce, smp_processor_id());
	if (res == -EAGAIN)
		res = clockevents_replace(cu->ce);
	cu->res = res;
	raw_spin_unlock(&clockevents_lock);
}

/*
 * Issues smp function call to unbind a per cpu device. Called with
 * clockevents_mutex held.
 */
static int clockevents_unbind(struct clock_event_device *ced, int cpu)
{
	struct ce_unbind cu = { .ce = ced, .res = -ENODEV };

	smp_call_function_single(cpu, __clockevents_unbind, &cu, 1);
	return cu.res;
}

/*
 * Unbind a clockevents device.
 */
int clockevents_unbind_device(struct clock_event_device *ced, int cpu)
{
	int ret;

	mutex_lock(&clockevents_mutex);
	ret = clockevents_unbind(ced, cpu);
	mutex_unlock(&clockevents_mutex);
	return ret;
}
EXPORT_SYMBOL_GPL(clockevents_unbind_device);

/* Sanity check of state transition callbacks */
static int clockevents_sanity_check(struct clock_event_device *dev)
{
	/* Legacy set_mode() callback */
	if (dev->set_mode) {
		/* We shouldn't be supporting new modes now */
		WARN_ON(dev->set_state_periodic || dev->set_state_oneshot ||
			dev->set_state_shutdown || dev->tick_resume);

		BUG_ON(dev->mode != CLOCK_EVT_MODE_UNUSED);
		return 0;
	}

	if (dev->features & CLOCK_EVT_FEAT_DUMMY)
		return 0;

	/* New state-specific callbacks */
	if (!dev->set_state_shutdown)
		return -EINVAL;

	if ((dev->features & CLOCK_EVT_FEAT_PERIODIC) &&
	    !dev->set_state_periodic)
		return -EINVAL;

	if ((dev->features & CLOCK_EVT_FEAT_ONESHOT) &&
	    !dev->set_state_oneshot)
		return -EINVAL;

	return 0;
}

/**
 * clockevents_register_device - register a clock event device
 * @dev:	device to register
 */
void clockevents_register_device(struct clock_event_device *dev)
{
	unsigned long flags;

	BUG_ON(clockevents_sanity_check(dev));

	/* Initialize state to DETACHED */
	dev->state = CLOCK_EVT_STATE_DETACHED;

	if (!dev->cpumask) {
		WARN_ON(num_possible_cpus() > 1);
		dev->cpumask = cpumask_of(smp_processor_id());
	}

	raw_spin_lock_irqsave(&clockevents_lock, flags);

	list_add(&dev->list, &clockevent_devices);
	tick_check_new_device(dev);
	clockevents_notify_released();

	raw_spin_unlock_irqrestore(&clockevents_lock, flags);
}
EXPORT_SYMBOL_GPL(clockevents_register_device);

void clockevents_config(struct clock_event_device *dev, u32 freq)
{
	u64 sec;

	if (!(dev->features & CLOCK_EVT_FEAT_ONESHOT))
		return;

	/*
	 * Calculate the maximum number of seconds we can sleep. Limit
	 * to 10 minutes for hardware which can program more than
	 * 32bit ticks so we still get reasonable conversion values.
	 */
	sec = dev->max_delta_ticks;
	do_div(sec, freq);
	if (!sec)
		sec = 1;
	else if (sec > 600 && dev->max_delta_ticks > UINT_MAX)
		sec = 600;

	clockevents_calc_mult_shift(dev, freq, sec);
	dev->min_delta_ns = cev_delta2ns(dev->min_delta_ticks, dev, false);
	dev->max_delta_ns = cev_delta2ns(dev->max_delta_ticks, dev, true);
}

/**
 * clockevents_config_and_register - Configure and register a clock event device
 * @dev:	device to register
 * @freq:	The clock frequency
 * @min_delta:	The minimum clock ticks to program in oneshot mode
 * @max_delta:	The maximum clock ticks to program in oneshot mode
 *
 * min/max_delta can be 0 for devices which do not support oneshot mode.
 */
void clockevents_config_and_register(struct clock_event_device *dev,
				     u32 freq, unsigned long min_delta,
				     unsigned long max_delta)
{
	dev->min_delta_ticks = min_delta;
	dev->max_delta_ticks = max_delta;
	clockevents_config(dev, freq);
	clockevents_register_device(dev);
}
EXPORT_SYMBOL_GPL(clockevents_config_and_register);

int __clockevents_update_freq(struct clock_event_device *dev, u32 freq)
{
	clockevents_config(dev, freq);

	if (dev->state == CLOCK_EVT_STATE_ONESHOT)
		return clockevents_program_event(dev, dev->next_event, false);

	if (dev->state == CLOCK_EVT_STATE_PERIODIC)
		return __clockevents_set_state(dev, CLOCK_EVT_STATE_PERIODIC);

	return 0;
}

/**
 * clockevents_update_freq - Update frequency and reprogram a clock event device.
 * @dev:	device to modify
 * @freq:	new device frequency
 *
 * Reconfigure and reprogram a clock event device in oneshot
 * mode. Must be called on the cpu for which the device delivers per
 * cpu timer events. If called for the broadcast device the core takes
 * care of serialization.
 *
 * Returns 0 on success, -ETIME when the event is in the past.
 */
int clockevents_update_freq(struct clock_event_device *dev, u32 freq)
{
	unsigned long flags;
	int ret;

	local_irq_save(flags);
	ret = tick_broadcast_update_freq(dev, freq);
	if (ret == -ENODEV)
		ret = __clockevents_update_freq(dev, freq);
	local_irq_restore(flags);
	return ret;
}

/*
 * Noop handler when we shut down an event device
 */
void clockevents_handle_noop(struct clock_event_device *dev)
{
}

/**
 * clockevents_exchange_device - release and request clock devices
 * @old:	device to release (can be NULL)
 * @new:	device to request (can be NULL)
 *
 * Called from various tick functions with clockevents_lock held and
 * interrupts disabled.
 */
void clockevents_exchange_device(struct clock_event_device *old,
				 struct clock_event_device *new)
{
	/*
	 * Caller releases a clock event device. We queue it into the
	 * released list and do a notify add later.
	 */
	if (old) {
		module_put(old->owner);
		clockevents_set_state(old, CLOCK_EVT_STATE_DETACHED);
		list_del(&old->list);
		list_add(&old->list, &clockevents_released);
	}

	if (new) {
		BUG_ON(new->state != CLOCK_EVT_STATE_DETACHED);
		clockevents_shutdown(new);
	}
}

/**
 * clockevents_suspend - suspend clock devices
 */
void clockevents_suspend(void)
{
	struct clock_event_device *dev;

	list_for_each_entry_reverse(dev, &clockevent_devices, list)
		if (dev->suspend)
			dev->suspend(dev);
}

/**
 * clockevents_resume - resume clock devices
 */
void clockevents_resume(void)
{
	struct clock_event_device *dev;

	list_for_each_entry(dev, &clockevent_devices, list)
		if (dev->resume)
			dev->resume(dev);
}

#ifdef CONFIG_HOTPLUG_CPU
/**
 * tick_cleanup_dead_cpu - Cleanup the tick and clockevents of a dead cpu
 */
void tick_cleanup_dead_cpu(int cpu)
{
	struct clock_event_device *dev, *tmp;
	unsigned long flags;

	raw_spin_lock_irqsave(&clockevents_lock, flags);

	tick_shutdown_broadcast_oneshot(cpu);
	tick_shutdown_broadcast(cpu);
	tick_shutdown(cpu);
	/*
	 * Unregister the clock event devices which were
	 * released from the users in the notify chain.
	 */
	list_for_each_entry_safe(dev, tmp, &clockevents_released, list)
		list_del(&dev->list);
	/*
	 * Now check whether the CPU has left unused per cpu devices
	 */
	list_for_each_entry_safe(dev, tmp, &clockevent_devices, list) {
		if (cpumask_test_cpu(cpu, dev->cpumask) &&
		    cpumask_weight(dev->cpumask) == 1 &&
		    !tick_is_broadcast_device(dev)) {
			BUG_ON(dev->state != CLOCK_EVT_STATE_DETACHED);
			list_del(&dev->list);
		}
	}
	raw_spin_unlock_irqrestore(&clockevents_lock, flags);
}
#endif

#ifdef CONFIG_SYSFS
struct bus_type clockevents_subsys = {
	.name		= "clockevents",
	.dev_name       = "clockevent",
};

static DEFINE_PER_CPU(struct device, tick_percpu_dev);
static struct tick_device *tick_get_tick_dev(struct device *dev);

static ssize_t sysfs_show_current_tick_dev(struct device *dev,
					   struct device_attribute *attr,
					   char *buf)
{
	struct tick_device *td;
	ssize_t count = 0;

	raw_spin_lock_irq(&clockevents_lock);
	td = tick_get_tick_dev(dev);
	if (td && td->evtdev)
		count = snprintf(buf, PAGE_SIZE, "%s\n", td->evtdev->name);
	raw_spin_unlock_irq(&clockevents_lock);
	return count;
}
static DEVICE_ATTR(current_device, 0444, sysfs_show_current_tick_dev, NULL);

/* We don't support the abomination of removable broadcast devices */
static ssize_t sysfs_unbind_tick_dev(struct device *dev,
				     struct device_attribute *attr,
				     const char *buf, size_t count)
{
	char name[CS_NAME_LEN];
	ssize_t ret = sysfs_get_uname(buf, name, count);
	struct clock_event_device *ce;

	if (ret < 0)
		return ret;

	ret = -ENODEV;
	mutex_lock(&clockevents_mutex);
	raw_spin_lock_irq(&clockevents_lock);
	list_for_each_entry(ce, &clockevent_devices, list) {
		if (!strcmp(ce->name, name)) {
			ret = __clockevents_try_unbind(ce, dev->id);
			break;
		}
	}
	raw_spin_unlock_irq(&clockevents_lock);
	/*
	 * We hold clockevents_mutex, so ce can't go away
	 */
	if (ret == -EAGAIN)
		ret = clockevents_unbind(ce, dev->id);
	mutex_unlock(&clockevents_mutex);
	return ret ? ret : count;
}
static DEVICE_ATTR(unbind_device, 0200, NULL, sysfs_unbind_tick_dev);

#ifdef CONFIG_GENERIC_CLOCKEVENTS_BROADCAST
static struct device tick_bc_dev = {
	.init_name	= "broadcast",
	.id		= 0,
	.bus		= &clockevents_subsys,
};

static struct tick_device *tick_get_tick_dev(struct device *dev)
{
	return dev == &tick_bc_dev ? tick_get_broadcast_device() :
		&per_cpu(tick_cpu_device, dev->id);
}

static __init int tick_broadcast_init_sysfs(void)
{
	int err = device_register(&tick_bc_dev);

	if (!err)
		err = device_create_file(&tick_bc_dev, &dev_attr_current_device);
	return err;
}
#else
static struct tick_device *tick_get_tick_dev(struct device *dev)
{
	return &per_cpu(tick_cpu_device, dev->id);
}
static inline int tick_broadcast_init_sysfs(void) { return 0; }
#endif

static int __init tick_init_sysfs(void)
{
	int cpu;

	for_each_possible_cpu(cpu) {
		struct device *dev = &per_cpu(tick_percpu_dev, cpu);
		int err;

		dev->id = cpu;
		dev->bus = &clockevents_subsys;
		err = device_register(dev);
		if (!err)
			err = device_create_file(dev, &dev_attr_current_device);
		if (!err)
			err = device_create_file(dev, &dev_attr_unbind_device);
		if (err)
			return err;
	}
	return tick_broadcast_init_sysfs();
}

static int __init clockevents_init_sysfs(void)
{
	int err = subsys_system_register(&clockevents_subsys, NULL);

	if (!err)
		err = tick_init_sysfs();
	return err;
}
device_initcall(clockevents_init_sysfs);
#endif /* SYSFS */
