// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2012
 * Altera Corporation <www.altera.com>
 */

#include <common.h>
#include <dm.h>
#include <fdtdec.h>
#include <malloc.h>
#include <reset.h>
#include <spi.h>
#include <linux/errno.h>
#include "cadence_qspi.h"

#define CQSPI_STIG_READ			0
#define CQSPI_STIG_WRITE		1
#define CQSPI_INDIRECT_READ		2
#define CQSPI_INDIRECT_WRITE		3

static int cadence_spi_write_speed(struct udevice *bus, uint hz)
{
	struct cadence_spi_platdata *plat = bus->platdata;
	struct cadence_spi_priv *priv = dev_get_priv(bus);

	cadence_qspi_apb_config_baudrate_div(priv->regbase,
					     CONFIG_CQSPI_REF_CLK, hz);

	/* Reconfigure delay timing if speed is changed. */
	cadence_qspi_apb_delay(priv->regbase, CONFIG_CQSPI_REF_CLK, hz,
			       plat->tshsl_ns, plat->tsd2d_ns,
			       plat->tchsh_ns, plat->tslch_ns);

	return 0;
}

/* Calibration sequence to determine the read data capture delay register */
static int spi_calibration(struct udevice *bus, uint hz)
{
	struct cadence_spi_priv *priv = dev_get_priv(bus);
	void *base = priv->regbase;
	u8 opcode_rdid = 0x9F;
	unsigned int idcode = 0, temp = 0;
	int err = 0, i, range_lo = -1, range_hi = -1;

	/* start with slowest clock (1 MHz) */
	cadence_spi_write_speed(bus, 1000000);

	/* configure the read data capture delay register to 0 */
	cadence_qspi_apb_readdata_capture(base, 1, 0);

	/* Enable QSPI */
	cadence_qspi_apb_controller_enable(base);

	/* read the ID which will be our golden value */
	err = cadence_qspi_apb_command_read(base, 1, &opcode_rdid,
		3, (u8 *)&idcode);
	if (err) {
		puts("SF: Calibration failed (read)\n");
		return err;
	}

	/* use back the intended clock and find low range */
	cadence_spi_write_speed(bus, hz);
	for (i = 0; i < CQSPI_READ_CAPTURE_MAX_DELAY; i++) {
		/* Disable QSPI */
		cadence_qspi_apb_controller_disable(base);

		/* reconfigure the read data capture delay register */
		cadence_qspi_apb_readdata_capture(base, 1, i);

		/* Enable back QSPI */
		cadence_qspi_apb_controller_enable(base);

		/* issue a RDID to get the ID value */
		err = cadence_qspi_apb_command_read(base, 1, &opcode_rdid,
			3, (u8 *)&temp);
		if (err) {
			puts("SF: Calibration failed (read)\n");
			return err;
		}

		/* search for range lo */
		if (range_lo == -1 && temp == idcode) {
			range_lo = i;
			continue;
		}

		/* search for range hi */
		if (range_lo != -1 && temp != idcode) {
			range_hi = i - 1;
			break;
		}
		range_hi = i;
	}

	if (range_lo == -1) {
		puts("SF: Calibration failed (low range)\n");
		return err;
	}

	/* Disable QSPI for subsequent initialization */
	cadence_qspi_apb_controller_disable(base);

	/* configure the final value for read data capture delay register */
	cadence_qspi_apb_readdata_capture(base, 1, (range_hi + range_lo) / 2);
	debug("SF: Read data capture delay calibrated to %i (%i - %i)\n",
	      (range_hi + range_lo) / 2, range_lo, range_hi);

	/* just to ensure we do once only when speed or chip select change */
	priv->qspi_calibrated_hz = hz;
	priv->qspi_calibrated_cs = spi_chip_select(bus);

	return 0;
}

