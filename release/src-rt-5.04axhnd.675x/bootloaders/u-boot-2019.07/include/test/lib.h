/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2019 Heinrich Schuchardt <xypron.glpk@gmx.de>
 */

#ifndef __TEST_LIB_H__
#define __TEST_LIB_H__

#include <test/test.h>

/* Declare a new library function test */
#define LIB_TEST(_name, _flags)	UNIT_TEST(_name, _flags, lib_test)

#endif /* __TEST_LIB_H__ */
