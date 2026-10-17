// SPDX-License-Identifier: GPL-2.0+
// (C) 2022 Pali Rohár <pali@kernel.org>

#include <common.h>
#include <dm.h>
#include <i2c.h>
#include <asm/gpio.h>
#include <linux/log2.h>

enum commands_e {
	CMD_GET_STATUS_WORD                 = 0x01,
	CMD_GENERAL_CONTROL                 = 0x02,

	/* available if STS_FEATURES_SUPPORTED bit set in status word */
	CMD_GET_FEATURES                    = 0x10,

	/* available if FEAT_EXT_CMDS bit is set in features */
	CMD_GET_EXT_STATUS_DWORD            = 0x11,

	/* available if FEAT_EXT_CMDS and FEAT_PERIPH_MCU bits are set in featurs */
	CMD_EXT_CONTROL                     = 0x12,
	CMD_GET_EXT_CONTROL_STATUS          = 0x13,
};

/* CMD_GET_STATUS_WORD */
enum sts_word_e {
	STS_MCU_TYPE_MASK                = GENMASK(1, 0),
	STS_MCU_TYPE_STM32               = 0,
	STS_MCU_TYPE_GD32                = 1,
	STS_MCU_TYPE_MKL                 = 2,
	STS_FEATURES_SUPPORTED           = BIT(2),
	STS_USER_REGULATOR_NOT_SUPPORTED = BIT(3),
	STS_CARD_DET                     = BIT(4),
	STS_MSATA_IND                    = BIT(5),
	STS_USB30_OVC                    = BIT(6),
	STS_USB31_OVC                    = BIT(7),
	STS_USB30_PWRON                  = BIT(8),
	STS_USB31_PWRON                  = BIT(9),
	STS_ENABLE_4V5                   = BIT(10),
	STS_BUTTON_MODE                  = BIT(11),
	STS_BUTTON_PRESSED               = BIT(12),
	STS_BUTTON_COUNTER_MASK          = GENMASK(15, 13)
};

/* CMD_GENERAL_CONTROL */
enum ctl_byte_e {
	CTL_LIGHT_RST   = BIT(0),
	CTL_HARD_RST    = BIT(1),
	/*CTL_RESERVED    = BIT(2),*/
	CTL_USB30_PWRON = BIT(3),
	CTL_USB31_PWRON = BIT(4),
	CTL_ENABLE_4V5  = BIT(5),
	CTL_BUTTON_MODE = BIT(6),
	CTL_BOOTLOADER  = BIT(7)
};

/* CMD_GET_FEATURES */
enum features_e {
	FEAT_PERIPH_MCU         = BIT(0),
	FEAT_EXT_CMDS           = BIT(1),
};

struct turris_omnia_mcu_info {
	u16 features;
};

static int turris_omnia_mcu_get_function(struct udevice *dev, uint offset)
{
	struct turris_omnia_mcu_info *info = dev_get_plat(dev);

	switch (offset) {
	/* bank 0 */
	case 0 ... 15:
		switch (offset) {
		case ilog2(STS_USB30_PWRON):
		case ilog2(STS_USB31_PWRON):
		case ilog2(STS_ENABLE_4V5):
		case ilog2(STS_BUTTON_MODE):
			return GPIOF_OUTPUT;
		default:
			return GPIOF_INPUT;
		}

	/* bank 1 - supported only when FEAT_EXT_CMDS is set */
	case (16 + 0) ... (16 + 31):
		if (!(info->features & FEAT_EXT_CMDS))
			return -EINVAL;
		return GPIOF_INPUT;

	/* bank 2 - supported only when FEAT_EXT_CMDS and FEAT_PERIPH_MCU is set */
	case (16 + 32 + 0) ... (16 + 32 + 15):
		if (!(info->features & FEAT_EXT_CMDS))
			return -EINVAL;
		if (!(info->features & FEAT_PERIPH_MCU))
			return -EINVAL;
		return GPIOF_OUTPUT;

	default:
		return -EINVAL;
	}
}

static int turris_omnia_mcu_get_value(struct udevice *dev, uint offset)
{
	struct turris_omnia_mcu_info *info = dev_get_plat(dev);
	u8 val16[2];
	u8 val32[4];
	int ret;

	switch (offset) {
	/* bank 0 */
	case 0 ... 15:
		ret = dm_i2c_read(dev, CMD_GET_STATUS_WORD, val16, 2);
		if (ret)
			return ret;
		return ((((u16)val16[1] << 8) | val16[0]) >> offset) & 0x1;

	/* bank 1 - supported only when FEAT_EXT_CMDS is set */
	case (16 + 0) ... (16 + 31):
		if (!(info->features & FEAT_EXT_CMDS))
			return -EINVAL;
		ret = dm_i2c_read(dev, CMD_GET_EXT_STATUS_DWORD, val32, 4);
		if (ret)
			return ret;
		return ((((u32)val32[3] << 24) | ((u32)val32[2] << 16) |
			 ((u32)val32[1] << 8) | val32[0]) >> (offset - 16)) & 0x1;

	/* bank 2 - supported only when FEAT_EXT_CMDS and FEAT_PERIPH_MCU is set */
	case (16 + 32 + 0) ... (16 + 32 + 15):
		if (!(info->features & FEAT_EXT_CMDS))
			return -EINVAL;
		if (!(info->features & FEAT_PERIPH_MCU))
			return -EINVAL;
		ret = dm_i2c_read(dev, CMD_GET_EXT_CONTROL_STATUS, val16, 2);
		if (ret)
			return ret;
		return ((((u16)val16[1] << 8) | val16[0]) >> (offset - 16 - 32)) & 0x1;

	default:
		return -EINVAL;
	}
}

