// SPDX-License-Identifier: GPL-2.0+
/*
 * BTRFS filesystem implementation for U-Boot
 *
 * 2017 Marek Behún, CZ.NIC, kabel@kernel.org
 */

#include <linux/kernel.h>
#include <log.h>
#include <malloc.h>
#include <memalign.h>
#include "btrfs.h"
#include "disk-io.h"

static const struct btrfs_csum {
	u16 size;
	const char name[14];
} btrfs_csums[] = {
	[BTRFS_CSUM_TYPE_CRC32]		= {  4, "crc32c" },
	[BTRFS_CSUM_TYPE_XXHASH]	= {  8, "xxhash64" },
	[BTRFS_CSUM_TYPE_SHA256]	= { 32, "sha256" },
	[BTRFS_CSUM_TYPE_BLAKE2]	= { 32, "blake2" },
};

u16 btrfs_super_csum_size(const struct btrfs_super_block *sb)
{
	const u16 csum_type = btrfs_super_csum_type(sb);

	return btrfs_csums[csum_type].size;
}

const char *btrfs_super_csum_name(u16 csum_type)
{
	return btrfs_csums[csum_type].name;
}

size_t btrfs_super_num_csums(void)
{
	return ARRAY_SIZE(btrfs_csums);
}

u16 btrfs_csum_type_size(u16 csum_type)
{
	return btrfs_csums[csum_type].size;
}

struct btrfs_path *btrfs_alloc_path(void)
{
	struct btrfs_path *path;
	path = kzalloc(sizeof(struct btrfs_path), GFP_NOFS);
	return path;
}

void btrfs_free_path(struct btrfs_path *p)
{
	if (!p)
		return;
	btrfs_release_path(p);
	kfree(p);
}

void btrfs_release_path(struct btrfs_path *p)
{
	int i;
	for (i = 0; i < BTRFS_MAX_LEVEL; i++) {
		if (!p->nodes[i])
			continue;
		free_extent_buffer(p->nodes[i]);
	}
	memset(p, 0, sizeof(*p));
}

int btrfs_comp_cpu_keys(const struct btrfs_key *k1, const struct btrfs_key *k2)
{
	if (k1->objectid > k2->objectid)
		return 1;
	if (k1->objectid < k2->objectid)
		return -1;
	if (k1->type > k2->type)
		return 1;
	if (k1->type < k2->type)
		return -1;
	if (k1->offset > k2->offset)
		return 1;
	if (k1->offset < k2->offset)
		return -1;
	return 0;
}

static int btrfs_comp_keys(struct btrfs_disk_key *disk,
			     const struct btrfs_key *k2)
{
	struct btrfs_key k1;

	btrfs_disk_key_to_cpu(&k1, disk);
	return btrfs_comp_cpu_keys(&k1, k2);
}

enum btrfs_tree_block_status
btrfs_check_node(struct btrfs_fs_info *fs_info,
		 struct btrfs_disk_key *parent_key, struct extent_buffer *buf)
{
	int i;
	struct btrfs_key cpukey;
	struct btrfs_disk_key key;
	u32 nritems = btrfs_header_nritems(buf);
	enum btrfs_tree_block_status ret = BTRFS_TREE_BLOCK_INVALID_NRITEMS;

	if (nritems == 0 || nritems > BTRFS_NODEPTRS_PER_BLOCK(fs_info))
		goto fail;

	ret = BTRFS_TREE_BLOCK_INVALID_PARENT_KEY;
	if (parent_key && parent_key->type) {
		btrfs_node_key(buf, &key, 0);
		if (memcmp(parent_key, &key, sizeof(key)))
			goto fail;
	}
	ret = BTRFS_TREE_BLOCK_BAD_KEY_ORDER;
	for (i = 0; nritems > 1 && i < nritems - 2; i++) {
		btrfs_node_key(buf, &key, i);
		btrfs_node_key_to_cpu(buf, &cpukey, i + 1);
		if (btrfs_comp_keys(&key, &cpukey) >= 0)
			goto fail;
	}
	return BTRFS_TREE_BLOCK_CLEAN;
fail:
	return ret;
}

