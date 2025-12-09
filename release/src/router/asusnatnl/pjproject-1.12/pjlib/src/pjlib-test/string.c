/* $Id: string.c 3553 2011-05-05 06:14:19Z nanang $ */
/* 
 * Copyright (C) 2008-2011 Teluu Inc. (http://www.teluu.com)
 * Copyright (C) 2003-2008 Benny Prijono <benny@prijono.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
 */
#include <pj/string.h>
#include <pj/assert.h>
#include <pj/errno.h>
#include <pj/pool.h>
#include <pj/log.h>
#include <pj/os.h>
#include "test.h"

#define THIS_FILE	"string.c"

/**
 * \page page_pjlib_string_test Test: String
 *
 * This file provides implementation of \b string_test(). It tests the
 * functionality of the string API.
 *
 * \section sleep_test_sec Scope of the Test
 *
 * API tested:
 *  - pj_str()
 *  - pj_strcmp()
 *  - pj_strcmp2()
 *  - pj_stricmp()
 *  - pj_strlen()
 *  - pj_strncmp()
 *  - pj_strnicmp()
 *  - pj_strchr()
 *  - pj_strdup()
 *  - pj_strdup2()
 *  - pj_strcpy()
 *  - pj_strcat()
 *  - pj_strtrim()
 *  - pj_utoa()
 *  - pj_strtoul()
 *  - pj_strtoul2()
 *  - pj_create_random_string()
 *  - ... and mode..
 *
 * This file is <b>pjlib-test/string.c</b>
 *
 * \include pjlib-test/string.c
 */

#if INCLUDE_STRING_TEST

#ifdef _MSC_VER
#   pragma warning(disable: 4204)
#endif

#define HELLO_WORLD	"Hello World"
#define HELLO_WORLD_LEN	11
#define JUST_HELLO	"Hello"
#define JUST_HELLO_LEN	5
#define UL_VALUE	3456789012UL

#if 1
/* See if both integers have the same sign */
PJ_INLINE(int) cmp(const char *expr, int i, int j)
{
    int r = !((i>0 && j>0) || (i<0 && j<0) || (i==0 && j==0));
    if (r) {
	PJ_LOG(3,(THIS_FILE,"   error: %s: expecting %d, got %d", expr, j, i));
    }
    return r;
}
#else
/* For strict comparison, must be equal */
PJ_INLINE(int) cmp(const char *expr, int i, int j)
{
    PJ_UNUSED_ARG(expr);
    return i!=j;
}
#endif

#define C(expr, res)	cmp(#expr, expr, res)

