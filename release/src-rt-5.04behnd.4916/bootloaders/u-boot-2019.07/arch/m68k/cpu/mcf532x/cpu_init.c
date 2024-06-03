// SPDX-License-Identifier: GPL-2.0+
/*
 *
 * (C) Copyright 2000-2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2004-2008, 2012 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
 */

#include <common.h>
#include <watchdog.h>
#include <asm/immap.h>
#include <asm/io.h>

#if defined(CONFIG_CMD_NET)
#include <config.h>
#include <net.h>
#include <asm/fec.h>
#endif

#ifdef CONFIG_MCF5301x
void cpu_init_f(void)
{
	scm1_t *scm1 = (scm1_t *) MMAP_SCM1;
	gpio_t *gpio = (gpio_t *) MMAP_GPIO;
	fbcs_t *fbcs = (fbcs_t *) MMAP_FBCS;

	out_be32(&scm1->mpr, 0x77777777);
	out_be32(&scm1->pacra, 0);
	out_be32(&scm1->pacrb, 0);
	out_be32(&scm1->pacrc, 0);
	out_be32(&scm1->pacrd, 0);
	out_be32(&scm1->pacre, 0);
	out_be32(&scm1->pacrf, 0);
	out_be32(&scm1->pacrg, 0);

#if (defined(CONFIG_SYS_CS0_BASE) && defined(CONFIG_SYS_CS0_MASK) \
     && defined(CONFIG_SYS_CS0_CTRL))
	setbits_8(&gpio->par_cs, GPIO_PAR_CS0_CS0);
	out_be32(&fbcs->csar0, CONFIG_SYS_CS0_BASE);
	out_be32(&fbcs->cscr0, CONFIG_SYS_CS0_CTRL);
	out_be32(&fbcs->csmr0, CONFIG_SYS_CS0_MASK);
#endif

#if (defined(CONFIG_SYS_CS1_BASE) && defined(CONFIG_SYS_CS1_MASK) \
     && defined(CONFIG_SYS_CS1_CTRL))
	setbits_8(&gpio->par_cs, GPIO_PAR_CS1_CS1);
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
	setbits_8(&gpio->par_cs, GPIO_PAR_CS4);
	out_be32(&fbcs->csar4, CONFIG_SYS_CS4_BASE);
	out_be32(&fbcs->cscr4, CONFIG_SYS_CS4_CTRL);
	out_be32(&fbcs->csmr4, CONFIG_SYS_CS4_MASK);
#endif

#if (defined(CONFIG_SYS_CS5_BASE) && defined(CONFIG_SYS_CS5_MASK) \
     && defined(CONFIG_SYS_CS5_CTRL))
	setbits_8(&gpio->par_cs, GPIO_PAR_CS5);
	out_be32(&fbcs->csar5, CONFIG_SYS_CS5_BASE);
	out_be32(&fbcs->cscr5, CONFIG_SYS_CS5_CTRL);
	out_be32(&fbcs->csmr5, CONFIG_SYS_CS5_MASK);
#endif

#ifdef CONFIG_SYS_I2C_FSL
	out_8(&gpio->par_feci2c,
		GPIO_PAR_FECI2C_SDA_SDA | GPIO_PAR_FECI2C_SCL_SCL);
#endif

	icache_enable();
}

/* initialize higher level parts of CPU like timers */
int cpu_init_r(void)
{
#ifdef CONFIG_MCFFEC
	ccm_t *ccm = (ccm_t *) MMAP_CCM;
#endif
#ifdef CONFIG_MCFRTC
	rtc_t *rtc = (rtc_t *) (CONFIG_SYS_MCFRTC_BASE);
	rtcex_t *rtcex = (rtcex_t *) &rtc->extended;

	out_be32(&rtcex->gocu, CONFIG_SYS_RTC_CNT);
	out_be32(&rtcex->gocl, CONFIG_SYS_RTC_SETUP);

#endif
#ifdef CONFIG_MCFFEC
	if (CONFIG_SYS_FEC0_MIIBASE != CONFIG_SYS_FEC1_MIIBASE)
		setbits_be16(&ccm->misccr, CCM_MISCCR_FECM);
	else
		clrbits_be16(&ccm->misccr, CCM_MISCCR_FECM);
#endif

	return (0);
}

