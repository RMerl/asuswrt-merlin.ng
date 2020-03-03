#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/mtd/mtd.h>
#include <linux/platform_device.h>
#include <linux/bcma/bcma.h>

#include "bcm47xxsflash.h"

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Serial flash driver for BCMA bus");

static const char * const probes[] = { "bcm47xxpart", NULL };

/**************************************************
 * Various helpers
 **************************************************/

static void bcm47xxsflash_cmd(struct bcm47xxsflash *b47s, u32 opcode)
{
	int i;

	b47s->cc_write(b47s, BCMA_CC_FLASHCTL, BCMA_CC_FLASHCTL_START | opcode);
	for (i = 0; i < 1000; i++) {
		if (!(b47s->cc_read(b47s, BCMA_CC_FLASHCTL) &
		      BCMA_CC_FLASHCTL_BUSY))
			return;
		cpu_relax();
	}
	pr_err("Control command failed (timeout)!\n");
}

static int bcm47xxsflash_poll(struct bcm47xxsflash *b47s, int timeout)
{
	unsigned long deadline = jiffies + timeout;

	do {
		switch (b47s->type) {
		case BCM47XXSFLASH_TYPE_ST:
			bcm47xxsflash_cmd(b47s, OPCODE_ST_RDSR);
			if (!(b47s->cc_read(b47s, BCMA_CC_FLASHDATA) &
			      SR_ST_WIP))
				return 0;
			break;
		case BCM47XXSFLASH_TYPE_ATMEL:
			bcm47xxsflash_cmd(b47s, OPCODE_AT_STATUS);
			if (b47s->cc_read(b47s, BCMA_CC_FLASHDATA) &
			    SR_AT_READY)
				return 0;
			break;
		}

		cpu_relax();
		udelay(1);
	} while (!time_after_eq(jiffies, deadline));

	pr_err("Timeout waiting for flash to be ready!\n");

	return -EBUSY;
}

/**************************************************
 * MTD ops
 **************************************************/

static int bcm47xxsflash_erase(struct mtd_info *mtd, struct erase_info *erase)
{
	struct bcm47xxsflash *b47s = mtd->priv;
	int err;

	switch (b47s->type) {
	case BCM47XXSFLASH_TYPE_ST:
		bcm47xxsflash_cmd(b47s, OPCODE_ST_WREN);
		b47s->cc_write(b47s, BCMA_CC_FLASHADDR, erase->addr);
		/* Newer flashes have "sub-sectors" which can be erased
		 * independently with a new command: ST_SSE. The ST_SE command
		 * erases 64KB just as before.
		 */
		if (b47s->blocksize < (64 * 1024))
			bcm47xxsflash_cmd(b47s, OPCODE_ST_SSE);
		else
			bcm47xxsflash_cmd(b47s, OPCODE_ST_SE);
		break;
	case BCM47XXSFLASH_TYPE_ATMEL:
		b47s->cc_write(b47s, BCMA_CC_FLASHADDR, erase->addr << 1);
		bcm47xxsflash_cmd(b47s, OPCODE_AT_PAGE_ERASE);
		break;
	}

	err = bcm47xxsflash_poll(b47s, HZ);
	if (err)
		erase->state = MTD_ERASE_FAILED;
	else
		erase->state = MTD_ERASE_DONE;

	if (erase->callback)
		erase->callback(erase);

	return err;
}

static int bcm47xxsflash_read(struct mtd_info *mtd, loff_t from, size_t len,
			      size_t *retlen, u_char *buf)
{
	struct bcm47xxsflash *b47s = mtd->priv;

	/* Check address range */
	if ((from + len) > mtd->size)
		return -EINVAL;

	memcpy_fromio(buf, (void __iomem *)KSEG0ADDR(b47s->window + from),
		      len);
	*retlen = len;

	return len;
}

static int bcm47xxsflash_write_st(struct mtd_info *mtd, u32 offset, size_t len,
				  const u_char *buf)
{
	struct bcm47xxsflash *b47s = mtd->priv;
	int written = 0;

	/* Enable writes */
	bcm47xxsflash_cmd(b47s, OPCODE_ST_WREN);

	/* Write first byte */
	b47s->cc_write(b47s, BCMA_CC_FLASHADDR, offset);
	b47s->cc_write(b47s, BCMA_CC_FLASHDATA, *buf++);

	/* Program page */
	if (b47s->bcma_cc->core->id.rev < 20) {
		bcm47xxsflash_cmd(b47s, OPCODE_ST_PP);
		return 1; /* 1B written */
	}

	/* Program page and set CSA (on newer chips we can continue writing) */
	bcm47xxsflash_cmd(b47s, OPCODE_ST_CSA | OPCODE_ST_PP);
	offset++;
	len--;
	written++;

	while (len > 0) {
		/* Page boundary, another function call is needed */
		if ((offset & 0xFF) == 0)
			break;

		bcm47xxsflash_cmd(b47s, OPCODE_ST_CSA | *buf++);
		offset++;
		len--;
		written++;
	}

	/* All done, drop CSA & poll */
	b47s->cc_write(b47s, BCMA_CC_FLASHCTL, 0);
	udelay(1);
	if (bcm47xxsflash_poll(b47s, HZ / 10))
		pr_err("Flash rejected dropping CSA\n");

	return written;
}