static int stricmp_test(void)
{
/* This specificly tests and benchmark pj_stricmp(), pj_stricmp_alnum().
 * In addition, it also tests pj_stricmp2(), pj_strnicmp(), and 
 * pj_strnicmp2().
 */
#define STRTEST(res,res2,S1,S2,code)	\
	    do { \
		s1.ptr=S1; s1.slen=(S1)?len:0; \
		s2.ptr=S2; s2.slen=(S2)?len:0; \
		pj_get_timestamp(&t1); \
	        if (C(pj_stricmp(&s1,&s2),res)) return code; \
		pj_get_timestamp(&t2); \
		pj_sub_timestamp(&t2, &t1); \
		pj_add_timestamp(&e1, &t2); \
		pj_get_timestamp(&t1); \
	        if (C(pj_stricmp_alnum(&s1,&s2),res)) return code-1; \
		pj_get_timestamp(&t2); \
		pj_sub_timestamp(&t2, &t1); \
		pj_add_timestamp(&e2, &t2); \
		if (C(pj_stricmp2(&s1,S2),res2)) return code*10; \
		if (C(pj_strnicmp(&s1,&s2,len),res)) return code*100; \
		if (C(pj_strnicmp2(&s1,S2,len),res)) return code*1000; \
	    } while (0)

    char *buf;
    pj_str_t s1, s2;
    pj_timestamp t1, t2, e1, e2, zero;
    pj_uint32_t c1, c2;
    int len;

    e1.u32.hi = e1.u32.lo = e2.u32.hi = e2.u32.lo = 0;

    pj_thread_sleep(0);

#define SNULL 0

    /* Compare empty strings. */
    len=0;
    STRTEST( 0, 0, "","",-500);
    STRTEST( 0, 0, SNULL,"",-502);
    STRTEST( 0, 0, "",SNULL,-504);
    STRTEST( 0, 0, SNULL,SNULL,-506);
    STRTEST( 0, -1, "hello","world",-508);

    /* equal, length=1 
     * use buffer to simulate non-aligned string.
     */
    buf = "a""A";
    len=1;
    STRTEST( 0,  -1, "a",buf+0,-510);
    STRTEST( 0,  0, "a",buf+1,-512);
    STRTEST(-1, -1, "O", "P", -514);
    STRTEST(-1, -1, SNULL, "a", -516);
    STRTEST( 1,  1, "a", SNULL, -518);

    /* equal, length=2 
     * use buffer to simulate non-aligned string.
     */
    buf = "aa""Aa""aA""AA";
    len=2;
    STRTEST( 0, -1, "aa",buf+0,-520);
    STRTEST( 0, -1, "aa",buf+2,-522);
    STRTEST( 0, -1, "aa",buf+4,-524);
    STRTEST( 0, 0, "aa",buf+6,-524);

    /* equal, length=3 
     * use buffer to simulate non-aligned string.
     */
    buf = "aaa""Aaa""aAa""aaA""AAa""aAA""AaA""AAA";
    len=3;
    STRTEST( 0, -1, "aaa",buf+0,-530);
    STRTEST( 0, -1, "aaa",buf+3,-532);
    STRTEST( 0, -1, "aaa",buf+6,-534);
    STRTEST( 0, -1, "aaa",buf+9,-536);
    STRTEST( 0, -1, "aaa",buf+12,-538);
    STRTEST( 0, -1, "aaa",buf+15,-540);
    STRTEST( 0, -1, "aaa",buf+18,-542);
    STRTEST( 0, 0, "aaa",buf+21,-534);

    /* equal, length=4 */
    len=4;
    STRTEST( 0, 0, "aaaa","aaaa",-540);
    STRTEST( 0, 0, "aaaa","Aaaa",-542);
    STRTEST( 0, 0, "aaaa","aAaa",-544);
    STRTEST( 0, 0, "aaaa","aaAa",-546);
    STRTEST( 0, 0, "aaaa","aaaA",-548);
    STRTEST( 0, 0, "aaaa","AAaa",-550);
    STRTEST( 0, 0, "aaaa","aAAa",-552);
    STRTEST( 0, 0, "aaaa","aaAA",-554);
    STRTEST( 0, 0, "aaaa","AaAa",-556);
    STRTEST( 0, 0, "aaaa","aAaA",-558);
    STRTEST( 0, 0, "aaaa","AaaA",-560);
    STRTEST( 0, 0, "aaaa","AAAa",-562);
    STRTEST( 0, 0, "aaaa","aAAA",-564);
    STRTEST( 0, 0, "aaaa","AAaA",-566);
    STRTEST( 0, 0, "aaaa","AaAA",-568);
    STRTEST( 0, 0, "aaaa","AAAA",-570);

    /* equal, length=5 */
    buf = "aaaAa""AaaaA""AaAaA""AAAAA";
    len=5;
    STRTEST( 0, -1, "aaaaa",buf+0,-580);
    STRTEST( 0, -1, "aaaaa",buf+5,-582);
    STRTEST( 0, -1, "aaaaa",buf+10,-584);
    STRTEST( 0, 0, "aaaaa",buf+15,-586);

    /* not equal, length=1 */
    len=1;
    STRTEST( -1, -1, "a", "b", -600);

    /* not equal, length=2 */
    buf = "ab""ba";
    len=2;
    STRTEST( -1, -1, "aa", buf+0, -610);
    STRTEST( -1, -1, "aa", buf+2, -612);

    /* not equal, length=3 */
    buf = "aab""aba""baa";
    len=3;
    STRTEST( -1, -1, "aaa", buf+0, -620);
    STRTEST( -1, -1, "aaa", buf+3, -622);
    STRTEST( -1, -1, "aaa", buf+6, -624);

    /* not equal, length=4 */
    buf = "aaab""aaba""abaa""baaa";
    len=4;
    STRTEST( -1, -1, "aaaa", buf+0, -630);
    STRTEST( -1, -1, "aaaa", buf+4, -632);
    STRTEST( -1, -1, "aaaa", buf+8, -634);
    STRTEST( -1, -1, "aaaa", buf+12, -636);

    /* not equal, length=5 */
    buf="aaaab""aaaba""aabaa""abaaa""baaaa";
    len=5;
    STRTEST( -1, -1, "aaaaa", buf+0, -640);
    STRTEST( -1, -1, "aaaaa", buf+5, -642);
    STRTEST( -1, -1, "aaaaa", buf+10, -644);
    STRTEST( -1, -1, "aaaaa", buf+15, -646);
    STRTEST( -1, -1, "aaaaa", buf+20, -648);

    zero.u32.hi = zero.u32.lo = 0;
    c1 = pj_elapsed_cycle(&zero, &e1);
    c2 = pj_elapsed_cycle(&zero, &e2);

    if (c1 < c2) {
	PJ_LOG(3,("", "  info: pj_stricmp_alnum is slower than pj_stricmp!"));
	//return -700;
    }

    /* Avoid division by zero */
    if (c2 == 0) c2=1;
    
    PJ_LOG(3, ("", "  time: stricmp=%u, stricmp_alnum=%u (speedup=%d.%02dx)", 
		   c1, c2,
		   (c1 * 100 / c2) / 100,
		   (c1 * 100 / c2) % 100));
    return 0;
#undef STRTEST
}

