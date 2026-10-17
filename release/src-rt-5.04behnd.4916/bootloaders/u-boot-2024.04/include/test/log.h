/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2020, Heinrich Schuchardt <xypron.glpk@gmx.de>
 *
 * Tests for logging functions
 */

#ifndef __TEST_LOG_H__
#define __TEST_LOG_H__

#include <test/test.h>

#define LOGF_TEST (BIT(LOGF_FUNC) | BIT(LOGF_MSG))

/* Declare a new logging test */
#define LOG_TEST(_name) UNIT_TEST(_name, 0, log_test)
#define LOG_TEST_FLAGS(_name, _flags) UNIT_TEST(_name, _flags, log_test)

#endif /* __TEST_LOG_H__ */
