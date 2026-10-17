// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016  Nexell Co., Ltd.
 *
 * Author: junghyun, kim <jhkim@nexell.co.kr>
 */

#include <linux/types.h>
#include <linux/io.h>

#include "s5pxx18_soc_disptop.h"
#include "s5pxx18_soc_mipi.h"

static struct nx_mipi_register_set *__g_pregister[NUMBER_OF_MIPI_MODULE];

int nx_mipi_smoke_test(u32 module_index)
{
	register struct nx_mipi_register_set *pregister;

	pregister = __g_pregister[module_index];

	if (pregister->csis_config_ch0 != 0x000000FC)
		return false;

	if (pregister->dsim_intmsk != 0xB337FFFF)
		return false;

	writel(0xDEADC0DE, &pregister->csis_dphyctrl);
	writel(0xFFFFFFFF, &pregister->csis_ctrl2);
	writel(0xDEADC0DE, &pregister->dsim_msync);

	if (pregister->csis_dphyctrl != 0xDE80001E)
		return false;

	if ((pregister->csis_ctrl2 & (~1)) != 0xEEE00010)
		return false;

	if (pregister->dsim_msync != 0xDE80C0DE)
		return false;

	return true;
}

void nx_mipi_set_base_address(u32 module_index, void *base_address)
{
	__g_pregister[module_index] =
	    (struct nx_mipi_register_set *)base_address;
}

void *nx_mipi_get_base_address(u32 module_index)
{
	return (void *)__g_pregister[module_index];
}

u32 nx_mipi_get_physical_address(u32 module_index)
{
	const u32 physical_addr[] = PHY_BASEADDR_MIPI_LIST;

	return physical_addr[module_index];
}

#define __nx_mipi_valid_dsi_intmask__	\
	(~((1 << 26) | (1 << 23) | (1 << 22) | (1 << 19)))

void nx_mipi_set_interrupt_enable(u32 module_index, u32 int_num, int enable)
{
	register struct nx_mipi_register_set *pregister;
	register u32 regvalue;

	pregister = __g_pregister[module_index];
	if (int_num < 32) {
		regvalue = pregister->csis_intmsk;
		regvalue &= ~(1ul << int_num);
		regvalue |= (u32)enable << int_num;
		writel(regvalue, &pregister->csis_intmsk);
	} else {
		regvalue = pregister->dsim_intmsk;
		regvalue &= ~(1ul << (int_num - 32));
		regvalue |= (u32)enable << (int_num - 32);
		writel(regvalue, &pregister->dsim_intmsk);
	}
}

int nx_mipi_get_interrupt_enable(u32 module_index, u32 int_num)
{
	if (int_num < 32)
		return (int)((__g_pregister[module_index]->csis_intmsk >>
			      int_num) & 0x01);
	else
		return (int)((__g_pregister[module_index]->dsim_intmsk >>
			      (int_num - 32)) & 0x01);
}

int nx_mipi_get_interrupt_pending(u32 module_index, u32 int_num)
{
	register struct nx_mipi_register_set *pregister;
	register u32 regvalue;
	int ret;

	pregister = __g_pregister[module_index];
	if (int_num < 32) {
		regvalue = pregister->csis_intmsk;
		regvalue &= pregister->csis_intsrc;
		ret = (int)((regvalue >> int_num) & 0x01);
	} else {
		regvalue = pregister->dsim_intmsk;
		regvalue &= pregister->dsim_intsrc;
		ret = (int)((regvalue >> (int_num - 32)) & 0x01);
	}

	return ret;
}

void nx_mipi_clear_interrupt_pending(u32 module_index, u32 int_num)
{
	register struct nx_mipi_register_set *pregister;

	pregister = __g_pregister[module_index];
	if (int_num < 32)
		writel(1ul << int_num, &pregister->csis_intsrc);
	else
		writel(1ul << (int_num - 32), &pregister->dsim_intsrc);
}

