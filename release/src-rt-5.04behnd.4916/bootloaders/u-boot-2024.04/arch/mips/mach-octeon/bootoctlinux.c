// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Stefan Roese <sr@denx.de>
 */

#include <command.h>
#include <config.h>
#include <cpu_func.h>
#include <dm.h>
#include <elf.h>
#include <env.h>
#include <asm/global_data.h>

#include <asm/io.h>
#include <linux/compat.h>
#include <linux/ctype.h>
#include <linux/delay.h>
#include <linux/io.h>

#include <mach/cvmx-coremask.h>
#include <mach/cvmx-bootinfo.h>
#include <mach/cvmx-bootmem.h>
#include <mach/cvmx-regs.h>
#include <mach/cvmx-fuse.h>
#include <mach/octeon-model.h>
#include <mach/octeon-feature.h>
#include <mach/bootoct_cmd.h>
#include <mach/cvmx-ciu-defs.h>

DECLARE_GLOBAL_DATA_PTR;

/* ToDo: Revisit these settings */
#define OCTEON_RESERVED_LOW_MEM_SIZE		(512 * 1024)
#define OCTEON_RESERVED_LOW_BOOT_MEM_SIZE	(1024 * 1024)
#define BOOTLOADER_BOOTMEM_DESC_SPACE		(1024 * 1024)

/* Default stack and heap sizes, in bytes */
#define DEFAULT_STACK_SIZE			(1 * 1024 * 1024)
#define DEFAULT_HEAP_SIZE			(3 * 1024 * 1024)

/**
 * NOTE: This must duplicate octeon_boot_descriptor_t in the toolchain
 * octeon-app-init.h file.
 */
enum {
	/* If set, core should do app-wide init, only one core per app will have
	 * this flag set.
	 */
	BOOT_FLAG_INIT_CORE     = 1,
	OCTEON_BL_FLAG_DEBUG    = 1 << 1,
	OCTEON_BL_FLAG_NO_MAGIC = 1 << 2,
	/* If set, use uart1 for console */
	OCTEON_BL_FLAG_CONSOLE_UART1 = 1 << 3,
	OCTEON_BL_FLAG_CONSOLE_PCI = 1 << 4,	/* If set, use PCI console */
	/* Call exit on break on serial port */
	OCTEON_BL_FLAG_BREAK    = 1 << 5,
	/*
	 * Be sure to update OCTEON_APP_INIT_H_VERSION when new fields are added
	 * and to conditionalize the new flag's usage based on the version.
	 */
} octeon_boot_descriptor_flag;

/**
 * NOTE: This must duplicate octeon_boot_descriptor_t in the toolchain
 * octeon-app-init.h file.
 */
#ifndef OCTEON_CURRENT_DESC_VERSION
# define OCTEON_CURRENT_DESC_VERSION	7
#endif
/**
 * NOTE: This must duplicate octeon_boot_descriptor_t in the toolchain
 * octeon-app-init.h file.
 */
/* Version 7 changes: Change names of deprecated fields */
#ifndef OCTEON_ARGV_MAX_ARGS
# define OCTEON_ARGV_MAX_ARGS		64
#endif

/**
 * NOTE: This must duplicate octeon_boot_descriptor_t in the toolchain
 * octeon-app-init.h file.
 */
#ifndef OCTEON_SERIAL_LEN
# define OCTEON_SERIAL_LEN		20
#endif

/**
 * Bootloader structure used to pass info to Octeon executive startup code.
 * NOTE: all fields are deprecated except for:
 *  * desc_version
 *  * desc_size,
 *  * heap_base
 *  * heap_end
 *  * eclock_hz
 *  * flags
 *  * argc
 *  * argv
 *  * cvmx_desc_vaddr
 *  * debugger_flags_base_addr
 *
 *  All other fields have been moved to the cvmx_descriptor, and the new
 *  fields should be added there. They are left as placeholders in this
 *  structure for binary compatibility.
 *
 * NOTE: This structure must match what is in the toolchain octeon-app-init.h
 * file.
 */
