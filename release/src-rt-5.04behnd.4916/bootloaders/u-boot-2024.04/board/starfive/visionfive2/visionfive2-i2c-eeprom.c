// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2023 StarFive Technology Co., Ltd.
 * Author: Yanhong Wang<yanhong.wang@starfivetech.com>
 */

#include <common.h>
#include <command.h>
#include <env.h>
#include <i2c.h>
#include <init.h>
#include <u-boot/crc.h>
#include <linux/delay.h>

#define FORMAT_VERSION		0x2
#define PCB_VERSION		0xB1
#define BOM_VERSION		'A'
/*
 * BYTES_PER_EEPROM_PAGE: the 24FC04H datasheet says that data can
 * only be written in page mode, which means 16 bytes at a time:
 * 16-Byte Page Write Buffer
 */
#define BYTES_PER_EEPROM_PAGE		16

/*
 * EEPROM_WRITE_DELAY_MS: the 24FC04H datasheet says it takes up to
 * 5ms to complete a given write:
 * Write Cycle Time (byte or page) ro Page Write Time 5 ms, Maximum
 */
#define EEPROM_WRITE_DELAY_MS		5000
/*
 * StarFive OUI. Registration Date is 20xx-xx-xx
 */
#define STARFIVE_OUI_PREFIX		"6C:CF:39:"
#define STARFIVE_DEFAULT_MAC0		"6C:CF:39:6C:DE:AD"
#define STARFIVE_DEFAULT_MAC1		"6C:CF:39:6C:DE:AE"

/* Magic number at the first four bytes of EEPROM HATs */
#define STARFIVE_EEPROM_HATS_SIG	"SFVF" /* StarFive VisionFive */

#define STARFIVE_EEPROM_HATS_SIZE_MAX	256 /* Header + Atom1&4(v1) */
#define STARFIVE_EEPROM_WP_OFFSET	0 /* Read only field */
#define STARFIVE_EEPROM_ATOM1_PSTR	"VF7110A1-2228-D008E000-00000001\0"
#define STARFIVE_EEPROM_ATOM1_PSTR_SIZE	32
#define STARFIVE_EEPROM_ATOM1_SN_OFFSET	23
#define STARFIVE_EEPROM_ATOM1_VSTR	"StarFive Technology Co., Ltd.\0\0\0"
#define STARFIVE_EEPROM_ATOM1_VSTR_SIZE	32

#define MAGIC_NUMBER_BYTES	4
#define MAC_ADDR_BYTES		6
#define MAC_ADDR_STRLEN		17

/*
 * Atom Types
 * 0x0000 = invalid
 * 0x0001 = vendor info
 * 0x0002 = GPIO map
 * 0x0003 = Linux device tree blob
 * 0x0004 = manufacturer custom data
 * 0x0005-0xfffe = reserved for future use
 * 0xffff = invalid
 */

#define HATS_ATOM_INVALID	0x0000
#define HATS_ATOM_VENDOR	0x0001
#define HATS_ATOM_GPIO		0x0002
#define HATS_ATOM_DTB		0x0003
#define HATS_ATOM_CUSTOM	0x0004
#define HATS_ATOM_INVALID_END	0xffff

struct eeprom_header {
	char signature[MAGIC_NUMBER_BYTES];	/* ASCII table signature */
	u8 version;		/* EEPROM data format version */
				/* (0x00 reserved, 0x01 = first version) */
	u8 reversed;		/* 0x00, Reserved field */
	u16 numatoms;		/* total atoms in EEPROM */
	u32 eeplen;		/* total length in bytes of all eeprom data */
				/* (including this header) */
};

struct eeprom_atom_header {
	u16 type;
	u16 count;
	u32 dlen;
};

struct eeprom_atom1_data {
	u8 uuid[16];
	u16 pid;
	u16 pver;
	u8 vslen;
	u8 pslen;
	uchar vstr[STARFIVE_EEPROM_ATOM1_VSTR_SIZE];
	uchar pstr[STARFIVE_EEPROM_ATOM1_PSTR_SIZE]; /* product SN */
};

struct starfive_eeprom_atom1 {
	struct eeprom_atom_header header;
	struct eeprom_atom1_data data;
	u16 crc;
};

