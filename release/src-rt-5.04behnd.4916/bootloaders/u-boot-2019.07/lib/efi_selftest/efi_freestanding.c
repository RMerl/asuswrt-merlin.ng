// SPDX-License-Identifier: GPL-2.0+
/*
 * Library for freestanding binary
 *
 * Copyright 2019, Heinrich Schuchardt <xypron.glpk@gmx.de>
 *
 * GCC requires that freestanding programs provide memcpy(), memmove(),
 * memset(), and memcmp().
 */

#include "../efi_loader/efi_freestanding.c"
