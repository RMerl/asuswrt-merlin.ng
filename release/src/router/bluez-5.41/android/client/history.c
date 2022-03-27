/*
 * Copyright (C) 2013 Intel Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "history.h"

/* Very simple history storage for easy usage of tool */

#define HISTORY_DEPTH 40
#define LINE_SIZE 200
static char lines[HISTORY_DEPTH][LINE_SIZE];
static int last_line = 0;
static int history_size = 0;

/* TODO: Storing history not implemented yet */
void history_store(const char *filename)
{
}

/* Restoring history from file */
void history_restore(const char *filename)
{
	char line[1000];
	FILE *f = fopen(filename, "rt");

	if (f == NULL)
		return;

	for (;;) {
		if (fgets(line, 1000, f) != NULL) {
			int l = strlen(line);

			while (l > 0 && isspace(line[--l]))
				line[l] = 0;

			if (l > 0)
				history_add_line(line);
		} else
			break;
	}

	fclose(f);
}

/* Add new line to history buffer */
void history_add_line(const char *line)
{
	if (line == NULL || strlen(line) == 0)
		return;

	if (strcmp(line, lines[last_line]) == 0)
		return;

	last_line = (last_line + 1) % HISTORY_DEPTH;
	strncpy(&lines[last_line][0], line, LINE_SIZE - 1);
	if (history_size < HISTORY_DEPTH)
		history_size++;
}

/*
 * Get n-th line from history
 * 0 - means latest
 * -1 - means oldest
 * return -1 if there is no such line
 */
int history_get_line(int n, char *buf, int buf_size)
{
	if (n == -1)
		n = history_size - 1;

	if (n >= history_size || buf_size == 0 || n < 0)
		return -1;

	strncpy(buf,
		&lines[(HISTORY_DEPTH + last_line - n) % HISTORY_DEPTH][0],
		buf_size - 1);
	buf[buf_size - 1] = 0;

	return n;
}
