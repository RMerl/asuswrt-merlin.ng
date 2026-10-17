// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Microchip Technology, Inc.
 *		      Eugen Hristev <eugen.hristev@microchip.com>
 */

#include <asm/arch/sama7g5.h>

char *get_cpu_name(void)
{
	unsigned int extension_id = get_extension_chip_id();

	if (cpu_is_sama7g5())
		switch (extension_id) {
		case ARCH_EXID_SAMA7G51:
			return "SAMA7G51";
		case ARCH_EXID_SAMA7G52:
			return "SAMA7G52";
		case ARCH_EXID_SAMA7G53:
			return "SAMA7G53";
		case ARCH_EXID_SAMA7G54:
			return "SAMA7G54";
		case ARCH_EXID_SAMA7G54_D1G:
			return "SAMA7G54 1Gb DDR3L SiP";
		case ARCH_EXID_SAMA7G54_D2G:
			return "SAMA7G54 2Gb DDR3L SiP";
		case ARCH_EXID_SAMA7G54_D4G:
			return "SAMA7G54 4Gb DDR3L SiP";
		default:
			return "Unknown CPU type";
		}
	else
		return "Unknown CPU type";
}
