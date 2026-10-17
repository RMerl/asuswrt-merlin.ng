// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2019 Rockchip Electronics Co., Ltd
 */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <dm/pinctrl.h>
#include <regmap.h>
#include <syscon.h>
#include <linux/bitops.h>

#include "pinctrl-rockchip.h"

static struct rockchip_mux_route_data px30_mux_route_data[] = {
	{
		/* cif-d2m0 */
		.bank_num = 2,
		.pin = 0,
		.func = 1,
		.route_offset = 0x184,
		.route_val = BIT(16 + 7),
	}, {
		/* cif-d2m1 */
		.bank_num = 3,
		.pin = 3,
		.func = 3,
		.route_offset = 0x184,
		.route_val = BIT(16 + 7) | BIT(7),
	}, {
		/* pdm-m0 */
		.bank_num = 3,
		.pin = 22,
		.func = 2,
		.route_offset = 0x184,
		.route_val = BIT(16 + 8),
	}, {
		/* pdm-m1 */
		.bank_num = 2,
		.pin = 22,
		.func = 1,
		.route_offset = 0x184,
		.route_val = BIT(16 + 8) | BIT(8),
	}, {
		/* uart2-rxm0 */
		.bank_num = 1,
		.pin = 27,
		.func = 2,
		.route_offset = 0x184,
		.route_val = BIT(16 + 10),
	}, {
		/* uart2-rxm1 */
		.bank_num = 2,
		.pin = 14,
		.func = 2,
		.route_offset = 0x184,
		.route_val = BIT(16 + 10) | BIT(10),
	}, {
		/* uart3-rxm0 */
		.bank_num = 0,
		.pin = 17,
		.func = 2,
		.route_offset = 0x184,
		.route_val = BIT(16 + 9),
	}, {
		/* uart3-rxm1 */
		.bank_num = 1,
		.pin = 15,
		.func = 2,
		.route_offset = 0x184,
		.route_val = BIT(16 + 9) | BIT(9),
	},
};

static int px30_set_mux(struct rockchip_pin_bank *bank, int pin, int mux)
{
	struct rockchip_pinctrl_priv *priv = bank->priv;
	int iomux_num = (pin / 8);
	struct regmap *regmap;
	int reg, ret, mask, mux_type;
	u8 bit;
	u32 data;

	regmap = (bank->iomux[iomux_num].type & IOMUX_SOURCE_PMU)
				? priv->regmap_pmu : priv->regmap_base;

	/* get basic quadrupel of mux registers and the correct reg inside */
	mux_type = bank->iomux[iomux_num].type;
	reg = bank->iomux[iomux_num].offset;
	reg += rockchip_get_mux_data(mux_type, pin, &bit, &mask);

	data = (mask << (bit + 16));
	data |= (mux & mask) << bit;
	ret = regmap_write(regmap, reg, data);

	return ret;
}

#define PX30_PULL_PMU_OFFSET		0x10
#define PX30_PULL_GRF_OFFSET		0x60
#define PX30_PULL_BITS_PER_PIN		2
#define PX30_PULL_PINS_PER_REG		8
#define PX30_PULL_BANK_STRIDE		16

static void px30_calc_pull_reg_and_bit(struct rockchip_pin_bank *bank,
				       int pin_num, struct regmap **regmap,
				       int *reg, u8 *bit)
{
	struct rockchip_pinctrl_priv *priv = bank->priv;

	/* The first 32 pins of the first bank are located in PMU */
	if (bank->bank_num == 0) {
		*regmap = priv->regmap_pmu;
		*reg = PX30_PULL_PMU_OFFSET;
	} else {
		*regmap = priv->regmap_base;
		*reg = PX30_PULL_GRF_OFFSET;

		/* correct the offset, as we're starting with the 2nd bank */
		*reg -= 0x10;
		*reg += bank->bank_num * PX30_PULL_BANK_STRIDE;
	}

	*reg += ((pin_num / PX30_PULL_PINS_PER_REG) * 4);
	*bit = (pin_num % PX30_PULL_PINS_PER_REG);
	*bit *= PX30_PULL_BITS_PER_PIN;
}

