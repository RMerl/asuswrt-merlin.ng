/*
 * Copyright (C) 2012 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define _GNU_SOURCE
#define _FILE_OFFSET_BITS 64
#define _LARGEFILE64_SOURCE 1

#include <fcntl.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sparse/sparse.h>

#include "sparse_crc32.h"
#include "sparse_file.h"
#include "sparse_format.h"

#if defined(__APPLE__) && defined(__MACH__)
#define lseek64 lseek
#define off64_t off_t
#endif

#define SPARSE_HEADER_MAJOR_VER 1
#define SPARSE_HEADER_LEN       (sizeof(sparse_header_t))
#define CHUNK_HEADER_LEN (sizeof(chunk_header_t))

#define COPY_BUF_SIZE (1024U*1024U)
static char *copybuf;

#define min(a, b) \
	({ typeof(a) _a = (a); typeof(b) _b = (b); (_a < _b) ? _a : _b; })

static void verbose_error(bool verbose, int err, const char *fmt, ...)
{
	char *s = "";
	char *at = "";
	if (fmt) {
		va_list argp;
		int size;

		va_start(argp, fmt);
		size = vsnprintf(NULL, 0, fmt, argp);
		va_end(argp);

		if (size < 0) {
			return;
		}

		at = malloc(size + 1);
		if (at == NULL) {
			return;
		}

		va_start(argp, fmt);
		vsnprintf(at, size, fmt, argp);
		va_end(argp);
		at[size] = 0;
		s = " at ";
	}
	if (verbose) {
#ifndef USE_MINGW
		if (err == -EOVERFLOW) {
			sparse_print_verbose("EOF while reading file%s%s\n", s, at);
		} else
#endif
		if (err == -EINVAL) {
			sparse_print_verbose("Invalid sparse file format%s%s\n", s, at);
		} else if (err == -ENOMEM) {
			sparse_print_verbose("Failed allocation while reading file%s%s\n",
					s, at);
		} else {
			sparse_print_verbose("Unknown error %d%s%s\n", err, s, at);
		}
	}
	if (fmt) {
		free(at);
	}
}

static int process_raw_chunk(struct sparse_file *s, unsigned int chunk_size,
		int fd, int64_t offset, unsigned int blocks, unsigned int block,
		uint32_t *crc32)
{
	int ret;
	int chunk;
	unsigned int len = blocks * s->block_size;

	if (chunk_size % s->block_size != 0) {
		return -EINVAL;
	}

	if (chunk_size / s->block_size != blocks) {
		return -EINVAL;
	}

	ret = sparse_file_add_fd(s, fd, offset, len, block);
	if (ret < 0) {
		return ret;
	}

	if (crc32) {
		while (len) {
			chunk = min(len, COPY_BUF_SIZE);
			ret = read_all(fd, copybuf, chunk);
			if (ret < 0) {
				return ret;
			}
			*crc32 = sparse_crc32(*crc32, copybuf, chunk);
			len -= chunk;
		}
	} else {
		lseek64(fd, len, SEEK_CUR);
	}

	return 0;
}

static int process_fill_chunk(struct sparse_file *s, unsigned int chunk_size,
		int fd, unsigned int blocks, unsigned int block, uint32_t *crc32)
{
	int ret;
	int chunk;
	int64_t len = (int64_t)blocks * s->block_size;
	uint32_t fill_val;
	uint32_t *fillbuf;
	unsigned int i;

	if (chunk_size != sizeof(fill_val)) {
		return -EINVAL;
	}

	ret = read_all(fd, &fill_val, sizeof(fill_val));
	if (ret < 0) {
		return ret;
	}

	ret = sparse_file_add_fill(s, fill_val, len, block);
	if (ret < 0) {
		return ret;
	}

	if (crc32) {
		/* Fill copy_buf with the fill value */
		fillbuf = (uint32_t *)copybuf;
		for (i = 0; i < (COPY_BUF_SIZE / sizeof(fill_val)); i++) {
			fillbuf[i] = fill_val;
		}

		while (len) {
			chunk = min(len, COPY_BUF_SIZE);
			*crc32 = sparse_crc32(*crc32, copybuf, chunk);
			len -= chunk;
		}
	}

	return 0;
}

