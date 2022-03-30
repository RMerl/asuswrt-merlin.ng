/*
-------------------------------------------------------------------------
 * Filename:      jffs2.c
 * Version:       $Id: jffs2_1pass.c,v 1.7 2002/01/25 01:56:47 nyet Exp $
 * Copyright:     Copyright (C) 2001, Russ Dill
 * Author:        Russ Dill <Russ.Dill@asu.edu>
 * Description:   Module to load kernel from jffs2
 *-----------------------------------------------------------------------*/
/*
 * some portions of this code are taken from jffs2, and as such, the
 * following copyright notice is included.
 *
 * JFFS2 -- Journalling Flash File System, Version 2.
 *
 * Copyright (C) 2001 Red Hat, Inc.
 *
 * Created by David Woodhouse <dwmw2@cambridge.redhat.com>
 *
 * The original JFFS, from which the design for JFFS2 was derived,
 * was designed and implemented by Axis Communications AB.
 *
 * The contents of this file are subject to the Red Hat eCos Public
 * License Version 1.1 (the "Licence"); you may not use this file
 * except in compliance with the Licence.  You may obtain a copy of
 * the Licence at http://www.redhat.com/
 *
 * Software distributed under the Licence is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.
 * See the Licence for the specific language governing rights and
 * limitations under the Licence.
 *
 * The Original Code is JFFS2 - Journalling Flash File System, version 2
 *
 * Alternatively, the contents of this file may be used under the
 * terms of the GNU General Public License version 2 (the "GPL"), in
 * which case the provisions of the GPL are applicable instead of the
 * above.  If you wish to allow the use of your version of this file
 * only under the terms of the GPL and not to allow others to use your
 * version of this file under the RHEPL, indicate your decision by
 * deleting the provisions above and replace them with the notice and
 * other provisions required by the GPL.  If you do not delete the
 * provisions above, a recipient may use your version of this file
 * under either the RHEPL or the GPL.
 *
 * $Id: jffs2_1pass.c,v 1.7 2002/01/25 01:56:47 nyet Exp $
 *
 */

/* Ok, so anyone who knows the jffs2 code will probably want to get a papar
 * bag to throw up into before reading this code. I looked through the jffs2
 * code, the caching scheme is very elegant. I tried to keep the version
 * for a bootloader as small and simple as possible. Instead of worring about
 * unneccesary data copies, node scans, etc, I just optimized for the known
 * common case, a kernel, which looks like:
 *	(1) most pages are 4096 bytes
 *	(2) version numbers are somewhat sorted in acsending order
 *	(3) multiple compressed blocks making up one page is uncommon
 *
 * So I create a linked list of decending version numbers (insertions at the
 * head), and then for each page, walk down the list, until a matching page
 * with 4096 bytes is found, and then decompress the watching pages in
 * reverse order.
 *
 */

/*
 * Adapted by Nye Liu <nyet@zumanetworks.com> and
 * Rex Feany <rfeany@zumanetworks.com>
 * on Jan/2002 for U-Boot.
 *
 * Clipped out all the non-1pass functions, cleaned up warnings,
 * wrappers, etc. No major changes to the code.
 * Please, he really means it when he said have a paper bag
 * handy. We needed it ;).
 *
 */

/*
 * Bugfixing by Kai-Uwe Bloem <kai-uwe.bloem@auerswald.de>, (C) Mar/2003
 *
 * - overhaul of the memory management. Removed much of the "paper-bagging"
 *   in that part of the code, fixed several bugs, now frees memory when
 *   partition is changed.
 *   It's still ugly :-(
 * - fixed a bug in jffs2_1pass_read_inode where the file length calculation
 *   was incorrect. Removed a bit of the paper-bagging as well.
 * - removed double crc calculation for fragment headers in jffs2_private.h
 *   for speedup.
 * - scan_empty rewritten in a more "standard" manner (non-paperbag, that is).
 * - spinning wheel now spins depending on how much memory has been scanned
 * - lots of small changes all over the place to "improve" readability.
 * - implemented fragment sorting to ensure that the newest data is copied
 *   if there are multiple copies of fragments for a certain file offset.
 *
 * The fragment sorting feature must be enabled by CONFIG_SYS_JFFS2_SORT_FRAGMENTS.
 * Sorting is done while adding fragments to the lists, which is more or less a
 * bubble sort. This takes a lot of time, and is most probably not an issue if
 * the boot filesystem is always mounted readonly.
 *
 * You should define it if the boot filesystem is mounted writable, and updates
 * to the boot files are done by copying files to that filesystem.
 *
 *
 * There's a big issue left: endianess is completely ignored in this code. Duh!
 *
 *
 * You still should have paper bags at hand :-(. The code lacks more or less
 * any comment, and is still arcane and difficult to read in places. As this
 * might be incompatible with any new code from the jffs2 maintainers anyway,
 * it should probably be dumped and replaced by something like jffs2reader!
 */


#include <common.h>
#include <config.h>
#include <malloc.h>
#include <div64.h>
#include <linux/compiler.h>
#include <linux/stat.h>
#include <linux/time.h>
#include <watchdog.h>
#include <jffs2/jffs2.h>
#include <jffs2/jffs2_1pass.h>
#include <linux/compat.h>
#include <linux/errno.h>

#include "jffs2_private.h"


#define	NODE_CHUNK	1024	/* size of memory allocation chunk in b_nodes */
#define	SPIN_BLKSIZE	18	/* spin after having scanned 1<<BLKSIZE bytes */

/* Debugging switches */
#undef	DEBUG_DIRENTS		/* print directory entry list after scan */
#undef	DEBUG_FRAGMENTS		/* print fragment list after scan */
#undef	DEBUG			/* enable debugging messages */


#ifdef  DEBUG
# define DEBUGF(fmt,args...)	printf(fmt ,##args)
#else
# define DEBUGF(fmt,args...)
#endif

#include "summary.h"

/* keeps pointer to currentlu processed partition */
static struct part_info *current_part;

#if (defined(CONFIG_JFFS2_NAND) && \
     defined(CONFIG_CMD_NAND) )
#include <nand.h>
/*
 * Support for jffs2 on top of NAND-flash
 *
 * NAND memory isn't mapped in processor's address space,
 * so data should be fetched from flash before
 * being processed. This is exactly what functions declared
 * here do.
 *
 */

#define NAND_PAGE_SIZE 512
#define NAND_PAGE_SHIFT 9
#define NAND_PAGE_MASK (~(NAND_PAGE_SIZE-1))

#ifndef NAND_CACHE_PAGES
#define NAND_CACHE_PAGES 16
#endif
#define NAND_CACHE_SIZE (NAND_CACHE_PAGES*NAND_PAGE_SIZE)

static u8* nand_cache = NULL;
static u32 nand_cache_off = (u32)-1;

static int read_nand_cached(u32 off, u32 size, u_char *buf)
{
	struct mtdids *id = current_part->dev->id;
	struct mtd_info *mtd;
	u32 bytes_read = 0;
	size_t retlen;
	int cpy_bytes;

	mtd = get_nand_dev_by_index(id->num);
	if (!mtd)
		return -1;

	while (bytes_read < size) {
		if ((off + bytes_read < nand_cache_off) ||
		    (off + bytes_read >= nand_cache_off+NAND_CACHE_SIZE)) {
			nand_cache_off = (off + bytes_read) & NAND_PAGE_MASK;
			if (!nand_cache) {
				/* This memory never gets freed but 'cause
				   it's a bootloader, nobody cares */
				nand_cache = malloc(NAND_CACHE_SIZE);
				if (!nand_cache) {
					printf("read_nand_cached: can't alloc cache size %d bytes\n",
					       NAND_CACHE_SIZE);
					return -1;
				}
			}

			retlen = NAND_CACHE_SIZE;
			if (nand_read(mtd, nand_cache_off,
				      &retlen, nand_cache) < 0 ||
					retlen != NAND_CACHE_SIZE) {
				printf("read_nand_cached: error reading nand off %#x size %d bytes\n",
						nand_cache_off, NAND_CACHE_SIZE);
				return -1;
			}
		}
		cpy_bytes = nand_cache_off + NAND_CACHE_SIZE - (off + bytes_read);
		if (cpy_bytes > size - bytes_read)
			cpy_bytes = size - bytes_read;
		memcpy(buf + bytes_read,
		       nand_cache + off + bytes_read - nand_cache_off,
		       cpy_bytes);
		bytes_read += cpy_bytes;
	}
	return bytes_read;
}

