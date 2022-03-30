/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Porting to u-boot:
 *
 * (C) Copyright 2010
 * Stefano Babic, DENX Software Engineering, sbabic@denx.de
 *
 * Linux IPU driver for MX51:
 *
 * (C) Copyright 2005-2009 Freescale Semiconductor, Inc.
 */

#ifndef __IPU_REGS_INCLUDED__
#define __IPU_REGS_INCLUDED__

#define IPU_DISP0_BASE		0x00000000
#define IPU_MCU_T_DEFAULT	8
#define IPU_DISP1_BASE		(IPU_MCU_T_DEFAULT << 25)
#define IPU_CM_REG_BASE		0x00000000
#define IPU_STAT_REG_BASE	0x00000200
#define IPU_IDMAC_REG_BASE	0x00008000
#define IPU_ISP_REG_BASE	0x00010000
#define IPU_DP_REG_BASE		0x00018000
#define IPU_IC_REG_BASE		0x00020000
#define IPU_IRT_REG_BASE	0x00028000
#define IPU_CSI0_REG_BASE	0x00030000
#define IPU_CSI1_REG_BASE	0x00038000
#define IPU_DI0_REG_BASE	0x00040000
#define IPU_DI1_REG_BASE	0x00048000
#define IPU_SMFC_REG_BASE	0x00050000
#define IPU_DC_REG_BASE		0x00058000
#define IPU_DMFC_REG_BASE	0x00060000
#define IPU_VDI_REG_BASE	0x00680000
#if defined(CONFIG_MX51) || defined(CONFIG_MX53)
#define IPU_CPMEM_REG_BASE	0x01000000
#define IPU_LUT_REG_BASE	0x01020000
#define IPU_SRM_REG_BASE	0x01040000
#define IPU_TPM_REG_BASE	0x01060000
#define IPU_DC_TMPL_REG_BASE	0x01080000
#define IPU_ISP_TBPR_REG_BASE	0x010C0000
#elif defined(CONFIG_MX6)
#define IPU_CPMEM_REG_BASE	0x00100000
#define IPU_LUT_REG_BASE	0x00120000
#define IPU_SRM_REG_BASE	0x00140000
#define IPU_TPM_REG_BASE	0x00160000
#define IPU_DC_TMPL_REG_BASE	0x00180000
#define IPU_ISP_TBPR_REG_BASE	0x001C0000
#endif

#define IPU_CTRL_BASE_ADDR	(IPU_SOC_BASE_ADDR + IPU_SOC_OFFSET)

extern u32 *ipu_dc_tmpl_reg;

#define DC_EVT_NF		0
#define DC_EVT_NL		1
#define DC_EVT_EOF		2
#define DC_EVT_NFIELD		3
#define DC_EVT_EOL		4
#define DC_EVT_EOFIELD		5
#define DC_EVT_NEW_ADDR		6
#define DC_EVT_NEW_CHAN		7
#define DC_EVT_NEW_DATA		8

#define DC_EVT_NEW_ADDR_W_0	0
#define DC_EVT_NEW_ADDR_W_1	1
#define DC_EVT_NEW_CHAN_W_0	2
#define DC_EVT_NEW_CHAN_W_1	3
#define DC_EVT_NEW_DATA_W_0	4
#define DC_EVT_NEW_DATA_W_1	5
#define DC_EVT_NEW_ADDR_R_0	6
#define DC_EVT_NEW_ADDR_R_1	7
#define DC_EVT_NEW_CHAN_R_0	8
#define DC_EVT_NEW_CHAN_R_1	9
#define DC_EVT_NEW_DATA_R_0	10
#define DC_EVT_NEW_DATA_R_1	11

/* Software reset for ipu */
#define SW_IPU_RST	8

enum {
	IPU_CONF_DP_EN = 0x00000020,
	IPU_CONF_DI0_EN = 0x00000040,
	IPU_CONF_DI1_EN = 0x00000080,
	IPU_CONF_DMFC_EN = 0x00000400,
	IPU_CONF_DC_EN = 0x00000200,

	DI0_COUNTER_RELEASE = 0x01000000,
	DI1_COUNTER_RELEASE = 0x02000000,

	DI_DW_GEN_ACCESS_SIZE_OFFSET = 24,
	DI_DW_GEN_COMPONENT_SIZE_OFFSET = 16,

	DI_GEN_DI_CLK_EXT = 0x100000,
	DI_GEN_POLARITY_1 = 0x00000001,
	DI_GEN_POLARITY_2 = 0x00000002,
	DI_GEN_POLARITY_3 = 0x00000004,
	DI_GEN_POLARITY_4 = 0x00000008,
	DI_GEN_POLARITY_5 = 0x00000010,
	DI_GEN_POLARITY_6 = 0x00000020,
	DI_GEN_POLARITY_7 = 0x00000040,
	DI_GEN_POLARITY_8 = 0x00000080,
	DI_GEN_POL_CLK = 0x20000,

