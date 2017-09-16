/*
 * mount.c -- Linux NFS mount
 *
 * Copyright (C) 2006 Amit Gud <agud@redhat.com>
 *
 * - Basic code and wrapper around mount and umount code of NFS.
 *   Based on util-linux/mount/mount.c.
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
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mount.h>
#include <getopt.h>
#include <mntent.h>
#include <pwd.h>
#include <libgen.h>

#include "fstab.h"
#include "xcommon.h"
#include "nls.h"
#include "mount_constants.h"
#include "mount_config.h"
#include "nfs_paths.h"
#include "nfs_mntent.h"

#include "nfs_mount.h"
#include "nfs4_mount.h"
#include "mount.h"
#include "error.h"
#include "stropts.h"
#include "utils.h"

char *progname;
int nfs_mount_data_version;
int nomtab;
int verbose;
int sloppy;
int string;

#define FOREGROUND	(0)
#define BACKGROUND	(1)

static struct option longopts[] = {
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
  { NULL, 0, 0, 0 }
};

/*
 * Map from -o and fstab option strings to the flag argument to mount(2).
 */
struct opt_map {
	const char *opt;	/* option name */
	int skip;		/* skip in mtab option string */
	int inv;		/* true if flag value should be inverted */
	int mask;		/* flag mask value */
};

static const struct opt_map opt_map[] = {
  { "defaults", 0, 0, 0         },      /* default options */
  { "ro",       1, 0, MS_RDONLY },      /* read-only */
  { "rw",       1, 1, MS_RDONLY },      /* read-write */
  { "exec",     0, 1, MS_NOEXEC },      /* permit execution of binaries */
  { "noexec",   0, 0, MS_NOEXEC },      /* don't execute binaries */
  { "suid",     0, 1, MS_NOSUID },      /* honor suid executables */
  { "nosuid",   0, 0, MS_NOSUID },      /* don't honor suid executables */
  { "dev",      0, 1, MS_NODEV  },      /* interpret device files  */
  { "nodev",    0, 0, MS_NODEV  },      /* don't interpret devices */
  { "sync",     0, 0, MS_SYNCHRONOUS},  /* synchronous I/O */
  { "async",    0, 1, MS_SYNCHRONOUS},  /* asynchronous I/O */
  { "dirsync",  0, 0, MS_DIRSYNC},      /* synchronous directory modifications */
  { "remount",  0, 0, MS_REMOUNT},      /* Alter flags of mounted FS */
  { "bind",     0, 0, MS_BIND   },      /* Remount part of tree elsewhere */
  { "rbind",    0, 0, MS_BIND|MS_REC }, /* Idem, plus mounted subtrees */
  { "auto",     0, 0, MS_DUMMY },       /* Can be mounted using -a */
  { "noauto",   0, 0, MS_DUMMY },       /* Can  only be mounted explicitly */
  { "users",    1, 0, MS_USERS },	/* Allow ordinary user to mount */
  { "nousers",  0, 1, MS_DUMMY  },      /* Forbid ordinary user to mount */
  { "user",     1, 0, MS_USER  },	/* Allow ordinary user to mount */
  { "nouser",   0, 1, MS_DUMMY   },     /* Forbid ordinary user to mount */
  { "owner",    0, 0, MS_DUMMY  },      /* Let the owner of the device mount */
  { "noowner",  0, 0, MS_DUMMY  },      /* Device owner has no special privs */
  { "group",    0, 0, MS_DUMMY  },      /* Let the group of the device mount */
  { "nogroup",  0, 0, MS_DUMMY  },      /* Device group has no special privs */
  { "_netdev",  0, 0, MS_DUMMY},        /* Device requires network */
  { "comment",  0, 0, MS_DUMMY},        /* fstab comment only (kudzu,_netdev)*/

  /* add new options here */
#ifdef MS_NOSUB
  { "sub",      0, 1, MS_NOSUB  },      /* allow submounts */
  { "nosub",    0, 0, MS_NOSUB  },      /* don't allow submounts */
#endif
#ifdef MS_SILENT
  { "quiet",    0, 0, MS_SILENT    },   /* be quiet  */
  { "loud",     0, 1, MS_SILENT    },   /* print out messages. */
#endif
#ifdef MS_MANDLOCK
  { "mand",     0, 0, MS_MANDLOCK },    /* Allow mandatory locks on this FS */
  { "nomand",   0, 1, MS_MANDLOCK },    /* Forbid mandatory locks on this FS */
#endif
  { "loop",     1, 0, MS_DUMMY   },      /* use a loop device */
#ifdef MS_NOATIME
  { "atime",    0, 1, MS_NOATIME },     /* Update access time */
  { "noatime",  0, 0, MS_NOATIME },     /* Do not update access time */
#endif
#ifdef MS_NODIRATIME
  { "diratime", 0, 1, MS_NODIRATIME },  /* Update dir access times */
  { "nodiratime", 0, 0, MS_NODIRATIME },/* Do not update dir access times */
#endif
#ifdef MS_RELATIME
  { "relatime", 0, 0, MS_RELATIME },   /* Update access times relative to
                      mtime/ctime */
  { "norelatime", 0, 1, MS_RELATIME }, /* Update access time without regard
                      to mtime/ctime */
#endif
  { "noquota", 0, 0, MS_DUMMY },        /* Don't enforce quota */
  { "quota", 0, 0, MS_DUMMY },          /* Enforce user quota */
  { "usrquota", 0, 0, MS_DUMMY },       /* Enforce user quota */
  { "grpquota", 0, 0, MS_DUMMY },       /* Enforce group quota */
  { NULL,	0, 0, 0		}
};

