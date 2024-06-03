// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (c) Thomas Gleixner <tglx@linutronix.de>
 *
 * The parts taken from the kernel implementation are:
 *
 * Copyright (c) International Business Machines Corp., 2006
 */

#include <common.h>
#include <errno.h>
#include <ubispl.h>

#include <linux/crc32.h>

#include "ubispl.h"

/**
 * ubi_calc_fm_size - calculates the fastmap size in bytes for an UBI device.
 * @ubi: UBI device description object
 */
static size_t ubi_calc_fm_size(struct ubi_scan_info *ubi)
{
	size_t size;

	size = sizeof(struct ubi_fm_sb) +
		sizeof(struct ubi_fm_hdr) +
		sizeof(struct ubi_fm_scan_pool) +
		sizeof(struct ubi_fm_scan_pool) +
		(ubi->peb_count * sizeof(struct ubi_fm_ec)) +
		(sizeof(struct ubi_fm_eba) +
		(ubi->peb_count * sizeof(__be32))) +
		sizeof(struct ubi_fm_volhdr) * UBI_MAX_VOLUMES;
	return roundup(size, ubi->leb_size);
}

static int ubi_io_read(struct ubi_scan_info *ubi, void *buf, int pnum,
		       unsigned long from, unsigned long len)
{
	return ubi->read(pnum + ubi->peb_offset, from, len, buf);
}

static int ubi_io_is_bad(struct ubi_scan_info *ubi, int peb)
{
	return peb >= ubi->peb_count || peb < 0;
}

static int ubi_io_read_vid_hdr(struct ubi_scan_info *ubi, int pnum,
			       struct ubi_vid_hdr *vh, int unused)
{
	u32 magic;
	int res;

	/* No point in rescanning a corrupt block */
	if (test_bit(pnum, ubi->corrupt))
		return UBI_IO_BAD_HDR;
	/*
	 * If the block has been scanned already, no need to rescan
	 */
	if (test_and_set_bit(pnum, ubi->scanned))
		return 0;

	res = ubi_io_read(ubi, vh, pnum, ubi->vid_offset, sizeof(*vh));

	/*
	 * Bad block, unrecoverable ECC error, skip the block
	 */
	if (res) {
		ubi_dbg("Skipping bad or unreadable block %d", pnum);
		vh->magic = 0;
		generic_set_bit(pnum, ubi->corrupt);
		return res;
	}

	/* Magic number available ? */
	magic = be32_to_cpu(vh->magic);
	if (magic != UBI_VID_HDR_MAGIC) {
		generic_set_bit(pnum, ubi->corrupt);
		if (magic == 0xffffffff)
			return UBI_IO_FF;
		ubi_msg("Bad magic in block 0%d %08x", pnum, magic);
		return UBI_IO_BAD_HDR;
	}

	/* Header CRC correct ? */
	if (crc32(UBI_CRC32_INIT, vh, UBI_VID_HDR_SIZE_CRC) !=
	    be32_to_cpu(vh->hdr_crc)) {
		ubi_msg("Bad CRC in block 0%d", pnum);
		generic_set_bit(pnum, ubi->corrupt);
		return UBI_IO_BAD_HDR;
	}

	ubi_dbg("RV: pnum: %i sqnum %llu", pnum, be64_to_cpu(vh->sqnum));

	return 0;
}

static int ubi_rescan_fm_vid_hdr(struct ubi_scan_info *ubi,
				 struct ubi_vid_hdr *vh,
				 u32 fm_pnum, u32 fm_vol_id, u32 fm_lnum)
{
	int res;

	if (ubi_io_is_bad(ubi, fm_pnum))
		return -EINVAL;

	res = ubi_io_read_vid_hdr(ubi, fm_pnum, vh, 0);
	if (!res) {
		/* Check volume id, volume type and lnum */
		if (be32_to_cpu(vh->vol_id) == fm_vol_id &&
		    vh->vol_type == UBI_VID_STATIC &&
		    be32_to_cpu(vh->lnum) == fm_lnum)
			return 0;
		ubi_dbg("RS: PEB %u vol: %u : %u typ %u lnum %u %u",
			fm_pnum, fm_vol_id, vh->vol_type,
			be32_to_cpu(vh->vol_id),
			fm_lnum, be32_to_cpu(vh->lnum));
	}
	return res;
}

