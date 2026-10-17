// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018, STMicroelectronics - All Rights Reserved
 */

#include <common.h>
#include <command.h>
#include <fuse.h>
#include <misc.h>
#include <errno.h>
#include <dm/device.h>
#include <dm/uclass.h>
#include <power/stpmic1.h>

#define STM32MP_OTP_BANK	0
#define STM32MP_NVM_BANK	1

/*
 * The 'fuse' command API
 */
int fuse_read(u32 bank, u32 word, u32 *val)
{
	int ret;
	struct udevice *dev;

	switch (bank) {
	case STM32MP_OTP_BANK:
		ret = uclass_get_device_by_driver(UCLASS_MISC,
						  DM_DRIVER_GET(stm32mp_bsec),
						  &dev);
		if (ret)
			return ret;
		ret = misc_read(dev, word * 4 + STM32_BSEC_SHADOW_OFFSET,
				val, 4);
		if (ret != 4)
			ret = -EINVAL;
		else
			ret = 0;
		break;

#ifdef CONFIG_PMIC_STPMIC1
	case STM32MP_NVM_BANK:
		ret = uclass_get_device_by_driver(UCLASS_MISC,
						  DM_DRIVER_GET(stpmic1_nvm),
						  &dev);
		if (ret)
			return ret;
		*val = 0;
		ret = misc_read(dev, -word, val, 1);
		if (ret != 1)
			ret = -EINVAL;
		else
			ret = 0;
		break;
#endif /* CONFIG_PMIC_STPMIC1 */

	default:
		printf("stm32mp %s: wrong value for bank %i\n", __func__, bank);
		ret = -EINVAL;
		break;
	}

	return ret;
}

int fuse_prog(u32 bank, u32 word, u32 val)
{
	struct udevice *dev;
	int ret;

	switch (bank) {
	case STM32MP_OTP_BANK:
		ret = uclass_get_device_by_driver(UCLASS_MISC,
						  DM_DRIVER_GET(stm32mp_bsec),
						  &dev);
		if (ret)
			return ret;
		ret = misc_write(dev, word * 4 + STM32_BSEC_OTP_OFFSET,
				 &val, 4);
		if (ret != 4)
			ret = -EINVAL;
		else
			ret = 0;
		break;

#ifdef CONFIG_PMIC_STPMIC1
	case STM32MP_NVM_BANK:
		ret = uclass_get_device_by_driver(UCLASS_MISC,
						  DM_DRIVER_GET(stpmic1_nvm),
						  &dev);
		if (ret)
			return ret;
		ret = misc_write(dev, word, &val, 1);
		if (ret != 1)
			ret = -EINVAL;
		else
			ret = 0;
		break;
#endif /* CONFIG_PMIC_STPMIC1 */

	default:
		printf("stm32mp %s: wrong value for bank %i\n", __func__, bank);
		ret = -EINVAL;
		break;
	}

	return ret;
}

int fuse_sense(u32 bank, u32 word, u32 *val)
{
	struct udevice *dev;
	int ret;

	switch (bank) {
	case STM32MP_OTP_BANK:
		ret = uclass_get_device_by_driver(UCLASS_MISC,
						  DM_DRIVER_GET(stm32mp_bsec),
						  &dev);
		if (ret)
			return ret;
		ret = misc_read(dev, word * 4 + STM32_BSEC_OTP_OFFSET, val, 4);
		if (ret != 4)
			ret = -EINVAL;
		else
			ret = 0;
		break;

#ifdef CONFIG_PMIC_STPMIC1
	case STM32MP_NVM_BANK:
		ret = uclass_get_device_by_driver(UCLASS_MISC,
						  DM_DRIVER_GET(stpmic1_nvm),
						  &dev);
		if (ret)
			return ret;
		*val = 0;
		ret = misc_read(dev, word, val, 1);
		if (ret != 1)
			ret = -EINVAL;
		else
			ret = 0;
		break;
#endif /* CONFIG_PMIC_STPMIC1 */

	default:
		printf("stm32mp %s: wrong value for bank %i\n", __func__, bank);
		ret = -EINVAL;
		break;
	}

	return ret;
}

int fuse_override(u32 bank, u32 word, u32 val)
{
	struct udevice *dev;
	int ret;

	switch (bank) {
	case STM32MP_OTP_BANK:
		ret = uclass_get_device_by_driver(UCLASS_MISC,
						  DM_DRIVER_GET(stm32mp_bsec),
						  &dev);
		if (ret)
			return ret;
		ret = misc_write(dev, word * 4 + STM32_BSEC_SHADOW_OFFSET,
				 &val, 4);
		if (ret != 4)
			ret = -EINVAL;
		else
			ret = 0;
		break;

#ifdef CONFIG_PMIC_STPMIC1
	case STM32MP_NVM_BANK:
		ret = uclass_get_device_by_driver(UCLASS_MISC,
						  DM_DRIVER_GET(stpmic1_nvm),
						  &dev);
		if (ret)
			return ret;
		ret = misc_write(dev, -word, &val, 1);
		if (ret != 1)
			ret = -EINVAL;
		else
			ret = 0;
		break;
#endif /* CONFIG_PMIC_STPMIC1 */

	default:
		printf("stm32mp %s: wrong value for bank %i\n",
		       __func__, bank);
		ret = -EINVAL;
		break;
	}

	return ret;
}
