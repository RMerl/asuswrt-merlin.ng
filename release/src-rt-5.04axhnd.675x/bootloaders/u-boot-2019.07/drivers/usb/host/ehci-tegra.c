// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2011 The Chromium OS Authors.
 * Copyright (c) 2009-2015 NVIDIA Corporation
 * Copyright (c) 2013 Lucas Stach
 */

#include <common.h>
#include <dm.h>
#include <linux/errno.h>
#include <asm/io.h>
#include <asm-generic/gpio.h>
#include <asm/arch/clock.h>
#include <asm/arch-tegra/usb.h>
#include <asm/arch-tegra/clk_rst.h>
#include <usb.h>
#include <usb/ulpi.h>
#include <linux/libfdt.h>

#include "ehci.h"

#define USB1_ADDR_MASK	0xFFFF0000

#define HOSTPC1_DEVLC	0x84
#define HOSTPC1_PSPD(x)		(((x) >> 25) & 0x3)

#ifdef CONFIG_USB_ULPI
	#ifndef CONFIG_USB_ULPI_VIEWPORT
	#error	"To use CONFIG_USB_ULPI on Tegra Boards you have to also \
		define CONFIG_USB_ULPI_VIEWPORT"
	#endif
#endif

/* Parameters we need for USB */
enum {
	PARAM_DIVN,                     /* PLL FEEDBACK DIVIDer */
	PARAM_DIVM,                     /* PLL INPUT DIVIDER */
	PARAM_DIVP,                     /* POST DIVIDER (2^N) */
	PARAM_CPCON,                    /* BASE PLLC CHARGE Pump setup ctrl */
	PARAM_LFCON,                    /* BASE PLLC LOOP FILter setup ctrl */
	PARAM_ENABLE_DELAY_COUNT,       /* PLL-U Enable Delay Count */
	PARAM_STABLE_COUNT,             /* PLL-U STABLE count */
	PARAM_ACTIVE_DELAY_COUNT,       /* PLL-U Active delay count */
	PARAM_XTAL_FREQ_COUNT,          /* PLL-U XTAL frequency count */
	PARAM_DEBOUNCE_A_TIME,          /* 10MS DELAY for BIAS_DEBOUNCE_A */
	PARAM_BIAS_TIME,                /* 20US DELAY AFter bias cell op */

	PARAM_COUNT
};

/* Possible port types (dual role mode) */
enum dr_mode {
	DR_MODE_NONE = 0,
	DR_MODE_HOST,		/* supports host operation */
	DR_MODE_DEVICE,		/* supports device operation */
	DR_MODE_OTG,		/* supports both */
};

enum usb_ctlr_type {
	USB_CTLR_T20,
	USB_CTLR_T30,
	USB_CTLR_T114,
	USB_CTLR_T210,

	USB_CTRL_COUNT,
};

/* Information about a USB port */
struct fdt_usb {
	struct ehci_ctrl ehci;
	struct usb_ctlr *reg;	/* address of registers in physical memory */
	unsigned utmi:1;	/* 1 if port has external tranceiver, else 0 */
	unsigned ulpi:1;	/* 1 if port has external ULPI transceiver */
	unsigned enabled:1;	/* 1 to enable, 0 to disable */
	unsigned has_legacy_mode:1; /* 1 if this port has legacy mode */
	enum usb_ctlr_type type;
	enum usb_init_type init_type;
	enum dr_mode dr_mode;	/* dual role mode */
	enum periph_id periph_id;/* peripheral id */
	struct gpio_desc vbus_gpio;	/* GPIO for vbus enable */
	struct gpio_desc phy_reset_gpio; /* GPIO to reset ULPI phy */
};

/*
 * This table has USB timing parameters for each Oscillator frequency we
 * support. There are four sets of values:
 *
 * 1. PLLU configuration information (reference clock is osc/clk_m and
 * PLLU-FOs are fixed at 12MHz/60MHz/480MHz).
 *
 *  Reference frequency     13.0MHz      19.2MHz      12.0MHz      26.0MHz
 *  ----------------------------------------------------------------------
 *      DIVN                960 (0x3c0)  200 (0c8)    960 (3c0h)   960 (3c0)
 *      DIVM                13 (0d)      4 (04)       12 (0c)      26 (1a)
 * Filter frequency (MHz)   1            4.8          6            2
 * CPCON                    1100b        0011b        1100b        1100b
 * LFCON0                   0            0            0            0
 *
 * 2. PLL CONFIGURATION & PARAMETERS for different clock generators:
 *
 * Reference frequency     13.0MHz         19.2MHz         12.0MHz     26.0MHz
 * ---------------------------------------------------------------------------
 * PLLU_ENABLE_DLY_COUNT   02 (0x02)       03 (03)         02 (02)     04 (04)
 * PLLU_STABLE_COUNT       51 (33)         75 (4B)         47 (2F)    102 (66)
 * PLL_ACTIVE_DLY_COUNT    05 (05)         06 (06)         04 (04)     09 (09)
 * XTAL_FREQ_COUNT        127 (7F)        187 (BB)        118 (76)    254 (FE)
 *
 * 3. Debounce values IdDig, Avalid, Bvalid, VbusValid, VbusWakeUp, and
 * SessEnd. Each of these signals have their own debouncer and for each of
 * those one out of two debouncing times can be chosen (BIAS_DEBOUNCE_A or
 * BIAS_DEBOUNCE_B).
 *
 * The values of DEBOUNCE_A and DEBOUNCE_B are calculated as follows:
 *    0xffff -> No debouncing at all
 *    <n> ms = <n> *1000 / (1/19.2MHz) / 4
 *
 * So to program a 1 ms debounce for BIAS_DEBOUNCE_A, we have:
 * BIAS_DEBOUNCE_A[15:0] = 1000 * 19.2 / 4  = 4800 = 0x12c0
 *
 * We need to use only DebounceA for BOOTROM. We don't need the DebounceB
 * values, so we can keep those to default.
 *
 * 4. The 20 microsecond delay after bias cell operation.
 */
