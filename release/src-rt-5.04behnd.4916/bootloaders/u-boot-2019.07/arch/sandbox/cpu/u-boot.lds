/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2011-2012 The Chromium OS Authors.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

SECTIONS
{

	. = ALIGN(4);
	.u_boot_list : {
		KEEP(*(SORT(.u_boot_list*)));
	}

	__u_boot_sandbox_option_start = .;
	_u_boot_sandbox_getopt : { *(.u_boot_sandbox_getopt) }
	__u_boot_sandbox_option_end = .;

	.__efi_runtime_start : {
		*(.__efi_runtime_start)
	}

	.efi_runtime : {
		*(efi_runtime_text)
		*(efi_runtime_data)
	}

	.__efi_runtime_stop : {
		*(.__efi_runtime_stop)
	}

	.efi_runtime_rel_start :
	{
		*(.__efi_runtime_rel_start)
	}

	.efi_runtime_rel : {
		*(.relefi_runtime_text)
		*(.relefi_runtime_data)
	}

	.efi_runtime_rel_stop :
	{
		*(.__efi_runtime_rel_stop)
	}

	__bss_start = .;
}

INSERT BEFORE .data;
