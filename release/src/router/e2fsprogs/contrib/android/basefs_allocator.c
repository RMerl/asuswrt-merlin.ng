#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "basefs_allocator.h"
#include "block_range.h"
#include "hashmap.h"
#include "base_fs.h"

struct base_fs_allocator {
	struct ext2fs_hashmap *entries;
	struct basefs_entry *cur_entry;
	/* The next expected logical block to allocate for cur_entry. */
	blk64_t next_lblk;
	/* Blocks which are definitely owned by a single inode in BaseFS. */
	ext2fs_block_bitmap exclusive_block_map;
	/* Blocks which are available to the first inode that requests it. */
	ext2fs_block_bitmap dedup_block_map;
};

static errcode_t basefs_block_allocator(ext2_filsys, blk64_t, blk64_t *,
					struct blk_alloc_ctx *ctx);

/*
 * Free any reserved, but unconsumed block ranges in the allocator. This both
 * frees the block_range_list data structure and unreserves exclusive blocks
 * from the block map.
 */
static void fs_free_blocks_range(ext2_filsys fs,
				 struct base_fs_allocator *allocator,
				 struct block_range_list *list)
{
	ext2fs_block_bitmap exclusive_map = allocator->exclusive_block_map;

	blk64_t block;
	while (list->head) {
		block = consume_next_block(list);
		if (ext2fs_test_block_bitmap2(exclusive_map, block)) {
			ext2fs_unmark_block_bitmap2(fs->block_map, block);
			ext2fs_unmark_block_bitmap2(exclusive_map, block);
		}
	}
}

/*
 * Free any blocks in the bitmap that were reserved but never used. This is
 * needed to free dedup_block_map and ensure the free block bitmap is
 * internally consistent.
 */
static void fs_free_blocks_bitmap(ext2_filsys fs, ext2fs_block_bitmap bitmap)
{
	blk64_t block = 0;
	blk64_t start = fs->super->s_first_data_block;
	blk64_t end = ext2fs_blocks_count(fs->super) - 1;
	errcode_t retval;

	for (;;) {
		retval = ext2fs_find_first_set_block_bitmap2(bitmap, start, end,
			&block);
		if (retval)
			break;
		ext2fs_unmark_block_bitmap2(fs->block_map, block);
		start = block + 1;
	}
}

static void basefs_allocator_free(ext2_filsys fs,
				  struct base_fs_allocator *allocator)
{
	struct basefs_entry *e;
	struct ext2fs_hashmap_entry *it = NULL;
	struct ext2fs_hashmap *entries = allocator->entries;

	if (entries) {
		while ((e = ext2fs_hashmap_iter_in_order(entries, &it))) {
			fs_free_blocks_range(fs, allocator, &e->blocks);
			delete_block_ranges(&e->blocks);
		}
		ext2fs_hashmap_free(entries);
	}
	fs_free_blocks_bitmap(fs, allocator->dedup_block_map);
	ext2fs_free_block_bitmap(allocator->exclusive_block_map);
	ext2fs_free_block_bitmap(allocator->dedup_block_map);
	free(allocator);
}

/*
 * Build a bitmap of which blocks are definitely owned by exactly one file in
 * Base FS. Blocks which are not valid or are de-duplicated are skipped. This
 * is called during allocator initialization, to ensure that libext2fs does
 * not allocate which we want to re-use.
 *
 * If a block was allocated in the initial filesystem, it can never be re-used,
 * so it will appear in neither the exclusive or dedup set. If a block is used
 * by multiple files, it will be removed from the owned set and instead added
 * to the dedup set.
 *
 * The dedup set is not removed from fs->block_map. This allows us to re-use
 * dedup blocks separately and not have them be allocated outside of file data.
 */
static void fs_reserve_block(ext2_filsys fs,
			     struct base_fs_allocator *allocator,
			     blk64_t block)
{
	ext2fs_block_bitmap exclusive_map = allocator->exclusive_block_map;
	ext2fs_block_bitmap dedup_map = allocator->dedup_block_map;

	if (block >= ext2fs_blocks_count(fs->super))
		return;

	if (ext2fs_test_block_bitmap2(fs->block_map, block)) {
		if (!ext2fs_test_block_bitmap2(exclusive_map, block))
			return;
		ext2fs_unmark_block_bitmap2(exclusive_map, block);
		ext2fs_mark_block_bitmap2(dedup_map, block);
	} else {
		ext2fs_mark_block_bitmap2(fs->block_map, block);
		ext2fs_mark_block_bitmap2(exclusive_map, block);
	}
}

