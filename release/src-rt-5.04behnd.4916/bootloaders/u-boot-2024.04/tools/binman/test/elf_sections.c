// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2022 Google LLC
 *
 * Program containing two text sections
 */

int __attribute__((section(".sram_data"))) data[29];

int __attribute__((section(".sram_code"))) calculate(int x)
{
	data[0] = x;

	return x * x;
}

int main(void)
{
	return calculate(123);
}
