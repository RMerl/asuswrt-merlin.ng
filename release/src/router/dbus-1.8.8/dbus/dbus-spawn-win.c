#include <config.h>

//#define SPAWN_DEBUG

#if !defined(SPAWN_DEBUG) || defined(_MSC_VER)
#define PING()
#else
#define PING() fprintf (stderr, "%s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__); fflush (stderr)
#endif

#include <stdio.h>

/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/* dbus-spawn-win32.c Wrapper around g_spawn
 * 
 * Copyright (C) 2002, 2003, 2004  Red Hat, Inc.
 * Copyright (C) 2003 CodeFactory AB
 * Copyright (C) 2005 Novell, Inc.
 *
 * Licensed under the Academic Free License version 2.1
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */
#include "dbus-spawn.h"
#include "dbus-sysdeps.h"
#include "dbus-sysdeps-win.h"
#include "dbus-internals.h"
#include "dbus-test.h"
#include "dbus-protocol.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
//#define STRICT
//#include <windows.h>
//#undef STRICT
#include <winsock2.h>
#undef interface

#include <stdlib.h>

#ifndef DBUS_WINCE
#include <process.h>
#endif

/**
 * Babysitter implementation details
 */
struct DBusBabysitter
  {
    int refcount;

    HANDLE start_sync_event;
#ifdef DBUS_ENABLE_EMBEDDED_TESTS

    HANDLE end_sync_event;
#endif

    char *log_name;
    DBusSpawnChildSetupFunc child_setup;
    void *user_data;

    int argc;
    char **argv;
    char **envp;

    HANDLE child_handle;
    int socket_to_babysitter;	/* Connection to the babysitter thread */
    int socket_to_main;

    DBusWatchList *watches;
    DBusWatch *sitter_watch;
    DBusBabysitterFinishedFunc finished_cb;
    void *finished_data;

    dbus_bool_t have_spawn_errno;
    int spawn_errno;
    dbus_bool_t have_child_status;
    int child_status;
  };

static DBusBabysitter*
_dbus_babysitter_new (void)
{
  DBusBabysitter *sitter;

  sitter = dbus_new0 (DBusBabysitter, 1);
  if (sitter == NULL)
    return NULL;

  sitter->refcount = 1;

  sitter->start_sync_event = CreateEvent (NULL, FALSE, FALSE, NULL);
  if (sitter->start_sync_event == NULL)
    {
      _dbus_babysitter_unref (sitter);
      return NULL;
    }

#ifdef DBUS_ENABLE_EMBEDDED_TESTS
  sitter->end_sync_event = CreateEvent (NULL, FALSE, FALSE, NULL);
  if (sitter->end_sync_event == NULL)
    {
      _dbus_babysitter_unref (sitter);
      return NULL;
    }
#endif

  sitter->child_handle = NULL;

  sitter->socket_to_babysitter = sitter->socket_to_main = -1;

  sitter->argc = 0;
  sitter->argv = NULL;
  sitter->envp = NULL;

  sitter->watches = _dbus_watch_list_new ();
  if (sitter->watches == NULL)
    {
      _dbus_babysitter_unref (sitter);
      return NULL;
    }

  sitter->have_spawn_errno = FALSE;
  sitter->have_child_status = FALSE;

  return sitter;
}

/**
 * Increment the reference count on the babysitter object.
 *
 * @param sitter the babysitter
 * @returns the babysitter
 */
DBusBabysitter *
_dbus_babysitter_ref (DBusBabysitter *sitter)
{
  PING();
  _dbus_assert (sitter != NULL);
  _dbus_assert (sitter->refcount > 0);

  sitter->refcount += 1;

  return sitter;
}

