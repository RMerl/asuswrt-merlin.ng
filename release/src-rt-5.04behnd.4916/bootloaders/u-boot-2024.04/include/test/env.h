/* SPDX-License-Identifier: GPL-2.0 */
/*
 * (C) Copyright 2015
 * Joe Hershberger, National Instruments, joe.hershberger@ni.com
 */

#ifndef __TEST_ENV_H__
#define __TEST_ENV_H__

#include <test/test.h>

/* Declare a new environment test */
#define ENV_TEST(_name, _flags)	UNIT_TEST(_name, _flags, env_test)

#endif /* __TEST_ENV_H__ */
