// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 *
 * This file contains dummy implementations of SCSI functions requried so
 * that CONFIG_SCSI can be enabled for sandbox.
 */

#define LOG_CATEGORY UCLASS_SCSI

#include <common.h>
#include <dm.h>
#include <os.h>
#include <malloc.h>
#include <scsi.h>
#include <scsi_emul.h>

enum {
	SANDBOX_SCSI_BLOCK_LEN		= 512,
	SANDBOX_SCSI_BUF_SIZE		= 512,
};

/**
 * struct sandbox_scsi_priv
 *
 * @eminfo: emulator state
 * @pathanme: Path to the backing file, e.g. 'scsi.img'
 * @fd: File descriptor of backing file
 */
struct sandbox_scsi_priv {
	struct scsi_emul_info eminfo;
	const char *pathname;
	int fd;
};

static int sandbox_scsi_exec(struct udevice *dev, struct scsi_cmd *req)
{
	struct sandbox_scsi_priv *priv = dev_get_priv(dev);
	struct scsi_emul_info *info = &priv->eminfo;
	int ret;

	if (req->lun || req->target)
		return -EIO;
	ret = sb_scsi_emul_command(info, req, req->cmdlen);
	if (ret < 0) {
		log_debug("SCSI command 0x%02x ret errno %d\n", req->cmd[0],
			  ret);
		return ret;
	} else if (ret == SCSI_EMUL_DO_READ && priv->fd != -1) {
		long bytes_read;

		log_debug("read %x %x\n", info->seek_block, info->read_len);
		os_lseek(priv->fd, info->seek_block * info->block_size,
			 OS_SEEK_SET);
		bytes_read = os_read(priv->fd, req->pdata, info->buff_used);
		if (bytes_read < 0)
			return bytes_read;
		if (bytes_read != info->buff_used)
			return -EIO;
	} else if (!ret) {
		req->pdata = info->buff;
		info->phase = SCSIPH_STATUS;
		log_debug("sending buf\n");
	} else {
		log_debug("error\n");
		return -EIO;
	}

	return 0;
}

static int sandbox_scsi_bus_reset(struct udevice *dev)
{
	/* Not implemented */

	return 0;
}

static int sandbox_scsi_of_to_plat(struct udevice *dev)
{
	struct sandbox_scsi_priv *priv = dev_get_priv(dev);

	priv->pathname = dev_read_string(dev, "sandbox,filepath");

	return 0;
}

static int sandbox_scsi_probe(struct udevice *dev)
{
	struct scsi_plat *scsi_plat = dev_get_uclass_plat(dev);
	struct sandbox_scsi_priv *priv = dev_get_priv(dev);
	struct scsi_emul_info *info = &priv->eminfo;
	int ret;

	scsi_plat->max_id = 2;
	scsi_plat->max_lun = 3;
	scsi_plat->max_bytes_per_req = 1 << 20;

	info->vendor = "SANDBOX";
	info->product = "FAKE DISK";
	info->buff = malloc(SANDBOX_SCSI_BUF_SIZE);
	if (!info->buff)
		return log_ret(-ENOMEM);
	info->block_size = SANDBOX_SCSI_BLOCK_LEN;

	if (priv->pathname) {
		priv->fd = os_open(priv->pathname, OS_O_RDONLY);
		if (priv->fd != -1) {
			ret = os_get_filesize(priv->pathname, &info->file_size);
			if (ret)
				return log_msg_ret("sz", ret);
		}
	} else {
		priv->fd = -1;
	}
	log_debug("filename: %s, fd %d\n", priv->pathname, priv->fd);

	return 0;
}

static int sandbox_scsi_remove(struct udevice *dev)
{
	struct sandbox_scsi_priv *priv = dev_get_priv(dev);
	struct scsi_emul_info *info = &priv->eminfo;

	free(info->buff);

	return 0;
}

struct scsi_ops sandbox_scsi_ops = {
	.exec		= sandbox_scsi_exec,
	.bus_reset	= sandbox_scsi_bus_reset,
};

static const struct udevice_id sanbox_scsi_ids[] = {
	{ .compatible = "sandbox,scsi" },
	{ }
};

U_BOOT_DRIVER(sandbox_scsi) = {
	.name		= "sandbox_scsi",
	.id		= UCLASS_SCSI,
	.ops		= &sandbox_scsi_ops,
	.of_match	= sanbox_scsi_ids,
	.of_to_plat	= sandbox_scsi_of_to_plat,
	.probe		= sandbox_scsi_probe,
	.remove		= sandbox_scsi_remove,
	.priv_auto	= sizeof(struct sandbox_scsi_priv),
};
