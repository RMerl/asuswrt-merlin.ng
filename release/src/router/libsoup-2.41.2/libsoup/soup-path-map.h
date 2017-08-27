/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 2007 Novell, Inc.
 */

#ifndef SOUP_PATH_MAP_H
#define SOUP_PATH_MAP_H 1

#include <libsoup/soup-types.h>

typedef struct SoupPathMap SoupPathMap;

SoupPathMap *soup_path_map_new    (GDestroyNotify  data_free_func);
void         soup_path_map_free   (SoupPathMap    *map);

void         soup_path_map_add    (SoupPathMap    *map,
				   const char     *path,
				   gpointer        data);
void         soup_path_map_remove (SoupPathMap    *map,
				   const char     *path);

gpointer     soup_path_map_lookup (SoupPathMap    *map,
				   const char     *path);


#endif /* SOUP_PATH_MAP_H */
