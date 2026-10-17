/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2017 NXP
 */

#ifndef __ASM_ARCH_IMX8M_REGS_H__
#define __ASM_ARCH_IMX8M_REGS_H__

#define ARCH_MXC

#include <asm/mach-imx/regs-lcdif.h>

#define ROM_VERSION_A0		IS_ENABLED(CONFIG_IMX8MQ) ? 0x800 : 0x800
#define ROM_VERSION_B0		IS_ENABLED(CONFIG_IMX8MQ) ? 0x83C : 0x800

#define MCU_BOOTROM_BASE_ADDR   0x007E0000

#define GPIO1_BASE_ADDR		0X30200000
#define GPIO2_BASE_ADDR		0x30210000
#define GPIO3_BASE_ADDR		0x30220000
#define GPIO4_BASE_ADDR		0x30230000
#define GPIO5_BASE_ADDR		0x30240000
#define WDOG1_BASE_ADDR		0x30280000
#define WDOG2_BASE_ADDR		0x30290000
#define WDOG3_BASE_ADDR		0x302A0000
#define IOMUXC_BASE_ADDR	0x30330000
#define IOMUXC_GPR_BASE_ADDR	0x30340000
#define OCOTP_BASE_ADDR		0x30350000
#define ANATOP_BASE_ADDR	0x30360000
#define SNVS_BASE_ADDR		0x30370000
#define CCM_BASE_ADDR		0x30380000
#define SRC_BASE_ADDR		0x30390000
#define GPC_BASE_ADDR		0x303A0000
#define CSU_BASE_ADDR		0x303E0000

#define SYSCNT_RD_BASE_ADDR	0x306A0000
#define SYSCNT_CMP_BASE_ADDR	0x306B0000
#define SYSCNT_CTRL_BASE_ADDR	0x306C0000

#define UART1_BASE_ADDR		0x30860000
#define UART3_BASE_ADDR		0x30880000
#define UART2_BASE_ADDR		0x30890000
#define CAAM_BASE_ADDR		0x30900000
#define I2C1_BASE_ADDR		0x30A20000
#define I2C2_BASE_ADDR		0x30A30000
#define I2C3_BASE_ADDR		0x30A40000
#define I2C4_BASE_ADDR		0x30A50000
#define UART4_BASE_ADDR		0x30A60000
#ifdef CONFIG_IMX8MP
#define I2C5_BASE_ADDR          0x30AD0000
#define I2C6_BASE_ADDR          0x30AE0000
#endif
#define USDHC1_BASE_ADDR	0x30B40000
#define USDHC2_BASE_ADDR	0x30B50000
#define QSPI0_AMBA_BASE     0x08000000
#if defined(CONFIG_IMX8MM) || defined(CONFIG_IMX8MP)
#define USDHC3_BASE_ADDR	0x30B60000
#endif
#define UART_BASE_ADDR(n)	(			\
	!!sizeof(struct {				\
		static_assert((n) >= 1 && (n) <= 4);	\
		int pad;				\
		}) * (					\
	(n) == 1 ? UART1_BASE_ADDR :			\
	(n) == 2 ? UART2_BASE_ADDR :			\
	(n) == 3 ? UART3_BASE_ADDR :			\
	UART4_BASE_ADDR)				\
	)

#define TZASC_BASE_ADDR		0x32F80000

#define MXS_LCDIF_BASE		IS_ENABLED(CONFIG_IMX8MQ) ? \
					0x30320000 : 0x32e00000

#define SRC_IPS_BASE_ADDR	0x30390000
#define SRC_DDRC_RCR_ADDR	0x30391000
#define SRC_DDRC2_RCR_ADDR	0x30391004

#define APBH_DMA_ARB_BASE_ADDR	0x33000000
#define APBH_DMA_ARB_END_ADDR	0x33007FFF
#define MXS_APBH_BASE		APBH_DMA_ARB_BASE_ADDR

#define MXS_GPMI_BASE		(APBH_DMA_ARB_BASE_ADDR + 0x02000)
#define MXS_BCH_BASE		(APBH_DMA_ARB_BASE_ADDR + 0x04000)

#define GICD_BASE		0x38800000
#define GICR_BASE		0x38880000

#define DDRC_DDR_SS_GPR0	0x3d000000
#define DDRC_IPS_BASE_ADDR(X)	(0x3d400000 + ((X) * 0x2000000))
#define DDR_CSD1_BASE_ADDR	0x40000000

