// SPDX-License-Identifier: GPL-2.0+
/*
 * This file defines the compilation unit for the new hush shell version.  The
 * actual implementation from upstream BusyBox can be found in
 * `cli_hush_upstream.c` which is included at the end of this file.
 *
 * This "wrapper" technique is used to keep the changes to the upstream version
 * as minmal as possible.  Instead, all defines and redefines necessary are done
 * here, outside the upstream sources.  This will hopefully make upgrades to
 * newer revisions much easier.
 *
 * Copyright (c) 2021, Harald Seiler, DENX Software Engineering, hws@denx.de
 */

#include <env.h>
#include <malloc.h>         /* malloc, free, realloc*/
#include <linux/ctype.h>    /* isalpha, isdigit */
#include <console.h>
#include <bootretry.h>
#include <cli.h>
#include <cli_hush.h>
#include <command.h>        /* find_cmd */
#include <asm/global_data.h>

/*
 * BusyBox Version: UPDATE THIS WHEN PULLING NEW UPSTREAM REVISION!
 */
#define BB_VER			"1.35.0.git7d1c7d833785"

/*
 * Define hush features by the names used upstream.
 */
#define ENABLE_HUSH_INTERACTIVE	1
#define ENABLE_FEATURE_EDITING	1
#define ENABLE_HUSH_IF		1
#define ENABLE_HUSH_LOOPS	1
/* No MMU in U-Boot */
#define BB_MMU			0
#define USE_FOR_NOMMU(...)	__VA_ARGS__
#define USE_FOR_MMU(...)

/*
 * Size-saving "small" ints (arch-dependent)
 */
#if CONFIG_IS_ENABLED(X86) || CONFIG_IS_ENABLED(X86_64) || CONFIG_IS_ENABLED(MIPS)
/* add other arches which benefit from this... */
typedef signed char smallint;
typedef unsigned char smalluint;
#else
/* for arches where byte accesses generate larger code: */
typedef int smallint;
typedef unsigned smalluint;
#endif

/*
 * Alignment defines used by BusyBox.
 */
#define ALIGN1			__attribute__((aligned(1)))
#define ALIGN2			__attribute__((aligned(2)))
#define ALIGN4			__attribute__((aligned(4)))
#define ALIGN8			__attribute__((aligned(8)))
#define ALIGN_PTR		__attribute__((aligned(sizeof(void*))))

/*
 * Miscellaneous compiler/platform defines.
 */
#define FAST_FUNC /* not used in U-Boot */
#define UNUSED_PARAM		__always_unused
#define ALWAYS_INLINE		__always_inline
#define NOINLINE		noinline

/*
 * Defines to provide equivalents to what libc/BusyBox defines.
 */
#define EOF			(-1)
#define EXIT_SUCCESS		0
#define EXIT_FAILURE		1

/*
 * Stubs to provide libc/BusyBox functions based on U-Boot equivalents where it
 * makes sense.
 */
#define utoa			simple_itoa

static void __noreturn xfunc_die(void)
{
	panic("HUSH died!");
}

#define bb_error_msg_and_die(format, ...) do { \
panic("HUSH: " format, __VA_ARGS__); \
} while (0);

#define bb_simple_error_msg_and_die(msg) do { \
panic_str("HUSH: " msg); \
} while (0);

/* fdprintf() is used for debug output. */
static int __maybe_unused fdprintf(int fd, const char *format, ...)
{
	va_list args;
	uint i;

	assert(fd == 2);

	va_start(args, format);
	i = vprintf(format, args);
	va_end(args);

	return i;
}

static void bb_verror_msg(const char *s, va_list p, const char* strerr)
{
	/* TODO: what to do with strerr arg? */
	vprintf(s, p);
}

static void bb_error_msg(const char *s, ...)
{
	va_list p;

	va_start(p, s);
	bb_verror_msg(s, p, NULL);
	va_end(p);
}

static void bb_simple_error_msg(const char *s)
{
	bb_error_msg("%s", s);
}

static void *xmalloc(size_t size)
{
	void *p = NULL;
	if (!(p = malloc(size)))
		panic("out of memory");
	return p;
}

static void *xzalloc(size_t size)
{
	void *p = xmalloc(size);
	memset(p, 0, size);
	return p;
}

