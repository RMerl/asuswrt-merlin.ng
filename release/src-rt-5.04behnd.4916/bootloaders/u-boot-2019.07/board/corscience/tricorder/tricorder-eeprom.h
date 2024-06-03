/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2013
 * Corscience GmbH & Co. KG, <www.corscience.de>
 * Andreas Bießmann <andreas.biessmann@corscience.de>
 */
#ifndef TRICORDER_EEPROM_H_
#define TRICORDER_EEPROM_H_

#include <linux/compiler.h>

#define TRICORDER_EEPROM_MAGIC 0xc2a94f52
#define TRICORDER_EEPROM_VERSION 1

#define TRICORDER_BOARD_NAME_LENGTH		12
#define TRICORDER_BOARD_VERSION_LENGTH		4
#define TRICORDER_BOARD_SERIAL_LENGTH		12
#define TRICORDER_INTERFACE_VERSION_LENGTH	4

struct tricorder_eeprom {
	uint32_t magic;
	uint16_t length;
	uint16_t version;
	char board_name[TRICORDER_BOARD_NAME_LENGTH];
	char board_version[TRICORDER_BOARD_VERSION_LENGTH];
	char board_serial[TRICORDER_BOARD_SERIAL_LENGTH];
	char interface_version[TRICORDER_INTERFACE_VERSION_LENGTH];
	uint32_t crc32;
} __packed;

#define TRICORDER_EEPROM_SIZE		sizeof(struct tricorder_eeprom)
#define TRICORDER_EEPROM_CRC_SIZE	(TRICORDER_EEPROM_SIZE - \
					 sizeof(uint32_t))

/**
 * @brief read eeprom information from a specific eeprom address
 */
int tricorder_get_eeprom(int addr, struct tricorder_eeprom *eeprom);

#endif /* TRICORDER_EEPROM_H_ */
