// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2018, STMicroelectronics - All Rights Reserved
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <i2c.h>
#include <misc.h>
#include <sysreset.h>
#include <time.h>
#include <dm/device.h>
#include <dm/device_compat.h>
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
	{ .prefix = "vref-ddr", .driver = "stpmic1_vref_ddr" },
	{ .prefix = "pwr_sw", .driver = "stpmic1_pwr_sw" },
	{ .prefix = "pwr-sw", .driver = "stpmic1_pwr_sw" },
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
	int ret;
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

	if (!IS_ENABLED(CONFIG_SPL_BUILD)) {
		ret = device_bind_driver(dev, "stpmic1-nvm",
					 "stpmic1-nvm", NULL);
		if (ret)
			return ret;
	}

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
static int stpmic1_nvm_rw(struct udevice *dev, u8 addr, u8 *buf, int buf_len,
			  enum pmic_nvm_op op)
{
	unsigned long timeout;
	u8 cmd = STPMIC1_NVM_CMD_READ;
	int ret, len = buf_len;

	if (addr < STPMIC1_NVM_START_ADDRESS)
		return -EACCES;
	if (addr + buf_len > STPMIC1_NVM_START_ADDRESS + STPMIC1_NVM_SIZE)
		len = STPMIC1_NVM_START_ADDRESS + STPMIC1_NVM_SIZE - addr;

	if (op == SHADOW_READ) {
		ret = pmic_read(dev, addr, buf, len);
		if (ret < 0)
			return ret;
		else
			return len;
	}

	if (op == SHADOW_WRITE) {
		ret = pmic_write(dev, addr, buf, len);
		if (ret < 0)
			return ret;
		else
			return len;
	}

	if (op == NVM_WRITE) {
		cmd = STPMIC1_NVM_CMD_PROGRAM;

		ret = pmic_write(dev, addr, buf, len);
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
		ret = pmic_read(dev, addr, buf, len);
		if (ret < 0)
			return ret;
	}

	return len;
}

static int stpmic1_nvm_read(struct udevice *dev, int offset,
			    void *buf, int size)
{
	enum pmic_nvm_op op = NVM_READ;

	if (offset < 0) {
		op = SHADOW_READ;
		offset = -offset;
	}

	return stpmic1_nvm_rw(dev->parent, offset, buf, size, op);
}

static int stpmic1_nvm_write(struct udevice *dev, int offset,
			     const void *buf, int size)
{
	enum pmic_nvm_op op = NVM_WRITE;

	if (offset < 0) {
		op = SHADOW_WRITE;
		offset = -offset;
	}

	return stpmic1_nvm_rw(dev->parent, offset, (void *)buf, size, op);
}

static const struct misc_ops stpmic1_nvm_ops = {
	.read = stpmic1_nvm_read,
	.write = stpmic1_nvm_write,
};

U_BOOT_DRIVER(stpmic1_nvm) = {
	.name = "stpmic1-nvm",
	.id = UCLASS_MISC,
	.ops = &stpmic1_nvm_ops,
};
#endif /* CONFIG_SPL_BUILD */

#ifdef CONFIG_SYSRESET
static int stpmic1_sysreset_request(struct udevice *dev, enum sysreset_t type)
{
	struct udevice *pmic_dev = dev->parent;
	int ret;

	if (type != SYSRESET_POWER && type != SYSRESET_POWER_OFF)
		return -EPROTONOSUPPORT;

	ret = pmic_reg_read(pmic_dev, STPMIC1_MAIN_CR);
	if (ret < 0)
		return ret;

	ret |= STPMIC1_SWOFF;
	ret &= ~STPMIC1_RREQ_EN;
	/* request Power Cycle */
	if (type == SYSRESET_POWER)
		ret |= STPMIC1_RREQ_EN;

	ret = pmic_reg_write(pmic_dev, STPMIC1_MAIN_CR, ret);
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