static const unsigned T20_usb_pll[CLOCK_OSC_FREQ_COUNT][PARAM_COUNT] = {
	/* DivN, DivM, DivP, CPCON, LFCON, Delays             Debounce, Bias */
	{ 0x3C0, 0x0D, 0x00, 0xC,   0,  0x02, 0x33, 0x05, 0x7F, 0x7EF4, 5 },
	{ 0x0C8, 0x04, 0x00, 0x3,   0,  0x03, 0x4B, 0x06, 0xBB, 0xBB80, 7 },
	{ 0x3C0, 0x0C, 0x00, 0xC,   0,  0x02, 0x2F, 0x04, 0x76, 0x7530, 5 },
	{ 0x3C0, 0x1A, 0x00, 0xC,   0,  0x04, 0x66, 0x09, 0xFE, 0xFDE8, 9 },
	{ 0x000, 0x00, 0x00, 0x0,   0,  0x00, 0x00, 0x00, 0x00, 0x0000, 0 },
	{ 0x000, 0x00, 0x00, 0x0,   0,  0x00, 0x00, 0x00, 0x00, 0x0000, 0 }
};

static const unsigned T30_usb_pll[CLOCK_OSC_FREQ_COUNT][PARAM_COUNT] = {
	/* DivN, DivM, DivP, CPCON, LFCON, Delays             Debounce, Bias */
	{ 0x3C0, 0x0D, 0x00, 0xC,   1,  0x02, 0x33, 0x09, 0x7F, 0x7EF4, 5 },
	{ 0x0C8, 0x04, 0x00, 0x3,   0,  0x03, 0x4B, 0x0C, 0xBB, 0xBB80, 7 },
	{ 0x3C0, 0x0C, 0x00, 0xC,   1,  0x02, 0x2F, 0x08, 0x76, 0x7530, 5 },
	{ 0x3C0, 0x1A, 0x00, 0xC,   1,  0x04, 0x66, 0x09, 0xFE, 0xFDE8, 9 },
	{ 0x000, 0x00, 0x00, 0x0,   0,  0x00, 0x00, 0x00, 0x00, 0x0000, 0 },
	{ 0x000, 0x00, 0x00, 0x0,   0,  0x00, 0x00, 0x00, 0x00, 0x0000, 0 }
};

static const unsigned T114_usb_pll[CLOCK_OSC_FREQ_COUNT][PARAM_COUNT] = {
	/* DivN, DivM, DivP, CPCON, LFCON, Delays             Debounce, Bias */
	{ 0x3C0, 0x0D, 0x00, 0xC,   2,  0x02, 0x33, 0x09, 0x7F, 0x7EF4, 6 },
	{ 0x0C8, 0x04, 0x00, 0x3,   2,  0x03, 0x4B, 0x0C, 0xBB, 0xBB80, 8 },
	{ 0x3C0, 0x0C, 0x00, 0xC,   2,  0x02, 0x2F, 0x08, 0x76, 0x7530, 5 },
	{ 0x3C0, 0x1A, 0x00, 0xC,   2,  0x04, 0x66, 0x09, 0xFE, 0xFDE8, 11 },
	{ 0x000, 0x00, 0x00, 0x0,   0,  0x00, 0x00, 0x00, 0x00, 0x0000, 0 },
	{ 0x000, 0x00, 0x00, 0x0,   0,  0x00, 0x00, 0x00, 0x00, 0x0000, 0 }
};

/* NOTE: 13/26MHz settings are N/A for T210, so dupe 12MHz settings for now */
static const unsigned T210_usb_pll[CLOCK_OSC_FREQ_COUNT][PARAM_COUNT] = {
	/* DivN, DivM, DivP, KCP,   KVCO,  Delays              Debounce, Bias */
	{ 0x028, 0x01, 0x01, 0x0,   0,  0x02, 0x2F, 0x08, 0x76,  32500,  5 },
	{ 0x019, 0x01, 0x01, 0x0,   0,  0x03, 0x4B, 0x0C, 0xBB,  48000,  8 },
	{ 0x028, 0x01, 0x01, 0x0,   0,  0x02, 0x2F, 0x08, 0x76,  30000,  5 },
	{ 0x028, 0x01, 0x01, 0x0,   0,  0x02, 0x2F, 0x08, 0x76,  65000,  5 },
	{ 0x019, 0x02, 0x01, 0x0,   0,  0x05, 0x96, 0x18, 0x177, 96000, 15 },
	{ 0x028, 0x04, 0x01, 0x0,   0,  0x04, 0x66, 0x09, 0xFE, 120000, 20 }
};

