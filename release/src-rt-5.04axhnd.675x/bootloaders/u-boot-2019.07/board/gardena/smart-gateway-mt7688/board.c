// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Stefan Roese <sr@denx.de>
 */

#include <common.h>
#include <environment.h>
#include <led.h>
#include <net.h>
#include <spi.h>
#include <spi_flash.h>
#include <uuid.h>
#include <linux/ctype.h>
#include <linux/io.h>

#define MT76XX_AGPIO_CFG	0x1000003c

#define FACTORY_DATA_OFFS	0xc0000
#define FACTORY_DATA_SECT_SIZE	0x10000
#if ((CONFIG_ENV_OFFSET_REDUND + CONFIG_ENV_SIZE_REDUND) > FACTORY_DATA_OFFS)
#error "U-Boot image with environment too big (overlapping with factory-data)!"
#endif
#define FACTORY_DATA_USER_OFFS	0x140
#define FACTORY_DATA_SIZE	0x1f0
#define FACTORY_DATA_CRC_LEN	(FACTORY_DATA_SIZE -			\
				 FACTORY_DATA_USER_OFFS - sizeof(u32))

#define FACTORY_DATA_MAGIC	0xCAFEBABE

struct factory_data_values {
	u8 pad_1[4];
	u8 wifi_mac[6];		/* offs: 0x004: binary value */
	u8 pad_2[30];
	u8 eth_mac[6];		/* offs: 0x028: binary value */
	u8 pad_3[FACTORY_DATA_USER_OFFS - 4 - 6 - 30 - 6];
	/* User values start here at offset 0x140 */
	u32 crc;
	u32 magic;
	u32 version;
	char ipr_id[UUID_STR_LEN];	/* UUID as string w/o ending \0 */
	char hqv_id[UUID_STR_LEN];	/* UUID as string w/o ending \0 */
	char unielec_id[UUID_STR_LEN];	/* UUID as string w/o ending \0 */
};

int board_early_init_f(void)
{
	void __iomem *gpio_mode;

	/* Configure digital vs analog GPIOs */
	gpio_mode = ioremap_nocache(MT76XX_AGPIO_CFG, 0x100);
	iowrite32(0x00fe01ff, gpio_mode);

	return 0;
}

static bool prepare_uuid_var(const char *fd_ptr, const char *env_var_name,
			     char errorchar)
{
	char str[UUID_STR_LEN + 1] = { 0 };	/* Enough for UUID stuff */
	bool env_updated = false;
	char *env;
	int i;

	memcpy(str, fd_ptr, UUID_STR_LEN);

	/* Convert non-ascii character to 'X' */
	for (i = 0; i < UUID_STR_LEN; i++) {
		if (!(isascii(str[i]) && isprint(str[i])))
			str[i] = errorchar;
	}

	env = env_get(env_var_name);
	if (strcmp(env, str)) {
		env_set(env_var_name, str);
		env_updated = true;
	}

	return env_updated;
}

static void factory_data_env_config(void)
{
	struct factory_data_values *fd;
	struct spi_flash *sf;
	int env_updated = 0;
	char str[UUID_STR_LEN + 1];	/* Enough for UUID stuff */
	char *env;
	u8 *buf;
	u32 crc;
	int ret;
	u8 *ptr;

	buf = malloc(FACTORY_DATA_SIZE);
	if (!buf) {
		printf("F-Data:Unable to allocate buffer\n");
		return;
	}

	/*
	 * Get values from factory-data area in SPI NOR
	 */
	sf = spi_flash_probe(CONFIG_SF_DEFAULT_BUS,
			     CONFIG_SF_DEFAULT_CS,
			     CONFIG_SF_DEFAULT_SPEED,
			     CONFIG_SF_DEFAULT_MODE);
	if (!sf) {
		printf("F-Data:Unable to access SPI NOR flash\n");
		goto err_free;
	}

	ret = spi_flash_read(sf, FACTORY_DATA_OFFS, FACTORY_DATA_SIZE,
			     (void *)buf);
	if (ret) {
		printf("F-Data:Unable to read factory-data from SPI NOR\n");
		goto err_spi_flash;
	}

	fd = (struct factory_data_values *)buf;

	if (fd->magic != FACTORY_DATA_MAGIC)
		printf("F-Data:Magic value not correct\n");

	crc = crc32(0, (u8 *)&fd->magic, FACTORY_DATA_CRC_LEN);
	if (crc != fd->crc)
		printf("F-Data:CRC not correct\n");
	else
		printf("F-Data:factory-data version %x detected\n",
		       fd->version);

	/* Handle wifi_mac env variable */
	ptr = fd->wifi_mac;
	sprintf(str, "%pM", ptr);
	if (!is_valid_ethaddr(ptr))
		printf("F-Data:Invalid MAC addr: wifi_mac %s\n", str);

	env = env_get("wifiaddr");
	if (strcmp(env, str)) {
		env_set("wifiaddr", str);
		env_updated = 1;
	}

	/* Handle eth_mac env variable */
	ptr = fd->eth_mac;
	sprintf(str, "%pM", ptr);
	if (!is_valid_ethaddr(ptr))
		printf("F-Data:Invalid MAC addr: eth_mac %s\n", str);

	env = env_get("ethaddr");
	if (strcmp(env, str)) {
		env_set("ethaddr", str);
		env_updated = 1;
	}

	/* Handle UUID env variables */
	env_updated |= prepare_uuid_var(fd->ipr_id, "linuxmoduleid", 'X');
	env_updated |= prepare_uuid_var(fd->hqv_id, "linuxmodulehqvid", '\0');
	env_updated |= prepare_uuid_var(fd->unielec_id,
					"linuxmoduleunielecid", '\0');

	/* Check if the environment was updated and needs to get stored */
	if (env_updated != 0) {
		printf("F-Data:Values don't match env values -> saving\n");
		env_save();
	} else {
		debug("F-Data:Values match current env values\n");
	}

err_spi_flash:
	spi_flash_free(sf);

err_free:
	free(buf);
}

