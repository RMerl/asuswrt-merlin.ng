// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2006
 * Wolfgang Wegner, ASTRO Strobel Kommunikationssysteme GmbH,
 * w.wegner@astro-kom.de
 *
 * based on the files by
 * Heiko Schocher, DENX Software Engineering, hs@denx.de
 * and
 * Rich Ireland, Enterasys Networks, rireland@enterasys.com.
 * Keith Outwater, keith_outwater@mvis.com.
 */

/* Altera/Xilinx FPGA configuration support for the ASTRO "URMEL" board */

#include <common.h>
#include <console.h>
#include <watchdog.h>
#include <altera.h>
#include <ACEX1K.h>
#include <spartan3.h>
#include <command.h>
#include <asm/immap_5329.h>
#include <asm/io.h>
#include "fpga.h"

int altera_pre_fn(int cookie)
{
	gpio_t *gpiop = (gpio_t *)MMAP_GPIO;
	unsigned char tmp_char;
	unsigned short tmp_short;

	/* first, set the required pins to GPIO function */
	/* PAR_T0IN -> GPIO */
	tmp_char = readb(&gpiop->par_timer);
	tmp_char &= 0xfc;
	writeb(tmp_char, &gpiop->par_timer);
	/* all QSPI pins -> GPIO */
	writew(0x0000, &gpiop->par_qspi);
	/* U0RTS, U0CTS -> GPIO */
	tmp_short = __raw_readw(&gpiop->par_uart);
	tmp_short &= 0xfff3;
	__raw_writew(tmp_short, &gpiop->par_uart);
	/* all PWM pins -> GPIO */
	writeb(0x00, &gpiop->par_pwm);
	/* next, set data direction registers */
	writeb(0x01, &gpiop->pddr_timer);
	writeb(0x25, &gpiop->pddr_qspi);
	writeb(0x0c, &gpiop->pddr_uart);
	writeb(0x04, &gpiop->pddr_pwm);

	/* ensure other SPI peripherals are deselected */
	writeb(0x08, &gpiop->ppd_uart);
	writeb(0x38, &gpiop->ppd_qspi);

	/* CONFIG = 0 STATUS = 0 -> FPGA in reset state */
	writeb(0xFB, &gpiop->pclrr_uart);
	/* enable Altera configuration by clearing QSPI_CS2 and DT0IN */
	writeb(0xFE, &gpiop->pclrr_timer);
	writeb(0xDF, &gpiop->pclrr_qspi);
	return FPGA_SUCCESS;
}

/* Set the state of CONFIG Pin */
int altera_config_fn(int assert_config, int flush, int cookie)
{
	gpio_t *gpiop = (gpio_t *)MMAP_GPIO;

	if (assert_config)
		writeb(0x04, &gpiop->ppd_uart);
	else
		writeb(0xFB, &gpiop->pclrr_uart);
	return FPGA_SUCCESS;
}

/* Returns the state of STATUS Pin */
int altera_status_fn(int cookie)
{
	gpio_t *gpiop = (gpio_t *)MMAP_GPIO;

	if (readb(&gpiop->ppd_pwm) & 0x08)
		return FPGA_FAIL;
	return FPGA_SUCCESS;
}

/* Returns the state of CONF_DONE Pin */
int altera_done_fn(int cookie)
{
	gpio_t *gpiop = (gpio_t *)MMAP_GPIO;

	if (readb(&gpiop->ppd_pwm) & 0x20)
		return FPGA_FAIL;
	return FPGA_SUCCESS;
}

/*
 * writes the complete buffer to the FPGA
 * writing the complete buffer in one function is much faster,
 * then calling it for every bit
 */
int altera_write_fn(const void *buf, size_t len, int flush, int cookie)
{
	size_t bytecount = 0;
	gpio_t *gpiop = (gpio_t *)MMAP_GPIO;
	unsigned char *data = (unsigned char *)buf;
	unsigned char val = 0;
	int i;
	int len_40 = len / 40;

	while (bytecount < len) {
		val = data[bytecount++];
		i = 8;
		do {
			writeb(0xFB, &gpiop->pclrr_qspi);
			if (val & 0x01)
				writeb(0x01, &gpiop->ppd_qspi);
			else
				writeb(0xFE, &gpiop->pclrr_qspi);
			writeb(0x04, &gpiop->ppd_qspi);
			val >>= 1;
			i--;
		} while (i > 0);

		if (bytecount % len_40 == 0) {
#if defined(CONFIG_HW_WATCHDOG) || defined(CONFIG_WATCHDOG)
			WATCHDOG_RESET();
#endif
#ifdef CONFIG_SYS_FPGA_PROG_FEEDBACK
			putc('.');	/* let them know we are alive */
#endif
#ifdef CONFIG_SYS_FPGA_CHECK_CTRLC
			if (ctrlc())
				return FPGA_FAIL;
#endif
		}
	}
	return FPGA_SUCCESS;
}

