// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Command for accessing Arcturus factory environment.
 *
 * Copyright 2013-2019 Arcturus Networks Inc.
 *           https://www.arcturusnetworks.com/products/
 *           by Oleksandr G Zhadan et al.
 *
 */

#include <common.h>
#include <div64.h>
#include <malloc.h>
#include <spi_flash.h>
#include <mmc.h>
#include <version.h>
#include <environment.h>
#include <asm/io.h>

static ulong fwenv_addr[MAX_FWENV_ADDR];
const char mystrerr[] = "ERROR: Failed to save factory info";

static int ishwaddr(char *hwaddr)
{
	if (strlen(hwaddr) == MAX_HWADDR_SIZE)
		if (hwaddr[2] == ':' &&
		    hwaddr[5] == ':' &&
		    hwaddr[8] == ':' &&
		    hwaddr[11] == ':' &&
		    hwaddr[14] == ':')
			return 0;
	return -1;
}

#if (FWENV_TYPE == FWENV_MMC)

static char smac[29][18] __attribute__ ((aligned(0x200)));	/* 1 MMC block is 512 bytes */

int set_mmc_arc_product(int argc, char *const argv[])
{
	struct mmc *mmc;
	u32 blk, cnt, n;
	int i, err = 1;
	void *addr;
	const u8 mmc_dev_num = CONFIG_SYS_MMC_ENV_DEV;

	mmc = find_mmc_device(mmc_dev_num);
	if (!mmc) {
		printf("No SD/MMC/eMMC card found\n");
		return 0;
	}
	if (mmc_init(mmc)) {
		printf("%s(%d) init failed\n", IS_SD(mmc) ? "SD" : "MMC",
		       mmc_dev_num);
		return 0;
	}
	if (mmc_getwp(mmc) == 1) {
		printf("Error: card is write protected!\n");
		return CMD_RET_FAILURE;
	}

	/* Save factory defaults */
	addr = (void *)smac;
	cnt = 1;		/* One 512 bytes block */

	for (i = 0; i < MAX_FWENV_ADDR; i++)
		if (fwenv_addr[i] != -1) {
			blk = fwenv_addr[i] / 512;
			n = blk_dwrite(mmc_get_blk_desc(mmc), blk, cnt, addr);
			if (n != cnt)
				printf("%s: %s [%d]\n", __func__, mystrerr, i);
			else
				err = 0;
		}
	if (err)
		return -2;

	return err;
}

static int read_mmc_arc_info(void)
{
	struct mmc *mmc;
	u32 blk, cnt, n;
	int i;
	void *addr;
	const u8 mmc_dev_num = CONFIG_SYS_MMC_ENV_DEV;

	mmc = find_mmc_device(mmc_dev_num);
	if (!mmc) {
		printf("No SD/MMC/eMMC card found\n");
		return 0;
	}
	if (mmc_init(mmc)) {
		printf("%s(%d) init failed\n", IS_SD(mmc) ? "SD" : "MMC",
		       mmc_dev_num);
		return 0;
	}

	addr = (void *)smac;
	cnt = 1;		/* One 512 bytes block */

	for (i = 0; i < MAX_FWENV_ADDR; i++)
		if (fwenv_addr[i] != -1) {
			blk = fwenv_addr[i] / 512;
			n = blk_dread(mmc_get_blk_desc(mmc), blk, cnt, addr);
			flush_cache((ulong) addr, 512);
			if (n == cnt)
				return (i + 1);
		}
	return 0;
}
#endif

#if (FWENV_TYPE == FWENV_SPI_FLASH)

static struct spi_flash *flash;
static char smac[4][18];

int set_spi_arc_product(int argc, char *const argv[])
{
	int i, err = 1;

	flash = spi_flash_probe(CONFIG_ENV_SPI_BUS, CONFIG_ENV_SPI_CS,
				CONFIG_ENV_SPI_MAX_HZ, CONFIG_ENV_SPI_MODE);
	if (!flash) {
		printf("Failed to initialize SPI flash at %u:%u\n",
		       CONFIG_ENV_SPI_BUS, CONFIG_ENV_SPI_CS);
		return -1;
	}

	/* Save factory defaults */
	for (i = 0; i < MAX_FWENV_ADDR; i++)
		if (fwenv_addr[i] != -1)
			if (spi_flash_write
			    (flash, fwenv_addr[i], sizeof(smac), smac))
				printf("%s: %s [%d]\n", __func__, mystrerr, i);
			else
				err = 0;
	if (err)
		return -2;

	return err;
}

