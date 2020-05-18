/*****************************************************

  SIMD_CP instrinsics for X86 with SSE and MMX

***************************************************/

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




/* Include the correct SIMD file */



/* Includes */
/* Includes */
/* Includes */

   #include <xmmintrin.h>


/* Register/Variable typedefs */
/* Register/Variable typedefs */
/* Register/Variable typedefs */

/* simd_m128 is a multi-use 128bit variable containing 4 ints or 4 floats */
/* ALL SIMD ops in this header use this variable type */

   typedef union {
	__m128 v   ;  /* the packed SIMD data (vector) */
	float  f[4];  /* signed float access */
	int    i[4];  /* int access */
	short  s[8];  /* short access */
	char   c[16]; /* char access */
	__m64  m[2];  /* x86 needs for MMX commands - sse has no integer */
   } simd_m128 ;






/* Helper commands */
/* Helper commands */
/* Helper commands */

   #define simd_load4Floats(v,a,b,c,d) v.f[0]=a;v.f[1]=b;v.f[2]=c;v.f[3]=d
   #define simd_load4Ints(v,a,b,c,d)   v.i[0]=a;v.i[1]=b;v.i[2]=c;v.i[3]=d

   /* print a simd_m128 variable:  l=label to print, a=simd_m128 variable */
   #define simd_print4Floats(l,a) printf("%s: %f  %f  %f  %f\n",l,a.f[0],a.f[1],a.f[2],a.f[3])
   #define simd_print4Ints(l,a)   printf("%s: %d  %d  %d  %d\n",l,a.i[0],a.i[1],a.i[2],a.i[3])
   #define simd_print4UInts(l,a)  printf("%s: %d  %d  %d  %d\n",l,(unsigned int)a.i[0],(unsigned int)a.i[1],(unsigned int)a.i[2],(unsigned int)a.i[3])
   #define simd_print4Hex(l,a)    printf("%s: 0x%08x  0x%08x  0x%08x  0x%08x\n",l,a.i[0],a.i[1],a.i[2],a.i[3])

   /* v0 = v1 {mask} v2, where mask controlls which packed values are copied */
   #define simd_copy(v1,v2,m,v0)  v0.v=_mm_suffle_ps(v1.v,v2.v,m)



/* Integer SIMD intrinsics */
/* Integer SIMD intrinsics */
/* Integer SIMD intrinsics */


   /* Signed ints (4 packed ints) */

   /* v0 = 4 ints in v1 added to 4 ints in v2 */
   #define simd_4i_add(v1,v2,v0)  v0.m[0]=_mm_add_pi32(v1.m[0],v2.m[0]);v0.m[1]=_mm_add_pi32(v1.m[1],v2.m[1])

   /* v0 = 4 ints in v1 subracted from 4 ints in v2 */
   #define simd_4i_sub(v1,v2,v0)  v0.m[0]=_mm_sub_pi32(v1.m[0],v2.m[0]);v0.m[1]=_mm_sub_pi32(v1.m[1],v2.m[1])

   /* no integer SIMD mult on x86? */
   /* v0 = 4 ints in v1 multiplied by 4 ints in v2 */
   #define simd_4i_mult(v1,v2,v0) v0.i[0]=v1.i[0]*v2.i[0]; v0.i[1]=v1.i[1]*v2.i[1]; v0.i[2]=v1.i[2]*v2.i[2]; v0.i[3]=v1.i[3]*v2.i[3]

   /* no integer SIMD div on x86? */
   /* v0 = 4 ints in v1 divided by 4 ints in v2 */
   #define simd_4i_div(v1,v2,v0) v0.i[0]=v1.i[0]/v2.i[0]; v0.i[1]=v1.i[1]/v2.i[1]; v0.i[2]=v1.i[2]/v2.i[2]; v0.i[3]=v1.i[3]/v2.i[3]

   /* ints logical/bitwise */
   
   /* v0 = 4 ints in v1 & 4 ints in v2 */
   #define simd_4i_band(v1,v2,v0) v0.v=_mm_and_ps(v1.v,v2.v)

   /* v0 = 4 ints in v1 & 4 ints in v2 */
   #define simd_4i_bor(v1,v2,v0)  v0.v=_mm_or_ps(v1.v,v2.v)

   /* v0 = 4 ints in v1 & 4 ints in v2 */
   #define simd_4i_bxor(v1,v2,v0) v0.v=_mm_xor_ps(v1.v,v2.v)
   
   /* V0 = 4 ints in V1, each shifted left  (<<) X bits */
   #define simd_4i_shftl(v1,x,v0)  v0.m[0]=_mm_sll_pi32(v1.m[0],x);v0.m[1]=_mm_sll_pi32(v1.m[1],x)

   /* V0 = 4 ints in V1, each shifted right (>>) X bits */
   #define simd_4i_shftr(v1,x,v0)  v0.m[0]=_mm_srl_pi32(v1.m[0],x);v0.m[1]=_mm_srl_pi32(v1.m[1],x)


   
   /* Signed shorts (8 packed shorts) */

   /* v0 = 8 shorts in v1 added to 8 shorts in v2 */
   #define simd_4s_add(v1,v2,v0)  v0.m[0]=_mm_add_pi16(v1.m[0],v2.m[0]);v0.m[1]=_mm_add_pi16(v1.m[1],v2.m[1])

   /* v0 = 8 shorts in v1 subracted from 8 shorts in v2 */
   #define simd_4s_sub(v1,v2,v0)  v0.m[0]=_mm_sub_pi16(v1.m[0],v2.m[0]);v0.m[1]=_mm_sub_pi16(v1.m[1],v2.m[1])

   /* V0 = 8 shorts in V1, each shifted left  (<<) X bits */
   #define simd_4s_shftl(v1,x,v0)  v0.m[0]=_mm_sll_pi16(v1.m[0],x);v0.m[1]=_mm_sll_pi16(v1.m[1],x)

   /* V0 = 8 shorts in V1, each shifted right (>>) X bits */
   #define simd_4s_shftr(v1,x,v0)  v0.m[0]=_mm_srl_pi16(v1.m[0],x);v0.m[1]=_mm_srl_pi16(v1.m[1],x)
   

   
   /* Signed chars (16 packed chars) */

   /* v0 = 16 chars in v1 added to 16 chars in v2 */
   #define simd_4c_add(v1,v2,v0)  v0.m[0]=_mm_add_pi8(v1.m[0],v2.m[0]);v0.m[1]=_mm_add_pi8(v1.m[1],v2.m[1])

   /* v0 = 16 chars in v1 subracted from 16 chars in v2 */
   #define simd_4c_sub(v1,v2,v0)  v0.m[0]=_mm_sub_pi8(v1.m[0],v2.m[0]);v0.m[1]=_mm_sub_pi8(v1.m[1],v2.m[1])

   /* V0 = 16 chars in V1, each shifted left  (<<) X bits */
   #define simd_4c_shftl(v1,x,v0)  v0.m[0]=_mm_sll_pi8(v1.m[0],x);v0.m[1]=_mm_sll_pi8(v1.m[1],x)

   /* V0 = 16 chars in V1, each shifted right (>>) X bits */
   #define simd_4c_shftr(v1,x,v0)  v0.m[0]=_mm_srl_pi8(v1.m[0],x);v0.m[1]=_mm_srl_pi8(v1.m[1],x)






