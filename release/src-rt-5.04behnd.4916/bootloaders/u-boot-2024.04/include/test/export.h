/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2020 Sean Anderson <seanga2@gmail.com>
 */

#ifndef TEST_EXPORT_H
#define TEST_EXPORT_H

/* Declare something static, unless we are doing unit tests */
#ifdef CONFIG_UNIT_TEST
#define TEST_STATIC
#else
#define TEST_STATIC static
#endif

#endif /* TEST_EXPORT_H */
