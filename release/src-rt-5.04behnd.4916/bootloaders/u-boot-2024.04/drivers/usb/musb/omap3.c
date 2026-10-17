// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2009 Wind River Systems, Inc.
 * Tom Rix <Tom.Rix@windriver.com>
 *
 * This is file is based on
 * repository git.gitorious.org/u-boot-omap3/mainline.git,
 * branch omap3-dev-usb, file drivers/usb/host/omap3530_usb.c
 *
 * This is the unique part of its copyright :
 *
 * ------------------------------------------------------------------------
 *
 * Copyright (c) 2009 Texas Instruments
 *
 * ------------------------------------------------------------------------
 */

#include <serial.h>
#include <asm/omap_common.h>
#include <twl4030.h>
#include <twl6030.h>
#include "omap3.h"

static int platform_needs_initialization = 1;

struct musb_config musb_cfg = {
	.regs		= (struct musb_regs *)MENTOR_USB0_BASE,
	.timeout	= OMAP3_USB_TIMEOUT,
	.musb_speed	= 0,
};

/*
 * OMAP3 USB OTG registers.
 */
struct omap3_otg_regs {
	u32	revision;
	u32	sysconfig;
	u32	sysstatus;
	u32	interfsel;
	u32	simenable;
	u32	forcestdby;
};

static struct omap3_otg_regs *otg;

#define OMAP3_OTG_SYSCONFIG_SMART_STANDBY_MODE		0x2000
#define OMAP3_OTG_SYSCONFIG_NO_STANDBY_MODE		0x1000
#define OMAP3_OTG_SYSCONFIG_SMART_IDLE_MODE		0x0010
#define OMAP3_OTG_SYSCONFIG_NO_IDLE_MODE		0x0008
#define OMAP3_OTG_SYSCONFIG_ENABLEWAKEUP		0x0004
#define OMAP3_OTG_SYSCONFIG_SOFTRESET			0x0002
#define OMAP3_OTG_SYSCONFIG_AUTOIDLE			0x0001

#define OMAP3_OTG_SYSSTATUS_RESETDONE			0x0001

/* OMAP4430 has an internal PHY, use it */
#ifdef CONFIG_OMAP44XX
#define OMAP3_OTG_INTERFSEL_OMAP			0x0000
#else
#define OMAP3_OTG_INTERFSEL_OMAP			0x0001
#endif

#define OMAP3_OTG_FORCESTDBY_STANDBY			0x0001


#ifdef DEBUG_MUSB_OMAP3
static void musb_db_otg_regs(void)
{
	u32 l;
	l = readl(&otg->revision);
	serial_printf("OTG_REVISION 0x%x\n", l);
	l = readl(&otg->sysconfig);
	serial_printf("OTG_SYSCONFIG 0x%x\n", l);
	l = readl(&otg->sysstatus);
	serial_printf("OTG_SYSSTATUS 0x%x\n", l);
	l = readl(&otg->interfsel);
	serial_printf("OTG_INTERFSEL 0x%x\n", l);
	l = readl(&otg->forcestdby);
	serial_printf("OTG_FORCESTDBY 0x%x\n", l);
}
#endif

int musb_platform_init(void)
{
	int ret = -1;

	if (platform_needs_initialization) {
		u32 stdby;

		/*
		 * OMAP3EVM uses ISP1504 phy and so
		 * twl4030 related init is not required.
		 */
#ifdef CONFIG_TWL4030_USB
		if (twl4030_usb_ulpi_init()) {
			serial_printf("ERROR: %s Could not initialize PHY\n",
				__PRETTY_FUNCTION__);
			goto end;
		}
#endif

#ifdef CONFIG_TWL6030_POWER
		twl6030_usb_device_settings();
#endif

		otg = (struct omap3_otg_regs *)OMAP3_OTG_BASE;

		/* Set OTG to always be on */
		writel(OMAP3_OTG_SYSCONFIG_NO_STANDBY_MODE |
		       OMAP3_OTG_SYSCONFIG_NO_IDLE_MODE, &otg->sysconfig);

		/* Set the interface */
		writel(OMAP3_OTG_INTERFSEL_OMAP, &otg->interfsel);

		/* Clear force standby */
		stdby = readl(&otg->forcestdby);
		stdby &= ~OMAP3_OTG_FORCESTDBY_STANDBY;
		writel(stdby, &otg->forcestdby);

#ifdef CONFIG_TARGET_OMAP3_EVM
		musb_cfg.extvbus = omap3_evm_need_extvbus();
#endif

#ifdef CONFIG_OMAP44XX
		u32 *usbotghs_control =
			(u32 *)((*ctrl)->control_usbotghs_ctrl);
		*usbotghs_control = 0x15;
#endif
		platform_needs_initialization = 0;
	}

	ret = platform_needs_initialization;

#ifdef CONFIG_TWL4030_USB
end:
#endif
	return ret;

}

void musb_platform_deinit(void)
{
	/* noop */
}
