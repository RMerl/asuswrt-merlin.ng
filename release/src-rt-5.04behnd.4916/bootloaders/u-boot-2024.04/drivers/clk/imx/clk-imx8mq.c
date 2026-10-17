// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2019 NXP
 * Copyright 2022 Purism
 * Peng Fan <peng.fan@nxp.com>
 */

#include <common.h>
#include <clk.h>
#include <clk-uclass.h>
#include <dm.h>
#include <log.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <dt-bindings/clock/imx8mq-clock.h>

#include "clk.h"

static const char *const pll_ref_sels[] = { "clock-osc-25m", "clock-osc-27m", "clock-phy-27m", "dummy", };
static const char *const arm_pll_bypass_sels[] = {"arm_pll", "arm_pll_ref_sel", };
static const char *const gpu_pll_bypass_sels[] = {"gpu_pll", "gpu_pll_ref_sel", };
static const char *const vpu_pll_bypass_sels[] = {"vpu_pll", "vpu_pll_ref_sel", };
static const char *const audio_pll1_bypass_sels[] = {"audio_pll1", "audio_pll1_ref_sel", };
static const char *const audio_pll2_bypass_sels[] = {"audio_pll2", "audio_pll2_ref_sel", };
static const char *const video_pll1_bypass_sels[] = {"video_pll1", "video_pll1_ref_sel", };

static const char *const imx8mq_a53_core_sels[] = {"arm_a53_div", "arm_pll_out", };
static const char *const imx8mq_a53_sels[] = {"clock-osc-25m", "arm_pll_out", "sys_pll2_500m",
					      "sys_pll2_1000m", "sys_pll1_800m", "sys_pll1_400m",
					      "audio_pll1_out", "sys_pll3_out", };

static const char *const imx8mq_ahb_sels[] = {"clock-osc-25m", "sys_pll1_133m", "sys_pll1_800m",
					      "sys_pll1_400m", "sys_pll2_125m", "sys_pll3_out",
					      "audio_pll1_out", "video_pll1_out", };

static const char *const imx8mq_dram_alt_sels[] = {"osc_25m", "sys_pll1_800m", "sys_pll1_100m",
						   "sys_pll2_500m", "sys_pll2_250m",
						   "sys_pll1_400m", "audio_pll1_out", "sys_pll1_266m", }  ;

static const char * const imx8mq_dram_apb_sels[] = {"osc_25m", "sys_pll2_200m", "sys_pll1_40m",
						    "sys_pll1_160m", "sys_pll1_800m", "sys_pll3_out",
						    "sys_pll2_250m", "audio_pll2_out", };

static const char *const imx8mq_enet_axi_sels[] = {"clock-osc-25m", "sys_pll1_266m", "sys_pll1_800m",
						   "sys_pll2_250m", "sys_pll2_200m", "audio_pll1_out",
						   "video_pll1_out", "sys_pll3_out", };

static const char *const imx8mq_enet_ref_sels[] = {"clock-osc-25m", "sys_pll2_125m", "sys_pll2_50m",
						   "sys_pll2_100m", "sys_pll1_160m", "audio_pll1_out",
						   "video_pll1_out", "clk_ext4", };

static const char *const imx8mq_enet_timer_sels[] = {"clock-osc-25m", "sys_pll2_100m", "audio_pll1_out",
						     "clk_ext1", "clk_ext2", "clk_ext3", "clk_ext4",
						     "video_pll1_out", };

static const char *const imx8mq_enet_phy_sels[] = {"clock-osc-25m", "sys_pll2_50m", "sys_pll2_125m",
						   "sys_pll2_200m", "sys_pll2_500m", "video_pll1_out",
						   "audio_pll2_out", };

static const char *const imx8mq_nand_usdhc_sels[] = {"clock-osc-25m", "sys_pll1_266m", "sys_pll1_800m",
						     "sys_pll2_200m", "sys_pll1_133m", "sys_pll3_out",
						     "sys_pll2_250m", "audio_pll1_out", };

static const char *const imx8mq_usb_bus_sels[] = {"clock-osc-25m", "sys_pll2_500m", "sys_pll1_800m",
						  "sys_pll2_100m", "sys_pll2_200m", "clk_ext2",
						  "clk_ext4", "audio_pll2_out", };

static const char *const imx8mq_usdhc1_sels[] = {"clock-osc-25m", "sys_pll1_400m", "sys_pll1_800m",
						 "sys_pll2_500m", "sys_pll3_out", "sys_pll1_266m",
						 "audio_pll2_out", "sys_pll1_100m", };

