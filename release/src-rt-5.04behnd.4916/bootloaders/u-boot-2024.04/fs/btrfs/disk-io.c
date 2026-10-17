// SPDX-License-Identifier: GPL-2.0+
#include <common.h>
#include <fs_internal.h>
#include <log.h>
#include <uuid.h>
#include <memalign.h>
#include "kernel-shared/btrfs_tree.h"
#include "common/rbtree-utils.h"
#include "disk-io.h"
#include "ctree.h"
#include "btrfs.h"
#include "volumes.h"
#include "extent-io.h"
#include "crypto/hash.h"

/* specified errno for check_tree_block */
#define BTRFS_BAD_BYTENR		(-1)
#define BTRFS_BAD_FSID			(-2)
#define BTRFS_BAD_LEVEL			(-3)
#define BTRFS_BAD_NRITEMS		(-4)

/* Calculate max possible nritems for a leaf/node */
static u32 max_nritems(u8 level, u32 nodesize)
{

	if (level == 0)
		return ((nodesize - sizeof(struct btrfs_header)) /
			sizeof(struct btrfs_item));
	return ((nodesize - sizeof(struct btrfs_header)) /
		sizeof(struct btrfs_key_ptr));
}

static int check_tree_block(struct btrfs_fs_info *fs_info,
			    struct extent_buffer *buf)
{

	struct btrfs_fs_devices *fs_devices = fs_info->fs_devices;
	u32 nodesize = fs_info->nodesize;
	bool fsid_match = false;
	int ret = BTRFS_BAD_FSID;

	if (buf->start != btrfs_header_bytenr(buf))
		return BTRFS_BAD_BYTENR;
	if (btrfs_header_level(buf) >= BTRFS_MAX_LEVEL)
		return BTRFS_BAD_LEVEL;
	if (btrfs_header_nritems(buf) > max_nritems(btrfs_header_level(buf),
						    nodesize))
		return BTRFS_BAD_NRITEMS;

	/* Only leaf can be empty */
	if (btrfs_header_nritems(buf) == 0 &&
	    btrfs_header_level(buf) != 0)
		return BTRFS_BAD_NRITEMS;

	while (fs_devices) {
		/*
		 * Checking the incompat flag is only valid for the current
		 * fs. For seed devices it's forbidden to have their uuid
		 * changed so reading ->fsid in this case is fine
		 */
		if (fs_devices == fs_info->fs_devices &&
		    btrfs_fs_incompat(fs_info, METADATA_UUID))
			fsid_match = !memcmp_extent_buffer(buf,
						   fs_devices->metadata_uuid,
						   btrfs_header_fsid(),
						   BTRFS_FSID_SIZE);
		else
			fsid_match = !memcmp_extent_buffer(buf,
						    fs_devices->fsid,
						    btrfs_header_fsid(),
						    BTRFS_FSID_SIZE);


		if (fsid_match) {
			ret = 0;
			break;
		}
		fs_devices = fs_devices->seed;
	}
	return ret;
}

static void print_tree_block_error(struct btrfs_fs_info *fs_info,
				struct extent_buffer *eb,
				int err)
{
	char fs_uuid[BTRFS_UUID_UNPARSED_SIZE] = {'\0'};
	char found_uuid[BTRFS_UUID_UNPARSED_SIZE] = {'\0'};
	u8 buf[BTRFS_UUID_SIZE];

	if (!err)
		return;

	fprintf(stderr, "bad tree block %llu, ", eb->start);
	switch (err) {
	case BTRFS_BAD_FSID:
		read_extent_buffer(eb, buf, btrfs_header_fsid(),
				   BTRFS_UUID_SIZE);
		uuid_unparse(buf, found_uuid);
		uuid_unparse(fs_info->fs_devices->metadata_uuid, fs_uuid);
		fprintf(stderr, "fsid mismatch, want=%s, have=%s\n",
			fs_uuid, found_uuid);
		break;
	case BTRFS_BAD_BYTENR:
		fprintf(stderr, "bytenr mismatch, want=%llu, have=%llu\n",
			eb->start, btrfs_header_bytenr(eb));
		break;
	case BTRFS_BAD_LEVEL:
		fprintf(stderr, "bad level, %u > %d\n",
			btrfs_header_level(eb), BTRFS_MAX_LEVEL);
		break;
	case BTRFS_BAD_NRITEMS:
		fprintf(stderr, "invalid nr_items: %u\n",
			btrfs_header_nritems(eb));
		break;
	}
}

