// SPDX-License-Identifier: GPL-2.0+
/*
 * Based on acpi.c from coreboot
 *
 * Copyright (C) 2015, Saket Sinha <saket.sinha89@gmail.com>
 * Copyright (C) 2016, Bin Meng <bmeng.cn@gmail.com>
 */

#define LOG_CATEGORY LOGC_ACPI

#include <common.h>
#include <bloblist.h>
#include <cpu.h>
#include <dm.h>
#include <log.h>
#include <dm/uclass-internal.h>
#include <mapmem.h>
#include <serial.h>
#include <acpi/acpigen.h>
#include <acpi/acpi_device.h>
#include <acpi/acpi_table.h>
#include <asm/acpi/global_nvs.h>
#include <asm/ioapic.h>
#include <asm/global_data.h>
#include <asm/lapic.h>
#include <asm/mpspec.h>
#include <asm/tables.h>
#include <asm/arch/global_nvs.h>
#include <dm/acpi.h>
#include <linux/err.h>

static int acpi_create_madt_lapic(struct acpi_madt_lapic *lapic,
				  u8 cpu, u8 apic)
{
	lapic->type = ACPI_APIC_LAPIC;
	lapic->length = sizeof(struct acpi_madt_lapic);
	lapic->flags = LOCAL_APIC_FLAG_ENABLED;
	lapic->processor_id = cpu;
	lapic->apic_id = apic;

	return lapic->length;
}

int acpi_create_madt_lapics(u32 current)
{
	struct udevice *dev;
	int total_length = 0;
	int cpu_num = 0;

	for (uclass_find_first_device(UCLASS_CPU, &dev);
	     dev;
	     uclass_find_next_device(&dev)) {
		struct cpu_plat *plat = dev_get_parent_plat(dev);
		int length;

		length = acpi_create_madt_lapic(
			(struct acpi_madt_lapic *)current, cpu_num++,
			plat->cpu_id);
		current += length;
		total_length += length;
	}

	return total_length;
}

int acpi_create_madt_ioapic(struct acpi_madt_ioapic *ioapic, u8 id,
			    u32 addr, u32 gsi_base)
{
	ioapic->type = ACPI_APIC_IOAPIC;
	ioapic->length = sizeof(struct acpi_madt_ioapic);
	ioapic->reserved = 0x00;
	ioapic->gsi_base = gsi_base;
	ioapic->ioapic_id = id;
	ioapic->ioapic_addr = addr;

	return ioapic->length;
}

int acpi_create_madt_irqoverride(struct acpi_madt_irqoverride *irqoverride,
				 u8 bus, u8 source, u32 gsirq, u16 flags)
{
	irqoverride->type = ACPI_APIC_IRQ_SRC_OVERRIDE;
	irqoverride->length = sizeof(struct acpi_madt_irqoverride);
	irqoverride->bus = bus;
	irqoverride->source = source;
	irqoverride->gsirq = gsirq;
	irqoverride->flags = flags;

	return irqoverride->length;
}

int acpi_create_madt_lapic_nmi(struct acpi_madt_lapic_nmi *lapic_nmi,
			       u8 cpu, u16 flags, u8 lint)
{
	lapic_nmi->type = ACPI_APIC_LAPIC_NMI;
	lapic_nmi->length = sizeof(struct acpi_madt_lapic_nmi);
	lapic_nmi->flags = flags;
	lapic_nmi->processor_id = cpu;
	lapic_nmi->lint = lint;

	return lapic_nmi->length;
}

static int acpi_create_madt_irq_overrides(u32 current)
{
	struct acpi_madt_irqoverride *irqovr;
	u16 sci_flags = MP_IRQ_TRIGGER_LEVEL | MP_IRQ_POLARITY_HIGH;
	int length = 0;

	irqovr = (void *)current;
	length += acpi_create_madt_irqoverride(irqovr, 0, 0, 2, 0);

	irqovr = (void *)(current + length);
	length += acpi_create_madt_irqoverride(irqovr, 0, 9, 9, sci_flags);

	return length;
}

__weak u32 acpi_fill_madt(u32 current)
{
	current += acpi_create_madt_lapics(current);

	current += acpi_create_madt_ioapic((struct acpi_madt_ioapic *)current,
			io_apic_read(IO_APIC_ID) >> 24, IO_APIC_ADDR, 0);

	current += acpi_create_madt_irq_overrides(current);

	return current;
}

