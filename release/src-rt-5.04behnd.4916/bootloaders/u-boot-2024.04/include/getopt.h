/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * getopt.h - a simple getopt(3) implementation.
 *
 * Copyright (C) 2020 Sean Anderson <seanga2@gmail.com>
 * Copyright (c) 2007 Sascha Hauer <s.hauer@pengutronix.de>, Pengutronix
 */

#ifndef __GETOPT_H
#define __GETOPT_H

#include <stdbool.h>

/**
 * struct getopt_state - Saved state across getopt() calls
 */
struct getopt_state {
	/**
	 * @index: Index of the next unparsed argument of @argv. If getopt() has
	 * parsed all of @argv, then @index will equal @argc.
	 */
	int index;
	/* private: */
	/** @arg_index: Index within the current argument */
	int arg_index;
	union {
		/* public: */
		/**
		 * @opt: Option being parsed when an error occurs. @opt is only
		 * valid when getopt() returns ``?`` or ``:``.
		 */
		int opt;
		/**
		 * @arg: The argument to an option, NULL if there is none. @arg
		 * is only valid when getopt() returns an option character.
		 */
		char *arg;
	/* private: */
	};
};

/**
 * getopt_init_state() - Initialize a &struct getopt_state
 * @gs: The state to initialize
 *
 * This must be called before using @gs with getopt().
 */
void getopt_init_state(struct getopt_state *gs);

int __getopt(struct getopt_state *gs, int argc, char *const argv[],
	     const char *optstring, bool silent);

/**
 * getopt() - Parse short command-line options
 * @gs: Internal state and out-of-band return arguments. This must be
 *      initialized with getopt_init_context() beforehand.
 * @argc: Number of arguments, not including the %NULL terminator
 * @argv: Argument list, terminated by %NULL
 * @optstring: Option specification, as described below
 *
 * getopt() parses short options. Short options are single characters. They may
 * be followed by a required argument or an optional argument. Arguments to
 * options may occur in the same argument as an option (like ``-larg``), or
 * in the following argument (like ``-l arg``). An argument containing
 * options begins with a ``-``. If an option expects no arguments, then it may
 * be immediately followed by another option (like ``ls -alR``).
 *
 * @optstring is a list of accepted options. If an option is followed by ``:``
 * in @optstring, then it expects a mandatory argument. If an option is followed
 * by ``::`` in @optstring, it expects an optional argument. @gs.arg points
 * to the argument, if one is parsed.
 *
 * getopt() stops parsing options when it encounters the first non-option
 * argument, when it encounters the argument ``--``, or when it runs out of
 * arguments. For example, in ``ls -l foo -R``, option parsing will stop when
 * getopt() encounters ``foo``, if ``l`` does not expect an argument. However,
 * the whole list of arguments would be parsed if ``l`` expects an argument.
 *
 * An example invocation of getopt() might look like::
 *
 *     char *argv[] = { "program", "-cbx", "-a", "foo", "bar", 0 };
 *     int opt, argc = ARRAY_SIZE(argv) - 1;
 *     struct getopt_state gs;
 *
 *     getopt_init_state(&gs);
 *     while ((opt = getopt(&gs, argc, argv, "a::b:c")) != -1)
 *         printf("opt = %c, index = %d, arg = \"%s\"\n", opt, gs.index, gs.arg);
 *     printf("%d argument(s) left\n", argc - gs.index);
 *
 * and would produce an output of::
 *
 *     opt = c, index = 1, arg = "<NULL>"
 *     opt = b, index = 2, arg = "x"
 *     opt = a, index = 4, arg = "foo"
 *     1 argument(s) left
 *
 * For further information, refer to the getopt(3) man page.
 *
 * Return:
 * * An option character if an option is found. @gs.arg is set to the
 *   argument if there is one, otherwise it is set to ``NULL``.
 * * ``-1`` if there are no more options, if a non-option argument is
 *   encountered, or if an ``--`` argument is encountered.
 * * ``'?'`` if we encounter an option not in @optstring. @gs.opt is set to
 *   the unknown option.
 * * ``':'`` if an argument is required, but no argument follows the
 *   option. @gs.opt is set to the option missing its argument.
 *
 * @gs.index is always set to the index of the next unparsed argument in @argv.
 */
static inline int getopt(struct getopt_state *gs, int argc,
			 char *const argv[], const char *optstring)
{
	return __getopt(gs, argc, argv, optstring, false);
}

/**
 * getopt_silent() - Parse short command-line options silently
 * @gs: State
 * @argc: Argument count
 * @argv: Argument list
 * @optstring: Option specification
 *
 * Same as getopt(), except no error messages are printed.
 */
static inline int getopt_silent(struct getopt_state *gs, int argc,
				char *const argv[], const char *optstring)
{
	return __getopt(gs, argc, argv, optstring, true);
}

#endif /* __GETOPT_H */