int btrfs_csum_data(u16 csum_type, const u8 *data, u8 *out, size_t len)
{
	memset(out, 0, BTRFS_CSUM_SIZE);

	switch (csum_type) {
	case BTRFS_CSUM_TYPE_CRC32:
		return hash_crc32c(data, len, out);
	case BTRFS_CSUM_TYPE_XXHASH:
		return hash_xxhash(data, len, out);
	case BTRFS_CSUM_TYPE_SHA256:
		return hash_sha256(data, len, out);
	case BTRFS_CSUM_TYPE_BLAKE2:
		return hash_blake2(data, len, out);
	default:
		printf("Unknown csum type %d\n", csum_type);
		return -EINVAL;
	}
}

/*
 * Check if the super is valid:
 * - nodesize/sectorsize - minimum, maximum, alignment
 * - tree block starts   - alignment
 * - number of devices   - something sane
 * - sys array size      - maximum
 */
static int btrfs_check_super(struct btrfs_super_block *sb)
{
	u8 result[BTRFS_CSUM_SIZE];
	u16 csum_type;
	int csum_size;
	u8 *metadata_uuid;

	if (btrfs_super_magic(sb) != BTRFS_MAGIC)
		return -EIO;

	csum_type = btrfs_super_csum_type(sb);
	if (csum_type >= btrfs_super_num_csums()) {
		error("unsupported checksum algorithm %u", csum_type);
		return -EIO;
	}
	csum_size = btrfs_super_csum_size(sb);

	btrfs_csum_data(csum_type, (u8 *)sb + BTRFS_CSUM_SIZE,
			result, BTRFS_SUPER_INFO_SIZE - BTRFS_CSUM_SIZE);

	if (memcmp(result, sb->csum, csum_size)) {
		error("superblock checksum mismatch");
		return -EIO;
	}
	if (btrfs_super_root_level(sb) >= BTRFS_MAX_LEVEL) {
		error("tree_root level too big: %d >= %d",
			btrfs_super_root_level(sb), BTRFS_MAX_LEVEL);
		goto error_out;
	}
	if (btrfs_super_chunk_root_level(sb) >= BTRFS_MAX_LEVEL) {
		error("chunk_root level too big: %d >= %d",
			btrfs_super_chunk_root_level(sb), BTRFS_MAX_LEVEL);
		goto error_out;
	}
	if (btrfs_super_log_root_level(sb) >= BTRFS_MAX_LEVEL) {
		error("log_root level too big: %d >= %d",
			btrfs_super_log_root_level(sb), BTRFS_MAX_LEVEL);
		goto error_out;
	}

	if (!IS_ALIGNED(btrfs_super_root(sb), 4096)) {
		error("tree_root block unaligned: %llu", btrfs_super_root(sb));
		goto error_out;
	}
	if (!IS_ALIGNED(btrfs_super_chunk_root(sb), 4096)) {
		error("chunk_root block unaligned: %llu",
			btrfs_super_chunk_root(sb));
		goto error_out;
	}
	if (!IS_ALIGNED(btrfs_super_log_root(sb), 4096)) {
		error("log_root block unaligned: %llu",
			btrfs_super_log_root(sb));
		goto error_out;
	}
	if (btrfs_super_nodesize(sb) < 4096) {
		error("nodesize too small: %u < 4096",
			btrfs_super_nodesize(sb));
		goto error_out;
	}
	if (!IS_ALIGNED(btrfs_super_nodesize(sb), 4096)) {
		error("nodesize unaligned: %u", btrfs_super_nodesize(sb));
		goto error_out;
	}
	if (btrfs_super_sectorsize(sb) < 4096) {
		error("sectorsize too small: %u < 4096",
			btrfs_super_sectorsize(sb));
		goto error_out;
	}
	if (!IS_ALIGNED(btrfs_super_sectorsize(sb), 4096)) {
		error("sectorsize unaligned: %u", btrfs_super_sectorsize(sb));
		goto error_out;
	}
	if (btrfs_super_total_bytes(sb) == 0) {
		error("invalid total_bytes 0");
		goto error_out;
	}
	if (btrfs_super_bytes_used(sb) < 6 * btrfs_super_nodesize(sb)) {
		error("invalid bytes_used %llu", btrfs_super_bytes_used(sb));
		goto error_out;
	}
	if ((btrfs_super_stripesize(sb) != 4096)
		&& (btrfs_super_stripesize(sb) != btrfs_super_sectorsize(sb))) {
		error("invalid stripesize %u", btrfs_super_stripesize(sb));
		goto error_out;
	}

	if (btrfs_super_incompat_flags(sb) & BTRFS_FEATURE_INCOMPAT_METADATA_UUID)
		metadata_uuid = sb->metadata_uuid;
	else
		metadata_uuid = sb->fsid;

	if (memcmp(metadata_uuid, sb->dev_item.fsid, BTRFS_FSID_SIZE) != 0) {
		char fsid[BTRFS_UUID_UNPARSED_SIZE];
		char dev_fsid[BTRFS_UUID_UNPARSED_SIZE];

		uuid_unparse(sb->metadata_uuid, fsid);
		uuid_unparse(sb->dev_item.fsid, dev_fsid);
		error("dev_item UUID does not match fsid: %s != %s",
			dev_fsid, fsid);
		goto error_out;
	}

	/*
	 * Hint to catch really bogus numbers, bitflips or so
	 */
	if (btrfs_super_num_devices(sb) > (1UL << 31)) {
		error("suspicious number of devices: %llu",
			btrfs_super_num_devices(sb));
	}

	if (btrfs_super_num_devices(sb) == 0) {
		error("number of devices is 0");
		goto error_out;
	}

	/*
	 * Obvious sys_chunk_array corruptions, it must hold at least one key
	 * and one chunk
	 */
	if (btrfs_super_sys_array_size(sb) > BTRFS_SYSTEM_CHUNK_ARRAY_SIZE) {
		error("system chunk array too big %u > %u",
		      btrfs_super_sys_array_size(sb),
		      BTRFS_SYSTEM_CHUNK_ARRAY_SIZE);
		goto error_out;
	}
	if (btrfs_super_sys_array_size(sb) < sizeof(struct btrfs_disk_key)
			+ sizeof(struct btrfs_chunk)) {
		error("system chunk array too small %u < %zu",
		      btrfs_super_sys_array_size(sb),
		      sizeof(struct btrfs_disk_key) +
		      sizeof(struct btrfs_chunk));
		goto error_out;
	}

	return 0;

error_out:
	error("superblock checksum matches but it has invalid members");
	return -EIO;
}

