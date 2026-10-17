// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020-2022 Marvell International Ltd.
 */

#include <dm.h>
#include <dm/uclass.h>
#include <env.h>
#include <iomux.h>
#include <asm/global_data.h>
#include <linux/bitfield.h>
#include <linux/bitops.h>
#include <linux/compat.h>
#include <linux/io.h>
#include <mach/clock.h>
#include <mach/cavm-reg.h>
#include <mach/cvmx-bootmem.h>
#include <mach/cvmx-regs.h>
#include <mach/cvmx-sata-defs.h>
#include <mach/octeon-model.h>
#include <mach/octeon-feature.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Important:
 * This address cannot be changed as the PCI console tool relies on exactly
 * this value!
 */
#define BOOTLOADER_BOOTMEM_DESC_ADDR	0x6c100
#define BOOTLOADER_BOOTMEM_DESC_SPACE	(BOOTLOADER_BOOTMEM_DESC_ADDR + 0x8)

#define OCTEON_RESERVED_LOW_BOOT_MEM_SIZE (1024 * 1024)

#define BOOTCMD_NAME			"pci-bootcmd"
#define CONSOLE_NAME			"pci-console@0"
#define OCTEON_BOOTLOADER_LOAD_MEM_NAME	"__tmp_load"

/*
 * TRUE for devices having registers with little-endian byte
 * order, FALSE for registers with native-endian byte order.
 * PCI mandates little-endian, USB and SATA are configurable,
 * but we chose little-endian for these.
 *
 * This table will be referened in the Octeon platform specific
 * mangle-port.h header.
 */
const bool octeon_should_swizzle_table[256] = {
	[0x00] = true,	/* bootbus/CF */
	[0x1b] = true,	/* PCI mmio window */
	[0x1c] = true,	/* PCI mmio window */
	[0x1d] = true,	/* PCI mmio window */
	[0x1e] = true,	/* PCI mmio window */
	[0x68] = true,	/* OCTEON III USB */
	[0x69] = true,	/* OCTEON III USB */
	[0x6f] = true,	/* OCTEON II USB */
};

static int get_clocks(void)
{
	const u64 ref_clock = PLL_REF_CLK;
	void __iomem *rst_boot;
	u64 val;

	rst_boot = ioremap(CAVM_RST_BOOT, 0);
	val = ioread64(rst_boot);
	gd->cpu_clk = ref_clock * FIELD_GET(RST_BOOT_C_MUL, val);
	gd->bus_clk = ref_clock * FIELD_GET(RST_BOOT_PNR_MUL, val);

	debug("%s: cpu: %lu, bus: %lu\n", __func__, gd->cpu_clk, gd->bus_clk);

	return 0;
}

/* Early mach init code run from flash */
int mach_cpu_init(void)
{
	void __iomem *mio_boot_reg_cfg0;

	/* Remap boot-bus 0x1fc0.0000 -> 0x1f40.0000 */
	/* ToDo: Move this to an early running bus (bootbus) DM driver */
	mio_boot_reg_cfg0 = ioremap(CAVM_MIO_BOOT_REG_CFG0, 0);
	clrsetbits_be64(mio_boot_reg_cfg0, 0xffff, 0x1f40);

	/* Get clocks and store them in GD */
	get_clocks();

	return 0;
}

/**
 * Returns number of cores
 *
 * Return:	number of CPU cores for the specified node
 */
static int cavm_octeon_num_cores(void)
{
	void __iomem *ciu_fuse;

	ciu_fuse = ioremap(CAVM_CIU_FUSE, 0);
	return fls64(ioread64(ciu_fuse) & 0xffffffffffff);
}

int print_cpuinfo(void)
{
	printf("SoC:   Octeon CN73xx (%d cores)\n", cavm_octeon_num_cores());

	return 0;
}

