// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013-2014 Panasonic Corporation
 * Copyright (C) 2015-2017 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 */

#include <stdio.h>
#include <linux/errno.h>
#include <linux/io.h>
#include <linux/printk.h>

#include "soc-info.h"

int print_cpuinfo(void)
{
	unsigned int id, model, rev, required_model = 1, required_rev = 1;

	id = uniphier_get_soc_id();
	model = uniphier_get_soc_model();
	rev = uniphier_get_soc_revision();

	puts("SoC:   ");

	switch (id) {
	case UNIPHIER_LD4_ID:
		puts("LD4");
		required_rev = 2;
		break;
	case UNIPHIER_PRO4_ID:
		puts("Pro4");
		break;
	case UNIPHIER_SLD8_ID:
		puts("sLD8");
		break;
	case UNIPHIER_PRO5_ID:
		puts("Pro5");
		break;
	case UNIPHIER_PXS2_ID:
		puts("PXs2");
		break;
	case UNIPHIER_LD6B_ID:
		puts("LD6b");
		break;
	case UNIPHIER_LD11_ID:
		puts("LD11");
		break;
	case UNIPHIER_LD20_ID:
		puts("LD20");
		break;
	case UNIPHIER_PXS3_ID:
		puts("PXs3");
		break;
	default:
		printf("Unknown Processor ID (0x%x)\n", id);
		return -ENOTSUPP;
	}

	printf(" (model %d, revision %d)\n", model, rev);

	if (model < required_model) {
		pr_err("Only model %d or newer is supported.\n",
		       required_model);
		return -ENOTSUPP;
	} else if (rev < required_rev) {
		pr_err("Only revision %d or newer is supported.\n",
		       required_rev);
		return -ENOTSUPP;
	}

	return 0;
}