int board_late_init(void)
{
	if (IS_ENABLED(CONFIG_LED))
		led_default_state();

	factory_data_env_config();

	return 0;
}

static void copy_or_generate_uuid(char *fd_ptr, const char *env_var_name)
{
	char str[UUID_STR_LEN + 1] = { 0 };	/* Enough for UUID stuff */
	char *env;

	/* Don't use the UUID dest place, as the \0 char won't fit */
	env = env_get(env_var_name);
	if (env)
		strncpy(str, env, UUID_STR_LEN);
	else
		gen_rand_uuid_str(str, UUID_STR_FORMAT_STD);

	memcpy(fd_ptr, str, UUID_STR_LEN);
}

/*
 * Helper function to provide some sane factory-data values for testing
 * purpose, when these values are not programmed correctly
 */
int do_fd_write(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	struct factory_data_values *fd;
	struct spi_flash *sf;
	u8 *buf;
	int ret = CMD_RET_FAILURE;

	buf = malloc(FACTORY_DATA_SECT_SIZE);
	if (!buf) {
		printf("F-Data:Unable to allocate buffer\n");
		return CMD_RET_FAILURE;
	}

	sf = spi_flash_probe(CONFIG_SF_DEFAULT_BUS,
			     CONFIG_SF_DEFAULT_CS,
			     CONFIG_SF_DEFAULT_SPEED,
			     CONFIG_SF_DEFAULT_MODE);
	if (!sf) {
		printf("F-Data:Unable to access SPI NOR flash\n");
		goto err_free;
	}

	/* Generate the factory-data struct */

	/* Fist read complete sector into buffer */
	ret = spi_flash_read(sf, FACTORY_DATA_OFFS, FACTORY_DATA_SECT_SIZE,
			     (void *)buf);
	if (ret) {
		printf("F-Data:spi_flash_read failed (%d)\n", ret);
		goto err_spi_flash;
	}

	fd = (struct factory_data_values *)buf;
	fd->magic = FACTORY_DATA_MAGIC;
	fd->version = 0x1;

	/* Use existing MAC and UUID values or generate some random ones */
	if (!eth_env_get_enetaddr("wifiaddr", fd->wifi_mac)) {
		net_random_ethaddr(fd->wifi_mac);
		/* to get a different seed value for the MAC address */
		mdelay(10);
	}

	if (!eth_env_get_enetaddr("ethaddr", fd->eth_mac))
		net_random_ethaddr(fd->eth_mac);

	copy_or_generate_uuid(fd->ipr_id, "linuxmoduleid");
	copy_or_generate_uuid(fd->hqv_id, "linuxmodulehqvid");
	copy_or_generate_uuid(fd->unielec_id, "linuxmoduleunielecid");

	printf("New factory-data values:\n");
	printf("wifiaddr=%pM\n", fd->wifi_mac);
	printf("ethaddr=%pM\n", fd->eth_mac);

	/*
	 * We don't have the \0 char at the end, so we need to specify the
	 * length in the printf format instead
	 */
	printf("linuxmoduleid=%." __stringify(UUID_STR_LEN) "s\n", fd->ipr_id);
	printf("linuxmodulehqvid=%." __stringify(UUID_STR_LEN) "s\n",
	       fd->hqv_id);
	printf("linuxmoduleunielecid=%." __stringify(UUID_STR_LEN) "s\n",
	       fd->unielec_id);

	fd->crc = crc32(0, (u8 *)&fd->magic, FACTORY_DATA_CRC_LEN);

	ret = spi_flash_erase(sf, FACTORY_DATA_OFFS, FACTORY_DATA_SECT_SIZE);
	if (ret) {
		printf("F-Data:spi_flash_erase failed (%d)\n", ret);
		goto err_spi_flash;
	}

	ret = spi_flash_write(sf, FACTORY_DATA_OFFS, FACTORY_DATA_SECT_SIZE,
			      buf);
	if (ret) {
		printf("F-Data:spi_flash_write failed (%d)\n", ret);
		goto err_spi_flash;
	}

	printf("F-Data:factory-data values written to SPI NOR flash\n");

err_spi_flash:
	spi_flash_free(sf);

err_free:
	free(buf);

	return ret;
}

U_BOOT_CMD(
	fd_write,	1,	0,	do_fd_write,
	"Write test factory-data values to SPI NOR",
	"\n"
);