/*
 * btrfs_read_dev_super - read a valid primary superblock from a block device
 * @desc,@part:	file descriptor of the device
 * @sb:		buffer where the superblock is going to be read in
 *
 * Unlike the btrfs-progs/kernel version, here we ony care about the first
 * super block, thus it's much simpler.
 */
int btrfs_read_dev_super(struct blk_desc *desc, struct disk_partition *part,
			 struct btrfs_super_block *sb)
{
	ALLOC_CACHE_ALIGN_BUFFER(char, tmp, BTRFS_SUPER_INFO_SIZE);
	struct btrfs_super_block *buf = (struct btrfs_super_block *)tmp;
	int ret;

	ret = __btrfs_devread(desc, part, tmp, BTRFS_SUPER_INFO_SIZE,
			      BTRFS_SUPER_INFO_OFFSET);
	if (ret < BTRFS_SUPER_INFO_SIZE)
		return -EIO;

	if (btrfs_super_bytenr(buf) != BTRFS_SUPER_INFO_OFFSET)
		return -EIO;

	if (btrfs_check_super(buf))
		return -EIO;

	memcpy(sb, buf, BTRFS_SUPER_INFO_SIZE);
	return 0;
}

static int __csum_tree_block_size(struct extent_buffer *buf, u16 csum_size,
				  int verify, int silent, u16 csum_type)
{
	u8 result[BTRFS_CSUM_SIZE];
	u32 len;

	len = buf->len - BTRFS_CSUM_SIZE;
	btrfs_csum_data(csum_type, (u8 *)buf->data + BTRFS_CSUM_SIZE,
			result, len);

	if (verify) {
		if (memcmp_extent_buffer(buf, result, 0, csum_size)) {
			/* FIXME: format */
			if (!silent)
				printk("checksum verify failed on %llu found %08X wanted %08X\n",
				       (unsigned long long)buf->start,
				       result[0],
				       buf->data[0]);
			return 1;
		}
	} else {
		write_extent_buffer(buf, result, 0, csum_size);
	}
	return 0;
}

int csum_tree_block_size(struct extent_buffer *buf, u16 csum_size, int verify,
			 u16 csum_type)
{
	return __csum_tree_block_size(buf, csum_size, verify, 0, csum_type);
}

static int csum_tree_block(struct btrfs_fs_info *fs_info,
			   struct extent_buffer *buf, int verify)
{
	u16 csum_size = btrfs_super_csum_size(fs_info->super_copy);
	u16 csum_type = btrfs_super_csum_type(fs_info->super_copy);

	return csum_tree_block_size(buf, csum_size, verify, csum_type);
}

struct extent_buffer *btrfs_find_tree_block(struct btrfs_fs_info *fs_info,
					    u64 bytenr, u32 blocksize)
{
	return find_extent_buffer(&fs_info->extent_cache,
				  bytenr, blocksize);
}

