/* source: xioexit.c */
/* Copyright Gerhard Rieger and contributors (see file CHANGES) */
/* Published under the GNU General Public License V.2, see file COPYING */

/* this file contains the source for the extended exit function */

#include "xiosysincludes.h"
#include "compat.h"
#include "xio.h"
#include "error.h"


/* this function closes all open xio sockets on exit, if they are still open.
   It must be registered with atexit(). */ 
void xioexit(void) {
   int i;

   diag_in_handler = 0;
   Debug("starting xioexit()");
   for (i = 0; i < XIO_MAXSOCK; ++i) {
      if (sock[i] != NULL && sock[i]->tag != XIO_TAG_INVALID) {
	 xioclose(sock[i]);
      }
   }
   Debug("finished xioexit()");
}
