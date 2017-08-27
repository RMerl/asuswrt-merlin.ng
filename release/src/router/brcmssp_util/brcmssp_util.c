/* Copyright (C) 2005 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */


/*
 * brcmssp_util: Broadcom Stack Smashing protection utility library. 
 *
 * (Adapted from dl-osinfo.h, __uClibc_main.c, ssp.c and dl-osinfo.h from uClibc)
 *
 * The GCC -fstack-protector flag introduces canary retrieval and checking code at the 
 * entry and exit points of each function. The canary code references two external symbols:
 * __stack_chk_guard ( the actual canary  value ) and __stack_chk_fail ( function to call 
 * when canary value matching fails ). This library provides Broadcom's custom implementation 
 * of __stack_chk_guard and __stack_chk_fail for all object files which are compiled with
 * the -fstack-protector flags
 *
 */

/** includes **/
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

/** function prototypes **/
static uintptr_t getBrcmSSPStackCanary(void);

/** global variables **/
uintptr_t __stack_chk_guard = 0;

/*********************************************************************/
/* initSSPCanary(). This function is called everytime this library   */
/* is loaded. This function retrives a new canary value and stores   */
/* it in __stack_chk_guard                                           */    
/*********************************************************************/
__attribute__((constructor)) void initBrcmSSPCanary(void)
{
   __stack_chk_guard = getBrcmSSPStackCanary();
}

/*********************************************************************/
/* getStackCanary(). This funcion retrieves the canary value.        */
/*********************************************************************/
static uintptr_t getBrcmSSPStackCanary(void)
{
   uintptr_t ret;
   {
      int fd = open("/dev/urandom", O_RDONLY, 0);
      if (fd >= 0) 
      {
         size_t size = read(fd, &ret, sizeof(ret));
         close(fd);
         if (size == (size_t) sizeof(ret))
            return ret;
      }
   }

   /* Start with the "terminator canary". */
   ret = 0xFF0A0D00UL;

   /* Everything failed? Or we are using a weakened model of the
    * terminator canary */
   {
      struct timeval tv;
      if (gettimeofday(&tv, NULL) != (-1))
         ret ^= tv.tv_usec ^ tv.tv_sec;
   }
   return ret;
}

/*********************************************************************/
/* __stack_chk_fail(). This funcion is called when the canary value  */
/* comparision check fails in a function call. This function sends a */ 
/* SIGABRT to the calling process                                    */
/*********************************************************************/
void __stack_chk_fail(void)
{
   fprintf(stderr, "stack smashing detected: Pid %d, terminated\n", getpid());
   raise(SIGABRT);
}

