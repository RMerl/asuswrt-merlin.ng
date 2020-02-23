/*
<:
:>
*/
#ifndef  _LIB_UTIL_H
#define _LIB_UTIL_H

/*Handy align macros*/
#define ALIGN_MASK(N) (N-0x1)
#define ALIGN_MASK_COMPL(N) (~ALIGN_MASK(N))
/* this will round to floor with respect to an align number*/
#define ALIGN_FLR(AN,N) ((N)&ALIGN_MASK_COMPL(AN))
#define ALIGN_DOWN(N,AN) ALIGN_FLR(AN,N)
/* this will round to ceiling with respect to an align number*/
#define ALIGN(N,AN) (((N) + ALIGN_MASK(AN))&ALIGN_MASK_COMPL(AN))
#define ALIGNED(N,AN) (((N)&ALIGN_MASK((AN)))==0) 
#endif /*_LIB_UTIL_H_*/
