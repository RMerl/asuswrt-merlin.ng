// SPDX-License-Identifier: GPL-2.0
/*
 * From coreboot src/soc/intel/broadwell/igd.c
 *
 * Copyright (C) 2016 Google, Inc
 */

#include <common.h>
#include <bios_emul.h>
#include <dm.h>
#include <vbe.h>
#include <video.h>
#include <asm/cpu.h>
#include <asm/intel_regs.h>
#include <asm/io.h>
#include <asm/mtrr.h>
#include <asm/arch/cpu.h>
#include <asm/arch/iomap.h>
#include <asm/arch/pch.h>
#include "i915_reg.h"

struct broadwell_igd_priv {
	u8 *regs;
};

struct broadwell_igd_plat {
	u32 dp_hotplug[3];

	int port_select;
	int power_up_delay;
	int power_backlight_on_delay;
	int power_down_delay;
	int power_backlight_off_delay;
	int power_cycle_delay;
	int cpu_backlight;
	int pch_backlight;
	int cdclk;
	int pre_graphics_delay;
};

#define GT_RETRY		1000
#define GT_CDCLK_337		0
#define GT_CDCLK_450		1
#define GT_CDCLK_540		2
#define GT_CDCLK_675		3

u32 board_map_oprom_vendev(u32 vendev)
{
	return SA_IGD_OPROM_VENDEV;
}

static int poll32(u8 *addr, uint mask, uint value)
{
	ulong start;

	start = get_timer(0);
	debug("%s: addr %p = %x\n", __func__, addr, readl(addr));
	while ((readl(addr) & mask) != value) {
		if (get_timer(start) > GT_RETRY) {
			debug("poll32: timeout: %x\n", readl(addr));
			return -ETIMEDOUT;
		}
	}

	return 0;
}

static int haswell_early_init(struct udevice *dev)
{
	struct broadwell_igd_priv *priv = dev_get_priv(dev);
	u8 *regs = priv->regs;
	int ret;

	/* Enable Force Wake */
	writel(0x00000020, regs + 0xa180);
	writel(0x00010001, regs + 0xa188);
	ret = poll32(regs + 0x130044, 1, 1);
	if (ret)
		goto err;

	/* Enable Counters */
	setbits_le32(regs + 0xa248, 0x00000016);

	/* GFXPAUSE settings */
	writel(0x00070020, regs + 0xa000);

	/* ECO Settings */
	clrsetbits_le32(regs + 0xa180, ~0xff3fffff, 0x15000000);

	/* Enable DOP Clock Gating */
	writel(0x000003fd, regs + 0x9424);

	/* Enable Unit Level Clock Gating */
	writel(0x00000080, regs + 0x9400);
	writel(0x40401000, regs + 0x9404);
	writel(0x00000000, regs + 0x9408);
	writel(0x02000001, regs + 0x940c);

	/*
	 * RC6 Settings
	 */

	/* Wake Rate Limits */
	setbits_le32(regs + 0xa090, 0x00000000);
	setbits_le32(regs + 0xa098, 0x03e80000);
	setbits_le32(regs + 0xa09c, 0x00280000);
	setbits_le32(regs + 0xa0a8, 0x0001e848);
	setbits_le32(regs + 0xa0ac, 0x00000019);

	/* Render/Video/Blitter Idle Max Count */
	writel(0x0000000a, regs + 0x02054);
	writel(0x0000000a, regs + 0x12054);
	writel(0x0000000a, regs + 0x22054);
	writel(0x0000000a, regs + 0x1a054);

	/* RC Sleep / RCx Thresholds */
	setbits_le32(regs + 0xa0b0, 0x00000000);
	setbits_le32(regs + 0xa0b4, 0x000003e8);
	setbits_le32(regs + 0xa0b8, 0x0000c350);

	/* RP Settings */
	setbits_le32(regs + 0xa010, 0x000f4240);
	setbits_le32(regs + 0xa014, 0x12060000);
	setbits_le32(regs + 0xa02c, 0x0000e808);
	setbits_le32(regs + 0xa030, 0x0003bd08);
	setbits_le32(regs + 0xa068, 0x000101d0);
	setbits_le32(regs + 0xa06c, 0x00055730);
	setbits_le32(regs + 0xa070, 0x0000000a);

	/* RP Control */
	writel(0x00000b92, regs + 0xa024);

	/* HW RC6 Control */
	writel(0x88040000, regs + 0xa090);

	/* Video Frequency Request */
	writel(0x08000000, regs + 0xa00c);

	/* Set RC6 VIDs */
	ret = poll32(regs + 0x138124, (1 << 31), 0);
	if (ret)
		goto err;
	writel(0, regs + 0x138128);
	writel(0x80000004, regs + 0x138124);
	ret = poll32(regs + 0x138124, (1 << 31), 0);
	if (ret)
		goto err;

	/* Enable PM Interrupts */
	writel(0x03000076, regs + 0x4402c);

	/* Enable RC6 in idle */
	writel(0x00040000, regs + 0xa094);

	return 0;
err:
	debug("%s: ret=%d\n", __func__, ret);
	return ret;
};

