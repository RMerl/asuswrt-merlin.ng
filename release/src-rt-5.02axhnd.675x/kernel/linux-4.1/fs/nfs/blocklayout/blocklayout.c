/*
 *  linux/fs/nfs/blocklayout/blocklayout.c
 *
 *  Module for the NFSv4.1 pNFS block layout driver.
 *
 *  Copyright (c) 2006 The Regents of the University of Michigan.
 *  All rights reserved.
 *
 *  Andy Adamson <andros@citi.umich.edu>
 *  Fred Isaman <iisaman@umich.edu>
 *
 * permission is granted to use, copy, create derivative works and
 * redistribute this software and such derivative works for any purpose,
 * so long as the name of the university of michigan is not used in
 * any advertising or publicity pertaining to the use or distribution
 * of this software without specific, written prior authorization.  if
 * the above copyright notice or any other identification of the
 * university of michigan is included in any copy of any portion of
 * this software, then the disclaimer below must also be included.
 *
 * this software is provided as is, without representation from the
 * university of michigan as to its fitness for any purpose, and without
 * warranty by the university of michigan of any kind, either express
 * or implied, including without limitation the implied warranties of
 * merchantability and fitness for a particular purpose.  the regents
 * of the university of michigan shall not be liable for any damages,
 * including special, indirect, incidental, or consequential damages,
 * with respect to any claim arising out or in connection with the use
 * of the software, even if it has been or is hereafter advised of the
 * possibility of such damages.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/mount.h>
#include <linux/namei.h>
#include <linux/bio.h>		/* struct bio */
#include <linux/prefetch.h>
#include <linux/pagevec.h>

#include "../pnfs.h"
#include "../nfs4session.h"
#include "../internal.h"
#include "blocklayout.h"

#define NFSDBG_FACILITY	NFSDBG_PNFS_LD

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Andy Adamson <andros@citi.umich.edu>");
MODULE_DESCRIPTION("The NFSv4.1 pNFS Block layout driver");

static bool is_hole(struct pnfs_block_extent *be)
{
	switch (be->be_state) {
	case PNFS_BLOCK_NONE_DATA:
		return true;
	case PNFS_BLOCK_INVALID_DATA:
		return be->be_tag ? false : true;
	default:
		return false;
	}
}

/* The data we are handed might be spread across several bios.  We need
 * to track when the last one is finished.
 */
struct parallel_io {
	struct kref refcnt;
	void (*pnfs_callback) (void *data);
	void *data;
};

static inline struct parallel_io *alloc_parallel(void *data)
{
	struct parallel_io *rv;

	rv  = kmalloc(sizeof(*rv), GFP_NOFS);
	if (rv) {
		rv->data = data;
		kref_init(&rv->refcnt);
	}
	return rv;
}

static inline void get_parallel(struct parallel_io *p)
{
	kref_get(&p->refcnt);
}

static void destroy_parallel(struct kref *kref)
{
	struct parallel_io *p = container_of(kref, struct parallel_io, refcnt);

	dprintk("%s enter\n", __func__);
	p->pnfs_callback(p->data);
	kfree(p);
}

static inline void put_parallel(struct parallel_io *p)
{
	kref_put(&p->refcnt, destroy_parallel);
}

static struct bio *
bl_submit_bio(int rw, struct bio *bio)
{
	if (bio) {
		get_parallel(bio->bi_private);
		dprintk("%s submitting %s bio %u@%llu\n", __func__,
			rw == READ ? "read" : "write", bio->bi_iter.bi_size,
			(unsigned long long)bio->bi_iter.bi_sector);
		submit_bio(rw, bio);
	}
	return NULL;
}

