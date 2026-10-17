// SPDX-License-Identifier: GPL-2.0+
/*
 *
 * (C) Copyright 2000-2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2004-2007, 2012 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
 */

#include <cpu_func.h>
#include <init.h>
#include <watchdog.h>
#include <asm/immap.h>
#include <asm/processor.h>
#include <asm/rtc.h>
#include <asm/io.h>
#include <linux/compiler.h>

#if defined(CONFIG_CMD_NET)
#include <config.h>
#include <net.h>
#include <asm/fec.h>
#endif

void init_fbcs(void)
{
	fbcs_t *fbcs __maybe_unused = (fbcs_t *) MMAP_FBCS;

#if !defined(CONFIG_SERIAL_BOOT)
#if (defined(CFG_SYS_CS0_BASE) && defined(CFG_SYS_CS0_MASK) && defined(CFG_SYS_CS0_CTRL))
	out_be32(&fbcs->csar0, CFG_SYS_CS0_BASE);
	out_be32(&fbcs->cscr0, CFG_SYS_CS0_CTRL);
	out_be32(&fbcs->csmr0, CFG_SYS_CS0_MASK);
#endif
#endif

#if (defined(CFG_SYS_CS1_BASE) && defined(CFG_SYS_CS1_MASK) && defined(CFG_SYS_CS1_CTRL))
	/* Latch chipselect */
	out_be32(&fbcs->csar1, CFG_SYS_CS1_BASE);
	out_be32(&fbcs->cscr1, CFG_SYS_CS1_CTRL);
	out_be32(&fbcs->csmr1, CFG_SYS_CS1_MASK);
#endif

#if (defined(CFG_SYS_CS2_BASE) && defined(CFG_SYS_CS2_MASK) && defined(CFG_SYS_CS2_CTRL))
	out_be32(&fbcs->csar2, CFG_SYS_CS2_BASE);
	out_be32(&fbcs->cscr2, CFG_SYS_CS2_CTRL);
	out_be32(&fbcs->csmr2, CFG_SYS_CS2_MASK);
#endif

#if (defined(CFG_SYS_CS3_BASE) && defined(CFG_SYS_CS3_MASK) && defined(CFG_SYS_CS3_CTRL))
	out_be32(&fbcs->csar3, CFG_SYS_CS3_BASE);
	out_be32(&fbcs->cscr3, CFG_SYS_CS3_CTRL);
	out_be32(&fbcs->csmr3, CFG_SYS_CS3_MASK);
#endif

#if (defined(CONFIG_SYS_CS4_BASE) && defined(CONFIG_SYS_CS4_MASK) && defined(CONFIG_SYS_CS4_CTRL))
	out_be32(&fbcs->csar4, CONFIG_SYS_CS4_BASE);
	out_be32(&fbcs->cscr4, CONFIG_SYS_CS4_CTRL);
	out_be32(&fbcs->csmr4, CONFIG_SYS_CS4_MASK);
#endif

#if (defined(CONFIG_SYS_CS5_BASE) && defined(CONFIG_SYS_CS5_MASK) && defined(CONFIG_SYS_CS5_CTRL))
	out_be32(&fbcs->csar5, CONFIG_SYS_CS5_BASE);
	out_be32(&fbcs->cscr5, CONFIG_SYS_CS5_CTRL);
	out_be32(&fbcs->csmr5, CONFIG_SYS_CS5_MASK);
#endif
}

#ifdef CONFIG_CF_DSPI
void cfspi_port_conf(void)
{
	gpio_t *gpio = (gpio_t *)MMAP_GPIO;

#ifdef CONFIG_MCF5441x
	pm_t *pm = (pm_t *)MMAP_PM;

	out_8(&gpio->par_dspi0,
	      GPIO_PAR_DSPI0_SIN_DSPI0SIN | GPIO_PAR_DSPI0_SOUT_DSPI0SOUT |
	      GPIO_PAR_DSPI0_SCK_DSPI0SCK);
	out_8(&gpio->srcr_dspiow, 3);

	/* DSPI0 */
	out_8(&pm->pmcr0, 23);
#endif
}
#endif

