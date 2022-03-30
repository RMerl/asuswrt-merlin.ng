/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2012-2014 The Linux Foundation. All rights reserved.
 */

#ifndef _LINUX_IOPOLL_H
#define _LINUX_IOPOLL_H

#include <linux/errno.h>
#include <linux/io.h>
#include <time.h>

/**
 * readx_poll_timeout - Periodically poll an address until a condition is met or a timeout occurs
 * @op: accessor function (takes @addr as its only argument)
 * @addr: Address to poll
 * @val: Variable to read the value into
 * @cond: Break condition (usually involving @val)
 * @timeout_us: Timeout in us, 0 means never timeout
 *
 * Returns 0 on success and -ETIMEDOUT upon a timeout. In either
 * case, the last read value at @addr is stored in @val.
 *
 * When available, you'll probably want to use one of the specialized
 * macros defined below rather than this macro directly.
 */
#define readx_poll_timeout(op, addr, val, cond, timeout_us)	\
({ \
	unsigned long timeout = timer_get_us() + timeout_us; \
	for (;;) { \
		(val) = op(addr); \
		if (cond) \
			break; \
		if (timeout_us && time_after(timer_get_us(), timeout)) { \
			(val) = op(addr); \
			break; \
		} \
	} \
	(cond) ? 0 : -ETIMEDOUT; \
})


#define readb_poll_timeout(addr, val, cond, timeout_us) \
	readx_poll_timeout(readb, addr, val, cond, timeout_us)

#define readw_poll_timeout(addr, val, cond, timeout_us) \
	readx_poll_timeout(readw, addr, val, cond, timeout_us)

#define readl_poll_timeout(addr, val, cond, timeout_us) \
	readx_poll_timeout(readl, addr, val, cond, timeout_us)

#define readq_poll_timeout(addr, val, cond, timeout_us) \
	readx_poll_timeout(readq, addr, val, cond, timeout_us)

#define readb_relaxed_poll_timeout(addr, val, cond, timeout_us) \
	readx_poll_timeout(readb_relaxed, addr, val, cond, timeout_us)

#define readw_relaxed_poll_timeout(addr, val, cond, timeout_us) \
	readx_poll_timeout(readw_relaxed, addr, val, cond, timeout_us)

#define readl_relaxed_poll_timeout(addr, val, cond, timeout_us) \
	readx_poll_timeout(readl_relaxed, addr, val, cond, timeout_us)

#define readq_relaxed_poll_timeout(addr, val, cond, timeout_us) \
	readx_poll_timeout(readq_relaxed, addr, val, cond, timeout_us)

#endif /* _LINUX_IOPOLL_H */
