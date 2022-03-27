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

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <glib.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <inttypes.h>

#include "gobex/gobex.h"
#include "gobex/gobex-apparam.h"

#include "obexd/src/obexd.h"
#include "obexd/src/plugin.h"
#include "obexd/src/log.h"
#include "obexd/src/obex.h"
#include "obexd/src/service.h"
#include "obexd/src/manager.h"
#include "obexd/src/mimetype.h"
#include "phonebook.h"
#include "filesystem.h"

#define PHONEBOOK_TYPE		"x-bt/phonebook"
#define VCARDLISTING_TYPE	"x-bt/vcard-listing"
#define VCARDENTRY_TYPE		"x-bt/vcard"

#define ORDER_TAG		0x01
#define SEARCHVALUE_TAG		0x02
#define SEARCHATTRIB_TAG	0x03
#define MAXLISTCOUNT_TAG	0x04
#define LISTSTARTOFFSET_TAG	0x05
#define FILTER_TAG		0x06
#define FORMAT_TAG		0X07
#define PHONEBOOKSIZE_TAG	0X08
#define NEWMISSEDCALLS_TAG	0X09

struct cache {
	gboolean valid;
	uint32_t index;
	GSList *entries;
};

struct cache_entry {
	uint32_t handle;
	char *id;
	char *name;
	char *sound;
	char *tel;
};

struct pbap_session {
	struct apparam_field *params;
	char *folder;
	uint32_t find_handle;
	struct cache cache;
	struct pbap_object *obj;
};

struct pbap_object {
	GString *buffer;
	GObexApparam *apparam;
	gboolean firstpacket;
	gboolean lastpart;
	struct pbap_session *session;
	void *request;
};

static const uint8_t PBAP_TARGET[TARGET_SIZE] = {
			0x79, 0x61, 0x35, 0xF0,  0xF0, 0xC5, 0x11, 0xD8,
			0x09, 0x66, 0x08, 0x00,  0x20, 0x0C, 0x9A, 0x66  };

typedef int (*cache_entry_find_f) (const struct cache_entry *entry,
			const char *value);

static void cache_entry_free(void *data)
{
	struct cache_entry *entry = data;

	g_free(entry->id);
	g_free(entry->name);
	g_free(entry->sound);
	g_free(entry->tel);
	g_free(entry);
}

static gboolean entry_name_find(const struct cache_entry *entry,
		const char *value)
{
	char *name;
	gboolean ret;

	if (!entry->name)
		return FALSE;

	if (strlen(value) == 0)
		return TRUE;

	name = g_utf8_strdown(entry->name, -1);
	ret = (g_strstr_len(name, -1, value) ? TRUE : FALSE);
	g_free(name);

	return ret;
}

static gboolean entry_sound_find(const struct cache_entry *entry,
		const char *value)
{
	if (!entry->sound)
		return FALSE;

	return (g_strstr_len(entry->sound, -1, value) ? TRUE : FALSE);
}

static gboolean entry_tel_find(const struct cache_entry *entry,
		const char *value)
{
	if (!entry->tel)
		return FALSE;

	return (g_strstr_len(entry->tel, -1, value) ? TRUE : FALSE);
}

static const char *cache_find(struct cache *cache, uint32_t handle)
{
	GSList *l;

	for (l = cache->entries; l; l = l->next) {
		struct cache_entry *entry = l->data;

		if (entry->handle == handle)
			return entry->id;
	}

	return NULL;
}

static void cache_clear(struct cache *cache)
{
	g_slist_free_full(cache->entries, cache_entry_free);
	cache->entries = NULL;
}

static void phonebook_size_result(const char *buffer, size_t bufsize,
					int vcards, int missed,
					gboolean lastpart, void *user_data)
{
	struct pbap_session *pbap = user_data;
	uint16_t phonebooksize;

	if (pbap->obj->request) {
		phonebook_req_finalize(pbap->obj->request);
		pbap->obj->request = NULL;
	}

	if (vcards < 0)
		vcards = 0;

	DBG("vcards %d", vcards);

	phonebooksize = vcards;

	pbap->obj->apparam = g_obex_apparam_set_uint16(NULL, PHONEBOOKSIZE_TAG,
								phonebooksize);

	pbap->obj->firstpacket = TRUE;

	if (missed > 0)	{
		DBG("missed %d", missed);

		pbap->obj->apparam = g_obex_apparam_set_uint16(
							pbap->obj->apparam,
							NEWMISSEDCALLS_TAG,
							missed);
	}

	obex_object_set_io_flags(pbap->obj, G_IO_IN, 0);
}

