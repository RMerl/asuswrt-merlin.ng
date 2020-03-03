/*  *********************************************************************
    *  Broadcom Common Firmware Environment (CFE)
    *  
    *  Basic types				File: lib_types.h
    *  
    *  This module defines the basic types used in CFE.  Simple
    *  types, such as uint64_t, are defined here.
    *  
    *  Author:  Mitch Lichtenberg (mpl@broadcom.com)
    *  
    *********************************************************************  
    *
    *  Copyright 2000,2001,2002,2003
    *  Broadcom Corporation. All rights reserved.
    *  
    *  This software is furnished under license and may be used and 
    *  copied only in accordance with the following terms and 
    *  conditions.  Subject to these conditions, you may download, 
    *  copy, install, use, modify and distribute modified or unmodified 
    *  copies of this software in source and/or binary form.  No title 
    *  or ownership is transferred hereby.
    *  
    *  1) Any source code used, modified or distributed must reproduce 
    *     and retain this copyright notice and list of conditions 
    *     as they appear in the source file.
    *  
    *  2) No right is granted to use any trade name, trademark, or 
    *     logo of Broadcom Corporation.  The "Broadcom Corporation" 
    *     name may not be used to endorse or promote products derived 
    *     from this software without the prior written permission of 
    *     Broadcom Corporation.
    *  
    *  3) THIS SOFTWARE IS PROVIDED "AS-IS" AND ANY EXPRESS OR
    *     IMPLIED WARRANTIES, INCLUDING BUT NOT LIMITED TO, ANY IMPLIED
    *     WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR 
    *     PURPOSE, OR NON-INFRINGEMENT ARE DISCLAIMED. IN NO EVENT 
    *     SHALL BROADCOM BE LIABLE FOR ANY DAMAGES WHATSOEVER, AND IN 
    *     PARTICULAR, BROADCOM SHALL NOT BE LIABLE FOR DIRECT, INDIRECT,
    *     INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
    *     (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
    *     GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
    *     BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY 
    *     OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR 
    *     TORT (INCLUDING NEGLIGENCE OR OTHERWISE), EVEN IF ADVISED OF 
    *     THE POSSIBILITY OF SUCH DAMAGE.
    ********************************************************************* */


#ifndef __ASSEMBLER__

#ifdef __mips
#if ((defined(__MIPSEB)+defined(__MIPSEL)) != 1)
#error "Either __MIPSEB or __MIPSEL must be defined!"
#endif
#endif


#ifndef _LIB_TYPES_H
#define _LIB_TYPES_H


/*  *********************************************************************
    *  Constants
    ********************************************************************* */

#ifndef NULL
#define NULL 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

/*  *********************************************************************
    *  Basic types
    ********************************************************************* */

#ifndef _SIZE_T
#define _SIZE_T
typedef __SIZE_TYPE__ size_t;
#endif


#ifndef __BIT_TYPES_DEFINED__
#define __BIT_TYPES_DEFINED__

typedef char int8_t;
typedef unsigned char uint8_t;

typedef short int16_t;
typedef unsigned short uint16_t;

typedef int int32_t;
typedef unsigned int uint32_t;

typedef long long int64_t;
typedef unsigned long long uint64_t;

#endif /* !(__BIT_TYPES_DEFINED__) */

#ifndef _GCC_STDINT_H
typedef long intptr_t;
typedef unsigned long uintptr_t;
#endif

/*  *********************************************************************
    *  Macros
    ********************************************************************* */

#ifndef offsetof
#define offsetof(type,memb) ((size_t)&((type *)0)->memb)
#endif

/*  *********************************************************************
    *  Structures
    ********************************************************************* */

typedef struct cons_s {
    char *str;
    int num;
} cons_t;

#endif

#endif /* !(__ASSEMBLER__) */