struct eeprom_atom4_data {
	u16 version;
	u8 pcb_revision;		/* PCB version */
	u8 bom_revision;		/* BOM version */
	u8 mac0_addr[MAC_ADDR_BYTES];	/* Ethernet0 MAC */
	u8 mac1_addr[MAC_ADDR_BYTES];	/* Ethernet1 MAC */
	u8 reserved[2];
};

struct starfive_eeprom_atom4 {
	struct eeprom_atom_header header;
	struct eeprom_atom4_data data;
	u16 crc;
};

struct starfive_eeprom {
	struct eeprom_header header;
	struct starfive_eeprom_atom1 atom1;
	struct starfive_eeprom_atom4 atom4;
};

static union {
	struct starfive_eeprom eeprom;
	uchar buf[STARFIVE_EEPROM_HATS_SIZE_MAX];
} pbuf __section(".data");

/* Set to 1 if we've read EEPROM into memory */
static int has_been_read __section(".data");

static inline int is_match_magic(void)
{
	return strncmp(pbuf.eeprom.header.signature, STARFIVE_EEPROM_HATS_SIG,
				MAGIC_NUMBER_BYTES);
}

/* Calculate the current CRC */
static inline u32 calculate_crc16(struct eeprom_atom_header *head)
{
	uint len = sizeof(struct eeprom_atom_header) + head->dlen - sizeof(u16);

	return crc16(0, (void *)head, len);
}

/* This function should be called after each update to the EEPROM structure */
static inline void update_crc(void)
{
	pbuf.eeprom.atom1.crc = calculate_crc16(&pbuf.eeprom.atom1.header);
	pbuf.eeprom.atom4.crc = calculate_crc16(&pbuf.eeprom.atom4.header);
}

static void dump_raw_eeprom(void)
{
	unsigned int i;
	u32 len;

	len = sizeof(struct starfive_eeprom);
	for (i = 0; i < len; i++) {
		if ((i % 16) == 0)
			printf("%02X: ", i);
		printf("%02X ", ((u8 *)pbuf.buf)[i]);
		if (((i % 16) == 15) || (i == len - 1))
			printf("\n");
	}
}

/**
 * show_eeprom - display the contents of the EEPROM
 */
static void show_eeprom(void)
{
	if (has_been_read != 1)
		return;

	printf("\n--------EEPROM INFO--------\n");
	printf("Vendor : %s\n", pbuf.eeprom.atom1.data.vstr);
	printf("Product full SN: %s\n", pbuf.eeprom.atom1.data.pstr);
	printf("data version: 0x%x\n", pbuf.eeprom.atom4.data.version);
	if (pbuf.eeprom.atom4.data.version == 2) {
		printf("PCB revision: 0x%x\n", pbuf.eeprom.atom4.data.pcb_revision);
		printf("BOM revision: %c\n", pbuf.eeprom.atom4.data.bom_revision);
		printf("Ethernet MAC0 address: %02x:%02x:%02x:%02x:%02x:%02x\n",
		       pbuf.eeprom.atom4.data.mac0_addr[0], pbuf.eeprom.atom4.data.mac0_addr[1],
		       pbuf.eeprom.atom4.data.mac0_addr[2], pbuf.eeprom.atom4.data.mac0_addr[3],
		       pbuf.eeprom.atom4.data.mac0_addr[4], pbuf.eeprom.atom4.data.mac0_addr[5]);
		printf("Ethernet MAC1 address: %02x:%02x:%02x:%02x:%02x:%02x\n",
		       pbuf.eeprom.atom4.data.mac1_addr[0], pbuf.eeprom.atom4.data.mac1_addr[1],
		       pbuf.eeprom.atom4.data.mac1_addr[2], pbuf.eeprom.atom4.data.mac1_addr[3],
		       pbuf.eeprom.atom4.data.mac1_addr[4], pbuf.eeprom.atom4.data.mac1_addr[5]);
	} else {
		printf("Custom data v%d is not Supported\n", pbuf.eeprom.atom4.data.version);
		dump_raw_eeprom();
	}
	printf("--------EEPROM INFO--------\n\n");
}