static int haswell_late_init(struct udevice *dev)
{
	struct broadwell_igd_priv *priv = dev_get_priv(dev);
	u8 *regs = priv->regs;
	int ret;

	/* Lock settings */
	setbits_le32(regs + 0x0a248, (1 << 31));
	setbits_le32(regs + 0x0a004, (1 << 4));
	setbits_le32(regs + 0x0a080, (1 << 2));
	setbits_le32(regs + 0x0a180, (1 << 31));

	/* Disable Force Wake */
	writel(0x00010000, regs + 0xa188);
	ret = poll32(regs + 0x130044, 1, 0);
	if (ret)
		goto err;
	writel(0x00000001, regs + 0xa188);

	/* Enable power well for DP and Audio */
	setbits_le32(regs + 0x45400, (1 << 31));
	ret = poll32(regs + 0x45400, 1 << 30, 1 << 30);
	if (ret)
		goto err;

	return 0;
err:
	debug("%s: ret=%d\n", __func__, ret);
	return ret;
};

static int broadwell_early_init(struct udevice *dev)
{
	struct broadwell_igd_priv *priv = dev_get_priv(dev);
	u8 *regs = priv->regs;
	int ret;

	/* Enable Force Wake */
	writel(0x00010001, regs + 0xa188);
	ret = poll32(regs + 0x130044, 1, 1);
	if (ret)
		goto err;

	/* Enable push bus metric control and shift */
	writel(0x00000004, regs + 0xa248);
	writel(0x000000ff, regs + 0xa250);
	writel(0x00000010, regs + 0xa25c);

	/* GFXPAUSE settings (set based on stepping) */

	/* ECO Settings */
	writel(0x45200000, regs + 0xa180);

	/* Enable DOP Clock Gating */
	writel(0x000000fd, regs + 0x9424);

	/* Enable Unit Level Clock Gating */
	writel(0x00000000, regs + 0x9400);
	writel(0x40401000, regs + 0x9404);
	writel(0x00000000, regs + 0x9408);
	writel(0x02000001, regs + 0x940c);
	writel(0x0000000a, regs + 0x1a054);

	/* Video Frequency Request */
	writel(0x08000000, regs + 0xa00c);

	writel(0x00000009, regs + 0x138158);
	writel(0x0000000d, regs + 0x13815c);

	/*
	 * RC6 Settings
	 */

	/* Wake Rate Limits */
	clrsetbits_le32(regs + 0x0a090, ~0, 0);
	setbits_le32(regs + 0x0a098, 0x03e80000);
	setbits_le32(regs + 0x0a09c, 0x00280000);
	setbits_le32(regs + 0x0a0a8, 0x0001e848);
	setbits_le32(regs + 0x0a0ac, 0x00000019);

	/* Render/Video/Blitter Idle Max Count */
	writel(0x0000000a, regs + 0x02054);
	writel(0x0000000a, regs + 0x12054);
	writel(0x0000000a, regs + 0x22054);

	/* RC Sleep / RCx Thresholds */
	setbits_le32(regs + 0x0a0b0, 0x00000000);
	setbits_le32(regs + 0x0a0b8, 0x00000271);

	/* RP Settings */
	setbits_le32(regs + 0x0a010, 0x000f4240);
	setbits_le32(regs + 0x0a014, 0x12060000);
	setbits_le32(regs + 0x0a02c, 0x0000e808);
	setbits_le32(regs + 0x0a030, 0x0003bd08);
	setbits_le32(regs + 0x0a068, 0x000101d0);
	setbits_le32(regs + 0x0a06c, 0x00055730);
	setbits_le32(regs + 0x0a070, 0x0000000a);
	setbits_le32(regs + 0x0a168, 0x00000006);

	/* RP Control */
	writel(0x00000b92, regs + 0xa024);

	/* HW RC6 Control */
	writel(0x90040000, regs + 0xa090);

	/* Set RC6 VIDs */
	ret = poll32(regs + 0x138124, (1 << 31), 0);
	if (ret)
		goto err;
	writel(0, regs + 0x138128);
	writel(0x80000004, regs + 0x138124);
	ret = poll32(regs + 0x138124, (1 << 31), 0);
	if (ret)
		goto err;

	/* Enable PM Interrupts */
	writel(0x03000076, regs + 0x4402c);

	/* Enable RC6 in idle */
	writel(0x00040000, regs + 0xa094);

	return 0;
err:
	debug("%s: ret=%d\n", __func__, ret);
	return ret;
}