static int octeon_bootmem_init(void)
{
	int ret;

	/* Call old single-node func: it uses only gd->ram_size */
	ret = cvmx_bootmem_phy_mem_list_init(gd->ram_size,
					     OCTEON_RESERVED_LOW_BOOT_MEM_SIZE,
					     (void *)CKSEG0ADDR(BOOTLOADER_BOOTMEM_DESC_SPACE));
	if (!ret) {
		printf("FATAL: Error initializing bootmem list\n");
		return -ENOSPC;
	}

	/*
	 * Put bootmem descriptor address in known location for host.
	 * Make sure it is not in kseg0, as we want physical address
	 */
	writeq((u64)__cvmx_bootmem_internal_get_desc_ptr() & 0x7fffffffull,
	       (void *)CKSEG0ADDR(BOOTLOADER_BOOTMEM_DESC_ADDR));

	debug("Reserving first 1MB of memory\n");
	ret = cvmx_bootmem_reserve_memory(0, OCTEON_RESERVED_LOW_BOOT_MEM_SIZE,
					  "__low_reserved", 0);
	if (!ret)
		puts("Error reserving low 1MB of memory\n");

#ifdef DEBUG
	cvmx_bootmem_phy_list_print();
#endif

	return 0;
}

static int octeon_configure_load_memory(void)
{
	char *eptr;
	u32 addr;
	u32 size;
	int ret;

	eptr = env_get("octeon_reserved_mem_load_size");
	if (!eptr || !strcmp("auto", eptr)) {
		/*
		 * Pick a size that we think is appropriate.
		 * Please note that for small memory boards this guess
		 * will likely not be ideal.
		 * Please pick a specific size for boards/applications
		 * that require it.
		 */
		if (gd->ram_size <= (256 << 20)) {
			size = min_t(u64, (128 << 20),
				     ((gd->ram_size * 2) / 5) & ~0xFFFFF);
		} else {
			size = min_t(u64, (256 << 20),
				     ((gd->ram_size - (256 << 20)) / 3) & ~0xFFFFF);
		}
	} else {
		size = simple_strtol(eptr, NULL, 16);
		debug("octeon_reserved_mem_load_size=0x%08x\n", size);
	}

	if (size) {
		debug("Linux reserved load size 0x%08x\n", size);
		eptr = env_get("octeon_reserved_mem_load_base");
		if (!eptr || !strcmp("auto", eptr)) {
			u64 mem_top;
			/*
			 * Leave some room for previous allocations that
			 * are made starting at the top of the low
			 * 256 Mbytes of DRAM
			 */
			int adjust = (1 << 20);

			if (gd->ram_size <= (512 << 20))
				adjust = (17 << 20);

			/* Put block at the top of DDR0, or bottom of DDR2 */
			if ((gd->ram_size <= (256 << 20)) ||
			    (size > (gd->ram_size - (256 << 20)))) {
				mem_top = min_t(u64, gd->ram_size - adjust,
						(256 << 20) - adjust);
			} else if ((gd->ram_size <= (512 << 20)) ||
				   (size > (gd->ram_size - (512 << 20)))) {
				mem_top = min_t(u64, gd->ram_size - adjust,
						(512 << 20) - adjust);
			} else {
				/*
				 * We have enough room, so set
				 * mem_top so that the block is
				 * at the base of the DDR2
				 * segment
				 */
				mem_top = (512 << 20) + size;
			}

			/*
			 * Adjust for boot bus memory hole on OCTEON II
			 * and later.
			 */
			if ((gd->ram_size > (256 << 20)))
				mem_top += (256 << 20);

			debug("Adjusted memory top is 0x%llx\n", mem_top);
			addr = mem_top - size;
			if (addr > (512 << 20))
				addr = (512 << 20);
			if ((addr >= (256 << 20)) && addr < (512 << 20)) {
				/*
				 * The address landed in the boot-bus
				 * memory hole.  Dig it out of the hole.
				 */
				addr = (512 << 20);
			}
		} else {
			addr = simple_strtol(eptr, NULL, 16);
		}

		ret = cvmx_bootmem_phy_named_block_alloc(size, addr,
							 addr + size, 0,
							 OCTEON_BOOTLOADER_LOAD_MEM_NAME,
							 0);
		if (ret < 0) {
			printf("ERROR: Unable to allocate bootloader reserved memory (addr: 0x%x, size: 0x%x).\n",
			       addr, size);
		} else {
			/*
			 * Set default load address to base of memory
			 * reserved for loading. The setting of the
			 * env. variable also sets the load_addr global
			 * variable.
			 * This environment variable is overridden each
			 * boot if a reserved block is created.
			 */
			char str[20];

			snprintf(str, sizeof(str), "0x%x", addr);
			env_set("loadaddr", str);
			debug("Setting load address to 0x%08x, size 0x%x\n",
			      addr, size);
		}
		return 0;
	}

	printf("WARNING: No reserved memory for image loading.\n");
	return -1;
}

