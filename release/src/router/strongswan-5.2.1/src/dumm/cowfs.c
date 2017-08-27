/*
 * Copyright (C) 2009 Tobias Brunner
 * Copyright (C) 2007 Martin Willi
 * Hochschule fuer Technik Rapperswil
 * Copyright (C) 2001-2007 Miklos Szeredi
 *
 * Based on example shipped with FUSE.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */


#define FUSE_USE_VERSION 26
#define _GNU_SOURCE

#include <fuse.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>

#include "cowfs.h"

#include <library.h>
#include <utils/debug.h>
#include <threading/thread.h>
#include <threading/rwlock.h>
#include <collections/linked_list.h>

/** define _XOPEN_SOURCE 500 fails when using libstrongswan, define popen */
extern ssize_t pread(int fd, void *buf, size_t count, off_t offset);
extern ssize_t pwrite(int fd, const void *buf, size_t count, off_t offset);

typedef struct private_cowfs_t private_cowfs_t;

struct private_cowfs_t {
	/** public cowfs interface */
	cowfs_t public;
	/** fuse channel to mountpoint */
	struct fuse_chan *chan;
	/** fuse handle */
	struct fuse *fuse;
	/** mountpoint of cowfs FUSE */
	char *mount;
	/** master filesystem path */
	char *master;
	/** host filesystem path */
	char *host;
	/** overlay filesystems */
	linked_list_t *overlays;
	/** lock for overlays */
	rwlock_t *lock;
	/** fd of read only master filesystem */
	int master_fd;
	/** copy on write overlay to master */
	int host_fd;
	/** thread processing FUSE */
	thread_t *thread;
};

typedef struct overlay_t overlay_t;

/**
 * data for overlay filesystems
 */
struct overlay_t {
	/** path to overlay */
	char *path;
	/** overlay fd */
	int fd;
};

/**
 * destroy an overlay
 */
static void overlay_destroy(overlay_t *this)
{
	close(this->fd);
	free(this->path);
	free(this);
}

/**
 * compare two overlays by path
 */
static bool overlay_equals(overlay_t *this, overlay_t *other)
{
	return streq(this->path, other->path);
}

/**
 * remove and destroy the overlay with the given absolute path.
 * returns FALSE, if not found.
 */
static bool overlay_remove(private_cowfs_t *this, char *path)
{
	overlay_t over, *current;
	over.path = path;
	if (this->overlays->find_first(this->overlays,
			(linked_list_match_t)overlay_equals, (void**)&current, &over) != SUCCESS)
	{
		return FALSE;
	}
	this->overlays->remove(this->overlays, current, NULL);
	overlay_destroy(current);
	return TRUE;
}

/**
 * get this pointer stored in fuse context
 */
static private_cowfs_t *get_this()
{
	return (fuse_get_context())->private_data;
}

/**
 * make a path relative
 */
static void rel(const char **path)
{
	if (**path == '/')
	{
		(*path)++;
	}
	if (**path == '\0')
	{
		*path = ".";
	}
}

/**
 * get the highest overlay in which path exists
 */
static int get_rd(const char *path)
{
	overlay_t *over;
	enumerator_t *enumerator;
	private_cowfs_t *this = get_this();

	this->lock->read_lock(this->lock);
	enumerator = this->overlays->create_enumerator(this->overlays);
	while (enumerator->enumerate(enumerator, (void**)&over))
	{
		if (faccessat(over->fd, path, F_OK, 0) == 0)
		{
			int fd = over->fd;
			enumerator->destroy(enumerator);
			this->lock->unlock(this->lock);
			return fd;
		}
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);

	if (faccessat(this->host_fd, path, F_OK, 0) == 0)
	{
		return this->host_fd;
	}
	return this->master_fd;
}

/**
 * get the highest overlay available, to write something
 */
static int get_wr(const char *path)
{
	overlay_t *over;
	private_cowfs_t *this = get_this();
	int fd = this->host_fd;
	this->lock->read_lock(this->lock);
	if (this->overlays->get_first(this->overlays, (void**)&over) == SUCCESS)
	{
		fd = over->fd;
	}
	this->lock->unlock(this->lock);
	return fd;
}

/**
 * create full "path" at "wr" the same way they exist at "rd"
 */
