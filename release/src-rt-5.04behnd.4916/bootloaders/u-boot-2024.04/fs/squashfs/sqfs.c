// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2020 Bootlin
 *
 * Author: Joao Marcos Costa <joaomarcos.costa@bootlin.com>
 *
 * sqfs.c: SquashFS filesystem implementation
 */

#include <asm/unaligned.h>
#include <div64.h>
#include <errno.h>
#include <fs.h>
#include <linux/types.h>
#include <asm/byteorder.h>
#include <linux/compat.h>
#include <memalign.h>
#include <stdlib.h>
#include <string.h>
#include <squashfs.h>
#include <part.h>

#include "sqfs_decompressor.h"
#include "sqfs_filesystem.h"
#include "sqfs_utils.h"

static struct squashfs_ctxt ctxt;

static int sqfs_disk_read(__u32 block, __u32 nr_blocks, void *buf)
{
	ulong ret;

	if (!ctxt.cur_dev)
		return -1;

	ret = blk_dread(ctxt.cur_dev, ctxt.cur_part_info.start + block,
			nr_blocks, buf);

	if (ret != nr_blocks)
		return -1;

	return ret;
}

static int sqfs_read_sblk(struct squashfs_super_block **sblk)
{
	*sblk = malloc_cache_aligned(ctxt.cur_dev->blksz);
	if (!*sblk)
		return -ENOMEM;

	if (sqfs_disk_read(0, 1, *sblk) != 1) {
		free(*sblk);
		*sblk = NULL;
		return -EINVAL;
	}

	return 0;
}

static int sqfs_count_tokens(const char *filename)
{
	int token_count = 1, l;

	for (l = 1; l < strlen(filename); l++) {
		if (filename[l] == '/')
			token_count++;
	}

	/* Ignore trailing '/' in path */
	if (filename[strlen(filename) - 1] == '/')
		token_count--;

	if (!token_count)
		token_count = 1;

	return token_count;
}

/*
 * Calculates how many blocks are needed for the buffer used in sqfs_disk_read.
 * The memory section (e.g. inode table) start offset and its end (i.e. the next
 * table start) must be specified. It also calculates the offset from which to
 * start reading the buffer.
 */
static int sqfs_calc_n_blks(__le64 start, __le64 end, u64 *offset)
{
	u64 start_, table_size;

	table_size = le64_to_cpu(end) - le64_to_cpu(start);
	start_ = lldiv(le64_to_cpu(start), ctxt.cur_dev->blksz);
	*offset = le64_to_cpu(start) - (start_ * ctxt.cur_dev->blksz);

	return DIV_ROUND_UP(table_size + *offset, ctxt.cur_dev->blksz);
}

/*
 * Retrieves fragment block entry and returns true if the fragment block is
 * compressed
 */
static int sqfs_frag_lookup(u32 inode_fragment_index,
			    struct squashfs_fragment_block_entry *e)
{
	u64 start, end, exp_tbl, n_blks, src_len, table_offset, start_block;
	unsigned char *metadata_buffer, *metadata, *table;
	struct squashfs_fragment_block_entry *entries;
	struct squashfs_super_block *sblk = ctxt.sblk;
	unsigned long dest_len;
	int block, offset, ret;
	u16 header;

	metadata_buffer = NULL;
	entries = NULL;
	table = NULL;

	if (inode_fragment_index >= get_unaligned_le32(&sblk->fragments))
		return -EINVAL;

	start = get_unaligned_le64(&sblk->fragment_table_start);
	end = get_unaligned_le64(&sblk->id_table_start);
	exp_tbl = get_unaligned_le64(&sblk->export_table_start);

	if (exp_tbl > start && exp_tbl < end)
		end = exp_tbl;

	n_blks = sqfs_calc_n_blks(sblk->fragment_table_start,
				  cpu_to_le64(end), &table_offset);

	start /= ctxt.cur_dev->blksz;

	/* Allocate a proper sized buffer to store the fragment index table */
	table = malloc_cache_aligned(n_blks * ctxt.cur_dev->blksz);
	if (!table) {
		ret = -ENOMEM;
		goto out;
	}

	if (sqfs_disk_read(start, n_blks, table) < 0) {
		ret = -EINVAL;
		goto out;
	}

	block = SQFS_FRAGMENT_INDEX(inode_fragment_index);
	offset = SQFS_FRAGMENT_INDEX_OFFSET(inode_fragment_index);

	/*
	 * Get the start offset of the metadata block that contains the right
	 * fragment block entry
	 */
	start_block = get_unaligned_le64(table + table_offset + block *
					 sizeof(u64));

	start = start_block / ctxt.cur_dev->blksz;
	n_blks = sqfs_calc_n_blks(cpu_to_le64(start_block),
				  sblk->fragment_table_start, &table_offset);

	metadata_buffer = malloc_cache_aligned(n_blks * ctxt.cur_dev->blksz);
	if (!metadata_buffer) {
		ret = -ENOMEM;
		goto out;
	}

	if (sqfs_disk_read(start, n_blks, metadata_buffer) < 0) {
		ret = -EINVAL;
		goto out;
	}

	/* Every metadata block starts with a 16-bit header */
	header = get_unaligned_le16(metadata_buffer + table_offset);
	metadata = metadata_buffer + table_offset + SQFS_HEADER_SIZE;

	if (!metadata || !header) {
		ret = -ENOMEM;
		goto out;
	}

	entries = malloc(SQFS_METADATA_BLOCK_SIZE);
	if (!entries) {
		ret = -ENOMEM;
		goto out;
	}

	if (SQFS_COMPRESSED_METADATA(header)) {
		src_len = SQFS_METADATA_SIZE(header);
		dest_len = SQFS_METADATA_BLOCK_SIZE;
		ret = sqfs_decompress(&ctxt, entries, &dest_len, metadata,
				      src_len);
		if (ret) {
			ret = -EINVAL;
			goto out;
		}
	} else {
		memcpy(entries, metadata, SQFS_METADATA_SIZE(header));
	}

	*e = entries[offset];
	ret = SQFS_COMPRESSED_BLOCK(e->size);

out:
	free(entries);
	free(metadata_buffer);
	free(table);

	return ret;
}

/*
 * The entry name is a flexible array member, and we don't know its size before
 * actually reading the entry. So we need a first copy to retrieve this size so
 * we can finally copy the whole struct.
 */
