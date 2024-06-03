// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015, Bin Meng <bmeng.cn@gmail.com>
 *
 * Adapted from coreboot src/arch/x86/boot/mpspec.c
 */

#include <common.h>
#include <cpu.h>
#include <dm.h>
#include <errno.h>
#include <fdtdec.h>
#include <asm/cpu.h>
#include <asm/irq.h>
#include <asm/ioapic.h>
#include <asm/lapic.h>
#include <asm/mpspec.h>
#include <asm/tables.h>
#include <dm/uclass-internal.h>

DECLARE_GLOBAL_DATA_PTR;

static bool isa_irq_occupied[16];

struct mp_config_table *mp_write_floating_table(struct mp_floating_table *mf)
{
	ulong mc;

	memcpy(mf->mpf_signature, MPF_SIGNATURE, 4);
	mf->mpf_physptr = (ulong)mf + sizeof(struct mp_floating_table);
	mf->mpf_length = 1;
	mf->mpf_spec = MPSPEC_V14;
	mf->mpf_checksum = 0;
	/* We don't use the default configuration table */
	mf->mpf_feature1 = 0;
	/* Indicate that virtual wire mode is always implemented */
	mf->mpf_feature2 = 0;
	mf->mpf_feature3 = 0;
	mf->mpf_feature4 = 0;
	mf->mpf_feature5 = 0;
	mf->mpf_checksum = table_compute_checksum(mf, mf->mpf_length * 16);

	mc = (ulong)mf + sizeof(struct mp_floating_table);
	return (struct mp_config_table *)mc;
}

void mp_config_table_init(struct mp_config_table *mc)
{
	memcpy(mc->mpc_signature, MPC_SIGNATURE, 4);
	mc->mpc_length = sizeof(struct mp_config_table);
	mc->mpc_spec = MPSPEC_V14;
	mc->mpc_checksum = 0;
	mc->mpc_oemptr = 0;
	mc->mpc_oemsize = 0;
	mc->mpc_entry_count = 0;
	mc->mpc_lapic = LAPIC_DEFAULT_BASE;
	mc->mpe_length = 0;
	mc->mpe_checksum = 0;
	mc->reserved = 0;

	/* The oem/product id fields are exactly 8/12 bytes long */
	table_fill_string(mc->mpc_oem, CONFIG_SYS_VENDOR, 8, ' ');
	table_fill_string(mc->mpc_product, CONFIG_SYS_BOARD, 12, ' ');
}

void mp_write_processor(struct mp_config_table *mc)
{
	struct mpc_config_processor *mpc;
	struct udevice *dev;
	u8 boot_apicid, apicver;
	u32 cpusignature, cpufeature;
	struct cpuid_result result;

	boot_apicid = lapicid();
	apicver = lapic_read(LAPIC_LVR) & 0xff;
	result = cpuid(1);
	cpusignature = result.eax;
	cpufeature = result.edx;

	for (uclass_find_first_device(UCLASS_CPU, &dev);
	     dev;
	     uclass_find_next_device(&dev)) {
		struct cpu_platdata *plat = dev_get_parent_platdata(dev);
		u8 cpuflag = MPC_CPU_EN;

		if (!device_active(dev))
			continue;

		mpc = (struct mpc_config_processor *)mp_next_mpc_entry(mc);
		mpc->mpc_type = MP_PROCESSOR;
		mpc->mpc_apicid = plat->cpu_id;
		mpc->mpc_apicver = apicver;
		if (boot_apicid == plat->cpu_id)
			cpuflag |= MPC_CPU_BP;
		mpc->mpc_cpuflag = cpuflag;
		mpc->mpc_cpusignature = cpusignature;
		mpc->mpc_cpufeature = cpufeature;
		mpc->mpc_reserved[0] = 0;
		mpc->mpc_reserved[1] = 0;
		mp_add_mpc_entry(mc, sizeof(*mpc));
	}
}

