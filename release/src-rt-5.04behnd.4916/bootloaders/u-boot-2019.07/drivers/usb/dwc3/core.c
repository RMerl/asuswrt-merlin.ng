// SPDX-License-Identifier: GPL-2.0
/**
 * core.c - DesignWare USB3 DRD Controller Core file
 *
 * Copyright (C) 2015 Texas Instruments Incorporated - http://www.ti.com
 *
 * Authors: Felipe Balbi <balbi@ti.com>,
 *	    Sebastian Andrzej Siewior <bigeasy@linutronix.de>
 *
 * Taken from Linux Kernel v3.19-rc1 (drivers/usb/dwc3/core.c) and ported
 * to uboot.
 *
 * commit cd72f890d2 : usb: dwc3: core: enable phy suspend quirk on non-FPGA
 */

#include <common.h>
#include <malloc.h>
#include <dwc3-uboot.h>
#include <asm/dma-mapping.h>
#include <linux/ioport.h>
#include <dm.h>
#include <generic-phy.h>
#include <linux/usb/ch9.h>
#include <linux/usb/gadget.h>

#include "core.h"
#include "gadget.h"
#include "io.h"

#include "linux-compat.h"

static LIST_HEAD(dwc3_list);
/* -------------------------------------------------------------------------- */

static void dwc3_set_mode(struct dwc3 *dwc, u32 mode)
{
	u32 reg;

	reg = dwc3_readl(dwc->regs, DWC3_GCTL);
	reg &= ~(DWC3_GCTL_PRTCAPDIR(DWC3_GCTL_PRTCAP_OTG));
	reg |= DWC3_GCTL_PRTCAPDIR(mode);
	dwc3_writel(dwc->regs, DWC3_GCTL, reg);
}

/**
 * dwc3_core_soft_reset - Issues core soft reset and PHY reset
 * @dwc: pointer to our context structure
 */
static int dwc3_core_soft_reset(struct dwc3 *dwc)
{
	u32		reg;

	/* Before Resetting PHY, put Core in Reset */
	reg = dwc3_readl(dwc->regs, DWC3_GCTL);
	reg |= DWC3_GCTL_CORESOFTRESET;
	dwc3_writel(dwc->regs, DWC3_GCTL, reg);

	/* Assert USB3 PHY reset */
	reg = dwc3_readl(dwc->regs, DWC3_GUSB3PIPECTL(0));
	reg |= DWC3_GUSB3PIPECTL_PHYSOFTRST;
	dwc3_writel(dwc->regs, DWC3_GUSB3PIPECTL(0), reg);

	/* Assert USB2 PHY reset */
	reg = dwc3_readl(dwc->regs, DWC3_GUSB2PHYCFG(0));
	reg |= DWC3_GUSB2PHYCFG_PHYSOFTRST;
	dwc3_writel(dwc->regs, DWC3_GUSB2PHYCFG(0), reg);

	mdelay(100);

	/* Clear USB3 PHY reset */
	reg = dwc3_readl(dwc->regs, DWC3_GUSB3PIPECTL(0));
	reg &= ~DWC3_GUSB3PIPECTL_PHYSOFTRST;
	dwc3_writel(dwc->regs, DWC3_GUSB3PIPECTL(0), reg);

	/* Clear USB2 PHY reset */
	reg = dwc3_readl(dwc->regs, DWC3_GUSB2PHYCFG(0));
	reg &= ~DWC3_GUSB2PHYCFG_PHYSOFTRST;
	dwc3_writel(dwc->regs, DWC3_GUSB2PHYCFG(0), reg);

	mdelay(100);

	/* After PHYs are stable we can take Core out of reset state */
	reg = dwc3_readl(dwc->regs, DWC3_GCTL);
	reg &= ~DWC3_GCTL_CORESOFTRESET;
	dwc3_writel(dwc->regs, DWC3_GCTL, reg);

	return 0;
}

/**
 * dwc3_free_one_event_buffer - Frees one event buffer
 * @dwc: Pointer to our controller context structure
 * @evt: Pointer to event buffer to be freed
 */
static void dwc3_free_one_event_buffer(struct dwc3 *dwc,
		struct dwc3_event_buffer *evt)
{
	dma_free_coherent(evt->buf);
}