static struct bio *
bl_alloc_init_bio(int npg, struct block_device *bdev, sector_t disk_sector,
		void (*end_io)(struct bio *, int err), struct parallel_io *par)
{
	struct bio *bio;

	npg = min(npg, BIO_MAX_PAGES);
	bio = bio_alloc(GFP_NOIO, npg);
	if (!bio && (current->flags & PF_MEMALLOC)) {
		while (!bio && (npg /= 2))
			bio = bio_alloc(GFP_NOIO, npg);
	}

	if (bio) {
		bio->bi_iter.bi_sector = disk_sector;
		bio->bi_bdev = bdev;
		bio->bi_end_io = end_io;
		bio->bi_private = par;
	}
	return bio;
}

static struct bio *
do_add_page_to_bio(struct bio *bio, int npg, int rw, sector_t isect,
		struct page *page, struct pnfs_block_dev_map *map,
		struct pnfs_block_extent *be,
		void (*end_io)(struct bio *, int err),
		struct parallel_io *par, unsigned int offset, int *len)
{
	struct pnfs_block_dev *dev =
		container_of(be->be_device, struct pnfs_block_dev, node);
	u64 disk_addr, end;

	dprintk("%s: npg %d rw %d isect %llu offset %u len %d\n", __func__,
		npg, rw, (unsigned long long)isect, offset, *len);

	/* translate to device offset */
	isect += be->be_v_offset;
	isect -= be->be_f_offset;

	/* translate to physical disk offset */
	disk_addr = (u64)isect << SECTOR_SHIFT;
	if (disk_addr < map->start || disk_addr >= map->start + map->len) {
		if (!dev->map(dev, disk_addr, map))
			return ERR_PTR(-EIO);
		bio = bl_submit_bio(rw, bio);
	}
	disk_addr += map->disk_offset;
	disk_addr -= map->start;

	/* limit length to what the device mapping allows */
	end = disk_addr + *len;
	if (end >= map->start + map->len)
		*len = map->start + map->len - disk_addr;

retry:
	if (!bio) {
		bio = bl_alloc_init_bio(npg, map->bdev,
				disk_addr >> SECTOR_SHIFT, end_io, par);
		if (!bio)
			return ERR_PTR(-ENOMEM);
	}
	if (bio_add_page(bio, page, *len, offset) < *len) {
		bio = bl_submit_bio(rw, bio);
		goto retry;
	}
	return bio;
}

static void bl_end_io_read(struct bio *bio, int err)
{
	struct parallel_io *par = bio->bi_private;

	if (err) {
		struct nfs_pgio_header *header = par->data;

		if (!header->pnfs_error)
			header->pnfs_error = -EIO;
		pnfs_set_lo_fail(header->lseg);
	}

	bio_put(bio);
	put_parallel(par);
}

static void bl_read_cleanup(struct work_struct *work)
{
	struct rpc_task *task;
	struct nfs_pgio_header *hdr;
	dprintk("%s enter\n", __func__);
	task = container_of(work, struct rpc_task, u.tk_work);
	hdr = container_of(task, struct nfs_pgio_header, task);
	pnfs_ld_read_done(hdr);
}

static void
bl_end_par_io_read(void *data)
{
	struct nfs_pgio_header *hdr = data;

	hdr->task.tk_status = hdr->pnfs_error;
	INIT_WORK(&hdr->task.u.tk_work, bl_read_cleanup);
	schedule_work(&hdr->task.u.tk_work);
}