#define IOMUXC_GPR_GPR1_GPR_ENET1_RGMII_EN		BIT(22)
#define IOMUXC_GPR_GPR1_GPR_ENET_QOS_RGMII_EN		BIT(21)
#define IOMUXC_GPR_GPR1_GPR_ENET_QOS_CLK_TX_CLK_SEL	BIT(20)
#define IOMUXC_GPR_GPR1_GPR_ENET_QOS_CLK_GEN_EN		BIT(19)
#define IOMUXC_GPR_GPR1_GPR_ENET_QOS_INTF_SEL_MASK	GENMASK(18, 16)
#define IOMUXC_GPR_GPR1_GPR_ENET_QOS_INTF_SEL_MII	(0 << 16)
#define IOMUXC_GPR_GPR1_GPR_ENET_QOS_INTF_SEL_RGMII	(1 << 16)
#define IOMUXC_GPR_GPR1_GPR_ENET_QOS_INTF_SEL_RMII	(4 << 16)
#define IOMUXC_GPR_GPR1_GPR_ENET1_TX_CLK_SEL		BIT(13)
#define FEC_QUIRK_ENET_MAC

#ifdef CONFIG_ARMV8_PSCI	/* Final jump location */
#define CPU_RELEASE_ADDR               0x900000
#endif

#define CAAM_ARB_BASE_ADDR              (0x00100000)
#define CAAM_ARB_END_ADDR               (0x00107FFF)
#define CAAM_IPS_BASE_ADDR              (0x30900000)
#define CFG_SYS_FSL_SEC_OFFSET       (0)
#define CFG_SYS_FSL_SEC_ADDR         (CAAM_IPS_BASE_ADDR + \
					 CFG_SYS_FSL_SEC_OFFSET)
#define CFG_SYS_FSL_JR0_OFFSET       (0x1000)
#define CFG_SYS_FSL_JR0_ADDR         (CFG_SYS_FSL_SEC_ADDR + \
					 CFG_SYS_FSL_JR0_OFFSET)
#if !defined(__ASSEMBLY__)
#include <asm/types.h>
#include <linux/bitops.h>
#include <stdbool.h>

#define GPR_TZASC_EN					BIT(0)
#define GPR_TZASC_ID_SWAP_BYPASS		BIT(1)
#define GPR_TZASC_EN_LOCK				BIT(16)
#define GPR_TZASC_ID_SWAP_BYPASS_LOCK	BIT(17)

#define SRC_SCR_M4_ENABLE_OFFSET	3
#define SRC_SCR_M4_ENABLE_MASK		BIT(3)
#define SRC_SCR_M4C_NON_SCLR_RST_OFFSET	0
#define SRC_SCR_M4C_NON_SCLR_RST_MASK	BIT(0)
#define SRC_DDR1_ENABLE_MASK		0x8F000000UL
#define SRC_DDR2_ENABLE_MASK		0x8F000000UL
#define SRC_DDR1_RCR_PHY_PWROKIN_N_MASK	BIT(3)
#define SRC_DDR1_RCR_PHY_RESET_MASK	BIT(2)
#define SRC_DDR1_RCR_CORE_RESET_N_MASK	BIT(1)
#define SRC_DDR1_RCR_PRESET_N_MASK	BIT(0)

#define SNVS_LPSR			0x4c
#define SNVS_LPLVDR			0x64
#define SNVS_LPPGDR_INIT		0x41736166

struct iomuxc_gpr_base_regs {
	u32 gpr[47];
};

struct ocotp_regs {
	u32	ctrl;
	u32	ctrl_set;
	u32     ctrl_clr;
	u32	ctrl_tog;
	u32	timing;
	u32     rsvd0[3];
	u32     data;
	u32     rsvd1[3];
	u32     read_ctrl;
	u32     rsvd2[3];
	u32	read_fuse_data;
	u32     rsvd3[3];
	u32	sw_sticky;
	u32     rsvd4[3];
	u32     scs;
	u32     scs_set;
	u32     scs_clr;
	u32     scs_tog;
	u32     crc_addr;
	u32     rsvd5[3];
	u32     crc_value;
	u32     rsvd6[3];
	u32     version;
	u32     rsvd7[0xdb];

	/* fuse banks */
	struct fuse_bank {
		u32	fuse_regs[0x10];
	} bank[0];
};

#ifdef CONFIG_IMX8MP
struct fuse_bank0_regs {
	u32 lock;
	u32 rsvd0[7];
	u32 uid_low;
	u32 rsvd1[3];
	u32 uid_high;
	u32 rsvd2[3];
};
#else
struct fuse_bank0_regs {
	u32 lock;
	u32 rsvd0[3];
	u32 uid_low;
	u32 rsvd1[3];
	u32 uid_high;
	u32 rsvd2[7];
};
#endif

