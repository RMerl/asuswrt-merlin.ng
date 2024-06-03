// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2017 STMicroelectronics
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <i2c.h>
#include <reset.h>

#include <dm/device.h>
#include <linux/io.h>

/* STM32 I2C registers */
struct stm32_i2c_regs {
	u32 cr1;	/* I2C control register 1 */
	u32 cr2;	/* I2C control register 2 */
	u32 oar1;	/* I2C own address 1 register */
	u32 oar2;	/* I2C own address 2 register */
	u32 timingr;	/* I2C timing register */
	u32 timeoutr;	/* I2C timeout register */
	u32 isr;	/* I2C interrupt and status register */
	u32 icr;	/* I2C interrupt clear register */
	u32 pecr;	/* I2C packet error checking register */
	u32 rxdr;	/* I2C receive data register */
	u32 txdr;	/* I2C transmit data register */
};

#define STM32_I2C_CR1				0x00
#define STM32_I2C_CR2				0x04
#define STM32_I2C_TIMINGR			0x10
#define STM32_I2C_ISR				0x18
#define STM32_I2C_ICR				0x1C
#define STM32_I2C_RXDR				0x24
#define STM32_I2C_TXDR				0x28

/* STM32 I2C control 1 */
#define STM32_I2C_CR1_ANFOFF			BIT(12)
#define STM32_I2C_CR1_ERRIE			BIT(7)
#define STM32_I2C_CR1_TCIE			BIT(6)
#define STM32_I2C_CR1_STOPIE			BIT(5)
#define STM32_I2C_CR1_NACKIE			BIT(4)
#define STM32_I2C_CR1_ADDRIE			BIT(3)
#define STM32_I2C_CR1_RXIE			BIT(2)
#define STM32_I2C_CR1_TXIE			BIT(1)
#define STM32_I2C_CR1_PE			BIT(0)

/* STM32 I2C control 2 */
#define STM32_I2C_CR2_AUTOEND			BIT(25)
#define STM32_I2C_CR2_RELOAD			BIT(24)
#define STM32_I2C_CR2_NBYTES_MASK		GENMASK(23, 16)
#define STM32_I2C_CR2_NBYTES(n)			((n & 0xff) << 16)
#define STM32_I2C_CR2_NACK			BIT(15)
#define STM32_I2C_CR2_STOP			BIT(14)
#define STM32_I2C_CR2_START			BIT(13)
#define STM32_I2C_CR2_HEAD10R			BIT(12)
#define STM32_I2C_CR2_ADD10			BIT(11)
#define STM32_I2C_CR2_RD_WRN			BIT(10)
#define STM32_I2C_CR2_SADD10_MASK		GENMASK(9, 0)
#define STM32_I2C_CR2_SADD10(n)			(n & STM32_I2C_CR2_SADD10_MASK)
#define STM32_I2C_CR2_SADD7_MASK		GENMASK(7, 1)
#define STM32_I2C_CR2_SADD7(n)			((n & 0x7f) << 1)
#define STM32_I2C_CR2_RESET_MASK		(STM32_I2C_CR2_HEAD10R \
						| STM32_I2C_CR2_NBYTES_MASK \
						| STM32_I2C_CR2_SADD7_MASK \
						| STM32_I2C_CR2_RELOAD \
						| STM32_I2C_CR2_RD_WRN)

/* STM32 I2C Interrupt Status */
#define STM32_I2C_ISR_BUSY			BIT(15)
#define STM32_I2C_ISR_ARLO			BIT(9)
#define STM32_I2C_ISR_BERR			BIT(8)
#define STM32_I2C_ISR_TCR			BIT(7)
#define STM32_I2C_ISR_TC			BIT(6)
#define STM32_I2C_ISR_STOPF			BIT(5)
#define STM32_I2C_ISR_NACKF			BIT(4)
#define STM32_I2C_ISR_ADDR			BIT(3)
#define STM32_I2C_ISR_RXNE			BIT(2)
#define STM32_I2C_ISR_TXIS			BIT(1)
#define STM32_I2C_ISR_TXE			BIT(0)
#define STM32_I2C_ISR_ERRORS			(STM32_I2C_ISR_BERR \
						| STM32_I2C_ISR_ARLO)

