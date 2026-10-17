/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2019 Heinrich Schuchardt <xypron.glpk@gmx.de>
 * Copyright (c) 2021 Steffen Jaeckel <jaeckel-floss@eyet-services.de>
 */

#ifndef __TEST_COMMON_H__
#define __TEST_COMMON_H__

#include <test/test.h>

/* Declare a new common function test */
#define COMMON_TEST(_name, _flags) UNIT_TEST(_name, _flags, common_test)

#endif /* __TEST_COMMON_H__ */