int acpi_write_madt(struct acpi_ctx *ctx, const struct acpi_writer *entry)
{
	struct acpi_table_header *header;
	struct acpi_madt *madt;
	u32 current;

	madt = ctx->current;

	memset(madt, '\0', sizeof(struct acpi_madt));
	header = &madt->header;

	/* Fill out header fields */
	acpi_fill_header(header, "APIC");
	header->length = sizeof(struct acpi_madt);
	header->revision = ACPI_MADT_REV_ACPI_3_0;

	madt->lapic_addr = LAPIC_DEFAULT_BASE;
	madt->flags = ACPI_MADT_PCAT_COMPAT;

	current = (u32)madt + sizeof(struct acpi_madt);
	current = acpi_fill_madt(current);

	/* (Re)calculate length and checksum */
	header->length = current - (u32)madt;

	header->checksum = table_compute_checksum((void *)madt, header->length);
	acpi_add_table(ctx, madt);
	acpi_inc(ctx, madt->header.length);

	return 0;
}
ACPI_WRITER(5x86, NULL, acpi_write_madt, 0);

/**
 * acpi_create_tcpa() - Create a TCPA table
 *
 * Trusted Computing Platform Alliance Capabilities Table
 * TCPA PC Specific Implementation SpecificationTCPA is defined in the PCI
 * Firmware Specification 3.0
 */
int acpi_write_tcpa(struct acpi_ctx *ctx, const struct acpi_writer *entry)
{
	struct acpi_table_header *header;
	struct acpi_tcpa *tcpa;
	u32 current;
	int size = 0x10000;	/* Use this as the default size */
	void *log;
	int ret;

	if (!IS_ENABLED(CONFIG_TPM_V1))
		return -ENOENT;
	if (!CONFIG_IS_ENABLED(BLOBLIST))
		return -ENXIO;

	tcpa = ctx->current;
	header = &tcpa->header;
	memset(tcpa, '\0', sizeof(struct acpi_tcpa));

	/* Fill out header fields */
	acpi_fill_header(header, "TCPA");
	header->length = sizeof(struct acpi_tcpa);
	header->revision = 1;

	ret = bloblist_ensure_size_ret(BLOBLISTT_TCPA_LOG, &size, &log);
	if (ret)
		return log_msg_ret("blob", ret);

	tcpa->platform_class = 0;
	tcpa->laml = size;
	tcpa->lasa = nomap_to_sysmem(log);

	/* (Re)calculate length and checksum */
	current = (u32)tcpa + sizeof(struct acpi_tcpa);
	header->length = current - (u32)tcpa;
	header->checksum = table_compute_checksum(tcpa, header->length);

	acpi_inc(ctx, tcpa->header.length);
	acpi_add_table(ctx, tcpa);

	return 0;
}
ACPI_WRITER(5tcpa, "TCPA", acpi_write_tcpa, 0);

static int get_tpm2_log(void **ptrp, int *sizep)
{
	const int tpm2_default_log_len = 0x10000;
	int size;
	int ret;

	*sizep = 0;
	size = tpm2_default_log_len;
	ret = bloblist_ensure_size_ret(BLOBLISTT_TPM2_TCG_LOG, &size, ptrp);
	if (ret)
		return log_msg_ret("blob", ret);
	*sizep = size;

	return 0;
}

static int acpi_write_tpm2(struct acpi_ctx *ctx,
			   const struct acpi_writer *entry)
{
	struct acpi_table_header *header;
	struct acpi_tpm2 *tpm2;
	int tpm2_log_len;
	void *lasa;
	int ret;

	if (!IS_ENABLED(CONFIG_TPM_V2))
		return log_msg_ret("none", -ENOENT);

	tpm2 = ctx->current;
	header = &tpm2->header;
	memset(tpm2, '\0', sizeof(struct acpi_tpm2));

	/*
	 * Some payloads like SeaBIOS depend on log area to use TPM2.
	 * Get the memory size and address of TPM2 log area or initialize it.
	 */
	ret = get_tpm2_log(&lasa, &tpm2_log_len);
	if (ret)
		return log_msg_ret("log", ret);

	/* Fill out header fields. */
	acpi_fill_header(header, "TPM2");
	memcpy(header->aslc_id, ASLC_ID, 4);

	header->length = sizeof(struct acpi_tpm2);
	header->revision = acpi_get_table_revision(ACPITAB_TPM2);

	/* Hard to detect for U-Boot. Just set it to 0 */
	tpm2->platform_class = 0;

	/* Must be set to 0 for FIFO-interface support */
	tpm2->control_area = 0;
	tpm2->start_method = 6;
	memset(tpm2->msp, 0, sizeof(tpm2->msp));

	/* Fill the log area size and start address fields. */
	tpm2->laml = tpm2_log_len;
	tpm2->lasa = nomap_to_sysmem(lasa);

	/* Calculate checksum. */
	header->checksum = table_compute_checksum(tpm2, header->length);

	acpi_inc(ctx, tpm2->header.length);
	acpi_add_table(ctx, tpm2);

	return 0;
}
ACPI_WRITER(5tpm2, "TPM2", acpi_write_tpm2, 0);

