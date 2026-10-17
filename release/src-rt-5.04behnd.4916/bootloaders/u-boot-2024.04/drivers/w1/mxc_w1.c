// SPDX-License-Identifier: GPL-2.0+
/*
 * Driver for one wire controller in some i.MX Socs
 *
 * There are currently two silicon variants:
 * V1: i.MX21, i.MX27, i.MX31, i.MX51
 * V2: i.MX25, i.MX35, i.MX50, i.MX53
 * Newer i.MX SoCs such as the i.MX6 do not have one wire controllers.
 *
 * The V1 controller only supports single bit operations.
 * The V2 controller is backwards compatible on the register level but adds
 * byte size operations and a "search ROM accelerator mode"
 *
 * This driver does not currently support the search ROM accelerator
 *
 * Copyright (c) 2018 Flowbird
 * Martin Fuzzey <martin.fuzzey@flowbird.group>
 */

#include <common.h>
#include <asm/arch/clock.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <w1.h>

struct mxc_w1_regs {
	u16 control;
#define MXC_W1_CONTROL_RPP	BIT(7)
#define MXC_W1_CONTROL_PST	BIT(6)
#define MXC_W1_CONTROL_WR(x)	BIT(5 - (x))
#define MXC_W1_CONTROL_RDST	BIT(3)

	u16 time_divider;
	u16 reset;

	/* Registers below on V2 silicon only */
	u16 command;
	u16 tx_rx;
	u16 interrupt;
#define MXC_W1_INTERRUPT_TBE	BIT(2)
#define MXC_W1_INTERRUPT_TSRE	BIT(3)
#define MXC_W1_INTERRUPT_RBF	BIT(4)
#define MXC_W1_INTERRUPT_RSRF	BIT(5)

	u16 interrupt_en;
};

struct mxc_w1_pdata {
	struct mxc_w1_regs *regs;
};

/*
 * this is the low level routine to read/write a bit on the One Wire
 * interface on the hardware. It does write 0 if parameter bit is set
 * to 0, otherwise a write 1/read.
 */
static u8 mxc_w1_touch_bit(struct mxc_w1_pdata *pdata, u8 bit)
{
	u16 *ctrl_addr = &pdata->regs->control;
	u16 mask = MXC_W1_CONTROL_WR(bit);
	unsigned int timeout_cnt = 400; /* Takes max. 120us according to
					 * datasheet.
					 */

	writew(mask, ctrl_addr);

	while (timeout_cnt--) {
		if (!(readw(ctrl_addr) & mask))
			break;

		udelay(1);
	}

	return (readw(ctrl_addr) & MXC_W1_CONTROL_RDST) ? 1 : 0;
}

static u8 mxc_w1_read_byte(struct udevice *dev)
{
	struct mxc_w1_pdata *pdata = dev_get_plat(dev);
	struct mxc_w1_regs *regs = pdata->regs;
	u16 status;

	if (dev_get_driver_data(dev) < 2) {
		int i;
		u8 ret = 0;

		for (i = 0; i < 8; i++)
			ret |= (mxc_w1_touch_bit(pdata, 1) << i);

		return ret;
	}

	readw(&regs->tx_rx);
	writew(0xFF, &regs->tx_rx);

	do {
		udelay(1); /* Without this bytes are sometimes duplicated... */
		status = readw(&regs->interrupt);
	} while (!(status & MXC_W1_INTERRUPT_RBF));

	return (u8)readw(&regs->tx_rx);
}

static void mxc_w1_write_byte(struct udevice *dev, u8 byte)
{
	struct mxc_w1_pdata *pdata = dev_get_plat(dev);
	struct mxc_w1_regs *regs = pdata->regs;
	u16 status;

	if (dev_get_driver_data(dev) < 2) {
		int i;

		for (i = 0; i < 8; i++)
			mxc_w1_touch_bit(pdata, (byte >> i) & 0x1);

		return;
	}

	readw(&regs->tx_rx);
	writew(byte, &regs->tx_rx);

	do {
		udelay(1);
		status = readw(&regs->interrupt);
	} while (!(status & MXC_W1_INTERRUPT_TSRE));
}

static bool mxc_w1_reset(struct udevice *dev)
{
	struct mxc_w1_pdata *pdata = dev_get_plat(dev);
	u16 reg_val;

	writew(MXC_W1_CONTROL_RPP, &pdata->regs->control);

	do {
		reg_val = readw(&pdata->regs->control);
	}  while (reg_val & MXC_W1_CONTROL_RPP);

	return !(reg_val & MXC_W1_CONTROL_PST);
}

static u8 mxc_w1_triplet(struct udevice *dev, bool bdir)
{
	struct mxc_w1_pdata *pdata = dev_get_plat(dev);
	u8 id_bit   = mxc_w1_touch_bit(pdata, 1);
	u8 comp_bit = mxc_w1_touch_bit(pdata, 1);
	u8 retval;

	if (id_bit && comp_bit)
		return 0x03;  /* error */

	if (!id_bit && !comp_bit) {
		/* Both bits are valid, take the direction given */
		retval = bdir ? 0x04 : 0;
	} else {
		/* Only one bit is valid, take that direction */
		bdir = id_bit;
		retval = id_bit ? 0x05 : 0x02;
	}

	mxc_w1_touch_bit(pdata, bdir);

	return retval;
}

static int mxc_w1_of_to_plat(struct udevice *dev)
{
	struct mxc_w1_pdata *pdata = dev_get_plat(dev);
	fdt_addr_t addr;

	addr = dev_read_addr(dev);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	pdata->regs = (struct mxc_w1_regs *)addr;

	return 0;
};

static int mxc_w1_probe(struct udevice *dev)
{
	struct mxc_w1_pdata *pdata = dev_get_plat(dev);
	unsigned int clkrate = mxc_get_clock(MXC_IPG_PERCLK);
	unsigned int clkdiv;

	if (clkrate < 10000000) {
		dev_err(dev, "input clock frequency (%u Hz) too low\n",
			clkrate);
		return -EINVAL;
	}

	clkdiv = clkrate / 1000000;
	clkrate /= clkdiv;
	if (clkrate < 980000 || clkrate > 1020000) {
		dev_err(dev, "Incorrect time base frequency %u Hz\n", clkrate);
		return -EINVAL;
	}

	writew(clkdiv - 1, &pdata->regs->time_divider);

	return 0;
}

static const struct w1_ops mxc_w1_ops = {
	.read_byte	= mxc_w1_read_byte,
	.reset		= mxc_w1_reset,
	.triplet	= mxc_w1_triplet,
	.write_byte	= mxc_w1_write_byte,
};

static const struct udevice_id mxc_w1_id[] = {
	{ .compatible = "fsl,imx21-owire", .data = 1 },
	{ .compatible = "fsl,imx27-owire", .data = 1 },
	{ .compatible = "fsl,imx31-owire", .data = 1 },
	{ .compatible = "fsl,imx51-owire", .data = 1 },

	{ .compatible = "fsl,imx25-owire", .data = 2 },
	{ .compatible = "fsl,imx35-owire", .data = 2 },
	{ .compatible = "fsl,imx50-owire", .data = 2 },
	{ .compatible = "fsl,imx53-owire", .data = 2 },
	{ },
};

U_BOOT_DRIVER(mxc_w1_drv) = {
	.id				= UCLASS_W1,
	.name				= "mxc_w1_drv",
	.of_match			= mxc_w1_id,
	.of_to_plat		= mxc_w1_of_to_plat,
	.ops				= &mxc_w1_ops,
	.plat_auto	= sizeof(struct mxc_w1_pdata),
	.probe				= mxc_w1_probe,
};
