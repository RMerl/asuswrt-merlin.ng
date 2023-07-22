/* GLib testing framework examples and tests
 * Copyright (C) 2018 Canonical Ltd
 * Authors: Marco Trevisan <marco@ubuntu.com>
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

#include <glib-object.h>
#include <string.h>

G_DECLARE_DERIVABLE_TYPE (TestAutoCleanupBase, test_base_auto_cleanup, TEST, BASE_AUTO_CLEANUP, GObject)

struct _TestAutoCleanupBaseClass {
  GObjectClass parent_class;
};

G_DECLARE_FINAL_TYPE (TestAutoCleanup, test_auto_cleanup, TEST, AUTO_CLEANUP, TestAutoCleanupBase)

struct _TestAutoCleanup
{
  TestAutoCleanupBase parent_instance;
};

G_DEFINE_TYPE (TestAutoCleanup, test_auto_cleanup, G_TYPE_OBJECT)

static void
test_auto_cleanup_class_init (TestAutoCleanupClass *class)
{
}

static void
test_auto_cleanup_init (TestAutoCleanup *tac)
{
}

static TestAutoCleanup *
test_auto_cleanup_new (void)
{
  return g_object_new (test_auto_cleanup_get_type (), NULL);
}

/* Verify that an object declared with G_DECLARE_FINAL_TYPE provides by default
 * autocleanup functions, defined using the ones of the base type (defined with
 * G_DECLARE_DERIVABLE_TYPE) and so that it can be used with g_autoptr */
static void
test_autoptr (void)
{
  TestAutoCleanup *tac_ptr = test_auto_cleanup_new ();
  g_object_add_weak_pointer (G_OBJECT (tac_ptr), (gpointer *) &tac_ptr);

  {
    g_autoptr (TestAutoCleanup) tac = tac_ptr;
  }
#ifdef __GNUC__
  g_assert_null (tac_ptr);
#endif
}

/* Verify that an object declared with G_DECLARE_FINAL_TYPE provides by default
 * autocleanup functions, defined using the ones of the base type (defined with
 * G_DECLARE_DERIVABLE_TYPE) and that stealing an autopointer works properly */
static void
test_autoptr_steal (void)
{
  g_autoptr (TestAutoCleanup) tac1 = test_auto_cleanup_new ();
  TestAutoCleanup *tac_ptr = tac1;

  g_object_add_weak_pointer (G_OBJECT (tac_ptr), (gpointer *) &tac_ptr);

  {
    g_autoptr (TestAutoCleanup) tac2 = g_steal_pointer (&tac1);
    g_assert_nonnull (tac_ptr);
    g_assert_null (tac1);
    g_assert_true (tac2 == tac_ptr);
  }
#ifdef __GNUC__
  g_assert_null (tac_ptr);
#endif
}

/* Verify that an object declared with G_DECLARE_FINAL_TYPE provides by default
 * autolist cleanup functions defined using the ones of the parent type
 * and so that can be used with g_autolist, and that freeing the list correctly
 * unrefs the object too */
static void
test_autolist (void)
{
  TestAutoCleanup *tac1 = test_auto_cleanup_new ();
  TestAutoCleanup *tac2 = test_auto_cleanup_new ();
  g_autoptr (TestAutoCleanup) tac3 = test_auto_cleanup_new ();

  g_object_add_weak_pointer (G_OBJECT (tac1), (gpointer *) &tac1);
  g_object_add_weak_pointer (G_OBJECT (tac2), (gpointer *) &tac2);
  g_object_add_weak_pointer (G_OBJECT (tac3), (gpointer *) &tac3);

  {
    g_autolist (TestAutoCleanup) l = NULL;

    l = g_list_prepend (l, tac1);
    l = g_list_prepend (l, tac2);
  }

  /* Only assert if autoptr works */
#ifdef __GNUC__
  g_assert_null (tac1);
  g_assert_null (tac2);
#endif
  g_assert_nonnull (tac3);

  g_clear_object (&tac3);
  g_assert_null (tac3);
}

/* Verify that an object declared with G_DECLARE_FINAL_TYPE provides by default
 * autoslist cleanup functions (defined using the ones of the base type declared
 * with G_DECLARE_DERIVABLE_TYPE) and so that can be used with g_autoslist, and
 * that freeing the slist correctly unrefs the object too */
static void
test_autoslist (void)
{
  TestAutoCleanup *tac1 = test_auto_cleanup_new ();
  TestAutoCleanup *tac2 = test_auto_cleanup_new ();
  g_autoptr (TestAutoCleanup) tac3 = test_auto_cleanup_new ();

  g_object_add_weak_pointer (G_OBJECT (tac1), (gpointer *) &tac1);
  g_object_add_weak_pointer (G_OBJECT (tac2), (gpointer *) &tac2);
  g_object_add_weak_pointer (G_OBJECT (tac3), (gpointer *) &tac3);

  {
    g_autoslist (TestAutoCleanup) l = NULL;

    l = g_slist_prepend (l, tac1);
    l = g_slist_prepend (l, tac2);
  }

  /* Only assert if autoptr works */
#ifdef __GNUC__
  g_assert_null (tac1);
  g_assert_null (tac2);
#endif
  g_assert_nonnull (tac3);

  g_clear_object (&tac3);
  g_assert_null (tac3);
}

int
main (int argc, gchar *argv[])
{
  g_test_init (&argc, &argv, NULL);

  g_test_add_func ("/autoptr/autoptr", test_autoptr);
  g_test_add_func ("/autoptr/autoptr_steal", test_autoptr_steal);
  g_test_add_func ("/autoptr/autolist", test_autolist);
  g_test_add_func ("/autoptr/autoslist", test_autoslist);

  return g_test_run ();
}
