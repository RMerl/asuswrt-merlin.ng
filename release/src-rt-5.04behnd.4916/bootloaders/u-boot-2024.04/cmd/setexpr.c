// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2008 Freescale Semiconductor, Inc.
 * Copyright 2013 Wolfgang Denk <wd@denx.de>
 */

/*
 * This file provides a shell like 'expr' function to return.
 */

#include <common.h>
#include <config.h>
#include <command.h>
#include <ctype.h>
#include <env.h>
#include <log.h>
#include <malloc.h>
#include <mapmem.h>
#include <linux/sizes.h>
#include "printf.h"

#define MAX_STR_LEN 128

/**
 * struct expr_arg: Holds an argument to an expression
 *
 * @ival: Integer value (if width is not CMD_DATA_SIZE_STR)
 * @sval: String value (if width is CMD_DATA_SIZE_STR)
 */
struct expr_arg {
	union {
		ulong ival;
		char *sval;
	};
};

static int get_arg(char *s, int w, struct expr_arg *argp)
{
	struct expr_arg arg;

	/*
	 * If the parameter starts with a '*' then assume it is a pointer to
	 * the value we want.
	 */
	if (s[0] == '*') {
		ulong *p;
		ulong addr;
		ulong val;
		int len;
		char *str;

		addr = hextoul(&s[1], NULL);
		switch (w) {
		case 1:
			p = map_sysmem(addr, sizeof(uchar));
			val = (ulong)*(uchar *)p;
			unmap_sysmem(p);
			arg.ival = val;
			break;
		case 2:
			p = map_sysmem(addr, sizeof(ushort));
			val = (ulong)*(ushort *)p;
			unmap_sysmem(p);
			arg.ival = val;
			break;
		case CMD_DATA_SIZE_STR:
			p = map_sysmem(addr, SZ_64K);

			/* Maximum string length of 64KB plus terminator */
			len = strnlen((char *)p, SZ_64K) + 1;
			str = malloc(len);
			if (!str) {
				printf("Out of memory\n");
				return -ENOMEM;
			}
			memcpy(str, p, len);
			str[len - 1] = '\0';
			unmap_sysmem(p);
			arg.sval = str;
			break;
		case 4:
			p = map_sysmem(addr, sizeof(u32));
			val = *(u32 *)p;
			unmap_sysmem(p);
			arg.ival = val;
			break;
		default:
			p = map_sysmem(addr, sizeof(ulong));
			val = *p;
			unmap_sysmem(p);
			arg.ival = val;
			break;
		}
	} else {
		if (w == CMD_DATA_SIZE_STR)
			return -EINVAL;
		arg.ival = hextoul(s, NULL);
	}
	*argp = arg;

	return 0;
}

#ifdef CONFIG_REGEX

#include <slre.h>

/*
 * memstr - Find the first substring in memory
 * @s1: The string to be searched
 * @s2: The string to search for
 *
 * Similar to and based on strstr(),
 * but strings do not need to be NUL terminated.
 */
static char *memstr(const char *s1, int l1, const char *s2, int l2)
{
	if (!l2)
		return (char *)s1;

	while (l1 >= l2) {
		l1--;
		if (!memcmp(s1, s2, l2))
			return (char *)s1;
		s1++;
	}
	return NULL;
}

/**
 * substitute() - Substitute part of one string with another
 *
 * This updates @string so that the first occurrence of @old is replaced with
 * @new
 *
 * @string: String buffer containing string to update at the start
 * @slen: Pointer to current string length, updated on success
 * @ssize: Size of string buffer
 * @old: Old string to find in the buffer (no terminator needed)
 * @olen: Length of @old excluding terminator
 * @new: New string to replace @old with
 * @nlen: Length of @new excluding terminator
 * Return: pointer to immediately after the copied @new in @string, or NULL if
 *	no replacement took place
 */
static char *substitute(char *string, int *slen, int ssize,
			const char *old, int olen, const char *new, int nlen)
{
	char *p = memstr(string, *slen, old, olen);

	if (p == NULL)
		return NULL;

	debug("## Match at pos %ld: match len %d, subst len %d\n",
		(long)(p - string), olen, nlen);

	/* make sure replacement matches */
	if (*slen + nlen - olen > ssize) {
		printf("## error: substitution buffer overflow\n");
		return NULL;
	}

	/* move tail if needed */
	if (olen != nlen) {
		int tail, len;

		len = (olen > nlen) ? olen : nlen;

		tail = ssize - (p + len - string);

		debug("## tail len %d\n", tail);

		memmove(p + nlen, p + olen, tail);
	}

	/* insert substitute */
	memcpy(p, new, nlen);

	*slen += nlen - olen;

	return p + nlen;
}

