// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#ifndef USE_HOSTCC
#include <common.h>
#include <bootstage.h>
#include <bzlib.h>
#include <errno.h>
#include <fdt_support.h>
#include <lmb.h>
#include <malloc.h>
#include <mapmem.h>
#include <asm/io.h>
#include <linux/lzo.h>
#include <lzma/LzmaTypes.h>
#include <lzma/LzmaDec.h>
#include <lzma/LzmaTools.h>
#if defined(CONFIG_CMD_USB)
#include <usb.h>
#endif
#else
#include "mkimage.h"
#endif

#include <command.h>
#include <bootm.h>
#include <image.h>

#ifndef CONFIG_SYS_BOOTM_LEN
/* use 8MByte as default max gunzip size */
#define CONFIG_SYS_BOOTM_LEN	0x800000
#endif

#define IH_INITRD_ARCH IH_ARCH_DEFAULT

#ifndef USE_HOSTCC

DECLARE_GLOBAL_DATA_PTR;

bootm_headers_t images;		/* pointers to os/initrd/fdt images */

static const void *boot_get_kernel(cmd_tbl_t *cmdtp, int flag, int argc,
				   char * const argv[], bootm_headers_t *images,
				   ulong *os_data, ulong *os_len);

__weak void board_quiesce_devices(void)
{
}

#ifdef CONFIG_LMB
static void boot_start_lmb(bootm_headers_t *images)
{
	ulong		mem_start;
	phys_size_t	mem_size;

	mem_start = env_get_bootm_low();
	mem_size = env_get_bootm_size();

	lmb_init_and_reserve_range(&images->lmb, (phys_addr_t)mem_start,
				   mem_size, NULL);
}
#else
#define lmb_reserve(lmb, base, size)
static inline void boot_start_lmb(bootm_headers_t *images) { }
#endif

static int bootm_start(cmd_tbl_t *cmdtp, int flag, int argc,
		       char * const argv[])
{
	memset((void *)&images, 0, sizeof(images));
	images.verify = env_get_yesno("verify");

	boot_start_lmb(&images);

	bootstage_mark_name(BOOTSTAGE_ID_BOOTM_START, "bootm_start");
	images.state = BOOTM_STATE_START;

	return 0;
}

