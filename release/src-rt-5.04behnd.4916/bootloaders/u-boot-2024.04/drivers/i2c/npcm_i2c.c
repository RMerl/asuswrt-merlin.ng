// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2021 Nuvoton Technology Corp.
 */

#include <clk.h>
#include <dm.h>
#include <i2c.h>
#include <asm/io.h>
#include <linux/iopoll.h>
#include <asm/arch/gcr.h>

#define I2C_FREQ_100K			100000
#define NPCM_I2C_TIMEOUT_MS		10
#define NPCM7XX_I2CSEGCTL_INIT_VAL	0x0333F000
#define NPCM8XX_I2CSEGCTL_INIT_VAL	0x9333F000

/* SCLFRQ min/max field values  */
#define SCLFRQ_MIN		10
#define SCLFRQ_MAX		511

/* SMBCTL1 */
#define SMBCTL1_START		BIT(0)
#define SMBCTL1_STOP		BIT(1)
#define SMBCTL1_INTEN		BIT(2)
#define SMBCTL1_ACK		BIT(4)
#define SMBCTL1_STASTRE		BIT(7)

/* SMBCTL2 */
#define SMBCTL2_ENABLE		BIT(0)

/* SMBCTL3 */
#define SMBCTL3_SCL_LVL		BIT(7)
#define SMBCTL3_SDA_LVL		BIT(6)

/* SMBCST */
#define SMBCST_BB		BIT(1)
#define SMBCST_TGSCL		BIT(5)

/* SMBST */
#define SMBST_XMIT		BIT(0)
#define SMBST_MASTER		BIT(1)
#define SMBST_STASTR		BIT(3)
#define SMBST_NEGACK		BIT(4)
#define SMBST_BER		BIT(5)
#define SMBST_SDAST		BIT(6)

/* SMBCST3 in bank0 */
#define SMBCST3_EO_BUSY		BIT(7)

/* SMBFIF_CTS in bank1 */
#define SMBFIF_CTS_CLR_FIFO	BIT(6)

#define SMBFIF_CTL_FIFO_EN	BIT(4)
#define SMBCTL3_BNK_SEL		BIT(5)

enum {
	I2C_ERR_NACK = 1,
	I2C_ERR_BER,
	I2C_ERR_TIMEOUT,
};

struct smb_bank0_regs {
	u8 addr3;
	u8 addr7;
	u8 addr4;
	u8 addr8;
	u16 addr5;
	u16 addr6;
	u8 cst2;
	u8 cst3;
	u8 ctl4;
	u8 ctl5;
	u8 scllt;
	u8 fif_ctl;
	u8 sclht;
};

struct smb_bank1_regs {
	u8 fif_cts;
	u8 fair_per;
	u16 txf_ctl;
	u32 t_out;
	u8 cst2;
	u8 cst3;
	u16 txf_sts;
	u16 rxf_sts;
	u8 rxf_ctl;
};

struct npcm_i2c_regs {
	u16 sda;
	u16 st;
	u16 cst;
	u16 ctl1;
	u16 addr;
	u16 ctl2;
	u16 addr2;
	u16 ctl3;
	union {
		struct smb_bank0_regs bank0;
		struct smb_bank1_regs bank1;
	};

};

struct npcm_i2c_bus {
	struct npcm_i2c_regs *reg;
	int num;
	u32 apb_clk;
	u32 freq;
	bool started;
};

static void npcm_dump_regs(struct npcm_i2c_bus *bus)
{
	struct npcm_i2c_regs *reg = bus->reg;

	printf("\n");
	printf("SMBST=0x%x\n", readb(&reg->st));
	printf("SMBCST=0x%x\n", readb(&reg->cst));
	printf("SMBCTL1=0x%x\n", readb(&reg->ctl1));
	printf("\n");
}