static int sqfs_read_entry(struct squashfs_directory_entry **dest, void *src)
{
	struct squashfs_directory_entry *tmp;
	u16 sz;

	tmp = src;
	sz = get_unaligned_le16(src + sizeof(*tmp) - sizeof(u16));
	/*
	 * 'src' points to the begin of a directory entry, and 'sz' gets its
	 * 'name_size' member's value. name_size is actually the string
	 * length - 1, so adding 2 compensates this difference and adds space
	 * for the trailling null byte.
	 */
	*dest = malloc(sizeof(*tmp) + sz + 2);
	if (!*dest)
		return -ENOMEM;

	memcpy(*dest, src, sizeof(*tmp) + sz + 1);
	(*dest)->name[sz + 1] = '\0';

	return 0;
}

static int sqfs_get_tokens_length(char **tokens, int count)
{
	int length = 0, i;

	/*
	 * 1 is added to the result of strlen to consider the slash separator
	 * between the tokens.
	 */
	for (i = 0; i < count; i++)
		length += strlen(tokens[i]) + 1;

	return length;
}

/* Takes a token list and returns a single string with '/' as separator. */
static char *sqfs_concat_tokens(char **token_list, int token_count)
{
	char *result;
	int i, length = 0, offset = 0;

	length = sqfs_get_tokens_length(token_list, token_count);

	result = malloc(length + 1);
	if (!result)
		return NULL;

	result[length] = '\0';

	for (i = 0; i < token_count; i++) {
		strcpy(result + offset, token_list[i]);
		offset += strlen(token_list[i]);
		result[offset++] = '/';
	}

	return result;
}

/*
 * Differently from sqfs_concat_tokens, sqfs_join writes the result into a
 * previously allocated string, and returns the number of bytes written.
 */
static int sqfs_join(char **strings, char *dest, int start, int end,
		     char separator)
{
	int i, offset = 0;

	for (i = start; i < end; i++) {
		strcpy(dest + offset, strings[i]);
		offset += strlen(strings[i]);
		if (i < end - 1)
			dest[offset++] = separator;
	}

	return offset;
}

/*
 * Fills the given token list using its size (count) and a source string (str)
 */
static int sqfs_tokenize(char **tokens, int count, const char *str)
{
	int i, j, ret = 0;
	char *aux, *strc;

	strc = strdup(str);
	if (!strc)
		return -ENOMEM;

	if (!strcmp(strc, "/")) {
		tokens[0] = strdup(strc);
		if (!tokens[0]) {
			ret = -ENOMEM;
			goto free_strc;
		}
	} else {
		for (j = 0; j < count; j++) {
			aux = strtok(!j ? strc : NULL, "/");
			tokens[j] = strdup(aux);
			if (!tokens[j]) {
				for (i = 0; i < j; i++)
					free(tokens[i]);
				ret = -ENOMEM;
				goto free_strc;
			}
		}
	}

free_strc:
	free(strc);

	return ret;
}

/*
 * Remove last 'updir + 1' tokens from the base path tokens list. This leaves us
 * with a token list containing only the tokens needed to form the resolved
 * path, and returns the decremented size of the token list.
 */
static int sqfs_clean_base_path(char **base, int count, int updir)
{
	int i;

	for (i = count - updir - 1; i < count; i++)
		free(base[i]);

	return count - updir - 1;
}

/*
 * Given the base ("current dir.") path and the relative one, generate the
 * absolute path.
 */
static char *sqfs_get_abs_path(const char *base, const char *rel)
{
	char **base_tokens, **rel_tokens, *resolved = NULL;
	int ret, bc, rc, i, updir = 0, resolved_size = 0, offset = 0;

	base_tokens = NULL;
	rel_tokens = NULL;

	/* Memory allocation for the token lists */
	bc = sqfs_count_tokens(base);
	rc = sqfs_count_tokens(rel);
	if (bc < 1 || rc < 1)
		return NULL;

	base_tokens = calloc(bc, sizeof(char *));
	if (!base_tokens)
		return NULL;

	rel_tokens = calloc(rc, sizeof(char *));
	if (!rel_tokens)
		goto out;

	/* Fill token lists */
	ret = sqfs_tokenize(base_tokens, bc, base);
	if (ret)
		goto out;

	ret = sqfs_tokenize(rel_tokens, rc, rel);
	if (ret)
		goto out;

	/* count '..' occurrences in target path */
	for (i = 0; i < rc; i++) {
		if (!strcmp(rel_tokens[i], ".."))
			updir++;
	}

	/* Remove the last token and the '..' occurrences */
	bc = sqfs_clean_base_path(base_tokens, bc, updir);
	if (bc < 0)
		goto out;

	/* Calculate resolved path size */
	if (!bc)
		resolved_size++;

	resolved_size += sqfs_get_tokens_length(base_tokens, bc) +
		sqfs_get_tokens_length(rel_tokens, rc);

	resolved = malloc(resolved_size + 1);
	if (!resolved)
		goto out;

	/* Set resolved path */
	memset(resolved, '\0', resolved_size + 1);
	offset += sqfs_join(base_tokens, resolved + offset, 0, bc, '/');
	resolved[offset++] = '/';
	offset += sqfs_join(rel_tokens, resolved + offset, updir, rc, '/');

out:
	if (rel_tokens)
		for (i = 0; i < rc; i++)
			free(rel_tokens[i]);
	if (base_tokens)
		for (i = 0; i < bc; i++)
			free(base_tokens[i]);

	free(rel_tokens);
	free(base_tokens);

	return resolved;
}

static char *sqfs_resolve_symlink(struct squashfs_symlink_inode *sym,
				  const char *base_path)
{
	char *resolved, *target;
	u32 sz;

	sz = get_unaligned_le32(&sym->symlink_size);
	target = malloc(sz + 1);
	if (!target)
		return NULL;

	/*
	 * There is no trailling null byte in the symlink's target path, so a
	 * copy is made and a '\0' is added at its end.
	 */
	target[sz] = '\0';
	/* Get target name (relative path) */
	strncpy(target, sym->symlink, sz);

	/* Relative -> absolute path conversion */
	resolved = sqfs_get_abs_path(base_path, target);

	free(target);

	return resolved;
}

/*
 * m_list contains each metadata block's position, and m_count is the number of
 * elements of m_list. Those metadata blocks come from the compressed directory
 * table.
 */
