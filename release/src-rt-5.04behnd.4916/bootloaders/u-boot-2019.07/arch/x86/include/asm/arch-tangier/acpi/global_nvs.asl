/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2017 Intel Corporation
 *
 * Partially based on global_nvs.asl for other x86 platforms
 */

#include <asm/acpi/global_nvs.h>

OperationRegion(GNVS, SystemMemory, ACPI_GNVS_ADDR, ACPI_GNVS_SIZE)
Field(GNVS, ByteAcc, NoLock, Preserve)
{
    PCNT, 8,    /* processor count */
}
