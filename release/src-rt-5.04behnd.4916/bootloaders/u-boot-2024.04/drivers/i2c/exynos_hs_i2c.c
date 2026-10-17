// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2016, Google Inc
 *
 * (C) Copyright 2002
 * David Mueller, ELSOFT AG, d.mueller@elsoft.ch
 */

#include <common.h>
#include <dm.h>
#include <i2c.h>
#include <log.h>
#include <asm/arch/clk.h>
#include <asm/arch/cpu.h>
#include <asm/arch/pinmux.h>
#include <asm/global_data.h>
#include <linux/delay.h>
#include "s3c24x0_i2c.h"

DECLARE_GLOBAL_DATA_PTR;

/* HSI2C-specific register description */

/* I2C_CTL Register bits */
#define HSI2C_FUNC_MODE_I2C		(1u << 0)
#define HSI2C_MASTER			(1u << 3)
#define HSI2C_RXCHON			(1u << 6)	/* Write/Send */
#define HSI2C_TXCHON			(1u << 7)	/* Read/Receive */
#define HSI2C_SW_RST			(1u << 31)

/* I2C_FIFO_CTL Register bits */
#define HSI2C_RXFIFO_EN			(1u << 0)
#define HSI2C_TXFIFO_EN			(1u << 1)
#define HSI2C_TXFIFO_TRIGGER_LEVEL	(0x20 << 16)
#define HSI2C_RXFIFO_TRIGGER_LEVEL	(0x20 << 4)

/* I2C_TRAILING_CTL Register bits */
#define HSI2C_TRAILING_COUNT		(0xff)

/* I2C_INT_EN Register bits */
#define HSI2C_TX_UNDERRUN_EN		(1u << 2)
#define HSI2C_TX_OVERRUN_EN		(1u << 3)
#define HSI2C_RX_UNDERRUN_EN		(1u << 4)
#define HSI2C_RX_OVERRUN_EN		(1u << 5)
#define HSI2C_INT_TRAILING_EN		(1u << 6)
#define HSI2C_INT_I2C_EN		(1u << 9)

#define HSI2C_INT_ERROR_MASK	(HSI2C_TX_UNDERRUN_EN |\
				 HSI2C_TX_OVERRUN_EN  |\
				 HSI2C_RX_UNDERRUN_EN |\
				 HSI2C_RX_OVERRUN_EN  |\
				 HSI2C_INT_TRAILING_EN)

/* I2C_CONF Register bits */
#define HSI2C_AUTO_MODE			(1u << 31)
#define HSI2C_10BIT_ADDR_MODE		(1u << 30)
#define HSI2C_HS_MODE			(1u << 29)

/* I2C_AUTO_CONF Register bits */
#define HSI2C_READ_WRITE		(1u << 16)
#define HSI2C_STOP_AFTER_TRANS		(1u << 17)
#define HSI2C_MASTER_RUN		(1u << 31)

/* I2C_TIMEOUT Register bits */
#define HSI2C_TIMEOUT_EN		(1u << 31)

/* I2C_TRANS_STATUS register bits */
#define HSI2C_MASTER_BUSY		(1u << 17)
#define HSI2C_SLAVE_BUSY		(1u << 16)
#define HSI2C_TIMEOUT_AUTO		(1u << 4)
#define HSI2C_NO_DEV			(1u << 3)
#define HSI2C_NO_DEV_ACK		(1u << 2)
#define HSI2C_TRANS_ABORT		(1u << 1)
#define HSI2C_TRANS_SUCCESS		(1u << 0)
#define HSI2C_TRANS_ERROR_MASK	(HSI2C_TIMEOUT_AUTO |\
				 HSI2C_NO_DEV | HSI2C_NO_DEV_ACK |\
				 HSI2C_TRANS_ABORT)
#define HSI2C_TRANS_FINISHED_MASK (HSI2C_TRANS_ERROR_MASK | HSI2C_TRANS_SUCCESS)


/* I2C_FIFO_STAT Register bits */
#define HSI2C_RX_FIFO_EMPTY		(1u << 24)
#define HSI2C_RX_FIFO_FULL		(1u << 23)
#define HSI2C_TX_FIFO_EMPTY		(1u << 8)
#define HSI2C_TX_FIFO_FULL		(1u << 7)
#define HSI2C_RX_FIFO_LEVEL(x)		(((x) >> 16) & 0x7f)
#define HSI2C_TX_FIFO_LEVEL(x)		((x) & 0x7f)

