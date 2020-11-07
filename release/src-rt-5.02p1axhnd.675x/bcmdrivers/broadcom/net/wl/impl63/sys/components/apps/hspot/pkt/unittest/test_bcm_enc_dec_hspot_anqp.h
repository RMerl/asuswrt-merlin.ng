/*
 * Basic unit test for encode & decode of hspot anqp module
 *
 * Copyright (C) 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: test_bcm_enc_dec_hspot_anqp.c xxxxxx 2014-04-09 06:00:44Z shaib $
 */
#include <check.h> /* Includes Check framework */
#ifndef  MAP_ANONYMOUS
#ifdef MAP_ANON
#define MAP_ANONYMOUS MAP_ANON
#endif // endif
#endif	/* MAP_ANONYMOUS */
Suite *bcm_enc_dec_hspot_anqp_suite(void);
