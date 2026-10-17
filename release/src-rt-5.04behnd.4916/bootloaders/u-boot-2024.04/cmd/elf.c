// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2001 William L. Pitts
 * All rights reserved.
 */

#include <common.h>
#include <command.h>
#include <cpu_func.h>
#include <elf.h>
#include <env.h>
#include <image.h>
#include <log.h>
#include <net.h>
#include <vxworks.h>
#ifdef CONFIG_X86
#include <vesa.h>
#include <asm/cache.h>
#include <asm/e820.h>
#include <linux/linkage.h>
#endif

/* Allow ports to override the default behavior */
static unsigned long do_bootelf_exec(ulong (*entry)(int, char * const[]),
				     int argc, char *const argv[])
{
	unsigned long ret;

	/*
	 * pass address parameter as argv[0] (aka command name),
	 * and all remaining args
	 */
	ret = entry(argc, argv);

	return ret;
}

/* Interpreter command to boot an arbitrary ELF image from memory */
int do_bootelf(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	unsigned long addr; /* Address of the ELF image */
	unsigned long rc; /* Return value from user code */
	char *sload = NULL;
	int rcode = 0;

	/* Consume 'bootelf' */
	argc--; argv++;

	/* Check for flag. */
	if (argc >= 1 && (argv[0][0] == '-' && \
				(argv[0][1] == 'p' || argv[0][1] == 's'))) {
		sload = argv[0];
		/* Consume flag. */
		argc--; argv++;
	}
	/* Check for address. */
	if (argc >= 1 && strict_strtoul(argv[0], 16, &addr) != -EINVAL) {
		/* Consume address */
		argc--; argv++;
	} else
		addr = image_load_addr;

	if (!valid_elf_image(addr))
		return 1;

	if (sload && sload[1] == 'p')
		addr = load_elf_image_phdr(addr);
	else
		addr = load_elf_image_shdr(addr);

	if (!env_get_autostart())
		return rcode;

	printf("## Starting application at 0x%08lx ...\n", addr);
	flush();

	/*
	 * pass address parameter as argv[0] (aka command name),
	 * and all remaining args
	 */
	rc = do_bootelf_exec((void *)addr, argc, argv);
	if (rc != 0)
		rcode = 1;

	printf("## Application terminated, rc = 0x%lx\n", rc);

	return rcode;
}

/*
 * Interpreter command to boot VxWorks from a memory image.  The image can
 * be either an ELF image or a raw binary.  Will attempt to setup the
 * bootline and other parameters correctly.
 */
