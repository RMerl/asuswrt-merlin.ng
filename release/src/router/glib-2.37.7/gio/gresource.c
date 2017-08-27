/* GIO - GLib Input, Output and Streaming Library
 *
 * Copyright © 2011 Red Hat, Inc
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
 * Authors: Alexander Larsson <alexl@redhat.com>
 */

#include "config.h"

#include <string.h>

#include "gresource.h"
#include <gvdb/gvdb-reader.h>
#include <gi18n.h>
#include <gio/gmemoryinputstream.h>
#include <gio/gzlibdecompressor.h>
#include <gio/gconverterinputstream.h>

struct _GResource
{
  int ref_count;

  GvdbTable *table;
};

static void register_lazy_static_resources (void);

G_DEFINE_BOXED_TYPE (GResource, g_resource, g_resource_ref, g_resource_unref)

/**
 * SECTION:gresource
 * @short_description: Resource framework
 * @include: gio/gio.h
 *
 * Applications and libraries often contain binary or textual data that is really part of the
 * application, rather than user data. For instance #GtkBuilder .ui files, splashscreen images,
 * GMenu markup xml, CSS files, icons, etc. These are often shipped as files in <filename>$datadir/appname</filename>, or
 * manually included as literal strings in the code.
 *
 * The #GResource API and the <link linkend="glib-compile-resources">glib-compile-resources</link> program
 * provide a convenient and efficient alternative to this which has some nice properties. You
 * maintain the files as normal files, so its easy to edit them, but during the build the files
 * are combined into a binary bundle that is linked into the executable. This means that loading
 * the resource files are efficient (as they are already in memory, shared with other instances) and
 * simple (no need to check for things like I/O errors or locate the files in the filesystem). It
 * also makes it easier to create relocatable applications.
 *
 * Resource files can also be marked as compressed. Such files will be included in the resource bundle
 * in a compressed form, but will be automatically uncompressed when the resource is used. This
 * is very useful e.g. for larger text files that are parsed once (or rarely) and then thrown away.
 *
 * Resource files can also be marked to be preprocessed, by setting the value of the
 * <literal>preprocess</literal> attribute to a comma-separated list of preprocessing options.
 * The only options currently supported are:
 *
 * <literal>xml-stripblanks</literal> which will use <command>xmllint</command> to strip
 * ignorable whitespace from the xml file. For this to work, the <envar>XMLLINT</envar>
 * environment variable must be set to the full path to the xmllint executable, or xmllint
 * must be in the PATH; otherwise the preprocessing step is skipped.
 *
 * <literal>to-pixdata</literal> which will use <command>gdk-pixbuf-pixdata</command> to convert
 * images to the GdkPixdata format, which allows you to create pixbufs directly using the data inside
 * the resource file, rather than an (uncompressed) copy if it. For this, the gdk-pixbuf-pixdata
 * program must be in the PATH, or the <envar>GDK_PIXBUF_PIXDATA</envar> environment variable must be
 * set to the full path to the gdk-pixbuf-pixdata executable; otherwise the resource compiler will
 * abort.
 *
 * Resource bundles are created by the <link linkend="glib-compile-resources">glib-compile-resources</link> program
 * which takes an xml file that describes the bundle, and a set of files that the xml references. These
 * are combined into a binary resource bundle.
 *
 * <example id="resource-example"><title>Example resource description</title>
 * <programlisting><![CDATA[
 * <?xml version="1.0" encoding="UTF-8"?>
 * <gresources>
 *   <gresource prefix="/org/gtk/Example">
 *     <file>data/splashscreen.png</file>
 *     <file compressed="true">dialog.ui</file>
 *     <file preprocess="xml-stripblanks">menumarkup.xml</file>
 *   </gresource>
 * </gresources>
 * ]]></programlisting></example>
 *
 * This will create a resource bundle with the following files:
 * <programlisting><![CDATA[
 * /org/gtk/Example/data/splashscreen.png
 * /org/gtk/Example/dialog.ui
 * /org/gtk/Example/menumarkup.xml
 * ]]></programlisting>
 *
 * Note that all resources in the process share the same namespace, so use java-style
 * path prefixes (like in the above example) to avoid conflicts.
 *
 * You can then use <link linkend="glib-compile-resources">glib-compile-resources</link> to compile the xml to a
 * binary bundle that you can load with g_resource_load(). However, its more common to use the --generate-source and
 * --generate-header arguments to create a source file and header to link directly into your application.
 *
 * Once a #GResource has been created and registered all the data in it can be accessed globally in the process by
 * using API calls like g_resources_open_stream() to stream the data or g_resources_lookup_data() to get a direct pointer
 * to the data. You can also use uris like "resource:///org/gtk/Example/data/splashscreen.png" with #GFile to access
 * the resource data.
 *
 * There are two forms of the generated source, the default version uses the compiler support for constructor
 * and destructor functions (where available) to automatically create and register the #GResource on startup
 * or library load time. If you pass --manual-register two functions to register/unregister the resource is instead
 * created. This requires an explicit initialization call in your application/library, but it works on all platforms,
 * even on the minor ones where this is not available. (Constructor support is available for at least Win32, MacOS and Linux.)
 *
 * Note that resource data can point directly into the data segment of e.g. a library, so if you are unloading libraries
 * during runtime you need to be very careful with keeping around pointers to data from a resource, as this goes away
 * when the library is unloaded. However, in practice this is not generally a problem, since most resource accesses
 * is for your own resources, and resource data is often used once, during parsing, and then released.
 *
 * Since: 2.32
 */

