// SPDX-License-Identifier: GPL-2.0+
/*
 * board.c
 *
 * Common board functions for AM33XX based boards
 *
 * Copyright (C) 2011, Texas Instruments, Incorporated - http://www.ti.com/
 */

#include <common.h>
#include <dm.h>
#include <debug_uart.h>
#include <errno.h>
#include <ns16550.h>
#include <spl.h>
#include <asm/arch/cpu.h>
#include <asm/arch/hardware.h>
#include <asm/arch/omap.h>
#include <asm/arch/ddr_defs.h>
#include <asm/arch/clock.h>
#include <asm/arch/gpio.h>
#include <asm/arch/i2c.h>
#include <asm/arch/mem.h>
#include <asm/arch/mmc_host_def.h>
#include <asm/arch/sys_proto.h>
#include <asm/io.h>
#include <asm/emif.h>
#include <asm/gpio.h>
#include <asm/omap_common.h>
#include <i2c.h>
#include <miiphy.h>
#include <cpsw.h>
#include <linux/errno.h>
#include <linux/compiler.h>
#include <linux/usb/ch9.h>
#include <linux/usb/gadget.h>
#include <linux/usb/musb.h>
#include <asm/omap_musb.h>
#include <asm/davinci_rtc.h>

#define AM43XX_EMIF_BASE				0x4C000000
#define AM43XX_SDRAM_CONFIG_OFFSET			0x8
#define AM43XX_SDRAM_TYPE_MASK				0xE0000000
#define AM43XX_SDRAM_TYPE_SHIFT				29
#define AM43XX_SDRAM_TYPE_DDR3				3
#define AM43XX_READ_WRITE_LEVELING_CTRL_OFFSET		0xDC
#define AM43XX_RDWRLVLFULL_START			0x80000000

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
#ifndef CONFIG_SKIP_LOWLEVEL_INIT
	sdram_init();
#endif

	/* dram_init must store complete ramsize in gd->ram_size */
	gd->ram_size = get_ram_size(
			(void *)CONFIG_SYS_SDRAM_BASE,
			CONFIG_MAX_RAM_BANK_SIZE);
	return 0;
}

int dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = CONFIG_SYS_SDRAM_BASE;
	gd->bd->bi_dram[0].size = gd->ram_size;

	return 0;
}

#if !CONFIG_IS_ENABLED(OF_CONTROL)
static const struct ns16550_platdata am33xx_serial[] = {
	{ .base = CONFIG_SYS_NS16550_COM1, .reg_shift = 2,
	  .clock = CONFIG_SYS_NS16550_CLK, .fcr = UART_FCR_DEFVAL, },
# ifdef CONFIG_SYS_NS16550_COM2
	{ .base = CONFIG_SYS_NS16550_COM2, .reg_shift = 2,
	  .clock = CONFIG_SYS_NS16550_CLK, .fcr = UART_FCR_DEFVAL, },
#  ifdef CONFIG_SYS_NS16550_COM3
	{ .base = CONFIG_SYS_NS16550_COM3, .reg_shift = 2,
	  .clock = CONFIG_SYS_NS16550_CLK, .fcr = UART_FCR_DEFVAL, },
	{ .base = CONFIG_SYS_NS16550_COM4, .reg_shift = 2,
	  .clock = CONFIG_SYS_NS16550_CLK, .fcr = UART_FCR_DEFVAL, },
	{ .base = CONFIG_SYS_NS16550_COM5, .reg_shift = 2,
	  .clock = CONFIG_SYS_NS16550_CLK, .fcr = UART_FCR_DEFVAL, },
	{ .base = CONFIG_SYS_NS16550_COM6, .reg_shift = 2,
	  .clock = CONFIG_SYS_NS16550_CLK, .fcr = UART_FCR_DEFVAL, },
#  endif
# endif
};

U_BOOT_DEVICES(am33xx_uarts) = {
	{ "ns16550_serial", &am33xx_serial[0] },
#  ifdef CONFIG_SYS_NS16550_COM2
	{ "ns16550_serial", &am33xx_serial[1] },
#   ifdef CONFIG_SYS_NS16550_COM3
	{ "ns16550_serial", &am33xx_serial[2] },
	{ "ns16550_serial", &am33xx_serial[3] },
	{ "ns16550_serial", &am33xx_serial[4] },
	{ "ns16550_serial", &am33xx_serial[5] },
#   endif
#  endif
};

