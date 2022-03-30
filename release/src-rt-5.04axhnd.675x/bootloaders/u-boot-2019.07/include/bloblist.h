/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * This provides a standard way of passing information between boot phases
 * (TPL -> SPL -> U-Boot proper.)
 *
 * A list of blobs of data, tagged with their owner. The list resides in memory
 * and can be updated by SPL, U-Boot, etc.
 *
 * Copyright 2018 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef __BLOBLIST_H
#define __BLOBLIST_H

enum {
	BLOBLIST_VERSION	= 0,
	BLOBLIST_MAGIC		= 0xb00757a3,
	BLOBLIST_ALIGN		= 16,
};

enum bloblist_tag_t {
	BLOBLISTT_NONE = 0,

	/* Vendor-specific tags are permitted here */
	BLOBLISTT_EC_HOSTEVENT,		/* Chromium OS EC host-event mask */
	BLOBLISTT_SPL_HANDOFF,		/* Hand-off info from SPL */
	BLOBLISTT_VBOOT_CTX,		/* Chromium OS verified boot context */
	BLOBLISTT_VBOOT_HANDOFF,	/* Chromium OS internal handoff info */
};

/**
 * struct bloblist_hdr - header for the bloblist
 *
 * This is stored at the start of the bloblist which is always on a 16-byte
 * boundary. Records follow this header. The bloblist normally stays in the
 * same place in memory as SPL and U-Boot execute, but it can be safely moved
 * around.
 *
 * None of the bloblist structures contain pointers but it is possible to put
 * pointers inside a bloblist record if desired. This is not encouraged,
 * since it can make part of the bloblist inaccessible if the pointer is
 * no-longer valid. It is better to just store all the data inside a bloblist
 * record.
 *
 * Each bloblist record is aligned to a 16-byte boundary and follows immediately
 * from the last.
 *
 * @version: BLOBLIST_VERSION
 * @hdr_size: Size of this header, normally sizeof(struct bloblist_hdr). The
 *	first bloblist_rec starts at this offset from the start of the header
 * @flags: Space for BLOBLISTF_... flags (none yet)
 * @magic: BLOBLIST_MAGIC
 * @size: Total size of all records (non-zero if valid) including this header.
 *	The bloblist extends for this many bytes from the start of this header.
 * @alloced: Total size allocated for this bloblist. When adding new records,
 *	the bloblist can grow up to this size. This starts out as
 *	sizeof(bloblist_hdr) since we need at least that much space to store a
 *	valid bloblist
 * @spare: Space space
 * @chksum: CRC32 for the entire bloblist allocated area. Since any of the
 *	blobs can be altered after being created, this checksum is only valid
 *	when the bloblist is finalised before jumping to the next stage of boot.
 *	Note: @chksum is last to make it easier to exclude it from the checksum
 *	calculation.
 */
struct bloblist_hdr {
	u32 version;
	u32 hdr_size;
	u32 flags;
	u32 magic;

	u32 size;
	u32 alloced;
	u32 spare;
	u32 chksum;
};

/**
 * struct bloblist_rec - record for the bloblist
 *
 * NOTE: Only exported for testing purposes. Do not use this struct.
 *
 * The bloblist contains a number of records each consisting of this record
 * structure followed by the data contained. Each records is 16-byte aligned.
 *
 * @tag: Tag indicating what the record contains
 * @hdr_size: Size of this header, normally sizeof(struct bloblist_rec). The
 *	record's data starts at this offset from the start of the record
 * @size: Size of record in bytes, excluding the header size. This does not
 *	need to be aligned (e.g. 3 is OK).
 * @spare: Spare space for other things
 */
struct bloblist_rec {
	u32 tag;
	u32 hdr_size;
	u32 size;
	u32 spare;
};

/**
 * bloblist_find() - Find a blob
 *
 * Searches the bloblist and returns the blob with the matching tag
 *
 * @tag:	Tag to search for (enum bloblist_tag_t)
 * @size:	Expected size of the blob
 * @return pointer to blob if found, or NULL if not found, or a blob was found
 *	but it is the wrong size
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
 * @return pointer to the newly added block, or NULL if there is not enough
 *	space for the blob
 */
void *bloblist_add(uint tag, int size);

/**
 * bloblist_ensure_size() - Find or add a blob
 *
 * Find an existing blob, or add a new one if not found
 *
 * @tag:	Tag to add (enum bloblist_tag_t)
 * @size:	Size of the blob
 * @blobp:	Returns a pointer to blob on success
 * @return 0 if OK, -ENOSPC if it is missing and could not be added due to lack
 *	of space, or -ESPIPE it exists but has the wrong size
 */
int bloblist_ensure_size(uint tag, int size, void **blobp);

/**
 * bloblist_ensure() - Find or add a blob
 *
 * Find an existing blob, or add a new one if not found
 *
 * @tag:	Tag to add (enum bloblist_tag_t)
 * @size:	Size of the blob
 * @return pointer to blob, or NULL if it is missing and could not be added due
 *	to lack of space, or it exists but has the wrong size
 */
void *bloblist_ensure(uint tag, int size);

/**
 * bloblist_new() - Create a new, empty bloblist of a given size
 *
 * @addr: Address of bloblist
 * @size: Initial size for bloblist
 * @flags: Flags to use for bloblist
 * @return 0 if OK, -EFAULT if addr is not aligned correctly, -ENOSPC is the
 *	area is not large enough
 */
int bloblist_new(ulong addr, uint size, uint flags);

/**
 * bloblist_check() - Check if a bloblist exists
 *
 * @addr: Address of bloblist
 * @size: Expected size of blobsize, or 0 to detect the size
 * @return 0 if OK, -ENOENT if the magic number doesn't match (indicating that
 *	there problem is no bloblist at the given address), -EPROTONOSUPPORT
 *	if the version does not match, -EIO if the checksum does not match,
 *	-EFBIG if the expected size does not match the detected size
 */
int bloblist_check(ulong addr, uint size);

/**
 * bloblist_finish() - Set up the bloblist for the next U-Boot part
 *
 * This sets the correct checksum for the bloblist. This ensures that the
 * bloblist will be detected correctly by the next phase of U-Boot.
 *
 * @return 0
 */
int bloblist_finish(void);

/**
 * bloblist_init() - Init the bloblist system with a single bloblist
 *
 * This uses CONFIG_BLOBLIST_ADDR and CONFIG_BLOBLIST_SIZE to set up a bloblist
 * for use by U-Boot.
 */
int bloblist_init(void);

#endif /* __BLOBLIST_H */
