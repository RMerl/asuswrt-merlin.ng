// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2003
 * Josef Baumgartner <josef.baumgartner@telex.de>
 *
 * MCF5282 additionals
 * (C) Copyright 2005
 * BuS Elektronik GmbH & Co. KG <esw@bus-elektronik.de>
 * (c) Copyright 2010
 * Arcturus Networks Inc. <www.arcturusnetworks.com>
 *
 * Copyright (C) 2004-2007, 2012 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
 * Hayden Fraser (Hayden.Fraser@freescale.com)
 *
 * MCF5275 additions
 * Copyright (C) 2008 Arthur Shipkowski (art@videon-central.com)
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

#ifndef CONFIG_M5272
/* Only 5272 Flexbus chipselect is different from the rest */
void init_fbcs(void)
{
	fbcs_t *fbcs = (fbcs_t *) (MMAP_FBCS);

#if (defined(CONFIG_SYS_CS0_BASE) && defined(CONFIG_SYS_CS0_MASK) \
     && defined(CONFIG_SYS_CS0_CTRL))
	out_be32(&fbcs->csar0, CONFIG_SYS_CS0_BASE);
	out_be32(&fbcs->cscr0, CONFIG_SYS_CS0_CTRL);
	out_be32(&fbcs->csmr0, CONFIG_SYS_CS0_MASK);
#else
#warning "Chip Select 0 are not initialized/used"
#endif
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
#if (defined(CONFIG_SYS_CS6_BASE) && defined(CONFIG_SYS_CS6_MASK) \
     && defined(CONFIG_SYS_CS6_CTRL))
	out_be32(&fbcs->csar6, CONFIG_SYS_CS6_BASE);
	out_be32(&fbcs->cscr6, CONFIG_SYS_CS6_CTRL);
	out_be32(&fbcs->csmr6, CONFIG_SYS_CS6_MASK);
#endif
#if (defined(CONFIG_SYS_CS7_BASE) && defined(CONFIG_SYS_CS7_MASK) \
     && defined(CONFIG_SYS_CS7_CTRL))
	out_be32(&fbcs->csar7, CONFIG_SYS_CS7_BASE);
	out_be32(&fbcs->cscr7, CONFIG_SYS_CS7_CTRL);
	out_be32(&fbcs->csmr7, CONFIG_SYS_CS7_MASK);
#endif
}
#endif

#if defined(CONFIG_M5208)
void cpu_init_f(void)
{
	scm1_t *scm1 = (scm1_t *) MMAP_SCM1;

#ifndef CONFIG_WATCHDOG
	wdog_t *wdg = (wdog_t *) MMAP_WDOG;

	/* Disable the watchdog if we aren't using it */
	out_be16(&wdg->cr, 0);
#endif

	out_be32(&scm1->mpr, 0x77777777);
	out_be32(&scm1->pacra, 0);
	out_be32(&scm1->pacrb, 0);
	out_be32(&scm1->pacrc, 0);
	out_be32(&scm1->pacrd, 0);
	out_be32(&scm1->pacre, 0);
	out_be32(&scm1->pacrf, 0);

	/* FlexBus Chipselect */
	init_fbcs();

	icache_enable();
}

/* initialize higher level parts of CPU like timers */
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
		clrbits_be16(&gpio->par_uart, ~GPIO_PAR_UART0_UNMASK);
		setbits_be16(&gpio->par_uart, GPIO_PAR_UART_U0TXD | GPIO_PAR_UART_U0RXD);
		break;
	case 1:
		clrbits_be16(&gpio->par_uart, ~GPIO_PAR_UART0_UNMASK);
		setbits_be16(&gpio->par_uart, GPIO_PAR_UART_U1TXD | GPIO_PAR_UART_U1RXD);
		break;
	case 2:
