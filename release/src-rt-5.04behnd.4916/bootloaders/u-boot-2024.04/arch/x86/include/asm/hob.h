/* SPDX-License-Identifier: Intel */
/*
 * Copyright (C) 2013, Intel Corporation
 * Copyright (C) 2014, Bin Meng <bmeng.cn@gmail.com>
 */

#ifndef __HOB_H__
#define __HOB_H__

#include <efi.h>
#include <efi_loader.h>

/* Type of HOB Header */
#define HOB_TYPE_MEM_ALLOC	0x0002
#define HOB_TYPE_RES_DESC	0x0003
#define HOB_TYPE_GUID_EXT	0x0004
#define HOB_TYPE_UNUSED		0xFFFE
#define HOB_TYPE_EOH		0xFFFF

/* Value of ResourceType in HOB_RES_DESC */
#define RES_SYS_MEM		0x00000000
#define RES_MMAP_IO		0x00000001
#define RES_IO			0x00000002
#define RES_FW_DEVICE		0x00000003
#define RES_MMAP_IO_PORT	0x00000004
#define RES_MEM_RESERVED	0x00000005
#define RES_IO_RESERVED		0x00000006
#define RES_MAX_MEM_TYPE	0x00000007

/*
 * These types can be ORed together as needed.
 *
 * The first three enumerations describe settings
 * The rest of the settings describe capabilities
 */
#define RES_ATTR_PRESENT			0x00000001
#define RES_ATTR_INITIALIZED			0x00000002
#define RES_ATTR_TESTED				0x00000004
#define RES_ATTR_SINGLE_BIT_ECC			0x00000008
#define RES_ATTR_MULTIPLE_BIT_ECC		0x00000010
#define RES_ATTR_ECC_RESERVED_1			0x00000020
#define RES_ATTR_ECC_RESERVED_2			0x00000040
#define RES_ATTR_READ_PROTECTED			0x00000080
#define RES_ATTR_WRITE_PROTECTED		0x00000100
#define RES_ATTR_EXECUTION_PROTECTED		0x00000200
#define RES_ATTR_UNCACHEABLE			0x00000400
#define RES_ATTR_WRITE_COMBINEABLE		0x00000800
#define RES_ATTR_WRITE_THROUGH_CACHEABLE	0x00001000
#define RES_ATTR_WRITE_BACK_CACHEABLE		0x00002000
#define RES_ATTR_16_BIT_IO			0x00004000
#define RES_ATTR_32_BIT_IO			0x00008000
#define RES_ATTR_64_BIT_IO			0x00010000
#define RES_ATTR_UNCACHED_EXPORTED		0x00020000

/*
 * Describes the format and size of the data inside the HOB.
 * All HOBs must contain this generic HOB header.
 */
struct hob_header {
	u16	type;		/* HOB type */
	u16	len;		/* HOB length */
	u32	reserved;	/* always zero */
};

/*
 * Describes all memory ranges used during the HOB producer phase that
 * exist outside the HOB list. This HOB type describes how memory is used,
 * not the physical attributes of memory.
 */
struct hob_mem_alloc {
	struct hob_header	hdr;
	/*
	 * A GUID that defines the memory allocation region's type and purpose,
	 * as well as other fields within the memory allocation HOB. This GUID
	 * is used to define the additional data within the HOB that may be
	 * present for the memory allocation HOB. Type efi_guid_t is defined in
	 * InstallProtocolInterface() in the UEFI 2.0 specification.
	 */
	efi_guid_t		name;
	/*
	 * The base address of memory allocated by this HOB.
	 * Type phys_addr_t is defined in AllocatePages() in the UEFI 2.0
	 * specification.
	 */
	phys_addr_t		mem_base;
	/* The length in bytes of memory allocated by this HOB */
	phys_size_t		mem_len;
	/*
	 * Defines the type of memory allocated by this HOB.
	 * The memory type definition follows the EFI_MEMORY_TYPE definition.
	 * Type EFI_MEMORY_TYPE is defined in AllocatePages() in the UEFI 2.0
	 * specification.
	 */
	enum efi_memory_type	mem_type;
	/* padding */
	u8			reserved[4];
};

/*
 * Describes the resource properties of all fixed, nonrelocatable resource
 * ranges found on the processor host bus during the HOB producer phase.
 */
