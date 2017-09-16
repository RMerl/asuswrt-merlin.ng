/*
 * Copyright 2009 Oracle.  All rights reserved.
 *
 * This file is part of nfs-utils.
 *
 * nfs-utils is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * nfs-utils is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with nfs-utils.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * NSM for Linux.
 *
 * Callback information and NSM state is stored in files, usually
 * under /var/lib/nfs.  A database of information contained in local
 * files stores NLM callback data and what remote peers to notify of
 * reboots.
 *
 * For each monitored remote peer, a text file is created under the
 * directory specified by NSM_MONITOR_DIR.  The name of the file
 * is a valid DNS hostname.  The hostname string must be a valid
 * ASCII DNS name, and must not contain slash characters, white space,
 * or '\0' (ie. anything that might have some special meaning in a
 * path name).
 *
 * The contents of each file include seven blank-separated fields of
 * text, finished with '\n'.  The first field contains the network
 * address of the NLM service to call back.  The current implementation
 * supports using only IPv4 addresses, so the only contents of this
 * field are a network order IPv4 address expressed in 8 hexadecimal
 * characters.
 *
 * The next four fields are text strings of hexadecimal characters,
 * representing:
 *
 * 2. A 4 byte RPC program number of the NLM service to call back
 * 3. A 4 byte RPC version number of the NLM service to call back
 * 4. A 4 byte RPC procedure number of the NLM service to call back
 * 5. A 16 byte opaque cookie that the NLM service uses to identify
 *    the monitored host
 *
 * The sixth field is the monitored host's mon_name, passed to statd
 * via an SM_MON request.
 *
 * The seventh field is the my_name for this peer, which is the
 * hostname of the local NLM (currently on Linux, the result of
 * `uname -n`).  This can be used as the source address/hostname
 * when sending SM_NOTIFY requests.
 *
 * The NSM protocol does not limit the contents of these strings
 * in any way except that they must fit into 1024 bytes.  Our
 * implementation requires that these strings not contain
 * white space or '\0'.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#ifdef HAVE_SYS_CAPABILITY_H
#include <sys/capability.h>
#endif
#include <sys/prctl.h>
#include <sys/stat.h>

#include <ctype.h>
#include <string.h>
#include <stdint.h>
#ifndef S_SPLINT_S
#include <unistd.h>
#endif
#include <libgen.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <grp.h>

#include "xlog.h"
#include "nsm.h"

#define RPCARGSLEN	(4 * (8 + 1))
#define LINELEN		(RPCARGSLEN + SM_PRIV_SIZE * 2 + 1)

#define NSM_KERNEL_STATE_FILE	"/proc/sys/fs/nfs/nsm_local_state"

static char nsm_base_dirname[PATH_MAX] = NSM_DEFAULT_STATEDIR;

#define NSM_MONITOR_DIR	"sm"
#define NSM_NOTIFY_DIR	"sm.bak"
#define NSM_STATE_FILE	"state"


static _Bool
error_check(const int len, const size_t buflen)
{
	return (len < 0) || ((size_t)len >= buflen);
}

static _Bool
exact_error_check(const ssize_t len, const size_t buflen)
{
	return (len < 0) || ((size_t)len != buflen);
}

/*
 * Returns a dynamically allocated, '\0'-terminated buffer
 * containing an appropriate pathname, or NULL if an error
 * occurs.  Caller must free the returned result with free(3).
 */
__attribute__((__malloc__))
static char *
nsm_make_record_pathname(const char *directory, const char *hostname)
{
	const char *c;
	size_t size;
	char *path;
	int len;

	/*
	 * Block hostnames that contain characters that have
	 * meaning to the file system (like '/'), or that can
	 * be confusing on visual inspection (like ' ').
	 */
	for (c = hostname; *c != '\0'; c++)
		if (*c == '/' || isspace((int)*c) != 0) {
			xlog(D_GENERAL, "Hostname contains invalid characters");
			return NULL;
		}

	size = strlen(nsm_base_dirname) + strlen(directory) + strlen(hostname) + 3;
	if (size > PATH_MAX) {
		xlog(D_GENERAL, "Hostname results in pathname that is too long");
		return NULL;
	}

	path = malloc(size);
	if (path == NULL) {
		xlog(D_GENERAL, "Failed to allocate memory for pathname");
		return NULL;
	}

	len = snprintf(path, size, "%s/%s/%s",
			nsm_base_dirname, directory, hostname);
	if (error_check(len, size)) {
		xlog(D_GENERAL, "Pathname did not fit in specified buffer");
		free(path);
		return NULL;
	}

	return path;
}