int acpi_write_spcr(struct acpi_ctx *ctx, const struct acpi_writer *entry)
{
	struct serial_device_info serial_info = {0};
	ulong serial_address, serial_offset;
	struct acpi_table_header *header;
	struct acpi_spcr *spcr;
	struct udevice *dev;
	uint serial_config;
	uint serial_width;
	int access_size;
	int space_id;
	int ret = -ENODEV;

	spcr = ctx->current;
	header = &spcr->header;

	memset(spcr, '\0', sizeof(struct acpi_spcr));

	/* Fill out header fields */
	acpi_fill_header(header, "SPCR");
	header->length = sizeof(struct acpi_spcr);
	header->revision = 2;

	/* Read the device once, here. It is reused below */
	dev = gd->cur_serial_dev;
	if (dev)
		ret = serial_getinfo(dev, &serial_info);
	if (ret)
		serial_info.type = SERIAL_CHIP_UNKNOWN;

	/* Encode chip type */
	switch (serial_info.type) {
	case SERIAL_CHIP_16550_COMPATIBLE:
		spcr->interface_type = ACPI_DBG2_16550_COMPATIBLE;
		break;
	case SERIAL_CHIP_UNKNOWN:
	default:
		spcr->interface_type = ACPI_DBG2_UNKNOWN;
		break;
	}

	/* Encode address space */
	switch (serial_info.addr_space) {
	case SERIAL_ADDRESS_SPACE_MEMORY:
		space_id = ACPI_ADDRESS_SPACE_MEMORY;
		break;
	case SERIAL_ADDRESS_SPACE_IO:
	default:
		space_id = ACPI_ADDRESS_SPACE_IO;
		break;
	}

	serial_width = serial_info.reg_width * 8;
	serial_offset = serial_info.reg_offset << serial_info.reg_shift;
	serial_address = serial_info.addr + serial_offset;

	/* Encode register access size */
	switch (serial_info.reg_shift) {
	case 0:
		access_size = ACPI_ACCESS_SIZE_BYTE_ACCESS;
		break;
	case 1:
		access_size = ACPI_ACCESS_SIZE_WORD_ACCESS;
		break;
	case 2:
		access_size = ACPI_ACCESS_SIZE_DWORD_ACCESS;
		break;
	case 3:
		access_size = ACPI_ACCESS_SIZE_QWORD_ACCESS;
		break;
	default:
		access_size = ACPI_ACCESS_SIZE_UNDEFINED;
		break;
	}

	debug("UART type %u @ %lx\n", spcr->interface_type, serial_address);

	/* Fill GAS */
	spcr->serial_port.space_id = space_id;
	spcr->serial_port.bit_width = serial_width;
	spcr->serial_port.bit_offset = 0;
	spcr->serial_port.access_size = access_size;
	spcr->serial_port.addrl = lower_32_bits(serial_address);
	spcr->serial_port.addrh = upper_32_bits(serial_address);

	/* Encode baud rate */
	switch (serial_info.baudrate) {
	case 9600:
		spcr->baud_rate = 3;
		break;
	case 19200:
		spcr->baud_rate = 4;
		break;
	case 57600:
		spcr->baud_rate = 6;
		break;
	case 115200:
		spcr->baud_rate = 7;
		break;
	default:
		spcr->baud_rate = 0;
		break;
	}

	serial_config = SERIAL_DEFAULT_CONFIG;
	if (dev)
		ret = serial_getconfig(dev, &serial_config);

	spcr->parity = SERIAL_GET_PARITY(serial_config);
	spcr->stop_bits = SERIAL_GET_STOP(serial_config);

	/* No PCI devices for now */
	spcr->pci_device_id = 0xffff;
	spcr->pci_vendor_id = 0xffff;

	/*
	 * SPCR has no clue if the UART base clock speed is different
	 * to the default one. However, the SPCR 1.04 defines baud rate
	 * 0 as a preconfigured state of UART and OS is supposed not
	 * to touch the configuration of the serial device.
	 */
	if (serial_info.clock != SERIAL_DEFAULT_CLOCK)
		spcr->baud_rate = 0;

	/* Fix checksum */
	header->checksum = table_compute_checksum((void *)spcr, header->length);

	acpi_add_table(ctx, spcr);
	acpi_inc(ctx, spcr->header.length);

	return 0;
}
ACPI_WRITER(5spcr, "SPCR", acpi_write_spcr, 0);