#ifdef CONFIG_DM_I2C
static const struct omap_i2c_platdata am33xx_i2c[] = {
	{ I2C_BASE1, 100000, OMAP_I2C_REV_V2},
	{ I2C_BASE2, 100000, OMAP_I2C_REV_V2},
	{ I2C_BASE3, 100000, OMAP_I2C_REV_V2},
};

U_BOOT_DEVICES(am33xx_i2c) = {
	{ "i2c_omap", &am33xx_i2c[0] },
	{ "i2c_omap", &am33xx_i2c[1] },
	{ "i2c_omap", &am33xx_i2c[2] },
};
#endif

#ifdef CONFIG_DM_GPIO
static const struct omap_gpio_platdata am33xx_gpio[] = {
	{ 0, AM33XX_GPIO0_BASE },
	{ 1, AM33XX_GPIO1_BASE },
	{ 2, AM33XX_GPIO2_BASE },
	{ 3, AM33XX_GPIO3_BASE },
#ifdef CONFIG_AM43XX
	{ 4, AM33XX_GPIO4_BASE },
	{ 5, AM33XX_GPIO5_BASE },
#endif
};

U_BOOT_DEVICES(am33xx_gpios) = {
	{ "gpio_omap", &am33xx_gpio[0] },
	{ "gpio_omap", &am33xx_gpio[1] },
	{ "gpio_omap", &am33xx_gpio[2] },
	{ "gpio_omap", &am33xx_gpio[3] },
#ifdef CONFIG_AM43XX
	{ "gpio_omap", &am33xx_gpio[4] },
	{ "gpio_omap", &am33xx_gpio[5] },
#endif
};
#endif
#endif

#ifndef CONFIG_DM_GPIO
static const struct gpio_bank gpio_bank_am33xx[] = {
	{ (void *)AM33XX_GPIO0_BASE },
	{ (void *)AM33XX_GPIO1_BASE },
	{ (void *)AM33XX_GPIO2_BASE },
	{ (void *)AM33XX_GPIO3_BASE },
#ifdef CONFIG_AM43XX
	{ (void *)AM33XX_GPIO4_BASE },
	{ (void *)AM33XX_GPIO5_BASE },
#endif
};

const struct gpio_bank *const omap_gpio_bank = gpio_bank_am33xx;
#endif

#if defined(CONFIG_MMC_OMAP_HS)
int cpu_mmc_init(bd_t *bis)
{
	int ret;

	ret = omap_mmc_init(0, 0, 0, -1, -1);
	if (ret)
		return ret;

	return omap_mmc_init(1, 0, 0, -1, -1);
}
#endif

/*
 * RTC only with DDR in self-refresh mode magic value, checked against during
 * boot to see if we have a valid config. This should be in sync with the value
 * that will be in drivers/soc/ti/pm33xx.c.
 */
#define RTC_MAGIC_VAL		0x8cd0

/* Board type field bit shift for RTC only with DDR in self-refresh mode */
#define RTC_BOARD_TYPE_SHIFT	16

/* AM33XX has two MUSB controllers which can be host or gadget */
#if (defined(CONFIG_USB_MUSB_GADGET) || defined(CONFIG_USB_MUSB_HOST)) && \
	(defined(CONFIG_AM335X_USB0) || defined(CONFIG_AM335X_USB1)) && \
	(!CONFIG_IS_ENABLED(DM_USB) || !CONFIG_IS_ENABLED(OF_CONTROL)) && \
	(!defined(CONFIG_SPL_BUILD) || defined(CONFIG_SPL_MUSB_NEW_SUPPORT))

static struct musb_hdrc_config musb_config = {
	.multipoint     = 1,
	.dyn_fifo       = 1,
	.num_eps        = 16,
	.ram_bits       = 12,
};

#if CONFIG_IS_ENABLED(DM_USB) && !CONFIG_IS_ENABLED(OF_CONTROL)
static struct ti_musb_platdata usb0 = {
	.base = (void *)USB0_OTG_BASE,
	.ctrl_mod_base = &((struct ctrl_dev *)CTRL_DEVICE_BASE)->usb_ctrl0,
	.plat = {
		.config         = &musb_config,
		.power          = 50,
		.platform_ops	= &musb_dsps_ops,
		},
};

