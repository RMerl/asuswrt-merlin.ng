// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2023 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

/*
 * Decode and dump U-Boot trace information into formats that can be used
 * by trace-cmd, kernelshark or flamegraph.pl
 *
 * See doc/develop/trace.rst for more information
 */

#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <regex.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/types.h>

#include <compiler.h>
#include <trace.h>
#include <abuf.h>

#include <linux/list.h>

/* Set to 1 to emit version 7 file (currently this doesn't work) */
#define VERSION7	0

/* enable some debug features */
#define _DEBUG	0

/* from linux/kernel.h */
#define __ALIGN_MASK(x, mask)	(((x) + (mask)) & ~(mask))
#define ALIGN(x, a)		__ALIGN_MASK((x), (typeof(x))(a) - 1)

/**
 * container_of - cast a member of a structure out to the containing structure
 * @ptr:	the pointer to the member.
 * @type:	the type of the container struct this is embedded in.
 * @member:	the name of the member within the struct.
 *
 * (this is needed by list.h)
 */
#define container_of(ptr, type, member) ({			\
	const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
	(type *)( (char *)__mptr - offsetof(type,member) );})

enum {
	FUNCF_TRACE	= 1 << 0,	/* Include this function in trace */
	TRACE_PAGE_SIZE	= 4096,		/* Assumed page size for trace */
	TRACE_PID	= 1,		/* PID to use for U-Boot */
	LEN_STACK_SIZE	= 4,		/* number of nested length fix-ups */
	TRACE_PAGE_MASK	= TRACE_PAGE_SIZE - 1,
	MAX_STACK_DEPTH	= 50,		/* Max nested function calls */
	MAX_LINE_LEN	= 500,		/* Max characters per line */
};

/**
 * enum out_format_t - supported output formats
 *
 * @OUT_FMT_DEFAULT: Use the default for the output file
 * @OUT_FMT_FUNCTION: Write ftrace 'function' records
 * @OUT_FMT_FUNCGRAPH: Write ftrace funcgraph_entry and funcgraph_exit records
 * @OUT_FMT_FLAMEGRAPH_CALLS: Write a file suitable for flamegraph.pl
 * @OUT_FMT_FLAMEGRAPH_TIMING: Write a file suitable for flamegraph.pl with the
 * counts set to the number of microseconds used by each function
 */
enum out_format_t {
	OUT_FMT_DEFAULT,
	OUT_FMT_FUNCTION,
	OUT_FMT_FUNCGRAPH,
	OUT_FMT_FLAMEGRAPH_CALLS,
	OUT_FMT_FLAMEGRAPH_TIMING,
};

/* Section types for v7 format (trace-cmd format) */
enum {
	SECTION_OPTIONS,
};

/* Option types (trace-cmd format) */
enum {
	OPTION_DONE,
	OPTION_DATE,
	OPTION_CPUSTAT,
	OPTION_BUFFER,
	OPTION_TRACECLOCK,
	OPTION_UNAME,
	OPTION_HOOK,
	OPTION_OFFSET,
	OPTION_CPUCOUNT,
	OPTION_VERSION,
	OPTION_PROCMAPS,
	OPTION_TRACEID,
	OPTION_TIME_SHIFT,
	OPTION_GUEST,
	OPTION_TSC2NSEC,
};

/* types of trace records (trace-cmd format) */
enum trace_type {
	__TRACE_FIRST_TYPE = 0,

	TRACE_FN,
	TRACE_CTX,
	TRACE_WAKE,
	TRACE_STACK,
	TRACE_PRINT,
	TRACE_BPRINT,
	TRACE_MMIO_RW,
	TRACE_MMIO_MAP,
	TRACE_BRANCH,
	TRACE_GRAPH_RET,
	TRACE_GRAPH_ENT,
};

/**
 * struct flame_node - a node in the call-stack tree
 *
 * Each stack frame detected in the trace is given a node corresponding to a
 * function call in the call stack. Functions can appear multiple times when
 * they are called by a different set of parent functions.
 *
 * @parent: Parent node (the call stack for the function that called this one)
 * @child_head: List of children of this node (functions called from here)
 * @sibling: Next node in the list of children
 * @func: Function this node refers to (NULL for root node)
 * @count: Number of times this call-stack occurred
 * @duration: Number of microseconds taken to run this function, excluding all
 * of the functions it calls
 */
struct flame_node {
	struct flame_node *parent;
	struct list_head child_head;
	struct list_head sibling_node;
	struct func_info *func;
	int count;
	ulong duration;
};

/**
 * struct flame_state - state information for building the flame graph
 *
 * @node: Current node being processed (corresponds to a function call)
 * @stack: Stack of call-start time for this function as well as the
 * accumulated total time of all child calls (so we can subtract them from the
 * function's call time. This is an 'empty' stack, meaning that @stack_ptr
 * points to the next available stack position
 * @stack_ptr: points to first empty position in the stack
 * @nodes: Number of nodes created (running count)
 */
struct flame_state {
	struct flame_node *node;
	struct stack_info {
		ulong timestamp;
		ulong child_total;
	} stack[MAX_STACK_DEPTH];
	int stack_ptr;
	int nodes;
};

/**
 * struct func_info - information recorded for each function
 *
 * @offset: Function offset in the image, measured from the text_base
 * @name: Function name
 * @code_size: Total code size of the function
 * @flags: Either 0 or FUNCF_TRACE
 */
struct func_info {
	unsigned long offset;
	const char *name;
	unsigned long code_size;
	unsigned flags;
};

/**
 * enum trace_line_type - whether to include or exclude a function
 *
 * @TRACE_LINE_INCLUDE: Include the function
 * @TRACE_LINE_EXCLUDE: Exclude the function
 */
enum trace_line_type {
	TRACE_LINE_INCLUDE,
	TRACE_LINE_EXCLUDE,
};

/**
 * struct trace_configline_info - information about a config-file line
 *
 * @next: Next line
 * @type: Line type
 * @name: identifier name / wildcard
 * @regex: Regex to use if name starts with '/'
 */
struct trace_configline_info {
	struct trace_configline_info *next;
	enum trace_line_type type;
	const char *name;
	regex_t regex;
};

/**
 * struct tw_len - holds information about a length value that need fix-ups
 *
 * This is used to record a placeholder for a u32 or u64 length which is written
 * to the output file but needs to be updated once the length is actually known
 *
 * This allows us to write tw->ptr - @len_base to position @ptr in the file
 *
 * @ptr: Position of the length value in the file
 * @base: Base position for the calculation
 * @size: Size of the length value, in bytes (4 or 8)
 */
struct tw_len {
	int ptr;
	int base;
	int size;
};

/**
 * struct twriter - Writer for trace records
 *
 * Maintains state used when writing the output file in trace-cmd format
 *
 * @ptr: Current file position
 * @len_stack: Stack of length values that need fixing up
 * @len: Number of items on @len_stack
 * @str_buf: Buffer of strings (for v7 format)
 * @str_ptr: Current write-position in the buffer for strings
 * @fout: Output file
 */
struct twriter {
	int ptr;
	struct tw_len len_stack[LEN_STACK_SIZE];
	int len_count;
	struct abuf str_buf;
	int str_ptr;
	FILE *fout;
};

/* The contents of the trace config file */
struct trace_configline_info *trace_config_head;

/* list of all functions in System.map file, sorted by offset in the image */
struct func_info *func_list;

