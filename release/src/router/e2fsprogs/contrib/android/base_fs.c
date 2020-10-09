#include "base_fs.h"
#include <stdio.h>

#define BASE_FS_VERSION "Base EXT4 version 1.0"

struct base_fs {
	FILE *file;
	const char *mountpoint;
	struct basefs_entry entry;
};

static FILE *basefs_open(const char *file)
{
	char *line = NULL;
	size_t len;
	FILE *f = fopen(file, "r");
	if (!f)
		return NULL;

	if (getline(&line, &len, f) == -1 || !line)
		goto err_getline;

	if (strncmp(line, BASE_FS_VERSION, strlen(BASE_FS_VERSION)))
		goto err_header;

	free(line);
	return f;

err_header:
	free(line);
err_getline:
	fclose(f);
	return NULL;
}

static struct basefs_entry *basefs_readline(FILE *f, const char *mountpoint,
					    int *err)
{
	char *line = NULL, *saveptr1, *saveptr2, *block_range, *block;
	int offset;
	size_t len;
	struct basefs_entry *entry = NULL;
	blk64_t range_start, range_end;

	if (getline(&line, &len, f) == -1) {
		if (feof(f))
			goto end;
		goto err_getline;
	}

	entry = calloc(1, sizeof(*entry));
	if (!entry)
		goto err_alloc;

	/*
	 * With BASEFS version 1.0, a typical line looks like this:
	 * /bin/mke2fs 5000-5004,8000,9000-9990
	 */
	if (sscanf(line, "%ms%n", &entry->path, &offset) != 1)
		goto err_sscanf;
	len = strlen(mountpoint);
	memmove(entry->path, entry->path + len, strlen(entry->path) - len + 1);

	while (line[offset] == ' ')
		++offset;

	block_range = strtok_r(line + offset, ",\n", &saveptr1);
	while (block_range) {
		block = strtok_r(block_range, "-", &saveptr2);
		if (!block)
			break;
		range_start = atoll(block);
		block = strtok_r(NULL, "-", &saveptr2);
		range_end = block ? atoll(block) : range_start;
		add_blocks_to_range(&entry->blocks, range_start, range_end);
		block_range = strtok_r(NULL, ",\n", &saveptr1);
	}
end:
	*err = 0;
	free(line);
	return entry;

err_sscanf:
	free(entry);
err_alloc:
	free(line);
err_getline:
	*err = 1;
	return NULL;
}

static void free_base_fs_entry(void *e)
{
	struct basefs_entry *entry = e;
	if (entry) {
		free(entry->path);
		free(entry);
	}
}

struct ext2fs_hashmap *basefs_parse(const char *file, const char *mountpoint)
{
	int err;
	struct ext2fs_hashmap *entries = NULL;
	struct basefs_entry *entry;
	FILE *f = basefs_open(file);
	if (!f)
		return NULL;
	entries = ext2fs_hashmap_create(ext2fs_djb2_hash, free_base_fs_entry, 1024);
	if (!entries)
		goto end;

	while ((entry = basefs_readline(f, mountpoint, &err)))
		ext2fs_hashmap_add(entries, entry, entry->path,
				   strlen(entry->path));

	if (err) {
		fclose(f);
		ext2fs_hashmap_free(entries);
		return NULL;
	}
end:
	fclose(f);
	return entries;
}

static void *init(const char *file, const char *mountpoint)
{
	struct base_fs *params = malloc(sizeof(*params));

	if (!params)
		return NULL;
	params->mountpoint = mountpoint;
	params->file = fopen(file, "w+");
	if (!params->file) {
		free(params);
		return NULL;
	}
	if (fwrite(BASE_FS_VERSION"\n", 1, strlen(BASE_FS_VERSION"\n"),
		   params->file) != strlen(BASE_FS_VERSION"\n")) {
		fclose(params->file);
		free(params);
		return NULL;
	}
	return params;
}

static int start_new_file(char *path, ext2_ino_t ino EXT2FS_ATTR((unused)),
			  struct ext2_inode *inode, void *data)
{
	struct base_fs *params = data;

	params->entry.path = LINUX_S_ISREG(inode->i_mode) ? path : NULL;
	return 0;
}

static int add_block(ext2_filsys fs EXT2FS_ATTR((unused)), blk64_t blocknr,
		     int metadata, void *data)
{
	struct base_fs *params = data;

	if (params->entry.path && !metadata)
		add_blocks_to_range(&params->entry.blocks, blocknr, blocknr);
	return 0;
}

static int inline_data(void *inline_data EXT2FS_ATTR((unused)),
		       void *data EXT2FS_ATTR((unused)))
{
	return 0;
}

static int end_new_file(void *data)
{
	struct base_fs *params = data;

	if (!params->entry.path)
		return 0;
	if (fprintf(params->file, "%s%s ", params->mountpoint,
		    params->entry.path) < 0
	    || write_block_ranges(params->file, params->entry.blocks.head, ",")
	    || fwrite("\n", 1, 1, params->file) != 1)
		return -1;

	delete_block_ranges(&params->entry.blocks);
	return 0;
}

static int cleanup(void *data)
{
	struct base_fs *params = data;

	fclose(params->file);
	free(params);
	return 0;
}

struct fsmap_format base_fs_format = {
	.init = init,
	.start_new_file = start_new_file,
	.add_block = add_block,
	.inline_data = inline_data,
	.end_new_file = end_new_file,
	.cleanup = cleanup,
};
