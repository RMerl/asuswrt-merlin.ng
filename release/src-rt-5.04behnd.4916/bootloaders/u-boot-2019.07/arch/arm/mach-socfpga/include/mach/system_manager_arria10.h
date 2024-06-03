/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2016-2017 Intel Corporation <www.intel.com>
 */

#ifndef _SYSTEM_MANAGER_ARRIA10_H_
#define _SYSTEM_MANAGER_ARRIA10_H_

struct socfpga_system_manager {
	u32  siliconid1;
	u32  siliconid2;
	u32  wddbg;
	u32  bootinfo;
	u32  mpu_ctrl_l2_ecc;
	u32  _pad_0x14_0x1f[3];
	u32  dma;
	u32  dma_periph;
	u32  sdmmcgrp_ctrl;
	u32  sdmmc_l3master;
	u32  nand_bootstrap;
	u32  nand_l3master;
	u32  usb0_l3master;
	u32  usb1_l3master;
	u32  emac_global;
	u32  emac[3];
	u32  _pad_0x50_0x5f[4];
	u32  fpgaintf_en_global;
	u32  fpgaintf_en_0;
	u32  fpgaintf_en_1;
	u32  fpgaintf_en_2;
	u32  fpgaintf_en_3;
	u32  _pad_0x74_0x7f[3];
	u32  noc_addr_remap_value;
	u32  noc_addr_remap_set;
	u32  noc_addr_remap_clear;
	u32  _pad_0x8c_0x8f;
	u32  ecc_intmask_value;
	u32  ecc_intmask_set;
	u32  ecc_intmask_clr;
	u32  ecc_intstatus_serr;
	u32  ecc_intstatus_derr;
	u32  mpu_status_l2_ecc;
	u32  mpu_clear_l2_ecc;
	u32  mpu_status_l1_parity;
	u32  mpu_clear_l1_parity;
	u32  mpu_set_l1_parity;
	u32  _pad_0xb8_0xbf[2];
	u32  noc_timeout;
	u32  noc_idlereq_set;
	u32  noc_idlereq_clr;
	u32  noc_idlereq_value;
	u32  noc_idleack;
	u32  noc_idlestatus;
	u32  fpga2soc_ctrl;
	u32  _pad_0xdc_0xff[9];
	u32  tsmc_tsel_0;
	u32  tsmc_tsel_1;
	u32  tsmc_tsel_2;
	u32  tsmc_tsel_3;
	u32  _pad_0x110_0x200[60];
	u32  romhw_ctrl;
	u32  romcode_ctrl;
	u32  romcode_cpu1startaddr;
	u32  romcode_initswstate;
	u32  romcode_initswlastld;
	u32  _pad_0x214_0x217;
	u32  warmram_enable;
	u32  warmram_datastart;
	u32  warmram_length;
	u32  warmram_execution;
	u32  warmram_crc;
	u32  _pad_0x22c_0x22f;
	u32  isw_handoff[8];
	u32  romcode_bootromswstate[8];
};

#define SYSMGR_SDMMC_SMPLSEL_SHIFT	4
#define SYSMGR_BOOTINFO_BSEL_SHIFT	12

#endif /* _SYSTEM_MANAGER_ARRIA10_H_ */