int func_count;			/* number of functions */
struct trace_call *call_list;	/* list of all calls in the input trace file */
int call_count;			/* number of calls */
int verbose;	/* Verbosity level 0=none, 1=warn, 2=notice, 3=info, 4=debug */
ulong text_offset;		/* text address of first function */
ulong text_base;		/* CONFIG_TEXT_BASE from trace file */

/* debugging helpers */
static void outf(int level, const char *fmt, ...)
		__attribute__ ((format (__printf__, 2, 3)));
#define error(fmt, b...) outf(0, fmt, ##b)
#define warn(fmt, b...) outf(1, fmt, ##b)
#define notice(fmt, b...) outf(2, fmt, ##b)
#define info(fmt, b...) outf(3, fmt, ##b)
#define debug(fmt, b...) outf(4, fmt, ##b)

static void outf(int level, const char *fmt, ...)
{
	if (verbose >= level) {
		va_list args;

		va_start(args, fmt);
		vfprintf(stderr, fmt, args);
		va_end(args);
	}
}

static void usage(void)
{
	fprintf(stderr,
		"Usage: proftool [-cmtv] <cmd> <profdata>\n"
		"\n"
		"Commands\n"
		"   dump-ftrace\t\tDump out records in ftrace format for use by trace-cmd\n"
		"   dump-flamegraph\tWrite a file for use with flamegraph.pl\n"
		"\n"
		"Options:\n"
		"   -c <cfg>\tSpecify config file\n"
		"   -f <subtype>\tSpecify output subtype\n"
		"   -m <map>\tSpecify Systen.map file\n"
		"   -o <fname>\tSpecify output file\n"
		"   -t <fname>\tSpecify trace data file (from U-Boot 'trace calls')\n"
		"   -v <0-4>\tSpecify verbosity\n"
		"\n"
		"Subtypes for dump-ftrace:\n"
		"   function - write function-call records (caller/callee)\n"
		"   funcgraph - write function entry/exit records (graph)\n"
		"\n"
		"Subtypes for dump-flamegraph\n"
		"   calls - create a flamegraph of stack frames\n"
		"   timing - create a flamegraph of microseconds for each stack frame\n");
	exit(EXIT_FAILURE);
}

/**
 * h_cmp_offset - bsearch() function to compare two functions bny their offset
 *
 * @v1: Pointer to first function (struct func_info)
 * @v2: Pointer to second function (struct func_info)
 * Returns: < 0 if v1 offset < v2 offset, 0 if equal, > 0 otherwise
 */
static int h_cmp_offset(const void *v1, const void *v2)
{
	const struct func_info *f1 = v1, *f2 = v2;

	return (f1->offset / FUNC_SITE_SIZE) - (f2->offset / FUNC_SITE_SIZE);
}

/**
 * read_system_map() - read the System.map file to create a list of functions
 *
 * This also reads the text_offset value, since we assume that the first text
 * symbol is at that address
 *
 * @fin: File to read
 * Returns: 0 if OK, non-zero on error
 */
static int read_system_map(FILE *fin)
{
	unsigned long offset, start = 0;
	struct func_info *func;
	char buff[MAX_LINE_LEN];
	char symtype;
	char symname[MAX_LINE_LEN + 1];
	int linenum;
	int alloced;

	for (linenum = 1, alloced = func_count = 0;; linenum++) {
		int fields = 0;

		if (fgets(buff, sizeof(buff), fin))
			fields = sscanf(buff, "%lx %c %100s\n", &offset,
				&symtype, symname);
		if (fields == 2) {
			continue;
		} else if (feof(fin)) {
			break;
		} else if (fields < 2) {
			error("Map file line %d: invalid format\n", linenum);
			return 1;
		}

		/* Must be a text symbol */
		symtype = tolower(symtype);
		if (symtype != 't' && symtype != 'w')
			continue;

		if (func_count == alloced) {
			alloced += 256;
			func_list = realloc(func_list,
					sizeof(struct func_info) * alloced);
			assert(func_list);
		}
		if (!func_count)
			start = offset;

		func = &func_list[func_count++];
		memset(func, '\0', sizeof(*func));
		func->offset = offset - start;
		func->name = strdup(symname);
		func->flags = FUNCF_TRACE;	/* trace by default */

		/* Update previous function's code size */
		if (func_count > 1)
			func[-1].code_size = func->offset - func[-1].offset;
	}
	notice("%d functions found in map file, start addr %lx\n", func_count,
	       start);
	text_offset = start;

	return 0;
}

static int read_data(FILE *fin, void *buff, int size)
{
	int err;

	err = fread(buff, 1, size, fin);
	if (!err)
		return 1;
	if (err != size) {
		error("Cannot read trace file at pos %lx\n", ftell(fin));
		return -1;
	}
	return 0;
}

/**
 * find_func_by_offset() - Look up a function by its offset
 *
 * @offset: Offset to search for, from text_base
 * Returns: function, if found, else NULL
 *
 * This does a fast search for a function given its offset from text_base
 *
 */
static struct func_info *find_func_by_offset(uint offset)
{
	struct func_info key, *found;

	key.offset = offset;
	found = bsearch(&key, func_list, func_count, sizeof(struct func_info),
			h_cmp_offset);

	return found;
}

/**
 * find_caller_by_offset() - finds the function which contains the given offset
 *
 * @offset: Offset to search for, from text_base
 * Returns: function, if found, else NULL
 *
 * If the offset falls between two functions, then it is assumed to belong to
 * the first function (with the lowest offset). This is a way of figuring out
 * which function owns code at a particular offset
 */
static struct func_info *find_caller_by_offset(uint offset)
{
	int low;	/* least function that could be a match */
	int high;	/* greated function that could be a match */
	struct func_info key;

	low = 0;
	high = func_count - 1;
	key.offset = offset;
	while (high > low + 1) {
		int mid = (low + high) / 2;
		int result;

		result = h_cmp_offset(&key, &func_list[mid]);
		if (result > 0)
			low = mid;
		else if (result < 0)
			high = mid;
		else
			return &func_list[mid];
	}

	return low >= 0 ? &func_list[low] : NULL;
}

/**
 * read_calls() - Read the list of calls from the trace data
 *
 * The calls are stored consecutively in the trace output produced by U-Boot
 *
 * @fin: File to read from
 * @count: Number of calls to read
 * Returns: 0 if OK, -1 on error
 */
static int read_calls(FILE *fin, size_t count)
{
	struct trace_call *call_data;
	int i;

	notice("call count: %zu\n", count);
	call_list = (struct trace_call *)calloc(count, sizeof(*call_data));
	if (!call_list) {
		error("Cannot allocate call_list\n");
		return -1;
	}
	call_count = count;

	call_data = call_list;
	for (i = 0; i < count; i++, call_data++) {
		if (read_data(fin, call_data, sizeof(*call_data)))
			return -1;
	}
	return 0;
}

/**
 * read_trace() - Read the U-Boot trace file
 *
 * Read in the calls from the trace file. The function list is ignored at
 * present
 *
 * @fin: File to read
 * Returns 0 if OK, non-zero on error
 */
static int read_trace(FILE *fin)
{
	struct trace_output_hdr hdr;

	while (!feof(fin)) {
		int err;

		err = read_data(fin, &hdr, sizeof(hdr));
		if (err == 1)
			break; /* EOF */
		else if (err)
			return 1;
		text_base = hdr.text_base;

		switch (hdr.type) {
		case TRACE_CHUNK_FUNCS:
			/* Ignored at present */
			break;

		case TRACE_CHUNK_CALLS:
			if (read_calls(fin, hdr.rec_count))
				return 1;
			break;
		}
	}
	return 0;
}

