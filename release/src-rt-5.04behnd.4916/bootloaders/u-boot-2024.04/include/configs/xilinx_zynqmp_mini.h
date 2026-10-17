/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration for Xilinx ZynqMP Flash utility
 *
 * (C) Copyright 2018 Xilinx, Inc.
 * Michal Simek <michal.simek@amd.com>
 * Siva Durga Prasad Paladugu <siva.durga.prasad.paladugu@amd.com>>
 */

#ifndef __CONFIG_ZYNQMP_MINI_H
#define __CONFIG_ZYNQMP_MINI_H

#define CFG_EXTRA_ENV_SETTINGS

#include <configs/xilinx_zynqmp.h>

/* Undef unneeded configs */
#undef CFG_EXTRA_ENV_SETTINGS

#endif /* __CONFIG_ZYNQMP_MINI_H */