struct octeon_boot_descriptor {
	/* Start of block referenced by assembly code - do not change! */
	u32 desc_version;
	u32 desc_size;
	u64 stack_top;
	u64 heap_base;
	u64 heap_end;
	u64 deprecated17;
	u64 deprecated16;
	/* End of block referenced by assembly code - do not change! */
	u32 deprecated18;
	u32 deprecated15;
	u32 deprecated14;
	u32 argc;  /* argc for main() */
	u32 argv[OCTEON_ARGV_MAX_ARGS];  /* argv for main() */
	u32 flags;   /* Flags for application */
	u32 core_mask;   /* Coremask running this image */
	u32 dram_size;  /* DEPRECATED, DRAM size in megabyes. Used up to SDK 1.8.1 */
	u32 phy_mem_desc_addr;
	u32 debugger_flags_base_addr;  /* used to pass flags from app to debugger. */
	u32 eclock_hz;  /* CPU clock speed, in hz. */
	u32 deprecated10;
	u32 deprecated9;
	u16 deprecated8;
	u8  deprecated7;
	u8  deprecated6;
	u16 deprecated5;
	u8  deprecated4;
	u8  deprecated3;
	char deprecated2[OCTEON_SERIAL_LEN];
	u8  deprecated1[6];
	u8  deprecated0;
	u64 cvmx_desc_vaddr;  /* Address of cvmx descriptor */
};

static struct octeon_boot_descriptor boot_desc[CVMX_MIPS_MAX_CORES];
static struct cvmx_bootinfo cvmx_bootinfo_array[CVMX_MIPS_MAX_CORES];

/**
 * Programs the boot bus moveable region
 * @param	base	base address to place the boot bus moveable region
 *			(bits [31:7])
 * @param	region_num	Selects which region, 0 or 1 for node 0,
 *				2 or 3 for node 1
 * @param	enable		Set true to enable, false to disable
 * @param	data		Pointer to data to put in the region, up to
 *				16 dwords.
 * @param	num_words	Number of data dwords (up to 32)
 *
 * Return:	0 for success, -1 on error
 */
static int octeon_set_moveable_region(u32 base, int region_num,
				      bool enable, const u64 *data,
				      unsigned int num_words)
{
	int node = region_num >> 1;
	u64 val;
	int i;
	u8 node_mask = 0x01;	/* ToDo: Currently only one node is supported */

	debug("%s(0x%x, %d, %d, %p, %u)\n", __func__, base, region_num, enable,
	      data, num_words);

	if (num_words > 32) {
		printf("%s: Too many words (%d) for region %d\n", __func__,
		       num_words, region_num);
		return -1;
	}

	if (base & 0x7f) {
		printf("%s: Error: base address 0x%x must be 128 byte aligned\n",
		       __func__, base);
		return -1;
	}

	if (region_num > (node_mask > 1 ? 3 : 1)) {
		printf("%s: Region number %d out of range\n",
		       __func__, region_num);
		return -1;
	}

	if (!data && num_words > 0) {
		printf("%s: Error: NULL data\n", __func__);
		return -1;
	}

	region_num &= 1;

	val = MIO_BOOT_LOC_CFG_EN |
		FIELD_PREP(MIO_BOOT_LOC_CFG_BASE, base >> 7);
	debug("%s: Setting MIO_BOOT_LOC_CFG(%d) on node %d to 0x%llx\n",
	      __func__, region_num, node, val);
	csr_wr(CVMX_MIO_BOOT_LOC_CFGX(region_num & 1), val);

	val = FIELD_PREP(MIO_BOOT_LOC_ADR_ADR, (region_num ? 0x80 : 0x00) >> 3);
	debug("%s: Setting MIO_BOOT_LOC_ADR start to 0x%llx\n", __func__, val);
	csr_wr(CVMX_MIO_BOOT_LOC_ADR, val);

	for (i = 0; i < num_words; i++) {
		debug("  0x%02llx: 0x%016llx\n",
		      csr_rd(CVMX_MIO_BOOT_LOC_ADR), data[i]);
		csr_wr(CVMX_MIO_BOOT_LOC_DAT, data[i]);
	}

	return 0;
}

/**
 * Parse comma separated numbers into an array
 *
 * @param[out] values values read for each node
 * @param[in] str string to parse
 * @param base 0 for auto, otherwise 8, 10 or 16 for the number base
 *
 * Return: number of values read.
 */
static int octeon_parse_nodes(u64 values[CVMX_MAX_NODES],
			      const char *str, int base)
{
	int node = 0;
	char *sep;

	do {
		debug("Parsing node %d: \"%s\"\n", node, str);
		values[node] = simple_strtoull(str, &sep, base);
		debug("  node %d: 0x%llx\n", node, values[node]);
		str = sep + 1;
	} while (++node < CVMX_MAX_NODES && *sep == ',');

	debug("%s: returning %d\n", __func__, node);
	return node;
}