/**
 * read_map_file() - Read the System.map file
 *
 * This reads the file into the func_list array
 *
 * @fname: Filename to read
 * Returns 0 if OK, non-zero on error
 */
static int read_map_file(const char *fname)
{
	FILE *fmap;
	int err = 0;

	fmap = fopen(fname, "r");
	if (!fmap) {
		error("Cannot open map file '%s'\n", fname);
		return 1;
	}
	if (fmap) {
		err = read_system_map(fmap);
		fclose(fmap);
	}
	return err;
}

/**
 * read_trace_file() - Open and read the U-Boot trace file
 *
 * Read in the calls from the trace file. The function list is ignored at
 * present
 *
 * @fin: File to read
 * Returns 0 if OK, non-zero on error
 */
static int read_trace_file(const char *fname)
{
	FILE *fprof;
	int err;

	fprof = fopen(fname, "rb");
	if (!fprof) {
		error("Cannot open trace data file '%s'\n",
		      fname);
		return 1;
	} else {
		err = read_trace(fprof);
		fclose(fprof);
		if (err)
			return err;
	}
	return 0;
}

static int regex_report_error(regex_t *regex, int err, const char *op,
			      const char *name)
{
	char buf[200];

	regerror(err, regex, buf, sizeof(buf));
	error("Regex error '%s' in %s '%s'\n", buf, op, name);
	return -1;
}

static void check_trace_config_line(struct trace_configline_info *item)
{
	struct func_info *func, *end;
	int err;

	debug("Checking trace config line '%s'\n", item->name);
	for (func = func_list, end = func + func_count; func < end; func++) {
		err = regexec(&item->regex, func->name, 0, NULL, 0);
		debug("   - regex '%s', string '%s': %d\n", item->name,
		      func->name, err);
		if (err == REG_NOMATCH)
			continue;

		if (err) {
			regex_report_error(&item->regex, err, "match",
					   item->name);
			break;
		}

		/* It matches, so perform the action */
		switch (item->type) {
		case TRACE_LINE_INCLUDE:
			info("      include %s at %lx\n", func->name,
			     text_offset + func->offset);
			func->flags |= FUNCF_TRACE;
			break;

		case TRACE_LINE_EXCLUDE:
			info("      exclude %s at %lx\n", func->name,
			     text_offset + func->offset);
			func->flags &= ~FUNCF_TRACE;
			break;
		}
	}
}

/** check_trace_config() - Check trace-config file, reporting any problems */
static void check_trace_config(void)
{
	struct trace_configline_info *line;

	for (line = trace_config_head; line; line = line->next)
		check_trace_config_line(line);
}

/**
 * read_trace_config() - read the trace-config file
 *
 * This file consists of lines like:
 *
 * include-func <regex>
 * exclude-func <regex>
 *
 * where <regex> is a regular expression matched against function names. It
 * allows some functions to be dropped from the trace when producing ftrace
 * records
 *
 * @fin: File to process
 * Returns: 0 if OK, -1 on error
 */
static int read_trace_config(FILE *fin)
{
	char buff[200];
	int linenum = 0;
	struct trace_configline_info **tailp = &trace_config_head;

	while (fgets(buff, sizeof(buff), fin)) {
		int len = strlen(buff);
		struct trace_configline_info *line;
		char *saveptr;
		char *s, *tok;
		int err;

		linenum++;
		if (len && buff[len - 1] == '\n')
			buff[len - 1] = '\0';

		/* skip blank lines and comments */
		for (s = buff; *s == ' ' || *s == '\t'; s++)
			;
		if (!*s || *s == '#')
			continue;

		line = (struct trace_configline_info *)calloc(1, sizeof(*line));
		if (!line) {
			error("Cannot allocate config line\n");
			return -1;
		}

		tok = strtok_r(s, " \t", &saveptr);
		if (!tok) {
			error("Invalid trace config data on line %d\n",
			      linenum);
			return -1;
		}
		if (0 == strcmp(tok, "include-func")) {
			line->type = TRACE_LINE_INCLUDE;
		} else if (0 == strcmp(tok, "exclude-func")) {
			line->type = TRACE_LINE_EXCLUDE;
		} else {
			error("Unknown command in trace config data line %d\n",
			      linenum);
			return -1;
		}

		tok = strtok_r(NULL, " \t", &saveptr);
		if (!tok) {
			error("Missing pattern in trace config data line %d\n",
			      linenum);
			return -1;
		}

		err = regcomp(&line->regex, tok, REG_NOSUB);
		if (err) {
			int r = regex_report_error(&line->regex, err,
						   "compile", tok);
			free(line);
			return r;
		}

		/* link this new one to the end of the list */
		line->name = strdup(tok);
		line->next = NULL;
		*tailp = line;
		tailp = &line->next;
	}

	if (!feof(fin)) {
		error("Cannot read from trace config file at position %ld\n",
		      ftell(fin));
		return -1;
	}
	return 0;
}

static int read_trace_config_file(const char *fname)
{
	FILE *fin;
	int err;

	fin = fopen(fname, "r");
	if (!fin) {
		error("Cannot open trace_config file '%s'\n", fname);
		return -1;
	}
	err = read_trace_config(fin);
	fclose(fin);
	return err;
}

/**
 * tputh() - Write a 16-bit little-endian value to a file
 *
 * @fout: File to write to
 * @val: Value to write
 * Returns: number of bytes written (2)
 */
static int tputh(FILE *fout, unsigned int val)
{
	fputc(val, fout);
	fputc(val >> 8, fout);

	return 2;
}

/**
 * tputl() - Write a 32-bit little-endian value to a file
 *
 * @fout: File to write to
 * @val: Value to write
 * Returns: number of bytes written (4)
 */
static int tputl(FILE *fout, ulong val)
{
	fputc(val, fout);
	fputc(val >> 8, fout);
	fputc(val >> 16, fout);
	fputc(val >> 24, fout);

	return 4;
}

/**
 * tputh() - Write a 64-bit little-endian value to a file
 *
 * @fout: File to write to
 * @val: Value to write
 * Returns: number of bytes written (8)
 */
static int tputq(FILE *fout, unsigned long long val)
{
	tputl(fout, val);
	tputl(fout, val >> 32U);

	return 8;
}

/**
 * tputh() - Write a string to a file
 *
 * The string is written without its terminator
 *
 * @fout: File to write to
 * @val: Value to write
 * Returns: number of bytes written
 */
static int tputs(FILE *fout, const char *str)
{
	fputs(str, fout);

	return strlen(str);
}

/**
 * add_str() - add a name string to the string table
 *
 * This is used by the v7 format
 *
 * @tw: Writer context
 * @name: String to write
 * Returns: Updated value of string pointer, or -1 if out of memory
 */
static int add_str(struct twriter *tw, const char *name)
{
	int str_ptr;
	int len;

	len = strlen(name) + 1;
	str_ptr = tw->str_ptr;
	tw->str_ptr += len;

	if (tw->str_ptr > abuf_size(&tw->str_buf)) {
		int new_size;

		new_size = ALIGN(tw->str_ptr, 4096);
		if (!abuf_realloc(&tw->str_buf, new_size))
			return -1;
	}

	return str_ptr;
}

