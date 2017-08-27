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

#include <string.h>

#include "gcontenttype.h"
#include "gwin32appinfo.h"
#include "gappinfo.h"
#include "gioerror.h"
#include "gfile.h"
#include <glib/gstdio.h>
#include "glibintl.h"

#include <windows.h>
#include <shlwapi.h>


#ifndef ASSOCF_INIT_BYEXENAME
#define ASSOCF_INIT_BYEXENAME 0x00000002
#endif

/* These were wrong in MingW */
#define REAL_ASSOCSTR_COMMAND 1
#define REAL_ASSOCSTR_EXECUTABLE 2
#define REAL_ASSOCSTR_FRIENDLYDOCNAME 3
#define REAL_ASSOCSTR_FRIENDLYAPPNAME 4

#ifndef AssocQueryString
#pragma message("AssocQueryString not available with SDK used")
#endif

static void g_win32_app_info_iface_init (GAppInfoIface *iface);

struct _GWin32AppInfo
{
  GObject parent_instance;
  wchar_t *id;
  char *id_utf8;
  gboolean id_is_exename;
  char *executable;
  char *name;
  gboolean no_open_with;
};

G_DEFINE_TYPE_WITH_CODE (GWin32AppInfo, g_win32_app_info, G_TYPE_OBJECT,
			 G_IMPLEMENT_INTERFACE (G_TYPE_APP_INFO,
						g_win32_app_info_iface_init))


static void
g_win32_app_info_finalize (GObject *object)
{
  GWin32AppInfo *info;

  info = G_WIN32_APP_INFO (object);

  g_free (info->id);
  g_free (info->id_utf8);
  g_free (info->name);
  g_free (info->executable);

  G_OBJECT_CLASS (g_win32_app_info_parent_class)->finalize (object);
}

static void
g_win32_app_info_class_init (GWin32AppInfoClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  
  gobject_class->finalize = g_win32_app_info_finalize;
}

static void
g_win32_app_info_init (GWin32AppInfo *local)
{
}

static GAppInfo *
g_desktop_app_info_new_from_id (wchar_t *id /* takes ownership */,
				gboolean id_is_exename)
{
#ifdef AssocQueryString
  ASSOCF flags;
#endif
  wchar_t buffer[1024];
  DWORD buffer_size;
  GWin32AppInfo *info;
  HKEY app_key;
  
  info = g_object_new (G_TYPE_WIN32_APP_INFO, NULL);
  info->id = id; /* Takes ownership */
  info->id_utf8 = g_utf16_to_utf8 (id, -1, NULL, NULL, NULL);  
  info->id_is_exename = id_is_exename;

#ifdef AssocQueryString  
  flags = 0;
  if (id_is_exename)
    flags |= ASSOCF_INIT_BYEXENAME;

  buffer_size = 1024;
  if (AssocQueryStringW(flags,
			REAL_ASSOCSTR_EXECUTABLE,
			id,
			NULL,
			buffer,
			&buffer_size) == S_OK)
    info->executable = g_utf16_to_utf8 (buffer, -1, NULL, NULL, NULL);
 
  buffer_size = 1024;
  if (AssocQueryStringW(flags,
			REAL_ASSOCSTR_FRIENDLYAPPNAME,
			id,
			NULL,
			buffer,
			&buffer_size) == S_OK)
    info->name = g_utf16_to_utf8 (buffer, -1, NULL, NULL, NULL);
#endif

  if (info->name == NULL)
    {
      /* TODO: Should look up name from executable resources */
      if (info->executable)
	info->name = g_path_get_basename (info->executable);
      else
	info->name = g_strdup (info->id_utf8);
    }

#ifdef AssocQueryString
  if (AssocQueryKeyW(flags,
		     ASSOCKEY_APP,
		     info->id,
		     NULL,
		     &app_key) == S_OK)
    {
      if (RegQueryValueExW (app_key, L"NoOpenWith", 0,
			    NULL, NULL, NULL) == ERROR_SUCCESS)
	info->no_open_with = TRUE;
      RegCloseKey (app_key);
    }
#endif
  
  return G_APP_INFO (info);
}

