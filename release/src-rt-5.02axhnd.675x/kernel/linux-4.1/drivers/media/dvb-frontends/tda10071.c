/*
 * NXP TDA10071 + Conexant CX24118A DVB-S/S2 demodulator + tuner driver
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

#include "tda10071_priv.h"

/* Max transfer size done by I2C transfer functions */
#define MAX_XFER_SIZE  64

static struct dvb_frontend_ops tda10071_ops;

/* write multiple registers */
static int tda10071_wr_regs(struct tda10071_priv *priv, u8 reg, u8 *val,
	int len)
{
	int ret;
	u8 buf[MAX_XFER_SIZE];
	struct i2c_msg msg[1] = {
		{
			.addr = priv->cfg.demod_i2c_addr,
			.flags = 0,
			.len = 1 + len,
			.buf = buf,
		}
	};

	if (1 + len > sizeof(buf)) {
		dev_warn(&priv->i2c->dev,
				"%s: i2c wr reg=%04x: len=%d is too big!\n",
				KBUILD_MODNAME, reg, len);
		return -EINVAL;
	}

	buf[0] = reg;
	memcpy(&buf[1], val, len);

	ret = i2c_transfer(priv->i2c, msg, 1);
	if (ret == 1) {
		ret = 0;
	} else {
		dev_warn(&priv->i2c->dev,
				"%s: i2c wr failed=%d reg=%02x len=%d\n",
				KBUILD_MODNAME, ret, reg, len);
		ret = -EREMOTEIO;
	}
	return ret;
}

/* read multiple registers */
static int tda10071_rd_regs(struct tda10071_priv *priv, u8 reg, u8 *val,
	int len)
{
	int ret;
	u8 buf[MAX_XFER_SIZE];
	struct i2c_msg msg[2] = {
		{
			.addr = priv->cfg.demod_i2c_addr,
			.flags = 0,
			.len = 1,
			.buf = &reg,
		}, {
			.addr = priv->cfg.demod_i2c_addr,
			.flags = I2C_M_RD,
			.len = len,
			.buf = buf,
		}
	};

	if (len > sizeof(buf)) {
		dev_warn(&priv->i2c->dev,
				"%s: i2c wr reg=%04x: len=%d is too big!\n",
				KBUILD_MODNAME, reg, len);
		return -EINVAL;
	}

	ret = i2c_transfer(priv->i2c, msg, 2);
	if (ret == 2) {
		memcpy(val, buf, len);
		ret = 0;
	} else {
		dev_warn(&priv->i2c->dev,
				"%s: i2c rd failed=%d reg=%02x len=%d\n",
				KBUILD_MODNAME, ret, reg, len);
		ret = -EREMOTEIO;
	}
	return ret;
}

/* write single register */
static int tda10071_wr_reg(struct tda10071_priv *priv, u8 reg, u8 val)
{
	return tda10071_wr_regs(priv, reg, &val, 1);
}

/* read single register */
static int tda10071_rd_reg(struct tda10071_priv *priv, u8 reg, u8 *val)
{
	return tda10071_rd_regs(priv, reg, val, 1);
}

/* write single register with mask */
static int tda10071_wr_reg_mask(struct tda10071_priv *priv,
				u8 reg, u8 val, u8 mask)
{
	int ret;
	u8 tmp;

	/* no need for read if whole reg is written */
	if (mask != 0xff) {
		ret = tda10071_rd_regs(priv, reg, &tmp, 1);
		if (ret)
			return ret;

		val &= mask;
		tmp &= ~mask;
		val |= tmp;
	}

	return tda10071_wr_regs(priv, reg, &val, 1);
}

/* read single register with mask */
static int tda10071_rd_reg_mask(struct tda10071_priv *priv,
				u8 reg, u8 *val, u8 mask)
{
	int ret, i;
	u8 tmp;

	ret = tda10071_rd_regs(priv, reg, &tmp, 1);
	if (ret)
		return ret;

	tmp &= mask;

	/* find position of the first bit */
	for (i = 0; i < 8; i++) {
		if ((mask >> i) & 0x01)
			break;
	}
	*val = tmp >> i;

	return 0;
}

/* execute firmware command */
static int tda10071_cmd_execute(struct tda10071_priv *priv,
	struct tda10071_cmd *cmd)
{
	int ret, i;
	u8 tmp;

	if (!priv->warm) {
		ret = -EFAULT;
		goto error;
	}

	/* write cmd and args for firmware */
	ret = tda10071_wr_regs(priv, 0x00, cmd->args, cmd->len);
	if (ret)
		goto error;

	/* start cmd execution */
	ret = tda10071_wr_reg(priv, 0x1f, 1);
	if (ret)
		goto error;

	/* wait cmd execution terminate */
	for (i = 1000, tmp = 1; i && tmp; i--) {
		ret = tda10071_rd_reg(priv, 0x1f, &tmp);
		if (ret)
			goto error;

		usleep_range(200, 5000);
	}

	dev_dbg(&priv->i2c->dev, "%s: loop=%d\n", __func__, i);

	if (i == 0) {
		ret = -ETIMEDOUT;
		goto error;
	}

	return ret;
error:
	dev_dbg(&priv->i2c->dev, "%s: failed=%d\n", __func__, ret);
	return ret;
}

