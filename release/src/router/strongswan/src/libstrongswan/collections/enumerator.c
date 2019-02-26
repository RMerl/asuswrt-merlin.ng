/*
 * Copyright (C) 2008-2017 Tobias Brunner
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

#include "enumerator.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#include <stdio.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>

#ifdef HAVE_GLOB_H
#include <glob.h>
#endif /* HAVE_GLOB_H */

#include <utils/debug.h>

/*
 * Described in header.
 */
bool enumerator_enumerate_default(enumerator_t *enumerator, ...)
{
	va_list args;
	bool result;

	if (!enumerator->venumerate)
	{
		DBG1(DBG_LIB, "!!! ENUMERATE DEFAULT: venumerate() missing !!!");
		return FALSE;
	}
	va_start(args, enumerator);
	result = enumerator->venumerate(enumerator, args);
	va_end(args);
	return result;
}

METHOD(enumerator_t, enumerate_empty, bool,
	enumerator_t *enumerator, va_list args)
{
	return FALSE;
}

/*
 * Described in header
 */
enumerator_t* enumerator_create_empty()
{
	enumerator_t *this;

	INIT(this,
		.enumerate = enumerator_enumerate_default,
		.venumerate = _enumerate_empty,
		.destroy = (void*)free,
	);
	return this;
}

/**
 * Enumerator implementation for directory enumerator
 */
typedef struct {
	/** implements enumerator_t */
	enumerator_t public;
	/** directory handle */
	DIR *dir;
	/** absolute path of current file */
	char full[PATH_MAX];
	/** where directory part of full ends and relative file gets written */
	char *full_end;
} dir_enum_t;

METHOD(enumerator_t, destroy_dir_enum, void,
	dir_enum_t *this)
{
	closedir(this->dir);
	free(this);
}

METHOD(enumerator_t, enumerate_dir_enum, bool,
	dir_enum_t *this, va_list args)
{
	struct dirent *entry = readdir(this->dir);
	struct stat *st;
	size_t remaining;
	char **relative, **absolute;
	int len;

	VA_ARGS_VGET(args, relative, absolute, st);

	if (!entry)
	{
		return FALSE;
	}
	if (streq(entry->d_name, ".") || streq(entry->d_name, ".."))
	{
		return this->public.enumerate(&this->public, relative, absolute, st);
	}
	if (relative)
	{
		*relative = entry->d_name;
	}
	if (absolute || st)
	{
		remaining = sizeof(this->full) - (this->full_end - this->full);
		len = snprintf(this->full_end, remaining, "%s", entry->d_name);
		if (len < 0 || len >= remaining)
		{
			DBG1(DBG_LIB, "buffer too small to enumerate file '%s'",
				 entry->d_name);
			return FALSE;
		}
		if (absolute)
		{
			*absolute = this->full;
		}
		if (st)
		{
			if (stat(this->full, st))
			{
				DBG1(DBG_LIB, "stat() on '%s' failed: %s", this->full,
					 strerror(errno));
				return FALSE;
			}
		}
	}
	return TRUE;
}

/*
 * Described in header
 */
enumerator_t* enumerator_create_directory(const char *path)
{
	dir_enum_t *this;
	int len;

	INIT(this,
		.public = {
			.enumerate = enumerator_enumerate_default,
			.venumerate = _enumerate_dir_enum,
			.destroy = _destroy_dir_enum,
		},
	);

	if (*path == '\0')
	{
		path = "./";
	}
	len = snprintf(this->full, sizeof(this->full)-1, "%s", path);
	if (len < 0 || len >= sizeof(this->full)-1)
	{
		DBG1(DBG_LIB, "path string '%s' too long", path);
		free(this);
		return NULL;
	}
	/* append a '/' if not already done */
	if (this->full[len-1] != '/')
	{
		this->full[len++] = '/';
		this->full[len] = '\0';
	}
	this->full_end = &this->full[len];

	this->dir = opendir(path);
	if (!this->dir)
	{
		DBG1(DBG_LIB, "opening directory '%s' failed: %s", path,
			 strerror(errno));
		free(this);
		return NULL;
	}
	return &this->public;
}

#ifdef HAVE_GLOB_H

/**
 * Enumerator implementation for glob enumerator
 */
typedef struct {
	/** implements enumerator_t */
	enumerator_t public;
	/** glob data */
	glob_t glob;
	/** iteration count */
	u_int pos;
	/** absolute path of current file */
	char full[PATH_MAX];
} glob_enum_t;

METHOD(enumerator_t, destroy_glob_enum, void,
	glob_enum_t *this)
{
	globfree(&this->glob);
	free(this);
}

