/* pathexp.c -- The shell interface to the globbing library. */

/* Copyright (C) 1995-2014 Free Software Foundation, Inc.

   This file is part of GNU Bash, the Bourne Again SHell.

   Bash is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Bash is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Bash.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "config.h"

#include "bashtypes.h"
#include <stdio.h>

#if defined (HAVE_UNISTD_H)
#  include <unistd.h>
#endif

#include "bashansi.h"

#include "shell.h"
#include "pathexp.h"
#include "flags.h"

#include "shmbutil.h"
#include "bashintl.h"

#include <glob/strmatch.h>

static int glob_name_is_acceptable __P((const char *));
static void ignore_globbed_names __P((char **, sh_ignore_func_t *));
static char *split_ignorespec __P((char *, int *));
	       
#if defined (USE_POSIX_GLOB_LIBRARY)
#  include <glob.h>
typedef int posix_glob_errfunc_t __P((const char *, int));
#else
#  include <glob/glob.h>
#endif

/* Control whether * matches .files in globbing. */
int glob_dot_filenames;

/* Control whether the extended globbing features are enabled. */
int extended_glob = EXTGLOB_DEFAULT;

/* Control enabling special handling of `**' */
int glob_star = 0;

/* Return nonzero if STRING has any unquoted special globbing chars in it.  */
int
unquoted_glob_pattern_p (string)
     register char *string;
{
  register int c;
  char *send;
  int open;

  DECLARE_MBSTATE;

  open = 0;
  send = string + strlen (string);

  while (c = *string++)
    {
      switch (c)
	{
	case '?':
	case '*':
	  return (1);

	case '[':
	  open++;
	  continue;

	case ']':
	  if (open)
	    return (1);
	  continue;

	case '+':
	case '@':
	case '!':
	  if (*string == '(')	/*)*/
	    return (1);
	  continue;

	case CTLESC:
	case '\\':
	  if (*string++ == '\0')
	    return (0);
	}

      /* Advance one fewer byte than an entire multibyte character to
	 account for the auto-increment in the loop above. */
#ifdef HANDLE_MULTIBYTE
      string--;
      ADVANCE_CHAR_P (string, send - string);
      string++;
#else
      ADVANCE_CHAR_P (string, send - string);
#endif
    }
  return (0);
}

/* Return 1 if C is a character that is `special' in a POSIX ERE and needs to
   be quoted to match itself. */
static inline int
ere_char (c)
     int c;
{
  switch (c)
    {
    case '.':
    case '[':
    case '\\':
    case '(':
    case ')':
    case '*':
    case '+':
    case '?':
    case '{':
    case '|':
    case '^':
    case '$':
      return 1;
    default: 
      return 0;
    }
  return (0);
}

int
glob_char_p (s)
     const char *s;
{
  switch (*s)
    {
    case '*':
    case '[':
    case ']':
    case '?':
    case '\\':
      return 1;
    case '+':
    case '@':
    case '!':
      if (s[1] == '(')	/*(*/
	return 1;
      break;
    }
  return 0;
}

/* PATHNAME can contain characters prefixed by CTLESC; this indicates
   that the character is to be quoted.  We quote it here in the style
   that the glob library recognizes.  If flags includes QGLOB_CVTNULL,
   we change quoted null strings (pathname[0] == CTLNUL) into empty
   strings (pathname[0] == 0).  If this is called after quote removal
   is performed, (flags & QGLOB_CVTNULL) should be 0; if called when quote
   removal has not been done (for example, before attempting to match a
   pattern while executing a case statement), flags should include
   QGLOB_CVTNULL.  If flags includes QGLOB_FILENAME, appropriate quoting
   to match a filename should be performed.  QGLOB_REGEXP means we're
   quoting for a Posix ERE (for [[ string =~ pat ]]) and that requires
   some special handling. */
