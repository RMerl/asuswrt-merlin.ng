// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
 * Copyright (c) 2010-2011 NVIDIA Corporation
 *  NVIDIA Corporation <www.nvidia.com>
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <i2c.h>
#include <asm/io.h>
#include <clk.h>
#include <reset.h>
#ifndef CONFIG_TEGRA186
#include <asm/arch/clock.h>
#include <asm/arch/funcmux.h>
#endif
#include <asm/arch/gpio.h>
#include <asm/arch-tegra/tegra_i2c.h>

enum i2c_type {
	TYPE_114,
	TYPE_STD,
	TYPE_DVC,
};

/* Information about i2c controller */
struct i2c_bus {
	int			id;
	struct reset_ctl	reset_ctl;
	struct clk		clk;
	int			speed;
	int			pinmux_config;
	struct i2c_control	*control;
	struct i2c_ctlr		*regs;
	enum i2c_type		type;
	int			inited;	/* bus is inited */
};

static void set_packet_mode(struct i2c_bus *i2c_bus)
{
	u32 config;

	config = I2C_CNFG_NEW_MASTER_FSM_MASK | I2C_CNFG_PACKET_MODE_MASK;

	if (i2c_bus->type == TYPE_DVC) {
		struct dvc_ctlr *dvc = (struct dvc_ctlr *)i2c_bus->regs;

		writel(config, &dvc->cnfg);
	} else {
		writel(config, &i2c_bus->regs->cnfg);
		/*
		 * program I2C_SL_CNFG.NEWSL to ENABLE. This fixes probe
		 * issues, i.e., some slaves may be wrongly detected.
		 */
		setbits_le32(&i2c_bus->regs->sl_cnfg, I2C_SL_CNFG_NEWSL_MASK);
	}
}

static void i2c_reset_controller(struct i2c_bus *i2c_bus)
{
	/* Reset I2C controller. */
	reset_assert(&i2c_bus->reset_ctl);
	udelay(1);
	reset_deassert(&i2c_bus->reset_ctl);
	udelay(1);

	/* re-program config register to packet mode */
	set_packet_mode(i2c_bus);
}

static int i2c_init_clock(struct i2c_bus *i2c_bus, unsigned rate)
{
	int ret;

	ret = reset_assert(&i2c_bus->reset_ctl);
	if (ret)
		return ret;
	ret = clk_enable(&i2c_bus->clk);
	if (ret)
		return ret;
	ret = clk_set_rate(&i2c_bus->clk, rate);
	if (IS_ERR_VALUE(ret))
		return ret;
	ret = reset_deassert(&i2c_bus->reset_ctl);
	if (ret)
		return ret;

	return 0;
}

static void i2c_init_controller(struct i2c_bus *i2c_bus)
{
	if (!i2c_bus->speed)
		return;
	debug("%s: speed=%d\n", __func__, i2c_bus->speed);
	/*
	 * Use PLLP - DP-04508-001_v06 datasheet indicates a divisor of 8
	 * here, in section 23.3.1, but in fact we seem to need a factor of
	 * 16 to get the right frequency.
	 */
	i2c_init_clock(i2c_bus, i2c_bus->speed * 2 * 8);

	if (i2c_bus->type == TYPE_114) {
		/*
		 * T114 I2C went to a single clock source for standard/fast and
		 * HS clock speeds. The new clock rate setting calculation is:
		 *  SCL = CLK_SOURCE.I2C /
		 *   (CLK_MULT_STD_FAST_MODE * (I2C_CLK_DIV_STD_FAST_MODE+1) *
		 *   I2C FREQUENCY DIVISOR) as per the T114 TRM (sec 30.3.1).
		 *
		 * NOTE: We do this here, after the initial clock/pll start,
		 * because if we read the clk_div reg before the controller
		 * is running, we hang, and we need it for the new calc.
		 */
		int clk_div_stdfst_mode = readl(&i2c_bus->regs->clk_div) >> 16;
		unsigned rate = CLK_MULT_STD_FAST_MODE *
				(clk_div_stdfst_mode + 1) * i2c_bus->speed * 2;
		debug("%s: CLK_DIV_STD_FAST_MODE setting = %d\n", __func__,
			clk_div_stdfst_mode);

		i2c_init_clock(i2c_bus, rate);
	}

	/* Reset I2C controller. */
	i2c_reset_controller(i2c_bus);

	/* Configure I2C controller. */
	if (i2c_bus->type == TYPE_DVC) {	/* only for DVC I2C */
		struct dvc_ctlr *dvc = (struct dvc_ctlr *)i2c_bus->regs;

		setbits_le32(&dvc->ctrl3, DVC_CTRL_REG3_I2C_HW_SW_PROG_MASK);
	}

#ifndef CONFIG_TEGRA186
	funcmux_select(i2c_bus->clk.id, i2c_bus->pinmux_config);
#endif
}

