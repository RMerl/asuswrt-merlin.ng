/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Extensible Firmware Interface
 * Based on 'Extensible Firmware Interface Specification' version 0.9,
 * April 30, 1999
 *
 * Copyright (C) 1999 VA Linux Systems
 * Copyright (C) 1999 Walt Drummond <drummond@valinux.com>
 * Copyright (C) 1999, 2002-2003 Hewlett-Packard Co.
 *	David Mosberger-Tang <davidm@hpl.hp.com>
 *	Stephane Eranian <eranian@hpl.hp.com>
 *
 * From include/linux/efi.h in kernel 4.1 with some additions/subtractions
 */

#ifndef _EFI_H
#define _EFI_H

#include <linux/linkage.h>
#include <linux/string.h>
#include <linux/types.h>

/* Type INTN in UEFI specification */
#define efi_intn_t ssize_t
/* Type UINTN in UEFI specification*/
#define efi_uintn_t size_t

/*
 * EFI on x86_64 uses the Microsoft ABI which is not the default for GCC.
 *
 * There are two scenarios for EFI on x86_64: building a 64-bit EFI stub
 * codes (CONFIG_EFI_STUB_64BIT) and building a 64-bit U-Boot (CONFIG_X86_64).
 * Either needs to be properly built with the '-m64' compiler flag, and hence
 * it is enough to only check the compiler provided define __x86_64__ here.
 */
#ifdef __x86_64__
#define EFIAPI __attribute__((ms_abi))
#define efi_va_list __builtin_ms_va_list
#define efi_va_start __builtin_ms_va_start
#define efi_va_copy __builtin_ms_va_copy
#define efi_va_arg __builtin_va_arg
#define efi_va_end __builtin_ms_va_end
#else
#define EFIAPI asmlinkage
#define efi_va_list va_list
#define efi_va_start va_start
#define efi_va_copy va_copy
#define efi_va_arg va_arg
#define efi_va_end va_end
#endif /* __x86_64__ */

#define EFI32_LOADER_SIGNATURE	"EL32"
#define EFI64_LOADER_SIGNATURE	"EL64"

/**
 * struct efi_device_path - device path protocol
 *
 * @type:	device path type
 * @sub_type:	device path sub-type
 * @length:	length of the device path node including the header
 */
struct efi_device_path {
	u8 type;
	u8 sub_type;
	u16 length;
} __packed;

/*
 * The EFI spec defines the EFI_GUID as
 * "128-bit buffer containing a unique identifier value. Unless otherwise specified,
 * aligned on a 64-bit boundary".
 * Page 163 of the UEFI specification v2.10 and
 * EDK2 reference implementation both define EFI_GUID as
 * struct { u32 a; u16; b; u16 c; u8 d[8]; }; which is 4-byte
 * aligned.
 */
typedef struct {
	u8 b[16];
} efi_guid_t __attribute__((aligned(4)));

#define EFI_BITS_PER_LONG	(sizeof(long) * 8)