/* STM32 I2C Interrupt Clear */
#define STM32_I2C_ICR_ARLOCF			BIT(9)
#define STM32_I2C_ICR_BERRCF			BIT(8)
#define STM32_I2C_ICR_STOPCF			BIT(5)
#define STM32_I2C_ICR_NACKCF			BIT(4)

/* STM32 I2C Timing */
#define STM32_I2C_TIMINGR_PRESC(n)		((n & 0xf) << 28)
#define STM32_I2C_TIMINGR_SCLDEL(n)		((n & 0xf) << 20)
#define STM32_I2C_TIMINGR_SDADEL(n)		((n & 0xf) << 16)
#define STM32_I2C_TIMINGR_SCLH(n)		((n & 0xff) << 8)
#define STM32_I2C_TIMINGR_SCLL(n)		(n & 0xff)

#define STM32_I2C_MAX_LEN			0xff

#define STM32_I2C_DNF_DEFAULT			0
#define STM32_I2C_DNF_MAX			16

#define STM32_I2C_ANALOG_FILTER_ENABLE	1
#define STM32_I2C_ANALOG_FILTER_DELAY_MIN	50	/* ns */
#define STM32_I2C_ANALOG_FILTER_DELAY_MAX	260	/* ns */

#define STM32_I2C_RISE_TIME_DEFAULT		25	/* ns */
#define STM32_I2C_FALL_TIME_DEFAULT		10	/* ns */

#define STM32_PRESC_MAX				BIT(4)
#define STM32_SCLDEL_MAX			BIT(4)
#define STM32_SDADEL_MAX			BIT(4)
#define STM32_SCLH_MAX				BIT(8)
#define STM32_SCLL_MAX				BIT(8)

#define STM32_NSEC_PER_SEC			1000000000L

#define STANDARD_RATE				100000
#define FAST_RATE				400000
#define FAST_PLUS_RATE				1000000

enum stm32_i2c_speed {
	STM32_I2C_SPEED_STANDARD, /* 100 kHz */
	STM32_I2C_SPEED_FAST, /* 400 kHz */
	STM32_I2C_SPEED_FAST_PLUS, /* 1 MHz */
	STM32_I2C_SPEED_END,
};

/**
 * struct stm32_i2c_spec - private i2c specification timing
 * @rate: I2C bus speed (Hz)
 * @rate_min: 80% of I2C bus speed (Hz)
 * @rate_max: 120% of I2C bus speed (Hz)
 * @fall_max: Max fall time of both SDA and SCL signals (ns)
 * @rise_max: Max rise time of both SDA and SCL signals (ns)
 * @hddat_min: Min data hold time (ns)
 * @vddat_max: Max data valid time (ns)
 * @sudat_min: Min data setup time (ns)
 * @l_min: Min low period of the SCL clock (ns)
 * @h_min: Min high period of the SCL clock (ns)
 */

struct stm32_i2c_spec {
	u32 rate;
	u32 rate_min;
	u32 rate_max;
	u32 fall_max;
	u32 rise_max;
	u32 hddat_min;
	u32 vddat_max;
	u32 sudat_min;
	u32 l_min;
	u32 h_min;
};

/**
 * struct stm32_i2c_setup - private I2C timing setup parameters
 * @speed: I2C speed mode (standard, Fast Plus)
 * @speed_freq: I2C speed frequency  (Hz)
 * @clock_src: I2C clock source frequency (Hz)
 * @rise_time: Rise time (ns)
 * @fall_time: Fall time (ns)
 * @dnf: Digital filter coefficient (0-16)
 * @analog_filter: Analog filter delay (On/Off)
 */
struct stm32_i2c_setup {
	enum stm32_i2c_speed speed;
	u32 speed_freq;
	u32 clock_src;
	u32 rise_time;
	u32 fall_time;
	u8 dnf;
	bool analog_filter;
};

/**
 * struct stm32_i2c_timings - private I2C output parameters
 * @prec: Prescaler value
 * @scldel: Data setup time
 * @sdadel: Data hold time
 * @sclh: SCL high period (master mode)
 * @sclh: SCL low period (master mode)
 */