void uart_port_conf(int port)
{
	gpio_t *gpio = (gpio_t *) MMAP_GPIO;

	/* Setup Ports: */
	switch (port) {
	case 0:
		clrbits_8(&gpio->par_uart,
			GPIO_PAR_UART_U0TXD | GPIO_PAR_UART_U0RXD);
		setbits_8(&gpio->par_uart,
			GPIO_PAR_UART_U0TXD | GPIO_PAR_UART_U0RXD);
		break;
	case 1:
#ifdef CONFIG_SYS_UART1_ALT1_GPIO
		clrbits_8(&gpio->par_simp1h,
			GPIO_PAR_SIMP1H_DATA1_UNMASK |
			GPIO_PAR_SIMP1H_VEN1_UNMASK);
		setbits_8(&gpio->par_simp1h,
			GPIO_PAR_SIMP1H_DATA1_U1TXD |
			GPIO_PAR_SIMP1H_VEN1_U1RXD);
#elif defined(CONFIG_SYS_UART1_ALT2_GPIO)
		clrbits_8(&gpio->par_ssih,
			GPIO_PAR_SSIH_RXD_UNMASK |
			GPIO_PAR_SSIH_TXD_UNMASK);
		setbits_8(&gpio->par_ssih,
			GPIO_PAR_SSIH_RXD_U1RXD |
			GPIO_PAR_SSIH_TXD_U1TXD);
#endif
		break;
	case 2:
#ifdef CONFIG_SYS_UART2_PRI_GPIO
		setbits_8(&gpio->par_uart,
			GPIO_PAR_UART_U2TXD |
			GPIO_PAR_UART_U2RXD);
#elif defined(CONFIG_SYS_UART2_ALT1_GPIO)
		clrbits_8(&gpio->par_dspih,
			GPIO_PAR_DSPIH_SIN_UNMASK |
			GPIO_PAR_DSPIH_SOUT_UNMASK);
		setbits_8(&gpio->par_dspih,
			GPIO_PAR_DSPIH_SIN_U2RXD |
			GPIO_PAR_DSPIH_SOUT_U2TXD);
#elif defined(CONFIG_SYS_UART2_ALT2_GPIO)
		clrbits_8(&gpio->par_feci2c,
			GPIO_PAR_FECI2C_SDA_UNMASK |
			GPIO_PAR_FECI2C_SCL_UNMASK);
		setbits_8(&gpio->par_feci2c,
			GPIO_PAR_FECI2C_SDA_U2TXD |
			GPIO_PAR_FECI2C_SCL_U2RXD);
#endif
		break;
	}
}

#if defined(CONFIG_CMD_NET)
int fecpin_setclear(struct eth_device *dev, int setclear)
{
	gpio_t *gpio = (gpio_t *) MMAP_GPIO;
	struct fec_info_s *info = (struct fec_info_s *)dev->priv;

	if (setclear) {
		if (info->iobase == CONFIG_SYS_FEC0_IOBASE) {
			setbits_8(&gpio->par_fec,
				GPIO_PAR_FEC0_7W_FEC | GPIO_PAR_FEC0_RMII_FEC);
			setbits_8(&gpio->par_feci2c,
				GPIO_PAR_FECI2C_MDC0 | GPIO_PAR_FECI2C_MDIO0);
		} else {
			setbits_8(&gpio->par_fec,
				GPIO_PAR_FEC1_7W_FEC | GPIO_PAR_FEC1_RMII_FEC);
			setbits_8(&gpio->par_feci2c,
				GPIO_PAR_FECI2C_MDC1 | GPIO_PAR_FECI2C_MDIO1);
		}
	} else {
		if (info->iobase == CONFIG_SYS_FEC0_IOBASE) {
			clrbits_8(&gpio->par_fec,
				GPIO_PAR_FEC0_7W_FEC | GPIO_PAR_FEC0_RMII_FEC);
			clrbits_8(&gpio->par_feci2c, ~GPIO_PAR_FECI2C_RMII0_UNMASK);
		} else {
			clrbits_8(&gpio->par_fec,
				GPIO_PAR_FEC1_7W_FEC | GPIO_PAR_FEC1_RMII_FEC);
			clrbits_8(&gpio->par_feci2c, ~GPIO_PAR_FECI2C_RMII1_UNMASK);
		}
	}
	return 0;
}
#endif				/* CONFIG_CMD_NET */
#endif				/* CONFIG_MCF5301x */

