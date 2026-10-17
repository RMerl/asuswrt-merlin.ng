/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016 Bin Meng <bmeng.cn@gmail.com>
 */

#include <asm/acpi/global_nvs.h>

OperationRegion(GNVS, SystemMemory, ACPI_GNVS_ADDR, ACPI_GNVS_SIZE)
Field(GNVS, ByteAcc, NoLock, Preserve)
{
	PCNT, 8,	/* processor count */
	IURE, 8,	/* internal UART enabled */
}
