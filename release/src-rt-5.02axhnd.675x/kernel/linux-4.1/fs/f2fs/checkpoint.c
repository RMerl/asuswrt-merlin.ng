/*
 * fs/f2fs/checkpoint.c
 *
 * Copyright (c) 2012 Samsung Electronics Co., Ltd.
 *             http://www.samsung.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/fs.h>
#include <linux/bio.h>
#include <linux/mpage.h>
#include <linux/writeback.h>
#include <linux/blkdev.h>
#include <linux/f2fs_fs.h>
#include <linux/pagevec.h>
#include <linux/swap.h>

#include "f2fs.h"
#include "node.h"
#include "segment.h"
#include "trace.h"
#include <trace/events/f2fs.h>

static struct kmem_cache *ino_entry_slab;
struct kmem_cache *inode_entry_slab;

/*
 * We guarantee no failure on the returned page.
 */
struct page *grab_meta_page(struct f2fs_sb_info *sbi, pgoff_t index)
{
	struct address_space *mapping = META_MAPPING(sbi);
	struct page *page = NULL;
repeat:
	page = grab_cache_page(mapping, index);
	if (!page) {
		cond_resched();
		goto repeat;
	}
	f2fs_wait_on_page_writeback(page, META);
	SetPageUptodate(page);
	return page;
}

/*
 * We guarantee no failure on the returned page.
 */
struct page *get_meta_page(struct f2fs_sb_info *sbi, pgoff_t index)
{
	struct address_space *mapping = META_MAPPING(sbi);
	struct page *page;
	struct f2fs_io_info fio = {
		.type = META,
		.rw = READ_SYNC | REQ_META | REQ_PRIO,
		.blk_addr = index,
	};
repeat:
	page = grab_cache_page(mapping, index);
	if (!page) {
		cond_resched();
		goto repeat;
	}
	if (PageUptodate(page))
		goto out;

	if (f2fs_submit_page_bio(sbi, page, &fio))
		goto repeat;

	lock_page(page);
	if (unlikely(page->mapping != mapping)) {
		f2fs_put_page(page, 1);
		goto repeat;
	}
out:
	return page;
}

static inline bool is_valid_blkaddr(struct f2fs_sb_info *sbi,
						block_t blkaddr, int type)
{
	switch (type) {
	case META_NAT:
		break;
	case META_SIT:
		if (unlikely(blkaddr >= SIT_BLK_CNT(sbi)))
			return false;
		break;
	case META_SSA:
		if (unlikely(blkaddr >= MAIN_BLKADDR(sbi) ||
			blkaddr < SM_I(sbi)->ssa_blkaddr))
			return false;
		break;
	case META_CP:
		if (unlikely(blkaddr >= SIT_I(sbi)->sit_base_addr ||
			blkaddr < __start_cp_addr(sbi)))
			return false;
		break;
	case META_POR:
		if (unlikely(blkaddr >= MAX_BLKADDR(sbi) ||
			blkaddr < MAIN_BLKADDR(sbi)))
			return false;
		break;
	default:
		BUG();
	}

	return true;
}

/*
 * Readahead CP/NAT/SIT/SSA pages
 */
int ra_meta_pages(struct f2fs_sb_info *sbi, block_t start, int nrpages, int type)
{
	block_t prev_blk_addr = 0;
	struct page *page;
	block_t blkno = start;
	struct f2fs_io_info fio = {
		.type = META,
		.rw = READ_SYNC | REQ_META | REQ_PRIO
	};

	for (; nrpages-- > 0; blkno++) {

		if (!is_valid_blkaddr(sbi, blkno, type))
			goto out;

		switch (type) {
		case META_NAT:
			if (unlikely(blkno >=
					NAT_BLOCK_OFFSET(NM_I(sbi)->max_nid)))
				blkno = 0;
			/* get nat block addr */
			fio.blk_addr = current_nat_addr(sbi,
					blkno * NAT_ENTRY_PER_BLOCK);
			break;
		case META_SIT:
			/* get sit block addr */
			fio.blk_addr = current_sit_addr(sbi,
					blkno * SIT_ENTRY_PER_BLOCK);
			if (blkno != start && prev_blk_addr + 1 != fio.blk_addr)
				goto out;
			prev_blk_addr = fio.blk_addr;
			break;
		case META_SSA:
		case META_CP:
		case META_POR:
			fio.blk_addr = blkno;
			break;
		default:
			BUG();
		}

		page = grab_cache_page(META_MAPPING(sbi), fio.blk_addr);
		if (!page)
			continue;
		if (PageUptodate(page)) {
			f2fs_put_page(page, 1);
			continue;
		}

		f2fs_submit_page_mbio(sbi, page, &fio);
		f2fs_put_page(page, 0);
	}
out:
	f2fs_submit_merged_bio(sbi, META, READ);
	return blkno - start;
}

