#ifndef _CREATE_INODE_H
#define _CREATE_INODE_H

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "et/com_err.h"
#include "e2p/e2p.h"
#include "ext2fs/ext2fs.h"

struct hdlink_s
{
	dev_t src_dev;
	ino_t src_ino;
	ext2_ino_t dst_ino;
};

struct hdlinks_s
{
	int count;
	int size;
	struct hdlink_s *hdl;
};

#define HDLINK_CNT	(4)

struct fs_ops_callbacks {
	errcode_t (* create_new_inode)(ext2_filsys fs, const char *target_path,
		const char *name, ext2_ino_t parent_ino, ext2_ino_t root,
		mode_t mode);
	errcode_t (* end_create_new_inode)(ext2_filsys fs,
		const char *target_path, const char *name,
		ext2_ino_t parent_ino, ext2_ino_t root, mode_t mode);
};

extern int no_copy_xattrs; 	/* this should eventually be a flag
				   passed to populate_fs3() */

/* For populating the filesystem */
extern errcode_t populate_fs(ext2_filsys fs, ext2_ino_t parent_ino,
			     const char *source_dir, ext2_ino_t root);
extern errcode_t populate_fs2(ext2_filsys fs, ext2_ino_t parent_ino,
			      const char *source_dir, ext2_ino_t root,
			      struct fs_ops_callbacks *fs_callbacks);
extern errcode_t do_mknod_internal(ext2_filsys fs, ext2_ino_t cwd,
				   const char *name, unsigned int st_mode,
				   unsigned int st_rdev);
extern errcode_t do_symlink_internal(ext2_filsys fs, ext2_ino_t cwd,
				     const char *name, char *target,
				     ext2_ino_t root);
extern errcode_t do_mkdir_internal(ext2_filsys fs, ext2_ino_t cwd,
				   const char *name, ext2_ino_t root);
extern errcode_t do_write_internal(ext2_filsys fs, ext2_ino_t cwd,
				   const char *src, const char *dest,
				   ext2_ino_t root);

#endif /* _CREATE_INODE_H */