/* Insert the logic block into the volume info */
static int ubi_add_peb_to_vol(struct ubi_scan_info *ubi,
			      struct ubi_vid_hdr *vh, u32 vol_id,
			      u32 pnum, u32 lnum)
{
	struct ubi_vol_info *vi = ubi->volinfo + vol_id;
	u32 *ltp;

	/*
	 * If the volume is larger than expected, yell and give up :(
	 */
	if (lnum >= UBI_MAX_VOL_LEBS) {
		ubi_warn("Vol: %u LEB %d > %d", vol_id, lnum, UBI_MAX_VOL_LEBS);
		return -EINVAL;
	}

	ubi_dbg("SC: Add PEB %u to Vol %u as LEB %u fnd %d sc %d",
		pnum, vol_id, lnum, !!test_bit(lnum, vi->found),
		!!test_bit(pnum, ubi->scanned));

	/* Points to the translation entry */
	ltp = vi->lebs_to_pebs + lnum;

	/* If the block is already assigned, check sqnum */
	if (__test_and_set_bit(lnum, vi->found)) {
		u32 cur_pnum = *ltp;
		struct ubi_vid_hdr *cur = ubi->blockinfo + cur_pnum;

		/*
		 * If the current block hase not yet been scanned, we
		 * need to do that. The other block might be stale or
		 * the current block corrupted and the FM not yet
		 * updated.
		 */
		if (!test_bit(cur_pnum, ubi->scanned)) {
			/*
			 * If the scan fails, we use the valid block
			 */
			if (ubi_rescan_fm_vid_hdr(ubi, cur, cur_pnum, vol_id,
						  lnum)) {
				*ltp = pnum;
				return 0;
			}
		}

		/*
		 * Should not happen ....
		 */
		if (test_bit(cur_pnum, ubi->corrupt)) {
			*ltp = pnum;
			return 0;
		}

		ubi_dbg("Vol %u LEB %u PEB %u->sqnum %llu NPEB %u->sqnum %llu",
			vol_id, lnum, cur_pnum, be64_to_cpu(cur->sqnum), pnum,
			be64_to_cpu(vh->sqnum));

		/*
		 * Compare sqnum and take the newer one
		 */
		if (be64_to_cpu(cur->sqnum) < be64_to_cpu(vh->sqnum))
			*ltp = pnum;
	} else {
		*ltp = pnum;
		if (lnum > vi->last_block)
			vi->last_block = lnum;
	}

	return 0;
}

static int ubi_scan_vid_hdr(struct ubi_scan_info *ubi, struct ubi_vid_hdr *vh,
			    u32 pnum)
{
	u32 vol_id, lnum;
	int res;

	if (ubi_io_is_bad(ubi, pnum))
		return -EINVAL;

	res = ubi_io_read_vid_hdr(ubi, pnum, vh, 0);
	if (res)
		return res;

	/* Get volume id */
	vol_id = be32_to_cpu(vh->vol_id);

	/* If this is the fastmap anchor, return right away */
	if (vol_id == UBI_FM_SB_VOLUME_ID)
		return ubi->fm_enabled ? UBI_FASTMAP_ANCHOR : 0;

	/* We only care about static volumes with an id < UBI_SPL_VOL_IDS */
	if (vol_id >= UBI_SPL_VOL_IDS || vh->vol_type != UBI_VID_STATIC)
		return 0;

	/* We are only interested in the volumes to load */
	if (!test_bit(vol_id, ubi->toload))
		return 0;

	lnum = be32_to_cpu(vh->lnum);
	return ubi_add_peb_to_vol(ubi, vh, vol_id, pnum, lnum);
}

static int assign_aeb_to_av(struct ubi_scan_info *ubi, u32 pnum, u32 lnum,
			     u32 vol_id, u32 vol_type, u32 used)
{
	struct ubi_vid_hdr *vh;

	if (ubi_io_is_bad(ubi, pnum))
		return -EINVAL;

	ubi->fastmap_pebs++;

	if (vol_id >= UBI_SPL_VOL_IDS || vol_type != UBI_STATIC_VOLUME)
		return 0;

	/* We are only interested in the volumes to load */
	if (!test_bit(vol_id, ubi->toload))
		return 0;

	vh = ubi->blockinfo + pnum;

	return ubi_scan_vid_hdr(ubi, vh, pnum);
}

