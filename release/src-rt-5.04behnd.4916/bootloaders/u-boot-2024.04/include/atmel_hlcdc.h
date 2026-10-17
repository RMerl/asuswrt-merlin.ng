/* SPDX-License-Identifier: GPL-2.0+ */
/*
 *  Header file for AT91/AT32 MULTI LAYER LCD Controller
 *
 *  Data structure and register user interface
 *
 *  Copyright (C) 2012 Atmel Corporation
 */
#ifndef __ATMEL_HLCDC_H__
#define __ATMEL_HLCDC_H__

/* Atmel multi layer lcdc hardware registers */
struct atmel_hlcd_regs {
	u32	lcdc_lcdcfg0;
	u32	lcdc_lcdcfg1;
	u32	lcdc_lcdcfg2;
	u32	lcdc_lcdcfg3;
	u32	lcdc_lcdcfg4;
	u32	lcdc_lcdcfg5;
	u32	lcdc_lcdcfg6;
	u32	res1;
	u32	lcdc_lcden;
	u32	lcdc_lcddis;
	u32	lcdc_lcdsr;
	u32	res2;
	u32	lcdc_lcdidr;
	u32	res3[3];
	u32	lcdc_basecher;
	u32	res4[3];
	u32	lcdc_baseidr;
	u32	res5[3];
	u32	lcdc_baseaddr;
	u32	lcdc_basectrl;
	u32	lcdc_basenext;
	u32	lcdc_basecfg0;
	u32	lcdc_basecfg1;
	u32	lcdc_basecfg2;
	u32	lcdc_basecfg3;
	u32	lcdc_basecfg4;
};

#define LCDC_LCDCFG0_CLKPOL	(0x1 << 0)
#define LCDC_LCDCFG0_CLKSEL	(0x1 << 2)
#define LCDC_LCDCFG0_CLKPWMSEL	(0x1 << 3)
#define LCDC_LCDCFG0_CGDISBASE	(0x1 << 8)
#define LCDC_LCDCFG0_CGDISOVR1	(0x1 << 9)
#define LCDC_LCDCFG0_CGDISHEO	(0x1 << 11)
#define LCDC_LCDCFG0_CGDISHCR	(0x1 << 12)
#define LCDC_LCDCFG0_CLKDIV_Pos	16
#define LCDC_LCDCFG0_CLKDIV_Msk	(0xff << LCDC_LCDCFG0_CLKDIV_Pos)
#define LCDC_LCDCFG0_CLKDIV(value) \
	((LCDC_LCDCFG0_CLKDIV_Msk & ((value) << LCDC_LCDCFG0_CLKDIV_Pos)))

#define LCDC_LCDCFG1_HSPW_Pos	0
#define LCDC_LCDCFG1_HSPW_Msk	(0x3f << LCDC_LCDCFG1_HSPW_Pos)
#define LCDC_LCDCFG1_HSPW(value) \
	((LCDC_LCDCFG1_HSPW_Msk & ((value) << LCDC_LCDCFG1_HSPW_Pos)))
#define LCDC_LCDCFG1_VSPW_Pos	16
#define LCDC_LCDCFG1_VSPW_Msk	(0x3f << LCDC_LCDCFG1_VSPW_Pos)
#define LCDC_LCDCFG1_VSPW(value) \
	((LCDC_LCDCFG1_VSPW_Msk & ((value) << LCDC_LCDCFG1_VSPW_Pos)))

#define LCDC_LCDCFG2_VFPW_Pos	0
#define LCDC_LCDCFG2_VFPW_Msk	(0x3f << LCDC_LCDCFG2_VFPW_Pos)
#define LCDC_LCDCFG2_VFPW(value) \
	((LCDC_LCDCFG2_VFPW_Msk & ((value) << LCDC_LCDCFG2_VFPW_Pos)))
