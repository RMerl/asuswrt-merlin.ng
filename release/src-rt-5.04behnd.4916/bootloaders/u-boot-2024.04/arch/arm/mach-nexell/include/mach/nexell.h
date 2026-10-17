/* SPDX-License-Identifier: GPL-2.0+
 *
 * (C) Copyright 2016 Nexell
 * Hyunseok, Jung <hsjung@nexell.co.kr>
 */

#ifndef __NEXELL_H__
#define __NEXELL_H__

#define PHY_BASEADDR_DMA0		(0xC0000000)
#define PHY_BASEADDR_DMA1		(0xC0001000)
#if defined(CONFIG_ARCH_S5P4418)
#define PHY_BASEADDR_INTC0		(0xC0002000)
#define PHY_BASEADDR_INTC1		(0xC0003000)
#elif defined(CONFIG_ARCH_S5P6818)
#define PHY_BASEADDR_INTC		(0xC0008000)
#endif
#define PHY_BASEADDR_CLKPWR		(0xC0010000)
#define PHY_BASEADDR_RTC		(0xC0010C00)
#define PHY_BASEADDR_ALIVE		(0xC0010800)
#define PHY_BASEADDR_RSTCON		(0xC0012000)
#define PHY_BASEADDR_TIEOFF		(0xC0011000)
#define PHY_BASEADDR_PDM		(0xC0014000)
#define PHY_BASEADDR_CRYPTO		(0xC0015000)
#define PHY_BASEADDR_TIMER		(0xC0017000)
#define PHY_BASEADDR_PWM		(0xC0018000)
#define PHY_BASEADDR_WDT		(0xC0019000)
#define PHY_BASEADDR_GPIOA		(0xC001A000)
#define PHY_BASEADDR_GPIOB		(0xC001B000)
#define PHY_BASEADDR_GPIOC		(0xC001C000)
#define PHY_BASEADDR_GPIOD		(0xC001D000)
#define PHY_BASEADDR_GPIOE		(0xC001E000)
#define PHY_BASEADDR_OHCI		(0xC0020000)
#define PHY_BASEADDR_EHCI		(0xC0030000)
#define PHY_BASEADDR_HSOTG		(0xC0040000)
#define PHY_BASEADDR_ADC		(0xC0053000)
#define PHY_BASEADDR_PPM		(0xC0054000)
#define PHY_BASEADDR_I2S0		(0xC0055000)
#define PHY_BASEADDR_I2S1		(0xC0056000)
#define PHY_BASEADDR_I2S2		(0xC0057000)
#define PHY_BASEADDR_AC97		(0xC0058000)
#define PHY_BASEADDR_SPDIF_TX		(0xC0059000)
#define PHY_BASEADDR_SPDIF_RX		(0xC005A000)
#define PHY_BASEADDR_SSP0		(0xC005B000)
#define PHY_BASEADDR_SSP1		(0xC005C000)
#define PHY_BASEADDR_SSP2		(0xC005F000)
#define PHY_BASEADDR_MPEGTSI		(0xC005D000)
#define PHY_BASEADDR_GMAC		(0xC0060000)
#define PHY_BASEADDR_VIP0		(0xC0063000)
#define PHY_BASEADDR_VIP1		(0xC0064000)
#if defined(CONFIG_ARCH_S5P6818)
#define PHY_BASEADDR_VIP2		(0xC0099000)
#endif
#define PHY_BASEADDR_DEINTERLACE	(0xC0065000)
#define PHY_BASEADDR_SCALER		(0xC0066000)
#define PHY_BASEADDR_ECID		(0xC0067000)
#define PHY_BASEADDR_SDMMC0		(0xC0062000)
#define PHY_BASEADDR_SDMMC1		(0xC0068000)
#define PHY_BASEADDR_SDMMC2		(0xC0069000)
#define PHY_BASEADDR_MALI400		(0xC0070000)
#define PHY_BASEADDR_CODA_APB0		(0xC0080000)
#define PHY_BASEADDR_CODA_APB1		(0xC0081000)
#define PHY_BASEADDR_CODA_APB2		(0xC0082000)
#define PHY_BASEADDR_CODA_APB3		(0xC0083000)
/* dma (O), modem(X), UART0_MODULE */
#define PHY_BASEADDR_UART0		(0xC00A1000)
/* dma (O), modem(O), pl01115_Uart_modem_MODULE */
#define PHY_BASEADDR_UART1		(0xC00A0000)
/* dma (O), modem(X), UART1_MODULE */
#define PHY_BASEADDR_UART2		(0xC00A2000)
/* dma (X), modem(X), pl01115_Uart_nodma0_MODULE */
#define PHY_BASEADDR_UART3		(0xC00A3000)
/* dma (X), modem(X), pl01115_Uart_nodma1_MODULE */
#define PHY_BASEADDR_UART4		(0xC006D000)
/* dma (X), modem(X), pl01115_Uart_nodma2_MODULE */
#define PHY_BASEADDR_UART5		(0xC006F000)
#define PHY_BASEADDR_I2C0		(0xC00A4000)
#define PHY_BASEADDR_I2C1		(0xC00A5000)
#define PHY_BASEADDR_I2C2		(0xC00A6000)
#define PHY_BASEADDR_CAN0		(0xC00CE000)
#define PHY_BASEADDR_CAN1		(0xC00CF000)
#define PHY_BASEADDR_MIPI		(0xC00D0000)
#define PHY_BASEADDR_DISPLAYTOP		(0xC0100000)

