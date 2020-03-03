/*
 * Driver for I2C adapter in Rockchip RK3xxx SoC
 *
 * Max Schwarz <max.schwarz@online.de>
 * based on the patches by Rockchip Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/spinlock.h>
#include <linux/clk.h>
#include <linux/wait.h>
#include <linux/mfd/syscon.h>
#include <linux/regmap.h>
#include <linux/math64.h>


/* Register Map */
#define REG_CON        0x00 /* control register */
#define REG_CLKDIV     0x04 /* clock divisor register */
#define REG_MRXADDR    0x08 /* slave address for REGISTER_TX */
#define REG_MRXRADDR   0x0c /* slave register address for REGISTER_TX */
#define REG_MTXCNT     0x10 /* number of bytes to be transmitted */
#define REG_MRXCNT     0x14 /* number of bytes to be received */
#define REG_IEN        0x18 /* interrupt enable */
#define REG_IPD        0x1c /* interrupt pending */
#define REG_FCNT       0x20 /* finished count */

/* Data buffer offsets */
#define TXBUFFER_BASE 0x100
#define RXBUFFER_BASE 0x200

/* REG_CON bits */
#define REG_CON_EN        BIT(0)
enum {
	REG_CON_MOD_TX = 0,      /* transmit data */
	REG_CON_MOD_REGISTER_TX, /* select register and restart */
	REG_CON_MOD_RX,          /* receive data */
	REG_CON_MOD_REGISTER_RX, /* broken: transmits read addr AND writes
				  * register addr */
};
#define REG_CON_MOD(mod)  ((mod) << 1)
#define REG_CON_MOD_MASK  (BIT(1) | BIT(2))
#define REG_CON_START     BIT(3)
#define REG_CON_STOP      BIT(4)
#define REG_CON_LASTACK   BIT(5) /* 1: send NACK after last received byte */
#define REG_CON_ACTACK    BIT(6) /* 1: stop if NACK is received */

/* REG_MRXADDR bits */
#define REG_MRXADDR_VALID(x) BIT(24 + (x)) /* [x*8+7:x*8] of MRX[R]ADDR valid */

/* REG_IEN/REG_IPD bits */
#define REG_INT_BTF       BIT(0) /* a byte was transmitted */
#define REG_INT_BRF       BIT(1) /* a byte was received */
#define REG_INT_MBTF      BIT(2) /* master data transmit finished */
#define REG_INT_MBRF      BIT(3) /* master data receive finished */
#define REG_INT_START     BIT(4) /* START condition generated */
#define REG_INT_STOP      BIT(5) /* STOP condition generated */
#define REG_INT_NAKRCV    BIT(6) /* NACK received */
#define REG_INT_ALL       0x7f

/* Constants */
#define WAIT_TIMEOUT      200 /* ms */
#define DEFAULT_SCL_RATE  (100 * 1000) /* Hz */

enum rk3x_i2c_state {
	STATE_IDLE,
	STATE_START,
	STATE_READ,
	STATE_WRITE,
	STATE_STOP
};

/**
 * @grf_offset: offset inside the grf regmap for setting the i2c type
 */
struct rk3x_i2c_soc_data {
	int grf_offset;
};

struct rk3x_i2c {
	struct i2c_adapter adap;
	struct device *dev;
	struct rk3x_i2c_soc_data *soc_data;

	/* Hardware resources */
	void __iomem *regs;
	struct clk *clk;
	struct notifier_block clk_rate_nb;

	/* Settings */
	unsigned int scl_frequency;
	unsigned int scl_rise_ns;
	unsigned int scl_fall_ns;
	unsigned int sda_fall_ns;

	/* Synchronization & notification */
	spinlock_t lock;
	wait_queue_head_t wait;
	bool busy;

	/* Current message */
	struct i2c_msg *msg;
	u8 addr;
	unsigned int mode;
	bool is_last_msg;

	/* I2C state machine */
	enum rk3x_i2c_state state;
	unsigned int processed; /* sent/received bytes */
	int error;
};

static inline void i2c_writel(struct rk3x_i2c *i2c, u32 value,
			      unsigned int offset)
{
	writel(value, i2c->regs + offset);
}

static inline u32 i2c_readl(struct rk3x_i2c *i2c, unsigned int offset)
{
	return readl(i2c->regs + offset);
}

/* Reset all interrupt pending bits */
static inline void rk3x_i2c_clean_ipd(struct rk3x_i2c *i2c)
{
	i2c_writel(i2c, REG_INT_ALL, REG_IPD);
}

