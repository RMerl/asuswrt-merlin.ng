// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2021 Google LLC
 *
 * Simple program including some embedded data that can be accessed by binman.
 * This is used by binman tests.
 */

int first[10] = {1};
int before[2] __attribute__((section(".embed"))) = {2, 3};
int embed[3] __attribute__((section(".embed"))) = {0x1234, 0x5678};
int second[10] = {1};

int main(void)
{
	return 0;
}