int do_bootvx(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	unsigned long addr; /* Address of image */
	unsigned long bootaddr = 0; /* Address to put the bootline */
	char *bootline; /* Text of the bootline */
	char *tmp; /* Temporary char pointer */
	char build_buf[128]; /* Buffer for building the bootline */
	int ptr = 0;
#ifdef CONFIG_X86
	ulong base;
	struct e820_info *info;
	struct e820_entry *data;
	struct efi_gop_info *gop;
	struct vesa_mode_info *vesa = &mode_info.vesa;
#endif

	/*
	 * Check the loadaddr variable.
	 * If we don't know where the image is then we're done.
	 */
	if (argc < 2)
		addr = image_load_addr;
	else
		addr = hextoul(argv[1], NULL);

#if defined(CONFIG_CMD_NET)
	/*
	 * Check to see if we need to tftp the image ourselves
	 * before starting
	 */
	if ((argc == 2) && (strcmp(argv[1], "tftp") == 0)) {
		if (net_loop(TFTPGET) <= 0)
			return 1;
		printf("Automatic boot of VxWorks image at address 0x%08lx ...\n",
			addr);
	}
#endif

	/*
	 * This should equate to
	 * NV_RAM_ADRS + NV_BOOT_OFFSET + NV_ENET_OFFSET
	 * from the VxWorks BSP header files.
	 * This will vary from board to board
	 */
#if defined(CONFIG_SYS_VXWORKS_MAC_PTR)
	tmp = (char *)CONFIG_SYS_VXWORKS_MAC_PTR;
	eth_env_get_enetaddr("ethaddr", (uchar *)build_buf);
	memcpy(tmp, build_buf, 6);
#else
	puts("## Ethernet MAC address not copied to NV RAM\n");
#endif

#ifdef CONFIG_X86
	/*
	 * Get VxWorks's physical memory base address from environment,
	 * if we don't specify it in the environment, use a default one.
	 */
	base = env_get_hex("vx_phys_mem_base", VXWORKS_PHYS_MEM_BASE);
	data = (struct e820_entry *)(base + E820_DATA_OFFSET);
	info = (struct e820_info *)(base + E820_INFO_OFFSET);

	memset(info, 0, sizeof(struct e820_info));
	info->sign = E820_SIGNATURE;
	info->entries = install_e820_map(E820MAX, data);
	info->addr = (info->entries - 1) * sizeof(struct e820_entry) +
		     E820_DATA_OFFSET;

	/*
	 * Explicitly clear the bootloader image size otherwise if memory
	 * at this offset happens to contain some garbage data, the final
	 * available memory size for the kernel is insane.
	 */
	*(u32 *)(base + BOOT_IMAGE_SIZE_OFFSET) = 0;

	/*
	 * Prepare compatible framebuffer information block.
	 * The VESA mode has to be 32-bit RGBA.
	 */
	if (vesa->x_resolution && vesa->y_resolution) {
		gop = (struct efi_gop_info *)(base + EFI_GOP_INFO_OFFSET);
		gop->magic = EFI_GOP_INFO_MAGIC;
		gop->info.version = 0;
		gop->info.width = vesa->x_resolution;
		gop->info.height = vesa->y_resolution;
		gop->info.pixel_format = EFI_GOT_RGBA8;
		gop->info.pixels_per_scanline = vesa->bytes_per_scanline / 4;
		gop->fb_base = vesa->phys_base_ptr;
		gop->fb_size = vesa->bytes_per_scanline * vesa->y_resolution;
	}
#endif

	/*
	 * Use bootaddr to find the location in memory that VxWorks
	 * will look for the bootline string. The default value is
	 * (LOCAL_MEM_LOCAL_ADRS + BOOT_LINE_OFFSET) as defined by
	 * VxWorks BSP. For example, on PowerPC it defaults to 0x4200.
	 */
	tmp = env_get("bootaddr");
	if (!tmp) {
#ifdef CONFIG_X86
		bootaddr = base + X86_BOOT_LINE_OFFSET;
#else
		printf("## VxWorks bootline address not specified\n");
		return 1;
#endif
	}

	if (!bootaddr)
		bootaddr = hextoul(tmp, NULL);

	/*
	 * Check to see if the bootline is defined in the 'bootargs' parameter.
	 * If it is not defined, we may be able to construct the info.
	 */
	bootline = env_get("bootargs");
	if (!bootline) {
		tmp = env_get("bootdev");
		if (tmp) {
			strcpy(build_buf, tmp);
			ptr = strlen(tmp);
		} else {
			printf("## VxWorks boot device not specified\n");
		}

		tmp = env_get("bootfile");
		if (tmp)
			ptr += sprintf(build_buf + ptr, "host:%s ", tmp);
		else
			ptr += sprintf(build_buf + ptr, "host:vxWorks ");

		/*
		 * The following parameters are only needed if 'bootdev'
		 * is an ethernet device, otherwise they are optional.
		 */
		tmp = env_get("ipaddr");
		if (tmp) {
			ptr += sprintf(build_buf + ptr, "e=%s", tmp);
			tmp = env_get("netmask");
			if (tmp) {
				u32 mask = env_get_ip("netmask").s_addr;
				ptr += sprintf(build_buf + ptr,
					       ":%08x ", ntohl(mask));
			} else {
				ptr += sprintf(build_buf + ptr, " ");
			}
		}

		tmp = env_get("serverip");
		if (tmp)
			ptr += sprintf(build_buf + ptr, "h=%s ", tmp);

		tmp = env_get("gatewayip");
		if (tmp)
			ptr += sprintf(build_buf + ptr, "g=%s ", tmp);

		tmp = env_get("hostname");
		if (tmp)
			ptr += sprintf(build_buf + ptr, "tn=%s ", tmp);

		tmp = env_get("othbootargs");
		if (tmp) {
			strcpy(build_buf + ptr, tmp);
			ptr += strlen(tmp);
		}

		bootline = build_buf;
	}

	memcpy((void *)bootaddr, bootline, max(strlen(bootline), (size_t)255));
	flush_cache(bootaddr, max(strlen(bootline), (size_t)255));
	printf("## Using bootline (@ 0x%lx): %s\n", bootaddr, (char *)bootaddr);

	/*
	 * If the data at the load address is an elf image, then
	 * treat it like an elf image. Otherwise, assume that it is a
	 * binary image.
	 */
	if (valid_elf_image(addr))
		addr = load_elf_image_phdr(addr);
	else
		puts("## Not an ELF image, assuming binary\n");

	printf("## Starting vxWorks at 0x%08lx ...\n", addr);
	flush();

	dcache_disable();
#if defined(CONFIG_ARM64) && defined(CONFIG_ARMV8_PSCI)
	armv8_setup_psci();
	smp_kick_all_cpus();
#endif

#ifdef CONFIG_X86
	/* VxWorks on x86 uses stack to pass parameters */
	((asmlinkage void (*)(int))addr)(0);
#else
	((void (*)(int))addr)(0);
#endif

	puts("## vxWorks terminated\n");

	return 1;
}

U_BOOT_CMD(
	bootelf, CONFIG_SYS_MAXARGS, 0, do_bootelf,
	"Boot from an ELF image in memory",
	"[-p|-s] [address]\n"
	"\t- load ELF image at [address] via program headers (-p)\n"
	"\t  or via section headers (-s)"
);

U_BOOT_CMD(
	bootvx, 2, 0, do_bootvx,
	"Boot vxWorks from an ELF image",
	" [address] - load address of vxWorks ELF image."
);
