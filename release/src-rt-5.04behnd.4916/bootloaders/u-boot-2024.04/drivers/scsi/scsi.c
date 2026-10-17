// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2001
 * Denis Peter, MPL AG Switzerland
 */

#define LOG_CATEGORY	UCLASS_SCSI

#include <common.h>
#include <blk.h>
#include <bootdev.h>
#include <bootstage.h>
#include <dm.h>
#include <env.h>
#include <libata.h>
#include <log.h>
#include <memalign.h>
#include <part.h>
#include <pci.h>
#include <scsi.h>
#include <dm/device-internal.h>
#include <dm/uclass-internal.h>

static struct scsi_cmd tempccb;	/* temporary scsi command buffer */

DEFINE_CACHE_ALIGN_BUFFER(u8, tempbuff, 512);	/* temporary data buffer */

/* almost the maximum amount of the scsi_ext command.. */
#define SCSI_MAX_BLK 0xFFFF
#define SCSI_LBA48_READ	0xFFFFFFF

static void scsi_print_error(struct scsi_cmd *pccb)
{
	/* Dummy function that could print an error for debugging */
}

#ifdef CONFIG_SYS_64BIT_LBA
void scsi_setup_read16(struct scsi_cmd *pccb, lbaint_t start,
		       unsigned long blocks)
{
	pccb->cmd[0] = SCSI_READ16;
	pccb->cmd[1] = pccb->lun << 5;
	pccb->cmd[2] = (unsigned char)(start >> 56) & 0xff;
	pccb->cmd[3] = (unsigned char)(start >> 48) & 0xff;
	pccb->cmd[4] = (unsigned char)(start >> 40) & 0xff;
	pccb->cmd[5] = (unsigned char)(start >> 32) & 0xff;
	pccb->cmd[6] = (unsigned char)(start >> 24) & 0xff;
	pccb->cmd[7] = (unsigned char)(start >> 16) & 0xff;
	pccb->cmd[8] = (unsigned char)(start >> 8) & 0xff;
	pccb->cmd[9] = (unsigned char)start & 0xff;
	pccb->cmd[10] = 0;
	pccb->cmd[11] = (unsigned char)(blocks >> 24) & 0xff;
	pccb->cmd[12] = (unsigned char)(blocks >> 16) & 0xff;
	pccb->cmd[13] = (unsigned char)(blocks >> 8) & 0xff;
	pccb->cmd[14] = (unsigned char)blocks & 0xff;
	pccb->cmd[15] = 0;
	pccb->cmdlen = 16;
	pccb->msgout[0] = SCSI_IDENTIFY; /* NOT USED */
	debug("scsi_setup_read16: cmd: %02X %02X startblk %02X%02X%02X%02X%02X%02X%02X%02X blccnt %02X%02X%02X%02X\n",
	      pccb->cmd[0], pccb->cmd[1],
	      pccb->cmd[2], pccb->cmd[3], pccb->cmd[4], pccb->cmd[5],
	      pccb->cmd[6], pccb->cmd[7], pccb->cmd[8], pccb->cmd[9],
	      pccb->cmd[11], pccb->cmd[12], pccb->cmd[13], pccb->cmd[14]);
}
#endif

static void scsi_setup_inquiry(struct scsi_cmd *pccb)
{
	pccb->cmd[0] = SCSI_INQUIRY;
	pccb->cmd[1] = pccb->lun << 5;
	pccb->cmd[2] = 0;
	pccb->cmd[3] = 0;
	if (pccb->datalen > 255)
		pccb->cmd[4] = 255;
	else
		pccb->cmd[4] = (unsigned char)pccb->datalen;
	pccb->cmd[5] = 0;
	pccb->cmdlen = 6;
	pccb->msgout[0] = SCSI_IDENTIFY; /* NOT USED */
}

