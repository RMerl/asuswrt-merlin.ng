/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2004-2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <unistd.h>
#include <inttypes.h>

#ifdef HAVE_BACKTRACE_SUPPORT
#include <execinfo.h>
#include <elfutils/libdwfl.h>
#endif

#include "src/log.h"
#include "src/backtrace.h"

void btd_backtrace_init(void)
{
#ifdef HAVE_BACKTRACE_SUPPORT
	void *frames[1];

	/*
	 * initialize the backtracer, since the ctor calls dlopen(), which
	 * calls malloc(), which isn't signal-safe.
	 */
	backtrace(frames, 1);
#endif
}

void btd_backtrace(uint16_t index)
{
#ifdef HAVE_BACKTRACE_SUPPORT
	char *debuginfo_path = NULL;
	const Dwfl_Callbacks callbacks = {
		.find_debuginfo = dwfl_standard_find_debuginfo,
		.find_elf = dwfl_linux_proc_find_elf,
		.debuginfo_path = &debuginfo_path,
	};
	Dwfl *dwfl;
	void *frames[48];
	int n, n_ptrs;

	dwfl = dwfl_begin(&callbacks);

	if (dwfl_linux_proc_report(dwfl, getpid()))
		goto done;

	dwfl_report_end(dwfl, NULL, NULL);

	n_ptrs = backtrace(frames, 48);
	if (n_ptrs < 1)
		goto done;

	btd_error(index, "++++++++ backtrace ++++++++");

	for (n = 1; n < n_ptrs; n++) {
		GElf_Addr addr = (uintptr_t) frames[n];
		GElf_Sym sym;
		GElf_Word shndx;
		Dwfl_Module *module = dwfl_addrmodule(dwfl, addr);
		Dwfl_Line *line;
		const char *name, *modname;

		if (!module) {
			btd_error(index, "#%-2u ?? [%#" PRIx64 "]", n, addr);
			continue;
		}

		name = dwfl_module_addrsym(module, addr, &sym, &shndx);
		if (!name) {
			modname = dwfl_module_info(module, NULL, NULL, NULL,
							NULL, NULL, NULL, NULL);
			btd_error(index, "#%-2u ?? (%s) [%#" PRIx64 "]",
							n, modname, addr);
			continue;
		}

		line = dwfl_module_getsrc(module, addr);
		if (line) {
			int lineno;
			const char *src = dwfl_lineinfo(line, NULL, &lineno,
							NULL, NULL, NULL);

			if (src) {
				btd_error(index, "#%-2u %s+%#" PRIx64 " "
						"(%s:%d) [%#" PRIx64 "]",
						n, name, addr - sym.st_value,
							src, lineno, addr);
				continue;
			}
		}

		modname = dwfl_module_info(module, NULL, NULL, NULL,
						NULL, NULL, NULL, NULL);
		btd_error(index, "#%-2u %s+%#" PRIx64 " (%s) [%#" PRIx64 "]",
						n, name, addr - sym.st_value,
								modname, addr);
	}

	btd_error(index, "+++++++++++++++++++++++++++");

done:
	dwfl_end(dwfl);
#endif
}

void btd_assertion_message_expr(const char *file, int line,
					const char *func, const char *expr)
{
	btd_error(0xffff, "Assertion failed: (%s) %s:%d in %s",
						expr, file, line, func);
	btd_backtrace(0xffff);
}
