/*
 *  Advanced Linux Sound Architecture Control Program
 *  Copyright (c) by Abramo Bagnara <abramo@alsa-project.org>
 *                   Jaroslav Kysela <perex@perex.cz>
 *
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
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */

#include "aconfig.h"
#include "version.h"
#include <getopt.h>
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <alsa/asoundlib.h>
#include "alsactl.h"

#define SYS_ASOUNDRC "/etc/asound.state"

int debugflag = 0;
int force_restore = 1;
int ignore_nocards = 0;
char *command;
char *statefile = NULL;

static void help(void)
{
	printf("Usage: alsactl <options> command\n");
	printf("\nAvailable global options:\n");
	printf("  -h,--help        this help\n");
	printf("  -d,--debug       debug mode\n");
	printf("  -v,--version     print version of this program\n");
	printf("\nAvailable state options:\n");
	printf("  -f,--file #      configuration file (default " SYS_ASOUNDRC ")\n");
	printf("  -F,--force       try to restore the matching controls as much as possible\n");
	printf("                   (default mode)\n");
	printf("  -g,--ignore      ignore 'No soundcards found' error\n");
	printf("  -P,--pedantic    do not restore mismatching controls (old default)\n");
	printf("  -I,--no-init-fallback\n"
	       "                   don't initialize even if restore fails\n");
	printf("  -r,--runstate #  save restore and init state to this file (only errors)\n");
	printf("                   default settings is 'no file set'\n");
	printf("  -R,--remove      remove runstate file at first, otherwise append errors\n");
	printf("\nAvailable init options:\n");
	printf("  -E,--env #=#	   set environment variable for init phase (NAME=VALUE)\n");
	printf("  -i,--initfile #  main configuation file for init phase (default " DATADIR "/init/00main)\n");
	printf("\n");
	printf("\nAvailable commands:\n");
	printf("  store   <card #> save current driver setup for one or each soundcards\n");
	printf("                   to configuration file\n");
	printf("  restore <card #> load current driver setup for one or each soundcards\n");
	printf("                   from configuration file\n");
	printf("  init	  <card #> initialize driver to a default state\n");
	printf("  names   <card #> dump information about all the known present (sub-)devices\n");
	printf("                   into configuration file (DEPRECATED)\n");
}

int main(int argc, char *argv[])
{
	static const struct option long_option[] =
	{
		{"help", 0, NULL, 'h'},
		{"file", 1, NULL, 'f'},
		{"env", 1, NULL, 'E'},
		{"initfile", 1, NULL, 'i'},
		{"no-init-fallback", 0, NULL, 'I'},
		{"force", 0, NULL, 'F'},
		{"ignore", 0, NULL, 'g'},
		{"pedantic", 0, NULL, 'P'},
		{"runstate", 0, NULL, 'r'},
		{"remove", 0, NULL, 'R'},
		{"debug", 0, NULL, 'd'},
		{"version", 0, NULL, 'v'},
		{NULL, 0, NULL, 0},
	};
	static const char *const devfiles[] = {
		"/dev/snd/controlC",
		"/dev/snd/pcmC",
		"/dev/snd/midiC",
		"/dev/snd/hwC",
		NULL
	};
	char *cfgfile = SYS_ASOUNDRC;
	char *initfile = DATADIR "/init/00main";
	char *cardname, ncardname[16];
	const char *const *tmp;
	int removestate = 0;
	int init_fallback = 1; /* new default behavior */
	int res;

	command = argv[0];
	while (1) {
		int c;

		if ((c = getopt_long(argc, argv, "hdvf:FgE:i:IPr:R", long_option, NULL)) < 0)
			break;
		switch (c) {
		case 'h':
			help();
			return EXIT_SUCCESS;
		case 'f':
			cfgfile = optarg;
			break;
		case 'F':
			force_restore = 1;
			break;
		case 'g':
			ignore_nocards = 1;
			break;
		case 'E':
			if (putenv(optarg)) {
				fprintf(stderr, "environment string '%s' is wrong\n", optarg);
				return EXIT_FAILURE;
			}
			break;
		case 'i':
			initfile = optarg;
			break;
		case 'I':
			init_fallback = 0;
			break;
		case 'r':
			statefile = optarg;
			break;
		case 'R':
			removestate = 1;
			break;
		case 'P':
			force_restore = 0;
			break;
		case 'd':
			debugflag = 1;
			break;
		case 'v':
			printf("alsactl version " SND_UTIL_VERSION_STR "\n");
			return EXIT_SUCCESS;
		case '?':		// error msg already printed
			help();
			return EXIT_FAILURE;
			break;
		default:		// should never happen
			fprintf(stderr, 
			"Invalid option '%c' (%d) not handled??\n", c, c);
		}
	}
	if (argc - optind <= 0) {
		fprintf(stderr, "alsactl: Specify command...\n");
		return 0;
	}

	cardname = argc - optind > 1 ? argv[optind + 1] : NULL;
	for (tmp = devfiles; cardname != NULL && *tmp != NULL; tmp++) {
		int len = strlen(*tmp);
		if (!strncmp(cardname, *tmp, len)) {
			long l = strtol(cardname + len, NULL, 0);
			sprintf(ncardname, "%li", l);
			cardname = ncardname;
			break;
		}
	}

	if (!strcmp(argv[optind], "init")) {
		res = init(initfile, cardname);
	} else if (!strcmp(argv[optind], "store")) {
		res = save_state(cfgfile, cardname);
	} else if (!strcmp(argv[optind], "restore")) {
		if (removestate)
			remove(statefile);
		res = load_state(cfgfile, initfile, cardname, init_fallback);
	} else {
		fprintf(stderr, "alsactl: Unknown command '%s'...\n", 
			argv[optind]);
		res = -ENODEV;
	}

	snd_config_update_free_global();
	return res < 0 ? -res : 0;
}
