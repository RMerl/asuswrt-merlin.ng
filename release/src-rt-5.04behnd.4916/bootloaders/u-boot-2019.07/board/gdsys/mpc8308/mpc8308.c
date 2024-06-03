// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014
 * Dirk Eibach,  Guntermann & Drunck GmbH, dirk.eibach@gdsys.cc
 */

#include <common.h>
#include <command.h>
#include <asm/processor.h>
#include <asm/io.h>
#include <asm/global_data.h>

#include "mpc8308.h"
#include <gdsys_fpga.h>

#define REFLECTION_TESTPATTERN 0xdede
#define REFLECTION_TESTPATTERN_INV (~REFLECTION_TESTPATTERN & 0xffff)

#ifdef CONFIG_SYS_FPGA_NO_RFL_HI
#define REFLECTION_TESTREG reflection_low
#else
#define REFLECTION_TESTREG reflection_high
#endif

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_GDSYS_LEGACY_DRIVERS
/* as gpio output status cannot be read back, we have to buffer it locally */
u32 gpio0_out;

void setbits_gpio0_out(u32 mask)
{
	immap_t *immr = (immap_t *)CONFIG_SYS_IMMR;

	gpio0_out |= mask;
	out_be32(&immr->gpio[0].dat, gpio0_out);
}

void clrbits_gpio0_out(u32 mask)
{
	immap_t *immr = (immap_t *)CONFIG_SYS_IMMR;

	gpio0_out &= ~mask;
	out_be32(&immr->gpio[0].dat, gpio0_out);
}

int get_fpga_state(uint dev)
{
	return gd->arch.fpga_state[dev];
}

int board_early_init_f(void)
{
	uint k;

	for (k = 0; k < CONFIG_SYS_FPGA_COUNT; ++k)
		gd->arch.fpga_state[k] = 0;

	return 0;
}

int board_early_init_r(void)
{
	uint k;
	uint ctr;

	for (k = 0; k < CONFIG_SYS_FPGA_COUNT; ++k)
		gd->arch.fpga_state[k] = 0;

	/*
	 * reset FPGA
	 */
	mpc8308_init();

	mpc8308_set_fpga_reset(1);

	mpc8308_setup_hw();

	for (k = 0; k < CONFIG_SYS_FPGA_COUNT; ++k) {
		ctr = 0;
		while (!mpc8308_get_fpga_done(k)) {
			mdelay(100);
			if (ctr++ > 5) {
				gd->arch.fpga_state[k] |=
					FPGA_STATE_DONE_FAILED;
				break;
			}
		}
	}

	udelay(10);

	mpc8308_set_fpga_reset(0);

	for (k = 0; k < CONFIG_SYS_FPGA_COUNT; ++k) {
		/*
		 * wait for fpga out of reset
		 */
		ctr = 0;
		while (1) {
			u16 val;

			FPGA_SET_REG(k, reflection_low, REFLECTION_TESTPATTERN);

			FPGA_GET_REG(k, REFLECTION_TESTREG, &val);
			if (val == REFLECTION_TESTPATTERN_INV)
				break;

			mdelay(100);
			if (ctr++ > 5) {
				gd->arch.fpga_state[k] |=
					FPGA_STATE_REFLECTION_FAILED;
				break;
			}
		}
	}

	return 0;
}
#endif