/* This tests pj_strcmp(), pj_strcmp2(), pj_strncmp(), pj_strncmp2() */
static int strcmp_test(void)
{
#define STR_TEST(res,S1,S2,code)    \
	    do { \
		s1.ptr=S1; s1.slen=S1?len:0; \
		s2.ptr=S2; s2.slen=S2?len:0; \
	        if (C(pj_strcmp(&s1,&s2),res)) return code; \
		if (C(pj_strcmp2(&s1,S2),res)) return code-1; \
		if (C(pj_strncmp(&s1,&s2,len),res)) return code-2; \
		if (C(pj_strncmp2(&s1,S2,len),res)) return code-3; \
	    } while (0)

    pj_str_t s1, s2;
    int len;
    
    /* Test with length == 0 */
    len=0;
    STR_TEST(0, "", "", -400);
    STR_TEST(0, SNULL, "", -405);
    STR_TEST(0, "", SNULL, -410);
    STR_TEST(0, SNULL, SNULL, -415);
    STR_TEST(0, "hello", "", -420);
    STR_TEST(0, "hello", SNULL, -425);

    /* Test with length != 0 */
    len = 2;
    STR_TEST(0, "12", "12", -430);
    STR_TEST(1, "12", "1", -435);
    STR_TEST(-1, "1", "12", -440);
    STR_TEST(-1, SNULL, "12", -445);
    STR_TEST(1, "12", SNULL, -450);

    return 0;

#undef STR_TEST
}

static int verify_strxcpy(const char *src, int dst_size, int exp_ret, 
                          const char *exp_dst)
{
    char dst[6];
    char GUARD = '@';
    int i, ret;

    PJ_ASSERT_RETURN(src && dst_size <= 5, -700);

    memset(dst, GUARD, sizeof(dst));

    ret = pj_ansi_strxcpy(dst, src, dst_size);

    /* verify return value */
    if (ret != exp_ret) {
        PJ_LOG(3,("", "  strxcpy \"%s\", dst_size=%d: ret %d != %d",
                  src, dst_size, ret, exp_ret));
        return -704;
    }

    /* expected dst content */
    if (exp_dst) {
        if (strcmp(dst, exp_dst)) {
            PJ_LOG(3,("", "  strxcpy \"%s\", dst_size=%d: dst content mismatch: \"%s\"!=\"%s\"",  
                          src, dst_size, dst, exp_dst));
            return -708;
        }
    }

    /* verify not writing pass buffer */
    for (i=exp_dst?strlen(exp_dst)+1:0; i<sizeof(dst); ++i) {
        if (dst[i] != GUARD) {
            PJ_LOG(3,("", "  strxcpy \"%s\", dst_size=%d: overflow at %d",
                          src, dst_size, i));
            return -710;
        }
    }

    return 0;
}