static int bootm_find_os(cmd_tbl_t *cmdtp, int flag, int argc,
			 char * const argv[])
{
	const void *os_hdr;
	bool ep_found = false;
	int ret;

	/* get kernel image header, start address and length */
	os_hdr = boot_get_kernel(cmdtp, flag, argc, argv,
			&images, &images.os.image_start, &images.os.image_len);
	if (images.os.image_len == 0) {
		puts("ERROR: can't get kernel image!\n");
		return 1;
	}

	/* get image parameters */
	switch (genimg_get_format(os_hdr)) {
#if defined(CONFIG_IMAGE_FORMAT_LEGACY)
	case IMAGE_FORMAT_LEGACY:
		images.os.type = image_get_type(os_hdr);
		images.os.comp = image_get_comp(os_hdr);
		images.os.os = image_get_os(os_hdr);

		images.os.end = image_get_image_end(os_hdr);
		images.os.load = image_get_load(os_hdr);
		images.os.arch = image_get_arch(os_hdr);
		break;
#endif
#if IMAGE_ENABLE_FIT
	case IMAGE_FORMAT_FIT:
		if (fit_image_get_type(images.fit_hdr_os,
				       images.fit_noffset_os,
				       &images.os.type)) {
			puts("Can't get image type!\n");
			bootstage_error(BOOTSTAGE_ID_FIT_TYPE);
			return 1;
		}

		if (fit_image_get_comp(images.fit_hdr_os,
				       images.fit_noffset_os,
				       &images.os.comp)) {
			puts("Can't get image compression!\n");
			bootstage_error(BOOTSTAGE_ID_FIT_COMPRESSION);
			return 1;
		}

		if (fit_image_get_os(images.fit_hdr_os, images.fit_noffset_os,
				     &images.os.os)) {
			puts("Can't get image OS!\n");
			bootstage_error(BOOTSTAGE_ID_FIT_OS);
			return 1;
		}

		if (fit_image_get_arch(images.fit_hdr_os,
				       images.fit_noffset_os,
				       &images.os.arch)) {
			puts("Can't get image ARCH!\n");
			return 1;
		}

		images.os.end = fit_get_end(images.fit_hdr_os);

		if (fit_image_get_load(images.fit_hdr_os, images.fit_noffset_os,
				       &images.os.load)) {
			puts("Can't get image load address!\n");
			bootstage_error(BOOTSTAGE_ID_FIT_LOADADDR);
			return 1;
		}
		break;
#endif
#ifdef CONFIG_ANDROID_BOOT_IMAGE
	case IMAGE_FORMAT_ANDROID:
		images.os.type = IH_TYPE_KERNEL;
		images.os.comp = android_image_get_kcomp(os_hdr);
		images.os.os = IH_OS_LINUX;

		images.os.end = android_image_get_end(os_hdr);
		images.os.load = android_image_get_kload(os_hdr);
		images.ep = images.os.load;
		ep_found = true;
		break;
#endif
	default:
		puts("ERROR: unknown image format type!\n");
		return 1;
	}

	/* If we have a valid setup.bin, we will use that for entry (x86) */
	if (images.os.arch == IH_ARCH_I386 ||
	    images.os.arch == IH_ARCH_X86_64) {
		ulong len;

		ret = boot_get_setup(&images, IH_ARCH_I386, &images.ep, &len);
		if (ret < 0 && ret != -ENOENT) {
			puts("Could not find a valid setup.bin for x86\n");
			return 1;
		}
		/* Kernel entry point is the setup.bin */
	} else if (images.legacy_hdr_valid) {
		images.ep = image_get_ep(&images.legacy_hdr_os_copy);
#if IMAGE_ENABLE_FIT
	} else if (images.fit_uname_os) {
		int ret;

		ret = fit_image_get_entry(images.fit_hdr_os,
					  images.fit_noffset_os, &images.ep);
		if (ret) {
			puts("Can't get entry point property!\n");
			return 1;
		}
#endif
	} else if (!ep_found) {
		puts("Could not find kernel entry point!\n");
		return 1;
	}

	if (images.os.type == IH_TYPE_KERNEL_NOLOAD) {
		if (CONFIG_IS_ENABLED(CMD_BOOTI) &&
		    images.os.arch == IH_ARCH_ARM64) {
			ulong image_addr;
			ulong image_size;

			ret = booti_setup(images.os.image_start, &image_addr,
					  &image_size, true);
			if (ret != 0)
				return 1;

			images.os.type = IH_TYPE_KERNEL;
			images.os.load = image_addr;
			images.ep = image_addr;
		} else {
			images.os.load = images.os.image_start;
			images.ep += images.os.image_start;
		}
	}

	images.os.start = map_to_sysmem(os_hdr);

	return 0;
}

/**
 * bootm_find_images - wrapper to find and locate various images
 * @flag: Ignored Argument
 * @argc: command argument count
 * @argv: command argument list
 *
 * boot_find_images() will attempt to load an available ramdisk,
 * flattened device tree, as well as specifically marked
 * "loadable" images (loadables are FIT only)
 *
 * Note: bootm_find_images will skip an image if it is not found
 *
 * @return:
 *     0, if all existing images were loaded correctly
 *     1, if an image is found but corrupted, or invalid
 */
int bootm_find_images(int flag, int argc, char * const argv[])
{
	int ret;

	/* find ramdisk */
	ret = boot_get_ramdisk(argc, argv, &images, IH_INITRD_ARCH,
			       &images.rd_start, &images.rd_end);
	if (ret) {
		puts("Ramdisk image is corrupt or invalid\n");
		return 1;
	}

#if IMAGE_ENABLE_OF_LIBFDT
	/* find flattened device tree */
	ret = boot_get_fdt(flag, argc, argv, IH_ARCH_DEFAULT, &images,
			   &images.ft_addr, &images.ft_len);
	if (ret) {
		puts("Could not find a valid device tree\n");
		return 1;
	}
	if (CONFIG_IS_ENABLED(CMD_FDT))
		set_working_fdt_addr(map_to_sysmem(images.ft_addr));
#endif

#if IMAGE_ENABLE_FIT
#if defined(CONFIG_FPGA)
	/* find bitstreams */
	ret = boot_get_fpga(argc, argv, &images, IH_ARCH_DEFAULT,
			    NULL, NULL);
	if (ret) {
		printf("FPGA image is corrupted or invalid\n");
		return 1;
	}
#endif

	/* find all of the loadables */
	ret = boot_get_loadable(argc, argv, &images, IH_ARCH_DEFAULT,
			       NULL, NULL);
	if (ret) {
		printf("Loadable(s) is corrupt or invalid\n");
		return 1;
	}
#endif

	return 0;
}

