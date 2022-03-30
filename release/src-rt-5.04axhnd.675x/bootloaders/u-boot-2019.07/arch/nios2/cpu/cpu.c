// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2004, Psyent Corporation <www.psyent.com>
 * Scott McNutt <smcnutt@psyent.com>
 */

#include <common.h>
#include <cpu.h>
#include <dm.h>
#include <errno.h>
#include <asm/cache.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_DISPLAY_CPUINFO
int print_cpuinfo(void)
{
	printf("CPU:   Nios-II\n");
	return 0;
}
#endif /* CONFIG_DISPLAY_CPUINFO */

#ifdef CONFIG_ALTERA_SYSID
int checkboard(void)
{
	display_sysid();
	return 0;
}
#endif

int do_reset(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	disable_interrupts();
	/* indirect call to go beyond 256MB limitation of toolchain */
	nios2_callr(gd->arch.reset_addr);
	return 0;
}

/*
 * COPY EXCEPTION TRAMPOLINE -- copy the tramp to the
 * exception address. Define CONFIG_ROM_STUBS to prevent
 * the copy (e.g. exception in flash or in other
 * softare/firmware component).
 */
#ifndef CONFIG_ROM_STUBS
static void copy_exception_trampoline(void)
{
	extern int _except_start, _except_end;
	void *except_target = (void *)gd->arch.exception_addr;

	if (&_except_start != except_target) {
		memcpy(except_target, &_except_start,
		       &_except_end - &_except_start);
		flush_cache(gd->arch.exception_addr,
			    &_except_end - &_except_start);
	}
}
#endif

int arch_cpu_init_dm(void)
{
	struct udevice *dev;
	int ret;

	ret = uclass_first_device_err(UCLASS_CPU, &dev);
	if (ret)
		return ret;

	gd->ram_size = CONFIG_SYS_SDRAM_SIZE;
#ifndef CONFIG_ROM_STUBS
	copy_exception_trampoline();
#endif

	return 0;
}

static int altera_nios2_get_desc(struct udevice *dev, char *buf, int size)
{
	const char *cpu_name = "Nios-II";

	if (size < strlen(cpu_name))
		return -ENOSPC;
	strcpy(buf, cpu_name);

	return 0;
}

static int altera_nios2_get_info(struct udevice *dev, struct cpu_info *info)
{
	info->cpu_freq = gd->cpu_clk;
	info->features = (1 << CPU_FEAT_L1_CACHE) |
		(gd->arch.has_mmu ? (1 << CPU_FEAT_MMU) : 0);

	return 0;
}

static int altera_nios2_get_count(struct udevice *dev)
{
	return 1;
}

static int altera_nios2_probe(struct udevice *dev)
{
	const void *blob = gd->fdt_blob;
	int node = dev_of_offset(dev);

	gd->cpu_clk = fdtdec_get_int(blob, node,
		"clock-frequency", 0);
	gd->arch.dcache_line_size = fdtdec_get_int(blob, node,
		"dcache-line-size", 0);
	gd->arch.icache_line_size = fdtdec_get_int(blob, node,
		"icache-line-size", 0);
	gd->arch.dcache_size = fdtdec_get_int(blob, node,
		"dcache-size", 0);
	gd->arch.icache_size = fdtdec_get_int(blob, node,
		"icache-size", 0);
	gd->arch.reset_addr = fdtdec_get_int(blob, node,
		"altr,reset-addr", 0);
	gd->arch.exception_addr = fdtdec_get_int(blob, node,
		"altr,exception-addr", 0);
	gd->arch.has_initda = fdtdec_get_int(blob, node,
		"altr,has-initda", 0);
	gd->arch.has_mmu = fdtdec_get_int(blob, node,
		"altr,has-mmu", 0);
	gd->arch.io_region_base = gd->arch.has_mmu ? 0xe0000000 : 0x80000000;
	gd->arch.mem_region_base = gd->arch.has_mmu ? 0xc0000000 : 0x00000000;
	gd->arch.physaddr_mask = gd->arch.has_mmu ? 0x1fffffff : 0x7fffffff;

	return 0;
}

static const struct cpu_ops altera_nios2_ops = {
	.get_desc	= altera_nios2_get_desc,
	.get_info	= altera_nios2_get_info,
	.get_count	= altera_nios2_get_count,
};

static const struct udevice_id altera_nios2_ids[] = {
	{ .compatible = "altr,nios2-1.0" },
	{ .compatible = "altr,nios2-1.1" },
	{ }
};

U_BOOT_DRIVER(altera_nios2) = {
	.name		= "altera_nios2",
	.id		= UCLASS_CPU,
	.of_match	= altera_nios2_ids,
	.probe		= altera_nios2_probe,
	.ops		= &altera_nios2_ops,
	.flags		= DM_FLAG_PRE_RELOC,
};

/* This is a dummy function on nios2 */
int dram_init(void)
{
	return 0;
}
