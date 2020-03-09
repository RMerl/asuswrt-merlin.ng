/* Copyright (c) 2003-2004, Roger Dingledine
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2019, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file compat_compiler.h
 * \brief Utility macros to handle different features and behavior in different
 *    compilers.
 **/

#ifndef TOR_COMPAT_COMPILER_H
#define TOR_COMPAT_COMPILER_H

#include "orconfig.h"
#include <inttypes.h>

#if defined(__has_feature)
#  if __has_feature(address_sanitizer)
/* Some of the fancy glibc strcmp() macros include references to memory that
 * clang rejects because it is off the end of a less-than-3. Clang hates this,
 * even though those references never actually happen. */
#    undef strcmp
#endif /* __has_feature(address_sanitizer) */
#endif /* defined(__has_feature) */

#ifndef NULL_REP_IS_ZERO_BYTES
#error "It seems your platform does not represent NULL as zero. We can't cope."
#endif

#ifndef DOUBLE_0_REP_IS_ZERO_BYTES
#error "It seems your platform does not represent 0.0 as zeros. We can't cope."
#endif

#if 'a'!=97 || 'z'!=122 || 'A'!=65 || ' '!=32
#error "It seems that you encode characters in something other than ASCII."
#endif

/* GCC can check printf and scanf types on arbitrary functions. */
#ifdef __GNUC__
#define CHECK_PRINTF(formatIdx, firstArg) \
   __attribute__ ((format(printf, formatIdx, firstArg)))
#else
#define CHECK_PRINTF(formatIdx, firstArg)
#endif /* defined(__GNUC__) */
#ifdef __GNUC__
#define CHECK_SCANF(formatIdx, firstArg) \
   __attribute__ ((format(scanf, formatIdx, firstArg)))
#else
#define CHECK_SCANF(formatIdx, firstArg)
#endif /* defined(__GNUC__) */

/* What GCC do we have? */
#ifdef __GNUC__
#define GCC_VERSION (__GNUC__ * 100 + __GNUC_MINOR__)
#else
#define GCC_VERSION 0
#endif

/* Temporarily enable and disable warnings. */
#ifdef __GNUC__
#  define PRAGMA_STRINGIFY_(s) #s
#  define PRAGMA_JOIN_STRINGIFY_(a,b) PRAGMA_STRINGIFY_(a ## b)
/* Support for macro-generated pragmas (c99) */
#  define PRAGMA_(x) _Pragma (#x)
#  ifdef __clang__
#    define PRAGMA_DIAGNOSTIC_(x) PRAGMA_(clang diagnostic x)
#  else
#    define PRAGMA_DIAGNOSTIC_(x) PRAGMA_(GCC diagnostic x)
#  endif
#  if defined(__clang__) || GCC_VERSION >= 406
/* we have push/pop support */
#    define DISABLE_GCC_WARNING(warningopt) \
          PRAGMA_DIAGNOSTIC_(push) \
          PRAGMA_DIAGNOSTIC_(ignored PRAGMA_JOIN_STRINGIFY_(-W,warningopt))
#    define ENABLE_GCC_WARNING(warningopt) \
          PRAGMA_DIAGNOSTIC_(pop)
#else /* !(defined(__clang__) || GCC_VERSION >= 406) */
/* older version of gcc: no push/pop support. */
#    define DISABLE_GCC_WARNING(warningopt) \
         PRAGMA_DIAGNOSTIC_(ignored PRAGMA_JOIN_STRINGIFY_(-W,warningopt))
#    define ENABLE_GCC_WARNING(warningopt) \
         PRAGMA_DIAGNOSTIC_(warning PRAGMA_JOIN_STRINGIFY_(-W,warningopt))
#endif /* defined(__clang__) || GCC_VERSION >= 406 */
#else /* !defined(__GNUC__) */
/* not gcc at all */
# define DISABLE_GCC_WARNING(warning)
# define ENABLE_GCC_WARNING(warning)
#endif /* defined(__GNUC__) */

/* inline is __inline on windows. */
#ifdef _WIN32
#define inline __inline
#endif

/* Try to get a reasonable __func__ substitute in place. */
#if defined(_MSC_VER)

#define __func__ __FUNCTION__

#else
/* For platforms where autoconf works, make sure __func__ is defined
 * sanely. */
#ifndef HAVE_MACRO__func__
#ifdef HAVE_MACRO__FUNCTION__
#define __func__ __FUNCTION__
#elif HAVE_MACRO__FUNC__
#define __func__ __FUNC__
#else
#define __func__ "???"
#endif /* defined(HAVE_MACRO__FUNCTION__) || ... */
#endif /* !defined(HAVE_MACRO__func__) */
#endif /* defined(_MSC_VER) */

