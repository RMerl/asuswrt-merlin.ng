/*
 * rcar_du_crtc.c  --  R-Car Display Unit CRTCs
 *
 * Copyright (C) 2013-2014 Renesas Electronics Corporation
 *
 * Contact: Laurent Pinchart (laurent.pinchart@ideasonboard.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/clk.h>
#include <linux/mutex.h>

#include <drm/drmP.h>
#include <drm/drm_atomic.h>
#include <drm/drm_atomic_helper.h>
#include <drm/drm_crtc.h>
#include <drm/drm_crtc_helper.h>
#include <drm/drm_fb_cma_helper.h>
#include <drm/drm_gem_cma_helper.h>
#include <drm/drm_plane_helper.h>

#include "rcar_du_crtc.h"
#include "rcar_du_drv.h"
#include "rcar_du_kms.h"
#include "rcar_du_plane.h"
#include "rcar_du_regs.h"

static u32 rcar_du_crtc_read(struct rcar_du_crtc *rcrtc, u32 reg)
{
	struct rcar_du_device *rcdu = rcrtc->group->dev;

	return rcar_du_read(rcdu, rcrtc->mmio_offset + reg);
}

static void rcar_du_crtc_write(struct rcar_du_crtc *rcrtc, u32 reg, u32 data)
{
	struct rcar_du_device *rcdu = rcrtc->group->dev;

	rcar_du_write(rcdu, rcrtc->mmio_offset + reg, data);
}

static void rcar_du_crtc_clr(struct rcar_du_crtc *rcrtc, u32 reg, u32 clr)
{
	struct rcar_du_device *rcdu = rcrtc->group->dev;

	rcar_du_write(rcdu, rcrtc->mmio_offset + reg,
		      rcar_du_read(rcdu, rcrtc->mmio_offset + reg) & ~clr);
}

static void rcar_du_crtc_set(struct rcar_du_crtc *rcrtc, u32 reg, u32 set)
{
	struct rcar_du_device *rcdu = rcrtc->group->dev;

	rcar_du_write(rcdu, rcrtc->mmio_offset + reg,
		      rcar_du_read(rcdu, rcrtc->mmio_offset + reg) | set);
}

static void rcar_du_crtc_clr_set(struct rcar_du_crtc *rcrtc, u32 reg,
				 u32 clr, u32 set)
{
	struct rcar_du_device *rcdu = rcrtc->group->dev;
	u32 value = rcar_du_read(rcdu, rcrtc->mmio_offset + reg);

	rcar_du_write(rcdu, rcrtc->mmio_offset + reg, (value & ~clr) | set);
}

static int rcar_du_crtc_get(struct rcar_du_crtc *rcrtc)
{
	int ret;

	ret = clk_prepare_enable(rcrtc->clock);
	if (ret < 0)
		return ret;

	ret = clk_prepare_enable(rcrtc->extclock);
	if (ret < 0)
		goto error_clock;

	ret = rcar_du_group_get(rcrtc->group);
	if (ret < 0)
		goto error_group;

	return 0;

error_group:
	clk_disable_unprepare(rcrtc->extclock);
error_clock:
	clk_disable_unprepare(rcrtc->clock);
	return ret;
}

static void rcar_du_crtc_put(struct rcar_du_crtc *rcrtc)
{
	rcar_du_group_put(rcrtc->group);

	clk_disable_unprepare(rcrtc->extclock);
	clk_disable_unprepare(rcrtc->clock);
}

/* -----------------------------------------------------------------------------
 * Hardware Setup
 */

