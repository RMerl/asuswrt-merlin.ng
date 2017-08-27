/*
 * Copyright (C) 2006-2013 Martin Willi
 * Hochschule fuer Technik Rapperswil
 * Copyright (C) 2013 revosec AG
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

#define _GNU_SOURCE

#ifdef HAVE_BACKTRACE
# include <execinfo.h>
#endif /* HAVE_BACKTRACE */
#ifdef HAVE_DBGHELP
# include <winsock2.h>
# include <windows.h>
# include <dbghelp.h>
#endif /* HAVE_DBGHELP */
#include <string.h>

#include "backtrace.h"

#include <utils/debug.h>

#ifdef WIN32
# include <psapi.h>
/* missing in MinGW */
#ifdef WIN64
#ifndef GetModuleInformation
WINBOOL K32GetModuleInformation(HANDLE hProcess, HMODULE hModule,
								LPMODULEINFO lpmodinfo, DWORD cb);
#define GetModuleInformation K32GetModuleInformation
#endif /* !GetModuleInformation */
#ifndef GetModuleFileNameEx
DWORD K32GetModuleFileNameExA(HANDLE hProcess, HMODULE hModule,
							  LPTSTR lpFilename, DWORD nSize);
#define GetModuleFileNameEx K32GetModuleFileNameExA
#endif /* !GetModuleFileNameEx */
#endif /* WIN64 */
#endif

typedef struct private_backtrace_t private_backtrace_t;

/**
 * Private data of an backtrace_t object.
 */
struct private_backtrace_t {

	/**
	 * Public backtrace_t interface.
	 */
	backtrace_t public;

	/**
	 * Number of stacks frames obtained in stack_frames
	 */
	int frame_count;

	/**
	 * Recorded stack frames.
	 */
	void *frames[];
};

/**
 * Forward declaration of method getter
 */
static backtrace_t get_methods();

/**
 * Write a format string with arguments to a FILE line, if it is NULL to DBG
 */
static void println(FILE *file, char *format, ...)
{
	char buf[512];
	va_list args;

	va_start(args, format);
	if (file)
	{
		vfprintf(file, format, args);
		fputs("\n", file);
	}
	else
	{
		vsnprintf(buf, sizeof(buf), format, args);
		DBG1(DBG_LIB, "%s", buf);
	}
	va_end(args);
}

/**
 * Same as tty_escape_get(), but for a potentially NULL FILE*
 */
static inline char* esc(FILE *file, tty_escape_t escape)
{
	if (file)
	{
		return tty_escape_get(fileno(file), escape);
	}
	return "";
}

#ifdef HAVE_DBGHELP

#include <dbghelp.h>
#include <threading/mutex.h>

/**
 * Mutex to access non-thread-safe dbghelp functions
 */
static mutex_t *dbghelp_mutex;

void backtrace_init()
{
	SymSetOptions(SYMOPT_LOAD_LINES);
	SymInitialize(GetCurrentProcess(), NULL, TRUE);
	dbghelp_mutex = mutex_create(MUTEX_TYPE_DEFAULT);
}

void backtrace_deinit()
{
	dbghelp_mutex->destroy(dbghelp_mutex);
	SymCleanup(GetCurrentProcess());
}

#elif defined(HAVE_DLADDR) || defined(HAVE_BFD_H)

#ifdef HAVE_DLADDR
#include <dlfcn.h>
#endif

#ifdef HAVE_BFD_H

#include <bfd.h>
#include <collections/hashtable.h>
#include <threading/mutex.h>

/**
 * Hashtable-cached bfd handle
 */
typedef struct {
	/** binary file name on disk */
	char *filename;
	/** bfd handle */
	bfd *abfd;
	/** loaded symbols */
	asymbol **syms;
} bfd_entry_t;

/**
 * Destroy a bfd_entry
 */
