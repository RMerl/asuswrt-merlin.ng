/*

*/

#ifndef _LIB_MATH_H_
#define _LIB_MATH_H_

#ifndef __ASSEMBLER__

#define MIN(a,b)  ((a) <= (b)? (a):(b))
#define MAX(a,b)  ((a) >= (b)? (a):(b))
#define MAX3(a,b,c) MAX(MAX(a,b),c) 
#define MAX4(a,b,c,d) MAX(MAX3(a,b,c),d) 


#endif /*__ASSEMBLER__*/

#endif