static int px30_set_pull(struct rockchip_pin_bank *bank,
			 int pin_num, int pull)
{
	struct regmap *regmap;
	int reg, ret;
	u8 bit, type;
	u32 data;

	if (pull == PIN_CONFIG_BIAS_PULL_PIN_DEFAULT)
		return -ENOTSUPP;

	px30_calc_pull_reg_and_bit(bank, pin_num, &regmap, &reg, &bit);
	type = bank->pull_type[pin_num / 8];
	ret = rockchip_translate_pull_value(type, pull);
	if (ret < 0) {
		debug("unsupported pull setting %d\n", pull);
		return ret;
	}

	/* enable the write to the equivalent lower bits */
	data = ((1 << ROCKCHIP_PULL_BITS_PER_PIN) - 1) << (bit + 16);
	data |= (ret << bit);
	ret = regmap_write(regmap, reg, data);

	return ret;
}

#define PX30_DRV_PMU_OFFSET		0x20
#define PX30_DRV_GRF_OFFSET		0xf0
#define PX30_DRV_BITS_PER_PIN		2
#define PX30_DRV_PINS_PER_REG		8
#define PX30_DRV_BANK_STRIDE		16

static void px30_calc_drv_reg_and_bit(struct rockchip_pin_bank *bank,
				      int pin_num, struct regmap **regmap,
				      int *reg, u8 *bit)
{
	struct rockchip_pinctrl_priv *priv = bank->priv;

	/* The first 32 pins of the first bank are located in PMU */
	if (bank->bank_num == 0) {
		*regmap = priv->regmap_pmu;
		*reg = PX30_DRV_PMU_OFFSET;
	} else {
		*regmap = priv->regmap_base;
		*reg = PX30_DRV_GRF_OFFSET;

		/* correct the offset, as we're starting with the 2nd bank */
		*reg -= 0x10;
		*reg += bank->bank_num * PX30_DRV_BANK_STRIDE;
	}

	*reg += ((pin_num / PX30_DRV_PINS_PER_REG) * 4);
	*bit = (pin_num % PX30_DRV_PINS_PER_REG);
	*bit *= PX30_DRV_BITS_PER_PIN;
}

static int px30_set_drive(struct rockchip_pin_bank *bank,
			  int pin_num, int strength)
{
	struct regmap *regmap;
	int reg, ret;
	u32 data, rmask_bits, temp;
	u8 bit;
	int drv_type = bank->drv[pin_num / 8].drv_type;

	px30_calc_drv_reg_and_bit(bank, pin_num, &regmap, &reg, &bit);
	ret = rockchip_translate_drive_value(drv_type, strength);
	if (ret < 0) {
		debug("unsupported driver strength %d\n", strength);
		return ret;
	}

	switch (drv_type) {
	case DRV_TYPE_IO_1V8_3V0_AUTO:
	case DRV_TYPE_IO_3V3_ONLY:
		rmask_bits = ROCKCHIP_DRV_3BITS_PER_PIN;
		switch (bit) {
		case 0 ... 12:
			/* regular case, nothing to do */
			break;
		case 15:
			/*
			 * drive-strength offset is special, as it is spread
			 * over 2 registers, the bit data[15] contains bit 0
			 * of the value while temp[1:0] contains bits 2 and 1
			 */
			data = (ret & 0x1) << 15;
			temp = (ret >> 0x1) & 0x3;

			data |= BIT(31);
			ret = regmap_write(regmap, reg, data);
			if (ret)
				return ret;

			temp |= (0x3 << 16);
			reg += 0x4;
			ret = regmap_write(regmap, reg, temp);

			return ret;
		case 18 ... 21:
			/* setting fully enclosed in the second register */
			reg += 4;
			bit -= 16;
			break;
		default:
			debug("unsupported bit: %d for pinctrl drive type: %d\n",
			      bit, drv_type);
			return -EINVAL;
		}
		break;
	case DRV_TYPE_IO_DEFAULT:
	case DRV_TYPE_IO_1V8_OR_3V0:
	case DRV_TYPE_IO_1V8_ONLY:
		rmask_bits = ROCKCHIP_DRV_BITS_PER_PIN;
		break;
	default:
		debug("unsupported pinctrl drive type: %d\n",
		      drv_type);
		return -EINVAL;
	}

	/* enable the write to the equivalent lower bits */
	data = ((1 << rmask_bits) - 1) << (bit + 16);
	data |= (ret << bit);
	ret = regmap_write(regmap, reg, data);

	return ret;
}