static struct ti_musb_platdata usb1 = {
	.base = (void *)USB1_OTG_BASE,
	.ctrl_mod_base = &((struct ctrl_dev *)CTRL_DEVICE_BASE)->usb_ctrl1,
	.plat = {
		.config         = &musb_config,
		.power          = 50,
		.platform_ops	= &musb_dsps_ops,
		},
};

U_BOOT_DEVICES(am33xx_usbs) = {
#if CONFIG_AM335X_USB0_MODE == MUSB_PERIPHERAL
	{ "ti-musb-peripheral", &usb0 },
#elif CONFIG_AM335X_USB0_MODE == MUSB_HOST
	{ "ti-musb-host", &usb0 },
#endif
#if CONFIG_AM335X_USB1_MODE == MUSB_PERIPHERAL
	{ "ti-musb-peripheral", &usb1 },
#elif CONFIG_AM335X_USB1_MODE == MUSB_HOST
	{ "ti-musb-host", &usb1 },
#endif
};

int arch_misc_init(void)
{
	return 0;
}
#else
static struct ctrl_dev *cdev = (struct ctrl_dev *)CTRL_DEVICE_BASE;

/* USB 2.0 PHY Control */
#define CM_PHY_PWRDN			(1 << 0)
#define CM_PHY_OTG_PWRDN		(1 << 1)
#define OTGVDET_EN			(1 << 19)
#define OTGSESSENDEN			(1 << 20)

static void am33xx_usb_set_phy_power(u8 on, u32 *reg_addr)
{
	if (on) {
		clrsetbits_le32(reg_addr, CM_PHY_PWRDN | CM_PHY_OTG_PWRDN,
				OTGVDET_EN | OTGSESSENDEN);
	} else {
		clrsetbits_le32(reg_addr, 0, CM_PHY_PWRDN | CM_PHY_OTG_PWRDN);
	}
}

#ifdef CONFIG_AM335X_USB0
static void am33xx_otg0_set_phy_power(struct udevice *dev, u8 on)
{
	am33xx_usb_set_phy_power(on, &cdev->usb_ctrl0);
}

struct omap_musb_board_data otg0_board_data = {
	.set_phy_power = am33xx_otg0_set_phy_power,
};

static struct musb_hdrc_platform_data otg0_plat = {
	.mode           = CONFIG_AM335X_USB0_MODE,
	.config         = &musb_config,
	.power          = 50,
	.platform_ops	= &musb_dsps_ops,
	.board_data	= &otg0_board_data,
};
#endif

#ifdef CONFIG_AM335X_USB1
static void am33xx_otg1_set_phy_power(struct udevice *dev, u8 on)
{
	am33xx_usb_set_phy_power(on, &cdev->usb_ctrl1);
}

struct omap_musb_board_data otg1_board_data = {
	.set_phy_power = am33xx_otg1_set_phy_power,
};

static struct musb_hdrc_platform_data otg1_plat = {
	.mode           = CONFIG_AM335X_USB1_MODE,
	.config         = &musb_config,
	.power          = 50,
	.platform_ops	= &musb_dsps_ops,
	.board_data	= &otg1_board_data,
};
#endif

int arch_misc_init(void)
{
#ifdef CONFIG_AM335X_USB0
	musb_register(&otg0_plat, &otg0_board_data,
		(void *)USB0_OTG_BASE);
#endif
#ifdef CONFIG_AM335X_USB1
	musb_register(&otg1_plat, &otg1_board_data,
		(void *)USB1_OTG_BASE);
#endif
	return 0;
}
#endif

#else	/* CONFIG_USB_MUSB_* && CONFIG_AM335X_USB* && !CONFIG_DM_USB */

int arch_misc_init(void)
{
	struct udevice *dev;
	int ret;

	ret = uclass_first_device(UCLASS_MISC, &dev);
	if (ret || !dev)
		return ret;

#if defined(CONFIG_DM_ETH) && defined(CONFIG_USB_ETHER)
	ret = usb_ether_init();
	if (ret) {
		pr_err("USB ether init failed\n");
		return ret;
	}
#endif

	return 0;
}

#endif /* CONFIG_USB_MUSB_* && CONFIG_AM335X_USB* && !CONFIG_DM_USB */

#ifndef CONFIG_SKIP_LOWLEVEL_INIT

#if defined(CONFIG_SPL_AM33XX_ENABLE_RTC32K_OSC) || \
	(defined(CONFIG_SPL_BUILD) && defined(CONFIG_SPL_RTC_DDR_SUPPORT))