/*
 * Returns a dynamically allocated, '\0'-terminated buffer
 * containing an appropriate pathname, or NULL if an error
 * occurs.  Caller must free the returned result with free(3).
 */
__attribute__((__malloc__))
static char *
nsm_make_pathname(const char *directory)
{
	size_t size;
	char *path;
	int len;

	size = strlen(nsm_base_dirname) + strlen(directory) + 2;
	if (size > PATH_MAX)
		return NULL;

	path = malloc(size);
	if (path == NULL)
		return NULL;

	len = snprintf(path, size, "%s/%s", nsm_base_dirname, directory);
	if (error_check(len, size)) {
		free(path);
		return NULL;
	}

	return path;
}

/*
 * Returns a dynamically allocated, '\0'-terminated buffer
 * containing an appropriate pathname, or NULL if an error
 * occurs.  Caller must free the returned result with free(3).
 */
__attribute__((__malloc__))
static char *
nsm_make_temp_pathname(const char *pathname)
{
	size_t size;
	char *path;
	int len;

	size = strlen(pathname) + sizeof(".new") + 2;
	if (size > PATH_MAX)
		return NULL;

	path = malloc(size);
	if (path == NULL)
		return NULL;

	len = snprintf(path, size, "%s.new", pathname);
	if (error_check(len, size)) {
		free(path);
		return NULL;
	}

	return path;
}

/*
 * Use "mktemp, write, rename" to update the contents of a file atomically.
 *
 * Returns true if completely successful, or false if some error occurred.
 */
static _Bool
nsm_atomic_write(const char *path, const void *buf, const size_t buflen)
{
	_Bool result = false;
	ssize_t len;
	char *temp;
	int fd;

	temp = nsm_make_temp_pathname(path);
	if (temp == NULL) {
		xlog(L_ERROR, "Failed to create new path for %s", path);
		goto out;
	}

	fd = open(temp, O_CREAT | O_TRUNC | O_SYNC | O_WRONLY, 0644);
	if (fd == -1) {
		xlog(L_ERROR, "Failed to create %s: %m", temp);
		goto out;
	}

	len = write(fd, buf, buflen);
	if (exact_error_check(len, buflen)) {
		xlog(L_ERROR, "Failed to write %s: %m", temp);
		(void)close(fd);
		(void)unlink(temp);
		goto out;
	}

	if (close(fd) == -1) {
		xlog(L_ERROR, "Failed to close %s: %m", temp);
		(void)unlink(temp);
		goto out;
	}

	if (rename(temp, path) == -1) {
		xlog(L_ERROR, "Failed to rename %s -> %s: %m",
				temp, path);
		(void)unlink(temp);
		goto out;
	}

	/* Ostensibly, a sync(2) is not needed here because
	 * open(O_CREAT), write(O_SYNC), and rename(2) are
	 * already synchronous with persistent storage, for
	 * any file system we care about. */

	result = true;

out:
	free(temp);
	return result;
}

/**
 * nsm_setup_pathnames - set up pathname
 * @progname: C string containing name of program, for error messages
 * @parentdir: C string containing pathname to on-disk state, or NULL
 *
 * This runs before logging is set up, so error messages are directed
 * to stderr.
 *
 * Returns true and sets up our pathnames, if @parentdir was valid
 * and usable; otherwise false is returned.
 */
