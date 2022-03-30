// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018 Google
 */

#include <common.h>

int board_init(void)
{
	return 0;
}

/* provided to defeat compiler optimisation in board_init_f() */
void gru_dummy_function(int i)
{
}
