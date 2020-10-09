#include "fsmap.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "support/nls-enable.h"

struct walk_ext_priv_data {
	char			*path;
	ext2_filsys		fs;
	struct fsmap_format	*format;
};

static int walk_block(ext2_filsys fs  EXT2FS_ATTR((unused)), blk64_t *blocknr,
		      e2_blkcnt_t blockcnt,
		      blk64_t ref64_blk EXT2FS_ATTR((unused)),
		      int ref_offset EXT2FS_ATTR((unused)),
		      void *priv)
{
	struct walk_ext_priv_data *pdata = priv;
	struct fsmap_format *format = pdata->format;

	return format->add_block(fs, *blocknr, blockcnt < 0, format->private);
}

static errcode_t ino_iter_extents(ext2_filsys fs, ext2_ino_t ino,
				  ext2_extent_handle_t extents,
				  struct walk_ext_priv_data *pdata)
{
	blk64_t block;
	errcode_t retval;
	blk64_t next_lblk = 0;
	int op = EXT2_EXTENT_ROOT;
	struct ext2fs_extent extent;
	struct fsmap_format *format = pdata->format;

	for (;;) {
		retval = ext2fs_extent_get(extents, op, &extent);
		if (retval)
			break;

		op = EXT2_EXTENT_NEXT;

		if ((extent.e_flags & EXT2_EXTENT_FLAGS_SECOND_VISIT) ||
		    !(extent.e_flags & EXT2_EXTENT_FLAGS_LEAF))
			continue;

		for (; next_lblk < extent.e_lblk; next_lblk++)
			format->add_block(fs, 0, 0, format->private);

		block = extent.e_pblk;
		for (; next_lblk < extent.e_lblk + extent.e_len; next_lblk++)
			format->add_block(fs, block++, 0, format->private);
	}

	if (retval == EXT2_ET_EXTENT_NO_NEXT)
		retval = 0;
	if (retval) {
		com_err(__func__, retval, ("getting extents of ino \"%u\""),
			ino);
	}
	return retval;
}

static errcode_t ino_iter_blocks(ext2_filsys fs, ext2_ino_t ino,
				 struct walk_ext_priv_data *pdata)
{
	errcode_t retval;
	struct ext2_inode inode;
	ext2_extent_handle_t extents;
	struct fsmap_format *format = pdata->format;

	retval = ext2fs_read_inode(fs, ino, &inode);
	if (retval)
		return retval;

	if (!ext2fs_inode_has_valid_blocks2(fs, &inode))
		return format->inline_data(&(inode.i_block[0]),
					   format->private);

	retval = ext2fs_extent_open(fs, ino, &extents);
	if (retval == EXT2_ET_INODE_NOT_EXTENT) {
		retval = ext2fs_block_iterate3(fs, ino, BLOCK_FLAG_READ_ONLY,
			NULL, walk_block, pdata);
		if (retval) {
			com_err(__func__, retval, _("listing blocks of ino \"%u\""),
				ino);
		}
		return retval;
	}

	retval = ino_iter_extents(fs, ino, extents, pdata);

	ext2fs_extent_free(extents);
	return retval;
}

static int is_dir(ext2_filsys fs, ext2_ino_t ino)
{
	struct ext2_inode inode;

	if (ext2fs_read_inode(fs, ino, &inode))
		return 0;
	return S_ISDIR(inode.i_mode);
}

static int walk_ext_dir(ext2_ino_t dir EXT2FS_ATTR((unused)),
			int flags EXT2FS_ATTR((unused)),
			struct ext2_dir_entry *de,
			int offset EXT2FS_ATTR((unused)),
			int blocksize EXT2FS_ATTR((unused)),
			char *buf EXT2FS_ATTR((unused)), void *priv_data)
{
	errcode_t retval;
	struct ext2_inode inode;
	char *filename, *cur_path, *name = de->name;
	int name_len = de->name_len & 0xff;
	struct walk_ext_priv_data *pdata = priv_data;
	struct fsmap_format *format = pdata->format;

	if (!strncmp(name, ".", name_len)
	    || !strncmp(name, "..", name_len)
	    || !strncmp(name, "lost+found", 10))
		return 0;

	if (asprintf(&filename, "%s/%.*s", pdata->path, name_len, name) < 0)
		return -ENOMEM;

	retval = ext2fs_read_inode(pdata->fs, de->inode, &inode);
	if (retval) {
		com_err(__func__, retval, _("reading ino \"%u\""), de->inode);
		goto end;
	}
	format->start_new_file(filename, de->inode, &inode, format->private);
	retval = ino_iter_blocks(pdata->fs, de->inode, pdata);
	if (retval)
		return retval;
	format->end_new_file(format->private);

	retval = 0;
	if (is_dir(pdata->fs, de->inode)) {
		cur_path = pdata->path;
		pdata->path = filename;
		retval = ext2fs_dir_iterate2(pdata->fs, de->inode, 0, NULL,
				    walk_ext_dir, pdata);
		pdata->path = cur_path;
	}

end:
	free(filename);
	return retval;
}

errcode_t fsmap_iter_filsys(ext2_filsys fs, struct fsmap_format *format,
			    const char *file, const char *mountpoint)
{
	struct walk_ext_priv_data pdata;
	errcode_t retval;

	format->private = format->init(file, mountpoint);
	pdata.fs = fs;
	pdata.path = "";
	pdata.format = format;

	retval = ext2fs_dir_iterate2(fs, EXT2_ROOT_INO, 0, NULL, walk_ext_dir, &pdata);

	format->cleanup(format->private);
	return retval;
}
