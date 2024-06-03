// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013, The Chromium Authors
 */

#include <common.h>
#include <bootm.h>
#include <command.h>
#include <malloc.h>
#include <mapmem.h>
#include <asm/io.h>

#include <u-boot/zlib.h>
#include <bzlib.h>

#include <lzma/LzmaTypes.h>
#include <lzma/LzmaDec.h>
#include <lzma/LzmaTools.h>

#include <linux/lzo.h>
#include <test/compression.h>
#include <test/suites.h>
#include <test/ut.h>

static const char plain[] =
	"I am a highly compressable bit of text.\n"
	"I am a highly compressable bit of text.\n"
	"I am a highly compressable bit of text.\n"
	"There are many like me, but this one is mine.\n"
	"If I were any shorter, there wouldn't be much sense in\n"
	"compressing me in the first place. At least with lzo, anyway,\n"
	"which appears to behave poorly in the face of short text\n"
	"messages.\n";

/* bzip2 -c /tmp/plain.txt > /tmp/plain.bz2 */
static const char bzip2_compressed[] =
	"\x42\x5a\x68\x39\x31\x41\x59\x26\x53\x59\xe5\x63\xdd\x09\x00\x00"
	"\x28\x57\x80\x00\x10\x40\x85\x20\x20\x04\x00\x3f\xef\xdf\xf0\x30"
	"\x00\xd6\xd0\x34\x91\x89\xa6\xf5\x4d\x19\x1a\x19\x0d\x02\x34\xd4"
	"\xc9\x00\x34\x34\x00\x02\x48\x41\x35\x4f\xd4\xc6\x88\xd3\x50\x3d"
	"\x4f\x51\x82\x4f\x88\xc3\x0d\x05\x62\x4f\x91\xa3\x52\x1b\xd0\x52"
	"\x41\x4a\xa3\x98\xc2\x6b\xca\xa3\x82\xa5\xac\x8b\x15\x99\x68\xad"
	"\xdf\x29\xd6\xf1\xf7\x5a\x10\xcd\x8c\x26\x61\x94\x95\xfe\x9e\x16"
	"\x18\x28\x69\xd4\x23\x64\xcc\x2b\xe5\xe8\x5f\x00\xa4\x70\x26\x2c"
	"\xee\xbd\x59\x6d\x6a\xec\xfc\x31\xda\x59\x0a\x14\x2a\x60\x1c\xf0"
	"\x04\x86\x73\x9a\xc5\x5b\x87\x3f\x5b\x4c\x93\xe6\xb5\x35\x0d\xa6"
	"\xb1\x2e\x62\x7b\xab\x67\xe7\x99\x2a\x14\x5e\x9f\x64\xcb\x96\xf4"
	"\x0d\x65\xd4\x39\xe6\x8b\x7e\xea\x1c\x03\x69\x97\x83\x58\x91\x96"
	"\xe1\xf0\x9d\xa4\x15\x8b\xb8\xc6\x93\xdc\x3d\xd9\x3c\x22\x55\xef"
	"\xfb\xbb\x2a\xd3\x87\xa2\x8b\x04\xd9\x19\xf8\xe2\xfd\x4f\xdb\x1a"
	"\x07\xc8\x60\xa3\x3f\xf8\xbb\x92\x29\xc2\x84\x87\x2b\x1e\xe8\x48";
static const unsigned long bzip2_compressed_size = 240;

