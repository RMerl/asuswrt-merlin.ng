// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Marek Behun <marek.behun@nic.cz>
 * Copyright (C) 2016 Tomas Hlavacek <tomas.hlavacek@nic.cz>
 *
 * Derived from the code for
 *   Marvell/db-88f6820-gp by Stefan Roese <sr@denx.de>
 */

#include <common.h>
#include <environment.h>
#include <i2c.h>
#include <miiphy.h>
#include <netdev.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>
#include <dm/uclass.h>
#include <fdt_support.h>
#include <time.h>
# include <atsha204a-i2c.h>

#include "../drivers/ddr/marvell/a38x/ddr3_init.h"
#include <../serdes/a38x/high_speed_env_spec.h>

DECLARE_GLOBAL_DATA_PTR;

#define OMNIA_I2C_BUS_NAME		"i2c@11000->i2cmux@70->i2c@0"

#define OMNIA_I2C_MCU_CHIP_ADDR		0x2a
#define OMNIA_I2C_MCU_CHIP_LEN		1

#define OMNIA_I2C_EEPROM_CHIP_ADDR	0x54
#define OMNIA_I2C_EEPROM_CHIP_LEN	2
#define OMNIA_I2C_EEPROM_MAGIC		0x0341a034

enum mcu_commands {
	CMD_GET_STATUS_WORD	= 0x01,
	CMD_GET_RESET		= 0x09,
	CMD_WATCHDOG_STATE	= 0x0b,
};

enum status_word_bits {
	CARD_DET_STSBIT		= 0x0010,
	MSATA_IND_STSBIT	= 0x0020,
};

#define OMNIA_ATSHA204_OTP_VERSION	0
#define OMNIA_ATSHA204_OTP_SERIAL	1
#define OMNIA_ATSHA204_OTP_MAC0		3
#define OMNIA_ATSHA204_OTP_MAC1		4

/*
 * Those values and defines are taken from the Marvell U-Boot version
 * "u-boot-2013.01-2014_T3.0"
 */
#define OMNIA_GPP_OUT_ENA_LOW					\
	(~(BIT(1)  | BIT(4)  | BIT(6)  | BIT(7)  | BIT(8)  | BIT(9)  |	\
	   BIT(10) | BIT(11) | BIT(19) | BIT(22) | BIT(23) | BIT(25) |	\
	   BIT(26) | BIT(27) | BIT(29) | BIT(30) | BIT(31)))
#define OMNIA_GPP_OUT_ENA_MID					\
	(~(BIT(0) | BIT(1) | BIT(2) | BIT(3) | BIT(4) | BIT(15) |	\
	   BIT(16) | BIT(17) | BIT(18)))

#define OMNIA_GPP_OUT_VAL_LOW	0x0
#define OMNIA_GPP_OUT_VAL_MID	0x0
#define OMNIA_GPP_POL_LOW	0x0
#define OMNIA_GPP_POL_MID	0x0

static struct serdes_map board_serdes_map_pex[] = {
	{PEX0, SERDES_SPEED_5_GBPS, PEX_ROOT_COMPLEX_X1, 0, 0},
	{USB3_HOST0, SERDES_SPEED_5_GBPS, SERDES_DEFAULT_MODE, 0, 0},
	{PEX1, SERDES_SPEED_5_GBPS, PEX_ROOT_COMPLEX_X1, 0, 0},
	{USB3_HOST1, SERDES_SPEED_5_GBPS, SERDES_DEFAULT_MODE, 0, 0},
	{PEX2, SERDES_SPEED_5_GBPS, PEX_ROOT_COMPLEX_X1, 0, 0},
	{SGMII2, SERDES_SPEED_1_25_GBPS, SERDES_DEFAULT_MODE, 0, 0}
};

static struct serdes_map board_serdes_map_sata[] = {
	{SATA0, SERDES_SPEED_6_GBPS, SERDES_DEFAULT_MODE, 0, 0},
	{USB3_HOST0, SERDES_SPEED_5_GBPS, SERDES_DEFAULT_MODE, 0, 0},
	{PEX1, SERDES_SPEED_5_GBPS, PEX_ROOT_COMPLEX_X1, 0, 0},
	{USB3_HOST1, SERDES_SPEED_5_GBPS, SERDES_DEFAULT_MODE, 0, 0},
	{PEX2, SERDES_SPEED_5_GBPS, PEX_ROOT_COMPLEX_X1, 0, 0},
	{SGMII2, SERDES_SPEED_1_25_GBPS, SERDES_DEFAULT_MODE, 0, 0}
};

