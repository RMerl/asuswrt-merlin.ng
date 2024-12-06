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

/* control regs */
#define PCM_CTRL		( 0x0000 )
#define PCM_CLK_CTRL0		( 0x0010 )
#define PCM_CLK_CTRL1		( 0x0014 )/*fcw scale*/
#define PCM_CLK_CTRL2		( 0x0018 )/*act fcw*/

/* control bits */
#define PCM_SAMPLE_DEPTH_SHIFT	22
#define PCM_SAMPLE_DEPTH_MASK	( 3 << PCM_SAMPLE_DEPTH_SHIFT )
#define PCM_SAMPLE_DEPTH_8BITS	0
#define PCM_SAMPLE_DEPTH_16BITS	1
#define PCM_SAMPLE_DEPTH_32BITS	2
#define PCM_CLK_DIV_SHIFT	14
#define PCM_CLK_DIV_MASK 	( 7 << PCM_CLK_DIV_SHIFT )
#define PCM_CLK_DIV_2		0
#define PCM_CLK_DIV_4		1
#define PCM_CLK_DIV_8		2
#define PCM_CH_SHIFT		0
#define PCM_CH_MASK		( 0xff << PCM_CH_SHIFT )
#define PCM_CH			2

#define PCM_NCO_FCW_SCALE	0x40000000
#define PCM_NCO_SOFT_INIT_SHIFT	7
#define PCM_NCO_SOFT_INIT_MASK	(1<<PCM_NCO_SOFT_INIT_SHIFT)
#define PCM_NCO_MUX_SHIFT	4
#define PCM_NCO_MUX_CTRL_MASK	( 3 << PCM_NCO_MUX_SHIFT )
#define PCM_NCO_MUX_CTRL_TRC	0
#define PCM_NCO_MUX_CTRL_NTPFCW	1
#define PCM_NCO_MUX_CTRL_DPLL	2
#define PCM_NCO_MUX_CTRL_MFCW	3
#define PCM_NCO_LOAD_MISC_SHIFT	6
#define PCM_NCO_LOAD_MISC_MASK	( 1 << PCM_NCO_LOAD_MISC_SHIFT )
#define PCM_NCO_SHIFT_MASK	0x0f

#define PCM_ENDIANNESS_CFG 0
#define PCM_ENDIANNESS_UBUS_CD_SM 0x05
#define PCM_ENDIANNESS_NONUBUS_CD_INT 0x06

struct clk_pcm_data {
	struct regmap *regmap_pcmcfg;
};

struct clk_pcm {
	struct clk_hw hw;
	struct clk_pcm_data *pcm_data;
};

#define to_clk_pcm(_hw) container_of(_hw, struct clk_pcm, hw)

static void pcm_nco_softinit( struct regmap *regmap)
{
	regmap_update_bits(regmap, PCM_CLK_CTRL2,
		PCM_NCO_SOFT_INIT_MASK, 1<<PCM_NCO_SOFT_INIT_SHIFT );
	regmap_update_bits(regmap, PCM_CLK_CTRL2,
		PCM_NCO_SOFT_INIT_MASK, 0 );
}

static void pcm_nco_load_misc( struct regmap *regmap )
{
	regmap_update_bits(regmap,PCM_CLK_CTRL2,
				PCM_NCO_LOAD_MISC_MASK,
				1 << PCM_NCO_LOAD_MISC_SHIFT );
	regmap_update_bits(regmap,PCM_CLK_CTRL2,
				PCM_NCO_LOAD_MISC_MASK,
				0  ); 
}

static void pcm_nco_setfcwinput( struct regmap *regmap, unsigned int source)
{
	regmap_update_bits(regmap,PCM_CLK_CTRL2,
				PCM_NCO_MUX_CTRL_MASK,
				source<<PCM_NCO_MUX_SHIFT);
}

static void pcm_nco_setscale( struct regmap *regmap, unsigned int scale)
{
	regmap_write(regmap, PCM_CLK_CTRL1, scale);
}

static int pcm_nco_setfcw( struct regmap *regmap,
			unsigned long rate,unsigned long parent_rate)
{
	uint32_t fcwnco,pcm_ctrl,sample_depth,pcm_channel,pcm_clk_div;
	regmap_read(regmap,PCM_CTRL, &pcm_ctrl);
	pcm_channel = pcm_ctrl & PCM_CH_MASK >> PCM_CH_SHIFT;
	pcm_clk_div = pcm_ctrl & PCM_CLK_DIV_MASK >> PCM_CLK_DIV_SHIFT;
	sample_depth = 
	(pcm_ctrl & PCM_SAMPLE_DEPTH_MASK >> PCM_SAMPLE_DEPTH_SHIFT)*16;
	if( !pcm_channel || sample_depth > 32 ) {
		pr_err("%s: Invalid parameters for PCM clock.\n", __func__);
		return -EINVAL;
	}
	fcwnco = DIV_ROUND_CLOSEST_ULL( rate * sample_depth * pcm_channel *
			pcm_clk_div * (uint64_t)1<<35 , parent_rate );
	regmap_write(regmap, PCM_CLK_CTRL0, fcwnco);
	return 0;
}

static void pcm_nco_setshift( struct regmap *regmap,unsigned int shift)
{
	regmap_update_bits(regmap, PCM_CLK_CTRL2, PCM_NCO_SHIFT_MASK, shift );
}

