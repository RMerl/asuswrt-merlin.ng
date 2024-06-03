/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2007 Sergey Kubushyn <ksi@koi8.net>
 *
 * Based on:
 *
 * -------------------------------------------------------------------------
 *
 *  linux/include/asm-arm/arch-davinci/hardware.h
 *
 *  Copyright (C) 2006 Texas Instruments.
 */
#ifndef __ASM_ARCH_HARDWARE_H
#define __ASM_ARCH_HARDWARE_H

#include <linux/sizes.h>

#define	REG(addr)	(*(volatile unsigned int *)(addr))
#define REG_P(addr)	((volatile unsigned int *)(addr))

#ifndef __ASSEMBLY__
typedef volatile unsigned int	dv_reg;
typedef volatile unsigned int *	dv_reg_p;
#endif

#define DAVINCI_UART0_BASE			0x01c42000
#define DAVINCI_UART1_BASE			0x01d0c000
#define DAVINCI_UART2_BASE			0x01d0d000
#define DAVINCI_I2C0_BASE			0x01c22000
#define DAVINCI_I2C1_BASE			0x01e28000
#define DAVINCI_TIMER0_BASE			0x01c20000
#define DAVINCI_TIMER1_BASE			0x01c21000
#define DAVINCI_WDOG_BASE			0x01c21000
#define DAVINCI_RTC_BASE			0x01c23000
#define DAVINCI_PLL_CNTRL0_BASE			0x01c11000
#define DAVINCI_PLL_CNTRL1_BASE			0x01e1a000
#define DAVINCI_PSC0_BASE			0x01c10000
#define DAVINCI_PSC1_BASE			0x01e27000
#define DAVINCI_SPI0_BASE			0x01c41000
#define DAVINCI_USB_OTG_BASE			0x01e00000
#define DAVINCI_SPI1_BASE			(cpu_is_da830() ? \
						0x01e12000 : 0x01f0e000)
#define DAVINCI_GPIO_BASE			0x01e26000
#define DAVINCI_EMAC_CNTRL_REGS_BASE		0x01e23000
#define DAVINCI_EMAC_WRAPPER_CNTRL_REGS_BASE	0x01e22000
#define DAVINCI_EMAC_WRAPPER_RAM_BASE		0x01e20000
#define DAVINCI_MDIO_CNTRL_REGS_BASE		0x01e24000
#define DAVINCI_SYSCFG1_BASE			0x01e2c000
#define DAVINCI_MMC_SD0_BASE			0x01c40000
#define DAVINCI_MMC_SD1_BASE			0x01e1b000
#define DAVINCI_TIMER2_BASE			0x01f0c000
#define DAVINCI_TIMER3_BASE			0x01f0d000
#define DAVINCI_ASYNC_EMIF_CNTRL_BASE		0x68000000
#define DAVINCI_ASYNC_EMIF_DATA_CE0_BASE	0x40000000
#define DAVINCI_ASYNC_EMIF_DATA_CE2_BASE	0x60000000
#define DAVINCI_ASYNC_EMIF_DATA_CE3_BASE	0x62000000
#define DAVINCI_ASYNC_EMIF_DATA_CE4_BASE	0x64000000
#define DAVINCI_ASYNC_EMIF_DATA_CE5_BASE	0x66000000
#define DAVINCI_DDR_EMIF_CTRL_BASE		0xb0000000
#define DAVINCI_DDR_EMIF_DATA_BASE		0xc0000000
#define DAVINCI_INTC_BASE			0xfffee000
#define DAVINCI_BOOTCFG_BASE			0x01c14000
#define DAVINCI_LCD_CNTL_BASE			0x01e13000
#define DAVINCI_L3CBARAM_BASE			0x80000000
#define JTAG_ID_REG                            (DAVINCI_BOOTCFG_BASE + 0x18)
#define CHIP_REV_ID_REG				(DAVINCI_BOOTCFG_BASE + 0x24)
#define HOST1CFG				(DAVINCI_BOOTCFG_BASE + 0x44)
#define PSC0_MDCTL				(DAVINCI_PSC0_BASE + 0xa00)

