// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2002
 * Rich Ireland, Enterasys Networks, rireland@enterasys.com.
 * Keith Outwater, keith_outwater@mvis.com
 *
 * Copyright (c) 2019 SED Systems, a division of Calian Ltd.
 */

/*
 * Configuration support for Xilinx Virtex2 devices.  Based
 * on spartan2.c (Rich Ireland, rireland@enterasys.com).
 */

#define LOG_CATEGORY UCLASS_FPGA

#include <common.h>
#include <console.h>
#include <log.h>
#include <virtex2.h>
#include <linux/delay.h>

/*
 * If the SelectMap interface can be overrun by the processor, enable
 * CONFIG_SYS_FPGA_CHECK_BUSY and/or define CFG_FPGA_DELAY in the board
 * configuration file and add board-specific support for checking BUSY status.
 * By default, assume that the SelectMap interface cannot be overrun.
 */

#ifndef CFG_FPGA_DELAY
#define CFG_FPGA_DELAY()
#endif

/*
 * Check for errors during configuration by default
 */
#ifndef CFG_SYS_FPGA_CHECK_ERROR
#define CFG_SYS_FPGA_CHECK_ERROR
#endif

/*
 * The default timeout in mS for INIT_B to deassert after PROG_B has
 * been deasserted. Per the latest Virtex II Handbook (page 347), the
 * max time from PORG_B deassertion to INIT_B deassertion is 4uS per
 * data frame for the XC2V8000.  The XC2V8000 has 2860 data frames
 * which yields 11.44 mS.  So let's make it bigger in order to handle
 * an XC2V1000, if anyone can ever get ahold of one.
 */
#ifndef CFG_SYS_FPGA_WAIT_INIT
#define CFG_SYS_FPGA_WAIT_INIT	CONFIG_SYS_HZ / 2	/* 500 ms */
#endif

/*
 * The default timeout for waiting for BUSY to deassert during configuration.
 * This is normally not necessary since for most reasonable configuration
 * clock frequencies (i.e. 66 MHz or less), BUSY monitoring is unnecessary.
 */
#ifndef CFG_SYS_FPGA_WAIT_BUSY
#define CFG_SYS_FPGA_WAIT_BUSY	CONFIG_SYS_HZ / 200	/* 5 ms*/
#endif

/* Default timeout for waiting for FPGA to enter operational mode after
 * configuration data has been written.
 */
#ifndef	CFG_SYS_FPGA_WAIT_CONFIG
#define CFG_SYS_FPGA_WAIT_CONFIG	CONFIG_SYS_HZ / 5	/* 200 ms */
#endif

static int virtex2_ssm_load(xilinx_desc *desc, const void *buf, size_t bsize);
static int virtex2_ssm_dump(xilinx_desc *desc, const void *buf, size_t bsize);

static int virtex2_ss_load(xilinx_desc *desc, const void *buf, size_t bsize);
static int virtex2_ss_dump(xilinx_desc *desc, const void *buf, size_t bsize);

static int virtex2_load(xilinx_desc *desc, const void *buf, size_t bsize,
			bitstream_type bstype, int flags)
{
	int ret_val = FPGA_FAIL;

	switch (desc->iface) {
	case slave_serial:
		log_debug("Launching Slave Serial Load\n");
		ret_val = virtex2_ss_load(desc, buf, bsize);
		break;

	case slave_selectmap:
		log_debug("Launching Slave Parallel Load\n");
		ret_val = virtex2_ssm_load(desc, buf, bsize);
		break;

	default:
		printf("%s: Unsupported interface type, %d\n",
		       __func__, desc->iface);
	}
	return ret_val;
}

static int virtex2_dump(xilinx_desc *desc, const void *buf, size_t bsize)
{
	int ret_val = FPGA_FAIL;

	switch (desc->iface) {
	case slave_serial:
		log_debug("Launching Slave Serial Dump\n");
		ret_val = virtex2_ss_dump(desc, buf, bsize);
		break;

	case slave_parallel:
		log_debug("Launching Slave Parallel Dump\n");
		ret_val = virtex2_ssm_dump(desc, buf, bsize);
		break;

	default:
		printf("%s: Unsupported interface type, %d\n",
		       __func__, desc->iface);
	}
	return ret_val;
}