/* called, when programming is aborted */
int altera_abort_fn(int cookie)
{
	gpio_t *gpiop = (gpio_t *)MMAP_GPIO;

	writeb(0x20, &gpiop->ppd_qspi);
	writeb(0x08, &gpiop->ppd_uart);
	return FPGA_SUCCESS;
}

/* called, when programming was succesful */
int altera_post_fn(int cookie)
{
	return altera_abort_fn(cookie);
}

/*
 * Note that these are pointers to code that is in Flash. They will be
 * relocated at runtime.
 * FIXME: relocation not yet working for coldfire, see below!
 */
Altera_CYC2_Passive_Serial_fns altera_fns = {
	altera_pre_fn,
	altera_config_fn,
	altera_status_fn,
	altera_done_fn,
	altera_write_fn,
	altera_abort_fn,
	altera_post_fn
};

Altera_desc altera_fpga[CONFIG_FPGA_COUNT] = {
	{Altera_CYC2,
	 passive_serial,
	 85903,
	 (void *)&altera_fns,
	 NULL,
	 0}
};

/* Initialize the fpga.  Return 1 on success, 0 on failure. */
int astro5373l_altera_load(void)
{
	int i;

	for (i = 0; i < CONFIG_FPGA_COUNT; i++) {
		/*
		 * I did not yet manage to get relocation work properly,
		 * so set stuff here instead of static initialisation:
		 */
		altera_fns.pre = altera_pre_fn;
		altera_fns.config = altera_config_fn;
		altera_fns.status = altera_status_fn;
		altera_fns.done = altera_done_fn;
		altera_fns.write = altera_write_fn;
		altera_fns.abort = altera_abort_fn;
		altera_fns.post = altera_post_fn;
		altera_fpga[i].iface_fns = (void *)&altera_fns;
		fpga_add(fpga_altera, &altera_fpga[i]);
	}
	return 1;
}

/* Set the FPGA's PROG_B line to the specified level */
int xilinx_pgm_config_fn(int assert, int flush, int cookie)
{
	gpio_t *gpiop = (gpio_t *)MMAP_GPIO;

	if (assert)
		writeb(0xFB, &gpiop->pclrr_uart);
	else
		writeb(0x04, &gpiop->ppd_uart);
	return assert;
}

/*
 * Test the state of the active-low FPGA INIT line.  Return 1 on INIT
 * asserted (low).
 */
int xilinx_init_config_fn(int cookie)
{
	gpio_t *gpiop = (gpio_t *)MMAP_GPIO;

	return (readb(&gpiop->ppd_pwm) & 0x08) == 0;
}

/* Test the state of the active-high FPGA DONE pin */
int xilinx_done_config_fn(int cookie)
{
	gpio_t *gpiop = (gpio_t *)MMAP_GPIO;

	return (readb(&gpiop->ppd_pwm) & 0x20) >> 5;
}

/* Abort an FPGA operation */
int xilinx_abort_config_fn(int cookie)
{
	gpio_t *gpiop = (gpio_t *)MMAP_GPIO;
	/* ensure all SPI peripherals and FPGAs are deselected */
	writeb(0x08, &gpiop->ppd_uart);
	writeb(0x01, &gpiop->ppd_timer);
	writeb(0x38, &gpiop->ppd_qspi);
	return FPGA_FAIL;
}

/*
 * FPGA pre-configuration function. Just make sure that
 * FPGA reset is asserted to keep the FPGA from starting up after
 * configuration.
 */
int xilinx_pre_config_fn(int cookie)
{
	gpio_t *gpiop = (gpio_t *)MMAP_GPIO;
	unsigned char tmp_char;
	unsigned short tmp_short;

	/* first, set the required pins to GPIO function */
	/* PAR_T0IN -> GPIO */
	tmp_char = readb(&gpiop->par_timer);
	tmp_char &= 0xfc;
	writeb(tmp_char, &gpiop->par_timer);
	/* all QSPI pins -> GPIO */
	writew(0x0000, &gpiop->par_qspi);
	/* U0RTS, U0CTS -> GPIO */
	tmp_short = __raw_readw(&gpiop->par_uart);
	tmp_short &= 0xfff3;
	__raw_writew(tmp_short, &gpiop->par_uart);
	/* all PWM pins -> GPIO */
	writeb(0x00, &gpiop->par_pwm);
	/* next, set data direction registers */
	writeb(0x01, &gpiop->pddr_timer);
	writeb(0x25, &gpiop->pddr_qspi);
	writeb(0x0c, &gpiop->pddr_uart);
	writeb(0x04, &gpiop->pddr_pwm);

	/* ensure other SPI peripherals are deselected */
	writeb(0x08, &gpiop->ppd_uart);
	writeb(0x38, &gpiop->ppd_qspi);
	writeb(0x01, &gpiop->ppd_timer);

	/* CONFIG = 0, STATUS = 0 -> FPGA in reset state */
	writeb(0xFB, &gpiop->pclrr_uart);
	/* enable Xilinx configuration by clearing QSPI_CS2 and U0CTS */
	writeb(0xF7, &gpiop->pclrr_uart);
	writeb(0xDF, &gpiop->pclrr_qspi);
	return 0;
}

