// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2016 Google, Inc
 *
 * Simple program to create a _dt_ucode_base_size symbol which can be read
 * by binutils. This is used by binman tests.
 */

int bss_data[10];
int __bss_size = sizeof(bss_data);

int main()
{
	bss_data[2] = 2;

	return 0;
}
