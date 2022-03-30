// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2002
 * Rich Ireland, Enterasys Networks, rireland@enterasys.com.
 * Keith Outwater, keith_outwater@mvis.com
 */

/*
 * Configuration support for Xilinx Virtex2 devices.  Based
 * on spartan2.c (Rich Ireland, rireland@enterasys.com).
 */

#include <common.h>
#include <console.h>
#include <virtex2.h>

#if 0
#define FPGA_DEBUG
#endif

#ifdef	FPGA_DEBUG
#define	PRINTF(fmt,args...)	printf (fmt ,##args)
#else
#define PRINTF(fmt,args...)
#endif

/*
 * If the SelectMap interface can be overrun by the processor, define
 * CONFIG_SYS_FPGA_CHECK_BUSY and/or CONFIG_FPGA_DELAY in the board configuration
 * file and add board-specific support for checking BUSY status. By default,
 * assume that the SelectMap interface cannot be overrun.
 */
#ifndef CONFIG_SYS_FPGA_CHECK_BUSY
#undef CONFIG_SYS_FPGA_CHECK_BUSY
#endif

#ifndef CONFIG_FPGA_DELAY
#define CONFIG_FPGA_DELAY()
#endif

#ifndef CONFIG_SYS_FPGA_PROG_FEEDBACK
#define CONFIG_SYS_FPGA_PROG_FEEDBACK
#endif

/*
 * Don't allow config cycle to be interrupted
 */
#ifndef CONFIG_SYS_FPGA_CHECK_CTRLC
#undef CONFIG_SYS_FPGA_CHECK_CTRLC
#endif

/*
 * Check for errors during configuration by default
 */
#ifndef CONFIG_SYS_FPGA_CHECK_ERROR
#define CONFIG_SYS_FPGA_CHECK_ERROR
#endif

/*
 * The default timeout in mS for INIT_B to deassert after PROG_B has
 * been deasserted. Per the latest Virtex II Handbook (page 347), the
 * max time from PORG_B deassertion to INIT_B deassertion is 4uS per
 * data frame for the XC2V8000.  The XC2V8000 has 2860 data frames
 * which yields 11.44 mS.  So let's make it bigger in order to handle
 * an XC2V1000, if anyone can ever get ahold of one.
 */
#ifndef CONFIG_SYS_FPGA_WAIT_INIT
#define CONFIG_SYS_FPGA_WAIT_INIT	CONFIG_SYS_HZ/2	/* 500 ms */
#endif

/*
 * The default timeout for waiting for BUSY to deassert during configuration.
 * This is normally not necessary since for most reasonable configuration
 * clock frequencies (i.e. 66 MHz or less), BUSY monitoring is unnecessary.
 */
#ifndef CONFIG_SYS_FPGA_WAIT_BUSY
#define CONFIG_SYS_FPGA_WAIT_BUSY	CONFIG_SYS_HZ/200	/* 5 ms*/
#endif

/* Default timeout for waiting for FPGA to enter operational mode after
 * configuration data has been written.
 */
#ifndef	CONFIG_SYS_FPGA_WAIT_CONFIG
#define CONFIG_SYS_FPGA_WAIT_CONFIG	CONFIG_SYS_HZ/5	/* 200 ms */
#endif

static int virtex2_ssm_load(xilinx_desc *desc, const void *buf, size_t bsize);
static int virtex2_ssm_dump(xilinx_desc *desc, const void *buf, size_t bsize);

static int virtex2_ss_load(xilinx_desc *desc, const void *buf, size_t bsize);
static int virtex2_ss_dump(xilinx_desc *desc, const void *buf, size_t bsize);

