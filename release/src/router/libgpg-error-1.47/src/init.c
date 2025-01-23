/* init.c - Initialize the GnuPG error library.
   Copyright (C) 2005, 2010 g10 Code GmbH

   This file is part of libgpg-error.

   libgpg-error is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License
   as published by the Free Software Foundation; either version 2.1 of
   the License, or (at your option) any later version.

   libgpg-error is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this program; if not, see <https://www.gnu.org/licenses/>.
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "gpgrt-int.h"
#include "gettext.h"
#include "init.h"


/* Locale directory support.  */

#if HAVE_W32_SYSTEM

#include <windows.h>

static int tls_index = TLS_OUT_OF_INDEXES;  /* Index for the TLS functions.  */

static char *get_locale_dir (void);
static void drop_locale_dir (char *locale_dir);

#else /*!HAVE_W32_SYSTEM*/

#define get_locale_dir() LOCALEDIR
#define drop_locale_dir(dir)

#endif /*!HAVE_W32_SYSTEM*/


/* The list of emergency cleanup functions; see _gpgrt_abort and
 * _gpgrt_add_emergency_cleanup.  */
struct emergency_cleanup_item_s;
typedef struct emergency_cleanup_item_s *emergency_cleanup_item_t;
struct emergency_cleanup_item_s
{
  emergency_cleanup_item_t next;
  void (*func) (void);
};
static emergency_cleanup_item_t emergency_cleanup_list;




/* The realloc function as set by gpgrt_set_alloc_func.  */
static void *(*custom_realloc)(void *a, size_t n);



static void
real_init (void)
{
#ifdef ENABLE_NLS
  char *locale_dir;

  /* We only have to bind our locale directory to our text domain.  */
  locale_dir = get_locale_dir ();
  if (locale_dir)
    {
      bindtextdomain (PACKAGE, locale_dir);
      drop_locale_dir (locale_dir);
    }
#endif
  _gpgrt_estream_init ();
}

/* Initialize the library.  This function should be run early.  */
gpg_error_t
_gpg_err_init (void)
{
#ifdef HAVE_W32_SYSTEM
# ifdef DLL_EXPORT
  /* We always have a constructor and thus this function is called
     automatically.  Due to the way the C init code of mingw works,
     the constructors are called before our DllMain function is
     called.  The problem with that is that the TLS has not been setup
     and w32-gettext.c requires TLS.  To solve this we do nothing here
     but call the actual init code from our DllMain.  */
# else /*!DLL_EXPORT*/
  /* Note that if the TLS is actually used, we can't release the TLS
     as there is no way to know when a thread terminates (i.e. no
     thread-specific-atexit).  You are really better off to use the
     DLL! */
  if (tls_index == TLS_OUT_OF_INDEXES)
    {
      tls_index = TlsAlloc ();
      if (tls_index == TLS_OUT_OF_INDEXES)
        {
          /* No way to continue - commit suicide.  */
          _gpgrt_abort ();
        }
      _gpg_w32__init_gettext_module ();
      real_init ();
    }
# endif /*!DLL_EXPORT*/
#else
  real_init ();
#endif
  return 0;
}


/* Deinitialize libgpg-error.  This function is only used in special
   circumstances.  No gpg-error function should be used after this
   function has been called.  A value of 0 passed for MODE
   deinitializes the entire libgpg-error, a value of 1 releases
   resources allocated for the current thread and only that thread may
   not anymore access libgpg-error after such a call.  Under Windows
   this function may be called from the DllMain function of a DLL
   which statically links to libgpg-error.  */
void
_gpg_err_deinit (int mode)
{
#if defined (HAVE_W32_SYSTEM) && !defined(DLL_EXPORT)
  struct tls_space_s *tls;

  tls = TlsGetValue (tls_index);
  if (tls)
    {
      TlsSetValue (tls_index, NULL);
      LocalFree (tls);
    }

  if (mode == 0)
    {
      TlsFree (tls_index);
      tls_index = TLS_OUT_OF_INDEXES;
    }
#else
  (void)mode;
#endif
}


/* Add the emergency cleanup function F to the list of those function.
 * If the a function with that address has already been registered, it
 * is not added a second time.  These emergency functions are called
 * whenever gpgrt_abort is called and at no other place.  Like signal
 * handles the emergency cleanup functions shall not call any
 * non-trivial functions and return as soon as possible.  They allow
 * to cleanup internal states which should not go into a core dumps or
 * similar.  This is independent of any atexit functions.  We don't
 * use locks here because in an emergency case we can't use them
 * anyway.  */
