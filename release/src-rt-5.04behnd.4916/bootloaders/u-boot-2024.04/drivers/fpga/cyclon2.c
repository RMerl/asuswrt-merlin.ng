// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2006
 * Heiko Schocher, hs@denx.de
 * Based on ACE1XK.c
 */

#define LOG_CATEGORY UCLASS_FPGA

#include <common.h>		/* core U-Boot definitions */
#include <log.h>
#include <altera.h>
#include <ACEX1K.h>		/* ACEX device family */
#include <linux/delay.h>

/* Note: The assumption is that we cannot possibly run fast enough to
 * overrun the device (the Slave Parallel mode can free run at 50MHz).
 * If there is a need to operate slower, define CFG_FPGA_DELAY in
 * the board config file to slow things down.
 */
#ifndef CFG_FPGA_DELAY
#define CFG_FPGA_DELAY()
#endif

#ifndef CFG_SYS_FPGA_WAIT
#define CFG_SYS_FPGA_WAIT CONFIG_SYS_HZ / 10		/* 100 ms */
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
		log_debug("Launching Passive Serial Loader\n");
		ret_val = CYC2_ps_load(desc, buf, bsize);
		break;

	case fast_passive_parallel:
		/* Fast Passive Parallel (FPP) and PS only differ in what is
		 * done in the write() callback. Use the existing PS load
		 * function for FPP, too.
		 */
		log_debug("Launching Fast Passive Parallel Loader\n");
		ret_val = CYC2_ps_load(desc, buf, bsize);
		break;

		/* Add new interface types here */

	default:
		printf("%s: Unsupported interface type, %d\n",
		       __func__, desc->iface);
	}

	return ret_val;
}

int CYC2_dump(Altera_desc *desc, const void *buf, size_t bsize)
{
	int ret_val = FPGA_FAIL;

	switch (desc->iface) {
	case passive_serial:
		log_debug("Launching Passive Serial Dump\n");
		ret_val = CYC2_ps_dump(desc, buf, bsize);
		break;

		/* Add new interface types here */

	default:
		printf("%s: Unsupported interface type, %d\n",
		       __func__, desc->iface);
	}

	return ret_val;
}

int CYC2_info(Altera_desc *desc)
{
	return FPGA_SUCCESS;
}

/* ------------------------------------------------------------------------- */
/* CYCLON2 Passive Serial Generic Implementation                             */
static int CYC2_ps_load(Altera_desc *desc, const void *buf, size_t bsize)
{
	int ret_val = FPGA_FAIL;	/* assume the worst */
	Altera_CYC2_Passive_Serial_fns *fn = desc->iface_fns;
	int	ret = 0;

	log_debug("start with interface functions @ 0x%p\n", fn);

	if (fn) {
		int cookie = desc->cookie;	/* make a local copy */
		unsigned long ts;		/* timestamp */

		log_debug("Function Table:\n"
			  "ptr:\t0x%p\n"
			  "struct: 0x%p\n"
			  "config:\t0x%p\n"
			  "status:\t0x%p\n"
			  "write:\t0x%p\n"
			  "done:\t0x%p\n\n",
			  &fn, fn, fn->config, fn->status,
			  fn->write, fn->done);
#ifdef CONFIG_SYS_FPGA_PROG_FEEDBACK
		printf("Loading FPGA Device %d...", cookie);
#endif

		/*
		 * Run the pre configuration function if there is one.
		 */
		if (*fn->pre)
			(*fn->pre) (cookie);

		/* Establish the initial state */
		(*fn->config) (false, true, cookie);	/* De-assert nCONFIG */
		udelay(100);
		(*fn->config) (true, true, cookie);	/* Assert nCONFIG */

		udelay(2);		/* T_cfg > 2us	*/

		/* Wait for nSTATUS to be asserted */
		ts = get_timer(0);		/* get current time */
		do {
			CFG_FPGA_DELAY();
			if (get_timer(ts) > CFG_SYS_FPGA_WAIT) {
				/* check the time */
				puts("** Timeout waiting for STATUS to go high.\n");
				(*fn->abort) (cookie);
				return FPGA_FAIL;
			}
		} while (!(*fn->status) (cookie));

		/* Get ready for the burn */
		CFG_FPGA_DELAY();

		ret = (*fn->write) (buf, bsize, true, cookie);
		if (ret) {
			puts("** Write failed.\n");
			(*fn->abort) (cookie);
			return FPGA_FAIL;
		}
#ifdef CONFIG_SYS_FPGA_PROG_FEEDBACK
		puts(" OK? ...");
#endif

		CFG_FPGA_DELAY();

#ifdef CONFIG_SYS_FPGA_PROG_FEEDBACK
		putc(' ');			/* terminate the dotted line */
#endif

		/*
		 * Checking FPGA's CONF_DONE signal - correctly booted ?
		 */

		if (!(*fn->done) (cookie)) {
			puts("** Booting failed! CONF_DONE is still deasserted.\n");
			(*fn->abort) (cookie);
			return FPGA_FAIL;
		}
#ifdef CONFIG_SYS_FPGA_PROG_FEEDBACK
		puts(" OK\n");
#endif

		ret_val = FPGA_SUCCESS;

#ifdef CONFIG_SYS_FPGA_PROG_FEEDBACK
		if (ret_val == FPGA_SUCCESS)
			puts("Done.\n");
		else
			puts("Fail.\n");
#endif

		/*
		 * Run the post configuration function if there is one.
		 */
		if (*fn->post)
			(*fn->post) (cookie);
	} else {
		printf("%s: NULL Interface function table!\n", __func__);
	}

	return ret_val;
}

static int CYC2_ps_dump(Altera_desc *desc, const void *buf, size_t bsize)
{
	/* Readback is only available through the Slave Parallel and         */
	/* boundary-scan interfaces.                                         */
	printf("%s: Passive Serial Dumping is unavailable\n", __func__);
	return FPGA_FAIL;
}
