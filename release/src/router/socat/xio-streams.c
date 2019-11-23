/* source: xio-streams.c */
/* Copyright Gerhard Rieger and contributors (see file CHANGES) */
/* Published under the GNU General Public License V.2, see file COPYING */

/* this file contains definitions and functions for handling POSIX STREAMS */

/* with this source file a new experimental approach is being introduced:
   normally when adding a new option at least four existing files have to be
   adapted; this is similar for new address types.

   in the future i would like to have a more automatic assembling of socat from
   topic oriented source files. this should make integration and control of
   contributions more easy.

   all code of a new topic - address and option definitions, open code, option
   handing code, ev.parser code, etc. should go into one source file. the
   selection of the desired code parts during the compilation is done by
   setting cpp defines.

   in the same was all public declarations should go in one header (.h) file.
*/

/* do not compile this file directly but include it from other .c files. with
   CPP defines you select one part you want to really get included:

   ENABLE_OPTIONS: activate the definition of the address option records
   ENABLE_APPLYOPTS: C code that applies the address option passed in opt
*/

#ifdef ENABLE_OPTIONS

#ifdef I_POP
const struct optdesc opt_streams_i_pop_all = { "streams-i-pop-all", "pop-all", OPT_STREAMS_I_POP_ALL, GROUP_FD, PH_FD, TYPE_BOOL,   OFUNC_STREAMS_I_POP_ALL, 0, 0 };
#endif
#ifdef I_PUSH
const struct optdesc opt_streams_i_push    = { "streams-i-push",    "push",    OPT_STREAMS_I_PUSH,    GROUP_FD, PH_FD, TYPE_STRING, OFUNC_STREAMS_I_PUSH,    0, 0 };
#endif

#elif defined(ENABLE_APPLYOPT)

#if 0
void dummy(void) {
   if (0) { { ;
#endif
#ifdef I_POP
      } else if (opt->desc->func == OFUNC_STREAMS_I_POP_ALL) {
	 while (Ioctl(fd, I_POP, 0) >= 0) {
	    Warn2("ioctl(%d, I_POP, 0): %s", fd, strerror(errno));
	 }
#endif
#ifdef I_PUSH
      } else if (opt->desc->func == OFUNC_STREAMS_I_PUSH) {
	 if (Ioctl(fd, I_PUSH, opt->value.u_string) < 0) {
	    Warn3("ioctl(%d, I_PUSH, \"%s\"): %s",
		  fd, opt->value.u_string, strerror(errno));
	    opt->desc = ODESC_ERROR; ++opt; continue;
	 }
#endif
#if 0
} } }
#endif

#else /* !defined(ENABLE_APPLYOPT) */

#include "xiosysincludes.h"
#if WITH_STREAMS        /* make this address configure dependend */
#include "xioopen.h"

#include "xio-fd.h"
#include "xio-socket.h" /* _xioopen_connect() */
#include "xio-listen.h"
#include "xio-ipapp.h"
#include "xio-openssl.h"



#endif /* WITH_STREAMS */

#endif /* !defined(ENABLE_OPTIONS) */
