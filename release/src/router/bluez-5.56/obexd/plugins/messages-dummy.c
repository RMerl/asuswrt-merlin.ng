// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *
 *  OBEX Server
 *
 *  Copyright (C) 2010-2011  Nokia Corporation
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "obexd/src/log.h"

#include "messages.h"

#define MSG_LIST_XML "mlisting.xml"

static char *root_folder = NULL;

struct session {
	char *cwd;
	char *cwd_absolute;
	void *request;
};

struct folder_listing_data {
	struct session *session;
	const char *name;
	uint16_t max;
	uint16_t offset;
	messages_folder_listing_cb callback;
	void *user_data;
};

struct message_listing_data {
	struct session *session;
	const char *name;
	uint16_t max;
	uint16_t offset;
	uint8_t subject_len;
	uint16_t size;
	char *path;
	FILE *fp;
	const struct messages_filter *filter;
	messages_get_messages_listing_cb callback;
	void *user_data;
};

/* NOTE: Neither IrOBEX nor MAP specs says that folder listing needs to
 * be sorted (in IrOBEX examples it is not). However existing implementations
 * seem to follow the fig. 3-2 from MAP specification v1.0, and I've seen a
 * test suite requiring folder listing to be in that order.
 */
static int folder_names_cmp(gconstpointer a, gconstpointer b,
						gpointer user_data)
{
	static const char *order[] = {
		"inbox", "outbox", "sent", "deleted", "draft", NULL
	};
	struct session *session = user_data;
	int ia, ib;

	if (g_strcmp0(session->cwd, "telecom/msg") == 0) {
		for (ia = 0; order[ia]; ia++) {
			if (g_strcmp0(a, order[ia]) == 0)
				break;
		}
		for (ib = 0; order[ib]; ib++) {
			if (g_strcmp0(b, order[ib]) == 0)
				break;
		}
		if (ia != ib)
			return ia - ib;
	}

	return g_strcmp0(a, b);
}

static char *get_next_subdir(DIR *dp, char *path)
{
	struct dirent *ep;
	char *abs, *name;

	for (;;) {
		if ((ep = readdir(dp)) == NULL)
			return NULL;

		if (strcmp(ep->d_name, ".") == 0 || strcmp(ep->d_name, "..") == 0)
			continue;

		abs = g_build_filename(path, ep->d_name, NULL);

		if (g_file_test(abs, G_FILE_TEST_IS_DIR)) {
			g_free(abs);
			break;
		}

		g_free(abs);
	}

	name = g_filename_to_utf8(ep->d_name, -1, NULL, NULL, NULL);

	if (name == NULL) {
		DBG("g_filename_to_utf8(): invalid filename");
		return NULL;
	}

	return name;
}

static ssize_t get_subdirs(struct folder_listing_data *fld, GSList **list)
{
	DIR *dp;
	char *path, *name;
	size_t n;

	path = g_build_filename(fld->session->cwd_absolute, fld->name, NULL);
	dp = opendir(path);

	if (dp == NULL) {
		int err = -errno;

		DBG("opendir(): %d, %s", -err, strerror(-err));
		g_free(path);

		return err;
	}

	n = 0;

	while ((name = get_next_subdir(dp, path)) != NULL) {
		n++;
		if (fld->max > 0)
			*list = g_slist_prepend(*list, name);
	}

	closedir(dp);
	g_free(path);

	*list = g_slist_sort_with_data(*list, folder_names_cmp, fld->session);

	return n;
}

static void return_folder_listing(struct folder_listing_data *fld, GSList *list)
{
	struct session *session = fld->session;
	GSList *cur;
	uint16_t num = 0;
	uint16_t offs = 0;

	for (cur = list; offs < fld->offset; offs++) {
		cur = cur->next;
		if (cur == NULL)
			break;
	}

	for (; cur != NULL && num < fld->max; cur = cur->next, num++)
		fld->callback(session, -EAGAIN, 0, cur->data, fld->user_data);

	fld->callback(session, 0, 0, NULL, fld->user_data);
}

static gboolean get_folder_listing(void *d)
{
	struct folder_listing_data *fld = d;
	ssize_t n;
	GSList *list = NULL;

	n = get_subdirs(fld, &list);

	if (n < 0) {
		fld->callback(fld->session, n, 0, NULL, fld->user_data);
		return FALSE;
	}

	if (fld->max == 0) {
		fld->callback(fld->session, 0, n, NULL, fld->user_data);
		return FALSE;
	}

	return_folder_listing(fld, list);
	g_slist_free_full(list, g_free);

	return FALSE;
}