static int broadwell_late_init(struct udevice *dev)
{
	struct broadwell_igd_priv *priv = dev_get_priv(dev);
	u8 *regs = priv->regs;
	int ret;

	/* Lock settings */
	setbits_le32(regs + 0x0a248, 1 << 31);
	setbits_le32(regs + 0x0a000, 1 << 18);
	setbits_le32(regs + 0x0a180, 1 << 31);

	/* Disable Force Wake */
	writel(0x00010000, regs + 0xa188);
	ret = poll32(regs + 0x130044, 1, 0);
	if (ret)
		goto err;

	/* Enable power well for DP and Audio */
	setbits_le32(regs + 0x45400, 1 << 31);
	ret = poll32(regs + 0x45400, 1 << 30, 1 << 30);
	if (ret)
		goto err;

	return 0;
err:
	debug("%s: ret=%d\n", __func__, ret);
	return ret;
};


static unsigned long gtt_read(struct broadwell_igd_priv *priv,
			      unsigned long reg)
{
	return readl(priv->regs + reg);
}

static void gtt_write(struct broadwell_igd_priv *priv, unsigned long reg,
		      unsigned long data)
{
	writel(data, priv->regs + reg);
}

static inline void gtt_clrsetbits(struct broadwell_igd_priv *priv, u32 reg,
				  u32 bic, u32 or)
{
	clrsetbits_le32(priv->regs + reg, bic, or);
}

static int gtt_poll(struct broadwell_igd_priv *priv, u32 reg, u32 mask,
		    u32 value)
{
	unsigned try = GT_RETRY;
	u32 data;

	while (try--) {
		data = gtt_read(priv, reg);
		if ((data & mask) == value)
			return 0;
		udelay(10);
	}

	debug("GT init timeout\n");
	return -ETIMEDOUT;
}

