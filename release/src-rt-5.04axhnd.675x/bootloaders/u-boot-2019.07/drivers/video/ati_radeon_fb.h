#ifndef __ATI_RADEON_FB_H
#define __ATI_RADEON_FB_H

/***************************************************************
 * Most of the definitions here are adapted right from XFree86 *
 ***************************************************************/

/*
 * Chip families. Must fit in the low 16 bits of a long word
 */
enum radeon_family {
	CHIP_FAMILY_UNKNOW,
	CHIP_FAMILY_LEGACY,
	CHIP_FAMILY_RADEON,
	CHIP_FAMILY_RV100,
	CHIP_FAMILY_RS100,    /* U1 (IGP320M) or A3 (IGP320)*/
	CHIP_FAMILY_RV200,
	CHIP_FAMILY_RS200,    /* U2 (IGP330M/340M/350M) or A4 (IGP330/340/345/350),
				 RS250 (IGP 7000) */
	CHIP_FAMILY_R200,
	CHIP_FAMILY_RV250,
	CHIP_FAMILY_RS300,    /* Radeon 9000 IGP */
	CHIP_FAMILY_RV280,
	CHIP_FAMILY_R300,
	CHIP_FAMILY_R350,
	CHIP_FAMILY_RV350,
	CHIP_FAMILY_RV380,    /* RV370/RV380/M22/M24 */
	CHIP_FAMILY_R420,     /* R420/R423/M18 */
	CHIP_FAMILY_LAST,
};

#define IS_RV100_VARIANT(rinfo) (((rinfo)->family == CHIP_FAMILY_RV100)  || \
				 ((rinfo)->family == CHIP_FAMILY_RV200)  || \
				 ((rinfo)->family == CHIP_FAMILY_RS100)  || \
				 ((rinfo)->family == CHIP_FAMILY_RS200)  || \
				 ((rinfo)->family == CHIP_FAMILY_RV250)  || \
				 ((rinfo)->family == CHIP_FAMILY_RV280)  || \
				 ((rinfo)->family == CHIP_FAMILY_RS300))

#define IS_R300_VARIANT(rinfo) (((rinfo)->family == CHIP_FAMILY_R300)  || \
				((rinfo)->family == CHIP_FAMILY_RV350) || \
				((rinfo)->family == CHIP_FAMILY_R350)  || \
				((rinfo)->family == CHIP_FAMILY_RV380) || \
				((rinfo)->family == CHIP_FAMILY_R420))

struct radeonfb_info {
	char name[20];

	struct pci_device_id	pdev;
	u16			family;

	u32			fb_base_bus;
	u32			mmio_base_bus;

	void			*mmio_base;
	void			*fb_base;

	u32			video_ram;
	u32			mapped_vram;
	int			vram_width;
	int			vram_ddr;

	u32			fb_local_base;
};

#define INREG8(addr)		readb((rinfo->mmio_base)+addr)
#define OUTREG8(addr,val)	writeb(val, (rinfo->mmio_base)+addr)
#define INREG16(addr)		readw((rinfo->mmio_base)+addr)
#define OUTREG16(addr,val)	writew(val, (rinfo->mmio_base)+addr)
#define INREG(addr)		readl((rinfo->mmio_base)+addr)
#define OUTREG(addr,val)	writel(val, (rinfo->mmio_base)+addr)

static inline void _OUTREGP(struct radeonfb_info *rinfo, u32 addr,
		       u32 val, u32 mask)
{
	unsigned int tmp;

	tmp = INREG(addr);
	tmp &= (mask);
	tmp |= (val);
	OUTREG(addr, tmp);
}

#define OUTREGP(addr,val,mask)	_OUTREGP(rinfo, addr, val,mask)

/*
 * 2D Engine helper routines
 */
static inline void radeon_engine_flush (struct radeonfb_info *rinfo)
{
	int i;

	/* initiate flush */
	OUTREGP(RB2D_DSTCACHE_CTLSTAT, RB2D_DC_FLUSH_ALL,
		~RB2D_DC_FLUSH_ALL);

	for (i=0; i < 2000000; i++) {
		if (!(INREG(RB2D_DSTCACHE_CTLSTAT) & RB2D_DC_BUSY))
			return;
		udelay(1);
	}
	printf("radeonfb: Flush Timeout !\n");
}

static inline void _radeon_fifo_wait(struct radeonfb_info *rinfo, int entries)
{
	int i;

	for (i=0; i<2000000; i++) {
		if ((INREG(RBBM_STATUS) & 0x7f) >= entries)
			return;
		udelay(1);
	}
	printf("radeonfb: FIFO Timeout !\n");
}

static inline void _radeon_engine_idle(struct radeonfb_info *rinfo)
{
	int i;

	/* ensure FIFO is empty before waiting for idle */
	_radeon_fifo_wait (rinfo, 64);

	for (i=0; i<2000000; i++) {
		if (((INREG(RBBM_STATUS) & GUI_ACTIVE)) == 0) {
			radeon_engine_flush (rinfo);
			return;
		}
		udelay(1);
	}
	printf("radeonfb: Idle Timeout !\n");
}