/**
 * push_len() - Push a new length request onto the stack
 *
 * @tw: Writer context
 * @base: Base position of the length calculation
 * @msg: Indicates the type of caller, for debugging
 * @size: Size of the length value, either 4 bytes or 8
 * Returns number of bytes written to the file (=@size on success), -ve on error
 *
 * This marks a place where a length must be written, covering data that is
 * about to be written. It writes a placeholder value.
 *
 * Once the data is written, calling pop_len() will update the placeholder with
 * the correct length based on how many bytes have been written
 */
static int push_len(struct twriter *tw, int base, const char *msg, int size)
{
	struct tw_len *lp;

	if (tw->len_count >= LEN_STACK_SIZE) {
		fprintf(stderr, "Length-stack overflow: %s\n", msg);
		return -1;
	}
	if (size != 4 && size != 8) {
		fprintf(stderr, "Length-stack invalid size %d: %s\n", size,
			msg);
		return -1;
	}

	lp = &tw->len_stack[tw->len_count++];
	lp->base = base;
	lp->ptr = tw->ptr;
	lp->size = size;

	return size == 8 ? tputq(tw->fout, 0) : tputl(tw->fout, 0);
}

/**
 * pop_len() - Update a length value once the length is known
 *
 * Pops a value of the length stack and updates the file at that position with
 * the number of bytes written between now and then. Once done, the file is
 * seeked to the current (tw->ptr) position again, so writing can continue as
 * normal.
 *
 * @tw: Writer context
 * @msg: Indicates the type of caller, for debugging
 * Returns 0 if OK, -1 on error
 */
static int pop_len(struct twriter *tw, const char *msg)
{
	struct tw_len *lp;
	int len, ret;

	if (!tw->len_count) {
		fprintf(stderr, "Length-stack underflow: %s\n", msg);
		return -1;
	}

	lp = &tw->len_stack[--tw->len_count];
	if (fseek(tw->fout, lp->ptr, SEEK_SET))
		return -1;
	len = tw->ptr - lp->base;
	ret = lp->size == 8 ? tputq(tw->fout, len) : tputl(tw->fout, len);
	if (ret < 0)
		return -1;
	if (fseek(tw->fout, tw->ptr, SEEK_SET))
		return -1;

	return 0;
}

/**
 * start_header() - Start a v7 section
 *
 * Writes a header in v7 format
 *
 * @tw: Writer context
 * @id: ID of header to write (SECTION_...)
 * @flags: Flags value to write
 * @name: Name of section
 * Returns: number of bytes written
 */
static int start_header(struct twriter *tw, int id, uint flags,
			const char *name)
{
	int str_id;
	int lptr;
	int base;
	int ret;

	base = tw->ptr + 16;
	lptr = 0;
	lptr += tputh(tw->fout, id);
	lptr += tputh(tw->fout, flags);
	str_id = add_str(tw, name);
	if (str_id < 0)
		return -1;
	lptr += tputl(tw->fout, str_id);

	/* placeholder for size */
	ret = push_len(tw, base, "v7 header", 8);
	if (ret < 0)
		return -1;
	lptr += ret;

	return lptr;
}

/**
 * start_page() - Start a new page of output data
 *
 * The output is arranged in 4KB pages with a base timestamp at the start of
 * each. This starts a new page, making sure it is aligned to 4KB in the output
 * file.
 *
 * @tw: Writer context
 * @timestamp: Base timestamp for the page
 */
static int start_page(struct twriter *tw, ulong timestamp)
{
	int start;
	int ret;

	/* move to start of next page */
	start = ALIGN(tw->ptr, TRACE_PAGE_SIZE);
	ret = fseek(tw->fout, start, SEEK_SET);
	if (ret < 0) {
		fprintf(stderr, "Cannot seek to page start\n");
		return -1;
	}
	tw->ptr = start;

	/* page header */
	tw->ptr += tputq(tw->fout, timestamp);
	ret = push_len(tw, start + 16, "page", 8);
	if (ret < 0)
		return ret;
	tw->ptr += ret;

	return 0;
}

/**
 * finish_page() - finish a page
 *
 * Sets the lengths correctly and moves to the start of the next page
 *
 * @tw: Writer context
 * Returns: 0 on success, -1 on error
 */
static int finish_page(struct twriter *tw)
{
	int ret, end;

	ret = pop_len(tw, "page");
	if (ret < 0)
		return ret;
	end = ALIGN(tw->ptr, TRACE_PAGE_SIZE);

	/*
	 * Write a byte so that the data actually makes to the file, in the case
	 * that we never write any more pages
	 */
	if (tw->ptr != end) {
		if (fseek(tw->fout, end - 1, SEEK_SET)) {
			fprintf(stderr, "cannot seek to start of next page\n");
			return -1;
		}
		fputc(0, tw->fout);
		tw->ptr = end;
	}

	return 0;
}

/**
 * output_headers() - Output v6 headers to the file
 *
 * Writes out the various formats so that trace-cmd and kernelshark can make
 * sense of the data
 *
 * This updates tw->ptr as it goes
 *
 * @tw: Writer context
 * Returns: 0 on success, -ve on error
 */