#define PHY_BASEADDR_CLKGEN0		(0xC00BB000)	/* TIMER_1 */
#define PHY_BASEADDR_CLKGEN1		(0xC00BC000)	/* TIMER_2 */
#define PHY_BASEADDR_CLKGEN2		(0xC00BD000)	/* TIMER_3 */
#define PHY_BASEADDR_CLKGEN3		(0xC00BE000)	/* PWM_1 */
#define PHY_BASEADDR_CLKGEN4		(0xC00BF000)	/* PWM_2 */
#define PHY_BASEADDR_CLKGEN5		(0xC00C0000)	/* PWM_3 */
#define PHY_BASEADDR_CLKGEN6		(0xC00AE000)	/* I2C_0 */
#define PHY_BASEADDR_CLKGEN7		(0xC00AF000)	/* I2C_1 */
#define PHY_BASEADDR_CLKGEN8		(0xC00B0000)	/* I2C_2 */
#define PHY_BASEADDR_CLKGEN9		(0xC00CA000)	/* MIPI */
#define PHY_BASEADDR_CLKGEN10		(0xC00C8000)	/* GMAC */
#define PHY_BASEADDR_CLKGEN11		(0xC00B8000)	/* SPDIF_TX */
#define PHY_BASEADDR_CLKGEN12		(0xC00B7000)	/* MPEGTSI */
#define PHY_BASEADDR_CLKGEN13		(0xC00BA000)	/* PWM_0 */
#define PHY_BASEADDR_CLKGEN14		(0xC00B9000)	/* TIMER_0 */
#define PHY_BASEADDR_CLKGEN15		(0xC00B2000)	/* I2S_0 */
#define PHY_BASEADDR_CLKGEN16		(0xC00B3000)	/* I2S_1 */
#define PHY_BASEADDR_CLKGEN17		(0xC00B4000)	/* I2S_2 */
#define PHY_BASEADDR_CLKGEN18		(0xC00C5000)	/* SDHC_0 */
#define PHY_BASEADDR_CLKGEN19		(0xC00CC000)	/* SDHC_1 */
#define PHY_BASEADDR_CLKGEN20		(0xC00CD000)	/* SDHC_2 */
#define PHY_BASEADDR_CLKGEN21		(0xC00C3000)	/* MALI */
#define PHY_BASEADDR_CLKGEN22		(0xC00A9000)	/* UART_0 */
#define PHY_BASEADDR_CLKGEN23		(0xC00AA000)	/* UART_2 */
#define PHY_BASEADDR_CLKGEN24		(0xC00A8000)	/* UART_1 */
#define PHY_BASEADDR_CLKGEN25		(0xC00AB000)	/* UART_3 */
#define PHY_BASEADDR_CLKGEN26		(0xC006E000)	/* UART_4 */
#define PHY_BASEADDR_CLKGEN27		(0xC00B1000)	/* UART_5 */
#define PHY_BASEADDR_CLKGEN28		(0xC00B5000)	/* DEINTERLACE */
#define PHY_BASEADDR_CLKGEN29		(0xC00C4000)	/* PPM */
#define PHY_BASEADDR_CLKGEN30		(0xC00C1000)	/* VIP_0 */
#define PHY_BASEADDR_CLKGEN31		(0xC00C2000)	/* VIP_1 */
#define PHY_BASEADDR_CLKGEN32		(0xC006B000)	/* USB2HOST */
#define PHY_BASEADDR_CLKGEN33		(0xC00C7000)	/* CODA */
#define PHY_BASEADDR_CLKGEN34		(0xC00C6000)	/* CRYPTO */
#define PHY_BASEADDR_CLKGEN35		(0xC00B6000)	/* SCALER */
#define PHY_BASEADDR_CLKGEN36		(0xC00CB000)	/* PDM */
#define PHY_BASEADDR_CLKGEN37		(0xC00AC000)	/* SPI0 */
#define PHY_BASEADDR_CLKGEN38		(0xC00AD000)	/* SPI1 */
#define PHY_BASEADDR_CLKGEN39		(0xC00A7000)	/* SPI2 */
#if defined(CONFIG_ARCH_S5P6818)
#define PHY_BASEADDR_CLKGEN40		(0xC009A000)
#endif
#define PHY_BASEADDR_DREX		(0xC00E0000)

