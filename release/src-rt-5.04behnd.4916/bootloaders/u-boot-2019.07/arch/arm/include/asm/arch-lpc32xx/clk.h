/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2011 by Vladimir Zapolskiy <vz@mleia.com>
 */

#ifndef _LPC32XX_CLK_H
#define _LPC32XX_CLK_H

#include <asm/types.h>

#define OSC_CLK_FREQUENCY	13000000
#define RTC_CLK_FREQUENCY	32768

/* Clocking and Power Control Registers */
struct clk_pm_regs {
	u32 reserved0[5];
	u32 boot_map;		/* Boot Map Control Register		*/
	u32 p0_intr_er;		/* Port 0/1 Start and Interrupt Enable	*/
	u32 usbdiv_ctrl;	/* USB Clock Pre-Divide Register	*/
	/* Internal Start Signal Sources Registers	*/
	u32 start_er_int;	/* Start Enable Register		*/
	u32 start_rsr_int;	/* Start Raw Status Register		*/
	u32 start_sr_int;	/* Start Status Register		*/
	u32 start_apr_int;	/* Start Activation Polarity Register	*/
	/* Device Pin Start Signal Sources Registers	*/
	u32 start_er_pin;	/* Start Enable Register		*/
	u32 start_rsr_pin;	/* Start Raw Status Register		*/
	u32 start_sr_pin;	/* Start Status Register		*/
	u32 start_apr_pin;	/* Start Activation Polarity Register	*/
	/* Clock Control Registers			*/
	u32 hclkdiv_ctrl;	/* HCLK Divider Control Register	*/
	u32 pwr_ctrl;		/* Power Control Register		*/
	u32 pll397_ctrl;	/* PLL397 Control Register		*/
	u32 osc_ctrl;		/* Main Oscillator Control Register	*/
	u32 sysclk_ctrl;	/* SYSCLK Control Register		*/
	u32 lcdclk_ctrl;	/* LCD Clock Control Register		*/
	u32 hclkpll_ctrl;	/* HCLK PLL Control Register		*/
	u32 reserved1;
	u32 adclk_ctrl1;	/* ADC Clock Control1 Register		*/
	u32 usb_ctrl;		/* USB Control Register			*/
	u32 sdramclk_ctrl;	/* SDRAM Clock Control Register		*/
	u32 ddr_lap_nom;	/* DDR Calibration Nominal Value	*/
	u32 ddr_lap_count;	/* DDR Calibration Measured Value	*/
	u32 ddr_cal_delay;	/* DDR Calibration Delay Value		*/
	u32 ssp_ctrl;		/* SSP Control Register			*/
	u32 i2s_ctrl;		/* I2S Clock Control Register		*/
	u32 ms_ctrl;		/* Memory Card Control Register		*/
	u32 reserved2[3];
	u32 macclk_ctrl;	/* Ethernet MAC Clock Control Register	*/
	u32 reserved3[4];
	u32 test_clk;		/* Test Clock Selection Register	*/
	u32 sw_int;		/* Software Interrupt Register		*/
	u32 i2cclk_ctrl;	/* I2C Clock Control Register		*/
	u32 keyclk_ctrl;	/* Keyboard Scan Clock Control Register	*/
	u32 adclk_ctrl;		/* ADC Clock Control Register		*/
	u32 pwmclk_ctrl;	/* PWM Clock Control Register		*/
	u32 timclk_ctrl;	/* Watchdog and Highspeed Timer Control */
	u32 timclk_ctrl1;	/* Motor and Timer Clock Control	*/
	u32 spi_ctrl;		/* SPI Control Register			*/
	u32 flashclk_ctrl;	/* NAND Flash Clock Control Register	*/
	u32 reserved4;
	u32 u3clk;		/* UART 3 Clock Control Register	*/
	u32 u4clk;		/* UART 4 Clock Control Register	*/
	u32 u5clk;		/* UART 5 Clock Control Register	*/
	u32 u6clk;		/* UART 6 Clock Control Register	*/
	u32 irdaclk;		/* IrDA Clock Control Register		*/
	u32 uartclk_ctrl;	/* UART Clock Control Register		*/
	u32 dmaclk_ctrl;	/* DMA Clock Control Register		*/
	u32 autoclk_ctrl;	/* Autoclock Control Register		*/
};

/* HCLK Divider Control Register bits */
#define CLK_HCLK_DDRAM_MASK		(0x3 << 7)
#define CLK_HCLK_DDRAM_HALF		(0x2 << 7)
#define CLK_HCLK_DDRAM_NOMINAL		(0x1 << 7)
#define CLK_HCLK_DDRAM_STOPPED		(0x0 << 7)
#define CLK_HCLK_PERIPH_DIV_MASK	(0x1F << 2)
#define CLK_HCLK_PERIPH_DIV(n)		((((n) - 1) & 0x1F) << 2)
#define CLK_HCLK_ARM_PLL_DIV_MASK	(0x3 << 0)
#define CLK_HCLK_ARM_PLL_DIV_4		(0x2 << 0)
#define CLK_HCLK_ARM_PLL_DIV_2		(0x1 << 0)
#define CLK_HCLK_ARM_PLL_DIV_1		(0x0 << 0)

