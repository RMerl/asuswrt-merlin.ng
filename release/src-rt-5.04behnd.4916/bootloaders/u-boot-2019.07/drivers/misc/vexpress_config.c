// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Arm Ltd
 * Author: Liviu Dudau <liviu.dudau@foss.arm.com>
 *
 */
#include <common.h>
#include <dm.h>
#include <dm/read.h>
#include <asm/io.h>
#include <linux/delay.h>
#include <misc.h>

#define SYS_CFGDATA		0xa0

#define SYS_CFGCTRL		0xa4
#define SYS_CFGCTRL_START	BIT(31)
#define SYS_CFGCTRL_WRITE	BIT(30)

#define SYS_CFGSTAT		0xa8
#define SYS_CFGSTAT_ERR		BIT(1)
#define SYS_CFGSTAT_COMPLETE	BIT(0)

struct vexpress_config_sysreg {
	phys_addr_t addr;
	u32 site;
};

static int vexpress_config_exec(struct vexpress_config_sysreg *syscfg,
				bool write, void *buf, int size)
{
	u32 cmd, status, tries = 100;

	cmd = (*(u32 *)buf) | SYS_CFGCTRL_START | (syscfg->site << 16);

	if (!write) {
		/* write a canary in the data register for reads */
		writel(0xdeadbeef, syscfg->addr + SYS_CFGDATA);
	} else {
		cmd |= SYS_CFGCTRL_WRITE;
		writel(((u32 *)buf)[1], syscfg->addr + SYS_CFGDATA);
	}
	writel(0, syscfg->addr + SYS_CFGSTAT);
	writel(cmd, syscfg->addr + SYS_CFGCTRL);

	/* completion of command takes ages, go to sleep (150us) */
	do {
		udelay(150);
		status = readl(syscfg->addr + SYS_CFGSTAT);
		if (status & SYS_CFGSTAT_ERR)
			return -EFAULT;
	} while (--tries && !(status & SYS_CFGSTAT_COMPLETE));

	if (!tries)
		return -ETIMEDOUT;

	if (!write)
		(*(u32 *)buf) = readl(syscfg->addr + SYS_CFGDATA);

	return 0;
}

static int vexpress_config_read(struct udevice *dev,
				int offset, void *buf, int size)
{
	struct vexpress_config_sysreg *priv = dev_get_uclass_priv(dev);

	if (size != sizeof(u32))
		return -EINVAL;

	return vexpress_config_exec(priv, false, buf, size);
}

static int vexpress_config_write(struct udevice *dev,
				 int offset, const void *buf, int size)
{
	struct vexpress_config_sysreg *priv = dev_get_uclass_priv(dev);

	if (size != sizeof(u32) * 2)
		return -EINVAL;

	return vexpress_config_exec(priv, true, (void *)buf, size);
}

static struct misc_ops vexpress_config_ops = {
	.read = vexpress_config_read,
	.write = vexpress_config_write,
};

static int vexpress_config_probe(struct udevice *dev)
{
	struct ofnode_phandle_args args;
	struct vexpress_config_sysreg *priv;
	const char *prop;
	int err, prop_size;

	err = dev_read_phandle_with_args(dev, "arm,vexpress,config-bridge",
					 NULL, 0, 0, &args);
	if (err)
		return err;

	prop = ofnode_get_property(args.node, "compatible", &prop_size);
	if (!prop || (strncmp(prop, "arm,vexpress-sysreg", 19) != 0))
		return -ENOENT;

	priv = calloc(1, sizeof(*priv));
	if (!priv)
		return -ENOMEM;

	dev->uclass_priv = priv;
	priv->addr = ofnode_get_addr(args.node);

	return dev_read_u32(dev, "arm,vexpress,site", &priv->site);
}

static const struct udevice_id vexpress_config_ids[] = {
	{ .compatible = "arm,vexpress,config-bus" },
	{ }
};

U_BOOT_DRIVER(vexpress_config_drv) = {
	.name = "vexpress_config_bus",
	.id = UCLASS_MISC,
	.of_match = vexpress_config_ids,
	.bind = dm_scan_fdt_dev,
	.probe = vexpress_config_probe,
	.ops = &vexpress_config_ops,
};
