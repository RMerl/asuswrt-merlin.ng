/*
 * main test for hotspot packet encoder & decoder module
 *
 * Copyright (C) 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: main.c xxxxxx 2013-10-30 06:00:44Z emanuell,shaib $
 */

/*	***************************************************************
 *	Definitions for module components to be tested with Check  tool
 *	***************************************************************
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <trace.h>
#include <test_bcm_enc_dec.h>
#include <test_bcm_enc_dec_ie.h>
#include <test_bcm_enc_dec_gas.h>
#include <test_bcm_enc_dec_anqp.h>
#include <test_bcm_enc_dec_hspot_anqp.h>
#include <test_bcm_enc_dec_qos.h>
#include <test_bcm_enc_dec_wnm.h>

/*	**********************
 *	Start of Test Section
 *	**********************
 */

#include <check.h> /* Includes Check framework */

/*
 * Main flow:
 * 1. Define SRunner object which will aggregate all suites.
 * 2. Adding all suites to SRunner which enables consecutive suite(s) running.
 * 3. Running all suites contained in SRunner.
 */

int main(void)
{
	int number_failed; /* Count number of failed tests */
	TRACE_LEVEL_SET(TRACE_ALL);

	/* Adding suit to SRunner. */
	SRunner *sr = srunner_create(bcm_enc_dec_suite());
	/* Adding another suit to SRunner. */
	srunner_add_suite(sr, bcm_enc_dec_ie_suite());
	srunner_add_suite(sr, bcm_enc_dec_gas_suite());
	srunner_add_suite(sr, bcm_enc_dec_anqp_suite());
	srunner_add_suite(sr, bcm_enc_dec_hspot_anqp_suite());
	srunner_add_suite(sr, bcm_enc_dec_qos_suite());
	srunner_add_suite(sr, bcm_enc_dec_wnm_suite());
	/* Prints the summary one message per test (Only failed) */
	srunner_run_all(sr, CK_VERBOSE);
	/* count all the failed tests */
	number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);
	return (number_failed == 0) ? EXIT_SUCCESS: EXIT_FAILURE;
}
