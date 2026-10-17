// SPDX-License-Identifier: GPL-2.0+
/*
 * Image code used by boards (and not host tools)
 *
 * (C) Copyright 2008 Semihalf
 *
 * (C) Copyright 2000-2006
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <common.h>
#include <bootstage.h>
#include <cpu_func.h>
#include <display_options.h>
#include <env.h>
#include <fpga.h>
#include <image.h>
#include <init.h>
#include <log.h>
#include <mapmem.h>
#include <rtc.h>
#include <watchdog.h>
#include <asm/cache.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

/**
 * image_get_ramdisk - get and verify ramdisk image
 * @rd_addr: ramdisk image start address
 * @arch: expected ramdisk architecture
 * @verify: checksum verification flag
 *
 * image_get_ramdisk() returns a pointer to the verified ramdisk image
 * header. Routine receives image start address and expected architecture
 * flag. Verification done covers data and header integrity and os/type/arch
 * fields checking.
 *
 * returns:
 *     pointer to a ramdisk image header, if image was found and valid
 *     otherwise, return NULL
 */
static const struct legacy_img_hdr *image_get_ramdisk(ulong rd_addr, u8 arch,
						      int verify)
{
	const struct legacy_img_hdr *rd_hdr = (const struct legacy_img_hdr *)rd_addr;

	if (!image_check_magic(rd_hdr)) {
		puts("Bad Magic Number\n");
		bootstage_error(BOOTSTAGE_ID_RD_MAGIC);
		return NULL;
	}

	if (!image_check_hcrc(rd_hdr)) {
		puts("Bad Header Checksum\n");
		bootstage_error(BOOTSTAGE_ID_RD_HDR_CHECKSUM);
		return NULL;
	}

	bootstage_mark(BOOTSTAGE_ID_RD_MAGIC);
	image_print_contents(rd_hdr);

	if (verify) {
		puts("   Verifying Checksum ... ");
		if (!image_check_dcrc(rd_hdr)) {
			puts("Bad Data CRC\n");
			bootstage_error(BOOTSTAGE_ID_RD_CHECKSUM);
			return NULL;
		}
		puts("OK\n");
	}

	bootstage_mark(BOOTSTAGE_ID_RD_HDR_CHECKSUM);

	if (!image_check_os(rd_hdr, IH_OS_LINUX) ||
	    !image_check_arch(rd_hdr, arch) ||
	    !image_check_type(rd_hdr, IH_TYPE_RAMDISK)) {
		printf("No Linux %s Ramdisk Image\n",
		       genimg_get_arch_name(arch));
		bootstage_error(BOOTSTAGE_ID_RAMDISK);
		return NULL;
	}

	return rd_hdr;
}

/*****************************************************************************/
/* Shared dual-format routines */
/*****************************************************************************/
ulong image_load_addr = CONFIG_SYS_LOAD_ADDR;	/* Default Load Address */
ulong image_save_addr;			/* Default Save Address */
ulong image_save_size;			/* Default Save Size (in bytes) */

static int on_loadaddr(const char *name, const char *value, enum env_op op,
		       int flags)
{
	switch (op) {
	case env_op_create:
	case env_op_overwrite:
		image_load_addr = hextoul(value, NULL);
		break;
	default:
		break;
	}

	return 0;
}
U_BOOT_ENV_CALLBACK(loadaddr, on_loadaddr);

ulong env_get_bootm_low(void)
{
	char *s = env_get("bootm_low");

	if (s) {
		ulong tmp = hextoul(s, NULL);
		return tmp;
	}

#if defined(CFG_SYS_SDRAM_BASE)
	return CFG_SYS_SDRAM_BASE;
#elif defined(CONFIG_ARM) || defined(CONFIG_MICROBLAZE) || defined(CONFIG_RISCV)
	return gd->bd->bi_dram[0].start;
#else
	return 0;
#endif
}

phys_size_t env_get_bootm_size(void)
{
	phys_size_t tmp, size;
	phys_addr_t start;
	char *s = env_get("bootm_size");

	if (s) {
		tmp = (phys_size_t)simple_strtoull(s, NULL, 16);
		return tmp;
	}

	start = gd->ram_base;
	size = gd->ram_size;

	if (start + size > gd->ram_top)
		size = gd->ram_top - start;

	s = env_get("bootm_low");
	if (s)
		tmp = (phys_size_t)simple_strtoull(s, NULL, 16);
	else
		tmp = start;

	return size - (tmp - start);
}