/**
 * Generate a START condition, which triggers a REG_INT_START interrupt.
 */
static void rk3x_i2c_start(struct rk3x_i2c *i2c)
{
	u32 val;

	rk3x_i2c_clean_ipd(i2c);
	i2c_writel(i2c, REG_INT_START, REG_IEN);

	/* enable adapter with correct mode, send START condition */
	val = REG_CON_EN | REG_CON_MOD(i2c->mode) | REG_CON_START;

	/* if we want to react to NACK, set ACTACK bit */
	if (!(i2c->msg->flags & I2C_M_IGNORE_NAK))
		val |= REG_CON_ACTACK;

	i2c_writel(i2c, val, REG_CON);
}

/**
 * Generate a STOP condition, which triggers a REG_INT_STOP interrupt.
 *
 * @error: Error code to return in rk3x_i2c_xfer
 */
static void rk3x_i2c_stop(struct rk3x_i2c *i2c, int error)
{
	unsigned int ctrl;

	i2c->processed = 0;
	i2c->msg = NULL;
	i2c->error = error;

	if (i2c->is_last_msg) {
		/* Enable stop interrupt */
		i2c_writel(i2c, REG_INT_STOP, REG_IEN);

		i2c->state = STATE_STOP;

		ctrl = i2c_readl(i2c, REG_CON);
		ctrl |= REG_CON_STOP;
		i2c_writel(i2c, ctrl, REG_CON);
	} else {
		/* Signal rk3x_i2c_xfer to start the next message. */
		i2c->busy = false;
		i2c->state = STATE_IDLE;

		/*
		 * The HW is actually not capable of REPEATED START. But we can
		 * get the intended effect by resetting its internal state
		 * and issuing an ordinary START.
		 */
		i2c_writel(i2c, 0, REG_CON);

		/* signal that we are finished with the current msg */
		wake_up(&i2c->wait);
	}
}

/**
 * Setup a read according to i2c->msg
 */
static void rk3x_i2c_prepare_read(struct rk3x_i2c *i2c)
{
	unsigned int len = i2c->msg->len - i2c->processed;
	u32 con;

	con = i2c_readl(i2c, REG_CON);

	/*
	 * The hw can read up to 32 bytes at a time. If we need more than one
	 * chunk, send an ACK after the last byte of the current chunk.
	 */
	if (len > 32) {
		len = 32;
		con &= ~REG_CON_LASTACK;
	} else {
		con |= REG_CON_LASTACK;
	}

	/* make sure we are in plain RX mode if we read a second chunk */
	if (i2c->processed != 0) {
		con &= ~REG_CON_MOD_MASK;
		con |= REG_CON_MOD(REG_CON_MOD_RX);
	}

	i2c_writel(i2c, con, REG_CON);
	i2c_writel(i2c, len, REG_MRXCNT);
}

/**
 * Fill the transmit buffer with data from i2c->msg
 */
static void rk3x_i2c_fill_transmit_buf(struct rk3x_i2c *i2c)
{
	unsigned int i, j;
	u32 cnt = 0;
	u32 val;
	u8 byte;

	for (i = 0; i < 8; ++i) {
		val = 0;
		for (j = 0; j < 4; ++j) {
			if ((i2c->processed == i2c->msg->len) && (cnt != 0))
				break;

			if (i2c->processed == 0 && cnt == 0)
				byte = (i2c->addr & 0x7f) << 1;
			else
				byte = i2c->msg->buf[i2c->processed++];

			val |= byte << (j * 8);
			cnt++;
		}

		i2c_writel(i2c, val, TXBUFFER_BASE + 4 * i);

		if (i2c->processed == i2c->msg->len)
			break;
	}

	i2c_writel(i2c, cnt, REG_MTXCNT);
}


/* IRQ handlers for individual states */

static void rk3x_i2c_handle_start(struct rk3x_i2c *i2c, unsigned int ipd)
{
	if (!(ipd & REG_INT_START)) {
		rk3x_i2c_stop(i2c, -EIO);
		dev_warn(i2c->dev, "unexpected irq in START: 0x%x\n", ipd);
		rk3x_i2c_clean_ipd(i2c);
		return;
	}

	/* ack interrupt */
	i2c_writel(i2c, REG_INT_START, REG_IPD);

	/* disable start bit */
	i2c_writel(i2c, i2c_readl(i2c, REG_CON) & ~REG_CON_START, REG_CON);

	/* enable appropriate interrupts and transition */
	if (i2c->mode == REG_CON_MOD_TX) {
		i2c_writel(i2c, REG_INT_MBTF | REG_INT_NAKRCV, REG_IEN);
		i2c->state = STATE_WRITE;
		rk3x_i2c_fill_transmit_buf(i2c);
	} else {
		/* in any other case, we are going to be reading. */
		i2c_writel(i2c, REG_INT_MBRF | REG_INT_NAKRCV, REG_IEN);
		i2c->state = STATE_READ;
		rk3x_i2c_prepare_read(i2c);
	}
}