/* UTMIP Idle Wait Delay */
static const u8 utmip_idle_wait_delay = 17;

/* UTMIP Elastic limit */
static const u8 utmip_elastic_limit = 16;

/* UTMIP High Speed Sync Start Delay */
static const u8 utmip_hs_sync_start_delay = 9;

struct fdt_usb_controller {
	/* flag to determine whether controller supports hostpc register */
	u32 has_hostpc:1;
	const unsigned *pll_parameter;
};

static struct fdt_usb_controller fdt_usb_controllers[USB_CTRL_COUNT] = {
	{
		.has_hostpc	= 0,
		.pll_parameter	= (const unsigned *)T20_usb_pll,
	},
	{
		.has_hostpc	= 1,
		.pll_parameter	= (const unsigned *)T30_usb_pll,
	},
	{
		.has_hostpc	= 1,
		.pll_parameter	= (const unsigned *)T114_usb_pll,
	},
	{
		.has_hostpc	= 1,
		.pll_parameter	= (const unsigned *)T210_usb_pll,
	},
};

/*
 * A known hardware issue where Connect Status Change bit of PORTSC register
 * of USB1 controller will be set after Port Reset.
 * We have to clear it in order for later device enumeration to proceed.
 */
static void tegra_ehci_powerup_fixup(struct ehci_ctrl *ctrl,
				     uint32_t *status_reg, uint32_t *reg)
{
	struct fdt_usb *config = ctrl->priv;
	struct fdt_usb_controller *controller;

	controller = &fdt_usb_controllers[config->type];
	mdelay(50);
	/* This is to avoid PORT_ENABLE bit to be cleared in "ehci-hcd.c". */
	if (controller->has_hostpc)
		*reg |= EHCI_PS_PE;

	if (!config->has_legacy_mode)
		return;
	/* For EHCI_PS_CSC to be cleared in ehci_hcd.c */
	if (ehci_readl(status_reg) & EHCI_PS_CSC)
		*reg |= EHCI_PS_CSC;
}

static void tegra_ehci_set_usbmode(struct ehci_ctrl *ctrl)
{
	struct fdt_usb *config = ctrl->priv;
	struct usb_ctlr *usbctlr;
	uint32_t tmp;

	usbctlr = config->reg;

	tmp = ehci_readl(&usbctlr->usb_mode);
	tmp |= USBMODE_CM_HC;
	ehci_writel(&usbctlr->usb_mode, tmp);
}

static int tegra_ehci_get_port_speed(struct ehci_ctrl *ctrl, uint32_t reg)
{
	struct fdt_usb *config = ctrl->priv;
	struct fdt_usb_controller *controller;
	uint32_t tmp;
	uint32_t *reg_ptr;

	controller = &fdt_usb_controllers[config->type];
	if (controller->has_hostpc) {
		reg_ptr = (uint32_t *)((u8 *)&ctrl->hcor->or_usbcmd +
				HOSTPC1_DEVLC);
		tmp = ehci_readl(reg_ptr);
		return HOSTPC1_PSPD(tmp);
	} else
		return PORTSC_PSPD(reg);
}

/* Set up VBUS for host/device mode */
static void set_up_vbus(struct fdt_usb *config, enum usb_init_type init)
{
	/*
	 * If we are an OTG port initializing in host mode,
	 * check if remote host is driving VBus and bail out in this case.
	 */
	if (init == USB_INIT_HOST &&
	    config->dr_mode == DR_MODE_OTG &&
	    (readl(&config->reg->phy_vbus_sensors) & VBUS_VLD_STS)) {
		printf("tegrausb: VBUS input active; not enabling as host\n");
		return;
	}

	if (dm_gpio_is_valid(&config->vbus_gpio)) {
		int vbus_value;

		vbus_value = (init == USB_INIT_HOST);
		dm_gpio_set_value(&config->vbus_gpio, vbus_value);

		debug("set_up_vbus: GPIO %d %d\n",
		      gpio_get_number(&config->vbus_gpio), vbus_value);
	}
}

static void usbf_reset_controller(struct fdt_usb *config,
				  struct usb_ctlr *usbctlr)
{
	/* Reset the USB controller with 2us delay */
	reset_periph(config->periph_id, 2);

	/*
	 * Set USB1_NO_LEGACY_MODE to 1, Registers are accessible under
	 * base address
	 */
	if (config->has_legacy_mode)
		setbits_le32(&usbctlr->usb1_legacy_ctrl, USB1_NO_LEGACY_MODE);

	/* Put UTMIP1/3 in reset */
	setbits_le32(&usbctlr->susp_ctrl, UTMIP_RESET);

	/* Enable the UTMIP PHY */
	if (config->utmi)
		setbits_le32(&usbctlr->susp_ctrl, UTMIP_PHY_ENB);
}

static const unsigned *get_pll_timing(struct fdt_usb_controller *controller)
{
	const unsigned *timing;

	timing = controller->pll_parameter +
		clock_get_osc_freq() * PARAM_COUNT;

	return timing;
}

