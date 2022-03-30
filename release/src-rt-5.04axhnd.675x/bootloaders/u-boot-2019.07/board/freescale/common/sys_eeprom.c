// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2006, 2008-2009, 2011 Freescale Semiconductor
 * York Sun (yorksun@freescale.com)
 * Haiying Wang (haiying.wang@freescale.com)
 * Timur Tabi (timur@freescale.com)
 */

#include <common.h>
#include <command.h>
#include <i2c.h>
#include <linux/ctype.h>

#ifdef CONFIG_SYS_I2C_EEPROM_CCID
#include "../common/eeprom.h"
#define MAX_NUM_PORTS	8
#endif

#ifdef CONFIG_SYS_I2C_EEPROM_NXID
/* some boards with non-256-bytes EEPROM have special define */
/* for MAX_NUM_PORTS in board-specific file */
#ifndef MAX_NUM_PORTS
#define MAX_NUM_PORTS	16
#endif
#define NXID_VERSION	1
#endif

/**
 * static eeprom: EEPROM layout for CCID or NXID formats
 *
 * See application note AN3638 for details.
 */
static struct __attribute__ ((__packed__)) eeprom {
#ifdef CONFIG_SYS_I2C_EEPROM_CCID
	u8 id[4];         /* 0x00 - 0x03 EEPROM Tag 'CCID' */
	u8 major;         /* 0x04        Board revision, major */
	u8 minor;         /* 0x05        Board revision, minor */
	u8 sn[10];        /* 0x06 - 0x0F Serial Number*/
	u8 errata[2];     /* 0x10 - 0x11 Errata Level */
	u8 date[6];       /* 0x12 - 0x17 Build Date */
	u8 res_0[40];     /* 0x18 - 0x3f Reserved */
	u8 mac_count;     /* 0x40        Number of MAC addresses */
	u8 mac_flag;      /* 0x41        MAC table flags */
	u8 mac[MAX_NUM_PORTS][6];     /* 0x42 - 0x71 MAC addresses */
	u32 crc;          /* 0x72        CRC32 checksum */
#endif
#ifdef CONFIG_SYS_I2C_EEPROM_NXID
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
	u8 mac[MAX_NUM_PORTS][6];     /* 0x42 - 0xa1 MAC addresses */
	u8 res_2[90];     /* 0xa2 - 0xfb Reserved */	
	u32 crc;          /* 0xfc - 0xff CRC32 checksum */
#endif
} e;

/* Set to 1 if we've read EEPROM into memory */
static int has_been_read = 0;

#ifdef CONFIG_SYS_I2C_EEPROM_NXID
/* Is this a valid NXID EEPROM? */
#define is_valid ((e.id[0] == 'N') || (e.id[1] == 'X') || \
		  (e.id[2] == 'I') || (e.id[3] == 'D'))
#endif

#ifdef CONFIG_SYS_I2C_EEPROM_CCID
/* Is this a valid CCID EEPROM? */
#define is_valid ((e.id[0] == 'C') || (e.id[1] == 'C') || \
		  (e.id[2] == 'I') || (e.id[3] == 'D'))
#endif

/**
 * show_eeprom - display the contents of the EEPROM
 */