static void
close_socket_to_babysitter (DBusBabysitter *sitter)
{
  _dbus_verbose ("Closing babysitter\n");

  if (sitter->sitter_watch != NULL)
    {
      _dbus_assert (sitter->watches != NULL);
      _dbus_watch_list_remove_watch (sitter->watches,  sitter->sitter_watch);
      _dbus_watch_invalidate (sitter->sitter_watch);
      _dbus_watch_unref (sitter->sitter_watch);
      sitter->sitter_watch = NULL;
    }

  if (sitter->socket_to_babysitter != -1)
    {
      _dbus_close_socket (sitter->socket_to_babysitter, NULL);
      sitter->socket_to_babysitter = -1;
    }
}

/**
 * Decrement the reference count on the babysitter object.
 *
 * @param sitter the babysitter
 */
void
_dbus_babysitter_unref (DBusBabysitter *sitter)
{
  int i;

  PING();
  _dbus_assert (sitter != NULL);
  _dbus_assert (sitter->refcount > 0);

  sitter->refcount -= 1;

  if (sitter->refcount == 0)
    {
      close_socket_to_babysitter (sitter);

      if (sitter->socket_to_main != -1)
        {
          _dbus_close_socket (sitter->socket_to_main, NULL);
          sitter->socket_to_main = -1;
        }

      PING();
      if (sitter->argv != NULL)
        {
          for (i = 0; i < sitter->argc; i++)
            if (sitter->argv[i] != NULL)
              {
                dbus_free (sitter->argv[i]);
                sitter->argv[i] = NULL;
              }
          dbus_free (sitter->argv);
          sitter->argv = NULL;
        }

      if (sitter->envp != NULL)
        {
          char **e = sitter->envp;

          while (*e)
            dbus_free (*e++);
          dbus_free (sitter->envp);
          sitter->envp = NULL;
        }

      if (sitter->child_handle != NULL)
        {
          CloseHandle (sitter->child_handle);
          sitter->child_handle = NULL;
        }

      if (sitter->sitter_watch)
        {
          _dbus_watch_invalidate (sitter->sitter_watch);
          _dbus_watch_unref (sitter->sitter_watch);
          sitter->sitter_watch = NULL;
        }

      if (sitter->watches)
        _dbus_watch_list_free (sitter->watches);

      if (sitter->start_sync_event != NULL)
        {
          PING();
          CloseHandle (sitter->start_sync_event);
          sitter->start_sync_event = NULL;
        }

#ifdef DBUS_ENABLE_EMBEDDED_TESTS
      if (sitter->end_sync_event != NULL)
        {
          CloseHandle (sitter->end_sync_event);
          sitter->end_sync_event = NULL;
        }
#endif

      dbus_free (sitter->log_name);

      dbus_free (sitter);
    }
}

void
_dbus_babysitter_kill_child (DBusBabysitter *sitter)
{
  PING();
  if (sitter->child_handle == NULL)
    return; /* child is already dead, or we're so hosed we'll never recover */

  PING();
  TerminateProcess (sitter->child_handle, 12345);
}

/**
 * Checks whether the child has exited, without blocking.
 *
 * @param sitter the babysitter
 */
dbus_bool_t
_dbus_babysitter_get_child_exited (DBusBabysitter *sitter)
{
  PING();
  return (sitter->child_handle == NULL);
}

/**
 * Gets the exit status of the child. We do this so implementation specific
 * detail is not cluttering up dbus, for example the system launcher code.
 * This can only be called if the child has exited, i.e. call
 * _dbus_babysitter_get_child_exited(). It returns FALSE if the child
 * did not return a status code, e.g. because the child was signaled
 * or we failed to ever launch the child in the first place.
 *
 * @param sitter the babysitter
 * @param status the returned status code
 * @returns #FALSE on failure
 */
dbus_bool_t
_dbus_babysitter_get_child_exit_status (DBusBabysitter *sitter,
                                        int            *status)
{
  if (!_dbus_babysitter_get_child_exited (sitter))
    _dbus_assert_not_reached ("Child has not exited");

  if (!sitter->have_child_status ||
      sitter->child_status == STILL_ACTIVE)
    return FALSE;

  *status = sitter->child_status;
  return TRUE;
}

