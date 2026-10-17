/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (c) 2007 Pengutronix, Sascha Hauer <s.hauer@pengutronix.de>
 * (c) 2009 Ilya Yanok, Emcraft Systems <yanok@emcraft.com>
 */

#ifndef _IMX_REGS_H
#define _IMX_REGS_H

#include <asm/arch/regs-rtc.h>

#ifndef __ASSEMBLY__

extern void imx_gpio_mode (int gpio_mode);

#ifdef CONFIG_MXC_UART
extern void mx27_uart1_init_pins(void);
#endif /* CONFIG_MXC_UART */

#ifdef CONFIG_FEC_MXC
extern void mx27_fec_init_pins(void);
#endif /* CONFIG_FEC_MXC */

#ifdef CONFIG_MMC_MXC
extern void mx27_sd1_init_pins(void);
extern void mx27_sd2_init_pins(void);
#endif /* CONFIG_MMC_MXC */

/* AIPI */
struct aipi_regs {
	u32 psr0;
	u32 psr1;
};

/* System Control */
struct system_control_regs {
	u32 res[5];
	u32 fmcr;
	u32 gpcr;
	u32 wbcr;
	u32 dscr1;
	u32 dscr2;
	u32 dscr3;
	u32 dscr4;
	u32 dscr5;
	u32 dscr6;
	u32 dscr7;
	u32 dscr8;
	u32 dscr9;
	u32 dscr10;
	u32 dscr11;
	u32 dscr12;
	u32 dscr13;
	u32 pscr;
	u32 pmcr;
	u32 res1;
	u32 dcvr0;
	u32 dcvr1;
	u32 dcvr2;
	u32 dcvr3;
};

/* Chip Select Registers */
struct weim_regs {
	u32 cs0u;	/* Chip Select 0 Upper Register */
	u32 cs0l;	/* Chip Select 0 Lower Register */
	u32 cs0a;	/* Chip Select 0 Addition Register */
	u32 pad0;
	u32 cs1u;	/* Chip Select 1 Upper Register */
	u32 cs1l;	/* Chip Select 1 Lower Register */
	u32 cs1a;	/* Chip Select 1 Addition Register */
	u32 pad1;
	u32 cs2u;	/* Chip Select 2 Upper Register */
	u32 cs2l;	/* Chip Select 2 Lower Register */
	u32 cs2a;	/* Chip Select 2 Addition Register */
	u32 pad2;
	u32 cs3u;	/* Chip Select 3 Upper Register */
	u32 cs3l;	/* Chip Select 3 Lower Register */
	u32 cs3a;	/* Chip Select 3 Addition Register */
	u32 pad3;
	u32 cs4u;	/* Chip Select 4 Upper Register */
	u32 cs4l;	/* Chip Select 4 Lower Register */
	u32 cs4a;	/* Chip Select 4 Addition Register */
	u32 pad4;
	u32 cs5u;	/* Chip Select 5 Upper Register */
	u32 cs5l;	/* Chip Select 5 Lower Register */
	u32 cs5a;	/* Chip Select 5 Addition Register */
	u32 pad5;
	u32 eim;	/* WEIM Configuration Register */
};

/* SDRAM Controller registers */
struct esdramc_regs {
/* Enhanced SDRAM Control Register 0 */
	u32 esdctl0;
/* Enhanced SDRAM Configuration Register 0 */
	u32 esdcfg0;
/* Enhanced SDRAM Control Register 1 */
	u32 esdctl1;
/* Enhanced SDRAM Configuration Register 1 */
	u32 esdcfg1;
/* Enhanced SDRAM Miscellanious Register */
	u32 esdmisc;
};

/* Watchdog Registers*/
struct wdog_regs {
	u16 wcr;
	u16 wsr;
	u16 wstr;
};