int setexpr_regex_sub(char *data, uint data_size, char *nbuf, uint nbuf_size,
		      const char *r, const char *s, bool global)
{
	struct slre slre;
	char *datap = data;
	int res, len, nlen, loop;

	if (slre_compile(&slre, r) == 0) {
		printf("Error compiling regex: %s\n", slre.err_str);
		return 1;
	}

	len = strlen(data);
	for (loop = 0;; loop++) {
		struct cap caps[slre.num_caps + 2];
		const char *old;
		char *np;
		int i, olen;

		(void) memset(caps, 0, sizeof(caps));

		res = slre_match(&slre, datap, len - (datap - data), caps);

		debug("Result: %d\n", res);

		for (i = 0; i <= slre.num_caps; i++) {
			if (caps[i].len > 0) {
				debug("Substring %d: [%.*s]\n", i,
					caps[i].len, caps[i].ptr);
			}
		}

		if (res == 0) {
			if (loop == 0) {
				debug("%s: No match\n", data);
				return 1;
			} else {
				break;
			}
		}

		debug("## MATCH ## %s\n", data);

		if (!s)
			return 1;

		old = caps[0].ptr;
		olen = caps[0].len;
		nlen = strlen(s);

		if (nlen + 1 >= nbuf_size) {
			printf("## error: pattern buffer overflow: have %d, need %d\n",
			       nbuf_size, nlen + 1);
			return 1;
		}
		strcpy(nbuf, s);

		debug("## SUBST(1) ## %s\n", nbuf);

		/*
		 * Handle back references
		 *
		 * Support for \0 ... \9, where \0 is the
		 * whole matched pattern (similar to &).
		 *
		 * Implementation is a bit simpleminded as
		 * backrefs are substituted sequentially, one
		 * by one.  This will lead to somewhat
		 * unexpected results if the replacement
		 * strings contain any \N strings then then
		 * may get substitued, too.  We accept this
		 * restriction for the sake of simplicity.
		 */
		for (i = 0; i < 10; ++i) {
			char backref[2] = {
				'\\',
				'0',
			};

			if (caps[i].len == 0)
				break;

			backref[1] += i;

			debug("## BACKREF %d: replace \"%.*s\" by \"%.*s\" in \"%s\"\n",
				i,
				2, backref,
				caps[i].len, caps[i].ptr,
				nbuf);

			for (np = nbuf;;) {
				char *p = memstr(np, nlen, backref, 2);

				if (p == NULL)
					break;

				np = substitute(np, &nlen,
					nbuf_size - (np - nbuf),
					backref, 2,
					caps[i].ptr, caps[i].len);

				if (np == NULL)
					return 1;
			}
		}
		debug("## SUBST(2) ## %s\n", nbuf);

		datap = substitute(datap, &len, data_size - (datap - data),
				   old, olen, nbuf, nlen);

		if (datap == NULL)
			return 1;

		debug("## REMAINDER: %s\n", datap);

		debug("## RESULT: %s\n", data);

		if (!global)
			break;
	}
	debug("## FINAL (now env_set()) :  %s\n", data);

	return 0;
}

#define SLRE_BUFSZ	16384
#define SLRE_PATSZ	4096

/*
 * Perform regex operations on a environment variable
 *
 * Returns 0 if OK, 1 in case of errors.
 */
static int regex_sub_var(const char *name, const char *r, const char *s,
			 const char *t, int global)
{
	struct slre slre;
	char data[SLRE_BUFSZ];
	char nbuf[SLRE_PATSZ];
	const char *value;
	int len;
	int ret;

	if (!name)
		return 1;

	if (slre_compile(&slre, r) == 0) {
		printf("Error compiling regex: %s\n", slre.err_str);
		return 1;
	}

	if (!t) {
		value = env_get(name);
		if (!value) {
			printf("## Error: variable \"%s\" not defined\n", name);
			return 1;
		}
		t = value;
	}

	debug("REGEX on %s=%s\n", name, t);
	debug("REGEX=\"%s\", SUBST=\"%s\", GLOBAL=%d\n", r, s ? s : "<NULL>",
	      global);

	len = strlen(t);
	if (len + 1 > SLRE_BUFSZ) {
		printf("## error: subst buffer overflow: have %d, need %d\n",
		       SLRE_BUFSZ, len + 1);
		return 1;
	}

	strcpy(data, t);

	ret = setexpr_regex_sub(data, SLRE_BUFSZ, nbuf, SLRE_PATSZ, r, s,
				global);
	if (ret)
		return 1;

	debug("%s=%s\n", name, data);

	return env_set(name, data);
}
#endif