static struct udevice *omnia_get_i2c_chip(const char *name, uint addr,
					  uint offset_len)
{
	struct udevice *bus, *dev;
	int ret;

	ret = uclass_get_device_by_name(UCLASS_I2C, OMNIA_I2C_BUS_NAME, &bus);
	if (ret) {
		printf("Cannot get I2C bus %s: uclass_get_device_by_name failed: %i\n",
		       OMNIA_I2C_BUS_NAME, ret);
		return NULL;
	}

	ret = i2c_get_chip(bus, addr, offset_len, &dev);
	if (ret) {
		printf("Cannot get %s I2C chip: i2c_get_chip failed: %i\n",
		       name, ret);
		return NULL;
	}

	return dev;
}

static int omnia_mcu_read(u8 cmd, void *buf, int len)
{
	struct udevice *chip;

	chip = omnia_get_i2c_chip("MCU", OMNIA_I2C_MCU_CHIP_ADDR,
				  OMNIA_I2C_MCU_CHIP_LEN);
	if (!chip)
		return -ENODEV;

	return dm_i2c_read(chip, cmd, buf, len);
}

#ifndef CONFIG_SPL_BUILD
static int omnia_mcu_write(u8 cmd, const void *buf, int len)
{
	struct udevice *chip;

	chip = omnia_get_i2c_chip("MCU", OMNIA_I2C_MCU_CHIP_ADDR,
				  OMNIA_I2C_MCU_CHIP_LEN);
	if (!chip)
		return -ENODEV;

	return dm_i2c_write(chip, cmd, buf, len);
}

static bool disable_mcu_watchdog(void)
{
	int ret;

	puts("Disabling MCU watchdog... ");

	ret = omnia_mcu_write(CMD_WATCHDOG_STATE, "\x00", 1);
	if (ret) {
		printf("omnia_mcu_write failed: %i\n", ret);
		return false;
	}

	puts("disabled\n");

	return true;
}
#endif

static bool omnia_detect_sata(void)
{
	int ret;
	u16 stsword;

	puts("MiniPCIe/mSATA card detection... ");

	ret = omnia_mcu_read(CMD_GET_STATUS_WORD, &stsword, sizeof(stsword));
	if (ret) {
		printf("omnia_mcu_read failed: %i, defaulting to MiniPCIe card\n",
		       ret);
		return false;
	}

	if (!(stsword & CARD_DET_STSBIT)) {
		puts("none\n");
		return false;
	}

	if (stsword & MSATA_IND_STSBIT)
		puts("mSATA\n");
	else
		puts("MiniPCIe\n");

	return stsword & MSATA_IND_STSBIT ? true : false;
}

int hws_board_topology_load(struct serdes_map **serdes_map_array, u8 *count)
{
	if (omnia_detect_sata()) {
		*serdes_map_array = board_serdes_map_sata;
		*count = ARRAY_SIZE(board_serdes_map_sata);
	} else {
		*serdes_map_array = board_serdes_map_pex;
		*count = ARRAY_SIZE(board_serdes_map_pex);
	}

	return 0;
}

struct omnia_eeprom {
	u32 magic;
	u32 ramsize;
	char region[4];
	u32 crc;
};

static bool omnia_read_eeprom(struct omnia_eeprom *oep)
{
	struct udevice *chip;
	u32 crc;
	int ret;

	chip = omnia_get_i2c_chip("EEPROM", OMNIA_I2C_EEPROM_CHIP_ADDR,
				  OMNIA_I2C_EEPROM_CHIP_LEN);

	if (!chip)
		return false;

	ret = dm_i2c_read(chip, 0, (void *)oep, sizeof(*oep));
	if (ret) {
		printf("dm_i2c_read failed: %i, cannot read EEPROM\n", ret);
		return false;
	}

	if (oep->magic != OMNIA_I2C_EEPROM_MAGIC) {
		printf("bad EEPROM magic number (%08x, should be %08x)\n",
		       oep->magic, OMNIA_I2C_EEPROM_MAGIC);
		return false;
	}

	crc = crc32(0, (void *)oep, sizeof(*oep) - 4);
	if (crc != oep->crc) {
		printf("bad EEPROM CRC (stored %08x, computed %08x)\n",
		       oep->crc, crc);
		return false;
	}

	return true;
}

static int omnia_get_ram_size_gb(void)
{
	static int ram_size;
	struct omnia_eeprom oep;

	if (!ram_size) {
		/* Get the board config from EEPROM */
		if (omnia_read_eeprom(&oep)) {
			debug("Memory config in EEPROM: 0x%02x\n", oep.ramsize);

			if (oep.ramsize == 0x2)
				ram_size = 2;
			else
				ram_size = 1;
		} else {
			/* Hardcoded fallback */
			puts("Memory config from EEPROM read failed!\n");
			puts("Falling back to default 1 GiB!\n");
			ram_size = 1;
		}
	}

	return ram_size;
}

