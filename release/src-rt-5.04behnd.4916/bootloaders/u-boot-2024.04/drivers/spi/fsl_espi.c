// SPDX-License-Identifier: GPL-2.0+
/*
 * eSPI controller driver.
 *
 * Copyright 2010-2011 Freescale Semiconductor, Inc.
 * Copyright 2020 NXP
 * Author: Mingkai Hu (Mingkai.hu@freescale.com)
 *	   Chuanhua Han (chuanhua.han@nxp.com)
 */

#include <common.h>
#include <log.h>
#include <linux/bitops.h>
#include <linux/delay.h>

#include <malloc.h>
#include <spi.h>
#include <asm/global_data.h>
#include <asm/immap_85xx.h>
#include <dm.h>
#include <errno.h>
#include <fdtdec.h>
#include <dm/platform_data/fsl_espi.h>

struct fsl_spi_slave {
	struct spi_slave slave;
	ccsr_espi_t	*espi;
	u32		speed_hz;
	unsigned int	cs;
	unsigned int	div16;
	unsigned int	pm;
	int		tx_timeout;
	unsigned int	mode;
	size_t		cmd_len;
	u8		cmd_buf[16];
	size_t		data_len;
	unsigned int    max_transfer_length;
};

#define to_fsl_spi_slave(s) container_of(s, struct fsl_spi_slave, slave)
#define US_PER_SECOND		1000000UL

/* default SCK frequency, unit: HZ */
#define FSL_ESPI_DEFAULT_SCK_FREQ   10000000

#define ESPI_MAX_CS_NUM		4
#define ESPI_FIFO_WIDTH_BIT	32

#define ESPI_EV_RNE		BIT(9)
#define ESPI_EV_TNF		BIT(8)
#define ESPI_EV_DON		BIT(14)
#define ESPI_EV_TXE		BIT(15)
#define ESPI_EV_RFCNT_SHIFT	24
#define ESPI_EV_RFCNT_MASK	(0x3f << ESPI_EV_RFCNT_SHIFT)

#define ESPI_MODE_EN		BIT(31)	/* Enable interface */
#define ESPI_MODE_TXTHR(x)	((x) << 8)	/* Tx FIFO threshold */
#define ESPI_MODE_RXTHR(x)	((x) << 0)	/* Rx FIFO threshold */

#define ESPI_COM_CS(x)		((x) << 30)
#define ESPI_COM_TRANLEN(x)	((x) << 0)

#define ESPI_CSMODE_CI_INACTIVEHIGH	BIT(31)
#define ESPI_CSMODE_CP_BEGIN_EDGCLK	BIT(30)
#define ESPI_CSMODE_REV_MSB_FIRST	BIT(29)
#define ESPI_CSMODE_DIV16		BIT(28)
#define ESPI_CSMODE_PM(x)		((x) << 24)
#define ESPI_CSMODE_POL_ASSERTED_LOW	BIT(20)
#define ESPI_CSMODE_LEN(x)		((x) << 16)
#define ESPI_CSMODE_CSBEF(x)		((x) << 12)
#define ESPI_CSMODE_CSAFT(x)		((x) << 8)
#define ESPI_CSMODE_CSCG(x)		((x) << 3)

#define ESPI_CSMODE_INIT_VAL (ESPI_CSMODE_POL_ASSERTED_LOW | \
		ESPI_CSMODE_CSBEF(0) | ESPI_CSMODE_CSAFT(0) | \
		ESPI_CSMODE_CSCG(1))

#define ESPI_MAX_DATA_TRANSFER_LEN 0xFFF0

void fsl_spi_cs_activate(struct spi_slave *slave, uint cs)
{
	struct fsl_spi_slave *fsl = to_fsl_spi_slave(slave);
	ccsr_espi_t *espi = fsl->espi;
	unsigned int com = 0;
	size_t data_len = fsl->data_len;

	com &= ~(ESPI_COM_CS(0x3) | ESPI_COM_TRANLEN(0xFFFF));
	com |= ESPI_COM_CS(cs);
	com |= ESPI_COM_TRANLEN(data_len - 1);
	out_be32(&espi->com, com);
}

