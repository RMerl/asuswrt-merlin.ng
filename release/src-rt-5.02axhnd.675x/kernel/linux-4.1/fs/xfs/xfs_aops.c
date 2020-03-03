/*
 * Copyright (c) 2000-2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software Foundation,
 * Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include "xfs.h"
#include "xfs_shared.h"
#include "xfs_format.h"
#include "xfs_log_format.h"
#include "xfs_trans_resv.h"
#include "xfs_mount.h"
#include "xfs_inode.h"
#include "xfs_trans.h"
#include "xfs_inode_item.h"
#include "xfs_alloc.h"
#include "xfs_error.h"
#include "xfs_iomap.h"
#include "xfs_trace.h"
#include "xfs_bmap.h"
#include "xfs_bmap_util.h"
#include "xfs_bmap_btree.h"
#include <linux/gfp.h>
#include <linux/mpage.h>
#include <linux/pagevec.h>
#include <linux/writeback.h>

void
xfs_count_page_state(
	struct page		*page,
	int			*delalloc,
	int			*unwritten)
{
	struct buffer_head	*bh, *head;

	*delalloc = *unwritten = 0;

	bh = head = page_buffers(page);
	do {
		if (buffer_unwritten(bh))
			(*unwritten) = 1;
		else if (buffer_delay(bh))
			(*delalloc) = 1;
	} while ((bh = bh->b_this_page) != head);
}

STATIC struct block_device *
xfs_find_bdev_for_inode(
	struct inode		*inode)
{
	struct xfs_inode	*ip = XFS_I(inode);
	struct xfs_mount	*mp = ip->i_mount;

	if (XFS_IS_REALTIME_INODE(ip))
		return mp->m_rtdev_targp->bt_bdev;
	else
		return mp->m_ddev_targp->bt_bdev;
}

/*
 * We're now finished for good with this ioend structure.
 * Update the page state via the associated buffer_heads,
 * release holds on the inode and bio, and finally free
 * up memory.  Do not use the ioend after this.
 */
STATIC void
xfs_destroy_ioend(
	xfs_ioend_t		*ioend)
{
	struct buffer_head	*bh, *next;

	for (bh = ioend->io_buffer_head; bh; bh = next) {
		next = bh->b_private;
		bh->b_end_io(bh, !ioend->io_error);
	}

	mempool_free(ioend, xfs_ioend_pool);
}

/*
 * Fast and loose check if this write could update the on-disk inode size.
 */
static inline bool xfs_ioend_is_append(struct xfs_ioend *ioend)
{
	return ioend->io_offset + ioend->io_size >
		XFS_I(ioend->io_inode)->i_d.di_size;
}

STATIC int
xfs_setfilesize_trans_alloc(
	struct xfs_ioend	*ioend)
{
	struct xfs_mount	*mp = XFS_I(ioend->io_inode)->i_mount;
	struct xfs_trans	*tp;
	int			error;

	tp = xfs_trans_alloc(mp, XFS_TRANS_FSYNC_TS);

	error = xfs_trans_reserve(tp, &M_RES(mp)->tr_fsyncts, 0, 0);
	if (error) {
		xfs_trans_cancel(tp, 0);
		return error;
	}

	ioend->io_append_trans = tp;

	/*
	 * We may pass freeze protection with a transaction.  So tell lockdep
	 * we released it.
	 */
	rwsem_release(&ioend->io_inode->i_sb->s_writers.lock_map[SB_FREEZE_FS-1],
		      1, _THIS_IP_);
	/*
	 * We hand off the transaction to the completion thread now, so
	 * clear the flag here.
	 */
	current_restore_flags_nested(&tp->t_pflags, PF_FSTRANS);
	return 0;
}

/*
 * Update on-disk file size now that data has been written to disk.
 */
STATIC int
xfs_setfilesize(
	struct xfs_inode	*ip,
	struct xfs_trans	*tp,
	xfs_off_t		offset,
	size_t			size)
{
	xfs_fsize_t		isize;

	xfs_ilock(ip, XFS_ILOCK_EXCL);
	isize = xfs_new_eof(ip, offset + size);
	if (!isize) {
		xfs_iunlock(ip, XFS_ILOCK_EXCL);
		xfs_trans_cancel(tp, 0);
		return 0;
	}

	trace_xfs_setfilesize(ip, offset, size);

	ip->i_d.di_size = isize;
	xfs_trans_ijoin(tp, ip, XFS_ILOCK_EXCL);
	xfs_trans_log_inode(tp, ip, XFS_ILOG_CORE);

	return xfs_trans_commit(tp, 0);
}

STATIC int
xfs_setfilesize_ioend(
	struct xfs_ioend	*ioend)
{
	struct xfs_inode	*ip = XFS_I(ioend->io_inode);
	struct xfs_trans	*tp = ioend->io_append_trans;

	/*
	 * The transaction may have been allocated in the I/O submission thread,
	 * thus we need to mark ourselves as being in a transaction manually.
	 * Similarly for freeze protection.
	 */
	current_set_flags_nested(&tp->t_pflags, PF_FSTRANS);
	rwsem_acquire_read(&VFS_I(ip)->i_sb->s_writers.lock_map[SB_FREEZE_FS-1],
			   0, 1, _THIS_IP_);

	return xfs_setfilesize(ip, tp, ioend->io_offset, ioend->io_size);
}

/*
 * Schedule IO completion handling on the final put of an ioend.
 *
 * If there is no work to do we might as well call it a day and free the
 * ioend right now.
 */
STATIC void
xfs_finish_ioend(
	struct xfs_ioend	*ioend)
{
	if (atomic_dec_and_test(&ioend->io_remaining)) {
		struct xfs_mount	*mp = XFS_I(ioend->io_inode)->i_mount;

		if (ioend->io_type == XFS_IO_UNWRITTEN)
			queue_work(mp->m_unwritten_workqueue, &ioend->io_work);
		else if (ioend->io_append_trans)
			queue_work(mp->m_data_workqueue, &ioend->io_work);
		else
			xfs_destroy_ioend(ioend);
	}
}

/*
 * IO write completion.
 */
STATIC void
xfs_end_io(
	struct work_struct *work)
{
	xfs_ioend_t	*ioend = container_of(work, xfs_ioend_t, io_work);
	struct xfs_inode *ip = XFS_I(ioend->io_inode);
	int		error = 0;

	if (XFS_FORCED_SHUTDOWN(ip->i_mount)) {
		ioend->io_error = -EIO;
		goto done;
	}
	if (ioend->io_error)
		goto done;

	/*
	 * For unwritten extents we need to issue transactions to convert a
	 * range to normal written extens after the data I/O has finished.
	 */
	if (ioend->io_type == XFS_IO_UNWRITTEN) {
		error = xfs_iomap_write_unwritten(ip, ioend->io_offset,
						  ioend->io_size);
	} else if (ioend->io_append_trans) {
		error = xfs_setfilesize_ioend(ioend);
	} else {
		ASSERT(!xfs_ioend_is_append(ioend));
	}

done:
	if (error)
		ioend->io_error = error;
	xfs_destroy_ioend(ioend);
}

/*
 * Allocate and initialise an IO completion structure.
 * We need to track unwritten extent write completion here initially.
 * We'll need to extend this for updating the ondisk inode size later
 * (vs. incore size).
 */
STATIC xfs_ioend_t *
xfs_alloc_ioend(
	struct inode		*inode,
	unsigned int		type)
{
	xfs_ioend_t		*ioend;

	ioend = mempool_alloc(xfs_ioend_pool, GFP_NOFS);

	/*
	 * Set the count to 1 initially, which will prevent an I/O
	 * completion callback from happening before we have started
	 * all the I/O from calling the completion routine too early.
	 */
	atomic_set(&ioend->io_remaining, 1);
	ioend->io_error = 0;
	ioend->io_list = NULL;
	ioend->io_type = type;
	ioend->io_inode = inode;
	ioend->io_buffer_head = NULL;
	ioend->io_buffer_tail = NULL;
	ioend->io_offset = 0;
	ioend->io_size = 0;
	ioend->io_append_trans = NULL;

	INIT_WORK(&ioend->io_work, xfs_end_io);
	return ioend;
}