static int bootm_find_other(cmd_tbl_t *cmdtp, int flag, int argc,
			    char * const argv[])
{
	if (((images.os.type == IH_TYPE_KERNEL) ||
	     (images.os.type == IH_TYPE_KERNEL_NOLOAD) ||
	     (images.os.type == IH_TYPE_MULTI)) &&
	    (images.os.os == IH_OS_LINUX ||
		 images.os.os == IH_OS_VXWORKS))
		return bootm_find_images(flag, argc, argv);

	return 0;
}
#endif /* USE_HOSTC */

/**
 * print_decomp_msg() - Print a suitable decompression/loading message
 *
 * @type:	OS type (IH_OS_...)
 * @comp_type:	Compression type being used (IH_COMP_...)
 * @is_xip:	true if the load address matches the image start
 */
static void print_decomp_msg(int comp_type, int type, bool is_xip)
{
	const char *name = genimg_get_type_name(type);

	if (comp_type == IH_COMP_NONE)
		printf("   %s %s ... ", is_xip ? "XIP" : "Loading", name);
	else
		printf("   Uncompressing %s ... ", name);
}

/**
 * handle_decomp_error() - display a decompression error
 *
 * This function tries to produce a useful message. In the case where the
 * uncompressed size is the same as the available space, we can assume that
 * the image is too large for the buffer.
 *
 * @comp_type:		Compression type being used (IH_COMP_...)
 * @uncomp_size:	Number of bytes uncompressed
 * @unc_len:		Amount of space available for decompression
 * @ret:		Error code to report
 * @return BOOTM_ERR_RESET, indicating that the board must be reset
 */
static int handle_decomp_error(int comp_type, size_t uncomp_size,
			       size_t unc_len, int ret)
{
	const char *name = genimg_get_comp_name(comp_type);

	if (uncomp_size >= unc_len)
		printf("Image too large: increase CONFIG_SYS_BOOTM_LEN\n");
	else
		printf("%s: uncompress error %d\n", name, ret);

	/*
	 * The decompression routines are now safe, so will not write beyond
	 * their bounds. Probably it is not necessary to reset, but maintain
	 * the current behaviour for now.
	 */
	printf("Must RESET board to recover\n");
#ifndef USE_HOSTCC
	bootstage_error(BOOTSTAGE_ID_DECOMP_IMAGE);
#endif

	return BOOTM_ERR_RESET;
}

int bootm_decomp_image(int comp, ulong load, ulong image_start, int type,
		       void *load_buf, void *image_buf, ulong image_len,
		       uint unc_len, ulong *load_end)
{
	int ret = 0;

	*load_end = load;
	print_decomp_msg(comp, type, load == image_start);

	/*
	 * Load the image to the right place, decompressing if needed. After
	 * this, image_len will be set to the number of uncompressed bytes
	 * loaded, ret will be non-zero on error.
	 */
	switch (comp) {
	case IH_COMP_NONE:
		if (load == image_start)
			break;
		if (image_len <= unc_len)
			memmove_wd(load_buf, image_buf, image_len, CHUNKSZ);
		else
			ret = 1;
		break;
#ifdef CONFIG_GZIP
	case IH_COMP_GZIP: {
		ret = gunzip(load_buf, unc_len, image_buf, &image_len);
		break;
	}
#endif /* CONFIG_GZIP */
#ifdef CONFIG_BZIP2
	case IH_COMP_BZIP2: {
		uint size = unc_len;

		/*
		 * If we've got less than 4 MB of malloc() space,
		 * use slower decompression algorithm which requires
		 * at most 2300 KB of memory.
		 */
		ret = BZ2_bzBuffToBuffDecompress(load_buf, &size,
			image_buf, image_len,
			CONFIG_SYS_MALLOC_LEN < (4096 * 1024), 0);
		image_len = size;
		break;
	}
#endif /* CONFIG_BZIP2 */
#ifdef CONFIG_LZMA
	case IH_COMP_LZMA: {
		SizeT lzma_len = unc_len;

		ret = lzmaBuffToBuffDecompress(load_buf, &lzma_len,
					       image_buf, image_len);
		image_len = lzma_len;
		break;
	}
#endif /* CONFIG_LZMA */
#ifdef CONFIG_LZO
	case IH_COMP_LZO: {
		size_t size = unc_len;

		ret = lzop_decompress(image_buf, image_len, load_buf, &size);
		image_len = size;
		break;
	}
#endif /* CONFIG_LZO */
#ifdef CONFIG_LZ4
	case IH_COMP_LZ4: {
		size_t size = unc_len;

		ret = ulz4fn(image_buf, image_len, load_buf, &size);
		image_len = size;
		break;
	}
#endif /* CONFIG_LZ4 */
	default:
		printf("Unimplemented compression type %d\n", comp);
		return BOOTM_ERR_UNIMPLEMENTED;
	}

	if (ret)
		return handle_decomp_error(comp, image_len, unc_len, ret);
	*load_end = load + image_len;

	puts("OK\n");

	return 0;
}

