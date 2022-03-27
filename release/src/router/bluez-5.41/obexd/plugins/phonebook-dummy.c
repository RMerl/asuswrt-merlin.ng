/*
 *
 *  OBEX Server
 *
 *  Copyright (C) 2009-2010  Intel Corporation
 *  Copyright (C) 2007-2010  Marcel Holtmann <marcel@holtmann.org>
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

#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <glib.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <libical/ical.h>
#include <libical/vobject.h>
#include <libical/vcc.h>

#include "obexd/src/log.h"
#include "phonebook.h"

typedef void (*vcard_func_t) (const char *file, VObject *vo, void *user_data);

struct dummy_data {
	phonebook_cb cb;
	void *user_data;
	const struct apparam_field *apparams;
	char *folder;
	int fd;
	guint id;
};

struct cache_query {
	phonebook_entry_cb entry_cb;
	phonebook_cache_ready_cb ready_cb;
	void *user_data;
	DIR *dp;
};

static char *root_folder = NULL;

static void dummy_free(void *user_data)
{
	struct dummy_data *dummy = user_data;

	if (dummy->fd >= 0)
		close(dummy->fd);

	g_free(dummy->folder);
	g_free(dummy);
}

static void query_free(void *user_data)
{
	struct cache_query *query = user_data;

	if (query->dp)
		closedir(query->dp);

	g_free(query);
}

int phonebook_init(void)
{
	if (root_folder)
		return 0;

	/* FIXME: It should NOT be hard-coded */
	root_folder = g_build_filename(getenv("HOME"), "phonebook", NULL);

	return 0;
}

void phonebook_exit(void)
{
	g_free(root_folder);
	root_folder = NULL;
}

static int handle_cmp(gconstpointer a, gconstpointer b)
{
	const char *f1 = a;
	const char *f2 = b;
	unsigned int i1, i2;

	if (sscanf(f1, "%u.vcf", &i1) != 1)
		return -1;

	if (sscanf(f2, "%u.vcf", &i2) != 1)
		return -1;

	return (i1 - i2);
}

static int foreach_vcard(DIR *dp, vcard_func_t func, uint16_t offset,
			uint16_t maxlistcount, void *user_data, uint16_t *count)
{
	struct dirent *ep;
	GSList *sorted = NULL, *l;
	VObject *v;
	FILE *fp;
	int err, fd, folderfd;
	uint16_t n = 0;

	folderfd = dirfd(dp);
	if (folderfd < 0) {
		err = errno;
		error("dirfd(): %s(%d)", strerror(err), err);
		return -err;
	}

	/*
	 * Sorting vcards by file name. versionsort is a GNU extension.
	 * The simple sorting function implemented on handle_cmp address
	 * vcards handle only(handle is always a number). This sort function
	 * doesn't address filename started by "0".
	 */
	while ((ep = readdir(dp))) {
		char *filename;

		if (ep->d_name[0] == '.')
			continue;

		filename = g_filename_to_utf8(ep->d_name, -1, NULL, NULL, NULL);
		if (filename == NULL) {
			error("g_filename_to_utf8: invalid filename");
			continue;
		}

		if (!g_str_has_suffix(filename, ".vcf")) {
			g_free(filename);
			continue;
		}

		sorted = g_slist_insert_sorted(sorted, filename, handle_cmp);
	}

	/*
	 * Filtering only the requested vCards attributes. Offset
	 * shall be based on the first entry of the phonebook.
	 */
	for (l = g_slist_nth(sorted, offset);
			l && n < maxlistcount; l = l->next) {
		const char *filename = l->data;

		fd = openat(folderfd, filename, O_RDONLY);
		if (fd < 0) {
			err = errno;
			error("openat(%s): %s(%d)", filename, strerror(err), err);
			continue;
		}

		fp = fdopen(fd, "r");
		v = Parse_MIME_FromFile(fp);
		if (v != NULL) {
			func(filename, v, user_data);
			deleteVObject(v);
			n++;
		}

		close(fd);
	}

	g_slist_free_full(sorted, g_free);

	if (count)
		*count = n;

	return 0;
}

static void entry_concat(const char *filename, VObject *v, void *user_data)
{
	GString *buffer = user_data;
	char tmp[1024];
	int len;

	/*
	 * VObject API uses len for IN and OUT
	 * Written bytes is also returned in the len variable
	 */
	len = sizeof(tmp);
	memset(tmp, 0, len);

	writeMemVObject(tmp, &len, v);

	/* FIXME: only the requested fields must be added */
	g_string_append_len(buffer, tmp, len);
}

