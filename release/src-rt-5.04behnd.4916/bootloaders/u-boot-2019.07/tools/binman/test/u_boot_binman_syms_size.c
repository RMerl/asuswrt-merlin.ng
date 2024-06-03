// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2017 Google, Inc
 *
 * Simple program to create some binman symbols. This is used by binman tests.
 */

#define CONFIG_BINMAN
#include <binman_sym.h>

binman_sym_declare(char, u_boot_spl, pos);