static int cadence_spi_set_speed(struct udevice *bus, uint hz)
{
	struct cadence_spi_platdata *plat = bus->platdata;
	struct cadence_spi_priv *priv = dev_get_priv(bus);
	int err;

	if (hz > plat->max_hz)
		hz = plat->max_hz;

	/* Disable QSPI */
	cadence_qspi_apb_controller_disable(priv->regbase);

	/*
	 * Calibration required for different current SCLK speed, requested
	 * SCLK speed or chip select
	 */
	if (priv->previous_hz != hz ||
	    priv->qspi_calibrated_hz != hz ||
	    priv->qspi_calibrated_cs != spi_chip_select(bus)) {
		err = spi_calibration(bus, hz);
		if (err)
			return err;

		/* prevent calibration run when same as previous request */
		priv->previous_hz = hz;
	}

	/* Enable QSPI */
	cadence_qspi_apb_controller_enable(priv->regbase);

	debug("%s: speed=%d\n", __func__, hz);

	return 0;
}

static int cadence_spi_probe(struct udevice *bus)
{
	struct cadence_spi_platdata *plat = bus->platdata;
	struct cadence_spi_priv *priv = dev_get_priv(bus);
	int ret;

	priv->regbase = plat->regbase;
	priv->ahbbase = plat->ahbbase;

	ret = reset_get_bulk(bus, &priv->resets);
	if (ret)
		dev_warn(bus, "Can't get reset: %d\n", ret);
	else
		reset_deassert_bulk(&priv->resets);

	if (!priv->qspi_is_init) {
		cadence_qspi_apb_controller_init(plat);
		priv->qspi_is_init = 1;
	}

	return 0;
}

static int cadence_spi_remove(struct udevice *dev)
{
	struct cadence_spi_priv *priv = dev_get_priv(dev);

	return reset_release_bulk(&priv->resets);
}

static int cadence_spi_set_mode(struct udevice *bus, uint mode)
{
	struct cadence_spi_priv *priv = dev_get_priv(bus);

	/* Disable QSPI */
	cadence_qspi_apb_controller_disable(priv->regbase);

	/* Set SPI mode */
	cadence_qspi_apb_set_clk_mode(priv->regbase, mode);

	/* Enable QSPI */
	cadence_qspi_apb_controller_enable(priv->regbase);

	return 0;
}

static int cadence_spi_xfer(struct udevice *dev, unsigned int bitlen,
			    const void *dout, void *din, unsigned long flags)
{
	struct udevice *bus = dev->parent;
	struct cadence_spi_platdata *plat = bus->platdata;
	struct cadence_spi_priv *priv = dev_get_priv(bus);
	struct dm_spi_slave_platdata *dm_plat = dev_get_parent_platdata(dev);
	void *base = priv->regbase;
	u8 *cmd_buf = priv->cmd_buf;
	size_t data_bytes;
	int err = 0;
	u32 mode = CQSPI_STIG_WRITE;

	if (flags & SPI_XFER_BEGIN) {
		/* copy command to local buffer */
		priv->cmd_len = bitlen / 8;
		memcpy(cmd_buf, dout, priv->cmd_len);
	}

	if (flags == (SPI_XFER_BEGIN | SPI_XFER_END)) {
		/* if start and end bit are set, the data bytes is 0. */
		data_bytes = 0;
	} else {
		data_bytes = bitlen / 8;
	}
	debug("%s: len=%zu [bytes]\n", __func__, data_bytes);

	/* Set Chip select */
	cadence_qspi_apb_chipselect(base, spi_chip_select(dev),
				    plat->is_decoded_cs);

	if ((flags & SPI_XFER_END) || (flags == 0)) {
		if (priv->cmd_len == 0) {
			printf("QSPI: Error, command is empty.\n");
			return -1;
		}

		if (din && data_bytes) {
			/* read */
			/* Use STIG if no address. */
			if (!CQSPI_IS_ADDR(priv->cmd_len))
				mode = CQSPI_STIG_READ;
			else
				mode = CQSPI_INDIRECT_READ;
		} else if (dout && !(flags & SPI_XFER_BEGIN)) {
			/* write */
			if (!CQSPI_IS_ADDR(priv->cmd_len))
				mode = CQSPI_STIG_WRITE;
			else
				mode = CQSPI_INDIRECT_WRITE;
		}

		switch (mode) {
		case CQSPI_STIG_READ:
			err = cadence_qspi_apb_command_read(
				base, priv->cmd_len, cmd_buf,
				data_bytes, din);

		break;
		case CQSPI_STIG_WRITE:
			err = cadence_qspi_apb_command_write(base,
				priv->cmd_len, cmd_buf,
				data_bytes, dout);
		break;
		case CQSPI_INDIRECT_READ:
			err = cadence_qspi_apb_indirect_read_setup(plat,
				priv->cmd_len, dm_plat->mode, cmd_buf);
			if (!err) {
				err = cadence_qspi_apb_indirect_read_execute
				(plat, data_bytes, din);
			}
		break;
		case CQSPI_INDIRECT_WRITE:
			err = cadence_qspi_apb_indirect_write_setup
				(plat, priv->cmd_len, dm_plat->mode, cmd_buf);
			if (!err) {
				err = cadence_qspi_apb_indirect_write_execute
				(plat, data_bytes, dout);
			}
		break;
		default:
			err = -1;
			break;
		}

		if (flags & SPI_XFER_END) {
			/* clear command buffer */
			memset(cmd_buf, 0, sizeof(priv->cmd_buf));
			priv->cmd_len = 0;
		}
	}

	return err;
}