static void send_packet_headers(
	struct i2c_bus *i2c_bus,
	struct i2c_trans_info *trans,
	u32 packet_id,
	bool end_with_repeated_start)
{
	u32 data;

	/* prepare header1: Header size = 0 Protocol = I2C, pktType = 0 */
	data = PROTOCOL_TYPE_I2C << PKT_HDR1_PROTOCOL_SHIFT;
	data |= packet_id << PKT_HDR1_PKT_ID_SHIFT;
	data |= i2c_bus->id << PKT_HDR1_CTLR_ID_SHIFT;
	writel(data, &i2c_bus->control->tx_fifo);
	debug("pkt header 1 sent (0x%x)\n", data);

	/* prepare header2 */
	data = (trans->num_bytes - 1) << PKT_HDR2_PAYLOAD_SIZE_SHIFT;
	writel(data, &i2c_bus->control->tx_fifo);
	debug("pkt header 2 sent (0x%x)\n", data);

	/* prepare IO specific header: configure the slave address */
	data = trans->address << PKT_HDR3_SLAVE_ADDR_SHIFT;

	/* Enable Read if it is not a write transaction */
	if (!(trans->flags & I2C_IS_WRITE))
		data |= PKT_HDR3_READ_MODE_MASK;
	if (end_with_repeated_start)
		data |= PKT_HDR3_REPEAT_START_MASK;

	/* Write I2C specific header */
	writel(data, &i2c_bus->control->tx_fifo);
	debug("pkt header 3 sent (0x%x)\n", data);
}

static int wait_for_tx_fifo_empty(struct i2c_control *control)
{
	u32 count;
	int timeout_us = I2C_TIMEOUT_USEC;

	while (timeout_us >= 0) {
		count = (readl(&control->fifo_status) & TX_FIFO_EMPTY_CNT_MASK)
				>> TX_FIFO_EMPTY_CNT_SHIFT;
		if (count == I2C_FIFO_DEPTH)
			return 1;
		udelay(10);
		timeout_us -= 10;
	}

	return 0;
}

static int wait_for_rx_fifo_notempty(struct i2c_control *control)
{
	u32 count;
	int timeout_us = I2C_TIMEOUT_USEC;

	while (timeout_us >= 0) {
		count = (readl(&control->fifo_status) & TX_FIFO_FULL_CNT_MASK)
				>> TX_FIFO_FULL_CNT_SHIFT;
		if (count)
			return 1;
		udelay(10);
		timeout_us -= 10;
	}

	return 0;
}

static int wait_for_transfer_complete(struct i2c_control *control)
{
	int int_status;
	int timeout_us = I2C_TIMEOUT_USEC;

	while (timeout_us >= 0) {
		int_status = readl(&control->int_status);
		if (int_status & I2C_INT_NO_ACK_MASK)
			return -int_status;
		if (int_status & I2C_INT_ARBITRATION_LOST_MASK)
			return -int_status;
		if (int_status & I2C_INT_XFER_COMPLETE_MASK)
			return 0;

		udelay(10);
		timeout_us -= 10;
	}

	return -1;
}

static int send_recv_packets(struct i2c_bus *i2c_bus,
			     struct i2c_trans_info *trans)
{
	struct i2c_control *control = i2c_bus->control;
	u32 int_status;
	u32 words;
	u8 *dptr;
	u32 local;
	uchar last_bytes;
	int error = 0;
	int is_write = trans->flags & I2C_IS_WRITE;

	/* clear status from previous transaction, XFER_COMPLETE, NOACK, etc. */
	int_status = readl(&control->int_status);
	writel(int_status, &control->int_status);

	send_packet_headers(i2c_bus, trans, 1,
			    trans->flags & I2C_USE_REPEATED_START);

	words = DIV_ROUND_UP(trans->num_bytes, 4);
	last_bytes = trans->num_bytes & 3;
	dptr = trans->buf;

