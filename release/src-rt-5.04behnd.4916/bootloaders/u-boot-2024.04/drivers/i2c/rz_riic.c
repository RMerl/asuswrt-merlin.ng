// SPDX-License-Identifier: GPL-2.0+
/*
 * RZ/G2L I2C (RIIC) driver
 *
 * Copyright (C) 2021-2023 Renesas Electronics Corp.
 */

#include <asm/io.h>
#include <clk.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <errno.h>
#include <i2c.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <reset.h>
#include <wait_bit.h>

#define RIIC_ICCR1	0x00
#define RIIC_ICCR2	0x04
#define RIIC_ICMR1	0x08
#define RIIC_ICMR2	0x0c
#define RIIC_ICMR3	0x10
#define RIIC_ICFER	0x14
#define RIIC_ICSER	0x18
#define RIIC_ICIER	0x1c
#define RIIC_ICSR1	0x20
#define RIIC_ICSR2	0x24
#define RIIC_ICSAR0	0x28
#define RIIC_ICBRL	0x34
#define RIIC_ICBRH	0x38
#define RIIC_ICDRT	0x3c
#define RIIC_ICDRR	0x40

/* ICCR1 */
#define ICCR1_ICE	BIT(7)
#define ICCR1_IICRST	BIT(6)
#define ICCR1_CLO	BIT(5)
#define ICCR1_SOWP	BIT(4)
#define ICCR1_SCLO	BIT(3)
#define ICCR1_SDAO	BIT(2)
#define ICCR1_SCLI	BIT(1)
#define ICCR1_SDAI	BIT(0)

/* ICCR2 */
#define ICCR2_BBSY	BIT(7)
#define ICCR2_MST	BIT(6)
#define ICCR2_TRS	BIT(5)
#define ICCR2_SP	BIT(3)
#define ICCR2_RS	BIT(2)
#define ICCR2_ST	BIT(1)

/* ICMR1 */
#define ICMR1_MTWP	BIT(7)
#define ICMR1_CKS_MASK	GENMASK(6, 4)
#define ICMR1_BCWP	BIT(3)
#define ICMR1_BC_MASK	GENMASK(2, 0)

#define ICMR1_CKS(x)	(((x) << 4) & ICMR1_CKS_MASK)
#define ICMR1_BC(x)	((x) & ICMR1_BC_MASK)

/* ICMR2 */
#define ICMR2_DLCS	BIT(7)
#define ICMR2_SDDL_MASK	GENMASK(6, 4)
#define ICMR2_TMOH	BIT(2)
#define ICMR2_TMOL	BIT(1)
#define ICMR2_TMOS	BIT(0)

/* ICMR3 */
#define ICMR3_SMBS	BIT(7)
#define ICMR3_WAIT	BIT(6)
#define ICMR3_RDRFS	BIT(5)
#define ICMR3_ACKWP	BIT(4)
#define ICMR3_ACKBT	BIT(3)
#define ICMR3_ACKBR	BIT(2)
#define ICMR3_NF_MASK	GENMASK(1, 0)

/* ICFER */
#define ICFER_FMPE	BIT(7)
#define ICFER_SCLE	BIT(6)
#define ICFER_NFE	BIT(5)
#define ICFER_NACKE	BIT(4)
#define ICFER_SALE	BIT(3)
#define ICFER_NALE	BIT(2)
#define ICFER_MALE	BIT(1)
#define ICFER_TMOE	BIT(0)

/* ICSER */
#define ICSER_HOAE	BIT(7)
#define ICSER_DIDE	BIT(5)
#define ICSER_GCAE	BIT(3)
#define ICSER_SAR2E	BIT(2)
#define ICSER_SAR1E	BIT(1)
#define ICSER_SAR0E	BIT(0)

/* ICIER */
#define ICIER_TIE	BIT(7)
#define ICIER_TEIE	BIT(6)
#define ICIER_RIE	BIT(5)
#define ICIER_NAKIE	BIT(4)
#define ICIER_SPIE	BIT(3)
#define ICIER_STIE	BIT(2)
#define ICIER_ALIE	BIT(1)
#define ICIER_TMOIE	BIT(0)

/* ICSR1 */
#define ICSR1_HOA	BIT(7)
#define ICSR1_DID	BIT(5)
#define ICSR1_GCA	BIT(3)
#define ICSR1_AAS2	BIT(2)
#define ICSR1_AAS1	BIT(1)
#define ICSR1_AAS0	BIT(0)

