/* source: nestlex.c */
/* Copyright Gerhard Rieger and contributors (see file CHANGES) */
/* Published under the GNU General Public License V.2, see file COPYING */

/* a function for lexical scanning of nested character patterns */

#include "config.h"
#include "mytypes.h"

#include "sysincludes.h"

static int _nestlex(const char **addr,
		    char **token,
		    ptrdiff_t *len,
		    const char *ends[],
		    const char *hquotes[],
		    const char *squotes[],
		    const char *nests[],
		    bool dropquotes,
		    bool c_esc,
		    bool html_esc
		    );

/* sub: scan a string and copy its value to output string
   end scanning when an unescaped, unnested string from ends array is found
   does not copy the end pattern
   does not write a trailing \0 to token
   allows escaping with \ and quoting (\ and quotes are removed)
   allows nesting with div. parens
   returns -1 if out string was too small
   returns 1 if addr ended unexpectedly
   returns 0 if token could be extracted successfully
*/
int nestlex(const char **addr,	/* input string; aft points to end token */
	    char **token,	/* output token; aft points to first unwritten
				   char (caller might want to set it to \0) */
	    size_t *len,	/* remaining bytes in token space (incl. \0) */
	    const char *ends[],	/* list of end strings */
	    const char *hquotes[],/* list of strings that quote (hard qu.) */
	    const char *squotes[],/* list of strings that quote softly */
	    const char *nests[],/* list of strings that start nesting;
				   every second one is matching end */
	    bool dropquotes,	/* drop the outermost quotes */
	    bool c_esc,		/* solve C char escapes: \n \t \0 etc */
	    bool html_esc	/* solve HTML char escapes: %0d %08 etc */
	    ) {
   return
      _nestlex(addr, token, (ptrdiff_t *)len, ends, hquotes, squotes, nests,
	       dropquotes, c_esc, html_esc);
}

