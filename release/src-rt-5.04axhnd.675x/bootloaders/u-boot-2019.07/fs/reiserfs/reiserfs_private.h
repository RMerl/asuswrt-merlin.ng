/* SPDX-License-Identifier: GPL-2.0+ */
/*
 *  Copyright 2000-2002 by Hans Reiser, licensing governed by reiserfs/README
 *
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2000, 2001  Free Software Foundation, Inc.
 *
 *  (C) Copyright 2003 - 2004
 *  Sysgo AG, <www.elinos.com>, Pavel Bartusek <pba@sysgo.com>
 *
 */

/* An implementation for the ReiserFS filesystem ported from GRUB.
 * Some parts of this code (mainly the structures and defines) are
 * from the original reiser fs code, as found in the linux kernel.
 */

#ifndef __BYTE_ORDER
#if defined(__LITTLE_ENDIAN) && !defined(__BIG_ENDIAN)
#define __BYTE_ORDER __LITTLE_ENDIAN
#elif defined(__BIG_ENDIAN) && !defined(__LITTLE_ENDIAN)
#define __BYTE_ORDER __BIG_ENDIAN
#else
#error "unable to define __BYTE_ORDER"
#endif
#endif /* not __BYTE_ORDER */

#define FSYS_BUFLEN  0x8000
#define FSYS_BUF     fsys_buf

/* This is the new super block of a journaling reiserfs system */
struct reiserfs_super_block
{
  __u32 s_block_count;			/* blocks count		*/
  __u32 s_free_blocks;			/* free blocks count	*/
  __u32 s_root_block;			/* root block number	*/
  __u32 s_journal_block;		/* journal block number    */
  __u32 s_journal_dev;			/* journal device number  */
  __u32 s_journal_size;			/* size of the journal on FS creation.	used to make sure they don't overflow it */
  __u32 s_journal_trans_max;		/* max number of blocks in a transaction.  */
  __u32 s_journal_magic;		/* random value made on fs creation */
  __u32 s_journal_max_batch;		/* max number of blocks to batch into a trans */
  __u32 s_journal_max_commit_age;	/* in seconds, how old can an async commit be */
  __u32 s_journal_max_trans_age;	/* in seconds, how old can a transaction be */
  __u16 s_blocksize;			/* block size		*/
  __u16 s_oid_maxsize;			/* max size of object id array	*/
  __u16 s_oid_cursize;			/* current size of object id array */
  __u16 s_state;			/* valid or error	*/
  char s_magic[16];			/* reiserfs magic string indicates that file system is reiserfs */
  __u16 s_tree_height;			/* height of disk tree */
  __u16 s_bmap_nr;			/* amount of bitmap blocks needed to address each block of file system */
  __u16 s_version;
  char s_unused[128];			/* zero filled by mkreiserfs */
};


#define sb_root_block(sbp)	      (__le32_to_cpu((sbp)->s_root_block))
#define sb_journal_block(sbp)	      (__le32_to_cpu((sbp)->s_journal_block))
#define set_sb_journal_block(sbp,v)   ((sbp)->s_journal_block = __cpu_to_le32(v))
#define sb_journal_size(sbp)	      (__le32_to_cpu((sbp)->s_journal_size))
#define sb_blocksize(sbp)	      (__le16_to_cpu((sbp)->s_blocksize))
#define set_sb_blocksize(sbp,v)       ((sbp)->s_blocksize = __cpu_to_le16(v))
#define sb_version(sbp)		      (__le16_to_cpu((sbp)->s_version))
#define set_sb_version(sbp,v)	      ((sbp)->s_version = __cpu_to_le16(v))


#define REISERFS_MAX_SUPPORTED_VERSION 2
#define REISERFS_SUPER_MAGIC_STRING "ReIsErFs"
#define REISER2FS_SUPER_MAGIC_STRING "ReIsEr2Fs"
#define REISER3FS_SUPER_MAGIC_STRING "ReIsEr3Fs"

#define MAX_HEIGHT 7

/* must be correct to keep the desc and commit structs at 4k */
#define JOURNAL_TRANS_HALF 1018

/* first block written in a commit.  */
struct reiserfs_journal_desc {
  __u32 j_trans_id;			/* id of commit */
  __u32 j_len;				/* length of commit. len +1 is the commit block */
  __u32 j_mount_id;			/* mount id of this trans*/
  __u32 j_realblock[JOURNAL_TRANS_HALF]; /* real locations for the first blocks */
  char j_magic[12];
};

