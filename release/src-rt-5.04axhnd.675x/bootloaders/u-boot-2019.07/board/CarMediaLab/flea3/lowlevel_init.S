/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2007, Guennadi Liakhovetski <lg@denx.de>
 *
 * (C) Copyright 2008-2010 Freescale Semiconductor, Inc.
 *
 * Copyright (C) 2011, Stefano Babic <sbabic@denx.de>
 */

#include <config.h>
#include <asm/arch/lowlevel_macro.S>

.globl lowlevel_init
lowlevel_init:

	core_init

	init_aips

	init_max

	init_m3if

	mov pc, lr