	DI_POL_DRDY_DATA_POLARITY = 0x00000080,
	DI_POL_DRDY_POLARITY_15 = 0x00000010,
	DI_VSYNC_SEL_OFFSET = 13,

	DC_WR_CH_CONF_FIELD_MODE = 0x00000200,
	DC_WR_CH_CONF_PROG_TYPE_OFFSET = 5,
	DC_WR_CH_CONF_PROG_TYPE_MASK = 0x000000E0,
	DC_WR_CH_CONF_PROG_DI_ID = 0x00000004,
	DC_WR_CH_CONF_PROG_DISP_ID_OFFSET = 3,
	DC_WR_CH_CONF_PROG_DISP_ID_MASK = 0x00000018,

	DP_COM_CONF_FG_EN = 0x00000001,
	DP_COM_CONF_GWSEL = 0x00000002,
	DP_COM_CONF_GWAM = 0x00000004,
	DP_COM_CONF_GWCKE = 0x00000008,
	DP_COM_CONF_CSC_DEF_MASK = 0x00000300,
	DP_COM_CONF_CSC_DEF_OFFSET = 8,
	DP_COM_CONF_CSC_DEF_FG = 0x00000300,
	DP_COM_CONF_CSC_DEF_BG = 0x00000200,
	DP_COM_CONF_CSC_DEF_BOTH = 0x00000100,
	DP_COM_CONF_GAMMA_EN = 0x00001000,
	DP_COM_CONF_GAMMA_YUV_EN = 0x00002000,
};

enum di_pins {
	DI_PIN11 = 0,
	DI_PIN12 = 1,
	DI_PIN13 = 2,
	DI_PIN14 = 3,
	DI_PIN15 = 4,
	DI_PIN16 = 5,
	DI_PIN17 = 6,
	DI_PIN_CS = 7,

	DI_PIN_SER_CLK = 0,
	DI_PIN_SER_RS = 1,
};

enum di_sync_wave {
	DI_SYNC_NONE = -1,
	DI_SYNC_CLK = 0,
	DI_SYNC_INT_HSYNC = 1,
	DI_SYNC_HSYNC = 2,
	DI_SYNC_VSYNC = 3,
	DI_SYNC_DE = 5,
};

struct ipu_cm {
	u32 conf;
	u32 sisg_ctrl0;
	u32 sisg_ctrl1;
	u32 sisg_set[6];
	u32 sisg_clear[6];
	u32 int_ctrl[15];
	u32 sdma_event[10];
	u32 srm_pri1;
	u32 srm_pri2;
	u32 fs_proc_flow[3];
	u32 fs_disp_flow[2];
	u32 skip;
	u32 disp_alt_conf;
	u32 disp_gen;
	u32 disp_alt[4];
	u32 snoop;
	u32 mem_rst;
	u32 pm;
	u32 gpr;
	u32 reserved0[26];
	u32 ch_db_mode_sel[2];
	u32 reserved1[4];
	u32 alt_ch_db_mode_sel[2];
	u32 reserved2[2];
	u32 ch_trb_mode_sel[2];
};

struct ipu_idmac {
	u32 conf;
	u32 ch_en[2];
	u32 sep_alpha;
	u32 alt_sep_alpha;
	u32 ch_pri[2];
	u32 wm_en[2];
	u32 lock_en[2];
	u32 sub_addr[5];
	u32 bndm_en[2];
	u32 sc_cord[2];
	u32 reserved[44];
	u32 ch_busy[2];
};

struct ipu_com_async {
	u32 com_conf_async;
	u32 graph_wind_ctrl_async;
	u32 fg_pos_async;
	u32 cur_pos_async;
	u32 cur_map_async;
	u32 gamma_c_async[8];
	u32 gamma_s_async[4];
	u32 dp_csca_async[4];
	u32 dp_csc_async[2];
};

struct ipu_dp {
	u32 com_conf_sync;
	u32 graph_wind_ctrl_sync;
	u32 fg_pos_sync;
	u32 cur_pos_sync;
	u32 cur_map_sync;
	u32 gamma_c_sync[8];
	u32 gamma_s_sync[4];
	u32 csca_sync[4];
	u32 csc_sync[2];
	u32 cur_pos_alt;
	struct ipu_com_async async[2];
};