phys_size_t env_get_bootm_mapsize(void)
{
	phys_size_t tmp;
	char *s = env_get("bootm_mapsize");

	if (s) {
		tmp = (phys_size_t)simple_strtoull(s, NULL, 16);
		return tmp;
	}

#if defined(CFG_SYS_BOOTMAPSZ)
	return CFG_SYS_BOOTMAPSZ;
#else
	return env_get_bootm_size();
#endif
}

void memmove_wd(void *to, void *from, size_t len, ulong chunksz)
{
	if (to == from)
		return;

	if (IS_ENABLED(CONFIG_HW_WATCHDOG) || IS_ENABLED(CONFIG_WATCHDOG)) {
		if (to > from) {
			from += len;
			to += len;
		}
		while (len > 0) {
			size_t tail = (len > chunksz) ? chunksz : len;

			schedule();
			if (to > from) {
				to -= tail;
				from -= tail;
			}
			memmove(to, from, tail);
			if (to < from) {
				to += tail;
				from += tail;
			}
			len -= tail;
		}
	} else {
		memmove(to, from, len);
	}
}

ulong genimg_get_kernel_addr_fit(const char *const img_addr,
				 const char **fit_uname_config,
				 const char **fit_uname_kernel)
{
	ulong kernel_addr;

	/* find out kernel image address */
	if (!img_addr) {
		kernel_addr = image_load_addr;
		debug("*  kernel: default image load address = 0x%08lx\n",
		      image_load_addr);
	} else if (CONFIG_IS_ENABLED(FIT) &&
		   fit_parse_conf(img_addr, image_load_addr, &kernel_addr,
				  fit_uname_config)) {
		debug("*  kernel: config '%s' from image at 0x%08lx\n",
		      *fit_uname_config, kernel_addr);
	} else if (CONFIG_IS_ENABLED(FIT) &&
		   fit_parse_subimage(img_addr, image_load_addr, &kernel_addr,
				      fit_uname_kernel)) {
		debug("*  kernel: subimage '%s' from image at 0x%08lx\n",
		      *fit_uname_kernel, kernel_addr);
	} else {
		kernel_addr = hextoul(img_addr, NULL);
		debug("*  kernel: cmdline image address = 0x%08lx\n",
		      kernel_addr);
	}

	return kernel_addr;
}

/**
 * genimg_get_kernel_addr() is the simple version of
 * genimg_get_kernel_addr_fit(). It ignores those return FIT strings
 */
ulong genimg_get_kernel_addr(char * const img_addr)
{
	const char *fit_uname_config = NULL;
	const char *fit_uname_kernel = NULL;

	return genimg_get_kernel_addr_fit(img_addr, &fit_uname_config,
					  &fit_uname_kernel);
}

/**
 * genimg_get_format - get image format type
 * @img_addr: image start address
 *
 * genimg_get_format() checks whether provided address points to a valid
 * legacy or FIT image.
 *
 * New uImage format and FDT blob are based on a libfdt. FDT blob
 * may be passed directly or embedded in a FIT image. In both situations
 * genimg_get_format() must be able to dectect libfdt header.
 *
 * returns:
 *     image format type or IMAGE_FORMAT_INVALID if no image is present
 */
int genimg_get_format(const void *img_addr)
{
	if (CONFIG_IS_ENABLED(LEGACY_IMAGE_FORMAT)) {
		const struct legacy_img_hdr *hdr;

		hdr = (const struct legacy_img_hdr *)img_addr;
		if (image_check_magic(hdr))
			return IMAGE_FORMAT_LEGACY;
	}
	if (CONFIG_IS_ENABLED(FIT) || CONFIG_IS_ENABLED(OF_LIBFDT)) {
		if (!fdt_check_header(img_addr))
			return IMAGE_FORMAT_FIT;
	}
	if (IS_ENABLED(CONFIG_ANDROID_BOOT_IMAGE) &&
	    is_android_boot_image_header(img_addr))
		return IMAGE_FORMAT_ANDROID;

	return IMAGE_FORMAT_INVALID;
}