static int tda10071_set_tone(struct dvb_frontend *fe,
	fe_sec_tone_mode_t fe_sec_tone_mode)
{
	struct tda10071_priv *priv = fe->demodulator_priv;
	struct tda10071_cmd cmd;
	int ret;
	u8 tone;

	if (!priv->warm) {
		ret = -EFAULT;
		goto error;
	}

	dev_dbg(&priv->i2c->dev, "%s: tone_mode=%d\n", __func__,
			fe_sec_tone_mode);

	switch (fe_sec_tone_mode) {
	case SEC_TONE_ON:
		tone = 1;
		break;
	case SEC_TONE_OFF:
		tone = 0;
		break;
	default:
		dev_dbg(&priv->i2c->dev, "%s: invalid fe_sec_tone_mode\n",
				__func__);
		ret = -EINVAL;
		goto error;
	}

	cmd.args[0] = CMD_LNB_PCB_CONFIG;
	cmd.args[1] = 0;
	cmd.args[2] = 0x00;
	cmd.args[3] = 0x00;
	cmd.args[4] = tone;
	cmd.len = 5;
	ret = tda10071_cmd_execute(priv, &cmd);
	if (ret)
		goto error;

	return ret;
error:
	dev_dbg(&priv->i2c->dev, "%s: failed=%d\n", __func__, ret);
	return ret;
}

static int tda10071_set_voltage(struct dvb_frontend *fe,
	fe_sec_voltage_t fe_sec_voltage)
{
	struct tda10071_priv *priv = fe->demodulator_priv;
	struct tda10071_cmd cmd;
	int ret;
	u8 voltage;

	if (!priv->warm) {
		ret = -EFAULT;
		goto error;
	}

	dev_dbg(&priv->i2c->dev, "%s: voltage=%d\n", __func__, fe_sec_voltage);

	switch (fe_sec_voltage) {
	case SEC_VOLTAGE_13:
		voltage = 0;
		break;
	case SEC_VOLTAGE_18:
		voltage = 1;
		break;
	case SEC_VOLTAGE_OFF:
		voltage = 0;
		break;
	default:
		dev_dbg(&priv->i2c->dev, "%s: invalid fe_sec_voltage\n",
				__func__);
		ret = -EINVAL;
		goto error;
	}

	cmd.args[0] = CMD_LNB_SET_DC_LEVEL;
	cmd.args[1] = 0;
	cmd.args[2] = voltage;
	cmd.len = 3;
	ret = tda10071_cmd_execute(priv, &cmd);
	if (ret)
		goto error;

	return ret;
error:
	dev_dbg(&priv->i2c->dev, "%s: failed=%d\n", __func__, ret);
	return ret;
}

static int tda10071_diseqc_send_master_cmd(struct dvb_frontend *fe,
	struct dvb_diseqc_master_cmd *diseqc_cmd)
{
	struct tda10071_priv *priv = fe->demodulator_priv;
	struct tda10071_cmd cmd;
	int ret, i;
	u8 tmp;

	if (!priv->warm) {
		ret = -EFAULT;
		goto error;
	}

	dev_dbg(&priv->i2c->dev, "%s: msg_len=%d\n", __func__,
			diseqc_cmd->msg_len);

	if (diseqc_cmd->msg_len < 3 || diseqc_cmd->msg_len > 6) {
		ret = -EINVAL;
		goto error;
	}

	/* wait LNB TX */
	for (i = 500, tmp = 0; i && !tmp; i--) {
		ret = tda10071_rd_reg_mask(priv, 0x47, &tmp, 0x01);
		if (ret)
			goto error;

		usleep_range(10000, 20000);
	}

	dev_dbg(&priv->i2c->dev, "%s: loop=%d\n", __func__, i);

	if (i == 0) {
		ret = -ETIMEDOUT;
		goto error;
	}

	ret = tda10071_wr_reg_mask(priv, 0x47, 0x00, 0x01);
	if (ret)
		goto error;

	cmd.args[0] = CMD_LNB_SEND_DISEQC;
	cmd.args[1] = 0;
	cmd.args[2] = 0;
	cmd.args[3] = 0;
	cmd.args[4] = 2;
	cmd.args[5] = 0;
	cmd.args[6] = diseqc_cmd->msg_len;
	memcpy(&cmd.args[7], diseqc_cmd->msg, diseqc_cmd->msg_len);
	cmd.len = 7 + diseqc_cmd->msg_len;
	ret = tda10071_cmd_execute(priv, &cmd);
	if (ret)
		goto error;

	return ret;
error:
	dev_dbg(&priv->i2c->dev, "%s: failed=%d\n", __func__, ret);
	return ret;
}

static int tda10071_diseqc_recv_slave_reply(struct dvb_frontend *fe,
	struct dvb_diseqc_slave_reply *reply)
{
	struct tda10071_priv *priv = fe->demodulator_priv;
	struct tda10071_cmd cmd;
	int ret, i;
	u8 tmp;

	if (!priv->warm) {
		ret = -EFAULT;
		goto error;
	}

	dev_dbg(&priv->i2c->dev, "%s:\n", __func__);

