/*
 * Basic unit test for ppr module
 *
 * Copyright (C) 2013, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: test_ppr.c xxxxxx 2013-10-30 06:00:44Z emanuell,shaib $
 */

/* ******************************************************************************************************************
************* Definitions for module components to be tested with Check  tool ***************************************
********************************************************************************************************************/

#include <typedefs.h>
#include <bcmendian.h>
#include <bcmwifi_channels.h>
#include <wlc_ppr.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "test_ppr_clean.h"

/* ***************************************************************************************************************
   ************************************* Start of Test Section ***************************************************
   ***************************************************************************************************************/

#include <check.h> /* Includes Check framework */

/*
 * In order to run unit tests with Check, we must create some test cases,
 * aggregate them into a suite, and run them with a suite runner.

 * The pattern of every unit test is as following

 * START_TEST(name_of_test){
 *
 *     perform tests;
 *	       ...assert results
 * }
 * END_TEST

 * Test Case is a set of at least 1 unit test
 * Test Suite is a collection of Test Cases
 * Check Framework can run multiple Test Suites.
 * More details will be on Twiki
 */

/* ------------- Global Definitoions ------------------------- */

/*
 * Global variables definitions, for setup and teardown function.
 */
static ppr_t* pprptr;
static wl_tx_bw_t bw;
static osl_t* osh;

/* ------------- Startup and Teardown - Fixtures ---------------
 * Setting up objects for each unit test,
 * it may be convenient to add some setup that is constant across all the tests in a test case
 * rather than setting up objects for each unit test.
 * Before each unit test in a test case, the setup() function is run, if defined.
 */
void
setup2(void)
{
	// Create ppr pointer
	bw = WL_TX_BW_20;
	pprptr = ppr_create(osh,bw);
}

/*
 * Tear down objects for each unit test,
 * it may be convenient to add teardown that is constant across all the tests in a test case
 * rather than tearing down objects for each unit test.
 * After each unit test in a test case, the setup() function is run, if defined.
 * Note: checked teardown() fixture will not run if the unit test fails.
*/
void
teardown2(void)
{
	// Delete ppr pointer
	ppr_delete(osh,pprptr);
}

/*
 * The START_TEST/END_TEST pair are macros that setup basic structures to permit testing.
 */

START_TEST(test_ppr_bw20_delete){
	ppr_t* pprptr2;
	wl_tx_bw_t bw2 = WL_TX_BW_40;
	osl_t* osh2;
	pprptr2 = ppr_create(osh2,bw2);

	int ppr_size20=ppr_size(bw2);

	ppr_delete(osh2,pprptr2);
  }
END_TEST

START_TEST(test_ppr_bw20_clear){
	ppr_t* pprptr2;
	wl_tx_bw_t bw2 = WL_TX_BW_20;
	osl_t* osh2;
	pprptr2 = ppr_create(osh2,bw2);
	int ppr_size20 = 182;

	memset( ((uchar*)pprptr2) + 4, (int8)0x30, ppr_size(bw2)-4);

	int i;
	//ppr_delete(osh2,pprptr2);
	ppr_clear(pprptr2);

	for (i = 0; i < ppr_size20 - 4; i++)
		ck_assert_int_eq ( 0xFF & *(4 + i + ((uchar*)pprptr2)), 0xFF & WL_RATE_DISABLED);
  }
END_TEST

START_TEST(test_ppr_bw40_clear){

	ppr_t* pprptr2;
	wl_tx_bw_t bw2 = WL_TX_BW_40;
	osl_t* osh2;
	pprptr2 = ppr_create(osh2,bw2);
	//int ppr_size40=(uint)ppr_size(bw2);
	int ppr_size40 = 364;

	memset( ((uchar*)pprptr2) + 4, (int8)0x50, ppr_size(bw2)-4);

	int i;
	ppr_clear(pprptr2);

		for (i = 0; i < ppr_size40 - 4; i++)
			ck_assert_int_eq ( 0xFF & *(4 + i + ((uchar*)pprptr2)), 0xFF & WL_RATE_DISABLED);
  }
END_TEST

START_TEST(test_ppr_bw80_clear){
	ppr_t* pprptr2;
	wl_tx_bw_t bw2 = WL_TX_BW_80;
	osl_t* osh2;
	pprptr2 = ppr_create(osh2,bw2);
	int ppr_size80 = 546;
	memset( ((uchar*)pprptr2) + 4, (int8)0x70, ppr_size(bw2)-4);

	int i;
	ppr_clear(pprptr2);

	for (i = 0; i < ppr_size80 - 4; i++)
		ck_assert_int_eq ( 0xFF & *(4 + i + ((uchar*)pprptr2)), 0xFF & WL_RATE_DISABLED);
  }
END_TEST

/*
 * Suite of test cases which Asserts the size routine for user serialization
 * allocations.
 *
 */
Suite * ppr_clean(void)
{
	// Suite definition - aggregates test cases into a suite, and run them with a suite runner.
	Suite *s2 = suite_create("PPR - Size routine for user serialization allocations");
	// Test case definition
	TCase *tc_ser_size = tcase_create("Test Case - SER SIZE");
	// Checked fixture to current test case
	tcase_add_checked_fixture(tc_ser_size, setup2, teardown2);
	// Adding unit tests to test case.
	tcase_add_test(tc_ser_size,test_ppr_bw20_delete);
	tcase_add_test(tc_ser_size,test_ppr_bw20_clear);
	tcase_add_test(tc_ser_size,test_ppr_bw40_clear);
	tcase_add_test(tc_ser_size,test_ppr_bw80_clear);
	// Adding 'tc_ser_size' test case to a suite
	suite_add_tcase(s2, tc_ser_size);
	return s2;
}