/* select the PHY to use with a USB controller */
static void init_phy_mux(struct fdt_usb *config, uint pts,
			 enum usb_init_type init)
{
	struct usb_ctlr *usbctlr = config->reg;

#if defined(CONFIG_TEGRA20)
	if (config->periph_id == PERIPH_ID_USBD) {
		clrsetbits_le32(&usbctlr->port_sc1, PTS1_MASK,
				pts << PTS1_SHIFT);
		clrbits_le32(&usbctlr->port_sc1, STS1);
	} else {
		clrsetbits_le32(&usbctlr->port_sc1, PTS_MASK,
				pts << PTS_SHIFT);
		clrbits_le32(&usbctlr->port_sc1, STS);
	}
#else
	/* Set to Host mode (if applicable) after Controller Reset was done */
	clrsetbits_le32(&usbctlr->usb_mode, USBMODE_CM_HC,
			(init == USB_INIT_HOST) ? USBMODE_CM_HC : 0);
	/*
	 * Select PHY interface after setting host mode.
	 * For device mode, the ordering requirement is not an issue, since
	 * only the first USB controller supports device mode, and that USB
	 * controller can only talk to a UTMI PHY, so the PHY selection is
	 * already made at reset time, so this write is a no-op.
	 */
	clrsetbits_le32(&usbctlr->hostpc1_devlc, PTS_MASK,
			pts << PTS_SHIFT);
	clrbits_le32(&usbctlr->hostpc1_devlc, STS);
#endif
}

/* set up the UTMI USB controller with the parameters provided */
static int init_utmi_usb_controller(struct fdt_usb *config,
				    enum usb_init_type init)
{
	struct fdt_usb_controller *controller;
	u32 b_sess_valid_mask, val;
	int loop_count;
	const unsigned *timing;
	struct usb_ctlr *usbctlr = config->reg;
	struct clk_rst_ctlr *clkrst;
	struct usb_ctlr *usb1ctlr;

	clock_enable(config->periph_id);

	/* Reset the usb controller */
	usbf_reset_controller(config, usbctlr);

	/* Stop crystal clock by setting UTMIP_PHY_XTAL_CLOCKEN low */
	clrbits_le32(&usbctlr->utmip_misc_cfg1, UTMIP_PHY_XTAL_CLOCKEN);

	/* Follow the crystal clock disable by >100ns delay */
	udelay(1);

	b_sess_valid_mask = (VBUS_B_SESS_VLD_SW_VALUE | VBUS_B_SESS_VLD_SW_EN);
	clrsetbits_le32(&usbctlr->phy_vbus_sensors, b_sess_valid_mask,
			(init == USB_INIT_DEVICE) ? b_sess_valid_mask : 0);

	/*
	 * To Use the A Session Valid for cable detection logic, VBUS_WAKEUP
	 * mux must be switched to actually use a_sess_vld threshold.
	 */
	if (config->dr_mode == DR_MODE_OTG &&
	    dm_gpio_is_valid(&config->vbus_gpio))
		clrsetbits_le32(&usbctlr->usb1_legacy_ctrl,
			VBUS_SENSE_CTL_MASK,
			VBUS_SENSE_CTL_A_SESS_VLD << VBUS_SENSE_CTL_SHIFT);

	controller = &fdt_usb_controllers[config->type];
	debug("controller=%p, type=%d\n", controller, config->type);

	/*
	 * PLL Delay CONFIGURATION settings. The following parameters control
	 * the bring up of the plls.
	 */
	timing = get_pll_timing(controller);

