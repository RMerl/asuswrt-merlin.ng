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

#include <assert.h>
#include <stdlib.h>

#include <sparse/sparse.h>

#include "sparse_file.h"

#include "output_file.h"
#include "backed_block.h"
#include "sparse_defs.h"
#include "sparse_format.h"

struct sparse_file *sparse_file_new(unsigned int block_size, int64_t len)
{
	struct sparse_file *s = calloc(sizeof(struct sparse_file), 1);
	if (!s) {
		return NULL;
	}

	s->backed_block_list = backed_block_list_new(block_size);
	if (!s->backed_block_list) {
		free(s);
		return NULL;
	}

	s->block_size = block_size;
	s->len = len;

	return s;
}

void sparse_file_destroy(struct sparse_file *s)
{
	backed_block_list_destroy(s->backed_block_list);
	free(s);
}

int sparse_file_add_data(struct sparse_file *s,
		void *data, unsigned int len, unsigned int block)
{
	return backed_block_add_data(s->backed_block_list, data, len, block);
}

int sparse_file_add_fill(struct sparse_file *s,
		uint32_t fill_val, unsigned int len, unsigned int block)
{
	return backed_block_add_fill(s->backed_block_list, fill_val, len, block);
}

int sparse_file_add_file(struct sparse_file *s,
		const char *filename, int64_t file_offset, unsigned int len,
		unsigned int block)
{
	return backed_block_add_file(s->backed_block_list, filename, file_offset,
			len, block);
}

int sparse_file_add_fd(struct sparse_file *s,
		int fd, int64_t file_offset, unsigned int len, unsigned int block)
{
	return backed_block_add_fd(s->backed_block_list, fd, file_offset,
			len, block);
}
unsigned int sparse_count_chunks(struct sparse_file *s)
{
	struct backed_block *bb;
	unsigned int last_block = 0;
	unsigned int chunks = 0;

	for (bb = backed_block_iter_new(s->backed_block_list); bb;
			bb = backed_block_iter_next(bb)) {
		if (backed_block_block(bb) > last_block) {
			/* If there is a gap between chunks, add a skip chunk */
			chunks++;
		}
		chunks++;
		last_block = backed_block_block(bb) +
				DIV_ROUND_UP(backed_block_len(bb), s->block_size);
	}
	if (last_block < DIV_ROUND_UP(s->len, s->block_size)) {
		chunks++;
	}

	return chunks;
}

static void sparse_file_write_block(struct output_file *out,
		struct backed_block *bb)
{
	switch (backed_block_type(bb)) {
	case BACKED_BLOCK_DATA:
		write_data_chunk(out, backed_block_len(bb), backed_block_data(bb));
		break;
	case BACKED_BLOCK_FILE:
		write_file_chunk(out, backed_block_len(bb),
				backed_block_filename(bb), backed_block_file_offset(bb));
		break;
	case BACKED_BLOCK_FD:
		write_fd_chunk(out, backed_block_len(bb),
				backed_block_fd(bb), backed_block_file_offset(bb));
		break;
	case BACKED_BLOCK_FILL:
		write_fill_chunk(out, backed_block_len(bb),
				backed_block_fill_val(bb));
		break;
	}
}

static int write_all_blocks(struct sparse_file *s, struct output_file *out)
{
	struct backed_block *bb;
	unsigned int last_block = 0;
	int64_t pad;

	for (bb = backed_block_iter_new(s->backed_block_list); bb;
			bb = backed_block_iter_next(bb)) {
		if (backed_block_block(bb) > last_block) {
			unsigned int blocks = backed_block_block(bb) - last_block;
			write_skip_chunk(out, (int64_t)blocks * s->block_size);
		}
		sparse_file_write_block(out, bb);
		last_block = backed_block_block(bb) +
				DIV_ROUND_UP(backed_block_len(bb), s->block_size);
	}

	pad = s->len - (int64_t)last_block * s->block_size;
	assert(pad >= 0);
	if (pad > 0) {
		write_skip_chunk(out, pad);
	}

	return 0;
}