void ra_meta_pages_cond(struct f2fs_sb_info *sbi, pgoff_t index)
{
	struct page *page;
	bool readahead = false;

	page = find_get_page(META_MAPPING(sbi), index);
	if (!page || (page && !PageUptodate(page)))
		readahead = true;
	f2fs_put_page(page, 0);

	if (readahead)
		ra_meta_pages(sbi, index, MAX_BIO_BLOCKS(sbi), META_POR);
}

static int f2fs_write_meta_page(struct page *page,
				struct writeback_control *wbc)
{
	struct f2fs_sb_info *sbi = F2FS_P_SB(page);

	trace_f2fs_writepage(page, META);

	if (unlikely(is_sbi_flag_set(sbi, SBI_POR_DOING)))
		goto redirty_out;
	if (wbc->for_reclaim && page->index < GET_SUM_BLOCK(sbi, 0))
		goto redirty_out;
	if (unlikely(f2fs_cp_error(sbi)))
		goto redirty_out;

	f2fs_wait_on_page_writeback(page, META);
	write_meta_page(sbi, page);
	dec_page_count(sbi, F2FS_DIRTY_META);
	unlock_page(page);

	if (wbc->for_reclaim)
		f2fs_submit_merged_bio(sbi, META, WRITE);
	return 0;

redirty_out:
	redirty_page_for_writepage(wbc, page);
	return AOP_WRITEPAGE_ACTIVATE;
}

static int f2fs_write_meta_pages(struct address_space *mapping,
				struct writeback_control *wbc)
{
	struct f2fs_sb_info *sbi = F2FS_M_SB(mapping);
	long diff, written;

	trace_f2fs_writepages(mapping->host, wbc, META);

	/* collect a number of dirty meta pages and write together */
	if (wbc->for_kupdate ||
		get_pages(sbi, F2FS_DIRTY_META) < nr_pages_to_skip(sbi, META))
		goto skip_write;

	/* if mounting is failed, skip writing node pages */
	mutex_lock(&sbi->cp_mutex);
	diff = nr_pages_to_write(sbi, META, wbc);
	written = sync_meta_pages(sbi, META, wbc->nr_to_write);
	mutex_unlock(&sbi->cp_mutex);
	wbc->nr_to_write = max((long)0, wbc->nr_to_write - written - diff);
	return 0;

skip_write:
	wbc->pages_skipped += get_pages(sbi, F2FS_DIRTY_META);
	return 0;
}

long sync_meta_pages(struct f2fs_sb_info *sbi, enum page_type type,
						long nr_to_write)
{
	struct address_space *mapping = META_MAPPING(sbi);
	pgoff_t index = 0, end = LONG_MAX;
	struct pagevec pvec;
	long nwritten = 0;
	struct writeback_control wbc = {
		.for_reclaim = 0,
	};

	pagevec_init(&pvec, 0);

	while (index <= end) {
		int i, nr_pages;
		nr_pages = pagevec_lookup_tag(&pvec, mapping, &index,
				PAGECACHE_TAG_DIRTY,
				min(end - index, (pgoff_t)PAGEVEC_SIZE-1) + 1);
		if (unlikely(nr_pages == 0))
			break;

		for (i = 0; i < nr_pages; i++) {
			struct page *page = pvec.pages[i];

			lock_page(page);

			if (unlikely(page->mapping != mapping)) {
continue_unlock:
				unlock_page(page);
				continue;
			}
			if (!PageDirty(page)) {
				/* someone wrote it for us */
				goto continue_unlock;
			}

			if (!clear_page_dirty_for_io(page))
				goto continue_unlock;

			if (mapping->a_ops->writepage(page, &wbc)) {
				unlock_page(page);
				break;
			}
			nwritten++;
			if (unlikely(nwritten >= nr_to_write))
				break;
		}
		pagevec_release(&pvec);
		cond_resched();
	}

	if (nwritten)
		f2fs_submit_merged_bio(sbi, type, WRITE);

	return nwritten;
}

static int f2fs_set_meta_page_dirty(struct page *page)
{
	trace_f2fs_set_page_dirty(page, META);

	SetPageUptodate(page);
	if (!PageDirty(page)) {
		__set_page_dirty_nobuffers(page);
		inc_page_count(F2FS_P_SB(page), F2FS_DIRTY_META);
		SetPagePrivate(page);
		f2fs_trace_pid(page);
		return 1;
	}
	return 0;
}

const struct address_space_operations f2fs_meta_aops = {
	.writepage	= f2fs_write_meta_page,
	.writepages	= f2fs_write_meta_pages,
	.set_page_dirty	= f2fs_set_meta_page_dirty,
	.invalidatepage = f2fs_invalidate_page,
	.releasepage	= f2fs_release_page,
};

