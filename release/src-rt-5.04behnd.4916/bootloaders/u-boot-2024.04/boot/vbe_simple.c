// SPDX-License-Identifier: GPL-2.0+
/*
 * Verified Boot for Embedded (VBE) 'simple' method
 *
 * Copyright 2022 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY LOGC_BOOT

#include <common.h>
#include <bootdev.h>
#include <bootflow.h>
#include <bootmeth.h>
#include <dm.h>
#include <log.h>
#include <memalign.h>
#include <mmc.h>
#include <vbe.h>
#include <dm/device-internal.h>
#include <dm/ofnode.h>
#include <u-boot/crc.h>
#include "vbe_simple.h"

/** struct simple_nvdata - storage format for non-volatile data */
struct simple_nvdata {
	u8 crc8;
	u8 hdr;
	u16 spare1;
	u32 fw_vernum;
	u8 spare2[0x38];
};

static int simple_read_version(struct udevice *dev, struct blk_desc *desc,
			       u8 *buf, struct simple_state *state)
{
	struct simple_priv *priv = dev_get_priv(dev);
	int start;

	if (priv->version_size > MMC_MAX_BLOCK_LEN)
		return log_msg_ret("ver", -E2BIG);

	start = priv->area_start + priv->version_offset;
	if (start & (MMC_MAX_BLOCK_LEN - 1))
		return log_msg_ret("get", -EBADF);
	start /= MMC_MAX_BLOCK_LEN;

	if (blk_dread(desc, start, 1, buf) != 1)
		return log_msg_ret("read", -EIO);
	strlcpy(state->fw_version, buf, MAX_VERSION_LEN);
	log_debug("version=%s\n", state->fw_version);

	return 0;
}

static int simple_read_nvdata(struct udevice *dev, struct blk_desc *desc,
			      u8 *buf, struct simple_state *state)
{
	struct simple_priv *priv = dev_get_priv(dev);
	uint hdr_ver, hdr_size, size, crc;
	const struct simple_nvdata *nvd;
	int start;

	if (priv->state_size > MMC_MAX_BLOCK_LEN)
		return log_msg_ret("state", -E2BIG);

	start = priv->area_start + priv->state_offset;
	if (start & (MMC_MAX_BLOCK_LEN - 1))
		return log_msg_ret("get", -EBADF);
	start /= MMC_MAX_BLOCK_LEN;

	if (blk_dread(desc, start, 1, buf) != 1)
		return log_msg_ret("read", -EIO);
	nvd = (struct simple_nvdata *)buf;
	hdr_ver = (nvd->hdr & NVD_HDR_VER_MASK) >> NVD_HDR_VER_SHIFT;
	hdr_size = (nvd->hdr & NVD_HDR_SIZE_MASK) >> NVD_HDR_SIZE_SHIFT;
	if (hdr_ver != NVD_HDR_VER_CUR)
		return log_msg_ret("hdr", -EPERM);
	size = 1 << hdr_size;
	if (size > sizeof(*nvd))
		return log_msg_ret("sz", -ENOEXEC);

	crc = crc8(0, buf + 1, size - 1);
	if (crc != nvd->crc8)
		return log_msg_ret("crc", -EPERM);
	state->fw_vernum = nvd->fw_vernum;

	log_debug("version=%s\n", state->fw_version);

	return 0;
}

int vbe_simple_read_state(struct udevice *dev, struct simple_state *state)
{
	ALLOC_CACHE_ALIGN_BUFFER(u8, buf, MMC_MAX_BLOCK_LEN);
	struct simple_priv *priv = dev_get_priv(dev);
	struct blk_desc *desc;
	char devname[16];
	const char *end;
	int devnum;
	int ret;

	/* First figure out the block device */
	log_debug("storage=%s\n", priv->storage);
	devnum = trailing_strtoln_end(priv->storage, NULL, &end);
	if (devnum == -1)
		return log_msg_ret("num", -ENODEV);
	if (end - priv->storage >= sizeof(devname))
		return log_msg_ret("end", -E2BIG);
	strlcpy(devname, priv->storage, end - priv->storage + 1);
	log_debug("dev=%s, %x\n", devname, devnum);

	desc = blk_get_dev(devname, devnum);
	if (!desc)
		return log_msg_ret("get", -ENXIO);

	ret = simple_read_version(dev, desc, buf, state);
	if (ret)
		return log_msg_ret("ver", ret);

	ret = simple_read_nvdata(dev, desc, buf, state);
	if (ret)
		return log_msg_ret("nvd", ret);

	return 0;
}

