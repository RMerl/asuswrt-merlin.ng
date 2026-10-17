/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2012 The Chromium OS Authors.
 */

#ifndef __TRACE_H
#define __TRACE_H

/* this file is included from a tool so uses uint32_t instead of u32, etc. */

enum {
	/*
	 * This affects the granularity of our trace. We can bin function
	 * entry points into groups on the basis that functions typically
	 * have a minimum size, so entry points can't appear any closer
	 * than this to each other.
	 *
	 * The value here assumes a minimum instruction size of 4 bytes,
	 * or that instructions are 2 bytes but there are at least 2 of
	 * them in every function. Given that each function needs a call to
	 * __cyg_profile_func_enter() and __cyg_profile_func_exit() as well,
	 * we cannot have functions smaller that 16 bytes.
	 *
	 * Increasing this value reduces the number of functions we can
	 * resolve, but reduces the size of the uintptr_t array used for
	 * our function list, which is the length of the code divided by
	 * this value.
	 */
	FUNC_SITE_SIZE	= 16,	/* distance between function sites */

	TRACE_VERSION	= 1,
};

enum trace_chunk_type {
	TRACE_CHUNK_FUNCS,
	TRACE_CHUNK_CALLS,
};

/* A trace record for a function, as written to the profile output file */
struct trace_output_func {
	uint32_t offset;		/* Function offset into code */
	uint32_t call_count;		/* Number of times called */
};

/* A header at the start of the trace output buffer */
struct trace_output_hdr {
	enum trace_chunk_type type;	/* Record type */
	uint32_t version;		/* Version (TRACE_VERSION) */
	uint32_t rec_count;		/* Number of records */
	uint32_t spare;			/* 0 */
	uint64_t text_base;		/* Value of CONFIG_TEXT_BASE */
	uint64_t spare2;		/* 0 */
};

/* Print statistics about traced function calls */
void trace_print_stats(void);

/**
 * Dump a list of functions and call counts into a buffer
 *
 * Each record in the buffer is a struct trace_func_stats. The 'needed'
 * parameter returns the number of bytes needed to complete the operation,
 * which may be more than buff_size if your buffer is too small.
 *
 * @param buff		Buffer in which to place data, or NULL to count size
 * @param buff_size	Size of buffer
 * @param needed	Returns number of bytes used / needed
 * Return: 0 if ok, -1 on error (buffer exhausted)
 */
int trace_list_functions(void *buff, size_t buff_size, size_t *needed);

/* Flags for ftrace_record */
enum ftrace_flags {
	FUNCF_EXIT		= 0UL << 30,
	FUNCF_ENTRY		= 1UL << 30,
	/* two more values are available */

	FUNCF_TIMESTAMP_MASK	= 0x3fffffff,
};

#define TRACE_CALL_TYPE(call)	((call)->flags & 0xc0000000UL)

/* Information about a single function entry/exit */
struct trace_call {
	uint32_t func;		/* Function offset */
	uint32_t caller;	/* Caller function offset */
	uint32_t flags;		/* Flags and timestamp */
};

int trace_list_calls(void *buff, size_t buff_size, size_t *needed);

/**
 * Turn function tracing on and off
 *
 * Don't enable trace if it has not been initialised.
 *
 * @param enabled	1 to enable trace, 0 to disable
 */
void trace_set_enabled(int enabled);

int trace_early_init(void);

/**
 * Init the trace system
 *
 * This should be called after relocation with a suitably large buffer
 * (typically as large as the U-Boot text area)
 *
 * @param buff		Pointer to trace buffer
 * @param buff_size	Size of trace buffer
 */
int trace_init(void *buff, size_t buff_size);

#endif
