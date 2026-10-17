// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 Fraunhofer AISEC,
 * Lukas Auer <lukas.auer@aisec.fraunhofer.de>
 */

#include <asm/encoding.h>
#include <asm/sbi.h>

int riscv_init_ipi(void)
{
	return 0;
}

int riscv_send_ipi(int hart)
{
	ulong mask;

	mask = 1UL << hart;
	sbi_send_ipi(&mask);

	return 0;
}

int riscv_clear_ipi(int hart)
{
	csr_clear(CSR_SIP, SIP_SSIP);

	return 0;
}

int riscv_get_ipi(int hart, int *pending)
{
	/*
	 * The SBI does not support reading the IPI status. We always return 0
	 * to indicate that no IPI is pending.
	 */
	*pending = 0;

	return 0;
}