static void show_eeprom(void)
{
	int i;
	unsigned int crc;

	/* EEPROM tag ID, either CCID or NXID */
#ifdef CONFIG_SYS_I2C_EEPROM_NXID
	printf("ID: %c%c%c%c v%u\n", e.id[0], e.id[1], e.id[2], e.id[3],
	       be32_to_cpu(e.version));
#else
	printf("ID: %c%c%c%c\n", e.id[0], e.id[1], e.id[2], e.id[3]);
#endif

	/* Serial number */
	printf("SN: %s\n", e.sn);

	/* Errata level. */
#ifdef CONFIG_SYS_I2C_EEPROM_NXID
	printf("Errata: %s\n", e.errata);
#else
	printf("Errata: %c%c\n",
		e.errata[0] ? e.errata[0] : '.',
		e.errata[1] ? e.errata[1] : '.');
#endif

	/* Build date, BCD date values, as YYMMDDhhmmss */
	printf("Build date: 20%02x/%02x/%02x %02x:%02x:%02x %s\n",
		e.date[0], e.date[1], e.date[2],
		e.date[3] & 0x7F, e.date[4], e.date[5],
		e.date[3] & 0x80 ? "PM" : "");

	/* Show MAC addresses  */
	for (i = 0; i < min(e.mac_count, (u8)MAX_NUM_PORTS); i++) {

		u8 *p = e.mac[i];

		printf("Eth%u: %02x:%02x:%02x:%02x:%02x:%02x\n", i,
			p[0], p[1], p[2], p[3],	p[4], p[5]);
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
static int read_eeprom(void)
{
	int ret;
#ifdef CONFIG_SYS_EEPROM_BUS_NUM
	unsigned int bus;
#endif

	if (has_been_read)
		return 0;

#ifdef CONFIG_SYS_EEPROM_BUS_NUM
	bus = i2c_get_bus_num();
	i2c_set_bus_num(CONFIG_SYS_EEPROM_BUS_NUM);
#endif

	ret = i2c_read(CONFIG_SYS_I2C_EEPROM_ADDR, 0, CONFIG_SYS_I2C_EEPROM_ADDR_LEN,
		(void *)&e, sizeof(e));

#ifdef CONFIG_SYS_EEPROM_BUS_NUM
	i2c_set_bus_num(bus);
#endif

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
	u32 crc;

	crc = crc32(0, (void *)&e, sizeof(e) - 4);
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
#ifdef CONFIG_SYS_EEPROM_BUS_NUM
	unsigned int bus;
#endif

	/* Set the reserved values to 0xFF   */
#ifdef CONFIG_SYS_I2C_EEPROM_NXID
	e.res_0 = 0xFF;
	memset(e.res_1, 0xFF, sizeof(e.res_1));
#else
	memset(e.res_0, 0xFF, sizeof(e.res_0));
#endif
	update_crc();

#ifdef CONFIG_SYS_EEPROM_BUS_NUM
	bus = i2c_get_bus_num();
	i2c_set_bus_num(CONFIG_SYS_EEPROM_BUS_NUM);
#endif

	/*
	 * The AT24C02 datasheet says that data can only be written in page
	 * mode, which means 8 bytes at a time, and it takes up to 5ms to
	 * complete a given write.
	 */
	for (i = 0, p = &e; i < sizeof(e); i += 8, p += 8) {
		ret = i2c_write(CONFIG_SYS_I2C_EEPROM_ADDR, i, CONFIG_SYS_I2C_EEPROM_ADDR_LEN,
				p, min((int)(sizeof(e) - i), 8));
		if (ret)
			break;
		udelay(5000);	/* 5ms write cycle timing */
	}

	if (!ret) {
		/* Verify the write by reading back the EEPROM and comparing */
		struct eeprom e2;

		ret = i2c_read(CONFIG_SYS_I2C_EEPROM_ADDR, 0,
			CONFIG_SYS_I2C_EEPROM_ADDR_LEN, (void *)&e2, sizeof(e2));
		if (!ret && memcmp(&e, &e2, sizeof(e)))
			ret = -1;
	}

#ifdef CONFIG_SYS_EEPROM_BUS_NUM
	i2c_set_bus_num(bus);
#endif

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
	char *p = (char *) string;
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
#ifdef CONFIG_SYS_I2C_EEPROM_NXID
		memcpy(e.id, "NXID", sizeof(e.id));
		e.version = cpu_to_be32(NXID_VERSION);
#else
		memcpy(e.id, "CCID", sizeof(e.id));
#endif
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
#ifdef CONFIG_SYS_I2C_EEPROM_NXID
		memset(e.errata, 0, 5);
		strncpy((char *)e.errata, argv[2], 4);
#else
		e.errata[0] = argv[2][0];
		e.errata[1] = argv[2][1];
#endif
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
int mac_read_from_eeprom(void)
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

#ifdef CONFIG_SYS_I2C_EEPROM_NXID
	/*
	 * If we've read an NXID v0 EEPROM, then we need to set the CRC offset
	 * to where it is in v0.
	 */
	if (e.version == 0)
		crc_offset = 0x72;
#endif

	crc = crc32(0, (void *)&e, crc_offset);
	crcp = (void *)&e + crc_offset;
	if (crc != be32_to_cpu(*crcp)) {
		printf("CRC mismatch (%08x != %08x)\n", crc, be32_to_cpu(e.crc));
		return 0;
	}

#ifdef CONFIG_SYS_I2C_EEPROM_NXID
	/*
	 * MAC address #9 in v1 occupies the same position as the CRC in v0.
	 * Erase it so that it's not mistaken for a MAC address.  We'll
	 * update the CRC later.
	 */
	if (e.version == 0)
		memset(e.mac[8], 0xff, 6);
#endif

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

#ifdef CONFIG_SYS_I2C_EEPROM_NXID
	printf("%c%c%c%c v%u\n", e.id[0], e.id[1], e.id[2], e.id[3],
	       be32_to_cpu(e.version));
#else
	printf("%c%c%c%c\n", e.id[0], e.id[1], e.id[2], e.id[3]);
#endif

#ifdef CONFIG_SYS_I2C_EEPROM_NXID
	/*
	 * Now we need to upconvert the data into v1 format.  We do this last so
	 * that at boot time, U-Boot will still say "NXID v0".
	 */
	if (e.version == 0) {
		e.version = cpu_to_be32(NXID_VERSION);
		update_crc();
	}
#endif

	return 0;
}

#ifdef CONFIG_SYS_I2C_EEPROM_CCID

/**
 * get_cpu_board_revision - get the CPU board revision on 85xx boards
 *
 * Read the EEPROM to determine the board revision.
 *
 * This function is called before relocation, so we need to read a private
 * copy of the EEPROM into a local variable on the stack.
 *
 * Also, we assume that CONFIG_SYS_EEPROM_BUS_NUM == CONFIG_SYS_SPD_BUS_NUM.  The global
 * variable i2c_bus_num must be compile-time initialized to CONFIG_SYS_SPD_BUS_NUM,
 * so that the SPD code will work.  This means that all pre-relocation I2C
 * operations can only occur on the CONFIG_SYS_SPD_BUS_NUM bus.  So if
 * CONFIG_SYS_EEPROM_BUS_NUM != CONFIG_SYS_SPD_BUS_NUM, then we can't read the EEPROM when
 * this function is called.  Oh well.
 */
unsigned int get_cpu_board_revision(void)
{
	struct board_eeprom {
		u32 id;           /* 0x00 - 0x03 EEPROM Tag 'CCID' */
		u8 major;         /* 0x04        Board revision, major */
		u8 minor;         /* 0x05        Board revision, minor */
	} be;

	i2c_read(CONFIG_SYS_I2C_EEPROM_ADDR, 0, CONFIG_SYS_I2C_EEPROM_ADDR_LEN,
		(void *)&be, sizeof(be));

	if (be.id != (('C' << 24) | ('C' << 16) | ('I' << 8) | 'D'))
		return MPC85XX_CPU_BOARD_REV(0, 0);

	if ((be.major == 0xff) && (be.minor == 0xff))
		return MPC85XX_CPU_BOARD_REV(0, 0);

	return MPC85XX_CPU_BOARD_REV(be.major, be.minor);
}
#endif
