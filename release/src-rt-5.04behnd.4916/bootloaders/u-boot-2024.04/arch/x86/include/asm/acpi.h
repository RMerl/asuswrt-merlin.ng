/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2018, Bin Meng <bmeng.cn@gmail.com>
 */

#ifndef __ASM_ACPI_H__
#define __ASM_ACPI_H__

struct acpi_fadt;

/**
 * acpi_find_fadt() - find ACPI FADT table in the system memory
 *
 * This routine parses the ACPI table to locate the ACPI FADT table.
 *
 * @return:	a pointer to the ACPI FADT table in the system memory
 */
struct acpi_fadt *acpi_find_fadt(void);

/**
 * acpi_find_wakeup_vector() - find OS installed wake up vector address
 *
 * This routine parses the ACPI table to locate the wake up vector installed
 * by the OS previously.
 *
 * @fadt:	a pointer to the ACPI FADT table in the system memory
 * @return:	wake up vector address installed by the OS
 */
void *acpi_find_wakeup_vector(struct acpi_fadt *fadt);

/**
 * enter_acpi_mode() - enter into ACPI mode
 *
 * This programs the ACPI-defined PM1_CNT register to enable SCI interrupt
 * so that the whole system swiches to ACPI mode.
 *
 * @pm1_cnt:	PM1_CNT register I/O address
 */
void enter_acpi_mode(int pm1_cnt);

#endif /* __ASM_ACPI_H__ */
