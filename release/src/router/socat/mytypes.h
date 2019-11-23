/* source: mytypes.h */
/* Copyright Gerhard Rieger and contributors (see file CHANGES) */
/* Published under the GNU General Public License V.2, see file COPYING */

#ifndef __mytypes_h_included
#define __mytypes_h_included 1

/* some types and macros I miss in C89 */

#ifndef HAVE_TYPE_BOOL
#  undef bool
typedef enum { false, true } bool;
#endif

#ifndef Min
#define Min(x,y) ((x)<=(y)?(x):(y))
#endif

#ifndef Max
#define Max(x,y) ((x)>=(y)?(x):(y))
#endif

#define SOCKADDR_MAX UNIX_PATH_MAX

#endif /* __mytypes_h_included */
