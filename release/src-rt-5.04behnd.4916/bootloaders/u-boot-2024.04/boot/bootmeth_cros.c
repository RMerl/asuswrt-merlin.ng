// SPDX-License-Identifier: GPL-2.0+
/*
 * Bootmethod for ChromiumOS
 *
 * Copyright 2023 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY UCLASS_BOOTSTD

#include <common.h>
#include <blk.h>
#include <bootdev.h>
#include <bootflow.h>
#include <bootm.h>
#include <bootmeth.h>
#include <display_options.h>
#include <dm.h>
#include <efi.h>
#include <malloc.h>
#include <mapmem.h>
#include <part.h>
#include <linux/sizes.h>
#include "bootmeth_cros.h"

static const efi_guid_t cros_kern_type = PARTITION_CROS_KERNEL;

/*
 * Layout of the ChromeOS kernel
 *
 * Partitions 2 and 4 contain kernels with type GUID_CROS_KERNEL
 *
 * Contents are:
 *
 * Offset	Contents
 *   0		struct vb2_keyblock
 *   m		struct vb2_kernel_preamble
 *   m + n	kernel buffer
 *
 * m is keyblock->keyblock_size
 * n is preamble->preamble_size
 *
 * The kernel buffer itself consists of various parts:
 *
 * Offset	Contents
 *   m + n	kernel image (Flat vmlinux binary or FIT)
 *   b - 8KB	Command line text
 *   b - 4KB	X86 setup block (struct boot_params, extends for about 16KB)
 *   b          X86 bootloader (continuation of setup block)
 *   b + 16KB	X86 setup block (copy, used for hold data pointed to)
 *
 * b is m + n + preamble->bootloader_address - preamble->body_load_address
 *
 * Useful metadata extends from b - 8KB through to b + 32 KB
 */

enum {
	PROBE_SIZE	= SZ_4K,	/* initial bytes read from partition */

	X86_SETUP_OFFSET = -0x1000,	/* setup offset relative to base */
	CMDLINE_OFFSET	= -0x2000,	/* cmdline offset relative to base */
	X86_KERNEL_OFFSET = 0x4000,	/* kernel offset relative to base */
};

/**
 * struct cros_priv - Private data
 *
 * This is read from the disk and recorded for use when the full kernel must
 * be loaded and booted
 *
 * @body_offset: Offset of kernel body from start of partition (in bytes)
 * @body_size: Size of kernel body in bytes
 * @part_start: Block offset of selected partition from the start of the disk
 * @body_load_address: Nominal load address for kernel body
 * @bootloader_address: Address of bootloader, after body is loaded at
 *	body_load_address
 * @bootloader_size:  Size of bootloader in bytes
 * @info_buf: Buffer containing ChromiumOS info
 */
struct cros_priv {
	ulong body_offset;
	ulong body_size;
	lbaint_t part_start;
	ulong body_load_address;
	ulong bootloader_address;
	ulong bootloader_size;
	void *info_buf;
};

static int cros_check(struct udevice *dev, struct bootflow_iter *iter)
{
	/* This only works on block and network devices */
	if (bootflow_iter_check_blk(iter))
		return log_msg_ret("blk", -ENOTSUPP);

	return 0;
}

static int copy_cmdline(const char *from, const char *uuid, char **bufp)
{
	const int maxlen = 2048;
	char buf[maxlen];
	char *cmd, *to, *end;
	int len;

	/* Allow space for cmdline + UUID */
	len = strnlen(from, sizeof(buf));
	if (len >= maxlen)
		return -E2BIG;

	log_debug("uuid %d %s\n", uuid ? (int)strlen(uuid) : 0, uuid);
	for (to = buf, end = buf + maxlen - UUID_STR_LEN - 1; *from; from++) {
		if (to >= end)
			return -E2BIG;
		if (from[0] == '%' && from[1] == 'U' && uuid &&
		    strlen(uuid) == UUID_STR_LEN) {
			strcpy(to, uuid);
			to += UUID_STR_LEN;
			from++;
		} else {
			*to++ = *from;
		}
	}
	*to = '\0';
	len = to - buf;
	cmd = strdup(buf);
	if (!cmd)
		return -ENOMEM;
	free(*bufp);
	*bufp = cmd;

	return 0;
}