	/* wait LNB RX */
	for (i = 500, tmp = 0; i && !tmp; i--) {
		ret = tda10071_rd_reg_mask(priv, 0x47, &tmp, 0x02);
		if (ret)
			goto error;

		usleep_range(10000, 20000);
	}

	dev_dbg(&priv->i2c->dev, "%s: loop=%d\n", __func__, i);

	if (i == 0) {
		ret = -ETIMEDOUT;
		goto error;
	}

	/* reply len */
	ret = tda10071_rd_reg(priv, 0x46, &tmp);
	if (ret)
		goto error;

	reply->msg_len = tmp & 0x1f; /* [4:0] */
	if (reply->msg_len > sizeof(reply->msg))
		reply->msg_len = sizeof(reply->msg); /* truncate API max */

	/* read reply */
	cmd.args[0] = CMD_LNB_UPDATE_REPLY;
	cmd.args[1] = 0;
	cmd.len = 2;
	ret = tda10071_cmd_execute(priv, &cmd);
	if (ret)
		goto error;

	ret = tda10071_rd_regs(priv, cmd.len, reply->msg, reply->msg_len);
	if (ret)
		goto error;

	return ret;
error:
	dev_dbg(&priv->i2c->dev, "%s: failed=%d\n", __func__, ret);
	return ret;
}

static int tda10071_diseqc_send_burst(struct dvb_frontend *fe,
	fe_sec_mini_cmd_t fe_sec_mini_cmd)
{
	struct tda10071_priv *priv = fe->demodulator_priv;
	struct tda10071_cmd cmd;
	int ret, i;
	u8 tmp, burst;

	if (!priv->warm) {
		ret = -EFAULT;
		goto error;
	}

	dev_dbg(&priv->i2c->dev, "%s: fe_sec_mini_cmd=%d\n", __func__,
			fe_sec_mini_cmd);

	switch (fe_sec_mini_cmd) {
	case SEC_MINI_A:
		burst = 0;
		break;
	case SEC_MINI_B:
		burst = 1;
		break;
	default:
		dev_dbg(&priv->i2c->dev, "%s: invalid fe_sec_mini_cmd\n",
				__func__);
		ret = -EINVAL;
		goto error;
	}

	/* wait LNB TX */
	for (i = 500, tmp = 0; i && !tmp; i--) {
		ret = tda10071_rd_reg_mask(priv, 0x47, &tmp, 0x01);
		if (ret)
			goto error;

		usleep_range(10000, 20000);
	}

	dev_dbg(&priv->i2c->dev, "%s: loop=%d\n", __func__, i);

	if (i == 0) {
		ret = -ETIMEDOUT;
		goto error;
	}

	ret = tda10071_wr_reg_mask(priv, 0x47, 0x00, 0x01);
	if (ret)
		goto error;

	cmd.args[0] = CMD_LNB_SEND_TONEBURST;
	cmd.args[1] = 0;
	cmd.args[2] = burst;
	cmd.len = 3;
	ret = tda10071_cmd_execute(priv, &cmd);
	if (ret)
		goto error;

	return ret;
error:
	dev_dbg(&priv->i2c->dev, "%s: failed=%d\n", __func__, ret);
	return ret;
}

static int tda10071_read_status(struct dvb_frontend *fe, fe_status_t *status)
{
	struct tda10071_priv *priv = fe->demodulator_priv;
	int ret;
	u8 tmp;

	*status = 0;

	if (!priv->warm) {
		ret = 0;
		goto error;
	}

	ret = tda10071_rd_reg(priv, 0x39, &tmp);
	if (ret)
		goto error;

	/* 0x39[0] tuner PLL */
	if (tmp & 0x02) /* demod PLL */
		*status |= FE_HAS_SIGNAL | FE_HAS_CARRIER;
	if (tmp & 0x04) /* viterbi or LDPC*/
		*status |= FE_HAS_VITERBI;
	if (tmp & 0x08) /* RS or BCH */
		*status |= FE_HAS_SYNC | FE_HAS_LOCK;

	priv->fe_status = *status;

	return ret;
error:
	dev_dbg(&priv->i2c->dev, "%s: failed=%d\n", __func__, ret);
	return ret;
}

static int tda10071_read_snr(struct dvb_frontend *fe, u16 *snr)
{
	struct tda10071_priv *priv = fe->demodulator_priv;
	int ret;
	u8 buf[2];

	if (!priv->warm || !(priv->fe_status & FE_HAS_LOCK)) {
		*snr = 0;
		ret = 0;
		goto error;
	}

	ret = tda10071_rd_regs(priv, 0x3a, buf, 2);
	if (ret)
		goto error;

	/* Es/No dBx10 */
	*snr = buf[0] << 8 | buf[1];

	return ret;
error:
	dev_dbg(&priv->i2c->dev, "%s: failed=%d\n", __func__, ret);
	return ret;
}