#ifndef USE_HOSTCC
static int bootm_load_os(bootm_headers_t *images, int boot_progress)
{
	image_info_t os = images->os;
	ulong load = os.load;
	ulong load_end;
	ulong blob_start = os.start;
	ulong blob_end = os.end;
	ulong image_start = os.image_start;
	ulong image_len = os.image_len;
	ulong flush_start = ALIGN_DOWN(load, ARCH_DMA_MINALIGN);
	bool no_overlap;
	void *load_buf, *image_buf;
	int err;

	load_buf = map_sysmem(load, 0);
	image_buf = map_sysmem(os.image_start, image_len);
	err = bootm_decomp_image(os.comp, load, os.image_start, os.type,
				 load_buf, image_buf, image_len,
				 CONFIG_SYS_BOOTM_LEN, &load_end);
	if (err) {
		bootstage_error(BOOTSTAGE_ID_DECOMP_IMAGE);
		return err;
	}

	flush_cache(flush_start, ALIGN(load_end, ARCH_DMA_MINALIGN) - flush_start);

	debug("   kernel loaded at 0x%08lx, end = 0x%08lx\n", load, load_end);
	bootstage_mark(BOOTSTAGE_ID_KERNEL_LOADED);

	no_overlap = (os.comp == IH_COMP_NONE && load == image_start);

	if (!no_overlap && load < blob_end && load_end > blob_start) {
		debug("images.os.start = 0x%lX, images.os.end = 0x%lx\n",
		      blob_start, blob_end);
		debug("images.os.load = 0x%lx, load_end = 0x%lx\n", load,
		      load_end);

		/* Check what type of image this is. */
		if (images->legacy_hdr_valid) {
			if (image_get_type(&images->legacy_hdr_os_copy)
					== IH_TYPE_MULTI)
				puts("WARNING: legacy format multi component image overwritten\n");
			return BOOTM_ERR_OVERLAP;
		} else {
			puts("ERROR: new format image overwritten - must RESET the board to recover\n");
			bootstage_error(BOOTSTAGE_ID_OVERWRITTEN);
			return BOOTM_ERR_RESET;
		}
	}

	lmb_reserve(&images->lmb, images->os.load, (load_end -
						    images->os.load));
	return 0;
}

/**
 * bootm_disable_interrupts() - Disable interrupts in preparation for load/boot
 *
 * @return interrupt flag (0 if interrupts were disabled, non-zero if they were
 *	enabled)
 */
