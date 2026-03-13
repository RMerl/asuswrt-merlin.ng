/*
 * Copyright (C) 2008-2017 Tobias Brunner
 * Copyright (C) 2008 Martin Willi
 *
 * Copyright (C) secunet Security Networks AG
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

/**
 * @defgroup utils_i utils
 * @{ @ingroup utils
 */

#ifndef UTILS_H_
#define UTILS_H_

#define _GNU_SOURCE
#include <sys/types.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <sys/time.h>
#include <string.h>
#include <stdarg.h>

#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif

#ifdef WIN32
# include "compat/windows.h"
#else
# include <arpa/inet.h>
# include <sys/socket.h>
# include <netdb.h>
# include <netinet/in.h>
# include <sched.h>
# include <poll.h>
# include <signal.h>
#endif

#include "utils/types.h"
#include "enum.h"
#include "utils/atomics.h"
#include "utils/align.h"
#include "utils/byteorder.h"
#include "utils/string.h"
#include "utils/memory.h"
#include "utils/strerror.h"
#include "utils/status.h"
#include "utils/object.h"
#include "utils/path.h"
#include "utils/time.h"
#include "utils/tty.h"
#ifdef __APPLE__
# include "compat/apple.h"
#endif
#ifdef __ANDROID__
# include "compat/android.h"
#endif

/**
 * Initialize utility functions
 */
void utils_init();

/**
 * Deinitialize utility functions
 */
void utils_deinit();

/**
 * strongSwan program return codes
 */
#define SS_RC_LIBSTRONGSWAN_INTEGRITY	64
#define SS_RC_DAEMON_INTEGRITY			65
#define SS_RC_INITIALIZATION_FAILED		66

#define SS_RC_FIRST	SS_RC_LIBSTRONGSWAN_INTEGRITY
#define SS_RC_LAST	SS_RC_INITIALIZATION_FAILED

/**
 * Number of bits in a byte
 */
#define BITS_PER_BYTE 8

/**
 * Default length for various auxiliary text buffers
 */
#define BUF_LEN 512

/**
 * Build assertion macro for integer expressions, evaluates to 0
 */
#define BUILD_ASSERT(x) (sizeof(char[(x) ? 0 : -1]))

/**
 * Build time check to assert a is an array, evaluates to 0
 *
 * The address of an array element has a pointer type, which is not compatible
 * to the array type.
 */
#define BUILD_ASSERT_ARRAY(a) \
		BUILD_ASSERT(!__builtin_types_compatible_p(typeof(a), typeof(&(a)[0])))

/**
 * LLVM/Clang __has_feature support
 */
#ifndef __has_feature
# define __has_feature(x) 0
#endif

/**
 * Address sanitizer support
 */
#if __has_feature(address_sanitizer) || \
	(defined(__GNUC__) && defined(__SANITIZE_ADDRESS__))
# define ADDRESS_SANITIZER_EXCLUDE __attribute__((no_sanitize_address))
#else
# define ADDRESS_SANITIZER_EXCLUDE
#endif

/**
 * Debug macro to follow control flow
 */
#define POS printf("%s, line %d\n", __FILE__, __LINE__)

/**
 * This macro allows counting the number of arguments passed to a macro.
 * Combined with the VA_ARGS_DISPATCH() macro this can be used to implement
 * macro overloading based on the number of arguments.
 * 0 to 10 arguments are currently supported.
 */
#define VA_ARGS_NUM(...) _VA_ARGS_NUM(0,##__VA_ARGS__,10,9,8,7,6,5,4,3,2,1,0)
#define _VA_ARGS_NUM(_0,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,NUM,...) NUM

/**
 * This macro can be used to dispatch a macro call based on the number of given
 * arguments, for instance:
 *
 * @code
 * #define MY_MACRO(...) VA_ARGS_DISPATCH(MY_MACRO, __VA_ARGS__)(__VA_ARGS__)
 * #define MY_MACRO1(arg) one_arg(arg)
 * #define MY_MACRO2(arg1,arg2) two_args(arg1,arg2)
 * @endcode
 *
 * MY_MACRO() can now be called with either one or two arguments, which will
 * resolve to one_arg(arg) or two_args(arg1,arg2), respectively.
 */