/**
 * dwc3_alloc_one_event_buffer - Allocates one event buffer structure
 * @dwc: Pointer to our controller context structure
 * @length: size of the event buffer
 *
 * Returns a pointer to the allocated event buffer structure on success
 * otherwise ERR_PTR(errno).
 */
static struct dwc3_event_buffer *dwc3_alloc_one_event_buffer(struct dwc3 *dwc,
		unsigned length)
{
	struct dwc3_event_buffer	*evt;

	evt = devm_kzalloc((struct udevice *)dwc->dev, sizeof(*evt),
			   GFP_KERNEL);
	if (!evt)
		return ERR_PTR(-ENOMEM);

	evt->dwc	= dwc;
	evt->length	= length;
	evt->buf	= dma_alloc_coherent(length,
					     (unsigned long *)&evt->dma);
	if (!evt->buf)
		return ERR_PTR(-ENOMEM);

	dwc3_flush_cache((uintptr_t)evt->buf, evt->length);

	return evt;
}

/**
 * dwc3_free_event_buffers - frees all allocated event buffers
 * @dwc: Pointer to our controller context structure
 */
static void dwc3_free_event_buffers(struct dwc3 *dwc)
{
	struct dwc3_event_buffer	*evt;
	int i;

	for (i = 0; i < dwc->num_event_buffers; i++) {
		evt = dwc->ev_buffs[i];
		if (evt)
			dwc3_free_one_event_buffer(dwc, evt);
	}
}

/**
 * dwc3_alloc_event_buffers - Allocates @num event buffers of size @length
 * @dwc: pointer to our controller context structure
 * @length: size of event buffer
 *
 * Returns 0 on success otherwise negative errno. In the error case, dwc
 * may contain some buffers allocated but not all which were requested.
 */
static int dwc3_alloc_event_buffers(struct dwc3 *dwc, unsigned length)
{
	int			num;
	int			i;

	num = DWC3_NUM_INT(dwc->hwparams.hwparams1);
	dwc->num_event_buffers = num;

	dwc->ev_buffs = memalign(CONFIG_SYS_CACHELINE_SIZE,
				 sizeof(*dwc->ev_buffs) * num);
	if (!dwc->ev_buffs)
		return -ENOMEM;

	for (i = 0; i < num; i++) {
		struct dwc3_event_buffer	*evt;

		evt = dwc3_alloc_one_event_buffer(dwc, length);
		if (IS_ERR(evt)) {
			dev_err(dwc->dev, "can't allocate event buffer\n");
			return PTR_ERR(evt);
		}
		dwc->ev_buffs[i] = evt;
	}

	return 0;
}

/**
 * dwc3_event_buffers_setup - setup our allocated event buffers
 * @dwc: pointer to our controller context structure
 *
 * Returns 0 on success otherwise negative errno.
 */
static int dwc3_event_buffers_setup(struct dwc3 *dwc)
{
	struct dwc3_event_buffer	*evt;
	int				n;

	for (n = 0; n < dwc->num_event_buffers; n++) {
		evt = dwc->ev_buffs[n];
		dev_dbg(dwc->dev, "Event buf %p dma %08llx length %d\n",
				evt->buf, (unsigned long long) evt->dma,
				evt->length);

		evt->lpos = 0;

		dwc3_writel(dwc->regs, DWC3_GEVNTADRLO(n),
				lower_32_bits(evt->dma));
		dwc3_writel(dwc->regs, DWC3_GEVNTADRHI(n),
				upper_32_bits(evt->dma));
		dwc3_writel(dwc->regs, DWC3_GEVNTSIZ(n),
				DWC3_GEVNTSIZ_SIZE(evt->length));
		dwc3_writel(dwc->regs, DWC3_GEVNTCOUNT(n), 0);
	}

	return 0;
}