ulong bootm_disable_interrupts(void)
{
	ulong iflag;

	/*
	 * We have reached the point of no return: we are going to
	 * overwrite all exception vector code, so we cannot easily
	 * recover from any failures any more...
	 */
	iflag = disable_interrupts();
#ifdef CONFIG_NETCONSOLE
	/* Stop the ethernet stack if NetConsole could have left it up */
	eth_halt();
# ifndef CONFIG_DM_ETH
	eth_unregister(eth_get_dev());
# endif
#endif

#if defined(CONFIG_CMD_USB)
	/*
	 * turn off USB to prevent the host controller from writing to the
	 * SDRAM while Linux is booting. This could happen (at least for OHCI
	 * controller), because the HCCA (Host Controller Communication Area)
	 * lies within the SDRAM and the host controller writes continously to
	 * this area (as busmaster!). The HccaFrameNumber is for example
	 * updated every 1 ms within the HCCA structure in SDRAM! For more
	 * details see the OpenHCI specification.
	 */
	usb_stop();
#endif
	return iflag;
}

#if defined(CONFIG_SILENT_CONSOLE) && !defined(CONFIG_SILENT_U_BOOT_ONLY)

#define CONSOLE_ARG     "console="
#define CONSOLE_ARG_LEN (sizeof(CONSOLE_ARG) - 1)

static void fixup_silent_linux(void)
{
	char *buf;
	const char *env_val;
	char *cmdline = env_get("bootargs");
	int want_silent;

	/*
	 * Only fix cmdline when requested. The environment variable can be:
	 *
	 *	no - we never fixup
	 *	yes - we always fixup
	 *	unset - we rely on the console silent flag
	 */
	want_silent = env_get_yesno("silent_linux");
	if (want_silent == 0)
		return;
	else if (want_silent == -1 && !(gd->flags & GD_FLG_SILENT))
		return;

	debug("before silent fix-up: %s\n", cmdline);
	if (cmdline && (cmdline[0] != '\0')) {
		char *start = strstr(cmdline, CONSOLE_ARG);

		/* Allocate space for maximum possible new command line */
		buf = malloc(strlen(cmdline) + 1 + CONSOLE_ARG_LEN + 1);
		if (!buf) {
			debug("%s: out of memory\n", __func__);
			return;
		}

		if (start) {
			char *end = strchr(start, ' ');
			int num_start_bytes = start - cmdline + CONSOLE_ARG_LEN;

			strncpy(buf, cmdline, num_start_bytes);
			if (end)
				strcpy(buf + num_start_bytes, end);
			else
				buf[num_start_bytes] = '\0';
		} else {
			sprintf(buf, "%s %s", cmdline, CONSOLE_ARG);
		}
		env_val = buf;
	} else {
		buf = NULL;
		env_val = CONSOLE_ARG;
	}

	env_set("bootargs", env_val);
	debug("after silent fix-up: %s\n", env_val);
	free(buf);
}
#endif /* CONFIG_SILENT_CONSOLE */

/**
 * Execute selected states of the bootm command.
 *
 * Note the arguments to this state must be the first argument, Any 'bootm'
 * or sub-command arguments must have already been taken.
 *
 * Note that if states contains more than one flag it MUST contain
 * BOOTM_STATE_START, since this handles and consumes the command line args.
 *
 * Also note that aside from boot_os_fn functions and bootm_load_os no other
 * functions we store the return value of in 'ret' may use a negative return
 * value, without special handling.
 *
 * @param cmdtp		Pointer to bootm command table entry
 * @param flag		Command flags (CMD_FLAG_...)
 * @param argc		Number of subcommand arguments (0 = no arguments)
 * @param argv		Arguments
 * @param states	Mask containing states to run (BOOTM_STATE_...)
 * @param images	Image header information
 * @param boot_progress 1 to show boot progress, 0 to not do this
 * @return 0 if ok, something else on error. Some errors will cause this
 *	function to perform a reboot! If states contains BOOTM_STATE_OS_GO
 *	then the intent is to boot an OS, so this function will not return
 *	unless the image type is standalone.
 */
