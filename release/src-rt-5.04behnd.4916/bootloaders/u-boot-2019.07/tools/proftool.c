// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013 Google, Inc
 */

/* Decode and dump U-Boot profiling information */

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

#define MAX_LINE_LEN 500

enum {
	FUNCF_TRACE	= 1 << 0,	/* Include this function in trace */
};

struct func_info {
	unsigned long offset;
	const char *name;
	unsigned long code_size;
	unsigned long call_count;
	unsigned flags;
	/* the section this function is in */
	struct objsection_info *objsection;
};

enum trace_line_type {
	TRACE_LINE_INCLUDE,
	TRACE_LINE_EXCLUDE,
};

struct trace_configline_info {
	struct trace_configline_info *next;
	enum trace_line_type type;
	const char *name;	/* identifier name / wildcard */
	regex_t regex;		/* Regex to use if name starts with / */
};

/* The contents of the trace config file */
struct trace_configline_info *trace_config_head;

struct func_info *func_list;
int func_count;
struct trace_call *call_list;
int call_count;
int verbose;	/* Verbosity level 0=none, 1=warn, 2=notice, 3=info, 4=debug */
unsigned long text_offset;		/* text address of first function */

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
		"Usage: proftool -cds -v3 <cmd> <profdata>\n"
		"\n"
		"Commands\n"
		"   dump-ftrace\t\tDump out textual data in ftrace format\n"
		"\n"
		"Options:\n"
		"   -m <map>\tSpecify Systen.map file\n"
		"   -t <trace>\tSpecific trace data file (from U-Boot)\n"
		"   -v <0-4>\tSpecify verbosity\n");
	exit(EXIT_FAILURE);
}

static int h_cmp_offset(const void *v1, const void *v2)
{
	const struct func_info *f1 = v1, *f2 = v2;

	return (f1->offset / FUNC_SITE_SIZE) - (f2->offset / FUNC_SITE_SIZE);
}

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
	notice("%d functions found in map file\n", func_count);
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
		error("Cannot read profile file at pos %ld\n", ftell(fin));
		return -1;
	}
	return 0;
}

static struct func_info *find_func_by_offset(uint32_t offset)
{
	struct func_info key, *found;

	key.offset = offset;
	found = bsearch(&key, func_list, func_count, sizeof(struct func_info),
			h_cmp_offset);

	return found;
}

/* This finds the function which contains the given offset */
static struct func_info *find_caller_by_offset(uint32_t offset)
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

static int read_calls(FILE *fin, int count)
{
	struct trace_call *call_data;
	int i;

	notice("call count: %d\n", count);
	call_list = (struct trace_call *)calloc(count, sizeof(*call_data));
	if (!call_list) {
		error("Cannot allocate call_list\n");
		return -1;
	}
	call_count = count;

	call_data = call_list;
	for (i = 0; i < count; i++, call_data++) {
		if (read_data(fin, call_data, sizeof(*call_data)))
			return 1;
	}
	return 0;
}

