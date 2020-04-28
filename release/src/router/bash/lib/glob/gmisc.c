/* gmisc.c -- miscellaneous pattern matching utility functions for Bash.

   Copyright (C) 2010 Free Software Foundation, Inc.

   This file is part of GNU Bash, the Bourne-Again SHell.
   
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

#include <config.h>

#include "bashtypes.h"

#if defined (HAVE_UNISTD_H)
#  include <unistd.h>
#endif

#include "bashansi.h"
#include "shmbutil.h"
#include "chartypes.h"

#include "stdc.h"

#ifndef FNM_CASEFOLD
#  include "strmatch.h"
#endif

#ifndef LPAREN
#  define LPAREN '('
#endif
#ifndef RPAREN
#  define RPAREN ')'
#endif

#if defined (HANDLE_MULTIBYTE)
#define WLPAREN         L'('
#define WRPAREN         L')'

/* Make sure these names continue to agree with what's in smatch.c */
extern char *glob_patscan __P((char *, char *, int));
extern wchar_t *glob_patscan_wc __P((wchar_t *, wchar_t *, int));

#define FOLD(c) ((flags & FNM_CASEFOLD) && iswupper (c) ? towlower (c) : (c))

/* Return 1 of the first character of WSTRING could match the first
   character of pattern WPAT.  Wide character version.  FLAGS is a
   subset of strmatch flags; used to do case-insensitive matching for now. */
int
match_pattern_wchar (wpat, wstring, flags)
     wchar_t *wpat, *wstring;
     int flags;
{
  wchar_t wc;

  if (*wstring == 0)
    return (*wpat == L'*');	/* XXX  - allow only * to match empty string */

  switch (wc = *wpat++)
    {
    default:
      return (FOLD(*wstring) == FOLD(wc));
    case L'\\':
      return (FOLD(*wstring) == FOLD(*wpat));
    case L'?':
      return (*wpat == WLPAREN ? 1 : (*wstring != L'\0'));
    case L'*':
      return (1);
    case L'+':
    case L'!':
    case L'@':
      return (*wpat == WLPAREN ? 1 : (FOLD(*wstring) == FOLD(wc)));
    case L'[':
      return (*wstring != L'\0');
    }
}

int
wmatchlen (wpat, wmax)
     wchar_t *wpat;
     size_t wmax;
{
  wchar_t wc;
  int matlen, bracklen, t, in_cclass, in_collsym, in_equiv;

  if (*wpat == 0)
    return (0);

  matlen = in_cclass = in_collsym = in_equiv = 0;
  while (wc = *wpat++)
    {
      switch (wc)
	{
	default:
	  matlen++;
	  break;
	case L'\\':
	  if (*wpat == 0)
	    return ++matlen;
	  else
	    {
	      matlen++;
	      wpat++;
	    }
	  break;
	case L'?':
	  if (*wpat == WLPAREN)
	    return (matlen = -1);		/* XXX for now */
	  else
	    matlen++;
	  break;
	case L'*':
	  return (matlen = -1);
	case L'+':
	case L'!':
	case L'@':
	  if (*wpat == WLPAREN)
	    return (matlen = -1);		/* XXX for now */
	  else
	    matlen++;
	  break;
	case L'[':
	  /* scan for ending `]', skipping over embedded [:...:] */
	  bracklen = 1;
	  wc = *wpat++;
	  do
	    {
	      if (wc == 0)
		{
		  wpat--;			/* back up to NUL */
	          matlen += bracklen;
	          goto bad_bracket;
	        }
	      else if (wc == L'\\')
		{
		  /* *wpat == backslash-escaped character */
		  bracklen++;
		  /* If the backslash or backslash-escape ends the string,
		     bail.  The ++wpat skips over the backslash escape */
		  if (*wpat == 0 || *++wpat == 0)
		    {
		      matlen += bracklen;
		      goto bad_bracket;
		    }
		}
	      else if (wc == L'[' && *wpat == L':')	/* character class */
		{
		  wpat++;
		  bracklen++;
		  in_cclass = 1;
		}
	      else if (in_cclass && wc == L':' && *wpat == L']')
		{
		  wpat++;
		  bracklen++;
		  in_cclass = 0;
		}
	      else if (wc == L'[' && *wpat == L'.')	/* collating symbol */
		{
		  wpat++;
		  bracklen++;
		  if (*wpat == L']')	/* right bracket can appear as collating symbol */
		    {
		      wpat++;
		      bracklen++;
		    }
		  in_collsym = 1;
		}
	      else if (in_collsym && wc == L'.' && *wpat == L']')
		{
		  wpat++;
		  bracklen++;
		  in_collsym = 0;
		}
	      else if (wc == L'[' && *wpat == L'=')	/* equivalence class */
		{
		  wpat++;
		  bracklen++;
		  if (*wpat == L']')	/* right bracket can appear as equivalence class */
		    {
		      wpat++;
		      bracklen++;
		    }
		  in_equiv = 1;
		}
	      else if (in_equiv && wc == L'=' && *wpat == L']')
		{
		  wpat++;
		  bracklen++;
		  in_equiv = 0;
		}
	      else
		bracklen++;
	    }
	  while ((wc = *wpat++) != L']');
	  matlen++;		/* bracket expression can only match one char */
bad_bracket:
	  break;
	}
    }

  return matlen;
}
#endif