static int process_skip_chunk(struct sparse_file *s, unsigned int chunk_size,
		int fd, unsigned int blocks, unsigned int block, uint32_t *crc32)
{
	int ret;
	int chunk;
	int64_t len = (int64_t)blocks * s->block_size;
	uint32_t fill_val;
	uint32_t *fillbuf;
	unsigned int i;

	if (chunk_size != 0) {
		return -EINVAL;
	}

	if (crc32) {
		memset(copybuf, 0, COPY_BUF_SIZE);

		while (len) {
			chunk = min(len, COPY_BUF_SIZE);
			*crc32 = sparse_crc32(*crc32, copybuf, chunk);
			len -= chunk;
		}
	}

	return 0;
}

static int process_crc32_chunk(int fd, unsigned int chunk_size, uint32_t crc32)
{
	uint32_t file_crc32;
	int ret;

	if (chunk_size != sizeof(file_crc32)) {
		return -EINVAL;
	}

	ret = read_all(fd, &file_crc32, sizeof(file_crc32));
	if (ret < 0) {
		return ret;
	}

	if (file_crc32 != crc32) {
		return -EINVAL;
	}

	return 0;
}

static int process_chunk(struct sparse_file *s, int fd, off64_t offset,
		unsigned int chunk_hdr_sz, chunk_header_t *chunk_header,
		unsigned int cur_block, uint32_t *crc_ptr)
{
	int ret;
	unsigned int chunk_data_size;

	chunk_data_size = chunk_header->total_sz - chunk_hdr_sz;

	switch (chunk_header->chunk_type) {
		case CHUNK_TYPE_RAW:
			ret = process_raw_chunk(s, chunk_data_size, fd, offset,
					chunk_header->chunk_sz, cur_block, crc_ptr);
			if (ret < 0) {
				verbose_error(s->verbose, ret, "data block at %lld", offset);
				return ret;
			}
			return chunk_header->chunk_sz;
		case CHUNK_TYPE_FILL:
			ret = process_fill_chunk(s, chunk_data_size, fd,
					chunk_header->chunk_sz, cur_block, crc_ptr);
			if (ret < 0) {
				verbose_error(s->verbose, ret, "fill block at %lld", offset);
				return ret;
			}
			return chunk_header->chunk_sz;
		case CHUNK_TYPE_DONT_CARE:
			ret = process_skip_chunk(s, chunk_data_size, fd,
					chunk_header->chunk_sz, cur_block, crc_ptr);
			if (chunk_data_size != 0) {
				if (ret < 0) {
					verbose_error(s->verbose, ret, "skip block at %lld", offset);
					return ret;
				}
			}
			return chunk_header->chunk_sz;
		case CHUNK_TYPE_CRC32:
			ret = process_crc32_chunk(fd, chunk_data_size, *crc_ptr);
			if (ret < 0) {
				verbose_error(s->verbose, -EINVAL, "crc block at %lld",
						offset);
				return ret;
			}
			return 0;
		default:
			verbose_error(s->verbose, -EINVAL, "unknown block %04X at %lld",
					chunk_header->chunk_type, offset);
	}

	return 0;
}

static int sparse_file_read_sparse(struct sparse_file *s, int fd, bool crc)
{
	int ret;
	unsigned int i;
	sparse_header_t sparse_header;
	chunk_header_t chunk_header;
	uint32_t crc32 = 0;
	uint32_t *crc_ptr = 0;
	unsigned int cur_block = 0;
	off64_t offset;

	if (!copybuf) {
		copybuf = malloc(COPY_BUF_SIZE);
	}

	if (!copybuf) {
		return -ENOMEM;
	}

	if (crc) {
		crc_ptr = &crc32;
	}

	ret = read_all(fd, &sparse_header, sizeof(sparse_header));
	if (ret < 0) {
		return ret;
	}

	if (sparse_header.magic != SPARSE_HEADER_MAGIC) {
		return -EINVAL;
	}

	if (sparse_header.major_version != SPARSE_HEADER_MAJOR_VER) {
		return -EINVAL;
	}

	if (sparse_header.file_hdr_sz < SPARSE_HEADER_LEN) {
		return -EINVAL;
	}

	if (sparse_header.chunk_hdr_sz < sizeof(chunk_header)) {
		return -EINVAL;
	}

	if (sparse_header.file_hdr_sz > SPARSE_HEADER_LEN) {
		/* Skip the remaining bytes in a header that is longer than
		 * we expected.
		 */
		lseek64(fd, sparse_header.file_hdr_sz - SPARSE_HEADER_LEN, SEEK_CUR);
	}

	for (i = 0; i < sparse_header.total_chunks; i++) {
		ret = read_all(fd, &chunk_header, sizeof(chunk_header));
		if (ret < 0) {
			return ret;
		}

		if (sparse_header.chunk_hdr_sz > CHUNK_HEADER_LEN) {
			/* Skip the remaining bytes in a header that is longer than
			 * we expected.
			 */
			lseek64(fd, sparse_header.chunk_hdr_sz - CHUNK_HEADER_LEN, SEEK_CUR);
		}

		offset = lseek64(fd, 0, SEEK_CUR);

		ret = process_chunk(s, fd, offset, sparse_header.chunk_hdr_sz, &chunk_header,
				cur_block, crc_ptr);
		if (ret < 0) {
			return ret;
		}

		cur_block += ret;
	}

	if (sparse_header.total_blks != cur_block) {
		return -EINVAL;
	}

	return 0;
}

