/*
 * Dropbear - a SSH2 server
 * 
 * Copyright (c) 2002,2003 Matt Johnston
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE. */

#ifndef DROPBEAR_DEBUG_H_
#define DROPBEAR_DEBUG_H_

#include "includes.h"

/* Debugging */

/* Work well for valgrind - don't clear environment, be nicer with signals
 * etc. Don't use this normally, it might cause problems */
/* #define DEBUG_VALGRIND */

/* All functions writing to the cleartext payload buffer call
 * CHECKCLEARTOWRITE() before writing. This is only really useful if you're
 * attempting to track down a problem */
/*#define CHECKCLEARTOWRITE() assert(ses.writepayload->len == 0 && \
		ses.writepayload->pos == 0)*/

#ifndef CHECKCLEARTOWRITE
#define CHECKCLEARTOWRITE()
#endif

/* A couple of flags, not usually useful, and mightn't do anything */

/*#define DEBUG_KEXHASH*/
/*#define DEBUG_RSA*/

/* The level of TRACE() statements */
#define DROPBEAR_VERBOSE_LEVEL 4

#if DEBUG_TRACE
extern int debug_trace;
#endif

/* Enable debug trace levels.
   We can't use __VA_ARGS_ here because Dropbear supports 
   old ~C89 compilers */
/* Default is to discard output ... */
#define DEBUG1(X)
#define DEBUG2(X)
#define DEBUG3(X)
#define TRACE(X)
#define TRACE2(X)
/* ... unless DEBUG_TRACE is high enough */
#if (DEBUG_TRACE>=1)
#undef DEBUG1
#define DEBUG1(X) dropbear_trace1 X;
#endif
#if (DEBUG_TRACE>=2)
#undef DEBUG2
#define DEBUG2(X) dropbear_trace2 X;
#endif
#if (DEBUG_TRACE>=3)
#undef DEBUG3
#define DEBUG3(X) dropbear_trace3 X;
#endif
#if (DEBUG_TRACE>=4)
#undef TRACE
#define TRACE(X) dropbear_trace4 X;
#endif
#if (DEBUG_TRACE>=5)
#undef TRACE2
#define TRACE2(X) dropbear_trace5 X;
#endif

/* To debug with GDB it is easier to run with no forking of child processes.
   You will need to pass "-F" as well. */
#ifndef DEBUG_NOFORK
#define DEBUG_NOFORK 0
#endif


/* For testing as non-root on shadowed systems, include the crypt of a password
 * here. You can then log in as any user with this password. Ensure that you
 * make your own password, and are careful about using this. This will also
 * disable some of the chown pty code etc*/
/* #define DEBUG_HACKCRYPT "hL8nrFDt0aJ3E" */ /* this is crypt("password") */

#endif
