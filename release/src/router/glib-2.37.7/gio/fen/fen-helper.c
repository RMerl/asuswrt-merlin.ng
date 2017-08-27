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
#include "fen-helper.h"
#include "fen-kernel.h"
#ifdef GIO_COMPILATION
#include <gio/gfilemonitor.h>
#else
#include "gam_event.h"
#include "gam_server.h"
#include "gam_protocol.h"
#endif

#ifdef GIO_COMPILATION
#define FH_W if (FALSE) g_debug
#else
#include "gam_error.h"
#define FH_W(...) GAM_DEBUG(DEBUG_INFO, __VA_ARGS__)
#endif

G_LOCK_EXTERN (fen_lock);

/* misc */
static void
scan_children_init(node_t *f, gpointer sub)
{
    gboolean emit;
    gint event;

    FH_W ("%s %s [0x%p]\n", __func__, NODE_NAME(f), f);

#ifdef GIO_COMPILATION
    emit = FALSE;
    event = G_FILE_MONITOR_EVENT_CREATED;
#else
    emit = TRUE;
    event = GAMIN_EVENT_EXISTS;
#endif

    if (!NODE_HAS_FLAG(f, NODE_FLAG_SNAPSHOT_UPDATED)) {
        /* TODO snapshot should also compare to the sub created timestamp. */
        /* GIO initially doesn't emit created/existed events. */
        node_create_children_snapshot(f, event, emit);
    } else {
        GHashTableIter iter;
        gpointer value;

        g_hash_table_iter_init (&iter, f->children);
        while (g_hash_table_iter_next (&iter, NULL, &value)) {
            node_t *child = (node_t *)value;

#ifdef GIO_COMPILATION
            /* GIO initially doesn't emit created/existed events. */
            /* g_file_monitor_emit_event(G_FILE_MONITOR(sub), child->gfile, NULL, event); */
#else
            gam_server_emit_one_event(NODE_NAME(child), gam_subscription_is_dir(sub), event, sub, 1);
#endif
        }
    }
}

/**
 * fen_add
 * 
 * Won't hold a ref, we have a timout callback to clean unused node_t.
 * If there is no value for a key, add it and return it; else return the old
 * one.
 */
void
fen_add (const gchar *filename, gpointer sub, gboolean is_mondir)
{
	node_t* f;

    g_assert (filename);
    g_assert (sub);

    G_LOCK (fen_lock);
	f = node_find(NULL, filename, TRUE);
    FH_W ("%s 0x%p sub[0x%p] %s\n", __func__, f, sub, filename);
    g_assert (f);

    /* Update timestamp, the events in global queue will compare itself to this
     * timestamp to decide if be emitted. TODO, timestamp should be per sub.
     */
    if (!NODE_IS_ACTIVE(f)) {
        g_get_current_time(&f->atv);
    }

    if (is_mondir) {
        f->dir_subs = g_list_prepend(f->dir_subs, sub);
    } else {
        f->subs = g_list_prepend(f->subs, sub);
    }
    
    if (NODE_HAS_STATE(f, NODE_STATE_ASSOCIATED) ||
      (node_lstat(f) == 0 && port_add(f) == 0)) {
#ifndef GIO_COMPILATION
        gam_server_emit_one_event (NODE_NAME(f),
          gam_subscription_is_dir (sub), GAMIN_EVENT_EXISTS, sub, 1);
#endif
        if (is_mondir) {
            scan_children_init (f, sub);
        }
    } else {
#ifndef GIO_COMPILATION
        gam_server_emit_one_event (NODE_NAME(f),
          gam_subscription_is_dir (sub), GAMIN_EVENT_DELETED, sub, 1);
#endif
        node_adjust_deleted (f);
    }
#ifndef GIO_COMPILATION
    gam_server_emit_one_event (NODE_NAME(f),
      gam_subscription_is_dir (sub), GAMIN_EVENT_ENDEXISTS, sub, 1);
#endif
    G_UNLOCK (fen_lock);
}

void
fen_remove (const gchar *filename, gpointer sub, gboolean is_mondir)
{
    node_t* f;
    
    g_assert (filename);
    g_assert (sub);

    G_LOCK (fen_lock);
	f = node_find(NULL, filename, FALSE);
    FH_W ("%s 0x%p sub[0x%p] %s\n", __func__, f, sub, filename);

    if (f) {
        if (is_mondir) {
            f->dir_subs = g_list_remove(f->dir_subs, sub);
        } else {
            f->subs = g_list_remove(f->subs, sub);
        }

        if (!NODE_IS_ACTIVE(f)) {
            node_try_delete (f);
        }
    }
    G_UNLOCK (fen_lock);
}

/**
 * fen_init:
 * 
 * FEN subsystem initializing.
 */
gboolean
fen_init ()
{
    static gboolean initialized = FALSE;
    static gboolean result = FALSE;

    G_LOCK (fen_lock);
    if (initialized) {
        G_UNLOCK (fen_lock);
        return result;
    }

    result = node_class_init();

    if (!result) {
        G_UNLOCK (fen_lock);
        return result;
    }

    initialized = TRUE;

    G_UNLOCK (fen_lock);
    return result;
}