static void query_result(const char *buffer, size_t bufsize, int vcards,
				int missed, gboolean lastpart, void *user_data)
{
	struct pbap_session *pbap = user_data;

	DBG("");

	if (pbap->obj->request && lastpart) {
		phonebook_req_finalize(pbap->obj->request);
		pbap->obj->request = NULL;
	}

	pbap->obj->lastpart = lastpart;

	if (vcards < 0) {
		obex_object_set_io_flags(pbap->obj, G_IO_ERR, -ENOENT);
		return;
	}

	if (!pbap->obj->buffer)
		pbap->obj->buffer = g_string_new_len(buffer, bufsize);
	else
		pbap->obj->buffer = g_string_append_len(pbap->obj->buffer,
							buffer,	bufsize);

	if (missed > 0)	{
		DBG("missed %d", missed);

		pbap->obj->firstpacket = TRUE;

		pbap->obj->apparam = g_obex_apparam_set_uint16(
							pbap->obj->apparam,
							NEWMISSEDCALLS_TAG,
							missed);
	}

	obex_object_set_io_flags(pbap->obj, G_IO_IN, 0);
}

static void cache_entry_notify(const char *id, uint32_t handle,
					const char *name, const char *sound,
					const char *tel, void *user_data)
{
	struct pbap_session *pbap = user_data;
	struct cache_entry *entry = g_new0(struct cache_entry, 1);
	struct cache *cache = &pbap->cache;

	if (handle != PHONEBOOK_INVALID_HANDLE)
		entry->handle = handle;
	else
		entry->handle = ++pbap->cache.index;

	entry->id = g_strdup(id);
	entry->name = g_strdup(name);
	entry->sound = g_strdup(sound);
	entry->tel = g_strdup(tel);

	cache->entries = g_slist_append(cache->entries, entry);
}

static int alpha_sort(gconstpointer a, gconstpointer b)
{
	const struct cache_entry *e1 = a;
	const struct cache_entry *e2 = b;

	return g_strcmp0(e1->name, e2->name);
}

static int indexed_sort(gconstpointer a, gconstpointer b)
{
	const struct cache_entry *e1 = a;
	const struct cache_entry *e2 = b;

	return (e1->handle - e2->handle);
}

static int phonetical_sort(gconstpointer a, gconstpointer b)
{
	const struct cache_entry *e1 = a;
	const struct cache_entry *e2 = b;

	/* SOUND attribute is optional. Use Indexed sort if not present. */
	if (!e1->sound || !e2->sound)
		return indexed_sort(a, b);

	return g_strcmp0(e1->sound, e2->sound);
}

static GSList *sort_entries(GSList *l, uint8_t order, uint8_t search_attrib,
							const char *value)
{
	GSList *sorted = NULL;
	cache_entry_find_f find;
	GCompareFunc sort;
	char *searchval;

	/*
	 * Default sorter is "Indexed". Some backends doesn't inform the index,
	 * for this case a sequential internal index is assigned.
	 * 0x00 = indexed
	 * 0x01 = alphanumeric
	 * 0x02 = phonetic
	 */
	switch (order) {
	case 0x01:
		sort = alpha_sort;
		break;
	case 0x02:
		sort = phonetical_sort;
		break;
	default:
		sort = indexed_sort;
		break;
	}

	/*
	 * This implementation checks if the given field CONTAINS the
	 * search value(case insensitive). Name is the default field
	 * when the attribute is not provided.
	 */
	switch (search_attrib) {
		/* Number */
		case 1:
			find = entry_tel_find;
			break;
		/* Sound */
		case 2:
			find = entry_sound_find;
			break;
		default:
			find = entry_name_find;
			break;
	}

	searchval = value ? g_utf8_strdown(value, -1) : NULL;
	for (; l; l = l->next) {
		struct cache_entry *entry = l->data;

		if (searchval && !find(entry, (const char *) searchval))
			continue;

		sorted = g_slist_insert_sorted(sorted, entry, sort);
	}

	g_free(searchval);

	return sorted;
}