static wchar_t *
dup_wstring (wchar_t *str)
{
  gsize len;
  for (len = 0; str[len] != 0; len++)
    ;
  return (wchar_t *)g_memdup (str, (len + 1) * 2);
}

static GAppInfo *
g_win32_app_info_dup (GAppInfo *appinfo)
{
  GWin32AppInfo *info = G_WIN32_APP_INFO (appinfo);
  GWin32AppInfo *new_info;
  
  new_info = g_object_new (G_TYPE_WIN32_APP_INFO, NULL);

  new_info->id = dup_wstring (info->id);
  new_info->id_utf8 = g_strdup (info->id_utf8);
  new_info->id_is_exename = info->id_is_exename;
  new_info->name = g_strdup (info->name);
  new_info->executable = g_strdup (info->executable);
  new_info->no_open_with = info->no_open_with;
  
  return G_APP_INFO (new_info);
}

static gboolean
g_win32_app_info_equal (GAppInfo *appinfo1,
                        GAppInfo *appinfo2)
{
  GWin32AppInfo *info1 = G_WIN32_APP_INFO (appinfo1);
  GWin32AppInfo *info2 = G_WIN32_APP_INFO (appinfo2);

  if (info1->executable == NULL ||
      info2->executable == NULL)
    return FALSE;
  
  return strcmp (info1->executable, info2->executable) == 0;
}

static const char *
g_win32_app_info_get_id (GAppInfo *appinfo)
{
  GWin32AppInfo *info = G_WIN32_APP_INFO (appinfo);

  return info->id_utf8;
}

static const char *
g_win32_app_info_get_name (GAppInfo *appinfo)
{
  GWin32AppInfo *info = G_WIN32_APP_INFO (appinfo);

  if (info->name == NULL)
    return _("Unnamed");
  
  return info->name;
}

static const char *
g_win32_app_info_get_description (GAppInfo *appinfo)
{
  /* Win32 has no app descriptions */
  return NULL;
}

static const char *
g_win32_app_info_get_executable (GAppInfo *appinfo)
{
  GWin32AppInfo *info = G_WIN32_APP_INFO (appinfo);
  
  return info->executable;
}

static GIcon *
g_win32_app_info_get_icon (GAppInfo *appinfo)
{
  /* GWin32AppInfo *info = G_WIN32_APP_INFO (appinfo); */

  /* TODO: How to handle icons */
  return NULL;
}

static gboolean
g_win32_app_info_launch (GAppInfo           *appinfo,
			 GList              *files,
			 GAppLaunchContext  *launch_context,
			 GError            **error)
{
  GWin32AppInfo *info = G_WIN32_APP_INFO (appinfo);
#ifdef AssocQueryString
  ASSOCF flags;
#endif
  HKEY class_key;
  SHELLEXECUTEINFOW exec_info = {0};
  GList *l;

  /* TODO:  What might startup_id mean on win32? */
#ifdef AssocQueryString  
  flags = 0;
  if (info->id_is_exename)
    flags |= ASSOCF_INIT_BYEXENAME;

  if (AssocQueryKeyW (flags,
		      ASSOCKEY_SHELLEXECCLASS,
		      info->id,
		      NULL,
		      &class_key) != S_OK)
    {
      g_set_error_literal (error, G_IO_ERROR, G_IO_ERROR_FAILED, _("Can't find application"));
      return FALSE;
    }
#endif

  /* FIXME: Need to do something with
   * g_app_launch_context_get_environment()... ShellExecuteExW()
   * doesn't have any way to pass an environment though. We need to
   * either (a) update environment, ShellExecuteExW(), revert
   * environment; or (b) find an API to figure out what app
   * ShellExecuteExW() would launch, and then use g_spawn_async()
   * instead.
   */

  for (l = files; l != NULL; l = l->next)
    {
      char *path = g_file_get_path (l->data);
      wchar_t *wfilename = g_utf8_to_utf16 (path, -1, NULL, NULL, NULL);

      g_free (path);
      
      memset (&exec_info, 0, sizeof (exec_info));
      exec_info.cbSize = sizeof (exec_info);
      exec_info.fMask = SEE_MASK_FLAG_DDEWAIT | SEE_MASK_CLASSKEY;
      exec_info.lpFile = wfilename;     
      exec_info.nShow = SW_SHOWNORMAL;
      exec_info.hkeyClass = class_key;
      
      if (!ShellExecuteExW (&exec_info))
	{
	  char *message_utf8 = g_win32_error_message (GetLastError ());

	  g_set_error (error, G_IO_ERROR, G_IO_ERROR_FAILED, _("Error launching application: %s"), message_utf8);
	  g_free (message_utf8);
	  
	  g_free (wfilename);
	  RegCloseKey (class_key);
	  return FALSE;
	}
      
      g_free (wfilename);
    }
  
  RegCloseKey (class_key);
  
  return TRUE;
}

