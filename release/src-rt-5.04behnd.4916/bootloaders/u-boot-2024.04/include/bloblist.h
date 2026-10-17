/* SPDX-License-Identifier: GPL-2.0+ BSD-3-Clause */
/*
 * This provides a standard way of passing information between boot phases
 * (TPL -> SPL -> U-Boot proper.)
 *
 * It consists of a list of blobs of data, tagged with their owner / contents.
 * The list resides in memory and can be updated by SPL, U-Boot, etc.
 *
 * Design goals for bloblist:
 *
 * 1. Small and efficient structure. This avoids UUIDs or 16-byte name fields,
 * since a 32-bit tag provides enough space for all the tags we will even need.
 * If UUIDs are desired, they can be added inside a particular blob.
 *
 * 2. Avoids use of pointers, so the structure can be relocated in memory. The
 * data in each blob is inline, rather than using pointers.
 *
 * 3. Bloblist is designed to start small in TPL or SPL, when only a few things
 * are needed, like the memory size or whether console output should be enabled.
 * Then it can grow in U-Boot proper, e.g. to include space for ACPI tables.
 *
 * 4. The bloblist structure is simple enough that it can be implemented in a
 * small amount of C code. The API does not require use of strings or UUIDs,
 * which would add to code size. For Thumb-2 the code size needed in SPL is
 * approximately 940 bytes (e.g. for chromebook_bob).
 *
 * 5. Bloblist uses 8-byte alignment internally and is designed to start on a
 * 8-byte boundary. Its headers are 8 bytes long. It is possible to achieve
 * larger alignment (e.g. 16 bytes) by adding a dummy header, For use in SPL and
 * TPL the alignment can be relaxed, since it can be relocated to an aligned
 * address in U-Boot proper.
 *
 * 6. Bloblist is designed to be passed to Linux as reserved memory. While linux
 * doesn't understand the bloblist header, it can be passed the indivdual blobs.
 * For example, ACPI tables can reside in a blob and the address of those is
 * passed to Linux, without Linux ever being away of the existence of a
 * bloblist. Having all the blobs contiguous in memory simplifies the
 * reserved-memory space.
 *
 * 7. Bloblist tags are defined in the enum below. There is an area for
 * project-specific stuff (e.g. U-Boot, TF-A) and vendor-specific stuff, e.g.
 * something used only on a particular SoC. There is also a private area for
 * temporary, local use.
 *
 * 8. Bloblist includes a simple checksum, so that each boot phase can update
 * this and allow the next phase to check that all is well. While the bloblist
 * is small, this is quite cheap to calculate. When it grows (e.g. in U-Boot\
 * proper), the CPU is likely running faster, so it is not prohibitive. Having
 * said that, U-Boot is often the last phase that uses bloblist, so calculating
 * the checksum there may not be necessary.
 *
 * 9. It would be possible to extend bloblist to support a non-contiguous
 * structure, e.g. by creating a blob type that points to the next bloblist.
 * This does not seem necessary for now. It adds complexity and code. We can
 * always just copy it.
 *
 * 10. Bloblist is designed for simple structures, those that can be defined by
 * a single C struct. More complex structures should be passed in a device tree.
 * There are some exceptions, chiefly the various binary structures that Intel
 * is fond of creating. But device tree provides a dictionary-type format which
 * is fairly efficient (for use in U-Boot proper and Linux at least), along with
 * a schema and a good set of tools. New formats should be designed around
 * device tree rather than creating new binary formats, unless they are needed
 * early in boot (where libfdt's 3KB of overhead is too large) and are trival
 * enough to be described by a C struct.
 *
 * Copyright 2018 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 * Adjusted July 2023 to match Firmware handoff specification, Release 0.9
 */

#ifndef __BLOBLIST_H
#define __BLOBLIST_H

#include <mapmem.h>

enum {
	BLOBLIST_VERSION	= 1,
	BLOBLIST_MAGIC		= 0x4a0fb10b,

	BLOBLIST_BLOB_ALIGN_LOG2 = 3,
	BLOBLIST_BLOB_ALIGN	 = 1 << BLOBLIST_BLOB_ALIGN_LOG2,

	BLOBLIST_ALIGN_LOG2	= 3,
	BLOBLIST_ALIGN		= 1 << BLOBLIST_ALIGN_LOG2,
};

/* Supported tags - add new ones to tag_name in bloblist.c */
enum bloblist_tag_t {
	BLOBLISTT_VOID = 0,

