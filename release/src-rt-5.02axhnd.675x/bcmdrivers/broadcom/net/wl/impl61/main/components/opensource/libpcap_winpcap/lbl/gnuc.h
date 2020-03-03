/* @(#) $Header: /tcpdump/master/libpcap/lbl/Attic/gnuc.h,v 1.3.1.1 1999-10-07 23:46:41 mcr Exp $ (LBL) */

/* Define __P() macro, if necessary */
#ifndef __P
#if __STDC__
#define __P(protos) protos
#else
#define __P(protos) ()
#endif // endif
#endif // endif

/* inline foo */
#ifdef __GNUC__
#define inline __inline
#else
#define inline
#endif // endif

/*
 * Handle new and old "dead" routine prototypes
 *
 * For example:
 *
 *	__dead void foo(void) __attribute__((volatile));
 *
 */
#ifdef __GNUC__
#ifndef __dead
#define __dead volatile
#endif // endif
#if __GNUC__ < 2  || (__GNUC__ == 2 && __GNUC_MINOR__ < 5)
#ifndef __attribute__
#define __attribute__(args)
#endif // endif
#endif // endif
#else
#ifndef __dead
#define __dead
#endif // endif
#ifndef __attribute__
#define __attribute__(args)
#endif // endif
#endif // endif