/* PLL registers */
struct pll_regs {
	u32 cscr;	/* Clock Source Control Register */
	u32 mpctl0;	/* MCU PLL Control Register 0 */
	u32 mpctl1;	/* MCU PLL Control Register 1 */
	u32 spctl0;	/* System PLL Control Register 0 */
	u32 spctl1;	/* System PLL Control Register 1 */
	u32 osc26mctl;	/* Oscillator 26M Register */
	u32 pcdr0;	/* Peripheral Clock Divider Register 0 */
	u32 pcdr1;	/* Peripheral Clock Divider Register 1 */
	u32 pccr0;	/* Peripheral Clock Control Register 0 */
	u32 pccr1;	/* Peripheral Clock Control Register 1 */
	u32 ccsr;	/* Clock Control Status Register */
};

/*
 * Definitions for the clocksource registers
 */
struct gpt_regs {
	u32 gpt_tctl;
	u32 gpt_tprer;
	u32 gpt_tcmp;
	u32 gpt_tcr;
	u32 gpt_tcn;
	u32 gpt_tstat;
};

/* IIM Control Registers */
struct iim_regs {
	u32 iim_stat;
	u32 iim_statm;
	u32 iim_err;
	u32 iim_emask;
	u32 iim_fctl;
	u32 iim_ua;
	u32 iim_la;
	u32 iim_sdat;
	u32 iim_prev;
	u32 iim_srev;
	u32 iim_prg_p;
	u32 iim_scs0;
	u32 iim_scs1;
	u32 iim_scs2;
	u32 iim_scs3;
	u32 res[0x1f1];
	struct fuse_bank {
		u32 fuse_regs[0x20];
		u32 fuse_rsvd[0xe0];
	} bank[2];
};

struct fuse_bank0_regs {
	u32 fuse0_3[5];
	u32 mac_addr[6];
	u32 fuse10_31[0x16];
};

#endif

#define ARCH_MXC

#define IMX_IO_BASE		0x10000000

#define IMX_AIPI1_BASE		(0x00000 + IMX_IO_BASE)
#define IMX_WDT_BASE		(0x02000 + IMX_IO_BASE)
#define IMX_TIM1_BASE		(0x03000 + IMX_IO_BASE)
#define IMX_TIM2_BASE		(0x04000 + IMX_IO_BASE)
#define IMX_TIM3_BASE		(0x05000 + IMX_IO_BASE)
#define IMX_RTC_BASE		(0x07000 + IMX_IO_BASE)
#define UART1_BASE_ADDR		(0x0a000 + IMX_IO_BASE)
#define UART2_BASE_ADDR		(0x0b000 + IMX_IO_BASE)
#define UART3_BASE_ADDR		(0x0c000 + IMX_IO_BASE)
#define UART4_BASE_ADDR		(0x0d000 + IMX_IO_BASE)
#define I2C1_BASE_ADDR		(0x12000 + IMX_IO_BASE)
#define IMX_GPIO_BASE		(0x15000 + IMX_IO_BASE)
#define IMX_TIM4_BASE		(0x19000 + IMX_IO_BASE)
#define IMX_TIM5_BASE		(0x1a000 + IMX_IO_BASE)
#define UART5_BASE_ADDR		(0x1b000 + IMX_IO_BASE)
#define UART6_BASE_ADDR		(0x1c000 + IMX_IO_BASE)
#define I2C2_BASE_ADDR		(0x1D000 + IMX_IO_BASE)
#define IMX_TIM6_BASE		(0x1f000 + IMX_IO_BASE)
#define IMX_AIPI2_BASE		(0x20000 + IMX_IO_BASE)
#define IMX_PLL_BASE		(0x27000 + IMX_IO_BASE)
#define IMX_SYSTEM_CTL_BASE	(0x27800 + IMX_IO_BASE)
#define IMX_IIM_BASE		(0x28000 + IMX_IO_BASE)
#define IIM_BASE_ADDR		IMX_IIM_BASE
#define IMX_FEC_BASE		(0x2b000 + IMX_IO_BASE)

#define IMX_NFC_BASE		(0xD8000000)
#define IMX_ESD_BASE		(0xD8001000)
#define IMX_WEIM_BASE		(0xD8002000)