static int tda10071_read_signal_strength(struct dvb_frontend *fe, u16 *strength)
{
	struct tda10071_priv *priv = fe->demodulator_priv;
	struct tda10071_cmd cmd;
	int ret;
	u8 tmp;

	if (!priv->warm || !(priv->fe_status & FE_HAS_LOCK)) {
		*strength = 0;
		ret = 0;
		goto error;
	}

	cmd.args[0] = CMD_GET_AGCACC;
	cmd.args[1] = 0;
	cmd.len = 2;
	ret = tda10071_cmd_execute(priv, &cmd);
	if (ret)
		goto error;

	/* input power estimate dBm */
	ret = tda10071_rd_reg(priv, 0x50, &tmp);
	if (ret)
		goto error;

	if (tmp < 181)
		tmp = 181; /* -75 dBm */
	else if (tmp > 236)
		tmp = 236; /* -20 dBm */

	/* scale value to 0x0000-0xffff */
	*strength = (tmp-181) * 0xffff / (236-181);

	return ret;
error:
	dev_dbg(&priv->i2c->dev, "%s: failed=%d\n", __func__, ret);
	return ret;
}

static int tda10071_read_ber(struct dvb_frontend *fe, u32 *ber)
{
	struct tda10071_priv *priv = fe->demodulator_priv;
	struct tda10071_cmd cmd;
	int ret, i, len;
	u8 tmp, reg, buf[8];

	if (!priv->warm || !(priv->fe_status & FE_HAS_LOCK)) {
		*ber = priv->ber = 0;
		ret = 0;
		goto error;
	}

	switch (priv->delivery_system) {
	case SYS_DVBS:
		reg = 0x4c;
		len = 8;
		i = 1;
		break;
	case SYS_DVBS2:
		reg = 0x4d;
		len = 4;
		i = 0;
		break;
	default:
		*ber = priv->ber = 0;
		return 0;
	}

	ret = tda10071_rd_reg(priv, reg, &tmp);
	if (ret)
		goto error;

	if (priv->meas_count[i] == tmp) {
		dev_dbg(&priv->i2c->dev, "%s: meas not ready=%02x\n", __func__,
				tmp);
		*ber = priv->ber;
		return 0;
	} else {
		priv->meas_count[i] = tmp;
	}

	cmd.args[0] = CMD_BER_UPDATE_COUNTERS;
	cmd.args[1] = 0;
	cmd.args[2] = i;
	cmd.len = 3;
	ret = tda10071_cmd_execute(priv, &cmd);
	if (ret)
		goto error;

	ret = tda10071_rd_regs(priv, cmd.len, buf, len);
	if (ret)
		goto error;

	if (priv->delivery_system == SYS_DVBS) {
		*ber = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
		priv->ucb += (buf[4] << 8) | buf[5];
	} else {
		*ber = (buf[0] << 8) | buf[1];
	}
	priv->ber = *ber;

	return ret;
error:
	dev_dbg(&priv->i2c->dev, "%s: failed=%d\n", __func__, ret);
	return ret;
}

static int tda10071_read_ucblocks(struct dvb_frontend *fe, u32 *ucblocks)
{
	struct tda10071_priv *priv = fe->demodulator_priv;
	int ret = 0;

	if (!priv->warm || !(priv->fe_status & FE_HAS_LOCK)) {
		*ucblocks = 0;
		goto error;
	}

	/* UCB is updated when BER is read. Assume BER is read anyway. */

	*ucblocks = priv->ucb;

	return ret;
error:
	dev_dbg(&priv->i2c->dev, "%s: failed=%d\n", __func__, ret);
	return ret;
}

