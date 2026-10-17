/* SPDX-License-Identifier: GPL-2.0+
 *
 * Copyright (C) 2016  Nexell Co., Ltd.
 *
 * Author: junghyun, kim <jhkim@nexell.co.kr>
 */

#ifndef _S5PXX18_SOC_DPC_H_
#define _S5PXX18_SOC_DPC_H_

#include "s5pxx18_soc_disptype.h"

#define	IRQ_OFFSET	32
#define IRQ_DPC_P    (IRQ_OFFSET + 33)
#define IRQ_DPC_S   (IRQ_OFFSET + 34)

#define NUMBER_OF_DPC_MODULE	2
#define PHY_BASEADDR_DPC0	0xC0102800
#define PHY_BASEADDR_DPC1	0xC0102C00

#define	PHY_BASEADDR_DPC_LIST	\
		{ PHY_BASEADDR_DPC0, PHY_BASEADDR_DPC1 }

struct nx_dpc_register_set {
	u32 ntsc_stata;
	u32 ntsc_ecmda;
	u32 ntsc_ecmdb;
	u32 ntsc_glk;
	u32 ntsc_sch;
	u32 ntsc_hue;
	u32 ntsc_sat;
	u32 ntsc_cont;
	u32 ntsc_bright;
	u32 ntsc_fsc_adjh;
	u32 ntsc_fsc_adjl;
	u32 ntsc_ecmdc;
	u32 ntsc_csdly;
	u32 __ntsc_reserved_0_[3];
	u32 ntsc_dacsel10;
	u32 ntsc_dacsel32;
	u32 ntsc_dacsel54;
	u32 ntsc_daclp;
	u32 ntsc_dacpd;
	u32 __ntsc_reserved_1_[(0x20 - 0x15)];
	u32 ntsc_icntl;
	u32 ntsc_hvoffst;
	u32 ntsc_hoffst;
	u32 ntsc_voffset;
	u32 ntsc_hsvso;
	u32 ntsc_hsob;
	u32 ntsc_hsoe;
	u32 ntsc_vsob;
	u32 ntsc_vsoe;
	u32 __reserved[(0xf8 / 4) - 0x29];
	u32 dpchtotal;
	u32 dpchswidth;
	u32 dpchastart;
	u32 dpchaend;
	u32 dpcvtotal;
	u32 dpcvswidth;
	u32 dpcvastart;
	u32 dpcvaend;
	u32 dpcctrl0;
	u32 dpcctrl1;
	u32 dpcevtotal;
	u32 dpcevswidth;
	u32 dpcevastart;
	u32 dpcevaend;
	u32 dpcctrl2;
	u32 dpcvseoffset;
	u32 dpcvssoffset;
	u32 dpcevseoffset;
	u32 dpcevssoffset;
	u32 dpcdelay0;
	u32 dpcupscalecon0;
	u32 dpcupscalecon1;
	u32 dpcupscalecon2;

	u32 dpcrnumgencon0;
	u32 dpcrnumgencon1;
	u32 dpcrnumgencon2;
	u32 dpcrndconformula_l;
	u32 dpcrndconformula_h;
	u32 dpcfdtaddr;
	u32 dpcfrdithervalue;
	u32 dpcfgdithervalue;
	u32 dpcfbdithervalue;
	u32 dpcdelay1;
	u32 dpcmputime0;
	u32 dpcmputime1;
	u32 dpcmpuwrdatal;
	u32 dpcmpuindex;
	u32 dpcmpustatus;
	u32 dpcmpudatah;
	u32 dpcmpurdatal;
	u32 dpcdummy12;
	u32 dpccmdbufferdatal;
	u32 dpccmdbufferdatah;
	u32 dpcpolctrl;
	u32 dpcpadposition[8];
	u32 dpcrgbmask[2];
	u32 dpcrgbshift;
	u32 dpcdataflush;
	u32 __reserved06[((0x3c0) - (2 * 0x0ec)) / 4];

	u32 dpcclkenb;
	u32 dpcclkgen[2][2];
};

enum {
	nx_dpc_int_vsync = 0
};

