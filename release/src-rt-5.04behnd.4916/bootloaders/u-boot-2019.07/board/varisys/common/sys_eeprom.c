// SPDX-License-Identifier: GPL-2.0+
/*
 * Based on board/freescale/common/sys_eeprom.c
 * Copyright 2006, 2008-2009, 2011 Freescale Semiconductor
 *
 * This defines the API for storing board information in the
 * eeprom. It has been adapted from an earlier version of the
 * Freescale API, but has a number of key differences. Because
 * the two APIs are independent and may diverge further, the
 * Varisys version of the API is implemented separately here.
 */

#include <common.h>
#include <command.h>
#include <i2c.h>
#include <linux/ctype.h>

#include "eeprom.h"

#ifdef CONFIG_SYS_I2C_EEPROM_NXID_MAC
#define MAX_NUM_PORTS	CONFIG_SYS_I2C_EEPROM_NXID_MAC
#else
#define MAX_NUM_PORTS	8
#endif
#define NXID_VERSION	0

/**
 * static eeprom: EEPROM layout for NXID formats
 *
 * See Freescale application note AN3638 for details.
 */
static struct __attribute__ ((__packed__)) eeprom {
	u8 id[4];         /* 0x00 - 0x03 EEPROM Tag 'NXID' */
	u8 sn[12];        /* 0x04 - 0x0F Serial Number */
	u8 errata[5];     /* 0x10 - 0x14 Errata Level */
	u8 date[6];       /* 0x15 - 0x1a Build Date */
	u8 res_0;         /* 0x1b        Reserved */
	u32 version;      /* 0x1c - 0x1f NXID Version */
	u8 tempcal[8];    /* 0x20 - 0x27 Temperature Calibration Factors */
	u8 tempcalsys[2]; /* 0x28 - 0x29 System Temperature Calibration Factors */
	u8 tempcalflags;  /* 0x2a        Temperature Calibration Flags */
	u8 res_1[21];     /* 0x2b - 0x3f Reserved */
	u8 mac_count;     /* 0x40        Number of MAC addresses */
	u8 mac_flag;      /* 0x41        MAC table flags */
	u8 mac[MAX_NUM_PORTS][6];     /* 0x42 - x MAC addresses */
	u32 crc;          /* x+1         CRC32 checksum */
} e;

/* Set to 1 if we've read EEPROM into memory */
static int has_been_read;

/* Is this a valid NXID EEPROM? */
#define is_valid ((e.id[0] == 'N') || (e.id[1] == 'X') || \
		  (e.id[2] == 'I') || (e.id[3] == 'D'))

/** Fixed ID field in EEPROM */
static unsigned char uid[16];

static int eeprom_bus_num = -1;
static int eeprom_addr;
static int eeprom_addr_len;

/**
 * This must be called before any eeprom access.
 */
void init_eeprom(int bus_num, int addr, int addr_len)
{
	eeprom_bus_num = bus_num;
	eeprom_addr = addr;
	eeprom_addr_len = addr_len;
}

/**
 * show_eeprom - display the contents of the EEPROM
 */
