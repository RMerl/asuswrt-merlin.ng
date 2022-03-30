/* SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause */
/*
 * Copyright (C) 2018, STMicroelectronics - All Rights Reserved
 */

#ifndef _HWSPINLOCK_H_
#define _HWSPINLOCK_H_

/**
 * Implement a hwspinlock uclass.
 * Hardware spinlocks are used to perform hardware protection of
 * critical sections and synchronisation between multiprocessors.
 */

struct udevice;

/**
 * struct hwspinlock - A handle to (allowing control of) a single hardware
 * spinlock.
 *
 * @dev: The device which implements the hardware spinlock.
 * @id: The hardware spinlock ID within the provider.
 */
struct hwspinlock {
	struct udevice *dev;
	unsigned long id;
};

#if CONFIG_IS_ENABLED(DM_HWSPINLOCK)

/**
 * hwspinlock_get_by_index - Get a hardware spinlock by integer index
 *
 * This looks up and request a hardware spinlock. The index is relative to the
 * client device; each device is assumed to have n hardware spinlock associated
 * with it somehow, and this function finds and requests one of them.
 *
 * @dev:	The client device.
 * @index:	The index of the hardware spinlock to request, within the
 *		client's list of hardware spinlock.
 * @hws:	A pointer to a hardware spinlock struct to initialize.
 * @return 0 if OK, or a negative error code.
 */
int hwspinlock_get_by_index(struct udevice *dev,
			    int index, struct hwspinlock *hws);

/**
 * Lock the hardware spinlock
 *
 * @hws:	A hardware spinlock struct that previously requested by
 *		hwspinlock_get_by_index
 * @timeout:	Timeout value in msecs
 * @return: 0 if OK, -ETIMEDOUT if timeout, -ve on other errors
 */
int hwspinlock_lock_timeout(struct hwspinlock *hws, unsigned int timeout);

/**
 * Unlock the hardware spinlock
 *
 * @hws:	A hardware spinlock struct that previously requested by
 *		hwspinlock_get_by_index
 * @return: 0 if OK, -ve on error
 */
int hwspinlock_unlock(struct hwspinlock *hws);

#else

static inline int hwspinlock_get_by_index(struct udevice *dev,
					  int index,
					  struct hwspinlock *hws)
{
	return -ENOSYS;
}

static inline int hwspinlock_lock_timeout(struct hwspinlock *hws,
					  int timeout)
{
	return -ENOSYS;
}

static inline int hwspinlock_unlock(struct hwspinlock *hws)
{
	return -ENOSYS;
}

#endif /* CONFIG_DM_HWSPINLOCK */

struct ofnode_phandle_args;

/**
 * struct hwspinlock_ops - Driver model hwspinlock operations
 *
 * The uclass interface is implemented by all hwspinlock devices which use
 * driver model.
 */
struct hwspinlock_ops {
	/**
	 * of_xlate - Translate a client's device-tree (OF) hardware specifier.
	 *
	 * The hardware core calls this function as the first step in
	 * implementing a client's hwspinlock_get_by_*() call.
	 *
	 * @hws:	The hardware spinlock struct to hold the translation
	 *			result.
	 * @args:	The hardware spinlock specifier values from device tree.
	 * @return 0 if OK, or a negative error code.
	 */
	int (*of_xlate)(struct hwspinlock *hws,
			struct ofnode_phandle_args *args);

	/**
	 * Lock the hardware spinlock
	 *
	 * @dev:	hwspinlock Device
	 * @index:	index of the lock to be used
	 * @return 0 if OK, -ve on error
	 */
	int (*lock)(struct udevice *dev, int index);

	/**
	 * Unlock the hardware spinlock
	 *
	 * @dev:	hwspinlock Device
	 * @index:	index of the lock to be unlocked
	 * @return 0 if OK, -ve on error
	 */
	int (*unlock)(struct udevice *dev, int index);

	/**
	 * Relax - optional
	 *       Platform-specific relax method, called by hwspinlock core
	 *       while spinning on a lock, between two successive call to
	 *       lock
	 *
	 * @dev:	hwspinlock Device
	 */
	void (*relax)(struct udevice *dev);
};

#endif /* _HWSPINLOCK_H_ */
