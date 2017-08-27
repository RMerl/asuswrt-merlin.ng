/* GIO - GLib Input, Output and Streaming Library
 *
 * Copyright (C) 2006-2007 Red Hat, Inc.
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
#include "gappinfo.h"
#include "glibintl.h"
#include <gioerror.h>
#include <gfile.h>


/**
 * SECTION:gappinfo
 * @short_description: Application information and launch contexts
 * @include: gio/gio.h
 * 
 * #GAppInfo and #GAppLaunchContext are used for describing and launching
 * applications installed on the system.
 *
 * As of GLib 2.20, URIs will always be converted to POSIX paths
 * (using g_file_get_path()) when using g_app_info_launch() even if
 * the application requested an URI and not a POSIX path. For example
 * for an desktop-file based application with Exec key <literal>totem
 * &percnt;U</literal> and a single URI,
 * <literal>sftp://foo/file.avi</literal>, then
 * <literal>/home/user/.gvfs/sftp on foo/file.avi</literal> will be
 * passed. This will only work if a set of suitable GIO extensions
 * (such as gvfs 2.26 compiled with FUSE support), is available and
 * operational; if this is not the case, the URI will be passed
 * unmodified to the application. Some URIs, such as
 * <literal>mailto:</literal>, of course cannot be mapped to a POSIX
 * path (in gvfs there's no FUSE mount for it); such URIs will be
 * passed unmodified to the application.
 *
 * Specifically for gvfs 2.26 and later, the POSIX URI will be mapped
 * back to the GIO URI in the #GFile constructors (since gvfs
 * implements the #GVfs extension point). As such, if the application
 * needs to examine the URI, it needs to use g_file_get_uri() or
 * similar on #GFile. In other words, an application cannot assume
 * that the URI passed to e.g. g_file_new_for_commandline_arg() is
 * equal to the result of g_file_get_uri(). The following snippet
 * illustrates this:
 *
 * <programlisting>
 * GFile *f;
 * char *uri;
 *
 * file = g_file_new_for_commandline_arg (uri_from_commandline);
 *
 * uri = g_file_get_uri (file);
 * strcmp (uri, uri_from_commandline) == 0; // FALSE
 * g_free (uri);
 *
 * if (g_file_has_uri_scheme (file, "cdda"))
 *   {
 *     // do something special with uri
 *   }
 * g_object_unref (file);
 * </programlisting>
 *
 * This code will work when both <literal>cdda://sr0/Track
 * 1.wav</literal> and <literal>/home/user/.gvfs/cdda on sr0/Track
 * 1.wav</literal> is passed to the application. It should be noted
 * that it's generally not safe for applications to rely on the format
 * of a particular URIs. Different launcher applications (e.g. file
 * managers) may have different ideas of what a given URI means.
 *
 **/

typedef GAppInfoIface GAppInfoInterface;
G_DEFINE_INTERFACE (GAppInfo, g_app_info, G_TYPE_OBJECT)

static void
g_app_info_default_init (GAppInfoInterface *iface)
{
}


/**
 * g_app_info_dup:
 * @appinfo: a #GAppInfo.
 * 
 * Creates a duplicate of a #GAppInfo.
 *
 * Returns: (transfer full): a duplicate of @appinfo.
 **/
GAppInfo *
g_app_info_dup (GAppInfo *appinfo)
{
  GAppInfoIface *iface;

  g_return_val_if_fail (G_IS_APP_INFO (appinfo), NULL);

  iface = G_APP_INFO_GET_IFACE (appinfo);

  return (* iface->dup) (appinfo);
}

/**
 * g_app_info_equal:
 * @appinfo1: the first #GAppInfo.
 * @appinfo2: the second #GAppInfo.
 *
 * Checks if two #GAppInfo<!-- -->s are equal.
 *
 * Returns: %TRUE if @appinfo1 is equal to @appinfo2. %FALSE otherwise.
 **/