	while (words) {
		u32 *wptr = (u32 *)dptr;

		if (is_write) {
			/* deal with word alignment */
			if ((words == 1) && last_bytes) {
				local = 0;
				memcpy(&local, dptr, last_bytes);
			} else if ((unsigned long)dptr & 3) {
				memcpy(&local, dptr, sizeof(u32));
			} else {
				local = *wptr;
			}
			writel(local, &control->tx_fifo);
			debug("pkt data sent (0x%x)\n", local);
			if (!wait_for_tx_fifo_empty(control)) {
				error = -1;
				goto exit;
			}
		} else {
			if (!wait_for_rx_fifo_notempty(control)) {
				error = -1;
				goto exit;
			}
			/*
			 * for the last word, we read into our local buffer,
			 * in case that caller did not provide enough buffer.
			 */
			local = readl(&control->rx_fifo);
			if ((words == 1) && last_bytes)
				memcpy(dptr, (char *)&local, last_bytes);
			else if ((unsigned long)dptr & 3)
				memcpy(dptr, &local, sizeof(u32));
			else
				*wptr = local;
			debug("pkt data received (0x%x)\n", local);
		}
		words--;
		dptr += sizeof(u32);
	}

	if (wait_for_transfer_complete(control)) {
		error = -1;
		goto exit;
	}
	return 0;
exit:
	/* error, reset the controller. */
	i2c_reset_controller(i2c_bus);

	return error;
}

static int tegra_i2c_write_data(struct i2c_bus *i2c_bus, u32 addr, u8 *data,
				u32 len, bool end_with_repeated_start)
{
	int error;
	struct i2c_trans_info trans_info;

	trans_info.address = addr;
	trans_info.buf = data;
	trans_info.flags = I2C_IS_WRITE;
	if (end_with_repeated_start)
		trans_info.flags |= I2C_USE_REPEATED_START;
	trans_info.num_bytes = len;
	trans_info.is_10bit_address = 0;

	error = send_recv_packets(i2c_bus, &trans_info);
	if (error)
		debug("tegra_i2c_write_data: Error (%d) !!!\n", error);

	return error;
}

static int tegra_i2c_read_data(struct i2c_bus *i2c_bus, u32 addr, u8 *data,
			       u32 len)
{
	int error;
	struct i2c_trans_info trans_info;

	trans_info.address = addr | 1;
	trans_info.buf = data;
	trans_info.flags = 0;
	trans_info.num_bytes = len;
	trans_info.is_10bit_address = 0;

	error = send_recv_packets(i2c_bus, &trans_info);
	if (error)
		debug("tegra_i2c_read_data: Error (%d) !!!\n", error);

	return error;
}

static int tegra_i2c_set_bus_speed(struct udevice *dev, unsigned int speed)
{
	struct i2c_bus *i2c_bus = dev_get_priv(dev);

	i2c_bus->speed = speed;
	i2c_init_controller(i2c_bus);

	return 0;
}

static int tegra_i2c_probe(struct udevice *dev)
{
	struct i2c_bus *i2c_bus = dev_get_priv(dev);
	int ret;
	bool is_dvc;

	i2c_bus->id = dev->seq;
	i2c_bus->type = dev_get_driver_data(dev);
	i2c_bus->regs = (struct i2c_ctlr *)dev_read_addr(dev);
	if ((ulong)i2c_bus->regs == FDT_ADDR_T_NONE) {
		debug("%s: Cannot get regs address\n", __func__);
		return -EINVAL;
	}

	ret = reset_get_by_name(dev, "i2c", &i2c_bus->reset_ctl);
	if (ret) {
		pr_err("reset_get_by_name() failed: %d\n", ret);
		return ret;
	}
	ret = clk_get_by_name(dev, "div-clk", &i2c_bus->clk);
	if (ret) {
		pr_err("clk_get_by_name() failed: %d\n", ret);
		return ret;
	}

#ifndef CONFIG_TEGRA186
	/*
	 * We don't have a binding for pinmux yet. Leave it out for now. So
	 * far no one needs anything other than the default.
	 */
	i2c_bus->pinmux_config = FUNCMUX_DEFAULT;

	/*
	 * We can't specify the pinmux config in the fdt, so I2C2 will not
	 * work on Seaboard. It normally has no devices on it anyway.
	 * You could add in this little hack if you need to use it.
	 * The correct solution is a pinmux binding in the fdt.
	 *
	 *	if (i2c_bus->clk.id == PERIPH_ID_I2C2)
	 *		i2c_bus->pinmux_config = FUNCMUX_I2C2_PTA;
	 */
#endif

	is_dvc = dev_get_driver_data(dev) == TYPE_DVC;
	if (is_dvc) {
		i2c_bus->control =
			&((struct dvc_ctlr *)i2c_bus->regs)->control;
	} else {
		i2c_bus->control = &i2c_bus->regs->control;
	}
	i2c_init_controller(i2c_bus);
	debug("%s: controller bus %d at %p, speed %d: ",
	      is_dvc ? "dvc" : "i2c", dev->seq, i2c_bus->regs, i2c_bus->speed);

	return 0;
}

