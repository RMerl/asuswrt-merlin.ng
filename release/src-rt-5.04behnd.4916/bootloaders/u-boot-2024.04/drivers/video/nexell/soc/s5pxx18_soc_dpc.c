// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016  Nexell Co., Ltd.
 *
 * Author: junghyun, kim <jhkim@nexell.co.kr>
 */

#include <linux/types.h>
#include <linux/io.h>

#include "s5pxx18_soc_dpc.h"

static struct {
	struct nx_dpc_register_set *pregister;
} __g_module_variables[NUMBER_OF_DPC_MODULE] = { { NULL,},};

int nx_dpc_initialize(void)
{
	static int binit;
	u32 i;

	if (binit == 0) {
		for (i = 0; i < NUMBER_OF_DPC_MODULE; i++)
			__g_module_variables[i].pregister = NULL;
		binit = 1;
	}
	return 1;
}

u32 nx_dpc_get_number_of_module(void)
{
	return NUMBER_OF_DPC_MODULE;
}

u32 nx_dpc_get_physical_address(u32 module_index)
{
	const u32 physical_addr[] = PHY_BASEADDR_DPC_LIST;

	return physical_addr[module_index];
}

void nx_dpc_set_base_address(u32 module_index, void *base_address)
{
	__g_module_variables[module_index].pregister =
	    (struct nx_dpc_register_set *)base_address;
}

void *nx_dpc_get_base_address(u32 module_index)
{
	return (void *)__g_module_variables[module_index].pregister;
}

void nx_dpc_set_interrupt_enable(u32 module_index, int32_t int_num, int enable)
{
	const u32 intenb_pos = 11;
	const u32 intenb_mask = 1ul << intenb_pos;
	const u32 intpend_pos = 10;
	const u32 intpend_mask = 1ul << intpend_pos;

	register u32 regvalue;
	register struct nx_dpc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	regvalue = pregister->dpcctrl0;
	regvalue &= ~(intenb_mask | intpend_mask);
	regvalue |= (u32)enable << intenb_pos;

	writel(regvalue, &pregister->dpcctrl0);
}

int nx_dpc_get_interrupt_enable(u32 module_index, int32_t int_num)
{
	const u32 intenb_pos = 11;
	const u32 intenb_mask = 1ul << intenb_pos;

	return (int)((__g_module_variables[module_index].pregister->dpcctrl0 &
		      intenb_mask) >> intenb_pos);
}

void nx_dpc_set_interrupt_enable32(u32 module_index, u32 enable_flag)
{
	const u32 intenb_pos = 11;
	const u32 intenb_mask = 1 << intenb_pos;
	const u32 intpend_pos = 10;
	const u32 intpend_mask = 1 << intpend_pos;

	register struct nx_dpc_register_set *pregister;
	register u32 read_value;

	pregister = __g_module_variables[module_index].pregister;
	read_value = pregister->dpcctrl0 & ~(intpend_mask | intenb_mask);

	writel((u32)(read_value | (enable_flag & 0x01) << intenb_pos),
	       &pregister->dpcctrl0);
}

u32 nx_dpc_get_interrupt_enable32(u32 module_index)
{
	const u32 intenb_pos = 11;
	const u32 intenb_mask = 1 << intenb_pos;

	return (u32)((__g_module_variables[module_index].pregister->dpcctrl0 &
		       intenb_mask) >> intenb_pos);
}

int nx_dpc_get_interrupt_pending(u32 module_index, int32_t int_num)
{
	const u32 intpend_pos = 10;
	const u32 intpend_mask = 1ul << intpend_pos;

	return (int)((__g_module_variables[module_index].pregister->dpcctrl0 &
		      intpend_mask) >> intpend_pos);
}

u32 nx_dpc_get_interrupt_pending32(u32 module_index)
{
	const u32 intpend_pos = 10;
	const u32 intpend_mask = 1 << intpend_pos;

	return (u32)((__g_module_variables[module_index].pregister->dpcctrl0 &
		       intpend_mask) >> intpend_pos);
}

void nx_dpc_clear_interrupt_pending(u32 module_index, int32_t int_num)
{
	const u32 intpend_pos = 10;
	register struct nx_dpc_register_set *pregister;
	register u32 regvalue;

	pregister = __g_module_variables[module_index].pregister;
	regvalue = pregister->dpcctrl0;
	regvalue |= 1ul << intpend_pos;

	writel(regvalue, &pregister->dpcctrl0);
}

void nx_dpc_clear_interrupt_pending32(u32 module_index, u32 pending_flag)
{
	const u32 intpend_pos = 10;
	const u32 intpend_mask = 1 << intpend_pos;
	register struct nx_dpc_register_set *pregister;
	register u32 read_value;

	pregister = __g_module_variables[module_index].pregister;
	read_value = pregister->dpcctrl0 & ~intpend_mask;

	writel((u32)(read_value | ((pending_flag & 0x01) << intpend_pos)),
	       &pregister->dpcctrl0);
}

void nx_dpc_set_interrupt_enable_all(u32 module_index, int enable)
{
	const u32 intenb_pos = 11;
	const u32 intenb_mask = 1ul << intenb_pos;
	const u32 intpend_pos = 10;
	const u32 intpend_mask = 1ul << intpend_pos;
	register u32 regvalue;
	register struct nx_dpc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	regvalue = pregister->dpcctrl0;
	regvalue &= ~(intenb_mask | intpend_mask);
	regvalue |= (u32)enable << intenb_pos;

	writel(regvalue, &pregister->dpcctrl0);
}

int nx_dpc_get_interrupt_enable_all(u32 module_index)
{
	const u32 intenb_pos = 11;
	const u32 intenb_mask = 1ul << intenb_pos;

	return (int)((__g_module_variables[module_index].pregister->dpcctrl0 &
		      intenb_mask) >> intenb_pos);
}

int nx_dpc_get_interrupt_pending_all(u32 module_index)
{
	const u32 intpend_pos = 10;
	const u32 intpend_mask = 1ul << intpend_pos;

	return (int)((__g_module_variables[module_index].pregister->dpcctrl0 &
		      intpend_mask) >> intpend_pos);
}

void nx_dpc_clear_interrupt_pending_all(u32 module_index)
{
	const u32 intpend_pos = 10;
	register struct nx_dpc_register_set *pregister;
	register u32 regvalue;

	pregister = __g_module_variables[module_index].pregister;
	regvalue = pregister->dpcctrl0;
	regvalue |= 1ul << intpend_pos;

	writel(regvalue, &pregister->dpcctrl0);
}

int32_t nx_dpc_get_interrupt_pending_number(u32 module_index)
{
	const u32 intenb_pos = 11;
	const u32 intpend_pos = 10;
	register struct nx_dpc_register_set *pregister;
	register u32 pend;

	pregister = __g_module_variables[module_index].pregister;
	pend = ((pregister->dpcctrl0 >> intenb_pos) &&
		(pregister->dpcctrl0 >> intpend_pos));

	if (pend & 0x01)
		return 0;

	return -1;
}

