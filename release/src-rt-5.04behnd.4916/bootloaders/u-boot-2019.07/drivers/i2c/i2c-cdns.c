// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Moritz Fischer <moritz.fischer@ettus.com>
 * IP from Cadence (ID T-CS-PE-0007-100, Version R1p10f2)
 *
 * This file is based on: drivers/i2c/zynq_i2c.c,
 * with added driver-model support and code cleanup.
 */

#include <common.h>
#include <dm.h>
#include <linux/types.h>
#include <linux/io.h>
#include <linux/errno.h>
#include <dm/root.h>
#include <i2c.h>
#include <fdtdec.h>
#include <mapmem.h>
#include <wait_bit.h>
#include <clk.h>

/* i2c register set */
struct cdns_i2c_regs {
	u32 control;
	u32 status;
	u32 address;
	u32 data;
	u32 interrupt_status;
	u32 transfer_size;
	u32 slave_mon_pause;
	u32 time_out;
	u32 interrupt_mask;
	u32 interrupt_enable;
	u32 interrupt_disable;
};

/* Control register fields */
#define CDNS_I2C_CONTROL_RW		0x00000001
#define CDNS_I2C_CONTROL_MS		0x00000002
#define CDNS_I2C_CONTROL_NEA		0x00000004
#define CDNS_I2C_CONTROL_ACKEN		0x00000008
#define CDNS_I2C_CONTROL_HOLD		0x00000010
#define CDNS_I2C_CONTROL_SLVMON		0x00000020
#define CDNS_I2C_CONTROL_CLR_FIFO	0x00000040
#define CDNS_I2C_CONTROL_DIV_B_SHIFT	8
#define CDNS_I2C_CONTROL_DIV_B_MASK	0x00003F00
#define CDNS_I2C_CONTROL_DIV_A_SHIFT	14
#define CDNS_I2C_CONTROL_DIV_A_MASK	0x0000C000

/* Status register values */
#define CDNS_I2C_STATUS_RXDV	0x00000020
#define CDNS_I2C_STATUS_TXDV	0x00000040
#define CDNS_I2C_STATUS_RXOVF	0x00000080
#define CDNS_I2C_STATUS_BA	0x00000100

/* Interrupt register fields */
#define CDNS_I2C_INTERRUPT_COMP		0x00000001
#define CDNS_I2C_INTERRUPT_DATA		0x00000002
#define CDNS_I2C_INTERRUPT_NACK		0x00000004
#define CDNS_I2C_INTERRUPT_TO		0x00000008
#define CDNS_I2C_INTERRUPT_SLVRDY	0x00000010
#define CDNS_I2C_INTERRUPT_RXOVF	0x00000020
#define CDNS_I2C_INTERRUPT_TXOVF	0x00000040
#define CDNS_I2C_INTERRUPT_RXUNF	0x00000080
#define CDNS_I2C_INTERRUPT_ARBLOST	0x00000200

#define CDNS_I2C_INTERRUPTS_MASK	(CDNS_I2C_INTERRUPT_COMP | \
					CDNS_I2C_INTERRUPT_DATA | \
					CDNS_I2C_INTERRUPT_NACK | \
					CDNS_I2C_INTERRUPT_TO | \
					CDNS_I2C_INTERRUPT_SLVRDY | \
					CDNS_I2C_INTERRUPT_RXOVF | \
					CDNS_I2C_INTERRUPT_TXOVF | \
					CDNS_I2C_INTERRUPT_RXUNF | \
					CDNS_I2C_INTERRUPT_ARBLOST)

#define CDNS_I2C_FIFO_DEPTH		16
#define CDNS_I2C_TRANSFER_SIZE_MAX	255 /* Controller transfer limit */
#define CDNS_I2C_TRANSFER_SIZE		(CDNS_I2C_TRANSFER_SIZE_MAX - 3)

#define CDNS_I2C_BROKEN_HOLD_BIT	BIT(0)

#define CDNS_I2C_ARB_LOST_MAX_RETRIES	10