void nx_mipi_set_interrupt_enable_all(u32 module_index, int enable)
{
	register struct nx_mipi_register_set *pregister;

	pregister = __g_pregister[module_index];
	if (enable)
		writel(__nx_mipi_valid_dsi_intmask__, &pregister->dsim_intmsk);
	else
		writel(0, &pregister->dsim_intmsk);
}

int nx_mipi_get_interrupt_enable_all(u32 module_index)
{
	if (__g_pregister[module_index]->csis_intmsk)
		return true;

	if (__g_pregister[module_index]->dsim_intmsk)
		return true;

	return false;
}

int nx_mipi_get_interrupt_pending_all(u32 module_index)
{
	register struct nx_mipi_register_set *pregister;
	register u32 regvalue;

	pregister = __g_pregister[module_index];
	regvalue = pregister->csis_intmsk;
	regvalue &= pregister->csis_intsrc;

	if (regvalue)
		return true;

	regvalue = pregister->dsim_intmsk;
	regvalue &= pregister->dsim_intsrc;

	if (regvalue)
		return true;

	return false;
}

void nx_mipi_clear_interrupt_pending_all(u32 module_index)
{
	register struct nx_mipi_register_set *pregister;

	pregister = __g_pregister[module_index];
	writel(__nx_mipi_valid_dsi_intmask__, &pregister->dsim_intsrc);
}

int32_t nx_mipi_get_interrupt_pending_number(u32 module_index)
{
	register struct nx_mipi_register_set *pregister;
	register u32 regvalue;
	int i;

	pregister = __g_pregister[module_index];
	regvalue = pregister->csis_intmsk;
	regvalue &= pregister->csis_intsrc;
	if (regvalue != 0) {
		for (i = 0; i < 32; i++) {
			if (regvalue & 1ul)
				return i;
			regvalue >>= 1;
		}
	}

	regvalue = pregister->dsim_intmsk;
	regvalue &= pregister->dsim_intsrc;
	if (regvalue != 0) {
		for (i = 0; i < 32; i++) {
			if (regvalue & 1ul)
				return i + 32;
			regvalue >>= 1;
		}
	}
	return -1;
}

#define writereg(regname, mask, value) \
	regvalue = pregister->(regname);	\
	regvalue = (regvalue & (~(mask))) | (value); \
	writel(regvalue, &pregister->(regname))

void nx_mipi_dsi_get_status(u32 module_index, u32 *pulps, u32 *pstop,
			    u32 *pispllstable, u32 *pisinreset,
			    u32 *pisbackward, u32 *pishsclockready)
{
	register struct nx_mipi_register_set *pregister;
	register u32 regvalue;

	pregister = __g_pregister[module_index];
	regvalue = pregister->dsim_status;
	if (pulps) {
		*pulps = 0;
		if (regvalue & (1 << 4))
			*pulps |= (1 << 0);
		if (regvalue & (1 << 5))
			*pulps |= (1 << 1);
		if (regvalue & (1 << 6))
			*pulps |= (1 << 2);
		if (regvalue & (1 << 7))
			*pulps |= (1 << 3);
		if (regvalue & (1 << 9))
			*pulps |= (1 << 4);
	}

	if (pstop) {
		*pstop = 0;
		if (regvalue & (1 << 0))
			*pstop |= (1 << 0);
		if (regvalue & (1 << 1))
			*pstop |= (1 << 1);
		if (regvalue & (1 << 2))
			*pstop |= (1 << 2);
		if (regvalue & (1 << 3))
			*pstop |= (1 << 3);
		if (regvalue & (1 << 8))
			*pstop |= (1 << 4);
	}

	if (pispllstable)
		*pispllstable = (regvalue >> 31) & 1;

	if (pisinreset)
		*pisinreset = ((regvalue >> 20) & 1) ? 0 : 1;

	if (pisbackward)
		*pisbackward = (regvalue >> 16) & 1;

	if (pishsclockready)
		*pishsclockready = (regvalue >> 10) & 1;
}