static int generate_response(void *user_data)
{
	struct pbap_session *pbap = user_data;
	GSList *sorted;
	GSList *l;
	uint16_t max = pbap->params->maxlistcount;

	DBG("");

	if (max == 0) {
		/* Ignore all other parameter and return PhoneBookSize */
		uint16_t size = g_slist_length(pbap->cache.entries);

		pbap->obj->apparam = g_obex_apparam_set_uint16(
							pbap->obj->apparam,
							PHONEBOOKSIZE_TAG,
							size);

		return 0;
	}

	/*
	 * Don't free the sorted list content: this list contains
	 * only the reference for the "real" cache entry.
	 */
	sorted = sort_entries(pbap->cache.entries, pbap->params->order,
				pbap->params->searchattrib,
				(const char *) pbap->params->searchval);

	/* Computing offset considering first entry of the phonebook */
	l = g_slist_nth(sorted, pbap->params->liststartoffset);

	pbap->obj->buffer = g_string_new(VCARD_LISTING_BEGIN);
	for (; l && max; l = l->next, max--) {
		const struct cache_entry *entry = l->data;
		char *escaped_name = g_markup_escape_text(entry->name, -1);

		g_string_append_printf(pbap->obj->buffer,
			VCARD_LISTING_ELEMENT, entry->handle, escaped_name);

		g_free(escaped_name);
	}

	pbap->obj->buffer = g_string_append(pbap->obj->buffer,
							VCARD_LISTING_END);
	g_slist_free(sorted);

	return 0;
}

static void cache_ready_notify(void *user_data)
{
	struct pbap_session *pbap = user_data;

	DBG("");

	phonebook_req_finalize(pbap->obj->request);
	pbap->obj->request = NULL;

	pbap->cache.valid = TRUE;

	generate_response(pbap);
	obex_object_set_io_flags(pbap->obj, G_IO_IN, 0);
}

static void cache_entry_done(void *user_data)
{
	struct pbap_session *pbap = user_data;
	const char *id;
	int ret;

	DBG("");

	pbap->cache.valid = TRUE;

	id = cache_find(&pbap->cache, pbap->find_handle);
	if (id == NULL) {
		DBG("Entry %d not found on cache", pbap->find_handle);
		obex_object_set_io_flags(pbap->obj, G_IO_ERR, -ENOENT);
		return;
	}

	phonebook_req_finalize(pbap->obj->request);
	pbap->obj->request = phonebook_get_entry(pbap->folder, id,
				pbap->params, query_result, pbap, &ret);
	if (ret < 0)
		obex_object_set_io_flags(pbap->obj, G_IO_ERR, ret);
}

static struct apparam_field *parse_aparam(const uint8_t *buffer, uint32_t hlen)
{
	GObexApparam *apparam;
	struct apparam_field *param;

	apparam = g_obex_apparam_decode(buffer, hlen);
	if (apparam == NULL)
		return NULL;

	param = g_new0(struct apparam_field, 1);

	/*
	 * As per spec when client doesn't include MAXLISTCOUNT_TAG then it
	 * should be assume as Maximum value in vcardlisting 65535
	 */
	param->maxlistcount = UINT16_MAX;

	g_obex_apparam_get_uint8(apparam, ORDER_TAG, &param->order);
	g_obex_apparam_get_uint8(apparam, SEARCHATTRIB_TAG,
						&param->searchattrib);
	g_obex_apparam_get_uint8(apparam, FORMAT_TAG, &param->format);
	g_obex_apparam_get_uint16(apparam, MAXLISTCOUNT_TAG,
						&param->maxlistcount);
	g_obex_apparam_get_uint16(apparam, LISTSTARTOFFSET_TAG,
						&param->liststartoffset);
	g_obex_apparam_get_uint64(apparam, FILTER_TAG, &param->filter);
	param->searchval = g_obex_apparam_get_string(apparam, SEARCHVALUE_TAG);

	DBG("o %x sa %x sv %s fil %" G_GINT64_MODIFIER "x for %x max %x off %x",
			param->order, param->searchattrib, param->searchval,
			param->filter, param->format, param->maxlistcount,
			param->liststartoffset);

	g_obex_apparam_free(apparam);

