/* source: xiolockfile.h */
/* Copyright Gerhard Rieger and contributors (see file CHANGES) */
/* Published under the GNU General Public License V.2, see file COPYING */

#ifndef __xiolockfile_h_included
#define __xiolockfile_h_included 1

/* preferred lock handling functions */
extern int xiolock(xiolock_t *lock);
extern int xiounlock(const char *lockfile);

/* more "internal" functions */
extern int xiogetlock(const char *lockfile);
extern int xiowaitlock(const char *lockfile, struct timespec *intervall);
extern int xiofiledroplock(xiofile_t *xfd);

#endif /* !defined(__xiolockfile_h_included) */
