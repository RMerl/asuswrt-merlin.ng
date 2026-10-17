/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2015 Linaro
 * Peter Griffin <peter.griffin@linaro.org>
 */

#ifndef __HI6220_ALWAYSON_H__
#define __HI6220_ALWAYSON_H__

#define ALWAYSON_CTRL_BASE			0xF7800000

struct alwayson_sc_regs {
	u32 ctrl0;		/*0x0*/
	u32 ctrl1;
	u32 ctrl2;

	u32 unknown;

	u32 stat0;		/*0x10*/
	u32 stat1;
	u32 mcu_imctrl;
	u32 mcu_imstat;

	u32 unknown_1[9];

	u32 secondary_int_en0;	/*0x44*/
	u32 secondary_int_statr0;
	u32 secondary_int_statm0;

	u32 unknown_2;

	u32 mcu_wkup_int_en6;	/*0x54*/
	u32 mcu_wkup_int_statr6;
	u32 mcu_wkup_int_statm6;

	u32 unknown_3;

	u32 mcu_wkup_int_en5;	/*0x64*/
	u32 mcu_wkup_int_statr5;
	u32 mcu_wkup_int_statm5;

	u32 unknown_4[9];

	u32 mcu_wkup_int_en4;	/*0x94*/
	u32 mcu_wkup_int_statr4;
	u32 mcu_wkup_int_statm4;

	u32 unknown_5[2];

	u32 mcu_wkup_int_en0;	/*0xa8*/
	u32 mcu_wkup_int_statr0;
	u32 mcu_wkup_int_statm0;

	u32 mcu_wkup_int_en1;	/*0xb4*/
	u32 mcu_wkup_int_statr1;
	u32 mcu_wkup_int_statm1;

	u32 unknown_6;

	u32 int_statr;		/*0xc4*/
	u32 int_statm;
	u32 int_clear;

	u32 int_en_set;		/*0xd0*/
	u32 int_en_dis;
	u32 int_en_stat;

	u32 unknown_7[2];

	u32 int_statr1;		/*0xc4*/
	u32 int_statm1;
	u32 int_clear1;

	u32 int_en_set1;	/*0xf0*/
	u32 int_en_dis1;
	u32 int_en_stat1;

	u32 unknown_8[53];

	u32 timer_en0;		/*0x1d0*/
	u32 timer_en1;

	u32 unknown_9[6];

	u32 timer_en4;		/*0x1f0*/
	u32 timer_en5;

	u32 unknown_10[130];

	u32 mcu_subsys_ctrl0;	/*0x400*/
	u32 mcu_subsys_ctrl1;
	u32 mcu_subsys_ctrl2;
	u32 mcu_subsys_ctrl3;
	u32 mcu_subsys_ctrl4;
	u32 mcu_subsys_ctrl5;
	u32 mcu_subsys_ctrl6;
	u32 mcu_subsys_ctrl7;

	u32 unknown_10_1[8];

	u32 mcu_subsys_stat0;	/*0x440*/
	u32 mcu_subsys_stat1;
	u32 mcu_subsys_stat2;
	u32 mcu_subsys_stat3;
	u32 mcu_subsys_stat4;
	u32 mcu_subsys_stat5;
	u32 mcu_subsys_stat6;
	u32 mcu_subsys_stat7;

	u32 unknown_11[116];

	u32 clk4_en;		/*0x630*/
	u32 clk4_dis;
	u32 clk4_stat;

	u32 clk5_en;		/*0x63c*/
	u32 clk5_dis;
	u32 clk5_stat;

	u32 unknown_12[42];

	u32 rst4_en;		/*0x6f0*/
	u32 rst4_dis;
	u32 rst4_stat;

	u32 rst5_en;		/*0x6fc*/
	u32 rst5_dis;
	u32 rst5_stat;

	u32 unknown_13[62];

	u32 pw_clk0_en;		/*0x800*/
	u32 pw_clk0_dis;
	u32 pw_clk0_stat;

