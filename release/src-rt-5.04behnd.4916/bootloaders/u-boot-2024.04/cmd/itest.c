// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2003
 * Tait Electronics Limited, Christchurch, New Zealand
 */

/*
 * This file provides a shell like 'test' function to return
 * true/false from an integer or string compare of two memory
 * locations or a location and a scalar/literal.
 * A few parts were lifted from bash 'test' command
 */

#include <common.h>
#include <config.h>
#include <command.h>
#include <env.h>
#include <mapmem.h>

#include <asm/io.h>

#define EQ	0
#define NE	1
#define LT	2
#define GT	3
#define LE	4
#define GE	5

struct op_tbl_s {
	char	*op;		/* operator string */
	int	opcode;		/* internal representation of opcode */
};

typedef struct op_tbl_s op_tbl_t;

static const op_tbl_t op_table [] = {
	{ "-lt", LT },
	{ "<"  , LT },
	{ "-gt", GT },
	{ ">"  , GT },
	{ "-eq", EQ },
	{ "==" , EQ },
	{ "-ne", NE },
	{ "!=" , NE },
	{ "<>" , NE },
	{ "-ge", GE },
	{ ">=" , GE },
	{ "-le", LE },
	{ "<=" , LE },
};

static long evalexp(char *s, int w)
{
	long l = 0;
	unsigned long addr;
	void *buf;

	/* if the parameter starts with a * then assume is a pointer to the value we want */
	if (s[0] == '*') {
		addr = hextoul(&s[1], NULL);
		buf = map_physmem(addr, w, MAP_WRBACK);
		if (!buf && addr) {
			puts("Failed to map physical memory\n");
			return 0;
		}
		switch (w) {
		case 1:
			l = (long)(*(u8 *)buf);
			break;
		case 2:
			l = (long)(*(u16 *)buf);
			break;
		case 4:
			l = (long)(*(u32 *)buf);
			break;
#ifdef CONFIG_PHYS_64BIT
		case 8:
			l = (long)(*(unsigned long *)buf);
			break;
#endif
		}
		unmap_physmem(buf, w);
		return l;
	} else {
		l = hextoul(s, NULL);
	}

	/* avoid overflow on mask calculus */
	return (w >= sizeof(long)) ? l : (l & ((1UL << (w * 8)) - 1));
}

static char * evalstr(char *s)
{
	/* if the parameter starts with a * then assume a string pointer else its a literal */
	if (s[0] == '*') {
		return (char *)hextoul(&s[1], NULL);
	} else if (s[0] == '$') {
		int i = 2;

		if (s[1] != '{')
			return NULL;

		while (s[i] != '}') {
			if (s[i] == 0)
				return NULL;
			i++;
		}
		s[i] = 0;
		return  env_get((const char *)&s[2]);
	} else {
		return s;
	}
}

static int stringcomp(char *s, char *t, int op)
{
	int p;
	char *l, *r;

	l = evalstr(s);
	r = evalstr(t);

	p = strcmp(l, r);
	switch (op) {
	case EQ: return (p == 0);
	case NE: return (p != 0);
	case LT: return (p < 0);
	case GT: return (p > 0);
	case LE: return (p <= 0);
	case GE: return (p >= 0);
	}
	return (0);
}

static int arithcomp (char *s, char *t, int op, int w)
{
	long l, r;

	l = evalexp (s, w);
	r = evalexp (t, w);

	switch (op) {
	case EQ: return (l == r);
	case NE: return (l != r);
	case LT: return (l < r);
	case GT: return (l > r);
	case LE: return (l <= r);
	case GE: return (l >= r);
	}
	return (0);
}

static int binary_test(char *op, char *arg1, char *arg2, int w)
{
	int len, i;
	const op_tbl_t *optp;

	len = strlen(op);

	for (optp = (op_tbl_t *)&op_table, i = 0;
	     i < ARRAY_SIZE(op_table);
	     optp++, i++) {

		if ((strncmp (op, optp->op, len) == 0) && (len == strlen (optp->op))) {
			if (w == 0) {
				return (stringcomp(arg1, arg2, optp->opcode));
			} else {
				return (arithcomp (arg1, arg2, optp->opcode, w));
			}
		}
	}

	printf("Unknown operator '%s'\n", op);
	return 0;	/* op code not found */
}

/* command line interface to the shell test */
static int do_itest(struct cmd_tbl *cmdtp, int flag, int argc,
		    char *const argv[])
{
	int	value, w;

	/* Validate arguments */
	if ((argc != 4))
		return CMD_RET_USAGE;

	/* Check for a data width specification.
	 * Defaults to long (4) if no specification.
	 * Uses -2 as 'width' for .s (string) so as not to upset existing code
	 */
	switch (w = cmd_get_data_size(argv[0], 4)) {
	case 1:
	case 2:
	case 4:
#ifdef CONFIG_PHYS_64BIT
	case 8:
#endif
		value = binary_test (argv[2], argv[1], argv[3], w);
		break;
	case CMD_DATA_SIZE_STR:
		value = binary_test (argv[2], argv[1], argv[3], 0);
		break;
	case CMD_DATA_SIZE_ERR:
	default:
		puts("Invalid data width specifier\n");
		value = 0;
		break;
	}

	return !value;
}

U_BOOT_CMD(
	itest, 4, 0, do_itest,
	"return true/false on integer compare",
#ifdef CONFIG_PHYS_64BIT
	"[.b, .w, .l, .q, .s] [*]value1 <op> [*]value2"
#else
	"[.b, .w, .l, .s] [*]value1 <op> [*]value2"
#endif
);