static enum pnfs_try_status
bl_read_pagelist(struct nfs_pgio_header *header)
{
	struct pnfs_block_layout *bl = BLK_LSEG2EXT(header->lseg);
	struct pnfs_block_dev_map map = { .start = NFS4_MAX_UINT64 };
	struct bio *bio = NULL;
	struct pnfs_block_extent be;
	sector_t isect, extent_length = 0;
	struct parallel_io *par;
	loff_t f_offset = header->args.offset;
	size_t bytes_left = header->args.count;
	unsigned int pg_offset, pg_len;
	struct page **pages = header->args.pages;
	int pg_index = header->args.pgbase >> PAGE_CACHE_SHIFT;
	const bool is_dio = (header->dreq != NULL);
	struct blk_plug plug;
	int i;

	dprintk("%s enter nr_pages %u offset %lld count %u\n", __func__,
		header->page_array.npages, f_offset,
		(unsigned int)header->args.count);

	par = alloc_parallel(header);
	if (!par)
		return PNFS_NOT_ATTEMPTED;
	par->pnfs_callback = bl_end_par_io_read;

	blk_start_plug(&plug);

	isect = (sector_t) (f_offset >> SECTOR_SHIFT);
	/* Code assumes extents are page-aligned */
	for (i = pg_index; i < header->page_array.npages; i++) {
		if (extent_length <= 0) {
			/* We've used up the previous extent */
			bio = bl_submit_bio(READ, bio);

			/* Get the next one */
			if (!ext_tree_lookup(bl, isect, &be, false)) {
				header->pnfs_error = -EIO;
				goto out;
			}
			extent_length = be.be_length - (isect - be.be_f_offset);
		}

		pg_offset = f_offset & ~PAGE_CACHE_MASK;
		if (is_dio) {
			if (pg_offset + bytes_left > PAGE_CACHE_SIZE)
				pg_len = PAGE_CACHE_SIZE - pg_offset;
			else
				pg_len = bytes_left;
		} else {
			BUG_ON(pg_offset != 0);
			pg_len = PAGE_CACHE_SIZE;
		}

		isect += (pg_offset >> SECTOR_SHIFT);
		extent_length -= (pg_offset >> SECTOR_SHIFT);

		if (is_hole(&be)) {
			bio = bl_submit_bio(READ, bio);
			/* Fill hole w/ zeroes w/o accessing device */
			dprintk("%s Zeroing page for hole\n", __func__);
			zero_user_segment(pages[i], pg_offset, pg_len);

			/* invalidate map */
			map.start = NFS4_MAX_UINT64;
		} else {
			bio = do_add_page_to_bio(bio,
						 header->page_array.npages - i,
						 READ,
						 isect, pages[i], &map, &be,
						 bl_end_io_read, par,
						 pg_offset, &pg_len);
			if (IS_ERR(bio)) {
				header->pnfs_error = PTR_ERR(bio);
				bio = NULL;
				goto out;
			}
		}
		isect += (pg_len >> SECTOR_SHIFT);
		extent_length -= (pg_len >> SECTOR_SHIFT);
		f_offset += pg_len;
		bytes_left -= pg_len;
	}
	if ((isect << SECTOR_SHIFT) >= header->inode->i_size) {
		header->res.eof = 1;
		header->res.count = header->inode->i_size - header->args.offset;
	} else {
		header->res.count = (isect << SECTOR_SHIFT) - header->args.offset;
	}
out:
	bl_submit_bio(READ, bio);
	blk_finish_plug(&plug);
	put_parallel(par);
	return PNFS_ATTEMPTED;
}

static void bl_end_io_write(struct bio *bio, int err)
{
	struct parallel_io *par = bio->bi_private;
	const int uptodate = test_bit(BIO_UPTODATE, &bio->bi_flags);
	struct nfs_pgio_header *header = par->data;

	if (!uptodate) {
		if (!header->pnfs_error)
			header->pnfs_error = -EIO;
		pnfs_set_lo_fail(header->lseg);
	}
	bio_put(bio);
	put_parallel(par);
}

/* Function scheduled for call during bl_end_par_io_write,
 * it marks sectors as written and extends the commitlist.
 */
