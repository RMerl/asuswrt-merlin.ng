/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2015 Miao Yan <yanmiaobest@gmail.com>
 */

#ifndef __FW_CFG__
#define __FW_CFG__

#include <linux/list.h>

enum qemu_fwcfg_items {
	FW_CFG_SIGNATURE	= 0x00,
	FW_CFG_ID		= 0x01,
	FW_CFG_UUID		= 0x02,
	FW_CFG_RAM_SIZE		= 0x03,
	FW_CFG_NOGRAPHIC	= 0x04,
	FW_CFG_NB_CPUS		= 0x05,
	FW_CFG_MACHINE_ID	= 0x06,
	FW_CFG_KERNEL_ADDR	= 0x07,
	FW_CFG_KERNEL_SIZE	= 0x08,
	FW_CFG_KERNEL_CMDLINE   = 0x09,
	FW_CFG_INITRD_ADDR	= 0x0a,
	FW_CFG_INITRD_SIZE	= 0x0b,
	FW_CFG_BOOT_DEVICE	= 0x0c,
	FW_CFG_NUMA		= 0x0d,
	FW_CFG_BOOT_MENU	= 0x0e,
	FW_CFG_MAX_CPUS		= 0x0f,
	FW_CFG_KERNEL_ENTRY	= 0x10,
	FW_CFG_KERNEL_DATA	= 0x11,
	FW_CFG_INITRD_DATA	= 0x12,
	FW_CFG_CMDLINE_ADDR	= 0x13,
	FW_CFG_CMDLINE_SIZE	= 0x14,
	FW_CFG_CMDLINE_DATA	= 0x15,
	FW_CFG_SETUP_ADDR	= 0x16,
	FW_CFG_SETUP_SIZE	= 0x17,
	FW_CFG_SETUP_DATA	= 0x18,
	FW_CFG_FILE_DIR		= 0x19,
	FW_CFG_FILE_FIRST	= 0x20,
	FW_CFG_WRITE_CHANNEL	= 0x4000,
	FW_CFG_ARCH_LOCAL	= 0x8000,
	FW_CFG_INVALID		= 0xffff,
};

enum {
	BIOS_LINKER_LOADER_COMMAND_ALLOCATE	= 0x1,
	BIOS_LINKER_LOADER_COMMAND_ADD_POINTER  = 0x2,
	BIOS_LINKER_LOADER_COMMAND_ADD_CHECKSUM = 0x3,
};

enum {
	BIOS_LINKER_LOADER_ALLOC_ZONE_HIGH = 0x1,
	BIOS_LINKER_LOADER_ALLOC_ZONE_FSEG = 0x2,
};

#define FW_CFG_FILE_SLOTS	0x10
#define FW_CFG_MAX_ENTRY	(FW_CFG_FILE_FIRST + FW_CFG_FILE_SLOTS)
#define FW_CFG_ENTRY_MASK	 ~(FW_CFG_WRITE_CHANNEL | FW_CFG_ARCH_LOCAL)

#define FW_CFG_MAX_FILE_PATH	56
#define BIOS_LINKER_LOADER_FILESZ FW_CFG_MAX_FILE_PATH

#define QEMU_FW_CFG_SIGNATURE	(('Q' << 24) | ('E' << 16) | ('M' << 8) | 'U')

#define FW_CFG_DMA_ERROR	(1 << 0)
#define FW_CFG_DMA_READ	(1 << 1)
#define FW_CFG_DMA_SKIP	(1 << 2)
#define FW_CFG_DMA_SELECT	(1 << 3)

#define FW_CFG_DMA_ENABLED	(1 << 1)

struct fw_cfg_file {
	__be32 size;
	__be16 select;
	__be16 reserved;
	char name[FW_CFG_MAX_FILE_PATH];
};

struct fw_file {
	struct fw_cfg_file cfg; /* firmware file information */
	unsigned long addr;     /* firmware file in-memory address */
	struct list_head list;  /* list node to link to fw_list */
};

struct fw_cfg_file_iter {
	struct list_head *entry; /* structure to iterate file list */
};

struct fw_cfg_dma_access {
	__be32 control;
	__be32 length;
	__be64 address;
};

struct fw_cfg_arch_ops {
	void (*arch_read_pio)(uint16_t selector, uint32_t size,
			void *address);
	void (*arch_read_dma)(struct fw_cfg_dma_access *dma);
};

struct bios_linker_entry {
	__le32 command;
	union {
		/*
		 * COMMAND_ALLOCATE - allocate a table from @alloc.file
		 * subject to @alloc.align alignment (must be power of 2)
		 * and @alloc.zone (can be HIGH or FSEG) requirements.
		 *
		 * Must appear exactly once for each file, and before
		 * this file is referenced by any other command.
		 */
		struct {
			char file[BIOS_LINKER_LOADER_FILESZ];
			__le32 align;
			uint8_t zone;
		} alloc;

		/*
		 * COMMAND_ADD_POINTER - patch the table (originating from
		 * @dest_file) at @pointer.offset, by adding a pointer to the
		 * table originating from @src_file. 1,2,4 or 8 byte unsigned
		 * addition is used depending on @pointer.size.
		 */
		struct {
			char dest_file[BIOS_LINKER_LOADER_FILESZ];
			char src_file[BIOS_LINKER_LOADER_FILESZ];
			__le32 offset;
			uint8_t size;
		} pointer;

		/*
		 * COMMAND_ADD_CHECKSUM - calculate checksum of the range
		 * specified by @cksum_start and @cksum_length fields,
		 * and then add the value at @cksum.offset.
		 * Checksum simply sums -X for each byte X in the range
		 * using 8-bit math.
		 */
		struct {
			char file[BIOS_LINKER_LOADER_FILESZ];
			__le32 offset;
			__le32 start;
			__le32 length;
		} cksum;

		/* padding */
		char pad[124];
	};
} __packed;

/**
 * Initialize QEMU fw_cfg interface
 *
 * @ops: arch specific read operations
 */
void qemu_fwcfg_init(struct fw_cfg_arch_ops *ops);

void qemu_fwcfg_read_entry(uint16_t entry, uint32_t length, void *address);
int qemu_fwcfg_read_firmware_list(void);
struct fw_file *qemu_fwcfg_find_file(const char *name);

/**
 * Get system cpu number
 *
 * @return:   cpu number in system
 */
int qemu_fwcfg_online_cpus(void);

/* helper functions to iterate firmware file list */
struct fw_file *qemu_fwcfg_file_iter_init(struct fw_cfg_file_iter *iter);
struct fw_file *qemu_fwcfg_file_iter_next(struct fw_cfg_file_iter *iter);
bool qemu_fwcfg_file_iter_end(struct fw_cfg_file_iter *iter);

bool qemu_fwcfg_present(void);
bool qemu_fwcfg_dma_present(void);

#endif
