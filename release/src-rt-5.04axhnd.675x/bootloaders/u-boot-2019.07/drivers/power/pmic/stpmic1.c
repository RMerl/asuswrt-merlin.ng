// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2018, STMicroelectronics - All Rights Reserved
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <i2c.h>
#include <sysreset.h>
#include <dm/device.h>
#include <dm/lists.h>
#include <power/pmic.h>
#include <power/stpmic1.h>

#define STPMIC1_NUM_OF_REGS 0x100

#define STPMIC1_NVM_SIZE 8
#define STPMIC1_NVM_POLL_TIMEOUT 100000
#define STPMIC1_NVM_START_ADDRESS 0xf8

enum pmic_nvm_op {
	SHADOW_READ,
	SHADOW_WRITE,
	NVM_READ,
	NVM_WRITE,
};

#if CONFIG_IS_ENABLED(DM_REGULATOR)
static const struct pmic_child_info stpmic1_children_info[] = {
	{ .prefix = "ldo", .driver = "stpmic1_ldo" },
	{ .prefix = "buck", .driver = "stpmic1_buck" },
	{ .prefix = "vref_ddr", .driver = "stpmic1_vref_ddr" },
	{ .prefix = "pwr_sw", .driver = "stpmic1_pwr_sw" },
	{ .prefix = "boost", .driver = "stpmic1_boost" },
	{ },
};
#endif /* DM_REGULATOR */

static int stpmic1_reg_count(struct udevice *dev)
{
	return STPMIC1_NUM_OF_REGS;
}

static int stpmic1_write(struct udevice *dev, uint reg, const uint8_t *buff,
			 int len)
{
	int ret;

	ret = dm_i2c_write(dev, reg, buff, len);
	if (ret)
		dev_err(dev, "%s: failed to write register %#x :%d",
			__func__, reg, ret);

	return ret;
}

static int stpmic1_read(struct udevice *dev, uint reg, uint8_t *buff, int len)
{
	int ret;

	ret = dm_i2c_read(dev, reg, buff, len);
	if (ret)
		dev_err(dev, "%s: failed to read register %#x : %d",
			__func__, reg, ret);

	return ret;
}

static int stpmic1_bind(struct udevice *dev)
{
#if CONFIG_IS_ENABLED(DM_REGULATOR)
	ofnode regulators_node;
	int children;

	regulators_node = dev_read_subnode(dev, "regulators");
	if (!ofnode_valid(regulators_node)) {
		dev_dbg(dev, "regulators subnode not found!");
		return -ENXIO;
	}
	dev_dbg(dev, "found regulators subnode\n");

	children = pmic_bind_children(dev, regulators_node,
				      stpmic1_children_info);
	if (!children)
		dev_dbg(dev, "no child found\n");
#endif /* DM_REGULATOR */

	if (CONFIG_IS_ENABLED(SYSRESET))
		return device_bind_driver(dev, "stpmic1-sysreset",
					  "stpmic1-sysreset", NULL);

	return 0;
}

static struct dm_pmic_ops stpmic1_ops = {
	.reg_count = stpmic1_reg_count,
	.read = stpmic1_read,
	.write = stpmic1_write,
};

static const struct udevice_id stpmic1_ids[] = {
	{ .compatible = "st,stpmic1" },
	{ }
};

U_BOOT_DRIVER(pmic_stpmic1) = {
	.name = "stpmic1_pmic",
	.id = UCLASS_PMIC,
	.of_match = stpmic1_ids,
	.bind = stpmic1_bind,
	.ops = &stpmic1_ops,
};