static int strxcpy_test(void)
{
    int rc;
#define CHECK_(src, dst_size, exp_ret, exp_dst)   \
               rc = verify_strxcpy(src, dst_size, exp_ret, exp_dst); \
               if (rc) return rc

    CHECK_( "",     0, -PJ_ETOOBIG, NULL);
    CHECK_( "a",    0, -PJ_ETOOBIG, NULL);

    {
        /* special test 1 (dst contains null) */
        char dst[4];
        pj_bzero(dst, sizeof(dst));
        rc = pj_ansi_strxcpy(dst, "a", 1);
        if (rc != -PJ_ETOOBIG) {
            PJ_LOG(3,("", "  pj_ansi_strxcpy special test 1: ret %d!=%d",
                    rc, -PJ_ETOOBIG));
            return -700;
        }
    }

    CHECK_( "",     1, 0, "");
    CHECK_( "a",    1, -PJ_ETOOBIG, "");
    CHECK_( "ab",   1, -PJ_ETOOBIG, "");
    CHECK_( "abcd", 1, -PJ_ETOOBIG, "");

    CHECK_( "abc",  2, -PJ_ETOOBIG, "a");
    CHECK_( "ab",   2, -PJ_ETOOBIG, "a");
    CHECK_( "a",    2, 1, "a");
    CHECK_( "",     2, 0, "");

    CHECK_( "abcd", 3, -PJ_ETOOBIG, "ab");
    CHECK_( "abc",  3, -PJ_ETOOBIG, "ab");
    CHECK_( "ab",   3, 2, "ab");
    CHECK_( "a",    3, 1, "a");
    CHECK_( "",     3, 0, "");

    CHECK_( "abcde", 4, -PJ_ETOOBIG, "abc");
    CHECK_( "abcd",  4, -PJ_ETOOBIG, "abc");
    CHECK_( "abc",   4, 3, "abc");
    CHECK_( "ab",    4, 2, "ab");
    CHECK_( "a",     4, 1, "a");
    CHECK_( "",      4, 0, "");

    CHECK_( "abcdef", 5, -PJ_ETOOBIG, "abcd");
    CHECK_( "abcde",  5, -PJ_ETOOBIG, "abcd");
    CHECK_( "abcd",   5, 4, "abcd");
    CHECK_( "abc",    5, 3, "abc");
    CHECK_( "ab",     5, 2, "ab");
    CHECK_( "a",      5, 1, "a");
    CHECK_( "",       5, 0, "");

#undef CHECK_

    return 0;
}

static int verify_strxcpy2(const pj_str_t *src, int dst_size, int exp_ret,
                           const char *exp_dst)
{
    char dst[6];
    char GUARD = '@';
    int i, ret;

    PJ_ASSERT_RETURN(src && dst_size <= 5, -720);

    memset(dst, GUARD, sizeof(dst));

    ret = pj_ansi_strxcpy2(dst, src, dst_size);

    /* verify return value */
    if (ret != exp_ret) {
        PJ_LOG(3,("", "  strxcpy2 \"%.*s\" slen=%ld, dst_size=%d: ret %d!=%d",
                 (int)src->slen, src->ptr, src->slen, dst_size, ret, exp_ret));
        return -724;
    }

    /* expected dst content */
    if (exp_dst) {
        if (strcmp(dst, exp_dst)) {
            PJ_LOG(3,("", "  strxcpy2 \"%.*s\" slen=%ld, dst_size=%d: "
                          "dst content mismatch: \"%s\"!=\"%s\"",  
                          (int)src->slen, src->ptr, src->slen, dst_size, dst,
                          exp_dst));
            return -726;
        }
    }

    /* verify not writing pass buffer */
    for (i=dst_size; i<sizeof(dst); ++i) {
        if (dst[i] != GUARD) {
            PJ_LOG(3,("", "  strxcpy2 \"%.*s\" slen=%ld, dst_size=%d: "
                          "overflow at %d (chr %d)",
                          (int)src->slen, src->ptr, src->slen, dst_size, i,
                          (char)(dst[i] & 0xFF)));
            return -728;
        }
    }

    return 0;
}


