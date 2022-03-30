/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2017-2019 Intel Corporation <www.intel.com>
 * All rights reserved.
 */

#include <asm/cache.h>
#include <altera.h>
#include <image.h>

#ifndef _FPGA_MANAGER_ARRIA10_H_
#define _FPGA_MANAGER_ARRIA10_H_

#define ALT_FPGAMGR_IMGCFG_STAT_F2S_CRC_ERROR_SET_MSK		BIT(0)
#define ALT_FPGAMGR_IMGCFG_STAT_F2S_EARLY_USERMODE_SET_MSK	BIT(1)
#define ALT_FPGAMGR_IMGCFG_STAT_F2S_USERMODE_SET_MSK 		BIT(2)
#define ALT_FPGAMGR_IMGCFG_STAT_F2S_INITDONE_OE_SET_MSK 	BIT(3)
#define ALT_FPGAMGR_IMGCFG_STAT_F2S_NSTATUS_PIN_SET_MSK		BIT(4)
#define ALT_FPGAMGR_IMGCFG_STAT_F2S_NSTATUS_OE_SET_MSK		BIT(5)
#define ALT_FPGAMGR_IMGCFG_STAT_F2S_CONDONE_PIN_SET_MSK		BIT(6)
#define ALT_FPGAMGR_IMGCFG_STAT_F2S_CONDONE_OE_SET_MSK		BIT(7)
#define ALT_FPGAMGR_IMGCFG_STAT_F2S_CVP_CONF_DONE_SET_MSK	BIT(8)
#define ALT_FPGAMGR_IMGCFG_STAT_F2S_PR_READY_SET_MSK		BIT(9)
#define ALT_FPGAMGR_IMGCFG_STAT_F2S_PR_DONE_SET_MSK		BIT(10)
#define ALT_FPGAMGR_IMGCFG_STAT_F2S_PR_ERROR_SET_MSK		BIT(11)
#define ALT_FPGAMGR_IMGCFG_STAT_F2S_NCONFIG_PIN_SET_MSK		BIT(12)
#define ALT_FPGAMGR_IMGCFG_STAT_F2S_NCEO_OE_SET_MSK		BIT(13)
#define ALT_FPGAMGR_IMGCFG_STAT_F2S_MSEL0_SET_MSK    		BIT(16)
#define ALT_FPGAMGR_IMGCFG_STAT_F2S_MSEL1_SET_MSK    		BIT(17)
#define ALT_FPGAMGR_IMGCFG_STAT_F2S_MSEL2_SET_MSK    		BIT(18)
#define ALT_FPGAMGR_IMGCFG_STAT_F2S_MSEL_SET_MSD (\
	ALT_FPGAMGR_IMGCFG_STAT_F2S_MSEL0_SET_MSK |\
	ALT_FPGAMGR_IMGCFG_STAT_F2S_MSEL1_SET_MSK |\
	ALT_FPGAMGR_IMGCFG_STAT_F2S_MSEL2_SET_MSK)
#define ALT_FPGAMGR_IMGCFG_STAT_F2S_IMGCFG_FIFOEMPTY_SET_MSK	BIT(24)
#define ALT_FPGAMGR_IMGCFG_STAT_F2S_IMGCFG_FIFOFULL_SET_MSK	BIT(25)
#define ALT_FPGAMGR_IMGCFG_STAT_F2S_JTAGM_SET_MSK		BIT(28)
#define ALT_FPGAMGR_IMGCFG_STAT_F2S_EMR_SET_MSK			BIT(29)
#define ALT_FPGAMGR_IMGCFG_STAT_F2S_MSEL0_LSB			16