static int read_spi_arc_info(void)
{
	int i;

	flash = spi_flash_probe(CONFIG_ENV_SPI_BUS, CONFIG_ENV_SPI_CS,
				CONFIG_ENV_SPI_MAX_HZ, CONFIG_ENV_SPI_MODE);
	if (!flash) {
		printf("Failed to initialize SPI flash at %u:%u\n",
		       CONFIG_ENV_SPI_BUS, CONFIG_ENV_SPI_CS);
		return 0;
	}
	for (i = 0; i < MAX_FWENV_ADDR; i++)
		if (fwenv_addr[i] != -1)
			if (!spi_flash_read
			    (flash, fwenv_addr[i], sizeof(smac), smac))
				return (i + 1);
	return 0;
}
#endif

#if (FWENV_TYPE == FWENV_NOR_FLASH)

static char smac[4][18];

int set_nor_arc_product(int argc, char *const argv[])
{
	int i, err = 1;

	/* Save factory defaults */
	for (i = 0; i < MAX_FWENV_ADDR; i++)
		if (fwenv_addr[i] != -1) {
			ulong fwenv_end = fwenv_addr[i] + 4;

			flash_sect_roundb(&fwenv_end);
			flash_sect_protect(0, fwenv_addr[i], fwenv_end);
			if (flash_write
			    ((char *)smac, fwenv_addr[i], sizeof(smac)))
				printf("%s: %s [%d]\n", __func__, mystrerr, i);
			else
				err = 0;
			flash_sect_protect(1, fwenv_addr[i], fwenv_end);
		}
	if (err)
		return -2;

	return err;
}

static int read_nor_arc_info(void)
{
	int i;

	for (i = 0; i < MAX_FWENV_ADDR; i++)
		if (fwenv_addr[i] != -1) {
			memcpy(smac, (void *)fwenv_addr[i], sizeof(smac));
			return (i + 1);
		}

	return 0;
}
#endif

int set_arc_product(int argc, char *const argv[])
{
	if (argc != 5)
		return -1;

	/* Check serial number */
	if (strlen(argv[1]) != MAX_SERIAL_SIZE)
		return -1;

	/* Check HWaddrs */
	if (ishwaddr(argv[2]) || ishwaddr(argv[3]) || ishwaddr(argv[4]))
		return -1;

	strcpy(smac[0], argv[1]);
	strcpy(smac[1], argv[2]);
	strcpy(smac[2], argv[3]);
	strcpy(smac[3], argv[4]);

#if (FWENV_TYPE == FWENV_NOR_FLASH)
	return set_nor_arc_product(argc, argv);
#endif
#if (FWENV_TYPE == FWENV_SPI_FLASH)
	return set_spi_arc_product(argc, argv);
#endif
#if (FWENV_TYPE == FWENV_MMC)
	return set_mmc_arc_product(argc, argv);
#endif
	return -2;
}

static int read_arc_info(void)
{
#if (FWENV_TYPE == FWENV_NOR_FLASH)
	return read_nor_arc_info();
#endif
#if (FWENV_TYPE == FWENV_SPI_FLASH)
	return read_spi_arc_info();
#endif
#if (FWENV_TYPE == FWENV_MMC)
	return read_mmc_arc_info();
#endif
	return 0;
}