/* lzma -z -c /tmp/plain.txt > /tmp/plain.lzma */
static const char lzma_compressed[] =
	"\x5d\x00\x00\x80\x00\xff\xff\xff\xff\xff\xff\xff\xff\x00\x24\x88"
	"\x08\x26\xd8\x41\xff\x99\xc8\xcf\x66\x3d\x80\xac\xba\x17\xf1\xc8"
	"\xb9\xdf\x49\x37\xb1\x68\xa0\x2a\xdd\x63\xd1\xa7\xa3\x66\xf8\x15"
	"\xef\xa6\x67\x8a\x14\x18\x80\xcb\xc7\xb1\xcb\x84\x6a\xb2\x51\x16"
	"\xa1\x45\xa0\xd6\x3e\x55\x44\x8a\x5c\xa0\x7c\xe5\xa8\xbd\x04\x57"
	"\x8f\x24\xfd\xb9\x34\x50\x83\x2f\xf3\x46\x3e\xb9\xb0\x00\x1a\xf5"
	"\xd3\x86\x7e\x8f\x77\xd1\x5d\x0e\x7c\xe1\xac\xde\xf8\x65\x1f\x4d"
	"\xce\x7f\xa7\x3d\xaa\xcf\x26\xa7\x58\x69\x1e\x4c\xea\x68\x8a\xe5"
	"\x89\xd1\xdc\x4d\xc7\xe0\x07\x42\xbf\x0c\x9d\x06\xd7\x51\xa2\x0b"
	"\x7c\x83\x35\xe1\x85\xdf\xee\xfb\xa3\xee\x2f\x47\x5f\x8b\x70\x2b"
	"\xe1\x37\xf3\x16\xf6\x27\x54\x8a\x33\x72\x49\xea\x53\x7d\x60\x0b"
	"\x21\x90\x66\xe7\x9e\x56\x61\x5d\xd8\xdc\x59\xf0\xac\x2f\xd6\x49"
	"\x6b\x85\x40\x08\x1f\xdf\x26\x25\x3b\x72\x44\xb0\xb8\x21\x2f\xb3"
	"\xd7\x9b\x24\x30\x78\x26\x44\x07\xc3\x33\xd1\x4d\x03\x1b\xe1\xff"
	"\xfd\xf5\x50\x8d\xca";
static const unsigned long lzma_compressed_size = 229;

/* lzop -c /tmp/plain.txt > /tmp/plain.lzo */
static const char lzo_compressed[] =
	"\x89\x4c\x5a\x4f\x00\x0d\x0a\x1a\x0a\x10\x30\x20\x60\x09\x40\x01"
	"\x05\x03\x00\x00\x09\x00\x00\x81\xb4\x52\x09\x54\xf1\x00\x00\x00"
	"\x00\x09\x70\x6c\x61\x69\x6e\x2e\x74\x78\x74\x65\xb1\x07\x9c\x00"
	"\x00\x01\x5e\x00\x00\x01\x0f\xc3\xc7\x7a\xe0\x00\x16\x49\x20\x61"
	"\x6d\x20\x61\x20\x68\x69\x67\x68\x6c\x79\x20\x63\x6f\x6d\x70\x72"
	"\x65\x73\x73\x61\x62\x6c\x65\x20\x62\x69\x74\x20\x6f\x66\x20\x74"
	"\x65\x78\x74\x2e\x0a\x20\x2f\x9c\x00\x00\x22\x54\x68\x65\x72\x65"
	"\x20\x61\x72\x65\x20\x6d\x61\x6e\x79\x20\x6c\x69\x6b\x65\x20\x6d"
	"\x65\x2c\x20\x62\x75\x74\x20\x74\x68\x69\x73\x20\x6f\x6e\x65\x20"
	"\x69\x73\x20\x6d\x69\x6e\x65\x2e\x0a\x49\x66\x20\x49\x20\x77\x84"
	"\x06\x0a\x6e\x79\x20\x73\x68\x6f\x72\x74\x65\x72\x2c\x20\x74\x90"
	"\x08\x00\x08\x77\x6f\x75\x6c\x64\x6e\x27\x74\x20\x62\x65\x20\x6d"
	"\x75\x63\x68\x20\x73\x65\x6e\x73\x65\x20\x69\x6e\x0a\xf8\x19\x02"
	"\x69\x6e\x67\x20\x6d\x64\x02\x64\x06\x00\x5a\x20\x66\x69\x72\x73"
	"\x74\x20\x70\x6c\x61\x63\x65\x2e\x20\x41\x74\x20\x6c\x65\x61\x73"
	"\x74\x20\x77\x69\x74\x68\x20\x6c\x7a\x6f\x2c\x20\x61\x6e\x79\x77"
	"\x61\x79\x2c\x0a\x77\x68\x69\x63\x68\x20\x61\x70\x70\x65\x61\x72"
	"\x73\x20\x74\x6f\x20\x62\x65\x68\x61\x76\x65\x20\x70\x6f\x6f\x72"
	"\x6c\x79\x20\x69\x6e\x20\x74\x68\x65\x20\x66\x61\x63\x65\x20\x6f"
	"\x66\x20\x73\x68\x6f\x72\x74\x20\x74\x65\x78\x74\x0a\x6d\x65\x73"
	"\x73\x61\x67\x65\x73\x2e\x0a\x11\x00\x00\x00\x00\x00\x00";