/* Floating SIMD intrinsics */
/* Floating SIMD intrinsics */
/* Floating SIMD intrinsics */


   /* math */

   /* v0 = 4 floats in v1 added to 4 floats in v2 */
   #define simd_4f_add(v1,v2,v0)  v0.v=_mm_add_ps(v1.v,v2.v)

   /* v0 = 4 floats in v1 subtracted from 4 floats in v2 */
   #define simd_4f_sub(v1,v2,v0)  v0.v=_mm_sub_ps(v1.v,v2.v)

   /* v0 = 4 floats in v1 multiplied by 4 floats in v2 */
   #define simd_4f_mult(v1,v2,v0) v0.v=_mm_mul_ps(v1.v,v2.v)

   /* v0 = 4 floats in v1 divided by 4 floats in v2 */
   #define simd_4f_div(v1,v2,v0)  v0.v=_mm_div_ps(v1.v,v2.v)

   /* v0 = 4 floats in v1 sqrt */
   #define simd_4f_sqrt(v1,v0)    v0.v=_mm_sqrt_ps(v1.v)
   
   /* v0 = 1/(4 floats in v1) */
   #define simd_4f_recp(v1,v0)    v0.v=_mm_rcp_ps(v1.v)
   
   /* compares */
   
   /* v0 = 4 floats min(v1,v2) */
   #define simd_4f_min(v1,v2,v0)  v0.v=_mm_min_ps(v1.v,v2.v)
   
   /* v0 = 4 floats max(v1,v2) */
   #define simd_4f_max(v1,v2,v0)  v0.v=_mm_max_ps(v1.v,v2.v)

   /* compare masks */
   
   /* v0 = (4 floats in v1 == 4 floats in v1) */
   #define simd_4f_meq(v1,v2,v0)  v0.v=_mm_cmpeq_ps(v1.v,v2.v)
   
   /* v0 = (4 floats in v1 != 4 floats in v1) */
   #define simd_4f_mneq(v1,v2,v0)  v0.v=_mm_cmpneq_ps(v1.v,v2.v)

   /* v0 = (4 floats in v1 < 4 floats in v1) */
   #define simd_4f_mlt(v1,v2,v0)  v0.v=_mm_cmplt_ps(v1.v,v2.v)

   /* v0 = (4 floats in v1 > 4 floats in v1) */
   #define simd_4f_mgt(v1,v2,v0)  v0.v=_mm_cmpgt_ps(v1.v,v2.v)

   /* v0 = (4 floats in v1 <= 4 floats in v1) */
   #define simd_4f_mle(v1,v2,v0)  v0.v=_mm_cmple_ps(v1.v,v2.v)

   /* v0 = (4 floats in v1 >= 4 floats in v1) */
   #define simd_4f_mge(v1,v2,v0)  v0.v=_mm_cmpge_ps(v1.v,v2.v)



/* Utility Functions and non-SIMD */
/* Utility Functions and non-SIMD */
/* Utility Functions and non-SIMD */

/********************************

  These functions that optional.  If they
  are not available on an architecture, they
  are defined out, and won't hurt anything.  If they
  are available, they will only help.
*/

   /* Tell the MMU you'll soon be accessing some memory from
      the address "p".  On x86, one cache line is loaded.  There
      are three flavors for x86, on other procs they'll probably
      all be defined out or all set to the same type */
   
   /* This is for data which will be accessed once
      (OTU = One Time Use */
   #define simd_ut_prefetchOTU(p) _mm_prefetch(p,_MM_HINT_NTA)

   /* Fetch into the L2 and pass to the L1 cache */
   #define simd_ut_prefetchL1(p) _mm_prefetch(p,_MM_HINT_NTA)

   /* Fetch into the L2 cache only */
   #define simd_ut_prefetchL2(p) _mm_prefetch(p,_MM_HINT_NTA)


