/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2015 Miao Yan <yanmiaobest@gmail.com>
 */

#ifndef __FW_CFG__
#define __FW_CFG__

#include <linux/list.h>

/*
 * List of firmware configuration item selectors. The official source of truth
 * for these is the QEMU source itself; see
 * https://github.com/qemu/qemu/blob/master/hw/nvram/fw_cfg.c
 */
enum {
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

/* Bit set in FW_CFG_ID response to indicate DMA interface availability. */
#define FW_CFG_DMA_ENABLED	(1 << 1)

/* Structs read from FW_CFG_FILE_DIR. */
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
	struct list_head *entry, *end; /* structures to iterate file list */
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

/* DMA transfer control data between UCLASS_QFW and QEMU. */
struct qfw_dma {
	__be32 control;
	__be32 length;
	__be64 address;
};

/* uclass per-device configuration information */
struct qfw_dev {
	struct udevice *dev;		/* Transport device */
	bool dma_present;		/* DMA interface usable? */
	struct list_head fw_list;	/* Cached firmware file list */
};

/* Ops used internally between UCLASS_QFW and its driver implementations. */
struct dm_qfw_ops {
	/**
	 * read_entry_io() - Read a firmware config entry using the regular
	 * IO interface for the platform (either PIO or MMIO)
	 *
	 * Supply %FW_CFG_INVALID as the entry to continue a previous read.  In
	 * this case, no selector will be issued before reading.
	 *
	 * @dev: Device to use
	 * @entry: Firmware config entry number (e.g. %FW_CFG_SIGNATURE)
	 * @size: Number of bytes to read
	 * @address: Target location for read
	 */
	void (*read_entry_io)(struct udevice *dev, u16 entry, u32 size,
			      void *address);

	/**
	 * read_entry_dma() - Read a firmware config entry using the DMA
	 * interface
	 *
	 * Supply FW_CFG_INVALID as the entry to continue a previous read.  In
	 * this case, no selector will be issued before reading.
	 *
	 * This method assumes DMA availability has already been confirmed.
	 *
	 * @dev: Device to use
	 * @dma: DMA transfer control struct
	 */
	void (*read_entry_dma)(struct udevice *dev, struct qfw_dma *dma);
};

#define dm_qfw_get_ops(dev) \
		((struct dm_qfw_ops *)(dev)->driver->ops)

/**
 * qfw_register() - Called by a qfw driver after successful probe.
 * @dev: Device registering itself with the uclass.
 *
 * Used internally by driver implementations on successful probe.
 *
 * Return: 0 on success, negative otherwise.
 */
int qfw_register(struct udevice *dev);

struct udevice;

/**
 * qfw_get_dev() - Get QEMU firmware config device.
 * @devp: Pointer to be filled with address of the qfw device.
 *
 * Gets the active QEMU firmware config device, for use with qfw_read_entry()
 * and others.
 *
 * Return: 0 on success, -ENODEV if the device is not available.
 */
int qfw_get_dev(struct udevice **devp);

/**
 * qfw_read_entry() - Read a QEMU firmware config entry
 * @dev: QFW device to use.
 * @entry: Firmware config entry number (e.g. %FW_CFG_SIGNATURE).
 * @size: Number of bytes to read.
 * @address: Target location for read.
 *
 * Reads a QEMU firmware config entry using @dev.  DMA will be used if the QEMU
 * machine supports it, otherwise PIO/MMIO.
 */
void qfw_read_entry(struct udevice *dev, u16 entry, u32 size, void *address);

/**
 * qfw_read_firmware_list() - Read and cache the QEMU firmware config file
 * list.
 * @dev: QFW device to use.
 *
 * Reads the QEMU firmware config file list, caching it against @dev for later
 * use with qfw_find_file().
 *
 * If the list has already been read, does nothing and returns 0 (success).
 *
 * Return: 0 on success, -ENOMEM if unable to allocate.
 */
int qfw_read_firmware_list(struct udevice *dev);

/**
 * qfw_find_file() - Find a file by name in the QEMU firmware config file
 * list.
 * @dev: QFW device to use.
 * @name: Name of file to locate (e.g. "etc/table-loader").
 *
 * Finds a file by name in the QEMU firmware config file list cached against
 * @dev.  You must call qfw_read_firmware_list() successfully first for this to
 * succeed.
 *
 * Return: Pointer to &struct fw_file if found, %NULL if not present.
 */
struct fw_file *qfw_find_file(struct udevice *dev, const char *name);

/**
 * qfw_online_cpus() - Get number of CPUs in system from QEMU firmware config.
 * @dev: QFW device to use.
 *
 * Asks QEMU to report how many CPUs it is emulating for the machine.
 *
 * Return: Number of CPUs in the system.
 */
int qfw_online_cpus(struct udevice *dev);

/**
 * qfw_file_iter_init() - Start iterating cached firmware file list.
 * @dev: QFW device to use.
 * @iter: Iterator to be initialised.
 *
 * Starts iterating the cached firmware file list in @dev.  You must call
 * qfw_read_firmware_list() successfully first, otherwise you will always get
 * an empty list.
 *
 * qfw_file_iter_init() returns the first &struct fw_file, but it may be
 * invalid if the list is empty.  Check that ``!qfw_file_iter_end(&iter)``
 * first.
 *
 * Return: The first &struct fw_file item in the firmware file list, if any.
 * Only valid when qfw_file_iter_end() is not true after the call.
 */
struct fw_file *qfw_file_iter_init(struct udevice *dev,
				   struct fw_cfg_file_iter *iter);

/**
 * qfw_file_iter_next() - Iterate cached firmware file list.
 * @iter: Iterator to use.
 *
 * Continues iterating the cached firmware file list in @dev.  You must call
 * qfw_file_iter_init() first to initialise it.  Check that
 * ``!qfw_file_iter_end(&iter)`` before using the return value of this
 * function.
 *
 * Return: The next &struct fw_file item in the firmware file list.  Only valid
 * when qfw_file_iter_end() is not true after the call.
 */
struct fw_file *qfw_file_iter_next(struct fw_cfg_file_iter *iter);

/**
 * qfw_file_iter_end() - Check if iter is at end of list.
 * @iter: Iterator to use.
 *
 * Checks whether or not the iterator is at its end position.  If so, the
 * qfw_file_iter_init() or qfw_file_iter_next() call that immediately preceded
 * returned invalid data.
 *
 * Return: True if the iterator is at its end; false otherwise.
 */
bool qfw_file_iter_end(struct fw_cfg_file_iter *iter);

/**
 * qemu_cpu_fixup() - Fix up the CPUs for QEMU.
 *
 * Return: 0 on success, -ENODEV if no CPUs, -ENOMEM if out of memory, other <
 * 0 on on other error.
 */
int qemu_cpu_fixup(void);

/*
 * qemu_fwcfg_setup_kernel() - Prepare the kernel for zboot
 *
 * Loads kernel data to 'load_addr', initrd to 'initrd_addr' and kernel command
 * line using qemu fw_cfg interface
 *
 * @load_addr: Load address for kernel
 * @initrd_addr: Load address for ramdisk
 * @return 0 if OK, -ENOENT if no kernel
 */
int qemu_fwcfg_setup_kernel(struct udevice *qfw_dev, ulong load_addr,
			    ulong initrd_addr);

#endif
