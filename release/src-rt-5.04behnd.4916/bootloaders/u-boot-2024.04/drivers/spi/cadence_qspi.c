// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2012
 * Altera Corporation <www.altera.com>
 */

#include <common.h>
#include <clk.h>
#include <log.h>
#include <dm.h>
#include <fdtdec.h>
#include <malloc.h>
#include <reset.h>
#include <spi.h>
#include <spi-mem.h>
#include <dm/device_compat.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/io.h>
#include <linux/sizes.h>
#include <linux/time.h>
#include <zynqmp_firmware.h>
#include "cadence_qspi.h"
#include <dt-bindings/power/xlnx-versal-power.h>

#define CQSPI_STIG_READ			0
#define CQSPI_STIG_WRITE		1
#define CQSPI_READ			2
#define CQSPI_WRITE			3

__weak int cadence_qspi_apb_dma_read(struct cadence_spi_priv *priv,
				     const struct spi_mem_op *op)
{
	return 0;
}

__weak int cadence_qspi_versal_flash_reset(struct udevice *dev)
{
	return 0;
}

__weak ofnode cadence_qspi_get_subnode(struct udevice *dev)
{
	return dev_read_first_subnode(dev);
}

static int cadence_spi_write_speed(struct udevice *bus, uint hz)
{
	struct cadence_spi_priv *priv = dev_get_priv(bus);

	cadence_qspi_apb_config_baudrate_div(priv->regbase,
					     priv->ref_clk_hz, hz);

	/* Reconfigure delay timing if speed is changed. */
	cadence_qspi_apb_delay(priv->regbase, priv->ref_clk_hz, hz,
			       priv->tshsl_ns, priv->tsd2d_ns,
			       priv->tchsh_ns, priv->tslch_ns);

	return 0;
}

static int cadence_spi_read_id(struct cadence_spi_priv *priv, u8 len,
			       u8 *idcode)
{
	int err;

	struct spi_mem_op op = SPI_MEM_OP(SPI_MEM_OP_CMD(0x9F, 1),
					  SPI_MEM_OP_NO_ADDR,
					  SPI_MEM_OP_NO_DUMMY,
					  SPI_MEM_OP_DATA_IN(len, idcode, 1));

	err = cadence_qspi_apb_command_read_setup(priv, &op);
	if (!err)
		err = cadence_qspi_apb_command_read(priv, &op);

	return err;
}