#define LCDC_LCDCFG2_VBPW_Pos	16
#define LCDC_LCDCFG2_VBPW_Msk	(0x3f << LCDC_LCDCFG2_VBPW_Pos)
#define LCDC_LCDCFG2_VBPW(value) \
	((LCDC_LCDCFG2_VBPW_Msk & ((value) << LCDC_LCDCFG2_VBPW_Pos)))

#define LCDC_LCDCFG3_HFPW_Pos	0
#define LCDC_LCDCFG3_HFPW_Msk	(0xff << LCDC_LCDCFG3_HFPW_Pos)
#define LCDC_LCDCFG3_HFPW(value) \
	((LCDC_LCDCFG3_HFPW_Msk & ((value) << LCDC_LCDCFG3_HFPW_Pos)))
#define LCDC_LCDCFG3_HBPW_Pos	16
#define LCDC_LCDCFG3_HBPW_Msk	(0xff << LCDC_LCDCFG3_HBPW_Pos)
#define LCDC_LCDCFG3_HBPW(value) \
	((LCDC_LCDCFG3_HBPW_Msk & ((value) << LCDC_LCDCFG3_HBPW_Pos)))

#define LCDC_LCDCFG4_PPL_Pos	0
#define LCDC_LCDCFG4_PPL_Msk	(0x7ff << LCDC_LCDCFG4_PPL_Pos)
#define LCDC_LCDCFG4_PPL(value) \
	((LCDC_LCDCFG4_PPL_Msk & ((value) << LCDC_LCDCFG4_PPL_Pos)))
#define LCDC_LCDCFG4_RPF_Pos	16
#define LCDC_LCDCFG4_RPF_Msk	(0x7ff << LCDC_LCDCFG4_RPF_Pos)
#define LCDC_LCDCFG4_RPF(value) \
	((LCDC_LCDCFG4_RPF_Msk & ((value) << LCDC_LCDCFG4_RPF_Pos)))

#define LCDC_LCDCFG5_HSPOL	(0x1 << 0)
#define LCDC_LCDCFG5_VSPOL	(0x1 << 1)
#define LCDC_LCDCFG5_VSPDLYS	(0x1 << 2)
#define LCDC_LCDCFG5_VSPDLYE	(0x1 << 3)
#define LCDC_LCDCFG5_DISPPOL	(0x1 << 4)
#define LCDC_LCDCFG5_SERIAL	(0x1 << 5)
#define LCDC_LCDCFG5_DITHER	(0x1 << 6)
#define LCDC_LCDCFG5_DISPDLY	(0x1 << 7)
#define LCDC_LCDCFG5_MODE_Pos	8
#define LCDC_LCDCFG5_MODE_Msk	(0x3 << LCDC_LCDCFG5_MODE_Pos)
#define LCDC_LCDCFG5_MODE_OUTPUT_12BPP	(0x0 << 8)
#define LCDC_LCDCFG5_MODE_OUTPUT_16BPP	(0x1 << 8)
#define LCDC_LCDCFG5_MODE_OUTPUT_18BPP	(0x2 << 8)
#define LCDC_LCDCFG5_MODE_OUTPUT_24BPP	(0x3 << 8)
#define LCDC_LCDCFG5_VSPSU		(0x1 << 12)
#define LCDC_LCDCFG5_VSPHO		(0x1 << 13)
#define LCDC_LCDCFG5_GUARDTIME_Pos	16
#define LCDC_LCDCFG5_GUARDTIME_Msk	(0x1f << LCDC_LCDCFG5_GUARDTIME_Pos)
#define LCDC_LCDCFG5_GUARDTIME(value) \
	((LCDC_LCDCFG5_GUARDTIME_Msk & ((value) << LCDC_LCDCFG5_GUARDTIME_Pos)))

#define LCDC_LCDCFG6_PWMPS_Pos		0
#define LCDC_LCDCFG6_PWMPS_Msk		(0x7 << LCDC_LCDCFG6_PWMPS_Pos)
#define LCDC_LCDCFG6_PWMPS(value) \
	((LCDC_LCDCFG6_PWMPS_Msk & ((value) << LCDC_LCDCFG6_PWMPS_Pos)))