static int read_profile(FILE *fin, int *not_found)
{
	struct trace_output_hdr hdr;

	*not_found = 0;
	while (!feof(fin)) {
		int err;

		err = read_data(fin, &hdr, sizeof(hdr));
		if (err == 1)
			break; /* EOF */
		else if (err)
			return 1;

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

static int read_profile_file(const char *fname)
{
	int not_found = INT_MAX;
	FILE *fprof;
	int err;

	fprof = fopen(fname, "rb");
	if (!fprof) {
		error("Cannot open profile data file '%s'\n",
		      fname);
		return 1;
	} else {
		err = read_profile(fprof, &not_found);
		fclose(fprof);
		if (err)
			return err;

		if (not_found) {
			warn("%d profile functions could not be found in the map file - are you sure that your profile data and map file correspond?\n",
			     not_found);
			return 1;
		}
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

static void check_trace_config(void)
{
	struct trace_configline_info *line;

	for (line = trace_config_head; line; line = line->next)
		check_trace_config_line(line);
}

/**
 * Check the functions to see if they each have an objsection. If not, then
 * the linker must have eliminated them.
 */
static void check_functions(void)
{
	struct func_info *func, *end;
	unsigned long removed_code_size = 0;
	int not_found = 0;

	/* Look for missing functions */
	for (func = func_list, end = func + func_count; func < end; func++) {
		if (!func->objsection) {
			removed_code_size += func->code_size;
			not_found++;
		}
	}

	/* Figure out what functions we want to trace */
	check_trace_config();

	warn("%d functions removed by linker, %ld code size\n",
	     not_found, removed_code_size);
}

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

		line = (struct trace_configline_info *)calloc(1,
							      sizeof(*line));
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

static void out_func(ulong func_offset, int is_caller, const char *suffix)
{
	struct func_info *func;

	func = (is_caller ? find_caller_by_offset : find_func_by_offset)
		(func_offset);

	if (func)
		printf("%s%s", func->name, suffix);
	else
		printf("%lx%s", func_offset, suffix);
}

/*
 * # tracer: function
 * #
 * #           TASK-PID   CPU#    TIMESTAMP  FUNCTION
 * #              | |      |          |         |
 * #           bash-4251  [01] 10152.583854: path_put <-path_walk
 * #           bash-4251  [01] 10152.583855: dput <-path_put
 * #           bash-4251  [01] 10152.583855: _atomic_dec_and_lock <-dput
 */
static int make_ftrace(void)
{
	struct trace_call *call;
	int missing_count = 0, skip_count = 0;
	int i;

	printf("# tracer: ftrace\n"
		"#\n"
		"#           TASK-PID   CPU#    TIMESTAMP  FUNCTION\n"
		"#              | |      |          |         |\n");
	for (i = 0, call = call_list; i < call_count; i++, call++) {
		struct func_info *func = find_func_by_offset(call->func);
		ulong time = call->flags & FUNCF_TIMESTAMP_MASK;

		if (TRACE_CALL_TYPE(call) != FUNCF_ENTRY &&
		    TRACE_CALL_TYPE(call) != FUNCF_EXIT)
			continue;
		if (!func) {
			warn("Cannot find function at %lx\n",
			     text_offset + call->func);
			missing_count++;
			continue;
		}

		if (!(func->flags & FUNCF_TRACE)) {
			debug("Funcion '%s' is excluded from trace\n",
			      func->name);
			skip_count++;
			continue;
		}

		printf("%16s-%-5d [01] %lu.%06lu: ", "uboot", 1,
		       time / 1000000, time % 1000000);

		out_func(call->func, 0, " <- ");
		out_func(call->caller, 1, "\n");
	}
	info("ftrace: %d functions not found, %d excluded\n", missing_count,
	     skip_count);

	return 0;
}

static int prof_tool(int argc, char * const argv[],
		     const char *prof_fname, const char *map_fname,
		     const char *trace_config_fname)
{
	int err = 0;

	if (read_map_file(map_fname))
		return -1;
	if (prof_fname && read_profile_file(prof_fname))
		return -1;
	if (trace_config_fname && read_trace_config_file(trace_config_fname))
		return -1;

	check_functions();

	for (; argc; argc--, argv++) {
		const char *cmd = *argv;

		if (0 == strcmp(cmd, "dump-ftrace"))
			err = make_ftrace();
		else
			warn("Unknown command '%s'\n", cmd);
	}

	return err;
}

int main(int argc, char *argv[])
{
	const char *map_fname = "System.map";
	const char *prof_fname = NULL;
	const char *trace_config_fname = NULL;
	int opt;

	verbose = 2;
	while ((opt = getopt(argc, argv, "m:p:t:v:")) != -1) {
		switch (opt) {
		case 'm':
			map_fname = optarg;
			break;

		case 'p':
			prof_fname = optarg;
			break;

		case 't':
			trace_config_fname = optarg;
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

	debug("Debug enabled\n");
	return prof_tool(argc, argv, prof_fname, map_fname,
			 trace_config_fname);
}