static void *get_fl_mem_nand(u32 off, u32 size, void *ext_buf)
{
	u_char *buf = ext_buf ? (u_char*)ext_buf : (u_char*)malloc(size);

	if (NULL == buf) {
		printf("get_fl_mem_nand: can't alloc %d bytes\n", size);
		return NULL;
	}
	if (read_nand_cached(off, size, buf) < 0) {
		if (!ext_buf)
			free(buf);
		return NULL;
	}

	return buf;
}

static void *get_node_mem_nand(u32 off, void *ext_buf)
{
	struct jffs2_unknown_node node;
	void *ret = NULL;

	if (NULL == get_fl_mem_nand(off, sizeof(node), &node))
		return NULL;

	if (!(ret = get_fl_mem_nand(off, node.magic ==
			       JFFS2_MAGIC_BITMASK ? node.totlen : sizeof(node),
			       ext_buf))) {
		printf("off = %#x magic %#x type %#x node.totlen = %d\n",
		       off, node.magic, node.nodetype, node.totlen);
	}
	return ret;
}

static void put_fl_mem_nand(void *buf)
{
	free(buf);
}
#endif

#if defined(CONFIG_CMD_ONENAND)

#include <linux/mtd/mtd.h>
#include <linux/mtd/onenand.h>
#include <onenand_uboot.h>

#define ONENAND_PAGE_SIZE 2048
#define ONENAND_PAGE_SHIFT 11
#define ONENAND_PAGE_MASK (~(ONENAND_PAGE_SIZE-1))

#ifndef ONENAND_CACHE_PAGES
#define ONENAND_CACHE_PAGES 4
#endif
#define ONENAND_CACHE_SIZE (ONENAND_CACHE_PAGES*ONENAND_PAGE_SIZE)

static u8* onenand_cache;
static u32 onenand_cache_off = (u32)-1;

static int read_onenand_cached(u32 off, u32 size, u_char *buf)
{
	u32 bytes_read = 0;
	size_t retlen;
	int cpy_bytes;

	while (bytes_read < size) {
		if ((off + bytes_read < onenand_cache_off) ||
		    (off + bytes_read >= onenand_cache_off + ONENAND_CACHE_SIZE)) {
			onenand_cache_off = (off + bytes_read) & ONENAND_PAGE_MASK;
			if (!onenand_cache) {
				/* This memory never gets freed but 'cause
				   it's a bootloader, nobody cares */
				onenand_cache = malloc(ONENAND_CACHE_SIZE);
				if (!onenand_cache) {
					printf("read_onenand_cached: can't alloc cache size %d bytes\n",
					       ONENAND_CACHE_SIZE);
					return -1;
				}
			}

			retlen = ONENAND_CACHE_SIZE;
			if (onenand_read(&onenand_mtd, onenand_cache_off, retlen,
						&retlen, onenand_cache) < 0 ||
					retlen != ONENAND_CACHE_SIZE) {
				printf("read_onenand_cached: error reading nand off %#x size %d bytes\n",
					onenand_cache_off, ONENAND_CACHE_SIZE);
				return -1;
			}
		}
		cpy_bytes = onenand_cache_off + ONENAND_CACHE_SIZE - (off + bytes_read);
		if (cpy_bytes > size - bytes_read)
			cpy_bytes = size - bytes_read;
		memcpy(buf + bytes_read,
		       onenand_cache + off + bytes_read - onenand_cache_off,
		       cpy_bytes);
		bytes_read += cpy_bytes;
	}
	return bytes_read;
}

static void *get_fl_mem_onenand(u32 off, u32 size, void *ext_buf)
{
	u_char *buf = ext_buf ? (u_char *)ext_buf : (u_char *)malloc(size);

	if (NULL == buf) {
		printf("get_fl_mem_onenand: can't alloc %d bytes\n", size);
		return NULL;
	}
	if (read_onenand_cached(off, size, buf) < 0) {
		if (!ext_buf)
			free(buf);
		return NULL;
	}

	return buf;
}

static void *get_node_mem_onenand(u32 off, void *ext_buf)
{
	struct jffs2_unknown_node node;
	void *ret = NULL;

	if (NULL == get_fl_mem_onenand(off, sizeof(node), &node))
		return NULL;

	ret = get_fl_mem_onenand(off, node.magic ==
			JFFS2_MAGIC_BITMASK ? node.totlen : sizeof(node),
			ext_buf);
	if (!ret) {
		printf("off = %#x magic %#x type %#x node.totlen = %d\n",
		       off, node.magic, node.nodetype, node.totlen);
	}
	return ret;
}


static void put_fl_mem_onenand(void *buf)
{
	free(buf);
}
#endif


#if defined(CONFIG_CMD_FLASH)
/*
 * Support for jffs2 on top of NOR-flash
 *
 * NOR flash memory is mapped in processor's address space,
 * just return address.
 */
static inline void *get_fl_mem_nor(u32 off, u32 size, void *ext_buf)
{
	u32 addr = off;
	struct mtdids *id = current_part->dev->id;

	extern flash_info_t flash_info[];
	flash_info_t *flash = &flash_info[id->num];

	addr += flash->start[0];
	if (ext_buf) {
		memcpy(ext_buf, (void *)addr, size);
		return ext_buf;
	}
	return (void*)addr;
}

static inline void *get_node_mem_nor(u32 off, void *ext_buf)
{
	struct jffs2_unknown_node *pNode;

	/* pNode will point directly to flash - don't provide external buffer
	   and don't care about size */
	pNode = get_fl_mem_nor(off, 0, NULL);
	return (void *)get_fl_mem_nor(off, pNode->magic == JFFS2_MAGIC_BITMASK ?
			pNode->totlen : sizeof(*pNode), ext_buf);
}
#endif


/*
 * Generic jffs2 raw memory and node read routines.
 *
 */
static inline void *get_fl_mem(u32 off, u32 size, void *ext_buf)
{
	struct mtdids *id = current_part->dev->id;

	switch(id->type) {
#if defined(CONFIG_CMD_FLASH)
	case MTD_DEV_TYPE_NOR:
		return get_fl_mem_nor(off, size, ext_buf);
		break;
#endif
#if defined(CONFIG_JFFS2_NAND) && defined(CONFIG_CMD_NAND)
	case MTD_DEV_TYPE_NAND:
		return get_fl_mem_nand(off, size, ext_buf);
		break;
#endif
#if defined(CONFIG_CMD_ONENAND)
	case MTD_DEV_TYPE_ONENAND:
		return get_fl_mem_onenand(off, size, ext_buf);
		break;
#endif
	default:
		printf("get_fl_mem: unknown device type, " \
			"using raw offset!\n");
	}
	return (void*)off;
}

static inline void *get_node_mem(u32 off, void *ext_buf)
{
	struct mtdids *id = current_part->dev->id;

	switch(id->type) {
#if defined(CONFIG_CMD_FLASH)
	case MTD_DEV_TYPE_NOR:
		return get_node_mem_nor(off, ext_buf);
		break;
#endif
#if defined(CONFIG_JFFS2_NAND) && \
    defined(CONFIG_CMD_NAND)
	case MTD_DEV_TYPE_NAND:
		return get_node_mem_nand(off, ext_buf);
		break;
#endif
#if defined(CONFIG_CMD_ONENAND)
	case MTD_DEV_TYPE_ONENAND:
		return get_node_mem_onenand(off, ext_buf);
		break;
#endif
	default:
		printf("get_fl_mem: unknown device type, " \
			"using raw offset!\n");
	}
	return (void*)off;
}

