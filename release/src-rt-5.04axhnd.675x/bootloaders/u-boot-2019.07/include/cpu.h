/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef __CPU_H
#define __CPU_H

/**
 * struct cpu_platdata - platform data for a CPU
 * @cpu_id:	   Platform-specific way of identifying the CPU.
 * @ucode_version: Microcode version, if CPU_FEAT_UCODE is set
 * @device_id:     Driver-defined device identifier
 * @family:        DMTF CPU Family identifier
 * @id:            DMTF CPU Processor identifier
 * @timebase_freq: the current frequency at which the cpu timer timebase
 *		   registers are updated (in Hz)
 *
 * This can be accessed with dev_get_parent_platdata() for any UCLASS_CPU
 * device.
 */
struct cpu_platdata {
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
 */
struct cpu_info {
	ulong cpu_freq;
	ulong features;
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
	int (*get_desc)(struct udevice *dev, char *buf, int size);

	/**
	 * get_info() - Get information about a CPU
	 *
	 * @dev:	Device to check (UCLASS_CPU)
	 * @info:	Returns CPU info
	 * @return 0 if OK, -ve on error
	 */
	int (*get_info)(struct udevice *dev, struct cpu_info *info);

	/**
	 * get_count() - Get number of CPUs
	 *
	 * @dev:	Device to check (UCLASS_CPU)
	 * @return CPU count if OK, -ve on error
	 */
	int (*get_count)(struct udevice *dev);

	/**
	 * get_vendor() - Get vendor name of a CPU
	 *
	 * @dev:	Device to check (UCLASS_CPU)
	 * @buf:	Buffer to place string
	 * @size:	Size of string space
	 * @return 0 if OK, -ENOSPC if buffer is too small, other -ve on error
	 */
	int (*get_vendor)(struct udevice *dev, char *buf, int size);
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
int cpu_get_desc(struct udevice *dev, char *buf, int size);

/**
 * cpu_get_info() - Get information about a CPU
 * @dev:	Device to check (UCLASS_CPU)
 * @info:	Returns CPU info
 *
 * Return: 0 if OK, -ve on error
 */
int cpu_get_info(struct udevice *dev, struct cpu_info *info);

/**
 * cpu_get_count() - Get number of CPUs
 * @dev:	Device to check (UCLASS_CPU)
 *
 * Return: CPU count if OK, -ve on error
 */
int cpu_get_count(struct udevice *dev);

/**
 * cpu_get_vendor() - Get vendor name of a CPU
 * @dev:	Device to check (UCLASS_CPU)
 * @buf:	Buffer to place string
 * @size:	Size of string space
 *
 * Return: 0 if OK, -ENOSPC if buffer is too small, other -ve on error
 */
int cpu_get_vendor(struct udevice *dev, char *buf, int size);

/**
 * cpu_probe_all() - Probe all available CPUs
 *
 * Return: 0 if OK, -ve on error
 */
int cpu_probe_all(void);

#endif
