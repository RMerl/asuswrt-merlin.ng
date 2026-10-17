// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2021 Weidmüller Interface GmbH & Co. KG
 * Roland Gaudig <roland.gaudig@weidmueller.com>
 *
 * Copyright 1999 Dave Cinege
 * Portions copyright (C) 1990-1996 Free Software Foundation, Inc.
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
/*
 * This file provides a shell printf like format string expansion as required
 * for the setexpr <name> fmt <format> <value> command.
 * This source file was mostly taken from the BusyBox project (www.busybox.net)
 * In contrast to the original sources the output is not written to stdout
 * anymore but into a char array, which can be used as input for the env_set()
 * function.
 */
/* Usage: printf format [argument...]
 *
 * A front end to the printf function that lets it be used from the shell.
 *
 * Backslash escapes:
 *
 * \" = double quote
 * \\ = backslash
 * \a = alert (bell)
 * \b = backspace
 * \c = produce no further output
 * \f = form feed
 * \n = new line
 * \r = carriage return
 * \t = horizontal tab
 * \v = vertical tab
 * \0ooo = octal number (ooo is 0 to 3 digits)
 * \xhhh = hexadecimal number (hhh is 1 to 3 digits)
 *
 * Additional directive:
 *
 * %b = print an argument string, interpreting backslash escapes
 *
 * The 'format' argument is re-used as many times as necessary
 * to convert all of the given arguments.
 *
 * David MacKenzie <djm@gnu.ai.mit.edu>
 */
/* 19990508 Busy Boxed! Dave Cinege */

//config:config PRINTF
//config:	bool "printf (3.8 kb)"
//config:	default y
//config:	help
//config:	printf is used to format and print specified strings.
//config:	It's similar to 'echo' except it has more options.

//applet:IF_PRINTF(APPLET_NOFORK(printf, printf, BB_DIR_USR_BIN, BB_SUID_DROP, printf))

//kbuild:lib-$(CONFIG_PRINTF) += printf.o
//kbuild:lib-$(CONFIG_ASH_PRINTF)  += printf.o
//kbuild:lib-$(CONFIG_HUSH_PRINTF) += printf.o

//usage:#define printf_trivial_usage
//usage:       "FORMAT [ARG]..."
//usage:#define printf_full_usage "\n\n"
//usage:       "Format and print ARG(s) according to FORMAT (a-la C printf)"
//usage:
//usage:#define printf_example_usage
//usage:       "$ printf \"Val=%d\\n\" 5\n"
//usage:       "Val=5\n"

/* A note on bad input: neither bash 3.2 nor coreutils 6.10 stop on it.
 * They report it:
 *  bash: printf: XXX: invalid number
 *  printf: XXX: expected a numeric value
 *  bash: printf: 123XXX: invalid number
 *  printf: 123XXX: value not completely converted
 * but then they use 0 (or partially converted numeric prefix) as a value
 * and continue. They exit with 1 in this case.
 * Both accept insane field width/precision (e.g. %9999999999.9999999999d).
 * Both print error message and assume 0 if %*.*f width/precision is "bad"
 *  (but negative numbers are not "bad").
 * Both accept negative numbers for %u specifier.
 *
 * We try to be compatible.
 */

#include <common.h>
#include <ctype.h>
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define WANT_HEX_ESCAPES 0
#define PRINT_CONVERSION_ERROR 1
#define PRINT_TRUNCATED_ERROR 2
#define PRINT_SIZE_ERROR 4

struct print_inf {
	char *str;
	size_t size;
	size_t offset;
	unsigned int error;
};

typedef void (*converter)(const char *arg, void *result);

/**
 * printf_str() - print formatted into char array with length checks
 *
 * This function povides a printf like function for printing into a char array
 * with checking the boundaries.
 * Unlike snprintf, all checks are performed inside this function and status
 * reports are stored inside the print_inf struct. That way, this function can
 * be used almost as drop-in replacement without needing much code changes.
 * Unlike snprintf errors are not reported by return value, but inside the
 * error member of struct print_inf. The output stored inside the struct
 * print_inf str member shall only be used when the error member is 0.
 *
 * @inf: Info structure for print operation
 * @char: format string with optional arguments
 */
