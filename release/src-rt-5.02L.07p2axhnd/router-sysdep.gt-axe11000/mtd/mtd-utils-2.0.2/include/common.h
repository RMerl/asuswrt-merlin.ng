/*
 * Copyright (c) Artem Bityutskiy, 2007, 2008
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __MTD_UTILS_COMMON_H__
#define __MTD_UTILS_COMMON_H__

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <features.h>
#include <inttypes.h>
#include <unistd.h>
#include <sys/sysmacros.h>

#include "config.h"

#ifndef PROGRAM_NAME
# error "You must define PROGRAM_NAME before including this header"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MIN	/* some C lib headers define this for us */
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif
#define min(a, b) MIN(a, b) /* glue for linux kernel source */
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

#define ALIGN(x,a) __ALIGN_MASK(x,(typeof(x))(a)-1)
#define __ALIGN_MASK(x,mask) (((x)+(mask))&~(mask))

#define min_t(t,x,y) ({ \
	typeof((x)) _x = (x); \
	typeof((y)) _y = (y); \
	(_x < _y) ? _x : _y; \
})

#define max_t(t,x,y) ({ \
	typeof((x)) _x = (x); \
	typeof((y)) _y = (y); \
	(_x > _y) ? _x : _y; \
})

#ifndef O_CLOEXEC
#define O_CLOEXEC 0
#endif

/* Verbose messages */
#define bareverbose(verbose, fmt, ...) do {                        \
	if (verbose)                                               \
		printf(fmt, ##__VA_ARGS__);                        \
} while(0)
#define verbose(verbose, fmt, ...) \
	bareverbose(verbose, "%s: " fmt "\n", PROGRAM_NAME, ##__VA_ARGS__)

/* Normal messages */
#define normsg_cont(fmt, ...) do {                                 \
	printf("%s: " fmt, PROGRAM_NAME, ##__VA_ARGS__);           \
} while(0)
#define normsg(fmt, ...) do {                                      \
	normsg_cont(fmt "\n", ##__VA_ARGS__);                      \
} while(0)

/* Error messages */
#define errmsg(fmt, ...)  ({                                                \
	fprintf(stderr, "%s: error!: " fmt "\n", PROGRAM_NAME, ##__VA_ARGS__); \
	-1;                                                                 \
})
#define errmsg_die(fmt, ...) do {                                           \
	exit(errmsg(fmt, ##__VA_ARGS__));                                   \
} while(0)

/* System error messages */
#define sys_errmsg(fmt, ...)  ({                                            \
	int _err = errno;                                                   \
	errmsg(fmt, ##__VA_ARGS__);                                         \
	fprintf(stderr, "%*serror %d (%s)\n", (int)sizeof(PROGRAM_NAME) + 1,\
		"", _err, strerror(_err));                                  \
	-1;                                                                 \
})
#define sys_errmsg_die(fmt, ...) do {                                       \
	exit(sys_errmsg(fmt, ##__VA_ARGS__));                               \
} while(0)

/* Warnings */
#define warnmsg(fmt, ...) do {                                                \
	fprintf(stderr, "%s: warning!: " fmt "\n", PROGRAM_NAME, ##__VA_ARGS__); \
} while(0)

/* for tagging functions that always exit */
#if defined(__GNUC__) || defined(__clang__)
	#define NORETURN __attribute__((noreturn))
#else
	#define NORETURN
#endif

/**
 * prompt the user for confirmation
 */
static inline bool prompt(const char *msg, bool def)
{
	bool ret = def;
	char line[64];

	do {
		normsg_cont("%s (%c/%c) ", msg, def ? 'Y' : 'y', def ? 'n' : 'N');
		fflush(stdout);

		if (fgets(line, sizeof(line), stdin) == NULL) {
			printf("failed to read prompt; assuming '%s'\n",
				def ? "yes" : "no");
			break;
		}

		if (strcmp("\n", line) != 0) {
			switch (line[0]) {
			case 'N':
			case 'n': ret = false; break;
			case 'Y':
			case 'y': ret = true; break;
			default:
				puts("unknown response; please try again");
				continue;
			}
		}
		break;
	} while (1);

	return ret;
}

static inline int is_power_of_2(unsigned long long n)
{
	return (n != 0 && ((n & (n - 1)) == 0));
}

/* Check whether buffer is filled with character 'pattern' */
static inline int buffer_check_pattern(unsigned char *buffer, size_t size,
				       unsigned char pattern)
{
	/* Invalid input */
	if (!buffer || (size == 0))
		return 0;

	/* No match on first byte */
	if (*buffer != pattern)
		return 0;

	/* First byte matched and buffer is 1 byte long, OK. */
	if (size == 1)
		return 1;

	/*
	 * Check buffer longer than 1 byte. We already know that buffer[0]
	 * matches the pattern, so the test below only checks whether the
	 * buffer[0...size-2] == buffer[1...size-1] , which is a test for
	 * whether the buffer is filled with constant value.
	 */
	return !memcmp(buffer, buffer + 1, size - 1);
}

/**
 * simple_strtoX - convert a hex/dec/oct string into a number
 * @snum: buffer to convert
 * @error: set to 1 when buffer isn't fully consumed
 *
 * These functions are similar to the standard strtoX() functions, but they are
 * a little bit easier to use if you want to convert full string of digits into
 * the binary form. The typical usage:
 *
 * int error = 0;
 * unsigned long num;
 *
 * num = simple_strtoul(str, &error);
 * if (error || ... if needed, your check that num is not out of range ...)
 * 	error_happened();
 */
#define simple_strtoX(func, type) \
static inline type simple_##func(const char *snum, int *error) \
{ \
	char *endptr; \
	type ret = func(snum, &endptr, 0); \
 \
	if (error && (!*snum || *endptr)) { \
		errmsg("%s: unable to parse the number '%s'", #func, snum); \
		*error = 1; \
	} \
 \
	return ret; \
}
simple_strtoX(strtol, long int)
simple_strtoX(strtoll, long long int)
simple_strtoX(strtoul, unsigned long int)
simple_strtoX(strtoull, unsigned long long int)

/* Simple version-printing for utils */
#define common_print_version() \
do { \
	printf("%s (mtd-utils) %s\n", PROGRAM_NAME, VERSION); \
} while (0)

#include "xalloc.h"

long long util_get_bytes(const char *str);
void util_print_bytes(long long bytes, int bracket);
int util_srand(void);

/*
 * The following helpers are here to avoid compiler complaints about unchecked
 * return code.
 * FIXME: The proper fix would be to check the return code in all those places,
 * but it's usually placed in old code which have no proper exit path and
 * handling  errors requires rewriting a lot of code.
 *
 * WARNING: Please do not use these helpers in new code. Instead, make sure
 * you check the function return code and provide coherent error handling in
 * case of error.
 */
static inline ssize_t read_nocheck(int fd, void *buf, size_t count)
{
	return read(fd, buf, count);
}

static inline ssize_t write_nocheck(int fd, void *buf, size_t count)
{
	return write(fd, buf, count);
}

static inline ssize_t pread_nocheck(int fd, void *buf, size_t count,
				    off_t offset)
{
	return pread(fd, buf, count, offset);
}

static inline ssize_t pwrite_nocheck(int fd, void *buf, size_t count,
				     off_t offset)
{
	return pwrite(fd, buf, count, offset);
}
#ifdef __cplusplus
}
#endif

#endif /* !__MTD_UTILS_COMMON_H__ */