/**
 * g_resource_error_quark:
 *
 * Gets the #GResource Error Quark.
 *
 * Return value: a #GQuark
 *
 * Since: 2.32
 */
G_DEFINE_QUARK (g-resource-error-quark, g_resource_error)

/**
 * g_resource_ref:
 * @resource: A #GResource
 *
 * Atomically increments the reference count of @array by one. This
 * function is MT-safe and may be called from any thread.
 *
 * Returns: The passed in #GResource
 *
 * Since: 2.32
 **/
GResource *
g_resource_ref (GResource *resource)
{
  g_atomic_int_inc (&resource->ref_count);
  return resource;
}

/**
 * g_resource_unref:
 * @resource: A #GResource
 *
 * Atomically decrements the reference count of @resource by one. If the
 * reference count drops to 0, all memory allocated by the array is
 * released. This function is MT-safe and may be called from any
 * thread.
 *
 * Since: 2.32
 **/
void
g_resource_unref (GResource *resource)
{
  if (g_atomic_int_dec_and_test (&resource->ref_count))
    {
      gvdb_table_unref (resource->table);
      g_free (resource);
    }
}

/*
 * g_resource_new_from_table:
 * @table: (transfer full): a GvdbTable
 *
 * Returns: (transfer full): a new #GResource for @table
 */
static GResource *
g_resource_new_from_table (GvdbTable *table)
{
  GResource *resource;

  resource = g_new (GResource, 1);
  resource->ref_count = 1;
  resource->table = table;

  return resource;
}

/**
 * g_resource_new_from_data:
 * @data: A #GBytes
 * @error: return location for a #GError, or %NULL
 *
 * Creates a GResource from a reference to the binary resource bundle.
 * This will keep a reference to @data while the resource lives, so
 * the data should not be modified or freed.
 *
 * If you want to use this resource in the global resource namespace you need
 * to register it with g_resources_register().
 *
 * Return value: (transfer full): a new #GResource, or %NULL on error
 *
 * Since: 2.32
 **/
GResource *
g_resource_new_from_data (GBytes  *data,
                          GError **error)
{
  GvdbTable *table;

  table = gvdb_table_new_from_data (g_bytes_get_data (data, NULL),
                                    g_bytes_get_size (data),
                                    TRUE,
                                    g_bytes_ref (data),
                                    (GvdbRefFunc)g_bytes_ref,
                                    (GDestroyNotify)g_bytes_unref,
                                    error);

  if (table == NULL)
    return NULL;

  return g_resource_new_from_table (table);
}

/**
 * g_resource_load:
 * @filename: (type filename): the path of a filename to load, in the GLib filename encoding
 * @error: return location for a #GError, or %NULL
 *
 * Loads a binary resource bundle and creates a #GResource representation of it, allowing
 * you to query it for data.
 *
 * If you want to use this resource in the global resource namespace you need
 * to register it with g_resources_register().
 *
 * Return value: (transfer full): a new #GResource, or %NULL on error
 *
 * Since: 2.32
 **/
GResource *
g_resource_load (const gchar  *filename,
                 GError      **error)
{
  GvdbTable *table;

  table = gvdb_table_new (filename, FALSE, error);
  if (table == NULL)
    return NULL;

  return g_resource_new_from_table (table);
}

