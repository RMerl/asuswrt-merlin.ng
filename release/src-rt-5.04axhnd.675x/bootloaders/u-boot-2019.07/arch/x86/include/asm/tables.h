/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2015, Bin Meng <bmeng.cn@gmail.com>
 */

#ifndef _X86_TABLES_H_
#define _X86_TABLES_H_

#include <tables_csum.h>

#define ROM_TABLE_ADDR	CONFIG_ROM_TABLE_ADDR
#define ROM_TABLE_END	(CONFIG_ROM_TABLE_ADDR + CONFIG_ROM_TABLE_SIZE - 1)

#define ROM_TABLE_ALIGN	1024

/* SeaBIOS expects coreboot tables at address range 0x0000-0x1000 */
#define CB_TABLE_ADDR	0x800

/**
 * table_compute_checksum() - Compute a table checksum
 *
 * This computes an 8-bit checksum for the configuration table.
 * All bytes in the configuration table, including checksum itself and
 * reserved bytes must add up to zero.
 *
 * @v:		configuration table base address
 * @len:	configuration table size
 * @return:	the 8-bit checksum
 */
u8 table_compute_checksum(void *v, int len);

/**
 * table_fill_string() - Fill a string with pad in the configuration table
 *
 * This fills a string in the configuration table. It copies number of bytes
 * from the source string, and if source string length is shorter than the
 * required size to copy, pad the table string with the given pad character.
 *
 * @dest:	where to fill a string
 * @src:	where to copy from
 * @n:		number of bytes to copy
 * @pad:	character to pad the remaining bytes
 */
void table_fill_string(char *dest, const char *src, size_t n, char pad);

/**
 * write_tables() - Write x86 configuration tables
 *
 * This writes x86 configuration tables, including PIRQ routing table,
 * Multi-Processor table and ACPI table. Whether a specific type of
 * configuration table is written is controlled by a Kconfig option.
 */
void write_tables(void);

/**
 * write_pirq_routing_table() - Write PIRQ routing table
 *
 * This writes PIRQ routing table at a given address.
 *
 * @start:	start address to write PIRQ routing table
 * @return:	end address of PIRQ routing table
 */
ulong write_pirq_routing_table(ulong start);

#endif /* _X86_TABLES_H_ */
