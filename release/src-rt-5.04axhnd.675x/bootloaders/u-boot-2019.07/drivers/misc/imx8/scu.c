// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2018 NXP
 *
 * Peng Fan <peng.fan@nxp.com>
 */

#include <common.h>
#include <asm/io.h>
#include <dm.h>
#include <dm/lists.h>
#include <dm/root.h>
#include <dm/device-internal.h>
#include <asm/arch/sci/sci.h>
#include <linux/iopoll.h>
#include <misc.h>

DECLARE_GLOBAL_DATA_PTR;

struct mu_type {
	u32 tr[4];
	u32 rr[4];
	u32 sr;
	u32 cr;
};

struct imx8_scu {
	struct mu_type *base;
	struct udevice *clk;
	struct udevice *pinclk;
};

#define MU_CR_GIE_MASK		0xF0000000u
#define MU_CR_RIE_MASK		0xF000000u
#define MU_CR_GIR_MASK		0xF0000u
#define MU_CR_TIE_MASK		0xF00000u
#define MU_CR_F_MASK		0x7u
#define MU_SR_TE0_MASK		BIT(23)
#define MU_SR_RF0_MASK		BIT(27)
#define MU_TR_COUNT		4
#define MU_RR_COUNT		4

static inline void mu_hal_init(struct mu_type *base)
{
	/* Clear GIEn, RIEn, TIEn, GIRn and ABFn. */
	clrbits_le32(&base->cr, MU_CR_GIE_MASK | MU_CR_RIE_MASK |
		     MU_CR_TIE_MASK | MU_CR_GIR_MASK | MU_CR_F_MASK);
}

static int mu_hal_sendmsg(struct mu_type *base, u32 reg_index, u32 msg)
{
	u32 mask = MU_SR_TE0_MASK >> reg_index;
	u32 val;
	int ret;

	assert(reg_index < MU_TR_COUNT);

	/* Wait TX register to be empty. */
	ret = readl_poll_timeout(&base->sr, val, val & mask, 10000);
	if (ret < 0) {
		printf("%s timeout\n", __func__);
		return -ETIMEDOUT;
	}

	writel(msg, &base->tr[reg_index]);

	return 0;
}

static int mu_hal_receivemsg(struct mu_type *base, u32 reg_index, u32 *msg)
{
	u32 mask = MU_SR_RF0_MASK >> reg_index;
	u32 val;
	int ret;

	assert(reg_index < MU_TR_COUNT);

	/* Wait RX register to be full. */
	ret = readl_poll_timeout(&base->sr, val, val & mask, 10000);
	if (ret < 0) {
		printf("%s timeout\n", __func__);
		return -ETIMEDOUT;
	}

	*msg = readl(&base->rr[reg_index]);

	return 0;
}

static int sc_ipc_read(struct mu_type *base, void *data)
{
	struct sc_rpc_msg_s *msg = (struct sc_rpc_msg_s *)data;
	int ret;
	u8 count = 0;

	if (!msg)
		return -EINVAL;

	/* Read first word */
	ret = mu_hal_receivemsg(base, 0, (u32 *)msg);
	if (ret)
		return ret;
	count++;

	/* Check size */
	if (msg->size > SC_RPC_MAX_MSG) {
		*((u32 *)msg) = 0;
		return -EINVAL;
	}

	/* Read remaining words */
	while (count < msg->size) {
		ret = mu_hal_receivemsg(base, count % MU_RR_COUNT,
					&msg->DATA.u32[count - 1]);
		if (ret)
			return ret;
		count++;
	}

	return 0;
}

static int sc_ipc_write(struct mu_type *base, void *data)
{
	struct sc_rpc_msg_s *msg = (struct sc_rpc_msg_s *)data;
	int ret;
	u8 count = 0;

	if (!msg)
		return -EINVAL;

	/* Check size */
	if (msg->size > SC_RPC_MAX_MSG)
		return -EINVAL;

	/* Write first word */
	ret = mu_hal_sendmsg(base, 0, *((u32 *)msg));
	if (ret)
		return ret;
	count++;

	/* Write remaining words */
	while (count < msg->size) {
		ret = mu_hal_sendmsg(base, count % MU_TR_COUNT,
				     msg->DATA.u32[count - 1]);
		if (ret)
			return ret;
		count++;
	}

	return 0;
}