static void igd_setup_panel(struct udevice *dev)
{
	struct broadwell_igd_plat *plat = dev_get_platdata(dev);
	struct broadwell_igd_priv *priv = dev_get_priv(dev);
	u32 reg32;

	/* Setup Digital Port Hotplug */
	reg32 = (plat->dp_hotplug[0] & 0x7) << 2;
	reg32 |= (plat->dp_hotplug[1] & 0x7) << 10;
	reg32 |= (plat->dp_hotplug[2] & 0x7) << 18;
	gtt_write(priv, PCH_PORT_HOTPLUG, reg32);

	/* Setup Panel Power On Delays */
	reg32 = (plat->port_select & 0x3) << 30;
	reg32 |= (plat->power_up_delay & 0x1fff) << 16;
	reg32 |= (plat->power_backlight_on_delay & 0x1fff);
	gtt_write(priv, PCH_PP_ON_DELAYS, reg32);

	/* Setup Panel Power Off Delays */
	reg32 = (plat->power_down_delay & 0x1fff) << 16;
	reg32 |= (plat->power_backlight_off_delay & 0x1fff);
	gtt_write(priv, PCH_PP_OFF_DELAYS, reg32);

	/* Setup Panel Power Cycle Delay */
	if (plat->power_cycle_delay) {
		reg32 = gtt_read(priv, PCH_PP_DIVISOR);
		reg32 &= ~0xff;
		reg32 |= plat->power_cycle_delay & 0xff;
		gtt_write(priv, PCH_PP_DIVISOR, reg32);
	}

	/* Enable Backlight if needed */
	if (plat->cpu_backlight) {
		gtt_write(priv, BLC_PWM_CPU_CTL2, BLC_PWM2_ENABLE);
		gtt_write(priv, BLC_PWM_CPU_CTL, plat->cpu_backlight);
	}
	if (plat->pch_backlight) {
		gtt_write(priv, BLC_PWM_PCH_CTL1, BLM_PCH_PWM_ENABLE);
		gtt_write(priv, BLC_PWM_PCH_CTL2, plat->pch_backlight);
	}
}

static int igd_cdclk_init_haswell(struct udevice *dev)
{
	struct broadwell_igd_plat *plat = dev_get_platdata(dev);
	struct broadwell_igd_priv *priv = dev_get_priv(dev);
	int cdclk = plat->cdclk;
	u16 devid;
	int gpu_is_ulx = 0;
	u32 dpdiv, lpcll;
	int ret;

	dm_pci_read_config16(dev, PCI_DEVICE_ID, &devid);

	/* Check for ULX GT1 or GT2 */
	if (devid == 0x0a0e || devid == 0x0a1e)
		gpu_is_ulx = 1;

	/* 675MHz is not supported on haswell */
	if (cdclk == GT_CDCLK_675)
		cdclk = GT_CDCLK_337;

	/* If CD clock is fixed or ULT then set to 450MHz */
	if ((gtt_read(priv, 0x42014) & 0x1000000) || cpu_is_ult())
		cdclk = GT_CDCLK_450;

	/* 540MHz is not supported on ULX */
	if (gpu_is_ulx && cdclk == GT_CDCLK_540)
		cdclk = GT_CDCLK_337;

	/* 337.5MHz is not supported on non-ULT/ULX */
	if (!gpu_is_ulx && !cpu_is_ult() && cdclk == GT_CDCLK_337)
		cdclk = GT_CDCLK_450;

	/* Set variables based on CD Clock setting */
	switch (cdclk) {
	case GT_CDCLK_337:
		dpdiv = 169;
		lpcll = (1 << 26);
		break;
	case GT_CDCLK_450:
		dpdiv = 225;
		lpcll = 0;
		break;
	case GT_CDCLK_540:
		dpdiv = 270;
		lpcll = (1 << 26);
		break;
	default:
		ret = -EDOM;
		goto err;
	}

	/* Set LPCLL_CTL CD Clock Frequency Select */
	gtt_clrsetbits(priv, 0x130040, ~0xf3ffffff, lpcll);

	/* ULX: Inform power controller of selected frequency */
	if (gpu_is_ulx) {
		if (cdclk == GT_CDCLK_450)
			gtt_write(priv, 0x138128, 0x00000000); /* 450MHz */
		else
			gtt_write(priv, 0x138128, 0x00000001); /* 337.5MHz */
		gtt_write(priv, 0x13812c, 0x00000000);
		gtt_write(priv, 0x138124, 0x80000017);
	}

	/* Set CPU DP AUX 2X bit clock dividers */
	gtt_clrsetbits(priv, 0x64010, ~0xfffff800, dpdiv);
	gtt_clrsetbits(priv, 0x64810, ~0xfffff800, dpdiv);

	return 0;
err:
	debug("%s: ret=%d\n", __func__, ret);
	return ret;
}