#define GPIO_BANK0_REG_DIR_ADDR			(DAVINCI_GPIO_BASE + 0x10)
#define GPIO_BANK0_REG_OPDATA_ADDR		(DAVINCI_GPIO_BASE + 0x14)
#define GPIO_BANK0_REG_SET_ADDR			(DAVINCI_GPIO_BASE + 0x18)
#define GPIO_BANK0_REG_CLR_ADDR			(DAVINCI_GPIO_BASE + 0x1c)
#define GPIO_BANK2_REG_DIR_ADDR			(DAVINCI_GPIO_BASE + 0x38)
#define GPIO_BANK2_REG_OPDATA_ADDR		(DAVINCI_GPIO_BASE + 0x3c)
#define GPIO_BANK2_REG_SET_ADDR			(DAVINCI_GPIO_BASE + 0x40)
#define GPIO_BANK2_REG_CLR_ADDR			(DAVINCI_GPIO_BASE + 0x44)
#define GPIO_BANK6_REG_DIR_ADDR			(DAVINCI_GPIO_BASE + 0x88)
#define GPIO_BANK6_REG_OPDATA_ADDR		(DAVINCI_GPIO_BASE + 0x8c)
#define GPIO_BANK6_REG_SET_ADDR			(DAVINCI_GPIO_BASE + 0x90)
#define GPIO_BANK6_REG_CLR_ADDR			(DAVINCI_GPIO_BASE + 0x94)

/* Power and Sleep Controller (PSC) Domains */
#define DAVINCI_GPSC_ARMDOMAIN		0
#define DAVINCI_GPSC_DSPDOMAIN		1

#define DAVINCI_LPSC_TPCC		0
#define DAVINCI_LPSC_TPTC0		1
#define DAVINCI_LPSC_TPTC1		2
#define DAVINCI_LPSC_AEMIF		3
#define DAVINCI_LPSC_SPI0		4
#define DAVINCI_LPSC_MMC_SD		5
#define DAVINCI_LPSC_AINTC		6
#define DAVINCI_LPSC_ARM_RAM_ROM	7
#define DAVINCI_LPSC_SECCTL_KEYMGR	8
#define DAVINCI_LPSC_UART0		9
#define DAVINCI_LPSC_SCR0		10
#define DAVINCI_LPSC_SCR1		11
#define DAVINCI_LPSC_SCR2		12
#define DAVINCI_LPSC_DMAX		13
#define DAVINCI_LPSC_ARM		14
#define DAVINCI_LPSC_GEM		15

/* for LPSCs in PSC1, offset from 32 for differentiation */
#define DAVINCI_LPSC_PSC1_BASE		32
#define DAVINCI_LPSC_USB20		(DAVINCI_LPSC_PSC1_BASE + 1)
#define DAVINCI_LPSC_USB11		(DAVINCI_LPSC_PSC1_BASE + 2)
#define DAVINCI_LPSC_GPIO		(DAVINCI_LPSC_PSC1_BASE + 3)
#define DAVINCI_LPSC_UHPI		(DAVINCI_LPSC_PSC1_BASE + 4)
#define DAVINCI_LPSC_EMAC		(DAVINCI_LPSC_PSC1_BASE + 5)
#define DAVINCI_LPSC_DDR_EMIF		(DAVINCI_LPSC_PSC1_BASE + 6)
#define DAVINCI_LPSC_McASP0		(DAVINCI_LPSC_PSC1_BASE + 7)
#define DAVINCI_LPSC_SPI1		(DAVINCI_LPSC_PSC1_BASE + 10)
#define DAVINCI_LPSC_I2C1		(DAVINCI_LPSC_PSC1_BASE + 11)
#define DAVINCI_LPSC_UART1		(DAVINCI_LPSC_PSC1_BASE + 12)
#define DAVINCI_LPSC_UART2		(DAVINCI_LPSC_PSC1_BASE + 13)
#define DAVINCI_LPSC_LCDC		(DAVINCI_LPSC_PSC1_BASE + 16)
#define DAVINCI_LPSC_ePWM		(DAVINCI_LPSC_PSC1_BASE + 17)
#define DAVINCI_LPSC_MMCSD1		(DAVINCI_LPSC_PSC1_BASE + 18)
#define DAVINCI_LPSC_eCAP		(DAVINCI_LPSC_PSC1_BASE + 20)
#define DAVINCI_LPSC_L3_CBA_RAM		(DAVINCI_LPSC_PSC1_BASE + 31)

/* DA830-specific peripherals */
#define DAVINCI_LPSC_McASP1		(DAVINCI_LPSC_PSC1_BASE + 8)
#define DAVINCI_LPSC_McASP2		(DAVINCI_LPSC_PSC1_BASE + 9)
#define DAVINCI_LPSC_eQEP		(DAVINCI_LPSC_PSC1_BASE + 21)
#define DAVINCI_LPSC_SCR8		(DAVINCI_LPSC_PSC1_BASE + 24)
#define DAVINCI_LPSC_SCR7		(DAVINCI_LPSC_PSC1_BASE + 25)
#define DAVINCI_LPSC_SCR12		(DAVINCI_LPSC_PSC1_BASE + 26)