static void scsi_setup_read_ext(struct scsi_cmd *pccb, lbaint_t start,
				unsigned short blocks)
{
	pccb->cmd[0] = SCSI_READ10;
	pccb->cmd[1] = pccb->lun << 5;
	pccb->cmd[2] = (unsigned char)(start >> 24) & 0xff;
	pccb->cmd[3] = (unsigned char)(start >> 16) & 0xff;
	pccb->cmd[4] = (unsigned char)(start >> 8) & 0xff;
	pccb->cmd[5] = (unsigned char)start & 0xff;
	pccb->cmd[6] = 0;
	pccb->cmd[7] = (unsigned char)(blocks >> 8) & 0xff;
	pccb->cmd[8] = (unsigned char)blocks & 0xff;
	pccb->cmd[6] = 0;
	pccb->cmdlen = 10;
	pccb->msgout[0] = SCSI_IDENTIFY; /* NOT USED */
	debug("scsi_setup_read_ext: cmd: %02X %02X startblk %02X%02X%02X%02X blccnt %02X%02X\n",
	      pccb->cmd[0], pccb->cmd[1],
	      pccb->cmd[2], pccb->cmd[3], pccb->cmd[4], pccb->cmd[5],
	      pccb->cmd[7], pccb->cmd[8]);
}

static void scsi_setup_write_ext(struct scsi_cmd *pccb, lbaint_t start,
				 unsigned short blocks)
{
	pccb->cmd[0] = SCSI_WRITE10;
	pccb->cmd[1] = pccb->lun << 5;
	pccb->cmd[2] = (unsigned char)(start >> 24) & 0xff;
	pccb->cmd[3] = (unsigned char)(start >> 16) & 0xff;
	pccb->cmd[4] = (unsigned char)(start >> 8) & 0xff;
	pccb->cmd[5] = (unsigned char)start & 0xff;
	pccb->cmd[6] = 0;
	pccb->cmd[7] = ((unsigned char)(blocks >> 8)) & 0xff;
	pccb->cmd[8] = (unsigned char)blocks & 0xff;
	pccb->cmd[9] = 0;
	pccb->cmdlen = 10;
	pccb->msgout[0] = SCSI_IDENTIFY;  /* NOT USED */
	debug("%s: cmd: %02X %02X startblk %02X%02X%02X%02X blccnt %02X%02X\n",
	      __func__,
	      pccb->cmd[0], pccb->cmd[1],
	      pccb->cmd[2], pccb->cmd[3], pccb->cmd[4], pccb->cmd[5],
	      pccb->cmd[7], pccb->cmd[8]);
}

static ulong scsi_read(struct udevice *dev, lbaint_t blknr, lbaint_t blkcnt,
		       void *buffer)
{
	struct blk_desc *block_dev = dev_get_uclass_plat(dev);
	struct udevice *bdev = dev->parent;
	struct scsi_plat *uc_plat = dev_get_uclass_plat(bdev);
	lbaint_t start, blks, max_blks;
	uintptr_t buf_addr;
	unsigned short smallblks = 0;
	struct scsi_cmd *pccb = (struct scsi_cmd *)&tempccb;

	/* Setup device */
	pccb->target = block_dev->target;
	pccb->lun = block_dev->lun;
	buf_addr = (unsigned long)buffer;
	start = blknr;
	blks = blkcnt;
	if (uc_plat->max_bytes_per_req)
		max_blks = uc_plat->max_bytes_per_req / block_dev->blksz;
	else
		max_blks = SCSI_MAX_BLK;

	debug("\nscsi_read: dev %d startblk " LBAF
	      ", blccnt " LBAF " buffer %lx\n",
	      block_dev->devnum, start, blks, (unsigned long)buffer);
	do {
		pccb->pdata = (unsigned char *)buf_addr;
		pccb->dma_dir = DMA_FROM_DEVICE;
#ifdef CONFIG_SYS_64BIT_LBA
		if (start > SCSI_LBA48_READ) {
			unsigned long blocks;
			blocks = min_t(lbaint_t, blks, max_blks);
			pccb->datalen = block_dev->blksz * blocks;
			scsi_setup_read16(pccb, start, blocks);
			start += blocks;
			blks -= blocks;
		} else
#endif
		if (blks > max_blks) {
			pccb->datalen = block_dev->blksz * max_blks;
			smallblks = max_blks;
			scsi_setup_read_ext(pccb, start, smallblks);
			start += max_blks;
			blks -= max_blks;
		} else {
			pccb->datalen = block_dev->blksz * blks;
			smallblks = (unsigned short)blks;
			scsi_setup_read_ext(pccb, start, smallblks);
			start += blks;
			blks = 0;
		}
		debug("scsi_read_ext: startblk " LBAF
		      ", blccnt %x buffer %lX\n",
		      start, smallblks, buf_addr);
		if (scsi_exec(bdev, pccb)) {
			scsi_print_error(pccb);
			blkcnt -= blks;
			break;
		}
		buf_addr += pccb->datalen;
	} while (blks != 0);
	debug("scsi_read_ext: end startblk " LBAF
	      ", blccnt %x buffer %lX\n", start, smallblks, buf_addr);
	return blkcnt;
}