void
_gpgrt_add_emergency_cleanup (void (*f)(void))
{
  emergency_cleanup_item_t item;

  for (item = emergency_cleanup_list; item; item = item->next)
    if (item->func == f)
      return; /* Function has already been registered.  */

  /* We use a standard malloc here.  */
  item = malloc (sizeof *item);
  if (item)
    {
      item->func = f;
      item->next = emergency_cleanup_list;
      emergency_cleanup_list = item;
    }
  else
    _gpgrt_log_fatal ("out of core in gpgrt_add_emergency_cleanup\n");
}


/* Run the emergency handlers.  No locks are used because we are anyway
 * in an emergency state.  We also can't release any memory.  */
static void
run_emergency_cleanup (void)
{
  emergency_cleanup_item_t next;
  void (*f)(void);

  while (emergency_cleanup_list)
    {
      next = emergency_cleanup_list->next;
      f = emergency_cleanup_list->func;
      emergency_cleanup_list->func = NULL;
      emergency_cleanup_list = next;
      if (f)
        f ();
    }
}


/* Wrapper around abort to be able to run all emergency cleanup
 * functions.  */
void
_gpgrt_abort (void)
{
  run_emergency_cleanup ();
  abort ();
}



/* Register F as allocation function.  This function is used for all
   APIs which return an allocated buffer.  F needs to have standard
   realloc semantics.  It should be called as early as possible and
   not changed later. */
void
_gpgrt_set_alloc_func (void *(*f)(void *a, size_t n))
{
  custom_realloc = f;
}


/* The realloc to be used for data returned by the public API.  */
void *
_gpgrt_realloc (void *a, size_t n)
{
  if (custom_realloc)
    return custom_realloc (a, n);

  if (!n)
    {
      free (a);
      return NULL;
    }

  if (!a)
    return malloc (n);

  return realloc (a, n);
}


/* This is safe version of realloc useful for reallocing a calloced
 * array.  There are two ways to call it:  The first example
 * reallocates the array A to N elements each of SIZE but does not
 * clear the newly allocated elements:
 *
 *  p = gpgrt_reallocarray (a, n, n, nsize);
 *
 * Note that when NOLD is larger than N no cleaning is needed anyway.
 * The second example reallocates an array of size NOLD to N elements
 * each of SIZE but clear the newly allocated elements:
 *
 *  p = gpgrt_reallocarray (a, nold, n, nsize);
 *
 * Note that gpgrt_reallocarray (NULL, 0, n, nsize) is equivalent to
 * _gpgrt_calloc (n, nsize).
 *
 */
void *
_gpgrt_reallocarray (void *a, size_t oldnmemb, size_t nmemb, size_t size)
{
  size_t oldbytes, bytes;
  char *p;

  bytes = nmemb * size; /* size_t is unsigned so the behavior on overflow
                         * is defined. */
  if (size && bytes / size != nmemb)
    {
      _gpg_err_set_errno (ENOMEM);
      return NULL;
    }

  p = _gpgrt_realloc (a, bytes);
  if (p && oldnmemb < nmemb)
    {
      /* OLDNMEMBS is lower than NMEMB thus the user asked for a
         calloc.  Clear all newly allocated members.  */
      oldbytes = oldnmemb * size;
      if (size && oldbytes / size != oldnmemb)
        {
          xfree (p);
          _gpg_err_set_errno (ENOMEM);
          return NULL;
        }
      memset (p + oldbytes, 0, bytes - oldbytes);
    }
  return p;
}


/* The malloc to be used for data returned by the public API.  */
void *
_gpgrt_malloc (size_t n)
{
  if (!n)
    n++;
  return _gpgrt_realloc (NULL, n);
}


void *
_gpgrt_calloc (size_t n, size_t m)
{
  size_t bytes;
  void *p;

  bytes = n * m; /* size_t is unsigned so the behavior on overflow is
                    defined. */
  if (m && bytes / m != n)
    {
      _gpg_err_set_errno (ENOMEM);
      return NULL;
    }

  p = _gpgrt_realloc (NULL, bytes);
  if (p)
    memset (p, 0, bytes);
  return p;
}


char *
_gpgrt_strdup (const char *string)
{
  size_t len = strlen (string);
  char *p;

  p = _gpgrt_realloc (NULL, len + 1);
  if (p)
    strcpy (p, string);
  return p;
}


