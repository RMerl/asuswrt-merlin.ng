// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 Fraunhofer AISEC,
 * Lukas Auer <lukas.auer@aisec.fraunhofer.de>
 */

#include <common.h>
#include <asm/sbi.h>

int riscv_send_ipi(int hart)
{
	ulong mask;

	mask = 1UL << hart;
	sbi_send_ipi(&mask);

	return 0;
}

int riscv_clear_ipi(int hart)
{
	sbi_clear_ipi();

	return 0;
}