static int virtex2_load(xilinx_desc *desc, const void *buf, size_t bsize,
			bitstream_type bstype)
{
	int ret_val = FPGA_FAIL;

	switch (desc->iface) {
	case slave_serial:
		PRINTF ("%s: Launching Slave Serial Load\n", __FUNCTION__);
		ret_val = virtex2_ss_load(desc, buf, bsize);
		break;

	case slave_selectmap:
		PRINTF ("%s: Launching Slave Parallel Load\n", __FUNCTION__);
		ret_val = virtex2_ssm_load(desc, buf, bsize);
		break;

	default:
		printf ("%s: Unsupported interface type, %d\n",
				__FUNCTION__, desc->iface);
	}
	return ret_val;
}

static int virtex2_dump(xilinx_desc *desc, const void *buf, size_t bsize)
{
	int ret_val = FPGA_FAIL;

	switch (desc->iface) {
	case slave_serial:
		PRINTF ("%s: Launching Slave Serial Dump\n", __FUNCTION__);
		ret_val = virtex2_ss_dump(desc, buf, bsize);
		break;

	case slave_parallel:
		PRINTF ("%s: Launching Slave Parallel Dump\n", __FUNCTION__);
		ret_val = virtex2_ssm_dump(desc, buf, bsize);
		break;

	default:
		printf ("%s: Unsupported interface type, %d\n",
				__FUNCTION__, desc->iface);
	}
	return ret_val;
}

static int virtex2_info(xilinx_desc *desc)
{
	return FPGA_SUCCESS;
}

/*
 * Virtex-II Slave SelectMap configuration loader. Configuration via
 * SelectMap is as follows:
 * 1. Set the FPGA's PROG_B line low.
 * 2. Set the FPGA's PROG_B line high.  Wait for INIT_B to go high.
 * 3. Write data to the SelectMap port.  If INIT_B goes low at any time
 *    this process, a configuration error (most likely CRC failure) has
 *    ocurred.  At this point a status word may be read from the
 *    SelectMap interface to determine the source of the problem (You
 *    could, for instance, put this in your 'abort' function handler).
 * 4. After all data has been written, test the state of the FPGA
 *    INIT_B and DONE lines.  If both are high, configuration has
 *    succeeded. Congratulations!
 */
