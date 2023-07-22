/* flags.c
 * Copyright (C) 2018 Arthur Demchenkov
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
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, see
 * <http://www.gnu.org/licenses/>.
 */

#include <glib-object.h>

/* Check that validation of flags works on architectures where
 * #gint and #glong are different sizes, as the flags are cast
 * between types a few times.
 *
 * See: https://gitlab.gnome.org/GNOME/glib/issues/1572
 */

enum {
  PROP_FLAGS = 1
};

typedef struct _GTest GTest;
typedef struct _GTestClass GTestClass;

typedef enum {
  NO_FLAG      = 0,
  LOWEST_FLAG  = 1,
  HIGHEST_FLAG = 1 << 31
} MyFlagsEnum;

struct _GTest {
  GObject object;
  MyFlagsEnum flags;
};

struct _GTestClass {
  GObjectClass parent_class;
};

static GType my_test_get_type (void);
static GType my_test_flags_get_type (void);

#define G_TYPE_TEST (my_test_get_type())
#define MY_TEST(test) (G_TYPE_CHECK_INSTANCE_CAST ((test), G_TYPE_TEST, GTest))
G_DEFINE_TYPE (GTest, my_test, G_TYPE_OBJECT)

static void my_test_class_init (GTestClass * klass);
static void my_test_init (GTest * test);
static void my_test_get_property (GObject    *object,
				  guint       prop_id,
				  GValue     *value,
				  GParamSpec *pspec);
static void my_test_set_property (GObject      *object,
				  guint         prop_id,
				  const GValue *value,
				  GParamSpec   *pspec);

static GType
my_test_flags_get_type (void)
{
  static GType flags_type = 0;

  if (G_UNLIKELY(flags_type == 0))
    {
      static const GFlagsValue values[] = {
	{ LOWEST_FLAG,  "LOWEST_FLAG",  "lowest" },
	{ HIGHEST_FLAG, "HIGHEST_FLAG", "highest" },
	{ 0, NULL, NULL }
      };

      flags_type = g_flags_register_static (g_intern_static_string ("GTestFlags"), values);
    }
  return flags_type;
}

static void
my_test_class_init (GTestClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->get_property = my_test_get_property;
  gobject_class->set_property = my_test_set_property;

  g_object_class_install_property (gobject_class, 1,
				   g_param_spec_flags ("flags",
						       "Flags",
						       "Flags test property",
						       my_test_flags_get_type(), 0,
						       G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
}

static void my_test_init (GTest *test)
{
}

static void
my_test_get_property (GObject    *object,
		      guint       prop_id,
		      GValue     *value,
		      GParamSpec *pspec)
{
  GTest *test = MY_TEST (object);

  switch (prop_id)
    {
    case PROP_FLAGS:
      g_value_set_flags (value, test->flags);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
my_test_set_property (GObject      *object,
		      guint         prop_id,
		      const GValue *value,
		      GParamSpec   *pspec)
{
  GTest *test = MY_TEST (object);

  switch (prop_id)
    {
    case PROP_FLAGS:
      test->flags = g_value_get_flags (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
check_flags_validation (void)
{
  guint test_flags[] = {
    NO_FLAG,
    LOWEST_FLAG,
    HIGHEST_FLAG,
    LOWEST_FLAG | HIGHEST_FLAG
  };
  guint flag_read;
  gsize i;

  for (i = 0; i < G_N_ELEMENTS (test_flags); i++)
    {
      guint flag_set = test_flags[i];
      GObject *test = g_object_new (G_TYPE_TEST,
				    "flags", flag_set,
				    NULL);

      g_object_get (test, "flags", &flag_read, NULL);

      /* This check will fail in case of gint -> glong conversion
       * in value_flags_enum_collect_value() */
      g_assert_cmpint (flag_read, ==, flag_set);

      g_object_unref (test);
    }
}

int
main (int argc, char **argv)
{
  g_test_init (&argc, &argv, NULL);
  g_test_add_func ("/gobject/flags/validate", check_flags_validation);
  return g_test_run ();
}
