// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2016 Google, Inc
 *
 * Simple program to create a _dt_ucode_base_size symbol which can be read
 * by binutils. This is used by binman tests.
 */

static unsigned long _dt_ucode_base_size[2]
		__attribute__((section(".ucode"))) = {1, 2};
