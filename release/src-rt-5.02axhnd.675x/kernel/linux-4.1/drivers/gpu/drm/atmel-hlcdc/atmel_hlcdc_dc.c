/*
 * Copyright (C) 2014 Traphandler
 * Copyright (C) 2014 Free Electrons
 * Copyright (C) 2014 Atmel
 *
 * Author: Jean-Jacques Hiblot <jjhiblot@traphandler.com>
 * Author: Boris BREZILLON <boris.brezillon@free-electrons.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <linux/clk.h>
#include <linux/irq.h>
#include <linux/irqchip.h>
#include <linux/module.h>
#include <linux/pm_runtime.h>

#include "atmel_hlcdc_dc.h"

#define ATMEL_HLCDC_LAYER_IRQS_OFFSET		8

static const struct atmel_hlcdc_layer_desc atmel_hlcdc_sama5d3_layers[] = {
	{
		.name = "base",
		.formats = &atmel_hlcdc_plane_rgb_formats,
		.regs_offset = 0x40,
		.id = 0,
		.type = ATMEL_HLCDC_BASE_LAYER,
		.nconfigs = 7,
		.layout = {
			.xstride = { 2 },
			.default_color = 3,
			.general_config = 4,
			.disc_pos = 5,
			.disc_size = 6,
		},
	},
	{
		.name = "overlay1",
		.formats = &atmel_hlcdc_plane_rgb_formats,
		.regs_offset = 0x140,
		.id = 1,
		.type = ATMEL_HLCDC_OVERLAY_LAYER,
		.nconfigs = 10,
		.layout = {
			.pos = 2,
			.size = 3,
			.xstride = { 4 },
			.pstride = { 5 },
			.default_color = 6,
			.chroma_key = 7,
			.chroma_key_mask = 8,
			.general_config = 9,
		},
	},
	{
		.name = "overlay2",
		.formats = &atmel_hlcdc_plane_rgb_formats,
		.regs_offset = 0x240,
		.id = 2,
		.type = ATMEL_HLCDC_OVERLAY_LAYER,
		.nconfigs = 10,
		.layout = {
			.pos = 2,
			.size = 3,
			.xstride = { 4 },
			.pstride = { 5 },
			.default_color = 6,
			.chroma_key = 7,
			.chroma_key_mask = 8,
			.general_config = 9,
		},
	},
	{
		.name = "high-end-overlay",
		.formats = &atmel_hlcdc_plane_rgb_and_yuv_formats,
		.regs_offset = 0x340,
		.id = 3,
		.type = ATMEL_HLCDC_OVERLAY_LAYER,
		.nconfigs = 42,
		.layout = {
			.pos = 2,
			.size = 3,
			.memsize = 4,
			.xstride = { 5, 7 },
			.pstride = { 6, 8 },
			.default_color = 9,
			.chroma_key = 10,
			.chroma_key_mask = 11,
			.general_config = 12,
			.csc = 14,
		},
	},
	{
		.name = "cursor",
		.formats = &atmel_hlcdc_plane_rgb_formats,
		.regs_offset = 0x440,
		.id = 4,
		.type = ATMEL_HLCDC_CURSOR_LAYER,
		.nconfigs = 10,
		.max_width = 128,
		.max_height = 128,
		.layout = {
			.pos = 2,
			.size = 3,
			.xstride = { 4 },
			.pstride = { 5 },
			.default_color = 6,
			.chroma_key = 7,
			.chroma_key_mask = 8,
			.general_config = 9,
		},
	},
};

static const struct atmel_hlcdc_dc_desc atmel_hlcdc_dc_sama5d3 = {
	.min_width = 0,
	.min_height = 0,
	.max_width = 2048,
	.max_height = 2048,
	.nlayers = ARRAY_SIZE(atmel_hlcdc_sama5d3_layers),
	.layers = atmel_hlcdc_sama5d3_layers,
};

static const struct of_device_id atmel_hlcdc_of_match[] = {
	{
		.compatible = "atmel,sama5d3-hlcdc",
		.data = &atmel_hlcdc_dc_sama5d3,
	},
	{ /* sentinel */ },
};