_Bool
nsm_setup_pathnames(const char *progname, const char *parentdir)
{
	static char buf[PATH_MAX];
	struct stat st;
	char *path;

	/* First: test length of name and whether it exists */
	if (lstat(parentdir, &st) == -1) {
		(void)fprintf(stderr, "%s: Failed to stat %s: %s",
				progname, parentdir, strerror(errno));
		return false;
	}

	/* Ensure we have a clean directory pathname */
	strncpy(buf, parentdir, sizeof(buf));
	path = dirname(buf);
	if (*path == '.') {
		(void)fprintf(stderr, "%s: Unusable directory %s",
				progname, parentdir);
		return false;
	}

	xlog(D_CALL, "Using %s as the state directory", parentdir);
	strncpy(nsm_base_dirname, parentdir, sizeof(nsm_base_dirname));
	return true;
}

/**
 * nsm_is_default_parentdir - check if parent directory is default
 *
 * Returns true if the active statd parent directory, set by
 * nsm_change_pathname(), is the same as the built-in default
 * parent directory; otherwise false is returned.
 */
_Bool
nsm_is_default_parentdir(void)
{
	return strcmp(nsm_base_dirname, NSM_DEFAULT_STATEDIR) == 0;
}

/*
 * Clear all capabilities but CAP_NET_BIND_SERVICE.  This permits
 * callers to acquire privileged source ports, but all other root
 * capabilities are disallowed.
 *
 * Returns true if successful, or false if some error occurred.
 */
#ifdef HAVE_SYS_CAPABILITY_H
static _Bool
nsm_clear_capabilities(void)
{
	cap_t caps;

	caps = cap_from_text("cap_net_bind_service=ep");
	if (caps == NULL) {
		xlog(L_ERROR, "Failed to allocate capability: %m");
		return false;
	}

	if (cap_set_proc(caps) == -1) {
		xlog(L_ERROR, "Failed to set capability flags: %m");
		(void)cap_free(caps);
		return false;
	}

	(void)cap_free(caps);
	return true;
}

#define CAP_BOUND_PROCFILE "/proc/sys/kernel/cap-bound"
static _Bool
prune_bounding_set(void)
{
#ifdef PR_CAPBSET_DROP
	int ret;
	unsigned long i;
	struct stat st;

	/*
	 * Prior to kernel 2.6.25, the capabilities bounding set was a global
	 * value. Check to see if /proc/sys/kernel/cap-bound exists and don't
	 * bother to clear the bounding set if it does.
	 */
	ret = stat(CAP_BOUND_PROCFILE, &st);
	if (!ret) {
		xlog(L_WARNING, "%s exists. Not attempting to clear "
				"capabilities bounding set.",
				CAP_BOUND_PROCFILE);
		return true;
	} else if (errno != ENOENT) {
		/* Warn, but attempt to clear the bounding set anyway. */
		xlog(L_WARNING, "Unable to stat %s: %m", CAP_BOUND_PROCFILE);
	}

	/* prune the bounding set to nothing */
	for (i = 0; prctl(PR_CAPBSET_READ, i, 0, 0, 0) >=0 ; ++i) {
		ret = prctl(PR_CAPBSET_DROP, i, 0, 0, 0);
		if (ret) {
			xlog(L_ERROR, "Unable to prune capability %lu from "
				      "bounding set: %m", i);
			return false;
		}
	}
#endif /* PR_CAPBSET_DROP */
	return true;
}
#else /* !HAVE_SYS_CAPABILITY_H */
static _Bool
nsm_clear_capabilities(void)
{
	return true;
}

static _Bool
prune_bounding_set(void)
{
	return true;
}
#endif /* HAVE_SYS_CAPABILITY_H */

/**
 * nsm_drop_privileges - drop root privileges
 * @pidfd: file descriptor of a pid file
 *
 * Returns true if successful, or false if some error occurred.
 *
 * Set our effective UID and GID to that of our on-disk database.
 */
