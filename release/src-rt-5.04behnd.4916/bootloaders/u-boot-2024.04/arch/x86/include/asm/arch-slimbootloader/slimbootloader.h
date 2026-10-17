/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2019 Intel Corporation <www.intel.com>
 */

#ifndef __SLIMBOOTLOADER_ARCH_H__
#define __SLIMBOOTLOADER_ARCH_H__

#include <asm/hob.h>

/**
 * A GUID to get MemoryMap info hob which is provided by Slim Bootloader
 */
#define SBL_MEMORY_MAP_INFO_GUID \
	EFI_GUID(0xa1ff7424, 0x7a1a, 0x478e, \
		0xa9, 0xe4, 0x92, 0xf3, 0x57, 0xd1, 0x28, 0x32)

/**
 * A GUID to get SerialPort info hob which is provided by Slim Bootloader
 */
#define SBL_SERIAL_PORT_INFO_GUID \
	EFI_GUID(0x6c6872fe, 0x56a9, 0x4403, \
		0xbb, 0x98, 0x95, 0x8d, 0x62, 0xde, 0x87, 0xf1)

/**
 * A GUID to get boot performance info hob which is provided by Slim Bootloader
 */
#define SBL_PERFORMANCE_INFO_GUID \
	EFI_GUID(0x868204be, 0x23d0, 0x4ff9, \
		0xac, 0x34, 0xb9, 0x95, 0xac, 0x04, 0xb1, 0xb9)

/**
 * A single entry of memory map information
 *
 * @addr: start address of a memory map entry
 * @size: size of a memory map entry
 * @type: usable:1, reserved:2, acpi:3, nvs:4, unusable:5
 * @flag: only used in Slim Bootloader
 * @rsvd: padding for alignment
 */
struct sbl_memory_map_entry {
	u64	addr;
	u64	size;
	u8	type;
	u8	flag;
	u8	rsvd[6];
};

/**
 * This includes all memory map entries which is sorted based on physical start
 * address, from low to high, and carved out reserved, acpi nvs, acpi reclaim
 * and usable memory.
 *
 * @rev  : revision of memory_map_info structure. currently 1.
 * @rsvd : padding for alignment
 * @count: the number of memory map entries
 * @entry: array of all memory map entries
 */
struct sbl_memory_map_info {
	u8	rev;
	u8	rsvd[3];
	u32	count;
	struct sbl_memory_map_entry	entry[0];
};

/**
 * This includes serial port info which has already been initialized in previous
 * Slim Bootloader stage.
 * The Slim Bootloader initializes serial port regardless of debug/release build
 * modes, and it passes the information to a payload thru hob. So, a payload can
 * re-use the serial information without re-initializing serial port.
 *
 * @rev   : revision of serial_port_info structure. currently 1.
 * @rsvd  : padding for alignment
 * @type  : port io: 1, mmio: 2
 * @base  : io base address. ex) 0x3f8, 0x80001000
 * @baud  : uart baud rate
 * @stride: register stride in Bytes
 * @clk   : uart frequency in Hz
 * @rsvd1 : reserved
 */
struct sbl_serial_port_info {
	u8      rev;
	u8      rsvd[3];
	u32     type;
	u32     base;
	u32     baud;
	u32     stride;
	u32     clk;
	u32     rsvd1;
};

/**
 * This includes timestamp data which has been collected in Slim Bootloader
 * stages from the reset vector. In addition, this has TSC frequency in KHz to
 * calculate each timestamp.
 *
 * @rev   : revision of performance_info structure. currently 1.
 * @rsvd  : padding for alignment
 * @count : the number of collected timestamp data
 * @flags : only used in Slim Bootloader
 * @frequency: tsc frequency in KHz
 * @timestamp: the array of timestamp data which has 64-bit tsc value
 */
struct sbl_performance_info {
	u8      rev;
	u8      rsvd[3];
	u16     count;
	u16     flags;
	u32     frequency;
	u64     timestamp[0];
};

#endif /* __SLIMBOOTLOADER_ARCH_H__ */