	u32 unknown_13_1;

	u32 pw_rst0_en;		/*0x810*/
	u32 pw_rst0_dis;
	u32 pw_rst0_stat;

	u32 unknown_14;

	u32 pw_isoen0;		/*0x820*/
	u32 pw_isodis0;
	u32 pw_iso_stat0;

	u32 unknown_14_1;

	u32 pw_mtcmos_en0;	/*0x830*/
	u32 pw_mtcmos_dis0;
	u32 pw_mtcmos_stat0;
	u32 pw_mtcmos_ack_stat0;
	u32 pw_mtcmos_timeout_stat0;

	u32 unknown_14_2[3];

	u32 pw_stat0;		/*0x850*/
	u32 pw_stat1;

	u32 unknown_15[10];

	u32 systest_stat;	/*0x880*/

	u32 unknown_16[3];

	u32 systest_slicer_cnt0;/*0x890*/
	u32 systest_slicer_cnt1;

	u32 unknown_17[12];

	u32 pw_ctrl1;		/*0x8C8*/
	u32 pw_ctrl;

	u32 mcpu_voteen;
	u32 mcpu_votedis;
	u32 mcpu_votestat;

	u32 unknown_17_1;

	u32 mcpu_vote_msk0;	/*0x8E0*/
	u32 mcpu_vote_msk1;
	u32 mcpu_votestat0_msk;
	u32 mcpu_votestat1_msk;

	u32 peri_voteen;	/*0x8F0*/
	u32 peri_votedis;
	u32 peri_votestat;

	u32 unknown_17_2;

	u32 peri_vote_msk0;	/*0x900*/
	u32 peri_vote_msk1;
	u32 peri_votestat0_msk;
	u32 erpi_votestat1_msk;
	u32 acpu_voteen;
	u32 acpu_votedis;
	u32 acpu_votestat;

	u32 unknown_18;

	u32 acpu_vote_msk0;	/*0x920*/
	u32 acpu_vote_msk1;
	u32 acpu_votestat0_msk;
	u32 acpu_votestat1_msk;
	u32 mcu_voteen;
	u32 mcu_votedis;
	u32 mcu_votestat;

	u32 unknown_18_1;

	u32 mcu_vote_msk0;	/*0x940*/
	u32 mcu_vote_msk1;
	u32 mcu_vote_votestat0_msk;
	u32 mcu_vote_votestat1_msk;

	u32 unknown_18_1_2[4];

	u32 mcu_vote_vote1en;	/*0x960*/
	u32 mcu_vote_vote1dis;
	u32 mcu_vote_vote1stat;

	u32 unknown_18_2;

	u32 mcu_vote_vote1_msk0;/*0x970*/
	u32 mcu_vote_vote1_msk1;
	u32 mcu_vote_vote1stat0_msk;
	u32 mcu_vote_vote1stat1_msk;
	u32 mcu_vote_vote2en;
	u32 mcu_vote_vote2dis;
	u32 mcu_vote_vote2stat;

	u32 unknown_18_3;

	u32 mcu_vote2_msk0;	/*0x990*/
	u32 mcu_vote2_msk1;
	u32 mcu_vote2stat0_msk;
	u32 mcu_vote2stat1_msk;
	u32 vote_ctrl;
	u32 vote_stat;		/*0x9a4*/

	u32 unknown_19[342];

	u32 econum;		/*0xf00*/

	u32 unknown_20_1[3];

	u32 scchipid;		/*0xf10*/

	u32 unknown_20_2[2];

	u32 scsocid;		/*0xf1c*/

	u32 unknown_20[48];

	u32 soc_fpga_rtl_def;	/*0xfe0*/
	u32 soc_fpga_pr_def;
	u32 soc_fpga_res_def0;
	u32 soc_fpga_res_def1;	/*0xfec*/
};

/* ctrl0 bit definitions */

#define ALWAYSON_SC_SYS_CTRL0_MODE_NORMAL			0x004
#define ALWAYSON_SC_SYS_CTRL0_MODE_MASK				0x007