static int igd_cdclk_init_broadwell(struct udevice *dev)
{
	struct broadwell_igd_plat *plat = dev_get_platdata(dev);
	struct broadwell_igd_priv *priv = dev_get_priv(dev);
	int cdclk = plat->cdclk;
	u32 dpdiv, lpcll, pwctl, cdset;
	int ret;

	/* Inform power controller of upcoming frequency change */
	gtt_write(priv, 0x138128, 0);
	gtt_write(priv, 0x13812c, 0);
	gtt_write(priv, 0x138124, 0x80000018);

	/* Poll GT driver mailbox for run/busy clear */
	if (gtt_poll(priv, 0x138124, 1 << 31, 0 << 31))
		cdclk = GT_CDCLK_450;

	if (gtt_read(priv, 0x42014) & 0x1000000) {
		/* If CD clock is fixed then set to 450MHz */
		cdclk = GT_CDCLK_450;
	} else {
		/* Program CD clock to highest supported freq */
		if (cpu_is_ult())
			cdclk = GT_CDCLK_540;
		else
			cdclk = GT_CDCLK_675;
	}

	/* CD clock frequency 675MHz not supported on ULT */
	if (cpu_is_ult() && cdclk == GT_CDCLK_675)
		cdclk = GT_CDCLK_540;

	/* Set variables based on CD Clock setting */
	switch (cdclk) {
	case GT_CDCLK_337:
		cdset = 337;
		lpcll = (1 << 27);
		pwctl = 2;
		dpdiv = 169;
		break;
	case GT_CDCLK_450:
		cdset = 449;
		lpcll = 0;
		pwctl = 0;
		dpdiv = 225;
		break;
	case GT_CDCLK_540:
		cdset = 539;
		lpcll = (1 << 26);
		pwctl = 1;
		dpdiv = 270;
		break;
	case GT_CDCLK_675:
		cdset = 674;
		lpcll = (1 << 26) | (1 << 27);
		pwctl = 3;
		dpdiv = 338;
		break;
	default:
		ret = -EDOM;
		goto err;
	}
	debug("%s: frequency = %d\n", __func__, cdclk);

	/* Set LPCLL_CTL CD Clock Frequency Select */
	gtt_clrsetbits(priv, 0x130040, ~0xf3ffffff, lpcll);

	/* Inform power controller of selected frequency */
	gtt_write(priv, 0x138128, pwctl);
	gtt_write(priv, 0x13812c, 0);
	gtt_write(priv, 0x138124, 0x80000017);

	/* Program CD Clock Frequency */
	gtt_clrsetbits(priv, 0x46200, ~0xfffffc00, cdset);

	/* Set CPU DP AUX 2X bit clock dividers */
	gtt_clrsetbits(priv, 0x64010, ~0xfffff800, dpdiv);
	gtt_clrsetbits(priv, 0x64810, ~0xfffff800, dpdiv);

	return 0;
err:
	debug("%s: ret=%d\n", __func__, ret);
	return ret;
}

u8 systemagent_revision(struct udevice *bus)
{
	ulong val;

	pci_bus_read_config(bus, PCI_BDF(0, 0, 0), PCI_REVISION_ID, &val,
			    PCI_SIZE_32);

	return val;
}