static const char *const imx8mq_usdhc2_sels[] = {"clock-osc-25m", "sys_pll1_400m", "sys_pll1_800m",
						 "sys_pll2_500m", "sys_pll3_out", "sys_pll1_266m",
						 "audio_pll2_out", "sys_pll1_100m", };

static const char *const imx8mq_i2c1_sels[] = {"clock-osc-25m", "sys_pll1_160m", "sys_pll2_50m",
					       "sys_pll3_out", "audio_pll1_out", "video_pll1_out",
					       "audio_pll2_out", "sys_pll1_133m", };

static const char *const imx8mq_i2c2_sels[] = {"clock-osc-25m", "sys_pll1_160m", "sys_pll2_50m",
					       "sys_pll3_out", "audio_pll1_out", "video_pll1_out",
					       "audio_pll2_out", "sys_pll1_133m", };

static const char *const imx8mq_i2c3_sels[] = {"clock-osc-25m", "sys_pll1_160m", "sys_pll2_50m",
					       "sys_pll3_out", "audio_pll1_out", "video_pll1_out",
					       "audio_pll2_out", "sys_pll1_133m", };

static const char *const imx8mq_i2c4_sels[] = {"clock-osc-25m", "sys_pll1_160m", "sys_pll2_50m",
					       "sys_pll3_out", "audio_pll1_out", "video_pll1_out",
					       "audio_pll2_out", "sys_pll1_133m", };

static const char *const imx8mq_uart1_sels[] = {"clock-osc-25m", "sys_pll1_80m", "sys_pll2_200m",
						"sys_pll2_100m", "sys_pll3_out", "clk_ext2",
						"clk_ext4", "audio_pll2_out", };

static const char *const imx8mq_uart2_sels[] = {"clock-osc-25m", "sys_pll1_80m", "sys_pll2_200m",
						"sys_pll2_100m", "sys_pll3_out", "clk_ext2",
						"clk_ext3", "audio_pll2_out", };

static const char *const imx8mq_uart3_sels[] = {"clock-osc-25m", "sys_pll1_80m", "sys_pll2_200m",
						"sys_pll2_100m", "sys_pll3_out", "clk_ext2",
						"clk_ext4", "audio_pll2_out", };

static const char *const imx8mq_uart4_sels[] = {"clock-osc-25m", "sys_pll1_80m", "sys_pll2_200m",
						"sys_pll2_100m", "sys_pll3_out", "clk_ext2",
						"clk_ext3", "audio_pll2_out", };

static const char *const imx8mq_wdog_sels[] = {"clock-osc-25m", "sys_pll1_133m", "sys_pll1_160m",
					       "vpu_pll_out", "sys_pll2_125m", "sys_pll3_out",
					       "sys_pll1_80m", "sys_pll2_166m", };

static const char *const imx8mq_qspi_sels[] = {"clock-osc-25m", "sys_pll1_400m", "sys_pll2_333m",
					       "sys_pll2_500m", "audio_pll2_out", "sys_pll1_266m",
					       "sys_pll3_out", "sys_pll1_100m", };

static const char *const imx8mq_usb_core_sels[] = {"clock-osc-25m", "sys_pll1_100m", "sys_pll1_40m",
						   "sys_pll2_100m", "sys_pll2_200m", "clk_ext2",
						   "clk_ext3", "audio_pll2_out", };

static const char *const imx8mq_usb_phy_sels[] = {"clock-osc-25m", "sys_pll1_100m", "sys_pll1_40m",
						  "sys_pll2_100m", "sys_pll2_200m", "clk_ext2",
						  "clk_ext3", "audio_pll2_out", };

static const char *const imx8mq_ecspi1_sels[] = {"clock-osc-25m", "sys_pll2_200m", "sys_pll1_40m",
						 "sys_pll1_160m", "sys_pll1_800m", "sys_pll3_out",
						 "sys_pll2_250m", "audio_pll2_out", };

static const char *const imx8mq_ecspi2_sels[] = {"clock-osc-25m", "sys_pll2_200m", "sys_pll1_40m",
						 "sys_pll1_160m", "sys_pll1_800m", "sys_pll3_out",
						 "sys_pll2_250m", "audio_pll2_out", };