static int sqfs_search_dir(struct squashfs_dir_stream *dirs, char **token_list,
			   int token_count, u32 *m_list, int m_count)
{
	struct squashfs_super_block *sblk = ctxt.sblk;
	char *path, *target, **sym_tokens, *res, *rem;
	int j, ret = 0, new_inode_number, offset;
	struct squashfs_symlink_inode *sym;
	struct squashfs_ldir_inode *ldir;
	struct squashfs_dir_inode *dir;
	struct fs_dir_stream *dirsp;
	struct fs_dirent *dent;
	unsigned char *table;

	res = NULL;
	rem = NULL;
	path = NULL;
	target = NULL;
	sym_tokens = NULL;

	dirsp = (struct fs_dir_stream *)dirs;

	/* Start by root inode */
	table = sqfs_find_inode(dirs->inode_table, le32_to_cpu(sblk->inodes),
				sblk->inodes, sblk->block_size);

	dir = (struct squashfs_dir_inode *)table;
	ldir = (struct squashfs_ldir_inode *)table;

	/* get directory offset in directory table */
	offset = sqfs_dir_offset(table, m_list, m_count);
	dirs->table = &dirs->dir_table[offset];

	/* Setup directory header */
	dirs->dir_header = malloc(SQFS_DIR_HEADER_SIZE);
	if (!dirs->dir_header)
		return -ENOMEM;

	memcpy(dirs->dir_header, dirs->table, SQFS_DIR_HEADER_SIZE);

	/* Initialize squashfs_dir_stream members */
	dirs->table += SQFS_DIR_HEADER_SIZE;
	dirs->size = get_unaligned_le16(&dir->file_size) - SQFS_DIR_HEADER_SIZE;
	dirs->entry_count = dirs->dir_header->count + 1;

	/* No path given -> root directory */
	if (!strcmp(token_list[0], "/")) {
		dirs->table = &dirs->dir_table[offset];
		memcpy(&dirs->i_dir, dir, sizeof(*dir));
		return 0;
	}

	for (j = 0; j < token_count; j++) {
		if (!sqfs_is_dir(get_unaligned_le16(&dir->inode_type))) {
			printf("** Cannot find directory. **\n");
			ret = -EINVAL;
			goto out;
		}

		while (!sqfs_readdir(dirsp, &dent)) {
			ret = strcmp(dent->name, token_list[j]);
			if (!ret)
				break;
			free(dirs->entry);
			dirs->entry = NULL;
		}

		if (ret) {
			printf("** Cannot find directory. **\n");
			ret = -EINVAL;
			goto out;
		}

		/* Redefine inode as the found token */
		new_inode_number = dirs->entry->inode_offset +
			dirs->dir_header->inode_number;

		/* Get reference to inode in the inode table */
		table = sqfs_find_inode(dirs->inode_table, new_inode_number,
					sblk->inodes, sblk->block_size);
		dir = (struct squashfs_dir_inode *)table;

		/* Check for symbolic link and inode type sanity */
		if (get_unaligned_le16(&dir->inode_type) == SQFS_SYMLINK_TYPE) {
			sym = (struct squashfs_symlink_inode *)table;
			/* Get first j + 1 tokens */
			path = sqfs_concat_tokens(token_list, j + 1);
			if (!path) {
				ret = -ENOMEM;
				goto out;
			}
			/* Resolve for these tokens */
			target = sqfs_resolve_symlink(sym, path);
			if (!target) {
				ret = -ENOMEM;
				goto out;
			}
			/* Join remaining tokens */
			rem = sqfs_concat_tokens(token_list + j + 1, token_count -
						 j - 1);
			if (!rem) {
				ret = -ENOMEM;
				goto out;
			}
			/* Concatenate remaining tokens and symlink's target */
			res = malloc(strlen(rem) + strlen(target) + 1);
			if (!res) {
				ret = -ENOMEM;
				goto out;
			}
			strcpy(res, target);
			res[strlen(target)] = '/';
			strcpy(res + strlen(target) + 1, rem);
			token_count = sqfs_count_tokens(res);

			if (token_count < 0) {
				ret = -EINVAL;
				goto out;
			}

			sym_tokens = malloc(token_count * sizeof(char *));
			if (!sym_tokens) {
				ret = -EINVAL;
				goto out;
			}

			/* Fill tokens list */
			ret = sqfs_tokenize(sym_tokens, token_count, res);
			if (ret) {
				ret = -EINVAL;
				goto out;
			}
			free(dirs->entry);
			dirs->entry = NULL;

			ret = sqfs_search_dir(dirs, sym_tokens, token_count,
					      m_list, m_count);
			goto out;
		} else if (!sqfs_is_dir(get_unaligned_le16(&dir->inode_type))) {
			printf("** Cannot find directory. **\n");
			free(dirs->entry);
			dirs->entry = NULL;
			ret = -EINVAL;
			goto out;
		}

		/* Check if it is an extended dir. */
		if (get_unaligned_le16(&dir->inode_type) == SQFS_LDIR_TYPE)
			ldir = (struct squashfs_ldir_inode *)table;

		/* Get dir. offset into the directory table */
		offset = sqfs_dir_offset(table, m_list, m_count);
		dirs->table = &dirs->dir_table[offset];

		/* Copy directory header */
		memcpy(dirs->dir_header, &dirs->dir_table[offset],
		       SQFS_DIR_HEADER_SIZE);

		/* Check for empty directory */
		if (sqfs_is_empty_dir(table)) {
			printf("Empty directory.\n");
			free(dirs->entry);
			dirs->entry = NULL;
			ret = SQFS_EMPTY_DIR;
			goto out;
		}

		dirs->table += SQFS_DIR_HEADER_SIZE;
		dirs->size = get_unaligned_le16(&dir->file_size);
		dirs->entry_count = dirs->dir_header->count + 1;
		dirs->size -= SQFS_DIR_HEADER_SIZE;
		free(dirs->entry);
		dirs->entry = NULL;
	}

	offset = sqfs_dir_offset(table, m_list, m_count);
	dirs->table = &dirs->dir_table[offset];

	if (get_unaligned_le16(&dir->inode_type) == SQFS_DIR_TYPE)
		memcpy(&dirs->i_dir, dir, sizeof(*dir));
	else
		memcpy(&dirs->i_ldir, ldir, sizeof(*ldir));

out:
	free(res);
	free(rem);
	free(path);
	free(target);
	free(sym_tokens);
	return ret;
}

/*
 * Inode and directory tables are stored as a series of metadata blocks, and
 * given the compressed size of this table, we can calculate how much metadata
 * blocks are needed to store the result of the decompression, since a
 * decompressed metadata block should have a size of 8KiB.
 */
static int sqfs_count_metablks(void *table, u32 offset, int table_size)
{
	int count = 0, cur_size = 0, ret;
	u32 data_size;
	bool comp;

	do {
		ret = sqfs_read_metablock(table, offset + cur_size, &comp,
					  &data_size);
		if (ret)
			return -EINVAL;
		cur_size += data_size + SQFS_HEADER_SIZE;
		count++;
	} while (cur_size < table_size);

	return count;
}