static const unsigned long lzo_compressed_size = 334;

/* lz4 -z /tmp/plain.txt > /tmp/plain.lz4 */
static const char lz4_compressed[] =
	"\x04\x22\x4d\x18\x64\x70\xb9\x01\x01\x00\x00\xff\x19\x49\x20\x61"
	"\x6d\x20\x61\x20\x68\x69\x67\x68\x6c\x79\x20\x63\x6f\x6d\x70\x72"
	"\x65\x73\x73\x61\x62\x6c\x65\x20\x62\x69\x74\x20\x6f\x66\x20\x74"
	"\x65\x78\x74\x2e\x0a\x28\x00\x3d\xf1\x25\x54\x68\x65\x72\x65\x20"
	"\x61\x72\x65\x20\x6d\x61\x6e\x79\x20\x6c\x69\x6b\x65\x20\x6d\x65"
	"\x2c\x20\x62\x75\x74\x20\x74\x68\x69\x73\x20\x6f\x6e\x65\x20\x69"
	"\x73\x20\x6d\x69\x6e\x65\x2e\x0a\x49\x66\x20\x49\x20\x77\x32\x00"
	"\xd1\x6e\x79\x20\x73\x68\x6f\x72\x74\x65\x72\x2c\x20\x74\x45\x00"
	"\xf4\x0b\x77\x6f\x75\x6c\x64\x6e\x27\x74\x20\x62\x65\x20\x6d\x75"
	"\x63\x68\x20\x73\x65\x6e\x73\x65\x20\x69\x6e\x0a\xcf\x00\x50\x69"
	"\x6e\x67\x20\x6d\x12\x00\x00\x32\x00\xf0\x11\x20\x66\x69\x72\x73"
	"\x74\x20\x70\x6c\x61\x63\x65\x2e\x20\x41\x74\x20\x6c\x65\x61\x73"
	"\x74\x20\x77\x69\x74\x68\x20\x6c\x7a\x6f\x2c\x63\x00\xf5\x14\x77"
	"\x61\x79\x2c\x0a\x77\x68\x69\x63\x68\x20\x61\x70\x70\x65\x61\x72"
	"\x73\x20\x74\x6f\x20\x62\x65\x68\x61\x76\x65\x20\x70\x6f\x6f\x72"
	"\x6c\x79\x4e\x00\x30\x61\x63\x65\x27\x01\x01\x95\x00\x01\x2d\x01"
	"\xb0\x0a\x6d\x65\x73\x73\x61\x67\x65\x73\x2e\x0a\x00\x00\x00\x00"
	"\x9d\x12\x8c\x9d";
static const unsigned long lz4_compressed_size = 276;


#define TEST_BUFFER_SIZE	512

typedef int (*mutate_func)(struct unit_test_state *uts, void *, unsigned long,
			   void *, unsigned long, unsigned long *);

static int compress_using_gzip(struct unit_test_state *uts,
			       void *in, unsigned long in_size,
			       void *out, unsigned long out_max,
			       unsigned long *out_size)
{
	int ret;
	unsigned long inout_size = out_max;

	ret = gzip(out, &inout_size, in, in_size);
	if (out_size)
		*out_size = inout_size;

	return ret;
}

static int uncompress_using_gzip(struct unit_test_state *uts,
				 void *in, unsigned long in_size,
				 void *out, unsigned long out_max,
				 unsigned long *out_size)
{
	int ret;
	unsigned long inout_size = in_size;

	ret = gunzip(out, out_max, in, &inout_size);
	if (out_size)
		*out_size = inout_size;

	return ret;
}

static int compress_using_bzip2(struct unit_test_state *uts,
				void *in, unsigned long in_size,
				void *out, unsigned long out_max,
				unsigned long *out_size)
{
	/* There is no bzip2 compression in u-boot, so fake it. */
	ut_asserteq(in_size, strlen(plain));
	ut_asserteq(0, memcmp(plain, in, in_size));

	if (bzip2_compressed_size > out_max)
		return -1;

	memcpy(out, bzip2_compressed, bzip2_compressed_size);
	if (out_size)
		*out_size = bzip2_compressed_size;

	return 0;
}