struct fuse_bank1_regs {
	u32 tester3;
	u32 rsvd0[3];
	u32 tester4;
	u32 rsvd1[3];
	u32 tester5;
	u32 rsvd2[3];
	u32 cfg0;
	u32 rsvd3[3];
};

struct fuse_bank3_regs {
	u32 mem_trim0;
	u32 rsvd0[3];
	u32 mem_trim1;
	u32 rsvd1[3];
	u32 mem_trim2;
	u32 rsvd2[3];
	u32 ana0;
	u32 rsvd3[3];
};

struct fuse_bank9_regs {
	u32 mac_addr0;
	u32 rsvd0[3];
	u32 mac_addr1;
	u32 rsvd1[11];
};

struct fuse_bank38_regs {
	u32 ana_trim1; /* trim0 is at 0xD70, bank 37*/
	u32 rsvd0[3];
	u32 ana_trim2;
	u32 rsvd1[3];
	u32 ana_trim3;
	u32 rsvd2[3];
	u32 ana_trim4;
	u32 rsvd3[3];
};

struct fuse_bank39_regs {
	u32 ana_trim5;
	u32 rsvd[15];
};

#ifdef CONFIG_IMX8MQ
struct anamix_pll {
	u32 audio_pll1_cfg0;
	u32 audio_pll1_cfg1;
	u32 audio_pll2_cfg0;
	u32 audio_pll2_cfg1;
	u32 video_pll_cfg0;
	u32 video_pll_cfg1;
	u32 gpu_pll_cfg0;
	u32 gpu_pll_cfg1;
	u32 vpu_pll_cfg0;
	u32 vpu_pll_cfg1;
	u32 arm_pll_cfg0;
	u32 arm_pll_cfg1;
	u32 sys_pll1_cfg0;
	u32 sys_pll1_cfg1;
	u32 sys_pll1_cfg2;
	u32 sys_pll2_cfg0;
	u32 sys_pll2_cfg1;
	u32 sys_pll2_cfg2;
	u32 sys_pll3_cfg0;
	u32 sys_pll3_cfg1;
	u32 sys_pll3_cfg2;
	u32 video_pll2_cfg0;
	u32 video_pll2_cfg1;
	u32 video_pll2_cfg2;
	u32 dram_pll_cfg0;
	u32 dram_pll_cfg1;
	u32 dram_pll_cfg2;
	u32 digprog;
	u32 osc_misc_cfg;
	u32 pllout_monitor_cfg;
	u32 frac_pllout_div_cfg;
	u32 sscg_pllout_div_cfg;
};
#else
struct anamix_pll {
	u32 audio_pll1_gnrl_ctl;
	u32 audio_pll1_fdiv_ctl0;
	u32 audio_pll1_fdiv_ctl1;
	u32 audio_pll1_sscg_ctl;
	u32 audio_pll1_mnit_ctl;
	u32 audio_pll2_gnrl_ctl;
	u32 audio_pll2_fdiv_ctl0;
	u32 audio_pll2_fdiv_ctl1;
	u32 audio_pll2_sscg_ctl;
	u32 audio_pll2_mnit_ctl;
	u32 video_pll1_gnrl_ctl;
	u32 video_pll1_fdiv_ctl0;
	u32 video_pll1_fdiv_ctl1;
	u32 video_pll1_sscg_ctl;
	u32 video_pll1_mnit_ctl;
	u32 reserved[5];
	u32 dram_pll_gnrl_ctl;
	u32 dram_pll_fdiv_ctl0;
	u32 dram_pll_fdiv_ctl1;
	u32 dram_pll_sscg_ctl;
	u32 dram_pll_mnit_ctl;
	u32 gpu_pll_gnrl_ctl;
	u32 gpu_pll_div_ctl;
	u32 gpu_pll_locked_ctl1;
	u32 gpu_pll_mnit_ctl;
	u32 vpu_pll_gnrl_ctl;
	u32 vpu_pll_div_ctl;
	u32 vpu_pll_locked_ctl1;
	u32 vpu_pll_mnit_ctl;
	u32 arm_pll_gnrl_ctl;
	u32 arm_pll_div_ctl;
	u32 arm_pll_locked_ctl1;
	u32 arm_pll_mnit_ctl;
	u32 sys_pll1_gnrl_ctl;
	u32 sys_pll1_div_ctl;
	u32 sys_pll1_locked_ctl1;
	u32 reserved2[24];
	u32 sys_pll1_mnit_ctl;
	u32 sys_pll2_gnrl_ctl;
	u32 sys_pll2_div_ctl;
	u32 sys_pll2_locked_ctl1;
	u32 sys_pll2_mnit_ctl;
	u32 sys_pll3_gnrl_ctl;
	u32 sys_pll3_div_ctl;
	u32 sys_pll3_locked_ctl1;
	u32 sys_pll3_mnit_ctl;
	u32 anamix_misc_ctl;
	u32 anamix_clk_mnit_ctl;
	u32 reserved3[437];
	u32 digprog;
};
#endif