static int do_get_arc_info(void)
{
	int l = read_arc_info();
	char *oldserial = env_get("SERIAL");
	char *oldversion = env_get("VERSION");

	if (oldversion != NULL)
		if (strcmp(oldversion, U_BOOT_VERSION) != 0)
			oldversion = NULL;

	if (l == 0) {
		printf("%s: failed to read factory info\n", __func__);
		return -2;
	}

	printf("\rSERIAL:  ");
	if (smac[0][0] == EMPY_CHAR) {
		printf("<not found>\n");
	} else {
		printf("%s\n", smac[0]);
		env_set("SERIAL", smac[0]);
	}

	if (strcmp(smac[1], "00:00:00:00:00:00") == 0) {
		env_set("ethaddr", NULL);
		env_set("eth1addr", NULL);
		env_set("eth2addr", NULL);
		goto done;
	}

	printf("HWADDR0: ");
	if (smac[1][0] == EMPY_CHAR) {
		printf("<not found>\n");
	} else {
		char *ret = env_get("ethaddr");

		if (ret == NULL) {
			env_set("ethaddr", smac[1]);
			printf("%s\n", smac[1]);
		} else if (strcmp(ret, __stringify(CONFIG_ETHADDR)) == 0) {
			env_set("ethaddr", smac[1]);
			printf("%s (factory)\n", smac[1]);
		} else {
			printf("%s\n", ret);
		}
	}

	if (strcmp(smac[2], "00:00:00:00:00:00") == 0) {
		env_set("eth1addr", NULL);
		env_set("eth2addr", NULL);
		goto done;
	}

	printf("HWADDR1: ");
	if (smac[2][0] == EMPY_CHAR) {
		printf("<not found>\n");
	} else {
		char *ret = env_get("eth1addr");

		if (ret == NULL) {
			env_set("ethaddr", smac[2]);
			printf("%s\n", smac[2]);
		} else if (strcmp(ret, __stringify(CONFIG_ETH1ADDR)) == 0) {
			env_set("eth1addr", smac[2]);
			printf("%s (factory)\n", smac[2]);
		} else {
			printf("%s\n", ret);
		}
	}

	if (strcmp(smac[3], "00:00:00:00:00:00") == 0) {
		env_set("eth2addr", NULL);
		goto done;
	}

	printf("HWADDR2: ");
	if (smac[3][0] == EMPY_CHAR) {
		printf("<not found>\n");
	} else {
		char *ret = env_get("eth2addr");

		if (ret == NULL) {
			env_set("ethaddr", smac[3]);
			printf("%s\n", smac[3]);
		} else if (strcmp(ret, __stringify(CONFIG_ETH2ADDR)) == 0) {
			env_set("eth2addr", smac[3]);
			printf("%s (factory)\n", smac[3]);
		} else {
			printf("%s\n", ret);
		}
	}
done:
	if (oldserial == NULL || oldversion == NULL) {
		if (oldversion == NULL)
			env_set("VERSION", U_BOOT_VERSION);
		env_save();
	}

	return 0;
}

static int init_fwenv(void)
{
	int i, ret = -1;

	fwenv_addr[0] = FWENV_ADDR1;
	fwenv_addr[1] = FWENV_ADDR2;
	fwenv_addr[2] = FWENV_ADDR3;
	fwenv_addr[3] = FWENV_ADDR4;

	for (i = 0; i < MAX_FWENV_ADDR; i++)
		if (fwenv_addr[i] != -1)
			ret = 0;
	if (ret)
		printf("%s: No firmfare info storage address is defined\n",
		       __func__);
	return ret;
}

void get_arc_info(void)
{
	if (!init_fwenv())
		do_get_arc_info();
}

static int do_arc_cmd(cmd_tbl_t * cmdtp, int flag, int argc, char *const argv[])
{
	const char *cmd;
	int ret = -1;

	cmd = argv[1];
	--argc;
	++argv;

	if (init_fwenv())
		return ret;

	if (strcmp(cmd, "product") == 0)
		ret = set_arc_product(argc, argv);
	else if (strcmp(cmd, "info") == 0)
		ret = do_get_arc_info();

	if (ret == -1)
		return CMD_RET_USAGE;

	return ret;
}

U_BOOT_CMD(arc, 6, 1, do_arc_cmd,
	   "Arcturus product command sub-system",
	   "product serial hwaddr0 hwaddr1 hwaddr2    - save Arcturus factory env\n"
	   "info                                      - show Arcturus factory env\n\n");
