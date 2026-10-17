// SPDX-License-Identifier: GPL-2.0+
#include "internal.h"
#include <fs_internal.h>

struct erofs_sb_info sbi;

static struct erofs_ctxt {
	struct disk_partition cur_part_info;
	struct blk_desc *cur_dev;
} ctxt;

int erofs_dev_read(int device_id, void *buf, u64 offset, size_t len)
{
	lbaint_t sect = offset >> ctxt.cur_dev->log2blksz;
	int off = offset & (ctxt.cur_dev->blksz - 1);

	if (!ctxt.cur_dev)
		return -EIO;

	if (fs_devread(ctxt.cur_dev, &ctxt.cur_part_info, sect,
		       off, len, buf))
		return 0;
	return -EIO;
}

int erofs_blk_read(void *buf, erofs_blk_t start, u32 nblocks)
{
	return erofs_dev_read(0, buf, erofs_pos(start),
			 erofs_pos(nblocks));
}

int erofs_probe(struct blk_desc *fs_dev_desc,
		struct disk_partition *fs_partition)
{
	int ret;

	ctxt.cur_dev = fs_dev_desc;
	ctxt.cur_part_info = *fs_partition;

	ret = erofs_read_superblock();
	if (ret)
		goto error;

	return 0;
error:
	ctxt.cur_dev = NULL;
	return ret;
}

struct erofs_dir_stream {
	struct fs_dir_stream fs_dirs;
	struct fs_dirent dirent;

	struct erofs_inode inode;
	char dblk[EROFS_MAX_BLOCK_SIZE];
	unsigned int maxsize, de_end;
	erofs_off_t pos;
};

static int erofs_readlink(struct erofs_inode *vi)
{
	size_t len = vi->i_size;
	char *target;
	int err;

	target = malloc(len + 1);
	if (!target)
		return -ENOMEM;
	target[len] = '\0';

	err = erofs_pread(vi, target, len, 0);
	if (err)
		goto err_out;

	err = erofs_ilookup(target, vi);
	if (err)
		goto err_out;

err_out:
	free(target);
	return err;
}

int erofs_opendir(const char *filename, struct fs_dir_stream **dirsp)
{
	struct erofs_dir_stream *dirs;
	int err;

	dirs = calloc(1, sizeof(*dirs));
	if (!dirs)
		return -ENOMEM;

	err = erofs_ilookup(filename, &dirs->inode);
	if (err)
		goto err_out;

	if (S_ISLNK(dirs->inode.i_mode)) {
		err = erofs_readlink(&dirs->inode);
		if (err)
			goto err_out;
	}

	if (!S_ISDIR(dirs->inode.i_mode)) {
		err = -ENOTDIR;
		goto err_out;
	}
	*dirsp = (struct fs_dir_stream *)dirs;
	return 0;
err_out:
	free(dirs);
	return err;
}

int erofs_readdir(struct fs_dir_stream *fs_dirs, struct fs_dirent **dentp)
{
	struct erofs_dir_stream *dirs = (struct erofs_dir_stream *)fs_dirs;
	struct fs_dirent *dent = &dirs->dirent;
	erofs_off_t pos = dirs->pos;
	unsigned int nameoff, de_namelen;
	struct erofs_dirent *de;
	char *de_name;
	int err;

	if (pos >= dirs->inode.i_size)
		return 1;

	if (!dirs->maxsize) {
		dirs->maxsize = min_t(unsigned int, EROFS_MAX_BLOCK_SIZE,
				      dirs->inode.i_size - pos);

		err = erofs_pread(&dirs->inode, dirs->dblk,
				  dirs->maxsize, pos);
		if (err)
			return err;

		de = (struct erofs_dirent *)dirs->dblk;
		dirs->de_end = le16_to_cpu(de->nameoff);
		if (dirs->de_end < sizeof(struct erofs_dirent) ||
		    dirs->de_end >= EROFS_MAX_BLOCK_SIZE) {
			erofs_err("invalid de[0].nameoff %u @ nid %llu",
				  dirs->de_end, de->nid | 0ULL);
			return -EFSCORRUPTED;
		}
	}

	de = (struct erofs_dirent *)(dirs->dblk + erofs_blkoff(pos));
	nameoff = le16_to_cpu(de->nameoff);
	de_name = (char *)dirs->dblk + nameoff;

	/* the last dirent in the block? */
	if (de + 1 >= (struct erofs_dirent *)(dirs->dblk + dirs->de_end))
		de_namelen = strnlen(de_name, dirs->maxsize - nameoff);
	else
		de_namelen = le16_to_cpu(de[1].nameoff) - nameoff;

	/* a corrupted entry is found */
	if (nameoff + de_namelen > dirs->maxsize ||
	    de_namelen > EROFS_NAME_LEN) {
		erofs_err("bogus dirent @ nid %llu", de->nid | 0ULL);
		DBG_BUGON(1);
		return -EFSCORRUPTED;
	}

	memcpy(dent->name, de_name, de_namelen);
	dent->name[de_namelen] = '\0';

	if (de->file_type == EROFS_FT_DIR) {
		dent->type = FS_DT_DIR;
	} else if (de->file_type == EROFS_FT_SYMLINK) {
		dent->type = FS_DT_LNK;
	} else {
		struct erofs_inode vi;

		dent->type = FS_DT_REG;
		vi.nid = de->nid;

		err = erofs_read_inode_from_disk(&vi);
		if (err)
			return err;
		dent->size = vi.i_size;
	}
	*dentp = dent;

	pos += sizeof(*de);
	if (erofs_blkoff(pos) >= dirs->de_end) {
		pos = erofs_pos(erofs_blknr(pos) + 1);
		dirs->maxsize = 0;
	}
	dirs->pos = pos;
	return 0;
}

void erofs_closedir(struct fs_dir_stream *fs_dirs)
{
	free(fs_dirs);
}

int erofs_exists(const char *filename)
{
	struct erofs_inode vi;
	int err;

	err = erofs_ilookup(filename, &vi);
	return err == 0;
}

int erofs_size(const char *filename, loff_t *size)
{
	struct erofs_inode vi;
	int err;

	err = erofs_ilookup(filename, &vi);
	if (err)
		return err;
	*size = vi.i_size;
	return 0;
}

int erofs_read(const char *filename, void *buf, loff_t offset, loff_t len,
	       loff_t *actread)
{
	struct erofs_inode vi;
	int err;

	err = erofs_ilookup(filename, &vi);
	if (err)
		return err;

	if (S_ISLNK(vi.i_mode)) {
		err = erofs_readlink(&vi);
		if (err)
			return err;
	}

	if (!len)
		len = vi.i_size;

	err = erofs_pread(&vi, buf, len, offset);
	if (err) {
		*actread = 0;
		return err;
	}

	if (offset >= vi.i_size)
		*actread = 0;
	else if (offset + len > vi.i_size)
		*actread = vi.i_size - offset;
	else
		*actread = len;
	return 0;
}

void erofs_close(void)
{
	ctxt.cur_dev = NULL;
}

int erofs_uuid(char *uuid_str)
{
	if (IS_ENABLED(CONFIG_LIB_UUID)) {
		if (ctxt.cur_dev)
			uuid_bin_to_str(sbi.uuid, uuid_str,
					UUID_STR_FORMAT_STD);
		return 0;
	}
	return -ENOSYS;
}