static int uncompress_using_bzip2(struct unit_test_state *uts,
				  void *in, unsigned long in_size,
				  void *out, unsigned long out_max,
				  unsigned long *out_size)
{
	int ret;
	unsigned int inout_size = out_max;

	ret = BZ2_bzBuffToBuffDecompress(out, &inout_size, in, in_size,
			CONFIG_SYS_MALLOC_LEN < (4096 * 1024), 0);
	if (out_size)
		*out_size = inout_size;

	return (ret != BZ_OK);
}

static int compress_using_lzma(struct unit_test_state *uts,
			       void *in, unsigned long in_size,
			       void *out, unsigned long out_max,
			       unsigned long *out_size)
{
	/* There is no lzma compression in u-boot, so fake it. */
	ut_asserteq(in_size,  strlen(plain));
	ut_asserteq(0, memcmp(plain, in, in_size));

	if (lzma_compressed_size > out_max)
		return -1;

	memcpy(out, lzma_compressed, lzma_compressed_size);
	if (out_size)
		*out_size = lzma_compressed_size;

	return 0;
}

static int uncompress_using_lzma(struct unit_test_state *uts,
				 void *in, unsigned long in_size,
				 void *out, unsigned long out_max,
				 unsigned long *out_size)
{
	int ret;
	SizeT inout_size = out_max;

	ret = lzmaBuffToBuffDecompress(out, &inout_size, in, in_size);
	if (out_size)
		*out_size = inout_size;

	return (ret != SZ_OK);
}

static int compress_using_lzo(struct unit_test_state *uts,
			      void *in, unsigned long in_size,
			      void *out, unsigned long out_max,
			      unsigned long *out_size)
{
	/* There is no lzo compression in u-boot, so fake it. */
	ut_asserteq(in_size,  strlen(plain));
	ut_asserteq(0, memcmp(plain, in, in_size));

	if (lzo_compressed_size > out_max)
		return -1;

	memcpy(out, lzo_compressed, lzo_compressed_size);
	if (out_size)
		*out_size = lzo_compressed_size;

	return 0;
}

static int uncompress_using_lzo(struct unit_test_state *uts,
				void *in, unsigned long in_size,
				void *out, unsigned long out_max,
				unsigned long *out_size)
{
	int ret;
	size_t input_size = in_size;
	size_t output_size = out_max;

	ret = lzop_decompress(in, input_size, out, &output_size);
	if (out_size)
		*out_size = output_size;

	return (ret != LZO_E_OK);
}

static int compress_using_lz4(struct unit_test_state *uts,
			      void *in, unsigned long in_size,
			      void *out, unsigned long out_max,
			      unsigned long *out_size)
{
	/* There is no lz4 compression in u-boot, so fake it. */
	ut_asserteq(in_size,  strlen(plain));
	ut_asserteq(0, memcmp(plain, in, in_size));

	if (lz4_compressed_size > out_max)
		return -1;

	memcpy(out, lz4_compressed, lz4_compressed_size);
	if (out_size)
		*out_size = lz4_compressed_size;

	return 0;
}

static int uncompress_using_lz4(struct unit_test_state *uts,
				void *in, unsigned long in_size,
				void *out, unsigned long out_max,
				unsigned long *out_size)
{
	int ret;
	size_t input_size = in_size;
	size_t output_size = out_max;

	ret = ulz4fn(in, input_size, out, &output_size);
	if (out_size)
		*out_size = output_size;

	return (ret != 0);
}

#define errcheck(statement) if (!(statement)) { \
	fprintf(stderr, "\tFailed: %s\n", #statement); \
	ret = 1; \
	goto out; \
}

struct buf_state {
	ulong orig_size;
	ulong compressed_size;
	ulong uncompressed_size;
	void *orig_buf;
	void *compressed_buf;
	void *uncompressed_buf;
	void *compare_buf;
};

static int run_test_internal(struct unit_test_state *uts, char *name,
			     mutate_func compress, mutate_func uncompress,
			     struct buf_state *buf)
{
	int ret;