void mp_write_bus(struct mp_config_table *mc, int id, const char *bustype)
{
	struct mpc_config_bus *mpc;

	mpc = (struct mpc_config_bus *)mp_next_mpc_entry(mc);
	mpc->mpc_type = MP_BUS;
	mpc->mpc_busid = id;
	memcpy(mpc->mpc_bustype, bustype, 6);
	mp_add_mpc_entry(mc, sizeof(*mpc));
}

void mp_write_ioapic(struct mp_config_table *mc, int id, int ver, u32 apicaddr)
{
	struct mpc_config_ioapic *mpc;

	mpc = (struct mpc_config_ioapic *)mp_next_mpc_entry(mc);
	mpc->mpc_type = MP_IOAPIC;
	mpc->mpc_apicid = id;
	mpc->mpc_apicver = ver;
	mpc->mpc_flags = MPC_APIC_USABLE;
	mpc->mpc_apicaddr = apicaddr;
	mp_add_mpc_entry(mc, sizeof(*mpc));
}

void mp_write_intsrc(struct mp_config_table *mc, int irqtype, int irqflag,
		     int srcbus, int srcbusirq, int dstapic, int dstirq)
{
	struct mpc_config_intsrc *mpc;

	mpc = (struct mpc_config_intsrc *)mp_next_mpc_entry(mc);
	mpc->mpc_type = MP_INTSRC;
	mpc->mpc_irqtype = irqtype;
	mpc->mpc_irqflag = irqflag;
	mpc->mpc_srcbus = srcbus;
	mpc->mpc_srcbusirq = srcbusirq;
	mpc->mpc_dstapic = dstapic;
	mpc->mpc_dstirq = dstirq;
	mp_add_mpc_entry(mc, sizeof(*mpc));
}

void mp_write_pci_intsrc(struct mp_config_table *mc, int irqtype,
			 int srcbus, int dev, int pin, int dstapic, int dstirq)
{
	u8 srcbusirq = (dev << 2) | (pin - 1);

	mp_write_intsrc(mc, irqtype, MP_IRQ_TRIGGER_LEVEL | MP_IRQ_POLARITY_LOW,
			srcbus, srcbusirq, dstapic, dstirq);
}

void mp_write_lintsrc(struct mp_config_table *mc, int irqtype, int irqflag,
		      int srcbus, int srcbusirq, int destapic, int destlint)
{
	struct mpc_config_lintsrc *mpc;

	mpc = (struct mpc_config_lintsrc *)mp_next_mpc_entry(mc);
	mpc->mpc_type = MP_LINTSRC;
	mpc->mpc_irqtype = irqtype;
	mpc->mpc_irqflag = irqflag;
	mpc->mpc_srcbusid = srcbus;
	mpc->mpc_srcbusirq = srcbusirq;
	mpc->mpc_destapic = destapic;
	mpc->mpc_destlint = destlint;
	mp_add_mpc_entry(mc, sizeof(*mpc));
}

void mp_write_address_space(struct mp_config_table *mc,
			    int busid, int addr_type,
			    u32 addr_base_low, u32 addr_base_high,
			    u32 addr_length_low, u32 addr_length_high)
{
	struct mp_ext_system_address_space *mpe;

	mpe = (struct mp_ext_system_address_space *)mp_next_mpe_entry(mc);
	mpe->mpe_type = MPE_SYSTEM_ADDRESS_SPACE;
	mpe->mpe_length = sizeof(*mpe);
	mpe->mpe_busid = busid;
	mpe->mpe_addr_type = addr_type;
	mpe->mpe_addr_base_low = addr_base_low;
	mpe->mpe_addr_base_high = addr_base_high;
	mpe->mpe_addr_length_low = addr_length_low;
	mpe->mpe_addr_length_high = addr_length_high;
	mp_add_mpe_entry(mc, (struct mp_ext_config *)mpe);
}

