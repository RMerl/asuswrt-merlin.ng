#ifndef BASE_FS_ALLOCATOR_H
# define BASE_FS_ALLOCATOR_H

# include <time.h>
# include <ext2fs/ext2fs.h>

errcode_t base_fs_alloc_load(ext2_filsys fs, const char *file,
			     const char *mountpoint, const char *src_dir);
void base_fs_alloc_cleanup(ext2_filsys fs);

errcode_t base_fs_alloc_set_target(ext2_filsys fs, const char *target_path,
	const char *name, ext2_ino_t parent_ino, ext2_ino_t root, mode_t mode);
errcode_t base_fs_alloc_unset_target(ext2_filsys fs, const char *target_path,
	const char *name, ext2_ino_t parent_ino, ext2_ino_t root, mode_t mode);

#endif /* !BASE_FS_ALLOCATOR_H */