#ifdef DEBUG
static void cdns_i2c_debug_status(struct cdns_i2c_regs *cdns_i2c)
{
	int int_status;
	int status;
	int_status = readl(&cdns_i2c->interrupt_status);

	status = readl(&cdns_i2c->status);
	if (int_status || status) {
		debug("Status: ");
		if (int_status & CDNS_I2C_INTERRUPT_COMP)
			debug("COMP ");
		if (int_status & CDNS_I2C_INTERRUPT_DATA)
			debug("DATA ");
		if (int_status & CDNS_I2C_INTERRUPT_NACK)
			debug("NACK ");
		if (int_status & CDNS_I2C_INTERRUPT_TO)
			debug("TO ");
		if (int_status & CDNS_I2C_INTERRUPT_SLVRDY)
			debug("SLVRDY ");
		if (int_status & CDNS_I2C_INTERRUPT_RXOVF)
			debug("RXOVF ");
		if (int_status & CDNS_I2C_INTERRUPT_TXOVF)
			debug("TXOVF ");
		if (int_status & CDNS_I2C_INTERRUPT_RXUNF)
			debug("RXUNF ");
		if (int_status & CDNS_I2C_INTERRUPT_ARBLOST)
			debug("ARBLOST ");
		if (status & CDNS_I2C_STATUS_RXDV)
			debug("RXDV ");
		if (status & CDNS_I2C_STATUS_TXDV)
			debug("TXDV ");
		if (status & CDNS_I2C_STATUS_RXOVF)
			debug("RXOVF ");
		if (status & CDNS_I2C_STATUS_BA)
			debug("BA ");
		debug("TS%d ", readl(&cdns_i2c->transfer_size));
		debug("\n");
	}
}
#endif

struct i2c_cdns_bus {
	int id;
	unsigned int input_freq;
	struct cdns_i2c_regs __iomem *regs;	/* register base */

	int hold_flag;
	u32 quirks;
};

struct cdns_i2c_platform_data {
	u32 quirks;
};

/* Wait for an interrupt */
static u32 cdns_i2c_wait(struct cdns_i2c_regs *cdns_i2c, u32 mask)
{
	int timeout, int_status;

	for (timeout = 0; timeout < 100; timeout++) {
		int_status = readl(&cdns_i2c->interrupt_status);
		if (int_status & mask)
			break;
		udelay(100);
	}

	/* Clear interrupt status flags */
	writel(int_status & mask, &cdns_i2c->interrupt_status);

	return int_status & mask;
}

#define CDNS_I2C_DIVA_MAX	4
#define CDNS_I2C_DIVB_MAX	64

static int cdns_i2c_calc_divs(unsigned long *f, unsigned long input_clk,
		unsigned int *a, unsigned int *b)
{
	unsigned long fscl = *f, best_fscl = *f, actual_fscl, temp;
	unsigned int div_a, div_b, calc_div_a = 0, calc_div_b = 0;
	unsigned int last_error, current_error;

	/* calculate (divisor_a+1) x (divisor_b+1) */
	temp = input_clk / (22 * fscl);

	/*
	 * If the calculated value is negative or 0CDNS_I2C_DIVA_MAX,
	 * the fscl input is out of range. Return error.
	 */
	if (!temp || (temp > (CDNS_I2C_DIVA_MAX * CDNS_I2C_DIVB_MAX)))
		return -EINVAL;

	last_error = -1;
	for (div_a = 0; div_a < CDNS_I2C_DIVA_MAX; div_a++) {
		div_b = DIV_ROUND_UP(input_clk, 22 * fscl * (div_a + 1));

		if ((div_b < 1) || (div_b > CDNS_I2C_DIVB_MAX))
			continue;
		div_b--;

		actual_fscl = input_clk / (22 * (div_a + 1) * (div_b + 1));

		if (actual_fscl > fscl)
			continue;

		current_error = ((actual_fscl > fscl) ? (actual_fscl - fscl) :
							(fscl - actual_fscl));

		if (last_error > current_error) {
			calc_div_a = div_a;
			calc_div_b = div_b;
			best_fscl = actual_fscl;
			last_error = current_error;
		}
	}

	*a = calc_div_a;
	*b = calc_div_b;
	*f = best_fscl;

	return 0;
}

