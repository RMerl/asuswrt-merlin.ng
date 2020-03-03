/*
 * Allegro A8293 SEC driver
 *
 * Copyright (C) 2011 Antti Palosaari <crope@iki.fi>
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License along
 *    with this program; if not, write to the Free Software Foundation, Inc.,
 *    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "dvb_frontend.h"
#include "a8293.h"

struct a8293_priv {
	struct i2c_adapter *i2c;
	const struct a8293_config *cfg;
	u8 reg[2];
};

static int a8293_i2c(struct a8293_priv *priv, u8 *val, int len, bool rd)
{
	int ret;
	struct i2c_msg msg[1] = {
		{
			.addr = priv->cfg->i2c_addr,
			.len = len,
			.buf = val,
		}
	};

	if (rd)
		msg[0].flags = I2C_M_RD;
	else
		msg[0].flags = 0;

	ret = i2c_transfer(priv->i2c, msg, 1);
	if (ret == 1) {
		ret = 0;
	} else {
		dev_warn(&priv->i2c->dev, "%s: i2c failed=%d rd=%d\n",
				KBUILD_MODNAME, ret, rd);
		ret = -EREMOTEIO;
	}

	return ret;
}

static int a8293_wr(struct a8293_priv *priv, u8 *val, int len)
{
	return a8293_i2c(priv, val, len, 0);
}

static int a8293_rd(struct a8293_priv *priv, u8 *val, int len)
{
	return a8293_i2c(priv, val, len, 1);
}

static int a8293_set_voltage(struct dvb_frontend *fe,
	fe_sec_voltage_t fe_sec_voltage)
{
	struct a8293_priv *priv = fe->sec_priv;
	int ret;

	dev_dbg(&priv->i2c->dev, "%s: fe_sec_voltage=%d\n", __func__,
			fe_sec_voltage);

	switch (fe_sec_voltage) {
	case SEC_VOLTAGE_OFF:
		/* ENB=0 */
		priv->reg[0] = 0x10;
		break;
	case SEC_VOLTAGE_13:
		/* VSEL0=1, VSEL1=0, VSEL2=0, VSEL3=0, ENB=1*/
		priv->reg[0] = 0x31;
		break;
	case SEC_VOLTAGE_18:
		/* VSEL0=0, VSEL1=0, VSEL2=0, VSEL3=1, ENB=1*/
		priv->reg[0] = 0x38;
		break;
	default:
		ret = -EINVAL;
		goto err;
	}

	ret = a8293_wr(priv, &priv->reg[0], 1);
	if (ret)
		goto err;

	usleep_range(1500, 50000);

	return ret;
err:
	dev_dbg(&priv->i2c->dev, "%s: failed=%d\n", __func__, ret);
	return ret;
}

static void a8293_release_sec(struct dvb_frontend *fe)
{
	a8293_set_voltage(fe, SEC_VOLTAGE_OFF);

	kfree(fe->sec_priv);
	fe->sec_priv = NULL;
}

struct dvb_frontend *a8293_attach(struct dvb_frontend *fe,
	struct i2c_adapter *i2c, const struct a8293_config *cfg)
{
	int ret;
	struct a8293_priv *priv = NULL;
	u8 buf[2];

	/* allocate memory for the internal priv */
	priv = kzalloc(sizeof(struct a8293_priv), GFP_KERNEL);
	if (priv == NULL) {
		ret = -ENOMEM;
		goto err;
	}

	/* setup the priv */
	priv->i2c = i2c;
	priv->cfg = cfg;
	fe->sec_priv = priv;

	/* check if the SEC is there */
	ret = a8293_rd(priv, buf, 2);
	if (ret)
		goto err;

	/* ENB=0 */
	priv->reg[0] = 0x10;
	ret = a8293_wr(priv, &priv->reg[0], 1);
	if (ret)
		goto err;

	/* TMODE=0, TGATE=1 */
	priv->reg[1] = 0x82;
	ret = a8293_wr(priv, &priv->reg[1], 1);
	if (ret)
		goto err;

	fe->ops.release_sec = a8293_release_sec;

	/* override frontend ops */
	fe->ops.set_voltage = a8293_set_voltage;

	dev_info(&priv->i2c->dev, "%s: Allegro A8293 SEC attached\n",
			KBUILD_MODNAME);

	return fe;
err:
	dev_dbg(&i2c->dev, "%s: failed=%d\n", __func__, ret);
	kfree(priv);
	return NULL;
}
EXPORT_SYMBOL(a8293_attach);

MODULE_AUTHOR("Antti Palosaari <crope@iki.fi>");
MODULE_DESCRIPTION("Allegro A8293 SEC driver");
MODULE_LICENSE("GPL");
