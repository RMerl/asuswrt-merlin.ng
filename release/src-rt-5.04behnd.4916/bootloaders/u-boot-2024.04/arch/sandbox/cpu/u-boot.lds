/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2011-2012 The Chromium OS Authors.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

SECTIONS
{

	. = ALIGN(32);
	__u_boot_list : {
		KEEP(*(SORT(__u_boot_list*)));
	}

	_u_boot_sandbox_getopt : {
		*(_u_boot_sandbox_getopt_start)
		KEEP(*(_u_boot_sandbox_getopt))
		*(_u_boot_sandbox_getopt_end)
	}

	efi_runtime_start : {
		*(___efi_runtime_start)
	}

	efi_runtime : {
		*(efi_runtime_text)
		*(efi_runtime_data)
	}

	efi_runtime_stop : {
		*(___efi_runtime_stop)
	}

	efi_runtime_rel_start : {
		*(___efi_runtime_rel_start)
	}

	efi_runtime_rel : {
		*(.relefi_runtime_text)
		*(.relefi_runtime_data)
	}

	efi_runtime_rel_stop : {
		*(___efi_runtime_rel_stop)
	}

	.dynsym :
	{
		__dyn_sym_start = .;
		*(.dynsym)
		__dyn_sym_end = .;
	}
}

INSERT BEFORE .data;