_Bool
nsm_drop_privileges(const int pidfd)
{
	struct stat st;

	(void)umask(S_IRWXO);

	/*
	 * XXX: If we can't stat dirname, or if dirname is owned by
	 *      root, we should use "statduser" instead, which is set up
	 *      by configure.ac.  Nothing in nfs-utils seems to use
	 *      "statduser," though.
	 */
	if (lstat(nsm_base_dirname, &st) == -1) {
		xlog(L_ERROR, "Failed to stat %s: %m", nsm_base_dirname);
		return false;
	}

	if (chdir(nsm_base_dirname) == -1) {
		xlog(L_ERROR, "Failed to change working directory to %s: %m",
				nsm_base_dirname);
		return false;
	}

	if (!prune_bounding_set())
		return false;

	if (st.st_uid == 0) {
		xlog_warn("Running as root.  "
			"chown %s to choose different user", nsm_base_dirname);
		return true;
	}

	/*
	 * If the pidfile happens to reside on NFS, dropping privileges
	 * will probably cause us to lose access, even though we are
	 * holding it open.  Chown it to prevent this.
	 */
	if (pidfd >= 0)
		if (fchown(pidfd, st.st_uid, st.st_gid) == -1)
			xlog_warn("Failed to change owner of pidfile: %m");

	/*
	 * Don't clear capabilities when dropping root.
	 */
        if (prctl(PR_SET_KEEPCAPS, 1, 0, 0, 0) == -1) {
                xlog(L_ERROR, "prctl(PR_SET_KEEPCAPS) failed: %m");
		return false;
	}

	if (setgroups(0, NULL) == -1) {
		xlog(L_ERROR, "Failed to drop supplementary groups: %m");
		return false;
	}

	/*
	 * ORDER
	 *
	 * setgid(2) first, as setuid(2) may remove privileges needed
	 * to set the group id.
	 */
	if (setgid(st.st_gid) == -1 || setuid(st.st_uid) == -1) {
		xlog(L_ERROR, "Failed to drop privileges: %m");
		return false;
	}

	xlog(D_CALL, "Effective UID, GID: %u, %u", st.st_uid, st.st_gid);

	return nsm_clear_capabilities();
}

/**
 * nsm_get_state - retrieve on-disk NSM state number
 *
 * Returns an odd NSM state number read from disk, or an initial
 * state number.  Zero is returned if some error occurs.
 */
int
nsm_get_state(_Bool update)
{
	int fd, state = 0;
	ssize_t result;
	char *path = NULL;

	path = nsm_make_pathname(NSM_STATE_FILE);
	if (path == NULL) {
		xlog(L_ERROR, "Failed to allocate path for " NSM_STATE_FILE);
		goto out;
	}

	fd = open(path, O_RDONLY);
	if (fd == -1) {
		if (errno != ENOENT) {
			xlog(L_ERROR, "Failed to open %s: %m", path);
			goto out;
		}

		xlog(L_NOTICE, "Initializing NSM state");
		state = 1;
		update = true;
		goto update;
	}

	result = read(fd, &state, sizeof(state));
	if (exact_error_check(result, sizeof(state))) {
		xlog_warn("Failed to read %s: %m", path);

		xlog(L_NOTICE, "Initializing NSM state");
		state = 1;
		update = true;
		goto update;
	}

	if ((state & 1) == 0)
		state++;

update:
	if(fd >= 0)
		(void)close(fd);

	if (update) {
		state += 2;
		if (!nsm_atomic_write(path, &state, sizeof(state)))
			state = 0;
	}

out:
	free(path);
	return state;
}

/**
 * nsm_update_kernel_state - attempt to post new NSM state to kernel
 * @state: NSM state number
 *
 */
void
nsm_update_kernel_state(const int state)
{
	ssize_t result;
	char buf[20];
	int fd, len;

	fd = open(NSM_KERNEL_STATE_FILE, O_WRONLY);
	if (fd == -1) {
		xlog(D_GENERAL, "Failed to open " NSM_KERNEL_STATE_FILE ": %m");
		return;
	}

	len = snprintf(buf, sizeof(buf), "%d", state);
	if (error_check(len, sizeof(buf))) {
		xlog_warn("Failed to form NSM state number string");
		return;
	}

	result = write(fd, buf, strlen(buf));
	if (exact_error_check(result, strlen(buf)))
		xlog_warn("Failed to write NSM state number: %m");

	if (close(fd) == -1)
		xlog(L_ERROR, "Failed to close NSM state file "
				NSM_KERNEL_STATE_FILE ": %m");
}