static bool clone_path(int rd, int wr, const char *path)
{
	char *pos, *full;
	struct stat st;
	full = strdupa(path);
	pos = full;

	while ((pos = strchr(pos, '/')))
	{
		*pos = '\0';
		if (fstatat(wr, full, &st, 0) < 0)
		{
			/* TODO: handle symlinks!? */
			if (fstatat(rd, full, &st, 0) < 0)
			{
				return FALSE;
			}
			if (mkdirat(wr, full, st.st_mode) < 0)
			{
				return FALSE;
			}
		}
		*pos = '/';
		pos++;
	}
	return TRUE;
}

/**
 * copy a (special) file from a readonly to a read-write overlay
 */
static int copy(const char *path)
{
	char *buf[4096];
	int len;
	int rd, wr;
	int from, to;
	struct stat st;

	rd = get_rd(path);
	wr = get_wr(path);

	if (rd == wr)
	{
		/* already writeable */
		return wr;
	}
	if (fstatat(rd, path, &st, 0) < 0)
	{
		return -1;
	}
	if (!clone_path(rd, wr, path))
	{
		return -1;
	}
	if (mknodat(wr, path, st.st_mode, st.st_rdev) < 0)
	{
		return -1;
	}
	/* copy if no special file */
	if (st.st_size)
	{
		from = openat(rd, path, O_RDONLY, st.st_mode);
		if (from < 0)
		{
			return -1;
		}
		to = openat(wr, path, O_WRONLY , st.st_mode);
		if (to < 0)
		{
			close(from);
			return -1;
		}
		while ((len = read(from, buf, sizeof(buf))) > 0)
		{
			if (write(to, buf, len) < len)
			{
				/* TODO: only on len < 0 ? */
				close(from);
				close(to);
				return -1;
			}
		}
		close(from);
		close(to);
		if (len < 0)
		{
			return -1;
		}
	}
	return wr;
}

/**
 * FUSE getattr method
 */
static int cowfs_getattr(const char *path, struct stat *stbuf)
{
	rel(&path);

	if (fstatat(get_rd(path), path, stbuf, AT_SYMLINK_NOFOLLOW) < 0)
	{
		return -errno;
	}
	return 0;
}

/**
 * FUSE access method
 */
static int cowfs_access(const char *path, int mask)
{
	rel(&path);

	if (faccessat(get_rd(path), path, mask, 0) < 0)
	{
		return -errno;
	}
	return 0;
}

/**
 * FUSE readlink method
 */
static int cowfs_readlink(const char *path, char *buf, size_t size)
{
	int res;

	rel(&path);

	res = readlinkat(get_rd(path), path, buf, size - 1);
	if (res < 0)
	{
		return -errno;
	}
	buf[res] = '\0';
	return 0;
}

/**
 * get a directory stream of two concatenated paths
 */
static DIR* get_dir(char *dir, const char *subdir)
{
	char *full;

	if (dir == NULL)
	{
		return NULL;
	}

	full = alloca(strlen(dir) + strlen(subdir) + 1);
	strcpy(full, dir);
	strcat(full, subdir);

	return opendir(full);
}

/**
 * check if a directory stream contains a directory
 */
static bool contains_dir(DIR *d, char *dirname)
{
	struct dirent *ent;

	rewinddir(d);
	while ((ent = readdir(d)))
	{
		if (streq(ent->d_name, dirname))
		{
			return TRUE;
		}
	}
	return FALSE;
}

/**
 * check if one of the higher overlays contains a directory
 */
static bool overlays_contain_dir(DIR **d, char *dirname)
{
	for (; *d; ++d)
	{
		if (contains_dir(*d, dirname))
		{
			return TRUE;
		}
	}
	return FALSE;
}

/**
 * FUSE readdir method
 */