/*
 * Storing the metadata blocks header's positions will be useful while looking
 * for an entry in the directory table, using the reference (index and offset)
 * given by its inode.
 */
static int sqfs_get_metablk_pos(u32 *pos_list, void *table, u32 offset,
				int metablks_count)
{
	u32 data_size, cur_size = 0;
	int j, ret = 0;
	bool comp;

	if (!metablks_count)
		return -EINVAL;

	for (j = 0; j < metablks_count; j++) {
		ret = sqfs_read_metablock(table, offset + cur_size, &comp,
					  &data_size);
		if (ret)
			return -EINVAL;

		cur_size += data_size + SQFS_HEADER_SIZE;
		pos_list[j] = cur_size;
	}

	return ret;
}

static int sqfs_read_inode_table(unsigned char **inode_table)
{
	struct squashfs_super_block *sblk = ctxt.sblk;
	u64 start, n_blks, table_offset, table_size;
	int j, ret = 0, metablks_count;
	unsigned char *src_table, *itb;
	u32 src_len, dest_offset = 0;
	unsigned long dest_len = 0;
	bool compressed;

	table_size = get_unaligned_le64(&sblk->directory_table_start) -
		get_unaligned_le64(&sblk->inode_table_start);
	start = get_unaligned_le64(&sblk->inode_table_start) /
		ctxt.cur_dev->blksz;
	n_blks = sqfs_calc_n_blks(sblk->inode_table_start,
				  sblk->directory_table_start, &table_offset);

	/* Allocate a proper sized buffer (itb) to store the inode table */
	itb = malloc_cache_aligned(n_blks * ctxt.cur_dev->blksz);
	if (!itb)
		return -ENOMEM;

	if (sqfs_disk_read(start, n_blks, itb) < 0) {
		ret = -EINVAL;
		goto free_itb;
	}

	/* Parse inode table (metadata block) header */
	ret = sqfs_read_metablock(itb, table_offset, &compressed, &src_len);
	if (ret) {
		ret = -EINVAL;
		goto free_itb;
	}

	/* Calculate size to store the whole decompressed table */
	metablks_count = sqfs_count_metablks(itb, table_offset, table_size);
	if (metablks_count < 1) {
		ret = -EINVAL;
		goto free_itb;
	}

	*inode_table = kcalloc(metablks_count, SQFS_METADATA_BLOCK_SIZE,
			       GFP_KERNEL);
	if (!*inode_table) {
		ret = -ENOMEM;
		printf("Error: failed to allocate squashfs inode_table of size %i, increasing CONFIG_SYS_MALLOC_LEN could help\n",
		       metablks_count * SQFS_METADATA_BLOCK_SIZE);
		goto free_itb;
	}

	src_table = itb + table_offset + SQFS_HEADER_SIZE;

	/* Extract compressed Inode table */
	for (j = 0; j < metablks_count; j++) {
		sqfs_read_metablock(itb, table_offset, &compressed, &src_len);
		if (compressed) {
			dest_len = SQFS_METADATA_BLOCK_SIZE;
			ret = sqfs_decompress(&ctxt, *inode_table +
					      dest_offset, &dest_len,
					      src_table, src_len);
			if (ret) {
				free(*inode_table);
				*inode_table = NULL;
				goto free_itb;
			}

			dest_offset += dest_len;
		} else {
			memcpy(*inode_table + (j * SQFS_METADATA_BLOCK_SIZE),
			       src_table, src_len);
		}

		/*
		 * Offsets to the decompression destination, to the metadata
		 * buffer 'itb' and to the decompression source, respectively.
		 */

		table_offset += src_len + SQFS_HEADER_SIZE;
		src_table += src_len + SQFS_HEADER_SIZE;
	}

free_itb:
	free(itb);

	return ret;
}

static int sqfs_read_directory_table(unsigned char **dir_table, u32 **pos_list)
{
	u64 start, n_blks, table_offset, table_size;
	struct squashfs_super_block *sblk = ctxt.sblk;
	int j, ret = 0, metablks_count = -1;
	unsigned char *src_table, *dtb;
	u32 src_len, dest_offset = 0;
	unsigned long dest_len = 0;
	bool compressed;

	*dir_table = NULL;
	*pos_list = NULL;
	/* DIRECTORY TABLE */
	table_size = get_unaligned_le64(&sblk->fragment_table_start) -
		get_unaligned_le64(&sblk->directory_table_start);
	start = get_unaligned_le64(&sblk->directory_table_start) /
		ctxt.cur_dev->blksz;
	n_blks = sqfs_calc_n_blks(sblk->directory_table_start,
				  sblk->fragment_table_start, &table_offset);

	/* Allocate a proper sized buffer (dtb) to store the directory table */
	dtb = malloc_cache_aligned(n_blks * ctxt.cur_dev->blksz);
	if (!dtb)
		return -ENOMEM;

	if (sqfs_disk_read(start, n_blks, dtb) < 0)
		goto out;

	/* Parse directory table (metadata block) header */
	ret = sqfs_read_metablock(dtb, table_offset, &compressed, &src_len);
	if (ret)
		goto out;

	/* Calculate total size to store the whole decompressed table */
	metablks_count = sqfs_count_metablks(dtb, table_offset, table_size);
	if (metablks_count < 1)
		goto out;

	*dir_table = malloc(metablks_count * SQFS_METADATA_BLOCK_SIZE);
	if (!*dir_table)
		goto out;

	*pos_list = malloc(metablks_count * sizeof(u32));
	if (!*pos_list)
		goto out;

	ret = sqfs_get_metablk_pos(*pos_list, dtb, table_offset,
				   metablks_count);
	if (ret) {
		metablks_count = -1;
		goto out;
	}

	src_table = dtb + table_offset + SQFS_HEADER_SIZE;

	/* Extract compressed Directory table */
	dest_offset = 0;
	for (j = 0; j < metablks_count; j++) {
		sqfs_read_metablock(dtb, table_offset, &compressed, &src_len);
		if (compressed) {
			dest_len = SQFS_METADATA_BLOCK_SIZE;
			ret = sqfs_decompress(&ctxt, *dir_table +
					      (j * SQFS_METADATA_BLOCK_SIZE),
					      &dest_len, src_table, src_len);
			if (ret) {
				metablks_count = -1;
				goto out;
			}

			if (dest_len < SQFS_METADATA_BLOCK_SIZE) {
				dest_offset += dest_len;
				break;
			}

			dest_offset += dest_len;
		} else {
			memcpy(*dir_table + (j * SQFS_METADATA_BLOCK_SIZE),
			       src_table, src_len);
		}

		/*
		 * Offsets to the decompression destination, to the metadata
		 * buffer 'dtb' and to the decompression source, respectively.
		 */
		table_offset += src_len + SQFS_HEADER_SIZE;
		src_table += src_len + SQFS_HEADER_SIZE;
	}

out:
	if (metablks_count < 1) {
		free(*dir_table);
		free(*pos_list);
		*dir_table = NULL;
		*pos_list = NULL;
	}
	free(dtb);

	return metablks_count;
}

