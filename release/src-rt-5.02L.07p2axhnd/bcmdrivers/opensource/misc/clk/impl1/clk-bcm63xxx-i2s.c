/*
<:copyright-BRCM:2018:DUAL/GPL:standard

   Copyright (c) 2018 Broadcom 
   All Rights Reserved

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

:>
*/

#include <linux/clk.h>
#include <linux/clk-provider.h>
#include <linux/mfd/syscon.h>
#include <linux/slab.h>
#include <linux/regmap.h>

#define I2S_TX_CFG                      ( 0x0000 ) /* 0x00 -- 0x2080 cfg */
#define I2S_TX_MCLK_RATIO_SHIFT         20
#define I2S_TX_MCLK_RATIO_MASK          ( 0x0F << I2S_TX_MCLK_RATIO_SHIFT )

#define I2S_TX_SCLK_1FS_DIV32_SHIFT     4 
#define I2S_TX_64BITS_PERFRAME          ( 0x02 << I2S_TX_SCLK_1FS_DIV32_SHIFT )
#define I2S_TX_SCLKS_PER_1FS_DIV32_MASK ( 0x0f << I2S_TX_SCLK_1FS_DIV32_SHIFT )

#define I2S_RX_CFG                      ( 0x0040 ) /* 0x40 -- 0x20c0 cfg*/
#define I2S_RX_MCLK_RATIO_SHIFT         20
#define I2S_RX_MCLK_RATIO_MASK          ( 0x0F << I2S_RX_MCLK_RATIO_SHIFT )


#define I2S_CLKDENOM_CFG                ( 0x0070 ) /* 0x70 -- 0x20f0  */
#define I2S_CLKNUMER_CFG                ( 0x0074 ) /* 0x74 -- 0x20f4  */
#define I2S_MCLK_DISABLE                ( 0x0078 ) /* 0x78 -- 0x20f8  */
#define CLK_DENOM                       0x0000FFFF

#define CLK_DENOM_MASK                   0xFFFFFFFF
#define CLK_NOMIN_MASK                   0xFFFFFFFF

#define CLK_NUMERATOR(x,y) ( y <= 48000 ? \
                           ( div_u64(div_u64((uint64_t)x*CLK_DENOM,1024),y) ) \
                          :( div_u64(div_u64((uint64_t)x*CLK_DENOM,128),y) ) )

struct clk_i2s_data {
  struct regmap *regmap_i2scfg;
};

struct clk_i2s {
  struct clk_hw hw;
  struct clk_i2s_data *i2s_data;
};

#define to_clk_i2s(_hw) container_of(_hw, struct clk_i2s, hw)

/* sclk=sample_rate*64=(parent_rate*(numerator/dinominator))/(mclk_ratio*2)  */
static int bcm_i2s_clk_set_rate(struct clk_hw *hw, unsigned long rate, 
                                                   unsigned long parent_rate)
{
   struct clk_i2s *i2s = to_clk_i2s(hw);
   struct clk_i2s_data *pdata = i2s->i2s_data;

   if( rate < 8000 || rate > 384000)
   {
      pr_err("%s: unsupported sample rate\n", __func__);
      return EINVAL;
   }

   regmap_update_bits(pdata->regmap_i2scfg,I2S_TX_CFG,
                                           I2S_TX_SCLKS_PER_1FS_DIV32_MASK,
                                           I2S_TX_64BITS_PERFRAME ); 
   regmap_update_bits(pdata->regmap_i2scfg,I2S_TX_CFG, I2S_TX_MCLK_RATIO_MASK, 
                                           (rate <= 48000 ? 8 : 1) << 
                                           I2S_TX_MCLK_RATIO_SHIFT);
   regmap_update_bits(pdata->regmap_i2scfg,I2S_RX_CFG, I2S_RX_MCLK_RATIO_MASK, 
                                           (rate <= 48000 ? 8 : 1) << 
                                           I2S_RX_MCLK_RATIO_SHIFT);
   regmap_update_bits(pdata->regmap_i2scfg,I2S_CLKDENOM_CFG, 
                                           CLK_DENOM_MASK,
                                           CLK_DENOM);
   regmap_update_bits(pdata->regmap_i2scfg,I2S_CLKNUMER_CFG, 
                                           CLK_NOMIN_MASK,
                                           CLK_NUMERATOR(parent_rate,rate));
                                           
   return 0;
}

