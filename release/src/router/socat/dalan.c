/* source: dalan.c */
/* Copyright Gerhard Rieger and contributors (see file CHANGES) */
/* Published under the GNU General Public License V.2, see file COPYING */

/* idea of a low level data description language. currently only a most
   primitive subset exists. */

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#if HAVE_STDBOOL_H
#include <stdbool.h>
#endif
#include <ctype.h>
#include "dalan.h"

/* test structure to find maximal alignment */
static struct {
  char a;
  long double b;
} maxalign;

/* test structure to find minimal alignment */
static struct {
  char a;
  char b;
} minalign;

/* test union to find kind of byte ordering */
static union {
  char a[2];
  short b;
} byteorder = { "01" };

struct dalan_opts_s dalan_opts = {
  sizeof(int),
  sizeof(short),
  sizeof(long),
  sizeof(char),
  sizeof(float),
  sizeof(double)
} ;

/* fill the dalan_opts structure with machine dependent defaults values. */
static void _dalan_dflts(struct dalan_opts_s *dlo) {
  dlo->c_int = sizeof(int);
  dlo->c_short = sizeof(short);
  dlo->c_long = sizeof(long);
  dlo->c_char = sizeof(char);
  dlo->c_float = sizeof(float);
  dlo->c_double = sizeof(double);
  dlo->maxalign = (char *)&maxalign.b-&maxalign.a;
  dlo->minalign = &minalign.b-&minalign.a;
  dlo->byteorder = (byteorder.b!=7711);
}

/* allocate a new dalan_opts structure, fills it with machine dependent
   defaults values, and returns the pointer. */
struct dalan_opts_s *dalan_props(void) {
  struct dalan_opts_s *dlo;
  dlo = malloc(sizeof(struct dalan_opts_s));
  if (dlo == NULL) {
    return NULL;
  }
  _dalan_dflts(dlo);
  return dlo;
}

void dalan_init(void) {
  _dalan_dflts(&dalan_opts);
}

/* read data description from line, write result to data; do not write
   so much data that *p exceeds n !
   p must be initialized to 0.
   return 0 on success,
   -1 if the data was cut due to n limit,
   1 if a syntax error occurred
   *p is a global data counter; especially it must be used when calculating
     alignment. On successful return from the function *p must be actual!
*/
int dalan(const char *line, char *data, size_t *p, size_t n) {
  int align, mask, i, x;
  size_t p1 = *p;
  char c;

  /*fputs(line, stderr); fputc('\n', stderr);*/
  while (c = *line++) {
    switch (c) {
    case ' ':
    case '\t':
    case '\r':
    case '\n':
      break;
    case ',':
      align = 2;
      while (*line == ',') {
	align <<= 1;
	++line;
      }
      mask = align - 1;	/* create the bitmask */
      i = (align - (p1 & mask)) & mask;
      while (i && p1<n) data[p1++] = 0, --i;
      if (i) { *p = p1; return -1; }
      break;
    case ';':
      align = dalan_opts.c_int;
      mask = align - 1;
      i = (align - (p1 & mask)) & mask;
      while (i && p1<n) data[p1++] = 0, --i;
      if (i) { *p = p1; return -1; }
      break;
    case '"':
      while (1) {
	switch (c = *line++) {
	case '\0': fputs("unterminated string\n", stderr);
	  return 1;
	case '"':
	  break;
	case '\\':
	  if (!(c = *line++)) {
	    fputs("continuation line not implemented\n", stderr);
	    return 1;
	  }
	  switch (c) {
	  case 'n': c = '\n'; break;
	  case 'r': c = '\r'; break;
	  case 't': c = '\t'; break;
	  case 'f': c = '\f'; break;
	  case 'b': c = '\b'; break;
	  case 'a': c = '\a'; break;
#if 0
	  case 'e': c = '\e'; break;
#else
	  case 'e': c = '\033'; break;
#endif
	  case '0': c = '\0'; break;
	  }
	  /* PASSTHROUGH */
	default:
	  if (p1 >= n) { *p = p1; return -1; }
	  data[p1++] = c;
	  continue;
	}
	if (c == '"')
	  break;
      }
      break;
    case '\'':
      switch (c = *line++) {
      case '\0': fputs("unterminated character\n", stderr);
	return 1;
      case '\'': fputs("error in character\n", stderr);
	return 1;
      case '\\':
	if (!(c = *line++)) {
	  fputs("continuation line not implemented\n", stderr);
	  return 1;
	}
	switch (c) {
	case 'n': c = '\n'; break;
	case 'r': c = '\r'; break;
	case 't': c = '\t'; break;
	case 'f': c = '\f'; break;
	case 'b': c = '\b'; break;
	case 'a': c = '\a'; break;
#if 0
	case 'e': c = '\e'; break;
#else
	case 'e': c = '\033'; break;
#endif
	}
	/* PASSTHROUGH */
      default:
	if (p1 >= n) { *p = p1; return -1; }
	data[p1++] = c;
	break;
      }
      if (*line != '\'') {
	fputs("error in character termination\n", stderr);
	*p = p1; return 1;
      }
      ++line;
      break;
#if LATER
    case '0':
      c = *line++;
      if (c == 'x') {
      /* hexadecimal */ ;
      } else if (isdigit(c&0xff)) {
	/* octal */
      } else {
	/* it was only 0 */
      }
      break;
#endif /* LATER */
    case 'x':
      /* expecting hex data, must be an even number of digits!! */
      while (true) {
	c = *line;
	if (isdigit(c&0xff)) {
	  x = (c-'0') << 4;
	} else if (isxdigit(c&0xff)) {
	  x = ((c&0x07) + 9) << 4;
	} else
	  break;
	++line;
	c = *line;
	if (isdigit(c&0xff)) {
	  x |= (c-'0');
	} else if (isxdigit(c&0xff)) {
	  x |= (c&0x07) + 9;
	} else {
	  fputs("odd number of hexadecimal digits\n", stderr);
	  *p = p1; return 1;
	}
	++line;
	if (p1 >= n) { *p = p1; return -1; }
	data[p1++] = x;
      }
      break;
    case 'A': case 'a':
    case 'C': case 'c':
    default: fprintf(stderr, "syntax error in \"%s\"\n", line-1);
      return 1;
    }
  }
  *p = p1; return 0;
}