/**
 * scan_part() - Scan a kernel partition to see if has a ChromeOS header
 *
 * This reads the first PROBE_SIZE of a partition, loookng for
 * VB2_KEYBLOCK_MAGIC
 *
 * @blk: Block device to scan
 * @partnum: Partition number to scan
 * @info: Please to put partition info
 * @hdrp: Return allocated keyblock header on success
 */
static int scan_part(struct udevice *blk, int partnum,
		     struct disk_partition *info, struct vb2_keyblock **hdrp)
{
	struct blk_desc *desc = dev_get_uclass_plat(blk);
	struct vb2_keyblock *hdr;
	struct uuid type;
	ulong num_blks;
	int ret;

	if (!partnum)
		return log_msg_ret("efi", -ENOENT);

	ret = part_get_info(desc, partnum, info);
	if (ret)
		return log_msg_ret("part", ret);

	/* Check for kernel partition type */
	log_debug("part %x: type=%s\n", partnum, info->type_guid);
	if (uuid_str_to_bin(info->type_guid, (u8 *)&type, UUID_STR_FORMAT_GUID))
		return log_msg_ret("typ", -EINVAL);

	if (memcmp(&cros_kern_type, &type, sizeof(type)))
		return log_msg_ret("typ", -ENOEXEC);

	/* Make a buffer for the header information */
	num_blks = PROBE_SIZE >> desc->log2blksz;
	log_debug("Reading header, blk=%s, start=%lx, blocks=%lx\n",
		  blk->name, (ulong)info->start, num_blks);
	hdr = memalign(SZ_1K, PROBE_SIZE);
	if (!hdr)
		return log_msg_ret("hdr", -ENOMEM);
	ret = blk_read(blk, info->start, num_blks, hdr);
	if (ret != num_blks) {
		free(hdr);
		return log_msg_ret("inf", -EIO);
	}

	if (memcmp(VB2_KEYBLOCK_MAGIC, hdr->magic, VB2_KEYBLOCK_MAGIC_SIZE)) {
		free(hdr);
		log_debug("no magic\n");
		return -ENOENT;
	}

	*hdrp = hdr;

	return 0;
}

/**
 * cros_read_buf() - Read information into a buf and parse it
 *
 * @bflow: Bootflow to update
 * @buf: Buffer to use
 * @size: Size of buffer and number of bytes to read thereinto
 * @start: Start offset to read from on disk
 * @before_base: Number of bytes to read before the bootloader base
 * @uuid: UUID string if supported, else NULL
 * Return: 0 if OK, -ENOMEM if out of memory, -EIO on read failure
 */
static int cros_read_buf(struct bootflow *bflow, void *buf, ulong size,
			 loff_t start, ulong before_base, const char *uuid)
{
	struct blk_desc *desc = dev_get_uclass_plat(bflow->blk);
	ulong base, setup, cmdline, kern_base;
	ulong num_blks;
	int ret;

	num_blks = size >> desc->log2blksz;
	log_debug("Reading info to %lx, blk=%s, size=%lx, blocks=%lx\n",
		  (ulong)map_to_sysmem(buf), bflow->blk->name, size, num_blks);
	ret = blk_read(bflow->blk, start, num_blks, buf);
	if (ret != num_blks)
		return log_msg_ret("inf", -EIO);
	base = map_to_sysmem(buf) + before_base;

	setup = base + X86_SETUP_OFFSET;
	cmdline = base + CMDLINE_OFFSET;
	kern_base = base + X86_KERNEL_OFFSET;
	log_debug("base %lx setup %lx cmdline %lx kern_base %lx\n", base,
		  setup, cmdline, kern_base);

#ifdef CONFIG_X86
	const char *version;

	version = zimage_get_kernel_version(map_sysmem(setup, 0),
					    map_sysmem(kern_base, 0));
	log_debug("version %s\n", version);
	if (version)
		bflow->name = strdup(version);
#endif
	if (!bflow->name)
		bflow->name = strdup("ChromeOS");
	if (!bflow->name)
		return log_msg_ret("nam", -ENOMEM);
	bflow->os_name = strdup("ChromeOS");
	if (!bflow->os_name)
		return log_msg_ret("os", -ENOMEM);

	ret = copy_cmdline(map_sysmem(cmdline, 0), uuid, &bflow->cmdline);
	if (ret)
		return log_msg_ret("cmd", ret);
	bflow->x86_setup = map_sysmem(setup, 0);

	return 0;
}