static void rk3x_i2c_handle_write(struct rk3x_i2c *i2c, unsigned int ipd)
{
	if (!(ipd & REG_INT_MBTF)) {
		rk3x_i2c_stop(i2c, -EIO);
		dev_err(i2c->dev, "unexpected irq in WRITE: 0x%x\n", ipd);
		rk3x_i2c_clean_ipd(i2c);
		return;
	}

	/* ack interrupt */
	i2c_writel(i2c, REG_INT_MBTF, REG_IPD);

	/* are we finished? */
	if (i2c->processed == i2c->msg->len)
		rk3x_i2c_stop(i2c, i2c->error);
	else
		rk3x_i2c_fill_transmit_buf(i2c);
}

static void rk3x_i2c_handle_read(struct rk3x_i2c *i2c, unsigned int ipd)
{
	unsigned int i;
	unsigned int len = i2c->msg->len - i2c->processed;
	u32 uninitialized_var(val);
	u8 byte;

	/* we only care for MBRF here. */
	if (!(ipd & REG_INT_MBRF))
		return;

	/* ack interrupt */
	i2c_writel(i2c, REG_INT_MBRF, REG_IPD);

	/* Can only handle a maximum of 32 bytes at a time */
	if (len > 32)
		len = 32;

	/* read the data from receive buffer */
	for (i = 0; i < len; ++i) {
		if (i % 4 == 0)
			val = i2c_readl(i2c, RXBUFFER_BASE + (i / 4) * 4);

		byte = (val >> ((i % 4) * 8)) & 0xff;
		i2c->msg->buf[i2c->processed++] = byte;
	}

	/* are we finished? */
	if (i2c->processed == i2c->msg->len)
		rk3x_i2c_stop(i2c, i2c->error);
	else
		rk3x_i2c_prepare_read(i2c);
}

static void rk3x_i2c_handle_stop(struct rk3x_i2c *i2c, unsigned int ipd)
{
	unsigned int con;

	if (!(ipd & REG_INT_STOP)) {
		rk3x_i2c_stop(i2c, -EIO);
		dev_err(i2c->dev, "unexpected irq in STOP: 0x%x\n", ipd);
		rk3x_i2c_clean_ipd(i2c);
		return;
	}

	/* ack interrupt */
	i2c_writel(i2c, REG_INT_STOP, REG_IPD);

	/* disable STOP bit */
	con = i2c_readl(i2c, REG_CON);
	con &= ~REG_CON_STOP;
	i2c_writel(i2c, con, REG_CON);

	i2c->busy = false;
	i2c->state = STATE_IDLE;

	/* signal rk3x_i2c_xfer that we are finished */
	wake_up(&i2c->wait);
}

static irqreturn_t rk3x_i2c_irq(int irqno, void *dev_id)
{
	struct rk3x_i2c *i2c = dev_id;
	unsigned int ipd;

	spin_lock(&i2c->lock);

	ipd = i2c_readl(i2c, REG_IPD);
	if (i2c->state == STATE_IDLE) {
		dev_warn(i2c->dev, "irq in STATE_IDLE, ipd = 0x%x\n", ipd);
		rk3x_i2c_clean_ipd(i2c);
		goto out;
	}

	dev_dbg(i2c->dev, "IRQ: state %d, ipd: %x\n", i2c->state, ipd);

	/* Clean interrupt bits we don't care about */
	ipd &= ~(REG_INT_BRF | REG_INT_BTF);

	if (ipd & REG_INT_NAKRCV) {
		/*
		 * We got a NACK in the last operation. Depending on whether
		 * IGNORE_NAK is set, we have to stop the operation and report
		 * an error.
		 */
		i2c_writel(i2c, REG_INT_NAKRCV, REG_IPD);

		ipd &= ~REG_INT_NAKRCV;

		if (!(i2c->msg->flags & I2C_M_IGNORE_NAK))
			rk3x_i2c_stop(i2c, -ENXIO);
	}

	/* is there anything left to handle? */
	if ((ipd & REG_INT_ALL) == 0)
		goto out;

	switch (i2c->state) {
	case STATE_START:
		rk3x_i2c_handle_start(i2c, ipd);
		break;
	case STATE_WRITE:
		rk3x_i2c_handle_write(i2c, ipd);
		break;
	case STATE_READ:
		rk3x_i2c_handle_read(i2c, ipd);
		break;
	case STATE_STOP:
		rk3x_i2c_handle_stop(i2c, ipd);
		break;
	case STATE_IDLE:
		break;
	}

out:
	spin_unlock(&i2c->lock);
	return IRQ_HANDLED;
}

