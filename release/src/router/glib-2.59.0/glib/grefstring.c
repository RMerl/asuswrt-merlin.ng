/* grefstring.c: Reference counted strings
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

/**
 * SECTION:refstring
 * @Title: Reference counted strings
 * @Short_description: Strings with reference counted memory management
 *
 * Reference counted strings are normal C strings that have been augmented
 * with a reference counter to manage their resources. You allocate a new
 * reference counted string and acquire and release references as needed,
 * instead of copying the string among callers; when the last reference on
 * the string is released, the resources allocated for it are freed.
 *
 * Typically, reference counted strings can be used when parsing data from
 * files and storing them into data structures that are passed to various
 * callers:
 *
 * |[<!-- language="C" -->
 * PersonDetails *
 * person_details_from_data (const char *data)
 * {
 *   // Use g_autoptr() to simplify error cases
 *   g_autoptr(GRefString) full_name = NULL;
 *   g_autoptr(GRefString) address =  NULL;
 *   g_autoptr(GRefString) city = NULL;
 *   g_autoptr(GRefString) state = NULL;
 *   g_autoptr(GRefString) zip_code = NULL;
 *
 *   // parse_person_details() is defined elsewhere; returns refcounted strings
 *   if (!parse_person_details (data, &full_name, &address, &city, &state, &zip_code))
 *     return NULL;
 *
 *   if (!validate_zip_code (zip_code))
 *     return NULL;
 *
 *   // add_address_to_cache() and add_full_name_to_cache() are defined
 *   // elsewhere; they add strings to various caches, using refcounted
 *   // strings to avoid copying data over and over again
 *   add_address_to_cache (address, city, state, zip_code);
 *   add_full_name_to_cache (full_name);
 *
 *   // person_details_new() is defined elsewhere; it takes a reference
 *   // on each string
 *   PersonDetails *res = person_details_new (full_name,
 *                                            address,
 *                                            city,
 *                                            state,
 *                                            zip_code);
 *
 *   return res;
 * }
 * ]|
 *
 * In the example above, we have multiple functions taking the same strings
 * for different uses; with typical C strings, we'd have to copy the strings
 * every time the life time rules of the data differ from the life time of
 * the string parsed from the original buffer. With reference counted strings,
 * each caller can take a reference on the data, and keep it as long as it
 * needs to own the string.
 *
 * Reference counted strings can also be "interned" inside a global table
 * owned by GLib; while an interned string has at least a reference, creating
 * a new interned reference counted string with the same contents will return
 * a reference to the existing string instead of creating a new reference
 * counted string instance. Once the string loses its last reference, it will
 * be automatically removed from the global interned strings table.
 *
 * Since: 2.58
 */

#include "config.h"

#include "grefstring.h"

#include "ghash.h"
#include "gmessages.h"
#include "grcbox.h"
#include "gthread.h"

#include <string.h>

/* A global table of refcounted strings; the hash table does not own
 * the strings, just a pointer to them. Strings are interned as long
 * as they are alive; once their reference count drops to zero, they
 * are removed from the table
 */
G_LOCK_DEFINE_STATIC (interned_ref_strings);
static GHashTable *interned_ref_strings;

/**
 * g_ref_string_new:
 * @str: (not nullable): a NUL-terminated string
 *
 * Creates a new reference counted string and copies the contents of @str
 * into it.
 *
 * Returns: (transfer full) (not nullable): the newly created reference counted string
 *
 * Since: 2.58
 */
char *
g_ref_string_new (const char *str)
{
  char *res;
  gsize len;

  g_return_val_if_fail (str != NULL, NULL);
  
  len = strlen (str);
  
  res = (char *) g_atomic_rc_box_dup (sizeof (char) * len + 1, str);

  return res;
}

/**
 * g_ref_string_new_len:
 * @str: (not nullable): a string
 * @len: length of @str to use, or -1 if @str is nul-terminated
 *
 * Creates a new reference counted string and copies the contents of @str
 * into it, up to @len bytes.
 *
 * Since this function does not stop at nul bytes, it is the caller's
 * responsibility to ensure that @str has at least @len addressable bytes.
 *
 * Returns: (transfer full) (not nullable): the newly created reference counted string
 *
 * Since: 2.58
 */