	/*
	 * Standard area to allocate blobs used across firmware components, for
	 * things that are very commonly used, particularly in multiple
	 * projects.
	 */
	BLOBLISTT_AREA_FIRMWARE_TOP = 0x1,
	/*
	 * Devicetree for use by firmware. On some platforms this is passed to
	 * the OS also
	 */
	BLOBLISTT_CONTROL_FDT = 1,
	BLOBLISTT_HOB_BLOCK = 2,
	BLOBLISTT_HOB_LIST = 3,
	BLOBLISTT_ACPI_TABLES = 4,
	BLOBLISTT_TPM_EVLOG = 5,
	BLOBLISTT_TPM_CRB_BASE = 6,

	/* Standard area to allocate blobs used across firmware components */
	BLOBLISTT_AREA_FIRMWARE = 0x10,
	BLOBLISTT_TPM2_TCG_LOG = 0x10,	/* TPM v2 log space */
	BLOBLISTT_TCPA_LOG = 0x11,	/* TPM log space */
	/*
	 * Advanced Configuration and Power Interface Global Non-Volatile
	 * Sleeping table. This forms part of the ACPI tables passed to Linux.
	 */
	BLOBLISTT_ACPI_GNVS = 0x12,

	/* Standard area to allocate blobs used for Trusted Firmware */
	BLOBLISTT_AREA_TF = 0x100,
	BLOBLISTT_OPTEE_PAGABLE_PART = 0x100,

	/* Other standard area to allocate blobs */
	BLOBLISTT_AREA_OTHER = 0x200,
	BLOBLISTT_INTEL_VBT = 0x200,	/* Intel Video-BIOS table */
	BLOBLISTT_SMBIOS_TABLES = 0x201, /* SMBIOS tables for x86 */
	BLOBLISTT_VBOOT_CTX = 0x202,	/* Chromium OS verified boot context */

	/*
	 * Tags from here are on reserved for private use within a single
	 * firmware binary (i.e. a single executable or phase of a project).
	 * These tags can be passed between binaries within a local
	 * implementation, but cannot be used in upstream code. Allocate a
	 * tag in one of the areas above if you want that.
	 *
	 * Project-specific tags are permitted here. Projects can be open source
	 * or not, but the format of the data must be fuily documented in an
	 * open source project, including all fields, bits, etc. Naming should
	 * be: BLOBLISTT_<project>_<purpose_here>
	 *
	 * Vendor-specific tags are also permitted. Projects can be open source
	 * or not, but the format of the data must be fuily documented in an
	 * open source project, including all fields, bits, etc. Naming should
	 * be BLOBLISTT_<vendor>_<purpose_here>
	 */
	BLOBLISTT_PRIVATE_AREA		= 0xfff000,
	BLOBLISTT_U_BOOT_SPL_HANDOFF	= 0xfff000, /* Hand-off info from SPL */
	BLOBLISTT_VBE			= 0xfff001, /* VBE per-phase state */
	BLOBLISTT_U_BOOT_VIDEO		= 0xfff002, /* Video info from SPL */
};

/**
 * struct bloblist_hdr - header for the bloblist
 *
 * This is stored at the start of the bloblist which is always on a 16-byte
 * boundary. Records follow this header. The bloblist normally stays in the
 * same place in memory as SPL and U-Boot execute, but it can be safely moved
 * around.
 *
 * None of the bloblist headers themselves contain pointers but it is possible
 * to put pointers inside a bloblist record if desired. This is not encouraged,
 * since it can make part of the bloblist inaccessible if the pointer is
 * no-longer valid. It is better to just store all the data inside a bloblist
 * record.
 *
 * Each bloblist record is aligned to a 16-byte boundary and follows immediately
 * from the last.
 *
 * @magic: BLOBLIST_MAGIC
 * @chksum: checksum for the entire bloblist allocated area. Since any of the
 *	blobs can be altered after being created, this checksum is only valid
 *	when the bloblist is finalized before jumping to the next stage of boot.
 *	This is the value needed to make all checksummed bytes sum to 0
 * @version: BLOBLIST_VERSION
 * @hdr_size: Size of this header, normally sizeof(struct bloblist_hdr). The
 *	first bloblist_rec starts at this offset from the start of the header
 * @align_log2: Power of two of the maximum alignment required by this list
 * @used_size: Size allocated so far for this bloblist. This starts out as
 *	sizeof(bloblist_hdr) since we need at least that much space to store a
 *	valid bloblist
 * @total_size: The number of total bytes that the bloblist can occupy.
 *	Any blob producer must check if there is sufficient space before adding
 *	a record to the bloblist.
 * @flags: Space for BLOBLISTF... flags (none yet)
 * @spare: Spare space (for future use)
 */
struct bloblist_hdr {
	u32 magic;
	u8 chksum;
	u8 version;
	u8 hdr_size;
	u8 align_log2;
	u32 used_size;
	u32 total_size;
	u32 flags;
	u32 spare;
};