/**
 * Calculate divider values for desired SCL frequency
 *
 * @clk_rate: I2C input clock rate
 * @scl_rate: Desired SCL rate
 * @scl_rise_ns: How many ns it takes for SCL to rise.
 * @scl_fall_ns: How many ns it takes for SCL to fall.
 * @sda_fall_ns: How many ns it takes for SDA to fall.
 * @div_low: Divider output for low
 * @div_high: Divider output for high
 *
 * Returns: 0 on success, -EINVAL if the goal SCL rate is too slow. In that case
 * a best-effort divider value is returned in divs. If the target rate is
 * too high, we silently use the highest possible rate.
 */
static int rk3x_i2c_calc_divs(unsigned long clk_rate, unsigned long scl_rate,
			      unsigned long scl_rise_ns,
			      unsigned long scl_fall_ns,
			      unsigned long sda_fall_ns,
			      unsigned long *div_low, unsigned long *div_high)
{
	unsigned long spec_min_low_ns, spec_min_high_ns;
	unsigned long spec_setup_start, spec_max_data_hold_ns;
	unsigned long data_hold_buffer_ns;

	unsigned long min_low_ns, min_high_ns;
	unsigned long max_low_ns, min_total_ns;

	unsigned long clk_rate_khz, scl_rate_khz;

	unsigned long min_low_div, min_high_div;
	unsigned long max_low_div;

	unsigned long min_div_for_hold, min_total_div;
	unsigned long extra_div, extra_low_div, ideal_low_div;

	int ret = 0;

	/* Only support standard-mode and fast-mode */
	if (WARN_ON(scl_rate > 400000))
		scl_rate = 400000;

	/* prevent scl_rate_khz from becoming 0 */
	if (WARN_ON(scl_rate < 1000))
		scl_rate = 1000;

	/*
	 * min_low_ns:  The minimum number of ns we need to hold low to
	 *		meet I2C specification, should include fall time.
	 * min_high_ns: The minimum number of ns we need to hold high to
	 *		meet I2C specification, should include rise time.
	 * max_low_ns:  The maximum number of ns we can hold low to meet
	 *		I2C specification.
	 *
	 * Note: max_low_ns should be (maximum data hold time * 2 - buffer)
	 *	 This is because the i2c host on Rockchip holds the data line
	 *	 for half the low time.
	 */
	if (scl_rate <= 100000) {
		/* Standard-mode */
		spec_min_low_ns = 4700;
		spec_setup_start = 4700;
		spec_min_high_ns = 4000;
		spec_max_data_hold_ns = 3450;
		data_hold_buffer_ns = 50;
	} else {
		/* Fast-mode */
		spec_min_low_ns = 1300;
		spec_setup_start = 600;
		spec_min_high_ns = 600;
		spec_max_data_hold_ns = 900;
		data_hold_buffer_ns = 50;
	}
	min_high_ns = scl_rise_ns + spec_min_high_ns;

	/*
	 * Timings for repeated start:
	 * - controller appears to drop SDA at .875x (7/8) programmed clk high.
	 * - controller appears to keep SCL high for 2x programmed clk high.
	 *
	 * We need to account for those rules in picking our "high" time so
	 * we meet tSU;STA and tHD;STA times.
	 */
	min_high_ns = max(min_high_ns,
		DIV_ROUND_UP((scl_rise_ns + spec_setup_start) * 1000, 875));
	min_high_ns = max(min_high_ns,
		DIV_ROUND_UP((scl_rise_ns + spec_setup_start +
			      sda_fall_ns + spec_min_high_ns), 2));

	min_low_ns = scl_fall_ns + spec_min_low_ns;
	max_low_ns = spec_max_data_hold_ns * 2 - data_hold_buffer_ns;
	min_total_ns = min_low_ns + min_high_ns;

	/* Adjust to avoid overflow */
	clk_rate_khz = DIV_ROUND_UP(clk_rate, 1000);
	scl_rate_khz = scl_rate / 1000;

	/*
	 * We need the total div to be >= this number
	 * so we don't clock too fast.
	 */
	min_total_div = DIV_ROUND_UP(clk_rate_khz, scl_rate_khz * 8);

	/* These are the min dividers needed for min hold times. */
	min_low_div = DIV_ROUND_UP(clk_rate_khz * min_low_ns, 8 * 1000000);
	min_high_div = DIV_ROUND_UP(clk_rate_khz * min_high_ns, 8 * 1000000);
	min_div_for_hold = (min_low_div + min_high_div);

	/*
	 * This is the maximum divider so we don't go over the maximum.
	 * We don't round up here (we round down) since this is a maximum.
	 */
	max_low_div = clk_rate_khz * max_low_ns / (8 * 1000000);

	if (min_low_div > max_low_div) {
		WARN_ONCE(true,
			  "Conflicting, min_low_div %lu, max_low_div %lu\n",
			  min_low_div, max_low_div);
		max_low_div = min_low_div;
	}

	if (min_div_for_hold > min_total_div) {
		/*
		 * Time needed to meet hold requirements is important.
		 * Just use that.
		 */
		*div_low = min_low_div;
		*div_high = min_high_div;
	} else {
		/*
		 * We've got to distribute some time among the low and high
		 * so we don't run too fast.
		 */
		extra_div = min_total_div - min_div_for_hold;

		/*
		 * We'll try to split things up perfectly evenly,
		 * biasing slightly towards having a higher div
		 * for low (spend more time low).
		 */
		ideal_low_div = DIV_ROUND_UP(clk_rate_khz * min_low_ns,
					     scl_rate_khz * 8 * min_total_ns);

		/* Don't allow it to go over the maximum */
		if (ideal_low_div > max_low_div)
			ideal_low_div = max_low_div;

		/*
		 * Handle when the ideal low div is going to take up
		 * more than we have.
		 */
		if (ideal_low_div > min_low_div + extra_div)
			ideal_low_div = min_low_div + extra_div;

		/* Give low the "ideal" and give high whatever extra is left */
		extra_low_div = ideal_low_div - min_low_div;
		*div_low = ideal_low_div;
		*div_high = min_high_div + (extra_div - extra_low_div);
	}

	/*
	 * Adjust to the fact that the hardware has an implicit "+1".
	 * NOTE: Above calculations always produce div_low > 0 and div_high > 0.
	 */
	*div_low = *div_low - 1;
	*div_high = *div_high - 1;

	/* Maximum divider supported by hw is 0xffff */
	if (*div_low > 0xffff) {
		*div_low = 0xffff;
		ret = -EINVAL;
	}

	if (*div_high > 0xffff) {
		*div_high = 0xffff;
		ret = -EINVAL;
	}

	return ret;
}

