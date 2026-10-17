/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2020 Sean Anderson <seanga2@gmail.com>
 * Copyright (c) 2013 Google, Inc
 *
 * (C) Copyright 2012
 * Pavel Herrmann <morpheus.ibis@gmail.com>
 * Marek Vasut <marex@denx.de>
 */

#ifndef _DM_DEVICE_COMPAT_H
#define _DM_DEVICE_COMPAT_H

#include <log.h>
#include <linux/build_bug.h>
#include <linux/compat.h>
#include <linux/printk.h>

/*
 * Define a new identifier which can be tested on by C code. A similar
 * definition is made for DEBUG in <log.h>.
 */
#ifdef VERBOSE_DEBUG
#define _VERBOSE_DEBUG 1
#else
#define _VERBOSE_DEBUG 0
#endif

/**
 * dev_printk_emit() - Emit a formatted log message
 * @cat: Category of the message
 * @level: Log level of the message
 * @fmt: Format string
 * @...: Arguments for @fmt
 *
 * This macro logs a message through the appropriate channel. It is a macro so
 * the if statements can be optimized out (as @level should be a constant known
 * at compile-time).
 *
 * If DEBUG or VERBOSE_DEBUG is defined, then some messages are always printed
 * (through printf()). This is to match the historical behavior of the dev_xxx
 * functions.
 *
 * If LOG is enabled, use log() to emit the message, otherwise print it based on
 * the console loglevel.
 */
#define dev_printk_emit(cat, level, fmt, ...) \
({ \
	if ((_DEBUG && level == LOGL_DEBUG) || \
	    (_VERBOSE_DEBUG && level == LOGL_DEBUG_CONTENT)) \
		printf(fmt, ##__VA_ARGS__); \
	else if (CONFIG_IS_ENABLED(LOG)) \
		log(cat, level, fmt, ##__VA_ARGS__); \
	else if (level < CONFIG_VAL(LOGLEVEL)) \
		printf(fmt, ##__VA_ARGS__); \
})

/**
 * __dev_printk() - Log a message for a device
 * @level: Log level of the message
 * @dev: A &struct udevice or &struct device
 * @fmt: Format string
 * @...: Arguments for @fmt
 *
 * This macro formats and prints dev_xxx log messages. It is done as a macro
 * because working with variadic argument is much easier this way, we can
 * interrogate the type of device we are passed (and whether it *is* a &struct
 * udevice or &struct device), and dev_printk_emit() can optimize out unused if
 * branches.
 *
 * Because this is a macro, we must enforce type checks ourselves. Ideally, we
 * would only accept udevices, but there is a significant amount of code (mostly
 * USB) which calls dev_xxx with &struct device. When assigning ``__dev``, we
 * must first cast ``dev`` to ``void *`` so we don't get warned when ``dev`` is
 * a &struct device. Even though the latter branch is not taken, it will still
 * get compiled and type-checked.
 *
 * The format strings in case of a ``NULL`` ``dev`` MUST be kept the same.
 * Otherwise, @fmt will be duplicated in the data section (with slightly
 * different prefixes). This is why ``(NULL udevice *)`` is printed as two
 * string arguments, and not by string pasting.
 */
#define __dev_printk(level, dev, fmt, ...) \
({ \
	if (__same_type(dev, struct device  *)) { \
		dev_printk_emit(LOG_CATEGORY, level, fmt, ##__VA_ARGS__); \
	} else { \
		BUILD_BUG_ON(!__same_type(dev, struct udevice *)); \
		struct udevice *__dev = (void *)dev; \
		if (__dev) \
			dev_printk_emit(__dev->driver->id, level, \
					"%s %s: " fmt, \
					__dev->driver->name, __dev->name, \
					##__VA_ARGS__); \
		else \
			dev_printk_emit(LOG_CATEGORY, level, \
					"%s %s: " fmt, \
					"(NULL", "udevice *)", \
					##__VA_ARGS__); \
	} \
})

#define dev_emerg(dev, fmt, ...) \
	__dev_printk(LOGL_EMERG, dev, fmt, ##__VA_ARGS__)
#define dev_alert(dev, fmt, ...) \
	__dev_printk(LOGL_ALERT, dev, fmt, ##__VA_ARGS__)
#define dev_crit(dev, fmt, ...) \
	__dev_printk(LOGL_CRIT, dev, fmt, ##__VA_ARGS__)
#define dev_err(dev, fmt, ...) \
	__dev_printk(LOGL_ERR, dev, fmt, ##__VA_ARGS__)
#define dev_warn(dev, fmt, ...) \
	__dev_printk(LOGL_WARNING, dev, fmt, ##__VA_ARGS__)
#define dev_notice(dev, fmt, ...) \
	__dev_printk(LOGL_NOTICE, dev, fmt, ##__VA_ARGS__)
#define dev_info(dev, fmt, ...) \
	__dev_printk(LOGL_INFO, dev, fmt, ##__VA_ARGS__)
#define dev_dbg(dev, fmt, ...) \
	__dev_printk(LOGL_DEBUG, dev, fmt, ##__VA_ARGS__)
#define dev_vdbg(dev, fmt, ...) \
	__dev_printk(LOGL_DEBUG_CONTENT, dev, fmt, ##__VA_ARGS__)

#endif