/* Bit mask for EFI status code with error */
#define EFI_ERROR_MASK (1UL << (EFI_BITS_PER_LONG - 1))
/* Status codes returned by EFI protocols */
#define EFI_SUCCESS			0
#define EFI_LOAD_ERROR			(EFI_ERROR_MASK | 1)
#define EFI_INVALID_PARAMETER		(EFI_ERROR_MASK | 2)
#define EFI_UNSUPPORTED			(EFI_ERROR_MASK | 3)
#define EFI_BAD_BUFFER_SIZE		(EFI_ERROR_MASK | 4)
#define EFI_BUFFER_TOO_SMALL		(EFI_ERROR_MASK | 5)
#define EFI_NOT_READY			(EFI_ERROR_MASK | 6)
#define EFI_DEVICE_ERROR		(EFI_ERROR_MASK | 7)
#define EFI_WRITE_PROTECTED		(EFI_ERROR_MASK | 8)
#define EFI_OUT_OF_RESOURCES		(EFI_ERROR_MASK | 9)
#define EFI_VOLUME_CORRUPTED		(EFI_ERROR_MASK | 10)
#define EFI_VOLUME_FULL			(EFI_ERROR_MASK | 11)
#define EFI_NO_MEDIA			(EFI_ERROR_MASK | 12)
#define EFI_MEDIA_CHANGED		(EFI_ERROR_MASK | 13)
#define EFI_NOT_FOUND			(EFI_ERROR_MASK | 14)
#define EFI_ACCESS_DENIED		(EFI_ERROR_MASK | 15)
#define EFI_NO_RESPONSE			(EFI_ERROR_MASK | 16)
#define EFI_NO_MAPPING			(EFI_ERROR_MASK | 17)
#define EFI_TIMEOUT			(EFI_ERROR_MASK | 18)
#define EFI_NOT_STARTED			(EFI_ERROR_MASK | 19)
#define EFI_ALREADY_STARTED		(EFI_ERROR_MASK | 20)
#define EFI_ABORTED			(EFI_ERROR_MASK | 21)
#define EFI_ICMP_ERROR			(EFI_ERROR_MASK | 22)
#define EFI_TFTP_ERROR			(EFI_ERROR_MASK | 23)
#define EFI_PROTOCOL_ERROR		(EFI_ERROR_MASK | 24)
#define EFI_INCOMPATIBLE_VERSION	(EFI_ERROR_MASK | 25)
#define EFI_SECURITY_VIOLATION		(EFI_ERROR_MASK | 26)
#define EFI_CRC_ERROR			(EFI_ERROR_MASK | 27)
#define EFI_END_OF_MEDIA		(EFI_ERROR_MASK | 28)
#define EFI_END_OF_FILE			(EFI_ERROR_MASK | 31)
#define EFI_INVALID_LANGUAGE		(EFI_ERROR_MASK | 32)
#define EFI_COMPROMISED_DATA		(EFI_ERROR_MASK | 33)
#define EFI_IP_ADDRESS_CONFLICT		(EFI_ERROR_MASK | 34)
#define EFI_HTTP_ERROR			(EFI_ERROR_MASK | 35)

#define EFI_WARN_UNKNOWN_GLYPH		1
#define EFI_WARN_DELETE_FAILURE		2
#define EFI_WARN_WRITE_FAILURE		3
#define EFI_WARN_BUFFER_TOO_SMALL	4
#define EFI_WARN_STALE_DATA		5
#define EFI_WARN_FILE_SYSTEM		6
#define EFI_WARN_RESET_REQUIRED		7

typedef unsigned long efi_status_t;
typedef u64 efi_physical_addr_t;
typedef u64 efi_virtual_addr_t;
typedef struct efi_object *efi_handle_t;

#define EFI_GUID(a, b, c, d0, d1, d2, d3, d4, d5, d6, d7) \
	{{ (a) & 0xff, ((a) >> 8) & 0xff, ((a) >> 16) & 0xff, \
		((a) >> 24) & 0xff, \
		(b) & 0xff, ((b) >> 8) & 0xff, \
		(c) & 0xff, ((c) >> 8) & 0xff, \
		(d0), (d1), (d2), (d3), (d4), (d5), (d6), (d7) } }

/* Generic EFI table header */
struct efi_table_hdr {
	u64 signature;
	u32 revision;
	u32 headersize;
	u32 crc32;
	u32 reserved;
};

/* Allocation types for calls to boottime->allocate_pages*/
/**
 * enum efi_allocate_type - address restriction for memory allocation
 */
enum efi_allocate_type {
	/**
	 * @EFI_ALLOCATE_ANY_PAGES:
	 * Allocate any block of sufficient size. Ignore memory address.
	 */
	EFI_ALLOCATE_ANY_PAGES,
	/**
	 * @EFI_ALLOCATE_MAX_ADDRESS:
	 * Allocate a memory block with an uppermost address less or equal
	 * to the indicated address.
	 */
	EFI_ALLOCATE_MAX_ADDRESS,
	/**
	 * @EFI_ALLOCATE_ADDRESS:
	 * Allocate a memory block starting at the indicatged adress.
	 */
	EFI_ALLOCATE_ADDRESS,
	/**
	 * @EFI_MAX_ALLOCATE_TYPE:
	 * Value use for range checking.
	 */
	EFI_MAX_ALLOCATE_TYPE,
};