/**
 * fit_has_config - check if there is a valid FIT configuration
 * @images: pointer to the bootm command headers structure
 *
 * fit_has_config() checks if there is a FIT configuration in use
 * (if FTI support is present).
 *
 * returns:
 *     0, no FIT support or no configuration found
 *     1, configuration found
 */
int genimg_has_config(struct bootm_headers *images)
{
	if (CONFIG_IS_ENABLED(FIT) && images->fit_uname_cfg)
		return 1;

	return 0;
}

/**
 * select_ramdisk() - Select and locate the ramdisk to use
 *
 * @images: pointer to the bootm images structure
 * @select: name of ramdisk to select, or hex address, NULL for any
 * @arch: expected ramdisk architecture
 * @rd_datap: pointer to a ulong variable, will hold ramdisk pointer
 * @rd_lenp: pointer to a ulong variable, will hold ramdisk length
 * Return: 0 if OK, -ENOPKG if no ramdisk (but an error should not be reported),
 *	other -ve value on other error
 */
static int select_ramdisk(struct bootm_headers *images, const char *select, u8 arch,
			  ulong *rd_datap, ulong *rd_lenp)
{
	const char *fit_uname_config;
	const char *fit_uname_ramdisk;
	bool done_select = !select;
	bool done = false;
	int rd_noffset;
	ulong rd_addr = 0;
	char *buf;

	if (CONFIG_IS_ENABLED(FIT)) {
		fit_uname_config = images->fit_uname_cfg;
		fit_uname_ramdisk = NULL;

		if (select) {
			ulong default_addr;
			/*
			 * If the init ramdisk comes from the FIT image and
			 * the FIT image address is omitted in the command
			 * line argument, try to use os FIT image address or
			 * default load address.
			 */
			if (images->fit_uname_os)
				default_addr = (ulong)images->fit_hdr_os;
			else
				default_addr = image_load_addr;

			if (fit_parse_conf(select, default_addr, &rd_addr,
					   &fit_uname_config)) {
				debug("*  ramdisk: config '%s' from image at 0x%08lx\n",
				      fit_uname_config, rd_addr);
				done_select = true;
			} else if (fit_parse_subimage(select, default_addr,
						      &rd_addr,
						      &fit_uname_ramdisk)) {
				debug("*  ramdisk: subimage '%s' from image at 0x%08lx\n",
				      fit_uname_ramdisk, rd_addr);
				done_select = true;
			}
		}
	}
	if (!done_select) {
		rd_addr = hextoul(select, NULL);
		debug("*  ramdisk: cmdline image address = 0x%08lx\n", rd_addr);
	}
	if (CONFIG_IS_ENABLED(FIT) && !select) {
		/* use FIT configuration provided in first bootm
		 * command argument. If the property is not defined,
		 * quit silently (with -ENOPKG)
		 */
		rd_addr = map_to_sysmem(images->fit_hdr_os);
		rd_noffset = fit_get_node_from_config(images, FIT_RAMDISK_PROP,
						      rd_addr);
		if (rd_noffset == -ENOENT)
			return -ENOPKG;
		else if (rd_noffset < 0)
			return rd_noffset;
	}

	/*
	 * Check if there is an initrd image at the
	 * address provided in the second bootm argument
	 * check image type, for FIT images get FIT node.
	 */
	buf = map_sysmem(rd_addr, 0);
	switch (genimg_get_format(buf)) {
	case IMAGE_FORMAT_LEGACY:
		if (CONFIG_IS_ENABLED(LEGACY_IMAGE_FORMAT)) {
			const struct legacy_img_hdr *rd_hdr;

			printf("## Loading init Ramdisk from Legacy Image at %08lx ...\n",
			       rd_addr);

			bootstage_mark(BOOTSTAGE_ID_CHECK_RAMDISK);
			rd_hdr = image_get_ramdisk(rd_addr, arch,
						   images->verify);

			if (!rd_hdr)
				return -ENOENT;

			*rd_datap = image_get_data(rd_hdr);
			*rd_lenp = image_get_data_size(rd_hdr);
			done = true;
		}
		break;
	case IMAGE_FORMAT_FIT:
		if (CONFIG_IS_ENABLED(FIT)) {
			rd_noffset = fit_image_load(images, rd_addr,
						    &fit_uname_ramdisk,
						    &fit_uname_config,
						    arch, IH_TYPE_RAMDISK,
						    BOOTSTAGE_ID_FIT_RD_START,
						    FIT_LOAD_OPTIONAL_NON_ZERO,
						    rd_datap, rd_lenp);
			if (rd_noffset < 0)
				return rd_noffset;

			images->fit_hdr_rd = map_sysmem(rd_addr, 0);
			images->fit_uname_rd = fit_uname_ramdisk;
			images->fit_noffset_rd = rd_noffset;
			done = true;
		}
		break;
	case IMAGE_FORMAT_ANDROID:
		if (IS_ENABLED(CONFIG_ANDROID_BOOT_IMAGE)) {
			int ret;
			if (IS_ENABLED(CONFIG_CMD_ABOOTIMG)) {
				void *boot_img = map_sysmem(get_abootimg_addr(), 0);
				void *vendor_boot_img = map_sysmem(get_avendor_bootimg_addr(), 0);

				ret = android_image_get_ramdisk(boot_img, vendor_boot_img,
								rd_datap, rd_lenp);
				unmap_sysmem(vendor_boot_img);
				unmap_sysmem(boot_img);
			} else {
				void *ptr = map_sysmem(images->os.start, 0);

				ret = android_image_get_ramdisk(ptr, NULL, rd_datap, rd_lenp);
				unmap_sysmem(ptr);
			}

			if (ret)
				return ret;
			done = true;
		}
		break;
	}

	if (!done) {
		if (IS_ENABLED(CONFIG_SUPPORT_RAW_INITRD)) {
			char *end = NULL;

			if (select)
				end = strchr(select, ':');
			if (end) {
				*rd_lenp = hextoul(++end, NULL);
				*rd_datap = rd_addr;
				done = true;
			}
		}

		if (!done) {
			puts("Wrong Ramdisk Image Format\n");
			return -EINVAL;
		}
	}

	return 0;
}

