/*
 * Command for mmc_spi setup.
 *
 * Copyright (C) 2010 Thomas Chou <thomas@wytron.com.tw>
 * Licensed under the GPL-2 or later.
 */

#include <common.h>
#include <mmc.h>
#include <spi.h>

#ifndef CONFIG_MMC_SPI_BUS
# define CONFIG_MMC_SPI_BUS 0
#endif
#ifndef CONFIG_MMC_SPI_CS
# define CONFIG_MMC_SPI_CS 1
#endif
/* in SPI mode, MMC speed limit is 20MHz, while SD speed limit is 25MHz */
#ifndef CONFIG_MMC_SPI_SPEED
# define CONFIG_MMC_SPI_SPEED 25000000
#endif
/* MMC and SD specs only seem to care that sampling is on the
 * rising edge ... meaning SPI modes 0 or 3.  So either SPI mode
 * should be legit.  We'll use mode 0 since the steady state is 0,
 * which is appropriate for hotplugging, unless the platform data
 * specify mode 3 (if hardware is not compatible to mode 0).
 */
#ifndef CONFIG_MMC_SPI_MODE
# define CONFIG_MMC_SPI_MODE SPI_MODE_0
#endif

static int do_mmc_spi(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	uint bus = CONFIG_MMC_SPI_BUS;
	uint cs = CONFIG_MMC_SPI_CS;
	uint speed = CONFIG_MMC_SPI_SPEED;
	uint mode = CONFIG_MMC_SPI_MODE;
	char *endp;
	struct mmc *mmc;

	if (argc < 2)
		goto usage;

	cs = simple_strtoul(argv[1], &endp, 0);
	if (*argv[1] == 0 || (*endp != 0 && *endp != ':'))
		goto usage;
	if (*endp == ':') {
		if (endp[1] == 0)
			goto usage;
		bus = cs;
		cs = simple_strtoul(endp + 1, &endp, 0);
		if (*endp != 0)
			goto usage;
	}
	if (argc >= 3) {
		speed = simple_strtoul(argv[2], &endp, 0);
		if (*argv[2] == 0 || *endp != 0)
			goto usage;
	}
	if (argc >= 4) {
		mode = simple_strtoul(argv[3], &endp, 16);
		if (*argv[3] == 0 || *endp != 0)
			goto usage;
	}
	if (!spi_cs_is_valid(bus, cs)) {
		printf("Invalid SPI bus %u cs %u\n", bus, cs);
		return 1;
	}

	mmc = mmc_spi_init(bus, cs, speed, mode);
	if (!mmc) {
		printf("Failed to create MMC Device\n");
		return 1;
	}
	printf("%s: %d at %u:%u hz %u mode %u\n", mmc->cfg->name,
	       mmc->block_dev.devnum, bus, cs, speed, mode);
	mmc_init(mmc);
	return 0;

usage:
	return CMD_RET_USAGE;
}

U_BOOT_CMD(
	mmc_spi,	4,	0,	do_mmc_spi,
	"mmc_spi setup",
	"[bus:]cs [hz] [mode]	- setup mmc_spi device"
);
