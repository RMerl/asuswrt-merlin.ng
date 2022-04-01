/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 * Copyright 2020, ASUSTeK Inc.
 * Based on fbida code by Gerd Hoffmann.
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND ASUS GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <rtconfig.h>

#ifdef RTCONFIG_URLFW

#include <curl/curl.h>
#include <curl/easy.h>

struct url_state {
	CURLM *multi;
	CURL *curl;
	size_t pos;
	struct {
		size_t pos;
		size_t len;
		size_t size;
		unsigned char *data;
	} buf;
	struct {
		size_t pos;
		int running:1;
		int body:1;
		int eof:1;
	} stream;
	char *path;
};

/* not necessary for single applet
static void __attribute__((constructor)) curl_init(void)
{
	curl_global_init(CURL_GLOBAL_ALL);
}

static void __attribute__((destructor)) curl_fini(void)
{
	curl_global_cleanup();
}
*/

static size_t curl_header(void *data, size_t size, size_t nmemb, void *handle)
{
	struct url_state *h = handle;

	h->stream.running = 1;
	size *= nmemb;

	return size;
}

static size_t curl_write(void *data, size_t size, size_t nmemb, void *handle)
{
	struct url_state *h = handle;

	h->stream.running = h->stream.body = 1;
	size *= nmemb;

	if (h->buf.size < size) {
		void *buf = realloc(h->buf.data, size);
		if (buf) {
			h->buf.data = buf;
			h->buf.size = size;
		} else
			size = h->buf.size;
	}

	h->buf.pos = h->stream.pos;
	h->buf.len = size;
	memcpy(h->buf.data, data, size);

	h->stream.pos += size;
	return size;
}