static void rtc32k_unlock(struct davinci_rtc *rtc)
{
	/*
	 * Unlock the RTC's registers.  For more details please see the
	 * RTC_SS section of the TRM.  In order to unlock we need to
	 * write these specific values (keys) in this order.
	 */
	writel(RTC_KICK0R_WE, &rtc->kick0r);
	writel(RTC_KICK1R_WE, &rtc->kick1r);
}
#endif

#if defined(CONFIG_SPL_BUILD) && defined(CONFIG_SPL_RTC_DDR_SUPPORT)
/*
 * Write contents of the RTC_SCRATCH1 register based on board type
 * Two things are passed
 * on. First 16 bits (0:15) are written with RTC_MAGIC value. Once the
 * control gets to kernel, kernel reads the scratchpad register and gets to
 * know that bootloader has rtc_only support.
 *
 * Second important thing is the board type  (16:31). This is needed in the
 * rtc_only boot where in we want to avoid costly i2c reads to eeprom to
 * identify the board type and we go ahead and copy the board strings to
 * am43xx_board_name.
 */
void update_rtc_magic(void)
{
	struct davinci_rtc *rtc = (struct davinci_rtc *)RTC_BASE;
	u32 magic = RTC_MAGIC_VAL;

	magic |= (rtc_only_get_board_type() << RTC_BOARD_TYPE_SHIFT);

	rtc32k_unlock(rtc);

	/* write magic */
	writel(magic, &rtc->scratch1);
}
#endif

/*
 * In the case of non-SPL based booting we'll want to call these
 * functions a tiny bit later as it will require gd to be set and cleared
 * and that's not true in s_init in this case so we cannot do it there.
 */
int board_early_init_f(void)
{
	prcm_init();
	set_mux_conf_regs();
#if defined(CONFIG_SPL_BUILD) && defined(CONFIG_SPL_RTC_DDR_SUPPORT)
	update_rtc_magic();
#endif
	return 0;
}

/*
 * This function is the place to do per-board things such as ramp up the
 * MPU clock frequency.
 */
__weak void am33xx_spl_board_init(void)
{
}

#if defined(CONFIG_SPL_AM33XX_ENABLE_RTC32K_OSC)
static void rtc32k_enable(void)
{
	struct davinci_rtc *rtc = (struct davinci_rtc *)RTC_BASE;

	rtc32k_unlock(rtc);

	/* Enable the RTC 32K OSC by setting bits 3 and 6. */
	writel((1 << 3) | (1 << 6), &rtc->osc);
}
#endif

static void uart_soft_reset(void)
{
	struct uart_sys *uart_base = (struct uart_sys *)DEFAULT_UART_BASE;
	u32 regval;

	regval = readl(&uart_base->uartsyscfg);
	regval |= UART_RESET;
	writel(regval, &uart_base->uartsyscfg);
	while ((readl(&uart_base->uartsyssts) &
		UART_CLK_RUNNING_MASK) != UART_CLK_RUNNING_MASK)
		;

	/* Disable smart idle */
	regval = readl(&uart_base->uartsyscfg);
	regval |= UART_SMART_IDLE_EN;
	writel(regval, &uart_base->uartsyscfg);
}

static void watchdog_disable(void)
{
	struct wd_timer *wdtimer = (struct wd_timer *)WDT_BASE;

	writel(0xAAAA, &wdtimer->wdtwspr);
	while (readl(&wdtimer->wdtwwps) != 0x0)
		;
	writel(0x5555, &wdtimer->wdtwspr);
	while (readl(&wdtimer->wdtwwps) != 0x0)
		;
}

#if defined(CONFIG_SPL_BUILD) && defined(CONFIG_SPL_RTC_DDR_SUPPORT)
/*
 * Check if we are executing rtc-only + DDR mode, and resume from it if needed
 */
