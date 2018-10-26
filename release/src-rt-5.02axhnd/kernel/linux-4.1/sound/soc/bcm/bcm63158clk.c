/*********************************************************************
 * bcm63168clk.c -- ALSA SoC clock driver for 63158 platform 
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
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/io.h>
#include <sound/soc.h>
#include <sound/pcm_params.h>
#include "bcm63xx-i2s.h"
#include "bcm63158clk.h"

extern struct regmap *regmap_i2s;
struct regmap        *regmap_ntr_rst;
struct regmap        *regmap_ntr_clk;

#define DRV_NAME "xpon_clock"

struct i2s_pllclk_freq_map pll_i2s_freq_map[MAX_PCM_FORMAT][MAX_SAMPLE_RATE]=
{
	
	{},	{},
	{  /* for S16_LE */
    /*  fs       mclk_rate     div      denom         sel                */  /*  req clk     act clk  */
    {   8000,       16,      1999969,   65535,   CLK_SEL_250MHZ_SYNCE_PLL }, /*  512000,     51119.87  */     
    {  11025,       16,      1451225,   65535,   CLK_SEL_250MHZ_SYNCE_PLL }, /*  705600,    705599.52  */     
    {  16000,       16,       999985,   65535,   CLK_SEL_250MHZ_SYNCE_PLL }, /* 1024000,   1024000.76  */     
    {  22050,       16,       725613,   65535,   CLK_SEL_250MHZ_SYNCE_PLL }, /* 1411200,   1411200.99  */     
    {  32000,       16,       499992,   65535,   CLK_SEL_250MHZ_SYNCE_PLL }, /* 2048000,   2048001.56  */     
    {  44100,       8,        725613,   65535,   CLK_SEL_250MHZ_SYNCE_PLL }, /* 2822400,   2822401.99  */
    {  48000,       8,        666656,   65535,   CLK_SEL_250MHZ_SYNCE_PLL }, /* 3072000,   3072002.28  */      
    {  96000,       4,        666656,   65535,   CLK_SEL_250MHZ_SYNCE_PLL }, /* 6144000,   6144004.55  */     
    { 192000,       2,        666656,   65535,   CLK_SEL_250MHZ_SYNCE_PLL }, /* 12288000, 12288009.10 */     
    { 384000,       1,        666656,   65535,   CLK_SEL_250MHZ_SYNCE_PLL }, /* 24576000, 24576018.20 */ 
    {      0,       0,            0,       0,                          0 },

	},
	{},	{},	{},	{},	{},	{},	{},
	{
    /*  fs       mclk_rate     div      denom         sel                */  /*  req clk     act clk  */
    {   8000,       16,      999985,   65535,   CLK_SEL_250MHZ_SYNCE_PLL }, /*  512000,     51119.87  */     
    {  11025,       16,      725613,   65535,   CLK_SEL_250MHZ_SYNCE_PLL }, /*  705600,    705599.52  */     
    {  16000,       16,      499992,   65535,   CLK_SEL_250MHZ_SYNCE_PLL }, /* 1024000,   1024000.76  */     
    {  22050,       16,      362806,   65535,   CLK_SEL_250MHZ_SYNCE_PLL }, /* 1411200,   1411200.99  */     
    {  32000,       8,       499992,   65535,   CLK_SEL_250MHZ_SYNCE_PLL }, /* 2048000,   2048001.56  */     
    {  44100,       8,       362806,   65535,   CLK_SEL_250MHZ_SYNCE_PLL }, /* 2822400,   2822401.99  */
    {  48000,       8,       333328,   65535,   CLK_SEL_250MHZ_SYNCE_PLL }, /* 3072000,   3072002.28  */      
    {  96000,       4,       333328,   65535,   CLK_SEL_250MHZ_SYNCE_PLL }, /* 6144000,   6144004.55  */     
    { 192000,       2,       333328 ,  65535,   CLK_SEL_250MHZ_SYNCE_PLL }, /* 12288000, 12288009.10 */     
    { 384000,       1,       333328,   65535,   CLK_SEL_250MHZ_SYNCE_PLL }, /* 24576000, 24576018.20 */
    {      0,       0,            0,       0,                          0 }, 
  }
	
};