static int output_headers(struct twriter *tw)
{
	FILE *fout = tw->fout;
	char str[800];
	int len, ret;

	tw->ptr += fprintf(fout, "%c%c%ctracing6%c%c%c", 0x17, 0x08, 0x44,
			   0 /* terminator */, 0 /* little endian */,
			   4 /* 32-bit long values */);

	/* host-machine page size 4KB */
	tw->ptr += tputl(fout, 4 << 10);

	tw->ptr += fprintf(fout, "header_page%c", 0);

	snprintf(str, sizeof(str),
		 "\tfield: u64 timestamp;\toffset:0;\tsize:8;\tsigned:0;\n"
		 "\tfield: local_t commit;\toffset:8;\tsize:8;\tsigned:1;\n"
		 "\tfield: int overwrite;\toffset:8;\tsize:1;\tsigned:1;\n"
		 "\tfield: char data;\toffset:16;\tsize:4080;\tsigned:1;\n");
	len = strlen(str);
	tw->ptr += tputq(fout, len);
	tw->ptr += tputs(fout, str);

	if (VERSION7) {
		/* no compression */
		tw->ptr += fprintf(fout, "none%cversion%c\n", 0, 0);

		ret = start_header(tw, SECTION_OPTIONS, 0, "options");
		if (ret < 0) {
			fprintf(stderr, "Cannot start option header\n");
			return -1;
		}
		tw->ptr += ret;
		tw->ptr += tputh(fout, OPTION_DONE);
		tw->ptr += tputl(fout, 8);
		tw->ptr += tputl(fout, 0);
		ret = pop_len(tw, "t7 header");
		if (ret < 0) {
			fprintf(stderr, "Cannot finish option header\n");
			return -1;
		}
	}

	tw->ptr += fprintf(fout, "header_event%c", 0);
	snprintf(str, sizeof(str),
		 "# compressed entry header\n"
		 "\ttype_len    :    5 bits\n"
		 "\ttime_delta  :   27 bits\n"
		 "\tarray       :   32 bits\n"
		 "\n"
		 "\tpadding     : type == 29\n"
		 "\ttime_extend : type == 30\n"
		 "\ttime_stamp : type == 31\n"
		 "\tdata max type_len  == 28\n");
	len = strlen(str);
	tw->ptr += tputq(fout, len);
	tw->ptr += tputs(fout, str);

	/* number of ftrace-event-format files */
	tw->ptr += tputl(fout, 3);

	snprintf(str, sizeof(str),
		 "name: function\n"
		 "ID: 1\n"
		 "format:\n"
		 "\tfield:unsigned short common_type;\toffset:0;\tsize:2;\tsigned:0;\n"
		 "\tfield:unsigned char common_flags;\toffset:2;\tsize:1;\tsigned:0;\n"
		 "\tfield:unsigned char common_preempt_count;\toffset:3;\tsize:1;signed:0;\n"
		 "\tfield:int common_pid;\toffset:4;\tsize:4;\tsigned:1;\n"
		 "\n"
		 "\tfield:unsigned long ip;\toffset:8;\tsize:8;\tsigned:0;\n"
		 "\tfield:unsigned long parent_ip;\toffset:16;\tsize:8;\tsigned:0;\n"
		 "\n"
		 "print fmt: \" %%ps <-- %%ps\", (void *)REC->ip, (void *)REC->parent_ip\n");
	len = strlen(str);
	tw->ptr += tputq(fout, len);
	tw->ptr += tputs(fout, str);

	snprintf(str, sizeof(str),
		 "name: funcgraph_entry\n"
		 "ID: 11\n"
		 "format:\n"
		 "\tfield:unsigned short common_type;\toffset:0;\tsize:2;\tsigned:0;\n"
		 "\tfield:unsigned char common_flags;\toffset:2;\tsize:1;\tsigned:0;\n"
		 "\tfield:unsigned char common_preempt_count;\toffset:3;\tsize:1;signed:0;\n"
		 "\tfield:int common_pid;\toffset:4;\tsize:4;\tsigned:1;\n"
		 "\n"
		 "\tfield:unsigned long func;\toffset:8;\tsize:8;\tsigned:0;\n"
		 "\tfield:int depth;\toffset:16;\tsize:4;\tsigned:1;\n"
		"\n"
		 "print fmt: \"--> %%ps (%%d)\", (void *)REC->func, REC->depth\n");
	len = strlen(str);
	tw->ptr += tputq(fout, len);
	tw->ptr += tputs(fout, str);

	snprintf(str, sizeof(str),
		 "name: funcgraph_exit\n"
		 "ID: 10\n"
		 "format:\n"
		 "\tfield:unsigned short common_type;\toffset:0;\tsize:2;\tsigned:0;\n"
		 "\tfield:unsigned char common_flags;\toffset:2;\tsize:1;\tsigned:0;\n"
		 "\tfield:unsigned char common_preempt_count;\toffset:3;\tsize:1;signed:0;\n"
		 "\tfield:int common_pid;\toffset:4;\tsize:4;\tsigned:1;\n"
		 "\n"
		 "\tfield:unsigned long func;\toffset:8;\tsize:8;\tsigned:0;\n"
		 "\tfield:int depth;\toffset:16;\tsize:4;\tsigned:1;\n"
		 "\tfield:unsigned int overrun;\toffset:20;\tsize:4;\tsigned:0;\n"
		 "\tfield:unsigned long long calltime;\toffset:24;\tsize:8;\tsigned:0;\n"
		 "\tfield:unsigned long long rettime;\toffset:32;\tsize:8;\tsigned:0;\n"
		 "\n"
		 "print fmt: \"<-- %%ps (%%d) (start: %%llx  end: %%llx) over: %%d\", (void *)REC->func, REC->depth, REC->calltime, REC->rettime, REC->depth\n");
	len = strlen(str);
	tw->ptr += tputq(fout, len);
	tw->ptr += tputs(fout, str);

	return 0;
}

/**
 * write_symbols() - Write the symbols out
 *
 * Writes the symbol information in the following format to mimic the Linux
 * /proc/kallsyms file:
 *
 * <address> T <name>
 *
 * This updates tw->ptr as it goes
 *
 * @tw: Writer context
 * Returns: 0 on success, -ve on error
 */
static int write_symbols(struct twriter *tw)
{
	char str[200];
	int ret, i;

	/* write symbols */
	ret = push_len(tw, tw->ptr + 4, "syms", 4);
	if (ret < 0)
		return -1;
	tw->ptr += ret;
	for (i = 0; i < func_count; i++) {
		struct func_info *func = &func_list[i];

		snprintf(str, sizeof(str), "%016lx T %s\n",
			 text_offset + func->offset, func->name);
		tw->ptr += tputs(tw->fout, str);
	}
	ret = pop_len(tw, "syms");
	if (ret < 0)
		return -1;
	tw->ptr += ret;

	return 0;
}

/**
 * write_options() - Write the options out
 *
 * Writes various options which are needed or useful. We use OPTION_TSC2NSEC
 * to indicates that values in the output need to be multiplied by 1000 since
 * U-Boot's trace values are in microseconds.
 *
 * This updates tw->ptr as it goes
 *
 * @tw: Writer context
 * Returns: 0 on success, -ve on error
 */
static int write_options(struct twriter *tw)
{
	FILE *fout = tw->fout;
	char str[200];
	int len;

	/* trace_printk, 0 for now */
	tw->ptr += tputl(fout, 0);

	/* processes */
	snprintf(str, sizeof(str), "%d u-boot\n", TRACE_PID);
	len = strlen(str);
	tw->ptr += tputq(fout, len);
	tw->ptr += tputs(fout, str);

	/* number of CPUs */
	tw->ptr += tputl(fout, 1);

	tw->ptr += fprintf(fout, "options  %c", 0);

	/* traceclock */
	tw->ptr += tputh(fout, OPTION_TRACECLOCK);
	tw->ptr += tputl(fout, 0);

	/* uname */
	tw->ptr += tputh(fout, OPTION_UNAME);
	snprintf(str, sizeof(str), "U-Boot");
	len = strlen(str);
	tw->ptr += tputl(fout, len);
	tw->ptr += tputs(fout, str);

	/* version */
	tw->ptr += tputh(fout, OPTION_VERSION);
	snprintf(str, sizeof(str), "unknown");
	len = strlen(str);
	tw->ptr += tputl(fout, len);
	tw->ptr += tputs(fout, str);

	/* trace ID */
	tw->ptr += tputh(fout, OPTION_TRACEID);
	tw->ptr += tputl(fout, 8);
	tw->ptr += tputq(fout, 0x123456780abcdef0);

	/* time conversion */
	tw->ptr += tputh(fout, OPTION_TSC2NSEC);
	tw->ptr += tputl(fout, 16);
	tw->ptr += tputl(fout, 1000);	/* multiplier */
	tw->ptr += tputl(fout, 0);	/* shift */
	tw->ptr += tputq(fout, 0);	/* offset */

	/* cpustat - bogus data for now, but at least it mentions the CPU */
	tw->ptr += tputh(fout, OPTION_CPUSTAT);
	snprintf(str, sizeof(str),
		 "CPU: 0\n"
		 "entries: 100\n"
		 "overrun: 43565\n"
		 "commit overrun: 0\n"
		 "bytes: 3360\n"
		 "oldest event ts: 963732.447752\n"
		 "now ts: 963832.146824\n"
		 "dropped events: 0\n"
		 "read events: 42379\n");
	len = strlen(str);
	tw->ptr += tputl(fout, len);
	tw->ptr += tputs(fout, str);

	tw->ptr += tputh(fout, OPTION_DONE);

	return 0;
}

