// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <dm.h>
#include <dt-structs.h>
#include <log.h>
#include <malloc.h>
#include <regmap.h>
#include <syscon.h>
#include <asm/arch-rockchip/clock.h>

static const struct udevice_id rk3288_syscon_ids[] = {
	{ .compatible = "rockchip,rk3288-noc", .data = ROCKCHIP_SYSCON_NOC },
	{ .compatible = "rockchip,rk3288-grf", .data = ROCKCHIP_SYSCON_GRF },
	{ .compatible = "rockchip,rk3288-sgrf", .data = ROCKCHIP_SYSCON_SGRF },
	{ .compatible = "rockchip,rk3288-pmu", .data = ROCKCHIP_SYSCON_PMU },
	{ }
};

U_BOOT_DRIVER(syscon_rk3288) = {
	.name = "rk3288_syscon",
	.id = UCLASS_SYSCON,
	.of_match = rk3288_syscon_ids,
};

#if CONFIG_IS_ENABLED(OF_PLATDATA)
#if IS_ENABLED(CONFIG_FDT_64BIT)
struct rockchip_rk3288_noc_plat {
	struct dtd_rockchip_rk3288_noc dtplat;
};

struct rockchip_rk3288_grf_plat {
	struct dtd_rockchip_rk3288_grf dtplat;
};

struct rockchip_rk3288_sgrf_plat {
	struct dtd_rockchip_rk3288_sgrf dtplat;
};

struct rockchip_rk3288_pmu_plat {
	struct dtd_rockchip_rk3288_pmu dtplat;
};

static int rk3288_noc_bind_of_plat(struct udevice *dev)
{
	struct rockchip_rk3288_noc_plat *plat = dev_get_plat(dev);
	struct syscon_uc_info *priv = dev_get_uclass_priv(dev);
	int size = dev->uclass->uc_drv->per_device_auto;

	if (size && !priv) {
		priv = calloc(1, size);
		if (!priv)
			return -ENOMEM;
		dev_set_uclass_priv(dev, priv);
	}

	dev->driver_data = dev->driver->of_match->data;
	debug("syscon: %s %d\n", dev->name, (uint)dev->driver_data);

	return regmap_init_mem_plat(dev, plat->dtplat.reg, sizeof(plat->dtplat.reg[0]),
				    ARRAY_SIZE(plat->dtplat.reg) / 2, &priv->regmap);
}

static int rk3288_grf_bind_of_plat(struct udevice *dev)
{
	struct rockchip_rk3288_grf_plat *plat = dev_get_plat(dev);
	struct syscon_uc_info *priv = dev_get_uclass_priv(dev);
	int size = dev->uclass->uc_drv->per_device_auto;

	if (size && !priv) {
		priv = calloc(1, size);
		if (!priv)
			return -ENOMEM;
		dev_set_uclass_priv(dev, priv);
	}

	dev->driver_data = dev->driver->of_match->data;
	debug("syscon: %s %d\n", dev->name, (uint)dev->driver_data);

	return regmap_init_mem_plat(dev, plat->dtplat.reg, sizeof(plat->dtplat.reg[0]),
				    ARRAY_SIZE(plat->dtplat.reg) / 2, &priv->regmap);
}

static int rk3288_sgrf_bind_of_plat(struct udevice *dev)
{
	struct rockchip_rk3288_sgrf_plat *plat = dev_get_plat(dev);
	struct syscon_uc_info *priv = dev_get_uclass_priv(dev);
	int size = dev->uclass->uc_drv->per_device_auto;

	if (size && !priv) {
		priv = calloc(1, size);
		if (!priv)
			return -ENOMEM;
		dev_set_uclass_priv(dev, priv);
	}

	dev->driver_data = dev->driver->of_match->data;
	debug("syscon: %s %d\n", dev->name, (uint)dev->driver_data);

	return regmap_init_mem_plat(dev, plat->dtplat.reg, sizeof(plat->dtplat.reg[0]),
				    ARRAY_SIZE(plat->dtplat.reg) / 2, &priv->regmap);
}

static int rk3288_pmu_bind_of_plat(struct udevice *dev)
{
	struct rockchip_rk3288_pmu_plat *plat = dev_get_plat(dev);
	struct syscon_uc_info *priv = dev_get_uclass_priv(dev);
	int size = dev->uclass->uc_drv->per_device_auto;

	if (size && !priv) {
		priv = calloc(1, size);
		if (!priv)
			return -ENOMEM;
		dev_set_uclass_priv(dev, priv);
	}

	dev->driver_data = dev->driver->of_match->data;
	debug("syscon: %s %d\n", dev->name, (uint)dev->driver_data);

	return regmap_init_mem_plat(dev, plat->dtplat.reg, sizeof(plat->dtplat.reg[0]),
				    ARRAY_SIZE(plat->dtplat.reg) / 2, &priv->regmap);
}
#else
static int rk3288_syscon_bind_of_plat(struct udevice *dev)
{
	dev->driver_data = dev->driver->of_match->data;
	debug("syscon: %s %d\n", dev->name, (uint)dev->driver_data);

	return 0;
}
#endif

U_BOOT_DRIVER(rockchip_rk3288_noc) = {
	.name = "rockchip_rk3288_noc",
	.id = UCLASS_SYSCON,
	.of_match = rk3288_syscon_ids,
#if IS_ENABLED(CONFIG_FDT_64BIT)
	.bind = rk3288_noc_bind_of_plat,
	.plat_auto = sizeof(struct rockchip_rk3288_noc_plat),
#else
	.bind = rk3288_syscon_bind_of_plat,
#endif
};

U_BOOT_DRIVER(rockchip_rk3288_grf) = {
	.name = "rockchip_rk3288_grf",
	.id = UCLASS_SYSCON,
	.of_match = rk3288_syscon_ids + 1,
#if IS_ENABLED(CONFIG_FDT_64BIT)
	.bind = rk3288_grf_bind_of_plat,
	.plat_auto = sizeof(struct rockchip_rk3288_grf_plat),
#else
	.bind = rk3288_syscon_bind_of_plat,
#endif
};

U_BOOT_DRIVER(rockchip_rk3288_sgrf) = {
	.name = "rockchip_rk3288_sgrf",
	.id = UCLASS_SYSCON,
	.of_match = rk3288_syscon_ids + 2,
#if IS_ENABLED(CONFIG_FDT_64BIT)
	.bind = rk3288_sgrf_bind_of_plat,
	.plat_auto = sizeof(struct rockchip_rk3288_sgrf_plat),
#else
	.bind = rk3288_syscon_bind_of_plat,
#endif
};

U_BOOT_DRIVER(rockchip_rk3288_pmu) = {
	.name = "rockchip_rk3288_pmu",
	.id = UCLASS_SYSCON,
	.of_match = rk3288_syscon_ids + 3,
#if IS_ENABLED(CONFIG_FDT_64BIT)
	.bind = rk3288_pmu_bind_of_plat,
	.plat_auto = sizeof(struct rockchip_rk3288_pmu_plat),
#else
	.bind = rk3288_syscon_bind_of_plat,
#endif
};
#endif
