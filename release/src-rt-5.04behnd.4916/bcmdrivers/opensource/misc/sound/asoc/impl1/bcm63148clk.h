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
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 * 
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 * 
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
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