#ifdef CONFIG_MCF532x
void cpu_init_f(void)
{
	scm1_t *scm1 = (scm1_t *) MMAP_SCM1;
	scm2_t *scm2 = (scm2_t *) MMAP_SCM2;
	gpio_t *gpio = (gpio_t *) MMAP_GPIO;
	fbcs_t *fbcs = (fbcs_t *) MMAP_FBCS;
#ifndef CONFIG_WATCHDOG
	wdog_t *wdog = (wdog_t *) MMAP_WDOG;

	/* watchdog is enabled by default - disable the watchdog */
	out_be16(&wdog->cr, 0);
#endif

	out_be32(&scm1->mpr0, 0x77777777);
	out_be32(&scm2->pacra, 0);
	out_be32(&scm2->pacrb, 0);
	out_be32(&scm2->pacrc, 0);
	out_be32(&scm2->pacrd, 0);
	out_be32(&scm2->pacre, 0);
	out_be32(&scm2->pacrf, 0);
	out_be32(&scm2->pacrg, 0);
	out_be32(&scm1->pacrh, 0);

	/* Port configuration */
	out_8(&gpio->par_cs, 0);

#if (defined(CONFIG_SYS_CS0_BASE) && defined(CONFIG_SYS_CS0_MASK) \
     && defined(CONFIG_SYS_CS0_CTRL))
	out_be32(&fbcs->csar0, CONFIG_SYS_CS0_BASE);
	out_be32(&fbcs->cscr0, CONFIG_SYS_CS0_CTRL);
	out_be32(&fbcs->csmr0, CONFIG_SYS_CS0_MASK);
#endif

#if (defined(CONFIG_SYS_CS1_BASE) && defined(CONFIG_SYS_CS1_MASK) \
     && defined(CONFIG_SYS_CS1_CTRL))
	/* Latch chipselect */
	setbits_8(&gpio->par_cs, GPIO_PAR_CS1);
	out_be32(&fbcs->csar1, CONFIG_SYS_CS1_BASE);
	out_be32(&fbcs->cscr1, CONFIG_SYS_CS1_CTRL);
	out_be32(&fbcs->csmr1, CONFIG_SYS_CS1_MASK);
#endif

#if (defined(CONFIG_SYS_CS2_BASE) && defined(CONFIG_SYS_CS2_MASK) \
     && defined(CONFIG_SYS_CS2_CTRL))
	setbits_8(&gpio->par_cs, GPIO_PAR_CS2);
	out_be32(&fbcs->csar2, CONFIG_SYS_CS2_BASE);
	out_be32(&fbcs->cscr2, CONFIG_SYS_CS2_CTRL);
	out_be32(&fbcs->csmr2, CONFIG_SYS_CS2_MASK);
#endif

#if (defined(CONFIG_SYS_CS3_BASE) && defined(CONFIG_SYS_CS3_MASK) \
     && defined(CONFIG_SYS_CS3_CTRL))
	setbits_8(&gpio->par_cs, GPIO_PAR_CS3);
	out_be32(&fbcs->csar3, CONFIG_SYS_CS3_BASE);
	out_be32(&fbcs->cscr3, CONFIG_SYS_CS3_CTRL);
	out_be32(&fbcs->csmr3, CONFIG_SYS_CS3_MASK);
#endif