int sqfs_opendir(const char *filename, struct fs_dir_stream **dirsp)
{
	unsigned char *inode_table = NULL, *dir_table = NULL;
	int j, token_count = 0, ret = 0, metablks_count;
	struct squashfs_dir_stream *dirs;
	char **token_list = NULL, *path = NULL;
	u32 *pos_list = NULL;

	dirs = calloc(1, sizeof(*dirs));
	if (!dirs)
		return -EINVAL;

	/* these should be set to NULL to prevent dangling pointers */
	dirs->dir_header = NULL;
	dirs->entry = NULL;
	dirs->table = NULL;
	dirs->inode_table = NULL;
	dirs->dir_table = NULL;

	ret = sqfs_read_inode_table(&inode_table);
	if (ret) {
		ret = -EINVAL;
		goto out;
	}

	metablks_count = sqfs_read_directory_table(&dir_table, &pos_list);
	if (metablks_count < 1) {
		ret = -EINVAL;
		goto out;
	}

	/* Tokenize filename */
	token_count = sqfs_count_tokens(filename);
	if (token_count < 0) {
		ret = -EINVAL;
		goto out;
	}

	path = strdup(filename);
	if (!path) {
		ret = -EINVAL;
		goto out;
	}

	token_list = malloc(token_count * sizeof(char *));
	if (!token_list) {
		ret = -EINVAL;
		goto out;
	}

	/* Fill tokens list */
	ret = sqfs_tokenize(token_list, token_count, path);
	if (ret)
		goto out;
	/*
	 * ldir's (extended directory) size is greater than dir, so it works as
	 * a general solution for the malloc size, since 'i' is a union.
	 */
	dirs->inode_table = inode_table;
	dirs->dir_table = dir_table;
	ret = sqfs_search_dir(dirs, token_list, token_count, pos_list,
			      metablks_count);
	if (ret)
		goto out;

	if (le16_to_cpu(dirs->i_dir.inode_type) == SQFS_DIR_TYPE)
		dirs->size = le16_to_cpu(dirs->i_dir.file_size);
	else
		dirs->size = le32_to_cpu(dirs->i_ldir.file_size);

	/* Setup directory header */
	memcpy(dirs->dir_header, dirs->table, SQFS_DIR_HEADER_SIZE);
	dirs->entry_count = dirs->dir_header->count + 1;
	dirs->size -= SQFS_DIR_HEADER_SIZE;

	/* Setup entry */
	dirs->entry = NULL;
	dirs->table += SQFS_DIR_HEADER_SIZE;

	*dirsp = (struct fs_dir_stream *)dirs;

out:
	for (j = 0; j < token_count; j++)
		free(token_list[j]);
	free(token_list);
	free(pos_list);
	free(path);
	if (ret) {
		free(inode_table);
		free(dirs);
	}

	return ret;
}

int sqfs_readdir(struct fs_dir_stream *fs_dirs, struct fs_dirent **dentp)
{
	struct squashfs_super_block *sblk = ctxt.sblk;
	struct squashfs_dir_stream *dirs;
	struct squashfs_lreg_inode *lreg;
	struct squashfs_base_inode *base;
	struct squashfs_reg_inode *reg;
	int i_number, offset = 0, ret;
	struct fs_dirent *dent;
	unsigned char *ipos;
	u16 name_size;

	dirs = (struct squashfs_dir_stream *)fs_dirs;
	if (!dirs->size) {
		*dentp = NULL;
		return -SQFS_STOP_READDIR;
	}

	dent = &dirs->dentp;

	if (!dirs->entry_count) {
		if (dirs->size > SQFS_DIR_HEADER_SIZE) {
			dirs->size -= SQFS_DIR_HEADER_SIZE;
		} else {
			*dentp = NULL;
			dirs->size = 0;
			return -SQFS_STOP_READDIR;
		}

		if (dirs->size > SQFS_EMPTY_FILE_SIZE) {
			/* Read follow-up (emitted) dir. header */
			memcpy(dirs->dir_header, dirs->table,
			       SQFS_DIR_HEADER_SIZE);
			dirs->entry_count = dirs->dir_header->count + 1;
			ret = sqfs_read_entry(&dirs->entry, dirs->table +
					      SQFS_DIR_HEADER_SIZE);
			if (ret)
				return -SQFS_STOP_READDIR;

			dirs->table += SQFS_DIR_HEADER_SIZE;
		}
	} else {
		ret = sqfs_read_entry(&dirs->entry, dirs->table);
		if (ret)
			return -SQFS_STOP_READDIR;
	}

	i_number = dirs->dir_header->inode_number + dirs->entry->inode_offset;
	ipos = sqfs_find_inode(dirs->inode_table, i_number, sblk->inodes,
			       sblk->block_size);

	base = (struct squashfs_base_inode *)ipos;

	/* Set entry type and size */
	switch (dirs->entry->type) {
	case SQFS_DIR_TYPE:
	case SQFS_LDIR_TYPE:
		dent->type = FS_DT_DIR;
		break;
	case SQFS_REG_TYPE:
	case SQFS_LREG_TYPE:
		/*
		 * Entries do not differentiate extended from regular types, so
		 * it needs to be verified manually.
		 */
		if (get_unaligned_le16(&base->inode_type) == SQFS_LREG_TYPE) {
			lreg = (struct squashfs_lreg_inode *)ipos;
			dent->size = get_unaligned_le64(&lreg->file_size);
		} else {
			reg = (struct squashfs_reg_inode *)ipos;
			dent->size = get_unaligned_le32(&reg->file_size);
		}

		dent->type = FS_DT_REG;
		break;
	case SQFS_BLKDEV_TYPE:
	case SQFS_CHRDEV_TYPE:
	case SQFS_LBLKDEV_TYPE:
	case SQFS_LCHRDEV_TYPE:
	case SQFS_FIFO_TYPE:
	case SQFS_SOCKET_TYPE:
	case SQFS_LFIFO_TYPE:
	case SQFS_LSOCKET_TYPE:
		dent->type = SQFS_MISC_ENTRY_TYPE;
		break;
	case SQFS_SYMLINK_TYPE:
	case SQFS_LSYMLINK_TYPE:
		dent->type = FS_DT_LNK;
		break;
	default:
		return -SQFS_STOP_READDIR;
	}

	/* Set entry name (capped at FS_DIRENT_NAME_LEN which is a U-Boot limitation) */
	name_size = min_t(u16, dirs->entry->name_size + 1, FS_DIRENT_NAME_LEN - 1);
	strncpy(dent->name, dirs->entry->name, name_size);
	dent->name[name_size] = '\0';

	offset = dirs->entry->name_size + 1 + SQFS_ENTRY_BASE_LENGTH;
	dirs->entry_count--;

	/* Decrement size to be read */
	if (dirs->size > offset)
		dirs->size -= offset;
	else
		dirs->size = 0;

	/* Keep a reference to the current entry before incrementing it */
	dirs->table += offset;

	*dentp = dent;

	return 0;
}

