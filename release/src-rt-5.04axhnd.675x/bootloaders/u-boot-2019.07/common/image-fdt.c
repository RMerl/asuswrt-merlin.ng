// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013, Google Inc.
 *
 * (C) Copyright 2008 Semihalf
 *
 * (C) Copyright 2000-2006
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <common.h>
#include <fdt_support.h>
#include <fdtdec.h>
#include <errno.h>
#include <image.h>
#include <linux/libfdt.h>
#include <mapmem.h>
#include <asm/io.h>

#ifndef CONFIG_SYS_FDT_PAD
#define CONFIG_SYS_FDT_PAD 0x3000
#endif

/* adding a ramdisk needs 0x44 bytes in version 2008.10 */
#define FDT_RAMDISK_OVERHEAD	0x80

DECLARE_GLOBAL_DATA_PTR;

static void fdt_error(const char *msg)
{
	puts("ERROR: ");
	puts(msg);
	puts(" - must RESET the board to recover.\n");
}

#if defined(CONFIG_IMAGE_FORMAT_LEGACY)
static const image_header_t *image_get_fdt(ulong fdt_addr)
{
	const image_header_t *fdt_hdr = map_sysmem(fdt_addr, 0);

	image_print_contents(fdt_hdr);

	puts("   Verifying Checksum ... ");
	if (!image_check_hcrc(fdt_hdr)) {
		fdt_error("fdt header checksum invalid");
		return NULL;
	}

	if (!image_check_dcrc(fdt_hdr)) {
		fdt_error("fdt checksum invalid");
		return NULL;
	}
	puts("OK\n");

	if (!image_check_type(fdt_hdr, IH_TYPE_FLATDT)) {
		fdt_error("uImage is not a fdt");
		return NULL;
	}
	if (image_get_comp(fdt_hdr) != IH_COMP_NONE) {
		fdt_error("uImage is compressed");
		return NULL;
	}
	if (fdt_check_header((void *)image_get_data(fdt_hdr)) != 0) {
		fdt_error("uImage data is not a fdt");
		return NULL;
	}
	return fdt_hdr;
}
#endif

static void boot_fdt_reserve_region(struct lmb *lmb, uint64_t addr,
				    uint64_t size)
{
	long ret;

	ret = lmb_reserve(lmb, addr, size);
	if (ret >= 0) {
		debug("   reserving fdt memory region: addr=%llx size=%llx\n",
		      (unsigned long long)addr, (unsigned long long)size);
	} else {
		puts("ERROR: reserving fdt memory region failed ");
		printf("(addr=%llx size=%llx)\n",
		       (unsigned long long)addr, (unsigned long long)size);
	}
}

/**
 * boot_fdt_add_mem_rsv_regions - Mark the memreserve and reserved-memory
 * sections as unusable
 * @lmb: pointer to lmb handle, will be used for memory mgmt
 * @fdt_blob: pointer to fdt blob base address
 *
 * Adds the and reserved-memorymemreserve regions in the dtb to the lmb block.
 * Adding the memreserve regions prevents u-boot from using them to store the
 * initrd or the fdt blob.
 */
void boot_fdt_add_mem_rsv_regions(struct lmb *lmb, void *fdt_blob)
{
	uint64_t addr, size;
	int i, total, ret;
	int nodeoffset, subnode;
	struct fdt_resource res;

	if (fdt_check_header(fdt_blob) != 0)
		return;

	/* process memreserve sections */
	total = fdt_num_mem_rsv(fdt_blob);
	for (i = 0; i < total; i++) {
		if (fdt_get_mem_rsv(fdt_blob, i, &addr, &size) != 0)
			continue;
		boot_fdt_reserve_region(lmb, addr, size);
	}

	/* process reserved-memory */
	nodeoffset = fdt_subnode_offset(fdt_blob, 0, "reserved-memory");
	if (nodeoffset >= 0) {
		subnode = fdt_first_subnode(fdt_blob, nodeoffset);
		while (subnode >= 0) {
			/* check if this subnode has a reg property */
			ret = fdt_get_resource(fdt_blob, subnode, "reg", 0,
					       &res);
			if (!ret) {
				addr = res.start;
				size = res.end - res.start + 1;
				boot_fdt_reserve_region(lmb, addr, size);
			}

			subnode = fdt_next_subnode(fdt_blob, subnode);
		}
	}
}

