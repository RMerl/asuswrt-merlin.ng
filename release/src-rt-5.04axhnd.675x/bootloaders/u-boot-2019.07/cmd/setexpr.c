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
#include <mapmem.h>

static ulong get_arg(char *s, int w)
{
	/*
	 * If the parameter starts with a '*' then assume it is a pointer to
	 * the value we want.
	 */
	if (s[0] == '*') {
		ulong *p;
		ulong addr;
		ulong val;

		addr = simple_strtoul(&s[1], NULL, 16);
		switch (w) {
		case 1:
			p = map_sysmem(addr, sizeof(uchar));
			val = (ulong)*(uchar *)p;
			unmap_sysmem(p);
			return val;
		case 2:
			p = map_sysmem(addr, sizeof(ushort));
			val = (ulong)*(ushort *)p;
			unmap_sysmem(p);
			return val;
		case 4:
		default:
			p = map_sysmem(addr, sizeof(ulong));
			val = *p;
			unmap_sysmem(p);
			return val;
		}
	} else {
		return simple_strtoul(s, NULL, 16);
	}
}

#ifdef CONFIG_REGEX

#include <slre.h>

#define SLRE_BUFSZ	16384
#define SLRE_PATSZ	4096

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

static char *substitute(char *string,	/* string buffer */
			int *slen,	/* current string length */
			int ssize,	/* string bufer size */
			const char *old,/* old (replaced) string */
			int olen,	/* length of old string */
			const char *new,/* new (replacement) string */
			int nlen)	/* length of new string */
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

	/* insert substitue */
	memcpy(p, new, nlen);

	*slen += nlen - olen;

	return p + nlen;
}

/*
 * Perform regex operations on a environment variable
 *
 * Returns 0 if OK, 1 in case of errors.
 */
static int regex_sub(const char *name,
	const char *r, const char *s, const char *t,
	int global)
{
	struct slre slre;
	char data[SLRE_BUFSZ];
	char *datap = data;
	const char *value;
	int res, len, nlen, loop;

	if (name == NULL)
		return 1;

	if (slre_compile(&slre, r) == 0) {
		printf("Error compiling regex: %s\n", slre.err_str);
		return 1;
	}

	if (t == NULL) {
		value = env_get(name);

		if (value == NULL) {
			printf("## Error: variable \"%s\" not defined\n", name);
			return 1;
		}
		t = value;
	}

	debug("REGEX on %s=%s\n", name, t);
	debug("REGEX=\"%s\", SUBST=\"%s\", GLOBAL=%d\n",
		r, s ? s : "<NULL>", global);

	len = strlen(t);
	if (len + 1 > SLRE_BUFSZ) {
		printf("## error: subst buffer overflow: have %d, need %d\n",
			SLRE_BUFSZ, len + 1);
		return 1;
	}

	strcpy(data, t);

	if (s == NULL)
		nlen = 0;
	else
		nlen = strlen(s);

	for (loop = 0;; loop++) {
		struct cap caps[slre.num_caps + 2];
		char nbuf[SLRE_PATSZ];
		const char *old;
		char *np;
		int i, olen;

		(void) memset(caps, 0, sizeof(caps));

		res = slre_match(&slre, datap, len, caps);

		debug("Result: %d\n", res);

		for (i = 0; i < slre.num_caps; i++) {
			if (caps[i].len > 0) {
				debug("Substring %d: [%.*s]\n", i,
					caps[i].len, caps[i].ptr);
			}
		}

		if (res == 0) {
			if (loop == 0) {
				printf("%s: No match\n", t);
				return 1;
			} else {
				break;
			}
		}

		debug("## MATCH ## %s\n", data);

		if (s == NULL) {
			printf("%s=%s\n", name, t);
			return 1;
		}

		old = caps[0].ptr;
		olen = caps[0].len;

		if (nlen + 1 >= SLRE_PATSZ) {
			printf("## error: pattern buffer overflow: have %d, need %d\n",
				SLRE_BUFSZ, nlen + 1);
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
					SLRE_PATSZ,
					backref, 2,
					caps[i].ptr, caps[i].len);

				if (np == NULL)
					return 1;
			}
		}
		debug("## SUBST(2) ## %s\n", nbuf);

		datap = substitute(datap, &len, SLRE_BUFSZ,
				old, olen,
				nbuf, nlen);

		if (datap == NULL)
			return 1;

		debug("## REMAINDER: %s\n", datap);

		debug("## RESULT: %s\n", data);

		if (!global)
			break;
	}
	debug("## FINAL (now env_set()) :  %s\n", data);

	printf("%s=%s\n", name, data);

	return env_set(name, data);
}
#endif

static int do_setexpr(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	ulong a, b;
	ulong value;
	int w;

	/*
	 * We take 3, 5, or 6 arguments:
	 * 3 : setexpr name value
	 * 5 : setexpr name val1 op val2
	 *     setexpr name [g]sub r s
	 * 6 : setexpr name [g]sub r s t
	 */

	/* > 6 already tested by max command args */
	if ((argc < 3) || (argc == 4))
		return CMD_RET_USAGE;

	w = cmd_get_data_size(argv[0], 4);

	a = get_arg(argv[2], w);

	/* plain assignment: "setexpr name value" */
	if (argc == 3) {
		env_set_hex(argv[1], a);
		return 0;
	}

	/* 5 or 6 args (6 args only with [g]sub) */
#ifdef CONFIG_REGEX
	/*
	 * rexep handling: "setexpr name [g]sub r s [t]"
	 * with 5 args, "t" will be NULL
	 */
	if (strcmp(argv[2], "gsub") == 0)
		return regex_sub(argv[1], argv[3], argv[4], argv[5], 1);

	if (strcmp(argv[2], "sub") == 0)
		return regex_sub(argv[1], argv[3], argv[4], argv[5], 0);
#endif

	/* standard operators: "setexpr name val1 op val2" */
	if (argc != 5)
		return CMD_RET_USAGE;

	if (strlen(argv[3]) != 1)
		return CMD_RET_USAGE;

	b = get_arg(argv[4], w);

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

	return 0;
}

U_BOOT_CMD(
	setexpr, 6, 0, do_setexpr,
	"set environment variable as the result of eval expression",
	"[.b, .w, .l] name [*]value1 <op> [*]value2\n"
	"    - set environment variable 'name' to the result of the evaluated\n"
	"      expression specified by <op>.  <op> can be &, |, ^, +, -, *, /, %\n"
	"      size argument is only meaningful if value1 and/or value2 are\n"
	"      memory addresses (*)\n"
	"setexpr[.b, .w, .l] name [*]value\n"
	"    - load a value into a variable"
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
