/* garcbox.c: Atomically reference counted data
 *
 * Copyright 2018  Emmanuele Bassi
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#include "grcboxprivate.h"

#include "gmessages.h"
#include "grefcount.h"

#ifdef ENABLE_VALGRIND
#include "valgrind.h"
#endif

#include "glib_trace.h"

#include <string.h>

#define G_ARC_BOX(p)            (GArcBox *) (((char *) (p)) - G_ARC_BOX_SIZE)

/**
 * SECTION:arcbox
 * @Title: Atomically reference counted data
 * @Short_description: Allocated memory with atomic reference counting semantics
 *
 * An "atomically reference counted box", or "ArcBox", is an opaque wrapper
 * data type that is guaranteed to be as big as the size of a given data type,
 * and which augments the given data type with thread safe reference counting
 * semantics for its memory management.
 *
 * ArcBox is useful if you have a plain old data type, like a structure
 * typically placed on the stack, and you wish to provide additional API
 * to use it on the heap; or if you want to implement a new type to be
 * passed around by reference without necessarily implementing copy/free
 * semantics or your own reference counting.
 *
 * The typical use is:
 *
 * |[<!-- language="C" -->
 * typedef struct {
 *   char *name;
 *   char *address;
 *   char *city;
 *   char *state;
 *   int age;
 * } Person;
 *
 * Person *
 * person_new (void)
 * {
 *   return g_atomic_rc_box_new0 (Person);
 * }
 * ]|
 *
 * Every time you wish to acquire a reference on the memory, you should
 * call g_atomic_rc_box_acquire(); similarly, when you wish to release a reference
 * you should call g_atomic_rc_box_release():
 *
 * |[<!-- language="C" -->
 * // Add a Person to the Database; the Database acquires ownership
 * // of the Person instance
 * void
 * add_person_to_database (Database *db, Person *p)
 * {
 *   db->persons = g_list_prepend (db->persons, g_atomic_rc_box_acquire (p));
 * }
 *
 * // Removes a Person from the Database; the reference acquired by
 * // add_person_to_database() is released here
 * void
 * remove_person_from_database (Database *db, Person *p)
 * {
 *   db->persons = g_list_remove (db->persons, p);
 *   g_atomic_rc_box_release (p);
 * }
 * ]|
 *
 * If you have additional memory allocated inside the structure, you can
 * use g_atomic_rc_box_release_full(), which takes a function pointer, which
 * will be called if the reference released was the last:
 *
 * |[<!-- language="C" -->
 * void
 * person_clear (Person *p)
 * {
 *   g_free (p->name);
 *   g_free (p->address);
 *   g_free (p->city);
 *   g_free (p->state);
 * }
 *
 * void
 * remove_person_from_database (Database *db, Person *p)
 * {
 *   db->persons = g_list_remove (db->persons, p);
 *   g_atomic_rc_box_release_full (p, (GDestroyNotify) person_clear);
 * }
 * ]|
 *
 * If you wish to transfer the ownership of a reference counted data
 * type without increasing the reference count, you can use g_steal_pointer():
 *
 * |[<!-- language="C" -->
 *   Person *p = g_atomic_rc_box_new (Person);
 *
 *   fill_person_details (p);
 *
 *   add_person_to_database (db, g_steal_pointer (&p));
 * ]|
 *
 * ## Thread safety
 *
 * The reference counting operations on data allocated using g_atomic_rc_box_alloc(),
 * g_atomic_rc_box_new(), and g_atomic_rc_box_dup() are guaranteed to be atomic, and thus
 * can be safely be performed by different threads. It is important to note that
 * only the reference acquisition and release are atomic; changes to the content
 * of the data are your responsibility.
 *
 * ## Automatic pointer clean up
 *
 * If you want to add g_autoptr() support to your plain old data type through
 * reference counting, you can use the G_DEFINE_AUTOPTR_CLEANUP_FUNC() and
 * g_atomic_rc_box_release():
 *
 * |[<!-- language="C" -->
 * G_DEFINE_AUTOPTR_CLEANUP_FUNC (MyDataStruct, g_atomic_rc_box_release)
 * ]|
 *
 * If you need to clear the contents of the data, you will need to use an
 * ancillary function that calls g_rc_box_release_full():
 *
 * |[<!-- laguage="C" -->
 * static void
 * my_data_struct_release (MyDataStruct *data)
 * {
 *   // my_data_struct_clear() is defined elsewhere
 *   g_atomic_rc_box_release_full (data, (GDestroyNotify) my_data_struct_clear);
 * }
 *
 * G_DEFINE_AUTOPTR_CLEANUP_FUNC (MyDataStruct, my_data_struct_clear)
 * ]|
 *
 * Since: 2.58.
 */

/**
 * g_atomic_rc_box_alloc:
 * @block_size: the size of the allocation, must be greater than 0
 *
 * Allocates @block_size bytes of memory, and adds atomic
 * reference counting semantics to it.
 *
 * The data will be freed when its reference count drops to
 * zero.
 *
 * The allocated data is guaranteed to be suitably aligned for any
 * built-in type.
 *
 * Returns: (transfer full) (not nullable): a pointer to the allocated memory
 *
 * Since: 2.58
 */
gpointer
g_atomic_rc_box_alloc (gsize block_size)
{
  g_return_val_if_fail (block_size > 0, NULL);

  return g_rc_box_alloc_full (block_size, STRUCT_ALIGNMENT, TRUE, FALSE);
}

/**
 * g_atomic_rc_box_alloc0:
 * @block_size: the size of the allocation, must be greater than 0
 *
 * Allocates @block_size bytes of memory, and adds atomic
 * referenc counting semantics to it.
 *
 * The contents of the returned data is set to zero.
 *
 * The data will be freed when its reference count drops to
 * zero.
 *
 * The allocated data is guaranteed to be suitably aligned for any
 * built-in type.
 *
 * Returns: (transfer full) (not nullable): a pointer to the allocated memory
 *
 * Since: 2.58
 */