/**
 * struct bloblist_rec - record for the bloblist
 *
 * The bloblist contains a number of records each consisting of this record
 * structure followed by the data contained. Each records is 16-byte aligned.
 *
 * NOTE: Only exported for testing purposes. Do not use this struct.
 *
 * @tag_and_hdr_size: Tag indicating what the record contains (bottom 24 bits), and
 *	size of this header (top 8 bits), normally sizeof(struct bloblist_rec).
 *	The record's data starts at this offset from the start of the record
 * @size: Size of record in bytes, excluding the header size. This does not
 *	need to be aligned (e.g. 3 is OK).
 */
struct bloblist_rec {
	u32 tag_and_hdr_size;
	u32 size;
};

enum {
	BLOBLISTR_TAG_SHIFT		= 0,
	BLOBLISTR_TAG_MASK		= 0xffffffU << BLOBLISTR_TAG_SHIFT,
	BLOBLISTR_HDR_SIZE_SHIFT	= 24,
	BLOBLISTR_HDR_SIZE_MASK		= 0xffU << BLOBLISTR_HDR_SIZE_SHIFT,

	BLOBLIST_HDR_SIZE		= sizeof(struct bloblist_hdr),
	BLOBLIST_REC_HDR_SIZE		= sizeof(struct bloblist_rec),
};

/**
 * bloblist_check_magic() - return a bloblist if the magic matches
 *
 * @addr: Address to check
 * Return: pointer to bloblist, if the magic matches, else NULL
 */
static inline void *bloblist_check_magic(ulong addr)
{
	u32 *ptr;

	if (!addr)
		return NULL;
	ptr = map_sysmem(addr, 0);
	if (*ptr != BLOBLIST_MAGIC)
		return NULL;

	return ptr;
}

/**
 * bloblist_find() - Find a blob
 *
 * Searches the bloblist and returns the blob with the matching tag
 *
 * @tag:	Tag to search for (enum bloblist_tag_t)
 * @size:	Expected size of the blob, or 0 for any size
 * Return: pointer to blob if found, or NULL if not found, or a blob was found
 * but it is the wrong size
 */
void *bloblist_find(uint tag, int size);

/**
 * bloblist_add() - Add a new blob
 *
 * Add a new blob to the bloblist
 *
 * This should only be called if you konw there is no existing blob for a
 * particular tag. It is typically safe to call in the first phase of U-Boot
 * (e.g. TPL or SPL). After that, bloblist_ensure() should be used instead.
 *
 * @tag:	Tag to add (enum bloblist_tag_t)
 * @size:	Size of the blob
 * @align_log2:	Alignment of the blob (in bytes log2), 0 for default
 * Return: pointer to the newly added block, or NULL if there is not enough
 * space for the blob
 */
void *bloblist_add(uint tag, int size, int align_log2);

/**
 * bloblist_ensure_size() - Find or add a blob
 *
 * Find an existing blob, or add a new one if not found
 *
 * @tag:	Tag to add (enum bloblist_tag_t)
 * @size:	Size of the blob
 * @blobp:	Returns a pointer to blob on success
 * @align_log2:	Alignment of the blob (in bytes log2), 0 for default
 * Return: 0 if OK, -ENOSPC if it is missing and could not be added due to lack
 * of space, or -ESPIPE it exists but has the wrong size
 */
int bloblist_ensure_size(uint tag, int size, int align_log2, void **blobp);

/**
 * bloblist_ensure() - Find or add a blob
 *
 * Find an existing blob, or add a new one if not found
 *
 * @tag:	Tag to add (enum bloblist_tag_t)
 * @size:	Size of the blob
 * Return: pointer to blob, or NULL if it is missing and could not be added due
 * to lack of space, or it exists but has the wrong size
 */
void *bloblist_ensure(uint tag, int size);

/**
 * bloblist_ensure_size_ret() - Find or add a blob
 *
 * Find an existing blob, or add a new one if not found
 *
 * @tag:	Tag to add (enum bloblist_tag_t)
 * @sizep:	Size of the blob to create; returns size of actual blob
 * @blobp:	Returns a pointer to blob on success
 * Return: 0 if OK, -ENOSPC if it is missing and could not be added due to lack
 * of space
 */
int bloblist_ensure_size_ret(uint tag, int *sizep, void **blobp);

/**
 * bloblist_resize() - resize a blob
 *
 * Any blobs above this one are relocated up or down. The resized blob remains
 * in the same place.
 *
 * @tag:	Tag to add (enum bloblist_tag_t)
 * @new_size:	New size of the blob (>0 to expand, <0 to contract)
 * Return: 0 if OK, -ENOSPC if the bloblist does not have enough space, -ENOENT
 * if the tag is not found
 */
