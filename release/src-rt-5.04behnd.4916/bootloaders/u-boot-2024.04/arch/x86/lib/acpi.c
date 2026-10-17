// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018, Bin Meng <bmeng.cn@gmail.com>
 */

#include <common.h>
#include <log.h>
#include <acpi/acpi_table.h>
#include <asm/io.h>
#include <asm/tables.h>

static struct acpi_rsdp *acpi_valid_rsdp(struct acpi_rsdp *rsdp)
{
	if (strncmp((char *)rsdp, RSDP_SIG, sizeof(RSDP_SIG) - 1) != 0)
		return NULL;

	debug("Looking on %p for valid checksum\n", rsdp);

	if (table_compute_checksum((void *)rsdp, 20) != 0)
		return NULL;
	debug("acpi rsdp checksum 1 passed\n");

	if ((rsdp->revision > 1) &&
	    (table_compute_checksum((void *)rsdp, rsdp->length) != 0))
		return NULL;
	debug("acpi rsdp checksum 2 passed\n");

	return rsdp;
}

struct acpi_fadt *acpi_find_fadt(void)
{
	char *p, *end;
	struct acpi_rsdp *rsdp = NULL;
	struct acpi_rsdt *rsdt;
	struct acpi_fadt *fadt = NULL;
	int i;

	/* Find RSDP */
	for (p = (char *)ROM_TABLE_ADDR; p < (char *)ROM_TABLE_END; p += 16) {
		rsdp = acpi_valid_rsdp((struct acpi_rsdp *)p);
		if (rsdp)
			break;
	}

	if (!rsdp)
		return NULL;

	debug("RSDP found at %p\n", rsdp);
	rsdt = (struct acpi_rsdt *)(uintptr_t)rsdp->rsdt_address;

	end = (char *)rsdt + rsdt->header.length;
	debug("RSDT found at %p ends at %p\n", rsdt, end);

	for (i = 0; ((char *)&rsdt->entry[i]) < end; i++) {
		fadt = (struct acpi_fadt *)(uintptr_t)rsdt->entry[i];
		if (strncmp((char *)fadt, "FACP", 4) == 0)
			break;
		fadt = NULL;
	}

	if (!fadt)
		return NULL;

	debug("FADT found at %p\n", fadt);
	return fadt;
}

void *acpi_find_wakeup_vector(struct acpi_fadt *fadt)
{
	struct acpi_facs *facs;
	void *wake_vec;

	debug("Trying to find the wakeup vector...\n");

	facs = (struct acpi_facs *)(uintptr_t)fadt->firmware_ctrl;

	if (!facs) {
		debug("No FACS found, wake up from S3 not possible.\n");
		return NULL;
	}

	debug("FACS found at %p\n", facs);
	wake_vec = (void *)(uintptr_t)facs->firmware_waking_vector;
	debug("OS waking vector is %p\n", wake_vec);

	return wake_vec;
}

void enter_acpi_mode(int pm1_cnt)
{
	u16 val = inw(pm1_cnt);

	/*
	 * PM1_CNT register bit0 selects the power management event to be
	 * either an SCI or SMI interrupt. When this bit is set, then power
	 * management events will generate an SCI interrupt. When this bit
	 * is reset power management events will generate an SMI interrupt.
	 *
	 * Per ACPI spec, it is the responsibility of the hardware to set
	 * or reset this bit. OSPM always preserves this bit position.
	 *
	 * U-Boot does not support SMI. And we don't have plan to support
	 * anything running in SMM within U-Boot. To create a legacy-free
	 * system, and expose ourselves to OSPM as working under ACPI mode
	 * already, turn this bit on.
	 */
	outw(val | PM1_CNT_SCI_EN, pm1_cnt);
}