static inline void put_fl_mem(void *buf, void *ext_buf)
{
	struct mtdids *id = current_part->dev->id;

	/* If buf is the same as ext_buf, it was provided by the caller -
	   we shouldn't free it then. */
	if (buf == ext_buf)
		return;
	switch (id->type) {
#if defined(CONFIG_JFFS2_NAND) && defined(CONFIG_CMD_NAND)
	case MTD_DEV_TYPE_NAND:
		return put_fl_mem_nand(buf);
#endif
#if defined(CONFIG_CMD_ONENAND)
	case MTD_DEV_TYPE_ONENAND:
		return put_fl_mem_onenand(buf);
#endif
	}
}

/* Compression names */
static char *compr_names[] = {
	"NONE",
	"ZERO",
	"RTIME",
	"RUBINMIPS",
	"COPY",
	"DYNRUBIN",
	"ZLIB",
#if defined(CONFIG_JFFS2_LZO)
	"LZO",
#endif
};

/* Memory management */
struct mem_block {
	u32	index;
	struct mem_block *next;
	struct b_node nodes[NODE_CHUNK];
};


static void
free_nodes(struct b_list *list)
{
	while (list->listMemBase != NULL) {
		struct mem_block *next = list->listMemBase->next;
		free( list->listMemBase );
		list->listMemBase = next;
	}
}

static struct b_node *
add_node(struct b_list *list)
{
	u32 index = 0;
	struct mem_block *memBase;
	struct b_node *b;

	memBase = list->listMemBase;
	if (memBase != NULL)
		index = memBase->index;
#if 0
	putLabeledWord("add_node: index = ", index);
	putLabeledWord("add_node: memBase = ", list->listMemBase);
#endif

	if (memBase == NULL || index >= NODE_CHUNK) {
		/* we need more space before we continue */
		memBase = mmalloc(sizeof(struct mem_block));
		if (memBase == NULL) {
			putstr("add_node: malloc failed\n");
			return NULL;
		}
		memBase->next = list->listMemBase;
		index = 0;
#if 0
		putLabeledWord("add_node: alloced a new membase at ", *memBase);
#endif

	}
	/* now we have room to add it. */
	b = &memBase->nodes[index];
	index ++;

	memBase->index = index;
	list->listMemBase = memBase;
	list->listCount++;
	return b;
}

static struct b_node *
insert_node(struct b_list *list, u32 offset)
{
	struct b_node *new;

	if (!(new = add_node(list))) {
		putstr("add_node failed!\r\n");
		return NULL;
	}
	new->offset = offset;
	new->next = NULL;

	if (list->listTail != NULL)
		list->listTail->next = new;
	else
		list->listHead = new;
	list->listTail = new;

	return new;
}

#ifdef CONFIG_SYS_JFFS2_SORT_FRAGMENTS
/* Sort data entries with the latest version last, so that if there
 * is overlapping data the latest version will be used.
 */
static int compare_inodes(struct b_node *new, struct b_node *old)
{
	/*
	 * Only read in the version info from flash, not the entire inode.
	 * This can make a big difference to speed if flash is slow.
	 */
	u32 new_version;
	u32 old_version;
	get_fl_mem(new->offset + offsetof(struct jffs2_raw_inode, version),
		   sizeof(new_version), &new_version);
	get_fl_mem(old->offset + offsetof(struct jffs2_raw_inode, version),
		   sizeof(old_version), &old_version);

	return new_version > old_version;
}

/* Sort directory entries so all entries in the same directory
 * with the same name are grouped together, with the latest version
 * last. This makes it easy to eliminate all but the latest version
 * by marking the previous version dead by setting the inode to 0.
 */
static int compare_dirents(struct b_node *new, struct b_node *old)
{
	/*
	 * Using NULL as the buffer for NOR flash prevents the entire node
	 * being read. This makes most comparisons much quicker as only one
	 * or two entries from the node will be used most of the time.
	 */
	struct jffs2_raw_dirent *jNew = get_node_mem(new->offset, NULL);
	struct jffs2_raw_dirent *jOld = get_node_mem(old->offset, NULL);
	int cmp;
	int ret;

	if (jNew->pino != jOld->pino) {
		/* ascending sort by pino */
		ret = jNew->pino > jOld->pino;
	} else if (jNew->nsize != jOld->nsize) {
		/*
		 * pino is the same, so use ascending sort by nsize,
		 * so we don't do strncmp unless we really must.
		 */
		ret = jNew->nsize > jOld->nsize;
	} else {
		/*
		 * length is also the same, so use ascending sort by name
		 */
		cmp = strncmp((char *)jNew->name, (char *)jOld->name,
			jNew->nsize);
		if (cmp != 0) {
			ret = cmp > 0;
		} else {
			/*
			 * we have duplicate names in this directory,
			 * so use ascending sort by version
			 */
			ret = jNew->version > jOld->version;
		}
	}
	put_fl_mem(jNew, NULL);
	put_fl_mem(jOld, NULL);

	return ret;
}
#endif

void
jffs2_free_cache(struct part_info *part)
{
	struct b_lists *pL;

	if (part->jffs2_priv != NULL) {
		pL = (struct b_lists *)part->jffs2_priv;
		free_nodes(&pL->frag);
		free_nodes(&pL->dir);
		free(pL->readbuf);
		free(pL);
	}
}

static u32
jffs_init_1pass_list(struct part_info *part)
{
	struct b_lists *pL;

	jffs2_free_cache(part);

	if (NULL != (part->jffs2_priv = malloc(sizeof(struct b_lists)))) {
		pL = (struct b_lists *)part->jffs2_priv;

		memset(pL, 0, sizeof(*pL));
#ifdef CONFIG_SYS_JFFS2_SORT_FRAGMENTS
		pL->dir.listCompare = compare_dirents;
		pL->frag.listCompare = compare_inodes;
#endif
	}
	return 0;
}