static void parse_opts(const char *options, int *flags, char **extra_opts);

/*
 * Build a canonical mount option string for /etc/mtab.
 */
static char *fix_opts_string(int flags, const char *extra_opts)
{
	const struct opt_map *om;
	char *new_opts;

	new_opts = xstrdup((flags & MS_RDONLY) ? "ro" : "rw");
	if (flags & MS_USER) {
		/* record who mounted this so they can unmount */
		struct passwd *pw = getpwuid(getuid());
		if(pw)
			new_opts = xstrconcat3(new_opts, ",user=", pw->pw_name);
	}
	if (flags & MS_USERS)
		new_opts = xstrconcat3(new_opts, ",users", "");

	for (om = opt_map; om->opt != NULL; om++) {
		if (om->skip)
			continue;
		if (om->inv || !om->mask || (flags & om->mask) != om->mask)
			continue;
		new_opts = xstrconcat3(new_opts, ",", om->opt);
		flags &= ~om->mask;
	}
	if (extra_opts && *extra_opts) {
		new_opts = xstrconcat3(new_opts, ",", extra_opts);
	}
	return new_opts;
}

static void
init_mntent(struct mntent *mnt, char *fsname, char *dir, char *type,
		int flags, char *opts)
{
	mnt->mnt_fsname	= fsname;
	mnt->mnt_dir	= dir;
	mnt->mnt_type	= type;
	mnt->mnt_opts	= fix_opts_string(flags & ~MS_NOMTAB, opts);

	/* these are always zero for NFS */
	mnt->mnt_freq	= 0;
	mnt->mnt_passno	= 0;
}

/* Create mtab with a root entry.  */
static void
create_mtab (void) {
	struct mntentchn *fstab;
	struct mntent mnt;
	int flags;
	mntFILE *mfp;

	lock_mtab();

	mfp = nfs_setmntent (MOUNTED, "a+");
	if (mfp == NULL || mfp->mntent_fp == NULL) {
		int errsv = errno;
		die (EX_FILEIO, _("mount: can't open %s for writing: %s"),
		     MOUNTED, strerror (errsv));
	}

	/* Find the root entry by looking it up in fstab */
	if ((fstab = getfsfile ("/")) || (fstab = getfsfile ("root"))) {
		char *extra_opts;
		parse_opts (fstab->m.mnt_opts, &flags, &extra_opts);
		init_mntent(&mnt, xstrdup(fstab->m.mnt_fsname), "/",
				fstab->m.mnt_type, flags, extra_opts);
		free(extra_opts);

		if (nfs_addmntent (mfp, &mnt) == 1) {
			int errsv = errno;
			die (EX_FILEIO, _("mount: error writing %s: %s"),
			     _PATH_MOUNTED, strerror (errsv));
		}
	}
	if (fchmod (fileno (mfp->mntent_fp), 0644) < 0)
		if (errno != EROFS) {
			int errsv = errno;
			die (EX_FILEIO,
			     _("mount: error changing mode of %s: %s"),
			     _PATH_MOUNTED, strerror (errsv));
		}
	nfs_endmntent (mfp);

	unlock_mtab();

	reset_mtab_info();
}

