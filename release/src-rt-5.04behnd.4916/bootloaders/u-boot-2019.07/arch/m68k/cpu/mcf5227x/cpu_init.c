// SPDX-License-Identifier: GPL-2.0+
/*
 *
 * (C) Copyright 2000-2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2004-2007, 2012 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
 */

#include <common.h>
#include <watchdog.h>

#include <asm/immap.h>
#include <asm/io.h>
#include <asm/rtc.h>
#include <linux/compiler.h>

void cfspi_port_conf(void)
{
	gpio_t *gpio = (gpio_t *)MMAP_GPIO;

	out_8(&gpio->par_dspi,
	      GPIO_PAR_DSPI_SIN_SIN | GPIO_PAR_DSPI_SOUT_SOUT |
	      GPIO_PAR_DSPI_SCK_SCK);
}

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
	fbcs_t *fbcs __maybe_unused = (fbcs_t *) MMAP_FBCS;

#if !defined(CONFIG_CF_SBF)
	scm1_t *scm1 = (scm1_t *) MMAP_SCM1;
	pll_t *pll = (pll_t *)MMAP_PLL;

	/* Workaround, must place before fbcs */
	out_be32(&pll->psr, 0x12);

	out_be32(&scm1->mpr, 0x77777777);
	out_be32(&scm1->pacra, 0);
	out_be32(&scm1->pacrb, 0);
	out_be32(&scm1->pacrc, 0);
	out_be32(&scm1->pacrd, 0);
	out_be32(&scm1->pacre, 0);
	out_be32(&scm1->pacrf, 0);
	out_be32(&scm1->pacrg, 0);
	out_be32(&scm1->pacri, 0);

#if (defined(CONFIG_SYS_CS0_BASE) && defined(CONFIG_SYS_CS0_MASK) \
     && defined(CONFIG_SYS_CS0_CTRL))
	out_be32(&fbcs->csar0, CONFIG_SYS_CS0_BASE);
	out_be32(&fbcs->cscr0, CONFIG_SYS_CS0_CTRL);
	out_be32(&fbcs->csmr0, CONFIG_SYS_CS0_MASK);
#endif
#endif				/* CONFIG_CF_SBF */

#if (defined(CONFIG_SYS_CS1_BASE) && defined(CONFIG_SYS_CS1_MASK) \
     && defined(CONFIG_SYS_CS1_CTRL))
	out_be32(&fbcs->csar1, CONFIG_SYS_CS1_BASE);
	out_be32(&fbcs->cscr1, CONFIG_SYS_CS1_CTRL);
	out_be32(&fbcs->csmr1, CONFIG_SYS_CS1_MASK);
#endif

#if (defined(CONFIG_SYS_CS2_BASE) && defined(CONFIG_SYS_CS2_MASK) \
     && defined(CONFIG_SYS_CS2_CTRL))
	out_be32(&fbcs->csar2, CONFIG_SYS_CS2_BASE);
	out_be32(&fbcs->cscr2, CONFIG_SYS_CS2_CTRL);
	out_be32(&fbcs->csmr2, CONFIG_SYS_CS2_MASK);
#endif

#if (defined(CONFIG_SYS_CS3_BASE) && defined(CONFIG_SYS_CS3_MASK) \
     && defined(CONFIG_SYS_CS3_CTRL))
	out_be32(&fbcs->csar3, CONFIG_SYS_CS3_BASE);
	out_be32(&fbcs->cscr3, CONFIG_SYS_CS3_CTRL);
	out_be32(&fbcs->csmr3, CONFIG_SYS_CS3_MASK);
#endif

#if (defined(CONFIG_SYS_CS4_BASE) && defined(CONFIG_SYS_CS4_MASK) \
     && defined(CONFIG_SYS_CS4_CTRL))
	out_be32(&fbcs->csar4, CONFIG_SYS_CS4_BASE);
	out_be32(&fbcs->cscr4, CONFIG_SYS_CS4_CTRL);
	out_be32(&fbcs->csmr4, CONFIG_SYS_CS4_MASK);
#endif

#if (defined(CONFIG_SYS_CS5_BASE) && defined(CONFIG_SYS_CS5_MASK) \
     && defined(CONFIG_SYS_CS5_CTRL))
	out_be32(&fbcs->csar5, CONFIG_SYS_CS5_BASE);
	out_be32(&fbcs->cscr5, CONFIG_SYS_CS5_CTRL);
	out_be32(&fbcs->csmr5, CONFIG_SYS_CS5_MASK);
#endif

#ifdef CONFIG_SYS_I2C_FSL
	out_8(&gpio->par_i2c, GPIO_PAR_I2C_SCL_SCL | GPIO_PAR_I2C_SDA_SDA);
#endif

	icache_enable();

	cfspi_port_conf();
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

	/* Setup Ports: */
	switch (port) {
	case 0:
		clrbits_be16(&gpio->par_uart,
			~(GPIO_PAR_UART_U0TXD_UNMASK & GPIO_PAR_UART_U0RXD_UNMASK));
		setbits_be16(&gpio->par_uart,
			GPIO_PAR_UART_U0TXD_U0TXD | GPIO_PAR_UART_U0RXD_U0RXD);
		break;
	case 1:
		clrbits_be16(&gpio->par_uart,
			~(GPIO_PAR_UART_U1TXD_UNMASK & GPIO_PAR_UART_U1RXD_UNMASK));
		setbits_be16(&gpio->par_uart,
			GPIO_PAR_UART_U1TXD_U1TXD | GPIO_PAR_UART_U1RXD_U1RXD);
		break;
	case 2:
		clrbits_8(&gpio->par_dspi,
			~(GPIO_PAR_DSPI_SIN_UNMASK & GPIO_PAR_DSPI_SOUT_UNMASK));
		out_8(&gpio->par_dspi,
			GPIO_PAR_DSPI_SIN_U2RXD | GPIO_PAR_DSPI_SOUT_U2TXD);
		break;
	}
}
