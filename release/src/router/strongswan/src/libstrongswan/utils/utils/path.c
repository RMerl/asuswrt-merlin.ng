/*
 * Copyright (C) 2008-2014 Tobias Brunner
 * Copyright (C) 2005-2008 Martin Willi
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

#define _GNU_SOURCE /* for memrchr */
#include <utils/utils.h>
#include <utils/debug.h>
#include <utils/chunk.h>

#include <limits.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>

/**
 * Described in header.
 */
char* path_dirname(const char *path)
{
	char *pos;

	pos = path ? strrchr(path, DIRECTORY_SEPARATOR[0]) : NULL;

	if (pos && !pos[1])
	{	/* if path ends with slashes we have to look beyond them */
		while (pos > path && *pos == DIRECTORY_SEPARATOR[0])
		{	/* skip trailing slashes */
			pos--;
		}
		pos = memrchr(path, DIRECTORY_SEPARATOR[0], pos - path + 1);
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
	while (pos > path && *pos == DIRECTORY_SEPARATOR[0])
	{	/* skip superfluous slashes */
		pos--;
	}
	return strndup(path, pos - path + 1);
}

/**
 * Described in header.
 */
char* path_basename(const char *path)
{
	char *pos, *trail = NULL;

	if (!path || !*path)
	{
		return strdup(".");
	}
	pos = strrchr(path, DIRECTORY_SEPARATOR[0]);
	if (pos && !pos[1])
	{	/* if path ends with slashes we have to look beyond them */
		while (pos > path && *pos == DIRECTORY_SEPARATOR[0])
		{	/* skip trailing slashes */
			pos--;
		}
		if (pos == path && *pos == DIRECTORY_SEPARATOR[0])
		{	/* contains only slashes */
			return strdup(DIRECTORY_SEPARATOR);
		}
		trail = pos + 1;
		pos = memrchr(path, DIRECTORY_SEPARATOR[0], trail - path);
	}
	pos = pos ? pos + 1 : (char*)path;
	return trail ? strndup(pos, trail - pos) : strdup(pos);
}

/**
 * Described in header.
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
#else /* !WIN32 */
	if (path[0] == DIRECTORY_SEPARATOR[0])
	{
		return TRUE;
	}
#endif
	return FALSE;
}

/**
 * Described in header.
 */
bool mkdir_p(const char *path, mode_t mode)
{
	int len;
	char *pos, full[PATH_MAX];
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
	/* ensure that the path ends with a '/' */
	if (full[len-1] != '/')
	{
		full[len++] = '/';
		full[len] = '\0';
	}
	/* skip '/' at the beginning */
	while (*pos == '/')
	{
		pos++;
	}
	while ((pos = strchr(pos, '/')))
	{
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
		*pos = '/';
		pos++;
	}
	return TRUE;
}
