/*********************************************************************
 * bcm63148clk.h -- ALSA SoC clock driver header file for 63138/63148 platform 
 *
 * Author: Kevin Li <kevin-ke.li@broadcom.com>
 * 
 * Copyright (c) 2018 Broadcom Corporation
 * All Rights Reserved
 *
 * <:label-BRCM:2018:DUAL/GPL:standard
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as published by
 * the Free Software Foundation (the "GPL").
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * 
 * A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
 * writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 * 
 * :>
*********************************************************************/
#ifndef __BCM63148CLK_H
#define __BCM63148CLK_H

#define MAX_PCM_FORMAT SNDRV_PCM_FORMAT_S32_LE+1
#define MAX_SAMPLE_RATE 8

struct i2s_sysclk_freq_map
{
   int freq;                        /* Desired sampling frequency */
   unsigned int mclk_rate;          /* mclk/2*bclk = mclk_rate */
   unsigned int clk_sel;            /* The mclk frequency */
};

int set_i2s_clk(int fmtid,int freq);
struct i2s_sysclk_freq_map * get_sys_freq_map(int fmtid, int frequency );


#endif