static void __add_ino_entry(struct f2fs_sb_info *sbi, nid_t ino, int type)
{
	struct inode_management *im = &sbi->im[type];
	struct ino_entry *e;
retry:
	if (radix_tree_preload(GFP_NOFS)) {
		cond_resched();
		goto retry;
	}

	spin_lock(&im->ino_lock);

	e = radix_tree_lookup(&im->ino_root, ino);
	if (!e) {
		e = kmem_cache_alloc(ino_entry_slab, GFP_ATOMIC);
		if (!e) {
			spin_unlock(&im->ino_lock);
			radix_tree_preload_end();
			goto retry;
		}
		if (radix_tree_insert(&im->ino_root, ino, e)) {
			spin_unlock(&im->ino_lock);
			kmem_cache_free(ino_entry_slab, e);
			radix_tree_preload_end();
			goto retry;
		}
		memset(e, 0, sizeof(struct ino_entry));
		e->ino = ino;

		list_add_tail(&e->list, &im->ino_list);
		if (type != ORPHAN_INO)
			im->ino_num++;
	}
	spin_unlock(&im->ino_lock);
	radix_tree_preload_end();
}

static void __remove_ino_entry(struct f2fs_sb_info *sbi, nid_t ino, int type)
{
	struct inode_management *im = &sbi->im[type];
	struct ino_entry *e;

	spin_lock(&im->ino_lock);
	e = radix_tree_lookup(&im->ino_root, ino);
	if (e) {
		list_del(&e->list);
		radix_tree_delete(&im->ino_root, ino);
		im->ino_num--;
		spin_unlock(&im->ino_lock);
		kmem_cache_free(ino_entry_slab, e);
		return;
	}
	spin_unlock(&im->ino_lock);
}

void add_dirty_inode(struct f2fs_sb_info *sbi, nid_t ino, int type)
{
	/* add new dirty ino entry into list */
	__add_ino_entry(sbi, ino, type);
}

void remove_dirty_inode(struct f2fs_sb_info *sbi, nid_t ino, int type)
{
	/* remove dirty ino entry from list */
	__remove_ino_entry(sbi, ino, type);
}

/* mode should be APPEND_INO or UPDATE_INO */
bool exist_written_data(struct f2fs_sb_info *sbi, nid_t ino, int mode)
{
	struct inode_management *im = &sbi->im[mode];
	struct ino_entry *e;

	spin_lock(&im->ino_lock);
	e = radix_tree_lookup(&im->ino_root, ino);
	spin_unlock(&im->ino_lock);
	return e ? true : false;
}

void release_dirty_inode(struct f2fs_sb_info *sbi)
{
	struct ino_entry *e, *tmp;
	int i;

	for (i = APPEND_INO; i <= UPDATE_INO; i++) {
		struct inode_management *im = &sbi->im[i];

		spin_lock(&im->ino_lock);
		list_for_each_entry_safe(e, tmp, &im->ino_list, list) {
			list_del(&e->list);
			radix_tree_delete(&im->ino_root, e->ino);
			kmem_cache_free(ino_entry_slab, e);
			im->ino_num--;
		}
		spin_unlock(&im->ino_lock);
	}
}

int acquire_orphan_inode(struct f2fs_sb_info *sbi)
{
	struct inode_management *im = &sbi->im[ORPHAN_INO];
	int err = 0;

	spin_lock(&im->ino_lock);
	if (unlikely(im->ino_num >= sbi->max_orphans))
		err = -ENOSPC;
	else
		im->ino_num++;
	spin_unlock(&im->ino_lock);

	return err;
}

void release_orphan_inode(struct f2fs_sb_info *sbi)
{
	struct inode_management *im = &sbi->im[ORPHAN_INO];

	spin_lock(&im->ino_lock);
	f2fs_bug_on(sbi, im->ino_num == 0);
	im->ino_num--;
	spin_unlock(&im->ino_lock);
}

void add_orphan_inode(struct f2fs_sb_info *sbi, nid_t ino)
{
	/* add new orphan ino entry into list */
	__add_ino_entry(sbi, ino, ORPHAN_INO);
}

void remove_orphan_inode(struct f2fs_sb_info *sbi, nid_t ino)
{
	/* remove orphan entry from orphan list */
	__remove_ino_entry(sbi, ino, ORPHAN_INO);
}

static void recover_orphan_inode(struct f2fs_sb_info *sbi, nid_t ino)
{
	struct inode *inode = f2fs_iget(sbi->sb, ino);
	f2fs_bug_on(sbi, IS_ERR(inode));
	clear_nlink(inode);

	/* truncate all the data during iput */
	iput(inode);
}

void recover_orphan_inodes(struct f2fs_sb_info *sbi)
{
	block_t start_blk, orphan_blocks, i, j;

	if (!is_set_ckpt_flags(F2FS_CKPT(sbi), CP_ORPHAN_PRESENT_FLAG))
		return;

	set_sbi_flag(sbi, SBI_POR_DOING);

	start_blk = __start_cp_addr(sbi) + 1 + __cp_payload(sbi);
	orphan_blocks = __start_sum_addr(sbi) - 1 - __cp_payload(sbi);

	ra_meta_pages(sbi, start_blk, orphan_blocks, META_CP);

	for (i = 0; i < orphan_blocks; i++) {
		struct page *page = get_meta_page(sbi, start_blk + i);
		struct f2fs_orphan_block *orphan_blk;

		orphan_blk = (struct f2fs_orphan_block *)page_address(page);
		for (j = 0; j < le32_to_cpu(orphan_blk->entry_count); j++) {
			nid_t ino = le32_to_cpu(orphan_blk->ino[j]);
			recover_orphan_inode(sbi, ino);
		}
		f2fs_put_page(page, 1);
	}
	/* clear Orphan Flag */
	clear_ckpt_flags(F2FS_CKPT(sbi), CP_ORPHAN_PRESENT_FLAG);
	clear_sbi_flag(sbi, SBI_POR_DOING);
	return;
}