void fsl_spi_cs_deactivate(struct spi_slave *slave)
{
	struct fsl_spi_slave *fsl = to_fsl_spi_slave(slave);
	ccsr_espi_t *espi = fsl->espi;

	/* clear the RXCNT and TXCNT */
	out_be32(&espi->mode, in_be32(&espi->mode) & (~ESPI_MODE_EN));
	out_be32(&espi->mode, in_be32(&espi->mode) | ESPI_MODE_EN);
}

static void fsl_espi_tx(struct fsl_spi_slave *fsl, const void *dout)
{
	ccsr_espi_t *espi = fsl->espi;
	unsigned int tmpdout, event;
	int tmp_tx_timeout;

	if (dout)
		tmpdout = *(u32 *)dout;
	else
		tmpdout = 0;

	out_be32(&espi->tx, tmpdout);
	out_be32(&espi->event, ESPI_EV_TNF);
	debug("***spi_xfer:...%08x written\n", tmpdout);

	tmp_tx_timeout = fsl->tx_timeout;
	/* Wait for eSPI transmit to go out */
	while (tmp_tx_timeout--) {
		event = in_be32(&espi->event);
		if (event & ESPI_EV_DON || event & ESPI_EV_TXE) {
			out_be32(&espi->event, ESPI_EV_TXE);
			break;
		}
		udelay(1);
	}

	if (tmp_tx_timeout < 0)
		debug("***spi_xfer:...Tx timeout! event = %08x\n", event);
}

static int fsl_espi_rx(struct fsl_spi_slave *fsl, void *din,
		       unsigned int bytes)
{
	ccsr_espi_t *espi = fsl->espi;
	unsigned int tmpdin, rx_times;
	unsigned char *buf, *p_cursor;

	if (bytes <= 0)
		return 0;

	rx_times = DIV_ROUND_UP(bytes, 4);
	buf = (unsigned char *)malloc(4 * rx_times);
	if (!buf) {
		debug("SF: Failed to malloc memory.\n");
		return -1;
	}
	p_cursor = buf;
	while (rx_times--) {
		tmpdin = in_be32(&espi->rx);
		debug("***spi_xfer:...%08x readed\n", tmpdin);
		*(u32 *)p_cursor = tmpdin;
		p_cursor += 4;
	}

	if (din)
		memcpy(din, buf, bytes);

	free(buf);
	out_be32(&espi->event, ESPI_EV_RNE);

	return bytes;
}

void  espi_release_bus(struct fsl_spi_slave *fsl)
{
	/* Disable the SPI hardware */
	 out_be32(&fsl->espi->mode,
		  in_be32(&fsl->espi->mode) & (~ESPI_MODE_EN));
}