int bloblist_resize(uint tag, int new_size);

/**
 * bloblist_new() - Create a new, empty bloblist of a given size
 *
 * @addr: Address of bloblist
 * @size: Initial size for bloblist
 * @flags: Flags to use for bloblist
 * @align_log2: Log base 2 of maximum alignment provided by this bloblist
 * Return: 0 if OK, -EFAULT if addr is not aligned correctly, -ENOSPC is the
 * area is not large enough
 */
int bloblist_new(ulong addr, uint size, uint flags, uint align_log2);

/**
 * bloblist_check() - Check if a bloblist exists
 *
 * @addr: Address of bloblist
 * @size: Expected size of blobsize, or 0 to detect the size
 * Return: 0 if OK, -ENOENT if the magic number doesn't match (indicating that
 * there problem is no bloblist at the given address), -EPROTONOSUPPORT
 * if the version does not match, -EIO if the checksum does not match,
 * -EFBIG if the expected size does not match the detected size, -ENOSPC
 * if the size is not large enough to hold the headers
 */
int bloblist_check(ulong addr, uint size);

/**
 * bloblist_finish() - Set up the bloblist for the next U-Boot part
 *
 * This sets the correct checksum for the bloblist. This ensures that the
 * bloblist will be detected correctly by the next phase of U-Boot.
 *
 * Return: 0
 */
int bloblist_finish(void);

/**
 * bloblist_get_stats() - Get information about the bloblist
 *
 * This returns useful information about the bloblist
 *
 * @basep: Returns base address of bloblist
 * @tsizep: Returns the total number of bytes of the bloblist
 * @usizep: Returns the number of used bytes of the bloblist
 */
void bloblist_get_stats(ulong *basep, ulong *tsizep, ulong *usizep);

/**
 * bloblist_get_base() - Get the base address of the bloblist
 *
 * Return: base address of bloblist
 */
ulong bloblist_get_base(void);

/**
 * bloblist_get_size() - Get the size of the bloblist
 *
 * Return: the size in bytes
 */
ulong bloblist_get_size(void);

/**
 * bloblist_get_total_size() - Get the total size of the bloblist
 *
 * Return: the size in bytes
 */
ulong bloblist_get_total_size(void);

/**
 * bloblist_show_stats() - Show information about the bloblist
 *
 * This shows useful information about the bloblist on the console
 */
void bloblist_show_stats(void);

/**
 * bloblist_show_list() - Show a list of blobs in the bloblist
 *
 * This shows a list of blobs, showing their address, size and tag.
 */
void bloblist_show_list(void);

/**
 * bloblist_tag_name() - Get the name for a tag
 *
 * @tag: Tag to check
 * Return: name of tag, or "invalid" if an invalid tag is provided
 */
const char *bloblist_tag_name(enum bloblist_tag_t tag);

/**
 * bloblist_reloc() - Relocate the bloblist and optionally resize it
 *
 * @to: Pointer to new bloblist location (must not overlap old location)
 * @to_size: New size for bloblist (must be larger than from_size)
 * @from: Pointer to bloblist to relocate
 * @from_size: Size of bloblist to relocate
 */
void bloblist_reloc(void *to, uint to_size, void *from, uint from_size);

/**
 * bloblist_init() - Init the bloblist system with a single bloblist
 *
 * This locates and sets up the blocklist for use.
 *
 * If CONFIG_BLOBLIST_FIXED is selected, it uses CONFIG_BLOBLIST_ADDR and
 * CONFIG_BLOBLIST_SIZE to set up a bloblist for use by U-Boot.
 *
 * If CONFIG_BLOBLIST_ALLOC is selected, it allocates memory for a bloblist of
 * size CONFIG_BLOBLIST_SIZE.
 *
 * If CONFIG_BLOBLIST_PASSAGE is selected, it uses the bloblist in the incoming
 * standard passage. The size is detected automatically so CONFIG_BLOBLIST_SIZE
 * can be 0.
 *
 * Sets GD_FLG_BLOBLIST_READY in global_data flags on success
 *
 * Return: 0 if OK, -ve on error
 */
int bloblist_init(void);

#if CONFIG_IS_ENABLED(BLOBLIST)
/**
 * bloblist_maybe_init() - Init the bloblist system if not already done
 *
 * Calls bloblist_init() if the GD_FLG_BLOBLIST_READY flag is not et
 *
 * Return: 0 if OK, -ve on error
 */
int bloblist_maybe_init(void);
#else
static inline int bloblist_maybe_init(void)
{
	return 0;
}
#endif /* BLOBLIST */

#endif /* __BLOBLIST_H */
