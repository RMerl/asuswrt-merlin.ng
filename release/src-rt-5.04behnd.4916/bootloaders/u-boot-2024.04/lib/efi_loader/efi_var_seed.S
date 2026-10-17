/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Predefined UEFI variables
 *
 * Copyright (c) 2020, Heinrich Schuchardt <xypron.glpk@gmx.de>
 */

#include <config.h>

.section .rodata.efi_seed.init,"a"
.balign 16
.global __efi_var_file_begin
__efi_var_file_begin:
.incbin CONFIG_EFI_VAR_SEED_FILE
.global __efi_var_file_end
__efi_var_file_end:
.balign 16