#ifdef CONFIG_SYS_UART2_PRI_GPIO
		clrbits_8(&gpio->par_timer,
			~(GPIO_PAR_TMR_TIN0_UNMASK | GPIO_PAR_TMR_TIN1_UNMASK));
		setbits_8(&gpio->par_timer,
			GPIO_PAR_TMR_TIN0_U2TXD | GPIO_PAR_TMR_TIN1_U2RXD);
#endif
#ifdef CONFIG_SYS_UART2_ALT1_GPIO
		clrbits_8(&gpio->par_feci2c,
			~(GPIO_PAR_FECI2C_MDC_UNMASK | GPIO_PAR_FECI2C_MDIO_UNMASK));
		setbits_8(&gpio->par_feci2c,
			GPIO_PAR_FECI2C_MDC_U2TXD | GPIO_PAR_FECI2C_MDIO_U2RXD);
#endif
#ifdef CONFIG_SYS_UART2_ALT1_GPIO
		clrbits_8(&gpio->par_feci2c,
			~(GPIO_PAR_FECI2C_SDA_UNMASK | GPIO_PAR_FECI2C_SCL_UNMASK));
		setbits_8(&gpio->par_feci2c,
			GPIO_PAR_FECI2C_SDA_U2TXD | GPIO_PAR_FECI2C_SCL_U2RXD);
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
			GPIO_PAR_FECI2C_MDC_MDC | GPIO_PAR_FECI2C_MDIO_MDIO);
	} else {
		clrbits_8(&gpio->par_fec,
			~(GPIO_PAR_FEC_7W_UNMASK & GPIO_PAR_FEC_MII_UNMASK));
		clrbits_8(&gpio->par_feci2c, ~GPIO_PAR_FECI2C_RMII_UNMASK);
	}
	return 0;
}
#endif				/* CONFIG_CMD_NET */
#endif				/* CONFIG_M5208 */

#if defined(CONFIG_M5253)
/*
 * Breath some life into the CPU...
 *
 * Set up the memory map,
 * initialize a bunch of registers,
 * initialize the UPM's
 */
void cpu_init_f(void)
{
	mbar_writeByte(MCFSIM_MPARK, 0x40);	/* 5249 Internal Core takes priority over DMA */
	mbar_writeByte(MCFSIM_SYPCR, 0x00);
	mbar_writeByte(MCFSIM_SWIVR, 0x0f);
	mbar_writeByte(MCFSIM_SWSR, 0x00);
	mbar_writeByte(MCFSIM_SWDICR, 0x00);
	mbar_writeByte(MCFSIM_TIMER1ICR, 0x00);
	mbar_writeByte(MCFSIM_TIMER2ICR, 0x88);
	mbar_writeByte(MCFSIM_I2CICR, 0x00);
	mbar_writeByte(MCFSIM_UART1ICR, 0x00);
	mbar_writeByte(MCFSIM_UART2ICR, 0x00);
	mbar_writeByte(MCFSIM_ICR6, 0x00);
	mbar_writeByte(MCFSIM_ICR7, 0x00);
	mbar_writeByte(MCFSIM_ICR8, 0x00);
	mbar_writeByte(MCFSIM_ICR9, 0x00);
	mbar_writeByte(MCFSIM_QSPIICR, 0x00);

	mbar2_writeLong(MCFSIM_GPIO_INT_EN, 0x00000080);
	mbar2_writeByte(MCFSIM_INTBASE, 0x40);	/* Base interrupts at 64 */
	mbar2_writeByte(MCFSIM_SPURVEC, 0x00);

	/*mbar2_writeLong(MCFSIM_IDECONFIG1, 0x00000020); */ /* Enable a 1 cycle pre-drive cycle on CS1 */

	/* FlexBus Chipselect */
	init_fbcs();

#ifdef CONFIG_SYS_I2C_FSL
	CONFIG_SYS_I2C_PINMUX_REG =
	    CONFIG_SYS_I2C_PINMUX_REG & CONFIG_SYS_I2C_PINMUX_CLR;
	CONFIG_SYS_I2C_PINMUX_REG |= CONFIG_SYS_I2C_PINMUX_SET;
#ifdef CONFIG_SYS_I2C2_OFFSET
	CONFIG_SYS_I2C2_PINMUX_REG &= CONFIG_SYS_I2C2_PINMUX_CLR;
	CONFIG_SYS_I2C2_PINMUX_REG |= CONFIG_SYS_I2C2_PINMUX_SET;
#endif
#endif

	/* enable instruction cache now */
	icache_enable();
}