static void write_orphan_inodes(struct f2fs_sb_info *sbi, block_t start_blk)
{
	struct list_head *head;
	struct f2fs_orphan_block *orphan_blk = NULL;
	unsigned int nentries = 0;
	unsigned short index;
	unsigned short orphan_blocks;
	struct page *page = NULL;
	struct ino_entry *orphan = NULL;
	struct inode_management *im = &sbi->im[ORPHAN_INO];

	orphan_blocks = GET_ORPHAN_BLOCKS(im->ino_num);

	for (index = 0; index < orphan_blocks; index++)
		grab_meta_page(sbi, start_blk + index);

	index = 1;
	spin_lock(&im->ino_lock);
	head = &im->ino_list;

	/* loop for each orphan inode entry and write them in Jornal block */
	list_for_each_entry(orphan, head, list) {
		if (!page) {
			page = find_get_page(META_MAPPING(sbi), start_blk++);
			f2fs_bug_on(sbi, !page);
			orphan_blk =
				(struct f2fs_orphan_block *)page_address(page);
			memset(orphan_blk, 0, sizeof(*orphan_blk));
			f2fs_put_page(page, 0);
		}

		orphan_blk->ino[nentries++] = cpu_to_le32(orphan->ino);

		if (nentries == F2FS_ORPHANS_PER_BLOCK) {
			/*
			 * an orphan block is full of 1020 entries,
			 * then we need to flush current orphan blocks
			 * and bring another one in memory
			 */
			orphan_blk->blk_addr = cpu_to_le16(index);
			orphan_blk->blk_count = cpu_to_le16(orphan_blocks);
			orphan_blk->entry_count = cpu_to_le32(nentries);
			set_page_dirty(page);
			f2fs_put_page(page, 1);
			index++;
			nentries = 0;
			page = NULL;
		}
	}

	if (page) {
		orphan_blk->blk_addr = cpu_to_le16(index);
		orphan_blk->blk_count = cpu_to_le16(orphan_blocks);
		orphan_blk->entry_count = cpu_to_le32(nentries);
		set_page_dirty(page);
		f2fs_put_page(page, 1);
	}

	spin_unlock(&im->ino_lock);
}

static struct page *validate_checkpoint(struct f2fs_sb_info *sbi,
				block_t cp_addr, unsigned long long *version)
{
	struct page *cp_page_1, *cp_page_2 = NULL;
	unsigned long blk_size = sbi->blocksize;
	struct f2fs_checkpoint *cp_block;
	unsigned long long cur_version = 0, pre_version = 0;
	size_t crc_offset;
	__u32 crc = 0;

	/* Read the 1st cp block in this CP pack */
	cp_page_1 = get_meta_page(sbi, cp_addr);

	/* get the version number */
	cp_block = (struct f2fs_checkpoint *)page_address(cp_page_1);
	crc_offset = le32_to_cpu(cp_block->checksum_offset);
	if (crc_offset >= blk_size)
		goto invalid_cp1;

	crc = le32_to_cpu(*((__le32 *)((unsigned char *)cp_block + crc_offset)));
	if (!f2fs_crc_valid(crc, cp_block, crc_offset))
		goto invalid_cp1;

	pre_version = cur_cp_version(cp_block);

	/* Read the 2nd cp block in this CP pack */
	cp_addr += le32_to_cpu(cp_block->cp_pack_total_block_count) - 1;
	cp_page_2 = get_meta_page(sbi, cp_addr);

	cp_block = (struct f2fs_checkpoint *)page_address(cp_page_2);
	crc_offset = le32_to_cpu(cp_block->checksum_offset);
	if (crc_offset >= blk_size)
		goto invalid_cp2;

	crc = le32_to_cpu(*((__le32 *)((unsigned char *)cp_block + crc_offset)));
	if (!f2fs_crc_valid(crc, cp_block, crc_offset))
		goto invalid_cp2;

	cur_version = cur_cp_version(cp_block);

	if (cur_version == pre_version) {
		*version = cur_version;
		f2fs_put_page(cp_page_2, 1);
		return cp_page_1;
	}
invalid_cp2:
	f2fs_put_page(cp_page_2, 1);
invalid_cp1:
	f2fs_put_page(cp_page_1, 1);
	return NULL;
}