void nx_mipi_dsi_software_reset(u32 module_index)
{
	register struct nx_mipi_register_set *pregister;

	pregister = __g_pregister[module_index];

	writel(0x00010001, &pregister->dsim_swrst);

	while (0 != (readl(&pregister->dsim_status) & (1 << 20)))
		;

	writel(0x00000000, &pregister->dsim_swrst);
}

void nx_mipi_dsi_set_clock(u32 module_index, int enable_txhsclock,
			   int use_external_clock, int enable_byte_clock,
			   int enable_escclock_clock_lane,
			   int enable_escclock_data_lane0,
			   int enable_escclock_data_lane1,
			   int enable_escclock_data_lane2,
			   int enable_escclock_data_lane3,
			   int enable_escprescaler, u32 escprescalervalue)
{
	register struct nx_mipi_register_set *pregister;
	register u32 regvalue;

	pregister = __g_pregister[module_index];
	regvalue = 0;
	regvalue |= (enable_txhsclock << 31);
	regvalue |= (use_external_clock << 27);
	regvalue |= (enable_byte_clock << 24);
	regvalue |= (enable_escclock_clock_lane << 19);
	regvalue |= (enable_escclock_data_lane0 << 20);
	regvalue |= (enable_escclock_data_lane1 << 21);
	regvalue |= (enable_escclock_data_lane2 << 22);
	regvalue |= (enable_escclock_data_lane3 << 23);
	regvalue |= (enable_escprescaler << 28);
	regvalue |= escprescalervalue;

	writel(regvalue, &pregister->dsim_clkctrl);
}

void nx_mipi_dsi_set_timeout(u32 module_index, u32 bta_tout, u32 lpdrtout)
{
	register struct nx_mipi_register_set *pregister;
	register u32 regvalue;

	pregister = __g_pregister[module_index];
	regvalue = 0;
	regvalue |= (bta_tout << 16);
	regvalue |= (lpdrtout << 0);

	writel(regvalue, &pregister->dsim_timeout);
}

void nx_mipi_dsi_set_config_video_mode(u32 module_index,
				       int enable_auto_flush_main_display_fifo,
				       int enable_auto_vertical_count,
				       int enable_burst,
				       enum nx_mipi_dsi_syncmode sync_mode,
				       int enable_eo_tpacket,
				       int enable_hsync_end_packet,
				       int enable_hfp, int enable_hbp,
				       int enable_hsa,
				       u32 number_of_virtual_channel,
				       enum nx_mipi_dsi_format format,
				       u32 number_of_words_in_hfp,
				       u32 number_of_words_in_hbp,
				       u32 number_of_words_in_hsync,
				       u32 number_of_lines_in_vfp,
				       u32 number_of_lines_in_vbp,
				       u32 number_of_lines_in_vsync,
				       u32 number_of_lines_in_command_allow)
{
	register struct nx_mipi_register_set *pregister;
	register u32 regvalue;
	u32 newvalue;

	pregister = __g_pregister[module_index];
	newvalue = (1 << 25);
	newvalue |= ((1 - enable_auto_flush_main_display_fifo) << 29);
	newvalue |= (enable_auto_vertical_count << 24);
	newvalue |= (enable_burst << 26);
	newvalue |= (sync_mode << 27);
	newvalue |= ((1 - enable_eo_tpacket) << 28);
	newvalue |= (enable_hsync_end_packet << 23);
	newvalue |= ((1 - enable_hfp) << 22);
	newvalue |= ((1 - enable_hbp) << 21);
	newvalue |= ((1 - enable_hsa) << 20);
	newvalue |= (number_of_virtual_channel << 18);
	newvalue |= (format << 12);

	writereg(dsim_config, 0xFFFFFF00, newvalue);

	newvalue = (number_of_lines_in_command_allow << 28);
	newvalue |= (number_of_lines_in_vfp << 16);
	newvalue |= (number_of_lines_in_vbp << 0);

	writel(newvalue, &pregister->dsim_mvporch);

	newvalue = (number_of_words_in_hfp << 16);
	newvalue |= (number_of_words_in_hbp << 0);

	writel(newvalue, &pregister->dsim_mhporch);

	newvalue = (number_of_words_in_hsync << 0);
	newvalue |= (number_of_lines_in_vsync << 22);

	writel(newvalue, &pregister->dsim_msync);
}

