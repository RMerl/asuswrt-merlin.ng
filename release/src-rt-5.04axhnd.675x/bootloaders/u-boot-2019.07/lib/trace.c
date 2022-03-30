// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2012 The Chromium OS Authors.
 */

#include <common.h>
#include <mapmem.h>
#include <trace.h>
#include <asm/io.h>
#include <asm/sections.h>

DECLARE_GLOBAL_DATA_PTR;

static char trace_enabled __attribute__((section(".data")));
static char trace_inited __attribute__((section(".data")));

/* The header block at the start of the trace memory area */
struct trace_hdr {
	int func_count;		/* Total number of function call sites */
	u64 call_count;		/* Total number of tracked function calls */
	u64 untracked_count;	/* Total number of untracked function calls */
	int funcs_used;		/* Total number of functions used */

	/*
	 * Call count for each function. This is indexed by the word offset
	 * of the function from gd->relocaddr
	 */
	uintptr_t *call_accum;

	/* Function trace list */
	struct trace_call *ftrace;	/* The function call records */
	ulong ftrace_size;	/* Num. of ftrace records we have space for */
	ulong ftrace_count;	/* Num. of ftrace records written */
	ulong ftrace_too_deep_count;	/* Functions that were too deep */

	int depth;
	int depth_limit;
	int max_depth;
};

static struct trace_hdr *hdr;	/* Pointer to start of trace buffer */

static inline uintptr_t __attribute__((no_instrument_function))
		func_ptr_to_num(void *func_ptr)
{
	uintptr_t offset = (uintptr_t)func_ptr;

#ifdef CONFIG_SANDBOX
	offset -= (uintptr_t)&_init;
#else
	if (gd->flags & GD_FLG_RELOC)
		offset -= gd->relocaddr;
	else
		offset -= CONFIG_SYS_TEXT_BASE;
#endif
	return offset / FUNC_SITE_SIZE;
}

static void __attribute__((no_instrument_function)) add_ftrace(void *func_ptr,
				void *caller, ulong flags)
{
	if (hdr->depth > hdr->depth_limit) {
		hdr->ftrace_too_deep_count++;
		return;
	}
	if (hdr->ftrace_count < hdr->ftrace_size) {
		struct trace_call *rec = &hdr->ftrace[hdr->ftrace_count];

		rec->func = func_ptr_to_num(func_ptr);
		rec->caller = func_ptr_to_num(caller);
		rec->flags = flags | (timer_get_us() & FUNCF_TIMESTAMP_MASK);
	}
	hdr->ftrace_count++;
}

static void __attribute__((no_instrument_function)) add_textbase(void)
{
	if (hdr->ftrace_count < hdr->ftrace_size) {
		struct trace_call *rec = &hdr->ftrace[hdr->ftrace_count];

		rec->func = CONFIG_SYS_TEXT_BASE;
		rec->caller = 0;
		rec->flags = FUNCF_TEXTBASE;
	}
	hdr->ftrace_count++;
}

/**
 * This is called on every function entry
 *
 * We add to our tally for this function and add to the list of called
 * functions.
 *
 * @param func_ptr	Pointer to function being entered
 * @param caller	Pointer to function which called this function
 */
void __attribute__((no_instrument_function)) __cyg_profile_func_enter(
		void *func_ptr, void *caller)
{
	if (trace_enabled) {
		int func;

		add_ftrace(func_ptr, caller, FUNCF_ENTRY);
		func = func_ptr_to_num(func_ptr);
		if (func < hdr->func_count) {
			hdr->call_accum[func]++;
			hdr->call_count++;
		} else {
			hdr->untracked_count++;
		}
		hdr->depth++;
		if (hdr->depth > hdr->depth_limit)
			hdr->max_depth = hdr->depth;
	}
}

/**
 * This is called on every function exit
 *
 * We do nothing here.
 *
 * @param func_ptr	Pointer to function being entered
 * @param caller	Pointer to function which called this function
 */