enum btrfs_tree_block_status
btrfs_check_leaf(struct btrfs_fs_info *fs_info,
		 struct btrfs_disk_key *parent_key, struct extent_buffer *buf)
{
	int i;
	struct btrfs_key cpukey;
	struct btrfs_disk_key key;
	u32 nritems = btrfs_header_nritems(buf);
	enum btrfs_tree_block_status ret = BTRFS_TREE_BLOCK_INVALID_NRITEMS;

	if (nritems * sizeof(struct btrfs_item) > buf->len)  {
		fprintf(stderr, "invalid number of items %llu\n",
			(unsigned long long)buf->start);
		goto fail;
	}

	if (btrfs_header_level(buf) != 0) {
		ret = BTRFS_TREE_BLOCK_INVALID_LEVEL;
		fprintf(stderr, "leaf is not a leaf %llu\n",
		       (unsigned long long)btrfs_header_bytenr(buf));
		goto fail;
	}
	if (btrfs_leaf_free_space(buf) < 0) {
		ret = BTRFS_TREE_BLOCK_INVALID_FREE_SPACE;
		fprintf(stderr, "leaf free space incorrect %llu %d\n",
			(unsigned long long)btrfs_header_bytenr(buf),
			btrfs_leaf_free_space(buf));
		goto fail;
	}

	if (nritems == 0)
		return BTRFS_TREE_BLOCK_CLEAN;

	btrfs_item_key(buf, &key, 0);
	if (parent_key && parent_key->type &&
	    memcmp(parent_key, &key, sizeof(key))) {
		ret = BTRFS_TREE_BLOCK_INVALID_PARENT_KEY;
		fprintf(stderr, "leaf parent key incorrect %llu\n",
		       (unsigned long long)btrfs_header_bytenr(buf));
		goto fail;
	}
	for (i = 0; nritems > 1 && i < nritems - 1; i++) {
		btrfs_item_key(buf, &key, i);
		btrfs_item_key_to_cpu(buf, &cpukey, i + 1);
		if (btrfs_comp_keys(&key, &cpukey) >= 0) {
			ret = BTRFS_TREE_BLOCK_BAD_KEY_ORDER;
			fprintf(stderr, "bad key ordering %d %d\n", i, i+1);
			goto fail;
		}
		if (btrfs_item_offset_nr(buf, i) !=
			btrfs_item_end_nr(buf, i + 1)) {
			ret = BTRFS_TREE_BLOCK_INVALID_OFFSETS;
			fprintf(stderr, "incorrect offsets %u %u\n",
				btrfs_item_offset_nr(buf, i),
				btrfs_item_end_nr(buf, i + 1));
			goto fail;
		}
		if (i == 0 && btrfs_item_end_nr(buf, i) !=
		    BTRFS_LEAF_DATA_SIZE(fs_info)) {
			ret = BTRFS_TREE_BLOCK_INVALID_OFFSETS;
			fprintf(stderr, "bad item end %u wanted %u\n",
				btrfs_item_end_nr(buf, i),
				(unsigned)BTRFS_LEAF_DATA_SIZE(fs_info));
			goto fail;
		}
	}

	for (i = 0; i < nritems; i++) {
		if (btrfs_item_end_nr(buf, i) >
				BTRFS_LEAF_DATA_SIZE(fs_info)) {
			btrfs_item_key(buf, &key, 0);
			ret = BTRFS_TREE_BLOCK_INVALID_OFFSETS;
			fprintf(stderr, "slot end outside of leaf %llu > %llu\n",
				(unsigned long long)btrfs_item_end_nr(buf, i),
				(unsigned long long)BTRFS_LEAF_DATA_SIZE(
					fs_info));
			goto fail;
		}
	}

