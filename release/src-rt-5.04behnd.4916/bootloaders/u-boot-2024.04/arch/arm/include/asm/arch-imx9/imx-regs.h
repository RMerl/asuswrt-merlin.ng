/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2022 NXP
 */

#ifndef __ASM_ARCH_IMX9_REGS_H__
#define __ASM_ARCH_IMX9_REGS_H__

#define ARCH_MXC
#define FEC_QUIRK_ENET_MAC

#define IOMUXC_BASE_ADDR	0x443C0000UL
#define CCM_BASE_ADDR		0x44450000UL
#define CCM_CCGR_BASE_ADDR	0x44458000UL
#define SYSCNT_CTRL_BASE_ADDR	0x44290000

#define ANATOP_BASE_ADDR    0x44480000UL

#define WDG3_BASE_ADDR      0x42490000UL
#define WDG4_BASE_ADDR      0x424a0000UL
#define WDG5_BASE_ADDR      0x424b0000UL

#define FSB_BASE_ADDR       0x47510000UL

#define ANATOP_BASE_ADDR    0x44480000UL

#define BLK_CTRL_WAKEUPMIX_BASE_ADDR 0x42420000
#define BLK_CTRL_S_ANOMIX_BASE_ADDR  0x444f0000

#define SRC_IPS_BASE_ADDR	(0x44460000)
#define SRC_GLOBAL_RBASE	(SRC_IPS_BASE_ADDR + 0x0000)

#define SRC_DDR_RBASE		(SRC_IPS_BASE_ADDR + 0x1000)
#define SRC_ML_RBASE		(SRC_IPS_BASE_ADDR + 0x1800)
#define SRC_MEDIA_RBASE		(SRC_IPS_BASE_ADDR + 0x2400)
#define SRC_M33P_RBASE		(SRC_IPS_BASE_ADDR + 0x2800)

#define SRC_MIX_SLICE_FUNC_STAT_PSW_STAT BIT(0)
#define SRC_MIX_SLICE_FUNC_STAT_RST_STAT BIT(2)
#define SRC_MIX_SLICE_FUNC_STAT_ISO_STAT BIT(4)
#define SRC_MIX_SLICE_FUNC_STAT_MEM_STAT BIT(12)

#define IMG_CONTAINER_BASE             (0x80000000UL)

#define BCTRL_GPR_ENET_QOS_INTF_MODE_MASK        GENMASK(3, 1)
#define BCTRL_GPR_ENET_QOS_INTF_SEL_MII          (0x0 << 1)
#define BCTRL_GPR_ENET_QOS_INTF_SEL_RMII         (0x4 << 1)
#define BCTRL_GPR_ENET_QOS_INTF_SEL_RGMII        (0x1 << 1)
#define BCTRL_GPR_ENET_QOS_CLK_GEN_EN            (0x1 << 0)

#define MARKETING_GRADING_MASK	GENMASK(5, 4)
#define SPEED_GRADING_MASK	GENMASK(11, 6)

#if !(defined(__KERNEL_STRICT_NAMES) || defined(__ASSEMBLY__))
#include <asm/types.h>
#include <stdbool.h>

struct mu_type {
	u32 ver;
	u32 par;
	u32 cr;
	u32 sr;
	u32 reserved0[60];
	u32 fcr;
	u32 fsr;
	u32 reserved1[2];
	u32 gier;
	u32 gcr;
	u32 gsr;
	u32 reserved2;
	u32 tcr;
	u32 tsr;
	u32 rcr;
	u32 rsr;
	u32 reserved3[52];
	u32 tr[16];
	u32 reserved4[16];
	u32 rr[16];
	u32 reserved5[14];
	u32 mu_attr;
};

enum mix_power_domain {
	MIX_PD_MEDIAMIX,
	MIX_PD_MLMIX,
	MIX_PD_DDRMIX,
};

enum src_mix_slice_id {
	SRC_MIX_EDGELOCK = 0,
	SRC_MIX_AONMIX = 1,
	SRC_MIX_WAKEUPMIX = 2,
	SRC_MIX_DDRMIX = 3,
	SRC_MIX_DDRPHY = 4,
	SRC_MIX_ML = 5,
	SRC_MIX_NIC = 6,
	SRC_MIX_HSIO = 7,
	SRC_MIX_MEDIA = 8,
	SRC_MIX_CM33 = 9,
	SRC_MIX_CA55C0 = 10,
	SRC_MIX_CA55C1 = 11,
	SRC_MIX_CA55CLUSTER = 12,
};