static int strxcpy2_test(void)
{
    pj_str_t src;
    char nulls[6];
    int rc;

    pj_bzero(nulls, sizeof(nulls));

#define CHECK2_(s, src_len, dst_size, exp_ret, exp_dst)   \
                src.ptr = s; src.slen = src_len; \
                rc = verify_strxcpy2(&src, dst_size, exp_ret, exp_dst); \
                if (rc) return rc

    CHECK2_( NULL,   0, 0, -PJ_ETOOBIG, NULL);
    CHECK2_( "a!",   1, 0, -PJ_ETOOBIG, NULL);

    CHECK2_( "abc!", 3, 1, -PJ_ETOOBIG, "");
    CHECK2_( "ab!",  2, 1, -PJ_ETOOBIG, "");

    /* note for test below: although src contains null and the strlen
       of result (i.e. dst) is zero, strxcpy2 would still return 
       -PJ_ETOOBIG because the required buffer size is assumed to
       be 2 (one for the src->ptr content (although the content is a
       null character), one for the null terminator)
    */
    CHECK2_( nulls,  1, 1, 0, "");
    CHECK2_( "a!",   1, 1, -PJ_ETOOBIG, "");
    CHECK2_( "a",    1, 1, -PJ_ETOOBIG, "");
    CHECK2_( "",     0, 1, 0, "");
    CHECK2_( NULL,   0, 1, 0, "");

    CHECK2_( "abc",  3, 2, -PJ_ETOOBIG, "a");
    CHECK2_( "ab",   2, 2, -PJ_ETOOBIG, "a");
    CHECK2_( "a!",   1, 2, 1, "a");
    CHECK2_( nulls,  1, 2, 0, "");
    CHECK2_( "!",    0, 2, 0, "");
    CHECK2_( NULL,   0, 2, 0, "");

    CHECK2_( "abc",  3, 3, -PJ_ETOOBIG, "ab");
    CHECK2_( "ab",   3, 3, 2, "ab");
    CHECK2_( nulls,  3, 3, 0, "");
    CHECK2_( "abc",  2, 3, 2, "ab");
    CHECK2_( "ab",   2, 3, 2, "ab");
    CHECK2_( "a",    2, 3, 1, "a");
    CHECK2_( nulls,  2, 3, 0, "");
    CHECK2_( "a",    1, 3, 1, "a");
    CHECK2_( "",     1, 3, 0, "");
    CHECK2_( "",     0, 3, 0, "");
    CHECK2_( NULL,   0, 3, 0, "");

    CHECK2_( "abcde",5, 4, -PJ_ETOOBIG, "abc");
    CHECK2_( "abcd", 4, 4, -PJ_ETOOBIG, "abc");
    CHECK2_( "abc",  4, 4, 3, "abc");
    CHECK2_( "ab",   4, 4, 2, "ab");
    CHECK2_( "a",    4, 4, 1, "a");
    CHECK2_( nulls,  4, 4, 0, "");
    CHECK2_( "abc",  3, 4, 3, "abc");
    CHECK2_( "ab",   3, 4, 2, "ab");
    CHECK2_( "ab",   2, 4, 2, "ab");
    CHECK2_( "a",    2, 4, 1, "a");
    CHECK2_( "",     2, 4, 0, "");
    CHECK2_( nulls,  2, 4, 0, "");
    CHECK2_( "a",    1, 4, 1, "a");
    CHECK2_( nulls,  1, 4, 0, "");
    CHECK2_( "a",    0, 4, 0, "");
    CHECK2_( "",     0, 4, 0, "");
    CHECK2_( NULL,   0, 4, 0, "");

#undef CHECK2_

    return 0;
}


