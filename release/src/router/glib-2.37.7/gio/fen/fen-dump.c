/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* vim:set expandtab ts=4 shiftwidth=4: */
/* 
 * Copyright (c) 2008, 2010 Oracle and/or its affiliates, Inc. All rights
 * reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Authors: Lin Ma <lin.ma@sun.com>
 */

#include "config.h"
#include <glib.h>
#include <glib/gprintf.h>
#include "fen-node.h"
#include "fen-dump.h"

G_LOCK_EXTERN (fen_lock);

/*-------------------- node ------------------*/
static void
dump_node (node_t* node, gpointer data)
{
    g_printf ("n:0x%p ds:0x%p s:0x%p %s\n", node, node->dir_subs, node->subs, NODE_NAME(node));
}

static void
dump_tree (node_t* node)
{
    if (G_TRYLOCK (fen_lock)) {
        node_traverse(NULL, dump_node, NULL);
        G_UNLOCK (fen_lock);
    }
}

/* ------------------ fdata port hash --------------------*/
void
dump_hash_cb (gpointer key,
  gpointer value,
  gpointer user_data)
{
    g_printf ("k:0x%p v:0x%p >\n", key, value);
}

gboolean
dump_hash (GHashTable* hash, gpointer user_data)
{
    if (G_TRYLOCK (fen_lock)) {
        if (g_hash_table_size (hash) > 0) {
            g_hash_table_foreach (hash, dump_hash_cb, user_data);
        }
        G_UNLOCK (fen_lock);
    }
    return TRUE;
}

/* ------------------ event --------------------*/
void
dump_event (node_event_t* ev, gpointer user_data)
{
    node_t* node = ev->user_data;
    g_printf ("ne:0x%p e:%p n:0x%p ds:0x%p s:0x%p s\n", ev, ev->e, node, node->dir_subs, node->subs, NODE_NAME(node));
}