static int cdns_i2c_set_bus_speed(struct udevice *dev, unsigned int speed)
{
	struct i2c_cdns_bus *bus = dev_get_priv(dev);
	u32 div_a = 0, div_b = 0;
	unsigned long speed_p = speed;
	int ret = 0;

	if (speed > 400000) {
		debug("%s, failed to set clock speed to %u\n", __func__,
		      speed);
		return -EINVAL;
	}

	ret = cdns_i2c_calc_divs(&speed_p, bus->input_freq, &div_a, &div_b);
	if (ret)
		return ret;

	debug("%s: div_a: %d, div_b: %d, input freq: %d, speed: %d/%ld\n",
	      __func__, div_a, div_b, bus->input_freq, speed, speed_p);

	writel((div_b << CDNS_I2C_CONTROL_DIV_B_SHIFT) |
	       (div_a << CDNS_I2C_CONTROL_DIV_A_SHIFT), &bus->regs->control);

	/* Enable master mode, ack, and 7-bit addressing */
	setbits_le32(&bus->regs->control, CDNS_I2C_CONTROL_MS |
		CDNS_I2C_CONTROL_ACKEN | CDNS_I2C_CONTROL_NEA);

	return 0;
}

static inline u32 is_arbitration_lost(struct cdns_i2c_regs *regs)
{
	return (readl(&regs->interrupt_status) & CDNS_I2C_INTERRUPT_ARBLOST);
}

static int cdns_i2c_write_data(struct i2c_cdns_bus *i2c_bus, u32 addr, u8 *data,
			       u32 len)
{
	u8 *cur_data = data;
	struct cdns_i2c_regs *regs = i2c_bus->regs;
	u32 ret;

	/* Set the controller in Master transmit mode and clear FIFO */
	setbits_le32(&regs->control, CDNS_I2C_CONTROL_CLR_FIFO);
	clrbits_le32(&regs->control, CDNS_I2C_CONTROL_RW);

	/* Check message size against FIFO depth, and set hold bus bit
	 * if it is greater than FIFO depth
	 */
	if (len > CDNS_I2C_FIFO_DEPTH)
		setbits_le32(&regs->control, CDNS_I2C_CONTROL_HOLD);

	/* Clear the interrupts in status register */
	writel(CDNS_I2C_INTERRUPTS_MASK, &regs->interrupt_status);

	writel(addr, &regs->address);

	while (len-- && !is_arbitration_lost(regs)) {
		writel(*(cur_data++), &regs->data);
		if (readl(&regs->transfer_size) == CDNS_I2C_FIFO_DEPTH) {
			ret = cdns_i2c_wait(regs, CDNS_I2C_INTERRUPT_COMP |
					    CDNS_I2C_INTERRUPT_ARBLOST);
			if (ret & CDNS_I2C_INTERRUPT_ARBLOST)
				return -EAGAIN;
			if (ret & CDNS_I2C_INTERRUPT_COMP)
				continue;
			/* Release the bus */
			clrbits_le32(&regs->control,
				     CDNS_I2C_CONTROL_HOLD);
			return -ETIMEDOUT;
		}
	}

	if (len && is_arbitration_lost(regs))
		return -EAGAIN;

	/* All done... release the bus */
	if (!i2c_bus->hold_flag)
		clrbits_le32(&regs->control, CDNS_I2C_CONTROL_HOLD);

	/* Wait for the address and data to be sent */
	ret = cdns_i2c_wait(regs, CDNS_I2C_INTERRUPT_COMP |
			    CDNS_I2C_INTERRUPT_ARBLOST);
	if (!(ret & (CDNS_I2C_INTERRUPT_ARBLOST |
		     CDNS_I2C_INTERRUPT_COMP)))
		return -ETIMEDOUT;
	if (ret & CDNS_I2C_INTERRUPT_ARBLOST)
		return -EAGAIN;

	return 0;
}