/* last block written in a commit */
struct reiserfs_journal_commit {
  __u32 j_trans_id;			/* must match j_trans_id from the desc block */
  __u32 j_len;			/* ditto */
  __u32 j_realblock[JOURNAL_TRANS_HALF]; /* real locations for the last blocks */
  char j_digest[16];			/* md5 sum of all the blocks involved, including desc and commit. not used, kill it */
};

/* this header block gets written whenever a transaction is considered
   fully flushed, and is more recent than the last fully flushed
   transaction.
   fully flushed means all the log blocks and all the real blocks are
   on disk, and this transaction does not need to be replayed.
*/
struct reiserfs_journal_header {
  /* id of last fully flushed transaction */
  __u32 j_last_flush_trans_id;
  /* offset in the log of where to start replay after a crash */
  __u32 j_first_unflushed_offset;
  /* mount id to detect very old transactions */
  __u32 j_mount_id;
};

/* magic string to find desc blocks in the journal */
#define JOURNAL_DESC_MAGIC "ReIsErLB"


/*
 * directories use this key as well as old files
 */
struct offset_v1
{
  /*
   * for regular files this is the offset to the first byte of the
   * body, contained in the object-item, as measured from the start of
   * the entire body of the object.
   *
   * for directory entries, k_offset consists of hash derived from
   * hashing the name and using few bits (23 or more) of the resulting
   * hash, and generation number that allows distinguishing names with
   * hash collisions. If number of collisions overflows generation
   * number, we return EEXIST.	High order bit is 0 always
   */
  __u32 k_offset;
  __u32 k_uniqueness;
};

struct offset_v2 {
  /*
   * for regular files this is the offset to the first byte of the
   * body, contained in the object-item, as measured from the start of
   * the entire body of the object.
   *
   * for directory entries, k_offset consists of hash derived from
   * hashing the name and using few bits (23 or more) of the resulting
   * hash, and generation number that allows distinguishing names with
   * hash collisions. If number of collisions overflows generation
   * number, we return EEXIST.	High order bit is 0 always
   */

#if defined(__LITTLE_ENDIAN_BITFIELD)
	    /* little endian version */
	    __u64 k_offset:60;
	    __u64 k_type: 4;
#elif defined(__BIG_ENDIAN_BITFIELD)
	    /* big endian version */
	    __u64 k_type: 4;
	    __u64 k_offset:60;
#else
#error "__LITTLE_ENDIAN_BITFIELD or __BIG_ENDIAN_BITFIELD must be defined"
#endif
} __attribute__ ((__packed__));

#define TYPE_MAXTYPE 3
#define TYPE_ANY 15

#if (__BYTE_ORDER == __BIG_ENDIAN)
typedef union {
    struct offset_v2 offset_v2;
    __u64 linear;
} __attribute__ ((__packed__)) offset_v2_esafe_overlay;

static inline __u16 offset_v2_k_type( const struct offset_v2 *v2 )
{
    offset_v2_esafe_overlay tmp = *(const offset_v2_esafe_overlay *)v2;
    tmp.linear = __le64_to_cpu( tmp.linear );
    return (tmp.offset_v2.k_type <= TYPE_MAXTYPE)?tmp.offset_v2.k_type:TYPE_ANY;
}

static inline loff_t offset_v2_k_offset( const struct offset_v2 *v2 )
{
    offset_v2_esafe_overlay tmp = *(const offset_v2_esafe_overlay *)v2;
    tmp.linear = __le64_to_cpu( tmp.linear );
    return tmp.offset_v2.k_offset;
}
#elif (__BYTE_ORDER == __LITTLE_ENDIAN)
# define offset_v2_k_type(v2)		((v2)->k_type)
# define offset_v2_k_offset(v2)		((v2)->k_offset)
#else
#error "__BYTE_ORDER must be __LITTLE_ENDIAN or __BIG_ENDIAN"
#endif

struct key
{
  /* packing locality: by default parent directory object id */
  __u32 k_dir_id;
  /* object identifier */
  __u32 k_objectid;
  /* the offset and node type (old and new form) */
  union
  {
    struct offset_v1 v1;
    struct offset_v2 v2;
  }
  u;
};

#define KEY_SIZE (sizeof (struct key))

