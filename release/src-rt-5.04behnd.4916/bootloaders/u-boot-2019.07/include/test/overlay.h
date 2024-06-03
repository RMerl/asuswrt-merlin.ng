/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2016 NextThing Co
 * Copyright (c) 2016 Free Electrons
 */

#ifndef __TEST_OVERLAY_H__
#define __TEST_OVERLAY_H__

#include <test/test.h>

/* Declare a new environment test */
#define OVERLAY_TEST(_name, _flags)	UNIT_TEST(_name, _flags, overlay_test)

#endif /* __TEST_OVERLAY_H__ */
