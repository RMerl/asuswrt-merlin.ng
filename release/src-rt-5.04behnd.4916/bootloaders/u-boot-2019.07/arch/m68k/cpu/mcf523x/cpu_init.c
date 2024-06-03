// SPDX-License-Identifier: GPL-2.0+
/*
 *
 * (C) Copyright 2000-2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2007, 2012 Freescale Semiconductor, Inc.
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

/* The registers in fbcs_t struct can be 16-bit for CONFIG_M5235 or 32-bit wide otherwise. */
#ifdef CONFIG_M5235
#define out_be_fbcs_reg		out_be16
#else
#define out_be_fbcs_reg		out_be32
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
	fbcs_t *fbcs = (fbcs_t *) MMAP_FBCS;
	wdog_t *wdog = (wdog_t *) MMAP_WDOG;
	scm_t *scm = (scm_t *) MMAP_SCM;

	/* watchdog is enabled by default - disable the watchdog */
#ifndef CONFIG_WATCHDOG
	out_be16(&wdog->cr, 0);
#endif

	out_be32(&scm->rambar, CONFIG_SYS_INIT_RAM_ADDR | SCM_RAMBAR_BDE);

	/* Port configuration */
	out_8(&gpio->par_cs, 0);

#if (defined(CONFIG_SYS_CS0_BASE) && defined(CONFIG_SYS_CS0_MASK) && defined(CONFIG_SYS_CS0_CTRL))
	out_be_fbcs_reg(&fbcs->csar0, CONFIG_SYS_CS0_BASE);
	out_be_fbcs_reg(&fbcs->cscr0, CONFIG_SYS_CS0_CTRL);
	out_be32(&fbcs->csmr0, CONFIG_SYS_CS0_MASK);
#endif

#if (defined(CONFIG_SYS_CS1_BASE) && defined(CONFIG_SYS_CS1_MASK) && defined(CONFIG_SYS_CS1_CTRL))
	setbits_8(&gpio->par_cs, GPIO_PAR_CS_CS1);
	out_be_fbcs_reg(&fbcs->csar1, CONFIG_SYS_CS1_BASE);
	out_be_fbcs_reg(&fbcs->cscr1, CONFIG_SYS_CS1_CTRL);
	out_be32(&fbcs->csmr1, CONFIG_SYS_CS1_MASK);
#endif

#if (defined(CONFIG_SYS_CS2_BASE) && defined(CONFIG_SYS_CS2_MASK) && defined(CONFIG_SYS_CS2_CTRL))
	setbits_8(&gpio->par_cs, GPIO_PAR_CS_CS2);
	out_be_fbcs_reg(&fbcs->csar2, CONFIG_SYS_CS2_BASE);
	out_be_fbcs_reg(&fbcs->cscr2, CONFIG_SYS_CS2_CTRL);
	out_be32(&fbcs->csmr2, CONFIG_SYS_CS2_MASK);
#endif

#if (defined(CONFIG_SYS_CS3_BASE) && defined(CONFIG_SYS_CS3_MASK) && defined(CONFIG_SYS_CS3_CTRL))
	setbits_8(&gpio->par_cs, GPIO_PAR_CS_CS3);
	out_be_fbcs_reg(&fbcs->csar3, CONFIG_SYS_CS3_BASE);
	out_be_fbcs_reg(&fbcs->cscr3, CONFIG_SYS_CS3_CTRL);
	out_be32(&fbcs->csmr3, CONFIG_SYS_CS3_MASK);
#endif

#if (defined(CONFIG_SYS_CS4_BASE) && defined(CONFIG_SYS_CS4_MASK) && defined(CONFIG_SYS_CS4_CTRL))
	setbits_8(&gpio->par_cs, GPIO_PAR_CS_CS4);
	out_be_fbcs_reg(&fbcs->csar4, CONFIG_SYS_CS4_BASE);
	out_be_fbcs_reg(&fbcs->cscr4, CONFIG_SYS_CS4_CTRL);
	out_be32(&fbcs->csmr4, CONFIG_SYS_CS4_MASK);
#endif

