/*
 * names.c		db names
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "names.h"
#include "utils.h"

#define MAX_ENTRIES  256
#define NAME_MAX_LEN 512

static int read_id_name(FILE *fp, int *id, char *name)
{
	char buf[NAME_MAX_LEN];
	int min, maj;

	while (fgets(buf, sizeof(buf), fp)) {
		char *p = buf;

		while (*p == ' ' || *p == '\t')
			p++;

		if (*p == '#' || *p == '\n' || *p == 0)
			continue;

		if (sscanf(p, "%x:%x %s\n", &maj, &min, name) == 3) {
			*id = (maj << 16) | min;
		} else if (sscanf(p, "%x:%x %s #", &maj, &min, name) == 3) {
			*id = (maj << 16) | min;
		} else if (sscanf(p, "0x%x %s\n", id, name) != 2 &&
				sscanf(p, "0x%x %s #", id, name) != 2 &&
				sscanf(p, "%d %s\n", id, name) != 2 &&
				sscanf(p, "%d %s #", id, name) != 2) {
			strcpy(name, p);
			return -1;
		}
		return 1;
	}

	return 0;
}

struct db_names *db_names_alloc(void)
{
	struct db_names *db;

	db = malloc(sizeof(*db));
	if (!db)
		return NULL;

	memset(db, 0, sizeof(*db));

	db->size = MAX_ENTRIES;
	db->hash = malloc(sizeof(struct db_entry *) * db->size);
	memset(db->hash, 0, sizeof(struct db_entry *) * db->size);

	return db;
}

int db_names_load(struct db_names *db, const char *path)
{
	struct db_entry *entry;
	FILE *fp;
	int id;
	char namebuf[NAME_MAX_LEN] = {0};
	int ret = -1;

	fp = fopen(path, "r");
	if (!fp)
		return -ENOENT;

	while ((ret = read_id_name(fp, &id, &namebuf[0]))) {
		if (ret == -1) {
			fprintf(stderr, "Database %s is corrupted at %s\n",
					path, namebuf);
			goto Exit;
		}
		ret = -1;

		if (id < 0)
			continue;

		entry = malloc(sizeof(*entry));
		if (!entry)
			goto Exit;

		entry->name = strdup(namebuf);
		if (!entry->name) {
			free(entry);
			goto Exit;
		}

		entry->id   = id;
		entry->next = db->hash[id & (db->size - 1)];
		db->hash[id & (db->size - 1)] = entry;
	}
	ret = 0;

Exit:
	fclose(fp);
	return ret;
}

void db_names_free(struct db_names *db)
{
	int i;

	if (!db)
		return;

	for (i = 0; i < db->size; i++) {
		struct db_entry *entry = db->hash[i];

		while (entry) {
			struct db_entry *next = entry->next;

			free(entry->name);
			free(entry);
			entry = next;
		}
	}

	free(db->hash);
	free(db);
}

char *id_to_name(struct db_names *db, int id, char *name)
{
	struct db_entry *entry;

	if (!db)
		return NULL;

	entry = db->hash[id & (db->size - 1)];
	while (entry && entry->id != id)
		entry = entry->next;

	if (entry) {
		strncpy(name, entry->name, IDNAME_MAX);
		return name;
	}

	snprintf(name, IDNAME_MAX, "%d", id);
	return NULL;
}

int name_to_id(struct db_names *db, int *id, const char *name)
{
	struct db_entry *entry;
	int i;

	if (!db)
		return -1;

	if (db->cached && strcmp(db->cached->name, name) == 0) {
		*id = db->cached->id;
		return 0;
	}

	for (i = 0; i < db->size; i++) {
		entry = db->hash[i];
		while (entry && strcmp(entry->name, name))
			entry = entry->next;

		if (entry) {
			db->cached = entry;
			*id = entry->id;
			return 0;
		}
	}

	return -1;
}