char *
quote_string_for_globbing (pathname, qflags)
     const char *pathname;
     int qflags;
{
  char *temp;
  register int i, j;
  int brack, cclass, collsym, equiv, c, last_was_backslash;
  int savei, savej;

  temp = (char *)xmalloc (2 * strlen (pathname) + 1);

  if ((qflags & QGLOB_CVTNULL) && QUOTED_NULL (pathname))
    {
      temp[0] = '\0';
      return temp;
    }

  brack = cclass = collsym = equiv = last_was_backslash = 0;
  for (i = j = 0; pathname[i]; i++)
    {
      /* Fix for CTLESC at the end of the string? */
      if (pathname[i] == CTLESC && pathname[i+1] == '\0')
	{
	  temp[j++] = pathname[i++];
	  break;
	}
      /* If we are parsing regexp, turn CTLESC CTLESC into CTLESC. It's not an
	 ERE special character, so we should just be able to pass it through. */
      else if ((qflags & QGLOB_REGEXP) && pathname[i] == CTLESC && pathname[i+1] == CTLESC)
	{
	  i++;
	  temp[j++] = pathname[i];
	  continue;
	}
      else if (pathname[i] == CTLESC)
	{
	  if ((qflags & QGLOB_FILENAME) && pathname[i+1] == '/')
	    continue;
	  /* What to do if preceding char is backslash? */
	  if (pathname[i+1] != CTLESC && (qflags & QGLOB_REGEXP) && ere_char (pathname[i+1]) == 0)
	    continue;
	  temp[j++] = '\\';
	  i++;
	  if (pathname[i] == '\0')
	    break;
	}
      else if ((qflags & QGLOB_REGEXP) && (i == 0 || pathname[i-1] != CTLESC) && pathname[i] == '[')	/*]*/
	{
	  brack = 1;
	  temp[j++] = pathname[i++];	/* open bracket */
	  savej = j;
	  savei = i;
	  c = pathname[i++];	/* c == char after open bracket */
	  do
	    {
	      if (c == 0)
		goto endpat;
	      else if (c == CTLESC)
		{
		  /* skip c, check for EOS, let assignment at end of loop */
		  /* pathname[i] == backslash-escaped character */
		  if (pathname[i] == 0)
		    goto endpat;
		  temp[j++] = pathname[i++];
		}
	      else if (c == '[' && pathname[i] == ':')
		{
		  temp[j++] = c;
		  temp[j++] = pathname[i++];
		  cclass = 1;
		}
	      else if (cclass && c == ':' && pathname[i] == ']')
		{
		  temp[j++] = c;
		  temp[j++] = pathname[i++];
		  cclass = 0;
		}
	      else if (c == '[' && pathname[i] == '=')
		{
		  temp[j++] = c;
		  temp[j++] = pathname[i++];
		  if (pathname[i] == ']')
		    temp[j++] = pathname[i++];		/* right brack can be in equiv */
		  equiv = 1;
		}
	      else if (equiv && c == '=' && pathname[i] == ']')
		{
		  temp[j++] = c;
		  temp[j++] = pathname[i++];
		  equiv = 0;
		}
	      else if (c == '[' && pathname[i] == '.')
		{
		  temp[j++] = c;
		  temp[j++] = pathname[i++];
		  if (pathname[i] == ']')
		    temp[j++] = pathname[i++];		/* right brack can be in collsym */
		  collsym = 1;
		}
	      else if (collsym && c == '.' && pathname[i] == ']')
		{
		  temp[j++] = c;
		  temp[j++] = pathname[i++];
		  collsym = 0;
		}
	      else
		temp[j++] = c;
	    }
	  while (((c = pathname[i++]) != ']') && c != 0);

	  /* If we don't find the closing bracket before we hit the end of
	     the string, rescan string without treating it as a bracket
	     expression (has implications for backslash and special ERE
	     chars) */
	  if (c == 0)
	    {
	      i = savei - 1;	/* -1 for autoincrement above */
	      j = savej;
	      continue;
	    }

	  temp[j++] = c;	/* closing right bracket */
	  i--;			/* increment will happen above in loop */
	  continue;		/* skip double assignment below */
	}
      else if (pathname[i] == '\\' && (qflags & QGLOB_REGEXP) == 0)
	{
	  /* XXX - if not quoting regexp, use backslash as quote char. Should
	     we just pass it through without treating it as special? That is
	     what ksh93 seems to do. */

	  /* If we want to pass through backslash unaltered, comment out these
	     lines. */
	  temp[j++] = '\\';

	  i++;
	  if (pathname[i] == '\0')
	    break;
	}
      else if (pathname[i] == '\\' && (qflags & QGLOB_REGEXP))
        last_was_backslash = 1;
      temp[j++] = pathname[i];
    }
endpat:
  temp[j] = '\0';

  return (temp);
}

