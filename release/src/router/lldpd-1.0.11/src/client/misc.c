/* -*- mode: c; c-file-style: "openbsd" -*- */
/*
 * Copyright (c) 2012 Vincent Bernat <bernat@luffy.cx>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "client.h"
#include <string.h>
#include <ctype.h>

/**
 * Check if an element is present in a comma-separated list.
 *
 * @param list    Comma-separated list of elements.
 * @param element Element we want to check for.
 * @return 0 if the element was not found, 1 otherwise.
 */
int
contains(const char *list, const char *element)
{
	int len;
	if (element == NULL || list == NULL) return 0;
	while (list) {
		len = strlen(element);
		if (!strncmp(list, element, len) &&
		    (list[len] == '\0' || list[len] == ','))
			return 1;
		list = strchr(list, ',');
		if (list) list++;
	}
	return 0;
}

/**
 * Transform a string to a tag. This puts the string into lower space and
 * replace spaces with '-'. The result is statically allocated.
 *
 * @param value String to transform to a tag.
 * @return The tagged value or the string "none" if @c value is @c NULL
 */
char*
totag(const char *value)
{
	int i;
	static char *result = NULL;
	free(result); result = NULL;
	if (!value) return "none";
	result = calloc(1, strlen(value) + 1);
	if (!result) return "none";
	for (i = 0; i < strlen(value); i++) {
		switch (value[i]) {
		case ' ': result[i] = '-'; break;
		default: result[i] = tolower((int)value[i]); break;
		}
	}
	return result;
}
