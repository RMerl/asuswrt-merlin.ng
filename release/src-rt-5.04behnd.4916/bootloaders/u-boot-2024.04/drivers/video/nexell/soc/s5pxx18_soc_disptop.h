/* SPDX-License-Identifier: GPL-2.0+
 *
 * Copyright (C) 2016  Nexell Co., Ltd.
 *
 * Author: junghyun, kim <jhkim@nexell.co.kr>
 */

#ifndef _S5PXX18_SOC_DISPTOP_H_
#define _S5PXX18_SOC_DISPTOP_H_

#include "s5pxx18_soc_disptype.h"

#define NUMBER_OF_DISPTOP_MODULE	1
#define PHY_BASEADDR_DISPLAYTOP_MODULE 0xC0100000
#define	PHY_BASEADDR_DISPTOP_LIST	\
		{ PHY_BASEADDR_DISPLAYTOP_MODULE }

#define HDMI_ADDR_OFFSET                                                       \
	(((PHY_BASEADDR_DISPLAYTOP_MODULE / 0x00100000) % 2) ? 0x100000        \
							     : 0x000000)
#define OTHER_ADDR_OFFSET                                                      \
	(((PHY_BASEADDR_DISPLAYTOP_MODULE / 0x00100000) % 2) ? 0x000000        \
							     : 0x100000)
#define PHY_BASEADDR_DISPLAYTOP_MODULE_OFFSET (OTHER_ADDR_OFFSET + 0x001000)
#define PHY_BASEADDR_DUALDISPLAY_MODULE                                        \
	(PHY_BASEADDR_DISPLAYTOP_MODULE + OTHER_ADDR_OFFSET + 0x002000)
#define PHY_BASEADDR_RESCONV_MODULE                                            \
	(PHY_BASEADDR_DISPLAYTOP_MODULE + OTHER_ADDR_OFFSET + 0x003000)
#define PHY_BASEADDR_LCDINTERFACE_MODULE                                       \
	(PHY_BASEADDR_DISPLAYTOP_MODULE + OTHER_ADDR_OFFSET + 0x004000)
#define PHY_BASEADDR_HDMI_MODULE (PHY_BASEADDR_DISPLAYTOP_MODULE + 0x000000)
#define PHY_BASEADDR_LVDS_MODULE                                               \
	(PHY_BASEADDR_DISPLAYTOP_MODULE + OTHER_ADDR_OFFSET + 0x00a000)

#define NUMBER_OF_DUALDISPLAY_MODULE 1
#define INTNUM_OF_DUALDISPLAY_MODULE_PRIMIRQ                                   \
	INTNUM_OF_DISPLAYTOP_MODULE_DUALDISPLAY_PRIMIRQ
#define INTNUM_OF_DUALDISPLAY_MODULE_SECONDIRQ                                 \
	INTNUM_OF_DISPLAYTOP_MODULE_DUALDISPLAY_SECONDIRQ
#define RESETINDEX_OF_DUALDISPLAY_MODULE_I_NRST                                \
	RESETINDEX_OF_DISPLAYTOP_MODULE_I_DUALDISPLAY_NRST
#define PADINDEX_OF_DUALDISPLAY_O_NCS                                          \
	PADINDEX_OF_DISPLAYTOP_O_DUAL_DISPLAY_PADPRIMVCLK
#define PADINDEX_OF_DUALDISPLAY_O_NRD                                          \
	PADINDEX_OF_DISPLAYTOP_O_DUAL_DISPLAY_PRIM_PADN_HSYNC
#define PADINDEX_OF_DUALDISPLAY_O_RS                                           \
	PADINDEX_OF_DISPLAYTOP_O_DUAL_DISPLAY_PRIM_PADN_VSYNC
#define PADINDEX_OF_DUALDISPLAY_O_NWR                                          \
	PADINDEX_OF_DISPLAYTOP_O_DUAL_DISPLAY_PRIM_PADDE