/**
 * set_mac_address() - stores a MAC address into the local EEPROM copy
 *
 * This function takes a pointer to MAC address string
 * (i.e."XX:XX:XX:XX:XX:XX", where "XX" is a two-digit hex number),
 * stores it in the MAC address field of the EEPROM local copy, and
 * updates the local copy of the CRC.
 */
static void set_mac_address(char *string, int index)
{
	u8 i;
	u8 *mac;

	if (strncasecmp(STARFIVE_OUI_PREFIX, string,
			strlen(STARFIVE_OUI_PREFIX))) {
		printf("The MAC address doesn't match StarFive OUI %s\n",
		       STARFIVE_OUI_PREFIX);
		return;
	}
	mac = (index == 0) ? pbuf.eeprom.atom4.data.mac0_addr :
			pbuf.eeprom.atom4.data.mac1_addr;

	for (i = 0; *string && (i < MAC_ADDR_BYTES); i++) {
		mac[i] = hextoul(string, &string);

		if (*string == ':')
			string++;
	}

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
	memset((void *)pbuf.buf, 0, sizeof(struct starfive_eeprom));
	memcpy(pbuf.eeprom.header.signature, STARFIVE_EEPROM_HATS_SIG,
	       strlen(STARFIVE_EEPROM_HATS_SIG));
	pbuf.eeprom.header.version = FORMAT_VERSION;
	pbuf.eeprom.header.numatoms = 2;
	pbuf.eeprom.header.eeplen = sizeof(struct starfive_eeprom);

	pbuf.eeprom.atom1.header.type = HATS_ATOM_VENDOR;
	pbuf.eeprom.atom1.header.count = 1;
	pbuf.eeprom.atom1.header.dlen = sizeof(struct eeprom_atom1_data) + sizeof(u16);
	pbuf.eeprom.atom1.data.vslen = STARFIVE_EEPROM_ATOM1_VSTR_SIZE;
	pbuf.eeprom.atom1.data.pslen = STARFIVE_EEPROM_ATOM1_PSTR_SIZE;
	memcpy(pbuf.eeprom.atom1.data.vstr, STARFIVE_EEPROM_ATOM1_VSTR,
	       strlen(STARFIVE_EEPROM_ATOM1_VSTR));
	memcpy(pbuf.eeprom.atom1.data.pstr, STARFIVE_EEPROM_ATOM1_PSTR,
	       strlen(STARFIVE_EEPROM_ATOM1_PSTR));

	pbuf.eeprom.atom4.header.type = HATS_ATOM_CUSTOM;
	pbuf.eeprom.atom4.header.count = 2;
	pbuf.eeprom.atom4.header.dlen = sizeof(struct eeprom_atom4_data) + sizeof(u16);
	pbuf.eeprom.atom4.data.version = FORMAT_VERSION;
	pbuf.eeprom.atom4.data.pcb_revision = PCB_VERSION;
	pbuf.eeprom.atom4.data.bom_revision = BOM_VERSION;
	set_mac_address(STARFIVE_DEFAULT_MAC0, 0);
	set_mac_address(STARFIVE_DEFAULT_MAC1, 1);
}

/**
 * prog_eeprom() - write the EEPROM from memory
 */
static int prog_eeprom(unsigned int size)
{
	unsigned int i;
	void *p;
	uchar tmp_buff[STARFIVE_EEPROM_HATS_SIZE_MAX];
	struct udevice *dev;
	int ret;

	if (is_match_magic()) {
		printf("MAGIC ERROR, Please check the data@%p.\n", pbuf.buf);
		return -1;
	}

	ret = i2c_get_chip_for_busnum(CONFIG_SYS_EEPROM_BUS_NUM,
				      CONFIG_SYS_I2C_EEPROM_ADDR,
				      CONFIG_SYS_I2C_EEPROM_ADDR_LEN,
				      &dev);
	if (ret) {
		printf("Get i2c bus:%d addr:%d fail.\n", CONFIG_SYS_EEPROM_BUS_NUM,
		       CONFIG_SYS_I2C_EEPROM_ADDR);
		return ret;
	}

	for (i = 0, p = (u8 *)pbuf.buf; i < size; ) {
		if (!ret)
			ret = dm_i2c_write(dev, i, p, min((int)(size - i),
							  BYTES_PER_EEPROM_PAGE));
		if (ret)
			break;

		udelay(EEPROM_WRITE_DELAY_MS);
		i += BYTES_PER_EEPROM_PAGE;
		p += BYTES_PER_EEPROM_PAGE;
	}

	if (!ret) {
		/* Verify the write by reading back the EEPROM and comparing */
		ret = dm_i2c_read(dev,
				  STARFIVE_EEPROM_WP_OFFSET,
				  tmp_buff,
				  STARFIVE_EEPROM_HATS_SIZE_MAX);
		if (!ret && memcmp((void *)pbuf.buf, (void *)tmp_buff,
				   STARFIVE_EEPROM_HATS_SIZE_MAX))
			ret = -1;
	}

	if (ret) {
		has_been_read = -1;
		printf("Programming failed.\n");
		return -1;
	}

	printf("Programming passed.\n");
	return 0;
}