static void dwc3_event_buffers_cleanup(struct dwc3 *dwc)
{
	struct dwc3_event_buffer	*evt;
	int				n;

	for (n = 0; n < dwc->num_event_buffers; n++) {
		evt = dwc->ev_buffs[n];

		evt->lpos = 0;

		dwc3_writel(dwc->regs, DWC3_GEVNTADRLO(n), 0);
		dwc3_writel(dwc->regs, DWC3_GEVNTADRHI(n), 0);
		dwc3_writel(dwc->regs, DWC3_GEVNTSIZ(n), DWC3_GEVNTSIZ_INTMASK
				| DWC3_GEVNTSIZ_SIZE(0));
		dwc3_writel(dwc->regs, DWC3_GEVNTCOUNT(n), 0);
	}
}

static int dwc3_alloc_scratch_buffers(struct dwc3 *dwc)
{
	if (!dwc->has_hibernation)
		return 0;

	if (!dwc->nr_scratch)
		return 0;

	dwc->scratchbuf = kmalloc_array(dwc->nr_scratch,
			DWC3_SCRATCHBUF_SIZE, GFP_KERNEL);
	if (!dwc->scratchbuf)
		return -ENOMEM;

	return 0;
}

static int dwc3_setup_scratch_buffers(struct dwc3 *dwc)
{
	dma_addr_t scratch_addr;
	u32 param;
	int ret;

	if (!dwc->has_hibernation)
		return 0;

	if (!dwc->nr_scratch)
		return 0;

	scratch_addr = dma_map_single(dwc->scratchbuf,
				      dwc->nr_scratch * DWC3_SCRATCHBUF_SIZE,
				      DMA_BIDIRECTIONAL);
	if (dma_mapping_error(dwc->dev, scratch_addr)) {
		dev_err(dwc->dev, "failed to map scratch buffer\n");
		ret = -EFAULT;
		goto err0;
	}

	dwc->scratch_addr = scratch_addr;

	param = lower_32_bits(scratch_addr);

	ret = dwc3_send_gadget_generic_command(dwc,
			DWC3_DGCMD_SET_SCRATCHPAD_ADDR_LO, param);
	if (ret < 0)
		goto err1;

	param = upper_32_bits(scratch_addr);

	ret = dwc3_send_gadget_generic_command(dwc,
			DWC3_DGCMD_SET_SCRATCHPAD_ADDR_HI, param);
	if (ret < 0)
		goto err1;

	return 0;

err1:
	dma_unmap_single((void *)(uintptr_t)dwc->scratch_addr, dwc->nr_scratch *
			 DWC3_SCRATCHBUF_SIZE, DMA_BIDIRECTIONAL);

err0:
	return ret;
}

static void dwc3_free_scratch_buffers(struct dwc3 *dwc)
{
	if (!dwc->has_hibernation)
		return;

	if (!dwc->nr_scratch)
		return;

	dma_unmap_single((void *)(uintptr_t)dwc->scratch_addr, dwc->nr_scratch *
			 DWC3_SCRATCHBUF_SIZE, DMA_BIDIRECTIONAL);
	kfree(dwc->scratchbuf);
}

static void dwc3_core_num_eps(struct dwc3 *dwc)
{
	struct dwc3_hwparams	*parms = &dwc->hwparams;

	dwc->num_in_eps = DWC3_NUM_IN_EPS(parms);
	dwc->num_out_eps = DWC3_NUM_EPS(parms) - dwc->num_in_eps;

	dev_vdbg(dwc->dev, "found %d IN and %d OUT endpoints\n",
			dwc->num_in_eps, dwc->num_out_eps);
}

static void dwc3_cache_hwparams(struct dwc3 *dwc)
{
	struct dwc3_hwparams	*parms = &dwc->hwparams;

	parms->hwparams0 = dwc3_readl(dwc->regs, DWC3_GHWPARAMS0);
	parms->hwparams1 = dwc3_readl(dwc->regs, DWC3_GHWPARAMS1);
	parms->hwparams2 = dwc3_readl(dwc->regs, DWC3_GHWPARAMS2);
	parms->hwparams3 = dwc3_readl(dwc->regs, DWC3_GHWPARAMS3);
	parms->hwparams4 = dwc3_readl(dwc->regs, DWC3_GHWPARAMS4);
	parms->hwparams5 = dwc3_readl(dwc->regs, DWC3_GHWPARAMS5);
	parms->hwparams6 = dwc3_readl(dwc->regs, DWC3_GHWPARAMS6);
	parms->hwparams7 = dwc3_readl(dwc->regs, DWC3_GHWPARAMS7);
	parms->hwparams8 = dwc3_readl(dwc->regs, DWC3_GHWPARAMS8);
}

