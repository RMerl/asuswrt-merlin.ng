/*  *********************************************************************
    *  Broadcom Common Firmware Environment (CFE)
    *  
    *  ARM Macros				File: armmacros.h
    *
    *  Macros to deal with various arm-related things.
    *  
    *  Author:  
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
#ifndef	__ARMMACROS_H__
#define __ARMMACROS_H__

/*  *********************************************************************
    *  32/64-bit macros
    ********************************************************************* */

#ifdef __long64
#define _VECT_	.dword
#define _LONG_	.dword
#define REGSIZE	8
#define BPWSIZE 3		/* bits per word size */
#define _TBLIDX(x) ((x)*REGSIZE)
#else
#define _VECT_	.word
#define _LONG_	.word
#define REGSIZE 4
#define BPWSIZE 2
#define _TBLIDX(x) ((x)*REGSIZE)
#endif

#ifndef FUNC
  #if defined(__thumb__)
    #define FUNC(x)	THUMBLEAF(x)
  #else
    #define FUNC(x)	LEAF(x)
  #endif	/* __thumb__ */
#endif

#ifdef CFG_ARMV8_AARCH64
#define lr	x30
#define R_FUNC	x15
#define R_ARG	x0

#define BL	blr
#define RET	ret
#else
#define R_FUNC	r13
#define R_ARG	r0

#define BL	blx
#define RET	mov pc, lr
#endif


/*  *********************************************************************
    *  LOADREL(reg,label)
    *
    *  Load the address of a label, but do it in a position-independent
    *  way.
    *
    *  Input parameters:
    *	   reg - register to load
    *	   label - label whose address to load
    *
    *  Return value:
    *	   ra is trashed!
    ********************************************************************* */
#if !defined(CFG_RAMAPP)
#define	LOADREL(reg,label)			\
	bl 1f;					\
1:	nop;					\
	ldr reg,=1b;				\
	sub lr,lr,reg;				\
	ldr reg,label;				\
	add reg,reg,lr;
#else
#define	LOADREL(reg,label)			\
	ldr reg, label
#endif

/* use this MACRO for calling a func in PIC code. Use r13(sp) for temp storage
   assume stack is not setup and used in PIC code
*/
#define CALLINIT(func)                          \
	LOADREL(R_FUNC, func);                      \
	BL	R_FUNC

#if defined(__ASSEMBLER__)
#define EX_LEVEL_3  0x03
#define EX_LEVEL_2  0x02
.macro GET_EXCEPTION_LEVEL
	mrs x0, CurrentEL           // Current EL is in bits [3:2]
	lsr x0, x0, #2              // Put it in [1:0]
	and x0, x0, #3              // Only want [1:0]
.endm
#if !defined(CFG_RAMAPP)
#define SETLEDS(a,b,c,d)			      \
	ldr R_ARG, =(((a)<<24)|((b)<<16)|((c)<<8)|(d)) ;  \
	CALLINIT(=board_setleds)
#else
#define SETLEDS(a,b,c,d)
#endif
#endif

#define board_setledsC(a,b,c,d) board_setleds(((a)<<24)|((b)<<16)|((c)<<8)|(d))

#endif