int atmel_hlcdc_dc_mode_valid(struct atmel_hlcdc_dc *dc,
			      struct drm_display_mode *mode)
{
	int vfront_porch = mode->vsync_start - mode->vdisplay;
	int vback_porch = mode->vtotal - mode->vsync_end;
	int vsync_len = mode->vsync_end - mode->vsync_start;
	int hfront_porch = mode->hsync_start - mode->hdisplay;
	int hback_porch = mode->htotal - mode->hsync_end;
	int hsync_len = mode->hsync_end - mode->hsync_start;

	if (hsync_len > 0x40 || hsync_len < 1)
		return MODE_HSYNC;

	if (vsync_len > 0x40 || vsync_len < 1)
		return MODE_VSYNC;

	if (hfront_porch > 0x200 || hfront_porch < 1 ||
	    hback_porch > 0x200 || hback_porch < 1 ||
	    mode->hdisplay < 1)
		return MODE_H_ILLEGAL;

	if (vfront_porch > 0x40 || vfront_porch < 1 ||
	    vback_porch > 0x40 || vback_porch < 0 ||
	    mode->vdisplay < 1)
		return MODE_V_ILLEGAL;

	return MODE_OK;
}

static irqreturn_t atmel_hlcdc_dc_irq_handler(int irq, void *data)
{
	struct drm_device *dev = data;
	struct atmel_hlcdc_dc *dc = dev->dev_private;
	unsigned long status;
	unsigned int imr, isr;
	int i;

	regmap_read(dc->hlcdc->regmap, ATMEL_HLCDC_IMR, &imr);
	regmap_read(dc->hlcdc->regmap, ATMEL_HLCDC_ISR, &isr);
	status = imr & isr;
	if (!status)
		return IRQ_NONE;

	if (status & ATMEL_HLCDC_SOF)
		atmel_hlcdc_crtc_irq(dc->crtc);

	for (i = 0; i < ATMEL_HLCDC_MAX_LAYERS; i++) {
		struct atmel_hlcdc_layer *layer = dc->layers[i];

		if (!(ATMEL_HLCDC_LAYER_STATUS(i) & status) || !layer)
			continue;

		atmel_hlcdc_layer_irq(layer);
	}

	return IRQ_HANDLED;
}

static struct drm_framebuffer *atmel_hlcdc_fb_create(struct drm_device *dev,
		struct drm_file *file_priv, struct drm_mode_fb_cmd2 *mode_cmd)
{
	return drm_fb_cma_create(dev, file_priv, mode_cmd);
}

static void atmel_hlcdc_fb_output_poll_changed(struct drm_device *dev)
{
	struct atmel_hlcdc_dc *dc = dev->dev_private;

	if (dc->fbdev) {
		drm_fbdev_cma_hotplug_event(dc->fbdev);
	} else {
		dc->fbdev = drm_fbdev_cma_init(dev, 24,
				dev->mode_config.num_crtc,
				dev->mode_config.num_connector);
		if (IS_ERR(dc->fbdev))
			dc->fbdev = NULL;
	}
}

static const struct drm_mode_config_funcs mode_config_funcs = {
	.fb_create = atmel_hlcdc_fb_create,
	.output_poll_changed = atmel_hlcdc_fb_output_poll_changed,
	.atomic_check = drm_atomic_helper_check,
	.atomic_commit = drm_atomic_helper_commit,
};

static int atmel_hlcdc_dc_modeset_init(struct drm_device *dev)
{
	struct atmel_hlcdc_dc *dc = dev->dev_private;
	struct atmel_hlcdc_planes *planes;
	int ret;
	int i;

	drm_mode_config_init(dev);

	ret = atmel_hlcdc_create_outputs(dev);
	if (ret) {
		dev_err(dev->dev, "failed to create panel: %d\n", ret);
		return ret;
	}

	planes = atmel_hlcdc_create_planes(dev);
	if (IS_ERR(planes)) {
		dev_err(dev->dev, "failed to create planes\n");
		return PTR_ERR(planes);
	}

	dc->planes = planes;

	dc->layers[planes->primary->layer.desc->id] =
						&planes->primary->layer;

	if (planes->cursor)
		dc->layers[planes->cursor->layer.desc->id] =
							&planes->cursor->layer;

	for (i = 0; i < planes->noverlays; i++)
		dc->layers[planes->overlays[i]->layer.desc->id] =
						&planes->overlays[i]->layer;

	ret = atmel_hlcdc_crtc_create(dev);
	if (ret) {
		dev_err(dev->dev, "failed to create crtc\n");
		return ret;
	}

	dev->mode_config.min_width = dc->desc->min_width;
	dev->mode_config.min_height = dc->desc->min_height;
	dev->mode_config.max_width = dc->desc->max_width;
	dev->mode_config.max_height = dc->desc->max_height;
	dev->mode_config.funcs = &mode_config_funcs;

	return 0;
}