	return param;
}

static void *pbap_connect(struct obex_session *os, int *err)
{
	struct pbap_session *pbap;

	manager_register_session(os);

	pbap = g_new0(struct pbap_session, 1);
	pbap->folder = g_strdup("/");
	pbap->find_handle = PHONEBOOK_INVALID_HANDLE;

	if (err)
		*err = 0;

	return pbap;
}

static int pbap_get(struct obex_session *os, void *user_data)
{
	struct pbap_session *pbap = user_data;
	const char *type = obex_get_type(os);
	const char *name = obex_get_name(os);
	struct apparam_field *params;
	const uint8_t *buffer;
	char *path;
	ssize_t rsize;
	int ret;

	DBG("name %s type %s pbap %p", name, type, pbap);

	if (type == NULL)
		return -EBADR;

	rsize = obex_get_apparam(os, &buffer);
	if (rsize < 0) {
		if (g_ascii_strcasecmp(type, VCARDENTRY_TYPE) != 0)
			return -EBADR;

		rsize = 0;
	}

	params = parse_aparam(buffer, rsize);
	if (params == NULL)
		return -EBADR;

	if (pbap->params) {
		g_free(pbap->params->searchval);
		g_free(pbap->params);
	}

	pbap->params = params;

	if (g_ascii_strcasecmp(type, PHONEBOOK_TYPE) == 0) {
		/* Always contains the absolute path */
		if (g_path_is_absolute(name))
			path = g_strdup(name);
		else
			path = g_build_filename("/", name, NULL);

	} else if (g_ascii_strcasecmp(type, VCARDLISTING_TYPE) == 0) {
		/* Always relative */
		if (!name || strlen(name) == 0)
			/* Current folder */
			path = g_strdup(pbap->folder);
		else
			/* Current folder + relative path */
			path = g_build_filename(pbap->folder, name, NULL);

	} else if (g_ascii_strcasecmp(type, VCARDENTRY_TYPE) == 0) {
		/* File name only */
		path = g_strdup(name);
	} else
		return -EBADR;

	if (path == NULL)
		return -EBADR;

	ret = obex_get_stream_start(os, path);

	g_free(path);

	return ret;
}

static int pbap_setpath(struct obex_session *os, void *user_data)
{
	struct pbap_session *pbap = user_data;
	const char *name;
	const uint8_t *nonhdr;
	char *fullname;
	int err;

	if (obex_get_non_header_data(os, &nonhdr) != 2) {
		error("Set path failed: flag and constants not found!");
		return -EBADMSG;
	}

	name = obex_get_name(os);

	DBG("name %s folder %s nonhdr 0x%x%x", name, pbap->folder,
							nonhdr[0], nonhdr[1]);

	fullname = phonebook_set_folder(pbap->folder, name, nonhdr[0], &err);
	if (err < 0)
		return err;

	g_free(pbap->folder);
	pbap->folder = fullname;

	/*
	 * FIXME: Define a criteria to mark the cache as invalid
	 */
	pbap->cache.valid = FALSE;
	pbap->cache.index = 0;
	cache_clear(&pbap->cache);

	return 0;
}

static void pbap_disconnect(struct obex_session *os, void *user_data)
{
	struct pbap_session *pbap = user_data;

	manager_unregister_session(os);

	if (pbap->obj)
		pbap->obj->session = NULL;

	if (pbap->params) {
		g_free(pbap->params->searchval);
		g_free(pbap->params);
	}

	cache_clear(&pbap->cache);
	g_free(pbap->folder);
	g_free(pbap);
}

static int pbap_chkput(struct obex_session *os, void *user_data)
{
	/* Rejects all PUTs */
	return -EBADR;
}

static struct obex_service_driver pbap = {
	.name = "Phonebook Access server",
	.service = OBEX_PBAP,
	.target = PBAP_TARGET,
	.target_size = TARGET_SIZE,
	.connect = pbap_connect,
	.get = pbap_get,
	.setpath = pbap_setpath,
	.disconnect = pbap_disconnect,
	.chkput = pbap_chkput
};

static struct pbap_object *vobject_create(struct pbap_session *pbap,
								void *request)
{
	struct pbap_object *obj;

	obj = g_new0(struct pbap_object, 1);
	obj->session = pbap;
	pbap->obj = obj;
	obj->request = request;

	return obj;
}