#define ALT_FPGAMGR_IMGCFG_CTL_00_S2F_NENABLE_NCONFIG_SET_MSK	BIT(0)
#define ALT_FPGAMGR_IMGCFG_CTL_00_S2F_NENABLE_NSTATUS_SET_MSK	BIT(1)
#define ALT_FPGAMGR_IMGCFG_CTL_00_S2F_NENABLE_CONDONE_SET_MSK	BIT(2)
#define ALT_FPGAMGR_IMGCFG_CTL_00_S2F_NCONFIG_SET_MSK		BIT(8)
#define ALT_FPGAMGR_IMGCFG_CTL_00_S2F_NSTATUS_OE_SET_MSK	BIT(16)
#define ALT_FPGAMGR_IMGCFG_CTL_00_S2F_CONDONE_OE_SET_MSK	BIT(24)

#define ALT_FPGAMGR_IMGCFG_CTL_01_S2F_NENABLE_CONFIG_SET_MSK	BIT(0)
#define ALT_FPGAMGR_IMGCFG_CTL_01_S2F_PR_REQUEST_SET_MSK	BIT(16)
#define ALT_FPGAMGR_IMGCFG_CTL_01_S2F_NCE_SET_MSK		BIT(24)

#define ALT_FPGAMGR_IMGCFG_CTL_02_EN_CFG_CTRL_SET_MSK    	BIT(0)
#define ALT_FPGAMGR_IMGCFG_CTL_02_EN_CFG_DATA_SET_MSK    	BIT(8)
#define ALT_FPGAMGR_IMGCFG_CTL_02_CDRATIO_SET_MSK    		0x00030000
#define ALT_FPGAMGR_IMGCFG_CTL_02_CFGWIDTH_SET_MSK		BIT(24)
#define ALT_FPGAMGR_IMGCFG_CTL_02_CDRATIO_LSB			16

#define FPGA_SOCFPGA_A10_RBF_UNENCRYPTED	0xa65c
#define FPGA_SOCFPGA_A10_RBF_ENCRYPTED		0xa65d
#define FPGA_SOCFPGA_A10_RBF_PERIPH		0x0001
#define FPGA_SOCFPGA_A10_RBF_CORE		0x8001
#ifndef __ASSEMBLY__

struct socfpga_fpga_manager {
	u32  _pad_0x0_0x7[2];
	u32  dclkcnt;
	u32  dclkstat;
	u32  gpo;
	u32  gpi;
	u32  misci;
	u32  _pad_0x1c_0x2f[5];
	u32  emr_data0;
	u32  emr_data1;
	u32  emr_data2;
	u32  emr_data3;
	u32  emr_data4;
	u32  emr_data5;
	u32  emr_valid;
	u32  emr_en;
	u32  jtag_config;
	u32  jtag_status;
	u32  jtag_kick;
	u32  _pad_0x5c_0x5f;
	u32  jtag_data_w;
	u32  jtag_data_r;
	u32  _pad_0x68_0x6f[2];
	u32  imgcfg_ctrl_00;
	u32  imgcfg_ctrl_01;
	u32  imgcfg_ctrl_02;
	u32  _pad_0x7c_0x7f;
	u32  imgcfg_stat;
	u32  intr_masked_status;
	u32  intr_mask;
	u32  intr_polarity;
	u32  dma_config;
	u32  imgcfg_fifo_status;
};

enum rbf_type {
	unknown,
	periph_section,
	core_section
};

enum rbf_security {
	invalid,
	unencrypted,
	encrypted
};

struct rbf_info {
	enum rbf_type section;
	enum rbf_security security;
};

struct fpga_loadfs_info {
	fpga_fs_info *fpga_fsinfo;
	u32 remaining;
	u32 offset;
	struct rbf_info rbfinfo;
};

/* Functions */
int fpgamgr_program_init(u32 * rbf_data, size_t rbf_size);
int fpgamgr_program_finish(void);
int is_fpgamgr_user_mode(void);
int fpgamgr_wait_early_user_mode(void);
const char *get_fpga_filename(void);
int is_fpgamgr_early_user_mode(void);
int socfpga_loadfs(fpga_fs_info *fpga_fsinfo, const void *buf, size_t bsize,
		  u32 offset);
void fpgamgr_program(const void *buf, size_t bsize, u32 offset);
#endif /* __ASSEMBLY__ */

#endif /* _FPGA_MANAGER_ARRIA10_H_ */