gboolean
g_app_info_equal (GAppInfo *appinfo1,
		  GAppInfo *appinfo2)
{
  GAppInfoIface *iface;

  g_return_val_if_fail (G_IS_APP_INFO (appinfo1), FALSE);
  g_return_val_if_fail (G_IS_APP_INFO (appinfo2), FALSE);

  if (G_TYPE_FROM_INSTANCE (appinfo1) != G_TYPE_FROM_INSTANCE (appinfo2))
    return FALSE;
  
  iface = G_APP_INFO_GET_IFACE (appinfo1);

  return (* iface->equal) (appinfo1, appinfo2);
}

/**
 * g_app_info_get_id:
 * @appinfo: a #GAppInfo.
 * 
 * Gets the ID of an application. An id is a string that
 * identifies the application. The exact format of the id is
 * platform dependent. For instance, on Unix this is the
 * desktop file id from the xdg menu specification.
 *
 * Note that the returned ID may be %NULL, depending on how
 * the @appinfo has been constructed.
 *
 * Returns: a string containing the application's ID.
 **/
const char *
g_app_info_get_id (GAppInfo *appinfo)
{
  GAppInfoIface *iface;
  
  g_return_val_if_fail (G_IS_APP_INFO (appinfo), NULL);

  iface = G_APP_INFO_GET_IFACE (appinfo);

  return (* iface->get_id) (appinfo);
}

/**
 * g_app_info_get_name:
 * @appinfo: a #GAppInfo.
 * 
 * Gets the installed name of the application. 
 *
 * Returns: the name of the application for @appinfo.
 **/
const char *
g_app_info_get_name (GAppInfo *appinfo)
{
  GAppInfoIface *iface;
  
  g_return_val_if_fail (G_IS_APP_INFO (appinfo), NULL);

  iface = G_APP_INFO_GET_IFACE (appinfo);

  return (* iface->get_name) (appinfo);
}

/**
 * g_app_info_get_display_name:
 * @appinfo: a #GAppInfo.
 *
 * Gets the display name of the application. The display name is often more
 * descriptive to the user than the name itself.
 *
 * Returns: the display name of the application for @appinfo, or the name if
 * no display name is available.
 *
 * Since: 2.24
 **/
const char *
g_app_info_get_display_name (GAppInfo *appinfo)
{
  GAppInfoIface *iface;

  g_return_val_if_fail (G_IS_APP_INFO (appinfo), NULL);

  iface = G_APP_INFO_GET_IFACE (appinfo);

  if (iface->get_display_name == NULL)
    return (* iface->get_name) (appinfo);

  return (* iface->get_display_name) (appinfo);
}

/**
 * g_app_info_get_description:
 * @appinfo: a #GAppInfo.
 * 
 * Gets a human-readable description of an installed application.
 *
 * Returns: a string containing a description of the 
 * application @appinfo, or %NULL if none. 
 **/
const char *
g_app_info_get_description (GAppInfo *appinfo)
{
  GAppInfoIface *iface;
  
  g_return_val_if_fail (G_IS_APP_INFO (appinfo), NULL);

  iface = G_APP_INFO_GET_IFACE (appinfo);

  return (* iface->get_description) (appinfo);
}

/**
 * g_app_info_get_executable:
 * @appinfo: a #GAppInfo
 * 
 * Gets the executable's name for the installed application.
 *
 * Returns: a string containing the @appinfo's application 
 * binaries name
 **/
const char *
g_app_info_get_executable (GAppInfo *appinfo)
{
  GAppInfoIface *iface;
  
  g_return_val_if_fail (G_IS_APP_INFO (appinfo), NULL);

  iface = G_APP_INFO_GET_IFACE (appinfo);

  return (* iface->get_executable) (appinfo);
}


/**
 * g_app_info_get_commandline:
 * @appinfo: a #GAppInfo
 * 
 * Gets the commandline with which the application will be
 * started.  
 *
 * Returns: a string containing the @appinfo's commandline, 
 *     or %NULL if this information is not available
 *
 * Since: 2.20
 **/