static void bl_write_cleanup(struct work_struct *work)
{
	struct rpc_task *task = container_of(work, struct rpc_task, u.tk_work);
	struct nfs_pgio_header *hdr =
			container_of(task, struct nfs_pgio_header, task);

	dprintk("%s enter\n", __func__);

	if (likely(!hdr->pnfs_error)) {
		struct pnfs_block_layout *bl = BLK_LSEG2EXT(hdr->lseg);
		u64 start = hdr->args.offset & (loff_t)PAGE_CACHE_MASK;
		u64 end = (hdr->args.offset + hdr->args.count +
			PAGE_CACHE_SIZE - 1) & (loff_t)PAGE_CACHE_MASK;

		ext_tree_mark_written(bl, start >> SECTOR_SHIFT,
					(end - start) >> SECTOR_SHIFT);
	}

	pnfs_ld_write_done(hdr);
}

/* Called when last of bios associated with a bl_write_pagelist call finishes */
static void bl_end_par_io_write(void *data)
{
	struct nfs_pgio_header *hdr = data;

	hdr->task.tk_status = hdr->pnfs_error;
	hdr->verf.committed = NFS_FILE_SYNC;
	INIT_WORK(&hdr->task.u.tk_work, bl_write_cleanup);
	schedule_work(&hdr->task.u.tk_work);
}

static enum pnfs_try_status
bl_write_pagelist(struct nfs_pgio_header *header, int sync)
{
	struct pnfs_block_layout *bl = BLK_LSEG2EXT(header->lseg);
	struct pnfs_block_dev_map map = { .start = NFS4_MAX_UINT64 };
	struct bio *bio = NULL;
	struct pnfs_block_extent be;
	sector_t isect, extent_length = 0;
	struct parallel_io *par = NULL;
	loff_t offset = header->args.offset;
	size_t count = header->args.count;
	struct page **pages = header->args.pages;
	int pg_index = header->args.pgbase >> PAGE_CACHE_SHIFT;
	unsigned int pg_len;
	struct blk_plug plug;
	int i;

	dprintk("%s enter, %Zu@%lld\n", __func__, count, offset);

	/* At this point, header->page_aray is a (sequential) list of nfs_pages.
	 * We want to write each, and if there is an error set pnfs_error
	 * to have it redone using nfs.
	 */
	par = alloc_parallel(header);
	if (!par)
		return PNFS_NOT_ATTEMPTED;
	par->pnfs_callback = bl_end_par_io_write;

	blk_start_plug(&plug);

	/* we always write out the whole page */
	offset = offset & (loff_t)PAGE_CACHE_MASK;
	isect = offset >> SECTOR_SHIFT;

	for (i = pg_index; i < header->page_array.npages; i++) {
		if (extent_length <= 0) {
			/* We've used up the previous extent */
			bio = bl_submit_bio(WRITE, bio);
			/* Get the next one */
			if (!ext_tree_lookup(bl, isect, &be, true)) {
				header->pnfs_error = -EINVAL;
				goto out;
			}

			extent_length = be.be_length - (isect - be.be_f_offset);
		}

		pg_len = PAGE_CACHE_SIZE;
		bio = do_add_page_to_bio(bio, header->page_array.npages - i,
					 WRITE, isect, pages[i], &map, &be,
					 bl_end_io_write, par,
					 0, &pg_len);
		if (IS_ERR(bio)) {
			header->pnfs_error = PTR_ERR(bio);
			bio = NULL;
			goto out;
		}

		offset += pg_len;
		count -= pg_len;
		isect += (pg_len >> SECTOR_SHIFT);
		extent_length -= (pg_len >> SECTOR_SHIFT);
	}

	header->res.count = header->args.count;
out:
	bl_submit_bio(WRITE, bio);
	blk_finish_plug(&plug);
	put_parallel(par);
	return PNFS_ATTEMPTED;
}

static void bl_free_layout_hdr(struct pnfs_layout_hdr *lo)
{
	struct pnfs_block_layout *bl = BLK_LO2EXT(lo);
	int err;

	dprintk("%s enter\n", __func__);

	err = ext_tree_remove(bl, true, 0, LLONG_MAX);
	WARN_ON(err);

	kfree(bl);
}