static int npcm_i2c_check_sda(struct npcm_i2c_bus *bus)
{
	struct npcm_i2c_regs *reg = bus->reg;
	ulong start_time;
	int err = I2C_ERR_TIMEOUT;
	u8 val;

	start_time = get_timer(0);
	/* wait SDAST to be 1 */
	while (get_timer(start_time) < NPCM_I2C_TIMEOUT_MS) {
		val = readb(&reg->st);
		if (val & SMBST_NEGACK) {
			err = I2C_ERR_NACK;
			break;
		}
		if (val & SMBST_BER) {
			err = I2C_ERR_BER;
			break;
		}
		if (val & SMBST_SDAST) {
			err = 0;
			break;
		}
	}

	if (err)
		printf("%s: err %d\n", __func__, err);

	return err;
}

static int npcm_i2c_send_start(struct npcm_i2c_bus *bus)
{
	struct npcm_i2c_regs *reg = bus->reg;
	ulong start_time;
	int err = I2C_ERR_TIMEOUT;

	/* Generate START condition */
	setbits_8(&reg->ctl1, SMBCTL1_START);

	start_time = get_timer(0);
	while (get_timer(start_time) < NPCM_I2C_TIMEOUT_MS) {
		if (readb(&reg->st) & SMBST_BER)
			return I2C_ERR_BER;
		if (readb(&reg->st) & SMBST_MASTER) {
			err = 0;
			break;
		}
	}
	bus->started = true;

	return err;
}

static int npcm_i2c_send_stop(struct npcm_i2c_bus *bus, bool wait)
{
	struct npcm_i2c_regs *reg = bus->reg;
	ulong start_time;
	int err = I2C_ERR_TIMEOUT;

	setbits_8(&reg->ctl1, SMBCTL1_STOP);

	/* Clear NEGACK, STASTR and BER bits  */
	writeb(SMBST_STASTR | SMBST_NEGACK | SMBST_BER, &reg->st);

	bus->started = false;

	if (!wait)
		return 0;

	start_time = get_timer(0);
	while (get_timer(start_time) < NPCM_I2C_TIMEOUT_MS) {
		if ((readb(&reg->ctl1) & SMBCTL1_STOP) == 0) {
			err = 0;
			break;
		}
	}
	if (err) {
		printf("%s: err %d\n", __func__, err);
		npcm_dump_regs(bus);
	}

	return err;
}

static void npcm_i2c_reset(struct npcm_i2c_bus *bus)
{
	struct npcm_i2c_regs *reg = bus->reg;

	debug("%s: module %d\n", __func__, bus->num);
	/* disable & enable SMB moudle */
	clrbits_8(&reg->ctl2, SMBCTL2_ENABLE);
	setbits_8(&reg->ctl2, SMBCTL2_ENABLE);

	/* clear BB and status */
	writeb(SMBCST_BB, &reg->cst);
	writeb(0xff, &reg->st);

	/* select bank 1 */
	setbits_8(&reg->ctl3, SMBCTL3_BNK_SEL);
	/* Clear all fifo bits */
	writeb(SMBFIF_CTS_CLR_FIFO, &reg->bank1.fif_cts);

	/* select bank 0 */
	clrbits_8(&reg->ctl3, SMBCTL3_BNK_SEL);
	/* clear EOB bit */
	writeb(SMBCST3_EO_BUSY, &reg->bank0.cst3);
	/* single byte mode */
	clrbits_8(&reg->bank0.fif_ctl, SMBFIF_CTL_FIFO_EN);

	/* set POLL mode */
	writeb(0, &reg->ctl1);
}