struct extent_buffer* btrfs_find_create_tree_block(
		struct btrfs_fs_info *fs_info, u64 bytenr)
{
	return alloc_extent_buffer(fs_info, bytenr, fs_info->nodesize);
}

static int verify_parent_transid(struct extent_io_tree *io_tree,
				 struct extent_buffer *eb, u64 parent_transid,
				 int ignore)
{
	int ret;

	if (!parent_transid || btrfs_header_generation(eb) == parent_transid)
		return 0;

	if (extent_buffer_uptodate(eb) &&
	    btrfs_header_generation(eb) == parent_transid) {
		ret = 0;
		goto out;
	}
	printk("parent transid verify failed on %llu wanted %llu found %llu\n",
	       (unsigned long long)eb->start,
	       (unsigned long long)parent_transid,
	       (unsigned long long)btrfs_header_generation(eb));
	if (ignore) {
		eb->flags |= EXTENT_BAD_TRANSID;
		printk("Ignoring transid failure\n");
		return 0;
	}

	ret = 1;
out:
	clear_extent_buffer_uptodate(eb);
	return ret;

}

int read_whole_eb(struct btrfs_fs_info *info, struct extent_buffer *eb, int mirror)
{
	unsigned long offset = 0;
	struct btrfs_multi_bio *multi = NULL;
	struct btrfs_device *device;
	int ret = 0;
	u64 read_len;
	unsigned long bytes_left = eb->len;

	while (bytes_left) {
		read_len = bytes_left;
		device = NULL;

		ret = btrfs_map_block(info, READ, eb->start + offset,
				      &read_len, &multi, mirror, NULL);
		if (ret) {
			printk("Couldn't map the block %Lu\n", eb->start + offset);
			kfree(multi);
			return -EIO;
		}
		device = multi->stripes[0].dev;

		if (!device->desc || !device->part) {
			kfree(multi);
			return -EIO;
		}

		if (read_len > bytes_left)
			read_len = bytes_left;

		ret = read_extent_from_disk(device->desc, device->part,
					    multi->stripes[0].physical, eb,
					    offset, read_len);
		kfree(multi);
		multi = NULL;

		if (ret)
			return -EIO;
		offset += read_len;
		bytes_left -= read_len;
	}
	return 0;
}

struct extent_buffer* read_tree_block(struct btrfs_fs_info *fs_info, u64 bytenr,
		u64 parent_transid)
{
	int ret;
	struct extent_buffer *eb;
	u64 best_transid = 0;
	u32 sectorsize = fs_info->sectorsize;
	int mirror_num = 1;
	int good_mirror = 0;
	int candidate_mirror = 0;
	int num_copies;
	int ignore = 0;

	/*
	 * Don't even try to create tree block for unaligned tree block
	 * bytenr.
	 * Such unaligned tree block will free overlapping extent buffer,
	 * causing use-after-free bugs for fuzzed images.
	 */
	if (bytenr < sectorsize || !IS_ALIGNED(bytenr, sectorsize)) {
		error("tree block bytenr %llu is not aligned to sectorsize %u",
		      bytenr, sectorsize);
		return ERR_PTR(-EIO);
	}

	eb = btrfs_find_create_tree_block(fs_info, bytenr);
	if (!eb)
		return ERR_PTR(-ENOMEM);

	if (btrfs_buffer_uptodate(eb, parent_transid))
		return eb;

	num_copies = btrfs_num_copies(fs_info, eb->start, eb->len);
	while (1) {
		ret = read_whole_eb(fs_info, eb, mirror_num);
		if (ret == 0 && csum_tree_block(fs_info, eb, 1) == 0 &&
		    check_tree_block(fs_info, eb) == 0 &&
		    verify_parent_transid(&fs_info->extent_cache, eb,
					  parent_transid, ignore) == 0) {
			/*
			 * check_tree_block() is less strict to allow btrfs
			 * check to get raw eb with bad key order and fix it.
			 * But we still need to try to get a good copy if
			 * possible, or bad key order can go into tools like
			 * btrfs ins dump-tree.
			 */
			if (btrfs_header_level(eb))
				ret = btrfs_check_node(fs_info, NULL, eb);
			else
				ret = btrfs_check_leaf(fs_info, NULL, eb);
			if (!ret || candidate_mirror == mirror_num) {
				btrfs_set_buffer_uptodate(eb);
				return eb;
			}
			if (candidate_mirror <= 0)
				candidate_mirror = mirror_num;
		}
		if (ignore) {
			if (candidate_mirror > 0) {
				mirror_num = candidate_mirror;
				continue;
			}
			if (check_tree_block(fs_info, eb))
				print_tree_block_error(fs_info, eb,
						check_tree_block(fs_info, eb));
			else
				fprintf(stderr, "Csum didn't match\n");
			ret = -EIO;
			break;
		}
		if (num_copies == 1) {
			ignore = 1;
			continue;
		}
		if (btrfs_header_generation(eb) > best_transid) {
			best_transid = btrfs_header_generation(eb);
			good_mirror = mirror_num;
		}
		mirror_num++;
		if (mirror_num > num_copies) {
			if (candidate_mirror > 0)
				mirror_num = candidate_mirror;
			else
				mirror_num = good_mirror;
			ignore = 1;
			continue;
		}
	}
	/*
	 * We failed to read this tree block, it be should deleted right now
	 * to avoid stale cache populate the cache.
	 */
	free_extent_buffer(eb);
	return ERR_PTR(ret);
}