static struct pnfs_layout_hdr *bl_alloc_layout_hdr(struct inode *inode,
						   gfp_t gfp_flags)
{
	struct pnfs_block_layout *bl;

	dprintk("%s enter\n", __func__);
	bl = kzalloc(sizeof(*bl), gfp_flags);
	if (!bl)
		return NULL;

	bl->bl_ext_rw = RB_ROOT;
	bl->bl_ext_ro = RB_ROOT;
	spin_lock_init(&bl->bl_ext_lock);

	return &bl->bl_layout;
}

static void bl_free_lseg(struct pnfs_layout_segment *lseg)
{
	dprintk("%s enter\n", __func__);
	kfree(lseg);
}

/* Tracks info needed to ensure extents in layout obey constraints of spec */
struct layout_verification {
	u32 mode;	/* R or RW */
	u64 start;	/* Expected start of next non-COW extent */
	u64 inval;	/* Start of INVAL coverage */
	u64 cowread;	/* End of COW read coverage */
};

/* Verify the extent meets the layout requirements of the pnfs-block draft,
 * section 2.3.1.
 */
static int verify_extent(struct pnfs_block_extent *be,
			 struct layout_verification *lv)
{
	if (lv->mode == IOMODE_READ) {
		if (be->be_state == PNFS_BLOCK_READWRITE_DATA ||
		    be->be_state == PNFS_BLOCK_INVALID_DATA)
			return -EIO;
		if (be->be_f_offset != lv->start)
			return -EIO;
		lv->start += be->be_length;
		return 0;
	}
	/* lv->mode == IOMODE_RW */
	if (be->be_state == PNFS_BLOCK_READWRITE_DATA) {
		if (be->be_f_offset != lv->start)
			return -EIO;
		if (lv->cowread > lv->start)
			return -EIO;
		lv->start += be->be_length;
		lv->inval = lv->start;
		return 0;
	} else if (be->be_state == PNFS_BLOCK_INVALID_DATA) {
		if (be->be_f_offset != lv->start)
			return -EIO;
		lv->start += be->be_length;
		return 0;
	} else if (be->be_state == PNFS_BLOCK_READ_DATA) {
		if (be->be_f_offset > lv->start)
			return -EIO;
		if (be->be_f_offset < lv->inval)
			return -EIO;
		if (be->be_f_offset < lv->cowread)
			return -EIO;
		/* It looks like you might want to min this with lv->start,
		 * but you really don't.
		 */
		lv->inval = lv->inval + be->be_length;
		lv->cowread = be->be_f_offset + be->be_length;
		return 0;
	} else
		return -EIO;
}

static int decode_sector_number(__be32 **rp, sector_t *sp)
{
	uint64_t s;

	*rp = xdr_decode_hyper(*rp, &s);
	if (s & 0x1ff) {
		printk(KERN_WARNING "NFS: %s: sector not aligned\n", __func__);
		return -1;
	}
	*sp = s >> SECTOR_SHIFT;
	return 0;
}

static int
bl_alloc_extent(struct xdr_stream *xdr, struct pnfs_layout_hdr *lo,
		struct layout_verification *lv, struct list_head *extents,
		gfp_t gfp_mask)
{
	struct pnfs_block_extent *be;
	struct nfs4_deviceid id;
	int error;
	__be32 *p;

	p = xdr_inline_decode(xdr, 28 + NFS4_DEVICEID4_SIZE);
	if (!p)
		return -EIO;

	be = kzalloc(sizeof(*be), GFP_NOFS);
	if (!be)
		return -ENOMEM;

	memcpy(&id, p, NFS4_DEVICEID4_SIZE);
	p += XDR_QUADLEN(NFS4_DEVICEID4_SIZE);

	error = -EIO;
	be->be_device = nfs4_find_get_deviceid(NFS_SERVER(lo->plh_inode), &id,
						lo->plh_lc_cred, gfp_mask);
	if (!be->be_device)
		goto out_free_be;

	/*
	 * The next three values are read in as bytes, but stored in the
	 * extent structure in 512-byte granularity.
	 */
	if (decode_sector_number(&p, &be->be_f_offset) < 0)
		goto out_put_deviceid;
	if (decode_sector_number(&p, &be->be_length) < 0)
		goto out_put_deviceid;
	if (decode_sector_number(&p, &be->be_v_offset) < 0)
		goto out_put_deviceid;
	be->be_state = be32_to_cpup(p++);

	error = verify_extent(be, lv);
	if (error) {
		dprintk("%s: extent verification failed\n", __func__);
		goto out_put_deviceid;
	}

	list_add_tail(&be->be_list, extents);
	return 0;

out_put_deviceid:
	nfs4_put_deviceid_node(be->be_device);
out_free_be:
	kfree(be);
	return error;
}

