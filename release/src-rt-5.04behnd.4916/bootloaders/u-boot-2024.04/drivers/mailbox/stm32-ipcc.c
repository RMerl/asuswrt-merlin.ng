// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) STMicroelectronics 2019 - All Rights Reserved
 */

#define LOG_CATEGORY UCLASS_MAILBOX

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <log.h>
#include <mailbox-uclass.h>
#include <malloc.h>
#include <asm/io.h>
#include <dm/device_compat.h>
#include <linux/bitops.h>

/*
 * IPCC has one set of registers per CPU
 * IPCC_PROC_OFFST allows to define cpu registers set base address
 * according to the assigned proc_id.
 */

#define IPCC_PROC_OFFST		0x010

#define IPCC_XSCR		0x008
#define IPCC_XTOYSR		0x00c

#define IPCC_HWCFGR		0x3f0
#define IPCFGR_CHAN_MASK	GENMASK(7, 0)

#define RX_BIT_CHAN(chan)	BIT(chan)
#define TX_BIT_SHIFT		16
#define TX_BIT_CHAN(chan)	BIT(TX_BIT_SHIFT + (chan))

#define STM32_MAX_PROCS		2

struct stm32_ipcc {
	void __iomem *reg_base;
	void __iomem *reg_proc;
	u32 proc_id;
	u32 n_chans;
};

static int stm32_ipcc_request(struct mbox_chan *chan)
{
	struct stm32_ipcc *ipcc = dev_get_priv(chan->dev);

	dev_dbg(chan->dev, "chan=%p\n", chan);

	if (chan->id >= ipcc->n_chans) {
		dev_dbg(chan->dev, "failed to request channel: %ld\n",
			chan->id);
		return -EINVAL;
	}

	return 0;
}

static int stm32_ipcc_free(struct mbox_chan *chan)
{
	dev_dbg(chan->dev, "chan=%p\n", chan);

	return 0;
}

static int stm32_ipcc_send(struct mbox_chan *chan, const void *data)
{
	struct stm32_ipcc *ipcc = dev_get_priv(chan->dev);

	dev_dbg(chan->dev, "chan=%p, data=%p\n", chan, data);

	if (readl(ipcc->reg_proc + IPCC_XTOYSR) & BIT(chan->id))
		return -EBUSY;

	/* set channel n occupied */
	setbits_le32(ipcc->reg_proc + IPCC_XSCR, TX_BIT_CHAN(chan->id));

	return 0;
}

static int stm32_ipcc_recv(struct mbox_chan *chan, void *data)
{
	struct stm32_ipcc *ipcc = dev_get_priv(chan->dev);
	u32 val;
	int proc_offset;

	dev_dbg(chan->dev, "chan=%p, data=%p\n", chan, data);

	/* read 'channel occupied' status from other proc */
	proc_offset = ipcc->proc_id ? -IPCC_PROC_OFFST : IPCC_PROC_OFFST;
	val = readl(ipcc->reg_proc + proc_offset + IPCC_XTOYSR);

	if (!(val & BIT(chan->id)))
		return -ENODATA;

	setbits_le32(ipcc->reg_proc + IPCC_XSCR, RX_BIT_CHAN(chan->id));

	return 0;
}

static int stm32_ipcc_probe(struct udevice *dev)
{
	struct stm32_ipcc *ipcc = dev_get_priv(dev);
	fdt_addr_t addr;
	struct clk clk;
	int ret;

	dev_dbg(dev, "\n");

	addr = dev_read_addr(dev);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	ipcc->reg_base = (void __iomem *)addr;

	/* proc_id */
	ret = dev_read_u32_index(dev, "st,proc_id", 1, &ipcc->proc_id);
	if (ret) {
		dev_dbg(dev, "Missing st,proc_id\n");
		return -EINVAL;
	}

	if (ipcc->proc_id >= STM32_MAX_PROCS) {
		dev_err(dev, "Invalid proc_id (%d)\n", ipcc->proc_id);
		return -EINVAL;
	}

	ipcc->reg_proc = ipcc->reg_base + ipcc->proc_id * IPCC_PROC_OFFST;

	ret = clk_get_by_index(dev, 0, &clk);
	if (ret)
		return ret;

	ret = clk_enable(&clk);
	if (ret)
		return ret;

	/* get channel number */
	ipcc->n_chans = readl(ipcc->reg_base + IPCC_HWCFGR);
	ipcc->n_chans &= IPCFGR_CHAN_MASK;

	return 0;
}

static const struct udevice_id stm32_ipcc_ids[] = {
	{ .compatible = "st,stm32mp1-ipcc" },
	{ }
};

struct mbox_ops stm32_ipcc_mbox_ops = {
	.request = stm32_ipcc_request,
	.rfree = stm32_ipcc_free,
	.send = stm32_ipcc_send,
	.recv = stm32_ipcc_recv,
};

U_BOOT_DRIVER(stm32_ipcc) = {
	.name = "stm32_ipcc",
	.id = UCLASS_MAILBOX,
	.of_match = stm32_ipcc_ids,
	.probe = stm32_ipcc_probe,
	.priv_auto	= sizeof(struct stm32_ipcc),
	.ops = &stm32_ipcc_mbox_ops,
};
