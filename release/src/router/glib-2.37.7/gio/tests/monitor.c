#include <gio/gio.h>

typedef struct {
  GFile *file;
  GOutputStream *stream;
  GMainLoop *loop;
  gint state;
} MonitorData;

static gboolean
create_file_idle (gpointer data)
{
  MonitorData *d = data;
  GError *error = NULL;

  g_assert (d->state == 0);

  d->stream = (GOutputStream*)g_file_create (d->file, 0, NULL, &error);
  g_assert_no_error (error);

  d->state = 1;

  return G_SOURCE_REMOVE;
}

static gboolean
write_file_idle (gpointer data)
{
  MonitorData *d = data;
  GError *error = NULL;

  g_assert (d->state == 2);

  g_output_stream_write (d->stream, "abcd", 4, NULL, &error);
  g_assert_no_error (error);
  g_object_unref (d->stream);
  d->stream = NULL;

  d->state = 3;


  return G_SOURCE_REMOVE;
}

static gboolean
delete_file_idle (gpointer data)
{
  MonitorData *d = data;
  GError *error = NULL;

  g_assert (d->state == 4);

  g_file_delete (d->file, NULL, &error);
  g_assert_no_error (error);

  d->state = 5;

  return G_SOURCE_REMOVE;
}

static void
changed_cb (GFileMonitor      *monitor,
            GFile             *file,
            GFile             *other_file,
            GFileMonitorEvent  event,
            gpointer           data)
{
  MonitorData *d = data;

  switch (d->state)
    {
    case 1:
      g_assert (event == G_FILE_MONITOR_EVENT_CREATED);
      d->state = 2;
      g_idle_add (write_file_idle, data);
      break;
    case 3:
      g_assert (event == G_FILE_MONITOR_EVENT_CHANGED ||
                event == G_FILE_MONITOR_EVENT_CHANGES_DONE_HINT);
      if (event == G_FILE_MONITOR_EVENT_CHANGES_DONE_HINT)
        {
          d->state = 4;
          g_idle_add (delete_file_idle, data);
        }
      break;
    case 5:
      g_assert (event == G_FILE_MONITOR_EVENT_DELETED);
      d->state = 6;
      if (d->loop)
        g_main_loop_quit (d->loop);
      break;
    default:
      g_assert_not_reached ();
      break;
    }
}

static void
file_changed_cb (GFileMonitor      *monitor,
                 GFile             *file,
                 GFile             *other_file,
                 GFileMonitorEvent  event,
                 gpointer           data)
{
  gint *state = data;

  switch (*state)
    {
    case 0:
      g_assert (event == G_FILE_MONITOR_EVENT_CREATED);
      *state = 1;
      break;
    case 1:
      g_assert (event == G_FILE_MONITOR_EVENT_CHANGED ||
                event == G_FILE_MONITOR_EVENT_CHANGES_DONE_HINT);
      if (event == G_FILE_MONITOR_EVENT_CHANGES_DONE_HINT)
        *state = 2;
      break;
    case 2:
      g_assert (event == G_FILE_MONITOR_EVENT_DELETED);
      *state = 3;
      break;
    default:
      g_assert_not_reached ();
      break;
    }
}

static void
test_directory_monitor (void)
{
  gchar *path;
  GFile *file;
  GFile *child;
  GFileMonitor *dir_monitor;
  GFileMonitor *file_monitor;
  GError *error = NULL;
  MonitorData data;
  gint state;
  GMainLoop *loop;

  path = g_mkdtemp (g_strdup ("file_monitor_XXXXXX"));
  file = g_file_new_for_path (path);
  dir_monitor = g_file_monitor_directory (file, 0, NULL, &error);
  g_assert_no_error (error);

  child = g_file_get_child (file, "test-file");
  file_monitor = g_file_monitor_file (child, 0, NULL, &error);
  g_assert_no_error (error);

  loop = g_main_loop_new (NULL, FALSE);

  g_signal_connect (dir_monitor, "changed", G_CALLBACK (changed_cb), &data);
  g_signal_connect (file_monitor, "changed", G_CALLBACK (file_changed_cb), &state);

  data.loop = loop;
  data.file = child;
  data.state = 0;
  state = 0;

  g_idle_add (create_file_idle, &data);

  g_main_loop_run (loop);

  g_assert_cmpint (data.state, ==, 6);
  g_assert_cmpint (state, ==, 3);

  g_main_loop_unref (loop);
  g_object_unref (dir_monitor);
  g_object_unref (file_monitor);
  g_object_unref (child);
  g_object_unref (file);
  g_free (path);
}

int
main (int argc, char *argv[])
{
  g_test_init (&argc, &argv, NULL);

  g_test_add_func ("/monitor/directory", test_directory_monitor);

  return g_test_run ();
}
