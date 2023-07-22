/* 
 * Copyright (C) 2011 Red Hat, Inc.
 *
 * This work is provided "as is"; redistribution and modification
 * in whole or in part, in any medium, physical or electronic is
 * permitted without restriction.
 *
 * This work is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * In no event shall the authors or contributors be liable for any
 * direct, indirect, incidental, special, exemplary, or consequential
 * damages (including, but not limited to, procurement of substitute
 * goods or services; loss of use, data, or profits; or business
 * interruption) however caused and on any theory of liability, whether
 * in contract, strict liability, or tort (including negligence or
 * otherwise) arising in any way out of the use of this software, even
 * if advised of the possibility of such damage.
 *
 * Author: Colin Walters <walters@verbum.org> 
 */

#include "config.h"

#include <glib.h>
#include <string.h>
#include <fcntl.h>

#ifdef G_OS_UNIX
#include <glib-unix.h>
#endif

#ifdef G_OS_WIN32
#include <io.h>
#define LINEEND "\r\n"
#else
#define LINEEND "\n"
#endif

/* MinGW builds are likely done using a BASH-style shell, so run the
 * normal script there, as on non-Windows builds, as it is more likely
 * that one will run 'make check' in such shells to test the code
 */
#if defined (G_OS_WIN32) && defined (_MSC_VER)
#define SCRIPT_EXT ".bat"
#else
#define SCRIPT_EXT
#endif

static char *echo_prog_path;
static char *echo_script_path;

typedef struct {
  GMainLoop *loop;
  gboolean child_exited;
  gboolean stdout_done;
  GString *stdout_buf;
} SpawnAsyncMultithreadedData;

static gboolean
on_child_exited (GPid     pid,
		 gint     status,
		 gpointer datap)
{
  SpawnAsyncMultithreadedData *data = datap;

  data->child_exited = TRUE;
  if (data->child_exited && data->stdout_done)
    g_main_loop_quit (data->loop);
  
  return G_SOURCE_REMOVE;
}

static gboolean
on_child_stdout (GIOChannel   *channel,
		 GIOCondition  condition,
		 gpointer      datap)
{
  char buf[1024];
  GError *error = NULL;
  gsize bytes_read;
  SpawnAsyncMultithreadedData *data = datap;

  if (condition & G_IO_IN)
    {
      GIOStatus status;
      status = g_io_channel_read_chars (channel, buf, sizeof (buf), &bytes_read, &error);
      g_assert_no_error (error);
      g_string_append_len (data->stdout_buf, buf, (gssize) bytes_read);
      if (status == G_IO_STATUS_EOF)
	data->stdout_done = TRUE;
    }
  if (condition & G_IO_HUP)
    data->stdout_done = TRUE;
  if (condition & G_IO_ERR)
    g_error ("Error reading from child stdin");

  if (data->child_exited && data->stdout_done)
    g_main_loop_quit (data->loop);

  return !data->stdout_done;
}

static void
test_spawn_async (void)
{
  int tnum = 1;
  GError *error = NULL;
  GPtrArray *argv;
  char *arg;
  GPid pid;
  GMainContext *context;
  GMainLoop *loop;
  GIOChannel *channel;
  GSource *source;
  int child_stdout_fd;
  SpawnAsyncMultithreadedData data;

  context = g_main_context_new ();
  loop = g_main_loop_new (context, TRUE);

  arg = g_strdup_printf ("thread %d", tnum);

  argv = g_ptr_array_new ();
  g_ptr_array_add (argv, echo_prog_path);
  g_ptr_array_add (argv, arg);
  g_ptr_array_add (argv, NULL);

  g_spawn_async_with_pipes (NULL, (char**)argv->pdata, NULL, G_SPAWN_DO_NOT_REAP_CHILD, NULL, NULL, &pid, NULL,
			    &child_stdout_fd, NULL, &error);
  g_assert_no_error (error);
  g_ptr_array_free (argv, TRUE);

  data.loop = loop;
  data.stdout_done = FALSE;
  data.child_exited = FALSE;
  data.stdout_buf = g_string_new (0);

  source = g_child_watch_source_new (pid);
  g_source_set_callback (source, (GSourceFunc)on_child_exited, &data, NULL);
  g_source_attach (source, context);
  g_source_unref (source);

  channel = g_io_channel_unix_new (child_stdout_fd);
  source = g_io_create_watch (channel, G_IO_IN | G_IO_HUP | G_IO_ERR);
  g_source_set_callback (source, (GSourceFunc)on_child_stdout, &data, NULL);
  g_source_attach (source, context);
  g_source_unref (source);

  g_main_loop_run (loop);

  g_assert (data.child_exited);
  g_assert (data.stdout_done);
  g_assert_cmpstr (data.stdout_buf->str, ==, arg);
  g_string_free (data.stdout_buf, TRUE);

  g_io_channel_unref (channel);
  g_main_context_unref (context);
  g_main_loop_unref (loop);

  g_free (arg);
}

