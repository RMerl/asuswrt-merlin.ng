// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright 2021 ASPEED Technology Inc.
 */
#include <config.h>
#include <common.h>
#include <clk.h>
#include <dm.h>
#include <asm/types.h>
#include <asm/io.h>
#include <dm/device.h>
#include <dm/fdtaddr.h>
#include <linux/delay.h>
#include <u-boot/rsa-mod-exp.h>

/* ACRY register offsets */
#define ACRY_CTRL1		0x00
#define   ACRY_CTRL1_RSA_DMA		BIT(1)
#define   ACRY_CTRL1_RSA_START		BIT(0)
#define ACRY_CTRL2		0x44
#define ACRY_CTRL3		0x48
#define   ACRY_CTRL3_SRAM_AHB_ACCESS	BIT(8)
#define   ACRY_CTRL3_ECC_RSA_MODE_MASK	GENMASK(5, 4)
#define   ACRY_CTRL3_ECC_RSA_MODE_SHIFT	4
#define ACRY_DMA_DRAM_SADDR	0x4c
#define ACRY_DMA_DMEM_TADDR	0x50
#define   ACRY_DMA_DMEM_TADDR_LEN_MASK	GENMASK(15, 0)
#define   ACRY_DMA_DMEM_TADDR_LEN_SHIFT	0
#define ACRY_RSA_PARAM		0x58
#define   ACRY_RSA_PARAM_EXP_MASK	GENMASK(31, 16)
#define   ACRY_RSA_PARAM_EXP_SHIFT	16
#define   ACRY_RSA_PARAM_MOD_MASK	GENMASK(15, 0)
#define   ACRY_RSA_PARAM_MOD_SHIFT	0
#define ACRY_RSA_INT_EN		0x3f8
#define   ACRY_RSA_INT_EN_RSA_READY	BIT(2)
#define   ACRY_RSA_INT_EN_RSA_CMPLT	BIT(1)
#define ACRY_RSA_INT_STS	0x3fc
#define   ACRY_RSA_INT_STS_RSA_READY	BIT(2)
#define   ACRY_RSA_INT_STS_RSA_CMPLT	BIT(1)

/* misc. constant */
#define ACRY_ECC_MODE	2
#define ACRY_RSA_MODE	3
#define ACRY_CTX_BUFSZ	0x600

struct aspeed_acry {
	phys_addr_t base;
	phys_addr_t sram_base; /* internal sram */
	struct clk clk;
};

static int aspeed_acry_mod_exp(struct udevice *dev, const uint8_t *sig, uint32_t sig_len,
			       struct key_prop *prop, uint8_t *out)
{
	int i, j;
	u8 *ctx;
	u8 *ptr;
	u32 reg;
	struct aspeed_acry *acry = dev_get_priv(dev);

	ctx = memalign(16, ACRY_CTX_BUFSZ);
	if (!ctx)
		return -ENOMEM;

	memset(ctx, 0, ACRY_CTX_BUFSZ);

	ptr = (u8 *)prop->public_exponent;
	for (i = prop->exp_len - 1, j = 0; i >= 0; --i) {
		ctx[j] = ptr[i];
		j++;
		j = (j % 16) ? j : j + 32;
	}

	ptr = (u8 *)prop->modulus;
	for (i = (prop->num_bits >> 3) - 1, j = 0; i >= 0; --i) {
		ctx[j + 16] = ptr[i];
		j++;
		j = (j % 16) ? j : j + 32;
	}

	ptr = (u8 *)sig;
	for (i = sig_len - 1, j = 0; i >= 0; --i) {
		ctx[j + 32] = ptr[i];
		j++;
		j = (j % 16) ? j : j + 32;
	}

	writel((u32)ctx, acry->base + ACRY_DMA_DRAM_SADDR);

	reg = (((prop->exp_len << 3) << ACRY_RSA_PARAM_EXP_SHIFT) & ACRY_RSA_PARAM_EXP_MASK) |
		  ((prop->num_bits << ACRY_RSA_PARAM_MOD_SHIFT) & ACRY_RSA_PARAM_MOD_MASK);
	writel(reg, acry->base + ACRY_RSA_PARAM);

	reg = (ACRY_CTX_BUFSZ << ACRY_DMA_DMEM_TADDR_LEN_SHIFT) & ACRY_DMA_DMEM_TADDR_LEN_MASK;
	writel(reg, acry->base + ACRY_DMA_DMEM_TADDR);

	reg = (ACRY_RSA_MODE << ACRY_CTRL3_ECC_RSA_MODE_SHIFT) & ACRY_CTRL3_ECC_RSA_MODE_MASK;
	writel(reg, acry->base + ACRY_CTRL3);

	writel(ACRY_CTRL1_RSA_DMA | ACRY_CTRL1_RSA_START, acry->base + ACRY_CTRL1);

	/* polling RSA status */
	while (1) {
		reg = readl(acry->base + ACRY_RSA_INT_STS);
		if ((reg & ACRY_RSA_INT_STS_RSA_READY) && (reg & ACRY_RSA_INT_STS_RSA_CMPLT)) {
			writel(reg, acry->base + ACRY_RSA_INT_STS);
			break;
		}
		udelay(20);
	}

	/* grant SRAM access permission to CPU */
	writel(0x0, acry->base + ACRY_CTRL1);
	writel(ACRY_CTRL3_SRAM_AHB_ACCESS, acry->base + ACRY_CTRL3);
	udelay(20);

	for (i = (prop->num_bits / 8) - 1, j = 0; i >= 0; --i) {
		out[i] = readb(acry->sram_base + (j + 32));
		j++;
		j = (j % 16) ? j : j + 32;
	}

	/* return SRAM access permission to ACRY */
	writel(0, acry->base + ACRY_CTRL3);

	free(ctx);

	return 0;
}

static int aspeed_acry_probe(struct udevice *dev)
{
	struct aspeed_acry *acry = dev_get_priv(dev);
	int ret;

	ret = clk_get_by_index(dev, 0, &acry->clk);
	if (ret < 0) {
		debug("Can't get clock for %s: %d\n", dev->name, ret);
		return ret;
	}

	ret = clk_enable(&acry->clk);
	if (ret) {
		debug("Failed to enable acry clock (%d)\n", ret);
		return ret;
	}

	acry->base = devfdt_get_addr_index(dev, 0);
	if (acry->base == FDT_ADDR_T_NONE) {
		debug("Failed to get acry base\n");
		return acry->base;
	}

	acry->sram_base = devfdt_get_addr_index(dev, 1);
	if (acry->sram_base == FDT_ADDR_T_NONE) {
		debug("Failed to get acry SRAM base\n");
		return acry->sram_base;
	}

	return ret;
}

static int aspeed_acry_remove(struct udevice *dev)
{
	struct aspeed_acry *acry = dev_get_priv(dev);

	clk_disable(&acry->clk);

	return 0;
}

static const struct mod_exp_ops aspeed_acry_ops = {
	.mod_exp = aspeed_acry_mod_exp,
};

static const struct udevice_id aspeed_acry_ids[] = {
	{ .compatible = "aspeed,ast2600-acry" },
	{ }
};

U_BOOT_DRIVER(aspeed_acry) = {
	.name = "aspeed_acry",
	.id = UCLASS_MOD_EXP,
	.of_match = aspeed_acry_ids,
	.probe = aspeed_acry_probe,
	.remove = aspeed_acry_remove,
	.priv_auto = sizeof(struct aspeed_acry),
	.ops = &aspeed_acry_ops,
	.flags = DM_FLAG_PRE_RELOC,
};