static void rtc_only(void)
{
	struct davinci_rtc *rtc = (struct davinci_rtc *)RTC_BASE;
	struct prm_device_inst *prm_device =
				(struct prm_device_inst *)PRM_DEVICE_INST;

	u32 scratch1, sdrc;
	void (*resume_func)(void);

	scratch1 = readl(&rtc->scratch1);

	/*
	 * Check RTC scratch against RTC_MAGIC_VAL, RTC_MAGIC_VAL is only
	 * written to this register when we want to wake up from RTC only
	 * with DDR in self-refresh mode. Contents of the RTC_SCRATCH1:
	 * bits 0-15:  RTC_MAGIC_VAL
	 * bits 16-31: board type (needed for sdram_init)
	 */
	if ((scratch1 & 0xffff) != RTC_MAGIC_VAL)
		return;

	rtc32k_unlock(rtc);

	/* Clear RTC magic */
	writel(0, &rtc->scratch1);

	/*
	 * Update board type based on value stored on RTC_SCRATCH1, this
	 * is done so that we don't need to read the board type from eeprom
	 * over i2c bus which is expensive
	 */
	rtc_only_update_board_type(scratch1 >> RTC_BOARD_TYPE_SHIFT);

	/*
	 * Enable EMIF_DEVOFF in PRCM_PRM_EMIF_CTRL to indicate to EMIF we
	 * are resuming from self-refresh. This avoids an unnecessary re-init
	 * of the DDR. The re-init takes time and we would need to wait for
	 * it to complete before accessing DDR to avoid L3 NOC errors.
	 */
	writel(EMIF_CTRL_DEVOFF, &prm_device->emif_ctrl);

	rtc_only_prcm_init();
	sdram_init();

	/* Check EMIF4D_SDRAM_CONFIG[31:29] SDRAM_TYPE */
	/* Only perform leveling if SDRAM_TYPE = 3 (DDR3) */
	sdrc = readl(AM43XX_EMIF_BASE + AM43XX_SDRAM_CONFIG_OFFSET);

	sdrc &= AM43XX_SDRAM_TYPE_MASK;
	sdrc >>= AM43XX_SDRAM_TYPE_SHIFT;

	if (sdrc == AM43XX_SDRAM_TYPE_DDR3) {
		writel(AM43XX_RDWRLVLFULL_START,
		       AM43XX_EMIF_BASE +
		       AM43XX_READ_WRITE_LEVELING_CTRL_OFFSET);
		mdelay(1);

am43xx_wait:
		sdrc = readl(AM43XX_EMIF_BASE +
			     AM43XX_READ_WRITE_LEVELING_CTRL_OFFSET);
		if (sdrc == AM43XX_RDWRLVLFULL_START)
			goto am43xx_wait;
	}

	resume_func = (void *)readl(&rtc->scratch0);
	if (resume_func)
		resume_func();
}
#endif

void s_init(void)
{
#if defined(CONFIG_SPL_BUILD) && defined(CONFIG_SPL_RTC_DDR_SUPPORT)
	rtc_only();
#endif
}

void early_system_init(void)
{
	/*
	 * The ROM will only have set up sufficient pinmux to allow for the
	 * first 4KiB NOR to be read, we must finish doing what we know of
	 * the NOR mux in this space in order to continue.
	 */
#ifdef CONFIG_NOR_BOOT
	enable_norboot_pin_mux();
#endif
	watchdog_disable();
	set_uart_mux_conf();
	setup_early_clocks();
	uart_soft_reset();
#ifdef CONFIG_SPL_BUILD
	/*
	 * Save the boot parameters passed from romcode.
	 * We cannot delay the saving further than this,
	 * to prevent overwrites.
	 */
	save_omap_boot_params();
#endif
#ifdef CONFIG_DEBUG_UART_OMAP
	debug_uart_init();
#endif

#ifdef CONFIG_SPL_BUILD
	spl_early_init();
#endif

#ifdef CONFIG_TI_I2C_BOARD_DETECT
	do_board_detect();
#endif

#if defined(CONFIG_SPL_AM33XX_ENABLE_RTC32K_OSC)
	/* Enable RTC32K clock */
	rtc32k_enable();
#endif
}

#ifdef CONFIG_SPL_BUILD
void board_init_f(ulong dummy)
{
	hw_data_init();
	early_system_init();
	board_early_init_f();
	sdram_init();
	/* dram_init must store complete ramsize in gd->ram_size */
	gd->ram_size = get_ram_size(
			(void *)CONFIG_SYS_SDRAM_BASE,
			CONFIG_MAX_RAM_BANK_SIZE);
}
#endif

#endif

int arch_cpu_init_dm(void)
{
	hw_data_init();
#ifndef CONFIG_SKIP_LOWLEVEL_INIT
	early_system_init();
#endif
	return 0;
}