/*
 * Define the DDR layout / topology here in the board file. This will
 * be used by the DDR3 init code in the SPL U-Boot version to configure
 * the DDR3 controller.
 */
static struct mv_ddr_topology_map board_topology_map_1g = {
	DEBUG_LEVEL_ERROR,
	0x1, /* active interfaces */
	/* cs_mask, mirror, dqs_swap, ck_swap X PUPs */
	{ { { {0x1, 0, 0, 0},
	      {0x1, 0, 0, 0},
	      {0x1, 0, 0, 0},
	      {0x1, 0, 0, 0},
	      {0x1, 0, 0, 0} },
	    SPEED_BIN_DDR_1600K,	/* speed_bin */
	    MV_DDR_DEV_WIDTH_16BIT,	/* memory_width */
	    MV_DDR_DIE_CAP_4GBIT,			/* mem_size */
	    MV_DDR_FREQ_800,		/* frequency */
	    0, 0,			/* cas_wl cas_l */
	    MV_DDR_TEMP_NORMAL,		/* temperature */
	    MV_DDR_TIM_2T} },		/* timing */
	BUS_MASK_32BIT,			/* Busses mask */
	MV_DDR_CFG_DEFAULT,		/* ddr configuration data source */
	{ {0} },			/* raw spd data */
	{0}				/* timing parameters */
};

static struct mv_ddr_topology_map board_topology_map_2g = {
	DEBUG_LEVEL_ERROR,
	0x1, /* active interfaces */
	/* cs_mask, mirror, dqs_swap, ck_swap X PUPs */
	{ { { {0x1, 0, 0, 0},
	      {0x1, 0, 0, 0},
	      {0x1, 0, 0, 0},
	      {0x1, 0, 0, 0},
	      {0x1, 0, 0, 0} },
	    SPEED_BIN_DDR_1600K,	/* speed_bin */
	    MV_DDR_DEV_WIDTH_16BIT,	/* memory_width */
	    MV_DDR_DIE_CAP_8GBIT,			/* mem_size */
	    MV_DDR_FREQ_800,		/* frequency */
	    0, 0,			/* cas_wl cas_l */
	    MV_DDR_TEMP_NORMAL,		/* temperature */
	    MV_DDR_TIM_2T} },		/* timing */
	BUS_MASK_32BIT,			/* Busses mask */
	MV_DDR_CFG_DEFAULT,		/* ddr configuration data source */
	{ {0} },			/* raw spd data */
	{0}				/* timing parameters */
};

struct mv_ddr_topology_map *mv_ddr_topology_map_get(void)
{
	if (omnia_get_ram_size_gb() == 2)
		return &board_topology_map_2g;
	else
		return &board_topology_map_1g;
}

#ifndef CONFIG_SPL_BUILD
static int set_regdomain(void)
{
	struct omnia_eeprom oep;
	char rd[3] = {' ', ' ', 0};

	if (omnia_read_eeprom(&oep))
		memcpy(rd, &oep.region, 2);
	else
		puts("EEPROM regdomain read failed.\n");

	printf("Regdomain set to %s\n", rd);
	return env_set("regdomain", rd);
}

/*
 * default factory reset bootcommand on Omnia first sets all the front LEDs
 * to green and then tries to load the rescue image from SPI flash memory and
 * boot it
 */
#define OMNIA_FACTORY_RESET_BOOTCMD \
	"i2c dev 2; " \
	"i2c mw 0x2a.1 0x3 0x1c 1; " \
	"i2c mw 0x2a.1 0x4 0x1c 1; " \
	"mw.l 0x01000000 0x00ff000c; " \
	"i2c write 0x01000000 0x2a.1 0x5 4 -s; " \
	"setenv bootargs \"$bootargs omniarescue=$omnia_reset\"; " \
	"sf probe; " \
	"sf read 0x1000000 0x100000 0x700000; " \
	"bootm 0x1000000; " \
	"bootz 0x1000000"

static void handle_reset_button(void)
{
	int ret;
	u8 reset_status;

	ret = omnia_mcu_read(CMD_GET_RESET, &reset_status, 1);
	if (ret) {
		printf("omnia_mcu_read failed: %i, reset status unknown!\n",
		       ret);
		return;
	}

	env_set_ulong("omnia_reset", reset_status);

	if (reset_status) {
		printf("RESET button was pressed, overwriting bootcmd!\n");
		env_set("bootcmd", OMNIA_FACTORY_RESET_BOOTCMD);
	}
}
#endif