static void rk3x_i2c_adapt_div(struct rk3x_i2c *i2c, unsigned long clk_rate)
{
	unsigned long div_low, div_high;
	u64 t_low_ns, t_high_ns;
	int ret;

	ret = rk3x_i2c_calc_divs(clk_rate, i2c->scl_frequency, i2c->scl_rise_ns,
				 i2c->scl_fall_ns, i2c->sda_fall_ns,
				 &div_low, &div_high);
	WARN_ONCE(ret != 0, "Could not reach SCL freq %u", i2c->scl_frequency);

	clk_enable(i2c->clk);
	i2c_writel(i2c, (div_high << 16) | (div_low & 0xffff), REG_CLKDIV);
	clk_disable(i2c->clk);

	t_low_ns = div_u64(((u64)div_low + 1) * 8 * 1000000000, clk_rate);
	t_high_ns = div_u64(((u64)div_high + 1) * 8 * 1000000000, clk_rate);
	dev_dbg(i2c->dev,
		"CLK %lukhz, Req %uns, Act low %lluns high %lluns\n",
		clk_rate / 1000,
		1000000000 / i2c->scl_frequency,
		t_low_ns, t_high_ns);
}

/**
 * rk3x_i2c_clk_notifier_cb - Clock rate change callback
 * @nb:		Pointer to notifier block
 * @event:	Notification reason
 * @data:	Pointer to notification data object
 *
 * The callback checks whether a valid bus frequency can be generated after the
 * change. If so, the change is acknowledged, otherwise the change is aborted.
 * New dividers are written to the HW in the pre- or post change notification
 * depending on the scaling direction.
 *
 * Code adapted from i2c-cadence.c.
 *
 * Return:	NOTIFY_STOP if the rate change should be aborted, NOTIFY_OK
 *		to acknowedge the change, NOTIFY_DONE if the notification is
 *		considered irrelevant.
 */