static int cadence_spi_ofdata_to_platdata(struct udevice *bus)
{
	struct cadence_spi_platdata *plat = bus->platdata;
	ofnode subnode;

	plat->regbase = (void *)devfdt_get_addr_index(bus, 0);
	plat->ahbbase = (void *)devfdt_get_addr_index(bus, 1);
	plat->is_decoded_cs = dev_read_bool(bus, "cdns,is-decoded-cs");
	plat->fifo_depth = dev_read_u32_default(bus, "cdns,fifo-depth", 128);
	plat->fifo_width = dev_read_u32_default(bus, "cdns,fifo-width", 4);
	plat->trigger_address = dev_read_u32_default(bus,
						     "cdns,trigger-address",
						     0);

	/* All other paramters are embedded in the child node */
	subnode = dev_read_first_subnode(bus);
	if (!ofnode_valid(subnode)) {
		printf("Error: subnode with SPI flash config missing!\n");
		return -ENODEV;
	}

	/* Use 500 KHz as a suitable default */
	plat->max_hz = ofnode_read_u32_default(subnode, "spi-max-frequency",
					       500000);

	/* Read other parameters from DT */
	plat->page_size = ofnode_read_u32_default(subnode, "page-size", 256);
	plat->block_size = ofnode_read_u32_default(subnode, "block-size", 16);
	plat->tshsl_ns = ofnode_read_u32_default(subnode, "cdns,tshsl-ns",
						 200);
	plat->tsd2d_ns = ofnode_read_u32_default(subnode, "cdns,tsd2d-ns",
						 255);
	plat->tchsh_ns = ofnode_read_u32_default(subnode, "cdns,tchsh-ns", 20);
	plat->tslch_ns = ofnode_read_u32_default(subnode, "cdns,tslch-ns", 20);

	debug("%s: regbase=%p ahbbase=%p max-frequency=%d page-size=%d\n",
	      __func__, plat->regbase, plat->ahbbase, plat->max_hz,
	      plat->page_size);

	return 0;
}

static const struct dm_spi_ops cadence_spi_ops = {
	.xfer		= cadence_spi_xfer,
	.set_speed	= cadence_spi_set_speed,
	.set_mode	= cadence_spi_set_mode,
	/*
	 * cs_info is not needed, since we require all chip selects to be
	 * in the device tree explicitly
	 */
};

static const struct udevice_id cadence_spi_ids[] = {
	{ .compatible = "cdns,qspi-nor" },
	{ }
};

U_BOOT_DRIVER(cadence_spi) = {
	.name = "cadence_spi",
	.id = UCLASS_SPI,
	.of_match = cadence_spi_ids,
	.ops = &cadence_spi_ops,
	.ofdata_to_platdata = cadence_spi_ofdata_to_platdata,
	.platdata_auto_alloc_size = sizeof(struct cadence_spi_platdata),
	.priv_auto_alloc_size = sizeof(struct cadence_spi_priv),
	.probe = cadence_spi_probe,
	.remove = cadence_spi_remove,
	.flags = DM_FLAG_OS_PREPARE,
};
