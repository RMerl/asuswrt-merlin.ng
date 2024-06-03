// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2013
 * Corscience GmbH & Co. KG, <www.corscience.de>
 * Andreas Bießmann <andreas.biessmann@corscience.de>
 */
#include <common.h>
#include <i2c.h>

#include "tricorder-eeprom.h"

static inline void warn_wrong_value(const char *msg, unsigned int a,
		unsigned int b)
{
	printf("Expected EEPROM %s %08x, got %08x\n", msg, a, b);
}

static int handle_eeprom_v0(struct tricorder_eeprom *eeprom)
{
	struct tricorder_eeprom_v0 {
		uint32_t magic;
		uint16_t length;
		uint16_t version;
		char board_name[TRICORDER_BOARD_NAME_LENGTH];
		char board_version[TRICORDER_BOARD_VERSION_LENGTH];
		char board_serial[TRICORDER_BOARD_SERIAL_LENGTH];
		uint32_t crc32;
	} __packed eepromv0;
	uint32_t crc;

	printf("Old EEPROM (v0), consider rewrite!\n");

	if (be16_to_cpu(eeprom->length) != sizeof(eepromv0)) {
		warn_wrong_value("length", sizeof(eepromv0),
				 be16_to_cpu(eeprom->length));
		return 1;
	}

	memcpy(&eepromv0, eeprom, sizeof(eepromv0));

	crc = crc32(0L, (unsigned char *)&eepromv0,
		    sizeof(eepromv0) - sizeof(eepromv0.crc32));
	if (be32_to_cpu(eepromv0.crc32) != crc) {
		warn_wrong_value("CRC", be32_to_cpu(eepromv0.crc32),
				 crc);
		return 1;
	}

	/* Ok the content is correct, do the conversion */
	memset(eeprom->interface_version, 0x0,
	       TRICORDER_INTERFACE_VERSION_LENGTH);
	crc = crc32(0L, (unsigned char *)eeprom, TRICORDER_EEPROM_CRC_SIZE);
	eeprom->crc32 = cpu_to_be32(crc);

	return 0;
}

static int handle_eeprom_v1(struct tricorder_eeprom *eeprom)
{
	uint32_t crc;

	if (be16_to_cpu(eeprom->length) != TRICORDER_EEPROM_SIZE) {
		warn_wrong_value("length", TRICORDER_EEPROM_SIZE,
				 be16_to_cpu(eeprom->length));
		return 1;
	}

	crc = crc32(0L, (unsigned char *)eeprom, TRICORDER_EEPROM_CRC_SIZE);
	if (be32_to_cpu(eeprom->crc32) != crc) {
		warn_wrong_value("CRC", be32_to_cpu(eeprom->crc32), crc);
		return 1;
	}

	return 0;
}

int tricorder_get_eeprom(int addr, struct tricorder_eeprom *eeprom)
{
	unsigned int bus = i2c_get_bus_num();
	i2c_set_bus_num(CONFIG_SYS_EEPROM_BUS_NUM);

	memset(eeprom, 0, TRICORDER_EEPROM_SIZE);

	i2c_read(addr, 0, 2, (unsigned char *)eeprom, TRICORDER_EEPROM_SIZE);
	i2c_set_bus_num(bus);

	if (be32_to_cpu(eeprom->magic) != TRICORDER_EEPROM_MAGIC) {
		warn_wrong_value("magic", TRICORDER_EEPROM_MAGIC,
				 be32_to_cpu(eeprom->magic));
		return 1;
	}

	switch (be16_to_cpu(eeprom->version)) {
	case 0:
		return handle_eeprom_v0(eeprom);
	case 1:
		return handle_eeprom_v1(eeprom);
	default:
		warn_wrong_value("version", TRICORDER_EEPROM_VERSION,
				 be16_to_cpu(eeprom->version));
		return 1;
	}
}

