// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2016 Google, Inc
 *
 * Simple program to create a bad _dt_ucode_base_size symbol to create an
 * error when it is used. This is used by binman tests.
 */

static unsigned long not__dt_ucode_base_size[2]
		__attribute__((section(".ucode"))) = {1, 2};