#define NFC_BASE_ADDR		IMX_NFC_BASE

#define UART_BASE_ADDR(n)	(			\
	!!sizeof(struct {				\
		static_assert((n) >= 1 && (n) <= 6);	\
		int pad;				\
		}) * (					\
	(n) == 1 ? UART1_BASE_ADDR :			\
	(n) == 2 ? UART2_BASE_ADDR :			\
	(n) == 3 ? UART3_BASE_ADDR :			\
	(n) == 4 ? UART4_BASE_ADDR :			\
	(n) == 5 ? UART5_BASE_ADDR :			\
	UART6_BASE_ADDR)				\
	)

/* FMCR System Control bit definition*/
#define UART4_RXD_CTL	(1 << 25)
#define UART4_RTS_CTL	(1 << 24)
#define KP_COL6_CTL	(1 << 18)
#define KP_ROW7_CTL	(1 << 17)
#define KP_ROW6_CTL	(1 << 16)
#define PC_WAIT_B_CTL	(1 << 14)
#define PC_READY_CTL	(1 << 13)
#define PC_VS1_CTL	(1 << 12)
#define PC_VS2_CTL	(1 << 11)
#define PC_BVD1_CTL	(1 << 10)
#define PC_BVD2_CTL	(1 << 9)
#define IOS16_CTL	(1 << 8)
#define NF_FMS		(1 << 5)
#define NF_16BIT_SEL	(1 << 4)
#define SLCDC_SEL	(1 << 2)
#define SDCS1_SEL	(1 << 1)
#define SDCS0_SEL	(1 << 0)


/* important definition of some bits of WCR */
#define WCR_WDE 0x04

#define CSCR_MPEN		(1 << 0)
#define CSCR_SPEN		(1 << 1)
#define CSCR_FPM_EN		(1 << 2)
#define CSCR_OSC26M_DIS		(1 << 3)
#define CSCR_OSC26M_DIV1P5	(1 << 4)
#define CSCR_AHB_DIV
#define CSCR_ARM_DIV
#define CSCR_ARM_SRC_MPLL	(1 << 15)
#define CSCR_MCU_SEL		(1 << 16)
#define CSCR_SP_SEL		(1 << 17)
#define CSCR_MPLL_RESTART	(1 << 18)
#define CSCR_SPLL_RESTART	(1 << 19)
#define CSCR_MSHC_SEL		(1 << 20)
#define CSCR_H264_SEL		(1 << 21)
#define CSCR_SSI1_SEL		(1 << 22)
#define CSCR_SSI2_SEL		(1 << 23)
#define CSCR_SD_CNT
#define CSCR_USB_DIV
#define CSCR_UPDATE_DIS		(1 << 31)

#define MPCTL1_BRMO		(1 << 6)
#define MPCTL1_LF		(1 << 15)

#define PCCR0_SSI2_EN	(1 << 0)
#define PCCR0_SSI1_EN	(1 << 1)
#define PCCR0_SLCDC_EN	(1 << 2)
#define PCCR0_SDHC3_EN	(1 << 3)
#define PCCR0_SDHC2_EN	(1 << 4)
#define PCCR0_SDHC1_EN	(1 << 5)
#define PCCR0_SDC_EN	(1 << 6)
#define PCCR0_SAHARA_EN	(1 << 7)
#define PCCR0_RTIC_EN	(1 << 8)
#define PCCR0_RTC_EN	(1 << 9)
#define PCCR0_PWM_EN	(1 << 11)
#define PCCR0_OWIRE_EN	(1 << 12)
#define PCCR0_MSHC_EN	(1 << 13)
#define PCCR0_LCDC_EN	(1 << 14)
#define PCCR0_KPP_EN	(1 << 15)
#define PCCR0_IIM_EN	(1 << 16)
#define PCCR0_I2C2_EN	(1 << 17)
#define PCCR0_I2C1_EN	(1 << 18)
#define PCCR0_GPT6_EN	(1 << 19)
#define PCCR0_GPT5_EN	(1 << 20)
#define PCCR0_GPT4_EN	(1 << 21)
#define PCCR0_GPT3_EN	(1 << 22)
#define PCCR0_GPT2_EN	(1 << 23)
#define PCCR0_GPT1_EN	(1 << 24)
#define PCCR0_GPIO_EN	(1 << 25)
#define PCCR0_FEC_EN	(1 << 26)
#define PCCR0_EMMA_EN	(1 << 27)
#define PCCR0_DMA_EN	(1 << 28)
#define PCCR0_CSPI3_EN	(1 << 29)
#define PCCR0_CSPI2_EN	(1 << 30)
#define PCCR0_CSPI1_EN	(1 << 31)

