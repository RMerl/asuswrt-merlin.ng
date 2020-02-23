/*
 * Copyright (C) 2010 The Android Open Source Project
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
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "backed_block.h"
#include "sparse_defs.h"

struct backed_block {
	unsigned int block;
	unsigned int len;
	enum backed_block_type type;
	union {
		struct {
			void *data;
		} data;
		struct {
			char *filename;
			int64_t offset;
		} file;
		struct {
			int fd;
			int64_t offset;
		} fd;
		struct {
			uint32_t val;
		} fill;
	};
	struct backed_block *next;
};

struct backed_block_list {
	struct backed_block *data_blocks;
	struct backed_block *last_used;
	unsigned int block_size;
};

struct backed_block *backed_block_iter_new(struct backed_block_list *bbl)
{
	return bbl->data_blocks;
}

struct backed_block *backed_block_iter_next(struct backed_block *bb)
{
	return bb->next;
}

unsigned int backed_block_len(struct backed_block *bb)
{
	return bb->len;
}

unsigned int backed_block_block(struct backed_block *bb)
{
	return bb->block;
}

void *backed_block_data(struct backed_block *bb)
{
	assert(bb->type == BACKED_BLOCK_DATA);
	return bb->data.data;
}

const char *backed_block_filename(struct backed_block *bb)
{
	assert(bb->type == BACKED_BLOCK_FILE);
	return bb->file.filename;
}

int backed_block_fd(struct backed_block *bb)
{
	assert(bb->type == BACKED_BLOCK_FD);
	return bb->fd.fd;
}

int64_t backed_block_file_offset(struct backed_block *bb)
{
	assert(bb->type == BACKED_BLOCK_FILE || bb->type == BACKED_BLOCK_FD);
	if (bb->type == BACKED_BLOCK_FILE) {
		return bb->file.offset;
	} else { /* bb->type == BACKED_BLOCK_FD */
		return bb->fd.offset;
	}
}

uint32_t backed_block_fill_val(struct backed_block *bb)
{
	assert(bb->type == BACKED_BLOCK_FILL);
	return bb->fill.val;
}

enum backed_block_type backed_block_type(struct backed_block *bb)
{
	return bb->type;
}

void backed_block_destroy(struct backed_block *bb)
{
	if (bb->type == BACKED_BLOCK_FILE) {
		free(bb->file.filename);
	}

	free(bb);
}

struct backed_block_list *backed_block_list_new(unsigned int block_size)
{
	struct backed_block_list *b = calloc(sizeof(struct backed_block_list), 1);
	b->block_size = block_size;
	return b;
}

void backed_block_list_destroy(struct backed_block_list *bbl)
{
	if (bbl->data_blocks) {
		struct backed_block *bb = bbl->data_blocks;
		while (bb) {
			struct backed_block *next = bb->next;
			backed_block_destroy(bb);
			bb = next;
		}
	}

	free(bbl);
}

void backed_block_list_move(struct backed_block_list *from,
		struct backed_block_list *to, struct backed_block *start,
		struct backed_block *end)
{
	struct backed_block *bb;

	if (start == NULL) {
		start = from->data_blocks;
	}

	if (!end) {
		for (end = start; end && end->next; end = end->next)
			;
	}

	if (start == NULL || end == NULL) {
		return;
	}

	from->last_used = NULL;
	to->last_used = NULL;
	if (from->data_blocks == start) {
		from->data_blocks = end->next;
	} else {
		for (bb = from->data_blocks; bb; bb = bb->next) {
			if (bb->next == start) {
				bb->next = end->next;
				break;
			}
		}
	}

	if (!to->data_blocks) {
		to->data_blocks = start;
		end->next = NULL;
	} else {
		for (bb = to->data_blocks; bb; bb = bb->next) {
			if (!bb->next || bb->next->block > start->block) {
				end->next = bb->next;
				bb->next = start;
				break;
			}
		}
	}
}

/* may free b */
static int merge_bb(struct backed_block_list *bbl,
		struct backed_block *a, struct backed_block *b)
{
	unsigned int block_len;

	/* Block doesn't exist (possible if one block is the last block) */
	if (!a || !b) {
		return -EINVAL;
	}

	assert(a->block < b->block);

	/* Blocks are of different types */
	if (a->type != b->type) {
		return -EINVAL;
	}

	/* Blocks are not adjacent */
	block_len = a->len / bbl->block_size; /* rounds down */
	if (a->block + block_len != b->block) {
		return -EINVAL;
	}

	switch (a->type) {
	case BACKED_BLOCK_DATA:
		/* Don't support merging data for now */
		return -EINVAL;
	case BACKED_BLOCK_FILL:
		if (a->fill.val != b->fill.val) {
			return -EINVAL;
		}
		break;
	case BACKED_BLOCK_FILE:
		if (a->file.filename != b->file.filename ||
				a->file.offset + a->len != b->file.offset) {
			return -EINVAL;
		}
		break;
	case BACKED_BLOCK_FD:
		if (a->fd.fd != b->fd.fd ||
				a->fd.offset + a->len != b->fd.offset) {
			return -EINVAL;
		}
		break;
	}