struct i2s_pllclk_freq_map * get_i2s_pll_freq_map( int fmtid, unsigned int frequency )
{
   int i;
   struct i2s_pllclk_freq_map * freq_map_ptr = NULL;

   for( i=0; pll_i2s_freq_map[fmtid][i].freq; i++ )
   {
      if( frequency == pll_i2s_freq_map[fmtid][i].freq )
      {
         freq_map_ptr = &pll_i2s_freq_map[fmtid][i];        
         break;
      }
   }

   /* Set the init function */
   if ( !freq_map_ptr )
   {
      printk(KERN_ERR"%s: Cannot find PLL mapping for freq:%d \n", __FUNCTION__, (int)frequency);
   }
     
   return freq_map_ptr;         
}

int set_i2s_prg_clk_div( struct i2s_pllclk_freq_map * freq_map_ptr )
{
   /* Reset prg clk divider*/
   regmap_update_bits(regmap_ntr_rst, NTR_REET_REG, CLK_DIV_RST_N, 0 );  // Low
   regmap_update_bits(regmap_ntr_rst, NTR_REET_REG, CLK_DIV_RST_N, CLK_DIV_RST_N ); //High 

   /* Program div */
   regmap_update_bits(regmap_ntr_clk, NTR_CLK_I2S_PRGDIVCFG1, 
                                      NTR_CLK_SEL_MASK | CLK_DIV_MASK, 
                                      (freq_map_ptr->prg_clk_sel << CLK_SEL_SHIFT) |
                                      (freq_map_ptr->prg_clk_div & CLK_DIV_MASK) );


   /* Program denom */
   regmap_update_bits(regmap_ntr_clk, NTR_CLK_I2S_PRGDIVCFG2, 
                                      CLK_DENOM_MASK, 
                                      freq_map_ptr->prg_clk_denom & CLK_DENOM_MASK );

   return 0;

}

int set_i2s_clk(int fmtid, int freq)
{
   struct i2s_pllclk_freq_map * freq_map_ptr = NULL;

   /* Based on sampling frequency, choose MCLK and *
    * select divide ratio for required BCLK        */
   freq_map_ptr = get_i2s_pll_freq_map( fmtid, freq );
   
   if( freq_map_ptr )
   {
   	  set_i2s_prg_clk_div( freq_map_ptr );
      regmap_update_bits(regmap_i2s, I2S_CFG, I2S_MCLK_CLKSEL_CLR_MASK, 
                         freq_map_ptr->mclk_rate << I2S_MCLK_RATE_SHIFT | 
                         I2S_CLK_PLL  << I2S_CLKSEL_SHIFT);
  
      return(0);
   }

   return (-EINVAL);
}

static bool brcm_ntr_rst_wr_reg(struct device *dev, unsigned int reg)
{
   switch (reg) 
   {
      case NTR_REET_REG:
        return true;
      default:
        return false;
   }
}

static bool brcm_ntr_rst_rd_reg(struct device *dev, unsigned int reg)
{
   switch (reg)
   {
      case NTR_REET_REG:
         return true;
      default:
         return false;
   }
}

static bool brcm_ntr_rst_volatile_reg(struct device *dev, unsigned int reg)
{
   switch (reg)
   {
      case NTR_REET_REG:
         return true;
      default:
         return false;
   }
}

static const struct regmap_config brcm_ntr_rst_regmap_config = {
   .reg_bits      = 32,
   .reg_stride    = 4,
   .val_bits      = 32,
   .max_register  = NTR_REET_REG_MAX,
   .writeable_reg = brcm_ntr_rst_wr_reg,
   .readable_reg  = brcm_ntr_rst_rd_reg,
   .volatile_reg  = brcm_ntr_rst_volatile_reg,
   .cache_type    = REGCACHE_FLAT,
};