/*initialize higher level parts of CPU like timers */
int cpu_init_r(void)
{
	return (0);
}

void uart_port_conf(int port)
{
	u32 *par = (u32 *) MMAP_PAR;

	/* Setup Ports: */
	switch (port) {
	case 1:
		clrbits_be32(par, 0x00180000);
		setbits_be32(par, 0x00180000);
		break;
	case 2:
		clrbits_be32(par, 0x00000003);
		clrbits_be32(par, 0xFFFFFFFC);
		break;
	}
}
#endif				/* #if defined(CONFIG_M5253) */

#if defined(CONFIG_M5271)
void cpu_init_f(void)
{
#ifndef CONFIG_WATCHDOG
	/* Disable the watchdog if we aren't using it */
	mbar_writeShort(MCF_WTM_WCR, 0);
#endif

	/* FlexBus Chipselect */
	init_fbcs();

#ifdef CONFIG_SYS_MCF_SYNCR
	/* Set clockspeed according to board header file */
	mbar_writeLong(MCF_FMPLL_SYNCR, CONFIG_SYS_MCF_SYNCR);
#else
	/* Set clockspeed to 100MHz */
	mbar_writeLong(MCF_FMPLL_SYNCR,
			MCF_FMPLL_SYNCR_MFD(0) | MCF_FMPLL_SYNCR_RFD(0));
#endif
	while (!(mbar_readByte(MCF_FMPLL_SYNSR) & MCF_FMPLL_SYNSR_LOCK)) ;
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
	u16 temp;

	/* Setup Ports: */
	switch (port) {
	case 0:
		temp = mbar_readShort(MCF_GPIO_PAR_UART) & 0xFFF3;
		temp |= (MCF_GPIO_PAR_UART_U0TXD | MCF_GPIO_PAR_UART_U0RXD);
		mbar_writeShort(MCF_GPIO_PAR_UART, temp);
		break;
	case 1:
		temp = mbar_readShort(MCF_GPIO_PAR_UART) & 0xF0FF;
		temp |= (MCF_GPIO_PAR_UART_U1RXD_UART1 | MCF_GPIO_PAR_UART_U1TXD_UART1);
		mbar_writeShort(MCF_GPIO_PAR_UART, temp);
		break;
	case 2:
		temp = mbar_readShort(MCF_GPIO_PAR_UART) & 0xCFFF;
		temp |= (0x3000);
		mbar_writeShort(MCF_GPIO_PAR_UART, temp);
		break;
	}
}

#if defined(CONFIG_CMD_NET)
int fecpin_setclear(struct eth_device *dev, int setclear)
{
	if (setclear) {
		/* Enable Ethernet pins */
		mbar_writeByte(MCF_GPIO_PAR_FECI2C,
			       (mbar_readByte(MCF_GPIO_PAR_FECI2C) | 0xF0));
	} else {
	}

	return 0;
}
#endif				/* CONFIG_CMD_NET */

#endif				/* CONFIG_M5271 */

#if defined(CONFIG_M5272)
/*
 * Breath some life into the CPU...
 *
 * Set up the memory map,
 * initialize a bunch of registers,
 * initialize the UPM's
 */