/**
 * Parse command line arguments
 *
 * @param argc			number of arguments
 * @param[in] argv		array of argument strings
 * @param cmd			command type
 * @param[out] boot_args	parsed values
 *
 * Return: number of arguments parsed
 */
int octeon_parse_bootopts(int argc, char *const argv[],
			  enum octeon_boot_cmd_type cmd,
			  struct octeon_boot_args *boot_args)
{
	u64 node_values[CVMX_MAX_NODES];
	int arg, j;
	int num_values;
	int node;
	u8 node_mask = 0x01;	/* ToDo: Currently only one node is supported */

	debug("%s(%d, %p, %d, %p)\n", __func__, argc, argv, cmd, boot_args);
	memset(boot_args, 0, sizeof(*boot_args));
	boot_args->stack_size = DEFAULT_STACK_SIZE;
	boot_args->heap_size = DEFAULT_HEAP_SIZE;
	boot_args->node_mask = 0;

	for (arg = 0; arg < argc; arg++) {
		debug("  argv[%d]: %s\n", arg, argv[arg]);
		if (cmd == BOOTOCT && !strncmp(argv[arg], "stack=", 6)) {
			boot_args->stack_size = simple_strtoul(argv[arg] + 6,
							       NULL, 0);
		} else if (cmd == BOOTOCT && !strncmp(argv[arg], "heap=", 5)) {
			boot_args->heap_size = simple_strtoul(argv[arg] + 5,
							      NULL, 0);
		} else if (!strncmp(argv[arg], "debug", 5)) {
			puts("setting debug flag!\n");
			boot_args->boot_flags |= OCTEON_BL_FLAG_DEBUG;
		} else if (cmd == BOOTOCT && !strncmp(argv[arg], "break", 5)) {
			puts("setting break flag!\n");
			boot_args->boot_flags |= OCTEON_BL_FLAG_BREAK;
		} else if (!strncmp(argv[arg], "forceboot", 9)) {
			boot_args->forceboot = true;
		} else if (!strncmp(argv[arg], "nodemask=", 9)) {
			boot_args->node_mask = hextoul(argv[arg] + 9, NULL);
		} else if (!strncmp(argv[arg], "numcores=", 9)) {
			memset(node_values, 0, sizeof(node_values));
			num_values = octeon_parse_nodes(node_values,
							argv[arg] + 9, 0);
			for (j = 0; j < num_values; j++)
				boot_args->num_cores[j] = node_values[j];
			boot_args->num_cores_set = true;
		} else if (!strncmp(argv[arg], "skipcores=", 10)) {
			memset(node_values, 0, sizeof(node_values));
			num_values = octeon_parse_nodes(node_values,
							argv[arg] + 10, 0);
			for (j = 0; j < num_values; j++)
				boot_args->num_skipped[j] = node_values[j];
			boot_args->num_skipped_set = true;
		} else if (!strncmp(argv[arg], "console_uart=", 13)) {
			boot_args->console_uart = simple_strtoul(argv[arg] + 13,
								 NULL, 0);
			if (boot_args->console_uart == 1) {
				boot_args->boot_flags |=
					OCTEON_BL_FLAG_CONSOLE_UART1;
			} else if (!boot_args->console_uart) {
				boot_args->boot_flags &=
					~OCTEON_BL_FLAG_CONSOLE_UART1;
			}
		} else if (!strncmp(argv[arg], "coremask=", 9)) {
			memset(node_values, 0, sizeof(node_values));
			num_values = octeon_parse_nodes(node_values,
							argv[arg] + 9, 16);
			for (j = 0; j < num_values; j++)
				cvmx_coremask_set64_node(&boot_args->coremask,
							 j, node_values[j]);
			boot_args->coremask_set = true;
		} else if (cmd == BOOTOCTLINUX &&
			   !strncmp(argv[arg], "namedblock=", 11)) {
			boot_args->named_block = argv[arg] + 11;
		} else if (!strncmp(argv[arg], "endbootargs", 11)) {
			boot_args->endbootargs = 1;
			arg++;
			if (argc >= arg && cmd != BOOTOCTLINUX)
				boot_args->app_name = argv[arg];
			break;
		} else {
			debug(" Unknown argument \"%s\"\n", argv[arg]);
		}
	}

