/*
 * Copyright © 2010 Codethink Limited
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 2 of the licence or (at
 * your option) any later version.
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
 * Authors: Ryan Lortie <desrt@desrt.ca>
 */

/* Prologue {{{1 */
#include "config.h"

#include "gapplication.h"

#include "gapplicationcommandline.h"
#include "gsimpleactiongroup.h"
#include "gremoteactiongroup.h"
#include "gapplicationimpl.h"
#include "gactiongroup.h"
#include "gactionmap.h"
#include "gmenumodel.h"
#include "gsettings.h"

#include "gioenumtypes.h"
#include "gioenums.h"
#include "gfile.h"

#include "glibintl.h"

#include <string.h>

/**
 * SECTION:gapplication
 * @title: GApplication
 * @short_description: Core application class
 *
 * A #GApplication is the foundation of an application.  It wraps some
 * low-level platform-specific services and is intended to act as the
 * foundation for higher-level application classes such as
 * #GtkApplication or #MxApplication.  In general, you should not use
 * this class outside of a higher level framework.
 *
 * GApplication provides convenient life cycle management by maintaining
 * a <firstterm>use count</firstterm> for the primary application instance.
 * The use count can be changed using g_application_hold() and
 * g_application_release(). If it drops to zero, the application exits.
 * Higher-level classes such as #GtkApplication employ the use count to
 * ensure that the application stays alive as long as it has any opened
 * windows.
 *
 * Another feature that GApplication (optionally) provides is process
 * uniqueness.  Applications can make use of this functionality by
 * providing a unique application ID.  If given, only one application
 * with this ID can be running at a time per session.  The session
 * concept is platform-dependent, but corresponds roughly to a graphical
 * desktop login.  When your application is launched again, its
 * arguments are passed through platform communication to the already
 * running program.  The already running instance of the program is
 * called the <firstterm>primary instance</firstterm>; for non-unique
 * applications this is the always the current instance.
 * On Linux, the D-Bus session bus is used for communication.
 *
 * The use of #GApplication differs from some other commonly-used
 * uniqueness libraries (such as libunique) in important ways.  The
 * application is not expected to manually register itself and check if
 * it is the primary instance.  Instead, the <code>main()</code>
 * function of a #GApplication should do very little more than
 * instantiating the application instance, possibly connecting signal
 * handlers, then calling g_application_run().  All checks for
 * uniqueness are done internally.  If the application is the primary
 * instance then the startup signal is emitted and the mainloop runs.
 * If the application is not the primary instance then a signal is sent
 * to the primary instance and g_application_run() promptly returns.
 * See the code examples below.
 *
 * If used, the expected form of an application identifier is very close
 * to that of of a
 * <ulink url="http://dbus.freedesktop.org/doc/dbus-specification.html#message-protocol-names-interface">DBus bus name</ulink>.
 * Examples include: "com.example.MyApp", "org.example.internal-apps.Calculator".
 * For details on valid application identifiers, see g_application_id_is_valid().
 *
 * On Linux, the application identifier is claimed as a well-known bus name
 * on the user's session bus.  This means that the uniqueness of your
 * application is scoped to the current session.  It also means that your
 * application may provide additional services (through registration of other
 * object paths) at that bus name.  The registration of these object paths
 * should be done with the shared GDBus session bus.  Note that due to the
 * internal architecture of GDBus, method calls can be dispatched at any time
 * (even if a main loop is not running).  For this reason, you must ensure that
 * any object paths that you wish to register are registered before #GApplication
 * attempts to acquire the bus name of your application (which happens in
 * g_application_register()).  Unfortunately, this means that you cannot use
 * g_application_get_is_remote() to decide if you want to register object paths.
 *
 * GApplication also implements the #GActionGroup and #GActionMap
 * interfaces and lets you easily export actions by adding them with
 * g_action_map_add_action(). When invoking an action by calling
 * g_action_group_activate_action() on the application, it is always
 * invoked in the primary instance. The actions are also exported on
 * the session bus, and GIO provides the #GDBusActionGroup wrapper to
 * conveniently access them remotely. GIO provides a #GDBusMenuModel wrapper
 * for remote access to exported #GMenuModels.
 *
 * There is a number of different entry points into a GApplication:
 * <itemizedlist>
 * <listitem>via 'Activate' (i.e. just starting the application)</listitem>
 * <listitem>via 'Open' (i.e. opening some files)</listitem>
 * <listitem>by handling a command-line</listitem>
 * <listitem>via activating an action</listitem>
 * </itemizedlist>
 * The #GApplication::startup signal lets you handle the application
 * initialization for all of these in a single place.
 *
 * Regardless of which of these entry points is used to start the application,
 * GApplication passes some <firstterm id="platform-data">platform
 * data</firstterm> from the launching instance to the primary instance,
 * in the form of a #GVariant dictionary mapping strings to variants.
 * To use platform data, override the @before_emit or @after_emit virtual
 * functions in your #GApplication subclass. When dealing with
 * #GApplicationCommandLine objects, the platform data is directly
 * available via g_application_command_line_get_cwd(),
 * g_application_command_line_get_environ() and
 * g_application_command_line_get_platform_data().
 *
 * As the name indicates, the platform data may vary depending on the
 * operating system, but it always includes the current directory (key
 * "cwd"), and optionally the environment (ie the set of environment
 * variables and their values) of the calling process (key "environ").
 * The environment is only added to the platform data if the
 * %G_APPLICATION_SEND_ENVIRONMENT flag is set. #GApplication subclasses
 * can add their own platform data by overriding the @add_platform_data
 * virtual function. For instance, #GtkApplication adds startup notification
 * data in this way.
 *
 * To parse commandline arguments you may handle the
 * #GApplication::command-line signal or override the local_command_line()
 * vfunc, to parse them in either the primary instance or the local instance,
 * respectively.
 *
 * <example id="gapplication-example-open"><title>Opening files with a GApplication</title>
 * <programlisting>
 * <xi:include xmlns:xi="http://www.w3.org/2001/XInclude" parse="text" href="../../../../gio/tests/gapplication-example-open.c">
 *   <xi:fallback>FIXME: MISSING XINCLUDE CONTENT</xi:fallback>
 * </xi:include>
 * </programlisting>
 * </example>
 *
 * <example id="gapplication-example-actions"><title>A GApplication with actions</title>
 * <programlisting>
 * <xi:include xmlns:xi="http://www.w3.org/2001/XInclude" parse="text" href="../../../../gio/tests/gapplication-example-actions.c">
 *   <xi:fallback>FIXME: MISSING XINCLUDE CONTENT</xi:fallback>
 * </xi:include>
 * </programlisting>
 * </example>
 *
 * <example id="gapplication-example-menu"><title>A GApplication with menus</title>
 * <programlisting>
 * <xi:include xmlns:xi="http://www.w3.org/2001/XInclude" parse="text" href="../../../../gio/tests/gapplication-example-menu.c">
 *   <xi:fallback>FIXME: MISSING XINCLUDE CONTENT</xi:fallback>
 * </xi:include>
 * </programlisting>
 * </example>
 *
 * <example id="gapplication-example-dbushooks"><title>Using extra D-Bus hooks with a GApplication</title>
 * <programlisting>
 * <xi:include xmlns:xi="http://www.w3.org/2001/XInclude" parse="text" href="../../../../gio/tests/gapplication-example-dbushooks.c">
 *   <xi:fallback>FIXME: MISSING XINCLUDE CONTENT</xi:fallback>
 * </xi:include>
 * </programlisting>
 * </example>
 */

