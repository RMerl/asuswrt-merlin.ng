/*
 * mount_libmount.c -- Linux NFS [u]mount based on libmount
 *
 * Copyright (C) 2011 Karel Zak <kzak@redhat.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 0211-1301 USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>
#include <libgen.h>

#include <libmount/libmount.h>

#include "nls.h"
#include "mount_config.h"

#include "nfs_mount.h"
#include "nfs4_mount.h"
#include "stropts.h"
#include "version.h"
#include "xcommon.h"

#include "error.h"
#include "utils.h"

char *progname;
int nfs_mount_data_version;
int verbose;
int sloppy;
int string;
int nomtab;

#define FOREGROUND	(0)
#define BACKGROUND	(1)

/*
 * Store mount options to mtab (or /dev/.mount/utab), called from mount.nfs.
 *
 * Note that on systems without /etc/mtab the fs-specific options are not
 * managed by libmount at all. We have to use "mount attributes" that are
 * private for mount.<type> helpers.
 */
static void store_mount_options(struct libmnt_fs *fs, const char *nfs_opts)
{
	char *o = NULL;

	mnt_fs_set_attributes(fs, nfs_opts);	/* for non-mtab systems */

	/* for mtab create a new options list */
	mnt_optstr_append_option(&o, mnt_fs_get_vfs_options(fs), NULL);
	mnt_optstr_append_option(&o, nfs_opts, NULL);
	mnt_optstr_append_option(&o, mnt_fs_get_user_options(fs), NULL);

	mnt_fs_set_options(fs, o);
	free(o);
}

/*
 * Retrieve mount options from mtab (or /dev/.mount/utab) called from umount.nfs.
 *
 * The result can passed to free().
 */
char *retrieve_mount_options(struct libmnt_fs *fs)
{
	const char *opts;

	if (!fs)
		return NULL;

	opts = mnt_fs_get_attributes(fs);	/* /dev/.mount/utab */
	if (opts)
		return strdup(opts);

	return mnt_fs_strdup_options(fs);	/* /etc/mtab */
}

static int try_mount(struct libmnt_context *cxt, int bg)
{
	struct libmnt_fs *fs;
	const char *p;
	char *src = NULL, *tgt = NULL, *type = NULL, *opts = NULL;
	unsigned long flags = 0;
	int fake, ret = 0;

	fs = mnt_context_get_fs(cxt);

	/* libmount returns read-only pointers (const char)
	 * so, reallocate for nfsmount() functions.
	 */
	if ((p = mnt_fs_get_source(fs)))	/* spec */
		src = strdup(p);
	if ((p = mnt_fs_get_target(fs)))	/* mountpoint */
		tgt = strdup(p);
	if ((p = mnt_fs_get_fstype(fs)))	/* FS type */
		type = strdup(p);
	if ((p = mnt_fs_get_fs_options(fs)))	/* mount options */
		opts = strdup(p);

	mnt_context_get_mflags(cxt, &flags);	/* mount(2) flags */
	fake = mnt_context_is_fake(cxt);

	if (string)
		ret = nfsmount_string(src, tgt, type, flags, &opts, fake, bg);

	else if (strcmp(type, "nfs4") == 0)
		ret = nfs4mount(src, tgt, flags, &opts, fake, bg);
	else
		ret = nfsmount(src, tgt, flags, &opts, fake, bg);

	/* Store mount options if not called with mount --no-mtab */
	if (!ret && !mnt_context_is_nomtab(cxt))
		store_mount_options(fs, opts);

	free(src);
	free(tgt);
	free(type);
	free(opts);

	return ret;
}

/* returns: error = -1, success = 1 , not vers4 == 0 */
static int is_vers4(struct libmnt_context *cxt)
{
	struct libmnt_fs *fs = mnt_context_get_fs(cxt);
	struct libmnt_table *tb = NULL;
	const char *src = mnt_context_get_source(cxt),
		   *tgt = mnt_context_get_target(cxt);
	int rc = 0;

	if (!src || !tgt)
		return -1;

	if (!mnt_fs_is_kernel(fs)) {
		struct libmnt_table *tb = mnt_new_table_from_file("/proc/mounts");

		if (!tb)
			return -1;
		fs = mnt_table_find_pair(tb, src, tgt, MNT_ITER_BACKWARD);
	}

	if (fs) {
		const char *type = mnt_fs_get_fstype(fs);
		if (type && strcmp(type, "nfs4") == 0)
			rc = 1;
	}
	mnt_free_table(tb);
	return rc;
}

