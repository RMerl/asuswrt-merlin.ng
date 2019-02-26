/*
 * Copyright (C) 2007 Martin Willi
 * HSR Hochschule fuer Technik Rapperswil
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#define _GNU_SOURCE

#include "fast_request.h"

#include <library.h>
#include <utils/debug.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <ClearSilver/ClearSilver.h>

#include <threading/thread.h>
#include <threading/thread_value.h>

typedef struct private_fast_request_t private_fast_request_t;

/**
 * private data of the task manager
 */
struct private_fast_request_t {

	/**
	 * public functions
	 */
	fast_request_t public;

	/**
	 * FastCGI request object
	 */
	FCGX_Request req;

	/**
	 * length of the req.envp array
	 */
	int req_env_len;

	/**
	 * ClearSilver CGI Kit context
	 */
	CGI *cgi;

	/**
	 * ClearSilver HDF dataset for this request
	 */
	HDF *hdf;

	/**
	 * close the session?
	 */
	bool closed;

	/**
	 * reference count
	 */
	refcount_t ref;
};

/**
 * ClearSilver cgiwrap is not threadsafe, so we use a private
 * context for each thread.
 */
static thread_value_t *thread_this;

/**
 * control variable for pthread_once
 */
pthread_once_t once = PTHREAD_ONCE_INIT;

/**
 * fcgiwrap read callback
 */
static int read_cb(void *null, char *buf, int size)
{
	private_fast_request_t *this;

	this = (private_fast_request_t*)thread_this->get(thread_this);

	return FCGX_GetStr(buf, size, this->req.in);
}

/**
 * fcgiwrap writef callback
 */
static int writef_cb(void *null, const char *format, va_list args)
{
	private_fast_request_t *this;

	this = (private_fast_request_t*)thread_this->get(thread_this);

	FCGX_VFPrintF(this->req.out, format, args);
	return 0;
}
/**
 * fcgiwrap write callback
 */
static int write_cb(void *null, const char *buf, int size)
{
	private_fast_request_t *this;

	this = (private_fast_request_t*)thread_this->get(thread_this);

	return FCGX_PutStr(buf, size, this->req.out);
}

/**
 * fcgiwrap getenv callback
 */
static char *getenv_cb(void *null, const char *key)
{
	char *value;
	private_fast_request_t *this;

	this = (private_fast_request_t*)thread_this->get(thread_this);

	value = FCGX_GetParam(key, this->req.envp);
	return strdupnull(value);
}

/**
 * fcgiwrap getenv callback
 */
static int putenv_cb(void *null, const char *key, const char *value)
{
	/* not supported */
	return 1;
}

/**
 * fcgiwrap iterenv callback
 */
static int iterenv_cb(void *null, int num, char **key, char **value)
{
	private_fast_request_t *this;

	*key = NULL;
	*value = NULL;
	this = (private_fast_request_t*)thread_this->get(thread_this);

	if (num < this->req_env_len)
	{
		char *eq;

		eq = strchr(this->req.envp[num], '=');
		if (eq)
		{
			*key = strndup(this->req.envp[num], eq - this->req.envp[num]);
			*value = strdup(eq + 1);
		}
		if (*key == NULL || *value == NULL)
		{
			free(*key);
			free(*value);
			return 1;
		}
	}
	return 0;
}

METHOD(fast_request_t, get_cookie, char*,
	private_fast_request_t *this, char *name)
{
	return hdf_get_valuef(this->hdf, "Cookie.%s", name);
}

METHOD(fast_request_t, get_path, char*,
	private_fast_request_t *this)
{
	char *path = FCGX_GetParam("PATH_INFO", this->req.envp);
	return path ? path : "";
}

METHOD(fast_request_t, get_host, char*,
	private_fast_request_t *this)
{
	char *addr = FCGX_GetParam("REMOTE_ADDR", this->req.envp);
	return addr ? addr : "";
}