#define PADINDEX_OF_DUALDISPLAY_PADPRIMVCLK                                    \
	PADINDEX_OF_DISPLAYTOP_O_DUAL_DISPLAY_PADPRIMVCLK
#define PADINDEX_OF_DUALDISPLAY_O_PRIM_PADN_HSYNC                              \
	PADINDEX_OF_DISPLAYTOP_O_DUAL_DISPLAY_PRIM_PADN_HSYNC
#define PADINDEX_OF_DUALDISPLAY_O_PRIM_PADN_VSYNC                              \
	PADINDEX_OF_DISPLAYTOP_O_DUAL_DISPLAY_PRIM_PADN_VSYNC
#define PADINDEX_OF_DUALDISPLAY_O_PRIM_PADDE                                   \
	PADINDEX_OF_DISPLAYTOP_O_DUAL_DISPLAY_PRIM_PADDE
#define PADINDEX_OF_DUALDISPLAY_PRIM_0_                                        \
	PADINDEX_OF_DISPLAYTOP_DUAL_DISPLAY_PRIM_0_
#define PADINDEX_OF_DUALDISPLAY_PRIM_1_                                        \
	PADINDEX_OF_DISPLAYTOP_DUAL_DISPLAY_PRIM_1_
#define PADINDEX_OF_DUALDISPLAY_PRIM_2_                                        \
	PADINDEX_OF_DISPLAYTOP_DUAL_DISPLAY_PRIM_2_
#define PADINDEX_OF_DUALDISPLAY_PRIM_3_                                        \
	PADINDEX_OF_DISPLAYTOP_DUAL_DISPLAY_PRIM_3_
#define PADINDEX_OF_DUALDISPLAY_PRIM_4_                                        \
	PADINDEX_OF_DISPLAYTOP_DUAL_DISPLAY_PRIM_4_
#define PADINDEX_OF_DUALDISPLAY_PRIM_5_                                        \
	PADINDEX_OF_DISPLAYTOP_DUAL_DISPLAY_PRIM_5_
#define PADINDEX_OF_DUALDISPLAY_PRIM_6_                                        \
	PADINDEX_OF_DISPLAYTOP_DUAL_DISPLAY_PRIM_6_
#define PADINDEX_OF_DUALDISPLAY_PRIM_7_                                        \
	PADINDEX_OF_DISPLAYTOP_DUAL_DISPLAY_PRIM_7_
#define PADINDEX_OF_DUALDISPLAY_PRIM_8_                                        \
	PADINDEX_OF_DISPLAYTOP_DUAL_DISPLAY_PRIM_8_
#define PADINDEX_OF_DUALDISPLAY_PRIM_9_                                        \
	PADINDEX_OF_DISPLAYTOP_DUAL_DISPLAY_PRIM_9_
#define PADINDEX_OF_DUALDISPLAY_PRIM_10_                                       \
	PADINDEX_OF_DISPLAYTOP_DUAL_DISPLAY_PRIM_10_
#define PADINDEX_OF_DUALDISPLAY_PRIM_11_                                       \
	PADINDEX_OF_DISPLAYTOP_DUAL_DISPLAY_PRIM_11_
#define PADINDEX_OF_DUALDISPLAY_PRIM_12_                                       \
	PADINDEX_OF_DISPLAYTOP_DUAL_DISPLAY_PRIM_12_
#define PADINDEX_OF_DUALDISPLAY_PRIM_13_                                       \
	PADINDEX_OF_DISPLAYTOP_DUAL_DISPLAY_PRIM_13_
#define PADINDEX_OF_DUALDISPLAY_PRIM_14_                                       \
	PADINDEX_OF_DISPLAYTOP_DUAL_DISPLAY_PRIM_14_
#define PADINDEX_OF_DUALDISPLAY_PRIM_15_                                       \
	PADINDEX_OF_DISPLAYTOP_DUAL_DISPLAY_PRIM_15_