static
gboolean do_lookup (GResource             *resource,
                    const gchar           *path,
                    GResourceLookupFlags   lookup_flags,
                    gsize                 *size,
                    guint32               *flags,
                    const void           **data,
                    gsize                 *data_size,
                    GError               **error)
{
  char *free_path = NULL;
  gsize path_len;
  gboolean res = FALSE;
  GVariant *value;

  path_len = strlen (path);
  if (path[path_len-1] == '/')
    {
      path = free_path = g_strdup (path);
      free_path[path_len-1] = 0;
    }

  value = gvdb_table_get_raw_value (resource->table, path);

  if (value == NULL)
    {
      g_set_error (error, G_RESOURCE_ERROR, G_RESOURCE_ERROR_NOT_FOUND,
                   _("The resource at '%s' does not exist"),
                   path);
    }
  else
    {
      guint32 _size, _flags;
      GVariant *array;

      g_variant_get (value, "(uu@ay)",
                     &_size,
                     &_flags,
                     &array);

      _size = GUINT32_FROM_LE (_size);
      _flags = GUINT32_FROM_LE (_flags);

      if (size)
        *size = _size;
      if (flags)
        *flags = _flags;
      if (data)
        *data = g_variant_get_data (array);
      if (data_size)
        {
          /* Don't report trailing newline that non-compressed files has */
          if (_flags & G_RESOURCE_FLAGS_COMPRESSED)
            *data_size = g_variant_get_size (array);
          else
            *data_size = g_variant_get_size (array) - 1;
        }
      g_variant_unref (array);
      g_variant_unref (value);

      res = TRUE;
    }

  g_free (free_path);
  return res;
}

/**
 * g_resource_open_stream:
 * @resource: A #GResource
 * @path: A pathname inside the resource
 * @lookup_flags: A #GResourceLookupFlags
 * @error: return location for a #GError, or %NULL
 *
 * Looks for a file at the specified @path in the resource and
 * returns a #GInputStream that lets you read the data.
 *
 * @lookup_flags controls the behaviour of the lookup.
 *
 * Returns: (transfer full): #GInputStream or %NULL on error.
 *     Free the returned object with g_object_unref()
 *
 * Since: 2.32
 **/
GInputStream *
g_resource_open_stream (GResource             *resource,
                        const gchar           *path,
                        GResourceLookupFlags   lookup_flags,
                        GError               **error)
{
  const void *data;
  gsize data_size;
  guint32 flags;
  GInputStream *stream, *stream2;

  if (!do_lookup (resource, path, lookup_flags, NULL, &flags, &data, &data_size, error))
    return NULL;

  stream = g_memory_input_stream_new_from_data (data, data_size, NULL);
  g_object_set_data_full (G_OBJECT (stream), "g-resource",
                          g_resource_ref (resource),
                          (GDestroyNotify)g_resource_unref);

  if (flags & G_RESOURCE_FLAGS_COMPRESSED)
    {
      GZlibDecompressor *decompressor =
        g_zlib_decompressor_new (G_ZLIB_COMPRESSOR_FORMAT_ZLIB);

      stream2 = g_converter_input_stream_new (stream, G_CONVERTER (decompressor));
      g_object_unref (decompressor);
      g_object_unref (stream);
      stream = stream2;
    }

  return stream;
}

/**
 * g_resource_lookup_data:
 * @resource: A #GResource
 * @path: A pathname inside the resource
 * @lookup_flags: A #GResourceLookupFlags
 * @error: return location for a #GError, or %NULL
 *
 * Looks for a file at the specified @path in the resource and
 * returns a #GBytes that lets you directly access the data in
 * memory.
 *
 * The data is always followed by a zero byte, so you
 * can safely use the data as a C string. However, that byte
 * is not included in the size of the GBytes.
 *
 * For uncompressed resource files this is a pointer directly into
 * the resource bundle, which is typically in some readonly data section
 * in the program binary. For compressed files we allocate memory on
 * the heap and automatically uncompress the data.
 *
 * @lookup_flags controls the behaviour of the lookup.
 *
 * Returns: (transfer full): #GBytes or %NULL on error.
 *     Free the returned object with g_bytes_unref()
 *
 * Since: 2.32
 **/
