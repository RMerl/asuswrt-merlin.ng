// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018, Bin Meng <bmeng.cn@gmail.com>
 */

#include <common.h>
#include <cpu.h>
#include <dm.h>
#include <log.h>
#include <asm/csr.h>
#include <asm/encoding.h>
#include <dm/uclass-internal.h>

/*
 * The variables here must be stored in the data section since they are used
 * before the bss section is available.
 */
#ifdef CONFIG_OF_PRIOR_STAGE
phys_addr_t prior_stage_fdt_address __attribute__((section(".data")));
#endif
#ifndef CONFIG_XIP
u32 hart_lottery __attribute__((section(".data"))) = 0;

/*
 * The main hart running U-Boot has acquired available_harts_lock until it has
 * finished initialization of global data.
 */
u32 available_harts_lock = 1;
#endif

static inline bool supports_extension(char ext)
{
#ifdef CONFIG_CPU
	struct udevice *dev;
	char desc[32];

	uclass_find_first_device(UCLASS_CPU, &dev);
	if (!dev) {
		debug("unable to find the RISC-V cpu device\n");
		return false;
	}
	if (!cpu_get_desc(dev, desc, sizeof(desc))) {
		/* skip the first 4 characters (rv32|rv64) */
		if (strchr(desc + 4, ext))
			return true;
	}

	return false;
#else  /* !CONFIG_CPU */
#ifdef CONFIG_RISCV_MMODE
	return csr_read(misa) & (1 << (ext - 'a'));
#else  /* !CONFIG_RISCV_MMODE */
#warning "There is no way to determine the available extensions in S-mode."
#warning "Please convert your board to use the RISC-V CPU driver."
	return false;
#endif /* CONFIG_RISCV_MMODE */
#endif /* CONFIG_CPU */
}

static int riscv_cpu_probe(void)
{
#ifdef CONFIG_CPU
	int ret;

	/* probe cpus so that RISC-V timer can be bound */
	ret = cpu_probe_all();
	if (ret)
		return log_msg_ret("RISC-V cpus probe failed\n", ret);
#endif

	return 0;
}

int arch_cpu_init_dm(void)
{
	int ret;

	ret = riscv_cpu_probe();
	if (ret)
		return ret;

	/* Enable FPU */
	if (supports_extension('d') || supports_extension('f')) {
		csr_set(MODE_PREFIX(status), MSTATUS_FS);
		csr_write(fcsr, 0);
	}

	if (CONFIG_IS_ENABLED(RISCV_MMODE)) {
		/*
		 * Enable perf counters for cycle, time,
		 * and instret counters only
		 */
		csr_write(mcounteren, GENMASK(2, 0));

		/* Disable paging */
		if (supports_extension('s'))
			csr_write(satp, 0);
	}

	return 0;
}

int arch_early_init_r(void)
{
	return riscv_cpu_probe();
}
