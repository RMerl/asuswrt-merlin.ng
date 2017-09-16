/*
 * nfsdcltrack.c -- NFSv4 client name tracking program
 *
 * Copyright (C) 2012 Jeff Layton <jlayton@redhat.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <getopt.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <libgen.h>
#include <sys/inotify.h>
#include <dirent.h>
#include <limits.h>
#ifdef HAVE_SYS_CAPABILITY_H
#include <sys/prctl.h>
#include <sys/capability.h>
#endif

#include "xlog.h"
#include "sqlite.h"

#ifndef CLD_DEFAULT_STORAGEDIR
#define CLD_DEFAULT_STORAGEDIR NFS_STATEDIR "/nfsdcltrack"
#endif

#define NFSD_END_GRACE_FILE "/proc/fs/nfsd/v4_end_grace"

/* defined by RFC 3530 */
#define NFS4_OPAQUE_LIMIT	1024

/* private data structures */
struct cltrack_cmd {
	char *name;
	bool needs_arg;
	int (*func)(const char *arg);
};

/* forward declarations */
static int cltrack_init(const char *unused);
static int cltrack_create(const char *id);
static int cltrack_remove(const char *id);
static int cltrack_check(const char *id);
static int cltrack_gracedone(const char *gracetime);

/* global variables */
static struct option longopts[] =
{
	{ "help", 0, NULL, 'h' },
	{ "debug", 0, NULL, 'd' },
	{ "foreground", 0, NULL, 'f' },
	{ "storagedir", 1, NULL, 's' },
	{ NULL, 0, 0, 0 },
};

static struct cltrack_cmd commands[] =
{
	{ "init", false, cltrack_init },
	{ "create", true, cltrack_create },
	{ "remove", true, cltrack_remove },
	{ "check", true, cltrack_check },
	{ "gracedone", true, cltrack_gracedone },
	{ NULL, false, NULL },
};

static char *storagedir = CLD_DEFAULT_STORAGEDIR;

/* common buffer for holding id4 blobs */
static unsigned char blob[NFS4_OPAQUE_LIMIT];

static void
usage(char *progname)
{
	printf("%s [ -hfd ] [ -s dir ] < cmd > < arg >\n", progname);
	printf("Where < cmd > is one of the following and takes the following < arg >:\n");
	printf("    init\n");
	printf("    create <nfs_client_id4>\n");
	printf("    remove <nfs_client_id4>\n");
	printf("    check  <nfs_client_id4>\n");
	printf("    gracedone <epoch time>\n");
}


/**
 * hex_to_bin - convert a hex digit to its real value
 * @ch: ascii character represents hex digit
 *
 * hex_to_bin() converts one hex digit to its actual value or -1 in case of bad
 * input.
 *
 * Note: borrowed from lib/hexdump.c in the Linux kernel sources.
 */
static int
hex_to_bin(char ch)
{
	if ((ch >= '0') && (ch <= '9'))
		return ch - '0';
	ch = tolower(ch);
	if ((ch >= 'a') && (ch <= 'f'))
		return ch - 'a' + 10;
	return -1;
}

/**
 * hex_str_to_bin - convert a hexidecimal string into a binary blob
 *
 * @src: string of hex digit pairs
 * @dst: destination buffer to hold binary data
 * @dstsize: size of the destination buffer
 *
 * Walk a string of hex digit pairs and convert them into binary data. Returns
 * the resulting length of the binary data or a negative error code. If the
 * data will not fit in the buffer, it returns -ENOBUFS (but will likely have
 * clobbered the dst buffer in the process of determining that). If there are
 * non-hexidecimal characters in the src, or an odd number of them then it
 * returns -EINVAL.
 */