/* System Reset Controller (SRC) */
struct src {
	u32 scr;
	u32 a53rcr;
	u32 a53rcr1;
	u32 m4rcr;
	u32 reserved1[4];
	u32 usbophy1_rcr;
	u32 usbophy2_rcr;
	u32 mipiphy_rcr;
	u32 pciephy_rcr;
	u32 hdmi_rcr;
	u32 disp_rcr;
	u32 reserved2[2];
	u32 gpu_rcr;
	u32 vpu_rcr;
	u32 pcie2_rcr;
	u32 mipiphy1_rcr;
	u32 mipiphy2_rcr;
	u32 reserved3;
	u32 sbmr1;
	u32 srsr;
	u32 reserved4[2];
	u32 sisr;
	u32 simr;
	u32 sbmr2;
	u32 gpr1;
	u32 gpr2;
	u32 gpr3;
	u32 gpr4;
	u32 gpr5;
	u32 gpr6;
	u32 gpr7;
	u32 gpr8;
	u32 gpr9;
	u32 gpr10;
	u32 reserved5[985];
	u32 ddr1_rcr;
	u32 ddr2_rcr;
};

#define PWMCR_PRESCALER(x)	(((x - 1) & 0xFFF) << 4)
#define PWMCR_DOZEEN		(1 << 24)
#define PWMCR_WAITEN		(1 << 23)
#define PWMCR_DBGEN		(1 << 22)
#define PWMCR_CLKSRC_IPG_HIGH	(2 << 16)
#define PWMCR_CLKSRC_IPG	(1 << 16)
#define PWMCR_EN		(1 << 0)

struct pwm_regs {
	u32	cr;
	u32	sr;
	u32	ir;
	u32	sar;
	u32	pr;
	u32	cnr;
};

#define WDOG_WDT_MASK	BIT(3)
#define WDOG_WDZST_MASK	BIT(0)
struct wdog_regs {
	u16	wcr;	/* Control */
	u16	wsr;	/* Service */
	u16	wrsr;	/* Reset Status */
	u16	wicr;	/* Interrupt Control */
	u16	wmcr;	/* Miscellaneous Control */
};

struct bootrom_sw_info {
	u8 reserved_1;
	u8 boot_dev_instance;
	u8 boot_dev_type;
	u8 reserved_2;
	u32 core_freq;
	u32 axi_freq;
	u32 ddr_freq;
	u32 tick_freq;
	u32 reserved_3[3];
};

#define ROM_SW_INFO_ADDR_B0	(IS_ENABLED(CONFIG_IMX8MQ) ? 0x00000968 :\
				 0x000009e8)
#define ROM_SW_INFO_ADDR_A0	0x000009e8

#define ROM_SW_INFO_ADDR is_soc_rev(CHIP_REV_1_0) ? \
		(struct bootrom_sw_info **)ROM_SW_INFO_ADDR_A0 : \
		(struct bootrom_sw_info **)ROM_SW_INFO_ADDR_B0

