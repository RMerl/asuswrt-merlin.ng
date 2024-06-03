/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2002-2010
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#ifndef	__ASM_GBL_DATA_H
#define __ASM_GBL_DATA_H

#ifndef __ASSEMBLY__

#include <asm/processor.h>

enum pei_boot_mode_t {
	PEI_BOOT_NONE = 0,
	PEI_BOOT_SOFT_RESET,
	PEI_BOOT_RESUME,

};

struct dimm_info {
	uint32_t dimm_size;
	uint16_t ddr_type;
	uint16_t ddr_frequency;
	uint8_t rank_per_dimm;
	uint8_t channel_num;
	uint8_t dimm_num;
	uint8_t bank_locator;
	/* The 5th byte is '\0' for the end of string */
	uint8_t serial[5];
	/* The 19th byte is '\0' for the end of string */
	uint8_t module_part_number[19];
	uint16_t mod_id;
	uint8_t mod_type;
	uint8_t bus_width;
} __packed;

struct pei_memory_info {
	uint8_t dimm_cnt;
	/* Maximum num of dimm is 8 */
	struct dimm_info dimm[8];
} __packed;

struct memory_area {
	uint64_t start;
	uint64_t size;
};

struct memory_info {
	int num_areas;
	uint64_t total_memory;
	uint64_t total_32bit_memory;
	struct memory_area area[CONFIG_NR_DRAM_BANKS];
};

#define MAX_MTRR_REQUESTS	8

/**
 * A request for a memory region to be set up in a particular way. These
 * requests are processed before board_init_r() is called. They are generally
 * optional and can be ignored with some performance impact.
 */
struct mtrr_request {
	int type;		/* MTRR_TYPE_... */
	uint64_t start;
	uint64_t size;
};

/* Architecture-specific global data */
struct arch_global_data {
	u64 gdt[X86_GDT_NUM_ENTRIES] __aligned(16);
	struct global_data *gd_addr;	/* Location of Global Data */
	uint8_t x86;			/* CPU family */
	uint8_t x86_vendor;		/* CPU vendor */
	uint8_t x86_model;
	uint8_t x86_mask;
	uint32_t x86_device;
	uint64_t tsc_base;		/* Initial value returned by rdtsc() */
	unsigned long clock_rate;	/* Clock rate of timer in Hz */
	void *new_fdt;			/* Relocated FDT */
	uint32_t bist;			/* Built-in self test value */
	enum pei_boot_mode_t pei_boot_mode;
	const struct pch_gpio_map *gpio_map;	/* board GPIO map */
	struct memory_info meminfo;	/* Memory information */
	struct pei_memory_info pei_meminfo;	/* PEI memory information */
#ifdef CONFIG_HAVE_FSP
	void *hob_list;			/* FSP HOB list */
#endif
	struct mtrr_request mtrr_req[MAX_MTRR_REQUESTS];
	int mtrr_req_count;
	int has_mtrr;
	/* MRC training data to save for the next boot */
	char *mrc_output;
	unsigned int mrc_output_len;
	ulong table;			/* Table pointer from previous loader */
	int turbo_state;		/* Current turbo state */
	struct irq_routing_table *pirq_routing_table;
#ifdef CONFIG_SEABIOS
	u32 high_table_ptr;
	u32 high_table_limit;
#endif
#ifdef CONFIG_HAVE_ACPI_RESUME
	int prev_sleep_state;		/* Previous sleep state ACPI_S0/1../5 */
	ulong backup_mem;		/* Backup memory address for S3 */
#endif
};

#endif

#include <asm-generic/global_data.h>

#ifndef __ASSEMBLY__
# if defined(CONFIG_EFI_APP) || CONFIG_IS_ENABLED(X86_64)

/* TODO(sjg@chromium.org): Consider using a fixed register for gd on x86_64 */
#define gd global_data_ptr

#define DECLARE_GLOBAL_DATA_PTR   extern struct global_data *global_data_ptr
# else
static inline __attribute__((no_instrument_function)) gd_t *get_fs_gd_ptr(void)
{
	gd_t *gd_ptr;

#if CONFIG_IS_ENABLED(X86_64)
	asm volatile("fs mov 0, %0\n" : "=r" (gd_ptr));
#else
	asm volatile("fs movl 0, %0\n" : "=r" (gd_ptr));
#endif

	return gd_ptr;
}

#define gd	get_fs_gd_ptr()

#define DECLARE_GLOBAL_DATA_PTR
# endif

#endif

/*
 * Our private Global Data Flags
 */
#define GD_FLG_COLD_BOOT	0x10000	/* Cold Boot */
#define GD_FLG_WARM_BOOT	0x20000	/* Warm Boot */

#endif /* __ASM_GBL_DATA_H */
