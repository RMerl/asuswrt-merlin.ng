// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 Fraunhofer AISEC,
 * Lukas Auer <lukas.auer@aisec.fraunhofer.de>
 */

#include <common.h>
#include <dm.h>
#include <asm/barrier.h>
#include <asm/smp.h>

DECLARE_GLOBAL_DATA_PTR;

/**
 * riscv_send_ipi() - Send inter-processor interrupt (IPI)
 *
 * Platform code must provide this function.
 *
 * @hart: Hart ID of receiving hart
 * @return 0 if OK, -ve on error
 */
extern int riscv_send_ipi(int hart);

/**
 * riscv_clear_ipi() - Clear inter-processor interrupt (IPI)
 *
 * Platform code must provide this function.
 *
 * @hart: Hart ID of hart to be cleared
 * @return 0 if OK, -ve on error
 */
extern int riscv_clear_ipi(int hart);

static int send_ipi_many(struct ipi_data *ipi)
{
	ofnode node, cpus;
	u32 reg;
	int ret;

	cpus = ofnode_path("/cpus");
	if (!ofnode_valid(cpus)) {
		pr_err("Can't find cpus node!\n");
		return -EINVAL;
	}

	ofnode_for_each_subnode(node, cpus) {
		/* skip if hart is marked as not available in the device tree */
		if (!ofnode_is_available(node))
			continue;

		/* read hart ID of CPU */
		ret = ofnode_read_u32(node, "reg", &reg);
		if (ret)
			continue;

		/* skip if it is the hart we are running on */
		if (reg == gd->arch.boot_hart)
			continue;

		if (reg >= CONFIG_NR_CPUS) {
			pr_err("Hart ID %d is out of range, increase CONFIG_NR_CPUS\n",
			       reg);
			continue;
		}

#ifndef CONFIG_XIP
		/* skip if hart is not available */
		if (!(gd->arch.available_harts & (1 << reg)))
			continue;
#endif

		gd->arch.ipi[reg].addr = ipi->addr;
		gd->arch.ipi[reg].arg0 = ipi->arg0;
		gd->arch.ipi[reg].arg1 = ipi->arg1;

		ret = riscv_send_ipi(reg);
		if (ret) {
			pr_err("Cannot send IPI to hart %d\n", reg);
			return ret;
		}
	}

	return 0;
}

void handle_ipi(ulong hart)
{
	int ret;
	void (*smp_function)(ulong hart, ulong arg0, ulong arg1);

	if (hart >= CONFIG_NR_CPUS)
		return;

	ret = riscv_clear_ipi(hart);
	if (ret) {
		pr_err("Cannot clear IPI of hart %ld\n", hart);
		return;
	}

	__smp_mb();

	smp_function = (void (*)(ulong, ulong, ulong))gd->arch.ipi[hart].addr;
	invalidate_icache_all();

	smp_function(hart, gd->arch.ipi[hart].arg0, gd->arch.ipi[hart].arg1);
}

int smp_call_function(ulong addr, ulong arg0, ulong arg1)
{
	int ret = 0;
	struct ipi_data ipi;

	ipi.addr = addr;
	ipi.arg0 = arg0;
	ipi.arg1 = arg1;

	ret = send_ipi_many(&ipi);

	return ret;
}