struct stm32_i2c_timings {
	struct list_head node;
	u8 presc;
	u8 scldel;
	u8 sdadel;
	u8 sclh;
	u8 scll;
};

struct stm32_i2c_priv {
	struct stm32_i2c_regs *regs;
	struct clk clk;
	struct stm32_i2c_setup *setup;
	int speed;
};

static const struct stm32_i2c_spec i2c_specs[] = {
	[STM32_I2C_SPEED_STANDARD] = {
		.rate = STANDARD_RATE,
		.rate_min = 8000,
		.rate_max = 120000,
		.fall_max = 300,
		.rise_max = 1000,
		.hddat_min = 0,
		.vddat_max = 3450,
		.sudat_min = 250,
		.l_min = 4700,
		.h_min = 4000,
	},
	[STM32_I2C_SPEED_FAST] = {
		.rate = FAST_RATE,
		.rate_min = 320000,
		.rate_max = 480000,
		.fall_max = 300,
		.rise_max = 300,
		.hddat_min = 0,
		.vddat_max = 900,
		.sudat_min = 100,
		.l_min = 1300,
		.h_min = 600,
	},
	[STM32_I2C_SPEED_FAST_PLUS] = {
		.rate = FAST_PLUS_RATE,
		.rate_min = 800000,
		.rate_max = 1200000,
		.fall_max = 100,
		.rise_max = 120,
		.hddat_min = 0,
		.vddat_max = 450,
		.sudat_min = 50,
		.l_min = 500,
		.h_min = 260,
	},
};

static const struct stm32_i2c_setup stm32f7_setup = {
	.rise_time = STM32_I2C_RISE_TIME_DEFAULT,
	.fall_time = STM32_I2C_FALL_TIME_DEFAULT,
	.dnf = STM32_I2C_DNF_DEFAULT,
	.analog_filter = STM32_I2C_ANALOG_FILTER_ENABLE,
};

static int stm32_i2c_check_device_busy(struct stm32_i2c_priv *i2c_priv)
{
	struct stm32_i2c_regs *regs = i2c_priv->regs;
	u32 status = readl(&regs->isr);

	if (status & STM32_I2C_ISR_BUSY)
		return -EBUSY;

	return 0;
}

static void stm32_i2c_message_start(struct stm32_i2c_priv *i2c_priv,
				    struct i2c_msg *msg, bool stop)
{
	struct stm32_i2c_regs *regs = i2c_priv->regs;
	u32 cr2 = readl(&regs->cr2);

	/* Set transfer direction */
	cr2 &= ~STM32_I2C_CR2_RD_WRN;
	if (msg->flags & I2C_M_RD)
		cr2 |= STM32_I2C_CR2_RD_WRN;

	/* Set slave address */
	cr2 &= ~(STM32_I2C_CR2_HEAD10R | STM32_I2C_CR2_ADD10);
	if (msg->flags & I2C_M_TEN) {
		cr2 &= ~STM32_I2C_CR2_SADD10_MASK;
		cr2 |= STM32_I2C_CR2_SADD10(msg->addr);
		cr2 |= STM32_I2C_CR2_ADD10;
	} else {
		cr2 &= ~STM32_I2C_CR2_SADD7_MASK;
		cr2 |= STM32_I2C_CR2_SADD7(msg->addr);
	}

	/* Set nb bytes to transfer and reload or autoend bits */
	cr2 &= ~(STM32_I2C_CR2_NBYTES_MASK | STM32_I2C_CR2_RELOAD |
		 STM32_I2C_CR2_AUTOEND);
	if (msg->len > STM32_I2C_MAX_LEN) {
		cr2 |= STM32_I2C_CR2_NBYTES(STM32_I2C_MAX_LEN);
		cr2 |= STM32_I2C_CR2_RELOAD;
	} else {
		cr2 |= STM32_I2C_CR2_NBYTES(msg->len);
	}

	/* Write configurations register */
	writel(cr2, &regs->cr2);

	/* START/ReSTART generation */
	setbits_le32(&regs->cr2, STM32_I2C_CR2_START);
}