void mp_write_bus_hierarchy(struct mp_config_table *mc,
			    int busid, int bus_info, int parent_busid)
{
	struct mp_ext_bus_hierarchy *mpe;

	mpe = (struct mp_ext_bus_hierarchy *)mp_next_mpe_entry(mc);
	mpe->mpe_type = MPE_BUS_HIERARCHY;
	mpe->mpe_length = sizeof(*mpe);
	mpe->mpe_busid = busid;
	mpe->mpe_bus_info = bus_info;
	mpe->mpe_parent_busid = parent_busid;
	mpe->reserved[0] = 0;
	mpe->reserved[1] = 0;
	mpe->reserved[2] = 0;
	mp_add_mpe_entry(mc, (struct mp_ext_config *)mpe);
}

void mp_write_compat_address_space(struct mp_config_table *mc, int busid,
				   int addr_modifier, u32 range_list)
{
	struct mp_ext_compat_address_space *mpe;

	mpe = (struct mp_ext_compat_address_space *)mp_next_mpe_entry(mc);
	mpe->mpe_type = MPE_COMPAT_ADDRESS_SPACE;
	mpe->mpe_length = sizeof(*mpe);
	mpe->mpe_busid = busid;
	mpe->mpe_addr_modifier = addr_modifier;
	mpe->mpe_range_list = range_list;
	mp_add_mpe_entry(mc, (struct mp_ext_config *)mpe);
}

u32 mptable_finalize(struct mp_config_table *mc)
{
	ulong end;

	mc->mpe_checksum = table_compute_checksum((void *)mp_next_mpc_entry(mc),
						  mc->mpe_length);
	mc->mpc_checksum = table_compute_checksum(mc, mc->mpc_length);
	end = mp_next_mpe_entry(mc);

	debug("Write the MP table at: %lx - %lx\n", (ulong)mc, end);

	return end;
}

static void mptable_add_isa_interrupts(struct mp_config_table *mc, int bus_isa,
				       int apicid, int external_int2)
{
	int i;

	mp_write_intsrc(mc, external_int2 ? MP_INT : MP_EXTINT,
			MP_IRQ_TRIGGER_EDGE | MP_IRQ_POLARITY_HIGH,
			bus_isa, 0, apicid, 0);
	mp_write_intsrc(mc, MP_INT, MP_IRQ_TRIGGER_EDGE | MP_IRQ_POLARITY_HIGH,
			bus_isa, 1, apicid, 1);
	mp_write_intsrc(mc, external_int2 ? MP_EXTINT : MP_INT,
			MP_IRQ_TRIGGER_EDGE | MP_IRQ_POLARITY_HIGH,
			bus_isa, 0, apicid, 2);

	for (i = 3; i < 16; i++) {
		/*
		 * Do not write ISA interrupt entry if it is already occupied
		 * by the platform devices.
		 */
		if (isa_irq_occupied[i])
			continue;

		mp_write_intsrc(mc, MP_INT,
				MP_IRQ_TRIGGER_EDGE | MP_IRQ_POLARITY_HIGH,
				bus_isa, i, apicid, i);
	}
}

/*
 * Check duplicated I/O interrupt assignment table entry, to make sure
 * there is only one entry with the given bus, device and interrupt pin.
 */
static bool check_dup_entry(struct mpc_config_intsrc *intsrc_base,
			    int entry_num, int bus, int device, int pin)
{
	struct mpc_config_intsrc *intsrc = intsrc_base;
	int i;

	for (i = 0; i < entry_num; i++) {
		if (intsrc->mpc_srcbus == bus &&
		    intsrc->mpc_srcbusirq == ((device << 2) | (pin - 1)))
			break;
		intsrc++;
	}

	return (i == entry_num) ? false : true;
}

/* TODO: move this to driver model */
__weak int mp_determine_pci_dstirq(int bus, int dev, int func, int pirq)
{
	/* PIRQ[A-H] are connected to I/O APIC INTPIN#16-23 */
	return pirq + 16;
}