/* find the inode from the slashless name given a parent */
static long
jffs2_1pass_read_inode(struct b_lists *pL, u32 inode, char *dest)
{
	struct b_node *b;
	struct jffs2_raw_inode *jNode;
	u32 totalSize = 0;
	u32 latestVersion = 0;
	uchar *lDest;
	uchar *src;
	int i;
	u32 counter = 0;
#ifdef CONFIG_SYS_JFFS2_SORT_FRAGMENTS
	/* Find file size before loading any data, so fragments that
	 * start past the end of file can be ignored. A fragment
	 * that is partially in the file is loaded, so extra data may
	 * be loaded up to the next 4K boundary above the file size.
	 * This shouldn't cause trouble when loading kernel images, so
	 * we will live with it.
	 */
	for (b = pL->frag.listHead; b != NULL; b = b->next) {
		jNode = (struct jffs2_raw_inode *) get_fl_mem(b->offset,
			sizeof(struct jffs2_raw_inode), pL->readbuf);
		if ((inode == jNode->ino)) {
			/* get actual file length from the newest node */
			if (jNode->version >= latestVersion) {
				totalSize = jNode->isize;
				latestVersion = jNode->version;
			}
		}
		put_fl_mem(jNode, pL->readbuf);
	}
	/*
	 * If no destination is provided, we are done.
	 * Just return the total size.
	 */
	if (!dest)
		return totalSize;
#endif

	for (b = pL->frag.listHead; b != NULL; b = b->next) {
		/*
		 * Copy just the node and not the data at this point,
		 * since we don't yet know if we need this data.
		 */
		jNode = (struct jffs2_raw_inode *)get_fl_mem(b->offset,
				sizeof(struct jffs2_raw_inode),
				pL->readbuf);
		if (inode == jNode->ino) {
#if 0
			putLabeledWord("\r\n\r\nread_inode: totlen = ", jNode->totlen);
			putLabeledWord("read_inode: inode = ", jNode->ino);
			putLabeledWord("read_inode: version = ", jNode->version);
			putLabeledWord("read_inode: isize = ", jNode->isize);
			putLabeledWord("read_inode: offset = ", jNode->offset);
			putLabeledWord("read_inode: csize = ", jNode->csize);
			putLabeledWord("read_inode: dsize = ", jNode->dsize);
			putLabeledWord("read_inode: compr = ", jNode->compr);
			putLabeledWord("read_inode: usercompr = ", jNode->usercompr);
			putLabeledWord("read_inode: flags = ", jNode->flags);
#endif

#ifndef CONFIG_SYS_JFFS2_SORT_FRAGMENTS
			/* get actual file length from the newest node */
			if (jNode->version >= latestVersion) {
				totalSize = jNode->isize;
				latestVersion = jNode->version;
			}
#endif

			if(dest) {
				/*
				 * Now that the inode has been checked,
				 * read the entire inode, including data.
				 */
				put_fl_mem(jNode, pL->readbuf);
				jNode = (struct jffs2_raw_inode *)
					get_node_mem(b->offset, pL->readbuf);
				src = ((uchar *)jNode) +
					sizeof(struct jffs2_raw_inode);
				/* ignore data behind latest known EOF */
				if (jNode->offset > totalSize) {
					put_fl_mem(jNode, pL->readbuf);
					continue;
				}
				if (b->datacrc == CRC_UNKNOWN)
					b->datacrc = data_crc(jNode) ?
						CRC_OK : CRC_BAD;
				if (b->datacrc == CRC_BAD) {
					put_fl_mem(jNode, pL->readbuf);
					continue;
				}

				lDest = (uchar *) (dest + jNode->offset);
#if 0
				putLabeledWord("read_inode: src = ", src);
				putLabeledWord("read_inode: dest = ", lDest);
#endif
				switch (jNode->compr) {
				case JFFS2_COMPR_NONE:
					ldr_memcpy(lDest, src, jNode->dsize);
					break;
				case JFFS2_COMPR_ZERO:
					for (i = 0; i < jNode->dsize; i++)
						*(lDest++) = 0;
					break;
				case JFFS2_COMPR_RTIME:
					rtime_decompress(src, lDest, jNode->csize, jNode->dsize);
					break;
				case JFFS2_COMPR_DYNRUBIN:
					/* this is slow but it works */
					dynrubin_decompress(src, lDest, jNode->csize, jNode->dsize);
					break;
				case JFFS2_COMPR_ZLIB:
					zlib_decompress(src, lDest, jNode->csize, jNode->dsize);
					break;
#if defined(CONFIG_JFFS2_LZO)
				case JFFS2_COMPR_LZO:
					lzo_decompress(src, lDest, jNode->csize, jNode->dsize);
					break;
#endif
				default:
					/* unknown */
					putLabeledWord("UNKNOWN COMPRESSION METHOD = ", jNode->compr);
					put_fl_mem(jNode, pL->readbuf);
					return -1;
					break;
				}
			}

#if 0
			putLabeledWord("read_inode: totalSize = ", totalSize);
#endif
		}
		counter++;
		put_fl_mem(jNode, pL->readbuf);
	}

#if 0
	putLabeledWord("read_inode: returning = ", totalSize);
#endif
	return totalSize;
}

/* find the inode from the slashless name given a parent */
static u32
jffs2_1pass_find_inode(struct b_lists * pL, const char *name, u32 pino)
{
	struct b_node *b;
	struct jffs2_raw_dirent *jDir;
	int len;
	u32 counter;
	u32 version = 0;
	u32 inode = 0;

	/* name is assumed slash free */
	len = strlen(name);

	counter = 0;
	/* we need to search all and return the inode with the highest version */
	for(b = pL->dir.listHead; b; b = b->next, counter++) {
		jDir = (struct jffs2_raw_dirent *) get_node_mem(b->offset,
								pL->readbuf);
		if ((pino == jDir->pino) && (len == jDir->nsize) &&
		    (!strncmp((char *)jDir->name, name, len))) {	/* a match */
			if (jDir->version < version) {
				put_fl_mem(jDir, pL->readbuf);
				continue;
			}

			if (jDir->version == version && inode != 0) {
				/* I'm pretty sure this isn't legal */
				putstr(" ** ERROR ** ");
				putnstr(jDir->name, jDir->nsize);
				putLabeledWord(" has dup version =", version);
			}
			inode = jDir->ino;
			version = jDir->version;
		}
#if 0
		putstr("\r\nfind_inode:p&l ->");
		putnstr(jDir->name, jDir->nsize);
		putstr("\r\n");
		putLabeledWord("pino = ", jDir->pino);
		putLabeledWord("nsize = ", jDir->nsize);
		putLabeledWord("b = ", (u32) b);
		putLabeledWord("counter = ", counter);
#endif
		put_fl_mem(jDir, pL->readbuf);
	}
	return inode;
}

char *mkmodestr(unsigned long mode, char *str)
{
	static const char *l = "xwr";
	int mask = 1, i;
	char c;

	switch (mode & S_IFMT) {
		case S_IFDIR:    str[0] = 'd'; break;
		case S_IFBLK:    str[0] = 'b'; break;
		case S_IFCHR:    str[0] = 'c'; break;
		case S_IFIFO:    str[0] = 'f'; break;
		case S_IFLNK:    str[0] = 'l'; break;
		case S_IFSOCK:   str[0] = 's'; break;
		case S_IFREG:    str[0] = '-'; break;
		default:         str[0] = '?';
	}

	for(i = 0; i < 9; i++) {
		c = l[i%3];
		str[9-i] = (mode & mask)?c:'-';
		mask = mask<<1;
	}

	if(mode & S_ISUID) str[3] = (mode & S_IXUSR)?'s':'S';
	if(mode & S_ISGID) str[6] = (mode & S_IXGRP)?'s':'S';
	if(mode & S_ISVTX) str[9] = (mode & S_IXOTH)?'t':'T';
	str[10] = '\0';
	return str;
}

static inline void dump_stat(struct stat *st, const char *name)
{
	char str[20];
	char s[64], *p;

	if (st->st_mtime == (time_t)(-1)) /* some ctimes really hate -1 */
		st->st_mtime = 1;

	ctime_r((time_t *)&st->st_mtime, s/*,64*/); /* newlib ctime doesn't have buflen */

	if ((p = strchr(s,'\n')) != NULL) *p = '\0';
	if ((p = strchr(s,'\r')) != NULL) *p = '\0';

/*
	printf("%6lo %s %8ld %s %s\n", st->st_mode, mkmodestr(st->st_mode, str),
		st->st_size, s, name);
*/

	printf(" %s %8ld %s %s", mkmodestr(st->st_mode,str), st->st_size, s, name);
}

static inline u32 dump_inode(struct b_lists * pL, struct jffs2_raw_dirent *d, struct jffs2_raw_inode *i)
{
	char fname[256];
	struct stat st;

	if(!d || !i) return -1;

	strncpy(fname, (char *)d->name, d->nsize);
	fname[d->nsize] = '\0';

	memset(&st,0,sizeof(st));

	st.st_mtime = i->mtime;
	st.st_mode = i->mode;
	st.st_ino = i->ino;
	st.st_size = i->isize;

	dump_stat(&st, fname);

	if (d->type == DT_LNK) {
		unsigned char *src = (unsigned char *) (&i[1]);
	        putstr(" -> ");
		putnstr(src, (int)i->dsize);
	}

	putstr("\r\n");

	return 0;
}

