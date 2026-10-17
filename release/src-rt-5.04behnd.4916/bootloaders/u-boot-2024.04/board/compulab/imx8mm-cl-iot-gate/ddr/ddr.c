// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2017 NXP
 * Copyright 2020 Linaro
 *
 */

#include <common.h>
#include <spl.h>
#include <asm/io.h>
#include <errno.h>
#include <command.h>
#include <asm/io.h>
#include <asm/arch/lpddr4_define.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/mach-imx/gpio.h>
#include <asm-generic/gpio.h>
#include <asm/arch/ddr.h>
#include <asm/arch/imx8mq_pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/clock.h>
#include <asm/mach-imx/gpio.h>
#include "ddr.h"

#include <linux/delay.h>

struct lpddr4_desc {
	char name[16];
	unsigned int id;
	unsigned int size;
	unsigned int count;
	/* an optional field
	 * use it if default is not the
	 * 1-st array entry
	 */
	unsigned int _default;
	/* An optional field to distiguish DRAM chips that
	 * have different geometry, though return the same MRR.
	 * Default value 0xff
	 */
	u8	subind;
	struct dram_timing_info *timing;
	char *desc[4];
};

#define DEFAULT (('D' << 24) + ('E' << 16) + ('F' << 8) + 'A')
static const struct lpddr4_desc lpddr4_array[] = {
	{ .name = "Nanya",	.id = 0x05000010, .subind = 0xff,
	  .size = 2048, .count = 1, .timing = &ucm_dram_timing_01061010},
	{ .name = "Samsung",	.id = 0x01061010, .subind = 0xff,
	  .size = 2048, .count = 1, .timing = &ucm_dram_timing_01061010},
	{ .name = "Kingston",	.id = 0xff000010, .subind = 0x04,
	  .size = 4096, .count = 1, .timing = &ucm_dram_timing_ff000110},
	{ .name = "Kingston",	.id = 0xff000010, .subind = 0x02,
	  .size = 2048, .count = 1, .timing = &ucm_dram_timing_01061010},
	{ .name = "Micron",	.id = 0xff020008, .subind = 0xff,
	  .size = 2048, .count = 1, .timing = &ucm_dram_timing_ff020008},
	{ .name = "Micron",	.id = 0xff000110, .subind = 0xff,
	  .size = 4096, .count = 1, .timing = &ucm_dram_timing_ff000110},
};

static unsigned int lpddr4_get_mr(void)
{
	int i = 0, attempts = 5;
	unsigned int ddr_info = 0;
	unsigned int regs[] = { 5, 6, 7, 8 };

	do {
		for (i = 0 ; i < ARRAY_SIZE(regs) ; i++) {
			unsigned int data = 0;

			data = lpddr4_mr_read(0xF, regs[i]);
			ddr_info <<= 8;
			ddr_info += (data & 0xFF);
		}
		if (ddr_info != 0xFFFFFFFF && ddr_info != 0)
			break; // The attempt was successful
	} while (--attempts);
	return	ddr_info;
}

static void spl_tcm_init(struct lpddr4_tcm_desc *lpddr4_tcm_desc)
{
	if (lpddr4_tcm_desc->sign == DEFAULT)
		return;

	lpddr4_tcm_desc->sign = DEFAULT;
	lpddr4_tcm_desc->index = 0;
}

static void spl_tcm_fini(struct lpddr4_tcm_desc *lpddr4_tcm_desc)
{
	if (lpddr4_tcm_desc->sign != DEFAULT)
		return;

	lpddr4_tcm_desc->sign = ~DEFAULT;
	lpddr4_tcm_desc->index = 0;
}

#define SPL_TCM_DATA 0x7e0000
#define SPL_TCM_INIT spl_tcm_init(lpddr4_tcm_desc)
#define SPL_TCM_FINI spl_tcm_fini(lpddr4_tcm_desc)