enum nx_dpc_format {
	nx_dpc_format_rgb555 = 0ul,
	nx_dpc_format_rgb565 = 1ul,
	nx_dpc_format_rgb666 = 2ul,
	nx_dpc_format_rgb666b = 18ul,
	nx_dpc_format_rgb888 = 3ul,
	nx_dpc_format_mrgb555a = 4ul,
	nx_dpc_format_mrgb555b = 5ul,
	nx_dpc_format_mrgb565 = 6ul,
	nx_dpc_format_mrgb666 = 7ul,
	nx_dpc_format_mrgb888a = 8ul,
	nx_dpc_format_mrgb888b = 9ul,
	nx_dpc_format_ccir656 = 10ul,
	nx_dpc_format_ccir601a = 12ul,
	nx_dpc_format_ccir601b = 13ul,
	nx_dpc_format_srgb888 = 14ul,
	nx_dpc_format_srgbd8888 = 15ul,
	nx_dpc_format_4096color = 1ul,
	nx_dpc_format_16gray = 3ul
};

enum nx_dpc_ycorder {
	nx_dpc_ycorder_cb_ycr_y = 0ul,
	nx_dpc_ycorder_cr_ycb_y = 1ul,
	nx_dpc_ycorder_ycbycr = 2ul,
	nx_dpc_ycorder_ycrycb = 3ul
};

enum nx_dpc_padclk {
	nx_dpc_padclk_vclk = 0ul,
	nx_dpc_padclk_vclk2 = 1ul,
	nx_dpc_padclk_vclk3 = 2ul
};

enum nx_dpc_dither {
	nx_dpc_dither_bypass = 0ul,
	nx_dpc_dither_4bit = 1ul,
	nx_dpc_dither_5bit = 2ul,
	nx_dpc_dither_6bit = 3ul
};

enum nx_dpc_vbs {
	nx_dpc_vbs_ntsc_m = 0ul,
	nx_dpc_vbs_ntsc_n = 1ul,
	nx_dpc_vbs_ntsc_443 = 2ul,
	nx_dpc_vbs_pal_m = 3ul,
	nx_dpc_vbs_pal_n = 4ul,
	nx_dpc_vbs_pal_bghi = 5ul,
	nx_dpc_vbs_pseudo_pal = 6ul,
	nx_dpc_vbs_pseudo_ntsc = 7ul
};

enum nx_dpc_bandwidth {
	nx_dpc_bandwidth_low = 0ul,
	nx_dpc_bandwidth_medium = 1ul,
	nx_dpc_bandwidth_high = 2ul
};

int nx_dpc_initialize(void);
u32 nx_dpc_get_number_of_module(void);
u32 nx_dpc_get_physical_address(u32 module_index);
u32 nx_dpc_get_size_of_register_set(void);
void nx_dpc_set_base_address(u32 module_index, void *base_address);
void *nx_dpc_get_base_address(u32 module_index);
int nx_dpc_open_module(u32 module_index);
int nx_dpc_close_module(u32 module_index);
int nx_dpc_check_busy(u32 module_index);
int nx_dpc_can_power_down(u32 module_index);
int32_t nx_dpc_get_interrupt_number(u32 module_index);
void nx_dpc_set_interrupt_enable(u32 module_index, int32_t int_num,
				 int enable);
int nx_dpc_get_interrupt_enable(u32 module_index, int32_t int_num);
int nx_dpc_get_interrupt_pending(u32 module_index, int32_t int_num);
void nx_dpc_clear_interrupt_pending(u32 module_index, int32_t int_num);
void nx_dpc_set_interrupt_enable_all(u32 module_index, int enable);
int nx_dpc_get_interrupt_enable_all(u32 module_index);
int nx_dpc_get_interrupt_pending_all(u32 module_index);
void nx_dpc_clear_interrupt_pending_all(u32 module_index);
void nx_dpc_set_interrupt_enable32(u32 module_index, u32 enable_flag);
u32 nx_dpc_get_interrupt_enable32(u32 module_index);
u32 nx_dpc_get_interrupt_pending32(u32 module_index);
void nx_dpc_clear_interrupt_pending32(u32 module_index,
				      u32 pending_flag);
int32_t nx_dpc_get_interrupt_pending_number(u32 module_index);
void nx_dpc_set_clock_pclk_mode(u32 module_index, enum nx_pclkmode mode);
enum nx_pclkmode nx_dpc_get_clock_pclk_mode(u32 module_index);
void nx_dpc_set_clock_source(u32 module_index, u32 index, u32 clk_src);
u32 nx_dpc_get_clock_source(u32 module_index, u32 index);
void nx_dpc_set_clock_divisor(u32 module_index, u32 index, u32 divisor);
u32 nx_dpc_get_clock_divisor(u32 module_index, u32 index);
void nx_dpc_set_clock_out_inv(u32 module_index, u32 index,
			      int out_clk_inv);