#define HSI2C_SLV_ADDR_MAS(x)		((x & 0x3ff) << 10)

#define HSI2C_TIMEOUT_US 10000 /* 10 ms, finer granularity */

/*
 * Wait for transfer completion.
 *
 * This function reads the interrupt status register waiting for the INT_I2C
 * bit to be set, which indicates copletion of a transaction.
 *
 * @param i2c: pointer to the appropriate register bank
 *
 * @return: I2C_OK in case of successful completion, I2C_NOK_TIMEOUT in case
 *          the status bits do not get set in time, or an approrpiate error
 *          value in case of transfer errors.
 */
static int hsi2c_wait_for_trx(struct exynos5_hsi2c *i2c)
{
	int i = HSI2C_TIMEOUT_US;

	while (i-- > 0) {
		u32 int_status = readl(&i2c->usi_int_stat);

		if (int_status & HSI2C_INT_I2C_EN) {
			u32 trans_status = readl(&i2c->usi_trans_status);

			/* Deassert pending interrupt. */
			writel(int_status, &i2c->usi_int_stat);

			if (trans_status & HSI2C_NO_DEV_ACK) {
				debug("%s: no ACK from device\n", __func__);
				return I2C_NACK;
			}
			if (trans_status & HSI2C_NO_DEV) {
				debug("%s: no device\n", __func__);
				return I2C_NOK;
			}
			if (trans_status & HSI2C_TRANS_ABORT) {
				debug("%s: arbitration lost\n", __func__);
				return I2C_NOK_LA;
			}
			if (trans_status & HSI2C_TIMEOUT_AUTO) {
				debug("%s: device timed out\n", __func__);
				return I2C_NOK_TOUT;
			}
			return I2C_OK;
		}
		udelay(1);
	}
	debug("%s: transaction timeout!\n", __func__);
	return I2C_NOK_TOUT;
}

static int hsi2c_get_clk_details(struct s3c24x0_i2c_bus *i2c_bus)
{
	struct exynos5_hsi2c *hsregs = i2c_bus->hsregs;
	ulong clkin;
	unsigned int op_clk = i2c_bus->clock_frequency;
	unsigned int i = 0, utemp0 = 0, utemp1 = 0;
	unsigned int t_ftl_cycle;

#if defined(CONFIG_ARCH_EXYNOS4) || defined(CONFIG_ARCH_EXYNOS5)
	clkin = get_i2c_clk();
#else
	clkin = get_PCLK();
#endif
	/* FPCLK / FI2C =
	 * (CLK_DIV + 1) * (TSCLK_L + TSCLK_H + 2) + 8 + 2 * FLT_CYCLE
	 * uTemp0 = (CLK_DIV + 1) * (TSCLK_L + TSCLK_H + 2)
	 * uTemp1 = (TSCLK_L + TSCLK_H + 2)
	 * uTemp2 = TSCLK_L + TSCLK_H
	 */
	t_ftl_cycle = (readl(&hsregs->usi_conf) >> 16) & 0x7;
	utemp0 = (clkin / op_clk) - 8 - 2 * t_ftl_cycle;

	/* CLK_DIV max is 256 */
	for (i = 0; i < 256; i++) {
		utemp1 = utemp0 / (i + 1);
		if ((utemp1 < 512) && (utemp1 > 4)) {
			i2c_bus->clk_cycle = utemp1 - 2;
			i2c_bus->clk_div = i;
			return 0;
		}
	}
	return -EINVAL;
}