static int atmel_hlcdc_dc_load(struct drm_device *dev)
{
	struct platform_device *pdev = to_platform_device(dev->dev);
	const struct of_device_id *match;
	struct atmel_hlcdc_dc *dc;
	int ret;

	match = of_match_node(atmel_hlcdc_of_match, dev->dev->parent->of_node);
	if (!match) {
		dev_err(&pdev->dev, "invalid compatible string\n");
		return -ENODEV;
	}

	if (!match->data) {
		dev_err(&pdev->dev, "invalid hlcdc description\n");
		return -EINVAL;
	}

	dc = devm_kzalloc(dev->dev, sizeof(*dc), GFP_KERNEL);
	if (!dc)
		return -ENOMEM;

	dc->wq = alloc_ordered_workqueue("atmel-hlcdc-dc", 0);
	if (!dc->wq)
		return -ENOMEM;

	dc->desc = match->data;
	dc->hlcdc = dev_get_drvdata(dev->dev->parent);
	dev->dev_private = dc;

	ret = clk_prepare_enable(dc->hlcdc->periph_clk);
	if (ret) {
		dev_err(dev->dev, "failed to enable periph_clk\n");
		goto err_destroy_wq;
	}

	pm_runtime_enable(dev->dev);

	ret = atmel_hlcdc_dc_modeset_init(dev);
	if (ret < 0) {
		dev_err(dev->dev, "failed to initialize mode setting\n");
		goto err_periph_clk_disable;
	}

	drm_mode_config_reset(dev);

	ret = drm_vblank_init(dev, 1);
	if (ret < 0) {
		dev_err(dev->dev, "failed to initialize vblank\n");
		goto err_periph_clk_disable;
	}

	pm_runtime_get_sync(dev->dev);
	ret = drm_irq_install(dev, dc->hlcdc->irq);
	pm_runtime_put_sync(dev->dev);
	if (ret < 0) {
		dev_err(dev->dev, "failed to install IRQ handler\n");
		goto err_periph_clk_disable;
	}

	platform_set_drvdata(pdev, dev);

	drm_kms_helper_poll_init(dev);

	/* force connectors detection */
	drm_helper_hpd_irq_event(dev);

	return 0;

err_periph_clk_disable:
	pm_runtime_disable(dev->dev);
	clk_disable_unprepare(dc->hlcdc->periph_clk);

err_destroy_wq:
	destroy_workqueue(dc->wq);

	return ret;
}

static void atmel_hlcdc_dc_unload(struct drm_device *dev)
{
	struct atmel_hlcdc_dc *dc = dev->dev_private;

	if (dc->fbdev)
		drm_fbdev_cma_fini(dc->fbdev);
	flush_workqueue(dc->wq);
	drm_kms_helper_poll_fini(dev);
	drm_mode_config_cleanup(dev);
	drm_vblank_cleanup(dev);

	pm_runtime_get_sync(dev->dev);
	drm_irq_uninstall(dev);
	pm_runtime_put_sync(dev->dev);

	dev->dev_private = NULL;

	pm_runtime_disable(dev->dev);
	clk_disable_unprepare(dc->hlcdc->periph_clk);
	destroy_workqueue(dc->wq);
}

