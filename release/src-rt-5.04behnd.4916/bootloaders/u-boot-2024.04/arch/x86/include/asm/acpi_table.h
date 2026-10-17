/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Based on acpi.c from coreboot
 *
 * Copyright (C) 2015, Saket Sinha <saket.sinha89@gmail.com>
 * Copyright (C) 2016, Bin Meng <bmeng.cn@gmail.com>
 */

#ifndef __ASM_ACPI_TABLE_H__
#define __ASM_ACPI_TABLE_H__

#ifndef __ACPI__

#include <pci.h>

struct acpi_facs;
struct acpi_fadt;
struct acpi_global_nvs;
struct acpi_madt_ioapic;
struct acpi_madt_irqoverride;
struct acpi_madt_lapic_nmi;
struct acpi_mcfg_mmconfig;
struct acpi_table_header;

/* These can be used by the target port */

int acpi_create_madt_lapics(u32 current);
int acpi_create_madt_ioapic(struct acpi_madt_ioapic *ioapic, u8 id,
			    u32 addr, u32 gsi_base);
int acpi_create_madt_irqoverride(struct acpi_madt_irqoverride *irqoverride,
				 u8 bus, u8 source, u32 gsirq, u16 flags);
int acpi_create_madt_lapic_nmi(struct acpi_madt_lapic_nmi *lapic_nmi,
			       u8 cpu, u16 flags, u8 lint);
u32 acpi_fill_madt(u32 current);
int acpi_create_mcfg_mmconfig(struct acpi_mcfg_mmconfig *mmconfig, u32 base,
			      u16 seg_nr, u8 start, u8 end);

/**
 * acpi_write_hpet() - Write out a HPET table
 *
 * Write out the table for High-Precision Event Timers
 *
 * @ctx: Current ACPI context
 * Return: 0 if OK, -ve on error
 */
int acpi_write_hpet(struct acpi_ctx *ctx);

/**
 * acpi_write_dbg2_pci_uart() - Write out a DBG2 table
 *
 * @ctx: Current ACPI context
 * @dev: Debug UART device to describe
 * @access_size: Access size for UART (e.g. ACPI_ACCESS_SIZE_DWORD_ACCESS)
 * Return: 0 if OK, -ve on error
 */
int acpi_write_dbg2_pci_uart(struct acpi_ctx *ctx, struct udevice *dev,
			     uint access_size);

/**
 * acpi_create_gnvs() - Create a GNVS (Global Non Volatile Storage) table
 *
 * @gnvs: Table to fill in
 * Return: 0 if OK, -ve on error
 */
int acpi_create_gnvs(struct acpi_global_nvs *gnvs);

/**
 * arch_read_sci_irq_select() - Read the system-control interrupt number
 *
 * @returns value of IRQ register in the PMC
 */
int arch_read_sci_irq_select(void);

/**
 * arch_write_sci_irq_select() - Set the system-control interrupt number
 *
 * @scis: New value for IRQ register in the PMC
 */
int arch_write_sci_irq_select(uint scis);

/**
 * arch_madt_sci_irq_polarity() - Return the priority to use for the MADT
 *
 * @sci: System-control interrupt number
 * Return: priority to use (MP_IRQ_POLARITY_...)
 */
int arch_madt_sci_irq_polarity(int sci);

/**
 * acpi_create_dmar_drhd() - Create a table for DMA remapping with the IOMMU
 *
 * See here for the specification
 * https://software.intel.com/sites/default/files/managed/c5/15/vt-directed-io-spec.pdf
 *
 * @ctx: ACPI context pointer
 * @flags: (DRHD_INCLUDE_...)
 * @segment: PCI segment asscociated with this unit
 * @bar: Base address of remapping hardware register-set for this unit
 */
void acpi_create_dmar_drhd(struct acpi_ctx *ctx, uint flags, uint segment,
			   u64 bar);

/**
 * acpi_create_dmar_rmrr() - Set up an RMRR
 *
 * This sets up a Reserved-Memory Region Reporting structure, used to allow
 * DMA to regions used by devices that the BIOS controls.
 *
 * @ctx: ACPI context pointer
 * @segment: PCI segment asscociated with this unit
 * @bar: Base address of mapping
 * @limit: End address of mapping
 */
void acpi_create_dmar_rmrr(struct acpi_ctx *ctx, uint segment, u64 bar,
			   u64 limit);

/**
 * acpi_dmar_drhd_fixup() - Set the length of an DRHD
 *
 * This sets the DRHD length field based on the current ctx->current
 *
 * @ctx: ACPI context pointer
 * @base: Address of the start of the DRHD
 */
void acpi_dmar_drhd_fixup(struct acpi_ctx *ctx, void *base);

/**
 * acpi_dmar_rmrr_fixup() - Set the length of an RMRR
 *
 * This sets the RMRR length field based on the current ctx->current
 *
 * @ctx: ACPI context pointer
 * @base: Address of the start of the RMRR
 */
void acpi_dmar_rmrr_fixup(struct acpi_ctx *ctx, void *base);

/**
 * acpi_create_dmar_ds_pci() - Set up a DMAR scope for a PCI device
 *
 * @ctx: ACPI context pointer
 * @bdf: PCI device to add
 * Return: length of mapping in bytes
 */
int acpi_create_dmar_ds_pci(struct acpi_ctx *ctx, pci_dev_t bdf);

/**
 * acpi_create_dmar_ds_pci_br() - Set up a DMAR scope for a PCI bridge
 *
 * This is used to provide a mapping for a PCI bridge
 *
 * @ctx: ACPI context pointer
 * @bdf: PCI device to add
 * Return: length of mapping in bytes
 */
int acpi_create_dmar_ds_pci_br(struct acpi_ctx *ctx, pci_dev_t bdf);

/**
 * acpi_create_dmar_ds_ioapic() - Set up a DMAR scope for an IOAPIC device
 *
 * @ctx: ACPI context pointer
 * @enumeration_id: Enumeration ID (typically 2)
 * @bdf: PCI device to add
 * Return: length of mapping in bytes
 */
int acpi_create_dmar_ds_ioapic(struct acpi_ctx *ctx, uint enumeration_id,
			       pci_dev_t bdf);

/**
 * acpi_create_dmar_ds_msi_hpet() - Set up a DMAR scope for an HPET
 *
 * Sets up a scope for a High-Precision Event Timer that supports
 * Message-Signalled Interrupts
 *
 * @ctx: ACPI context pointer
 * @enumeration_id: Enumeration ID (typically 0)
 * @bdf: PCI device to add
 * Return: length of mapping in bytes
 */
int acpi_create_dmar_ds_msi_hpet(struct acpi_ctx *ctx, uint enumeration_id,
				 pci_dev_t bdf);

/**
 * acpi_fadt_common() - Handle common parts of filling out an FADT
 *
 * This sets up the Fixed ACPI Description Table
 *
 * @fadt: Pointer to place to put FADT
 * @facs: Pointer to the FACS
 * @dsdt: Pointer to the DSDT
 */
void acpi_fadt_common(struct acpi_fadt *fadt, struct acpi_facs *facs,
		      void *dsdt);

/**
 * intel_acpi_fill_fadt() - Set up the contents of the FADT
 *
 * This sets up parts of the Fixed ACPI Description Table that are common to
 * Intel chips
 *
 * @fadt: Pointer to place to put FADT
 */
void intel_acpi_fill_fadt(struct acpi_fadt *fadt);

#endif /* !__ACPI__ */

#endif /* __ASM_ACPI_TABLE_H__ */