gpointer
g_atomic_rc_box_alloc0 (gsize block_size)
{
  g_return_val_if_fail (block_size > 0, NULL);

  return g_rc_box_alloc_full (block_size, STRUCT_ALIGNMENT, TRUE, TRUE);
}

/**
 * g_atomic_rc_box_new:
 * @type: the type to allocate, typically a structure name
 *
 * A convenience macro to allocate atomically reference counted
 * data with the size of the given @type.
 *
 * This macro calls g_atomic_rc_box_alloc() with `sizeof (@type)` and
 * casts the returned pointer to a pointer of the given @type,
 * avoiding a type cast in the source code.
 *
 * Returns: (transfer full) (not nullable): a pointer to the allocated
 *   memory, cast to a pointer for the given @type
 *
 * Since: 2.58
 */

/**
 * g_atomic_rc_box_new0:
 * @type: the type to allocate, typically a structure name
 *
 * A convenience macro to allocate atomically reference counted
 * data with the size of the given @type, and set its contents
 * to zero.
 *
 * This macro calls g_atomic_rc_box_alloc0() with `sizeof (@type)` and
 * casts the returned pointer to a pointer of the given @type,
 * avoiding a type cast in the source code.
 *
 * Returns: (transfer full) (not nullable): a pointer to the allocated
 *   memory, cast to a pointer for the given @type
 *
 * Since: 2.58
 */

/**
 * g_atomic_rc_box_dup:
 * @block_size: the number of bytes to copy, must be greater than 0
 * @mem_block: (not nullable): the memory to copy
 *
 * Allocates a new block of data with atomit reference counting
 * semantics, and copies @block_size bytes of @mem_block
 * into it.
 *
 * Returns: (transfer full) (not nullable): a pointer to the allocated
 *   memory
 *
 * Since: 2.58
 */
gpointer
(g_atomic_rc_box_dup) (gsize         block_size,
                       gconstpointer mem_block)
{
  gpointer res;

  g_return_val_if_fail (block_size > 0, NULL);
  g_return_val_if_fail (mem_block != NULL, NULL);

  res = g_rc_box_alloc_full (block_size, STRUCT_ALIGNMENT, TRUE, FALSE);
  memcpy (res, mem_block, block_size);

  return res;
}

/**
 * g_atomic_rc_box_acquire:
 * @mem_block: (not nullable): a pointer to reference counted data
 *
 * Atomically acquires a reference on the data pointed by @mem_block.
 *
 * Returns: (transfer full) (not nullable): a pointer to the data,
 *   with its reference count increased
 *
 * Since: 2.58
 */
gpointer
(g_atomic_rc_box_acquire) (gpointer mem_block)
{
  GArcBox *real_box = G_ARC_BOX (mem_block);

  g_return_val_if_fail (mem_block != NULL, NULL);
#ifndef G_DISABLE_ASSERT
  g_return_val_if_fail (real_box->magic == G_BOX_MAGIC, NULL);
#endif

  g_atomic_ref_count_inc (&real_box->ref_count);

  TRACE (GLIB_RCBOX_ACQUIRE (mem_block, 1));

  return mem_block;
}

/**
 * g_atomic_rc_box_release:
 * @mem_block: (transfer full) (not nullable): a pointer to reference counted data
 *
 * Atomically releases a reference on the data pointed by @mem_block.
 *
 * If the reference was the last one, it will free the
 * resources allocated for @mem_block.
 *
 * Since: 2.58
 */
void
g_atomic_rc_box_release (gpointer mem_block)
{
  g_atomic_rc_box_release_full (mem_block, NULL);
}

/**
 * g_atomic_rc_box_release_full:
 * @mem_block: (transfer full) (not nullable): a pointer to reference counted data
 * @clear_func: (not nullable): a function to call when clearing the data
 *
 * Atomically releases a reference on the data pointed by @mem_block.
 *
 * If the reference was the last one, it will call @clear_func
 * to clear the contents of @mem_block, and then will free the
 * resources allocated for @mem_block.
 *
 * Since: 2.58
 */
void
g_atomic_rc_box_release_full (gpointer       mem_block,
                              GDestroyNotify clear_func)
{
  GArcBox *real_box = G_ARC_BOX (mem_block);

  g_return_if_fail (mem_block != NULL);
#ifndef G_DISABLE_ASSERT
  g_return_if_fail (real_box->magic == G_BOX_MAGIC);
#endif

  if (g_atomic_ref_count_dec (&real_box->ref_count))
    {
      char *real_mem = (char *) real_box - real_box->private_offset;

      TRACE (GLIB_RCBOX_RELEASE (mem_block, 1));

      if (clear_func != NULL)
        clear_func (mem_block);

      TRACE (GLIB_RCBOX_FREE (mem_block));
      g_free (real_mem);
    }
}

/**
 * g_atomic_rc_box_get_size:
 * @mem_block: (not nullable): a pointer to reference counted data
 *
 * Retrieves the size of the reference counted data pointed by @mem_block.
 *
 * Returns: the size of the data, in bytes
 *
 * Since: 2.58
 */
gsize
g_atomic_rc_box_get_size (gpointer mem_block)
{
  GArcBox *real_box = G_ARC_BOX (mem_block);

  g_return_val_if_fail (mem_block != NULL, 0);
#ifndef G_DISABLE_ASSERT
  g_return_val_if_fail (real_box->magic == G_BOX_MAGIC, 0);
#endif

  return real_box->mem_size;
}