static int umount_main(struct libmnt_context *cxt, int argc, char **argv)
{
	int rc, c;
	char *spec = NULL, *opts = NULL;
	int ret = EX_FAIL, verbose = 0;

	static const struct option longopts[] = {
		{ "force", 0, 0, 'f' },
		{ "help", 0, 0, 'h' },
		{ "no-mtab", 0, 0, 'n' },
		{ "verbose", 0, 0, 'v' },
		{ "read-only", 0, 0, 'r' },
		{ "lazy", 0, 0, 'l' },
		{ "types", 1, 0, 't' },
		{ NULL, 0, 0, 0 }
	};

	mnt_context_init_helper(cxt, MNT_ACT_UMOUNT, 0);

	while ((c = getopt_long (argc, argv, "fvnrlh", longopts, NULL)) != -1) {

		rc = mnt_context_helper_setopt(cxt, c, optarg);
		if (rc == 0)		/* valid option */
			continue;
		if (rc < 0)		/* error (probably ENOMEM) */
			goto err;
					/* rc==1 means unknow option */
		umount_usage();
		return EX_USAGE;
	}

	verbose = mnt_context_is_verbose(cxt);

	if (optind < argc)
		spec = argv[optind++];

	if (!spec || (*spec != '/' && strchr(spec,':') == NULL)) {
		nfs_error(_("%s: no mount point provided"), progname);
		umount_usage();
		return EX_USAGE;
	}

	if (mnt_context_set_target(cxt, spec))
		goto err;

	/* read mtab/fstab, evaluate permissions, etc. */
	rc = mnt_context_prepare_umount(cxt);
	if (rc) {
		nfs_error(_("%s: failed to prepare umount: %s\n"),
					progname, strerror(-rc));
		goto err;
	}

	if (mnt_context_get_fstype(cxt) &&
	    !mnt_match_fstype(mnt_context_get_fstype(cxt), "nfs,nfs4")) {

		nfs_error(_("%s: %s: is not an NFS filesystem"), progname, spec);
		ret = EX_USAGE;
		goto err;
	}

	if (verbose)
		printf(_("%s: %s mount point detected\n"), spec,
					mnt_context_get_fstype(cxt));

	opts = retrieve_mount_options(mnt_context_get_fs(cxt));

	if (!mnt_context_is_lazy(cxt)) {
		if (opts) {
			/* we have full FS description (e.g. from mtab or /proc) */
			switch (is_vers4(cxt)) {
			case 0:
				/* We ignore the error from nfs_umount23.
				 * If the actual umount succeeds (in del_mtab),
				 * we don't want to signal an error, as that
				 * could cause /sbin/mount to retry!
				 */
				nfs_umount23(mnt_context_get_source(cxt), opts);
				break;
			case 1:			/* unknown */
				break;
			default:		/* error */
				goto err;
			}
		} else
			/* strange, no entry in mtab or /proc not mounted */
			nfs_umount23(spec, "tcp,v3");
	}

	ret = EX_FILEIO;
	rc = mnt_context_do_umount(cxt);	/* call umount(2) syscall */
	mnt_context_finalize_mount(cxt);	/* mtab update */

	if (rc && !mnt_context_get_status(cxt)) {
		/* mnt_context_do_umount() returns errno if umount(2) failed */
		umount_error(rc, spec);
		goto err;
	}
	ret = EX_SUCCESS;
err:
	if (verbose) {
		if (ret == EX_SUCCESS)
			printf(_("%s: umounted\n"), spec);
		else
			printf(_("%s: umount failed\n"), spec);
	}
	free(opts);
	return ret;
}