	if (!controller->has_hostpc) {
		val = readl(&usbctlr->utmip_misc_cfg1);
		clrsetbits_le32(&val, UTMIP_PLLU_STABLE_COUNT_MASK,
				timing[PARAM_STABLE_COUNT] <<
				UTMIP_PLLU_STABLE_COUNT_SHIFT);
		clrsetbits_le32(&val, UTMIP_PLL_ACTIVE_DLY_COUNT_MASK,
				timing[PARAM_ACTIVE_DELAY_COUNT] <<
				UTMIP_PLL_ACTIVE_DLY_COUNT_SHIFT);
		writel(val, &usbctlr->utmip_misc_cfg1);

		/* Set PLL enable delay count and crystal frequency count */
		val = readl(&usbctlr->utmip_pll_cfg1);
		clrsetbits_le32(&val, UTMIP_PLLU_ENABLE_DLY_COUNT_MASK,
				timing[PARAM_ENABLE_DELAY_COUNT] <<
				UTMIP_PLLU_ENABLE_DLY_COUNT_SHIFT);
		clrsetbits_le32(&val, UTMIP_XTAL_FREQ_COUNT_MASK,
				timing[PARAM_XTAL_FREQ_COUNT] <<
				UTMIP_XTAL_FREQ_COUNT_SHIFT);
		writel(val, &usbctlr->utmip_pll_cfg1);
	} else {
		clkrst = (struct clk_rst_ctlr *)NV_PA_CLK_RST_BASE;

		val = readl(&clkrst->crc_utmip_pll_cfg2);
		clrsetbits_le32(&val, UTMIP_PLLU_STABLE_COUNT_MASK,
				timing[PARAM_STABLE_COUNT] <<
				UTMIP_PLLU_STABLE_COUNT_SHIFT);
		clrsetbits_le32(&val, UTMIP_PLL_ACTIVE_DLY_COUNT_MASK,
				timing[PARAM_ACTIVE_DELAY_COUNT] <<
				UTMIP_PLL_ACTIVE_DLY_COUNT_SHIFT);
		writel(val, &clkrst->crc_utmip_pll_cfg2);

		/* Set PLL enable delay count and crystal frequency count */
		val = readl(&clkrst->crc_utmip_pll_cfg1);
		clrsetbits_le32(&val, UTMIP_PLLU_ENABLE_DLY_COUNT_MASK,
				timing[PARAM_ENABLE_DELAY_COUNT] <<
				UTMIP_PLLU_ENABLE_DLY_COUNT_SHIFT);
		clrsetbits_le32(&val, UTMIP_XTAL_FREQ_COUNT_MASK,
				timing[PARAM_XTAL_FREQ_COUNT] <<
				UTMIP_XTAL_FREQ_COUNT_SHIFT);
		writel(val, &clkrst->crc_utmip_pll_cfg1);

		/* Disable Power Down state for PLL */
		clrbits_le32(&clkrst->crc_utmip_pll_cfg1,
			     PLLU_POWERDOWN | PLL_ENABLE_POWERDOWN |
			     PLL_ACTIVE_POWERDOWN);

		/* Recommended PHY settings for EYE diagram */
		val = readl(&usbctlr->utmip_xcvr_cfg0);
		clrsetbits_le32(&val, UTMIP_XCVR_SETUP_MASK,
				0x4 << UTMIP_XCVR_SETUP_SHIFT);
		clrsetbits_le32(&val, UTMIP_XCVR_SETUP_MSB_MASK,
				0x3 << UTMIP_XCVR_SETUP_MSB_SHIFT);
		clrsetbits_le32(&val, UTMIP_XCVR_HSSLEW_MSB_MASK,
				0x8 << UTMIP_XCVR_HSSLEW_MSB_SHIFT);
		writel(val, &usbctlr->utmip_xcvr_cfg0);
		clrsetbits_le32(&usbctlr->utmip_xcvr_cfg1,
				UTMIP_XCVR_TERM_RANGE_ADJ_MASK,
				0x7 << UTMIP_XCVR_TERM_RANGE_ADJ_SHIFT);

		/* Some registers can be controlled from USB1 only. */
		if (config->periph_id != PERIPH_ID_USBD) {
			clock_enable(PERIPH_ID_USBD);
			/* Disable Reset if in Reset state */
			reset_set_enable(PERIPH_ID_USBD, 0);
		}
		usb1ctlr = (struct usb_ctlr *)
			((unsigned long)config->reg & USB1_ADDR_MASK);
		val = readl(&usb1ctlr->utmip_bias_cfg0);
		setbits_le32(&val, UTMIP_HSDISCON_LEVEL_MSB);
		clrsetbits_le32(&val, UTMIP_HSDISCON_LEVEL_MASK,
				0x1 << UTMIP_HSDISCON_LEVEL_SHIFT);
		clrsetbits_le32(&val, UTMIP_HSSQUELCH_LEVEL_MASK,
				0x2 << UTMIP_HSSQUELCH_LEVEL_SHIFT);
		writel(val, &usb1ctlr->utmip_bias_cfg0);

		/* Miscellaneous setting mentioned in Programming Guide */
		clrbits_le32(&usbctlr->utmip_misc_cfg0,
			     UTMIP_SUSPEND_EXIT_ON_EDGE);
	}

	/* Setting the tracking length time */
	clrsetbits_le32(&usbctlr->utmip_bias_cfg1,
		UTMIP_BIAS_PDTRK_COUNT_MASK,
		timing[PARAM_BIAS_TIME] << UTMIP_BIAS_PDTRK_COUNT_SHIFT);

	/* Program debounce time for VBUS to become valid */
	clrsetbits_le32(&usbctlr->utmip_debounce_cfg0,
		UTMIP_DEBOUNCE_CFG0_MASK,
		timing[PARAM_DEBOUNCE_A_TIME] << UTMIP_DEBOUNCE_CFG0_SHIFT);

	if (timing[PARAM_DEBOUNCE_A_TIME] > 0xFFFF) {
		clrsetbits_le32(&usbctlr->utmip_debounce_cfg0,
				UTMIP_DEBOUNCE_CFG0_MASK,
				(timing[PARAM_DEBOUNCE_A_TIME] >> 1)
				<< UTMIP_DEBOUNCE_CFG0_SHIFT);
		clrsetbits_le32(&usbctlr->utmip_bias_cfg1,
				UTMIP_BIAS_DEBOUNCE_TIMESCALE_MASK,
				1 << UTMIP_BIAS_DEBOUNCE_TIMESCALE_SHIFT);
	}

	setbits_le32(&usbctlr->utmip_tx_cfg0, UTMIP_FS_PREAMBLE_J);

	/* Disable battery charge enabling bit */
	setbits_le32(&usbctlr->utmip_bat_chrg_cfg0, UTMIP_PD_CHRG);