struct ipu_di {
	u32 general;
	u32 bs_clkgen0;
	u32 bs_clkgen1;
	u32 sw_gen0[9];
	u32 sw_gen1[9];
	u32 sync_as;
	u32 dw_gen[12];
	u32 dw_set[48];
	u32 stp_rep[4];
	u32 stp_rep9;
	u32 ser_conf;
	u32 ssc;
	u32 pol;
	u32 aw0;
	u32 aw1;
	u32 scr_conf;
	u32 stat;
};

struct ipu_stat {
	u32 int_stat[15];
	u32 cur_buf[2];
	u32 alt_cur_buf_0;
	u32 alt_cur_buf_1;
	u32 srm_stat;
	u32 proc_task_stat;
	u32 disp_task_stat;
	u32 triple_cur_buf[4];
	u32 ch_buf0_rdy[2];
	u32 ch_buf1_rdy[2];
	u32 alt_ch_buf0_rdy[2];
	u32 alt_ch_buf1_rdy[2];
	u32 ch_buf2_rdy[2];
};

struct ipu_dc_ch {
	u32 wr_ch_conf;
	u32 wr_ch_addr;
	u32 rl[5];
};

struct ipu_dc {
	struct ipu_dc_ch dc_ch0_1_2[3];
	u32 cmd_ch_conf_3;
	u32 cmd_ch_conf_4;
	struct ipu_dc_ch dc_ch5_6[2];
	struct ipu_dc_ch dc_ch8;
	u32 rl6_ch_8;
	struct ipu_dc_ch dc_ch9;
	u32 rl6_ch_9;
	u32 gen;
	u32 disp_conf1[4];
	u32 disp_conf2[4];
	u32 di0_conf[2];
	u32 di1_conf[2];
	u32 dc_map_ptr[15];
	u32 dc_map_val[12];
	u32 udge[16];
	u32 lla[2];
	u32 r_lla[2];
	u32 wr_ch_addr_5_alt;
	u32 stat;
};

struct ipu_dmfc {
	u32 rd_chan;
	u32 wr_chan;
	u32 wr_chan_def;
	u32 dp_chan;
	u32 dp_chan_def;
	u32 general[2];
	u32 ic_ctrl;
	u32 wr_chan_alt;
	u32 wr_chan_def_alt;
	u32 general1_alt;
	u32 stat;
};

#define IPU_CM_REG		((struct ipu_cm *)(IPU_CTRL_BASE_ADDR + \
				IPU_CM_REG_BASE))
#define IPU_CONF		(&IPU_CM_REG->conf)
#define IPU_SRM_PRI1		(&IPU_CM_REG->srm_pri1)
#define IPU_SRM_PRI2		(&IPU_CM_REG->srm_pri2)
#define IPU_FS_PROC_FLOW1	(&IPU_CM_REG->fs_proc_flow[0])
#define IPU_FS_PROC_FLOW2	(&IPU_CM_REG->fs_proc_flow[1])
#define IPU_FS_PROC_FLOW3	(&IPU_CM_REG->fs_proc_flow[2])
#define IPU_FS_DISP_FLOW1	(&IPU_CM_REG->fs_disp_flow[0])
#define IPU_DISP_GEN		(&IPU_CM_REG->disp_gen)
#define IPU_MEM_RST		(&IPU_CM_REG->mem_rst)
#define IPU_GPR			(&IPU_CM_REG->gpr)
#define IPU_CHA_DB_MODE_SEL(ch)	(&IPU_CM_REG->ch_db_mode_sel[ch / 32])

#define IPU_STAT		((struct ipu_stat *)(IPU_CTRL_BASE_ADDR + \
				IPU_STAT_REG_BASE))
#define IPU_INT_STAT(n)		(&IPU_STAT->int_stat[(n) - 1])
#define IPU_CHA_CUR_BUF(ch)	(&IPU_STAT->cur_buf[ch / 32])
#define IPU_CHA_BUF0_RDY(ch)	(&IPU_STAT->ch_buf0_rdy[ch / 32])
#define IPU_CHA_BUF1_RDY(ch)	(&IPU_STAT->ch_buf1_rdy[ch / 32])
#define IPUIRQ_2_STATREG(irq)	(IPU_INT_STAT(1) + ((irq) / 32))
#define IPUIRQ_2_MASK(irq)	(1UL << ((irq) & 0x1F))

#define IPU_INT_CTRL(n)		(&IPU_CM_REG->int_ctrl[(n) - 1])

#define IDMAC_REG		((struct ipu_idmac *)(IPU_CTRL_BASE_ADDR + \
				IPU_IDMAC_REG_BASE))
#define IDMAC_CONF		(&IDMAC_REG->conf)
#define IDMAC_CHA_EN(ch)	(&IDMAC_REG->ch_en[ch / 32])
#define IDMAC_CHA_PRI(ch)	(&IDMAC_REG->ch_pri[ch / 32])