static void rcar_du_crtc_set_display_timing(struct rcar_du_crtc *rcrtc)
{
	const struct drm_display_mode *mode = &rcrtc->crtc.state->adjusted_mode;
	unsigned long mode_clock = mode->clock * 1000;
	unsigned long clk;
	u32 value;
	u32 escr;
	u32 div;

	/* Compute the clock divisor and select the internal or external dot
	 * clock based on the requested frequency.
	 */
	clk = clk_get_rate(rcrtc->clock);
	div = DIV_ROUND_CLOSEST(clk, mode_clock);
	div = clamp(div, 1U, 64U) - 1;
	escr = div | ESCR_DCLKSEL_CLKS;

	if (rcrtc->extclock) {
		unsigned long extclk;
		unsigned long extrate;
		unsigned long rate;
		u32 extdiv;

		extclk = clk_get_rate(rcrtc->extclock);
		extdiv = DIV_ROUND_CLOSEST(extclk, mode_clock);
		extdiv = clamp(extdiv, 1U, 64U) - 1;

		rate = clk / (div + 1);
		extrate = extclk / (extdiv + 1);

		if (abs((long)extrate - (long)mode_clock) <
		    abs((long)rate - (long)mode_clock)) {
			dev_dbg(rcrtc->group->dev->dev,
				"crtc%u: using external clock\n", rcrtc->index);
			escr = extdiv | ESCR_DCLKSEL_DCLKIN;
		}
	}

	rcar_du_group_write(rcrtc->group, rcrtc->index % 2 ? ESCR2 : ESCR,
			    escr);
	rcar_du_group_write(rcrtc->group, rcrtc->index % 2 ? OTAR2 : OTAR, 0);

	/* Signal polarities */
	value = ((mode->flags & DRM_MODE_FLAG_PVSYNC) ? DSMR_VSL : 0)
	      | ((mode->flags & DRM_MODE_FLAG_PHSYNC) ? DSMR_HSL : 0)
	      | DSMR_DIPM_DE | DSMR_CSPM;
	rcar_du_crtc_write(rcrtc, DSMR, value);

	/* Display timings */
	rcar_du_crtc_write(rcrtc, HDSR, mode->htotal - mode->hsync_start - 19);
	rcar_du_crtc_write(rcrtc, HDER, mode->htotal - mode->hsync_start +
					mode->hdisplay - 19);
	rcar_du_crtc_write(rcrtc, HSWR, mode->hsync_end -
					mode->hsync_start - 1);
	rcar_du_crtc_write(rcrtc, HCR,  mode->htotal - 1);

	rcar_du_crtc_write(rcrtc, VDSR, mode->crtc_vtotal -
					mode->crtc_vsync_end - 2);
	rcar_du_crtc_write(rcrtc, VDER, mode->crtc_vtotal -
					mode->crtc_vsync_end +
					mode->crtc_vdisplay - 2);
	rcar_du_crtc_write(rcrtc, VSPR, mode->crtc_vtotal -
					mode->crtc_vsync_end +
					mode->crtc_vsync_start - 1);
	rcar_du_crtc_write(rcrtc, VCR,  mode->crtc_vtotal - 1);

	rcar_du_crtc_write(rcrtc, DESR,  mode->htotal - mode->hsync_start - 1);
	rcar_du_crtc_write(rcrtc, DEWR,  mode->hdisplay);
}

void rcar_du_crtc_route_output(struct drm_crtc *crtc,
			       enum rcar_du_output output)
{
	struct rcar_du_crtc *rcrtc = to_rcar_crtc(crtc);
	struct rcar_du_device *rcdu = rcrtc->group->dev;

	/* Store the route from the CRTC output to the DU output. The DU will be
	 * configured when starting the CRTC.
	 */
	rcrtc->outputs |= BIT(output);

	/* Store RGB routing to DPAD0, the hardware will be configured when
	 * starting the CRTC.
	 */
	if (output == RCAR_DU_OUTPUT_DPAD0)
		rcdu->dpad0_source = rcrtc->index;
}

static unsigned int plane_zpos(struct rcar_du_plane *plane)
{
	return to_rcar_du_plane_state(plane->plane.state)->zpos;
}

static const struct rcar_du_format_info *
plane_format(struct rcar_du_plane *plane)
{
	return to_rcar_du_plane_state(plane->plane.state)->format;
}