static int sparse_file_read_normal(struct sparse_file *s, int fd)
{
	int ret;
	uint32_t *buf = malloc(s->block_size);
	unsigned int block = 0;
	int64_t remain = s->len;
	int64_t offset = 0;
	unsigned int to_read;
	char *ptr;
	unsigned int i;
	bool sparse_block;

	if (!buf) {
		return -ENOMEM;
	}

	while (remain > 0) {
		to_read = min(remain, s->block_size);
		ret = read_all(fd, buf, to_read);
		if (ret < 0) {
			error("failed to read sparse file");
			return ret;
		}

		if (to_read == s->block_size) {
			sparse_block = true;
			for (i = 1; i < s->block_size / sizeof(uint32_t); i++) {
				if (buf[0] != buf[i]) {
					sparse_block = false;
					break;
				}
			}
		} else {
			sparse_block = false;
		}

		if (sparse_block) {
			/* TODO: add flag to use skip instead of fill for buf[0] == 0 */
			sparse_file_add_fill(s, buf[0], to_read, block);
		} else {
			sparse_file_add_fd(s, fd, offset, to_read, block);
		}

		remain -= to_read;
		offset += to_read;
		block++;
	}

	return 0;
}

int sparse_file_read(struct sparse_file *s, int fd, bool sparse, bool crc)
{
	if (crc && !sparse) {
		return -EINVAL;
	}

	if (sparse) {
		return sparse_file_read_sparse(s, fd, crc);
	} else {
		return sparse_file_read_normal(s, fd);
	}
}

struct sparse_file *sparse_file_import(int fd, bool verbose, bool crc)
{
	int ret;
	sparse_header_t sparse_header;
	int64_t len;
	struct sparse_file *s;

	ret = read_all(fd, &sparse_header, sizeof(sparse_header));
	if (ret < 0) {
		verbose_error(verbose, ret, "header");
		return NULL;
	}

	if (sparse_header.magic != SPARSE_HEADER_MAGIC) {
		verbose_error(verbose, -EINVAL, "header magic");
		return NULL;
	}

	if (sparse_header.major_version != SPARSE_HEADER_MAJOR_VER) {
		verbose_error(verbose, -EINVAL, "header major version");
		return NULL;
	}

	if (sparse_header.file_hdr_sz < SPARSE_HEADER_LEN) {
		return NULL;
	}

	if (sparse_header.chunk_hdr_sz < sizeof(chunk_header_t)) {
		return NULL;
	}

	len = (int64_t)sparse_header.total_blks * sparse_header.blk_sz;
	s = sparse_file_new(sparse_header.blk_sz, len);
	if (!s) {
		verbose_error(verbose, -EINVAL, NULL);
		return NULL;
	}

	ret = lseek64(fd, 0, SEEK_SET);
	if (ret < 0) {
		verbose_error(verbose, ret, "seeking");
		sparse_file_destroy(s);
		return NULL;
	}

	s->verbose = verbose;

	ret = sparse_file_read(s, fd, true, crc);
	if (ret < 0) {
		sparse_file_destroy(s);
		return NULL;
	}

	return s;
}

struct sparse_file *sparse_file_import_auto(int fd, bool crc)
{
	struct sparse_file *s;
	int64_t len;
	int ret;

	s = sparse_file_import(fd, true, crc);
	if (s) {
		return s;
	}

	len = lseek64(fd, 0, SEEK_END);
	if (len < 0) {
		return NULL;
	}

	lseek64(fd, 0, SEEK_SET);

	s = sparse_file_new(4096, len);
	if (!s) {
		return NULL;
	}

	ret = sparse_file_read_normal(s, fd);
	if (ret < 0) {
		sparse_file_destroy(s);
		return NULL;
	}

	return s;
}