int boot_get_ramdisk(char const *select, struct bootm_headers *images,
		     uint arch, ulong *rd_start, ulong *rd_end)
{
	ulong rd_data, rd_len;

	*rd_start = 0;
	*rd_end = 0;

	/*
	 * Look for a '-' which indicates to ignore the
	 * ramdisk argument
	 */
	if (select && strcmp(select, "-") ==  0) {
		debug("## Skipping init Ramdisk\n");
		rd_len = 0;
		rd_data = 0;
	} else if (select || genimg_has_config(images)) {
		int ret;

		ret = select_ramdisk(images, select, arch, &rd_data, &rd_len);
		if (ret == -ENOPKG)
			return 0;
		else if (ret)
			return ret;
	} else if (images->legacy_hdr_valid &&
			image_check_type(&images->legacy_hdr_os_copy,
					 IH_TYPE_MULTI)) {
		/*
		 * Now check if we have a legacy mult-component image,
		 * get second entry data start address and len.
		 */
		bootstage_mark(BOOTSTAGE_ID_RAMDISK);
		printf("## Loading init Ramdisk from multi component Legacy Image at %08lx ...\n",
		       (ulong)images->legacy_hdr_os);

		image_multi_getimg(images->legacy_hdr_os, 1, &rd_data, &rd_len);
	} else {
		/*
		 * no initrd image
		 */
		bootstage_mark(BOOTSTAGE_ID_NO_RAMDISK);
		rd_len = 0;
		rd_data = 0;
	}

	if (!rd_data) {
		debug("## No init Ramdisk\n");
	} else {
		*rd_start = rd_data;
		*rd_end = rd_data + rd_len;
	}
	debug("   ramdisk start = 0x%08lx, ramdisk end = 0x%08lx\n",
	      *rd_start, *rd_end);

	return 0;
}

/**
 * boot_ramdisk_high - relocate init ramdisk
 * @lmb: pointer to lmb handle, will be used for memory mgmt
 * @rd_data: ramdisk data start address
 * @rd_len: ramdisk data length
 * @initrd_start: pointer to a ulong variable, will hold final init ramdisk
 *      start address (after possible relocation)
 * @initrd_end: pointer to a ulong variable, will hold final init ramdisk
 *      end address (after possible relocation)
 *
 * boot_ramdisk_high() takes a relocation hint from "initrd_high" environment
 * variable and if requested ramdisk data is moved to a specified location.
 *
 * Initrd_start and initrd_end are set to final (after relocation) ramdisk
 * start/end addresses if ramdisk image start and len were provided,
 * otherwise set initrd_start and initrd_end set to zeros.
 *
 * returns:
 *      0 - success
 *     -1 - failure
 */
