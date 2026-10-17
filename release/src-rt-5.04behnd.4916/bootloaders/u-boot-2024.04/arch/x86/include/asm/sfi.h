/* SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause */
/*
 * Copyright(c) 2009 Intel Corporation. All rights reserved.
 */

#ifndef _LINUX_SFI_H
#define _LINUX_SFI_H

#include <errno.h>
#include <linux/types.h>

/* Table signatures reserved by the SFI specification */
#define SFI_SIG_SYST		"SYST"
#define SFI_SIG_FREQ		"FREQ"
#define SFI_SIG_CPUS		"CPUS"
#define SFI_SIG_MTMR		"MTMR"
#define SFI_SIG_MRTC		"MRTC"
#define SFI_SIG_MMAP		"MMAP"
#define SFI_SIG_APIC		"APIC"
#define SFI_SIG_XSDT		"XSDT"
#define SFI_SIG_WAKE		"WAKE"
#define SFI_SIG_DEVS		"DEVS"
#define SFI_SIG_GPIO		"GPIO"

#define SFI_SIGNATURE_SIZE	4
#define SFI_OEM_ID_SIZE		6
#define SFI_OEM_TABLE_ID_SIZE	8

#define SFI_NAME_LEN		16
#define SFI_TABLE_MAX_ENTRIES	16

#define SFI_GET_NUM_ENTRIES(ptable, entry_type) \
	((ptable->header.len - sizeof(struct sfi_table_header)) / \
	(sizeof(entry_type)))
/*
 * Table structures must be byte-packed to match the SFI specification,
 * as they are provided by the BIOS.
 */
struct __packed sfi_table_header {
	char	sig[SFI_SIGNATURE_SIZE];
	u32	len;
	u8	rev;
	u8	csum;
	char	oem_id[SFI_OEM_ID_SIZE];
	char	oem_table_id[SFI_OEM_TABLE_ID_SIZE];
};

struct __packed sfi_table_simple {
	struct sfi_table_header		header;
	u64				pentry[1];
};

/* Comply with UEFI spec 2.1 */
struct __packed sfi_mem_entry {
	u32	type;
	u64	phys_start;
	u64	virt_start;
	u64	pages;
	u64	attrib;
};

/* Memory type definitions */
enum sfi_mem_type {
	SFI_MEM_RESERVED,
	SFI_LOADER_CODE,
	SFI_LOADER_DATA,
	SFI_BOOT_SERVICE_CODE,
	SFI_BOOT_SERVICE_DATA,
	SFI_RUNTIME_SERVICE_CODE,
	SFI_RUNTIME_SERVICE_DATA,
	SFI_MEM_CONV,
	SFI_MEM_UNUSABLE,
	SFI_ACPI_RECLAIM,
	SFI_ACPI_NVS,
	SFI_MEM_MMIO,
	SFI_MEM_IOPORT,
	SFI_PAL_CODE,
	SFI_MEM_TYPEMAX,
};

struct __packed sfi_cpu_table_entry {
	u32	apic_id;
};

struct __packed sfi_cstate_table_entry {
	u32	hint;		/* MWAIT hint */
	u32	latency;	/* latency in ms */
};

struct __packed sfi_apic_table_entry {
	u64	phys_addr;	/* phy base addr for APIC reg */
};

struct __packed sfi_freq_table_entry {
	u32	freq_mhz;	/* in MHZ */
	u32	latency;	/* transition latency in ms */
	u32	ctrl_val;	/* value to write to PERF_CTL */
};

struct __packed sfi_wake_table_entry {
	u64	phys_addr;	/* pointer to where the wake vector locates */
};

struct __packed sfi_timer_table_entry {
	u64	phys_addr;	/* phy base addr for the timer */
	u32	freq_hz;	/* in HZ */
	u32	irq;
};

struct __packed sfi_rtc_table_entry {
	u64	phys_addr;	/* phy base addr for the RTC */
	u32	irq;
};

struct __packed sfi_device_table_entry {
	u8	type;		/* bus type, I2C, SPI or ...*/
	u8	host_num;	/* attached to host 0, 1...*/
	u16	addr;
	u8	irq;
	u32	max_freq;
	char	name[SFI_NAME_LEN];
};

enum {
	SFI_DEV_TYPE_SPI	= 0,
	SFI_DEV_TYPE_I2C,
	SFI_DEV_TYPE_UART,
	SFI_DEV_TYPE_HSI,
	SFI_DEV_TYPE_IPC,
	SFI_DEV_TYPE_SD,
};

struct __packed sfi_gpio_table_entry {
	char	controller_name[SFI_NAME_LEN];
	u16	pin_no;
	char	pin_name[SFI_NAME_LEN];
};

struct sfi_xsdt_header {
	uint32_t oem_revision;
	uint32_t creator_id;
	uint32_t creator_revision;
};

typedef int (*sfi_table_handler) (struct sfi_table_header *table);

/**
 * write_sfi_table() - Write Simple Firmware Interface tables
 *
 * @base:	Address to write table to
 * Return: address to use for the next table
 */
ulong write_sfi_table(ulong base);

#endif /*_LINUX_SFI_H */