	if (boot_args->coremask_set && boot_args->num_cores_set) {
		puts("Warning: both coremask and numcores are set, using coremask.\n");
	} else if (!boot_args->coremask_set && !boot_args->num_cores_set) {
		cvmx_coremask_set_core(&boot_args->coremask, 0);
		boot_args->coremask_set = true;
	} else if ((!boot_args->coremask_set) && boot_args->num_cores_set) {
		cvmx_coremask_for_each_node(node, node_mask)
			cvmx_coremask_set64_node(&boot_args->coremask, node,
				((1ull << boot_args->num_cores[node]) - 1) <<
					boot_args->num_skipped[node]);
		boot_args->coremask_set = true;
	}

	/* Update the node mask based on the coremask or the number of cores */
	for (j = 0; j < CVMX_MAX_NODES; j++) {
		if (cvmx_coremask_get64_node(&boot_args->coremask, j))
			boot_args->node_mask |= 1 << j;
	}

	debug("%s: return %d\n", __func__, arg);
	return arg;
}

int do_bootoctlinux(struct cmd_tbl *cmdtp, int flag, int argc,
		    char *const argv[])
{
	typedef void __noreturn (*kernel_entry_t)(int, ulong, ulong, ulong);
	kernel_entry_t kernel;
	struct octeon_boot_args boot_args;
	int arg_start = 1;
	int arg_count;
	u64 addr = 0;		/* Address of the ELF image     */
	int arg0;
	u64 arg1;
	u64 arg2;
	u64 arg3;
	int ret;
	struct cvmx_coremask core_mask;
	struct cvmx_coremask coremask_to_run;
	struct cvmx_coremask avail_coremask;
	int first_core;
	int core;
	const u64 *nmi_code;
	int num_dwords;
	u8 node_mask = 0x01;
	int i;

	cvmx_coremask_clear_all(&core_mask);
	cvmx_coremask_clear_all(&coremask_to_run);

	if (argc >= 2 && (isxdigit(argv[1][0]) && (isxdigit(argv[1][1]) ||
						   argv[1][1] == 'x' ||
						   argv[1][1] == 'X' ||
						   argv[1][1] == '\0'))) {
		addr = hextoul(argv[1], NULL);
		if (!addr)
			addr = CONFIG_SYS_LOAD_ADDR;
		arg_start++;
	}
	if (addr == 0)
		addr = CONFIG_SYS_LOAD_ADDR;

	debug("%s: arg start: %d\n", __func__, arg_start);
	arg_count = octeon_parse_bootopts(argc - arg_start, argv + arg_start,
					  BOOTOCTLINUX, &boot_args);

	debug("%s:\n"
	      " named block: %s\n"
	      " node mask: 0x%x\n"
	      " stack size: 0x%x\n"
	      " heap size: 0x%x\n"
	      " boot flags: 0x%x\n"
	      " force boot: %s\n"
	      " coremask set: %s\n"
	      " num cores set: %s\n"
	      " num skipped set: %s\n"
	      " endbootargs: %s\n",
	      __func__,
	      boot_args.named_block ? boot_args.named_block : "none",
	      boot_args.node_mask,
	      boot_args.stack_size,
	      boot_args.heap_size,
	      boot_args.boot_flags,
	      boot_args.forceboot ? "true" : "false",
	      boot_args.coremask_set ? "true" : "false",
	      boot_args.num_cores_set ? "true" : "false",
	      boot_args.num_skipped_set ? "true" : "false",
	      boot_args.endbootargs ? "true" : "false");
	debug(" num cores: ");
	for (i = 0; i < CVMX_MAX_NODES; i++)
		debug("%s%d", i > 0 ? ", " : "", boot_args.num_cores[i]);
	debug("\n num skipped: ");
	for (i = 0; i < CVMX_MAX_NODES; i++) {
		debug("%s%d", i > 0 ? ", " : "", boot_args.num_skipped[i]);
		debug("\n coremask:\n");
		cvmx_coremask_dprint(&boot_args.coremask);
	}

	if (boot_args.endbootargs) {
		debug("endbootargs set, adjusting argc from %d to %d, arg_count: %d, arg_start: %d\n",
		      argc, argc - (arg_count + arg_start), arg_count,
		      arg_start);
		argc -= (arg_count + arg_start);
		argv += (arg_count + arg_start);
	}

	/*
	 * numcores specification overrides a coremask on the same command line
	 */
	cvmx_coremask_copy(&core_mask, &boot_args.coremask);

	/*
	 * Remove cores from coremask based on environment variable stored in
	 * flash
	 */
	if (validate_coremask(&core_mask) != 0) {
		puts("Invalid coremask.\n");
		return 1;
	} else if (cvmx_coremask_is_empty(&core_mask)) {
		puts("Coremask is empty after coremask_override mask.  Nothing to do.\n");
		return 0;
	}

	if (cvmx_coremask_intersects(&core_mask, &coremask_to_run)) {
		puts("ERROR: Can't load code on core twice!  Provided coremask:\n");
		cvmx_coremask_print(&core_mask);
		puts("overlaps previously loaded coremask:\n");
		cvmx_coremask_print(&coremask_to_run);
		return -1;
	}

	debug("Setting up boot descriptor block with core mask:\n");
	cvmx_coremask_dprint(&core_mask);

	/*
	 * Add coremask to global mask of cores that have been set up and are
	 * runable
	 */
	cvmx_coremask_or(&coremask_to_run, &coremask_to_run, &core_mask);

	/*
	 * Load kernel ELF image, or try binary if ELF is not detected.
	 * This way the much smaller vmlinux.bin can also be started but
	 * has to be loaded at the correct address (ep as parameter).
	 */
	if (!valid_elf_image(addr))
		printf("Booting binary image instead (vmlinux.bin)...\n");
	else
		addr = load_elf_image_shdr(addr);

	/* Set kernel entry point */
	kernel = (kernel_entry_t)addr;

	/* Init bootmem list for Linux kernel booting */
	if (!cvmx_bootmem_phy_mem_list_init(
		    gd->ram_size, OCTEON_RESERVED_LOW_MEM_SIZE,
		    (void *)CKSEG0ADDR(BOOTLOADER_BOOTMEM_DESC_SPACE))) {
		printf("FATAL: Error initializing free memory list\n");
		return 0;
	}

	first_core = cvmx_coremask_get_first_core(&coremask_to_run);

	cvmx_coremask_for_each_core(core, &coremask_to_run) {
		debug("%s: Activating core %d\n",  __func__, core);

		cvmx_bootinfo_array[core].core_mask =
			cvmx_coremask_get32(&coremask_to_run);
		cvmx_coremask_copy(&cvmx_bootinfo_array[core].ext_core_mask,
				   &coremask_to_run);

		if (core == first_core)
			cvmx_bootinfo_array[core].flags |= BOOT_FLAG_INIT_CORE;

		cvmx_bootinfo_array[core].dram_size = gd->ram_size /
			(1024 * 1024);

		cvmx_bootinfo_array[core].dclock_hz = gd->mem_clk * 1000000;
		cvmx_bootinfo_array[core].eclock_hz = gd->cpu_clk;

		cvmx_bootinfo_array[core].led_display_base_addr = 0;
		cvmx_bootinfo_array[core].phy_mem_desc_addr =
			((u32)(u64)__cvmx_bootmem_internal_get_desc_ptr()) &
			0x7ffffff;

		cvmx_bootinfo_array[core].major_version = CVMX_BOOTINFO_MAJ_VER;
		cvmx_bootinfo_array[core].minor_version = CVMX_BOOTINFO_MIN_VER;
		cvmx_bootinfo_array[core].fdt_addr = virt_to_phys(gd->fdt_blob);

		boot_desc[core].dram_size = gd->ram_size / (1024 * 1024);
		boot_desc[core].cvmx_desc_vaddr =
			virt_to_phys(&cvmx_bootinfo_array[core]);

		boot_desc[core].desc_version = OCTEON_CURRENT_DESC_VERSION;
		boot_desc[core].desc_size = sizeof(boot_desc[0]);

		boot_desc[core].flags = cvmx_bootinfo_array[core].flags;
		boot_desc[core].eclock_hz = cvmx_bootinfo_array[core].eclock_hz;

		boot_desc[core].argc = argc;
		for (i = 0; i < argc; i++)
			boot_desc[core].argv[i] = (u32)virt_to_phys(argv[i]);
	}

	core = 0;
	arg0 = argc;
	arg1 = (u64)argv;
	arg2 = 0x1;	/* Core 0 sets init core for Linux */
	arg3 = XKPHYS | virt_to_phys(&boot_desc[core]);

	debug("## Transferring control to Linux (at address %p) ...\n", kernel);

	/*
	 * Flush cache before jumping to application. Let's flush the
	 * whole SDRAM area, since we don't know the size of the image
	 * that was loaded.
	 */
	flush_cache(gd->ram_base, gd->ram_top - gd->ram_base);

	/* Take all cores out of reset */
	csr_wr(CVMX_CIU_PP_RST, 0);
	sync();

	/* Wait a short while for the other cores... */
	mdelay(100);

	/* Install boot code into moveable bus for NMI (other cores) */
	nmi_code = (const u64 *)nmi_bootvector;
	num_dwords = (((u64)&nmi_handler_para[0] - (u64)nmi_code) + 7) / 8;

	ret = octeon_set_moveable_region(0x1fc00000, 0, true, nmi_code,
					 num_dwords);
	if (ret) {
		printf("Error installing NMI handler for SMP core startup\n");
		return 0;
	}

	/* Write NMI handler parameters for Linux kernel booting */
	nmi_handler_para[0] = (u64)kernel;
	nmi_handler_para[1] = arg0;
	nmi_handler_para[2] = arg1;
	nmi_handler_para[3] = 0; /* Don't set init core for secondary cores */
	nmi_handler_para[4] = arg3;
	sync();

	/* Wait a short while for the other cores... */
	mdelay(100);

	/*
	 * Cores have already been taken out of reset to conserve power.
	 * We need to send a NMI to get the cores out of their wait loop
	 */
	octeon_get_available_coremask(&avail_coremask);
	debug("Available coremask:\n");
	cvmx_coremask_dprint(&avail_coremask);
	debug("Starting coremask:\n");
	cvmx_coremask_dprint(&coremask_to_run);
	debug("Sending NMIs to other cores\n");
	if (octeon_has_feature(OCTEON_FEATURE_CIU3)) {
		u64 avail_cm;
		int node;

		cvmx_coremask_for_each_node(node, node_mask) {
			avail_cm = cvmx_coremask_get64_node(&avail_coremask,
							    node);

			if (avail_cm != 0) {
				debug("Sending NMI  to node %d, coremask=0x%llx, CIU3_NMI=0x%llx\n",
				      node, avail_cm,
				      (node > 0 ? -1ull : -2ull) & avail_cm);
				csr_wr(CVMX_CIU3_NMI,
				       (node > 0 ? -1ull : -2ull) & avail_cm);
			}
		}
	} else {
		csr_wr(CVMX_CIU_NMI,
		       -2ull & cvmx_coremask_get64(&avail_coremask));
	}
	debug("Done sending NMIs\n");

	/* Wait a short while for the other cores... */
	mdelay(100);

	/*
	 * pass address parameter as argv[0] (aka command name),
	 * and all remaining args
	 * a0 = argc
	 * a1 = argv (32 bit physical addresses, not pointers)
	 * a2 = init core
	 * a3 = boot descriptor address
	 * a4/t0 = entry point (only used by assembly stub)
	 */
	kernel(arg0, arg1, arg2, arg3);

	return 0;
}