void cpu_init_f(void)
{
	/* if we come from RAM we assume the CPU is
	 * already initialized.
	 */
#ifndef CONFIG_MONITOR_IS_IN_RAM
	sysctrl_t *sysctrl = (sysctrl_t *) (CONFIG_SYS_MBAR);
	gpio_t *gpio = (gpio_t *) (MMAP_GPIO);
	csctrl_t *csctrl = (csctrl_t *) (MMAP_FBCS);

	out_be16(&sysctrl->sc_scr, CONFIG_SYS_SCR);
	out_be16(&sysctrl->sc_spr, CONFIG_SYS_SPR);

	/* Setup Ports: */
	out_be32(&gpio->gpio_pacnt, CONFIG_SYS_PACNT);
	out_be16(&gpio->gpio_paddr, CONFIG_SYS_PADDR);
	out_be16(&gpio->gpio_padat, CONFIG_SYS_PADAT);
	out_be32(&gpio->gpio_pbcnt, CONFIG_SYS_PBCNT);
	out_be16(&gpio->gpio_pbddr, CONFIG_SYS_PBDDR);
	out_be16(&gpio->gpio_pbdat, CONFIG_SYS_PBDAT);
	out_be32(&gpio->gpio_pdcnt, CONFIG_SYS_PDCNT);

	/* Memory Controller: */
	out_be32(&csctrl->cs_br0, CONFIG_SYS_BR0_PRELIM);
	out_be32(&csctrl->cs_or0, CONFIG_SYS_OR0_PRELIM);

#if (defined(CONFIG_SYS_OR1_PRELIM) && defined(CONFIG_SYS_BR1_PRELIM))
	out_be32(&csctrl->cs_br1, CONFIG_SYS_BR1_PRELIM);
	out_be32(&csctrl->cs_or1, CONFIG_SYS_OR1_PRELIM);
#endif

#if defined(CONFIG_SYS_OR2_PRELIM) && defined(CONFIG_SYS_BR2_PRELIM)
	out_be32(&csctrl->cs_br2, CONFIG_SYS_BR2_PRELIM);
	out_be32(&csctrl->cs_or2, CONFIG_SYS_OR2_PRELIM);
#endif

#if defined(CONFIG_SYS_OR3_PRELIM) && defined(CONFIG_SYS_BR3_PRELIM)
	out_be32(&csctrl->cs_br3, CONFIG_SYS_BR3_PRELIM);
	out_be32(&csctrl->cs_or3, CONFIG_SYS_OR3_PRELIM);
#endif

#if defined(CONFIG_SYS_OR4_PRELIM) && defined(CONFIG_SYS_BR4_PRELIM)
	out_be32(&csctrl->cs_br4, CONFIG_SYS_BR4_PRELIM);
	out_be32(&csctrl->cs_or4, CONFIG_SYS_OR4_PRELIM);
#endif

#if defined(CONFIG_SYS_OR5_PRELIM) && defined(CONFIG_SYS_BR5_PRELIM)
	out_be32(&csctrl->cs_br5, CONFIG_SYS_BR5_PRELIM);
	out_be32(&csctrl->cs_or5, CONFIG_SYS_OR5_PRELIM);
#endif

#if defined(CONFIG_SYS_OR6_PRELIM) && defined(CONFIG_SYS_BR6_PRELIM)
	out_be32(&csctrl->cs_br6, CONFIG_SYS_BR6_PRELIM);
	out_be32(&csctrl->cs_or6, CONFIG_SYS_OR6_PRELIM);
#endif

#if defined(CONFIG_SYS_OR7_PRELIM) && defined(CONFIG_SYS_BR7_PRELIM)
	out_be32(&csctrl->cs_br7, CONFIG_SYS_BR7_PRELIM);
	out_be32(&csctrl->cs_or7, CONFIG_SYS_OR7_PRELIM);
#endif

#endif				/* #ifndef CONFIG_MONITOR_IS_IN_RAM */

	/* enable instruction cache now */
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
		clrbits_be32(&gpio->gpio_pbcnt,
			GPIO_PBCNT_PB0MSK | GPIO_PBCNT_PB1MSK);
		setbits_be32(&gpio->gpio_pbcnt,
			GPIO_PBCNT_URT0_TXD | GPIO_PBCNT_URT0_RXD);
		break;
	case 1:
		clrbits_be32(&gpio->gpio_pdcnt,
			GPIO_PDCNT_PD1MSK | GPIO_PDCNT_PD4MSK);
		setbits_be32(&gpio->gpio_pdcnt,
			GPIO_PDCNT_URT1_RXD | GPIO_PDCNT_URT1_TXD);
		break;
	}
}

