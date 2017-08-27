/* Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *	 http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>

#if !defined(__MINGW32__)
#  include <sys/wait.h>
#endif

#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <assert.h>

#ifdef __EMX__
#  define SHELL_CMD 			"sh"
#  define GEN_EXPORTS			"emxexp"
#  define DEF2IMPLIB_CMD		"emximp"
#  define SHARE_SW			"-Zdll -Zmtd"
#  define USE_OMF 1
#  define TRUNCATE_DLL_NAME
#  define DYNAMIC_LIB_EXT		"dll"
#  define EXE_EX			".exe"
/* OMF is the native format under OS/2 */
#  if USE_OMF

#    define STATIC_LIB_EXT		"lib"
#    define OBJECT_EXT			"obj"
#    define LIBRARIAN			"emxomfar"
#    define LIBRARIAN_OPTS		"cr"
#  else
/* but the alternative, a.out, can fork() which is sometimes necessary */
#    define STATIC_LIB_EXT		"a"
#    define OBJECT_EXT			"o"
#    define LIBRARIAN			"ar"
#    define LIBRARIAN_OPTS		"cr"
#  endif
#endif

#if defined(__APPLE__)
#  define SHELL_CMD			"/bin/sh"
#  define DYNAMIC_LIB_EXT		"dylib"
#  define MODULE_LIB_EXT		"bundle"
#  define STATIC_LIB_EXT		"a"
#  define OBJECT_EXT			"o"
#  define LIBRARIAN			"ar"
#  define LIBRARIAN_OPTS		"cr"
/* man libtool(1) documents ranlib option of -c.  */
#  define RANLIB			"ranlib"
#  define PIC_FLAG			"-fPIC -fno-common"
#  define SHARED_OPTS			"-dynamiclib"
#  define MODULE_OPTS			"-bundle -dynamic"
#  define DYNAMIC_LINK_OPTS		"-flat_namespace"
#  define DYNAMIC_LINK_UNDEFINED	"-undefined suppress"
#  define dynamic_link_version_func	darwin_dynamic_link_function
#  define DYNAMIC_INSTALL_NAME		"-install_name"
#  define DYNAMIC_LINK_NO_INSTALL	"-dylib_file"
#  define HAS_REALPATH
/*-install_name  /Users/jerenk/apache-2.0-cvs/lib/libapr.0.dylib -compatibility_version 1 -current_version 1.0 */
#  define LD_LIBRARY_PATH		"DYLD_LIBRARY_PATH"
#  define LD_LIBRARY_PATH_LOCAL		"DYLD_FALLBACK_LIBRARY_PATH"
#endif

#if defined(__linux__) || defined(__FreeBSD__) || defined(__NetBSD__)
#  define SHELL_CMD 			"/bin/sh"
#  define DYNAMIC_LIB_EXT		"so"
#  define MODULE_LIB_EXT		"so"
#  define STATIC_LIB_EXT		"a"
#  define OBJECT_EXT			"o"
#  define LIBRARIAN			"ar"
#  define LIBRARIAN_OPTS		"cr"
#  define RANLIB			"ranlib"
#  define PIC_FLAG			"-fPIC"
#  define RPATH				"-rpath"
#  define SHARED_OPTS			"-shared"
#  define MODULE_OPTS			"-shared"
#  define LINKER_FLAG_PREFIX		"-Wl,"
#  define DYNAMIC_LINK_OPTS		LINKER_FLAG_PREFIX "-export-dynamic"
#  define ADD_MINUS_L
#  define LD_RUN_PATH			"LD_RUN_PATH"
#  define LD_LIBRARY_PATH		"LD_LIBRARY_PATH"
#  define LD_LIBRARY_PATH_LOCAL		LD_LIBRARY_PATH
#endif

#if defined(sun)
#  define SHELL_CMD			"/bin/sh"
#  define DYNAMIC_LIB_EXT		"so"
#  define MODULE_LIB_EXT		"so"
#  define STATIC_LIB_EXT		"a"
#  define OBJECT_EXT			"o"
#  define LIBRARIAN			"ar"
#  define LIBRARIAN_OPTS		"cr"
#  define RANLIB			"ranlib"
#  define PIC_FLAG			"-KPIC"
#  define RPATH				"-R"
#  define SHARED_OPTS			"-G"
#  define MODULE_OPTS			"-G"
#  define DYNAMIC_LINK_OPTS		""
#  define LINKER_FLAG_NO_EQUALS
#  define ADD_MINUS_L
#  define HAS_REALPATH
#  define LD_RUN_PATH			"LD_RUN_PATH"
#  define LD_LIBRARY_PATH		"LD_LIBRARY_PATH"
#  define LD_LIBRARY_PATH_LOCAL		LD_LIBRARY_PATH
#endif

#if defined(_OSD_POSIX)
#  define SHELL_CMD			"/usr/bin/sh"
#  define DYNAMIC_LIB_EXT		"so"
#  define MODULE_LIB_EXT		"so"
#  define STATIC_LIB_EXT		"a"
#  define OBJECT_EXT			"o"
#  define LIBRARIAN			"ar"
#  define LIBRARIAN_OPTS		"cr"
#  define SHARED_OPTS			"-G"
#  define MODULE_OPTS			"-G"
#  define LINKER_FLAG_PREFIX		"-Wl,"
#  define NEED_SNPRINTF
#endif

#if defined(sinix) && defined(mips) && defined(__SNI_TARG_UNIX)
#  define SHELL_CMD			"/usr/bin/sh"
#  define DYNAMIC_LIB_EXT		"so"
#  define MODULE_LIB_EXT		"so"
#  define STATIC_LIB_EXT		"a"
#  define OBJECT_EXT			"o"
#  define LIBRARIAN			"ar"
#  define LIBRARIAN_OPTS		"cr"
#  define RPATH				"-Brpath"
#  define SHARED_OPTS			"-G"
#  define MODULE_OPTS			"-G"
#  define LINKER_FLAG_PREFIX		"-Wl,"
#  define DYNAMIC_LINK_OPTS		LINKER_FLAG_PREFIX "-Blargedynsym"

#  define NEED_SNPRINTF
#  define LD_RUN_PATH			"LD_RUN_PATH"
#  define LD_LIBRARY_PATH		"LD_LIBRARY_PATH"
#  define LD_LIBRARY_PATH_LOCAL		LD_LIBRARY_PATH
#endif

#if defined(__MINGW32__)
#  define SHELL_CMD			"sh"
#  define DYNAMIC_LIB_EXT		"dll"
#  define MODULE_LIB_EXT 		"dll"
#  define STATIC_LIB_EXT		"a"
#  define OBJECT_EXT			"o"
#  define LIBRARIAN			"ar"
#  define LIBRARIAN_OPTS		"cr"
#  define RANLIB			"ranlib"
#  define LINKER_FLAG_PREFIX		"-Wl,"
#  define SHARED_OPTS			"-shared"
#  define MODULE_OPTS			"-shared"
#  define MKDIR_NO_UMASK
#  define EXE_EXT			".exe"
#endif

#ifndef CC
#define CC				"gcc"
#endif

#ifndef CXX
#define CXX				"g++"
#endif

#ifndef LINK_C
#define LINK_C				"gcc"
#endif

#ifndef LINK_CXX
#define LINK_CXX			"g++"
#endif

#ifndef LIBDIR
#define LIBDIR				"/usr/local/lib"
#endif

#define OBJDIR				".libs"

#ifndef SHELL_CMD
#error Unsupported platform: Please add defines for SHELL_CMD etc. for your platform.
#endif

#ifdef NEED_SNPRINTF
#include <stdarg.h>
#endif

#ifdef __EMX__
#include <process.h>
#endif

#ifndef PATH_MAX
#define PATH_MAX 1024
#endif


/* We want to say we are libtool 1.4 for shlibtool compatibility. */
#define VERSION "1.4"

#define DEBUG(fmt, ...) if(cmd->options.debug) printf(fmt, ## __VA_ARGS__)
#define NOTICE(fmt, ...) if(!cmd->options.silent) printf(fmt, ## __VA_ARGS__)
#define ERROR(fmt, ...) fprintf(stderr, fmt, ## __VA_ARGS__)

enum tool_mode {
	MODE_UNKNOWN,
	MODE_COMPILE,
	MODE_LINK,
	MODE_EXECUTE,
	MODE_INSTALL,
};

enum output_type {
	OUT_GENERAL,
	OUT_OBJECT,
	OUT_PROGRAM,
	OUT_LIB,
	OUT_STATIC_LIB_ONLY,
	OUT_DYNAMIC_LIB_ONLY,
	OUT_MODULE,
};

enum pic_mode {
	PIC_UNKNOWN,
	PIC_PREFER,
	PIC_AVOID,
};

enum shared_mode {
	SHARE_UNSET,
	SHARE_STATIC,
	SHARE_SHARED,
};

enum lib_type {
	TYPE_UKNOWN,
	TYPE_STATIC_LIB,
	TYPE_DYNAMIC_LIB,
	TYPE_MODULE_LIB,
	TYPE_OBJECT,
};

typedef struct {
	char const **vals;
	int num;
} count_chars;

typedef struct {
	char const *normal;
	char const *install;
} library_name;

