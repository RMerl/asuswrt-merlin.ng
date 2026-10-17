// SPDX-License-Identifier: GPL-2.0+
/*
 * BTRFS filesystem implementation for U-Boot
 *
 * 2017 Marek Behún, CZ.NIC, kabel@kernel.org
 */

#include "btrfs.h"
#include <abuf.h>
#include <log.h>
#include <malloc.h>
#include <linux/lzo.h>
#include <linux/zstd.h>
#include <linux/compat.h>
#include <u-boot/zlib.h>
#include <asm/unaligned.h>

/* Header for each segment, LE32, recording the compressed size */
#define LZO_LEN		4
static u32 decompress_lzo(const u8 *cbuf, u32 clen, u8 *dbuf, u32 dlen)
{
	u32 tot_len, tot_in, in_len, res;
	size_t out_len;
	int ret;

	if (clen < LZO_LEN)
		return -1;

	tot_len = le32_to_cpu(get_unaligned((u32 *)cbuf));
	tot_in = 0;
	cbuf += LZO_LEN;
	clen -= LZO_LEN;
	tot_len -= LZO_LEN;
	tot_in += LZO_LEN;

	if (tot_len == 0 && dlen)
		return -1;
	if (tot_len < LZO_LEN)
		return -1;

	res = 0;

	while (tot_len > LZO_LEN) {
		u32 rem_page;

		in_len = le32_to_cpu(get_unaligned((u32 *)cbuf));
		cbuf += LZO_LEN;
		clen -= LZO_LEN;

		if (in_len > clen || tot_len < LZO_LEN + in_len)
			return -1;

		tot_len -= (LZO_LEN + in_len);
		tot_in += (LZO_LEN + in_len);

		out_len = dlen;
		ret = lzo1x_decompress_safe(cbuf, in_len, dbuf, &out_len);
		if (ret != LZO_E_OK)
			return -1;

		cbuf += in_len;
		clen -= in_len;
		dbuf += out_len;
		dlen -= out_len;

		res += out_len;

		/*
		 * If the 4 bytes header does not fit to the rest of the page we
		 * have to move to next one, or we read some garbage.
		 */
		rem_page = PAGE_SIZE - (tot_in % PAGE_SIZE);
		if (rem_page < LZO_LEN) {
			cbuf += rem_page;
			tot_in += rem_page;
			clen -= rem_page;
			tot_len -= rem_page;
		}
	}

	return res;
}

/* from zutil.h */
#define PRESET_DICT 0x20

static u32 decompress_zlib(const u8 *_cbuf, u32 clen, u8 *dbuf, u32 dlen)
{
	int wbits = MAX_WBITS, ret = -1;
	z_stream stream;
	u8 *cbuf;
	u32 res;

	memset(&stream, 0, sizeof(stream));

	cbuf = (u8 *) _cbuf;

	stream.total_in = 0;

	stream.next_out = dbuf;
	stream.avail_out = dlen;
	stream.total_out = 0;

	/* skip adler32 check if deflate and no dictionary */
	if (clen > 2 && !(cbuf[1] & PRESET_DICT) &&
	    ((cbuf[0] & 0x0f) == Z_DEFLATED) &&
	    !(((cbuf[0] << 8) + cbuf[1]) % 31)) {
		wbits = -((cbuf[0] >> 4) + 8);
		cbuf += 2;
		clen -= 2;
	}

	if (Z_OK != inflateInit2(&stream, wbits))
		return -1;

	while (stream.total_in < clen) {
		stream.next_in = cbuf + stream.total_in;
		stream.avail_in = min((u32) (clen - stream.total_in),
					current_fs_info->sectorsize);

		ret = inflate(&stream, Z_NO_FLUSH);
		if (ret != Z_OK)
			break;
	}

	res = stream.total_out;
	inflateEnd(&stream);

	if (ret != Z_STREAM_END)
		return -1;

	return res;
}

#define ZSTD_BTRFS_MAX_WINDOWLOG 17
#define ZSTD_BTRFS_MAX_INPUT (1 << ZSTD_BTRFS_MAX_WINDOWLOG)

static u32 decompress_zstd(const u8 *cbuf, u32 clen, u8 *dbuf, u32 dlen)
{
	struct abuf in, out;

	abuf_init_set(&in, (u8 *)cbuf, clen);
	abuf_init_set(&out, dbuf, dlen);

	return zstd_decompress(&in, &out);
}

u32 btrfs_decompress(u8 type, const char *c, u32 clen, char *d, u32 dlen)
{
	u32 res;
	const u8 *cbuf;
	u8 *dbuf;

	cbuf = (const u8 *) c;
	dbuf = (u8 *) d;

	switch (type) {
	case BTRFS_COMPRESS_NONE:
		res = dlen < clen ? dlen : clen;
		memcpy(dbuf, cbuf, res);
		return res;
	case BTRFS_COMPRESS_ZLIB:
		return decompress_zlib(cbuf, clen, dbuf, dlen);
	case BTRFS_COMPRESS_LZO:
		return decompress_lzo(cbuf, clen, dbuf, dlen);
	case BTRFS_COMPRESS_ZSTD:
		return decompress_zstd(cbuf, clen, dbuf, dlen);
	default:
		printf("%s: Unsupported compression in extent: %i\n", __func__,
		       type);
		return -1;
	}
}