GBytes *
g_resource_lookup_data (GResource             *resource,
                        const gchar           *path,
                        GResourceLookupFlags   lookup_flags,
                        GError               **error)
{
  const void *data;
  guint32 flags;
  gsize data_size;
  gsize size;

  if (!do_lookup (resource, path, lookup_flags, &size, &flags, &data, &data_size, error))
    return NULL;

  if (flags & G_RESOURCE_FLAGS_COMPRESSED)
    {
      char *uncompressed, *d;
      const char *s;
      GConverterResult res;
      gsize d_size, s_size;
      gsize bytes_read, bytes_written;


      GZlibDecompressor *decompressor =
        g_zlib_decompressor_new (G_ZLIB_COMPRESSOR_FORMAT_ZLIB);

      uncompressed = g_malloc (size + 1);

      s = data;
      s_size = data_size;
      d = uncompressed;
      d_size = size;

      do
        {
          res = g_converter_convert (G_CONVERTER (decompressor),
                                     s, s_size,
                                     d, d_size,
                                     G_CONVERTER_INPUT_AT_END,
                                     &bytes_read,
                                     &bytes_written,
                                     NULL);
          if (res == G_CONVERTER_ERROR)
            {
              g_free (uncompressed);
              g_object_unref (decompressor);

              g_set_error (error, G_RESOURCE_ERROR, G_RESOURCE_ERROR_INTERNAL,
                           _("The resource at '%s' failed to decompress"),
                           path);
              return NULL;

            }
          s += bytes_read;
          s_size -= bytes_read;
          d += bytes_written;
          d_size -= bytes_written;
        }
      while (res != G_CONVERTER_FINISHED);

      uncompressed[size] = 0; /* Zero terminate */

      g_object_unref (decompressor);

      return g_bytes_new_take (uncompressed, size);
    }
  else
    return g_bytes_new_with_free_func (data, data_size, (GDestroyNotify)g_resource_unref, g_resource_ref (resource));
}

/**
 * g_resource_get_info:
 * @resource: A #GResource
 * @path: A pathname inside the resource
 * @lookup_flags: A #GResourceLookupFlags
 * @size:  (out) (allow-none): a location to place the length of the contents of the file,
 *    or %NULL if the length is not needed
 * @flags:  (out) (allow-none): a location to place the flags about the file,
 *    or %NULL if the length is not needed
 * @error: return location for a #GError, or %NULL
 *
 * Looks for a file at the specified @path in the resource and
 * if found returns information about it.
 *
 * @lookup_flags controls the behaviour of the lookup.
 *
 * Returns: %TRUE if the file was found. %FALSE if there were errors
 *
 * Since: 2.32
 **/
gboolean
g_resource_get_info (GResource             *resource,
                     const gchar           *path,
                     GResourceLookupFlags   lookup_flags,
                     gsize                 *size,
                     guint32               *flags,
                     GError               **error)
{
  return do_lookup (resource, path, lookup_flags, size, flags, NULL, NULL, error);
}

/**
 * g_resource_enumerate_children:
 * @resource: A #GResource
 * @path: A pathname inside the resource
 * @lookup_flags: A #GResourceLookupFlags
 * @error: return location for a #GError, or %NULL
 *
 * Returns all the names of children at the specified @path in the resource.
 * The return result is a %NULL terminated list of strings which should
 * be released with g_strfreev().
 *
 * @lookup_flags controls the behaviour of the lookup.
 *
 * Returns: (array zero-terminated=1) (transfer full): an array of constant strings
 *
 * Since: 2.32
 **/
gchar **
g_resource_enumerate_children (GResource             *resource,
                               const gchar           *path,
                               GResourceLookupFlags   lookup_flags,
                               GError               **error)
{
  gchar **children;
  gsize path_len;
  char *path_with_slash;

  if (*path == 0)
    {
      g_set_error (error, G_RESOURCE_ERROR, G_RESOURCE_ERROR_NOT_FOUND,
                   _("The resource at '%s' does not exist"),
                   path);
      return NULL;
    }

  path_len = strlen (path);
  if (path[path_len-1] != '/')
    path_with_slash = g_strconcat (path, "/", NULL);
  else
    path_with_slash = g_strdup (path);

  children = gvdb_table_list (resource->table, path_with_slash);
  g_free (path_with_slash);

  if (children == NULL)
    {
      g_set_error (error, G_RESOURCE_ERROR, G_RESOURCE_ERROR_NOT_FOUND,
                   _("The resource at '%s' does not exist"),
                   path);
      return NULL;
    }

  return children;
}