/* ICSR2 */
#define ICSR2_TDRE	BIT(7)
#define ICSR2_TEND	BIT(6)
#define ICSR2_RDRF	BIT(5)
#define ICSR2_NACKF	BIT(4)
#define ICSR2_STOP	BIT(3)
#define ICSR2_START	BIT(2)
#define ICSR2_AL	BIT(1)
#define ICSR2_TMOF	BIT(0)

/* ICBRH */
#define ICBRH_RESERVED	GENMASK(7, 5)	/* The write value should always be 1 */
#define ICBRH_BRH_MASK	GENMASK(4, 0)

/* ICBRL */
#define ICBRL_RESERVED	GENMASK(7, 5)	/* The write value should always be 1 */
#define ICBRL_BRL_MASK	GENMASK(4, 0)

#define RIIC_TIMEOUT_MSEC	100

#define RIIC_FLAG_DEFAULT_SCL_RISE_TIME		BIT(0)
#define RIIC_FLAG_DEFAULT_SCL_FALL_TIME		BIT(1)

/*
 * If SDA is stuck in a low state, the I2C spec says up to 9 clock cycles on SCL
 * may be needed to unblock whichever other device on the bus is holding SDA low.
 */
#define I2C_DEBLOCK_MAX_CYCLES 9

struct riic_priv {
	void __iomem *base;
	struct clk clk;
	uint bus_speed;
	u32 scl_rise_ns;
	u32 scl_fall_ns;
	u32 flags;
};

static int riic_check_busy(struct udevice *dev)
{
	struct riic_priv *priv = dev_get_priv(dev);
	int ret;

	ret = wait_for_bit_8(priv->base + RIIC_ICCR2, ICCR2_BBSY, 0,
			     RIIC_TIMEOUT_MSEC, 0);
	if (ret == -ETIMEDOUT) {
		dev_dbg(dev, "bus is busy!\n");
		return -EBUSY;
	}

	return ret;
}

static int riic_wait_for_icsr2(struct udevice *dev, u8 bit)
{
	struct riic_priv *priv = dev_get_priv(dev);
	ulong start = get_timer(0);
	u8 icsr2;

	/* We can't use wait_for_bit_8() here as we need to check for NACK. */
	while (!((icsr2 = readb(priv->base + RIIC_ICSR2)) & bit)) {
		if (icsr2 & ICSR2_NACKF)
			return -EIO;
		if (get_timer(start) > RIIC_TIMEOUT_MSEC) {
			dev_dbg(dev, "timeout! (bit=%x, icsr2=%x, iccr2=%x)\n",
				bit, icsr2, readb(priv->base + RIIC_ICCR2));
			return -ETIMEDOUT;
		}
		udelay(1);
		schedule();
	}

	return 0;
}

static int riic_check_nack_receive(struct udevice *dev)
{
	struct riic_priv *priv = dev_get_priv(dev);

	if (readb(priv->base + RIIC_ICSR2) & ICSR2_NACKF) {
		dev_dbg(dev, "received nack!\n");
		/* received NACK */
		clrbits_8(priv->base + RIIC_ICSR2, ICSR2_NACKF);
		setbits_8(priv->base + RIIC_ICCR2, ICCR2_SP);
		readb(priv->base + RIIC_ICDRR);	/* dummy read */
		return -EIO;
	}
	return 0;
}

static int riic_i2c_raw_write(struct udevice *dev, u8 *buf, size_t len)
{
	struct riic_priv *priv = dev_get_priv(dev);
	size_t i;
	int ret;

	for (i = 0; i < len; i++) {
		ret = riic_check_nack_receive(dev);
		if (ret < 0)
			return ret;

		ret = riic_wait_for_icsr2(dev, ICSR2_TDRE);
		if (ret < 0)
			return ret;

		writeb(buf[i], priv->base + RIIC_ICDRT);
	}

	return riic_check_nack_receive(dev);
}

static int riic_send_start_cond(struct udevice *dev, int restart)
{
	struct riic_priv *priv = dev_get_priv(dev);
	int ret;

	if (restart)
		setbits_8(priv->base + RIIC_ICCR2, ICCR2_RS);
	else
		setbits_8(priv->base + RIIC_ICCR2, ICCR2_ST);

	ret = riic_wait_for_icsr2(dev, ICSR2_START);
	if (ret < 0)
		return ret;
	clrbits_8(priv->base + RIIC_ICSR2, ICSR2_START);

	return ret;
}

