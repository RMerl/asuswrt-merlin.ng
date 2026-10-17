/* SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause */
/*
 * Copyright (C) 2019, STMicroelectronics - All Rights Reserved
 */

#ifndef _RAM_STM32MP1_TESTS_H_
#define _RAM_STM32MP1_TESTS_H_

#include "stm32mp1_ddr_regs.h"

enum test_result {
	TEST_PASSED,
	TEST_FAILED,
	TEST_ERROR
};

struct test_desc {
	enum test_result (*fct)(struct stm32mp1_ddrctl *ctl,
				struct stm32mp1_ddrphy *phy,
				char *string,
				int argc, char *argv[]);
	const char *name;
	const char *usage;
	const char *help;
	u8 max_args;
};

extern const struct test_desc test[];
extern const int test_nb;

#endif