static int virtex2_ssm_load(xilinx_desc *desc, const void *buf, size_t bsize)
{
	int ret_val = FPGA_FAIL;
	xilinx_virtex2_slave_selectmap_fns *fn = desc->iface_fns;

	PRINTF ("%s:%d: Start with interface functions @ 0x%p\n",
			__FUNCTION__, __LINE__, fn);

	if (fn) {
		size_t bytecount = 0;
		unsigned char *data = (unsigned char *) buf;
		int cookie = desc->cookie;
		unsigned long ts;

		/* Gotta split this one up (so the stack won't blow??) */
		PRINTF ("%s:%d: Function Table:\n"
				"  base   0x%p\n"
				"  struct 0x%p\n"
				"  pre    0x%p\n"
				"  prog   0x%p\n"
				"  init   0x%p\n"
				"  error  0x%p\n",
				__FUNCTION__, __LINE__,
				&fn, fn, fn->pre, fn->pgm, fn->init, fn->err);
		PRINTF ("  clock  0x%p\n"
				"  cs     0x%p\n"
				"  write  0x%p\n"
				"  rdata  0x%p\n"
				"  wdata  0x%p\n"
				"  busy   0x%p\n"
				"  abort  0x%p\n"
				"  post   0x%p\n\n",
				fn->clk, fn->cs, fn->wr, fn->rdata, fn->wdata,
				fn->busy, fn->abort, fn->post);

#ifdef CONFIG_SYS_FPGA_PROG_FEEDBACK
		printf ("Initializing FPGA Device %d...\n", cookie);
#endif
		/*
		 * Run the pre configuration function if there is one.
		 */
		if (*fn->pre) {
			(*fn->pre) (cookie);
		}

		/*
		 * Assert the program line.  The minimum pulse width for
		 * Virtex II devices is 300 nS (Tprogram parameter in datasheet).
		 * There is no maximum value for the pulse width.  Check to make
		 * sure that INIT_B goes low after assertion of PROG_B
		 */
		(*fn->pgm) (true, true, cookie);
		udelay (10);
		ts = get_timer (0);
		do {
			if (get_timer (ts) > CONFIG_SYS_FPGA_WAIT_INIT) {
				printf ("%s:%d: ** Timeout after %d ticks waiting for INIT"
						" to assert.\n", __FUNCTION__, __LINE__,
						CONFIG_SYS_FPGA_WAIT_INIT);
				(*fn->abort) (cookie);
				return FPGA_FAIL;
			}
		} while (!(*fn->init) (cookie));

		(*fn->pgm) (false, true, cookie);
		CONFIG_FPGA_DELAY ();
		(*fn->clk) (true, true, cookie);

		/*
		 * Start a timer and wait for INIT_B to go high
		 */
		ts = get_timer (0);
		do {
			CONFIG_FPGA_DELAY ();
			if (get_timer (ts) > CONFIG_SYS_FPGA_WAIT_INIT) {
				printf ("%s:%d: ** Timeout after %d ticks waiting for INIT"
						" to deassert.\n", __FUNCTION__, __LINE__,
						CONFIG_SYS_FPGA_WAIT_INIT);
				(*fn->abort) (cookie);
				return FPGA_FAIL;
			}
		} while ((*fn->init) (cookie) && (*fn->busy) (cookie));

		(*fn->wr) (true, true, cookie);
		(*fn->cs) (true, true, cookie);

		udelay (10000);

		/*
		 * Load the data byte by byte
		 */
		while (bytecount < bsize) {
#ifdef CONFIG_SYS_FPGA_CHECK_CTRLC
			if (ctrlc ()) {
				(*fn->abort) (cookie);
				return FPGA_FAIL;
			}
#endif

			if ((*fn->done) (cookie) == FPGA_SUCCESS) {
			    PRINTF ("%s:%d:done went active early, bytecount = %d\n",
				    __FUNCTION__, __LINE__, bytecount);
			    break;
			}

#ifdef CONFIG_SYS_FPGA_CHECK_ERROR
			if ((*fn->init) (cookie)) {
				printf ("\n%s:%d:  ** Error: INIT asserted during"
						" configuration\n", __FUNCTION__, __LINE__);
				printf ("%d = buffer offset, %d = buffer size\n",
					bytecount, bsize);
				(*fn->abort) (cookie);
				return FPGA_FAIL;
			}
#endif

			(*fn->wdata) (data[bytecount++], true, cookie);
			CONFIG_FPGA_DELAY ();

			/*
			 * Cycle the clock pin
			 */
			(*fn->clk) (false, true, cookie);
			CONFIG_FPGA_DELAY ();
			(*fn->clk) (true, true, cookie);

#ifdef CONFIG_SYS_FPGA_CHECK_BUSY
			ts = get_timer (0);
			while ((*fn->busy) (cookie)) {
				if (get_timer (ts) > CONFIG_SYS_FPGA_WAIT_BUSY) {
					printf ("%s:%d: ** Timeout after %d ticks waiting for"
							" BUSY to deassert\n",
							__FUNCTION__, __LINE__, CONFIG_SYS_FPGA_WAIT_BUSY);
					(*fn->abort) (cookie);
					return FPGA_FAIL;
				}
			}
#endif

#ifdef CONFIG_SYS_FPGA_PROG_FEEDBACK
			if (bytecount % (bsize / 40) == 0)
				putc ('.');
#endif
		}

		/*
		 * Finished writing the data; deassert FPGA CS_B and WRITE_B signals.
		 */
		CONFIG_FPGA_DELAY ();
		(*fn->cs) (false, true, cookie);
		(*fn->wr) (false, true, cookie);

#ifdef CONFIG_SYS_FPGA_PROG_FEEDBACK
		putc ('\n');
#endif

		/*
		 * Check for successful configuration.  FPGA INIT_B and DONE should
		 * both be high upon successful configuration.
		 */
		ts = get_timer (0);
		ret_val = FPGA_SUCCESS;
		while (((*fn->done) (cookie) == FPGA_FAIL) || (*fn->init) (cookie)) {
			if (get_timer (ts) > CONFIG_SYS_FPGA_WAIT_CONFIG) {
				printf ("%s:%d: ** Timeout after %d ticks waiting for DONE to"
						"assert and INIT to deassert\n",
						__FUNCTION__, __LINE__, CONFIG_SYS_FPGA_WAIT_CONFIG);
				(*fn->abort) (cookie);
				ret_val = FPGA_FAIL;
				break;
			}
		}

		if (ret_val == FPGA_SUCCESS) {
#ifdef CONFIG_SYS_FPGA_PROG_FEEDBACK
			printf ("Initialization of FPGA device %d complete\n", cookie);
#endif
			/*
			 * Run the post configuration function if there is one.
			 */
			if (*fn->post) {
				(*fn->post) (cookie);
			}
		} else {
#ifdef CONFIG_SYS_FPGA_PROG_FEEDBACK
			printf ("** Initialization of FPGA device %d FAILED\n",
					cookie);
#endif
		}
	} else {
		printf ("%s:%d: NULL Interface function table!\n",
				__FUNCTION__, __LINE__);
	}
	return ret_val;
}