static int add_mtab(char *spec, char *mount_point, char *fstype,
			int flags, char *opts)
{
	struct mntent ment;
	int result = EX_SUCCESS;

	init_mntent(&ment, spec, mount_point, fstype, flags, opts);

	if (!nomtab && mtab_does_not_exist()) {
		if (verbose > 1)
			printf(_("mount: no %s found - creating it..\n"),
			       MOUNTED);
		create_mtab ();
	}

	if (!nomtab && mtab_is_writable()) {
		if (flags & MS_REMOUNT)
			update_mtab(ment.mnt_dir, &ment);
		else {
			mntFILE *mtab;
			
			lock_mtab();
			mtab = nfs_setmntent(MOUNTED, "a+");
			if (mtab == NULL || mtab->mntent_fp == NULL) {
				nfs_error(_("Can't open mtab: %s"),
						strerror(errno));
				result = EX_FILEIO;
			} else {
				if (nfs_addmntent(mtab, &ment) == 1) {
					nfs_error(_("Can't write mount entry to mtab: %s"),
							strerror(errno));
					result = EX_FILEIO;
				}
			}
			nfs_endmntent(mtab);
			unlock_mtab();
		}
	}

	free(ment.mnt_opts);

	return result;
}

static void parse_opt(const char *opt, int *mask, char *extra_opts, size_t len)
{
	const struct opt_map *om;

	for (om = opt_map; om->opt != NULL; om++) {
		if (!strcmp (opt, om->opt)) {
			if (om->inv)
				*mask &= ~om->mask;
			else
				*mask |= om->mask;
			return;
		}
	}

	len -= strlen(extra_opts);

	if (*extra_opts && --len > 0)
		strcat(extra_opts, ",");

	if ((len -= strlen(opt)) > 0)
		strcat(extra_opts, opt);
}

/*
 * Convert the provided mount command-line options into the 4th &
 * 5th arguments to mount(2).  Output parameter "@flags" gets the
 * standard options (indicated by MS_ bits), and output parameter
 * "@extra_opts" gets all the filesystem-specific options.
 */
static void parse_opts(const char *options, int *flags, char **extra_opts)
{
	if (options != NULL) {
		char *opts = xstrdup(options);
		char *opt, *p;
		size_t len = strlen(opts) + 1;	/* include room for a null */
		int open_quote = 0;

		*extra_opts = xmalloc(len);
		**extra_opts = '\0';

		for (p = opts, opt = NULL; p && *p; p++) {
			if (!opt)
				opt = p;	/* begin of the option item */
			if (*p == '"')
				open_quote ^= 1; /* reverse the status */
			if (open_quote)
				continue;	/* still in a quoted block */
			if (*p == ',')
				*p = '\0';	/* terminate the option item */

			/* end of option item or last item */
			if (*p == '\0' || *(p + 1) == '\0') {
				parse_opt(opt, flags, *extra_opts, len);
				opt = NULL;
			}
		}
		free(opts);
	}
}

static int try_mount(char *spec, char *mount_point, int flags,
			char *fs_type, char **extra_opts, char *mount_opts,
			int fake, int bg)
{
	int ret;

	if (string)
		ret = nfsmount_string(spec, mount_point, fs_type, flags,
					extra_opts, fake, bg);
	else {
		if (strcmp(fs_type, "nfs4") == 0)
			ret = nfs4mount(spec, mount_point, flags,
					extra_opts, fake, bg);
		else
			ret = nfsmount(spec, mount_point, flags,
					extra_opts, fake, bg);
	}

	if (ret)
		return ret;

	if (!fake)
		print_one(spec, mount_point, fs_type, mount_opts);

	return add_mtab(spec, mount_point, fs_type, flags, *extra_opts);
}

