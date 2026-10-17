// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014 Andreas Bießmann <andreas@biessmann.org>
 */

/*
 * This is a host tool for generating an appropriate string out of board
 * configuration. The string is required for correct generation of PMECC
 * header which in turn is required for NAND flash booting of Atmel AT91 style
 * hardware.
 *
 * See doc/README.atmel_pmecc for more information.
 */

#include <config.h>
#include <stdlib.h>

static int pmecc_get_ecc_bytes(int cap, int sector_size)
{
	int m = 12 + sector_size / 512;
	return (m * cap + 7) / 8;
}

int main(int argc, char *argv[])
{
	unsigned int use_pmecc = 0;
	unsigned int sector_per_page;
	unsigned int sector_size = CONFIG_PMECC_SECTOR_SIZE;
	unsigned int oob_size = CONFIG_SYS_NAND_OOBSIZE;
	unsigned int ecc_bits = CONFIG_PMECC_CAP;
	unsigned int ecc_offset;

#ifdef CONFIG_ATMEL_NAND_HW_PMECC
	use_pmecc = 1;
#endif

	sector_per_page = CONFIG_SYS_NAND_PAGE_SIZE / CONFIG_PMECC_SECTOR_SIZE;
	ecc_offset = oob_size -
		pmecc_get_ecc_bytes(ecc_bits, sector_size) * sector_per_page;

	printf("usePmecc=%d,", use_pmecc);
	printf("sectorPerPage=%d,", sector_per_page);
	printf("sectorSize=%d,", sector_size);
	printf("spareSize=%d,", oob_size);
	printf("eccBits=%d,", ecc_bits);
	printf("eccOffset=%d", ecc_offset);
	printf("\n");

	exit(EXIT_SUCCESS);
}
