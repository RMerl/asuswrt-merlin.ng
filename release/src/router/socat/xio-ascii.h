/* source: xio-ascii.h */
/* Copyright Gerhard Rieger and contributors (see file CHANGES) */
/* Published under the GNU General Public License V.2, see file COPYING */

#ifndef __xio_ascii_h_included
#define __xio_ascii_h_included 1

extern char *
   xiob64encodeline(const char *data,		/* input data */
		    size_t bytes,		/* length of input data, >=0 */
		    char *coded	/* output buffer, must be long enough */
		    );
extern char *xiosanitize(const char *data,	/* input data */
		  size_t bytes,			/* length of input data, >=0 */
		  char *coded	/* output buffer, must be long enough */
			 );
extern char *
   xiohexdump(const unsigned char *data, size_t bytes, char *coded);

extern char *
xiodump(const unsigned char *data, size_t bytes, char *coded, size_t codlen,
	int coding);

#endif /* !defined(__xio_ascii_h_included) */
