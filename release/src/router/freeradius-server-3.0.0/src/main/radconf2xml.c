/*
 * radconf2xml.c	Converts radiusd.conf to XML.
 *
 * Version:	$Id$
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 *
 * Copyright 2008   The FreeRADIUS server project
 * Copyright 2008   Alan DeKok <aland@deployingradius.com>
 */

RCSID("$Id$")

#include <freeradius-devel/radiusd.h>

#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

/*
 *	For configuration file stuff.
 */
char const *raddb_dir = RADDBDIR;
char const *progname = "radconf2xml";

/*
 *	The rest of this is because the conffile.c, etc. assume
 *	they're running inside of the server.  And we don't (yet)
 *	have a "libfreeradius-server", or "libfreeradius-util".
 */
log_debug_t debug_flag = 0;
struct main_config_t mainconfig;
char *request_log_file = NULL;
char *debug_log_file = NULL;

#include <sys/wait.h>
pid_t rad_fork(void)
{
	return fork();
}

pid_t rad_waitpid(pid_t pid, int *status)
{
	return waitpid(pid, status, 0);
}
int check_config = false;

static int usage(void)
{
	printf("Usage: %s [ -d raddb_dir ] [ -o output_file ] [ -n name ]\n", progname);
	printf("  -d raddb_dir    Configuration files are in \"raddbdir/*\".\n");
	printf("  -n name	 Read raddb/name.conf instead of raddb/radiusd.conf\n");
	printf("  -o output_file  File where XML output will be written.\n");

	exit(1);
}

int main(int argc, char **argv)
{
	int argval;
	CONF_SECTION *cs;
	char const *file = NULL;
	char const *name = "radiusd";
	FILE *fp;
	char buffer[2048];

	if ((progname = strrchr(argv[0], FR_DIR_SEP)) == NULL)
		progname = argv[0];
	else
		progname++;

	while ((argval = getopt(argc, argv, "d:ho:n:")) != EOF) {
		switch(argval) {
		case 'd':
			if (file) {
				fprintf(stderr, "%s: -d and -f cannot be used together.\n", progname);
				exit(1);
			}
			raddb_dir = optarg;
			break;

		default:
		case 'h':
			usage();
			break;

		case 'n':
			name = optarg;
			break;

		case 'o':
			file = optarg;
			break;
		}
	}

	snprintf(buffer, sizeof(buffer), "%s/%s.conf", raddb_dir, name);
	cs = cf_file_read(buffer);
	if (!cs) {
		fprintf(stderr, "%s: Errors reading or parsing %s\n",
			progname, buffer);
		exit(1);
	}

	if (!file || (strcmp(file, "-") == 0)) {
		fp = stdout;
		file = NULL;
	} else {
		fp = fopen(file, "w");
		if (!fp) {
			fprintf(stderr, "%s: Failed openng %s: %s\n",
				progname, file, strerror(errno));
			exit(1);
		}
	}

	if (!cf_section2xml(fp, cs)) {
		if (file) unlink(file);
		return 1;
	}

	return 0;
}