int acpi_write_gnvs(struct acpi_ctx *ctx, const struct acpi_writer *entry)
{
	ulong addr;

	if (!IS_ENABLED(CONFIG_ACPI_GNVS_EXTERNAL)) {
		int i;

		/* We need the DSDT to be done */
		if (!ctx->dsdt)
			return log_msg_ret("dsdt", -EAGAIN);

		/* Pack GNVS into the ACPI table area */
		for (i = 0; i < ctx->dsdt->length; i++) {
			u32 *gnvs = (u32 *)((u32)ctx->dsdt + i);

			if (*gnvs == ACPI_GNVS_ADDR) {
				*gnvs = nomap_to_sysmem(ctx->current);
				log_debug("Fix up global NVS in DSDT to %#08x\n",
					  *gnvs);
				break;
			}
		}

		/*
		 * Recalculate the length and update the DSDT checksum since we
		 * patched the GNVS address. Set the checksum to zero since it
		 * is part of the region being checksummed.
		 */
		ctx->dsdt->checksum = 0;
		ctx->dsdt->checksum = table_compute_checksum((void *)ctx->dsdt,
							     ctx->dsdt->length);
	}

	/* Fill in platform-specific global NVS variables */
	addr = acpi_create_gnvs(ctx->current);
	if (IS_ERR_VALUE(addr))
		return log_msg_ret("gnvs", (int)addr);

	acpi_inc_align(ctx, sizeof(struct acpi_global_nvs));

	return 0;
}
ACPI_WRITER(4gnvs, "GNVS", acpi_write_gnvs, 0);

/**
 * acpi_write_hpet() - Write out a HPET table
 *
 * Write out the table for High-Precision Event Timers
 *
 * @hpet: Place to put HPET table
 */
static int acpi_create_hpet(struct acpi_hpet *hpet)
{
	struct acpi_table_header *header = &hpet->header;
	struct acpi_gen_regaddr *addr = &hpet->addr;

	/*
	 * See IA-PC HPET (High Precision Event Timers) Specification v1.0a
	 * https://www.intel.com/content/dam/www/public/us/en/documents/technical-specifications/software-developers-hpet-spec-1-0a.pdf
	 */
	memset((void *)hpet, '\0', sizeof(struct acpi_hpet));

	/* Fill out header fields. */
	acpi_fill_header(header, "HPET");

	header->aslc_revision = ASL_REVISION;
	header->length = sizeof(struct acpi_hpet);
	header->revision = acpi_get_table_revision(ACPITAB_HPET);

	/* Fill out HPET address */
	addr->space_id = 0;  /* Memory */
	addr->bit_width = 64;
	addr->bit_offset = 0;
	addr->addrl = CONFIG_HPET_ADDRESS & 0xffffffff;
	addr->addrh = ((unsigned long long)CONFIG_HPET_ADDRESS) >> 32;

	hpet->id = *(u32 *)CONFIG_HPET_ADDRESS;
	hpet->number = 0;
	hpet->min_tick = 0; /* HPET_MIN_TICKS */

	header->checksum = table_compute_checksum(hpet,
						  sizeof(struct acpi_hpet));

	return 0;
}

int acpi_write_hpet(struct acpi_ctx *ctx)
{
	struct acpi_hpet *hpet;
	int ret;

	log_debug("ACPI:    * HPET\n");

	hpet = ctx->current;
	acpi_inc_align(ctx, sizeof(struct acpi_hpet));
	acpi_create_hpet(hpet);
	ret = acpi_add_table(ctx, hpet);
	if (ret)
		return log_msg_ret("add", ret);

	return 0;
}

int acpi_write_dbg2_pci_uart(struct acpi_ctx *ctx, struct udevice *dev,
			     uint access_size)
{
	struct acpi_dbg2_header *dbg2 = ctx->current;
	char path[ACPI_PATH_MAX];
	struct acpi_gen_regaddr address;
	phys_addr_t addr;
	int ret;

	if (!device_active(dev)) {
		log_info("Device not enabled\n");
		return -EACCES;
	}
	/*
	 * PCI devices don't remember their resource allocation information in
	 * U-Boot at present. We assume that MMIO is used for the UART and that
	 * the address space is 32 bytes: ns16550 uses 8 registers of up to
	 * 32-bits each. This is only for debugging so it is not a big deal.
	 */
	addr = dm_pci_read_bar32(dev, 0);
	log_debug("UART addr %lx\n", (ulong)addr);

	memset(&address, '\0', sizeof(address));
	address.space_id = ACPI_ADDRESS_SPACE_MEMORY;
	address.addrl = (uint32_t)addr;
	address.addrh = (uint32_t)((addr >> 32) & 0xffffffff);
	address.access_size = access_size;

	ret = acpi_device_path(dev, path, sizeof(path));
	if (ret)
		return log_msg_ret("path", ret);
	acpi_create_dbg2(dbg2, ACPI_DBG2_SERIAL_PORT,
			 ACPI_DBG2_16550_COMPATIBLE, &address, 0x1000, path);

	acpi_inc_align(ctx, dbg2->header.length);
	acpi_add_table(ctx, dbg2);

	return 0;
}