/* list inodes with the given pino */
static u32
jffs2_1pass_list_inodes(struct b_lists * pL, u32 pino)
{
	struct b_node *b;
	struct jffs2_raw_dirent *jDir;

	for (b = pL->dir.listHead; b; b = b->next) {
		jDir = (struct jffs2_raw_dirent *) get_node_mem(b->offset,
								pL->readbuf);
		if (pino == jDir->pino) {
			u32 i_version = 0;
			struct jffs2_raw_inode *jNode, *i = NULL;
			struct b_node *b2;

#ifdef CONFIG_SYS_JFFS2_SORT_FRAGMENTS
			/* Check for more recent versions of this file */
			int match;
			do {
				struct b_node *next = b->next;
				struct jffs2_raw_dirent *jDirNext;
				if (!next)
					break;
				jDirNext = (struct jffs2_raw_dirent *)
					get_node_mem(next->offset, NULL);
				match = jDirNext->pino == jDir->pino &&
					jDirNext->nsize == jDir->nsize &&
					strncmp((char *)jDirNext->name,
						(char *)jDir->name,
						jDir->nsize) == 0;
				if (match) {
					/* Use next. It is more recent */
					b = next;
					/* Update buffer with the new info */
					*jDir = *jDirNext;
				}
				put_fl_mem(jDirNext, NULL);
			} while (match);
#endif
			if (jDir->ino == 0) {
				/* Deleted file */
				put_fl_mem(jDir, pL->readbuf);
				continue;
			}

			for (b2 = pL->frag.listHead; b2; b2 = b2->next) {
				jNode = (struct jffs2_raw_inode *)
					get_fl_mem(b2->offset, sizeof(*jNode),
						   NULL);
				if (jNode->ino == jDir->ino &&
				    jNode->version >= i_version) {
					i_version = jNode->version;
					if (i)
						put_fl_mem(i, NULL);

					if (jDir->type == DT_LNK)
						i = get_node_mem(b2->offset,
								 NULL);
					else
						i = get_fl_mem(b2->offset,
							       sizeof(*i),
							       NULL);
				}
				put_fl_mem(jNode, NULL);
			}

			dump_inode(pL, jDir, i);
			put_fl_mem(i, NULL);
		}
		put_fl_mem(jDir, pL->readbuf);
	}
	return pino;
}

static u32
jffs2_1pass_search_inode(struct b_lists * pL, const char *fname, u32 pino)
{
	int i;
	char tmp[256];
	char working_tmp[256];
	char *c;

	/* discard any leading slash */
	i = 0;
	while (fname[i] == '/')
		i++;
	strcpy(tmp, &fname[i]);

	while ((c = (char *) strchr(tmp, '/')))	/* we are still dired searching */
	{
		strncpy(working_tmp, tmp, c - tmp);
		working_tmp[c - tmp] = '\0';
#if 0
		putstr("search_inode: tmp = ");
		putstr(tmp);
		putstr("\r\n");
		putstr("search_inode: wtmp = ");
		putstr(working_tmp);
		putstr("\r\n");
		putstr("search_inode: c = ");
		putstr(c);
		putstr("\r\n");
#endif
		for (i = 0; i < strlen(c) - 1; i++)
			tmp[i] = c[i + 1];
		tmp[i] = '\0';
#if 0
		putstr("search_inode: post tmp = ");
		putstr(tmp);
		putstr("\r\n");
#endif

		if (!(pino = jffs2_1pass_find_inode(pL, working_tmp, pino))) {
			putstr("find_inode failed for name=");
			putstr(working_tmp);
			putstr("\r\n");
			return 0;
		}
	}
	/* this is for the bare filename, directories have already been mapped */
	if (!(pino = jffs2_1pass_find_inode(pL, tmp, pino))) {
		putstr("find_inode failed for name=");
		putstr(tmp);
		putstr("\r\n");
		return 0;
	}
	return pino;

}

static u32
jffs2_1pass_resolve_inode(struct b_lists * pL, u32 ino)
{
	struct b_node *b;
	struct b_node *b2;
	struct jffs2_raw_dirent *jDir;
	struct jffs2_raw_inode *jNode;
	u8 jDirFoundType = 0;
	u32 jDirFoundIno = 0;
	u32 jDirFoundPino = 0;
	char tmp[256];
	u32 version = 0;
	u32 pino;
	unsigned char *src;

	/* we need to search all and return the inode with the highest version */
	for(b = pL->dir.listHead; b; b = b->next) {
		jDir = (struct jffs2_raw_dirent *) get_node_mem(b->offset,
								pL->readbuf);
		if (ino == jDir->ino) {
			if (jDir->version < version) {
				put_fl_mem(jDir, pL->readbuf);
				continue;
			}

			if (jDir->version == version && jDirFoundType) {
				/* I'm pretty sure this isn't legal */
				putstr(" ** ERROR ** ");
				putnstr(jDir->name, jDir->nsize);
				putLabeledWord(" has dup version (resolve) = ",
					version);
			}

			jDirFoundType = jDir->type;
			jDirFoundIno = jDir->ino;
			jDirFoundPino = jDir->pino;
			version = jDir->version;
		}
		put_fl_mem(jDir, pL->readbuf);
	}
	/* now we found the right entry again. (shoulda returned inode*) */
	if (jDirFoundType != DT_LNK)
		return jDirFoundIno;

	/* it's a soft link so we follow it again. */
	b2 = pL->frag.listHead;
	while (b2) {
		jNode = (struct jffs2_raw_inode *) get_node_mem(b2->offset,
								pL->readbuf);
		if (jNode->ino == jDirFoundIno) {
			src = (unsigned char *)jNode + sizeof(struct jffs2_raw_inode);

#if 0
			putLabeledWord("\t\t dsize = ", jNode->dsize);
			putstr("\t\t target = ");
			putnstr(src, jNode->dsize);
			putstr("\r\n");
#endif
			strncpy(tmp, (char *)src, jNode->dsize);
			tmp[jNode->dsize] = '\0';
			put_fl_mem(jNode, pL->readbuf);
			break;
		}
		b2 = b2->next;
		put_fl_mem(jNode, pL->readbuf);
	}
	/* ok so the name of the new file to find is in tmp */
	/* if it starts with a slash it is root based else shared dirs */
	if (tmp[0] == '/')
		pino = 1;
	else
		pino = jDirFoundPino;

	return jffs2_1pass_search_inode(pL, tmp, pino);
}

static u32
jffs2_1pass_search_list_inodes(struct b_lists * pL, const char *fname, u32 pino)
{
	int i;
	char tmp[256];
	char working_tmp[256];
	char *c;

	/* discard any leading slash */
	i = 0;
	while (fname[i] == '/')
		i++;
	strcpy(tmp, &fname[i]);
	working_tmp[0] = '\0';
	while ((c = (char *) strchr(tmp, '/')))	/* we are still dired searching */
	{
		strncpy(working_tmp, tmp, c - tmp);
		working_tmp[c - tmp] = '\0';
		for (i = 0; i < strlen(c) - 1; i++)
			tmp[i] = c[i + 1];
		tmp[i] = '\0';
		/* only a failure if we arent looking at top level */
		if (!(pino = jffs2_1pass_find_inode(pL, working_tmp, pino)) &&
		    (working_tmp[0])) {
			putstr("find_inode failed for name=");
			putstr(working_tmp);
			putstr("\r\n");
			return 0;
		}
	}

	if (tmp[0] && !(pino = jffs2_1pass_find_inode(pL, tmp, pino))) {
		putstr("find_inode failed for name=");
		putstr(tmp);
		putstr("\r\n");
		return 0;
	}
	/* this is for the bare filename, directories have already been mapped */
	if (!(pino = jffs2_1pass_list_inodes(pL, pino))) {
		putstr("find_inode failed for name=");
		putstr(tmp);
		putstr("\r\n");
		return 0;
	}
	return pino;

}