int espi_xfer(struct fsl_spi_slave *fsl,  uint cs, unsigned int bitlen,
	      const void *data_out, void *data_in, unsigned long flags)
{
	struct spi_slave *slave = &fsl->slave;
	ccsr_espi_t *espi = fsl->espi;
	unsigned int event, rx_bytes;
	const void *dout = NULL;
	void *din = NULL;
	int len = 0;
	int num_blks, num_chunks, max_tran_len, tran_len;
	int num_bytes;
	unsigned char *buffer = NULL;
	size_t buf_len;
	u8 *cmd_buf = fsl->cmd_buf;
	size_t cmd_len = fsl->cmd_len;
	size_t data_len = bitlen / 8;
	size_t rx_offset = 0;
	int rf_cnt;

	max_tran_len = fsl->max_transfer_length;
	switch (flags) {
	case SPI_XFER_BEGIN:
		cmd_len = data_len;
		fsl->cmd_len = cmd_len;
		memcpy(cmd_buf, data_out, cmd_len);
		return 0;
	case 0:
	case SPI_XFER_END:
		if (bitlen == 0) {
			fsl_spi_cs_deactivate(slave);
			return 0;
		}
		buf_len = 2 * cmd_len + min(data_len, (size_t)max_tran_len);
		len = cmd_len + data_len;
		rx_offset = cmd_len;
		buffer = (unsigned char *)malloc(buf_len);
		if (!buffer) {
			debug("SF: Failed to malloc memory.\n");
			return 1;
		}
		memcpy(buffer, cmd_buf, cmd_len);
		if (data_in == NULL)
			memcpy(buffer + cmd_len, data_out, data_len);
		break;
	case SPI_XFER_BEGIN | SPI_XFER_END:
		len = data_len;
		buffer = (unsigned char *)malloc(len * 2);
		if (!buffer) {
			debug("SF: Failed to malloc memory.\n");
			return 1;
		}
		memcpy(buffer, data_out, len);
		rx_offset = len;
		cmd_len = 0;
		break;
	}

	debug("spi_xfer: data_out %08X(%p) data_in %08X(%p) len %u\n",
	      *(uint *)data_out, data_out, *(uint *)data_in, data_in, len);

	num_chunks = DIV_ROUND_UP(data_len, max_tran_len);
	while (num_chunks--) {
		if (data_in)
			din = buffer + rx_offset;
		dout = buffer;
		tran_len = min(data_len, (size_t)max_tran_len);
		num_blks = DIV_ROUND_UP(tran_len + cmd_len, 4);
		num_bytes = (tran_len + cmd_len) % 4;
		fsl->data_len = tran_len + cmd_len;
		fsl_spi_cs_activate(slave, cs);

		/* Clear all eSPI events */
		out_be32(&espi->event , 0xffffffff);
		/* handle data in 32-bit chunks */
		while (num_blks) {
			event = in_be32(&espi->event);
			if (event & ESPI_EV_TNF) {
				fsl_espi_tx(fsl, dout);
				/* Set up the next iteration */
				if (len > 4) {
					len -= 4;
					dout += 4;
				}
			}

			event = in_be32(&espi->event);
			if (event & ESPI_EV_RNE) {
				rf_cnt = ((event & ESPI_EV_RFCNT_MASK)
						>> ESPI_EV_RFCNT_SHIFT);
				if (rf_cnt >= 4)
					rx_bytes = 4;
				else if (num_blks == 1 && rf_cnt == num_bytes)
					rx_bytes = num_bytes;
				else
					continue;
				if (fsl_espi_rx(fsl, din, rx_bytes)
						== rx_bytes) {
					num_blks--;
					if (din)
						din = (unsigned char *)din
							+ rx_bytes;
				}
			}
		}
		if (data_in) {
			memcpy(data_in, buffer + 2 * cmd_len, tran_len);
			if (*buffer == 0x0b) {
				data_in += tran_len;
				data_len -= tran_len;
				*(int *)buffer += tran_len;
			}
		}
		fsl_spi_cs_deactivate(slave);
	}

	free(buffer);
	return 0;
}

void espi_claim_bus(struct fsl_spi_slave *fsl, unsigned int cs)
{
	ccsr_espi_t *espi = fsl->espi;
	unsigned char pm = fsl->pm;
	unsigned int mode =  fsl->mode;
	unsigned int div16 = fsl->div16;
	int i;

	/* Enable eSPI interface */
	out_be32(&espi->mode, ESPI_MODE_RXTHR(3)
			| ESPI_MODE_TXTHR(4) | ESPI_MODE_EN);

	out_be32(&espi->event, 0xffffffff); /* Clear all eSPI events */
	out_be32(&espi->mask, 0x00000000); /* Mask  all eSPI interrupts */

	/* Init CS mode interface */
	for (i = 0; i < ESPI_MAX_CS_NUM; i++)
		out_be32(&espi->csmode[i], ESPI_CSMODE_INIT_VAL);

	out_be32(&espi->csmode[cs], in_be32(&espi->csmode[cs]) &
		~(ESPI_CSMODE_PM(0xF) | ESPI_CSMODE_DIV16
		| ESPI_CSMODE_CI_INACTIVEHIGH | ESPI_CSMODE_CP_BEGIN_EDGCLK
		| ESPI_CSMODE_REV_MSB_FIRST | ESPI_CSMODE_LEN(0xF)));

	/* Set eSPI BRG clock source */
	out_be32(&espi->csmode[cs], in_be32(&espi->csmode[cs])
		| ESPI_CSMODE_PM(pm) | div16);

	/* Set eSPI mode */
	if (mode & SPI_CPHA)
		out_be32(&espi->csmode[cs], in_be32(&espi->csmode[cs])
			| ESPI_CSMODE_CP_BEGIN_EDGCLK);
	if (mode & SPI_CPOL)
		out_be32(&espi->csmode[cs], in_be32(&espi->csmode[cs])
			| ESPI_CSMODE_CI_INACTIVEHIGH);

	/* Character bit order: msb first */
	out_be32(&espi->csmode[cs], in_be32(&espi->csmode[cs])
		| ESPI_CSMODE_REV_MSB_FIRST);

	/* Character length in bits, between 0x3~0xf, i.e. 4bits~16bits */
	out_be32(&espi->csmode[cs], in_be32(&espi->csmode[cs])
		| ESPI_CSMODE_LEN(7));
}