/* Enumeration of memory types introduced in UEFI */
enum efi_memory_type {
	EFI_RESERVED_MEMORY_TYPE,
	/*
	 * The code portions of a loaded application.
	 * (Note that UEFI OS loaders are UEFI applications.)
	 */
	EFI_LOADER_CODE,
	/*
	 * The data portions of a loaded application and
	 * the default data allocation type used by an application
	 * to allocate pool memory.
	 */
	EFI_LOADER_DATA,
	/* The code portions of a loaded Boot Services Driver */
	EFI_BOOT_SERVICES_CODE,
	/*
	 * The data portions of a loaded Boot Services Driver and
	 * the default data allocation type used by a Boot Services
	 * Driver to allocate pool memory.
	 */
	EFI_BOOT_SERVICES_DATA,
	/* The code portions of a loaded Runtime Services Driver */
	EFI_RUNTIME_SERVICES_CODE,
	/*
	 * The data portions of a loaded Runtime Services Driver and
	 * the default data allocation type used by a Runtime Services
	 * Driver to allocate pool memory.
	 */
	EFI_RUNTIME_SERVICES_DATA,
	/* Free (unallocated) memory */
	EFI_CONVENTIONAL_MEMORY,
	/* Memory in which errors have been detected */
	EFI_UNUSABLE_MEMORY,
	/* Memory that holds the ACPI tables */
	EFI_ACPI_RECLAIM_MEMORY,
	/* Address space reserved for use by the firmware */
	EFI_ACPI_MEMORY_NVS,
	/*
	 * Used by system firmware to request that a memory-mapped IO region
	 * be mapped by the OS to a virtual address so it can be accessed by
	 * EFI runtime services.
	 */
	EFI_MMAP_IO,
	/*
	 * System memory-mapped IO region that is used to translate
	 * memory cycles to IO cycles by the processor.
	 */
	EFI_MMAP_IO_PORT,
	/*
	 * Address space reserved by the firmware for code that is
	 * part of the processor.
	 */
	EFI_PAL_CODE,
	/*
	 * Byte addressable non-volatile memory.
	 */
	EFI_PERSISTENT_MEMORY_TYPE,
	/*
	 * Unaccepted memory must be accepted by boot target before usage.
	 */
	EFI_UNACCEPTED_MEMORY_TYPE,

	EFI_MAX_MEMORY_TYPE,
};

/* Attribute values */
#define EFI_MEMORY_UC		((u64)0x0000000000000001ULL)	/* uncached */
#define EFI_MEMORY_WC		((u64)0x0000000000000002ULL)	/* write-coalescing */
#define EFI_MEMORY_WT		((u64)0x0000000000000004ULL)	/* write-through */
#define EFI_MEMORY_WB		((u64)0x0000000000000008ULL)	/* write-back */
#define EFI_MEMORY_UCE		((u64)0x0000000000000010ULL)	/* uncached, exported */
#define EFI_MEMORY_WP		((u64)0x0000000000001000ULL)	/* write-protect */
#define EFI_MEMORY_RP		((u64)0x0000000000002000ULL)	/* read-protect */
#define EFI_MEMORY_XP		((u64)0x0000000000004000ULL)	/* execute-protect */
#define EFI_MEMORY_NV		((u64)0x0000000000008000ULL)	/* non-volatile */
#define EFI_MEMORY_MORE_RELIABLE \
				((u64)0x0000000000010000ULL)	/* higher reliability */
#define EFI_MEMORY_RO		((u64)0x0000000000020000ULL)	/* read-only */
#define EFI_MEMORY_SP		((u64)0x0000000000040000ULL)	/* specific-purpose memory (SPM) */
#define EFI_MEMORY_CPU_CRYPTO	((u64)0x0000000000080000ULL)	/* cryptographically protectable */
#define EFI_MEMORY_RUNTIME	((u64)0x8000000000000000ULL)	/* range requires runtime mapping */
#define EFI_MEM_DESC_VERSION	1

#define EFI_PAGE_SHIFT		12
#define EFI_PAGE_SIZE		(1ULL << EFI_PAGE_SHIFT)
#define EFI_PAGE_MASK		(EFI_PAGE_SIZE - 1)

struct efi_mem_desc {
	u32 type;
	u32 reserved;
	efi_physical_addr_t physical_start;
	efi_virtual_addr_t virtual_start;
	u64 num_pages;
	u64 attribute;
};

