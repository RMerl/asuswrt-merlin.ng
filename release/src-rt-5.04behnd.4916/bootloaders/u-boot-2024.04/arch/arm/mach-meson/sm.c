// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 Beniamino Galvani <b.galvani@gmail.com>
 *
 * Secure monitor calls.
 */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <regmap.h>
#include <sm.h>
#include <syscon.h>
#include <asm/arch/sm.h>
#include <asm/cache.h>
#include <asm/global_data.h>
#include <asm/ptrace.h>
#include <linux/bitops.h>
#include <linux/err.h>
#include <linux/kernel.h>
#include <linux/bitfield.h>
#include <meson/sm.h>

static inline struct udevice *meson_get_sm_device(void)
{
	struct udevice *dev;
	int err;

	err = uclass_first_device_err(UCLASS_SM, &dev);
	if (err) {
		pr_err("Mesom SM device not found\n");
		return ERR_PTR(err);
	}

	return dev;
}

ssize_t meson_sm_read_efuse(uintptr_t offset, void *buffer, size_t size)
{
	struct udevice *dev;
	struct pt_regs regs = { 0 };
	int err;

	dev = meson_get_sm_device();
	if (IS_ERR(dev))
		return PTR_ERR(dev);

	regs.regs[1] = offset;
	regs.regs[2] = size;

	err = sm_call_read(dev, buffer, size,
			   MESON_SMC_CMD_EFUSE_READ, &regs);
	if (err < 0)
		pr_err("Failed to read efuse memory (%d)\n", err);

	return err;
}

ssize_t meson_sm_write_efuse(uintptr_t offset, void *buffer, size_t size)
{
	struct udevice *dev;
	struct pt_regs regs = { 0 };
	int err;

	dev = meson_get_sm_device();
	if (IS_ERR(dev))
		return PTR_ERR(dev);

	regs.regs[1] = offset;
	regs.regs[2] = size;

	err = sm_call_write(dev, buffer, size,
			    MESON_SMC_CMD_EFUSE_WRITE, &regs);
	if (err < 0)
		pr_err("Failed to write efuse memory (%d)\n", err);

	return err;
}

#define SM_CHIP_ID_LENGTH	119
#define SM_CHIP_ID_OFFSET	4
#define SM_CHIP_ID_SIZE		12

int meson_sm_get_serial(void *buffer, size_t size)
{
	struct udevice *dev;
	struct pt_regs regs = { 0 };
	u8 id_buffer[SM_CHIP_ID_LENGTH];
	int err;

	dev = meson_get_sm_device();
	if (IS_ERR(dev))
		return PTR_ERR(dev);

	err = sm_call_read(dev, id_buffer, SM_CHIP_ID_LENGTH,
			   MESON_SMC_CMD_CHIP_ID_GET, &regs);
	if (err < 0)
		pr_err("Failed to read serial number (%d)\n", err);

	memcpy(buffer, id_buffer + SM_CHIP_ID_OFFSET, size);

	return 0;
}

#define AO_SEC_SD_CFG15		0xfc
#define REBOOT_REASON_MASK	GENMASK(15, 12)

int meson_sm_get_reboot_reason(void)
{
	struct regmap *regmap;
	int nodeoffset;
	ofnode node;
	unsigned int reason;

	/* find the offset of compatible node */
	nodeoffset = fdt_node_offset_by_compatible(gd->fdt_blob, -1,
						   "amlogic,meson-gx-ao-secure");
	if (nodeoffset < 0) {
		printf("%s: failed to get amlogic,meson-gx-ao-secure\n",
		       __func__);
		return -ENODEV;
	}

	/* get regmap from the syscon node */
	node = offset_to_ofnode(nodeoffset);
	regmap = syscon_node_to_regmap(node);
	if (IS_ERR(regmap)) {
		printf("%s: failed to get regmap\n", __func__);
		return -EINVAL;
	}

	regmap_read(regmap, AO_SEC_SD_CFG15, &reason);

	/* The SMC call is not used, we directly use AO_SEC_SD_CFG15 */
	return FIELD_GET(REBOOT_REASON_MASK, reason);
}

int meson_sm_pwrdm_set(size_t index, int cmd)
{
	struct udevice *dev;
	struct pt_regs regs = { 0 };
	int err;

	dev = meson_get_sm_device();
	if (IS_ERR(dev))
		return PTR_ERR(dev);

	regs.regs[1] = index;
	regs.regs[2] = cmd;

	err = sm_call(dev, MESON_SMC_CMD_PWRDM_SET, NULL, &regs);
	if (err)
		pr_err("Failed to %s power domain ind=%zu (%d)\n", cmd == PWRDM_ON ?
				"enable" : "disable", index, err);

	return err;
}