static gboolean
g_win32_app_info_supports_uris (GAppInfo *appinfo)
{
  return FALSE;
}

static gboolean
g_win32_app_info_supports_files (GAppInfo *appinfo)
{
  return TRUE;
}

static gboolean
g_win32_app_info_launch_uris (GAppInfo           *appinfo,
			      GList              *uris,
			      GAppLaunchContext  *launch_context,
			      GError            **error)
{
  g_set_error_literal (error, G_IO_ERROR, 
                       G_IO_ERROR_NOT_SUPPORTED, 
                       _("URIs not supported"));
  return FALSE;
}

static gboolean
g_win32_app_info_should_show (GAppInfo *appinfo)
{
  GWin32AppInfo *info = G_WIN32_APP_INFO (appinfo);

  if (info->no_open_with)
    return FALSE;
  
  return TRUE;
}

static gboolean
g_win32_app_info_set_as_default_for_type (GAppInfo    *appinfo,
                                          const char  *content_type,
                                          GError     **error)
{
  g_set_error_literal (error, G_IO_ERROR, 
                       G_IO_ERROR_NOT_SUPPORTED, 
                       _("association changes not supported on win32"));
  return FALSE;
}

GAppInfo *
g_app_info_create_from_commandline (const char           *commandline,
				    const char           *application_name,
				    GAppInfoCreateFlags   flags,
				    GError              **error)
{
  g_set_error_literal (error, G_IO_ERROR, 
                       G_IO_ERROR_NOT_SUPPORTED, 
                       _("Association creation not supported on win32"));
  return NULL;
}


static void
g_win32_app_info_iface_init (GAppInfoIface *iface)
{
  iface->dup = g_win32_app_info_dup;
  iface->equal = g_win32_app_info_equal;
  iface->get_id = g_win32_app_info_get_id;
  iface->get_name = g_win32_app_info_get_name;
  iface->get_description = g_win32_app_info_get_description;
  iface->get_executable = g_win32_app_info_get_executable;
  iface->get_icon = g_win32_app_info_get_icon;
  iface->launch = g_win32_app_info_launch;
  iface->supports_uris = g_win32_app_info_supports_uris;
  iface->supports_files = g_win32_app_info_supports_files;
  iface->launch_uris = g_win32_app_info_launch_uris;
  iface->should_show = g_win32_app_info_should_show;
  iface->set_as_default_for_type = g_win32_app_info_set_as_default_for_type;
}