const char *
g_app_info_get_commandline (GAppInfo *appinfo)
{
  GAppInfoIface *iface;
  
  g_return_val_if_fail (G_IS_APP_INFO (appinfo), NULL);

  iface = G_APP_INFO_GET_IFACE (appinfo);

  if (iface->get_commandline)
    return (* iface->get_commandline) (appinfo);
 
  return NULL;
}

/**
 * g_app_info_set_as_default_for_type:
 * @appinfo: a #GAppInfo.
 * @content_type: the content type.
 * @error: a #GError.
 * 
 * Sets the application as the default handler for a given type.
 *
 * Returns: %TRUE on success, %FALSE on error.
 **/
gboolean
g_app_info_set_as_default_for_type (GAppInfo    *appinfo,
				    const char  *content_type,
				    GError     **error)
{
  GAppInfoIface *iface;
  
  g_return_val_if_fail (G_IS_APP_INFO (appinfo), FALSE);
  g_return_val_if_fail (content_type != NULL, FALSE);

  iface = G_APP_INFO_GET_IFACE (appinfo);

  return (* iface->set_as_default_for_type) (appinfo, content_type, error);
}

/**
 * g_app_info_set_as_last_used_for_type:
 * @appinfo: a #GAppInfo.
 * @content_type: the content type.
 * @error: a #GError.
 *
 * Sets the application as the last used application for a given type.
 * This will make the application appear as first in the list returned
 * by g_app_info_get_recommended_for_type(), regardless of the default
 * application for that content type.
 *
 * Returns: %TRUE on success, %FALSE on error.
 **/
gboolean
g_app_info_set_as_last_used_for_type (GAppInfo    *appinfo,
                                      const char  *content_type,
                                      GError     **error)
{
  GAppInfoIface *iface;
  
  g_return_val_if_fail (G_IS_APP_INFO (appinfo), FALSE);
  g_return_val_if_fail (content_type != NULL, FALSE);

  iface = G_APP_INFO_GET_IFACE (appinfo);

  return (* iface->set_as_last_used_for_type) (appinfo, content_type, error);
}

/**
 * g_app_info_set_as_default_for_extension:
 * @appinfo: a #GAppInfo.
 * @extension: a string containing the file extension (without the dot).
 * @error: a #GError.
 * 
 * Sets the application as the default handler for the given file extension.
 *
 * Returns: %TRUE on success, %FALSE on error.
 **/
gboolean
g_app_info_set_as_default_for_extension (GAppInfo    *appinfo,
					 const char  *extension,
					 GError     **error)
{
  GAppInfoIface *iface;
  
  g_return_val_if_fail (G_IS_APP_INFO (appinfo), FALSE);
  g_return_val_if_fail (extension != NULL, FALSE);

  iface = G_APP_INFO_GET_IFACE (appinfo);

  if (iface->set_as_default_for_extension)
    return (* iface->set_as_default_for_extension) (appinfo, extension, error);

  g_set_error_literal (error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED,
                       "g_app_info_set_as_default_for_extension not supported yet");
  return FALSE;
}


/**
 * g_app_info_add_supports_type:
 * @appinfo: a #GAppInfo.
 * @content_type: a string.
 * @error: a #GError.
 * 
 * Adds a content type to the application information to indicate the 
 * application is capable of opening files with the given content type.
 *
 * Returns: %TRUE on success, %FALSE on error.
 **/
gboolean
g_app_info_add_supports_type (GAppInfo    *appinfo,
			      const char  *content_type,
			      GError     **error)
{
  GAppInfoIface *iface;
  
  g_return_val_if_fail (G_IS_APP_INFO (appinfo), FALSE);
  g_return_val_if_fail (content_type != NULL, FALSE);

  iface = G_APP_INFO_GET_IFACE (appinfo);

  if (iface->add_supports_type)
    return (* iface->add_supports_type) (appinfo, content_type, error);

  g_set_error_literal (error, G_IO_ERROR, 
                       G_IO_ERROR_NOT_SUPPORTED, 
                       "g_app_info_add_supports_type not supported yet");

  return FALSE;
}