static void npcm_i2c_recovery(struct npcm_i2c_bus *bus, u32 addr)
{
	u8 val;
	int iter = 27;
	struct npcm_i2c_regs *reg = bus->reg;
	int err;

	val = readb(&reg->ctl3);
	/* Skip recovery, bus not stucked */
	if ((val & SMBCTL3_SCL_LVL) && (val & SMBCTL3_SDA_LVL))
		return;

	printf("Performing I2C bus %d recovery...\n", bus->num);
	/* SCL/SDA are not releaed, perform recovery */
	while (1) {
		/* toggle SCL line */
		writeb(SMBCST_TGSCL, &reg->cst);

		udelay(20);
		val = readb(&reg->ctl3);
		if (val & SMBCTL3_SDA_LVL)
			break;
		if (iter-- == 0)
			break;
	}

	if (val & SMBCTL3_SDA_LVL) {
		writeb((u8)((addr << 1) & 0xff), &reg->sda);
		err = npcm_i2c_send_start(bus);
		if (!err) {
			udelay(20);
			npcm_i2c_send_stop(bus, false);
			udelay(200);
			printf("I2C bus %d recovery completed\n",
			       bus->num);
		} else {
			printf("%s: send START err %d\n", __func__, err);
		}
	} else {
		printf("Fail to recover I2C bus %d\n", bus->num);
	}
	npcm_i2c_reset(bus);
}

static int npcm_i2c_send_address(struct npcm_i2c_bus *bus, u8 addr,
				 bool stall)
{
	struct npcm_i2c_regs *reg = bus->reg;
	ulong start_time;
	u8 val;

	/* Stall After Start Enable */
	if (stall)
		setbits_8(&reg->ctl1, SMBCTL1_STASTRE);

	writeb(addr, &reg->sda);
	if (stall) {
		start_time = get_timer(0);
		while (get_timer(start_time) < NPCM_I2C_TIMEOUT_MS) {
			if (readb(&reg->st) & SMBST_STASTR)
				break;

			if (readb(&reg->st) & SMBST_BER) {
				clrbits_8(&reg->ctl1, SMBCTL1_STASTRE);
				return I2C_ERR_BER;
			}
		}
	}

	/* check ACK */
	val = readb(&reg->st);
	if (val & SMBST_NEGACK) {
		debug("NACK on addr 0x%x\n", addr >> 1);
		/* After a Stop condition, writing 1 to NEGACK clears it */
		return I2C_ERR_NACK;
	}
	if (val & SMBST_BER)
		return I2C_ERR_BER;

	return 0;
}

static int npcm_i2c_read_bytes(struct npcm_i2c_bus *bus, u8 *data, int len)
{
	struct npcm_i2c_regs *reg = bus->reg;
	u8 val;
	int i;
	int err = 0;

	if (len == 1) {
		/* bus should be stalled before receiving last byte */
		setbits_8(&reg->ctl1, SMBCTL1_ACK);

		/* clear STASTRE if it is set */
		if (readb(&reg->ctl1) & SMBCTL1_STASTRE) {
			writeb(SMBST_STASTR, &reg->st);
			clrbits_8(&reg->ctl1, SMBCTL1_STASTRE);
		}
		npcm_i2c_check_sda(bus);
		npcm_i2c_send_stop(bus, false);
		*data = readb(&reg->sda);
		/* this must be done to generate STOP condition */
		writeb(SMBST_NEGACK, &reg->st);
	} else {
		for (i = 0; i < len; i++) {
			/*
			 * When NEGACK bit is set to 1 after the transmission of a byte,
			 * SDAST is not set to 1.
			 */
			if (i != (len - 1)) {
				err = npcm_i2c_check_sda(bus);
			} else {
				err = readb_poll_timeout(&reg->ctl1, val,
							 !(val & SMBCTL1_ACK), 100000);
				if (err) {
					printf("wait nack timeout\n");
					err = I2C_ERR_TIMEOUT;
					npcm_dump_regs(bus);
				}
			}
			if (err && err != I2C_ERR_TIMEOUT)
				break;
			if (i == (len - 2)) {
				/* set NACK before last byte */
				setbits_8(&reg->ctl1, SMBCTL1_ACK);
			}
			if (i == (len - 1)) {
				/* last byte, send STOP condition */
				npcm_i2c_send_stop(bus, false);
				*data = readb(&reg->sda);
				writeb(SMBST_NEGACK, &reg->st);
				break;
			}
			*data = readb(&reg->sda);
			data++;
		}
	}

	return err;
}

