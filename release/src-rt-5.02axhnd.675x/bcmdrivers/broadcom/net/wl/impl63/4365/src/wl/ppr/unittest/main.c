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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "test_ppr_size_routine.h"
#include "test_ppr_clean.h"

/* ***************************************************************************************************************
   ************************************* Start of Test Section ***************************************************
   ***************************************************************************************************************/

#include <check.h> /* Includes Check framework */

/*
 * Main flow:
 * 1. Define SRunner object which will aggregate all suites.
 * 2. Adding all suites to SRunner which enables consecutive suite(s) running.
 * 3. Running all suites contained in SRunner.
 */

int main(void){
	int number_failed; /* Count number of failed tests*/
	//Adding suit to SRunner.
	SRunner *sr = srunner_create(ppr_size_routine());
	//Adding another suit to SRunner.
	srunner_add_suite (sr,ppr_clean());
	srunner_run_all (sr, CK_VERBOSE); /* Prints the summary one message per test (passed or failed) */
	number_failed = (int) srunner_ntests_failed(sr); /* count all the failed tests */
	srunner_free(sr);
	return (number_failed == 0) ? EXIT_SUCCESS: EXIT_FAILURE;
}
