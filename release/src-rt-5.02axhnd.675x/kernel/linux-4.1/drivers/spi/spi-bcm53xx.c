#define pr_fmt(fmt)		KBUILD_MODNAME ": " fmt

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/bcma/bcma.h>
#include <linux/spi/spi.h>

#include "spi-bcm53xx.h"

#define BCM53XXSPI_MAX_SPI_BAUD	13500000	/* 216 MHz? */

/* The longest observed required wait was 19 ms */
#define BCM53XXSPI_SPE_TIMEOUT_MS	80

struct bcm53xxspi {
	struct bcma_device *core;
	struct spi_master *master;

	size_t read_offset;
};

static inline u32 bcm53xxspi_read(struct bcm53xxspi *b53spi, u16 offset)
{
	return bcma_read32(b53spi->core, offset);
}

static inline void bcm53xxspi_write(struct bcm53xxspi *b53spi, u16 offset,
				    u32 value)
{
	bcma_write32(b53spi->core, offset, value);
}

static inline unsigned int bcm53xxspi_calc_timeout(size_t len)
{
	/* Do some magic calculation based on length and buad. Add 10% and 1. */
	return (len * 9000 / BCM53XXSPI_MAX_SPI_BAUD * 110 / 100) + 1;
}

static int bcm53xxspi_wait(struct bcm53xxspi *b53spi, unsigned int timeout_ms)
{
	unsigned long deadline;
	u32 tmp;

	/* SPE bit has to be 0 before we read MSPI STATUS */
	deadline = jiffies + msecs_to_jiffies(BCM53XXSPI_SPE_TIMEOUT_MS);
	do {
		tmp = bcm53xxspi_read(b53spi, B53SPI_MSPI_SPCR2);
		if (!(tmp & B53SPI_MSPI_SPCR2_SPE))
			break;
		udelay(5);
	} while (!time_after_eq(jiffies, deadline));

	if (tmp & B53SPI_MSPI_SPCR2_SPE)
		goto spi_timeout;

	/* Check status */
	deadline = jiffies + msecs_to_jiffies(timeout_ms);
	do {
		tmp = bcm53xxspi_read(b53spi, B53SPI_MSPI_MSPI_STATUS);
		if (tmp & B53SPI_MSPI_MSPI_STATUS_SPIF) {
			bcm53xxspi_write(b53spi, B53SPI_MSPI_MSPI_STATUS, 0);
			return 0;
		}

		cpu_relax();
		udelay(100);
	} while (!time_after_eq(jiffies, deadline));

spi_timeout:
	bcm53xxspi_write(b53spi, B53SPI_MSPI_MSPI_STATUS, 0);

	pr_err("Timeout waiting for SPI to be ready!\n");

	return -EBUSY;
}

static void bcm53xxspi_buf_write(struct bcm53xxspi *b53spi, u8 *w_buf,
				 size_t len, bool cont)
{
	u32 tmp;
	int i;

	for (i = 0; i < len; i++) {
		/* Transmit Register File MSB */
		bcm53xxspi_write(b53spi, B53SPI_MSPI_TXRAM + 4 * (i * 2),
				 (unsigned int)w_buf[i]);
	}

	for (i = 0; i < len; i++) {
		tmp = B53SPI_CDRAM_CONT | B53SPI_CDRAM_PCS_DISABLE_ALL |
		      B53SPI_CDRAM_PCS_DSCK;
		if (!cont && i == len - 1)
			tmp &= ~B53SPI_CDRAM_CONT;
		tmp &= ~0x1;
		/* Command Register File */
		bcm53xxspi_write(b53spi, B53SPI_MSPI_CDRAM + 4 * i, tmp);
	}

	/* Set queue pointers */
	bcm53xxspi_write(b53spi, B53SPI_MSPI_NEWQP, 0);
	bcm53xxspi_write(b53spi, B53SPI_MSPI_ENDQP, len - 1);

	if (cont)
		bcm53xxspi_write(b53spi, B53SPI_MSPI_WRITE_LOCK, 1);

	/* Start SPI transfer */
	tmp = bcm53xxspi_read(b53spi, B53SPI_MSPI_SPCR2);
	tmp |= B53SPI_MSPI_SPCR2_SPE;
	if (cont)
		tmp |= B53SPI_MSPI_SPCR2_CONT_AFTER_CMD;
	bcm53xxspi_write(b53spi, B53SPI_MSPI_SPCR2, tmp);

	/* Wait for SPI to finish */
	bcm53xxspi_wait(b53spi, bcm53xxspi_calc_timeout(len));

	if (!cont)
		bcm53xxspi_write(b53spi, B53SPI_MSPI_WRITE_LOCK, 0);

	b53spi->read_offset = len;
}

