// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2007
 * Eran Liberty, Extricom , eran.liberty@gmail.com
 */

#include <common.h>		/* core U-Boot definitions */
#include <altera.h>
#include <linux/delay.h>

int StratixII_ps_fpp_load (Altera_desc * desc, void *buf, size_t bsize,
			   int isSerial, int isSecure);
int StratixII_ps_fpp_dump (Altera_desc * desc, void *buf, size_t bsize);

/****************************************************************/
/* Stratix II Generic Implementation                            */
int StratixII_load (Altera_desc * desc, void *buf, size_t bsize)
{
	int ret_val = FPGA_FAIL;

	switch (desc->iface) {
	case passive_serial:
		ret_val = StratixII_ps_fpp_load (desc, buf, bsize, 1, 0);
		break;
	case fast_passive_parallel:
		ret_val = StratixII_ps_fpp_load (desc, buf, bsize, 0, 0);
		break;
	case fast_passive_parallel_security:
		ret_val = StratixII_ps_fpp_load (desc, buf, bsize, 0, 1);
		break;

		/* Add new interface types here */
	default:
		printf ("%s: Unsupported interface type, %d\n", __FUNCTION__,
			desc->iface);
	}
	return ret_val;
}

int StratixII_dump (Altera_desc * desc, void *buf, size_t bsize)
{
	int ret_val = FPGA_FAIL;

	switch (desc->iface) {
	case passive_serial:
	case fast_passive_parallel:
	case fast_passive_parallel_security:
		ret_val = StratixII_ps_fpp_dump (desc, buf, bsize);
		break;
		/* Add new interface types here */
	default:
		printf ("%s: Unsupported interface type, %d\n", __FUNCTION__,
			desc->iface);
	}
	return ret_val;
}

int StratixII_info (Altera_desc * desc)
{
	return FPGA_SUCCESS;
}

int StratixII_ps_fpp_dump (Altera_desc * desc, void *buf, size_t bsize)
{
	printf ("Stratix II Fast Passive Parallel dump is not implemented\n");
	return FPGA_FAIL;
}

int StratixII_ps_fpp_load (Altera_desc * desc, void *buf, size_t bsize,
			   int isSerial, int isSecure)
{
	altera_board_specific_func *fns;
	int cookie;
	int ret_val = FPGA_FAIL;
	int bytecount;
	char *buff = buf;
	int i;

	if (!desc) {
		printf ("%s(%d) Altera_desc missing\n", __FUNCTION__, __LINE__);
		return FPGA_FAIL;
	}
	if (!buff) {
		printf ("%s(%d) buffer is missing\n", __FUNCTION__, __LINE__);
		return FPGA_FAIL;
	}
	if (!bsize) {
		printf ("%s(%d) size is zero\n", __FUNCTION__, __LINE__);
		return FPGA_FAIL;
	}
	if (!desc->iface_fns) {
		printf
		    ("%s(%d) Altera_desc function interface table is missing\n",
		     __FUNCTION__, __LINE__);
		return FPGA_FAIL;
	}
	fns = (altera_board_specific_func *) (desc->iface_fns);
	cookie = desc->cookie;

	if (!
	    (fns->config && fns->status && fns->done && fns->data
	     && fns->abort)) {
		printf
		    ("%s(%d) Missing some function in the function interface table\n",
		     __FUNCTION__, __LINE__);
		return FPGA_FAIL;
	}

	/* 1. give board specific a chance to do anything before we start */
	if (fns->pre) {
		if ((ret_val = fns->pre (cookie)) < 0) {
			return ret_val;
		}
	}

	/* from this point on we must fail gracfully by calling lower layer abort */

	/* 2. Strat burn cycle by deasserting config for t_CFG and waiting t_CF2CK after reaserted */
	fns->config (0, 1, cookie);
	udelay(5);		/* nCONFIG low pulse width 2usec */
	fns->config (1, 1, cookie);
	udelay(100);		/* nCONFIG high to first rising edge on DCLK */

	/* 3. Start the Data cycle with clk deasserted */
	bytecount = 0;
	fns->clk (0, 1, cookie);

	printf ("loading to fpga    ");
	while (bytecount < bsize) {
		/* 3.1 check stratix has not signaled us an error */
		if (fns->status (cookie) != 1) {
			printf
			    ("\n%s(%d) Stratix failed (byte transferred till failure 0x%x)\n",
			     __FUNCTION__, __LINE__, bytecount);
			fns->abort (cookie);
			return FPGA_FAIL;
		}
		if (isSerial) {
			int i;
			uint8_t data = buff[bytecount++];
			for (i = 0; i < 8; i++) {
				/* 3.2(ps) put data on the bus */
				fns->data ((data >> i) & 1, 1, cookie);

				/* 3.3(ps) clock once */
				fns->clk (1, 1, cookie);
				fns->clk (0, 1, cookie);
			}
		} else {
			/* 3.2(fpp) put data on the bus */
			fns->data (buff[bytecount++], 1, cookie);

			/* 3.3(fpp) clock once */
			fns->clk (1, 1, cookie);
			fns->clk (0, 1, cookie);

			/* 3.4(fpp) for secure cycle push 3 more  clocks */
			for (i = 0; isSecure && i < 3; i++) {
				fns->clk (1, 1, cookie);
				fns->clk (0, 1, cookie);
			}
		}

		/* 3.5 while clk is deasserted it is safe to print some progress indication */
		if ((bytecount % (bsize / 100)) == 0) {
			printf ("\b\b\b%02d\%", bytecount * 100 / bsize);
		}
	}

	/* 4. Set one last clock and check conf done signal */
	fns->clk (1, 1, cookie);
	udelay(100);
	if (!fns->done (cookie)) {
		printf (" error!.\n");
		fns->abort (cookie);
		return FPGA_FAIL;
	} else {
		printf ("\b\b\b done.\n");
	}

	/* 5. call lower layer post configuration */
	if (fns->post) {
		if ((ret_val = fns->post (cookie)) < 0) {
			fns->abort (cookie);
			return ret_val;
		}
	}

	return FPGA_SUCCESS;
}