/* ctrl1 bit definitions */

#define ALWAYSON_SC_SYS_CTRL1_AARM_WD_RST_CFG			(1 << 0)
#define ALWAYSON_SC_SYS_CTRL1_REMAP_SRAM_AARM			(1 << 1)
#define ALWAYSON_SC_SYS_CTRL1_EFUSEC_REMAP			(1 << 2)
#define ALWAYSON_SC_SYS_CTRL1_EXT_PLL_SEL			(1 << 3)
#define ALWAYSON_SC_SYS_CTRL1_MCU_WDG0_RSTMCU_CFG		(1 << 4)
#define ALWAYSON_SC_SYS_CTRL1_USIM0_HPD_DE_BOUNCE_CFG		(1 << 6)
#define ALWAYSON_SC_SYS_CTRL1_USIM0_HPD_OE_CFG			(1 << 7)
#define ALWAYSON_SC_SYS_CTRL1_USIM1_HPD_DE_BOUNCE_CFG		(1 << 8)
#define ALWAYSON_SC_SYS_CTRL1_USIM1_HPD_OE_CFG			(1 << 9)
#define ALWAYSON_SC_SYS_CTRL1_BUS_DFS_FORE_HD_CFG		(1 << 10)
#define ALWAYSON_SC_SYS_CTRL1_BUS_DFS_FORE_HD_CFG1		(1 << 11)
#define ALWAYSON_SC_SYS_CTRL1_USIM0_HPD_OE_SFT			(1 << 12)
#define ALWAYSON_SC_SYS_CTRL1_USIM1_HPD_OE_SFT			(1 << 13)
#define ALWAYSON_SC_SYS_CTRL1_MCU_CLKEN_HARDCFG			(1 << 15)
#define ALWAYSON_SC_SYS_CTRL1_AARM_WD_RST_CFG_MSK		(1 << 16)
#define ALWAYSON_SC_SYS_CTRL1_REMAP_SRAM_AARM_MSK		(1 << 17)
#define ALWAYSON_SC_SYS_CTRL1_EFUSEC_REMAP_MSK			(1 << 18)
#define ALWAYSON_SC_SYS_CTRL1_EXT_PLL_SEL_MSK			(1 << 19)
#define ALWAYSON_SC_SYS_CTRL1_MCU_WDG0_RSTMCU_CFG_MSK		(1 << 20)
#define ALWAYSON_SC_SYS_CTRL1_USIM0_HPD_DE_BOUNCE_CFG_MSK	(1 << 22)
#define ALWAYSON_SC_SYS_CTRL1_USIM0_HPD_OE_CFG_MSK		(1 << 23)
#define ALWAYSON_SC_SYS_CTRL1_USIM1_HPD_DE_BOUNCE_CFG_MSK	(1 << 24)
#define ALWAYSON_SC_SYS_CTRL1_USIM1_HPD_OE_CFG_MSK		(1 << 25)
#define ALWAYSON_SC_SYS_CTRL1_BUS_DFS_FORE_HD_CFG_MSK		(1 << 26)
#define ALWAYSON_SC_SYS_CTRL1_BUS_DFS_FORE_HD_CFG1_MSK		(1 << 27)
#define ALWAYSON_SC_SYS_CTRL1_USIM0_HPD_OE_SFT_MSK		(1 << 28)
#define ALWAYSON_SC_SYS_CTRL1_USIM1_HPD_OE_SFT_MSK		(1 << 29)
#define ALWAYSON_SC_SYS_CTRL1_MCU_CLKEN_HARDCFG_MSK		(1 << 31)

/* ctrl2 bit definitions */

