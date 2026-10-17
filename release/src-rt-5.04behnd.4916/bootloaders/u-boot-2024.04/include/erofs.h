/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _EROFS_H_
#define _EROFS_H_

struct disk_partition;

int erofs_opendir(const char *filename, struct fs_dir_stream **dirsp);
int erofs_readdir(struct fs_dir_stream *dirs, struct fs_dirent **dentp);
int erofs_probe(struct blk_desc *fs_dev_desc,
		struct disk_partition *fs_partition);
int erofs_read(const char *filename, void *buf, loff_t offset,
	       loff_t len, loff_t *actread);
int erofs_size(const char *filename, loff_t *size);
int erofs_exists(const char *filename);
void erofs_close(void);
void erofs_closedir(struct fs_dir_stream *dirs);
int erofs_uuid(char *uuid_str);

#endif /* _EROFS_H */