/**
 * Sets the #DBusError with an explanation of why the spawned
 * child process exited (on a signal, or whatever). If
 * the child process has not exited, does nothing (error
 * will remain unset).
 *
 * @param sitter the babysitter
 * @param error an error to fill in
 */
void
_dbus_babysitter_set_child_exit_error (DBusBabysitter *sitter,
                                       DBusError      *error)
{
  PING();
  if (!_dbus_babysitter_get_child_exited (sitter))
    return;

  PING();
  if (sitter->have_spawn_errno)
    {
      char *emsg = _dbus_win_error_string (sitter->spawn_errno);
      dbus_set_error (error, DBUS_ERROR_SPAWN_EXEC_FAILED,
                      "Failed to execute program %s: %s",
                      sitter->log_name, emsg);
      _dbus_win_free_error_string (emsg);
    }
  else if (sitter->have_child_status)
    {
      PING();
      dbus_set_error (error, DBUS_ERROR_SPAWN_CHILD_EXITED,
                      "Process %s exited with status %d",
                      sitter->log_name, sitter->child_status);
    }
  else
    {
      PING();
      dbus_set_error (error, DBUS_ERROR_FAILED,
                      "Process %s exited, status unknown",
                      sitter->log_name);
    }
  PING();
}

dbus_bool_t
_dbus_babysitter_set_watch_functions (DBusBabysitter            *sitter,
                                      DBusAddWatchFunction       add_function,
                                      DBusRemoveWatchFunction    remove_function,
                                      DBusWatchToggledFunction   toggled_function,
                                      void                      *data,
                                      DBusFreeFunction           free_data_function)
{
  PING();
  return _dbus_watch_list_set_functions (sitter->watches,
                                         add_function,
                                         remove_function,
                                         toggled_function,
                                         data,
                                         free_data_function);
}

static dbus_bool_t
handle_watch (DBusWatch       *watch,
              unsigned int     condition,
              void            *data)
{
  DBusBabysitter *sitter = data;

  /* On Unix dbus-spawn uses a babysitter *process*, thus it has to
   * actually send the exit statuses, error codes and whatnot through
   * sockets and/or pipes. On Win32, the babysitter is jus a thread,
   * so it can set the status fields directly in the babysitter struct
   * just fine. The socket pipe is used just so we can watch it with
   * select(), as soon as anything is written to it we know that the
   * babysitter thread has recorded the status in the babysitter
   * struct.
   */

  PING();
  close_socket_to_babysitter (sitter);
  PING();

  if (_dbus_babysitter_get_child_exited (sitter) &&
      sitter->finished_cb != NULL)
    {
      sitter->finished_cb (sitter, sitter->finished_data);
      sitter->finished_cb = NULL;
    }

  return TRUE;
}

/* protect_argv lifted from GLib, relicensed by author, Tor Lillqvist */
static int
protect_argv (char  **argv,
              char ***new_argv)
{
  int i;
  int argc = 0;

  while (argv[argc])
    ++argc;
  *new_argv = dbus_malloc ((argc + 1) * sizeof (char *));
  if (*new_argv == NULL)
    return -1;

  for (i = 0; i < argc; i++)
    (*new_argv)[i] = NULL;

  /* Quote each argv element if necessary, so that it will get
   * reconstructed correctly in the C runtime startup code.  Note that
   * the unquoting algorithm in the C runtime is really weird, and
   * rather different than what Unix shells do. See stdargv.c in the C
   * runtime sources (in the Platform SDK, in src/crt).
   *
   * Note that an new_argv[0] constructed by this function should
   * *not* be passed as the filename argument to a spawn* or exec*
   * family function. That argument should be the real file name
   * without any quoting.
   */
  for (i = 0; i < argc; i++)
    {
      char *p = argv[i];
      char *q;
      int len = 0;
      int need_dblquotes = FALSE;
      while (*p)
        {
          if (*p == ' ' || *p == '\t')
            need_dblquotes = TRUE;
          else if (*p == '"')
            len++;
          else if (*p == '\\')
            {
              char *pp = p;
              while (*pp && *pp == '\\')
                pp++;
              if (*pp == '"')
                len++;
            }
          len++;
          p++;
        }

      q = (*new_argv)[i] = dbus_malloc (len + need_dblquotes*2 + 1);

      if (q == NULL)
        return -1;


      p = argv[i];

      if (need_dblquotes)
        *q++ = '"';

      while (*p)
        {
          if (*p == '"')
            *q++ = '\\';
          else if (*p == '\\')
            {
              char *pp = p;
              while (*pp && *pp == '\\')
                pp++;
              if (*pp == '"')
                *q++ = '\\';
            }
          *q++ = *p;
          p++;
        }

      if (need_dblquotes)
        *q++ = '"';
      *q++ = '\0';
      /* printf ("argv[%d]:%s, need_dblquotes:%s len:%d => %s\n", i, argv[i], need_dblquotes?"TRUE":"FALSE", len, (*new_argv)[i]); */
    }
  (*new_argv)[argc] = NULL;

  return argc;
}