/* Windows close() causes failure through the Invalid Parameter Handler
 * Routine if the file descriptor does not exist.
 */
static void
sane_close (int fd)
{
  if (fd >= 0)
    close (fd);
}

/* Test g_spawn_async_with_fds() with a variety of different inputs */
static void
test_spawn_async_with_fds (void)
{
  int tnum = 1;
  GPtrArray *argv;
  char *arg;
  int i;

  /* Each test has 3 variable parameters: stdin, stdout, stderr */
  enum fd_type {
    NO_FD,        /* pass fd -1 (unset) */
    FD_NEGATIVE,  /* pass fd of negative value (equivalent to unset) */
    PIPE,         /* pass fd of new/unique pipe */
    STDOUT_PIPE,  /* pass the same pipe as stdout */
  } tests[][3] = {
    { NO_FD, NO_FD, NO_FD },       /* Test with no fds passed */
    { NO_FD, FD_NEGATIVE, NO_FD }, /* Test another negative fd value */
    { PIPE, PIPE, PIPE },          /* Test with unique fds passed */
    { NO_FD, PIPE, STDOUT_PIPE },  /* Test the same fd for stdout + stderr */
  };

  arg = g_strdup_printf ("thread %d", tnum);

  argv = g_ptr_array_new ();
  g_ptr_array_add (argv, echo_prog_path);
  g_ptr_array_add (argv, arg);
  g_ptr_array_add (argv, NULL);

  for (i = 0; i < G_N_ELEMENTS (tests); i++)
    {
      GError *error = NULL;
      GPid pid;
      GMainContext *context;
      GMainLoop *loop;
      GIOChannel *channel = NULL;
      GSource *source;
      SpawnAsyncMultithreadedData data;
      enum fd_type *fd_info = tests[i];
      gint test_pipe[3][2];
      int j;

      for (j = 0; j < 3; j++)
        {
          switch (fd_info[j])
            {
            case NO_FD:
              test_pipe[j][0] = -1;
              test_pipe[j][1] = -1;
              break;
            case FD_NEGATIVE:
              test_pipe[j][0] = -5;
              test_pipe[j][1] = -5;
              break;
            case PIPE:
#ifdef G_OS_UNIX
              g_unix_open_pipe (test_pipe[j], FD_CLOEXEC, &error);
              g_assert_no_error (error);
#else
              g_assert_cmpint (_pipe (test_pipe[j], 4096, _O_BINARY), >=, 0);
#endif
              break;
            case STDOUT_PIPE:
              g_assert_cmpint (j, ==, 2); /* only works for stderr */
              test_pipe[j][0] = test_pipe[1][0];
              test_pipe[j][1] = test_pipe[1][1];
              break;
            default:
              g_assert_not_reached ();
            }
        }

      context = g_main_context_new ();
      loop = g_main_loop_new (context, TRUE);

      g_spawn_async_with_fds (NULL, (char**)argv->pdata, NULL,
			      G_SPAWN_DO_NOT_REAP_CHILD, NULL, NULL, &pid,
			      test_pipe[0][0], test_pipe[1][1], test_pipe[2][1],
			      &error);
      g_assert_no_error (error);
      sane_close (test_pipe[0][0]);
      sane_close (test_pipe[1][1]);
      if (fd_info[2] != STDOUT_PIPE)
        sane_close (test_pipe[2][1]);

      data.loop = loop;
      data.stdout_done = FALSE;
      data.child_exited = FALSE;
      data.stdout_buf = g_string_new (0);

      source = g_child_watch_source_new (pid);
      g_source_set_callback (source, (GSourceFunc)on_child_exited, &data, NULL);
      g_source_attach (source, context);
      g_source_unref (source);

      if (test_pipe[1][0] >= 0)
        {
          channel = g_io_channel_unix_new (test_pipe[1][0]);
          source = g_io_create_watch (channel, G_IO_IN | G_IO_HUP | G_IO_ERR);
          g_source_set_callback (source, (GSourceFunc)on_child_stdout,
                                 &data, NULL);
          g_source_attach (source, context);
          g_source_unref (source);
        }
      else
        {
          /* Don't check stdout data if we didn't pass a fd */
          data.stdout_done = TRUE;
        }

      g_main_loop_run (loop);

      g_assert_true (data.child_exited);

      if (test_pipe[1][0] >= 0)
        {
          /* Check for echo on stdout */
          g_assert_true (data.stdout_done);
          g_assert_cmpstr (data.stdout_buf->str, ==, arg);
          g_io_channel_unref (channel);
        }
      g_string_free (data.stdout_buf, TRUE);

      g_main_context_unref (context);
      g_main_loop_unref (loop);
      sane_close (test_pipe[0][1]);
      sane_close (test_pipe[1][0]);
      if (fd_info[2] != STDOUT_PIPE)
        sane_close (test_pipe[2][0]);
    }

  g_ptr_array_free (argv, TRUE);
  g_free (arg);
}