/**
 * boot_relocate_fdt - relocate flat device tree
 * @lmb: pointer to lmb handle, will be used for memory mgmt
 * @of_flat_tree: pointer to a char* variable, will hold fdt start address
 * @of_size: pointer to a ulong variable, will hold fdt length
 *
 * boot_relocate_fdt() allocates a region of memory within the bootmap and
 * relocates the of_flat_tree into that region, even if the fdt is already in
 * the bootmap.  It also expands the size of the fdt by CONFIG_SYS_FDT_PAD
 * bytes.
 *
 * of_flat_tree and of_size are set to final (after relocation) values
 *
 * returns:
 *      0 - success
 *      1 - failure
 */
int boot_relocate_fdt(struct lmb *lmb, char **of_flat_tree, ulong *of_size)
{
	void	*fdt_blob = *of_flat_tree;
	void	*of_start = NULL;
	char	*fdt_high;
	ulong	of_len = 0;
	int	err;
	int	disable_relocation = 0;

	/* nothing to do */
	if (*of_size == 0)
		return 0;

	if (fdt_check_header(fdt_blob) != 0) {
		fdt_error("image is not a fdt");
		goto error;
	}

	/* position on a 4K boundary before the alloc_current */
	/* Pad the FDT by a specified amount */
	of_len = *of_size + CONFIG_SYS_FDT_PAD;

	/* If fdt_high is set use it to select the relocation address */
	fdt_high = env_get("fdt_high");
	if (fdt_high) {
		void *desired_addr = (void *)simple_strtoul(fdt_high, NULL, 16);

		if (((ulong) desired_addr) == ~0UL) {
			/* All ones means use fdt in place */
			of_start = fdt_blob;
			lmb_reserve(lmb, (ulong)of_start, of_len);
			disable_relocation = 1;
		} else if (desired_addr) {
			of_start =
			    (void *)(ulong) lmb_alloc_base(lmb, of_len, 0x1000,
							   (ulong)desired_addr);
			if (of_start == NULL) {
				puts("Failed using fdt_high value for Device Tree");
				goto error;
			}
		} else {
			of_start =
			    (void *)(ulong) lmb_alloc(lmb, of_len, 0x1000);
		}
	} else {
		of_start =
		    (void *)(ulong) lmb_alloc_base(lmb, of_len, 0x1000,
						   env_get_bootm_mapsize()
						   + env_get_bootm_low());
	}

	if (of_start == NULL) {
		puts("device tree - allocation error\n");
		goto error;
	}

	if (disable_relocation) {
		/*
		 * We assume there is space after the existing fdt to use
		 * for padding
		 */
		fdt_set_totalsize(of_start, of_len);
		printf("   Using Device Tree in place at %p, end %p\n",
		       of_start, of_start + of_len - 1);
	} else {
		debug("## device tree at %p ... %p (len=%ld [0x%lX])\n",
		      fdt_blob, fdt_blob + *of_size - 1, of_len, of_len);

		printf("   Loading Device Tree to %p, end %p ... ",
		       of_start, of_start + of_len - 1);

		err = fdt_open_into(fdt_blob, of_start, of_len);
		if (err != 0) {
			fdt_error("fdt move failed");
			goto error;
		}
		puts("OK\n");
	}

	*of_flat_tree = of_start;
	*of_size = of_len;

	if (CONFIG_IS_ENABLED(CMD_FDT))
		set_working_fdt_addr(map_to_sysmem(*of_flat_tree));
	return 0;

error:
	return 1;
}

