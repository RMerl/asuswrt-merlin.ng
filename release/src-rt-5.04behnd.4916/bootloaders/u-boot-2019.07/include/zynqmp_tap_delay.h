/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Xilinx ZynqMP SoC Tap Delay Programming
 *
 * Copyright (C) 2018 Xilinx, Inc.
 */

#ifndef __ZYNQMP_TAP_DELAY_H__
#define __ZYNQMP_TAP_DELAY_H__

#ifdef CONFIG_ARCH_ZYNQMP
void zynqmp_dll_reset(u8 deviceid);
void arasan_zynqmp_set_tapdelay(u8 device_id, u8 uhsmode, u8 bank);
#else
inline void zynqmp_dll_reset(u8 deviceid) {}
inline void arasan_zynqmp_set_tapdelay(u8 device_id, u8 uhsmode, u8 bank) {}
#endif

#endif