/* Power Control Register bits */
#define CLK_PWR_HCLK_RUN_PERIPH		(1 << 10)
#define CLK_PWR_EMC_SREFREQ		(1 << 9)
#define CLK_PWR_EMC_SREFREQ_UPDATE	(1 << 8)
#define CLK_PWR_SDRAM_SREFREQ		(1 << 7)
#define CLK_PWR_HIGHCORE_LEVEL		(1 << 5)
#define CLK_PWR_SYSCLKEN_LEVEL		(1 << 4)
#define CLK_PWR_SYSCLKEN_CTRL		(1 << 3)
#define CLK_PWR_NORMAL_RUN		(1 << 2)
#define CLK_PWR_HIGHCORE_CTRL		(1 << 1)
#define CLK_PWR_STOP_MODE		(1 << 0)

/* SYSCLK Control Register bits */
#define CLK_SYSCLK_PLL397		(1 << 1)
#define CLK_SYSCLK_MUX			(1 << 0)

/* HCLK PLL Control Register bits */
#define CLK_HCLK_PLL_OPERATING		(1 << 16)
#define CLK_HCLK_PLL_BYPASS		(1 << 15)
#define CLK_HCLK_PLL_DIRECT		(1 << 14)
#define CLK_HCLK_PLL_FEEDBACK		(1 << 13)
#define CLK_HCLK_PLL_POSTDIV_MASK	(0x3 << 11)
#define CLK_HCLK_PLL_POSTDIV_16		(0x3 << 11)
#define CLK_HCLK_PLL_POSTDIV_8		(0x2 << 11)
#define CLK_HCLK_PLL_POSTDIV_4		(0x1 << 11)
#define CLK_HCLK_PLL_POSTDIV_2		(0x0 << 11)
#define CLK_HCLK_PLL_PREDIV_MASK	(0x3 << 9)
#define CLK_HCLK_PLL_PREDIV_4		(0x3 << 9)
#define CLK_HCLK_PLL_PREDIV_3		(0x2 << 9)
#define CLK_HCLK_PLL_PREDIV_2		(0x1 << 9)
#define CLK_HCLK_PLL_PREDIV_1		(0x0 << 9)
#define CLK_HCLK_PLL_FEEDBACK_DIV_MASK	(0xFF << 1)
#define CLK_HCLK_PLL_FEEDBACK_DIV(n)	((((n) - 1) & 0xFF) << 1)
#define CLK_HCLK_PLL_LOCKED		(1 << 0)

/* Ethernet MAC Clock Control Register bits	*/
#define CLK_MAC_RMII			(0x3 << 3)
#define CLK_MAC_MII			(0x1 << 3)
#define CLK_MAC_MASTER			(1 << 2)
#define CLK_MAC_SLAVE			(1 << 1)
#define CLK_MAC_REG			(1 << 0)

/* I2C Clock Control Register bits	*/
#define CLK_I2C2_ENABLE			(1 << 1)
#define CLK_I2C1_ENABLE			(1 << 0)

/* Timer Clock Control1 Register bits */
#define CLK_TIMCLK_MOTOR		(1 << 6)
#define CLK_TIMCLK_TIMER3		(1 << 5)
#define CLK_TIMCLK_TIMER2		(1 << 4)
#define CLK_TIMCLK_TIMER1		(1 << 3)
#define CLK_TIMCLK_TIMER0		(1 << 2)
#define CLK_TIMCLK_TIMER5		(1 << 1)
#define CLK_TIMCLK_TIMER4		(1 << 0)

/* Timer Clock Control Register bits */
#define CLK_TIMCLK_HSTIMER		(1 << 1)
#define CLK_TIMCLK_WATCHDOG		(1 << 0)

/* UART Clock Control Register bits */
#define CLK_UART(n)			(1 << ((n) - 3))

/* UARTn Clock Select Registers bits */
#define CLK_UART_HCLK			(1 << 16)
#define CLK_UART_X_DIV(n)		(((n) & 0xFF) << 8)
#define CLK_UART_Y_DIV(n)		(((n) & 0xFF) << 0)

/* DMA Clock Control Register bits */
#define CLK_DMA_ENABLE			(1 << 0)

/* NAND Clock Control Register bits */
#define CLK_NAND_SLC			(1 << 0)
#define CLK_NAND_MLC			(1 << 1)
#define CLK_NAND_SLC_SELECT		(1 << 2)
#define CLK_NAND_MLC_INT		(1 << 5)

/* SSP Clock Control Register bits */
#define CLK_SSP0_ENABLE_CLOCK		(1 << 0)

/* SDRAMCLK register bits */
#define CLK_SDRAM_DDR_SEL		(1 << 1)

/* USB control register definitions */
#define CLK_USBCTRL_PLL_STS		(1 << 0)
#define CLK_USBCTRL_FDBK_PLUS1(n)	(((n) & 0xFF) << 1)
#define CLK_USBCTRL_POSTDIV_2POW(n)	(((n) & 0x3) << 11)
#define CLK_USBCTRL_PLL_PWRUP		(1 << 16)
#define CLK_USBCTRL_CLK_EN1		(1 << 17)
#define CLK_USBCTRL_CLK_EN2		(1 << 18)
#define CLK_USBCTRL_BUS_KEEPER		(0x1 << 19)
#define CLK_USBCTRL_USBHSTND_EN		(1 << 21)
#define CLK_USBCTRL_USBDVND_EN		(1 << 22)
#define CLK_USBCTRL_HCLK_EN		(1 << 24)

unsigned int get_sys_clk_rate(void);
unsigned int get_hclk_pll_rate(void);
unsigned int get_hclk_clk_div(void);
unsigned int get_hclk_clk_rate(void);
unsigned int get_periph_clk_div(void);
unsigned int get_periph_clk_rate(void);
unsigned int get_sdram_clk_rate(void);

#endif /* _LPC32XX_CLK_H */