static gboolean read_dir(void *user_data)
{
	struct dummy_data *dummy = user_data;
	GString *buffer;
	DIR *dp;
	uint16_t count = 0, max, offset;

	buffer = g_string_new("");

	dp = opendir(dummy->folder);
	if (dp == NULL) {
		int err = errno;
		DBG("opendir(): %s(%d)", strerror(err), err);
		goto done;
	}

	/*
	 * For PullPhoneBook function, the decision of returning the size
	 * or contacts is made in the PBAP core. When MaxListCount is ZERO,
	 * PCE wants to know the size of a given folder, PSE shall ignore all
	 * other applicattion parameters that may be present in the request.
	 */
	if (dummy->apparams->maxlistcount == 0) {
		max = 0xffff;
		offset = 0;
	} else {
		max = dummy->apparams->maxlistcount;
		offset = dummy->apparams->liststartoffset;
	}

	foreach_vcard(dp, entry_concat, offset, max, buffer, &count);

	closedir(dp);
done:
	/* FIXME: Missing vCards fields filtering */
	dummy->cb(buffer->str, buffer->len, count, 0, TRUE, dummy->user_data);

	g_string_free(buffer, TRUE);

	return FALSE;
}

static void entry_notify(const char *filename, VObject *v, void *user_data)
{
	struct cache_query *query = user_data;
	VObject *property, *subproperty;
	GString *name;
	const char *tel;
	long unsigned int handle;

	property = isAPropertyOf(v, VCNameProp);
	if (!property)
		return;

	if (sscanf(filename, "%lu.vcf", &handle) != 1)
		return;

	if (handle > UINT32_MAX)
		return;

	/* LastName; FirstName; MiddleName; Prefix; Suffix */

	name = g_string_new("");
	subproperty = isAPropertyOf(property, VCFamilyNameProp);
	if (subproperty) {
		g_string_append(name,
				fakeCString(vObjectUStringZValue(subproperty)));
	}

	subproperty = isAPropertyOf(property, VCGivenNameProp);
	if (subproperty)
		g_string_append_printf(name, ";%s",
				fakeCString(vObjectUStringZValue(subproperty)));

	subproperty = isAPropertyOf(property, VCAdditionalNamesProp);
	if (subproperty)
		g_string_append_printf(name, ";%s",
				fakeCString(vObjectUStringZValue(subproperty)));

	subproperty = isAPropertyOf(property, VCNamePrefixesProp);
	if (subproperty)
		g_string_append_printf(name, ";%s",
				fakeCString(vObjectUStringZValue(subproperty)));


	subproperty = isAPropertyOf(property, VCNameSuffixesProp);
	if (subproperty)
		g_string_append_printf(name, ";%s",
				fakeCString(vObjectUStringZValue(subproperty)));

	property = isAPropertyOf(v, VCTelephoneProp);

	tel = property ? fakeCString(vObjectUStringZValue(property)) : NULL;

	query->entry_cb(filename, handle, name->str, NULL, tel,
							query->user_data);
	g_string_free(name, TRUE);
}

static gboolean create_cache(void *user_data)
{
	struct cache_query *query = user_data;

	/*
	 * MaxListCount and ListStartOffset shall not be used
	 * when creating the cache. All entries shall be fetched.
	 * PBAP core is responsible for consider these application
	 * parameters before reply the entries.
	 */
	foreach_vcard(query->dp, entry_notify, 0, 0xffff, query, NULL);

	query->ready_cb(query->user_data);

	return FALSE;
}

static gboolean read_entry(void *user_data)
{
	struct dummy_data *dummy = user_data;
	char buffer[1024];
	ssize_t count;

	memset(buffer, 0, sizeof(buffer));
	count = read(dummy->fd, buffer, sizeof(buffer));

	if (count < 0) {
		int err = errno;
		error("read(): %s(%d)", strerror(err), err);
		count = 0;
	}

	/* FIXME: Missing vCards fields filtering */

	dummy->cb(buffer, count, 1, 0, TRUE, dummy->user_data);

	return FALSE;
}

static gboolean is_dir(const char *dir)
{
	struct stat st;

	if (stat(dir, &st) < 0) {
		int err = errno;
		error("stat(%s): %s (%d)", dir, strerror(err), err);
		return FALSE;
	}

	return S_ISDIR(st.st_mode);
}

