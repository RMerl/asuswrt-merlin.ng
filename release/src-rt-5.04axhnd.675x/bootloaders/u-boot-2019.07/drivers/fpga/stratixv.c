// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Stefan Roese <sr@denx.de>
 */

#include <common.h>
#include <altera.h>
#include <spi.h>
#include <asm/io.h>
#include <linux/errno.h>

/* Write the RBF data to FPGA via SPI */
static int program_write(int spi_bus, int spi_dev, const void *rbf_data,
			 unsigned long rbf_size)
{
	struct spi_slave *slave;
	int ret;

	debug("%s (%d): data=%p size=%ld\n",
	      __func__, __LINE__, rbf_data, rbf_size);

	/* FIXME: How to get the max. SPI clock and SPI mode? */
	slave = spi_setup_slave(spi_bus, spi_dev, 27777777, SPI_MODE_3);
	if (!slave)
		return -1;

	if (spi_claim_bus(slave))
		return -1;

	ret = spi_xfer(slave, rbf_size * 8, rbf_data, (void *)rbf_data,
		       SPI_XFER_BEGIN | SPI_XFER_END);

	spi_release_bus(slave);

	return ret;
}

/*
 * This is the interface used by FPGA driver.
 * Return 0 for sucess, non-zero for error.
 */
int stratixv_load(Altera_desc *desc, const void *rbf_data, size_t rbf_size)
{
	altera_board_specific_func *pfns = desc->iface_fns;
	int cookie = desc->cookie;
	int spi_bus;
	int spi_dev;
	int ret = 0;

	if ((u32)rbf_data & 0x3) {
		puts("FPGA: Unaligned data, realign to 32bit boundary.\n");
		return -EINVAL;
	}

	/* Run the pre configuration function if there is one */
	if (pfns->pre)
		(pfns->pre)(cookie);

	/* Establish the initial state */
	if (pfns->config) {
		/* De-assert nCONFIG */
		(pfns->config)(false, true, cookie);

		/* nConfig minimum low pulse width is 2us */
		udelay(200);

		/* Assert nCONFIG */
		(pfns->config)(true, true, cookie);

		/* nCONFIG high to first rising clock on DCLK min 1506 us */
		udelay(1600);
	}

	/* Write the RBF data to FPGA */
	if (pfns->write) {
		/*
		 * Use board specific data function to write bitstream
		 * into the FPGA
		 */
		ret = (pfns->write)(rbf_data, rbf_size, true, cookie);
	} else {
		/*
		 * Use common SPI functions to write bitstream into the
		 * FPGA
		 */
		spi_bus = COOKIE2SPI_BUS(cookie);
		spi_dev = COOKIE2SPI_DEV(cookie);
		ret = program_write(spi_bus, spi_dev, rbf_data, rbf_size);
	}
	if (ret)
		return ret;

	/* Check done pin */
	if (pfns->done) {
		ret = (pfns->done)(cookie);

		if (ret)
			printf("Error: DONE not set (ret=%d)!\n", ret);
	}

	return ret;
}