#define PHY_BASEADDR_CS_NAND		(0x2C000000)

#define PHY_BASEADDR_SRAM		(0xFFFF0000)

/*
 * Nexell clock generator
 */
#define CLK_ID_TIMER_1			0
#define CLK_ID_TIMER_2			1
#define CLK_ID_TIMER_3			2
#define CLK_ID_PWM_1			3
#define CLK_ID_PWM_2			4
#define CLK_ID_PWM_3			5
#define CLK_ID_I2C_0			6
#define CLK_ID_I2C_1			7
#define CLK_ID_I2C_2			8
#define CLK_ID_MIPI			9
#define CLK_ID_GMAC			10	/* External Clock 1 */
#define CLK_ID_SPDIF_TX			11
#define CLK_ID_MPEGTSI			12
#define CLK_ID_PWM_0			13
#define CLK_ID_TIMER_0			14
#define CLK_ID_I2S_0			15	/* External Clock 1 */
#define CLK_ID_I2S_1			16	/* External Clock 1 */
#define CLK_ID_I2S_2			17	/* External Clock 1 */
#define CLK_ID_SDHC_0			18
#define CLK_ID_SDHC_1			19
#define CLK_ID_SDHC_2			20
#define CLK_ID_MALI			21
#define CLK_ID_UART_0			22	/* UART0_MODULE */
#define CLK_ID_UART_2			23	/* UART1_MODULE */
#define CLK_ID_UART_1			24	/* pl01115_Uart_modem_MODULE  */
#define CLK_ID_UART_3			25	/* pl01115_Uart_nodma0_MODULE */
#define CLK_ID_UART_4			26	/* pl01115_Uart_nodma1_MODULE */
#define CLK_ID_UART_5			27	/* pl01115_Uart_nodma2_MODULE */
#define CLK_ID_DIT			28
#define CLK_ID_PPM			29
#define CLK_ID_VIP_0			30	/* External Clock 1 */
#define CLK_ID_VIP_1			31	/* External Clock 1, 2 */
#define CLK_ID_USB2HOST			32	/* External Clock 2 */
#define CLK_ID_CODA			33
#define CLK_ID_CRYPTO			34
#define CLK_ID_SCALER			35
#define CLK_ID_PDM			36
#define CLK_ID_SPI_0			37
#define CLK_ID_SPI_1			38
#define CLK_ID_SPI_2			39
#define CLK_ID_MAX			39

/*
 * Nexell Reset control
 */