static const char *const imx8mq_ecspi3_sels[] = {"clock-osc-25m", "sys_pll2_200m", "sys_pll1_40m",
						 "sys_pll1_160m", "sys_pll1_800m", "sys_pll3_out",
						 "sys_pll2_250m", "audio_pll2_out", };

static const char *const imx8mq_dram_core_sels[] = {"dram_pll_out", "dram_alt_root", };

static const char *const pllout_monitor_sels[] = {"clock-osc-25m", "clock-osc-27m", "clock-phy-27m",
						  "dummy", "clock-ckil", "audio_pll1_out_monitor",
						  "audio_pll2_out_monitor", "gpu_pll_out_monitor",
						  "vpu_pll_out_monitor", "video_pll1_out_monitor",
						  "arm_pll_out_monitor", "sys_pll1_out_monitor",
						  "sys_pll2_out_monitor", "sys_pll3_out_monitor",
						  "video_pll2_out_monitor", "dram_pll_out_monitor", };

static int imx8mq_clk_probe(struct udevice *dev)
{
	void __iomem *base;

	base = (void *)ANATOP_BASE_ADDR;

	clk_dm(IMX8MQ_CLK_32K, clk_register_fixed_rate(NULL, "ckil", 32768));
	clk_dm(IMX8MQ_CLK_27M, clk_register_fixed_rate(NULL, "clock-osc-27m", 27000000));

	clk_dm(IMX8MQ_DRAM_PLL1_REF_SEL,
	       imx_clk_mux("dram_pll_ref_sel", base + 0x60, 0, 2,
			   pll_ref_sels, ARRAY_SIZE(pll_ref_sels)));
	clk_dm(IMX8MQ_ARM_PLL_REF_SEL,
	       imx_clk_mux("arm_pll_ref_sel", base + 0x28, 0, 2,
			   pll_ref_sels, ARRAY_SIZE(pll_ref_sels)));
	clk_dm(IMX8MQ_GPU_PLL_REF_SEL,
	       imx_clk_mux("gpu_pll_ref_sel", base + 0x18, 0, 2,
			   pll_ref_sels, ARRAY_SIZE(pll_ref_sels)));
	clk_dm(IMX8MQ_VPU_PLL_REF_SEL,
	       imx_clk_mux("vpu_pll_ref_sel", base + 0x20, 0, 2,
			   pll_ref_sels, ARRAY_SIZE(pll_ref_sels)));
	clk_dm(IMX8MQ_SYS3_PLL1_REF_SEL,
	       imx_clk_mux("sys3_pll_ref_sel", base + 0x48, 0, 2,
			   pll_ref_sels, ARRAY_SIZE(pll_ref_sels)));
	clk_dm(IMX8MQ_AUDIO_PLL1_REF_SEL,
	       imx_clk_mux("audio_pll1_ref_sel", base + 0x0, 0, 2,
			   pll_ref_sels, ARRAY_SIZE(pll_ref_sels)));
	clk_dm(IMX8MQ_AUDIO_PLL2_REF_SEL,
	       imx_clk_mux("audio_pll2_ref_sel", base + 0x8, 0, 2,
			   pll_ref_sels, ARRAY_SIZE(pll_ref_sels)));
	clk_dm(IMX8MQ_VIDEO_PLL1_REF_SEL,
	       imx_clk_mux("video_pll1_ref_sel", base + 0x10, 0, 2,
			   pll_ref_sels, ARRAY_SIZE(pll_ref_sels)));
	clk_dm(IMX8MQ_VIDEO2_PLL1_REF_SEL,
	       imx_clk_mux("video_pll2_ref_sel", base + 0x54, 0, 2,
			   pll_ref_sels, ARRAY_SIZE(pll_ref_sels)));

	clk_dm(IMX8MQ_ARM_PLL,
	       imx_clk_pll14xx("arm_pll", "arm_pll_ref_sel",
			       base + 0x28, &imx_1416x_pll));
	clk_dm(IMX8MQ_GPU_PLL,
	       imx_clk_pll14xx("gpu_pll", "gpu_pll_ref_sel",
			       base + 0x18, &imx_1416x_pll));
	clk_dm(IMX8MQ_VPU_PLL,
	       imx_clk_pll14xx("vpu_pll", "vpu_pll_ref_sel",
			       base + 0x20, &imx_1416x_pll));

	clk_dm(IMX8MQ_SYS1_PLL1,
	       clk_register_fixed_rate(NULL, "sys1_pll", 800000000));
	clk_dm(IMX8MQ_SYS2_PLL1,
	       clk_register_fixed_rate(NULL, "sys2_pll", 1000000000));
	clk_dm(IMX8MQ_SYS2_PLL1,
	       clk_register_fixed_rate(NULL, "sys3_pll", 1000000000));
	clk_dm(IMX8MQ_AUDIO_PLL1,
	       imx_clk_pll14xx("audio_pll1", "audio_pll1_ref_sel",
			       base + 0x0, &imx_1443x_pll));
	clk_dm(IMX8MQ_AUDIO_PLL2,
	       imx_clk_pll14xx("audio_pll2", "audio_pll2_ref_sel",
			       base + 0x8, &imx_1443x_pll));
	clk_dm(IMX8MQ_VIDEO_PLL1,
	       imx_clk_pll14xx("video_pll1", "video_pll1_ref_sel",
			       base + 0x10, &imx_1443x_pll));

	/* PLL bypass out */
	clk_dm(IMX8MQ_ARM_PLL_BYPASS,
	       imx_clk_mux_flags("arm_pll_bypass", base + 0x28, 4, 1,
				 arm_pll_bypass_sels,
				 ARRAY_SIZE(arm_pll_bypass_sels),
				 CLK_SET_RATE_PARENT));
	clk_dm(IMX8MQ_GPU_PLL_BYPASS,
	       imx_clk_mux_flags("gpu_pll_bypass", base + 0x18, 4, 1,
				 gpu_pll_bypass_sels,
				 ARRAY_SIZE(gpu_pll_bypass_sels),
				 CLK_SET_RATE_PARENT));
	clk_dm(IMX8MQ_VPU_PLL_BYPASS,
	       imx_clk_mux_flags("vpu_pll_bypass", base + 0x20, 4, 1,
				 vpu_pll_bypass_sels,
				 ARRAY_SIZE(vpu_pll_bypass_sels),
				 CLK_SET_RATE_PARENT));
	clk_dm(IMX8MQ_AUDIO_PLL1_BYPASS,
	       imx_clk_mux_flags("audio_pll1_bypass", base + 0x0, 4, 1,
				 audio_pll1_bypass_sels,
				 ARRAY_SIZE(audio_pll1_bypass_sels),
				 CLK_SET_RATE_PARENT));
	clk_dm(IMX8MQ_AUDIO_PLL2_BYPASS,
	       imx_clk_mux_flags("audio_pll2_bypass", base + 0x8, 4, 1,
				 audio_pll2_bypass_sels,
				 ARRAY_SIZE(audio_pll2_bypass_sels),
				 CLK_SET_RATE_PARENT));
	clk_dm(IMX8MQ_VIDEO_PLL1_BYPASS,
	       imx_clk_mux_flags("video_pll1_bypass", base + 0x10, 4, 1,
				 video_pll1_bypass_sels,
				 ARRAY_SIZE(video_pll1_bypass_sels),
				 CLK_SET_RATE_PARENT));

	/* PLL out gate */
	clk_dm(IMX8MQ_DRAM_PLL_OUT,
	       imx_clk_gate("dram_pll_out", "dram_pll_ref_sel",
			    base + 0x60, 13));
	clk_dm(IMX8MQ_ARM_PLL_OUT,
	       imx_clk_gate("arm_pll_out", "arm_pll_bypass",
			    base + 0x28, 11));
	clk_dm(IMX8MQ_GPU_PLL_OUT,
	       imx_clk_gate("gpu_pll_out", "gpu_pll_bypass",
			    base + 0x18, 11));
	clk_dm(IMX8MQ_VPU_PLL_OUT,
	       imx_clk_gate("vpu_pll_out", "vpu_pll_bypass",
			    base + 0x20, 11));
	clk_dm(IMX8MQ_AUDIO_PLL1_OUT,
	       imx_clk_gate("audio_pll1_out", "audio_pll1_bypass",
			    base + 0x0, 11));
	clk_dm(IMX8MQ_AUDIO_PLL2_OUT,
	       imx_clk_gate("audio_pll2_out", "audio_pll2_bypass",
			    base + 0x8, 11));
	clk_dm(IMX8MQ_VIDEO_PLL1_OUT,
	       imx_clk_gate("video_pll1_out", "video_pll1_bypass",
			    base + 0x10, 11));

	clk_dm(IMX8MQ_SYS1_PLL_OUT,
	       imx_clk_gate("sys_pll1_out", "sys1_pll",
			    base + 0x30, 11));
	clk_dm(IMX8MQ_SYS2_PLL_OUT,
	       imx_clk_gate("sys_pll2_out", "sys2_pll",
			    base + 0x3c, 11));
	clk_dm(IMX8MQ_SYS3_PLL_OUT,
	       imx_clk_gate("sys_pll3_out", "sys3_pll",
			    base + 0x48, 11));
	clk_dm(IMX8MQ_VIDEO2_PLL_OUT,
	       imx_clk_gate("video_pll2_out", "video_pll2_ref_sel",
			    base + 0x54, 11));

	/* SYS PLL fixed output */
	clk_dm(IMX8MQ_SYS1_PLL_40M,
	       imx_clk_fixed_factor("sys_pll1_40m", "sys_pll1_out", 1, 20));
	clk_dm(IMX8MQ_SYS1_PLL_80M,
	       imx_clk_fixed_factor("sys_pll1_80m", "sys_pll1_out", 1, 10));
	clk_dm(IMX8MQ_SYS1_PLL_100M,
	       imx_clk_fixed_factor("sys_pll1_100m", "sys_pll1_out", 1, 8));
	clk_dm(IMX8MQ_SYS1_PLL_133M,
	       imx_clk_fixed_factor("sys_pll1_133m", "sys_pll1_out", 1, 6));
	clk_dm(IMX8MQ_SYS1_PLL_160M,
	       imx_clk_fixed_factor("sys_pll1_160m", "sys_pll1_out", 1, 5));
	clk_dm(IMX8MQ_SYS1_PLL_200M,
	       imx_clk_fixed_factor("sys_pll1_200m", "sys_pll1_out", 1, 4));
	clk_dm(IMX8MQ_SYS1_PLL_266M,
	       imx_clk_fixed_factor("sys_pll1_266m", "sys_pll1_out", 1, 3));
	clk_dm(IMX8MQ_SYS1_PLL_400M,
	       imx_clk_fixed_factor("sys_pll1_400m", "sys_pll1_out", 1, 2));
	clk_dm(IMX8MQ_SYS1_PLL_800M,
	       imx_clk_fixed_factor("sys_pll1_800m", "sys_pll1_out", 1, 1));

	clk_dm(IMX8MQ_SYS2_PLL_50M,
	       imx_clk_fixed_factor("sys_pll2_50m", "sys_pll2_out", 1, 20));
	clk_dm(IMX8MQ_SYS2_PLL_100M,
	       imx_clk_fixed_factor("sys_pll2_100m", "sys_pll2_out", 1, 10));
	clk_dm(IMX8MQ_SYS2_PLL_125M,
	       imx_clk_fixed_factor("sys_pll2_125m", "sys_pll2_out", 1, 8));
	clk_dm(IMX8MQ_SYS2_PLL_166M,
	       imx_clk_fixed_factor("sys_pll2_166m", "sys_pll2_out", 1, 6));
	clk_dm(IMX8MQ_SYS2_PLL_200M,
	       imx_clk_fixed_factor("sys_pll2_200m", "sys_pll2_out", 1, 5));
	clk_dm(IMX8MQ_SYS2_PLL_250M,
	       imx_clk_fixed_factor("sys_pll2_250m", "sys_pll2_out", 1, 4));
	clk_dm(IMX8MQ_SYS2_PLL_333M,
	       imx_clk_fixed_factor("sys_pll2_333m", "sys_pll2_out", 1, 3));
	clk_dm(IMX8MQ_SYS2_PLL_500M,
	       imx_clk_fixed_factor("sys_pll2_500m", "sys_pll2_out", 1, 2));
	clk_dm(IMX8MQ_SYS2_PLL_1000M,
	       imx_clk_fixed_factor("sys_pll2_1000m", "sys_pll2_out", 1, 1));

	clk_dm(IMX8MQ_CLK_MON_AUDIO_PLL1_DIV,
	       imx_clk_divider("audio_pll1_out_monitor", "audio_pll1_bypass", base + 0x78, 0, 3));
	clk_dm(IMX8MQ_CLK_MON_AUDIO_PLL2_DIV,
	       imx_clk_divider("audio_pll2_out_monitor", "audio_pll2_bypass", base + 0x78, 4, 3));
	clk_dm(IMX8MQ_CLK_MON_VIDEO_PLL1_DIV,
	       imx_clk_divider("video_pll1_out_monitor", "video_pll1_bypass", base + 0x78, 8, 3));
	clk_dm(IMX8MQ_CLK_MON_GPU_PLL_DIV,
	       imx_clk_divider("gpu_pll_out_monitor", "gpu_pll_bypass", base + 0x78, 12, 3));
	clk_dm(IMX8MQ_CLK_MON_VPU_PLL_DIV,
	       imx_clk_divider("vpu_pll_out_monitor", "vpu_pll_bypass", base + 0x78, 16, 3));
	clk_dm(IMX8MQ_CLK_MON_ARM_PLL_DIV,
	       imx_clk_divider("arm_pll_out_monitor", "arm_pll_bypass", base + 0x78, 20, 3));
	clk_dm(IMX8MQ_CLK_MON_SYS_PLL1_DIV,
	       imx_clk_divider("sys_pll1_out_monitor", "sys_pll1_out", base + 0x7c, 0, 3));
	clk_dm(IMX8MQ_CLK_MON_SYS_PLL2_DIV,
	       imx_clk_divider("sys_pll2_out_monitor", "sys_pll2_out", base + 0x7c, 4, 3));
	clk_dm(IMX8MQ_CLK_MON_SYS_PLL3_DIV,
	       imx_clk_divider("sys_pll3_out_monitor", "sys_pll3_out", base + 0x7c, 8, 3));
	clk_dm(IMX8MQ_CLK_MON_DRAM_PLL_DIV,
	       imx_clk_divider("dram_pll_out_monitor", "dram_pll_out", base + 0x7c, 12, 3));
	clk_dm(IMX8MQ_CLK_MON_VIDEO_PLL2_DIV,
	       imx_clk_divider("video_pll2_out_monitor", "video_pll2_out", base + 0x7c, 16, 3));
	clk_dm(IMX8MQ_CLK_MON_SEL,
	       imx_clk_mux_flags("pllout_monitor_sel", base + 0x74, 0, 4,
				 pllout_monitor_sels,
				 ARRAY_SIZE(pllout_monitor_sels),
				 CLK_SET_RATE_PARENT));
	clk_dm(IMX8MQ_CLK_MON_CLK2_OUT,
	       imx_clk_gate4("pllout_monitor_clk2", "pllout_monitor_sel", base + 0x74, 4));

	base = dev_read_addr_ptr(dev);
	if (!base) {
		printf("%s : base failed\n", __func__);
		return -EINVAL;
	}

	clk_dm(IMX8MQ_CLK_A53_SRC,
	       imx_clk_mux2("arm_a53_src", base + 0x8000, 24, 3,
			    imx8mq_a53_sels, ARRAY_SIZE(imx8mq_a53_sels)));
	clk_dm(IMX8MQ_CLK_A53_CG,
	       imx_clk_gate3("arm_a53_cg", "arm_a53_src", base + 0x8000, 28));
	clk_dm(IMX8MQ_CLK_A53_DIV,
	       imx_clk_divider2("arm_a53_div", "arm_a53_cg",
				base + 0x8000, 0, 3));
	clk_dm(IMX8MQ_CLK_A53_CORE,
	       imx_clk_mux2("arm_a53_src", base + 0x9880, 24, 1,
			    imx8mq_a53_core_sels, ARRAY_SIZE(imx8mq_a53_core_sels)));

	clk_dm(IMX8MQ_CLK_AHB,
	       imx8m_clk_composite_critical("ahb", imx8mq_ahb_sels,
					    base + 0x9000));
	clk_dm(IMX8MQ_CLK_IPG_ROOT,
	       imx_clk_divider2("ipg_root", "ahb", base + 0x9080, 0, 1));

	clk_dm(IMX8MQ_CLK_ENET_AXI,
	       imx8m_clk_composite("enet_axi", imx8mq_enet_axi_sels,
				   base + 0x8880));
	clk_dm(IMX8MQ_CLK_NAND_USDHC_BUS,
	       imx8m_clk_composite_critical("nand_usdhc_bus",
					    imx8mq_nand_usdhc_sels,
					    base + 0x8900));
	clk_dm(IMX8MQ_CLK_USB_BUS,
	       imx8m_clk_composite("usb_bus", imx8mq_usb_bus_sels, base + 0x8b80));

	/* DRAM */
	clk_dm(IMX8MQ_CLK_DRAM_CORE,
	       imx_clk_mux2("dram_core_clk", base + 0x9800, 24, 1,
			    imx8mq_dram_core_sels, ARRAY_SIZE(imx8mq_dram_core_sels)));
	clk_dm(IMX8MQ_CLK_DRAM_ALT,
	       imx8m_clk_composite("dram_alt", imx8mq_dram_alt_sels, base + 0xa000));
	clk_dm(IMX8MQ_CLK_DRAM_APB,
	       imx8m_clk_composite_critical("dram_apb", imx8mq_dram_apb_sels, base + 0xa080));

	/* IP */
	clk_dm(IMX8MQ_CLK_USDHC1,
	       imx8m_clk_composite("usdhc1", imx8mq_usdhc1_sels,
				   base + 0xac00));
	clk_dm(IMX8MQ_CLK_USDHC2,
	       imx8m_clk_composite("usdhc2", imx8mq_usdhc2_sels,
				   base + 0xac80));
	clk_dm(IMX8MQ_CLK_I2C1,
	       imx8m_clk_composite("i2c1", imx8mq_i2c1_sels, base + 0xad00));
	clk_dm(IMX8MQ_CLK_I2C2,
	       imx8m_clk_composite("i2c2", imx8mq_i2c2_sels, base + 0xad80));
	clk_dm(IMX8MQ_CLK_I2C3,
	       imx8m_clk_composite("i2c3", imx8mq_i2c3_sels, base + 0xae00));
	clk_dm(IMX8MQ_CLK_I2C4,
	       imx8m_clk_composite("i2c4", imx8mq_i2c4_sels, base + 0xae80));
	clk_dm(IMX8MQ_CLK_WDOG,
	       imx8m_clk_composite("wdog", imx8mq_wdog_sels, base + 0xb900));
	clk_dm(IMX8MQ_CLK_UART1,
	       imx8m_clk_composite("uart1", imx8mq_uart1_sels, base + 0xaf00));
	clk_dm(IMX8MQ_CLK_UART2,
	       imx8m_clk_composite("uart2", imx8mq_uart2_sels, base + 0xaf80));
	clk_dm(IMX8MQ_CLK_UART3,
	       imx8m_clk_composite("uart3", imx8mq_uart3_sels, base + 0xb000));
	clk_dm(IMX8MQ_CLK_UART4,
	       imx8m_clk_composite("uart4", imx8mq_uart4_sels, base + 0xb080));
	clk_dm(IMX8MQ_CLK_QSPI,
	       imx8m_clk_composite("qspi", imx8mq_qspi_sels, base + 0xab80));
	clk_dm(IMX8MQ_CLK_USB_CORE_REF,
	       imx8m_clk_composite("usb_core_ref", imx8mq_usb_core_sels, base + 0xb100));
	clk_dm(IMX8MQ_CLK_USB_PHY_REF,
	       imx8m_clk_composite("usb_phy_ref", imx8mq_usb_phy_sels, base + 0xb180));
	clk_dm(IMX8MQ_CLK_ECSPI1,
	       imx8m_clk_composite("ecspi1", imx8mq_ecspi1_sels, base + 0xb280));
	clk_dm(IMX8MQ_CLK_ECSPI2,
	       imx8m_clk_composite("ecspi2", imx8mq_ecspi2_sels, base + 0xb300));
	clk_dm(IMX8MQ_CLK_ECSPI3,
	       imx8m_clk_composite("ecspi3", imx8mq_ecspi3_sels, base + 0xc180));

	clk_dm(IMX8MQ_CLK_ECSPI1_ROOT,
	       imx_clk_gate4("ecspi1_root_clk", "ecspi1", base + 0x4070, 0));
	clk_dm(IMX8MQ_CLK_ECSPI2_ROOT,
	       imx_clk_gate4("ecspi2_root_clk", "ecspi2", base + 0x4080, 0));
	clk_dm(IMX8MQ_CLK_ECSPI3_ROOT,
	       imx_clk_gate4("ecspi3_root_clk", "ecspi3", base + 0x4090, 0));
	clk_dm(IMX8MQ_CLK_I2C1_ROOT,
	       imx_clk_gate4("i2c1_root_clk", "i2c1", base + 0x4170, 0));
	clk_dm(IMX8MQ_CLK_I2C2_ROOT,
	       imx_clk_gate4("i2c2_root_clk", "i2c2", base + 0x4180, 0));
	clk_dm(IMX8MQ_CLK_I2C3_ROOT,
	       imx_clk_gate4("i2c3_root_clk", "i2c3", base + 0x4190, 0));
	clk_dm(IMX8MQ_CLK_I2C4_ROOT,
	       imx_clk_gate4("i2c4_root_clk", "i2c4", base + 0x41a0, 0));
	clk_dm(IMX8MQ_CLK_UART1_ROOT,
	       imx_clk_gate4("uart1_root_clk", "uart1", base + 0x4490, 0));
	clk_dm(IMX8MQ_CLK_UART2_ROOT,
	       imx_clk_gate4("uart2_root_clk", "uart2", base + 0x44a0, 0));
	clk_dm(IMX8MQ_CLK_UART3_ROOT,
	       imx_clk_gate4("uart3_root_clk", "uart3", base + 0x44b0, 0));
	clk_dm(IMX8MQ_CLK_UART4_ROOT,
	       imx_clk_gate4("uart4_root_clk", "uart4", base + 0x44c0, 0));
	clk_dm(IMX8MQ_CLK_OCOTP_ROOT,
	       imx_clk_gate4("ocotp_root_clk", "ipg_root", base + 0x4220, 0));
	clk_dm(IMX8MQ_CLK_USDHC1_ROOT,
	       imx_clk_gate4("usdhc1_root_clk", "usdhc1", base + 0x4510, 0));
	clk_dm(IMX8MQ_CLK_USDHC2_ROOT,
	       imx_clk_gate4("usdhc2_root_clk", "usdhc2", base + 0x4520, 0));
	clk_dm(IMX8MQ_CLK_WDOG1_ROOT,
	       imx_clk_gate4("wdog1_root_clk", "wdog", base + 0x4530, 0));
	clk_dm(IMX8MQ_CLK_WDOG2_ROOT,
	       imx_clk_gate4("wdog2_root_clk", "wdog", base + 0x4540, 0));
	clk_dm(IMX8MQ_CLK_WDOG3_ROOT,
	       imx_clk_gate4("wdog3_root_clk", "wdog", base + 0x4550, 0));
	clk_dm(IMX8MQ_CLK_QSPI_ROOT,
	       imx_clk_gate4("qspi_root_clk", "qspi", base + 0x42f0, 0));
	clk_dm(IMX8MQ_CLK_USB1_CTRL_ROOT,
	       imx_clk_gate4("usb1_ctrl_root_clk", "usb_bus", base + 0x44d0, 0));
	clk_dm(IMX8MQ_CLK_USB2_CTRL_ROOT,
	       imx_clk_gate4("usb2_ctrl_root_clk", "usb_bus", base + 0x44e0, 0));
	clk_dm(IMX8MQ_CLK_USB1_PHY_ROOT,
	       imx_clk_gate4("usb1_phy_root_clk", "usb_phy_ref", base + 0x44f0, 0));
	clk_dm(IMX8MQ_CLK_USB2_PHY_ROOT,
	       imx_clk_gate4("usb2_phy_root_clk", "usb_phy_ref", base + 0x4500, 0));

	clk_dm(IMX8MQ_CLK_ENET_REF,
	       imx8m_clk_composite("enet_ref", imx8mq_enet_ref_sels,
				   base + 0xa980));
	clk_dm(IMX8MQ_CLK_ENET_TIMER,
	       imx8m_clk_composite("enet_timer", imx8mq_enet_timer_sels,
				   base + 0xaa00));
	clk_dm(IMX8MQ_CLK_ENET_PHY_REF,
	       imx8m_clk_composite("enet_phy", imx8mq_enet_phy_sels,
				   base + 0xaa80));
	clk_dm(IMX8MQ_CLK_ENET1_ROOT,
	       imx_clk_gate4("enet1_root_clk", "enet_axi",
			     base + 0x40a0, 0));

	clk_dm(IMX8MQ_CLK_DRAM_ALT_ROOT,
	       imx_clk_fixed_factor("dram_alt_root", "dram_alt", 1, 4));

	return 0;
}

static const struct udevice_id imx8mq_clk_ids[] = {
	{ .compatible = "fsl,imx8mq-ccm" },
	{ },
};

U_BOOT_DRIVER(imx8mq_clk) = {
	.name = "clk_imx8mq",
	.id = UCLASS_CLK,
	.of_match = imx8mq_clk_ids,
	.ops = &ccf_clk_ops,
	.probe = imx8mq_clk_probe,
	.flags = DM_FLAG_PRE_RELOC,
};