static int turris_omnia_mcu_set_value(struct udevice *dev, uint offset, int value)
{
	struct turris_omnia_mcu_info *info = dev_get_plat(dev);
	u8 val16[2];
	u8 val32[4];

	switch (offset) {
	/* bank 0 */
	case 0 ... 15:
		switch (offset) {
		case ilog2(STS_USB30_PWRON):
			val16[1] = CTL_USB30_PWRON;
			break;
		case ilog2(STS_USB31_PWRON):
			val16[1] = CTL_USB31_PWRON;
			break;
		case ilog2(STS_ENABLE_4V5):
			val16[1] = CTL_ENABLE_4V5;
			break;
		case ilog2(STS_BUTTON_MODE):
			val16[1] = CTL_BUTTON_MODE;
			break;
		default:
			return -EINVAL;
		}
		val16[0] = value ? val16[1] : 0;
		return dm_i2c_write(dev, CMD_GENERAL_CONTROL, val16, sizeof(val16));

	/* bank 2 - supported only when FEAT_EXT_CMDS and FEAT_PERIPH_MCU is set */
	case (16 + 32 + 0) ... (16 + 32 + 15):
		if (!(info->features & FEAT_EXT_CMDS))
			return -EINVAL;
		if (!(info->features & FEAT_PERIPH_MCU))
			return -EINVAL;
		val32[3] = BIT(offset - 16 - 32) >> 8;
		val32[2] = BIT(offset - 16 - 32) & 0xff;
		val32[1] = value ? val32[3] : 0;
		val32[0] = value ? val32[2] : 0;
		return dm_i2c_write(dev, CMD_EXT_CONTROL, val32, sizeof(val32));

	default:
		return -EINVAL;
	}
}

static int turris_omnia_mcu_direction_input(struct udevice *dev, uint offset)
{
	int ret;

	ret = turris_omnia_mcu_get_function(dev, offset);
	if (ret < 0)
		return ret;
	else if (ret != GPIOF_INPUT)
		return -EOPNOTSUPP;

	return 0;
}

static int turris_omnia_mcu_direction_output(struct udevice *dev, uint offset, int value)
{
	int ret;

	ret = turris_omnia_mcu_get_function(dev, offset);
	if (ret < 0)
		return ret;
	else if (ret != GPIOF_OUTPUT)
		return -EOPNOTSUPP;

	return turris_omnia_mcu_set_value(dev, offset, value);
}

static int turris_omnia_mcu_xlate(struct udevice *dev, struct gpio_desc *desc,
				  struct ofnode_phandle_args *args)
{
	uint bank, gpio, flags, offset;
	int ret;

	if (args->args_count != 3)
		return -EINVAL;

	bank = args->args[0];
	gpio = args->args[1];
	flags = args->args[2];

	switch (bank) {
	case 0:
		if (gpio >= 16)
			return -EINVAL;
		offset = gpio;
		break;
	case 1:
		if (gpio >= 32)
			return -EINVAL;
		offset = 16 + gpio;
		break;
	case 2:
		if (gpio >= 16)
			return -EINVAL;
		offset = 16 + 32 + gpio;
		break;
	default:
		return -EINVAL;
	}

	ret = turris_omnia_mcu_get_function(dev, offset);
	if (ret < 0)
		return ret;

	desc->offset = offset;
	desc->flags = gpio_flags_xlate(flags);

	return 0;
}

static const struct dm_gpio_ops turris_omnia_mcu_ops = {
	.direction_input	= turris_omnia_mcu_direction_input,
	.direction_output	= turris_omnia_mcu_direction_output,
	.get_value		= turris_omnia_mcu_get_value,
	.set_value		= turris_omnia_mcu_set_value,
	.get_function		= turris_omnia_mcu_get_function,
	.xlate			= turris_omnia_mcu_xlate,
};

static int turris_omnia_mcu_probe(struct udevice *dev)
{
	struct turris_omnia_mcu_info *info = dev_get_plat(dev);
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	u16 status;
	u8 val[2];
	int ret;

	ret = dm_i2c_read(dev, CMD_GET_STATUS_WORD, val, 2);
	if (ret) {
		printf("Error: turris_omnia_mcu CMD_GET_STATUS_WORD failed: %d\n", ret);
		return ret;
	}

	status = ((u16)val[1] << 8) | val[0];

	if (status & STS_FEATURES_SUPPORTED) {
		ret = dm_i2c_read(dev, CMD_GET_FEATURES, val, 2);
		if (ret) {
			printf("Error: turris_omnia_mcu CMD_GET_FEATURES failed: %d\n", ret);
			return ret;
		}
		info->features = ((u16)val[1] << 8) | val[0];
	}

	uc_priv->bank_name = "mcu_";

	if ((info->features & FEAT_EXT_CMDS) && (info->features & FEAT_PERIPH_MCU))
		uc_priv->gpio_count = 16 + 32 + 16;
	else if (info->features & FEAT_EXT_CMDS)
		uc_priv->gpio_count = 16 + 32;
	else
		uc_priv->gpio_count = 16;

	return 0;
}

static const struct udevice_id turris_omnia_mcu_ids[] = {
	{ .compatible = "cznic,turris-omnia-mcu" },
	{ }
};

U_BOOT_DRIVER(turris_omnia_mcu) = {
	.name		= "turris-omnia-mcu",
	.id		= UCLASS_GPIO,
	.ops		= &turris_omnia_mcu_ops,
	.probe		= turris_omnia_mcu_probe,
	.plat_auto	= sizeof(struct turris_omnia_mcu_info),
	.of_match	= turris_omnia_mcu_ids,
};