struct hob_res_desc {
	struct hob_header	hdr;
	/*
	 * A GUID representing the owner of the resource. This GUID is
	 * used by HOB consumer phase components to correlate device
	 * ownership of a resource.
	 */
	efi_guid_t		owner;
	u32			type;
	u32			attr;
	/* The physical start address of the resource region */
	phys_addr_t		phys_start;
	/* The number of bytes of the resource region */
	phys_size_t		len;
};

/*
 * Allows writers of executable content in the HOB producer phase to
 * maintain and manage HOBs with specific GUID.
 */
struct hob_guid {
	struct hob_header	hdr;
	/* A GUID that defines the contents of this HOB */
	efi_guid_t		name;
	/* GUID specific data goes here */
};

/**
 * get_next_hob() - return a pointer to the next HOB in the HOB list
 *
 * This macro returns a pointer to HOB that follows the HOB specified by hob
 * in the HOB List.
 *
 * @hdr:    A pointer to a HOB.
 *
 * Return: A pointer to the next HOB in the HOB list.
 */
static inline const struct hob_header *get_next_hob(const struct hob_header
						    *hdr)
{
	return (const struct hob_header *)((uintptr_t)hdr + hdr->len);
}

/**
 * end_of_hob() - determine if a HOB is the last HOB in the HOB list
 *
 * This macro determine if the HOB specified by hob is the last HOB in the
 * HOB list.  If hob is last HOB in the HOB list, then true is returned.
 * Otherwise, false is returned.
 *
 * @hdr:          A pointer to a HOB.
 *
 * Return: true:  The HOB specified by hdr is the last HOB in the HOB list.
 * Return: false: The HOB specified by hdr is not the last HOB in the HOB list.
 */
static inline bool end_of_hob(const struct hob_header *hdr)
{
	return hdr->type == HOB_TYPE_EOH;
}

/**
 * get_guid_hob_data() - return a pointer to data buffer from a HOB of
 *                       type HOB_TYPE_GUID_EXT
 *
 * This macro returns a pointer to the data buffer in a HOB specified by hob.
 * hob is assumed to be a HOB of type HOB_TYPE_GUID_EXT.
 *
 * @hdr:    A pointer to a HOB.
 *
 * Return: A pointer to the data buffer in a HOB.
 */
static inline void *get_guid_hob_data(const struct hob_header *hdr)
{
	return (void *)((uintptr_t)hdr + sizeof(struct hob_guid));
}

/**
 * get_guid_hob_data_size() - return the size of the data buffer from a HOB
 *                            of type HOB_TYPE_GUID_EXT
 *
 * This macro returns the size, in bytes, of the data buffer in a HOB
 * specified by hob. hob is assumed to be a HOB of type HOB_TYPE_GUID_EXT.
 *
 * @hdr:    A pointer to a HOB.
 *
 * Return: The size of the data buffer.
 */
static inline u16 get_guid_hob_data_size(const struct hob_header *hdr)
{
	return hdr->len - sizeof(struct hob_guid);
}

/**
 * Returns the next instance of a HOB type from the starting HOB.
 *
 * @type:     HOB type to search
 * @hob_list: A pointer to the HOB list
 *
 * Return: A HOB object with matching type; Otherwise NULL.
 */
const struct hob_header *hob_get_next_hob(uint type, const void *hob_list);

/**
 * Returns the next instance of the matched GUID HOB from the starting HOB.
 *
 * @guid:     GUID to search
 * @hob_list: A pointer to the HOB list
 *
 * Return: A HOB object with matching GUID; Otherwise NULL.
 */
const struct hob_header *hob_get_next_guid_hob(const efi_guid_t *guid,
					       const void *hob_list);

/**
 * This function retrieves a GUID HOB data buffer and size.
 *
 * @hob_list:      A HOB list pointer.
 * @len:           A pointer to the GUID HOB data buffer length.
 *                 If the GUID HOB is located, the length will be updated.
 * @guid           A pointer to HOB GUID.
 *
 * Return: NULL:   Failed to find the GUID HOB.
 * Return: others: GUID HOB data buffer pointer.
 */
void *hob_get_guid_hob_data(const void *hob_list, u32 *len,
			    const efi_guid_t *guid);

#endif /* __HOB_H__ */