void nx_dpc_set_clock_pclk_mode(u32 module_index, enum nx_pclkmode mode)
{
	const u32 pclkmode_pos = 3;
	register u32 regvalue;
	register struct nx_dpc_register_set *pregister;
	u32 clkmode = 0;

	pregister = __g_module_variables[module_index].pregister;
	switch (mode) {
	case nx_pclkmode_dynamic:
		clkmode = 0;
		break;
	case nx_pclkmode_always:
		clkmode = 1;
		break;
	default:
		break;
	}
	regvalue = pregister->dpcclkenb;
	regvalue &= ~(1ul << pclkmode_pos);
	regvalue |= (clkmode & 0x01) << pclkmode_pos;

	writel(regvalue, &pregister->dpcclkenb);
}

enum nx_pclkmode nx_dpc_get_clock_pclk_mode(u32 module_index)
{
	const u32 pclkmode_pos = 3;

	if (__g_module_variables[module_index].pregister->dpcclkenb &
	    (1ul << pclkmode_pos)) {
		return nx_pclkmode_always;
	}
	return nx_pclkmode_dynamic;
}

void nx_dpc_set_clock_source(u32 module_index, u32 index, u32 clk_src)
{
	const u32 clksrcsel_pos = 2;
	const u32 clksrcsel_mask = 0x07 << clksrcsel_pos;
	register struct nx_dpc_register_set *pregister;
	register u32 read_value;

	pregister = __g_module_variables[module_index].pregister;
	read_value = pregister->dpcclkgen[index][0];
	read_value &= ~clksrcsel_mask;
	read_value |= clk_src << clksrcsel_pos;

	writel(read_value, &pregister->dpcclkgen[index][0]);
}

u32 nx_dpc_get_clock_source(u32 module_index, u32 index)
{
	const u32 clksrcsel_pos = 2;
	const u32 clksrcsel_mask = 0x07 << clksrcsel_pos;

	return (__g_module_variables[module_index]
		.pregister->dpcclkgen[index][0] &
		clksrcsel_mask) >> clksrcsel_pos;
}

void nx_dpc_set_clock_divisor(u32 module_index, u32 index, u32 divisor)
{
	const u32 clkdiv_pos = 5;
	const u32 clkdiv_mask = ((1 << 8) - 1) << clkdiv_pos;
	register struct nx_dpc_register_set *pregister;
	register u32 read_value;

	pregister = __g_module_variables[module_index].pregister;
	read_value = pregister->dpcclkgen[index][0];
	read_value &= ~clkdiv_mask;
	read_value |= (divisor - 1) << clkdiv_pos;

	writel(read_value, &pregister->dpcclkgen[index][0]);
}

u32 nx_dpc_get_clock_divisor(u32 module_index, u32 index)
{
	const u32 clkdiv_pos = 5;
	const u32 clkdiv_mask = ((1 << 8) - 1) << clkdiv_pos;

	return ((__g_module_variables[module_index]
	       .pregister->dpcclkgen[index][0] &
	       clkdiv_mask) >> clkdiv_pos) + 1;
}

void nx_dpc_set_clock_out_inv(u32 module_index, u32 index, int out_clk_inv)
{
	const u32 outclkinv_pos = 1;
	const u32 outclkinv_mask = 1ul << outclkinv_pos;
	register struct nx_dpc_register_set *pregister;
	register u32 read_value;

	pregister = __g_module_variables[module_index].pregister;
	read_value = pregister->dpcclkgen[index][0];
	read_value &= ~outclkinv_mask;
	read_value |= out_clk_inv << outclkinv_pos;

	writel(read_value, &pregister->dpcclkgen[index][0]);
}

int nx_dpc_get_clock_out_inv(u32 module_index, u32 index)
{
	const u32 outclkinv_pos = 1;
	const u32 outclkinv_mask = 1ul << outclkinv_pos;

	return (int)((__g_module_variables[module_index]
		     .pregister->dpcclkgen[index][0] &
		     outclkinv_mask) >> outclkinv_pos);
}

void nx_dpc_set_clock_out_select(u32 module_index, u32 index, int bbypass)
{
	const u32 outclksel_pos = 0;
	const u32 outclksel_mask = 1ul << outclksel_pos;
	register struct nx_dpc_register_set *pregister;
	register u32 read_value;

	pregister = __g_module_variables[module_index].pregister;
	read_value = pregister->dpcclkgen[index][0];
	read_value &= ~outclksel_mask;
	if (bbypass == 0)
		read_value |= outclksel_mask;

	writel(read_value, &pregister->dpcclkgen[index][0]);
}

int nx_dpc_get_clock_out_select(u32 module_index, u32 index)
{
	const u32 outclksel_pos = 0;
	const u32 outclksel_mask = 1ul << outclksel_pos;

	if (__g_module_variables[module_index].pregister->dpcclkgen[index][0] &
	    outclksel_mask) {
		return 0;
	} else {
		return 1;
	}
}

void nx_dpc_set_clock_polarity(u32 module_index, int bpolarity)
{
	const u32 clkpol_pos = 2;
	const u32 clkpol_mask = 1ul << clkpol_pos;
	register struct nx_dpc_register_set *pregister;
	register u32 read_value;

	pregister = __g_module_variables[module_index].pregister;
	read_value = pregister->dpcctrl1;
	read_value &= ~clkpol_mask;
	if (bpolarity == 1)
		read_value |= clkpol_mask;

	writel(read_value, &pregister->dpcctrl1);
}

int nx_dpc_get_clock_polarity(u32 module_index)
{
	const u32 clkpol_pos = 2;
	const u32 clkpol_mask = 1ul << clkpol_pos;

	if (__g_module_variables[module_index].pregister->dpcctrl1 &
	    clkpol_mask) {
		return 1;
	} else {
		return 0;
	}
}

void nx_dpc_set_clock_out_enb(u32 module_index, u32 index, int out_clk_enb)
{
	const u32 outclkenb_pos = 15;
	const u32 outclkenb_mask = 1ul << outclkenb_pos;
	register struct nx_dpc_register_set *pregister;
	register u32 read_value;

	pregister = __g_module_variables[module_index].pregister;
	read_value = pregister->dpcclkgen[index][0];
	read_value &= ~outclkenb_mask;

	if (out_clk_enb == 1)
		read_value |= outclkenb_mask;

	writel(read_value, &pregister->dpcclkgen[index][0]);
}

int nx_dpc_get_clock_out_enb(u32 module_index, u32 index)
{
	const u32 outclkenb_pos = 15;
	const u32 outclkenb_mask = 1ul << outclkenb_pos;

	if (__g_module_variables[module_index].pregister->dpcclkgen[index][0] &
	    outclkenb_mask) {
		return 1;
	} else {
		return 0;
	}
}

void nx_dpc_set_clock_out_delay(u32 module_index, u32 index, u32 delay)
{
	const u32 outclkdelay_pos = 0;
	const u32 outclkdelay_mask = 0x1f << outclkdelay_pos;
	register struct nx_dpc_register_set *pregister;
	register u32 read_value;

	pregister = __g_module_variables[module_index].pregister;
	read_value = pregister->dpcclkgen[index][1];
	read_value &= ~outclkdelay_mask;
	read_value |= (u32)delay << outclkdelay_pos;

	writel(read_value, &pregister->dpcclkgen[index][1]);
}

