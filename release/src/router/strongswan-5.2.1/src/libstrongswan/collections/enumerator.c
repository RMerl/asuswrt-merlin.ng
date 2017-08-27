/*
 * Copyright (C) 2008-2013 Tobias Brunner
 * Copyright (C) 2007 Martin Willi
 * Hochschule fuer Technik Rapperswil
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

/**
 * Implementation of enumerator_create_empty().enumerate
 */
static bool enumerate_empty(enumerator_t *enumerator, ...)
{
	return FALSE;
}

/**
 * See header
 */
enumerator_t* enumerator_create_empty()
{
	enumerator_t *this = malloc_thing(enumerator_t);
	this->enumerate = enumerate_empty;
	this->destroy = (void*)free;
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

/**
 * Implementation of enumerator_create_directory().destroy
 */
static void destroy_dir_enum(dir_enum_t *this)
{
	closedir(this->dir);
	free(this);
}

/**
 * Implementation of enumerator_create_directory().enumerate
 */
static bool enumerate_dir_enum(dir_enum_t *this, char **relative,
							   char **absolute, struct stat *st)
{
	struct dirent *entry = readdir(this->dir);
	size_t remaining;
	int len;

	if (!entry)
	{
		return FALSE;
	}
	if (streq(entry->d_name, ".") || streq(entry->d_name, ".."))
	{
		return enumerate_dir_enum(this, relative, absolute, st);
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

/**
 * See header
 */
enumerator_t* enumerator_create_directory(const char *path)
{
	int len;
	dir_enum_t *this = malloc_thing(dir_enum_t);
	this->public.enumerate = (void*)enumerate_dir_enum;
	this->public.destroy = (void*)destroy_dir_enum;

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
	if (this->dir == NULL)
	{
		DBG1(DBG_LIB, "opening directory '%s' failed: %s", path, strerror(errno));
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

/**
 * Implementation of enumerator_create_glob().destroy
 */
static void destroy_glob_enum(glob_enum_t *this)
{
	globfree(&this->glob);
	free(this);
}

/**
 * Implementation of enumerator_create_glob().enumerate
 */
static bool enumerate_glob_enum(glob_enum_t *this, char **file, struct stat *st)
{
	char *match;

	if (this->pos >= this->glob.gl_pathc)
	{
		return FALSE;
	}
	match = this->glob.gl_pathv[this->pos++];
	if (file)
	{
		*file = match;
	}
	if (st)
	{
		if (stat(match, st))
		{
			DBG1(DBG_LIB, "stat() on '%s' failed: %s", match,
				 strerror(errno));
			return FALSE;
		}
	}
	return TRUE;
}

/**
 * See header
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
			.enumerate = (void*)enumerate_glob_enum,
			.destroy = (void*)destroy_glob_enum,
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
	/** separater chars */
	const char *sep;
	/** trim chars */
	const char *trim;
} token_enum_t;

/**
 * Implementation of enumerator_create_token().destroy
 */
static void destroy_token_enum(token_enum_t *this)
{
	free(this->string);
	free(this);
}

/**
 * Implementation of enumerator_create_token().enumerate
 */
static bool enumerate_token_enum(token_enum_t *this, char **token)
{
	const char *sep, *trim;
	char *pos = NULL, *tmp;
	bool last = FALSE;

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

/**
 * See header
 */
enumerator_t* enumerator_create_token(const char *string, const char *sep,
									  const char *trim)
{
	token_enum_t *enumerator = malloc_thing(token_enum_t);

	enumerator->public.enumerate = (void*)enumerate_token_enum;
	enumerator->public.destroy = (void*)destroy_token_enum;
	enumerator->string = strdup(string);
	enumerator->pos = enumerator->string;
	enumerator->sep = sep;
	enumerator->trim = trim;

	return &enumerator->public;
}

/**
 * enumerator for nested enumerations
 */
typedef struct {
	/* implements enumerator_t */
	enumerator_t public;
	/* outer enumerator */
	enumerator_t *outer;
	/* inner enumerator */
	enumerator_t *inner;
	/* constructor for inner enumerator */
	enumerator_t *(*create_inner)(void *outer, void *data);
	/* data to pass to constructor above */
	void *data;
	/* destructor for data */
	void (*destroy_data)(void *data);
} nested_enumerator_t;


/**
 * Implementation of enumerator_create_nested().enumerate()
 */
static bool enumerate_nested(nested_enumerator_t *this, void *v1, void *v2,
							 void *v3, void *v4, void *v5)
{
	while (TRUE)
	{
		while (this->inner == NULL)
		{
			void *outer;

			if (!this->outer->enumerate(this->outer, &outer))
			{
				return FALSE;
			}
			this->inner = this->create_inner(outer, this->data);
		}
		if (this->inner->enumerate(this->inner, v1, v2, v3, v4, v5))
		{
			return TRUE;
		}
		this->inner->destroy(this->inner);
		this->inner = NULL;
	}
}

/**
 * Implementation of enumerator_create_nested().destroy()
 **/
static void destroy_nested(nested_enumerator_t *this)
{
	if (this->destroy_data)
	{
		this->destroy_data(this->data);
	}
	DESTROY_IF(this->inner);
	this->outer->destroy(this->outer);
	free(this);
}

/**
 * See header
 */
enumerator_t *enumerator_create_nested(enumerator_t *outer,
					enumerator_t *(inner_constructor)(void *outer, void *data),
					void *data, void (*destroy_data)(void *data))
{
	nested_enumerator_t *enumerator = malloc_thing(nested_enumerator_t);

	enumerator->public.enumerate = (void*)enumerate_nested;
	enumerator->public.destroy = (void*)destroy_nested;
	enumerator->outer = outer;
	enumerator->inner = NULL;
	enumerator->create_inner = (void*)inner_constructor;
	enumerator->data = data;
	enumerator->destroy_data = destroy_data;

	return &enumerator->public;
}

/**
 * enumerator for filtered enumerator
 */
typedef struct {
	enumerator_t public;
	enumerator_t *unfiltered;
	void *data;
	bool (*filter)(void *data, ...);
	void (*destructor)(void *data);
} filter_enumerator_t;

/**
 * Implementation of enumerator_create_filter().destroy
 */
static void destroy_filter(filter_enumerator_t *this)
{
	if (this->destructor)
	{
		this->destructor(this->data);
	}
	this->unfiltered->destroy(this->unfiltered);
	free(this);
}

/**
 * Implementation of enumerator_create_filter().enumerate
 */
static bool enumerate_filter(filter_enumerator_t *this, void *o1, void *o2,
							 void *o3, void *o4, void *o5)
{
	void *i1, *i2, *i3, *i4, *i5;

	while (this->unfiltered->enumerate(this->unfiltered, &i1, &i2, &i3, &i4, &i5))
	{
		if (this->filter(this->data, &i1, o1, &i2, o2, &i3, o3, &i4, o4, &i5, o5))
		{
			return TRUE;
		}
	}
	return FALSE;
}

/**
 * see header
 */
enumerator_t *enumerator_create_filter(enumerator_t *unfiltered,
									   bool (*filter)(void *data, ...),
									   void *data, void (*destructor)(void *data))
{
	filter_enumerator_t *this = malloc_thing(filter_enumerator_t);

	this->public.enumerate = (void*)enumerate_filter;
	this->public.destroy = (void*)destroy_filter;
	this->unfiltered = unfiltered;
	this->filter = filter;
	this->data = data;
	this->destructor = destructor;

	return &this->public;
}

/**
 * enumerator for cleaner enumerator
 */
typedef struct {
	enumerator_t public;
	enumerator_t *wrapped;
	void (*cleanup)(void *data);
	void *data;
} cleaner_enumerator_t;

/**
 * Implementation of enumerator_create_cleanup().destroy
 */
static void destroy_cleaner(cleaner_enumerator_t *this)
{
	this->cleanup(this->data);
	this->wrapped->destroy(this->wrapped);
	free(this);
}

/**
 * Implementation of enumerator_create_cleaner().enumerate
 */
static bool enumerate_cleaner(cleaner_enumerator_t *this, void *v1, void *v2,
							  void *v3, void *v4, void *v5)
{
	return this->wrapped->enumerate(this->wrapped, v1, v2, v3, v4, v5);
}

/**
 * see header
 */
enumerator_t *enumerator_create_cleaner(enumerator_t *wrapped,
										void (*cleanup)(void *data), void *data)
{
	cleaner_enumerator_t *this = malloc_thing(cleaner_enumerator_t);

	this->public.enumerate = (void*)enumerate_cleaner;
	this->public.destroy = (void*)destroy_cleaner;
	this->wrapped = wrapped;
	this->cleanup = cleanup;
	this->data = data;

	return &this->public;
}

/**
 * enumerator for single enumerator
 */
typedef struct {
	enumerator_t public;
	void *item;
	void (*cleanup)(void *item);
	bool done;
} single_enumerator_t;

/**
 * Implementation of enumerator_create_single().destroy
 */
static void destroy_single(single_enumerator_t *this)
{
	if (this->cleanup)
	{
		this->cleanup(this->item);
	}
	free(this);
}

/**
 * Implementation of enumerator_create_single().enumerate
 */
static bool enumerate_single(single_enumerator_t *this, void **item)
{
	if (this->done)
	{
		return FALSE;
	}
	*item = this->item;
	this->done = TRUE;
	return TRUE;
}

/**
 * see header
 */
enumerator_t *enumerator_create_single(void *item, void (*cleanup)(void *item))
{
	single_enumerator_t *this = malloc_thing(single_enumerator_t);

	this->public.enumerate = (void*)enumerate_single;
	this->public.destroy = (void*)destroy_single;
	this->item = item;
	this->cleanup = cleanup;
	this->done = FALSE;

	return &this->public;
}