static void hsi2c_ch_init(struct s3c24x0_i2c_bus *i2c_bus)
{
	struct exynos5_hsi2c *hsregs = i2c_bus->hsregs;
	unsigned int t_sr_release;
	unsigned int n_clkdiv;
	unsigned int t_start_su, t_start_hd;
	unsigned int t_stop_su;
	unsigned int t_data_su, t_data_hd;
	unsigned int t_scl_l, t_scl_h;
	u32 i2c_timing_s1;
	u32 i2c_timing_s2;
	u32 i2c_timing_s3;
	u32 i2c_timing_sla;

	n_clkdiv = i2c_bus->clk_div;
	t_scl_l = i2c_bus->clk_cycle / 2;
	t_scl_h = i2c_bus->clk_cycle / 2;
	t_start_su = t_scl_l;
	t_start_hd = t_scl_l;
	t_stop_su = t_scl_l;
	t_data_su = t_scl_l / 2;
	t_data_hd = t_scl_l / 2;
	t_sr_release = i2c_bus->clk_cycle;

	i2c_timing_s1 = t_start_su << 24 | t_start_hd << 16 | t_stop_su << 8;
	i2c_timing_s2 = t_data_su << 24 | t_scl_l << 8 | t_scl_h << 0;
	i2c_timing_s3 = n_clkdiv << 16 | t_sr_release << 0;
	i2c_timing_sla = t_data_hd << 0;

	writel(HSI2C_TRAILING_COUNT, &hsregs->usi_trailing_ctl);

	/* Clear to enable Timeout */
	clrsetbits_le32(&hsregs->usi_timeout, HSI2C_TIMEOUT_EN, 0);

	/* set AUTO mode */
	writel(readl(&hsregs->usi_conf) | HSI2C_AUTO_MODE, &hsregs->usi_conf);

	/* Enable completion conditions' reporting. */
	writel(HSI2C_INT_I2C_EN, &hsregs->usi_int_en);

	/* Enable FIFOs */
	writel(HSI2C_RXFIFO_EN | HSI2C_TXFIFO_EN, &hsregs->usi_fifo_ctl);

	/* Currently operating in Fast speed mode. */
	writel(i2c_timing_s1, &hsregs->usi_timing_fs1);
	writel(i2c_timing_s2, &hsregs->usi_timing_fs2);
	writel(i2c_timing_s3, &hsregs->usi_timing_fs3);
	writel(i2c_timing_sla, &hsregs->usi_timing_sla);
}

/* SW reset for the high speed bus */
static void exynos5_i2c_reset(struct s3c24x0_i2c_bus *i2c_bus)
{
	struct exynos5_hsi2c *i2c = i2c_bus->hsregs;
	u32 i2c_ctl;

	/* Set and clear the bit for reset */
	i2c_ctl = readl(&i2c->usi_ctl);
	i2c_ctl |= HSI2C_SW_RST;
	writel(i2c_ctl, &i2c->usi_ctl);

	i2c_ctl = readl(&i2c->usi_ctl);
	i2c_ctl &= ~HSI2C_SW_RST;
	writel(i2c_ctl, &i2c->usi_ctl);

	/* Initialize the configure registers */
	hsi2c_ch_init(i2c_bus);
}

/*
 * Poll the appropriate bit of the fifo status register until the interface is
 * ready to process the next byte or timeout expires.
 *
 * In addition to the FIFO status register this function also polls the
 * interrupt status register to be able to detect unexpected transaction
 * completion.
 *
 * When FIFO is ready to process the next byte, this function returns I2C_OK.
 * If in course of polling the INT_I2C assertion is detected, the function
 * returns I2C_NOK. If timeout happens before any of the above conditions is
 * met - the function returns I2C_NOK_TOUT;

 * @param i2c: pointer to the appropriate i2c register bank.
 * @param rx_transfer: set to True if the receive transaction is in progress.
 * @return: as described above.
 */
static unsigned hsi2c_poll_fifo(struct exynos5_hsi2c *i2c, bool rx_transfer)
{
	u32 fifo_bit = rx_transfer ? HSI2C_RX_FIFO_EMPTY : HSI2C_TX_FIFO_FULL;
	int i = HSI2C_TIMEOUT_US;

	while (readl(&i2c->usi_fifo_stat) & fifo_bit) {
		if (readl(&i2c->usi_int_stat) & HSI2C_INT_I2C_EN) {
			/*
			 * There is a chance that assertion of
			 * HSI2C_INT_I2C_EN and deassertion of
			 * HSI2C_RX_FIFO_EMPTY happen simultaneously. Let's
			 * give FIFO status priority and check it one more
			 * time before reporting interrupt. The interrupt will
			 * be reported next time this function is called.
			 */
			if (rx_transfer &&
			    !(readl(&i2c->usi_fifo_stat) & fifo_bit))
				break;
			return I2C_NOK;
		}
		if (!i--) {
			debug("%s: FIFO polling timeout!\n", __func__);
			return I2C_NOK_TOUT;
		}
		udelay(1);
	}
	return I2C_OK;
}