#define LCDC_LCDCFG6_PWMPOL		(0x1 << 4)
#define LCDC_LCDCFG6_PWMCVAL_Pos	8
#define LCDC_LCDCFG6_PWMCVAL_Msk	(0xff << LCDC_LCDCFG6_PWMCVAL_Pos)
#define LCDC_LCDCFG6_PWMCVAL(value) \
	((LCDC_LCDCFG6_PWMCVAL_Msk & ((value) << LCDC_LCDCFG6_PWMCVAL_Pos)))

#define LCDC_LCDEN_CLKEN	(0x1 << 0)
#define LCDC_LCDEN_SYNCEN	(0x1 << 1)
#define LCDC_LCDEN_DISPEN	(0x1 << 2)
#define LCDC_LCDEN_PWMEN	(0x1 << 3)

#define LCDC_LCDDIS_CLKDIS	(0x1 << 0)
#define LCDC_LCDDIS_SYNCDIS	(0x1 << 1)
#define LCDC_LCDDIS_DISPDIS	(0x1 << 2)
#define LCDC_LCDDIS_PWMDIS	(0x1 << 3)
#define LCDC_LCDDIS_CLKRST	(0x1 << 8)
#define LCDC_LCDDIS_SYNCRST	(0x1 << 9)
#define LCDC_LCDDIS_DISPRST	(0x1 << 10)
#define LCDC_LCDDIS_PWMRST	(0x1 << 11)

#define LCDC_LCDSR_CLKSTS	(0x1 << 0)
#define LCDC_LCDSR_LCDSTS	(0x1 << 1)
#define LCDC_LCDSR_DISPSTS	(0x1 << 2)
#define LCDC_LCDSR_PWMSTS	(0x1 << 3)
#define LCDC_LCDSR_SIPSTS	(0x1 << 4)

#define LCDC_LCDIDR_SOFID	(0x1 << 0)
#define LCDC_LCDIDR_DISID	(0x1 << 1)
#define LCDC_LCDIDR_DISPID	(0x1 << 2)
#define LCDC_LCDIDR_FIFOERRID	(0x1 << 4)
#define LCDC_LCDIDR_BASEID	(0x1 << 8)
#define LCDC_LCDIDR_OVR1ID	(0x1 << 9)
#define LCDC_LCDIDR_HEOID	(0x1 << 11)
#define LCDC_LCDIDR_HCRID	(0x1 << 12)

#define LCDC_BASECHER_CHEN	(0x1 << 0)
#define LCDC_BASECHER_UPDATEEN	(0x1 << 1)
#define LCDC_BASECHER_A2QEN	(0x1 << 2)

#define LCDC_BASEIDR_DMA	(0x1 << 2)
#define LCDC_BASEIDR_DSCR	(0x1 << 3)
#define LCDC_BASEIDR_ADD	(0x1 << 4)
#define LCDC_BASEIDR_DONE	(0x1 << 5)
#define LCDC_BASEIDR_OVR	(0x1 << 6)

#define LCDC_BASECTRL_DFETCH	(0x1 << 0)
#define LCDC_BASECTRL_LFETCH	(0x1 << 1)
#define LCDC_BASECTRL_DMAIEN	(0x1 << 2)
#define LCDC_BASECTRL_DSCRIEN	(0x1 << 3)
#define LCDC_BASECTRL_ADDIEN	(0x1 << 4)
#define LCDC_BASECTRL_DONEIEN	(0x1 << 5)

#define LCDC_BASECFG0_BLEN_Pos		4
#define LCDC_BASECFG0_BLEN_AHB_SINGLE	(0x0 << 4)
#define LCDC_BASECFG0_BLEN_AHB_INCR4	(0x1 << 4)
#define LCDC_BASECFG0_BLEN_AHB_INCR8	(0x2 << 4)
#define LCDC_BASECFG0_BLEN_AHB_INCR16	(0x3 << 4)
#define LCDC_BASECFG0_DLBO		(0x1 << 8)