METHOD(fast_request_t, get_user_agent, char*,
	private_fast_request_t *this)
{
	char *agent = FCGX_GetParam("HTTP_USER_AGENT", this->req.envp);
	return agent ? agent : "";
}

METHOD(fast_request_t, get_query_data, char*,
	private_fast_request_t *this, char *name)
{
	return hdf_get_valuef(this->hdf, "Query.%s", name);
}

METHOD(fast_request_t, get_env_var, char*,
	private_fast_request_t *this, char *name)
{
	return FCGX_GetParam(name, this->req.envp);
}

METHOD(fast_request_t, read_data, int,
	private_fast_request_t *this, char *buf, int len)
{
	return FCGX_GetStr(buf, len, this->req.in);
}

METHOD(fast_request_t, get_base, char*,
	private_fast_request_t *this)
{
	return FCGX_GetParam("SCRIPT_NAME", this->req.envp);
}

METHOD(fast_request_t, add_cookie, void,
	private_fast_request_t *this, char *name, char *value)
{
	thread_this->set(thread_this, this);
	cgi_cookie_set(this->cgi, name, value, NULL, NULL, NULL, 0, 0);
}

METHOD(fast_request_t, redirect, void,
	private_fast_request_t *this, char *fmt, ...)
{
	va_list args;

	FCGX_FPrintF(this->req.out, "Status: 303 See Other\n");
	FCGX_FPrintF(this->req.out, "Location: %s%s", get_base(this),
				 *fmt == '/' ? "" : "/");
	va_start(args, fmt);
	FCGX_VFPrintF(this->req.out, fmt, args);
	va_end(args);
	FCGX_FPrintF(this->req.out, "\n\n");
}

METHOD(fast_request_t, get_referer, char*,
	private_fast_request_t *this)
{
	return FCGX_GetParam("HTTP_REFERER", this->req.envp);
}

METHOD(fast_request_t, to_referer, void,
	private_fast_request_t *this)
{
	char *referer;

	referer = get_referer(this);
	if (referer)
	{
		FCGX_FPrintF(this->req.out, "Status: 303 See Other\n");
		FCGX_FPrintF(this->req.out, "Location: %s\n\n", referer);
	}
	else
	{
		redirect(this, "/");
	}
}

METHOD(fast_request_t, session_closed, bool,
	private_fast_request_t *this)
{
	return this->closed;
}

METHOD(fast_request_t, close_session, void,
	private_fast_request_t *this)
{
	this->closed = TRUE;
}

METHOD(fast_request_t, serve, void,
	private_fast_request_t *this, char *headers, chunk_t chunk)
{
	FCGX_FPrintF(this->req.out, "%s\n\n", headers);

	FCGX_PutStr(chunk.ptr, chunk.len, this->req.out);
}

METHOD(fast_request_t, sendfile, bool,
	private_fast_request_t *this, char *path, char *mime)
{
	chunk_t *data;
	int written;
	char buf[24];

	data = chunk_map(path, FALSE);
	if (!data)
	{
		return FALSE;
	}
	/* FCGX does not like large integers, print to a buffer using libc */
	snprintf(buf, sizeof(buf), "%lld", (int64_t)data->len);
	FCGX_FPrintF(this->req.out, "Content-Length: %s\n", buf);
	if (mime)
	{
		FCGX_FPrintF(this->req.out, "Content-Type: %s\n", mime);
	}
	FCGX_FPrintF(this->req.out, "\n");

	while (data->len)
	{
		written = FCGX_PutStr(data->ptr, data->len, this->req.out);
		if (written == -1)
		{
			chunk_unmap(data);
			return FALSE;
		}
		*data = chunk_skip(*data, written);
	}

	chunk_unmap(data);
	return TRUE;
}

METHOD(fast_request_t, render, void,
	private_fast_request_t *this, char *template)
{
	NEOERR* err;

	thread_this->set(thread_this, this);
	err = cgi_display(this->cgi, template);
	if (err)
	{
		cgi_neo_error(this->cgi, err);
		nerr_log_error(err);
	}
}