static int rk3x_i2c_clk_notifier_cb(struct notifier_block *nb, unsigned long
				    event, void *data)
{
	struct clk_notifier_data *ndata = data;
	struct rk3x_i2c *i2c = container_of(nb, struct rk3x_i2c, clk_rate_nb);
	unsigned long div_low, div_high;

	switch (event) {
	case PRE_RATE_CHANGE:
		if (rk3x_i2c_calc_divs(ndata->new_rate, i2c->scl_frequency,
				       i2c->scl_rise_ns, i2c->scl_fall_ns,
				       i2c->sda_fall_ns,
				       &div_low, &div_high) != 0)
			return NOTIFY_STOP;

		/* scale up */
		if (ndata->new_rate > ndata->old_rate)
			rk3x_i2c_adapt_div(i2c, ndata->new_rate);

		return NOTIFY_OK;
	case POST_RATE_CHANGE:
		/* scale down */
		if (ndata->new_rate < ndata->old_rate)
			rk3x_i2c_adapt_div(i2c, ndata->new_rate);
		return NOTIFY_OK;
	case ABORT_RATE_CHANGE:
		/* scale up */
		if (ndata->new_rate > ndata->old_rate)
			rk3x_i2c_adapt_div(i2c, ndata->old_rate);
		return NOTIFY_OK;
	default:
		return NOTIFY_DONE;
	}
}

/**
 * Setup I2C registers for an I2C operation specified by msgs, num.
 *
 * Must be called with i2c->lock held.
 *
 * @msgs: I2C msgs to process
 * @num: Number of msgs
 *
 * returns: Number of I2C msgs processed or negative in case of error
 */
static int rk3x_i2c_setup(struct rk3x_i2c *i2c, struct i2c_msg *msgs, int num)
{
	u32 addr = (msgs[0].addr & 0x7f) << 1;
	int ret = 0;

	/*
	 * The I2C adapter can issue a small (len < 4) write packet before
	 * reading. This speeds up SMBus-style register reads.
	 * The MRXADDR/MRXRADDR hold the slave address and the slave register
	 * address in this case.
	 */

	if (num >= 2 && msgs[0].len < 4 &&
	    !(msgs[0].flags & I2C_M_RD) && (msgs[1].flags & I2C_M_RD)) {
		u32 reg_addr = 0;
		int i;

		dev_dbg(i2c->dev, "Combined write/read from addr 0x%x\n",
			addr >> 1);

		/* Fill MRXRADDR with the register address(es) */
		for (i = 0; i < msgs[0].len; ++i) {
			reg_addr |= msgs[0].buf[i] << (i * 8);
			reg_addr |= REG_MRXADDR_VALID(i);
		}

		/* msgs[0] is handled by hw. */
		i2c->msg = &msgs[1];

		i2c->mode = REG_CON_MOD_REGISTER_TX;

		i2c_writel(i2c, addr | REG_MRXADDR_VALID(0), REG_MRXADDR);
		i2c_writel(i2c, reg_addr, REG_MRXRADDR);

		ret = 2;
	} else {
		/*
		 * We'll have to do it the boring way and process the msgs
		 * one-by-one.
		 */

		if (msgs[0].flags & I2C_M_RD) {
			addr |= 1; /* set read bit */

			/*
			 * We have to transmit the slave addr first. Use
			 * MOD_REGISTER_TX for that purpose.
			 */
			i2c->mode = REG_CON_MOD_REGISTER_TX;
			i2c_writel(i2c, addr | REG_MRXADDR_VALID(0),
				   REG_MRXADDR);
			i2c_writel(i2c, 0, REG_MRXRADDR);
		} else {
			i2c->mode = REG_CON_MOD_TX;
		}

		i2c->msg = &msgs[0];

		ret = 1;
	}

	i2c->addr = msgs[0].addr;
	i2c->busy = true;
	i2c->state = STATE_START;
	i2c->processed = 0;
	i2c->error = 0;

	rk3x_i2c_clean_ipd(i2c);

	return ret;
}

static int rk3x_i2c_xfer(struct i2c_adapter *adap,
			 struct i2c_msg *msgs, int num)
{
	struct rk3x_i2c *i2c = (struct rk3x_i2c *)adap->algo_data;
	unsigned long timeout, flags;
	int ret = 0;
	int i;

	spin_lock_irqsave(&i2c->lock, flags);

	clk_enable(i2c->clk);

	i2c->is_last_msg = false;