static struct pnfs_layout_segment *
bl_alloc_lseg(struct pnfs_layout_hdr *lo, struct nfs4_layoutget_res *lgr,
		gfp_t gfp_mask)
{
	struct layout_verification lv = {
		.mode = lgr->range.iomode,
		.start = lgr->range.offset >> SECTOR_SHIFT,
		.inval = lgr->range.offset >> SECTOR_SHIFT,
		.cowread = lgr->range.offset >> SECTOR_SHIFT,
	};
	struct pnfs_block_layout *bl = BLK_LO2EXT(lo);
	struct pnfs_layout_segment *lseg;
	struct xdr_buf buf;
	struct xdr_stream xdr;
	struct page *scratch;
	int status, i;
	uint32_t count;
	__be32 *p;
	LIST_HEAD(extents);

	dprintk("---> %s\n", __func__);

	lseg = kzalloc(sizeof(*lseg), gfp_mask);
	if (!lseg)
		return ERR_PTR(-ENOMEM);

	status = -ENOMEM;
	scratch = alloc_page(gfp_mask);
	if (!scratch)
		goto out;

	xdr_init_decode_pages(&xdr, &buf,
			lgr->layoutp->pages, lgr->layoutp->len);
	xdr_set_scratch_buffer(&xdr, page_address(scratch), PAGE_SIZE);

	status = -EIO;
	p = xdr_inline_decode(&xdr, 4);
	if (unlikely(!p))
		goto out_free_scratch;

	count = be32_to_cpup(p++);
	dprintk("%s: number of extents %d\n", __func__, count);

	/*
	 * Decode individual extents, putting them in temporary staging area
	 * until whole layout is decoded to make error recovery easier.
	 */
	for (i = 0; i < count; i++) {
		status = bl_alloc_extent(&xdr, lo, &lv, &extents, gfp_mask);
		if (status)
			goto process_extents;
	}

	if (lgr->range.offset + lgr->range.length !=
			lv.start << SECTOR_SHIFT) {
		dprintk("%s Final length mismatch\n", __func__);
		status = -EIO;
		goto process_extents;
	}

	if (lv.start < lv.cowread) {
		dprintk("%s Final uncovered COW extent\n", __func__);
		status = -EIO;
	}

process_extents:
	while (!list_empty(&extents)) {
		struct pnfs_block_extent *be =
			list_first_entry(&extents, struct pnfs_block_extent,
					 be_list);
		list_del(&be->be_list);

		if (!status)
			status = ext_tree_insert(bl, be);

		if (status) {
			nfs4_put_deviceid_node(be->be_device);
			kfree(be);
		}
	}

out_free_scratch:
	__free_page(scratch);
out:
	dprintk("%s returns %d\n", __func__, status);
	if (status) {
		kfree(lseg);
		return ERR_PTR(status);
	}
	return lseg;
}

static void
bl_return_range(struct pnfs_layout_hdr *lo,
		struct pnfs_layout_range *range)
{
	struct pnfs_block_layout *bl = BLK_LO2EXT(lo);
	sector_t offset = range->offset >> SECTOR_SHIFT, end;