/*
 * Preapre hsi2c transaction, either read or write.
 *
 * Set up transfer as described in section 27.5.1.2 'I2C Channel Auto Mode' of
 * the 5420 UM.
 *
 * @param i2c: pointer to the appropriate i2c register bank.
 * @param chip: slave address on the i2c bus (with read/write bit exlcuded)
 * @param len: number of bytes expected to be sent or received
 * @param rx_transfer: set to true for receive transactions
 * @param: issue_stop: set to true if i2c stop condition should be generated
 *         after this transaction.
 * @return: I2C_NOK_TOUT in case the bus remained busy for HSI2C_TIMEOUT_US,
 *          I2C_OK otherwise.
 */
static int hsi2c_prepare_transaction(struct exynos5_hsi2c *i2c,
				     u8 chip,
				     u16 len,
				     bool rx_transfer,
				     bool issue_stop)
{
	u32 conf;

	conf = len | HSI2C_MASTER_RUN;

	if (issue_stop)
		conf |= HSI2C_STOP_AFTER_TRANS;

	/* Clear to enable Timeout */
	writel(readl(&i2c->usi_timeout) & ~HSI2C_TIMEOUT_EN, &i2c->usi_timeout);

	/* Set slave address */
	writel(HSI2C_SLV_ADDR_MAS(chip), &i2c->i2c_addr);

	if (rx_transfer) {
		/* i2c master, read transaction */
		writel((HSI2C_RXCHON | HSI2C_FUNC_MODE_I2C | HSI2C_MASTER),
		       &i2c->usi_ctl);

		/* read up to len bytes, stop after transaction is finished */
		writel(conf | HSI2C_READ_WRITE, &i2c->usi_auto_conf);
	} else {
		/* i2c master, write transaction */
		writel((HSI2C_TXCHON | HSI2C_FUNC_MODE_I2C | HSI2C_MASTER),
		       &i2c->usi_ctl);

		/* write up to len bytes, stop after transaction is finished */
		writel(conf, &i2c->usi_auto_conf);
	}

	/* Reset all pending interrupt status bits we care about, if any */
	writel(HSI2C_INT_I2C_EN, &i2c->usi_int_stat);

	return I2C_OK;
}

/*
 * Wait while i2c bus is settling down (mostly stop gets completed).
 */
static int hsi2c_wait_while_busy(struct exynos5_hsi2c *i2c)
{
	int i = HSI2C_TIMEOUT_US;

	while (readl(&i2c->usi_trans_status) & HSI2C_MASTER_BUSY) {
		if (!i--) {
			debug("%s: bus busy\n", __func__);
			return I2C_NOK_TOUT;
		}
		udelay(1);
	}
	return I2C_OK;
}

static int hsi2c_write(struct exynos5_hsi2c *i2c,
		       unsigned char chip,
		       unsigned char addr[],
		       unsigned char alen,
		       unsigned char data[],
		       unsigned short len,
		       bool issue_stop)
{
	int i, rv = 0;

	if (!(len + alen)) {
		/* Writes of zero length not supported in auto mode. */
		debug("%s: zero length writes not supported\n", __func__);
		return I2C_NOK;
	}

	rv = hsi2c_prepare_transaction
		(i2c, chip, len + alen, false, issue_stop);
	if (rv != I2C_OK)
		return rv;

	/* Move address, if any, and the data, if any, into the FIFO. */
	for (i = 0; i < alen; i++) {
		rv = hsi2c_poll_fifo(i2c, false);
		if (rv != I2C_OK) {
			debug("%s: address write failed\n", __func__);
			goto write_error;
		}
		writel(addr[i], &i2c->usi_txdata);
	}

	for (i = 0; i < len; i++) {
		rv = hsi2c_poll_fifo(i2c, false);
		if (rv != I2C_OK) {
			debug("%s: data write failed\n", __func__);
			goto write_error;
		}
		writel(data[i], &i2c->usi_txdata);
	}

	rv = hsi2c_wait_for_trx(i2c);

 write_error:
	if (issue_stop) {
		int tmp_ret = hsi2c_wait_while_busy(i2c);
		if (rv == I2C_OK)
			rv = tmp_ret;
	}

	writel(HSI2C_FUNC_MODE_I2C, &i2c->usi_ctl); /* done */
	return rv;
}