void acpi_fadt_common(struct acpi_fadt *fadt, struct acpi_facs *facs,
		      void *dsdt)
{
	struct acpi_table_header *header = &fadt->header;

	memset((void *)fadt, '\0', sizeof(struct acpi_fadt));

	acpi_fill_header(header, "FACP");
	header->length = sizeof(struct acpi_fadt);
	header->revision = 4;
	memcpy(header->oem_id, OEM_ID, 6);
	memcpy(header->oem_table_id, OEM_TABLE_ID, 8);
	memcpy(header->aslc_id, ASLC_ID, 4);
	header->aslc_revision = 1;

	fadt->x_firmware_ctrl = map_to_sysmem(facs);
	fadt->x_dsdt = map_to_sysmem(dsdt);

	fadt->preferred_pm_profile = ACPI_PM_MOBILE;

	/* Use ACPI 3.0 revision */
	fadt->header.revision = 4;
}

void acpi_create_dmar_drhd(struct acpi_ctx *ctx, uint flags, uint segment,
			   u64 bar)
{
	struct dmar_entry *drhd = ctx->current;

	memset(drhd, '\0', sizeof(*drhd));
	drhd->type = DMAR_DRHD;
	drhd->length = sizeof(*drhd); /* will be fixed up later */
	drhd->flags = flags;
	drhd->segment = segment;
	drhd->bar = bar;
	acpi_inc(ctx, drhd->length);
}

void acpi_create_dmar_rmrr(struct acpi_ctx *ctx, uint segment, u64 bar,
			   u64 limit)
{
	struct dmar_rmrr_entry *rmrr = ctx->current;

	memset(rmrr, '\0', sizeof(*rmrr));
	rmrr->type = DMAR_RMRR;
	rmrr->length = sizeof(*rmrr); /* will be fixed up later */
	rmrr->segment = segment;
	rmrr->bar = bar;
	rmrr->limit = limit;
	acpi_inc(ctx, rmrr->length);
}

void acpi_dmar_drhd_fixup(struct acpi_ctx *ctx, void *base)
{
	struct dmar_entry *drhd = base;

	drhd->length = ctx->current - base;
}

void acpi_dmar_rmrr_fixup(struct acpi_ctx *ctx, void *base)
{
	struct dmar_rmrr_entry *rmrr = base;

	rmrr->length = ctx->current - base;
}

static int acpi_create_dmar_ds(struct acpi_ctx *ctx, enum dev_scope_type type,
			       uint enumeration_id, pci_dev_t bdf)
{
	/* we don't support longer paths yet */
	const size_t dev_scope_length = sizeof(struct dev_scope) + 2;
	struct dev_scope *ds = ctx->current;

	memset(ds, '\0', dev_scope_length);
	ds->type = type;
	ds->length = dev_scope_length;
	ds->enumeration = enumeration_id;
	ds->start_bus = PCI_BUS(bdf);
	ds->path[0].dev = PCI_DEV(bdf);
	ds->path[0].fn = PCI_FUNC(bdf);

	return ds->length;
}

int acpi_create_dmar_ds_pci_br(struct acpi_ctx *ctx, pci_dev_t bdf)
{
	return acpi_create_dmar_ds(ctx, SCOPE_PCI_SUB, 0, bdf);
}

int acpi_create_dmar_ds_pci(struct acpi_ctx *ctx, pci_dev_t bdf)
{
	return acpi_create_dmar_ds(ctx, SCOPE_PCI_ENDPOINT, 0, bdf);
}

int acpi_create_dmar_ds_ioapic(struct acpi_ctx *ctx, uint enumeration_id,
			       pci_dev_t bdf)
{
	return acpi_create_dmar_ds(ctx, SCOPE_IOAPIC, enumeration_id, bdf);
}

int acpi_create_dmar_ds_msi_hpet(struct acpi_ctx *ctx, uint enumeration_id,
				 pci_dev_t bdf)
{
	return acpi_create_dmar_ds(ctx, SCOPE_MSI_HPET, enumeration_id, bdf);
}