int board_early_init_f(void)
{
	/* Configure MPP */
	writel(0x11111111, MVEBU_MPP_BASE + 0x00);
	writel(0x11111111, MVEBU_MPP_BASE + 0x04);
	writel(0x11244011, MVEBU_MPP_BASE + 0x08);
	writel(0x22222111, MVEBU_MPP_BASE + 0x0c);
	writel(0x22200002, MVEBU_MPP_BASE + 0x10);
	writel(0x30042022, MVEBU_MPP_BASE + 0x14);
	writel(0x55550555, MVEBU_MPP_BASE + 0x18);
	writel(0x00005550, MVEBU_MPP_BASE + 0x1c);

	/* Set GPP Out value */
	writel(OMNIA_GPP_OUT_VAL_LOW, MVEBU_GPIO0_BASE + 0x00);
	writel(OMNIA_GPP_OUT_VAL_MID, MVEBU_GPIO1_BASE + 0x00);

	/* Set GPP Polarity */
	writel(OMNIA_GPP_POL_LOW, MVEBU_GPIO0_BASE + 0x0c);
	writel(OMNIA_GPP_POL_MID, MVEBU_GPIO1_BASE + 0x0c);

	/* Set GPP Out Enable */
	writel(OMNIA_GPP_OUT_ENA_LOW, MVEBU_GPIO0_BASE + 0x04);
	writel(OMNIA_GPP_OUT_ENA_MID, MVEBU_GPIO1_BASE + 0x04);

	return 0;
}

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = mvebu_sdram_bar(0) + 0x100;

#ifndef CONFIG_SPL_BUILD
	disable_mcu_watchdog();
#endif

	return 0;
}

int board_late_init(void)
{
#ifndef CONFIG_SPL_BUILD
	set_regdomain();
	handle_reset_button();
#endif

	return 0;
}

static struct udevice *get_atsha204a_dev(void)
{
	static struct udevice *dev;

	if (dev)
		return dev;

	if (uclass_get_device_by_name(UCLASS_MISC, "atsha204a@64", &dev)) {
		puts("Cannot find ATSHA204A on I2C bus!\n");
		dev = NULL;
	}

	return dev;
}

int checkboard(void)
{
	u32 version_num, serial_num;
	int err = 1;

	struct udevice *dev = get_atsha204a_dev();

	if (dev) {
		err = atsha204a_wakeup(dev);
		if (err)
			goto out;

		err = atsha204a_read(dev, ATSHA204A_ZONE_OTP, false,
				     OMNIA_ATSHA204_OTP_VERSION,
				     (u8 *)&version_num);
		if (err)
			goto out;

		err = atsha204a_read(dev, ATSHA204A_ZONE_OTP, false,
				     OMNIA_ATSHA204_OTP_SERIAL,
				     (u8 *)&serial_num);
		if (err)
			goto out;

		atsha204a_sleep(dev);
	}

out:
	printf("Turris Omnia:\n");
	printf("  RAM size: %i MiB\n", omnia_get_ram_size_gb() * 1024);
	if (err)
		printf("  Serial Number: unknown\n");
	else
		printf("  Serial Number: %08X%08X\n", be32_to_cpu(version_num),
		       be32_to_cpu(serial_num));

	return 0;
}

static void increment_mac(u8 *mac)
{
	int i;

	for (i = 5; i >= 3; i--) {
		mac[i] += 1;
		if (mac[i])
			break;
	}
}

int misc_init_r(void)
{
	int err;
	struct udevice *dev = get_atsha204a_dev();
	u8 mac0[4], mac1[4], mac[6];

	if (!dev)
		goto out;

	err = atsha204a_wakeup(dev);
	if (err)
		goto out;

	err = atsha204a_read(dev, ATSHA204A_ZONE_OTP, false,
			     OMNIA_ATSHA204_OTP_MAC0, mac0);
	if (err)
		goto out;

	err = atsha204a_read(dev, ATSHA204A_ZONE_OTP, false,
			     OMNIA_ATSHA204_OTP_MAC1, mac1);
	if (err)
		goto out;

	atsha204a_sleep(dev);

	mac[0] = mac0[1];
	mac[1] = mac0[2];
	mac[2] = mac0[3];
	mac[3] = mac1[1];
	mac[4] = mac1[2];
	mac[5] = mac1[3];

	if (is_valid_ethaddr(mac))
		eth_env_set_enetaddr("ethaddr", mac);

	increment_mac(mac);

	if (is_valid_ethaddr(mac))
		eth_env_set_enetaddr("eth1addr", mac);

	increment_mac(mac);

	if (is_valid_ethaddr(mac))
		eth_env_set_enetaddr("eth2addr", mac);

out:
	return 0;
}

