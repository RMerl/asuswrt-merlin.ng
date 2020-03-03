/*
 * Copyright (c) 2014, NVIDIA CORPORATION. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <linux/clk.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/reset.h>
#include <linux/regulator/consumer.h>
#include <linux/iommu.h>
#include <soc/tegra/fuse.h>
#include <soc/tegra/pmc.h>

#include "nouveau_drm.h"
#include "nouveau_platform.h"

static int nouveau_platform_power_up(struct nouveau_platform_gpu *gpu)
{
	int err;

	err = regulator_enable(gpu->vdd);
	if (err)
		goto err_power;

	err = clk_prepare_enable(gpu->clk);
	if (err)
		goto err_clk;
	err = clk_prepare_enable(gpu->clk_pwr);
	if (err)
		goto err_clk_pwr;
	clk_set_rate(gpu->clk_pwr, 204000000);
	udelay(10);

	reset_control_assert(gpu->rst);
	udelay(10);

	err = tegra_powergate_remove_clamping(TEGRA_POWERGATE_3D);
	if (err)
		goto err_clamp;
	udelay(10);

	reset_control_deassert(gpu->rst);
	udelay(10);

	return 0;

err_clamp:
	clk_disable_unprepare(gpu->clk_pwr);
err_clk_pwr:
	clk_disable_unprepare(gpu->clk);
err_clk:
	regulator_disable(gpu->vdd);
err_power:
	return err;
}

static int nouveau_platform_power_down(struct nouveau_platform_gpu *gpu)
{
	int err;

	reset_control_assert(gpu->rst);
	udelay(10);

	clk_disable_unprepare(gpu->clk_pwr);
	clk_disable_unprepare(gpu->clk);
	udelay(10);

	err = regulator_disable(gpu->vdd);
	if (err)
		return err;

	return 0;
}

static void nouveau_platform_probe_iommu(struct device *dev,
					 struct nouveau_platform_gpu *gpu)
{
	int err;
	unsigned long pgsize_bitmap;

	mutex_init(&gpu->iommu.mutex);

	if (iommu_present(&platform_bus_type)) {
		gpu->iommu.domain = iommu_domain_alloc(&platform_bus_type);
		if (IS_ERR(gpu->iommu.domain))
			goto error;

		/*
		 * A IOMMU is only usable if it supports page sizes smaller
		 * or equal to the system's PAGE_SIZE, with a preference if
		 * both are equal.
		 */
		pgsize_bitmap = gpu->iommu.domain->ops->pgsize_bitmap;
		if (pgsize_bitmap & PAGE_SIZE) {
			gpu->iommu.pgshift = PAGE_SHIFT;
		} else {
			gpu->iommu.pgshift = fls(pgsize_bitmap & ~PAGE_MASK);
			if (gpu->iommu.pgshift == 0) {
				dev_warn(dev, "unsupported IOMMU page size\n");
				goto free_domain;
			}
			gpu->iommu.pgshift -= 1;
		}

		err = iommu_attach_device(gpu->iommu.domain, dev);
		if (err)
			goto free_domain;

		err = nvkm_mm_init(&gpu->iommu._mm, 0,
				   (1ULL << 40) >> gpu->iommu.pgshift, 1);
		if (err)
			goto detach_device;

		gpu->iommu.mm = &gpu->iommu._mm;
	}

	return;

detach_device:
	iommu_detach_device(gpu->iommu.domain, dev);

free_domain:
	iommu_domain_free(gpu->iommu.domain);

error:
	gpu->iommu.domain = NULL;
	gpu->iommu.pgshift = 0;
	dev_err(dev, "cannot initialize IOMMU MM\n");
}

static void nouveau_platform_remove_iommu(struct device *dev,
					  struct nouveau_platform_gpu *gpu)
{
	if (gpu->iommu.domain) {
		nvkm_mm_fini(&gpu->iommu._mm);
		iommu_detach_device(gpu->iommu.domain, dev);
		iommu_domain_free(gpu->iommu.domain);
	}
}

static int nouveau_platform_probe(struct platform_device *pdev)
{
	struct nouveau_platform_gpu *gpu;
	struct nouveau_platform_device *device;
	struct drm_device *drm;
	int err;

	gpu = devm_kzalloc(&pdev->dev, sizeof(*gpu), GFP_KERNEL);
	if (!gpu)
		return -ENOMEM;

	gpu->vdd = devm_regulator_get(&pdev->dev, "vdd");
	if (IS_ERR(gpu->vdd))
		return PTR_ERR(gpu->vdd);

	gpu->rst = devm_reset_control_get(&pdev->dev, "gpu");
	if (IS_ERR(gpu->rst))
		return PTR_ERR(gpu->rst);

	gpu->clk = devm_clk_get(&pdev->dev, "gpu");
	if (IS_ERR(gpu->clk))
		return PTR_ERR(gpu->clk);

	gpu->clk_pwr = devm_clk_get(&pdev->dev, "pwr");
	if (IS_ERR(gpu->clk_pwr))
		return PTR_ERR(gpu->clk_pwr);

	nouveau_platform_probe_iommu(&pdev->dev, gpu);

	err = nouveau_platform_power_up(gpu);
	if (err)
		return err;

	drm = nouveau_platform_device_create(pdev, &device);
	if (IS_ERR(drm)) {
		err = PTR_ERR(drm);
		goto power_down;
	}

	device->gpu = gpu;
	device->gpu_speedo = tegra_sku_info.gpu_speedo_value;

	err = drm_dev_register(drm, 0);
	if (err < 0)
		goto err_unref;

	return 0;

err_unref:
	drm_dev_unref(drm);

power_down:
	nouveau_platform_power_down(gpu);
	nouveau_platform_remove_iommu(&pdev->dev, gpu);

	return err;
}

static int nouveau_platform_remove(struct platform_device *pdev)
{
	struct drm_device *drm_dev = platform_get_drvdata(pdev);
	struct nouveau_drm *drm = nouveau_drm(drm_dev);
	struct nvkm_device *device = nvxx_device(&drm->device);
	struct nouveau_platform_gpu *gpu = nv_device_to_platform(device)->gpu;
	int err;

	nouveau_drm_device_remove(drm_dev);

	err = nouveau_platform_power_down(gpu);

	nouveau_platform_remove_iommu(&pdev->dev, gpu);

	return err;
}

#if IS_ENABLED(CONFIG_OF)
static const struct of_device_id nouveau_platform_match[] = {
	{ .compatible = "nvidia,gk20a" },
	{ }
};

MODULE_DEVICE_TABLE(of, nouveau_platform_match);
#endif

struct platform_driver nouveau_platform_driver = {
	.driver = {
		.name = "nouveau",
		.of_match_table = of_match_ptr(nouveau_platform_match),
	},
	.probe = nouveau_platform_probe,
	.remove = nouveau_platform_remove,
};
