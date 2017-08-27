#include <unistd.h>
#include <glib.h>
#include <glib-object.h>

#define G_TYPE_TEST               (my_test_get_type ())
#define MY_TEST(test)              (G_TYPE_CHECK_INSTANCE_CAST ((test), G_TYPE_TEST, GTest))
#define MY_IS_TEST(test)           (G_TYPE_CHECK_INSTANCE_TYPE ((test), G_TYPE_TEST))
#define MY_TEST_CLASS(tclass)      (G_TYPE_CHECK_CLASS_CAST ((tclass), G_TYPE_TEST, GTestClass))
#define MY_IS_TEST_CLASS(tclass)   (G_TYPE_CHECK_CLASS_TYPE ((tclass), G_TYPE_TEST))
#define MY_TEST_GET_CLASS(test)    (G_TYPE_INSTANCE_GET_CLASS ((test), G_TYPE_TEST, GTestClass))

enum {
  PROP_0,
  PROP_DUMMY
};

typedef struct _GTest GTest;
typedef struct _GTestClass GTestClass;

struct _GTest
{
  GObject object;
  gint id;
  gint dummy;

  gint count;
};

struct _GTestClass
{
  GObjectClass parent_class;
};

static GType my_test_get_type (void);
static volatile gboolean stopping;

static void my_test_class_init (GTestClass * klass);
static void my_test_init (GTest * test);
static void my_test_dispose (GObject * object);
static void my_test_get_property (GObject    *object,
                                  guint       prop_id,
                                  GValue     *value,
                                  GParamSpec *pspec);
static void my_test_set_property (GObject      *object,
                                  guint         prop_id,
                                  const GValue *value,
                                  GParamSpec   *pspec);

static GObjectClass *parent_class = NULL;

static GType
my_test_get_type (void)
{
  static GType test_type = 0;

  if (!test_type) {
    const GTypeInfo test_info = {
      sizeof (GTestClass),
      NULL,
      NULL,
      (GClassInitFunc) my_test_class_init,
      NULL,
      NULL,
      sizeof (GTest),
      0,
      (GInstanceInitFunc) my_test_init,
      NULL
    };

    test_type = g_type_register_static (G_TYPE_OBJECT, "GTest", &test_info, 0);
  }
  return test_type;
}

static void
my_test_class_init (GTestClass * klass)
{
  GObjectClass *gobject_class;

  gobject_class = (GObjectClass *) klass;

  parent_class = g_type_class_ref (G_TYPE_OBJECT);

  gobject_class->dispose = my_test_dispose;
  gobject_class->get_property = my_test_get_property;
  gobject_class->set_property = my_test_set_property;

  g_object_class_install_property (gobject_class,
				   PROP_DUMMY,
				   g_param_spec_int ("dummy",
						     NULL, 
						     NULL,
						     0, G_MAXINT, 0,
						     G_PARAM_READWRITE));
}

static void
my_test_init (GTest * test)
{
  static guint static_id = 1;
  test->id = static_id++;
}

static void
my_test_dispose (GObject * object)
{
  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void 
my_test_get_property (GObject    *object,
                      guint       prop_id,
                      GValue     *value,
                      GParamSpec *pspec)
{
  GTest *test;

  test = MY_TEST (object);

  switch (prop_id)
    {
    case PROP_DUMMY:
      g_value_set_int (value, test->dummy);
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
  GTest *test;

  test = MY_TEST (object);

  switch (prop_id)
    {
    case PROP_DUMMY:
      test->dummy = g_value_get_int (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
dummy_notify (GObject    *object,
              GParamSpec *pspec)
{
  GTest *test;

  test = MY_TEST (object);

  test->count++;  
}

static void
my_test_do_property (GTest * test)
{
  gint dummy;

  g_object_get (test, "dummy", &dummy, NULL);
  g_object_set (test, "dummy", dummy + 1, NULL);
}

static gpointer
run_thread (GTest * test)
{
  gint i = 1;
  
  while (!stopping) {
    my_test_do_property (test);
    if ((i++ % 10000) == 0)
      {
        g_print (".%c", 'a' + test->id);
        g_thread_yield(); /* force context switch */
      }
  }

  g_object_unref (test);

  return NULL;
}

int
main (int argc, char **argv)
{
  gint i;
  GArray *test_objects;
  GArray *test_threads;
  const gint n_threads = 5;

  g_print ("START: %s\n", argv[0]);
  g_log_set_always_fatal (G_LOG_LEVEL_WARNING | G_LOG_LEVEL_CRITICAL | g_log_set_always_fatal (G_LOG_FATAL_MASK));

  test_objects = g_array_new (FALSE, FALSE, sizeof (GTest *));

  for (i = 0; i < n_threads; i++) {
    GTest *test;
    
    test = g_object_new (G_TYPE_TEST, NULL);
    g_array_append_val (test_objects, test);

    g_assert (test->count == test->dummy);
    g_signal_connect (test, "notify::dummy", G_CALLBACK (dummy_notify), NULL);
  }
    
  test_threads = g_array_new (FALSE, FALSE, sizeof (GThread *));

  stopping = FALSE;

  for (i = 0; i < n_threads; i++) {
    GThread *thread;
    GTest *test;

    test = g_array_index (test_objects, GTest *, i);

    thread = g_thread_create ((GThreadFunc) run_thread, test, TRUE, NULL);
    g_array_append_val (test_threads, thread);
  }
  g_usleep (3000000);

  stopping = TRUE;
  g_print ("\nstopping\n");

  /* join all threads */
  for (i = 0; i < n_threads; i++) {
    GThread *thread;

    thread = g_array_index (test_threads, GThread *, i);
    g_thread_join (thread);
  }

  g_print ("stopped\n");

  for (i = 0; i < n_threads; i++) {
    GTest *test;

    test = g_array_index (test_objects, GTest *, i);

    g_assert (test->count == test->dummy);
  }

  return 0;
}