void __attribute__((no_instrument_function)) __cyg_profile_func_exit(
		void *func_ptr, void *caller)
{
	if (trace_enabled) {
		add_ftrace(func_ptr, caller, FUNCF_EXIT);
		hdr->depth--;
	}
}

/**
 * Produce a list of called functions
 *
 * The information is written into the supplied buffer - a header followed
 * by a list of function records.
 *
 * @param buff		Buffer to place list into
 * @param buff_size	Size of buffer
 * @param needed	Returns size of buffer needed, which may be
 *			greater than buff_size if we ran out of space.
 * @return 0 if ok, -1 if space was exhausted
 */
int trace_list_functions(void *buff, int buff_size, unsigned int *needed)
{
	struct trace_output_hdr *output_hdr = NULL;
	void *end, *ptr = buff;
	int func;
	int upto;

	end = buff ? buff + buff_size : NULL;

	/* Place some header information */
	if (ptr + sizeof(struct trace_output_hdr) < end)
		output_hdr = ptr;
	ptr += sizeof(struct trace_output_hdr);

	/* Add information about each function */
	for (func = upto = 0; func < hdr->func_count; func++) {
		int calls = hdr->call_accum[func];

		if (!calls)
			continue;

		if (ptr + sizeof(struct trace_output_func) < end) {
			struct trace_output_func *stats = ptr;

			stats->offset = func * FUNC_SITE_SIZE;
			stats->call_count = calls;
			upto++;
		}
		ptr += sizeof(struct trace_output_func);
	}

	/* Update the header */
	if (output_hdr) {
		output_hdr->rec_count = upto;
		output_hdr->type = TRACE_CHUNK_FUNCS;
	}

	/* Work out how must of the buffer we used */
	*needed = ptr - buff;
	if (ptr > end)
		return -ENOSPC;

	return 0;
}

int trace_list_calls(void *buff, int buff_size, unsigned *needed)
{
	struct trace_output_hdr *output_hdr = NULL;
	void *end, *ptr = buff;
	int rec, upto;
	int count;

	end = buff ? buff + buff_size : NULL;

	/* Place some header information */
	if (ptr + sizeof(struct trace_output_hdr) < end)
		output_hdr = ptr;
	ptr += sizeof(struct trace_output_hdr);

	/* Add information about each call */
	count = hdr->ftrace_count;
	if (count > hdr->ftrace_size)
		count = hdr->ftrace_size;
	for (rec = upto = 0; rec < count; rec++) {
		if (ptr + sizeof(struct trace_call) < end) {
			struct trace_call *call = &hdr->ftrace[rec];
			struct trace_call *out = ptr;

			out->func = call->func * FUNC_SITE_SIZE;
			out->caller = call->caller * FUNC_SITE_SIZE;
			out->flags = call->flags;
			upto++;
		}
		ptr += sizeof(struct trace_call);
	}

	/* Update the header */
	if (output_hdr) {
		output_hdr->rec_count = upto;
		output_hdr->type = TRACE_CHUNK_CALLS;
	}

	/* Work out how must of the buffer we used */
	*needed = ptr - buff;
	if (ptr > end)
		return -ENOSPC;

	return 0;
}

/* Print basic information about tracing */
void trace_print_stats(void)
{
	ulong count;

#ifndef FTRACE
	puts("Warning: make U-Boot with FTRACE to enable function instrumenting.\n");
	puts("You will likely get zeroed data here\n");
#endif
	if (!trace_inited) {
		printf("Trace is disabled\n");
		return;
	}
	print_grouped_ull(hdr->func_count, 10);
	puts(" function sites\n");
	print_grouped_ull(hdr->call_count, 10);
	puts(" function calls\n");
	print_grouped_ull(hdr->untracked_count, 10);
	puts(" untracked function calls\n");
	count = min(hdr->ftrace_count, hdr->ftrace_size);
	print_grouped_ull(count, 10);
	puts(" traced function calls");
	if (hdr->ftrace_count > hdr->ftrace_size) {
		printf(" (%lu dropped due to overflow)",
		       hdr->ftrace_count - hdr->ftrace_size);
	}
	puts("\n");
	printf("%15d maximum observed call depth\n", hdr->max_depth);
	printf("%15d call depth limit\n", hdr->depth_limit);
	print_grouped_ull(hdr->ftrace_too_deep_count, 10);
	puts(" calls not traced due to depth\n");
}