static int scan_pool(struct ubi_scan_info *ubi, __be32 *pebs, int pool_size)
{
	struct ubi_vid_hdr *vh;
	u32 pnum;
	int i;

	ubi_dbg("Scanning pool size: %d", pool_size);

	for (i = 0; i < pool_size; i++) {
		pnum = be32_to_cpu(pebs[i]);

		if (ubi_io_is_bad(ubi, pnum)) {
			ubi_err("FM: Bad PEB in fastmap pool! %u", pnum);
			return UBI_BAD_FASTMAP;
		}

		vh = ubi->blockinfo + pnum;
		/*
		 * We allow the scan to fail here. The loader will notice
		 * and look for a replacement.
		 */
		ubi_scan_vid_hdr(ubi, vh, pnum);
	}
	return 0;
}

/*
 * Fastmap code is stolen from Linux kernel and this stub structure is used
 * to make it happy.
 */
struct ubi_attach_info {
	int i;
};

static int ubi_attach_fastmap(struct ubi_scan_info *ubi,
			      struct ubi_attach_info *ai,
			      struct ubi_fastmap_layout *fm)
{
	struct ubi_fm_hdr *fmhdr;
	struct ubi_fm_scan_pool *fmpl1, *fmpl2;
	struct ubi_fm_ec *fmec;
	struct ubi_fm_volhdr *fmvhdr;
	struct ubi_fm_eba *fm_eba;
	int ret, i, j, pool_size, wl_pool_size;
	size_t fm_pos = 0, fm_size = ubi->fm_size;
	void *fm_raw = ubi->fm_buf;

	memset(ubi->fm_used, 0, sizeof(ubi->fm_used));

	fm_pos += sizeof(struct ubi_fm_sb);
	if (fm_pos >= fm_size)
		goto fail_bad;

	fmhdr = (struct ubi_fm_hdr *)(fm_raw + fm_pos);
	fm_pos += sizeof(*fmhdr);
	if (fm_pos >= fm_size)
		goto fail_bad;

	if (be32_to_cpu(fmhdr->magic) != UBI_FM_HDR_MAGIC) {
		ubi_err("bad fastmap header magic: 0x%x, expected: 0x%x",
			be32_to_cpu(fmhdr->magic), UBI_FM_HDR_MAGIC);
		goto fail_bad;
	}

	fmpl1 = (struct ubi_fm_scan_pool *)(fm_raw + fm_pos);
	fm_pos += sizeof(*fmpl1);
	if (fm_pos >= fm_size)
		goto fail_bad;
	if (be32_to_cpu(fmpl1->magic) != UBI_FM_POOL_MAGIC) {
		ubi_err("bad fastmap pool magic: 0x%x, expected: 0x%x",
			be32_to_cpu(fmpl1->magic), UBI_FM_POOL_MAGIC);
		goto fail_bad;
	}

	fmpl2 = (struct ubi_fm_scan_pool *)(fm_raw + fm_pos);
	fm_pos += sizeof(*fmpl2);
	if (fm_pos >= fm_size)
		goto fail_bad;
	if (be32_to_cpu(fmpl2->magic) != UBI_FM_POOL_MAGIC) {
		ubi_err("bad fastmap pool magic: 0x%x, expected: 0x%x",
			be32_to_cpu(fmpl2->magic), UBI_FM_POOL_MAGIC);
		goto fail_bad;
	}

	pool_size = be16_to_cpu(fmpl1->size);
	wl_pool_size = be16_to_cpu(fmpl2->size);
	fm->max_pool_size = be16_to_cpu(fmpl1->max_size);
	fm->max_wl_pool_size = be16_to_cpu(fmpl2->max_size);

	if (pool_size > UBI_FM_MAX_POOL_SIZE || pool_size < 0) {
		ubi_err("bad pool size: %i", pool_size);
		goto fail_bad;
	}