/*
 * RELOAD mode must be selected if total number of data bytes to be
 * sent is greater than MAX_LEN
 */

static void stm32_i2c_handle_reload(struct stm32_i2c_priv *i2c_priv,
				    struct i2c_msg *msg, bool stop)
{
	struct stm32_i2c_regs *regs = i2c_priv->regs;
	u32 cr2 = readl(&regs->cr2);

	cr2 &= ~STM32_I2C_CR2_NBYTES_MASK;

	if (msg->len > STM32_I2C_MAX_LEN) {
		cr2 |= STM32_I2C_CR2_NBYTES(STM32_I2C_MAX_LEN);
	} else {
		cr2 &= ~STM32_I2C_CR2_RELOAD;
		cr2 |= STM32_I2C_CR2_NBYTES(msg->len);
	}

	writel(cr2, &regs->cr2);
}

static int stm32_i2c_wait_flags(struct stm32_i2c_priv *i2c_priv,
				u32 flags, u32 *status)
{
	struct stm32_i2c_regs *regs = i2c_priv->regs;
	u32 time_start = get_timer(0);

	*status = readl(&regs->isr);
	while (!(*status & flags)) {
		if (get_timer(time_start) > CONFIG_SYS_HZ) {
			debug("%s: i2c timeout\n", __func__);
			return -ETIMEDOUT;
		}

		*status = readl(&regs->isr);
	}

	return 0;
}

static int stm32_i2c_check_end_of_message(struct stm32_i2c_priv *i2c_priv)
{
	struct stm32_i2c_regs *regs = i2c_priv->regs;
	u32 mask = STM32_I2C_ISR_ERRORS | STM32_I2C_ISR_NACKF |
		   STM32_I2C_ISR_STOPF;
	u32 status;
	int ret;

	ret = stm32_i2c_wait_flags(i2c_priv, mask, &status);
	if (ret)
		return ret;

	if (status & STM32_I2C_ISR_BERR) {
		debug("%s: Bus error\n", __func__);

		/* Clear BERR flag */
		setbits_le32(&regs->icr, STM32_I2C_ICR_BERRCF);

		return -EIO;
	}

	if (status & STM32_I2C_ISR_ARLO) {
		debug("%s: Arbitration lost\n", __func__);

		/* Clear ARLO flag */
		setbits_le32(&regs->icr, STM32_I2C_ICR_ARLOCF);

		return -EAGAIN;
	}

	if (status & STM32_I2C_ISR_NACKF) {
		debug("%s: Receive NACK\n", __func__);

		/* Clear NACK flag */
		setbits_le32(&regs->icr, STM32_I2C_ICR_NACKCF);

		/* Wait until STOPF flag is set */
		mask = STM32_I2C_ISR_STOPF;
		ret = stm32_i2c_wait_flags(i2c_priv, mask, &status);
		if (ret)
			return ret;

		ret = -EIO;
	}

	if (status & STM32_I2C_ISR_STOPF) {
		/* Clear STOP flag */
		setbits_le32(&regs->icr, STM32_I2C_ICR_STOPCF);

		/* Clear control register 2 */
		setbits_le32(&regs->cr2, STM32_I2C_CR2_RESET_MASK);
	}

	return ret;
}

static int stm32_i2c_message_xfer(struct stm32_i2c_priv *i2c_priv,
				  struct i2c_msg *msg, bool stop)
{
	struct stm32_i2c_regs *regs = i2c_priv->regs;
	u32 status;
	u32 mask = msg->flags & I2C_M_RD ? STM32_I2C_ISR_RXNE :
		   STM32_I2C_ISR_TXIS | STM32_I2C_ISR_NACKF;
	int bytes_to_rw = msg->len > STM32_I2C_MAX_LEN ?
			  STM32_I2C_MAX_LEN : msg->len;
	int ret = 0;

	/* Add errors */
	mask |= STM32_I2C_ISR_ERRORS;

	stm32_i2c_message_start(i2c_priv, msg, stop);