/**
 * GApplicationClass:
 * @startup: invoked on the primary instance immediately after registration
 * @shutdown: invoked only on the registered primary instance immediately
 *      after the main loop terminates
 * @activate: invoked on the primary instance when an activation occurs
 * @open: invoked on the primary instance when there are files to open
 * @command_line: invoked on the primary instance when a command-line is
 *   not handled locally
 * @local_command_line: invoked (locally) when the process has been invoked
 *     via commandline execution (as opposed to, say, D-Bus activation - which
 *     is not currently supported by GApplication). The virtual function has
 *     the chance to inspect (and possibly replace) the list of command line
 *     arguments. See g_application_run() for more information.
 * @before_emit: invoked on the primary instance before 'activate', 'open',
 *     'command-line' or any action invocation, gets the 'platform data' from
 *     the calling instance
 * @after_emit: invoked on the primary instance after 'activate', 'open',
 *     'command-line' or any action invocation, gets the 'platform data' from
 *     the calling instance
 * @add_platform_data: invoked (locally) to add 'platform data' to be sent to
 *     the primary instance when activating, opening or invoking actions
 * @quit_mainloop: Used to be invoked on the primary instance when the use
 *     count of the application drops to zero (and after any inactivity
 *     timeout, if requested). Not used anymore since 2.32
 * @run_mainloop: Used to be invoked on the primary instance from
 *     g_application_run() if the use-count is non-zero. Since 2.32,
 *     GApplication is iterating the main context directly and is not
 *     using @run_mainloop anymore
 * @dbus_register: invoked locally during registration, if the application is
 *     using its D-Bus backend. You can use this to export extra objects on the
 *     bus, that need to exist before the application tries to own the bus name.
 *     The function is passed the #GDBusConnection to to session bus, and the
 *     object path that #GApplication will use to export is D-Bus API.
 *     If this function returns %TRUE, registration will proceed; otherwise
 *     registration will abort. Since: 2.34
 * @dbus_unregister: invoked locally during unregistration, if the application
 *     is using its D-Bus backend. Use this to undo anything done by the
 *     @dbus_register vfunc. Since: 2.34
 *
 * Virtual function table for #GApplication.
 *
 * Since: 2.28
 */

struct _GApplicationPrivate
{
  GApplicationFlags  flags;
  gchar             *id;

  GActionGroup      *actions;
  GMenuModel        *app_menu;
  GMenuModel        *menubar;

  guint              inactivity_timeout_id;
  guint              inactivity_timeout;
  guint              use_count;
  guint              busy_count;

  guint              is_registered : 1;
  guint              is_remote : 1;
  guint              did_startup : 1;
  guint              did_shutdown : 1;
  guint              must_quit_now : 1;

  GRemoteActionGroup *remote_actions;
  GApplicationImpl   *impl;
};

enum
{
  PROP_NONE,
  PROP_APPLICATION_ID,
  PROP_FLAGS,
  PROP_IS_REGISTERED,
  PROP_IS_REMOTE,
  PROP_INACTIVITY_TIMEOUT,
  PROP_ACTION_GROUP
};

enum
{
  SIGNAL_STARTUP,
  SIGNAL_SHUTDOWN,
  SIGNAL_ACTIVATE,
  SIGNAL_OPEN,
  SIGNAL_ACTION,
  SIGNAL_COMMAND_LINE,
  NR_SIGNALS
};

static guint g_application_signals[NR_SIGNALS];

static void g_application_action_group_iface_init (GActionGroupInterface *);
static void g_application_action_map_iface_init (GActionMapInterface *);
G_DEFINE_TYPE_WITH_CODE (GApplication, g_application, G_TYPE_OBJECT,
 G_ADD_PRIVATE (GApplication)
 G_IMPLEMENT_INTERFACE (G_TYPE_ACTION_GROUP, g_application_action_group_iface_init)
 G_IMPLEMENT_INTERFACE (G_TYPE_ACTION_MAP, g_application_action_map_iface_init))

/* GApplicationExportedActions {{{1 */

/* We create a subclass of GSimpleActionGroup that implements
 * GRemoteActionGroup and deals with the platform data using
 * GApplication's before/after_emit vfuncs.  This is the action group we
 * will be exporting.
 *
 * We could implement GRemoteActionGroup on GApplication directly, but
 * this would be potentially extremely confusing to have exposed as part
 * of the public API of GApplication.  We certainly don't want anyone in
 * the same process to be calling these APIs...
 */
typedef GSimpleActionGroupClass GApplicationExportedActionsClass;
typedef struct
{
  GSimpleActionGroup parent_instance;
  GApplication *application;
} GApplicationExportedActions;

static GType g_application_exported_actions_get_type   (void);
static void  g_application_exported_actions_iface_init (GRemoteActionGroupInterface *iface);
G_DEFINE_TYPE_WITH_CODE (GApplicationExportedActions, g_application_exported_actions, G_TYPE_SIMPLE_ACTION_GROUP,
                         G_IMPLEMENT_INTERFACE (G_TYPE_REMOTE_ACTION_GROUP, g_application_exported_actions_iface_init))

static void
g_application_exported_actions_activate_action_full (GRemoteActionGroup *remote,
                                                     const gchar        *action_name,
                                                     GVariant           *parameter,
                                                     GVariant           *platform_data)
{
  GApplicationExportedActions *exported = (GApplicationExportedActions *) remote;

  G_APPLICATION_GET_CLASS (exported->application)
    ->before_emit (exported->application, platform_data);

  g_action_group_activate_action (G_ACTION_GROUP (exported), action_name, parameter);

  G_APPLICATION_GET_CLASS (exported->application)
    ->after_emit (exported->application, platform_data);
}

static void
g_application_exported_actions_change_action_state_full (GRemoteActionGroup *remote,
                                                         const gchar        *action_name,
                                                         GVariant           *value,
                                                         GVariant           *platform_data)
{
  GApplicationExportedActions *exported = (GApplicationExportedActions *) remote;

  G_APPLICATION_GET_CLASS (exported->application)
    ->before_emit (exported->application, platform_data);

  g_action_group_change_action_state (G_ACTION_GROUP (exported), action_name, value);

  G_APPLICATION_GET_CLASS (exported->application)
    ->after_emit (exported->application, platform_data);
}

static void
g_application_exported_actions_init (GApplicationExportedActions *actions)
{
}

static void
g_application_exported_actions_iface_init (GRemoteActionGroupInterface *iface)
{
  iface->activate_action_full = g_application_exported_actions_activate_action_full;
  iface->change_action_state_full = g_application_exported_actions_change_action_state_full;
}

static void
g_application_exported_actions_class_init (GApplicationExportedActionsClass *class)
{
}

static GActionGroup *
g_application_exported_actions_new (GApplication *application)
{
  GApplicationExportedActions *actions;

  actions = g_object_new (g_application_exported_actions_get_type (), NULL);
  actions->application = application;

  return G_ACTION_GROUP (actions);
}

/* vfunc defaults {{{1 */
static void
g_application_real_before_emit (GApplication *application,
                                GVariant     *platform_data)
{
}

static void
g_application_real_after_emit (GApplication *application,
                               GVariant     *platform_data)
{
}