	if (wl_pool_size > UBI_FM_MAX_POOL_SIZE || wl_pool_size < 0) {
		ubi_err("bad WL pool size: %i", wl_pool_size);
		goto fail_bad;
	}

	if (fm->max_pool_size > UBI_FM_MAX_POOL_SIZE ||
	    fm->max_pool_size < 0) {
		ubi_err("bad maximal pool size: %i", fm->max_pool_size);
		goto fail_bad;
	}

	if (fm->max_wl_pool_size > UBI_FM_MAX_POOL_SIZE ||
	    fm->max_wl_pool_size < 0) {
		ubi_err("bad maximal WL pool size: %i", fm->max_wl_pool_size);
		goto fail_bad;
	}

	/* read EC values from free list */
	for (i = 0; i < be32_to_cpu(fmhdr->free_peb_count); i++) {
		fmec = (struct ubi_fm_ec *)(fm_raw + fm_pos);
		fm_pos += sizeof(*fmec);
		if (fm_pos >= fm_size)
			goto fail_bad;
	}

	/* read EC values from used list */
	for (i = 0; i < be32_to_cpu(fmhdr->used_peb_count); i++) {
		fmec = (struct ubi_fm_ec *)(fm_raw + fm_pos);
		fm_pos += sizeof(*fmec);
		if (fm_pos >= fm_size)
			goto fail_bad;

		generic_set_bit(be32_to_cpu(fmec->pnum), ubi->fm_used);
	}

	/* read EC values from scrub list */
	for (i = 0; i < be32_to_cpu(fmhdr->scrub_peb_count); i++) {
		fmec = (struct ubi_fm_ec *)(fm_raw + fm_pos);
		fm_pos += sizeof(*fmec);
		if (fm_pos >= fm_size)
			goto fail_bad;
	}

	/* read EC values from erase list */
	for (i = 0; i < be32_to_cpu(fmhdr->erase_peb_count); i++) {
		fmec = (struct ubi_fm_ec *)(fm_raw + fm_pos);
		fm_pos += sizeof(*fmec);
		if (fm_pos >= fm_size)
			goto fail_bad;
	}

	/* Iterate over all volumes and read their EBA table */
	for (i = 0; i < be32_to_cpu(fmhdr->vol_count); i++) {
		u32 vol_id, vol_type, used, reserved;

		fmvhdr = (struct ubi_fm_volhdr *)(fm_raw + fm_pos);
		fm_pos += sizeof(*fmvhdr);
		if (fm_pos >= fm_size)
			goto fail_bad;

		if (be32_to_cpu(fmvhdr->magic) != UBI_FM_VHDR_MAGIC) {
			ubi_err("bad fastmap vol header magic: 0x%x, " \
				"expected: 0x%x",
				be32_to_cpu(fmvhdr->magic), UBI_FM_VHDR_MAGIC);
			goto fail_bad;
		}

		vol_id = be32_to_cpu(fmvhdr->vol_id);
		vol_type = fmvhdr->vol_type;
		used = be32_to_cpu(fmvhdr->used_ebs);

		fm_eba = (struct ubi_fm_eba *)(fm_raw + fm_pos);
		fm_pos += sizeof(*fm_eba);
		fm_pos += (sizeof(__be32) * be32_to_cpu(fm_eba->reserved_pebs));
		if (fm_pos >= fm_size)
			goto fail_bad;

		if (be32_to_cpu(fm_eba->magic) != UBI_FM_EBA_MAGIC) {
			ubi_err("bad fastmap EBA header magic: 0x%x, " \
				"expected: 0x%x",
				be32_to_cpu(fm_eba->magic), UBI_FM_EBA_MAGIC);
			goto fail_bad;
		}

		reserved = be32_to_cpu(fm_eba->reserved_pebs);
		ubi_dbg("FA: vol %u used %u res: %u", vol_id, used, reserved);
		for (j = 0; j < reserved; j++) {
			int pnum = be32_to_cpu(fm_eba->pnum[j]);

			if ((int)be32_to_cpu(fm_eba->pnum[j]) < 0)
				continue;

			if (!__test_and_clear_bit(pnum, ubi->fm_used))
				continue;

			/*
			 * We only handle static volumes so used_ebs
			 * needs to be handed in. And we do not assign
			 * the reserved blocks
			 */
			if (j >= used)
				continue;

			ret = assign_aeb_to_av(ubi, pnum, j, vol_id,
					       vol_type, used);
			if (!ret)
				continue;

			/*
			 * Nasty: The fastmap claims that the volume
			 * has one block more than it, but that block
			 * is always empty and the other blocks have
			 * the correct number of total LEBs in the
			 * headers. Deal with it.
			 */
			if (ret != UBI_IO_FF && j != used - 1)
				goto fail_bad;
			ubi_dbg("FA: Vol: %u Ignoring empty LEB %d of %d",
				vol_id, j, used);
		}
	}

