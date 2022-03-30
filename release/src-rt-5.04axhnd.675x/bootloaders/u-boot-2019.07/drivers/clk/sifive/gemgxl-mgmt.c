// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019, Bin Meng <bmeng.cn@gmail.com>
 */

#include <common.h>
#include <clk-uclass.h>
#include <dm.h>
#include <asm/io.h>

struct gemgxl_mgmt_regs {
	__u32 tx_clk_sel;
};

struct gemgxl_mgmt_platdata {
	struct gemgxl_mgmt_regs *regs;
};

static int gemgxl_mgmt_ofdata_to_platdata(struct udevice *dev)
{
	struct gemgxl_mgmt_platdata *plat = dev_get_platdata(dev);

	plat->regs = (struct gemgxl_mgmt_regs *)dev_read_addr(dev);

	return 0;
}

static ulong gemgxl_mgmt_set_rate(struct clk *clk, ulong rate)
{
	struct gemgxl_mgmt_platdata *plat = dev_get_platdata(clk->dev);

	/*
	 * GEMGXL TX clock operation mode:
	 *
	 * 0 = GMII mode. Use 125 MHz gemgxlclk from PRCI in TX logic
	 *     and output clock on GMII output signal GTX_CLK
	 * 1 = MII mode. Use MII input signal TX_CLK in TX logic
	 */
	writel(rate != 125000000, &plat->regs->tx_clk_sel);

	return 0;
}

const struct clk_ops gemgxl_mgmt_ops = {
	.set_rate = gemgxl_mgmt_set_rate,
};

static const struct udevice_id gemgxl_mgmt_match[] = {
	{ .compatible = "sifive,cadencegemgxlmgmt0", },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(sifive_gemgxl_mgmt) = {
	.name = "sifive-gemgxl-mgmt",
	.id = UCLASS_CLK,
	.of_match = gemgxl_mgmt_match,
	.ofdata_to_platdata = gemgxl_mgmt_ofdata_to_platdata,
	.platdata_auto_alloc_size = sizeof(struct gemgxl_mgmt_platdata),
	.ops = &gemgxl_mgmt_ops,
};
