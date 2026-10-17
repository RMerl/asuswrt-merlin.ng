// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2020-2022 NXP
 */

#include <common.h>
#include <asm/io.h>
#include <dm.h>
#include <dm/lists.h>
#include <dm/root.h>
#include <dm/device-internal.h>
#include <asm/mach-imx/ele_api.h>
#include <asm/arch/imx-regs.h>
#include <linux/iopoll.h>
#include <misc.h>

DECLARE_GLOBAL_DATA_PTR;

struct imx8ulp_mu {
	struct mu_type *base;
};

#define MU_SR_TE0_MASK		BIT(0)
#define MU_SR_RF0_MASK		BIT(0)
#define MU_TR_COUNT		8
#define MU_RR_COUNT		4

void mu_hal_init(ulong base)
{
	struct mu_type *mu_base = (struct mu_type *)base;

	writel(0, &mu_base->tcr);
	writel(0, &mu_base->rcr);
}

int mu_hal_sendmsg(ulong base, u32 reg_index, u32 msg)
{
	struct mu_type *mu_base = (struct mu_type *)base;
	u32 mask = MU_SR_TE0_MASK << reg_index;
	u32 val;
	int ret;

	assert(reg_index < MU_TR_COUNT);

	debug("sendmsg tsr 0x%x\n", readl(&mu_base->tsr));

	/* Wait TX register to be empty. */
	ret = readl_poll_timeout(&mu_base->tsr, val, val & mask, 10000);
	if (ret < 0) {
		debug("%s timeout\n", __func__);
		return -ETIMEDOUT;
	}

	debug("tr[%d] 0x%x\n", reg_index, msg);

	writel(msg, &mu_base->tr[reg_index]);

	return 0;
}

int mu_hal_receivemsg(ulong base, u32 reg_index, u32 *msg)
{
	struct mu_type *mu_base = (struct mu_type *)base;
	u32 mask = MU_SR_RF0_MASK << reg_index;
	u32 val;
	int ret;
	u32 count = 10;

	assert(reg_index < MU_RR_COUNT);

	debug("receivemsg rsr 0x%x\n", readl(&mu_base->rsr));

	do {
		/* Wait RX register to be full. */
		ret = readl_poll_timeout(&mu_base->rsr, val, val & mask, 1000000);
		if (ret < 0) {
			count--;
			printf("mu receive msg wait %us\n", 10 - count);
		} else {
			break;
		}
	} while (count > 0);

	if (count == 0) {
		debug("%s timeout\n", __func__);
		return -ETIMEDOUT;
	}

	*msg = readl(&mu_base->rr[reg_index]);

	debug("rr[%d] 0x%x\n", reg_index, *msg);

	return 0;
}

static int imx8ulp_mu_read(struct mu_type *base, void *data)
{
	struct ele_msg *msg = (struct ele_msg *)data;
	int ret;
	u8 count = 0;

	if (!msg)
		return -EINVAL;

	/* Read first word */
	ret = mu_hal_receivemsg((ulong)base, 0, (u32 *)msg);
	if (ret)
		return ret;
	count++;

	/* Check size */
	if (msg->size > ELE_MAX_MSG) {
		*((u32 *)msg) = 0;
		return -EINVAL;
	}

	/* Read remaining words */
	while (count < msg->size) {
		ret = mu_hal_receivemsg((ulong)base, count % MU_RR_COUNT,
					&msg->data[count - 1]);
		if (ret)
			return ret;
		count++;
	}

	return 0;
}

static int imx8ulp_mu_write(struct mu_type *base, void *data)
{
	struct ele_msg *msg = (struct ele_msg *)data;
	int ret;
	u8 count = 0;

	if (!msg)
		return -EINVAL;

	/* Check size */
	if (msg->size > ELE_MAX_MSG)
		return -EINVAL;

	/* Write first word */
	ret = mu_hal_sendmsg((ulong)base, 0, *((u32 *)msg));
	if (ret)
		return ret;
	count++;

	/* Write remaining words */
	while (count < msg->size) {
		ret = mu_hal_sendmsg((ulong)base, count % MU_TR_COUNT,
				     msg->data[count - 1]);
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
static int imx8ulp_mu_call(struct udevice *dev, int no_resp, void *tx_msg,
			   int tx_size, void *rx_msg, int rx_size)
{
	struct imx8ulp_mu *priv = dev_get_priv(dev);
	u32 result;
	int ret;

	/* Expect tx_msg, rx_msg are the same value */
	if (rx_msg && tx_msg != rx_msg)
		printf("tx_msg %p, rx_msg %p\n", tx_msg, rx_msg);

	ret = imx8ulp_mu_write(priv->base, tx_msg);
	if (ret)
		return ret;
	if (!no_resp) {
		ret = imx8ulp_mu_read(priv->base, rx_msg);
		if (ret)
			return ret;
	}

	result = ((struct ele_msg *)rx_msg)->data[0];
	if ((result & 0xff) == 0xd6)
		return 0;

	return -EIO;
}

static int imx8ulp_mu_probe(struct udevice *dev)
{
	struct imx8ulp_mu *priv = dev_get_priv(dev);
	fdt_addr_t addr;

	debug("%s(dev=%p) (priv=%p)\n", __func__, dev, priv);

	addr = devfdt_get_addr(dev);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	priv->base = (struct mu_type *)addr;

	debug("mu base 0x%lx\n", (ulong)priv->base);

	/* U-Boot not enable interrupts, so need to enable RX interrupts */
	mu_hal_init((ulong)priv->base);

	gd->arch.ele_dev = dev;

	return 0;
}

static int imx8ulp_mu_remove(struct udevice *dev)
{
	return 0;
}

static int imx8ulp_mu_bind(struct udevice *dev)
{
	debug("%s(dev=%p)\n", __func__, dev);

	return 0;
}

static struct misc_ops imx8ulp_mu_ops = {
	.call = imx8ulp_mu_call,
};

static const struct udevice_id imx8ulp_mu_ids[] = {
	{ .compatible = "fsl,imx8ulp-mu" },
	{ .compatible = "fsl,imx93-mu-s4" },
	{ }
};

U_BOOT_DRIVER(imx8ulp_mu) = {
	.name		= "imx8ulp_mu",
	.id		= UCLASS_MISC,
	.of_match	= imx8ulp_mu_ids,
	.probe		= imx8ulp_mu_probe,
	.bind		= imx8ulp_mu_bind,
	.remove		= imx8ulp_mu_remove,
	.ops		= &imx8ulp_mu_ops,
	.priv_auto	= sizeof(struct imx8ulp_mu),
};