/**
 * g_app_info_can_remove_supports_type:
 * @appinfo: a #GAppInfo.
 * 
 * Checks if a supported content type can be removed from an application.
 *
 * Returns: %TRUE if it is possible to remove supported 
 *     content types from a given @appinfo, %FALSE if not.
 **/
gboolean
g_app_info_can_remove_supports_type (GAppInfo *appinfo)
{
  GAppInfoIface *iface;
  
  g_return_val_if_fail (G_IS_APP_INFO (appinfo), FALSE);

  iface = G_APP_INFO_GET_IFACE (appinfo);

  if (iface->can_remove_supports_type)
    return (* iface->can_remove_supports_type) (appinfo);

  return FALSE;
}


/**
 * g_app_info_remove_supports_type:
 * @appinfo: a #GAppInfo.
 * @content_type: a string.
 * @error: a #GError.
 *
 * Removes a supported type from an application, if possible.
 * 
 * Returns: %TRUE on success, %FALSE on error.
 **/
gboolean
g_app_info_remove_supports_type (GAppInfo    *appinfo,
				 const char  *content_type,
				 GError     **error)
{
  GAppInfoIface *iface;
  
  g_return_val_if_fail (G_IS_APP_INFO (appinfo), FALSE);
  g_return_val_if_fail (content_type != NULL, FALSE);

  iface = G_APP_INFO_GET_IFACE (appinfo);

  if (iface->remove_supports_type)
    return (* iface->remove_supports_type) (appinfo, content_type, error);

  g_set_error_literal (error, G_IO_ERROR, 
                       G_IO_ERROR_NOT_SUPPORTED, 
                       "g_app_info_remove_supports_type not supported yet");

  return FALSE;
}

/**
 * g_app_info_get_supported_types:
 * @appinfo: a #GAppInfo that can handle files
 *
 * Retrieves the list of content types that @app_info claims to support.
 * If this information is not provided by the environment, this function
 * will return %NULL.
 * This function does not take in consideration associations added with
 * g_app_info_add_supports_type(), but only those exported directly by
 * the application.
 *
 * Returns: (transfer none) (array zero-terminated=1) (element-type utf8):
 *    a list of content types.
 *
 * Since: 2.34
 */
const char **
g_app_info_get_supported_types (GAppInfo *appinfo)
{
  GAppInfoIface *iface;

  g_return_val_if_fail (G_IS_APP_INFO (appinfo), NULL);

  iface = G_APP_INFO_GET_IFACE (appinfo);

  if (iface->get_supported_types)
    return iface->get_supported_types (appinfo);
  else
    return NULL;
}


/**
 * g_app_info_get_icon:
 * @appinfo: a #GAppInfo.
 * 
 * Gets the icon for the application.
 *
 * Returns: (transfer none): the default #GIcon for @appinfo or %NULL
 * if there is no default icon.
 **/
GIcon *
g_app_info_get_icon (GAppInfo *appinfo)
{
  GAppInfoIface *iface;
  
  g_return_val_if_fail (G_IS_APP_INFO (appinfo), NULL);

  iface = G_APP_INFO_GET_IFACE (appinfo);

  return (* iface->get_icon) (appinfo);
}