/* DA850-specific peripherals */
#define DAVINCI_LPSC_TPCC1		(DAVINCI_LPSC_PSC1_BASE + 0)
#define DAVINCI_LPSC_SATA		(DAVINCI_LPSC_PSC1_BASE + 8)
#define DAVINCI_LPSC_VPIF		(DAVINCI_LPSC_PSC1_BASE + 9)
#define DAVINCI_LPSC_McBSP0		(DAVINCI_LPSC_PSC1_BASE + 14)
#define DAVINCI_LPSC_McBSP1		(DAVINCI_LPSC_PSC1_BASE + 15)
#define DAVINCI_LPSC_MMC_SD1		(DAVINCI_LPSC_PSC1_BASE + 18)
#define DAVINCI_LPSC_uPP		(DAVINCI_LPSC_PSC1_BASE + 19)
#define DAVINCI_LPSC_TPTC2		(DAVINCI_LPSC_PSC1_BASE + 21)
#define DAVINCI_LPSC_SCR_F0		(DAVINCI_LPSC_PSC1_BASE + 24)
#define DAVINCI_LPSC_SCR_F1		(DAVINCI_LPSC_PSC1_BASE + 25)
#define DAVINCI_LPSC_SCR_F2		(DAVINCI_LPSC_PSC1_BASE + 26)
#define DAVINCI_LPSC_SCR_F6		(DAVINCI_LPSC_PSC1_BASE + 27)
#define DAVINCI_LPSC_SCR_F7		(DAVINCI_LPSC_PSC1_BASE + 28)
#define DAVINCI_LPSC_SCR_F8		(DAVINCI_LPSC_PSC1_BASE + 29)
#define DAVINCI_LPSC_BR_F7		(DAVINCI_LPSC_PSC1_BASE + 30)

#ifndef __ASSEMBLY__
void lpsc_on(unsigned int id);
void lpsc_syncreset(unsigned int id);
void lpsc_disable(unsigned int id);
void dsp_on(void);

void davinci_enable_uart0(void);
void davinci_enable_emac(void);
void davinci_enable_i2c(void);
void davinci_errata_workarounds(void);

#define	PSC_ENABLE		0x3
#define	PSC_DISABLE		0x2
#define	PSC_SYNCRESET		0x1
#define	PSC_SWRSTDISABLE	0x0

#define PSC_PSC0_MODULE_ID_CNT		16
#define PSC_PSC1_MODULE_ID_CNT		32

#define UART0_PWREMU_MGMT		(0x01c42030)

struct davinci_psc_regs {
	dv_reg	revid;
	dv_reg	rsvd0[71];
	dv_reg	ptcmd;
	dv_reg	rsvd1;
	dv_reg	ptstat;
	dv_reg	rsvd2[437];
	union {
		struct {
			dv_reg	mdstat[PSC_PSC0_MODULE_ID_CNT];
			dv_reg	rsvd3[112];
			dv_reg	mdctl[PSC_PSC0_MODULE_ID_CNT];
		} psc0;
		struct {
			dv_reg	mdstat[PSC_PSC1_MODULE_ID_CNT];
			dv_reg	rsvd3[96];
			dv_reg	mdctl[PSC_PSC1_MODULE_ID_CNT];
		} psc1;
	};
};

#define davinci_psc0_regs ((struct davinci_psc_regs *)DAVINCI_PSC0_BASE)
#define davinci_psc1_regs ((struct davinci_psc_regs *)DAVINCI_PSC1_BASE)

#define PSC_MDSTAT_STATE		0x3f
#define PSC_MDCTL_NEXT			0x07

struct davinci_pllc_regs {
	dv_reg	revid;
	dv_reg	rsvd1[56];
	dv_reg	rstype;
	dv_reg	rsvd2[6];
	dv_reg	pllctl;
	dv_reg	ocsel;
	dv_reg	rsvd3[2];
	dv_reg	pllm;
	dv_reg	prediv;
	dv_reg	plldiv1;
	dv_reg	plldiv2;
	dv_reg	plldiv3;
	dv_reg	oscdiv;
	dv_reg	postdiv;
	dv_reg	rsvd4[3];
	dv_reg	pllcmd;
	dv_reg	pllstat;
	dv_reg	alnctl;
	dv_reg	dchange;
	dv_reg	cken;
	dv_reg	ckstat;
	dv_reg	systat;
	dv_reg	rsvd5[3];
	dv_reg	plldiv4;
	dv_reg	plldiv5;
	dv_reg	plldiv6;
	dv_reg	plldiv7;
	dv_reg	rsvd6[32];
	dv_reg	emucnt0;
	dv_reg	emucnt1;
};