	/*
	 * Process msgs. We can handle more than one message at once (see
	 * rk3x_i2c_setup()).
	 */
	for (i = 0; i < num; i += ret) {
		ret = rk3x_i2c_setup(i2c, msgs + i, num - i);

		if (ret < 0) {
			dev_err(i2c->dev, "rk3x_i2c_setup() failed\n");
			break;
		}

		if (i + ret >= num)
			i2c->is_last_msg = true;

		spin_unlock_irqrestore(&i2c->lock, flags);

		rk3x_i2c_start(i2c);

		timeout = wait_event_timeout(i2c->wait, !i2c->busy,
					     msecs_to_jiffies(WAIT_TIMEOUT));

		spin_lock_irqsave(&i2c->lock, flags);

		if (timeout == 0) {
			dev_err(i2c->dev, "timeout, ipd: 0x%02x, state: %d\n",
				i2c_readl(i2c, REG_IPD), i2c->state);

			/* Force a STOP condition without interrupt */
			i2c_writel(i2c, 0, REG_IEN);
			i2c_writel(i2c, REG_CON_EN | REG_CON_STOP, REG_CON);

			i2c->state = STATE_IDLE;

			ret = -ETIMEDOUT;
			break;
		}

		if (i2c->error) {
			ret = i2c->error;
			break;
		}
	}

	clk_disable(i2c->clk);
	spin_unlock_irqrestore(&i2c->lock, flags);

	return ret < 0 ? ret : num;
}

static u32 rk3x_i2c_func(struct i2c_adapter *adap)
{
	return I2C_FUNC_I2C | I2C_FUNC_SMBUS_EMUL | I2C_FUNC_PROTOCOL_MANGLING;
}

static const struct i2c_algorithm rk3x_i2c_algorithm = {
	.master_xfer		= rk3x_i2c_xfer,
	.functionality		= rk3x_i2c_func,
};

static struct rk3x_i2c_soc_data soc_data[3] = {
	{ .grf_offset = 0x154 }, /* rk3066 */
	{ .grf_offset = 0x0a4 }, /* rk3188 */
	{ .grf_offset = -1 },    /* no I2C switching needed */
};

static const struct of_device_id rk3x_i2c_match[] = {
	{ .compatible = "rockchip,rk3066-i2c", .data = (void *)&soc_data[0] },
	{ .compatible = "rockchip,rk3188-i2c", .data = (void *)&soc_data[1] },
	{ .compatible = "rockchip,rk3288-i2c", .data = (void *)&soc_data[2] },
	{},
};