int sqfs_probe(struct blk_desc *fs_dev_desc, struct disk_partition *fs_partition)
{
	struct squashfs_super_block *sblk;
	int ret;

	ctxt.cur_dev = fs_dev_desc;
	ctxt.cur_part_info = *fs_partition;

	ret = sqfs_read_sblk(&sblk);
	if (ret)
		goto error;

	/* Make sure it has a valid SquashFS magic number*/
	if (get_unaligned_le32(&sblk->s_magic) != SQFS_MAGIC_NUMBER) {
		debug("Bad magic number for SquashFS image.\n");
		ret = -EINVAL;
		goto error;
	}

	ctxt.sblk = sblk;

	ret = sqfs_decompressor_init(&ctxt);
	if (ret) {
		goto error;
	}

	return 0;
error:
	ctxt.cur_dev = NULL;
	free(ctxt.sblk);
	ctxt.sblk = NULL;
	return ret;
}

static char *sqfs_basename(char *path)
{
	char *fname;

	fname = path + strlen(path) - 1;
	while (fname >= path) {
		if (*fname == '/') {
			fname++;
			break;
		}

		fname--;
	}

	return fname;
}

static char *sqfs_dirname(char *path)
{
	char *fname;

	fname = sqfs_basename(path);
	--fname;
	*fname = '\0';

	return path;
}

/*
 * Takes a path to file and splits it in two parts: the filename itself and the
 * directory's path, e.g.:
 * path: /path/to/file.txt
 * file: file.txt
 * dir: /path/to
 */
static int sqfs_split_path(char **file, char **dir, const char *path)
{
	char *dirc, *basec, *bname, *dname, *tmp_path;
	int ret = 0;

	*file = NULL;
	*dir = NULL;
	dirc = NULL;
	basec = NULL;
	bname = NULL;
	dname = NULL;
	tmp_path = NULL;

	/* check for first slash in path*/
	if (path[0] == '/') {
		tmp_path = strdup(path);
		if (!tmp_path) {
			ret = -ENOMEM;
			goto out;
		}
	} else {
		tmp_path = malloc(strlen(path) + 2);
		if (!tmp_path) {
			ret = -ENOMEM;
			goto out;
		}
		tmp_path[0] = '/';
		strcpy(tmp_path + 1, path);
	}

	/* String duplicates */
	dirc = strdup(tmp_path);
	if (!dirc) {
		ret = -ENOMEM;
		goto out;
	}

	basec = strdup(tmp_path);
	if (!basec) {
		ret = -ENOMEM;
		goto out;
	}

	dname = sqfs_dirname(dirc);
	bname = sqfs_basename(basec);

	*file = strdup(bname);

	if (!*file) {
		ret = -ENOMEM;
		goto out;
	}

	if (*dname == '\0') {
		*dir = malloc(2);
		if (!*dir) {
			ret = -ENOMEM;
			goto out;
		}

		(*dir)[0] = '/';
		(*dir)[1] = '\0';
	} else {
		*dir = strdup(dname);
		if (!*dir) {
			ret = -ENOMEM;
			goto out;
		}
	}

out:
	if (ret) {
		free(*file);
		free(*dir);
		*dir = NULL;
		*file = NULL;
	}
	free(basec);
	free(dirc);
	free(tmp_path);

	return ret;
}

static int sqfs_get_regfile_info(struct squashfs_reg_inode *reg,
				 struct squashfs_file_info *finfo,
				 struct squashfs_fragment_block_entry *fentry,
				 __le32 blksz)
{
	int datablk_count = 0, ret;

	finfo->size = get_unaligned_le32(&reg->file_size);
	finfo->offset = get_unaligned_le32(&reg->offset);
	finfo->start = get_unaligned_le32(&reg->start_block);
	finfo->frag = SQFS_IS_FRAGMENTED(get_unaligned_le32(&reg->fragment));

	if (finfo->frag && finfo->offset == 0xFFFFFFFF)
		return -EINVAL;

	if (finfo->size < 1 || finfo->start == 0xFFFFFFFF)
		return -EINVAL;

	if (finfo->frag) {
		datablk_count = finfo->size / le32_to_cpu(blksz);
		ret = sqfs_frag_lookup(get_unaligned_le32(&reg->fragment),
				       fentry);
		if (ret < 0)
			return -EINVAL;
		finfo->comp = ret;
		if (fentry->size < 1 || fentry->start == 0x7FFFFFFF)
			return -EINVAL;
	} else {
		datablk_count = DIV_ROUND_UP(finfo->size, le32_to_cpu(blksz));
	}

	finfo->blk_sizes = malloc(datablk_count * sizeof(u32));
	if (!finfo->blk_sizes)
		return -ENOMEM;

	return datablk_count;
}

static int sqfs_get_lregfile_info(struct squashfs_lreg_inode *lreg,
				  struct squashfs_file_info *finfo,
				  struct squashfs_fragment_block_entry *fentry,
				 __le32 blksz)
{
	int datablk_count = 0, ret;

	finfo->size = get_unaligned_le64(&lreg->file_size);
	finfo->offset = get_unaligned_le32(&lreg->offset);
	finfo->start = get_unaligned_le64(&lreg->start_block);
	finfo->frag = SQFS_IS_FRAGMENTED(get_unaligned_le32(&lreg->fragment));

	if (finfo->frag && finfo->offset == 0xFFFFFFFF)
		return -EINVAL;

	if (finfo->size < 1 || finfo->start == 0x7FFFFFFF)
		return -EINVAL;

	if (finfo->frag) {
		datablk_count = finfo->size / le32_to_cpu(blksz);
		ret = sqfs_frag_lookup(get_unaligned_le32(&lreg->fragment),
				       fentry);
		if (ret < 0)
			return -EINVAL;
		finfo->comp = ret;
		if (fentry->size < 1 || fentry->start == 0x7FFFFFFF)
			return -EINVAL;
	} else {
		datablk_count = DIV_ROUND_UP(finfo->size, le32_to_cpu(blksz));
	}

	finfo->blk_sizes = malloc(datablk_count * sizeof(u32));
	if (!finfo->blk_sizes)
		return -ENOMEM;

	return datablk_count;
}

