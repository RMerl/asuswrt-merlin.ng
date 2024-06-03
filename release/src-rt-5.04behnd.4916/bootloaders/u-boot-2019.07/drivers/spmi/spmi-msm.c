// SPDX-License-Identifier: BSD-3-Clause
/*
 * Qualcomm SPMI bus driver
 *
 * (C) Copyright 2015 Mateusz Kulikowski <mateusz.kulikowski@gmail.com>
 *
 * Loosely based on Little Kernel driver
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <fdtdec.h>
#include <asm/io.h>
#include <spmi/spmi.h>

DECLARE_GLOBAL_DATA_PTR;

/* PMIC Arbiter configuration registers */
#define PMIC_ARB_VERSION		0x0000
#define PMIC_ARB_VERSION_V2_MIN		0x20010000

#define ARB_CHANNEL_OFFSET(n)		(0x4 * (n))
#define SPMI_CH_OFFSET(chnl)		((chnl) * 0x8000)

#define SPMI_REG_CMD0			0x0
#define SPMI_REG_CONFIG			0x4
#define SPMI_REG_STATUS			0x8
#define SPMI_REG_WDATA			0x10
#define SPMI_REG_RDATA			0x18

#define SPMI_CMD_OPCODE_SHIFT		27
#define SPMI_CMD_SLAVE_ID_SHIFT		20
#define SPMI_CMD_ADDR_SHIFT		12
#define SPMI_CMD_ADDR_OFFSET_SHIFT	4
#define SPMI_CMD_BYTE_CNT_SHIFT		0

#define SPMI_CMD_EXT_REG_WRITE_LONG	0x00
#define SPMI_CMD_EXT_REG_READ_LONG	0x01

#define SPMI_STATUS_DONE		0x1

#define SPMI_MAX_CHANNELS	128
#define SPMI_MAX_SLAVES		16
#define SPMI_MAX_PERIPH		256

struct msm_spmi_priv {
	phys_addr_t arb_chnl; /* ARB channel mapping base */
	phys_addr_t spmi_core; /* SPMI core */
	phys_addr_t spmi_obs; /* SPMI observer */
	/* SPMI channel map */
	uint8_t channel_map[SPMI_MAX_SLAVES][SPMI_MAX_PERIPH];
};

static int msm_spmi_write(struct udevice *dev, int usid, int pid, int off,
			  uint8_t val)
{
	struct msm_spmi_priv *priv = dev_get_priv(dev);
	unsigned channel;
	uint32_t reg = 0;

	if (usid >= SPMI_MAX_SLAVES)
		return -EIO;
	if (pid >= SPMI_MAX_PERIPH)
		return -EIO;

	channel = priv->channel_map[usid][pid];

	/* Disable IRQ mode for the current channel*/
	writel(0x0, priv->spmi_core + SPMI_CH_OFFSET(channel) +
	       SPMI_REG_CONFIG);

	/* Write single byte */
	writel(val, priv->spmi_core + SPMI_CH_OFFSET(channel) + SPMI_REG_WDATA);

	/* Prepare write command */
	reg |= SPMI_CMD_EXT_REG_WRITE_LONG << SPMI_CMD_OPCODE_SHIFT;
	reg |= (usid << SPMI_CMD_SLAVE_ID_SHIFT);
	reg |= (pid << SPMI_CMD_ADDR_SHIFT);
	reg |= (off << SPMI_CMD_ADDR_OFFSET_SHIFT);
	reg |= 1; /* byte count */

	/* Send write command */
	writel(reg, priv->spmi_core + SPMI_CH_OFFSET(channel) + SPMI_REG_CMD0);

	/* Wait till CMD DONE status */
	reg = 0;
	while (!reg) {
		reg = readl(priv->spmi_core + SPMI_CH_OFFSET(channel) +
			    SPMI_REG_STATUS);
	}

	if (reg ^ SPMI_STATUS_DONE) {
		printf("SPMI write failure.\n");
		return -EIO;
	}

	return 0;
}