#define davinci_pllc0_regs ((struct davinci_pllc_regs *)DAVINCI_PLL_CNTRL0_BASE)
#define davinci_pllc1_regs ((struct davinci_pllc_regs *)DAVINCI_PLL_CNTRL1_BASE)
#define DAVINCI_PLLC_DIV_MASK	0x1f

/*
 * A clock ID is a 32-bit number where bit 16 represents the PLL controller
 * (clear is PLLC0, set is PLLC1) and the low 16 bits represent the divisor,
 * counting from 1. Clock IDs may be passed to clk_get().
 */

/* flags to select PLL controller */
#define DAVINCI_PLLC0_FLAG			(0)
#define DAVINCI_PLLC1_FLAG			(1 << 16)

enum davinci_clk_ids {
	/*
	 * Clock IDs for PLL outputs. Each may be switched on/off
	 * independently, and each may map to one or more peripherals.
	 */
	DAVINCI_PLL0_SYSCLK2			= DAVINCI_PLLC0_FLAG | 2,
	DAVINCI_PLL0_SYSCLK4			= DAVINCI_PLLC0_FLAG | 4,
	DAVINCI_PLL0_SYSCLK6			= DAVINCI_PLLC0_FLAG | 6,
	DAVINCI_PLL1_SYSCLK1			= DAVINCI_PLLC1_FLAG | 1,
	DAVINCI_PLL1_SYSCLK2			= DAVINCI_PLLC1_FLAG | 2,

	/* map peripherals to clock IDs */
	DAVINCI_ARM_CLKID			= DAVINCI_PLL0_SYSCLK6,
	DAVINCI_DDR_CLKID			= DAVINCI_PLL1_SYSCLK1,
	DAVINCI_MDIO_CLKID			= DAVINCI_PLL0_SYSCLK4,
	DAVINCI_MMC_CLKID			= DAVINCI_PLL0_SYSCLK2,
	DAVINCI_SPI0_CLKID			= DAVINCI_PLL0_SYSCLK2,
	DAVINCI_MMCSD_CLKID			= DAVINCI_PLL0_SYSCLK2,

	/* special clock ID - output of PLL multiplier */
	DAVINCI_PLLM_CLKID			= 0x0FF,

	/* special clock ID - output of PLL post divisor */
	DAVINCI_PLLC_CLKID			= 0x100,

	/* special clock ID - PLL bypass */
	DAVINCI_AUXCLK_CLKID			= 0x101,
};

#define DAVINCI_UART2_CLKID	(cpu_is_da830() ? DAVINCI_PLL0_SYSCLK2 \
						: get_async3_src())

#define DAVINCI_SPI1_CLKID	(cpu_is_da830() ? DAVINCI_PLL0_SYSCLK2 \
						: get_async3_src())

int clk_get(enum davinci_clk_ids id);

/* Boot config */
struct davinci_syscfg_regs {
	dv_reg	revid;
	dv_reg	rsvd[7];
	dv_reg	bootcfg;
	dv_reg	chiprevidr;
	dv_reg	rsvd2[4];
	dv_reg	kick0;
	dv_reg	kick1;
	dv_reg	rsvd1[52];
	dv_reg	mstpri[3];
	dv_reg  rsvd3;
	dv_reg	pinmux[20];
	dv_reg	suspsrc;
	dv_reg	chipsig;
	dv_reg	chipsig_clr;
	dv_reg	cfgchip0;
	dv_reg	cfgchip1;
	dv_reg	cfgchip2;
	dv_reg	cfgchip3;
	dv_reg	cfgchip4;
};

#define davinci_syscfg_regs \
	((struct davinci_syscfg_regs *)DAVINCI_BOOTCFG_BASE)

enum {
	DAVINCI_NAND8_BOOT	= 0b001110,
	DAVINCI_NAND16_BOOT	= 0b010000,
	DAVINCI_SD_OR_MMC_BOOT	= 0b011100,
	DAVINCI_MMC_ONLY_BOOT	= 0b111100,
	DAVINCI_SPI0_FLASH_BOOT	= 0b001010,
	DAVINCI_SPI1_FLASH_BOOT	= 0b001100,
};

#define pinmux(x)	(&davinci_syscfg_regs->pinmux[x])