int get_valid_checkpoint(struct f2fs_sb_info *sbi)
{
	struct f2fs_checkpoint *cp_block;
	struct f2fs_super_block *fsb = sbi->raw_super;
	struct page *cp1, *cp2, *cur_page;
	unsigned long blk_size = sbi->blocksize;
	unsigned long long cp1_version = 0, cp2_version = 0;
	unsigned long long cp_start_blk_no;
	unsigned int cp_blks = 1 + __cp_payload(sbi);
	block_t cp_blk_no;
	int i;

	sbi->ckpt = kzalloc(cp_blks * blk_size, GFP_KERNEL);
	if (!sbi->ckpt)
		return -ENOMEM;
	/*
	 * Finding out valid cp block involves read both
	 * sets( cp pack1 and cp pack 2)
	 */
	cp_start_blk_no = le32_to_cpu(fsb->cp_blkaddr);
	cp1 = validate_checkpoint(sbi, cp_start_blk_no, &cp1_version);

	/* The second checkpoint pack should start at the next segment */
	cp_start_blk_no += ((unsigned long long)1) <<
				le32_to_cpu(fsb->log_blocks_per_seg);
	cp2 = validate_checkpoint(sbi, cp_start_blk_no, &cp2_version);

	if (cp1 && cp2) {
		if (ver_after(cp2_version, cp1_version))
			cur_page = cp2;
		else
			cur_page = cp1;
	} else if (cp1) {
		cur_page = cp1;
	} else if (cp2) {
		cur_page = cp2;
	} else {
		goto fail_no_cp;
	}

	cp_block = (struct f2fs_checkpoint *)page_address(cur_page);
	memcpy(sbi->ckpt, cp_block, blk_size);

	if (cp_blks <= 1)
		goto done;

	cp_blk_no = le32_to_cpu(fsb->cp_blkaddr);
	if (cur_page == cp2)
		cp_blk_no += 1 << le32_to_cpu(fsb->log_blocks_per_seg);

	for (i = 1; i < cp_blks; i++) {
		void *sit_bitmap_ptr;
		unsigned char *ckpt = (unsigned char *)sbi->ckpt;

		cur_page = get_meta_page(sbi, cp_blk_no + i);
		sit_bitmap_ptr = page_address(cur_page);
		memcpy(ckpt + i * blk_size, sit_bitmap_ptr, blk_size);
		f2fs_put_page(cur_page, 1);
	}
done:
	f2fs_put_page(cp1, 1);
	f2fs_put_page(cp2, 1);
	return 0;

fail_no_cp:
	kfree(sbi->ckpt);
	return -EINVAL;
}

static int __add_dirty_inode(struct inode *inode, struct inode_entry *new)
{
	struct f2fs_sb_info *sbi = F2FS_I_SB(inode);

	if (is_inode_flag_set(F2FS_I(inode), FI_DIRTY_DIR))
		return -EEXIST;

	set_inode_flag(F2FS_I(inode), FI_DIRTY_DIR);
	F2FS_I(inode)->dirty_dir = new;
	list_add_tail(&new->list, &sbi->dir_inode_list);
	stat_inc_dirty_dir(sbi);
	return 0;
}

void update_dirty_page(struct inode *inode, struct page *page)
{
	struct f2fs_sb_info *sbi = F2FS_I_SB(inode);
	struct inode_entry *new;
	int ret = 0;

	if (!S_ISDIR(inode->i_mode) && !S_ISREG(inode->i_mode))
		return;

	if (!S_ISDIR(inode->i_mode)) {
		inode_inc_dirty_pages(inode);
		goto out;
	}

	new = f2fs_kmem_cache_alloc(inode_entry_slab, GFP_NOFS);
	new->inode = inode;
	INIT_LIST_HEAD(&new->list);

	spin_lock(&sbi->dir_inode_lock);
	ret = __add_dirty_inode(inode, new);
	inode_inc_dirty_pages(inode);
	spin_unlock(&sbi->dir_inode_lock);

	if (ret)
		kmem_cache_free(inode_entry_slab, new);
out:
	SetPagePrivate(page);
	f2fs_trace_pid(page);
}

void add_dirty_dir_inode(struct inode *inode)
{
	struct f2fs_sb_info *sbi = F2FS_I_SB(inode);
	struct inode_entry *new =
			f2fs_kmem_cache_alloc(inode_entry_slab, GFP_NOFS);
	int ret = 0;

	new->inode = inode;
	INIT_LIST_HEAD(&new->list);

	spin_lock(&sbi->dir_inode_lock);
	ret = __add_dirty_inode(inode, new);
	spin_unlock(&sbi->dir_inode_lock);

	if (ret)
		kmem_cache_free(inode_entry_slab, new);
}