static int verify_strxcat(const char *cdst, const char *src, int dst_size,
                          int exp_ret, const char *exp_dst)
{
    char dst[6];
    char GUARD = '@';
    int i, ret;

    PJ_ASSERT_RETURN(src && strlen(cdst) <= 4, -730);
    PJ_ASSERT_RETURN(strlen(cdst) < dst_size ||
                     (strlen(cdst)==0 && dst_size==0), -731);

    memset(dst, GUARD, sizeof(dst));
    if (dst_size) {
        ret = pj_ansi_strxcpy(dst, cdst, dst_size);
        PJ_ASSERT_RETURN(ret==strlen(cdst), -732);
    }

    ret = pj_ansi_strxcat(dst, src, dst_size);

    /* verify return value */
    if (ret != exp_ret) {
        PJ_LOG(3,("", "  strxcat \"%s\", \"%s\", dst_size=%d: ret %d!=%d",
                  cdst, src, dst_size, ret, exp_ret));
        return -734;
    }

    /* expected dst content */
    if (exp_dst) {
        if (strcmp(dst, exp_dst)) {
            PJ_LOG(3,("", "  strxcat \"%s\", \"%s\", dst_size=%d: "
                          "dst content mismatch: \"%s\"!=\"%s\"",  
                          cdst, src, dst_size, dst, exp_dst));
            return -736;
        }
    }

    /* verify not writing past buffer */
    for (i=exp_dst?strlen(exp_dst)+1:0; i<sizeof(dst); ++i) {
        if (dst[i] != GUARD) {
            PJ_LOG(3,("", "  strxcat \"%s\", \"%s\", dst_size=%d: "
                          "overflow at %d",
                          cdst, src, dst_size, i));
            return -738;
        }
    }

    return 0;
}


static int strxcat_test(void)
{
    int rc;
#define CHECK3_(dst, src, dst_size, exp_ret, exp_dst)   \
               rc = verify_strxcat(dst, src, dst_size, exp_ret, exp_dst); \
               if (rc) return rc

    CHECK3_( "", "",     0, -PJ_ETOOBIG, NULL);
    CHECK3_( "", "a",    0, -PJ_ETOOBIG, NULL);

    CHECK3_( "", "",     1, 0, "");
    CHECK3_( "", "a",    1, -PJ_ETOOBIG, "");

    CHECK3_( "", "a",    2, 1, "a");
    CHECK3_( "", "ab",   2, -PJ_ETOOBIG, "a");
    CHECK3_( "0", "",    2, 1, "0");
    CHECK3_( "0", "a",   2, -PJ_ETOOBIG, "0");

    CHECK3_( "", "a",    3, 1, "a");
    CHECK3_( "", "ab",   3, 2, "ab");
    CHECK3_( "", "abc",  3, -PJ_ETOOBIG, "ab");
    CHECK3_( "0", "",    3, 1, "0");
    CHECK3_( "0", "a",   3, 2, "0a");
    CHECK3_( "0", "ab",  3, -PJ_ETOOBIG, "0a");
    CHECK3_( "01", "",   3, 2, "01");
    CHECK3_( "01", "a",  3, -PJ_ETOOBIG, "01");
    CHECK3_( "01", "ab", 3, -PJ_ETOOBIG, "01");

    CHECK3_( "", "a",     4, 1, "a");
    CHECK3_( "", "ab",    4, 2, "ab");
    CHECK3_( "", "abc",   4, 3, "abc");
    CHECK3_( "", "abcd",  4, -PJ_ETOOBIG, "abc");
    CHECK3_( "0", "",     4, 1, "0");
    CHECK3_( "0", "a",    4, 2, "0a");
    CHECK3_( "0", "ab",   4, 3, "0ab");
    CHECK3_( "0", "abc",  4, -PJ_ETOOBIG, "0ab");
    CHECK3_( "01", "",    4, 2, "01");
    CHECK3_( "01", "a",   4, 3, "01a");
    CHECK3_( "01", "ab",  4, -PJ_ETOOBIG, "01a");
    CHECK3_( "01", "abc", 4, -PJ_ETOOBIG, "01a");
    CHECK3_( "012", "",   4, 3, "012");
    CHECK3_( "012", "a",  4, -PJ_ETOOBIG, "012");
    CHECK3_( "012", "ab", 4, -PJ_ETOOBIG, "012");
    CHECK3_( "012", "abc",4, -PJ_ETOOBIG, "012");

#undef CHECK3_
    return 0;
}