void show_eeprom(void)
{
	int i;
	unsigned int crc;

	/* EEPROM tag ID, either CCID or NXID */
	printf("ID: %c%c%c%c v%u\n", e.id[0], e.id[1], e.id[2], e.id[3],
		be32_to_cpu(e.version));

	/* Serial number */
	printf("SN: %s\n", e.sn);

	printf("UID: ");
	for (i = 0; i < 16; i++)
		printf("%02x", uid[i]);
	printf("\n");

	/* Errata level. */
	printf("Errata: %s\n", e.errata);

	/* Build date, BCD date values, as YYMMDDhhmmss */
	printf("Build date: 20%02x/%02x/%02x %02x:%02x:%02x %s\n",
		e.date[0], e.date[1], e.date[2],
		e.date[3] & 0x7F, e.date[4], e.date[5],
		e.date[3] & 0x80 ? "PM" : "");

	/* Show MAC addresses  */
	for (i = 0; i < min(e.mac_count, (u8)MAX_NUM_PORTS); i++) {
		u8 *p = e.mac[i];

		printf("Eth%u: %02x:%02x:%02x:%02x:%02x:%02x\n", i,
		       p[0], p[1], p[2], p[3], p[4], p[5]);
	}

	crc = crc32(0, (void *)&e, sizeof(e) - 4);

	if (crc == be32_to_cpu(e.crc))
		printf("CRC: %08x\n", be32_to_cpu(e.crc));
	else
		printf("CRC: %08x (should be %08x)\n",
		       be32_to_cpu(e.crc), crc);

#ifdef DEBUG
	printf("EEPROM dump: (0x%x bytes)\n", sizeof(e));
	for (i = 0; i < sizeof(e); i++) {
		if ((i % 16) == 0)
			printf("%02X: ", i);
		printf("%02X ", ((u8 *)&e)[i]);
		if (((i % 16) == 15) || (i == sizeof(e) - 1))
			printf("\n");
	}
#endif
}

/**
 * read_eeprom - read the EEPROM into memory
 */
int read_eeprom(void)
{
	int ret;
	unsigned int bus;

	if (eeprom_bus_num < 0) {
		printf("EEPROM not configured\n");
		return -1;
	}

	if (has_been_read)
		return 0;

	bus = i2c_get_bus_num();
	i2c_set_bus_num(eeprom_bus_num);

	ret = i2c_read(eeprom_addr, 0, eeprom_addr_len,
		(void *)&e, sizeof(e));


	/* Fixed address of ID field */
	i2c_read(0x5f, 0x80, 1, uid, 16);

	i2c_set_bus_num(bus);

#ifdef DEBUG
	show_eeprom();
#endif

	has_been_read = (ret == 0) ? 1 : 0;

	return ret;
}

/**
 *  update_crc - update the CRC
 *
 *  This function should be called after each update to the EEPROM structure,
 *  to make sure the CRC is always correct.
 */
static void update_crc(void)
{
	u32 crc, crc_offset = offsetof(struct eeprom, crc);

	crc = crc32(0, (void *)&e, crc_offset);
	e.crc = cpu_to_be32(crc);
}

/**
 * prog_eeprom - write the EEPROM from memory
 */
static int prog_eeprom(void)
{
	int ret = 0;
	int i;
	void *p;
	unsigned int bus;

	if (eeprom_bus_num < 0) {
		printf("EEPROM not configured\n");
		return -1;
	}

	/* Set the reserved values to 0xFF   */
	e.res_0 = 0xFF;
	memset(e.res_1, 0xFF, sizeof(e.res_1));
	update_crc();

	bus = i2c_get_bus_num();
	i2c_set_bus_num(eeprom_bus_num);

	/*
	 * The AT24C02 datasheet says that data can only be written in page
	 * mode, which means 8 bytes at a time, and it takes up to 5ms to
	 * complete a given write.
	 */
	for (i = 0, p = &e; i < sizeof(e); i += 8, p += 8) {
		ret = i2c_write(eeprom_addr, i, eeprom_addr_len,
			p, min((int)(sizeof(e) - i), 8));
		if (ret)
			break;
		udelay(5000);	/* 5ms write cycle timing */
	}

	if (!ret) {
		/* Verify the write by reading back the EEPROM and comparing */
		struct eeprom e2;

		ret = i2c_read(eeprom_addr, 0,
			eeprom_addr_len, (void *)&e2, sizeof(e2));
		if (!ret && memcmp(&e, &e2, sizeof(e)))
			ret = -1;
	}

	i2c_set_bus_num(bus);

	if (ret) {
		printf("Programming failed.\n");
		has_been_read = 0;
		return -1;
	}

	printf("Programming passed.\n");
	return 0;
}