u32 nx_dpc_get_clock_out_delay(u32 module_index, u32 index)
{
	register struct nx_dpc_register_set *pregister;
	const u32 outclkdelay_pos = 0;
	const u32 outclkdelay_mask = 0x1f << outclkdelay_pos;

	pregister = __g_module_variables[module_index].pregister;

	return (u32)((pregister->dpcclkgen[index][1] & outclkdelay_mask) >>
		     outclkdelay_pos);
}

void nx_dpc_set_clock_divisor_enable(u32 module_index, int enable)
{
	const u32 clkgenenb_pos = 2;
	const u32 clkgenenb_mask = 1ul << clkgenenb_pos;
	register struct nx_dpc_register_set *pregister;
	register u32 read_value;

	pregister = __g_module_variables[module_index].pregister;
	read_value = pregister->dpcclkenb;
	read_value &= ~clkgenenb_mask;
	read_value |= (u32)enable << clkgenenb_pos;

	writel(read_value, &pregister->dpcclkenb);
}

int nx_dpc_get_clock_divisor_enable(u32 module_index)
{
	const u32 clkgenenb_pos = 2;
	const u32 clkgenenb_mask = 1ul << clkgenenb_pos;

	return (int)((__g_module_variables[module_index].pregister->dpcclkenb &
		     clkgenenb_mask) >> clkgenenb_pos);
}

void nx_dpc_set_dpc_enable(u32 module_index, int benb)
{
	const u32 intpend_pos = 10;
	const u32 intpend_mask = 1ul << intpend_pos;
	const u32 dpcenb_pos = 15;
	const u32 dpcenb_mask = 1ul << dpcenb_pos;
	register struct nx_dpc_register_set *pregister;
	register u32 read_value;

	pregister = __g_module_variables[module_index].pregister;
	read_value = pregister->dpcctrl0;
	read_value &= ~(intpend_mask | dpcenb_mask);
	read_value |= (u32)benb << dpcenb_pos;

	writel(read_value, &pregister->dpcctrl0);
}

int nx_dpc_get_dpc_enable(u32 module_index)
{
	const u32 dpcenb_pos = 15;
	const u32 dpcenb_mask = 1ul << dpcenb_pos;

	return (int)((__g_module_variables[module_index].pregister->dpcctrl0 &
		      dpcenb_mask) >> dpcenb_pos);
}

void nx_dpc_set_delay(u32 module_index, u32 delay_rgb_pvd, u32 delay_hs_cp1,
		      u32 delay_vs_fram, u32 delay_de_cp2)
{
	const u32 intpend_mask = 1u << 10;
	const u32 delayrgb_pos = 4;
	const u32 delayrgb_mask = 0xfu << delayrgb_pos;
	register u32 temp;
	const u32 delayde_pos = 0;
	const u32 delayvs_pos = 8;
	const u32 delayhs_pos = 0;
	register struct nx_dpc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	temp = pregister->dpcctrl0;
	temp &= (u32)~(intpend_mask | delayrgb_mask);
	temp = (u32)(temp | (delay_rgb_pvd << delayrgb_pos));

	writel(temp, &pregister->dpcctrl0);

	writel((u32)((delay_vs_fram << delayvs_pos) |
		   (delay_hs_cp1 << delayhs_pos)), &pregister->dpcdelay0);

	writel((u32)(delay_de_cp2 << delayde_pos), &pregister->dpcdelay1);
}

void nx_dpc_get_delay(u32 module_index, u32 *pdelayrgb_pvd, u32 *pdelayhs_cp1,
		      u32 *pdelayvs_fram, u32 *pdelayde_cp2)
{
	const u32 delayrgb_pos = 4;
	const u32 delayrgb_mask = 0xfu << delayrgb_pos;
	const u32 delayde_pos = 0;
	const u32 delayde_mask = 0x3fu << delayde_pos;
	const u32 delayvs_pos = 8;
	const u32 delayvs_mask = 0x3fu << delayvs_pos;
	const u32 delayhs_pos = 0;
	const u32 delayhs_mask = 0x3fu << delayhs_pos;
	register u32 temp;

	temp = __g_module_variables[module_index].pregister->dpcctrl0;
	if (pdelayrgb_pvd)
		*pdelayrgb_pvd = (u32)((temp & delayrgb_mask) >> delayrgb_pos);
	temp = __g_module_variables[module_index].pregister->dpcdelay0;
	if (pdelayhs_cp1)
		*pdelayhs_cp1 = (u32)((temp & delayhs_mask) >> delayhs_pos);
	if (pdelayvs_fram)
		*pdelayvs_fram = (u32)((temp & delayvs_mask) >> delayvs_pos);
	temp = __g_module_variables[module_index].pregister->dpcdelay1;
	if (pdelayde_cp2)
		*pdelayde_cp2 = (u32)((temp & delayde_mask) >> delayde_pos);
}

void nx_dpc_set_dither(u32 module_index, enum nx_dpc_dither dither_r,
		       enum nx_dpc_dither dither_g, enum nx_dpc_dither dither_b)
{
	const u32 dither_mask = 0x3fu;
	const u32 rdither_pos = 0;
	const u32 gdither_pos = 2;
	const u32 bdither_pos = 4;
	register u32 temp;
	register struct nx_dpc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	temp = pregister->dpcctrl1;
	temp &= (u32)~dither_mask;
	temp = (u32)(temp |
		     ((dither_b << bdither_pos) | (dither_g << gdither_pos) |
		     (dither_r << rdither_pos)));

	writel(temp, &pregister->dpcctrl1);
}

void nx_dpc_get_dither(u32 module_index, enum nx_dpc_dither *pditherr,
		       enum nx_dpc_dither *pditherg,
		       enum nx_dpc_dither *pditherb)
{
	const u32 rdither_pos = 0;
	const u32 rdither_mask = 0x3u << rdither_pos;
	const u32 gdither_pos = 2;
	const u32 gdither_mask = 0x3u << gdither_pos;
	const u32 bdither_pos = 4;
	const u32 bdither_mask = 0x3u << bdither_pos;
	register u32 temp;

	temp = __g_module_variables[module_index].pregister->dpcctrl1;
	if (pditherr)
		*pditherr =
		    (enum nx_dpc_dither)((temp & rdither_mask) >> rdither_pos);
	if (pditherg)
		*pditherg =
		    (enum nx_dpc_dither)((temp & gdither_mask) >> gdither_pos);
	if (pditherb)
		*pditherb =
		    (enum nx_dpc_dither)((temp & bdither_mask) >> bdither_pos);
}