enum src_mem_slice_id {
	SRC_MEM_AONMIX = 0,
	SRC_MEM_WAKEUPMIX = 1,
	SRC_MEM_DDRMIX = 2,
	SRC_MEM_DDRPHY = 3,
	SRC_MEM_ML = 4,
	SRC_MEM_NIC = 5,
	SRC_MEM_OCRAM = 6,
	SRC_MEM_HSIO = 7,
	SRC_MEM_MEDIA = 8,
	SRC_MEM_CA55C0 = 9,
	SRC_MEM_CA55C1 = 10,
	SRC_MEM_CA55CLUSTER = 11,
	SRC_MEM_L3 = 12,
};

struct blk_ctrl_s_aonmix_regs {
	u32 cm33_irq_mask[7];
	u32 initnsvtor;
	u32 reserved1[8];
	u32 ca55_irq_mask[7];
	u32 initsvtor;
	u32 m33_cfg;
	u32 reserved2[11];
	u32 axbs_aon_ctrl;
	u32 reserved3[27];
	u32 dap_access_stkybit;
	u32 reserved4[3];
	u32 lp_handshake[2];
	u32 ca55_cpuwait;
	u32 ca55_rvbaraddr0_l;
	u32 ca55_rvbaraddr0_h;
	u32 ca55_rvbaraddr1_l;
	u32 ca55_rvbaraddr1_h;
	u32 s401_irq_mask;
	u32 s401_reset_req_mask;
	u32 s401_halt_st;
	u32 ca55_mode;
	u32 nmi_mask;
	u32 nmi_clr;
	u32 wdog_any_mask;
	u32 s4v1_ipi_noclk_ref1;
};

struct blk_ctrl_wakeupmix_regs {
	u32 upper_addr;
	u32 ipg_debug_cm33;
	u32 reserved[2];
	u32 qch_dis;
	u32 ssi;
	u32 reserved1[1];
	u32 dexsc_err;
	u32 mqs_setting;
	u32 sai_clk_sel;
	u32 eqos_gpr;
	u32 enet_clk_sel;
	u32 reserved2[1];
	u32 volt_detect;
	u32 i3c2_wakeup;
	u32 ipg_debug_ca55c0;
	u32 ipg_debug_ca55c1;
	u32 axi_attr_cfg;
	u32 i3c2_sda_irq;
};

struct src_general_regs {
	u32 reserved[1];
	u32 authen_ctrl;
	u32 reserved1[2];
	u32 scr;
	u32 srtmr;
	u32 srmask;
	u32 reserved2[1];
	u32 srmr[6];
	u32 reserved3[2];
	u32 sbmr[2];
	u32 reserved4[2];
	u32 srsr;
	u32 gpr[19];
	u32 reserved5[24];
	u32 gpr20;
	u32 cm_quiesce;
	u32 cold_reset_ssar_ack_ctrl;
	u32 sp_iso_ctrl;
	u32 rom_lp_ctrl;
	u32 a55_deny_stat;
};

struct src_mem_slice_regs {
	u32 reserved[1];
	u32 mem_ctrl;
	u32 memlp_ctrl_0;
	u32 reserved1[1];
	u32 memlp_ctrl_1;
	u32 memlp_ctrl_2;
	u32 mem_stat;
};

struct src_mix_slice_regs {
	u32 reserved[1];
	u32 authen_ctrl;
	u32 reserved1[2];
	u32 lpm_setting[3];
	u32 reserved2[1];
	u32 slice_sw_ctrl;
	u32 single_reset_sw_ctrl;
	u32 reserved3[6];
	u32 a55_hdsk_ack_ctrl;
	u32 a55_hdsk_ack_stat;
	u32 reserved4[2];
	u32 ssar_ack_ctrl;
	u32 ssar_ack_stat;
	u32 reserved5[1];
	u32 iso_off_dly_por;
	u32 iso_on_dly;
	u32 iso_off_dly;
	u32 psw_off_lf_dly;
	u32 reserved6[1];
	u32 psw_off_hf_dly;
	u32 psw_on_lf_dly;
	u32 psw_on_hf_dly;
	u32 reserved7[1];
	u32 psw_ack_ctrl[2];
	u32 psw_ack_stat;
	u32 reserved8[1];
	u32 mtr_ack_ctrl;
	u32 mtr_ack_stat;
	u32 reserved9[2];
	u32 upi_stat[4];
	u32 fsm_stat;
	u32 func_stat;
};
#endif

#endif