int nx_dpc_get_clock_out_inv(u32 module_index, u32 index);
void nx_dpc_set_clock_out_select(u32 module_index, u32 index,
				 int bbypass);
int nx_dpc_get_clock_out_select(u32 module_index, u32 index);
void nx_dpc_set_clock_polarity(u32 module_index, int bpolarity);
int nx_dpc_get_clock_polarity(u32 module_index);
void nx_dpc_set_clock_out_enb(u32 module_index, u32 index,
			      int out_clk_enb);
int nx_dpc_get_clock_out_enb(u32 module_index, u32 index);
void nx_dpc_set_clock_out_delay(u32 module_index, u32 index, u32 delay);
u32 nx_dpc_get_clock_out_delay(u32 module_index, u32 index);
void nx_dpc_set_clock_divisor_enable(u32 module_index, int enable);
int nx_dpc_get_clock_divisor_enable(u32 module_index);

void nx_dpc_set_dpc_enable(u32 module_index, int benb);
int nx_dpc_get_dpc_enable(u32 module_index);
void nx_dpc_set_delay(u32 module_index, u32 delay_rgb_pvd,
		      u32 delay_hs_cp1, u32 delay_vs_fram,
		      u32 delay_de_cp2);
void nx_dpc_get_delay(u32 module_index, u32 *pdelayrgb_pvd,
		      u32 *pdelayhs_cp1, u32 *pdelayvs_fram,
		      u32 *pdelayde_cp2);
void nx_dpc_set_dither(u32 module_index, enum nx_dpc_dither dither_r,
		       enum nx_dpc_dither dither_g,
		       enum nx_dpc_dither dither_b);
void nx_dpc_get_dither(u32 module_index, enum nx_dpc_dither *pditherr,
		       enum nx_dpc_dither *pditherg,
		       enum nx_dpc_dither *pditherb);
void nx_dpc_set_horizontal_up_scaler(u32 module_index, int benb,
				     u32 sourcewidth, u32 destwidth);
void nx_dpc_get_horizontal_up_scaler(u32 module_index, int *pbenb,
				     u32 *psourcewidth,
				     u32 *pdestwidth);

void nx_dpc_set_mode(u32 module_index, enum nx_dpc_format format,
		     int binterlace, int binvertfield, int brgbmode,
		     int bswaprb, enum nx_dpc_ycorder ycorder,
		     int bclipyc, int bembeddedsync,
		     enum nx_dpc_padclk clock, int binvertclock,
		     int bdualview);
void nx_dpc_get_mode(u32 module_index, enum nx_dpc_format *pformat,
		     int *pbinterlace, int *pbinvertfield,
		     int *pbrgbmode, int *pbswaprb,
		     enum nx_dpc_ycorder *pycorder, int *pbclipyc,
		     int *pbembeddedsync, enum nx_dpc_padclk *pclock,
		     int *pbinvertclock, int *pbdualview);
void nx_dpc_set_hsync(u32 module_index, u32 avwidth, u32 hsw, u32 hfp,
		      u32 hbp, int binvhsync);
void nx_dpc_get_hsync(u32 module_index, u32 *pavwidth, u32 *phsw,
		      u32 *phfp, u32 *phbp, int *pbinvhsync);
void nx_dpc_set_vsync(u32 module_index, u32 avheight, u32 vsw, u32 vfp,
		      u32 vbp, int binvvsync, u32 eavheight, u32 evsw,
		      u32 evfp, u32 evbp);
void nx_dpc_get_vsync(u32 module_index, u32 *pavheight, u32 *pvsw,
		      u32 *pvfp, u32 *pvbp, int *pbinvvsync,
		      u32 *peavheight, u32 *pevsw, u32 *pevfp,
		      u32 *pevbp);
void nx_dpc_set_vsync_offset(u32 module_index, u32 vssoffset,
			     u32 vseoffset, u32 evssoffset,
			     u32 evseoffset);
void nx_dpc_get_vsync_offset(u32 module_index, u32 *pvssoffset,
			     u32 *pvseoffset, u32 *pevssoffset,
			     u32 *pevseoffset);