static void rcar_du_crtc_update_planes(struct rcar_du_crtc *rcrtc)
{
	struct rcar_du_plane *planes[RCAR_DU_NUM_HW_PLANES];
	unsigned int num_planes = 0;
	unsigned int prio = 0;
	unsigned int i;
	u32 dptsr = 0;
	u32 dspr = 0;

	for (i = 0; i < ARRAY_SIZE(rcrtc->group->planes.planes); ++i) {
		struct rcar_du_plane *plane = &rcrtc->group->planes.planes[i];
		unsigned int j;

		if (plane->plane.state->crtc != &rcrtc->crtc)
			continue;

		/* Insert the plane in the sorted planes array. */
		for (j = num_planes++; j > 0; --j) {
			if (plane_zpos(planes[j-1]) <= plane_zpos(plane))
				break;
			planes[j] = planes[j-1];
		}

		planes[j] = plane;
		prio += plane_format(plane)->planes * 4;
	}

	for (i = 0; i < num_planes; ++i) {
		struct rcar_du_plane *plane = planes[i];
		struct drm_plane_state *state = plane->plane.state;
		unsigned int index = to_rcar_du_plane_state(state)->hwindex;

		prio -= 4;
		dspr |= (index + 1) << prio;
		dptsr |= DPTSR_PnDK(index) |  DPTSR_PnTS(index);

		if (plane_format(plane)->planes == 2) {
			index = (index + 1) % 8;

			prio -= 4;
			dspr |= (index + 1) << prio;
			dptsr |= DPTSR_PnDK(index) |  DPTSR_PnTS(index);
		}
	}

	/* Select display timing and dot clock generator 2 for planes associated
	 * with superposition controller 2.
	 */
	if (rcrtc->index % 2) {
		/* The DPTSR register is updated when the display controller is
		 * stopped. We thus need to restart the DU. Once again, sorry
		 * for the flicker. One way to mitigate the issue would be to
		 * pre-associate planes with CRTCs (either with a fixed 4/4
		 * split, or through a module parameter). Flicker would then
		 * occur only if we need to break the pre-association.
		 */
		mutex_lock(&rcrtc->group->lock);
		if (rcar_du_group_read(rcrtc->group, DPTSR) != dptsr) {
			rcar_du_group_write(rcrtc->group, DPTSR, dptsr);
			if (rcrtc->group->used_crtcs)
				rcar_du_group_restart(rcrtc->group);
		}
		mutex_unlock(&rcrtc->group->lock);
	}

	rcar_du_group_write(rcrtc->group, rcrtc->index % 2 ? DS2PR : DS1PR,
			    dspr);
}

/* -----------------------------------------------------------------------------
 * Page Flip
 */

static void rcar_du_crtc_finish_page_flip(struct rcar_du_crtc *rcrtc)
{
	struct drm_pending_vblank_event *event;
	struct drm_device *dev = rcrtc->crtc.dev;
	unsigned long flags;

	spin_lock_irqsave(&dev->event_lock, flags);
	event = rcrtc->event;
	rcrtc->event = NULL;
	spin_unlock_irqrestore(&dev->event_lock, flags);

	if (event == NULL)
		return;

	spin_lock_irqsave(&dev->event_lock, flags);
	drm_send_vblank_event(dev, rcrtc->index, event);
	wake_up(&rcrtc->flip_wait);
	spin_unlock_irqrestore(&dev->event_lock, flags);

	drm_crtc_vblank_put(&rcrtc->crtc);
}

static bool rcar_du_crtc_page_flip_pending(struct rcar_du_crtc *rcrtc)
{
	struct drm_device *dev = rcrtc->crtc.dev;
	unsigned long flags;
	bool pending;

	spin_lock_irqsave(&dev->event_lock, flags);
	pending = rcrtc->event != NULL;
	spin_unlock_irqrestore(&dev->event_lock, flags);

	return pending;
}

static void rcar_du_crtc_wait_page_flip(struct rcar_du_crtc *rcrtc)
{
	struct rcar_du_device *rcdu = rcrtc->group->dev;

	if (wait_event_timeout(rcrtc->flip_wait,
			       !rcar_du_crtc_page_flip_pending(rcrtc),
			       msecs_to_jiffies(50)))
		return;

	dev_warn(rcdu->dev, "page flip timeout\n");

	rcar_du_crtc_finish_page_flip(rcrtc);
}

/* -----------------------------------------------------------------------------
 * Start/Stop and Suspend/Resume
 */

static void rcar_du_crtc_start(struct rcar_du_crtc *rcrtc)
{
	struct drm_crtc *crtc = &rcrtc->crtc;
	bool interlaced;

	if (rcrtc->started)
		return;

	/* Set display off and background to black */
	rcar_du_crtc_write(rcrtc, DOOR, DOOR_RGB(0, 0, 0));
	rcar_du_crtc_write(rcrtc, BPOR, BPOR_RGB(0, 0, 0));

	/* Configure display timings and output routing */
	rcar_du_crtc_set_display_timing(rcrtc);
	rcar_du_group_set_routing(rcrtc->group);

	/* Start with all planes disabled. */
	rcar_du_group_write(rcrtc->group, rcrtc->index % 2 ? DS2PR : DS1PR, 0);

	/* Select master sync mode. This enables display operation in master
	 * sync mode (with the HSYNC and VSYNC signals configured as outputs and
	 * actively driven).
	 */
	interlaced = rcrtc->crtc.mode.flags & DRM_MODE_FLAG_INTERLACE;
	rcar_du_crtc_clr_set(rcrtc, DSYSR, DSYSR_TVM_MASK | DSYSR_SCM_MASK,
			     (interlaced ? DSYSR_SCM_INT_VIDEO : 0) |
			     DSYSR_TVM_MASTER);

	rcar_du_group_start_stop(rcrtc->group, true);

	/* Turn vertical blanking interrupt reporting back on. */
	drm_crtc_vblank_on(crtc);

	rcrtc->started = true;
}