#define ALWAYSON_SC_SYS_CTRL2_MCU_SFT_RST_STAT_CLEAR		(1 << 26)
#define ALWAYSON_SC_SYS_CTRL2_MCU_WDG0_RST_STAT_CLEAR		(1 << 27)
#define ALWAYSON_SC_SYS_CTRL2_TSENSOR_RST_STAT_CLEAR		(1 << 28)
#define ALWAYSON_SC_SYS_CTRL2_ACPU_WDG_RST_STAT_CLEAR		(1 << 29)
#define ALWAYSON_SC_SYS_CTRL2_MCU_WDG1_RST_STAT_CLEAR		(1 << 30)
#define ALWAYSON_SC_SYS_CTRL2_GLB_SRST_STAT_CLEAR		(1 << 31)

/* stat0 bit definitions */

#define ALWAYSON_SC_SYS_STAT0_MCU_RST_STAT			(1 << 25)
#define ALWAYSON_SC_SYS_STAT0_MCU_SOFTRST_STAT			(1 << 26)
#define ALWAYSON_SC_SYS_STAT0_MCU_WDGRST_STAT			(1 << 27)
#define ALWAYSON_SC_SYS_STAT0_TSENSOR_HARDRST_STAT		(1 << 28)
#define ALWAYSON_SC_SYS_STAT0_ACPU_WD_GLB_RST_STAT		(1 << 29)
#define ALWAYSON_SC_SYS_STAT0_CM3_WDG1_RST_STAT			(1 << 30)
#define ALWAYSON_SC_SYS_STAT0_GLB_SRST_STAT			(1 << 31)

/* stat1 bit definitions */

#define ALWAYSON_SC_SYS_STAT1_MODE_STATUS			(1 << 0)
#define ALWAYSON_SC_SYS_STAT1_BOOT_SEL_LOCK			(1 << 16)
#define ALWAYSON_SC_SYS_STAT1_FUNC_MODE_LOCK			(1 << 17)
#define ALWAYSON_SC_SYS_STAT1_BOOT_MODE_LOCK			(1 << 19)
#define ALWAYSON_SC_SYS_STAT1_FUN_JTAG_MODE_OUT			(1 << 20)
#define ALWAYSON_SC_SYS_STAT1_SECURITY_BOOT_FLG			(1 << 27)
#define ALWAYSON_SC_SYS_STAT1_EFUSE_NANDBOOT_MSK		(1 << 28)
#define ALWAYSON_SC_SYS_STAT1_EFUSE_NAND_BITWIDE		(1 << 29)

/* ctrl3 bit definitions */

#define ALWAYSON_SC_MCU_SUBSYS_CTRL3_RCLK_3			0x003
#define ALWAYSON_SC_MCU_SUBSYS_CTRL3_RCLK_MASK			0x007
#define ALWAYSON_SC_MCU_SUBSYS_CTRL3_CSSYS_CTRL_PROT		(1 << 3)
#define ALWAYSON_SC_MCU_SUBSYS_CTRL3_TCXO_AFC_OEN_CRG		(1 << 4)
#define ALWAYSON_SC_MCU_SUBSYS_CTRL3_AOB_IO_SEL18_USIM1		(1 << 8)
#define ALWAYSON_SC_MCU_SUBSYS_CTRL3_AOB_IO_SEL18_USIM0		(1 << 9)
#define ALWAYSON_SC_MCU_SUBSYS_CTRL3_AOB_IO_SEL18_SD		(1 << 10)
#define ALWAYSON_SC_MCU_SUBSYS_CTRL3_MCU_SUBSYS_CTRL3_RESERVED	(1 << 11)

/* clk4_en bit definitions */