static int curl_setup(struct url_state *h, int *running)
{
	CURLMcode rc;
	char *url;

	if (h->curl == NULL) {
		h->curl = curl_easy_init();
		if (h->curl == NULL)
			return -1;
		url = h->path;
	} else {
		curl_multi_remove_handle(h->multi, h->curl);
		if (curl_easy_getinfo(h->curl, CURLINFO_EFFECTIVE_URL, &url) == CURLE_OK &&
		    url != NULL && strcmp(url, h->path) != 0) {
			free(h->path);
			h->path = strdup(url);
			if (h->path == NULL)
				goto error;
		}
		curl_easy_reset(h->curl);
	}

	curl_easy_setopt(h->curl, CURLOPT_URL, h->path);
	curl_easy_setopt(h->curl, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(h->curl, CURLOPT_FAILONERROR, 1L);

	curl_easy_setopt(h->curl, CURLOPT_WRITEFUNCTION, curl_write);
	curl_easy_setopt(h->curl, CURLOPT_WRITEDATA, h);
	curl_easy_setopt(h->curl, CURLOPT_HEADERFUNCTION, curl_header);
	curl_easy_setopt(h->curl, CURLOPT_HEADERDATA, h);
	curl_easy_setopt(h->curl, CURLOPT_HEADER, 0L);

	//curl_easy_setopt(h->curl, CURLOPT_VERBOSE, 1L);
	curl_easy_setopt(h->curl, CURLOPT_NOPROGRESS, 1L);
	curl_easy_setopt(h->curl, CURLOPT_NOSIGNAL, 1L);

	curl_easy_setopt(h->curl, CURLOPT_CONNECTTIMEOUT, 10L);
	//curl_easy_setopt(curl, CURLOPT_TIMEOUT, 120L);

	memset(&h->stream, 0, sizeof(h->stream));

	if (curl_multi_add_handle(h->multi, h->curl) != CURLM_OK)
		goto error;

	while ((rc = curl_multi_perform(h->multi, running)) == CURLM_CALL_MULTI_PERFORM);
	if (rc != CURLM_OK) {
		curl_multi_remove_handle(h->multi, h->curl);
		goto error;
	}

	return 0;

error:
	curl_easy_cleanup(h->curl);
	h->curl = NULL;
	return -1;
}

static int curl_transfer(struct url_state *h)
{
	CURLMcode rc;
	int running;

	rc = curl_multi_wait(h->multi, NULL, 0, 1000000, NULL);
	if (rc != CURLM_OK)
		return -1;

	while ((rc = curl_multi_perform(h->multi, &running)) == CURLM_CALL_MULTI_PERFORM);
	if (rc != CURLM_OK)
		return -1;

	if (running == 0)
		h->stream.eof = 1;

	return 0;
}

static ssize_t url_read(void *handle, char *buf, size_t size)
{
	struct url_state *h = handle;
	size_t bytes, total;
	int running;

	for (total = 0; size > 0;) {
		if (h->pos >= h->buf.pos && h->pos < h->buf.pos + h->buf.len) {
			bytes = h->buf.pos + h->buf.len - h->pos;
			if (bytes > size)
				bytes = size;
			memcpy(buf + total, h->buf.data + h->pos - h->buf.pos, bytes);
			size -= bytes;
			total += bytes;
			h->pos += bytes;
			continue;
		}
		if (h->pos < h->buf.pos) {
			h->buf.pos = h->buf.len = 0;
			if (curl_setup(h, &running) < 0)
				return -1;
		}
		if (h->stream.eof)
			break;
		if (curl_transfer(h) < 0) {
			if (total == 0)
				return -1;
			break;
		}
	}

	return total;
}

static int url_seek(void *handle, off64_t *pos, int whence)
{
	struct url_state *h = handle;
	CURLMcode rc;
	curl_off_t size;
	off64_t new = *pos;

	switch (whence) {
	case SEEK_SET:
		break;
	case SEEK_CUR:
		new += h->pos;
		break;
	case SEEK_END:
		rc = curl_easy_getinfo(h->curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &size);
		if (rc != CURLM_OK || size == -1) {
			errno = EINVAL;
			return -1;
		}
		new += size;
		break;
	}
	*pos = h->pos = new;

	return 0;
}

static int url_close(void *handle)
{
	struct url_state *h = handle;

	if (h->multi && h->curl)
		curl_multi_remove_handle(h->multi, h->curl);
	if (h->multi)
		curl_multi_cleanup(h->multi);
	if (h->curl)
		curl_easy_cleanup(h->curl);
	free(h->buf.data);
	free(h->path);
	free(h);

	return 0;
}

static cookie_io_functions_t url_hooks = {
	.read = url_read,
	.seek = url_seek,
	.close = url_close,
};

static int url_scheme(const char *path)
{
	static const char * const protocols[] = {
#ifdef RTCONFIG_HTTPS
		"https://",
#endif
		"http://",
		"ftp://",
		NULL,
	};
	const char * const *p;

	for (p = protocols; *p != NULL; p++) {
		if (strncasecmp(path, *p, strlen(*p)) == 0)
			return 1;
	}

	return 0;
}

FILE *url_fopen(const char *path, const char *mode)
{
	struct url_state *h;
	FILE *fp;
	int running, err = 0;

	if (!url_scheme(path))
		return fopen(path, mode);

	if (strchr(mode, 'w') != NULL) {
		errno = EACCES;
		return NULL;
	}

	h = calloc(1, sizeof(*h));
	if (h == NULL)
		return NULL;

	h->path = strdup(path);
	if (h->path == NULL) {
		err = errno;
		goto error;
	}

	h->multi = curl_multi_init();
	if (h->multi == NULL) {
		err = errno;
		goto error;
	}

	if (curl_setup(h, &running) < 0) {
		err = errno;
		goto error;
	}

	if (!running && !h->stream.running) {
		err = ENOENT;
		goto error;
	}

	fp = fopencookie(h, mode, url_hooks);
	if (fp == NULL) {
		err = errno;
		goto error;
	}

	while (!h->stream.body && !h->stream.eof) {
		if (curl_transfer(h) < 0)
			break;
	}

	return fp;

error:
	url_close(h);
	errno = err;
	return NULL;
}

#endif /* RTCONFIG_URLFW */
