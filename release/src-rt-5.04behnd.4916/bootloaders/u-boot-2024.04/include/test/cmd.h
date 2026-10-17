/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright 2023 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef __TEST_CMD_H__
#define __TEST_CMD_H__

#include <test/test.h>

/* Declare a new command test */
#define CMD_TEST(_name, _flags) UNIT_TEST(_name, _flags, cmd_test)

#endif /* __TEST_CMD_H__ */