int read_extent_data(struct btrfs_fs_info *fs_info, char *data, u64 logical,
		     u64 *len, int mirror)
{
	u64 orig_len = *len;
	u64 cur = logical;
	struct btrfs_multi_bio *multi = NULL;
	struct btrfs_device *device;
	int ret = 0;

	while (cur < logical + orig_len) {
		u64 cur_len = logical + orig_len - cur;

		ret = btrfs_map_block(fs_info, READ, cur, &cur_len, &multi,
				      mirror, NULL);
		if (ret) {
			error("Couldn't map the block %llu", cur);
			goto err;
		}
		device = multi->stripes[0].dev;
		if (!device->desc || !device->part) {
			error("devid %llu is missing", device->devid);
			ret = -EIO;
			goto err;
		}
		ret = __btrfs_devread(device->desc, device->part,
				data + (cur - logical), cur_len,
				multi->stripes[0].physical);
		if (ret != cur_len) {
			error("read failed on devid %llu physical %llu",
			      device->devid, multi->stripes[0].physical);
			ret = -EIO;
			goto err;
		}
		cur += cur_len;
		ret = 0;
	}
err:
	kfree(multi);
	return ret;
}

void btrfs_setup_root(struct btrfs_root *root, struct btrfs_fs_info *fs_info,
		      u64 objectid)
{
	root->node = NULL;
	root->track_dirty = 0;

	root->fs_info = fs_info;
	root->objectid = objectid;
	root->last_trans = 0;
	root->last_inode_alloc = 0;

	memset(&root->root_key, 0, sizeof(root->root_key));
	memset(&root->root_item, 0, sizeof(root->root_item));
	root->root_key.objectid = objectid;
}

static int find_and_setup_root(struct btrfs_root *tree_root,
			       struct btrfs_fs_info *fs_info,
			       u64 objectid, struct btrfs_root *root)
{
	int ret;
	u64 generation;

	btrfs_setup_root(root, fs_info, objectid);
	ret = btrfs_find_last_root(tree_root, objectid,
				   &root->root_item, &root->root_key);
	if (ret)
		return ret;

	generation = btrfs_root_generation(&root->root_item);
	root->node = read_tree_block(fs_info,
			btrfs_root_bytenr(&root->root_item), generation);
	if (!extent_buffer_uptodate(root->node))
		return -EIO;

	return 0;
}

int btrfs_free_fs_root(struct btrfs_root *root)
{
	if (root->node)
		free_extent_buffer(root->node);
	kfree(root);
	return 0;
}

static void __free_fs_root(struct rb_node *node)
{
	struct btrfs_root *root;

	root = container_of(node, struct btrfs_root, rb_node);
	btrfs_free_fs_root(root);
}

FREE_RB_BASED_TREE(fs_roots, __free_fs_root);

struct btrfs_root *btrfs_read_fs_root_no_cache(struct btrfs_fs_info *fs_info,
					       struct btrfs_key *location)
{
	struct btrfs_root *root;
	struct btrfs_root *tree_root = fs_info->tree_root;
	struct btrfs_path *path;
	struct extent_buffer *l;
	u64 generation;
	int ret = 0;

	root = calloc(1, sizeof(*root));
	if (!root)
		return ERR_PTR(-ENOMEM);
	if (location->offset == (u64)-1) {
		ret = find_and_setup_root(tree_root, fs_info,
					  location->objectid, root);
		if (ret) {
			free(root);
			return ERR_PTR(ret);
		}
		goto insert;
	}

	btrfs_setup_root(root, fs_info,
			 location->objectid);

	path = btrfs_alloc_path();
	if (!path) {
		free(root);
		return ERR_PTR(-ENOMEM);
	}

