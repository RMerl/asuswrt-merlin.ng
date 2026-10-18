/*
 *
 * Copyright (C) 2019-2024, Broadband Forum
 * Copyright (C) 2016-2024  CommScope, Inc
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/**
 * \file common_defs.h
 *
 * Header file containing commonly used macros and definitions
 *
 */
#ifndef COMMON_DEFS_H
#define COMMON_DEFS_H

#include <stdbool.h>    // for bool
#include <stddef.h>     // for NULL
#include <time.h>       // for time_t
#include <limits.h>     // for INT_MAX
#include <string.h>
#include "vendor_defs.h"
#include "usp_err.h"
#include "usp_log.h"
#include "usp_mem.h"

// Number of elements in an array
#define NUM_ELEM(x) (sizeof((x)) / sizeof((x)[0]))

// Minimum of two values
#ifndef MIN
#define MIN(x, y)  ( ((x) <= (y)) ? (x) : (y) )
#endif

// Maximum of two values
#ifndef MAX
#define MAX(x, y)  ( ((x) >= (y)) ? (x) : (y) )
#endif

// Whether a character is an alphanumeric symbol character
#define IS_ALPHA(c)  ( ((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')) )
#define IS_NUMERIC(c)  ((c >= '0') && (c <= '9'))
#define IS_ALPHA_NUMERIC(c)  ( IS_ALPHA(c) || IS_NUMERIC(c) )

// Safe version of snprintf, that ensures buffer is always zero terminated, and does not overrun
extern int USP_SNPRINTF(char *dest, size_t size, const char *fmt, ...) __attribute__((format(printf, 3, 4)));

// Safe version of strncpy, that ensures buffer is always zero terminated, and does not overrun
#define USP_STRNCPY(dest, src, len) strncpy(dest, src, len-1); (dest)[len-1] = '\0';

// Used to make while loops that do not have an outer level exit condition, readable
#define FOREVER 1

// Define to be used with all flags arguments to functions, if no flag in the bitmask needs to be set
#define NO_FLAGS 0

//------------------------------------------------------------------------------
// Common defines for time
#define SECONDS 1000   // 1 second in milliseconds
#define END_OF_TIME   ((time_t)INT_MAX)

//------------------------------------------------------------------------------
// Macro that converts the given pre-processor argument to a string (TO_STR)
#define STRINGIFY(x) #x
#define TO_STR(x) STRINGIFY(x)

//----------------------------------------------------------------
// Macros to read a value from a byte stream buffer (big endian format), but not update the pointer and length of stream left
#define CONVERT_2_BYTES(p) ((p)[0]<<8) + (p)[1];
#define CONVERT_3_BYTES(p)  ((p)[0]<<16) + ((p)[1]<<8) + (p)[2];
#define CONVERT_4_BYTES(p)  ((p)[0]<<24) + ((p)[1]<<16) + ((p)[2]<<8) + (p)[3];

//----------------------------------------------------------------
// Macros to read a value from a byte stream buffer (big endian format), updating the pointer and length of stream left (in input buffer)
#define READ_BYTE(p,l)     (p)[0]; (p)++; (l)--;
#define READ_2_BYTES(p,l)  CONVERT_2_BYTES(p);   (p) += 2;  (l) -= 2;
#define READ_3_BYTES(p,l)  CONVERT_3_BYTES(p);   (p) += 3;  (l) -= 3;
#define READ_4_BYTES(p,l)  CONVERT_4_BYTES(p);   (p) += 4;  (l) -= 4;
#define READ_N_BYTES(dest, src, n, len)  memcpy((dest), (src), (n)); (src) += n; (len) -= n;

//----------------------------------------------------------------------------
// Macros to write a value to a byte stream buffer (big endian format), but not update the pointer
#define STORE_BYTE(buf, v)  buf[0] = (unsigned char)(v & 0xFF);
#define STORE_2_BYTES(buf, v) buf[0] = (unsigned char)((v >> 8) & 0xFF);  buf[1] = (unsigned char)(v & 0xFF);
#define STORE_3_BYTES(buf, v) buf[0] = (unsigned char)((v >> 16) & 0xFF); buf[1] = (unsigned char)((v >> 8) & 0xFF); buf[2] = (unsigned char)(v & 0xFF);
#define STORE_4_BYTES(buf, v) buf[0] = (unsigned char)((v >> 24) & 0xFF); buf[1] = (unsigned char)((v >> 16) & 0xFF); buf[2] = (unsigned char)((v >> 8) & 0xFF); buf[3] = (unsigned char)(v & 0xFF);

//-----------------------------------------------------------------------------------------------
// Macros to write a value to a byte stream buffer (big endian format), updating the pointer
#define WRITE_BYTE(buf, v)  STORE_BYTE(buf, v); buf++;
#define WRITE_2_BYTES(buf, v) STORE_2_BYTES(buf, v); buf += 2;
#define WRITE_3_BYTES(buf, v) STORE_3_BYTES(buf, v); buf += 3;
#define WRITE_4_BYTES(buf, v) STORE_4_BYTES(buf, v); buf += 4;
#define WRITE_N_BYTES(dest, src, n)  memcpy((dest), (src), (n)); (dest) += n;

//----------------------------------------------------------------------------
// Macros to extract bits from an int
#define BIT(m, x)      ((x >> m) & 0x0001)
#define BITS(n, m, x)  ((x >> m) & ((1 <<  (n-m+1)) - 1)) // NOTE: n must be greater than m

//----------------------------------------------------------------------------
// Macros to modify bits in an int
#define MODIFY_BIT(m, x, v)     x = (x & (~ (1 << m))) | (((v) & 1) << m);
#define MODIFY_BITS(n, m, x, v) x = (x & (~ (((1 <<  (n-m+1)) - 1) << m )) ) | (((v)  & ((1 <<  (n-m+1)) - 1)) << m); // NOTE: n must be greater than m

//-----------------------------------------------------------------------------------------------
// Global variables set by command line
extern bool enable_callstack_debug;

#ifndef REMOVE_USP_SERVICE
// String specified using the '-R' option. Contains the data model paths to register
extern char *usp_service_objects;
#define RUNNING_AS_USP_SERVICE() (usp_service_objects != NULL)
#else
#define RUNNING_AS_USP_SERVICE() false
#endif


#endif