/**
 * g_app_info_launch:
 * @appinfo: a #GAppInfo
 * @files: (allow-none) (element-type GFile): a #GList of #GFile objects
 * @launch_context: (allow-none): a #GAppLaunchContext or %NULL
 * @error: a #GError
 * 
 * Launches the application. Passes @files to the launched application
 * as arguments, using the optional @launch_context to get information
 * about the details of the launcher (like what screen it is on).
 * On error, @error will be set accordingly.
 *
 * To launch the application without arguments pass a %NULL @files list.
 *
 * Note that even if the launch is successful the application launched
 * can fail to start if it runs into problems during startup. There is
 * no way to detect this.
 *
 * Some URIs can be changed when passed through a GFile (for instance
 * unsupported URIs with strange formats like mailto:), so if you have
 * a textual URI you want to pass in as argument, consider using
 * g_app_info_launch_uris() instead.
 *
 * The launched application inherits the environment of the launching
 * process, but it can be modified with g_app_launch_context_setenv() and
 * g_app_launch_context_unsetenv().
 *
 * On UNIX, this function sets the <envar>GIO_LAUNCHED_DESKTOP_FILE</envar>
 * environment variable with the path of the launched desktop file and
 * <envar>GIO_LAUNCHED_DESKTOP_FILE_PID</envar> to the process
 * id of the launched process. This can be used to ignore
 * <envar>GIO_LAUNCHED_DESKTOP_FILE</envar>, should it be inherited
 * by further processes. The <envar>DISPLAY</envar> and
 * <envar>DESKTOP_STARTUP_ID</envar> environment variables are also
 * set, based on information provided in @launch_context.
 *
 * Returns: %TRUE on successful launch, %FALSE otherwise.
 **/
gboolean
g_app_info_launch (GAppInfo           *appinfo,
		   GList              *files,
		   GAppLaunchContext  *launch_context,
		   GError            **error)
{
  GAppInfoIface *iface;
  
  g_return_val_if_fail (G_IS_APP_INFO (appinfo), FALSE);

  iface = G_APP_INFO_GET_IFACE (appinfo);

  return (* iface->launch) (appinfo, files, launch_context, error);
}


/**
 * g_app_info_supports_uris:
 * @appinfo: a #GAppInfo.
 * 
 * Checks if the application supports reading files and directories from URIs.
 *
 * Returns: %TRUE if the @appinfo supports URIs.
 **/
gboolean
g_app_info_supports_uris (GAppInfo *appinfo)
{
  GAppInfoIface *iface;
  
  g_return_val_if_fail (G_IS_APP_INFO (appinfo), FALSE);

  iface = G_APP_INFO_GET_IFACE (appinfo);

  return (* iface->supports_uris) (appinfo);
}


/**
 * g_app_info_supports_files:
 * @appinfo: a #GAppInfo.
 * 
 * Checks if the application accepts files as arguments.
 *
 * Returns: %TRUE if the @appinfo supports files.
 **/
gboolean
g_app_info_supports_files (GAppInfo *appinfo)
{
  GAppInfoIface *iface;
  
  g_return_val_if_fail (G_IS_APP_INFO (appinfo), FALSE);

  iface = G_APP_INFO_GET_IFACE (appinfo);

  return (* iface->supports_files) (appinfo);
}


/**
 * g_app_info_launch_uris:
 * @appinfo: a #GAppInfo
 * @uris: (allow-none) (element-type utf8): a #GList containing URIs to launch.
 * @launch_context: (allow-none): a #GAppLaunchContext or %NULL
 * @error: a #GError
 * 
 * Launches the application. This passes the @uris to the launched application
 * as arguments, using the optional @launch_context to get information
 * about the details of the launcher (like what screen it is on).
 * On error, @error will be set accordingly.
 *
 * To launch the application without arguments pass a %NULL @uris list.
 *
 * Note that even if the launch is successful the application launched
 * can fail to start if it runs into problems during startup. There is
 * no way to detect this.
 *
 * Returns: %TRUE on successful launch, %FALSE otherwise.
 **/
gboolean
g_app_info_launch_uris (GAppInfo           *appinfo,
			GList              *uris,
			GAppLaunchContext  *launch_context,
			GError            **error)
{
  GAppInfoIface *iface;
  
  g_return_val_if_fail (G_IS_APP_INFO (appinfo), FALSE);

  iface = G_APP_INFO_GET_IFACE (appinfo);

  return (* iface->launch_uris) (appinfo, uris, launch_context, error);
}