static int virtex2_info(xilinx_desc *desc)
{
	return FPGA_SUCCESS;
}

/*
 * Virtex-II Slave SelectMap or Serial configuration loader. Configuration
 * is as follows:
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
static int virtex2_slave_pre(xilinx_virtex2_slave_fns *fn, int cookie)
{
	unsigned long ts;

	log_debug("Start with interface functions @ 0x%p\n", fn);

	if (!fn) {
		printf("%s:%d: NULL Interface function table!\n",
		       __func__, __LINE__);
		return FPGA_FAIL;
	}

	/* Gotta split this one up (so the stack won't blow??) */
	log_debug("Function Table:\n"
		  "  base   0x%p\n"
		  "  struct 0x%p\n"
		  "  pre    0x%p\n"
		  "  prog   0x%p\n"
		  "  init   0x%p\n"
		  "  error  0x%p\n",
		  &fn, fn, fn->pre, fn->pgm, fn->init, fn->err);
	log_debug("  clock  0x%p\n"
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
	printf("Initializing FPGA Device %d...\n", cookie);
#endif
	/*
	 * Run the pre configuration function if there is one.
	 */
	if (*fn->pre)
		(*fn->pre)(cookie);

	/*
	 * Assert the program line.  The minimum pulse width for
	 * Virtex II devices is 300 nS (Tprogram parameter in datasheet).
	 * There is no maximum value for the pulse width. Check to make
	 * sure that INIT_B goes low after assertion of PROG_B
	 */
	(*fn->pgm)(true, true, cookie);
	udelay(10);
	ts = get_timer(0);
	do {
		if (get_timer(ts) > CFG_SYS_FPGA_WAIT_INIT) {
			printf("%s:%d: ** Timeout after %d ticks waiting for INIT to assert.\n",
			       __func__, __LINE__, CFG_SYS_FPGA_WAIT_INIT);
			(*fn->abort)(cookie);
			return FPGA_FAIL;
		}
	} while (!(*fn->init)(cookie));

	(*fn->pgm)(false, true, cookie);
	CFG_FPGA_DELAY();
	if (fn->clk)
		(*fn->clk)(true, true, cookie);

	/*
	 * Start a timer and wait for INIT_B to go high
	 */
	ts = get_timer(0);
	do {
		CFG_FPGA_DELAY();
		if (get_timer(ts) > CFG_SYS_FPGA_WAIT_INIT) {
			printf("%s:%d: ** Timeout after %d ticks waiting for INIT to deassert.\n",
			       __func__, __LINE__, CFG_SYS_FPGA_WAIT_INIT);
			(*fn->abort)(cookie);
			return FPGA_FAIL;
		}
	} while ((*fn->init)(cookie) && (*fn->busy)(cookie));

	if (fn->wr)
		(*fn->wr)(true, true, cookie);
	if (fn->cs)
		(*fn->cs)(true, true, cookie);

	mdelay(10);
	return FPGA_SUCCESS;
}