u32 nx_dpc_enable_pad_tft(u32 module_index, u32 mode_index);
u32 nx_dpc_enable_pad_i80(u32 module_index, u32 mode_index);

enum syncgenmode {
	progressive = 0,
	interlace = 1
};

enum polarity {
	polarity_activehigh = 0,
	polarity_activelow = 1
};

enum outputformat {
	outputformat_rgb555 = 0,
	outputformat_rgb565 = 1,
	outputformat_rgb666 = 2,
	outputformat_rgb888 = 3,
	outputformat_mrgb555a = 4,
	outputformat_mrgb555b = 5,
	outputformat_mrgb565 = 6,
	outputformat_mrgb666 = 7,
	outputformat_mrgb888a = 8,
	outputformat_mrgb888b = 9,
	outputformat_bgr555 = 10,
	outputformat_bgr565 = 11,
	outputformat_bgr666 = 12,
	outputformat_bgr888 = 13,
	outputformat_mbgr555a = 14,
	outputformat_mbgr555b = 15,
	outputformat_mbgr565 = 16,
	outputformat_mbgr666 = 17,
	outputformat_mbgr888a = 18,
	outputformat_mbgr888b = 19,
	outputformat_ccir656 = 20,
	outputformat_ccir601_8 = 21,
	outputformat_ccir601_16a = 22,
	outputformat_ccir601_16b = 23,
	outputformat_srgb888 = 24,
	outputformat_srgbd8888 = 25
};

enum outpadclksel {
	padvclk = 0,
	padvclk2 = 1,
	padvclk3 = 2
};

enum qmode {
	qmode_220 = 0,
	qmode_256 = 1
};

void nx_dpc_set_sync(u32 module_index, enum syncgenmode sync_gen_mode,
		     u32 avwidth, u32 avheight, u32 hsw, u32 hfp,
		     u32 hbp, u32 vsw, u32 vfp, u32 vbp,
		     enum polarity field_polarity,
		     enum polarity hsyncpolarity,
		     enum polarity vsyncpolarity, u32 even_vsw,
		     u32 even_vfp, u32 even_vbp, u32 vsetpixel,
		     u32 vsclrpixel, u32 evenvsetpixel,
		     u32 evenvsclrpixel);
void nx_dpc_set_output_format(u32 module_index,
			      enum outputformat output_format,
			      u8 output_video_config);
void nx_dpc_set_quantization_mode(u32 module_index, enum qmode rgb2yc,
				  enum qmode yc2rgb);
void nx_dpc_set_enable(u32 module_index, int enable, int rgbmode,
		       int use_ntscsync, int use_analog_output,
		       int seavenable);
void nx_dpc_set_enable_with_interlace(u32 module_index, int enable,
				      int rgbmode, int use_ntscsync,
				      int use_analog_output,
				      int seavenable);
void nx_dpc_set_enable_with_interlace(u32 module_index, int enable,
				      int rgbmode, int use_ntscsync,
				      int use_analog_output,
				      int seavenable);
void nx_dpc_set_out_video_clk_select(u32 module_index,
				     enum outpadclksel out_pad_vclk_sel);
void nx_dpc_set_reg_flush(u32 module_index);
void nx_dpc_set_sramon(u32 module_index);
void nx_dpc_set_sync_lcdtype(u32 module_index, int stnlcd,
			     int dual_view_enb, int bit_widh,
			     u8 cpcycle);
void nx_dpc_set_up_scale_control(u32 module_index, int up_scale_enb,
				 int filter_enb, u32 hscale,
				 u16 source_width);

void nx_dpc_set_mputime(u32 module_index, u8 setup, u8 hold, u8 acc);
void nx_dpc_set_index(u32 module_index, u32 index);
void nx_dpc_set_data(u32 module_index, u32 data);
void nx_dpc_set_cmd_buffer_flush(u32 module_index);
void nx_dpc_set_cmd_buffer_clear(u32 module_index);
void nx_dpc_set_cmd_buffer_write(u32 module_index, u32 cmd_data);
void nx_dpc_set(u32 module_index);
u32 nx_dpc_get_data(u32 module_index);
u32 nx_dpc_get_status(u32 module_index);
void nx_dpc_rgbmask(u32 module_index, u32 rgbmask);
void nx_dpc_set_pad_location(u32 module_index, u32 index, u32 regvalue);
u32 nx_dpc_get_field_flag(u32 module_index);