static void
g_application_real_startup (GApplication *application)
{
  application->priv->did_startup = TRUE;
}

static void
g_application_real_shutdown (GApplication *application)
{
  application->priv->did_shutdown = TRUE;
}

static void
g_application_real_activate (GApplication *application)
{
  if (!g_signal_has_handler_pending (application,
                                     g_application_signals[SIGNAL_ACTIVATE],
                                     0, TRUE) &&
      G_APPLICATION_GET_CLASS (application)->activate == g_application_real_activate)
    {
      static gboolean warned;

      if (warned)
        return;

      g_warning ("Your application does not implement "
                 "g_application_activate() and has no handlers connected "
                 "to the 'activate' signal.  It should do one of these.");
      warned = TRUE;
    }
}

static void
g_application_real_open (GApplication  *application,
                         GFile        **files,
                         gint           n_files,
                         const gchar   *hint)
{
  if (!g_signal_has_handler_pending (application,
                                     g_application_signals[SIGNAL_OPEN],
                                     0, TRUE) &&
      G_APPLICATION_GET_CLASS (application)->open == g_application_real_open)
    {
      static gboolean warned;

      if (warned)
        return;

      g_warning ("Your application claims to support opening files "
                 "but does not implement g_application_open() and has no "
                 "handlers connected to the 'open' signal.");
      warned = TRUE;
    }
}

static int
g_application_real_command_line (GApplication            *application,
                                 GApplicationCommandLine *cmdline)
{
  if (!g_signal_has_handler_pending (application,
                                     g_application_signals[SIGNAL_COMMAND_LINE],
                                     0, TRUE) &&
      G_APPLICATION_GET_CLASS (application)->command_line == g_application_real_command_line)
    {
      static gboolean warned;

      if (warned)
        return 1;

      g_warning ("Your application claims to support custom command line "
                 "handling but does not implement g_application_command_line() "
                 "and has no handlers connected to the 'command-line' signal.");

      warned = TRUE;
    }

    return 1;
}

static gboolean
g_application_real_local_command_line (GApplication   *application,
                                       gchar        ***arguments,
                                       int            *exit_status)
{
  if (application->priv->flags & G_APPLICATION_HANDLES_COMMAND_LINE)
    return FALSE;

  else
    {
      GError *error = NULL;
      gint n_args;

      if (!g_application_register (application, NULL, &error))
        {
          g_critical ("%s", error->message);
          g_error_free (error);
          *exit_status = 1;
          return TRUE;
        }

      n_args = g_strv_length (*arguments);

      if (application->priv->flags & G_APPLICATION_IS_SERVICE)
        {
          if ((*exit_status = n_args > 1))
            {
              g_printerr ("GApplication service mode takes no arguments.\n");
              application->priv->flags &= ~G_APPLICATION_IS_SERVICE;
            }

          return TRUE;
        }

      if (n_args <= 1)
        {
          g_application_activate (application);
          *exit_status = 0;
        }

      else
        {
          if (~application->priv->flags & G_APPLICATION_HANDLES_OPEN)
            {
              g_critical ("This application can not open files.");
              *exit_status = 1;
            }
          else
            {
              GFile **files;
              gint n_files;
              gint i;

              n_files = n_args - 1;
              files = g_new (GFile *, n_files);

              for (i = 0; i < n_files; i++)
                files[i] = g_file_new_for_commandline_arg ((*arguments)[i + 1]);

              g_application_open (application, files, n_files, "");

              for (i = 0; i < n_files; i++)
                g_object_unref (files[i]);
              g_free (files);

              *exit_status = 0;
            }
        }

      return TRUE;
    }
}

static void
g_application_real_add_platform_data (GApplication    *application,
                                      GVariantBuilder *builder)
{
}

static gboolean
g_application_real_dbus_register (GApplication    *application,
                                  GDBusConnection *connection,
                                  const gchar     *object_path,
                                  GError         **error)
{
  return TRUE;
}

static void
g_application_real_dbus_unregister (GApplication    *application,
                                    GDBusConnection *connection,
                                    const gchar     *object_path)
{
}

/* GObject implementation stuff {{{1 */
static void
g_application_set_property (GObject      *object,
                            guint         prop_id,
                            const GValue *value,
                            GParamSpec   *pspec)
{
  GApplication *application = G_APPLICATION (object);

  switch (prop_id)
    {
    case PROP_APPLICATION_ID:
      g_application_set_application_id (application,
                                        g_value_get_string (value));
      break;

    case PROP_FLAGS:
      g_application_set_flags (application, g_value_get_flags (value));
      break;

    case PROP_INACTIVITY_TIMEOUT:
      g_application_set_inactivity_timeout (application,
                                            g_value_get_uint (value));
      break;

    case PROP_ACTION_GROUP:
      g_clear_object (&application->priv->actions);
      application->priv->actions = g_value_dup_object (value);
      break;

    default:
      g_assert_not_reached ();
    }
}

/**
 * g_application_set_action_group:
 * @application: a #GApplication
 * @action_group: (allow-none): a #GActionGroup, or %NULL
 *
 * This used to be how actions were associated with a #GApplication.
 * Now there is #GActionMap for that.
 *
 * Since: 2.28
 *
 * Deprecated:2.32:Use the #GActionMap interface instead.  Never ever
 * mix use of this API with use of #GActionMap on the same @application
 * or things will go very badly wrong.  This function is known to
 * introduce buggy behaviour (ie: signals not emitted on changes to the
 * action group), so you should really use #GActionMap instead.
 **/
void
g_application_set_action_group (GApplication *application,
                                GActionGroup *action_group)
{
  g_return_if_fail (G_IS_APPLICATION (application));
  g_return_if_fail (!application->priv->is_registered);

  if (application->priv->actions != NULL)
    g_object_unref (application->priv->actions);

  application->priv->actions = action_group;

  if (application->priv->actions != NULL)
    g_object_ref (application->priv->actions);
}

static void
g_application_get_property (GObject    *object,
                            guint       prop_id,
                            GValue     *value,
                            GParamSpec *pspec)
{
  GApplication *application = G_APPLICATION (object);

  switch (prop_id)
    {
    case PROP_APPLICATION_ID:
      g_value_set_string (value,
                          g_application_get_application_id (application));
      break;

    case PROP_FLAGS:
      g_value_set_flags (value,
                         g_application_get_flags (application));
      break;

    case PROP_IS_REGISTERED:
      g_value_set_boolean (value,
                           g_application_get_is_registered (application));
      break;

    case PROP_IS_REMOTE:
      g_value_set_boolean (value,
                           g_application_get_is_remote (application));
      break;

    case PROP_INACTIVITY_TIMEOUT:
      g_value_set_uint (value,
                        g_application_get_inactivity_timeout (application));
      break;

    default:
      g_assert_not_reached ();
    }
}

static void
g_application_constructed (GObject *object)
{
  GApplication *application = G_APPLICATION (object);

  if (g_application_get_default () == NULL)
    g_application_set_default (application);
}

static void
g_application_finalize (GObject *object)
{
  GApplication *application = G_APPLICATION (object);

  if (application->priv->impl)
    g_application_impl_destroy (application->priv->impl);
  g_free (application->priv->id);

  if (g_application_get_default () == application)
    g_application_set_default (NULL);

  if (application->priv->actions)
    g_object_unref (application->priv->actions);

  G_OBJECT_CLASS (g_application_parent_class)
    ->finalize (object);
}