/**
 * dwc3_phy_setup - Configure USB PHY Interface of DWC3 Core
 * @dwc: Pointer to our controller context structure
 */
static void dwc3_phy_setup(struct dwc3 *dwc)
{
	u32 reg;

	reg = dwc3_readl(dwc->regs, DWC3_GUSB3PIPECTL(0));

	/*
	 * Above 1.94a, it is recommended to set DWC3_GUSB3PIPECTL_SUSPHY
	 * to '0' during coreConsultant configuration. So default value
	 * will be '0' when the core is reset. Application needs to set it
	 * to '1' after the core initialization is completed.
	 */
	if (dwc->revision > DWC3_REVISION_194A)
		reg |= DWC3_GUSB3PIPECTL_SUSPHY;

	if (dwc->u2ss_inp3_quirk)
		reg |= DWC3_GUSB3PIPECTL_U2SSINP3OK;

	if (dwc->req_p1p2p3_quirk)
		reg |= DWC3_GUSB3PIPECTL_REQP1P2P3;

	if (dwc->del_p1p2p3_quirk)
		reg |= DWC3_GUSB3PIPECTL_DEP1P2P3_EN;

	if (dwc->del_phy_power_chg_quirk)
		reg |= DWC3_GUSB3PIPECTL_DEPOCHANGE;

	if (dwc->lfps_filter_quirk)
		reg |= DWC3_GUSB3PIPECTL_LFPSFILT;

	if (dwc->rx_detect_poll_quirk)
		reg |= DWC3_GUSB3PIPECTL_RX_DETOPOLL;

	if (dwc->tx_de_emphasis_quirk)
		reg |= DWC3_GUSB3PIPECTL_TX_DEEPH(dwc->tx_de_emphasis);

	if (dwc->dis_u3_susphy_quirk)
		reg &= ~DWC3_GUSB3PIPECTL_SUSPHY;

	dwc3_writel(dwc->regs, DWC3_GUSB3PIPECTL(0), reg);

	mdelay(100);

	reg = dwc3_readl(dwc->regs, DWC3_GUSB2PHYCFG(0));

	/*
	 * Above 1.94a, it is recommended to set DWC3_GUSB2PHYCFG_SUSPHY to
	 * '0' during coreConsultant configuration. So default value will
	 * be '0' when the core is reset. Application needs to set it to
	 * '1' after the core initialization is completed.
	 */
	if (dwc->revision > DWC3_REVISION_194A)
		reg |= DWC3_GUSB2PHYCFG_SUSPHY;

	if (dwc->dis_u2_susphy_quirk)
		reg &= ~DWC3_GUSB2PHYCFG_SUSPHY;

	dwc3_writel(dwc->regs, DWC3_GUSB2PHYCFG(0), reg);

	mdelay(100);
}

/**
 * dwc3_core_init - Low-level initialization of DWC3 Core
 * @dwc: Pointer to our controller context structure
 *
 * Returns 0 on success otherwise negative errno.
 */
