/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019 Broadcom Ltd.
 */

/* define DEBUG before common.h to enable debug macro */
#define DEBUG
#include <common.h>
#include <environment.h>
#include <nand.h>
#include <u-boot/sha256.h>
#include "boot_blob.h"
#include "boot_flash.h"
#include "spl_env.h"

/* overlay entry table */
#include "generated/hashtable.h"

static int search_blob_in_hash(uint32_t magic, uint32_t sel, int *len,
			       uint8_t * sha256)
{
	struct overlays entry;
	int i = 0, offset = -1;

	do {
		entry = ovl[i];
		if (entry.ovltype == 0)
			break;

		if (entry.ovltype == magic && entry.selector == sel) {
			offset = entry.offset;
			memcpy(sha256, entry.sha, 32);
			*len = entry.size;
			break;
		}
		i++;

	} while (1);

	if (offset == -1)
		printf("blob not found for magic 0x%x sel 0x%x!\n", magic, sel);

	return offset;
}

static int validate_blob_sha256_digest(void *buf, int size, void *pdigest)
{
	uint8_t out[32];
	sha256_context ctx;
	sha256_starts(&ctx);
	sha256_update(&ctx, buf, size);
	sha256_finish(&ctx, out);
	return memcmp(pdigest, out, 32);
}

static struct magic_search_s {
	uint32_t index;
	uint32_t last;
	uint32_t length;
	uint32_t dpfe_fs; /* dpfe blob fast search */  
	uint32_t magics[BOOT_BLOB_MAX_MAGIC_NUMS][BOOT_BLOB_MAX_MAGIC_SEARCH + 1];
} magic_search;

static void init_find_magic(void)
{
	int col, i;
	uint32_t e;
	printf("FFinit ");
	if (magic_search.last != 0) {
		return;
	}
	i = 0;
	while ((e = ovl[i].ovltype) != 0) {
		for (col = 0; col < BOOT_BLOB_MAX_MAGIC_NUMS; col++) {
			if (magic_search.magics[col][0] == e) {
				break;
			} else if (magic_search.magics[col][0] == 0) {
				magic_search.magics[col][0] = e;
				break;
			}
		}
		i++;
	}
	printf("done\n");

}

static int is_magic_match(uint32_t magic)
{
	/* DPFE blob maigc last nibble is the segment id. Ignore it */
	if ( IS_DPFE_MAGIC(magic_search.last))
		return ((magic_search.last&DPFE_MAGIC_MASK)
			== (magic&DPFE_MAGIC_MASK));
	else
		return (magic_search.last == magic);
}

static uint32_t find_magic(uint32_t magic)
{
	boot_blob_hdr hdr;
	uint32_t found;
	uint32_t end_addr = BOOT_BLOB_SEARCH_END_ADDR;

	if (!is_magic_match(magic)) {
		init_find_magic();
		magic_search.index = BOOT_BLOB_SEARCH_START_ADDR;
	}

	/* TODO start with cached offsets if already located ... */
	/* debug("look for magic number 0x%x starting at address 0x%x\n", */
	/*			   magic, magic_search.index); */
	while (magic_search.index < end_addr) {
		/* For DPFE segment blobs, all the segments are concontinous 
		 * in the flash. So the subsequent segment will be at least 
		 * at current addr + segment size. so we skip the search for 
		 * current segment to redcue read on boot blocks
		 */ 
		if (IS_DPFE_MAGIC(magic_search.last) && magic == (magic_search.last+1)
		    && magic_search.dpfe_fs) {
			magic_search.index += ROUND(magic_search.length + sizeof(hdr),
			    BOOT_BLOB_SEARCH_BOUNDARY);
			/* printf("magic 0x%x skip current dpfe blob, quick search at 0x%x\n", 
			      magic, magic_search.index);*/
		}
		
		hdr.length = hdr.magic = 0;
		/* if read fail, we hit bad block and revert back to 4k search */
		if (read_boot_device(magic_search.index, &hdr, sizeof(hdr)) < 0)
			magic_search.dpfe_fs = 0;

		/* TODO cache other magic numbers location while searching ... */
		found = magic_search.index;

		/* Return magic if we found it */
		if (hdr.magic == magic) {
			debug("find magic number 0x%x at address 0x%x\n",
				   magic, found);
			magic_search.last = hdr.magic;
			magic_search.length = hdr.length;
			if (IS_DPFE_MAGIC(magic_search.last))
				magic_search.dpfe_fs = 1;
			return found;
		} else {
			magic_search.dpfe_fs = 0;
		}

		/* Update search index to point to next entry */
		magic_search.index += BOOT_BLOB_SEARCH_BOUNDARY;
	}
	return ~0;
}

static int read_blob_from_flash(uint32_t magic, uint32_t offset, void *data,
				int len, uint8_t * digest)
{
	uint32_t entry;
	int ret = -1;

	while ((entry = find_magic(magic)) < BOOT_BLOB_SEARCH_END_ADDR) {
		printf("reading blob from 0x%x offset 0x%x len %d\n", entry, offset, len);
		read_boot_device(entry + offset, data, len);
		ret = validate_blob_sha256_digest(data, len, digest);
		if (ret == 0) {
			debug("digest sha256 OK\n");
			return BOOT_BLOB_SUCCESS;
		} else {
			debug("digest sha256 mismatch\n");
			magic_search.index += BOOT_BLOB_SEARCH_BOUNDARY;
			magic_search.dpfe_fs = 0;
		}
	}

	return BOOT_BLOB_MAGIC_NOT_FOUND;
}

int load_boot_blob(uint32_t magic, uint32_t sel, void *data, int* len)
{
	int offset;
	int size;
	unsigned char digest[32];

	/* find out the offset of the blob from the hash table first */
	if ((offset = search_blob_in_hash(magic, sel, &size, digest)) < 0)
		return BOOT_BLOB_NOT_IN_HASTTBL;

	if (size > *len) {
		printf("buffer too small\n");
		return BOOT_BLOB_INVALID_PARAM;
	}

	*len = size;
	/* find out the first the blob with the magic number from flash */
	return read_blob_from_flash(magic, offset, data, size, digest);
}

__weak void * load_spl_env(void *buffer)
{
	uint32_t entry;
	uint32_t len;
	uint32_t crc;
	uint32_t got;
	env_t *ep;
	while ((entry =
		find_magic(UBOOT_ENV_MAGIC)) < BOOT_BLOB_SEARCH_END_ADDR) {
		uint32_t *d = (uint32_t *) buffer;
		read_boot_device(entry, d, 8);
		len = d[1];
		if (len > BOOT_BLOB_MAX_ENV_SIZE) {
			magic_search.index += BOOT_BLOB_SEARCH_BOUNDARY;
			continue;
		}
		read_boot_device(entry, d, len + 12);
		ep = (env_t *)(d+2);
		memcpy(&crc, &ep->crc, sizeof(crc));

		got = crc32(0, ep->data, len-4);

		if (got != crc) {
			debug("CRC mismatch len = %d\n",len);
			debug("computed %x \n",got);
			debug("expected %x \n",crc);
			magic_search.index += BOOT_BLOB_SEARCH_BOUNDARY;
		} else {
			return(buffer);
		}
	}
	return(NULL);
}


struct overlays* get_boot_blob_hash_entry(int i)
{

	if (i >= sizeof(ovl)/sizeof(struct overlays))
		return NULL;
	else
		return &ovl[i];
}
