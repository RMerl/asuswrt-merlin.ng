/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2012 The Chromium OS Authors.
 */

#ifndef __TRACE_H
#define __TRACE_H

enum {
	/*
	 * This affects the granularity of our trace. We can bin function
	 * entry points into groups on the basis that functions typically
	 * have a minimum size, so entry points can't appear any closer
	 * than this to each other.
	 *
	 * The value here assumes a minimum instruction size of 4 bytes,
	 * or that instructions are 2 bytes but there are at least 2 of
	 * them in every function.
	 *
	 * Increasing this value reduces the number of functions we can
	 * resolve, but reduces the size of the uintptr_t array used for
	 * our function list, which is the length of the code divided by
	 * this value.
	 */
	FUNC_SITE_SIZE	= 4,	/* distance between function sites */
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
	uint32_t rec_count;		/* Number of records */
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
 * @return 0 if ok, -1 on error (buffer exhausted)
 */
int trace_list_functions(void *buff, int buff_size, unsigned *needed);

/* Flags for ftrace_record */
enum ftrace_flags {
	FUNCF_EXIT		= 0UL << 30,
	FUNCF_ENTRY		= 1UL << 30,
	FUNCF_TEXTBASE		= 2UL << 30,

	FUNCF_TIMESTAMP_MASK	= 0x3fffffff,
};

#define TRACE_CALL_TYPE(call)	((call)->flags & 0xc0000000UL)

/* Information about a single function entry/exit */
struct trace_call {
	uint32_t func;		/* Function offset */
	uint32_t caller;	/* Caller function offset */
	uint32_t flags;		/* Flags and timestamp */
};

int trace_list_calls(void *buff, int buff_size, unsigned int *needed);

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