/**
 * read_eeprom() - read the EEPROM into memory, if it hasn't been read already
 */
static int read_eeprom(void)
{
	int ret;
	struct udevice *dev;

	if (has_been_read == 1)
		return 0;

	ret = i2c_get_chip_for_busnum(CONFIG_SYS_EEPROM_BUS_NUM,
				      CONFIG_SYS_I2C_EEPROM_ADDR, 1, &dev);
	if (!ret)
		ret = dm_i2c_read(dev, 0, (u8 *)pbuf.buf,
				  STARFIVE_EEPROM_HATS_SIZE_MAX);

	has_been_read = (ret == 0) ? 1 : 0;

	return ret;
}

/**
 * set_pcb_revision() - stores a StarFive PCB revision into the local EEPROM copy
 *
 * Takes a pointer to a string representing the numeric PCB revision in
 * decimal ("0" - "255"), stores it in the pcb_revision field of the
 * EEPROM local copy, and updates the CRC of the local copy.
 */
static void set_pcb_revision(char *string)
{
	u32 p;

	p = simple_strtoul(string, &string, 16);
	if (p > U8_MAX) {
		printf("%s must not be greater than %d\n", "PCB revision",
		       U8_MAX);
		return;
	}

	pbuf.eeprom.atom4.data.pcb_revision = p;

	update_crc();
}

/**
 * set_bom_revision() - stores a StarFive BOM revision into the local EEPROM copy
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

	pbuf.eeprom.atom4.data.bom_revision = string[0];

	update_crc();
}

/**
 * set_product_id() - stores a StarFive product ID into the local EEPROM copy
 *
 * Takes a pointer to a string representing the numeric product ID  in
 * string ("VF7100A1-2150-D008E000-00000001\0"), stores it in the product string
 * field of the EEPROM local copy, and updates the CRC of the local copy.
 */
