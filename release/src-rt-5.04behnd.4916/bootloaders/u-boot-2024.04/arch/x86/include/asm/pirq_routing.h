/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2015, Bin Meng <bmeng.cn@gmail.com>
 *
 * Ported from coreboot src/arch/x86/include/arch/pirq_routing.h
 */

#ifndef _PIRQ_ROUTING_H_
#define _PIRQ_ROUTING_H_

/*
 * This is the maximum number on interrupt entries that a PCI device may have.
 *   This is NOT the number of slots or devices in the system
 *   This is NOT the number of entries in the PIRQ table
 *
 * This tells us that in the PIRQ table, we are going to have 4 link-bitmap
 * entries per PCI device which is fixed at 4: INTA, INTB, INTC, and INTD.
 *
 * CAUTION: If you change this, PIRQ routing will not work correctly.
 */
#define MAX_INTX_ENTRIES	4

#define PIRQ_SIGNATURE		\
	(('$' << 0) + ('P' << 8) + ('I' << 16) + ('R' << 24))
#define PIRQ_VERSION		0x0100

struct __packed irq_info {
	u8 bus;			/* Bus number */
	u8 devfn;		/* Device and function number */
	struct __packed {
		u8 link;	/* IRQ line ID, 0=not routed */
		u16 bitmap;	/* Available IRQs */
	} irq[MAX_INTX_ENTRIES];
	u8 slot;		/* Slot number, 0=onboard */
	u8 rfu;
};

struct __packed irq_routing_table {
	u32 signature;		/* PIRQ_SIGNATURE */
	u16 version;		/* PIRQ_VERSION */
	u16 size;		/* Table size in bytes */
	u8 rtr_bus;		/* busno of the interrupt router */
	u8 rtr_devfn;		/* devfn of the interrupt router */
	u16 exclusive_irqs;	/* IRQs devoted exclusively to PCI usage */
	u16 rtr_vendor;		/* Vendor ID of the interrupt router */
	u16 rtr_device;		/* Device ID of the interrupt router */
	u32 miniport_data;
	u8 rfu[11];
	u8 checksum;		/* Modulo 256 checksum must give zero */
	struct irq_info slots[CONFIG_IRQ_SLOT_COUNT];
};

/**
 * get_irq_slot_count() - Get the number of entries in the irq_info table
 *
 * This calculates the number of entries for the irq_info table.
 *
 * @rt:		pointer to the base address of the struct irq_info
 * @return:	number of entries
 */
static inline int get_irq_slot_count(struct irq_routing_table *rt)
{
	return (rt->size - 32) / sizeof(struct irq_info);
}

/**
 * pirq_check_irq_routed() - Check whether an IRQ is routed to 8259 PIC
 *
 * This function checks whether an IRQ is routed to 8259 PIC for a given link.
 *
 * Note: this function should be provided by the platform codes, as the
 * implementation of interrupt router may be different.
 *
 * @dev:	irq router's udevice
 * @link:	link number which represents a PIRQ
 * @irq:	the 8259 IRQ number
 * @return:	true if the irq is already routed to 8259 for a given link,
 *		false elsewise
 */
bool pirq_check_irq_routed(struct udevice *dev, int link, u8 irq);

/**
 * pirq_translate_link() - Translate a link value
 *
 * This function translates a platform-specific link value to a link number.
 * On Intel platforms, the link value is normally a offset into the PCI
 * configuration space into the legacy bridge.
 *
 * Note: this function should be provided by the platform codes, as the
 * implementation of interrupt router may be different.
 *
 * @dev:	irq router's udevice
 * @link:	platform-specific link value
 * @return:	link number which represents a PIRQ
 */
int pirq_translate_link(struct udevice *dev, int link);

/**
 * pirq_assign_irq() - Assign an IRQ to a PIRQ link
 *
 * This function assigns the IRQ to a PIRQ link so that the PIRQ is routed to
 * the 8259 PIC.
 *
 * Note: this function should be provided by the platform codes, as the
 * implementation of interrupt router may be different.
 *
 * @dev:	irq router's udevice
 * @link:	link number which represents a PIRQ
 * @irq:	IRQ to which the PIRQ is routed
 */
void pirq_assign_irq(struct udevice *dev, int link, u8 irq);

/**
 * pirq_route_irqs() - Route PIRQs to 8259 PIC
 *
 * This function configures all PCI devices' interrupt pins and maps them to
 * PIRQs and finally 8259 PIC. The routed irq number is written to interrupt
 * line register in the configuration space of the PCI device for OS to use.
 * The configuration source is taken from a struct irq_info table, the format
 * of which is defined in PIRQ routing table spec and PCI BIOS spec.
 *
 * @dev:	irq router's udevice
 * @irq:	pointer to the base address of the struct irq_info
 * @num:	number of entries in the struct irq_info
 */
void pirq_route_irqs(struct udevice *dev, struct irq_info *irq, int num);

/**
 * copy_pirq_routing_table() - Copy a PIRQ routing table
 *
 * This helper function copies the given PIRQ routing table to a given address.
 * Before copying, it does several sanity tests against the PIRQ routing table.
 * It also fixes up the table checksum and align the given address to a 16 byte
 * boundary to meet the PIRQ routing table spec requirements.
 *
 * @addr:	address to store the copied PIRQ routing table
 * @rt:		pointer to the PIRQ routing table to copy from
 * @return:	end address of the copied PIRQ routing table
 */
u32 copy_pirq_routing_table(u32 addr, struct irq_routing_table *rt);

#endif /* _PIRQ_ROUTING_H_ */