static int tda10071_set_frontend(struct dvb_frontend *fe)
{
	struct tda10071_priv *priv = fe->demodulator_priv;
	struct tda10071_cmd cmd;
	struct dtv_frontend_properties *c = &fe->dtv_property_cache;
	int ret, i;
	u8 mode, rolloff, pilot, inversion, div;
	fe_modulation_t modulation;

	dev_dbg(&priv->i2c->dev,
			"%s: delivery_system=%d modulation=%d frequency=%d symbol_rate=%d inversion=%d pilot=%d rolloff=%d\n",
			__func__, c->delivery_system, c->modulation,
			c->frequency, c->symbol_rate, c->inversion, c->pilot,
			c->rolloff);

	priv->delivery_system = SYS_UNDEFINED;

	if (!priv->warm) {
		ret = -EFAULT;
		goto error;
	}

	switch (c->inversion) {
	case INVERSION_OFF:
		inversion = 1;
		break;
	case INVERSION_ON:
		inversion = 0;
		break;
	case INVERSION_AUTO:
		/* 2 = auto; try first on then off
		 * 3 = auto; try first off then on */
		inversion = 3;
		break;
	default:
		dev_dbg(&priv->i2c->dev, "%s: invalid inversion\n", __func__);
		ret = -EINVAL;
		goto error;
	}

	switch (c->delivery_system) {
	case SYS_DVBS:
		modulation = QPSK;
		rolloff = 0;
		pilot = 2;
		break;
	case SYS_DVBS2:
		modulation = c->modulation;

		switch (c->rolloff) {
		case ROLLOFF_20:
			rolloff = 2;
			break;
		case ROLLOFF_25:
			rolloff = 1;
			break;
		case ROLLOFF_35:
			rolloff = 0;
			break;
		case ROLLOFF_AUTO:
		default:
			dev_dbg(&priv->i2c->dev, "%s: invalid rolloff\n",
					__func__);
			ret = -EINVAL;
			goto error;
		}

		switch (c->pilot) {
		case PILOT_OFF:
			pilot = 0;
			break;
		case PILOT_ON:
			pilot = 1;
			break;
		case PILOT_AUTO:
			pilot = 2;
			break;
		default:
			dev_dbg(&priv->i2c->dev, "%s: invalid pilot\n",
					__func__);
			ret = -EINVAL;
			goto error;
		}
		break;
	default:
		dev_dbg(&priv->i2c->dev, "%s: invalid delivery_system\n",
				__func__);
		ret = -EINVAL;
		goto error;
	}

	for (i = 0, mode = 0xff; i < ARRAY_SIZE(TDA10071_MODCOD); i++) {
		if (c->delivery_system == TDA10071_MODCOD[i].delivery_system &&
			modulation == TDA10071_MODCOD[i].modulation &&
			c->fec_inner == TDA10071_MODCOD[i].fec) {
			mode = TDA10071_MODCOD[i].val;
			dev_dbg(&priv->i2c->dev, "%s: mode found=%02x\n",
					__func__, mode);
			break;
		}
	}

	if (mode == 0xff) {
		dev_dbg(&priv->i2c->dev, "%s: invalid parameter combination\n",
				__func__);
		ret = -EINVAL;
		goto error;
	}

	if (c->symbol_rate <= 5000000)
		div = 14;
	else
		div = 4;

	ret = tda10071_wr_reg(priv, 0x81, div);
	if (ret)
		goto error;

	ret = tda10071_wr_reg(priv, 0xe3, div);
	if (ret)
		goto error;

	cmd.args[0] = CMD_CHANGE_CHANNEL;
	cmd.args[1] = 0;
	cmd.args[2] = mode;
	cmd.args[3] = (c->frequency >> 16) & 0xff;
	cmd.args[4] = (c->frequency >>  8) & 0xff;
	cmd.args[5] = (c->frequency >>  0) & 0xff;
	cmd.args[6] = ((c->symbol_rate / 1000) >> 8) & 0xff;
	cmd.args[7] = ((c->symbol_rate / 1000) >> 0) & 0xff;
	cmd.args[8] = (tda10071_ops.info.frequency_tolerance >> 8) & 0xff;
	cmd.args[9] = (tda10071_ops.info.frequency_tolerance >> 0) & 0xff;
	cmd.args[10] = rolloff;
	cmd.args[11] = inversion;
	cmd.args[12] = pilot;
	cmd.args[13] = 0x00;
	cmd.args[14] = 0x00;
	cmd.len = 15;
	ret = tda10071_cmd_execute(priv, &cmd);
	if (ret)
		goto error;

	priv->delivery_system = c->delivery_system;

	return ret;
error:
	dev_dbg(&priv->i2c->dev, "%s: failed=%d\n", __func__, ret);
	return ret;
}

static int tda10071_get_frontend(struct dvb_frontend *fe)
{
	struct tda10071_priv *priv = fe->demodulator_priv;
	struct dtv_frontend_properties *c = &fe->dtv_property_cache;
	int ret, i;
	u8 buf[5], tmp;

	if (!priv->warm || !(priv->fe_status & FE_HAS_LOCK)) {
		ret = -EFAULT;
		goto error;
	}

	ret = tda10071_rd_regs(priv, 0x30, buf, 5);
	if (ret)
		goto error;

	tmp = buf[0] & 0x3f;
	for (i = 0; i < ARRAY_SIZE(TDA10071_MODCOD); i++) {
		if (tmp == TDA10071_MODCOD[i].val) {
			c->modulation = TDA10071_MODCOD[i].modulation;
			c->fec_inner = TDA10071_MODCOD[i].fec;
			c->delivery_system = TDA10071_MODCOD[i].delivery_system;
		}
	}

	switch ((buf[1] >> 0) & 0x01) {
	case 0:
		c->inversion = INVERSION_ON;
		break;
	case 1:
		c->inversion = INVERSION_OFF;
		break;
	}

	switch ((buf[1] >> 7) & 0x01) {
	case 0:
		c->pilot = PILOT_OFF;
		break;
	case 1:
		c->pilot = PILOT_ON;
		break;
	}

	c->frequency = (buf[2] << 16) | (buf[3] << 8) | (buf[4] << 0);

	ret = tda10071_rd_regs(priv, 0x52, buf, 3);
	if (ret)
		goto error;

	c->symbol_rate = ((buf[0] << 16) | (buf[1] << 8) | (buf[2] << 0)) * 1000;

	return ret;
error:
	dev_dbg(&priv->i2c->dev, "%s: failed=%d\n", __func__, ret);
	return ret;
}