void __attribute__((no_instrument_function)) trace_set_enabled(int enabled)
{
	trace_enabled = enabled != 0;
}

/**
 * Init the tracing system ready for used, and enable it
 *
 * @param buff		Pointer to trace buffer
 * @param buff_size	Size of trace buffer
 */
int __attribute__((no_instrument_function)) trace_init(void *buff,
		size_t buff_size)
{
	ulong func_count = gd->mon_len / FUNC_SITE_SIZE;
	size_t needed;
	int was_disabled = !trace_enabled;

	if (!was_disabled) {
#ifdef CONFIG_TRACE_EARLY
		char *end;
		ulong used;

		/*
		 * Copy over the early trace data if we have it. Disable
		 * tracing while we are doing this.
		 */
		trace_enabled = 0;
		hdr = map_sysmem(CONFIG_TRACE_EARLY_ADDR,
				 CONFIG_TRACE_EARLY_SIZE);
		end = (char *)&hdr->ftrace[min(hdr->ftrace_count,
					       hdr->ftrace_size)];
		used = end - (char *)hdr;
		printf("trace: copying %08lx bytes of early data from %x to %08lx\n",
		       used, CONFIG_TRACE_EARLY_ADDR,
		       (ulong)map_to_sysmem(buff));
		memcpy(buff, hdr, used);
#else
		puts("trace: already enabled\n");
		return -EALREADY;
#endif
	}
	hdr = (struct trace_hdr *)buff;
	needed = sizeof(*hdr) + func_count * sizeof(uintptr_t);
	if (needed > buff_size) {
		printf("trace: buffer size %zd bytes: at least %zd needed\n",
		       buff_size, needed);
		return -ENOSPC;
	}

	if (was_disabled)
		memset(hdr, '\0', needed);
	hdr->func_count = func_count;
	hdr->call_accum = (uintptr_t *)(hdr + 1);

	/* Use any remaining space for the timed function trace */
	hdr->ftrace = (struct trace_call *)(buff + needed);
	hdr->ftrace_size = (buff_size - needed) / sizeof(*hdr->ftrace);
	add_textbase();

	puts("trace: enabled\n");
	hdr->depth_limit = 15;
	trace_enabled = 1;
	trace_inited = 1;

	return 0;
}

#ifdef CONFIG_TRACE_EARLY
int __attribute__((no_instrument_function)) trace_early_init(void)
{
	ulong func_count = gd->mon_len / FUNC_SITE_SIZE;
	size_t buff_size = CONFIG_TRACE_EARLY_SIZE;
	size_t needed;

	/* We can ignore additional calls to this function */
	if (trace_enabled)
		return 0;

	hdr = map_sysmem(CONFIG_TRACE_EARLY_ADDR, CONFIG_TRACE_EARLY_SIZE);
	needed = sizeof(*hdr) + func_count * sizeof(uintptr_t);
	if (needed > buff_size) {
		printf("trace: buffer size is %zd bytes, at least %zd needed\n",
		       buff_size, needed);
		return -ENOSPC;
	}

	memset(hdr, '\0', needed);
	hdr->call_accum = (uintptr_t *)(hdr + 1);
	hdr->func_count = func_count;

	/* Use any remaining space for the timed function trace */
	hdr->ftrace = (struct trace_call *)((char *)hdr + needed);
	hdr->ftrace_size = (buff_size - needed) / sizeof(*hdr->ftrace);
	add_textbase();
	hdr->depth_limit = 200;
	printf("trace: early enable at %08x\n", CONFIG_TRACE_EARLY_ADDR);

	trace_enabled = 1;

	return 0;
}
#endif