U_BOOT_CMD(bootoctlinux, 32, 0, do_bootoctlinux,
	   "Boot from a linux ELF image in memory",
	   "elf_address [coremask=mask_to_run | numcores=core_cnt_to_run] "
	   "[forceboot] [skipcores=core_cnt_to_skip] [namedblock=name] [endbootargs] [app_args ...]\n"
	   "elf_address - address of ELF image to load. If 0, default load address\n"
	   "              is  used.\n"
	   "coremask    - mask of cores to run on.  Anded with coremask_override\n"
	   "              environment variable to ensure only working cores are used\n"
	   "numcores    - number of cores to run on.  Runs on specified number of cores,\n"
	   "              taking into account the coremask_override.\n"
	   "skipcores   - only meaningful with numcores.  Skips this many cores\n"
	   "              (starting from 0) when loading the numcores cores.\n"
	   "              For example, setting skipcores to 1 will skip core 0\n"
	   "              and load the application starting at the next available core.\n"
	   "forceboot   - if set, boots application even if core 0 is not in mask\n"
	   "namedblock	- specifies a named block to load the kernel\n"
	   "endbootargs - if set, bootloader does not process any further arguments and\n"
	   "              only passes the arguments that follow to the kernel.\n"
	   "              If not set, the kernel gets the entire commnad line as\n"
	   "              arguments.\n" "\n");
