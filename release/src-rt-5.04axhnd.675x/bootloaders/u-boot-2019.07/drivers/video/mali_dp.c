// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016-2018 ARM Ltd.
 * Author: Liviu Dudau <liviu.dudau@foss.arm.com>
 *
 */
#define DEBUG
#include <common.h>
#include <video.h>
#include <dm.h>
#ifdef CONFIG_DISPLAY
#include <display.h>
#endif
#include <fdtdec.h>
#include <asm/io.h>
#include <os.h>
#include <fdt_support.h>
#include <clk.h>
#include <linux/sizes.h>

#define MALIDP_CORE_ID		0x0018
#define MALIDP_REG_BG_COLOR	0x0044
#define MALIDP_LAYER_LV1	0x0100
#define MALIDP_DC_STATUS	0xc000
#define MALIDP_DC_CONTROL	0xc010
#define MALIDP_DC_CFG_VALID	0xc014

/* offsets inside the modesetting register block */
#define MALIDP_H_INTERVALS	0x0000
#define MALIDP_V_INTERVALS	0x0004
#define MALIDP_SYNC_CONTROL	0x0008
#define MALIDP_HV_ACTIVESIZE	0x000c
#define MALIDP_OUTPUT_DEPTH	0x001c

/* offsets inside the layer register block */
#define MALIDP_LAYER_FORMAT	0x0000
#define MALIDP_LAYER_CONTROL	0x0004
#define MALIDP_LAYER_IN_SIZE	0x000c
#define MALIDP_LAYER_CMP_SIZE	0x0010
#define MALIDP_LAYER_STRIDE	0x0018
#define MALIDP_LAYER_PTR_LOW	0x0024
#define MALIDP_LAYER_PTR_HIGH	0x0028

/* offsets inside the IRQ control blocks */
#define MALIDP_REG_MASKIRQ	0x0008
#define MALIDP_REG_CLEARIRQ	0x000c

#define M1BITS	0x0001
#define M2BITS	0x0003
#define M4BITS	0x000f
#define M8BITS	0x00ff
#define M10BITS	0x03ff
#define M12BITS	0x0fff
#define M13BITS	0x1fff
#define M16BITS	0xffff
#define M17BITS	0x1ffff

#define MALIDP_H_FRONTPORCH(x)	(((x) & M12BITS) << 0)
#define MALIDP_H_BACKPORCH(x)	(((x) & M10BITS) << 16)
#define MALIDP_V_FRONTPORCH(x)	(((x) & M12BITS) << 0)
#define MALIDP_V_BACKPORCH(x)	(((x) & M8BITS) << 16)
#define MALIDP_H_SYNCWIDTH(x)	(((x) & M10BITS) << 0)
#define MALIDP_V_SYNCWIDTH(x)	(((x) & M8BITS) << 16)
#define MALIDP_H_ACTIVE(x)	(((x) & M13BITS) << 0)
#define MALIDP_V_ACTIVE(x)	(((x) & M13BITS) << 16)

#define MALIDP_CMP_V_SIZE(x)	(((x) & M13BITS) << 16)
#define MALIDP_CMP_H_SIZE(x)	(((x) & M13BITS) << 0)

#define MALIDP_IN_V_SIZE(x)	(((x) & M13BITS) << 16)
#define MALIDP_IN_H_SIZE(x)	(((x) & M13BITS) << 0)

#define MALIDP_DC_CM_CONTROL(x)	((x) & M1BITS) << 16, 1 << 16
#define MALIDP_DC_STATUS_GET_CM(reg) (((reg) >> 16) & M1BITS)

#define MALIDP_FORMAT_ARGB8888	0x08
#define MALIDP_DEFAULT_BG_R 0x0
#define MALIDP_DEFAULT_BG_G 0x0
#define MALIDP_DEFAULT_BG_B 0x0

#define MALIDP_PRODUCT_ID(core_id)	((u32)(core_id) >> 16)

#define MALIDP500	0x500

DECLARE_GLOBAL_DATA_PTR;

struct malidp_priv {
	phys_addr_t base_addr;
	phys_addr_t dc_status_addr;
	phys_addr_t dc_control_addr;
	phys_addr_t cval_addr;
	struct udevice *display;	/* display device attached */
	struct clk aclk;
	struct clk pxlclk;
	u16 modeset_regs_offset;
	u8 config_bit_shift;
	u8 clear_irq;			/* offset for IRQ clear register */
};

static const struct video_ops malidp_ops = {
};

static int malidp_get_hwid(phys_addr_t base_addr)
{
	int hwid;

	/*
	 * reading from the old CORE_ID offset will always
	 * return 0x5000000 on DP500
	 */
	hwid = readl(base_addr + MALIDP_CORE_ID);
	if (MALIDP_PRODUCT_ID(hwid) == MALIDP500)
		return hwid;
	/* otherwise try the other gen CORE_ID offset */
	hwid = readl(base_addr + MALIDP_DC_STATUS + MALIDP_CORE_ID);

	return hwid;
}

/*
 * wait for config mode bit setup to be acted upon by the hardware
 */