METHOD(enumerator_t, enumerate_glob_enum, bool,
	glob_enum_t *this, va_list args)
{
	struct stat *st;
	char *match;
	char **file;

	VA_ARGS_VGET(args, file, st);

	if (this->pos >= this->glob.gl_pathc)
	{
		return FALSE;
	}
	match = this->glob.gl_pathv[this->pos++];
	if (file)
	{
		*file = match;
	}
	if (st && stat(match, st))
	{
		DBG1(DBG_LIB, "stat() on '%s' failed: %s", match,
			 strerror(errno));
		return FALSE;
	}
	return TRUE;
}

/*
 * Described in header
 */
enumerator_t* enumerator_create_glob(const char *pattern)
{
	glob_enum_t *this;
	int status;

	if (!pattern)
	{
		return enumerator_create_empty();
	}

	INIT(this,
		.public = {
			.enumerate = enumerator_enumerate_default,
			.venumerate = _enumerate_glob_enum,
			.destroy = _destroy_glob_enum,
		},
	);

	status = glob(pattern, GLOB_ERR, NULL, &this->glob);
	if (status == GLOB_NOMATCH)
	{
		DBG1(DBG_LIB, "no files found matching '%s'", pattern);
	}
	else if (status != 0)
	{
		DBG1(DBG_LIB, "expanding file pattern '%s' failed: %s", pattern,
			 strerror(errno));
	}
	return &this->public;
}

#else /* HAVE_GLOB_H */

enumerator_t* enumerator_create_glob(const char *pattern)
{
	return NULL;
}

#endif /* HAVE_GLOB_H */

/**
 * Enumerator implementation for token enumerator
 */
typedef struct {
	/** implements enumerator_t */
	enumerator_t public;
	/** string to parse */
	char *string;
	/** current position */
	char *pos;
	/** separator chars */
	const char *sep;
	/** trim chars */
	const char *trim;
} token_enum_t;

METHOD(enumerator_t, destroy_token_enum, void,
	token_enum_t *this)
{
	free(this->string);
	free(this);
}

METHOD(enumerator_t, enumerate_token_enum, bool,
	token_enum_t *this, va_list args)
{
	const char *sep, *trim;
	char *pos = NULL, *tmp, **token;
	bool last = FALSE;

	VA_ARGS_VGET(args, token);

	/* trim leading characters/separators */
	while (*this->pos)
	{
		trim = this->trim;
		while (*trim)
		{
			if (*trim == *this->pos)
			{
				this->pos++;
				break;
			}
			trim++;
		}
		sep = this->sep;
		while (*sep)
		{
			if (*sep == *this->pos)
			{
				this->pos++;
				break;
			}
			sep++;
		}
		if (!*trim && !*sep)
		{
			break;
		}
	}

	switch (*this->pos)
	{
		case '"':
		case '\'':
		{
			/* read quoted token */
			tmp = strchr(this->pos + 1, *this->pos);
			if (tmp)
			{
				*token = this->pos + 1;
				*tmp = '\0';
				this->pos = tmp + 1;
				return TRUE;
			}
			/* unterminated string, FALL-THROUGH */
		}
		default:
		{
			/* find nearest separator */
			sep = this->sep;
			while (*sep)
			{
				tmp = strchr(this->pos, *sep);
				if (tmp && (pos == NULL || tmp < pos))
				{
					pos = tmp;
				}
				sep++;
			}
			*token = this->pos;
			if (pos)
			{
				*pos = '\0';
				this->pos = pos + 1;
			}
			else
			{
				last = TRUE;
				pos = this->pos = strchr(this->pos, '\0');
			}
			break;
		}
	}

	/* trim trailing characters */
	pos--;
	while (pos >= *token)
	{
		trim = this->trim;
		while (*trim)
		{
			if (*trim == *pos)
			{
				*(pos--) = '\0';
				break;
			}
			trim++;
		}
		if (!*trim)
		{
			break;
		}
	}

	if (!last || pos >= *token)
	{
		return TRUE;
	}
	return FALSE;
}

/*
 * Described in header
 */
enumerator_t* enumerator_create_token(const char *string, const char *sep,
									  const char *trim)
{
	token_enum_t *this;

	INIT(this,
		.public = {
			.enumerate = enumerator_enumerate_default,
			.venumerate = _enumerate_token_enum,
			.destroy = _destroy_token_enum,
		},
		.string = strdup(string),
		.sep = sep,
		.trim = trim,
	);
	this->pos = this->string;

	return &this->public;
}

/**
 * Enumerator for nested enumerations
 */
typedef struct {
	enumerator_t public;
	enumerator_t *outer;
	enumerator_t *inner;
	enumerator_t *(*create_inner)(void *outer, void *data);
	void *data;
	void (*destructor)(void *data);
} nested_enumerator_t;