static int msm_spmi_read(struct udevice *dev, int usid, int pid, int off)
{
	struct msm_spmi_priv *priv = dev_get_priv(dev);
	unsigned channel;
	uint32_t reg = 0;

	if (usid >= SPMI_MAX_SLAVES)
		return -EIO;
	if (pid >= SPMI_MAX_PERIPH)
		return -EIO;

	channel = priv->channel_map[usid][pid];

	/* Disable IRQ mode for the current channel*/
	writel(0x0, priv->spmi_obs + SPMI_CH_OFFSET(channel) + SPMI_REG_CONFIG);

	/* Prepare read command */
	reg |= SPMI_CMD_EXT_REG_READ_LONG << SPMI_CMD_OPCODE_SHIFT;
	reg |= (usid << SPMI_CMD_SLAVE_ID_SHIFT);
	reg |= (pid << SPMI_CMD_ADDR_SHIFT);
	reg |= (off << SPMI_CMD_ADDR_OFFSET_SHIFT);
	reg |= 1; /* byte count */

	/* Request read */
	writel(reg, priv->spmi_obs + SPMI_CH_OFFSET(channel) + SPMI_REG_CMD0);

	/* Wait till CMD DONE status */
	reg = 0;
	while (!reg) {
		reg = readl(priv->spmi_obs + SPMI_CH_OFFSET(channel) +
			    SPMI_REG_STATUS);
	}

	if (reg ^ SPMI_STATUS_DONE) {
		printf("SPMI read failure.\n");
		return -EIO;
	}

	/* Read the data */
	return readl(priv->spmi_obs + SPMI_CH_OFFSET(channel) +
		     SPMI_REG_RDATA) & 0xFF;
}

static struct dm_spmi_ops msm_spmi_ops = {
	.read = msm_spmi_read,
	.write = msm_spmi_write,
};

static int msm_spmi_probe(struct udevice *dev)
{
	struct udevice *parent = dev->parent;
	struct msm_spmi_priv *priv = dev_get_priv(dev);
	int node = dev_of_offset(dev);
	u32 hw_ver;
	bool is_v1;
	int i;

	priv->arb_chnl = devfdt_get_addr(dev);
	priv->spmi_core = fdtdec_get_addr_size_auto_parent(gd->fdt_blob,
			dev_of_offset(parent), node, "reg", 1, NULL, false);
	priv->spmi_obs = fdtdec_get_addr_size_auto_parent(gd->fdt_blob,
			dev_of_offset(parent), node, "reg", 2, NULL, false);

	hw_ver = readl(priv->arb_chnl + PMIC_ARB_VERSION - 0x800);
	is_v1  = (hw_ver < PMIC_ARB_VERSION_V2_MIN);

	dev_dbg(dev, "PMIC Arb Version-%d (0x%x)\n", (is_v1 ? 1 : 2), hw_ver);

	if (priv->arb_chnl == FDT_ADDR_T_NONE ||
	    priv->spmi_core == FDT_ADDR_T_NONE ||
	    priv->spmi_obs == FDT_ADDR_T_NONE)
		return -EINVAL;

	/* Scan peripherals connected to each SPMI channel */
	for (i = 0; i < SPMI_MAX_PERIPH ; i++) {
		uint32_t periph = readl(priv->arb_chnl + ARB_CHANNEL_OFFSET(i));
		uint8_t slave_id = (periph & 0xf0000) >> 16;
		uint8_t pid = (periph & 0xff00) >> 8;

		priv->channel_map[slave_id][pid] = i;
	}
	return 0;
}

static const struct udevice_id msm_spmi_ids[] = {
	{ .compatible = "qcom,spmi-pmic-arb" },
	{ }
};

U_BOOT_DRIVER(msm_spmi) = {
	.name = "msm_spmi",
	.id = UCLASS_SPMI,
	.of_match = msm_spmi_ids,
	.ops = &msm_spmi_ops,
	.probe = msm_spmi_probe,
	.priv_auto_alloc_size = sizeof(struct msm_spmi_priv),
};
