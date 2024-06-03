/* vi: set sw=4 ts=4: */
/*
 * Mini rm implementation for busybox
 *
 * Copyright (C) 2001 Matt Kraai <kraai@alumni.carnegiemellon.edu>
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */

/* BB_AUDIT SUSv3 compliant */
/* http://www.opengroup.org/onlinepubs/007904975/utilities/rm.html */

/* Mar 16, 2003      Manuel Novoa III   (mjn3@codepoet.org)
 *
 * Size reduction.
 */

//usage:#define rm_trivial_usage
//usage:       "[-irf] FILE..."
//usage:#define rm_full_usage "\n\n"
//usage:       "Remove (unlink) FILEs\n"
//usage:     "\n	-i	Always prompt before removing"
//usage:     "\n	-f	Never prompt"
//usage:     "\n	-R,-r	Recurse"
//usage:
//usage:#define rm_example_usage
//usage:       "$ rm -rf /tmp/foo\n"

#include "libbb.h"
#include <rtconfig.h>

#if (defined(RTCONFIG_HND_ROUTER_AX_6756) || defined(RTCONFIG_HND_ROUTER_BE_4916)) && !defined(RTCONFIG_BCM_MFG)
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

static int system_file(char *path)
{
	if (!path)
		return 0;

	if (!strcmp(path, "/tmp/mnt/defaults/wl/nvram.nvm") ||
	    !strcmp(path, "/mnt/defaults/wl/nvram.nvm") ||
	    ((f_exists("/tmp/mnt/defaults/wl/nvram.nvm") ||
	      f_exists("/mnt/defaults/wl/nvram.nvm")) &&
	       (!strcmp(path, "/tmp/mnt") ||
		!strcmp(path, "/tmp/mnt/defaults") ||
		!strcmp(path, "/tmp/mnt/defaults/wl") ||
		!strcmp(path, "/mnt") ||
		!strcmp(path, "/mnt/defaults") ||
		!strcmp(path, "/mnt/defaults/wl"))))
		return 1;
	else
		return 0;
}
#endif

int f_exists(const char *path)  // note: anything but a directory
{
        struct stat st;
        return (stat(path, &st) == 0) && (!S_ISDIR(st.st_mode));
}

/* This is a NOFORK applet. Be very careful! */

int rm_main(int argc, char **argv) MAIN_EXTERNALLY_VISIBLE;
int rm_main(int argc UNUSED_PARAM, char **argv)
{
	int status = 0;
	int flags = 0;
	unsigned opt;
	char path[PATH_MAX+1];

	opt_complementary = "f-i:i-f";
	opt = getopt32(argv, "fiRrv");
	argv += optind;
	if (opt & 1)
		flags |= FILEUTILS_FORCE;
	if (opt & 2)
		flags |= FILEUTILS_INTERACTIVE;
	if (opt & (8|4))
		flags |= FILEUTILS_RECUR;
	if ((opt & 16) && FILEUTILS_VERBOSE)
		flags |= FILEUTILS_VERBOSE;

	if (*argv != NULL) {
		do {
			const char *base = bb_get_last_path_component_strip(*argv);
			if (DOT_OR_DOTDOT(base)) {
				bb_error_msg("can't remove '.' or '..'");
#if (defined(RTCONFIG_HND_ROUTER_AX_6756) || defined(RTCONFIG_HND_ROUTER_BE_4916)) && !defined(RTCONFIG_BCM_MFG)
			} else if (!f_exists("/tmp/asusdebug") && system_file(realpath(*argv, path))) {
				bb_error_msg("can't remove the system file");
#endif
			} else if (remove_file(*argv, flags) >= 0) {
				continue;
			}
			status = 1;
		} while (*++argv);
	} else if (!(flags & FILEUTILS_FORCE)) {
		bb_show_usage();
	}

	return status;
}