	if (range->offset % 8) {
		dprintk("%s: offset %lld not block size aligned\n",
			__func__, range->offset);
		return;
	}

	if (range->length != NFS4_MAX_UINT64) {
		if (range->length % 8) {
			dprintk("%s: length %lld not block size aligned\n",
				__func__, range->length);
			return;
		}

		end = offset + (range->length >> SECTOR_SHIFT);
	} else {
		end = round_down(NFS4_MAX_UINT64, PAGE_SIZE);
	}

	ext_tree_remove(bl, range->iomode & IOMODE_RW, offset, end);
}

static int
bl_prepare_layoutcommit(struct nfs4_layoutcommit_args *arg)
{
	return ext_tree_prepare_commit(arg);
}

static void
bl_cleanup_layoutcommit(struct nfs4_layoutcommit_data *lcdata)
{
	ext_tree_mark_committed(&lcdata->args, lcdata->res.status);
}

static int
bl_set_layoutdriver(struct nfs_server *server, const struct nfs_fh *fh)
{
	dprintk("%s enter\n", __func__);

	if (server->pnfs_blksize == 0) {
		dprintk("%s Server did not return blksize\n", __func__);
		return -EINVAL;
	}
	if (server->pnfs_blksize > PAGE_SIZE) {
		printk(KERN_ERR "%s: pNFS blksize %d not supported.\n",
			__func__, server->pnfs_blksize);
		return -EINVAL;
	}

	return 0;
}

static bool
is_aligned_req(struct nfs_pageio_descriptor *pgio,
		struct nfs_page *req, unsigned int alignment)
{
	/*
	 * Always accept buffered writes, higher layers take care of the
	 * right alignment.
	 */
	if (pgio->pg_dreq == NULL)
		return true;

	if (!IS_ALIGNED(req->wb_offset, alignment))
		return false;

	if (IS_ALIGNED(req->wb_bytes, alignment))
		return true;

	if (req_offset(req) + req->wb_bytes == i_size_read(pgio->pg_inode)) {
		/*
		 * If the write goes up to the inode size, just write
		 * the full page.  Data past the inode size is
		 * guaranteed to be zeroed by the higher level client
		 * code, and this behaviour is mandated by RFC 5663
		 * section 2.3.2.
		 */
		return true;
	}

	return false;
}

static void
bl_pg_init_read(struct nfs_pageio_descriptor *pgio, struct nfs_page *req)
{
	if (!is_aligned_req(pgio, req, SECTOR_SIZE)) {
		nfs_pageio_reset_read_mds(pgio);
		return;
	}

	pnfs_generic_pg_init_read(pgio, req);
}

/*
 * Return 0 if @req cannot be coalesced into @pgio, otherwise return the number
 * of bytes (maximum @req->wb_bytes) that can be coalesced.
 */
static size_t
bl_pg_test_read(struct nfs_pageio_descriptor *pgio, struct nfs_page *prev,
		struct nfs_page *req)
{
	if (!is_aligned_req(pgio, req, SECTOR_SIZE))
		return 0;
	return pnfs_generic_pg_test(pgio, prev, req);
}

/*
 * Return the number of contiguous bytes for a given inode
 * starting at page frame idx.
 */
static u64 pnfs_num_cont_bytes(struct inode *inode, pgoff_t idx)
{
	struct address_space *mapping = inode->i_mapping;
	pgoff_t end;

	/* Optimize common case that writes from 0 to end of file */
	end = DIV_ROUND_UP(i_size_read(inode), PAGE_CACHE_SIZE);
	if (end != inode->i_mapping->nrpages) {
		rcu_read_lock();
		end = page_cache_next_hole(mapping, idx + 1, ULONG_MAX);
		rcu_read_unlock();
	}

	if (!end)
		return i_size_read(inode) - (idx << PAGE_CACHE_SHIFT);
	else
		return (end - idx) << PAGE_CACHE_SHIFT;
}