typedef struct {
	count_chars *normal;
	count_chars *install;
	count_chars *dependencies;
} library_opts;

typedef struct {
	int silent;
	int debug;
	enum shared_mode shared;
	int export_all;
	int dry_run;
	enum pic_mode pic_mode;
	int export_dynamic;
	int no_install;
} options_t;

typedef struct {
	enum tool_mode mode;
	enum output_type output;
	options_t options;

	char const *output_name;
	char const *fake_output_name;
	char const *basename;

	char const *install_path;
	char const *compiler;
	char const *program;
	count_chars *program_opts;

	count_chars *arglist;
	count_chars *tmp_dirs;
	count_chars *obj_files;
	count_chars *dep_rpaths;
	count_chars *rpaths;

	library_name static_name;
	library_name shared_name;
	library_name module_name;

	library_opts static_opts;
	library_opts shared_opts;

	char const *version_info;
	char const *undefined_flag;
} command_t;

#ifdef RPATH
static void add_rpath(count_chars *cc, char const *path);
#endif

static void usage(int code)
{
	printf("Usage: jlibtool [OPTIONS...] COMMANDS...\n");
	printf("jlibtool is a replacement for GNU libtool with similar functionality.\n\n");

	printf("  --config	 show all configuration variables\n");
	printf("  --debug	  enable verbose shell tracing\n");
	printf("  --dry-run	display commands without modifying any files\n");
	printf("  --help	   display this help message and exit\n");
	printf("  --mode=MODE	   use operational mode MODE (you *must* set mode)\n");

	printf("  --silent	 don't print informational messages\n");
	printf("  --tag=TAG	Ignored for libtool compatibility\n");
	printf("  --version	print version information\n");


	printf("  --shared	 Build shared libraries when using --mode=link\n");
	printf("  --export-all	   Try to export 'def' file on some platforms\n");

	printf("\nMODE must be one of the following:\n\n");
	printf("  compile	  compile a source file into a jlibtool object\n");
	printf("  execute	  automatically set library path, then run a program\n");
	printf("  install	  install libraries or executables\n");
	printf("  link	     create a library or an executable\n");

	printf("\nMODE-ARGS can be the following:\n\n");
	printf("  -export-dynamic  accepted and ignored\n");
	printf("  -module	  create a module when linking\n");
	printf("  -shared	  create a shared library when linking\n");
	printf("  -prefer-pic      prefer position-independent-code when compiling\n");
	printf("  -prefer-non-pic  prefer non position-independent-code when compiling\n");
	printf("  -static	  create a static library when linking\n");
	printf("  -no-install      link libraries locally\n");
	printf("  -rpath arg	   Set install path for shared libraries\n");
	printf("  -l arg	   pass '-l arg' to the link stage\n");
	printf("  -L arg	   pass '-L arg' to the link stage\n");
	printf("  -R dir	   add 'dir' to runtime library search path.\n");
	printf("  -Zexe	    accepted and ignored\n");
	printf("  -avoid-version   accepted and ignored\n");

	exit(code);
}

#if defined(NEED_SNPRINTF)
/* Write at most n characters to the buffer in str, return the
 * number of chars written or -1 if the buffer would have been
 * overflowed.
 *
 * This is portable to any POSIX-compliant system has /dev/null
 */
static FILE *f = NULL;
static int vsnprintf(char *str, size_t n, char const *fmt, va_list ap)
{
	int res;

	if (!f) {
		f = fopen("/dev/null","w");
	}

	if (!f) {
		return -1;
	}

	setvbuf(f, str, _IOFBF, n);

	res = vfprintf(f, fmt, ap);

	if ((res > 0) && (res < n)) {
		res = vsprintf( str, fmt, ap );
	}
	return res;
}

static int snprintf(char *str, size_t n, char const *fmt, ...)
{
	va_list ap;
	int res;

	va_start( ap, fmt );
	res = vsnprintf( str, n, fmt, ap );
	va_end( ap );
	return res;
}
#endif

static void *lt_malloc(size_t size)
{
	void *out;

	out = malloc(size);
	if (!out) {
		ERROR("Failed allocating %zu bytes, OOM", size);
		exit(1);
	}

	return out;
}

static void lt_const_free(const void *ptr)
{
	void *tmp;

	memcpy(&tmp, &ptr, sizeof(tmp));
	free(tmp);
}

static void init_count_chars(count_chars *cc)
{
	cc->vals = (char const**) lt_malloc(PATH_MAX*sizeof(char*));
	cc->num = 0;
}

static count_chars *alloc_countchars()
{
	count_chars *out;
	out = lt_malloc(sizeof(count_chars));
	if (!out) {
		exit(1);
	}
	init_count_chars(out);

	return out;
}

static void clear_count_chars(count_chars *cc)
{
	int i;
	for (i = 0; i < cc->num; i++) {
		cc->vals[i] = NULL;
	}

	cc->num = 0;
}

static void push_count_chars(count_chars *cc, char const *newval)
{
	cc->vals[cc->num++] = newval;
}

static void pop_count_chars(count_chars *cc)
{
	cc->num--;
}

static void insert_count_chars(count_chars *cc, char const *newval, int position)
{
	int i;

	for (i = cc->num; i > position; i--) {
		cc->vals[i] = cc->vals[i-1];
	}

	cc->vals[position] = newval;
	cc->num++;
}

static void append_count_chars(count_chars *cc, count_chars *cctoadd)
{
	int i;
	for (i = 0; i < cctoadd->num; i++) {
		if (cctoadd->vals[i]) {
			push_count_chars(cc, cctoadd->vals[i]);
		}
	}
}

static char const *flatten_count_chars(count_chars *cc, int space)
{
	int i, size;
	char *newval;

	size = 0;
	for (i = 0; i < cc->num; i++) {
		if (cc->vals[i]) {
			size += strlen(cc->vals[i]) + 1;
			if (space) {
			  size++;
			}
		}
	}

	newval = (char*)lt_malloc(size + 1);
	newval[0] = 0;

	for (i = 0; i < cc->num; i++) {
		if (cc->vals[i]) {
			strcat(newval, cc->vals[i]);
			if (space) {
				strcat(newval, " ");
			}
		}
	}

	return newval;
}

static char *shell_esc(char const *str)
{
	int in_quote = 0;
	char *cmd;
	uint8_t *d;
	uint8_t const *s;

	cmd = (char *)lt_malloc(2 * strlen(str) + 3);
	d = (unsigned char *)cmd;
	s = (const unsigned char *)str;

#ifdef __MINGW32__
	*d++ = '\"';
#endif

	for (; *s; ++s) {
		if (*s == '"') {
			*d++ = '\\';
			in_quote++;
		}
		else if (*s == '\\' || (*s == ' ' && (in_quote % 2))) {
			*d++ = '\\';
		}
		*d++ = *s;
	}

#ifdef __MINGW32__
	*d++ = '\"';
#endif

	*d = '\0';
	return cmd;
}

static int external_spawn(command_t *cmd, char const *file, char const **argv)
{
	file = file;		/* -Wunused */

	if (!cmd->options.silent) {
		char const **argument = argv;
		NOTICE("Executing: ");
		while (*argument) {
			NOTICE("%s ", *argument);
			argument++;
		}
		puts("");
	}

	if (cmd->options.dry_run) {
		return 0;
	}
#if defined(__EMX__) || defined(__MINGW32__)
	return spawnvp(P_WAIT, argv[0], argv);
#else
	{
		pid_t pid;
		pid = fork();
		if (pid == 0) {
			return execvp(argv[0], (char**)argv);
		}
		else {
			int statuscode;
			waitpid(pid, &statuscode, 0);
			if (WIFEXITED(statuscode)) {
				return WEXITSTATUS(statuscode);
			}
			return 0;
		}
	}
#endif
}

static int run_command(command_t *cmd, count_chars *cc)
{
	int ret;
	char *command;
	char *tmp;
	char const *raw;
	char const *spawn_args[4];
	count_chars tmpcc;

	init_count_chars(&tmpcc);

	if (cmd->program) {
		push_count_chars(&tmpcc, cmd->program);
	}

	append_count_chars(&tmpcc, cmd->program_opts);

	append_count_chars(&tmpcc, cc);

	raw = flatten_count_chars(&tmpcc, 1);
	command = shell_esc(raw);

	memcpy(&tmp, &raw, sizeof(tmp));
	free(tmp);

	spawn_args[0] = SHELL_CMD;
	spawn_args[1] = "-c";
	spawn_args[2] = command;
	spawn_args[3] = NULL;
	ret = external_spawn(cmd, spawn_args[0], spawn_args);

	free(command);

	return ret;
}

/*
 * print configuration
 * shlibpath_var is used in configure.
 */
#define printc(_x,_y) if (!value || !strcmp(value, _x)) printf(_x "=\"%s\"\n", _y)