static int riic_receive_data(struct udevice *dev, struct i2c_msg *msg)
{
	struct riic_priv *priv = dev_get_priv(dev);
	int ret, stop_ret, i;

	ret = riic_wait_for_icsr2(dev, ICSR2_RDRF);
	if (ret < 0)
		goto send_stop;

	ret = riic_check_nack_receive(dev);
	if (ret < 0)
		goto send_stop;

	setbits_8(priv->base + RIIC_ICMR3, ICMR3_WAIT | ICMR3_ACKWP | ICMR3_RDRFS);

	/* A dummy read must be performed to trigger data reception */
	readb(priv->base + RIIC_ICDRR);

	for (i = 0; i < msg->len; i++) {
		ret = riic_wait_for_icsr2(dev, ICSR2_RDRF);
		if (ret < 0)
			goto send_stop;

		if (i == (msg->len - 1)) {
			clrbits_8(priv->base + RIIC_ICSR2, ICSR2_STOP);
			setbits_8(priv->base + RIIC_ICCR2, ICCR2_SP);
			setbits_8(priv->base + RIIC_ICMR3, ICMR3_ACKBT);
		} else {
			clrbits_8(priv->base + RIIC_ICMR3, ICMR3_ACKBT);
		}

		msg->buf[i] = readb(priv->base + RIIC_ICDRR);
	};

send_stop:
	if (ret) {
		/*
		 * We got here due to an error condition, so we need to perform
		 * a dummy read to issue the stop bit.
		 */
		clrbits_8(priv->base + RIIC_ICSR2, ICSR2_STOP);
		setbits_8(priv->base + RIIC_ICCR2, ICCR2_SP);
		readb(priv->base + RIIC_ICDRR);
	}
	stop_ret = riic_wait_for_icsr2(dev, ICSR2_STOP);
	clrbits_8(priv->base + RIIC_ICSR2, ICSR2_STOP | ICSR2_NACKF);
	clrbits_8(priv->base + RIIC_ICMR3, ICMR3_WAIT | ICMR3_ACKWP | ICMR3_RDRFS);
	return ret ? ret : stop_ret;
}

static int riic_transmit_stop(struct udevice *dev)
{
	struct riic_priv *priv = dev_get_priv(dev);
	int ret;

	clrbits_8(priv->base + RIIC_ICSR2, ICSR2_STOP);
	setbits_8(priv->base + RIIC_ICCR2, ICCR2_SP);

	ret = riic_wait_for_icsr2(dev, ICSR2_STOP);
	clrbits_8(priv->base + RIIC_ICSR2, ICSR2_STOP | ICSR2_NACKF);
	return ret;
}

static int riic_transmit_data(struct udevice *dev, struct i2c_msg *msg)
{
	int ret, stop_ret;

	ret = riic_i2c_raw_write(dev, msg->buf, msg->len);
	if (ret < 0)
		goto send_stop;

	ret = riic_wait_for_icsr2(dev, ICSR2_TEND);
	if (ret < 0)
		goto send_stop;

	if (!ret && !(msg->flags & I2C_M_STOP))
		return 0;

send_stop:
	stop_ret = riic_transmit_stop(dev);
	return ret ? ret : stop_ret;
}

static int riic_xfer_one(struct udevice *dev, struct i2c_msg *msg, int first_msg)
{
	u8 addr_byte = ((msg->addr << 1) | (msg->flags & I2C_M_RD));
	int ret;

	if (!(msg->flags & I2C_M_NOSTART)) {
		/*
		 * Send a start for the first message and a restart for
		 * subsequent messages.
		 */
		ret = riic_send_start_cond(dev, !first_msg);
		if (ret < 0)
			return ret;
	}

	ret = riic_i2c_raw_write(dev, &addr_byte, 1);
	if (ret < 0) {
		/*
		 * We're aborting the transfer while still in master transmit
		 * mode.
		 */
		riic_transmit_stop(dev);
		return ret;
	}

	if (msg->flags & I2C_M_RD)
		return riic_receive_data(dev, msg);

	return riic_transmit_data(dev, msg);
}

static int riic_xfer(struct udevice *dev, struct i2c_msg *msg, int nmsgs)
{
	int ret, i;

	ret = riic_check_busy(dev);
	if (ret < 0)
		return ret;

	/* Ensure that the last message is terminated with a stop bit. */
	msg[nmsgs - 1].flags |= I2C_M_STOP;

	for (i = 0; i < nmsgs; i++) {
		ret = riic_xfer_one(dev, &msg[i], !i);
		if (ret)
			return ret;
	}

	return 0;
}

