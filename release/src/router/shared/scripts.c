/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <dirent.h>
#include <ctype.h>
#include "shutils.h"
#include "shared.h"


long fappend(FILE *out, const char *fname)
{
	FILE *in;
	char buf[1024];
	int n;
	long r;

	if ((in = fopen(fname, "r")) == NULL) return -1;
	r = 0;
	while ((n = fread(buf, 1, sizeof(buf), in)) > 0) {
		if (fwrite(buf, 1, n, out) != n) {
			r = -1;
			break;
		}
		else {
			r += n;
		}
	}
	fclose(in);
	return r;
}

void run_custom_script(char *name, int timeout, char *arg1, char *arg2)
{
	char script[120];
	char *cmd[4];
	int pid;
	struct stat st;
	char *error;

	snprintf(script, sizeof(script), "/jffs/scripts/%s", name);

	if (!stat(script, &st)) {
		if (nvram_match("jffs2_scripts", "0"))
			error = "custom script execution is disabled!";
		else if (!(st.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)))
			error = "script is not set executable!";
		else
			error = NULL;

		if (error) {
			logmessage("custom_script", "Found %s, but %s", name, error);
			return;
		}

		if (arg1)
			logmessage("custom_script" ,"Running %s (args: %s%s%s)", script, arg1, (arg2 ? " " : ""), (arg2 ? arg2 : ""));
		else
			logmessage("custom_script" ,"Running %s", script);

		cmd[0] = script;
		cmd[1] = arg1;
		cmd[2] = arg2;
		cmd[3] = NULL;
		_eval( cmd, NULL, timeout, (timeout ? NULL : &pid));
	}
}


void run_postconf(char *name, char *config)
{
	char filename[64];

	snprintf(filename, sizeof (filename), "%s.postconf", name);
	run_custom_script(filename, 120, config, NULL);
}


void use_custom_config(char *config, char *target)
{
        char filename[256];

        snprintf(filename, sizeof(filename), "/jffs/configs/%s", config);

	if (f_exists(filename)) {
		if (nvram_match("jffs2_scripts", "0")) {
			logmessage("custom config", "Found %s, but custom configs are disabled!", filename);
			return;
		}
		logmessage("custom config", "Using custom %s config file.", filename);
		eval("cp", filename, target, NULL);
	}
}


void append_custom_config(char *config, FILE *fp)
{
	char filename[256];

	snprintf(filename, sizeof(filename), "/jffs/configs/%s.add", config);

	if (f_exists(filename)) {
		if (nvram_match("jffs2_scripts", "0")) {
			logmessage("custom config", "Found %s, but custom configs are disabled!", filename);
			return;
		}
		logmessage("custom config", "Appending content of %s.", filename);
		fappend(fp, filename);
	}
}

