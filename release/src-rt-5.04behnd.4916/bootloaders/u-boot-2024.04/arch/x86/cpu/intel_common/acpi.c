// SPDX-License-Identifier: GPL-2.0+
/*
 * Generic Intel ACPI table generation
 *
 * Copyright (C) 2017 Intel Corp.
 * Copyright 2019 Google LLC
 *
 * Modified from coreboot src/soc/intel/common/block/acpi.c
 */

#include <common.h>
#include <bloblist.h>
#include <cpu.h>
#include <dm.h>
#include <acpi/acpigen.h>
#include <asm/acpigen.h>
#include <asm/acpi_table.h>
#include <asm/cpu.h>
#include <asm/cpu_common.h>
#include <asm/global_data.h>
#include <asm/intel_acpi.h>
#include <asm/ioapic.h>
#include <asm/mpspec.h>
#include <asm/smm.h>
#include <asm/turbo.h>
#include <asm/intel_gnvs.h>
#include <asm/arch/iomap.h>
#include <asm/arch/pm.h>
#include <asm/arch/systemagent.h>
#include <dm/acpi.h>
#include <linux/err.h>
#include <power/acpi_pmc.h>

int acpi_fill_mcfg(struct acpi_ctx *ctx)
{
	size_t size;

	/* PCI Segment Group 0, Start Bus Number 0, End Bus Number is 255 */
	size = acpi_create_mcfg_mmconfig((void *)ctx->current,
					 CONFIG_MMCONF_BASE_ADDRESS, 0, 0,
					 (CONFIG_SA_PCIEX_LENGTH >> 20) - 1);
	acpi_inc(ctx, size);

	return 0;
}

static int acpi_sci_irq(void)
{
	int sci_irq = 9;
	uint scis;
	int ret;

	ret = arch_read_sci_irq_select();
	if (IS_ERR_VALUE(ret))
		return log_msg_ret("sci_irq", ret);
	scis = ret;
	scis &= SCI_IRQ_MASK;
	scis >>= SCI_IRQ_SHIFT;

	/* Determine how SCI is routed. */
	switch (scis) {
	case SCIS_IRQ9:
	case SCIS_IRQ10:
	case SCIS_IRQ11:
		sci_irq = scis - SCIS_IRQ9 + 9;
		break;
	case SCIS_IRQ20:
	case SCIS_IRQ21:
	case SCIS_IRQ22:
	case SCIS_IRQ23:
		sci_irq = scis - SCIS_IRQ20 + 20;
		break;
	default:
		log_warning("Invalid SCI route! Defaulting to IRQ9\n");
		sci_irq = 9;
		break;
	}

	log_debug("SCI is IRQ%d\n", sci_irq);

	return sci_irq;
}

static unsigned long acpi_madt_irq_overrides(unsigned long current)
{
	int sci = acpi_sci_irq();
	u16 flags = MP_IRQ_TRIGGER_LEVEL;

	if (sci < 0)
		return log_msg_ret("sci irq", sci);

	/* INT_SRC_OVR */
	current += acpi_create_madt_irqoverride((void *)current, 0, 0, 2, 0);

	flags |= arch_madt_sci_irq_polarity(sci);

	/* SCI */
	current +=
	    acpi_create_madt_irqoverride((void *)current, 0, sci, sci, flags);

	return current;
}

u32 acpi_fill_madt(u32 current)
{
	/* Local APICs */
	current += acpi_create_madt_lapics(current);

	/* IOAPIC */
	current += acpi_create_madt_ioapic((void *)current, 2, IO_APIC_ADDR, 0);

	return acpi_madt_irq_overrides(current);
}