char *
quote_globbing_chars (string)
     const char *string;
{
  size_t slen;
  char *temp, *t;
  const char *s, *send;
  DECLARE_MBSTATE;

  slen = strlen (string);
  send = string + slen;

  temp = (char *)xmalloc (slen * 2 + 1);
  for (t = temp, s = string; *s; )
    {
      if (glob_char_p (s))
	*t++ = '\\';

      /* Copy a single (possibly multibyte) character from s to t,
	 incrementing both. */
      COPY_CHAR_P (t, s, send);
    }
  *t = '\0';
  return temp;
}

/* Call the glob library to do globbing on PATHNAME. */
char **
shell_glob_filename (pathname)
     const char *pathname;
{
#if defined (USE_POSIX_GLOB_LIBRARY)
  register int i;
  char *temp, **results;
  glob_t filenames;
  int glob_flags;

  temp = quote_string_for_globbing (pathname, QGLOB_FILENAME);

  filenames.gl_offs = 0;

#  if defined (GLOB_PERIOD)
  glob_flags = glob_dot_filenames ? GLOB_PERIOD : 0;
#  else
  glob_flags = 0;
#  endif /* !GLOB_PERIOD */

  glob_flags |= (GLOB_ERR | GLOB_DOOFFS);

  i = glob (temp, glob_flags, (posix_glob_errfunc_t *)NULL, &filenames);

  free (temp);

  if (i == GLOB_NOSPACE || i == GLOB_ABORTED)
    return ((char **)NULL);
  else if (i == GLOB_NOMATCH)
    filenames.gl_pathv = (char **)NULL;
  else if (i != 0)		/* other error codes not in POSIX.2 */
    filenames.gl_pathv = (char **)NULL;

  results = filenames.gl_pathv;

  if (results && ((GLOB_FAILED (results)) == 0))
    {
      if (should_ignore_glob_matches ())
	ignore_glob_matches (results);
      if (results && results[0])
	strvec_sort (results);
      else
	{
	  FREE (results);
	  results = (char **)NULL;
	}
    }

  return (results);

#else /* !USE_POSIX_GLOB_LIBRARY */

  char *temp, **results;

  noglob_dot_filenames = glob_dot_filenames == 0;

  temp = quote_string_for_globbing (pathname, QGLOB_FILENAME);
  results = glob_filename (temp, glob_star ? GX_GLOBSTAR : 0);
  free (temp);

  if (results && ((GLOB_FAILED (results)) == 0))
    {
      if (should_ignore_glob_matches ())
	ignore_glob_matches (results);
      if (results && results[0])
	strvec_sort (results);
      else
	{
	  FREE (results);
	  results = (char **)&glob_error_return;
	}
    }

  return (results);
#endif /* !USE_POSIX_GLOB_LIBRARY */
}

/* Stuff for GLOBIGNORE. */

static struct ignorevar globignore =
{
  "GLOBIGNORE",
  (struct ign *)0,
  0,
  (char *)0,
  (sh_iv_item_func_t *)0,
};

/* Set up to ignore some glob matches because the value of GLOBIGNORE
   has changed.  If GLOBIGNORE is being unset, we also need to disable
   the globbing of filenames beginning with a `.'. */
void
setup_glob_ignore (name)
     char *name;
{
  char *v;

  v = get_string_value (name);
  setup_ignore_patterns (&globignore);

  if (globignore.num_ignores)
    glob_dot_filenames = 1;
  else if (v == 0)
    glob_dot_filenames = 0;
}

int
should_ignore_glob_matches ()
{
  return globignore.num_ignores;
}