/*
 * FPGA post configuration function. Should perform a test if FPGA is running.
 */
int xilinx_post_config_fn(int cookie)
{
	int rc = 0;

	/*
	 * no test yet
	 */
	return rc;
}

int xilinx_clk_config_fn(int assert_clk, int flush, int cookie)
{
	gpio_t *gpiop = (gpio_t *)MMAP_GPIO;

	if (assert_clk)
		writeb(0x04, &gpiop->ppd_qspi);
	else
		writeb(0xFB, &gpiop->pclrr_qspi);
	return assert_clk;
}

int xilinx_wr_config_fn(int assert_write, int flush, int cookie)
{
	gpio_t *gpiop = (gpio_t *)MMAP_GPIO;

	if (assert_write)
		writeb(0x01, &gpiop->ppd_qspi);
	else
		writeb(0xFE, &gpiop->pclrr_qspi);
	return assert_write;
}

int xilinx_fastwr_config_fn(void *buf, size_t len, int flush, int cookie)
{
	size_t bytecount = 0;
	gpio_t *gpiop = (gpio_t *)MMAP_GPIO;
	unsigned char *data = (unsigned char *)buf;
	unsigned char val = 0;
	int i;
	int len_40 = len / 40;

	for (bytecount = 0; bytecount < len; bytecount++) {
		val = *(data++);
		for (i = 8; i > 0; i--) {
			writeb(0xFB, &gpiop->pclrr_qspi);
			if (val & 0x80)
				writeb(0x01, &gpiop->ppd_qspi);
			else
				writeb(0xFE, &gpiop->pclrr_qspi);
			writeb(0x04, &gpiop->ppd_qspi);
			val <<= 1;
		}
		if (bytecount % len_40 == 0) {
#if defined(CONFIG_HW_WATCHDOG) || defined(CONFIG_WATCHDOG)
			WATCHDOG_RESET();
#endif
#ifdef CONFIG_SYS_FPGA_PROG_FEEDBACK
			putc('.');	/* let them know we are alive */
#endif
#ifdef CONFIG_SYS_FPGA_CHECK_CTRLC
			if (ctrlc())
				return FPGA_FAIL;
#endif
		}
	}
	return FPGA_SUCCESS;
}

/*
 * Note that these are pointers to code that is in Flash.  They will be
 * relocated at runtime.
 * FIXME: relocation not yet working for coldfire, see below!
 */
xilinx_spartan3_slave_serial_fns xilinx_fns = {
	xilinx_pre_config_fn,
	xilinx_pgm_config_fn,
	xilinx_clk_config_fn,
	xilinx_init_config_fn,
	xilinx_done_config_fn,
	xilinx_wr_config_fn,
	0,
	xilinx_fastwr_config_fn
};

xilinx_desc xilinx_fpga[CONFIG_FPGA_COUNT] = {
	{xilinx_spartan3,
	 slave_serial,
	 XILINX_XC3S4000_SIZE,
	 (void *)&xilinx_fns,
	 0,
	 &spartan3_op}
};

/* Initialize the fpga.  Return 1 on success, 0 on failure. */
int astro5373l_xilinx_load(void)
{
	int i;

	fpga_init();

	for (i = 0; i < CONFIG_FPGA_COUNT; i++) {
		/*
		 * I did not yet manage to get relocation work properly,
		 * so set stuff here instead of static initialisation:
		 */
		xilinx_fns.pre = xilinx_pre_config_fn;
		xilinx_fns.pgm = xilinx_pgm_config_fn;
		xilinx_fns.clk = xilinx_clk_config_fn;
		xilinx_fns.init = xilinx_init_config_fn;
		xilinx_fns.done = xilinx_done_config_fn;
		xilinx_fns.wr = xilinx_wr_config_fn;
		xilinx_fns.bwr = xilinx_fastwr_config_fn;
		xilinx_fpga[i].iface_fns = (void *)&xilinx_fns;
		fpga_add(fpga_xilinx, &xilinx_fpga[i]);
	}
	return 1;
}
