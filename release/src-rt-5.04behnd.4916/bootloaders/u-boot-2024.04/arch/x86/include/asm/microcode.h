/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2015 Google, Inc
 */

#ifndef __ASM_ARCH_MICROCODE_H
#define __ASM_ARCH_MICROCODE_H

#ifndef __ASSEMBLY__

/* This is a declaration for ucode_base in start.S */
extern u32 ucode_base;
extern u32 ucode_size;

/**
 * microcode_update_intel() - Apply microcode updates
 *
 * Applies any microcode updates in the device tree.
 *
 * Return: 0 if OK, -EEXIST if the updates were already applied, -ENOENT if
 * not updates were found, -EINVAL if an update was invalid
 */
int microcode_update_intel(void);

/**
 * microcode_read_rev() - Read the microcode version
 *
 * This reads the microcode version of the currently running CPU
 *
 * Return: microcode version number
 */
int microcode_read_rev(void);
#endif /* __ASSEMBLY__ */

#endif