#define EFI_MEMORY_DESCRIPTOR_VERSION 1

/* Types and defines for Time Services */
#define EFI_TIME_ADJUST_DAYLIGHT 0x1
#define EFI_TIME_IN_DAYLIGHT     0x2
#define EFI_UNSPECIFIED_TIMEZONE 0x07ff

struct efi_time {
	u16 year;
	u8 month;
	u8 day;
	u8 hour;
	u8 minute;
	u8 second;
	u8 pad1;
	u32 nanosecond;
	s16 timezone;
	u8 daylight;
	u8 pad2;
};

struct efi_time_cap {
	u32 resolution;
	u32 accuracy;
	u8 sets_to_zero;
};

enum efi_locate_search_type {
	ALL_HANDLES,
	BY_REGISTER_NOTIFY,
	BY_PROTOCOL
};

struct efi_open_protocol_info_entry {
	efi_handle_t agent_handle;
	efi_handle_t controller_handle;
	u32 attributes;
	u32 open_count;
};

enum efi_entry_t {
	EFIET_END,	/* Signals this is the last (empty) entry */
	EFIET_MEMORY_MAP,
	EFIET_GOP_MODE,
	EFIET_SYS_TABLE,

	/* Number of entries */
	EFIET_MEMORY_COUNT,
};

#define EFI_TABLE_VERSION	1

/**
 * struct efi_info_hdr - Header for the EFI info table
 *
 * @version:	EFI_TABLE_VERSION
 * @hdr_size:	Size of this struct in bytes
 * @total_size:	Total size of this header plus following data
 * @spare:	Spare space for expansion
 */
struct efi_info_hdr {
	u32 version;
	u32 hdr_size;
	u32 total_size;
	u32 spare[5];
};

/**
 * struct efi_entry_hdr - Header for a table entry
 *
 * @type:	enum eft_entry_t
 * @size:	size of entry bytes excluding header and padding
 * @addr:	address of this entry (0 if it follows the header )
 * @link:	size of entry including header and padding
 * @spare1:	Spare space for expansion
 * @spare2:	Spare space for expansion
 */
struct efi_entry_hdr {
	u32 type;
	u32 size;
	u64 addr;
	u32 link;
	u32 spare1;
	u64 spare2;
};

/**
 * struct efi_entry_memmap - a memory map table passed to U-Boot
 *
 * @version:	EFI's memory map table version
 * @desc_size:	EFI's size of each memory descriptor
 * @spare:	Spare space for expansion
 * @desc:	An array of descriptors, each @desc_size bytes apart
 */
struct efi_entry_memmap {
	u32 version;
	u32 desc_size;
	u64 spare;
	struct efi_mem_desc desc[];
};

/**
 * struct efi_entry_gopmode - a GOP mode table passed to U-Boot
 *
 * @fb_base:	EFI's framebuffer base address
 * @fb_size:	EFI's framebuffer size
 * @info_size:	GOP mode info structure size
 * @info:	Start address of the GOP mode info structure
 */
struct efi_entry_gopmode {
	efi_physical_addr_t fb_base;
	/*
	 * Not like the ones in 'struct efi_gop_mode' which are 'unsigned
	 * long', @fb_size and @info_size have to be 'u64' here. As the EFI
	 * stub codes may have different bit size from the U-Boot payload,
	 * using 'long' will cause mismatch between the producer (stub) and
	 * the consumer (payload).
	 */
	u64 fb_size;
	u64 info_size;
	/*
	 * We cannot directly use 'struct efi_gop_mode_info info[]' here as
	 * it causes compiler to complain: array type has incomplete element
	 * type 'struct efi_gop_mode_info'.
	 */
	struct /* efi_gop_mode_info */ {
		u32 version;
		u32 width;
		u32 height;
		u32 pixel_format;
		u32 pixel_bitmask[4];
		u32 pixels_per_scanline;
	} info[];
};

/**
 * struct efi_entry_systable - system table passed to U-Boot
 *
 * @sys_table:	EFI system table address
 */
struct efi_entry_systable {
	efi_physical_addr_t sys_table;
};