/* Header of a disk block.  More precisely, header of a formatted leaf
   or internal node, and not the header of an unformatted node. */
struct block_head
{
  __u16 blk_level;	  /* Level of a block in the tree. */
  __u16 blk_nr_item;	  /* Number of keys/items in a block. */
  __u16 blk_free_space;   /* Block free space in bytes. */
  struct key  blk_right_delim_key; /* Right delimiting key for this block (supported for leaf level nodes
				      only) */
};
#define BLKH_SIZE (sizeof (struct block_head))
#define DISK_LEAF_NODE_LEVEL  1 /* Leaf node level.			  */

struct item_head
{
	/* Everything in the tree is found by searching for it based on
	 * its key.*/
	struct key ih_key;
	union {
		/* The free space in the last unformatted node of an
		   indirect item if this is an indirect item.  This
		   equals 0xFFFF iff this is a direct item or stat data
		   item. Note that the key, not this field, is used to
		   determine the item type, and thus which field this
		   union contains. */
		__u16 ih_free_space;
		/* Iff this is a directory item, this field equals the
		   number of directory entries in the directory item. */
		__u16 ih_entry_count;
	} __attribute__ ((__packed__)) u;
	__u16 ih_item_len;	     /* total size of the item body */
	__u16 ih_item_location;      /* an offset to the item body
				      * within the block */
	__u16 ih_version;	     /* 0 for all old items, 2 for new
					ones. Highest bit is set by fsck
					temporary, cleaned after all
					done */
} __attribute__ ((__packed__));

/* size of item header	   */
#define IH_SIZE (sizeof (struct item_head))

#define ITEM_VERSION_1 0
#define ITEM_VERSION_2 1

#define ih_version(ih)	  (__le16_to_cpu((ih)->ih_version))

#define IH_KEY_OFFSET(ih) (ih_version(ih) == ITEM_VERSION_1 \
			   ? __le32_to_cpu((ih)->ih_key.u.v1.k_offset) \
			   : offset_v2_k_offset(&((ih)->ih_key.u.v2)))

#define IH_KEY_ISTYPE(ih, type) (ih_version(ih) == ITEM_VERSION_1 \
				 ? __le32_to_cpu((ih)->ih_key.u.v1.k_uniqueness) == V1_##type \
				 : offset_v2_k_type(&((ih)->ih_key.u.v2)) == V2_##type)

/***************************************************************************/
/*			DISK CHILD					   */
/***************************************************************************/
/* Disk child pointer: The pointer from an internal node of the tree
   to a node that is on disk. */
struct disk_child {
  __u32       dc_block_number;		    /* Disk child's block number. */
  __u16       dc_size;			    /* Disk child's used space.   */
  __u16       dc_reserved;
};

#define DC_SIZE (sizeof(struct disk_child))
#define dc_block_number(dc_p)	(__le32_to_cpu((dc_p)->dc_block_number))


/*
 * old stat data is 32 bytes long. We are going to distinguish new one by
 * different size
 */
struct stat_data_v1
{
    __u16 sd_mode;	/* file type, permissions */
    __u16 sd_nlink;	/* number of hard links */
    __u16 sd_uid;		/* owner */
    __u16 sd_gid;		/* group */
    __u32 sd_size;	/* file size */
    __u32 sd_atime;	/* time of last access */
    __u32 sd_mtime;	/* time file was last modified	*/
    __u32 sd_ctime;	/* time inode (stat data) was last changed (except changes to sd_atime and sd_mtime) */
    union {
	__u32 sd_rdev;
	__u32 sd_blocks;	/* number of blocks file uses */
    } __attribute__ ((__packed__)) u;
    __u32 sd_first_direct_byte; /* first byte of file which is stored
				   in a direct item: except that if it
				   equals 1 it is a symlink and if it
				   equals ~(__u32)0 there is no
				   direct item.  The existence of this
				   field really grates on me. Let's
				   replace it with a macro based on
				   sd_size and our tail suppression
				   policy.  Someday.  -Hans */
} __attribute__ ((__packed__));

#define stat_data_v1(ih)	(ih_version(ih) == ITEM_VERSION_1)
#define sd_v1_mode(sdp)		((sdp)->sd_mode)
#define sd_v1_nlink(sdp)	(__le16_to_cpu((sdp)->sd_nlink))
#define sd_v1_uid(sdp)		(__le16_to_cpu((sdp)->sd_uid))
#define sd_v1_gid(sdp)		(__le16_to_cpu((sdp)->sd_gid))
#define sd_v1_size(sdp)		(__le32_to_cpu((sdp)->sd_size))
#define sd_v1_mtime(sdp)	(__le32_to_cpu((sdp)->sd_mtime))