static GRWLock resources_lock;
static GList *registered_resources;

/* This is updated atomically, so we can append to it and check for NULL outside the
   lock, but all other accesses are done under the write lock */
static GStaticResource *lazy_register_resources;

static void
g_resources_register_unlocked (GResource *resource)
{
  registered_resources = g_list_prepend (registered_resources, g_resource_ref (resource));
}

static void
g_resources_unregister_unlocked (GResource *resource)
{
  if (g_list_find (registered_resources, resource) == NULL)
    {
      g_warning ("Tried to remove not registered resource");
    }
  else
    {
      registered_resources = g_list_remove (registered_resources, resource);
      g_resource_unref (resource);
    }
}

/**
 * g_resources_register:
 * @resource: A #GResource
 *
 * Registers the resource with the process-global set of resources.
 * Once a resource is registered the files in it can be accessed
 * with the global resource lookup functions like g_resources_lookup_data().
 *
 * Since: 2.32
 **/
void
g_resources_register (GResource *resource)
{
  g_rw_lock_writer_lock (&resources_lock);
  g_resources_register_unlocked (resource);
  g_rw_lock_writer_unlock (&resources_lock);
}

/**
 * g_resources_unregister:
 * @resource: A #GResource
 *
 * Unregisters the resource from the process-global set of resources.
 *
 * Since: 2.32
 **/
void
g_resources_unregister (GResource *resource)
{
  g_rw_lock_writer_lock (&resources_lock);
  g_resources_unregister_unlocked (resource);
  g_rw_lock_writer_unlock (&resources_lock);
}

/**
 * g_resources_open_stream:
 * @path: A pathname inside the resource
 * @lookup_flags: A #GResourceLookupFlags
 * @error: return location for a #GError, or %NULL
 *
 * Looks for a file at the specified @path in the set of
 * globally registered resources and returns a #GInputStream
 * that lets you read the data.
 *
 * @lookup_flags controls the behaviour of the lookup.
 *
 * Returns: (transfer full): #GInputStream or %NULL on error.
 *     Free the returned object with g_object_unref()
 *
 * Since: 2.32
 **/
GInputStream *
g_resources_open_stream (const gchar           *path,
                         GResourceLookupFlags   lookup_flags,
                         GError               **error)
{
  GInputStream *res = NULL;
  GList *l;
  GInputStream *stream;

  register_lazy_static_resources ();

  g_rw_lock_reader_lock (&resources_lock);

  for (l = registered_resources; l != NULL; l = l->next)
    {
      GResource *r = l->data;
      GError *my_error = NULL;

      stream = g_resource_open_stream (r, path, lookup_flags, &my_error);
      if (stream == NULL &&
          g_error_matches (my_error, G_RESOURCE_ERROR, G_RESOURCE_ERROR_NOT_FOUND))
        {
          g_clear_error (&my_error);
        }
      else
        {
          if (stream == NULL)
            g_propagate_error (error, my_error);
          res = stream;
          break;
        }
    }

  if (l == NULL)
    g_set_error (error, G_RESOURCE_ERROR, G_RESOURCE_ERROR_NOT_FOUND,
                 _("The resource at '%s' does not exist"),
                 path);

  g_rw_lock_reader_unlock (&resources_lock);

  return res;
}

/**
 * g_resources_lookup_data:
 * @path: A pathname inside the resource
 * @lookup_flags: A #GResourceLookupFlags
 * @error: return location for a #GError, or %NULL
 *
 * Looks for a file at the specified @path in the set of
 * globally registered resources and returns a #GBytes that
 * lets you directly access the data in memory.
 *
 * The data is always followed by a zero byte, so you
 * can safely use the data as a C string. However, that byte
 * is not included in the size of the GBytes.
 *
 * For uncompressed resource files this is a pointer directly into
 * the resource bundle, which is typically in some readonly data section
 * in the program binary. For compressed files we allocate memory on
 * the heap and automatically uncompress the data.
 *
 * @lookup_flags controls the behaviour of the lookup.
 *
 * Returns: (transfer full): #GBytes or %NULL on error.
 *     Free the returned object with g_bytes_unref()
 *
 * Since: 2.32
 **/