static inline bool cdns_is_hold_quirk(int hold_quirk, int curr_recv_count)
{
	return hold_quirk && (curr_recv_count == CDNS_I2C_FIFO_DEPTH + 1);
}

static int cdns_i2c_read_data(struct i2c_cdns_bus *i2c_bus, u32 addr, u8 *data,
			      u32 recv_count)
{
	u8 *cur_data = data;
	struct cdns_i2c_regs *regs = i2c_bus->regs;
	u32 curr_recv_count;
	int updatetx, hold_quirk;
	u32 ret;

	curr_recv_count = recv_count;

	/* Check for the message size against the FIFO depth */
	if (recv_count > CDNS_I2C_FIFO_DEPTH)
		setbits_le32(&regs->control, CDNS_I2C_CONTROL_HOLD);

	setbits_le32(&regs->control, CDNS_I2C_CONTROL_CLR_FIFO |
		CDNS_I2C_CONTROL_RW);

	if (recv_count > CDNS_I2C_TRANSFER_SIZE) {
		curr_recv_count = CDNS_I2C_TRANSFER_SIZE;
		writel(curr_recv_count, &regs->transfer_size);
	} else {
		writel(recv_count, &regs->transfer_size);
	}

	/* Start reading data */
	writel(addr, &regs->address);

	updatetx = recv_count > curr_recv_count;

	hold_quirk = (i2c_bus->quirks & CDNS_I2C_BROKEN_HOLD_BIT) && updatetx;

	while (recv_count && !is_arbitration_lost(regs)) {
		while (readl(&regs->status) & CDNS_I2C_STATUS_RXDV) {
			if (recv_count < CDNS_I2C_FIFO_DEPTH &&
			    !i2c_bus->hold_flag) {
				clrbits_le32(&regs->control,
					     CDNS_I2C_CONTROL_HOLD);
			}
			*(cur_data)++ = readl(&regs->data);
			recv_count--;
			curr_recv_count--;

			if (cdns_is_hold_quirk(hold_quirk, curr_recv_count))
				break;
		}

		if (cdns_is_hold_quirk(hold_quirk, curr_recv_count)) {
			/* wait while fifo is full */
			while (readl(&regs->transfer_size) !=
				     (curr_recv_count - CDNS_I2C_FIFO_DEPTH))
				;
			/*
			 * Check number of bytes to be received against maximum
			 * transfer size and update register accordingly.
			 */
			if ((recv_count - CDNS_I2C_FIFO_DEPTH) >
			    CDNS_I2C_TRANSFER_SIZE) {
				writel(CDNS_I2C_TRANSFER_SIZE,
				       &regs->transfer_size);
				curr_recv_count = CDNS_I2C_TRANSFER_SIZE +
					CDNS_I2C_FIFO_DEPTH;
			} else {
				writel(recv_count - CDNS_I2C_FIFO_DEPTH,
				       &regs->transfer_size);
				curr_recv_count = recv_count;
			}
		} else if (recv_count && !hold_quirk && !curr_recv_count) {
			writel(addr, &regs->address);
			if (recv_count > CDNS_I2C_TRANSFER_SIZE) {
				writel(CDNS_I2C_TRANSFER_SIZE,
				       &regs->transfer_size);
				curr_recv_count = CDNS_I2C_TRANSFER_SIZE;
			} else {
				writel(recv_count, &regs->transfer_size);
				curr_recv_count = recv_count;
			}
		}
	}

	/* Wait for the address and data to be sent */
	ret = cdns_i2c_wait(regs, CDNS_I2C_INTERRUPT_COMP |
			    CDNS_I2C_INTERRUPT_ARBLOST);
	if (!(ret & (CDNS_I2C_INTERRUPT_ARBLOST |
		     CDNS_I2C_INTERRUPT_COMP)))
		return -ETIMEDOUT;
	if (ret & CDNS_I2C_INTERRUPT_ARBLOST)
		return -EAGAIN;

	return 0;
}

