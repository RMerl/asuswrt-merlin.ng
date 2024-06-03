// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2006 Freescale Semiconductor
 * York Sun (yorksun@freescale.com)
 */

#include <common.h>
#include <command.h>

extern int do_mac(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);

U_BOOT_CMD(
	mac, 3, 1,  do_mac,
	"display and program the system ID and MAC addresses in EEPROM",
	"[read|save|id|num|errata|date|ports|port_number]\n"
	"mac read\n"
	"    - read EEPROM content into memory data structure\n"
	"mac save\n"
	"    - save memory data structure to the EEPROM\n"
	"mac id\n"
	"    - program system id per hard coded value\n"
	"mac num string\n"
	"    - program system serial number to value string\n"
	"mac errata string\n"
	"    - program errata data to value string\n"
	"mac date YYMMDDhhmmss\n"
	"    - program date to string value YYMMDDhhmmss\n"
	"mac ports N\n"
	"    - program the number of network ports to integer N\n"
	"mac X string\n"
	"    - program MAC addr for port X [X=0,1..] to colon separated string"
);