/*
 * Breath some life into the CPU...
 *
 * Set up the memory map,
 * initialize a bunch of registers,
 * initialize the UPM's
 */
void cpu_init_f(void)
{
	gpio_t *gpio = (gpio_t *) MMAP_GPIO;

#ifdef CONFIG_MCF5441x
	scm_t *scm = (scm_t *) MMAP_SCM;
	pm_t *pm = (pm_t *) MMAP_PM;

	/* Disable Switch */
	*(unsigned long *)(MMAP_L2_SW0 + 0x00000024) = 0;

	/* Disable core watchdog */
	out_be16(&scm->cwcr, 0);
	out_8(&gpio->par_fbctl,
		GPIO_PAR_FBCTL_ALE_FB_ALE | GPIO_PAR_FBCTL_OE_FB_OE |
		GPIO_PAR_FBCTL_FBCLK | GPIO_PAR_FBCTL_RW |
		GPIO_PAR_FBCTL_TA_TA);
	out_8(&gpio->par_be,
		GPIO_PAR_BE_BE3_BE3 | GPIO_PAR_BE_BE2_BE2 |
		GPIO_PAR_BE_BE1_BE1 | GPIO_PAR_BE_BE0_BE0);

	/* eDMA */
	out_8(&pm->pmcr0, 17);

	/* INTR0 - INTR2 */
	out_8(&pm->pmcr0, 18);
	out_8(&pm->pmcr0, 19);
	out_8(&pm->pmcr0, 20);

	/* I2C */
	out_8(&pm->pmcr0, 22);
	out_8(&pm->pmcr1, 4);
	out_8(&pm->pmcr1, 7);

	/* DTMR0 - DTMR3*/
	out_8(&pm->pmcr0, 28);
	out_8(&pm->pmcr0, 29);
	out_8(&pm->pmcr0, 30);
	out_8(&pm->pmcr0, 31);

	/* PIT0 - PIT3 */
	out_8(&pm->pmcr0, 32);
	out_8(&pm->pmcr0, 33);
	out_8(&pm->pmcr0, 34);
	out_8(&pm->pmcr0, 35);

	/* Edge Port */
	out_8(&pm->pmcr0, 36);
	out_8(&pm->pmcr0, 37);

	/* USB OTG */
	out_8(&pm->pmcr0, 44);
	/* USB Host */
	out_8(&pm->pmcr0, 45);

	/* ESDHC */
	out_8(&pm->pmcr0, 51);

	/* ENET0 - ENET1 */
	out_8(&pm->pmcr0, 53);
	out_8(&pm->pmcr0, 54);

	/* NAND */
	out_8(&pm->pmcr0, 63);

#ifdef CFG_SYS_I2C_0
	out_8(&gpio->par_cani2c, 0xF0);
	/* I2C0 pull up */
	out_be16(&gpio->pcr_b, 0x003C);
	/* I2C0 max speed */
	out_8(&gpio->srcr_cani2c, 0x03);
#endif
#ifdef CFG_SYS_I2C_2
	/* I2C2 */
	out_8(&gpio->par_ssi0h, 0xA0);
	/* I2C2, UART7 */
	out_8(&gpio->par_ssi0h, 0xA8);
	/* UART7 */
	out_8(&gpio->par_ssi0l, 0x2);
	/* UART8, UART9 */
	out_8(&gpio->par_cani2c, 0xAA);
	/* UART4, UART0 */
	out_8(&gpio->par_uart0, 0xAF);
	/* UART5, UART1 */
	out_8(&gpio->par_uart1, 0xAF);
	/* UART6, UART2 */
	out_8(&gpio->par_uart2, 0xAF);
	/* I2C2 pull up */
	out_be16(&gpio->pcr_h, 0xF000);
#endif
#ifdef CFG_SYS_I2C_5
	/* I2C5 */
	out_8(&gpio->par_uart1, 0x0A);
	/* I2C5 pull up */
	out_be16(&gpio->pcr_e, 0x0003);
	out_be16(&gpio->pcr_f, 0xC000);
#endif

	/* Lowest slew rate for UART0,1,2 */
	out_8(&gpio->srcr_uart, 0x00);

#ifdef CONFIG_FSL_ESDHC_IMX
	/* eSDHC pin as faster speed */
	out_8(&gpio->srcr_sdhc, 0x03);

	/* All esdhc pins as SD */
	out_8(&gpio->par_sdhch, 0xff);
	out_8(&gpio->par_sdhcl, 0xff);
#endif
#endif		/* CONFIG_MCF5441x */

	/* FlexBus Chipselect */
	init_fbcs();

#ifdef CFG_SYS_CS0_BASE
	/*
	 * now the flash base address is no longer at 0 (Newer ColdFire family
	 * boot at address 0 instead of 0xFFnn_nnnn). The vector table must
	 * also move to the new location.
	 */
	if (CFG_SYS_CS0_BASE != 0)
		setvbr(CFG_SYS_CS0_BASE);
#endif

	icache_enable();
}