static void
enumerate_open_with_list (HKEY    dir_key,
			  GList **prognames)
{
  DWORD index;
  wchar_t name[256];
  DWORD name_len, nbytes;
  wchar_t data[256];
  wchar_t *data_alloc;
  DWORD type;

  /* Must also look inside for a,b,c, + MRUList */
  index = 0;
  name_len = 256;
  nbytes = sizeof (data) - 2;
  while (RegEnumValueW (dir_key,
		        index,
		        name,
		        &name_len,
		        0,
		        &type,
		        (LPBYTE)data,
		        &nbytes) == ERROR_SUCCESS)
    {
      data[nbytes/2] = '\0';
      if (type == REG_SZ &&
	  /* Ignore things like MRUList, just look at 'a', 'b', 'c', etc */
	  name_len == 1)
	{
	  data_alloc = (wchar_t *)g_memdup (data, nbytes + 2);
	  data_alloc[nbytes/2] = 0;
	  *prognames = g_list_prepend (*prognames, data_alloc);
	}
      index++;
      name_len = 256;
      nbytes = sizeof (data) - 2;
    }
  
  index = 0;
  name_len = 256;
  while (RegEnumKeyExW (dir_key,
		        index,
		        name,
		        &name_len,
		        NULL,
		        NULL,
		        NULL,
		        NULL) == ERROR_SUCCESS)
    {
      *prognames = g_list_prepend (*prognames, g_memdup (name, (name_len + 1) * 2));
      index++;
      name_len = 256;
    }
}

static void
enumerate_open_with_progids (HKEY dir_key,
			     GList **progids)
{
  DWORD index;
  wchar_t name[256];
  DWORD name_len, type;

  index = 0;
  name_len = 256;
  while (RegEnumValueW (dir_key,
		        index,
		        name,
		        &name_len,
		        0,
		        &type,
		        NULL,
		        0) == ERROR_SUCCESS)
    {
      *progids = g_list_prepend (*progids, g_memdup (name, (name_len + 1) * 2));
      index++;
      name_len = 256;
    }
}

static void
enumerate_open_with_root (HKEY    dir_key,
			  GList **progids,
			  GList **prognames)
{
  HKEY reg_key = NULL;
  
  if (RegOpenKeyExW (dir_key, L"OpenWithList", 0,
		     KEY_READ, &reg_key) == ERROR_SUCCESS)
    {
      enumerate_open_with_list (reg_key, prognames);
      RegCloseKey (reg_key);
    }
  
  if (RegOpenKeyExW (dir_key, L"OpenWithProgids", 0,
		     KEY_QUERY_VALUE, &reg_key) == ERROR_SUCCESS)
    {
      enumerate_open_with_progids (reg_key, progids);
      RegCloseKey (reg_key);
    }
}

static gboolean
app_info_in_list (GAppInfo *info, 
                  GList    *list)
{
  while (list != NULL)
    {
      if (g_app_info_equal (info, list->data))
	return TRUE;
      list = list->next;
    }
  return FALSE;
}

GList *
g_app_info_get_all_for_type (const char *content_type)
{
  GList *progids = NULL;
  GList *prognames = NULL;
  HKEY reg_key, sys_file_assoc_key, reg_key2;
  wchar_t percieved_type[128];
  DWORD nchars, key_type;
  wchar_t *wc_key;
  GList *l;
  GList *infos;

  wc_key = g_utf8_to_utf16 (content_type, -1, NULL, NULL, NULL);
  if (RegOpenKeyExW (HKEY_CLASSES_ROOT, wc_key, 0,
		     KEY_QUERY_VALUE, &reg_key) == ERROR_SUCCESS)
    {
      enumerate_open_with_root (reg_key, &progids, &prognames);

      nchars = sizeof (percieved_type) / sizeof(wchar_t);
      if (RegQueryValueExW (reg_key, L"PerceivedType", 0,
			    &key_type, (LPBYTE) percieved_type, &nchars) == ERROR_SUCCESS)
	{
	  if (key_type == REG_SZ &&
	      RegOpenKeyExW (HKEY_CLASSES_ROOT, L"SystemFileAssociations", 0,
			     KEY_QUERY_VALUE, &sys_file_assoc_key) == ERROR_SUCCESS)
	    {
	      if (RegOpenKeyExW (sys_file_assoc_key, percieved_type, 0,
				 KEY_QUERY_VALUE, &reg_key2) == ERROR_SUCCESS)
		{
		  enumerate_open_with_root (reg_key2, &progids, &prognames);
		  RegCloseKey (reg_key2);
		}

	      RegCloseKey (sys_file_assoc_key);
	    }
	}
      RegCloseKey (reg_key);
    }

  if (RegOpenKeyExW (HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts", 0,
		     KEY_QUERY_VALUE, &reg_key) == ERROR_SUCCESS)
    {
      if (RegOpenKeyExW (reg_key, wc_key, 0,
			 KEY_QUERY_VALUE, &reg_key2) == ERROR_SUCCESS)
	{
	  enumerate_open_with_root (reg_key2, &progids, &prognames);
	  RegCloseKey (reg_key2);
	}
      
      RegCloseKey (reg_key);
    }

  infos = NULL;
  for (l = prognames; l != NULL; l = l->next)
    {
      GAppInfo *info;

      /* l->data ownership is taken */
      info = g_desktop_app_info_new_from_id ((wchar_t *)l->data, TRUE);
      if (app_info_in_list (info, infos))
	g_object_unref (info);
      else
	infos = g_list_prepend (infos, info);
    }
  g_list_free (prognames);

  for (l = progids; l != NULL; l = l->next)
    {
      GAppInfo *info;

      /* l->data ownership is taken */
      info = g_desktop_app_info_new_from_id ((wchar_t *)l->data, FALSE);
      if (app_info_in_list (info, infos))
	g_object_unref (info);
      else
	infos = g_list_prepend (infos, info);
    }
  g_list_free (progids);
  
  g_free (wc_key);
  return g_list_reverse (infos);
}

