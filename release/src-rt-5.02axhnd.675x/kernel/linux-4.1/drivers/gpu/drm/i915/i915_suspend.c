/*
 *
 * Copyright 2008 (c) Intel Corporation
 *   Jesse Barnes <jbarnes@virtuousgeek.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL TUNGSTEN GRAPHICS AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <drm/drmP.h>
#include <drm/i915_drm.h>
#include "intel_drv.h"
#include "i915_reg.h"

static void i915_save_display(struct drm_device *dev)
{
	struct drm_i915_private *dev_priv = dev->dev_private;

	/* Display arbitration control */
	if (INTEL_INFO(dev)->gen <= 4)
		dev_priv->regfile.saveDSPARB = I915_READ(DSPARB);

	/* LVDS state */
	if (HAS_PCH_IBX(dev) || HAS_PCH_CPT(dev))
		dev_priv->regfile.saveLVDS = I915_READ(PCH_LVDS);
	else if (INTEL_INFO(dev)->gen <= 4 && IS_MOBILE(dev) && !IS_I830(dev))
		dev_priv->regfile.saveLVDS = I915_READ(LVDS);

	/* Panel power sequencer */
	if (HAS_PCH_SPLIT(dev)) {
		dev_priv->regfile.savePP_CONTROL = I915_READ(PCH_PP_CONTROL);
		dev_priv->regfile.savePP_ON_DELAYS = I915_READ(PCH_PP_ON_DELAYS);
		dev_priv->regfile.savePP_OFF_DELAYS = I915_READ(PCH_PP_OFF_DELAYS);
		dev_priv->regfile.savePP_DIVISOR = I915_READ(PCH_PP_DIVISOR);
	} else if (!IS_VALLEYVIEW(dev)) {
		dev_priv->regfile.savePP_CONTROL = I915_READ(PP_CONTROL);
		dev_priv->regfile.savePP_ON_DELAYS = I915_READ(PP_ON_DELAYS);
		dev_priv->regfile.savePP_OFF_DELAYS = I915_READ(PP_OFF_DELAYS);
		dev_priv->regfile.savePP_DIVISOR = I915_READ(PP_DIVISOR);
	}

	/* save FBC interval */
	if (HAS_FBC(dev) && INTEL_INFO(dev)->gen <= 4 && !IS_G4X(dev))
		dev_priv->regfile.saveFBC_CONTROL = I915_READ(FBC_CONTROL);
}

static void i915_restore_display(struct drm_device *dev)
{
	struct drm_i915_private *dev_priv = dev->dev_private;
	u32 mask = 0xffffffff;

	/* Display arbitration */
	if (INTEL_INFO(dev)->gen <= 4)
		I915_WRITE(DSPARB, dev_priv->regfile.saveDSPARB);

	mask = ~LVDS_PORT_EN;

	/* LVDS state */
	if (HAS_PCH_IBX(dev) || HAS_PCH_CPT(dev))
		I915_WRITE(PCH_LVDS, dev_priv->regfile.saveLVDS & mask);
	else if (INTEL_INFO(dev)->gen <= 4 && IS_MOBILE(dev) && !IS_I830(dev))
		I915_WRITE(LVDS, dev_priv->regfile.saveLVDS & mask);

	/* Panel power sequencer */
	if (HAS_PCH_SPLIT(dev)) {
		I915_WRITE(PCH_PP_ON_DELAYS, dev_priv->regfile.savePP_ON_DELAYS);
		I915_WRITE(PCH_PP_OFF_DELAYS, dev_priv->regfile.savePP_OFF_DELAYS);
		I915_WRITE(PCH_PP_DIVISOR, dev_priv->regfile.savePP_DIVISOR);
		I915_WRITE(PCH_PP_CONTROL, dev_priv->regfile.savePP_CONTROL);
	} else if (!IS_VALLEYVIEW(dev)) {
		I915_WRITE(PP_ON_DELAYS, dev_priv->regfile.savePP_ON_DELAYS);
		I915_WRITE(PP_OFF_DELAYS, dev_priv->regfile.savePP_OFF_DELAYS);
		I915_WRITE(PP_DIVISOR, dev_priv->regfile.savePP_DIVISOR);
		I915_WRITE(PP_CONTROL, dev_priv->regfile.savePP_CONTROL);
	}

	/* only restore FBC info on the platform that supports FBC*/
	intel_fbc_disable(dev);

	/* restore FBC interval */
	if (HAS_FBC(dev) && INTEL_INFO(dev)->gen <= 4 && !IS_G4X(dev))
		I915_WRITE(FBC_CONTROL, dev_priv->regfile.saveFBC_CONTROL);

	i915_redisable_vga(dev);
}

int i915_save_state(struct drm_device *dev)
{
	struct drm_i915_private *dev_priv = dev->dev_private;
	int i;

	mutex_lock(&dev->struct_mutex);

	i915_save_display(dev);

	if (IS_GEN4(dev))
		pci_read_config_word(dev->pdev, GCDGMBUS,
				     &dev_priv->regfile.saveGCDGMBUS);

	/* Cache mode state */
	if (INTEL_INFO(dev)->gen < 7)
		dev_priv->regfile.saveCACHE_MODE_0 = I915_READ(CACHE_MODE_0);

	/* Memory Arbitration state */
	dev_priv->regfile.saveMI_ARB_STATE = I915_READ(MI_ARB_STATE);

	/* Scratch space */
	for (i = 0; i < 16; i++) {
		dev_priv->regfile.saveSWF0[i] = I915_READ(SWF00 + (i << 2));
		dev_priv->regfile.saveSWF1[i] = I915_READ(SWF10 + (i << 2));
	}
	for (i = 0; i < 3; i++)
		dev_priv->regfile.saveSWF2[i] = I915_READ(SWF30 + (i << 2));

	mutex_unlock(&dev->struct_mutex);

	return 0;
}

int i915_restore_state(struct drm_device *dev)
{
	struct drm_i915_private *dev_priv = dev->dev_private;
	int i;

	mutex_lock(&dev->struct_mutex);

	i915_gem_restore_fences(dev);

	if (IS_GEN4(dev))
		pci_write_config_word(dev->pdev, GCDGMBUS,
				      dev_priv->regfile.saveGCDGMBUS);
	i915_restore_display(dev);

	/* Cache mode state */
	if (INTEL_INFO(dev)->gen < 7)
		I915_WRITE(CACHE_MODE_0, dev_priv->regfile.saveCACHE_MODE_0 |
			   0xffff0000);

	/* Memory arbitration state */
	I915_WRITE(MI_ARB_STATE, dev_priv->regfile.saveMI_ARB_STATE | 0xffff0000);

	for (i = 0; i < 16; i++) {
		I915_WRITE(SWF00 + (i << 2), dev_priv->regfile.saveSWF0[i]);
		I915_WRITE(SWF10 + (i << 2), dev_priv->regfile.saveSWF1[i]);
	}
	for (i = 0; i < 3; i++)
		I915_WRITE(SWF30 + (i << 2), dev_priv->regfile.saveSWF2[i]);

	mutex_unlock(&dev->struct_mutex);

	intel_i2c_reset(dev);

	return 0;
}