/**
 * calc_min_depth() - Calculate the minimum call depth from the call list
 *
 * Starting with a depth of 0, this works through the call list, adding 1 for
 * each function call and subtracting 1 for each function return. Most likely
 * the value ends up being negative, since the trace does not start at the
 * very top of the call stack, e.g. main(), but some function called by that.
 *
 * This value can be used to calculate the depth value for the first call,
 * such that it never goes negative for subsequent returns.
 *
 * Returns: minimum call depth (e.g. -2)
 */
static int calc_min_depth(void)
{
	struct trace_call *call;
	int depth, min_depth, i;

	/* Calculate minimum depth */
	depth = 0;
	min_depth = 0;
	for (i = 0, call = call_list; i < call_count; i++, call++) {
		switch (TRACE_CALL_TYPE(call)) {
		case FUNCF_ENTRY:
			depth++;
			break;
		case FUNCF_EXIT:
			depth--;
			if (depth < min_depth)
				min_depth = depth;
			break;
		}
	}

	return min_depth;
}

/**
 * write_pages() - Write the pages of trace data
 *
 * This works through all the calls, writing out as many pages of data as are
 * needed.
 *
 * @tw: Writer context
 * @out_format: Output format to use
 * @missing_countp: Returns number of missing functions (not found in function
 * list)
 * @skip_countp: Returns number of skipped functions (excluded from trace)
 *
 * Returns: 0 on success, -ve on error
 */
static int write_pages(struct twriter *tw, enum out_format_t out_format,
		       int *missing_countp, int *skip_countp)
{
	ulong func_stack[MAX_STACK_DEPTH];
	int stack_ptr;	/* next free position in stack */
	int upto, depth, page_upto, i;
	int missing_count = 0, skip_count = 0;
	struct trace_call *call;
	ulong last_timestamp;
	FILE *fout = tw->fout;
	int last_delta = 0;
	int err_count;
	bool in_page;

	in_page = false;
	last_timestamp = 0;
	upto = 0;
	page_upto = 0;
	err_count = 0;

	/* maintain a stack of start times for calling functions */
	stack_ptr = 0;

	/*
	 * The first thing in the trace may not be the top-level function, so
	 * set the initial depth so that no function goes below depth 0
	 */
	depth = -calc_min_depth();
	for (i = 0, call = call_list; i < call_count; i++, call++) {
		bool entry = TRACE_CALL_TYPE(call) == FUNCF_ENTRY;
		struct func_info *func;
		ulong timestamp;
		uint rec_words;
		int delta;

		func = find_func_by_offset(call->func);
		if (!func) {
			warn("Cannot find function at %lx\n",
			     text_offset + call->func);
			missing_count++;
			if (missing_count > 20) {
				/* perhaps trace does not match System.map */
				fprintf(stderr, "Too many missing functions\n");
				return -1;
			}
			continue;
		}

		if (!(func->flags & FUNCF_TRACE)) {
			debug("Funcion '%s' is excluded from trace\n",
			      func->name);
			skip_count++;
			continue;
		}

		if (out_format == OUT_FMT_FUNCTION)
			rec_words = 6;
		else /* 2 header words and then 3 or 8 others */
			rec_words = 2 + (entry ? 3 : 8);

		/* convert timestamp from us to ns */
		timestamp = call->flags & FUNCF_TIMESTAMP_MASK;
		if (in_page) {
			if (page_upto + rec_words * 4 > TRACE_PAGE_SIZE) {
				if (finish_page(tw))
					return -1;
				in_page = false;
			}
		}
		if (!in_page) {
			if (start_page(tw, timestamp))
				return -1;
			in_page = true;
			last_timestamp = timestamp;
			last_delta = 0;
			page_upto = tw->ptr & TRACE_PAGE_MASK;
			if (_DEBUG) {
				fprintf(stderr,
					"new page, last_timestamp=%ld, upto=%d\n",
					last_timestamp, upto);
			}
		}

		delta = timestamp - last_timestamp;
		if (delta < 0) {
			fprintf(stderr, "Time went backwards\n");
			err_count++;
		}

		if (err_count > 20) {
			fprintf(stderr, "Too many errors, giving up\n");
			return -1;
		}

		if (delta > 0x07fffff) {
			/*
			 * hard to imagine how this could happen since it means
			 * that no function calls were made for a long time
			 */
			fprintf(stderr, "cannot represent time delta %x\n",
				delta);
			return -1;
		}

		if (out_format == OUT_FMT_FUNCTION) {
			struct func_info *caller_func;

			if (_DEBUG) {
				fprintf(stderr, "%d: delta=%d, stamp=%ld\n",
					upto, delta, timestamp);
				fprintf(stderr,
					"   last_delta %x to %x: last_timestamp=%lx, "
					"timestamp=%lx, call->flags=%x, upto=%d\n",
					last_delta, delta, last_timestamp,
					timestamp, call->flags, upto);
			}

			/* type_len is 6, meaning 4 * 6 = 24 bytes */
			tw->ptr += tputl(fout, rec_words | (uint)delta << 5);
			tw->ptr += tputh(fout, TRACE_FN);
			tw->ptr += tputh(fout, 0);	/* flags */
			tw->ptr += tputl(fout, TRACE_PID);	/* PID */
			/* function */
			tw->ptr += tputq(fout, text_offset + func->offset);
			caller_func = find_caller_by_offset(call->caller);
			/* caller */
			tw->ptr += tputq(fout,
					 text_offset + caller_func->offset);
		} else {
			tw->ptr += tputl(fout, rec_words | delta << 5);
			tw->ptr += tputh(fout, entry ? TRACE_GRAPH_ENT
						: TRACE_GRAPH_RET);
			tw->ptr += tputh(fout, 0);	/* flags */
			tw->ptr += tputl(fout, TRACE_PID); /* PID */
			/* function */
			tw->ptr += tputq(fout, text_offset + func->offset);
			tw->ptr += tputl(fout, depth); /* depth */
			if (entry) {
				depth++;
				if (stack_ptr < MAX_STACK_DEPTH)
					func_stack[stack_ptr] = timestamp;
				stack_ptr++;
			} else {
				ulong func_duration = 0;

				depth--;
				if (stack_ptr && stack_ptr <= MAX_STACK_DEPTH) {
					ulong start = func_stack[--stack_ptr];

					func_duration = timestamp - start;
				}
				tw->ptr += tputl(fout, 0);	/* overrun */
				tw->ptr += tputq(fout, 0);	/* calltime */
				/* rettime */
				tw->ptr += tputq(fout, func_duration);
			}
		}

		last_delta = delta;
		last_timestamp = timestamp;
		page_upto += 4 + rec_words * 4;
		upto++;
		if (stack_ptr == MAX_STACK_DEPTH)
			break;
	}
	if (in_page && finish_page(tw))
		return -1;
	*missing_countp = missing_count;
	*skip_countp = skip_count;

	return 0;
}

/**
 * write_flyrecord() - Write the flyrecord information
 *
 * Writes the header and pages of data for the "flyrecord" section. It also
 * writes out the counter-type info, selecting "[local]"
 *
 * @tw: Writer context
 * @out_format: Output format to use
 * @missing_countp: Returns number of missing functions (not found in function
 * list)
 * @skip_countp: Returns number of skipped functions (excluded from trace)
 *
 * Returns: 0 on success, -ve on error
 */