/**
 * nsm_retire_monitored_hosts - back up all hosts from "sm/" to "sm.bak/"
 *
 * Returns the count of host records that were moved.
 *
 * Note that if any error occurs during this process, some monitor
 * records may be left in the "sm" directory.
 */
unsigned int
nsm_retire_monitored_hosts(void)
{
	unsigned int count = 0;
	struct dirent *de;
	char *path;
	DIR *dir;

	path = nsm_make_pathname(NSM_MONITOR_DIR);
	if (path == NULL) {
		xlog(L_ERROR, "Failed to allocate path for " NSM_MONITOR_DIR);
		return count;
	}

	dir = opendir(path);
	free(path);
	if (dir == NULL) {
		xlog_warn("Failed to open " NSM_MONITOR_DIR ": %m");
		return count;
	}

	while ((de = readdir(dir)) != NULL) {
		char *src, *dst;
		struct stat stb;

		if (de->d_name[0] == '.')
			continue;

		src = nsm_make_record_pathname(NSM_MONITOR_DIR, de->d_name);
		if (src == NULL) {
			xlog_warn("Bad monitor file name, skipping");
			continue;
		}

		/* NB: not all file systems fill in d_type correctly */
		if (lstat(src, &stb) == -1) {
			xlog_warn("Bad monitor file %s, skipping: %m",
					de->d_name);
			free(src);
			continue;
		}
		if (!S_ISREG(stb.st_mode)) {
			xlog(D_GENERAL, "Skipping non-regular file %s",
					de->d_name);
			free(src);
			continue;
		}

		dst = nsm_make_record_pathname(NSM_NOTIFY_DIR, de->d_name);
		if (dst == NULL) {
			free(src);
			xlog_warn("Bad notify file name, skipping");
			continue;
		}

		if (rename(src, dst) == -1)
			xlog_warn("Failed to rename %s -> %s: %m",
				src, dst);
		else {
			xlog(D_GENERAL, "Retired record for mon_name %s",
					de->d_name);
			count++;
		}

		free(dst);
		free(src);
	}

	(void)closedir(dir);
	return count;
}

/*
 * nsm_priv_to_hex - convert a NSM private cookie to a hex string.
 *
 * @priv: buffer holding the binary NSM private cookie
 * @buf: output buffer for NULL terminated hex string
 * @buflen: size of output buffer
 *
 * Returns the length of the resulting string or 0 on error
 */
size_t
nsm_priv_to_hex(const char *priv, char *buf, const size_t buflen)
{
	int i, len;
	size_t remaining = buflen;

	for (i = 0; i < SM_PRIV_SIZE; i++) {
		len = snprintf(buf, remaining, "%02x",
				(unsigned int)(0xff & priv[i]));
		if (error_check(len, remaining))
			return 0;
		buf += len;
		remaining -= (size_t)len;
	}

	return buflen - remaining;
}

/*
 * Returns the length in bytes of the created record.
 */
__attribute__((__noinline__))
static size_t
nsm_create_monitor_record(char *buf, const size_t buflen,
		const struct sockaddr *sap, const struct mon *m)
{
	const struct sockaddr_in *sin = (const struct sockaddr_in *)sap;
	size_t hexlen, remaining = buflen;
	int len;

	len = snprintf(buf, remaining, "%08x %08x %08x %08x ",
			(unsigned int)sin->sin_addr.s_addr,
			(unsigned int)m->mon_id.my_id.my_prog,
			(unsigned int)m->mon_id.my_id.my_vers,
			(unsigned int)m->mon_id.my_id.my_proc);
	if (error_check(len, remaining))
		return 0;
	buf += len;
	remaining -= (size_t)len;

	hexlen = nsm_priv_to_hex(m->priv, buf, remaining);
	if (hexlen == 0)
		return 0;
	buf += hexlen;
	remaining -= hexlen;

	len = snprintf(buf, remaining, " %s %s\n",
			m->mon_id.mon_name, m->mon_id.my_id.my_name);
	if (error_check(len, remaining))
		return 0;
	remaining -= (size_t)len;

	return buflen - remaining;
}