static int dwc3_core_init(struct dwc3 *dwc)
{
	unsigned long		timeout;
	u32			hwparams4 = dwc->hwparams.hwparams4;
	u32			reg;
	int			ret;

	reg = dwc3_readl(dwc->regs, DWC3_GSNPSID);
	/* This should read as U3 followed by revision number */
	if ((reg & DWC3_GSNPSID_MASK) != 0x55330000) {
		dev_err(dwc->dev, "this is not a DesignWare USB3 DRD Core\n");
		ret = -ENODEV;
		goto err0;
	}
	dwc->revision = reg;

	/* Handle USB2.0-only core configuration */
	if (DWC3_GHWPARAMS3_SSPHY_IFC(dwc->hwparams.hwparams3) ==
			DWC3_GHWPARAMS3_SSPHY_IFC_DIS) {
		if (dwc->maximum_speed == USB_SPEED_SUPER)
			dwc->maximum_speed = USB_SPEED_HIGH;
	}

	/* issue device SoftReset too */
	timeout = 5000;
	dwc3_writel(dwc->regs, DWC3_DCTL, DWC3_DCTL_CSFTRST);
	while (timeout--) {
		reg = dwc3_readl(dwc->regs, DWC3_DCTL);
		if (!(reg & DWC3_DCTL_CSFTRST))
			break;
	};

	if (!timeout) {
		dev_err(dwc->dev, "Reset Timed Out\n");
		ret = -ETIMEDOUT;
		goto err0;
	}

	ret = dwc3_core_soft_reset(dwc);
	if (ret)
		goto err0;

	reg = dwc3_readl(dwc->regs, DWC3_GCTL);
	reg &= ~DWC3_GCTL_SCALEDOWN_MASK;

	switch (DWC3_GHWPARAMS1_EN_PWROPT(dwc->hwparams.hwparams1)) {
	case DWC3_GHWPARAMS1_EN_PWROPT_CLK:
		/**
		 * WORKAROUND: DWC3 revisions between 2.10a and 2.50a have an
		 * issue which would cause xHCI compliance tests to fail.
		 *
		 * Because of that we cannot enable clock gating on such
		 * configurations.
		 *
		 * Refers to:
		 *
		 * STAR#9000588375: Clock Gating, SOF Issues when ref_clk-Based
		 * SOF/ITP Mode Used
		 */
		if ((dwc->dr_mode == USB_DR_MODE_HOST ||
				dwc->dr_mode == USB_DR_MODE_OTG) &&
				(dwc->revision >= DWC3_REVISION_210A &&
				dwc->revision <= DWC3_REVISION_250A))
			reg |= DWC3_GCTL_DSBLCLKGTNG | DWC3_GCTL_SOFITPSYNC;
		else
			reg &= ~DWC3_GCTL_DSBLCLKGTNG;
		break;
	case DWC3_GHWPARAMS1_EN_PWROPT_HIB:
		/* enable hibernation here */
		dwc->nr_scratch = DWC3_GHWPARAMS4_HIBER_SCRATCHBUFS(hwparams4);

		/*
		 * REVISIT Enabling this bit so that host-mode hibernation
		 * will work. Device-mode hibernation is not yet implemented.
		 */
		reg |= DWC3_GCTL_GBLHIBERNATIONEN;
		break;
	default:
		dev_dbg(dwc->dev, "No power optimization available\n");
	}

	/* check if current dwc3 is on simulation board */
	if (dwc->hwparams.hwparams6 & DWC3_GHWPARAMS6_EN_FPGA) {
		dev_dbg(dwc->dev, "it is on FPGA board\n");
		dwc->is_fpga = true;
	}

	if(dwc->disable_scramble_quirk && !dwc->is_fpga)
		WARN(true,
		     "disable_scramble cannot be used on non-FPGA builds\n");

	if (dwc->disable_scramble_quirk && dwc->is_fpga)
		reg |= DWC3_GCTL_DISSCRAMBLE;
	else
		reg &= ~DWC3_GCTL_DISSCRAMBLE;

	if (dwc->u2exit_lfps_quirk)
		reg |= DWC3_GCTL_U2EXIT_LFPS;

	/*
	 * WORKAROUND: DWC3 revisions <1.90a have a bug
	 * where the device can fail to connect at SuperSpeed
	 * and falls back to high-speed mode which causes
	 * the device to enter a Connect/Disconnect loop
	 */
	if (dwc->revision < DWC3_REVISION_190A)
		reg |= DWC3_GCTL_U2RSTECN;

	dwc3_core_num_eps(dwc);

	dwc3_writel(dwc->regs, DWC3_GCTL, reg);

	dwc3_phy_setup(dwc);

	ret = dwc3_alloc_scratch_buffers(dwc);
	if (ret)
		goto err0;

	ret = dwc3_setup_scratch_buffers(dwc);
	if (ret)
		goto err1;

	return 0;

err1:
	dwc3_free_scratch_buffers(dwc);

err0:
	return ret;
}

static void dwc3_core_exit(struct dwc3 *dwc)
{
	dwc3_free_scratch_buffers(dwc);
}

