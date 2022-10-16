/*
 * Copyright (C) 2008-2014 Tobias Brunner
 * Copyright (C) 2008 Martin Willi
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

/**
 * @defgroup path_i path
 * @{ @ingroup utils_i
 */

#ifndef PATH_H_
#define PATH_H_

/**
 * Directory separator character in paths on this platform
 */
#ifdef WIN32
# define DIRECTORY_SEPARATOR "\\"
/* the Windows API also accepts / as separator */
# define DIRECTORY_SEPARATOR_ALT "/"
#else
# define DIRECTORY_SEPARATOR "/"
#endif

/**
 * Check if the given character is a directory separator.
 *
 * @param c			character
 * @return			TRUE if the character is a directory separator
 */
static inline bool path_is_separator(char c)
{
#ifdef WIN32
	return c == DIRECTORY_SEPARATOR[0] || c == DIRECTORY_SEPARATOR_ALT[0];
#else
	return c == DIRECTORY_SEPARATOR[0];
#endif
}

/**
 * Returns a pointer to the first occurrence of a directory separator in the
 * given string (optionally limited to a given length).
 *
 * @param path		path to search
 * @param len		optional length to search, -1 for the full string
 * @return			pointer to separator, or NULL if not found
 */
char *path_first_separator(const char *path, int len);

/**
 * Returns a pointer to the last occurrence of a directory separator in the
 * given string (optionally limited to a given length).
 *
 * @param path		path to search
 * @param len		optional length to search, -1 for the full string
 * @return			pointer to separator, or NULL if not found
 */
char *path_last_separator(const char *path, int len);

/**
 * Like dirname(3) returns the directory part of the given null-terminated
 * pathname, up to but not including the final '/' (or '.' if no '/' is found).
 * Trailing '/' are not counted as part of the pathname.
 *
 * The difference is that it does this in a thread-safe manner (i.e. it does not
 * use static buffers) and does not modify the original path.
 *
 * @param path		original pathname
 * @return			allocated directory component
 */
char *path_dirname(const char *path);

/**
 * Like basename(3) returns the filename part of the given null-terminated path,
 * i.e. the part following the final '/' (or '.' if path is empty or NULL).
 * Trailing '/' are not counted as part of the pathname.
 *
 * The difference is that it does this in a thread-safe manner (i.e. it does not
 * use static buffers) and does not modify the original path.
 *
 * @param path		original pathname
 * @return			allocated filename component
 */
char *path_basename(const char *path);

/**
 * Check if a given path is absolute.
 *
 * @param path		path to check
 * @return			TRUE if absolute, FALSE if relative
 */
bool path_absolute(const char *path);

/**
 * Creates a directory and all required parent directories.
 *
 * @param path		path to the new directory
 * @param mode		permissions of the new directory/directories
 * @return			TRUE on success
 */
bool mkdir_p(const char *path, mode_t mode);

#endif /** PATH_H_ @} */