char *
g_ref_string_new_len (const char *str, gssize len)
{
  char *res;

  g_return_val_if_fail (str != NULL, NULL);

  if (len < 0)
    return g_ref_string_new (str);

  /* allocate then copy as str[len] may not be readable */
  res = (char *) g_atomic_rc_box_alloc ((gsize) len + 1);
  memcpy (res, str, len);
  res[len] = '\0';

  return res;
}

/* interned_str_equal: variant of g_str_equal() that compares
 * pointers as well as contents; this avoids running strcmp()
 * on arbitrarily long strings, as it's more likely to have
 * g_ref_string_new_intern() being called on the same refcounted
 * string instance, than on a different string with the same
 * contents
 */
static gboolean
interned_str_equal (gconstpointer v1,
                    gconstpointer v2)
{
  const char *str1 = v1;
  const char *str2 = v2;

  if (v1 == v2)
    return TRUE;

  return strcmp (str1, str2) == 0;
}

/**
 * g_ref_string_new_intern:
 * @str: (not nullable): a NUL-terminated string
 *
 * Creates a new reference counted string and copies the content of @str
 * into it.
 *
 * If you call this function multiple times with the same @str, or with
 * the same contents of @str, it will return a new reference, instead of
 * creating a new string.
 *
 * Returns: (transfer full) (not nullable): the newly created reference
 *   counted string, or a new reference to an existing string
 *
 * Since: 2.58
 */
char *
g_ref_string_new_intern (const char *str)
{
  char *res;

  g_return_val_if_fail (str != NULL, NULL);

  G_LOCK (interned_ref_strings);

  if (G_UNLIKELY (interned_ref_strings == NULL))
    interned_ref_strings = g_hash_table_new (g_str_hash, interned_str_equal);

  res = g_hash_table_lookup (interned_ref_strings, str);
  if (res != NULL)
    {
      /* We acquire the reference while holding the lock, to
       * avoid a potential race between releasing the lock on
       * the hash table and another thread releasing the reference
       * on the same string
       */
      g_atomic_rc_box_acquire (res);
      G_UNLOCK (interned_ref_strings);
      return res;
    }

  res = g_ref_string_new (str);
  g_hash_table_add (interned_ref_strings, res);
  G_UNLOCK (interned_ref_strings);

  return res;
}

/**
 * g_ref_string_acquire:
 * @str: a reference counted string
 *
 * Acquires a reference on a string.
 *
 * Returns: the given string, with its reference count increased
 *
 * Since: 2.58
 */
char *
g_ref_string_acquire (char *str)
{
  g_return_val_if_fail (str != NULL, NULL);

  return g_atomic_rc_box_acquire (str);
}

static void
remove_if_interned (gpointer data)
{
  char *str = data;

  G_LOCK (interned_ref_strings);

  if (G_LIKELY (interned_ref_strings != NULL))
    {
      g_hash_table_remove (interned_ref_strings, str);

      if (g_hash_table_size (interned_ref_strings) == 0)
        g_clear_pointer (&interned_ref_strings, g_hash_table_destroy);
    }

  G_UNLOCK (interned_ref_strings);
}

/**
 * g_ref_string_release:
 * @str: a reference counted string
 *
 * Releases a reference on a string; if it was the last reference, the
 * resources allocated by the string are freed as well.
 *
 * Since: 2.58
 */
void
g_ref_string_release (char *str)
{
  g_return_if_fail (str != NULL);

  g_atomic_rc_box_release_full (str, remove_if_interned);
}

/**
 * g_ref_string_length:
 * @str: a reference counted string
 *
 * Retrieves the length of @str.
 *
 * Returns: the length of the given string, in bytes
 *
 * Since: 2.58
 */
gsize
g_ref_string_length (char *str)
{
  g_return_val_if_fail (str != NULL, 0);

  return g_atomic_rc_box_get_size (str) - 1;
}