void nx_dpc_set_sync_v(u32 module_index, u32 avheight, u32 vsw, u32 vfp,
		       u32 vbp);

int nx_dpc_init_reg_test(u32 module_index);
void nx_dpc_set_encoder_control_reg(u32 module_index, u32 param_a,
				    u32 param_b, u32 param_c);
void nx_dpc_set_encoder_shcphase_control(u32 module_index,
					 u32 chroma_param);
void nx_dpc_set_encoder_timing_config_reg(u32 module_index, u32 inctl);
void nx_dpc_set_encoder_dacoutput_select(u32 module_index, u8 dacsel0,
					 u8 dacsel1, u8 dacsel2,
					 u8 dacsel3, u8 dacsel4,
					 u8 dacsel5);
void nx_dpc_set_encoder_sync_location(u32 module_index, u16 hsoe,
				      u16 hsob, u16 vsob, u16 vsoe,
				      u8 vsost, int novrst);
void nx_dpc_set_encoder_dacpower_enable(u32 module_index, u8 dacpd);
void nx_dpc_set_ycorder(u32 module_index, enum nx_dpc_ycorder ycorder);
void nx_dpc_set_luma_gain(u32 module_index, u32 luma_gain);

void nx_dpc_set_secondary_dpcsync(u32 module_index, int benb);
int nx_dpc_get_secondary_dpcsync(u32 module_index);
void nx_dpc_set_encenable(u32 module_index, int benb);
int nx_dpc_get_encenable(u32 module_index);
void nx_dpc_set_video_encoder_power_down(u32 module_index, int benb);
int nx_dpc_get_video_encoder_power_down(u32 module_index);
void nx_dpc_set_video_encoder_mode(u32 module_index, enum nx_dpc_vbs vbs,
				   int bpedestal);
void nx_dpc_set_video_encoder_schlock_control(u32 module_index,
					      int bfreerun);
int nx_dpc_get_video_encoder_schlock_control(u32 module_index);
void nx_dpc_set_video_encoder_bandwidth(u32 module_index,
					enum nx_dpc_bandwidth luma,
					enum nx_dpc_bandwidth chroma);
void nx_dpc_get_video_encoder_bandwidth(u32 module_index,
					enum nx_dpc_bandwidth *pluma,
					enum nx_dpc_bandwidth *pchroma);
void nx_dpc_set_video_encoder_color_control(u32 module_index, s8 sch,
					    s8 hue, s8 sat,
					    s8 crt, s8 brt);
void nx_dpc_get_video_encoder_color_control(u32 module_index,
					    s8 *psch, s8 *phue,
					    s8 *psat, s8 *pcrt,
					    s8 *pbrt);
void nx_dpc_set_video_encoder_fscadjust(u32 module_index,
					int16_t adjust);
u16 nx_dpc_get_video_encoder_fscadjust(u32 module_index);
void nx_dpc_set_video_encoder_timing(u32 module_index, u32 hsos,
				     u32 hsoe, u32 vsos, u32 vsoe);
void nx_dpc_get_video_encoder_timing(u32 module_index, u32 *phsos,
				     u32 *phsoe, u32 *pvsos,
				     u32 *pvsoe);
void nx_dpc_set_sync_v(u32 module_index, u32 avheight, u32 vsw, u32 vfp,
		       u32 vbp);

int nx_dpc_init_reg_test(u32 module_index);
void nx_dpc_set_encoder_control_reg(u32 module_index, u32 param_a,
				    u32 param_b, u32 param_c);
void nx_dpc_set_encoder_shcphase_control(u32 module_index,
					 u32 chroma_param);
void nx_dpc_set_encoder_timing_config_reg(u32 module_index, u32 inctl);
void nx_dpc_set_encoder_dacoutput_select(u32 module_index, u8 dacsel0,
					 u8 dacsel1, u8 dacsel2,
					 u8 dacsel3, u8 dacsel4,
					 u8 dacsel5);
void nx_dpc_set_encoder_sync_location(u32 module_index, u16 hsoe,
				      u16 hsob, u16 vsob, u16 vsoe,
				      u8 vsost, int novrst);
void nx_dpc_set_encoder_dacpower_enable(u32 module_index, u8 dacpd);
void nx_dpc_set_ycorder(u32 module_index, enum nx_dpc_ycorder ycorder);
void nx_dpc_set_luma_gain(u32 module_index, u32 luma_gain);

#endif
