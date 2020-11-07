/************************************************************
 *
 * <:copyright-BRCM:2012:DUAL/GPL:standard
 * 
 *    Copyright (c) 2012 Broadcom 
 *    All Rights Reserved
 * 
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 * 
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 * 
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
 * 
 * :>
 ************************************************************/


#ifdef __mips
#if ((defined(__MIPSEB)+defined(__MIPSEL)) != 1)
#error "Either __MIPSEB or __MIPSEL must be defined!"
#endif
#endif


#ifndef _LIB_TYPES_H
#define _LIB_TYPES_H

#ifndef _LINUX_TYPES_H

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
typedef long unsigned int size_t;

typedef char int8_t;
typedef unsigned char uint8_t;

typedef short int16_t;
typedef unsigned short uint16_t;

#ifdef __long64
typedef int int32_t;
typedef unsigned int uint32_t;
#else
typedef long int32_t;
typedef unsigned long uint32_t;
#endif

typedef long long int64_t;
typedef unsigned long long uint64_t;

#define unsigned signed		/* Kludge to get unsigned size-shaped type. */
typedef __SIZE_TYPE__ intptr_t;
#undef unsigned
typedef __SIZE_TYPE__ uintptr_t;

/*  *********************************************************************
    *  Macros
    ********************************************************************* */

#ifndef offsetof
#define offsetof(type,memb) ((size_t)&((type *)0)->memb)
#endif

#endif

/*  *********************************************************************
    *  Structures
    ********************************************************************* */

typedef struct cons_s {
    char *str;
    int num;
} cons_t;

#endif