static void
test_spawn_sync (void)
{
  int tnum = 1;
  GError *error = NULL;
  GPtrArray *argv;
  char *arg;
  char *stdout_str;
  int estatus;

  arg = g_strdup_printf ("thread %d", tnum);

  argv = g_ptr_array_new ();
  g_ptr_array_add (argv, echo_prog_path);
  g_ptr_array_add (argv, arg);
  g_ptr_array_add (argv, NULL);

  g_spawn_sync (NULL, (char**)argv->pdata, NULL, 0, NULL, NULL, &stdout_str, NULL, &estatus, &error);
  g_assert_no_error (error);
  g_assert_cmpstr (arg, ==, stdout_str);
  g_free (arg);
  g_free (stdout_str);
  g_ptr_array_free (argv, TRUE);
}

/* Like test_spawn_sync but uses spawn flags that trigger the optimized
 * posix_spawn codepath.
 */
static void
test_posix_spawn (void)
{
  int tnum = 1;
  GError *error = NULL;
  GPtrArray *argv;
  char *arg;
  char *stdout_str;
  int estatus;
  GSpawnFlags flags = G_SPAWN_CLOEXEC_PIPES | G_SPAWN_LEAVE_DESCRIPTORS_OPEN;

  arg = g_strdup_printf ("thread %d", tnum);

  argv = g_ptr_array_new ();
  g_ptr_array_add (argv, echo_prog_path);
  g_ptr_array_add (argv, arg);
  g_ptr_array_add (argv, NULL);

  g_spawn_sync (NULL, (char**)argv->pdata, NULL, flags, NULL, NULL, &stdout_str, NULL, &estatus, &error);
  g_assert_no_error (error);
  g_assert_cmpstr (arg, ==, stdout_str);
  g_free (arg);
  g_free (stdout_str);
  g_ptr_array_free (argv, TRUE);
}

static void
test_spawn_script (void)
{
  GError *error = NULL;
  GPtrArray *argv;
  char *stdout_str;
  int estatus;

  argv = g_ptr_array_new ();
  g_ptr_array_add (argv, echo_script_path);
  g_ptr_array_add (argv, NULL);

  g_spawn_sync (NULL, (char**)argv->pdata, NULL, 0, NULL, NULL, &stdout_str, NULL, &estatus, &error);
  g_assert_no_error (error);
  g_assert_cmpstr ("echo" LINEEND, ==, stdout_str);
  g_free (stdout_str);
  g_ptr_array_free (argv, TRUE);
}

/* Test that spawning a non-existent executable returns %G_SPAWN_ERROR_NOENT. */
static void
test_spawn_nonexistent (void)
{
  GError *error = NULL;
  GPtrArray *argv = NULL;
  gchar *stdout_str = NULL;
  gint exit_status = -1;

  argv = g_ptr_array_new ();
  g_ptr_array_add (argv, "this does not exist");
  g_ptr_array_add (argv, NULL);

  g_spawn_sync (NULL, (char**) argv->pdata, NULL, 0, NULL, NULL, &stdout_str,
                NULL, &exit_status, &error);
  g_assert_error (error, G_SPAWN_ERROR, G_SPAWN_ERROR_NOENT);
  g_assert_null (stdout_str);
  g_assert_cmpint (exit_status, ==, -1);

  g_ptr_array_free (argv, TRUE);

  g_clear_error (&error);
}

int
main (int   argc,
      char *argv[])
{
  char *dirname;
  int ret;

  g_test_init (&argc, &argv, NULL);

  dirname = g_path_get_dirname (argv[0]);
  echo_prog_path = g_build_filename (dirname, "test-spawn-echo" EXEEXT, NULL);
  if (!g_file_test (echo_prog_path, G_FILE_TEST_EXISTS))
    {
      g_free (echo_prog_path);
      echo_prog_path = g_build_filename (dirname, "lt-test-spawn-echo" EXEEXT, NULL);
    }
  echo_script_path = g_build_filename (dirname, "echo-script" SCRIPT_EXT, NULL);
  if (!g_file_test (echo_script_path, G_FILE_TEST_EXISTS))
    {
      g_free (echo_script_path);
      echo_script_path = g_test_build_filename (G_TEST_DIST, "echo-script" SCRIPT_EXT, NULL);
    }
  g_free (dirname);

  g_assert (g_file_test (echo_prog_path, G_FILE_TEST_EXISTS));
  g_assert (g_file_test (echo_script_path, G_FILE_TEST_EXISTS));

  g_test_add_func ("/gthread/spawn-single-sync", test_spawn_sync);
  g_test_add_func ("/gthread/spawn-single-async", test_spawn_async);
  g_test_add_func ("/gthread/spawn-single-async-with-fds", test_spawn_async_with_fds);
  g_test_add_func ("/gthread/spawn-script", test_spawn_script);
  g_test_add_func ("/gthread/spawn/nonexistent", test_spawn_nonexistent);
  g_test_add_func ("/gthread/spawn-posix-spawn", test_posix_spawn);

  ret = g_test_run();

  g_free (echo_script_path);
  g_free (echo_prog_path);

  return ret;
}