#if defined(CONFIG_CMD_NET)
int fecpin_setclear(struct eth_device *dev, int setclear)
{
	gpio_t *gpio = (gpio_t *) MMAP_GPIO;

	if (setclear) {
		setbits_be32(&gpio->gpio_pbcnt,
			GPIO_PBCNT_E_MDC | GPIO_PBCNT_E_RXER |
			GPIO_PBCNT_E_RXD1 | GPIO_PBCNT_E_RXD2 |
			GPIO_PBCNT_E_RXD3 | GPIO_PBCNT_E_TXD1 |
			GPIO_PBCNT_E_TXD2 | GPIO_PBCNT_E_TXD3);
	} else {
	}
	return 0;
}
#endif				/* CONFIG_CMD_NET */
#endif				/* #if defined(CONFIG_M5272) */

#if defined(CONFIG_M5275)

/*
 * Breathe some life into the CPU...
 *
 * Set up the memory map,
 * initialize a bunch of registers,
 * initialize the UPM's
 */
void cpu_init_f(void)
{
	/*
	 * if we come from RAM we assume the CPU is
	 * already initialized.
	 */

#ifndef CONFIG_MONITOR_IS_IN_RAM
	wdog_t *wdog_reg = (wdog_t *) (MMAP_WDOG);
	gpio_t *gpio_reg = (gpio_t *) (MMAP_GPIO);

	/* Kill watchdog so we can initialize the PLL */
	out_be16(&wdog_reg->wcr, 0);

	/* FlexBus Chipselect */
	init_fbcs();
#endif				/* #ifndef CONFIG_MONITOR_IS_IN_RAM */

#ifdef CONFIG_SYS_I2C_FSL
	CONFIG_SYS_I2C_PINMUX_REG &= CONFIG_SYS_I2C_PINMUX_CLR;
	CONFIG_SYS_I2C_PINMUX_REG |= CONFIG_SYS_I2C_PINMUX_SET;
#endif

	/* enable instruction cache now */
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
		clrbits_be16(&gpio->par_uart, UART0_ENABLE_MASK);
		setbits_be16(&gpio->par_uart, UART0_ENABLE_MASK);
		break;
	case 1:
		clrbits_be16(&gpio->par_uart, UART1_ENABLE_MASK);
		setbits_be16(&gpio->par_uart, UART1_ENABLE_MASK);
		break;
	case 2:
		clrbits_be16(&gpio->par_uart, UART2_ENABLE_MASK);
		setbits_be16(&gpio->par_uart, UART2_ENABLE_MASK);
		break;
	}
}

#if defined(CONFIG_CMD_NET)
int fecpin_setclear(struct eth_device *dev, int setclear)
{
	struct fec_info_s *info = (struct fec_info_s *) dev->priv;
	gpio_t *gpio = (gpio_t *)MMAP_GPIO;

	if (setclear) {
		/* Enable Ethernet pins */
		if (info->iobase == CONFIG_SYS_FEC0_IOBASE) {
			setbits_be16(&gpio->par_feci2c, 0x0f00);
			setbits_8(&gpio->par_fec0hl, 0xc0);
		} else {
			setbits_be16(&gpio->par_feci2c, 0x00a0);
			setbits_8(&gpio->par_fec1hl, 0xc0);
		}
	} else {
		if (info->iobase == CONFIG_SYS_FEC0_IOBASE) {
			clrbits_be16(&gpio->par_feci2c, 0x0f00);
			clrbits_8(&gpio->par_fec0hl, 0xc0);
		} else {
			clrbits_be16(&gpio->par_feci2c, 0x00a0);
			clrbits_8(&gpio->par_fec1hl, 0xc0);
		}
	}

	return 0;
}
#endif				/* CONFIG_CMD_NET */
#endif				/* #if defined(CONFIG_M5275) */

