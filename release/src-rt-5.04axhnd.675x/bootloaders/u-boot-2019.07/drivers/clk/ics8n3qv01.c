// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2017
 * Mario Six,  Guntermann & Drunck GmbH, mario.six@gdsys.cc
 *
 * based on the gdsys osd driver, which is
 *
 * (C) Copyright 2010
 * Dirk Eibach,  Guntermann & Drunck GmbH, eibach@gdsys.de
 */

#include <common.h>
#include <dm.h>
#include <clk-uclass.h>
#include <i2c.h>

const long long ICS8N3QV01_FREF = 114285000;
const long long ICS8N3QV01_FREF_LL = 114285000LL;
const long long ICS8N3QV01_F_DEFAULT_0 = 156250000LL;
const long long ICS8N3QV01_F_DEFAULT_1 = 125000000LL;
const long long ICS8N3QV01_F_DEFAULT_2 = 100000000LL;
const long long ICS8N3QV01_F_DEFAULT_3 = 25175000LL;

const uint MAX_FREQ_INDEX = 3;

struct ics8n3qv01_priv {
	ulong rate;
};

static int ics8n3qv01_get_fout_calc(struct udevice *dev, uint index,
				    uint *fout_calc)
{
	u64 n, mint, mfrac;
	u8 reg_a, reg_b, reg_c, reg_d, reg_f;
	int val[6];
	int i;

	if (index > MAX_FREQ_INDEX)
		return -EINVAL;

	for (i = 0; i <= 5; ++i) {
		u8 tmp = dm_i2c_reg_read(dev, 4 * i + index);

		if (tmp < 0) {
			debug("%s: Error while reading i2c register %d.\n",
			      dev->name, 4 * i + index);
			return tmp;
		}

		val[i] = tmp;
	}

	reg_a = val[0]; /* Register 0 + index */
	reg_b = val[1]; /* Register 4 + index */
	reg_c = val[2]; /* Register 8 + index */
	reg_d = val[3]; /* Register 12 + index */
	reg_f = val[5]; /* Register 20 + index */

	mint = ((reg_a >> 1) & 0x1f) | /* MINTi[4-0]*/
		(reg_f & 0x20);        /* MINTi[5] */
	mfrac = ((reg_a & 0x01) << 17) | /* MFRACi[17] */
		 (reg_b << 9) |          /* MFRACi[16-9] */
		 (reg_c << 1) |          /* MFRACi[8-1] */
		 (reg_d >> 7);           /* MFRACi[0] */
	n = reg_d & 0x7f; /* Ni[6-0] */

	*fout_calc = (mint * ICS8N3QV01_FREF_LL
		      + mfrac * ICS8N3QV01_FREF_LL / 262144LL
		      + ICS8N3QV01_FREF_LL / 524288LL
		      + n / 2)
		    / n
		    * 1000000
		    / (1000000 - 100);

	return 0;
}

static int ics8n3qv01_calc_parameters(uint fout, uint *_mint, uint *_mfrac,
				      uint *_n)
{
	uint n, foutiic, fvcoiic, mint;
	u64 mfrac;

	n = (2215000000U + fout / 2) / fout;
	if (fout < 417000000U)
		n = 2 * ((2215000000U / 2 + fout / 2) / fout);
	else
		n = (2215000000U + fout / 2) / fout;

	if ((n & 1) && n > 5)
		n -= 1;

	foutiic = fout - (fout / 10000);
	fvcoiic = foutiic * n;

	mint = fvcoiic / 114285000;
	if (mint < 17 || mint > 63)
		return -EINVAL;

	mfrac = ((u64)fvcoiic % 114285000LL) * 262144LL
		/ 114285000LL;

	*_mint = mint;
	*_mfrac = mfrac;
	*_n = n;

	return 0;
}

static ulong ics8n3qv01_set_rate(struct clk *clk, ulong fout)
{
	struct ics8n3qv01_priv *priv = dev_get_priv(clk->dev);
	uint n, mint, mfrac;
	uint fout_calc = 0;
	u64 fout_prog;
	long long off_ppm;
	int res, i;
	u8 reg[6];
	int tmp;
	int addr[] = {0, 4, 8, 12, 18, 20};

	priv->rate = fout;

	res = ics8n3qv01_get_fout_calc(clk->dev, 1, &fout_calc);

	if (res) {
		debug("%s: Error during output frequency calculation.\n",
		      clk->dev->name);
		return res;
	}

	off_ppm = (fout_calc - ICS8N3QV01_F_DEFAULT_1) * 1000000
		  / ICS8N3QV01_F_DEFAULT_1;
	printf("%s: PLL is off by %lld ppm\n", clk->dev->name, off_ppm);
	fout_prog = (u64)fout * (u64)fout_calc
		    / ICS8N3QV01_F_DEFAULT_1;
	res = ics8n3qv01_calc_parameters(fout_prog, &mint, &mfrac, &n);

	if (res) {
		debug("%s: Cannot determine mint parameter.\n",
		      clk->dev->name);
		return res;
	}

	/* Register 0 */
	tmp = dm_i2c_reg_read(clk->dev, 0) & 0xc0;
	if (tmp < 0)
		return tmp;
	reg[0] = tmp | (mint & 0x1f) << 1;
	reg[0] |= (mfrac >> 17) & 0x01;

	/* Register 4 */
	reg[1] = mfrac >> 9;

	/* Register 8 */
	reg[2] = mfrac >> 1;

	/* Register 12 */
	reg[3] = mfrac << 7;
	reg[3] |= n & 0x7f;

	/* Register 18 */
	tmp = dm_i2c_reg_read(clk->dev, 18) & 0x03;
	if (tmp < 0)
		return tmp;
	reg[4] = tmp | 0x20;

	/* Register 20 */
	tmp = dm_i2c_reg_read(clk->dev, 20) & 0x1f;
	if (tmp < 0)
		return tmp;
	reg[5] = tmp | (mint & (1 << 5));

	for (i = 0; i <= 5; ++i) {
		res = dm_i2c_reg_write(clk->dev, addr[i], reg[i]);
		if (res < 0)
			return res;
	}

	return 0;
}

static int ics8n3qv01_request(struct clk *clock)
{
	return 0;
}

static ulong ics8n3qv01_get_rate(struct clk *clk)
{
	struct ics8n3qv01_priv *priv = dev_get_priv(clk->dev);

	return priv->rate;
}

static int ics8n3qv01_enable(struct clk *clk)
{
	return 0;
}

static int ics8n3qv01_disable(struct clk *clk)
{
	return 0;
}

static const struct clk_ops ics8n3qv01_ops = {
	.request = ics8n3qv01_request,
	.get_rate = ics8n3qv01_get_rate,
	.set_rate = ics8n3qv01_set_rate,
	.enable = ics8n3qv01_enable,
	.disable = ics8n3qv01_disable,
};

static const struct udevice_id ics8n3qv01_ids[] = {
	{ .compatible = "idt,ics8n3qv01" },
	{ /* sentinel */ }
};

int ics8n3qv01_probe(struct udevice *dev)
{
	return 0;
}

U_BOOT_DRIVER(ics8n3qv01) = {
	.name           = "ics8n3qv01",
	.id             = UCLASS_CLK,
	.ops		= &ics8n3qv01_ops,
	.of_match       = ics8n3qv01_ids,
	.probe		= ics8n3qv01_probe,
	.priv_auto_alloc_size	= sizeof(struct ics8n3qv01_priv),
};