int boot_ramdisk_high(struct lmb *lmb, ulong rd_data, ulong rd_len,
		      ulong *initrd_start, ulong *initrd_end)
{
	char	*s;
	ulong	initrd_high;
	int	initrd_copy_to_ram = 1;

	s = env_get("initrd_high");
	if (s) {
		/* a value of "no" or a similar string will act like 0,
		 * turning the "load high" feature off. This is intentional.
		 */
		initrd_high = hextoul(s, NULL);
		if (initrd_high == ~0)
			initrd_copy_to_ram = 0;
	} else {
		initrd_high = env_get_bootm_mapsize() + env_get_bootm_low();
	}

	debug("## initrd_high = 0x%08lx, copy_to_ram = %d\n",
	      initrd_high, initrd_copy_to_ram);

	if (rd_data) {
		if (!initrd_copy_to_ram) {	/* zero-copy ramdisk support */
			debug("   in-place initrd\n");
			*initrd_start = rd_data;
			*initrd_end = rd_data + rd_len;
			lmb_reserve(lmb, rd_data, rd_len);
		} else {
			if (initrd_high)
				*initrd_start = (ulong)lmb_alloc_base(lmb,
						rd_len, 0x1000, initrd_high);
			else
				*initrd_start = (ulong)lmb_alloc(lmb, rd_len,
								 0x1000);

			if (*initrd_start == 0) {
				puts("ramdisk - allocation error\n");
				goto error;
			}
			bootstage_mark(BOOTSTAGE_ID_COPY_RAMDISK);

			*initrd_end = *initrd_start + rd_len;
			printf("   Loading Ramdisk to %08lx, end %08lx ... ",
			       *initrd_start, *initrd_end);

			memmove_wd((void *)*initrd_start,
				   (void *)rd_data, rd_len, CHUNKSZ);

			/*
			 * Ensure the image is flushed to memory to handle
			 * AMP boot scenarios in which we might not be
			 * HW cache coherent
			 */
			if (IS_ENABLED(CONFIG_MP)) {
				flush_cache((unsigned long)*initrd_start,
					    ALIGN(rd_len, ARCH_DMA_MINALIGN));
			}
			puts("OK\n");
		}
	} else {
		*initrd_start = 0;
		*initrd_end = 0;
	}
	debug("   ramdisk load start = 0x%08lx, ramdisk load end = 0x%08lx\n",
	      *initrd_start, *initrd_end);

	return 0;

error:
	return -1;
}

int boot_get_setup(struct bootm_headers *images, u8 arch,
		   ulong *setup_start, ulong *setup_len)
{
	if (!CONFIG_IS_ENABLED(FIT))
		return -ENOENT;

	return boot_get_setup_fit(images, arch, setup_start, setup_len);
}

int boot_get_fpga(struct bootm_headers *images)
{
	ulong tmp_img_addr, img_data, img_len;
	void *buf;
	int conf_noffset;
	int fit_img_result;
	const char *uname, *name;
	int err;
	int devnum = 0; /* TODO support multi fpga platforms */

	if (!IS_ENABLED(CONFIG_FPGA))
		return -ENOSYS;

	/* Check to see if the images struct has a FIT configuration */
	if (!genimg_has_config(images)) {
		debug("## FIT configuration was not specified\n");
		return 0;
	}

	/*
	 * Obtain the os FIT header from the images struct
	 */
	tmp_img_addr = map_to_sysmem(images->fit_hdr_os);
	buf = map_sysmem(tmp_img_addr, 0);
	/*
	 * Check image type. For FIT images get FIT node
	 * and attempt to locate a generic binary.
	 */
	switch (genimg_get_format(buf)) {
	case IMAGE_FORMAT_FIT:
		conf_noffset = fit_conf_get_node(buf, images->fit_uname_cfg);

		uname = fdt_stringlist_get(buf, conf_noffset, FIT_FPGA_PROP, 0,
					   NULL);
		if (!uname) {
			debug("## FPGA image is not specified\n");
			return 0;
		}
		fit_img_result = fit_image_load(images,
						tmp_img_addr,
						(const char **)&uname,
						&images->fit_uname_cfg,
						IH_ARCH_DEFAULT,
						IH_TYPE_FPGA,
						BOOTSTAGE_ID_FPGA_INIT,
						FIT_LOAD_OPTIONAL_NON_ZERO,
						&img_data, &img_len);

		debug("FPGA image (%s) loaded to 0x%lx/size 0x%lx\n",
		      uname, img_data, img_len);

		if (fit_img_result < 0) {
			/* Something went wrong! */
			return fit_img_result;
		}

		if (!fpga_is_partial_data(devnum, img_len)) {
			name = "full";
			err = fpga_loadbitstream(devnum, (char *)img_data,
						 img_len, BIT_FULL);
			if (err)
				err = fpga_load(devnum, (const void *)img_data,
						img_len, BIT_FULL, 0);
		} else {
			name = "partial";
			err = fpga_loadbitstream(devnum, (char *)img_data,
						 img_len, BIT_PARTIAL);
			if (err)
				err = fpga_load(devnum, (const void *)img_data,
						img_len, BIT_PARTIAL, 0);
		}

		if (err)
			return err;

		printf("   Programming %s bitstream... OK\n", name);
		break;
	default:
		printf("The given image format is not supported (corrupt?)\n");
		return 1;
	}

	return 0;
}