static void fs_reserve_blocks_range(ext2_filsys fs,
				    struct base_fs_allocator *allocator,
				    struct block_range_list *list)
{
	blk64_t block;
	struct block_range *blocks = list->head;

	while (blocks) {
		for (block = blocks->start; block <= blocks->end; block++)
			fs_reserve_block(fs, allocator, block);
		blocks = blocks->next;
	}
}

/*
 * For each file in the base FS map, ensure that its blocks are reserved in
 * the actual block map. This prevents libext2fs from allocating them for
 * general purpose use, and ensures that if the file needs data blocks, they
 * can be re-acquired exclusively for that file.
 *
 * If a file in the base map is missing, or not a regular file in the new
 * filesystem, then it's skipped to ensure that its blocks are reusable.
 */
static errcode_t fs_reserve_blocks(ext2_filsys fs,
			      struct base_fs_allocator *allocator,
			      const char *src_dir)
{
	int nbytes;
	char full_path[PATH_MAX];
	const char *sep = "/";
	struct stat st;
	struct basefs_entry *e;
	struct ext2fs_hashmap_entry *it = NULL;
	struct ext2fs_hashmap *entries = allocator->entries;

	if (strlen(src_dir) && src_dir[strlen(src_dir) - 1] == '/')
		sep = "";

	while ((e = ext2fs_hashmap_iter_in_order(entries, &it))) {
		nbytes = snprintf(full_path, sizeof(full_path), "%s%s%s",
			src_dir, sep, e->path);
		if (nbytes >= sizeof(full_path))
			return ENAMETOOLONG;
		if (lstat(full_path, &st) || !S_ISREG(st.st_mode))
			continue;
		fs_reserve_blocks_range(fs, allocator, &e->blocks);
	}
	return 0;
}

errcode_t base_fs_alloc_load(ext2_filsys fs, const char *file,
			     const char *mountpoint, const char *src_dir)
{
	errcode_t retval = 0;
	struct base_fs_allocator *allocator;

	allocator = calloc(1, sizeof(*allocator));
	if (!allocator) {
		retval = ENOMEM;
		goto out;
	}

	retval = ext2fs_read_bitmaps(fs);
	if (retval)
		goto err_load;

	allocator->cur_entry = NULL;
	allocator->entries = basefs_parse(file, mountpoint);
	if (!allocator->entries) {
		retval = EIO;
		goto err_load;
	}
	retval = ext2fs_allocate_block_bitmap(fs, "exclusive map",
		&allocator->exclusive_block_map);
	if (retval)
		goto err_load;
	retval = ext2fs_allocate_block_bitmap(fs, "dedup map",
		&allocator->dedup_block_map);
	if (retval)
		goto err_load;

	retval = fs_reserve_blocks(fs, allocator, src_dir);
	if (retval)
		goto err_load;

	/* Override the default allocator */
	fs->get_alloc_block2 = basefs_block_allocator;
	fs->priv_data = allocator;

	goto out;

err_load:
	basefs_allocator_free(fs, allocator);
out:
	return retval;
}

/* Try and acquire the next usable block from the Base FS map. */
static errcode_t get_next_block(ext2_filsys fs, struct base_fs_allocator *allocator,
				struct block_range_list* list, blk64_t *ret)
{
	blk64_t block;
	ext2fs_block_bitmap exclusive_map = allocator->exclusive_block_map;
	ext2fs_block_bitmap dedup_map = allocator->dedup_block_map;

	if (!list->head)
		return EXT2_ET_BLOCK_ALLOC_FAIL;

	block = consume_next_block(list);
	if (block >= ext2fs_blocks_count(fs->super))
		return EXT2_ET_BLOCK_ALLOC_FAIL;
	if (ext2fs_test_block_bitmap2(exclusive_map, block)) {
		ext2fs_unmark_block_bitmap2(exclusive_map, block);
		*ret = block;
		return 0;
	}
	if (ext2fs_test_block_bitmap2(dedup_map, block)) {
		ext2fs_unmark_block_bitmap2(dedup_map, block);
		*ret = block;
		return 0;
	}
	return EXT2_ET_BLOCK_ALLOC_FAIL;
}