static void printf_str(struct print_inf *inf, char *format, ...)
{
	va_list args;
	int i;

	if (!inf)
		return;

	/* Do not write anything if previous error is pending */
	if (inf->error)
		return;

	/* Check if end of receiving buffer is already reached */
	if (inf->offset >= inf->size) {
		inf->error |= PRINT_SIZE_ERROR;
		return;
	}

	size_t remaining = inf->size - inf->offset;

	va_start(args, format);
	i = vsnprintf(inf->str + inf->offset, remaining, format, args);
	va_end(args);

	if (i < 0)
		inf->error |= PRINT_CONVERSION_ERROR;
	else if ((unsigned int)i >= remaining)
		inf->error |= PRINT_TRUNCATED_ERROR;
	else
		inf->offset += i;
}

/**
 * putchar_str() - Print single character into char array with length checks
 *
 * This function provices a putchar like function, which stores the output
 * into a char array with checking boundaries.
 *
 * @inf: Info structure for print operation
 * @char: Single character to be printed
 */
static void putchar_str(struct print_inf *inf, char c)
{
	printf_str(inf, "%c", c);
}

static char process_escape_sequence(const char **ptr)
{
	const char *q;
	unsigned int num_digits;
	unsigned int n;
	unsigned int base;

	num_digits = 0;
	n = 0;
	base = 8;
	q = *ptr;

	if (WANT_HEX_ESCAPES && *q == 'x') {
		++q;
		base = 16;
		++num_digits;
	}

	/* bash requires leading 0 in octal escapes:
	 * \02 works, \2 does not (prints \ and 2).
	 * We treat \2 as a valid octal escape sequence.
	 */
	do {
		unsigned int r;
		unsigned int d = (unsigned char)(*q) - '0';
#if WANT_HEX_ESCAPES
		if (d >= 10) {
			d = (unsigned char)tolower(*q) - 'a';
			//d += 10;
			/* The above would map 'A'-'F' and 'a'-'f' to 10-15,
			 * however, some chars like '@' would map to 9 < base.
			 * Do not allow that, map invalid chars to N > base:
			 */
			if ((int)d >= 0)
				d += 10;
		}
#endif
		if (d >= base) {
			if (WANT_HEX_ESCAPES && base == 16) {
				--num_digits;
				if (num_digits == 0) {
					/* \x<bad_char>: return '\',
					 * leave ptr pointing to x
					 */
					return '\\';
				}
			}
			break;
		}

		r = n * base + d;
		if (r > 255)
			break;

		n = r;
		++q;
	} while (++num_digits < 3);

	if (num_digits == 0) {
		/* Not octal or hex escape sequence.
		 * Is it one-letter one?
		 */
		/* bash builtin "echo -e '\ec'" interprets \e as ESC,
		 * but coreutils "/bin/echo -e '\ec'" does not.
		 * Manpages tend to support coreutils way.
		 * Update: coreutils added support for \e on 28 Oct 2009.
		 */
		static const char charmap[] = {
			'a',  'b', 'e', 'f',  'n',  'r',  't',  'v',  '\\', '\0',
			'\a', '\b', 27, '\f', '\n', '\r', '\t', '\v', '\\', '\\',
		};

		const char *p = charmap;

		do {
			if (*p == *q) {
				q++;
				break;
			}
		} while (*++p != '\0');
		/* p points to found escape char or NUL,
		 * advance it and find what it translates to.
		 * Note that \NUL and unrecognized sequence \z return '\'
		 * and leave ptr pointing to NUL or z.
		 */
		n = p[sizeof(charmap) / 2];
	}

	*ptr = q;

	return (char)n;
}

static char *skip_whitespace(const char *s)
{
	/* In POSIX/C locale (the only locale we care about: do we REALLY want
	 * to allow Unicode whitespace in, say, .conf files? nuts!)
	 * isspace is only these chars: "\t\n\v\f\r" and space.
	 * "\t\n\v\f\r" happen to have ASCII codes 9,10,11,12,13.
	 * Use that.
	 */
	while (*s == ' ' || (unsigned char)(*s - 9) <= (13 - 9))
		s++;

	return (char *)s;
}

/* Like strcpy but can copy overlapping strings. */
static void overlapping_strcpy(char *dst, const char *src)
{
	/* Cheap optimization for dst == src case -
	 * better to have it here than in many callers.
	 */
	if (dst != src) {
		while ((*dst = *src) != '\0') {
			dst++;
			src++;
		}
	}
}

static int multiconvert(const char *arg, void *result, converter convert)
{
	if (*arg == '"' || *arg == '\'')
		sprintf((char *)arg + strlen(arg), "%u", (unsigned char)arg[1]);
	//errno = 0;
	convert(arg, result);
	/* Unlike their Posix counterparts, simple_strtoll and
	 * simple_strtoull do not set errno
	 *
	 * if (errno) {
	 *	printf("error invalid number '%s'", arg);
	 *	return 1;
	 * }
	 */
	return 0;
}