static int virtex2_slave_post(xilinx_virtex2_slave_fns *fn,
			      int cookie)
{
	int ret_val = FPGA_SUCCESS;
	int num_done = 0;
	unsigned long ts;

	/*
	 * Finished writing the data; deassert FPGA CS_B and WRITE_B signals.
	 */
	CFG_FPGA_DELAY();
	if (fn->cs)
		(*fn->cs)(false, true, cookie);
	if (fn->wr)
		(*fn->wr)(false, true, cookie);

#ifdef CONFIG_SYS_FPGA_PROG_FEEDBACK
	putc('\n');
#endif

	/*
	 * Check for successful configuration.  FPGA INIT_B and DONE
	 * should both be high upon successful configuration. Continue pulsing
	 * clock with data set to all ones until DONE is asserted and for 8
	 * clock cycles afterwards.
	 */
	ts = get_timer(0);
	while (true) {
		if ((*fn->done)(cookie) == FPGA_SUCCESS &&
		    !((*fn->init)(cookie))) {
			if (num_done++ >= 8)
				break;
		}

		if (get_timer(ts) > CFG_SYS_FPGA_WAIT_CONFIG) {
			printf("%s:%d: ** Timeout after %d ticks waiting for DONE to assert and INIT to deassert\n",
			       __func__, __LINE__, CFG_SYS_FPGA_WAIT_CONFIG);
			(*fn->abort)(cookie);
			ret_val = FPGA_FAIL;
			break;
		}
		if (fn->wbulkdata) {
			unsigned char dummy = 0xff;
			(*fn->wbulkdata)(&dummy, 1, true, cookie);
		} else {
			(*fn->wdata)(0xff, true, cookie);
			CFG_FPGA_DELAY();
			(*fn->clk)(false, true, cookie);
			CFG_FPGA_DELAY();
			(*fn->clk)(true, true, cookie);
		}
	}

	if (ret_val == FPGA_SUCCESS) {
#ifdef CONFIG_SYS_FPGA_PROG_FEEDBACK
		printf("Initialization of FPGA device %d complete\n", cookie);
#endif
		/*
		 * Run the post configuration function if there is one.
		 */
		if (*fn->post)
			(*fn->post)(cookie);
	} else {
#ifdef CONFIG_SYS_FPGA_PROG_FEEDBACK
		printf("** Initialization of FPGA device %d FAILED\n",
		       cookie);
#endif
	}
	return ret_val;
}

static int virtex2_ssm_load(xilinx_desc *desc, const void *buf, size_t bsize)
{
	int ret_val = FPGA_FAIL;
	xilinx_virtex2_slave_fns *fn = desc->iface_fns;
	size_t bytecount = 0;
	unsigned char *data = (unsigned char *)buf;
	int cookie = desc->cookie;

	ret_val = virtex2_slave_pre(fn, cookie);
	if (ret_val != FPGA_SUCCESS)
		return ret_val;

	/*
	 * Load the data byte by byte
	 */
	while (bytecount < bsize) {
#ifdef CONFIG_SYS_FPGA_CHECK_CTRLC
		if (ctrlc()) {
			(*fn->abort)(cookie);
			return FPGA_FAIL;
		}
#endif

		if ((*fn->done)(cookie) == FPGA_SUCCESS) {
			log_debug("done went active early, bytecount = %zu\n",
				  bytecount);
			break;
		}

#ifdef CFG_SYS_FPGA_CHECK_ERROR
		if ((*fn->init)(cookie)) {
			printf("\n%s:%d:  ** Error: INIT asserted during configuration\n",
			       __func__, __LINE__);
			printf("%zu = buffer offset, %zu = buffer size\n",
			       bytecount, bsize);
			(*fn->abort)(cookie);
			return FPGA_FAIL;
		}
#endif

		(*fn->wdata)(data[bytecount++], true, cookie);
		CFG_FPGA_DELAY();

		/*
		 * Cycle the clock pin
		 */
		(*fn->clk)(false, true, cookie);
		CFG_FPGA_DELAY();
		(*fn->clk)(true, true, cookie);

#ifdef CONFIG_SYS_FPGA_CHECK_BUSY
		ts = get_timer(0);
		while ((*fn->busy)(cookie)) {
			if (get_timer(ts) > CFG_SYS_FPGA_WAIT_BUSY) {
				printf("%s:%d: ** Timeout after %d ticks waiting for BUSY to deassert\n",
				       __func__, __LINE__,
				       CFG_SYS_FPGA_WAIT_BUSY);
				(*fn->abort)(cookie);
				return FPGA_FAIL;
			}
		}
#endif

#ifdef CONFIG_SYS_FPGA_PROG_FEEDBACK
		if (bytecount % (bsize / 40) == 0)
			putc('.');
#endif
	}

	return virtex2_slave_post(fn, cookie);
}

/*
 * Read the FPGA configuration data
 */