static int rk3x_i2c_probe(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	const struct of_device_id *match;
	struct rk3x_i2c *i2c;
	struct resource *mem;
	int ret = 0;
	int bus_nr;
	u32 value;
	int irq;
	unsigned long clk_rate;

	i2c = devm_kzalloc(&pdev->dev, sizeof(struct rk3x_i2c), GFP_KERNEL);
	if (!i2c)
		return -ENOMEM;

	match = of_match_node(rk3x_i2c_match, np);
	i2c->soc_data = (struct rk3x_i2c_soc_data *)match->data;

	if (of_property_read_u32(pdev->dev.of_node, "clock-frequency",
				 &i2c->scl_frequency)) {
		dev_info(&pdev->dev, "using default SCL frequency: %d\n",
			 DEFAULT_SCL_RATE);
		i2c->scl_frequency = DEFAULT_SCL_RATE;
	}

	if (i2c->scl_frequency == 0 || i2c->scl_frequency > 400 * 1000) {
		dev_warn(&pdev->dev, "invalid SCL frequency specified.\n");
		dev_warn(&pdev->dev, "using default SCL frequency: %d\n",
			 DEFAULT_SCL_RATE);
		i2c->scl_frequency = DEFAULT_SCL_RATE;
	}

	/*
	 * Read rise and fall time from device tree. If not available use
	 * the default maximum timing from the specification.
	 */
	if (of_property_read_u32(pdev->dev.of_node, "i2c-scl-rising-time-ns",
				 &i2c->scl_rise_ns)) {
		if (i2c->scl_frequency <= 100000)
			i2c->scl_rise_ns = 1000;
		else
			i2c->scl_rise_ns = 300;
	}
	if (of_property_read_u32(pdev->dev.of_node, "i2c-scl-falling-time-ns",
				 &i2c->scl_fall_ns))
		i2c->scl_fall_ns = 300;
	if (of_property_read_u32(pdev->dev.of_node, "i2c-sda-falling-time-ns",
				 &i2c->scl_fall_ns))
		i2c->sda_fall_ns = i2c->scl_fall_ns;

	strlcpy(i2c->adap.name, "rk3x-i2c", sizeof(i2c->adap.name));
	i2c->adap.owner = THIS_MODULE;
	i2c->adap.algo = &rk3x_i2c_algorithm;
	i2c->adap.retries = 3;
	i2c->adap.dev.of_node = np;
	i2c->adap.algo_data = i2c;
	i2c->adap.dev.parent = &pdev->dev;

	i2c->dev = &pdev->dev;

	spin_lock_init(&i2c->lock);
	init_waitqueue_head(&i2c->wait);

	i2c->clk = devm_clk_get(&pdev->dev, NULL);
	if (IS_ERR(i2c->clk)) {
		dev_err(&pdev->dev, "cannot get clock\n");
		return PTR_ERR(i2c->clk);
	}

	mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	i2c->regs = devm_ioremap_resource(&pdev->dev, mem);
	if (IS_ERR(i2c->regs))
		return PTR_ERR(i2c->regs);

	/* Try to set the I2C adapter number from dt */
	bus_nr = of_alias_get_id(np, "i2c");

	/*
	 * Switch to new interface if the SoC also offers the old one.
	 * The control bit is located in the GRF register space.
	 */
	if (i2c->soc_data->grf_offset >= 0) {
		struct regmap *grf;

		grf = syscon_regmap_lookup_by_phandle(np, "rockchip,grf");
		if (IS_ERR(grf)) {
			dev_err(&pdev->dev,
				"rk3x-i2c needs 'rockchip,grf' property\n");
			return PTR_ERR(grf);
		}

		if (bus_nr < 0) {
			dev_err(&pdev->dev, "rk3x-i2c needs i2cX alias");
			return -EINVAL;
		}

		/* 27+i: write mask, 11+i: value */
		value = BIT(27 + bus_nr) | BIT(11 + bus_nr);

		ret = regmap_write(grf, i2c->soc_data->grf_offset, value);
		if (ret != 0) {
			dev_err(i2c->dev, "Could not write to GRF: %d\n", ret);
			return ret;
		}
	}

	/* IRQ setup */
	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
		dev_err(&pdev->dev, "cannot find rk3x IRQ\n");
		return irq;
	}

	ret = devm_request_irq(&pdev->dev, irq, rk3x_i2c_irq,
			       0, dev_name(&pdev->dev), i2c);
	if (ret < 0) {
		dev_err(&pdev->dev, "cannot request IRQ\n");
		return ret;
	}

	platform_set_drvdata(pdev, i2c);

	ret = clk_prepare(i2c->clk);
	if (ret < 0) {
		dev_err(&pdev->dev, "Could not prepare clock\n");
		return ret;
	}

	i2c->clk_rate_nb.notifier_call = rk3x_i2c_clk_notifier_cb;
	ret = clk_notifier_register(i2c->clk, &i2c->clk_rate_nb);
	if (ret != 0) {
		dev_err(&pdev->dev, "Unable to register clock notifier\n");
		goto err_clk;
	}

	clk_rate = clk_get_rate(i2c->clk);
	rk3x_i2c_adapt_div(i2c, clk_rate);

	ret = i2c_add_adapter(&i2c->adap);
	if (ret < 0) {
		dev_err(&pdev->dev, "Could not register adapter\n");
		goto err_clk_notifier;
	}

	dev_info(&pdev->dev, "Initialized RK3xxx I2C bus at %p\n", i2c->regs);

	return 0;

err_clk_notifier:
	clk_notifier_unregister(i2c->clk, &i2c->clk_rate_nb);
err_clk:
	clk_unprepare(i2c->clk);
	return ret;
}

static int rk3x_i2c_remove(struct platform_device *pdev)
{
	struct rk3x_i2c *i2c = platform_get_drvdata(pdev);

	i2c_del_adapter(&i2c->adap);

	clk_notifier_unregister(i2c->clk, &i2c->clk_rate_nb);
	clk_unprepare(i2c->clk);

	return 0;
}

static struct platform_driver rk3x_i2c_driver = {
	.probe   = rk3x_i2c_probe,
	.remove  = rk3x_i2c_remove,
	.driver  = {
		.name  = "rk3x-i2c",
		.of_match_table = rk3x_i2c_match,
	},
};

module_platform_driver(rk3x_i2c_driver);

MODULE_DESCRIPTION("Rockchip RK3xxx I2C Bus driver");
MODULE_AUTHOR("Max Schwarz <max.schwarz@online.de>");
MODULE_LICENSE("GPL v2");