#define PCCR1_MSHC_BAUDEN	(1 << 2)
#define PCCR1_NFC_BAUDEN	(1 << 3)
#define PCCR1_SSI2_BAUDEN	(1 << 4)
#define PCCR1_SSI1_BAUDEN	(1 << 5)
#define PCCR1_H264_BAUDEN	(1 << 6)
#define PCCR1_PERCLK4_EN	(1 << 7)
#define PCCR1_PERCLK3_EN	(1 << 8)
#define PCCR1_PERCLK2_EN	(1 << 9)
#define PCCR1_PERCLK1_EN	(1 << 10)
#define PCCR1_HCLK_USB		(1 << 11)
#define PCCR1_HCLK_SLCDC	(1 << 12)
#define PCCR1_HCLK_SAHARA	(1 << 13)
#define PCCR1_HCLK_RTIC		(1 << 14)
#define PCCR1_HCLK_LCDC		(1 << 15)
#define PCCR1_HCLK_H264		(1 << 16)
#define PCCR1_HCLK_FEC		(1 << 17)
#define PCCR1_HCLK_EMMA		(1 << 18)
#define PCCR1_HCLK_EMI		(1 << 19)
#define PCCR1_HCLK_DMA		(1 << 20)
#define PCCR1_HCLK_CSI		(1 << 21)
#define PCCR1_HCLK_BROM		(1 << 22)
#define PCCR1_HCLK_ATA		(1 << 23)
#define PCCR1_WDT_EN		(1 << 24)
#define PCCR1_USB_EN		(1 << 25)
#define PCCR1_UART6_EN		(1 << 26)
#define PCCR1_UART5_EN		(1 << 27)
#define PCCR1_UART4_EN		(1 << 28)
#define PCCR1_UART3_EN		(1 << 29)
#define PCCR1_UART2_EN		(1 << 30)
#define PCCR1_UART1_EN		(1 << 31)

/* SDRAM Controller registers bitfields */
#define ESDCTL_PRCT(x)		(((x) & 0x3f) << 0)
#define ESDCTL_BL		(1 << 7)
#define ESDCTL_FP		(1 << 8)
#define ESDCTL_PWDT(x)		(((x) & 3) << 10)
#define ESDCTL_SREFR(x)		(((x) & 7) << 13)
#define ESDCTL_DSIZ_16_UPPER	(0 << 16)
#define ESDCTL_DSIZ_16_LOWER	(1 << 16)
#define ESDCTL_DSIZ_32		(2 << 16)
#define ESDCTL_COL8		(0 << 20)
#define ESDCTL_COL9		(1 << 20)
#define ESDCTL_COL10		(2 << 20)
#define ESDCTL_ROW11		(0 << 24)
#define ESDCTL_ROW12		(1 << 24)
#define ESDCTL_ROW13		(2 << 24)
#define ESDCTL_ROW14		(3 << 24)
#define ESDCTL_ROW15		(4 << 24)
#define ESDCTL_SP		(1 << 27)
#define ESDCTL_SMODE_NORMAL	(0 << 28)
#define ESDCTL_SMODE_PRECHARGE	(1 << 28)
#define ESDCTL_SMODE_AUTO_REF	(2 << 28)
#define ESDCTL_SMODE_LOAD_MODE	(3 << 28)
#define ESDCTL_SMODE_MAN_REF	(4 << 28)
#define ESDCTL_SDE		(1 << 31)

