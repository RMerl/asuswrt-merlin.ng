/* GIO - GLib Input, Output and Streaming Library
 *
 * Copyright 2011 Red Hat, Inc
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
 */

#include "config.h"
#include "glib.h"
#include "glibintl.h"

#include "gnetworkmonitor.h"
#include "ginetaddress.h"
#include "ginetsocketaddress.h"
#include "ginitable.h"
#include "gioenumtypes.h"
#include "giomodule-priv.h"
#include "gtask.h"

/**
 * SECTION:gnetworkmonitor
 * @title: GNetworkMonitor
 * @short_description: Network status monitor
 * @include: gio/gio.h
 *
 * #GNetworkMonitor provides an easy-to-use cross-platform API
 * for monitoring network connectivity. On Linux, the implementation
 * is based on the kernel's netlink interface.
 */

/**
 * GNetworkMonitor:
 *
 * #GNetworkMonitor monitors the status of network connections and
 * indicates when a possibly-user-visible change has occurred.
 *
 * Since: 2.32
 */

G_DEFINE_INTERFACE_WITH_CODE (GNetworkMonitor, g_network_monitor, G_TYPE_OBJECT,
                              g_type_interface_add_prerequisite (g_define_type_id, G_TYPE_INITABLE);)


enum {
  NETWORK_CHANGED,
  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

/**
 * g_network_monitor_get_default:
 *
 * Gets the default #GNetworkMonitor for the system.
 *
 * Returns: (transfer none): a #GNetworkMonitor
 *
 * Since: 2.32
 */
GNetworkMonitor *
g_network_monitor_get_default (void)
{
  return _g_io_module_get_default (G_NETWORK_MONITOR_EXTENSION_POINT_NAME,
                                   "GIO_USE_NETWORK_MONITOR",
                                   NULL);
}

/**
 * g_network_monitor_get_network_available:
 * @monitor: the #GNetworkMonitor
 *
 * Checks if the network is available. "Available" here means that the
 * system has a default route available for at least one of IPv4 or
 * IPv6. It does not necessarily imply that the public Internet is
 * reachable. See #GNetworkMonitor:network-available for more details.
 *
 * Return value: whether the network is available
 *
 * Since: 2.32
 */
gboolean
g_network_monitor_get_network_available (GNetworkMonitor *monitor)
{
  gboolean available = FALSE;

  g_object_get (G_OBJECT (monitor), "network-available", &available, NULL);
  return available;
}

/**
 * g_network_monitor_can_reach:
 * @monitor: a #GNetworkMonitor
 * @connectable: a #GSocketConnectable
 * @cancellable: (allow-none): a #GCancellable, or %NULL
 * @error: return location for a #GError, or %NULL
 *
 * Attempts to determine whether or not the host pointed to by
 * @connectable can be reached, without actually trying to connect to
 * it.
 *
 * This may return %TRUE even when #GNetworkMonitor:network-available
 * is %FALSE, if, for example, @monitor can determine that
 * @connectable refers to a host on a local network.
 *
 * If @monitor believes that an attempt to connect to @connectable
 * will succeed, it will return %TRUE. Otherwise, it will return
 * %FALSE and set @error to an appropriate error (such as
 * %G_IO_ERROR_HOST_UNREACHABLE).
 *
 * Note that although this does not attempt to connect to
 * @connectable, it may still block for a brief period of time (eg,
 * trying to do multicast DNS on the local network), so if you do not
 * want to block, you should use g_network_monitor_can_reach_async().
 *
 * Return value: %TRUE if @connectable is reachable, %FALSE if not.
 *
 * Since: 2.32
 */
gboolean
g_network_monitor_can_reach (GNetworkMonitor     *monitor,
                             GSocketConnectable  *connectable,
                             GCancellable        *cancellable,
                             GError             **error)
{
  GNetworkMonitorInterface *iface;

  iface = G_NETWORK_MONITOR_GET_INTERFACE (monitor);
  return iface->can_reach (monitor, connectable, cancellable, error);
}

static void
g_network_monitor_real_can_reach_async (GNetworkMonitor     *monitor,
                                        GSocketConnectable  *connectable,
                                        GCancellable        *cancellable,
                                        GAsyncReadyCallback  callback,
                                        gpointer             user_data)
{
  GTask *task;
  GError *error = NULL;

  task = g_task_new (monitor, cancellable, callback, user_data);
  if (g_network_monitor_can_reach (monitor, connectable, cancellable, &error))
    g_task_return_boolean (task, TRUE);
  else
    g_task_return_error (task, error);
  g_object_unref (task);
}

/**
 * g_network_monitor_can_reach_async:
 * @monitor: a #GNetworkMonitor
 * @connectable: a #GSocketConnectable
 * @cancellable: (allow-none): a #GCancellable, or %NULL
 * @callback: (scope async): a #GAsyncReadyCallback to call when the
 *     request is satisfied
 * @user_data: (closure): the data to pass to callback function
 *
 * Asynchronously attempts to determine whether or not the host
 * pointed to by @connectable can be reached, without actually
 * trying to connect to it.
 *
 * For more details, see g_network_monitor_can_reach().
 *
 * When the operation is finished, @callback will be called.
 * You can then call g_network_monitor_can_reach_finish()
 * to get the result of the operation.
 */
void
g_network_monitor_can_reach_async (GNetworkMonitor     *monitor,
                                   GSocketConnectable  *connectable,
                                   GCancellable        *cancellable,
                                   GAsyncReadyCallback  callback,
                                   gpointer             user_data)
{
  GNetworkMonitorInterface *iface;

  iface = G_NETWORK_MONITOR_GET_INTERFACE (monitor);
  iface->can_reach_async (monitor, connectable, cancellable, callback, user_data);
}

static gboolean
g_network_monitor_real_can_reach_finish (GNetworkMonitor  *monitor,
                                         GAsyncResult     *result,
                                         GError          **error)
{
  g_return_val_if_fail (g_task_is_valid (result, monitor), FALSE);

  return g_task_propagate_boolean (G_TASK (result), error);
}

/**
 * g_network_monitor_can_reach_finish:
 * @monitor: a #GNetworkMonitor
 * @result: a #GAsyncResult
 * @error: return location for errors, or %NULL
 *
 * Finishes an async network connectivity test.
 * See g_network_monitor_can_reach_async().
 *
 * Return value: %TRUE if network is reachable, %FALSE if not.
 */
gboolean
g_network_monitor_can_reach_finish (GNetworkMonitor     *monitor,
                                    GAsyncResult        *result,
                                    GError             **error)
{
  GNetworkMonitorInterface *iface;

  iface = G_NETWORK_MONITOR_GET_INTERFACE (monitor);
  return iface->can_reach_finish (monitor, result, error);
}

static void
g_network_monitor_default_init (GNetworkMonitorInterface *iface)
{
  iface->can_reach_async  = g_network_monitor_real_can_reach_async;
  iface->can_reach_finish = g_network_monitor_real_can_reach_finish;

  /**
   * GNetworkMonitor::network-changed:
   * @monitor: a #GNetworkMonitor
   * @available: the current value of #GNetworkMonitor:network-available
   *
   * Emitted when the network configuration changes. If @available is
   * %TRUE, then some hosts may be reachable that were not reachable
   * before, while others that were reachable before may no longer be
   * reachable. If @available is %FALSE, then no remote hosts are
   * reachable.
   *
   * Since: 2.32
   */
  signals[NETWORK_CHANGED] =
    g_signal_new (I_("network-changed"),
                  G_TYPE_NETWORK_MONITOR,
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (GNetworkMonitorInterface, network_changed),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__BOOLEAN,
                  G_TYPE_NONE, 1,
                  G_TYPE_BOOLEAN);

  /**
   * GNetworkMonitor:network-available:
   *
   * Whether the network is considered available. That is, whether the
   * system has a default route for at least one of IPv4 or IPv6.
   *
   * Real-world networks are of course much more complicated than
   * this; the machine may be connected to a wifi hotspot that
   * requires payment before allowing traffic through, or may be
   * connected to a functioning router that has lost its own upstream
   * connectivity. Some hosts might only be accessible when a VPN is
   * active. Other hosts might only be accessible when the VPN is
   * <emphasis>not</emphasis> active. Thus, it is best to use
   * g_network_monitor_can_reach() or
   * g_network_monitor_can_reach_async() to test for reachability on a
   * host-by-host basis. (On the other hand, when the property is
   * %FALSE, the application can reasonably expect that no remote
   * hosts at all are reachable, and should indicate this to the user
   * in its UI.)
   *
   * See also #GNetworkMonitor::network-changed.
   *
   * Since: 2.32
   */
  g_object_interface_install_property (iface,
                                       g_param_spec_boolean ("network-available",
                                                             P_("Network available"),
                                                             P_("Whether the network is available"),
                                                             FALSE,
                                                             G_PARAM_READABLE |
                                                             G_PARAM_STATIC_STRINGS));
}