/*
 * initialize higher level parts of CPU like timers
 */
int cpu_init_r(void)
{
#ifdef CONFIG_MCFRTC
	rtc_t *rtc = (rtc_t *)(CONFIG_SYS_MCFRTC_BASE);
	rtcex_t *rtcex = (rtcex_t *)&rtc->extended;

	out_be32(&rtcex->gocu, (CONFIG_SYS_RTC_OSCILLATOR >> 16) & 0xffff);
	out_be32(&rtcex->gocl, CONFIG_SYS_RTC_OSCILLATOR & 0xffff);
#endif

	return (0);
}

void uart_port_conf(int port)
{
	gpio_t *gpio = (gpio_t *) MMAP_GPIO;
#ifdef CONFIG_MCF5441x
	pm_t *pm = (pm_t *) MMAP_PM;
#endif

	/* Setup Ports: */
	switch (port) {
#ifdef CONFIG_MCF5441x
	case 0:
		/* UART0 */
		out_8(&pm->pmcr0, 24);
		clrbits_8(&gpio->par_uart0,
			~(GPIO_PAR_UART0_U0RXD_MASK | GPIO_PAR_UART0_U0TXD_MASK));
		setbits_8(&gpio->par_uart0,
			GPIO_PAR_UART0_U0RXD_U0RXD | GPIO_PAR_UART0_U0TXD_U0TXD);
		break;
	case 1:
		/* UART1 */
		out_8(&pm->pmcr0, 25);
		clrbits_8(&gpio->par_uart1,
			~(GPIO_PAR_UART1_U1RXD_MASK | GPIO_PAR_UART1_U1TXD_MASK));
		setbits_8(&gpio->par_uart1,
			GPIO_PAR_UART1_U1RXD_U1RXD | GPIO_PAR_UART1_U1TXD_U1TXD);
		break;
	case 2:
		/* UART2 */
		out_8(&pm->pmcr0, 26);
		clrbits_8(&gpio->par_uart2,
			~(GPIO_PAR_UART2_U2RXD_MASK | GPIO_PAR_UART2_U2TXD_MASK));
		setbits_8(&gpio->par_uart2,
			GPIO_PAR_UART2_U2RXD_U2RXD | GPIO_PAR_UART2_U2TXD_U2TXD);
		break;
	case 3:
		/* UART3 */
		out_8(&pm->pmcr0, 27);
		clrbits_8(&gpio->par_dspi0,
			~(GPIO_PAR_DSPI0_SIN_MASK | GPIO_PAR_DSPI0_SOUT_MASK));
		setbits_8(&gpio->par_dspi0,
			GPIO_PAR_DSPI0_SIN_U3RXD | GPIO_PAR_DSPI0_SOUT_U3TXD);
		break;
	case 4:
		/* UART4 */
		out_8(&pm->pmcr1, 24);
		clrbits_8(&gpio->par_uart0,
			~(GPIO_PAR_UART0_U0CTS_MASK | GPIO_PAR_UART0_U0RTS_MASK));
		setbits_8(&gpio->par_uart0,
			GPIO_PAR_UART0_U0CTS_U4TXD | GPIO_PAR_UART0_U0RTS_U4RXD);
		break;
	case 5:
		/* UART5 */
		out_8(&pm->pmcr1, 25);
		clrbits_8(&gpio->par_uart1,
			~(GPIO_PAR_UART1_U1CTS_MASK | GPIO_PAR_UART1_U1RTS_MASK));
		setbits_8(&gpio->par_uart1,
			GPIO_PAR_UART1_U1CTS_U5TXD | GPIO_PAR_UART1_U1RTS_U5RXD);
		break;
	case 6:
		/* UART6 */
		out_8(&pm->pmcr1, 26);
		clrbits_8(&gpio->par_uart2,
			~(GPIO_PAR_UART2_U2CTS_MASK | GPIO_PAR_UART2_U2RTS_MASK));
		setbits_8(&gpio->par_uart2,
			GPIO_PAR_UART2_U2CTS_U6TXD | GPIO_PAR_UART2_U2RTS_U6RXD);
		break;
	case 7:
		/* UART7 */
		out_8(&pm->pmcr1, 27);
		clrbits_8(&gpio->par_ssi0h, ~GPIO_PAR_SSI0H_RXD_MASK);
		clrbits_8(&gpio->par_ssi0l, ~GPIO_PAR_SSI0L_BCLK_MASK);
		setbits_8(&gpio->par_ssi0h, GPIO_PAR_SSI0H_FS_U7TXD);
		setbits_8(&gpio->par_ssi0l, GPIO_PAR_SSI0L_BCLK_U7RXD);
		break;
	case 8:
		/* UART8 */
		out_8(&pm->pmcr0, 28);
		clrbits_8(&gpio->par_cani2c,
			~(GPIO_PAR_CANI2C_I2C0SCL_MASK | GPIO_PAR_CANI2C_I2C0SDA_MASK));
		setbits_8(&gpio->par_cani2c,
			GPIO_PAR_CANI2C_I2C0SCL_U8TXD | GPIO_PAR_CANI2C_I2C0SDA_U8RXD);
		break;
	case 9:
		/* UART9 */
		out_8(&pm->pmcr1, 29);
		clrbits_8(&gpio->par_cani2c,
			~(GPIO_PAR_CANI2C_CAN1TX_MASK | GPIO_PAR_CANI2C_CAN1RX_MASK));
		setbits_8(&gpio->par_cani2c,
			GPIO_PAR_CANI2C_CAN1TX_U9TXD | GPIO_PAR_CANI2C_CAN1RX_U9RXD);
		break;
#endif
	}
}

#if defined(CONFIG_CMD_NET)
int fecpin_setclear(fec_info_t *info, int setclear)
{
	gpio_t *gpio = (gpio_t *) MMAP_GPIO;
	u32 fec0_base;

	if (fec_get_base_addr(0, &fec0_base))
		return -1;

#ifdef CONFIG_MCF5441x
	if (setclear) {
		out_8(&gpio->par_fec, 0x03);
		out_8(&gpio->srcr_fec, 0x0F);
		clrsetbits_8(&gpio->par_simp0h, ~GPIO_PAR_SIMP0H_DAT_MASK,
			GPIO_PAR_SIMP0H_DAT_GPIO);
		clrsetbits_8(&gpio->pddr_g, ~GPIO_PDDR_G4_MASK,
			GPIO_PDDR_G4_OUTPUT);
		clrbits_8(&gpio->podr_g, ~GPIO_PODR_G4_MASK);

	} else
		clrbits_8(&gpio->par_fec, ~GPIO_PAR_FEC_FEC_MASK);
#endif
	return 0;
}
#endif