static int init_pcie_console(void)
{
	char *stdinname = env_get("stdin");
	char *stdoutname = env_get("stdout");
	char *stderrname = env_get("stderr");
	struct udevice *pcie_console_dev = NULL;
	bool stdin_set, stdout_set, stderr_set;
	char iomux_name[128];
	int ret = 0;

	debug("%s: stdin: %s, stdout: %s, stderr: %s\n", __func__, stdinname,
	      stdoutname, stderrname);
	if (!stdinname) {
		env_set("stdin", "serial");
		stdinname = env_get("stdin");
	}
	if (!stdoutname) {
		env_set("stdout", "serial");
		stdoutname = env_get("stdout");
	}
	if (!stderrname) {
		env_set("stderr", "serial");
		stderrname = env_get("stderr");
	}

	if (!stdinname || !stdoutname || !stderrname) {
		printf("%s: Error setting environment variables for serial\n",
		       __func__);
		return -1;
	}

	stdin_set = !!strstr(stdinname, CONSOLE_NAME);
	stdout_set = !!strstr(stdoutname, CONSOLE_NAME);
	stderr_set = !!strstr(stderrname, CONSOLE_NAME);

	log_debug("stdin: %d, \"%s\", stdout: %d, \"%s\", stderr: %d, \"%s\"\n",
		  stdin_set, stdinname, stdout_set, stdoutname,
		  stderr_set, stderrname);
	ret = uclass_get_device_by_name(UCLASS_SERIAL, CONSOLE_NAME,
					&pcie_console_dev);
	if (ret || !pcie_console_dev) {
		debug("%s: No PCI console device %s found\n", __func__,
		      CONSOLE_NAME);
		return 0;
	}

	if (stdin_set)
		strncpy(iomux_name, stdinname, sizeof(iomux_name));
	else
		snprintf(iomux_name, sizeof(iomux_name), "%s,%s",
			 stdinname, pcie_console_dev->name);

	ret = iomux_doenv(stdin, iomux_name);
	if (ret) {
		log_err("%s: Error setting I/O stdin MUX to %s\n",
			__func__, iomux_name);
		return ret;
	}

	if (!stdin_set)
		env_set("stdin", iomux_name);

	if (stdout_set)
		strncpy(iomux_name, stdoutname, sizeof(iomux_name));
	else
		snprintf(iomux_name, sizeof(iomux_name), "%s,%s", stdoutname,
			 pcie_console_dev->name);

	ret = iomux_doenv(stdout, iomux_name);
	if (ret) {
		log_err("%s: Error setting I/O stdout MUX to %s\n",
			__func__, iomux_name);
		return ret;
	}
	if (!stdout_set)
		env_set("stdout", iomux_name);

	if (stderr_set)
		strncpy(iomux_name, stderrname, sizeof(iomux_name));
	else
		snprintf(iomux_name, sizeof(iomux_name), "%s,%s", stderrname,
			 pcie_console_dev->name);

	ret = iomux_doenv(stderr, iomux_name);
	if (ret) {
		log_err("%s: Error setting I/O stderr MUX to %s\n",
			__func__, iomux_name);
		return ret;
	}

	if (!stderr_set)
		env_set("stderr", iomux_name);

	debug("%s: stdin: %s, stdout: %s, stderr: %s, ret: %d\n",
	      __func__, env_get("stdin"), env_get("stdout"),
	      env_get("stderr"), ret);

	return ret;
}