#define PADINDEX_OF_DUALDISPLAY_PRIM_16_                                       \
	PADINDEX_OF_DISPLAYTOP_DUAL_DISPLAY_PRIM_16_
#define PADINDEX_OF_DUALDISPLAY_PRIM_17_                                       \
	PADINDEX_OF_DISPLAYTOP_DUAL_DISPLAY_PRIM_17_
#define PADINDEX_OF_DUALDISPLAY_PRIM_18_                                       \
	PADINDEX_OF_DISPLAYTOP_DUAL_DISPLAY_PRIM_18_
#define PADINDEX_OF_DUALDISPLAY_PRIM_19_                                       \
	PADINDEX_OF_DISPLAYTOP_DUAL_DISPLAY_PRIM_19_
#define PADINDEX_OF_DUALDISPLAY_PRIM_20_                                       \
	PADINDEX_OF_DISPLAYTOP_DUAL_DISPLAY_PRIM_20_
#define PADINDEX_OF_DUALDISPLAY_PRIM_21_                                       \
	PADINDEX_OF_DISPLAYTOP_DUAL_DISPLAY_PRIM_21_
#define PADINDEX_OF_DUALDISPLAY_PRIM_22_                                       \
	PADINDEX_OF_DISPLAYTOP_DUAL_DISPLAY_PRIM_22_
#define PADINDEX_OF_DUALDISPLAY_PRIM_23_                                       \
	PADINDEX_OF_DISPLAYTOP_DUAL_DISPLAY_PRIM_23_
#define PADINDEX_OF_DUALDISPLAY_PADSECONDVCLK                                  \
	PADINDEX_OF_DISPLAYTOP_O_DUAL_DISPLAY_PADPRIMVCLK
#define PADINDEX_OF_DUALDISPLAY_O_SECOND_PADN_HSYNC                            \
	PADINDEX_OF_DISPLAYTOP_O_DUAL_DISPLAY_PRIM_PADN_HSYNC
#define PADINDEX_OF_DUALDISPLAY_O_SECOND_PADN_VSYNC                            \
	PADINDEX_OF_DISPLAYTOP_O_DUAL_DISPLAY_PRIM_PADN_VSYNC
#define PADINDEX_OF_DUALDISPLAY_O_SECOND_PADDE                                 \
	PADINDEX_OF_DISPLAYTOP_O_DUAL_DISPLAY_PRIM_PADDE
#define PADINDEX_OF_DUALDISPLAY_SECOND_0_                                      \
	PADINDEX_OF_DISPLAYTOP_DUAL_DISPLAY_PRIM_0_
#define PADINDEX_OF_DUALDISPLAY_SECOND_1_                                      \
	PADINDEX_OF_DISPLAYTOP_DUAL_DISPLAY_PRIM_1_
#define PADINDEX_OF_DUALDISPLAY_SECOND_2_                                      \
	PADINDEX_OF_DISPLAYTOP_DUAL_DISPLAY_PRIM_2_
#define PADINDEX_OF_DUALDISPLAY_SECOND_3_                                      \
	PADINDEX_OF_DISPLAYTOP_DUAL_DISPLAY_PRIM_3_
#define PADINDEX_OF_DUALDISPLAY_SECOND_4_                                      \
	PADINDEX_OF_DISPLAYTOP_DUAL_DISPLAY_PRIM_4_
#define PADINDEX_OF_DUALDISPLAY_SECOND_5_                                      \
	PADINDEX_OF_DISPLAYTOP_DUAL_DISPLAY_PRIM_5_
#define PADINDEX_OF_DUALDISPLAY_SECOND_6_                                      \
	PADINDEX_OF_DISPLAYTOP_DUAL_DISPLAY_PRIM_6_
#define PADINDEX_OF_DUALDISPLAY_SECOND_7_                                      \
	PADINDEX_OF_DISPLAYTOP_DUAL_DISPLAY_PRIM_7_