void intel_acpi_fill_fadt(struct acpi_fadt *fadt)
{
	const u16 pmbase = IOMAP_ACPI_BASE;

	/* Use ACPI 3.0 revision. */
	fadt->header.revision = acpi_get_table_revision(ACPITAB_FADT);

	fadt->sci_int = acpi_sci_irq();
	fadt->smi_cmd = APM_CNT;
	fadt->acpi_enable = APM_CNT_ACPI_ENABLE;
	fadt->acpi_disable = APM_CNT_ACPI_DISABLE;
	fadt->s4bios_req = 0x0;
	fadt->pstate_cnt = 0;

	fadt->pm1a_evt_blk = pmbase + PM1_STS;
	fadt->pm1b_evt_blk = 0x0;
	fadt->pm1a_cnt_blk = pmbase + PM1_CNT;
	fadt->pm1b_cnt_blk = 0x0;

	fadt->gpe0_blk = pmbase + GPE0_STS;

	fadt->pm1_evt_len = 4;
	fadt->pm1_cnt_len = 2;

	/* GPE0 STS/EN pairs each 32 bits wide. */
	fadt->gpe0_blk_len = 2 * GPE0_REG_MAX * sizeof(uint32_t);

	fadt->flush_size = 0x400;	/* twice of cache size */
	fadt->flush_stride = 0x10;	/* Cache line width  */
	fadt->duty_offset = 1;
	fadt->day_alrm = 0xd;

	fadt->flags = ACPI_FADT_WBINVD | ACPI_FADT_C1_SUPPORTED |
	    ACPI_FADT_C2_MP_SUPPORTED | ACPI_FADT_SLEEP_BUTTON |
	    ACPI_FADT_RESET_REGISTER | ACPI_FADT_SEALED_CASE |
	    ACPI_FADT_S4_RTC_WAKE | ACPI_FADT_PLATFORM_CLOCK;

	fadt->reset_reg.space_id = 1;
	fadt->reset_reg.bit_width = 8;
	fadt->reset_reg.addrl = IO_PORT_RESET;
	fadt->reset_value = RST_CPU | SYS_RST;

	fadt->x_pm1a_evt_blk.space_id = 1;
	fadt->x_pm1a_evt_blk.bit_width = fadt->pm1_evt_len * 8;
	fadt->x_pm1a_evt_blk.addrl = pmbase + PM1_STS;

	fadt->x_pm1b_evt_blk.space_id = 1;

	fadt->x_pm1a_cnt_blk.space_id = 1;
	fadt->x_pm1a_cnt_blk.bit_width = fadt->pm1_cnt_len * 8;
	fadt->x_pm1a_cnt_blk.addrl = pmbase + PM1_CNT;

	fadt->x_pm1b_cnt_blk.space_id = 1;

	fadt->x_gpe1_blk.space_id = 1;
}

int intel_southbridge_write_acpi_tables(const struct udevice *dev,
					struct acpi_ctx *ctx)
{
	int ret;

	ret = acpi_write_dbg2_pci_uart(ctx, gd->cur_serial_dev,
				       ACPI_ACCESS_SIZE_DWORD_ACCESS);
	if (ret)
		return log_msg_ret("dbg2", ret);

	ret = acpi_write_hpet(ctx);
	if (ret)
		return log_msg_ret("hpet", ret);

	return 0;
}

__weak u32 acpi_fill_soc_wake(u32 generic_pm1_en,
			      const struct chipset_power_state *ps)
{
	return generic_pm1_en;
}

__weak int acpi_create_gnvs(struct acpi_global_nvs *gnvs)
{
	return 0;
}

int southbridge_inject_dsdt(const struct udevice *dev, struct acpi_ctx *ctx)
{
	struct acpi_global_nvs *gnvs;
	int ret;

	ret = bloblist_ensure_size(BLOBLISTT_ACPI_GNVS, sizeof(*gnvs), 0,
				   (void **)&gnvs);
	if (ret)
		return log_msg_ret("bloblist", ret);

	ret = acpi_create_gnvs(gnvs);
	if (ret)
		return log_msg_ret("gnvs", ret);

	/*
	 * TODO(sjg@chromum.org): tell SMI about it
	 * smm_setup_structures(gnvs, NULL, NULL);
	 */

	/* Add it to DSDT */
	acpigen_write_scope(ctx, "\\");
	acpigen_write_name_dword(ctx, "NVSA", (uintptr_t)gnvs);
	acpigen_pop_len(ctx);

	return 0;
}

static int calculate_power(int tdp, int p1_ratio, int ratio)
{
	u32 m;
	u32 power;

	/*
	 * M = ((1.1 - ((p1_ratio - ratio) * 0.00625)) / 1.1) ^ 2
	 *
	 * Power = (ratio / p1_ratio) * m * tdp
	 */

	m = (110000 - ((p1_ratio - ratio) * 625)) / 11;
	m = (m * m) / 1000;

	power = ((ratio * 100000 / p1_ratio) / 100);
	power *= (m / 100) * (tdp / 1000);
	power /= 1000;

	return power;
}