void spl_dram_init_compulab(void)
{
	unsigned int ddr_info = 0xdeadbeef;
	unsigned int ddr_info_mrr = 0xdeadbeef;
	unsigned int ddr_found = 0;
	int i = 0;

	struct lpddr4_tcm_desc *lpddr4_tcm_desc =
		(struct lpddr4_tcm_desc *)SPL_TCM_DATA;

	if (lpddr4_tcm_desc->sign != DEFAULT) {
		/* get ddr type from the eeprom if not in tcm scan mode */
		ddr_info = cl_eeprom_get_ddrinfo();
		for (i = 0; i < ARRAY_SIZE(lpddr4_array); i++) {
			if (lpddr4_array[i].id == ddr_info &&
			lpddr4_array[i].subind == cl_eeprom_get_subind()) {
				ddr_found = 1;
				break;
			}
		}
	}

	/* Walk trought all available ddr ids and apply
	 * one by one. Save the index at the tcm memory that
	 * persists after the reset.
	 */
	if (ddr_found == 0) {
		SPL_TCM_INIT;

		if (lpddr4_tcm_desc->index < ARRAY_SIZE(lpddr4_array)) {
			printf("DDRINFO: Cfg attempt: [ %d/%lu ]\n",
			       lpddr4_tcm_desc->index + 1,
			       ARRAY_SIZE(lpddr4_array));
			i = lpddr4_tcm_desc->index;
			lpddr4_tcm_desc->index += 1;
		} else {
			/* Ran out all available ddr setings */
			printf("DDRINFO: Ran out all [ %lu ] cfg attempts. A non supported configuration.\n",
			       ARRAY_SIZE(lpddr4_array));
			while (1)
				;
		}
		ddr_info = lpddr4_array[i].id;
	} else {
		printf("DDRINFO(%s): %s %dG\n", (ddr_found ? "D" : "?"),
		       lpddr4_array[i].name,
		       lpddr4_array[i].size);
	}

	if (ddr_init(lpddr4_array[i].timing)) {
		SPL_TCM_INIT;
		do_reset(NULL, 0, 0, NULL);
	}

	ddr_info_mrr = lpddr4_get_mr();
	if (ddr_info_mrr == 0xFFFFFFFF) {
		printf("DDRINFO(M): mr5-8 [ 0x%x ] is invalid; reset\n",
		       ddr_info_mrr);
		SPL_TCM_INIT;
		do_reset(NULL, 0, 0, NULL);
	}

	printf("DDRINFO(M): mr5-8 [ 0x%x ]\n", ddr_info_mrr);
	printf("DDRINFO(%s): mr5-8 [ 0x%x ]\n", (ddr_found ? "E" : "T"),
	       ddr_info);

	if (ddr_info_mrr != ddr_info) {
		SPL_TCM_INIT;
		do_reset(NULL, 0, 0, NULL);
	}

	SPL_TCM_FINI;

	if (ddr_found == 0) {
		/* Update eeprom */
		cl_eeprom_set_ddrinfo(ddr_info_mrr);
		mdelay(10);
		ddr_info = cl_eeprom_get_ddrinfo();
		mdelay(10);
		cl_eeprom_set_subind(lpddr4_array[i].subind);
		/* make sure that the ddr_info has reached the eeprom */
		printf("DDRINFO(E): mr5-8 [ 0x%x ], read back\n", ddr_info);
		if (ddr_info_mrr != ddr_info || cl_eeprom_get_subind() != lpddr4_array[i].subind) {
			printf("DDRINFO(EEPROM): make sure that the eeprom is accessible\n");
			printf("DDRINFO(EEPROM): i2c dev 1; i2c md 0x51 0x40 0x50\n");
		}
	}

	/* Pass the dram size to th U-Boot through the tcm memory */
	{ /* To figure out what to store into the TCM buffer */
	  /* For debug purpouse only. To override the real memsize */
		unsigned int ddr_tcm_size = cl_eeprom_get_osize();

		if (ddr_tcm_size == 0 || ddr_tcm_size == -1)
			ddr_tcm_size = lpddr4_array[i].size;

		lpddr4_tcm_desc->size = ddr_tcm_size;
	}
}