static inline struct efi_mem_desc *efi_get_next_mem_desc(
		struct efi_mem_desc *desc, int desc_size)
{
	return (struct efi_mem_desc *)((ulong)desc + desc_size);
}

/**
 * struct efi_priv - Information about the environment provided by EFI
 *
 * @parent_image: image passed into the EFI app or stub
 * @sys_table: Pointer to system table
 * @boot: Pointer to boot-services table
 * @run: Pointer to runtime-services table
 * @memmap_key: Key returned from get_memory_map()
 * @memmap_desc: List of memory-map records
 * @memmap_alloc: Amount of memory allocated for memory map list
 * @memmap_size Size of memory-map list in bytes
 * @memmap_desc_size: Size of an individual memory-map record, in bytes
 * @memmap_version: Memory-map version
 *
 * @use_pool_for_malloc: true if all allocation should go through the EFI 'pool'
 *	methods allocate_pool() and free_pool(); false to use 'pages' methods
 *	allocate_pages() and free_pages()
 * @ram_base: Base address of RAM (size CONFIG_EFI_RAM_SIZE)
 * @image_data_type: Type of the loaded image (e.g. EFI_LOADER_CODE)
 *
 * @info: Header of the info list, holding info collected by the stub and passed
 *	to U-Boot
 * @info_size: Size of the info list @info in bytes
 * @next_hdr: Pointer to where to put the next header when adding to the list
 */
struct efi_priv {
	efi_handle_t parent_image;
	struct efi_system_table *sys_table;
	struct efi_boot_services *boot;
	struct efi_runtime_services *run;
	efi_uintn_t memmap_key;
	struct efi_mem_desc *memmap_desc;
	efi_uintn_t memmap_alloc;
	efi_uintn_t memmap_size;
	efi_uintn_t memmap_desc_size;
	u32 memmap_version;

	/* app: */
	bool use_pool_for_malloc;
	unsigned long ram_base;
	unsigned int image_data_type;

	/* stub: */
	struct efi_info_hdr *info;
	unsigned int info_size;
	void *next_hdr;
};

/*
 * EFI attributes of the udevice handled by efi_media driver
 *
 * @handle: handle of the controller on which this driver is installed
 * @blkio: block io protocol proxied by this driver
 * @device_path: EFI path to the device
 */
struct efi_media_plat {
	efi_handle_t handle;
	struct efi_block_io *blkio;
	struct efi_device_path *device_path;
};

/* Base address of the EFI image */
extern char image_base[];

/* Start and end of U-Boot image (for payload) */
extern char _binary_u_boot_bin_start[], _binary_u_boot_bin_end[];

/*
 * Variable Attributes
 */
#define EFI_VARIABLE_NON_VOLATILE       0x0000000000000001
#define EFI_VARIABLE_BOOTSERVICE_ACCESS 0x0000000000000002
#define EFI_VARIABLE_RUNTIME_ACCESS     0x0000000000000004
#define EFI_VARIABLE_HARDWARE_ERROR_RECORD 0x0000000000000008
#define EFI_VARIABLE_AUTHENTICATED_WRITE_ACCESS 0x0000000000000010
#define EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS 0x0000000000000020
#define EFI_VARIABLE_APPEND_WRITE	0x0000000000000040

#define EFI_VARIABLE_MASK	(EFI_VARIABLE_NON_VOLATILE | \
				EFI_VARIABLE_BOOTSERVICE_ACCESS | \
				EFI_VARIABLE_RUNTIME_ACCESS | \
				EFI_VARIABLE_HARDWARE_ERROR_RECORD | \
				EFI_VARIABLE_AUTHENTICATED_WRITE_ACCESS | \
				EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS | \
				EFI_VARIABLE_APPEND_WRITE)

/**
 * efi_get_priv() - Get access to the EFI-private information
 *
 * This struct it used by both the stub and the app to record things about the
 * EFI environment. It is not available in U-Boot proper after the stub has
 * jumped there. Use efi_info_get() to obtain info in that case.
 *
 * Return: pointer to private info
 */
struct efi_priv *efi_get_priv(void);

/**
 * efi_set_priv() - Set up a pointer to the EFI-private information
 *
 * This is called in the stub and app to record the location of this
 * information.
 *
 * @priv: New location of private data
 */
void efi_set_priv(struct efi_priv *priv);