static int dwc3_core_init_mode(struct dwc3 *dwc)
{
	int ret;

	switch (dwc->dr_mode) {
	case USB_DR_MODE_PERIPHERAL:
		dwc3_set_mode(dwc, DWC3_GCTL_PRTCAP_DEVICE);
		ret = dwc3_gadget_init(dwc);
		if (ret) {
			dev_err(dev, "failed to initialize gadget\n");
			return ret;
		}
		break;
	case USB_DR_MODE_HOST:
		dwc3_set_mode(dwc, DWC3_GCTL_PRTCAP_HOST);
		ret = dwc3_host_init(dwc);
		if (ret) {
			dev_err(dev, "failed to initialize host\n");
			return ret;
		}
		break;
	case USB_DR_MODE_OTG:
		dwc3_set_mode(dwc, DWC3_GCTL_PRTCAP_OTG);
		ret = dwc3_host_init(dwc);
		if (ret) {
			dev_err(dev, "failed to initialize host\n");
			return ret;
		}

		ret = dwc3_gadget_init(dwc);
		if (ret) {
			dev_err(dev, "failed to initialize gadget\n");
			return ret;
		}
		break;
	default:
		dev_err(dev, "Unsupported mode of operation %d\n", dwc->dr_mode);
		return -EINVAL;
	}

	return 0;
}

static void dwc3_core_exit_mode(struct dwc3 *dwc)
{
	switch (dwc->dr_mode) {
	case USB_DR_MODE_PERIPHERAL:
		dwc3_gadget_exit(dwc);
		break;
	case USB_DR_MODE_HOST:
		dwc3_host_exit(dwc);
		break;
	case USB_DR_MODE_OTG:
		dwc3_host_exit(dwc);
		dwc3_gadget_exit(dwc);
		break;
	default:
		/* do nothing */
		break;
	}
}

#define DWC3_ALIGN_MASK		(16 - 1)

/**
 * dwc3_uboot_init - dwc3 core uboot initialization code
 * @dwc3_dev: struct dwc3_device containing initialization data
 *
 * Entry point for dwc3 driver (equivalent to dwc3_probe in linux
 * kernel driver). Pointer to dwc3_device should be passed containing
 * base address and other initialization data. Returns '0' on success and
 * a negative value on failure.
 *
 * Generally called from board_usb_init() implemented in board file.
 */