static void bfd_entry_destroy(bfd_entry_t *this)
{
	free(this->filename);
	free(this->syms);
	bfd_close(this->abfd);
	free(this);
}

/**
 * Data to pass to find_addr()
 */
typedef struct {
	/** used bfd entry */
	bfd_entry_t *entry;
	/** backtrace address */
	bfd_vma vma;
	/** stream to log to */
	FILE *file;
	/** TRUE if complete */
	bool found;
} bfd_find_data_t;

/**
 * bfd entry cache
 */
static hashtable_t *bfds;

static mutex_t *bfd_mutex;

/**
 * Hashtable hash function
 */
static u_int bfd_hash(char *key)
{
	return chunk_hash(chunk_create(key, strlen(key)));
}

/**
 * Hashtable equals function
 */
static bool bfd_equals(char *a, char *b)
{
	return streq(a, b);
}

/**
 * See header.
 */
void backtrace_init()
{
	bfd_init();
	bfds = hashtable_create((hashtable_hash_t)bfd_hash,
							(hashtable_equals_t)bfd_equals, 8);
	bfd_mutex = mutex_create(MUTEX_TYPE_DEFAULT);
}

/**
 * See header.
 */
void backtrace_deinit()
{
	enumerator_t *enumerator;
	bfd_entry_t *entry;
	char *key;

	enumerator = bfds->create_enumerator(bfds);
	while (enumerator->enumerate(enumerator, &key, &entry))
	{
		bfds->remove_at(bfds, enumerator);
		bfd_entry_destroy(entry);
	}
	enumerator->destroy(enumerator);

	bfds->destroy(bfds);
	bfd_mutex->destroy(bfd_mutex);
}

/**
 * Find and print information to an address
 */
static void find_addr(bfd *abfd, asection *section, bfd_find_data_t *data)
{
	bfd_size_type size;
	bfd_vma vma;
	const char *source;
	const char *function;
	char fbuf[512] = "", sbuf[512] = "";
	u_int line;

	if (!data->found || (bfd_get_section_flags(abfd, section) & SEC_ALLOC) != 0)
	{
		vma = bfd_get_section_vma(abfd, section);
		if (data->vma >= vma)
		{
			size = bfd_get_section_size(section);
			if (data->vma < vma + size)
			{
				data->found = bfd_find_nearest_line(abfd, section,
											data->entry->syms, data->vma - vma,
											&source, &function, &line);
				if (data->found)
				{
					if (source || function)
					{
						if (function)
						{
							snprintf(fbuf, sizeof(fbuf), "%s%s() ",
								esc(data->file, TTY_FG_BLUE), function);
						}
						if (source)
						{
							snprintf(sbuf, sizeof(sbuf), "%s@ %s:%d",
								esc(data->file, TTY_FG_GREEN), source, line);
						}
						println(data->file, "    -> %s%s%s", fbuf, sbuf,
								esc(data->file, TTY_FG_DEF));
					}
				}
			}
		}
	}
}

/**
 * Find a cached bfd entry, create'n'cache if not found
 */
static bfd_entry_t *get_bfd_entry(char *filename)
{
	bool dynamic = FALSE, ok = FALSE;
	bfd_entry_t *entry;
	long size;

	/* check cache */
	entry = bfds->get(bfds, filename);
	if (entry)
	{
		return entry;
	}

	INIT(entry,
		.abfd = bfd_openr(filename, NULL),
	);

	if (!entry->abfd)
	{
		free(entry);
		return NULL;
	}
#ifdef BFD_DECOMPRESS
	entry->abfd->flags |= BFD_DECOMPRESS;
#endif
	if (bfd_check_format(entry->abfd, bfd_archive) == 0 &&
		bfd_check_format_matches(entry->abfd, bfd_object, NULL))
	{
		if (bfd_get_file_flags(entry->abfd) & HAS_SYMS)
		{
			size = bfd_get_symtab_upper_bound(entry->abfd);
			if (size == 0)
			{
				size = bfd_get_dynamic_symtab_upper_bound(entry->abfd);
				dynamic = TRUE;
			}
			if (size >= 0)
			{
				entry->syms = malloc(size);
				if (dynamic)
				{
					ok = bfd_canonicalize_dynamic_symtab(entry->abfd,
														 entry->syms) >= 0;
				}
				else
				{
					ok = bfd_canonicalize_symtab(entry->abfd,
												 entry->syms) >= 0;
				}
			}
		}
	}
	if (ok)
	{
		entry->filename = strdup(filename);
		bfds->put(bfds, entry->filename, entry);
		return entry;
	}
	bfd_entry_destroy(entry);
	return NULL;
}