	while (msg->len) {
		/*
		 * Wait until TXIS/NACKF/BERR/ARLO flags or
		 * RXNE/BERR/ARLO flags are set
		 */
		ret = stm32_i2c_wait_flags(i2c_priv, mask, &status);
		if (ret)
			break;

		if (status & (STM32_I2C_ISR_NACKF | STM32_I2C_ISR_ERRORS))
			break;

		if (status & STM32_I2C_ISR_RXNE) {
			*msg->buf++ = readb(&regs->rxdr);
			msg->len--;
			bytes_to_rw--;
		}

		if (status & STM32_I2C_ISR_TXIS) {
			writeb(*msg->buf++, &regs->txdr);
			msg->len--;
			bytes_to_rw--;
		}

		if (!bytes_to_rw && msg->len) {
			/* Wait until TCR flag is set */
			mask = STM32_I2C_ISR_TCR;
			ret = stm32_i2c_wait_flags(i2c_priv, mask, &status);
			if (ret)
				break;

			bytes_to_rw = msg->len > STM32_I2C_MAX_LEN ?
				      STM32_I2C_MAX_LEN : msg->len;
			mask = msg->flags & I2C_M_RD ? STM32_I2C_ISR_RXNE :
			       STM32_I2C_ISR_TXIS | STM32_I2C_ISR_NACKF;

			stm32_i2c_handle_reload(i2c_priv, msg, stop);
		} else if (!bytes_to_rw) {
			/* Wait until TC flag is set */
			mask = STM32_I2C_ISR_TC;
			ret = stm32_i2c_wait_flags(i2c_priv, mask, &status);
			if (ret)
				break;

			if (!stop)
				/* Message sent, new message has to be sent */
				return 0;
		}
	}

	/* End of transfer, send stop condition */
	mask = STM32_I2C_CR2_STOP;
	setbits_le32(&regs->cr2, mask);

	return stm32_i2c_check_end_of_message(i2c_priv);
}

static int stm32_i2c_xfer(struct udevice *bus, struct i2c_msg *msg,
			  int nmsgs)
{
	struct stm32_i2c_priv *i2c_priv = dev_get_priv(bus);
	int ret;

	ret = stm32_i2c_check_device_busy(i2c_priv);
	if (ret)
		return ret;

	for (; nmsgs > 0; nmsgs--, msg++) {
		ret = stm32_i2c_message_xfer(i2c_priv, msg, nmsgs == 1);
		if (ret)
			return ret;
	}

	return 0;
}

static int stm32_i2c_compute_solutions(struct stm32_i2c_setup *setup,
				       struct list_head *solutions)
{
	struct stm32_i2c_timings *v;
	u32 p_prev = STM32_PRESC_MAX;
	u32 i2cclk = DIV_ROUND_CLOSEST(STM32_NSEC_PER_SEC,
				       setup->clock_src);
	u32 af_delay_min, af_delay_max;
	u16 p, l, a;
	int sdadel_min, sdadel_max, scldel_min;
	int ret = 0;

	af_delay_min = setup->analog_filter ?
		       STM32_I2C_ANALOG_FILTER_DELAY_MIN : 0;
	af_delay_max = setup->analog_filter ?
		       STM32_I2C_ANALOG_FILTER_DELAY_MAX : 0;

	sdadel_min = i2c_specs[setup->speed].hddat_min + setup->fall_time -
		     af_delay_min - (setup->dnf + 3) * i2cclk;

	sdadel_max = i2c_specs[setup->speed].vddat_max - setup->rise_time -
		     af_delay_max - (setup->dnf + 4) * i2cclk;

	scldel_min = setup->rise_time + i2c_specs[setup->speed].sudat_min;

	if (sdadel_min < 0)
		sdadel_min = 0;
	if (sdadel_max < 0)
		sdadel_max = 0;

	debug("%s: SDADEL(min/max): %i/%i, SCLDEL(Min): %i\n", __func__,
	      sdadel_min, sdadel_max, scldel_min);