static ssize_t
hex_str_to_bin(const char *src, unsigned char *dst, ssize_t dstsize)
{
	unsigned char *tmpdst = dst;

	while (*src) {
		int hi, lo;

		/* make sure we don't overrun the dst buffer */
		if ((tmpdst - dst) >= dstsize)
			return -ENOBUFS;

		hi = hex_to_bin(*src++);

		/* did we get an odd number of characters? */
		if (!*src)
			return -EINVAL;
		lo = hex_to_bin(*src++);

		/* one of the characters isn't a hex digit */
		if (hi < 0 || lo < 0)
			return -EINVAL;

		/* now place it in the dst buffer */
		*tmpdst++ = (hi << 4) | lo;
	}

	return (ssize_t)(tmpdst - dst);
}

/*
 * This program will almost always be run with root privileges since the
 * kernel will call out to run it. Drop all capabilities prior to doing
 * anything important to limit the exposure to potential compromise.
 *
 * FIXME: should we setuid to a different user early on instead?
 */
static int
cltrack_set_caps(void)
{
	int ret = 0;
#ifdef HAVE_SYS_CAPABILITY_H
	unsigned long i;
	cap_t caps;

	/* prune the bounding set to nothing */
	for (i = 0; prctl(PR_CAPBSET_READ, i, 0, 0, 0) >= 0 ; ++i) {
		ret = prctl(PR_CAPBSET_DROP, i, 0, 0, 0);
		if (ret) {
			xlog(L_ERROR, "Unable to prune capability %lu from "
				      "bounding set: %m", i);
			return -errno;
		}
	}

	/* get a blank capset */
	caps = cap_init();
	if (caps == NULL) {
		xlog(L_ERROR, "Unable to get blank capability set: %m");
		return -errno;
	}

	/* reset the process capabilities */
	if (cap_set_proc(caps) != 0) {
		xlog(L_ERROR, "Unable to set process capabilities: %m");
		ret = -errno;
	}
	cap_free(caps);
#endif
	return ret;
}

/* Inform the kernel that it's OK to lift nfsd's grace period */
static void
cltrack_lift_grace_period(void)
{
	int fd;

	fd = open(NFSD_END_GRACE_FILE, O_WRONLY);
	if (fd < 0) {
		/* Don't warn if file isn't present */
		if (errno != ENOENT)
			xlog(L_WARNING, "Unable to open %s: %m",
				NFSD_END_GRACE_FILE);
		return;
	}

	if (write(fd, "Y", 1) < 0)
		xlog(L_WARNING, "Unable to write to %s: %m",
				NFSD_END_GRACE_FILE);

	close(fd);
	return;
}

/*
 * Fetch the contents of the NFSDCLTRACK_GRACE_START env var. If it's not set
 * or there's an error converting it to time_t, then return LONG_MAX.
 */
static time_t
cltrack_get_grace_start(void)
{
	time_t grace_start;
	char *end;
	char *grace_start_str = getenv("NFSDCLTRACK_GRACE_START");

	if (!grace_start_str)
		return LONG_MAX;

	errno = 0;
	grace_start = strtol(grace_start_str, &end, 0);
	/* Problem converting or value is too large? */
	if (errno)
		return LONG_MAX;

	return grace_start;
}

static bool
cltrack_reclaims_complete(void)
{
	time_t grace_start = cltrack_get_grace_start();

	/* Don't query DB if we didn't get a valid boot time */
	if (grace_start == LONG_MAX)
		return false;

	return !sqlite_query_reclaiming(grace_start);
}

static int
cltrack_init(const char __attribute__((unused)) *unused)
{
	int ret;

	/*
	 * see if the storagedir is writable by root w/o CAP_DAC_OVERRIDE.
	 * If it isn't then give the user a warning but proceed as if
	 * everything is OK. If the DB has already been created, then
	 * everything might still work. If it doesn't exist at all, then
	 * assume that the maindb init will be able to create it. Fail on
	 * anything else.
	 */
	if (access(storagedir, W_OK) == -1) {
		switch (errno) {
		case EACCES:
			xlog(L_WARNING, "Storage directory %s is not writable. "
					"Should be owned by root and writable "
					"by owner!", storagedir);
			break;
		case ENOENT:
			/* ignore and assume that we can create dir as root */
			break;
		default:
			xlog(L_ERROR, "Unexpected error when checking access "
				      "on %s: %m", storagedir);
			return -errno;
		}
	}

	/* set up storage db */
	ret = sqlite_prepare_dbh(storagedir);
	if (ret) {
		xlog(L_ERROR, "Failed to init database: %d", ret);
		/*
		 * Convert any error here into -EACCES. It's not truly
		 * accurate in all cases, but it should cause the kernel to
		 * stop upcalling until the problem is resolved.
		 */
		ret = -EACCES;
	} else {
		if (cltrack_reclaims_complete())
			cltrack_lift_grace_period();
	}

	return ret;
}