#define RESET_ID_AC97			0
#define RESET_ID_CPU1			1
#define RESET_ID_CPU2			2
#define RESET_ID_CPU3			3
#define RESET_ID_WD1			4
#define RESET_ID_WD2			5
#define RESET_ID_WD3			6
#define RESET_ID_CRYPTO			7
#define RESET_ID_DEINTERLACE		8
#define RESET_ID_DISP_TOP		9
#define RESET_ID_DISPLAY		10
#define RESET_ID_RESCONV		11
#define RESET_ID_LCDIF			12
#define RESET_ID_HDMI			13
#define RESET_ID_HDMI_VIDEO		14
#define RESET_ID_HDMI_SPDIF		15
#define RESET_ID_HDMI_TMDS		16
#define RESET_ID_HDMI_PHY		17
#define RESET_ID_LVDS			18
#define RESET_ID_ECID			19
#define RESET_ID_I2C0			20
#define RESET_ID_I2C1			21
#define RESET_ID_I2C2			22
#define RESET_ID_I2S0			23
#define RESET_ID_I2S1			24
#define RESET_ID_I2S2			25
#define RESET_ID_DREX_C			26
#define RESET_ID_DREX_A			27
#define RESET_ID_DREX			28
#define RESET_ID_MIPI			29
#define RESET_ID_MIPI_DSI		30
#define RESET_ID_MIPI_CSI		31
#define RESET_ID_MIPI_PHY_S		32
#define RESET_ID_MIPI_PHY_M		33
#define RESET_ID_MPEGTSI		34
#define RESET_ID_PDM			35
#define RESET_ID_TIMER			36
#define RESET_ID_PWM			37
#define RESET_ID_SCALER			38
#define RESET_ID_SDMMC0			39
#define RESET_ID_SDMMC1			40
#define RESET_ID_SDMMC2			41
#define RESET_ID_SPDIFRX		42
#define RESET_ID_SPDIFTX		43
#define RESET_ID_SSP0_P			44
#define RESET_ID_SSP0			45
#define RESET_ID_SSP1_P			46
#define RESET_ID_SSP1			47
#define RESET_ID_SSP2_P			48
#define RESET_ID_SSP2			49
#define RESET_ID_UART0			50	/* UART1 */
#define RESET_ID_UART1			51	/* pl01115_Uart_modem	*/
#define RESET_ID_UART2			52	/* UART1 */
#define RESET_ID_UART3			53	/* pl01115_Uart_nodma0 */
#define RESET_ID_UART4			54	/* pl01115_Uart_nodma1 */
#define RESET_ID_UART5			55	/* pl01115_Uart_nodma2 */
#define RESET_ID_USB20HOST		56
#define RESET_ID_USB20OTG		57
#define RESET_ID_WDT			58
#define RESET_ID_WDT_POR		59
#define RESET_ID_ADC			60
#define RESET_ID_CODA_A			61
#define RESET_ID_CODA_P			62
#define RESET_ID_CODA_C			63
#define RESET_ID_DWC_GMAC		64
#define RESET_ID_MALI400		65
#define RESET_ID_PPM			66
#define RESET_ID_VIP1			67
#define RESET_ID_VIP0			68
#if defined(CONFIG_ARCH_S5P6818)
#define RESET_ID_VIP2			69
#endif

/*
 * device name
 */
#define DEV_NAME_UART			"nx-uart" /* pl0115 (amba-pl011.c) */
#define DEV_NAME_FB			"nx-fb"
#define DEV_NAME_DISP			"nx-disp"
#define DEV_NAME_LCD			"nx-lcd"
#define DEV_NAME_LVDS			"nx-lvds"
#define DEV_NAME_HDMI			"nx-hdmi"
#define DEV_NAME_RESCONV		"nx-resconv"
#define DEV_NAME_MIPI			"nx-mipi"
#define DEV_NAME_PCM			"nx-pcm"
#define DEV_NAME_I2S			"nx-i2s"
#define DEV_NAME_SPDIF_TX		"nx-spdif-tx"
#define DEV_NAME_SPDIF_RX		"nx-spdif-rx"
#define DEV_NAME_I2C			"nx-i2c"
#define DEV_NAME_NAND			"nx-nand"
#define DEV_NAME_KEYPAD			"nx-keypad"
#define DEV_NAME_SDHC			"nx-sdhc"
#define DEV_NAME_PWM			"nx-pwm"
#define DEV_NAME_TIMER			"nx-timer"
#define DEV_NAME_SOC_PWM		"nx-soc-pwm"
#define DEV_NAME_GPIO			"nx-gpio"
#define DEV_NAME_RTC			"nx-rtc"
#define DEV_NAME_GMAC			"nx-gmac"
#define DEV_NAME_MPEGTSI		"nx-mpegtsi"
#define DEV_NAME_MALI			"nx-mali"
#define DEV_NAME_DIT			"nx-deinterlace"
#define DEV_NAME_PPM			"nx-ppm"
#define DEV_NAME_VIP			"nx-vip"
#define DEV_NAME_CODA			"nx-coda"
#define DEV_NAME_USB2HOST		"nx-usb2h"
#define DEV_NAME_CRYPTO			"nx-crypto"
#define DEV_NAME_SCALER			"nx-scaler"
#define DEV_NAME_PDM			"nx-pdm"
#define DEV_NAME_SPI			"nx-spi"
#define DEV_NAME_CPUFREQ		"nx-cpufreq"

