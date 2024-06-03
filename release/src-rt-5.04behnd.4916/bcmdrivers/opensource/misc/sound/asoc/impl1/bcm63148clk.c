/*********************************************************************
 * bcm63148clk.c -- ALSA SoC clock driver for 63138/63148 platform 
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

#include <linux/regmap.h>
#include <sound/pcm_params.h>
#include "bcm63148clk.h"
#include "bcm63xx-i2s.h"


extern struct regmap *regmap_i2s;

struct i2s_sysclk_freq_map freq_map[MAX_PCM_FORMAT][MAX_SAMPLE_RATE] =
{
	{},	{},
	{  /* for S16_LE */
   { 44100  ,    16 ,     I2S_CLK_25MHZ }, /*  force to 48K, hardware support issue.  */
   { 48000  ,    16 ,     I2S_CLK_25MHZ }, 
   { 96000  ,    8 ,      I2S_CLK_25MHZ }, 
   { 192000 ,    4 ,      I2S_CLK_25MHZ }, 
   { 384000 ,    2 ,      I2S_CLK_25MHZ }, 
   { 0      ,    0 ,      0             },		
	},
	{},	{},	{},	{},	{},	{},	{},
	{  /* for S32_LE */
   { 16000  ,    12,      I2S_CLK_25MHZ }, 
   { 32000  ,    12,      I2S_CLK_50MHZ }, /*  Req Bclk: 2,048,000  , Actual Bclk: 2,083,333  */
   { 44100  ,    9 ,      I2S_CLK_50MHZ }, /*  Req Bclk: 2,822,400  , Actual Bclk: 2,777,778  */
   { 48000  ,    8 ,      I2S_CLK_50MHZ }, /*  Req Bclk: 3,072,000  , Actual Bclk: 3,125,000  */
   { 96000  ,    4 ,      I2S_CLK_50MHZ }, /*  Req Bclk: 6,144,000  , Actual Bclk: 6,250,000  */
   { 192000 ,    2 ,      I2S_CLK_50MHZ }, /*  Req Bclk: 12,288,000 , Actual Bclk: 12,500,000 */
   { 384000 ,    1 ,      I2S_CLK_50MHZ }, /*  Req Bclk: 24,576,000 , Actual Bclk: 25,000,000 */
   { 0      ,    0 ,      0             } 
  }
};

struct i2s_sysclk_freq_map * get_sys_freq_map(int fmtid, int frequency )
{
   int i;
   struct i2s_sysclk_freq_map * freq_map_ptr = NULL;
   
   if( fmtid >= MAX_PCM_FORMAT )
   {
   	  return freq_map_ptr;
   }
   	  
   for( i=0; freq_map[fmtid][i].freq; i++ )
   {
      if( frequency == freq_map[fmtid][i].freq )
      {
         freq_map_ptr = &freq_map[fmtid][i];        
         break;
      }
   }
      
   return freq_map_ptr;         
}

int set_i2s_clk(int fmtid,int freq)
{
   struct i2s_sysclk_freq_map * freq_map_ptr = NULL;

   /* Based on sampling frequency, choose MCLK and *
    * select divide ratio for required BCLK        */
   freq_map_ptr = get_sys_freq_map( fmtid,freq );
   
   if( freq_map_ptr )
   {
      regmap_update_bits(regmap_i2s, I2S_TX_CFG, I2S_MCLK_CLKSEL_CLR_MASK, 0 );
      regmap_update_bits(regmap_i2s, I2S_TX_CFG, I2S_MCLK_CLKSEL_CLR_MASK,
                                     freq_map_ptr->mclk_rate << I2S_MCLK_RATE_SHIFT | 
                                     I2S_CLK_PLL  << I2S_CLKSEL_SHIFT);

      unsigned int tmp1,tmp2,val;
      regmap_read(regmap_i2s, I2S_TX_CFG, &val);
      tmp1 = tmp2 = val;
      if( (( tmp1 >> I2S_CLKSEL_SHIFT    & 0x03 ) == freq_map_ptr->clk_sel)  &&
      	  (( tmp2 >> I2S_MCLK_RATE_SHIFT & 0x0F ) == freq_map_ptr->mclk_rate)   )
      {  
        return(0);
      }
      else
      {
      	return (-EINVAL);
      }
   }

   return (-EINVAL);
}