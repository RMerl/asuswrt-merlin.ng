/*
 * Copyright (C) 2012-2020 Tobias Brunner
 * Copyright (C) 2006 Martin Willi
 *
 * Copyright (C) secunet Security Networks AG
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

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>

#include "file_logger.h"

#include <daemon.h>
#include <threading/mutex.h>
#include <threading/rwlock.h>

typedef struct private_file_logger_t private_file_logger_t;

/**
 * Private data of a file_logger_t object
 */
struct private_file_logger_t {

	/**
	 * Public data.
	 */
	file_logger_t public;

	/**
	 * File name of the target
	 */
	char *filename;

	/**
	 * Current output file
	 */
	FILE *out;

	/**
	 * Flush after writing a line?
	 */
	bool flush_line;

	/**
	 * Maximum level to log, for each group
	 */
	level_t levels[DBG_MAX];

	/**
	 * strftime() format of time prefix, if any
	 */
	char *time_format;

	/**
	 * Add milliseconds after the time string
	 */
	bool add_ms;

	/**
	 * Print the name/# of the IKE_SA?
	 */
	bool ike_name;

	/**
	 * Print the log level
	 */
	bool log_level;

	/**
	 * Mutex to ensure multi-line log messages are not torn apart
	 */
	mutex_t *mutex;

	/**
	 * Lock to read/write options (FD, levels, time_format, etc.)
	 */
	rwlock_t *lock;
};

METHOD(logger_t, log_, void,
	private_file_logger_t *this, debug_t group, level_t level, int thread,
	ike_sa_t* ike_sa, const char *message)
{
	char groupstr[5], timestr[128], namestr[128] = "";
	const char *current = message, *next;
	struct tm tm;
	timeval_t tv;
	time_t s;
	u_int ms = 0;

	this->lock->read_lock(this->lock);
	if (!this->out)
	{	/* file is not open */
		this->lock->unlock(this->lock);
		return;
	}

	if (this->time_format)
	{
		gettimeofday(&tv, NULL);
		s = tv.tv_sec;
		ms = tv.tv_usec / 1000;
		localtime_r(&s, &tm);
		strftime(timestr, sizeof(timestr), this->time_format, &tm);
	}

	if (this->log_level)
	{
		snprintf(groupstr, sizeof(groupstr), "%N%d", debug_names, group,
				 level);
	}
	else
	{
		snprintf(groupstr, sizeof(groupstr), "%N", debug_names, group);
	}

	if (this->ike_name && ike_sa)
	{
		if (ike_sa->get_peer_cfg(ike_sa))
		{
			snprintf(namestr, sizeof(namestr), " <%s|%d>",
				ike_sa->get_name(ike_sa), ike_sa->get_unique_id(ike_sa));
		}
		else
		{
			snprintf(namestr, sizeof(namestr), " <%d>",
				ike_sa->get_unique_id(ike_sa));
		}
	}
	else
	{
		namestr[0] = '\0';
	}

	/* prepend a prefix in front of every line */
	this->mutex->lock(this->mutex);
	while (TRUE)
	{
		next = strchr(current, '\n');
		if (this->time_format)
		{
			if (this->add_ms)
			{
				fprintf(this->out, "%s.%03u %.2d[%s]%s ",
						timestr, ms, thread, groupstr, namestr);
			}
			else
			{
				fprintf(this->out, "%s %.2d[%s]%s ",
						timestr, thread, groupstr, namestr);
			}
		}
		else
		{
			fprintf(this->out, "%.2d[%s]%s ",
					thread, groupstr, namestr);
		}
		if (next == NULL)
		{
			fprintf(this->out, "%s\n", current);
			break;
		}
		fprintf(this->out, "%.*s\n", (int)(next - current), current);
		current = next + 1;
	}
#ifndef HAVE_SETLINEBUF
	if (this->flush_line)
	{
		fflush(this->out);
	}
#endif /* !HAVE_SETLINEBUF */
	this->mutex->unlock(this->mutex);
	this->lock->unlock(this->lock);
}

