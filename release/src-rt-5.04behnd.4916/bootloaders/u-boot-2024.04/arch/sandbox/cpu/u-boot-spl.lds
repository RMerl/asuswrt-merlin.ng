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

	/* Private data for devices with OF_PLATDATA_RT */
	. = ALIGN(4);
	.priv_data : {
		__priv_data_start = .;
		*(SORT_BY_ALIGNMENT(SORT_BY_NAME(.priv_data*)))
		__priv_data_end = .;
	}

	_u_boot_sandbox_getopt : {
		*(_u_boot_sandbox_getopt_start)
		KEEP(*(_u_boot_sandbox_getopt))
		*(_u_boot_sandbox_getopt_end)
	}

	_image_binary_end = .;
}

INSERT AFTER .data;
