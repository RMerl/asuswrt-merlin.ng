/* gpg-error.c - Determining gpg-error error codes.
   Copyright (C) 2004, 2016 g10 Code GmbH

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
   License along with libgpg-error; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA.  */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>

#ifdef HAVE_LOCALE_H
# include <locale.h>
#endif
#ifdef ENABLE_NLS
#ifdef HAVE_W32_SYSTEM
# include "gettext.h"
#else
# include <libintl.h>
#endif
# define _(a) gettext (a)
# ifdef gettext_noop
#  define N_(a) gettext_noop (a)
# else
#  define N_(a) (a)
# endif
#else
# define _(a) (a)
# define N_(a) (a)
#endif

#include <gpg-error.h>


#if HAVE_W32_SYSTEM
/* The implementation follows below.  */
static char *get_locale_dir (void);
static void drop_locale_dir (char *locale_dir);
#else
#define get_locale_dir() LOCALEDIR
#define drop_locale_dir(dir)
#endif

static void
i18n_init (void)
{
#ifdef ENABLE_NLS
  char *locale_dir;

#ifdef HAVE_LC_MESSAGES
  setlocale (LC_TIME, "");
  setlocale (LC_MESSAGES, "");
#else
# ifndef HAVE_W32_SYSTEM
  setlocale (LC_ALL, "" );
# endif
#endif

  /* Note that for this program we would only need the textdomain call
     because libgpg-error already initializes itself to its locale dir
     (via gpg_err_init or a constructor).  However this is only done
     for the static standard locale and thus if the above setlocale
     calls select a different locale the bindtext below will do
     something else.  */

  locale_dir = get_locale_dir ();
  if (locale_dir)
    {
      bindtextdomain (PACKAGE, locale_dir);
      drop_locale_dir (locale_dir);
    }
  textdomain (PACKAGE);
#endif
}


#ifdef HAVE_W32_SYSTEM

#include <windows.h>


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
                 use the top directory instead.  */
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

#endif	/* HAVE_W32_SYSTEM */


const char *gpg_strerror_sym (gpg_error_t err);
const char *gpg_strsource_sym (gpg_error_t err);


/* Parse string STR assuming it is either a single number N or in the
 * form K.N to denote an error source code K and and error code N.
 * Returns false on error (e.g. invalid number) or true for valid
 * codes; if true is returned a full error code is stored at ERR.  */
static int
get_err_from_number (char *str, gpg_error_t *err)
{
  unsigned long nr;
  char *tail;

  gpg_err_set_errno (0);
  nr = strtoul (str, &tail, 0);
  if (errno)
    return 0;

  if (nr > UINT_MAX)
    return 0;

  if (*tail)
    {
      unsigned long cnr = strtoul (tail + 1, &tail, 0);
      if (errno || *tail)
	return 0;

      if (nr >= GPG_ERR_SOURCE_DIM || cnr >= GPG_ERR_CODE_DIM)
	return 0;

      nr = gpg_err_make (nr, cnr);
    }

  *err = (unsigned int) nr;
  return 1;
}


/* Helper function to parse a symbol either with a "GPG_ERR_SOURCE_"
 * or "GPG_ERR_" prefix.  If the symbol is not available false is
 * return; else the symbols value is ORed into the value at ERR
 * (shifted for a GPG_ERR_SOURCE_) and true returned.  HAVE_SOURCE and
 * HAVE_CODE are expected to be addresses where a 0 is stored; a 1 is
 * stored at the respective address to mark whether a code or source
 * value was found.  If one of those state variables already point to
 * a true value the function will return 0 and not change the value at
 * ERR.  */