/**
 * h2i - converts hex character into a number
 *
 * This function takes a hexadecimal character (e.g. '7' or 'C') and returns
 * the integer equivalent.
 */
static inline u8 h2i(char p)
{
	if ((p >= '0') && (p <= '9'))
		return p - '0';

	if ((p >= 'A') && (p <= 'F'))
		return (p - 'A') + 10;

	if ((p >= 'a') && (p <= 'f'))
		return (p - 'a') + 10;

	return 0;
}

/**
 * set_date - stores the build date into the EEPROM
 *
 * This function takes a pointer to a string in the format "YYMMDDhhmmss"
 * (2-digit year, 2-digit month, etc), converts it to a 6-byte BCD string,
 * and stores it in the build date field of the EEPROM local copy.
 */
static void set_date(const char *string)
{
	unsigned int i;

	if (strlen(string) != 12) {
		printf("Usage: mac date YYMMDDhhmmss\n");
		return;
	}

	for (i = 0; i < 6; i++)
		e.date[i] = h2i(string[2 * i]) << 4 | h2i(string[2 * i + 1]);

	update_crc();
}

/**
 * set_mac_address - stores a MAC address into the EEPROM
 *
 * This function takes a pointer to MAC address string
 * (i.e."XX:XX:XX:XX:XX:XX", where "XX" is a two-digit hex number) and
 * stores it in one of the MAC address fields of the EEPROM local copy.
 */
static void set_mac_address(unsigned int index, const char *string)
{
	char *p = (char *)string;
	unsigned int i;

	if ((index >= MAX_NUM_PORTS) || !string) {
		printf("Usage: mac <n> XX:XX:XX:XX:XX:XX\n");
		return;
	}

	for (i = 0; *p && (i < 6); i++) {
		e.mac[index][i] = simple_strtoul(p, &p, 16);
		if (*p == ':')
			p++;
	}

	update_crc();
}

int do_mac(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	char cmd;

	if (argc == 1) {
		show_eeprom();
		return 0;
	}

	cmd = argv[1][0];

	if (cmd == 'r') {
		read_eeprom();
		return 0;
	}

	if (cmd == 'i') {
		memcpy(e.id, "NXID", sizeof(e.id));
		e.version = NXID_VERSION;
		update_crc();
		return 0;
	}

	if (!is_valid) {
		printf("Please read the EEPROM ('r') and/or set the ID ('i') first.\n");
		return 0;
	}

	if (argc == 2) {
		switch (cmd) {
		case 's':	/* save */
			prog_eeprom();
			break;
		default:
			return cmd_usage(cmdtp);
		}

		return 0;
	}

	/* We know we have at least one parameter  */

	switch (cmd) {
	case 'n':	/* serial number */
		memset(e.sn, 0, sizeof(e.sn));
		strncpy((char *)e.sn, argv[2], sizeof(e.sn) - 1);
		update_crc();
		break;
	case 'e':	/* errata */
		memset(e.errata, 0, 5);
		strncpy((char *)e.errata, argv[2], 4);
		update_crc();
		break;
	case 'd':	/* date BCD format YYMMDDhhmmss */
		set_date(argv[2]);
		break;
	case 'p':	/* MAC table size */
		e.mac_count = simple_strtoul(argv[2], NULL, 16);
		update_crc();
		break;
	case '0' ... '9':	/* "mac 0" through "mac 22" */
		set_mac_address(simple_strtoul(argv[1], NULL, 10), argv[2]);
		break;
	case 'h':	/* help */
	default:
		return cmd_usage(cmdtp);
	}

	return 0;
}