static bool brcm_ntr_clk_wr_reg(struct device *dev, unsigned int reg)
{
   switch (reg) 
   {
      case NTR_CLK_I2S_PRGDIVCFG1:
	    case NTR_CLK_I2S_PRGDIVCFG2:
		     return true;
      default:
         return false;
   }
}

static bool brcm_ntr_clk_rd_reg(struct device *dev, unsigned int reg)
{
   switch (reg)
   {
      case NTR_CLK_I2S_PRGDIVCFG1:
      case NTR_CLK_I2S_PRGDIVCFG2:
         return true;
      default:
         return false;
   }
}

static bool brcm_ntr_clk_volatile_reg(struct device *dev, unsigned int reg)
{
   switch (reg)
   {
      case NTR_CLK_I2S_PRGDIVCFG1:
      case NTR_CLK_I2S_PRGDIVCFG2:
         return true;
      default:
         return false;
   }
}
static const struct regmap_config brcm_ntr_clk_regmap_config = {
   .reg_bits      = 32,
   .reg_stride    = 4,
   .val_bits      = 32,
   .max_register  = NTR_CLK_I2S_PRGDIVCFG1_MAX,
   .writeable_reg = brcm_ntr_clk_wr_reg,
   .readable_reg  = brcm_ntr_clk_rd_reg,
   .volatile_reg  = brcm_ntr_clk_volatile_reg,
   .cache_type    = REGCACHE_FLAT,
};

static int i2s_clk_probe(struct platform_device *pdev)
{
   void __iomem *regs;
   struct resource *r_mem, *region;

   r_mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
   if (!r_mem)
   {
      dev_err(&pdev->dev, "Unable to get register resource.\n");
      return -ENODEV;
   }

   region = devm_request_mem_region(&pdev->dev, r_mem->start,
                                    resource_size(r_mem), DRV_NAME);
   if (!region)
   {
      dev_err(&pdev->dev, "Memory region already claimed\n");
      return -EBUSY;
   }

   regs = devm_ioremap_nocache(&pdev->dev,r_mem->start, resource_size(r_mem));
   if ( !regs )
   {
      dev_err(&pdev->dev, "ioremap ntr reset failed\n");
      return -ENOMEM;
   }

   regmap_ntr_rst = devm_regmap_init_mmio(&pdev->dev, regs, &brcm_ntr_rst_regmap_config);//regmap
   if(!regmap_ntr_rst)
   {
      dev_err(&pdev->dev,"Failed to initialise managed register map\n");
   }

   regs = devm_ioremap_nocache(&pdev->dev,r_mem->start+NTR_CLK_PRG_SWCH_OFFSET, 
                               resource_size(r_mem)- NTR_CLK_PRG_SWCH_OFFSET);
  
   if ( !regs )
   {
      dev_err(&pdev->dev, "ioremap ntr clock program switch failed\n");
      return -ENOMEM;
   }

   regmap_ntr_clk = devm_regmap_init_mmio(&pdev->dev, regs, &brcm_ntr_clk_regmap_config); //regmap
   if(!regmap_ntr_clk)
   {
      dev_err(&pdev->dev,"Failed to initialise managed register map\n");
   }

   return 0;
}

static int i2s_clk_remove(struct platform_device *pdev)
{
   return 0;
}

#ifdef CONFIG_OF
static const struct of_device_id snd_soc_bcm_audio_match[] = 
{
	{ .compatible   = "brcm,bcm63xx-i2sclk" },
	{ }
};
#endif

static struct platform_driver i2s_clk_driver = {
   .probe     = i2s_clk_probe,
   .remove		= i2s_clk_remove,
   .driver		= 
   {
       .name	= "i2sclock",
       .of_match_table = of_match_ptr(snd_soc_bcm_audio_match),
   },
};

module_platform_driver(i2s_clk_driver);

MODULE_AUTHOR("Kevin Li <kevin-ke.li@broadcom.com>");
MODULE_DESCRIPTION("BCM I2S clock driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:" DRV_NAME);

