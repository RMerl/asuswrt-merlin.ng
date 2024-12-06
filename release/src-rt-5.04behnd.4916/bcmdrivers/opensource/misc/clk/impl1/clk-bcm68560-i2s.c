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

/*
 * Clock driver for bcm68560 based devices
 *
 *	Kevin Li <kevin-ke.li@broadcom.com>
 *
 */

#include <linux/clk.h>
#include <linux/clk-provider.h>
#include <linux/mfd/syscon.h>
#include <linux/slab.h>
#include <linux/regmap.h>

#define I2S_MCLK_CLKSEL_CLR_MASK       0x00F30000
#define I2S_MCLK_RATE_SHIFT            20 
#define I2S_CLKSEL_SHIFT               16 
#define I2S_CLK_100MHZ                 0
#define I2S_CLK_50MHZ                  1
#define I2S_CLK_25MHZ                  2
#define I2S_CLK_PLL                    3

#define I2S_CFG                        (0x0000)
#define I2S_SCLKS_PER_1FS_DIV32_SHIFT  4 
#define I2S_64BITS_PERFRAME            ( 2<<I2S_SCLKS_PER_1FS_DIV32_SHIFT )
#define I2S_SCLKS_PER_1FS_DIV32_MASK   ( 0x0f<<I2S_SCLKS_PER_1FS_DIV32_SHIFT )

struct clk_i2s_data {
  struct regmap *regmap_i2scfg;
};

struct clk_i2s {
	struct clk_hw hw;
	struct clk_i2s_data *i2s_data;
};

#define to_clk_i2s(_hw) container_of(_hw, struct clk_i2s, hw)

/* actual clock=64FS=MCLK/(2*Mclk_ratio) */
static int bcm_i2s_clk_set_rate(struct clk_hw *hw, unsigned long rate, unsigned long parent_rate)
{
   struct clk_i2s *clki2s     = to_clk_i2s(hw);
   struct clk_i2s_data *pdata = clki2s->i2s_data;
   
   regmap_update_bits(pdata->regmap_i2scfg, I2S_CFG, I2S_SCLKS_PER_1FS_DIV32_MASK, I2S_64BITS_PERFRAME );    
   regmap_update_bits(pdata->regmap_i2scfg, I2S_CFG, I2S_MCLK_CLKSEL_CLR_MASK, 0 );
   regmap_update_bits(pdata->regmap_i2scfg, I2S_CFG, I2S_MCLK_CLKSEL_CLR_MASK,
                                            DIV_ROUND_CLOSEST(parent_rate/rate,128) << I2S_MCLK_RATE_SHIFT | 
                                            I2S_CLK_50MHZ  << I2S_CLKSEL_SHIFT);

	return 0;
}

/* based on parent rate, re-calculate the rate and returns*/
static unsigned long bcm_i2s_clk_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
/* to do return smaple rate */
   return 0;
}
static long bcm_i2s_clk_round_rate(struct clk_hw *hw, unsigned long rate, unsigned long *parent_rate)
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
   init.name  = name;
   init.ops   = &clk_i2s_ops;
   init.flags = 0;
   init.parent_names = (parent_name ? &parent_name : NULL);
   init.num_parents  = (parent_name ? 1 : 0);

   clki2s->i2s_data = i2s_data;
   clki2s->hw.init  = &init;

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

   data->regmap_i2scfg = syscon_regmap_lookup_by_phandle(node, "clk-mclk-syscon");
   if (IS_ERR(data->regmap_i2scfg)) {
     pr_err("%s(): %s: get regmap i2scfg failed\n",
          __func__, node->name);
      goto out;
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