#if defined(CONFIG_M5282)
/*
 * Breath some life into the CPU...
 *
 * Set up the memory map,
 * initialize a bunch of registers,
 * initialize the UPM's
 */
void cpu_init_f(void)
{
#ifndef CONFIG_WATCHDOG
	/* disable watchdog if we aren't using it */
	MCFWTM_WCR = 0;
#endif

#ifndef CONFIG_MONITOR_IS_IN_RAM
	/* Set speed /PLL */
	MCFCLOCK_SYNCR =
	    MCFCLOCK_SYNCR_MFD(CONFIG_SYS_MFD) |
	    MCFCLOCK_SYNCR_RFD(CONFIG_SYS_RFD);
	while (!(MCFCLOCK_SYNSR & MCFCLOCK_SYNSR_LOCK)) ;

	MCFGPIO_PBCDPAR = 0xc0;

	/* Set up the GPIO ports */
#ifdef CONFIG_SYS_PEPAR
	MCFGPIO_PEPAR = CONFIG_SYS_PEPAR;
#endif
#ifdef	CONFIG_SYS_PFPAR
	MCFGPIO_PFPAR = CONFIG_SYS_PFPAR;
#endif
#ifdef CONFIG_SYS_PJPAR
	MCFGPIO_PJPAR = CONFIG_SYS_PJPAR;
#endif
#ifdef CONFIG_SYS_PSDPAR
	MCFGPIO_PSDPAR = CONFIG_SYS_PSDPAR;
#endif
#ifdef CONFIG_SYS_PASPAR
	MCFGPIO_PASPAR = CONFIG_SYS_PASPAR;
#endif
#ifdef CONFIG_SYS_PEHLPAR
	MCFGPIO_PEHLPAR = CONFIG_SYS_PEHLPAR;
#endif
#ifdef CONFIG_SYS_PQSPAR
	MCFGPIO_PQSPAR = CONFIG_SYS_PQSPAR;
#endif
#ifdef CONFIG_SYS_PTCPAR
	MCFGPIO_PTCPAR = CONFIG_SYS_PTCPAR;
#endif
#if defined(CONFIG_SYS_PORTTC)
	MCFGPIO_PORTTC = CONFIG_SYS_PORTTC;
#endif
#if defined(CONFIG_SYS_DDRTC)
	MCFGPIO_DDRTC  = CONFIG_SYS_DDRTC;
#endif
#ifdef CONFIG_SYS_PTDPAR
	MCFGPIO_PTDPAR = CONFIG_SYS_PTDPAR;
#endif
#ifdef CONFIG_SYS_PUAPAR
	MCFGPIO_PUAPAR = CONFIG_SYS_PUAPAR;
#endif

#if defined(CONFIG_SYS_DDRD)
	MCFGPIO_DDRD = CONFIG_SYS_DDRD;
#endif
#ifdef CONFIG_SYS_DDRUA
	MCFGPIO_DDRUA = CONFIG_SYS_DDRUA;
#endif

	/* FlexBus Chipselect */
	init_fbcs();

#endif				/* CONFIG_MONITOR_IS_IN_RAM */

	/* defer enabling cache until boot (see do_go) */
	/* icache_enable(); */
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
	/* Setup Ports: */
	switch (port) {
	case 0:
		MCFGPIO_PUAPAR &= 0xFc;
		MCFGPIO_PUAPAR |= 0x03;
		break;
	case 1:
		MCFGPIO_PUAPAR &= 0xF3;
		MCFGPIO_PUAPAR |= 0x0C;
		break;
	case 2:
		MCFGPIO_PASPAR &= 0xFF0F;
		MCFGPIO_PASPAR |= 0x00A0;
		break;
	}
}