static void rcar_du_crtc_stop(struct rcar_du_crtc *rcrtc)
{
	struct drm_crtc *crtc = &rcrtc->crtc;

	if (!rcrtc->started)
		return;

	/* Disable vertical blanking interrupt reporting. We first need to wait
	 * for page flip completion before stopping the CRTC as userspace
	 * expects page flips to eventually complete.
	 */
	rcar_du_crtc_wait_page_flip(rcrtc);
	drm_crtc_vblank_off(crtc);

	/* Select switch sync mode. This stops display operation and configures
	 * the HSYNC and VSYNC signals as inputs.
	 */
	rcar_du_crtc_clr_set(rcrtc, DSYSR, DSYSR_TVM_MASK, DSYSR_TVM_SWITCH);

	rcar_du_group_start_stop(rcrtc->group, false);

	rcrtc->started = false;
}

void rcar_du_crtc_suspend(struct rcar_du_crtc *rcrtc)
{
	rcar_du_crtc_stop(rcrtc);
	rcar_du_crtc_put(rcrtc);
}

void rcar_du_crtc_resume(struct rcar_du_crtc *rcrtc)
{
	unsigned int i;

	if (!rcrtc->enabled)
		return;

	rcar_du_crtc_get(rcrtc);
	rcar_du_crtc_start(rcrtc);

	/* Commit the planes state. */
	for (i = 0; i < ARRAY_SIZE(rcrtc->group->planes.planes); ++i) {
		struct rcar_du_plane *plane = &rcrtc->group->planes.planes[i];

		if (plane->plane.state->crtc != &rcrtc->crtc)
			continue;

		rcar_du_plane_setup(plane);
	}

	rcar_du_crtc_update_planes(rcrtc);
}

/* -----------------------------------------------------------------------------
 * CRTC Functions
 */

static void rcar_du_crtc_enable(struct drm_crtc *crtc)
{
	struct rcar_du_crtc *rcrtc = to_rcar_crtc(crtc);

	if (rcrtc->enabled)
		return;

	rcar_du_crtc_get(rcrtc);
	rcar_du_crtc_start(rcrtc);

	rcrtc->enabled = true;
}

static void rcar_du_crtc_disable(struct drm_crtc *crtc)
{
	struct rcar_du_crtc *rcrtc = to_rcar_crtc(crtc);

	if (!rcrtc->enabled)
		return;

	rcar_du_crtc_stop(rcrtc);
	rcar_du_crtc_put(rcrtc);

	rcrtc->enabled = false;
	rcrtc->outputs = 0;
}

static bool rcar_du_crtc_mode_fixup(struct drm_crtc *crtc,
				    const struct drm_display_mode *mode,
				    struct drm_display_mode *adjusted_mode)
{
	/* TODO Fixup modes */
	return true;
}

static void rcar_du_crtc_atomic_begin(struct drm_crtc *crtc)
{
	struct drm_pending_vblank_event *event = crtc->state->event;
	struct rcar_du_crtc *rcrtc = to_rcar_crtc(crtc);
	struct drm_device *dev = rcrtc->crtc.dev;
	unsigned long flags;

	if (event) {
		WARN_ON(drm_crtc_vblank_get(crtc) != 0);

		spin_lock_irqsave(&dev->event_lock, flags);
		rcrtc->event = event;
		spin_unlock_irqrestore(&dev->event_lock, flags);
	}
}

static void rcar_du_crtc_atomic_flush(struct drm_crtc *crtc)
{
	struct rcar_du_crtc *rcrtc = to_rcar_crtc(crtc);

	rcar_du_crtc_update_planes(rcrtc);
}

static const struct drm_crtc_helper_funcs crtc_helper_funcs = {
	.mode_fixup = rcar_du_crtc_mode_fixup,
	.disable = rcar_du_crtc_disable,
	.enable = rcar_du_crtc_enable,
	.atomic_begin = rcar_du_crtc_atomic_begin,
	.atomic_flush = rcar_du_crtc_atomic_flush,
};