static int tda10071_init(struct dvb_frontend *fe)
{
	struct tda10071_priv *priv = fe->demodulator_priv;
	struct tda10071_cmd cmd;
	int ret, i, len, remaining, fw_size;
	const struct firmware *fw;
	u8 *fw_file = TDA10071_FIRMWARE;
	u8 tmp, buf[4];
	struct tda10071_reg_val_mask tab[] = {
		{ 0xcd, 0x00, 0x07 },
		{ 0x80, 0x00, 0x02 },
		{ 0xcd, 0x00, 0xc0 },
		{ 0xce, 0x00, 0x1b },
		{ 0x9d, 0x00, 0x01 },
		{ 0x9d, 0x00, 0x02 },
		{ 0x9e, 0x00, 0x01 },
		{ 0x87, 0x00, 0x80 },
		{ 0xce, 0x00, 0x08 },
		{ 0xce, 0x00, 0x10 },
	};
	struct tda10071_reg_val_mask tab2[] = {
		{ 0xf1, 0x70, 0xff },
		{ 0x88, priv->cfg.pll_multiplier, 0x3f },
		{ 0x89, 0x00, 0x10 },
		{ 0x89, 0x10, 0x10 },
		{ 0xc0, 0x01, 0x01 },
		{ 0xc0, 0x00, 0x01 },
		{ 0xe0, 0xff, 0xff },
		{ 0xe0, 0x00, 0xff },
		{ 0x96, 0x1e, 0x7e },
		{ 0x8b, 0x08, 0x08 },
		{ 0x8b, 0x00, 0x08 },
		{ 0x8f, 0x1a, 0x7e },
		{ 0x8c, 0x68, 0xff },
		{ 0x8d, 0x08, 0xff },
		{ 0x8e, 0x4c, 0xff },
		{ 0x8f, 0x01, 0x01 },
		{ 0x8b, 0x04, 0x04 },
		{ 0x8b, 0x00, 0x04 },
		{ 0x87, 0x05, 0x07 },
		{ 0x80, 0x00, 0x20 },
		{ 0xc8, 0x01, 0xff },
		{ 0xb4, 0x47, 0xff },
		{ 0xb5, 0x9c, 0xff },
		{ 0xb6, 0x7d, 0xff },
		{ 0xba, 0x00, 0x03 },
		{ 0xb7, 0x47, 0xff },
		{ 0xb8, 0x9c, 0xff },
		{ 0xb9, 0x7d, 0xff },
		{ 0xba, 0x00, 0x0c },
		{ 0xc8, 0x00, 0xff },
		{ 0xcd, 0x00, 0x04 },
		{ 0xcd, 0x00, 0x20 },
		{ 0xe8, 0x02, 0xff },
		{ 0xcf, 0x20, 0xff },
		{ 0x9b, 0xd7, 0xff },
		{ 0x9a, 0x01, 0x03 },
		{ 0xa8, 0x05, 0x0f },
		{ 0xa8, 0x65, 0xf0 },
		{ 0xa6, 0xa0, 0xf0 },
		{ 0x9d, 0x50, 0xfc },
		{ 0x9e, 0x20, 0xe0 },
		{ 0xa3, 0x1c, 0x7c },
		{ 0xd5, 0x03, 0x03 },
	};

	if (priv->warm) {
		/* warm state - wake up device from sleep */

		for (i = 0; i < ARRAY_SIZE(tab); i++) {
			ret = tda10071_wr_reg_mask(priv, tab[i].reg,
				tab[i].val, tab[i].mask);
			if (ret)
				goto error;
		}

		cmd.args[0] = CMD_SET_SLEEP_MODE;
		cmd.args[1] = 0;
		cmd.args[2] = 0;
		cmd.len = 3;
		ret = tda10071_cmd_execute(priv, &cmd);
		if (ret)
			goto error;
	} else {
		/* cold state - try to download firmware */

		/* request the firmware, this will block and timeout */
		ret = request_firmware(&fw, fw_file, priv->i2c->dev.parent);
		if (ret) {
			dev_err(&priv->i2c->dev,
					"%s: did not find the firmware file. (%s) Please see linux/Documentation/dvb/ for more details on firmware-problems. (%d)\n",
					KBUILD_MODNAME, fw_file, ret);
			goto error;
		}

		/* init */
		for (i = 0; i < ARRAY_SIZE(tab2); i++) {
			ret = tda10071_wr_reg_mask(priv, tab2[i].reg,
				tab2[i].val, tab2[i].mask);
			if (ret)
				goto error_release_firmware;
		}

		/*  download firmware */
		ret = tda10071_wr_reg(priv, 0xe0, 0x7f);
		if (ret)
			goto error_release_firmware;

		ret = tda10071_wr_reg(priv, 0xf7, 0x81);
		if (ret)
			goto error_release_firmware;

		ret = tda10071_wr_reg(priv, 0xf8, 0x00);
		if (ret)
			goto error_release_firmware;

		ret = tda10071_wr_reg(priv, 0xf9, 0x00);
		if (ret)
			goto error_release_firmware;

		dev_info(&priv->i2c->dev,
				"%s: found a '%s' in cold state, will try to load a firmware\n",
				KBUILD_MODNAME, tda10071_ops.info.name);
		dev_info(&priv->i2c->dev,
				"%s: downloading firmware from file '%s'\n",
				KBUILD_MODNAME, fw_file);

		/* do not download last byte */
		fw_size = fw->size - 1;

		for (remaining = fw_size; remaining > 0;
			remaining -= (priv->cfg.i2c_wr_max - 1)) {
			len = remaining;
			if (len > (priv->cfg.i2c_wr_max - 1))
				len = (priv->cfg.i2c_wr_max - 1);

			ret = tda10071_wr_regs(priv, 0xfa,
				(u8 *) &fw->data[fw_size - remaining], len);
			if (ret) {
				dev_err(&priv->i2c->dev,
						"%s: firmware download failed=%d\n",
						KBUILD_MODNAME, ret);
				goto error_release_firmware;
			}
		}
		release_firmware(fw);

		ret = tda10071_wr_reg(priv, 0xf7, 0x0c);
		if (ret)
			goto error;

		ret = tda10071_wr_reg(priv, 0xe0, 0x00);
		if (ret)
			goto error;

		/* wait firmware start */
		msleep(250);

		/* firmware status */
		ret = tda10071_rd_reg(priv, 0x51, &tmp);
		if (ret)
			goto error;

		if (tmp) {
			dev_info(&priv->i2c->dev, "%s: firmware did not run\n",
					KBUILD_MODNAME);
			ret = -EFAULT;
			goto error;
		} else {
			priv->warm = true;
		}

		cmd.args[0] = CMD_GET_FW_VERSION;
		cmd.len = 1;
		ret = tda10071_cmd_execute(priv, &cmd);
		if (ret)
			goto error;

		ret = tda10071_rd_regs(priv, cmd.len, buf, 4);
		if (ret)
			goto error;

		dev_info(&priv->i2c->dev, "%s: firmware version %d.%d.%d.%d\n",
				KBUILD_MODNAME, buf[0], buf[1], buf[2], buf[3]);
		dev_info(&priv->i2c->dev, "%s: found a '%s' in warm state\n",
				KBUILD_MODNAME, tda10071_ops.info.name);

		ret = tda10071_rd_regs(priv, 0x81, buf, 2);
		if (ret)
			goto error;

		cmd.args[0] = CMD_DEMOD_INIT;
		cmd.args[1] = ((priv->cfg.xtal / 1000) >> 8) & 0xff;
		cmd.args[2] = ((priv->cfg.xtal / 1000) >> 0) & 0xff;
		cmd.args[3] = buf[0];
		cmd.args[4] = buf[1];
		cmd.args[5] = priv->cfg.pll_multiplier;
		cmd.args[6] = priv->cfg.spec_inv;
		cmd.args[7] = 0x00;
		cmd.len = 8;
		ret = tda10071_cmd_execute(priv, &cmd);
		if (ret)
			goto error;

		if (priv->cfg.tuner_i2c_addr)
			tmp = priv->cfg.tuner_i2c_addr;
		else
			tmp = 0x14;

		cmd.args[0] = CMD_TUNER_INIT;
		cmd.args[1] = 0x00;
		cmd.args[2] = 0x00;
		cmd.args[3] = 0x00;
		cmd.args[4] = 0x00;
		cmd.args[5] = tmp;
		cmd.args[6] = 0x00;
		cmd.args[7] = 0x03;
		cmd.args[8] = 0x02;
		cmd.args[9] = 0x02;
		cmd.args[10] = 0x00;
		cmd.args[11] = 0x00;
		cmd.args[12] = 0x00;
		cmd.args[13] = 0x00;
		cmd.args[14] = 0x00;
		cmd.len = 15;
		ret = tda10071_cmd_execute(priv, &cmd);
		if (ret)
			goto error;

		cmd.args[0] = CMD_MPEG_CONFIG;
		cmd.args[1] = 0;
		cmd.args[2] = priv->cfg.ts_mode;
		cmd.args[3] = 0x00;
		cmd.args[4] = 0x04;
		cmd.args[5] = 0x00;
		cmd.len = 6;
		ret = tda10071_cmd_execute(priv, &cmd);
		if (ret)
			goto error;

		ret = tda10071_wr_reg_mask(priv, 0xf0, 0x01, 0x01);
		if (ret)
			goto error;

		cmd.args[0] = CMD_LNB_CONFIG;
		cmd.args[1] = 0;
		cmd.args[2] = 150;
		cmd.args[3] = 3;
		cmd.args[4] = 22;
		cmd.args[5] = 1;
		cmd.args[6] = 1;
		cmd.args[7] = 30;
		cmd.args[8] = 30;
		cmd.args[9] = 30;
		cmd.args[10] = 30;
		cmd.len = 11;
		ret = tda10071_cmd_execute(priv, &cmd);
		if (ret)
			goto error;

		cmd.args[0] = CMD_BER_CONTROL;
		cmd.args[1] = 0;
		cmd.args[2] = 14;
		cmd.args[3] = 14;
		cmd.len = 4;
		ret = tda10071_cmd_execute(priv, &cmd);
		if (ret)
			goto error;
	}

	return ret;
error_release_firmware:
	release_firmware(fw);
error:
	dev_dbg(&priv->i2c->dev, "%s: failed=%d\n", __func__, ret);
	return ret;
}