#define PX30_SCHMITT_PMU_OFFSET			0x38
#define PX30_SCHMITT_GRF_OFFSET			0xc0
#define PX30_SCHMITT_PINS_PER_PMU_REG		16
#define PX30_SCHMITT_BANK_STRIDE		16
#define PX30_SCHMITT_PINS_PER_GRF_REG		8

static int px30_calc_schmitt_reg_and_bit(struct rockchip_pin_bank *bank,
					 int pin_num,
					 struct regmap **regmap,
					 int *reg, u8 *bit)
{
	struct rockchip_pinctrl_priv *priv = bank->priv;
	int pins_per_reg;

	if (bank->bank_num == 0) {
		*regmap = priv->regmap_pmu;
		*reg = PX30_SCHMITT_PMU_OFFSET;
		pins_per_reg = PX30_SCHMITT_PINS_PER_PMU_REG;
	} else {
		*regmap = priv->regmap_base;
		*reg = PX30_SCHMITT_GRF_OFFSET;
		pins_per_reg = PX30_SCHMITT_PINS_PER_GRF_REG;
		*reg += (bank->bank_num - 1) * PX30_SCHMITT_BANK_STRIDE;
	}
	*reg += ((pin_num / pins_per_reg) * 4);
	*bit = pin_num % pins_per_reg;

	return 0;
}

static int px30_set_schmitt(struct rockchip_pin_bank *bank,
			    int pin_num, int enable)
{
	struct regmap *regmap;
	int reg;
	u8 bit;
	u32 data;

	px30_calc_schmitt_reg_and_bit(bank, pin_num, &regmap, &reg, &bit);
	/* enable the write to the equivalent lower bits */
	data = BIT(bit + 16) | (enable << bit);

	return regmap_write(regmap, reg, data);
}

static struct rockchip_pin_bank px30_pin_banks[] = {
	PIN_BANK_IOMUX_FLAGS(0, 32, "gpio0", IOMUX_SOURCE_PMU,
					     IOMUX_SOURCE_PMU,
					     IOMUX_SOURCE_PMU,
					     IOMUX_SOURCE_PMU
			    ),
	PIN_BANK_IOMUX_FLAGS(1, 32, "gpio1", IOMUX_WIDTH_4BIT,
					     IOMUX_WIDTH_4BIT,
					     IOMUX_WIDTH_4BIT,
					     IOMUX_WIDTH_4BIT
			    ),
	PIN_BANK_IOMUX_FLAGS(2, 32, "gpio2", IOMUX_WIDTH_4BIT,
					     IOMUX_WIDTH_4BIT,
					     IOMUX_WIDTH_4BIT,
					     IOMUX_WIDTH_4BIT
			    ),
	PIN_BANK_IOMUX_FLAGS(3, 32, "gpio3", IOMUX_WIDTH_4BIT,
					     IOMUX_WIDTH_4BIT,
					     IOMUX_WIDTH_4BIT,
					     IOMUX_WIDTH_4BIT
			    ),
};

static struct rockchip_pin_ctrl px30_pin_ctrl = {
	.pin_banks		= px30_pin_banks,
	.nr_banks		= ARRAY_SIZE(px30_pin_banks),
	.grf_mux_offset		= 0x0,
	.pmu_mux_offset		= 0x0,
	.grf_drv_offset		= 0xf0,
	.pmu_drv_offset		= 0x20,
	.iomux_routes		= px30_mux_route_data,
	.niomux_routes		= ARRAY_SIZE(px30_mux_route_data),
	.set_mux		= px30_set_mux,
	.set_pull		= px30_set_pull,
	.set_drive		= px30_set_drive,
	.set_schmitt		= px30_set_schmitt,
};

static const struct udevice_id px30_pinctrl_ids[] = {
	{
		.compatible = "rockchip,px30-pinctrl",
		.data = (ulong)&px30_pin_ctrl
	},
	{ }
};

U_BOOT_DRIVER(pinctrl_px30) = {
	.name		= "rockchip_px30_pinctrl",
	.id		= UCLASS_PINCTRL,
	.of_match	= px30_pinctrl_ids,
	.priv_auto	= sizeof(struct rockchip_pinctrl_priv),
	.ops		= &rockchip_pinctrl_ops,
#if CONFIG_IS_ENABLED(OF_REAL)
	.bind		= dm_scan_fdt_dev,
#endif
	.probe		= rockchip_pinctrl_probe,
};