#if (defined(CONFIG_SYS_CS5_BASE) && defined(CONFIG_SYS_CS5_MASK) && defined(CONFIG_SYS_CS5_CTRL))
	setbits_8(&gpio->par_cs, GPIO_PAR_CS_CS5);
	out_be_fbcs_reg(&fbcs->csar5, CONFIG_SYS_CS5_BASE);
	out_be_fbcs_reg(&fbcs->cscr5, CONFIG_SYS_CS5_CTRL);
	out_be32(&fbcs->csmr5, CONFIG_SYS_CS5_MASK);
#endif

#if (defined(CONFIG_SYS_CS6_BASE) && defined(CONFIG_SYS_CS6_MASK) && defined(CONFIG_SYS_CS6_CTRL))
	setbits_8(&gpio->par_cs, GPIO_PAR_CS_CS6);
	out_be_fbcs_reg(&fbcs->csar6, CONFIG_SYS_CS6_BASE);
	out_be_fbcs_reg(&fbcs->cscr6, CONFIG_SYS_CS6_CTRL);
	out_be32(&fbcs->csmr6, CONFIG_SYS_CS6_MASK);
#endif

#if (defined(CONFIG_SYS_CS7_BASE) && defined(CONFIG_SYS_CS7_MASK) && defined(CONFIG_SYS_CS7_CTRL))
	setbits_8(&gpio->par_cs, GPIO_PAR_CS_CS7);
	out_be_fbcs_reg(&fbcs->csar7, CONFIG_SYS_CS7_BASE);
	out_be_fbcs_reg(&fbcs->cscr7, CONFIG_SYS_CS7_CTRL);
	out_be32(&fbcs->csmr7, CONFIG_SYS_CS7_MASK);
#endif

#ifdef CONFIG_SYS_I2C_FSL
	CONFIG_SYS_I2C_PINMUX_REG &= CONFIG_SYS_I2C_PINMUX_CLR;
	CONFIG_SYS_I2C_PINMUX_REG |= CONFIG_SYS_I2C_PINMUX_SET;
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
			GPIO_PAR_UART_U0RXD | GPIO_PAR_UART_U0TXD);
		setbits_be16(&gpio->par_uart,
			GPIO_PAR_UART_U0RXD | GPIO_PAR_UART_U0TXD);
		break;
	case 1:
		clrbits_be16(&gpio->par_uart,
			GPIO_PAR_UART_U1RXD_MASK | GPIO_PAR_UART_U1TXD_MASK);
		setbits_be16(&gpio->par_uart,
			GPIO_PAR_UART_U1RXD_U1RXD | GPIO_PAR_UART_U1TXD_U1TXD);
		break;
	case 2:
#ifdef CONFIG_SYS_UART2_PRI_GPIO
		clrbits_be16(&gpio->par_uart,
			GPIO_PAR_UART_U2RXD | GPIO_PAR_UART_U2TXD);
		setbits_be16(&gpio->par_uart,
			GPIO_PAR_UART_U2RXD | GPIO_PAR_UART_U2TXD);
#elif defined(CONFIG_SYS_UART2_ALT1_GPIO)
		clrbits_8(&gpio->par_feci2c,
			GPIO_PAR_FECI2C_EMDC_MASK | GPIO_PAR_FECI2C_EMDIO_MASK);
		setbits_8(&gpio->par_feci2c,
			GPIO_PAR_FECI2C_EMDC_U2TXD | GPIO_PAR_FECI2C_EMDIO_U2RXD);
#endif
		break;
	}
}

#if defined(CONFIG_CMD_NET)
int fecpin_setclear(struct eth_device *dev, int setclear)
{
	gpio_t *gpio = (gpio_t *) MMAP_GPIO;

	if (setclear) {
		setbits_8(&gpio->par_feci2c,
			GPIO_PAR_FECI2C_EMDC_FECEMDC |
			GPIO_PAR_FECI2C_EMDIO_FECEMDIO);
	} else {
		clrbits_8(&gpio->par_feci2c,
			GPIO_PAR_FECI2C_EMDC_MASK |
			GPIO_PAR_FECI2C_EMDIO_MASK);
	}

	return 0;
}
#endif