static void set_product_id(char *string)
{
	u32 len;

	len = (strlen(string) > STARFIVE_EEPROM_ATOM1_PSTR_SIZE) ?
		STARFIVE_EEPROM_ATOM1_PSTR_SIZE : strlen(string);

	memcpy((void *)pbuf.eeprom.atom1.data.pstr, (void *)string, len);

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
		has_been_read = 0;
		return read_eeprom();
	} else if (!strcmp(cmd, "initialize")) {
		init_local_copy();
		return 0;
	} else if (!strcmp(cmd, "write_eeprom")) {
		return prog_eeprom(STARFIVE_EEPROM_HATS_SIZE_MAX);
	} else if (!strcmp(cmd, "raw")) {
		dump_raw_eeprom();
		return 0;
	}

	if (argc != 3)
		return CMD_RET_USAGE;

	if (is_match_magic()) {
		printf("Please read the EEPROM ('read_eeprom') and/or initialize the EEPROM ('initialize') first.\n");
		return 0;
	}

	if (!strcmp(cmd, "mac0_address")) {
		set_mac_address(argv[2], 0);
		return 0;
	} else if (!strcmp(cmd, "mac1_address")) {
		set_mac_address(argv[2], 1);
		return 0;
	} else if (!strcmp(cmd, "pcb_revision")) {
		set_pcb_revision(argv[2]);
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
 * mac_read_from_eeprom() - read the MAC address & the serial number in EEPROM
 *
 * This function reads the MAC address and the serial number from EEPROM and
 * sets the appropriate environment variables for each one read.
 *
 * The environment variables are only set if they haven't been set already.
 * This ensures that any user-saved variables are never overwritten.
 *
 * If CONFIG_ID_EEPROM is enabled, this function will be called in
 * "static init_fnc_t init_sequence_r[]" of u-boot/common/board_r.c.
 */
int mac_read_from_eeprom(void)
{
	/**
	 * try to fill the buff from EEPROM,
	 * always return SUCCESS, even some error happens.
	 */
	if (read_eeprom()) {
		dump_raw_eeprom();
		return 0;
	}

	// 1, setup ethaddr env
	eth_env_set_enetaddr("ethaddr", pbuf.eeprom.atom4.data.mac0_addr);
	eth_env_set_enetaddr("eth1addr", pbuf.eeprom.atom4.data.mac1_addr);

	/**
	 * 2, setup serial# env, reference to hifive-platform-i2c-eeprom.c,
	 * serial# can be a ASCII string, but not just a hex number, so we
	 * setup serial# in the 32Byte format:
	 * "VF7100A1-2201-D008E000-00000001;"
	 * "<product>-<date>-<DDR&eMMC>-<serial_number>"
	 * <date>: 4Byte, should be the output of `date +%y%W`
	 * <DDR&eMMC>: 8Byte, "D008" means 8GB, "D01T" means 1TB;
	 *     "E000" means no eMMC，"E032" means 32GB, "E01T" means 1TB.
	 * <serial_number>: 8Byte, the Unique Identifier of board in hex.
	 */
	if (!env_get("serial#"))
		env_set("serial#", pbuf.eeprom.atom1.data.pstr);

	printf("StarFive EEPROM format v%u\n", pbuf.eeprom.header.version);
	show_eeprom();
	return 0;
}

/**
 * get_pcb_revision_from_eeprom - get the PCB revision
 *
 * 1.2A return 'A'/'a', 1.3B return 'B'/'b',other values are illegal
 */
u8 get_pcb_revision_from_eeprom(void)
{
	u8 pv = 0xFF;

	if (read_eeprom())
		return pv;

	return pbuf.eeprom.atom1.data.pstr[6];
}

/**
 * get_ddr_size_from_eeprom - get the DDR size
 * pstr:  VF7110A1-2228-D008E000-00000001
 * VF7110A1/VF7110B1 : VisionFive JH7110A /VisionFive JH7110B
 * D008:　8GB LPDDR4
 * E000: No emmc device, ECxx: include emmc device, xx: Capacity size[GB]
 * return: the field of 'D008E000'
 */

u32 get_ddr_size_from_eeprom(void)
{
	u32 pv = 0xFFFFFFFF;

	if (read_eeprom())
		return pv;

	return hextoul(&pbuf.eeprom.atom1.data.pstr[14], NULL);
}

U_BOOT_LONGHELP(mac,
	"\n"
	"    - display EEPROM content\n"
	"mac read_eeprom\n"
	"    - read EEPROM content into memory data structure\n"
	"mac write_eeprom\n"
	"    - save memory data structure to the EEPROM\n"
	"mac initialize\n"
	"    - initialize the in-memory EEPROM copy with default data\n"
	"mac raw\n"
	"    - hexdump memory data structure\n"
	"mac mac0_address <xx:xx:xx:xx:xx:xx>\n"
	"    - stores a MAC0 address into the local EEPROM copy\n"
	"mac mac1_address <xx:xx:xx:xx:xx:xx>\n"
	"    - stores a MAC1 address into the local EEPROM copy\n"
	"mac pcb_revision <?>\n"
	"    - stores a StarFive PCB revision into the local EEPROM copy\n"
	"mac bom_revision <A>\n"
	"    - stores a StarFive BOM revision into the local EEPROM copy\n"
	"mac product_id <VF7110A1-2228-D008E000-xxxxxxxx>\n"
	"    - stores a StarFive product ID into the local EEPROM copy\n");

U_BOOT_CMD(
	mac, 3, 1,  do_mac,
	"display and program the board revision and MAC address in EEPROM",
	mac_help_text);