	ret = scan_pool(ubi, fmpl1->pebs, pool_size);
	if (ret)
		goto fail;

	ret = scan_pool(ubi, fmpl2->pebs, wl_pool_size);
	if (ret)
		goto fail;

#ifdef CHECKME
	/*
	 * If fastmap is leaking PEBs (must not happen), raise a
	 * fat warning and fall back to scanning mode.
	 * We do this here because in ubi_wl_init() it's too late
	 * and we cannot fall back to scanning.
	 */
	if (WARN_ON(count_fastmap_pebs(ai) != ubi->peb_count -
		    ai->bad_peb_count - fm->used_blocks))
		goto fail_bad;
#endif

	return 0;

fail_bad:
	ret = UBI_BAD_FASTMAP;
fail:
	return ret;
}

static int ubi_scan_fastmap(struct ubi_scan_info *ubi,
			    struct ubi_attach_info *ai,
			    int fm_anchor)
{
	struct ubi_fm_sb *fmsb, *fmsb2;
	struct ubi_vid_hdr *vh;
	struct ubi_fastmap_layout *fm;
	int i, used_blocks, pnum, ret = 0;
	size_t fm_size;
	__be32 crc, tmp_crc;
	unsigned long long sqnum = 0;

	fmsb = &ubi->fm_sb;
	fm = &ubi->fm_layout;

	ret = ubi_io_read(ubi, fmsb, fm_anchor, ubi->leb_start, sizeof(*fmsb));
	if (ret && ret != UBI_IO_BITFLIPS)
		goto free_fm_sb;
	else if (ret == UBI_IO_BITFLIPS)
		fm->to_be_tortured[0] = 1;

	if (be32_to_cpu(fmsb->magic) != UBI_FM_SB_MAGIC) {
		ubi_err("bad super block magic: 0x%x, expected: 0x%x",
			be32_to_cpu(fmsb->magic), UBI_FM_SB_MAGIC);
		ret = UBI_BAD_FASTMAP;
		goto free_fm_sb;
	}

	if (fmsb->version != UBI_FM_FMT_VERSION) {
		ubi_err("bad fastmap version: %i, expected: %i",
			fmsb->version, UBI_FM_FMT_VERSION);
		ret = UBI_BAD_FASTMAP;
		goto free_fm_sb;
	}

	used_blocks = be32_to_cpu(fmsb->used_blocks);
	if (used_blocks > UBI_FM_MAX_BLOCKS || used_blocks < 1) {
		ubi_err("number of fastmap blocks is invalid: %i", used_blocks);
		ret = UBI_BAD_FASTMAP;
		goto free_fm_sb;
	}

	fm_size = ubi->leb_size * used_blocks;
	if (fm_size != ubi->fm_size) {
		ubi_err("bad fastmap size: %zi, expected: %zi", fm_size,
			ubi->fm_size);
		ret = UBI_BAD_FASTMAP;
		goto free_fm_sb;
	}

	vh = &ubi->fm_vh;

	for (i = 0; i < used_blocks; i++) {
		pnum = be32_to_cpu(fmsb->block_loc[i]);

		if (ubi_io_is_bad(ubi, pnum)) {
			ret = UBI_BAD_FASTMAP;
			goto free_hdr;
		}

#ifdef LATER
		int image_seq;
		ret = ubi_io_read_ec_hdr(ubi, pnum, ech, 0);
		if (ret && ret != UBI_IO_BITFLIPS) {
			ubi_err("unable to read fastmap block# %i EC (PEB: %i)",
				i, pnum);
			if (ret > 0)
				ret = UBI_BAD_FASTMAP;
			goto free_hdr;
		} else if (ret == UBI_IO_BITFLIPS)
			fm->to_be_tortured[i] = 1;

		image_seq = be32_to_cpu(ech->image_seq);
		if (!ubi->image_seq)
			ubi->image_seq = image_seq;
		/*
		 * Older UBI implementations have image_seq set to zero, so
		 * we shouldn't fail if image_seq == 0.
		 */
		if (image_seq && (image_seq != ubi->image_seq)) {
			ubi_err("wrong image seq:%d instead of %d",
				be32_to_cpu(ech->image_seq), ubi->image_seq);
			ret = UBI_BAD_FASTMAP;
			goto free_hdr;
		}
#endif
		ret = ubi_io_read_vid_hdr(ubi, pnum, vh, 0);
		if (ret && ret != UBI_IO_BITFLIPS) {
			ubi_err("unable to read fastmap block# %i (PEB: %i)",
				i, pnum);
			goto free_hdr;
		}

		/*
		 * Mainline code rescans the anchor header. We've done
		 * that already so we merily copy it over.
		 */
		if (pnum == fm_anchor)
			memcpy(vh, ubi->blockinfo + pnum, sizeof(*fm));

		if (i == 0) {
			if (be32_to_cpu(vh->vol_id) != UBI_FM_SB_VOLUME_ID) {
				ubi_err("bad fastmap anchor vol_id: 0x%x," \
					" expected: 0x%x",
					be32_to_cpu(vh->vol_id),
					UBI_FM_SB_VOLUME_ID);
				ret = UBI_BAD_FASTMAP;
				goto free_hdr;
			}
		} else {
			if (be32_to_cpu(vh->vol_id) != UBI_FM_DATA_VOLUME_ID) {
				ubi_err("bad fastmap data vol_id: 0x%x," \
					" expected: 0x%x",
					be32_to_cpu(vh->vol_id),
					UBI_FM_DATA_VOLUME_ID);
				ret = UBI_BAD_FASTMAP;
				goto free_hdr;
			}
		}

		if (sqnum < be64_to_cpu(vh->sqnum))
			sqnum = be64_to_cpu(vh->sqnum);

		ret = ubi_io_read(ubi, ubi->fm_buf + (ubi->leb_size * i), pnum,
				  ubi->leb_start, ubi->leb_size);
		if (ret && ret != UBI_IO_BITFLIPS) {
			ubi_err("unable to read fastmap block# %i (PEB: %i, " \
				"err: %i)", i, pnum, ret);
			goto free_hdr;
		}
	}

	fmsb2 = (struct ubi_fm_sb *)(ubi->fm_buf);
	tmp_crc = be32_to_cpu(fmsb2->data_crc);
	fmsb2->data_crc = 0;
	crc = crc32(UBI_CRC32_INIT, ubi->fm_buf, fm_size);
	if (crc != tmp_crc) {
		ubi_err("fastmap data CRC is invalid");
		ubi_err("CRC should be: 0x%x, calc: 0x%x", tmp_crc, crc);
		ret = UBI_BAD_FASTMAP;
		goto free_hdr;
	}

	fmsb2->sqnum = sqnum;

	fm->used_blocks = used_blocks;

	ret = ubi_attach_fastmap(ubi, ai, fm);
	if (ret) {
		if (ret > 0)
			ret = UBI_BAD_FASTMAP;
		goto free_hdr;
	}

	ubi->fm = fm;
	ubi->fm_pool.max_size = ubi->fm->max_pool_size;
	ubi->fm_wl_pool.max_size = ubi->fm->max_wl_pool_size;
	ubi_msg("attached by fastmap %uMB %u blocks",
		ubi->fsize_mb, ubi->peb_count);
	ubi_dbg("fastmap pool size: %d", ubi->fm_pool.max_size);
	ubi_dbg("fastmap WL pool size: %d", ubi->fm_wl_pool.max_size);

out:
	if (ret)
		ubi_err("Attach by fastmap failed, doing a full scan!");
	return ret;

free_hdr:
free_fm_sb:
	goto out;
}