/* based on parent rate, re-calculate the rate and returns*/
static unsigned long bcm_i2s_clk_recalc_rate(struct clk_hw *hw, 
                                             unsigned long parent_rate)
{
   struct clk_i2s *i2s = to_clk_i2s(hw);
   struct clk_i2s_data *pdata = i2s->i2s_data;
   unsigned int numerator,mclk_rate;

   regmap_read(pdata->regmap_i2scfg,I2S_CLKNUMER_CFG, &numerator);

   // we set mclk rate tx rx always same already
   regmap_read(pdata->regmap_i2scfg,I2S_TX_CFG, &mclk_rate);
   mclk_rate = ( mclk_rate & I2S_TX_MCLK_RATIO_MASK ) 
                 >> I2S_TX_MCLK_RATIO_SHIFT;

   /* ( parent_rate * CLK_DENOM )/( 128*mclk_rate )/numerator */
   return div_u64(div_u64((uint64_t)parent_rate*CLK_DENOM,(128*mclk_rate)),
                                                                numerator); 
}
static long bcm_i2s_clk_round_rate(struct clk_hw *hw, unsigned long rate, 
                                   unsigned long *parent_rate)
{
   return rate;
}
static const struct clk_ops clk_i2s_ops = {
  .set_rate    = bcm_i2s_clk_set_rate,
  .recalc_rate = bcm_i2s_clk_recalc_rate,
  .round_rate  = bcm_i2s_clk_round_rate,
};

static struct clk *bcm_i2s_clk_register(struct device *dev,
   const char *name,
   const char *parent_name,
   struct clk_i2s_data *i2s_data)
{
   struct clk_init_data init;
   struct clk_i2s *clki2s;
   struct clk *clk;
   clki2s = kzalloc(sizeof(*clki2s), GFP_KERNEL);
   if (!clki2s)
   {
      pr_err("%s: Out of memory\n", __func__);
      return ERR_PTR(-ENOMEM);
   }
   init.name = name;
   init.ops = &clk_i2s_ops;
   init.flags = 0;
   init.parent_names = (parent_name ? &parent_name : NULL);
   init.num_parents = (parent_name ? 1 : 0);

   clki2s->i2s_data = i2s_data;
   clki2s->hw.init = &init;

   clk = clk_register(NULL, &clki2s->hw);
   if (IS_ERR(clk))
   {
      kfree(clki2s);
   }
   return clk;
}

static void __init bcm_i2s_clk_init(struct device_node *node)
{
   const char *clk_name = node->name;
   const char *parent_name;
   struct clk_i2s_data *data;
   struct clk *clk;

   data = kzalloc(sizeof(*data), GFP_KERNEL);
   if (!data) {
      pr_err("%s: Out of memory\n", __func__);
      return;
   }

   data->regmap_i2scfg = 
                      syscon_regmap_lookup_by_phandle(node, "clk-mclk-syscon");
   if (IS_ERR(data->regmap_i2scfg)) {
      goto out ;
   }

   of_property_read_string(node, "clock-output-names", &clk_name);
   parent_name = of_clk_get_parent_name(node, 0);
   if (!parent_name) {
      pr_err("%s(): %s: of_clk_get_parent_name() failed\n",
           __func__, node->name);
    goto out;
   }

   clk = bcm_i2s_clk_register(NULL, clk_name, parent_name, data);
   if (!IS_ERR(clk)) {
      of_clk_add_provider(node, of_clk_src_simple_get, clk);
      pr_err("%s(): %s: clock register success\n", __func__, node->name);
      return;
   }
out:
   kfree(data);
   return;
}

static void __init of_bcm_i2s_clk_init(struct device_node *node)
{
   bcm_i2s_clk_init(node);
}
CLK_OF_DECLARE(bcm_gate_clk, "brcm,i2s-clock",	of_bcm_i2s_clk_init);
