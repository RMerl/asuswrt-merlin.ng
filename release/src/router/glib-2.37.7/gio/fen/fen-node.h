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

#include <port.h>
#include <gio/gio.h>

#ifndef _FEN_NODE_H_
#define _FEN_NODE_H_

#ifdef GIO_COMPILATION
#define FN_EVENT_CREATED G_FILE_MONITOR_EVENT_CREATED
#else
#define FN_EVENT_CREATED GAMIN_EVENT_CREATED
#endif

#define NODE_STATE_NONE       0x00000000
#define NODE_STATE_ASSOCIATED 0x00000001 /* This is a confilct to NODE_FLAG_STAT_DONE */
#define NODE_STATE_HAS_EVENTS 0x00000002

#define NODE_FLAG_NONE             0x00000000
#define NODE_FLAG_SNAPSHOT_UPDATED 0x00000001
#define NODE_FLAG_DIR              0x00000002
#define NODE_FLAG_STAT_UPDATED     0x00000004

#define	NODE_CLE_STATE(f, st)  (f->state &= ~(st))
#define	NODE_SET_STATE(f, st)  (f->state = ((f->state & ~(st)) | (st)))
#define	NODE_HAS_STATE(f, st)  (f->state & (st))

#define	NODE_CLE_FLAG(f, fl)  (f->flag &= ~(fl))
#define	NODE_SET_FLAG(f, fl)  (f->flag = ((f->flag & ~(fl)) | (fl)))
#define	NODE_HAS_FLAG(f, fl)  (f->flag & (fl))

typedef struct node node_t;
struct node
{
    file_obj_t  fobj;           /* Inherit from file_obj_t, must be the first. */
    GSource    *source;
    gchar      *basename;
    guint32     state;
	guint32     flag;
    GTimeVal    atv;            /* Timestamp for the first added sub. */

	/* the parent and children of node */
    node_t *parent;
    GHashTable *children; /* children in basename */

	/* List of subscriptions monitoring this fdata/path */
	GList *subs;
	GList *dir_subs;

#ifdef GIO_COMPILATION
    GFile* gfile;
#endif
};

#define FILE_OBJECT(f)                ((file_obj_t *)(f))
#define NODE_NAME(f)                  (FILE_OBJECT(f)->fo_name)
#define NODE_PARENT(f)                (((node_t *)f)->parent)
#define	NODE_IS_ACTIVE(f)             (f->dir_subs || f->subs)
#define	NODE_IS_REQUIRED_BY_PARENT(f) (NODE_PARENT(f) && NODE_PARENT(f)->dir_subs)

gboolean node_timeval_lt(const GTimeVal *val1, const GTimeVal *val2);
gboolean node_try_delete(node_t* node);
void     node_traverse(node_t* node, void(*traverse_cb)(node_t*, gpointer), gpointer user_data);
node_t*  node_find(node_t* node, const gchar* filename, gboolean create_on_missing);
gint     node_lstat(node_t *f);
void     node_create_children_snapshot(node_t *f, gint created_event, gboolean emit);
void     node_adjust_deleted(node_t *f);
gboolean node_class_init();

typedef struct node_event
{
    int e;
    gpointer user_data;
    gpointer pair_data;
    GTimeVal ctv;               /* Created timestamp */
    GTimeVal rename_tv;         /* Possible rename timestamp */
} node_event_t;

node_event_t* node_event_new (int event, gpointer user_data);
void node_event_delete (node_event_t* ev);

#endif /* _FEN_NODE_H_ */