	/* Blocks are compatible and adjacent, with a before b.  Merge b into a,
	 * and free b */
	a->len += b->len;
	a->next = b->next;

	backed_block_destroy(b);

	return 0;
}

static int queue_bb(struct backed_block_list *bbl, struct backed_block *new_bb)
{
	struct backed_block *bb;

	if (bbl->data_blocks == NULL) {
		bbl->data_blocks = new_bb;
		return 0;
	}

	if (bbl->data_blocks->block > new_bb->block) {
		new_bb->next = bbl->data_blocks;
		bbl->data_blocks = new_bb;
		return 0;
	}

	/* Optimization: blocks are mostly queued in sequence, so save the
	   pointer to the last bb that was added, and start searching from
	   there if the next block number is higher */
	if (bbl->last_used && new_bb->block > bbl->last_used->block)
		bb = bbl->last_used;
	else
		bb = bbl->data_blocks;
	bbl->last_used = new_bb;

	for (; bb->next && bb->next->block < new_bb->block; bb = bb->next)
		;

	if (bb->next == NULL) {
		bb->next = new_bb;
	} else {
		new_bb->next = bb->next;
		bb->next = new_bb;
	}

	merge_bb(bbl, new_bb, new_bb->next);
	merge_bb(bbl, bb, new_bb);

	return 0;
}

/* Queues a fill block of memory to be written to the specified data blocks */
int backed_block_add_fill(struct backed_block_list *bbl, unsigned int fill_val,
		unsigned int len, unsigned int block)
{
	struct backed_block *bb = calloc(1, sizeof(struct backed_block));
	if (bb == NULL) {
		return -ENOMEM;
	}

	bb->block = block;
	bb->len = len;
	bb->type = BACKED_BLOCK_FILL;
	bb->fill.val = fill_val;
	bb->next = NULL;

	return queue_bb(bbl, bb);
}

/* Queues a block of memory to be written to the specified data blocks */
int backed_block_add_data(struct backed_block_list *bbl, void *data,
		unsigned int len, unsigned int block)
{
	struct backed_block *bb = calloc(1, sizeof(struct backed_block));
	if (bb == NULL) {
		return -ENOMEM;
	}

	bb->block = block;
	bb->len = len;
	bb->type = BACKED_BLOCK_DATA;
	bb->data.data = data;
	bb->next = NULL;

	return queue_bb(bbl, bb);
}

/* Queues a chunk of a file on disk to be written to the specified data blocks */
int backed_block_add_file(struct backed_block_list *bbl, const char *filename,
		int64_t offset, unsigned int len, unsigned int block)
{
	struct backed_block *bb = calloc(1, sizeof(struct backed_block));
	if (bb == NULL) {
		return -ENOMEM;
	}

	bb->block = block;
	bb->len = len;
	bb->type = BACKED_BLOCK_FILE;
	bb->file.filename = strdup(filename);
	bb->file.offset = offset;
	bb->next = NULL;

	return queue_bb(bbl, bb);
}

/* Queues a chunk of a fd to be written to the specified data blocks */
int backed_block_add_fd(struct backed_block_list *bbl, int fd, int64_t offset,
		unsigned int len, unsigned int block)
{
	struct backed_block *bb = calloc(1, sizeof(struct backed_block));
	if (bb == NULL) {
		return -ENOMEM;
	}

	bb->block = block;
	bb->len = len;
	bb->type = BACKED_BLOCK_FD;
	bb->fd.fd = fd;
	bb->fd.offset = offset;
	bb->next = NULL;

	return queue_bb(bbl, bb);
}

int backed_block_split(struct backed_block_list *bbl, struct backed_block *bb,
		unsigned int max_len)
{
	struct backed_block *new_bb;

	max_len = ALIGN_DOWN(max_len, bbl->block_size);

	if (bb->len <= max_len) {
		return 0;
	}

	new_bb = malloc(sizeof(struct backed_block));
	if (bb == NULL) {
		return -ENOMEM;
	}

	*new_bb = *bb;

	new_bb->len = bb->len - max_len;
	new_bb->block = bb->block + max_len / bbl->block_size;
	new_bb->next = bb->next;
	bb->next = new_bb;
	bb->len = max_len;

	switch (bb->type) {
	case BACKED_BLOCK_DATA:
		new_bb->data.data = (char *)bb->data.data + max_len;
		break;
	case BACKED_BLOCK_FILE:
		new_bb->file.offset += max_len;
		break;
	case BACKED_BLOCK_FD:
		new_bb->fd.offset += max_len;
		break;
	case BACKED_BLOCK_FILL:
		break;
	}

	return 0;
}