static void fit_loadable_process(u8 img_type,
				 ulong img_data,
				 ulong img_len)
{
	int i;
	const unsigned int count =
			ll_entry_count(struct fit_loadable_tbl, fit_loadable);
	struct fit_loadable_tbl *fit_loadable_handler =
			ll_entry_start(struct fit_loadable_tbl, fit_loadable);
	/* For each loadable handler */
	for (i = 0; i < count; i++, fit_loadable_handler++)
		/* matching this type */
		if (fit_loadable_handler->type == img_type)
			/* call that handler with this image data */
			fit_loadable_handler->handler(img_data, img_len);
}

int boot_get_loadable(struct bootm_headers *images)
{
	/*
	 * These variables are used to hold the current image location
	 * in system memory.
	 */
	ulong tmp_img_addr;
	/*
	 * These two variables are requirements for fit_image_load, but
	 * their values are not used
	 */
	ulong img_data, img_len;
	void *buf;
	int loadables_index;
	int conf_noffset;
	int fit_img_result;
	const char *uname;
	u8 img_type;

	/* Check to see if the images struct has a FIT configuration */
	if (!genimg_has_config(images)) {
		debug("## FIT configuration was not specified\n");
		return 0;
	}

	/*
	 * Obtain the os FIT header from the images struct
	 */
	tmp_img_addr = map_to_sysmem(images->fit_hdr_os);
	buf = map_sysmem(tmp_img_addr, 0);
	/*
	 * Check image type. For FIT images get FIT node
	 * and attempt to locate a generic binary.
	 */
	switch (genimg_get_format(buf)) {
	case IMAGE_FORMAT_FIT:
		conf_noffset = fit_conf_get_node(buf, images->fit_uname_cfg);

		for (loadables_index = 0;
		     uname = fdt_stringlist_get(buf, conf_noffset,
						FIT_LOADABLE_PROP,
						loadables_index, NULL), uname;
		     loadables_index++) {
			fit_img_result = fit_image_load(images, tmp_img_addr,
							&uname,
							&images->fit_uname_cfg,
							IH_ARCH_DEFAULT,
							IH_TYPE_LOADABLE,
							BOOTSTAGE_ID_FIT_LOADABLE_START,
							FIT_LOAD_OPTIONAL_NON_ZERO,
							&img_data, &img_len);
			if (fit_img_result < 0) {
				/* Something went wrong! */
				return fit_img_result;
			}

			fit_img_result = fit_image_get_node(buf, uname);
			if (fit_img_result < 0) {
				/* Something went wrong! */
				return fit_img_result;
			}
			fit_img_result = fit_image_get_type(buf,
							    fit_img_result,
							    &img_type);
			if (fit_img_result < 0) {
				/* Something went wrong! */
				return fit_img_result;
			}

			fit_loadable_process(img_type, img_data, img_len);
		}
		break;
	default:
		printf("The given image format is not supported (corrupt?)\n");
		return 1;
	}

	return 0;
}