int main(int argc, char *argv[])
{
	int c, flags = 0, mnt_err = 1, fake = 0;
	char *spec = NULL, *mount_point = NULL, *fs_type = "nfs";
	char *extra_opts = NULL, *mount_opts = NULL;
	uid_t uid = getuid();

	progname = basename(argv[0]);

	nfs_mount_data_version = discover_nfs_mount_data_version(&string);

	if(!strncmp(progname, "umount", strlen("umount")))
		exit(nfsumount(argc, argv));

	if ((argc < 3)) {
		mount_usage();
		exit(EX_USAGE);
	}

	mount_config_init(progname);

	while ((c = getopt_long(argc, argv, "rvVwfno:hs",
				longopts, NULL)) != -1) {
		switch (c) {
		case 'r':
			flags |= MS_RDONLY;
			break;
		case 'v':
			++verbose;
			break;
		case 'V':
			printf("%s: ("PACKAGE_STRING")\n", progname);
			exit(EX_SUCCESS);
		case 'w':
			flags &= ~MS_RDONLY;
			break;
		case 'f':
			++fake;
			break;
		case 'n':
			++nomtab;
			break;
		case 'o':              /* specify mount options */
			if (mount_opts)
				mount_opts = xstrconcat3(mount_opts, ",", optarg);
			else
				mount_opts = xstrdup(optarg);
			break;
		case 's':
			++sloppy;
			break;
		case 'h':
		default:
			mount_usage();
			goto out_usage;
		}
	}

	/*
	 * Extra non-option words at the end are bogus...
	 */
	if (optind != argc - 2) {
		mount_usage();
		goto out_usage;
	} else {
		while (optind < argc) {
			if (!spec)
				spec = argv[optind];
			else
				mount_point = argv[optind];
			optind++;
		}
	}

	if (strcmp(progname, "mount.nfs4") == 0)
		fs_type = "nfs4";

	/*
	 * If a non-root user is attempting to mount, make sure the
	 * user's requested options match the options specified in
	 * /etc/fstab; otherwise, don't allow the mount.
	 */
	if (uid != 0) {
		struct mntentchn *mc;

		if ((mc = getfsfile(mount_point)) == NULL ||
		    strcmp(mc->m.mnt_fsname, spec) != 0 ||
		    strcmp(mc->m.mnt_type, fs_type) != 0) {
			nfs_error(_("%s: permission denied: no match for %s "
				"found in /etc/fstab"), progname, mount_point);
			goto out_usage;
		}

		/*
		 * 'mount' munges the options from fstab before passing them
		 * to us, so it is non-trivial to test that we have the correct
		 * set of options and we don't want to trust what the user
		 * gave us, so just take whatever is in /etc/fstab.
		 */
		mount_opts = strdup(mc->m.mnt_opts);
	}

	mount_point = canonicalize(mount_point);
	if (!mount_point) {
		nfs_error(_("%s: no mount point provided"), progname);
		goto out_usage;
	}
	if (mount_point[0] != '/') {
		nfs_error(_("%s: unrecognized mount point %s"),
			progname, mount_point);
		mnt_err = EX_USAGE;
		goto out;
	}
	/*
	 * Concatenate mount options from the configuration file
	 */
	mount_opts = mount_config_opts(spec, mount_point, mount_opts);

	parse_opts(mount_opts, &flags, &extra_opts);

	if (uid != 0) {
		if (!(flags & (MS_USERS|MS_USER))) {
			nfs_error(_("%s: permission denied"), progname);
			mnt_err = EX_USAGE;
			goto out;
		}

		if (geteuid() != 0) {
			nfs_error(_("%s: not installed setuid - "
				    "\"user\" NFS mounts not supported."), progname);
			exit(EX_FAIL);
		}
	}

	if (chk_mountpoint(mount_point)) {
		mnt_err = EX_USAGE;
		goto out;
	}

	mnt_err = try_mount(spec, mount_point, flags, fs_type, &extra_opts,
				mount_opts, fake, FOREGROUND);
	if (mnt_err == EX_BG) {
		printf(_("%s: backgrounding \"%s\"\n"),
			progname, spec);
		printf(_("%s: mount options: \"%s\"\n"),
			progname, extra_opts);

		fflush(stdout);

		/*
		 * Parent exits immediately with success.
		 */
		if (daemon(0, 0)) {
			nfs_error(_("%s: failed to start "
					"background process: %s\n"),
						progname, strerror(errno));
			exit(EX_FAIL);
		}

		mnt_err = try_mount(spec, mount_point, flags, fs_type,
					&extra_opts, mount_opts, fake,
					BACKGROUND);
		if (verbose && mnt_err)
			printf(_("%s: giving up \"%s\"\n"),
				progname, spec);
	}

out:
	free(mount_opts);
	free(extra_opts);
	free(mount_point);
	exit(mnt_err);

out_usage:
	free(mount_opts);
	exit(EX_USAGE);
}
