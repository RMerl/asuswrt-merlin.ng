/*
 * cramfs.c
 *
 * Copyright (C) 1999 Linus Torvalds
 *
 * Copyright (C) 2000-2002 Transmeta Corporation
 *
 * Copyright (C) 2003 Kai-Uwe Bloem,
 * Auerswald GmbH & Co KG, <linux-development@auerswald.de>
 * - adapted from the www.tuxbox.org u-boot tree, added "ls" command
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License (Version 2) as
 * published by the Free Software Foundation.
 *
 * Compressed ROM filesystem for Linux.
 *
 * TODO:
 * add support for resolving symbolic links
 */

/*
 * These are the VFS interfaces to the compressed ROM filesystem.
 * The actual compression is based on zlib, see the other files.
 */

#include <common.h>
#include <malloc.h>
#include <asm/byteorder.h>
#include <linux/stat.h>
#include <jffs2/jffs2.h>
#include <jffs2/load_kernel.h>
#include <cramfs/cramfs_fs.h>

/* These two macros may change in future, to provide better st_ino
   semantics. */
#define CRAMINO(x)	(CRAMFS_GET_OFFSET(x) ? CRAMFS_GET_OFFSET(x)<<2 : 1)
#define OFFSET(x)	((x)->i_ino)

struct cramfs_super super;

/* CPU address space offset calculation macro, struct part_info offset is
 * device address space offset, so we need to shift it by a device start address. */
#if defined(CONFIG_MTD_NOR_FLASH)
extern flash_info_t flash_info[];
#define PART_OFFSET(x)	((ulong)x->offset + \
			 flash_info[x->dev->id->num].start[0])
#else
#define PART_OFFSET(x)	((ulong)x->offset)
#endif

static int cramfs_uncompress (unsigned long begin, unsigned long offset,
			      unsigned long loadoffset);

static int cramfs_read_super (struct part_info *info)
{
	unsigned long root_offset;

	/* Read the first block and get the superblock from it */
	memcpy (&super, (void *) PART_OFFSET(info), sizeof (super));

	/* Do sanity checks on the superblock */
	if (super.magic != CRAMFS_32 (CRAMFS_MAGIC)) {
		/* check at 512 byte offset */
		memcpy (&super, (void *) PART_OFFSET(info) + 512, sizeof (super));
		if (super.magic != CRAMFS_32 (CRAMFS_MAGIC)) {
			printf ("cramfs: wrong magic\n");
			return -1;
		}
	}

	/* flags is reused several times, so swab it once */
	super.flags = CRAMFS_32 (super.flags);
	super.size = CRAMFS_32 (super.size);

	/* get feature flags first */
	if (super.flags & ~CRAMFS_SUPPORTED_FLAGS) {
		printf ("cramfs: unsupported filesystem features\n");
		return -1;
	}

	/* Check that the root inode is in a sane state */
	if (!S_ISDIR (CRAMFS_16 (super.root.mode))) {
		printf ("cramfs: root is not a directory\n");
		return -1;
	}
	root_offset = CRAMFS_GET_OFFSET (&(super.root)) << 2;
	if (root_offset == 0) {
		printf ("cramfs: empty filesystem");
	} else if (!(super.flags & CRAMFS_FLAG_SHIFTED_ROOT_OFFSET) &&
		   ((root_offset != sizeof (struct cramfs_super)) &&
		    (root_offset != 512 + sizeof (struct cramfs_super)))) {
		printf ("cramfs: bad root offset %lu\n", root_offset);
		return -1;
	}

	return 0;
}

/* Unpack to an allocated buffer, trusting in the inode's size field. */
static char *cramfs_uncompress_link (unsigned long begin, unsigned long offset)
{
	struct cramfs_inode *inode = (struct cramfs_inode *)(begin + offset);
	unsigned long size = CRAMFS_24 (inode->size);
	char *link = malloc (size + 1);

	if (!link || cramfs_uncompress (begin, offset, (unsigned long)link) != size) {
		free (link);
		link = NULL;
	} else {
		link[size] = '\0';
	}
	return link;
}