int messages_init(void)
{
	char *tmp;

	if (root_folder)
		return 0;

	tmp = getenv("MAP_ROOT");
	if (tmp) {
		root_folder = g_strdup(tmp);
		return 0;
	}

	tmp = getenv("HOME");
	if (!tmp)
		return -ENOENT;

	root_folder = g_build_filename(tmp, "map-messages", NULL);

	return 0;
}

void messages_exit(void)
{
	g_free(root_folder);
	root_folder = NULL;
}

int messages_connect(void **s)
{
	struct session *session;

	session = g_new0(struct session, 1);
	session->cwd = g_strdup("");
	session->cwd_absolute = g_strdup(root_folder);

	*s = session;

	return 0;
}

void messages_disconnect(void *s)
{
	struct session *session = s;

	g_free(session->cwd);
	g_free(session->cwd_absolute);
	g_free(session);
}

int messages_set_notification_registration(void *session,
		void (*send_event)(void *session,
			const struct messages_event *event, void *user_data),
		void *user_data)
{
	return -ENOSYS;
}

int messages_set_folder(void *s, const char *name, gboolean cdup)
{
	struct session *session = s;
	char *newrel = NULL;
	char *newabs;
	char *tmp;

	if (name && (strchr(name, '/') || strcmp(name, "..") == 0))
		return -EBADR;

	if (cdup) {
		if (session->cwd[0] == 0)
			return -ENOENT;

		newrel = g_path_get_dirname(session->cwd);

		/* We use empty string for indication of the root directory */
		if (newrel[0] == '.' && newrel[1] == 0)
			newrel[0] = 0;
	}

	tmp = newrel;
	if (!cdup && (!name || name[0] == 0))
		newrel = g_strdup("");
	else
		newrel = g_build_filename(newrel ? newrel : session->cwd, name,
				NULL);
	g_free(tmp);

	newabs = g_build_filename(root_folder, newrel, NULL);

	if (!g_file_test(newabs, G_FILE_TEST_IS_DIR)) {
		g_free(newrel);
		g_free(newabs);
		return -ENOENT;
	}

	g_free(session->cwd);
	session->cwd = newrel;

	g_free(session->cwd_absolute);
	session->cwd_absolute = newabs;

	return 0;
}

int messages_get_folder_listing(void *s, const char *name, uint16_t max,
					uint16_t offset,
					messages_folder_listing_cb callback,
					void *user_data)
{
	struct session *session =  s;
	struct folder_listing_data *fld;

	fld = g_new0(struct folder_listing_data, 1);
	fld->session = session;
	fld->name = name;
	fld->max = max;
	fld->offset = offset;
	fld->callback = callback;
	fld->user_data = user_data;

	session->request = fld;

	g_idle_add_full(G_PRIORITY_DEFAULT_IDLE, get_folder_listing,
								fld, g_free);

	return 0;
}

static void max_msg_element(GMarkupParseContext *ctxt, const char *element,
				const char **names, const char **values,
				gpointer user_data, GError **gerr)
{
	struct message_listing_data *mld = user_data;
	const char *key;
	int i;

	for (i = 0, key = names[i]; key; key = names[++i]) {
		if (g_strcmp0(names[i], "handle") == 0) {
			mld->size++;
			break;
		}
	}
}

static void msg_element(GMarkupParseContext *ctxt, const char *element,
				const char **names, const char **values,
				gpointer user_data, GError **gerr)
{
	struct message_listing_data *mld = user_data;
	struct messages_message *entry = NULL;
	int i;

	entry = g_new0(struct messages_message, 1);
	if (mld->filter->parameter_mask == 0) {
		entry->mask = (entry->mask | PMASK_SUBJECT \
			| PMASK_DATETIME | PMASK_RECIPIENT_ADDRESSING \
			| PMASK_SENDER_ADDRESSING \
			| PMASK_ATTACHMENT_SIZE | PMASK_TYPE \
			| PMASK_RECEPTION_STATUS);
	} else
		entry->mask = mld->filter->parameter_mask;

	for (i = 0 ; names[i]; ++i) {
		if (g_strcmp0(names[i], "handle") == 0) {
			entry->handle = g_strdup(values[i]);
			mld->size++;
			continue;
		}
		if (g_strcmp0(names[i], "attachment_size") == 0) {
			entry->attachment_size = g_strdup(values[i]);
			continue;
		}
		if (g_strcmp0(names[i], "datetime") == 0) {
			entry->datetime = g_strdup(values[i]);
			continue;
		}
		if (g_strcmp0(names[i], "subject") == 0) {
			entry->subject = g_strdup(values[i]);
			continue;
		}
		if (g_strcmp0(names[i], "recipient_addressing") == 0) {
			entry->recipient_addressing = g_strdup(values[i]);
			continue;
		}
		if (g_strcmp0(names[i], "sender_addressing") == 0) {
			entry->sender_addressing = g_strdup(values[i]);
			continue;
		}
		if (g_strcmp0(names[i], "type") == 0) {
			entry->type = g_strdup(values[i]);
			continue;
		}
		if (g_strcmp0(names[i], "reception_status") == 0)
			entry->reception_status = g_strdup(values[i]);
	}

	if (mld->size > mld->offset)
		mld->callback(mld->session, -EAGAIN, mld->size, 0, entry, mld->user_data);

	g_free(entry->reception_status);
	g_free(entry->type);
	g_free(entry->sender_addressing);
	g_free(entry->subject);
	g_free(entry->datetime);
	g_free(entry->attachment_size);
	g_free(entry->handle);
	g_free(entry);
}