static int do_setexpr(struct cmd_tbl *cmdtp, int flag, int argc,
		      char *const argv[])
{
	struct expr_arg aval, bval;
	ulong value;
	int ret = 0;
	int w;

	/*
	 * We take 3, 5, or 6 arguments, except fmt operation, which
	 * takes 4 to 8 arguments (limited by _maxargs):
	 * 3 : setexpr name value
	 * 5 : setexpr name val1 op val2
	 *     setexpr name [g]sub r s
	 * 6 : setexpr name [g]sub r s t
	 *     setexpr name fmt format [val1] [val2] [val3] [val4]
	 */

	if (argc < 3)
		return CMD_RET_USAGE;

	w = cmd_get_data_size(argv[0], 4);

	if (get_arg(argv[2], w, &aval))
		return CMD_RET_FAILURE;

	/* format string assignment: "setexpr name fmt %d value" */
	if (strcmp(argv[2], "fmt") == 0 && IS_ENABLED(CONFIG_CMD_SETEXPR_FMT)) {
		char str[MAX_STR_LEN];
		int result;

		if (argc == 3)
			return CMD_RET_USAGE;

		result = printf_setexpr(str, sizeof(str), argc - 3, &argv[3]);
		if (result)
			return result;

		return env_set(argv[1], str);
	}

	if (argc == 4 || argc > 6)
		return CMD_RET_USAGE;

	/* plain assignment: "setexpr name value" */
	if (argc == 3) {
		if (w == CMD_DATA_SIZE_STR) {
			ret = env_set(argv[1], aval.sval);
			free(aval.sval);
		} else {
			ret = env_set_hex(argv[1], aval.ival);
		}

		return ret;
	}

	/* 5 or 6 args (6 args only with [g]sub) */
#ifdef CONFIG_REGEX
	/*
	 * rexep handling: "setexpr name [g]sub r s [t]"
	 * with 5 args, "t" will be NULL
	 */
	if (strcmp(argv[2], "gsub") == 0)
		return regex_sub_var(argv[1], argv[3], argv[4], argv[5], 1);

	if (strcmp(argv[2], "sub") == 0)
		return regex_sub_var(argv[1], argv[3], argv[4], argv[5], 0);
#endif

	/* standard operators: "setexpr name val1 op val2" */
	if (argc != 5)
		return CMD_RET_USAGE;

	if (strlen(argv[3]) != 1)
		return CMD_RET_USAGE;

	if (get_arg(argv[4], w, &bval)) {
		if (w == CMD_DATA_SIZE_STR)
			free(aval.sval);
		return CMD_RET_FAILURE;
	}

	if (w == CMD_DATA_SIZE_STR) {
		int len;
		char *str;

		switch (argv[3][0]) {
		case '+':
			len = strlen(aval.sval) + strlen(bval.sval) + 1;
			str = malloc(len);
			if (!str) {
				printf("Out of memory\n");
				ret = CMD_RET_FAILURE;
			} else {
				/* These were copied out and checked earlier */
				strcpy(str, aval.sval);
				strcat(str, bval.sval);
				ret = env_set(argv[1], str);
				if (ret)
					printf("Could not set var\n");
				free(str);
			}
			break;
		default:
			printf("invalid op\n");
			ret = 1;
		}
	} else {
		ulong a = aval.ival;
		ulong b = bval.ival;

		switch (argv[3][0]) {
		case '|':
			value = a | b;
			break;
		case '&':
			value = a & b;
			break;
		case '+':
			value = a + b;
			break;
		case '^':
			value = a ^ b;
			break;
		case '-':
			value = a - b;
			break;
		case '*':
			value = a * b;
			break;
		case '/':
			value = a / b;
			break;
		case '%':
			value = a % b;
			break;
		default:
			printf("invalid op\n");
			return 1;
		}

		env_set_hex(argv[1], value);
	}

	if (w == CMD_DATA_SIZE_STR) {
		free(aval.sval);
		free(bval.sval);
	}

	return ret;
}

U_BOOT_CMD(
	setexpr, 8, 0, do_setexpr,
	"set environment variable as the result of eval expression",
	"[.b, .w, .l, .s] name [*]value1 <op> [*]value2\n"
	"    - set environment variable 'name' to the result of the evaluated\n"
	"      expression specified by <op>.  <op> can be &, |, ^, +, -, *, /, %\n"
	"      (for strings only + is supported)\n"
	"      size argument is only meaningful if value1 and/or value2 are\n"
	"      memory addresses (*)\n"
	"setexpr[.b, .w, .l] name [*]value\n"
	"    - load a value into a variable"
#ifdef CONFIG_CMD_SETEXPR_FMT
	"\n"
	"setexpr name fmt <format> [value1] [value2] [value3] [value4]\n"
	"    - set environment variable 'name' to the result of the bash like\n"
	"      format string evaluation of value."
#endif
#ifdef CONFIG_REGEX
	"\n"
	"setexpr name gsub r s [t]\n"
	"    - For each substring matching the regular expression <r> in the\n"
	"      string <t>, substitute the string <s>.  The result is\n"
	"      assigned to <name>.  If <t> is not supplied, use the old\n"
	"      value of <name>\n"
	"setexpr name sub r s [t]\n"
	"    - Just like gsub(), but replace only the first matching substring"
#endif
);