	return BTRFS_TREE_BLOCK_CLEAN;
fail:
	return ret;
}

static int noinline check_block(struct btrfs_fs_info *fs_info,
				struct btrfs_path *path, int level)
{
	struct btrfs_disk_key key;
	struct btrfs_disk_key *key_ptr = NULL;
	struct extent_buffer *parent;
	enum btrfs_tree_block_status ret;

	if (path->nodes[level + 1]) {
		parent = path->nodes[level + 1];
		btrfs_node_key(parent, &key, path->slots[level + 1]);
		key_ptr = &key;
	}
	if (level == 0)
		ret = btrfs_check_leaf(fs_info, key_ptr, path->nodes[0]);
	else
		ret = btrfs_check_node(fs_info, key_ptr, path->nodes[level]);
	if (ret == BTRFS_TREE_BLOCK_CLEAN)
		return 0;
	return -EIO;
}

/*
 * search for key in the extent_buffer.  The items start at offset p,
 * and they are item_size apart.  There are 'max' items in p.
 *
 * the slot in the array is returned via slot, and it points to
 * the place where you would insert key if it is not found in
 * the array.
 *
 * slot may point to max if the key is bigger than all of the keys
 */
static int generic_bin_search(struct extent_buffer *eb, unsigned long p,
			      int item_size, const struct btrfs_key *key,
			      int max, int *slot)
{
	int low = 0;
	int high = max;
	int mid;
	int ret;
	unsigned long offset;
	struct btrfs_disk_key *tmp;

	while(low < high) {
		mid = (low + high) / 2;
		offset = p + mid * item_size;

		tmp = (struct btrfs_disk_key *)(eb->data + offset);
		ret = btrfs_comp_keys(tmp, key);

		if (ret < 0)
			low = mid + 1;
		else if (ret > 0)
			high = mid;
		else {
			*slot = mid;
			return 0;
		}
	}
	*slot = low;
	return 1;
}

/*
 * simple bin_search frontend that does the right thing for
 * leaves vs nodes
 */
int btrfs_bin_search(struct extent_buffer *eb, const struct btrfs_key *key,
		     int *slot)
{
	if (btrfs_header_level(eb) == 0)
		return generic_bin_search(eb,
					  offsetof(struct btrfs_leaf, items),
					  sizeof(struct btrfs_item),
					  key, btrfs_header_nritems(eb),
					  slot);
	else
		return generic_bin_search(eb,
					  offsetof(struct btrfs_node, ptrs),
					  sizeof(struct btrfs_key_ptr),
					  key, btrfs_header_nritems(eb),
					  slot);
}

struct extent_buffer *read_node_slot(struct btrfs_fs_info *fs_info,
				   struct extent_buffer *parent, int slot)
{
	struct extent_buffer *ret;
	int level = btrfs_header_level(parent);

	if (slot < 0)
		return NULL;
	if (slot >= btrfs_header_nritems(parent))
		return NULL;

	if (level == 0)
		return NULL;

	ret = read_tree_block(fs_info, btrfs_node_blockptr(parent, slot),
		       btrfs_node_ptr_generation(parent, slot));
	if (!extent_buffer_uptodate(ret))
		return ERR_PTR(-EIO);

	if (btrfs_header_level(ret) != level - 1) {
		error("child eb corrupted: parent bytenr=%llu item=%d parent level=%d child level=%d",
		      btrfs_header_bytenr(parent), slot,
		      btrfs_header_level(parent), btrfs_header_level(ret));
		free_extent_buffer(ret);
		return ERR_PTR(-EIO);
	}
	return ret;
}

int btrfs_find_item(struct btrfs_root *fs_root, struct btrfs_path *found_path,
		u64 iobjectid, u64 ioff, u8 key_type,
		struct btrfs_key *found_key)
{
	int ret;
	struct btrfs_key key;
	struct extent_buffer *eb;
	struct btrfs_path *path;

	key.type = key_type;
	key.objectid = iobjectid;
	key.offset = ioff;