static _Bool
nsm_append_monitored_host(const char *path, const char *line)
{
	_Bool result = false;
	char *buf = NULL;
	struct stat stb;
	size_t buflen;
	ssize_t len;
	int fd;

	if (stat(path, &stb) == -1) {
		xlog(L_ERROR, "Failed to insert: "
			"could not stat original file %s: %m", path);
		goto out;
	}
	buflen = (size_t)stb.st_size + strlen(line);

	buf = malloc(buflen + 1);
	if (buf == NULL) {
		xlog(L_ERROR, "Failed to insert: no memory");
		goto out;
	}
	memset(buf, 0, buflen + 1);

	fd = open(path, O_RDONLY);
	if (fd == -1) {
		xlog(L_ERROR, "Failed to insert: "
			"could not open original file %s: %m", path);
		goto out;
	}

	len = read(fd, buf, (size_t)stb.st_size);
	if (exact_error_check(len, (size_t)stb.st_size)) {
		xlog(L_ERROR, "Failed to insert: "
			"could not read original file %s: %m", path);
		(void)close(fd);
		goto out;
	}
	(void)close(fd);

	strcat(buf, line);

	if (nsm_atomic_write(path, buf, buflen))
		result = true;

out:
	free(buf);
	return result;
}

/**
 * nsm_insert_monitored_host - write callback data for one host to disk
 * @hostname: C string containing a hostname
 * @sap: sockaddr containing NLM callback address
 * @mon: SM_MON arguments to save
 *
 * Returns true if successful, otherwise false if some error occurs.
 */
_Bool
nsm_insert_monitored_host(const char *hostname, const struct sockaddr *sap,
		const struct mon *m)
{
	static char buf[LINELEN + 1 + SM_MAXSTRLEN + 2];
	char *path;
	_Bool result = false;
	ssize_t len;
	size_t size;
	int fd;

	path = nsm_make_record_pathname(NSM_MONITOR_DIR, hostname);
	if (path == NULL) {
		xlog(L_ERROR, "Failed to insert: bad monitor hostname '%s'",
				hostname);
		return false;
	}

	size = nsm_create_monitor_record(buf, sizeof(buf), sap, m);
	if (size == 0) {
		xlog(L_ERROR, "Failed to insert: record too long");
		goto out;
	}

	/*
	 * If exclusive create fails, we're adding a new line to an
	 * existing file.
	 */
	fd = open(path, O_WRONLY | O_CREAT | O_EXCL | O_SYNC, S_IRUSR | S_IWUSR);
	if (fd == -1) {
		if (errno != EEXIST) {
			xlog(L_ERROR, "Failed to insert: creating %s: %m", path);
			goto out;
		}

		result = nsm_append_monitored_host(path, buf);
		goto out;
	}
	result = true;

	len = write(fd, buf, size);
	if (exact_error_check(len, size)) {
		xlog_warn("Failed to insert: writing %s: %m", path);
		(void)unlink(path);
		result = false;
	}

	if (close(fd) == -1) {
		xlog(L_ERROR, "Failed to insert: closing %s: %m", path);
		(void)unlink(path);
		result = false;
	}

out:
	free(path);
	return result;
}