static void conv_strtoull(const char *arg, void *result)
{
	/* both coreutils 6.10 and bash 3.2:
	 * $ printf '%x\n' -2
	 * fffffffffffffffe
	 * Mimic that:
	 */
	if (arg[0] == '-') {
		*(unsigned long long *)result = simple_strtoll(arg, NULL, 16);
		return;
	}
	/* Allow leading '+' - simple_strtoull() by itself does not allow it,
	 * and probably shouldn't (other callers might require purely numeric
	 * inputs to be allowed.
	 */
	if (arg[0] == '+')
		arg++;
	*(unsigned long long *)result = simple_strtoull(arg, NULL, 16);
}

static void conv_strtoll(const char *arg, void *result)
{
	if (arg[0] == '+')
		arg++;
	*(long long *)result = simple_strtoll(arg, NULL, 16);
}

/* Callers should check errno to detect errors */
static unsigned long long my_xstrtoull(const char *arg)
{
	unsigned long long result;

	if (multiconvert(arg, &result, conv_strtoull))
		result = 0;
	return result;
}

static long long my_xstrtoll(const char *arg)
{
	long long result;

	if (multiconvert(arg, &result, conv_strtoll))
		result = 0;
	return result;
}

/* Handles %b; return 1 if output is to be short-circuited by \c */
static int print_esc_string(struct print_inf *inf, const char *str)
{
	char c;

	while ((c = *str) != '\0') {
		str++;
		if (c == '\\') {
			/* %b also accepts 4-digit octals of the form \0### */
			if (*str == '0') {
				if ((unsigned char)(str[1] - '0') < 8) {
					/* 2nd char is 0..7: skip leading '0' */
					str++;
				}
			} else if (*str == 'c') {
				return 1;
			}
			{
				/* optimization: don't force arg to be on-stack,
				 * use another variable for that.
				 */
				const char *z = str;

				c = process_escape_sequence(&z);
				str = z;
			}
		}
		putchar_str(inf, c);
	}

	return 0;
}

static void print_direc(struct print_inf *inf, char *format, unsigned int fmt_length,
			int field_width, int precision,
			const char *argument)
{
	long long llv;
	char saved;
	char *have_prec, *have_width;

	saved = format[fmt_length];
	format[fmt_length] = '\0';

	have_prec = strstr(format, ".*");
	have_width = strchr(format, '*');
	if (have_width - 1 == have_prec)
		have_width = NULL;

	/* multiconvert sets errno = 0, but %s needs it cleared */
	errno = 0;

	switch (format[fmt_length - 1]) {
	case 'c':
		printf_str(inf, format, *argument);
		break;
	case 'd':
	case 'i':
		llv = my_xstrtoll(skip_whitespace(argument));
 print_long:
		if (!have_width) {
			if (!have_prec)
				printf_str(inf, format, llv);
			else
				printf_str(inf, format, precision, llv);
		} else {
			if (!have_prec)
				printf_str(inf, format, field_width, llv);
			else
				printf_str(inf, format, field_width, precision, llv);
		}
		break;
	case 'o':
	case 'u':
	case 'x':
	case 'X':
		llv = my_xstrtoull(skip_whitespace(argument));
		/* cheat: unsigned long and long have same width, so... */
		goto print_long;
	case 's':
		/* Are char* and long long the same? */
		if (sizeof(argument) == sizeof(llv)) {
			llv = (long long)(ptrdiff_t)argument;
			goto print_long;
		} else {
			/* Hope compiler will optimize it out by moving call
			 * instruction after the ifs...
			 */
			if (!have_width) {
				if (!have_prec)
					printf_str(inf, format, argument,
						   /*unused:*/ argument, argument);
				else
					printf_str(inf, format, precision,
						   argument, /*unused:*/ argument);
			} else {
				if (!have_prec)
					printf_str(inf, format, field_width,
						   argument, /*unused:*/ argument);
				else
					printf_str(inf, format, field_width,
						   precision, argument);
			}
			break;
		}
		break;
	} /* switch */

	format[fmt_length] = saved;
}

/* Handle params for "%*.*f". Negative numbers are ok (compat). */
static int get_width_prec(const char *str)
{
	long v = simple_strtol(str, NULL, 10);

	/* Unlike its Posix counterpart, simple_strtol does not set errno
	 *
	 * if (errno) {
	 *	printf("error invalid number '%s'", str);
	 *	v = 0;
	 * }
	 */
	return (int)v;
}