/*
 * Note the function prototype use msgid as the 2nd parameter, here
 * we take it as no_resp.
 */
static int imx8_scu_call(struct udevice *dev, int no_resp, void *tx_msg,
			 int tx_size, void *rx_msg, int rx_size)
{
	struct imx8_scu *plat = dev_get_platdata(dev);
	sc_err_t result;
	int ret;

	/* Expect tx_msg, rx_msg are the same value */
	if (rx_msg && tx_msg != rx_msg)
		printf("tx_msg %p, rx_msg %p\n", tx_msg, rx_msg);

	ret = sc_ipc_write(plat->base, tx_msg);
	if (ret)
		return ret;
	if (!no_resp) {
		ret = sc_ipc_read(plat->base, rx_msg);
		if (ret)
			return ret;
	}

	result = RPC_R8((struct sc_rpc_msg_s *)tx_msg);

	return sc_err_to_linux(result);
}

static int imx8_scu_probe(struct udevice *dev)
{
	struct imx8_scu *plat = dev_get_platdata(dev);
	fdt_addr_t addr;

	debug("%s(dev=%p) (plat=%p)\n", __func__, dev, plat);

	addr = devfdt_get_addr(dev);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

#ifdef CONFIG_SPL_BUILD
	plat->base = (struct mu_type *)CONFIG_MU_BASE_SPL;
#else
	plat->base = (struct mu_type *)addr;
#endif

	/* U-Boot not enable interrupts, so need to enable RX interrupts */
	mu_hal_init(plat->base);

	gd->arch.scu_dev = dev;

	device_probe(plat->clk);
	device_probe(plat->pinclk);

	return 0;
}

static int imx8_scu_remove(struct udevice *dev)
{
	return 0;
}

static int imx8_scu_bind(struct udevice *dev)
{
	struct imx8_scu *plat = dev_get_platdata(dev);
	int ret;
	struct udevice *child;
	int node;
	char *clk_compatible, *iomuxc_compatible;

	if (IS_ENABLED(CONFIG_IMX8QXP)) {
		clk_compatible = "fsl,imx8qxp-clk";
		iomuxc_compatible = "fsl,imx8qxp-iomuxc";
	} else if (IS_ENABLED(CONFIG_IMX8QM)) {
		clk_compatible = "fsl,imx8qm-clk";
		iomuxc_compatible = "fsl,imx8qm-iomuxc";
	} else {
		return -EINVAL;
	}

	debug("%s(dev=%p)\n", __func__, dev);

	node = fdt_node_offset_by_compatible(gd->fdt_blob, -1, clk_compatible);
	if (node < 0)
		panic("No clk node found\n");

	ret = lists_bind_fdt(dev, offset_to_ofnode(node), &child, true);
	if (ret)
		return ret;

	plat->clk = child;

	node = fdt_node_offset_by_compatible(gd->fdt_blob, -1,
					     iomuxc_compatible);
	if (node < 0)
		panic("No iomuxc node found\n");

	ret = lists_bind_fdt(dev, offset_to_ofnode(node), &child, true);
	if (ret)
		return ret;

	plat->pinclk = child;

	return 0;
}

static struct misc_ops imx8_scu_ops = {
	.call = imx8_scu_call,
};

static const struct udevice_id imx8_scu_ids[] = {
	{ .compatible = "fsl,imx8qxp-mu" },
	{ .compatible = "fsl,imx8-mu" },
	{ }
};

U_BOOT_DRIVER(imx8_scu) = {
	.name		= "imx8_scu",
	.id		= UCLASS_MISC,
	.of_match	= imx8_scu_ids,
	.probe		= imx8_scu_probe,
	.bind		= imx8_scu_bind,
	.remove		= imx8_scu_remove,
	.ops		= &imx8_scu_ops,
	.platdata_auto_alloc_size = sizeof(struct imx8_scu),
	.flags		= DM_FLAG_PRE_RELOC,
};