/*
 * Scan the flash and attempt to attach via fastmap
 */
static void ipl_scan(struct ubi_scan_info *ubi)
{
	unsigned int pnum;
	int res;

	/*
	 * Scan first for the fastmap super block
	 */
	for (pnum = 0; pnum < UBI_FM_MAX_START; pnum++) {
		res = ubi_scan_vid_hdr(ubi, ubi->blockinfo + pnum, pnum);
		/*
		 * We ignore errors here as we are meriliy scanning
		 * the headers.
		 */
		if (res != UBI_FASTMAP_ANCHOR)
			continue;

		/*
		 * If fastmap is disabled, continue scanning. This
		 * might happen because the previous attempt failed or
		 * the caller disabled it right away.
		 */
		if (!ubi->fm_enabled)
			continue;

		/*
		 * Try to attach the fastmap, if that fails continue
		 * scanning.
		 */
		if (!ubi_scan_fastmap(ubi, NULL, pnum))
			return;
		/*
		 * Fastmap failed. Clear everything we have and start
		 * over. We are paranoid and do not trust anything.
		 */
		memset(ubi->volinfo, 0, sizeof(ubi->volinfo));
		pnum = 0;
		break;
	}

	/*
	 * Continue scanning, ignore errors, we might find what we are
	 * looking for,
	 */
	for (; pnum < ubi->peb_count; pnum++)
		ubi_scan_vid_hdr(ubi, ubi->blockinfo + pnum, pnum);
}