__attribute__((__noinline__))
static _Bool
nsm_parse_line(char *line, struct sockaddr_in *sin, struct mon *m)
{
	unsigned int i, tmp;
	int count;
	char *c;

	c = strchr(line, '\n');
	if (c != NULL)
		*c = '\0';

	count = sscanf(line, "%8x %8x %8x %8x ",
			(unsigned int *)&sin->sin_addr.s_addr,
			(unsigned int *)&m->mon_id.my_id.my_prog,
			(unsigned int *)&m->mon_id.my_id.my_vers,
			(unsigned int *)&m->mon_id.my_id.my_proc);
	if (count != 4)
		return false;

	c = line + RPCARGSLEN;
	for (i = 0; i < SM_PRIV_SIZE; i++) {
		if (sscanf(c, "%2x", &tmp) != 1)
			return false;
		m->priv[i] = (char)tmp;
		c += 2;
	}

	c++;
	m->mon_id.mon_name = c;
	while (*c != '\0' && *c != ' ')
		c++;
	if (*c != '\0')
		*c++ = '\0';
	while (*c == ' ')
		c++;
	m->mon_id.my_id.my_name = c;

	return true;
}

/*
 * Stuff a 'struct mon' with callback data, and call @func.
 *
 * Returns the count of in-core records created.
 */
static unsigned int
nsm_read_line(const char *hostname, const time_t timestamp, char *line,
		nsm_populate_t func)
{
	struct sockaddr_in sin = {
		.sin_family		= AF_INET,
	};
	struct mon m;

	if (!nsm_parse_line(line, &sin, &m))
		return 0;

	return func(hostname, (struct sockaddr *)(char *)&sin, &m, timestamp);
}

/*
 * Given a filename, reads data from a file under "directory"
 * and invokes @func so caller can populate their in-core
 * database with this data.
 */
static unsigned int
nsm_load_host(const char *directory, const char *filename, nsm_populate_t func)
{
	char buf[LINELEN + 1 + SM_MAXSTRLEN + 2];
	unsigned int result = 0;
	struct stat stb;
	char *path;
	FILE *f;

	path = nsm_make_record_pathname(directory, filename);
	if (path == NULL)
		goto out_err;

	if (lstat(path, &stb) == -1) {
		xlog(L_ERROR, "Failed to stat %s: %m", path);
		goto out_freepath;
	}
	if (!S_ISREG(stb.st_mode)) {
		xlog(D_GENERAL, "Skipping non-regular file %s",
				path);
		goto out_freepath;
	}

	f = fopen(path, "r");
	if (f == NULL) {
		xlog(L_ERROR, "Failed to open %s: %m", path);
		goto out_freepath;
	}

	while (fgets(buf, (int)sizeof(buf), f) != NULL) {
		buf[sizeof(buf) - 1] = '\0';
		result += nsm_read_line(filename, stb.st_mtime, buf, func);
	}
	if (result == 0)
		xlog(L_ERROR, "Failed to read monitor data from %s", path);

	(void)fclose(f);

out_freepath:
	free(path);
out_err:
	return result;
}

static unsigned int
nsm_load_dir(const char *directory, nsm_populate_t func)
{
	unsigned int count = 0;
	struct dirent *de;
	char *path;
	DIR *dir;

	path = nsm_make_pathname(directory);
	if (path == NULL) {
		xlog(L_ERROR, "Failed to allocate path for directory %s",
				directory);
		return 0;
	}

	dir = opendir(path);
	free(path);
	if (dir == NULL) {
		xlog(L_ERROR, "Failed to open directory %s: %m",
				directory);
		return 0;
	}

	while ((de = readdir(dir)) != NULL) {
		if (de->d_name[0] == '.')
			continue;

		count += nsm_load_host(directory, de->d_name, func);
	}

	(void)closedir(dir);
	return count;
}

/**
 * nsm_load_monitor_list - load list of hosts to monitor
 * @func: callback function to create entry for one host
 *
 * Returns the count of hosts that were found in the directory.
 */
unsigned int
nsm_load_monitor_list(nsm_populate_t func)
{
	return nsm_load_dir(NSM_MONITOR_DIR, func);
}

/**
 * nsm_load_notify_list - load list of hosts to notify
 * @func: callback function to create entry for one host
 *
 * Returns the count of hosts that were found in the directory.
 */
