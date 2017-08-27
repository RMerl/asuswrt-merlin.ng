/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * soup-path-map.c: URI path prefix-matcher
 *
 * Copyright (C) 2007 Novell, Inc.
 */

#include <string.h>

#include "soup-path-map.h"

/* This could be replaced with something more clever, like a Patricia
 * trie, but it's probably not worth it since the total number of
 * mappings is likely to always be small. So we keep an array of
 * paths, sorted by decreasing length. (The first prefix match will
 * therefore be the longest.)
 */

typedef struct {
	char     *path;
	int       len;
	gpointer  data;
} SoupPathMapping;

struct SoupPathMap {
	GArray *mappings;
	GDestroyNotify free_func;
};

/**
 * soup_path_map_new:
 * @data_free_func: function to use to free data added with
 * soup_path_map_add().
 *
 * Creates a new %SoupPathMap.
 *
 * Return value: the new %SoupPathMap
 **/
SoupPathMap *
soup_path_map_new (GDestroyNotify data_free_func)
{
	SoupPathMap *map;

	map = g_slice_new0 (SoupPathMap);
	map->mappings = g_array_new (FALSE, FALSE, sizeof (SoupPathMapping));
	map->free_func = data_free_func;

	return map;
}

/**
 * soup_path_map_free:
 * @map: a %SoupPathMap
 *
 * Frees @map and all data stored in it.
 **/
void
soup_path_map_free (SoupPathMap *map)
{
	SoupPathMapping *mappings = (SoupPathMapping *)map->mappings->data;
	int i;

	for (i = 0; i < map->mappings->len; i++) {
		g_free (mappings[i].path);
		if (map->free_func)
			map->free_func (mappings[i].data);
	}
	g_array_free (map->mappings, TRUE);
	
	g_slice_free (SoupPathMap, map);
}

/* Scan @map looking for @path or one of its ancestors.
 * Sets *@match to the index of a match, or -1 if no match is found.
 * Sets *@insert to the index to insert @path at if a new mapping is
 * desired. Returns %TRUE if *@match is an exact match.
 */
static gboolean
mapping_lookup (SoupPathMap *map, const char *path, int *match, int *insert)
{
	SoupPathMapping *mappings = (SoupPathMapping *)map->mappings->data;
	int i, path_len;
	gboolean exact = FALSE;

	*match = -1;

	path_len = strcspn (path, "?");
	for (i = 0; i < map->mappings->len; i++) {
		if (mappings[i].len > path_len)
			continue;

		if (insert && mappings[i].len < path_len) {
			*insert = i;
			/* Clear insert so we don't try to set it again */
			insert = NULL;
		}

		if (!strncmp (mappings[i].path, path, mappings[i].len)) {
			*match = i;
			if (path_len == mappings[i].len)
				exact = TRUE;
			if (!insert)
				return exact;
		}
	}

	if (insert)
		*insert = i;
	return exact;
}	

/**
 * soup_path_map_add:
 * @map: a %SoupPathMap
 * @path: the path
 * @data: the data
 *
 * Adds @data to @map at @path. If there was already data at @path it
 * will be freed.
 **/
void
soup_path_map_add (SoupPathMap *map, const char *path, gpointer data)
{
	SoupPathMapping *mappings = (SoupPathMapping *)map->mappings->data;
	int match, insert;

	if (mapping_lookup (map, path, &match, &insert)) {
		if (map->free_func)
			map->free_func (mappings[match].data);
		mappings[match].data = data;
	} else {
		SoupPathMapping mapping;

		mapping.path = g_strdup (path);
		mapping.len = strlen (path);
		mapping.data = data;
		g_array_insert_val (map->mappings, insert, mapping);
	}
}

/**
 * soup_path_map_remove:
 * @map: a %SoupPathMap
 * @path: the path
 *
 * Removes @data from @map at @path. (This must be called with the same
 * path the data was originally added with, not a subdirectory.)
 **/
void
soup_path_map_remove (SoupPathMap *map, const char *path)
{
	SoupPathMapping *mappings = (SoupPathMapping *)map->mappings->data;
	int match;

	if (!mapping_lookup (map, path, &match, NULL))
		return;

	if (map->free_func)
		map->free_func (mappings[match].data);
	g_free (mappings[match].path);
	g_array_remove_index (map->mappings, match);
}

/**
 * soup_path_map_lookup:
 * @map: a %SoupPathMap
 * @path: the path
 *
 * Finds the data associated with @path in @map. If there is no data
 * specifically associated with @path, it will return the data for the
 * closest parent directory of @path that has data associated with it.
 *
 * Return value: the data set with soup_path_map_add(), or %NULL if no
 * data could be found for @path or any of its ancestors.
 **/
gpointer
soup_path_map_lookup (SoupPathMap *map, const char *path)
{
	SoupPathMapping *mappings = (SoupPathMapping *)map->mappings->data;
	int match;

	mapping_lookup (map, path, &match, NULL);
	if (match == -1)
		return NULL;
	else
		return mappings[match].data;
}