	ret = btrfs_search_slot(NULL, tree_root, location, path, 0, 0);
	if (ret != 0) {
		if (ret > 0)
			ret = -ENOENT;
		goto out;
	}
	l = path->nodes[0];
	read_extent_buffer(l, &root->root_item,
	       btrfs_item_ptr_offset(l, path->slots[0]),
	       sizeof(root->root_item));
	memcpy(&root->root_key, location, sizeof(*location));

	/* If this root is already an orphan, no need to read */
	if (btrfs_root_refs(&root->root_item) == 0) {
		ret = -ENOENT;
		goto out;
	}
	ret = 0;
out:
	btrfs_free_path(path);
	if (ret) {
		free(root);
		return ERR_PTR(ret);
	}
	generation = btrfs_root_generation(&root->root_item);
	root->node = read_tree_block(fs_info,
			btrfs_root_bytenr(&root->root_item), generation);
	if (!extent_buffer_uptodate(root->node)) {
		free(root);
		return ERR_PTR(-EIO);
	}
insert:
	root->ref_cows = 1;
	return root;
}

static int btrfs_fs_roots_compare_objectids(struct rb_node *node,
					    void *data)
{
	u64 objectid = *((u64 *)data);
	struct btrfs_root *root;

	root = rb_entry(node, struct btrfs_root, rb_node);
	if (objectid > root->objectid)
		return 1;
	else if (objectid < root->objectid)
		return -1;
	else
		return 0;
}

int btrfs_fs_roots_compare_roots(struct rb_node *node1, struct rb_node *node2)
{
	struct btrfs_root *root;

	root = rb_entry(node2, struct btrfs_root, rb_node);
	return btrfs_fs_roots_compare_objectids(node1, (void *)&root->objectid);
}

struct btrfs_root *btrfs_read_fs_root(struct btrfs_fs_info *fs_info,
				      struct btrfs_key *location)
{
	struct btrfs_root *root;
	struct rb_node *node;
	int ret;
	u64 objectid = location->objectid;

	if (location->objectid == BTRFS_ROOT_TREE_OBJECTID)
		return fs_info->tree_root;
	if (location->objectid == BTRFS_CHUNK_TREE_OBJECTID)
		return fs_info->chunk_root;
	if (location->objectid == BTRFS_CSUM_TREE_OBJECTID)
		return fs_info->csum_root;
	BUG_ON(location->objectid == BTRFS_TREE_RELOC_OBJECTID);

	node = rb_search(&fs_info->fs_root_tree, (void *)&objectid,
			 btrfs_fs_roots_compare_objectids, NULL);
	if (node)
		return container_of(node, struct btrfs_root, rb_node);

	root = btrfs_read_fs_root_no_cache(fs_info, location);
	if (IS_ERR(root))
		return root;

	ret = rb_insert(&fs_info->fs_root_tree, &root->rb_node,
			btrfs_fs_roots_compare_roots);
	BUG_ON(ret);
	return root;
}

void btrfs_free_fs_info(struct btrfs_fs_info *fs_info)
{
	free(fs_info->tree_root);
	free(fs_info->chunk_root);
	free(fs_info->csum_root);
	free(fs_info->super_copy);
	free(fs_info);
}

struct btrfs_fs_info *btrfs_new_fs_info(void)
{
	struct btrfs_fs_info *fs_info;

	fs_info = calloc(1, sizeof(struct btrfs_fs_info));
	if (!fs_info)
		return NULL;

	fs_info->tree_root = calloc(1, sizeof(struct btrfs_root));
	fs_info->chunk_root = calloc(1, sizeof(struct btrfs_root));
	fs_info->csum_root = calloc(1, sizeof(struct btrfs_root));
	fs_info->super_copy = calloc(1, BTRFS_SUPER_INFO_SIZE);

	if (!fs_info->tree_root || !fs_info->chunk_root ||
	    !fs_info->csum_root || !fs_info->super_copy)
		goto free_all;

	extent_io_tree_init(&fs_info->extent_cache);

	fs_info->fs_root_tree = RB_ROOT;
	cache_tree_init(&fs_info->mapping_tree.cache_tree);

	return fs_info;
free_all:
	btrfs_free_fs_info(fs_info);
	return NULL;
}

static int setup_root_or_create_block(struct btrfs_fs_info *fs_info,
				      struct btrfs_root *info_root,
				      u64 objectid, char *str)
{
	struct btrfs_root *root = fs_info->tree_root;
	int ret;

	ret = find_and_setup_root(root, fs_info, objectid, info_root);
	if (ret) {
		error("could not setup %s tree", str);
		return -EIO;
	}

	return 0;
}

static int get_default_subvolume(struct btrfs_fs_info *fs_info,
				 struct btrfs_key *key_ret)
{
	struct btrfs_root *root = fs_info->tree_root;
	struct btrfs_dir_item *dir_item;
	struct btrfs_path path;
	int ret = 0;

