// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2006
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <common.h>
#include <watchdog.h>
#include <command.h>
#include <console.h>
#include <image.h>
#include <malloc.h>
#include <memalign.h>
#include <u-boot/zlib.h>
#include <div64.h>

#define HEADER0			'\x1f'
#define HEADER1			'\x8b'
#define	ZALLOC_ALIGNMENT	16
#define HEAD_CRC		2
#define EXTRA_FIELD		4
#define ORIG_NAME		8
#define COMMENT			0x10
#define RESERVED		0xe0
#define DEFLATED		8

void *gzalloc(void *x, unsigned items, unsigned size)
{
	void *p;

	size *= items;
	size = (size + ZALLOC_ALIGNMENT - 1) & ~(ZALLOC_ALIGNMENT - 1);

	p = malloc (size);

	return (p);
}

void gzfree(void *x, void *addr, unsigned nb)
{
	free (addr);
}

int gzip_parse_header(const unsigned char *src, unsigned long len)
{
	int i, flags;

	/* skip header */
	i = 10;
	flags = src[3];
	if (src[2] != DEFLATED || (flags & RESERVED) != 0) {
		puts ("Error: Bad gzipped data\n");
		return (-1);
	}
	if ((flags & EXTRA_FIELD) != 0)
		i = 12 + src[10] + (src[11] << 8);
	if ((flags & ORIG_NAME) != 0)
		while (src[i++] != 0)
			;
	if ((flags & COMMENT) != 0)
		while (src[i++] != 0)
			;
	if ((flags & HEAD_CRC) != 0)
		i += 2;
	if (i >= len) {
		puts ("Error: gunzip out of data in header\n");
		return (-1);
	}
	return i;
}

int gunzip(void *dst, int dstlen, unsigned char *src, unsigned long *lenp)
{
	int offset = gzip_parse_header(src, *lenp);

	if (offset < 0)
		return offset;

	return zunzip(dst, dstlen, src, lenp, 1, offset);
}

#ifdef CONFIG_CMD_UNZIP
__weak
void gzwrite_progress_init(u64 expectedsize)
{
	putc('\n');
}

__weak
void gzwrite_progress(int iteration,
		     u64 bytes_written,
		     u64 total_bytes)
{
	if (0 == (iteration & 3))
		printf("%llu/%llu\r", bytes_written, total_bytes);
}

__weak
void gzwrite_progress_finish(int returnval,
			     u64 bytes_written,
			     u64 total_bytes,
			     u32 expected_crc,
			     u32 calculated_crc)
{
	if (0 == returnval) {
		printf("\n\t%llu bytes, crc 0x%08x\n",
		       total_bytes, calculated_crc);
	} else {
		printf("\n\tuncompressed %llu of %llu\n"
		       "\tcrcs == 0x%08x/0x%08x\n",
		       bytes_written, total_bytes,
		       expected_crc, calculated_crc);
	}
}

