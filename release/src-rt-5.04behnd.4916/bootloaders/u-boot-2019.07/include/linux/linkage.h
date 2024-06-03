/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * U-Boot - linkage.h
 *
 * Copyright (c) 2005-2007 Analog Devices Inc.
 */

#ifndef _LINUX_LINKAGE_H
#define _LINUX_LINKAGE_H

#include <asm/linkage.h>

/* Some toolchains use other characters (e.g. '`') to mark new line in macro */
#ifndef ASM_NL
#define ASM_NL		 ;
#endif

#ifdef __cplusplus
#define CPP_ASMLINKAGE		extern "C"
#else
#define CPP_ASMLINKAGE
#endif

#ifndef asmlinkage
#define asmlinkage CPP_ASMLINKAGE
#endif

#define SYMBOL_NAME_STR(X)	#X
#define SYMBOL_NAME(X)		X
#ifdef __STDC__
#define SYMBOL_NAME_LABEL(X)	X##:
#else
#define SYMBOL_NAME_LABEL(X)	X:
#endif

#ifndef __ALIGN
#define __ALIGN .align		4
#endif

#ifndef __ALIGN_STR
#define __ALIGN_STR		".align 4"
#endif

#ifdef __ASSEMBLY__

#define ALIGN			__ALIGN
#define ALIGN_STR		__ALIGN_STR

#define LENTRY(name) \
	ALIGN ASM_NL \
	SYMBOL_NAME_LABEL(name)

#define ENTRY(name) \
	.globl SYMBOL_NAME(name) ASM_NL \
	LENTRY(name)

#define WEAK(name) \
	.weak SYMBOL_NAME(name) ASM_NL \
	LENTRY(name)

#ifndef END
#define END(name) \
	.size name, .-name
#endif

#ifndef ENDPROC
#define ENDPROC(name) \
	.type name STT_FUNC ASM_NL \
	END(name)
#endif

#endif

#endif