static int write_flyrecord(struct twriter *tw, enum out_format_t out_format,
			   int *missing_countp, int *skip_countp)
{
	unsigned long long start, start_ofs, len;
	int ret;
	FILE *fout = tw->fout;
	char str[200];

	/* Record start pointer */
	start_ofs = tw->ptr;
	debug("Start of flyrecord header at: 0x%llx\n", start_ofs);

	tw->ptr += fprintf(fout, "flyrecord%c", 0);

	/* flyrecord\0 - allocated 10 bytes */
	start_ofs += 10;

	/*
	 * 8 bytes that are a 64-bit word containing the offset into the file
	 * that holds the data for the CPU.
	 *
	 * 8 bytes that are a 64-bit word containing the size of the CPU
	 * data at that offset.
	 */
	start_ofs += 16;

	snprintf(str, sizeof(str),
		 "[local] global counter uptime perf mono mono_raw boot x86-tsc\n");
	len = strlen(str);

	/* trace clock length - 8 bytes */
	start_ofs += 8;
	/* trace clock data */
	start_ofs += len;

	debug("Calculated flyrecord header end at: 0x%llx, trace clock len: 0x%llx\n",
	      start_ofs, len);

	/* trace data */
	start = ALIGN(start_ofs, TRACE_PAGE_SIZE);
	tw->ptr += tputq(fout, start);

	/* use a placeholder for the size */
	ret = push_len(tw, start, "flyrecord", 8);
	if (ret < 0)
		return -1;
	tw->ptr += ret;

	tw->ptr += tputq(fout, len);
	tw->ptr += tputs(fout, str);

	debug("End of flyrecord header at: 0x%x, offset: 0x%llx\n",
	      tw->ptr, start);

	debug("trace text base %lx, map file %lx\n", text_base, text_offset);

	ret = write_pages(tw, out_format, missing_countp, skip_countp);
	if (ret < 0) {
		fprintf(stderr, "Cannot output pages\n");
		return -1;
	}

	ret = pop_len(tw, "flyrecord");
	if (ret < 0) {
		fprintf(stderr, "Cannot finish flyrecord header\n");
		return -1;
	}

	return 0;
}

/**
 * make_ftrace() - Write out an ftrace file
 *
 * See here for format:
 *
 * https://github.com/rostedt/trace-cmd/blob/master/Documentation/trace-cmd/trace-cmd.dat.v7.5.txt
 *
 * @fout: Output file
 * @out_format: Output format to use
 * Returns: 0 on success, -ve on error
 */
static int make_ftrace(FILE *fout, enum out_format_t out_format)
{
	int missing_count, skip_count;
	struct twriter tws, *tw = &tws;
	int ret;

	memset(tw, '\0', sizeof(*tw));
	abuf_init(&tw->str_buf);
	tw->fout = fout;

	tw->ptr = 0;
	ret = output_headers(tw);
	if (ret < 0) {
		fprintf(stderr, "Cannot output headers\n");
		return -1;
	}
	/* number of event systems files */
	tw->ptr += tputl(fout, 0);

	ret = write_symbols(tw);
	if (ret < 0) {
		fprintf(stderr, "Cannot write symbols\n");
		return -1;
	}

	ret = write_options(tw);
	if (ret < 0) {
		fprintf(stderr, "Cannot write options\n");
		return -1;
	}

	ret = write_flyrecord(tw, out_format, &missing_count, &skip_count);
	if (ret < 0) {
		fprintf(stderr, "Cannot write flyrecord\n");
		return -1;
	}

	info("ftrace: %d functions not found, %d excluded\n", missing_count,
	     skip_count);

	return 0;
}

/**
 * create_node() - Create a new node in the flamegraph tree
 *
 * @msg: Message to use for debugging if something goes wrong
 * Returns: Pointer to newly created node, or NULL on error
 */
static struct flame_node *create_node(const char *msg)
{
	struct flame_node *node;

	node = calloc(1, sizeof(*node));
	if (!node) {
		fprintf(stderr, "Out of memory for %s\n", msg);
		return NULL;
	}
	INIT_LIST_HEAD(&node->child_head);

	return node;
}

/**
 * process_call(): Add a call to the flamegraph info
 *
 * For function calls, if this call stack has been seen before, this increments
 * the call count, creating a new node if needed.
 *
 * For function returns, it adds up the time spent in this call stack,
 * subtracting the time spent by child functions.
 *
 * @state: Current flamegraph state
 * @entry: true if this is a function entry, false if a function exit
 * @timestamp: Timestamp from the trace file (in microseconds)
 * @func: Function that was called/returned from
 *
 * Returns: 0 on success, -ve on error
 */
static int process_call(struct flame_state *state, bool entry, ulong timestamp,
			struct func_info *func)
{
	struct flame_node *node = state->node;
	int stack_ptr = state->stack_ptr;

	if (entry) {
		struct flame_node *child, *chd;

		/* see if we have this as a child node already */
		child = NULL;
		list_for_each_entry(chd, &node->child_head, sibling_node) {
			if (chd->func == func) {
				child = chd;
				break;
			}
		}
		if (!child) {
			/* create a new node */
			child = create_node("child");
			if (!child)
				return -1;
			list_add_tail(&child->sibling_node, &node->child_head);
			child->func = func;
			child->parent = node;
			state->nodes++;
		}
		debug("entry %s: move from %s to %s\n", func->name,
		      node->func ? node->func->name : "(root)",
		      child->func->name);
		child->count++;
		if (stack_ptr < MAX_STACK_DEPTH) {
			state->stack[stack_ptr].timestamp = timestamp;
			state->stack[stack_ptr].child_total = 0;
		}
		debug("%d: %20s: entry at %ld\n", stack_ptr, func->name,
		      timestamp);
		stack_ptr++;
		node = child;
	} else if (node->parent) {
		ulong total_duration = 0, child_duration = 0;
		struct stack_info *stk;

		debug("exit  %s: move from %s to %s\n", func->name,
		      node->func->name, node->parent->func ?
		      node->parent->func->name : "(root)");
		if (stack_ptr && stack_ptr <= MAX_STACK_DEPTH) {
			stk = &state->stack[--stack_ptr];

			/*
			 * get total duration of the function which just
			 * exited
			 */
			total_duration = timestamp - stk->timestamp;
			child_duration = stk->child_total;

			if (stack_ptr)
				state->stack[stack_ptr - 1].child_total += total_duration;

			debug("%d: %20s: exit at %ld, total %ld, child %ld, child_total=%ld\n",
			      stack_ptr, func->name, timestamp,
			      total_duration, child_duration,
			      stk->child_total);
		}
		node->duration += total_duration - child_duration;
		node = node->parent;
	}

	state->stack_ptr = stack_ptr;
	state->node = node;

	return 0;
}

/**
 * make_flame_tree() - Create a tree of stack traces
 *
 * Set up a tree, with the root node having the top-level functions as children
 * and the leaf nodes being leaf functions. Each node has a count of how many
 * times this function appears in the trace
 *
 * @out_format: Output format to use
 * @treep: Returns the resulting flamegraph tree
 * Returns: 0 on success, -ve on error
 */
static int make_flame_tree(enum out_format_t out_format,
			   struct flame_node **treep)
{
	struct flame_state state;
	struct flame_node *tree;
	struct trace_call *call;
	int i;

	/* maintain a stack of start times, etc. for 'calling' functions */
	state.stack_ptr = 0;

	tree = create_node("tree");
	if (!tree)
		return -1;
	state.node = tree;
	state.nodes = 0;