int dwc3_uboot_init(struct dwc3_device *dwc3_dev)
{
	struct dwc3		*dwc;
	struct device		*dev = NULL;
	u8			lpm_nyet_threshold;
	u8			tx_de_emphasis;
	u8			hird_threshold;

	int			ret;

	void			*mem;

	mem = devm_kzalloc((struct udevice *)dev,
			   sizeof(*dwc) + DWC3_ALIGN_MASK, GFP_KERNEL);
	if (!mem)
		return -ENOMEM;

	dwc = PTR_ALIGN(mem, DWC3_ALIGN_MASK + 1);
	dwc->mem = mem;

	dwc->regs = (void *)(uintptr_t)(dwc3_dev->base +
					DWC3_GLOBALS_REGS_START);

	/* default to highest possible threshold */
	lpm_nyet_threshold = 0xff;

	/* default to -3.5dB de-emphasis */
	tx_de_emphasis = 1;

	/*
	 * default to assert utmi_sleep_n and use maximum allowed HIRD
	 * threshold value of 0b1100
	 */
	hird_threshold = 12;

	dwc->maximum_speed = dwc3_dev->maximum_speed;
	dwc->has_lpm_erratum = dwc3_dev->has_lpm_erratum;
	if (dwc3_dev->lpm_nyet_threshold)
		lpm_nyet_threshold = dwc3_dev->lpm_nyet_threshold;
	dwc->is_utmi_l1_suspend = dwc3_dev->is_utmi_l1_suspend;
	if (dwc3_dev->hird_threshold)
		hird_threshold = dwc3_dev->hird_threshold;

	dwc->needs_fifo_resize = dwc3_dev->tx_fifo_resize;
	dwc->dr_mode = dwc3_dev->dr_mode;

	dwc->disable_scramble_quirk = dwc3_dev->disable_scramble_quirk;
	dwc->u2exit_lfps_quirk = dwc3_dev->u2exit_lfps_quirk;
	dwc->u2ss_inp3_quirk = dwc3_dev->u2ss_inp3_quirk;
	dwc->req_p1p2p3_quirk = dwc3_dev->req_p1p2p3_quirk;
	dwc->del_p1p2p3_quirk = dwc3_dev->del_p1p2p3_quirk;
	dwc->del_phy_power_chg_quirk = dwc3_dev->del_phy_power_chg_quirk;
	dwc->lfps_filter_quirk = dwc3_dev->lfps_filter_quirk;
	dwc->rx_detect_poll_quirk = dwc3_dev->rx_detect_poll_quirk;
	dwc->dis_u3_susphy_quirk = dwc3_dev->dis_u3_susphy_quirk;
	dwc->dis_u2_susphy_quirk = dwc3_dev->dis_u2_susphy_quirk;

	dwc->tx_de_emphasis_quirk = dwc3_dev->tx_de_emphasis_quirk;
	if (dwc3_dev->tx_de_emphasis)
		tx_de_emphasis = dwc3_dev->tx_de_emphasis;

	/* default to superspeed if no maximum_speed passed */
	if (dwc->maximum_speed == USB_SPEED_UNKNOWN)
		dwc->maximum_speed = USB_SPEED_SUPER;

	dwc->lpm_nyet_threshold = lpm_nyet_threshold;
	dwc->tx_de_emphasis = tx_de_emphasis;

	dwc->hird_threshold = hird_threshold
		| (dwc->is_utmi_l1_suspend << 4);

	dwc->index = dwc3_dev->index;

	dwc3_cache_hwparams(dwc);

	ret = dwc3_alloc_event_buffers(dwc, DWC3_EVENT_BUFFERS_SIZE);
	if (ret) {
		dev_err(dwc->dev, "failed to allocate event buffers\n");
		return -ENOMEM;
	}

	if (IS_ENABLED(CONFIG_USB_DWC3_HOST))
		dwc->dr_mode = USB_DR_MODE_HOST;
	else if (IS_ENABLED(CONFIG_USB_DWC3_GADGET))
		dwc->dr_mode = USB_DR_MODE_PERIPHERAL;

	if (dwc->dr_mode == USB_DR_MODE_UNKNOWN)
		dwc->dr_mode = USB_DR_MODE_OTG;

	ret = dwc3_core_init(dwc);
	if (ret) {
		dev_err(dev, "failed to initialize core\n");
		goto err0;
	}

	ret = dwc3_event_buffers_setup(dwc);
	if (ret) {
		dev_err(dwc->dev, "failed to setup event buffers\n");
		goto err1;
	}

	ret = dwc3_core_init_mode(dwc);
	if (ret)
		goto err2;

	list_add_tail(&dwc->list, &dwc3_list);

	return 0;

err2:
	dwc3_event_buffers_cleanup(dwc);

err1:
	dwc3_core_exit(dwc);

err0:
	dwc3_free_event_buffers(dwc);

	return ret;
}

/**
 * dwc3_uboot_exit - dwc3 core uboot cleanup code
 * @index: index of this controller
 *
 * Performs cleanup of memory allocated in dwc3_uboot_init and other misc
 * cleanups (equivalent to dwc3_remove in linux). index of _this_ controller
 * should be passed and should match with the index passed in
 * dwc3_device during init.
 *
 * Generally called from board file.
 */
void dwc3_uboot_exit(int index)
{
	struct dwc3 *dwc;

	list_for_each_entry(dwc, &dwc3_list, list) {
		if (dwc->index != index)
			continue;

		dwc3_core_exit_mode(dwc);
		dwc3_event_buffers_cleanup(dwc);
		dwc3_free_event_buffers(dwc);
		dwc3_core_exit(dwc);
		list_del(&dwc->list);
		kfree(dwc->mem);
		break;
	}
}

/**
 * dwc3_uboot_handle_interrupt - handle dwc3 core interrupt
 * @index: index of this controller
 *
 * Invokes dwc3 gadget interrupts.
 *
 * Generally called from board file.
 */
