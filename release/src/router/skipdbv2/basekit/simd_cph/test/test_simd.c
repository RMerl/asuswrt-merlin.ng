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


/* uncomment the line below to force SIMD emulation C code */  
/*#define __SIMD_EMU__  */

#include <simd_cp.h>
#include <stdio.h>

int main(void) {

  simd_m128 v0,v1,v2;
  
  simd_load4Floats(v1,2.0,2.0,2.0,2.0);
  simd_load4Floats(v2,10.0,20.0,30.0,40.0);
  
  printf("\nChecking 4f commands\n");

  simd_print4Floats("v1 ",v1);
  simd_print4Floats("v2 ",v2);
  puts("");
  
  /*  v0 = v1 + v2  */
  simd_4f_add(v1,v2,v0);
  simd_print4Floats("4f_add ",v0);

  /*  v0 = v1 - v2  */
  simd_4f_sub(v1,v2,v0);
  simd_print4Floats("4f_sub ",v0);

  /*  v0 = v1 * v2  */
  simd_4f_mult(v1,v2,v0);
  simd_print4Floats("4f_mult",v0);

  /*  v0 = v1 / v2  */
  simd_4f_div(v1,v2,v0);
  simd_print4Floats("4f_div",v0);
  
  /*
   * If you look at the disassembly of this section on an X86 processor, it will be
   * very tight, as X86 SSE/MMX only handles 2 regs- i.e. A+=B, instead of C=A+B.
   * For the best cross-platform performance, cater to the lowest demoninator and
   * write your code like this.
   */
   
  printf("\ndisassembly test\n");
  simd_4f_add(v1,v2,v1);
  simd_4f_mult(v1,v2,v1);
  simd_4f_sub(v1,v2,v1);
  simd_4f_div(v1,v2,v1);
  


  printf("\nChecking 4i commands\n");

  simd_load4Ints(v1,20,30,40,50);
  simd_load4Ints(v2,2,3,4,5);
  
  simd_print4Ints("v1 ",v1);
  simd_print4Ints("v2 ",v2);
  puts("");

  
  /*  v0 = v1 + v2  */
  simd_4i_add(v1,v2,v0);
  simd_print4Ints("4i_add ",v0);

  /*  v0 = v1 - v2  */
  simd_4i_sub(v1,v2,v0);
  simd_print4Ints("4i_sub ",v0);

  /*  v0 = v1 * v2  */
  simd_4i_mult(v1,v2,v0);
  simd_print4Ints("4i_mult",v0);

  /*  v0 = v1 / v2  */
  simd_4i_div(v1,v2,v0);
  simd_print4Ints("4i_div ",v0);
  

  return 0;
}