#define PADINDEX_OF_DUALDISPLAY_SECOND_8_                                      \
	PADINDEX_OF_DISPLAYTOP_DUAL_DISPLAY_PRIM_8_
#define PADINDEX_OF_DUALDISPLAY_SECOND_9_                                      \
	PADINDEX_OF_DISPLAYTOP_DUAL_DISPLAY_PRIM_9_
#define PADINDEX_OF_DUALDISPLAY_SECOND_10_                                     \
	PADINDEX_OF_DISPLAYTOP_DUAL_DISPLAY_PRIM_10_
#define PADINDEX_OF_DUALDISPLAY_SECOND_11_                                     \
	PADINDEX_OF_DISPLAYTOP_DUAL_DISPLAY_PRIM_11_
#define PADINDEX_OF_DUALDISPLAY_SECOND_12_                                     \
	PADINDEX_OF_DISPLAYTOP_DUAL_DISPLAY_PRIM_12_
#define PADINDEX_OF_DUALDISPLAY_SECOND_13_                                     \
	PADINDEX_OF_DISPLAYTOP_DUAL_DISPLAY_PRIM_13_
#define PADINDEX_OF_DUALDISPLAY_SECOND_14_                                     \
	PADINDEX_OF_DISPLAYTOP_DUAL_DISPLAY_PRIM_14_
#define PADINDEX_OF_DUALDISPLAY_SECOND_15_                                     \
	PADINDEX_OF_DISPLAYTOP_DUAL_DISPLAY_PRIM_15_
#define PADINDEX_OF_DUALDISPLAY_SECOND_16_                                     \
	PADINDEX_OF_DISPLAYTOP_DUAL_DISPLAY_PRIM_16_
#define PADINDEX_OF_DUALDISPLAY_SECOND_17_                                     \
	PADINDEX_OF_DISPLAYTOP_DUAL_DISPLAY_PRIM_17_
#define PADINDEX_OF_DUALDISPLAY_SECOND_18_                                     \
	PADINDEX_OF_DISPLAYTOP_DUAL_DISPLAY_PRIM_18_
#define PADINDEX_OF_DUALDISPLAY_SECOND_19_                                     \
	PADINDEX_OF_DISPLAYTOP_DUAL_DISPLAY_PRIM_19_
#define PADINDEX_OF_DUALDISPLAY_SECOND_20_                                     \
	PADINDEX_OF_DISPLAYTOP_DUAL_DISPLAY_PRIM_20_
#define PADINDEX_OF_DUALDISPLAY_SECOND_21_                                     \
	PADINDEX_OF_DISPLAYTOP_DUAL_DISPLAY_PRIM_21_
#define PADINDEX_OF_DUALDISPLAY_SECOND_22_                                     \
	PADINDEX_OF_DISPLAYTOP_DUAL_DISPLAY_PRIM_22_
#define PADINDEX_OF_DUALDISPLAY_SECOND_23_                                     \
	PADINDEX_OF_DISPLAYTOP_DUAL_DISPLAY_PRIM_23_

#define NUMBER_OF_RESCONV_MODULE 1
#define INTNUM_OF_RESCONV_MODULE INTNUM_OF_DISPLAYTOP_MODULE_RESCONV_IRQ
#define RESETINDEX_OF_RESCONV_MODULE_I_NRST                                    \
	RESETINDEX_OF_DISPLAYTOP_MODULE_I_RESCONV_NRST
#define RESETINDEX_OF_RESCONV_MODULE RESETINDEX_OF_RESCONV_MODULE_I_NRST
#define NUMBER_OF_LCDINTERFACE_MODULE 1
#define RESETINDEX_OF_LCDINTERFACE_MODULE_I_NRST                               \
	RESETINDEX_OF_DISPLAYTOP_MODULE_I_LCDIF_NRST
