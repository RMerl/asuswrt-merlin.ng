// SPDX-License-Identifier: GPL-2.0
/*
 * From coreboot src/soc/intel/broadwell/romstage/power_state.c
 *
 * Copyright (C) 2016 Google, Inc.
 */

#include <common.h>
#include <pci.h>
#include <asm/io.h>
#include <asm/intel_regs.h>
#include <asm/arch/iomap.h>
#include <asm/arch/lpc.h>
#include <asm/arch/pch.h>
#include <asm/arch/pm.h>

/* Return 0, 3, or 5 to indicate the previous sleep state. */
static int prev_sleep_state(struct chipset_power_state *ps)
{
	/* Default to S0. */
	int prev_sleep_state = SLEEP_STATE_S0;

	if (ps->pm1_sts & WAK_STS) {
		switch ((ps->pm1_cnt & SLP_TYP) >> SLP_TYP_SHIFT) {
#if CONFIG_HAVE_ACPI_RESUME
		case SLP_TYP_S3:
			prev_sleep_state = SLEEP_STATE_S3;
			break;
#endif
		case SLP_TYP_S5:
			prev_sleep_state = SLEEP_STATE_S5;
			break;
		}
		/* Clear SLP_TYP. */
		outl(ps->pm1_cnt & ~(SLP_TYP), ACPI_BASE_ADDRESS + PM1_CNT);
	}

	if (ps->gen_pmcon3 & (PWR_FLR | SUS_PWR_FLR))
		prev_sleep_state = SLEEP_STATE_S5;

	return prev_sleep_state;
}

static void dump_power_state(struct chipset_power_state *ps)
{
	debug("PM1_STS:   %04x\n", ps->pm1_sts);
	debug("PM1_EN:    %04x\n", ps->pm1_en);
	debug("PM1_CNT:   %08x\n", ps->pm1_cnt);
	debug("TCO_STS:   %04x %04x\n", ps->tco1_sts, ps->tco2_sts);

	debug("GPE0_STS:  %08x %08x %08x %08x\n",
	      ps->gpe0_sts[0], ps->gpe0_sts[1],
	      ps->gpe0_sts[2], ps->gpe0_sts[3]);
	debug("GPE0_EN:   %08x %08x %08x %08x\n",
	      ps->gpe0_en[0], ps->gpe0_en[1],
	      ps->gpe0_en[2], ps->gpe0_en[3]);

	debug("GEN_PMCON: %04x %04x %04x\n",
	      ps->gen_pmcon1, ps->gen_pmcon2, ps->gen_pmcon3);

	debug("Previous Sleep State: S%d\n",
	      ps->prev_sleep_state);
}

/* Fill power state structure from ACPI PM registers */
void power_state_get(struct udevice *pch_dev, struct chipset_power_state *ps)
{
	ps->pm1_sts = inw(ACPI_BASE_ADDRESS + PM1_STS);
	ps->pm1_en = inw(ACPI_BASE_ADDRESS + PM1_EN);
	ps->pm1_cnt = inl(ACPI_BASE_ADDRESS + PM1_CNT);
	ps->tco1_sts = inw(ACPI_BASE_ADDRESS + TCO1_STS);
	ps->tco2_sts = inw(ACPI_BASE_ADDRESS + TCO2_STS);
	ps->gpe0_sts[0] = inl(ACPI_BASE_ADDRESS + GPE0_STS(0));
	ps->gpe0_sts[1] = inl(ACPI_BASE_ADDRESS + GPE0_STS(1));
	ps->gpe0_sts[2] = inl(ACPI_BASE_ADDRESS + GPE0_STS(2));
	ps->gpe0_sts[3] = inl(ACPI_BASE_ADDRESS + GPE0_STS(3));
	ps->gpe0_en[0] = inl(ACPI_BASE_ADDRESS + GPE0_EN(0));
	ps->gpe0_en[1] = inl(ACPI_BASE_ADDRESS + GPE0_EN(1));
	ps->gpe0_en[2] = inl(ACPI_BASE_ADDRESS + GPE0_EN(2));
	ps->gpe0_en[3] = inl(ACPI_BASE_ADDRESS + GPE0_EN(3));

	dm_pci_read_config16(pch_dev, GEN_PMCON_1, &ps->gen_pmcon1);
	dm_pci_read_config16(pch_dev, GEN_PMCON_2, &ps->gen_pmcon2);
	dm_pci_read_config16(pch_dev, GEN_PMCON_3, &ps->gen_pmcon3);

	ps->prev_sleep_state = prev_sleep_state(ps);

	dump_power_state(ps);
}