int sqfs_read(const char *filename, void *buf, loff_t offset, loff_t len,
	      loff_t *actread)
{
	char *dir = NULL, *fragment_block, *datablock = NULL;
	char *fragment = NULL, *file = NULL, *resolved, *data;
	u64 start, n_blks, table_size, data_offset, table_offset, sparse_size;
	int ret, j, i_number, datablk_count = 0;
	struct squashfs_super_block *sblk = ctxt.sblk;
	struct squashfs_fragment_block_entry frag_entry;
	struct squashfs_file_info finfo = {0};
	struct squashfs_symlink_inode *symlink;
	struct fs_dir_stream *dirsp = NULL;
	struct squashfs_dir_stream *dirs;
	struct squashfs_lreg_inode *lreg;
	struct squashfs_base_inode *base;
	struct squashfs_reg_inode *reg;
	unsigned long dest_len;
	struct fs_dirent *dent;
	unsigned char *ipos;

	*actread = 0;

	if (offset) {
		/*
		 * TODO: implement reading at an offset in file
		 */
		printf("Error: reading at a specific offset in a squashfs file is not supported yet.\n");
		return -EINVAL;
	}

	/*
	 * sqfs_opendir will uncompress inode and directory tables, and will
	 * return a pointer to the directory that contains the requested file.
	 */
	sqfs_split_path(&file, &dir, filename);
	ret = sqfs_opendir(dir, &dirsp);
	if (ret) {
		goto out;
	}

	dirs = (struct squashfs_dir_stream *)dirsp;

	/* For now, only regular files are able to be loaded */
	while (!sqfs_readdir(dirsp, &dent)) {
		ret = strcmp(dent->name, file);
		if (!ret)
			break;

		free(dirs->entry);
		dirs->entry = NULL;
	}

	if (ret) {
		printf("File not found.\n");
		*actread = 0;
		ret = -ENOENT;
		goto out;
	}

	i_number = dirs->dir_header->inode_number + dirs->entry->inode_offset;
	ipos = sqfs_find_inode(dirs->inode_table, i_number, sblk->inodes,
			       sblk->block_size);

	base = (struct squashfs_base_inode *)ipos;
	switch (get_unaligned_le16(&base->inode_type)) {
	case SQFS_REG_TYPE:
		reg = (struct squashfs_reg_inode *)ipos;
		datablk_count = sqfs_get_regfile_info(reg, &finfo, &frag_entry,
						      sblk->block_size);
		if (datablk_count < 0) {
			ret = -EINVAL;
			goto out;
		}

		memcpy(finfo.blk_sizes, ipos + sizeof(*reg),
		       datablk_count * sizeof(u32));
		break;
	case SQFS_LREG_TYPE:
		lreg = (struct squashfs_lreg_inode *)ipos;
		datablk_count = sqfs_get_lregfile_info(lreg, &finfo,
						       &frag_entry,
						       sblk->block_size);
		if (datablk_count < 0) {
			ret = -EINVAL;
			goto out;
		}

		memcpy(finfo.blk_sizes, ipos + sizeof(*lreg),
		       datablk_count * sizeof(u32));
		break;
	case SQFS_SYMLINK_TYPE:
	case SQFS_LSYMLINK_TYPE:
		symlink = (struct squashfs_symlink_inode *)ipos;
		resolved = sqfs_resolve_symlink(symlink, filename);
		ret = sqfs_read(resolved, buf, offset, len, actread);
		free(resolved);
		goto out;
	case SQFS_BLKDEV_TYPE:
	case SQFS_CHRDEV_TYPE:
	case SQFS_LBLKDEV_TYPE:
	case SQFS_LCHRDEV_TYPE:
	case SQFS_FIFO_TYPE:
	case SQFS_SOCKET_TYPE:
	case SQFS_LFIFO_TYPE:
	case SQFS_LSOCKET_TYPE:
	default:
		printf("Unsupported entry type\n");
		ret = -EINVAL;
		goto out;
	}

	/* If the user specifies a length, check its sanity */
	if (len) {
		if (len > finfo.size) {
			ret = -EINVAL;
			goto out;
		}

		finfo.size = len;
	} else {
		len = finfo.size;
	}

	if (datablk_count) {
		data_offset = finfo.start;
		datablock = malloc(get_unaligned_le32(&sblk->block_size));
		if (!datablock) {
			ret = -ENOMEM;
			goto out;
		}
	}

	for (j = 0; j < datablk_count; j++) {
		char *data_buffer;

		start = lldiv(data_offset, ctxt.cur_dev->blksz);
		table_size = SQFS_BLOCK_SIZE(finfo.blk_sizes[j]);
		table_offset = data_offset - (start * ctxt.cur_dev->blksz);
		n_blks = DIV_ROUND_UP(table_size + table_offset,
				      ctxt.cur_dev->blksz);

		/* Don't load any data for sparse blocks */
		if (finfo.blk_sizes[j] == 0) {
			n_blks = 0;
			table_offset = 0;
			data_buffer = NULL;
			data = NULL;
		} else {
			data_buffer = malloc_cache_aligned(n_blks * ctxt.cur_dev->blksz);

			if (!data_buffer) {
				ret = -ENOMEM;
				goto out;
			}

			ret = sqfs_disk_read(start, n_blks, data_buffer);
			if (ret < 0) {
				/*
				 * Possible causes: too many data blocks or too large
				 * SquashFS block size. Tip: re-compile the SquashFS
				 * image with mksquashfs's -b <block_size> option.
				 */
				printf("Error: too many data blocks to be read.\n");
				goto out;
			}

			data = data_buffer + table_offset;
		}

		/* Load the data */
		if (finfo.blk_sizes[j] == 0) {
			/* This is a sparse block */
			sparse_size = get_unaligned_le32(&sblk->block_size);
			if ((*actread + sparse_size) > len)
				sparse_size = len - *actread;
			memset(buf + *actread, 0, sparse_size);
			*actread += sparse_size;
		} else if (SQFS_COMPRESSED_BLOCK(finfo.blk_sizes[j])) {
			dest_len = get_unaligned_le32(&sblk->block_size);
			ret = sqfs_decompress(&ctxt, datablock, &dest_len,
					      data, table_size);
			if (ret)
				goto out;

			if ((*actread + dest_len) > len)
				dest_len = len - *actread;
			memcpy(buf + *actread, datablock, dest_len);
			*actread += dest_len;
		} else {
			if ((*actread + table_size) > len)
				table_size = len - *actread;
			memcpy(buf + *actread, data, table_size);
			*actread += table_size;
		}

		data_offset += table_size;
		free(data_buffer);
		if (*actread >= len)
			break;
	}

	/*
	 * There is no need to continue if the file is not fragmented.
	 */
	if (!finfo.frag) {
		ret = 0;
		goto out;
	}

	start = lldiv(frag_entry.start, ctxt.cur_dev->blksz);
	table_size = SQFS_BLOCK_SIZE(frag_entry.size);
	table_offset = frag_entry.start - (start * ctxt.cur_dev->blksz);
	n_blks = DIV_ROUND_UP(table_size + table_offset, ctxt.cur_dev->blksz);

	fragment = malloc_cache_aligned(n_blks * ctxt.cur_dev->blksz);

	if (!fragment) {
		ret = -ENOMEM;
		goto out;
	}

	ret = sqfs_disk_read(start, n_blks, fragment);
	if (ret < 0)
		goto out;

	/* File compressed and fragmented */
	if (finfo.frag && finfo.comp) {
		dest_len = get_unaligned_le32(&sblk->block_size);
		fragment_block = malloc(dest_len);
		if (!fragment_block) {
			ret = -ENOMEM;
			goto out;
		}

		ret = sqfs_decompress(&ctxt, fragment_block, &dest_len,
				      (void *)fragment  + table_offset,
				      frag_entry.size);
		if (ret) {
			free(fragment_block);
			goto out;
		}

		memcpy(buf + *actread, &fragment_block[finfo.offset], finfo.size - *actread);
		*actread = finfo.size;

		free(fragment_block);

	} else if (finfo.frag && !finfo.comp) {
		fragment_block = (void *)fragment + table_offset;

		memcpy(buf + *actread, &fragment_block[finfo.offset], finfo.size - *actread);
		*actread = finfo.size;
	}

out:
	free(fragment);
	free(datablock);
	free(file);
	free(dir);
	free(finfo.blk_sizes);
	sqfs_closedir(dirsp);

	return ret;
}

