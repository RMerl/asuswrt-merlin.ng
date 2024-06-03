// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2003
 * Steven Scholz, imc Measurement & Control, steven.scholz@imc-berlin.de
 *
 * (C) Copyright 2002
 * Rich Ireland, Enterasys Networks, rireland@enterasys.com.
 */

#include <common.h>		/* core U-Boot definitions */
#include <console.h>
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

static int ACEX1K_ps_load(Altera_desc *desc, const void *buf, size_t bsize);
static int ACEX1K_ps_dump(Altera_desc *desc, const void *buf, size_t bsize);
/* static int ACEX1K_ps_info(Altera_desc *desc); */

/* ------------------------------------------------------------------------- */
/* ACEX1K Generic Implementation */
int ACEX1K_load(Altera_desc *desc, const void *buf, size_t bsize)
{
	int ret_val = FPGA_FAIL;

	switch (desc->iface) {
	case passive_serial:
		PRINTF ("%s: Launching Passive Serial Loader\n", __FUNCTION__);
		ret_val = ACEX1K_ps_load (desc, buf, bsize);
		break;

		/* Add new interface types here */

	default:
		printf ("%s: Unsupported interface type, %d\n",
				__FUNCTION__, desc->iface);
	}

	return ret_val;
}

int ACEX1K_dump(Altera_desc *desc, const void *buf, size_t bsize)
{
	int ret_val = FPGA_FAIL;

	switch (desc->iface) {
	case passive_serial:
		PRINTF ("%s: Launching Passive Serial Dump\n", __FUNCTION__);
		ret_val = ACEX1K_ps_dump (desc, buf, bsize);
		break;

		/* Add new interface types here */

	default:
		printf ("%s: Unsupported interface type, %d\n",
				__FUNCTION__, desc->iface);
	}

	return ret_val;
}

int ACEX1K_info( Altera_desc *desc )
{
	return FPGA_SUCCESS;
}


/* ------------------------------------------------------------------------- */
/* ACEX1K Passive Serial Generic Implementation                                  */

static int ACEX1K_ps_load(Altera_desc *desc, const void *buf, size_t bsize)
{
	int ret_val = FPGA_FAIL;	/* assume the worst */
	Altera_ACEX1K_Passive_Serial_fns *fn = desc->iface_fns;
	int i;

	PRINTF ("%s: start with interface functions @ 0x%p\n",
			__FUNCTION__, fn);

	if (fn) {
		size_t bytecount = 0;
		unsigned char *data = (unsigned char *) buf;
		int cookie = desc->cookie;	/* make a local copy */
		unsigned long ts;		/* timestamp */

		PRINTF ("%s: Function Table:\n"
				"ptr:\t0x%p\n"
				"struct: 0x%p\n"
				"config:\t0x%p\n"
				"status:\t0x%p\n"
				"clk:\t0x%p\n"
				"data:\t0x%p\n"
				"done:\t0x%p\n\n",
				__FUNCTION__, &fn, fn, fn->config, fn->status,
				fn->clk, fn->data, fn->done);
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
		(*fn->config) (true, true, cookie);	/* Assert nCONFIG */

		udelay(2);		/* T_cfg > 2us	*/

		/* nSTATUS should be asserted now */
		(*fn->done) (cookie);
		if ( !(*fn->status) (cookie) ) {
			puts ("** nSTATUS is not asserted.\n");
			(*fn->abort) (cookie);
			return FPGA_FAIL;
		}

		(*fn->config) (false, true, cookie);	/* Deassert nCONFIG */
		udelay(2);		/* T_cf2st1 < 4us	*/

		/* Wait for nSTATUS to be released (i.e. deasserted) */
		ts = get_timer (0);		/* get current time */
		do {
			CONFIG_FPGA_DELAY ();
			if (get_timer (ts) > CONFIG_SYS_FPGA_WAIT) {	/* check the time */
				puts ("** Timeout waiting for STATUS to go high.\n");
				(*fn->abort) (cookie);
				return FPGA_FAIL;
			}
			(*fn->done) (cookie);
		} while ((*fn->status) (cookie));

		/* Get ready for the burn */
		CONFIG_FPGA_DELAY ();

		/* Load the data */
		while (bytecount < bsize) {
			unsigned char val=0;
#ifdef CONFIG_SYS_FPGA_CHECK_CTRLC
			if (ctrlc ()) {
				(*fn->abort) (cookie);
				return FPGA_FAIL;
			}
#endif
			/* Altera detects an error if INIT goes low (active)
			   while DONE is low (inactive) */
#if 0 /* not yet implemented */
			if ((*fn->done) (cookie) == 0 && (*fn->init) (cookie)) {
				puts ("** CRC error during FPGA load.\n");
				(*fn->abort) (cookie);
				return (FPGA_FAIL);
			}
#endif
			val = data [bytecount ++ ];
			i = 8;
			do {
				/* Deassert the clock */
				(*fn->clk) (false, true, cookie);
				CONFIG_FPGA_DELAY ();
				/* Write data */
				(*fn->data) ((val & 0x01), true, cookie);
				CONFIG_FPGA_DELAY ();
				/* Assert the clock */
				(*fn->clk) (true, true, cookie);
				CONFIG_FPGA_DELAY ();
				val >>= 1;
				i --;
			} while (i > 0);

#ifdef CONFIG_SYS_FPGA_PROG_FEEDBACK
			if (bytecount % (bsize / 40) == 0)
				putc ('.');		/* let them know we are alive */
#endif
		}

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

	/*
	 * "DCLK must be clocked an additional 10 times fpr ACEX 1K..."
	 */

	for (i = 0; i < 12; i++) {
		CONFIG_FPGA_DELAY ();
		(*fn->clk) (true, true, cookie);	/* Assert the clock pin */
		CONFIG_FPGA_DELAY ();
		(*fn->clk) (false, true, cookie);	/* Deassert the clock pin */
	}

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

static int ACEX1K_ps_dump(Altera_desc *desc, const void *buf, size_t bsize)
{
	/* Readback is only available through the Slave Parallel and         */
	/* boundary-scan interfaces.                                         */
	printf ("%s: Passive Serial Dumping is unavailable\n",
			__FUNCTION__);
	return FPGA_FAIL;
}