	btrfs_init_path(&path);

	dir_item = btrfs_lookup_dir_item(NULL, root, &path,
					 BTRFS_ROOT_TREE_DIR_OBJECTID,
					 "default", 7, 0);
	if (IS_ERR(dir_item)) {
		ret = PTR_ERR(dir_item);
		goto out;
	}

	btrfs_dir_item_key_to_cpu(path.nodes[0], dir_item, key_ret);
out:
	btrfs_release_path(&path);
	return ret;
}

int btrfs_setup_all_roots(struct btrfs_fs_info *fs_info)
{
	struct btrfs_super_block *sb = fs_info->super_copy;
	struct btrfs_root *root;
	struct btrfs_key key;
	u64 root_tree_bytenr;
	u64 generation;
	int ret;

	root = fs_info->tree_root;
	btrfs_setup_root(root, fs_info, BTRFS_ROOT_TREE_OBJECTID);
	generation = btrfs_super_generation(sb);

	root_tree_bytenr = btrfs_super_root(sb);

	root->node = read_tree_block(fs_info, root_tree_bytenr, generation);
	if (!extent_buffer_uptodate(root->node)) {
		fprintf(stderr, "Couldn't read tree root\n");
		return -EIO;
	}

	ret = setup_root_or_create_block(fs_info, fs_info->csum_root,
					 BTRFS_CSUM_TREE_OBJECTID, "csum");
	if (ret)
		return ret;
	fs_info->csum_root->track_dirty = 1;

	fs_info->last_trans_committed = generation;

	ret = get_default_subvolume(fs_info, &key);
	if (ret) {
		/*
		 * The default dir item isn't there. Linux kernel behaviour is
		 * to silently use the top-level subvolume in this case.
		 */
		key.objectid = BTRFS_FS_TREE_OBJECTID;
		key.type = BTRFS_ROOT_ITEM_KEY;
		key.offset = (u64)-1;
	}

	fs_info->fs_root = btrfs_read_fs_root(fs_info, &key);

	if (IS_ERR(fs_info->fs_root))
		return -EIO;
	return 0;
}

void btrfs_release_all_roots(struct btrfs_fs_info *fs_info)
{
	if (fs_info->csum_root)
		free_extent_buffer(fs_info->csum_root->node);
	if (fs_info->tree_root)
		free_extent_buffer(fs_info->tree_root->node);
	if (fs_info->chunk_root)
		free_extent_buffer(fs_info->chunk_root->node);
}

static void free_map_lookup(struct cache_extent *ce)
{
	struct map_lookup *map;

	map = container_of(ce, struct map_lookup, ce);
	kfree(map);
}

FREE_EXTENT_CACHE_BASED_TREE(mapping_cache, free_map_lookup);

void btrfs_cleanup_all_caches(struct btrfs_fs_info *fs_info)
{
	free_mapping_cache_tree(&fs_info->mapping_tree.cache_tree);
	extent_io_tree_cleanup(&fs_info->extent_cache);
}

static int btrfs_scan_fs_devices(struct blk_desc *desc,
				 struct disk_partition *part,
				 struct btrfs_fs_devices **fs_devices)
{
	u64 total_devs;
	int ret;

	if (round_up(BTRFS_SUPER_INFO_SIZE + BTRFS_SUPER_INFO_OFFSET,
		     desc->blksz) > (part->size << desc->log2blksz)) {
		log_debug("superblock end %u is larger than device size " LBAFU,
			  BTRFS_SUPER_INFO_SIZE + BTRFS_SUPER_INFO_OFFSET,
			  part->size << desc->log2blksz);
		return -EINVAL;
	}

	ret = btrfs_scan_one_device(desc, part, fs_devices, &total_devs);
	if (ret) {
		/*
		 * Avoid showing this when probing for a possible Btrfs
		 *
		 * fprintf(stderr, "No valid Btrfs found\n");
		 */
		return ret;
	}
	return 0;
}

int btrfs_check_fs_compatibility(struct btrfs_super_block *sb)
{
	u64 features;

	features = btrfs_super_incompat_flags(sb) &
		   ~BTRFS_FEATURE_INCOMPAT_SUPP;
	if (features) {
		printk("couldn't open because of unsupported "
		       "option features (%llx).\n",
		       (unsigned long long)features);
		return -ENOTSUPP;
	}

	features = btrfs_super_incompat_flags(sb);
	if (!(features & BTRFS_FEATURE_INCOMPAT_MIXED_BACKREF)) {
		features |= BTRFS_FEATURE_INCOMPAT_MIXED_BACKREF;
		btrfs_set_super_incompat_flags(sb, features);
	}

	return 0;
}