static int init_bootcmd_console(void)
{
	char *stdinname = env_get("stdin");
	struct udevice *bootcmd_dev = NULL;
	bool stdin_set;
	char iomux_name[128];
	int ret = 0;

	debug("%s: stdin before: %s\n", __func__,
	      stdinname ? stdinname : "NONE");
	if (!stdinname) {
		env_set("stdin", "serial");
		stdinname = env_get("stdin");
	}
	stdin_set = !!strstr(stdinname, BOOTCMD_NAME);
	ret = uclass_get_device_by_driver(UCLASS_SERIAL,
					  DM_DRIVER_GET(octeon_bootcmd),
					  &bootcmd_dev);
	if (ret) {
		log_err("%s: Error getting %s serial class\n", __func__,
			BOOTCMD_NAME);
	} else if (bootcmd_dev) {
		if (stdin_set)
			strncpy(iomux_name, stdinname, sizeof(iomux_name));
		else
			snprintf(iomux_name, sizeof(iomux_name), "%s,%s",
				 stdinname, bootcmd_dev->name);
		ret = iomux_doenv(stdin, iomux_name);
		if (ret)
			log_err("%s: Error %d enabling the PCI bootcmd input console \"%s\"\n",
				__func__, ret, iomux_name);
		if (!stdin_set)
			env_set("stdin", iomux_name);
	}

	debug("%s: Set iomux and stdin to %s (ret: %d)\n",
	      __func__, iomux_name, ret);
	return ret;
}

static void configure_lmtdma_window(void)
{
	u64 tmp;
	u64 addr;
	u64 end_addr;

	CVMX_MF_CVM_MEM_CTL(tmp);
	tmp &= ~0x1ffull;
	tmp |= 0x104ull;

	/* enable LMTDMA */
	tmp |= (1ull << 51);
	/* configure scratch line 2 for LMT */
	/* TODO: reserve this scratch line, so that others will not use it */
	/* TODO: store LMTLINE in global var */
	tmp |= (CVMX_PKO_LMTLINE << 45);
	/* clear LMTLINE in scratch */
	addr = CVMX_PKO_LMTLINE * CVMX_CACHE_LINE_SIZE;
	end_addr = addr + CVMX_CACHE_LINE_SIZE;

	while (addr < end_addr) {
		*CASTPTR(volatile u64, addr + CVMX_SCRATCH_BASE) = (u64)0;
		addr += 8;
	}
	CVMX_MT_CVM_MEM_CTL(tmp);
}

int arch_early_init_r(void)
{
	int ret;

	/*
	 * Needs to be called pretty early, so that e.g. networking etc
	 * can access the bootmem infrastructure
	 */
	ret = octeon_bootmem_init();
	if (ret)
		return ret;

	if (octeon_has_feature(OCTEON_FEATURE_PKO3))
		configure_lmtdma_window();

	return 0;
}

int arch_misc_init(void)
{
	int ret;

	ret = octeon_configure_load_memory();
	if (ret)
		return ret;

	if (IS_ENABLED(CONFIG_OCTEON_SERIAL_PCIE_CONSOLE))
		init_pcie_console();

	if (IS_ENABLED(CONFIG_OCTEON_SERIAL_BOOTCMD))
		init_bootcmd_console();

	return 0;
}

int board_ahci_enable(void)
{
	cvmx_sata_uctl_shim_cfg_t shim_cfg;

	/*
	 * Configure proper endian swapping for the AHCI port so that the
	 * common AHCI code can be used
	 */
	shim_cfg.u64 = csr_rd(CVMX_SATA_UCTL_SHIM_CFG);
	shim_cfg.s.dma_endian_mode = 1;
	/* Use 1 for LE mode when running BE, or 3 for BE mode running BE */
	shim_cfg.s.csr_endian_mode = 3;	/* Don't byte swap */
	shim_cfg.s.dma_read_cmd = 1; /* No allocate L2C */
	csr_wr(CVMX_SATA_UCTL_SHIM_CFG, shim_cfg.u64);

	return 0;
}