static void *vobject_pull_open(const char *name, int oflag, mode_t mode,
				void *context, size_t *size, int *err)
{
	struct pbap_session *pbap = context;
	phonebook_cb cb;
	int ret;
	void *request;

	DBG("name %s context %p maxlistcount %d", name, context,
						pbap->params->maxlistcount);

	if (oflag != O_RDONLY) {
		ret = -EPERM;
		goto fail;
	}

	if (name == NULL) {
		ret = -EBADR;
		goto fail;
	}

	if (pbap->params->maxlistcount == 0)
		cb = phonebook_size_result;
	else
		cb = query_result;

	request = phonebook_pull(name, pbap->params, cb, pbap, &ret);

	if (ret < 0)
		goto fail;

	/* reading first part of results from backend */
	ret = phonebook_pull_read(request);
	if (ret < 0)
		goto fail;

	if (err)
		*err = 0;

	return vobject_create(pbap, request);

fail:
	if (err)
		*err = ret;

	return NULL;
}

static int vobject_close(void *object)
{
	struct pbap_object *obj = object;

	DBG("");

	if (obj->session)
		obj->session->obj = NULL;

	if (obj->buffer)
		g_string_free(obj->buffer, TRUE);

	if (obj->apparam)
		g_obex_apparam_free(obj->apparam);

	if (obj->request)
		phonebook_req_finalize(obj->request);

	g_free(obj);

	return 0;
}

static void *vobject_list_open(const char *name, int oflag, mode_t mode,
				void *context, size_t *size, int *err)
{
	struct pbap_session *pbap = context;
	struct pbap_object *obj = NULL;
	int ret;
	void *request;

	if (name == NULL) {
		ret = -EBADR;
		goto fail;
	}

	DBG("name %s context %p valid %d", name, context, pbap->cache.valid);

	if (oflag != O_RDONLY) {
		ret = -EPERM;
		goto fail;
	}

	/* PullvCardListing always get the contacts from the cache */

	if (pbap->cache.valid) {
		obj = vobject_create(pbap, NULL);
		ret = generate_response(pbap);
	} else {
		request = phonebook_create_cache(name, cache_entry_notify,
					cache_ready_notify, pbap, &ret);
		if (ret == 0)
			obj = vobject_create(pbap, request);
	}
	if (ret < 0)
		goto fail;

	if (err)
		*err = 0;

	return obj;

fail:
	if (obj)
		vobject_close(obj);

	if (err)
		*err = ret;

	return NULL;
}

static void *vobject_vcard_open(const char *name, int oflag, mode_t mode,
					void *context, size_t *size, int *err)
{
	struct pbap_session *pbap = context;
	const char *id;
	uint32_t handle;
	int ret;
	void *request;

	DBG("name %s context %p valid %d", name, context, pbap->cache.valid);

	if (oflag != O_RDONLY) {
		ret = -EPERM;
		goto fail;
	}

	if (name == NULL || sscanf(name, "%u.vcf", &handle) != 1) {
		ret = -EBADR;
		goto fail;
	}

	if (pbap->cache.valid == FALSE) {
		pbap->find_handle = handle;
		request = phonebook_create_cache(pbap->folder,
			cache_entry_notify, cache_entry_done, pbap, &ret);
		goto done;
	}

	id = cache_find(&pbap->cache, handle);
	if (!id) {
		ret = -ENOENT;
		goto fail;
	}

	request = phonebook_get_entry(pbap->folder, id, pbap->params,
						query_result, pbap, &ret);

done:
	if (ret < 0)
		goto fail;

	if (err)
		*err = 0;

	return vobject_create(pbap, request);

fail:
	if (err)
		*err = ret;

	return NULL;
}

static ssize_t vobject_pull_get_next_header(void *object, void *buf, size_t mtu,
								uint8_t *hi)
{
	struct pbap_object *obj = object;

	if (!obj->buffer && !obj->apparam)
		return -EAGAIN;

	*hi = G_OBEX_HDR_APPARAM;

	if (obj->firstpacket) {
		obj->firstpacket = FALSE;

		return g_obex_apparam_encode(obj->apparam, buf, mtu);
	}

	return 0;
}