/*******************************************************************************
 * scsi_write
 */

static ulong scsi_write(struct udevice *dev, lbaint_t blknr, lbaint_t blkcnt,
			const void *buffer)
{
	struct blk_desc *block_dev = dev_get_uclass_plat(dev);
	struct udevice *bdev = dev->parent;
	struct scsi_plat *uc_plat = dev_get_uclass_plat(bdev);
	lbaint_t start, blks, max_blks;
	uintptr_t buf_addr;
	unsigned short smallblks;
	struct scsi_cmd *pccb = (struct scsi_cmd *)&tempccb;

	/* Setup device */
	pccb->target = block_dev->target;
	pccb->lun = block_dev->lun;
	buf_addr = (unsigned long)buffer;
	start = blknr;
	blks = blkcnt;
	if (uc_plat->max_bytes_per_req)
		max_blks = uc_plat->max_bytes_per_req / block_dev->blksz;
	else
		max_blks = SCSI_MAX_BLK;

	debug("\n%s: dev %d startblk " LBAF ", blccnt " LBAF " buffer %lx\n",
	      __func__, block_dev->devnum, start, blks, (unsigned long)buffer);
	do {
		pccb->pdata = (unsigned char *)buf_addr;
		pccb->dma_dir = DMA_TO_DEVICE;
		if (blks > max_blks) {
			pccb->datalen = block_dev->blksz * max_blks;
			smallblks = max_blks;
			scsi_setup_write_ext(pccb, start, smallblks);
			start += max_blks;
			blks -= max_blks;
		} else {
			pccb->datalen = block_dev->blksz * blks;
			smallblks = (unsigned short)blks;
			scsi_setup_write_ext(pccb, start, smallblks);
			start += blks;
			blks = 0;
		}
		debug("%s: startblk " LBAF ", blccnt %x buffer %lx\n",
		      __func__, start, smallblks, buf_addr);
		if (scsi_exec(bdev, pccb)) {
			scsi_print_error(pccb);
			blkcnt -= blks;
			break;
		}
		buf_addr += pccb->datalen;
	} while (blks != 0);
	debug("%s: end startblk " LBAF ", blccnt %x buffer %lX\n",
	      __func__, start, smallblks, buf_addr);
	return blkcnt;
}

#if IS_ENABLED(CONFIG_BOUNCE_BUFFER)
static int scsi_buffer_aligned(struct udevice *dev, struct bounce_buffer *state)
{
	struct scsi_ops *ops = scsi_get_ops(dev->parent);

	if (ops->buffer_aligned)
		return ops->buffer_aligned(dev->parent, state);

	return 1;
}
#endif	/* CONFIG_BOUNCE_BUFFER */

/* copy src to dest, skipping leading and trailing blanks
 * and null terminate the string
 */