STATIC int
xfs_map_blocks(
	struct inode		*inode,
	loff_t			offset,
	struct xfs_bmbt_irec	*imap,
	int			type,
	int			nonblocking)
{
	struct xfs_inode	*ip = XFS_I(inode);
	struct xfs_mount	*mp = ip->i_mount;
	ssize_t			count = 1 << inode->i_blkbits;
	xfs_fileoff_t		offset_fsb, end_fsb;
	int			error = 0;
	int			bmapi_flags = XFS_BMAPI_ENTIRE;
	int			nimaps = 1;

	if (XFS_FORCED_SHUTDOWN(mp))
		return -EIO;

	if (type == XFS_IO_UNWRITTEN)
		bmapi_flags |= XFS_BMAPI_IGSTATE;

	if (!xfs_ilock_nowait(ip, XFS_ILOCK_SHARED)) {
		if (nonblocking)
			return -EAGAIN;
		xfs_ilock(ip, XFS_ILOCK_SHARED);
	}

	ASSERT(ip->i_d.di_format != XFS_DINODE_FMT_BTREE ||
	       (ip->i_df.if_flags & XFS_IFEXTENTS));
	ASSERT(offset <= mp->m_super->s_maxbytes);

	if ((xfs_ufsize_t)offset + count > mp->m_super->s_maxbytes)
		count = mp->m_super->s_maxbytes - offset;
	end_fsb = XFS_B_TO_FSB(mp, (xfs_ufsize_t)offset + count);
	offset_fsb = XFS_B_TO_FSBT(mp, offset);
	error = xfs_bmapi_read(ip, offset_fsb, end_fsb - offset_fsb,
				imap, &nimaps, bmapi_flags);
	xfs_iunlock(ip, XFS_ILOCK_SHARED);

	if (error)
		return error;

	if (type == XFS_IO_DELALLOC &&
	    (!nimaps || isnullstartblock(imap->br_startblock))) {
		error = xfs_iomap_write_allocate(ip, offset, imap);
		if (!error)
			trace_xfs_map_blocks_alloc(ip, offset, count, type, imap);
		return error;
	}

#ifdef DEBUG
	if (type == XFS_IO_UNWRITTEN) {
		ASSERT(nimaps);
		ASSERT(imap->br_startblock != HOLESTARTBLOCK);
		ASSERT(imap->br_startblock != DELAYSTARTBLOCK);
	}
#endif
	if (nimaps)
		trace_xfs_map_blocks_found(ip, offset, count, type, imap);
	return 0;
}

STATIC int
xfs_imap_valid(
	struct inode		*inode,
	struct xfs_bmbt_irec	*imap,
	xfs_off_t		offset)
{
	offset >>= inode->i_blkbits;

	return offset >= imap->br_startoff &&
		offset < imap->br_startoff + imap->br_blockcount;
}

/*
 * BIO completion handler for buffered IO.
 */
STATIC void
xfs_end_bio(
	struct bio		*bio,
	int			error)
{
	xfs_ioend_t		*ioend = bio->bi_private;

	ASSERT(atomic_read(&bio->bi_cnt) >= 1);
	ioend->io_error = test_bit(BIO_UPTODATE, &bio->bi_flags) ? 0 : error;

	/* Toss bio and pass work off to an xfsdatad thread */
	bio->bi_private = NULL;
	bio->bi_end_io = NULL;
	bio_put(bio);

	xfs_finish_ioend(ioend);
}

STATIC void
xfs_submit_ioend_bio(
	struct writeback_control *wbc,
	xfs_ioend_t		*ioend,
	struct bio		*bio)
{
	atomic_inc(&ioend->io_remaining);
	bio->bi_private = ioend;
	bio->bi_end_io = xfs_end_bio;
	submit_bio(wbc->sync_mode == WB_SYNC_ALL ? WRITE_SYNC : WRITE, bio);
}

STATIC struct bio *
xfs_alloc_ioend_bio(
	struct buffer_head	*bh)
{
	int			nvecs = bio_get_nr_vecs(bh->b_bdev);
	struct bio		*bio = bio_alloc(GFP_NOIO, nvecs);

	ASSERT(bio->bi_private == NULL);
	bio->bi_iter.bi_sector = bh->b_blocknr * (bh->b_size >> 9);
	bio->bi_bdev = bh->b_bdev;
	return bio;
}

STATIC void
xfs_start_buffer_writeback(
	struct buffer_head	*bh)
{
	ASSERT(buffer_mapped(bh));
	ASSERT(buffer_locked(bh));
	ASSERT(!buffer_delay(bh));
	ASSERT(!buffer_unwritten(bh));

	mark_buffer_async_write(bh);
	set_buffer_uptodate(bh);
	clear_buffer_dirty(bh);
}

STATIC void
xfs_start_page_writeback(
	struct page		*page,
	int			clear_dirty,
	int			buffers)
{
	ASSERT(PageLocked(page));
	ASSERT(!PageWriteback(page));

	/*
	 * if the page was not fully cleaned, we need to ensure that the higher
	 * layers come back to it correctly. That means we need to keep the page
	 * dirty, and for WB_SYNC_ALL writeback we need to ensure the
	 * PAGECACHE_TAG_TOWRITE index mark is not removed so another attempt to
	 * write this page in this writeback sweep will be made.
	 */
	if (clear_dirty) {
		clear_page_dirty_for_io(page);
		set_page_writeback(page);
	} else
		set_page_writeback_keepwrite(page);

	unlock_page(page);

	/* If no buffers on the page are to be written, finish it here */
	if (!buffers)
		end_page_writeback(page);
}

static inline int xfs_bio_add_buffer(struct bio *bio, struct buffer_head *bh)
{
	return bio_add_page(bio, bh->b_page, bh->b_size, bh_offset(bh));
}

/*
 * Submit all of the bios for all of the ioends we have saved up, covering the
 * initial writepage page and also any probed pages.
 *
 * Because we may have multiple ioends spanning a page, we need to start
 * writeback on all the buffers before we submit them for I/O. If we mark the
 * buffers as we got, then we can end up with a page that only has buffers
 * marked async write and I/O complete on can occur before we mark the other
 * buffers async write.
 *
 * The end result of this is that we trip a bug in end_page_writeback() because
 * we call it twice for the one page as the code in end_buffer_async_write()
 * assumes that all buffers on the page are started at the same time.
 *
 * The fix is two passes across the ioend list - one to start writeback on the
 * buffer_heads, and then submit them for I/O on the second pass.
 *
 * If @fail is non-zero, it means that we have a situation where some part of
 * the submission process has failed after we have marked paged for writeback
 * and unlocked them. In this situation, we need to fail the ioend chain rather
 * than submit it to IO. This typically only happens on a filesystem shutdown.
 */
STATIC void
xfs_submit_ioend(
	struct writeback_control *wbc,
	xfs_ioend_t		*ioend,
	int			fail)
{
	xfs_ioend_t		*head = ioend;
	xfs_ioend_t		*next;
	struct buffer_head	*bh;
	struct bio		*bio;
	sector_t		lastblock = 0;

	/* Pass 1 - start writeback */
	do {
		next = ioend->io_list;
		for (bh = ioend->io_buffer_head; bh; bh = bh->b_private)
			xfs_start_buffer_writeback(bh);
	} while ((ioend = next) != NULL);

	/* Pass 2 - submit I/O */
	ioend = head;
	do {
		next = ioend->io_list;
		bio = NULL;

		/*
		 * If we are failing the IO now, just mark the ioend with an
		 * error and finish it. This will run IO completion immediately
		 * as there is only one reference to the ioend at this point in
		 * time.
		 */
		if (fail) {
			ioend->io_error = fail;
			xfs_finish_ioend(ioend);
			continue;
		}

		for (bh = ioend->io_buffer_head; bh; bh = bh->b_private) {

			if (!bio) {
 retry:
				bio = xfs_alloc_ioend_bio(bh);
			} else if (bh->b_blocknr != lastblock + 1) {
				xfs_submit_ioend_bio(wbc, ioend, bio);
				goto retry;
			}

			if (xfs_bio_add_buffer(bio, bh) != bh->b_size) {
				xfs_submit_ioend_bio(wbc, ioend, bio);
				goto retry;
			}

			lastblock = bh->b_blocknr;
		}
		if (bio)
			xfs_submit_ioend_bio(wbc, ioend, bio);
		xfs_finish_ioend(ioend);
	} while ((ioend = next) != NULL);
}