/*
 * Fetch the contents of the NFSDCLTRACK_CLIENT_HAS_SESSION env var. If
 * it's set and the first character is 'Y' then return true. Otherwise
 * return false.
 */
static bool
cltrack_client_has_session(void)
{
	char *has_session = getenv("NFSDCLTRACK_CLIENT_HAS_SESSION");

	if (has_session && *has_session == 'Y')
		return true;

	return false;
}

static int
cltrack_create(const char *id)
{
	int ret;
	ssize_t len;
	bool has_session;

	xlog(D_GENERAL, "%s: create client record.", __func__);

	ret = sqlite_prepare_dbh(storagedir);
	if (ret)
		return ret;

	len = hex_str_to_bin(id, blob, sizeof(blob));
	if (len < 0)
		return (int)len;

	has_session = cltrack_client_has_session();

	ret = sqlite_insert_client(blob, len, has_session, false);

	if (!ret && has_session && cltrack_reclaims_complete())
		cltrack_lift_grace_period();

	return ret ? -EREMOTEIO : ret;
}

static int
cltrack_remove(const char *id)
{
	int ret;
	ssize_t len;

	xlog(D_GENERAL, "%s: remove client record.", __func__);

	ret = sqlite_prepare_dbh(storagedir);
	if (ret)
		return ret;

	len = hex_str_to_bin(id, blob, sizeof(blob));
	if (len < 0)
		return (int)len;

	ret = sqlite_remove_client(blob, len);

	return ret ? -EREMOTEIO : ret;
}

static int
cltrack_check_legacy(const unsigned char *blob, const ssize_t len,
			bool has_session)
{
	int ret;
	struct stat st;
	char *recdir = getenv("NFSDCLTRACK_LEGACY_RECDIR");

	if (!recdir) {
		xlog(D_GENERAL, "No NFSDCLTRACK_LEGACY_RECDIR env var");
		return -EOPNOTSUPP;
	}

	/* fail recovery on any stat failure */
	ret = stat(recdir, &st);
	if (ret) {
		xlog(D_GENERAL, "Unable to stat %s: %d", recdir, errno);
		return -errno;
	}

	/* fail if it isn't a directory */
	if (!S_ISDIR(st.st_mode)) {
		xlog(D_GENERAL, "%s is not a directory: mode=0%o", recdir
				, st.st_mode);
		return -ENOTDIR;
	}

	/* Dir exists, try to insert record into db */
	ret = sqlite_insert_client(blob, len, has_session, has_session);
	if (ret) {
		xlog(D_GENERAL, "Failed to insert client: %d", ret);
		return -EREMOTEIO;
	}

	/* remove the legacy recoverydir */
	ret = rmdir(recdir);
	if (ret) {
		xlog(D_GENERAL, "Failed to rmdir %s: %d", recdir, errno);
		return -errno;
	}
	return 0;
}

static int
cltrack_check(const char *id)
{
	int ret;
	ssize_t len;
	bool has_session;

	xlog(D_GENERAL, "%s: check client record", __func__);

	ret = sqlite_prepare_dbh(storagedir);
	if (ret)
		return ret;

	len = hex_str_to_bin(id, blob, sizeof(blob));
	if (len < 0)
		return (int)len;

	has_session = cltrack_client_has_session();

	ret = sqlite_check_client(blob, len, has_session);
	if (ret)
		ret = cltrack_check_legacy(blob, len, has_session);

	return ret ? -EPERM : ret;
}