	/* Compute possible values for PRESC, SCLDEL and SDADEL */
	for (p = 0; p < STM32_PRESC_MAX; p++) {
		for (l = 0; l < STM32_SCLDEL_MAX; l++) {
			u32 scldel = (l + 1) * (p + 1) * i2cclk;

			if (scldel < scldel_min)
				continue;

			for (a = 0; a < STM32_SDADEL_MAX; a++) {
				u32 sdadel = (a * (p + 1) + 1) * i2cclk;

				if (((sdadel >= sdadel_min) &&
				     (sdadel <= sdadel_max)) &&
				    (p != p_prev)) {
					v = calloc(1, sizeof(*v));
					if (!v)
						return -ENOMEM;

					v->presc = p;
					v->scldel = l;
					v->sdadel = a;
					p_prev = p;

					list_add_tail(&v->node, solutions);
					break;
				}
			}

			if (p_prev == p)
				break;
		}
	}

	if (list_empty(solutions)) {
		pr_err("%s: no Prescaler solution\n", __func__);
		ret = -EPERM;
	}

	return ret;
}

static int stm32_i2c_choose_solution(struct stm32_i2c_setup *setup,
				     struct list_head *solutions,
				     struct stm32_i2c_timings *s)
{
	struct stm32_i2c_timings *v;
	u32 i2cbus = DIV_ROUND_CLOSEST(STM32_NSEC_PER_SEC,
				       setup->speed_freq);
	u32 clk_error_prev = i2cbus;
	u32 i2cclk = DIV_ROUND_CLOSEST(STM32_NSEC_PER_SEC,
				       setup->clock_src);
	u32 clk_min, clk_max;
	u32 af_delay_min;
	u32 dnf_delay;
	u32 tsync;
	u16 l, h;
	bool sol_found = false;
	int ret = 0;

	af_delay_min = setup->analog_filter ?
		       STM32_I2C_ANALOG_FILTER_DELAY_MIN : 0;
	dnf_delay = setup->dnf * i2cclk;

	tsync = af_delay_min + dnf_delay + (2 * i2cclk);
	clk_max = STM32_NSEC_PER_SEC / i2c_specs[setup->speed].rate_min;
	clk_min = STM32_NSEC_PER_SEC / i2c_specs[setup->speed].rate_max;

	/*
	 * Among Prescaler possibilities discovered above figures out SCL Low
	 * and High Period. Provided:
	 * - SCL Low Period has to be higher than Low Period of the SCL Clock
	 *   defined by I2C Specification. I2C Clock has to be lower than
	 *   (SCL Low Period - Analog/Digital filters) / 4.
	 * - SCL High Period has to be lower than High Period of the SCL Clock
	 *   defined by I2C Specification
	 * - I2C Clock has to be lower than SCL High Period
	 */
	list_for_each_entry(v, solutions, node) {
		u32 prescaler = (v->presc + 1) * i2cclk;

		for (l = 0; l < STM32_SCLL_MAX; l++) {
			u32 tscl_l = (l + 1) * prescaler + tsync;

			if ((tscl_l < i2c_specs[setup->speed].l_min) ||
			    (i2cclk >=
			     ((tscl_l - af_delay_min - dnf_delay) / 4))) {
				continue;
			}

			for (h = 0; h < STM32_SCLH_MAX; h++) {
				u32 tscl_h = (h + 1) * prescaler + tsync;
				u32 tscl = tscl_l + tscl_h +
					   setup->rise_time + setup->fall_time;

				if ((tscl >= clk_min) && (tscl <= clk_max) &&
				    (tscl_h >= i2c_specs[setup->speed].h_min) &&
				    (i2cclk < tscl_h)) {
					int clk_error = tscl - i2cbus;

					if (clk_error < 0)
						clk_error = -clk_error;

					if (clk_error < clk_error_prev) {
						clk_error_prev = clk_error;
						v->scll = l;
						v->sclh = h;
						sol_found = true;
						memcpy(s, v, sizeof(*s));
					}
				}
			}
		}
	}

	if (!sol_found) {
		pr_err("%s: no solution at all\n", __func__);
		ret = -EPERM;
	}

	return ret;
}

static int stm32_i2c_compute_timing(struct stm32_i2c_priv *i2c_priv,
				    struct stm32_i2c_setup *setup,
				    struct stm32_i2c_timings *output)
{
	struct stm32_i2c_timings *v, *_v;
	struct list_head solutions;
	int ret;