void nx_mipi_dsi_set_config_command_mode(u32 module_index,
					 int
					 enable_auto_flush_main_display_fifo,
					 int enable_eo_tpacket,
					 u32 number_of_virtual_channel,
					 enum nx_mipi_dsi_format format)
{
	register struct nx_mipi_register_set *pregister;
	register u32 regvalue;
	u32 newvalue;

	pregister = __g_pregister[module_index];
	newvalue = (0 << 25);
	newvalue |= (enable_auto_flush_main_display_fifo << 29);
	newvalue |= (enable_eo_tpacket << 28);
	newvalue |= (number_of_virtual_channel << 18);
	newvalue |= (format << 12);
	writereg(dsim_config, 0xFFFFFF00, newvalue);
}

void nx_mipi_dsi_set_escape_mode(u32 module_index, u32 stop_state_count,
				 int force_stop_state, int force_bta,
				 enum nx_mipi_dsi_lpmode cmdin_lp,
				 enum nx_mipi_dsi_lpmode txinlp)
{
	register struct nx_mipi_register_set *pregister;
	register u32 regvalue;
	u32 newvalue;

	pregister = __g_pregister[module_index];
	newvalue = (stop_state_count << 21);
	newvalue |= (force_stop_state << 20);
	newvalue |= (force_bta << 16);
	newvalue |= (cmdin_lp << 7);
	newvalue |= (txinlp << 6);
	writereg(dsim_escmode, 0xFFFFFFC0, newvalue);
}

void nx_mipi_dsi_set_escape_lp(u32 module_index,
			       enum nx_mipi_dsi_lpmode cmdin_lp,
			       enum nx_mipi_dsi_lpmode txinlp)
{
	register struct nx_mipi_register_set *pregister;
	register u32 regvalue;
	u32 newvalue = 0;

	pregister = __g_pregister[module_index];
	newvalue |= (cmdin_lp << 7);
	newvalue |= (txinlp << 6);
	writereg(dsim_escmode, 0xC0, newvalue);
}

void nx_mipi_dsi_remote_reset_trigger(u32 module_index)
{
	register struct nx_mipi_register_set *pregister;
	register u32 regvalue;
	u32 newvalue;

	pregister = __g_pregister[module_index];
	newvalue = (1 << 4);
	writereg(dsim_escmode, (1 << 4), newvalue);

	while (readl(&pregister->dsim_escmode) & (1 << 4))
		;
}

void nx_mipi_dsi_set_ulps(u32 module_index, int ulpsclocklane, int ulpsdatalane)
{
	register struct nx_mipi_register_set *pregister;
	register u32 regvalue;

	pregister = __g_pregister[module_index];
	regvalue = pregister->dsim_escmode;

	if (ulpsclocklane) {
		regvalue &= ~(1 << 0);
		regvalue |= (1 << 1);
	} else {
		regvalue |= (1 << 0);
	}

	if (ulpsdatalane) {
		regvalue &= ~(1 << 2);
		regvalue |= (1 << 3);
	} else {
		regvalue |= (1 << 2);
	}

	writel(regvalue, &pregister->dsim_escmode);

	if (ulpsclocklane)
		while ((1 << 9) ==
		       (readl(&pregister->dsim_status) & (1 << 9)))
			;
	else
		while (0 != (readl(&pregister->dsim_status) & (1 << 9)))
			;

	if (ulpsdatalane)
		while ((15 << 4) ==
		       (readl(&pregister->dsim_status) & (15 << 4)))
			;
	else
		while (0 != (readl(&pregister->dsim_status) & (15 << 4)))
			;

	if (!ulpsclocklane)
		regvalue &= (3 << 0);

	if (!ulpsdatalane)
		regvalue |= (3 << 2);

	writel(regvalue, &pregister->dsim_escmode);
}