static int
get_err_from_symbol_one (char *str, gpg_error_t *err,
			 int *have_source, int *have_code)
{
  static const char src_prefix[] = "GPG_ERR_SOURCE_";
  static const char code_prefix[] = "GPG_ERR_";

  if (!strncasecmp (src_prefix, str, sizeof (src_prefix) - 1))
    {
      gpg_err_source_t src;

      if (*have_source)
	return 0;
      *have_source = 1;
      str += sizeof (src_prefix) - 1;

      for (src = 0; src < GPG_ERR_SOURCE_DIM; src++)
	{
	  const char *src_sym;

	  src_sym = gpg_strsource_sym (src << GPG_ERR_SOURCE_SHIFT);
	  if (src_sym && !strcasecmp (str, src_sym + sizeof (src_prefix) - 1))
	    {
	      *err |= src << GPG_ERR_SOURCE_SHIFT;
	      return 1;
	    }
	}
    }
  else if (!strncasecmp (code_prefix, str, sizeof (code_prefix) - 1))
    {
      gpg_err_code_t code;

      if (*have_code)
	return 0;
      *have_code = 1;
      str += sizeof (code_prefix) - 1;

      for (code = 0; code < GPG_ERR_CODE_DIM; code++)
	{
	  const char *code_sym = gpg_strerror_sym (code);
	  if (code_sym
	      && !strcasecmp (str, code_sym + sizeof (code_prefix) - 1))
	    {
	      *err |= code;
	      return 1;
	    }
	}
    }
  return 0;
}


/* Parse string STR assuming it is either a single symbol C or in the
 * form S.C to denote an error source symbold S and and error code
 * symbold C.  Returns false on error (e.g. invalid number) or true
 * for valid codes; if true is returned a full error code is stored at
 * ERR.  */
static int
get_err_from_symbol (char *str, gpg_error_t *err)
{
  char *str2 = str;
  int have_source = 0;
  int have_code = 0;
  int ret;
  char *saved_pos = NULL;
  char saved_char;

  *err = 0;
  while (*str2 && ((*str2 >= 'A' && *str2 <= 'Z')
		   || (*str2 >= '0' && *str2 <= '9')
		   || *str2 == '_'))
    str2++;
  if (*str2)
    {
      saved_pos = str2;
      saved_char = *str2;
      *str2 = '\0';
      str2++;
    }
  else
    str2 = NULL;

  ret = get_err_from_symbol_one (str, err, &have_source, &have_code);
  if (ret && str2)
    ret = get_err_from_symbol_one (str2, err, &have_source, &have_code);

  if (saved_pos)
    *saved_pos = saved_char;
  return ret;
}


/* Parse string STR assuming it partial code symbol and store its
 * value at ERR and return true.  */
static int
get_err_from_codesymbol (char *str, gpg_error_t *err)
{
  static const char code_prefix[] = "GPG_ERR_";
  gpg_err_code_t code;

  *err = 0;

  /* Skip an optional prefix.  */
  if (!strncasecmp (code_prefix, str, sizeof (code_prefix) - 1))
    str += sizeof (code_prefix) - 1;

  for (code = 0; code < GPG_ERR_CODE_DIM; code++)
    {
      const char *code_sym = gpg_strerror_sym (code);
      if (code_sym
          && !strcasecmp (str, code_sym + sizeof (code_prefix) - 1))
        {
          *err |= code;
          return 1;
        }
    }
  return 0;
}


/* Helper function to parse a string which maps back to a source or
 * code value.  If no source or code for the symbold is available
 * false is return; else the source or code value is ORed into the
 * value at ERR (shifted for a GPG_ERR_SOURCE_) and true returned.
 * The match is first tried on source values and then on code values.
 * HAVE_SOURCE and HAVE_CODE are expected to be addresses where a 0 is
 * stored; a 1 is stored at the respective address to mark whether a
 * code or source value was found.  If one of those state variables
 * already point to a true value the function will return 0 and not
 * change the value at ERR.  */