	/* Compress works as expected. */
	printf("\torig_size:%lu\n", buf->orig_size);
	memset(buf->compressed_buf, 'A', TEST_BUFFER_SIZE);
	errcheck(compress(uts, buf->orig_buf, buf->orig_size,
			  buf->compressed_buf, buf->compressed_size,
			  &buf->compressed_size) == 0);
	printf("\tcompressed_size:%lu\n", buf->compressed_size);
	errcheck(buf->compressed_size > 0);
	errcheck(buf->compressed_size < buf->orig_size);
	errcheck(((char *)buf->compressed_buf)[buf->compressed_size - 1] !=
			'A');
	errcheck(((char *)buf->compressed_buf)[buf->compressed_size] == 'A');

	/* Uncompresses with space remaining. */
	errcheck(uncompress(uts, buf->compressed_buf, buf->compressed_size,
			    buf->uncompressed_buf, buf->uncompressed_size,
			    &buf->uncompressed_size) == 0);
	printf("\tuncompressed_size:%lu\n", buf->uncompressed_size);
	errcheck(buf->uncompressed_size == buf->orig_size);
	errcheck(memcmp(buf->orig_buf, buf->uncompressed_buf,
			buf->orig_size) == 0);

	/* Uncompresses with exactly the right size output buffer. */
	memset(buf->uncompressed_buf, 'A', TEST_BUFFER_SIZE);
	errcheck(uncompress(uts, buf->compressed_buf, buf->compressed_size,
			    buf->uncompressed_buf, buf->orig_size,
			    &buf->uncompressed_size) == 0);
	errcheck(buf->uncompressed_size == buf->orig_size);
	errcheck(memcmp(buf->orig_buf, buf->uncompressed_buf,
			buf->orig_size) == 0);
	errcheck(((char *)buf->uncompressed_buf)[buf->orig_size] == 'A');

	/* Make sure compression does not over-run. */
	memset(buf->compare_buf, 'A', TEST_BUFFER_SIZE);
	ret = compress(uts, buf->orig_buf, buf->orig_size,
		       buf->compare_buf, buf->compressed_size - 1,
		       NULL);
	errcheck(((char *)buf->compare_buf)[buf->compressed_size] == 'A');
	errcheck(ret != 0);
	printf("\tcompress does not overrun\n");

	/* Make sure decompression does not over-run. */
	memset(buf->compare_buf, 'A', TEST_BUFFER_SIZE);
	ret = uncompress(uts, buf->compressed_buf, buf->compressed_size,
			 buf->compare_buf, buf->uncompressed_size - 1,
			 NULL);
	errcheck(((char *)buf->compare_buf)[buf->uncompressed_size - 1] == 'A');
	errcheck(ret != 0);
	printf("\tuncompress does not overrun\n");

	/* Got here, everything is fine. */
	ret = 0;

out:
	return ret;
}

static int run_test(struct unit_test_state *uts, char *name,
		    mutate_func compress, mutate_func uncompress)
{
	struct buf_state sbuf, *buf = &sbuf;
	int ret;

	printf(" testing %s ...\n", name);

	buf->orig_buf = (void *)plain;
	buf->orig_size = strlen(buf->orig_buf); /* Trailing NUL not included */
	errcheck(buf->orig_size > 0);

	buf->compressed_size = TEST_BUFFER_SIZE;
	buf->uncompressed_size = TEST_BUFFER_SIZE;
	buf->compressed_buf = malloc(buf->compressed_size);
	errcheck(buf->compressed_buf);
	buf->uncompressed_buf = malloc(buf->uncompressed_size);
	errcheck(buf->uncompressed_buf);
	buf->compare_buf = malloc(buf->uncompressed_size);
	errcheck(buf->compare_buf);

	ret = run_test_internal(uts, name, compress, uncompress, buf);
out:
	printf(" %s: %s\n", name, ret == 0 ? "ok" : "FAILED");

	free(buf->compare_buf);
	free(buf->uncompressed_buf);
	free(buf->compressed_buf);

	return ret;
}

static int compression_test_gzip(struct unit_test_state *uts)
{
	return run_test(uts, "gzip", compress_using_gzip,
			uncompress_using_gzip);
}
COMPRESSION_TEST(compression_test_gzip, 0);

static int compression_test_bzip2(struct unit_test_state *uts)
{
	return run_test(uts, "bzip2", compress_using_bzip2,
			uncompress_using_bzip2);
}
COMPRESSION_TEST(compression_test_bzip2, 0);