	if (found_path == NULL) {
		path = btrfs_alloc_path();
		if (!path)
			return -ENOMEM;
	} else
		path = found_path;

	ret = btrfs_search_slot(NULL, fs_root, &key, path, 0, 0);
	if ((ret < 0) || (found_key == NULL))
		goto out;

	eb = path->nodes[0];
	if (ret && path->slots[0] >= btrfs_header_nritems(eb)) {
		ret = btrfs_next_leaf(fs_root, path);
		if (ret)
			goto out;
		eb = path->nodes[0];
	}

	btrfs_item_key_to_cpu(eb, found_key, path->slots[0]);
	if (found_key->type != key.type ||
			found_key->objectid != key.objectid) {
		ret = 1;
		goto out;
	}

out:
	if (path != found_path)
		btrfs_free_path(path);
	return ret;
}

/*
 * look for key in the tree.  path is filled in with nodes along the way
 * if key is found, we return zero and you can find the item in the leaf
 * level of the path (level 0)
 *
 * If the key isn't found, the path points to the slot where it should
 * be inserted, and 1 is returned.  If there are other errors during the
 * search a negative error number is returned.
 *
 * if ins_len > 0, nodes and leaves will be split as we walk down the
 * tree.  if ins_len < 0, nodes will be merged as we walk down the tree (if
 * possible)
 *
 * NOTE: This version has no COW ability, thus we expect trans == NULL,
 * ins_len == 0 and cow == 0.
 */
int btrfs_search_slot(struct btrfs_trans_handle *trans,
		struct btrfs_root *root, const struct btrfs_key *key,
		struct btrfs_path *p, int ins_len, int cow)
{
	struct extent_buffer *b;
	int slot;
	int ret;
	int level;
	struct btrfs_fs_info *fs_info = root->fs_info;
	u8 lowest_level = 0;

	assert(trans == NULL && ins_len == 0 && cow == 0);
	lowest_level = p->lowest_level;
	WARN_ON(lowest_level && ins_len > 0);
	WARN_ON(p->nodes[0] != NULL);

	b = root->node;
	extent_buffer_get(b);
	while (b) {
		level = btrfs_header_level(b);
		/*
		if (cow) {
			int wret;
			wret = btrfs_cow_block(trans, root, b,
					       p->nodes[level + 1],
					       p->slots[level + 1],
					       &b);
			if (wret) {
				free_extent_buffer(b);
				return wret;
			}
		}
		*/
		BUG_ON(!cow && ins_len);
		if (level != btrfs_header_level(b))
			WARN_ON(1);
		level = btrfs_header_level(b);
		p->nodes[level] = b;
		ret = check_block(fs_info, p, level);
		if (ret)
			return -1;
		ret = btrfs_bin_search(b, key, &slot);
		if (level != 0) {
			if (ret && slot > 0)
				slot -= 1;
			p->slots[level] = slot;
			/*
			if ((p->search_for_split || ins_len > 0) &&
			    btrfs_header_nritems(b) >=
			    BTRFS_NODEPTRS_PER_BLOCK(fs_info) - 3) {
				int sret = split_node(trans, root, p, level);
				BUG_ON(sret > 0);
				if (sret)
					return sret;
				b = p->nodes[level];
				slot = p->slots[level];
			} else if (ins_len < 0) {
				int sret = balance_level(trans, root, p,
							 level);
				if (sret)
					return sret;
				b = p->nodes[level];
				if (!b) {
					btrfs_release_path(p);
					goto again;
				}
				slot = p->slots[level];
				BUG_ON(btrfs_header_nritems(b) == 1);
			}
			*/
			/* this is only true while dropping a snapshot */
			if (level == lowest_level)
				break;

			b = read_node_slot(fs_info, b, slot);
			if (!extent_buffer_uptodate(b))
				return -EIO;
		} else {
			p->slots[level] = slot;
			/*
			if (ins_len > 0 &&
			    ins_len > btrfs_leaf_free_space(b)) {
				int sret = split_leaf(trans, root, key,
						      p, ins_len, ret == 0);
				BUG_ON(sret > 0);
				if (sret)
					return sret;
			}
			*/
			return ret;
		}
	}
	return 1;
}

