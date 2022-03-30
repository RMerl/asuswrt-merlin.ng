/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2016 Google, Inc
 */

#ifndef _ASM_ARCH_WDT_H
#define _ASM_ARCH_WDT_H

#define WDT_BASE			0x1e785000

/*
 * Special value that needs to be written to counter_restart register to
 * (re)start the timer
 */
#define WDT_COUNTER_RESTART_VAL		0x4755

/* Control register */
#define WDT_CTRL_RESET_MODE_SHIFT	5
#define WDT_CTRL_RESET_MODE_MASK	3

#define WDT_CTRL_EN			(1 << 0)
#define WDT_CTRL_RESET			(1 << 1)
#define WDT_CTRL_CLK1MHZ		(1 << 4)
#define WDT_CTRL_2ND_BOOT		(1 << 7)

/* Values for Reset Mode */
#define WDT_CTRL_RESET_SOC		0
#define WDT_CTRL_RESET_CHIP		1
#define WDT_CTRL_RESET_CPU		2
#define WDT_CTRL_RESET_MASK		3

/* Reset Mask register */
#define WDT_RESET_ARM			(1 << 0)
#define WDT_RESET_COPROC		(1 << 1)
#define WDT_RESET_SDRAM			(1 << 2)
#define WDT_RESET_AHB			(1 << 3)
#define WDT_RESET_I2C			(1 << 4)
#define WDT_RESET_MAC1			(1 << 5)
#define WDT_RESET_MAC2			(1 << 6)
#define WDT_RESET_GCRT			(1 << 7)
#define WDT_RESET_USB20			(1 << 8)
#define WDT_RESET_USB11_HOST		(1 << 9)
#define WDT_RESET_USB11_EHCI2		(1 << 10)
#define WDT_RESET_VIDEO			(1 << 11)
#define WDT_RESET_HAC			(1 << 12)
#define WDT_RESET_LPC			(1 << 13)
#define WDT_RESET_SDSDIO		(1 << 14)
#define WDT_RESET_MIC			(1 << 15)
#define WDT_RESET_CRT2C			(1 << 16)
#define WDT_RESET_PWM			(1 << 17)
#define WDT_RESET_PECI			(1 << 18)
#define WDT_RESET_JTAG			(1 << 19)
#define WDT_RESET_ADC			(1 << 20)
#define WDT_RESET_GPIO			(1 << 21)
#define WDT_RESET_MCTP			(1 << 22)
#define WDT_RESET_XDMA			(1 << 23)
#define WDT_RESET_SPI			(1 << 24)
#define WDT_RESET_MISC			(1 << 25)

#define WDT_RESET_DEFAULT						\
	(WDT_RESET_ARM | WDT_RESET_COPROC | WDT_RESET_I2C |		\
	 WDT_RESET_MAC1 | WDT_RESET_MAC2 | WDT_RESET_GCRT |		\
	 WDT_RESET_USB20 | WDT_RESET_USB11_HOST | WDT_RESET_USB11_EHCI2 | \
	 WDT_RESET_VIDEO | WDT_RESET_HAC | WDT_RESET_LPC |		\
	 WDT_RESET_SDSDIO | WDT_RESET_MIC | WDT_RESET_CRT2C |		\
	 WDT_RESET_PWM | WDT_RESET_PECI | WDT_RESET_JTAG |		\
	 WDT_RESET_ADC | WDT_RESET_GPIO | WDT_RESET_MISC)

#ifndef __ASSEMBLY__
struct ast_wdt {
	u32 counter_status;
	u32 counter_reload_val;
	u32 counter_restart;
	u32 ctrl;
	u32 timeout_status;
	u32 clr_timeout_status;
	u32 reset_width;
	/* On pre-ast2500 SoCs this register is reserved. */
	u32 reset_mask;
};

/**
 * Given flags parameter passed to wdt_reset or wdt_start uclass functions,
 * gets Reset Mode value from it.
 *
 * @flags: flags parameter passed into wdt_reset or wdt_start
 * @return Reset Mode value
 */
u32 ast_reset_mode_from_flags(ulong flags);

/**
 * Given flags parameter passed to wdt_reset or wdt_start uclass functions,
 * gets Reset Mask value from it. Reset Mask is only supported on ast2500
 *
 * @flags: flags parameter passed into wdt_reset or wdt_start
 * @return Reset Mask value
 */
u32 ast_reset_mask_from_flags(ulong flags);

/**
 * Given Reset Mask and Reset Mode values, converts them to flags,
 * suitable for passing into wdt_start or wdt_reset uclass functions.
 *
 * On ast2500 Reset Mask is 25 bits wide and Reset Mode is 2 bits wide, so they
 * can both be packed into single 32 bits wide value.
 *
 * @reset_mode: Reset Mode
 * @reset_mask: Reset Mask
 */
ulong ast_flags_from_reset_mode_mask(u32 reset_mode, u32 reset_mask);
#endif  /* __ASSEMBLY__ */

#endif /* _ASM_ARCH_WDT_H */
