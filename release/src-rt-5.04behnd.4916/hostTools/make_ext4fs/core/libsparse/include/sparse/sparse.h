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

#ifndef _LIBSPARSE_SPARSE_H_
#define _LIBSPARSE_SPARSE_H_

#include <stdbool.h>
#include <stdint.h>

struct sparse_file;

/**
 * sparse_file_new - create a new sparse file cookie
 *
 * @block_size - minimum size of a chunk
 * @len - size of the expanded sparse file.
 *
 * Creates a new sparse_file cookie that can be used to associate data
 * blocks.  Can later be written to a file with a variety of options.
 * block_size specifies the minimum size of a chunk in the file.  The maximum
 * size of the file is 2**32 * block_size (16TB for 4k block size).
 *
 * Returns the sparse file cookie, or NULL on error.
 */
struct sparse_file *sparse_file_new(unsigned int block_size, int64_t len);

/**
 * sparse_file_destroy - destroy a sparse file cookie
 *
 * @s - sparse file cookie
 *
 * Destroys a sparse file cookie.  After destroy, all memory passed in to
 * sparse_file_add_data can be freed by the caller
 */
void sparse_file_destroy(struct sparse_file *s);

/**
 * sparse_file_add_data - associate a data chunk with a sparse file
 *
 * @s - sparse file cookie
 * @data - pointer to data block
 * @len - length of the data block
 * @block - offset in blocks into the sparse file to place the data chunk
 *
 * Associates a data chunk with a sparse file cookie.  The region
 * [block * block_size : block * block_size + len) must not already be used in
 * the sparse file. If len is not a multiple of the block size the data
 * will be padded with zeros.
 *
 * The data pointer must remain valid until the sparse file is closed or the
 * data block is removed from the sparse file.
 *
 * Returns 0 on success, negative errno on error.
 */
int sparse_file_add_data(struct sparse_file *s,
		void *data, unsigned int len, unsigned int block);

/**
 * sparse_file_add_fill - associate a fill chunk with a sparse file
 *
 * @s - sparse file cookie
 * @fill_val - 32 bit fill data
 * @len - length of the fill block
 * @block - offset in blocks into the sparse file to place the fill chunk
 *
 * Associates a chunk filled with fill_val with a sparse file cookie.
 * The region [block * block_size : block * block_size + len) must not already
 * be used in the sparse file. If len is not a multiple of the block size the
 * data will be padded with zeros.
 *
 * Returns 0 on success, negative errno on error.
 */
int sparse_file_add_fill(struct sparse_file *s,
		uint32_t fill_val, unsigned int len, unsigned int block);

/**
 * sparse_file_add_file - associate a chunk of a file with a sparse file
 *
 * @s - sparse file cookie
 * @filename - filename of the file to be copied
 * @file_offset - offset into the copied file
 * @len - length of the copied block
 * @block - offset in blocks into the sparse file to place the file chunk
 *
 * Associates a chunk of an existing file with a sparse file cookie.
 * The region [block * block_size : block * block_size + len) must not already
 * be used in the sparse file. If len is not a multiple of the block size the
 * data will be padded with zeros.
 *
 * Allows adding large amounts of data to a sparse file without needing to keep
 * it all mapped.  File size is limited by available virtual address space,
 * exceptionally large files may need to be added in multiple chunks.
 *
 * Returns 0 on success, negative errno on error.
 */
int sparse_file_add_file(struct sparse_file *s,
		const char *filename, int64_t file_offset, unsigned int len,
		unsigned int block);

/**
 * sparse_file_add_file - associate a chunk of a file with a sparse file
 *
 * @s - sparse file cookie
 * @filename - filename of the file to be copied
 * @file_offset - offset into the copied file
 * @len - length of the copied block
 * @block - offset in blocks into the sparse file to place the file chunk
 *
 * Associates a chunk of an existing fd with a sparse file cookie.
 * The region [block * block_size : block * block_size + len) must not already
 * be used in the sparse file. If len is not a multiple of the block size the
 * data will be padded with zeros.
 *
 * Allows adding large amounts of data to a sparse file without needing to keep
 * it all mapped.  File size is limited by available virtual address space,
 * exceptionally large files may need to be added in multiple chunks.
 *
 * The fd must remain open until the sparse file is closed or the fd block is
 * removed from the sparse file.
 *
 * Returns 0 on success, negative errno on error.
 */
int sparse_file_add_fd(struct sparse_file *s,
		int fd, int64_t file_offset, unsigned int len, unsigned int block);

/**
 * sparse_file_write - write a sparse file to a file
 *
 * @s - sparse file cookie
 * @fd - file descriptor to write to
 * @gz - write a gzipped file
 * @sparse - write in the Android sparse file format
 * @crc - append a crc chunk
 *
 * Writes a sparse file to a file.  If gz is true, the data will be passed
 * through zlib.  If sparse is true, the file will be written in the Android
 * sparse file format.  If sparse is false, the file will be written by seeking
 * over unused chunks, producing a smaller file if the filesystem supports
 * sparse files.  If crc is true, the crc of the expanded data will be
 * calculated and appended in a crc chunk.
 *
 * Returns 0 on success, negative errno on error.
 */
