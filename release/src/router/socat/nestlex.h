/* source: nestlex.h */
/* Copyright Gerhard Rieger and contributors (see file CHANGES) */
/* Published under the GNU General Public License V.2, see file COPYING */

#ifndef __nestlex_h_included
#define __nestlex_h_included 1

extern
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
	    );

#endif /* !defined(__nestlex_h_included) */