/* sclk=sample_rate*64=(parent_rate*(numerator/dinominator))/(mclk_ratio*2) */
static int bcm_pcm_clk_set_rate(struct clk_hw *hw, unsigned long rate,
			unsigned long parent_rate)
{
	struct clk_pcm *pcm = to_clk_pcm(hw);
	struct clk_pcm_data *pdata = pcm->pcm_data;
	struct regmap *regmap = pdata->regmap_pcmcfg;

	if( rate < 8000 || rate > 192000) {
		pr_err("%s: unsupported sample rate\n", __func__);
		return -EINVAL;
	}
	pcm_nco_softinit( regmap );
	pcm_nco_setfcwinput(regmap,PCM_NCO_MUX_CTRL_MFCW);
	if ( pcm_nco_setfcw(regmap,rate,parent_rate) )
		return -EINVAL;
	pcm_nco_setscale(regmap,PCM_NCO_FCW_SCALE);
	pcm_nco_setshift(regmap,0);
	pcm_nco_load_misc( regmap );

	return 0;
}

static unsigned long bcm_pcm_clk_recalc_rate(struct clk_hw *hw, 
			unsigned long parent_rate)
{
	struct clk_pcm *pcm = to_clk_pcm(hw);
	struct clk_pcm_data *pdata = pcm->pcm_data;
	uint32_t fcwnco,pcm_ctrl,sample_depth,pcm_channel,pcm_clk_div;

	regmap_read(pdata->regmap_pcmcfg,PCM_CTRL, &pcm_ctrl);
	pcm_channel  = pcm_ctrl & PCM_CH_MASK >> PCM_CH_SHIFT;
	pcm_clk_div  = pcm_ctrl & PCM_CLK_DIV_MASK >> PCM_CLK_DIV_SHIFT;
	sample_depth = 
	(pcm_ctrl & PCM_SAMPLE_DEPTH_MASK  >> PCM_SAMPLE_DEPTH_SHIFT)*16;
	regmap_read(pdata->regmap_pcmcfg, PCM_CLK_CTRL0, &fcwnco);

	if( !pcm_channel || sample_depth > 32 ) {
		pr_err("%s: Invalid parameters for PCM clock.\n", __func__);
		return -EINVAL;
	}

	return DIV_ROUND_CLOSEST_ULL(DIV_ROUND_CLOSEST_ULL(
			(uint64_t)fcwnco*parent_rate, (uint32_t)1<<31 ),
			pcm_channel*pcm_clk_div*sample_depth);
}

static long bcm_pcm_clk_round_rate(struct clk_hw *hw, unsigned long rate, 
			unsigned long *parent_rate)
{
	return rate;
}
static const struct clk_ops clk_pcm_ops = {
	.set_rate = bcm_pcm_clk_set_rate,
	.recalc_rate = bcm_pcm_clk_recalc_rate,
	.round_rate = bcm_pcm_clk_round_rate,
};

static struct clk *bcm_pcm_clk_register(struct device *dev,
	const char *name,
	const char *parent_name,
	struct clk_pcm_data *pcm_data)
{
	struct clk_init_data init;
	struct clk_pcm *clkpcm;
	struct clk *clk;
	clkpcm = kzalloc(sizeof(*clkpcm), GFP_KERNEL);
	if (!clkpcm) {
		pr_err("%s: Out of memory\n", __func__);
		return ERR_PTR(-ENOMEM);
	}
	init.name = name;
	init.ops = &clk_pcm_ops;
	init.flags = 0;
	init.parent_names = (parent_name ? &parent_name : NULL);
	init.num_parents = (parent_name ? 1 : 0);

	clkpcm->pcm_data = pcm_data;
	clkpcm->hw.init = &init;

	clk = clk_register(NULL, &clkpcm->hw);
	if (IS_ERR(clk))
		kfree(clkpcm);

	return clk;
}

static void __init bcm_pcm_clk_init(struct device_node *node)
{
	const char *clk_name = node->name;
	const char *parent_name;
	struct clk_pcm_data *data;
	struct clk *clk;

	data = kzalloc(sizeof(*data), GFP_KERNEL);
	if (!data) {
		pr_err("%s: Out of memory\n", __func__);
		return;
	}
	
	data->regmap_pcmcfg = 
		syscon_regmap_lookup_by_phandle(node, "pcm-endian-syscon");
	if (IS_ERR(data->regmap_pcmcfg)) {
		goto out ;
	}

	regmap_write(data->regmap_pcmcfg,PCM_ENDIANNESS_CFG,PCM_ENDIANNESS_NONUBUS_CD_INT);
	
	data->regmap_pcmcfg = 
		syscon_regmap_lookup_by_phandle(node, "clk-nco-syscon");
	if (IS_ERR(data->regmap_pcmcfg)) {
		goto out ;
	}
		
	of_property_read_string(node, "clock-output-names", &clk_name);
	parent_name = of_clk_get_parent_name(node, 0);
	if (!parent_name) {
		pr_err("%s(): %s: of_clk_get_parent_name() failed\n",
			__func__, node->name);
		goto out;
	}
	clk = bcm_pcm_clk_register(NULL, clk_name, parent_name, data);
	if (!IS_ERR(clk)) {
		of_clk_add_provider(node, of_clk_src_simple_get, clk);
		pr_err("%s(): %s: clock register success\n",
					 __func__, node->name);
		return;
	}
out:
	kfree(data);
	return;
}

static void __init of_bcm_pcm_clk_init(struct device_node *node)
{
	bcm_pcm_clk_init(node);
}
CLK_OF_DECLARE(bcm_gate_clk, "brcm,pcm-clock", of_bcm_pcm_clk_init);