/**
 * cros_read_info() - Read information and fill out the bootflow
 *
 * @bflow: Bootflow to update
 * @uuid: UUID string if supported, else NULL
 * @preamble: Kernel preamble information
 * Return: 0 if OK, -ENOMEM if out of memory, -EIO on read failure
 */
static int cros_read_info(struct bootflow *bflow, const char *uuid,
			  const struct vb2_kernel_preamble *preamble)
{
	struct cros_priv *priv = bflow->bootmeth_priv;
	struct udevice *blk = bflow->blk;
	struct blk_desc *desc = dev_get_uclass_plat(blk);
	ulong offset, size, before_base;
	void *buf;
	int ret;

	log_debug("Kernel preamble at %lx, version major %x, minor %x\n",
		  (ulong)map_to_sysmem(preamble),
		  preamble->header_version_major,
		  preamble->header_version_minor);

	log_debug("  - load_address %lx, bl_addr %lx, bl_size %lx\n",
		  (ulong)preamble->body_load_address,
		  (ulong)preamble->bootloader_address,
		  (ulong)preamble->bootloader_size);

	priv->body_size = preamble->body_signature.data_size;
	priv->body_load_address = preamble->body_load_address;
	priv->bootloader_address = preamble->bootloader_address;
	priv->bootloader_size = preamble->bootloader_size;
	log_debug("Kernel body at %lx size %lx\n", priv->body_offset,
		  priv->body_size);

	/* Work out how many bytes to read before the bootloader base */
	before_base = -CMDLINE_OFFSET;

	/* Read the cmdline through to the end of the bootloader */
	size = priv->bootloader_size + before_base;
	offset = priv->body_offset +
		(priv->bootloader_address - priv->body_load_address) +
		CMDLINE_OFFSET;
	buf = malloc(size);
	if (!buf)
		return log_msg_ret("buf", -ENOMEM);

	ret = cros_read_buf(bflow, buf, size,
			    priv->part_start + (offset >> desc->log2blksz),
			    before_base, uuid);
	if (ret) {
		/* Clear this since the buffer is invalid */
		bflow->x86_setup = NULL;
		free(buf);
		return log_msg_ret("pro", ret);
	}
	priv->info_buf = buf;

	return 0;
}

static int cros_read_kernel(struct bootflow *bflow)
{
	struct blk_desc *desc = dev_get_uclass_plat(bflow->blk);
	struct cros_priv *priv = bflow->bootmeth_priv;
	ulong base, setup;
	ulong num_blks;
	void *buf;
	int ret;

	bflow->size = priv->body_size;

	buf = memalign(SZ_1K, priv->body_size);
	if (!buf)
		return log_msg_ret("buf", -ENOMEM);

	/* Check that the header is not smaller than permitted */
	if (priv->body_offset < PROBE_SIZE)
		return log_msg_ret("san", EFAULT);

	/* Read kernel body */
	num_blks = priv->body_size >> desc->log2blksz;
	log_debug("Reading body to %lx, blk=%s, size=%lx, blocks=%lx\n",
		  (ulong)map_to_sysmem(buf), bflow->blk->name, priv->body_size,
		  num_blks);
	ret = blk_read(bflow->blk,
		       priv->part_start + (priv->body_offset >> desc->log2blksz),
		       num_blks, buf);
	if (ret != num_blks)
		return log_msg_ret("inf", -EIO);
	base = map_to_sysmem(buf) + priv->bootloader_address -
		priv->body_load_address;
	setup = base + X86_SETUP_OFFSET;

	bflow->buf = buf;
	bflow->x86_setup = map_sysmem(setup, 0);

	return 0;
}