static int cdns_i2c_xfer(struct udevice *dev, struct i2c_msg *msg,
			 int nmsgs)
{
	struct i2c_cdns_bus *i2c_bus = dev_get_priv(dev);
	int ret = 0;
	int count;
	bool hold_quirk;
	struct i2c_msg *message = msg;
	int num_msgs = nmsgs;

	hold_quirk = !!(i2c_bus->quirks & CDNS_I2C_BROKEN_HOLD_BIT);

	if (nmsgs > 1) {
		/*
		 * This controller does not give completion interrupt after a
		 * master receive message if HOLD bit is set (repeated start),
		 * resulting in SW timeout. Hence, if a receive message is
		 * followed by any other message, an error is returned
		 * indicating that this sequence is not supported.
		 */
		for (count = 0; (count < nmsgs - 1) && hold_quirk; count++) {
			if (msg[count].flags & I2C_M_RD) {
				printf("Can't do repeated start after a receive message\n");
				return -EOPNOTSUPP;
			}
		}

		i2c_bus->hold_flag = 1;
		setbits_le32(&i2c_bus->regs->control, CDNS_I2C_CONTROL_HOLD);
	} else {
		i2c_bus->hold_flag = 0;
	}

	debug("i2c_xfer: %d messages\n", nmsgs);
	for (u8 retry = 0; retry < CDNS_I2C_ARB_LOST_MAX_RETRIES &&
	     nmsgs > 0; nmsgs--, msg++) {
		debug("i2c_xfer: chip=0x%x, len=0x%x\n", msg->addr, msg->len);
		if (msg->flags & I2C_M_RD) {
			ret = cdns_i2c_read_data(i2c_bus, msg->addr, msg->buf,
						 msg->len);
		} else {
			ret = cdns_i2c_write_data(i2c_bus, msg->addr, msg->buf,
						  msg->len);
		}
		if (ret == -EAGAIN) {
			msg = message;
			nmsgs = num_msgs;
			retry++;
			printf("%s,arbitration lost, retrying:%d\n", __func__,
			       retry);
			continue;
		}

		if (ret) {
			debug("i2c_write: error sending\n");
			return -EREMOTEIO;
		}
	}

	return ret;
}

static int cdns_i2c_ofdata_to_platdata(struct udevice *dev)
{
	struct i2c_cdns_bus *i2c_bus = dev_get_priv(dev);
	struct cdns_i2c_platform_data *pdata =
		(struct cdns_i2c_platform_data *)dev_get_driver_data(dev);
	struct clk clk;
	int ret;

	i2c_bus->regs = (struct cdns_i2c_regs *)dev_read_addr(dev);
	if (!i2c_bus->regs)
		return -ENOMEM;

	if (pdata)
		i2c_bus->quirks = pdata->quirks;

	ret = clk_get_by_index(dev, 0, &clk);
	if (ret)
		return ret;

	i2c_bus->input_freq = clk_get_rate(&clk);

	return 0;
}

static const struct dm_i2c_ops cdns_i2c_ops = {
	.xfer = cdns_i2c_xfer,
	.set_bus_speed = cdns_i2c_set_bus_speed,
};

static const struct cdns_i2c_platform_data r1p10_i2c_def = {
	.quirks = CDNS_I2C_BROKEN_HOLD_BIT,
};

static const struct udevice_id cdns_i2c_of_match[] = {
	{ .compatible = "cdns,i2c-r1p10", .data = (ulong)&r1p10_i2c_def },
	{ .compatible = "cdns,i2c-r1p14" },
	{ /* end of table */ }
};

U_BOOT_DRIVER(cdns_i2c) = {
	.name = "i2c-cdns",
	.id = UCLASS_I2C,
	.of_match = cdns_i2c_of_match,
	.ofdata_to_platdata = cdns_i2c_ofdata_to_platdata,
	.priv_auto_alloc_size = sizeof(struct i2c_cdns_bus),
	.ops = &cdns_i2c_ops,
};
