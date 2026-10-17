// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 SiFive, Inc.
 *
 * Based on board/freescale/common/sys_eeprom.c:
 * Copyright 2006, 2008-2009, 2011 Freescale Semiconductor
 * York Sun (yorksun@freescale.com)
 * Haiying Wang (haiying.wang@freescale.com)
 * Timur Tabi (timur@freescale.com)
 */

#include <command.h>
#include <env.h>
#include <i2c.h>
#include <init.h>
#include <linux/ctype.h>
#include <linux/delay.h>
#include <u-boot/crc.h>

#ifndef CONFIG_SYS_EEPROM_BUS_NUM
#error Requires CONFIG_SYS_EEPROM_BUS_NUM to be defined
#endif

#define FORMAT_VERSION				0x1

/* Options for the manuf_test_status field */
#define SIFIVE_MANUF_TEST_STATUS_UNKNOWN	0
#define SIFIVE_MANUF_TEST_STATUS_PASS		1
#define SIFIVE_MANUF_TEST_STATUS_FAIL		2

/*
 * BYTES_PER_EEPROM_PAGE: the AT24C02 datasheet says that data can
 * only be written in page mode, which means 8 bytes at a time
 */
#define BYTES_PER_EEPROM_PAGE			8

/*
 * EEPROM_WRITE_DELAY_MS: the AT24C02 datasheet says it takes up to
 * 5ms to complete a given write
 */
#define EEPROM_WRITE_DELAY_MS			5000

/*
 * MAGIC_NUMBER_BYTES: number of bytes used by the magic number
 */
#define MAGIC_NUMBER_BYTES			4

/*
 * SERIAL_NUMBER_BYTES: number of bytes used by the board serial
 * number
 */
#define SERIAL_NUMBER_BYTES			16

/*
 * MAC_ADDR_BYTES: number of bytes used by the Ethernet MAC address
 */
#define MAC_ADDR_BYTES				6

/*
 * MAC_ADDR_STRLEN: length of mac address string
 */
#define MAC_ADDR_STRLEN				17

/*
 * SiFive OUI. Registration Date is 2018-02-15
 */
#define SIFIVE_OUI_PREFIX			"70:B3:D5:92:F"

/**
 * static eeprom: EEPROM layout for the SiFive platform I2C format
 */
static struct __attribute__ ((__packed__)) sifive_eeprom {
	u8 magic[MAGIC_NUMBER_BYTES];
	u8 format_ver;
	u16 product_id;
	u8 pcb_revision;
	u8 bom_revision;
	u8 bom_variant;
	u8 serial[SERIAL_NUMBER_BYTES];
	u8 manuf_test_status;
	u8 mac_addr[MAC_ADDR_BYTES];
	u32 crc;
} e;

struct sifive_product {
	u16 id;
	const char *name;
};

/* Set to 1 if we've read EEPROM into memory */
static int has_been_read;

/* Magic number at the first four bytes of EEPROM */
static const unsigned char magic[MAGIC_NUMBER_BYTES] = { 0xf1, 0x5e, 0x50, 0x45 };

/* Does the magic number match that of a SiFive EEPROM? */
static inline int is_match_magic(void)
{
	return (memcmp(&e.magic, &magic, MAGIC_NUMBER_BYTES) == 0);
}

/* Calculate the current CRC */
static inline u32 calculate_crc32(void)
{
	return crc32(0, (void *)&e, sizeof(struct sifive_eeprom) - sizeof(e.crc));
}

/* This function should be called after each update to the EEPROM structure */
static inline void update_crc(void)
{
	e.crc = calculate_crc32();
}

static struct sifive_product sifive_products[] = {
	{ 0, "Unknown"},
	{ 2, "HiFive Unmatched" },
};

/**
 * dump_raw_eeprom - display the raw contents of the EEPROM
 */
static void dump_raw_eeprom(void)
{
	unsigned int i;

	printf("EEPROM dump: (0x%lx bytes)\n", sizeof(e));
	for (i = 0; i < sizeof(e); i++) {
		if ((i % 16) == 0)
			printf("%02X: ", i);
		printf("%02X ", ((u8 *)&e)[i]);
		if (((i % 16) == 15) || (i == sizeof(e) - 1))
			printf("\n");
	}
}

/**
 * show_eeprom - display the contents of the EEPROM
 */
