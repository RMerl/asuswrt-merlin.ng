/*
 * Declare directives for structure packing. No padding will be provided
 * between the members of packed structures, and therefore, there is no
 * guarantee that structure members will be aligned.
 *
 * Declaring packed structures is compiler specific. In order to handle all
 * cases, packed structures should be delared as:
 *
 * #include <packed_section_start.h>
 *
 * typedef BWL_PRE_PACKED_STRUCT struct foobar_t {
 *    some_struct_members;
 * } BWL_POST_PACKED_STRUCT foobar_t;
 *
 * #include <packed_section_end.h>
 *
 *
 * Copyright (C) 2020, Broadcom. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: packed_section_start.h 777067 2019-07-18 09:28:30Z $
 */

#ifndef _alignment_test_
#define _alignment_test_

/* ASSERT default packing */
typedef struct T4 {
	uint8  a;
	uint32 b;
	uint16 c;
	uint8  d;
} T4_t;

/* 4 byte alignment support */
/*
* a . . .
* b b b b
* c c d .
*/

/*
 * Below function is meant to verify that this file is compiled with the default alignment of 4.
 * Function will fail to compile if the condition is not met.
 */
#ifdef __GNUC__
#define VARIABLE_IS_NOT_USED __attribute__ ((unused))
#else
#define VARIABLE_IS_NOT_USED
#endif // endif
static void alignment_test(void);
static void
VARIABLE_IS_NOT_USED alignment_test(void)
{
	/* verify 4 byte alignment support */
	STATIC_ASSERT(sizeof(T4_t) == 12);
}
#endif /* _alignment_test_ */

/* Error check - BWL_PACKED_SECTION is defined in packed_section_start.h
 * and undefined in packed_section_end.h. If it is already defined at this
 * point, then there is a missing include of packed_section_end.h.
 */
#ifdef BWL_PACKED_SECTION
	#error "BWL_PACKED_SECTION is already defined!"
#else
	#define BWL_PACKED_SECTION
#endif // endif

#if defined(BWL_DEFAULT_PACKING)
	/* generate an error if BWL_DEFAULT_PACKING is defined */
	#error "BWL_DEFAULT_PACKING not supported any more."
#endif /* BWL_PACKED_SECTION */

#if defined(_MSC_VER)
#pragma warning(disable:4103)
#pragma pack(push)
#pragma pack(1)
#endif // endif

/* Declare compiler-specific directives for structure packing. */
#if defined(_MSC_VER)
	#define	BWL_PRE_PACKED_STRUCT
	#define	BWL_POST_PACKED_STRUCT
#elif defined(__GNUC__) || defined(__lint)
	#define	BWL_PRE_PACKED_STRUCT
	#define	BWL_POST_PACKED_STRUCT	__attribute__ ((packed))
#elif defined(__CC_ARM)
	#define	BWL_PRE_PACKED_STRUCT	__packed
	#define	BWL_POST_PACKED_STRUCT
#else
	#error "Unknown compiler!"
#endif // endif
