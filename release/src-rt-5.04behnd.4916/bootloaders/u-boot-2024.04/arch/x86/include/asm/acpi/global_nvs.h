/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016, Bin Meng <bmeng.cn@gmail.com>
 */

#ifndef _ACPI_GNVS_H_
#define _ACPI_GNVS_H_

/*
 * This file provides two ACPI global NVS macros: ACPI_GNVS_ADDR and
 * ACPI_GNVS_SIZE. They are to be used in platform's global_nvs.asl file
 * to declare the GNVS OperationRegion, as well as write_acpi_tables()
 * for the GNVS address runtime fix up.
 *
 * If using CONFIG_ACPI_GNVS_EXTERNAL, we don't need to locate the GNVS in
 * DSDT, since it is created by code, so ACPI_GNVS_ADDR is unused.
 */
#define ACPI_GNVS_ADDR	0xdeadbeef
#define ACPI_GNVS_SIZE	0x1000

#endif /* _ACPI_GNVS_H_ */
