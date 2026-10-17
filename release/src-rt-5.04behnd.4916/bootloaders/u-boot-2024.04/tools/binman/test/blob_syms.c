// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2017 Google, Inc
 *
 * Simple program to create some binman symbols. This is used by binman tests.
 */

#include <linux/kconfig.h>
#include <binman_sym.h>

DECLARE_BINMAN_MAGIC_SYM;

unsigned long val1 = 123;
unsigned long val2 = 456;
binman_sym_declare(unsigned long, inset, offset);
unsigned long val3 = 789;
unsigned long val4 = 999;
binman_sym_declare(unsigned long, inset, size);