#define PADINDEX_OF_LCDINTERFACE_O_VCLK                                        \
	PADINDEX_OF_DISPLAYTOP_O_DUAL_DISPLAY_PADPRIMVCLK
#define PADINDEX_OF_LCDINTERFACE_O_NHSYNC                                      \
	PADINDEX_OF_DISPLAYTOP_O_DUAL_DISPLAY_PRIM_PADN_HSYNC
#define PADINDEX_OF_LCDINTERFACE_O_NVSYNC                                      \
	PADINDEX_OF_DISPLAYTOP_O_DUAL_DISPLAY_PRIM_PADN_VSYNC
#define PADINDEX_OF_LCDINTERFACE_O_DE                                          \
	PADINDEX_OF_DISPLAYTOP_O_DUAL_DISPLAY_PRIM_PADDE
#define PADINDEX_OF_LCDINTERFACE_RGB24_0_                                      \
	PADINDEX_OF_DISPLAYTOP_DUAL_DISPLAY_PRIM_0_
#define PADINDEX_OF_LCDINTERFACE_RGB24_1_                                      \
	PADINDEX_OF_DISPLAYTOP_DUAL_DISPLAY_PRIM_1_
#define PADINDEX_OF_LCDINTERFACE_RGB24_2_                                      \
	PADINDEX_OF_DISPLAYTOP_DUAL_DISPLAY_PRIM_2_
#define PADINDEX_OF_LCDINTERFACE_RGB24_3_                                      \
	PADINDEX_OF_DISPLAYTOP_DUAL_DISPLAY_PRIM_3_
#define PADINDEX_OF_LCDINTERFACE_RGB24_4_                                      \
	PADINDEX_OF_DISPLAYTOP_DUAL_DISPLAY_PRIM_4_
#define PADINDEX_OF_LCDINTERFACE_RGB24_5_                                      \
	PADINDEX_OF_DISPLAYTOP_DUAL_DISPLAY_PRIM_5_
#define PADINDEX_OF_LCDINTERFACE_RGB24_6_                                      \
	PADINDEX_OF_DISPLAYTOP_DUAL_DISPLAY_PRIM_6_
#define PADINDEX_OF_LCDINTERFACE_RGB24_7_                                      \
	PADINDEX_OF_DISPLAYTOP_DUAL_DISPLAY_PRIM_7_
#define PADINDEX_OF_LCDINTERFACE_RGB24_8_                                      \
	PADINDEX_OF_DISPLAYTOP_DUAL_DISPLAY_PRIM_8_
#define PADINDEX_OF_LCDINTERFACE_RGB24_9_                                      \
	PADINDEX_OF_DISPLAYTOP_DUAL_DISPLAY_PRIM_9_
#define PADINDEX_OF_LCDINTERFACE_RGB24_10_                                     \
	PADINDEX_OF_DISPLAYTOP_DUAL_DISPLAY_PRIM_10_
#define PADINDEX_OF_LCDINTERFACE_RGB24_11_                                     \
	PADINDEX_OF_DISPLAYTOP_DUAL_DISPLAY_PRIM_11_
#define PADINDEX_OF_LCDINTERFACE_RGB24_12_                                     \
	PADINDEX_OF_DISPLAYTOP_DUAL_DISPLAY_PRIM_12_
#define PADINDEX_OF_LCDINTERFACE_RGB24_13_                                     \
	PADINDEX_OF_DISPLAYTOP_DUAL_DISPLAY_PRIM_13_
#define PADINDEX_OF_LCDINTERFACE_RGB24_14_                                     \
	PADINDEX_OF_DISPLAYTOP_DUAL_DISPLAY_PRIM_14_
#define PADINDEX_OF_LCDINTERFACE_RGB24_15_                                     \
	PADINDEX_OF_DISPLAYTOP_DUAL_DISPLAY_PRIM_15_