static int malidp_wait_configdone(struct malidp_priv *malidp)
{
	u32 status, tries = 300;

	while (tries--) {
		status = readl(malidp->dc_status_addr);
		if ((status >> malidp->config_bit_shift) & 1)
			break;
		udelay(500);
	}

	if (!tries)
		return -ETIMEDOUT;

	return 0;
}

/*
 * signal the hardware to enter configuration mode
 */
static int malidp_enter_config(struct malidp_priv *malidp)
{
	setbits_le32(malidp->dc_control_addr, 1 << malidp->config_bit_shift);
	return malidp_wait_configdone(malidp);
}

/*
 * signal the hardware to exit configuration mode
 */
static int malidp_leave_config(struct malidp_priv *malidp)
{
	clrbits_le32(malidp->dc_control_addr, 1 << malidp->config_bit_shift);
	return malidp_wait_configdone(malidp);
}

static void malidp_setup_timings(struct malidp_priv *malidp,
				 struct display_timing *timings)
{
	u32 val = MALIDP_H_SYNCWIDTH(timings->hsync_len.typ) |
		  MALIDP_V_SYNCWIDTH(timings->vsync_len.typ);
	writel(val, malidp->base_addr + malidp->modeset_regs_offset +
	       MALIDP_SYNC_CONTROL);
	val = MALIDP_H_BACKPORCH(timings->hback_porch.typ) |
		MALIDP_H_FRONTPORCH(timings->hfront_porch.typ);
	writel(val, malidp->base_addr + malidp->modeset_regs_offset +
	       MALIDP_H_INTERVALS);
	val = MALIDP_V_BACKPORCH(timings->vback_porch.typ) |
		MALIDP_V_FRONTPORCH(timings->vfront_porch.typ);
	writel(val, malidp->base_addr + malidp->modeset_regs_offset +
	       MALIDP_V_INTERVALS);
	val = MALIDP_H_ACTIVE(timings->hactive.typ) |
		MALIDP_V_ACTIVE(timings->vactive.typ);
	writel(val, malidp->base_addr + malidp->modeset_regs_offset +
	       MALIDP_HV_ACTIVESIZE);
	/* default output bit-depth per colour is 8 bits */
	writel(0x080808, malidp->base_addr + malidp->modeset_regs_offset +
	       MALIDP_OUTPUT_DEPTH);
}

static int malidp_setup_mode(struct malidp_priv *malidp,
			     struct display_timing *timings)
{
	int err;

	if (clk_set_rate(&malidp->pxlclk, timings->pixelclock.typ) == 0)
		return -EIO;

	malidp_setup_timings(malidp, timings);

	err = display_enable(malidp->display, 8, timings);
	if (err)
		printf("display_enable failed with %d\n", err);

	return err;
}

static void malidp_setup_layer(struct malidp_priv *malidp,
			       struct display_timing *timings,
			       u32 layer_offset, phys_addr_t fb_addr)
{
	u32 val;

	/* setup the base layer's pixel format to A8R8G8B8 */
	writel(MALIDP_FORMAT_ARGB8888, malidp->base_addr + layer_offset +
	       MALIDP_LAYER_FORMAT);
	/* setup layer composition size */
	val = MALIDP_CMP_V_SIZE(timings->vactive.typ) |
		MALIDP_CMP_H_SIZE(timings->hactive.typ);
	writel(val, malidp->base_addr + layer_offset +
	       MALIDP_LAYER_CMP_SIZE);
	/* setup layer input size */
	val = MALIDP_IN_V_SIZE(timings->vactive.typ) |
		MALIDP_IN_H_SIZE(timings->hactive.typ);
	writel(val, malidp->base_addr + layer_offset + MALIDP_LAYER_IN_SIZE);
	/* setup layer stride in bytes */
	writel(timings->hactive.typ << 2, malidp->base_addr + layer_offset +
	       MALIDP_LAYER_STRIDE);
	/* set framebuffer address */
	writel(lower_32_bits(fb_addr), malidp->base_addr + layer_offset +
	       MALIDP_LAYER_PTR_LOW);
	writel(upper_32_bits(fb_addr), malidp->base_addr + layer_offset +
	       MALIDP_LAYER_PTR_HIGH);
	/* enable layer */
	setbits_le32(malidp->base_addr + layer_offset +
		     MALIDP_LAYER_CONTROL, 1);
}

static void malidp_set_configvalid(struct malidp_priv *malidp)
{
	setbits_le32(malidp->cval_addr, 1);
}

static int malidp_update_timings_from_edid(struct udevice *dev,
					   struct display_timing *timings)
{
#ifdef CONFIG_DISPLAY
	struct malidp_priv *priv = dev_get_priv(dev);
	struct udevice *disp_dev;
	int err;

	err = uclass_first_device(UCLASS_DISPLAY, &disp_dev);
	if (err)
		return err;

	priv->display = disp_dev;

	err = display_read_timing(disp_dev, timings);
	if (err)
		return err;

#endif
	return 0;
}