void espi_setup_slave(struct fsl_spi_slave *fsl)
{
	unsigned int max_hz;
	sys_info_t sysinfo;
	unsigned long spibrg = 0;
	unsigned long spi_freq = 0;
	unsigned char pm = 0;

	max_hz = fsl->speed_hz;

	get_sys_info(&sysinfo);
	spibrg = sysinfo.freq_systembus / 2;
	fsl->div16 = 0;
	if ((spibrg / max_hz) > 32) {
		fsl->div16 = ESPI_CSMODE_DIV16;
		pm = spibrg / (max_hz * 16 * 2);
		if (pm > 16) {
			pm = 16;
			debug("max_hz is too low: %d Hz, %ld Hz is used.\n",
			      max_hz, spibrg / (32 * 16));
		}
	} else {
		pm = spibrg / (max_hz * 2);
	}
	if (pm)
		pm--;
	fsl->pm = pm;

	if (fsl->div16)
		spi_freq = spibrg / ((pm + 1) * 2 * 16);
	else
		spi_freq = spibrg / ((pm + 1) * 2);

	/* set tx_timeout to 10 times of one espi FIFO entry go out */
	fsl->tx_timeout = DIV_ROUND_UP((US_PER_SECOND * ESPI_FIFO_WIDTH_BIT
				* 10), spi_freq);/* Set eSPI BRG clock source */
}

#if !CONFIG_IS_ENABLED(DM_SPI)
int spi_cs_is_valid(unsigned int bus, unsigned int cs)
{
	return bus == 0 && cs < ESPI_MAX_CS_NUM;
}

struct spi_slave *spi_setup_slave(unsigned int bus, unsigned int cs,
				  unsigned int max_hz, unsigned int mode)
{
	struct fsl_spi_slave *fsl;

	if (!spi_cs_is_valid(bus, cs))
		return NULL;

	fsl = spi_alloc_slave(struct fsl_spi_slave, bus, cs);
	if (!fsl)
		return NULL;

	fsl->espi = (void *)(CFG_SYS_MPC85xx_ESPI_ADDR);
	fsl->mode = mode;
	fsl->max_transfer_length = ESPI_MAX_DATA_TRANSFER_LEN;
	fsl->speed_hz = max_hz;

	espi_setup_slave(fsl);

	return &fsl->slave;
}

void spi_free_slave(struct spi_slave *slave)
{
	struct fsl_spi_slave *fsl = to_fsl_spi_slave(slave);

	free(fsl);
}

int spi_claim_bus(struct spi_slave *slave)
{
	struct fsl_spi_slave *fsl = to_fsl_spi_slave(slave);

	espi_claim_bus(fsl, slave->cs);

	return 0;
}

void spi_release_bus(struct spi_slave *slave)
{
	struct fsl_spi_slave *fsl = to_fsl_spi_slave(slave);

	espi_release_bus(fsl);
}

int spi_xfer(struct spi_slave *slave, unsigned int bitlen, const void *dout,
	     void *din, unsigned long flags)
{
	struct fsl_spi_slave *fsl = (struct fsl_spi_slave *)slave;

	return espi_xfer(fsl, slave->cs, bitlen, dout, din, flags);
}
#else
static void __espi_set_speed(struct fsl_spi_slave *fsl)
{
	espi_setup_slave(fsl);

	/* Set eSPI BRG clock source */
	out_be32(&fsl->espi->csmode[fsl->cs],
		 in_be32(&fsl->espi->csmode[fsl->cs])
			 | ESPI_CSMODE_PM(fsl->pm) | fsl->div16);
}

static void __espi_set_mode(struct fsl_spi_slave *fsl)
{
	/* Set eSPI mode */
	if (fsl->mode & SPI_CPHA)
		out_be32(&fsl->espi->csmode[fsl->cs],
			 in_be32(&fsl->espi->csmode[fsl->cs])
				| ESPI_CSMODE_CP_BEGIN_EDGCLK);
	if (fsl->mode & SPI_CPOL)
		out_be32(&fsl->espi->csmode[fsl->cs],
			 in_be32(&fsl->espi->csmode[fsl->cs])
				| ESPI_CSMODE_CI_INACTIVEHIGH);
}

