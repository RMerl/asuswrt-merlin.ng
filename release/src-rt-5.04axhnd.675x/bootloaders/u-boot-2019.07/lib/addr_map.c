// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2008 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <addr_map.h>

static struct {
	phys_addr_t paddr;
	phys_size_t size;
	unsigned long vaddr;
} address_map[CONFIG_SYS_NUM_ADDR_MAP];

phys_addr_t addrmap_virt_to_phys(void * vaddr)
{
	int i;

	for (i = 0; i < CONFIG_SYS_NUM_ADDR_MAP; i++) {
		u64 base, upper, addr;

		if (address_map[i].size == 0)
			continue;

		addr = (u64)((u32)vaddr);
		base = (u64)(address_map[i].vaddr);
		upper = (u64)(address_map[i].size) + base - 1;

		if (addr >= base && addr <= upper) {
			return addr - address_map[i].vaddr + address_map[i].paddr;
		}
	}

	return (phys_addr_t)(~0);
}

void *addrmap_phys_to_virt(phys_addr_t paddr)
{
	int i;

	for (i = 0; i < CONFIG_SYS_NUM_ADDR_MAP; i++) {
		phys_addr_t base, upper;

		if (address_map[i].size == 0)
			continue;

		base = address_map[i].paddr;
		upper = address_map[i].size + base - 1;

		if (paddr >= base && paddr <= upper) {
			phys_addr_t offset;

			offset = address_map[i].paddr - address_map[i].vaddr;

			return (void *)(unsigned long)(paddr - offset);
		}
	}

	return (void *)(~0);
}

void addrmap_set_entry(unsigned long vaddr, phys_addr_t paddr,
			phys_size_t size, int idx)
{
	if (idx > CONFIG_SYS_NUM_ADDR_MAP)
		return;

	address_map[idx].vaddr = vaddr;
	address_map[idx].paddr = paddr;
	address_map[idx].size  = size;
}