void remove_dirty_dir_inode(struct inode *inode)
{
	struct f2fs_sb_info *sbi = F2FS_I_SB(inode);
	struct inode_entry *entry;

	if (!S_ISDIR(inode->i_mode))
		return;

	spin_lock(&sbi->dir_inode_lock);
	if (get_dirty_pages(inode) ||
			!is_inode_flag_set(F2FS_I(inode), FI_DIRTY_DIR)) {
		spin_unlock(&sbi->dir_inode_lock);
		return;
	}

	entry = F2FS_I(inode)->dirty_dir;
	list_del(&entry->list);
	F2FS_I(inode)->dirty_dir = NULL;
	clear_inode_flag(F2FS_I(inode), FI_DIRTY_DIR);
	stat_dec_dirty_dir(sbi);
	spin_unlock(&sbi->dir_inode_lock);
	kmem_cache_free(inode_entry_slab, entry);

	/* Only from the recovery routine */
	if (is_inode_flag_set(F2FS_I(inode), FI_DELAY_IPUT)) {
		clear_inode_flag(F2FS_I(inode), FI_DELAY_IPUT);
		iput(inode);
	}
}

void sync_dirty_dir_inodes(struct f2fs_sb_info *sbi)
{
	struct list_head *head;
	struct inode_entry *entry;
	struct inode *inode;
retry:
	if (unlikely(f2fs_cp_error(sbi)))
		return;

	spin_lock(&sbi->dir_inode_lock);

	head = &sbi->dir_inode_list;
	if (list_empty(head)) {
		spin_unlock(&sbi->dir_inode_lock);
		return;
	}
	entry = list_entry(head->next, struct inode_entry, list);
	inode = igrab(entry->inode);
	spin_unlock(&sbi->dir_inode_lock);
	if (inode) {
		filemap_fdatawrite(inode->i_mapping);
		iput(inode);
	} else {
		/*
		 * We should submit bio, since it exists several
		 * wribacking dentry pages in the freeing inode.
		 */
		f2fs_submit_merged_bio(sbi, DATA, WRITE);
		cond_resched();
	}
	goto retry;
}

/*
 * Freeze all the FS-operations for checkpoint.
 */
static int block_operations(struct f2fs_sb_info *sbi)
{
	struct writeback_control wbc = {
		.sync_mode = WB_SYNC_ALL,
		.nr_to_write = LONG_MAX,
		.for_reclaim = 0,
	};
	struct blk_plug plug;
	int err = 0;

	blk_start_plug(&plug);

retry_flush_dents:
	f2fs_lock_all(sbi);
	/* write all the dirty dentry pages */
	if (get_pages(sbi, F2FS_DIRTY_DENTS)) {
		f2fs_unlock_all(sbi);
		sync_dirty_dir_inodes(sbi);
		if (unlikely(f2fs_cp_error(sbi))) {
			err = -EIO;
			goto out;
		}
		goto retry_flush_dents;
	}

	/*
	 * POR: we should ensure that there are no dirty node pages
	 * until finishing nat/sit flush.
	 */
retry_flush_nodes:
	down_write(&sbi->node_write);

	if (get_pages(sbi, F2FS_DIRTY_NODES)) {
		up_write(&sbi->node_write);
		sync_node_pages(sbi, 0, &wbc);
		if (unlikely(f2fs_cp_error(sbi))) {
			f2fs_unlock_all(sbi);
			err = -EIO;
			goto out;
		}
		goto retry_flush_nodes;
	}
out:
	blk_finish_plug(&plug);
	return err;
}

static void unblock_operations(struct f2fs_sb_info *sbi)
{
	up_write(&sbi->node_write);
	f2fs_unlock_all(sbi);
}

static void wait_on_all_pages_writeback(struct f2fs_sb_info *sbi)
{
	DEFINE_WAIT(wait);

	for (;;) {
		prepare_to_wait(&sbi->cp_wait, &wait, TASK_UNINTERRUPTIBLE);

		if (!get_pages(sbi, F2FS_WRITEBACK))
			break;

		io_schedule();
	}
	finish_wait(&sbi->cp_wait, &wait);
}