static int
get_err_from_str_one (char *str, gpg_error_t *err,
		      int *have_source, int *have_code)
{
  gpg_err_source_t src;
  gpg_err_code_t code;

  for (src = 0; src < GPG_ERR_SOURCE_DIM; src++)
    {
      const char *src_str = gpg_strsource (src << GPG_ERR_SOURCE_SHIFT);
      if (src_str && !strcasecmp (str, src_str))
	{
	  if (*have_source)
	    return 0;

	  *have_source = 1;
	  *err |= src << GPG_ERR_SOURCE_SHIFT;
	  return 1;
	}
    }

  for (code = 0; code < GPG_ERR_CODE_DIM; code++)
    {
      const char *code_str = gpg_strerror (code);
      if (code_str && !strcasecmp (str, code_str))
	{
	  if (*have_code)
	    return 0;

	  *have_code = 1;
	  *err |= code;
	  return 1;
	}
    }

  return 0;
}


/* Parse string STR assuming it is either a single desription string C
 * or in the form S.C to denote an error source descrition S and and
 * error code description C.  Returns false on error (e.g. invalid
 * symbol) or true for valid codes; if true is returned a full error
 * code is stored at ERR.  */
static int
get_err_from_str (char *str, gpg_error_t *err)
{
  char *str2 = str;
  int have_source = 0;
  int have_code = 0;
  int ret;
  char *saved_pos = NULL;
  char saved_char = 0; /* (avoid warning) */

  *err = 0;
  /* First match on the entire string to handle the case that it is
   * code description with spaces.  */
  ret = get_err_from_str_one (str, err, &have_source, &have_code);
  if (ret)
    return ret;

  /* Then figure out whether the first string is a simple word.  */
  while (*str2 && ((*str2 >= 'A' && *str2 <= 'Z')
		   || (*str2 >= 'a' && *str2 <= 'z')
		   || (*str2 >= '0' && *str2 <= '9')
		   || *str2 == '_'))
    str2++;
  if (*str2)
    {
      saved_pos = str2;
      saved_char = *str2;
      *((char *) str2) = '\0';
      str2++;
      while (*str2 && !((*str2 >= 'A' && *str2 <= 'Z')
			|| (*str2 >= 'a' && *str2 <= 'z')
			|| (*str2 >= '0' && *str2 <= '9')
			|| *str2 == '_'))
	str2++;
    }
  else
    str2 = NULL;

  ret = get_err_from_str_one (str, err, &have_source, &have_code);
  if (ret && str2)
    ret = get_err_from_str_one (str2, err, &have_source, &have_code);

  if (saved_pos)
    *saved_pos = saved_char;
  return ret;
}


static void
print_desc (const char *symbol)
{
  static int initialized;
  static FILE *fp;
  char line[512];
  char *p;
  int indesc = 0;
  int blanklines = 0;
  int last_was_keyword = 0;

  if (!symbol)
    return;

  if (!initialized)
    {
      initialized = 1;
      fp = fopen (PKGDATADIR "/errorref.txt", "r");
    }
  if (!fp)
    return;
  rewind (fp);
  while (fgets (line, sizeof line, fp))
    {
      if (*line == '#')
        continue;
      if (*line && line[strlen(line)-1] == '\n')
        line[strlen(line)-1] = 0;

      if (!strncmp (line, "GPG_ERR_", 8))
        {
          if (indesc == 1 && last_was_keyword)
            continue; /* Skip keywords immediately following a matched
                       * keyword.  */
          last_was_keyword = 1;

          indesc = 0;
          p = strchr (line, ' ');
          if (!p)
            continue;
          *p = 0;
          if (!strcmp (line, symbol))
            {
              indesc = 1;
              continue; /* Skip this line.  */
            }
        }
      else
        last_was_keyword = 0;
      if (!indesc)
        continue;
      if (indesc == 1 && !*line)
        continue; /* Skip leading empty lines in a description.  */
      if (indesc == 1)
        putchar ('\n'); /* One leading empty line.  */
      indesc = 2;
      if (!*line)
        {
          blanklines++;
          continue;
        }
      for (; blanklines; blanklines--)
        putchar ('\n');
      printf ("%s\n", line);
    }
  putchar ('\n'); /* One trailing blank line.  */
}