char *phonebook_set_folder(const char *current_folder,
		const char *new_folder, uint8_t flags, int *err)
{
	gboolean root, child;
	char *tmp1, *tmp2, *base, *absolute, *relative = NULL;
	int len, ret = 0;

	root = (g_strcmp0("/", current_folder) == 0);
	child = (new_folder && strlen(new_folder) != 0);

	switch (flags) {
	case 0x02:
		/* Go back to root */
		if (!child) {
			relative = g_strdup("/");
			goto done;
		}

		relative = g_build_filename(current_folder, new_folder, NULL);
		break;
	case 0x03:
		/* Go up 1 level */
		if (root) {
			/* Already root */
			ret = -EBADR;
			goto done;
		}

		/*
		 * Removing one level of the current folder. Current folder
		 * contains AT LEAST one level since it is not at root folder.
		 * Use glib utility functions to handle invalid chars in the
		 * folder path properly.
		 */
		tmp1 = g_path_get_basename(current_folder);
		tmp2 = g_strrstr(current_folder, tmp1);
		len = tmp2 - (current_folder + 1);

		g_free(tmp1);

		if (len == 0)
			base = g_strdup("/");
		else
			base = g_strndup(current_folder, len);

		/* Return: one level only */
		if (!child) {
			relative = base;
			goto done;
		}

		relative = g_build_filename(base, new_folder, NULL);
		g_free(base);

		break;
	default:
		ret = -EBADR;
		break;
	}

done:
	if (!relative) {
		if (err)
			*err = ret;

		return NULL;
	}

	absolute = g_build_filename(root_folder, relative, NULL);
	if (!is_dir(absolute)) {
		g_free(relative);
		relative = NULL;
		ret = -ENOENT;
	}

	g_free(absolute);

	if (err)
		*err = ret;

	return relative;
}

void phonebook_req_finalize(void *request)
{
	struct dummy_data *dummy = request;

	/* dummy_data will be cleaned when request will be finished via
	 * g_source_remove */
	if (dummy && dummy->id)
		g_source_remove(dummy->id);
}

void *phonebook_pull(const char *name, const struct apparam_field *params,
				phonebook_cb cb, void *user_data, int *err)
{
	struct dummy_data *dummy;
	char *filename, *folder;

	/*
	 * Main phonebook objects will be created dinamically based on the
	 * folder content. All vcards inside the given folder will be appended
	 * in the "virtual" main phonebook object.
	 */

	filename = g_build_filename(root_folder, name, NULL);

	if (!g_str_has_suffix(filename, ".vcf")) {
		g_free(filename);
		if (err)
			*err = -EBADR;
		return NULL;
	}

	folder = g_strndup(filename, strlen(filename) - 4);
	g_free(filename);
	if (!is_dir(folder)) {
		g_free(folder);
		if (err)
			*err = -ENOENT;
		return NULL;
	}

	dummy = g_new0(struct dummy_data, 1);
	dummy->cb = cb;
	dummy->user_data = user_data;
	dummy->apparams = params;
	dummy->folder = folder;
	dummy->fd = -1;

	if (err)
		*err = 0;

	return dummy;
}

int phonebook_pull_read(void *request)
{
	struct dummy_data *dummy = request;

	if (!dummy)
		return -ENOENT;

	dummy->id = g_idle_add_full(G_PRIORITY_DEFAULT_IDLE, read_dir, dummy,
								dummy_free);

	return 0;
}

void *phonebook_get_entry(const char *folder, const char *id,
			const struct apparam_field *params, phonebook_cb cb,
			void *user_data, int *err)
{
	struct dummy_data *dummy;
	char *filename;
	int fd;

	filename = g_build_filename(root_folder, folder, id, NULL);

	fd = open(filename, O_RDONLY);
	if (fd < 0) {
		DBG("open(): %s(%d)", strerror(errno), errno);
		if (err)
			*err = -ENOENT;
		return NULL;
	}

	dummy = g_new0(struct dummy_data, 1);
	dummy->cb = cb;
	dummy->user_data = user_data;
	dummy->apparams = params;
	dummy->fd = fd;

	dummy->id = g_idle_add_full(G_PRIORITY_DEFAULT_IDLE, read_entry, dummy,
								dummy_free);

	if (err)
		*err = 0;

	return dummy;
}

void *phonebook_create_cache(const char *name, phonebook_entry_cb entry_cb,
		phonebook_cache_ready_cb ready_cb, void *user_data, int *err)
{
	struct cache_query *query;
	char *foldername;
	DIR *dp;
	struct dummy_data *dummy;

	foldername = g_build_filename(root_folder, name, NULL);
	dp = opendir(foldername);
	g_free(foldername);

	if (dp == NULL) {
		DBG("opendir(): %s(%d)", strerror(errno), errno);
		if (err)
			*err = -ENOENT;
		return NULL;
	}

	query = g_new0(struct cache_query, 1);
	query->entry_cb = entry_cb;
	query->ready_cb = ready_cb;
	query->user_data = user_data;
	query->dp = dp;

	dummy = g_new0(struct dummy_data, 1);

	dummy->id = g_idle_add_full(G_PRIORITY_DEFAULT_IDLE, create_cache,
							query, query_free);

	if (err)
		*err = 0;

	return dummy;
}