int sparse_file_write(struct sparse_file *s, int fd, bool gz, bool sparse,
		bool crc);

/**
 * sparse_file_len - return the length of a sparse file if written to disk
 *
 * @s - sparse file cookie
 * @sparse - write in the Android sparse file format
 * @crc - append a crc chunk
 *
 * Returns the size a sparse file would be on disk if it were written in the
 * specified format.  If sparse is true, this is the size of the data in the
 * sparse format.  If sparse is false, this is the size of the normal
 * non-sparse file.
 */
int64_t sparse_file_len(struct sparse_file *s, bool sparse, bool crc);

/**
 * sparse_file_callback - call a callback for blocks in sparse file
 *
 * @s - sparse file cookie
 * @sparse - write in the Android sparse file format
 * @crc - append a crc chunk
 * @write - function to call for each block
 * @priv - value that will be passed as the first argument to write
 *
 * Writes a sparse file by calling a callback function.  If sparse is true, the
 * file will be written in the Android sparse file format.  If crc is true, the
 * crc of the expanded data will be calculated and appended in a crc chunk.
 * The callback 'write' will be called with data and length for each data,
 * and with data==NULL to skip over a region (only used for non-sparse format).
 * The callback should return negative on error, 0 on success.
 *
 * Returns 0 on success, negative errno on error.
 */
int sparse_file_callback(struct sparse_file *s, bool sparse, bool crc,
		int (*write)(void *priv, const void *data, int len), void *priv);

/**
 * sparse_file_read - read a file into a sparse file cookie
 *
 * @s - sparse file cookie
 * @fd - file descriptor to read from
 * @sparse - read a file in the Android sparse file format
 * @crc - verify the crc of a file in the Android sparse file format
 *
 * Reads a file into a sparse file cookie.  If sparse is true, the file is
 * assumed to be in the Android sparse file format.  If sparse is false, the
 * file will be sparsed by looking for block aligned chunks of all zeros or
 * another 32 bit value.  If crc is true, the crc of the sparse file will be
 * verified.
 *
 * Returns 0 on success, negative errno on error.
 */
int sparse_file_read(struct sparse_file *s, int fd, bool sparse, bool crc);

/**
 * sparse_file_import - import an existing sparse file
 *
 * @s - sparse file cookie
 * @verbose - print verbose errors while reading the sparse file
 * @crc - verify the crc of a file in the Android sparse file format
 *
 * Reads an existing sparse file into a sparse file cookie, recreating the same
 * sparse cookie that was used to write it.  If verbose is true, prints verbose
 * errors when the sparse file is formatted incorrectly.
 *
 * Returns a new sparse file cookie on success, NULL on error.
 */
struct sparse_file *sparse_file_import(int fd, bool verbose, bool crc);

/**
 * sparse_file_import_auto - import an existing sparse or normal file
 *
 * @fd - file descriptor to read from
 * @crc - verify the crc of a file in the Android sparse file format
 *
 * Reads an existing sparse or normal file into a sparse file cookie.
 * Attempts to determine if the file is sparse or not by looking for the sparse
 * file magic number in the first 4 bytes.  If the file is not sparse, the file
 * will be sparsed by looking for block aligned chunks of all zeros or another
 * 32 bit value.  If crc is true, the crc of the sparse file will be verified.
 *
 * Returns a new sparse file cookie on success, NULL on error.
 */
struct sparse_file *sparse_file_import_auto(int fd, bool crc);

/** sparse_file_resparse - rechunk an existing sparse file into smaller files
 *
 * @in_s - sparse file cookie of the existing sparse file
 * @max_len - maximum file size
 * @out_s - array of sparse file cookies
 * @out_s_count - size of out_s array
 *
 * Splits chunks of an existing sparse file into smaller sparse files such that
 * each sparse file is less than max_len.  Returns the number of sparse_files
 * that would have been written to out_s if out_s were big enough.
 */
int sparse_file_resparse(struct sparse_file *in_s, unsigned int max_len,
		struct sparse_file **out_s, int out_s_count);

/**
 * sparse_file_verbose - set a sparse file cookie to print verbose errors
 *
 * @s - sparse file cookie
 *
 * Print verbose sparse file errors whenever using the sparse file cookie.
 */
void sparse_file_verbose(struct sparse_file *s);

/**
 * sparse_print_verbose - function called to print verbose errors
 *
 * By default, verbose errors will print to standard error.
 * sparse_print_verbose may be overridden to log verbose errors somewhere else.
 *
 */
extern void (*sparse_print_verbose)(const char *fmt, ...);

#endif