static unsigned long cramfs_resolve (unsigned long begin, unsigned long offset,
				     unsigned long size, int raw,
				     char *filename)
{
	unsigned long inodeoffset = 0, nextoffset;

	while (inodeoffset < size) {
		struct cramfs_inode *inode;
		char *name;
		int namelen;

		inode = (struct cramfs_inode *) (begin + offset +
						 inodeoffset);

		/*
		 * Namelengths on disk are shifted by two
		 * and the name padded out to 4-byte boundaries
		 * with zeroes.
		 */
		namelen = CRAMFS_GET_NAMELEN (inode) << 2;
		name = (char *) inode + sizeof (struct cramfs_inode);

		nextoffset =
			inodeoffset + sizeof (struct cramfs_inode) + namelen;

		for (;;) {
			if (!namelen)
				return -1;
			if (name[namelen - 1])
				break;
			namelen--;
		}

		if (!strncmp(filename, name, namelen) &&
		    (namelen == strlen(filename))) {
			char *p = strtok (NULL, "/");

			if (raw && (p == NULL || *p == '\0'))
				return offset + inodeoffset;

			if (S_ISDIR (CRAMFS_16 (inode->mode))) {
				return cramfs_resolve (begin,
						       CRAMFS_GET_OFFSET
						       (inode) << 2,
						       CRAMFS_24 (inode->
								  size), raw,
						       p);
			} else if (S_ISREG (CRAMFS_16 (inode->mode))) {
				return offset + inodeoffset;
			} else if (S_ISLNK (CRAMFS_16 (inode->mode))) {
				unsigned long ret;
				char *link;
				if (p && strlen(p)) {
					printf ("unsupported symlink to \
						 non-terminal path\n");
					return 0;
				}
				link = cramfs_uncompress_link (begin,
						offset + inodeoffset);
				if (!link) {
					printf ("%*.*s: Error reading link\n",
						namelen, namelen, name);
					return 0;
				} else if (link[0] == '/') {
					printf ("unsupported symlink to \
						 absolute path\n");
					free (link);
					return 0;
				}
				ret = cramfs_resolve (begin,
						      offset,
						      size,
						      raw,
						      strtok(link, "/"));
				free (link);
				return ret;
			} else {
				printf ("%*.*s: unsupported file type (%x)\n",
					namelen, namelen, name,
					CRAMFS_16 (inode->mode));
				return 0;
			}
		}

		inodeoffset = nextoffset;
	}

	printf ("can't find corresponding entry\n");
	return 0;
}

static int cramfs_uncompress (unsigned long begin, unsigned long offset,
			      unsigned long loadoffset)
{
	struct cramfs_inode *inode = (struct cramfs_inode *) (begin + offset);
	u32 *block_ptrs = (u32 *)
		(begin + (CRAMFS_GET_OFFSET (inode) << 2));
	unsigned long curr_block = (CRAMFS_GET_OFFSET (inode) +
				    (((CRAMFS_24 (inode->size)) +
				      4095) >> 12)) << 2;
	int size, total_size = 0;
	int i;

	cramfs_uncompress_init ();

	for (i = 0; i < ((CRAMFS_24 (inode->size) + 4095) >> 12); i++) {
		size = cramfs_uncompress_block ((void *) loadoffset,
						(void *) (begin + curr_block),
						(CRAMFS_32 (block_ptrs[i]) -
						 curr_block));
		if (size < 0)
			return size;
		loadoffset += size;
		total_size += size;
		curr_block = CRAMFS_32 (block_ptrs[i]);
	}

	cramfs_uncompress_exit ();
	return total_size;
}

int cramfs_load (char *loadoffset, struct part_info *info, char *filename)
{
	unsigned long offset;

	if (cramfs_read_super (info))
		return -1;

	offset = cramfs_resolve (PART_OFFSET(info),
				 CRAMFS_GET_OFFSET (&(super.root)) << 2,
				 CRAMFS_24 (super.root.size), 0,
				 strtok (filename, "/"));

	if (offset <= 0)
		return offset;

	return cramfs_uncompress (PART_OFFSET(info), offset,
				  (unsigned long) loadoffset);
}