#define ESDCFG_TRC(x)		(((x) & 0xf) << 0)
#define ESDCFG_TRCD(x)		(((x) & 0x7) << 4)
#define ESDCFG_TCAS(x)		(((x) & 0x3) << 8)
#define ESDCFG_TRRD(x)		(((x) & 0x3) << 10)
#define ESDCFG_TRAS(x)		(((x) & 0x7) << 12)
#define ESDCFG_TWR		(1 << 15)
#define ESDCFG_TMRD(x)		(((x) & 0x3) << 16)
#define ESDCFG_TRP(x)		(((x) & 0x3) << 18)
#define ESDCFG_TWTR		(1 << 20)
#define ESDCFG_TXP(x)		(((x) & 0x3) << 21)

#define ESDMISC_RST		(1 << 1)
#define ESDMISC_MDDREN		(1 << 2)
#define ESDMISC_MDDR_DL_RST	(1 << 3)
#define ESDMISC_MDDR_MDIS	(1 << 4)
#define ESDMISC_LHD		(1 << 5)
#define ESDMISC_MA10_SHARE	(1 << 6)
#define ESDMISC_SDRAM_RDY	(1 << 31)

#define PC5_PF_I2C2_DATA	(GPIO_PORTC | GPIO_OUT | GPIO_PF | 5)
#define PC6_PF_I2C2_CLK		(GPIO_PORTC | GPIO_OUT | GPIO_PF | 6)
#define PC7_PF_USBOTG_DATA5	(GPIO_PORTC | GPIO_OUT | GPIO_PF | 7)
#define PC8_PF_USBOTG_DATA6	(GPIO_PORTC | GPIO_OUT | GPIO_PF | 8)
#define PC9_PF_USBOTG_DATA0	(GPIO_PORTC | GPIO_OUT | GPIO_PF | 9)
#define PC10_PF_USBOTG_DATA2	(GPIO_PORTC | GPIO_OUT | GPIO_PF | 10)
#define PC11_PF_USBOTG_DATA1	(GPIO_PORTC | GPIO_OUT | GPIO_PF | 11)
#define PC12_PF_USBOTG_DATA4	(GPIO_PORTC | GPIO_OUT | GPIO_PF | 12)
#define PC13_PF_USBOTG_DATA3	(GPIO_PORTC | GPIO_OUT | GPIO_PF | 13)

#define PD0_AIN_FEC_TXD0	(GPIO_PORTD | GPIO_OUT | GPIO_AIN | 0)
#define PD1_AIN_FEC_TXD1	(GPIO_PORTD | GPIO_OUT | GPIO_AIN | 1)
#define PD2_AIN_FEC_TXD2	(GPIO_PORTD | GPIO_OUT | GPIO_AIN | 2)
#define PD3_AIN_FEC_TXD3	(GPIO_PORTD | GPIO_OUT | GPIO_AIN | 3)
#define PD4_AOUT_FEC_RX_ER	(GPIO_PORTD | GPIO_IN | GPIO_AOUT | 4)
#define PD5_AOUT_FEC_RXD1	(GPIO_PORTD | GPIO_IN | GPIO_AOUT | 5)
#define PD6_AOUT_FEC_RXD2	(GPIO_PORTD | GPIO_IN | GPIO_AOUT | 6)
#define PD7_AOUT_FEC_RXD3	(GPIO_PORTD | GPIO_IN | GPIO_AOUT | 7)
#define PD8_AF_FEC_MDIO		(GPIO_PORTD | GPIO_IN | GPIO_AF | 8)
#define PD9_AIN_FEC_MDC		(GPIO_PORTD | GPIO_OUT | GPIO_AIN | 9)
#define PD10_AOUT_FEC_CRS	(GPIO_PORTD | GPIO_IN | GPIO_AOUT | 10)
#define PD11_AOUT_FEC_TX_CLK	(GPIO_PORTD | GPIO_IN | GPIO_AOUT | 11)
#define PD12_AOUT_FEC_RXD0	(GPIO_PORTD | GPIO_IN | GPIO_AOUT | 12)
#define PD13_AOUT_FEC_RX_DV	(GPIO_PORTD | GPIO_IN | GPIO_AOUT | 13)
#define PD14_AOUT_FEC_CLR	(GPIO_PORTD | GPIO_IN | GPIO_AOUT | 14)
#define PD15_AOUT_FEC_COL	(GPIO_PORTD | GPIO_IN | GPIO_AOUT | 15)
#define PD16_AIN_FEC_TX_ER	(GPIO_PORTD | GPIO_OUT | GPIO_AIN | 16)
#define PF23_AIN_FEC_TX_EN	(GPIO_PORTF | GPIO_OUT | GPIO_AIN | 23)