/*
 * Cancel submission of all buffer_heads so far in this endio.
 * Toss the endio too.  Only ever called for the initial page
 * in a writepage request, so only ever one page.
 */
STATIC void
xfs_cancel_ioend(
	xfs_ioend_t		*ioend)
{
	xfs_ioend_t		*next;
	struct buffer_head	*bh, *next_bh;

	do {
		next = ioend->io_list;
		bh = ioend->io_buffer_head;
		do {
			next_bh = bh->b_private;
			clear_buffer_async_write(bh);
			/*
			 * The unwritten flag is cleared when added to the
			 * ioend. We're not submitting for I/O so mark the
			 * buffer unwritten again for next time around.
			 */
			if (ioend->io_type == XFS_IO_UNWRITTEN)
				set_buffer_unwritten(bh);
			unlock_buffer(bh);
		} while ((bh = next_bh) != NULL);

		mempool_free(ioend, xfs_ioend_pool);
	} while ((ioend = next) != NULL);
}

/*
 * Test to see if we've been building up a completion structure for
 * earlier buffers -- if so, we try to append to this ioend if we
 * can, otherwise we finish off any current ioend and start another.
 * Return true if we've finished the given ioend.
 */
STATIC void
xfs_add_to_ioend(
	struct inode		*inode,
	struct buffer_head	*bh,
	xfs_off_t		offset,
	unsigned int		type,
	xfs_ioend_t		**result,
	int			need_ioend)
{
	xfs_ioend_t		*ioend = *result;

	if (!ioend || need_ioend || type != ioend->io_type) {
		xfs_ioend_t	*previous = *result;

		ioend = xfs_alloc_ioend(inode, type);
		ioend->io_offset = offset;
		ioend->io_buffer_head = bh;
		ioend->io_buffer_tail = bh;
		if (previous)
			previous->io_list = ioend;
		*result = ioend;
	} else {
		ioend->io_buffer_tail->b_private = bh;
		ioend->io_buffer_tail = bh;
	}

	bh->b_private = NULL;
	ioend->io_size += bh->b_size;
}

STATIC void
xfs_map_buffer(
	struct inode		*inode,
	struct buffer_head	*bh,
	struct xfs_bmbt_irec	*imap,
	xfs_off_t		offset)
{
	sector_t		bn;
	struct xfs_mount	*m = XFS_I(inode)->i_mount;
	xfs_off_t		iomap_offset = XFS_FSB_TO_B(m, imap->br_startoff);
	xfs_daddr_t		iomap_bn = xfs_fsb_to_db(XFS_I(inode), imap->br_startblock);

	ASSERT(imap->br_startblock != HOLESTARTBLOCK);
	ASSERT(imap->br_startblock != DELAYSTARTBLOCK);

	bn = (iomap_bn >> (inode->i_blkbits - BBSHIFT)) +
	      ((offset - iomap_offset) >> inode->i_blkbits);

	ASSERT(bn || XFS_IS_REALTIME_INODE(XFS_I(inode)));

	bh->b_blocknr = bn;
	set_buffer_mapped(bh);
}

STATIC void
xfs_map_at_offset(
	struct inode		*inode,
	struct buffer_head	*bh,
	struct xfs_bmbt_irec	*imap,
	xfs_off_t		offset)
{
	ASSERT(imap->br_startblock != HOLESTARTBLOCK);
	ASSERT(imap->br_startblock != DELAYSTARTBLOCK);

	xfs_map_buffer(inode, bh, imap, offset);
	set_buffer_mapped(bh);
	clear_buffer_delay(bh);
	clear_buffer_unwritten(bh);
}

/*
 * Test if a given page contains at least one buffer of a given @type.
 * If @check_all_buffers is true, then we walk all the buffers in the page to
 * try to find one of the type passed in. If it is not set, then the caller only
 * needs to check the first buffer on the page for a match.
 */
STATIC bool
xfs_check_page_type(
	struct page		*page,
	unsigned int		type,
	bool			check_all_buffers)
{
	struct buffer_head	*bh;
	struct buffer_head	*head;

	if (PageWriteback(page))
		return false;
	if (!page->mapping)
		return false;
	if (!page_has_buffers(page))
		return false;

	bh = head = page_buffers(page);
	do {
		if (buffer_unwritten(bh)) {
			if (type == XFS_IO_UNWRITTEN)
				return true;
		} else if (buffer_delay(bh)) {
			if (type == XFS_IO_DELALLOC)
				return true;
		} else if (buffer_dirty(bh) && buffer_mapped(bh)) {
			if (type == XFS_IO_OVERWRITE)
				return true;
		}

		/* If we are only checking the first buffer, we are done now. */
		if (!check_all_buffers)
			break;
	} while ((bh = bh->b_this_page) != head);

	return false;
}

/*
 * Allocate & map buffers for page given the extent map. Write it out.
 * except for the original page of a writepage, this is called on
 * delalloc/unwritten pages only, for the original page it is possible
 * that the page has no mapping at all.
 */
STATIC int
xfs_convert_page(
	struct inode		*inode,
	struct page		*page,
	loff_t			tindex,
	struct xfs_bmbt_irec	*imap,
	xfs_ioend_t		**ioendp,
	struct writeback_control *wbc)
{
	struct buffer_head	*bh, *head;
	xfs_off_t		end_offset;
	unsigned long		p_offset;
	unsigned int		type;
	int			len, page_dirty;
	int			count = 0, done = 0, uptodate = 1;
 	xfs_off_t		offset = page_offset(page);

	if (page->index != tindex)
		goto fail;
	if (!trylock_page(page))
		goto fail;
	if (PageWriteback(page))
		goto fail_unlock_page;
	if (page->mapping != inode->i_mapping)
		goto fail_unlock_page;
	if (!xfs_check_page_type(page, (*ioendp)->io_type, false))
		goto fail_unlock_page;

	/*
	 * page_dirty is initially a count of buffers on the page before
	 * EOF and is decremented as we move each into a cleanable state.
	 *
	 * Derivation:
	 *
	 * End offset is the highest offset that this page should represent.
	 * If we are on the last page, (end_offset & (PAGE_CACHE_SIZE - 1))
	 * will evaluate non-zero and be less than PAGE_CACHE_SIZE and
	 * hence give us the correct page_dirty count. On any other page,
	 * it will be zero and in that case we need page_dirty to be the
	 * count of buffers on the page.
	 */
	end_offset = min_t(unsigned long long,
			(xfs_off_t)(page->index + 1) << PAGE_CACHE_SHIFT,
			i_size_read(inode));

	/*
	 * If the current map does not span the entire page we are about to try
	 * to write, then give up. The only way we can write a page that spans
	 * multiple mappings in a single writeback iteration is via the
	 * xfs_vm_writepage() function. Data integrity writeback requires the
	 * entire page to be written in a single attempt, otherwise the part of
	 * the page we don't write here doesn't get written as part of the data
	 * integrity sync.
	 *
	 * For normal writeback, we also don't attempt to write partial pages
	 * here as it simply means that write_cache_pages() will see it under
	 * writeback and ignore the page until some point in the future, at
	 * which time this will be the only page in the file that needs
	 * writeback.  Hence for more optimal IO patterns, we should always
	 * avoid partial page writeback due to multiple mappings on a page here.
	 */
	if (!xfs_imap_valid(inode, imap, end_offset))
		goto fail_unlock_page;