void dwc3_uboot_handle_interrupt(int index)
{
	struct dwc3 *dwc = NULL;

	list_for_each_entry(dwc, &dwc3_list, list) {
		if (dwc->index != index)
			continue;

		dwc3_gadget_uboot_handle_interrupt(dwc);
		break;
	}
}

MODULE_ALIAS("platform:dwc3");
MODULE_AUTHOR("Felipe Balbi <balbi@ti.com>");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("DesignWare USB3 DRD Controller Driver");

#if CONFIG_IS_ENABLED(PHY) && CONFIG_IS_ENABLED(DM_USB)
int dwc3_setup_phy(struct udevice *dev, struct phy **array, int *num_phys)
{
	int i, ret, count;
	struct phy *usb_phys;

	/* Return if no phy declared */
	if (!dev_read_prop(dev, "phys", NULL))
		return 0;
	count = dev_count_phandle_with_args(dev, "phys", "#phy-cells");
	if (count <= 0)
		return count;

	usb_phys = devm_kcalloc(dev, count, sizeof(struct phy),
				GFP_KERNEL);
	if (!usb_phys)
		return -ENOMEM;

	for (i = 0; i < count; i++) {
		ret = generic_phy_get_by_index(dev, i, &usb_phys[i]);
		if (ret && ret != -ENOENT) {
			pr_err("Failed to get USB PHY%d for %s\n",
			       i, dev->name);
			return ret;
		}
	}

	for (i = 0; i < count; i++) {
		ret = generic_phy_init(&usb_phys[i]);
		if (ret) {
			pr_err("Can't init USB PHY%d for %s\n",
			       i, dev->name);
			goto phys_init_err;
		}
	}

	for (i = 0; i < count; i++) {
		ret = generic_phy_power_on(&usb_phys[i]);
		if (ret) {
			pr_err("Can't power USB PHY%d for %s\n",
			       i, dev->name);
			goto phys_poweron_err;
		}
	}

	*array = usb_phys;
	*num_phys =  count;
	return 0;

phys_poweron_err:
	for (i = count - 1; i >= 0; i--)
		generic_phy_power_off(&usb_phys[i]);

	for (i = 0; i < count; i++)
		generic_phy_exit(&usb_phys[i]);

	return ret;

phys_init_err:
	for (; i >= 0; i--)
		generic_phy_exit(&usb_phys[i]);

	return ret;
}

int dwc3_shutdown_phy(struct udevice *dev, struct phy *usb_phys, int num_phys)
{
	int i, ret;

	for (i = 0; i < num_phys; i++) {
		if (!generic_phy_valid(&usb_phys[i]))
			continue;

		ret = generic_phy_power_off(&usb_phys[i]);
		ret |= generic_phy_exit(&usb_phys[i]);
		if (ret) {
			pr_err("Can't shutdown USB PHY%d for %s\n",
			       i, dev->name);
		}
	}

	return 0;
}
#endif

#if CONFIG_IS_ENABLED(DM_USB_GADGET)
int dwc3_init(struct dwc3 *dwc)
{
	int ret;

	dwc3_cache_hwparams(dwc);

	ret = dwc3_alloc_event_buffers(dwc, DWC3_EVENT_BUFFERS_SIZE);
	if (ret) {
		dev_err(dwc->dev, "failed to allocate event buffers\n");
		return -ENOMEM;
	}

	ret = dwc3_core_init(dwc);
	if (ret) {
		dev_err(dev, "failed to initialize core\n");
		goto core_fail;
	}

	ret = dwc3_event_buffers_setup(dwc);
	if (ret) {
		dev_err(dwc->dev, "failed to setup event buffers\n");
		goto event_fail;
	}

	ret = dwc3_core_init_mode(dwc);
	if (ret)
		goto mode_fail;

	return 0;

mode_fail:
	dwc3_event_buffers_cleanup(dwc);

event_fail:
	dwc3_core_exit(dwc);

core_fail:
	dwc3_free_event_buffers(dwc);

	return ret;
}

void dwc3_remove(struct dwc3 *dwc)
{
	dwc3_core_exit_mode(dwc);
	dwc3_event_buffers_cleanup(dwc);
	dwc3_free_event_buffers(dwc);
	dwc3_core_exit(dwc);
	kfree(dwc->mem);
}
#endif