#ifdef ENUM_VALS_ARE_SIGNED
#define ENUM_BF(t) unsigned
#else
/** Wrapper for having a bitfield of an enumerated type. Where possible, we
 * just use the enumerated type (so the compiler can help us and notice
 * problems), but if enumerated types are unsigned, we must use unsigned,
 * so that the loss of precision doesn't make large values negative. */
#define ENUM_BF(t) t
#endif /* defined(ENUM_VALS_ARE_SIGNED) */

/* GCC has several useful attributes. */
#if defined(__GNUC__) && __GNUC__ >= 3
#define ATTR_NORETURN __attribute__((noreturn))
#define ATTR_CONST __attribute__((const))
#define ATTR_MALLOC __attribute__((malloc))
#define ATTR_NORETURN __attribute__((noreturn))
#define ATTR_WUR __attribute__((warn_unused_result))
#define ATTR_UNUSED __attribute__ ((unused))

/** Macro: Evaluates to <b>exp</b> and hints the compiler that the value
 * of <b>exp</b> will probably be true.
 *
 * In other words, "if (PREDICT_LIKELY(foo))" is the same as "if (foo)",
 * except that it tells the compiler that the branch will be taken most of the
 * time.  This can generate slightly better code with some CPUs.
 */
#define PREDICT_LIKELY(exp) __builtin_expect(!!(exp), 1)
/** Macro: Evaluates to <b>exp</b> and hints the compiler that the value
 * of <b>exp</b> will probably be false.
 *
 * In other words, "if (PREDICT_UNLIKELY(foo))" is the same as "if (foo)",
 * except that it tells the compiler that the branch will usually not be
 * taken.  This can generate slightly better code with some CPUs.
 */
#define PREDICT_UNLIKELY(exp) __builtin_expect(!!(exp), 0)
#else /* !(defined(__GNUC__) && __GNUC__ >= 3) */
#define ATTR_NORETURN
#define ATTR_CONST
#define ATTR_MALLOC
#define ATTR_NORETURN
#define ATTR_UNUSED
#define ATTR_WUR
#define PREDICT_LIKELY(exp) (exp)
#define PREDICT_UNLIKELY(exp) (exp)
#endif /* defined(__GNUC__) && __GNUC__ >= 3 */

/** Expands to a syntactically valid empty statement.  */
#define STMT_NIL (void)0

/** Expands to a syntactically valid empty statement, explicitly (void)ing its
 * argument. */
#define STMT_VOID(a) while (0) { (void)(a); }

#ifdef __GNUC__
/** STMT_BEGIN and STMT_END are used to wrap blocks inside macros so that
 * the macro can be used as if it were a single C statement. */
#define STMT_BEGIN (void) ({
#define STMT_END })
#elif defined(sun) || defined(__sun__)
#define STMT_BEGIN if (1) {
#define STMT_END } else STMT_NIL
#else
#define STMT_BEGIN do {
#define STMT_END } while (0)
#endif /* defined(__GNUC__) || ... */

/* Some tools (like coccinelle) don't like to see operators as macro
 * arguments. */
#define OP_LT <
#define OP_GT >
#define OP_GE >=
#define OP_LE <=
#define OP_EQ ==
#define OP_NE !=

#if defined(__MINGW32__) || defined(__MINGW64__)
#define MINGW_ANY
#endif

/** Macro: yield a pointer to the field at position <b>off</b> within the
 * structure <b>st</b>.  Example:
 * <pre>
 *   struct a { int foo; int bar; } x;
 *   ptrdiff_t bar_offset = offsetof(struct a, bar);
 *   int *bar_p = STRUCT_VAR_P(&x, bar_offset);
 *   *bar_p = 3;
 * </pre>
 */
#define STRUCT_VAR_P(st, off) ((void*) ( ((char*)(st)) + (off) ) )

/** Macro: yield a pointer to an enclosing structure given a pointer to
 * a substructure at offset <b>off</b>. Example:
 * <pre>
 *   struct base { ... };
 *   struct subtype { int x; struct base b; } x;
 *   struct base *bp = &x.base;
 *   struct *sp = SUBTYPE_P(bp, struct subtype, b);
 * </pre>
 */
#define SUBTYPE_P(p, subtype, basemember) \
  ((void*) ( ((char*)(p)) - offsetof(subtype, basemember) ))

/** Macro: Yields the number of elements in array x. */
#define ARRAY_LENGTH(x) ((sizeof(x)) / sizeof(x[0]))

/**
 * "Eat" a semicolon that somebody puts at the end of a top-level macro.
 *
 * Frequently, we want to declare a macro that people will use at file scope,
 * and we want to allow people to put a semicolon after the macro.
 *
 * This declaration of a struct can be repeated any number of times, and takes
 * a trailing semicolon afterwards.
 **/
#define EAT_SEMICOLON                                   \
  struct dummy_semicolon_eater__

#endif /* !defined(TOR_COMPAT_COMPILER_H) */