static int npcm_i2c_send_bytes(struct npcm_i2c_bus *bus, u8 *data, int len)
{
	struct npcm_i2c_regs *reg = bus->reg;
	u8 val;
	int i;
	int err = 0;

	val = readb(&reg->st);
	if (val & SMBST_NEGACK)
		return I2C_ERR_NACK;
	else if (val & SMBST_BER)
		return I2C_ERR_BER;

	/* clear STASTRE if it is set */
	if (readb(&reg->ctl1) & SMBCTL1_STASTRE)
		clrbits_8(&reg->ctl1, SMBCTL1_STASTRE);

	for (i = 0; i < len; i++) {
		err = npcm_i2c_check_sda(bus);
		if (err)
			break;
		writeb(*data, &reg->sda);
		data++;
	}
	npcm_i2c_check_sda(bus);

	return err;
}

static int npcm_i2c_read(struct npcm_i2c_bus *bus, u32 addr, u8 *data,
			 u32 len)
{
	struct npcm_i2c_regs *reg = bus->reg;
	int err;
	bool stall;

	if (len <= 0)
		return -EINVAL;

	/* send START condition */
	err = npcm_i2c_send_start(bus);
	if (err) {
		debug("%s: send START err %d\n", __func__, err);
		return err;
	}

	stall = (len == 1) ? true : false;
	/* send address byte */
	err = npcm_i2c_send_address(bus, (u8)(addr << 1) | 0x1, stall);

	if (!err && len)
		npcm_i2c_read_bytes(bus, data, len);

	if (err == I2C_ERR_NACK) {
		/* clear NACK */
		writeb(SMBST_NEGACK, &reg->st);
	}

	if (err)
		debug("%s: err %d\n", __func__, err);

	return err;
}

static int npcm_i2c_write(struct npcm_i2c_bus *bus, u32 addr, u8 *data,
			  u32 len)
{
	struct npcm_i2c_regs *reg = bus->reg;
	int err;
	bool stall;

	/* send START condition */
	err = npcm_i2c_send_start(bus);
	if (err) {
		debug("%s: send START err %d\n", __func__, err);
		return err;
	}

	stall = (len == 0) ? true : false;
	/* send address byte */
	err = npcm_i2c_send_address(bus, (u8)(addr << 1), stall);

	if (!err && len)
		err = npcm_i2c_send_bytes(bus, data, len);

	/* clear STASTRE if it is set */
	if (stall)
		clrbits_8(&reg->ctl1, SMBCTL1_STASTRE);

	if (err)
		debug("%s: err %d\n", __func__, err);

	return err;
}

static int npcm_i2c_xfer(struct udevice *dev,
			 struct i2c_msg *msg, int nmsgs)
{
	struct npcm_i2c_bus *bus = dev_get_priv(dev);
	struct npcm_i2c_regs *reg = bus->reg;
	int ret = 0, err = 0;

	if (nmsgs < 1 || nmsgs > 2) {
		printf("%s: commands not support\n", __func__);
		return -EREMOTEIO;
	}
	/* clear ST register */
	writeb(0xFF, &reg->st);

	for ( ; nmsgs > 0; nmsgs--, msg++) {
		if (msg->flags & I2C_M_RD)
			err = npcm_i2c_read(bus, msg->addr, msg->buf,
					    msg->len);
		else
			err = npcm_i2c_write(bus, msg->addr, msg->buf,
					     msg->len);
		if (err) {
			debug("i2c_xfer: error %d\n", err);
			ret = -EREMOTEIO;
			break;
		}
	}

	if (bus->started)
		npcm_i2c_send_stop(bus, true);

	if (err)
		npcm_i2c_recovery(bus, msg->addr);

	return ret;
}