static int igd_pre_init(struct udevice *dev, bool is_broadwell)
{
	struct broadwell_igd_plat *plat = dev_get_platdata(dev);
	struct broadwell_igd_priv *priv = dev_get_priv(dev);
	u32 rp1_gfx_freq;
	int ret;

	mdelay(plat->pre_graphics_delay);

	/* Early init steps */
	if (is_broadwell) {
		ret = broadwell_early_init(dev);
		if (ret)
			goto err;

		/* Set GFXPAUSE based on stepping */
		if (cpu_get_stepping() <= (CPUID_BROADWELL_E0 & 0xf) &&
		    systemagent_revision(pci_get_controller(dev)) <= 9) {
			gtt_write(priv, 0xa000, 0x300ff);
		} else {
			gtt_write(priv, 0xa000, 0x30020);
		}
	} else {
		ret = haswell_early_init(dev);
		if (ret)
			goto err;
	}

	/* Set RP1 graphics frequency */
	rp1_gfx_freq = (readl(MCHBAR_REG(0x5998)) >> 8) & 0xff;
	gtt_write(priv, 0xa008, rp1_gfx_freq << 24);

	/* Post VBIOS panel setup */
	igd_setup_panel(dev);

	return 0;
err:
	debug("%s: ret=%d\n", __func__, ret);
	return ret;
}

static int igd_post_init(struct udevice *dev, bool is_broadwell)
{
	int ret;

	/* Late init steps */
	if (is_broadwell) {
		ret = igd_cdclk_init_broadwell(dev);
		if (ret)
			return ret;
		ret = broadwell_late_init(dev);
		if (ret)
			return ret;
	} else {
		igd_cdclk_init_haswell(dev);
		ret = haswell_late_init(dev);
		if (ret)
			return ret;
	}

	return 0;
}

static int broadwell_igd_int15_handler(void)
{
	int res = 0;

	debug("%s: INT15 function %04x!\n", __func__, M.x86.R_AX);

	switch (M.x86.R_AX) {
	case 0x5f35:
		/*
		 * Boot Display Device Hook:
		 *  bit 0 = CRT
		 *  bit 1 = TV (eDP)
		 *  bit 2 = EFP
		 *  bit 3 = LFP
		 *  bit 4 = CRT2
		 *  bit 5 = TV2 (eDP)
		 *  bit 6 = EFP2
		 *  bit 7 = LFP2
		 */
		M.x86.R_AX = 0x005f;
		M.x86.R_CX = 0x0000; /* Use video bios default */
		res = 1;
		break;
	default:
		debug("Unknown INT15 function %04x!\n", M.x86.R_AX);
		break;
	}

	return res;
}

static int broadwell_igd_probe(struct udevice *dev)
{
	struct video_uc_platdata *plat = dev_get_uclass_platdata(dev);
	struct video_priv *uc_priv = dev_get_uclass_priv(dev);
	bool is_broadwell;
	int ret;

	if (!ll_boot_init()) {
		/*
		 * If we are running from EFI or coreboot, this driver can't
		 * work.
		 */
		printf("Not available (previous bootloader prevents it)\n");
		return -EPERM;
	}
	is_broadwell = cpu_get_family_model() == BROADWELL_FAMILY_ULT;
	bootstage_start(BOOTSTAGE_ID_ACCUM_LCD, "vesa display");
	debug("%s: is_broadwell=%d\n", __func__, is_broadwell);
	ret = igd_pre_init(dev, is_broadwell);
	if (!ret) {
		ret = vbe_setup_video(dev, broadwell_igd_int15_handler);
		if (ret)
			debug("failed to run video BIOS: %d\n", ret);
	}
	if (!ret)
		ret = igd_post_init(dev, is_broadwell);
	bootstage_accum(BOOTSTAGE_ID_ACCUM_LCD);
	if (ret)
		return ret;

	/* Use write-combining for the graphics memory, 256MB */
	ret = mtrr_add_request(MTRR_TYPE_WRCOMB, plat->base, 256 << 20);
	if (!ret)
		ret = mtrr_commit(true);
	if (ret && ret != -ENOSYS) {
		printf("Failed to add MTRR: Display will be slow (err %d)\n",
		       ret);
	}

	debug("fb=%lx, size %x, display size=%d %d %d\n", plat->base,
	      plat->size, uc_priv->xsize, uc_priv->ysize, uc_priv->bpix);

	return 0;
}

