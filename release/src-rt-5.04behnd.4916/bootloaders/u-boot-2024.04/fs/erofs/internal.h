/* SPDX-License-Identifier: GPL-2.0+ */
#ifndef __EROFS_INTERNAL_H
#define __EROFS_INTERNAL_H

#include "linux/compat.h"
#define __packed __attribute__((__packed__))

#include <linux/stat.h>
#include <linux/bug.h>
#include <linux/err.h>
#include <linux/printk.h>
#include <linux/log2.h>
#include <inttypes.h>
#include "erofs_fs.h"

#define erofs_err(fmt, ...)	\
	pr_err(fmt "\n", ##__VA_ARGS__)

#define erofs_info(fmt, ...)	\
	pr_info(fmt "\n", ##__VA_ARGS__)

#define erofs_dbg(fmt, ...)	\
	pr_debug(fmt "\n", ##__VA_ARGS__)

#define DBG_BUGON(condition)	BUG_ON(condition)

/* no obvious reason to support explicit PAGE_SIZE != 4096 for now */
#if PAGE_SIZE != 4096
#error incompatible PAGE_SIZE is already defined
#endif

#define PAGE_MASK		(~(PAGE_SIZE - 1))

#ifndef EROFS_MAX_BLOCK_SIZE
#define EROFS_MAX_BLOCK_SIZE	PAGE_SIZE
#endif

#define EROFS_ISLOTBITS		5
#define EROFS_SLOTSIZE		(1U << EROFS_ISLOTBITS)

typedef u64 erofs_off_t;
typedef u64 erofs_nid_t;
/* data type for filesystem-wide blocks number */
typedef u32 erofs_blk_t;

#define NULL_ADDR	((unsigned int)-1)
#define NULL_ADDR_UL	((unsigned long)-1)

/* global sbi */
extern struct erofs_sb_info sbi;

#define erofs_blksiz()		(1u << sbi.blkszbits)
#define erofs_blknr(addr)       ((addr) >> sbi.blkszbits)
#define erofs_blkoff(addr)      ((addr) & (erofs_blksiz() - 1))
#define erofs_pos(nr)           ((erofs_off_t)(nr) << sbi.blkszbits)

#define BLK_ROUND_UP(addr)	DIV_ROUND_UP(addr, 1u << sbi.blkszbits)

struct erofs_buffer_head;

struct erofs_device_info {
	u32 blocks;
	u32 mapped_blkaddr;
};

#define EROFS_PACKED_NID_UNALLOCATED	-1

struct erofs_sb_info {
	struct erofs_device_info *devs;

	u64 total_blocks;
	u64 primarydevice_blocks;

	erofs_blk_t meta_blkaddr;
	erofs_blk_t xattr_blkaddr;

	u32 feature_compat;
	u32 feature_incompat;
	u64 build_time;
	u32 build_time_nsec;

	unsigned char islotbits;
	unsigned char blkszbits;

	/* what we really care is nid, rather than ino.. */
	erofs_nid_t root_nid;
	/* used for statfs, f_files - f_favail */
	u64 inos;

	u8 uuid[16];
	char volume_name[16];

	u16 available_compr_algs;
	u16 lz4_max_distance;

	u32 checksum;
	u16 extra_devices;
	union {
		u16 devt_slotoff;		/* used for mkfs */
		u16 device_id_mask;		/* used for others */
	};
	erofs_nid_t packed_nid;

	u32 xattr_prefix_start;
	u8 xattr_prefix_count;
};

static inline erofs_off_t iloc(erofs_nid_t nid)
{
	return erofs_pos(sbi.meta_blkaddr) + (nid << sbi.islotbits);
}

#define EROFS_FEATURE_FUNCS(name, compat, feature) \
static inline bool erofs_sb_has_##name(void) \
{ \
	return sbi.feature_##compat & EROFS_FEATURE_##feature; \
} \
static inline void erofs_sb_set_##name(void) \
{ \
	sbi.feature_##compat |= EROFS_FEATURE_##feature; \
} \
static inline void erofs_sb_clear_##name(void) \
{ \
	sbi.feature_##compat &= ~EROFS_FEATURE_##feature; \
}

EROFS_FEATURE_FUNCS(lz4_0padding, incompat, INCOMPAT_ZERO_PADDING)
EROFS_FEATURE_FUNCS(compr_cfgs, incompat, INCOMPAT_COMPR_CFGS)
EROFS_FEATURE_FUNCS(big_pcluster, incompat, INCOMPAT_BIG_PCLUSTER)
EROFS_FEATURE_FUNCS(chunked_file, incompat, INCOMPAT_CHUNKED_FILE)
EROFS_FEATURE_FUNCS(device_table, incompat, INCOMPAT_DEVICE_TABLE)
EROFS_FEATURE_FUNCS(ztailpacking, incompat, INCOMPAT_ZTAILPACKING)
EROFS_FEATURE_FUNCS(fragments, incompat, INCOMPAT_FRAGMENTS)
EROFS_FEATURE_FUNCS(dedupe, incompat, INCOMPAT_DEDUPE)
EROFS_FEATURE_FUNCS(xattr_prefixes, incompat, INCOMPAT_XATTR_PREFIXES)
EROFS_FEATURE_FUNCS(sb_chksum, compat, COMPAT_SB_CHKSUM)

#define EROFS_I_EA_INITED	(1 << 0)
#define EROFS_I_Z_INITED	(1 << 1)

struct erofs_inode {
	struct list_head i_hash, i_subdirs, i_xattrs;

	union {
		/* (erofsfuse) runtime flags */
		unsigned int flags;
		/* (mkfs.erofs) device ID containing source file */
		u32 dev;
		/* (mkfs.erofs) queued sub-directories blocking dump */
		u32 subdirs_queued;
	};
	unsigned int i_count;
	struct erofs_inode *i_parent;

	umode_t i_mode;
	erofs_off_t i_size;

	u64 i_ino[2];
	u32 i_uid;
	u32 i_gid;
	u64 i_mtime;
	u32 i_mtime_nsec;
	u32 i_nlink;

	union {
		u32 i_blkaddr;
		u32 i_blocks;
		u32 i_rdev;
		struct {
			unsigned short	chunkformat;
			unsigned char	chunkbits;
		};
	} u;

	char *i_srcpath;

	unsigned char datalayout;
	unsigned char inode_isize;
	/* inline tail-end packing size */
	unsigned short idata_size;
	bool compressed_idata;
	bool lazy_tailblock;

	unsigned int xattr_isize;
	unsigned int extent_isize;

	unsigned int xattr_shared_count;
	unsigned int *xattr_shared_xattrs;

	erofs_nid_t nid;
	struct erofs_buffer_head *bh;
	struct erofs_buffer_head *bh_inline, *bh_data;

	void *idata;

	/* (ztailpacking) in order to recover uncompressed EOF data */
	void *eof_tailraw;
	unsigned int eof_tailrawsize;

	union {
		void *compressmeta;
		void *chunkindexes;
		struct {
			uint16_t z_advise;
			uint8_t  z_algorithmtype[2];
			uint8_t  z_logical_clusterbits;
			uint8_t  z_physical_clusterblks;
			uint64_t z_tailextent_headlcn;
			unsigned int    z_idataoff;
#define z_idata_size	idata_size
		};
	};
	uint64_t capabilities;
	erofs_off_t fragmentoff;
	unsigned int fragment_size;
};

static inline bool is_inode_layout_compression(struct erofs_inode *inode)
{
	return erofs_inode_is_data_compressed(inode->datalayout);
}

static inline unsigned int erofs_bitrange(unsigned int value, unsigned int bit,
					  unsigned int bits)
{
	return (value >> bit) & ((1 << bits) - 1);
}

static inline unsigned int erofs_inode_version(unsigned int value)
{
	return erofs_bitrange(value, EROFS_I_VERSION_BIT,
			      EROFS_I_VERSION_BITS);
}

static inline unsigned int erofs_inode_datalayout(unsigned int value)
{
	return erofs_bitrange(value, EROFS_I_DATALAYOUT_BIT,
			      EROFS_I_DATALAYOUT_BITS);
}

#define IS_ROOT(x)	((x) == (x)->i_parent)

struct erofs_dentry {
	struct list_head d_child;	/* child of parent list */

	unsigned int type;
	char name[EROFS_NAME_LEN];
	union {
		struct erofs_inode *inode;
		erofs_nid_t nid;
	};
};

static inline bool is_dot_dotdot_len(const char *name, unsigned int len)
{
	if (len >= 1 && name[0] != '.')
		return false;

	return len == 1 || (len == 2 && name[1] == '.');
}

static inline bool is_dot_dotdot(const char *name)
{
	if (name[0] != '.')
		return false;

	return name[1] == '\0' || (name[1] == '.' && name[2] == '\0');
}

enum {
	BH_Meta,
	BH_Mapped,
	BH_Encoded,
	BH_FullMapped,
	BH_Fragment,
	BH_Partialref,
};

/* Has a disk mapping */
#define EROFS_MAP_MAPPED	(1 << BH_Mapped)
/* Located in metadata (could be copied from bd_inode) */
#define EROFS_MAP_META		(1 << BH_Meta)
/* The extent is encoded */
#define EROFS_MAP_ENCODED	(1 << BH_Encoded)
/* The length of extent is full */
#define EROFS_MAP_FULL_MAPPED	(1 << BH_FullMapped)
/* Located in the special packed inode */
#define EROFS_MAP_FRAGMENT	(1 << BH_Fragment)
/* The extent refers to partial decompressed data */
#define EROFS_MAP_PARTIAL_REF	(1 << BH_Partialref)

struct erofs_map_blocks {
	char mpage[EROFS_MAX_BLOCK_SIZE];

	erofs_off_t m_pa, m_la;
	u64 m_plen, m_llen;

	unsigned short m_deviceid;
	char m_algorithmformat;
	unsigned int m_flags;
	erofs_blk_t index;
};

/*
 * Used to get the exact decompressed length, e.g. fiemap (consider lookback
 * approach instead if possible since it's more metadata lightweight.)
 */
#define EROFS_GET_BLOCKS_FIEMAP	0x0002
/* Used to map tail extent for tailpacking inline or fragment pcluster */
#define EROFS_GET_BLOCKS_FINDTAIL	0x0008

enum {
	Z_EROFS_COMPRESSION_SHIFTED = Z_EROFS_COMPRESSION_MAX,
	Z_EROFS_COMPRESSION_INTERLACED,
	Z_EROFS_COMPRESSION_RUNTIME_MAX
};

struct erofs_map_dev {
	erofs_off_t m_pa;
	unsigned int m_deviceid;
};

/* fs.c */
int erofs_blk_read(void *buf, erofs_blk_t start, u32 nblocks);
int erofs_dev_read(int device_id, void *buf, u64 offset, size_t len);

/* super.c */
int erofs_read_superblock(void);
void erofs_put_super(void);

/* namei.c */
int erofs_read_inode_from_disk(struct erofs_inode *vi);
int erofs_ilookup(const char *path, struct erofs_inode *vi);
int erofs_read_inode_from_disk(struct erofs_inode *vi);

/* data.c */
int erofs_pread(struct erofs_inode *inode, char *buf,
		erofs_off_t count, erofs_off_t offset);
int erofs_map_blocks(struct erofs_inode *inode, struct erofs_map_blocks *map,
		     int flags);
int erofs_map_dev(struct erofs_map_dev *map);
int erofs_read_one_data(struct erofs_map_blocks *map, char *buffer, u64 offset,
			size_t len);
int z_erofs_read_one_data(struct erofs_inode *inode,
			  struct erofs_map_blocks *map, char *raw, char *buffer,
			  erofs_off_t skip, erofs_off_t length, bool trimmed);

static inline int erofs_get_occupied_size(const struct erofs_inode *inode,
					  erofs_off_t *size)
{
	*size = 0;
	switch (inode->datalayout) {
	case EROFS_INODE_FLAT_INLINE:
	case EROFS_INODE_FLAT_PLAIN:
	case EROFS_INODE_CHUNK_BASED:
		*size = inode->i_size;
		break;
	case EROFS_INODE_COMPRESSED_FULL:
	case EROFS_INODE_COMPRESSED_COMPACT:
		*size = inode->u.i_blocks * erofs_blksiz();
		break;
	default:
		return -EOPNOTSUPP;
	}
	return 0;
}

/* data.c */
int erofs_getxattr(struct erofs_inode *vi, const char *name, char *buffer,
		   size_t buffer_size);
int erofs_listxattr(struct erofs_inode *vi, char *buffer, size_t buffer_size);

/* zmap.c */
int z_erofs_fill_inode(struct erofs_inode *vi);
int z_erofs_map_blocks_iter(struct erofs_inode *vi,
			    struct erofs_map_blocks *map, int flags);

#ifdef EUCLEAN
#define EFSCORRUPTED	EUCLEAN		/* Filesystem is corrupted */
#else
#define EFSCORRUPTED	EIO
#endif

#define CRC32C_POLY_LE	0x82F63B78
static inline u32 erofs_crc32c(u32 crc, const u8 *in, size_t len)
{
	int i;

	while (len--) {
		crc ^= *in++;
		for (i = 0; i < 8; i++)
			crc = (crc >> 1) ^ ((crc & 1) ? CRC32C_POLY_LE : 0);
	}
	return crc;
}

#endif
