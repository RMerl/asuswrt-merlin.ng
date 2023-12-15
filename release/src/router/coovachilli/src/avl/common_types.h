
/*
 * The olsr.org Optimized Link-State Routing daemon(olsrd)
 * Copyright (c) 2004-2011, the olsr.org team - see HISTORY file
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the
 *   distribution.
 * * Neither the name of olsr.org, olsrd nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Visit http://www.olsr.org for more information.
 *
 * If you find this software useful feel free to make a donation
 * to the project. For more information see the website or contact
 * the copyright holders.
 *
 */

#ifndef COMMON_TYPES_H_
#define COMMON_TYPES_H_

/* support EXPORT macro of OLSR */
#ifndef EXPORT
#  define EXPORT __attribute__((visibility ("default")))
#endif

/* give everyone an arraysize implementation */
#ifndef ARRAYSIZE
#define ARRAYSIZE(a)  (sizeof(a) / sizeof(*(a)))
#endif

/*
 * This force gcc to always inline, which prevents errors
 * with option -Os
 */
#ifndef INLINE
#ifdef __GNUC__
#define INLINE inline __attribute__((always_inline))
#else
#define INLINE inline
#endif
#endif

/*
 * This include file creates stdint/stdbool datatypes for
 * visual studio, because microsoft does not support C99
 */

/* types */
#ifdef _MSC_VER
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef signed char int8_t;
typedef signed short int16_t;
typedef signed int int32_t;
#else
#include <inttypes.h>
#endif

#if defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L

/* we have a C99 environment */
#include <stdbool.h>
#elif defined __GNUC__

/* we simulate a C99 environment */
#define bool _Bool
#define true 1
#define false 0
#define __bool_true_false_are_defined 1
#endif

/* add some safe-gaurds */
#ifndef _MSC_VER
#if !defined bool || !defined true || !defined false || !defined __bool_true_false_are_defined
#error You have no C99-like boolean types. Please extend src/olsr_type.h!
#endif
#endif

#endif /* COMMON_TYPES_H_ */
