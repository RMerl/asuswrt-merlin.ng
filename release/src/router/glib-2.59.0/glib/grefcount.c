/* grefcount.c: Reference counting
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
 * SECTION:refcount
 * @Title: Reference counting
 * @Short_description: Reference counting types and functions
 *
 * Reference counting is a garbage collection mechanism that is based on
 * assigning a counter to a data type, or any memory area; the counter is
 * increased whenever a new reference to that data type is acquired, and
 * decreased whenever the reference is released. Once the last reference
 * is released, the resources associated to that data type are freed.
 *
 * GLib uses reference counting in many of its data types, and provides
 * the #grefcount and #gatomicrefcount types to implement safe and atomic
 * reference counting semantics in new data types.
 *
 * It is important to note that #grefcount and #gatomicrefcount should be
 * considered completely opaque types; you should always use the provided
 * API to increase and decrease the counters, and you should never check
 * their content directly, or compare their content with other values.
 *
 * Since: 2.58
 */

#include "config.h"

#include "grefcount.h"

#include "gatomic.h"
#include "gmessages.h"

/**
 * grefcount:
 *
 * A type for implementing non-atomic reference count semantics.
 *
 * Use g_ref_count_init() to initialize it; g_ref_count_inc() to
 * increase the counter, and g_ref_count_dec() to decrease it.
 *
 * It is safe to use #grefcount only if you're expecting to operate
 * on the reference counter from a single thread. It is entirely up
 * to you to ensure that all reference count changes happen in the
 * same thread.
 *
 * See also: #gatomicrefcount
 *
 * Since: 2.58
 */

/**
 * gatomicrefcount:
 *
 * A type for implementing atomic reference count semantics.
 *
 * Use g_atomic_ref_count_init() to initialize it; g_atomic_ref_count_inc()
 * to increase the counter, and g_atomic_ref_count_dec() to decrease it.
 *
 * It is safe to use #gatomicrefcount if you're expecting to operate on the
 * reference counter from multiple threads.
 *
 * See also: #grefcount
 *
 * Since: 2.58
 */

/**
 * g_ref_count_init:
 * @rc: the address of a reference count variable
 *
 * Initializes a reference count variable.
 *
 * Since: 2.58
 */
void
(g_ref_count_init) (grefcount *rc)
{
  g_return_if_fail (rc != NULL);

  /* Non-atomic refcounting is implemented using the negative range
   * of signed integers:
   *
   * G_MININT                 Z¯< 0 > Z⁺                G_MAXINT
   * |----------------------------|----------------------------|
   *
   * Acquiring a reference moves us towards MININT, and releasing a
   * reference moves us towards 0.
   */
  *rc = -1;
}

/**
 * g_ref_count_inc:
 * @rc: the address of a reference count variable
 *
 * Increases the reference count.
 *
 * Since: 2.58
 */
void
(g_ref_count_inc) (grefcount *rc)
{
  grefcount rrc;

  g_return_if_fail (rc != NULL);

  rrc = *rc;

  g_return_if_fail (rrc < 0);

  /* Check for saturation */
  if (rrc == G_MININT)
    {
      g_critical ("Reference count %p has reached saturation", rc);
      return;
    }

  rrc -= 1;

  *rc = rrc;
}

/**
 * g_ref_count_dec:
 * @rc: the address of a reference count variable
 *
 * Decreases the reference count.
 *
 * Returns: %TRUE if the reference count reached 0, and %FALSE otherwise
 *
 * Since: 2.58
 */
gboolean
(g_ref_count_dec) (grefcount *rc)
{
  grefcount rrc;

  g_return_val_if_fail (rc != NULL, FALSE);

  rrc = *rc;

  g_return_val_if_fail (rrc < 0, FALSE);

  rrc += 1;
  if (rrc == 0)
    return TRUE;

  *rc = rrc;

  return FALSE;
}

/**
 * g_ref_count_compare:
 * @rc: the address of a reference count variable
 * @val: the value to compare
 *
 * Compares the current value of @rc with @val.
 *
 * Returns: %TRUE if the reference count is the same
 *   as the given value
 *
 * Since: 2.58
 */
gboolean
(g_ref_count_compare) (grefcount *rc,
                       gint       val)
{
  grefcount rrc;

  g_return_val_if_fail (rc != NULL, FALSE);
  g_return_val_if_fail (val >= 0, FALSE);

  rrc = *rc;

  if (val == G_MAXINT)
    return rrc == G_MININT;

  return rrc == -val;
}

/**
 * g_atomic_ref_count_init:
 * @arc: the address of an atomic reference count variable
 *
 * Initializes a reference count variable.
 *
 * Since: 2.58
 */
void
(g_atomic_ref_count_init) (gatomicrefcount *arc)
{
  g_return_if_fail (arc != NULL);

  /* Atomic refcounting is implemented using the positive range
   * of signed integers:
   *
   * G_MININT                 Z¯< 0 > Z⁺                G_MAXINT
   * |----------------------------|----------------------------|
   *
   * Acquiring a reference moves us towards MAXINT, and releasing a
   * reference moves us towards 0.
   */
  *arc = 1;
}

/**
 * g_atomic_ref_count_inc:
 * @arc: the address of an atomic reference count variable
 *
 * Atomically increases the reference count.
 *
 * Since: 2.58
 */
void
(g_atomic_ref_count_inc) (gatomicrefcount *arc)
{
  g_return_if_fail (arc != NULL);
  g_return_if_fail (g_atomic_int_get (arc) > 0);

  if (g_atomic_int_get (arc) == G_MAXINT)
    {
      g_critical ("Reference count has reached saturation");
      return;
    }

  g_atomic_int_inc (arc);
}

/**
 * g_atomic_ref_count_dec:
 * @arc: the address of an atomic reference count variable
 *
 * Atomically decreases the reference count.
 *
 * Returns: %TRUE if the reference count reached 0, and %FALSE otherwise
 *
 * Since: 2.58
 */
gboolean
(g_atomic_ref_count_dec) (gatomicrefcount *arc)
{
  g_return_val_if_fail (arc != NULL, FALSE);
  g_return_val_if_fail (g_atomic_int_get (arc) > 0, FALSE);

  return g_atomic_int_dec_and_test (arc);
}

/**
 * g_atomic_ref_count_compare:
 * @arc: the address of an atomic reference count variable
 * @val: the value to compare
 *
 * Atomically compares the current value of @arc with @val.
 *
 * Returns: %TRUE if the reference count is the same
 *   as the given value
 *
 * Since: 2.58
 */
gboolean
(g_atomic_ref_count_compare) (gatomicrefcount *arc,
                              gint             val)
{
  g_return_val_if_fail (arc != NULL, FALSE);
  g_return_val_if_fail (val >= 0, FALSE);

  return g_atomic_int_get (arc) == val;
}