#define PE0_PF_USBOTG_NXT	(GPIO_PORTE | GPIO_OUT | GPIO_PF | 0)
#define PE1_PF_USBOTG_STP	(GPIO_PORTE | GPIO_OUT | GPIO_PF | 1)
#define PE2_PF_USBOTG_DIR	(GPIO_PORTE | GPIO_OUT | GPIO_PF | 2)
#define PE3_PF_UART2_CTS	(GPIO_PORTE | GPIO_OUT | GPIO_PF | 3)
#define PE4_PF_UART2_RTS	(GPIO_PORTE | GPIO_IN  | GPIO_PF | 4)
#define PE6_PF_UART2_TXD	(GPIO_PORTE | GPIO_OUT | GPIO_PF | 6)
#define PE7_PF_UART2_RXD	(GPIO_PORTE | GPIO_IN  | GPIO_PF | 7)
#define PE8_PF_UART3_TXD	(GPIO_PORTE | GPIO_OUT | GPIO_PF | 8)
#define PE9_PF_UART3_RXD	(GPIO_PORTE | GPIO_IN  | GPIO_PF | 9)
#define PE10_PF_UART3_CTS	(GPIO_PORTE | GPIO_OUT | GPIO_PF | 10)
#define PE11_PF_UART3_RTS	(GPIO_PORTE | GPIO_IN  | GPIO_PF | 11)
#define PE12_PF_UART1_TXD	(GPIO_PORTE | GPIO_OUT | GPIO_PF | 12)
#define PE13_PF_UART1_RXD	(GPIO_PORTE | GPIO_IN  | GPIO_PF | 13)
#define PE14_PF_UART1_CTS	(GPIO_PORTE | GPIO_OUT | GPIO_PF | 14)
#define PE15_PF_UART1_RTS	(GPIO_PORTE | GPIO_IN  | GPIO_PF | 15)
#define PE18_PF_SD1_D0		(GPIO_PORTE | GPIO_PF | 18)
#define PE19_PF_SD1_D1		(GPIO_PORTE | GPIO_PF | 19)
#define PE20_PF_SD1_D2		(GPIO_PORTE | GPIO_PF | 20)
#define PE21_PF_SD1_D3		(GPIO_PORTE | GPIO_PF | 21)
#define PE22_PF_SD1_CMD		(GPIO_PORTE | GPIO_PF | 22)
#define PE23_PF_SD1_CLK		(GPIO_PORTE | GPIO_PF | 23)
#define PB4_PF_SD2_D0		(GPIO_PORTB | GPIO_PF | 4)
#define PB5_PF_SD2_D1		(GPIO_PORTB | GPIO_PF | 5)
#define PB6_PF_SD2_D2		(GPIO_PORTB | GPIO_PF | 6)
#define PB7_PF_SD2_D3		(GPIO_PORTB | GPIO_PF | 7)
#define PB8_PF_SD2_CMD		(GPIO_PORTB | GPIO_PF | 8)
#define PB9_PF_SD2_CLK		(GPIO_PORTB | GPIO_PF | 9)
#define PD17_PF_I2C_DATA	(GPIO_PORTD | GPIO_OUT | GPIO_PF | 17)
#define PD18_PF_I2C_CLK		(GPIO_PORTD | GPIO_OUT | GPIO_PF | 18)
#define PE24_PF_USBOTG_CLK	(GPIO_PORTE | GPIO_OUT | GPIO_PF | 24)
#define PE25_PF_USBOTG_DATA7	(GPIO_PORTE | GPIO_OUT | GPIO_PF | 25)