	if (setup->speed >= STM32_I2C_SPEED_END) {
		pr_err("%s: speed out of bound {%d/%d}\n", __func__,
		       setup->speed, STM32_I2C_SPEED_END - 1);
		return -EINVAL;
	}

	if ((setup->rise_time > i2c_specs[setup->speed].rise_max) ||
	    (setup->fall_time > i2c_specs[setup->speed].fall_max)) {
		pr_err("%s :timings out of bound Rise{%d>%d}/Fall{%d>%d}\n",
		       __func__,
		       setup->rise_time, i2c_specs[setup->speed].rise_max,
		       setup->fall_time, i2c_specs[setup->speed].fall_max);
		return -EINVAL;
	}

	if (setup->dnf > STM32_I2C_DNF_MAX) {
		pr_err("%s: DNF out of bound %d/%d\n", __func__,
		       setup->dnf, STM32_I2C_DNF_MAX);
		return -EINVAL;
	}

	if (setup->speed_freq > i2c_specs[setup->speed].rate) {
		pr_err("%s: Freq {%d/%d}\n", __func__,
		       setup->speed_freq, i2c_specs[setup->speed].rate);
		return -EINVAL;
	}

	INIT_LIST_HEAD(&solutions);
	ret = stm32_i2c_compute_solutions(setup, &solutions);
	if (ret)
		goto exit;

	ret = stm32_i2c_choose_solution(setup, &solutions, output);
	if (ret)
		goto exit;

	debug("%s: Presc: %i, scldel: %i, sdadel: %i, scll: %i, sclh: %i\n",
	      __func__, output->presc,
	      output->scldel, output->sdadel,
	      output->scll, output->sclh);

exit:
	/* Release list and memory */
	list_for_each_entry_safe(v, _v, &solutions, node) {
		list_del(&v->node);
		free(v);
	}

	return ret;
}

static int stm32_i2c_setup_timing(struct stm32_i2c_priv *i2c_priv,
				  struct stm32_i2c_timings *timing)
{
	struct stm32_i2c_setup *setup = i2c_priv->setup;
	int ret = 0;

	setup->speed = i2c_priv->speed;
	setup->speed_freq = i2c_specs[setup->speed].rate;
	setup->clock_src = clk_get_rate(&i2c_priv->clk);

	if (!setup->clock_src) {
		pr_err("%s: clock rate is 0\n", __func__);
		return -EINVAL;
	}

	do {
		ret = stm32_i2c_compute_timing(i2c_priv, setup, timing);
		if (ret) {
			debug("%s: failed to compute I2C timings.\n",
			      __func__);
			if (i2c_priv->speed > STM32_I2C_SPEED_STANDARD) {
				i2c_priv->speed--;
				setup->speed = i2c_priv->speed;
				setup->speed_freq =
					i2c_specs[setup->speed].rate;
				debug("%s: downgrade I2C Speed Freq to (%i)\n",
				      __func__, i2c_specs[setup->speed].rate);
			} else {
				break;
			}
		}
	} while (ret);

	if (ret) {
		pr_err("%s: impossible to compute I2C timings.\n", __func__);
		return ret;
	}

	debug("%s: I2C Speed(%i), Freq(%i), Clk Source(%i)\n", __func__,
	      setup->speed, setup->speed_freq, setup->clock_src);
	debug("%s: I2C Rise(%i) and Fall(%i) Time\n", __func__,
	      setup->rise_time, setup->fall_time);
	debug("%s: I2C Analog Filter(%s), DNF(%i)\n", __func__,
	      setup->analog_filter ? "On" : "Off", setup->dnf);

	return 0;
}