	len = 1 << inode->i_blkbits;
	p_offset = min_t(unsigned long, end_offset & (PAGE_CACHE_SIZE - 1),
					PAGE_CACHE_SIZE);
	p_offset = p_offset ? roundup(p_offset, len) : PAGE_CACHE_SIZE;
	page_dirty = p_offset / len;

	/*
	 * The moment we find a buffer that doesn't match our current type
	 * specification or can't be written, abort the loop and start
	 * writeback. As per the above xfs_imap_valid() check, only
	 * xfs_vm_writepage() can handle partial page writeback fully - we are
	 * limited here to the buffers that are contiguous with the current
	 * ioend, and hence a buffer we can't write breaks that contiguity and
	 * we have to defer the rest of the IO to xfs_vm_writepage().
	 */
	bh = head = page_buffers(page);
	do {
		if (offset >= end_offset)
			break;
		if (!buffer_uptodate(bh))
			uptodate = 0;
		if (!(PageUptodate(page) || buffer_uptodate(bh))) {
			done = 1;
			break;
		}

		if (buffer_unwritten(bh) || buffer_delay(bh) ||
		    buffer_mapped(bh)) {
			if (buffer_unwritten(bh))
				type = XFS_IO_UNWRITTEN;
			else if (buffer_delay(bh))
				type = XFS_IO_DELALLOC;
			else
				type = XFS_IO_OVERWRITE;

			/*
			 * imap should always be valid because of the above
			 * partial page end_offset check on the imap.
			 */
			ASSERT(xfs_imap_valid(inode, imap, offset));

			lock_buffer(bh);
			if (type != XFS_IO_OVERWRITE)
				xfs_map_at_offset(inode, bh, imap, offset);
			xfs_add_to_ioend(inode, bh, offset, type,
					 ioendp, done);

			page_dirty--;
			count++;
		} else {
			done = 1;
			break;
		}
	} while (offset += len, (bh = bh->b_this_page) != head);

	if (uptodate && bh == head)
		SetPageUptodate(page);

	if (count) {
		if (--wbc->nr_to_write <= 0 &&
		    wbc->sync_mode == WB_SYNC_NONE)
			done = 1;
	}
	xfs_start_page_writeback(page, !page_dirty, count);

	return done;
 fail_unlock_page:
	unlock_page(page);
 fail:
	return 1;
}

/*
 * Convert & write out a cluster of pages in the same extent as defined
 * by mp and following the start page.
 */
STATIC void
xfs_cluster_write(
	struct inode		*inode,
	pgoff_t			tindex,
	struct xfs_bmbt_irec	*imap,
	xfs_ioend_t		**ioendp,
	struct writeback_control *wbc,
	pgoff_t			tlast)
{
	struct pagevec		pvec;
	int			done = 0, i;

	pagevec_init(&pvec, 0);
	while (!done && tindex <= tlast) {
		unsigned len = min_t(pgoff_t, PAGEVEC_SIZE, tlast - tindex + 1);

		if (!pagevec_lookup(&pvec, inode->i_mapping, tindex, len))
			break;

		for (i = 0; i < pagevec_count(&pvec); i++) {
			done = xfs_convert_page(inode, pvec.pages[i], tindex++,
					imap, ioendp, wbc);
			if (done)
				break;
		}

		pagevec_release(&pvec);
		cond_resched();
	}
}

STATIC void
xfs_vm_invalidatepage(
	struct page		*page,
	unsigned int		offset,
	unsigned int		length)
{
	trace_xfs_invalidatepage(page->mapping->host, page, offset,
				 length);
	block_invalidatepage(page, offset, length);
}

/*
 * If the page has delalloc buffers on it, we need to punch them out before we
 * invalidate the page. If we don't, we leave a stale delalloc mapping on the
 * inode that can trip a BUG() in xfs_get_blocks() later on if a direct IO read
 * is done on that same region - the delalloc extent is returned when none is
 * supposed to be there.
 *
 * We prevent this by truncating away the delalloc regions on the page before
 * invalidating it. Because they are delalloc, we can do this without needing a
 * transaction. Indeed - if we get ENOSPC errors, we have to be able to do this
 * truncation without a transaction as there is no space left for block
 * reservation (typically why we see a ENOSPC in writeback).
 *
 * This is not a performance critical path, so for now just do the punching a
 * buffer head at a time.
 */
STATIC void
xfs_aops_discard_page(
	struct page		*page)
{
	struct inode		*inode = page->mapping->host;
	struct xfs_inode	*ip = XFS_I(inode);
	struct buffer_head	*bh, *head;
	loff_t			offset = page_offset(page);

	if (!xfs_check_page_type(page, XFS_IO_DELALLOC, true))
		goto out_invalidate;

	if (XFS_FORCED_SHUTDOWN(ip->i_mount))
		goto out_invalidate;

	xfs_alert(ip->i_mount,
		"page discard on page %p, inode 0x%llx, offset %llu.",
			page, ip->i_ino, offset);

	xfs_ilock(ip, XFS_ILOCK_EXCL);
	bh = head = page_buffers(page);
	do {
		int		error;
		xfs_fileoff_t	start_fsb;

		if (!buffer_delay(bh))
			goto next_buffer;

		start_fsb = XFS_B_TO_FSBT(ip->i_mount, offset);
		error = xfs_bmap_punch_delalloc_range(ip, start_fsb, 1);
		if (error) {
			/* something screwed, just bail */
			if (!XFS_FORCED_SHUTDOWN(ip->i_mount)) {
				xfs_alert(ip->i_mount,
			"page discard unable to remove delalloc mapping.");
			}
			break;
		}
next_buffer:
		offset += 1 << inode->i_blkbits;

	} while ((bh = bh->b_this_page) != head);

	xfs_iunlock(ip, XFS_ILOCK_EXCL);
out_invalidate:
	xfs_vm_invalidatepage(page, 0, PAGE_CACHE_SIZE);
	return;
}

/*
 * Write out a dirty page.
 *
 * For delalloc space on the page we need to allocate space and flush it.
 * For unwritten space on the page we need to start the conversion to
 * regular allocated space.
 * For any other dirty buffer heads on the page we should flush them.
 */
STATIC int
xfs_vm_writepage(
	struct page		*page,
	struct writeback_control *wbc)
{
	struct inode		*inode = page->mapping->host;
	struct buffer_head	*bh, *head;
	struct xfs_bmbt_irec	imap;
	xfs_ioend_t		*ioend = NULL, *iohead = NULL;
	loff_t			offset;
	unsigned int		type;
	__uint64_t              end_offset;
	pgoff_t                 end_index, last_index;
	ssize_t			len;
	int			err, imap_valid = 0, uptodate = 1;
	int			count = 0;
	int			nonblocking = 0;

	trace_xfs_writepage(inode, page, 0, 0);

	ASSERT(page_has_buffers(page));

	/*
	 * Refuse to write the page out if we are called from reclaim context.
	 *
	 * This avoids stack overflows when called from deeply used stacks in
	 * random callers for direct reclaim or memcg reclaim.  We explicitly
	 * allow reclaim from kswapd as the stack usage there is relatively low.
	 *
	 * This should never happen except in the case of a VM regression so
	 * warn about it.
	 */
	if (WARN_ON_ONCE((current->flags & (PF_MEMALLOC|PF_KSWAPD)) ==
			PF_MEMALLOC))
		goto redirty;

	/*
	 * Given that we do not allow direct reclaim to call us, we should
	 * never be called while in a filesystem transaction.
	 */
	if (WARN_ON_ONCE(current->flags & PF_FSTRANS))
		goto redirty;

	/* Is this page beyond the end of the file? */
	offset = i_size_read(inode);
	end_index = offset >> PAGE_CACHE_SHIFT;
	last_index = (offset - 1) >> PAGE_CACHE_SHIFT;