#define PADINDEX_OF_LCDINTERFACE_RGB24_16_                                     \
	PADINDEX_OF_DISPLAYTOP_DUAL_DISPLAY_PRIM_16_
#define PADINDEX_OF_LCDINTERFACE_RGB24_17_                                     \
	PADINDEX_OF_DISPLAYTOP_DUAL_DISPLAY_PRIM_17_
#define PADINDEX_OF_LCDINTERFACE_RGB24_18_                                     \
	PADINDEX_OF_DISPLAYTOP_DUAL_DISPLAY_PRIM_18_
#define PADINDEX_OF_LCDINTERFACE_RGB24_19_                                     \
	PADINDEX_OF_DISPLAYTOP_DUAL_DISPLAY_PRIM_19_
#define PADINDEX_OF_LCDINTERFACE_RGB24_20_                                     \
	PADINDEX_OF_DISPLAYTOP_DUAL_DISPLAY_PRIM_20_
#define PADINDEX_OF_LCDINTERFACE_RGB24_21_                                     \
	PADINDEX_OF_DISPLAYTOP_DUAL_DISPLAY_PRIM_21_
#define PADINDEX_OF_LCDINTERFACE_RGB24_22_                                     \
	PADINDEX_OF_DISPLAYTOP_DUAL_DISPLAY_PRIM_22_
#define PADINDEX_OF_LCDINTERFACE_RGB24_23_                                     \
	PADINDEX_OF_DISPLAYTOP_DUAL_DISPLAY_PRIM_23_

#define NUMBER_OF_HDMI_MODULE 1
#define INTNUM_OF_HDMI_MODULE INTNUM_OF_DISPLAYTOP_MODULE_HDMI_IRQ
#define RESETINDEX_OF_HDMI_MODULE_I_NRST                                       \
	RESETINDEX_OF_DISPLAYTOP_MODULE_I_HDMI_NRST
#define RESETINDEX_OF_HDMI_MODULE_I_NRST_VIDEO                                 \
	RESETINDEX_OF_DISPLAYTOP_MODULE_I_HDMI_VIDEO_NRST
#define RESETINDEX_OF_HDMI_MODULE_I_NRST_SPDIF                                 \
	RESETINDEX_OF_DISPLAYTOP_MODULE_I_HDMI_SPDIF_NRST
#define RESETINDEX_OF_HDMI_MODULE_I_NRST_TMDS                                  \
	RESETINDEX_OF_DISPLAYTOP_MODULE_I_HDMI_TMDS_NRST
#define RESETINDEX_OF_HDMI_MODULE_I_NRST_PHY                                   \
	RESETINDEX_OF_DISPLAYTOP_MODULE_I_HDMI_PHY_NRST
#define PADINDEX_OF_HDMI_I_PHY_CLKI PADINDEX_OF_DISPLAYTOP_I_HDMI_CLKI
#define PADINDEX_OF_HDMI_O_PHY_CLKO PADINDEX_OF_DISPLAYTOP_O_HDMI_CLKO
#define PADINDEX_OF_HDMI_IO_PHY_REXT PADINDEX_OF_DISPLAYTOP_IO_HDMI_REXT
#define PADINDEX_OF_HDMI_O_PHY_TX0P PADINDEX_OF_DISPLAYTOP_O_HDMI_TX0P
#define PADINDEX_OF_HDMI_O_PHY_TX0N PADINDEX_OF_DISPLAYTOP_O_HDMI_TX0N
#define PADINDEX_OF_HDMI_O_PHY_TX1P PADINDEX_OF_DISPLAYTOP_O_HDMI_TX1P
#define PADINDEX_OF_HDMI_O_PHY_TX1N PADINDEX_OF_DISPLAYTOP_O_HDMI_TX1N
#define PADINDEX_OF_HDMI_O_PHY_TX2P PADINDEX_OF_DISPLAYTOP_O_HDMI_TX2P
#define PADINDEX_OF_HDMI_O_PHY_TX2N PADINDEX_OF_DISPLAYTOP_O_HDMI_TX2N
#define PADINDEX_OF_HDMI_O_PHY_TXCP PADINDEX_OF_DISPLAYTOP_O_HDMI_TXCP
#define PADINDEX_OF_HDMI_O_PHY_TXCN PADINDEX_OF_DISPLAYTOP_O_HDMI_TXCN
#define PADINDEX_OF_HDMI_I_HOTPLUG PADINDEX_OF_DISPLAYTOP_I_HDMI_HOTPLUG_5V
#define PADINDEX_OF_HDMI_IO_PAD_CEC PADINDEX_OF_DISPLAYTOP_IO_HDMI_CEC
#define NUMBER_OF_LVDS_MODULE 1