#if defined(CONFIG_CMD_NET)
int fecpin_setclear(struct eth_device *dev, int setclear)
{
	if (setclear) {
		MCFGPIO_PASPAR |= 0x0F00;
		MCFGPIO_PEHLPAR = CONFIG_SYS_PEHLPAR;
	} else {
		MCFGPIO_PASPAR &= 0xF0FF;
		MCFGPIO_PEHLPAR &= ~CONFIG_SYS_PEHLPAR;
	}
	return 0;
}
#endif			/* CONFIG_CMD_NET */
#endif

#if defined(CONFIG_M5249)
/*
 * Breath some life into the CPU...
 *
 * Set up the memory map,
 * initialize a bunch of registers,
 * initialize the UPM's
 */
void cpu_init_f(void)
{
	/*
	 *  NOTE: by setting the GPIO_FUNCTION registers, we ensure that the UART pins
	 *        (UART0: gpio 30,27, UART1: gpio 31, 28) will be used as UART pins
	 *        which is their primary function.
	 *        ~Jeremy
	 */
	mbar2_writeLong(MCFSIM_GPIO_FUNC, CONFIG_SYS_GPIO_FUNC);
	mbar2_writeLong(MCFSIM_GPIO1_FUNC, CONFIG_SYS_GPIO1_FUNC);
	mbar2_writeLong(MCFSIM_GPIO_EN, CONFIG_SYS_GPIO_EN);
	mbar2_writeLong(MCFSIM_GPIO1_EN, CONFIG_SYS_GPIO1_EN);
	mbar2_writeLong(MCFSIM_GPIO_OUT, CONFIG_SYS_GPIO_OUT);
	mbar2_writeLong(MCFSIM_GPIO1_OUT, CONFIG_SYS_GPIO1_OUT);

	/*
	 *  dBug Compliance:
	 *    You can verify these values by using dBug's 'ird'
	 *    (Internal Register Display) command
	 *    ~Jeremy
	 *
	 */
	mbar_writeByte(MCFSIM_MPARK, 0x30);	/* 5249 Internal Core takes priority over DMA */
	mbar_writeByte(MCFSIM_SYPCR, 0x00);
	mbar_writeByte(MCFSIM_SWIVR, 0x0f);
	mbar_writeByte(MCFSIM_SWSR, 0x00);
	mbar_writeLong(MCFSIM_IMR, 0xfffffbff);
	mbar_writeByte(MCFSIM_SWDICR, 0x00);
	mbar_writeByte(MCFSIM_TIMER1ICR, 0x00);
	mbar_writeByte(MCFSIM_TIMER2ICR, 0x88);
	mbar_writeByte(MCFSIM_I2CICR, 0x00);
	mbar_writeByte(MCFSIM_UART1ICR, 0x00);
	mbar_writeByte(MCFSIM_UART2ICR, 0x00);
	mbar_writeByte(MCFSIM_ICR6, 0x00);
	mbar_writeByte(MCFSIM_ICR7, 0x00);
	mbar_writeByte(MCFSIM_ICR8, 0x00);
	mbar_writeByte(MCFSIM_ICR9, 0x00);
	mbar_writeByte(MCFSIM_QSPIICR, 0x00);

	mbar2_writeLong(MCFSIM_GPIO_INT_EN, 0x00000080);
	mbar2_writeByte(MCFSIM_INTBASE, 0x40);	/* Base interrupts at 64 */
	mbar2_writeByte(MCFSIM_SPURVEC, 0x00);
	mbar2_writeLong(MCFSIM_IDECONFIG1, 0x00000020);	/* Enable a 1 cycle pre-drive cycle on CS1 */

	/* Setup interrupt priorities for gpio7 */
	/* mbar2_writeLong(MCFSIM_INTLEV5, 0x70000000); */

	/* IDE Config registers */
	mbar2_writeLong(MCFSIM_IDECONFIG1, 0x00000020);
	mbar2_writeLong(MCFSIM_IDECONFIG2, 0x00000000);

	/* FlexBus Chipselect */
	init_fbcs();

	/* enable instruction cache now */
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
}
#endif				/* #if defined(CONFIG_M5249) */
