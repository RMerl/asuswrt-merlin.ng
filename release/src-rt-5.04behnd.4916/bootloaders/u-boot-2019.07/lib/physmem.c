/*
 * Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 */

#include <common.h>
#include <mapmem.h>
#include <physmem.h>
#include <linux/compiler.h>

phys_addr_t __weak arch_phys_memset(phys_addr_t s, int c, phys_size_t n)
{
	void *s_ptr = map_sysmem(s, n);

	assert(((phys_addr_t)(uintptr_t)s) == s);
	assert(((phys_addr_t)(uintptr_t)(s + n)) == s + n);

	return (phys_addr_t)(uintptr_t)memset(s_ptr, c, n);
}