	clrbits_le32(&usbctlr->utmip_xcvr_cfg0, UTMIP_XCVR_LSBIAS_SE);
	setbits_le32(&usbctlr->utmip_spare_cfg0, FUSE_SETUP_SEL);

	/*
	 * Configure the UTMIP_IDLE_WAIT and UTMIP_ELASTIC_LIMIT
	 * Setting these fields, together with default values of the
	 * other fields, results in programming the registers below as
	 * follows:
	 *         UTMIP_HSRX_CFG0 = 0x9168c000
	 *         UTMIP_HSRX_CFG1 = 0x13
	 */

	/* Set PLL enable delay count and Crystal frequency count */
	val = readl(&usbctlr->utmip_hsrx_cfg0);
	clrsetbits_le32(&val, UTMIP_IDLE_WAIT_MASK,
		utmip_idle_wait_delay << UTMIP_IDLE_WAIT_SHIFT);
	clrsetbits_le32(&val, UTMIP_ELASTIC_LIMIT_MASK,
		utmip_elastic_limit << UTMIP_ELASTIC_LIMIT_SHIFT);
	writel(val, &usbctlr->utmip_hsrx_cfg0);

	/* Configure the UTMIP_HS_SYNC_START_DLY */
	clrsetbits_le32(&usbctlr->utmip_hsrx_cfg1,
		UTMIP_HS_SYNC_START_DLY_MASK,
		utmip_hs_sync_start_delay << UTMIP_HS_SYNC_START_DLY_SHIFT);

	/* Preceed the crystal clock disable by >100ns delay. */
	udelay(1);

	/* Resuscitate crystal clock by setting UTMIP_PHY_XTAL_CLOCKEN */
	setbits_le32(&usbctlr->utmip_misc_cfg1, UTMIP_PHY_XTAL_CLOCKEN);

	if (controller->has_hostpc) {
		if (config->periph_id == PERIPH_ID_USBD)
			clrbits_le32(&clkrst->crc_utmip_pll_cfg2,
				     UTMIP_FORCE_PD_SAMP_A_POWERDOWN);
		if (config->periph_id == PERIPH_ID_USB2)
			clrbits_le32(&clkrst->crc_utmip_pll_cfg2,
				     UTMIP_FORCE_PD_SAMP_B_POWERDOWN);
		if (config->periph_id == PERIPH_ID_USB3)
			clrbits_le32(&clkrst->crc_utmip_pll_cfg2,
				     UTMIP_FORCE_PD_SAMP_C_POWERDOWN);
	}
	/* Finished the per-controller init. */

	/* De-assert UTMIP_RESET to bring out of reset. */
	clrbits_le32(&usbctlr->susp_ctrl, UTMIP_RESET);

	/* Wait for the phy clock to become valid in 100 ms */
	for (loop_count = 100000; loop_count != 0; loop_count--) {
		if (readl(&usbctlr->susp_ctrl) & USB_PHY_CLK_VALID)
			break;
		udelay(1);
	}
	if (!loop_count)
		return -ETIMEDOUT;

	/* Disable ICUSB FS/LS transceiver */
	clrbits_le32(&usbctlr->icusb_ctrl, IC_ENB1);

	/* Select UTMI parallel interface */
	init_phy_mux(config, PTS_UTMI, init);

	/* Deassert power down state */
	clrbits_le32(&usbctlr->utmip_xcvr_cfg0, UTMIP_FORCE_PD_POWERDOWN |
		UTMIP_FORCE_PD2_POWERDOWN | UTMIP_FORCE_PDZI_POWERDOWN);
	clrbits_le32(&usbctlr->utmip_xcvr_cfg1, UTMIP_FORCE_PDDISC_POWERDOWN |
		UTMIP_FORCE_PDCHRP_POWERDOWN | UTMIP_FORCE_PDDR_POWERDOWN);

	if (controller->has_hostpc) {
		/*
		 * BIAS Pad Power Down is common among all 3 USB
		 * controllers and can be controlled from USB1 only.
		 */
		usb1ctlr = (struct usb_ctlr *)
			((unsigned long)config->reg & USB1_ADDR_MASK);
		clrbits_le32(&usb1ctlr->utmip_bias_cfg0, UTMIP_BIASPD);
		udelay(25);
		clrbits_le32(&usb1ctlr->utmip_bias_cfg1,
			     UTMIP_FORCE_PDTRK_POWERDOWN);
	}
	return 0;
}

#ifdef CONFIG_USB_ULPI
/* if board file does not set a ULPI reference frequency we default to 24MHz */
#ifndef CONFIG_ULPI_REF_CLK
#define CONFIG_ULPI_REF_CLK 24000000
#endif

/* set up the ULPI USB controller with the parameters provided */
static int init_ulpi_usb_controller(struct fdt_usb *config,
				    enum usb_init_type init)
{
	u32 val;
	int loop_count;
	struct ulpi_viewport ulpi_vp;
	struct usb_ctlr *usbctlr = config->reg;
	int ret;

	/* set up ULPI reference clock on pllp_out4 */
	clock_enable(PERIPH_ID_DEV2_OUT);
	clock_set_pllout(CLOCK_ID_PERIPH, PLL_OUT4, CONFIG_ULPI_REF_CLK);