static int fsl_espi_claim_bus(struct udevice *dev)
{
	struct udevice *bus = dev->parent;
	struct fsl_spi_slave  *fsl =  dev_get_priv(bus);

	espi_claim_bus(fsl, fsl->cs);

	return 0;
}

static int fsl_espi_release_bus(struct udevice *dev)
{
	struct udevice *bus = dev->parent;
	struct fsl_spi_slave *fsl = dev_get_priv(bus);

	espi_release_bus(fsl);

	return 0;
}

static int fsl_espi_xfer(struct udevice *dev, unsigned int bitlen,
			 const void *dout, void *din, unsigned long flags)
{
	struct udevice *bus = dev->parent;
	struct fsl_spi_slave *fsl = dev_get_priv(bus);

	return espi_xfer(fsl, fsl->cs, bitlen, dout, din, flags);
}

static int fsl_espi_set_speed(struct udevice *bus, uint speed)
{
	struct fsl_spi_slave *fsl = dev_get_priv(bus);

	debug("%s speed %u\n", __func__, speed);
	fsl->speed_hz = speed;

	__espi_set_speed(fsl);

	return 0;
}

static int fsl_espi_set_mode(struct udevice *bus, uint mode)
{
	struct fsl_spi_slave *fsl = dev_get_priv(bus);

	debug("%s mode %u\n", __func__, mode);
	fsl->mode = mode;

	__espi_set_mode(fsl);

	return 0;
}

static int fsl_espi_child_pre_probe(struct udevice *dev)
{
	struct dm_spi_slave_plat *slave_plat = dev_get_parent_plat(dev);
	struct udevice *bus = dev->parent;
	struct fsl_spi_slave *fsl = dev_get_priv(bus);

	debug("%s cs %u\n", __func__, slave_plat->cs);
	fsl->cs = slave_plat->cs;

	return 0;
}

static int fsl_espi_probe(struct udevice *bus)
{
	struct fsl_espi_plat *plat = dev_get_plat(bus);
	struct fsl_spi_slave *fsl = dev_get_priv(bus);

	fsl->espi = (ccsr_espi_t *)((u32)plat->regs_addr);
	fsl->max_transfer_length = ESPI_MAX_DATA_TRANSFER_LEN;
	fsl->speed_hz = plat->speed_hz;

	debug("%s probe done, bus-num %d.\n", bus->name, dev_seq(bus));

	return 0;
}

static const struct dm_spi_ops fsl_espi_ops = {
	.claim_bus	= fsl_espi_claim_bus,
	.release_bus	= fsl_espi_release_bus,
	.xfer		= fsl_espi_xfer,
	.set_speed	= fsl_espi_set_speed,
	.set_mode	= fsl_espi_set_mode,
};

#if CONFIG_IS_ENABLED(OF_REAL)
static int fsl_espi_of_to_plat(struct udevice *bus)
{
	fdt_addr_t addr;
	struct fsl_espi_plat   *plat = dev_get_plat(bus);
	const void *blob = gd->fdt_blob;
	int node = dev_of_offset(bus);

	addr = dev_read_addr(bus);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	plat->regs_addr = lower_32_bits(addr);
	plat->speed_hz = fdtdec_get_int(blob, node, "spi-max-frequency",
					FSL_ESPI_DEFAULT_SCK_FREQ);

	debug("ESPI: regs=%p, max-frequency=%d\n",
	      &plat->regs_addr, plat->speed_hz);

	return 0;
}

static const struct udevice_id fsl_espi_ids[] = {
	{ .compatible = "fsl,mpc8536-espi" },
	{ }
};
#endif

U_BOOT_DRIVER(fsl_espi) = {
	.name	= "fsl_espi",
	.id	= UCLASS_SPI,
#if CONFIG_IS_ENABLED(OF_REAL)
	.of_match = fsl_espi_ids,
	.of_to_plat = fsl_espi_of_to_plat,
#endif
	.ops	= &fsl_espi_ops,
	.plat_auto	= sizeof(struct fsl_espi_plat),
	.priv_auto	= sizeof(struct fsl_spi_slave),
	.probe	= fsl_espi_probe,
	.child_pre_probe = fsl_espi_child_pre_probe,
};
#endif