static int tda10071_sleep(struct dvb_frontend *fe)
{
	struct tda10071_priv *priv = fe->demodulator_priv;
	struct tda10071_cmd cmd;
	int ret, i;
	struct tda10071_reg_val_mask tab[] = {
		{ 0xcd, 0x07, 0x07 },
		{ 0x80, 0x02, 0x02 },
		{ 0xcd, 0xc0, 0xc0 },
		{ 0xce, 0x1b, 0x1b },
		{ 0x9d, 0x01, 0x01 },
		{ 0x9d, 0x02, 0x02 },
		{ 0x9e, 0x01, 0x01 },
		{ 0x87, 0x80, 0x80 },
		{ 0xce, 0x08, 0x08 },
		{ 0xce, 0x10, 0x10 },
	};

	if (!priv->warm) {
		ret = -EFAULT;
		goto error;
	}

	cmd.args[0] = CMD_SET_SLEEP_MODE;
	cmd.args[1] = 0;
	cmd.args[2] = 1;
	cmd.len = 3;
	ret = tda10071_cmd_execute(priv, &cmd);
	if (ret)
		goto error;

	for (i = 0; i < ARRAY_SIZE(tab); i++) {
		ret = tda10071_wr_reg_mask(priv, tab[i].reg, tab[i].val,
			tab[i].mask);
		if (ret)
			goto error;
	}

	return ret;
error:
	dev_dbg(&priv->i2c->dev, "%s: failed=%d\n", __func__, ret);
	return ret;
}