	/* reset ULPI phy */
	if (dm_gpio_is_valid(&config->phy_reset_gpio)) {
		/*
		 * This GPIO is typically active-low, and marked as such in
		 * device tree. dm_gpio_set_value() takes this into account
		 * and inverts the value we pass here if required. In other
		 * words, this first call logically asserts the reset signal,
		 * which typically results in driving the physical GPIO low,
		 * and the second call logically de-asserts the reset signal,
		 * which typically results in driver the GPIO high.
		 */
		dm_gpio_set_value(&config->phy_reset_gpio, 1);
		mdelay(5);
		dm_gpio_set_value(&config->phy_reset_gpio, 0);
	}

	/* Reset the usb controller */
	clock_enable(config->periph_id);
	usbf_reset_controller(config, usbctlr);

	/* enable pinmux bypass */
	setbits_le32(&usbctlr->ulpi_timing_ctrl_0,
			ULPI_CLKOUT_PINMUX_BYP | ULPI_OUTPUT_PINMUX_BYP);

	/* Select ULPI parallel interface */
	init_phy_mux(config, PTS_ULPI, init);

	/* enable ULPI transceiver */
	setbits_le32(&usbctlr->susp_ctrl, ULPI_PHY_ENB);

	/* configure ULPI transceiver timings */
	val = 0;
	writel(val, &usbctlr->ulpi_timing_ctrl_1);

	val |= ULPI_DATA_TRIMMER_SEL(4);
	val |= ULPI_STPDIRNXT_TRIMMER_SEL(4);
	val |= ULPI_DIR_TRIMMER_SEL(4);
	writel(val, &usbctlr->ulpi_timing_ctrl_1);
	udelay(10);

	val |= ULPI_DATA_TRIMMER_LOAD;
	val |= ULPI_STPDIRNXT_TRIMMER_LOAD;
	val |= ULPI_DIR_TRIMMER_LOAD;
	writel(val, &usbctlr->ulpi_timing_ctrl_1);

	/* set up phy for host operation with external vbus supply */
	ulpi_vp.port_num = 0;
	ulpi_vp.viewport_addr = (u32)&usbctlr->ulpi_viewport;

	ret = ulpi_init(&ulpi_vp);
	if (ret) {
		printf("Tegra ULPI viewport init failed\n");
		return ret;
	}

	ulpi_set_vbus(&ulpi_vp, 1, 1);
	ulpi_set_vbus_indicator(&ulpi_vp, 1, 1, 0);

	/* enable wakeup events */
	setbits_le32(&usbctlr->port_sc1, WKCN | WKDS | WKOC);

	/* Enable and wait for the phy clock to become valid in 100 ms */
	setbits_le32(&usbctlr->susp_ctrl, USB_SUSP_CLR);
	for (loop_count = 100000; loop_count != 0; loop_count--) {
		if (readl(&usbctlr->susp_ctrl) & USB_PHY_CLK_VALID)
			break;
		udelay(1);
	}
	if (!loop_count)
		return -ETIMEDOUT;
	clrbits_le32(&usbctlr->susp_ctrl, USB_SUSP_CLR);

	return 0;
}
#else
static int init_ulpi_usb_controller(struct fdt_usb *config,
				    enum usb_init_type init)
{
	printf("No code to set up ULPI controller, please enable"
			"CONFIG_USB_ULPI and CONFIG_USB_ULPI_VIEWPORT");
	return -ENOSYS;
}
#endif

static void config_clock(const u32 timing[])
{
	debug("%s: DIVM = %d, DIVN = %d, DIVP = %d, cpcon/lfcon = %d/%d\n",
	      __func__, timing[PARAM_DIVM], timing[PARAM_DIVN],
	      timing[PARAM_DIVP], timing[PARAM_CPCON], timing[PARAM_LFCON]);

	clock_start_pll(CLOCK_ID_USB,
		timing[PARAM_DIVM], timing[PARAM_DIVN], timing[PARAM_DIVP],
		timing[PARAM_CPCON], timing[PARAM_LFCON]);
}

static int fdt_decode_usb(struct udevice *dev, struct fdt_usb *config)
{
	const char *phy, *mode;

	config->reg = (struct usb_ctlr *)dev_read_addr(dev);
	debug("reg=%p\n", config->reg);
	mode = dev_read_string(dev, "dr_mode");
	if (mode) {
		if (0 == strcmp(mode, "host"))
			config->dr_mode = DR_MODE_HOST;
		else if (0 == strcmp(mode, "peripheral"))
			config->dr_mode = DR_MODE_DEVICE;
		else if (0 == strcmp(mode, "otg"))
			config->dr_mode = DR_MODE_OTG;
		else {
			debug("%s: Cannot decode dr_mode '%s'\n", __func__,
			      mode);
			return -EINVAL;
		}
	} else {
		config->dr_mode = DR_MODE_HOST;
	}

	phy = dev_read_string(dev, "phy_type");
	config->utmi = phy && 0 == strcmp("utmi", phy);
	config->ulpi = phy && 0 == strcmp("ulpi", phy);
	config->has_legacy_mode = dev_read_bool(dev, "nvidia,has-legacy-mode");
	config->periph_id = clock_decode_periph_id(dev);
	if (config->periph_id == PERIPH_ID_NONE) {
		debug("%s: Missing/invalid peripheral ID\n", __func__);
		return -EINVAL;
	}
	gpio_request_by_name(dev, "nvidia,vbus-gpio", 0, &config->vbus_gpio,
			     GPIOD_IS_OUT);
	gpio_request_by_name(dev, "nvidia,phy-reset-gpio", 0,
			     &config->phy_reset_gpio, GPIOD_IS_OUT);
	debug("legacy_mode=%d, utmi=%d, ulpi=%d, periph_id=%d, vbus=%d, phy_reset=%d, dr_mode=%d, reg=%p\n",
	      config->has_legacy_mode, config->utmi, config->ulpi,
	      config->periph_id, gpio_get_number(&config->vbus_gpio),
	      gpio_get_number(&config->phy_reset_gpio), config->dr_mode,
	      config->reg);

	return 0;
}