static int bcm47xxsflash_write_at(struct mtd_info *mtd, u32 offset, size_t len,
				  const u_char *buf)
{
	struct bcm47xxsflash *b47s = mtd->priv;
	u32 mask = b47s->blocksize - 1;
	u32 page = (offset & ~mask) << 1;
	u32 byte = offset & mask;
	int written = 0;

	/* If we don't overwrite whole page, read it to the buffer first */
	if (byte || (len < b47s->blocksize)) {
		int err;

		b47s->cc_write(b47s, BCMA_CC_FLASHADDR, page);
		bcm47xxsflash_cmd(b47s, OPCODE_AT_BUF1_LOAD);
		/* 250 us for AT45DB321B */
		err = bcm47xxsflash_poll(b47s, HZ / 1000);
		if (err) {
			pr_err("Timeout reading page 0x%X info buffer\n", page);
			return err;
		}
	}

	/* Change buffer content with our data */
	while (len > 0) {
		/* Page boundary, another function call is needed */
		if (byte == b47s->blocksize)
			break;

		b47s->cc_write(b47s, BCMA_CC_FLASHADDR, byte++);
		b47s->cc_write(b47s, BCMA_CC_FLASHDATA, *buf++);
		bcm47xxsflash_cmd(b47s, OPCODE_AT_BUF1_WRITE);
		len--;
		written++;
	}

	/* Program page with the buffer content */
	b47s->cc_write(b47s, BCMA_CC_FLASHADDR, page);
	bcm47xxsflash_cmd(b47s, OPCODE_AT_BUF1_PROGRAM);

	return written;
}

static int bcm47xxsflash_write(struct mtd_info *mtd, loff_t to, size_t len,
			       size_t *retlen, const u_char *buf)
{
	struct bcm47xxsflash *b47s = mtd->priv;
	int written;

	/* Writing functions can return without writing all passed data, for
	 * example when the hardware is too old or when we git page boundary.
	 */
	while (len > 0) {
		switch (b47s->type) {
		case BCM47XXSFLASH_TYPE_ST:
			written = bcm47xxsflash_write_st(mtd, to, len, buf);
			break;
		case BCM47XXSFLASH_TYPE_ATMEL:
			written = bcm47xxsflash_write_at(mtd, to, len, buf);
			break;
		default:
			BUG_ON(1);
		}
		if (written < 0) {
			pr_err("Error writing at offset 0x%llX\n", to);
			return written;
		}
		to += (loff_t)written;
		len -= written;
		*retlen += written;
		buf += written;
	}

	return 0;
}

static void bcm47xxsflash_fill_mtd(struct bcm47xxsflash *b47s)
{
	struct mtd_info *mtd = &b47s->mtd;

	mtd->priv = b47s;
	mtd->name = "bcm47xxsflash";
	mtd->owner = THIS_MODULE;

	mtd->type = MTD_NORFLASH;
	mtd->flags = MTD_CAP_NORFLASH;
	mtd->size = b47s->size;
	mtd->erasesize = b47s->blocksize;
	mtd->writesize = 1;
	mtd->writebufsize = 1;

	mtd->_erase = bcm47xxsflash_erase;
	mtd->_read = bcm47xxsflash_read;
	mtd->_write = bcm47xxsflash_write;
}

/**************************************************
 * BCMA
 **************************************************/

static int bcm47xxsflash_bcma_cc_read(struct bcm47xxsflash *b47s, u16 offset)
{
	return bcma_cc_read32(b47s->bcma_cc, offset);
}

static void bcm47xxsflash_bcma_cc_write(struct bcm47xxsflash *b47s, u16 offset,
					u32 value)
{
	bcma_cc_write32(b47s->bcma_cc, offset, value);
}

static int bcm47xxsflash_bcma_probe(struct platform_device *pdev)
{
	struct bcma_sflash *sflash = dev_get_platdata(&pdev->dev);
	struct bcm47xxsflash *b47s;
	int err;

	b47s = devm_kzalloc(&pdev->dev, sizeof(*b47s), GFP_KERNEL);
	if (!b47s)
		return -ENOMEM;
	sflash->priv = b47s;

	b47s->bcma_cc = container_of(sflash, struct bcma_drv_cc, sflash);
	b47s->cc_read = bcm47xxsflash_bcma_cc_read;
	b47s->cc_write = bcm47xxsflash_bcma_cc_write;

	switch (b47s->bcma_cc->capabilities & BCMA_CC_CAP_FLASHT) {
	case BCMA_CC_FLASHT_STSER:
		b47s->type = BCM47XXSFLASH_TYPE_ST;
		break;
	case BCMA_CC_FLASHT_ATSER:
		b47s->type = BCM47XXSFLASH_TYPE_ATMEL;
		break;
	}

	b47s->window = sflash->window;
	b47s->blocksize = sflash->blocksize;
	b47s->numblocks = sflash->numblocks;
	b47s->size = sflash->size;
	bcm47xxsflash_fill_mtd(b47s);

	err = mtd_device_parse_register(&b47s->mtd, probes, NULL, NULL, 0);
	if (err) {
		pr_err("Failed to register MTD device: %d\n", err);
		return err;
	}

	if (bcm47xxsflash_poll(b47s, HZ / 10))
		pr_warn("Serial flash busy\n");

	return 0;
}

static int bcm47xxsflash_bcma_remove(struct platform_device *pdev)
{
	struct bcma_sflash *sflash = dev_get_platdata(&pdev->dev);
	struct bcm47xxsflash *b47s = sflash->priv;

	mtd_device_unregister(&b47s->mtd);

	return 0;
}

static struct platform_driver bcma_sflash_driver = {
	.probe	= bcm47xxsflash_bcma_probe,
	.remove = bcm47xxsflash_bcma_remove,
	.driver = {
		.name = "bcma_sflash",
	},
};

/**************************************************
 * Init
 **************************************************/

module_platform_driver(bcma_sflash_driver);
