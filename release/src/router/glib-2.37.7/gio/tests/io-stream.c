/* GLib testing framework examples and tests
 * Copyright (C) 2010 Collabora Ltd.
 * Authors: Xavier Claessens <xclaesse@gmail.com>
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
 */

#include <glib/glib.h>
#include <gio/gio.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
  GIOStream parent;
  GInputStream *input_stream;
  GOutputStream *output_stream;
} GTestIOStream;

typedef struct
{
  GIOStreamClass parent_class;
} GTestIOStreamClass;

static GType g_test_io_stream_get_type (void);
G_DEFINE_TYPE (GTestIOStream, g_test_io_stream, G_TYPE_IO_STREAM);


static GInputStream *
get_input_stream (GIOStream *io_stream)
{
  GTestIOStream *self =  (GTestIOStream *) io_stream;

  return self->input_stream;
}

static GOutputStream *
get_output_stream (GIOStream *io_stream)
{
  GTestIOStream *self =  (GTestIOStream *) io_stream;

  return self->output_stream;
}

static void
finalize (GObject *object)
{
  GTestIOStream *self = (GTestIOStream *) object;

  if (self->input_stream != NULL)
    g_object_unref (self->input_stream);

  if (self->output_stream != NULL)
    g_object_unref (self->output_stream);

  G_OBJECT_CLASS (g_test_io_stream_parent_class)->finalize (object);
}

static void
g_test_io_stream_class_init (GTestIOStreamClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GIOStreamClass *io_class = G_IO_STREAM_CLASS (klass);

  object_class->finalize = finalize;

  io_class->get_input_stream = get_input_stream;
  io_class->get_output_stream = get_output_stream;
}

static void
g_test_io_stream_init (GTestIOStream *self)
{
}

static GIOStream *
g_test_io_stream_new (GInputStream *input, GOutputStream *output)
{
  GTestIOStream *self;

  self = g_object_new (g_test_io_stream_get_type (), NULL);
  self->input_stream = g_object_ref (input);
  self->output_stream = g_object_ref (output);

  return G_IO_STREAM (self);
}

typedef struct
{
  GMainLoop *main_loop;
  const gchar *data1;
  const gchar *data2;
  GIOStream *iostream1;
  GIOStream *iostream2;
} TestCopyChunksData;

static void
test_copy_chunks_splice_cb (GObject *source_object,
    GAsyncResult *res,
    gpointer user_data)
{
  TestCopyChunksData *data = user_data;
  GMemoryOutputStream *ostream;
  gchar *received_data;
  GError *error = NULL;

  g_io_stream_splice_finish (res, &error);
  g_assert_no_error (error);

  ostream = G_MEMORY_OUTPUT_STREAM (((GTestIOStream *) data->iostream1)->output_stream);
  received_data = g_memory_output_stream_get_data (ostream);
  g_assert_cmpstr (received_data, ==, data->data2);

  ostream = G_MEMORY_OUTPUT_STREAM (((GTestIOStream *) data->iostream2)->output_stream);
  received_data = g_memory_output_stream_get_data (ostream);
  g_assert_cmpstr (received_data, ==, data->data1);

  g_assert (g_io_stream_is_closed (data->iostream1));
  g_assert (g_io_stream_is_closed (data->iostream2));

  g_main_loop_quit (data->main_loop);
}

static void
test_copy_chunks (void)
{
  TestCopyChunksData data;
  GInputStream *istream;
  GOutputStream *ostream;

  data.main_loop = g_main_loop_new (NULL, FALSE);
  data.data1 = "abcdefghijklmnopqrstuvwxyz";
  data.data2 = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

  istream = g_memory_input_stream_new_from_data (data.data1, -1, NULL);
  ostream = g_memory_output_stream_new (NULL, 0, g_realloc, g_free);
  data.iostream1 = g_test_io_stream_new (istream, ostream);
  g_object_unref (istream);
  g_object_unref (ostream);

  istream = g_memory_input_stream_new_from_data (data.data2, -1, NULL);
  ostream = g_memory_output_stream_new (NULL, 0, g_realloc, g_free);
  data.iostream2 = g_test_io_stream_new (istream, ostream);
  g_object_unref (istream);
  g_object_unref (ostream);

  g_io_stream_splice_async (data.iostream1, data.iostream2,
      G_IO_STREAM_SPLICE_CLOSE_STREAM1 | G_IO_STREAM_SPLICE_CLOSE_STREAM2 |
      G_IO_STREAM_SPLICE_WAIT_FOR_BOTH, G_PRIORITY_DEFAULT,
      NULL, test_copy_chunks_splice_cb, &data);

  /* We do not hold a ref in data struct, this is to make sure the operation
   * keeps the iostream objects alive until it finishes */
  g_object_unref (data.iostream1);
  g_object_unref (data.iostream2);

  g_main_loop_run (data.main_loop);
  g_main_loop_unref (data.main_loop);
}

int
main (int   argc,
      char *argv[])
{
  g_test_init (&argc, &argv, NULL);

  g_test_add_func ("/io-stream/copy-chunks", test_copy_chunks);

  return g_test_run();
}
