/*
 * utils.c - multibyte-string helpers
 * Copyright (c) Clemens Ladisch <clemens@ladisch.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#define _XOPEN_SOURCE
#include "aconfig.h"
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include "utils.h"

/*
 * mbs_at_width - compute screen position in a string
 *
 * For displaying strings on the screen, we have to know how many character
 * cells are occupied.  This function calculates the position in a multibyte
 * string that is at a desired position.
 *
 * Parameters:
 * s:     the string
 * width: on input, the desired number of character cells; on output, the actual
 *        position, in character cells, of the return value
 * dir:   -1 or 1; in which direction to round if a multi-column character goes
 *        over the desired width
 *
 * Return value:
 * Pointer to the place in the string that is as near the desired width as
 * possible.  If the string is too short, the return value points to the
 * terminating zero.  If the last character is a multi-column character that
 * goes over the desired width, the return value may be one character cell
 * earlier or later than desired, depending on the dir parameter.
 * In any case, the return value points after any zero-width characters that
 * follow the last character.
 */
const char *mbs_at_width(const char *s, int *width, int dir)
{
	size_t len;
	wchar_t wc;
	int bytes;
	int width_so_far, w;

	if (*width <= 0)
		return s;
	mbtowc(NULL, NULL, 0); /* reset shift state */
	len = strlen(s);
	width_so_far = 0;
	while (len && (bytes = mbtowc(&wc, s, len)) > 0) {
		w = wcwidth(wc);
		if (width_so_far + w > *width && dir < 0)
			break;
		if (w >= 0)
			width_so_far += w;
		s += bytes;
		len -= bytes;
		if (width_so_far >= *width) {
			while (len && (bytes = mbtowc(&wc, s, len)) > 0) {
				w = wcwidth(wc);
				if (w != 0)
					break;
				s += bytes;
				len -= bytes;
			}
			break;
		}
	}
	*width = width_so_far;
	return s;
}

/*
 * get_mbs_width - compute screen width of a string
 */
unsigned int get_mbs_width(const char *s)
{
	int width;

	width = INT_MAX;
	mbs_at_width(s, &width, 1);
	return width;
}

/*
 * get_max_mbs_width - get width of longest string in an array
 */
unsigned int get_max_mbs_width(const char *const *s, unsigned int count)
{
	unsigned int max_width, i, len;

	max_width = 0;
	for (i = 0; i < count; ++i) {
		len = get_mbs_width(s[i]);
		if (len > max_width)
			max_width = len;
	}
	return max_width;
}
