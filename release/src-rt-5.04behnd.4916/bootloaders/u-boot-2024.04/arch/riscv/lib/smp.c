// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 Fraunhofer AISEC,
 * Lukas Auer <lukas.auer@aisec.fraunhofer.de>
 */

#include <cpu_func.h>
#include <dm.h>
#include <asm/barrier.h>
#include <asm/global_data.h>
#include <asm/smp.h>
#include <linux/printk.h>

DECLARE_GLOBAL_DATA_PTR;

static int send_ipi_many(struct ipi_data *ipi, int wait)
{
	ofnode node, cpus;
	u32 reg;
	int ret, pending;

	cpus = ofnode_path("/cpus");
	if (!ofnode_valid(cpus)) {
		pr_err("Can't find cpus node!\n");
		return -EINVAL;
	}

	ofnode_for_each_subnode(node, cpus) {
		/* skip if hart is marked as not available in the device tree */
		if (!ofnode_is_enabled(node))
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

#if !CONFIG_IS_ENABLED(XIP)
#ifdef CONFIG_AVAILABLE_HARTS
		/* skip if hart is not available */
		if (!(gd->arch.available_harts & (1 << reg)))
			continue;
#endif
#endif

		gd->arch.ipi[reg].addr = ipi->addr;
		gd->arch.ipi[reg].arg0 = ipi->arg0;
		gd->arch.ipi[reg].arg1 = ipi->arg1;

		/*
		 * Ensure valid only becomes set when the IPI parameters are
		 * set. An IPI may already be pending on other harts, so we
		 * need a way to signal that the IPI device has been
		 * initialized, and that it is ok to call the function.
		 */
		__smp_store_release(&gd->arch.ipi[reg].valid, 1);

		ret = riscv_send_ipi(reg);
		if (ret) {
			pr_err("Cannot send IPI to hart %d\n", reg);
			return ret;
		}

		if (wait) {
			pending = 1;
			while (pending) {
				ret = riscv_get_ipi(reg, &pending);
				if (ret)
					return ret;
			}
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

	/*
	 * If valid is not set, then U-Boot has not requested the IPI. The
	 * IPI device may not be initialized, so all we can do is wait for
	 * U-Boot to initialize it and send an IPI
	 */
	if (!__smp_load_acquire(&gd->arch.ipi[hart].valid))
		return;

	smp_function = (void (*)(ulong, ulong, ulong))gd->arch.ipi[hart].addr;
	invalidate_icache_all();

	/*
	 * Clear the IPI to acknowledge the request before jumping to the
	 * requested function.
	 */
	ret = riscv_clear_ipi(hart);
	if (ret) {
		pr_err("Cannot clear IPI of hart %ld (error %d)\n", hart, ret);
		return;
	}

	smp_function(hart, gd->arch.ipi[hart].arg0, gd->arch.ipi[hart].arg1);
}

int smp_call_function(ulong addr, ulong arg0, ulong arg1, int wait)
{
	struct ipi_data ipi = {
		.addr = addr,
		.arg0 = arg0,
		.arg1 = arg1,
	};

	return send_ipi_many(&ipi, wait);
}