/* From GPGME, relicensed by g10 Code GmbH.  */
static char *
compose_string (char **strings, char separator)
{
  int i;
  int n = 0;
  char *buf;
  char *p;

  if (!strings || !strings[0])
    return 0;
  for (i = 0; strings[i]; i++)
    n += strlen (strings[i]) + 1;
  n++;

  buf = p = malloc (n);
  if (!buf)
    return NULL;
  for (i = 0; strings[i]; i++)
    {
      strcpy (p, strings[i]);
      p += strlen (strings[i]);
      *(p++) = separator;
    }
  p--;
  *(p++) = '\0';
  *p = '\0';

  return buf;
}

static char *
build_commandline (char **argv)
{
  return compose_string (argv, ' ');
}

static char *
build_env_string (char** envp)
{
  return compose_string (envp, '\0');
}

static HANDLE
spawn_program (char* name, char** argv, char** envp)
{
  PROCESS_INFORMATION pi = { NULL, 0, 0, 0 };
  STARTUPINFOA si;
  char *arg_string, *env_string;
  BOOL result;

#ifdef DBUS_WINCE
  if (argv && argv[0])
    arg_string = build_commandline (argv + 1);
  else
    arg_string = NULL;
#else
  arg_string = build_commandline (argv);
#endif
  if (!arg_string)
    return INVALID_HANDLE_VALUE;

  env_string = build_env_string(envp);

  memset (&si, 0, sizeof (si));
  si.cb = sizeof (si);
#ifdef DBUS_WINCE
  result = CreateProcessA (name, arg_string, NULL, NULL, FALSE, 0,
#else
  result = CreateProcessA (NULL, arg_string, NULL, NULL, FALSE, 0,
#endif
			   (LPVOID)env_string, NULL, &si, &pi);
  free (arg_string);
  if (env_string)
    free (env_string);

  if (!result)
    return INVALID_HANDLE_VALUE;

  CloseHandle (pi.hThread);
  return pi.hProcess;
}


static DWORD __stdcall
babysitter (void *parameter)
{
  DBusBabysitter *sitter = (DBusBabysitter *) parameter;

  PING();
  _dbus_babysitter_ref (sitter);

  if (sitter->child_setup)
    {
      PING();
      (*sitter->child_setup) (sitter->user_data);
    }

  _dbus_verbose ("babysitter: spawning %s\n", sitter->log_name);

  PING();
  sitter->child_handle = spawn_program (sitter->log_name,
					sitter->argv, sitter->envp);

  PING();
  if (sitter->child_handle == (HANDLE) -1)
    {
      sitter->child_handle = NULL;
      sitter->have_spawn_errno = TRUE;
      sitter->spawn_errno = GetLastError();
    }
  
  PING();
  SetEvent (sitter->start_sync_event);

  if (sitter->child_handle != NULL)
    {
      int ret;
      DWORD status;

      PING();
      WaitForSingleObject (sitter->child_handle, INFINITE);

      PING();
      ret = GetExitCodeProcess (sitter->child_handle, &status);

      sitter->child_status = status;
      sitter->have_child_status = TRUE;

      CloseHandle (sitter->child_handle);
      sitter->child_handle = NULL;
    }

#ifdef DBUS_ENABLE_EMBEDDED_TESTS
  SetEvent (sitter->end_sync_event);
#endif

  PING();
  send (sitter->socket_to_main, " ", 1, 0);

  _dbus_babysitter_unref (sitter);

  return 0;
}

dbus_bool_t
_dbus_spawn_async_with_babysitter (DBusBabysitter           **sitter_p,
                                   const char                *log_name,
                                   char                     **argv,
                                   char                     **envp,
                                   DBusSpawnChildSetupFunc    child_setup,
                                   void                      *user_data,
                                   DBusError                 *error)
{
  DBusBabysitter *sitter;
  HANDLE sitter_thread;
  DWORD sitter_thread_id;
  
  _DBUS_ASSERT_ERROR_IS_CLEAR (error);
  _dbus_assert (argv[0] != NULL);

  *sitter_p = NULL;

  PING();
  sitter = _dbus_babysitter_new ();
  if (sitter == NULL)
    {
      _DBUS_SET_OOM (error);
      return FALSE;
    }

  sitter->child_setup = child_setup;
  sitter->user_data = user_data;

  sitter->log_name = _dbus_strdup (log_name);
  if (sitter->log_name == NULL && log_name != NULL)
    {
      _DBUS_SET_OOM (error);
      goto out0;
    }

  if (sitter->log_name == NULL)
    sitter->log_name = _dbus_strdup (argv[0]);

  if (sitter->log_name == NULL)
    {
      _DBUS_SET_OOM (error);
      goto out0;
    }

  PING();
  if (!_dbus_full_duplex_pipe (&sitter->socket_to_babysitter,
                               &sitter->socket_to_main,
                               FALSE, error))
    goto out0;

  sitter->sitter_watch = _dbus_watch_new (sitter->socket_to_babysitter,
                                          DBUS_WATCH_READABLE,
                                          TRUE, handle_watch, sitter, NULL);
  PING();
  if (sitter->sitter_watch == NULL)
    {
      _DBUS_SET_OOM (error);
      goto out0;
    }

  PING();
  if (!_dbus_watch_list_add_watch (sitter->watches,  sitter->sitter_watch))
    {
      /* we need to free it early so the destructor won't try to remove it
       * without it having been added, which DBusLoop doesn't allow */
      _dbus_watch_invalidate (sitter->sitter_watch);
      _dbus_watch_unref (sitter->sitter_watch);
      sitter->sitter_watch = NULL;

      _DBUS_SET_OOM (error);
      goto out0;
    }

  sitter->argc = protect_argv (argv, &sitter->argv);
  if (sitter->argc == -1)
    {
      _DBUS_SET_OOM (error);
      goto out0;
    }
  sitter->envp = envp;

  PING();
  sitter_thread = (HANDLE) CreateThread (NULL, 0, babysitter,
                  sitter, 0, &sitter_thread_id);

  if (sitter_thread == 0)
    {
      PING();
      dbus_set_error_const (error, DBUS_ERROR_SPAWN_FORK_FAILED,
                            "Failed to create new thread");
      goto out0;
    }
  CloseHandle (sitter_thread);

  PING();
  WaitForSingleObject (sitter->start_sync_event, INFINITE);

  PING();
  if (sitter_p != NULL)
    *sitter_p = sitter;
  else
    _dbus_babysitter_unref (sitter);

  _DBUS_ASSERT_ERROR_IS_CLEAR (error);

  PING();
  return TRUE;

out0:
  _dbus_babysitter_unref (sitter);

  return FALSE;
}

void
_dbus_babysitter_set_result_function  (DBusBabysitter             *sitter,
                                       DBusBabysitterFinishedFunc  finished,
                                       void                       *user_data)
{
  sitter->finished_cb = finished;
  sitter->finished_data = user_data;
}

#ifdef DBUS_ENABLE_EMBEDDED_TESTS

static char *
get_test_exec (const char *exe,
               DBusString *scratch_space)
{
  const char *dbus_test_exec;

  dbus_test_exec = _dbus_getenv ("DBUS_TEST_EXEC");

  if (dbus_test_exec == NULL)
    dbus_test_exec = DBUS_TEST_EXEC;

  if (!_dbus_string_init (scratch_space))
    return NULL;

  if (!_dbus_string_append_printf (scratch_space, "%s/%s%s",
                                   dbus_test_exec, exe, DBUS_EXEEXT))
    {
      _dbus_string_free (scratch_space);
      return NULL;
    }

  return _dbus_string_get_data (scratch_space);
}

#define LIVE_CHILDREN(sitter) ((sitter)->child_handle != NULL)

static void
_dbus_babysitter_block_for_child_exit (DBusBabysitter *sitter)
{
  if (sitter->child_handle == NULL)
    return;

  WaitForSingleObject (sitter->end_sync_event, INFINITE);
}

static dbus_bool_t
check_spawn_nonexistent (void *data)
{
  char *argv[4] = { NULL, NULL, NULL, NULL };
  DBusBabysitter *sitter;
  DBusError error;

  sitter = NULL;

  dbus_error_init (&error);

  /*** Test launching nonexistent binary */

  argv[0] = "/this/does/not/exist/32542sdgafgafdg";
  if (_dbus_spawn_async_with_babysitter (&sitter, "spawn_nonexistent", argv, NULL,
                                         NULL, NULL,
                                         &error))
    {
      _dbus_babysitter_block_for_child_exit (sitter);
      _dbus_babysitter_set_child_exit_error (sitter, &error);
    }

  if (sitter)
    _dbus_babysitter_unref (sitter);

  if (!dbus_error_is_set (&error))
    {
      _dbus_warn ("Did not get an error launching nonexistent executable\n");
      return FALSE;
    }

  if (!(dbus_error_has_name (&error, DBUS_ERROR_NO_MEMORY) ||
        dbus_error_has_name (&error, DBUS_ERROR_SPAWN_EXEC_FAILED)))
    {
      _dbus_warn ("Not expecting error when launching nonexistent executable: %s: %s\n",
                  error.name, error.message);
      dbus_error_free (&error);
      return FALSE;
    }

  dbus_error_free (&error);

  return TRUE;
}

static dbus_bool_t
check_spawn_segfault (void *data)
{
  char *argv[4] = { NULL, NULL, NULL, NULL };
  DBusBabysitter *sitter;
  DBusError error;
  DBusString argv0;

  sitter = NULL;

  dbus_error_init (&error);

  /*** Test launching segfault binary */

  argv[0] = get_test_exec ("test-segfault", &argv0);

  if (argv[0] == NULL)
    {
      /* OOM was simulated, never mind */
      return TRUE;
    }

  if (_dbus_spawn_async_with_babysitter (&sitter, "spawn_segfault", argv, NULL,
                                         NULL, NULL,
                                         &error))
    {
      _dbus_babysitter_block_for_child_exit (sitter);
      _dbus_babysitter_set_child_exit_error (sitter, &error);
    }

  _dbus_string_free (&argv0);

  if (sitter)
    _dbus_babysitter_unref (sitter);

  if (!dbus_error_is_set (&error))
    {
      _dbus_warn ("Did not get an error launching segfaulting binary\n");
      return FALSE;
    }

  if (!(dbus_error_has_name (&error, DBUS_ERROR_NO_MEMORY) ||
        dbus_error_has_name (&error, DBUS_ERROR_SPAWN_CHILD_EXITED)))
    {
      _dbus_warn ("Not expecting error when launching segfaulting executable: %s: %s\n",
                  error.name, error.message);
      dbus_error_free (&error);
      return FALSE;
    }

  dbus_error_free (&error);

  return TRUE;
}

static dbus_bool_t
check_spawn_exit (void *data)
{
  char *argv[4] = { NULL, NULL, NULL, NULL };
  DBusBabysitter *sitter;
  DBusError error;
  DBusString argv0;

  sitter = NULL;

  dbus_error_init (&error);

  /*** Test launching exit failure binary */

  argv[0] = get_test_exec ("test-exit", &argv0);

  if (argv[0] == NULL)
    {
      /* OOM was simulated, never mind */
      return TRUE;
    }

  if (_dbus_spawn_async_with_babysitter (&sitter, "spawn_exit", argv, NULL,
                                         NULL, NULL,
                                         &error))
    {
      _dbus_babysitter_block_for_child_exit (sitter);
      _dbus_babysitter_set_child_exit_error (sitter, &error);
    }

  _dbus_string_free (&argv0);

  if (sitter)
    _dbus_babysitter_unref (sitter);

  if (!dbus_error_is_set (&error))
    {
      _dbus_warn ("Did not get an error launching binary that exited with failure code\n");
      return FALSE;
    }

  if (!(dbus_error_has_name (&error, DBUS_ERROR_NO_MEMORY) ||
        dbus_error_has_name (&error, DBUS_ERROR_SPAWN_CHILD_EXITED)))
    {
      _dbus_warn ("Not expecting error when launching exiting executable: %s: %s\n",
                  error.name, error.message);
      dbus_error_free (&error);
      return FALSE;
    }

  dbus_error_free (&error);

  return TRUE;
}

static dbus_bool_t
check_spawn_and_kill (void *data)
{
  char *argv[4] = { NULL, NULL, NULL, NULL };
  DBusBabysitter *sitter;
  DBusError error;
  DBusString argv0;

  sitter = NULL;

  dbus_error_init (&error);

  /*** Test launching sleeping binary then killing it */

  argv[0] = get_test_exec ("test-sleep-forever", &argv0);

  if (argv[0] == NULL)
    {
      /* OOM was simulated, never mind */
      return TRUE;
    }

  if (_dbus_spawn_async_with_babysitter (&sitter, "spawn_and_kill", argv, NULL,
                                         NULL, NULL,
                                         &error))
    {
      _dbus_babysitter_kill_child (sitter);

      _dbus_babysitter_block_for_child_exit (sitter);

      _dbus_babysitter_set_child_exit_error (sitter, &error);
    }

  _dbus_string_free (&argv0);

  if (sitter)
    _dbus_babysitter_unref (sitter);

  if (!dbus_error_is_set (&error))
    {
      _dbus_warn ("Did not get an error after killing spawned binary\n");
      return FALSE;
    }

  if (!(dbus_error_has_name (&error, DBUS_ERROR_NO_MEMORY) ||
        dbus_error_has_name (&error, DBUS_ERROR_SPAWN_CHILD_EXITED)))
    {
      _dbus_warn ("Not expecting error when killing executable: %s: %s\n",
                  error.name, error.message);
      dbus_error_free (&error);
      return FALSE;
    }

  dbus_error_free (&error);

  return TRUE;
}

dbus_bool_t
_dbus_spawn_test (const char *test_data_dir)
{
  if (!_dbus_test_oom_handling ("spawn_nonexistent",
                                check_spawn_nonexistent,
                                NULL))
    return FALSE;

  /* Don't run the obnoxious segfault test by default,
   * it's a pain to have to click all those error boxes.
   */
  if (getenv ("DO_SEGFAULT_TEST"))
    if (!_dbus_test_oom_handling ("spawn_segfault",
                                  check_spawn_segfault,
                                  NULL))
      return FALSE;

  if (!_dbus_test_oom_handling ("spawn_exit",
                                check_spawn_exit,
                                NULL))
    return FALSE;

  if (!_dbus_test_oom_handling ("spawn_and_kill",
                                check_spawn_and_kill,
                                NULL))
    return FALSE;

  return TRUE;
}
#endif