/**
 * boot_get_cmdline - allocate and initialize kernel cmdline
 * @lmb: pointer to lmb handle, will be used for memory mgmt
 * @cmd_start: pointer to a ulong variable, will hold cmdline start
 * @cmd_end: pointer to a ulong variable, will hold cmdline end
 *
 * This allocates space for kernel command line below
 * BOOTMAPSZ + env_get_bootm_low() address. If "bootargs" U-Boot environment
 * variable is present its contents is copied to allocated kernel
 * command line.
 *
 * returns:
 *      0 - success
 *     -1 - failure
 */
int boot_get_cmdline(struct lmb *lmb, ulong *cmd_start, ulong *cmd_end)
{
	int barg;
	char *cmdline;
	char *s;

	/*
	 * Help the compiler detect that this function is only called when
	 * CONFIG_SYS_BOOT_GET_CMDLINE is enabled
	 */
	if (!IS_ENABLED(CONFIG_SYS_BOOT_GET_CMDLINE))
		return 0;

	barg = IF_ENABLED_INT(CONFIG_SYS_BOOT_GET_CMDLINE, CONFIG_SYS_BARGSIZE);
	cmdline = (char *)(ulong)lmb_alloc_base(lmb, barg, 0xf,
				env_get_bootm_mapsize() + env_get_bootm_low());
	if (!cmdline)
		return -1;

	s = env_get("bootargs");
	if (!s)
		s = "";

	strcpy(cmdline, s);

	*cmd_start = (ulong)cmdline;
	*cmd_end = *cmd_start + strlen(cmdline);

	debug("## cmdline at 0x%08lx ... 0x%08lx\n", *cmd_start, *cmd_end);

	return 0;
}

/**
 * boot_get_kbd - allocate and initialize kernel copy of board info
 * @lmb: pointer to lmb handle, will be used for memory mgmt
 * @kbd: double pointer to board info data
 *
 * boot_get_kbd() allocates space for kernel copy of board info data below
 * BOOTMAPSZ + env_get_bootm_low() address and kernel board info is initialized
 * with the current u-boot board info data.
 *
 * returns:
 *      0 - success
 *     -1 - failure
 */
int boot_get_kbd(struct lmb *lmb, struct bd_info **kbd)
{
	*kbd = (struct bd_info *)(ulong)lmb_alloc_base(lmb,
						       sizeof(struct bd_info),
						       0xf,
						       env_get_bootm_mapsize() +
						       env_get_bootm_low());
	if (!*kbd)
		return -1;

	**kbd = *gd->bd;

	debug("## kernel board info at 0x%08lx\n", (ulong)*kbd);

	if (_DEBUG && IS_ENABLED(CONFIG_CMD_BDI))
		do_bdinfo(NULL, 0, 0, NULL);

	return 0;
}

int image_setup_linux(struct bootm_headers *images)
{
	ulong of_size = images->ft_len;
	char **of_flat_tree = &images->ft_addr;
	struct lmb *lmb = images_lmb(images);
	int ret;

	/* This function cannot be called without lmb support */
	if (!IS_ENABLED(CONFIG_LMB))
		return -EFAULT;
	if (CONFIG_IS_ENABLED(OF_LIBFDT))
		boot_fdt_add_mem_rsv_regions(lmb, *of_flat_tree);

	if (IS_ENABLED(CONFIG_SYS_BOOT_GET_CMDLINE)) {
		ret = boot_get_cmdline(lmb, &images->cmdline_start,
				       &images->cmdline_end);
		if (ret) {
			puts("ERROR with allocation of cmdline\n");
			return ret;
		}
	}

	if (CONFIG_IS_ENABLED(OF_LIBFDT)) {
		ret = boot_relocate_fdt(lmb, of_flat_tree, &of_size);
		if (ret)
			return ret;
	}

	if (CONFIG_IS_ENABLED(OF_LIBFDT) && of_size) {
		ret = image_setup_libfdt(images, *of_flat_tree, lmb);
		if (ret)
			return ret;
	}

	return 0;
}

void genimg_print_size(uint32_t size)
{
	printf("%d Bytes = ", size);
	print_size(size, "\n");
}

void genimg_print_time(time_t timestamp)
{
	struct rtc_time tm;

	rtc_to_tm(timestamp, &tm);
	printf("%4d-%02d-%02d  %2d:%02d:%02d UTC\n",
	       tm.tm_year, tm.tm_mon, tm.tm_mday,
	       tm.tm_hour, tm.tm_min, tm.tm_sec);
}