static int cowfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
						 off_t offset, struct fuse_file_info *fi)
{
#define ADD_DIR(overlay, base, path) ({\
	DIR *dir = get_dir(base, path);\
	if (dir) { *(--overlay) = dir; }\
})
	private_cowfs_t *this = get_this();
	int count;
	DIR **d, **overlays;
	struct stat st;
	struct dirent *ent;
	overlay_t *over;
	enumerator_t *enumerator;

	memset(&st, 0, sizeof(st));

	this->lock->read_lock(this->lock);
	/* create a null-terminated array of DIR objects for all overlays (including
	 * the master and host layer). the order is from bottom to top */
	count = this->overlays->get_count(this->overlays) + 2;
	overlays = calloc(count + 1, sizeof(DIR*));
	d = &overlays[count];

	enumerator = this->overlays->create_enumerator(this->overlays);
	while (enumerator->enumerate(enumerator, (void**)&over))
	{
		ADD_DIR(d, over->path, path);
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);

	ADD_DIR(d, this->host, path);
	ADD_DIR(d, this->master, path);

	for (; *d; ++d)
	{
		rewinddir(*d);
		while((ent = readdir(*d)))
		{
			if (!overlays_contain_dir(d + 1, ent->d_name))
			{
				st.st_ino = ent->d_ino;
				st.st_mode = ent->d_type << 12;
				filler(buf, ent->d_name, &st, 0);
			}
		}
		closedir(*d);
	}

	free(overlays);
	return 0;
}

/**
 * FUSE mknod method
 */
static int cowfs_mknod(const char *path, mode_t mode, dev_t rdev)
{
	int fd;
	rel(&path);

	fd = get_wr(path);
	if (!clone_path(get_rd(path), fd, path))
	{
		return -errno;
	}

	if (mknodat(fd, path, mode, rdev) < 0)
	{
		return -errno;
	}
	return 0;
}

/**
 * FUSE mkdir method
 */
static int cowfs_mkdir(const char *path, mode_t mode)
{
	int fd;
	rel(&path);

	fd = get_wr(path);
	if (!clone_path(get_rd(path), fd, path))
	{
		return -errno;
	}
	if (mkdirat(fd, path, mode) < 0)
	{
		return -errno;
	}
	return 0;
}

/**
 * FUSE unlink method
 */
static int cowfs_unlink(const char *path)
{
	rel(&path);

	/* TODO: whiteout master */
	if (unlinkat(get_wr(path), path, 0) < 0)
	{
		return -errno;
	}
	return 0;
}

/**
 * FUSE rmdir method
 */
static int cowfs_rmdir(const char *path)
{
	rel(&path);

	/* TODO: whiteout master */
	if (unlinkat(get_wr(path), path, AT_REMOVEDIR) < 0)
	{
		return -errno;
	}
	return 0;
}

/**
 * FUSE symlink method
 */
static int cowfs_symlink(const char *from, const char *to)
{
	int fd;
	const char *fromrel = from;

	rel(&to);
	rel(&fromrel);

	fd = get_wr(to);
	if (!clone_path(get_rd(fromrel), fd, fromrel))
	{
		return -errno;
	}
	if (symlinkat(from, fd, to) < 0)
	{
		return -errno;
	}
	return 0;
}

/**
 * FUSE rename method
 */
static int cowfs_rename(const char *from, const char *to)
{
	int fd;

	rel(&from);
	rel(&to);

	fd = copy(from);
	if (fd < 0)
	{
		return -errno;
	}
	if (renameat(fd, from, get_wr(to), to) < 0)
	{
		return -errno;
	}
	return 0;
}

/**
 * FUSE link method
 */
static int cowfs_link(const char *from, const char *to)
{
	int rd, wr;

	rel(&from);
	rel(&to);

	rd = get_rd(from);
	wr = get_wr(to);

	if (!clone_path(rd, wr, to))
	{
		DBG1(DBG_LIB, "cloning path '%s' failed", to);
		return -errno;
	}
	if (linkat(rd, from, wr, to, 0) < 0)
	{
		DBG1(DBG_LIB, "linking '%s' to '%s' failed", from, to);
		return -errno;
	}
	return 0;
}

/**
 * FUSE chmod method
 */
static int cowfs_chmod(const char *path, mode_t mode)
{
	int fd;
	struct stat st;

	rel(&path);
	fd = get_rd(path);
	if (fstatat(fd, path, &st, 0) < 0)
	{
		return -errno;
	}
	if (st.st_mode == mode)
	{
		return 0;
	}
	fd = copy(path);
	if (fd < 0)
	{
		return -errno;
	}
	if (fchmodat(fd, path, mode, 0) < 0)
	{
		return -errno;
	}
	return 0;
}

/**
 * FUSE chown method
 */