/**
 * g_app_info_should_show:
 * @appinfo: a #GAppInfo.
 *
 * Checks if the application info should be shown in menus that 
 * list available applications.
 * 
 * Returns: %TRUE if the @appinfo should be shown, %FALSE otherwise.
 **/
gboolean
g_app_info_should_show (GAppInfo *appinfo)
{
  GAppInfoIface *iface;
  
  g_return_val_if_fail (G_IS_APP_INFO (appinfo), FALSE);

  iface = G_APP_INFO_GET_IFACE (appinfo);

  return (* iface->should_show) (appinfo);
}

/**
 * g_app_info_launch_default_for_uri:
 * @uri: the uri to show
 * @launch_context: (allow-none): an optional #GAppLaunchContext.
 * @error: a #GError.
 *
 * Utility function that launches the default application
 * registered to handle the specified uri. Synchronous I/O
 * is done on the uri to detect the type of the file if
 * required.
 * 
 * Returns: %TRUE on success, %FALSE on error.
 **/
gboolean
g_app_info_launch_default_for_uri (const char         *uri,
				   GAppLaunchContext  *launch_context,
				   GError            **error)
{
  char *uri_scheme;
  GAppInfo *app_info = NULL;
  GList l;
  gboolean res;

  /* g_file_query_default_handler() calls
   * g_app_info_get_default_for_uri_scheme() too, but we have to do it
   * here anyway in case GFile can't parse @uri correctly.
   */
  uri_scheme = g_uri_parse_scheme (uri);
  if (uri_scheme && uri_scheme[0] != '\0')
    app_info = g_app_info_get_default_for_uri_scheme (uri_scheme);
  g_free (uri_scheme);

  if (!app_info)
    {
      GFile *file;

      file = g_file_new_for_uri (uri);
      app_info = g_file_query_default_handler (file, NULL, error);
      g_object_unref (file);
      if (app_info == NULL)
	return FALSE;

      /* We still use the original @uri rather than calling
       * g_file_get_uri(), because GFile might have modified the URI
       * in ways we don't want (eg, removing the fragment identifier
       * from a file: URI).
       */
    }

  l.data = (char *)uri;
  l.next = l.prev = NULL;
  res = g_app_info_launch_uris (app_info, &l,
				launch_context, error);

  g_object_unref (app_info);
  
  return res;
}

/**
 * g_app_info_can_delete:
 * @appinfo: a #GAppInfo
 *
 * Obtains the information whether the #GAppInfo can be deleted.
 * See g_app_info_delete().
 *
 * Returns: %TRUE if @appinfo can be deleted
 *
 * Since: 2.20
 */
gboolean
g_app_info_can_delete (GAppInfo *appinfo)
{
  GAppInfoIface *iface;
  
  g_return_val_if_fail (G_IS_APP_INFO (appinfo), FALSE);

  iface = G_APP_INFO_GET_IFACE (appinfo);

  if (iface->can_delete)
    return (* iface->can_delete) (appinfo);
 
  return FALSE; 
}


/**
 * g_app_info_delete:
 * @appinfo: a #GAppInfo
 *
 * Tries to delete a #GAppInfo.
 *
 * On some platforms, there may be a difference between user-defined
 * #GAppInfo<!-- -->s which can be deleted, and system-wide ones which
 * cannot. See g_app_info_can_delete().
 *
 * Virtual: do_delete
 * Returns: %TRUE if @appinfo has been deleted
 *
 * Since: 2.20
 */
gboolean
g_app_info_delete (GAppInfo *appinfo)
{
  GAppInfoIface *iface;
  
  g_return_val_if_fail (G_IS_APP_INFO (appinfo), FALSE);

  iface = G_APP_INFO_GET_IFACE (appinfo);

  if (iface->do_delete)
    return (* iface->do_delete) (appinfo);
 
  return FALSE; 
}


