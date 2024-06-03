/*
 * Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 */

/*
 * These functions work like memset but operate on physical memory which may
 * not be accessible directly.
 *
 * @param s	The physical address to start setting memory at.
 * @param c	The character to set each byte of the region to.
 * @param n	The number of bytes to set.
 *
 * @return	The physical address of the memory which was set.
 */
phys_addr_t arch_phys_memset(phys_addr_t s, int c, phys_size_t n);