static int npcm_i2c_init_clk(struct npcm_i2c_bus *bus, u32 bus_freq)
{
	struct npcm_i2c_regs *reg = bus->reg;
	u32 freq = bus->apb_clk;
	u32 sclfrq;
	u8 hldt, val;

	/* SCLFRQ = T(SCL)/4/T(CLK) = FREQ(CLK)/4/FREQ(SCL) */
	sclfrq = freq / (bus_freq * 4);
	if (sclfrq < SCLFRQ_MIN || sclfrq > SCLFRQ_MAX)
		return -EINVAL;

	if (freq >= 40000000)
		hldt = 17;
	else if (freq >= 12500000)
		hldt = 15;
	else
		hldt = 7;

	val = readb(&reg->ctl2) & 0x1;
	val |= (sclfrq & 0x7F) << 1;
	writeb(val, &reg->ctl2);

	/* clear 400K_MODE bit */
	val = readb(&reg->ctl3) & 0xc;
	val |= (sclfrq >> 7) & 0x3;
	writeb(val, &reg->ctl3);

	writeb(hldt, &reg->bank0.ctl4);

	return 0;
}

static int npcm_i2c_set_bus_speed(struct udevice *dev,
				  unsigned int speed)
{
	struct npcm_i2c_bus *bus = dev_get_priv(dev);

	return npcm_i2c_init_clk(bus, speed);
}

static int npcm_i2c_probe(struct udevice *dev)
{
	struct npcm_i2c_bus *bus = dev_get_priv(dev);
	struct npcm_gcr *gcr = (struct npcm_gcr *)NPCM_GCR_BA;
	struct npcm_i2c_regs *reg;
	u32 i2csegctl_val = dev_get_driver_data(dev);
	struct clk clk;
	int ret;

	ret = clk_get_by_index(dev, 0, &clk);
	if (ret) {
		printf("%s: ret %d\n", __func__, ret);
		return ret;
	}
	bus->apb_clk = clk_get_rate(&clk);
	if (bus->apb_clk <= 0) {
		printf("%s: fail to get rate\n", __func__);
		return -EINVAL;
	}

	bus->num = dev->seq_;
	bus->reg = dev_read_addr_ptr(dev);
	bus->freq = dev_read_u32_default(dev, "clock-frequency", 100000);
	bus->started = false;
	reg = bus->reg;

	if (npcm_i2c_init_clk(bus, bus->freq)) {
		printf("%s: init_clk failed\n", __func__);
		return -EINVAL;
	}

	/* set initial i2csegctl value */
	writel(i2csegctl_val, &gcr->i2csegctl);

	/* enable SMB module */
	setbits_8(&reg->ctl2, SMBCTL2_ENABLE);

	/* select register bank 0 */
	clrbits_8(&reg->ctl3, SMBCTL3_BNK_SEL);

	/* single byte mode */
	clrbits_8(&reg->bank0.fif_ctl, SMBFIF_CTL_FIFO_EN);

	/* set POLL mode */
	writeb(0, &reg->ctl1);

	printf("I2C bus %d ready. speed=%d, base=0x%x, apb=%u\n",
	       bus->num, bus->freq, (u32)(uintptr_t)bus->reg, bus->apb_clk);

	return 0;
}

static const struct dm_i2c_ops nuvoton_i2c_ops = {
	.xfer		    = npcm_i2c_xfer,
	.set_bus_speed	= npcm_i2c_set_bus_speed,
};

static const struct udevice_id nuvoton_i2c_of_match[] = {
	{ .compatible = "nuvoton,npcm845-i2c",  .data = NPCM8XX_I2CSEGCTL_INIT_VAL},
	{ .compatible = "nuvoton,npcm750-i2c",  .data = NPCM7XX_I2CSEGCTL_INIT_VAL},
	{}
};

U_BOOT_DRIVER(npcm_i2c_bus) = {
	.name = "npcm-i2c",
	.id = UCLASS_I2C,
	.of_match = nuvoton_i2c_of_match,
	.probe = npcm_i2c_probe,
	.priv_auto = sizeof(struct npcm_i2c_bus),
	.ops = &nuvoton_i2c_ops,
};