static int cramfs_list_inode (struct part_info *info, unsigned long offset)
{
	struct cramfs_inode *inode = (struct cramfs_inode *)
		(PART_OFFSET(info) + offset);
	char *name, str[20];
	int namelen, nextoff;

	/*
	 * Namelengths on disk are shifted by two
	 * and the name padded out to 4-byte boundaries
	 * with zeroes.
	 */
	namelen = CRAMFS_GET_NAMELEN (inode) << 2;
	name = (char *) inode + sizeof (struct cramfs_inode);
	nextoff = namelen;

	for (;;) {
		if (!namelen)
			return namelen;
		if (name[namelen - 1])
			break;
		namelen--;
	}

	printf (" %s %8d %*.*s", mkmodestr (CRAMFS_16 (inode->mode), str),
		CRAMFS_24 (inode->size), namelen, namelen, name);

	if ((CRAMFS_16 (inode->mode) & S_IFMT) == S_IFLNK) {
		char *link = cramfs_uncompress_link (PART_OFFSET(info), offset);
		if (link)
			printf (" -> %s\n", link);
		else
			printf (" [Error reading link]\n");
		free (link);
	} else
		printf ("\n");

	return nextoff;
}

int cramfs_ls (struct part_info *info, char *filename)
{
	struct cramfs_inode *inode;
	unsigned long inodeoffset = 0, nextoffset;
	unsigned long offset, size;

	if (cramfs_read_super (info))
		return -1;

	if (strlen (filename) == 0 || !strcmp (filename, "/")) {
		/* Root directory. Use root inode in super block */
		offset = CRAMFS_GET_OFFSET (&(super.root)) << 2;
		size = CRAMFS_24 (super.root.size);
	} else {
		/* Resolve the path */
		offset = cramfs_resolve (PART_OFFSET(info),
					 CRAMFS_GET_OFFSET (&(super.root)) <<
					 2, CRAMFS_24 (super.root.size), 1,
					 strtok (filename, "/"));

		if (offset <= 0)
			return offset;

		/* Resolving was successful. Examine the inode */
		inode = (struct cramfs_inode *) (PART_OFFSET(info) + offset);
		if (!S_ISDIR (CRAMFS_16 (inode->mode))) {
			/* It's not a directory - list it, and that's that */
			return (cramfs_list_inode (info, offset) > 0);
		}

		/* It's a directory. List files within */
		offset = CRAMFS_GET_OFFSET (inode) << 2;
		size = CRAMFS_24 (inode->size);
	}

	/* List the given directory */
	while (inodeoffset < size) {
		inode = (struct cramfs_inode *) (PART_OFFSET(info) + offset +
						 inodeoffset);

		nextoffset = cramfs_list_inode (info, offset + inodeoffset);
		if (nextoffset == 0)
			break;
		inodeoffset += sizeof (struct cramfs_inode) + nextoffset;
	}

	return 1;
}

int cramfs_info (struct part_info *info)
{
	if (cramfs_read_super (info))
		return 0;

	printf ("size: 0x%x (%u)\n", super.size, super.size);

	if (super.flags != 0) {
		printf ("flags:\n");
		if (super.flags & CRAMFS_FLAG_FSID_VERSION_2)
			printf ("\tFSID version 2\n");
		if (super.flags & CRAMFS_FLAG_SORTED_DIRS)
			printf ("\tsorted dirs\n");
		if (super.flags & CRAMFS_FLAG_HOLES)
			printf ("\tholes\n");
		if (super.flags & CRAMFS_FLAG_SHIFTED_ROOT_OFFSET)
			printf ("\tshifted root offset\n");
	}

	printf ("fsid:\n\tcrc: 0x%x\n\tedition: 0x%x\n",
		super.fsid.crc, super.fsid.edition);
	printf ("name: %16s\n", super.name);

	return 1;
}

int cramfs_check (struct part_info *info)
{
	struct cramfs_super *sb;

	if (info->dev->id->type != MTD_DEV_TYPE_NOR)
		return 0;

	sb = (struct cramfs_super *) PART_OFFSET(info);
	if (sb->magic != CRAMFS_32 (CRAMFS_MAGIC)) {
		/* check at 512 byte offset */
		sb = (struct cramfs_super *) (PART_OFFSET(info) + 512);
		if (sb->magic != CRAMFS_32 (CRAMFS_MAGIC))
			return 0;
	}
	return 1;
}