GBytes *
g_resources_lookup_data (const gchar           *path,
                         GResourceLookupFlags   lookup_flags,
                         GError               **error)
{
  GBytes *res = NULL;
  GList *l;
  GBytes *data;

  register_lazy_static_resources ();

  g_rw_lock_reader_lock (&resources_lock);

  for (l = registered_resources; l != NULL; l = l->next)
    {
      GResource *r = l->data;
      GError *my_error = NULL;

      data = g_resource_lookup_data (r, path, lookup_flags, &my_error);
      if (data == NULL &&
          g_error_matches (my_error, G_RESOURCE_ERROR, G_RESOURCE_ERROR_NOT_FOUND))
        {
          g_clear_error (&my_error);
        }
      else
        {
          if (data == NULL)
            g_propagate_error (error, my_error);
          res = data;
          break;
        }
    }

  if (l == NULL)
    g_set_error (error, G_RESOURCE_ERROR, G_RESOURCE_ERROR_NOT_FOUND,
                 _("The resource at '%s' does not exist"),
                 path);

  g_rw_lock_reader_unlock (&resources_lock);

  return res;
}

/**
 * g_resources_enumerate_children:
 * @path: A pathname inside the resource
 * @lookup_flags: A #GResourceLookupFlags
 * @error: return location for a #GError, or %NULL
 *
 * Returns all the names of children at the specified @path in the set of
 * globally registered resources.
 * The return result is a %NULL terminated list of strings which should
 * be released with g_strfreev().
 *
 * @lookup_flags controls the behaviour of the lookup.
 *
 * Returns: (array zero-terminated=1) (transfer full): an array of constant strings
 *
 * Since: 2.32
 **/
gchar **
g_resources_enumerate_children (const gchar           *path,
                                GResourceLookupFlags   lookup_flags,
                                GError               **error)
{
  GHashTable *hash = NULL;
  GList *l;
  char **children;
  int i;

  register_lazy_static_resources ();

  g_rw_lock_reader_lock (&resources_lock);

  for (l = registered_resources; l != NULL; l = l->next)
    {
      GResource *r = l->data;

      children = g_resource_enumerate_children (r, path, 0, NULL);

      if (children != NULL)
        {
          if (hash == NULL)
            hash = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);

          for (i = 0; children[i] != NULL; i++)
            g_hash_table_insert (hash, children[i], children[i]);
          g_free (children);
        }
    }

  g_rw_lock_reader_unlock (&resources_lock);

  if (hash == NULL)
    {
      g_set_error (error, G_RESOURCE_ERROR, G_RESOURCE_ERROR_NOT_FOUND,
                   _("The resource at '%s' does not exist"),
                   path);
      return NULL;
    }
  else
    {
      GHashTableIter iter;
      const char *key;
      guint n_children;
      n_children = g_hash_table_size (hash);
      children = g_new (char *, n_children + 1);
      i = 0;

      g_hash_table_iter_init (&iter, hash);
      while (g_hash_table_iter_next (&iter, (gpointer *)&key, NULL))
        children[i++] = g_strdup (key);
      children[i++] = NULL;

      g_hash_table_destroy (hash);

      return children;
    }
}

/**
 * g_resources_get_info:
 * @path: A pathname inside the resource
 * @lookup_flags: A #GResourceLookupFlags
 * @size:  (out) (allow-none): a location to place the length of the contents of the file,
 *    or %NULL if the length is not needed
 * @flags:  (out) (allow-none): a location to place the flags about the file,
 *    or %NULL if the length is not needed
 * @error: return location for a #GError, or %NULL
 *
 * Looks for a file at the specified @path in the set of
 * globally registered resources and if found returns information about it.
 *
 * @lookup_flags controls the behaviour of the lookup.
 *
 * Returns: %TRUE if the file was found. %FALSE if there were errors
 *
 * Since: 2.32
 **/
gboolean
g_resources_get_info (const gchar           *path,
                      GResourceLookupFlags   lookup_flags,
                      gsize                 *size,
                      guint32               *flags,
                      GError               **error)
{
  gboolean res = FALSE;
  GList *l;
  gboolean r_res;

  register_lazy_static_resources ();

  g_rw_lock_reader_lock (&resources_lock);

  for (l = registered_resources; l != NULL; l = l->next)
    {
      GResource *r = l->data;
      GError *my_error = NULL;

      r_res = g_resource_get_info (r, path, lookup_flags, size, flags, &my_error);
      if (!r_res &&
          g_error_matches (my_error, G_RESOURCE_ERROR, G_RESOURCE_ERROR_NOT_FOUND))
        {
          g_clear_error (&my_error);
        }
      else
        {
          if (!r_res)
            g_propagate_error (error, my_error);
          res = r_res;
          break;
        }
    }

  if (l == NULL)
    g_set_error (error, G_RESOURCE_ERROR, G_RESOURCE_ERROR_NOT_FOUND,
                 _("The resource at '%s' does not exist"),
                 path);

  g_rw_lock_reader_unlock (&resources_lock);

  return res;
}

