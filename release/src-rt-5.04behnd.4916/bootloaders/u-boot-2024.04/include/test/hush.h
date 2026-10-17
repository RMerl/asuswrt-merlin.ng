/* SPDX-License-Identifier: GPL-2.0 */
/*
 * (C) Copyright 2021
 * Francis Laniel, Amarula Solutions, francis.laniel@amarulasolutions.com
 */

#ifndef __TEST_HUSH_H__
#define __TEST_HUSH_H__

#include <test/test.h>

/* Declare a new environment test */
#define HUSH_TEST(_name, _flags)	UNIT_TEST(_name, _flags, hush_test)

#endif /* __TEST_HUSH_H__ */