/*
 * Helper to use instead of search slot if no exact match is needed but
 * instead the next or previous item should be returned.
 * When find_higher is true, the next higher item is returned, the next lower
 * otherwise.
 * When return_any and find_higher are both true, and no higher item is found,
 * return the next lower instead.
 * When return_any is true and find_higher is false, and no lower item is found,
 * return the next higher instead.
 * It returns 0 if any item is found, 1 if none is found (tree empty), and
 * < 0 on error
 */
int btrfs_search_slot_for_read(struct btrfs_root *root,
			       const struct btrfs_key *key,
			       struct btrfs_path *p, int find_higher,
			       int return_any)
{
	int ret;
	struct extent_buffer *leaf;

again:
	ret = btrfs_search_slot(NULL, root, key, p, 0, 0);
	if (ret <= 0)
		 return ret;
	/*
	 * A return value of 1 means the path is at the position where the item
	 * should be inserted. Normally this is the next bigger item, but in
	 * case the previous item is the last in a leaf, path points to the
	 * first free slot in the previous leaf, i.e. at an invalid item.
	 */
	leaf = p->nodes[0];

	if (find_higher) {
		if (p->slots[0] >= btrfs_header_nritems(leaf)) {
			ret = btrfs_next_leaf(root, p);
			if (ret <= 0)
				return ret;
			if (!return_any)
				return 1;
			/*
			 * No higher item found, return the next lower instead
			 */
			return_any = 0;
			find_higher = 0;
			btrfs_release_path(p);
			goto again;
		}
	} else {
		if (p->slots[0] == 0) {
			ret = btrfs_prev_leaf(root, p);
			if (ret < 0)
				return ret;
			if (!ret) {
				leaf = p->nodes[0];
				if (p->slots[0] == btrfs_header_nritems(leaf))
					p->slots[0]--;
				return 0;
			}
			if (!return_any)
				return 1;
			/*
			 * No lower item found, return the next higher instead
			 */
			return_any = 0;
			find_higher = 1;
			btrfs_release_path(p);
			goto again;
		} else {
			--p->slots[0];
		}
	}
	return 0;
}

/*
 * how many bytes are required to store the items in a leaf.  start
 * and nr indicate which items in the leaf to check.  This totals up the
 * space used both by the item structs and the item data
 */
static int leaf_space_used(struct extent_buffer *l, int start, int nr)
{
	int data_len;
	int nritems = btrfs_header_nritems(l);
	int end = min(nritems, start + nr) - 1;

	if (!nr)
		return 0;
	data_len = btrfs_item_end_nr(l, start);
	data_len = data_len - btrfs_item_offset_nr(l, end);
	data_len += sizeof(struct btrfs_item) * nr;
	WARN_ON(data_len < 0);
	return data_len;
}

/*
 * The space between the end of the leaf items and
 * the start of the leaf data.  IOW, how much room
 * the leaf has left for both items and data
 */
int btrfs_leaf_free_space(struct extent_buffer *leaf)
{
	int nritems = btrfs_header_nritems(leaf);
	u32 leaf_data_size;
	int ret;

	BUG_ON(leaf->fs_info && leaf->fs_info->nodesize != leaf->len);
	leaf_data_size = __BTRFS_LEAF_DATA_SIZE(leaf->len);
	ret = leaf_data_size - leaf_space_used(leaf, 0 ,nritems);
	if (ret < 0) {
		printk("leaf free space ret %d, leaf data size %u, used %d nritems %d\n",
		       ret, leaf_data_size, leaf_space_used(leaf, 0, nritems),
		       nritems);
	}
	return ret;
}