unsigned int
nsm_load_notify_list(nsm_populate_t func)
{
	return nsm_load_dir(NSM_NOTIFY_DIR, func);
}

static void
nsm_delete_host(const char *directory, const char *hostname,
		const char *mon_name, const char *my_name, const int chatty)
{
	char line[LINELEN + 1 + SM_MAXSTRLEN + 2];
	char *outbuf = NULL;
	struct stat stb;
	char *path, *next;
	size_t remaining;
	FILE *f;

	path = nsm_make_record_pathname(directory, hostname);
	if (path == NULL) {
		xlog(L_ERROR, "Bad filename, not deleting");
		return;
	}

	if (stat(path, &stb) == -1) {
		if (chatty)
			xlog(L_ERROR, "Failed to delete: "
				"could not stat original file %s: %m", path);
		goto out;
	}
	remaining = (size_t)stb.st_size + 1;

	outbuf = malloc(remaining);
	if (outbuf == NULL) {
		xlog(L_ERROR, "Failed to delete: no memory");
		goto out;
	}

	f = fopen(path, "r");
	if (f == NULL) {
		xlog(L_ERROR, "Failed to delete: "
			"could not open original file %s: %m", path);
		goto out;
	}

	/*
	 * Walk the records in the file, and copy the non-matching
	 * ones to our output buffer.
	 */
	next = outbuf;
	while (fgets(line, (int)sizeof(line), f) != NULL) {
		struct sockaddr_in sin;
		struct mon m;
		size_t len;

		if (!nsm_parse_line(line, &sin, &m)) {
			xlog(L_ERROR, "Failed to delete: "
				"could not parse original file %s", path);
			(void)fclose(f);
			goto out;
		}

		if (strcmp(mon_name, m.mon_id.mon_name) == 0 &&
			 strcmp(my_name, m.mon_id.my_id.my_name) == 0)
			continue;

		/* nsm_parse_line destroys the contents of line[], so
		 * reconstruct the copy in our output buffer. */
		len = nsm_create_monitor_record(next, remaining,
					(struct sockaddr *)(char *)&sin, &m);
		if (len == 0) {
			xlog(L_ERROR, "Failed to delete: "
				"could not construct output record");
			(void)fclose(f);
			goto out;
		}
		next += len;
		remaining -= len;
	}

	(void)fclose(f);

	/*
	 * If nothing was copied when we're done, then unlink the file.
	 * Otherwise, atomically update the contents of the file.
	 */
	if (next != outbuf) {
		if (!nsm_atomic_write(path, outbuf, strlen(outbuf)))
			xlog(L_ERROR, "Failed to delete: "
				"could not write new file %s: %m", path);
	} else {
		if (unlink(path) == -1)
			xlog(L_ERROR, "Failed to delete: "
				"could not unlink file %s: %m", path);
	}

out:
	free(outbuf);
	free(path);
}

/**
 * nsm_delete_monitored_host - delete on-disk record for monitored host
 * @hostname: '\0'-terminated C string containing hostname of record to delete
 * @mon_name: '\0'-terminated C string containing monname of record to delete
 * @my_name: '\0'-terminated C string containing myname of record to delete
 * @chatty: should an error be logged if the monitor file doesn't exist?
 *
 */
void
nsm_delete_monitored_host(const char *hostname, const char *mon_name,
		const char *my_name, const int chatty)
{
	nsm_delete_host(NSM_MONITOR_DIR, hostname, mon_name, my_name, chatty);
}

/**
 * nsm_delete_notified_host - delete on-disk host record after notification
 * @hostname: '\0'-terminated C string containing hostname of record to delete
 * @mon_name: '\0'-terminated C string containing monname of record to delete
 * @my_name: '\0'-terminated C string containing myname of record to delete
 *
 */
void
nsm_delete_notified_host(const char *hostname, const char *mon_name,
		const char *my_name)
{
	nsm_delete_host(NSM_NOTIFY_DIR, hostname, mon_name, my_name, 1);
}
