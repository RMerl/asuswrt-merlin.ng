// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015, Bin Meng <bmeng.cn@gmail.com>
 *
 * Part of this file is ported from coreboot src/arch/x86/boot/pirq_routing.c
 */

#include <common.h>
#include <log.h>
#include <pci.h>
#include <asm/global_data.h>
#include <asm/pci.h>
#include <asm/pirq_routing.h>

DECLARE_GLOBAL_DATA_PTR;

static u8 pirq_get_next_free_irq(struct udevice *dev, u8 *pirq, u16 bitmap,
				 bool irq_already_routed[])
{
	int i, link;
	u8 irq = 0;

	/* IRQ sharing starts from IRQ#3 */
	for (i = 3; i < 16; i++) {
		/* Can we assign this IRQ? */
		if (!((bitmap >> i) & 1))
			continue;

		/* We can, now let's assume we can use this IRQ */
		irq = i;

		/* Have we already routed it? */
		if (irq_already_routed[irq])
			continue;

		for (link = 0; link < CONFIG_MAX_PIRQ_LINKS; link++) {
			if (pirq_check_irq_routed(dev, link, irq)) {
				irq_already_routed[irq] = true;
				break;
			}
		}

		/* If it's not yet routed, use it */
		if (!irq_already_routed[irq]) {
			irq_already_routed[irq] = true;
			break;
		}

		/* But if it was already routed, try the next one */
	}

	/* Now we get our IRQ */
	return irq;
}

void pirq_route_irqs(struct udevice *dev, struct irq_info *irq, int num)
{
	unsigned char irq_slot[MAX_INTX_ENTRIES];
	unsigned char pirq[CONFIG_MAX_PIRQ_LINKS];
	bool irq_already_routed[16];
	int i, intx;

	memset(pirq, 0, CONFIG_MAX_PIRQ_LINKS);
	memset(irq_already_routed, '\0', sizeof(irq_already_routed));

	/* Set PCI IRQs */
	for (i = 0; i < num; i++) {
		debug("PIRQ Entry %d Dev: %d.%x.%d\n", i,
		      irq->bus, irq->devfn >> 3, irq->devfn & 7);

		for (intx = 0; intx < MAX_INTX_ENTRIES; intx++) {
			int link = irq->irq[intx].link;
			int bitmap = irq->irq[intx].bitmap;
			int irq = 0;

			debug("INT%c link: %x bitmap: %x ",
			      'A' + intx, link, bitmap);

			if (!bitmap || !link) {
				debug("not routed\n");
				irq_slot[intx] = irq;
				continue;
			}

			/* translate link value to link number */
			link = pirq_translate_link(dev, link);

			/* yet not routed */
			if (!pirq[link]) {
				irq = pirq_get_next_free_irq(dev, pirq, bitmap,
						irq_already_routed);
				pirq[link] = irq;
			} else {
				irq = pirq[link];
			}

			debug("IRQ: %d\n", irq);
			irq_slot[intx] = irq;

			/* Assign IRQ in the interrupt router */
			pirq_assign_irq(dev, link, irq);
		}

		/* Bus, device, slots IRQs for {A,B,C,D} */
		pci_assign_irqs(irq->bus, irq->devfn >> 3, irq_slot);

		irq++;
	}

	for (i = 0; i < CONFIG_MAX_PIRQ_LINKS; i++)
		debug("PIRQ%c: %d\n", 'A' + i, pirq[i]);
}

u32 copy_pirq_routing_table(u32 addr, struct irq_routing_table *rt)
{
	struct irq_routing_table *rom_rt;

	/* Align the table to be 16 byte aligned */
	addr = ALIGN(addr, 16);

	debug("Copying Interrupt Routing Table to 0x%x\n", addr);
	memcpy((void *)(uintptr_t)addr, rt, rt->size);

	/*
	 * We do the sanity check here against the copied table after memcpy,
	 * as something might go wrong after the memcpy, which is normally
	 * due to the F segment decode is not turned on to systeam RAM.
	 */
	rom_rt = (struct irq_routing_table *)(uintptr_t)addr;
	if (rom_rt->signature != PIRQ_SIGNATURE ||
	    rom_rt->version != PIRQ_VERSION || rom_rt->size % 16) {
		printf("Interrupt Routing Table not valid\n");
		return addr;
	}

	return addr + rt->size;
}

ulong write_pirq_routing_table(ulong addr)
{
	if (!gd->arch.pirq_routing_table)
		return addr;

	return copy_pirq_routing_table(addr, gd->arch.pirq_routing_table);
}
