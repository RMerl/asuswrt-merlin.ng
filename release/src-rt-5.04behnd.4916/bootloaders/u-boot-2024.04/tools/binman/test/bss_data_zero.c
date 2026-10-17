// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2016 Google, Inc
 *
 * Simple program to create a bss_data region so the symbol can be read
 * by binutils. This is used by binman tests.
 */

int bss_data[10];

int main(void)
{
	bss_data[2] = 2;

	return 0;
}