void nx_mipi_dsi_set_size(u32 module_index, u32 width, u32 height)
{
	register struct nx_mipi_register_set *pregister;
	register u32 regvalue;
	u32 newvalue;

	pregister = __g_pregister[module_index];
	newvalue = (height << 16);
	newvalue |= (width << 0);
	writereg(dsim_mdresol, 0x0FFFFFFF, newvalue);
}

void nx_mipi_dsi_set_enable(u32 module_index, int enable)
{
	register struct nx_mipi_register_set *pregister;
	register u32 regvalue;

	pregister = __g_pregister[module_index];
	writereg(dsim_mdresol, (1 << 31), (enable << 31));
}

void nx_mipi_dsi_set_phy(u32 module_index, u32 number_of_data_lanes,
			 int enable_clock_lane, int enable_data_lane0,
			 int enable_data_lane1, int enable_data_lane2,
			 int enable_data_lane3, int swap_clock_lane,
			 int swap_data_lane)
{
	register struct nx_mipi_register_set *pregister;
	register u32 regvalue;
	u32 newvalue;

	pregister = __g_pregister[module_index];
	newvalue = (number_of_data_lanes << 5);
	newvalue |= (enable_clock_lane << 0);
	newvalue |= (enable_data_lane0 << 1);
	newvalue |= (enable_data_lane1 << 2);
	newvalue |= (enable_data_lane2 << 3);
	newvalue |= (enable_data_lane3 << 4);
	writereg(dsim_config, 0xFF, newvalue);
	newvalue = (swap_clock_lane << 1);
	newvalue |= (swap_data_lane << 0);
	writereg(dsim_phyacchr1, 0x3, newvalue);
}

void nx_mipi_dsi_set_pll(u32 module_index, int enable, u32 pllstabletimer,
			 u32 m_pllpms, u32 m_bandctl, u32 m_dphyctl,
			 u32 b_dphyctl)
{
	register struct nx_mipi_register_set *pregister;
	register u32 regvalue;
	u32 newvalue;

	pregister = __g_pregister[module_index];
	if (!enable) {
		newvalue = (enable << 23);
		newvalue |= (m_pllpms << 1);
		newvalue |= (m_bandctl << 24);
		writereg(dsim_pllctrl, 0x0FFFFFFF, newvalue);
	}

	writel(m_dphyctl, &pregister->dsim_phyacchr);
	writel(pllstabletimer, &pregister->dsim_plltmr);
	writel((b_dphyctl << 9), &pregister->dsim_phyacchr1);

	if (enable) {
		newvalue = (enable << 23);
		newvalue |= (m_pllpms << 1);
		newvalue |= (m_bandctl << 24);
		writereg(dsim_pllctrl, 0x0FFFFFFF, newvalue);
	}
}

void nx_mipi_dsi_write_pkheader(u32 module_index, u32 data)
{
	register struct nx_mipi_register_set *pregister;

	pregister = __g_pregister[module_index];
	writel(data, &pregister->dsim_pkthdr);
}

void nx_mipi_dsi_write_payload(u32 module_index, u32 data)
{
	register struct nx_mipi_register_set *pregister;

	pregister = __g_pregister[module_index];
	writel(data, &pregister->dsim_payload);
}

u32 nx_mipi_dsi_read_fifo_status(u32 module_index)
{
	register struct nx_mipi_register_set *pregister;

	pregister = __g_pregister[module_index];
	return readl(&pregister->dsim_fifoctrl);
}