struct gpc_reg {
	u32 lpcr_bsc;
	u32 lpcr_ad;
	u32 lpcr_cpu1;
	u32 lpcr_cpu2;
	u32 lpcr_cpu3;
	u32 slpcr;
	u32 mst_cpu_mapping;
	u32 mmdc_cpu_mapping;
	u32 mlpcr;
	u32 pgc_ack_sel;
	u32 pgc_ack_sel_m4;
	u32 gpc_misc;
	u32 imr1_core0;
	u32 imr2_core0;
	u32 imr3_core0;
	u32 imr4_core0;
	u32 imr1_core1;
	u32 imr2_core1;
	u32 imr3_core1;
	u32 imr4_core1;
	u32 imr1_cpu1;
	u32 imr2_cpu1;
	u32 imr3_cpu1;
	u32 imr4_cpu1;
	u32 imr1_cpu3;
	u32 imr2_cpu3;
	u32 imr3_cpu3;
	u32 imr4_cpu3;
	u32 isr1_cpu0;
	u32 isr2_cpu0;
	u32 isr3_cpu0;
	u32 isr4_cpu0;
	u32 isr1_cpu1;
	u32 isr2_cpu1;
	u32 isr3_cpu1;
	u32 isr4_cpu1;
	u32 isr1_cpu2;
	u32 isr2_cpu2;
	u32 isr3_cpu2;
	u32 isr4_cpu2;
	u32 isr1_cpu3;
	u32 isr2_cpu3;
	u32 isr3_cpu3;
	u32 isr4_cpu3;
	u32 slt0_cfg;
	u32 slt1_cfg;
	u32 slt2_cfg;
	u32 slt3_cfg;
	u32 slt4_cfg;
	u32 slt5_cfg;
	u32 slt6_cfg;
	u32 slt7_cfg;
	u32 slt8_cfg;
	u32 slt9_cfg;
	u32 slt10_cfg;
	u32 slt11_cfg;
	u32 slt12_cfg;
	u32 slt13_cfg;
	u32 slt14_cfg;
	u32 pgc_cpu_0_1_mapping;
	u32 cpu_pgc_up_trg;
	u32 mix_pgc_up_trg;
	u32 pu_pgc_up_trg;
	u32 cpu_pgc_dn_trg;
	u32 mix_pgc_dn_trg;
	u32 pu_pgc_dn_trg;
	u32 lpcr_bsc2;
	u32 pgc_cpu_2_3_mapping;
	u32 lps_cpu0;
	u32 lps_cpu1;
	u32 lps_cpu2;
	u32 lps_cpu3;
	u32 gpc_gpr;
	u32 gtor;
	u32 debug_addr1;
	u32 debug_addr2;
	u32 cpu_pgc_up_status1;
	u32 mix_pgc_up_status0;
	u32 mix_pgc_up_status1;
	u32 mix_pgc_up_status2;
	u32 m4_mix_pgc_up_status0;
	u32 m4_mix_pgc_up_status1;
	u32 m4_mix_pgc_up_status2;
	u32 pu_pgc_up_status0;
	u32 pu_pgc_up_status1;
	u32 pu_pgc_up_status2;
	u32 m4_pu_pgc_up_status0;
	u32 m4_pu_pgc_up_status1;
	u32 m4_pu_pgc_up_status2;
	u32 a53_lp_io_0;
	u32 a53_lp_io_1;
	u32 a53_lp_io_2;
	u32 cpu_pgc_dn_status1;
	u32 mix_pgc_dn_status0;
	u32 mix_pgc_dn_status1;
	u32 mix_pgc_dn_status2;
	u32 m4_mix_pgc_dn_status0;
	u32 m4_mix_pgc_dn_status1;
	u32 m4_mix_pgc_dn_status2;
	u32 pu_pgc_dn_status0;
	u32 pu_pgc_dn_status1;
	u32 pu_pgc_dn_status2;
	u32 m4_pu_pgc_dn_status0;
	u32 m4_pu_pgc_dn_status1;
	u32 m4_pu_pgc_dn_status2;
	u32 res[3];
	u32 mix_pdn_flg;
	u32 pu_pdn_flg;
	u32 m4_mix_pdn_flg;
	u32 m4_pu_pdn_flg;
	u32 imr1_core2;
	u32 imr2_core2;
	u32 imr3_core2;
	u32 imr4_core2;
	u32 imr1_core3;
	u32 imr2_core3;
	u32 imr3_core3;
	u32 imr4_core3;
	u32 pgc_ack_sel_pu;
	u32 pgc_ack_sel_m4_pu;
	u32 slt15_cfg;
	u32 slt16_cfg;
	u32 slt17_cfg;
	u32 slt18_cfg;
	u32 slt19_cfg;
	u32 gpc_pu_pwrhsk;
	u32 slt0_cfg_pu;
	u32 slt1_cfg_pu;
	u32 slt2_cfg_pu;
	u32 slt3_cfg_pu;
	u32 slt4_cfg_pu;
	u32 slt5_cfg_pu;
	u32 slt6_cfg_pu;
	u32 slt7_cfg_pu;
	u32 slt8_cfg_pu;
	u32 slt9_cfg_pu;
	u32 slt10_cfg_pu;
	u32 slt11_cfg_pu;
	u32 slt12_cfg_pu;
	u32 slt13_cfg_pu;
	u32 slt14_cfg_pu;
	u32 slt15_cfg_pu;
	u32 slt16_cfg_pu;
	u32 slt17_cfg_pu;
	u32 slt18_cfg_pu;
	u32 slt19_cfg_pu;
};

struct pgc_reg {
	u32 pgcr;
	u32 pgpupscr;
	u32 pgpdnscr;
	u32 pgsr;
	u32 pgauxsw;
	u32 pgdr;
};
#endif
#endif