static int compression_test_lzma(struct unit_test_state *uts)
{
	return run_test(uts, "lzma", compress_using_lzma,
			uncompress_using_lzma);
}
COMPRESSION_TEST(compression_test_lzma, 0);

static int compression_test_lzo(struct unit_test_state *uts)
{
	return run_test(uts, "lzo", compress_using_lzo, uncompress_using_lzo);
}
COMPRESSION_TEST(compression_test_lzo, 0);

static int compression_test_lz4(struct unit_test_state *uts)
{
	return run_test(uts, "lz4", compress_using_lz4, uncompress_using_lz4);
}
COMPRESSION_TEST(compression_test_lz4, 0);

static int compress_using_none(struct unit_test_state *uts,
			       void *in, unsigned long in_size,
			       void *out, unsigned long out_max,
			       unsigned long *out_size)
{
	/* Here we just copy */
	memcpy(out, in, in_size);
	*out_size = in_size;

	return 0;
}

/**
 * run_bootm_test() - Run tests on the bootm decopmression function
 *
 * @comp_type:	Compression type to test
 * @compress:	Our function to compress data
 * @return 0 if OK, non-zero on failure
 */
static int run_bootm_test(struct unit_test_state *uts, int comp_type,
			  mutate_func compress)
{
	ulong compress_size = 1024;
	void *compress_buff;
	int unc_len;
	int err = 0;
	const ulong image_start = 0;
	const ulong load_addr = 0x1000;
	ulong load_end;

	printf("Testing: %s\n", genimg_get_comp_name(comp_type));
	compress_buff = map_sysmem(image_start, 0);
	unc_len = strlen(plain);
	compress(uts, (void *)plain, unc_len, compress_buff, compress_size,
		 &compress_size);
	err = bootm_decomp_image(comp_type, load_addr, image_start,
				 IH_TYPE_KERNEL, map_sysmem(load_addr, 0),
				 compress_buff, compress_size, unc_len,
				 &load_end);
	ut_assertok(err);
	err = bootm_decomp_image(comp_type, load_addr, image_start,
				 IH_TYPE_KERNEL, map_sysmem(load_addr, 0),
				 compress_buff, compress_size, unc_len - 1,
				 &load_end);
	ut_assert(err);

	/* We can't detect corruption when not decompressing */
	if (comp_type == IH_COMP_NONE)
		return 0;
	memset(compress_buff + compress_size / 2, '\x49',
	       compress_size / 2);
	err = bootm_decomp_image(comp_type, load_addr, image_start,
				 IH_TYPE_KERNEL, map_sysmem(load_addr, 0),
				 compress_buff, compress_size, 0x10000,
				 &load_end);
	ut_assert(err);

	return 0;
}

static int compression_test_bootm_gzip(struct unit_test_state *uts)
{
	return run_bootm_test(uts, IH_COMP_GZIP, compress_using_gzip);
}
COMPRESSION_TEST(compression_test_bootm_gzip, 0);

static int compression_test_bootm_bzip2(struct unit_test_state *uts)
{
	return run_bootm_test(uts, IH_COMP_BZIP2, compress_using_bzip2);
}
COMPRESSION_TEST(compression_test_bootm_bzip2, 0);

static int compression_test_bootm_lzma(struct unit_test_state *uts)
{
	return run_bootm_test(uts, IH_COMP_LZMA, compress_using_lzma);
}
COMPRESSION_TEST(compression_test_bootm_lzma, 0);

static int compression_test_bootm_lzo(struct unit_test_state *uts)
{
	return run_bootm_test(uts, IH_COMP_LZO, compress_using_lzo);
}
COMPRESSION_TEST(compression_test_bootm_lzo, 0);

static int compression_test_bootm_lz4(struct unit_test_state *uts)
{
	return run_bootm_test(uts, IH_COMP_LZ4, compress_using_lz4);
}
COMPRESSION_TEST(compression_test_bootm_lz4, 0);

static int compression_test_bootm_none(struct unit_test_state *uts)
{
	return run_bootm_test(uts, IH_COMP_NONE, compress_using_none);
}
COMPRESSION_TEST(compression_test_bootm_none, 0);

int do_ut_compression(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	struct unit_test *tests = ll_entry_start(struct unit_test,
						 compression_test);
	const int n_ents = ll_entry_count(struct unit_test, compression_test);

	return cmd_ut_category("compression", tests, n_ents, argc, argv);
}
