/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 *
 * Architecture-specific definitions (FSP config and VPD/UPD)
 */

#ifndef __FSP_ARCH_H__
#define __FSP_ARCH_H__

/*
 * Note: use #ifndef __ASSEMBLY__ around any struct definitions or other C code
 * since this file can be included from assembly.
 */

#include <asm/fsp1/fsp_api.h>
#include <asm/fsp1/fsp_ffs.h>
#include <asm/arch/fsp/fsp_vpd.h>
#include <asm/arch/fsp/fsp_configs.h>

#endif
