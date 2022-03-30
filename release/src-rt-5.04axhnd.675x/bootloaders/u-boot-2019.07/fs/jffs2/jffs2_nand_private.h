#ifndef jffs2_private_h
#define jffs2_private_h

#include <jffs2/jffs2.h>

struct b_node {
	struct b_node *next;
};

struct b_inode {
	struct b_inode *next;
	u32 offset;	/* physical offset to beginning of real inode */
	u32 version;
	u32 ino;
	u32 isize;
	u32 csize;
};

struct b_dirent {
	struct b_dirent *next;
	u32 offset;	/* physical offset to beginning of real dirent */
	u32 version;
	u32 pino;
	u32 ino;
	unsigned int nhash;
	unsigned char nsize;
	unsigned char type;
};

struct b_list {
	struct b_node *listTail;
	struct b_node *listHead;
	unsigned int listCount;
	struct mem_block *listMemBase;
};

struct b_lists {
	char *partOffset;
	struct b_list dir;
	struct b_list frag;
};

struct b_compr_info {
	u32 num_frags;
	u32 compr_sum;
	u32 decompr_sum;
};

struct b_jffs2_info {
	struct b_compr_info compr_info[JFFS2_NUM_COMPR];
};

static inline int
hdr_crc(struct jffs2_unknown_node *node)
{
#if 1
	u32 crc = crc32_no_comp(0, (unsigned char *)node, sizeof(struct jffs2_unknown_node) - 4);
#else
	/* what's the semantics of this? why is this here? */
	u32 crc = crc32_no_comp(~0, (unsigned char *)node, sizeof(struct jffs2_unknown_node) - 4);

	crc ^= ~0;
#endif
	if (node->hdr_crc != crc) {
		return 0;
	} else {
		return 1;
	}
}

static inline int
dirent_crc(struct jffs2_raw_dirent *node)
{
	if (node->node_crc != crc32_no_comp(0, (unsigned char *)node, sizeof(struct jffs2_raw_dirent) - 8)) {
		return 0;
	} else {
		return 1;
	}
}

static inline int
dirent_name_crc(struct jffs2_raw_dirent *node)
{
	if (node->name_crc != crc32_no_comp(0, (unsigned char *)&(node->name), node->nsize)) {
		return 0;
	} else {
		return 1;
	}
}

static inline int
inode_crc(struct jffs2_raw_inode *node)
{
	if (node->node_crc != crc32_no_comp(0, (unsigned char *)node, sizeof(struct jffs2_raw_inode) - 8)) {
		return 0;
	} else {
		return 1;
	}
}

/* Borrowed from include/linux/dcache.h */

/* Name hashing routines. Initial hash value */
/* Hash courtesy of the R5 hash in reiserfs modulo sign bits */
#define init_name_hash()		0

/* partial hash update function. Assume roughly 4 bits per character */
static inline unsigned long
partial_name_hash(unsigned long c, unsigned long prevhash)
{
	return (prevhash + (c << 4) + (c >> 4)) * 11;
}

/*
 * Finally: cut down the number of bits to a int value (and try to avoid
 * losing bits)
 */
static inline unsigned long end_name_hash(unsigned long hash)
{
	return (unsigned int) hash;
}

/* Compute the hash for a name string. */
static inline unsigned int
full_name_hash(const unsigned char *name, unsigned int len)
{
	unsigned long hash = init_name_hash();
	while (len--)
		hash = partial_name_hash(*name++, hash);
	return end_name_hash(hash);
}

#endif /* jffs2_private.h */
