#ifndef FSMAP_H
# define FSMAP_H

# ifndef _GNU_SOURCE
#  define _GNU_SOURCE // asprintf
# endif
# include <stdio.h>
# include <stdint.h>
# include <stdbool.h>
# include <sys/types.h>
# include <ext2fs/ext2fs.h>

struct fsmap_format {
	void* (* init)(const char *file, const char *mountpoint);
	int   (* start_new_file)(char *path, ext2_ino_t ino,
				 struct ext2_inode *inode, void *data);
	int   (* add_block)(ext2_filsys fs, blk64_t blocknr, int metadata,
			    void *data);
	int   (* inline_data)(void *inline_data, void *data);
	int   (* end_new_file)(void *data);
	int   (* cleanup)(void *data);

	void *private;
};

errcode_t fsmap_iter_filsys(ext2_filsys fs, struct fsmap_format *format,
			    const char *file, const char *mountpoint);

#endif /* !FSMAP_H */