#define VA_ARGS_DISPATCH(func, ...) _VA_ARGS_DISPATCH(func, VA_ARGS_NUM(__VA_ARGS__))
#define _VA_ARGS_DISPATCH(func, num) __VA_ARGS_DISPATCH(func, num)
#define __VA_ARGS_DISPATCH(func, num) func ## num

/**
 * Assign variadic arguments to the given variables.
 *
 * @note The order and types of the variables are significant and must match the
 * variadic arguments passed to the function that calls this macro exactly.
 *
 * @param last		the last argument before ... in the function that calls this
 * @param ...		variable names
 */
#define VA_ARGS_GET(last, ...) ({ \
	va_list _va_args_get_ap; \
	va_start(_va_args_get_ap, last); \
	_VA_ARGS_GET_ASGN(__VA_ARGS__) \
	va_end(_va_args_get_ap); \
})

/**
 * Assign variadic arguments from a va_list to the given variables.
 *
 * @note The order and types of the variables are significant and must match the
 * variadic arguments passed to the function that calls this macro exactly.
 *
 * @param list		the va_list variable in the function that calls this
 * @param ...		variable names
 */
#define VA_ARGS_VGET(list, ...) ({ \
	va_list _va_args_get_ap; \
	va_copy(_va_args_get_ap, list); \
	_VA_ARGS_GET_ASGN(__VA_ARGS__) \
	va_end(_va_args_get_ap); \
})

#define _VA_ARGS_GET_ASGN(...) VA_ARGS_DISPATCH(_VA_ARGS_GET_ASGN, __VA_ARGS__)(__VA_ARGS__)
#define _VA_ARGS_GET_ASGN1(v1) __VA_ARGS_GET_ASGN(v1)
#define _VA_ARGS_GET_ASGN2(v1,v2) __VA_ARGS_GET_ASGN(v1) __VA_ARGS_GET_ASGN(v2)
#define _VA_ARGS_GET_ASGN3(v1,v2,v3) __VA_ARGS_GET_ASGN(v1) __VA_ARGS_GET_ASGN(v2) \
	__VA_ARGS_GET_ASGN(v3)
#define _VA_ARGS_GET_ASGN4(v1,v2,v3,v4) __VA_ARGS_GET_ASGN(v1) __VA_ARGS_GET_ASGN(v2) \
	__VA_ARGS_GET_ASGN(v3) __VA_ARGS_GET_ASGN(v4)
#define _VA_ARGS_GET_ASGN5(v1,v2,v3,v4,v5) __VA_ARGS_GET_ASGN(v1) __VA_ARGS_GET_ASGN(v2) \
	__VA_ARGS_GET_ASGN(v3) __VA_ARGS_GET_ASGN(v4) __VA_ARGS_GET_ASGN(v5)
#define __VA_ARGS_GET_ASGN(v) v = va_arg(_va_args_get_ap, typeof(v));

/**
 * Macro to allocate a sized type.
 */
#define malloc_thing(thing) ((thing*)malloc(sizeof(thing)))

/**
 * Get the number of elements in an array
 */
#define countof(array) (sizeof(array)/sizeof((array)[0]) \
						+ BUILD_ASSERT_ARRAY(array))

/**
 * Ignore result of functions tagged with warn_unused_result attributes
 */
#define ignore_result(call) do { if(call){} } while(0)

#if !defined(HAVE_SIGWAITINFO) && !defined(WIN32)
/**
 * Block and wait for a set of signals
 *
 * We don't replicate the functionality of siginfo_t.  If info is not NULL
 * -1 is returned and errno is set to EINVAL.
 *
 * @param set		set of signals to wait for
 * @param info		must be NULL
 */
int sigwaitinfo(const sigset_t *set, void *info);
#endif

/**
 * Portable function to wait for SIGINT/SIGTERM (or equivalent).
 */
void wait_sigint();

/**
 * Portable function to send a SIGINT/SIGTERM (or equivalent) to the current
 * process to exit the above function.
 */
void send_sigint();

#ifndef HAVE_CLOSEFROM
/**
 * Close open file descriptors greater than or equal to lowfd.
 *
 * @param lowfd		start closing file descriptors from here
 */
void closefrom(int lowfd);
#endif

/**
 * returns null
 */
void *return_null();

/**
 * No-Operation function
 */
void nop();

/**
 * returns TRUE
 */
bool return_true();

/**
 * returns FALSE
 */
bool return_false();

#endif /** UTILS_H_ @}*/