#define DI_REG(di)		((struct ipu_di *)(IPU_CTRL_BASE_ADDR + \
				((di == 1) ? IPU_DI1_REG_BASE : \
				IPU_DI0_REG_BASE)))
#define DI_GENERAL(di)		(&DI_REG(di)->general)
#define DI_BS_CLKGEN0(di)	(&DI_REG(di)->bs_clkgen0)
#define DI_BS_CLKGEN1(di)	(&DI_REG(di)->bs_clkgen1)

#define DI_SW_GEN0(di, gen)	(&DI_REG(di)->sw_gen0[gen - 1])
#define DI_SW_GEN1(di, gen)	(&DI_REG(di)->sw_gen1[gen - 1])
#define DI_STP_REP(di, gen)	(&DI_REG(di)->stp_rep[(gen - 1) / 2])
#define DI_STP_REP9(di)		(&DI_REG(di)->stp_rep9)
#define DI_SYNC_AS_GEN(di)	(&DI_REG(di)->sync_as)
#define DI_DW_GEN(di, gen)	(&DI_REG(di)->dw_gen[gen])
#define DI_DW_SET(di, gen, set)	(&DI_REG(di)->dw_set[gen + 12 * set])
#define DI_POL(di)		(&DI_REG(di)->pol)
#define DI_SCR_CONF(di)		(&DI_REG(di)->scr_conf)

#define DMFC_REG		((struct ipu_dmfc *)(IPU_CTRL_BASE_ADDR + \
				IPU_DMFC_REG_BASE))
#define DMFC_WR_CHAN		(&DMFC_REG->wr_chan)
#define DMFC_WR_CHAN_DEF	(&DMFC_REG->wr_chan_def)
#define DMFC_DP_CHAN		(&DMFC_REG->dp_chan)
#define DMFC_DP_CHAN_DEF	(&DMFC_REG->dp_chan_def)
#define DMFC_GENERAL1		(&DMFC_REG->general[0])
#define DMFC_IC_CTRL		(&DMFC_REG->ic_ctrl)


#define DC_REG			((struct ipu_dc *)(IPU_CTRL_BASE_ADDR + \
				IPU_DC_REG_BASE))
#define DC_MAP_CONF_PTR(n)	(&DC_REG->dc_map_ptr[n / 2])
#define DC_MAP_CONF_VAL(n)	(&DC_REG->dc_map_val[n / 2])


static inline struct ipu_dc_ch *dc_ch_offset(int ch)
{
	switch (ch) {
	case 0:
	case 1:
	case 2:
		return &DC_REG->dc_ch0_1_2[ch];
	case 5:
	case 6:
		return &DC_REG->dc_ch5_6[ch - 5];
	case 8:
		return &DC_REG->dc_ch8;
	case 9:
		return &DC_REG->dc_ch9;
	default:
		printf("%s: invalid channel %d\n", __func__, ch);
		return NULL;
	}

}

#define DC_RL_CH(ch, evt)	(&dc_ch_offset(ch)->rl[evt / 2])

#define DC_WR_CH_CONF(ch)	(&dc_ch_offset(ch)->wr_ch_conf)
#define DC_WR_CH_ADDR(ch)	(&dc_ch_offset(ch)->wr_ch_addr)

#define DC_WR_CH_CONF_1		DC_WR_CH_CONF(1)
#define DC_WR_CH_CONF_5		DC_WR_CH_CONF(5)

#define DC_GEN			(&DC_REG->gen)
#define DC_DISP_CONF2(disp)	(&DC_REG->disp_conf2[disp])
#define DC_STAT			(&DC_REG->stat)

#define DP_SYNC 0
#define DP_ASYNC0 0x60
#define DP_ASYNC1 0xBC

#define DP_REG			((struct ipu_dp *)(IPU_CTRL_BASE_ADDR + \
				IPU_DP_REG_BASE))
#define DP_COM_CONF()		(&DP_REG->com_conf_sync)
#define DP_GRAPH_WIND_CTRL()	(&DP_REG->graph_wind_ctrl_sync)
#define DP_CSC_A_0()		(&DP_REG->csca_sync[0])
#define DP_CSC_A_1()		(&DP_REG->csca_sync[1])
#define DP_CSC_A_2()		(&DP_REG->csca_sync[2])
#define DP_CSC_A_3()		(&DP_REG->csca_sync[3])

#define DP_CSC_0()		(&DP_REG->csc_sync[0])
#define DP_CSC_1()		(&DP_REG->csc_sync[1])

/* DC template opcodes */
#define WROD(lf)		(0x18 | (lf << 1))

#endif
