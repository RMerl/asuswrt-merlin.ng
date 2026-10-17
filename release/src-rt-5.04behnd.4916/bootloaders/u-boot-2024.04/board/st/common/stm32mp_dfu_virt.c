// SPDX-License-Identifier: GPL-2.0-or-later OR BSD-3-Clause
/*
 * Copyright (C) 2023, STMicroelectronics - All Rights Reserved
 */

#include <common.h>
#include <dfu.h>
#include <dm.h>
#include <misc.h>
#include <asm/arch/stm32prog.h>
#include <power/stpmic1.h>

static int dfu_otp_read(u64 offset, u8 *buffer, long *size)
{
	struct udevice *dev;
	int ret;

	ret = uclass_get_device_by_driver(UCLASS_MISC,
					  DM_DRIVER_GET(stm32mp_bsec),
					  &dev);
	if (ret)
		return ret;

	ret = misc_read(dev, offset + STM32_BSEC_OTP_OFFSET, buffer, *size);
	if (ret >= 0) {
		*size = ret;
		ret = 0;
	}

	return 0;
}

static int dfu_pmic_read(u64 offset, u8 *buffer, long *size)
{
	int ret;
	struct udevice *dev;

	if (!IS_ENABLED(CONFIG_PMIC_STPMIC1)) {
		log_err("PMIC update not supported");
		return -EOPNOTSUPP;
	}

	ret = uclass_get_device_by_driver(UCLASS_MISC,
					  DM_DRIVER_GET(stpmic1_nvm),
					  &dev);
	if (ret)
		return ret;

	ret = misc_read(dev, 0xF8 + offset, buffer, *size);
	if (ret >= 0) {
		*size = ret;
		ret = 0;
	}
	if (ret == -EACCES) {
		*size = 0;
		ret = 0;
	}

	return ret;
}

int dfu_read_medium_virt(struct dfu_entity *dfu, u64 offset,
			 void *buf, long *len)
{
	switch (dfu->data.virt.dev_num) {
	case 0x0:
		return dfu_otp_read(offset, buf, len);
	case 0x1:
		return dfu_pmic_read(offset, buf, len);
	}

	if (IS_ENABLED(CONFIG_CMD_STM32PROG_USB) &&
	    dfu->data.virt.dev_num >= STM32PROG_VIRT_FIRST_DEV_NUM)
		return stm32prog_read_medium_virt(dfu, offset, buf, len);

	*len = 0;
	return 0;
}

int dfu_write_medium_virt(struct dfu_entity *dfu, u64 offset,
			  void *buf, long *len)
{
	if (IS_ENABLED(CONFIG_CMD_STM32PROG_USB) &&
	    dfu->data.virt.dev_num >= STM32PROG_VIRT_FIRST_DEV_NUM)
		return stm32prog_write_medium_virt(dfu, offset, buf, len);

	return -EOPNOTSUPP;
}

int dfu_get_medium_size_virt(struct dfu_entity *dfu, u64 *size)
{
	if (IS_ENABLED(CONFIG_CMD_STM32PROG_USB) &&
	    dfu->data.virt.dev_num >= STM32PROG_VIRT_FIRST_DEV_NUM)
		return stm32prog_get_medium_size_virt(dfu, size);

	*size = SZ_1K;

	return 0;
}