METHOD(enumerator_t, enumerate_nested, bool,
	nested_enumerator_t *this, va_list args)
{
	while (TRUE)
	{
		while (!this->inner)
		{
			void *outer;

			if (!this->outer->enumerate(this->outer, &outer))
			{
				return FALSE;
			}
			this->inner = this->create_inner(outer, this->data);
			if (this->inner && !this->inner->venumerate)
			{
				DBG1(DBG_LIB, "!!! ENUMERATE NESTED: venumerate() missing !!!");
				return FALSE;
			}
		}
		if (this->inner->venumerate(this->inner, args))
		{
			return TRUE;
		}
		this->inner->destroy(this->inner);
		this->inner = NULL;
	}
}

METHOD(enumerator_t, destroy_nested, void,
	nested_enumerator_t *this)
{
	if (this->destructor)
	{
		this->destructor(this->data);
	}
	DESTROY_IF(this->inner);
	this->outer->destroy(this->outer);
	free(this);
}

/*
 * Described in header
 */
enumerator_t *enumerator_create_nested(enumerator_t *outer,
					enumerator_t *(inner_constructor)(void *outer, void *data),
					void *data, void (*destructor)(void *data))
{
	nested_enumerator_t *this;

	INIT(this,
		.public = {
			.enumerate = enumerator_enumerate_default,
			.venumerate = _enumerate_nested,
			.destroy = _destroy_nested,
		},
		.outer = outer,
		.create_inner = inner_constructor,
		.data = data,
		.destructor = destructor,
	);
	return &this->public;
}

/**
 * Enumerator for filtered enumerator
 */
typedef struct {
	enumerator_t public;
	enumerator_t *orig;
	void *data;
	bool (*filter)(void*,enumerator_t*,va_list);
	void (*destructor)(void *data);
} filter_enumerator_t;

METHOD(enumerator_t, destroy_filter, void,
	filter_enumerator_t *this)
{
	if (this->destructor)
	{
		this->destructor(this->data);
	}
	this->orig->destroy(this->orig);
	free(this);
}

METHOD(enumerator_t, enumerate_filter, bool,
	filter_enumerator_t *this, va_list args)
{
	bool result = FALSE;

	if (this->filter(this->data, this->orig, args))
	{
		result = TRUE;
	}
	return result;
}

/*
 * Described in header
 */
enumerator_t *enumerator_create_filter(enumerator_t *orig,
			bool (*filter)(void *data, enumerator_t *orig, va_list args),
			void *data, void (*destructor)(void *data))
{
	filter_enumerator_t *this;

	INIT(this,
		.public = {
			.enumerate = enumerator_enumerate_default,
			.venumerate = _enumerate_filter,
			.destroy = _destroy_filter,
		},
		.orig = orig,
		.filter = filter,
		.data = data,
		.destructor = destructor,
	);
	return &this->public;
}

/**
 * Enumerator for cleaner enumerator
 */
typedef struct {
	enumerator_t public;
	enumerator_t *wrapped;
	void (*cleanup)(void *data);
	void *data;
} cleaner_enumerator_t;

METHOD(enumerator_t, destroy_cleaner, void,
	cleaner_enumerator_t *this)
{
	this->cleanup(this->data);
	this->wrapped->destroy(this->wrapped);
	free(this);
}

METHOD(enumerator_t, enumerate_cleaner, bool,
	cleaner_enumerator_t *this, va_list args)
{
	if (!this->wrapped->venumerate)
	{
		DBG1(DBG_LIB, "!!! CLEANER ENUMERATOR: venumerate() missing !!!");
		return FALSE;
	}
	return this->wrapped->venumerate(this->wrapped, args);
}

/*
 * Described in header
 */
enumerator_t *enumerator_create_cleaner(enumerator_t *wrapped,
										void (*cleanup)(void *data), void *data)
{
	cleaner_enumerator_t *this;

	INIT(this,
		.public = {
			.enumerate = enumerator_enumerate_default,
			.venumerate = _enumerate_cleaner,
			.destroy = _destroy_cleaner,
		},
		.wrapped = wrapped,
		.cleanup = cleanup,
		.data = data,
	);
	return &this->public;
}

/**
 * Enumerator for single enumerator
 */
typedef struct {
	enumerator_t public;
	void *item;
	void (*cleanup)(void *item);
	bool done;
} single_enumerator_t;

METHOD(enumerator_t, destroy_single, void,
	single_enumerator_t *this)
{
	if (this->cleanup)
	{
		this->cleanup(this->item);
	}
	free(this);
}

METHOD(enumerator_t, enumerate_single, bool,
	single_enumerator_t *this, va_list args)
{
	void **item;

	VA_ARGS_VGET(args, item);
	if (this->done)
	{
		return FALSE;
	}
	*item = this->item;
	this->done = TRUE;
	return TRUE;
}

/*
 * Described in header
 */
enumerator_t *enumerator_create_single(void *item, void (*cleanup)(void *item))
{
	single_enumerator_t *this;

	INIT(this,
		.public = {
			.enumerate = enumerator_enumerate_default,
			.venumerate = _enumerate_single,
			.destroy = _destroy_single,
		},
		.item = item,
		.cleanup = cleanup,
	);
	return &this->public;
}
