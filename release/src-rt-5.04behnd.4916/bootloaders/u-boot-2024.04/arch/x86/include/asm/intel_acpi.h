/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2018, Bin Meng <bmeng.cn@gmail.com>
 */

#ifndef __ASM_INTEL_ACPI_H__
#define __ASM_INTEL_ACPI_H__

struct acpi_cstate;
struct acpi_ctx;
struct acpi_tstate;
struct udevice;

/**
 * acpi_generate_cpu_header() - Start generating an ACPI CPU entry
 *
 * Generates the ACPI information for a CPU. After this, the caller should
 * generate_p_state_entries(), generate_t_state_entries and then
 * acpigen_pop_len() to close off this package.
 *
 * @ctx: ACPI context pointer
 * @core_id: CPU core number, as numbered by the SoC
 * @c_state_map: Information about each C state
 * @num_cstates: Number of entries in @c_state_map
 * Return: 0 if OK, -ve on error
 */
int acpi_generate_cpu_header(struct acpi_ctx *ctx, int core_id,
			     const struct acpi_cstate *c_state_map,
			     int num_cstates);

/**
 * acpi_generate_cpu_package_final() - Write out the CPU PPKG entry
 *
 * This writes information about the CPUs in the package
 *
 * @ctx: ACPI context pointer
 * @cores_per_package: Number of CPU cores in each package in the SoC
 */
int acpi_generate_cpu_package_final(struct acpi_ctx *ctx,
				    int cores_per_package);

void generate_p_state_entries(struct acpi_ctx *ctx, int core,
			      int cores_per_package);
void generate_t_state_entries(struct acpi_ctx *ctx, int core,
			      int cores_per_package, struct acpi_tstate *entry,
			      int nentries);
int southbridge_inject_dsdt(const struct udevice *dev, struct acpi_ctx *ctx);

int intel_southbridge_write_acpi_tables(const struct udevice *dev,
					struct acpi_ctx *ctx);

#endif /* __ASM_INTEL_ACPI_H__ */