/* Emulation suspend bits */
#define DAVINCI_SYSCFG_SUSPSRC_EMAC		(1 << 5)
#define DAVINCI_SYSCFG_SUSPSRC_I2C		(1 << 16)
#define DAVINCI_SYSCFG_SUSPSRC_SPI0		(1 << 21)
#define DAVINCI_SYSCFG_SUSPSRC_SPI1		(1 << 22)
#define DAVINCI_SYSCFG_SUSPSRC_UART0		(1 << 18)
#define DAVINCI_SYSCFG_SUSPSRC_UART1		(1 << 19)
#define DAVINCI_SYSCFG_SUSPSRC_UART2		(1 << 20)
#define DAVINCI_SYSCFG_SUSPSRC_TIMER0		(1 << 27)

struct davinci_syscfg1_regs {
	dv_reg	vtpio_ctl;
	dv_reg	ddr_slew;
	dv_reg	deepsleep;
	dv_reg	pupd_ena;
	dv_reg	pupd_sel;
	dv_reg	rxactive;
	dv_reg	pwrdwn;
};

#define davinci_syscfg1_regs \
	((struct davinci_syscfg1_regs *)DAVINCI_SYSCFG1_BASE)

#define DDR_SLEW_CMOSEN_BIT	4
#define DDR_SLEW_DDR_PDENA_BIT	5

#define VTP_POWERDWN		(1 << 6)
#define VTP_LOCK		(1 << 7)
#define VTP_CLKRZ		(1 << 13)
#define VTP_READY		(1 << 15)
#define VTP_IOPWRDWN		(1 << 14)

#define DV_SYSCFG_KICK0_UNLOCK	0x83e70b13
#define DV_SYSCFG_KICK1_UNLOCK	0x95a4f1e0

/* Interrupt controller */
struct davinci_aintc_regs {
	dv_reg	revid;
	dv_reg	cr;
	dv_reg	dummy0[2];
	dv_reg	ger;
	dv_reg	dummy1[219];
	dv_reg	ecr1;
	dv_reg	ecr2;
	dv_reg	ecr3;
	dv_reg	dummy2[1117];
	dv_reg	hier;
};

#define davinci_aintc_regs ((struct davinci_aintc_regs *)DAVINCI_INTC_BASE)

struct davinci_uart_ctrl_regs {
	dv_reg	revid1;
	dv_reg	revid2;
	dv_reg	pwremu_mgmt;
	dv_reg	mdr;
};

#define DAVINCI_UART_CTRL_BASE 0x28
#define DAVINCI_UART0_CTRL_ADDR (DAVINCI_UART0_BASE + DAVINCI_UART_CTRL_BASE)
#define DAVINCI_UART1_CTRL_ADDR (DAVINCI_UART1_BASE + DAVINCI_UART_CTRL_BASE)
#define DAVINCI_UART2_CTRL_ADDR (DAVINCI_UART2_BASE + DAVINCI_UART_CTRL_BASE)

#define davinci_uart0_ctrl_regs \
	((struct davinci_uart_ctrl_regs *)DAVINCI_UART0_CTRL_ADDR)
#define davinci_uart1_ctrl_regs \
	((struct davinci_uart_ctrl_regs *)DAVINCI_UART1_CTRL_ADDR)
#define davinci_uart2_ctrl_regs \
	((struct davinci_uart_ctrl_regs *)DAVINCI_UART2_CTRL_ADDR)

/* UART PWREMU_MGMT definitions */
#define DAVINCI_UART_PWREMU_MGMT_FREE	(1 << 0)
#define DAVINCI_UART_PWREMU_MGMT_URRST	(1 << 13)
#define DAVINCI_UART_PWREMU_MGMT_UTRST	(1 << 14)

static inline int cpu_is_da830(void)
{
	unsigned int jtag_id	= REG(JTAG_ID_REG);
	unsigned short part_no	= (jtag_id >> 12) & 0xffff;

	return ((part_no == 0xb7df) ? 1 : 0);
}
static inline int cpu_is_da850(void)
{
	unsigned int jtag_id    = REG(JTAG_ID_REG);
	unsigned short part_no  = (jtag_id >> 12) & 0xffff;

	return ((part_no == 0xb7d1) ? 1 : 0);
}

static inline enum davinci_clk_ids get_async3_src(void)
{
	return (REG(&davinci_syscfg_regs->cfgchip3) & 0x10) ?
			DAVINCI_PLL1_SYSCLK2 : DAVINCI_PLL0_SYSCLK2;
}

#endif /* !__ASSEMBLY__ */

#endif /* __ASM_ARCH_HARDWARE_H */