/*
 * Read the FPGA configuration data
 */
static int virtex2_ssm_dump(xilinx_desc *desc, const void *buf, size_t bsize)
{
	int ret_val = FPGA_FAIL;
	xilinx_virtex2_slave_selectmap_fns *fn = desc->iface_fns;

	if (fn) {
		unsigned char *data = (unsigned char *) buf;
		size_t bytecount = 0;
		int cookie = desc->cookie;

		printf ("Starting Dump of FPGA Device %d...\n", cookie);

		(*fn->cs) (true, true, cookie);
		(*fn->clk) (true, true, cookie);

		while (bytecount < bsize) {
#ifdef CONFIG_SYS_FPGA_CHECK_CTRLC
			if (ctrlc ()) {
				(*fn->abort) (cookie);
				return FPGA_FAIL;
			}
#endif
			/*
			 * Cycle the clock and read the data
			 */
			(*fn->clk) (false, true, cookie);
			(*fn->clk) (true, true, cookie);
			(*fn->rdata) (&(data[bytecount++]), cookie);
#ifdef CONFIG_SYS_FPGA_PROG_FEEDBACK
			if (bytecount % (bsize / 40) == 0)
				putc ('.');
#endif
		}

		/*
		 * Deassert CS_B and cycle the clock to deselect the device.
		 */
		(*fn->cs) (false, false, cookie);
		(*fn->clk) (false, true, cookie);
		(*fn->clk) (true, true, cookie);

#ifdef CONFIG_SYS_FPGA_PROG_FEEDBACK
		putc ('\n');
#endif
		puts ("Done.\n");
	} else {
		printf ("%s:%d: NULL Interface function table!\n",
				__FUNCTION__, __LINE__);
	}
	return ret_val;
}

static int virtex2_ss_load(xilinx_desc *desc, const void *buf, size_t bsize)
{
	printf ("%s: Slave Serial Loading is unsupported\n", __FUNCTION__);
	return FPGA_FAIL;
}

static int virtex2_ss_dump(xilinx_desc *desc, const void *buf, size_t bsize)
{
	printf ("%s: Slave Serial Dumping is unsupported\n", __FUNCTION__);
	return FPGA_FAIL;
}

/* vim: set ts=4 tw=78: */

struct xilinx_fpga_op virtex2_op = {
	.load = virtex2_load,
	.dump = virtex2_dump,
	.info = virtex2_info,
};
