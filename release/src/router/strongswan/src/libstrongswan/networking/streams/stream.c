/*
 * Copyright (C) 2013 Martin Willi
 * Copyright (C) 2013 revosec AG
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

#include <library.h>
#include <errno.h>
#include <unistd.h>

#include "stream.h"

typedef struct private_stream_t private_stream_t;

/**
 * Private data of an stream_t object.
 */
struct private_stream_t {

	/**
	 * Public stream_t interface.
	 */
	stream_t public;

	/**
	 * Underlying socket
	 */
	int fd;

	/**
	 * Callback if data is ready to read
	 */
	stream_cb_t read_cb;

	/**
	 * Data for read-ready callback
	 */
	void *read_data;

	/**
	 * Callback if write is non-blocking
	 */
	stream_cb_t write_cb;

	/**
	 * Data for write-ready callback
	 */
	void *write_data;
};

METHOD(stream_t, read_, ssize_t,
	private_stream_t *this, void *buf, size_t len, bool block)
{
	while (TRUE)
	{
		ssize_t ret;

		if (block)
		{
			ret = recv(this->fd, buf, len, 0);
		}
		else
		{
			ret = recv(this->fd, buf, len, MSG_DONTWAIT);
			if (ret == -1 && errno == EAGAIN)
			{
				/* unify EGAIN and EWOULDBLOCK */
				errno = EWOULDBLOCK;
			}
		}
		if (ret == -1 && errno == EINTR)
		{	/* interrupted, try again */
			continue;
		}
		return ret;
	}
}

METHOD(stream_t, read_all, bool,
	private_stream_t *this, void *buf, size_t len)
{
	ssize_t ret;

	while (len)
	{
		ret = read_(this, buf, len, TRUE);
		if (ret < 0)
		{
			return FALSE;
		}
		if (ret == 0)
		{
			errno = ECONNRESET;
			return FALSE;
		}
		len -= ret;
		buf += ret;
	}
	return TRUE;
}

METHOD(stream_t, write_, ssize_t,
	private_stream_t *this, void *buf, size_t len, bool block)
{
	ssize_t ret;

	while (TRUE)
	{
		if (block)
		{
			ret = send(this->fd, buf, len, 0);
		}
		else
		{
			ret = send(this->fd, buf, len, MSG_DONTWAIT);
			if (ret == -1 && errno == EAGAIN)
			{
				/* unify EGAIN and EWOULDBLOCK */
				errno = EWOULDBLOCK;
			}
		}
		if (ret == -1 && errno == EINTR)
		{	/* interrupted, try again */
			continue;
		}
		return ret;
	}
}

METHOD(stream_t, write_all, bool,
	private_stream_t *this, void *buf, size_t len)
{
	ssize_t ret;

	while (len)
	{
		ret = write_(this, buf, len, TRUE);
		if (ret < 0)
		{
			return FALSE;
		}
		if (ret == 0)
		{
			errno = ECONNRESET;
			return FALSE;
		}
		len -= ret;
		buf += ret;
	}
	return TRUE;
}

/**
 * Watcher callback
 */
static bool watch(private_stream_t *this, int fd, watcher_event_t event)
{
	bool keep = FALSE;
	stream_cb_t cb;

	switch (event)
	{
		case WATCHER_READ:
			cb = this->read_cb;
			this->read_cb = NULL;
			keep = cb(this->read_data, &this->public);
			if (keep)
			{
				this->read_cb = cb;
			}
			break;
		case WATCHER_WRITE:
			cb = this->write_cb;
			this->write_cb = NULL;
			keep = cb(this->write_data, &this->public);
			if (keep)
			{
				this->write_cb = cb;
			}
			break;
		case WATCHER_EXCEPT:
			break;
	}
	return keep;
}

/**
 * Register watcher for stream callbacks
 */
static void add_watcher(private_stream_t *this)
{
	watcher_event_t events = 0;

	if (this->read_cb)
	{
		events |= WATCHER_READ;
	}
	if (this->write_cb)
	{
		events |= WATCHER_WRITE;
	}
	if (events)
	{
		lib->watcher->add(lib->watcher, this->fd, events,
						  (watcher_cb_t)watch, this);
	}
}

METHOD(stream_t, on_read, void,
	private_stream_t *this, stream_cb_t cb, void *data)
{
	lib->watcher->remove(lib->watcher, this->fd);

	this->read_cb = cb;
	this->read_data = data;

	add_watcher(this);
}

METHOD(stream_t, on_write, void,
	private_stream_t *this, stream_cb_t cb, void *data)
{
	lib->watcher->remove(lib->watcher, this->fd);

	this->write_cb = cb;
	this->write_data = data;

	add_watcher(this);
}

METHOD(stream_t, get_file, FILE*,
	private_stream_t *this)
{
	FILE *file;
	int fd;

	/* fclose() closes the FD passed to fdopen(), so dup() it */
	fd = dup(this->fd);
	if (fd == -1)
	{
		return NULL;
	}
	file = fdopen(fd, "w+");
	if (!file)
	{
		close(fd);
	}
	return file;
}

METHOD(stream_t, destroy, void,
	private_stream_t *this)
{
	lib->watcher->remove(lib->watcher, this->fd);
	close(this->fd);
	free(this);
}

/**
 * See header
 */
stream_t *stream_create_from_fd(int fd)
{
	private_stream_t *this;

	INIT(this,
		.public = {
			.read = _read_,
			.read_all = _read_all,
			.on_read = _on_read,
			.write = _write_,
			.write_all = _write_all,
			.on_write = _on_write,
			.get_file = _get_file,
			.destroy = _destroy,
		},
		.fd = fd,
	);

	return &this->public;
}