/* Clocksource Bitfields */
#define TCTL_SWR	(1 << 15)	/* Software reset */
#define TCTL_FRR	(1 << 8)	/* Freerun / restart */
#define TCTL_CAP	(3 << 6)	/* Capture Edge */
#define TCTL_OM		(1 << 5)	/* output mode */
#define TCTL_IRQEN	(1 << 4)	/* interrupt enable */
#define TCTL_CLKSOURCE	1		/* Clock source bit position */
#define TCTL_TEN	1		/* Timer enable */
#define TPRER_PRES	0xff		/* Prescale */
#define TSTAT_CAPT	(1 << 1)	/* Capture event */
#define TSTAT_COMP	1		/* Compare event */

#define GPIO1_BASE_ADDR 0x10015000
#define GPIO2_BASE_ADDR 0x10015100
#define GPIO3_BASE_ADDR 0x10015200
#define GPIO4_BASE_ADDR 0x10015300
#define GPIO5_BASE_ADDR 0x10015400
#define GPIO6_BASE_ADDR 0x10015500

#define GPIO_OUT	(1 << 8)
#define GPIO_IN		(0 << 8)
#define GPIO_PUEN	(1 << 9)

#define GPIO_PF		(1 << 10)
#define GPIO_AF		(1 << 11)

#define GPIO_OCR_SHIFT	12
#define GPIO_OCR_MASK	(3 << GPIO_OCR_SHIFT)
#define GPIO_AIN	(0 << GPIO_OCR_SHIFT)
#define GPIO_BIN	(1 << GPIO_OCR_SHIFT)
#define GPIO_CIN	(2 << GPIO_OCR_SHIFT)
#define GPIO_GPIO	(3 << GPIO_OCR_SHIFT)

#define GPIO_AOUT_SHIFT	14
#define GPIO_AOUT_MASK	(3 << GPIO_AOUT_SHIFT)
#define GPIO_AOUT	(0 << GPIO_AOUT_SHIFT)
#define GPIO_AOUT_ISR	(1 << GPIO_AOUT_SHIFT)
#define GPIO_AOUT_0	(2 << GPIO_AOUT_SHIFT)
#define GPIO_AOUT_1	(3 << GPIO_AOUT_SHIFT)

#define GPIO_BOUT_SHIFT	16
#define GPIO_BOUT_MASK	(3 << GPIO_BOUT_SHIFT)
#define GPIO_BOUT	(0 << GPIO_BOUT_SHIFT)
#define GPIO_BOUT_ISR	(1 << GPIO_BOUT_SHIFT)
#define GPIO_BOUT_0	(2 << GPIO_BOUT_SHIFT)
#define GPIO_BOUT_1	(3 << GPIO_BOUT_SHIFT)

#define IIM_STAT_BUSY	(1 << 7)
#define IIM_STAT_PRGD	(1 << 1)
#define IIM_STAT_SNSD	(1 << 0)
#define IIM_ERR_PRGE	(1 << 7)
#define IIM_ERR_WPE	(1 << 6)
#define IIM_ERR_OPE	(1 << 5)
#define IIM_ERR_RPE	(1 << 4)
#define IIM_ERR_WLRE	(1 << 3)
#define IIM_ERR_SNSE	(1 << 2)
#define IIM_ERR_PARITYE	(1 << 1)

#endif				/* _IMX_REGS_H */