int do_bootm_states(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[],
		    int states, bootm_headers_t *images, int boot_progress)
{
	boot_os_fn *boot_fn;
	ulong iflag = 0;
	int ret = 0, need_boot_fn;

	images->state |= states;

	/*
	 * Work through the states and see how far we get. We stop on
	 * any error.
	 */
	if (states & BOOTM_STATE_START)
		ret = bootm_start(cmdtp, flag, argc, argv);

	if (!ret && (states & BOOTM_STATE_FINDOS))
		ret = bootm_find_os(cmdtp, flag, argc, argv);

	if (!ret && (states & BOOTM_STATE_FINDOTHER))
		ret = bootm_find_other(cmdtp, flag, argc, argv);

	/* Load the OS */
	if (!ret && (states & BOOTM_STATE_LOADOS)) {
		iflag = bootm_disable_interrupts();
		ret = bootm_load_os(images, 0);
		if (ret && ret != BOOTM_ERR_OVERLAP)
			goto err;
		else if (ret == BOOTM_ERR_OVERLAP)
			ret = 0;
	}

	/* Relocate the ramdisk */
#ifdef CONFIG_SYS_BOOT_RAMDISK_HIGH
	if (!ret && (states & BOOTM_STATE_RAMDISK)) {
		ulong rd_len = images->rd_end - images->rd_start;

		ret = boot_ramdisk_high(&images->lmb, images->rd_start,
			rd_len, &images->initrd_start, &images->initrd_end);
		if (!ret) {
			env_set_hex("initrd_start", images->initrd_start);
			env_set_hex("initrd_end", images->initrd_end);
		}
	}
#endif
#if IMAGE_ENABLE_OF_LIBFDT && defined(CONFIG_LMB)
	if (!ret && (states & BOOTM_STATE_FDT)) {
		boot_fdt_add_mem_rsv_regions(&images->lmb, images->ft_addr);
		ret = boot_relocate_fdt(&images->lmb, &images->ft_addr,
					&images->ft_len);
	}
#endif

	/* From now on, we need the OS boot function */
	if (ret)
		return ret;
	boot_fn = bootm_os_get_boot_func(images->os.os);
	need_boot_fn = states & (BOOTM_STATE_OS_CMDLINE |
			BOOTM_STATE_OS_BD_T | BOOTM_STATE_OS_PREP |
			BOOTM_STATE_OS_FAKE_GO | BOOTM_STATE_OS_GO);
	if (boot_fn == NULL && need_boot_fn) {
		if (iflag)
			enable_interrupts();
		printf("ERROR: booting os '%s' (%d) is not supported\n",
		       genimg_get_os_name(images->os.os), images->os.os);
		bootstage_error(BOOTSTAGE_ID_CHECK_BOOT_OS);
		return 1;
	}


	/* Call various other states that are not generally used */
	if (!ret && (states & BOOTM_STATE_OS_CMDLINE))
		ret = boot_fn(BOOTM_STATE_OS_CMDLINE, argc, argv, images);
	if (!ret && (states & BOOTM_STATE_OS_BD_T))
		ret = boot_fn(BOOTM_STATE_OS_BD_T, argc, argv, images);
	if (!ret && (states & BOOTM_STATE_OS_PREP)) {
#if defined(CONFIG_SILENT_CONSOLE) && !defined(CONFIG_SILENT_U_BOOT_ONLY)
		if (images->os.os == IH_OS_LINUX)
			fixup_silent_linux();
#endif
		ret = boot_fn(BOOTM_STATE_OS_PREP, argc, argv, images);
	}

#ifdef CONFIG_TRACE
	/* Pretend to run the OS, then run a user command */
	if (!ret && (states & BOOTM_STATE_OS_FAKE_GO)) {
		char *cmd_list = env_get("fakegocmd");

		ret = boot_selected_os(argc, argv, BOOTM_STATE_OS_FAKE_GO,
				images, boot_fn);
		if (!ret && cmd_list)
			ret = run_command_list(cmd_list, -1, flag);
	}
#endif

	/* Check for unsupported subcommand. */
	if (ret) {
		puts("subcommand not supported\n");
		return ret;
	}

	/* Now run the OS! We hope this doesn't return */
	if (!ret && (states & BOOTM_STATE_OS_GO))
		ret = boot_selected_os(argc, argv, BOOTM_STATE_OS_GO,
				images, boot_fn);

	/* Deal with any fallout */
err:
	if (iflag)
		enable_interrupts();

	if (ret == BOOTM_ERR_UNIMPLEMENTED)
		bootstage_error(BOOTSTAGE_ID_DECOMP_UNIMPL);
	else if (ret == BOOTM_ERR_RESET)
		do_reset(cmdtp, flag, argc, argv);

	return ret;
}

