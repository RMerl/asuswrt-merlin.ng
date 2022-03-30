// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <common.h>
#include <command.h>

static int mmc_nspi (const char *);

int do_dataflash_mmc_mux (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	switch (argc) {
	case 2:			/* on / off	*/
		switch (mmc_nspi (argv[1])) {
		case 0:	AT91F_SelectSPI ();
			break;
		case 1:	AT91F_SelectMMC ();
			break;
		}
	case 1:			/* get status */
		printf ("Mux is configured to be %s\n",
			AT91F_GetMuxStatus () ? "MMC" : "SPI");
		return 0;
	default:
		return CMD_RET_USAGE;
	}
	return 0;
}

static int mmc_nspi (const char *s)
{
	if (strcmp (s, "mmc") == 0) {
		return 1;
	} else if (strcmp (s, "spi") == 0) {
		return 0;
	}
	return -1;
}

U_BOOT_CMD(
	dataflash_mmc_mux, 2, 1, do_dataflash_mmc_mux,
	"enable or disable MMC or SPI\n",
	"[mmc, spi]\n"
	"    - enable or disable MMC or SPI"
);
