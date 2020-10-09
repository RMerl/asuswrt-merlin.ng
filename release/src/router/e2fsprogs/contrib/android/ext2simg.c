/*
 * Copyright (C) 2016 The Android Open Source Project
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

#include <libgen.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ext2fs/ext2fs.h>
#include <et/com_err.h>
#include <sparse/sparse.h>

struct {
	int	crc;
	int	sparse;
	int	gzip;
	char	*in_file;
	char	*out_file;
	bool	overwrite_input;
} params = {
	.crc	    = 0,
	.sparse	    = 1,
	.gzip	    = 0,
};

#define ext2fs_fatal(Retval, Format, ...) \
	do { \
		com_err("error", Retval, Format, __VA_ARGS__); \
		exit(EXIT_FAILURE); \
	} while(0)

#define sparse_fatal(Format) \
	do { \
		fprintf(stderr, "sparse: "Format); \
		exit(EXIT_FAILURE); \
	} while(0)

static void usage(char *path)
{
	char *progname = basename(path);

	fprintf(stderr, "%s [ options ] <image or block device> <output image>\n"
			"  -c include CRC block\n"
			"  -z gzip output\n"
			"  -S don't use sparse output format\n", progname);
}

static struct buf_item {
	struct buf_item	    *next;
	void		    *buf[0];
} *buf_list;

static void add_chunk(ext2_filsys fs, struct sparse_file *s, blk_t chunk_start, blk_t chunk_end)
{
	int retval;
	unsigned int nb_blk = chunk_end - chunk_start;
	size_t len = nb_blk * fs->blocksize;
	int64_t offset = (int64_t)chunk_start * (int64_t)fs->blocksize;

	if (params.overwrite_input == false) {
		if (sparse_file_add_file(s, params.in_file, offset, len, chunk_start) < 0)
			sparse_fatal("adding data to the sparse file");
	} else {
		/*
		 * The input file will be overwritten, make a copy of
		 * the blocks
		 */
		struct buf_item *bi = calloc(1, sizeof(struct buf_item) + len);
		if (buf_list == NULL)
			buf_list = bi;
		else {
			bi->next = buf_list;
			buf_list = bi;
		}

		retval = io_channel_read_blk64(fs->io, chunk_start, nb_blk, bi->buf);
		if (retval < 0)
			ext2fs_fatal(retval, "reading block %u - %u", chunk_start, chunk_end);

		if (sparse_file_add_data(s, bi->buf, len, chunk_start) < 0)
			sparse_fatal("adding data to the sparse file");
	}
}

static void free_chunks(void)
{
	struct buf_item *bi;

	while (buf_list) {
		bi = buf_list->next;
		free(buf_list);
		buf_list = bi;
	}
}

static struct sparse_file *ext_to_sparse(const char *in_file)
{
	errcode_t retval;
	ext2_filsys fs;
	struct sparse_file *s;
	int64_t chunk_start = -1;
	blk_t first_blk, last_blk, nb_blk, cur_blk;

	retval = ext2fs_open(in_file, 0, 0, 0, unix_io_manager, &fs);
	if (retval)
		ext2fs_fatal(retval, "while reading %s", in_file);

	retval = ext2fs_read_block_bitmap(fs);
	if (retval)
		ext2fs_fatal(retval, "while reading block bitmap of %s", in_file);

	first_blk = ext2fs_get_block_bitmap_start2(fs->block_map);
	last_blk = ext2fs_get_block_bitmap_end2(fs->block_map);
	nb_blk = last_blk - first_blk + 1;

	s = sparse_file_new(fs->blocksize, (uint64_t)fs->blocksize * (uint64_t)nb_blk);
	if (!s)
		sparse_fatal("creating sparse file");

	/*
	 * The sparse format encodes the size of a chunk (and its header) in a
	 * 32-bit unsigned integer (UINT32_MAX)
	 * When writing the chunk, the library uses a single call to write().
	 * Linux's implementation of the 'write' syscall does not allow transfers
	 * larger than INT32_MAX (32-bit _and_ 64-bit systems).
	 * Make sure we do not create chunks larger than this limit.
	 */
	int64_t max_blk_per_chunk = (INT32_MAX - 12) / fs->blocksize;

	/* Iter on the blocks to merge contiguous chunk */
	for (cur_blk = first_blk; cur_blk <= last_blk; ++cur_blk) {
		if (ext2fs_test_block_bitmap2(fs->block_map, cur_blk)) {
			if (chunk_start == -1) {
				chunk_start = cur_blk;
			} else if (cur_blk - chunk_start + 1 == max_blk_per_chunk) {
				add_chunk(fs, s, chunk_start, cur_blk);
				chunk_start = -1;
			}
		} else if (chunk_start != -1) {
			add_chunk(fs, s, chunk_start, cur_blk);
			chunk_start = -1;
		}
	}
	if (chunk_start != -1)
		add_chunk(fs, s, chunk_start, cur_blk - 1);

	ext2fs_free(fs);
	return s;
}

static bool same_file(const char *in, const char *out)
{
	struct stat st1, st2;

	if (access(out, F_OK) == -1)
		return false;

	if (lstat(in, &st1) == -1)
		ext2fs_fatal(errno, "stat %s\n", in);
	if (lstat(out, &st2) == -1)
		ext2fs_fatal(errno, "stat %s\n", out);
	return st1.st_ino == st2.st_ino;
}

int main(int argc, char *argv[])
{
	int opt;
	int out_fd;
	struct sparse_file *s;

	while ((opt = getopt(argc, argv, "czS")) != -1) {
		switch(opt) {
		case 'c':
			params.crc = 1;
			break;
		case 'z':
			params.gzip = 1;
			break;
		case 'S':
			params.sparse = 0;
			break;
		default:
			usage(argv[0]);
			exit(EXIT_FAILURE);
		}
	}
	if (optind + 1 >= argc) {
		usage(argv[0]);
		exit(EXIT_FAILURE);
	}
	params.in_file = strdup(argv[optind++]);
	params.out_file = strdup(argv[optind]);
	params.overwrite_input = same_file(params.in_file, params.out_file);

	s = ext_to_sparse(params.in_file);

	out_fd = open(params.out_file, O_WRONLY | O_CREAT | O_TRUNC, 0664);
	if (out_fd == -1)
		ext2fs_fatal(errno, "opening %s\n", params.out_file);
	if (sparse_file_write(s, out_fd, params.gzip, params.sparse, params.crc) < 0)
		sparse_fatal("writing sparse file");

	sparse_file_destroy(s);

	free(params.in_file);
	free(params.out_file);
	free_chunks();
	close(out_fd);

	return 0;
}