static ssize_t vobject_pull_read(void *object, void *buf, size_t count)
{
	struct pbap_object *obj = object;
	struct pbap_session *pbap = obj->session;
	int len, ret;

	DBG("buffer %p maxlistcount %d", obj->buffer,
						pbap->params->maxlistcount);

	if (!obj->buffer) {
		if (pbap->params->maxlistcount == 0)
			return -ENOSTR;

		return -EAGAIN;
	}

	len = string_read(obj->buffer, buf, count);
	if (len == 0 && !obj->lastpart) {
		/* in case when buffer is empty and we know that more
		 * data is still available in backend, requesting new
		 * data part via phonebook_pull_read and returning
		 * -EAGAIN to suspend request for now */
		ret = phonebook_pull_read(obj->request);
		if (ret)
			return -EPERM;

		return -EAGAIN;
	}

	return len;
}

static ssize_t vobject_list_get_next_header(void *object, void *buf, size_t mtu,
								uint8_t *hi)
{
	struct pbap_object *obj = object;
	struct pbap_session *pbap = obj->session;

	/* Backend still busy reading contacts */
	if (!pbap->cache.valid)
		return -EAGAIN;

	*hi = G_OBEX_HDR_APPARAM;

	if (pbap->params->maxlistcount == 0)
		return g_obex_apparam_encode(obj->apparam, buf, mtu);

	return 0;
}

static ssize_t vobject_list_read(void *object, void *buf, size_t count)
{
	struct pbap_object *obj = object;
	struct pbap_session *pbap = obj->session;

	DBG("valid %d maxlistcount %d", pbap->cache.valid,
						pbap->params->maxlistcount);

	if (pbap->params->maxlistcount == 0)
		return -ENOSTR;

	return string_read(obj->buffer, buf, count);
}

static ssize_t vobject_vcard_read(void *object, void *buf, size_t count)
{
	struct pbap_object *obj = object;

	DBG("buffer %p", obj->buffer);

	if (!obj->buffer)
		return -EAGAIN;

	return string_read(obj->buffer, buf, count);
}

static struct obex_mime_type_driver mime_pull = {
	.target = PBAP_TARGET,
	.target_size = TARGET_SIZE,
	.mimetype = "x-bt/phonebook",
	.open = vobject_pull_open,
	.close = vobject_close,
	.read = vobject_pull_read,
	.get_next_header = vobject_pull_get_next_header,
};

static struct obex_mime_type_driver mime_list = {
	.target = PBAP_TARGET,
	.target_size = TARGET_SIZE,
	.mimetype = "x-bt/vcard-listing",
	.open = vobject_list_open,
	.close = vobject_close,
	.read = vobject_list_read,
	.get_next_header = vobject_list_get_next_header,
};

static struct obex_mime_type_driver mime_vcard = {
	.target = PBAP_TARGET,
	.target_size = TARGET_SIZE,
	.mimetype = "x-bt/vcard",
	.open = vobject_vcard_open,
	.close = vobject_close,
	.read = vobject_vcard_read,
};

static int pbap_init(void)
{
	int err;

	err = phonebook_init();
	if (err < 0)
		return err;

	err = obex_mime_type_driver_register(&mime_pull);
	if (err < 0)
		goto fail_mime_pull;

	err = obex_mime_type_driver_register(&mime_list);
	if (err < 0)
		goto fail_mime_list;

	err = obex_mime_type_driver_register(&mime_vcard);
	if (err < 0)
		goto fail_mime_vcard;

	err = obex_service_driver_register(&pbap);
	if (err < 0)
		goto fail_pbap_reg;

	return 0;

fail_pbap_reg:
	obex_mime_type_driver_unregister(&mime_vcard);
fail_mime_vcard:
	obex_mime_type_driver_unregister(&mime_list);
fail_mime_list:
	obex_mime_type_driver_unregister(&mime_pull);
fail_mime_pull:
	phonebook_exit();

	return err;
}

static void pbap_exit(void)
{
	obex_service_driver_unregister(&pbap);
	obex_mime_type_driver_unregister(&mime_pull);
	obex_mime_type_driver_unregister(&mime_list);
	obex_mime_type_driver_unregister(&mime_vcard);
	phonebook_exit();
}

OBEX_PLUGIN_DEFINE(pbap, pbap_init, pbap_exit)
