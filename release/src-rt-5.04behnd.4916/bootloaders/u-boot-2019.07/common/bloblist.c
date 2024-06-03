// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <bloblist.h>
#include <log.h>
#include <mapmem.h>
#include <spl.h>

DECLARE_GLOBAL_DATA_PTR;

struct bloblist_rec *bloblist_first_blob(struct bloblist_hdr *hdr)
{
	if (hdr->alloced <= hdr->hdr_size)
		return NULL;
	return (struct bloblist_rec *)((void *)hdr + hdr->hdr_size);
}

struct bloblist_rec *bloblist_next_blob(struct bloblist_hdr *hdr,
					struct bloblist_rec *rec)
{
	ulong offset;

	offset = (void *)rec - (void *)hdr;
	offset += rec->hdr_size + ALIGN(rec->size, BLOBLIST_ALIGN);
	if (offset >= hdr->alloced)
		return NULL;
	return (struct bloblist_rec *)((void *)hdr + offset);
}

#define foreach_rec(_rec, _hdr) \
	for (_rec = bloblist_first_blob(_hdr); \
	     _rec; \
	     _rec = bloblist_next_blob(_hdr, _rec))

static struct bloblist_rec *bloblist_findrec(uint tag)
{
	struct bloblist_hdr *hdr = gd->bloblist;
	struct bloblist_rec *rec;

	if (!hdr)
		return NULL;

	foreach_rec(rec, hdr) {
		if (rec->tag == tag)
			return rec;
	}

	return NULL;
}

static int bloblist_addrec(uint tag, int size, struct bloblist_rec **recp)
{
	struct bloblist_hdr *hdr = gd->bloblist;
	struct bloblist_rec *rec;
	int new_alloced;

	new_alloced = hdr->alloced + sizeof(*rec) +
			ALIGN(size, BLOBLIST_ALIGN);
	if (new_alloced >= hdr->size) {
		log(LOGC_BLOBLIST, LOGL_ERR,
		    "Failed to allocate %x bytes size=%x, need size>=%x\n",
		    size, hdr->size, new_alloced);
		return log_msg_ret("bloblist add", -ENOSPC);
	}
	rec = (void *)hdr + hdr->alloced;
	hdr->alloced = new_alloced;

	rec->tag = tag;
	rec->hdr_size = sizeof(*rec);
	rec->size = size;
	rec->spare = 0;
	*recp = rec;

	return 0;
}

static int bloblist_ensurerec(uint tag, struct bloblist_rec **recp, int size)
{
	struct bloblist_rec *rec;

	rec = bloblist_findrec(tag);
	if (rec) {
		if (size && size != rec->size)
			return -ESPIPE;
	} else {
		int ret;

		ret = bloblist_addrec(tag, size, &rec);
		if (ret)
			return ret;
	}
	*recp = rec;

	return 0;
}

void *bloblist_find(uint tag, int size)
{
	struct bloblist_rec *rec;

	rec = bloblist_findrec(tag);
	if (!rec)
		return NULL;
	if (size && size != rec->size)
		return NULL;

	return (void *)rec + rec->hdr_size;
}

void *bloblist_add(uint tag, int size)
{
	struct bloblist_rec *rec;

	if (bloblist_addrec(tag, size, &rec))
		return NULL;

	return rec + 1;
}

int bloblist_ensure_size(uint tag, int size, void **blobp)
{
	struct bloblist_rec *rec;
	int ret;

	ret = bloblist_ensurerec(tag, &rec, size);
	if (ret)
		return ret;
	*blobp = (void *)rec + rec->hdr_size;

	return 0;
}

void *bloblist_ensure(uint tag, int size)
{
	struct bloblist_rec *rec;

	if (bloblist_ensurerec(tag, &rec, size))
		return NULL;

	return (void *)rec + rec->hdr_size;
}

static u32 bloblist_calc_chksum(struct bloblist_hdr *hdr)
{
	struct bloblist_rec *rec;
	u32 chksum;

	chksum = crc32(0, (unsigned char *)hdr,
		       offsetof(struct bloblist_hdr, chksum));
	foreach_rec(rec, hdr) {
		chksum = crc32(chksum, (void *)rec, rec->hdr_size);
		chksum = crc32(chksum, (void *)rec + rec->hdr_size, rec->size);
	}

	return chksum;
}

int bloblist_new(ulong addr, uint size, uint flags)
{
	struct bloblist_hdr *hdr;

	if (size < sizeof(*hdr))
		return log_ret(-ENOSPC);
	if (addr & (BLOBLIST_ALIGN - 1))
		return log_ret(-EFAULT);
	hdr = map_sysmem(addr, size);
	memset(hdr, '\0', sizeof(*hdr));
	hdr->version = BLOBLIST_VERSION;
	hdr->hdr_size = sizeof(*hdr);
	hdr->flags = flags;
	hdr->magic = BLOBLIST_MAGIC;
	hdr->size = size;
	hdr->alloced = hdr->hdr_size;
	hdr->chksum = 0;
	gd->bloblist = hdr;

	return 0;
}

int bloblist_check(ulong addr, uint size)
{
	struct bloblist_hdr *hdr;
	u32 chksum;

	hdr = map_sysmem(addr, sizeof(*hdr));
	if (hdr->magic != BLOBLIST_MAGIC)
		return log_msg_ret("Bad magic", -ENOENT);
	if (hdr->version != BLOBLIST_VERSION)
		return log_msg_ret("Bad version", -EPROTONOSUPPORT);
	if (size && hdr->size != size)
		return log_msg_ret("Bad size", -EFBIG);
	chksum = bloblist_calc_chksum(hdr);
	if (hdr->chksum != chksum) {
		log(LOGC_BLOBLIST, LOGL_ERR, "Checksum %x != %x\n", hdr->chksum,
		    chksum);
		return log_msg_ret("Bad checksum", -EIO);
	}
	gd->bloblist = hdr;

	return 0;
}

int bloblist_finish(void)
{
	struct bloblist_hdr *hdr = gd->bloblist;

	hdr->chksum = bloblist_calc_chksum(hdr);

	return 0;
}

int bloblist_init(void)
{
	bool expected;
	int ret = -ENOENT;

	/**
	 * Wed expect to find an existing bloblist in the first phase of U-Boot
	 * that runs
	 */
	expected = !u_boot_first_phase();
	if (expected)
		ret = bloblist_check(CONFIG_BLOBLIST_ADDR,
				     CONFIG_BLOBLIST_SIZE);
	if (ret) {
		log(LOGC_BLOBLIST, expected ? LOGL_WARNING : LOGL_DEBUG,
		    "Existing bloblist not found: creating new bloblist\n");
		ret = bloblist_new(CONFIG_BLOBLIST_ADDR, CONFIG_BLOBLIST_SIZE,
				   0);
	} else {
		log(LOGC_BLOBLIST, LOGL_DEBUG, "Found existing bloblist\n");
	}

	return ret;
}