static int vbe_simple_get_state_desc(struct udevice *dev, char *buf,
				     int maxsize)
{
	struct simple_state state;
	int ret;

	ret = vbe_simple_read_state(dev, &state);
	if (ret)
		return log_msg_ret("read", ret);

	if (maxsize < 30)
		return -ENOSPC;
	snprintf(buf, maxsize, "Version: %s\nVernum: %x/%x", state.fw_version,
		 state.fw_vernum >> FWVER_KEY_SHIFT,
		 state.fw_vernum & FWVER_FW_MASK);

	return 0;
}

static int vbe_simple_read_bootflow(struct udevice *dev, struct bootflow *bflow)
{
	int ret;

	if (CONFIG_IS_ENABLED(BOOTMETH_VBE_SIMPLE_FW)) {
		if (vbe_phase() == VBE_PHASE_FIRMWARE) {
			ret = vbe_simple_read_bootflow_fw(dev, bflow);
			if (ret)
				return log_msg_ret("fw", ret);
			return 0;
		}
	}

	return -EINVAL;
}

static int vbe_simple_read_file(struct udevice *dev, struct bootflow *bflow,
				const char *file_path, ulong addr, ulong *sizep)
{
	int ret;

	if (vbe_phase() == VBE_PHASE_OS) {
		ret = bootmeth_common_read_file(dev, bflow, file_path, addr,
						sizep);
		if (ret)
			return log_msg_ret("os", ret);
	}

	/* To be implemented */
	return -EINVAL;
}

static struct bootmeth_ops bootmeth_vbe_simple_ops = {
	.get_state_desc	= vbe_simple_get_state_desc,
	.read_bootflow	= vbe_simple_read_bootflow,
	.read_file	= vbe_simple_read_file,
};

static int bootmeth_vbe_simple_probe(struct udevice *dev)
{
	struct simple_priv *priv = dev_get_priv(dev);

	memset(priv, '\0', sizeof(*priv));
	if (dev_read_u32(dev, "area-start", &priv->area_start) ||
	    dev_read_u32(dev, "area-size", &priv->area_size) ||
	    dev_read_u32(dev, "version-offset", &priv->version_offset) ||
	    dev_read_u32(dev, "version-size", &priv->version_size) ||
	    dev_read_u32(dev, "state-offset", &priv->state_offset) ||
	    dev_read_u32(dev, "state-size", &priv->state_size))
		return log_msg_ret("read", -EINVAL);
	dev_read_u32(dev, "skip-offset", &priv->skip_offset);
	priv->storage = strdup(dev_read_string(dev, "storage"));
	if (!priv->storage)
		return log_msg_ret("str", -EINVAL);

	return 0;
}

static int bootmeth_vbe_simple_bind(struct udevice *dev)
{
	struct bootmeth_uc_plat *plat = dev_get_uclass_plat(dev);

	plat->desc = IS_ENABLED(CONFIG_BOOTSTD_FULL) ?
		"VBE simple" : "vbe-simple";
	plat->flags = BOOTMETHF_GLOBAL;

	return 0;
}

#if CONFIG_IS_ENABLED(OF_REAL)
static const struct udevice_id generic_simple_vbe_simple_ids[] = {
	{ .compatible = "fwupd,vbe-simple" },
	{ }
};
#endif

U_BOOT_DRIVER(vbe_simple) = {
	.name	= "vbe_simple",
	.id	= UCLASS_BOOTMETH,
	.of_match = of_match_ptr(generic_simple_vbe_simple_ids),
	.ops	= &bootmeth_vbe_simple_ops,
	.bind	= bootmeth_vbe_simple_bind,
	.probe	= bootmeth_vbe_simple_probe,
	.flags	= DM_FLAG_PRE_RELOC,
	.priv_auto	= sizeof(struct simple_priv),
};