METHOD(fast_request_t, streamf, int,
	private_fast_request_t *this, char *format, ...)
{
	va_list args;
	int written;

	va_start(args, format);
	written = FCGX_VFPrintF(this->req.out, format, args);
	va_end(args);
	if (written >= 0 &&
		FCGX_FFlush(this->req.out) == -1)
	{
		return -1;
	}
	return written;
}

METHOD(fast_request_t, set, void,
	private_fast_request_t *this, char *key, char *value)
{
	hdf_set_value(this->hdf, key, value);
}

METHOD(fast_request_t, setf, void,
	private_fast_request_t *this, char *format, ...)
{
	va_list args;

	va_start(args, format);
	hdf_set_valuevf(this->hdf, format, args);
	va_end(args);
}

METHOD(fast_request_t, get_ref, fast_request_t*,
	private_fast_request_t *this)
{
	ref_get(&this->ref);
	return &this->public;
}

METHOD(fast_request_t, destroy, void,
	private_fast_request_t *this)
{
	if (ref_put(&this->ref))
	{
		thread_this->set(thread_this, this);
		cgi_destroy(&this->cgi);
		FCGX_Finish_r(&this->req);
		free(this);
	}
}

/**
 * This initialization method is guaranteed to run only once
 * for all threads.
 */
static void init(void)
{
	cgiwrap_init_emu(NULL, read_cb, writef_cb, write_cb,
					 getenv_cb, putenv_cb, iterenv_cb);
	thread_this = thread_value_create(NULL);
}

/*
 * see header file
 */
fast_request_t *fast_request_create(int fd, bool debug)
{
	NEOERR* err;
	private_fast_request_t *this;
	bool failed = FALSE;

	INIT(this,
		.public = {
			.get_path = _get_path,
			.get_base = _get_base,
			.get_host = _get_host,
			.get_user_agent = _get_user_agent,
			.add_cookie = _add_cookie,
			.get_cookie = _get_cookie,
			.get_query_data = _get_query_data,
			.get_env_var = _get_env_var,
			.read_data = _read_data,
			.session_closed = _session_closed,
			.close_session = _close_session,
			.redirect = _redirect,
			.get_referer = _get_referer,
			.to_referer = _to_referer,
			.render = _render,
			.streamf = _streamf,
			.serve = _serve,
			.sendfile = _sendfile,
			.set = _set,
			.setf = _setf,
			.get_ref = _get_ref,
			.destroy = _destroy,
		},
		.ref = 1,
	);

	thread_cleanup_push(free, this);
	if (FCGX_InitRequest(&this->req, fd, 0) != 0 ||
		FCGX_Accept_r(&this->req) != 0)
	{
		failed = TRUE;
	}
	thread_cleanup_pop(failed);
	if (failed)
	{
		return NULL;
	}

	pthread_once(&once, init);
	thread_this->set(thread_this, this);

	while (this->req.envp[this->req_env_len] != NULL)
	{
		this->req_env_len++;
	}

	err = hdf_init(&this->hdf);
	if (!err)
	{
		hdf_set_value(this->hdf, "base", get_base(this));
		hdf_set_value(this->hdf, "Config.NoCache", "true");
		if (!debug)
		{
			hdf_set_value(this->hdf, "Config.TimeFooter", "0");
			hdf_set_value(this->hdf, "Config.CompressionEnabled", "1");
			hdf_set_value(this->hdf, "Config.WhiteSpaceStrip", "2");
		}

		err = cgi_init(&this->cgi, this->hdf);
		if (!err)
		{
			err = cgi_parse(this->cgi);
			if (!err)
			{
				return &this->public;
			}
			cgi_destroy(&this->cgi);
		}
	}
	nerr_log_error(err);
	FCGX_Finish_r(&this->req);
	free(this);
	return NULL;
}