static void
g_application_init (GApplication *application)
{
  application->priv = g_application_get_instance_private (application);

  application->priv->actions = g_application_exported_actions_new (application);

  /* application->priv->actions is the one and only ref on the group, so when
   * we dispose, the action group will die, disconnecting all signals.
   */
  g_signal_connect_swapped (application->priv->actions, "action-added",
                            G_CALLBACK (g_action_group_action_added), application);
  g_signal_connect_swapped (application->priv->actions, "action-enabled-changed",
                            G_CALLBACK (g_action_group_action_enabled_changed), application);
  g_signal_connect_swapped (application->priv->actions, "action-state-changed",
                            G_CALLBACK (g_action_group_action_state_changed), application);
  g_signal_connect_swapped (application->priv->actions, "action-removed",
                            G_CALLBACK (g_action_group_action_removed), application);
}

static void
g_application_class_init (GApplicationClass *class)
{
  GObjectClass *object_class = G_OBJECT_CLASS (class);

  object_class->constructed = g_application_constructed;
  object_class->finalize = g_application_finalize;
  object_class->get_property = g_application_get_property;
  object_class->set_property = g_application_set_property;

  class->before_emit = g_application_real_before_emit;
  class->after_emit = g_application_real_after_emit;
  class->startup = g_application_real_startup;
  class->shutdown = g_application_real_shutdown;
  class->activate = g_application_real_activate;
  class->open = g_application_real_open;
  class->command_line = g_application_real_command_line;
  class->local_command_line = g_application_real_local_command_line;
  class->add_platform_data = g_application_real_add_platform_data;
  class->dbus_register = g_application_real_dbus_register;
  class->dbus_unregister = g_application_real_dbus_unregister;

  g_object_class_install_property (object_class, PROP_APPLICATION_ID,
    g_param_spec_string ("application-id",
                         P_("Application identifier"),
                         P_("The unique identifier for the application"),
                         NULL, G_PARAM_READWRITE | G_PARAM_CONSTRUCT |
                         G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (object_class, PROP_FLAGS,
    g_param_spec_flags ("flags",
                        P_("Application flags"),
                        P_("Flags specifying the behaviour of the application"),
                        G_TYPE_APPLICATION_FLAGS, G_APPLICATION_FLAGS_NONE,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (object_class, PROP_IS_REGISTERED,
    g_param_spec_boolean ("is-registered",
                          P_("Is registered"),
                          P_("If g_application_register() has been called"),
                          FALSE, G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (object_class, PROP_IS_REMOTE,
    g_param_spec_boolean ("is-remote",
                          P_("Is remote"),
                          P_("If this application instance is remote"),
                          FALSE, G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (object_class, PROP_INACTIVITY_TIMEOUT,
    g_param_spec_uint ("inactivity-timeout",
                       P_("Inactivity timeout"),
                       P_("Time (ms) to stay alive after becoming idle"),
                       0, G_MAXUINT, 0,
                       G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (object_class, PROP_ACTION_GROUP,
    g_param_spec_object ("action-group",
                         P_("Action group"),
                         P_("The group of actions that the application exports"),
                         G_TYPE_ACTION_GROUP,
                         G_PARAM_DEPRECATED | G_PARAM_WRITABLE | G_PARAM_STATIC_STRINGS));

  /**
   * GApplication::startup:
   * @application: the application
   *
   * The ::startup signal is emitted on the primary instance immediately
   * after registration. See g_application_register().
   */
  g_application_signals[SIGNAL_STARTUP] =
    g_signal_new ("startup", G_TYPE_APPLICATION, G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (GApplicationClass, startup),
                  NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  /**
   * GApplication::shutdown:
   * @application: the application
   *
   * The ::shutdown signal is emitted only on the registered primary instance
   * immediately after the main loop terminates.
   */
  g_application_signals[SIGNAL_SHUTDOWN] =
    g_signal_new ("shutdown", G_TYPE_APPLICATION, G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (GApplicationClass, shutdown),
                  NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  /**
   * GApplication::activate:
   * @application: the application
   *
   * The ::activate signal is emitted on the primary instance when an
   * activation occurs. See g_application_activate().
   */
  g_application_signals[SIGNAL_ACTIVATE] =
    g_signal_new ("activate", G_TYPE_APPLICATION, G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (GApplicationClass, activate),
                  NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);


  /**
   * GApplication::open:
   * @application: the application
   * @files: (array length=n_files) (element-type GFile): an array of #GFiles
   * @n_files: the length of @files
   * @hint: a hint provided by the calling instance
   *
   * The ::open signal is emitted on the primary instance when there are
   * files to open. See g_application_open() for more information.
   */
  g_application_signals[SIGNAL_OPEN] =
    g_signal_new ("open", G_TYPE_APPLICATION, G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (GApplicationClass, open),
                  NULL, NULL, NULL,
                  G_TYPE_NONE, 3, G_TYPE_POINTER, G_TYPE_INT, G_TYPE_STRING);

  /**
   * GApplication::command-line:
   * @application: the application
   * @command_line: a #GApplicationCommandLine representing the
   *     passed commandline
   *
   * The ::command-line signal is emitted on the primary instance when
   * a commandline is not handled locally. See g_application_run() and
   * the #GApplicationCommandLine documentation for more information.
   *
   * Returns: An integer that is set as the exit status for the calling
   *   process. See g_application_command_line_set_exit_status().
   */
  g_application_signals[SIGNAL_COMMAND_LINE] =
    g_signal_new ("command-line", G_TYPE_APPLICATION, G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (GApplicationClass, command_line),
                  g_signal_accumulator_first_wins, NULL,
                  NULL,
                  G_TYPE_INT, 1, G_TYPE_APPLICATION_COMMAND_LINE);
}

static GVariant *
get_platform_data (GApplication *application)
{
  GVariantBuilder *builder;
  GVariant *result;

  builder = g_variant_builder_new (G_VARIANT_TYPE ("a{sv}"));

  {
    gchar *cwd = g_get_current_dir ();
    g_variant_builder_add (builder, "{sv}", "cwd",
                           g_variant_new_bytestring (cwd));
    g_free (cwd);
  }

  if (application->priv->flags & G_APPLICATION_SEND_ENVIRONMENT)
    {
      GVariant *array;
      gchar **envp;

      envp = g_get_environ ();
      array = g_variant_new_bytestring_array ((const gchar **) envp, -1);
      g_strfreev (envp);

      g_variant_builder_add (builder, "{sv}", "environ", array);
    }

  G_APPLICATION_GET_CLASS (application)->
    add_platform_data (application, builder);

  result = g_variant_builder_end (builder);
  g_variant_builder_unref (builder);

  return result;
}

/* Application ID validity {{{1 */

/**
 * g_application_id_is_valid:
 * @application_id: a potential application identifier
 *
 * Checks if @application_id is a valid application identifier.
 *
 * A valid ID is required for calls to g_application_new() and
 * g_application_set_application_id().
 *
 * For convenience, the restrictions on application identifiers are
 * reproduced here:
 * <itemizedlist>
 *   <listitem>Application identifiers must contain only the ASCII characters "[A-Z][a-z][0-9]_-." and must not begin with a digit.</listitem>
 *   <listitem>Application identifiers must contain at least one '.' (period) character (and thus at least three elements).</listitem>
 *   <listitem>Application identifiers must not begin or end with a '.' (period) character.</listitem>
 *   <listitem>Application identifiers must not contain consecutive '.' (period) characters.</listitem>
 *   <listitem>Application identifiers must not exceed 255 characters.</listitem>
 * </itemizedlist>
 *
 * Returns: %TRUE if @application_id is valid
 **/
gboolean
g_application_id_is_valid (const gchar *application_id)
{
  gsize len;
  gboolean allow_dot;
  gboolean has_dot;

  len = strlen (application_id);

  if (len > 255)
    return FALSE;

  if (!g_ascii_isalpha (application_id[0]))
    return FALSE;

  if (application_id[len-1] == '.')
    return FALSE;

  application_id++;
  allow_dot = TRUE;
  has_dot = FALSE;
  for (; *application_id; application_id++)
    {
      if (g_ascii_isalnum (*application_id) ||
          (*application_id == '-') ||
          (*application_id == '_'))
        {
          allow_dot = TRUE;
        }
      else if (allow_dot && *application_id == '.')
        {
          has_dot = TRUE;
          allow_dot = FALSE;
        }
      else
        return FALSE;
    }

  if (!has_dot)
    return FALSE;

  return TRUE;
}

/* Public Constructor {{{1 */
/**
 * g_application_new:
 * @application_id: (allow-none): the application id
 * @flags: the application flags
 *
 * Creates a new #GApplication instance.
 *
 * If non-%NULL, the application id must be valid.  See
 * g_application_id_is_valid().
 *
 * If no application ID is given then some features of #GApplication
 * (most notably application uniqueness) will be disabled.
 *
 * Returns: a new #GApplication instance
 **/
GApplication *
g_application_new (const gchar       *application_id,
                   GApplicationFlags  flags)
{
  g_return_val_if_fail (application_id == NULL || g_application_id_is_valid (application_id), NULL);

  return g_object_new (G_TYPE_APPLICATION,
                       "application-id", application_id,
                       "flags", flags,
                       NULL);
}

/* Simple get/set: application id, flags, inactivity timeout {{{1 */
/**
 * g_application_get_application_id:
 * @application: a #GApplication
 *
 * Gets the unique identifier for @application.
 *
 * Returns: the identifier for @application, owned by @application
 *
 * Since: 2.28
 **/
const gchar *
g_application_get_application_id (GApplication *application)
{
  g_return_val_if_fail (G_IS_APPLICATION (application), NULL);

  return application->priv->id;
}

/**
 * g_application_set_application_id:
 * @application: a #GApplication
 * @application_id: (allow-none): the identifier for @application
 *
 * Sets the unique identifier for @application.
 *
 * The application id can only be modified if @application has not yet
 * been registered.
 *
 * If non-%NULL, the application id must be valid.  See
 * g_application_id_is_valid().
 *
 * Since: 2.28
 **/
void
g_application_set_application_id (GApplication *application,
                                  const gchar  *application_id)
{
  g_return_if_fail (G_IS_APPLICATION (application));

  if (g_strcmp0 (application->priv->id, application_id) != 0)
    {
      g_return_if_fail (application_id == NULL || g_application_id_is_valid (application_id));
      g_return_if_fail (!application->priv->is_registered);

      g_free (application->priv->id);
      application->priv->id = g_strdup (application_id);

      g_object_notify (G_OBJECT (application), "application-id");
    }
}

/**
 * g_application_get_flags:
 * @application: a #GApplication
 *
 * Gets the flags for @application.
 *
 * See #GApplicationFlags.
 *
 * Returns: the flags for @application
 *
 * Since: 2.28
 **/
GApplicationFlags
g_application_get_flags (GApplication *application)
{
  g_return_val_if_fail (G_IS_APPLICATION (application), 0);

  return application->priv->flags;
}

/**
 * g_application_set_flags:
 * @application: a #GApplication
 * @flags: the flags for @application
 *
 * Sets the flags for @application.
 *
 * The flags can only be modified if @application has not yet been
 * registered.
 *
 * See #GApplicationFlags.
 *
 * Since: 2.28
 **/
void
g_application_set_flags (GApplication      *application,
                         GApplicationFlags  flags)
{
  g_return_if_fail (G_IS_APPLICATION (application));

  if (application->priv->flags != flags)
    {
      g_return_if_fail (!application->priv->is_registered);

      application->priv->flags = flags;

      g_object_notify (G_OBJECT (application), "flags");
    }
}

/**
 * g_application_get_inactivity_timeout:
 * @application: a #GApplication
 *
 * Gets the current inactivity timeout for the application.
 *
 * This is the amount of time (in milliseconds) after the last call to
 * g_application_release() before the application stops running.
 *
 * Returns: the timeout, in milliseconds
 *
 * Since: 2.28
 **/
guint
g_application_get_inactivity_timeout (GApplication *application)
{
  g_return_val_if_fail (G_IS_APPLICATION (application), 0);

  return application->priv->inactivity_timeout;
}

/**
 * g_application_set_inactivity_timeout:
 * @application: a #GApplication
 * @inactivity_timeout: the timeout, in milliseconds
 *
 * Sets the current inactivity timeout for the application.
 *
 * This is the amount of time (in milliseconds) after the last call to
 * g_application_release() before the application stops running.
 *
 * This call has no side effects of its own.  The value set here is only
 * used for next time g_application_release() drops the use count to
 * zero.  Any timeouts currently in progress are not impacted.
 *
 * Since: 2.28
 **/
void
g_application_set_inactivity_timeout (GApplication *application,
                                      guint         inactivity_timeout)
{
  g_return_if_fail (G_IS_APPLICATION (application));

  if (application->priv->inactivity_timeout != inactivity_timeout)
    {
      application->priv->inactivity_timeout = inactivity_timeout;

      g_object_notify (G_OBJECT (application), "inactivity-timeout");
    }
}
/* Read-only property getters (is registered, is remote, dbus stuff) {{{1 */
/**
 * g_application_get_is_registered:
 * @application: a #GApplication
 *
 * Checks if @application is registered.
 *
 * An application is registered if g_application_register() has been
 * successfully called.
 *
 * Returns: %TRUE if @application is registered
 *
 * Since: 2.28
 **/
gboolean
g_application_get_is_registered (GApplication *application)
{
  g_return_val_if_fail (G_IS_APPLICATION (application), FALSE);

  return application->priv->is_registered;
}

/**
 * g_application_get_is_remote:
 * @application: a #GApplication
 *
 * Checks if @application is remote.
 *
 * If @application is remote then it means that another instance of
 * application already exists (the 'primary' instance).  Calls to
 * perform actions on @application will result in the actions being
 * performed by the primary instance.
 *
 * The value of this property cannot be accessed before
 * g_application_register() has been called.  See
 * g_application_get_is_registered().
 *
 * Returns: %TRUE if @application is remote
 *
 * Since: 2.28
 **/
gboolean
g_application_get_is_remote (GApplication *application)
{
  g_return_val_if_fail (G_IS_APPLICATION (application), FALSE);
  g_return_val_if_fail (application->priv->is_registered, FALSE);

  return application->priv->is_remote;
}

/**
 * g_application_get_dbus_connection:
 * @application: a #GApplication
 *
 * Gets the #GDBusConnection being used by the application, or %NULL.
 *
 * If #GApplication is using its D-Bus backend then this function will
 * return the #GDBusConnection being used for uniqueness and
 * communication with the desktop environment and other instances of the
 * application.
 *
 * If #GApplication is not using D-Bus then this function will return
 * %NULL.  This includes the situation where the D-Bus backend would
 * normally be in use but we were unable to connect to the bus.
 *
 * This function must not be called before the application has been
 * registered.  See g_application_get_is_registered().
 *
 * Returns: (transfer none): a #GDBusConnection, or %NULL
 *
 * Since: 2.34
 **/
GDBusConnection *
g_application_get_dbus_connection (GApplication *application)
{
  g_return_val_if_fail (G_IS_APPLICATION (application), FALSE);
  g_return_val_if_fail (application->priv->is_registered, FALSE);

  return g_application_impl_get_dbus_connection (application->priv->impl);
}

/**
 * g_application_get_dbus_object_path:
 * @application: a #GApplication
 *
 * Gets the D-Bus object path being used by the application, or %NULL.
 *
 * If #GApplication is using its D-Bus backend then this function will
 * return the D-Bus object path that #GApplication is using.  If the
 * application is the primary instance then there is an object published
 * at this path.  If the application is not the primary instance then
 * the result of this function is undefined.
 *
 * If #GApplication is not using D-Bus then this function will return
 * %NULL.  This includes the situation where the D-Bus backend would
 * normally be in use but we were unable to connect to the bus.
 *
 * This function must not be called before the application has been
 * registered.  See g_application_get_is_registered().
 *
 * Returns: the object path, or %NULL
 *
 * Since: 2.34
 **/
const gchar *
g_application_get_dbus_object_path (GApplication *application)
{
  g_return_val_if_fail (G_IS_APPLICATION (application), FALSE);
  g_return_val_if_fail (application->priv->is_registered, FALSE);

  return g_application_impl_get_dbus_object_path (application->priv->impl);
}

/* Register {{{1 */
/**
 * g_application_register:
 * @application: a #GApplication
 * @cancellable: (allow-none): a #GCancellable, or %NULL
 * @error: a pointer to a NULL #GError, or %NULL
 *
 * Attempts registration of the application.
 *
 * This is the point at which the application discovers if it is the
 * primary instance or merely acting as a remote for an already-existing
 * primary instance.  This is implemented by attempting to acquire the
 * application identifier as a unique bus name on the session bus using
 * GDBus.
 *
 * If there is no application ID or if %G_APPLICATION_NON_UNIQUE was
 * given, then this process will always become the primary instance.
 *
 * Due to the internal architecture of GDBus, method calls can be
 * dispatched at any time (even if a main loop is not running).  For
 * this reason, you must ensure that any object paths that you wish to
 * register are registered before calling this function.
 *
 * If the application has already been registered then %TRUE is
 * returned with no work performed.
 *
 * The #GApplication::startup signal is emitted if registration succeeds
 * and @application is the primary instance (including the non-unique
 * case).
 *
 * In the event of an error (such as @cancellable being cancelled, or a
 * failure to connect to the session bus), %FALSE is returned and @error
 * is set appropriately.
 *
 * Note: the return value of this function is not an indicator that this
 * instance is or is not the primary instance of the application.  See
 * g_application_get_is_remote() for that.
 *
 * Returns: %TRUE if registration succeeded
 *
 * Since: 2.28
 **/
gboolean
g_application_register (GApplication  *application,
                        GCancellable  *cancellable,
                        GError       **error)
{
  g_return_val_if_fail (G_IS_APPLICATION (application), FALSE);

  if (!application->priv->is_registered)
    {
      if (application->priv->id == NULL)
        application->priv->flags |= G_APPLICATION_NON_UNIQUE;

      application->priv->impl =
        g_application_impl_register (application, application->priv->id,
                                     application->priv->flags,
                                     application->priv->actions,
                                     &application->priv->remote_actions,
                                     cancellable, error);

      if (application->priv->impl == NULL)
        return FALSE;

      application->priv->is_remote = application->priv->remote_actions != NULL;
      application->priv->is_registered = TRUE;

      g_object_notify (G_OBJECT (application), "is-registered");

      if (!application->priv->is_remote)
        {
          g_signal_emit (application, g_application_signals[SIGNAL_STARTUP], 0);

          if (!application->priv->did_startup)
            g_critical ("GApplication subclass '%s' failed to chain up on"
                        " ::startup (from start of override function)",
                        G_OBJECT_TYPE_NAME (application));
        }
    }

  return TRUE;
}

/* Hold/release {{{1 */
/**
 * g_application_hold:
 * @application: a #GApplication
 *
 * Increases the use count of @application.
 *
 * Use this function to indicate that the application has a reason to
 * continue to run.  For example, g_application_hold() is called by GTK+
 * when a toplevel window is on the screen.
 *
 * To cancel the hold, call g_application_release().
 **/
void
g_application_hold (GApplication *application)
{
  g_return_if_fail (G_IS_APPLICATION (application));

  if (application->priv->inactivity_timeout_id)
    {
      g_source_remove (application->priv->inactivity_timeout_id);
      application->priv->inactivity_timeout_id = 0;
    }

  application->priv->use_count++;
}

static gboolean
inactivity_timeout_expired (gpointer data)
{
  GApplication *application = G_APPLICATION (data);

  application->priv->inactivity_timeout_id = 0;

  return G_SOURCE_REMOVE;
}


/**
 * g_application_release:
 * @application: a #GApplication
 *
 * Decrease the use count of @application.
 *
 * When the use count reaches zero, the application will stop running.
 *
 * Never call this function except to cancel the effect of a previous
 * call to g_application_hold().
 **/
void
g_application_release (GApplication *application)
{
  g_return_if_fail (G_IS_APPLICATION (application));

  application->priv->use_count--;

  if (application->priv->use_count == 0 && application->priv->inactivity_timeout)
    application->priv->inactivity_timeout_id = g_timeout_add (application->priv->inactivity_timeout,
                                                              inactivity_timeout_expired, application);
}

/* Activate, Open {{{1 */
/**
 * g_application_activate:
 * @application: a #GApplication
 *
 * Activates the application.
 *
 * In essence, this results in the #GApplication::activate signal being
 * emitted in the primary instance.
 *
 * The application must be registered before calling this function.
 *
 * Since: 2.28
 **/
void
g_application_activate (GApplication *application)
{
  g_return_if_fail (G_IS_APPLICATION (application));
  g_return_if_fail (application->priv->is_registered);

  if (application->priv->is_remote)
    g_application_impl_activate (application->priv->impl,
                                 get_platform_data (application));

  else
    g_signal_emit (application, g_application_signals[SIGNAL_ACTIVATE], 0);
}

/**
 * g_application_open:
 * @application: a #GApplication
 * @files: (array length=n_files): an array of #GFiles to open
 * @n_files: the length of the @files array
 * @hint: a hint (or ""), but never %NULL
 *
 * Opens the given files.
 *
 * In essence, this results in the #GApplication::open signal being emitted
 * in the primary instance.
 *
 * @n_files must be greater than zero.
 *
 * @hint is simply passed through to the ::open signal.  It is
 * intended to be used by applications that have multiple modes for
 * opening files (eg: "view" vs "edit", etc).  Unless you have a need
 * for this functionality, you should use "".
 *
 * The application must be registered before calling this function
 * and it must have the %G_APPLICATION_HANDLES_OPEN flag set.
 *
 * Since: 2.28
 **/
void
g_application_open (GApplication  *application,
                    GFile        **files,
                    gint           n_files,
                    const gchar   *hint)
{
  g_return_if_fail (G_IS_APPLICATION (application));
  g_return_if_fail (application->priv->flags &
                    G_APPLICATION_HANDLES_OPEN);
  g_return_if_fail (application->priv->is_registered);

  if (application->priv->is_remote)
    g_application_impl_open (application->priv->impl,
                             files, n_files, hint,
                             get_platform_data (application));

  else
    g_signal_emit (application, g_application_signals[SIGNAL_OPEN],
                   0, files, n_files, hint);
}

/* Run {{{1 */
/**
 * g_application_run:
 * @application: a #GApplication
 * @argc: the argc from main() (or 0 if @argv is %NULL)
 * @argv: (array length=argc) (allow-none): the argv from main(), or %NULL
 *
 * Runs the application.
 *
 * This function is intended to be run from main() and its return value
 * is intended to be returned by main(). Although you are expected to pass
 * the @argc, @argv parameters from main() to this function, it is possible
 * to pass %NULL if @argv is not available or commandline handling is not
 * required.
 *
 * First, the local_command_line() virtual function is invoked.
 * This function always runs on the local instance. It gets passed a pointer
 * to a %NULL-terminated copy of @argv and is expected to remove the arguments
 * that it handled (shifting up remaining arguments). See
 * <xref linkend="gapplication-example-cmdline2"/> for an example of
 * parsing @argv manually. Alternatively, you may use the #GOptionContext API,
 * after setting <literal>argc = g_strv_length (argv);</literal>.
 *
 * The last argument to local_command_line() is a pointer to the @status
 * variable which can used to set the exit status that is returned from
 * g_application_run().
 *
 * If local_command_line() returns %TRUE, the command line is expected
 * to be completely handled, including possibly registering as the primary
 * instance, calling g_application_activate() or g_application_open(), etc.
 *
 * If local_command_line() returns %FALSE then the application is registered
 * and the #GApplication::command-line signal is emitted in the primary
 * instance (which may or may not be this instance). The signal handler
 * gets passed a #GApplicationCommandLine object that (among other things)
 * contains the remaining commandline arguments that have not been handled
 * by local_command_line().
 *
 * If the application has the %G_APPLICATION_HANDLES_COMMAND_LINE
 * flag set then the default implementation of local_command_line()
 * always returns %FALSE immediately, resulting in the commandline
 * always being handled in the primary instance.
 *
 * Otherwise, the default implementation of local_command_line() tries
 * to do a couple of things that are probably reasonable for most
 * applications.  First, g_application_register() is called to attempt
 * to register the application.  If that works, then the command line
 * arguments are inspected.  If no commandline arguments are given, then
 * g_application_activate() is called.  If commandline arguments are
 * given and the %G_APPLICATION_HANDLES_OPEN flag is set then they
 * are assumed to be filenames and g_application_open() is called.
 *
 * If you need to handle commandline arguments that are not filenames,
 * and you don't mind commandline handling to happen in the primary
 * instance, you should set %G_APPLICATION_HANDLES_COMMAND_LINE and
 * process the commandline arguments in your #GApplication::command-line
 * signal handler, either manually or using the #GOptionContext API.
 *
 * If you are interested in doing more complicated local handling of the
 * commandline then you should implement your own #GApplication subclass
 * and override local_command_line(). In this case, you most likely want
 * to return %TRUE from your local_command_line() implementation to
 * suppress the default handling. See
 * <xref linkend="gapplication-example-cmdline2"/> for an example.
 *
 * If, after the above is done, the use count of the application is zero
 * then the exit status is returned immediately.  If the use count is
 * non-zero then the default main context is iterated until the use count
 * falls to zero, at which point 0 is returned.
 *
 * If the %G_APPLICATION_IS_SERVICE flag is set, then the service will
 * run for as much as 10 seconds with a use count of zero while waiting
 * for the message that caused the activation to arrive.  After that,
 * if the use count falls to zero the application will exit immediately,
 * except in the case that g_application_set_inactivity_timeout() is in
 * use.
 *
 * This function sets the prgname (g_set_prgname()), if not already set,
 * to the basename of argv[0].  Since 2.38, if %G_APPLICATION_IS_SERVICE
 * is specified, the prgname is set to the application ID.  The main
 * impact of this is is that the wmclass of windows created by Gtk+ will
 * be set accordingly, which helps the window manager determine which
 * application is showing the window.
 *
 * Returns: the exit status
 *
 * Since: 2.28
 **/
int
g_application_run (GApplication  *application,
                   int            argc,
                   char         **argv)
{
  gchar **arguments;
  int status;
  gint i;

  g_return_val_if_fail (G_IS_APPLICATION (application), 1);
  g_return_val_if_fail (argc == 0 || argv != NULL, 1);
  g_return_val_if_fail (!application->priv->must_quit_now, 1);

  arguments = g_new (gchar *, argc + 1);
  for (i = 0; i < argc; i++)
    arguments[i] = g_strdup (argv[i]);
  arguments[i] = NULL;

  if (g_get_prgname () == NULL)
    {
      if (application->priv->flags & G_APPLICATION_IS_SERVICE)
        {
          g_set_prgname (application->priv->id);
        }
      else if (argc > 0)
        {
          gchar *prgname;

          prgname = g_path_get_basename (argv[0]);
          g_set_prgname (prgname);
          g_free (prgname);
        }
    }

  if (!G_APPLICATION_GET_CLASS (application)
        ->local_command_line (application, &arguments, &status))
    {
      GError *error = NULL;

      if (!g_application_register (application, NULL, &error))
        {
          g_printerr ("%s", error->message);
          g_error_free (error);
          return 1;
        }

      if (application->priv->is_remote)
        {
          GVariant *platform_data;

          platform_data = get_platform_data (application);
          status = g_application_impl_command_line (application->priv->impl,
                                                    arguments, platform_data);
        }
      else
        {
          GApplicationCommandLine *cmdline;
          GVariant *v;

          v = g_variant_new_bytestring_array ((const gchar **) arguments, -1);
          cmdline = g_object_new (G_TYPE_APPLICATION_COMMAND_LINE,
                                  "arguments", v, NULL);
          g_signal_emit (application,
                         g_application_signals[SIGNAL_COMMAND_LINE],
                         0, cmdline, &status);
          g_object_unref (cmdline);
        }
    }

  g_strfreev (arguments);

  if (application->priv->flags & G_APPLICATION_IS_SERVICE &&
      application->priv->is_registered &&
      !application->priv->use_count &&
      !application->priv->inactivity_timeout_id)
    {
      application->priv->inactivity_timeout_id =
        g_timeout_add (10000, inactivity_timeout_expired, application);
    }

  while (application->priv->use_count || application->priv->inactivity_timeout_id)
    {
      if (application->priv->must_quit_now)
        break;

      g_main_context_iteration (NULL, TRUE);
      status = 0;
    }

  if (application->priv->is_registered && !application->priv->is_remote)
    {
      g_signal_emit (application, g_application_signals[SIGNAL_SHUTDOWN], 0);

      if (!application->priv->did_shutdown)
        g_critical ("GApplication subclass '%s' failed to chain up on"
                    " ::shutdown (from end of override function)",
                    G_OBJECT_TYPE_NAME (application));
    }

  if (application->priv->impl)
    g_application_impl_flush (application->priv->impl);

  g_settings_sync ();

  return status;
}

static gchar **
g_application_list_actions (GActionGroup *action_group)
{
  GApplication *application = G_APPLICATION (action_group);

  g_return_val_if_fail (application->priv->is_registered, NULL);

  if (application->priv->remote_actions != NULL)
    return g_action_group_list_actions (G_ACTION_GROUP (application->priv->remote_actions));

  else if (application->priv->actions != NULL)
    return g_action_group_list_actions (application->priv->actions);

  else
    /* empty string array */
    return g_new0 (gchar *, 1);
}

static gboolean
g_application_query_action (GActionGroup        *group,
                            const gchar         *action_name,
                            gboolean            *enabled,
                            const GVariantType **parameter_type,
                            const GVariantType **state_type,
                            GVariant           **state_hint,
                            GVariant           **state)
{
  GApplication *application = G_APPLICATION (group);

  g_return_val_if_fail (application->priv->is_registered, FALSE);

  if (application->priv->remote_actions != NULL)
    return g_action_group_query_action (G_ACTION_GROUP (application->priv->remote_actions),
                                        action_name,
                                        enabled,
                                        parameter_type,
                                        state_type,
                                        state_hint,
                                        state);

  if (application->priv->actions != NULL)
    return g_action_group_query_action (application->priv->actions,
                                        action_name,
                                        enabled,
                                        parameter_type,
                                        state_type,
                                        state_hint,
                                        state);

  return FALSE;
}

static void
g_application_change_action_state (GActionGroup *action_group,
                                   const gchar  *action_name,
                                   GVariant     *value)
{
  GApplication *application = G_APPLICATION (action_group);

  g_return_if_fail (application->priv->is_remote ||
                    application->priv->actions != NULL);
  g_return_if_fail (application->priv->is_registered);

  if (application->priv->remote_actions)
    g_remote_action_group_change_action_state_full (application->priv->remote_actions,
                                                    action_name, value, get_platform_data (application));

  else
    g_action_group_change_action_state (application->priv->actions, action_name, value);
}

static void
g_application_activate_action (GActionGroup *action_group,
                               const gchar  *action_name,
                               GVariant     *parameter)
{
  GApplication *application = G_APPLICATION (action_group);

  g_return_if_fail (application->priv->is_remote ||
                    application->priv->actions != NULL);
  g_return_if_fail (application->priv->is_registered);

  if (application->priv->remote_actions)
    g_remote_action_group_activate_action_full (application->priv->remote_actions,
                                                action_name, parameter, get_platform_data (application));

  else
    g_action_group_activate_action (application->priv->actions, action_name, parameter);
}

static GAction *
g_application_lookup_action (GActionMap  *action_map,
                             const gchar *action_name)
{
  GApplication *application = G_APPLICATION (action_map);

  g_return_val_if_fail (G_IS_ACTION_MAP (application->priv->actions), NULL);

  return g_action_map_lookup_action (G_ACTION_MAP (application->priv->actions), action_name);
}

static void
g_application_add_action (GActionMap *action_map,
                          GAction    *action)
{
  GApplication *application = G_APPLICATION (action_map);

  g_return_if_fail (G_IS_ACTION_MAP (application->priv->actions));

  g_action_map_add_action (G_ACTION_MAP (application->priv->actions), action);
}

static void
g_application_remove_action (GActionMap  *action_map,
                             const gchar *action_name)
{
  GApplication *application = G_APPLICATION (action_map);

  g_return_if_fail (G_IS_ACTION_MAP (application->priv->actions));

  g_action_map_remove_action (G_ACTION_MAP (application->priv->actions), action_name);
}

static void
g_application_action_group_iface_init (GActionGroupInterface *iface)
{
  iface->list_actions = g_application_list_actions;
  iface->query_action = g_application_query_action;
  iface->change_action_state = g_application_change_action_state;
  iface->activate_action = g_application_activate_action;
}

static void
g_application_action_map_iface_init (GActionMapInterface *iface)
{
  iface->lookup_action = g_application_lookup_action;
  iface->add_action = g_application_add_action;
  iface->remove_action = g_application_remove_action;
}

/* Default Application {{{1 */

static GApplication *default_app;

/**
 * g_application_get_default:
 *
 * Returns the default #GApplication instance for this process.
 *
 * Normally there is only one #GApplication per process and it becomes
 * the default when it is created.  You can exercise more control over
 * this by using g_application_set_default().
 *
 * If there is no default application then %NULL is returned.
 *
 * Returns: (transfer none): the default application for this process, or %NULL
 *
 * Since: 2.32
 **/
GApplication *
g_application_get_default (void)
{
  return default_app;
}

/**
 * g_application_set_default:
 * @application: (allow-none): the application to set as default, or %NULL
 *
 * Sets or unsets the default application for the process, as returned
 * by g_application_get_default().
 *
 * This function does not take its own reference on @application.  If
 * @application is destroyed then the default application will revert
 * back to %NULL.
 *
 * Since: 2.32
 **/
void
g_application_set_default (GApplication *application)
{
  default_app = application;
}

/**
 * g_application_quit:
 * @application: a #GApplication
 *
 * Immediately quits the application.
 *
 * Upon return to the mainloop, g_application_run() will return,
 * calling only the 'shutdown' function before doing so.
 *
 * The hold count is ignored.
 *
 * The result of calling g_application_run() again after it returns is
 * unspecified.
 *
 * Since: 2.32
 **/
void
g_application_quit (GApplication *application)
{
  g_return_if_fail (G_IS_APPLICATION (application));

  application->priv->must_quit_now = TRUE;
}

/**
 * g_application_mark_busy:
 * @application: a #GApplication
 *
 * Increases the busy count of @application.
 *
 * Use this function to indicate that the application is busy, for instance
 * while a long running operation is pending.
 *
 * The busy state will be exposed to other processes, so a session shell will
 * use that information to indicate the state to the user (e.g. with a
 * spinner).
 *
 * To cancel the busy indication, use g_application_unmark_busy().
 *
 * Since: 2.38
 **/
void
g_application_mark_busy (GApplication *application)
{
  gboolean was_busy;

  g_return_if_fail (G_IS_APPLICATION (application));

  was_busy = (application->priv->busy_count > 0);
  application->priv->busy_count++;

  if (!was_busy)
    g_application_impl_set_busy_state (application->priv->impl, TRUE);
}

/**
 * g_application_unmark_busy:
 * @application: a #GApplication
 *
 * Decreases the busy count of @application.
 *
 * When the busy count reaches zero, the new state will be propagated
 * to other processes.
 *
 * This function must only be called to cancel the effect of a previous
 * call to g_application_mark_busy().
 *
 * Since: 2.38
 **/
void
g_application_unmark_busy (GApplication *application)
{
  g_return_if_fail (G_IS_APPLICATION (application));
  g_return_if_fail (application->priv->busy_count > 0);

  application->priv->busy_count--;

  if (application->priv->busy_count == 0)
    g_application_impl_set_busy_state (application->priv->impl, FALSE);
}

/* Epilogue {{{1 */
/* vim:set foldmethod=marker: */