static int cowfs_chown(const char *path, uid_t uid, gid_t gid)
{
	int fd;
	struct stat st;

	rel(&path);
	fd = get_rd(path);
	if (fstatat(fd, path, &st, 0) < 0)
	{
		return -errno;
	}
	if (st.st_uid == uid && st.st_gid == gid)
	{
		return 0;
	}
	fd = copy(path);
	if (fd < 0)
	{
		return -errno;
	}
	if (fchownat(fd, path, uid, gid, AT_SYMLINK_NOFOLLOW) < 0)
	{
		return -errno;
	}
	return 0;
}

/**
 * FUSE truncate method
 */
static int cowfs_truncate(const char *path, off_t size)
{
	int fd;
	struct stat st;

	rel(&path);
	fd = get_rd(path);
	if (fstatat(fd, path, &st, 0) < 0)
	{
		return -errno;
	}
	if (st.st_size == size)
	{
		return 0;
	}
	fd = copy(path);
	if (fd < 0)
	{
		return -errno;
	}
	fd = openat(fd, path, O_WRONLY);
	if (fd < 0)
	{
		return -errno;
	}
	if (ftruncate(fd, size) < 0)
	{
		close(fd);
		return -errno;
	}
	close(fd);
	return 0;
}

/**
 * FUSE utimens method
 */
static int cowfs_utimens(const char *path, const struct timespec ts[2])
{
	struct timeval tv[2];
	int fd;

	rel(&path);
	fd = copy(path);
	if (fd < 0)
	{
		return -errno;
	}

	tv[0].tv_sec = ts[0].tv_sec;
	tv[0].tv_usec = ts[0].tv_nsec / 1000;
	tv[1].tv_sec = ts[1].tv_sec;
	tv[1].tv_usec = ts[1].tv_nsec / 1000;

	if (futimesat(fd, path, tv) < 0)
	{
		return -errno;
	}
	return 0;
}

/**
 * FUSE open method
 */
static int cowfs_open(const char *path, struct fuse_file_info *fi)
{
	int fd;

	rel(&path);
	fd = get_rd(path);

	fd = openat(fd, path, fi->flags);
	if (fd < 0)
	{
		return -errno;
	}
	close(fd);
	return 0;
}

/**
 * FUSE read method
 */
static int cowfs_read(const char *path, char *buf, size_t size, off_t offset,
					  struct fuse_file_info *fi)
{
	int file, fd, res;

	rel(&path);

	fd = get_rd(path);

	file = openat(fd, path, O_RDONLY);
	if (file < 0)
	{
		return -errno;
	}

	res = pread(file, buf, size, offset);
	if (res < 0)
	{
		res = -errno;
	}
	close(file);
	return res;
}

/**
 * FUSE write method
 */
static int cowfs_write(const char *path, const char *buf, size_t size,
					   off_t offset, struct fuse_file_info *fi)
{
	int file, fd, res;

	rel(&path);

	fd = copy(path);
	if (fd < 0)
	{
		return -errno;
	}
	file = openat(fd, path, O_WRONLY);
	if (file < 0)
	{
		return -errno;
	}
	res = pwrite(file, buf, size, offset);
	if (res < 0)
	{
		res = -errno;
	}
	close(file);
	return res;
}

/**
 * FUSE statfs method
 */
static int cowfs_statfs(const char *path, struct statvfs *stbuf)
{
	int fd;

	fd = get_rd(path);
	if (fstatvfs(fd, stbuf) < 0)
	{
		return -errno;
	}

	return 0;
}

/**
 * FUSE init method
 */
static void *cowfs_init(struct fuse_conn_info *conn)
{
	struct fuse_context *ctx;

	ctx = fuse_get_context();

	return ctx->private_data;
}

/**
 * FUSE method vectors
 */
static struct fuse_operations cowfs_operations = {
	.getattr	= cowfs_getattr,
	.access		= cowfs_access,
	.readlink	= cowfs_readlink,
	.readdir	= cowfs_readdir,
	.mknod		= cowfs_mknod,
	.mkdir		= cowfs_mkdir,
	.symlink	= cowfs_symlink,
	.unlink		= cowfs_unlink,
	.rmdir		= cowfs_rmdir,
	.rename		= cowfs_rename,
	.link		= cowfs_link,
	.chmod		= cowfs_chmod,
	.chown		= cowfs_chown,
	.truncate	= cowfs_truncate,
	.utimens	= cowfs_utimens,
	.open		= cowfs_open,
	.read		= cowfs_read,
	.write		= cowfs_write,
	.statfs		= cowfs_statfs,
	.init		= cowfs_init,
};