#define ALWAYSON_SC_PERIPH_CLK4_EN_HCLK_MCU			(1 << 0)
#define ALWAYSON_SC_PERIPH_CLK4_EN_CLK_MCU_DAP			(1 << 3)
#define ALWAYSON_SC_PERIPH_CLK4_EN_PCLK_CM3_TIMER0		(1 << 4)
#define ALWAYSON_SC_PERIPH_CLK4_EN_PCLK_CM3_TIMER1		(1 << 5)
#define ALWAYSON_SC_PERIPH_CLK4_EN_PCLK_CM3_WDT0		(1 << 6)
#define ALWAYSON_SC_PERIPH_CLK4_EN_PCLK_CM3_WDT1		(1 << 7)
#define ALWAYSON_SC_PERIPH_CLK4_EN_HCLK_IPC_S			(1 << 8)
#define ALWAYSON_SC_PERIPH_CLK4_EN_HCLK_IPC_NS			(1 << 9)
#define ALWAYSON_SC_PERIPH_CLK4_EN_PCLK_EFUSEC			(1 << 10)
#define ALWAYSON_SC_PERIPH_CLK4_EN_PCLK_TZPC			(1 << 11)
#define ALWAYSON_SC_PERIPH_CLK4_EN_PCLK_WDT0			(1 << 12)
#define ALWAYSON_SC_PERIPH_CLK4_EN_PCLK_WDT1			(1 << 13)
#define ALWAYSON_SC_PERIPH_CLK4_EN_PCLK_WDT2			(1 << 14)
#define ALWAYSON_SC_PERIPH_CLK4_EN_PCLK_TIMER0			(1 << 15)
#define ALWAYSON_SC_PERIPH_CLK4_EN_PCLK_TIMER1			(1 << 16)
#define ALWAYSON_SC_PERIPH_CLK4_EN_PCLK_TIMER2			(1 << 17)
#define ALWAYSON_SC_PERIPH_CLK4_EN_PCLK_TIMER3			(1 << 18)
#define ALWAYSON_SC_PERIPH_CLK4_EN_PCLK_TIMER4			(1 << 19)
#define ALWAYSON_SC_PERIPH_CLK4_EN_PCLK_TIMER5			(1 << 20)
#define ALWAYSON_SC_PERIPH_CLK4_EN_PCLK_TIMER6			(1 << 21)
#define ALWAYSON_SC_PERIPH_CLK4_EN_PCLK_TIMER7			(1 << 22)
#define ALWAYSON_SC_PERIPH_CLK4_EN_PCLK_TIMER8			(1 << 23)
#define ALWAYSON_SC_PERIPH_CLK4_EN_CLK_UART0			(1 << 24)
#define ALWAYSON_SC_PERIPH_CLK4_EN_CLK_RTC0			(1 << 25)
#define ALWAYSON_SC_PERIPH_CLK4_EN_CLK_RTC1			(1 << 26)
#define ALWAYSON_SC_PERIPH_CLK4_EN_PCLK_PMUSSI			(1 << 27)
#define ALWAYSON_SC_PERIPH_CLK4_EN_CLK_JTAG_AUTH		(1 << 28)
#define ALWAYSON_SC_PERIPH_CLK4_EN_CLK_CS_DAPB_ON		(1 << 29)
#define ALWAYSON_SC_PERIPH_CLK4_EN_CLK_PDM			(1 << 30)
#define ALWAYSON_SC_PERIPH_CLK4_EN_CLK_SSI_PAD			(1 << 31)

/* clk5_en bit definitions */

#define ALWAYSON_SC_PERIPH_CLK5_EN_PCLK_PMUSSI_CCPU		(1 << 0)
#define ALWAYSON_SC_PERIPH_CLK5_EN_PCLK_EFUSEC_CCPU		(1 << 1)
#define ALWAYSON_SC_PERIPH_CLK5_EN_HCLK_IPC_CCPU		(1 << 2)
#define ALWAYSON_SC_PERIPH_CLK5_EN_HCLK_IPC_NS_CCPU		(1 << 3)
#define ALWAYSON_SC_PERIPH_CLK5_EN_PCLK_PMUSSI_MCU		(1 << 16)
#define ALWAYSON_SC_PERIPH_CLK5_EN_PCLK_EFUSEC_MCU		(1 << 17)
#define ALWAYSON_SC_PERIPH_CLK5_EN_HCLK_IPC_MCU			(1 << 18)
#define ALWAYSON_SC_PERIPH_CLK5_EN_HCLK_IPC_NS_MCU		(1 << 19)