int
extglob_pattern_p (pat)
     char *pat;
{
  switch (pat[0])
    {
    case '*':
    case '+':
    case '!':
    case '@':
    case '?':
      return (pat[1] == LPAREN);
    default:
      return 0;
    }
    
  return 0;
}

#undef FOLD
#define FOLD(c) ((flags & FNM_CASEFOLD) \
	? TOLOWER ((unsigned char)c) \
	: ((unsigned char)c))

/* Return 1 of the first character of STRING could match the first
   character of pattern PAT.  Used to avoid n2 calls to strmatch().
   FLAGS is a subset of strmatch flags; used to do case-insensitive
   matching for now. */
int
match_pattern_char (pat, string, flags)
     char *pat, *string;
     int flags;
{
  char c;

  if (*string == 0)
    return (*pat == '*');	/* XXX - allow only * to match empty string */

  switch (c = *pat++)
    {
    default:
      return (FOLD(*string) == FOLD(c));
    case '\\':
      return (FOLD(*string) == FOLD(*pat));
    case '?':
      return (*pat == LPAREN ? 1 : (*string != '\0'));
    case '*':
      return (1);
    case '+':
    case '!':
    case '@':
      return (*pat == LPAREN ? 1 : (FOLD(*string) == FOLD(c)));
    case '[':
      return (*string != '\0');
    }
}

int
umatchlen (pat, max)
     char *pat;
     size_t max;
{
  char c;
  int matlen, bracklen, t, in_cclass, in_collsym, in_equiv;

  if (*pat == 0)
    return (0);

  matlen = in_cclass = in_collsym = in_equiv = 0;
  while (c = *pat++)
    {
      switch (c)
	{
	default:
	  matlen++;
	  break;
	case '\\':
	  if (*pat == 0)
	    return ++matlen;
	  else
	    {
	      matlen++;
	      pat++;
	    }
	  break;
	case '?':
	  if (*pat == LPAREN)
	    return (matlen = -1);		/* XXX for now */
	  else
	    matlen++;
	  break;
	case '*':
	  return (matlen = -1);
	case '+':
	case '!':
	case '@':
	  if (*pat == LPAREN)
	    return (matlen = -1);		/* XXX for now */
	  else
	    matlen++;
	  break;
	case '[':
	  /* scan for ending `]', skipping over embedded [:...:] */
	  bracklen = 1;
	  c = *pat++;
	  do
	    {
	      if (c == 0)
		{
		  pat--;			/* back up to NUL */
		  matlen += bracklen;
		  goto bad_bracket;
	        }
	      else if (c == '\\')
		{
		  /* *pat == backslash-escaped character */
		  bracklen++;
		  /* If the backslash or backslash-escape ends the string,
		     bail.  The ++pat skips over the backslash escape */
		  if (*pat == 0 || *++pat == 0)
		    {
		      matlen += bracklen;
		      goto bad_bracket;
		    }
		}
	      else if (c == '[' && *pat == ':')	/* character class */
		{
		  pat++;
		  bracklen++;
		  in_cclass = 1;
		}
	      else if (in_cclass && c == ':' && *pat == ']')
		{
		  pat++;
		  bracklen++;
		  in_cclass = 0;
		}
	      else if (c == '[' && *pat == '.')	/* collating symbol */
		{
		  pat++;
		  bracklen++;
		  if (*pat == ']')	/* right bracket can appear as collating symbol */
		    {
		      pat++;
		      bracklen++;
		    }
		  in_collsym = 1;
		}
	      else if (in_collsym && c == '.' && *pat == ']')
		{
		  pat++;
		  bracklen++;
		  in_collsym = 0;
		}
	      else if (c == '[' && *pat == '=')	/* equivalence class */
		{
		  pat++;
		  bracklen++;
		  if (*pat == ']')	/* right bracket can appear as equivalence class */
		    {
		      pat++;
		      bracklen++;
		    }
		  in_equiv = 1;
		}
	      else if (in_equiv && c == '=' && *pat == ']')
		{
		  pat++;
		  bracklen++;
		  in_equiv = 0;
		}
	      else
		bracklen++;
	    }
	  while ((c = *pat++) != ']');
	  matlen++;		/* bracket expression can only match one char */
bad_bracket:
	  break;
	}
    }

  return matlen;
}

#if defined (EXTENDED_GLOB)
/* Skip characters in PAT and return the final occurrence of DIRSEP.  This
   is only called when extended_glob is set, so we have to skip over extglob
   patterns x(...) */
char *
glob_dirscan (pat, dirsep)
     char *pat;
     int dirsep;
{
  char *p, *d, *pe, *se;

  d = pe = se = 0;
  for (p = pat; p && *p; p++)
    {
      if (extglob_pattern_p (p))
	{
	  if (se == 0)
	    se = p + strlen (p) - 1;
	  pe = glob_patscan (p + 2, se, 0);
	  if (pe == 0)
	    continue;
	  else if (*pe == 0)
	    break;
	  p = pe - 1;	/* will do increment above */
	  continue;
	}
      if (*p ==  dirsep)
	d = p;
    }
  return d;
}
#endif /* EXTENDED_GLOB */