static int tda10071_get_tune_settings(struct dvb_frontend *fe,
	struct dvb_frontend_tune_settings *s)
{
	s->min_delay_ms = 8000;
	s->step_size = 0;
	s->max_drift = 0;

	return 0;
}

static void tda10071_release(struct dvb_frontend *fe)
{
	struct tda10071_priv *priv = fe->demodulator_priv;
	kfree(priv);
}

struct dvb_frontend *tda10071_attach(const struct tda10071_config *config,
	struct i2c_adapter *i2c)
{
	int ret;
	struct tda10071_priv *priv = NULL;
	u8 tmp;

	/* allocate memory for the internal priv */
	priv = kzalloc(sizeof(struct tda10071_priv), GFP_KERNEL);
	if (priv == NULL) {
		ret = -ENOMEM;
		goto error;
	}

	/* make sure demod i2c address is specified */
	if (!config->demod_i2c_addr) {
		dev_dbg(&i2c->dev, "%s: invalid demod i2c address\n", __func__);
		ret = -EINVAL;
		goto error;
	}

	/* make sure tuner i2c address is specified */
	if (!config->tuner_i2c_addr) {
		dev_dbg(&i2c->dev, "%s: invalid tuner i2c address\n", __func__);
		ret = -EINVAL;
		goto error;
	}

	/* setup the priv */
	priv->i2c = i2c;
	memcpy(&priv->cfg, config, sizeof(struct tda10071_config));

	/* chip ID */
	ret = tda10071_rd_reg(priv, 0xff, &tmp);
	if (ret || tmp != 0x0f)
		goto error;

	/* chip type */
	ret = tda10071_rd_reg(priv, 0xdd, &tmp);
	if (ret || tmp != 0x00)
		goto error;

	/* chip version */
	ret = tda10071_rd_reg(priv, 0xfe, &tmp);
	if (ret || tmp != 0x01)
		goto error;

	/* create dvb_frontend */
	memcpy(&priv->fe.ops, &tda10071_ops, sizeof(struct dvb_frontend_ops));
	priv->fe.demodulator_priv = priv;

	return &priv->fe;
error:
	dev_dbg(&i2c->dev, "%s: failed=%d\n", __func__, ret);
	kfree(priv);
	return NULL;
}
EXPORT_SYMBOL(tda10071_attach);

static struct dvb_frontend_ops tda10071_ops = {
	.delsys = { SYS_DVBS, SYS_DVBS2 },
	.info = {
		.name = "NXP TDA10071",
		.frequency_min = 950000,
		.frequency_max = 2150000,
		.frequency_tolerance = 5000,
		.symbol_rate_min = 1000000,
		.symbol_rate_max = 45000000,
		.caps = FE_CAN_INVERSION_AUTO |
			FE_CAN_FEC_1_2 |
			FE_CAN_FEC_2_3 |
			FE_CAN_FEC_3_4 |
			FE_CAN_FEC_4_5 |
			FE_CAN_FEC_5_6 |
			FE_CAN_FEC_6_7 |
			FE_CAN_FEC_7_8 |
			FE_CAN_FEC_8_9 |
			FE_CAN_FEC_AUTO |
			FE_CAN_QPSK |
			FE_CAN_RECOVER |
			FE_CAN_2G_MODULATION
	},

	.release = tda10071_release,

	.get_tune_settings = tda10071_get_tune_settings,

	.init = tda10071_init,
	.sleep = tda10071_sleep,

	.set_frontend = tda10071_set_frontend,
	.get_frontend = tda10071_get_frontend,

	.read_status = tda10071_read_status,
	.read_snr = tda10071_read_snr,
	.read_signal_strength = tda10071_read_signal_strength,
	.read_ber = tda10071_read_ber,
	.read_ucblocks = tda10071_read_ucblocks,

	.diseqc_send_master_cmd = tda10071_diseqc_send_master_cmd,
	.diseqc_recv_slave_reply = tda10071_diseqc_recv_slave_reply,
	.diseqc_send_burst = tda10071_diseqc_send_burst,

	.set_tone = tda10071_set_tone,
	.set_voltage = tda10071_set_voltage,
};

MODULE_AUTHOR("Antti Palosaari <crope@iki.fi>");
MODULE_DESCRIPTION("NXP TDA10071 DVB-S/S2 demodulator driver");
MODULE_LICENSE("GPL");
MODULE_FIRMWARE(TDA10071_FIRMWARE);