static void *xrealloc(void *ptr, size_t size)
{
	void *p = NULL;
	if (!(p = realloc(ptr, size)))
		panic("out of memory");
	return p;
}

static void *xmemdup(const void *s, int n)
{
	return memcpy(xmalloc(n), s, n);
}

#define xstrdup		strdup
#define xstrndup	strndup

static void *mempcpy(void *dest, const void *src, size_t len)
{
	return memcpy(dest, src, len) + len;
}

/* Like strcpy but can copy overlapping strings. */
static void overlapping_strcpy(char *dst, const char *src)
{
	/*
	 * Cheap optimization for dst == src case -
	 * better to have it here than in many callers.
	 */
	if (dst != src) {
		while ((*dst = *src) != '\0') {
			dst++;
			src++;
		}
	}
}

static char* skip_whitespace(const char *s)
{
	/*
	 * In POSIX/C locale (the only locale we care about: do we REALLY want
	 * to allow Unicode whitespace in, say, .conf files? nuts!)
	 * isspace is only these chars: "\t\n\v\f\r" and space.
	 * "\t\n\v\f\r" happen to have ASCII codes 9,10,11,12,13.
	 * Use that.
	 */
	while (*s == ' ' || (unsigned char)(*s - 9) <= (13 - 9))
		s++;

	return (char *) s;
}

static char* skip_non_whitespace(const char *s)
{
	while (*s != '\0' && *s != ' ' && (unsigned char)(*s - 9) > (13 - 9))
		s++;

	return (char *) s;
}

#define is_name(c)	((c) == '_' || isalpha((unsigned char)(c)))
#define is_in_name(c)	((c) == '_' || isalnum((unsigned char)(c)))

static const char* endofname(const char *name)
{
	if (!is_name(*name))
		return name;
	while (*++name) {
		if (!is_in_name(*name))
			break;
	}
	return name;
}

/**
 * list_size() - returns the number of elements in char ** before NULL.
 *
 * Argument must contain NULL to signalize its end.
 *
 * @list The list to count the number of element.
 * @return The number of element in list.
 */
static size_t list_size(char **list)
{
	size_t size;

	for (size = 0; list[size] != NULL; size++);

	return size;
}

static int varcmp(const char *p, const char *q)
{
	int c, d;

	while ((c = *p) == (d = *q)) {
		if (c == '\0' || c == '=')
			goto out;
		p++;
		q++;
	}
	if (c == '=')
		c = '\0';
	if (d == '=')
		d = '\0';
out:
	return c - d;
}

struct in_str;
static int u_boot_cli_readline(struct in_str *i);

struct in_str;
static int u_boot_cli_readline(struct in_str *i);

/*
 * BusyBox globals which are needed for hush.
 */
static uint8_t xfunc_error_retval;

static const char defifsvar[] __aligned(1) = "IFS= \t\n";
#define defifs (defifsvar + 4)

/* This define is used to check if exit command was called. */
#define EXIT_RET_CODE -2

/*
 * This define is used for changes that need be done directly in the upstream
 * sources still. Ideally, its use should be minimized as much as possible.
 */
#define __U_BOOT__

/*
 *
 * +-- Include of the upstream sources --+ *
 * V                                     V
 */
#include "cli_hush_upstream.c"
/*
 * A                                     A
 * +-- Include of the upstream sources --+ *
 *
 */

int u_boot_hush_start_modern(void)
{
	INIT_G();
	return 0;
}

static int u_boot_cli_readline(struct in_str *i)
{
	char *prompt;
	char __maybe_unused *ps_prompt = NULL;

	if (!G.promptmode)
		prompt = CONFIG_SYS_PROMPT;
#ifdef CONFIG_SYS_PROMPT_HUSH_PS2
	else
		prompt = CONFIG_SYS_PROMPT_HUSH_PS2;
#else
	/* TODO: default value? */
	#error "SYS_PROMPT_HUSH_PS2 is not defined!"
#endif

	if (CONFIG_IS_ENABLED(CMDLINE_PS_SUPPORT)) {
		if (!G.promptmode)
			ps_prompt = env_get("PS1");
		else
			ps_prompt = env_get("PS2");

		if (ps_prompt)
			prompt = ps_prompt;
	}

	return cli_readline(prompt);
}