enum {
  LAUNCH_FAILED,
  LAUNCHED,
  LAST_SIGNAL
};

struct _GAppLaunchContextPrivate {
  char **envp;
};

static guint signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE_WITH_PRIVATE (GAppLaunchContext, g_app_launch_context, G_TYPE_OBJECT)

/**
 * g_app_launch_context_new:
 * 
 * Creates a new application launch context. This is not normally used,
 * instead you instantiate a subclass of this, such as #GdkAppLaunchContext.
 *
 * Returns: a #GAppLaunchContext.
 **/
GAppLaunchContext *
g_app_launch_context_new (void)
{
  return g_object_new (G_TYPE_APP_LAUNCH_CONTEXT, NULL);
}

static void
g_app_launch_context_finalize (GObject *object)
{
  GAppLaunchContext *context = G_APP_LAUNCH_CONTEXT (object);

  g_strfreev (context->priv->envp);

  G_OBJECT_CLASS (g_app_launch_context_parent_class)->finalize (object);
}

static void
g_app_launch_context_class_init (GAppLaunchContextClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = g_app_launch_context_finalize;

  /*
   * GAppLaunchContext::launch-failed:
   * @context: the object emitting the signal
   * @startup_notify_id: the startup notification id for the failed launch
   *
   * The ::launch-failed signal is emitted when a #GAppInfo launch
   * fails. The startup notification id is provided, so that the launcher
   * can cancel the startup notification.
   *
   * Since: 2.36
   */
  signals[LAUNCH_FAILED] = g_signal_new ("launch-failed",
                                         G_OBJECT_CLASS_TYPE (object_class),
                                         G_SIGNAL_RUN_LAST,
                                         G_STRUCT_OFFSET (GAppLaunchContextClass, launch_failed),
                                         NULL, NULL, NULL,
                                         G_TYPE_NONE, 1, G_TYPE_STRING);

  /*
   * GAppLaunchContext::launched:
   * @context: the object emitting the signal
   * @info: the #GAppInfo that was just launched
   * @platform_data: additional platform-specific data for this launch
   *
   * The ::launched signal is emitted when a #GAppInfo is successfully
   * launched. The @platform_data is an GVariant dictionary mapping
   * strings to variants (ie a{sv}), which contains additional,
   * platform-specific data about this launch. On UNIX, at least the
   * "pid" and "startup-notification-id" keys will be present.
   *
   * Since: 2.36
   */
  signals[LAUNCHED] = g_signal_new ("launched",
                                    G_OBJECT_CLASS_TYPE (object_class),
                                    G_SIGNAL_RUN_LAST,
                                    G_STRUCT_OFFSET (GAppLaunchContextClass, launched),
                                    NULL, NULL, NULL,
                                    G_TYPE_NONE, 2,
                                    G_TYPE_APP_INFO, G_TYPE_VARIANT);
}

static void
g_app_launch_context_init (GAppLaunchContext *context)
{
  context->priv = g_app_launch_context_get_instance_private (context);
}

/**
 * g_app_launch_context_setenv:
 * @context: a #GAppLaunchContext
 * @variable: the environment variable to set
 * @value: the value for to set the variable to.
 *
 * Arranges for @variable to be set to @value in the child's
 * environment when @context is used to launch an application.
 *
 * Since: 2.32
 */
void
g_app_launch_context_setenv (GAppLaunchContext *context,
                             const char        *variable,
                             const char        *value)
{
  if (!context->priv->envp)
    context->priv->envp = g_get_environ ();

  context->priv->envp =
    g_environ_setenv (context->priv->envp, variable, value, TRUE);
}

/**
 * g_app_launch_context_unsetenv:
 * @context: a #GAppLaunchContext
 * @variable: the environment variable to remove
 *
 * Arranges for @variable to be unset in the child's environment
 * when @context is used to launch an application.
 *
 * Since: 2.32
 */