int usb_common_init(struct fdt_usb *config, enum usb_init_type init)
{
	int ret = 0;

	switch (init) {
	case USB_INIT_HOST:
		switch (config->dr_mode) {
		case DR_MODE_HOST:
		case DR_MODE_OTG:
			break;
		default:
			printf("tegrausb: Invalid dr_mode %d for host mode\n",
			       config->dr_mode);
			return -1;
		}
		break;
	case USB_INIT_DEVICE:
		if (config->periph_id != PERIPH_ID_USBD) {
			printf("tegrausb: Device mode only supported on first USB controller\n");
			return -1;
		}
		if (!config->utmi) {
			printf("tegrausb: Device mode only supported with UTMI PHY\n");
			return -1;
		}
		switch (config->dr_mode) {
		case DR_MODE_DEVICE:
		case DR_MODE_OTG:
			break;
		default:
			printf("tegrausb: Invalid dr_mode %d for device mode\n",
			       config->dr_mode);
			return -1;
		}
		break;
	default:
		printf("tegrausb: Unknown USB_INIT_* %d\n", init);
		return -1;
	}

	debug("%d, %d\n", config->utmi, config->ulpi);
	if (config->utmi)
		ret = init_utmi_usb_controller(config, init);
	else if (config->ulpi)
		ret = init_ulpi_usb_controller(config, init);
	if (ret)
		return ret;

	set_up_vbus(config, init);

	config->init_type = init;

	return 0;
}

void usb_common_uninit(struct fdt_usb *priv)
{
	struct usb_ctlr *usbctlr;

	usbctlr = priv->reg;

	/* Stop controller */
	writel(0, &usbctlr->usb_cmd);
	udelay(1000);

	/* Initiate controller reset */
	writel(2, &usbctlr->usb_cmd);
	udelay(1000);
}

static const struct ehci_ops tegra_ehci_ops = {
	.set_usb_mode		= tegra_ehci_set_usbmode,
	.get_port_speed		= tegra_ehci_get_port_speed,
	.powerup_fixup		= tegra_ehci_powerup_fixup,
};

static int ehci_usb_ofdata_to_platdata(struct udevice *dev)
{
	struct fdt_usb *priv = dev_get_priv(dev);
	int ret;

	ret = fdt_decode_usb(dev, priv);
	if (ret)
		return ret;

	priv->type = dev_get_driver_data(dev);

	return 0;
}

static int ehci_usb_probe(struct udevice *dev)
{
	struct usb_platdata *plat = dev_get_platdata(dev);
	struct fdt_usb *priv = dev_get_priv(dev);
	struct ehci_hccr *hccr;
	struct ehci_hcor *hcor;
	static bool clk_done;
	int ret;

	ret = usb_common_init(priv, plat->init_type);
	if (ret)
		return ret;
	hccr = (struct ehci_hccr *)&priv->reg->cap_length;
	hcor = (struct ehci_hcor *)&priv->reg->usb_cmd;
	if (!clk_done) {
		config_clock(get_pll_timing(&fdt_usb_controllers[priv->type]));
		clk_done = true;
	}

	return ehci_register(dev, hccr, hcor, &tegra_ehci_ops, 0,
			     plat->init_type);
}

static const struct udevice_id ehci_usb_ids[] = {
	{ .compatible = "nvidia,tegra20-ehci", .data = USB_CTLR_T20 },
	{ .compatible = "nvidia,tegra30-ehci", .data = USB_CTLR_T30 },
	{ .compatible = "nvidia,tegra114-ehci", .data = USB_CTLR_T114 },
	{ .compatible = "nvidia,tegra210-ehci", .data = USB_CTLR_T210 },
	{ }
};

U_BOOT_DRIVER(usb_ehci) = {
	.name	= "ehci_tegra",
	.id	= UCLASS_USB,
	.of_match = ehci_usb_ids,
	.ofdata_to_platdata = ehci_usb_ofdata_to_platdata,
	.probe = ehci_usb_probe,
	.remove = ehci_deregister,
	.ops	= &ehci_usb_ops,
	.platdata_auto_alloc_size = sizeof(struct usb_platdata),
	.priv_auto_alloc_size = sizeof(struct fdt_usb),
	.flags	= DM_FLAG_ALLOC_PRIV_DMA,
};
