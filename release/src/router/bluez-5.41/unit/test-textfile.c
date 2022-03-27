/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2004-2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>

#include "src/textfile.h"
#include "src/shared/tester.h"

static const char test_pathname[] = "/tmp/textfile";

static void util_create_empty(void)
{
	int fd;

	fd = creat(test_pathname, 0644);
	if (fd < 0)
		return;

	if (ftruncate(fd, 0) < 0)
		goto done;

done:
	close(fd);
}

static void util_create_pagesize(void)
{
	char value[512];
	unsigned int i;
	int fd, size;

	size = getpagesize();
	if (size < 0)
		return;

	fd = creat(test_pathname, 0644);
	if (fd < 0)
		return;

	if (ftruncate(fd, 0) < 0)
		goto done;

	memset(value, 0, sizeof(value));
	for (i = 0; i < (size / sizeof(value)); i++) {
		if (write(fd, value, sizeof(value)) < 0)
			break;
	}

done:
	close(fd);
}

static void test_pagesize(const void *data)
{
	char key[18], *str;
	int size;

	size = getpagesize();
	g_assert(size >= 4096);

	tester_debug("System uses a page size of %d bytes\n", size);

	util_create_pagesize();

	sprintf(key, "11:11:11:11:11:11");
	str = textfile_get(test_pathname, key);

	tester_debug("%s\n", str);

	g_assert(str == NULL);
	tester_test_passed();
}

static void test_delete(const void *data)
{
	char key[18], value[512], *str;

	util_create_empty();

	sprintf(key, "00:00:00:00:00:00");
	g_assert(textfile_del(test_pathname, key) == 0);

	memset(value, 0, sizeof(value));
	g_assert(textfile_put(test_pathname, key, value) == 0);

	str = textfile_get(test_pathname, key);
	g_assert(str != NULL);

	tester_debug("%s\n", str);

	g_free(str);
	tester_test_passed();
}

static void test_overwrite(const void *data)
{
	char key[18], value[512], *str;

	util_create_empty();

	sprintf(key, "00:00:00:00:00:00");
	memset(value, 0, sizeof(value));
	g_assert(textfile_put(test_pathname, key, value) == 0);

	snprintf(value, sizeof(value), "Test");
	g_assert(textfile_put(test_pathname, key, value) == 0);

	g_assert(textfile_put(test_pathname, key, value) == 0);

	g_assert(textfile_put(test_pathname, key, value) == 0);

	g_assert(textfile_del(test_pathname, key) == 0);

	str = textfile_get(test_pathname, key);

	tester_debug("%s\n", str);

	g_assert(str == NULL);
	tester_test_passed();
}

static void check_entry(char *key, char *value, void *data)
{
	unsigned int max = GPOINTER_TO_UINT(data);
	unsigned int len;

	len = strtol(key + 16, NULL, 16);
	if (len == 1)
		len = max;

	if (g_test_verbose())
		g_print("%s %s\n", key, value);

	g_assert(strlen(value) == len);
}

static void test_multiple(const void *data)
{
	char key[18], value[512], *str;
	unsigned int i, j, max = 10;

	util_create_empty();

	for (i = 1; i < max + 1; i++) {
		sprintf(key, "00:00:00:00:00:%02X", i);

		memset(value, 0, sizeof(value));
		for (j = 0; j < i; j++)
			value[j] = 'x';

		g_assert(textfile_put(test_pathname, key, value) == 0);

		str = textfile_get(test_pathname, key);

		tester_debug("%s %s\n", key, str);

		g_assert(str != NULL);
		g_assert(strcmp(str, value) == 0);

		free(str);
	}

	sprintf(key, "00:00:00:00:00:%02X", max);

	memset(value, 0, sizeof(value));
	for (j = 0; j < max; j++)
		value[j] = 'y';

	g_assert(textfile_put(test_pathname, key, value) == 0);

	str = textfile_get(test_pathname, key);

	tester_debug("%s %s\n", key, str);

	g_assert(str != NULL);
	g_assert(strcmp(str, value) == 0);

	free(str);

	sprintf(key, "00:00:00:00:00:%02X", 1);

	memset(value, 0, sizeof(value));
	for (j = 0; j < max; j++)
		value[j] = 'z';

	g_assert(textfile_put(test_pathname, key, value) == 0);

	str = textfile_get(test_pathname, key);

	tester_debug("%s %s\n", key, str);

	g_assert(str != NULL);
	g_assert(strcmp(str, value) == 0);

	free(str);

	for (i = 1; i < max + 1; i++) {
		sprintf(key, "00:00:00:00:00:%02X", i);
		str = textfile_get(test_pathname, key);

		tester_debug("%s %s\n", key, str);

		g_assert(str != NULL);

		if (i == 1)
			g_assert(strlen(str) == max);
		else
			g_assert(strlen(str) == i);

		g_free(str);
	}

	sprintf(key, "00:00:00:00:00:%02X", 2);
	g_assert(textfile_del(test_pathname, key) == 0);

	sprintf(key, "00:00:00:00:00:%02X", max - 3);
	g_assert(textfile_del(test_pathname, key) == 0);

	textfile_foreach(test_pathname, check_entry, GUINT_TO_POINTER(max));

	sprintf(key, "00:00:00:00:00:%02X", 1);
	g_assert(textfile_del(test_pathname, key) == 0);

	sprintf(key, "00:00:00:00:00:%02X", max);
	g_assert(textfile_del(test_pathname, key) == 0);

	sprintf(key, "00:00:00:00:00:%02X", max + 1);
	g_assert(textfile_del(test_pathname, key) == 0);

	textfile_foreach(test_pathname, check_entry, GUINT_TO_POINTER(max));
	tester_test_passed();
}

int main(int argc, char *argv[])
{
	tester_init(&argc, &argv);

	tester_add("/textfile/pagesize", NULL, NULL, test_pagesize, NULL);
	tester_add("/textfile/delete", NULL, NULL, test_delete, NULL);
	tester_add("/textfile/overwrite", NULL, NULL, test_overwrite, NULL);
	tester_add("/textfile/multiple", NULL, NULL, test_multiple, NULL);

	return tester_run();
}
