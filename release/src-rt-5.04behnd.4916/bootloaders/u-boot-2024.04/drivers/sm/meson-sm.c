// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2023 SberDevices, Inc.
 *
 * Author: Alexey Romanov <avromanov@salutedevices.com>
 */

#include <common.h>
#include <dm.h>
#include <regmap.h>
#include <sm.h>
#include <sm-uclass.h>
#include <stdlib.h>
#include <syscon.h>
#include <asm/ptrace.h>
#include <asm/system.h>
#include <meson/sm.h>
#include <linux/bitfield.h>
#include <linux/err.h>
#include <linux/sizes.h>

struct meson_sm_cmd {
	u32 smc_id;
};

#define SET_CMD(index, id)	\
	[index] = {		\
		.smc_id = (id),	\
	}

struct meson_sm_data {
	u32 cmd_get_shmem_in;
	u32 cmd_get_shmem_out;
	unsigned int shmem_size;
	struct meson_sm_cmd cmd[];
};

struct meson_sm_priv {
	void *sm_shmem_in;
	void *sm_shmem_out;
	const struct meson_sm_data *data;
};

static unsigned long __meson_sm_call(u32 cmd, const struct pt_regs *args)
{
	struct pt_regs r = *args;

	r.regs[0] = cmd;
	smc_call(&r);

	return r.regs[0];
};

static u32 meson_sm_get_cmd(const struct meson_sm_data *data,
			    u32 cmd_index)
{
	struct meson_sm_cmd cmd;

	if (cmd_index >= MESON_SMC_CMD_COUNT)
		return 0;

	cmd = data->cmd[cmd_index];
	return cmd.smc_id;
}

static int meson_sm_call(struct udevice *dev, u32 cmd_index, s32 *retval,
			 struct pt_regs *args)
{
	struct meson_sm_priv *priv = dev_get_priv(dev);
	u32 cmd, ret;

	cmd = meson_sm_get_cmd(priv->data, cmd_index);
	if (!cmd)
		return -ENOENT;

	ret = __meson_sm_call(cmd, args);
	if (retval)
		*retval = ret;

	return 0;
}

static int meson_sm_call_read(struct udevice *dev, void *buffer, size_t size,
			      u32 cmd_index, struct pt_regs *args)
{
	struct meson_sm_priv *priv = dev_get_priv(dev);
	s32 nbytes;
	int ret;

	if (!buffer || size > priv->data->shmem_size)
		return -EINVAL;

	ret = meson_sm_call(dev, cmd_index, &nbytes, args);
	if (ret)
		return ret;

	if (nbytes < 0 || nbytes > size)
		return -ENOBUFS;

	/* In some cases (for example GET_CHIP_ID command),
	 * SMC doesn't return the number of bytes read, even
	 * though the bytes were actually read into sm_shmem_out.
	 * So this check is needed.
	 */
	ret = nbytes;
	if (!nbytes)
		nbytes = size;

	memcpy(buffer, priv->sm_shmem_out, nbytes);

	return ret;
}

static int meson_sm_call_write(struct udevice *dev, void *buffer, size_t size,
			       u32 cmd_index, struct pt_regs *args)
{
	struct meson_sm_priv *priv = dev_get_priv(dev);
	s32 nbytes;
	int ret;

	if (!buffer || size > priv->data->shmem_size)
		return -EINVAL;

	memcpy(priv->sm_shmem_in, buffer, size);

	ret = meson_sm_call(dev, cmd_index, &nbytes, args);
	if (ret)
		return ret;

	if (nbytes <= 0 || nbytes > size)
		return -EIO;

	return nbytes;
}

static int meson_sm_probe(struct udevice *dev)
{
	struct meson_sm_priv *priv = dev_get_priv(dev);
	struct pt_regs regs = { 0 };

	priv->data = (struct meson_sm_data *)dev_get_driver_data(dev);
	if (!priv->data)
		return -EINVAL;

	priv->sm_shmem_in =
		(void *)__meson_sm_call(priv->data->cmd_get_shmem_in, &regs);

	if (!priv->sm_shmem_in)
		return -ENOMEM;

	priv->sm_shmem_out =
		(void *)__meson_sm_call(priv->data->cmd_get_shmem_out, &regs);

	if (!priv->sm_shmem_out)
		return -ENOMEM;

	pr_debug("meson sm driver probed\n"
		 "shmem_in addr: 0x%p, shmem_out addr: 0x%p\n",
		 priv->sm_shmem_in,
		 priv->sm_shmem_out);

	return 0;
}

static const struct meson_sm_data meson_sm_gxbb_data = {
	.cmd_get_shmem_in  = 0x82000020,
	.cmd_get_shmem_out = 0x82000021,
	.shmem_size = SZ_4K,
	.cmd = {
		SET_CMD(MESON_SMC_CMD_EFUSE_READ,  0x82000030),
		SET_CMD(MESON_SMC_CMD_EFUSE_WRITE, 0x82000031),
		SET_CMD(MESON_SMC_CMD_CHIP_ID_GET, 0x82000044),
		SET_CMD(MESON_SMC_CMD_PWRDM_SET,   0x82000093),
	},
};

static const struct udevice_id meson_sm_ids[] = {
	{
		.compatible = "amlogic,meson-gxbb-sm",
		.data = (ulong)&meson_sm_gxbb_data,
	},
	{ }
};

static const struct sm_ops sm_ops = {
	.sm_call = meson_sm_call,
	.sm_call_read = meson_sm_call_read,
	.sm_call_write = meson_sm_call_write,
};

U_BOOT_DRIVER(meson_sm) = {
	.name = "meson_sm",
	.id = UCLASS_SM,
	.of_match = meson_sm_ids,
	.probe = meson_sm_probe,
	.bind = dm_scan_fdt_dev,
	.priv_auto = sizeof(struct meson_sm_priv),
	.ops = &sm_ops,
};