#define RESETINDEX_OF_LVDS_MODULE_I_RESETN                                     \
	RESETINDEX_OF_DISPLAYTOP_MODULE_I_LVDS_NRST
#define RESETINDEX_OF_LVDS_MODULE RESETINDEX_OF_LVDS_MODULE_I_RESETN

#define PADINDEX_OF_LVDS_TAP PADINDEX_OF_DISPLAYTOP_LVDS_TXP_A
#define PADINDEX_OF_LVDS_TAN PADINDEX_OF_DISPLAYTOP_LVDS_TXN_A
#define PADINDEX_OF_LVDS_TBP PADINDEX_OF_DISPLAYTOP_LVDS_TXP_B
#define PADINDEX_OF_LVDS_TBN PADINDEX_OF_DISPLAYTOP_LVDS_TXN_B
#define PADINDEX_OF_LVDS_TCP PADINDEX_OF_DISPLAYTOP_LVDS_TXP_C
#define PADINDEX_OF_LVDS_TCN PADINDEX_OF_DISPLAYTOP_LVDS_TXN_C
#define PADINDEX_OF_LVDS_TDP PADINDEX_OF_DISPLAYTOP_LVDS_TXP_D
#define PADINDEX_OF_LVDS_TDN PADINDEX_OF_DISPLAYTOP_LVDS_TXN_D
#define PADINDEX_OF_LVDS_TCLKP PADINDEX_OF_DISPLAYTOP_LVDS_TXP_CLK
#define PADINDEX_OF_LVDS_TCLKN PADINDEX_OF_DISPLAYTOP_LVDS_TXN_CLK
#define PADINDEX_OF_LVDS_ROUT PADINDEX_OF_DISPLAYTOP_LVDS_ROUT
#define PADINDEX_OF_LVDS_TEP PADINDEX_OF_DISPLAYTOP_LVDS_TXN_E
#define PADINDEX_OF_LVDS_TEN PADINDEX_OF_DISPLAYTOP_LVDS_TXN_E
#define NUMBER_OF_DISPTOP_CLKGEN_MODULE 5

enum disptop_clkgen_module_index {
	res_conv_clkgen = 0,
	lcdif_clkgen = 1,
	to_mipi_clkgen = 2,
	to_lvds_clkgen = 3,
	hdmi_clkgen = 4,
};

enum disptop_res_conv_iclk_cclk {
	res_conv_iclk = 0,
	res_conv_cclk = 1,
};

enum disptop_res_conv_oclk {
	res_conv_oclk = 1,
};

enum disptop_lcdif_clk {
	lcdif_pixel_clkx_n = 0,
	lcdif_pixel_clk = 1,
};

#define HDMI_SPDIF_CLKGEN 2
#define HDMI_SPDIF_CLKOUT 0
#define HDMI_I_VCLK_CLKOUT 0
#define PHY_BASEADDR_DISPTOP_CLKGEN0_MODULE                                    \
	(PHY_BASEADDR_DISPLAYTOP_MODULE + OTHER_ADDR_OFFSET + 0x006000)
