/*
 * Copyright (C) 2008-2014 Tobias Brunner
 * Copyright (C) 2005-2008 Martin Willi
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

#define _GNU_SOURCE /* for memrchr */
#include <utils/utils.h>
#include <utils/debug.h>
#include <utils/chunk.h>

#include <limits.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>

/*
 * Described in header
 */
char *path_first_separator(const char *path, int len)
{
	if (!path)
	{
		return NULL;
	}
	if (len < 0)
	{
		len = strlen(path);
	}
	for (; len; path++, len--)
	{
		if (path_is_separator(*path))
		{
			return (char*)path;
		}
	}
	return NULL;
}

/*
 * Described in header
 */
char *path_last_separator(const char *path, int len)
{
	if (!path)
	{
		return NULL;
	}
	if (len < 0)
	{
		len = strlen(path);
	}
	while (len)
	{
		if (path_is_separator(path[--len]))
		{
			return (char*)&path[len];
		}
	}
	return NULL;
}

/*
 * Described in header
 */
char *path_dirname(const char *path)
{
	char *pos;

	pos = path_last_separator(path, -1);

	if (pos && !pos[1])
	{	/* if path ends with separators, we have to look beyond them */
		while (pos > path && path_is_separator(*pos))
		{	/* skip trailing separators */
			pos--;
		}
		pos = path_last_separator(path, pos - path + 1);
	}
	if (!pos)
	{
#ifdef WIN32
		if (path && strlen(path))
		{
			if ((isalpha(path[0]) && path[1] == ':'))
			{	/* if just a drive letter given, return that as dirname */
				return chunk_clone(chunk_from_chars(path[0], ':', 0)).ptr;
			}
		}
#endif
		return strdup(".");
	}
	while (pos > path && path_is_separator(*pos))
	{	/* skip superfluous separators */
		pos--;
	}
	return strndup(path, pos - path + 1);
}

/*
 * Described in header
 */
char *path_basename(const char *path)
{
	char *pos, *trail = NULL;

	if (!path || !*path)
	{
		return strdup(".");
	}
	pos = path_last_separator(path, -1);
	if (pos && !pos[1])
	{	/* if path ends with separators, we have to look beyond them */
		while (pos > path && path_is_separator(*pos))
		{	/* skip trailing separators */
			pos--;
		}
		if (pos == path && path_is_separator(*pos))
		{	/* contains only separators */
			return strndup(pos, 1);
		}
		trail = pos + 1;
		pos = path_last_separator(path, trail - path);
	}
	pos = pos ? pos + 1 : (char*)path;
	return trail ? strndup(pos, trail - pos) : strdup(pos);
}

/*
 * Described in header
 */
bool path_absolute(const char *path)
{
	if (!path)
	{
		return FALSE;
	}
#ifdef WIN32
	if (strpfx(path, "\\\\"))
	{	/* UNC */
		return TRUE;
	}
	if (strlen(path) && isalpha(path[0]) && path[1] == ':')
	{	/* drive letter */
		return TRUE;
	}
#endif /* WIN32 */
	if (path_is_separator(path[0]))
	{
		return TRUE;
	}
	return FALSE;
}

/*
 * Described in header
 */
bool mkdir_p(const char *path, mode_t mode)
{
	int len;
	char *pos, sep, full[PATH_MAX];
	pos = full;
	if (!path || *path == '\0')
	{
		return TRUE;
	}
	len = snprintf(full, sizeof(full)-1, "%s", path);
	if (len < 0 || len >= sizeof(full)-1)
	{
		DBG1(DBG_LIB, "path string %s too long", path);
		return FALSE;
	}
	/* ensure that the path ends with a separator */
	if (!path_is_separator(full[len-1]))
	{
		full[len++] = DIRECTORY_SEPARATOR[0];
		full[len] = '\0';
	}
	/* skip separators at the beginning */
	while (path_is_separator(*pos))
	{
		pos++;
	}
	while ((pos = path_first_separator(pos, -1)))
	{
		sep = *pos;
		*pos = '\0';
		if (access(full, F_OK) < 0)
		{
#ifdef WIN32
			if (_mkdir(full) < 0)
#else
			if (mkdir(full, mode) < 0)
#endif
			{
				DBG1(DBG_LIB, "failed to create directory %s", full);
				return FALSE;
			}
		}
		*pos = sep;
		pos++;
	}
	return TRUE;
}