static void print_config(char const *value)
{
#ifdef LD_RUN_PATH
	printc("runpath_var", LD_RUN_PATH);
#endif
#ifdef LD_LIBRARY_PATH
	printc("shlibpath_var", LD_LIBRARY_PATH);
#endif
#ifdef LD_LIBRARY_PATH_LOCAL
	printc("shlocallibpath_var", LD_LIBRARY_PATH_LOCAL);
#endif
#ifdef SHELL_CMD
	printc("SHELL", SHELL_CMD);
#endif
#ifdef OBJECT_EXT
	printc("objext", OBJECT_EXT);
#endif
#ifdef OBJDIR
	printc("objdir", OBJDIR);
#endif
#ifdef DYNAMIC_LIB_EXT
	/* add a '.' prefix because libtool does that. */
	printc("shrext_cmds", "echo ." DYNAMIC_LIB_EXT);
	/* add a '.' prefix because libtool does that. */
	printc("shrext", "." DYNAMIC_LIB_EXT);
#endif
#ifdef EXE_EXT
	printc("exeext", EXE_EXT);
#endif
#ifdef STATIC_LIB_EXT
	printc("libext", STATIC_LIB_EXT);
#endif
#ifdef LIBRARIAN
	printc("AR", LIBRARIAN);
#endif
#ifdef LIBRARIAN_OPTS
	printc("AR_FLAGS", LIBRARIAN_OPTS);
#endif
#ifdef LINKER_FLAG_PREFIX
	printc("wl", LINKER_FLAG_PREFIX);
#endif
#ifdef RANLIB
	printc("ranlib", RANLIB);
#endif

}
/*
 * Add a directory to the runtime library search path.
 */
static void add_runtime_dir_lib(char const *arg, command_t *cmd)
{
#ifdef RPATH
	add_rpath(cmd->shared_opts.dependencies, arg);
#else
	(void) arg;			/* -Wunused */
	(void) cmd;
#endif
}

static int parse_long_opt(char const *arg, command_t *cmd)
{
	char *equal_pos = strchr(arg, '=');
	char var[50];
	char value[500];

	if (equal_pos) {
		strncpy(var, arg, equal_pos - arg);
		var[equal_pos - arg] = 0;
	if (strlen(equal_pos + 1) >= sizeof(var)) return 0;
		strcpy(value, equal_pos + 1);
	} else {
		strncpy(var, arg, sizeof(var) - 1);
		var[sizeof(var) - 1] = '\0';

	value[0] = '\0';
	}

	if (strcmp(var, "silent") == 0) {
		cmd->options.silent = 1;
	} else if (strcmp(var, "quiet") == 0) {
		cmd->options.silent = 1;
	} else if (strcmp(var, "debug") == 0) {
		cmd->options.debug = 1;
	} else if (strcmp(var, "mode") == 0) {
		if (cmd->mode != MODE_UNKNOWN) {
			ERROR("Cannot set --mode twice\n");
			exit(1);
		}

		if (strcmp(value, "compile") == 0) {
			cmd->mode = MODE_COMPILE;
			cmd->output = OUT_OBJECT;

		} else if (strcmp(value, "link") == 0) {
			cmd->mode = MODE_LINK;
			cmd->output = OUT_LIB;

		} else if (strcmp(value, "install") == 0) {
			cmd->mode = MODE_INSTALL;

		} else if (strcmp(value, "execute") == 0) {
			cmd->mode = MODE_EXECUTE;

		} else {
			ERROR("Unknown mode \"%s\"\n", value);
			exit(1);
		}

	} else if (strcmp(var, "shared") == 0) {
		if ((cmd->mode == MODE_LINK) && (cmd->output == OUT_GENERAL)) {
			cmd->output = OUT_DYNAMIC_LIB_ONLY;
		}
		cmd->options.shared = SHARE_SHARED;

	} else if (strcmp(var, "export-all") == 0) {
		cmd->options.export_all = 1;

	} else if (strcmp(var, "dry-run") == 0) {
		NOTICE("Dry-run mode on!\n");
		cmd->options.dry_run = 1;

	} else if (strcmp(var, "version") == 0) {
		NOTICE("Version " VERSION "\n");

	} else if (strcmp(var, "help") == 0) {
		usage(0);

	} else if (strcmp(var, "config") == 0) {
		print_config(value);

		exit(0);
	} else {
		return 0;
	}

	return 1;
}

/* Return 1 if we eat it. */
static int parse_short_opt(char const *arg, command_t *cmd)
{
	if (strcmp(arg, "export-dynamic") == 0) {
		cmd->options.export_dynamic = 1;
		return 1;
	}

	if (strcmp(arg, "module") == 0) {
		cmd->output = OUT_MODULE;
		return 1;
	}

	if (strcmp(arg, "shared") == 0) {
		if (cmd->mode == MODE_LINK) {
			cmd->output = OUT_DYNAMIC_LIB_ONLY;
		}
		cmd->options.shared = SHARE_SHARED;
		return 1;
	}

	if (strcmp(arg, "Zexe") == 0) {
		return 1;
	}

	if (strcmp(arg, "avoid-version") == 0) {
		return 1;
	}

	if (strcmp(arg, "prefer-pic") == 0) {
		cmd->options.pic_mode = PIC_PREFER;
		return 1;
	}

	if (strcmp(arg, "prefer-non-pic") == 0) {
		cmd->options.pic_mode = PIC_AVOID;
		return 1;
	}

	if (strcmp(arg, "static") == 0) {
		if ((cmd->mode == MODE_LINK) && (cmd->output == OUT_LIB)) {
			cmd->output = OUT_STATIC_LIB_ONLY;
		}
		cmd->options.shared = SHARE_STATIC;
		return 1;
	}

	if (cmd->mode == MODE_LINK) {
		if (strcmp(arg, "no-install") == 0) {
			cmd->options.no_install = 1;
			return 1;
		}
		if (arg[0] == 'L' || arg[0] == 'l') {
			/* Hack... */
			arg--;
			push_count_chars(cmd->shared_opts.dependencies, arg);
			return 1;
		} else if (arg[0] == 'R' && arg[1]) {
			/* -Rdir Add dir to runtime library search path. */
			add_runtime_dir_lib(&arg[1], cmd);
			return 1;
		}
	}
	return 0;
}

#ifdef TRUNCATE_DLL_NAME
static char *truncate_dll_name(char *path)
{
	/* Cut DLL name down to 8 characters after removing any mod_ prefix */
	char *tmppath = strdup(path);
	char *newname = strrchr(tmppath, '/') + 1;
	char *ext = strrchr(newname, '.');
	int len;

	if (ext == NULL) {
		return tmppath;
	}

	len = ext - newname;

	if (strncmp(newname, "mod_", 4) == 0) {
		strcpy(newname, newname + 4);
		len -= 4;
	}

	if (len > 8) {
		strcpy(newname + 8, strchr(newname, '.'));
	}

	return tmppath;
}
#endif

static long safe_strtol(char const *nptr, char const **endptr, int base)
{
	long rv;

	errno = 0;

	rv = strtol(nptr, (char**)endptr, 10);

	if (errno == ERANGE) {
		return 0;
	}

	return rv;
}

static void safe_mkdir(command_t *cmd, char const *path)
{
	int status;
	mode_t old_umask;

	old_umask = umask(0);
	umask(old_umask);

#ifdef MKDIR_NO_UMASK
	status = mkdir(path);
#else
	status = mkdir(path, ~old_umask);
#endif
	if ((status < 0) && (errno != EEXIST)) {
		NOTICE("Warning: mkdir of %s failed\n", path);
	}
}

/** Returns a file's name without the path
 *
 * @param path to break apart.
 * @return pointer in path.
 */
static char const *file_name(char const *path)
{
	char const *name;

	name = strrchr(path, '/');
	if (!name) {
		name = strrchr(path, '\\'); 	/* eww windows? */
	}
	if (!name) {
		name = path;
	} else {
		name++;
	}

	return name;
}

#ifdef GEN_EXPORTS

/** Returns a file's name without path or extension
 *
 * @param path to check
 * @return pointer in path.
 */
static char const *file_name_stripped(char const *path)
{
	char const *name;
	char const *ext;

	name = file_name(path);
	ext = strrchr(name, '.');

	if (ext) {
		char *trimmed;

		trimmed = lt_malloc(ext - name + 1);
		strncpy(trimmed, name, ext - name);
		trimmed[ext-name] = 0;

		return trimmed;
	}

	return name;
}
#endif

/* version_info is in the form of MAJOR:MINOR:PATCH */
static char const *darwin_dynamic_link_function(char const *version_info)
{
	char *newarg;
	long major, minor, patch;

	major = 0;
	minor = 0;
	patch = 0;

	if (version_info) {
		major = safe_strtol(version_info, &version_info, 10);

		if (version_info) {
			if (version_info[0] == ':') {
				version_info++;
			}

			minor = safe_strtol(version_info, &version_info, 10);

			if (version_info) {
				if (version_info[0] == ':') {
					version_info++;
				}

				patch = safe_strtol(version_info, &version_info, 10);

			}
		}
	}

	/* Avoid -dylib_compatibility_version must be greater than zero errors. */
	if (major == 0) {
		major = 1;
	}
	newarg = (char*)lt_malloc(100);
	snprintf(newarg, 99,
			 "-compatibility_version %ld -current_version %ld.%ld",
			 major, major, minor);

	return newarg;
}


/*
 *	Add a '.libs/' to the buffer.  The caller ensures that
 *	The buffer is large enough to handle 6 extra characters.
 */