/*
 * Load a logical block of a volume into memory
 */
static int ubi_load_block(struct ubi_scan_info *ubi, uint8_t *laddr,
			  struct ubi_vol_info *vi, u32 vol_id, u32 lnum,
			  u32 last)
{
	struct ubi_vid_hdr *vh, *vrepl;
	u32 pnum, crc, dlen;

retry:
	/*
	 * If this is a fastmap run, we try to rescan full, otherwise
	 * we simply give up.
	 */
	if (!test_bit(lnum, vi->found)) {
		ubi_warn("LEB %d of %d is missing", lnum, last);
		return -EINVAL;
	}

	pnum = vi->lebs_to_pebs[lnum];

	ubi_dbg("Load vol %u LEB %u PEB %u", vol_id, lnum, pnum);

	if (ubi_io_is_bad(ubi, pnum)) {
		ubi_warn("Corrupted mapping block %d PB %d\n", lnum, pnum);
		return -EINVAL;
	}

	if (test_bit(pnum, ubi->corrupt))
		goto find_other;

	/*
	 * Lets try to read that block
	 */
	vh = ubi->blockinfo + pnum;

	if (!test_bit(pnum, ubi->scanned)) {
		ubi_warn("Vol: %u LEB %u PEB %u not yet scanned", vol_id,
			 lnum, pnum);
		if (ubi_rescan_fm_vid_hdr(ubi, vh, pnum, vol_id, lnum))
			goto find_other;
	}

	/*
	 * Check, if the total number of blocks is correct
	 */
	if (be32_to_cpu(vh->used_ebs) != last) {
		ubi_dbg("Block count missmatch.");
		ubi_dbg("vh->used_ebs: %d nrblocks: %d",
			be32_to_cpu(vh->used_ebs), last);
		generic_set_bit(pnum, ubi->corrupt);
		goto find_other;
	}

	/*
	 * Get the data length of this block.
	 */
	dlen = be32_to_cpu(vh->data_size);

	/*
	 * Read the data into RAM. We ignore the return value
	 * here as the only thing which might go wrong are
	 * bitflips. Try nevertheless.
	 */
	ubi_io_read(ubi, laddr, pnum, ubi->leb_start, dlen);

	/* Calculate CRC over the data */
	crc = crc32(UBI_CRC32_INIT, laddr, dlen);

	if (crc != be32_to_cpu(vh->data_crc)) {
		ubi_warn("Vol: %u LEB %u PEB %u data CRC failure", vol_id,
			 lnum, pnum);
		generic_set_bit(pnum, ubi->corrupt);
		goto find_other;
	}

	/* We are good. Return the data length we read */
	return dlen;

find_other:
	ubi_dbg("Find replacement for LEB %u PEB %u", lnum, pnum);
	generic_clear_bit(lnum, vi->found);
	vrepl = NULL;

	for (pnum = 0; pnum < ubi->peb_count; pnum++) {
		struct ubi_vid_hdr *tmp = ubi->blockinfo + pnum;
		u32 t_vol_id = be32_to_cpu(tmp->vol_id);
		u32 t_lnum = be32_to_cpu(tmp->lnum);

		if (test_bit(pnum, ubi->corrupt))
			continue;

		if (t_vol_id != vol_id || t_lnum != lnum)
			continue;

		if (!test_bit(pnum, ubi->scanned)) {
			ubi_warn("Vol: %u LEB %u PEB %u not yet scanned",
				 vol_id, lnum, pnum);
			if (ubi_rescan_fm_vid_hdr(ubi, tmp, pnum, vol_id, lnum))
				continue;
		}

		/*
		 * We found one. If its the first, assign it otherwise
		 * compare the sqnum
		 */
		generic_set_bit(lnum, vi->found);

		if (!vrepl) {
			vrepl = tmp;
			continue;
		}

		if (be64_to_cpu(vrepl->sqnum) < be64_to_cpu(tmp->sqnum))
			vrepl = tmp;
	}

	if (vrepl) {
		/* Update the vi table */
		pnum = vrepl - ubi->blockinfo;
		vi->lebs_to_pebs[lnum] = pnum;
		ubi_dbg("Trying PEB %u for LEB %u", pnum, lnum);
		vh = vrepl;
	}
	goto retry;
}

