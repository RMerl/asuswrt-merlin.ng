/* Copyright (c) 2013-2019, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file testsupport.h
 *
 * \brief Macros to implement mocking and selective exposure for the test code.
 *
 * Each Tor source file is built twice: once with TOR_UNIT_TESTS defined, and
 * once with it undefined.  The only difference between these configurations
 * should be that when building for the tests, more functions are exposed as
 * non-static, and a number of functions are declared as mockable.
 **/

#ifndef TOR_TESTSUPPORT_H
#define TOR_TESTSUPPORT_H

#ifdef TOR_UNIT_TESTS
/** The "STATIC" macro marks a function or variable that is static when
 * building Tor for production, but non-static when building the unit
 * tests. */
#define STATIC
#define EXTERN(type, name) extern type name;
#else
#define STATIC static
#define EXTERN(type, name)
#endif /* defined(TOR_UNIT_TESTS) */

/** Quick and dirty macros to implement test mocking.
 *
 * To use them, suppose that you have a function you'd like to mock
 * with the signature "void writebuf(size_t n, char *buf)".  You can then
 * declare the function as:
 *
 *     MOCK_DECL(void, writebuf, (size_t n, char *buf));
 *
 * and implement it as:
 *
 *     MOCK_IMPL(void,
 *     writebuf,(size_t n, char *buf))
 *     {
 *          ...
 *     }
 *
 * For the non-testing build, this will expand simply into:
 *
 *     void writebuf(size_t n, char *buf);
 *     void
 *     writebuf(size_t n, char *buf)
 *     {
 *         ...
 *     }
 *
 * But for the testing case, it will expand into:
 *
 *     void writebuf__real(size_t n, char *buf);
 *     extern void (*writebuf)(size_t n, char *buf);
 *
 *     void (*writebuf)(size_t n, char *buf) = writebuf__real;
 *     void
 *     writebuf__real(size_t n, char *buf)
 *     {
 *         ...
 *     }
 *
 * This is not a great mocking system!  It is deliberately "the simplest
 * thing that could work", and pays for its simplicity in its lack of
 * features, and in its uglification of the Tor code.  Replacing it with
 * something clever would be a fine thing.
 *
 * @{ */
#ifdef TOR_UNIT_TESTS
#define MOCK_DECL(rv, funcname, arglist)     \
  rv funcname ##__real arglist;              \
  extern rv(*funcname) arglist
#define MOCK_IMPL(rv, funcname, arglist)     \
  rv(*funcname) arglist = funcname ##__real; \
  rv funcname ##__real arglist
#define MOCK_DECL_ATTR(rv, funcname, arglist, attr) \
  rv funcname ##__real arglist attr;                \
  extern rv(*funcname) arglist
#define MOCK_IMPL(rv, funcname, arglist)     \
  rv(*funcname) arglist = funcname ##__real; \
  rv funcname ##__real arglist
#define MOCK(func, replacement)                 \
  do {                                          \
    (func) = (replacement);                     \
  } while (0)
#define UNMOCK(func)                            \
  do {                                          \
    func = func ##__real;                       \
  } while (0)
#else /* !(defined(TOR_UNIT_TESTS)) */
#define MOCK_DECL(rv, funcname, arglist) \
  rv funcname arglist
#define MOCK_DECL_ATTR(rv, funcname, arglist, attr) \
  rv funcname arglist attr
#define MOCK_IMPL(rv, funcname, arglist) \
  rv funcname arglist
#endif /* defined(TOR_UNIT_TESTS) */
/** @} */

#endif /* !defined(TOR_TESTSUPPORT_H) */