static void add_dotlibs(char *buffer)
{
	char *name = strrchr(buffer, '/');

	if (!name) {
		if (!buffer[0]) {
			strcpy(buffer, ".libs/");
			return;
		}
		name = buffer;
	} else {
		name++;
	}
	memmove(name + 6, name, strlen(name));
	memcpy(name, ".libs/", 6);
}

static char *gen_library_name(char const *name, enum lib_type genlib)
{
	char *newarg, *newext;

	newarg = (char *)calloc(strlen(name) + 11, 1);

	if (genlib == TYPE_MODULE_LIB && strncmp(name, "lib", 3) == 0) {
		name += 3;
	}

	if (genlib == TYPE_MODULE_LIB) {
		strcpy(newarg, file_name(name));
	}
	else {
		strcpy(newarg, name);
	}

	newext = strrchr(newarg, '.');
	if (!newext) {
		ERROR("Library path does not have an extension");
	free(newarg);

	return NULL;
	}
	newext++;

	switch (genlib) {
	case TYPE_STATIC_LIB:
		strcpy(newext, STATIC_LIB_EXT);
		break;
	case TYPE_DYNAMIC_LIB:
		strcpy(newext, DYNAMIC_LIB_EXT);
		break;
	case TYPE_MODULE_LIB:
		strcpy(newext, MODULE_LIB_EXT);
		break;

	default:
		break;
	}

	add_dotlibs(newarg);

	return newarg;
}

static char *gen_install_name(char const *name, enum lib_type genlib)
{
	char *newname;
	int rv;
	struct stat sb;

	newname = gen_library_name(name, genlib);
	if (!newname) return NULL;

	/* Check if it exists. If not, return NULL.  */
	rv = stat(newname, &sb);

	if (rv) {
		free(newname);
		return NULL;
	}

	return newname;
}

static char const *check_object_exists(command_t *cmd, char const *arg, int arglen)
{
	char *newarg, *ext;
	struct stat sb;

	newarg = (char *)lt_malloc(arglen + 10);
	memcpy(newarg, arg, arglen);
	newarg[arglen] = 0;
	ext = newarg + arglen;

	strcpy(ext, OBJECT_EXT);

	DEBUG("Checking (obj): %s\n", newarg);
	if (stat(newarg, &sb) == 0) {
		return newarg;
	}

	free(newarg);

	return NULL;
}

/* libdircheck values:
 * 0 - no .libs suffix
 * 1 - .libs suffix
 */
static char *check_library_exists(command_t *cmd, char const *arg, int pathlen,
				  int libdircheck, enum lib_type*libtype)
{
	char *newarg, *ext;
	int pass, rv, newpathlen;

	newarg = (char *)lt_malloc(strlen(arg) + 10);
	strcpy(newarg, arg);
	newarg[pathlen] = '\0';

	newpathlen = pathlen;
	if (libdircheck) {
		add_dotlibs(newarg);
		newpathlen += sizeof(".libs/") - 1;
	}

	strcpy(newarg + newpathlen, arg + pathlen);
	ext = strrchr(newarg, '.');
	if (!ext) {
		ERROR("Error: Library path does not have an extension");
		free(newarg);

		return NULL;
	}
	ext++;

	pass = 0;

	do {
		struct stat sb;

		switch (pass) {
		case 0:
			if (cmd->options.pic_mode != PIC_AVOID &&
				cmd->options.shared != SHARE_STATIC) {
				strcpy(ext, DYNAMIC_LIB_EXT);
				*libtype = TYPE_DYNAMIC_LIB;
				break;
			}
			pass = 1;
			/* Fall through */
		case 1:
			strcpy(ext, STATIC_LIB_EXT);
			*libtype = TYPE_STATIC_LIB;
			break;
		case 2:
			strcpy(ext, MODULE_LIB_EXT);
			*libtype = TYPE_MODULE_LIB;
			break;
		case 3:
			strcpy(ext, OBJECT_EXT);
			*libtype = TYPE_OBJECT;
			break;
		default:
			*libtype = TYPE_UKNOWN;
			break;
		}

		DEBUG("Checking (lib): %s\n", newarg);
		rv = stat(newarg, &sb);
	}
	while (rv != 0 && ++pass < 4);

	if (rv == 0) {
		return newarg;
	}

	free(newarg);

	return NULL;
}

static char * load_install_path(char const *arg)
{
	FILE *f;
	char *path;

	f = fopen(arg,"r");
	if (f == NULL) {
		return NULL;
	}

	path = lt_malloc(PATH_MAX);

	fgets(path, PATH_MAX, f);
	fclose(f);

	if (path[strlen(path)-1] == '\n') {
		path[strlen(path)-1] = '\0';
	}

	/* Check that we have an absolute path.
	 * Otherwise the file could be a GNU libtool file.
	 */
	if (path[0] != '/') {
		free(path);

		return NULL;
	}
	return path;
}

static char * load_noinstall_path(char const *arg, int pathlen)
{
	char *newarg, *expanded_path;
	int newpathlen;

	newarg = (char *)lt_malloc(strlen(arg) + 10);
	strcpy(newarg, arg);
	newarg[pathlen] = 0;

	newpathlen = pathlen;
	strcat(newarg, ".libs");
	newpathlen += sizeof(".libs") - 1;
	newarg[newpathlen] = 0;

#ifdef HAS_REALPATH
	expanded_path = lt_malloc(PATH_MAX);
	expanded_path = realpath(newarg, expanded_path);
	/* Uh, oh.  There was an error.  Fall back on our first guess. */
	if (!expanded_path) {
		expanded_path = newarg;
	}
#else
	/* We might get ../ or something goofy.  Oh, well. */
	expanded_path = newarg;
#endif

	return expanded_path;
}

static void add_dynamic_link_opts(command_t *cmd, count_chars *args)
{
#ifdef DYNAMIC_LINK_OPTS
	if (cmd->options.pic_mode != PIC_AVOID) {
		DEBUG("Adding linker opt: %s\n", DYNAMIC_LINK_OPTS);

		push_count_chars(args, DYNAMIC_LINK_OPTS);
		if (cmd->undefined_flag) {
			push_count_chars(args, "-undefined");
#if defined(__APPLE__)
			/* -undefined dynamic_lookup is used by the bundled Python in
			 * 10.4, but if we don't set MACOSX_DEPLOYMENT_TARGET to 10.3+,
			 * we'll get a linker error if we pass this flag.
			 */
			if (strcasecmp(cmd->undefined_flag, "dynamic_lookup") == 0) {
				insert_count_chars(cmd->program_opts, "MACOSX_DEPLOYMENT_TARGET=10.3", 0);
			}
#endif
			push_count_chars(args, cmd->undefined_flag);
		}
		else {
#ifdef DYNAMIC_LINK_UNDEFINED
			DEBUG("Adding linker opt: %s\n", DYNAMIC_LINK_UNDEFINED);

			push_count_chars(args, DYNAMIC_LINK_UNDEFINED);
#endif
		}
	}
#endif
}

/* Read the final install location and add it to runtime library search path. */
#ifdef RPATH
static void add_rpath(count_chars *cc, char const *path)
{
	int size = 0;
	char *tmp;

#ifdef LINKER_FLAG_PREFIX
	size = strlen(LINKER_FLAG_PREFIX);
#endif
	size = size + strlen(path) + strlen(RPATH) + 2;
	tmp = lt_malloc(size);

#ifdef LINKER_FLAG_PREFIX
	strcpy(tmp, LINKER_FLAG_PREFIX);
	strcat(tmp, RPATH);
#else
	strcpy(tmp, RPATH);
#endif
#ifndef LINKER_FLAG_NO_EQUALS
	strcat(tmp, "=");
#endif
	strcat(tmp, path);

	push_count_chars(cc, tmp);
}

static void add_rpath_file(count_chars *cc, char const *arg)
{
	char const *path;

	path = load_install_path(arg);
	if (path) {
		add_rpath(cc, path);
		lt_const_free(path);
	}
}

static void add_rpath_noinstall(count_chars *cc, char const *arg, int pathlen)
{
	char const *path;

	path = load_noinstall_path(arg, pathlen);
	if (path) {
		add_rpath(cc, path);
		lt_const_free(path);
	}
}
#endif

#ifdef DYNAMIC_LINK_NO_INSTALL
static void add_dylink_noinstall(count_chars *cc, char const *arg, int pathlen,
						  int extlen)
{
	char const *install_path, *current_path, *name;
	char *exp_argument;
	int i_p_len, c_p_len, name_len, dyext_len, cur_len;

	install_path = load_install_path(arg);
	current_path = load_noinstall_path(arg, pathlen);

	if (!install_path || !current_path) {
		return;
	}

	push_count_chars(cc, DYNAMIC_LINK_NO_INSTALL);

	i_p_len = strlen(install_path);
	c_p_len = strlen(current_path);

	name = arg+pathlen;
	name_len = extlen-pathlen;
	dyext_len = sizeof(DYNAMIC_LIB_EXT) - 1;

	/* No, we need to replace the extension. */
	exp_argument = (char *)lt_malloc(i_p_len + c_p_len + (name_len*2) +
								  (dyext_len*2) + 2);

	cur_len = 0;
	strcpy(exp_argument, install_path);
	cur_len += i_p_len;
	exp_argument[cur_len++] = '/';
	strncpy(exp_argument+cur_len, name, extlen-pathlen);
	cur_len += name_len;
	strcpy(exp_argument+cur_len, DYNAMIC_LIB_EXT);
	cur_len += dyext_len;
	exp_argument[cur_len++] = ':';
	strcpy(exp_argument+cur_len, current_path);
	cur_len += c_p_len;
	exp_argument[cur_len++] = '/';
	strncpy(exp_argument+cur_len, name, extlen-pathlen);
	cur_len += name_len;
	strcpy(exp_argument+cur_len, DYNAMIC_LIB_EXT);
	cur_len += dyext_len;

	push_count_chars(cc, exp_argument);
}
#endif

