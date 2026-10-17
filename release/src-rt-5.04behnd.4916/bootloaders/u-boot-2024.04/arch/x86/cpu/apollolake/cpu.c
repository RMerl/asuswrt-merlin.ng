// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2019 Google LLC
 */

#include <common.h>
#include <cpu.h>
#include <dm.h>
#include <log.h>
#include <acpi/acpigen.h>
#include <acpi/acpi_table.h>
#include <asm/cpu_common.h>
#include <asm/cpu_x86.h>
#include <asm/global_data.h>
#include <asm/intel_acpi.h>
#include <asm/msr.h>
#include <asm/mtrr.h>
#include <asm/arch/cpu.h>
#include <asm/arch/iomap.h>
#include <dm/acpi.h>

#ifdef CONFIG_ACPIGEN
#define CSTATE_RES(address_space, width, offset, address)		\
	{								\
	.space_id = address_space,					\
	.bit_width = width,						\
	.bit_offset = offset,						\
	.addrl = address,						\
	}

static struct acpi_cstate cstate_map[] = {
	{
		/* C1 */
		.ctype = 1,		/* ACPI C1 */
		.latency = 1,
		.power = 1000,
		.resource = {
			.space_id = ACPI_ADDRESS_SPACE_FIXED,
		},
	}, {
		.ctype = 2,		/* ACPI C2 */
		.latency = 50,
		.power = 10,
		.resource = {
			.space_id = ACPI_ADDRESS_SPACE_IO,
			.bit_width = 8,
			.addrl = 0x415,
		},
	}, {
		.ctype = 3,		/* ACPI C3 */
		.latency = 150,
		.power = 10,
		.resource = {
			.space_id = ACPI_ADDRESS_SPACE_IO,
			.bit_width = 8,
			.addrl = 0x419,
		},
	},
};

static int acpi_cpu_fill_ssdt(const struct udevice *dev, struct acpi_ctx *ctx)
{
	uint core_id = dev_seq(dev);
	int cores_per_package;
	int ret;

	cores_per_package = cpu_get_cores_per_package();
	ret = acpi_generate_cpu_header(ctx, core_id, cstate_map,
				       ARRAY_SIZE(cstate_map));

	/* Generate P-state tables */
	generate_p_state_entries(ctx, core_id, cores_per_package);

	/* Generate T-state tables */
	generate_t_state_entries(ctx, core_id, cores_per_package, NULL, 0);

	acpigen_pop_len(ctx);

	if (device_is_last_sibling(dev)) {
		ret = acpi_generate_cpu_package_final(ctx, cores_per_package);

		if (ret)
			return ret;
	}

	return 0;
}
#endif /* CONFIG_ACPIGEN */

static int apl_get_info(const struct udevice *dev, struct cpu_info *info)
{
	return cpu_intel_get_info(info, INTEL_BCLK_MHZ);
}

static void update_fixed_mtrrs(void)
{
	native_write_msr(MTRR_FIX_64K_00000_MSR,
			 MTRR_FIX_TYPE(MTRR_TYPE_WRBACK),
			 MTRR_FIX_TYPE(MTRR_TYPE_WRBACK));
	native_write_msr(MTRR_FIX_16K_80000_MSR,
			 MTRR_FIX_TYPE(MTRR_TYPE_WRBACK),
			 MTRR_FIX_TYPE(MTRR_TYPE_WRBACK));
	native_write_msr(MTRR_FIX_4K_E0000_MSR,
			 MTRR_FIX_TYPE(MTRR_TYPE_WRBACK),
			 MTRR_FIX_TYPE(MTRR_TYPE_WRBACK));
	native_write_msr(MTRR_FIX_4K_E8000_MSR,
			 MTRR_FIX_TYPE(MTRR_TYPE_WRBACK),
			 MTRR_FIX_TYPE(MTRR_TYPE_WRBACK));
	native_write_msr(MTRR_FIX_4K_F0000_MSR,
			 MTRR_FIX_TYPE(MTRR_TYPE_WRBACK),
			 MTRR_FIX_TYPE(MTRR_TYPE_WRBACK));
	native_write_msr(MTRR_FIX_4K_F8000_MSR,
			 MTRR_FIX_TYPE(MTRR_TYPE_WRBACK),
			 MTRR_FIX_TYPE(MTRR_TYPE_WRBACK));
}

static void setup_core_msrs(void)
{
	wrmsrl(MSR_PMG_CST_CONFIG_CONTROL,
	       PKG_C_STATE_LIMIT_C2_MASK | CORE_C_STATE_LIMIT_C10_MASK |
	       IO_MWAIT_REDIRECT_MASK | CST_CFG_LOCK_MASK);
	/* Power Management I/O base address for I/O trapping to C-states */
	wrmsrl(MSR_PMG_IO_CAPTURE_ADR, ACPI_PMIO_CST_REG |
	       (PMG_IO_BASE_CST_RNG_BLK_SIZE << 16));
	/* Disable C1E */
	msr_clrsetbits_64(MSR_POWER_CTL, 0x2, 0);
	/* Disable support for MONITOR and MWAIT instructions */
	msr_clrsetbits_64(MSR_IA32_MISC_ENABLE, MISC_ENABLE_MWAIT, 0);
	/*
	 * Enable and Lock the Advanced Encryption Standard (AES-NI)
	 * feature register
	 */
	msr_clrsetbits_64(MSR_FEATURE_CONFIG, FEATURE_CONFIG_RESERVED_MASK,
			  FEATURE_CONFIG_LOCK);

	update_fixed_mtrrs();
}

static int soc_core_init(void)
{
	struct udevice *pmc;
	int ret;

	/* Clear out pending MCEs */
	cpu_mca_configure();

	/* Set core MSRs */
	setup_core_msrs();
	/*
	 * Enable ACPI PM timer emulation, which also lets microcode know
	 * location of ACPI_BASE_ADDRESS. This also enables other features
	 * implemented in microcode.
	 */
	ret = uclass_first_device_err(UCLASS_ACPI_PMC, &pmc);
	if (ret)
		return log_msg_ret("PMC", ret);
	enable_pm_timer_emulation(pmc);

	return 0;
}

static int cpu_apl_probe(struct udevice *dev)
{
	if (gd->flags & GD_FLG_RELOC) {
		int ret;

		ret = soc_core_init();
		if (ret)
			return log_ret(ret);
	}

	return 0;
}

#ifdef CONFIG_ACPIGEN
struct acpi_ops apl_cpu_acpi_ops = {
	.fill_ssdt	= acpi_cpu_fill_ssdt,
};
#endif

static const struct cpu_ops cpu_x86_apl_ops = {
	.get_desc	= cpu_x86_get_desc,
	.get_info	= apl_get_info,
	.get_count	= cpu_x86_get_count,
	.get_vendor	= cpu_x86_get_vendor,
};

static const struct udevice_id cpu_x86_apl_ids[] = {
	{ .compatible = "intel,apl-cpu" },
	{ }
};

U_BOOT_DRIVER(intel_apl_cpu) = {
	.name		= "intel_apl_cpu",
	.id		= UCLASS_CPU,
	.of_match	= cpu_x86_apl_ids,
	.bind		= cpu_x86_bind,
	.probe		= cpu_apl_probe,
	.ops		= &cpu_x86_apl_ops,
	ACPI_OPS_PTR(&apl_cpu_acpi_ops)
	.flags		= DM_FLAG_PRE_RELOC,
};