/* Helper for _gpgrt_strconcat and gpgrt_strconcat.  */
char *
_gpgrt_strconcat_core (const char *s1, va_list arg_ptr)
{
  const char *argv[48];
  size_t argc;
  size_t needed;
  char *buffer, *p;

  argc = 0;
  argv[argc++] = s1;
  needed = strlen (s1);
  while (((argv[argc] = va_arg (arg_ptr, const char *))))
    {
      needed += strlen (argv[argc]);
      if (argc >= DIM (argv)-1)
        {
          _gpg_err_set_errno (EINVAL);
          return NULL;
        }
      argc++;
    }
  needed++;
  buffer = _gpgrt_malloc (needed);
  if (buffer)
    {
      for (p = buffer, argc=0; argv[argc]; argc++)
        p = stpcpy (p, argv[argc]);
    }
  return buffer;
}


char *
_gpgrt_strconcat (const char *s1, ...)
{
  va_list arg_ptr;
  char *result;

  if (!s1)
    result = _gpgrt_strdup ("");
  else
    {
      va_start (arg_ptr, s1);
      result = _gpgrt_strconcat_core (s1, arg_ptr);
      va_end (arg_ptr);
    }
  return result;
}


/* The free to be used for data returned by the public API.  */
void
_gpgrt_free (void *a)
{
  int save_errno;

  if (!a)
    return;  /* Shortcut */

  /* In case ERRNO is set we better save it so that the free machinery
   * may not accidentally change ERRNO.  We restore it only if it was
   * already set to comply with the usual C semantic for ERRNO.
   * See also https://dev.gnupg.org/T5393#146261  */
  save_errno = errno;
  _gpgrt_realloc (a, 0);
  if (save_errno && save_errno != errno)
    _gpg_err_set_errno (save_errno);
}


void
_gpg_err_set_errno (int err)
{
  errno = err;
}



/* Internal tracing functions.  Except for TRACE_FP we use flockfile
 * and funlockfile to protect their use.
 *
 * Warning: Take care with the trace functions - they may not use any
 * of our services, in particular not the syscall clamp mechanism for
 * reasons explained in w32-stream.c:create_reader.  */
static FILE *trace_fp;
static int trace_save_errno;
static int trace_with_errno;
static const char *trace_arg_module;
static const char *trace_arg_file;
static int trace_arg_line;
static int trace_missing_lf;
static int trace_prefix_done;

void
_gpgrt_internal_trace_begin (const char *module, const char *file, int line,
                             int with_errno)
{
  int save_errno = errno;

  if (!trace_fp)
    {
      FILE *fp;
      const char *s = getenv ("GPGRT_TRACE_FILE");

      if (!s || !(fp = fopen (s, "wb")))
        fp = stderr;
      trace_fp = fp;
    }

#ifdef HAVE_FLOCKFILE
  flockfile (trace_fp);
#endif
  trace_save_errno = save_errno;
  trace_with_errno = with_errno;
  trace_arg_module = module;
  trace_arg_file = file;
  trace_arg_line = line;
  trace_missing_lf = 0;
  trace_prefix_done = 0;
}

static void
print_internal_trace_prefix (void)
{
  if (!trace_prefix_done)
    {
      trace_prefix_done = 1;
      fprintf (trace_fp, "%s:%s:%d: ",
               trace_arg_module,/* npth_is_protected ()?"":"^",*/
               trace_arg_file, trace_arg_line);
    }
}

static void
do_internal_trace (const char *format, va_list arg_ptr)
{
  print_internal_trace_prefix ();
  vfprintf (trace_fp, format, arg_ptr);
  if (trace_with_errno)
    fprintf (trace_fp, " errno=%s", strerror (trace_save_errno));
  if (*format && format[strlen(format)-1] != '\n')
    fputc ('\n', trace_fp);
}

void
_gpgrt_internal_trace_printf (const char *format, ...)
{
  va_list arg_ptr;

  print_internal_trace_prefix ();
  va_start (arg_ptr, format) ;
  vfprintf (trace_fp, format, arg_ptr);
  va_end (arg_ptr);
  trace_missing_lf = (*format && format[strlen(format)-1] != '\n');
}


void
_gpgrt_internal_trace (const char *format, ...)
{
  va_list arg_ptr;

  va_start (arg_ptr, format) ;
  do_internal_trace (format, arg_ptr);
  va_end (arg_ptr);
}


void
_gpgrt_internal_trace_end (void)
{
  int save_errno = trace_save_errno;

  if (trace_missing_lf)
    fputc ('\n', trace_fp);
#ifdef HAVE_FLOCKFILE
  funlockfile (trace_fp);
#endif
  errno = save_errno;
}



