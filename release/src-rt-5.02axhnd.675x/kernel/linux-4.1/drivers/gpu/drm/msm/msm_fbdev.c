/*
 * Copyright (C) 2013 Red Hat
 * Author: Rob Clark <robdclark@gmail.com>
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

#include "msm_drv.h"

#include "drm_crtc.h"
#include "drm_fb_helper.h"
#include "msm_gem.h"

extern int msm_gem_mmap_obj(struct drm_gem_object *obj,
					struct vm_area_struct *vma);
static int msm_fbdev_mmap(struct fb_info *info, struct vm_area_struct *vma);

/*
 * fbdev funcs, to implement legacy fbdev interface on top of drm driver
 */

#define to_msm_fbdev(x) container_of(x, struct msm_fbdev, base)

struct msm_fbdev {
	struct drm_fb_helper base;
	struct drm_framebuffer *fb;
	struct drm_gem_object *bo;
};

static struct fb_ops msm_fb_ops = {
	.owner = THIS_MODULE,

	/* Note: to properly handle manual update displays, we wrap the
	 * basic fbdev ops which write to the framebuffer
	 */
	.fb_read = fb_sys_read,
	.fb_write = fb_sys_write,
	.fb_fillrect = sys_fillrect,
	.fb_copyarea = sys_copyarea,
	.fb_imageblit = sys_imageblit,
	.fb_mmap = msm_fbdev_mmap,

	.fb_check_var = drm_fb_helper_check_var,
	.fb_set_par = drm_fb_helper_set_par,
	.fb_pan_display = drm_fb_helper_pan_display,
	.fb_blank = drm_fb_helper_blank,
	.fb_setcmap = drm_fb_helper_setcmap,
};

static int msm_fbdev_mmap(struct fb_info *info, struct vm_area_struct *vma)
{
	struct drm_fb_helper *helper = (struct drm_fb_helper *)info->par;
	struct msm_fbdev *fbdev = to_msm_fbdev(helper);
	struct drm_gem_object *drm_obj = fbdev->bo;
	struct drm_device *dev = helper->dev;
	int ret = 0;

	if (drm_device_is_unplugged(dev))
		return -ENODEV;

	mutex_lock(&dev->struct_mutex);

	ret = drm_gem_mmap_obj(drm_obj, drm_obj->size, vma);

	mutex_unlock(&dev->struct_mutex);

	if (ret) {
		pr_err("%s:drm_gem_mmap_obj fail\n", __func__);
		return ret;
	}

	return msm_gem_mmap_obj(drm_obj, vma);
}

static int msm_fbdev_create(struct drm_fb_helper *helper,
		struct drm_fb_helper_surface_size *sizes)
{
	struct msm_fbdev *fbdev = to_msm_fbdev(helper);
	struct drm_device *dev = helper->dev;
	struct drm_framebuffer *fb = NULL;
	struct fb_info *fbi = NULL;
	struct drm_mode_fb_cmd2 mode_cmd = {0};
	uint32_t paddr;
	int ret, size;

	DBG("create fbdev: %dx%d@%d (%dx%d)", sizes->surface_width,
			sizes->surface_height, sizes->surface_bpp,
			sizes->fb_width, sizes->fb_height);

	mode_cmd.pixel_format = drm_mode_legacy_fb_format(sizes->surface_bpp,
			sizes->surface_depth);

	mode_cmd.width = sizes->surface_width;
	mode_cmd.height = sizes->surface_height;

	mode_cmd.pitches[0] = align_pitch(
			mode_cmd.width, sizes->surface_bpp);

	/* allocate backing bo */
	size = mode_cmd.pitches[0] * mode_cmd.height;
	DBG("allocating %d bytes for fb %d", size, dev->primary->index);
	mutex_lock(&dev->struct_mutex);
	fbdev->bo = msm_gem_new(dev, size, MSM_BO_SCANOUT |
			MSM_BO_WC | MSM_BO_STOLEN);
	mutex_unlock(&dev->struct_mutex);
	if (IS_ERR(fbdev->bo)) {
		ret = PTR_ERR(fbdev->bo);
		fbdev->bo = NULL;
		dev_err(dev->dev, "failed to allocate buffer object: %d\n", ret);
		goto fail;
	}

	fb = msm_framebuffer_init(dev, &mode_cmd, &fbdev->bo);
	if (IS_ERR(fb)) {
		dev_err(dev->dev, "failed to allocate fb\n");
		/* note: if fb creation failed, we can't rely on fb destroy
		 * to unref the bo:
		 */
		drm_gem_object_unreference(fbdev->bo);
		ret = PTR_ERR(fb);
		goto fail;
	}

	mutex_lock(&dev->struct_mutex);

