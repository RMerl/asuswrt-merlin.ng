/*
 * Copyright (C) 2013 - ARM Ltd
 * Author: Marc Zyngier <marc.zyngier@arm.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <common.h>
#include <errno.h>
#include <stdio_dev.h>
#include <linux/ctype.h>
#include <linux/types.h>
#include <asm/global_data.h>
#include <linux/libfdt.h>
#include <fdt_support.h>
#include <asm/armv7.h>
#include <asm/psci.h>

int armv7_apply_memory_carveout(u64 *start, u64 *size)
{
#ifdef CONFIG_ARMV7_SECURE_RESERVE_SIZE
	if (*start + *size < CONFIG_ARMV7_SECURE_BASE ||
	    *start >= (u64)CONFIG_ARMV7_SECURE_BASE +
		      CONFIG_ARMV7_SECURE_RESERVE_SIZE)
		return 0;

	/* carveout must be at the beginning or the end of the bank */
	if (*start == CONFIG_ARMV7_SECURE_BASE ||
	    *start + *size == (u64)CONFIG_ARMV7_SECURE_BASE +
			      CONFIG_ARMV7_SECURE_RESERVE_SIZE) {
		if (*size < CONFIG_ARMV7_SECURE_RESERVE_SIZE) {
			debug("Secure monitor larger than RAM bank!?\n");
			return -EINVAL;
		}
		*size -= CONFIG_ARMV7_SECURE_RESERVE_SIZE;
		if (*start == CONFIG_ARMV7_SECURE_BASE)
			*start += CONFIG_ARMV7_SECURE_RESERVE_SIZE;
		return 0;
	}
	debug("Secure monitor not located at beginning or end of RAM bank\n");
	return -EINVAL;
#else /* !CONFIG_ARMV7_SECURE_RESERVE_SIZE */
	return 0;
#endif
}

int psci_update_dt(void *fdt)
{
#ifdef CONFIG_ARMV7_NONSEC
	if (!armv7_boot_nonsec())
		return 0;
#endif
#ifndef CONFIG_ARMV7_SECURE_BASE
	/* secure code lives in RAM, keep it alive */
	fdt_add_mem_rsv(fdt, (unsigned long)__secure_start,
			__secure_end - __secure_start);
#endif

	return fdt_psci(fdt);
}