static int hsi2c_read(struct exynos5_hsi2c *i2c,
		      unsigned char chip,
		      unsigned char addr[],
		      unsigned char alen,
		      unsigned char data[],
		      unsigned short len)
{
	int i, rv, tmp_ret;
	bool drop_data = false;

	if (!len) {
		/* Reads of zero length not supported in auto mode. */
		debug("%s: zero length read adjusted\n", __func__);
		drop_data = true;
		len = 1;
	}

	if (alen) {
		/* Internal register adress needs to be written first. */
		rv = hsi2c_write(i2c, chip, addr, alen, NULL, 0, false);
		if (rv != I2C_OK)
			return rv;
	}

	rv = hsi2c_prepare_transaction(i2c, chip, len, true, true);

	if (rv != I2C_OK)
		return rv;

	for (i = 0; i < len; i++) {
		rv = hsi2c_poll_fifo(i2c, true);
		if (rv != I2C_OK)
			goto read_err;
		if (drop_data)
			continue;
		data[i] = readl(&i2c->usi_rxdata);
	}

	rv = hsi2c_wait_for_trx(i2c);

 read_err:
	tmp_ret = hsi2c_wait_while_busy(i2c);
	if (rv == I2C_OK)
		rv = tmp_ret;

	writel(HSI2C_FUNC_MODE_I2C, &i2c->usi_ctl); /* done */
	return rv;
}

static int exynos_hs_i2c_xfer(struct udevice *dev, struct i2c_msg *msg,
			      int nmsgs)
{
	struct s3c24x0_i2c_bus *i2c_bus = dev_get_priv(dev);
	struct exynos5_hsi2c *hsregs = i2c_bus->hsregs;
	int ret;

	for (; nmsgs > 0; nmsgs--, msg++) {
		if (msg->flags & I2C_M_RD) {
			ret = hsi2c_read(hsregs, msg->addr, 0, 0, msg->buf,
					 msg->len);
		} else {
			ret = hsi2c_write(hsregs, msg->addr, 0, 0, msg->buf,
					  msg->len, true);
		}
		if (ret) {
			exynos5_i2c_reset(i2c_bus);
			return -EREMOTEIO;
		}
	}

	return 0;
}

static int s3c24x0_i2c_set_bus_speed(struct udevice *dev, unsigned int speed)
{
	struct s3c24x0_i2c_bus *i2c_bus = dev_get_priv(dev);

	i2c_bus->clock_frequency = speed;

	if (hsi2c_get_clk_details(i2c_bus))
		return -EFAULT;
	hsi2c_ch_init(i2c_bus);

	return 0;
}

static int s3c24x0_i2c_probe(struct udevice *dev, uint chip, uint chip_flags)
{
	struct s3c24x0_i2c_bus *i2c_bus = dev_get_priv(dev);
	uchar buf[1];
	int ret;

	buf[0] = 0;

	/*
	 * What is needed is to send the chip address and verify that the
	 * address was <ACK>ed (i.e. there was a chip at that address which
	 * drove the data line low).
	 */
	ret = hsi2c_read(i2c_bus->hsregs, chip, 0, 0, buf, 1);

	return ret != I2C_OK;
}

static int s3c_i2c_of_to_plat(struct udevice *dev)
{
	const void *blob = gd->fdt_blob;
	struct s3c24x0_i2c_bus *i2c_bus = dev_get_priv(dev);
	int node;

	node = dev_of_offset(dev);

	i2c_bus->hsregs = dev_read_addr_ptr(dev);

	i2c_bus->id = pinmux_decode_periph_id(blob, node);

	i2c_bus->clock_frequency =
		dev_read_u32_default(dev, "clock-frequency",
				     I2C_SPEED_STANDARD_RATE);
	i2c_bus->node = node;
	i2c_bus->bus_num = dev_seq(dev);

	exynos_pinmux_config(i2c_bus->id, PINMUX_FLAG_HS_MODE);

	i2c_bus->active = true;

	return 0;
}

static const struct dm_i2c_ops exynos_hs_i2c_ops = {
	.xfer		= exynos_hs_i2c_xfer,
	.probe_chip	= s3c24x0_i2c_probe,
	.set_bus_speed	= s3c24x0_i2c_set_bus_speed,
};

static const struct udevice_id exynos_hs_i2c_ids[] = {
	{ .compatible = "samsung,exynos5-hsi2c" },
	{ }
};

U_BOOT_DRIVER(hs_i2c) = {
	.name	= "i2c_s3c_hs",
	.id	= UCLASS_I2C,
	.of_match = exynos_hs_i2c_ids,
	.of_to_plat = s3c_i2c_of_to_plat,
	.priv_auto	= sizeof(struct s3c24x0_i2c_bus),
	.ops	= &exynos_hs_i2c_ops,
};
