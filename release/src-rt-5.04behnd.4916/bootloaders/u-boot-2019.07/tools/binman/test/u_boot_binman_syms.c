// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2017 Google, Inc
 *
 * Simple program to create some binman symbols. This is used by binman tests.
 */

#define CONFIG_BINMAN
#include <binman_sym.h>

binman_sym_declare(unsigned long, u_boot_spl, offset);
binman_sym_declare(unsigned long long, u_boot_spl2, offset);
binman_sym_declare(unsigned long, u_boot_any, image_pos);
