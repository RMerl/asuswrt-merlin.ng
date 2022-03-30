#ifndef jffs2_private_h
#define jffs2_private_h

#include <jffs2/jffs2.h>


struct b_node {
	u32 offset;
	struct b_node *next;
	enum { CRC_UNKNOWN = 0, CRC_OK, CRC_BAD } datacrc;
};

struct b_list {
	struct b_node *listTail;
	struct b_node *listHead;
#ifdef CONFIG_SYS_JFFS2_SORT_FRAGMENTS
	struct b_node *listLast;
	int (*listCompare)(struct b_node *new, struct b_node *node);
	u32 listLoops;
#endif
	u32 listCount;
	struct mem_block *listMemBase;
};

struct b_lists {
	struct b_list dir;
	struct b_list frag;
	void *readbuf;
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

static inline int
data_crc(struct jffs2_raw_inode *node)
{
	if (node->data_crc != crc32_no_comp(0, (unsigned char *)
					    ((int) &node->node_crc + sizeof (node->node_crc)),
					     node->csize)) {
		return 0;
	} else {
		return 1;
	}
}

#if defined(CONFIG_SYS_JFFS2_SORT_FRAGMENTS)
/* External merge sort. */
int sort_list(struct b_list *list);
#endif
#endif /* jffs2_private.h */
