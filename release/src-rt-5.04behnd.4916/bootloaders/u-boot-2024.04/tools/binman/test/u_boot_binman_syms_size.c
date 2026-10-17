// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2017 Google, Inc
 *
 * Simple program to create some binman symbols. This is used by binman tests.
 */

#include <linux/kconfig.h>
#include <binman_sym.h>

DECLARE_BINMAN_MAGIC_SYM;

binman_sym_declare(char, u_boot_spl, pos);
