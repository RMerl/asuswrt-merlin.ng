/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2015, Bin Meng <bmeng.cn@gmail.com>
 */

#ifndef _ASM_CPU_X86_H
#define _ASM_CPU_X86_H

/**
 * cpu_x86_bind() - Bind an x86 CPU with the driver
 *
 * This updates cpu device's platform data with information from device tree,
 * like the processor local apic id.
 *
 * @dev:	Device to check (UCLASS_CPU)
 * @return	0 always
 */
int cpu_x86_bind(struct udevice *dev);

/**
 * cpu_x86_get_desc() - Get a description string for an x86 CPU
 *
 * This uses cpu_get_name() and is suitable to use as the get_desc() method for
 * the CPU uclass.
 *
 * @dev:	Device to check (UCLASS_CPU)
 * @buf:	Buffer to place string
 * @size:	Size of string space
 * @return:	0 if OK, -ENOSPC if buffer is too small, other -ve on error
 */
int cpu_x86_get_desc(struct udevice *dev, char *buf, int size);

/**
 * cpu_x86_get_vendor() - Get a vendor string for an x86 CPU
 *
 * This uses cpu_vendor_name() and is suitable to use as the get_vendor()
 * method for the CPU uclass.
 *
 * @dev:	Device to check (UCLASS_CPU)
 * @buf:	Buffer to place string
 * @size:	Size of string space
 * @return:	0 if OK, -ENOSPC if buffer is too small, other -ve on error
 */
int cpu_x86_get_vendor(struct udevice *dev, char *buf, int size);

#endif /* _ASM_CPU_X86_H */