unsigned char
jffs2_1pass_rescan_needed(struct part_info *part)
{
	struct b_node *b;
	struct jffs2_unknown_node onode;
	struct jffs2_unknown_node *node;
	struct b_lists *pL = (struct b_lists *)part->jffs2_priv;

	if (part->jffs2_priv == 0){
		DEBUGF ("rescan: First time in use\n");
		return 1;
	}

	/* if we have no list, we need to rescan */
	if (pL->frag.listCount == 0) {
		DEBUGF ("rescan: fraglist zero\n");
		return 1;
	}

	/* but suppose someone reflashed a partition at the same offset... */
	b = pL->dir.listHead;
	while (b) {
		node = (struct jffs2_unknown_node *) get_fl_mem(b->offset,
			sizeof(onode), &onode);
		if (node->nodetype != JFFS2_NODETYPE_DIRENT) {
			DEBUGF ("rescan: fs changed beneath me? (%lx)\n",
					(unsigned long) b->offset);
			return 1;
		}
		b = b->next;
	}
	return 0;
}

#ifdef CONFIG_JFFS2_SUMMARY
static u32 sum_get_unaligned32(u32 *ptr)
{
	u32 val;
	u8 *p = (u8 *)ptr;

	val = *p | (*(p + 1) << 8) | (*(p + 2) << 16) | (*(p + 3) << 24);

	return __le32_to_cpu(val);
}

static u16 sum_get_unaligned16(u16 *ptr)
{
	u16 val;
	u8 *p = (u8 *)ptr;

	val = *p | (*(p + 1) << 8);

	return __le16_to_cpu(val);
}

#define dbg_summary(...) do {} while (0);
/*
 * Process the stored summary information - helper function for
 * jffs2_sum_scan_sumnode()
 */

static int jffs2_sum_process_sum_data(struct part_info *part, uint32_t offset,
				struct jffs2_raw_summary *summary,
				struct b_lists *pL)
{
	void *sp;
	int i, pass;
	void *ret;

	for (pass = 0; pass < 2; pass++) {
		sp = summary->sum;

		for (i = 0; i < summary->sum_num; i++) {
			struct jffs2_sum_unknown_flash *spu = sp;
			dbg_summary("processing summary index %d\n", i);

			switch (sum_get_unaligned16(&spu->nodetype)) {
				case JFFS2_NODETYPE_INODE: {
				struct jffs2_sum_inode_flash *spi;
					if (pass) {
						spi = sp;

						ret = insert_node(&pL->frag,
							(u32)part->offset +
							offset +
							sum_get_unaligned32(
								&spi->offset));
						if (ret == NULL)
							return -1;
					}

					sp += JFFS2_SUMMARY_INODE_SIZE;

					break;
				}
				case JFFS2_NODETYPE_DIRENT: {
					struct jffs2_sum_dirent_flash *spd;
					spd = sp;
					if (pass) {
						ret = insert_node(&pL->dir,
							(u32) part->offset +
							offset +
							sum_get_unaligned32(
								&spd->offset));
						if (ret == NULL)
							return -1;
					}

					sp += JFFS2_SUMMARY_DIRENT_SIZE(
							spd->nsize);

					break;
				}
				default : {
					uint16_t nodetype = sum_get_unaligned16(
								&spu->nodetype);
					printf("Unsupported node type %x found"
							" in summary!\n",
							nodetype);
					if ((nodetype & JFFS2_COMPAT_MASK) ==
							JFFS2_FEATURE_INCOMPAT)
						return -EIO;
					return -EBADMSG;
				}
			}
		}
	}
	return 0;
}

/* Process the summary node - called from jffs2_scan_eraseblock() */
int jffs2_sum_scan_sumnode(struct part_info *part, uint32_t offset,
			   struct jffs2_raw_summary *summary, uint32_t sumsize,
			   struct b_lists *pL)
{
	struct jffs2_unknown_node crcnode;
	int ret, __maybe_unused ofs;
	uint32_t crc;

	ofs = part->sector_size - sumsize;

	dbg_summary("summary found for 0x%08x at 0x%08x (0x%x bytes)\n",
		    offset, offset + ofs, sumsize);

	/* OK, now check for node validity and CRC */
	crcnode.magic = JFFS2_MAGIC_BITMASK;
	crcnode.nodetype = JFFS2_NODETYPE_SUMMARY;
	crcnode.totlen = summary->totlen;
	crc = crc32_no_comp(0, (uchar *)&crcnode, sizeof(crcnode)-4);

	if (summary->hdr_crc != crc) {
		dbg_summary("Summary node header is corrupt (bad CRC or "
				"no summary at all)\n");
		goto crc_err;
	}

	if (summary->totlen != sumsize) {
		dbg_summary("Summary node is corrupt (wrong erasesize?)\n");
		goto crc_err;
	}

	crc = crc32_no_comp(0, (uchar *)summary,
			sizeof(struct jffs2_raw_summary)-8);

	if (summary->node_crc != crc) {
		dbg_summary("Summary node is corrupt (bad CRC)\n");
		goto crc_err;
	}

	crc = crc32_no_comp(0, (uchar *)summary->sum,
			sumsize - sizeof(struct jffs2_raw_summary));

	if (summary->sum_crc != crc) {
		dbg_summary("Summary node data is corrupt (bad CRC)\n");
		goto crc_err;
	}

	if (summary->cln_mkr)
		dbg_summary("Summary : CLEANMARKER node \n");

	ret = jffs2_sum_process_sum_data(part, offset, summary, pL);
	if (ret == -EBADMSG)
		return 0;
	if (ret)
		return ret;		/* real error */

	return 1;

crc_err:
	putstr("Summary node crc error, skipping summary information.\n");

	return 0;
}
#endif /* CONFIG_JFFS2_SUMMARY */

#ifdef DEBUG_FRAGMENTS
static void
dump_fragments(struct b_lists *pL)
{
	struct b_node *b;
	struct jffs2_raw_inode ojNode;
	struct jffs2_raw_inode *jNode;

	putstr("\r\n\r\n******The fragment Entries******\r\n");
	b = pL->frag.listHead;
	while (b) {
		jNode = (struct jffs2_raw_inode *) get_fl_mem(b->offset,
			sizeof(ojNode), &ojNode);
		putLabeledWord("\r\n\tbuild_list: FLASH_OFFSET = ", b->offset);
		putLabeledWord("\tbuild_list: totlen = ", jNode->totlen);
		putLabeledWord("\tbuild_list: inode = ", jNode->ino);
		putLabeledWord("\tbuild_list: version = ", jNode->version);
		putLabeledWord("\tbuild_list: isize = ", jNode->isize);
		putLabeledWord("\tbuild_list: atime = ", jNode->atime);
		putLabeledWord("\tbuild_list: offset = ", jNode->offset);
		putLabeledWord("\tbuild_list: csize = ", jNode->csize);
		putLabeledWord("\tbuild_list: dsize = ", jNode->dsize);
		putLabeledWord("\tbuild_list: compr = ", jNode->compr);
		putLabeledWord("\tbuild_list: usercompr = ", jNode->usercompr);
		putLabeledWord("\tbuild_list: flags = ", jNode->flags);
		putLabeledWord("\tbuild_list: offset = ", b->offset);	/* FIXME: ? [RS] */
		b = b->next;
	}
}
#endif

#ifdef DEBUG_DIRENTS
static void
dump_dirents(struct b_lists *pL)
{
	struct b_node *b;
	struct jffs2_raw_dirent *jDir;

	putstr("\r\n\r\n******The directory Entries******\r\n");
	b = pL->dir.listHead;
	while (b) {
		jDir = (struct jffs2_raw_dirent *) get_node_mem(b->offset,
								pL->readbuf);
		putstr("\r\n");
		putnstr(jDir->name, jDir->nsize);
		putLabeledWord("\r\n\tbuild_list: magic = ", jDir->magic);
		putLabeledWord("\tbuild_list: nodetype = ", jDir->nodetype);
		putLabeledWord("\tbuild_list: hdr_crc = ", jDir->hdr_crc);
		putLabeledWord("\tbuild_list: pino = ", jDir->pino);
		putLabeledWord("\tbuild_list: version = ", jDir->version);
		putLabeledWord("\tbuild_list: ino = ", jDir->ino);
		putLabeledWord("\tbuild_list: mctime = ", jDir->mctime);
		putLabeledWord("\tbuild_list: nsize = ", jDir->nsize);
		putLabeledWord("\tbuild_list: type = ", jDir->type);
		putLabeledWord("\tbuild_list: node_crc = ", jDir->node_crc);
		putLabeledWord("\tbuild_list: name_crc = ", jDir->name_crc);
		putLabeledWord("\tbuild_list: offset = ", b->offset);	/* FIXME: ? [RS] */
		b = b->next;
		put_fl_mem(jDir, pL->readbuf);
	}
}
#endif

