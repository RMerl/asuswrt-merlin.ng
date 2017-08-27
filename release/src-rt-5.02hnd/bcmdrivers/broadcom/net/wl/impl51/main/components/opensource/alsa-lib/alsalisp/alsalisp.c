/*
 *  ALSA lisp implementation
 *  Copyright (c) 2003 by Jaroslav Kysela <perex@perex.cz>
 *
 *
 *   This library is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as
 *   published by the Free Software Foundation; either version 2.1 of
 *   the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <err.h>

#include "asoundlib.h"
#include "alisp.h"

static int verbose = 0;
static int warning = 0;
static int debug = 0;

static void interpret_filename(const char *file)
{
	struct alisp_cfg cfg;
	snd_input_t *in;
	snd_output_t *out;
	int err;

	memset(&cfg, 0, sizeof(cfg));
	if (file != NULL && strcmp(file, "-") != 0) {
		if ((err = snd_input_stdio_open(&in, file, "r")) < 0) {
			fprintf(stderr, "unable to open filename '%s' (%s)\n", file, snd_strerror(err));
			return;
		}
	} else {
		if ((err = snd_input_stdio_attach(&in, stdin, 0)) < 0) {
			fprintf(stderr, "unable to attach stdin '%s' (%s)\n", file, snd_strerror(err));
			return;
		}
	}
	if (snd_output_stdio_attach(&out, stdout, 0) < 0) {
		snd_input_close(in);
		fprintf(stderr, "unable to attach stdout (%s)\n", strerror(errno));
		return;
	}
	cfg.verbose = verbose;
	cfg.warning = warning;
	cfg.debug = debug;
	cfg.in = in;
	cfg.out = cfg.eout = cfg.vout = cfg.wout = cfg.dout = out;
	err = alsa_lisp(&cfg, NULL);
	if (err < 0)
		fprintf(stderr, "alsa lisp returned error %i (%s)\n", err, strerror(err));
	else if (verbose)
		printf("file %s passed ok via alsa lisp interpreter\n", file);
	snd_output_close(out);
	snd_input_close(in);
}

static void usage(void)
{
	fprintf(stderr, "usage: alsalisp [-vdw] [file...]\n");
	exit(1);
}

int main(int argc, char **argv)
{
	int c;

	while ((c = getopt(argc, argv, "vdw")) != -1) {
		switch (c) {
		case 'v':
			verbose = 1;
			break;
		case 'd':
			debug = 1;
			break;
		case 'w':
			warning = 1;
			break;
		case '?':
		default:
			usage();
			/* NOTREACHED */
		}
	}
	argc -= optind;
	argv += optind;

	if (argc < 1)
		interpret_filename(NULL);
	else
		while (*argv)
			interpret_filename(*argv++);

	return 0;
}