static int riic_deblock(struct udevice *dev)
{
	struct riic_priv *priv = dev_get_priv(dev);
	int i = 0;

	/*
	 * Issue clock cycles on SCL to hopefully unblock whatever is holding
	 * SDA low. These clock cycles may trigger error conditions such as
	 * Arbitration Lost, so we clear the status bits in ICSR2 after each
	 * cycle.
	 */
	while (!(readb(priv->base + RIIC_ICCR1) & ICCR1_SDAI)) {
		if (i++ == I2C_DEBLOCK_MAX_CYCLES)
			return -EIO;

		setbits_8(priv->base + RIIC_ICCR1, ICCR1_CLO);
		if (wait_for_bit_8(priv->base + RIIC_ICCR1, ICCR1_CLO, 0,
				   RIIC_TIMEOUT_MSEC, false))
			return -ETIMEDOUT;
		writeb(0, priv->base + RIIC_ICSR2);
	}

	/*
	 * We have released SDA, but the I2C module is now out of sync
	 * with the bus state, so we need to reset its state machine.
	 */
	setbits_8(priv->base + RIIC_ICCR1, ICCR1_IICRST);
	clrbits_8(priv->base + RIIC_ICCR1, ICCR1_IICRST);

	return 0;
}

static int riic_set_bus_speed(struct udevice *dev, uint bus_speed)
{
	struct riic_priv *priv = dev_get_priv(dev);
	ulong refclk;
	uint total_ticks, cks, brl, brh;

	if (bus_speed > I2C_SPEED_FAST_PLUS_RATE) {
		dev_err(dev, "unsupported bus speed (%dHz). %d max\n", bus_speed,
			I2C_SPEED_FAST_PLUS_RATE);
		return -EINVAL;
	}

	/*
	 * Assume the default register settings:
	 *  FER.SCLE = 1 (SCL sync circuit enabled, adds 2 or 3 cycles)
	 *  FER.NFE = 1 (noise circuit enabled)
	 *  MR3.NF = 0 (1 cycle of noise filtered out)
	 *
	 * Freq (CKS=000) = (I2CCLK + tr + tf)/ (BRH + 3 + 1) + (BRL + 3 + 1)
	 * Freq (CKS!=000) = (I2CCLK + tr + tf)/ (BRH + 2 + 1) + (BRL + 2 + 1)
	 */

	/*
	 * Determine reference clock rate. We must be able to get the desired
	 * frequency with only 62 clock ticks max (31 high, 31 low).
	 * Aim for a duty of 60% LOW, 40% HIGH.
	 */
	refclk = clk_get_rate(&priv->clk);
	total_ticks = DIV_ROUND_UP(refclk, bus_speed ?: 1);

	for (cks = 0; cks < 7; cks++) {
		/*
		 * 60% low time must be less than BRL + 2 + 1
		 * BRL max register value is 0x1F.
		 */
		brl = ((total_ticks * 6) / 10);
		if (brl <= (0x1f + 3))
			break;

		total_ticks /= 2;
		refclk /= 2;
	}

	if (brl > (0x1f + 3)) {
		dev_err(dev, "invalid speed (%u). Too slow.\n", bus_speed);
		return -EINVAL;
	}

	brh = total_ticks - brl;

	/* Remove automatic clock ticks for sync circuit and NF */
	if (cks == 0) {
		brl -= 4;
		brh -= 4;
	} else {
		brl -= 3;
		brh -= 3;
	}

	/*
	 * If SCL rise and fall times weren't set in the device tree, set them
	 * based on the desired bus speed and the maximum timings given in the
	 * I2C specification.
	 */
	if (priv->flags & RIIC_FLAG_DEFAULT_SCL_RISE_TIME)
		priv->scl_rise_ns = bus_speed <= I2C_SPEED_STANDARD_RATE ? 1000 :
				    bus_speed <= I2C_SPEED_FAST_RATE ? 300 : 120;
	if (priv->flags & RIIC_FLAG_DEFAULT_SCL_FALL_TIME)
		priv->scl_fall_ns = bus_speed <= I2C_SPEED_FAST_RATE ? 300 : 120;

	/*
	 * Remove clock ticks for rise and fall times. Convert ns to clock
	 * ticks.
	 */
	brl -= priv->scl_fall_ns / (1000000000 / refclk);
	brh -= priv->scl_rise_ns / (1000000000 / refclk);

	/* Adjust for min register values for when SCLE=1 and NFE=1 */
	if (brl < 1)
		brl = 1;
	if (brh < 1)
		brh = 1;

	priv->bus_speed = refclk / total_ticks;
	dev_dbg(dev, "freq=%u, duty=%d, fall=%lu, rise=%lu, cks=%d, brl=%d, brh=%d\n",
		priv->bus_speed, ((brl + 3) * 100) / (brl + brh + 6),
		priv->scl_fall_ns / (1000000000 / refclk),
		priv->scl_rise_ns / (1000000000 / refclk), cks, brl, brh);

	setbits_8(priv->base + RIIC_ICCR1, ICCR1_IICRST);
	writeb(ICMR1_CKS(cks), priv->base + RIIC_ICMR1);
	writeb(brh | ICBRH_RESERVED, priv->base + RIIC_ICBRH);
	writeb(brl | ICBRL_RESERVED, priv->base + RIIC_ICBRL);
	clrbits_8(priv->base + RIIC_ICCR1, ICCR1_IICRST);

	return 0;
}

