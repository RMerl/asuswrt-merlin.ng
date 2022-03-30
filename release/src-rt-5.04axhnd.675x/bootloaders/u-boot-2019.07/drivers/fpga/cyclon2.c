// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2006
 * Heiko Schocher, hs@denx.de
 * Based on ACE1XK.c
 */

#include <common.h>		/* core U-Boot definitions */
#include <altera.h>
#include <ACEX1K.h>		/* ACEX device family */

/* Define FPGA_DEBUG to get debug printf's */
#ifdef	FPGA_DEBUG
#define PRINTF(fmt,args...)	printf (fmt ,##args)
#else
#define PRINTF(fmt,args...)
#endif

/* Note: The assumption is that we cannot possibly run fast enough to
 * overrun the device (the Slave Parallel mode can free run at 50MHz).
 * If there is a need to operate slower, define CONFIG_FPGA_DELAY in
 * the board config file to slow things down.
 */
#ifndef CONFIG_FPGA_DELAY
#define CONFIG_FPGA_DELAY()
#endif

#ifndef CONFIG_SYS_FPGA_WAIT
#define CONFIG_SYS_FPGA_WAIT CONFIG_SYS_HZ/10		/* 100 ms */
#endif

static int CYC2_ps_load(Altera_desc *desc, const void *buf, size_t bsize);
static int CYC2_ps_dump(Altera_desc *desc, const void *buf, size_t bsize);
/* static int CYC2_ps_info( Altera_desc *desc ); */

/* ------------------------------------------------------------------------- */
/* CYCLON2 Generic Implementation */
int CYC2_load(Altera_desc *desc, const void *buf, size_t bsize)
{
	int ret_val = FPGA_FAIL;

	switch (desc->iface) {
	case passive_serial:
		PRINTF ("%s: Launching Passive Serial Loader\n", __FUNCTION__);
		ret_val = CYC2_ps_load (desc, buf, bsize);
		break;

	case fast_passive_parallel:
		/* Fast Passive Parallel (FPP) and PS only differ in what is
		 * done in the write() callback. Use the existing PS load
		 * function for FPP, too.
		 */
		PRINTF ("%s: Launching Fast Passive Parallel Loader\n",
		      __FUNCTION__);
		ret_val = CYC2_ps_load(desc, buf, bsize);
		break;

		/* Add new interface types here */

	default:
		printf ("%s: Unsupported interface type, %d\n",
				__FUNCTION__, desc->iface);
	}

	return ret_val;
}

int CYC2_dump(Altera_desc *desc, const void *buf, size_t bsize)
{
	int ret_val = FPGA_FAIL;

	switch (desc->iface) {
	case passive_serial:
		PRINTF ("%s: Launching Passive Serial Dump\n", __FUNCTION__);
		ret_val = CYC2_ps_dump (desc, buf, bsize);
		break;

		/* Add new interface types here */

	default:
		printf ("%s: Unsupported interface type, %d\n",
				__FUNCTION__, desc->iface);
	}

	return ret_val;
}

int CYC2_info( Altera_desc *desc )
{
	return FPGA_SUCCESS;
}

/* ------------------------------------------------------------------------- */
/* CYCLON2 Passive Serial Generic Implementation                                  */
static int CYC2_ps_load(Altera_desc *desc, const void *buf, size_t bsize)
{
	int ret_val = FPGA_FAIL;	/* assume the worst */
	Altera_CYC2_Passive_Serial_fns *fn = desc->iface_fns;
	int	ret = 0;

	PRINTF ("%s: start with interface functions @ 0x%p\n",
			__FUNCTION__, fn);

	if (fn) {
		int cookie = desc->cookie;	/* make a local copy */
		unsigned long ts;		/* timestamp */

		PRINTF ("%s: Function Table:\n"
				"ptr:\t0x%p\n"
				"struct: 0x%p\n"
				"config:\t0x%p\n"
				"status:\t0x%p\n"
				"write:\t0x%p\n"
				"done:\t0x%p\n\n",
				__FUNCTION__, &fn, fn, fn->config, fn->status,
				fn->write, fn->done);
#ifdef CONFIG_SYS_FPGA_PROG_FEEDBACK
		printf ("Loading FPGA Device %d...", cookie);
#endif

		/*
		 * Run the pre configuration function if there is one.
		 */
		if (*fn->pre) {
			(*fn->pre) (cookie);
		}

		/* Establish the initial state */
		(*fn->config) (false, true, cookie);	/* De-assert nCONFIG */
		udelay(100);
		(*fn->config) (true, true, cookie);	/* Assert nCONFIG */

		udelay(2);		/* T_cfg > 2us	*/

		/* Wait for nSTATUS to be asserted */
		ts = get_timer (0);		/* get current time */
		do {
			CONFIG_FPGA_DELAY ();
			if (get_timer (ts) > CONFIG_SYS_FPGA_WAIT) {	/* check the time */
				puts ("** Timeout waiting for STATUS to go high.\n");
				(*fn->abort) (cookie);
				return FPGA_FAIL;
			}
		} while (!(*fn->status) (cookie));

		/* Get ready for the burn */
		CONFIG_FPGA_DELAY ();

		ret = (*fn->write) (buf, bsize, true, cookie);
		if (ret) {
			puts ("** Write failed.\n");
			(*fn->abort) (cookie);
			return FPGA_FAIL;
		}
#ifdef CONFIG_SYS_FPGA_PROG_FEEDBACK
		puts(" OK? ...");
#endif

		CONFIG_FPGA_DELAY ();

#ifdef CONFIG_SYS_FPGA_PROG_FEEDBACK
		putc (' ');			/* terminate the dotted line */
#endif

	/*
	 * Checking FPGA's CONF_DONE signal - correctly booted ?
	 */

	if ( ! (*fn->done) (cookie) ) {
		puts ("** Booting failed! CONF_DONE is still deasserted.\n");
		(*fn->abort) (cookie);
		return (FPGA_FAIL);
	}
#ifdef CONFIG_SYS_FPGA_PROG_FEEDBACK
	puts(" OK\n");
#endif

	ret_val = FPGA_SUCCESS;

#ifdef CONFIG_SYS_FPGA_PROG_FEEDBACK
	if (ret_val == FPGA_SUCCESS) {
		puts ("Done.\n");
	}
	else {
		puts ("Fail.\n");
	}
#endif
	(*fn->post) (cookie);

	} else {
		printf ("%s: NULL Interface function table!\n", __FUNCTION__);
	}

	return ret_val;
}

static int CYC2_ps_dump(Altera_desc *desc, const void *buf, size_t bsize)
{
	/* Readback is only available through the Slave Parallel and         */
	/* boundary-scan interfaces.                                         */
	printf ("%s: Passive Serial Dumping is unavailable\n",
			__FUNCTION__);
	return FPGA_FAIL;
}