/**
 * get_default_image() - Return default property from /images
 *
 * Return: Pointer to value of default property (or NULL)
 */
static const char *get_default_image(const void *fit)
{
	int images_noffset;

	images_noffset = fdt_path_offset(fit, FIT_IMAGES_PATH);
	if (images_noffset < 0)
		return NULL;

	return fdt_getprop(fit, images_noffset, FIT_DEFAULT_PROP, NULL);
}

int image_locate_script(void *buf, int size, const char *fit_uname,
			const char *confname, char **datap, uint *lenp)
{
	const struct legacy_img_hdr *hdr;
	const void *fit_data;
	const void *fit_hdr;
	size_t fit_len;
	int noffset;
	int verify;
	ulong len;
	u32 *data;

	verify = env_get_yesno("verify");

	switch (genimg_get_format(buf)) {
	case IMAGE_FORMAT_LEGACY:
		if (!IS_ENABLED(CONFIG_LEGACY_IMAGE_FORMAT)) {
			goto exit_image_format;
		} else {
			hdr = buf;

			if (!image_check_magic(hdr)) {
				puts("Bad magic number\n");
				return 1;
			}

			if (!image_check_hcrc(hdr)) {
				puts("Bad header crc\n");
				return 1;
			}

			if (verify) {
				if (!image_check_dcrc(hdr)) {
					puts("Bad data crc\n");
					return 1;
				}
			}

			if (!image_check_type(hdr, IH_TYPE_SCRIPT)) {
				puts("Bad image type\n");
				return 1;
			}

			/* get length of script */
			data = (u32 *)image_get_data(hdr);

			len = uimage_to_cpu(*data);
			if (!len) {
				puts("Empty Script\n");
				return 1;
			}

			/*
			 * scripts are just multi-image files with one
			 * component, so seek past the zero-terminated sequence
			 * of image lengths to get to the actual image data
			 */
			while (*data++);
		}
		break;
	case IMAGE_FORMAT_FIT:
		if (!IS_ENABLED(CONFIG_FIT)) {
			goto exit_image_format;
		} else {
			fit_hdr = buf;
			if (fit_check_format(fit_hdr, IMAGE_SIZE_INVAL)) {
				puts("Bad FIT image format\n");
				return 1;
			}

			if (!fit_uname) {
				/* If confname is empty, use the default */
				if (confname && *confname)
					noffset = fit_conf_get_node(fit_hdr, confname);
				else
					noffset = fit_conf_get_node(fit_hdr, NULL);
				if (noffset < 0) {
					if (!confname)
						goto fallback;
					printf("Could not find config %s\n", confname);
					return 1;
				}

				if (verify && fit_config_verify(fit_hdr, noffset))
					return 1;

				noffset = fit_conf_get_prop_node(fit_hdr,
								 noffset,
								 FIT_SCRIPT_PROP,
								 IH_PHASE_NONE);
				if (noffset < 0) {
					if (!confname)
						goto fallback;
					printf("Could not find script in %s\n", confname);
					return 1;
				}
			} else {
fallback:
				if (!fit_uname || !*fit_uname)
					fit_uname = get_default_image(fit_hdr);
				if (!fit_uname) {
					puts("No FIT subimage unit name\n");
					return 1;
				}

				/* get script component image node offset */
				noffset = fit_image_get_node(fit_hdr, fit_uname);
				if (noffset < 0) {
					printf("Can't find '%s' FIT subimage\n",
					       fit_uname);
					return 1;
				}
			}

			if (!fit_image_check_type(fit_hdr, noffset,
						  IH_TYPE_SCRIPT)) {
				puts("Not a image image\n");
				return 1;
			}

			/* verify integrity */
			if (verify && !fit_image_verify(fit_hdr, noffset)) {
				puts("Bad Data Hash\n");
				return 1;
			}

			/* get script subimage data address and length */
			if (fit_image_get_data_and_size(fit_hdr, noffset,
							&fit_data, &fit_len)) {
				puts("Could not find script subimage data\n");
				return 1;
			}

			data = (u32 *)fit_data;
			len = (ulong)fit_len;
		}
		break;
	default:
		goto exit_image_format;
	}

	*datap = (char *)data;
	*lenp = len;

	return 0;

exit_image_format:
	puts("Wrong image format for \"source\" command\n");
	return -EPERM;
}
