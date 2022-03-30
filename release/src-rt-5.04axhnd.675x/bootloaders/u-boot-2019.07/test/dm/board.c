// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2018
 * Mario Six, Guntermann & Drunck GmbH, mario.six@gdsys.cc
 */

#include <common.h>
#include <dm.h>
#include <dm/test.h>
#include <board.h>
#include <test/ut.h>

#include "../../drivers/board/sandbox.h"

static int dm_test_board(struct unit_test_state *uts)
{
	struct udevice *board;
	bool called_detect;
	char str[64];
	int i;

	board_get(&board);
	ut_assert(board);

	board_get_bool(board, BOOL_CALLED_DETECT, &called_detect);
	ut_assert(!called_detect);

	board_detect(board);

	board_get_bool(board, BOOL_CALLED_DETECT, &called_detect);
	ut_assert(called_detect);

	board_get_str(board, STR_VACATIONSPOT, sizeof(str), str);
	ut_assertok(strcmp(str, "R'lyeh"));

	board_get_int(board, INT_TEST1, &i);
	ut_asserteq(0, i);

	board_get_int(board, INT_TEST2, &i);
	ut_asserteq(100, i);

	board_get_str(board, STR_VACATIONSPOT, sizeof(str), str);
	ut_assertok(strcmp(str, "Carcosa"));

	board_get_int(board, INT_TEST1, &i);
	ut_asserteq(1, i);

	board_get_int(board, INT_TEST2, &i);
	ut_asserteq(99, i);

	board_get_str(board, STR_VACATIONSPOT, sizeof(str), str);
	ut_assertok(strcmp(str, "Yuggoth"));

	return 0;
}

DM_TEST(dm_test_board, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);
