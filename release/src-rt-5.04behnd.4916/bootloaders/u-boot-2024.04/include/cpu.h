/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef __CPU_H
#define __CPU_H

#include <linux/types.h>

struct udevice;

/**
 * struct cpu_plat - platform data for a CPU
 * @cpu_id:	   Platform-specific way of identifying the CPU.
 * @ucode_version: Microcode version, if CPU_FEAT_UCODE is set
 * @device_id:     Driver-defined device identifier
 * @family:        DMTF CPU Family identifier
 * @id:            DMTF CPU Processor identifier
 * @timebase_freq: the current frequency at which the cpu timer timebase
 *		   registers are updated (in Hz)
 *
 * This can be accessed with dev_get_parent_plat() for any UCLASS_CPU
 * device.
 */
struct cpu_plat {
	int cpu_id;
	int ucode_version;
	ulong device_id;
	u16 family;
	u32 id[2];
	u32 timebase_freq;
};

/* CPU features - mostly just a placeholder for now */
enum {
	CPU_FEAT_L1_CACHE	= 0,	/* Supports level 1 cache */
	CPU_FEAT_MMU		= 1,	/* Supports virtual memory */
	CPU_FEAT_UCODE		= 2,	/* Requires/uses microcode */
	CPU_FEAT_DEVICE_ID	= 3,	/* Provides a device ID */

	CPU_FEAT_COUNT,
};

/**
 * struct cpu_info - Information about a CPU
 *
 * @cpu_freq:	Current CPU frequency in Hz
 * @features:	Flags for supported CPU features
 * @address_width:	Width of the CPU address space in bits (e.g. 32)
 */
struct cpu_info {
	ulong cpu_freq;
	ulong features;
	uint address_width;
};

struct cpu_ops {
	/**
	 * get_desc() - Get a description string for a CPU
	 *
	 * @dev:	Device to check (UCLASS_CPU)
	 * @buf:	Buffer to place string
	 * @size:	Size of string space
	 * @return 0 if OK, -ENOSPC if buffer is too small, other -ve on error
	 */
	int (*get_desc)(const struct udevice *dev, char *buf, int size);

	/**
	 * get_info() - Get information about a CPU
	 *
	 * @dev:	Device to check (UCLASS_CPU)
	 * @info:	Returns CPU info
	 * @return 0 if OK, -ve on error
	 */
	int (*get_info)(const struct udevice *dev, struct cpu_info *info);

	/**
	 * get_count() - Get number of CPUs
	 *
	 * @dev:	Device to check (UCLASS_CPU)
	 * @return CPU count if OK, -ve on error
	 */
	int (*get_count)(const struct udevice *dev);

	/**
	 * get_vendor() - Get vendor name of a CPU
	 *
	 * @dev:	Device to check (UCLASS_CPU)
	 * @buf:	Buffer to place string
	 * @size:	Size of string space
	 * @return 0 if OK, -ENOSPC if buffer is too small, other -ve on error
	 */
	int (*get_vendor)(const struct udevice *dev, char *buf, int size);

	/**
	 * is_current() - Check if the CPU that U-Boot is currently running from
	 *
	 * @dev:	Device to check (UCLASS_CPU)
	 * @return 1 if the CPU that U-Boot is currently running from, 0
	 *         if not.
	 */
	int (*is_current)(struct udevice *dev);
};

#define cpu_get_ops(dev)        ((struct cpu_ops *)(dev)->driver->ops)

/**
 * cpu_get_desc() - Get a description string for a CPU
 * @dev:	Device to check (UCLASS_CPU)
 * @buf:	Buffer to place string
 * @size:	Size of string space
 *
 * Return: 0 if OK, -ENOSPC if buffer is too small, other -ve on error
 */
int cpu_get_desc(const struct udevice *dev, char *buf, int size);

/**
 * cpu_get_info() - Get information about a CPU
 * @dev:	Device to check (UCLASS_CPU)
 * @info:	Returns CPU info
 *
 * Return: 0 if OK, -ve on error
 */
int cpu_get_info(const struct udevice *dev, struct cpu_info *info);

/**
 * cpu_get_count() - Get number of CPUs
 * @dev:	Device to check (UCLASS_CPU)
 *
 * Return: CPU count if OK, -ve on error
 */
int cpu_get_count(const struct udevice *dev);

/**
 * cpu_get_vendor() - Get vendor name of a CPU
 * @dev:	Device to check (UCLASS_CPU)
 * @buf:	Buffer to place string
 * @size:	Size of string space
 *
 * Return: 0 if OK, -ENOSPC if buffer is too small, other -ve on error
 */
int cpu_get_vendor(const struct udevice *dev, char *buf, int size);

/**
 * cpu_probe_all() - Probe all available CPUs
 *
 * Return: 0 if OK, -ve on error
 */
int cpu_probe_all(void);

/**
 * cpu_is_current() - Check if the CPU that U-Boot is currently running from
 *
 * Return: 1 if yes, - 0 if not
 */
int cpu_is_current(struct udevice *cpu);

/**
 * cpu_get_current_dev() - Get CPU udevice for current CPU
 *
 * Return: udevice if OK, - NULL on error
 */
struct udevice *cpu_get_current_dev(void);

#endif