static void scsi_ident_cpy(unsigned char *dest, unsigned char *src,
			   unsigned int len)
{
	int start, end;

	start = 0;
	while (start < len) {
		if (src[start] != ' ')
			break;
		start++;
	}
	end = len-1;
	while (end > start) {
		if (src[end] != ' ')
			break;
		end--;
	}
	for (; start <= end; start++)
		*dest ++= src[start];
	*dest = '\0';
}

static int scsi_read_capacity(struct udevice *dev, struct scsi_cmd *pccb,
			      lbaint_t *capacity, unsigned long *blksz)
{
	*capacity = 0;

	memset(pccb->cmd, '\0', sizeof(pccb->cmd));
	pccb->cmd[0] = SCSI_RD_CAPAC10;
	pccb->cmd[1] = pccb->lun << 5;
	pccb->cmdlen = 10;
	pccb->dma_dir = DMA_FROM_DEVICE;
	pccb->msgout[0] = SCSI_IDENTIFY; /* NOT USED */

	pccb->datalen = 8;
	if (scsi_exec(dev, pccb))
		return 1;

	*capacity = ((lbaint_t)pccb->pdata[0] << 24) |
		    ((lbaint_t)pccb->pdata[1] << 16) |
		    ((lbaint_t)pccb->pdata[2] << 8)  |
		    ((lbaint_t)pccb->pdata[3]);

	if (*capacity != 0xffffffff) {
		/* Read capacity (10) was sufficient for this drive. */
		*blksz = ((unsigned long)pccb->pdata[4] << 24) |
			 ((unsigned long)pccb->pdata[5] << 16) |
			 ((unsigned long)pccb->pdata[6] << 8)  |
			 ((unsigned long)pccb->pdata[7]);
		return 0;
	}

	/* Read capacity (10) was insufficient. Use read capacity (16). */
	memset(pccb->cmd, '\0', sizeof(pccb->cmd));
	pccb->cmd[0] = SCSI_RD_CAPAC16;
	pccb->cmd[1] = 0x10;
	pccb->cmdlen = 16;
	pccb->msgout[0] = SCSI_IDENTIFY; /* NOT USED */

	pccb->datalen = 16;
	pccb->dma_dir = DMA_FROM_DEVICE;
	if (scsi_exec(dev, pccb))
		return 1;

	*capacity = ((uint64_t)pccb->pdata[0] << 56) |
		    ((uint64_t)pccb->pdata[1] << 48) |
		    ((uint64_t)pccb->pdata[2] << 40) |
		    ((uint64_t)pccb->pdata[3] << 32) |
		    ((uint64_t)pccb->pdata[4] << 24) |
		    ((uint64_t)pccb->pdata[5] << 16) |
		    ((uint64_t)pccb->pdata[6] << 8)  |
		    ((uint64_t)pccb->pdata[7]);

	*blksz = ((uint64_t)pccb->pdata[8]  << 56) |
		 ((uint64_t)pccb->pdata[9]  << 48) |
		 ((uint64_t)pccb->pdata[10] << 40) |
		 ((uint64_t)pccb->pdata[11] << 32) |
		 ((uint64_t)pccb->pdata[12] << 24) |
		 ((uint64_t)pccb->pdata[13] << 16) |
		 ((uint64_t)pccb->pdata[14] << 8)  |
		 ((uint64_t)pccb->pdata[15]);

	return 0;
}


/*
 * Some setup (fill-in) routines
 */
static void scsi_setup_test_unit_ready(struct scsi_cmd *pccb)
{
	pccb->cmd[0] = SCSI_TST_U_RDY;
	pccb->cmd[1] = pccb->lun << 5;
	pccb->cmd[2] = 0;
	pccb->cmd[3] = 0;
	pccb->cmd[4] = 0;
	pccb->cmd[5] = 0;
	pccb->cmdlen = 6;
	pccb->msgout[0] = SCSI_IDENTIFY; /* NOT USED */
}

/**
 * scsi_init_dev_desc_priv - initialize only SCSI specific blk_desc properties
 *
 * @dev_desc: Block device description pointer
 */
