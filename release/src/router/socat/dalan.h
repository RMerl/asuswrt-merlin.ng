/* source: dalan.h */
/* Copyright Gerhard Rieger and contributors (see file CHANGES) */
/* Published under the GNU General Public License V.2, see file COPYING */

#ifndef __dalan_h_included
#define __dalan_h_included 1

#include "mytypes.h"

/* machine properties and command line options */
struct dalan_opts_s {
  int c_int;		/* natural int size / C int size */
  int c_short;		/* C short size */
  int c_long;		/* C long size */
  int c_char;		/* C char size */
  int c_float;		/* C float size */
  int c_double;		/* C double size */
  int maxalign;		/* maximal alignment (double after char) */
  int minalign;		/* minimal alignment (char after char) */
  int byteorder;	/* 0: Motorola, network, big endian; 1: Intel, little
			   endian */
} ;

extern struct dalan_opts_s dalan_opts;

extern void dalan_init(void);
extern struct dalan_opts_s *dalan_props(void);
extern int dalan(const char *line, char *data, size_t *p, size_t n);

#endif /* !defined(__dalan_h_included) */