static void show_eeprom(void)
{
	unsigned int i;
	u32 crc;
	const char *product_name = "Unknown";
	char board_serial[SERIAL_NUMBER_BYTES + 1] = { 0 };

	if (!is_match_magic()) {
		printf("Not a SiFive HiFive EEPROM data format - magic bytes don't match\n");
		dump_raw_eeprom();
		return;
	};

	snprintf(board_serial, sizeof(board_serial), "%s", e.serial);

	for (i = 0; i < ARRAY_SIZE(sifive_products); i++) {
		if (sifive_products[i].id == e.product_id) {
			product_name = sifive_products[i].name;
			break;
		}
	};

	printf("SiFive PCB EEPROM format v%u\n", e.format_ver);
	printf("Product ID: %04hx (%s)\n", e.product_id, product_name);
	printf("PCB revision: %x\n", e.pcb_revision);
	printf("BOM revision: %c\n", e.bom_revision);
	printf("BOM variant: %x\n", e.bom_variant);
	printf("Serial number: %s\n", board_serial);
	printf("Ethernet MAC address: %02x:%02x:%02x:%02x:%02x:%02x\n",
	       e.mac_addr[0], e.mac_addr[1], e.mac_addr[2],
	       e.mac_addr[3], e.mac_addr[4], e.mac_addr[5]);

	crc = calculate_crc32();
	if (crc == e.crc) {
		printf("CRC: %08x\n", e.crc);
	} else {
		printf("CRC: %08x (should be %08x)\n", e.crc, crc);
		dump_raw_eeprom();
	}
}

/**
 * read_eeprom() - read the EEPROM into memory, if it hasn't been read already
 */
static int read_eeprom(void)
{
	int ret;
	struct udevice *dev;

	if (has_been_read)
		return 0;

	ret = i2c_get_chip_for_busnum(CONFIG_SYS_EEPROM_BUS_NUM,
				      CONFIG_SYS_I2C_EEPROM_ADDR,
				      1,
				      &dev);
	if (!ret)
		dm_i2c_read(dev, 0, (void *)&e,
			    sizeof(struct sifive_eeprom));

	show_eeprom();

	has_been_read = (ret == 0) ? 1 : 0;

	return ret;
}

/**
 * prog_eeprom() - write the EEPROM from memory
 */
static int prog_eeprom(void)
{
	int ret = 0;
	unsigned int i;
	void *p;

	if (!is_match_magic()) {
		printf("Please read the EEPROM ('read_eeprom') and/or initialize the EEPROM ('initialize') first.\n");
		return 0;
	}

	for (i = 0, p = &e; i < sizeof(e);
	     i += BYTES_PER_EEPROM_PAGE, p += BYTES_PER_EEPROM_PAGE) {
		struct udevice *dev;

		ret = i2c_get_chip_for_busnum(CONFIG_SYS_EEPROM_BUS_NUM,
					      CONFIG_SYS_I2C_EEPROM_ADDR,
					      CONFIG_SYS_I2C_EEPROM_ADDR_LEN,
					      &dev);
		if (!ret)
			ret = dm_i2c_write(dev, i, p,
					   min((int)(sizeof(e) - i),
					       BYTES_PER_EEPROM_PAGE));

		if (ret)
			break;

		udelay(EEPROM_WRITE_DELAY_MS);
	}

	if (!ret) {
		/* Verify the write by reading back the EEPROM and comparing */
		struct sifive_eeprom e2;
		struct udevice *dev;

		ret = i2c_get_chip_for_busnum(CONFIG_SYS_EEPROM_BUS_NUM,
					      CONFIG_SYS_I2C_EEPROM_ADDR,
					      CONFIG_SYS_I2C_EEPROM_ADDR_LEN,
					      &dev);
		if (!ret)
			ret = dm_i2c_read(dev, 0, (void *)&e2, sizeof(e2));
		if (!ret && memcmp(&e, &e2, sizeof(e)))
			ret = -1;
	}

	if (ret) {
		printf("Programming failed.\n");
		has_been_read = 0;
		return -1;
	}

	printf("Programming passed.\n");
	return 0;
}

/**
 * set_mac_address() - stores a MAC address into the local EEPROM copy
 *
 * This function takes a pointer to MAC address string
 * (i.e."XX:XX:XX:XX:XX:XX", where "XX" is a two-digit hex number),
 * stores it in the MAC address field of the EEPROM local copy, and
 * updates the local copy of the CRC.
 */