	/*
	 * NOTE: if we can be guaranteed to be able to map buffer
	 * in panic (ie. lock-safe, etc) we could avoid pinning the
	 * buffer now:
	 */
	ret = msm_gem_get_iova_locked(fbdev->bo, 0, &paddr);
	if (ret) {
		dev_err(dev->dev, "failed to get buffer obj iova: %d\n", ret);
		goto fail_unlock;
	}

	fbi = framebuffer_alloc(0, dev->dev);
	if (!fbi) {
		dev_err(dev->dev, "failed to allocate fb info\n");
		ret = -ENOMEM;
		goto fail_unlock;
	}

	DBG("fbi=%p, dev=%p", fbi, dev);

	fbdev->fb = fb;
	helper->fb = fb;
	helper->fbdev = fbi;

	fbi->par = helper;
	fbi->flags = FBINFO_DEFAULT;
	fbi->fbops = &msm_fb_ops;

	strcpy(fbi->fix.id, "msm");

	ret = fb_alloc_cmap(&fbi->cmap, 256, 0);
	if (ret) {
		ret = -ENOMEM;
		goto fail_unlock;
	}

	drm_fb_helper_fill_fix(fbi, fb->pitches[0], fb->depth);
	drm_fb_helper_fill_var(fbi, helper, sizes->fb_width, sizes->fb_height);

	dev->mode_config.fb_base = paddr;

	fbi->screen_base = msm_gem_vaddr_locked(fbdev->bo);
	fbi->screen_size = fbdev->bo->size;
	fbi->fix.smem_start = paddr;
	fbi->fix.smem_len = fbdev->bo->size;

	DBG("par=%p, %dx%d", fbi->par, fbi->var.xres, fbi->var.yres);
	DBG("allocated %dx%d fb", fbdev->fb->width, fbdev->fb->height);

	mutex_unlock(&dev->struct_mutex);

	return 0;

fail_unlock:
	mutex_unlock(&dev->struct_mutex);
fail:

	if (ret) {
		framebuffer_release(fbi);
		if (fb) {
			drm_framebuffer_unregister_private(fb);
			drm_framebuffer_remove(fb);
		}
	}

	return ret;
}

static void msm_crtc_fb_gamma_set(struct drm_crtc *crtc,
		u16 red, u16 green, u16 blue, int regno)
{
	DBG("fbdev: set gamma");
}

static void msm_crtc_fb_gamma_get(struct drm_crtc *crtc,
		u16 *red, u16 *green, u16 *blue, int regno)
{
	DBG("fbdev: get gamma");
}

static const struct drm_fb_helper_funcs msm_fb_helper_funcs = {
	.gamma_set = msm_crtc_fb_gamma_set,
	.gamma_get = msm_crtc_fb_gamma_get,
	.fb_probe = msm_fbdev_create,
};

/* initialize fbdev helper */
struct drm_fb_helper *msm_fbdev_init(struct drm_device *dev)
{
	struct msm_drm_private *priv = dev->dev_private;
	struct msm_fbdev *fbdev = NULL;
	struct drm_fb_helper *helper;
	int ret;

	fbdev = kzalloc(sizeof(*fbdev), GFP_KERNEL);
	if (!fbdev)
		goto fail;

	helper = &fbdev->base;

	drm_fb_helper_prepare(dev, helper, &msm_fb_helper_funcs);

	ret = drm_fb_helper_init(dev, helper,
			priv->num_crtcs, priv->num_connectors);
	if (ret) {
		dev_err(dev->dev, "could not init fbdev: ret=%d\n", ret);
		goto fail;
	}

	ret = drm_fb_helper_single_add_all_connectors(helper);
	if (ret)
		goto fini;

	ret = drm_fb_helper_initial_config(helper, 32);
	if (ret)
		goto fini;

	priv->fbdev = helper;

	return helper;

fini:
	drm_fb_helper_fini(helper);
fail:
	kfree(fbdev);
	return NULL;
}

void msm_fbdev_free(struct drm_device *dev)
{
	struct msm_drm_private *priv = dev->dev_private;
	struct drm_fb_helper *helper = priv->fbdev;
	struct msm_fbdev *fbdev;
	struct fb_info *fbi;

	DBG();

	fbi = helper->fbdev;

	/* only cleanup framebuffer if it is present */
	if (fbi) {
		unregister_framebuffer(fbi);
		framebuffer_release(fbi);
	}

	drm_fb_helper_fini(helper);

	fbdev = to_msm_fbdev(priv->fbdev);

	/* this will free the backing object */
	if (fbdev->fb) {
		drm_framebuffer_unregister_private(fbdev->fb);
		drm_framebuffer_remove(fbdev->fb);
	}

	kfree(fbdev);

	priv->fbdev = NULL;
}