static int atmel_hlcdc_dc_connector_plug_all(struct drm_device *dev)
{
	struct drm_connector *connector, *failed;
	int ret;

	mutex_lock(&dev->mode_config.mutex);
	list_for_each_entry(connector, &dev->mode_config.connector_list, head) {
		ret = drm_connector_register(connector);
		if (ret) {
			failed = connector;
			goto err;
		}
	}
	mutex_unlock(&dev->mode_config.mutex);
	return 0;

err:
	list_for_each_entry(connector, &dev->mode_config.connector_list, head) {
		if (failed == connector)
			break;

		drm_connector_unregister(connector);
	}
	mutex_unlock(&dev->mode_config.mutex);

	return ret;
}

static void atmel_hlcdc_dc_connector_unplug_all(struct drm_device *dev)
{
	mutex_lock(&dev->mode_config.mutex);
	drm_connector_unplug_all(dev);
	mutex_unlock(&dev->mode_config.mutex);
}

static void atmel_hlcdc_dc_preclose(struct drm_device *dev,
				    struct drm_file *file)
{
	struct drm_crtc *crtc;

	list_for_each_entry(crtc, &dev->mode_config.crtc_list, head)
		atmel_hlcdc_crtc_cancel_page_flip(crtc, file);
}

static void atmel_hlcdc_dc_lastclose(struct drm_device *dev)
{
	struct atmel_hlcdc_dc *dc = dev->dev_private;

	drm_fbdev_cma_restore_mode(dc->fbdev);
}

static int atmel_hlcdc_dc_irq_postinstall(struct drm_device *dev)
{
	struct atmel_hlcdc_dc *dc = dev->dev_private;
	unsigned int cfg = 0;
	int i;

	/* Enable interrupts on activated layers */
	for (i = 0; i < ATMEL_HLCDC_MAX_LAYERS; i++) {
		if (dc->layers[i])
			cfg |= ATMEL_HLCDC_LAYER_STATUS(i);
	}

	regmap_write(dc->hlcdc->regmap, ATMEL_HLCDC_IER, cfg);

	return 0;
}

static void atmel_hlcdc_dc_irq_uninstall(struct drm_device *dev)
{
	struct atmel_hlcdc_dc *dc = dev->dev_private;
	unsigned int isr;

	regmap_write(dc->hlcdc->regmap, ATMEL_HLCDC_IDR, 0xffffffff);
	regmap_read(dc->hlcdc->regmap, ATMEL_HLCDC_ISR, &isr);
}

static int atmel_hlcdc_dc_enable_vblank(struct drm_device *dev, int crtc)
{
	struct atmel_hlcdc_dc *dc = dev->dev_private;

	/* Enable SOF (Start Of Frame) interrupt for vblank counting */
	regmap_write(dc->hlcdc->regmap, ATMEL_HLCDC_IER, ATMEL_HLCDC_SOF);

	return 0;
}

static void atmel_hlcdc_dc_disable_vblank(struct drm_device *dev, int crtc)
{
	struct atmel_hlcdc_dc *dc = dev->dev_private;

	regmap_write(dc->hlcdc->regmap, ATMEL_HLCDC_IDR, ATMEL_HLCDC_SOF);
}

static const struct file_operations fops = {
	.owner              = THIS_MODULE,
	.open               = drm_open,
	.release            = drm_release,
	.unlocked_ioctl     = drm_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl       = drm_compat_ioctl,
#endif
	.poll               = drm_poll,
	.read               = drm_read,
	.llseek             = no_llseek,
	.mmap               = drm_gem_cma_mmap,
};

static struct drm_driver atmel_hlcdc_dc_driver = {
	.driver_features = DRIVER_HAVE_IRQ | DRIVER_GEM | DRIVER_MODESET,
	.preclose = atmel_hlcdc_dc_preclose,
	.lastclose = atmel_hlcdc_dc_lastclose,
	.irq_handler = atmel_hlcdc_dc_irq_handler,
	.irq_preinstall = atmel_hlcdc_dc_irq_uninstall,
	.irq_postinstall = atmel_hlcdc_dc_irq_postinstall,
	.irq_uninstall = atmel_hlcdc_dc_irq_uninstall,
	.get_vblank_counter = drm_vblank_count,
	.enable_vblank = atmel_hlcdc_dc_enable_vblank,
	.disable_vblank = atmel_hlcdc_dc_disable_vblank,
	.gem_free_object = drm_gem_cma_free_object,
	.gem_vm_ops = &drm_gem_cma_vm_ops,
	.dumb_create = drm_gem_cma_dumb_create,
	.dumb_map_offset = drm_gem_cma_dumb_map_offset,
	.dumb_destroy = drm_gem_dumb_destroy,
	.fops = &fops,
	.name = "atmel-hlcdc",
	.desc = "Atmel HLCD Controller DRM",
	.date = "20141504",
	.major = 1,
	.minor = 0,
};