int string_test(void)
{
    const pj_str_t hello_world = { HELLO_WORLD, HELLO_WORLD_LEN };
    const pj_str_t just_hello = { JUST_HELLO, JUST_HELLO_LEN };
    pj_str_t s1, s2, s3, s4, s5;
    enum { RCOUNT = 10, RLEN = 16 };
    pj_str_t random[RCOUNT];
    pj_pool_t *pool;
    int i;

    pool = pj_pool_create(mem, SNULL, 4096, 0, SNULL);
    if (!pool) return -5;
    
    /* 
     * pj_str(), pj_strcmp(), pj_stricmp(), pj_strlen(), 
     * pj_strncmp(), pj_strchr() 
     */
    s1 = pj_str(HELLO_WORLD);
    if (pj_strcmp(&s1, &hello_world) != 0)
	return -10;
    if (pj_stricmp(&s1, &hello_world) != 0)
	return -20;
    if (pj_strcmp(&s1, &just_hello) <= 0)
	return -30;
    if (pj_stricmp(&s1, &just_hello) <= 0)
	return -40;
    if (pj_strlen(&s1) != strlen(HELLO_WORLD))
	return -50;
    if (pj_strncmp(&s1, &hello_world, 5) != 0)
	return -60;
    if (pj_strnicmp(&s1, &hello_world, 5) != 0)
	return -70;
    if (pj_strchr(&s1, HELLO_WORLD[1]) != s1.ptr+1)
	return -80;

    /* 
     * pj_strdup() 
     */
    if (!pj_strdup(pool, &s2, &s1))
	return -100;
    if (pj_strcmp(&s1, &s2) != 0)
	return -110;
    
    /* 
     * pj_strcpy(), pj_strcat() 
     */
    s3.ptr = (char*) pj_pool_alloc(pool, 256);
    if (!s3.ptr) 
	return -200;
    pj_strcpy(&s3, &s2);
    pj_strcat(&s3, &just_hello);

    if (pj_strcmp2(&s3, HELLO_WORLD JUST_HELLO) != 0)
	return -210;

    /* 
     * pj_strdup2(), pj_strtrim(). 
     */
    pj_strdup2(pool, &s4, " " HELLO_WORLD "\t ");
    pj_strtrim(&s4);
    if (pj_strcmp2(&s4, HELLO_WORLD) != 0)
	return -250;

    /* 
     * pj_utoa() 
     */
    s5.ptr = (char*) pj_pool_alloc(pool, 16);
    if (!s5.ptr)
	return -270;
    s5.slen = pj_utoa(UL_VALUE, s5.ptr);

    /* 
     * pj_strtoul() 
     */
    if (pj_strtoul(&s5) != UL_VALUE)
	return -280;

    /*
     * pj_strtoul2()
     */
    s5 = pj_str("123456");

    pj_strtoul2(&s5, SNULL, 10);	/* Crash test */

    if (pj_strtoul2(&s5, &s4, 10) != 123456UL)
	return -290;
    if (s4.slen != 0)
	return -291;
    if (pj_strtoul2(&s5, &s4, 16) != 0x123456UL)
	return -292;

    s5 = pj_str("0123ABCD");
    if (pj_strtoul2(&s5, &s4, 10) != 123)
	return -293;
    if (s4.slen != 4)
	return -294;
    if (s4.ptr == SNULL || *s4.ptr != 'A')
	return -295;
    if (pj_strtoul2(&s5, &s4, 16) != 0x123ABCDUL)
	return -296;
    if (s4.slen != 0)
	return -297;

    /* 
     * pj_create_random_string() 
     * Check that no duplicate strings are returned.
     */
    for (i=0; i<RCOUNT; ++i) {
	int j;
	
	random[i].ptr = (char*) pj_pool_alloc(pool, RLEN);
	if (!random[i].ptr)
	    return -320;

        random[i].slen = RLEN;
	pj_create_random_string(random[i].ptr, RLEN);

	for (j=0; j<i; ++j) {
	    if (pj_strcmp(&random[i], &random[j])==0)
		return -330;
	}
    }

    /* Done. */
    pj_pool_release(pool);

    /* Case sensitive comparison test. */
    i = strcmp_test();
    if (i != 0)
	return i;

    /* Caseless comparison test. */
    i = stricmp_test();
    if (i != 0)
	return i;

    /* strxcpy test */
    i = strxcpy_test();
    if (i != 0)
        return i;

    /* strxcpy2 test */
    i = strxcpy2_test();
    if (i != 0)
        return i;

    /* strxcat test */
    i = strxcat_test();
    if (i != 0)
        return i;

    return 0;
}

#else
/* To prevent warning about "translation unit is empty"
 * when this test is disabled. 
 */
int dummy_string_test;
#endif	/* INCLUDE_STRING_TEST */