void nx_dpc_set_mode(u32 module_index, enum nx_dpc_format format,
		     int binterlace, int binvertfield, int brgbmode,
		     int bswaprb, enum nx_dpc_ycorder ycorder, int bclipyc,
		     int bembeddedsync, enum nx_dpc_padclk clock,
		     int binvertclock, int bdualview)
{
	const u32 polfield_pos = 2;
	const u32 seavenb_pos = 8;
	const u32 scanmode_pos = 9;
	const u32 intpend_pos = 10;
	const u32 rgbmode_pos = 12;

	const u32 dither_mask = 0x3f;
	const u32 ycorder_pos = 6;
	const u32 format_pos = 8;
	const u32 ycrange_pos = 13;
	const u32 swaprb_pos = 15;

	const u32 padclksel_pos = 0;
	const u32 padclksel_mask = 3u << padclksel_pos;
	const u32 lcdtype_pos = 7;
	const u32 lcdtype_mask = 3u << lcdtype_pos;
	register struct nx_dpc_register_set *pregister;
	register u32 temp;

	pregister = __g_module_variables[module_index].pregister;
	temp = pregister->dpcctrl0;
	temp &= (u32)~(1u << intpend_pos);
	if (binterlace)
		temp |= (u32)(1u << scanmode_pos);
	else
		temp &= (u32)~(1u << scanmode_pos);
	if (binvertfield)
		temp |= (u32)(1u << polfield_pos);
	else
		temp &= (u32)~(1u << polfield_pos);
	if (brgbmode)
		temp |= (u32)(1u << rgbmode_pos);
	else
		temp &= (u32)~(1u << rgbmode_pos);
	if (bembeddedsync)
		temp |= (u32)(1u << seavenb_pos);
	else
		temp &= (u32)~(1u << seavenb_pos);

	writel(temp, &pregister->dpcctrl0);
	temp = pregister->dpcctrl1;
	temp &= (u32)dither_mask;
	temp = (u32)(temp | (ycorder << ycorder_pos));
	if (format >= 16) {
		register u32 temp1;

		temp1 = pregister->dpcctrl2;
		temp1 = temp1 | (1 << 4);
		writel(temp1, &pregister->dpcctrl2);
	} else {
		register u32 temp1;

		temp1 = pregister->dpcctrl2;
		temp1 = temp1 & ~(1 << 4);
		writel(temp1, &pregister->dpcctrl2);
	}
	temp = (u32)(temp | ((format & 0xf) << format_pos));
	if (!bclipyc)
		temp |= (u32)(1u << ycrange_pos);
	if (bswaprb)
		temp |= (u32)(1u << swaprb_pos);

	writel(temp, &pregister->dpcctrl1);
	temp = pregister->dpcctrl2;
	temp &= (u32)~(padclksel_mask | lcdtype_mask);
	temp = (u32)(temp | (clock << padclksel_pos));

	writel(temp, &pregister->dpcctrl2);

	nx_dpc_set_clock_out_inv(module_index, 0, binvertclock);
	nx_dpc_set_clock_out_inv(module_index, 1, binvertclock);
}

void nx_dpc_get_mode(u32 module_index, enum nx_dpc_format *pformat,
		     int *pbinterlace, int *pbinvertfield, int *pbrgbmode,
		     int *pbswaprb, enum nx_dpc_ycorder *pycorder,
		     int *pbclipyc, int *pbembeddedsync,
		     enum nx_dpc_padclk *pclock, int *pbinvertclock,
		     int *pbdualview)
{
	const u32 polfield = 1u << 2;
	const u32 seavenb = 1u << 8;
	const u32 scanmode = 1u << 9;
	const u32 rgbmode = 1u << 12;

	const u32 ycorder_pos = 6;
	const u32 ycorder_mask = 0x3u << ycorder_pos;
	const u32 format_pos = 8;
	const u32 format_mask = 0xfu << format_pos;
	const u32 ycrange = 1u << 13;
	const u32 swaprb = 1u << 15;

	const u32 padclksel_pos = 0;
	const u32 padclksel_mask = 3u << padclksel_pos;
	const u32 lcdtype_pos = 7;
	const u32 lcdtype_mask = 3u << lcdtype_pos;
	register u32 temp;

	temp = __g_module_variables[module_index].pregister->dpcctrl0;
	if (pbinterlace)
		*pbinterlace = (temp & scanmode) ? 1 : 0;

	if (pbinvertfield)
		*pbinvertfield = (temp & polfield) ? 1 : 0;

	if (pbrgbmode)
		*pbrgbmode = (temp & rgbmode) ? 1 : 0;

	if (pbembeddedsync)
		*pbembeddedsync = (temp & seavenb) ? 1 : 0;

	temp = __g_module_variables[module_index].pregister->dpcctrl1;

	if (pycorder)
		*pycorder =
		    (enum nx_dpc_ycorder)((temp & ycorder_mask) >> ycorder_pos);

	if (pformat)
		*pformat =
		    (enum nx_dpc_format)((temp & format_mask) >> format_pos);
	if (pbclipyc)
		*pbclipyc = (temp & ycrange) ? 0 : 1;
	if (pbswaprb)
		*pbswaprb = (temp & swaprb) ? 1 : 0;

	temp = __g_module_variables[module_index].pregister->dpcctrl2;

	if (pclock)
		*pclock =
		    (enum nx_dpc_padclk)((temp & padclksel_mask) >>
					 padclksel_pos);

	if (pbdualview)
		*pbdualview = (2 == ((temp & lcdtype_mask) >> lcdtype_pos))
		    ? 1 : 0;

	if (pbinvertclock)
		*pbinvertclock = nx_dpc_get_clock_out_inv(module_index, 1);
}

void nx_dpc_set_hsync(u32 module_index, u32 avwidth, u32 hsw, u32 hfp, u32 hbp,
		      int binvhsync)
{
	const u32 intpend = 1u << 10;
	const u32 polhsync = 1u << 0;
	register u32 temp;
	register struct nx_dpc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;

	writel((u32)(hsw + hbp + avwidth + hfp - 1), &pregister->dpchtotal);

	writel((u32)(hsw - 1), &pregister->dpchswidth);

	writel((u32)(hsw + hbp - 1), &pregister->dpchastart);

	writel((u32)(hsw + hbp + avwidth - 1), &pregister->dpchaend);
	temp = pregister->dpcctrl0;
	temp &= ~intpend;
	if (binvhsync)
		temp |= (u32)polhsync;
	else
		temp &= (u32)~polhsync;

	writel(temp, &pregister->dpcctrl0);
}

void nx_dpc_get_hsync(u32 module_index, u32 *pavwidth, u32 *phsw, u32 *phfp,
		      u32 *phbp, int *pbinvhsync)
{
	const u32 polhsync = 1u << 0;
	u32 htotal, hsw, hab, hae;
	u32 avw, hfp, hbp;
	register struct nx_dpc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	htotal = (u32)pregister->dpchtotal + 1;
	hsw = (u32)pregister->dpchswidth + 1;
	hab = (u32)pregister->dpchastart + 1;
	hae = (u32)pregister->dpchaend + 1;
	hbp = hab - hsw;
	avw = hae - hab;
	hfp = htotal - hae;
	if (pavwidth)
		*pavwidth = avw;
	if (phsw)
		*phsw = hsw;
	if (phfp)
		*phfp = hfp;
	if (phbp)
		*phbp = hbp;
	if (pbinvhsync)
		*pbinvhsync = (pregister->dpcctrl0 & polhsync) ? 1 : 0;
}