/*
 * BaseFS lists blocks in logical block order. However, the allocator hook is
 * only called if a block needs to be allocated. In the case of a deduplicated
 * block, or a hole, the hook is not invoked. This means the next block
 * allocation request will be out of sequence. For example, consider if BaseFS
 * specifies the following (0 being a hole):
 *     1 2 3 0 4 5
 *
 * If the new file has a hole at logical block 0, we could accidentally
 * shift the entire expected block list as follows:
 *     0 1 2 0 3 4
 *
 * To account for this, we track the next expected logical block in the
 * allocator. If the current request is for a later logical block, we skip and
 * free the intermediate physical blocks that would have been allocated. This
 * ensures the original block assignment is respected.
 */
static void skip_blocks(ext2_filsys fs, struct base_fs_allocator *allocator,
			struct blk_alloc_ctx *ctx)
{
	blk64_t block;
	struct block_range_list *list = &allocator->cur_entry->blocks;
	ext2fs_block_bitmap exclusive_map = allocator->exclusive_block_map;

	while (list->head && allocator->next_lblk < ctx->lblk) {
		block = consume_next_block(list);
		if (block >= ext2fs_blocks_count(fs->super))
			continue;
		if (ext2fs_test_block_bitmap2(exclusive_map, block)) {
			ext2fs_unmark_block_bitmap2(exclusive_map, block);
			ext2fs_unmark_block_bitmap2(fs->block_map, block);
		}
		allocator->next_lblk++;
	}
}

static errcode_t basefs_block_allocator(ext2_filsys fs, blk64_t goal,
					blk64_t *ret, struct blk_alloc_ctx *ctx)
{
	errcode_t retval;
	struct base_fs_allocator *allocator = fs->priv_data;
	struct basefs_entry *e = allocator->cur_entry;
	ext2fs_block_bitmap dedup_map = allocator->dedup_block_map;

	if (e && ctx && (ctx->flags & BLOCK_ALLOC_DATA)) {
		if (allocator->next_lblk < ctx->lblk)
			skip_blocks(fs, allocator, ctx);
		allocator->next_lblk = ctx->lblk + 1;

		if (!get_next_block(fs, allocator, &e->blocks, ret))
			return 0;
	}

	retval = ext2fs_new_block2(fs, goal, fs->block_map, ret);
	if (!retval) {
		ext2fs_mark_block_bitmap2(fs->block_map, *ret);
		return 0;
	}
	if (retval != EXT2_ET_BLOCK_ALLOC_FAIL)
		return retval;

	/* Try to steal a block from the dedup pool. */
	retval = ext2fs_find_first_set_block_bitmap2(dedup_map,
		fs->super->s_first_data_block,
		ext2fs_blocks_count(fs->super) - 1, ret);
	if (!retval) {
		ext2fs_unmark_block_bitmap2(dedup_map, *ret);
		return 0;
	}

	/*
	 * As a last resort, take any block from our file's list. This
	 * risks bloating the diff, but means we are more likely to
	 * successfully build an image.
	 */
	while (e->blocks.head) {
		if (!get_next_block(fs, allocator, &e->blocks, ret))
			return 0;
	}
	return EXT2_ET_BLOCK_ALLOC_FAIL;
}

void base_fs_alloc_cleanup(ext2_filsys fs)
{
	basefs_allocator_free(fs, fs->priv_data);
	fs->priv_data = NULL;
	fs->get_alloc_block2 = NULL;
}

errcode_t base_fs_alloc_set_target(ext2_filsys fs, const char *target_path,
	const char *name EXT2FS_ATTR((unused)),
	ext2_ino_t parent_ino EXT2FS_ATTR((unused)),
	ext2_ino_t root EXT2FS_ATTR((unused)), mode_t mode)
{
	struct base_fs_allocator *allocator = fs->priv_data;

	if (mode != S_IFREG)
		return 0;

	if (allocator) {
		allocator->cur_entry = ext2fs_hashmap_lookup(allocator->entries,
						      target_path,
						      strlen(target_path));
		allocator->next_lblk = 0;
	}
	return 0;
}

errcode_t base_fs_alloc_unset_target(ext2_filsys fs,
        const char *target_path EXT2FS_ATTR((unused)),
	const char *name EXT2FS_ATTR((unused)),
	ext2_ino_t parent_ino EXT2FS_ATTR((unused)),
	ext2_ino_t root EXT2FS_ATTR((unused)), mode_t mode)
{
	struct base_fs_allocator *allocator = fs->priv_data;

	if (!allocator || !allocator->cur_entry || mode != S_IFREG)
		return 0;

	fs_free_blocks_range(fs, allocator, &allocator->cur_entry->blocks);
	delete_block_ranges(&allocator->cur_entry->blocks);
	return 0;
}
