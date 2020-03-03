/*
 * Renesas R-Car Gen1 SRU/SSI support
 *
 * Copyright (C) 2013 Renesas Solutions Corp.
 * Kuninori Morimoto <kuninori.morimoto.gx@renesas.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/*
 * #define DEBUG
 *
 * you can also add below in
 * ${LINUX}/drivers/base/regmap/regmap.c
 * for regmap debug
 *
 * #define LOG_DEVICE "xxxx.rcar_sound"
 */

#include "rsnd.h"

struct rsnd_gen {
	void __iomem *base[RSND_BASE_MAX];

	struct rsnd_gen_ops *ops;

	struct regmap *regmap[RSND_BASE_MAX];
	struct regmap_field *regs[RSND_REG_MAX];
	phys_addr_t res[RSND_REG_MAX];
};

#define rsnd_priv_to_gen(p)	((struct rsnd_gen *)(p)->gen)

struct rsnd_regmap_field_conf {
	int idx;
	unsigned int reg_offset;
	unsigned int id_offset;
};

#define RSND_REG_SET(id, offset, _id_offset)	\
{						\
	.idx = id,				\
	.reg_offset = offset,			\
	.id_offset = _id_offset,		\
}
/* single address mapping */
#define RSND_GEN_S_REG(id, offset)	\
	RSND_REG_SET(RSND_REG_##id, offset, 0)

/* multi address mapping */
#define RSND_GEN_M_REG(id, offset, _id_offset)	\
	RSND_REG_SET(RSND_REG_##id, offset, _id_offset)

/*
 *		basic function
 */
static int rsnd_is_accessible_reg(struct rsnd_priv *priv,
				  struct rsnd_gen *gen, enum rsnd_reg reg)
{
	if (!gen->regs[reg]) {
		struct device *dev = rsnd_priv_to_dev(priv);

		dev_err(dev, "unsupported register access %x\n", reg);
		return 0;
	}

	return 1;
}

u32 rsnd_read(struct rsnd_priv *priv,
	      struct rsnd_mod *mod, enum rsnd_reg reg)
{
	struct device *dev = rsnd_priv_to_dev(priv);
	struct rsnd_gen *gen = rsnd_priv_to_gen(priv);
	u32 val;

	if (!rsnd_is_accessible_reg(priv, gen, reg))
		return 0;

	dev_dbg(dev, "r %s[%d] - %4d : %08x\n",
		rsnd_mod_name(mod), rsnd_mod_id(mod), reg, val);

	regmap_fields_read(gen->regs[reg], rsnd_mod_id(mod), &val);

	return val;
}

void rsnd_write(struct rsnd_priv *priv,
		struct rsnd_mod *mod,
		enum rsnd_reg reg, u32 data)
{
	struct device *dev = rsnd_priv_to_dev(priv);
	struct rsnd_gen *gen = rsnd_priv_to_gen(priv);

	if (!rsnd_is_accessible_reg(priv, gen, reg))
		return;

	dev_dbg(dev, "w %s[%d] - %4d : %08x\n",
		rsnd_mod_name(mod), rsnd_mod_id(mod), reg, data);

	regmap_fields_write(gen->regs[reg], rsnd_mod_id(mod), data);
}

void rsnd_bset(struct rsnd_priv *priv, struct rsnd_mod *mod,
	       enum rsnd_reg reg, u32 mask, u32 data)
{
	struct device *dev = rsnd_priv_to_dev(priv);
	struct rsnd_gen *gen = rsnd_priv_to_gen(priv);

	if (!rsnd_is_accessible_reg(priv, gen, reg))
		return;

	dev_dbg(dev, "b %s[%d] - %4d : %08x/%08x\n",
		rsnd_mod_name(mod), rsnd_mod_id(mod), reg, data, mask);

	regmap_fields_update_bits(gen->regs[reg], rsnd_mod_id(mod),
				  mask, data);
}

phys_addr_t rsnd_gen_get_phy_addr(struct rsnd_priv *priv, int reg_id)
{
	struct rsnd_gen *gen = rsnd_priv_to_gen(priv);

	return	gen->res[reg_id];
}

#define rsnd_gen_regmap_init(priv, id_size, reg_id, name, conf)		\
	_rsnd_gen_regmap_init(priv, id_size, reg_id, name, conf, ARRAY_SIZE(conf))
static int _rsnd_gen_regmap_init(struct rsnd_priv *priv,
				 int id_size,
				 int reg_id,
				 const char *name,
				 struct rsnd_regmap_field_conf *conf,
				 int conf_size)
{
	struct platform_device *pdev = rsnd_priv_to_pdev(priv);
	struct rsnd_gen *gen = rsnd_priv_to_gen(priv);
	struct device *dev = rsnd_priv_to_dev(priv);
	struct resource *res;
	struct regmap_config regc;
	struct regmap_field *regs;
	struct regmap *regmap;
	struct reg_field regf;
	void __iomem *base;
	int i;

	memset(&regc, 0, sizeof(regc));
	regc.reg_bits = 32;
	regc.val_bits = 32;
	regc.reg_stride = 4;
	regc.name = name;

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, name);
	if (!res)
		res = platform_get_resource(pdev, IORESOURCE_MEM, reg_id);
	if (!res)
		return -ENODEV;

	base = devm_ioremap_resource(dev, res);
	if (IS_ERR(base))
		return PTR_ERR(base);

	regmap = devm_regmap_init_mmio(dev, base, &regc);
	if (IS_ERR(regmap))
		return PTR_ERR(regmap);

	gen->base[reg_id] = base;
	gen->regmap[reg_id] = regmap;
	gen->res[reg_id] = res->start;

	for (i = 0; i < conf_size; i++) {

		regf.reg	= conf[i].reg_offset;
		regf.id_offset	= conf[i].id_offset;
		regf.lsb	= 0;
		regf.msb	= 31;
		regf.id_size	= id_size;

		regs = devm_regmap_field_alloc(dev, regmap, regf);
		if (IS_ERR(regs))
			return PTR_ERR(regs);

		gen->regs[conf[i].idx] = regs;
	}

	return 0;
}

/*
 *		Gen2
 */
static int rsnd_gen2_probe(struct platform_device *pdev,
			   struct rsnd_priv *priv)
{
	struct rsnd_regmap_field_conf conf_ssiu[] = {
		RSND_GEN_S_REG(SSI_MODE0,	0x800),
		RSND_GEN_S_REG(SSI_MODE1,	0x804),
		/* FIXME: it needs SSI_MODE2/3 in the future */
		RSND_GEN_M_REG(SSI_BUSIF_MODE,	0x0,	0x80),
		RSND_GEN_M_REG(SSI_BUSIF_ADINR,	0x4,	0x80),
		RSND_GEN_M_REG(BUSIF_DALIGN,	0x8,	0x80),
		RSND_GEN_M_REG(SSI_CTRL,	0x10,	0x80),
		RSND_GEN_M_REG(INT_ENABLE,	0x18,	0x80),
	};
	struct rsnd_regmap_field_conf conf_scu[] = {
		RSND_GEN_M_REG(SRC_BUSIF_MODE,	0x0,	0x20),
		RSND_GEN_M_REG(SRC_ROUTE_MODE0,	0xc,	0x20),
		RSND_GEN_M_REG(SRC_CTRL,	0x10,	0x20),
		RSND_GEN_M_REG(SRC_INT_ENABLE0,	0x18,	0x20),
		RSND_GEN_M_REG(CMD_ROUTE_SLCT,	0x18c,	0x20),
		RSND_GEN_M_REG(CMD_CTRL,	0x190,	0x20),
		RSND_GEN_S_REG(SCU_SYS_STATUS0,	0x1c8),
		RSND_GEN_S_REG(SCU_SYS_INT_EN0,	0x1cc),
		RSND_GEN_S_REG(SCU_SYS_STATUS1,	0x1d0),
		RSND_GEN_S_REG(SCU_SYS_INT_EN1,	0x1d4),
		RSND_GEN_M_REG(SRC_SWRSR,	0x200,	0x40),
		RSND_GEN_M_REG(SRC_SRCIR,	0x204,	0x40),
		RSND_GEN_M_REG(SRC_ADINR,	0x214,	0x40),
		RSND_GEN_M_REG(SRC_IFSCR,	0x21c,	0x40),
		RSND_GEN_M_REG(SRC_IFSVR,	0x220,	0x40),
		RSND_GEN_M_REG(SRC_SRCCR,	0x224,	0x40),
		RSND_GEN_M_REG(SRC_BSDSR,	0x22c,	0x40),
		RSND_GEN_M_REG(SRC_BSISR,	0x238,	0x40),
		RSND_GEN_M_REG(DVC_SWRSR,	0xe00,	0x100),
		RSND_GEN_M_REG(DVC_DVUIR,	0xe04,	0x100),
		RSND_GEN_M_REG(DVC_ADINR,	0xe08,	0x100),
		RSND_GEN_M_REG(DVC_DVUCR,	0xe10,	0x100),
		RSND_GEN_M_REG(DVC_ZCMCR,	0xe14,	0x100),
		RSND_GEN_M_REG(DVC_VRCTR,	0xe18,	0x100),
		RSND_GEN_M_REG(DVC_VRPDR,	0xe1c,	0x100),
		RSND_GEN_M_REG(DVC_VRDBR,	0xe20,	0x100),
		RSND_GEN_M_REG(DVC_VOL0R,	0xe28,	0x100),
		RSND_GEN_M_REG(DVC_VOL1R,	0xe2c,	0x100),
		RSND_GEN_M_REG(DVC_DVUER,	0xe48,	0x100),
	};
	struct rsnd_regmap_field_conf conf_adg[] = {
		RSND_GEN_S_REG(BRRA,		0x00),
		RSND_GEN_S_REG(BRRB,		0x04),
		RSND_GEN_S_REG(SSICKR,		0x08),
		RSND_GEN_S_REG(AUDIO_CLK_SEL0,	0x0c),
		RSND_GEN_S_REG(AUDIO_CLK_SEL1,	0x10),
		RSND_GEN_S_REG(AUDIO_CLK_SEL2,	0x14),
		RSND_GEN_S_REG(DIV_EN,		0x30),
		RSND_GEN_S_REG(SRCIN_TIMSEL0,	0x34),
		RSND_GEN_S_REG(SRCIN_TIMSEL1,	0x38),
		RSND_GEN_S_REG(SRCIN_TIMSEL2,	0x3c),
		RSND_GEN_S_REG(SRCIN_TIMSEL3,	0x40),
		RSND_GEN_S_REG(SRCIN_TIMSEL4,	0x44),
		RSND_GEN_S_REG(SRCOUT_TIMSEL0,	0x48),
		RSND_GEN_S_REG(SRCOUT_TIMSEL1,	0x4c),
		RSND_GEN_S_REG(SRCOUT_TIMSEL2,	0x50),
		RSND_GEN_S_REG(SRCOUT_TIMSEL3,	0x54),
		RSND_GEN_S_REG(SRCOUT_TIMSEL4,	0x58),
		RSND_GEN_S_REG(CMDOUT_TIMSEL,	0x5c),
	};
	struct rsnd_regmap_field_conf conf_ssi[] = {
		RSND_GEN_M_REG(SSICR,		0x00,	0x40),
		RSND_GEN_M_REG(SSISR,		0x04,	0x40),
		RSND_GEN_M_REG(SSITDR,		0x08,	0x40),
		RSND_GEN_M_REG(SSIRDR,		0x0c,	0x40),
		RSND_GEN_M_REG(SSIWSR,		0x20,	0x40),
	};
	int ret_ssiu;
	int ret_scu;
	int ret_adg;
	int ret_ssi;

	ret_ssiu = rsnd_gen_regmap_init(priv, 10, RSND_GEN2_SSIU, "ssiu", conf_ssiu);
	ret_scu  = rsnd_gen_regmap_init(priv, 10, RSND_GEN2_SCU,  "scu",  conf_scu);
	ret_adg  = rsnd_gen_regmap_init(priv, 10, RSND_GEN2_ADG,  "adg",  conf_adg);
	ret_ssi  = rsnd_gen_regmap_init(priv, 10, RSND_GEN2_SSI,  "ssi",  conf_ssi);
	if (ret_ssiu < 0 ||
	    ret_scu  < 0 ||
	    ret_adg  < 0 ||
	    ret_ssi  < 0)
		return ret_ssiu | ret_scu | ret_adg | ret_ssi;

	return 0;
}

/*
 *		Gen1
 */

static int rsnd_gen1_probe(struct platform_device *pdev,
			   struct rsnd_priv *priv)
{
	struct rsnd_regmap_field_conf conf_sru[] = {
		RSND_GEN_S_REG(SRC_ROUTE_SEL,	0x00),
		RSND_GEN_S_REG(SRC_TMG_SEL0,	0x08),
		RSND_GEN_S_REG(SRC_TMG_SEL1,	0x0c),
		RSND_GEN_S_REG(SRC_TMG_SEL2,	0x10),
		RSND_GEN_S_REG(SRC_ROUTE_CTRL,	0xc0),
		RSND_GEN_S_REG(SSI_MODE0,	0xD0),
		RSND_GEN_S_REG(SSI_MODE1,	0xD4),
		RSND_GEN_M_REG(SRC_BUSIF_MODE,	0x20,	0x4),
		RSND_GEN_M_REG(SRC_ROUTE_MODE0,	0x50,	0x8),
		RSND_GEN_M_REG(SRC_SWRSR,	0x200,	0x40),
		RSND_GEN_M_REG(SRC_SRCIR,	0x204,	0x40),
		RSND_GEN_M_REG(SRC_ADINR,	0x214,	0x40),
		RSND_GEN_M_REG(SRC_IFSCR,	0x21c,	0x40),
		RSND_GEN_M_REG(SRC_IFSVR,	0x220,	0x40),
		RSND_GEN_M_REG(SRC_SRCCR,	0x224,	0x40),
		RSND_GEN_M_REG(SRC_MNFSR,	0x228,	0x40),
		/*
		 * ADD US
		 *
		 * SRC_STATUS
		 * SRC_INT_EN
		 * SCU_SYS_STATUS0
		 * SCU_SYS_STATUS1
		 * SCU_SYS_INT_EN0
		 * SCU_SYS_INT_EN1
		 */
	};
	struct rsnd_regmap_field_conf conf_adg[] = {
		RSND_GEN_S_REG(BRRA,		0x00),
		RSND_GEN_S_REG(BRRB,		0x04),
		RSND_GEN_S_REG(SSICKR,		0x08),
		RSND_GEN_S_REG(AUDIO_CLK_SEL0,	0x0c),
		RSND_GEN_S_REG(AUDIO_CLK_SEL1,	0x10),
		RSND_GEN_S_REG(AUDIO_CLK_SEL3,	0x18),
		RSND_GEN_S_REG(AUDIO_CLK_SEL4,	0x1c),
		RSND_GEN_S_REG(AUDIO_CLK_SEL5,	0x20),
	};
	struct rsnd_regmap_field_conf conf_ssi[] = {
		RSND_GEN_M_REG(SSICR,		0x00,	0x40),
		RSND_GEN_M_REG(SSISR,		0x04,	0x40),
		RSND_GEN_M_REG(SSITDR,		0x08,	0x40),
		RSND_GEN_M_REG(SSIRDR,		0x0c,	0x40),
		RSND_GEN_M_REG(SSIWSR,		0x20,	0x40),
	};
	int ret_sru;
	int ret_adg;
	int ret_ssi;

	ret_sru  = rsnd_gen_regmap_init(priv, 9, RSND_GEN1_SRU, "sru", conf_sru);
	ret_adg  = rsnd_gen_regmap_init(priv, 9, RSND_GEN1_ADG, "adg", conf_adg);
	ret_ssi  = rsnd_gen_regmap_init(priv, 9, RSND_GEN1_SSI, "ssi", conf_ssi);
	if (ret_sru  < 0 ||
	    ret_adg  < 0 ||
	    ret_ssi  < 0)
		return ret_sru | ret_adg | ret_ssi;

	return 0;
}

/*
 *		Gen
 */
static void rsnd_of_parse_gen(struct platform_device *pdev,
			      const struct rsnd_of_data *of_data,
			      struct rsnd_priv *priv)
{
	struct rcar_snd_info *info = priv->info;

	if (!of_data)
		return;

	info->flags = of_data->flags;
}

int rsnd_gen_probe(struct platform_device *pdev,
		   const struct rsnd_of_data *of_data,
		   struct rsnd_priv *priv)
{
	struct device *dev = rsnd_priv_to_dev(priv);
	struct rsnd_gen *gen;
	int ret;

	rsnd_of_parse_gen(pdev, of_data, priv);

	gen = devm_kzalloc(dev, sizeof(*gen), GFP_KERNEL);
	if (!gen) {
		dev_err(dev, "GEN allocate failed\n");
		return -ENOMEM;
	}

	priv->gen = gen;

	ret = -ENODEV;
	if (rsnd_is_gen1(priv))
		ret = rsnd_gen1_probe(pdev, priv);
	else if (rsnd_is_gen2(priv))
		ret = rsnd_gen2_probe(pdev, priv);

	if (ret < 0)
		dev_err(dev, "unknown generation R-Car sound device\n");

	return ret;
}