static int virtex2_ssm_dump(xilinx_desc *desc, const void *buf, size_t bsize)
{
	int ret_val = FPGA_FAIL;
	xilinx_virtex2_slave_fns *fn = desc->iface_fns;

	if (fn) {
		unsigned char *data = (unsigned char *)buf;
		size_t bytecount = 0;
		int cookie = desc->cookie;

		printf("Starting Dump of FPGA Device %d...\n", cookie);

		(*fn->cs)(true, true, cookie);
		(*fn->clk)(true, true, cookie);

		while (bytecount < bsize) {
#ifdef CONFIG_SYS_FPGA_CHECK_CTRLC
			if (ctrlc()) {
				(*fn->abort)(cookie);
				return FPGA_FAIL;
			}
#endif
			/*
			 * Cycle the clock and read the data
			 */
			(*fn->clk)(false, true, cookie);
			(*fn->clk)(true, true, cookie);
			(*fn->rdata)(&data[bytecount++], cookie);
#ifdef CONFIG_SYS_FPGA_PROG_FEEDBACK
			if (bytecount % (bsize / 40) == 0)
				putc('.');
#endif
		}

		/*
		 * Deassert CS_B and cycle the clock to deselect the device.
		 */
		(*fn->cs)(false, false, cookie);
		(*fn->clk)(false, true, cookie);
		(*fn->clk)(true, true, cookie);

#ifdef CONFIG_SYS_FPGA_PROG_FEEDBACK
		putc('\n');
#endif
		puts("Done.\n");
	} else {
		printf("%s:%d: NULL Interface function table!\n",
		       __func__, __LINE__);
	}
	return ret_val;
}

static int virtex2_ss_load(xilinx_desc *desc, const void *buf, size_t bsize)
{
	int ret_val = FPGA_FAIL;
	xilinx_virtex2_slave_fns *fn = desc->iface_fns;
	unsigned char *data = (unsigned char *)buf;
	int cookie = desc->cookie;

	ret_val = virtex2_slave_pre(fn, cookie);
	if (ret_val != FPGA_SUCCESS)
		return ret_val;

	if (fn->wbulkdata) {
		/* Load the data in a single chunk */
		(*fn->wbulkdata)(data, bsize, true, cookie);
	} else {
		size_t bytecount = 0;

		/*
		 * Load the data bit by bit
		 */
		while (bytecount < bsize) {
			unsigned char curr_data = data[bytecount++];
			int bit;

#ifdef CONFIG_SYS_FPGA_CHECK_CTRLC
			if (ctrlc()) {
				(*fn->abort) (cookie);
				return FPGA_FAIL;
			}
#endif

			if ((*fn->done)(cookie) == FPGA_SUCCESS) {
				log_debug("done went active early, bytecount = %zu\n",
					  bytecount);
				break;
			}

#ifdef CFG_SYS_FPGA_CHECK_ERROR
			if ((*fn->init)(cookie)) {
				printf("\n%s:%d:  ** Error: INIT asserted during configuration\n",
				       __func__, __LINE__);
				printf("%zu = buffer offset, %zu = buffer size\n",
				       bytecount, bsize);
				(*fn->abort)(cookie);
				return FPGA_FAIL;
			}
#endif

			for (bit = 7; bit >= 0; --bit) {
				unsigned char curr_bit = (curr_data >> bit) & 1;
				(*fn->wdata)(curr_bit, true, cookie);
				CFG_FPGA_DELAY();
				(*fn->clk)(false, true, cookie);
				CFG_FPGA_DELAY();
				(*fn->clk)(true, true, cookie);
			}

			/* Slave serial never uses a busy pin */

#ifdef CONFIG_SYS_FPGA_PROG_FEEDBACK
			if (bytecount % (bsize / 40) == 0)
				putc('.');
#endif
		}
	}

	return virtex2_slave_post(fn, cookie);
}

static int virtex2_ss_dump(xilinx_desc *desc, const void *buf, size_t bsize)
{
	printf("%s: Slave Serial Dumping is unsupported\n", __func__);
	return FPGA_FAIL;
}

/* vim: set ts=4 tw=78: */

struct xilinx_fpga_op virtex2_op = {
	.load = virtex2_load,
	.dump = virtex2_dump,
	.info = virtex2_info,
};