	for (i = 0, call = call_list; i < call_count; i++, call++) {
		bool entry = TRACE_CALL_TYPE(call) == FUNCF_ENTRY;
		ulong timestamp = call->flags & FUNCF_TIMESTAMP_MASK;
		struct func_info *func;

		func = find_func_by_offset(call->func);
		if (!func) {
			warn("Cannot find function at %lx\n",
			     text_offset + call->func);
			continue;
		}

		if (process_call(&state, entry, timestamp, func))
			return -1;
	}
	fprintf(stderr, "%d nodes\n", state.nodes);
	*treep = tree;

	return 0;
}

/**
 * output_tree() - Output a flamegraph tree
 *
 * Writes the tree out to a file in a format suitable for flamegraph.pl
 *
 * This works by maintaining a string shared across all recursive calls. The
 * function name for this node is added to the existing string, to make up the
 * full call-stack description. For example, on entry, @str might contain:
 *
 *    "initf_bootstage;bootstage_mark_name"
 *                                        ^ @base
 *
 * with @base pointing to the \0 at the end of the string. This function adds
 * a ';' following by the name of the current function, e.g. "timer_get_boot_us"
 * as well as the output value, to get the full line:
 *
 * initf_bootstage;bootstage_mark_name;timer_get_boot_us 123
 *
 * @fout: Output file
 * @out_format: Output format to use
 * @node: Node to output (pass the whole tree at first)
 * @str: String to use to build the output line (e.g. 500 charas long)
 * @maxlen: Maximum length of string
 * @base: Current base position in the string
 * @treep: Returns the resulting flamegraph tree
 * Returns 0 if OK, -1 on error
 */
static int output_tree(FILE *fout, enum out_format_t out_format,
		       const struct flame_node *node, char *str, int maxlen,
		       int base)
{
	const struct flame_node *child;
	int pos;

	if (node->count) {
		if (out_format == OUT_FMT_FLAMEGRAPH_CALLS) {
			fprintf(fout, "%s %d\n", str, node->count);
		} else {
			/*
			 * Write out the number of microseconds used by this
			 * call stack. Since the time taken by child calls is
			 * subtracted from this total, it can reach 0, meaning
			 * that this function took no time beyond what its
			 * children used. For this case, write 1 rather than 0,
			 * so that this call stack appears in the flamegraph.
			 * This has the effect of inflating the timing slightly,
			 * but only by at most 1 microsecond per function,
			 * assuming that is the timestamp resolution
			 */
			fprintf(fout, "%s %ld\n", str,
				node->duration ? node->duration : 1);
		}
	}

	pos = base;
	if (pos)
		str[pos++] = ';';
	list_for_each_entry(child, &node->child_head, sibling_node) {
		int len;

		len = strlen(child->func->name);
		if (pos + len + 1 >= maxlen) {
			fprintf(stderr, "String too short (%d chars)\n",
				maxlen);
			return -1;
		}
		strcpy(str + pos, child->func->name);
		if (output_tree(fout, out_format, child, str, maxlen,
				pos + len))
			return -1;
	}

	return 0;
}

/**
 * make_flamegraph() - Write out a flame graph
 *
 * @fout: Output file
 * @out_format: Output format to use, e.g. function counts or timing
 * Returns 0 if OK, -1 on error
 */
static int make_flamegraph(FILE *fout, enum out_format_t out_format)
{
	struct flame_node *tree;
	char str[500];

	if (make_flame_tree(out_format, &tree))
		return -1;

	*str = '\0';
	if (output_tree(fout, out_format, tree, str, sizeof(str), 0))
		return -1;

	return 0;
}

/**
 * prof_tool() - Performs requested action
 *
 * @argc: Number of arguments (used to obtain the command
 * @argv: List of arguments
 * @trace_fname: Filename of input file (trace data from U-Boot)
 * @map_fname: Filename of map file (System.map from U-Boot)
 * @trace_config_fname: Trace-configuration file, or NULL if none
 * @out_fname: Output filename
 */
static int prof_tool(int argc, char *const argv[],
		     const char *trace_fname, const char *map_fname,
		     const char *trace_config_fname, const char *out_fname,
		     enum out_format_t out_format)
{
	int err = 0;

	if (read_map_file(map_fname))
		return -1;
	if (trace_fname && read_trace_file(trace_fname))
		return -1;
	if (trace_config_fname && read_trace_config_file(trace_config_fname))
		return -1;

	check_trace_config();

	for (; argc; argc--, argv++) {
		const char *cmd = *argv;

		if (!strcmp(cmd, "dump-ftrace")) {
			FILE *fout;

			if (out_format != OUT_FMT_FUNCTION &&
			    out_format != OUT_FMT_FUNCGRAPH)
				out_format = OUT_FMT_FUNCTION;
			fout = fopen(out_fname, "w");
			if (!fout) {
				fprintf(stderr, "Cannot write file '%s'\n",
					out_fname);
				return -1;
			}
			err = make_ftrace(fout, out_format);
			fclose(fout);
		} else if (!strcmp(cmd, "dump-flamegraph")) {
			FILE *fout;

			if (out_format != OUT_FMT_FLAMEGRAPH_CALLS &&
			    out_format != OUT_FMT_FLAMEGRAPH_TIMING)
				out_format = OUT_FMT_FLAMEGRAPH_CALLS;
			fout = fopen(out_fname, "w");
			if (!fout) {
				fprintf(stderr, "Cannot write file '%s'\n",
					out_fname);
				return -1;
			}
			err = make_flamegraph(fout, out_format);
			fclose(fout);
		} else {
			warn("Unknown command '%s'\n", cmd);
		}
	}

	return err;
}

int main(int argc, char *argv[])
{
	enum out_format_t out_format = OUT_FMT_DEFAULT;
	const char *map_fname = "System.map";
	const char *trace_fname = NULL;
	const char *config_fname = NULL;
	const char *out_fname = NULL;
	int opt;

	verbose = 2;
	while ((opt = getopt(argc, argv, "c:f:m:o:t:v:")) != -1) {
		switch (opt) {
		case 'c':
			config_fname = optarg;
			break;
		case 'f':
			if (!strcmp("function", optarg)) {
				out_format = OUT_FMT_FUNCTION;
			} else if (!strcmp("funcgraph", optarg)) {
				out_format = OUT_FMT_FUNCGRAPH;
			} else if (!strcmp("calls", optarg)) {
				out_format = OUT_FMT_FLAMEGRAPH_CALLS;
			} else if (!strcmp("timing", optarg)) {
				out_format = OUT_FMT_FLAMEGRAPH_TIMING;
			} else {
				fprintf(stderr,
					"Invalid format: use function, funcgraph, calls, timing\n");
				exit(1);
			}
			break;
		case 'm':
			map_fname = optarg;
			break;
		case 'o':
			out_fname = optarg;
			break;
		case 't':
			trace_fname = optarg;
			break;
		case 'v':
			verbose = atoi(optarg);
			break;
		default:
			usage();
		}
	}
	argc -= optind; argv += optind;
	if (argc < 1)
		usage();

	if (!out_fname || !map_fname || !trace_fname) {
		fprintf(stderr,
			"Must provide trace data, System.map file and output file\n");
		usage();
	}

	debug("Debug enabled\n");
	return prof_tool(argc, argv, trace_fname, map_fname, config_fname,
			 out_fname, out_format);
}