#if defined(CONFIG_IMAGE_FORMAT_LEGACY)
/**
 * image_get_kernel - verify legacy format kernel image
 * @img_addr: in RAM address of the legacy format image to be verified
 * @verify: data CRC verification flag
 *
 * image_get_kernel() verifies legacy image integrity and returns pointer to
 * legacy image header if image verification was completed successfully.
 *
 * returns:
 *     pointer to a legacy image header if valid image was found
 *     otherwise return NULL
 */
static image_header_t *image_get_kernel(ulong img_addr, int verify)
{
	image_header_t *hdr = (image_header_t *)img_addr;

	if (!image_check_magic(hdr)) {
		puts("Bad Magic Number\n");
		bootstage_error(BOOTSTAGE_ID_CHECK_MAGIC);
		return NULL;
	}
	bootstage_mark(BOOTSTAGE_ID_CHECK_HEADER);

	if (!image_check_hcrc(hdr)) {
		puts("Bad Header Checksum\n");
		bootstage_error(BOOTSTAGE_ID_CHECK_HEADER);
		return NULL;
	}

	bootstage_mark(BOOTSTAGE_ID_CHECK_CHECKSUM);
	image_print_contents(hdr);

	if (verify) {
		puts("   Verifying Checksum ... ");
		if (!image_check_dcrc(hdr)) {
			printf("Bad Data CRC\n");
			bootstage_error(BOOTSTAGE_ID_CHECK_CHECKSUM);
			return NULL;
		}
		puts("OK\n");
	}
	bootstage_mark(BOOTSTAGE_ID_CHECK_ARCH);

	if (!image_check_target_arch(hdr)) {
		printf("Unsupported Architecture 0x%x\n", image_get_arch(hdr));
		bootstage_error(BOOTSTAGE_ID_CHECK_ARCH);
		return NULL;
	}
	return hdr;
}
#endif

/**
 * boot_get_kernel - find kernel image
 * @os_data: pointer to a ulong variable, will hold os data start address
 * @os_len: pointer to a ulong variable, will hold os data length
 *
 * boot_get_kernel() tries to find a kernel image, verifies its integrity
 * and locates kernel data.
 *
 * returns:
 *     pointer to image header if valid image was found, plus kernel start
 *     address and length, otherwise NULL
 */
static const void *boot_get_kernel(cmd_tbl_t *cmdtp, int flag, int argc,
				   char * const argv[], bootm_headers_t *images,
				   ulong *os_data, ulong *os_len)
{
#if defined(CONFIG_IMAGE_FORMAT_LEGACY)
	image_header_t	*hdr;
#endif
	ulong		img_addr;
	const void *buf;
	const char	*fit_uname_config = NULL;
	const char	*fit_uname_kernel = NULL;
#if IMAGE_ENABLE_FIT
	int		os_noffset;
#endif

	img_addr = genimg_get_kernel_addr_fit(argc < 1 ? NULL : argv[0],
					      &fit_uname_config,
					      &fit_uname_kernel);

	bootstage_mark(BOOTSTAGE_ID_CHECK_MAGIC);

	/* check image type, for FIT images get FIT kernel node */
	*os_data = *os_len = 0;
	buf = map_sysmem(img_addr, 0);
	switch (genimg_get_format(buf)) {
#if defined(CONFIG_IMAGE_FORMAT_LEGACY)
	case IMAGE_FORMAT_LEGACY:
		printf("## Booting kernel from Legacy Image at %08lx ...\n",
		       img_addr);
		hdr = image_get_kernel(img_addr, images->verify);
		if (!hdr)
			return NULL;
		bootstage_mark(BOOTSTAGE_ID_CHECK_IMAGETYPE);

		/* get os_data and os_len */
		switch (image_get_type(hdr)) {
		case IH_TYPE_KERNEL:
		case IH_TYPE_KERNEL_NOLOAD:
			*os_data = image_get_data(hdr);
			*os_len = image_get_data_size(hdr);
			break;
		case IH_TYPE_MULTI:
			image_multi_getimg(hdr, 0, os_data, os_len);
			break;
		case IH_TYPE_STANDALONE:
			*os_data = image_get_data(hdr);
			*os_len = image_get_data_size(hdr);
			break;
		default:
			printf("Wrong Image Type for %s command\n",
			       cmdtp->name);
			bootstage_error(BOOTSTAGE_ID_CHECK_IMAGETYPE);
			return NULL;
		}

		/*
		 * copy image header to allow for image overwrites during
		 * kernel decompression.
		 */
		memmove(&images->legacy_hdr_os_copy, hdr,
			sizeof(image_header_t));

		/* save pointer to image header */
		images->legacy_hdr_os = hdr;

		images->legacy_hdr_valid = 1;
		bootstage_mark(BOOTSTAGE_ID_DECOMP_IMAGE);
		break;
#endif
#if IMAGE_ENABLE_FIT
	case IMAGE_FORMAT_FIT:
		os_noffset = fit_image_load(images, img_addr,
				&fit_uname_kernel, &fit_uname_config,
				IH_ARCH_DEFAULT, IH_TYPE_KERNEL,
				BOOTSTAGE_ID_FIT_KERNEL_START,
				FIT_LOAD_IGNORED, os_data, os_len);
		if (os_noffset < 0)
			return NULL;

		images->fit_hdr_os = map_sysmem(img_addr, 0);
		images->fit_uname_os = fit_uname_kernel;
		images->fit_uname_cfg = fit_uname_config;
		images->fit_noffset_os = os_noffset;
		break;
#endif
#ifdef CONFIG_ANDROID_BOOT_IMAGE
	case IMAGE_FORMAT_ANDROID:
		printf("## Booting Android Image at 0x%08lx ...\n", img_addr);
		if (android_image_get_kernel(buf, images->verify,
					     os_data, os_len))
			return NULL;
		break;
#endif
	default:
		printf("Wrong Image Format for %s command\n", cmdtp->name);
		bootstage_error(BOOTSTAGE_ID_FIT_KERNEL_INFO);
		return NULL;
	}

	debug("   kernel data at 0x%08lx, len = 0x%08lx (%ld)\n",
	      *os_data, *os_len, *os_len);

	return buf;
}