static void
bl_pg_init_write(struct nfs_pageio_descriptor *pgio, struct nfs_page *req)
{
	u64 wb_size;

	if (!is_aligned_req(pgio, req, PAGE_SIZE)) {
		nfs_pageio_reset_write_mds(pgio);
		return;
	}

	if (pgio->pg_dreq == NULL)
		wb_size = pnfs_num_cont_bytes(pgio->pg_inode,
					      req->wb_index);
	else
		wb_size = nfs_dreq_bytes_left(pgio->pg_dreq);

	pnfs_generic_pg_init_write(pgio, req, wb_size);
}

/*
 * Return 0 if @req cannot be coalesced into @pgio, otherwise return the number
 * of bytes (maximum @req->wb_bytes) that can be coalesced.
 */
static size_t
bl_pg_test_write(struct nfs_pageio_descriptor *pgio, struct nfs_page *prev,
		 struct nfs_page *req)
{
	if (!is_aligned_req(pgio, req, PAGE_SIZE))
		return 0;
	return pnfs_generic_pg_test(pgio, prev, req);
}

static const struct nfs_pageio_ops bl_pg_read_ops = {
	.pg_init = bl_pg_init_read,
	.pg_test = bl_pg_test_read,
	.pg_doio = pnfs_generic_pg_readpages,
	.pg_cleanup = pnfs_generic_pg_cleanup,
};

static const struct nfs_pageio_ops bl_pg_write_ops = {
	.pg_init = bl_pg_init_write,
	.pg_test = bl_pg_test_write,
	.pg_doio = pnfs_generic_pg_writepages,
	.pg_cleanup = pnfs_generic_pg_cleanup,
};

static struct pnfs_layoutdriver_type blocklayout_type = {
	.id				= LAYOUT_BLOCK_VOLUME,
	.name				= "LAYOUT_BLOCK_VOLUME",
	.owner				= THIS_MODULE,
	.flags				= PNFS_LAYOUTRET_ON_SETATTR |
					  PNFS_READ_WHOLE_PAGE,
	.read_pagelist			= bl_read_pagelist,
	.write_pagelist			= bl_write_pagelist,
	.alloc_layout_hdr		= bl_alloc_layout_hdr,
	.free_layout_hdr		= bl_free_layout_hdr,
	.alloc_lseg			= bl_alloc_lseg,
	.free_lseg			= bl_free_lseg,
	.return_range			= bl_return_range,
	.prepare_layoutcommit		= bl_prepare_layoutcommit,
	.cleanup_layoutcommit		= bl_cleanup_layoutcommit,
	.set_layoutdriver		= bl_set_layoutdriver,
	.alloc_deviceid_node		= bl_alloc_deviceid_node,
	.free_deviceid_node		= bl_free_deviceid_node,
	.pg_read_ops			= &bl_pg_read_ops,
	.pg_write_ops			= &bl_pg_write_ops,
	.sync				= pnfs_generic_sync,
};

static int __init nfs4blocklayout_init(void)
{
	int ret;

	dprintk("%s: NFSv4 Block Layout Driver Registering...\n", __func__);

	ret = pnfs_register_layoutdriver(&blocklayout_type);
	if (ret)
		goto out;
	ret = bl_init_pipefs();
	if (ret)
		goto out_unregister;
	return 0;

out_unregister:
	pnfs_unregister_layoutdriver(&blocklayout_type);
out:
	return ret;
}

static void __exit nfs4blocklayout_exit(void)
{
	dprintk("%s: NFSv4 Block Layout Driver Unregistering...\n",
	       __func__);

	bl_cleanup_pipefs();
	pnfs_unregister_layoutdriver(&blocklayout_type);
}

MODULE_ALIAS("nfs-layouttype4-3");

module_init(nfs4blocklayout_init);
module_exit(nfs4blocklayout_exit);
