#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include <stdio.h>
#include <stdlib.h>

#include <dbus/dbus.h>

#ifdef DBUS_TEST_USE_INTERNAL

# include <dbus/dbus-mainloop.h>
# include <dbus/dbus-internals.h>
  typedef DBusLoop TestMainContext;

#else /* !DBUS_TEST_USE_INTERNAL */

# include <glib.h>
  typedef GMainContext TestMainContext;

#endif /* !DBUS_TEST_USE_INTERNAL */

TestMainContext *test_main_context_get            (void);
TestMainContext *test_main_context_ref            (TestMainContext *ctx);
void             test_main_context_unref          (TestMainContext *ctx);
void             test_main_context_iterate        (TestMainContext *ctx,
                                                   dbus_bool_t      may_block);

dbus_bool_t test_connection_setup                 (TestMainContext *ctx,
                                                   DBusConnection *connection);
void        test_connection_shutdown              (TestMainContext *ctx,
                                                   DBusConnection *connection);

dbus_bool_t test_server_setup                     (TestMainContext *ctx,
                                                   DBusServer    *server);
void        test_server_shutdown                  (TestMainContext *ctx,
                                                   DBusServer    *server);

#endif