/**
 * switch_to_non_secure_mode() - switch to non-secure mode
 *
 * This routine is overridden by architectures requiring this feature.
 */
void __weak switch_to_non_secure_mode(void)
{
}

#else /* USE_HOSTCC */

void memmove_wd(void *to, void *from, size_t len, ulong chunksz)
{
	memmove(to, from, len);
}

#if defined(CONFIG_FIT_SIGNATURE)
static int bootm_host_load_image(const void *fit, int req_image_type)
{
	const char *fit_uname_config = NULL;
	ulong data, len;
	bootm_headers_t images;
	int noffset;
	ulong load_end;
	uint8_t image_type;
	uint8_t imape_comp;
	void *load_buf;
	int ret;

	memset(&images, '\0', sizeof(images));
	images.verify = 1;
	noffset = fit_image_load(&images, (ulong)fit,
		NULL, &fit_uname_config,
		IH_ARCH_DEFAULT, req_image_type, -1,
		FIT_LOAD_IGNORED, &data, &len);
	if (noffset < 0)
		return noffset;
	if (fit_image_get_type(fit, noffset, &image_type)) {
		puts("Can't get image type!\n");
		return -EINVAL;
	}

	if (fit_image_get_comp(fit, noffset, &imape_comp)) {
		puts("Can't get image compression!\n");
		return -EINVAL;
	}

	/* Allow the image to expand by a factor of 4, should be safe */
	load_buf = malloc((1 << 20) + len * 4);
	ret = bootm_decomp_image(imape_comp, 0, data, image_type, load_buf,
				 (void *)data, len, CONFIG_SYS_BOOTM_LEN,
				 &load_end);
	free(load_buf);

	if (ret && ret != BOOTM_ERR_UNIMPLEMENTED)
		return ret;

	return 0;
}

int bootm_host_load_images(const void *fit, int cfg_noffset)
{
	static uint8_t image_types[] = {
		IH_TYPE_KERNEL,
		IH_TYPE_FLATDT,
		IH_TYPE_RAMDISK,
	};
	int err = 0;
	int i;

	for (i = 0; i < ARRAY_SIZE(image_types); i++) {
		int ret;

		ret = bootm_host_load_image(fit, image_types[i]);
		if (!err && ret && ret != -ENOENT)
			err = ret;
	}

	/* Return the first error we found */
	return err;
}
#endif

#endif /* ndef USE_HOSTCC */
