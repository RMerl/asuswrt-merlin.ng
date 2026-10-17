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
#include <command.h>
#include <fdt_support.h>
#include <fdtdec.h>
#include <env.h>
#include <errno.h>
#include <image.h>
#include <lmb.h>
#include <log.h>
#include <malloc.h>
#include <asm/global_data.h>
#include <linux/libfdt.h>
#include <mapmem.h>
#include <asm/io.h>
#include <dm/ofnode.h>
#include <tee/optee.h>

DECLARE_GLOBAL_DATA_PTR;

static void fdt_error(const char *msg)
{
	puts("ERROR: ");
	puts(msg);
	puts(" - must RESET the board to recover.\n");
}

#if CONFIG_IS_ENABLED(LEGACY_IMAGE_FORMAT)
static const struct legacy_img_hdr *image_get_fdt(ulong fdt_addr)
{
	const struct legacy_img_hdr *fdt_hdr = map_sysmem(fdt_addr, 0);

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
				    uint64_t size, enum lmb_flags flags)
{
	long ret;

	ret = lmb_reserve_flags(lmb, addr, size, flags);
	if (ret >= 0) {
		debug("   reserving fdt memory region: addr=%llx size=%llx flags=%x\n",
		      (unsigned long long)addr,
		      (unsigned long long)size, flags);
	} else {
		puts("ERROR: reserving fdt memory region failed ");
		printf("(addr=%llx size=%llx flags=%x)\n",
		       (unsigned long long)addr,
		       (unsigned long long)size, flags);
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
	enum lmb_flags flags;

	if (fdt_check_header(fdt_blob) != 0)
		return;

	/* process memreserve sections */
	total = fdt_num_mem_rsv(fdt_blob);
	for (i = 0; i < total; i++) {
		if (fdt_get_mem_rsv(fdt_blob, i, &addr, &size) != 0)
			continue;
		boot_fdt_reserve_region(lmb, addr, size, LMB_NONE);
	}

	/* process reserved-memory */
	nodeoffset = fdt_subnode_offset(fdt_blob, 0, "reserved-memory");
	if (nodeoffset >= 0) {
		subnode = fdt_first_subnode(fdt_blob, nodeoffset);
		while (subnode >= 0) {
			/* check if this subnode has a reg property */
			ret = fdt_get_resource(fdt_blob, subnode, "reg", 0,
					       &res);
			if (!ret && fdtdec_get_is_enabled(fdt_blob, subnode)) {
				flags = LMB_NONE;
				if (fdtdec_get_bool(fdt_blob, subnode,
						    "no-map"))
					flags = LMB_NOMAP;
				addr = res.start;
				size = res.end - res.start + 1;
				boot_fdt_reserve_region(lmb, addr, size, flags);
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
	u64	start, size, usable;
	char	*fdt_high;
	ulong	mapsize, low;
	ulong	of_len = 0;
	int	bank;
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
		ulong desired_addr = hextoul(fdt_high, NULL);
		ulong addr;

		if (desired_addr == ~0UL) {
			/* All ones means use fdt in place */
			of_start = fdt_blob;
			lmb_reserve(lmb, map_to_sysmem(of_start), of_len);
			disable_relocation = 1;
		} else if (desired_addr) {
			addr = lmb_alloc_base(lmb, of_len, 0x1000,
					      desired_addr);
			of_start = map_sysmem(addr, of_len);
			if (of_start == NULL) {
				puts("Failed using fdt_high value for Device Tree");
				goto error;
			}
		} else {
			addr = lmb_alloc(lmb, of_len, 0x1000);
			of_start = map_sysmem(addr, of_len);
		}
	} else {
		mapsize = env_get_bootm_mapsize();
		low = env_get_bootm_low();
		of_start = NULL;

		for (bank = 0; bank < CONFIG_NR_DRAM_BANKS; bank++) {
			start = gd->bd->bi_dram[bank].start;
			size = gd->bd->bi_dram[bank].size;

			/* DRAM bank addresses are too low, skip it. */
			if (start + size < low)
				continue;

			usable = min(size, (u64)mapsize);

			/*
			 * At least part of this DRAM bank is usable, try
			 * using it for LMB allocation.
			 */
			of_start = map_sysmem((ulong)lmb_alloc_base(lmb,
				    of_len, 0x1000, start + usable), of_len);
			/* Allocation succeeded, use this block. */
			if (of_start != NULL)
				break;

			/*
			 * Reduce the mapping size in the next bank
			 * by the size of attempt in current bank.
			 */
			mapsize -= usable - max(start, (u64)low);
			if (!mapsize)
				break;
		}
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

	if (IS_ENABLED(CONFIG_CMD_FDT))
		set_working_fdt_addr(map_to_sysmem(*of_flat_tree));
	return 0;

error:
	return 1;
}

/**
 * select_fdt() - Select and locate the FDT to use
 *
 * @images: pointer to the bootm images structure
 * @select: name of FDT to select, or NULL for any
 * @arch: expected FDT architecture
 * @fdt_addrp: pointer to a ulong variable, will hold FDT pointer
 * Return: 0 if OK, -ENOPKG if no FDT (but an error should not be reported),
 *	other -ve value on other error
 */

static int select_fdt(struct bootm_headers *images, const char *select, u8 arch,
		      ulong *fdt_addrp)
{
	const char *buf;
	ulong fdt_addr;

#if CONFIG_IS_ENABLED(FIT)
	const char *fit_uname_config = images->fit_uname_cfg;
	const char *fit_uname_fdt = NULL;
	ulong default_addr;
	int fdt_noffset;

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
				default_addr = image_load_addr;

			if (fit_parse_conf(select, default_addr, &fdt_addr,
					   &fit_uname_config)) {
				debug("*  fdt: config '%s' from image at 0x%08lx\n",
				      fit_uname_config, fdt_addr);
			} else if (fit_parse_subimage(select, default_addr, &fdt_addr,
				   &fit_uname_fdt)) {
				debug("*  fdt: subimage '%s' from image at 0x%08lx\n",
				      fit_uname_fdt, fdt_addr);
			} else
#endif
		{
			fdt_addr = hextoul(select, NULL);
			debug("*  fdt: cmdline image address = 0x%08lx\n",
			      fdt_addr);
		}
#if CONFIG_IS_ENABLED(FIT)
	} else {
		/* use FIT configuration provided in first bootm
		 * command argument
		 */
		fdt_addr = map_to_sysmem(images->fit_hdr_os);
		fdt_noffset = fit_get_node_from_config(images, FIT_FDT_PROP,
						       fdt_addr);
		if (fdt_noffset == -ENOENT)
			return -ENOPKG;
		else if (fdt_noffset < 0)
			return fdt_noffset;
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
#if CONFIG_IS_ENABLED(LEGACY_IMAGE_FORMAT)
	case IMAGE_FORMAT_LEGACY: {
			const struct legacy_img_hdr *fdt_hdr;
			ulong load, load_end;
			ulong image_start, image_data, image_end;

			/* verify fdt_addr points to a valid image header */
			printf("## Flattened Device Tree from Legacy Image at %08lx\n",
			       fdt_addr);
			fdt_hdr = image_get_fdt(fdt_addr);
			if (!fdt_hdr)
				return -ENOPKG;

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
				return -EFAULT;
			}

			debug("   Loading FDT from 0x%08lx to 0x%08lx\n",
			      image_data, load);

			memmove((void *)load,
				(void *)image_data,
				image_get_data_size(fdt_hdr));

			fdt_addr = load;
			break;
		}
#endif
	case IMAGE_FORMAT_FIT:
		/*
		 * This case will catch both: new uImage format
		 * (libfdt based) and raw FDT blob (also libfdt
		 * based).
		 */
#if CONFIG_IS_ENABLED(FIT)
			/* check FDT blob vs FIT blob */
			if (!fit_check_format(buf, IMAGE_SIZE_INVAL)) {
				ulong load, len;

				fdt_noffset = boot_get_fdt_fit(images, fdt_addr,
							       &fit_uname_fdt,
							       &fit_uname_config,
							       arch, &load, &len);

				if (fdt_noffset < 0)
					return -ENOENT;

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
		return -ENOENT;
	}
	*fdt_addrp = fdt_addr;

	return 0;
}

int boot_get_fdt(void *buf, const char *select, uint arch,
		 struct bootm_headers *images, char **of_flat_tree,
		 ulong *of_size)
{
	char *fdt_blob = NULL;
	ulong fdt_addr;

	*of_flat_tree = NULL;
	*of_size = 0;

	if (select || genimg_has_config(images)) {
		int ret;

		ret = select_fdt(images, select, arch, &fdt_addr);
		if (ret == -ENOPKG)
			goto no_fdt;
		else if (ret)
			return 1;
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
		void *hdr = buf;
		ulong		fdt_data, fdt_len;
		u32			fdt_size, dtb_idx;
		/*
		 * Firstly check if this android boot image has dtb field.
		 */
		dtb_idx = (u32)env_get_ulong("adtb_idx", 10, 0);
		if (android_image_get_dtb_by_index((ulong)hdr, 0,
						   dtb_idx, &fdt_addr, &fdt_size)) {
			fdt_blob = (char *)map_sysmem(fdt_addr, 0);
			if (fdt_check_header(fdt_blob))
				goto no_fdt;

			debug("## Using FDT in Android image dtb area with idx %u\n", dtb_idx);
		} else if (!android_image_get_second(hdr, &fdt_data, &fdt_len) &&
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

int image_setup_libfdt(struct bootm_headers *images, void *blob,
		       struct lmb *lmb)
{
	ulong *initrd_start = &images->initrd_start;
	ulong *initrd_end = &images->initrd_end;
	int ret, fdt_ret, of_size;

	if (IS_ENABLED(CONFIG_OF_ENV_SETUP)) {
		const char *fdt_fixup;

		fdt_fixup = env_get("fdt_fixup");
		if (fdt_fixup) {
			set_working_fdt_addr(map_to_sysmem(blob));
			ret = run_command_list(fdt_fixup, -1, 0);
			if (ret)
				printf("WARNING: fdt_fixup command returned %d\n",
				       ret);
		}
	}

	ret = -EPERM;

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

	fdt_ret = optee_copy_fdt_nodes(blob);
	if (fdt_ret) {
		printf("ERROR: transfer of optee nodes to new fdt failed: %s\n",
		       fdt_strerror(fdt_ret));
		goto err;
	}

	/* Store name of configuration node as u-boot,bootconf in /chosen node */
	if (images->fit_uname_cfg)
		fdt_find_and_setprop(blob, "/chosen", "u-boot,bootconf",
					images->fit_uname_cfg,
					strlen(images->fit_uname_cfg) + 1, 1);

	/* Update ethernet nodes */
	fdt_fixup_ethernet(blob);
#if IS_ENABLED(CONFIG_CMD_PSTORE)
	/* Append PStore configuration */
	fdt_fixup_pstore(blob);
#endif
	if (IS_ENABLED(CONFIG_OF_BOARD_SETUP)) {
		const char *skip_board_fixup;

		skip_board_fixup = env_get("skip_board_fixup");
		if (skip_board_fixup && ((int)simple_strtol(skip_board_fixup, NULL, 10) == 1)) {
			printf("skip board fdt fixup\n");
		} else {
			fdt_ret = ft_board_setup(blob, gd->bd);
			if (fdt_ret) {
				printf("ERROR: board-specific fdt fixup failed: %s\n",
				       fdt_strerror(fdt_ret));
				goto err;
			}
		}
	}
	if (IS_ENABLED(CONFIG_OF_SYSTEM_SETUP)) {
		fdt_ret = ft_system_setup(blob, gd->bd);
		if (fdt_ret) {
			printf("ERROR: system-specific fdt fixup failed: %s\n",
			       fdt_strerror(fdt_ret));
			goto err;
		}
	}

	if (fdt_initrd(blob, *initrd_start, *initrd_end))
		goto err;

	if (!ft_verify_fdt(blob))
		goto err;

	/* after here we are using a livetree */
	if (!of_live_active() && CONFIG_IS_ENABLED(EVENT)) {
		struct event_ft_fixup fixup;

		fixup.tree = oftree_from_fdt(blob);
		fixup.images = images;
		if (oftree_valid(fixup.tree)) {
			ret = event_notify(EVT_FT_FIXUP, &fixup, sizeof(fixup));
			if (ret) {
				printf("ERROR: fdt fixup event failed: %d\n",
				       ret);
				goto err;
			}
		}
	}

	/* Delete the old LMB reservation */
	if (lmb)
		lmb_free(lmb, map_to_sysmem(blob), fdt_totalsize(blob));

	ret = fdt_shrink_to_minimum(blob, 0);
	if (ret < 0)
		goto err;
	of_size = ret;

	/* Create a new LMB reservation */
	if (lmb)
		lmb_reserve(lmb, map_to_sysmem(blob), of_size);

#if defined(CONFIG_ARCH_KEYSTONE)
	if (IS_ENABLED(CONFIG_OF_BOARD_SETUP))
		ft_board_setup_ex(blob, gd->bd);
#endif

	return 0;
err:
	printf(" - must RESET the board to recover.\n\n");

	return ret;
}