/*
 * clock generator
 */
#define CORECLK_NAME_PLL0		"pll0"	/* cpu clock */
#define CORECLK_NAME_PLL1		"pll1"
#define CORECLK_NAME_PLL2		"pll2"
#define CORECLK_NAME_PLL3		"pll3"
#define CORECLK_NAME_FCLK		"fclk"
#define CORECLK_NAME_MCLK		"mclk"
#define CORECLK_NAME_BCLK		"bclk"
#define CORECLK_NAME_PCLK		"pclk"
#define CORECLK_NAME_HCLK		"hclk"

#define CORECLK_ID_PLL0			0
#define CORECLK_ID_PLL1			1
#define CORECLK_ID_PLL2			2
#define CORECLK_ID_PLL3			3
#define CORECLK_ID_FCLK			4
#define CORECLK_ID_MCLK			5
#define CORECLK_ID_BCLK			6
#define CORECLK_ID_PCLK			7
#define CORECLK_ID_HCLK			8

#define ALIVEPWRGATEREG			(PHY_BASEADDR_ALIVE + 0x0)

#if defined(CONFIG_ARCH_S5P4418)
#define	SCR_ARM_SECOND_BOOT		(0xC0010C1C)	 /* PWR scratch */
#define	SCR_ARM_SECOND_BOOT_REG1	(0xc0010234) /* ToDo : Check Address */
#elif defined(CONFIG_ARCH_S5P6818)
#define	SCR_ARM_SECOND_BOOT		(0xc0010230)	 /* PWR scratch */
#define	SCR_ARM_SECOND_BOOT_REG1	(0xc0010234) /* PWR scratch */
#define	SCR_ARM_SECOND_BOOT_REG2	(0xc0010238) /* PWR scratch */
#endif

#define	SCR_ALIVE_BASE			(PHY_BASEADDR_ALIVE)
#define	SCR_SIGNAGURE_RESET		(SCR_ALIVE_BASE + 0x068)
#define	SCR_SIGNAGURE_SET		(SCR_ALIVE_BASE + 0x06C)
#define	SCR_SIGNAGURE_READ		(SCR_ALIVE_BASE + 0x070)

#define SYSRSTCONFIG			(0x23C)
#define DEVICEBOOTINFO			(0x50)
#define BOOTMODE_MASK			(0x7)
#define BOOTMODE_SDMMC			5
#define BOOTMODE_USB			6
#define BOOTMODE_SDMMC_PORT_VAL(x)	((((x) >> 3) & 1) |	\
					 (((x) >> 19 & 1) << 1))
#define EMMC_PORT_NUM			2
#define SD_PORT_NUM			0
#define ID_REG_EC0				(0x54)
#define WIRE0_MASK				(0x1)

#ifndef __ASSEMBLY__

#define NS_IN_HZ (1000000000UL)
#define TO_PERIOD_NS(freq)	(NS_IN_HZ / (freq))
#define TO_DUTY_NS(duty, freq)  (duty ? TO_PERIOD_NS(freq) / (100 / duty) : 0)

#endif	/* __ASSEMBLY__ */

#endif /* __NEXELL_H__ */