METHOD(logger_t, get_level, level_t,
	private_file_logger_t *this, debug_t group)
{
	level_t level;

	this->lock->read_lock(this->lock);
	level = this->levels[group];
	this->lock->unlock(this->lock);
	return level;
}

METHOD(file_logger_t, set_level, void,
	private_file_logger_t *this, debug_t group, level_t level)
{
	this->lock->write_lock(this->lock);
	if (group < DBG_ANY)
	{
		this->levels[group] = level;
	}
	else
	{
		for (group = 0; group < DBG_MAX; group++)
		{
			this->levels[group] = level;
		}
	}
	this->lock->unlock(this->lock);
}

METHOD(file_logger_t, set_options, void,
	private_file_logger_t *this, char *time_format, bool add_ms, bool ike_name,
	bool log_level)
{
	this->lock->write_lock(this->lock);
	free(this->time_format);
	this->time_format = strdupnull(time_format);
	this->add_ms = add_ms;
	this->ike_name = ike_name;
	this->log_level = log_level;
	this->lock->unlock(this->lock);
}

/**
 * Close the current file, if any
 */
static void close_file(private_file_logger_t *this)
{
	if (this->out && this->out != stdout && this->out != stderr)
	{
		fclose(this->out);
		this->out = NULL;
	}
}

METHOD(file_logger_t, open_, void,
	private_file_logger_t *this, bool flush_line, bool append)
{
	FILE *file;

	if (streq(this->filename, "stderr"))
	{
		file = stderr;
	}
	else if (streq(this->filename, "stdout"))
	{
		file = stdout;
	}
	else
	{
		file = fopen(this->filename, append ? "a" : "w");
		if (file == NULL)
		{
			DBG1(DBG_DMN, "opening file %s for logging failed: %s",
				 this->filename, strerror(errno));
			return;
		}
#ifdef HAVE_CHOWN
		if (lib->caps->check(lib->caps, CAP_CHOWN))
		{
			if (chown(this->filename, lib->caps->get_uid(lib->caps),
					  lib->caps->get_gid(lib->caps)) != 0)
			{
				DBG1(DBG_NET, "changing owner/group for '%s' failed: %s",
					 this->filename, strerror(errno));
			}
		}
		else
		{
			if (chown(this->filename, -1, lib->caps->get_gid(lib->caps)) != 0)
			{
				DBG1(DBG_NET, "changing group for '%s' failed: %s",
					 this->filename, strerror(errno));
			}
		}
#endif /* HAVE_CHOWN */
#ifdef HAVE_SETLINEBUF
		if (flush_line)
		{
			setlinebuf(file);
		}
#endif /* HAVE_SETLINEBUF */
	}
	this->lock->write_lock(this->lock);
	close_file(this);
	this->out = file;
	this->flush_line = flush_line;
	this->lock->unlock(this->lock);
}

METHOD(file_logger_t, destroy, void,
	private_file_logger_t *this)
{
	this->lock->write_lock(this->lock);
	close_file(this);
	this->lock->unlock(this->lock);
	this->mutex->destroy(this->mutex);
	this->lock->destroy(this->lock);
	free(this->time_format);
	free(this->filename);
	free(this);
}

/*
 * Described in header.
 */
file_logger_t *file_logger_create(char *filename)
{
	private_file_logger_t *this;

	INIT(this,
		.public = {
			.logger = {
				.log = _log_,
				.get_level = _get_level,
			},
			.set_level = _set_level,
			.set_options = _set_options,
			.open = _open_,
			.destroy = _destroy,
		},
		.filename = strdup(filename),
		.mutex = mutex_create(MUTEX_TYPE_DEFAULT),
		.lock = rwlock_create(RWLOCK_TYPE_DEFAULT),
	);

	set_level(this, DBG_ANY, LEVEL_SILENT);

	return &this->public;
}