int sparse_file_write(struct sparse_file *s, int fd, bool gz, bool sparse,
		bool crc)
{
	int ret;
	int chunks;
	struct output_file *out;

	chunks = sparse_count_chunks(s);
	out = output_file_open_fd(fd, s->block_size, s->len, gz, sparse, chunks, crc);

	if (!out)
		return -ENOMEM;

	ret = write_all_blocks(s, out);

	output_file_close(out);

	return ret;
}

int sparse_file_callback(struct sparse_file *s, bool sparse, bool crc,
		int (*write)(void *priv, const void *data, int len), void *priv)
{
	int ret;
	int chunks;
	struct output_file *out;

	chunks = sparse_count_chunks(s);
	out = output_file_open_callback(write, priv, s->block_size, s->len, false,
			sparse, chunks, crc);

	if (!out)
		return -ENOMEM;

	ret = write_all_blocks(s, out);

	output_file_close(out);

	return ret;
}

static int out_counter_write(void *priv, const void *data, int len)
{
	int64_t *count = priv;
	*count += len;
	return 0;
}

int64_t sparse_file_len(struct sparse_file *s, bool sparse, bool crc)
{
	int ret;
	int chunks = sparse_count_chunks(s);
	int64_t count = 0;
	struct output_file *out;

	out = output_file_open_callback(out_counter_write, &count,
			s->block_size, s->len, false, sparse, chunks, crc);
	if (!out) {
		return -1;
	}

	ret = write_all_blocks(s, out);

	output_file_close(out);

	if (ret < 0) {
		return -1;
	}

	return count;
}

static struct backed_block *move_chunks_up_to_len(struct sparse_file *from,
		struct sparse_file *to, unsigned int len)
{
	int64_t count = 0;
	struct output_file *out_counter;
	struct backed_block *last_bb = NULL;
	struct backed_block *bb;
	struct backed_block *start;
	int64_t file_len = 0;

	/*
	 * overhead is sparse file header, initial skip chunk, split chunk, end
	 * skip chunk, and crc chunk.
	 */
	int overhead = sizeof(sparse_header_t) + 4 * sizeof(chunk_header_t) +
			sizeof(uint32_t);
	len -= overhead;

	start = backed_block_iter_new(from->backed_block_list);
	out_counter = output_file_open_callback(out_counter_write, &count,
			to->block_size, to->len, false, true, 0, false);
	if (!out_counter) {
		return NULL;
	}

	for (bb = start; bb; bb = backed_block_iter_next(bb)) {
		count = 0;
		/* will call out_counter_write to update count */
		sparse_file_write_block(out_counter, bb);
		if (file_len + count > len) {
			/*
			 * If the remaining available size is more than 1/8th of the
			 * requested size, split the chunk.  Results in sparse files that
			 * are at least 7/8ths of the requested size
			 */
			if (!last_bb || (len - file_len > (len / 8))) {
				backed_block_split(from->backed_block_list, bb, len - file_len);
				last_bb = bb;
			}
			goto out;
		}
		file_len += count;
		last_bb = bb;
	}

out:
	backed_block_list_move(from->backed_block_list,
		to->backed_block_list, start, last_bb);

	output_file_close(out_counter);

	return bb;
}

int sparse_file_resparse(struct sparse_file *in_s, unsigned int max_len,
		struct sparse_file **out_s, int out_s_count)
{
	struct backed_block *bb;
	unsigned int overhead;
	struct sparse_file *s;
	struct sparse_file *tmp;
	int c = 0;

	tmp = sparse_file_new(in_s->block_size, in_s->len);
	if (!tmp) {
		return -ENOMEM;
	}

	do {
		s = sparse_file_new(in_s->block_size, in_s->len);

		bb = move_chunks_up_to_len(in_s, s, max_len);

		if (c < out_s_count) {
			out_s[c] = s;
		} else {
			backed_block_list_move(s->backed_block_list, tmp->backed_block_list,
					NULL, NULL);
			sparse_file_destroy(s);
		}
		c++;
	} while (bb);

	backed_block_list_move(tmp->backed_block_list, in_s->backed_block_list,
			NULL, NULL);

	sparse_file_destroy(tmp);

	return c;
}

void sparse_file_verbose(struct sparse_file *s)
{
	s->verbose = true;
}