/* Calibration sequence to determine the read data capture delay register */
static int spi_calibration(struct udevice *bus, uint hz)
{
	struct cadence_spi_priv *priv = dev_get_priv(bus);
	void *base = priv->regbase;
	unsigned int idcode = 0, temp = 0;
	int err = 0, i, range_lo = -1, range_hi = -1;

	/* start with slowest clock (1 MHz) */
	cadence_spi_write_speed(bus, 1000000);

	/* configure the read data capture delay register to 0 */
	cadence_qspi_apb_readdata_capture(base, 1, 0);

	/* Enable QSPI */
	cadence_qspi_apb_controller_enable(base);

	/* read the ID which will be our golden value */
	err = cadence_spi_read_id(priv, 3, (u8 *)&idcode);
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
		err = cadence_spi_read_id(priv, 3, (u8 *)&temp);
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
	struct cadence_spi_priv *priv = dev_get_priv(bus);
	int err;

	if (!hz || hz > priv->max_hz)
		hz = priv->max_hz;
	/* Disable QSPI */
	cadence_qspi_apb_controller_disable(priv->regbase);

	/*
	 * If the device tree already provides a read delay value, use that
	 * instead of calibrating.
	 */
	if (priv->read_delay >= 0) {
		cadence_spi_write_speed(bus, hz);
		cadence_qspi_apb_readdata_capture(priv->regbase, 1,
						  priv->read_delay);
	} else if (priv->previous_hz != hz ||
		   priv->qspi_calibrated_hz != hz ||
		   priv->qspi_calibrated_cs != spi_chip_select(bus)) {
		/*
		 * Calibration required for different current SCLK speed,
		 * requested SCLK speed or chip select
		 */
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
	struct cadence_spi_plat *plat = dev_get_plat(bus);
	struct cadence_spi_priv *priv = dev_get_priv(bus);
	struct clk clk;
	int ret;

	priv->regbase		= plat->regbase;
	priv->ahbbase		= plat->ahbbase;
	priv->is_dma		= plat->is_dma;
	priv->is_decoded_cs	= plat->is_decoded_cs;
	priv->fifo_depth	= plat->fifo_depth;
	priv->fifo_width	= plat->fifo_width;
	priv->trigger_address	= plat->trigger_address;
	priv->read_delay	= plat->read_delay;
	priv->ahbsize		= plat->ahbsize;
	priv->max_hz		= plat->max_hz;

	priv->page_size		= plat->page_size;
	priv->block_size	= plat->block_size;
	priv->tshsl_ns		= plat->tshsl_ns;
	priv->tsd2d_ns		= plat->tsd2d_ns;
	priv->tchsh_ns		= plat->tchsh_ns;
	priv->tslch_ns		= plat->tslch_ns;

	if (IS_ENABLED(CONFIG_ZYNQMP_FIRMWARE))
		xilinx_pm_request(PM_REQUEST_NODE, PM_DEV_OSPI,
				  ZYNQMP_PM_CAPABILITY_ACCESS, ZYNQMP_PM_MAX_QOS,
				  ZYNQMP_PM_REQUEST_ACK_NO, NULL);

	if (priv->ref_clk_hz == 0) {
		ret = clk_get_by_index(bus, 0, &clk);
		if (ret) {
#ifdef CONFIG_HAS_CQSPI_REF_CLK
			priv->ref_clk_hz = CONFIG_CQSPI_REF_CLK;
#elif defined(CONFIG_ARCH_SOCFPGA)
			priv->ref_clk_hz = cm_get_qspi_controller_clk_hz();
#else
			return ret;
#endif
		} else {
			priv->ref_clk_hz = clk_get_rate(&clk);
			if (IS_ERR_VALUE(priv->ref_clk_hz))
				return priv->ref_clk_hz;
		}
	}

	priv->resets = devm_reset_bulk_get_optional(bus);
	if (priv->resets)
		reset_deassert_bulk(priv->resets);

	if (!priv->qspi_is_init) {
		cadence_qspi_apb_controller_init(priv);
		priv->qspi_is_init = 1;
	}

	priv->wr_delay = 50 * DIV_ROUND_UP(NSEC_PER_SEC, priv->ref_clk_hz);

	/* Versal and Versal-NET use spi calibration to set read delay */
	if (CONFIG_IS_ENABLED(ARCH_VERSAL) ||
	    CONFIG_IS_ENABLED(ARCH_VERSAL_NET))
		if (priv->read_delay >= 0)
			priv->read_delay = -1;

	/* Reset ospi flash device */
	return cadence_qspi_versal_flash_reset(bus);
}

static int cadence_spi_remove(struct udevice *dev)
{
	struct cadence_spi_priv *priv = dev_get_priv(dev);
	int ret = 0;

	if (priv->resets)
		ret = reset_release_bulk(priv->resets);

	return ret;
}

static int cadence_spi_set_mode(struct udevice *bus, uint mode)
{
	struct cadence_spi_priv *priv = dev_get_priv(bus);

	/* Disable QSPI */
	cadence_qspi_apb_controller_disable(priv->regbase);

	/* Set SPI mode */
	cadence_qspi_apb_set_clk_mode(priv->regbase, mode);

	/* Enable Direct Access Controller */
	if (priv->use_dac_mode)
		cadence_qspi_apb_dac_mode_enable(priv->regbase);

	/* Enable QSPI */
	cadence_qspi_apb_controller_enable(priv->regbase);

	return 0;
}

static int cadence_spi_mem_exec_op(struct spi_slave *spi,
				   const struct spi_mem_op *op)
{
	struct udevice *bus = spi->dev->parent;
	struct cadence_spi_priv *priv = dev_get_priv(bus);
	void *base = priv->regbase;
	int err = 0;
	u32 mode;

	/* Set Chip select */
	cadence_qspi_apb_chipselect(base, spi_chip_select(spi->dev),
				    priv->is_decoded_cs);

	if (op->data.dir == SPI_MEM_DATA_IN && op->data.buf.in) {
		/*
		 * Performing reads in DAC mode forces to read minimum 4 bytes
		 * which is unsupported on some flash devices during register
		 * reads, prefer STIG mode for such small reads.
		 */
		if (op->data.nbytes <= CQSPI_STIG_DATA_LEN_MAX)
			mode = CQSPI_STIG_READ;
		else
			mode = CQSPI_READ;
	} else {
		if (op->data.nbytes <= CQSPI_STIG_DATA_LEN_MAX)
			mode = CQSPI_STIG_WRITE;
		else
			mode = CQSPI_WRITE;
	}

	switch (mode) {
	case CQSPI_STIG_READ:
		err = cadence_qspi_apb_command_read_setup(priv, op);
		if (!err)
			err = cadence_qspi_apb_command_read(priv, op);
		break;
	case CQSPI_STIG_WRITE:
		err = cadence_qspi_apb_command_write_setup(priv, op);
		if (!err)
			err = cadence_qspi_apb_command_write(priv, op);
		break;
	case CQSPI_READ:
		err = cadence_qspi_apb_read_setup(priv, op);
		if (!err) {
			if (priv->is_dma)
				err = cadence_qspi_apb_dma_read(priv, op);
			else
				err = cadence_qspi_apb_read_execute(priv, op);
		}
		break;
	case CQSPI_WRITE:
		err = cadence_qspi_apb_write_setup(priv, op);
		if (!err)
			err = cadence_qspi_apb_write_execute(priv, op);
		break;
	default:
		err = -1;
		break;
	}

	return err;
}

static bool cadence_spi_mem_supports_op(struct spi_slave *slave,
					const struct spi_mem_op *op)
{
	bool all_true, all_false;

	/*
	 * op->dummy.dtr is required for converting nbytes into ncycles.
	 * Also, don't check the dtr field of the op phase having zero nbytes.
	 */
	all_true = op->cmd.dtr &&
		   (!op->addr.nbytes || op->addr.dtr) &&
		   (!op->dummy.nbytes || op->dummy.dtr) &&
		   (!op->data.nbytes || op->data.dtr);

	all_false = !op->cmd.dtr && !op->addr.dtr && !op->dummy.dtr &&
		    !op->data.dtr;

	/* Mixed DTR modes not supported. */
	if (!(all_true || all_false))
		return false;

	if (all_true)
		return spi_mem_dtr_supports_op(slave, op);
	else
		return spi_mem_default_supports_op(slave, op);
}

static int cadence_spi_of_to_plat(struct udevice *bus)
{
	struct cadence_spi_plat *plat = dev_get_plat(bus);
	struct cadence_spi_priv *priv = dev_get_priv(bus);
	ofnode subnode;

	plat->regbase = devfdt_get_addr_index_ptr(bus, 0);
	plat->ahbbase = devfdt_get_addr_size_index_ptr(bus, 1, &plat->ahbsize);
	plat->is_decoded_cs = dev_read_bool(bus, "cdns,is-decoded-cs");
	plat->fifo_depth = dev_read_u32_default(bus, "cdns,fifo-depth", 128);
	plat->fifo_width = dev_read_u32_default(bus, "cdns,fifo-width", 4);
	plat->trigger_address = dev_read_u32_default(bus,
						     "cdns,trigger-address",
						     0);
	/* Use DAC mode only when MMIO window is at least 8M wide */
	if (plat->ahbsize >= SZ_8M)
		priv->use_dac_mode = true;

	plat->is_dma = dev_read_bool(bus, "cdns,is-dma");

	/* All other parameters are embedded in the child node */
	subnode = cadence_qspi_get_subnode(bus);
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
	/*
	 * Read delay should be an unsigned value but we use a signed integer
	 * so that negative values can indicate that the device tree did not
	 * specify any signed values and we need to perform the calibration
	 * sequence to find it out.
	 */
	plat->read_delay = ofnode_read_s32_default(subnode, "cdns,read-delay",
						   -1);

	debug("%s: regbase=%p ahbbase=%p max-frequency=%d page-size=%d\n",
	      __func__, plat->regbase, plat->ahbbase, plat->max_hz,
	      plat->page_size);

	return 0;
}

static const struct spi_controller_mem_ops cadence_spi_mem_ops = {
	.exec_op = cadence_spi_mem_exec_op,
	.supports_op = cadence_spi_mem_supports_op,
};

static const struct dm_spi_ops cadence_spi_ops = {
	.set_speed	= cadence_spi_set_speed,
	.set_mode	= cadence_spi_set_mode,
	.mem_ops	= &cadence_spi_mem_ops,
	/*
	 * cs_info is not needed, since we require all chip selects to be
	 * in the device tree explicitly
	 */
};

static const struct udevice_id cadence_spi_ids[] = {
	{ .compatible = "cdns,qspi-nor" },
	{ .compatible = "ti,am654-ospi" },
	{ }
};

U_BOOT_DRIVER(cadence_spi) = {
	.name = "cadence_spi",
	.id = UCLASS_SPI,
	.of_match = cadence_spi_ids,
	.ops = &cadence_spi_ops,
	.of_to_plat = cadence_spi_of_to_plat,
	.plat_auto	= sizeof(struct cadence_spi_plat),
	.priv_auto	= sizeof(struct cadence_spi_priv),
	.probe = cadence_spi_probe,
	.remove = cadence_spi_remove,
	.flags = DM_FLAG_OS_PREPARE,
};