void nx_dpc_set_vsync(u32 module_index, u32 avheight, u32 vsw, u32 vfp, u32 vbp,
		      int binvvsync, u32 eavheight, u32 evsw, u32 evfp,
		      u32 evbp)
{
	const u32 intpend = 1u << 10;
	const u32 polvsync = 1u << 1;
	register u32 temp;
	register struct nx_dpc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;

	writel((u32)(vsw + vbp + avheight + vfp - 1), &pregister->dpcvtotal);

	writel((u32)(vsw - 1), &pregister->dpcvswidth);

	writel((u32)(vsw + vbp - 1), &pregister->dpcvastart);

	writel((u32)(vsw + vbp + avheight - 1), &pregister->dpcvaend);

	writel((u32)(evsw + evbp + eavheight + evfp - 1),
	       &pregister->dpcevtotal);

	writel((u32)(evsw - 1), &pregister->dpcevswidth);

	writel((u32)(evsw + evbp - 1), &pregister->dpcevastart);

	writel((u32)(evsw + evbp + eavheight - 1), &pregister->dpcevaend);
	temp = pregister->dpcctrl0;
	temp &= ~intpend;
	if (binvvsync)
		temp |= (u32)polvsync;
	else
		temp &= (u32)~polvsync;

	writel(temp, &pregister->dpcctrl0);
}

void nx_dpc_get_vsync(u32 module_index, u32 *pavheight, u32 *pvsw, u32 *pvfp,
		      u32 *pvbp, int *pbinvvsync, u32 *peavheight,
		      u32 *pevsw, u32 *pevfp, u32 *pevbp)
{
	const u32 polvsync = 1u << 1;
	u32 vtotal, vsw, vab, vae;
	u32 avh, vfp, vbp;
	register struct nx_dpc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	vtotal = (u32)pregister->dpcvtotal + 1;
	vsw = (u32)pregister->dpcvswidth + 1;
	vab = (u32)pregister->dpcvastart + 1;
	vae = (u32)pregister->dpcvaend + 1;
	vbp = vab - vsw;
	avh = vae - vab;
	vfp = vtotal - vae;
	if (pavheight)
		*pavheight = avh;
	if (pvsw)
		*pvsw = vsw;
	if (pvfp)
		*pvfp = vfp;
	if (pvbp)
		*pvbp = vbp;
	vtotal = (u32)pregister->dpcevtotal + 1;
	vsw = (u32)pregister->dpcevswidth + 1;
	vab = (u32)pregister->dpcevastart + 1;
	vae = (u32)pregister->dpcevaend + 1;
	vbp = vab - vsw;
	avh = vae - vab;
	vfp = vtotal - vae;
	if (peavheight)
		*peavheight = avh;
	if (pevsw)
		*pevsw = vsw;
	if (pevfp)
		*pevfp = vfp;
	if (pevbp)
		*pevbp = vbp;
	if (pbinvvsync)
		*pbinvvsync = (pregister->dpcctrl0 & polvsync) ? 1 : 0;
}

void nx_dpc_set_vsync_offset(u32 module_index, u32 vssoffset, u32 vseoffset,
			     u32 evssoffset, u32 evseoffset)
{
	register struct nx_dpc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;

	writel((u32)vseoffset, &pregister->dpcvseoffset);

	writel((u32)vssoffset, &pregister->dpcvssoffset);

	writel((u32)evseoffset, &pregister->dpcevseoffset);

	writel((u32)evssoffset, &pregister->dpcevssoffset);
}

void nx_dpc_get_vsync_offset(u32 module_index, u32 *pvssoffset,
			     u32 *pvseoffset, u32 *pevssoffset,
			     u32 *pevseoffset)
{
	register struct nx_dpc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;

	if (pvseoffset)
		*pvseoffset = (u32)pregister->dpcvseoffset;

	if (pvssoffset)
		*pvssoffset = (u32)pregister->dpcvssoffset;

	if (pevseoffset)
		*pevseoffset = (u32)pregister->dpcevseoffset;

	if (pevssoffset)
		*pevssoffset = (u32)pregister->dpcevssoffset;
}

void nx_dpc_set_horizontal_up_scaler(u32 module_index, int benb,
				     u32 sourcewidth, u32 destwidth)
{
	const u32 upscalel_pos = 8;
	const u32 upscaleh_pos = 0;
	const u32 upscaleh_mask = ((1 << 15) - 1) << upscaleh_pos;
	const u32 upscalerenb_pos = 0;
	register struct nx_dpc_register_set *pregister;
	register u32 regvalue;
	register u32 up_scale;

	pregister = __g_module_variables[module_index].pregister;
	up_scale = ((sourcewidth - 1) * (1 << 11)) / (destwidth - 1);
	regvalue = 0;
	regvalue |= (((u32)benb << upscalerenb_pos) |
		     (up_scale & 0xff) << upscalel_pos);

	writel(regvalue, &pregister->dpcupscalecon0);

	writel((up_scale >> 0x08) & upscaleh_mask, &pregister->dpcupscalecon1);

	writel(sourcewidth - 1, &pregister->dpcupscalecon2);
}

void nx_dpc_get_horizontal_up_scaler(u32 module_index, int *pbenb,
				     u32 *psourcewidth, u32 *pdestwidth)
{
	const u32 upscalerenb_pos = 0;
	const u32 upscalerenb_mask = 1u << upscalerenb_pos;
	register struct nx_dpc_register_set *pregister;

	u32 up_scale;
	u32 destwidth, srcwidth;

	pregister = __g_module_variables[module_index].pregister;
	up_scale = ((u32)(pregister->dpcupscalecon1 & 0x7fff) << 8) |
	    ((u32)(pregister->dpcupscalecon0 >> 8) & 0xff);
	srcwidth = pregister->dpcupscalecon2;
	destwidth = (srcwidth * (1 << 11)) / up_scale;
	if (pbenb)
		*pbenb = (pregister->dpcupscalecon0 & upscalerenb_mask);
	if (psourcewidth)
		*psourcewidth = srcwidth + 1;
	if (pdestwidth)
		*pdestwidth = destwidth + 1;
}

void nx_dpc_set_sync(u32 module_index, enum syncgenmode sync_gen_mode,
		     u32 avwidth, u32 avheight, u32 hsw, u32 hfp, u32 hbp,
		     u32 vsw, u32 vfp, u32 vbp, enum polarity field_polarity,
		     enum polarity hsyncpolarity, enum polarity vsyncpolarity,
		     u32 even_vsw, u32 even_vfp, u32 even_vbp, u32 vsetpixel,
		     u32 vsclrpixel, u32 evenvsetpixel, u32 evenvsclrpixel)
{
	register struct nx_dpc_register_set *pregister;
	u32 regvalue = 0;

	pregister = __g_module_variables[module_index].pregister;

	writel((u32)(hfp + hsw + hbp + avwidth - 1), &pregister->dpchtotal);
	writel((u32)(hsw - 1), &pregister->dpchswidth);
	writel((u32)(hsw + hbp - 1), &pregister->dpchastart);
	writel((u32)(hsw + hbp + avwidth - 1), &pregister->dpchaend);
	writel((u32)(vfp + vsw + vbp + avheight - 1), &pregister->dpcvtotal);
	writel((u32)(vsw - 1), &pregister->dpcvswidth);
	writel((u32)(vsw + vbp - 1), &pregister->dpcvastart);
	writel((u32)(vsw + vbp + avheight - 1), &pregister->dpcvaend);
	writel((u32)vsetpixel, &pregister->dpcvseoffset);
	writel((u32)(hfp + hsw + hbp + avwidth - vsclrpixel - 1),
	       &pregister->dpcvssoffset);
	writel((u32)evenvsetpixel, &pregister->dpcevseoffset);
	writel((u32)(hfp + hsw + hbp + avwidth - evenvsclrpixel - 1),
	       &pregister->dpcevssoffset);
	if (sync_gen_mode == 1) {
		writel((u32)(even_vfp + even_vsw + even_vbp + avheight - 1),
		       &pregister->dpcevtotal);
		writel((u32)(even_vsw - 1), &pregister->dpcevswidth);
		writel((u32)(even_vsw + even_vbp - 1),
		       &pregister->dpcevastart);
		writel((u32)(even_vsw + even_vbp + avheight - 1),
		       &pregister->dpcevaend);
	}
	regvalue = readl(&pregister->dpcctrl0) & 0xfff0ul;
	regvalue |= (((u32)field_polarity << 2) | ((u32)vsyncpolarity << 1) |
		     ((u32)hsyncpolarity << 0));
	writel((u32)regvalue, &pregister->dpcctrl0);
}