/**
 * boot_get_fdt - main fdt handling routine
 * @argc: command argument count
 * @argv: command argument list
 * @arch: architecture (IH_ARCH_...)
 * @images: pointer to the bootm images structure
 * @of_flat_tree: pointer to a char* variable, will hold fdt start address
 * @of_size: pointer to a ulong variable, will hold fdt length
 *
 * boot_get_fdt() is responsible for finding a valid flat device tree image.
 * Curently supported are the following ramdisk sources:
 *      - multicomponent kernel/ramdisk image,
 *      - commandline provided address of decicated ramdisk image.
 *
 * returns:
 *     0, if fdt image was found and valid, or skipped
 *     of_flat_tree and of_size are set to fdt start address and length if
 *     fdt image is found and valid
 *
 *     1, if fdt image is found but corrupted
 *     of_flat_tree and of_size are set to 0 if no fdt exists
 */
int boot_get_fdt(int flag, int argc, char * const argv[], uint8_t arch,
		bootm_headers_t *images, char **of_flat_tree, ulong *of_size)
{
#if defined(CONFIG_IMAGE_FORMAT_LEGACY)
	const image_header_t *fdt_hdr;
	ulong		load, load_end;
	ulong		image_start, image_data, image_end;
#endif
	ulong		img_addr;
	ulong		fdt_addr;
	char		*fdt_blob = NULL;
	void		*buf;
#if CONFIG_IS_ENABLED(FIT)
	const char	*fit_uname_config = images->fit_uname_cfg;
	const char	*fit_uname_fdt = NULL;
	ulong		default_addr;
	int		fdt_noffset;
#endif
	const char *select = NULL;

	*of_flat_tree = NULL;
	*of_size = 0;

	img_addr = (argc == 0) ? load_addr : simple_strtoul(argv[0], NULL, 16);
	buf = map_sysmem(img_addr, 0);

	if (argc > 2)
		select = argv[2];
	if (select || genimg_has_config(images)) {
#if CONFIG_IS_ENABLED(FIT)
		if (select) {
			/*
			 * If the FDT blob comes from the FIT image and the
			 * FIT image address is omitted in the command line
			 * argument, try to use ramdisk or os FIT image
			 * address or default load address.
			 */
			if (images->fit_uname_rd)
				default_addr = (ulong)images->fit_hdr_rd;
			else if (images->fit_uname_os)
				default_addr = (ulong)images->fit_hdr_os;
			else
				default_addr = load_addr;

			if (fit_parse_conf(select, default_addr,
					   &fdt_addr, &fit_uname_config)) {
				debug("*  fdt: config '%s' from image at 0x%08lx\n",
				      fit_uname_config, fdt_addr);
			} else if (fit_parse_subimage(select, default_addr,
				   &fdt_addr, &fit_uname_fdt)) {
				debug("*  fdt: subimage '%s' from image at 0x%08lx\n",
				      fit_uname_fdt, fdt_addr);
			} else
#endif
			{
				fdt_addr = simple_strtoul(select, NULL, 16);
				debug("*  fdt: cmdline image address = 0x%08lx\n",
				      fdt_addr);
			}
#if CONFIG_IS_ENABLED(FIT)
		} else {
			/* use FIT configuration provided in first bootm
			 * command argument
			 */
			fdt_addr = map_to_sysmem(images->fit_hdr_os);
			fdt_noffset = fit_get_node_from_config(images,
							       FIT_FDT_PROP,
							       fdt_addr);
			if (fdt_noffset == -ENOENT)
				return 0;
			else if (fdt_noffset < 0)
				return 1;
		}
#endif
		debug("## Checking for 'FDT'/'FDT Image' at %08lx\n",
		      fdt_addr);

		/*
		 * Check if there is an FDT image at the
		 * address provided in the second bootm argument
		 * check image type, for FIT images get a FIT node.
		 */
		buf = map_sysmem(fdt_addr, 0);
		switch (genimg_get_format(buf)) {
#if defined(CONFIG_IMAGE_FORMAT_LEGACY)
		case IMAGE_FORMAT_LEGACY:
			/* verify fdt_addr points to a valid image header */
			printf("## Flattened Device Tree from Legacy Image at %08lx\n",
			       fdt_addr);
			fdt_hdr = image_get_fdt(fdt_addr);
			if (!fdt_hdr)
				goto no_fdt;

			/*
			 * move image data to the load address,
			 * make sure we don't overwrite initial image
			 */
			image_start = (ulong)fdt_hdr;
			image_data = (ulong)image_get_data(fdt_hdr);
			image_end = image_get_image_end(fdt_hdr);

			load = image_get_load(fdt_hdr);
			load_end = load + image_get_data_size(fdt_hdr);

			if (load == image_start ||
			    load == image_data) {
				fdt_addr = load;
				break;
			}

			if ((load < image_end) && (load_end > image_start)) {
				fdt_error("fdt overwritten");
				goto error;
			}

			debug("   Loading FDT from 0x%08lx to 0x%08lx\n",
			      image_data, load);

			memmove((void *)load,
				(void *)image_data,
				image_get_data_size(fdt_hdr));

			fdt_addr = load;
			break;
#endif
		case IMAGE_FORMAT_FIT:
			/*
			 * This case will catch both: new uImage format
			 * (libfdt based) and raw FDT blob (also libfdt
			 * based).
			 */
#if CONFIG_IS_ENABLED(FIT)
			/* check FDT blob vs FIT blob */
			if (fit_check_format(buf)) {
				ulong load, len;

				fdt_noffset = boot_get_fdt_fit(images,
					fdt_addr, &fit_uname_fdt,
					&fit_uname_config,
					arch, &load, &len);

				images->fit_hdr_fdt = map_sysmem(fdt_addr, 0);
				images->fit_uname_fdt = fit_uname_fdt;
				images->fit_noffset_fdt = fdt_noffset;
				fdt_addr = load;

				break;
			} else
#endif
			{
				/*
				 * FDT blob
				 */
				debug("*  fdt: raw FDT blob\n");
				printf("## Flattened Device Tree blob at %08lx\n",
				       (long)fdt_addr);
			}
			break;
		default:
			puts("ERROR: Did not find a cmdline Flattened Device Tree\n");
			goto no_fdt;
		}

		printf("   Booting using the fdt blob at %#08lx\n", fdt_addr);
		fdt_blob = map_sysmem(fdt_addr, 0);
	} else if (images->legacy_hdr_valid &&
			image_check_type(&images->legacy_hdr_os_copy,
					 IH_TYPE_MULTI)) {
		ulong fdt_data, fdt_len;

		/*
		 * Now check if we have a legacy multi-component image,
		 * get second entry data start address and len.
		 */
		printf("## Flattened Device Tree from multi component Image at %08lX\n",
		       (ulong)images->legacy_hdr_os);

		image_multi_getimg(images->legacy_hdr_os, 2, &fdt_data,
				   &fdt_len);
		if (fdt_len) {
			fdt_blob = (char *)fdt_data;
			printf("   Booting using the fdt at 0x%p\n", fdt_blob);

			if (fdt_check_header(fdt_blob) != 0) {
				fdt_error("image is not a fdt");
				goto error;
			}

			if (fdt_totalsize(fdt_blob) != fdt_len) {
				fdt_error("fdt size != image size");
				goto error;
			}
		} else {
			debug("## No Flattened Device Tree\n");
			goto no_fdt;
		}
#ifdef CONFIG_ANDROID_BOOT_IMAGE
	} else if (genimg_get_format(buf) == IMAGE_FORMAT_ANDROID) {
		struct andr_img_hdr *hdr = buf;
		ulong fdt_data, fdt_len;

		if (!android_image_get_second(hdr, &fdt_data, &fdt_len) &&
		    !fdt_check_header((char *)fdt_data)) {
			fdt_blob = (char *)fdt_data;
			if (fdt_totalsize(fdt_blob) != fdt_len)
				goto error;

			debug("## Using FDT in Android image second area\n");
		} else {
			fdt_addr = env_get_hex("fdtaddr", 0);
			if (!fdt_addr)
				goto no_fdt;

			fdt_blob = map_sysmem(fdt_addr, 0);
			if (fdt_check_header(fdt_blob))
				goto no_fdt;

			debug("## Using FDT at ${fdtaddr}=Ox%lx\n", fdt_addr);
		}
#endif
	} else {
		debug("## No Flattened Device Tree\n");
		goto no_fdt;
	}

	*of_flat_tree = fdt_blob;
	*of_size = fdt_totalsize(fdt_blob);
	debug("   of_flat_tree at 0x%08lx size 0x%08lx\n",
	      (ulong)*of_flat_tree, *of_size);

	return 0;

no_fdt:
	debug("Continuing to boot without FDT\n");
	return 0;
error:
	return 1;
}