	/*
	 * The page index is less than the end_index, adjust the end_offset
	 * to the highest offset that this page should represent.
	 * -----------------------------------------------------
	 * |			file mapping	       | <EOF> |
	 * -----------------------------------------------------
	 * | Page ... | Page N-2 | Page N-1 |  Page N  |       |
	 * ^--------------------------------^----------|--------
	 * |     desired writeback range    |      see else    |
	 * ---------------------------------^------------------|
	 */
	if (page->index < end_index)
		end_offset = (xfs_off_t)(page->index + 1) << PAGE_CACHE_SHIFT;
	else {
		/*
		 * Check whether the page to write out is beyond or straddles
		 * i_size or not.
		 * -------------------------------------------------------
		 * |		file mapping		        | <EOF>  |
		 * -------------------------------------------------------
		 * | Page ... | Page N-2 | Page N-1 |  Page N   | Beyond |
		 * ^--------------------------------^-----------|---------
		 * |				    |      Straddles     |
		 * ---------------------------------^-----------|--------|
		 */
		unsigned offset_into_page = offset & (PAGE_CACHE_SIZE - 1);

		/*
		 * Skip the page if it is fully outside i_size, e.g. due to a
		 * truncate operation that is in progress. We must redirty the
		 * page so that reclaim stops reclaiming it. Otherwise
		 * xfs_vm_releasepage() is called on it and gets confused.
		 *
		 * Note that the end_index is unsigned long, it would overflow
		 * if the given offset is greater than 16TB on 32-bit system
		 * and if we do check the page is fully outside i_size or not
		 * via "if (page->index >= end_index + 1)" as "end_index + 1"
		 * will be evaluated to 0.  Hence this page will be redirtied
		 * and be written out repeatedly which would result in an
		 * infinite loop, the user program that perform this operation
		 * will hang.  Instead, we can verify this situation by checking
		 * if the page to write is totally beyond the i_size or if it's
		 * offset is just equal to the EOF.
		 */
		if (page->index > end_index ||
		    (page->index == end_index && offset_into_page == 0))
			goto redirty;

		/*
		 * The page straddles i_size.  It must be zeroed out on each
		 * and every writepage invocation because it may be mmapped.
		 * "A file is mapped in multiples of the page size.  For a file
		 * that is not a multiple of the page size, the remaining
		 * memory is zeroed when mapped, and writes to that region are
		 * not written out to the file."
		 */
		zero_user_segment(page, offset_into_page, PAGE_CACHE_SIZE);

		/* Adjust the end_offset to the end of file */
		end_offset = offset;
	}

	len = 1 << inode->i_blkbits;

	bh = head = page_buffers(page);
	offset = page_offset(page);
	type = XFS_IO_OVERWRITE;

	if (wbc->sync_mode == WB_SYNC_NONE)
		nonblocking = 1;

	do {
		int new_ioend = 0;

		if (offset >= end_offset)
			break;
		if (!buffer_uptodate(bh))
			uptodate = 0;

		/*
		 * set_page_dirty dirties all buffers in a page, independent
		 * of their state.  The dirty state however is entirely
		 * meaningless for holes (!mapped && uptodate), so skip
		 * buffers covering holes here.
		 */
		if (!buffer_mapped(bh) && buffer_uptodate(bh)) {
			imap_valid = 0;
			continue;
		}

		if (buffer_unwritten(bh)) {
			if (type != XFS_IO_UNWRITTEN) {
				type = XFS_IO_UNWRITTEN;
				imap_valid = 0;
			}
		} else if (buffer_delay(bh)) {
			if (type != XFS_IO_DELALLOC) {
				type = XFS_IO_DELALLOC;
				imap_valid = 0;
			}
		} else if (buffer_uptodate(bh)) {
			if (type != XFS_IO_OVERWRITE) {
				type = XFS_IO_OVERWRITE;
				imap_valid = 0;
			}
		} else {
			if (PageUptodate(page))
				ASSERT(buffer_mapped(bh));
			/*
			 * This buffer is not uptodate and will not be
			 * written to disk.  Ensure that we will put any
			 * subsequent writeable buffers into a new
			 * ioend.
			 */
			imap_valid = 0;
			continue;
		}

		if (imap_valid)
			imap_valid = xfs_imap_valid(inode, &imap, offset);
		if (!imap_valid) {
			/*
			 * If we didn't have a valid mapping then we need to
			 * put the new mapping into a separate ioend structure.
			 * This ensures non-contiguous extents always have
			 * separate ioends, which is particularly important
			 * for unwritten extent conversion at I/O completion
			 * time.
			 */
			new_ioend = 1;
			err = xfs_map_blocks(inode, offset, &imap, type,
					     nonblocking);
			if (err)
				goto error;
			imap_valid = xfs_imap_valid(inode, &imap, offset);
		}
		if (imap_valid) {
			lock_buffer(bh);
			if (type != XFS_IO_OVERWRITE)
				xfs_map_at_offset(inode, bh, &imap, offset);
			xfs_add_to_ioend(inode, bh, offset, type, &ioend,
					 new_ioend);
			count++;
		}

		if (!iohead)
			iohead = ioend;

	} while (offset += len, ((bh = bh->b_this_page) != head));

	if (uptodate && bh == head)
		SetPageUptodate(page);

	xfs_start_page_writeback(page, 1, count);

	/* if there is no IO to be submitted for this page, we are done */
	if (!ioend)
		return 0;

	ASSERT(iohead);

	/*
	 * Any errors from this point onwards need tobe reported through the IO
	 * completion path as we have marked the initial page as under writeback
	 * and unlocked it.
	 */
	if (imap_valid) {
		xfs_off_t		end_index;

		end_index = imap.br_startoff + imap.br_blockcount;

		/* to bytes */
		end_index <<= inode->i_blkbits;

		/* to pages */
		end_index = (end_index - 1) >> PAGE_CACHE_SHIFT;

		/* check against file size */
		if (end_index > last_index)
			end_index = last_index;

		xfs_cluster_write(inode, page->index + 1, &imap, &ioend,
				  wbc, end_index);
	}


	/*
	 * Reserve log space if we might write beyond the on-disk inode size.
	 */
	err = 0;
	if (ioend->io_type != XFS_IO_UNWRITTEN && xfs_ioend_is_append(ioend))
		err = xfs_setfilesize_trans_alloc(ioend);

	xfs_submit_ioend(wbc, iohead, err);

	return 0;

error:
	if (iohead)
		xfs_cancel_ioend(iohead);

	if (err == -EAGAIN)
		goto redirty;

	xfs_aops_discard_page(page);
	ClearPageUptodate(page);
	unlock_page(page);
	return err;

redirty:
	redirty_page_for_writepage(wbc, page);
	unlock_page(page);
	return 0;
}

STATIC int
xfs_vm_writepages(
	struct address_space	*mapping,
	struct writeback_control *wbc)
{
	xfs_iflags_clear(XFS_I(mapping->host), XFS_ITRUNCATED);
	return generic_writepages(mapping, wbc);
}

/*
 * Called to move a page into cleanable state - and from there
 * to be released. The page should already be clean. We always
 * have buffer heads in this call.
 *
 * Returns 1 if the page is ok to release, 0 otherwise.
 */
STATIC int
xfs_vm_releasepage(
	struct page		*page,
	gfp_t			gfp_mask)
{
	int			delalloc, unwritten;

	trace_xfs_releasepage(page->mapping->host, page, 0, 0);

	xfs_count_page_state(page, &delalloc, &unwritten);

	if (WARN_ON_ONCE(delalloc))
		return 0;
	if (WARN_ON_ONCE(unwritten))
		return 0;

	return try_to_free_buffers(page);
}

