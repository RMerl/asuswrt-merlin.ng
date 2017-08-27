/* GIO - GLib Input, Output and Streaming Library
 *
 * Copyright (C) 2009 Red Hat, Inc.
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
 * Author: Alexander Larsson <alexl@redhat.com>
 */

#include "config.h"
#include "ginitable.h"
#include "glibintl.h"


/**
 * SECTION:ginitable
 * @short_description: Failable object initialization interface
 * @include: gio/gio.h
 * @see_also: #GAsyncInitable
 *
 * #GInitable is implemented by objects that can fail during
 * initialization. If an object implements this interface then
 * it must be initialized as the first thing after construction,
 * either via g_initable_init() or g_async_initable_init_async()
 * (the latter is only available if it also implements #GAsyncInitable).
 *
 * If the object is not initialized, or initialization returns with an
 * error, then all operations on the object except g_object_ref() and
 * g_object_unref() are considered to be invalid, and have undefined
 * behaviour. They will often fail with g_critical() or g_warning(), but
 * this must not be relied on.
 *
 * Users of objects implementing this are not intended to use
 * the interface method directly, instead it will be used automatically
 * in various ways. For C applications you generally just call
 * g_initable_new() directly, or indirectly via a foo_thing_new() wrapper.
 * This will call g_initable_init() under the cover, returning %NULL and
 * setting a #GError on failure (at which point the instance is
 * unreferenced).
 *
 * For bindings in languages where the native constructor supports
 * exceptions the binding could check for objects implemention %GInitable
 * during normal construction and automatically initialize them, throwing
 * an exception on failure.
 */

typedef GInitableIface GInitableInterface;
G_DEFINE_INTERFACE (GInitable, g_initable, G_TYPE_OBJECT)

static void
g_initable_default_init (GInitableInterface *iface)
{
}

/**
 * g_initable_init:
 * @initable: a #GInitable.
 * @cancellable: optional #GCancellable object, %NULL to ignore.
 * @error: a #GError location to store the error occurring, or %NULL to
 * ignore.
 *
 * Initializes the object implementing the interface.
 *
 * The object must be initialized before any real use after initial
 * construction, either with this function or g_async_initable_init_async().
 *
 * Implementations may also support cancellation. If @cancellable is not %NULL,
 * then initialization can be cancelled by triggering the cancellable object
 * from another thread. If the operation was cancelled, the error
 * %G_IO_ERROR_CANCELLED will be returned. If @cancellable is not %NULL and
 * the object doesn't support cancellable initialization the error
 * %G_IO_ERROR_NOT_SUPPORTED will be returned.
 *
 * If the object is not initialized, or initialization returns with an
 * error, then all operations on the object except g_object_ref() and
 * g_object_unref() are considered to be invalid, and have undefined
 * behaviour. See the <xref linkend="ginitable"/> section introduction
 * for more details.
 *
 * Implementations of this method must be idempotent, i.e. multiple calls
 * to this function with the same argument should return the same results.
 * Only the first call initializes the object, further calls return the result
 * of the first call. This is so that it's safe to implement the singleton
 * pattern in the GObject constructor function.
 *
 * Returns: %TRUE if successful. If an error has occurred, this function will
 *     return %FALSE and set @error appropriately if present.
 *
 * Since: 2.22
 */
gboolean
g_initable_init (GInitable     *initable,
		 GCancellable  *cancellable,
		 GError       **error)
{
  GInitableIface *iface;

  g_return_val_if_fail (G_IS_INITABLE (initable), FALSE);

  iface = G_INITABLE_GET_IFACE (initable);

  return (* iface->init) (initable, cancellable, error);
}

/**
 * g_initable_new:
 * @object_type: a #GType supporting #GInitable.
 * @cancellable: optional #GCancellable object, %NULL to ignore.
 * @error: a #GError location to store the error occurring, or %NULL to
 *    ignore.
 * @first_property_name: (allow-none): the name of the first property, or %NULL if no
 *     properties
 * @...:  the value if the first property, followed by and other property
 *    value pairs, and ended by %NULL.
 *
 * Helper function for constructing #GInitable object. This is
 * similar to g_object_new() but also initializes the object
 * and returns %NULL, setting an error on failure.
 *
 * Return value: (type GObject.Object) (transfer full): a newly allocated
 *      #GObject, or %NULL on error
 *
 * Since: 2.22
 */
gpointer
g_initable_new (GType          object_type,
		GCancellable  *cancellable,
		GError       **error,
		const gchar   *first_property_name,
		...)
{
  GObject *object;
  va_list var_args;

  va_start (var_args, first_property_name);
  object = g_initable_new_valist (object_type,
				  first_property_name, var_args,
				  cancellable, error);
  va_end (var_args);

  return object;
}

/**
 * g_initable_newv:
 * @object_type: a #GType supporting #GInitable.
 * @n_parameters: the number of parameters in @parameters
 * @parameters: (array length=n_parameters): the parameters to use to construct the object
 * @cancellable: optional #GCancellable object, %NULL to ignore.
 * @error: a #GError location to store the error occurring, or %NULL to
 *     ignore.
 *
 * Helper function for constructing #GInitable object. This is
 * similar to g_object_newv() but also initializes the object
 * and returns %NULL, setting an error on failure.
 *
 * Return value: (type GObject.Object) (transfer full): a newly allocated
 *      #GObject, or %NULL on error
 *
 * Since: 2.22
 */
gpointer
g_initable_newv (GType          object_type,
		 guint          n_parameters,
		 GParameter    *parameters,
		 GCancellable  *cancellable,
		 GError       **error)
{
  GObject *obj;

  g_return_val_if_fail (G_TYPE_IS_INITABLE (object_type), NULL);

  obj = g_object_newv (object_type, n_parameters, parameters);

  if (!g_initable_init (G_INITABLE (obj), cancellable, error))
    {
      g_object_unref (obj);
      return NULL;
    }

  return (gpointer)obj;
}

/**
 * g_initable_new_valist:
 * @object_type: a #GType supporting #GInitable.
 * @first_property_name: the name of the first property, followed by
 * the value, and other property value pairs, and ended by %NULL.
 * @var_args: The var args list generated from @first_property_name.
 * @cancellable: optional #GCancellable object, %NULL to ignore.
 * @error: a #GError location to store the error occurring, or %NULL to
 *     ignore.
 *
 * Helper function for constructing #GInitable object. This is
 * similar to g_object_new_valist() but also initializes the object
 * and returns %NULL, setting an error on failure.
 *
 * Return value: (type GObject.Object) (transfer full): a newly allocated
 *      #GObject, or %NULL on error
 *
 * Since: 2.22
 */
GObject*
g_initable_new_valist (GType          object_type,
		       const gchar   *first_property_name,
		       va_list        var_args,
		       GCancellable  *cancellable,
		       GError       **error)
{
  GObject *obj;

  g_return_val_if_fail (G_TYPE_IS_INITABLE (object_type), NULL);

  obj = g_object_new_valist (object_type,
			     first_property_name,
			     var_args);

  if (!g_initable_init (G_INITABLE (obj), cancellable, error))
    {
      g_object_unref (obj);
      return NULL;
    }

  return obj;
}
