/*
 *
 * Copyright (C) 2020, Broadband Forum
 * Copyright (C) 2012-2020  Axiros GmbH
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
 * \file compiler.h
 *
 * Header file containing defines for function annotation to aid in statical code analysis
 *
 */

#ifndef COMPILER_H
#define COMPILER_H

#ifndef __has_builtin
#define __has_builtin(x) 0
#endif

#ifndef __has_attribute
#define __has_attribute(x) 0
#endif


/************************************************
  Compiler specific annotations for code analysis
 ************************************************/
#if defined(__GNUC__) || __has_attribute(nonnull)
	#define ARGINDEX_NONNULL(index) __attribute__ ((nonnull(index)))
	#define ARGS_NONNULL __attribute__ ((nonnull))
#else
	#define ARGINDEX_NONNULL(index)
	#define ARGS_NONNULL
#endif

#if __has_attribute(returns_nonnull)
	#define RETURNS_NONNULL __attribute__ ((returns_nonnull))
#else
	#define RETURNS_NONNULL
#endif

#if defined(__GNUC__) || __has_attribute(__format__)
	#define ARGS_FORMAT_PRINTF(fmt,args) __attribute__((__format__ (__printf__, fmt, args)))
#else
	#define ARGS_FORMAT_PRINTF(fmt,args)
#endif

#if defined(__GNUC__) || __has_attribute(malloc)
	#define MALLOC __attribute__ ((malloc))
#else
	#define MALLOC
#endif

#endif /* COMPILER_H */
