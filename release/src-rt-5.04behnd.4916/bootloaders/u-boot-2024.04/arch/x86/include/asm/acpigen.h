/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Generation of x86-specific ACPI tables
 *
 * Copyright 2020 Google LLC
 */

#ifndef __ASM_ACPIGEN_H__
#define __ASM_ACPIGEN_H__

struct acpi_ctx;

/**
 * acpigen_write_empty_pct() - Write an empty PCT
 *
 * See ACPI v6.3 section 8.4.6.1: _PCT (Performance Control)
 *
 * This writes an empty table so that CPU performance works as expected
 *
 * @ctx: ACPI context pointer
 */
void acpigen_write_empty_pct(struct acpi_ctx *ctx);

/**
 * acpigen_write_empty_ptc() - Write an empty PTC
 *
 * See ACPI v6.3 section 8.4.5.1: _PTC (Processor Throttling Control)
 *
 * This writes an empty table so that CPU performance works as expected
 *
 * @ctx: ACPI context pointer
 */
void acpigen_write_empty_ptc(struct acpi_ctx *ctx);

#endif /* __ASM_ACPI_H__ */