static void scsi_init_dev_desc_priv(struct blk_desc *dev_desc)
{
	memset(dev_desc, 0, sizeof(struct blk_desc));
	dev_desc->target = 0xff;
	dev_desc->lun = 0xff;
	dev_desc->log2blksz =
		LOG2_INVALID(typeof(dev_desc->log2blksz));
	dev_desc->type = DEV_TYPE_UNKNOWN;
#if IS_ENABLED(CONFIG_BOUNCE_BUFFER)
	dev_desc->bb = true;
#endif	/* CONFIG_BOUNCE_BUFFER */
}

/**
 * scsi_detect_dev - Detect scsi device
 *
 * @target: target id
 * @lun: target lun
 * @dev_desc: block device description
 *
 * The scsi_detect_dev detects and fills a dev_desc structure when the device is
 * detected.
 *
 * Return: 0 on success, error value otherwise
 */
static int scsi_detect_dev(struct udevice *dev, int target, int lun,
			   struct blk_desc *dev_desc)
{
	unsigned char perq, modi;
	lbaint_t capacity;
	unsigned long blksz;
	struct scsi_cmd *pccb = (struct scsi_cmd *)&tempccb;
	int count, err;

	pccb->target = target;
	pccb->lun = lun;
	pccb->pdata = tempbuff;
	pccb->datalen = 512;
	pccb->dma_dir = DMA_FROM_DEVICE;
	scsi_setup_inquiry(pccb);
	if (scsi_exec(dev, pccb)) {
		if (pccb->contr_stat == SCSI_SEL_TIME_OUT) {
			/*
			  * selection timeout => assuming no
			  * device present
			  */
			debug("Selection timeout ID %d\n",
			      pccb->target);
			return -ETIMEDOUT;
		}
		scsi_print_error(pccb);
		return -ENODEV;
	}
	perq = tempbuff[0];
	modi = tempbuff[1];
	if ((perq & 0x1f) == 0x1f)
		return -ENODEV; /* skip unknown devices */
	if ((modi & 0x80) == 0x80) /* drive is removable */
		dev_desc->removable = true;
	/* get info for this device */
	scsi_ident_cpy((unsigned char *)dev_desc->vendor,
		       &tempbuff[8], 8);
	scsi_ident_cpy((unsigned char *)dev_desc->product,
		       &tempbuff[16], 16);
	scsi_ident_cpy((unsigned char *)dev_desc->revision,
		       &tempbuff[32], 4);
	dev_desc->target = pccb->target;
	dev_desc->lun = pccb->lun;

	for (count = 0; count < 3; count++) {
		pccb->datalen = 0;
		pccb->dma_dir = DMA_NONE;
		scsi_setup_test_unit_ready(pccb);
		err = scsi_exec(dev, pccb);
		if (!err)
			break;
	}
	if (err) {
		if (dev_desc->removable) {
			dev_desc->type = perq;
			goto removable;
		}
		scsi_print_error(pccb);
		return -EINVAL;
	}
	if (scsi_read_capacity(dev, pccb, &capacity, &blksz)) {
		scsi_print_error(pccb);
		return -EINVAL;
	}
	dev_desc->lba = capacity;
	dev_desc->blksz = blksz;
	dev_desc->log2blksz = LOG2(dev_desc->blksz);
	dev_desc->type = perq;
removable:
	return 0;
}

/*
 * (re)-scan the scsi bus and reports scsi device info
 * to the user if mode = 1
 */
