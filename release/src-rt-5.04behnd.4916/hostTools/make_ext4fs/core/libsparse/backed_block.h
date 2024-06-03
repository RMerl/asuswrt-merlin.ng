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

#ifndef _BACKED_BLOCK_H_
#define _BACKED_BLOCK_H_

#include <stdint.h>

struct backed_block_list;
struct backed_block;

enum backed_block_type {
	BACKED_BLOCK_DATA,
	BACKED_BLOCK_FILE,
	BACKED_BLOCK_FD,
	BACKED_BLOCK_FILL,
};

int backed_block_add_data(struct backed_block_list *bbl, void *data,
		unsigned int len, unsigned int block);
int backed_block_add_fill(struct backed_block_list *bbl, unsigned int fill_val,
		unsigned int len, unsigned int block);
int backed_block_add_file(struct backed_block_list *bbl, const char *filename,
		int64_t offset, unsigned int len, unsigned int block);
int backed_block_add_fd(struct backed_block_list *bbl, int fd,
		int64_t offset, unsigned int len, unsigned int block);

struct backed_block *backed_block_iter_new(struct backed_block_list *bbl);
struct backed_block *backed_block_iter_next(struct backed_block *bb);
unsigned int backed_block_len(struct backed_block *bb);
unsigned int backed_block_block(struct backed_block *bb);
void *backed_block_data(struct backed_block *bb);
const char *backed_block_filename(struct backed_block *bb);
int backed_block_fd(struct backed_block *bb);
int64_t backed_block_file_offset(struct backed_block *bb);
uint32_t backed_block_fill_val(struct backed_block *bb);
enum backed_block_type backed_block_type(struct backed_block *bb);
int backed_block_split(struct backed_block_list *bbl, struct backed_block *bb,
		unsigned int max_len);

struct backed_block *backed_block_iter_new(struct backed_block_list *bbl);
struct backed_block *backed_block_iter_next(struct backed_block *bb);

struct backed_block_list *backed_block_list_new(unsigned int block_size);
void backed_block_list_destroy(struct backed_block_list *bbl);

void backed_block_list_move(struct backed_block_list *from,
		struct backed_block_list *to, struct backed_block *start,
		struct backed_block *end);

#endif