#define DEFAULT_EMPTY_SCAN_SIZE	256

static inline uint32_t EMPTY_SCAN_SIZE(uint32_t sector_size)
{
	if (sector_size < DEFAULT_EMPTY_SCAN_SIZE)
		return sector_size;
	else
		return DEFAULT_EMPTY_SCAN_SIZE;
}

static u32
jffs2_1pass_build_lists(struct part_info * part)
{
	struct b_lists *pL;
	struct jffs2_unknown_node *node;
	u32 nr_sectors;
	u32 i;
	u32 counter4 = 0;
	u32 counterF = 0;
	u32 counterN = 0;
	u32 max_totlen = 0;
	u32 buf_size;
	char *buf;

	nr_sectors = lldiv(part->size, part->sector_size);
	/* turn off the lcd.  Refreshing the lcd adds 50% overhead to the */
	/* jffs2 list building enterprise nope.  in newer versions the overhead is */
	/* only about 5 %.  not enough to inconvenience people for. */
	/* lcd_off(); */

	/* if we are building a list we need to refresh the cache. */
	jffs_init_1pass_list(part);
	pL = (struct b_lists *)part->jffs2_priv;
	buf = malloc(DEFAULT_EMPTY_SCAN_SIZE);
	puts ("Scanning JFFS2 FS:   ");

	/* start at the beginning of the partition */
	for (i = 0; i < nr_sectors; i++) {
		uint32_t sector_ofs = i * part->sector_size;
		uint32_t buf_ofs = sector_ofs;
		uint32_t buf_len;
		uint32_t ofs, prevofs;
#ifdef CONFIG_JFFS2_SUMMARY
		struct jffs2_sum_marker *sm;
		void *sumptr = NULL;
		uint32_t sumlen;
		int ret;
#endif
		/* Indicates a sector with a CLEANMARKER was found */
		int clean_sector = 0;

		/* Set buf_size to maximum length */
		buf_size = DEFAULT_EMPTY_SCAN_SIZE;
		WATCHDOG_RESET();

#ifdef CONFIG_JFFS2_SUMMARY
		buf_len = sizeof(*sm);

		/* Read as much as we want into the _end_ of the preallocated
		 * buffer
		 */
		get_fl_mem(part->offset + sector_ofs + part->sector_size -
				buf_len, buf_len, buf + buf_size - buf_len);

		sm = (void *)buf + buf_size - sizeof(*sm);
		if (sm->magic == JFFS2_SUM_MAGIC) {
			sumlen = part->sector_size - sm->offset;
			sumptr = buf + buf_size - sumlen;

			/* Now, make sure the summary itself is available */
			if (sumlen > buf_size) {
				/* Need to kmalloc for this. */
				sumptr = malloc(sumlen);
				if (!sumptr) {
					putstr("Can't get memory for summary "
							"node!\n");
					free(buf);
					jffs2_free_cache(part);
					return 0;
				}
				memcpy(sumptr + sumlen - buf_len, buf +
						buf_size - buf_len, buf_len);
			}
			if (buf_len < sumlen) {
				/* Need to read more so that the entire summary
				 * node is present
				 */
				get_fl_mem(part->offset + sector_ofs +
						part->sector_size - sumlen,
						sumlen - buf_len, sumptr);
			}
		}

		if (sumptr) {
			ret = jffs2_sum_scan_sumnode(part, sector_ofs, sumptr,
					sumlen, pL);

			if (buf_size && sumlen > buf_size)
				free(sumptr);
			if (ret < 0) {
				free(buf);
				jffs2_free_cache(part);
				return 0;
			}
			if (ret)
				continue;

		}
#endif /* CONFIG_JFFS2_SUMMARY */

		buf_len = EMPTY_SCAN_SIZE(part->sector_size);

		get_fl_mem((u32)part->offset + buf_ofs, buf_len, buf);

		/* We temporarily use 'ofs' as a pointer into the buffer/jeb */
		ofs = 0;

		/* Scan only 4KiB of 0xFF before declaring it's empty */
		while (ofs < EMPTY_SCAN_SIZE(part->sector_size) &&
				*(uint32_t *)(&buf[ofs]) == 0xFFFFFFFF)
			ofs += 4;

		if (ofs == EMPTY_SCAN_SIZE(part->sector_size))
			continue;

		ofs += sector_ofs;
		prevofs = ofs - 1;
		/*
		 * Set buf_size down to the minimum size required.
		 * This prevents reading in chunks of flash data unnecessarily.
		 */
		buf_size = sizeof(union jffs2_node_union);

	scan_more:
		while (ofs < sector_ofs + part->sector_size) {
			if (ofs == prevofs) {
				printf("offset %08x already seen, skip\n", ofs);
				ofs += 4;
				counter4++;
				continue;
			}
			prevofs = ofs;
			if (sector_ofs + part->sector_size <
					ofs + sizeof(*node))
				break;
			if (buf_ofs + buf_len < ofs + sizeof(*node)) {
				buf_len = min_t(uint32_t, buf_size, sector_ofs
						+ part->sector_size - ofs);
				get_fl_mem((u32)part->offset + ofs, buf_len,
					   buf);
				buf_ofs = ofs;
			}

			node = (struct jffs2_unknown_node *)&buf[ofs-buf_ofs];

			if (*(uint32_t *)(&buf[ofs-buf_ofs]) == 0xffffffff) {
				uint32_t inbuf_ofs;
				uint32_t scan_end;

				ofs += 4;
				scan_end = min_t(uint32_t, EMPTY_SCAN_SIZE(
							part->sector_size)/8,
							buf_len);
			more_empty:
				inbuf_ofs = ofs - buf_ofs;
				while (inbuf_ofs < scan_end) {
					if (*(uint32_t *)(&buf[inbuf_ofs]) !=
							0xffffffff)
						goto scan_more;

					inbuf_ofs += 4;
					ofs += 4;
				}
				/* Ran off end. */
				/*
				 * If this sector had a clean marker at the
				 * beginning, and immediately following this
				 * have been a bunch of FF bytes, treat the
				 * entire sector as empty.
				 */
				if (clean_sector)
					break;

				/* See how much more there is to read in this
				 * eraseblock...
				 */
				buf_len = min_t(uint32_t, buf_size,
						sector_ofs +
						part->sector_size - ofs);
				if (!buf_len) {
					/* No more to read. Break out of main
					 * loop without marking this range of
					 * empty space as dirty (because it's
					 * not)
					 */
					break;
				}
				scan_end = buf_len;
				get_fl_mem((u32)part->offset + ofs, buf_len,
					   buf);
				buf_ofs = ofs;
				goto more_empty;
			}
			/*
			 * Found something not erased in the sector, so reset
			 * the 'clean_sector' flag.
			 */
			clean_sector = 0;
			if (node->magic != JFFS2_MAGIC_BITMASK ||
					!hdr_crc(node)) {
				ofs += 4;
				counter4++;
				continue;
			}
			if (ofs + node->totlen >
					sector_ofs + part->sector_size) {
				ofs += 4;
				counter4++;
				continue;
			}
			/* if its a fragment add it */
			switch (node->nodetype) {
			case JFFS2_NODETYPE_INODE:
				if (buf_ofs + buf_len < ofs + sizeof(struct
							jffs2_raw_inode)) {
					buf_len = min_t(uint32_t,
							sizeof(struct jffs2_raw_inode),
							sector_ofs +
							part->sector_size -
							ofs);
					get_fl_mem((u32)part->offset + ofs,
						   buf_len, buf);
					buf_ofs = ofs;
					node = (void *)buf;
				}
				if (!inode_crc((struct jffs2_raw_inode *)node))
					break;

				if (insert_node(&pL->frag, (u32) part->offset +
						ofs) == NULL) {
					free(buf);
					jffs2_free_cache(part);
					return 0;
				}
				if (max_totlen < node->totlen)
					max_totlen = node->totlen;
				break;
			case JFFS2_NODETYPE_DIRENT:
				if (buf_ofs + buf_len < ofs + sizeof(struct
							jffs2_raw_dirent) +
							((struct
							 jffs2_raw_dirent *)
							node)->nsize) {
					buf_len = min_t(uint32_t,
							node->totlen,
							sector_ofs +
							part->sector_size -
							ofs);
					get_fl_mem((u32)part->offset + ofs,
						   buf_len, buf);
					buf_ofs = ofs;
					node = (void *)buf;
				}

				if (!dirent_crc((struct jffs2_raw_dirent *)
							node) ||
						!dirent_name_crc(
							(struct
							 jffs2_raw_dirent *)
							node))
					break;
				if (! (counterN%100))
					puts ("\b\b.  ");
				if (insert_node(&pL->dir, (u32) part->offset +
						ofs) == NULL) {
					free(buf);
					jffs2_free_cache(part);
					return 0;
				}
				if (max_totlen < node->totlen)
					max_totlen = node->totlen;
				counterN++;
				break;
			case JFFS2_NODETYPE_CLEANMARKER:
				if (node->totlen != sizeof(struct jffs2_unknown_node))
					printf("OOPS Cleanmarker has bad size "
						"%d != %zu\n",
						node->totlen,
						sizeof(struct jffs2_unknown_node));
				if ((node->totlen ==
				     sizeof(struct jffs2_unknown_node)) &&
				    (ofs == sector_ofs)) {
					/*
					 * Found a CLEANMARKER at the beginning
					 * of the sector. It's in the correct
					 * place with correct size and CRC.
					 */
					clean_sector = 1;
				}
				break;
			case JFFS2_NODETYPE_PADDING:
				if (node->totlen < sizeof(struct jffs2_unknown_node))
					printf("OOPS Padding has bad size "
						"%d < %zu\n",
						node->totlen,
						sizeof(struct jffs2_unknown_node));
				break;
			case JFFS2_NODETYPE_SUMMARY:
				break;
			default:
				printf("Unknown node type: %x len %d offset 0x%x\n",
					node->nodetype,
					node->totlen, ofs);
			}
			ofs += ((node->totlen + 3) & ~3);
			counterF++;
		}
	}

	free(buf);
#if defined(CONFIG_SYS_JFFS2_SORT_FRAGMENTS)
	/*
	 * Sort the lists.
	 */
	sort_list(&pL->frag);
	sort_list(&pL->dir);
#endif
	putstr("\b\b done.\r\n");		/* close off the dots */

	/* We don't care if malloc failed - then each read operation will
	 * allocate its own buffer as necessary (NAND) or will read directly
	 * from flash (NOR).
	 */
	pL->readbuf = malloc(max_totlen);

	/* turn the lcd back on. */
	/* splash(); */

#if 0
	putLabeledWord("dir entries = ", pL->dir.listCount);
	putLabeledWord("frag entries = ", pL->frag.listCount);
	putLabeledWord("+4 increments = ", counter4);
	putLabeledWord("+file_offset increments = ", counterF);

#endif

#ifdef DEBUG_DIRENTS
	dump_dirents(pL);
#endif

#ifdef DEBUG_FRAGMENTS
	dump_fragments(pL);
#endif

	/* give visual feedback that we are done scanning the flash */
	led_blink(0x0, 0x0, 0x1, 0x1);	/* off, forever, on 100ms, off 100ms */
	return 1;
}