static int btrfs_setup_chunk_tree_and_device_map(struct btrfs_fs_info *fs_info)
{
	struct btrfs_super_block *sb = fs_info->super_copy;
	u64 chunk_root_bytenr;
	u64 generation;
	int ret;

	btrfs_setup_root(fs_info->chunk_root, fs_info,
			BTRFS_CHUNK_TREE_OBJECTID);

	ret = btrfs_read_sys_array(fs_info);
	if (ret)
		return ret;

	generation = btrfs_super_chunk_root_generation(sb);
	chunk_root_bytenr = btrfs_super_chunk_root(sb);

	fs_info->chunk_root->node = read_tree_block(fs_info,
						    chunk_root_bytenr,
						    generation);
	if (!extent_buffer_uptodate(fs_info->chunk_root->node)) {
		error("cannot read chunk root");
		return -EIO;
	}

	ret = btrfs_read_chunk_tree(fs_info);
	if (ret) {
		fprintf(stderr, "Couldn't read chunk tree\n");
		return ret;
	}
	return 0;
}

struct btrfs_fs_info *open_ctree_fs_info(struct blk_desc *desc,
					 struct disk_partition *part)
{
	struct btrfs_fs_info *fs_info;
	struct btrfs_super_block *disk_super;
	struct btrfs_fs_devices *fs_devices = NULL;
	struct extent_buffer *eb;
	int ret;

	fs_info = btrfs_new_fs_info();
	if (!fs_info) {
		fprintf(stderr, "Failed to allocate memory for fs_info\n");
		return NULL;
	}

	ret = btrfs_scan_fs_devices(desc, part, &fs_devices);
	if (ret)
		goto out;

	fs_info->fs_devices = fs_devices;

	ret = btrfs_open_devices(fs_devices);
	if (ret)
		goto out;

	disk_super = fs_info->super_copy;
	ret = btrfs_read_dev_super(desc, part, disk_super);
	if (ret) {
		debug("No valid btrfs found\n");
		goto out_devices;
	}

	if (btrfs_super_flags(disk_super) & BTRFS_SUPER_FLAG_CHANGING_FSID) {
		fprintf(stderr, "ERROR: Filesystem UUID change in progress\n");
		goto out_devices;
	}

	ASSERT(!memcmp(disk_super->fsid, fs_devices->fsid, BTRFS_FSID_SIZE));
	if (btrfs_fs_incompat(fs_info, METADATA_UUID))
		ASSERT(!memcmp(disk_super->metadata_uuid,
			       fs_devices->metadata_uuid, BTRFS_FSID_SIZE));

	fs_info->sectorsize = btrfs_super_sectorsize(disk_super);
	fs_info->nodesize = btrfs_super_nodesize(disk_super);
	fs_info->stripesize = btrfs_super_stripesize(disk_super);

	ret = btrfs_check_fs_compatibility(fs_info->super_copy);
	if (ret)
		goto out_devices;

	ret = btrfs_setup_chunk_tree_and_device_map(fs_info);
	if (ret)
		goto out_chunk;

	/* Chunk tree root is unable to read, return directly */
	if (!fs_info->chunk_root)
		return fs_info;

	eb = fs_info->chunk_root->node;
	read_extent_buffer(eb, fs_info->chunk_tree_uuid,
			   btrfs_header_chunk_tree_uuid(eb),
			   BTRFS_UUID_SIZE);

	ret = btrfs_setup_all_roots(fs_info);
	if (ret)
		goto out_chunk;

	return fs_info;

out_chunk:
	btrfs_release_all_roots(fs_info);
	btrfs_cleanup_all_caches(fs_info);
out_devices:
	btrfs_close_devices(fs_devices);
out:
	btrfs_free_fs_info(fs_info);
	return NULL;
}

int close_ctree_fs_info(struct btrfs_fs_info *fs_info)
{
	int ret;

	free_fs_roots_tree(&fs_info->fs_root_tree);

	btrfs_release_all_roots(fs_info);
	ret = btrfs_close_devices(fs_info->fs_devices);
	btrfs_cleanup_all_caches(fs_info);
	btrfs_free_fs_info(fs_info);
	return ret;
}

int btrfs_buffer_uptodate(struct extent_buffer *buf, u64 parent_transid)
{
	int ret;

	ret = extent_buffer_uptodate(buf);
	if (!ret)
		return ret;

	ret = verify_parent_transid(&buf->fs_info->extent_cache, buf,
				    parent_transid, 1);
	return !ret;
}

int btrfs_set_buffer_uptodate(struct extent_buffer *eb)
{
	return set_extent_buffer_uptodate(eb);
}