static int atmel_hlcdc_dc_drm_probe(struct platform_device *pdev)
{
	struct drm_device *ddev;
	int ret;

	ddev = drm_dev_alloc(&atmel_hlcdc_dc_driver, &pdev->dev);
	if (!ddev)
		return -ENOMEM;

	ret = drm_dev_set_unique(ddev, dev_name(ddev->dev));
	if (ret)
		goto err_unref;

	ret = atmel_hlcdc_dc_load(ddev);
	if (ret)
		goto err_unref;

	ret = drm_dev_register(ddev, 0);
	if (ret)
		goto err_unload;

	ret = atmel_hlcdc_dc_connector_plug_all(ddev);
	if (ret)
		goto err_unregister;

	return 0;

err_unregister:
	drm_dev_unregister(ddev);

err_unload:
	atmel_hlcdc_dc_unload(ddev);

err_unref:
	drm_dev_unref(ddev);

	return ret;
}

static int atmel_hlcdc_dc_drm_remove(struct platform_device *pdev)
{
	struct drm_device *ddev = platform_get_drvdata(pdev);

	atmel_hlcdc_dc_connector_unplug_all(ddev);
	drm_dev_unregister(ddev);
	atmel_hlcdc_dc_unload(ddev);
	drm_dev_unref(ddev);

	return 0;
}

#ifdef CONFIG_PM_SLEEP
static int atmel_hlcdc_dc_drm_suspend(struct device *dev)
{
	struct drm_device *drm_dev = dev_get_drvdata(dev);
	struct drm_crtc *crtc;

	if (pm_runtime_suspended(dev))
		return 0;

	drm_modeset_lock_all(drm_dev);
	list_for_each_entry(crtc, &drm_dev->mode_config.crtc_list, head)
		atmel_hlcdc_crtc_suspend(crtc);
	drm_modeset_unlock_all(drm_dev);
	return 0;
}

static int atmel_hlcdc_dc_drm_resume(struct device *dev)
{
	struct drm_device *drm_dev = dev_get_drvdata(dev);
	struct drm_crtc *crtc;

	if (pm_runtime_suspended(dev))
		return 0;

	drm_modeset_lock_all(drm_dev);
	list_for_each_entry(crtc, &drm_dev->mode_config.crtc_list, head)
		atmel_hlcdc_crtc_resume(crtc);
	drm_modeset_unlock_all(drm_dev);
	return 0;
}
#endif

static SIMPLE_DEV_PM_OPS(atmel_hlcdc_dc_drm_pm_ops,
		atmel_hlcdc_dc_drm_suspend, atmel_hlcdc_dc_drm_resume);

static const struct of_device_id atmel_hlcdc_dc_of_match[] = {
	{ .compatible = "atmel,hlcdc-display-controller" },
	{ },
};

static struct platform_driver atmel_hlcdc_dc_platform_driver = {
	.probe	= atmel_hlcdc_dc_drm_probe,
	.remove	= atmel_hlcdc_dc_drm_remove,
	.driver	= {
		.name	= "atmel-hlcdc-display-controller",
		.pm	= &atmel_hlcdc_dc_drm_pm_ops,
		.of_match_table = atmel_hlcdc_dc_of_match,
	},
};
module_platform_driver(atmel_hlcdc_dc_platform_driver);

MODULE_AUTHOR("Jean-Jacques Hiblot <jjhiblot@traphandler.com>");
MODULE_AUTHOR("Boris Brezillon <boris.brezillon@free-electrons.com>");
MODULE_DESCRIPTION("Atmel HLCDC Display Controller DRM Driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:atmel-hlcdc-dc");