static u32
jffs2_1pass_fill_info(struct b_lists * pL, struct b_jffs2_info * piL)
{
	struct b_node *b;
	struct jffs2_raw_inode ojNode;
	struct jffs2_raw_inode *jNode;
	int i;

	for (i = 0; i < JFFS2_NUM_COMPR; i++) {
		piL->compr_info[i].num_frags = 0;
		piL->compr_info[i].compr_sum = 0;
		piL->compr_info[i].decompr_sum = 0;
	}

	b = pL->frag.listHead;
	while (b) {
		jNode = (struct jffs2_raw_inode *) get_fl_mem(b->offset,
			sizeof(ojNode), &ojNode);
		if (jNode->compr < JFFS2_NUM_COMPR) {
			piL->compr_info[jNode->compr].num_frags++;
			piL->compr_info[jNode->compr].compr_sum += jNode->csize;
			piL->compr_info[jNode->compr].decompr_sum += jNode->dsize;
		}
		b = b->next;
	}
	return 0;
}


static struct b_lists *
jffs2_get_list(struct part_info * part, const char *who)
{
	/* copy requested part_info struct pointer to global location */
	current_part = part;

	if (jffs2_1pass_rescan_needed(part)) {
		if (!jffs2_1pass_build_lists(part)) {
			printf("%s: Failed to scan JFFSv2 file structure\n", who);
			return NULL;
		}
	}
	return (struct b_lists *)part->jffs2_priv;
}


/* Print directory / file contents */
u32
jffs2_1pass_ls(struct part_info * part, const char *fname)
{
	struct b_lists *pl;
	long ret = 1;
	u32 inode;

	if (! (pl = jffs2_get_list(part, "ls")))
		return 0;

	if (! (inode = jffs2_1pass_search_list_inodes(pl, fname, 1))) {
		putstr("ls: Failed to scan jffs2 file structure\r\n");
		return 0;
	}


#if 0
	putLabeledWord("found file at inode = ", inode);
	putLabeledWord("read_inode returns = ", ret);
#endif

	return ret;
}


/* Load a file from flash into memory. fname can be a full path */
u32
jffs2_1pass_load(char *dest, struct part_info * part, const char *fname)
{

	struct b_lists *pl;
	long ret = 1;
	u32 inode;

	if (! (pl  = jffs2_get_list(part, "load")))
		return 0;

	if (! (inode = jffs2_1pass_search_inode(pl, fname, 1))) {
		putstr("load: Failed to find inode\r\n");
		return 0;
	}

	/* Resolve symlinks */
	if (! (inode = jffs2_1pass_resolve_inode(pl, inode))) {
		putstr("load: Failed to resolve inode structure\r\n");
		return 0;
	}

	if ((ret = jffs2_1pass_read_inode(pl, inode, dest)) < 0) {
		putstr("load: Failed to read inode\r\n");
		return 0;
	}

	DEBUGF ("load: loaded '%s' to 0x%lx (%ld bytes)\n", fname,
				(unsigned long) dest, ret);
	return ret;
}

/* Return information about the fs on this partition */
u32
jffs2_1pass_info(struct part_info * part)
{
	struct b_jffs2_info info;
	struct b_lists *pl;
	int i;

	if (! (pl  = jffs2_get_list(part, "info")))
		return 0;

	jffs2_1pass_fill_info(pl, &info);
	for (i = 0; i < JFFS2_NUM_COMPR; i++) {
		printf ("Compression: %s\n"
			"\tfrag count: %d\n"
			"\tcompressed sum: %d\n"
			"\tuncompressed sum: %d\n",
			compr_names[i],
			info.compr_info[i].num_frags,
			info.compr_info[i].compr_sum,
			info.compr_info[i].decompr_sum);
	}
	return 1;
}
