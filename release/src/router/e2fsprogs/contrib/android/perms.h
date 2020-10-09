#ifndef ANDROID_PERMS_H
# define ANDROID_PERMS_H

# include "config.h"
# include <ext2fs/ext2fs.h>

typedef void (*fs_config_f)(const char *path, int dir,
			    const char *target_out_path,
			    unsigned *uid, unsigned *gid,
			    unsigned *mode, uint64_t *capabilities);

/*
 * Represents a range of UID/GID mapping.
 * This maps the id in [|parent_id|, |parent_id| + |length|) into
 * [|child_id|, |child_id| + |length|)
 */
struct ugid_map_entry {
	unsigned int child_id;
	unsigned int parent_id;
	unsigned int length;
};

struct ugid_map {
	/* The number of elements in |entries|. */
	size_t size;

	/* An array of entries. If |size| is 0, this is a null pointer. */
	struct ugid_map_entry* entries;
};

# ifdef _WIN32
struct selabel_handle;
static inline errcode_t android_configure_fs(ext2_filsys fs,
					     char *src_dir,
					     char *target_out,
					     char *mountpoint,
					     void *seopts,
					     unsigned int nopt,
					     char *fs_config_file,
					     time_t fixed_time,
					     const struct ugid_map* uid_map,
					     const struct ugdi_map* gid_map)
{
	return 0;
}
# else
#  include <selinux/selinux.h>
#  include <selinux/label.h>
#  if defined(__ANDROID__)
#   include <selinux/android.h>
#  endif
#  include <private/android_filesystem_config.h>
#  include <private/canned_fs_config.h>

errcode_t android_configure_fs(ext2_filsys fs, char *src_dir,
			       char *target_out,
			       char *mountpoint,
			       struct selinux_opt *seopts,
			       unsigned int nopt,
			       char *fs_config_file, time_t fixed_time,
			       const struct ugid_map* uid_map,
			       const struct ugid_map* gid_map);

# endif
#endif /* !ANDROID_PERMS_H */