#ifdef ADD_MINUS_L
/* use -L -llibname to allow to use installed libraries */
static void add_minus_l(count_chars *cc, char const *arg)
{
	char *newarg;
	char *name = strrchr(arg, '/');
	char *file = strrchr(arg, '.');

	if ((name != NULL) && (file != NULL) &&
		(strstr(name, "lib") == (name + 1))) {
		*name = '\0';
		*file = '\0';
		file = name;
		file = file+4;
		push_count_chars(cc, "-L");
		push_count_chars(cc, arg);
		/* we need one argument like -lapr-1 */
		newarg = lt_malloc(strlen(file) + 3);
		strcpy(newarg, "-l");
		strcat(newarg, file);
		push_count_chars(cc, newarg);
	} else {
		push_count_chars(cc, arg);
	}
}
#endif

#if 0
static void add_linker_flag_prefix(count_chars *cc, char const *arg)
{
#ifndef LINKER_FLAG_PREFIX
	push_count_chars(cc, arg);
#else
	char *newarg;
	newarg = (char*)lt_malloc(strlen(arg) + sizeof(LINKER_FLAG_PREFIX) + 1);
	strcpy(newarg, LINKER_FLAG_PREFIX);
	strcat(newarg, arg);
	push_count_chars(cc, newarg);
#endif
}
#endif

static int explode_static_lib(command_t *cmd, char const *lib)
{
	count_chars tmpdir_cc, libname_cc;
	char const *tmpdir, *libname;
	char savewd[PATH_MAX];
	char const *name;
	DIR *dir;
	struct dirent *entry;
	char const *lib_args[4];

	/* Bah! */
	if (cmd->options.dry_run) {
		return 0;
	}

	name = file_name(lib);

	init_count_chars(&tmpdir_cc);
	push_count_chars(&tmpdir_cc, ".libs/");
	push_count_chars(&tmpdir_cc, name);
	push_count_chars(&tmpdir_cc, ".exploded/");
	tmpdir = flatten_count_chars(&tmpdir_cc, 0);

	NOTICE("Making: %s\n", tmpdir);

	safe_mkdir(cmd, tmpdir);

	push_count_chars(cmd->tmp_dirs, tmpdir);

	getcwd(savewd, sizeof(savewd));

	if (chdir(tmpdir) != 0) {
		NOTICE("Warning: could not explode %s\n", lib);

		return 1;
	}

	if (lib[0] == '/') {
		libname = lib;
	}
	else {
		init_count_chars(&libname_cc);
		push_count_chars(&libname_cc, "../../");
		push_count_chars(&libname_cc, lib);
		libname = flatten_count_chars(&libname_cc, 0);
	}

	lib_args[0] = LIBRARIAN;
	lib_args[1] = "x";
	lib_args[2] = libname;
	lib_args[3] = NULL;

	external_spawn(cmd, LIBRARIAN, lib_args);

	chdir(savewd);
	dir = opendir(tmpdir);

	while ((entry = readdir(dir)) != NULL) {
#if defined(__APPLE__) && defined(RANLIB)
		/* Apple inserts __.SYMDEF which isn't needed.
		 * Leopard (10.5+) can also add '__.SYMDEF SORTED' which isn't
		 * much fun either.  Just skip them.
		 */
		if (strstr(entry->d_name, "__.SYMDEF") != NULL) {
			continue;
		}
#endif
		if (entry->d_name[0] != '.') {
			push_count_chars(&tmpdir_cc, entry->d_name);
			name = flatten_count_chars(&tmpdir_cc, 0);

			DEBUG("Adding object: %s\n", name);
			push_count_chars(cmd->obj_files, name);
			pop_count_chars(&tmpdir_cc);
		}
	}

	closedir(dir);
	return 0;
}

static int parse_input_file_name(char const *arg, command_t *cmd)
{
	char const *ext = strrchr(arg, '.');
	char const *name;
	int pathlen;
	enum lib_type libtype;
	char const *newarg;

	/* Can't guess the extension */
	if (!ext) {
		return 0;
	}

	ext++;
	name = file_name(arg);
	pathlen = name - arg;

	/*
	 *	Were linking and have an archived object or object file
	 *	push it onto the list of object files which'll get used
	 *	to create the input files list for the linker.
	 *
	 *	We assume that these are outside of the project were building,
	 *	as there's no reason to create .a files as part of the build
	 *	process.
	 */
	if (!strcmp(ext, STATIC_LIB_EXT) && (cmd->mode == MODE_LINK)) {
		struct stat sb;

		if (!stat(arg, &sb)) {
			DEBUG("Adding object: %s\n", arg);

			push_count_chars(cmd->obj_files, arg);

			return 1;
		}
	}

	/*
	 *	More object files, if were linking they get set as input
	 *	files.
	 */
	if (!strcmp(ext, "lo") || !strcmp(ext, OBJECT_EXT)) {
		newarg = check_object_exists(cmd, arg, ext - arg);
		if (!newarg) {
			ERROR("Can not find suitable object file for %s\n", arg);
			exit(1);
		}

		if (cmd->mode == MODE_LINK) {
			DEBUG("Adding object: %s\n", newarg);

			push_count_chars(cmd->obj_files, newarg);
		} else {
			push_count_chars(cmd->arglist, newarg);
		}

		return 1;
	}

	if (!strcmp(ext, "la")) {
		switch (cmd->mode) {
		case MODE_LINK:
			/* Try the .libs dir first! */
			newarg = check_library_exists(cmd, arg, pathlen, 1, &libtype);
			if (!newarg) {
				/* Try the normal dir next. */
				newarg = check_library_exists(cmd, arg, pathlen, 0, &libtype);
				if (!newarg) {
					ERROR("Can not find suitable library for %s\n", arg);
					exit(1);
				}
			}

			/* It is not ok to just add the file: a library may added with:
			   1 - -L path library_name. (For *.so in Linux).
			   2 - library_name.
			 */
#ifdef ADD_MINUS_L
			if (libtype == TYPE_DYNAMIC_LIB) {
				add_minus_l(cmd->shared_opts.dependencies, newarg);
			} else if (cmd->output == OUT_LIB &&
					   libtype == TYPE_STATIC_LIB) {
				explode_static_lib(cmd, newarg);
			} else {
				push_count_chars(cmd->shared_opts.dependencies, newarg);
			}
#else
			if (cmd->output == OUT_LIB && libtype == TYPE_STATIC_LIB) {
				explode_static_lib(cmd, newarg);
			}
			else {
				push_count_chars(cmd->shared_opts.dependencies, newarg);
			}
#endif
			if (libtype == TYPE_DYNAMIC_LIB) {
				if (cmd->options.no_install) {
#ifdef RPATH
					add_rpath_noinstall(cmd->shared_opts.dependencies,
										arg, pathlen);
#endif
				}
				else {
#ifdef RPATH
					add_rpath_file(cmd->shared_opts.dependencies, arg);
#endif
				}
			}
			break;
		case MODE_INSTALL:
			/*
			 *	If we've already recorded a library to
			 *	install, we're most likely getting the .la
			 *	file that we want to install as.
			 *
			 *	The problem is that we need to add it as the
			 *	directory, not the .la file itself.
			 *	Otherwise, we'll do odd things.
			 */
			if (cmd->output == OUT_LIB) {
				char *tmp;

				tmp = strdup(arg);
				tmp[pathlen] = '\0';
				push_count_chars(cmd->arglist, tmp);

			} else {
				cmd->output = OUT_LIB;
				cmd->output_name = arg;
				cmd->static_name.install = gen_install_name(arg, 0);
				cmd->shared_name.install = gen_install_name(arg, 1);
				cmd->module_name.install = gen_install_name(arg, 2);

				if (!cmd->static_name.install &&
					!cmd->shared_name.install &&
					!cmd->module_name.install) {
					ERROR("Files to install do not exist\n");
					exit(1);
				}

			}
			break;
		default:
			break;
		}

		return 1;
	}

	if (!strcmp(ext, "c")) {
		/* If we don't already have an idea what our output name will be. */
		if (!cmd->basename) {
			char *tmp = lt_malloc(strlen(arg) + 4);
			strcpy(tmp, arg);
			strcpy(strrchr(tmp, '.') + 1, "lo");

			cmd->basename = tmp;

			cmd->fake_output_name = strrchr(cmd->basename, '/');
			if (cmd->fake_output_name) {
				cmd->fake_output_name++;
			} else {
				cmd->fake_output_name = cmd->basename;
			}
		}
	}

	return 0;
}

