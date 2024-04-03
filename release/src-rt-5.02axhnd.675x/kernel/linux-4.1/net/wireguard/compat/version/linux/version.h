/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2015-2021 Jason A. Donenfeld <Jason@zx2c4.com>. All Rights Reserved.
 */

#include_next <linux/version.h>
#undef KERNEL_VERSION
#define KERNEL_VERSION(a, b, c) (((a) << 24) + ((b) << 16) + (c))
#undef LINUX_VERSION_CODE
#define LINUX_VERSION_CODE KERNEL_VERSION(COMPAT_VERSION, COMPAT_PATCHLEVEL, COMPAT_SUBLEVEL)