static int broadwell_igd_ofdata_to_platdata(struct udevice *dev)
{
	struct broadwell_igd_plat *plat = dev_get_platdata(dev);
	struct broadwell_igd_priv *priv = dev_get_priv(dev);
	int node = dev_of_offset(dev);
	const void *blob = gd->fdt_blob;

	if (fdtdec_get_int_array(blob, node, "intel,dp-hotplug",
				 plat->dp_hotplug,
				 ARRAY_SIZE(plat->dp_hotplug)))
		return -EINVAL;
	plat->port_select = fdtdec_get_int(blob, node, "intel,port-select", 0);
	plat->power_cycle_delay = fdtdec_get_int(blob, node,
			"intel,power-cycle-delay", 0);
	plat->power_up_delay = fdtdec_get_int(blob, node,
			"intel,power-up-delay", 0);
	plat->power_down_delay = fdtdec_get_int(blob, node,
			"intel,power-down-delay", 0);
	plat->power_backlight_on_delay = fdtdec_get_int(blob, node,
			"intel,power-backlight-on-delay", 0);
	plat->power_backlight_off_delay = fdtdec_get_int(blob, node,
			"intel,power-backlight-off-delay", 0);
	plat->cpu_backlight = fdtdec_get_int(blob, node,
			"intel,cpu-backlight", 0);
	plat->pch_backlight = fdtdec_get_int(blob, node,
			"intel,pch-backlight", 0);
	plat->pre_graphics_delay = fdtdec_get_int(blob, node,
			"intel,pre-graphics-delay", 0);
	priv->regs = (u8 *)dm_pci_read_bar32(dev, 0);
	debug("%s: regs at %p\n", __func__, priv->regs);
	debug("dp_hotplug %d %d %d\n", plat->dp_hotplug[0], plat->dp_hotplug[1],
	      plat->dp_hotplug[2]);
	debug("port_select = %d\n", plat->port_select);
	debug("power_up_delay = %d\n", plat->power_up_delay);
	debug("power_backlight_on_delay = %d\n",
	      plat->power_backlight_on_delay);
	debug("power_down_delay = %d\n", plat->power_down_delay);
	debug("power_backlight_off_delay = %d\n",
	      plat->power_backlight_off_delay);
	debug("power_cycle_delay = %d\n", plat->power_cycle_delay);
	debug("cpu_backlight = %x\n", plat->cpu_backlight);
	debug("pch_backlight = %x\n", plat->pch_backlight);
	debug("cdclk = %d\n", plat->cdclk);
	debug("pre_graphics_delay = %d\n", plat->pre_graphics_delay);

	return 0;
}

static const struct video_ops broadwell_igd_ops = {
};

static const struct udevice_id broadwell_igd_ids[] = {
	{ .compatible = "intel,broadwell-igd" },
	{ }
};

U_BOOT_DRIVER(broadwell_igd) = {
	.name	= "broadwell_igd",
	.id	= UCLASS_VIDEO,
	.of_match = broadwell_igd_ids,
	.ops	= &broadwell_igd_ops,
	.ofdata_to_platdata = broadwell_igd_ofdata_to_platdata,
	.probe	= broadwell_igd_probe,
	.priv_auto_alloc_size	= sizeof(struct broadwell_igd_priv),
	.platdata_auto_alloc_size	= sizeof(struct broadwell_igd_plat),
};