GList *
g_app_info_get_recommended_for_type (const char *content_type)
{
  /* FIXME: this should generate a list of applications that are registered
   * as direct handlers for the given content type, without using MIME subclassing.
   * See g_app_info_get_recommended_for_type() in gdesktopappinfo.c for a reference
   * UNIX implementation.
   */
  return g_app_info_get_all_for_type (content_type);
}

GList *
g_app_info_get_fallback_for_type (const char *content_type)
{
  /* FIXME: this should generate a list of applications that are registered
   * as handlers for a superclass of the given content type, but are not
   * direct handlers for the content type itself. See g_app_info_get_fallback_for_type()
   * in gdesktopappinfo.c for a reference UNIX implementation.
   */
  return g_app_info_get_all_for_type (content_type);
}

GAppInfo *
g_app_info_get_default_for_type (const char *content_type,
				 gboolean    must_support_uris)
{
  wchar_t *wtype;
  wchar_t buffer[1024];
  DWORD buffer_size;

  wtype = g_utf8_to_utf16 (content_type, -1, NULL, NULL, NULL);

  /* Verify that we have some sort of app registered for this type */
#ifdef AssocQueryString
  buffer_size = 1024;
  if (AssocQueryStringW (0,
		  	 REAL_ASSOCSTR_COMMAND,
			 wtype,
			 NULL,
			 buffer,
			 &buffer_size) == S_OK)
    /* Takes ownership of wtype */
    return g_desktop_app_info_new_from_id (wtype, FALSE);
#endif

  g_free (wtype);
  return NULL;
}

GAppInfo *
g_app_info_get_default_for_uri_scheme (const char *uri_scheme)
{
  /* TODO: Implement */
  return NULL;
}

GList *
g_app_info_get_all (void)
{
  DWORD index;
  wchar_t name[256];
  DWORD name_len;
  HKEY reg_key;
  GList *infos;
  GAppInfo *info;

  if (RegOpenKeyExW (HKEY_CLASSES_ROOT, L"Applications", 0,
		     KEY_READ, &reg_key) != ERROR_SUCCESS)
    return NULL;

  infos = NULL;
  index = 0;
  name_len = 256;
  while (RegEnumKeyExW (reg_key,
		        index,
		        name,
		        &name_len,
		        NULL,
		        NULL,
		        NULL,
		        NULL) == ERROR_SUCCESS)
    {
      wchar_t *name_dup = g_memdup (name, (name_len+1)*2);
      /* name_dup ownership is taken */
      info = g_desktop_app_info_new_from_id (name_dup, TRUE);
      infos = g_list_prepend (infos, info);
      
      index++;
      name_len = 256;
    }
  
  RegCloseKey (reg_key);

  return g_list_reverse (infos);
}

void
g_app_info_reset_type_associations (const char *content_type)
{
  /* nothing to do */
}