/* Stat Data on disk (reiserfs version of UFS disk inode minus the
   address blocks) */
struct stat_data {
    __u16 sd_mode;	/* file type, permissions */
    __u16 sd_attrs;	/* persistent inode flags */
    __u32 sd_nlink;	/* number of hard links */
    __u64 sd_size;	/* file size */
    __u32 sd_uid;		/* owner */
    __u32 sd_gid;		/* group */
    __u32 sd_atime;	/* time of last access */
    __u32 sd_mtime;	/* time file was last modified	*/
    __u32 sd_ctime;	/* time inode (stat data) was last changed (except changes to sd_atime and sd_mtime) */
    __u32 sd_blocks;
    union {
	__u32 sd_rdev;
	__u32 sd_generation;
      /*__u32 sd_first_direct_byte; */
      /* first byte of file which is stored in a
				       direct item: except that if it equals 1
				       it is a symlink and if it equals
				       ~(__u32)0 there is no direct item.  The
				       existence of this field really grates
				       on me. Let's replace it with a macro
				       based on sd_size and our tail
				       suppression policy? */
  } __attribute__ ((__packed__)) u;
} __attribute__ ((__packed__));

#define stat_data_v2(ih)	(ih_version(ih) == ITEM_VERSION_2)
#define sd_v2_mode(sdp)		(__le16_to_cpu((sdp)->sd_mode))
#define sd_v2_nlink(sdp)	(__le32_to_cpu((sdp)->sd_nlink))
#define sd_v2_size(sdp)		(__le64_to_cpu((sdp)->sd_size))
#define sd_v2_uid(sdp)		(__le32_to_cpu((sdp)->sd_uid))
#define sd_v2_gid(sdp)		(__le32_to_cpu((sdp)->sd_gid))
#define sd_v2_mtime(sdp)	(__le32_to_cpu((sdp)->sd_mtime))

#define sd_mode(sdp)	     (__le16_to_cpu((sdp)->sd_mode))
#define sd_size(sdp)	     (__le32_to_cpu((sdp)->sd_size))
#define sd_size_hi(sdp)      (__le32_to_cpu((sdp)->sd_size_hi))

struct reiserfs_de_head
{
  __u32 deh_offset;  /* third component of the directory entry key */
  __u32 deh_dir_id;  /* objectid of the parent directory of the
			object, that is referenced by directory entry */
  __u32 deh_objectid;/* objectid of the object, that is referenced by
			directory entry */
  __u16 deh_location;/* offset of name in the whole item */
  __u16 deh_state;   /* whether 1) entry contains stat data (for
			future), and 2) whether entry is hidden
			(unlinked) */
};

#define DEH_SIZE (sizeof (struct reiserfs_de_head))
#define deh_offset(p_deh)	  (__le32_to_cpu((p_deh)->deh_offset))
#define deh_dir_id(p_deh)	  (__le32_to_cpu((p_deh)->deh_dir_id))
#define deh_objectid(p_deh)	  (__le32_to_cpu((p_deh)->deh_objectid))
#define deh_location(p_deh)	  (__le16_to_cpu((p_deh)->deh_location))
#define deh_state(p_deh)	  (__le16_to_cpu((p_deh)->deh_state))


#define DEH_Statdata (1 << 0)			/* not used now */
#define DEH_Visible  (1 << 2)

#define SD_OFFSET  0
#define SD_UNIQUENESS 0
#define DOT_OFFSET 1
#define DOT_DOT_OFFSET 2
#define DIRENTRY_UNIQUENESS 500

#define V1_TYPE_STAT_DATA 0x0
#define V1_TYPE_DIRECT 0xffffffff
#define V1_TYPE_INDIRECT 0xfffffffe
#define V1_TYPE_DIRECTORY_MAX 0xfffffffd
#define V2_TYPE_STAT_DATA 0
#define V2_TYPE_INDIRECT 1
#define V2_TYPE_DIRECT 2
#define V2_TYPE_DIRENTRY 3