void generate_p_state_entries(struct acpi_ctx *ctx, int core,
			      int cores_per_package)
{
	int ratio_min, ratio_max, ratio_turbo, ratio_step;
	int coord_type, power_max, num_entries;
	int ratio, power, clock, clock_max;
	bool turbo;

	coord_type = cpu_get_coord_type();
	ratio_min = cpu_get_min_ratio();
	ratio_max = cpu_get_max_ratio();
	clock_max = (ratio_max * cpu_get_bus_clock_khz()) / 1000;
	turbo = (turbo_get_state() == TURBO_ENABLED);

	/* Calculate CPU TDP in mW */
	power_max = cpu_get_power_max();

	/* Write _PCT indicating use of FFixedHW */
	acpigen_write_empty_pct(ctx);

	/* Write _PPC with no limit on supported P-state */
	acpigen_write_ppc_nvs(ctx);
	/* Write PSD indicating configured coordination type */
	acpigen_write_psd_package(ctx, core, 1, coord_type);

	/* Add P-state entries in _PSS table */
	acpigen_write_name(ctx, "_PSS");

	/* Determine ratio points */
	ratio_step = PSS_RATIO_STEP;
	do {
		num_entries = ((ratio_max - ratio_min) / ratio_step) + 1;
		if (((ratio_max - ratio_min) % ratio_step) > 0)
			num_entries += 1;
		if (turbo)
			num_entries += 1;
		if (num_entries > PSS_MAX_ENTRIES)
			ratio_step += 1;
	} while (num_entries > PSS_MAX_ENTRIES);

	/* _PSS package count depends on Turbo */
	acpigen_write_package(ctx, num_entries);

	/* P[T] is Turbo state if enabled */
	if (turbo) {
		ratio_turbo = cpu_get_max_turbo_ratio();

		/* Add entry for Turbo ratio */
		acpigen_write_pss_package(ctx, clock_max + 1,	/* MHz */
					  power_max,		/* mW */
					  PSS_LATENCY_TRANSITION,/* lat1 */
					  PSS_LATENCY_BUSMASTER,/* lat2 */
					  ratio_turbo << 8,	/* control */
					  ratio_turbo << 8);	/* status */
		num_entries -= 1;
	}

	/* First regular entry is max non-turbo ratio */
	acpigen_write_pss_package(ctx, clock_max,	/* MHz */
				  power_max,		/* mW */
				  PSS_LATENCY_TRANSITION,/* lat1 */
				  PSS_LATENCY_BUSMASTER,/* lat2 */
				  ratio_max << 8,	/* control */
				  ratio_max << 8);	/* status */
	num_entries -= 1;

	/* Generate the remaining entries */
	for (ratio = ratio_min + ((num_entries - 1) * ratio_step);
	     ratio >= ratio_min; ratio -= ratio_step) {
		/* Calculate power at this ratio */
		power = calculate_power(power_max, ratio_max, ratio);
		clock = (ratio * cpu_get_bus_clock_khz()) / 1000;

		acpigen_write_pss_package(ctx, clock,		/* MHz */
					  power,		/* mW */
					  PSS_LATENCY_TRANSITION,/* lat1 */
					  PSS_LATENCY_BUSMASTER,/* lat2 */
					  ratio << 8,		/* control */
					  ratio << 8);		/* status */
	}
	/* Fix package length */
	acpigen_pop_len(ctx);
}

void generate_t_state_entries(struct acpi_ctx *ctx, int core,
			      int cores_per_package, struct acpi_tstate *entry,
			      int nentries)
{
	if (!nentries)
		return;

	/* Indicate SW_ALL coordination for T-states */
	acpigen_write_tsd_package(ctx, core, cores_per_package, SW_ALL);

	/* Indicate FixedHW so OS will use MSR */
	acpigen_write_empty_ptc(ctx);

	/* Set NVS controlled T-state limit */
	acpigen_write_tpc(ctx, "\\TLVL");

	/* Write TSS table for MSR access */
	acpigen_write_tss_package(ctx, entry, nentries);
}

int acpi_generate_cpu_header(struct acpi_ctx *ctx, int core_id,
			     const struct acpi_cstate *c_state_map,
			     int num_cstates)
{
	bool is_first = !core_id;

	/* Generate processor \_PR.CPUx */
	acpigen_write_processor(ctx, core_id, is_first ? ACPI_BASE_ADDRESS : 0,
				is_first ? 6 : 0);

	/* Generate C-state tables */
	acpigen_write_cst_package(ctx, c_state_map, num_cstates);

	return 0;
}

int acpi_generate_cpu_package_final(struct acpi_ctx *ctx, int cores_per_package)
{
	/*
	 * PPKG is usually used for thermal management of the first and only
	 * package
	 */
	acpigen_write_processor_package(ctx, "PPKG", 0, cores_per_package);

	/* Add a method to notify processor nodes */
	acpigen_write_processor_cnot(ctx, cores_per_package);

	return 0;
}