/**
 * Print the source file with line number to file, libbfd variant
 */
static void print_sourceline(FILE *file, char *filename, void *ptr, void *base)
{
	bfd_entry_t *entry;
	bfd_find_data_t data = {
		.file = file,
		.vma = (uintptr_t)ptr,
	};
	bool old = FALSE;

	bfd_mutex->lock(bfd_mutex);
	if (lib && lib->leak_detective)
	{
		old = lib->leak_detective->set_state(lib->leak_detective, FALSE);
	}
	entry = get_bfd_entry(filename);
	if (entry)
	{
		data.entry = entry;
		bfd_map_over_sections(entry->abfd, (void*)find_addr, &data);
	}
	if (lib && lib->leak_detective)
	{
		lib->leak_detective->set_state(lib->leak_detective, old);
	}
	bfd_mutex->unlock(bfd_mutex);
}

#else /* !HAVE_BFD_H */

void backtrace_init() {}
void backtrace_deinit() {}

/**
 * Print the source file with line number to file, slow addr2line variant
 */
static void print_sourceline(FILE *file, char *filename, void *ptr, void* base)
{
	char buf[1024];
	FILE *output;
	int c, i = 0;

#ifdef __APPLE__
	snprintf(buf, sizeof(buf), "atos -o %s -l %p %p 2>&1 | tail -n1",
			 filename, base, ptr);
#else /* !__APPLE__ */
	snprintf(buf, sizeof(buf), "addr2line -e %s %p", filename, ptr);
#endif /* __APPLE__ */

	output = popen(buf, "r");
	if (output)
	{
		while (i < sizeof(buf))
		{
			c = getc(output);
			if (c == '\n' || c == EOF)
			{
				buf[i++] = 0;
				break;
			}
			buf[i++] = c;
		}
		pclose(output);

		println(file, "    -> %s%s%s", esc(file, TTY_FG_GREEN), buf,
				esc(file, TTY_FG_DEF));
	}
}

#endif /* HAVE_BFD_H */

#else /* !HAVE_DLADDR && !HAVE_DBGHELP */

void backtrace_init() {}
void backtrace_deinit() {}

#endif /* HAVE_DLADDR */