static const struct drm_crtc_funcs crtc_funcs = {
	.reset = drm_atomic_helper_crtc_reset,
	.destroy = drm_crtc_cleanup,
	.set_config = drm_atomic_helper_set_config,
	.page_flip = drm_atomic_helper_page_flip,
	.atomic_duplicate_state = drm_atomic_helper_crtc_duplicate_state,
	.atomic_destroy_state = drm_atomic_helper_crtc_destroy_state,
};

/* -----------------------------------------------------------------------------
 * Interrupt Handling
 */

static irqreturn_t rcar_du_crtc_irq(int irq, void *arg)
{
	struct rcar_du_crtc *rcrtc = arg;
	irqreturn_t ret = IRQ_NONE;
	u32 status;

	status = rcar_du_crtc_read(rcrtc, DSSR);
	rcar_du_crtc_write(rcrtc, DSRCR, status & DSRCR_MASK);

	if (status & DSSR_VBK) {
		drm_handle_vblank(rcrtc->crtc.dev, rcrtc->index);
		rcar_du_crtc_finish_page_flip(rcrtc);
		ret = IRQ_HANDLED;
	}

	return ret;
}

/* -----------------------------------------------------------------------------
 * Initialization
 */

int rcar_du_crtc_create(struct rcar_du_group *rgrp, unsigned int index)
{
	static const unsigned int mmio_offsets[] = {
		DU0_REG_OFFSET, DU1_REG_OFFSET, DU2_REG_OFFSET
	};

	struct rcar_du_device *rcdu = rgrp->dev;
	struct platform_device *pdev = to_platform_device(rcdu->dev);
	struct rcar_du_crtc *rcrtc = &rcdu->crtcs[index];
	struct drm_crtc *crtc = &rcrtc->crtc;
	unsigned int irqflags;
	struct clk *clk;
	char clk_name[9];
	char *name;
	int irq;
	int ret;

	/* Get the CRTC clock and the optional external clock. */
	if (rcar_du_has(rcdu, RCAR_DU_FEATURE_CRTC_IRQ_CLOCK)) {
		sprintf(clk_name, "du.%u", index);
		name = clk_name;
	} else {
		name = NULL;
	}

	rcrtc->clock = devm_clk_get(rcdu->dev, name);
	if (IS_ERR(rcrtc->clock)) {
		dev_err(rcdu->dev, "no clock for CRTC %u\n", index);
		return PTR_ERR(rcrtc->clock);
	}

	sprintf(clk_name, "dclkin.%u", index);
	clk = devm_clk_get(rcdu->dev, clk_name);
	if (!IS_ERR(clk)) {
		rcrtc->extclock = clk;
	} else if (PTR_ERR(rcrtc->clock) == -EPROBE_DEFER) {
		dev_info(rcdu->dev, "can't get external clock %u\n", index);
		return -EPROBE_DEFER;
	}

	init_waitqueue_head(&rcrtc->flip_wait);

	rcrtc->group = rgrp;
	rcrtc->mmio_offset = mmio_offsets[index];
	rcrtc->index = index;
	rcrtc->enabled = false;

	ret = drm_crtc_init_with_planes(rcdu->ddev, crtc,
					&rgrp->planes.planes[index % 2].plane,
					NULL, &crtc_funcs);
	if (ret < 0)
		return ret;

	drm_crtc_helper_add(crtc, &crtc_helper_funcs);

	/* Start with vertical blanking interrupt reporting disabled. */
	drm_crtc_vblank_off(crtc);

	/* Register the interrupt handler. */
	if (rcar_du_has(rcdu, RCAR_DU_FEATURE_CRTC_IRQ_CLOCK)) {
		irq = platform_get_irq(pdev, index);
		irqflags = 0;
	} else {
		irq = platform_get_irq(pdev, 0);
		irqflags = IRQF_SHARED;
	}

	if (irq < 0) {
		dev_err(rcdu->dev, "no IRQ for CRTC %u\n", index);
		return irq;
	}

	ret = devm_request_irq(rcdu->dev, irq, rcar_du_crtc_irq, irqflags,
			       dev_name(rcdu->dev), rcrtc);
	if (ret < 0) {
		dev_err(rcdu->dev,
			"failed to register IRQ for CRTC %u\n", index);
		return ret;
	}

	return 0;
}

void rcar_du_crtc_enable_vblank(struct rcar_du_crtc *rcrtc, bool enable)
{
	if (enable) {
		rcar_du_crtc_write(rcrtc, DSRCR, DSRCR_VBCL);
		rcar_du_crtc_set(rcrtc, DIER, DIER_VBE);
	} else {
		rcar_du_crtc_clr(rcrtc, DIER, DIER_VBE);
	}
}