static void do_checkpoint(struct f2fs_sb_info *sbi, struct cp_control *cpc)
{
	struct f2fs_checkpoint *ckpt = F2FS_CKPT(sbi);
	struct curseg_info *curseg = CURSEG_I(sbi, CURSEG_WARM_NODE);
	struct f2fs_nm_info *nm_i = NM_I(sbi);
	unsigned long orphan_num = sbi->im[ORPHAN_INO].ino_num;
	nid_t last_nid = nm_i->next_scan_nid;
	block_t start_blk;
	struct page *cp_page;
	unsigned int data_sum_blocks, orphan_blocks;
	__u32 crc32 = 0;
	void *kaddr;
	int i;
	int cp_payload_blks = __cp_payload(sbi);

	/*
	 * This avoids to conduct wrong roll-forward operations and uses
	 * metapages, so should be called prior to sync_meta_pages below.
	 */
	discard_next_dnode(sbi, NEXT_FREE_BLKADDR(sbi, curseg));

	/* Flush all the NAT/SIT pages */
	while (get_pages(sbi, F2FS_DIRTY_META)) {
		sync_meta_pages(sbi, META, LONG_MAX);
		if (unlikely(f2fs_cp_error(sbi)))
			return;
	}

	next_free_nid(sbi, &last_nid);

	/*
	 * modify checkpoint
	 * version number is already updated
	 */
	ckpt->elapsed_time = cpu_to_le64(get_mtime(sbi));
	ckpt->valid_block_count = cpu_to_le64(valid_user_blocks(sbi));
	ckpt->free_segment_count = cpu_to_le32(free_segments(sbi));
	for (i = 0; i < NR_CURSEG_NODE_TYPE; i++) {
		ckpt->cur_node_segno[i] =
			cpu_to_le32(curseg_segno(sbi, i + CURSEG_HOT_NODE));
		ckpt->cur_node_blkoff[i] =
			cpu_to_le16(curseg_blkoff(sbi, i + CURSEG_HOT_NODE));
		ckpt->alloc_type[i + CURSEG_HOT_NODE] =
				curseg_alloc_type(sbi, i + CURSEG_HOT_NODE);
	}
	for (i = 0; i < NR_CURSEG_DATA_TYPE; i++) {
		ckpt->cur_data_segno[i] =
			cpu_to_le32(curseg_segno(sbi, i + CURSEG_HOT_DATA));
		ckpt->cur_data_blkoff[i] =
			cpu_to_le16(curseg_blkoff(sbi, i + CURSEG_HOT_DATA));
		ckpt->alloc_type[i + CURSEG_HOT_DATA] =
				curseg_alloc_type(sbi, i + CURSEG_HOT_DATA);
	}

	ckpt->valid_node_count = cpu_to_le32(valid_node_count(sbi));
	ckpt->valid_inode_count = cpu_to_le32(valid_inode_count(sbi));
	ckpt->next_free_nid = cpu_to_le32(last_nid);

	/* 2 cp  + n data seg summary + orphan inode blocks */
	data_sum_blocks = npages_for_summary_flush(sbi, false);
	if (data_sum_blocks < NR_CURSEG_DATA_TYPE)
		set_ckpt_flags(ckpt, CP_COMPACT_SUM_FLAG);
	else
		clear_ckpt_flags(ckpt, CP_COMPACT_SUM_FLAG);

	orphan_blocks = GET_ORPHAN_BLOCKS(orphan_num);
	ckpt->cp_pack_start_sum = cpu_to_le32(1 + cp_payload_blks +
			orphan_blocks);

	if (__remain_node_summaries(cpc->reason))
		ckpt->cp_pack_total_block_count = cpu_to_le32(F2FS_CP_PACKS+
				cp_payload_blks + data_sum_blocks +
				orphan_blocks + NR_CURSEG_NODE_TYPE);
	else
		ckpt->cp_pack_total_block_count = cpu_to_le32(F2FS_CP_PACKS +
				cp_payload_blks + data_sum_blocks +
				orphan_blocks);

	if (cpc->reason == CP_UMOUNT)
		set_ckpt_flags(ckpt, CP_UMOUNT_FLAG);
	else
		clear_ckpt_flags(ckpt, CP_UMOUNT_FLAG);

	if (cpc->reason == CP_FASTBOOT)
		set_ckpt_flags(ckpt, CP_FASTBOOT_FLAG);
	else
		clear_ckpt_flags(ckpt, CP_FASTBOOT_FLAG);

	if (orphan_num)
		set_ckpt_flags(ckpt, CP_ORPHAN_PRESENT_FLAG);
	else
		clear_ckpt_flags(ckpt, CP_ORPHAN_PRESENT_FLAG);

	if (is_sbi_flag_set(sbi, SBI_NEED_FSCK))
		set_ckpt_flags(ckpt, CP_FSCK_FLAG);

	/* update SIT/NAT bitmap */
	get_sit_bitmap(sbi, __bitmap_ptr(sbi, SIT_BITMAP));
	get_nat_bitmap(sbi, __bitmap_ptr(sbi, NAT_BITMAP));

	crc32 = f2fs_crc32(ckpt, le32_to_cpu(ckpt->checksum_offset));
	*((__le32 *)((unsigned char *)ckpt +
				le32_to_cpu(ckpt->checksum_offset)))
				= cpu_to_le32(crc32);

	start_blk = __start_cp_addr(sbi);

	/* write out checkpoint buffer at block 0 */
	cp_page = grab_meta_page(sbi, start_blk++);
	kaddr = page_address(cp_page);
	memcpy(kaddr, ckpt, F2FS_BLKSIZE);
	set_page_dirty(cp_page);
	f2fs_put_page(cp_page, 1);

	for (i = 1; i < 1 + cp_payload_blks; i++) {
		cp_page = grab_meta_page(sbi, start_blk++);
		kaddr = page_address(cp_page);
		memcpy(kaddr, (char *)ckpt + i * F2FS_BLKSIZE, F2FS_BLKSIZE);
		set_page_dirty(cp_page);
		f2fs_put_page(cp_page, 1);
	}

	if (orphan_num) {
		write_orphan_inodes(sbi, start_blk);
		start_blk += orphan_blocks;
	}

	write_data_summaries(sbi, start_blk);
	start_blk += data_sum_blocks;
	if (__remain_node_summaries(cpc->reason)) {
		write_node_summaries(sbi, start_blk);
		start_blk += NR_CURSEG_NODE_TYPE;
	}

	/* writeout checkpoint block */
	cp_page = grab_meta_page(sbi, start_blk);
	kaddr = page_address(cp_page);
	memcpy(kaddr, ckpt, F2FS_BLKSIZE);
	set_page_dirty(cp_page);
	f2fs_put_page(cp_page, 1);

	/* wait for previous submitted node/meta pages writeback */
	wait_on_all_pages_writeback(sbi);

	if (unlikely(f2fs_cp_error(sbi)))
		return;

	filemap_fdatawait_range(NODE_MAPPING(sbi), 0, LONG_MAX);
	filemap_fdatawait_range(META_MAPPING(sbi), 0, LONG_MAX);

	/* update user_block_counts */
	sbi->last_valid_block_count = sbi->total_valid_block_count;
	sbi->alloc_valid_block_count = 0;

	/* Here, we only have one bio having CP pack */
	sync_meta_pages(sbi, META_FLUSH, LONG_MAX);

	/* wait for previous submitted meta pages writeback */
	wait_on_all_pages_writeback(sbi);

	release_dirty_inode(sbi);

	if (unlikely(f2fs_cp_error(sbi)))
		return;

	clear_prefree_segments(sbi);
	clear_sbi_flag(sbi, SBI_IS_DIRTY);
}

