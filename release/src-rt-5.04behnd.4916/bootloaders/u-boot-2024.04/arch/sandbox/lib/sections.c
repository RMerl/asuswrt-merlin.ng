// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2013 Albert ARIBAUD <albert.u.boot@aribaud.net>
 *
 */
#include <linux/compiler.h>

char __efi_runtime_start[0] __section("___efi_runtime_start");
char __efi_runtime_stop[0] __section("___efi_runtime_stop");
char __efi_runtime_rel_start[0]
		__section("___efi_runtime_rel_start");
char __efi_runtime_rel_stop[0]
		__section("___efi_runtime_rel_stop");