METHOD(backtrace_t, log_, void,
	private_backtrace_t *this, FILE *file, bool detailed)
{
#if defined(HAVE_BACKTRACE) || defined(HAVE_LIBUNWIND_H) || defined(WIN32)
	size_t i;
	char **strings = NULL;

	println(file, " dumping %d stack frame addresses:", this->frame_count);
	for (i = 0; i < this->frame_count; i++)
	{
#ifdef HAVE_DLADDR
		Dl_info info;

		if (dladdr(this->frames[i], &info))
		{
			void *ptr = this->frames[i];

			if (strstr(info.dli_fname, ".so"))
			{
				ptr = (void*)(this->frames[i] - info.dli_fbase);
			}
			if (info.dli_sname)
			{
				println(file, "  %s%s%s @ %p (%s%s%s+0x%tx) [%p]",
						esc(file, TTY_FG_YELLOW), info.dli_fname,
						esc(file, TTY_FG_DEF), info.dli_fbase,
						esc(file, TTY_FG_RED), info.dli_sname,
						esc(file, TTY_FG_DEF), this->frames[i] - info.dli_saddr,
						this->frames[i]);
			}
			else
			{
				println(file, "  %s%s%s @ %p [%p]",
						esc(file, TTY_FG_YELLOW), info.dli_fname,
						esc(file, TTY_FG_DEF), info.dli_fbase, this->frames[i]);
			}
			if (detailed && info.dli_fname[0])
			{
				print_sourceline(file, (char*)info.dli_fname,
								 ptr, info.dli_fbase);
			}
		}
		else
#elif defined(HAVE_DBGHELP)
		struct {
			SYMBOL_INFO hdr;
			char buf[128];
		} symbol;
		char filename[MAX_PATH];
		HINSTANCE module;
		HANDLE process;
		DWORD64 displace, frame;

		process = GetCurrentProcess();
		frame = (uintptr_t)this->frames[i];

		memset(&symbol, 0, sizeof(symbol));
		symbol.hdr.SizeOfStruct = sizeof(symbol.hdr);
		symbol.hdr.MaxNameLen = sizeof(symbol.buf) - 1;

		dbghelp_mutex->lock(dbghelp_mutex);

		module = (HINSTANCE)SymGetModuleBase64(process, frame);

		if (module && GetModuleFileName(module, filename, sizeof(filename)))
		{
			if (SymFromAddr(process, frame, &displace, &symbol.hdr))
			{
				println(file, "  %s%s%s @ %p (%s%s%s+0x%tx) [%p]",
						esc(file, TTY_FG_YELLOW), filename,
						esc(file, TTY_FG_DEF), (void*)module,
						esc(file, TTY_FG_RED), symbol.hdr.Name,
						esc(file, TTY_FG_DEF), displace,
						this->frames[i]);
			}
			else
			{
				println(file, "  %s%s%s @ %p [%p]",
						esc(file, TTY_FG_YELLOW), filename,
						esc(file, TTY_FG_DEF), (void*)module, this->frames[i]);
			}
			if (detailed)
			{
				IMAGEHLP_LINE64 line;
				DWORD off;

				memset(&line, 0, sizeof(line));
				line.SizeOfStruct = sizeof(line);

				if (SymGetLineFromAddr64(process, frame, &off, &line))
				{

					println(file, "    -> %s%s:%u%s", esc(file, TTY_FG_GREEN),
							line.FileName, line.LineNumber,
							esc(file, TTY_FG_DEF));
				}
			}
		}
		else
#elif defined(WIN32)
		HMODULE module;
		MODULEINFO info;
		char filename[MAX_PATH];

		if (GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
						this->frames[i], &module) &&
			GetModuleInformation(GetCurrentProcess(), module,
						&info, sizeof(info)) &&
			GetModuleFileNameEx(GetCurrentProcess(), module,
						filename, sizeof(filename)))
		{
			println(file, "  %s%s%s @ %p [%p]",
					esc(file, TTY_FG_YELLOW), filename,
					esc(file, TTY_FG_DEF), info.lpBaseOfDll, this->frames[i]);
#ifdef HAVE_BFD_H
			print_sourceline(file, filename, this->frames[i], info.lpBaseOfDll);
#endif /* HAVE_BFD_H */
		}
		else
#endif /* HAVE_DLADDR/HAVE_DBGHELP */
		{
#ifdef HAVE_BACKTRACE
			if (!strings)
			{
				strings = backtrace_symbols(this->frames, this->frame_count);
			}
			if (strings)
			{
				println(file, "    %s", strings[i]);
			}
			else
#endif /* HAVE_BACKTRACE */
			{
				println(file, "    %p", this->frames[i]);
			}
		}
#ifdef HAVE_DBGHELP
		dbghelp_mutex->unlock(dbghelp_mutex);
#endif
	}
	free(strings);
#else /* !HAVE_BACKTRACE && !HAVE_LIBUNWIND_H */
	println(file, "no support for capturing backtraces");