#if !defined(CONFIG_SPL)
int tricorder_eeprom_read(unsigned devaddr)
{
	struct tricorder_eeprom eeprom;
	int ret = tricorder_get_eeprom(devaddr, &eeprom);

	if (ret)
		return ret;

	printf("Board type:               %.*s\n",
	       sizeof(eeprom.board_name), eeprom.board_name);
	printf("Board version:            %.*s\n",
	       sizeof(eeprom.board_version), eeprom.board_version);
	printf("Board serial:             %.*s\n",
	       sizeof(eeprom.board_serial), eeprom.board_serial);
	printf("Board interface version:  %.*s\n",
	       sizeof(eeprom.interface_version),
	       eeprom.interface_version);

	return ret;
}

int tricorder_eeprom_write(unsigned devaddr, const char *name,
		const char *version, const char *serial, const char *interface)
{
	struct tricorder_eeprom eeprom, eeprom_verify;
	size_t length;
	uint32_t crc;
	int ret;
	unsigned char *p;
	int i;

	memset(eeprom, 0, TRICORDER_EEPROM_SIZE);
	memset(eeprom_verify, 0, TRICORDER_EEPROM_SIZE);

	eeprom.magic = cpu_to_be32(TRICORDER_EEPROM_MAGIC);
	eeprom.length = cpu_to_be16(TRICORDER_EEPROM_SIZE);
	eeprom.version = cpu_to_be16(TRICORDER_EEPROM_VERSION);

	length = min(sizeof(eeprom.board_name), strlen(name));
	strncpy(eeprom.board_name, name, length);

	length = min(sizeof(eeprom.board_version), strlen(version));
	strncpy(eeprom.board_version, version, length);

	length = min(sizeof(eeprom.board_serial), strlen(serial));
	strncpy(eeprom.board_serial, serial, length);

	if (interface) {
		length = min(sizeof(eeprom.interface_version),
				strlen(interface));
		strncpy(eeprom.interface_version, interface, length);
	}

	crc = crc32(0L, (unsigned char *)&eeprom, TRICORDER_EEPROM_CRC_SIZE);
	eeprom.crc32 = cpu_to_be32(crc);

#if defined(DEBUG)
	puts("Tricorder EEPROM content:\n");
	print_buffer(0, &eeprom, 1, sizeof(eeprom), 16);
#endif

	eeprom_init(CONFIG_SYS_EEPROM_BUS_NUM);

	ret = eeprom_write(devaddr, 0, (unsigned char *)&eeprom,
			TRICORDER_EEPROM_SIZE);
	if (ret)
		printf("Tricorder: Could not write EEPROM content!\n");

	ret = eeprom_read(devaddr, 0, (unsigned char *)&eeprom_verify,
			TRICORDER_EEPROM_SIZE);
	if (ret)
		printf("Tricorder: Could not read EEPROM content!\n");

	if (memcmp(&eeprom, &eeprom_verify, sizeof(eeprom)) != 0) {
		printf("Tricorder: Could not verify EEPROM content!\n");
		ret = 1;
	}

	return ret;
}

int do_tricorder_eeprom(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	if (argc == 3) {
		ulong dev_addr = simple_strtoul(argv[2], NULL, 16);

		if (strcmp(argv[1], "read") == 0)
			return tricorder_eeprom_read(dev_addr);
	} else if (argc == 6 || argc == 7) {
		ulong dev_addr = simple_strtoul(argv[2], NULL, 16);
		char *name = argv[3];
		char *version = argv[4];
		char *serial = argv[5];
		char *interface = NULL;

		if (argc == 7)
			interface = argv[6];

		if (strcmp(argv[1], "write") == 0)
			return tricorder_eeprom_write(dev_addr, name, version,
						      serial, interface);
	}

	return CMD_RET_USAGE;
}

U_BOOT_CMD(
	tricordereeprom,	7,	1,	do_tricorder_eeprom,
	"Tricorder EEPROM",
	"read  devaddr\n"
	"       - read Tricorder EEPROM at devaddr and print content\n"
	"tricordereeprom write devaddr name version serial [interface]\n"
	"       - write Tricorder EEPROM at devaddr with 'name', 'version'"
	"and 'serial'\n"
	"         optional add an HW interface parameter"
);
#endif /* CONFIG_SPL */
