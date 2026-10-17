/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright 2019 Google LLC
 */

#ifndef _ASM_ARCH_ACPI_H
#define _ASM_ARCH_ACPI_H

struct acpi_ctx;

/**
 * apl_acpi_fill_dmar() - Set up the DMAR for APL
 *
 * @ctx: ACPI context pointer
 */
int apl_acpi_fill_dmar(struct acpi_ctx *ctx);

#endif /* _ASM_ARCH_CPU_H */