static int malidp_probe(struct udevice *dev)
{
	struct video_priv *uc_priv = dev_get_uclass_priv(dev);
	struct video_uc_platdata *uc_plat = dev_get_uclass_platdata(dev);
	ofnode framebuffer = ofnode_find_subnode(dev_ofnode(dev), "framebuffer");
	struct malidp_priv *priv = dev_get_priv(dev);
	struct display_timing timings;
	phys_addr_t fb_base, fb_size;
	const char *format;
	u32 value;
	int err;

	if (!ofnode_valid(framebuffer))
		return -EINVAL;

	err = clk_get_by_name(dev, "pxlclk", &priv->pxlclk);
	if (err) {
		dev_err(dev, "failed to get pixel clock\n");
		return err;
	}
	err = clk_get_by_name(dev, "aclk", &priv->aclk);
	if (err) {
		dev_err(dev, "failed to get AXI clock\n");
		goto fail_aclk;
	}

	err = ofnode_decode_display_timing(dev_ofnode(dev), 1, &timings);
	if (err) {
		dev_err(dev, "failed to get any display timings\n");
		goto fail_timings;
	}

	err = malidp_update_timings_from_edid(dev, &timings);
	if (err) {
		printf("malidp_update_timings_from_edid failed: %d\n", err);
		goto fail_timings;
	}

	fb_base = ofnode_get_addr_size(framebuffer, "reg", &fb_size);
	if (fb_base != FDT_ADDR_T_NONE) {
		uc_plat->base = fb_base;
		uc_plat->size = fb_size;
	} else {
		printf("cannot get address size for framebuffer\n");
	}

	err = ofnode_read_u32(framebuffer, "width", &value);
	if (err)
		goto fail_timings;
	uc_priv->xsize = (ushort)value;

	err = ofnode_read_u32(framebuffer, "height", &value);
	if (err)
		goto fail_timings;
	uc_priv->ysize = (ushort)value;

	format = ofnode_read_string(framebuffer, "format");
	if (!format) {
		err = -EINVAL;
		goto fail_timings;
	} else if (!strncmp(format, "a8r8g8b8", 8)) {
		uc_priv->bpix = VIDEO_BPP32;
	}

	uc_priv->rot = 0;
	priv->base_addr = (phys_addr_t)dev_read_addr(dev);

	clk_enable(&priv->pxlclk);
	clk_enable(&priv->aclk);

	value = malidp_get_hwid(priv->base_addr);
	printf("Display: Arm Mali DP%3x r%dp%d\n", MALIDP_PRODUCT_ID(value),
	       (value >> 12) & 0xf, (value >> 8) & 0xf);

	if (MALIDP_PRODUCT_ID(value) == MALIDP500) {
		/* DP500 is special */
		priv->modeset_regs_offset = 0x28;
		priv->dc_status_addr = priv->base_addr;
		priv->dc_control_addr = priv->base_addr + 0xc;
		priv->cval_addr = priv->base_addr + 0xf00;
		priv->config_bit_shift = 17;
		priv->clear_irq = 0;
	} else {
		priv->modeset_regs_offset = 0x30;
		priv->dc_status_addr = priv->base_addr + MALIDP_DC_STATUS;
		priv->dc_control_addr = priv->base_addr + MALIDP_DC_CONTROL;
		priv->cval_addr = priv->base_addr + MALIDP_DC_CFG_VALID;
		priv->config_bit_shift = 16;
		priv->clear_irq = MALIDP_REG_CLEARIRQ;
	}

	/* enter config mode */
	err  = malidp_enter_config(priv);
	if (err)
		return err;

	/* disable interrupts */
	writel(0, priv->dc_status_addr + MALIDP_REG_MASKIRQ);
	writel(0xffffffff, priv->dc_status_addr + priv->clear_irq);

	err = malidp_setup_mode(priv, &timings);
	if (err)
		goto fail_timings;

	malidp_setup_layer(priv, &timings, MALIDP_LAYER_LV1,
			   (phys_addr_t)uc_plat->base);

	err = malidp_leave_config(priv);
	if (err)
		goto fail_timings;

	malidp_set_configvalid(priv);

	return 0;

fail_timings:
	clk_free(&priv->aclk);
fail_aclk:
	clk_free(&priv->pxlclk);

	return err;
}

static int malidp_bind(struct udevice *dev)
{
	struct video_uc_platdata *uc_plat = dev_get_uclass_platdata(dev);

	/* choose max possible size: 2K x 2K, XRGB888 framebuffer */
	uc_plat->size = 4 * 2048 * 2048;

	return 0;
}

static const struct udevice_id malidp_ids[] = {
	{ .compatible = "arm,mali-dp500" },
	{ .compatible = "arm,mali-dp550" },
	{ .compatible = "arm,mali-dp650" },
	{ }
};

U_BOOT_DRIVER(mali_dp) = {
	.name		= "mali_dp",
	.id		= UCLASS_VIDEO,
	.of_match	= malidp_ids,
	.bind		= malidp_bind,
	.probe		= malidp_probe,
	.priv_auto_alloc_size	= sizeof(struct malidp_priv),
	.ops		= &malidp_ops,
};