#define radeon_engine_idle()		_radeon_engine_idle(rinfo)
#define radeon_fifo_wait(entries)	_radeon_fifo_wait(rinfo,entries)
#define radeon_msleep(ms)		_radeon_msleep(rinfo,ms)

/*
 * This structure contains the various registers manipulated by this
 * driver for setting or restoring a mode. It's mostly copied from
 * XFree's RADEONSaveRec structure. A few chip settings might still be
 * tweaked without beeing reflected or saved in these registers though
 */
struct radeon_regs {
	/* Common registers */
	u32		ovr_clr;
	u32		ovr_wid_left_right;
	u32		ovr_wid_top_bottom;
	u32		ov0_scale_cntl;
	u32		mpp_tb_config;
	u32		mpp_gp_config;
	u32		subpic_cntl;
	u32		viph_control;
	u32		i2c_cntl_1;
	u32		gen_int_cntl;
	u32		cap0_trig_cntl;
	u32		cap1_trig_cntl;
	u32		bus_cntl;
	u32		surface_cntl;
	u32		bios_5_scratch;

	/* Other registers to save for VT switches or driver load/unload */
	u32		dp_datatype;
	u32		rbbm_soft_reset;
	u32		clock_cntl_index;
	u32		amcgpio_en_reg;
	u32		amcgpio_mask;

	/* Surface/tiling registers */
	u32		surf_lower_bound[8];
	u32		surf_upper_bound[8];
	u32		surf_info[8];

	/* CRTC registers */
	u32		crtc_gen_cntl;
	u32		crtc_ext_cntl;
	u32		dac_cntl;
	u32		crtc_h_total_disp;
	u32		crtc_h_sync_strt_wid;
	u32		crtc_v_total_disp;
	u32		crtc_v_sync_strt_wid;
	u32		crtc_offset;
	u32		crtc_offset_cntl;
	u32		crtc_pitch;
	u32		disp_merge_cntl;
	u32		grph_buffer_cntl;
	u32		crtc_more_cntl;

	/* CRTC2 registers */
	u32		crtc2_gen_cntl;
	u32		dac2_cntl;
	u32		disp_output_cntl;
	u32		disp_hw_debug;
	u32		disp2_merge_cntl;
	u32		grph2_buffer_cntl;
	u32		crtc2_h_total_disp;
	u32		crtc2_h_sync_strt_wid;
	u32		crtc2_v_total_disp;
	u32		crtc2_v_sync_strt_wid;
	u32		crtc2_offset;
	u32		crtc2_offset_cntl;
	u32		crtc2_pitch;

	/* Flat panel regs */
	u32		fp_crtc_h_total_disp;
	u32		fp_crtc_v_total_disp;
	u32		fp_gen_cntl;
	u32		fp2_gen_cntl;
	u32		fp_h_sync_strt_wid;
	u32		fp2_h_sync_strt_wid;
	u32		fp_horz_stretch;
	u32		fp_panel_cntl;
	u32		fp_v_sync_strt_wid;
	u32		fp2_v_sync_strt_wid;
	u32		fp_vert_stretch;
	u32		lvds_gen_cntl;
	u32		lvds_pll_cntl;
	u32		tmds_crc;
	u32		tmds_transmitter_cntl;

	/* Computed values for PLL */
	u32		dot_clock_freq;
	int		feedback_div;
	int		post_div;

	/* PLL registers */
	u32		ppll_div_3;
	u32		ppll_ref_div;
	u32		vclk_ecp_cntl;
	u32		clk_cntl_index;

	/* Computed values for PLL2 */
	u32		dot_clock_freq_2;
	int		feedback_div_2;
	int		post_div_2;

	/* PLL2 registers */
	u32		p2pll_ref_div;
	u32		p2pll_div_0;
	u32		htotal_cntl2;

	/* Palette */
	int		palette_valid;
};

static inline u32 __INPLL(struct radeonfb_info *rinfo, u32 addr)
{
	u32 data;

	OUTREG8(CLOCK_CNTL_INDEX, addr & 0x0000003f);
	/* radeon_pll_errata_after_index(rinfo); */
	data = INREG(CLOCK_CNTL_DATA);
	/* radeon_pll_errata_after_data(rinfo); */
	return data;
}

static inline void __OUTPLL(struct radeonfb_info *rinfo, unsigned int index,
			    u32 val)
{

	OUTREG8(CLOCK_CNTL_INDEX, (index & 0x0000003f) | 0x00000080);
	/* radeon_pll_errata_after_index(rinfo); */
	OUTREG(CLOCK_CNTL_DATA, val);
	/* radeon_pll_errata_after_data(rinfo); */
}

static inline void __OUTPLLP(struct radeonfb_info *rinfo, unsigned int index,
			     u32 val, u32 mask)
{
	unsigned int tmp;

	tmp  = __INPLL(rinfo, index);
	tmp &= (mask);
	tmp |= (val);
	__OUTPLL(rinfo, index, tmp);
}

#define INPLL(addr)			__INPLL(rinfo, addr)
#define OUTPLL(index, val)		__OUTPLL(rinfo, index, val)
#define OUTPLLP(index, val, mask)	__OUTPLLP(rinfo, index, val, mask)

#endif
