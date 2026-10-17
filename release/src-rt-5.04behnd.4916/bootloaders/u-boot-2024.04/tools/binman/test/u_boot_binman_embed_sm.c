// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2021 Google LLC
 *
 * Simple program to embed a devicetree. This is used by binman tests.
 */

int __attribute__((section(".mydtb"))) dtb_data[16];

int main(void)
{
	return 0;
}