#endif /* HAVE_BACKTRACE/HAVE_LIBUNWIND_H */
}

METHOD(backtrace_t, contains_function, bool,
	private_backtrace_t *this, char *function[], int count)
{
#ifdef HAVE_DLADDR
	int i, j;

	for (i = 0; i< this->frame_count; i++)
	{
		Dl_info info;

		if (dladdr(this->frames[i], &info) && info.dli_sname)
		{
			for (j = 0; j < count; j++)
			{
				if (streq(info.dli_sname, function[j]))
				{
					return TRUE;
				}
			}
		}
	}
#elif defined(HAVE_DBGHELP)
	int i, j;
	HANDLE process;

	process = GetCurrentProcess();

	dbghelp_mutex->lock(dbghelp_mutex);

	for (i = 0; i < this->frame_count; i++)
	{
		struct {
			SYMBOL_INFO hdr;
			char buf[128];
		} symbol;

		memset(&symbol, 0, sizeof(symbol));
		symbol.hdr.SizeOfStruct = sizeof(symbol.hdr);
		symbol.hdr.MaxNameLen = sizeof(symbol.buf) - 1;

		if (SymFromAddr(process, (DWORD64)this->frames[i], NULL, &symbol.hdr))
		{
			for (j = 0; j < count; j++)
			{
				if (streq(symbol.hdr.Name, function[j]))
				{
					dbghelp_mutex->unlock(dbghelp_mutex);
					return TRUE;
				}
			}
		}
	}

	dbghelp_mutex->unlock(dbghelp_mutex);
#endif /* HAVE_DLADDR/HAVE_DBGHELP */
	return FALSE;
}

METHOD(backtrace_t, equals, bool,
	private_backtrace_t *this, backtrace_t *other_public)
{
	private_backtrace_t *other = (private_backtrace_t*)other_public;
	int i;

	if (this == other)
	{
		return TRUE;
	}
	if (this->frame_count != other->frame_count)
	{
		return FALSE;
	}
	for (i = 0; i < this->frame_count; i++)
	{
		if (this->frames[i] != other->frames[i])
		{
			return FALSE;
		}
	}
	return TRUE;
}

/**
 * Frame enumerator
 */
typedef struct {
	/** implements enumerator_t */
	enumerator_t public;
	/** reference to backtrace */
	private_backtrace_t *bt;
	/** current position */
	int i;
} frame_enumerator_t;

METHOD(enumerator_t, frame_enumerate, bool,
	frame_enumerator_t *this, void **addr)
{
	if (this->i < this->bt->frame_count)
	{
		*addr = this->bt->frames[this->i++];
		return TRUE;
	}
	return FALSE;
}

METHOD(backtrace_t, create_frame_enumerator, enumerator_t*,
	private_backtrace_t *this)
{
	frame_enumerator_t *enumerator;

	INIT(enumerator,
		.public = {
			.enumerate = (void*)_frame_enumerate,
			.destroy = (void*)free,
		},
		.bt = this,
	);
	return &enumerator->public;
}

METHOD(backtrace_t, clone_, backtrace_t*,
	private_backtrace_t *this)
{
	private_backtrace_t *clone;

	clone = malloc(sizeof(private_backtrace_t) +
				   this->frame_count * sizeof(void*));
	memcpy(clone->frames, this->frames, this->frame_count * sizeof(void*));
	clone->frame_count = this->frame_count;

	clone->public = get_methods();

	return &clone->public;
}

METHOD(backtrace_t, destroy, void,
	private_backtrace_t *this)
{
	free(this);
}

#ifdef HAVE_LIBUNWIND_H
#define UNW_LOCAL_ONLY
#include <libunwind.h>

/**
 * libunwind variant for glibc backtrace()
 */