static void set_mac_address(char *string)
{
	unsigned int i;

	if (strncasecmp(SIFIVE_OUI_PREFIX, string, 13)) {
		printf("The MAC address doesn't match SiFive OUI %s\n",
		       SIFIVE_OUI_PREFIX);
		return;
	}

	for (i = 0; *string && (i < MAC_ADDR_BYTES); i++) {
		e.mac_addr[i] = hextoul(string, &string);
		if (*string == ':')
			string++;
	}

	update_crc();
}

/**
 * set_manuf_test_status() - stores a test status byte into the in-memory copy
 *
 * Takes a pointer to a manufacturing test status string ("unknown",
 * "pass", "fail") and stores the corresponding numeric ID to the
 * manuf_test_status field of the EEPROM local copy, and updates the
 * CRC of the local copy.
 */
static void set_manuf_test_status(char *string)
{
	if (!strcasecmp(string, "unknown")) {
		e.manuf_test_status = SIFIVE_MANUF_TEST_STATUS_UNKNOWN;
	} else if (!strcasecmp(string, "pass")) {
		e.manuf_test_status = SIFIVE_MANUF_TEST_STATUS_PASS;
	} else if (!strcasecmp(string, "fail")) {
		e.manuf_test_status = SIFIVE_MANUF_TEST_STATUS_FAIL;
	} else {
		printf("Usage: mac manuf_test_status (unknown|pass|fail)\n");
		return;
	}

	update_crc();
}

/**
 * set_pcb_revision() - stores a SiFive PCB revision into the local EEPROM copy
 *
 * Takes a pointer to a string representing the numeric PCB revision in
 * decimal ("0" - "255"), stores it in the pcb_revision field of the
 * EEPROM local copy, and updates the CRC of the local copy.
 */
static void set_pcb_revision(char *string)
{
	unsigned long p;

	p = dectoul(string, &string);
	if (p > U8_MAX) {
		printf("%s must not be greater than %d\n", "PCB revision",
		       U8_MAX);
		return;
	}

	e.pcb_revision = p;

	update_crc();
}

/**
 * set_bom_revision() - stores a SiFive BOM revision into the local EEPROM copy
 *
 * Takes a pointer to a uppercase ASCII character representing the BOM
 * revision ("A" - "Z"), stores it in the bom_revision field of the
 * EEPROM local copy, and updates the CRC of the local copy.
 */
static void set_bom_revision(char *string)
{
	if (string[0] < 'A' || string[0] > 'Z') {
		printf("BOM revision must be an uppercase letter between A and Z\n");
		return;
	}

	e.bom_revision = string[0];

	update_crc();
}

/**
 * set_bom_variant() - stores a SiFive BOM variant into the local EEPROM copy
 *
 * Takes a pointer to a string representing the numeric BOM variant in
 * decimal ("0" - "255"), stores it in the bom_variant field of the
 * EEPROM local copy, and updates the CRC of the local copy.
 */
static void set_bom_variant(char *string)
{
	unsigned long p;

	p = dectoul(string, &string);
	if (p > U8_MAX) {
		printf("%s must not be greater than %d\n", "BOM variant",
		       U8_MAX);
		return;
	}

	e.bom_variant = p;

	update_crc();
}

/**
 * set_product_id() - stores a SiFive product ID into the local EEPROM copy
 *
 * Takes a pointer to a string representing the numeric product ID  in
 * decimal ("0" - "65535"), stores it in the product ID field of the
 * EEPROM local copy, and updates the CRC of the local copy.
 */
static void set_product_id(char *string)
{
	unsigned long p;

	p = dectoul(string, &string);
	if (p > U16_MAX) {
		printf("%s must not be greater than %d\n", "Product ID",
		       U16_MAX);
		return;
	}

	e.product_id = p;

	update_crc();
}

/**
 * init_local_copy() - initialize the in-memory EEPROM copy
 *
 * Initialize the in-memory EEPROM copy with the magic number.  Must
 * be done when preparing to initialize a blank EEPROM, or overwrite
 * one with a corrupted magic number.
 */
static void init_local_copy(void)
{
	memset(&e, 0, sizeof(e));
	memcpy(e.magic, magic, sizeof(e.magic));
	e.format_ver = FORMAT_VERSION;
	update_crc();
}