/*
 * walk up the tree as far as required to find the previous leaf.
 * returns 0 if it found something or 1 if there are no lesser leaves.
 * returns < 0 on io errors.
 */
int btrfs_prev_leaf(struct btrfs_root *root, struct btrfs_path *path)
{
	int slot;
	int level = 1;
	struct extent_buffer *c;
	struct extent_buffer *next = NULL;
	struct btrfs_fs_info *fs_info = root->fs_info;

	while(level < BTRFS_MAX_LEVEL) {
		if (!path->nodes[level])
			return 1;

		slot = path->slots[level];
		c = path->nodes[level];
		if (slot == 0) {
			level++;
			if (level == BTRFS_MAX_LEVEL)
				return 1;
			continue;
		}
		slot--;

		next = read_node_slot(fs_info, c, slot);
		if (!extent_buffer_uptodate(next)) {
			if (IS_ERR(next))
				return PTR_ERR(next);
			return -EIO;
		}
		break;
	}
	path->slots[level] = slot;
	while(1) {
		level--;
		c = path->nodes[level];
		free_extent_buffer(c);
		slot = btrfs_header_nritems(next);
		if (slot != 0)
			slot--;
		path->nodes[level] = next;
		path->slots[level] = slot;
		if (!level)
			break;
		next = read_node_slot(fs_info, next, slot);
		if (!extent_buffer_uptodate(next)) {
			if (IS_ERR(next))
				return PTR_ERR(next);
			return -EIO;
		}
	}
	return 0;
}

/*
 * Walk up the tree as far as necessary to find the next sibling tree block.
 * More generic version of btrfs_next_leaf(), as it could find sibling nodes
 * if @path->lowest_level is not 0.
 *
 * returns 0 if it found something or 1 if there are no greater leaves.
 * returns < 0 on io errors.
 */
int btrfs_next_sibling_tree_block(struct btrfs_fs_info *fs_info,
				  struct btrfs_path *path)
{
	int slot;
	int level = path->lowest_level + 1;
	struct extent_buffer *c;
	struct extent_buffer *next = NULL;

	BUG_ON(path->lowest_level + 1 >= BTRFS_MAX_LEVEL);
	do {
		if (!path->nodes[level])
			return 1;

		slot = path->slots[level] + 1;
		c = path->nodes[level];
		if (slot >= btrfs_header_nritems(c)) {
			level++;
			if (level == BTRFS_MAX_LEVEL)
				return 1;
			continue;
		}

		next = read_node_slot(fs_info, c, slot);
		if (!extent_buffer_uptodate(next))
			return -EIO;
		break;
	} while (level < BTRFS_MAX_LEVEL);
	path->slots[level] = slot;
	while(1) {
		level--;
		c = path->nodes[level];
		free_extent_buffer(c);
		path->nodes[level] = next;
		path->slots[level] = 0;
		if (level == path->lowest_level)
			break;
		next = read_node_slot(fs_info, next, 0);
		if (!extent_buffer_uptodate(next))
			return -EIO;
	}
	return 0;
}

int btrfs_previous_item(struct btrfs_root *root,
			struct btrfs_path *path, u64 min_objectid,
			int type)
{
	struct btrfs_key found_key;
	struct extent_buffer *leaf;
	u32 nritems;
	int ret;

	while(1) {
		if (path->slots[0] == 0) {
			ret = btrfs_prev_leaf(root, path);
			if (ret != 0)
				return ret;
		} else {
			path->slots[0]--;
		}
		leaf = path->nodes[0];
		nritems = btrfs_header_nritems(leaf);
		if (nritems == 0)
			return 1;
		if (path->slots[0] == nritems)
			path->slots[0]--;

		btrfs_item_key_to_cpu(leaf, &found_key, path->slots[0]);
		if (found_key.objectid < min_objectid)
			break;
		if (found_key.type == type)
			return 0;
		if (found_key.objectid == min_objectid &&
		    found_key.type < type)
			break;
	}
	return 1;
}