#define PHY_BASEADDR_DISPTOP_CLKGEN1_MODULE                                    \
	(PHY_BASEADDR_DISPLAYTOP_MODULE + OTHER_ADDR_OFFSET + 0x007000)
#define PHY_BASEADDR_DISPTOP_CLKGEN2_MODULE                                    \
	(PHY_BASEADDR_DISPLAYTOP_MODULE + OTHER_ADDR_OFFSET + 0x005000)
#define PHY_BASEADDR_DISPTOP_CLKGEN3_MODULE                                    \
	(PHY_BASEADDR_DISPLAYTOP_MODULE + OTHER_ADDR_OFFSET + 0x008000)
#define PHY_BASEADDR_DISPTOP_CLKGEN4_MODULE                                    \
	(PHY_BASEADDR_DISPLAYTOP_MODULE + OTHER_ADDR_OFFSET + 0x009000)

struct nx_disp_top_register_set {
	u32 resconv_mux_ctrl;
	u32 interconv_mux_ctrl;
	u32 mipi_mux_ctrl;
	u32 lvds_mux_ctrl;
	u32 hdmifixctrl0;
	u32 hdmisyncctrl0;
	u32 hdmisyncctrl1;
	u32 hdmisyncctrl2;
	u32 hdmisyncctrl3;
	u32 tftmpu_mux;
	u32 hdmifieldctrl;
	u32 greg0;
	u32 greg1;
	u32 greg2;
	u32 greg3;
	u32 greg4;
	u32 greg5;
};

int nx_disp_top_initialize(void);
u32 nx_disp_top_get_number_of_module(void);

u32 nx_disp_top_get_physical_address(void);
u32 nx_disp_top_get_size_of_register_set(void);
void nx_disp_top_set_base_address(void *base_address);
void *nx_disp_top_get_base_address(void);
int nx_disp_top_open_module(void);
int nx_disp_top_close_module(void);
int nx_disp_top_check_busy(void);

enum mux_index {
	primary_mlc = 0,
	secondary_mlc = 1,
	resolution_conv = 2,
};

enum prim_pad_mux_index {
	padmux_primary_mlc = 0,
	padmux_primary_mpu = 1,
	padmux_secondary_mlc = 2,
	padmux_resolution_conv = 3,
};

void nx_disp_top_set_resconvmux(int benb, u32 sel);
void nx_disp_top_set_hdmimux(int benb, u32 sel);
void nx_disp_top_set_mipimux(int benb, u32 sel);
void nx_disp_top_set_lvdsmux(int benb, u32 sel);
void nx_disp_top_set_primary_mux(u32 sel);
void nx_disp_top_hdmi_set_vsync_start(u32 sel);
void nx_disp_top_hdmi_set_vsync_hsstart_end(u32 start, u32 end);
void nx_disp_top_hdmi_set_hactive_start(u32 sel);
void nx_disp_top_hdmi_set_hactive_end(u32 sel);

void nx_disp_top_set_hdmifield(u32 enable, u32 init_val, u32 vsynctoggle,
			       u32 hsynctoggle, u32 vsyncclr, u32 hsyncclr,
			       u32 field_use, u32 muxsel);

enum padclk_config {
	padclk_clk = 0,
	padclk_inv_clk = 1,
	padclk_reserved_clk = 2,
	padclk_reserved_inv_clk = 3,
	padclk_clk_div2_0 = 4,
	padclk_clk_div2_90 = 5,
	padclk_clk_div2_180 = 6,
	padclk_clk_div2_270 = 7,
};

void nx_disp_top_set_padclock(u32 mux_index, u32 padclk_cfg);
void nx_disp_top_set_lcdif_enb(int enb);
void nx_disp_top_set_hdmifield(u32 enable, u32 init_val, u32 vsynctoggle,
			       u32 hsynctoggle, u32 vsyncclr, u32 hsyncclr,
			       u32 field_use, u32 muxsel);

#endif