void nx_dpc_set_output_format(u32 module_index, enum outputformat output_format,
			      u8 output_video_config)
{
	const u32 format_table[] = {
		(0 << 0), (1 << 0), (2 << 0), (3 << 0), (4 << 0), (5 << 0),
		(6 << 0), (7 << 0), (8 << 0), (9 << 0), (0 << 0) | (1 << 7),
		(1 << 0) | (1 << 7), (2 << 0) | (1 << 7), (3 << 0) | (1 << 7),
		(4 << 0) | (1 << 7), (5 << 0) | (1 << 7), (6 << 0) | (1 << 7),
		(7 << 0) | (1 << 7), (8 << 0) | (1 << 7), (9 << 0) | (1 << 7),
		(10 << 0), (11 << 0), (12 << 0), (13 << 0), (14 << 0), (15 << 0)
	};
	u32 regvalue;
	u32 regvalue0;
	register struct nx_dpc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	regvalue = readl(&pregister->dpcctrl1) & 0x30fful;

	regvalue |= (format_table[output_format] << 8);
	writel((u32)regvalue, &pregister->dpcctrl1);
	regvalue0 = (u32)(readl(&pregister->dpcctrl1) & 0xff3f);
	regvalue0 = (u32)((output_video_config << 6) | regvalue0);
	writel((u32)regvalue0, &pregister->dpcctrl1);
}

void nx_dpc_set_quantization_mode(u32 module_index, enum qmode rgb2yc,
				  enum qmode yc2rgb)
{
	register struct nx_dpc_register_set *pregister;
	u32 regvalue;

	pregister = __g_module_variables[module_index].pregister;
	regvalue = readl(&pregister->dpcctrl1) & 0x8ffful;
	regvalue |= ((u32)rgb2yc << 13) | ((u32)yc2rgb << 12);
	writel((u32)regvalue, &pregister->dpcctrl1);
}

void nx_dpc_set_enable(u32 module_index, int enable, int rgbmode,
		       int use_ntscsync, int use_analog_output, int seavenable)
{
	u32 regvalue;
	register struct nx_dpc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	regvalue = readl(&pregister->dpcctrl0) & 0x0efful;
	regvalue |= ((u32)enable << 15) | ((u32)use_ntscsync << 14) |
	    ((u32)seavenable << 8) | ((u32)use_analog_output << 13) |
	    ((u32)rgbmode << 12);
	writel((u32)regvalue, &pregister->dpcctrl0);
}

void nx_dpc_set_out_video_clk_select(u32 module_index,
				     enum outpadclksel out_pad_vclk_sel)
{
	register struct nx_dpc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;

	writel((u32)((readl(&pregister->dpcctrl2)) | (out_pad_vclk_sel & 0x3)),
	       &pregister->dpcctrl2);
}

void nx_dpc_set_reg_flush(u32 module_index)
{
	u32 reg;
	register struct nx_dpc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	reg = readl(&pregister->dpcdataflush);
	writel((u32)(reg | (1ul << 4)), &pregister->dpcdataflush);
}

void nx_dpc_set_sramon(u32 module_index)
{
	u32 reg;
	register struct nx_dpc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	reg = (u32)(readl(&pregister->dpcctrl2) & 0xf3ff);
	writel((u32)(reg | (1ul << 10)), &pregister->dpcctrl2);
	reg = (u32)(readl(&pregister->dpcctrl2) & 0xf7ff);
	writel((u32)(reg | (1ul << 11)), &pregister->dpcctrl2);
}

void nx_dpc_set_sync_lcdtype(u32 module_index, int stnlcd, int dual_view_enb,
			     int bit_widh, u8 cpcycle)
{
	u32 reg;
	register struct nx_dpc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;

	reg = (u32)(readl(&pregister->dpcctrl2) & 0xc0f);
	writel((u32)(reg | (cpcycle << 12) | (bit_widh << 9) |
		      (dual_view_enb << 8) | (stnlcd << 7)),
	       &pregister->dpcctrl2);
}

void nx_dpc_set_up_scale_control(u32 module_index, int up_scale_enb,
				 int filter_enb, u32 hscale, u16 source_width)
{
	register struct nx_dpc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	writel((u32)((hscale << 8) | ((u32)filter_enb << 1) | (up_scale_enb)),
	       &pregister->dpcupscalecon0);
	writel((u32)(hscale >> 8), &pregister->dpcupscalecon1);
	writel(source_width, &pregister->dpcupscalecon2);
}

void nx_dpc_set_mputime(u32 module_index, u8 setup, u8 hold, u8 acc)
{
	register struct nx_dpc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	writel((u32)((setup << 8) | (hold & 0xff)), &pregister->dpcmputime0);
	writel((u32)(acc), &pregister->dpcmputime1);
}

void nx_dpc_set_index(u32 module_index, u32 index)
{
	u32 regvalue;
	register struct nx_dpc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	writel((u32)(index & 0xffff), &pregister->dpcmpuwrdatal);
	writel((u32)((index >> 16) & 0xff), &pregister->dpcmpuindex);
	if (index == 0x22) {
		regvalue = readl(&pregister->dpcctrl2);
		writel((regvalue | 0x10), &pregister->dpcctrl2);
	}
}

void nx_dpc_set_data(u32 module_index, u32 data)
{
	register struct nx_dpc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	writel((u32)(data & 0xffff), &pregister->dpcmpuwrdatal);
	writel((u32)((data >> 16) & 0xff), &pregister->dpcmpudatah);
}

void nx_dpc_set_cmd_buffer_flush(u32 module_index)
{
	u32 reg;
	register struct nx_dpc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	reg = readl(&pregister->dpcdataflush);
	writel((u32)(reg | (1 << 1)), &pregister->dpcdataflush);
}

void nx_dpc_set_cmd_buffer_clear(u32 module_index)
{
	u32 reg;
	register struct nx_dpc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	reg = readl(&pregister->dpcdataflush);
	writel((u32)(reg | (1 << 0)), &pregister->dpcdataflush);
}