METHOD(cowfs_t, add_overlay, bool,
	private_cowfs_t *this, char *path)
{
	overlay_t *over = malloc_thing(overlay_t);
	over->fd = open(path, O_RDONLY | O_DIRECTORY);
	if (over->fd < 0)
	{
		DBG1(DBG_LIB, "failed to open overlay directory '%s': %m", path);
		free(over);
		return FALSE;
	}
	over->path = realpath(path, NULL);
	this->lock->write_lock(this->lock);
	overlay_remove(this, over->path);
	this->overlays->insert_first(this->overlays, over);
	this->lock->unlock(this->lock);
	return TRUE;
}

METHOD(cowfs_t, del_overlay, bool,
	private_cowfs_t *this, char *path)
{
	bool removed;
	char real[PATH_MAX];
	this->lock->write_lock(this->lock);
	removed = overlay_remove(this, realpath(path, real));
	this->lock->unlock(this->lock);
	return removed;
}

METHOD(cowfs_t, pop_overlay, bool,
	private_cowfs_t *this)
{
	overlay_t *over;
	this->lock->write_lock(this->lock);
	if (this->overlays->remove_first(this->overlays, (void**)&over) != SUCCESS)
	{
		this->lock->unlock(this->lock);
		return FALSE;
	}
	this->lock->unlock(this->lock);
	overlay_destroy(over);
	return TRUE;
}

METHOD(cowfs_t, destroy, void,
	private_cowfs_t *this)
{
	fuse_exit(this->fuse);
	fuse_unmount(this->mount, this->chan);
	this->thread->join(this->thread);
	fuse_destroy(this->fuse);
	this->lock->destroy(this->lock);
	this->overlays->destroy_function(this->overlays, (void*)overlay_destroy);
	free(this->mount);
	free(this->master);
	free(this->host);
	close(this->master_fd);
	close(this->host_fd);
	free(this);
}

/**
 * creates a new cowfs fuse instance
 */
cowfs_t *cowfs_create(char *master, char *host, char *mount)
{
	struct fuse_args args = {0, NULL, 0};
	private_cowfs_t *this;

	INIT(this,
		.public = {
			.add_overlay = _add_overlay,
			.del_overlay = _del_overlay,
			.pop_overlay = _pop_overlay,
			.destroy = _destroy,
		}
	);

	this->master_fd = open(master, O_RDONLY | O_DIRECTORY);
	if (this->master_fd < 0)
	{
		DBG1(DBG_LIB, "failed to open master filesystem '%s'", master);
		free(this);
		return NULL;
	}
	this->host_fd = open(host, O_RDONLY | O_DIRECTORY);
	if (this->host_fd < 0)
	{
		DBG1(DBG_LIB, "failed to open host filesystem '%s'", host);
		close(this->master_fd);
		free(this);
		return NULL;
	}

	this->chan = fuse_mount(mount, &args);
	if (this->chan == NULL)
	{
		DBG1(DBG_LIB, "mounting cowfs FUSE on '%s' failed", mount);
		close(this->master_fd);
		close(this->host_fd);
		free(this);
		return NULL;
	}

	this->fuse = fuse_new(this->chan, &args, &cowfs_operations,
						  sizeof(cowfs_operations), this);
	if (this->fuse == NULL)
	{
		DBG1(DBG_LIB, "creating cowfs FUSE handle failed");
		close(this->master_fd);
		close(this->host_fd);
		fuse_unmount(mount, this->chan);
		free(this);
		return NULL;
	}

	this->mount = strdup(mount);
	this->master = strdup(master);
	this->host = strdup(host);
	this->overlays = linked_list_create();
	this->lock = rwlock_create(RWLOCK_TYPE_DEFAULT);

	this->thread = thread_create((thread_main_t)fuse_loop, this->fuse);
	if (!this->thread)
	{
		DBG1(DBG_LIB, "creating thread to handle FUSE failed");
		fuse_unmount(mount, this->chan);
		this->lock->destroy(this->lock);
		this->overlays->destroy(this->overlays);
		free(this->mount);
		free(this->master);
		free(this->host);
		close(this->master_fd);
		close(this->host_fd);
		free(this);
		return NULL;
	}

	return &this->public;
}