#ifdef HAVE_W32_SYSTEM
/*****************************************
 ******** Below is only Windows code. ****
 *****************************************/

static char *
get_locale_dir (void)
{
  static wchar_t moddir[MAX_PATH+5];
  char *result, *p;
  int nbytes;

  if (!GetModuleFileNameW (NULL, moddir, MAX_PATH))
    *moddir = 0;

#define SLDIR "\\share\\locale"
  if (*moddir)
    {
      nbytes = WideCharToMultiByte (CP_UTF8, 0, moddir, -1, NULL, 0, NULL, NULL);
      if (nbytes < 0)
        return NULL;

      result = malloc (nbytes + strlen (SLDIR) + 1);
      if (result)
        {
          nbytes = WideCharToMultiByte (CP_UTF8, 0, moddir, -1,
                                        result, nbytes, NULL, NULL);
          if (nbytes < 0)
            {
              free (result);
              result = NULL;
            }
          else
            {
              p = strrchr (result, '\\');
              if (p)
                *p = 0;
              /* If we are installed below "bin" strip that part and
                 use the top directory instead.

                 Background: Under Windows we don't install GnuPG
                 below bin/ but in the top directory with only share/,
                 lib/, and etc/ below it.  One of the reasons is to
                 keep the the length of the filenames at bay so not to
                 increase the limited length of the PATH envvar.
                 Another and more important reason, however, is that
                 the very first GPG versions on W32 were installed
                 into a flat directory structure and for best
                 compatibility with these versions we didn't changed
                 that later.  For WindowsCE we can right away install
                 it under bin, though.  The hack with detection of the
                 bin directory part allows us to eventually migrate to
                 such a directory layout under plain Windows without
                 the need to change libgpg-error.  */
              p = strrchr (result, '\\');
              if (p && !strcmp (p+1, "bin"))
                *p = 0;
              /* Append the static part.  */
              strcat (result, SLDIR);
            }
        }
    }
  else /* Use the old default value.  */
    {
      result = malloc (10 + strlen (SLDIR) + 1);
      if (result)
        {
          strcpy (result, "c:\\gnupg");
          strcat (result, SLDIR);
        }
    }
#undef SLDIR
  return result;
}


static void
drop_locale_dir (char *locale_dir)
{
  free (locale_dir);
}


/* Return the tls object.  This function is guaranteed to return a
   valid non-NULL object.  */
struct tls_space_s *
get_tls (void)
{
  struct tls_space_s *tls;

  tls = TlsGetValue (tls_index);
  if (!tls)
    {
      /* Called by a thread which existed before this DLL was loaded.
         Allocate the space.  */
      tls = LocalAlloc (LPTR, sizeof *tls);
      if (!tls)
        {
          /* No way to continue - commit suicide.  */
          _gpgrt_abort ();
        }
      tls->gt_use_utf8 = 0;
      TlsSetValue (tls_index, tls);
    }

  return tls;
}


/* Entry point called by the DLL loader.  */
#ifdef DLL_EXPORT
int WINAPI
DllMain (HINSTANCE hinst, DWORD reason, LPVOID reserved)
{
  struct tls_space_s *tls;
  (void)reserved;
  (void)hinst;

  switch (reason)
    {
    case DLL_PROCESS_ATTACH:
      tls_index = TlsAlloc ();
      if (tls_index == TLS_OUT_OF_INDEXES)
        return FALSE;
#ifndef _GPG_ERR_HAVE_CONSTRUCTOR
      /* If we have not constructors (e.g. MSC) we call it here.  */
      _gpg_w32__init_gettext_module ();
#endif
      /* fallthru.  */
    case DLL_THREAD_ATTACH:
      tls = LocalAlloc (LPTR, sizeof *tls);
      if (!tls)
        return FALSE;
      tls->gt_use_utf8 = 0;
      TlsSetValue (tls_index, tls);
      if (reason == DLL_PROCESS_ATTACH)
        {
          real_init ();
        }
      break;

    case DLL_THREAD_DETACH:
      tls = TlsGetValue (tls_index);
      if (tls)
        LocalFree (tls);
      break;

    case DLL_PROCESS_DETACH:
      tls = TlsGetValue (tls_index);
      if (tls)
        LocalFree (tls);
      TlsFree (tls_index);
      break;

    default:
      break;
    }

  return TRUE;
}
#endif /*DLL_EXPORT*/

#endif /*HAVE_W32_SYSTEM*/