/* Return 0 if NAME matches a pattern in the globignore.ignores list. */
static int
glob_name_is_acceptable (name)
     const char *name;
{
  struct ign *p;
  int flags;

  /* . and .. are never matched */
  if (name[0] == '.' && (name[1] == '\0' || (name[1] == '.' && name[2] == '\0')))
    return (0);

  flags = FNM_PATHNAME | FNMATCH_EXTFLAG | FNMATCH_NOCASEGLOB;
  for (p = globignore.ignores; p->val; p++)
    {
      if (strmatch (p->val, (char *)name, flags) != FNM_NOMATCH)
	return (0);
    }
  return (1);
}

/* Internal function to test whether filenames in NAMES should be
   ignored.  NAME_FUNC is a pointer to a function to call with each
   name.  It returns non-zero if the name is acceptable to the particular
   ignore function which called _ignore_names; zero if the name should
   be removed from NAMES. */

static void
ignore_globbed_names (names, name_func)
     char **names;
     sh_ignore_func_t *name_func;
{
  char **newnames;
  int n, i;

  for (i = 0; names[i]; i++)
    ;
  newnames = strvec_create (i + 1);

  for (n = i = 0; names[i]; i++)
    {
      if ((*name_func) (names[i]))
	newnames[n++] = names[i];
      else
	free (names[i]);
    }

  newnames[n] = (char *)NULL;

  if (n == 0)
    {
      names[0] = (char *)NULL;
      free (newnames);
      return;
    }

  /* Copy the acceptable names from NEWNAMES back to NAMES and set the
     new array end. */
  for (n = 0; newnames[n]; n++)
    names[n] = newnames[n];
  names[n] = (char *)NULL;
  free (newnames);
}

void
ignore_glob_matches (names)
     char **names;
{
  if (globignore.num_ignores == 0)
    return;

  ignore_globbed_names (names, glob_name_is_acceptable);
}

static char *
split_ignorespec (s, ip)
     char *s;
     int *ip;
{
  char *t;
  int n, i;

  if (s == 0)
    return 0;

  i = *ip;
  if (s[i] == 0)
    return 0;

  n = skip_to_delim (s, i, ":", SD_NOJMP|SD_EXTGLOB|SD_GLOB);
  t = substring (s, i, n);

  if (s[n] == ':')
    n++;  
  *ip = n;  
  return t;
}
  
void
setup_ignore_patterns (ivp)
     struct ignorevar *ivp;
{
  int numitems, maxitems, ptr;
  char *colon_bit, *this_ignoreval;
  struct ign *p;

  this_ignoreval = get_string_value (ivp->varname);

  /* If nothing has changed then just exit now. */
  if ((this_ignoreval && ivp->last_ignoreval && STREQ (this_ignoreval, ivp->last_ignoreval)) ||
      (!this_ignoreval && !ivp->last_ignoreval))
    return;

  /* Oops.  The ignore variable has changed.  Re-parse it. */
  ivp->num_ignores = 0;

  if (ivp->ignores)
    {
      for (p = ivp->ignores; p->val; p++)
	free(p->val);
      free (ivp->ignores);
      ivp->ignores = (struct ign *)NULL;
    }

  if (ivp->last_ignoreval)
    {
      free (ivp->last_ignoreval);
      ivp->last_ignoreval = (char *)NULL;
    }

  if (this_ignoreval == 0 || *this_ignoreval == '\0')
    return;

  ivp->last_ignoreval = savestring (this_ignoreval);

  numitems = maxitems = ptr = 0;

#if 0
  while (colon_bit = extract_colon_unit (this_ignoreval, &ptr))
#else
  while (colon_bit = split_ignorespec (this_ignoreval, &ptr))
#endif
    {
      if (numitems + 1 >= maxitems)
	{
	  maxitems += 10;
	  ivp->ignores = (struct ign *)xrealloc (ivp->ignores, maxitems * sizeof (struct ign));
	}
      ivp->ignores[numitems].val = colon_bit;
      ivp->ignores[numitems].len = strlen (colon_bit);
      ivp->ignores[numitems].flags = 0;
      if (ivp->item_func)
	(*ivp->item_func) (&ivp->ignores[numitems]);
      numitems++;
    }
  ivp->ignores[numitems].val = (char *)NULL;
  ivp->num_ignores = numitems;
}
