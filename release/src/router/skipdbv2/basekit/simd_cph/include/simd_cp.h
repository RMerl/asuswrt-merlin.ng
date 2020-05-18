/*
Copyright (c) 2004 Patrick Roberts

This software is provided 'as-is', without any express
or implied warranty. In no event will the authors be held
liable for any damages arising from the use of this software.

Permission is granted to anyone to use this software for any
purpose, including commercial applications, and to alter it
and redistribute it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented;
you must not claim that you wrote the original software.
If you use this software in a product, an acknowledgment in
the product documentation would be appreciated but is not required.

2. Altered source versions must be plainly marked as such,
and must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any source distribution.

4. THIS LICENSE MAY NOT BE CHANGED, ASSIGNED, OR MIGRATED WITHOUT
THE AUTHOR'S WRITTEN PERMISSION, WITH THE FOLLOWING EXCEPTIONS:

   a.  This file may be included with GPL/LGPL licensed
  software, but you may not change the license this file
  is released under.
*/


/*****************************************************

   Cross-platform SIMD intrinsics header file

      This is the main file and the only one an application should
   include.  It will find the correct (hopefully) SIMD_CPH
   dialect file to include.
   

*/

#ifndef __SIMD_CPHD_h


/* Defines */
/* Defines */
/* Defines */
#define __SIMD_CPHD_h


/* Figure out what compiler and CPU we have */

#if defined (__SIMD_NO_SIMD__)  || defined (__SIMD_EMU__)
#else /* broken pre-processor */

#ifdef __GNUC__  /* gcc compiler */

  /* x86 compiler */
   #ifdef __i386__
      #define __GNUC__X86__
      #define __FOUND
   #endif
   
  /* PowerPC Compiler */
   #ifdef __ALTIVEC__
      #define __GNUC__RS6__
      #define __FOUND
      #define __SIMD_NO_SIMD /* No PPC intrinsics file right now */
   #endif
   
   #ifdef __XSCALE__
      #define __GNUC__ARM_IWMMX__
      #define __FOUND
   #endif
       
#endif /* __GNUC__ */



#ifdef _MSC_VER  /* Microsoft VC Compiler */

   #ifdef _M_IX86  /* MSVC for X86 CPU  (same as GNUC?)*/
      #define __MSVC__X86__
      #define __FOUND
   #endif

   #ifdef _M_ALPHA /* for Compaq Alpha CPU */
      #define __MSVC__AXP__ 
      #define __FOUND
      #define __SIMD_NO_SIMD__
   #endif
   
#endif /* _WIN32 */



#ifndef __FOUND
  #define __SIMD_EMU__
  //#warning simd_cp.h does not support or could not figure out your compiler and/or CPU.
#endif

#endif /* SIMD_NO_SIMD and SIMD_EMU */


/* Include the correct SIMD file */

#if defined(__SIMD_NO_SIMD__) || defined(__SIMD_EMU__)
   #warning Including Emulated SIMD support...
   #define __UNK__EMU__
   #include <simd_cp_emu.h>
#endif

#if defined( __GNUC__X86__ ) || defined(__MSVC__X86__)
        /* gcc x86 compiler or msvc x86 */
   #include <simd_cp_x86.h>
#endif /* __GNUC__X86__ */

#ifdef __GNUC__ARM_IWMMX /* gcc ARM compiler with XSCALE SIMD */
   #include <simd_cp_arm-iwmmx.h>
#endif


/* Generic Helper commands */
/* Generic Helper commands */
/* Generic Helper commands */

   /* print a simd_m128 variable:  l=label to print, a=simd_m128 variable */
   #define simd_print4Floats(l,a) printf("%s: %f  %f  %f  %f\n",l,a.f[0],a.f[1],a.f[2],a.f[3])
   #define simd_print4Ints(l,a)   printf("%s: %d  %d  %d  %d\n",l,a.i[0],a.i[1],a.i[2],a.i[3])
   #define simd_print4UInts(l,a)  printf("%s: %d  %d  %d  %d\n",l,(unsigned int)a.i[0],(unsigned int)a.i[1],(unsigned int)a.i[2],(unsigned int)a.i[3])
   #define simd_print4Hex(l,a)    printf("%s: 0x%08x  0x%08x  0x%08x  0x%08x\n",l,a.i[0],a.i[1],a.i[2],a.i[3])


#endif /* CP_SIMD_h */
