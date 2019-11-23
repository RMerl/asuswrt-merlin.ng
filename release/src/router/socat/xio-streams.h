/* source: xio-streams.h */
/* Copyright Gerhard Rieger and contributors (see file CHANGES) */
/* Published under the GNU General Public License V.2, see file COPYING */

/* when including this file select one part that you need by defining the
   appropriate CPP define: 

   (none): standard define, variable, and function declarations
   ENABLE_OPTCODE: option codes for use in enum e_optcode
   ENABLE_OFUNC: option functions for use in enum e_func
*/

#ifdef ENABLE_OPTCODE

#if 0
enum {	/* make syntax feature of editors cooperative */
#endif
   OPT_STREAMS_I_POP_ALL,	/* with POSIX STREAMS */
   OPT_STREAMS_I_PUSH,	/* with POSIX STREAMS */
#if 0
} ;
#endif

#elif defined(ENABLE_OFUNC)

#if 0
enum {	/* make syntax feature of editors cooperative */
#endif
   OFUNC_STREAMS_I_POP_ALL,
   OFUNC_STREAMS_I_PUSH,
#if 0
} ;
#endif

#else	/* normal declarations */

extern const struct optdesc opt_streams_i_pop_all;
extern const struct optdesc opt_streams_i_push;

#endif