#ifndef CONFIG_SPL_BUILD
static int stpmic1_nvm_rw(u8 addr, u8 *buf, int buf_len, enum pmic_nvm_op op)
{
	struct udevice *dev;
	unsigned long timeout;
	u8 cmd = STPMIC1_NVM_CMD_READ;
	int ret;

	ret = uclass_get_device_by_driver(UCLASS_PMIC,
					  DM_GET_DRIVER(pmic_stpmic1), &dev);
	if (ret)
		/* No PMIC on power discrete board */
		return -EOPNOTSUPP;

	if (addr < STPMIC1_NVM_START_ADDRESS)
		return -EACCES;

	if (op == SHADOW_READ)
		return pmic_read(dev, addr, buf, buf_len);

	if (op == SHADOW_WRITE)
		return pmic_write(dev, addr, buf, buf_len);

	if (op == NVM_WRITE) {
		cmd = STPMIC1_NVM_CMD_PROGRAM;

		ret = pmic_write(dev, addr, buf, buf_len);
		if (ret < 0)
			return ret;
	}

	ret = pmic_reg_read(dev, STPMIC1_NVM_CR);
	if (ret < 0)
		return ret;

	ret = pmic_reg_write(dev, STPMIC1_NVM_CR, ret | cmd);
	if (ret < 0)
		return ret;

	timeout = timer_get_us() + STPMIC1_NVM_POLL_TIMEOUT;
	for (;;) {
		ret = pmic_reg_read(dev, STPMIC1_NVM_SR);
		if (ret < 0)
			return ret;

		if (!(ret & STPMIC1_NVM_BUSY))
			break;

		if (time_after(timer_get_us(), timeout))
			break;
	}

	if (ret & STPMIC1_NVM_BUSY)
		return -ETIMEDOUT;

	if (op == NVM_READ) {
		ret = pmic_read(dev, addr, buf, buf_len);
		if (ret < 0)
			return ret;
	}

	return 0;
}

int stpmic1_shadow_read_byte(u8 addr, u8 *buf)
{
	return stpmic1_nvm_rw(addr, buf, 1, SHADOW_READ);
}

int stpmic1_shadow_write_byte(u8 addr, u8 *buf)
{
	return stpmic1_nvm_rw(addr, buf, 1, SHADOW_WRITE);
}

int stpmic1_nvm_read_byte(u8 addr, u8 *buf)
{
	return stpmic1_nvm_rw(addr, buf, 1, NVM_READ);
}

int stpmic1_nvm_write_byte(u8 addr, u8 *buf)
{
	return stpmic1_nvm_rw(addr, buf, 1, NVM_WRITE);
}

int stpmic1_nvm_read_all(u8 *buf, int buf_len)
{
	if (buf_len != STPMIC1_NVM_SIZE)
		return -EINVAL;

	return stpmic1_nvm_rw(STPMIC1_NVM_START_ADDRESS,
			     buf, buf_len, NVM_READ);
}

int stpmic1_nvm_write_all(u8 *buf, int buf_len)
{
	if (buf_len != STPMIC1_NVM_SIZE)
		return -EINVAL;

	return stpmic1_nvm_rw(STPMIC1_NVM_START_ADDRESS,
			     buf, buf_len, NVM_WRITE);
}
#endif /* CONFIG_SPL_BUILD */

#ifdef CONFIG_SYSRESET
static int stpmic1_sysreset_request(struct udevice *dev, enum sysreset_t type)
{
	struct udevice *pmic_dev;
	int ret;

	if (type != SYSRESET_POWER)
		return -EPROTONOSUPPORT;

	ret = uclass_get_device_by_driver(UCLASS_PMIC,
					  DM_GET_DRIVER(pmic_stpmic1),
					  &pmic_dev);

	if (ret)
		return -EOPNOTSUPP;

	ret = pmic_reg_read(pmic_dev, STPMIC1_MAIN_CR);
	if (ret < 0)
		return ret;

	ret = pmic_reg_write(pmic_dev, STPMIC1_MAIN_CR,
			     ret | STPMIC1_SWOFF | STPMIC1_RREQ_EN);
	if (ret < 0)
		return ret;

	return -EINPROGRESS;
}

static struct sysreset_ops stpmic1_sysreset_ops = {
	.request = stpmic1_sysreset_request,
};

U_BOOT_DRIVER(stpmic1_sysreset) = {
	.name = "stpmic1-sysreset",
	.id = UCLASS_SYSRESET,
	.ops = &stpmic1_sysreset_ops,
};
#endif
