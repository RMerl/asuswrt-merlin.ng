/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) ASPEED Technology Inc.
 * Ryan Chen <ryan_chen@aspeedtech.com>
 *
 */

#ifndef _ASM_ARCH_PLATFORM_H
#define _ASM_ARCH_PLATFORM_H

#if defined(CONFIG_ASPEED_AST2500)
#define ASPEED_MAC_COUNT	2
#define ASPEED_DRAM_BASE	0x80000000
#define ASPEED_SRAM_BASE	0x1e720000
#define ASPEED_SRAM_SIZE	0x9000
#elif defined(CONFIG_ASPEED_AST2600)
#define ASPEED_MAC_COUNT	4
#define ASPEED_DRAM_BASE	0x80000000
#define ASPEED_SRAM_BASE	0x10000000
#define ASPEED_SRAM_SIZE	0x16000
#else
#err "Unrecognized Aspeed platform."
#endif

#endif