static int riic_get_bus_speed(struct udevice *dev)
{
	struct riic_priv *priv = dev_get_priv(dev);

	return priv->bus_speed;
}

static const struct dm_i2c_ops riic_ops = {
	.xfer           = riic_xfer,
	.deblock        = riic_deblock,
	.set_bus_speed	= riic_set_bus_speed,
	.get_bus_speed	= riic_get_bus_speed,
};

static int riic_init_setting(struct udevice *dev)
{
	struct riic_priv *priv = dev_get_priv(dev);
	int ret;

	clrbits_8(priv->base + RIIC_ICCR1, ICCR1_ICE);
	setbits_8(priv->base + RIIC_ICCR1, ICCR1_IICRST);
	setbits_8(priv->base + RIIC_ICCR1, ICCR1_ICE);

	/*
	 * Set a default bitrate. The rate may be overridden based on the device
	 * tree as part of i2c_post_probe().
	 */
	ret = riic_set_bus_speed(dev, I2C_SPEED_STANDARD_RATE);
	if (ret < 0)
		goto err;

	clrbits_8(priv->base + RIIC_ICCR1, ICCR1_IICRST);

	/* Make sure the bus is not stuck. */
	if (!(readb(priv->base + RIIC_ICCR1) & ICCR1_SDAI)) {
		dev_dbg(dev, "clearing SDA low state\n");
		ret = riic_deblock(dev);
		if (ret) {
			dev_err(dev, "failed to clear SDA low state!\n");
			goto err;
		}
	}
	return 0;

err:
	clrbits_8(priv->base + RIIC_ICCR1, ICCR1_ICE | ICCR1_IICRST);
	return ret;
}

static int riic_probe(struct udevice *dev)
{
	struct riic_priv *priv = dev_get_priv(dev);
	struct reset_ctl rst;
	int ret;

	priv->base = dev_read_addr_ptr(dev);

	ret = dev_read_u32(dev, "i2c-scl-rising-time-ns", &priv->scl_rise_ns);
	if (ret)
		priv->flags |= RIIC_FLAG_DEFAULT_SCL_RISE_TIME;
	ret = dev_read_u32(dev, "i2c-scl-falling-time-ns", &priv->scl_fall_ns);
	if (ret)
		priv->flags |= RIIC_FLAG_DEFAULT_SCL_FALL_TIME;

	ret = clk_get_by_index(dev, 0, &priv->clk);
	if (ret) {
		dev_err(dev, "failed to get clock\n");
		return ret;
	}

	ret = clk_enable(&priv->clk);
	if (ret) {
		dev_err(dev, "failed to enable clock\n");
		return ret;
	}

	ret = reset_get_by_index(dev, 0, &rst);
	if (ret < 0) {
		dev_err(dev, "failed to get reset line\n");
		goto err_get_reset;
	}

	ret = reset_deassert(&rst);
	if (ret < 0) {
		dev_err(dev, "failed to de-assert reset line\n");
		goto err_reset;
	}

	ret = riic_init_setting(dev);
	if (ret < 0) {
		dev_err(dev, "failed to init i2c bus interface\n");
		goto err_init;
	}

	return 0;

err_init:
	reset_assert(&rst);
err_reset:
	reset_free(&rst);
err_get_reset:
	clk_disable(&priv->clk);
	return ret;
}

static const struct udevice_id riic_ids[] = {
	{ .compatible = "renesas,riic-rz", },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(riic_i2c) = {
	.name           = "riic-i2c",
	.id             = UCLASS_I2C,
	.of_match       = riic_ids,
	.probe          = riic_probe,
	.priv_auto	= sizeof(struct riic_priv),
	.ops            = &riic_ops,
};