static int cros_read_bootflow(struct udevice *dev, struct bootflow *bflow)
{
	const struct vb2_kernel_preamble *preamble;
	struct disk_partition info;
	struct vb2_keyblock *hdr;
	const char *uuid = NULL;
	struct cros_priv *priv;
	int ret;

	log_debug("starting, part=%x\n", bflow->part);

	/* Check for kernel partitions */
	ret = scan_part(bflow->blk, bflow->part, &info, &hdr);
	if (ret) {
		log_debug("- scan failed: err=%d\n", ret);
		return log_msg_ret("scan", ret);
	}

	priv = malloc(sizeof(struct cros_priv));
	if (!priv) {
		free(hdr);
		return log_msg_ret("buf", -ENOMEM);
	}
	bflow->bootmeth_priv = priv;

	log_debug("Selected partition %d, header at %lx\n", bflow->part,
		  (ulong)map_to_sysmem(hdr));

	/* Grab a few things from the preamble */
	preamble = (void *)hdr + hdr->keyblock_size;
	priv->body_offset = hdr->keyblock_size + preamble->preamble_size;
	priv->part_start = info.start;

	/* Now read everything we can learn about kernel */
#if CONFIG_IS_ENABLED(PARTITION_UUIDS)
	uuid = info.uuid;
#endif
	ret = cros_read_info(bflow, uuid, preamble);
	preamble = NULL;
	free(hdr);
	if (ret) {
		free(priv->info_buf);
		free(priv);
		return log_msg_ret("inf", ret);
	}
	bflow->size = priv->body_size;
	bflow->state = BOOTFLOWST_READY;

	return 0;
}

static int cros_read_file(struct udevice *dev, struct bootflow *bflow,
			 const char *file_path, ulong addr, ulong *sizep)
{
	return -ENOSYS;
}

#if CONFIG_IS_ENABLED(BOOTSTD_FULL)
static int cros_read_all(struct udevice *dev, struct bootflow *bflow)
{
	int ret;

	if (bflow->buf)
		return log_msg_ret("ld", -EALREADY);
	ret = cros_read_kernel(bflow);
	if (ret)
		return log_msg_ret("rd", ret);

	return 0;
}
#endif /* BOOTSTD_FULL */

static int cros_boot(struct udevice *dev, struct bootflow *bflow)
{
	int ret;

	if (!bflow->buf) {
		ret = cros_read_kernel(bflow);
		if (ret)
			return log_msg_ret("rd", ret);
	}

	if (IS_ENABLED(CONFIG_X86)) {
		ret = zboot_start(map_to_sysmem(bflow->buf), bflow->size, 0, 0,
				  map_to_sysmem(bflow->x86_setup),
				  bflow->cmdline);
	} else {
		ret = bootm_boot_start(map_to_sysmem(bflow->buf),
				       bflow->cmdline);
	}

	return log_msg_ret("go", ret);
}

static int cros_bootmeth_bind(struct udevice *dev)
{
	struct bootmeth_uc_plat *plat = dev_get_uclass_plat(dev);

	plat->desc = "ChromiumOS boot";
	plat->flags = BOOTMETHF_ANY_PART;

	return 0;
}

static struct bootmeth_ops cros_bootmeth_ops = {
	.check		= cros_check,
	.read_bootflow	= cros_read_bootflow,
	.read_file	= cros_read_file,
	.boot		= cros_boot,
#if CONFIG_IS_ENABLED(BOOTSTD_FULL)
	.read_all	= cros_read_all,
#endif /* BOOTSTD_FULL */
};

static const struct udevice_id cros_bootmeth_ids[] = {
	{ .compatible = "u-boot,cros" },
	{ }
};

U_BOOT_DRIVER(bootmeth_cros) = {
	.name		= "bootmeth_cros",
	.id		= UCLASS_BOOTMETH,
	.of_match	= cros_bootmeth_ids,
	.ops		= &cros_bootmeth_ops,
	.bind		= cros_bootmeth_bind,
};