/* i2c write version without the register address */
static int i2c_write_data(struct i2c_bus *i2c_bus, uchar chip, uchar *buffer,
			  int len, bool end_with_repeated_start)
{
	int rc;

	debug("i2c_write_data: chip=0x%x, len=0x%x\n", chip, len);
	debug("write_data: ");
	/* use rc for counter */
	for (rc = 0; rc < len; ++rc)
		debug(" 0x%02x", buffer[rc]);
	debug("\n");

	/* Shift 7-bit address over for lower-level i2c functions */
	rc = tegra_i2c_write_data(i2c_bus, chip << 1, buffer, len,
				  end_with_repeated_start);
	if (rc)
		debug("i2c_write_data(): rc=%d\n", rc);

	return rc;
}

/* i2c read version without the register address */
static int i2c_read_data(struct i2c_bus *i2c_bus, uchar chip, uchar *buffer,
			 int len)
{
	int rc;

	debug("inside i2c_read_data():\n");
	/* Shift 7-bit address over for lower-level i2c functions */
	rc = tegra_i2c_read_data(i2c_bus, chip << 1, buffer, len);
	if (rc) {
		debug("i2c_read_data(): rc=%d\n", rc);
		return rc;
	}

	debug("i2c_read_data: ");
	/* reuse rc for counter*/
	for (rc = 0; rc < len; ++rc)
		debug(" 0x%02x", buffer[rc]);
	debug("\n");

	return 0;
}

/* Probe to see if a chip is present. */
static int tegra_i2c_probe_chip(struct udevice *bus, uint chip_addr,
				uint chip_flags)
{
	struct i2c_bus *i2c_bus = dev_get_priv(bus);
	int rc;
	u8 reg;

	/* Shift 7-bit address over for lower-level i2c functions */
	rc = tegra_i2c_write_data(i2c_bus, chip_addr << 1, &reg, sizeof(reg),
				  false);

	return rc;
}

static int tegra_i2c_xfer(struct udevice *bus, struct i2c_msg *msg,
			  int nmsgs)
{
	struct i2c_bus *i2c_bus = dev_get_priv(bus);
	int ret;

	debug("i2c_xfer: %d messages\n", nmsgs);
	for (; nmsgs > 0; nmsgs--, msg++) {
		bool next_is_read = nmsgs > 1 && (msg[1].flags & I2C_M_RD);

		debug("i2c_xfer: chip=0x%x, len=0x%x\n", msg->addr, msg->len);
		if (msg->flags & I2C_M_RD) {
			ret = i2c_read_data(i2c_bus, msg->addr, msg->buf,
					    msg->len);
		} else {
			ret = i2c_write_data(i2c_bus, msg->addr, msg->buf,
					     msg->len, next_is_read);
		}
		if (ret) {
			debug("i2c_write: error sending\n");
			return -EREMOTEIO;
		}
	}

	return 0;
}

int tegra_i2c_get_dvc_bus(struct udevice **busp)
{
	struct udevice *bus;

	for (uclass_first_device(UCLASS_I2C, &bus);
	     bus;
	     uclass_next_device(&bus)) {
		if (dev_get_driver_data(bus) == TYPE_DVC) {
			*busp = bus;
			return 0;
		}
	}

	return -ENODEV;
}

static const struct dm_i2c_ops tegra_i2c_ops = {
	.xfer		= tegra_i2c_xfer,
	.probe_chip	= tegra_i2c_probe_chip,
	.set_bus_speed	= tegra_i2c_set_bus_speed,
};

static const struct udevice_id tegra_i2c_ids[] = {
	{ .compatible = "nvidia,tegra114-i2c", .data = TYPE_114 },
	{ .compatible = "nvidia,tegra20-i2c", .data = TYPE_STD },
	{ .compatible = "nvidia,tegra20-i2c-dvc", .data = TYPE_DVC },
	{ }
};

U_BOOT_DRIVER(i2c_tegra) = {
	.name	= "i2c_tegra",
	.id	= UCLASS_I2C,
	.of_match = tegra_i2c_ids,
	.probe	= tegra_i2c_probe,
	.priv_auto_alloc_size = sizeof(struct i2c_bus),
	.ops	= &tegra_i2c_ops,
};