/*
 * We guarantee that this checkpoint procedure will not fail.
 */
void write_checkpoint(struct f2fs_sb_info *sbi, struct cp_control *cpc)
{
	struct f2fs_checkpoint *ckpt = F2FS_CKPT(sbi);
	unsigned long long ckpt_ver;

	mutex_lock(&sbi->cp_mutex);

	if (!is_sbi_flag_set(sbi, SBI_IS_DIRTY) &&
		(cpc->reason == CP_FASTBOOT || cpc->reason == CP_SYNC))
		goto out;
	if (unlikely(f2fs_cp_error(sbi)))
		goto out;
	if (f2fs_readonly(sbi->sb))
		goto out;

	trace_f2fs_write_checkpoint(sbi->sb, cpc->reason, "start block_ops");

	if (block_operations(sbi))
		goto out;

	trace_f2fs_write_checkpoint(sbi->sb, cpc->reason, "finish block_ops");

	f2fs_submit_merged_bio(sbi, DATA, WRITE);
	f2fs_submit_merged_bio(sbi, NODE, WRITE);
	f2fs_submit_merged_bio(sbi, META, WRITE);

	/*
	 * update checkpoint pack index
	 * Increase the version number so that
	 * SIT entries and seg summaries are written at correct place
	 */
	ckpt_ver = cur_cp_version(ckpt);
	ckpt->checkpoint_ver = cpu_to_le64(++ckpt_ver);

	/* write cached NAT/SIT entries to NAT/SIT area */
	flush_nat_entries(sbi);
	flush_sit_entries(sbi, cpc);

	/* unlock all the fs_lock[] in do_checkpoint() */
	do_checkpoint(sbi, cpc);

	unblock_operations(sbi);
	stat_inc_cp_count(sbi->stat_info);

	if (cpc->reason == CP_RECOVERY)
		f2fs_msg(sbi->sb, KERN_NOTICE,
			"checkpoint: version = %llx", ckpt_ver);
out:
	mutex_unlock(&sbi->cp_mutex);
	trace_f2fs_write_checkpoint(sbi->sb, cpc->reason, "finish checkpoint");
}

void init_ino_entry_info(struct f2fs_sb_info *sbi)
{
	int i;

	for (i = 0; i < MAX_INO_ENTRY; i++) {
		struct inode_management *im = &sbi->im[i];

		INIT_RADIX_TREE(&im->ino_root, GFP_ATOMIC);
		spin_lock_init(&im->ino_lock);
		INIT_LIST_HEAD(&im->ino_list);
		im->ino_num = 0;
	}

	sbi->max_orphans = (sbi->blocks_per_seg - F2FS_CP_PACKS -
			NR_CURSEG_TYPE - __cp_payload(sbi)) *
				F2FS_ORPHANS_PER_BLOCK;
}

int __init create_checkpoint_caches(void)
{
	ino_entry_slab = f2fs_kmem_cache_create("f2fs_ino_entry",
			sizeof(struct ino_entry));
	if (!ino_entry_slab)
		return -ENOMEM;
	inode_entry_slab = f2fs_kmem_cache_create("f2fs_inode_entry",
			sizeof(struct inode_entry));
	if (!inode_entry_slab) {
		kmem_cache_destroy(ino_entry_slab);
		return -ENOMEM;
	}
	return 0;
}

void destroy_checkpoint_caches(void)
{
	kmem_cache_destroy(ino_entry_slab);
	kmem_cache_destroy(inode_entry_slab);
}