/*
 * Verify the device tree.
 *
 * This function is called after all device tree fix-ups have been enacted,
 * so that the final device tree can be verified.  The definition of "verified"
 * is up to the specific implementation.  However, it generally means that the
 * addresses of some of the devices in the device tree are compared with the
 * actual addresses at which U-Boot has placed them.
 *
 * Returns 1 on success, 0 on failure.  If 0 is returned, U-Boot will halt the
 * boot process.
 */
__weak int ft_verify_fdt(void *fdt)
{
	return 1;
}

__weak int arch_fixup_fdt(void *blob)
{
	return 0;
}

int image_setup_libfdt(bootm_headers_t *images, void *blob,
		       int of_size, struct lmb *lmb)
{
	ulong *initrd_start = &images->initrd_start;
	ulong *initrd_end = &images->initrd_end;
	int ret = -EPERM;
	int fdt_ret;

	if (fdt_root(blob) < 0) {
		printf("ERROR: root node setup failed\n");
		goto err;
	}
	if (fdt_chosen(blob) < 0) {
		printf("ERROR: /chosen node create failed\n");
		goto err;
	}
	if (arch_fixup_fdt(blob) < 0) {
		printf("ERROR: arch-specific fdt fixup failed\n");
		goto err;
	}
	/* Update ethernet nodes */
	fdt_fixup_ethernet(blob);
	if (IMAGE_OF_BOARD_SETUP) {
		fdt_ret = ft_board_setup(blob, gd->bd);
		if (fdt_ret) {
			printf("ERROR: board-specific fdt fixup failed: %s\n",
			       fdt_strerror(fdt_ret));
			goto err;
		}
	}
	if (IMAGE_OF_SYSTEM_SETUP) {
		fdt_ret = ft_system_setup(blob, gd->bd);
		if (fdt_ret) {
			printf("ERROR: system-specific fdt fixup failed: %s\n",
			       fdt_strerror(fdt_ret));
			goto err;
		}
	}

	/* Delete the old LMB reservation */
	if (lmb)
		lmb_free(lmb, (phys_addr_t)(u32)(uintptr_t)blob,
			 (phys_size_t)fdt_totalsize(blob));

	ret = fdt_shrink_to_minimum(blob, 0);
	if (ret < 0)
		goto err;
	of_size = ret;

	if (*initrd_start && *initrd_end) {
		of_size += FDT_RAMDISK_OVERHEAD;
		fdt_set_totalsize(blob, of_size);
	}
	/* Create a new LMB reservation */
	if (lmb)
		lmb_reserve(lmb, (ulong)blob, of_size);

	fdt_initrd(blob, *initrd_start, *initrd_end);
	if (!ft_verify_fdt(blob))
		goto err;

#if defined(CONFIG_SOC_KEYSTONE)
	if (IMAGE_OF_BOARD_SETUP)
		ft_board_setup_ex(blob, gd->bd);
#endif

	return 0;
err:
	printf(" - must RESET the board to recover.\n\n");

	return ret;
}