static int mount_main(struct libmnt_context *cxt, int argc, char **argv)
{
	int rc, c;
	struct libmnt_fs *fs;
	char *spec = NULL, *mount_point = NULL, *opts = NULL;

	static const struct option longopts[] = {
	  { "fake", 0, 0, 'f' },
	  { "help", 0, 0, 'h' },
	  { "no-mtab", 0, 0, 'n' },
	  { "read-only", 0, 0, 'r' },
	  { "ro", 0, 0, 'r' },
	  { "verbose", 0, 0, 'v' },
	  { "version", 0, 0, 'V' },
	  { "read-write", 0, 0, 'w' },
	  { "rw", 0, 0, 'w' },
	  { "options", 1, 0, 'o' },
	  { "sloppy", 0, 0, 's' },
	  { NULL, 0, 0, 0 }
	};

	mount_config_init(progname);
	mnt_context_init_helper(cxt, MNT_ACT_MOUNT, 0);

	while ((c = getopt_long(argc, argv, "fhnrVvwo:s", longopts, NULL)) != -1) {

		rc = mnt_context_helper_setopt(cxt, c, optarg);
		if (rc == 0)		/* valid option */
			continue;
		if (rc < 0)		/* error (probably ENOMEM) */
			goto err;
					/* rc==1 means unknow option */
		switch (c) {
		case 'V':
			printf("%s: ("PACKAGE_STRING")\n", progname);
			return EX_SUCCESS;
		case 'h':
		default:
			mount_usage();
			return EX_USAGE;
		}
	}

	if (optind < argc)
		spec = argv[optind++];
	if (optind < argc)
		mount_point = argv[optind++];

	if (!mount_point) {
		nfs_error(_("%s: no mount point provided"), progname);
		mount_usage();
		goto err;
	}
	if (!spec) {
		nfs_error(_("%s: no mount spec provided"), progname);
		goto err;
	}

	if (geteuid() != 0) {
		nfs_error(_("%s: not installed setuid - "
			    "\"user\" NFS mounts not supported."), progname);
		goto err;
	}

	verbose = mnt_context_is_verbose(cxt);
	sloppy = mnt_context_is_sloppy(cxt);
	nomtab = mnt_context_is_nomtab(cxt);

	if (strcmp(progname, "mount.nfs4") == 0)
		mnt_context_set_fstype(cxt, "nfs4");
	else
		mnt_context_set_fstype(cxt, "nfs");	/* default */

	rc = mnt_context_set_source(cxt, spec);
	if (!rc)
		mnt_context_set_target(cxt, mount_point);
	if (rc) {
		nfs_error(_("%s: failed to set spec or mountpoint: %s"),
				progname, strerror(errno));
		goto err;
	}

	mount_point = mnt_resolve_path(mount_point,
				       mnt_context_get_cache(cxt));

	if (chk_mountpoint(mount_point))
		goto err;

	/*
	 * The libmount strictly uses only options from fstab if running in
	 * restricted mode (suid, non-root user). This is done in
	 * mnt_context_prepare_mount() by default.
	 *
	 * We have to read fstab before nfsmount.conf, otherwise the options
	 * from nfsmount.conf will be ignored (overwrited).
	 */
	rc = mnt_context_apply_fstab(cxt);
	if (rc) {
		nfs_error(_("%s: failed to apply fstab options\n"), progname);
		goto err;
	}

	/*
	 * Concatenate mount options from the configuration file
	 */
	fs = mnt_context_get_fs(cxt);
	if (fs) {
		opts = mnt_fs_strdup_options(fs);

		opts = mount_config_opts(spec, mount_point, opts);
		mnt_fs_set_options(fs, opts);
	}

	rc = mnt_context_prepare_mount(cxt);
	if (rc) {
		nfs_error(_("%s: failed to prepare mount: %s\n"),
					progname, strerror(-rc));
		goto err;
	}

	rc = try_mount(cxt, FOREGROUND);

	if (rc == EX_BG) {
		printf(_("%s: backgrounding \"%s\"\n"),
			progname, mnt_context_get_source(cxt));
		printf(_("%s: mount options: \"%s\"\n"),
			progname, opts);

		fflush(stdout);

		if (daemon(0, 0)) {
			nfs_error(_("%s: failed to start "
					"background process: %s\n"),
					progname, strerror(errno));
			exit(EX_FAIL);
		}

		rc = try_mount(cxt, BACKGROUND);

		if (verbose && rc)
			printf(_("%s: giving up \"%s\"\n"),
				progname, mnt_context_get_source(cxt));
	}

	mnt_context_set_syscall_status(cxt, rc == EX_SUCCESS ? 0 : -1);
	mnt_context_finalize_mount(cxt);	/* mtab update */
	return rc;
err:
	return EX_FAIL;
}

int main(int argc, char *argv[])
{
	struct libmnt_context *cxt;
	int rc;

	mnt_init_debug(0);
	cxt = mnt_new_context();
	if (!cxt) {
		nfs_error(_("Can't initilize libmount: %s"),
					strerror(errno));
		rc = EX_FAIL;
		goto done;
	}

	progname = basename(argv[0]);
	nfs_mount_data_version = discover_nfs_mount_data_version(&string);

	if(strncmp(progname, "umount", 6) == 0)
		rc = umount_main(cxt, argc, argv);
	else
		rc = mount_main(cxt, argc, argv);
done:
	mnt_free_context(cxt);
	return rc;
}