static inline int backtrace_unwind(void **frames, int count)
{
	unw_context_t context;
	unw_cursor_t cursor;
	unw_word_t ip;
	int depth = 0;

	unw_getcontext(&context);
	unw_init_local(&cursor, &context);
	do
	{
		unw_get_reg(&cursor, UNW_REG_IP, &ip);
		frames[depth++] = (void*)ip;
	}
	while (depth < count && unw_step(&cursor) > 0);

	return depth;
}
#endif /* HAVE_UNWIND */

#ifdef HAVE_DBGHELP

/**
 * Windows dbghelp variant for glibc backtrace()
 */
static inline int backtrace_win(void **frames, int count)
{
	STACKFRAME frame;
	HANDLE process, thread;
	DWORD machine;
	CONTEXT context;
	int got = 0;

	memset(&frame, 0, sizeof(frame));
	memset(&context, 0, sizeof(context));

	process = GetCurrentProcess();
	thread = GetCurrentThread();

#ifdef __x86_64
	machine = IMAGE_FILE_MACHINE_AMD64;

	frame.AddrPC.Offset = context.Rip;
	frame.AddrPC.Mode = AddrModeFlat;
	frame.AddrStack.Offset = context.Rsp;
	frame.AddrStack.Mode = AddrModeFlat;
	frame.AddrFrame.Offset = context.Rbp;
	frame.AddrFrame.Mode = AddrModeFlat;
#else /* x86 */
	machine = IMAGE_FILE_MACHINE_I386;

	frame.AddrPC.Offset = context.Eip;
	frame.AddrPC.Mode = AddrModeFlat;
	frame.AddrStack.Offset = context.Esp;
	frame.AddrStack.Mode = AddrModeFlat;
	frame.AddrFrame.Offset = context.Ebp;
	frame.AddrFrame.Mode = AddrModeFlat;
#endif /* x86_64/x86 */

	dbghelp_mutex->lock(dbghelp_mutex);

	RtlCaptureContext(&context);

	while (got < count)
	{
		if (!StackWalk64(machine, process, thread, &frame, &context,
						 NULL, SymFunctionTableAccess, SymGetModuleBase, NULL))
		{
			break;
		}
		frames[got++] = (void*)frame.AddrPC.Offset;
	}

	dbghelp_mutex->unlock(dbghelp_mutex);

	return got;
}

#endif /* HAVE_DBGHELP */

/**
 * Get implementation methods of backtrace_t
 */
static backtrace_t get_methods()
{
	return (backtrace_t) {
		.log = _log_,
		.contains_function = _contains_function,
		.equals = _equals,
		.clone = _clone_,
		.create_frame_enumerator = _create_frame_enumerator,
		.destroy = _destroy,
	};
}

/**
 * See header
 */
backtrace_t *backtrace_create(int skip)
{
	private_backtrace_t *this;
	void *frames[50];
	int frame_count = 0;

#ifdef HAVE_LIBUNWIND_H
	frame_count = backtrace_unwind(frames, countof(frames));
#elif defined(HAVE_BACKTRACE)
	frame_count = backtrace(frames, countof(frames));
#elif defined(HAVE_DBGHELP)
	frame_count = backtrace_win(frames, countof(frames));
#elif defined(WIN32)
	frame_count = CaptureStackBackTrace(skip, countof(frames), frames, NULL);
	skip = 0;
#endif
	frame_count = max(frame_count - skip, 0);
	this = malloc(sizeof(private_backtrace_t) + frame_count * sizeof(void*));
	memcpy(this->frames, frames + skip, frame_count * sizeof(void*));
	this->frame_count = frame_count;

	this->public = get_methods();

	return &this->public;
}

/**
 * See header
 */
void backtrace_dump(char *label, FILE *file, bool detailed)
{
	backtrace_t *backtrace;

	backtrace = backtrace_create(2);

	if (label)
	{
		println(file, "Debug backtrace: %s", label);
	}
	backtrace->log(backtrace, file, detailed);
	backtrace->destroy(backtrace);
}