static int stm32_i2c_hw_config(struct stm32_i2c_priv *i2c_priv)
{
	struct stm32_i2c_regs *regs = i2c_priv->regs;
	struct stm32_i2c_timings t;
	int ret;
	u32 timing = 0;

	ret = stm32_i2c_setup_timing(i2c_priv, &t);
	if (ret)
		return ret;

	/* Disable I2C */
	clrbits_le32(&regs->cr1, STM32_I2C_CR1_PE);

	/* Timing settings */
	timing |= STM32_I2C_TIMINGR_PRESC(t.presc);
	timing |= STM32_I2C_TIMINGR_SCLDEL(t.scldel);
	timing |= STM32_I2C_TIMINGR_SDADEL(t.sdadel);
	timing |= STM32_I2C_TIMINGR_SCLH(t.sclh);
	timing |= STM32_I2C_TIMINGR_SCLL(t.scll);
	writel(timing, &regs->timingr);

	/* Enable I2C */
	if (i2c_priv->setup->analog_filter)
		clrbits_le32(&regs->cr1, STM32_I2C_CR1_ANFOFF);
	else
		setbits_le32(&regs->cr1, STM32_I2C_CR1_ANFOFF);
	setbits_le32(&regs->cr1, STM32_I2C_CR1_PE);

	return 0;
}

static int stm32_i2c_set_bus_speed(struct udevice *bus, unsigned int speed)
{
	struct stm32_i2c_priv *i2c_priv = dev_get_priv(bus);

	switch (speed) {
	case STANDARD_RATE:
		i2c_priv->speed = STM32_I2C_SPEED_STANDARD;
		break;
	case FAST_RATE:
		i2c_priv->speed = STM32_I2C_SPEED_FAST;
		break;
	case FAST_PLUS_RATE:
		i2c_priv->speed = STM32_I2C_SPEED_FAST_PLUS;
		break;
	default:
		debug("%s: Speed %d not supported\n", __func__, speed);
		return -EINVAL;
	}

	return stm32_i2c_hw_config(i2c_priv);
}

static int stm32_i2c_probe(struct udevice *dev)
{
	struct stm32_i2c_priv *i2c_priv = dev_get_priv(dev);
	struct reset_ctl reset_ctl;
	fdt_addr_t addr;
	int ret;

	addr = dev_read_addr(dev);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	i2c_priv->regs = (struct stm32_i2c_regs *)addr;

	ret = clk_get_by_index(dev, 0, &i2c_priv->clk);
	if (ret)
		return ret;

	ret = clk_enable(&i2c_priv->clk);
	if (ret)
		goto clk_free;

	ret = reset_get_by_index(dev, 0, &reset_ctl);
	if (ret)
		goto clk_disable;

	reset_assert(&reset_ctl);
	udelay(2);
	reset_deassert(&reset_ctl);

	return 0;

clk_disable:
	clk_disable(&i2c_priv->clk);
clk_free:
	clk_free(&i2c_priv->clk);

	return ret;
}

static int stm32_ofdata_to_platdata(struct udevice *dev)
{
	struct stm32_i2c_priv *i2c_priv = dev_get_priv(dev);
	u32 rise_time, fall_time;

	i2c_priv->setup = (struct stm32_i2c_setup *)dev_get_driver_data(dev);
	if (!i2c_priv->setup)
		return -EINVAL;

	rise_time = dev_read_u32_default(dev, "i2c-scl-rising-time-ns", 0);
	if (rise_time)
		i2c_priv->setup->rise_time = rise_time;

	fall_time = dev_read_u32_default(dev, "i2c-scl-falling-time-ns", 0);
	if (fall_time)
		i2c_priv->setup->fall_time = fall_time;

	return 0;
}

static const struct dm_i2c_ops stm32_i2c_ops = {
	.xfer = stm32_i2c_xfer,
	.set_bus_speed = stm32_i2c_set_bus_speed,
};

static const struct udevice_id stm32_i2c_of_match[] = {
	{ .compatible = "st,stm32f7-i2c", .data = (ulong)&stm32f7_setup },
	{}
};

U_BOOT_DRIVER(stm32f7_i2c) = {
	.name = "stm32f7-i2c",
	.id = UCLASS_I2C,
	.of_match = stm32_i2c_of_match,
	.ofdata_to_platdata = stm32_ofdata_to_platdata,
	.probe = stm32_i2c_probe,
	.priv_auto_alloc_size = sizeof(struct stm32_i2c_priv),
	.ops = &stm32_i2c_ops,
};
