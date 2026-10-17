/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2019, Theobroma Systems Design und Consulting GmbH
 */

#ifndef __TEST_OPTEE_H__
#define __TEST_OPTEE_H__

#include <test/test.h>

/* Declare a new environment test */
#define OPTEE_TEST(_name, _flags)	UNIT_TEST(_name, _flags, optee_test)

#endif /* __TEST_OPTEE_H__ */