int mac_read_from_generic_eeprom(const char *envvar, int chip,
	int address, int mac_bus)
{
	int ret;
	unsigned int bus;
	unsigned char mac[6];
	char ethaddr[18];

	bus = i2c_get_bus_num();
	i2c_set_bus_num(mac_bus);

	ret = i2c_read(chip, address, 1, mac, 6);

	i2c_set_bus_num(bus);

	if (!ret) {
		sprintf(ethaddr, "%02X:%02X:%02X:%02X:%02X:%02X",
			mac[0],
			mac[1],
			mac[2],
			mac[3],
			mac[4],
			mac[5]);

		printf("MAC: %s\n", ethaddr);
		env_set(envvar, ethaddr);
	}

	return ret;
}

void mac_read_from_fixed_id(void)
{
#ifdef CONFIG_SYS_I2C_MAC1_CHIP_ADDR
	mac_read_from_generic_eeprom("ethaddr", CONFIG_SYS_I2C_MAC1_CHIP_ADDR,
		CONFIG_SYS_I2C_MAC1_DATA_ADDR, CONFIG_SYS_I2C_MAC1_BUS);
#endif
#ifdef CONFIG_SYS_I2C_MAC2_CHIP_ADDR
	mac_read_from_generic_eeprom("eth1addr", CONFIG_SYS_I2C_MAC2_CHIP_ADDR,
		CONFIG_SYS_I2C_MAC2_DATA_ADDR, CONFIG_SYS_I2C_MAC2_BUS);
#endif
}

/**
 * mac_read_from_eeprom - read the MAC addresses from EEPROM
 *
 * This function reads the MAC addresses from EEPROM and sets the
 * appropriate environment variables for each one read.
 *
 * The environment variables are only set if they haven't been set already.
 * This ensures that any user-saved variables are never overwritten.
 *
 * This function must be called after relocation.
 *
 * For NXID v1 EEPROMs, we support loading and up-converting the older NXID v0
 * format.  In a v0 EEPROM, there are only eight MAC addresses and the CRC is
 * located at a different offset.
 */
int mac_read_from_eeprom_common(void)
{
	unsigned int i;
	u32 crc, crc_offset = offsetof(struct eeprom, crc);
	u32 *crcp; /* Pointer to the CRC in the data read from the EEPROM */

	puts("EEPROM: ");

	if (read_eeprom()) {
		printf("Read failed.\n");
		return 0;
	}

	if (!is_valid) {
		printf("Invalid ID (%02x %02x %02x %02x)\n",
		       e.id[0], e.id[1], e.id[2], e.id[3]);
		return 0;
	}

	crc = crc32(0, (void *)&e, crc_offset);
	crcp = (void *)&e + crc_offset;
	if (crc != be32_to_cpu(*crcp)) {
		printf("CRC mismatch (%08x != %08x)\n", crc,
			be32_to_cpu(e.crc));
		return 0;
	}

	/*
	 * MAC address #9 in v1 occupies the same position as the CRC in v0.
	 * Erase it so that it's not mistaken for a MAC address.  We'll
	 * update the CRC later.
	 */
	if (e.version == 0)
		memset(e.mac[8], 0xff, 6);

	for (i = 0; i < min(e.mac_count, (u8)MAX_NUM_PORTS); i++) {
		if (memcmp(&e.mac[i], "\0\0\0\0\0\0", 6) &&
		    memcmp(&e.mac[i], "\xFF\xFF\xFF\xFF\xFF\xFF", 6)) {
			char ethaddr[18];
			char enetvar[9];

			sprintf(ethaddr, "%02X:%02X:%02X:%02X:%02X:%02X",
				e.mac[i][0],
				e.mac[i][1],
				e.mac[i][2],
				e.mac[i][3],
				e.mac[i][4],
				e.mac[i][5]);
			sprintf(enetvar, i ? "eth%daddr" : "ethaddr", i);
			/* Only initialize environment variables that are blank
			 * (i.e. have not yet been set)
			 */
			if (!env_get(enetvar))
				env_set(enetvar, ethaddr);
		}
	}

	printf("%c%c%c%c v%u\n", e.id[0], e.id[1], e.id[2], e.id[3],
		be32_to_cpu(e.version));

	return 0;
}