/*
 * When we map a DIO buffer, we may need to attach an ioend that describes the
 * type of write IO we are doing. This passes to the completion function the
 * operations it needs to perform. If the mapping is for an overwrite wholly
 * within the EOF then we don't need an ioend and so we don't allocate one.
 * This avoids the unnecessary overhead of allocating and freeing ioends for
 * workloads that don't require transactions on IO completion.
 *
 * If we get multiple mappings in a single IO, we might be mapping different
 * types. But because the direct IO can only have a single private pointer, we
 * need to ensure that:
 *
 * a) i) the ioend spans the entire region of unwritten mappings; or
 *    ii) the ioend spans all the mappings that cross or are beyond EOF; and
 * b) if it contains unwritten extents, it is *permanently* marked as such
 *
 * We could do this by chaining ioends like buffered IO does, but we only
 * actually get one IO completion callback from the direct IO, and that spans
 * the entire IO regardless of how many mappings and IOs are needed to complete
 * the DIO. There is only going to be one reference to the ioend and its life
 * cycle is constrained by the DIO completion code. hence we don't need
 * reference counting here.
 */
static void
xfs_map_direct(
	struct inode		*inode,
	struct buffer_head	*bh_result,
	struct xfs_bmbt_irec	*imap,
	xfs_off_t		offset)
{
	struct xfs_ioend	*ioend;
	xfs_off_t		size = bh_result->b_size;
	int			type;

	if (ISUNWRITTEN(imap))
		type = XFS_IO_UNWRITTEN;
	else
		type = XFS_IO_OVERWRITE;

	trace_xfs_gbmap_direct(XFS_I(inode), offset, size, type, imap);

	if (bh_result->b_private) {
		ioend = bh_result->b_private;
		ASSERT(ioend->io_size > 0);
		ASSERT(offset >= ioend->io_offset);
		if (offset + size > ioend->io_offset + ioend->io_size)
			ioend->io_size = offset - ioend->io_offset + size;

		if (type == XFS_IO_UNWRITTEN && type != ioend->io_type)
			ioend->io_type = XFS_IO_UNWRITTEN;

		trace_xfs_gbmap_direct_update(XFS_I(inode), ioend->io_offset,
					      ioend->io_size, ioend->io_type,
					      imap);
	} else if (type == XFS_IO_UNWRITTEN ||
		   offset + size > i_size_read(inode)) {
		ioend = xfs_alloc_ioend(inode, type);
		ioend->io_offset = offset;
		ioend->io_size = size;

		bh_result->b_private = ioend;
		set_buffer_defer_completion(bh_result);

		trace_xfs_gbmap_direct_new(XFS_I(inode), offset, size, type,
					   imap);
	} else {
		trace_xfs_gbmap_direct_none(XFS_I(inode), offset, size, type,
					    imap);
	}
}

/*
 * If this is O_DIRECT or the mpage code calling tell them how large the mapping
 * is, so that we can avoid repeated get_blocks calls.
 *
 * If the mapping spans EOF, then we have to break the mapping up as the mapping
 * for blocks beyond EOF must be marked new so that sub block regions can be
 * correctly zeroed. We can't do this for mappings within EOF unless the mapping
 * was just allocated or is unwritten, otherwise the callers would overwrite
 * existing data with zeros. Hence we have to split the mapping into a range up
 * to and including EOF, and a second mapping for beyond EOF.
 */
static void
xfs_map_trim_size(
	struct inode		*inode,
	sector_t		iblock,
	struct buffer_head	*bh_result,
	struct xfs_bmbt_irec	*imap,
	xfs_off_t		offset,
	ssize_t			size)
{
	xfs_off_t		mapping_size;

	mapping_size = imap->br_startoff + imap->br_blockcount - iblock;
	mapping_size <<= inode->i_blkbits;

	ASSERT(mapping_size > 0);
	if (mapping_size > size)
		mapping_size = size;
	if (offset < i_size_read(inode) &&
	    (xfs_ufsize_t)offset + mapping_size >= i_size_read(inode)) {
		/* limit mapping to block that spans EOF */
		mapping_size = roundup_64(i_size_read(inode) - offset,
					  1 << inode->i_blkbits);
	}
	if (mapping_size > LONG_MAX)
		mapping_size = LONG_MAX;

	bh_result->b_size = mapping_size;
}

STATIC int
__xfs_get_blocks(
	struct inode		*inode,
	sector_t		iblock,
	struct buffer_head	*bh_result,
	int			create,
	int			direct)
{
	struct xfs_inode	*ip = XFS_I(inode);
	struct xfs_mount	*mp = ip->i_mount;
	xfs_fileoff_t		offset_fsb, end_fsb;
	int			error = 0;
	int			lockmode = 0;
	struct xfs_bmbt_irec	imap;
	int			nimaps = 1;
	xfs_off_t		offset;
	ssize_t			size;
	int			new = 0;

	if (XFS_FORCED_SHUTDOWN(mp))
		return -EIO;

	offset = (xfs_off_t)iblock << inode->i_blkbits;
	ASSERT(bh_result->b_size >= (1 << inode->i_blkbits));
	size = bh_result->b_size;

	if (!create && direct && offset >= i_size_read(inode))
		return 0;

	/*
	 * Direct I/O is usually done on preallocated files, so try getting
	 * a block mapping without an exclusive lock first.  For buffered
	 * writes we already have the exclusive iolock anyway, so avoiding
	 * a lock roundtrip here by taking the ilock exclusive from the
	 * beginning is a useful micro optimization.
	 */
	if (create && !direct) {
		lockmode = XFS_ILOCK_EXCL;
		xfs_ilock(ip, lockmode);
	} else {
		lockmode = xfs_ilock_data_map_shared(ip);
	}

	ASSERT(offset <= mp->m_super->s_maxbytes);
	if ((xfs_ufsize_t)offset + size > mp->m_super->s_maxbytes)
		size = mp->m_super->s_maxbytes - offset;
	end_fsb = XFS_B_TO_FSB(mp, (xfs_ufsize_t)offset + size);
	offset_fsb = XFS_B_TO_FSBT(mp, offset);

	error = xfs_bmapi_read(ip, offset_fsb, end_fsb - offset_fsb,
				&imap, &nimaps, XFS_BMAPI_ENTIRE);
	if (error)
		goto out_unlock;

	if (create &&
	    (!nimaps ||
	     (imap.br_startblock == HOLESTARTBLOCK ||
	      imap.br_startblock == DELAYSTARTBLOCK))) {
		if (direct || xfs_get_extsz_hint(ip)) {
			/*
			 * Drop the ilock in preparation for starting the block
			 * allocation transaction.  It will be retaken
			 * exclusively inside xfs_iomap_write_direct for the
			 * actual allocation.
			 */
			xfs_iunlock(ip, lockmode);
			error = xfs_iomap_write_direct(ip, offset, size,
						       &imap, nimaps);
			if (error)
				return error;
			new = 1;
		} else {
			/*
			 * Delalloc reservations do not require a transaction,
			 * we can go on without dropping the lock here. If we
			 * are allocating a new delalloc block, make sure that
			 * we set the new flag so that we mark the buffer new so
			 * that we know that it is newly allocated if the write
			 * fails.
			 */
			if (nimaps && imap.br_startblock == HOLESTARTBLOCK)
				new = 1;
			error = xfs_iomap_write_delay(ip, offset, size, &imap);
			if (error)
				goto out_unlock;

			xfs_iunlock(ip, lockmode);
		}
		trace_xfs_get_blocks_alloc(ip, offset, size,
				ISUNWRITTEN(&imap) ? XFS_IO_UNWRITTEN
						   : XFS_IO_DELALLOC, &imap);
	} else if (nimaps) {
		trace_xfs_get_blocks_found(ip, offset, size,
				ISUNWRITTEN(&imap) ? XFS_IO_UNWRITTEN
						   : XFS_IO_OVERWRITE, &imap);
		xfs_iunlock(ip, lockmode);
	} else {
		trace_xfs_get_blocks_notfound(ip, offset, size);
		goto out_unlock;
	}

	/* trim mapping down to size requested */
	if (direct || size > (1 << inode->i_blkbits))
		xfs_map_trim_size(inode, iblock, bh_result,
				  &imap, offset, size);

	/*
	 * For unwritten extents do not report a disk address in the buffered
	 * read case (treat as if we're reading into a hole).
	 */
	if (imap.br_startblock != HOLESTARTBLOCK &&
	    imap.br_startblock != DELAYSTARTBLOCK &&
	    (create || !ISUNWRITTEN(&imap))) {
		xfs_map_buffer(inode, bh_result, &imap, offset);
		if (ISUNWRITTEN(&imap))
			set_buffer_unwritten(bh_result);
		/* direct IO needs special help */
		if (create && direct)
			xfs_map_direct(inode, bh_result, &imap, offset);
	}

	/*
	 * If this is a realtime file, data may be on a different device.
	 * to that pointed to from the buffer_head b_bdev currently.
	 */
	bh_result->b_bdev = xfs_find_bdev_for_inode(inode);

	/*
	 * If we previously allocated a block out beyond eof and we are now
	 * coming back to use it then we will need to flag it as new even if it
	 * has a disk address.
	 *
	 * With sub-block writes into unwritten extents we also need to mark
	 * the buffer as new so that the unwritten parts of the buffer gets
	 * correctly zeroed.
	 */
	if (create &&
	    ((!buffer_mapped(bh_result) && !buffer_uptodate(bh_result)) ||
	     (offset >= i_size_read(inode)) ||
	     (new || ISUNWRITTEN(&imap))))
		set_buffer_new(bh_result);

	if (imap.br_startblock == DELAYSTARTBLOCK) {
		BUG_ON(direct);
		if (create) {
			set_buffer_uptodate(bh_result);
			set_buffer_mapped(bh_result);
			set_buffer_delay(bh_result);
		}
	}

	return 0;

out_unlock:
	xfs_iunlock(ip, lockmode);
	return error;
}

