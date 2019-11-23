/* source: xioopen.h */
/* Copyright Gerhard Rieger and contributors (see file CHANGES) */
/* Published under the GNU General Public License V.2, see file COPYING */

#ifndef __xioopen_h_included
#define __xioopen_h_included 1

#include "compat.h"	/* F_pid */
#include "mytypes.h"
#include "error.h"
#include "utils.h"
#include "sysutils.h"

#include "sycls.h"
#include "sslcls.h"
#include "dalan.h"
#include "filan.h"
#include "xio.h"
#include "xioopts.h"


#if WITH_HELP
#define HELP(x) , x
#else
#define HELP(x)
#endif



/* xioinitialize performs asserts on these records */
extern const struct optdesc opt_crdly;
extern const struct optdesc opt_tabdly;
extern const struct optdesc opt_csize;


struct addrname {
   const char *name;
   const struct addrdesc *desc;
} ;

extern const char *ddirection[];
extern const char *filetypenames[];
extern const struct addrname addressnames[];
extern const struct optname optionnames[];

extern int xioopen_makedual(xiofile_t *file);

#define retropt_2bytes(o,c,r) retropt_ushort(o,c,r)

/* mode_t might be unsigned short or unsigned int or what else? */
#if HAVE_BASIC_MODE_T==1
#  define retropt_modet(x,y,z) retropt_short(x,y,z)
#elif HAVE_BASIC_MODE_T==2
#  define retropt_modet(x,y,z) retropt_ushort(x,y,z)
#elif HAVE_BASIC_MODE_T==3
#  define retropt_modet(x,y,z) retropt_int(x,y,z)
#elif HAVE_BASIC_MODE_T==4
#  define retropt_modet(x,y,z) retropt_uint(x,y,z)
#elif HAVE_BASIC_MODE_T==5
#  define retropt_modet(x,y,z) retropt_long(x,y,z)
#elif HAVE_BASIC_MODE_T==6
#  define retropt_modet(x,y,z) retropt_ulong(x,y,z)
#endif

#if HAVE_BASIC_UID_T==1
#  define retropt_uidt(x,y,z) retropt_short(x,y,z)
#elif HAVE_BASIC_UID_T==2
#  define retropt_uidt(x,y,z) retropt_ushort(x,y,z)
#elif HAVE_BASIC_UID_T==3
#  define retropt_uidt(x,y,z) retropt_int(x,y,z)
#elif HAVE_BASIC_UID_T==4
#  define retropt_uidt(x,y,z) retropt_uint(x,y,z)
#elif HAVE_BASIC_UID_T==5
#  define retropt_uidt(x,y,z) retropt_long(x,y,z)
#elif HAVE_BASIC_UID_T==6
#  define retropt_uidt(x,y,z) retropt_ulong(x,y,z)
#endif

#if HAVE_BASIC_GID_T==1
#  define retropt_gidt(x,y,z) retropt_short(x,y,z)
#elif HAVE_BASIC_GID_T==2
#  define retropt_gidt(x,y,z) retropt_ushort(x,y,z)
#elif HAVE_BASIC_GID_T==3
#  define retropt_gidt(x,y,z) retropt_int(x,y,z)
#elif HAVE_BASIC_GID_T==4
#  define retropt_gidt(x,y,z) retropt_uint(x,y,z)
#elif HAVE_BASIC_GID_T==5
#  define retropt_gidt(x,y,z) retropt_long(x,y,z)
#elif HAVE_BASIC_GID_T==6
#  define retropt_gidt(x,y,z) retropt_ulong(x,y,z)
#endif


#endif /* !defined(__xioopen_h_included) */