void
g_app_launch_context_unsetenv (GAppLaunchContext *context,
                               const char        *variable)
{
  if (!context->priv->envp)
    context->priv->envp = g_get_environ ();

  context->priv->envp =
    g_environ_unsetenv (context->priv->envp, variable);
}

/**
 * g_app_launch_context_get_environment:
 * @context: a #GAppLaunchContext
 *
 * Gets the complete environment variable list to be passed to
 * the child process when @context is used to launch an application.
 * This is a %NULL-terminated array of strings, where each string has
 * the form <literal>KEY=VALUE</literal>.
 *
 * Return value: (array zero-terminated=1) (transfer full): the
 *     child's environment
 *
 * Since: 2.32
 */
char **
g_app_launch_context_get_environment (GAppLaunchContext *context)
{
  if (!context->priv->envp)
    context->priv->envp = g_get_environ ();

  return g_strdupv (context->priv->envp);
}

/**
 * g_app_launch_context_get_display:
 * @context: a #GAppLaunchContext
 * @info: a #GAppInfo
 * @files: (element-type GFile): a #GList of #GFile objects
 *
 * Gets the display string for the @context. This is used to ensure new
 * applications are started on the same display as the launching
 * application, by setting the <envar>DISPLAY</envar> environment variable.
 *
 * Returns: a display string for the display.
 */
char *
g_app_launch_context_get_display (GAppLaunchContext *context,
				  GAppInfo          *info,
				  GList             *files)
{
  GAppLaunchContextClass *class;

  g_return_val_if_fail (G_IS_APP_LAUNCH_CONTEXT (context), NULL);
  g_return_val_if_fail (G_IS_APP_INFO (info), NULL);

  class = G_APP_LAUNCH_CONTEXT_GET_CLASS (context);

  if (class->get_display == NULL)
    return NULL;

  return class->get_display (context, info, files);
}

/**
 * g_app_launch_context_get_startup_notify_id:
 * @context: a #GAppLaunchContext
 * @info: a #GAppInfo
 * @files: (element-type GFile): a #GList of of #GFile objects
 * 
 * Initiates startup notification for the application and returns the
 * <envar>DESKTOP_STARTUP_ID</envar> for the launched operation,
 * if supported.
 *
 * Startup notification IDs are defined in the <ulink
 * url="http://standards.freedesktop.org/startup-notification-spec/startup-notification-latest.txt">
 * FreeDesktop.Org Startup Notifications standard</ulink>.
 *
 * Returns: a startup notification ID for the application, or %NULL if
 *     not supported.
 **/
char *
g_app_launch_context_get_startup_notify_id (GAppLaunchContext *context,
					    GAppInfo          *info,
					    GList             *files)
{
  GAppLaunchContextClass *class;

  g_return_val_if_fail (G_IS_APP_LAUNCH_CONTEXT (context), NULL);
  g_return_val_if_fail (G_IS_APP_INFO (info), NULL);

  class = G_APP_LAUNCH_CONTEXT_GET_CLASS (context);

  if (class->get_startup_notify_id == NULL)
    return NULL;

  return class->get_startup_notify_id (context, info, files);
}


/**
 * g_app_launch_context_launch_failed:
 * @context: a #GAppLaunchContext.
 * @startup_notify_id: the startup notification id that was returned by g_app_launch_context_get_startup_notify_id().
 *
 * Called when an application has failed to launch, so that it can cancel
 * the application startup notification started in g_app_launch_context_get_startup_notify_id().
 * 
 **/
void
g_app_launch_context_launch_failed (GAppLaunchContext *context,
				    const char        *startup_notify_id)
{
  g_return_if_fail (G_IS_APP_LAUNCH_CONTEXT (context));
  g_return_if_fail (startup_notify_id != NULL);

  g_signal_emit (context, signals[LAUNCH_FAILED], 0, startup_notify_id);
}
