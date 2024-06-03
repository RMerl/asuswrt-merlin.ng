/*
 * Common PSCI functions
 *
 * Copyright (C) 2016 Chen-Yu Tsai
 * Author: Chen-Yu Tsai <wens@csie.org>
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

#include <config.h>
#include <asm/armv7.h>
#include <asm/macro.h>
#include <asm/psci.h>
#include <asm/secure.h>
#include <linux/linkage.h>

static u32 psci_target_pc[CONFIG_ARMV7_PSCI_NR_CPUS] __secure_data = { 0 };
static u32 psci_context_id[CONFIG_ARMV7_PSCI_NR_CPUS] __secure_data = { 0 };

void __secure psci_save(int cpu, u32 pc, u32 context_id)
{
	psci_target_pc[cpu] = pc;
	psci_context_id[cpu] = context_id;
	dsb();
}

u32 __secure psci_get_target_pc(int cpu)
{
	return psci_target_pc[cpu];
}

u32 __secure psci_get_context_id(int cpu)
{
	return psci_context_id[cpu];
}