static const char *
my_strusage (int level)
{
  const char *p;

  switch (level)
    {
    case  9: p = "LGPL-2.1-or-later"; break;

    case 11: p = "gpg-error"; break;
    case 12: p = PACKAGE_NAME; break;
    case 13: p = PACKAGE_VERSION; break;
    case 14: p = "Copyright (C) 2019 g10 Code GmbH"; break;
    case 19: p = _("Please report bugs to <https://bugs.gnupg.org>.\n"); break;

    case 1:
    case 40:
      p = ("Usage: gpg-error [options] error-numbers");
      break;
    case 41:
      p = ("Map error numbers to strings and vice versa.\n");
      break;

    case 42:
      p = "1"; /* Flag: print 40 as part of 41. */
      break;

    default: p = NULL; break;
    }
  return p;
}



int
main (int argc, char *argv[])
{
  enum { CMD_DEFAULT     = 0,
         CMD_LIB_VERSION = 501,
         CMD_LIST,
         CMD_DEFINES,
         CMD_LOCALE,
         OPT_DESC
  };
  static gpgrt_opt_t opts[] = {
    ARGPARSE_c (CMD_LIB_VERSION, "lib-version",
                "Print library version"),
    ARGPARSE_c (CMD_LIST, "list",
                "Print all error codes"),
    ARGPARSE_c (CMD_DEFINES, "defines",
                "Print all error codes as #define lines"),
#if HAVE_W32_SYSTEM
    ARGPARSE_c (CMD_LOCALE, "locale",
                "Return the locale used for gettext"),
#else
    ARGPARSE_c (CMD_LOCALE, "locale",
                "@"),
#endif
    ARGPARSE_s_n (OPT_DESC, "desc",
                  "Print with error description"),
    ARGPARSE_end()
  };
  gpgrt_argparse_t pargs = { &argc, &argv };

  int i;
  int libversion = 0;
  int listmode = 0;
  int localemode = 0;
  int desc = 0;
  const char *s, *s2;
  const char *source_sym;
  const char *error_sym;
  gpg_error_t err;

  gpgrt_init ();
  i18n_init ();
  gpgrt_set_strusage (my_strusage);
  gpgrt_log_set_prefix (gpgrt_strusage (11), GPGRT_LOG_WITH_PREFIX);


  while (gpgrt_argparse (NULL, &pargs, opts))
    {
      switch (pargs.r_opt)
        {
        case CMD_LIB_VERSION: libversion = 1; break;
        case CMD_LIST:       listmode = 1; break;
        case CMD_DEFINES:    listmode = 2; break;
        case CMD_LOCALE:     localemode = 1; break;
        case OPT_DESC:       desc = 1; break;
        default: pargs.err = ARGPARSE_PRINT_WARNING; break;
        }
    }
  gpgrt_argparse (NULL, &pargs, NULL);  /* Free internal memory.  */

  if (libversion)
    {
      if (argc)
        gpgrt_usage (1);
    }
  else if (localemode)
    {
      if (argc > 1)
        gpgrt_usage (1);
    }
  else if ((argc && listmode) || (!argc && !listmode))
    gpgrt_usage (1);


  if (libversion)
    {
      argc--; argv++;
      printf ("Version from header: %s (0x%06x)\n",
              GPG_ERROR_VERSION, GPG_ERROR_VERSION_NUMBER);
      printf ("Version from binary: %s\n", gpg_error_check_version (NULL));
      s = gpg_error_check_version ("\x01\x01");
      while (*s && *s == '\n')
        s++;
      fputs ("Copyright blurb ...: ", stdout);
      for (; *s; s++)
        {
          if (*s == '\n')
            {
              for (s2=s+1; *s2 == '\n'; s2++)
                ;
              if (!*s2)
                break;  /* Cut off trailing LFs.  */
              fputs ("\n                     ", stdout);
            }
          else
            putc (*s, stdout);
        }
      putc ('\n', stdout);
    }
  else if (localemode)
    {
#if HAVE_W32_SYSTEM
      if (argc)
        {
          /* Warning: What we do here is not allowed because
           * gpgrt_w32_override_locale needs to be called as early as
           * possible.  However for this very purpose it is okay.  */
          if (**argv >= '0' && **argv <= '9')
            gpgrt_w32_override_locale (NULL, strtoul (*argv, NULL, 0));
          else
            gpgrt_w32_override_locale (*argv, 0);
        }

      printf ("%s\n", gettext_localename ());
#else
      log_info ("this command is only useful on Windows\n");
#endif
    }
  else if (listmode == 1)
    {
      for (i=0; i <  GPG_ERR_SOURCE_DIM; i++)
        {
          /* We use error code 1 because gpg_err_make requires a
             non-zero error code. */
          err = gpg_err_make (i, 1);
          err -= 1;
	  source_sym = gpg_strsource_sym (err);
          if (source_sym)
            {
              printf ("%u = (%u, -) = (%s, -) = (%s, -)\n",
                      err, gpg_err_source (err),
                      source_sym, gpg_strsource (err));
              if (desc)
                print_desc (source_sym);
            }
        }
      for (i=0; i <  GPG_ERR_CODE_DIM; i++)
        {
          err = gpg_err_make (GPG_ERR_SOURCE_UNKNOWN, i);
	  error_sym = gpg_strerror_sym (err);
          if (error_sym)
            {
              printf ("%u = (-, %u) = (-, %s) = (-, %s)\n",
                      err, gpg_err_code (err),
                      error_sym, gpg_strerror (err));
              if (desc)
                print_desc (error_sym);
            }
        }
    }
  else if (listmode == 2)
    {
      int n, nmax;

      for (i=0, nmax=0; i <  GPG_ERR_SOURCE_DIM; i++)
        {
          err = gpg_err_make (i, 1);
	  source_sym = gpg_strsource_sym (err);
          if (source_sym)
            {
              n = strlen (source_sym);
              if (n > nmax)
                nmax = n;
            }
        }
      for (i=0; i <  GPG_ERR_SOURCE_DIM; i++)
        {
          err = gpg_err_make (i, 1);
	  source_sym = gpg_strsource_sym (err);
          if (source_sym)
            printf ("#define %-*s %3u\n", nmax,source_sym,gpg_err_source (err));
        }


      for (i=0, nmax = 0; i <  GPG_ERR_CODE_DIM; i++)
        {
          err = gpg_err_make (GPG_ERR_SOURCE_UNKNOWN, i);
	  error_sym = gpg_strerror_sym (err);
          if (error_sym)
            {
              n = strlen (error_sym);
              if (n > nmax)
                nmax = n;
            }
        }
      for (i=0; i <  GPG_ERR_CODE_DIM; i++)
        {
          err = gpg_err_make (GPG_ERR_SOURCE_UNKNOWN, i);
	  error_sym = gpg_strerror_sym (err);
          if (error_sym)
            printf ("#define %-*s %5u\n", nmax, error_sym, gpg_err_code (err));
        }
    }
  else /* Standard mode.  */
    {
      for (i=0; i < argc; i++)
        {
          /* First check the arg is a number N or K.N,
           * then check the arg for CODESYM or SOURCESYM.CODESYM,
           * then check the arg for CODESYM or CODESYM w/o GPG_ERR_ prefix,
           * then check the arg for code description
           *                     or symbol dot code description.
           */
          if (get_err_from_number (argv[i], &err)
              || get_err_from_symbol (argv[i], &err)
              || get_err_from_codesymbol (argv[i], &err)
              || get_err_from_str (argv[i], &err))
            {
              source_sym = gpg_strsource_sym (err);
              error_sym = gpg_strerror_sym (err);

              printf ("%u = (%u, %u) = (%s, %s) = (%s, %s)\n",
                      err, gpg_err_source (err), gpg_err_code (err),
                      source_sym ? source_sym : "-", error_sym ? error_sym:"-",
                      gpg_strsource (err), gpg_strerror (err));
              if (desc)
                print_desc (error_sym);
            }
          else
            log_error (_("warning: could not recognize %s\n"), argv[i]);
        }
    }

  exit (0);
}