int
xfs_get_blocks(
	struct inode		*inode,
	sector_t		iblock,
	struct buffer_head	*bh_result,
	int			create)
{
	return __xfs_get_blocks(inode, iblock, bh_result, create, 0);
}

STATIC int
xfs_get_blocks_direct(
	struct inode		*inode,
	sector_t		iblock,
	struct buffer_head	*bh_result,
	int			create)
{
	return __xfs_get_blocks(inode, iblock, bh_result, create, 1);
}

/*
 * Complete a direct I/O write request.
 *
 * The ioend structure is passed from __xfs_get_blocks() to tell us what to do.
 * If no ioend exists (i.e. @private == NULL) then the write IO is an overwrite
 * wholly within the EOF and so there is nothing for us to do. Note that in this
 * case the completion can be called in interrupt context, whereas if we have an
 * ioend we will always be called in task context (i.e. from a workqueue).
 */
STATIC void
xfs_end_io_direct_write(
	struct kiocb		*iocb,
	loff_t			offset,
	ssize_t			size,
	void			*private)
{
	struct inode		*inode = file_inode(iocb->ki_filp);
	struct xfs_inode	*ip = XFS_I(inode);
	struct xfs_mount	*mp = ip->i_mount;
	struct xfs_ioend	*ioend = private;

	trace_xfs_gbmap_direct_endio(ip, offset, size,
				     ioend ? ioend->io_type : 0, NULL);

	if (!ioend) {
		ASSERT(offset + size <= i_size_read(inode));
		return;
	}

	if (XFS_FORCED_SHUTDOWN(mp))
		goto out_end_io;

	/*
	 * dio completion end_io functions are only called on writes if more
	 * than 0 bytes was written.
	 */
	ASSERT(size > 0);

	/*
	 * The ioend only maps whole blocks, while the IO may be sector aligned.
	 * Hence the ioend offset/size may not match the IO offset/size exactly.
	 * Because we don't map overwrites within EOF into the ioend, the offset
	 * may not match, but only if the endio spans EOF.  Either way, write
	 * the IO sizes into the ioend so that completion processing does the
	 * right thing.
	 */
	ASSERT(offset + size <= ioend->io_offset + ioend->io_size);
	ioend->io_size = size;
	ioend->io_offset = offset;

	/*
	 * The ioend tells us whether we are doing unwritten extent conversion
	 * or an append transaction that updates the on-disk file size. These
	 * cases are the only cases where we should *potentially* be needing
	 * to update the VFS inode size.
	 *
	 * We need to update the in-core inode size here so that we don't end up
	 * with the on-disk inode size being outside the in-core inode size. We
	 * have no other method of updating EOF for AIO, so always do it here
	 * if necessary.
	 *
	 * We need to lock the test/set EOF update as we can be racing with
	 * other IO completions here to update the EOF. Failing to serialise
	 * here can result in EOF moving backwards and Bad Things Happen when
	 * that occurs.
	 */
	spin_lock(&ip->i_flags_lock);
	if (offset + size > i_size_read(inode))
		i_size_write(inode, offset + size);
	spin_unlock(&ip->i_flags_lock);

	/*
	 * If we are doing an append IO that needs to update the EOF on disk,
	 * do the transaction reserve now so we can use common end io
	 * processing. Stashing the error (if there is one) in the ioend will
	 * result in the ioend processing passing on the error if it is
	 * possible as we can't return it from here.
	 */
	if (ioend->io_type == XFS_IO_OVERWRITE)
		ioend->io_error = xfs_setfilesize_trans_alloc(ioend);

out_end_io:
	xfs_end_io(&ioend->io_work);
	return;
}

STATIC ssize_t
xfs_vm_direct_IO(
	struct kiocb		*iocb,
	struct iov_iter		*iter,
	loff_t			offset)
{
	struct inode		*inode = iocb->ki_filp->f_mapping->host;
	struct block_device	*bdev = xfs_find_bdev_for_inode(inode);

	if (iov_iter_rw(iter) == WRITE) {
		return __blockdev_direct_IO(iocb, inode, bdev, iter, offset,
					    xfs_get_blocks_direct,
					    xfs_end_io_direct_write, NULL,
					    DIO_ASYNC_EXTEND);
	}
	return __blockdev_direct_IO(iocb, inode, bdev, iter, offset,
				    xfs_get_blocks_direct, NULL, NULL, 0);
}

/*
 * Punch out the delalloc blocks we have already allocated.
 *
 * Don't bother with xfs_setattr given that nothing can have made it to disk yet
 * as the page is still locked at this point.
 */
STATIC void
xfs_vm_kill_delalloc_range(
	struct inode		*inode,
	loff_t			start,
	loff_t			end)
{
	struct xfs_inode	*ip = XFS_I(inode);
	xfs_fileoff_t		start_fsb;
	xfs_fileoff_t		end_fsb;
	int			error;

	start_fsb = XFS_B_TO_FSB(ip->i_mount, start);
	end_fsb = XFS_B_TO_FSB(ip->i_mount, end);
	if (end_fsb <= start_fsb)
		return;

	xfs_ilock(ip, XFS_ILOCK_EXCL);
	error = xfs_bmap_punch_delalloc_range(ip, start_fsb,
						end_fsb - start_fsb);
	if (error) {
		/* something screwed, just bail */
		if (!XFS_FORCED_SHUTDOWN(ip->i_mount)) {
			xfs_alert(ip->i_mount,
		"xfs_vm_write_failed: unable to clean up ino %lld",
					ip->i_ino);
		}
	}
	xfs_iunlock(ip, XFS_ILOCK_EXCL);
}