static const GMarkupParser msg_parser = {
        msg_element,
        NULL,
        NULL,
        NULL,
        NULL
};

static const GMarkupParser max_msg_parser = {
        max_msg_element,
        NULL,
        NULL,
        NULL,
        NULL
};

static gboolean get_messages_listing(void *d)
{

	struct message_listing_data *mld = d;
	/* 1024 is the maximum size of the line which is calculated to be more
	 * sufficient*/
	char buffer[1024];
	GMarkupParseContext *ctxt;
	size_t len;

	while (fgets(buffer, 1024, mld->fp)) {
		len = strlen(buffer);

		if (mld->max == 0) {
			ctxt = g_markup_parse_context_new(&max_msg_parser, 0, mld, NULL);
			g_markup_parse_context_parse(ctxt, buffer, len, NULL);
			g_markup_parse_context_free(ctxt);
		} else {
			ctxt = g_markup_parse_context_new(&msg_parser, 0, mld, NULL);
			g_markup_parse_context_parse(ctxt, buffer, len, NULL);
			g_markup_parse_context_free(ctxt);
		}
	}

	if (mld->max == 0) {
		mld->callback(mld->session, 0, mld->size, 0, NULL, mld->user_data);
		goto done;
	}

	mld->callback(mld->session, 0, mld->size, 0, NULL, mld->user_data);

done:
	fclose(mld->fp);
	return FALSE;
}

int messages_get_messages_listing(void *session, const char *name,
				uint16_t max, uint16_t offset,
				uint8_t subject_len,
				const struct messages_filter *filter,
				messages_get_messages_listing_cb callback,
				void *user_data)
{
	struct message_listing_data *mld;
	struct session *s =  session;
	char *path;

	mld = g_new0(struct message_listing_data, 1);
	mld->session = s;
	mld->name = name;
	mld->max = max;
	mld->offset = offset;
	mld->subject_len = subject_len;
	mld->callback = callback;
	mld->filter = filter;
	mld->user_data = user_data;

	path = g_build_filename(s->cwd_absolute, MSG_LIST_XML, NULL);
	mld->fp = fopen(path, "r");
	if (mld->fp == NULL) {
		g_free(path);
		messages_set_folder(s, mld->name, 0);
		path = g_build_filename(s->cwd_absolute, MSG_LIST_XML, NULL);
		mld->fp = fopen(path, "r");
		if (mld->fp == NULL) {
			int err = -errno;
			DBG("fopen(): %d, %s", -err, strerror(-err));
			g_free(path);
			return -EBADR;
		}
	}


	g_idle_add_full(G_PRIORITY_DEFAULT_IDLE, get_messages_listing,
								mld, g_free);
	g_free(path);

	return 0;
}

int messages_get_message(void *session, const char *handle,
					unsigned long flags,
					messages_get_message_cb callback,
					void *user_data)
{
	return -ENOSYS;
}

int messages_update_inbox(void *session, messages_status_cb callback,
							void *user_data)
{
	return -ENOSYS;
}

int messages_set_read(void *session, const char *handle, uint8_t value,
				messages_status_cb callback, void *user_data)
{
	return -ENOSYS;
}

int messages_set_delete(void *session, const char *handle, uint8_t value,
				messages_status_cb callback, void *user_data)
{
	return -ENOSYS;
}

void messages_abort(void *s)
{
	struct session *session = s;

	if (session->request) {
		g_idle_remove_by_data(session->request);
		session->request = NULL;
	}
}
