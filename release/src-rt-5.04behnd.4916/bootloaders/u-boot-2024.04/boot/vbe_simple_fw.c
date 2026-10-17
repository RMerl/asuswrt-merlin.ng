// SPDX-License-Identifier: GPL-2.0
/*
 * Verified Boot for Embedded (VBE) loading firmware phases
 *
 * Copyright 2022 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY LOGC_BOOT

#include <common.h>
#include <bloblist.h>
#include <bootdev.h>
#include <bootflow.h>
#include <bootmeth.h>
#include <bootstage.h>
#include <dm.h>
#include <image.h>
#include <log.h>
#include <mapmem.h>
#include <memalign.h>
#include <mmc.h>
#include <spl.h>
#include <vbe.h>
#include <dm/device-internal.h>
#include "vbe_simple.h"

/**
 * vbe_simple_read_bootflow_fw() - Create a bootflow for firmware
 *
 * Locates and loads the firmware image (FIT) needed for the next phase. The FIT
 * should ideally use external data, to reduce the amount of it that needs to be
 * read.
 *
 * @bdev: bootdev device containing the firmwre
 * @meth: VBE simple bootmeth
 * @blow: Place to put the created bootflow, on success
 * @return 0 if OK, -ve on error
 */
int vbe_simple_read_bootflow_fw(struct udevice *dev, struct bootflow *bflow)
{
	ALLOC_CACHE_ALIGN_BUFFER(u8, sbuf, MMC_MAX_BLOCK_LEN);
	struct udevice *media = dev_get_parent(bflow->dev);
	struct udevice *meth = bflow->method;
	struct simple_priv *priv = dev_get_priv(meth);
	const char *fit_uname, *fit_uname_config;
	struct bootm_headers images = {};
	ulong offset, size, blknum, addr, len, load_addr, num_blks;
	enum image_phase_t phase;
	struct blk_desc *desc;
	struct udevice *blk;
	int node, ret;
	void *buf;

	log_debug("media=%s\n", media->name);
	ret = blk_get_from_parent(media, &blk);
	if (ret)
		return log_msg_ret("med", ret);
	log_debug("blk=%s\n", blk->name);
	desc = dev_get_uclass_plat(blk);

	offset = priv->area_start + priv->skip_offset;

	/* read in one block to find the FIT size */
	blknum =  offset / desc->blksz;
	log_debug("read at %lx, blknum %lx\n", offset, blknum);
	ret = blk_read(blk, blknum, 1, sbuf);
	if (ret < 0)
		return log_msg_ret("rd", ret);

	ret = fdt_check_header(sbuf);
	if (ret < 0)
		return log_msg_ret("fdt", -EINVAL);
	size = fdt_totalsize(sbuf);
	if (size > priv->area_size)
		return log_msg_ret("fdt", -E2BIG);
	log_debug("FIT size %lx\n", size);

	/*
	 * Load the FIT into the SPL memory. This is typically a FIT with
	 * external data, so this is quite small, perhaps a few KB.
	 */
	addr = CONFIG_VAL(TEXT_BASE);
	buf = map_sysmem(addr, size);
	num_blks = DIV_ROUND_UP(size, desc->blksz);
	log_debug("read %lx, %lx blocks to %lx / %p\n", size, num_blks, addr,
		  buf);
	ret = blk_read(blk, blknum, num_blks, buf);
	if (ret < 0)
		return log_msg_ret("rd", ret);

	/* figure out the phase to load */
	phase = IS_ENABLED(CONFIG_VPL_BUILD) ? IH_PHASE_SPL : IH_PHASE_U_BOOT;

	/*
	 * Load the image from the FIT. We ignore any load-address information
	 * so in practice this simply locates the image in the external-data
	 * region and returns its address and size. Since we only loaded the FIT
	 * itself, only a part of the image will be present, at best.
	 */
	fit_uname = NULL;
	fit_uname_config = NULL;
	log_debug("loading FIT\n");
	ret = fit_image_load(&images, addr, &fit_uname, &fit_uname_config,
			     IH_ARCH_SANDBOX, image_ph(phase, IH_TYPE_FIRMWARE),
			     BOOTSTAGE_ID_FIT_SPL_START, FIT_LOAD_IGNORED,
			     &load_addr, &len);
	if (ret < 0)
		return log_msg_ret("ld", ret);
	node = ret;
	log_debug("loaded to %lx\n", load_addr);

	/* For FIT external data, read in the external data */
	if (load_addr + len > addr + size) {
		ulong base, full_size;
		void *base_buf;

		/* Find the start address to load from */
		base = ALIGN_DOWN(load_addr, desc->blksz);

		/*
		 * Get the total number of bytes to load, taking care of
		 * block alignment
		 */
		full_size = load_addr + len - base;

		/*
		 * Get the start block number, number of blocks and the address
		 * to load to, then load the blocks
		 */
		blknum = (offset + base - addr) / desc->blksz;
		num_blks = DIV_ROUND_UP(full_size, desc->blksz);
		base_buf = map_sysmem(base, full_size);
		ret = blk_read(blk, blknum, num_blks, base_buf);
		log_debug("read %lx %lx, %lx blocks to %lx / %p: ret=%d\n",
			  blknum, full_size, num_blks, base, base_buf, ret);
		if (ret < 0)
			return log_msg_ret("rd", ret);
	}

	/* set up the bootflow with the info we obtained */
	bflow->name = strdup(fdt_get_name(buf, node, NULL));
	if (!bflow->name)
		return log_msg_ret("name", -ENOMEM);
	bflow->blk = blk;
	bflow->buf = map_sysmem(load_addr, len);
	bflow->size = len;

	return 0;
}

static int simple_load_from_image(struct spl_image_info *spl_image,
				  struct spl_boot_device *bootdev)
{
	struct udevice *meth, *bdev;
	struct simple_priv *priv;
	struct bootflow bflow;
	struct vbe_handoff *handoff;
	int ret;

	if (spl_phase() != PHASE_VPL && spl_phase() != PHASE_SPL)
		return -ENOENT;

	ret = bloblist_ensure_size(BLOBLISTT_VBE, sizeof(struct vbe_handoff),
				   0, (void **)&handoff);
	if (ret)
		return log_msg_ret("ro", ret);

	vbe_find_first_device(&meth);
	if (!meth)
		return log_msg_ret("vd", -ENODEV);
	log_debug("vbe dev %s\n", meth->name);
	ret = device_probe(meth);
	if (ret)
		return log_msg_ret("probe", ret);

	priv = dev_get_priv(meth);
	log_debug("simple %s\n", priv->storage);
	ret = bootdev_find_by_label(priv->storage, &bdev, NULL);
	if (ret)
		return log_msg_ret("bd", ret);
	log_debug("bootdev %s\n", bdev->name);

	bootflow_init(&bflow, bdev, meth);
	ret = bootmeth_read_bootflow(meth, &bflow);
	log_debug("\nfw ret=%d\n", ret);
	if (ret)
		return log_msg_ret("rd", ret);

	/* jump to the image */
	spl_image->flags = SPL_SANDBOXF_ARG_IS_BUF;
	spl_image->arg = bflow.buf;
	spl_image->size = bflow.size;
	log_debug("Image: %s at %p size %x\n", bflow.name, bflow.buf,
		  bflow.size);

	/* this is not used from now on, so free it */
	bootflow_free(&bflow);

	/* Record that VBE was used in this phase */
	handoff->phases |= 1 << spl_phase();

	return 0;
}
SPL_LOAD_IMAGE_METHOD("vbe_simple", 5, BOOT_DEVICE_VBE,
		      simple_load_from_image);