/* Print the text in FORMAT, using ARGV for arguments to any '%' directives.
 * Return advanced ARGV.
 */
static char **print_formatted(struct print_inf *inf, char *f, char **argv, int *conv_err)
{
	char *direc_start;          /* Start of % directive.  */
	unsigned int direc_length;  /* Length of % directive.  */
	int field_width;            /* Arg to first '*' */
	int precision;              /* Arg to second '*' */
	char **saved_argv = argv;

	for (; *f; ++f) {
		switch (*f) {
		case '%':
			direc_start = f++;
			direc_length = 1;
			field_width = 0;
			precision = 0;
			if (*f == '%') {
				putchar_str(inf, '%');
				break;
			}
			if (*f == 'b') {
				if (*argv) {
					if (print_esc_string(inf, *argv))
						return saved_argv; /* causes main() to exit */
					++argv;
				}
				break;
			}
			if (*f && strchr("-+ #", *f)) {
				++f;
				++direc_length;
			}
			if (*f == '*') {
				++f;
				++direc_length;
				if (*argv)
					field_width = get_width_prec(*argv++);
			} else {
				while (isdigit(*f)) {
					++f;
					++direc_length;
				}
			}
			if (*f == '.') {
				++f;
				++direc_length;
				if (*f == '*') {
					++f;
					++direc_length;
					if (*argv)
						precision = get_width_prec(*argv++);
				} else {
					while (isdigit(*f)) {
						++f;
						++direc_length;
					}
				}
			}

			/* Remove "lLhz" size modifiers, repeatedly.
			 * bash does not like "%lld", but coreutils
			 * happily takes even "%Llllhhzhhzd"!
			 * We are permissive like coreutils
			 */
			while ((*f | 0x20) == 'l' || *f == 'h' || *f == 'z')
				overlapping_strcpy(f, f + 1);
			/* Add "ll" if integer modifier, then print */
			{
				static const char format_chars[] = "diouxXcs";
				char *p = strchr(format_chars, *f);
				/* needed - try "printf %" without it */
				if (!p || *f == '\0') {
					printf("`%s': invalid format\n", direc_start);
					/* causes main() to exit with error */
					return saved_argv - 1;
				}
				++direc_length;
				if (p - format_chars <= 5) {
					/* it is one of "diouxX" */
					p = malloc(direc_length + 3);
					if (!p) {
						/* exit with error */
						return saved_argv - 1;
					}
					memcpy(p, direc_start, direc_length);
					p[direc_length + 1] = p[direc_length - 1];
					p[direc_length - 1] = 'l';
					p[direc_length] = 'l';
					//bb_error_msg("<%s>", p);
					direc_length += 2;
					direc_start = p;
				} else {
					p = NULL;
				}
				if (*argv) {
					print_direc(inf, direc_start, direc_length,
						    field_width, precision, *argv++);
				} else {
					print_direc(inf, direc_start, direc_length,
						    field_width, precision, "");
				}
				*conv_err |= errno;
				free(p);
			}
			break;
		case '\\':
			if (*++f == 'c')
				return saved_argv; /* causes main() to exit */
			putchar_str(inf, process_escape_sequence((const char **)&f));
			f--;
			break;
		default:
			putchar_str(inf, *f);
		}
	}

	return argv;
}

/**
 * printf_setexpr() - Implements the setexpr <name> fmt <format> command
 *
 * This function implements the format string evaluation for the
 * setexpr <name> fmt <format> <value> command.
 *
 * @str: Output string of the evaluated expression
 * @size: Length of @str buffer
 * @argc: Number of arguments
 * @argv: Argument list
 * @return: 0 if OK, 1 on error
 */
int printf_setexpr(char *str, size_t size, int argc, char *const *argv)
{
	int conv_err;
	char *format;
	char **argv2;
	struct print_inf inf = {
		.str = str,
		.size = size,
		.offset = 0,
		.error = 0,
	};

	if (!str || !size)
		return 1;

	inf.str[0] = '\0';

	format = argv[0];
	argv2 = (char **)argv + 1;

	conv_err = 0;
	argv = argv2;
	/* In case any print_str call raises an error inf.error will be
	 * set after print_formatted returns.
	 */
	argv2 = print_formatted(&inf, format, (char **)argv, &conv_err);

	/* coreutils compat (bash doesn't do this):
	 *if (*argv)
	 *	fprintf(stderr, "excess args ignored");
	 */

	return (argv2 < argv) || /* if true, print_formatted errored out */
		conv_err || /* print_formatted saw invalid number */
		inf.error;  /* print_str reported error */
}
