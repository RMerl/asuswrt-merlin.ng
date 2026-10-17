// SPDX-License-Identifier: GPL-2.0+
/* Copyright (C) 2019 Stephan Gerhold */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <video.h>
#include <asm/io.h>
#include <linux/bitfield.h>
#include <linux/iopoll.h>

#define MCDE_EXTSRC0A0			0x200
#define MCDE_EXTSRC0CONF		0x20C
#define MCDE_EXTSRC0CONF_BPP		GENMASK(11, 8)
#define MCDE_OVL0CONF			0x404
#define MCDE_OVL0CONF_PPL		GENMASK(10, 0)
#define MCDE_OVL0CONF_LPF		GENMASK(26, 16)
#define MCDE_CHNL0SYNCHMOD		0x608
#define MCDE_CHNL0SYNCHMOD_SRC_SYNCH	GENMASK(1, 0)
#define MCDE_CHNL0SYNCHSW		0x60C
#define MCDE_CHNL0SYNCHSW_SW_TRIG	BIT(0)
#define MCDE_CRA0			0x800
#define MCDE_CRA0_FLOEN			BIT(0)

#define MCDE_FLOW_COMPLETION_TIMEOUT	200000	/* us */

enum mcde_bpp {
	MCDE_EXTSRC0CONF_BPP_1BPP_PAL,
	MCDE_EXTSRC0CONF_BPP_2BPP_PAL,
	MCDE_EXTSRC0CONF_BPP_4BPP_PAL,
	MCDE_EXTSRC0CONF_BPP_8BPP_PAL,
	MCDE_EXTSRC0CONF_BPP_RGB444,
	MCDE_EXTSRC0CONF_BPP_ARGB4444,
	MCDE_EXTSRC0CONF_BPP_IRGB1555,
	MCDE_EXTSRC0CONF_BPP_RGB565,
	MCDE_EXTSRC0CONF_BPP_RGB888,
	MCDE_EXTSRC0CONF_BPP_XRGB8888,
	MCDE_EXTSRC0CONF_BPP_ARGB8888,
	MCDE_EXTSRC0CONF_BPP_YCBCR422,
};

enum mcde_src_synch {
	MCDE_CHNL0SYNCHMOD_SRC_SYNCH_HARDWARE,
	MCDE_CHNL0SYNCHMOD_SRC_SYNCH_NO_SYNCH,
	MCDE_CHNL0SYNCHMOD_SRC_SYNCH_SOFTWARE,
};

struct mcde_simple_priv {
	fdt_addr_t base;
	enum mcde_src_synch src_synch;
};

static int mcde_simple_probe(struct udevice *dev)
{
	struct mcde_simple_priv *priv = dev_get_priv(dev);
	struct video_uc_plat *plat = dev_get_uclass_plat(dev);
	struct video_priv *uc_priv = dev_get_uclass_priv(dev);
	u32 val;

	priv->base = dev_read_addr(dev);
	if (priv->base == FDT_ADDR_T_NONE)
		return -EINVAL;

	plat->base = readl(priv->base + MCDE_EXTSRC0A0);
	if (!plat->base)
		return -ENODEV;

	val = readl(priv->base + MCDE_OVL0CONF);
	uc_priv->xsize = FIELD_GET(MCDE_OVL0CONF_PPL, val);
	uc_priv->ysize = FIELD_GET(MCDE_OVL0CONF_LPF, val);
	uc_priv->rot = 0;

	val = readl(priv->base + MCDE_EXTSRC0CONF);
	switch (FIELD_GET(MCDE_EXTSRC0CONF_BPP, val)) {
	case MCDE_EXTSRC0CONF_BPP_RGB565:
		uc_priv->bpix = VIDEO_BPP16;
		break;
	case MCDE_EXTSRC0CONF_BPP_XRGB8888:
	case MCDE_EXTSRC0CONF_BPP_ARGB8888:
		uc_priv->bpix = VIDEO_BPP32;
		break;
	default:
		printf("unsupported format: %#x\n", val);
		return -EINVAL;
	}

	val = readl(priv->base + MCDE_CHNL0SYNCHMOD);
	priv->src_synch = FIELD_GET(MCDE_CHNL0SYNCHMOD_SRC_SYNCH, val);

	plat->size = uc_priv->xsize * uc_priv->ysize * VNBYTES(uc_priv->bpix);
	debug("MCDE base: %#lx, xsize: %d, ysize: %d, bpp: %d\n",
	      plat->base, uc_priv->xsize, uc_priv->ysize, VNBITS(uc_priv->bpix));

	video_set_flush_dcache(dev, true);
	return 0;
}

static int mcde_simple_video_sync(struct udevice *dev)
{
	struct mcde_simple_priv *priv = dev_get_priv(dev);
	unsigned int val;

	if (priv->src_synch != MCDE_CHNL0SYNCHMOD_SRC_SYNCH_SOFTWARE)
		return 0;

	/* Enable flow */
	val = readl(priv->base + MCDE_CRA0);
	val |= MCDE_CRA0_FLOEN;
	writel(val, priv->base + MCDE_CRA0);

	/* Trigger a software sync */
	writel(MCDE_CHNL0SYNCHSW_SW_TRIG, priv->base + MCDE_CHNL0SYNCHSW);

	/* Disable flow */
	val = readl(priv->base + MCDE_CRA0);
	val &= ~MCDE_CRA0_FLOEN;
	writel(val, priv->base + MCDE_CRA0);

	/* Wait for completion */
	return readl_poll_timeout(priv->base + MCDE_CRA0, val,
				  !(val & MCDE_CRA0_FLOEN),
				  MCDE_FLOW_COMPLETION_TIMEOUT);
}

static struct video_ops mcde_simple_ops = {
	.video_sync = mcde_simple_video_sync,
};

static const struct udevice_id mcde_simple_ids[] = {
	{ .compatible = "ste,mcde" },
	{ }
};

U_BOOT_DRIVER(mcde_simple) = {
	.name		= "mcde_simple",
	.id		= UCLASS_VIDEO,
	.ops		= &mcde_simple_ops,
	.of_match	= mcde_simple_ids,
	.probe		= mcde_simple_probe,
	.priv_auto	= sizeof(struct mcde_simple_priv),
};
