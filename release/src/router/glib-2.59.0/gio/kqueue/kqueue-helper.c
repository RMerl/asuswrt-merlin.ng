/*******************************************************************************
  Copyright (c) 2011, 2012 Dmitry Matveev <me@dmitrymatveev.co.uk>

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*******************************************************************************/

#include "config.h"
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <gio/glocalfile.h>
#include <gio/glocalfilemonitor.h>
#include <gio/gfile.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include "kqueue-helper.h"

typedef struct {
  kqueue_sub *sub;
  GFileMonitorSource *source;
  gboolean handle_deleted;
} handle_ctx;

/**
 * handle_created: 
 * @udata: a pointer to user data (#handle_context).
 * @path: file name of a new file.
 * @inode: inode number of a new file.
 *
 * A callback function for the directory diff calculation routine,
 * produces G_FILE_MONITOR_EVENT_CREATED event for a created file.
 **/
static void
handle_created (void *udata, const char *path, ino_t inode)
{
  handle_ctx *ctx = NULL;
  gint64 now;
  gchar *fullname;
  struct stat st;

  (void) inode;
  ctx = (handle_ctx *) udata;
  g_assert (udata != NULL);
  g_assert (ctx->sub != NULL);
  g_assert (ctx->source != NULL);

  now = g_get_monotonic_time ();
  g_file_monitor_source_handle_event (ctx->source, G_FILE_MONITOR_EVENT_CREATED, path,
                                      NULL, NULL, now);

  /* Copied from ih_event_callback to report 'CHANGES_DONE_HINT' earlier. */
  fullname = g_build_filename (ctx->sub->filename, path, NULL);
  if (stat (fullname, &st) != 0 || !S_ISREG (st.st_mode) || st.st_nlink != 1)
    g_file_monitor_source_handle_event (ctx->source, G_FILE_MONITOR_EVENT_CHANGES_DONE_HINT, path,
                                        NULL, NULL, now);
  g_free (fullname);
}

/**
 * handle_deleted:
 * @udata: a pointer to user data (#handle_context).
 * @path: file name of the removed file.
 * @inode: inode number of the removed file.
 *
 * A callback function for the directory diff calculation routine,
 * produces G_FILE_MONITOR_EVENT_DELETED event for a deleted file.
 **/
static void
handle_deleted (void *udata, const char *path, ino_t inode)
{
  handle_ctx *ctx = NULL;

  (void) inode;
  ctx = (handle_ctx *) udata;
  g_assert (udata != NULL);
  g_assert (ctx->sub != NULL);
  g_assert (ctx->source != NULL);

  if (!ctx->handle_deleted)
    return;

  g_file_monitor_source_handle_event (ctx->source, G_FILE_MONITOR_EVENT_DELETED, path,
                                      NULL, NULL, g_get_monotonic_time ());
}

/**
 * handle_moved:
 * @udata: a pointer to user data (#handle_context).
 * @from_path: file name of the source file.
 * @from_inode: inode number of the source file.
 * @to_path: file name of the replaced file.
 * @to_inode: inode number of the replaced file.
 *
 * A callback function for the directory diff calculation routine,
 * produces G_FILE_MONITOR_EVENT_RENAMED event on a move.
 **/
static void
handle_moved (void       *udata,
              const char *from_path,
              ino_t       from_inode,
              const char *to_path,
              ino_t       to_inode)
{
  handle_ctx *ctx = NULL;

  (void) from_inode;
  (void) to_inode;

  ctx = (handle_ctx *) udata;
  g_assert (udata != NULL);
  g_assert (ctx->sub != NULL);
  g_assert (ctx->source != NULL);

  g_file_monitor_source_handle_event (ctx->source, G_FILE_MONITOR_EVENT_RENAMED,
                                      from_path, to_path, NULL, g_get_monotonic_time ());
}

/**
 * handle_overwritten:
 * @data: a pointer to user data (#handle_context).
 * @path: file name of the overwritten file.
 * @node: inode number of the overwritten file.
 *
 * A callback function for the directory diff calculation routine,
 * produces G_FILE_MONITOR_EVENT_DELETED/CREATED event pair when
 * an overwrite occurs in the directory (see dep-list for details).
 **/
static void
handle_overwritten (void *udata, const char *path, ino_t inode)
{
  handle_ctx *ctx = NULL;

  (void) inode;
  ctx = (handle_ctx *) udata;
  g_assert (udata != NULL);
  g_assert (ctx->sub != NULL);
  g_assert (ctx->source != NULL);

  g_file_monitor_source_handle_event (ctx->source, G_FILE_MONITOR_EVENT_DELETED,
                                      path, NULL, NULL, g_get_monotonic_time ());

  g_file_monitor_source_handle_event (ctx->source, G_FILE_MONITOR_EVENT_CREATED,
                                      path, NULL, NULL, g_get_monotonic_time ());
}

static const traverse_cbs cbs = {
  handle_created,
  handle_deleted,
  handle_moved,
  handle_overwritten,
  handle_moved,
  NULL, /* many added */
  NULL, /* many removed */
  NULL, /* names updated */
};


void
_kh_dir_diff (kqueue_sub *sub, gboolean handle_deleted)
{
  dep_list *was;
  handle_ctx ctx;

  memset (&ctx, 0, sizeof (handle_ctx));
  ctx.sub = sub;
  ctx.source = sub->source;
  ctx.handle_deleted = handle_deleted;

  was = sub->deps;
  sub->deps = dl_listing (sub->filename);

  dl_calculate (was, sub->deps, &cbs, &ctx);

  dl_free (was);
}


