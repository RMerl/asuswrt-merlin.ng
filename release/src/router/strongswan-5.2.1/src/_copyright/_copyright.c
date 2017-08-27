/*
 * copyright reporter
 * (just avoids having the info in more than one place in the source)
 * Copyright (C) 2001  Henry Spencer.
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

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

#include <library.h>

static const char *copyright[] = {
	"Copyright (C) 1999-2013",
	"    Henry Spencer, D. Hugh Redelmeier, Michael Richardson, Ken Bantoft,",
	"    Stephen J. Bevan, JuanJo Ciarlante, Thomas Egerer, Heiko Hund,",
	"    Mathieu Lafon, Stephane Laroche, Kai Martius, Stephan Scholz,",
	"    Tuomo Soini, Herbert Xu.",
	"",
	"    Martin Berner, Marco Bertossa, David Buechi, Ueli Galizzi,",
	"    Christoph Gysin, Andreas Hess, Patric Lichtsteiner, Michael Meier,",
	"    Andreas Schleiss, Ariane Seiler, Mario Strasser, Lukas Suter,",
	"    Roger Wegmann, Simon Zwahlen,",
	"    ZHW Zuercher Hochschule Winterthur (Switzerland).",
	"",
	"    Philip Boetschi, Tobias Brunner, Christoph Buehler, Reto Buerki,",
	"    Sansar Choinyambuu, Adrian Doerig, Andreas Eigenmann, Giuliano Grassi,",
	"    Reto Guadagnini, Fabian Hartmann, Noah Heusser, Jan Hutter,",
	"    Thomas Kallenberg, Patrick Loetscher, Daniel Roethlisberger,",
	"    Adrian-Ken Rueegsegger, Ralf Sager, Joel Stillhart, Daniel Wydler,",
	"    Andreas Steffen,",
	"    HSR Hochschule fuer Technik Rapperswil (Switzerland).",
	"",
	"    Martin Willi (revosec AG), Clavister (Sweden).",
	"",
	"This program is free software; you can redistribute it and/or modify it",
	"under the terms of the GNU General Public License as published by the",
	"Free Software Foundation; either version 2 of the License, or (at your",
	"option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.",
	"",
	"This program is distributed in the hope that it will be useful, but",
	"WITHOUT ANY WARRANTY; without even the implied warranty of",
	"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General",
	"Public License (file COPYING in the distribution) for more details.",
	NULL,
};

char usage[] = "Usage: ipsec _copyright";
struct option opts[] = {
  {"help",	0,	NULL,	'h',},
  {"version",	0,	NULL,	'v',},
  {0,		0,	NULL,	0, },
};

char me[] = "ipsec _copyright";	/* for messages */

int
main(int argc, char *argv[])
{
	int opt;
	extern int optind;
	int errflg = 0;
	const char **notice = copyright;
	const char **co;

	library_init(NULL, "_copyright");
	atexit(library_deinit);

	while ((opt = getopt_long(argc, argv, "", opts, NULL)) != EOF)
		switch (opt) {
		case 'h':	/* help */
			printf("%s\n", usage);
			exit(0);
			break;
		case 'v':	/* version */
			printf("%s strongSwan "VERSION"\n", me);
			exit(0);
			break;
		case '?':
		default:
			errflg = 1;
			break;
		}
	if (errflg || optind != argc) {
		fprintf(stderr, "%s\n", usage);
		exit(2);
	}

	for (co = notice; *co != NULL; co++)
		printf("%s\n", *co);
	exit(0);
}