static int do_scsi_scan_one(struct udevice *dev, int id, int lun, bool verbose)
{
	int ret;
	struct udevice *bdev;
	struct blk_desc bd;
	struct blk_desc *bdesc;
	char str[10], *name;

	/*
	 * detect the scsi driver to get information about its geometry (block
	 * size, number of blocks) and other parameters (ids, type, ...)
	 */
	scsi_init_dev_desc_priv(&bd);
	if (scsi_detect_dev(dev, id, lun, &bd))
		return -ENODEV;

	/*
	* Create only one block device and do detection
	* to make sure that there won't be a lot of
	* block devices created
	*/
	snprintf(str, sizeof(str), "id%dlun%d", id, lun);
	name = strdup(str);
	if (!name)
		return log_msg_ret("nam", -ENOMEM);
	ret = blk_create_devicef(dev, "scsi_blk", name, UCLASS_SCSI, -1,
				 bd.blksz, bd.lba, &bdev);
	if (ret) {
		debug("Can't create device\n");
		return ret;
	}
	device_set_name_alloced(bdev);

	bdesc = dev_get_uclass_plat(bdev);
	bdesc->target = id;
	bdesc->lun = lun;
	bdesc->removable = bd.removable;
	bdesc->type = bd.type;
	bdesc->bb = bd.bb;
	memcpy(&bdesc->vendor, &bd.vendor, sizeof(bd.vendor));
	memcpy(&bdesc->product, &bd.product, sizeof(bd.product));
	memcpy(&bdesc->revision, &bd.revision,	sizeof(bd.revision));
	if (IS_ENABLED(CONFIG_SYS_BIG_ENDIAN)) {
		ata_swap_buf_le16((u16 *)&bdesc->vendor, sizeof(bd.vendor) / 2);
		ata_swap_buf_le16((u16 *)&bdesc->product, sizeof(bd.product) / 2);
		ata_swap_buf_le16((u16 *)&bdesc->revision, sizeof(bd.revision) / 2);
	}

	ret = blk_probe_or_unbind(bdev);
	if (ret < 0)
		/* TODO: undo create */
		return log_msg_ret("pro", ret);

	ret = bootdev_setup_for_sibling_blk(bdev, "scsi_bootdev");
	if (ret)
		return log_msg_ret("bd", ret);

	if (verbose) {
		printf("  Device %d: ", bdesc->devnum);
		dev_print(bdesc);
	}
	return 0;
}

int scsi_scan_dev(struct udevice *dev, bool verbose)
{
	struct scsi_plat *uc_plat; /* scsi controller plat */
	int ret;
	int i;
	int lun;

	/* probe SCSI controller driver */
	ret = device_probe(dev);
	if (ret)
		return ret;

	/* Get controller plat */
	uc_plat = dev_get_uclass_plat(dev);

	for (i = 0; i < uc_plat->max_id; i++)
		for (lun = 0; lun < uc_plat->max_lun; lun++)
			do_scsi_scan_one(dev, i, lun, verbose);

	return 0;
}

int scsi_scan(bool verbose)
{
	struct uclass *uc;
	struct udevice *dev; /* SCSI controller */
	int ret;

	if (verbose)
		printf("scanning bus for devices...\n");

	ret = uclass_get(UCLASS_SCSI, &uc);
	if (ret)
		return ret;

	/* remove all children of the SCSI devices */
	uclass_foreach_dev(dev, uc) {
		log_debug("unbind %s\n", dev->name);
		ret = device_chld_remove(dev, NULL, DM_REMOVE_NORMAL);
		if (!ret)
			ret = device_chld_unbind(dev, NULL);
		if (ret) {
			if (verbose)
				printf("unable to unbind devices (%dE)\n", ret);
			return log_msg_ret("unb", ret);
		}
	}

	uclass_foreach_dev(dev, uc) {
		ret = scsi_scan_dev(dev, verbose);
		if (ret)
			return ret;
	}

	return 0;
}

static const struct blk_ops scsi_blk_ops = {
	.read	= scsi_read,
	.write	= scsi_write,
#if IS_ENABLED(CONFIG_BOUNCE_BUFFER)
	.buffer_aligned	= scsi_buffer_aligned,
#endif	/* CONFIG_BOUNCE_BUFFER */
};

U_BOOT_DRIVER(scsi_blk) = {
	.name		= "scsi_blk",
	.id		= UCLASS_BLK,
	.ops		= &scsi_blk_ops,
};