static int _nestlex(const char **addr,
		    char **token,
		    ptrdiff_t *len,
		    const char *ends[],
		    const char *hquotes[],
		    const char *squotes[],
		    const char *nests[],
		    bool dropquotes,
		    bool c_esc,
		    bool html_esc
		    ) {
   const char *in = *addr;	/* pointer into input string */
   const char **endx;	/* loops over end patterns */
   const char **quotx;	/* loops over quote patterns */
   const char **nestx;	/* loops over nest patterns */
   char *out = *token;	/* pointer into output token */
   char c;
   int i;
   int result;

   while (true) {

      /* is this end of input string? */
      if (*in == 0)  {
	 
	 break; /* end of string */
      }

      /* first check the end patterns (e.g. for ']') */
      endx = ends;  i = 0;
      while (*endx) {
	 if (!strncmp(in, *endx, strlen(*endx))) {
	    /* this end pattern matches */
	    *addr = in;
	    *token = out;
	    return 0;
	 }
	 ++endx;
      }

      /* check for hard quoting pattern */
      quotx = hquotes;
      while (hquotes && *quotx) {
	 if (!strncmp(in, *quotx, strlen(*quotx))) {
	    /* this quote pattern matches */
	    const char *endnest[2];
	    if (dropquotes) {
	       /* we strip this quote */
	       in += strlen(*quotx);
	    } else {
	       for (i = strlen(*quotx); i > 0; --i) {
		  *out++ = *in++;
		  if (--*len <= 0) { *addr = in; *token = out; return -1; }
	       }
	    }
	    /* we call _nestlex recursively */
	    endnest[0] = *quotx;
	    endnest[1] = NULL;
	    result =
	       _nestlex(&in, &out, len, endnest, NULL/*hquotes*/,
		       NULL/*squotes*/, NULL/*nests*/,
		       false, c_esc, html_esc);
	    if (result == 0 && dropquotes) {
	       /* we strip this quote */
	       in += strlen(*quotx);
	    } else if (result < 0) {
	       *addr = in; *token = out; return result;
	    } else {
	       /* we copy the trailing quote */
	       for (i = strlen(*quotx); i > 0; --i) {
		  *out++ = *in++;
		  if (--*len <= 0) { *addr = in; *token = out; return -1; }
	       }
	    }
	       
	    break;
	 }
	 ++quotx;
      }
      if (hquotes && *quotx != NULL) {
	 /* there was a quote; string might continue with hard quote */
	 continue;
      }

      /* check for soft quoting pattern */
      quotx = squotes;
      while (squotes && *quotx) {
	 if (!strncmp(in, *quotx, strlen(*quotx))) {
	    /* this quote pattern matches */
	    /* we strip this quote */
	    /* we call _nestlex recursively */
	    const char *endnest[2];
	    if (dropquotes) {
	       /* we strip this quote */
	       in += strlen(*quotx);
	    } else {
	       for (i = strlen(*quotx); i > 0; --i) {
		  *out++ = *in++;
		  if (--*len <= 0) { *addr = in; *token = out; return -1; }
	       }
	    }
	    endnest[0] = *quotx;
	    endnest[1] = NULL;
	    result =
	       _nestlex(&in, &out, len, endnest, hquotes,
		       squotes, nests,
		       false, c_esc, html_esc);

	    if (result == 0 && dropquotes) {
	       /* we strip the trailing quote */
	       in += strlen(*quotx);
	    } else if (result < 0) {
	       *addr = in; *token = out; return result;
	    } else {
	       /* we copy the trailing quote */
	       for (i = strlen(*quotx); i > 0; --i) {
		  *out++ = *in++;
		  if (--*len <= 0) { *addr = in; *token = out; return -1; }
	       }
	    }
	    break;
	 }
	 ++quotx;
      }
      if (squotes && *quotx != NULL) {
	 /* there was a soft quote; string might continue with any quote */
	 continue;
      }

      /* check patterns that start a nested clause */
      nestx = nests;  i = 0;
      while (nests && *nestx) {
	 if (!strncmp(in, *nestx, strlen(*nestx))) {
	    /* this nest pattern matches */
	    const char *endnest[2];
	    endnest[0] = nestx[1];
	    endnest[1] = NULL;

	    for (i = strlen(nestx[1]); i > 0; --i) {
	       *out++ = *in++;
	       if (--*len <= 0)  { *addr = in; *token = out; return -1; }
	    }

	    result =
	       _nestlex(&in, &out, len, endnest, hquotes, squotes, nests,
		       false, c_esc, html_esc);
	    if (result == 0) {
	       /* copy endnest */
	       i = strlen(nestx[1]); while (i > 0) {
		  *out++ = *in++;
		  if (--*len <= 0) {
		     *addr = in;
		     *token = out;
		     return -1;
		  }
		  --i;
	       }
	    } else if (result < 0) {
	       *addr = in; *token = out; return result;
	    }
	    break;
	 }
	 nestx += 2;	/* skip matching end pattern in table */
      }
      if (nests && *nestx) {
	 /* we handled a nested expression, continue loop */
	 continue;
      }

      /* "normal" data, possibly escaped */
      c = *in++;
      if (c == '\\') {
	 /* found a plain \ escaped part */
	 c = *in++;
	 if (c == 0)  { /* Warn("trailing '\\'");*/ break; }
	 if (c_esc) { /* solve C char escapes: \n \t \0 etc */
	    switch (c) {
	    case '0': c = '\0'; break;
	    case 'a': c = '\a'; break;
	    case 'b': c = '\b'; break;
	    case 'f': c = '\f'; break;
	    case 'n': c = '\n'; break;
	    case 'r': c = '\r'; break;
	    case 't': c = '\t'; break;
	    case 'v': c = '\v'; break;
#if LATER
	    case 'x': !!! 1 to 2 hex digits; break;
	    case 'u': !!! 4 hex digits?; break;
	    case 'U': !!! 8 hex digits?; break;
#endif
	    default: break;
	    }
	 }
	 *out++ = c;
	 --*len;
	 if (*len <= 0) {
	    *addr = in;
	    *token = out;
	    return -1;	/* output overflow */
	 }
	 continue;
      }

      /* just a simple char */
      *out++ = c;
      --*len;
      if (*len <= 0) {
	 *addr = in;
	 *token = out;
	 return -1;	/* output overflow */
      }

   }
   /* never come here? */

   *addr = in;
   *token = out;
   return 0;	/* OK */
}