#define LCDC_BASECFG1_RGBMODE_12BPP_RGB_444		(0x0 << 4)
#define LCDC_BASECFG1_RGBMODE_16BPP_ARGB_4444		(0x1 << 4)
#define LCDC_BASECFG1_RGBMODE_16BPP_RGBA_4444		(0x2 << 4)
#define LCDC_BASECFG1_RGBMODE_16BPP_RGB_565		(0x3 << 4)
#define LCDC_BASECFG1_RGBMODE_16BPP_TRGB_1555		(0x4 << 4)
#define LCDC_BASECFG1_RGBMODE_18BPP_RGB_666		(0x5 << 4)
#define LCDC_BASECFG1_RGBMODE_18BPP_RGB_666_PACKED	(0x6 << 4)
#define LCDC_BASECFG1_RGBMODE_19BPP_TRGB_1666		(0x7 << 4)
#define LCDC_BASECFG1_RGBMODE_19BPP_TRGB_PACKED		(0x8 << 4)
#define LCDC_BASECFG1_RGBMODE_24BPP_RGB_888		(0x9 << 4)
#define LCDC_BASECFG1_RGBMODE_24BPP_RGB_888_PACKED	(0xA << 4)
#define LCDC_BASECFG1_RGBMODE_25BPP_TRGB_1888		(0xB << 4)
#define LCDC_BASECFG1_RGBMODE_32BPP_ARGB_8888		(0xC << 4)
#define LCDC_BASECFG1_RGBMODE_32BPP_RGBA_8888		(0xD << 4)

#define LCDC_BASECFG2_XSTRIDE_Pos 0
#define LCDC_BASECFG2_XSTRIDE_Msk (0xffffffff << LCDC_BASECFG2_XSTRIDE_Pos)
#define LCDC_BASECFG2_XSTRIDE(value) \
	((LCDC_BASECFG2_XSTRIDE_Msk & ((value) << LCDC_BASECFG2_XSTRIDE_Pos)))

#define LCDC_BASECFG3_BDEF_Pos	0
#define LCDC_BASECFG3_BDEF_Msk	(0xff << LCDC_BASECFG3_BDEF_Pos)
#define LCDC_BASECFG3_BDEF(value) \
	((LCDC_BASECFG3_BDEF_Msk & ((value) << LCDC_BASECFG3_BDEF_Pos)))
#define LCDC_BASECFG3_GDEF_Pos	8
#define LCDC_BASECFG3_GDEF_Msk	(0xff << LCDC_BASECFG3_GDEF_Pos)
#define LCDC_BASECFG3_GDEF(value) \
	((LCDC_BASECFG3_GDEF_Msk & ((value) << LCDC_BASECFG3_GDEF_Pos)))
#define LCDC_BASECFG3_RDEF_Pos	16
#define LCDC_BASECFG3_RDEF_Msk	(0xff << LCDC_BASECFG3_RDEF_Pos)
#define LCDC_BASECFG3_RDEF(value) \
	((LCDC_BASECFG3_RDEF_Msk & ((value) << LCDC_BASECFG3_RDEF_Pos)))

#define LCDC_BASECLUT_BCLUT_Pos 0
#define LCDC_BASECLUT_BCLUT_Msk (0xff << LCDC_BASECLUT_BCLUT_Pos)
#define LCDC_BASECLUT_GCLUT_Pos 8
#define LCDC_BASECLUT_GCLUT_Msk (0xff << LCDC_BASECLUT_GCLUT_Pos)
#define LCDC_BASECLUT_RCLUT_Pos 16
#define LCDC_BASECLUT_RCLUT_Msk (0xff << LCDC_BASECLUT_RCLUT_Pos)

#define LCDC_BASECFG4_DMA	(0x1 << 8)
#define LCDC_BASECFG4_REP	(0x1 << 9)

struct lcd_dma_desc {
	u32	address;
	u32	control;
	u32	next;
};

#define ATMEL_LCDC_LUT(n)	(0x0400 + ((n)*4))

#endif /* __ATMEL_HLCDC_H__ */