#define REISERFS_ROOT_OBJECTID 2
#define REISERFS_ROOT_PARENT_OBJECTID 1
#define REISERFS_DISK_OFFSET_IN_BYTES (64 * 1024)
/* the spot for the super in versions 3.5 - 3.5.11 (inclusive) */
#define REISERFS_OLD_DISK_OFFSET_IN_BYTES (8 * 1024)
#define REISERFS_OLD_BLOCKSIZE 4096

#define S_ISREG(mode) (((mode) & 0170000) == 0100000)
#define S_ISDIR(mode) (((mode) & 0170000) == 0040000)
#define S_ISLNK(mode) (((mode) & 0170000) == 0120000)

#define PATH_MAX       1024	/* include/linux/limits.h */
#define MAX_LINK_COUNT	  5	/* number of symbolic links to follow */

/* The size of the node cache */
#define FSYSREISER_CACHE_SIZE 24*1024
#define FSYSREISER_MIN_BLOCKSIZE SECTOR_SIZE
#define FSYSREISER_MAX_BLOCKSIZE FSYSREISER_CACHE_SIZE / 3

/* Info about currently opened file */
struct fsys_reiser_fileinfo
{
  __u32 k_dir_id;
  __u32 k_objectid;
};

/* In memory info about the currently mounted filesystem */
struct fsys_reiser_info
{
  /* The last read item head */
  struct item_head *current_ih;
  /* The last read item */
  char *current_item;
  /* The information for the currently opened file */
  struct fsys_reiser_fileinfo fileinfo;
  /* The start of the journal */
  __u32 journal_block;
  /* The size of the journal */
  __u32 journal_block_count;
  /* The first valid descriptor block in journal
     (relative to journal_block) */
  __u32 journal_first_desc;

  /* The ReiserFS version. */
  __u16 version;
  /* The current depth of the reiser tree. */
  __u16 tree_depth;
  /* SECTOR_SIZE << blocksize_shift == blocksize. */
  __u8	blocksize_shift;
  /* 1 << full_blocksize_shift == blocksize. */
  __u8	fullblocksize_shift;
  /* The reiserfs block size  (must be a power of 2) */
  __u16 blocksize;
  /* The number of cached tree nodes */
  __u16 cached_slots;
  /* The number of valid transactions in journal */
  __u16 journal_transactions;

  unsigned int blocks[MAX_HEIGHT];
  unsigned int next_key_nr[MAX_HEIGHT];
};

/* The cached s+tree blocks in FSYS_BUF,  see below
 * for a more detailed description.
 */
#define ROOT	 ((char *) ((int) FSYS_BUF))
#define CACHE(i) (ROOT + ((i) << INFO->fullblocksize_shift))
#define LEAF	 CACHE (DISK_LEAF_NODE_LEVEL)

#define BLOCKHEAD(cache) ((struct block_head *) cache)
#define ITEMHEAD	 ((struct item_head  *) ((int) LEAF + BLKH_SIZE))
#define KEY(cache)	 ((struct key	     *) ((int) cache + BLKH_SIZE))
#define DC(cache)	 ((struct disk_child *) \
			  ((int) cache + BLKH_SIZE + KEY_SIZE * nr_item))
/* The fsys_reiser_info block.
 */
#define INFO \
    ((struct fsys_reiser_info *) ((int) FSYS_BUF + FSYSREISER_CACHE_SIZE))
/*
 * The journal cache.  For each transaction it contains the number of
 * blocks followed by the real block numbers of this transaction.
 *
 * If the block numbers of some transaction won't fit in this space,
 * this list is stopped with a 0xffffffff marker and the remaining
 * uncommitted transactions aren't cached.
 */
#define JOURNAL_START	 ((__u32 *) (INFO + 1))
#define JOURNAL_END	 ((__u32 *) (FSYS_BUF + FSYS_BUFLEN))


static __inline__ unsigned long
log2 (unsigned long word)
{
#ifdef __I386__
  __asm__ ("bsfl %1,%0"
	   : "=r" (word)
	   : "r" (word));
  return word;
#else
  int i;

  for(i=0; i<(8*sizeof(word)); i++)
    if ((1<<i) & word)
      return i;

  return 0;
#endif
}

static __inline__ int
is_power_of_two (unsigned long word)
{
  return (word & -word) == word;
}

extern const char *bb_mode_string(int mode);
extern int reiserfs_devread (int sector, int byte_offset, int byte_len, char *buf);