#if (defined(CONFIG_SYS_CS4_BASE) && defined(CONFIG_SYS_CS4_MASK) \
     && defined(CONFIG_SYS_CS4_CTRL))
	setbits_8(&gpio->par_cs, GPIO_PAR_CS4);
	out_be32(&fbcs->csar4, CONFIG_SYS_CS4_BASE);
	out_be32(&fbcs->cscr4, CONFIG_SYS_CS4_CTRL);
	out_be32(&fbcs->csmr4, CONFIG_SYS_CS4_MASK);
#endif

#if (defined(CONFIG_SYS_CS5_BASE) && defined(CONFIG_SYS_CS5_MASK) \
     && defined(CONFIG_SYS_CS5_CTRL))
	setbits_8(&gpio->par_cs, GPIO_PAR_CS5);
	out_be32(&fbcs->csar5, CONFIG_SYS_CS5_BASE);
	out_be32(&fbcs->cscr5, CONFIG_SYS_CS5_CTRL);
	out_be32(&fbcs->csmr5, CONFIG_SYS_CS5_MASK);
#endif

#ifdef CONFIG_SYS_I2C_FSL
	out_8(&gpio->par_feci2c,
		GPIO_PAR_FECI2C_SCL_SCL | GPIO_PAR_FECI2C_SDA_SDA);
#endif

	icache_enable();
}

/*
 * initialize higher level parts of CPU like timers
 */
int cpu_init_r(void)
{
	return (0);
}

void uart_port_conf(int port)
{
	gpio_t *gpio = (gpio_t *) MMAP_GPIO;

	/* Setup Ports: */
	switch (port) {
	case 0:
		clrbits_be16(&gpio->par_uart,
			GPIO_PAR_UART_TXD0 | GPIO_PAR_UART_RXD0);
		setbits_be16(&gpio->par_uart,
			GPIO_PAR_UART_TXD0 | GPIO_PAR_UART_RXD0);
		break;
	case 1:
		clrbits_be16(&gpio->par_uart,
			GPIO_PAR_UART_TXD1(3) | GPIO_PAR_UART_RXD1(3));
		setbits_be16(&gpio->par_uart,
			GPIO_PAR_UART_TXD1(3) | GPIO_PAR_UART_RXD1(3));
		break;
	case 2:
#ifdef CONFIG_SYS_UART2_ALT1_GPIO
		clrbits_8(&gpio->par_timer, 0xf0);
		setbits_8(&gpio->par_timer,
			GPIO_PAR_TIN3_URXD2 | GPIO_PAR_TIN2_UTXD2);
#elif defined(CONFIG_SYS_UART2_ALT2_GPIO)
		clrbits_8(&gpio->par_feci2c, 0x00ff);
		setbits_8(&gpio->par_feci2c,
			GPIO_PAR_FECI2C_SCL_UTXD2 | GPIO_PAR_FECI2C_SDA_URXD2);
#elif defined(CONFIG_SYS_UART2_ALT3_GPIO)
		clrbits_be16(&gpio->par_ssi, 0x0f00);
		setbits_be16(&gpio->par_ssi,
			GPIO_PAR_SSI_RXD(2) | GPIO_PAR_SSI_TXD(2));
#endif
		break;
	}
}

#if defined(CONFIG_CMD_NET)
int fecpin_setclear(struct eth_device *dev, int setclear)
{
	gpio_t *gpio = (gpio_t *) MMAP_GPIO;

	if (setclear) {
		setbits_8(&gpio->par_fec,
			GPIO_PAR_FEC_7W_FEC | GPIO_PAR_FEC_MII_FEC);
		setbits_8(&gpio->par_feci2c,
			GPIO_PAR_FECI2C_MDC_EMDC | GPIO_PAR_FECI2C_MDIO_EMDIO);
	} else {
		clrbits_8(&gpio->par_fec,
			GPIO_PAR_FEC_7W_FEC | GPIO_PAR_FEC_MII_FEC);
		clrbits_8(&gpio->par_feci2c,
			GPIO_PAR_FECI2C_MDC_EMDC | GPIO_PAR_FECI2C_MDIO_EMDIO);
	}
	return 0;
}
#endif
#endif				/* CONFIG_MCF532x */