int sqfs_size(const char *filename, loff_t *size)
{
	struct squashfs_super_block *sblk = ctxt.sblk;
	struct squashfs_symlink_inode *symlink;
	struct fs_dir_stream *dirsp = NULL;
	struct squashfs_base_inode *base;
	struct squashfs_dir_stream *dirs;
	struct squashfs_lreg_inode *lreg;
	struct squashfs_reg_inode *reg;
	char *dir, *file, *resolved;
	struct fs_dirent *dent;
	unsigned char *ipos;
	int ret, i_number;

	sqfs_split_path(&file, &dir, filename);
	/*
	 * sqfs_opendir will uncompress inode and directory tables, and will
	 * return a pointer to the directory that contains the requested file.
	 */
	ret = sqfs_opendir(dir, &dirsp);
	if (ret) {
		ret = -EINVAL;
		goto free_strings;
	}

	dirs = (struct squashfs_dir_stream *)dirsp;

	while (!sqfs_readdir(dirsp, &dent)) {
		ret = strcmp(dent->name, file);
		if (!ret)
			break;
		free(dirs->entry);
		dirs->entry = NULL;
	}

	if (ret) {
		printf("File not found.\n");
		*size = 0;
		ret = -EINVAL;
		goto free_strings;
	}

	i_number = dirs->dir_header->inode_number + dirs->entry->inode_offset;
	ipos = sqfs_find_inode(dirs->inode_table, i_number, sblk->inodes,
			       sblk->block_size);
	free(dirs->entry);
	dirs->entry = NULL;

	base = (struct squashfs_base_inode *)ipos;
	switch (get_unaligned_le16(&base->inode_type)) {
	case SQFS_REG_TYPE:
		reg = (struct squashfs_reg_inode *)ipos;
		*size = get_unaligned_le32(&reg->file_size);
		break;
	case SQFS_LREG_TYPE:
		lreg = (struct squashfs_lreg_inode *)ipos;
		*size = get_unaligned_le64(&lreg->file_size);
		break;
	case SQFS_SYMLINK_TYPE:
	case SQFS_LSYMLINK_TYPE:
		symlink = (struct squashfs_symlink_inode *)ipos;
		resolved = sqfs_resolve_symlink(symlink, filename);
		ret = sqfs_size(resolved, size);
		free(resolved);
		break;
	case SQFS_BLKDEV_TYPE:
	case SQFS_CHRDEV_TYPE:
	case SQFS_LBLKDEV_TYPE:
	case SQFS_LCHRDEV_TYPE:
	case SQFS_FIFO_TYPE:
	case SQFS_SOCKET_TYPE:
	case SQFS_LFIFO_TYPE:
	case SQFS_LSOCKET_TYPE:
	default:
		printf("Unable to recover entry's size.\n");
		*size = 0;
		ret = -EINVAL;
		break;
	}

free_strings:
	free(dir);
	free(file);

	sqfs_closedir(dirsp);

	return ret;
}

int sqfs_exists(const char *filename)
{
	struct fs_dir_stream *dirsp = NULL;
	struct squashfs_dir_stream *dirs;
	char *dir, *file;
	struct fs_dirent *dent;
	int ret;

	sqfs_split_path(&file, &dir, filename);
	/*
	 * sqfs_opendir will uncompress inode and directory tables, and will
	 * return a pointer to the directory that contains the requested file.
	 */
	ret = sqfs_opendir(dir, &dirsp);
	if (ret) {
		ret = -EINVAL;
		goto free_strings;
	}

	dirs = (struct squashfs_dir_stream *)dirsp;

	while (!sqfs_readdir(dirsp, &dent)) {
		ret = strcmp(dent->name, file);
		if (!ret)
			break;
		free(dirs->entry);
		dirs->entry = NULL;
	}

	sqfs_closedir(dirsp);

free_strings:
	free(dir);
	free(file);

	return ret == 0;
}

void sqfs_close(void)
{
	sqfs_decompressor_cleanup(&ctxt);
	free(ctxt.sblk);
	ctxt.sblk = NULL;
	ctxt.cur_dev = NULL;
}

void sqfs_closedir(struct fs_dir_stream *dirs)
{
	struct squashfs_dir_stream *sqfs_dirs;

	if (!dirs)
		return;

	sqfs_dirs = (struct squashfs_dir_stream *)dirs;
	free(sqfs_dirs->inode_table);
	free(sqfs_dirs->dir_table);
	free(sqfs_dirs->dir_header);
	free(sqfs_dirs);
}