static void bcm53xxspi_buf_read(struct bcm53xxspi *b53spi, u8 *r_buf,
				size_t len, bool cont)
{
	u32 tmp;
	int i;

	for (i = 0; i < b53spi->read_offset + len; i++) {
		tmp = B53SPI_CDRAM_CONT | B53SPI_CDRAM_PCS_DISABLE_ALL |
		      B53SPI_CDRAM_PCS_DSCK;
		if (!cont && i == b53spi->read_offset + len - 1)
			tmp &= ~B53SPI_CDRAM_CONT;
		tmp &= ~0x1;
		/* Command Register File */
		bcm53xxspi_write(b53spi, B53SPI_MSPI_CDRAM + 4 * i, tmp);
	}

	/* Set queue pointers */
	bcm53xxspi_write(b53spi, B53SPI_MSPI_NEWQP, 0);
	bcm53xxspi_write(b53spi, B53SPI_MSPI_ENDQP,
			 b53spi->read_offset + len - 1);

	if (cont)
		bcm53xxspi_write(b53spi, B53SPI_MSPI_WRITE_LOCK, 1);

	/* Start SPI transfer */
	tmp = bcm53xxspi_read(b53spi, B53SPI_MSPI_SPCR2);
	tmp |= B53SPI_MSPI_SPCR2_SPE;
	if (cont)
		tmp |= B53SPI_MSPI_SPCR2_CONT_AFTER_CMD;
	bcm53xxspi_write(b53spi, B53SPI_MSPI_SPCR2, tmp);

	/* Wait for SPI to finish */
	bcm53xxspi_wait(b53spi, bcm53xxspi_calc_timeout(len));

	if (!cont)
		bcm53xxspi_write(b53spi, B53SPI_MSPI_WRITE_LOCK, 0);

	for (i = 0; i < len; ++i) {
		int offset = b53spi->read_offset + i;

		/* Data stored in the transmit register file LSB */
		r_buf[i] = (u8)bcm53xxspi_read(b53spi, B53SPI_MSPI_RXRAM + 4 * (1 + offset * 2));
	}

	b53spi->read_offset = 0;
}

static int bcm53xxspi_transfer_one(struct spi_master *master,
				   struct spi_device *spi,
				   struct spi_transfer *t)
{
	struct bcm53xxspi *b53spi = spi_master_get_devdata(master);
	u8 *buf;
	size_t left;

	if (t->tx_buf) {
		buf = (u8 *)t->tx_buf;
		left = t->len;
		while (left) {
			size_t to_write = min_t(size_t, 16, left);
			bool cont = left - to_write > 0;

			bcm53xxspi_buf_write(b53spi, buf, to_write, cont);
			left -= to_write;
			buf += to_write;
		}
	}

	if (t->rx_buf) {
		buf = (u8 *)t->rx_buf;
		left = t->len;
		while (left) {
			size_t to_read = min_t(size_t, 16 - b53spi->read_offset,
					       left);
			bool cont = left - to_read > 0;

			bcm53xxspi_buf_read(b53spi, buf, to_read, cont);
			left -= to_read;
			buf += to_read;
		}
	}

	return 0;
}

/**************************************************
 * BCMA
 **************************************************/

static struct spi_board_info bcm53xx_info = {
	.modalias	= "bcm53xxspiflash",
};

static const struct bcma_device_id bcm53xxspi_bcma_tbl[] = {
	BCMA_CORE(BCMA_MANUF_BCM, BCMA_CORE_NS_QSPI, BCMA_ANY_REV, BCMA_ANY_CLASS),
	{},
};
MODULE_DEVICE_TABLE(bcma, bcm53xxspi_bcma_tbl);

static int bcm53xxspi_bcma_probe(struct bcma_device *core)
{
	struct bcm53xxspi *b53spi;
	struct spi_master *master;
	int err;

	if (core->bus->drv_cc.core->id.rev != 42) {
		pr_err("SPI on SoC with unsupported ChipCommon rev\n");
		return -ENOTSUPP;
	}

	master = spi_alloc_master(&core->dev, sizeof(*b53spi));
	if (!master)
		return -ENOMEM;

	b53spi = spi_master_get_devdata(master);
	b53spi->master = master;
	b53spi->core = core;

	master->transfer_one = bcm53xxspi_transfer_one;

	bcma_set_drvdata(core, b53spi);

	err = devm_spi_register_master(&core->dev, master);
	if (err) {
		spi_master_put(master);
		bcma_set_drvdata(core, NULL);
		goto out;
	}

	/* Broadcom SoCs (at least with the CC rev 42) use SPI for flash only */
	spi_new_device(master, &bcm53xx_info);

out:
	return err;
}

static void bcm53xxspi_bcma_remove(struct bcma_device *core)
{
	struct bcm53xxspi *b53spi = bcma_get_drvdata(core);

	spi_unregister_master(b53spi->master);
}

static struct bcma_driver bcm53xxspi_bcma_driver = {
	.name		= KBUILD_MODNAME,
	.id_table	= bcm53xxspi_bcma_tbl,
	.probe		= bcm53xxspi_bcma_probe,
	.remove		= bcm53xxspi_bcma_remove,
};

/**************************************************
 * Init & exit
 **************************************************/

static int __init bcm53xxspi_module_init(void)
{
	int err = 0;

	err = bcma_driver_register(&bcm53xxspi_bcma_driver);
	if (err)
		pr_err("Failed to register bcma driver: %d\n", err);

	return err;
}

static void __exit bcm53xxspi_module_exit(void)
{
	bcma_driver_unregister(&bcm53xxspi_bcma_driver);
}

module_init(bcm53xxspi_module_init);
module_exit(bcm53xxspi_module_exit);

MODULE_DESCRIPTION("Broadcom BCM53xx SPI Controller driver");
MODULE_AUTHOR("Rafał Miłecki <zajec5@gmail.com>");
MODULE_LICENSE("GPL");