static int parse_output_file_name(char const *arg, command_t *cmd)
{
	char const *name;
	char const *ext;
	char *newarg = NULL;
	int pathlen;

	cmd->fake_output_name = arg;

	name = file_name(arg);
	ext = strrchr(name, '.');

#ifdef EXE_EXT
	if (!ext || strcmp(ext, EXE_EXT) == 0) {
#else
	if (!ext) {
#endif
		cmd->basename = arg;
		cmd->output = OUT_PROGRAM;
#if defined(_OSD_POSIX)
		cmd->options.pic_mode = PIC_AVOID;
#endif
		newarg = (char *)lt_malloc(strlen(arg) + 5);
		strcpy(newarg, arg);
#ifdef EXE_EXT
	if (!ext) {
	  strcat(newarg, EXE_EXT);
	}
#endif
		cmd->output_name = newarg;
		return 1;
	}

	ext++;
	pathlen = name - arg;

	if (strcmp(ext, "la") == 0) {
		assert(cmd->mode == MODE_LINK);

		cmd->basename = arg;
		cmd->static_name.normal = gen_library_name(arg, TYPE_STATIC_LIB);
		cmd->shared_name.normal = gen_library_name(arg, TYPE_DYNAMIC_LIB);
		cmd->module_name.normal = gen_library_name(arg, TYPE_MODULE_LIB);
		cmd->static_name.install = gen_install_name(arg, TYPE_STATIC_LIB);
		cmd->shared_name.install = gen_install_name(arg, TYPE_DYNAMIC_LIB);
		cmd->module_name.install = gen_install_name(arg, TYPE_MODULE_LIB);

		if (!cmd->options.dry_run) {
			char *newname;
			char *newext;
			newname = lt_malloc(strlen(cmd->static_name.normal) + 1);

			strcpy(newname, cmd->static_name.normal);
			newext = strrchr(newname, '/');
			if (!newext) {
				/* Check first to see if the dir already exists! */
				safe_mkdir(cmd, ".libs");
			} else {
				*newext = '\0';
				safe_mkdir(cmd, newname);
			}
			free(newname);
		}

#ifdef TRUNCATE_DLL_NAME
		if (shared) {
			arg = truncate_dll_name(arg);
		}
#endif

		cmd->output_name = arg;
		return 1;
	}

	if (strcmp(ext, STATIC_LIB_EXT) == 0) {
		assert(cmd->mode == MODE_LINK);

		cmd->basename = arg;
		cmd->options.shared = SHARE_STATIC;
		cmd->output = OUT_STATIC_LIB_ONLY;
		cmd->static_name.normal = gen_library_name(arg, TYPE_STATIC_LIB);
		cmd->static_name.install = gen_install_name(arg, TYPE_STATIC_LIB);

		if (!cmd->options.dry_run) {
			char *newname;
			char *newext;
			newname = lt_malloc(strlen(cmd->static_name.normal) + 1);

			strcpy(newname, cmd->static_name.normal);
			newext = strrchr(newname, '/');
			if (!newext) {
				/* Check first to see if the dir already exists! */
				safe_mkdir(cmd, ".libs");
			} else {
				*newext = '\0';
				safe_mkdir(cmd, newname);
			}
			free(newname);
		}

		cmd->output_name = arg;
		return 1;
	}

	if (strcmp(ext, DYNAMIC_LIB_EXT) == 0) {
		assert(cmd->mode == MODE_LINK);

		cmd->basename = arg;
		cmd->options.shared = SHARE_SHARED;
		cmd->output = OUT_DYNAMIC_LIB_ONLY;
		cmd->shared_name.normal = gen_library_name(arg, TYPE_DYNAMIC_LIB);
		cmd->module_name.normal = gen_library_name(arg, TYPE_MODULE_LIB);
		cmd->shared_name.install = gen_install_name(arg, TYPE_DYNAMIC_LIB);
		cmd->module_name.install = gen_install_name(arg, TYPE_MODULE_LIB);

		if (!cmd->options.dry_run) {
			char *newname;
			char *newext;
			newname = lt_malloc(strlen(cmd->shared_name.normal) + 1);

			strcpy(newname, cmd->shared_name.normal);
			newext = strrchr(newname, '/');
			if (!newext) {
				/* Check first to see if the dir already exists! */
				safe_mkdir(cmd, ".libs");
			} else {
				*newext = '\0';
				safe_mkdir(cmd, newname);
			}
			free(newname);
		}

		cmd->output_name = arg;
		return 1;
	}

	if (strcmp(ext, "lo") == 0) {
		char *newext;
		cmd->basename = arg;
		cmd->output = OUT_OBJECT;
		newarg = (char *)lt_malloc(strlen(arg) + 2);
		strcpy(newarg, arg);
		newext = strrchr(newarg, '.') + 1;
		strcpy(newext, OBJECT_EXT);
		cmd->output_name = newarg;
		return 1;
	}

	if (strcmp(ext, DYNAMIC_LIB_EXT) == 0) {
		ERROR("Please build libraries with .la target, not ."
		      DYNAMIC_LIB_EXT "\n");

		exit(1);
	}

	if (strcmp(ext, STATIC_LIB_EXT) == 0) {
		ERROR("Please build libraries with .la target, not ."
		      STATIC_LIB_EXT "\n");

		exit(1);
	}

	return 0;
}

static char const *automode(char const *arg, command_t *cmd)
{
	if (cmd->mode != MODE_UNKNOWN) return arg;

	if (!strcmp(arg, "CC") ||
	    !strcmp(arg, "CXX")) {
		DEBUG("Now in compile mode, guessed from: %s\n", arg);
		arg = CC;
		cmd->mode = MODE_COMPILE;

	} else if (!strcmp(arg, "LINK") ||
		   !strcmp(arg, "LINK.c") ||
		   !strcmp(arg, "LINK.cxx")) {
		DEBUG("Now in linker mode, guessed from: %s\n", arg);
		arg = LINK_C;
		cmd->mode = MODE_LINK;
	}

	return arg;
}


#ifdef GEN_EXPORTS
static void generate_def_file(command_t *cmd)
{
	char def_file[1024];
	char implib_file[1024];
	char *ext;
	FILE *hDef;
	char *export_args[1024];
	int num_export_args = 0;
	char *cmd;
	int cmd_size = 0;
	int a;

	if (cmd->output_name) {
		strcpy(def_file, cmd->output_name);
		strcat(def_file, ".def");
		hDef = fopen(def_file, "w");

		if (hDef != NULL) {
			fprintf(hDef, "LIBRARY '%s' INITINSTANCE\n", file_name_stripped(cmd->output_name));
			fprintf(hDef, "DATA NONSHARED\n");
			fprintf(hDef, "EXPORTS\n");
			fclose(hDef);

			for (a = 0; a < cmd->num_obj_files; a++) {
				cmd_size += strlen(cmd->obj_files[a]) + 1;
			}

			cmd_size += strlen(GEN_EXPORTS) + strlen(def_file) + 3;
			cmd = (char *)lt_malloc(cmd_size);
			strcpy(cmd, GEN_EXPORTS);

			for (a=0; a < cmd->num_obj_files; a++) {
				strcat(cmd, " ");
				strcat(cmd, cmd->obj_files[a] );
			}

			strcat(cmd, ">>");
			strcat(cmd, def_file);
			puts(cmd);
			export_args[num_export_args++] = SHELL_CMD;
			export_args[num_export_args++] = "-c";
			export_args[num_export_args++] = cmd;
			export_args[num_export_args++] = NULL;
			external_spawn(cmd, export_args[0], (char const**)export_args);
			cmd->arglist[cmd->num_args++] = strdup(def_file);

			/* Now make an import library for the dll */
			num_export_args = 0;
			export_args[num_export_args++] = DEF2IMPLIB_CMD;
			export_args[num_export_args++] = "-o";

			strcpy(implib_file, ".libs/");
			strcat(implib_file, cmd->basename);
			ext = strrchr(implib_file, '.');

			if (ext) {
				*ext = '\0';
			}

			strcat(implib_file, ".");
			strcat(implib_file, STATIC_LIB_EXT);

			export_args[num_export_args++] = implib_file;
			export_args[num_export_args++] = def_file;
			export_args[num_export_args++] = NULL;
			external_spawn(cmd, export_args[0], (char const**)export_args);

		}
	}
}
#endif

#if 0
static char const* expand_path(char const *relpath)
{
	char foo[PATH_MAX], *newpath;

	getcwd(foo, PATH_MAX-1);
	newpath = (char*)lt_malloc(strlen(foo)+strlen(relpath)+2);
	strcpy(newpath, foo);
	strcat(newpath, "/");
	strcat(newpath, relpath);
	return newpath;
}
#endif

static void link_fixup(command_t *cmd)
{
	/* If we were passed an -rpath directive, we need to build
	 * shared objects too.  Otherwise, we should only create static
	 * libraries.
	 */
	if (!cmd->install_path && (cmd->output == OUT_DYNAMIC_LIB_ONLY ||
		cmd->output == OUT_MODULE || cmd->output == OUT_LIB)) {
		if (cmd->options.shared == SHARE_SHARED) {
			cmd->install_path = LIBDIR;
		}
	}

	if (cmd->output == OUT_DYNAMIC_LIB_ONLY ||
		cmd->output == OUT_MODULE ||
		cmd->output == OUT_LIB) {

		push_count_chars(cmd->shared_opts.normal, "-o");
		if (cmd->output == OUT_MODULE) {
			push_count_chars(cmd->shared_opts.normal, cmd->module_name.normal);
		} else {
			push_count_chars(cmd->shared_opts.normal, cmd->shared_name.normal);
#ifdef DYNAMIC_INSTALL_NAME
			push_count_chars(cmd->shared_opts.normal, DYNAMIC_INSTALL_NAME);

			if (!cmd->install_path) {
				ERROR("Installation mode requires -rpath\n");
				exit(1);
			}

			{
				char *tmp = lt_malloc(PATH_MAX);
				strcpy(tmp, cmd->install_path);

				if (cmd->shared_name.install) {
					strcat(tmp, strrchr(cmd->shared_name.install, '/'));
				} else {
					strcat(tmp, strrchr(cmd->shared_name.normal, '/'));
				}

				push_count_chars(cmd->shared_opts.normal, tmp);
			}
#endif
		}

		append_count_chars(cmd->shared_opts.normal, cmd->obj_files);
		append_count_chars(cmd->shared_opts.normal, cmd->shared_opts.dependencies);

		if (cmd->options.export_all) {
#ifdef GEN_EXPORTS
			generate_def_file(cmd);
#endif
		}
	}

	if (cmd->output == OUT_LIB || cmd->output == OUT_STATIC_LIB_ONLY) {
		push_count_chars(cmd->static_opts.normal, "-o");
		push_count_chars(cmd->static_opts.normal, cmd->output_name);
	}

	if (cmd->output == OUT_PROGRAM) {
		if (cmd->output_name) {
			push_count_chars(cmd->arglist, "-o");
			push_count_chars(cmd->arglist, cmd->output_name);
			append_count_chars(cmd->arglist, cmd->obj_files);
			append_count_chars(cmd->arglist, cmd->shared_opts.dependencies);
			add_dynamic_link_opts(cmd, cmd->arglist);
		}
	}
}

static void post_parse_fixup(command_t *cmd)
{
	switch (cmd->mode) {
	case MODE_COMPILE:
#ifdef PIC_FLAG
		if (cmd->options.pic_mode != PIC_AVOID) {
			push_count_chars(cmd->arglist, PIC_FLAG);
		}
#endif
		if (cmd->output_name) {
			push_count_chars(cmd->arglist, "-o");
			push_count_chars(cmd->arglist, cmd->output_name);
		}
		break;
	case MODE_LINK:
		link_fixup(cmd);
		break;
	case MODE_INSTALL:
		if (cmd->output == OUT_LIB) {
			link_fixup(cmd);
		}
	default:
		break;
	}

#ifdef USE_OMF
	if (cmd->output == OUT_OBJECT ||
		cmd->output == OUT_PROGRAM ||
		cmd->output == OUT_LIB ||
		cmd->output == OUT_DYNAMIC_LIB_ONLY) {
		push_count_chars(cmd->arglist, "-Zomf");
	}
#endif

	if (cmd->options.shared &&
			(cmd->output == OUT_OBJECT ||
			 cmd->output == OUT_LIB ||
			 cmd->output == OUT_DYNAMIC_LIB_ONLY)) {
#ifdef SHARE_SW
		push_count_chars(cmd->arglist, SHARE_SW);
#endif
	}
}

static int run_mode(command_t *cmd)
{
	int rv = 0;
	count_chars *cctemp;

	cctemp = (count_chars*)lt_malloc(sizeof(count_chars));
	init_count_chars(cctemp);

	switch (cmd->mode) {
	case MODE_COMPILE:
		rv = run_command(cmd, cmd->arglist);
		if (rv) goto finish;
		break;
	case MODE_INSTALL:
		/* Well, we'll assume it's a file going to a directory... */
		/* For brain-dead install-sh based scripts, we have to repeat
		 * the command N-times.  install-sh should die.
		 */
		if (!cmd->output_name) {
			rv = run_command(cmd, cmd->arglist);
			if (rv) goto finish;
		}
		if (cmd->output_name) {
			append_count_chars(cctemp, cmd->arglist);
			insert_count_chars(cctemp,
							   cmd->output_name,
							   cctemp->num - 1);
			rv = run_command(cmd, cctemp);
			if (rv) goto finish;
			clear_count_chars(cctemp);
		}
		if (cmd->static_name.install) {
			append_count_chars(cctemp, cmd->arglist);
			insert_count_chars(cctemp,
							   cmd->static_name.install,
							   cctemp->num - 1);
			rv = run_command(cmd, cctemp);
			if (rv) goto finish;
#if defined(__APPLE__) && defined(RANLIB)
			/* From the Apple libtool(1) manpage on Tiger/10.4:
			 * ----
			 * With  the way libraries used to be created, errors were possible
			 * if the library was modified with ar(1) and  the  table  of
			 * contents  was  not updated  by  rerunning ranlib(1).  Thus the
			 * link editor, ld, warns when the modification date of a library
			 * is more  recent  than  the  creation date  of its table of
			 * contents.  Unfortunately, this means that you get the warning
			 * even if you only copy the library.
			 * ----
			 *
			 * This means that when we install the static archive, we need to
			 * rerun ranlib afterwards.
			 */
			char const *lib_args[3], *static_lib_name;

			{
				char *tmp;
				size_t len1, len2;

				len1 = strlen(cmd->arglist->vals[cmd->arglist->num - 1]);

				static_lib_name = file_name(cmd->static_name.install);
				len2 = strlen(static_lib_name);

				tmp = lt_malloc(len1 + len2 + 2);

				snprintf(tmp, len1 + len2 + 2, "%s/%s",
						cmd->arglist->vals[cmd->arglist->num - 1],
						static_lib_name);

				lib_args[0] = RANLIB;
				lib_args[1] = tmp;
				lib_args[2] = NULL;

				external_spawn(cmd, RANLIB, lib_args);

				free(tmp);
			}
#endif
			clear_count_chars(cctemp);
		}
		if (cmd->shared_name.install) {
			append_count_chars(cctemp, cmd->arglist);
			insert_count_chars(cctemp, cmd->shared_name.install,
					   cctemp->num - 1);
			rv = run_command(cmd, cctemp);
			if (rv) goto finish;
			clear_count_chars(cctemp);
		}
		if (cmd->module_name.install) {
			append_count_chars(cctemp, cmd->arglist);
			insert_count_chars(cctemp, cmd->module_name.install,
					   cctemp->num - 1);
			rv = run_command(cmd, cctemp);
			if (rv) goto finish;
			clear_count_chars(cctemp);
		}
		break;
	case MODE_LINK:
		if (cmd->output == OUT_STATIC_LIB_ONLY ||
			cmd->output == OUT_LIB) {
#ifdef RANLIB
			char const *lib_args[3];
#endif
			/* Removes compiler! */
			cmd->program = LIBRARIAN;
			push_count_chars(cmd->program_opts, LIBRARIAN_OPTS);
			push_count_chars(cmd->program_opts, cmd->static_name.normal);

			rv = run_command(cmd, cmd->obj_files);
			if (rv) goto finish;

#ifdef RANLIB
			lib_args[0] = RANLIB;
			lib_args[1] = cmd->static_name.normal;
			lib_args[2] = NULL;
			external_spawn(cmd, RANLIB, lib_args);
#endif
		}

		if (cmd->output == OUT_DYNAMIC_LIB_ONLY ||
			cmd->output == OUT_MODULE ||
			cmd->output == OUT_LIB) {
			cmd->program = NULL;
			clear_count_chars(cmd->program_opts);

			append_count_chars(cmd->program_opts, cmd->arglist);
			if (cmd->output == OUT_MODULE) {
#ifdef MODULE_OPTS
				push_count_chars(cmd->program_opts, MODULE_OPTS);
#endif
			} else {
#ifdef SHARED_OPTS
				push_count_chars(cmd->program_opts, SHARED_OPTS);
#endif
#ifdef dynamic_link_version_func
				push_count_chars(cmd->program_opts,
						 dynamic_link_version_func(cmd->version_info));
#endif
			}
			add_dynamic_link_opts(cmd, cmd->program_opts);

			rv = run_command(cmd, cmd->shared_opts.normal);
			if (rv) goto finish;
		}
		if (cmd->output == OUT_PROGRAM) {
			rv = run_command(cmd, cmd->arglist);
			if (rv) goto finish;
		}
		break;
	case MODE_EXECUTE:
	{
		char *l, libpath[PATH_MAX];

		if (strlen(cmd->arglist->vals[0]) >= PATH_MAX) {
			ERROR("Libpath too long no buffer space");
			rv = 1;

			goto finish;
		}

		strcpy(libpath, cmd->arglist->vals[0]);
		add_dotlibs(libpath);
	l = strrchr(libpath, '/');
	if (!l) l = strrchr(libpath, '\\');
	if (l) {
		*l = '\0';
		l = libpath;
	} else {
		l = ".libs/";
	}

	l = "./build/lib/.libs";
	setenv(LD_LIBRARY_PATH_LOCAL, l, 1);
	rv = run_command(cmd, cmd->arglist);
		if (rv) goto finish;
	}
	  break;

	default:
		break;
	}

	finish:

	free(cctemp);
	return rv;
}

static void cleanup_tmp_dir(char const *dirname)
{
	DIR *dir;
	struct dirent *entry;
	char fullname[1024];

	dir = opendir(dirname);
	if (!dir) {
		return;
	}

	if ((strlen(dirname) + 1 + sizeof(entry->d_name)) >= sizeof(fullname)) {
		ERROR("Dirname too long, out of buffer space");

		(void) closedir(dir);
		return;
	}

	while ((entry = readdir(dir)) != NULL) {
		if (entry->d_name[0] != '.') {
			strcpy(fullname, dirname);
			strcat(fullname, "/");
			strcat(fullname, entry->d_name);
			(void) remove(fullname);
		}
	}

	rmdir(dirname);

	(void) closedir(dir);
}

static void cleanup_tmp_dirs(command_t *cmd)
{
	int d;

	for (d = 0; d < cmd->tmp_dirs->num; d++) {
		cleanup_tmp_dir(cmd->tmp_dirs->vals[d]);
	}
}

static int ensure_fake_uptodate(command_t *cmd)
{
	/* FIXME: could do the stat/touch here, but nah... */
	char const *touch_args[3];

	if (cmd->mode == MODE_INSTALL) {
		return 0;
	}
	if (!cmd->fake_output_name) {
		return 0;
	}

	touch_args[0] = "touch";
	touch_args[1] = cmd->fake_output_name;
	touch_args[2] = NULL;
	return external_spawn(cmd, "touch", touch_args);
}

/* Store the install path in the *.la file */
static int add_for_runtime(command_t *cmd)
{
	if (cmd->mode == MODE_INSTALL) {
		return 0;
	}
	if (cmd->output == OUT_DYNAMIC_LIB_ONLY ||
		cmd->output == OUT_LIB) {
		FILE *f=fopen(cmd->fake_output_name,"w");
		if (f == NULL) {
			return -1;
		}
		fprintf(f,"%s\n", cmd->install_path);
		fclose(f);
		return(0);
	} else {
		return(ensure_fake_uptodate(cmd));
	}
}

static void parse_args(int argc, char *argv[], command_t *cmd)
{
	int a;
	char const *arg, *base;
	int arg_used;

	/*
	 *	We now take a major step past libtool.
	 *
	 *	IF there's no "--mode=...", AND we recognise
	 *	the binary as a "special" name, THEN replace it
	 * 	with the correct one, and set the correct mode.
	 *
	 *	For example if were called 'CC' then we know we should
	 *	probably be compiling stuff.
	 */
	base = file_name(argv[0]);
	arg = automode(base, cmd);
	if (arg != base) {
		push_count_chars(cmd->arglist, arg);

		assert(cmd->mode != MODE_UNKNOWN);
	}

	/*
	 *	We first pass over the command-line arguments looking for
	 *	"--mode", etc.  If so, then use the libtool compatibility
	 *	method for building the software.  Otherwise, auto-detect it
	 * 	via "-o" and the extensions.
	 */
	base = NULL;
	if (cmd->mode == MODE_UNKNOWN) for (a = 1; a < argc; a++) {
		arg = argv[a];

		if (strncmp(arg, "--mode=", 7) == 0) {
			base = NULL;
			break;
		}

		/*
		 *	Stop if we get another magic method
		 */
		if ((a == 1) &&
		    ((strncmp(arg, "LINK", 4) == 0) ||
		     (strcmp(arg, "CC") == 0) ||
		     (strcmp(arg, "CXX") == 0))) {
			base = NULL;
			break;
		}

		if (strncmp(arg, "-o", 2) == 0) {
			base = argv[++a];
		}
	}

	/*
	 *	There were no magic args or an explicit --mode= but we did
	 *	find an output file, so guess what mode were meant to be in
	 *	from its extension.
	 */
	if (base) {
		arg = strrchr(base, '.');
		if (!arg) {
			cmd->mode = MODE_LINK;
			push_count_chars(cmd->arglist, LINK_C);
		}
#ifdef EXE_EXT
		else if (strcmp(arg, EXE_EXT) == 0) {
			cmd->mode = MODE_LINK;
			push_count_chars(cmd->arglist, LINK_C);
		}
#endif
		else if (strcmp(arg + 1, DYNAMIC_LIB_EXT) == 0) {
			cmd->mode = MODE_LINK;
			push_count_chars(cmd->arglist, LINK_C);
		}
		else if (strcmp(arg + 1, STATIC_LIB_EXT) == 0) {
			cmd->mode = MODE_LINK;
			push_count_chars(cmd->arglist, LINK_C);
		}
		else if (strcmp(arg + 1, "la") == 0) {
			cmd->mode = MODE_LINK;
			push_count_chars(cmd->arglist, LINK_C);
		}
		else if ((strcmp(arg + 1, "lo") == 0) ||
			 (strcmp(arg + 1, "o") == 0)) {
			cmd->mode = MODE_COMPILE;
			push_count_chars(cmd->arglist, CC);
		}
	}

	for (a = 1; a < argc; a++) {
		arg = argv[a];
		arg_used = 1;

		if (arg[0] == '-') {
			/*
			 *	Double dashed (long) single dash (short)
			 */
			arg_used = (arg[1] == '-') ?
				parse_long_opt(arg + 2, cmd) :
				parse_short_opt(arg + 1, cmd);

			if (arg_used) continue;

			/*
			 *	We haven't done anything with it yet, but
			 *	there are still some arg/value pairs.
			 *
			 *	Try some of the more complicated short opts...
			 */
			if (a + 1 < argc) {
				/*
				 *	We found an output file!
				 */
				if ((arg[1] == 'o') && (arg[2] == '\0')) {
					arg = argv[++a];
					arg_used = parse_output_file_name(arg,
									  cmd);
				/*
				 *	-MT literal dependency
				 */
				} else if (!strcmp(arg + 1, "MT")) {
					DEBUG("Adding: %s\n", arg);

					push_count_chars(cmd->arglist, arg);
					arg = argv[++a];

					NOTICE(" %s\n", arg);

					push_count_chars(cmd->arglist, arg);
					arg_used = 1;
				/*
				 *	Runtime library search path
				 */
				} else if (!strcmp(arg + 1, "rpath")) {
					/* Aha, we should try to link both! */
					cmd->install_path = argv[++a];
					arg_used = 1;

				} else if (!strcmp(arg + 1, "release")) {
					/* Store for later deciphering */
					cmd->version_info = argv[++a];
					arg_used = 1;

				} else if (!strcmp(arg + 1, "version-info")) {
					/* Store for later deciphering */
					cmd->version_info = argv[++a];
					arg_used = 1;

				} else if (!strcmp(arg + 1,
						   "export-symbols-regex")) {
					/* Skip the argument. */
					++a;
					arg_used = 1;

				} else if (!strcmp(arg + 1, "undefined")) {
					cmd->undefined_flag = argv[++a];
					arg_used = 1;
				/*
				 *	Add dir to runtime library search path.
				 */
				} else if ((arg[1] == 'R') && !arg[2]) {

					add_runtime_dir_lib(argv[++a], cmd);
					arg_used = 1;
				}
			}
		/*
		 *	Ok.. the argument doesn't begin with a dash
		 *	maybe it's an input file.
		 *
		 *	Check its extension to see if it's a known input
		 *	file and verify it exists.
		 */
		} else {
			arg_used = parse_input_file_name(arg, cmd);
		}

		/*
		 *	If we still don't have a run mode, look for a magic
		 *	program name CC, LINK, or whatever.  Then replace that
		 *	with the name of the real program we want to run.
		 */
		if (!arg_used) {
			if ((cmd->arglist->num == 0) &&
				(cmd->mode == MODE_UNKNOWN)) {
				arg = automode(arg, cmd);
			}

			DEBUG("Adding: %s\n", arg);

			push_count_chars(cmd->arglist, arg);
		}
	}

}

int main(int argc, char *argv[])
{
	int rc;
	command_t cmd;

	memset(&cmd, 0, sizeof(cmd));

	cmd.options.pic_mode = PIC_UNKNOWN;
	cmd.mode = MODE_UNKNOWN;
	cmd.output = OUT_GENERAL;

	/*
	 *	Initialise the various argument lists
	 */
	cmd.program_opts		= alloc_countchars();
	cmd.arglist			= alloc_countchars();
	cmd.tmp_dirs 			= alloc_countchars();
	cmd.obj_files			= alloc_countchars();
	cmd.dep_rpaths 			= alloc_countchars();
	cmd.rpaths			= alloc_countchars();
	cmd.static_opts.normal		= alloc_countchars();
	cmd.shared_opts.normal		= alloc_countchars();
	cmd.shared_opts.dependencies	= alloc_countchars();

	/*
	 *	Fill up the various argument lists
	 */
	parse_args(argc, argv, &cmd);
	post_parse_fixup(&cmd);

	/*
	 *	We couldn't figure out which mode to operate in
	 */
	if (cmd.mode == MODE_UNKNOWN) {
		usage(1);
	}

	rc = run_mode(&cmd);
	if (!rc) {
		add_for_runtime(&cmd);
	}

	cleanup_tmp_dirs(&cmd);

	return rc;
}