int do_mac(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	char *cmd;

	if (argc == 1) {
		show_eeprom();
		return 0;
	}

	if (argc > 3)
		return CMD_RET_USAGE;

	cmd = argv[1];

	/* Commands with no argument */
	if (!strcmp(cmd, "read_eeprom")) {
		read_eeprom();
		return 0;
	} else if (!strcmp(cmd, "initialize")) {
		init_local_copy();
		return 0;
	} else if (!strcmp(cmd, "write_eeprom")) {
		prog_eeprom();
		return 0;
	}

	if (argc != 3)
		return CMD_RET_USAGE;

	if (!is_match_magic()) {
		printf("Please read the EEPROM ('read_eeprom') and/or initialize the EEPROM ('initialize') first.\n");
		return 0;
	}

	if (!strcmp(cmd, "manuf_test_status")) {
		set_manuf_test_status(argv[2]);
		return 0;
	} else if (!strcmp(cmd, "mac_address")) {
		set_mac_address(argv[2]);
		return 0;
	} else if (!strcmp(cmd, "pcb_revision")) {
		set_pcb_revision(argv[2]);
		return 0;
	} else if (!strcmp(cmd, "bom_variant")) {
		set_bom_variant(argv[2]);
		return 0;
	} else if (!strcmp(cmd, "bom_revision")) {
		set_bom_revision(argv[2]);
		return 0;
	} else if (!strcmp(cmd, "product_id")) {
		set_product_id(argv[2]);
		return 0;
	}

	return CMD_RET_USAGE;
}

/**
 * mac_read_from_eeprom() - read the MAC address from EEPROM
 *
 * This function reads the MAC address from EEPROM and sets the
 * appropriate environment variables for each one read.
 *
 * The environment variables are only set if they haven't been set already.
 * This ensures that any user-saved variables are never overwritten.
 *
 * This function must be called after relocation.
 */
int mac_read_from_eeprom(void)
{
	u32 crc;
	char board_serial[SERIAL_NUMBER_BYTES + 1] = { 0 };

	puts("EEPROM: ");

	if (read_eeprom()) {
		printf("Read failed.\n");
		return 0;
	}

	if (!is_match_magic()) {
		printf("Invalid ID (%02x %02x %02x %02x)\n",
		       e.magic[0], e.magic[1], e.magic[2], e.magic[3]);
		dump_raw_eeprom();
		return 0;
	}

	crc = calculate_crc32();
	if (crc != e.crc) {
		printf("CRC mismatch (%08x != %08x)\n", crc, e.crc);
		dump_raw_eeprom();
		return 0;
	}

	eth_env_set_enetaddr("ethaddr", e.mac_addr);

	if (!env_get("serial#")) {
		snprintf(board_serial, sizeof(board_serial), "%s", e.serial);
		env_set("serial#", board_serial);
	}

	return 0;
}

/**
 * get_pcb_revision_from_eeprom - get the PCB revision
 *
 * Read the EEPROM to determine the board revision.
 *
 * This function is called before relocation, so we need to read a private
 * copy of the EEPROM into a local variable on the stack.
 */
u8 get_pcb_revision_from_eeprom(void)
{
	struct __attribute__ ((__packed__)) board_eeprom {
		u8 magic[MAGIC_NUMBER_BYTES];
		u8 format_ver;
		u16 product_id;
		u8 pcb_revision;
	} be;

	int ret;
	struct udevice *dev;

	ret = i2c_get_chip_for_busnum(CONFIG_SYS_EEPROM_BUS_NUM,
				      CONFIG_SYS_I2C_EEPROM_ADDR,
				      1,
				      &dev);

	if (!ret)
		dm_i2c_read(dev, 0, (void *)&be,
			    sizeof(struct board_eeprom));

	return be.pcb_revision;
}

U_BOOT_LONGHELP(mac,
	"- displays memory copy of EEPROM\n"
	"mac read_eeprom - reads EEPROM into memory\n"
	"mac initialize - initializes memory copy with magic number\n"
	"mac write_eeprom -  writes the EEPROM from memory\n"
	"mac manuf_test_status [unknown|pass|fail] - sets test status in memory\n"
	"mac_address <addr> - sets MAC address in memory\n"
	"mac pcb_revision <rev> - sets PCB revision in memory\n"
	"mac bom_variant <var> - sets BOM variant in memory\n"
	"mac bom_revision <rev> - sets BOM revision in memory\n");

U_BOOT_CMD(
	mac, 3, 1,  do_mac,
	"display and program the board revision and MAC address in EEPROM",
	mac_help_text);