/**
 * efi_get_sys_table() - Get access to the main EFI system table
 *
 * Returns: pointer to EFI system table
 */
struct efi_system_table *efi_get_sys_table(void);

/**
 * efi_get_boot() - Get access to the EFI boot services table
 *
 * Returns: pointer to EFI boot services table
 */
struct efi_boot_services *efi_get_boot(void);

/**
 * efi_get_ram_base() - Find the base of RAM
 *
 * This is used when U-Boot is built as an EFI application.
 *
 * Returns: the base of RAM as known to U-Boot
 */
unsigned long efi_get_ram_base(void);

/**
 * efi_init() - Set up ready for use of EFI boot services
 *
 * @priv:	Pointer to our private EFI structure to fill in
 * @banner:	Banner to display when starting
 * @image:	The image handle passed to efi_main()
 * @sys_table:	The EFI system table pointer passed to efi_main()
 * Return: 0 on succcess, EFI error code on failure
 */
int efi_init(struct efi_priv *priv, const char *banner, efi_handle_t image,
	     struct efi_system_table *sys_table);

/**
 * efi_malloc() - Allocate some memory from EFI
 *
 * @priv:	Pointer to private EFI structure
 * @size:	Number of bytes to allocate
 * @retp:	Return EFI status result
 * Returns: pointer to memory allocated, or NULL on error
 */
void *efi_malloc(struct efi_priv *priv, int size, efi_status_t *retp);

/**
 * efi_free() - Free memory allocated from EFI
 *
 * @priv:	Pointer to private EFI structure
 * @ptr:	Pointer to memory to free
 */
void efi_free(struct efi_priv *priv, void *ptr);

/**
 * efi_puts() - Write out a string to the EFI console
 *
 * @priv:	Pointer to private EFI structure
 * @str:	String to write (note this is a ASCII, not unicode)
 */
void efi_puts(struct efi_priv *priv, const char *str);

/**
 * efi_putc() - Write out a character to the EFI console
 *
 * @priv:	Pointer to private EFI structure
 * @ch:		Character to write (note this is not unicode)
 */
void efi_putc(struct efi_priv *priv, const char ch);

/**
 * efi_info_get() - get an entry from an EFI table
 *
 * This function is called from U-Boot proper to read information set up by the
 * EFI stub. It can only be used when running from the EFI stub, not when U-Boot
 * is running as an app.
 *
 * @type:	Entry type to search for
 * @datap:	Returns pointer to entry data
 * @sizep:	Returns entry size
 * Return: 0 if OK, -ENODATA if there is no table, -ENOENT if there is no entry
 * of the requested type, -EPROTONOSUPPORT if the table has the wrong version
 */
int efi_info_get(enum efi_entry_t type, void **datap, int *sizep);

/**
 * efi_store_memory_map() - Collect the memory-map info from EFI
 *
 * Collect the memory info and store it for later use, e.g. in calling
 * exit_boot_services()
 *
 * @priv:	Pointer to private EFI structure
 * Returns: 0 if OK, non-zero on error
 */
int efi_store_memory_map(struct efi_priv *priv);

/**
 * efi_call_exit_boot_services() - Handle the exit-boot-service procedure
 *
 * Tell EFI we don't want their boot services anymore
 *
 * Return: 0 if OK, non-zero on error
 */
int efi_call_exit_boot_services(void);

/**
 * efi_get_mmap() - Get the memory map from EFI
 *
 * This is used in the app. The caller must free *@descp when done
 *
 * @descp:	Returns allocated pointer to EFI memory map table
 * @sizep:	Returns size of table in bytes
 * @keyp:	Returns memory-map key
 * @desc_sizep:	Returns size of each @desc_base record
 * @versionp:	Returns version number of memory map
 * Returns: 0 on success, -ve on error
 */
int efi_get_mmap(struct efi_mem_desc **descp, int *sizep, uint *keyp,
		 int *desc_sizep, uint *versionp);

/**
 * efi_show_tables() - Show a list of available tables
 *
 * Shows the address, GUID (and name where known) for each table
 *
 * @systab: System table containing the list of tables
 */
void efi_show_tables(struct efi_system_table *systab);

#endif /* _LINUX_EFI_H */