void nx_dpc_set_cmd_buffer_write(u32 module_index, u32 cmd_data)
{
	register struct nx_dpc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	writel((u32)(cmd_data & 0xffff), &pregister->dpccmdbufferdatal);
	writel((u32)(cmd_data >> 16), &pregister->dpccmdbufferdatah);
}

void nx_dpc_set(u32 module_index)
{
	u32 reg;
	register struct nx_dpc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	reg = readl(&pregister->dpcpolctrl);
	writel((u32)(reg | 0x1), &pregister->dpcpolctrl);
}

u32 nx_dpc_get_data(u32 module_index)
{
	u32 reg = 0;
	register struct nx_dpc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	reg = readl(&pregister->dpcmpudatah);
	reg = (reg << 16) | readl(&pregister->dpcmpurdatal);
	return reg;
}

u32 nx_dpc_get_status(u32 module_index)
{
	u32 reg = 0;
	register struct nx_dpc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	reg = readl(&pregister->dpcmpustatus);
	reg = (reg << 16) | readl(&pregister->dpcmpurdatal);
	return reg;
}

void nx_dpc_rgbmask(u32 module_index, u32 rgbmask)
{
	register struct nx_dpc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	writel((rgbmask >> 0) & 0xffff, &pregister->dpcrgbmask[0]);
	writel((rgbmask >> 16) & 0x00ff, &pregister->dpcrgbmask[1]);
}

void nx_dpc_set_pad_location(u32 module_index, u32 index, u32 regvalue)
{
	register struct nx_dpc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	writel(regvalue, &pregister->dpcpadposition[index]);
}

u32 nx_dpc_get_field_flag(u32 module_index)
{
	register struct nx_dpc_register_set *pregister;
	u32 regvalue;

	pregister = __g_module_variables[module_index].pregister;
	regvalue = readl(&pregister->dpcrgbshift);

	return (u32)((regvalue >> 5) & 0x01);
}

void nx_dpc_set_enable_with_interlace(u32 module_index, int enable, int rgbmode,
				      int use_ntscsync, int use_analog_output,
				      int seavenable)
{
	u32 regvalue;
	register struct nx_dpc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	regvalue = readl(&pregister->dpcctrl0) & 0x0eff;
	regvalue = readl(&pregister->dpcctrl0) & 0x0eff;
	regvalue |= ((u32)enable << 15) | ((u32)use_ntscsync << 14) |
	    ((u32)seavenable << 8) | ((u32)use_analog_output << 13) |
	    ((u32)rgbmode << 12);

	regvalue |= (1 << 9);
	writel((u16)regvalue, &pregister->dpcctrl0);
}

void nx_dpc_set_encoder_control_reg(u32 module_index, u32 param_a, u32 param_b,
				    u32 param_c)
{
	register struct nx_dpc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	writel(param_a, &pregister->ntsc_ecmda);
	writel(param_b, &pregister->ntsc_ecmdb);
	writel(param_c, &pregister->ntsc_ecmdc);
}

void nx_dpc_set_encoder_shcphase_control(u32 module_index, u32 chroma_param)
{
	register struct nx_dpc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	writel(chroma_param, &pregister->ntsc_sch);
}

void nx_dpc_set_encoder_timing_config_reg(u32 module_index, u32 icntl)
{
	register struct nx_dpc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	writel(icntl, &pregister->ntsc_icntl);
}

void nx_dpc_set_encoder_dacoutput_select(u32 module_index, u8 dacsel0,
					 u8 dacsel1, u8 dacsel2, u8 dacsel3,
					 u8 dacsel4, u8 dacsel5)
{
	register struct nx_dpc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	writel(((dacsel1 & 0xf) << 4) | (dacsel0 & 0xf),
	       &pregister->ntsc_dacsel10);
	writel(((dacsel3 & 0xf) << 4) | (dacsel2 & 0xf),
	       &pregister->ntsc_dacsel32);
	writel(((dacsel5 & 0xf) << 4) | (dacsel4 & 0xf),
	       &pregister->ntsc_dacsel54);
}

void nx_dpc_set_encoder_sync_location(u32 module_index, u16 hsoe, u16 hsob,
				      u16 vsob, u16 vsoe, u8 vsost, int novrst)
{
	register struct nx_dpc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	writel((u16)((((vsob & 0x100) >> 2) | ((hsob & 0x700) >> 5) |
		       (hsoe & 0x700) >> 8)), &pregister->ntsc_hsvso);
	writel((u16)(hsoe & 0xff), &pregister->ntsc_hsoe);
	writel((u16)(hsob & 0xff), &pregister->ntsc_hsob);
	writel((u16)(vsob & 0xff), &pregister->ntsc_vsob);
	writel((u16)(((vsost & 0x3) << 6) | (novrst << 5) | (vsoe & 0x1f)),
	       &pregister->ntsc_vsoe);
}

void nx_dpc_set_encoder_dacpower_enable(u32 module_index, u8 dacpd)
{
	register struct nx_dpc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	writel(dacpd, &pregister->ntsc_dacpd);
}

void nx_dpc_set_ycorder(u32 module_index, enum nx_dpc_ycorder ycorder)
{
	const u16 ycorder_pos = 6;
	register struct nx_dpc_register_set *pregister;
	u32 temp;

	pregister = __g_module_variables[module_index].pregister;
	temp = pregister->dpcctrl1 & (~(0xf << ycorder_pos));
	temp = (u16)(temp | (ycorder << ycorder_pos));
	writel(temp, &pregister->dpcctrl1);
}

void nx_dpc_set_luma_gain(u32 module_index, u32 luma_gain)
{
	register struct nx_dpc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	writel(luma_gain, &pregister->ntsc_cont);
}

void nx_dpc_set_encenable(u32 module_index, int benb)
{
	const u16 encmode = 1u << 14;
	const u16 encrst = 1u << 13;
	const u16 intpend = 1u << 10;
	register struct nx_dpc_register_set *pregister;
	register u16 temp;

	pregister = __g_module_variables[module_index].pregister;
	temp = readl(&pregister->dpcctrl0);
	temp &= (u16)~intpend;
	if (benb)
		temp |= (u16)encrst;
	else
		temp &= (u16)~encrst;
	writel((temp | encmode), &pregister->dpcctrl0);
	writel(7, &pregister->ntsc_icntl);
}

int nx_dpc_get_encenable(u32 module_index)
{
	const u16 encrst = 1u << 13;
	register struct nx_dpc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	return (readl(&pregister->dpcctrl0) & encrst) ? 1 : 0;
}

void nx_dpc_set_video_encoder_power_down(u32 module_index, int benb)
{
	const u16 pwdenc = 1u << 7;
	register struct nx_dpc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	if (benb) {
		writel(readl(&pregister->ntsc_ecmda) | (u16)pwdenc,
		       &pregister->ntsc_ecmda);
		writel(0, &pregister->ntsc_dacsel10);
	} else {
		writel(1, &pregister->ntsc_dacsel10);
		writel(readl(&pregister->ntsc_ecmda) & (u16)~pwdenc,
		       &pregister->ntsc_ecmda);
	}
}

int nx_dpc_get_video_encoder_power_down(u32 module_index)
{
	const u16 pwdenc = 1u << 7;
	register struct nx_dpc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	return (readl(&pregister->ntsc_ecmda) & pwdenc) ? 1 : 0;
}