/* Clean out the v4recoverydir -- best effort here */
static void
cltrack_legacy_gracedone(void)
{
	DIR *v4recovery;
	struct dirent *entry;
	char *dirname = getenv("NFSDCLTRACK_LEGACY_TOPDIR");

	if (!dirname)
		return;

	v4recovery = opendir(dirname);
	if (!v4recovery)
		return;

	while ((entry = readdir(v4recovery))) {
		int len;

		/* skip "." and ".." */
		if (entry->d_name[0] == '.') {
			switch (entry->d_name[1]) {
			case '\0':
				continue;
			case '.':
				if (entry->d_name[2] == '\0')
					continue;
			}
		}

		/* borrow the clientid blob for this */
		len = snprintf((char *)blob, sizeof(blob), "%s/%s", dirname,
				entry->d_name);

		/* if there's a problem, then skip this entry */
		if (len < 0 || (size_t)len >= sizeof(blob)) {
			xlog(L_WARNING, "%s: unable to build filename for %s!",
				__func__, entry->d_name);
			continue;
		}

		len = rmdir((char *)blob);
		if (len)
			xlog(L_WARNING, "%s: unable to rmdir %s: %d", __func__,
				(char *)blob, len);
	}

	closedir(v4recovery);
}

static int
cltrack_gracedone(const char *timestr)
{
	int ret;
	char *tail;
	time_t gracetime;


	ret = sqlite_prepare_dbh(storagedir);
	if (ret)
		return ret;

	errno = 0;
	gracetime = strtol(timestr, &tail, 0);

	/* did the resulting value overflow? (Probably -ERANGE here) */
	if (errno)
		return -errno;

	/* string wasn't fully converted */
	if (*tail)
		return -EINVAL;

	xlog(D_GENERAL, "%s: grace done. gracetime=%ld", __func__, gracetime);

	ret = sqlite_remove_unreclaimed(gracetime);

	cltrack_legacy_gracedone();

	return ret ? -EREMOTEIO : ret;
}

static struct cltrack_cmd *
find_cmd(char *cmdname)
{
	struct cltrack_cmd *current = &commands[0];

	while (current->name) {
		if (!strcmp(cmdname, current->name))
			return current;
		++current;
	}

	xlog(L_ERROR, "%s: '%s' doesn't match any known command",
			__func__, cmdname);
	return NULL;
}

int
main(int argc, char **argv)
{
	char arg;
	int rc = 0;
	char *progname, *cmdarg = NULL;
	struct cltrack_cmd *cmd;

	progname = basename(argv[0]);

	xlog_syslog(1);
	xlog_stderr(0);

	/* process command-line options */
	while ((arg = getopt_long(argc, argv, "hdfs:", longopts,
				  NULL)) != EOF) {
		switch (arg) {
		case 'd':
			xlog_config(D_ALL, 1);
		case 'f':
			xlog_syslog(0);
			xlog_stderr(1);
			break;
		case 's':
			storagedir = optarg;
			break;
		default:
			usage(progname);
			return 0;
		}
	}

	xlog_open(progname);

	/* we expect a command, at least */
	if (optind >= argc) {
		xlog(L_ERROR, "Missing command name\n");
		rc = -EINVAL;
		goto out;
	}

	/* drop all capabilities */
	rc = cltrack_set_caps();
	if (rc)
		goto out;

	cmd = find_cmd(argv[optind]);
	if (!cmd) {
		/*
		 * In the event that we get a command that we don't understand
		 * then return a distinct error. The kernel can use this to
		 * determine a new kernel/old userspace situation and cope
		 * with it.
		 */
		rc = -ENOSYS;
		goto out;
	}

	/* populate arg var if command needs it */
	if (cmd->needs_arg) {
		if (optind + 1 >= argc) {
			xlog(L_ERROR, "Command %s requires an argument\n",
				cmd->name);
			rc = -EINVAL;
			goto out;
		}
		cmdarg = argv[optind + 1];
	}
	rc = cmd->func(cmdarg);
out:
	return rc;
}
