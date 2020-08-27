/*
 * Common utility functions across router code
 *
 * Copyright (C) 2020, Broadcom. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: common_utils.c 473329 2014-04-29 00:37:52Z $
 */

#include <typedefs.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>

#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <assert.h>
#include <fcntl.h>
#include <limits.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ioctl.h>

#include <bcmnvram.h>
#include <shutils.h>
#include "common_utils.h"

char*
strncpy_n(char *destination, const char *source, size_t num)
{
	char *ret = strncpy(destination, source, num - 1);
	destination[num - 1] = '\0';
	return ret;
}

unsigned long long
getTimestamp(void)
{
	unsigned long long ts;
	struct timeval now;
	gettimeofday(&now, NULL);
	ts = (unsigned long long)now.tv_sec * 1000 + now.tv_usec / 1000;
	return ts;
}

void
append_numto_hexStr(char octetstr[], int octetstr_len, int num)
{
	char temp[3] = {0};
	memset(temp, 0, sizeof(temp));
	snprintf(temp, sizeof(temp), OCTET, num);
	strncat(octetstr, temp, min(strlen(temp), octetstr_len - strlen(octetstr)));
}

int
is_duplicate_octet(uint8* haystack, size_t size, uint8 needle)
{
	assert(haystack);
	int iter = 0;

	if (needle == 255 || needle == 0) {
		return FALSE;
	} else {
		for (iter = 0; iter < size; iter++) {
			if (haystack[iter] == needle)
				return TRUE;
		}
	}

	return FALSE;
}

int
is_octet_range_overlapping(uint8* haystack, size_t size, uint8 low_needle, uint8 high_needle)
{
	assert(haystack);
	int iter = 0, low_iter = 0, high_iter = 0;

	if (low_needle == 255 && high_needle == 255) {
		return FALSE;
	} else {
		for (iter = 0; iter < size; iter++) {
			low_iter = haystack[(iter*2)];
			high_iter = haystack[(iter*2)+1];

			if ((low_iter <= high_needle) && (low_needle <= high_iter))
				return TRUE;
		}
	}

	return FALSE;
}

void
bytes_to_hex(uchar* str, int strbuflen, uchar* utf8, int utf8buflen)
{
	char temp[3] = {0};
	uchar *src = str, *dst = utf8;
	int len = strlen((char*)src), i, optlen;
	optlen = len < (utf8buflen-1) ? len : (utf8buflen-1);

	for (i = 0; i < optlen; i++) {
		memset(temp, 0, sizeof(temp));
		snprintf(temp, sizeof(temp), OCTET, (uchar)src[i]);
		*dst++ = temp[0];
		*dst++ = temp[1];
	}
}

void
hex_to_bytes(uchar* str, int strbuflen, uchar* utf8, int utf8buflen)
{
	char temp[3] = {0};
	uchar *src = utf8, *dst = str;
	int len = strlen((char*)src)/2, i, optlen;
	optlen = len < strbuflen ? len : strbuflen;

	for (i = 0; i < optlen; i++) {
		memset(temp, 0, sizeof(temp));
		temp[0] = src[0];
		temp[1] = src[1];
		temp[2] = NULL_CH;
		*dst++ = (uchar) strtoul(temp, NULL, 16);
		src += 2;
	}
}

int
get_hex_data(uchar *data_str, uchar *hex_data, int len)
{
	uchar *src, *dest;
	uchar val;
	int idx;
	char hexstr[3] = {0};

	src = data_str;
	dest = hex_data;

	for (idx = 0; idx < len; idx++) {
		hexstr[0] = src[0];
		hexstr[1] = src[1];
		hexstr[2] = NULL_CH;

		val = (uchar) strtoul(hexstr, NULL, 16);

		*dest++ = val;
		src += 2;
	}

	return 0;
}

int
reallocate_string(char** string, const char* newstring)
{
	int newlength = 0;

	if (newstring)
		newlength = strlen(newstring);

	if (*string)
		free(*string);

	if (newlength <= 0)
		return 0;

	*string = (char*)malloc(newlength+1);
	if (*string == 0) {
		return 0;
	}

	strcpy(*string, newstring);

	return 1;
}

void *
memmem(const void *l, size_t l_len, const void *s, size_t s_len)
{
	register char *cur, *last;
	const char *cl = (const char *)l;
	const char *cs = (const char *)s;

	/* we need something to compare */
	if (l_len == 0 || s_len == 0)
		return NULL;

	/* "s" must be smaller or equal to "l" */
	if (l_len < s_len)
		return NULL;

	/* special case where s_len == 1 */
	if (s_len == 1)
		return memchr(l, (int)*cs, l_len);

	/* the last position where its possible to find "s" in "l" */
	last = (char *)cl + l_len - s_len;

	for (cur = (char *)cl; cur <= last; cur++)
		if (cur[0] == cs[0] && memcmp(cur, cs, s_len) == 0)
			return cur;

	return NULL;
}

/* To pass required information to restart the service in case
 * it is in a bad state or killed. Service restart is handled by
 * debug_monitor service. Arguments to be passed are process id,
 * command count, servce commands and dependent_services.
 */
void
dm_register_app_restart_info(int pid, int argc, char **argv,
		char *dependent_services)
{
	char buf[16];
	FILE *fp;
	DIR *dir;
	int i;
	dir = opendir("/tmp/dm");
	if (!dir) {
		printf("dm directory may not exist (%s). Creating it\n", strerror(errno));
		if (mkdir("/tmp/dm", 0777)) {
			printf("Unable to create dm directory. %s\n", strerror(errno));
			return;
		}
	} else {
		closedir(dir);
	}
	snprintf(buf, sizeof(buf), "/tmp/dm/%d", pid);
	fp = fopen(buf, "w");
	if (!fp) {
		printf("Unable to open file %s. %s\n", buf, strerror(errno));
	} else {
		if (dependent_services) {
			fprintf(fp, "%s\n", dependent_services);
		} else {
			fprintf(fp, "\n");
		}
		for (i = 0; i < argc; i++) {
			fprintf(fp, "%s ", argv[i]);
		}
		fclose(fp);
	}
}

/* To remove information from debug_monitor to restart the service.
 * This API is used when service exits normally, so it does not want
 * debug_monitor to restart the service.
 */
void
dm_unregister_app_restart_info(int pid)
{
	char buf[16];

	snprintf(buf, sizeof(buf), "/tmp/dm/%d", pid);
	if (remove(buf)) {
		printf("Unable to remove file %s. %s\n", buf, strerror(errno));
	}
}