STATIC void
xfs_vm_write_failed(
	struct inode		*inode,
	struct page		*page,
	loff_t			pos,
	unsigned		len)
{
	loff_t			block_offset;
	loff_t			block_start;
	loff_t			block_end;
	loff_t			from = pos & (PAGE_CACHE_SIZE - 1);
	loff_t			to = from + len;
	struct buffer_head	*bh, *head;

	/*
	 * The request pos offset might be 32 or 64 bit, this is all fine
	 * on 64-bit platform.  However, for 64-bit pos request on 32-bit
	 * platform, the high 32-bit will be masked off if we evaluate the
	 * block_offset via (pos & PAGE_MASK) because the PAGE_MASK is
	 * 0xfffff000 as an unsigned long, hence the result is incorrect
	 * which could cause the following ASSERT failed in most cases.
	 * In order to avoid this, we can evaluate the block_offset of the
	 * start of the page by using shifts rather than masks the mismatch
	 * problem.
	 */
	block_offset = (pos >> PAGE_CACHE_SHIFT) << PAGE_CACHE_SHIFT;

	ASSERT(block_offset + from == pos);

	head = page_buffers(page);
	block_start = 0;
	for (bh = head; bh != head || !block_start;
	     bh = bh->b_this_page, block_start = block_end,
				   block_offset += bh->b_size) {
		block_end = block_start + bh->b_size;

		/* skip buffers before the write */
		if (block_end <= from)
			continue;

		/* if the buffer is after the write, we're done */
		if (block_start >= to)
			break;

		if (!buffer_delay(bh))
			continue;

		if (!buffer_new(bh) && block_offset < i_size_read(inode))
			continue;

		xfs_vm_kill_delalloc_range(inode, block_offset,
					   block_offset + bh->b_size);

		/*
		 * This buffer does not contain data anymore. make sure anyone
		 * who finds it knows that for certain.
		 */
		clear_buffer_delay(bh);
		clear_buffer_uptodate(bh);
		clear_buffer_mapped(bh);
		clear_buffer_new(bh);
		clear_buffer_dirty(bh);
	}

}

/*
 * This used to call block_write_begin(), but it unlocks and releases the page
 * on error, and we need that page to be able to punch stale delalloc blocks out
 * on failure. hence we copy-n-waste it here and call xfs_vm_write_failed() at
 * the appropriate point.
 */
STATIC int
xfs_vm_write_begin(
	struct file		*file,
	struct address_space	*mapping,
	loff_t			pos,
	unsigned		len,
	unsigned		flags,
	struct page		**pagep,
	void			**fsdata)
{
	pgoff_t			index = pos >> PAGE_CACHE_SHIFT;
	struct page		*page;
	int			status;

	ASSERT(len <= PAGE_CACHE_SIZE);

	page = grab_cache_page_write_begin(mapping, index, flags);
	if (!page)
		return -ENOMEM;

	status = __block_write_begin(page, pos, len, xfs_get_blocks);
	if (unlikely(status)) {
		struct inode	*inode = mapping->host;
		size_t		isize = i_size_read(inode);

		xfs_vm_write_failed(inode, page, pos, len);
		unlock_page(page);

		/*
		 * If the write is beyond EOF, we only want to kill blocks
		 * allocated in this write, not blocks that were previously
		 * written successfully.
		 */
		if (pos + len > isize) {
			ssize_t start = max_t(ssize_t, pos, isize);

			truncate_pagecache_range(inode, start, pos + len);
		}

		page_cache_release(page);
		page = NULL;
	}

	*pagep = page;
	return status;
}

/*
 * On failure, we only need to kill delalloc blocks beyond EOF in the range of
 * this specific write because they will never be written. Previous writes
 * beyond EOF where block allocation succeeded do not need to be trashed, so
 * only new blocks from this write should be trashed. For blocks within
 * EOF, generic_write_end() zeros them so they are safe to leave alone and be
 * written with all the other valid data.
 */
STATIC int
xfs_vm_write_end(
	struct file		*file,
	struct address_space	*mapping,
	loff_t			pos,
	unsigned		len,
	unsigned		copied,
	struct page		*page,
	void			*fsdata)
{
	int			ret;

	ASSERT(len <= PAGE_CACHE_SIZE);

	ret = generic_write_end(file, mapping, pos, len, copied, page, fsdata);
	if (unlikely(ret < len)) {
		struct inode	*inode = mapping->host;
		size_t		isize = i_size_read(inode);
		loff_t		to = pos + len;

		if (to > isize) {
			/* only kill blocks in this write beyond EOF */
			if (pos > isize)
				isize = pos;
			xfs_vm_kill_delalloc_range(inode, isize, to);
			truncate_pagecache_range(inode, isize, to);
		}
	}
	return ret;
}

STATIC sector_t
xfs_vm_bmap(
	struct address_space	*mapping,
	sector_t		block)
{
	struct inode		*inode = (struct inode *)mapping->host;
	struct xfs_inode	*ip = XFS_I(inode);

	trace_xfs_vm_bmap(XFS_I(inode));
	xfs_ilock(ip, XFS_IOLOCK_SHARED);
	filemap_write_and_wait(mapping);
	xfs_iunlock(ip, XFS_IOLOCK_SHARED);
	return generic_block_bmap(mapping, block, xfs_get_blocks);
}

STATIC int
xfs_vm_readpage(
	struct file		*unused,
	struct page		*page)
{
	return mpage_readpage(page, xfs_get_blocks);
}

STATIC int
xfs_vm_readpages(
	struct file		*unused,
	struct address_space	*mapping,
	struct list_head	*pages,
	unsigned		nr_pages)
{
	return mpage_readpages(mapping, pages, nr_pages, xfs_get_blocks);
}

/*
 * This is basically a copy of __set_page_dirty_buffers() with one
 * small tweak: buffers beyond EOF do not get marked dirty. If we mark them
 * dirty, we'll never be able to clean them because we don't write buffers
 * beyond EOF, and that means we can't invalidate pages that span EOF
 * that have been marked dirty. Further, the dirty state can leak into
 * the file interior if the file is extended, resulting in all sorts of
 * bad things happening as the state does not match the underlying data.
 *
 * XXX: this really indicates that bufferheads in XFS need to die. Warts like
 * this only exist because of bufferheads and how the generic code manages them.
 */
STATIC int
xfs_vm_set_page_dirty(
	struct page		*page)
{
	struct address_space	*mapping = page->mapping;
	struct inode		*inode = mapping->host;
	loff_t			end_offset;
	loff_t			offset;
	int			newly_dirty;

	if (unlikely(!mapping))
		return !TestSetPageDirty(page);

	end_offset = i_size_read(inode);
	offset = page_offset(page);

	spin_lock(&mapping->private_lock);
	if (page_has_buffers(page)) {
		struct buffer_head *head = page_buffers(page);
		struct buffer_head *bh = head;

		do {
			if (offset < end_offset)
				set_buffer_dirty(bh);
			bh = bh->b_this_page;
			offset += 1 << inode->i_blkbits;
		} while (bh != head);
	}
	newly_dirty = !TestSetPageDirty(page);
	spin_unlock(&mapping->private_lock);

	if (newly_dirty) {
		/* sigh - __set_page_dirty() is static, so copy it here, too */
		unsigned long flags;

		spin_lock_irqsave(&mapping->tree_lock, flags);
		if (page->mapping) {	/* Race with truncate? */
			WARN_ON_ONCE(!PageUptodate(page));
			account_page_dirtied(page, mapping);
			radix_tree_tag_set(&mapping->page_tree,
					page_index(page), PAGECACHE_TAG_DIRTY);
		}
		spin_unlock_irqrestore(&mapping->tree_lock, flags);
		__mark_inode_dirty(mapping->host, I_DIRTY_PAGES);
	}
	return newly_dirty;
}

const struct address_space_operations xfs_address_space_operations = {
	.readpage		= xfs_vm_readpage,
	.readpages		= xfs_vm_readpages,
	.writepage		= xfs_vm_writepage,
	.writepages		= xfs_vm_writepages,
	.set_page_dirty		= xfs_vm_set_page_dirty,
	.releasepage		= xfs_vm_releasepage,
	.invalidatepage		= xfs_vm_invalidatepage,
	.write_begin		= xfs_vm_write_begin,
	.write_end		= xfs_vm_write_end,
	.bmap			= xfs_vm_bmap,
	.direct_IO		= xfs_vm_direct_IO,
	.migratepage		= buffer_migrate_page,
	.is_partially_uptodate  = block_is_partially_uptodate,
	.error_remove_page	= generic_error_remove_page,
};