/* This code is to handle registration of resources very early, from a constructor.
 * At that point we'd like to do minimal work, to avoid ordering issues. For instance,
 * we're not allowed to use g_malloc, as the user need to be able to call g_mem_set_vtable
 * before the first call to g_malloc.
 *
 * So, what we do at construction time is that we just register a static structure on
 * a list of resources that need to be initialized, and then later, when doing any lookups
 * in the global list of registered resources, or when getting a reference to the
 * lazily initialized resource we lazily create and register all the GResources on
 * the lazy list.
 *
 * To avoid having to use locks in the constructor, and having to grab the writer lock
 * when checking the lazy registering list we update lazy_register_resources in
 * a lock-less fashion (atomic prepend-only, atomic replace with NULL). However, all
 * operations except:
 *  * check if there are any resources to lazily initialize
 *  * Add a static resource to the lazy init list
 * Do use the full writer lock for protection.
 */

static void
register_lazy_static_resources_unlocked (void)
{
  GStaticResource *list;

  do
    list = lazy_register_resources;
  while (!g_atomic_pointer_compare_and_exchange (&lazy_register_resources, list, NULL));

  while (list != NULL)
    {
      GBytes *bytes = g_bytes_new_static (list->data, list->data_len);
      GResource *resource = g_resource_new_from_data (bytes, NULL);
      if (resource)
        {
          g_resources_register_unlocked (resource);
          g_atomic_pointer_set (&list->resource, resource);
        }
      g_bytes_unref (bytes);

      list = list->next;
    }
}

static void
register_lazy_static_resources (void)
{
  if (g_atomic_pointer_get (&lazy_register_resources) == NULL)
    return;

  g_rw_lock_writer_lock (&resources_lock);
  register_lazy_static_resources_unlocked ();
  g_rw_lock_writer_unlock (&resources_lock);
}

/**
 * g_static_resource_init:
 * @static_resource: pointer to a static #GStaticResource
 *
 * Initializes a GResource from static data using a
 * GStaticResource.
 *
 * This is normally used by code generated by
 * <link linkend="glib-compile-resources">glib-compile-resources</link>
 * and is not typically used by other code.
 *
 * Since: 2.32
 **/
void
g_static_resource_init (GStaticResource *static_resource)
{
  gpointer next;

  do
    {
      next = lazy_register_resources;
      static_resource->next = next;
    }
  while (!g_atomic_pointer_compare_and_exchange (&lazy_register_resources, next, static_resource));
}

/**
 * g_static_resource_fini:
 * @static_resource: pointer to a static #GStaticResource
 *
 * Finalized a GResource initialized by g_static_resource_init().
 *
 * This is normally used by code generated by
 * <link linkend="glib-compile-resources">glib-compile-resources</link>
 * and is not typically used by other code.
 *
 * Since: 2.32
 **/
void
g_static_resource_fini (GStaticResource *static_resource)
{
  GResource *resource;

  g_rw_lock_writer_lock (&resources_lock);

  register_lazy_static_resources_unlocked ();

  resource = g_atomic_pointer_get (&static_resource->resource);
  if (resource)
    {
      g_atomic_pointer_set (&static_resource->resource, NULL);
      g_resources_unregister_unlocked (resource);
      g_resource_unref (resource);
    }

  g_rw_lock_writer_unlock (&resources_lock);
}

/**
 * g_static_resource_get_resource:
 * @static_resource: pointer to a static #GStaticResource
 *
 * Gets the GResource that was registered by a call to g_static_resource_init().
 *
 * This is normally used by code generated by
 * <link linkend="glib-compile-resources">glib-compile-resources</link>
 * and is not typically used by other code.
 *
 * Return value:  (transfer none): a #GResource
 *
 * Since: 2.32
 **/
GResource *
g_static_resource_get_resource (GStaticResource *static_resource)
{
  register_lazy_static_resources ();

  return g_atomic_pointer_get (&static_resource->resource);
}
