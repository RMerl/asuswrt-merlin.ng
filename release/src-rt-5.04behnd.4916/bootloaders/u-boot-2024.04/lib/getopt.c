// SPDX-License-Identifier: GPL-2.0-only
/*
 * getopt.c - a simple getopt(3) implementation. See getopt.h for explanation.
 *
 * Copyright (C) 2020 Sean Anderson <seanga2@gmail.com>
 * Copyright (c) 2007 Sascha Hauer <s.hauer@pengutronix.de>, Pengutronix
 */

#define LOG_CATEGORY LOGC_CORE

#include <getopt.h>
#include <log.h>
#include <linux/string.h>

void getopt_init_state(struct getopt_state *gs)
{
	gs->index = 1;
	gs->arg_index = 1;
}

int __getopt(struct getopt_state *gs, int argc, char *const argv[],
	     const char *optstring, bool silent)
{
	char curopt;   /* current option character */
	const char *curoptp; /* pointer to the current option in optstring */

	while (1) {
		log_debug("arg_index: %d index: %d\n", gs->arg_index,
			  gs->index);

		/* `--` indicates the end of options */
		if (gs->arg_index == 1 && argv[gs->index] &&
		    !strcmp(argv[gs->index], "--")) {
			gs->index++;
			return -1;
		}

		/* Out of arguments */
		if (gs->index >= argc)
			return -1;

		/* Can't parse non-options */
		if (*argv[gs->index] != '-')
			return -1;

		/* We have found an option */
		curopt = argv[gs->index][gs->arg_index];
		if (curopt)
			break;
		/*
		 * no more options in current argv[] element; try the next one
		 */
		gs->index++;
		gs->arg_index = 1;
	}

	/* look up current option in optstring */
	curoptp = strchr(optstring, curopt);

	if (!curoptp) {
		if (!silent)
			printf("%s: invalid option -- %c\n", argv[0], curopt);
		gs->opt = curopt;
		gs->arg_index++;
		return '?';
	}

	if (*(curoptp + 1) != ':') {
		/* option with no argument. Just return it */
		gs->arg = NULL;
		gs->arg_index++;
		return curopt;
	}

	if (*(curoptp + 1) && *(curoptp + 2) == ':') {
		/* optional argument */
		if (argv[gs->index][gs->arg_index + 1]) {
			/* optional argument with directly following arg */
			gs->arg = argv[gs->index++] + gs->arg_index + 1;
			gs->arg_index = 1;
			return curopt;
		}
		if (gs->index + 1 == argc) {
			/* We are at the last argv[] element */
			gs->arg = NULL;
			gs->index++;
			return curopt;
		}
		if (*argv[gs->index + 1] != '-') {
			/*
			 * optional argument with arg in next argv[] element
			 */
			gs->index++;
			gs->arg = argv[gs->index++];
			gs->arg_index = 1;
			return curopt;
		}

		/* no optional argument found */
		gs->arg = NULL;
		gs->arg_index = 1;
		gs->index++;
		return curopt;
	}

	if (argv[gs->index][gs->arg_index + 1]) {
		/* required argument with directly following arg */
		gs->arg = argv[gs->index++] + gs->arg_index + 1;
		gs->arg_index = 1;
		return curopt;
	}

	gs->index++;
	gs->arg_index = 1;

	if (gs->index >= argc || argv[gs->index][0] == '-') {
		if (!silent)
			printf("option requires an argument -- %c\n", curopt);
		gs->opt = curopt;
		return ':';
	}

	gs->arg = argv[gs->index++];
	return curopt;
}