/* rst4_dis bit definitions */

#define ALWAYSON_SC_PERIPH_RST4_DIS_RESET_MCU_ECTR_N		(1 << 0)
#define ALWAYSON_SC_PERIPH_RST4_DIS_RESET_MCU_SYS_N		(1 << 1)
#define ALWAYSON_SC_PERIPH_RST4_DIS_RESET_MCU_POR_N		(1 << 2)
#define ALWAYSON_SC_PERIPH_RST4_DIS_RESET_MCU_DAP_N		(1 << 3)
#define ALWAYSON_SC_PERIPH_RST4_DIS_PRESET_CM3_TIMER0_N		(1 << 4)
#define ALWAYSON_SC_PERIPH_RST4_DIS_PRESET_CM3_TIMER1_N		(1 << 5)
#define ALWAYSON_SC_PERIPH_RST4_DIS_PRESET_CM3_WDT0_N		(1 << 6)
#define ALWAYSON_SC_PERIPH_RST4_DIS_PRESET_CM3_WDT1_N		(1 << 7)
#define ALWAYSON_SC_PERIPH_RST4_DIS_HRESET_IPC_S_N		(1 << 8)
#define ALWAYSON_SC_PERIPH_RST4_DIS_HRESET_IPC_NS_N		(1 << 9)
#define ALWAYSON_SC_PERIPH_RST4_DIS_PRESET_EFUSEC_N		(1 << 10)
#define ALWAYSON_SC_PERIPH_RST4_DIS_PRESET_WDT0_N		(1 << 12)
#define ALWAYSON_SC_PERIPH_RST4_DIS_PRESET_WDT1_N		(1 << 13)
#define ALWAYSON_SC_PERIPH_RST4_DIS_PRESET_WDT2_N		(1 << 14)
#define ALWAYSON_SC_PERIPH_RST4_DIS_PRESET_TIMER0_N		(1 << 15)
#define ALWAYSON_SC_PERIPH_RST4_DIS_PRESET_TIMER1_N		(1 << 16)
#define ALWAYSON_SC_PERIPH_RST4_DIS_PRESET_TIMER2_N		(1 << 17)
#define ALWAYSON_SC_PERIPH_RST4_DIS_PRESET_TIMER3_N		(1 << 18)
#define ALWAYSON_SC_PERIPH_RST4_DIS_PRESET_TIMER4_N		(1 << 19)
#define ALWAYSON_SC_PERIPH_RST4_DIS_PRESET_TIMER5_N		(1 << 20)
#define ALWAYSON_SC_PERIPH_RST4_DIS_PRESET_TIMER6_N		(1 << 21)
#define ALWAYSON_SC_PERIPH_RST4_DIS_PRESET_TIMER7_N		(1 << 22)
#define ALWAYSON_SC_PERIPH_RST4_DIS_PRESET_TIMER8_N		(1 << 23)
#define ALWAYSON_SC_PERIPH_RST4_DIS_PRESET_UART0_N		(1 << 24)
#define ALWAYSON_SC_PERIPH_RST4_DIS_RESET_RTC0_N		(1 << 25)
#define ALWAYSON_SC_PERIPH_RST4_DIS_RESET_RTC1_N		(1 << 26)
#define ALWAYSON_SC_PERIPH_RST4_DIS_PRESET_PMUSSI_N		(1 << 27)
#define ALWAYSON_SC_PERIPH_RST4_DIS_RESET_JTAG_AUTH_N		(1 << 28)
#define ALWAYSON_SC_PERIPH_RST4_DIS_RESET_CS_DAPB_ON_N		(1 << 29)
#define ALWAYSON_SC_PERIPH_RST4_DIS_MDM_SUBSYS_GLB		(1 << 30)

#define PCLK_TIMER1						(1 << 16)
#define PCLK_TIMER0						(1 << 15)

#endif /* __HI6220_ALWAYSON_H__ */
