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

#ifndef _OUTPUT_FILE_H_
#define _OUTPUT_FILE_H_

#include <sparse/sparse.h>

struct output_file;

struct output_file *output_file_open_fd(int fd, unsigned int block_size, int64_t len,
		int gz, int sparse, int chunks, int crc);
struct output_file *output_file_open_callback(int (*write)(void *, const void *, int),
		void *priv, unsigned int block_size, int64_t len, int gz, int sparse,
		int chunks, int crc);
int write_data_chunk(struct output_file *out, unsigned int len, void *data);
int write_fill_chunk(struct output_file *out, unsigned int len,
		uint32_t fill_val);
int write_file_chunk(struct output_file *out, unsigned int len,
		const char *file, int64_t offset);
int write_fd_chunk(struct output_file *out, unsigned int len,
		int fd, int64_t offset);
int write_skip_chunk(struct output_file *out, int64_t len);
void output_file_close(struct output_file *out);

int read_all(int fd, void *buf, size_t len);

#endif