void nx_dpc_set_video_encoder_mode(u32 module_index, enum nx_dpc_vbs vbs,
				   int bpedestal)
{
	register struct nx_dpc_register_set *pregister;

#define phalt (1u << 0)
#define ifmt (1u << 1)
#define ped (1u << 3)
#define fscsel_ntsc (0u << 4)
#define fscsel_pal (1u << 4)
#define fscsel_palm (2u << 4)
#define fscsel_paln (3u << 4)
#define fdrst (1u << 6)
#define pwdenc (1u << 7)
	register u16 temp;
	static const u8 ntsc_ecmda_table[] = {
		(u8)(fscsel_ntsc | fdrst), (u8)(ifmt | fscsel_ntsc),
		(u8)(fscsel_pal), (u8)(fscsel_palm | phalt),
		(u8)(ifmt | fscsel_paln | phalt),
		(u8)(ifmt | fscsel_pal | phalt | fdrst),
		(u8)(fscsel_pal | phalt),
		(u8)(ifmt | fscsel_ntsc)
	};
	pregister = __g_module_variables[module_index].pregister;
	temp = readl(&pregister->ntsc_ecmda);
	temp &= (u16)pwdenc;
	temp = (u16)(temp | (u16)ntsc_ecmda_table[vbs]);
	if (bpedestal)
		temp |= (u16)ped;
	writel(temp, &pregister->ntsc_ecmda);
#undef phalt
#undef ifmt
#undef ped
#undef fscsel_ntsc
#undef fscsel_pal
#undef fscsel_palm
#undef fscsel_paln
#undef fdrst
#undef pwdenc
}

void nx_dpc_set_video_encoder_schlock_control(u32 module_index, int bfreerun)
{
	const u16 fdrst = 1u << 6;
	register struct nx_dpc_register_set *pregister;
	register u16 temp;

	pregister = __g_module_variables[module_index].pregister;
	temp = readl(&pregister->ntsc_ecmda);
	if (bfreerun)
		temp |= (u16)fdrst;
	else
		temp &= (u16)~fdrst;
	writel(temp, &pregister->ntsc_ecmda);
}

int nx_dpc_get_video_encoder_schlock_control(u32 module_index)
{
	const u16 fdrst = 1u << 6;
	register struct nx_dpc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	return (readl(&pregister->ntsc_ecmda) & fdrst) ? 1 : 0;
}

void nx_dpc_set_video_encoder_bandwidth(u32 module_index,
					enum nx_dpc_bandwidth luma,
					enum nx_dpc_bandwidth chroma)
{
	const u16 ybw_pos = 0;
	const u16 cbw_pos = 2;
	register struct nx_dpc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	writel((u16)((chroma << cbw_pos) | (luma << ybw_pos)),
	       &pregister->ntsc_ecmdb);
}

void nx_dpc_get_video_encoder_bandwidth(u32 module_index,
					enum nx_dpc_bandwidth *pluma,
					enum nx_dpc_bandwidth *pchroma)
{
	const u16 ybw_pos = 0;
	const u16 ybw_mask = 3u << ybw_pos;
	const u16 cbw_pos = 2;
	const u16 cbw_mask = 3u << cbw_pos;
	register struct nx_dpc_register_set *pregister;
	register u16 temp;

	pregister = __g_module_variables[module_index].pregister;
	temp = readl(&pregister->ntsc_ecmdb);
	if (pluma)
		*pluma = (enum nx_dpc_bandwidth)((temp & ybw_mask) >> ybw_pos);
	if (pchroma)
		*pchroma =
		    (enum nx_dpc_bandwidth)((temp & cbw_mask) >> cbw_pos);
}

void nx_dpc_set_video_encoder_color_control(u32 module_index, s8 sch,
					    s8 hue, s8 sat, s8 crt,
					    s8 brt)
{
	register struct nx_dpc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	writel((u16)sch, &pregister->ntsc_sch);
	writel((u16)hue, &pregister->ntsc_hue);
	writel((u16)sat, &pregister->ntsc_sat);
	writel((u16)crt, &pregister->ntsc_cont);
	writel((u16)brt, &pregister->ntsc_bright);
}

void nx_dpc_get_video_encoder_color_control(u32 module_index, s8 *psch,
					    s8 *phue, s8 *psat,
					    s8 *pcrt, s8 *pbrt)
{
	register struct nx_dpc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	if (psch)
		*psch = (s8)readl(&pregister->ntsc_sch);
	if (phue)
		*phue = (s8)readl(&pregister->ntsc_hue);
	if (psat)
		*psat = (s8)readl(&pregister->ntsc_sat);
	if (pcrt)
		*pcrt = (s8)readl(&pregister->ntsc_cont);
	if (pbrt)
		*pbrt = (s8)readl(&pregister->ntsc_bright);
}

void nx_dpc_set_video_encoder_fscadjust(u32 module_index, int16_t adjust)
{
	register struct nx_dpc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	writel((u16)(adjust >> 8), &pregister->ntsc_fsc_adjh);
	writel((u16)(adjust & 0xff), &pregister->ntsc_fsc_adjl);
}

u16 nx_dpc_get_video_encoder_fscadjust(u32 module_index)
{
	register u32 temp;
	register struct nx_dpc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	temp = (u32)readl(&pregister->ntsc_fsc_adjh);
	temp <<= 8;
	temp |= (((u32)readl(&pregister->ntsc_fsc_adjl)) & 0xff);
	return (u16)temp;
}

void nx_dpc_set_video_encoder_timing(u32 module_index, u32 hsos, u32 hsoe,
				     u32 vsos, u32 vsoe)
{
	register struct nx_dpc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	hsos -= 1;
	hsoe -= 1;
	writel((u16)((((vsos >> 8) & 1u) << 6) | (((hsos >> 8) & 7u) << 3) |
		      (((hsoe >> 8) & 7u) << 0)), &pregister->ntsc_hsvso);
	writel((u16)(hsos & 0xffu), &pregister->ntsc_hsob);
	writel((u16)(hsoe & 0xffu), &pregister->ntsc_hsoe);
	writel((u16)(vsos & 0xffu), &pregister->ntsc_vsob);
	writel((u16)(vsoe & 0x1fu), &pregister->ntsc_vsoe);
}

void nx_dpc_get_video_encoder_timing(u32 module_index, u32 *phsos, u32 *phsoe,
				     u32 *pvsos, u32 *pvsoe)
{
	register u16 hsvso;
	register struct nx_dpc_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	hsvso = readl(&pregister->ntsc_hsvso);
	if (phsos)
		*phsos = (u32)((((hsvso >> 3) & 7u) << 8) |
				(readl(&pregister->ntsc_hsob) & 0xffu)) + 1;
	if (phsoe)
		*phsoe = (u32)((((hsvso >> 0) & 7u) << 8) |
				(readl(&pregister->ntsc_hsoe) & 0xffu)) + 1;
	if (pvsos)
		*pvsos = (u32)((((hsvso >> 6) & 1u) << 8) |
				(readl(&pregister->ntsc_vsob) & 0xffu));
	if (pvsoe)
		*pvsoe = (u32)(readl(&pregister->ntsc_vsoe) & 0x1fu);
}