/*
 * Load a volume into RAM
 */
static int ipl_load(struct ubi_scan_info *ubi, const u32 vol_id, uint8_t *laddr)
{
	struct ubi_vol_info *vi;
	u32 lnum, last, len;

	if (vol_id >= UBI_SPL_VOL_IDS)
		return -EINVAL;

	len = 0;
	vi = ubi->volinfo + vol_id;
	last = vi->last_block + 1;

	/* Read the blocks to RAM, check CRC */
	for (lnum = 0 ; lnum < last; lnum++) {
		int res = ubi_load_block(ubi, laddr, vi, vol_id, lnum, last);

		if (res < 0) {
			ubi_warn("Failed to load volume %u", vol_id);
			return res;
		}
		/* res is the data length of the read block */
		laddr += res;
		len += res;
	}
	return len;
}

int ubispl_load_volumes(struct ubispl_info *info, struct ubispl_load *lvols,
			int nrvols)
{
	struct ubi_scan_info *ubi = info->ubi;
	int res, i, fastmap = info->fastmap;
	u32 fsize;

retry:
	/*
	 * We do a partial initializiation of @ubi. Cleaning fm_buf is
	 * not necessary.
	 */
	memset(ubi, 0, offsetof(struct ubi_scan_info, fm_buf));

	ubi->read = info->read;

	/* Precalculate the offsets */
	ubi->vid_offset = info->vid_offset;
	ubi->leb_start = info->leb_start;
	ubi->leb_size = info->peb_size - ubi->leb_start;
	ubi->peb_count = info->peb_count;
	ubi->peb_offset = info->peb_offset;

	fsize = info->peb_size * info->peb_count;
	ubi->fsize_mb = fsize >> 20;

	/* Fastmap init */
	ubi->fm_size = ubi_calc_fm_size(ubi);
	ubi->fm_enabled = fastmap;

	for (i = 0; i < nrvols; i++) {
		struct ubispl_load *lv = lvols + i;

		generic_set_bit(lv->vol_id, ubi->toload);
	}

	ipl_scan(ubi);

	for (i = 0; i < nrvols; i++) {
		struct ubispl_load *lv = lvols + i;

		ubi_msg("Loading VolId #%d", lv->vol_id);
		res = ipl_load(ubi, lv->vol_id, lv->load_addr);
		if (res < 0) {
			if (fastmap) {
				fastmap = 0;
				goto retry;
			}
			ubi_warn("Failed");
			return res;
		}
	}
	return 0;
}