int gzwrite(unsigned char *src, int len,
	    struct blk_desc *dev,
	    unsigned long szwritebuf,
	    u64 startoffs,
	    u64 szexpected)
{
	int i, flags;
	z_stream s;
	int r = 0;
	unsigned char *writebuf;
	unsigned crc = 0;
	u64 totalfilled = 0;
	lbaint_t blksperbuf, outblock;
	u32 expected_crc;
	u32 payload_size;
	int iteration = 0;

	if (!szwritebuf ||
	    (szwritebuf % dev->blksz) ||
	    (szwritebuf < dev->blksz)) {
		printf("%s: size %lu not a multiple of %lu\n",
		       __func__, szwritebuf, dev->blksz);
		return -1;
	}

	if (startoffs & (dev->blksz-1)) {
		printf("%s: start offset %llu not a multiple of %lu\n",
		       __func__, startoffs, dev->blksz);
		return -1;
	}

	blksperbuf = szwritebuf / dev->blksz;
	outblock = lldiv(startoffs, dev->blksz);

	/* skip header */
	i = 10;
	flags = src[3];
	if (src[2] != DEFLATED || (flags & RESERVED) != 0) {
		puts("Error: Bad gzipped data\n");
		return -1;
	}
	if ((flags & EXTRA_FIELD) != 0)
		i = 12 + src[10] + (src[11] << 8);
	if ((flags & ORIG_NAME) != 0)
		while (src[i++] != 0)
			;
	if ((flags & COMMENT) != 0)
		while (src[i++] != 0)
			;
	if ((flags & HEAD_CRC) != 0)
		i += 2;

	if (i >= len-8) {
		puts("Error: gunzip out of data in header");
		return -1;
	}

	payload_size = len - i - 8;

	memcpy(&expected_crc, src + len - 8, sizeof(expected_crc));
	expected_crc = le32_to_cpu(expected_crc);
	u32 szuncompressed;
	memcpy(&szuncompressed, src + len - 4, sizeof(szuncompressed));
	if (szexpected == 0) {
		szexpected = le32_to_cpu(szuncompressed);
	} else if (szuncompressed != (u32)szexpected) {
		printf("size of %llx doesn't match trailer low bits %x\n",
		       szexpected, szuncompressed);
		return -1;
	}
	if (lldiv(szexpected, dev->blksz) > (dev->lba - outblock)) {
		printf("%s: uncompressed size %llu exceeds device size\n",
		       __func__, szexpected);
		return -1;
	}

	gzwrite_progress_init(szexpected);

	s.zalloc = gzalloc;
	s.zfree = gzfree;

	r = inflateInit2(&s, -MAX_WBITS);
	if (r != Z_OK) {
		printf("Error: inflateInit2() returned %d\n", r);
		return -1;
	}

	s.next_in = src + i;
	s.avail_in = payload_size+8;
	writebuf = (unsigned char *)malloc_cache_aligned(szwritebuf);

	/* decompress until deflate stream ends or end of file */
	do {
		if (s.avail_in == 0) {
			printf("%s: weird termination with result %d\n",
			       __func__, r);
			break;
		}

		/* run inflate() on input until output buffer not full */
		do {
			unsigned long blocks_written;
			int numfilled;
			lbaint_t writeblocks;

			s.avail_out = szwritebuf;
			s.next_out = writebuf;
			r = inflate(&s, Z_SYNC_FLUSH);
			if ((r != Z_OK) &&
			    (r != Z_STREAM_END)) {
				printf("Error: inflate() returned %d\n", r);
				goto out;
			}
			numfilled = szwritebuf - s.avail_out;
			crc = crc32(crc, writebuf, numfilled);
			totalfilled += numfilled;
			if (numfilled < szwritebuf) {
				writeblocks = (numfilled+dev->blksz-1)
						/ dev->blksz;
				memset(writebuf+numfilled, 0,
				       dev->blksz-(numfilled%dev->blksz));
			} else {
				writeblocks = blksperbuf;
			}

			gzwrite_progress(iteration++,
					 totalfilled,
					 szexpected);
			blocks_written = blk_dwrite(dev, outblock,
						    writeblocks, writebuf);
			outblock += blocks_written;
			if (ctrlc()) {
				puts("abort\n");
				goto out;
			}
			WATCHDOG_RESET();
		} while (s.avail_out == 0);
		/* done when inflate() says it's done */
	} while (r != Z_STREAM_END);

	if ((szexpected != totalfilled) ||
	    (crc != expected_crc))
		r = -1;
	else
		r = 0;

out:
	gzwrite_progress_finish(r, totalfilled, szexpected,
				expected_crc, crc);
	free(writebuf);
	inflateEnd(&s);

	return r;
}
#endif

/*
 * Uncompress blocks compressed with zlib without headers
 */
int zunzip(void *dst, int dstlen, unsigned char *src, unsigned long *lenp,
						int stoponerr, int offset)
{
	z_stream s;
	int err = 0;
	int r;

	s.zalloc = gzalloc;
	s.zfree = gzfree;

	r = inflateInit2(&s, -MAX_WBITS);
	if (r != Z_OK) {
		printf("Error: inflateInit2() returned %d\n", r);
		return -1;
	}
	s.next_in = src + offset;
	s.avail_in = *lenp - offset;
	s.next_out = dst;
	s.avail_out = dstlen;
	do {
		r = inflate(&s, Z_FINISH);
		if (stoponerr == 1 && r != Z_STREAM_END &&
		    (s.avail_in == 0 || s.avail_out == 0 || r != Z_BUF_ERROR)) {
			printf("Error: inflate() returned %d\n", r);
			err = -1;
			break;
		}
	} while (r == Z_BUF_ERROR);
	*lenp = s.next_out - (unsigned char *) dst;
	inflateEnd(&s);

	return err;
}