static int mptable_add_intsrc(struct mp_config_table *mc,
			      int bus_isa, int apicid)
{
	struct mpc_config_intsrc *intsrc_base;
	int intsrc_entries = 0;
	const void *blob = gd->fdt_blob;
	struct udevice *dev;
	int len, count;
	const u32 *cell;
	int i, ret;

	ret = uclass_first_device_err(UCLASS_IRQ, &dev);
	if (ret && ret != -ENODEV) {
		debug("%s: Cannot find irq router node\n", __func__);
		return ret;
	}

	/* Get I/O interrupt information from device tree */
	cell = fdt_getprop(blob, dev_of_offset(dev), "intel,pirq-routing",
			   &len);
	if (!cell)
		return -ENOENT;

	if ((len % sizeof(struct pirq_routing)) == 0)
		count = len / sizeof(struct pirq_routing);
	else
		return -EINVAL;

	intsrc_base = (struct mpc_config_intsrc *)mp_next_mpc_entry(mc);

	for (i = 0; i < count; i++) {
		struct pirq_routing pr;
		int bus, dev, func;
		int dstirq;

		pr.bdf = fdt_addr_to_cpu(cell[0]);
		pr.pin = fdt_addr_to_cpu(cell[1]);
		pr.pirq = fdt_addr_to_cpu(cell[2]);
		bus = PCI_BUS(pr.bdf);
		dev = PCI_DEV(pr.bdf);
		func = PCI_FUNC(pr.bdf);

		if (check_dup_entry(intsrc_base, intsrc_entries,
				    bus, dev, pr.pin)) {
			debug("found entry for bus %d device %d INT%c, skipping\n",
			      bus, dev, 'A' + pr.pin - 1);
			cell += sizeof(struct pirq_routing) / sizeof(u32);
			continue;
		}

		dstirq = mp_determine_pci_dstirq(bus, dev, func, pr.pirq);
		/*
		 * For PIRQ which is connected to I/O APIC interrupt pin#0-15,
		 * mark it as occupied so that we can skip it later.
		 */
		if (dstirq < 16)
			isa_irq_occupied[dstirq] = true;
		mp_write_pci_intsrc(mc, MP_INT, bus, dev, pr.pin,
				    apicid, dstirq);
		intsrc_entries++;
		cell += sizeof(struct pirq_routing) / sizeof(u32);
	}

	/* Legacy Interrupts */
	debug("Writing ISA IRQs\n");
	mptable_add_isa_interrupts(mc, bus_isa, apicid, 0);

	return 0;
}

static void mptable_add_lintsrc(struct mp_config_table *mc, int bus_isa)
{
	mp_write_lintsrc(mc, MP_EXTINT,
			 MP_IRQ_TRIGGER_EDGE | MP_IRQ_POLARITY_HIGH,
			 bus_isa, 0, MP_APIC_ALL, 0);
	mp_write_lintsrc(mc, MP_NMI,
			 MP_IRQ_TRIGGER_EDGE | MP_IRQ_POLARITY_HIGH,
			 bus_isa, 0, MP_APIC_ALL, 1);
}

ulong write_mp_table(ulong addr)
{
	struct mp_config_table *mc;
	int ioapic_id, ioapic_ver;
	int bus_isa = 0xff;
	int ret;
	ulong end;

	/* 16 byte align the table address */
	addr = ALIGN(addr, 16);

	/* Write floating table */
	mc = mp_write_floating_table((struct mp_floating_table *)addr);

	/* Write configuration table header */
	mp_config_table_init(mc);

	/* Write processor entry */
	mp_write_processor(mc);

	/* Write bus entry */
	mp_write_bus(mc, bus_isa, BUSTYPE_ISA);

	/* Write I/O APIC entry */
	ioapic_id = io_apic_read(IO_APIC_ID) >> 24;
	ioapic_ver = io_apic_read(IO_APIC_VER) & 0xff;
	mp_write_ioapic(mc, ioapic_id, ioapic_ver, IO_APIC_ADDR);

	/* Write I/O interrupt assignment entry */
	ret = mptable_add_intsrc(mc, bus_isa, ioapic_id);
	if (ret)
		debug("Failed to write I/O interrupt assignment table\n");

	/* Write local interrupt assignment entry */
	mptable_add_lintsrc(mc, bus_isa);

	/* Finalize the MP table */
	end = mptable_finalize(mc);

	return end;
}
