// SPDX-License-Identifier:    GPL-2.0
/*
 * Copyright (C) 2018 Marvell International Ltd.
 */

#include <clk.h>
#include <dm.h>
#include <i2c.h>
#include <time.h>
#include <asm/io.h>
#include <linux/bitfield.h>
#include <linux/compat.h>
#include <linux/delay.h>

#define TWSI_SW_TWSI		0x00
#define TWSI_TWSI_SW		0x08
#define TWSI_INT		0x10
#define TWSI_SW_TWSI_EXT	0x18

#define TWSI_SW_DATA_MASK	GENMASK_ULL(31, 0)
#define TWSI_SW_EOP_IA_MASK	GENMASK_ULL(34, 32)
#define TWSI_SW_IA_MASK		GENMASK_ULL(39, 35)
#define TWSI_SW_ADDR_MASK	GENMASK_ULL(49, 40)
#define TWSI_SW_SCR_MASK	GENMASK_ULL(51, 50)
#define TWSI_SW_SIZE_MASK	GENMASK_ULL(54, 52)
#define TWSI_SW_SOVR		BIT_ULL(55)
#define TWSI_SW_R		BIT_ULL(56)
#define TWSI_SW_OP_MASK		GENMASK_ULL(60, 57)
#define TWSI_SW_EIA		GENMASK_ULL(61)
#define TWSI_SW_SLONLY		BIT_ULL(62)
#define TWSI_SW_V		BIT_ULL(63)

#define TWSI_INT_SDA_OVR	BIT_ULL(8)
#define TWSI_INT_SCL_OVR	BIT_ULL(9)
#define TWSI_INT_SDA		BIT_ULL(10)
#define TWSI_INT_SCL		BIT_ULL(11)

enum {
	TWSI_OP_WRITE	= 0,
	TWSI_OP_READ	= 1,
};

enum {
	TWSI_EOP_SLAVE_ADDR = 0,
	TWSI_EOP_CLK_CTL = 3,
	TWSI_SW_EOP_IA   = 6,
};

enum {
	TWSI_SLAVEADD     = 0,
	TWSI_DATA         = 1,
	TWSI_CTL          = 2,
	TWSI_CLKCTL       = 3,
	TWSI_STAT         = 3,
	TWSI_SLAVEADD_EXT = 4,
	TWSI_RST          = 7,
};

enum {
	TWSI_CTL_AAK	= BIT(2),
	TWSI_CTL_IFLG	= BIT(3),
	TWSI_CTL_STP	= BIT(4),
	TWSI_CTL_STA	= BIT(5),
	TWSI_CTL_ENAB	= BIT(6),
	TWSI_CTL_CE	= BIT(7),
};

/*
 * Internal errors. When debugging is enabled, the driver will report the
 * error number and the user / developer can check the table below for the
 * detailed error description.
 */
enum {
	/** Bus error */
	TWSI_STAT_BUS_ERROR		= 0x00,
	/** Start condition transmitted */
	TWSI_STAT_START			= 0x08,
	/** Repeat start condition transmitted */
	TWSI_STAT_RSTART		= 0x10,
	/** Address + write bit transmitted, ACK received */
	TWSI_STAT_TXADDR_ACK		= 0x18,
	/** Address + write bit transmitted, /ACK received */
	TWSI_STAT_TXADDR_NAK		= 0x20,
	/** Data byte transmitted in master mode, ACK received */
	TWSI_STAT_TXDATA_ACK		= 0x28,
	/** Data byte transmitted in master mode, ACK received */
	TWSI_STAT_TXDATA_NAK		= 0x30,
	/** Arbitration lost in address or data byte */
	TWSI_STAT_TX_ARB_LOST		= 0x38,
	/** Address + read bit transmitted, ACK received */
	TWSI_STAT_RXADDR_ACK		= 0x40,
	/** Address + read bit transmitted, /ACK received */
	TWSI_STAT_RXADDR_NAK		= 0x48,
	/** Data byte received in master mode, ACK transmitted */
	TWSI_STAT_RXDATA_ACK_SENT	= 0x50,
	/** Data byte received, NACK transmitted */
	TWSI_STAT_RXDATA_NAK_SENT	= 0x58,
	/** Slave address received, sent ACK */
	TWSI_STAT_SLAVE_RXADDR_ACK	= 0x60,
	/**
	 * Arbitration lost in address as master, slave address + write bit
	 * received, ACK transmitted
	 */
	TWSI_STAT_TX_ACK_ARB_LOST	= 0x68,
	/** General call address received, ACK transmitted */
	TWSI_STAT_RX_GEN_ADDR_ACK	= 0x70,
	/**
	 * Arbitration lost in address as master, general call address
	 * received, ACK transmitted
	 */
	TWSI_STAT_RX_GEN_ADDR_ARB_LOST	= 0x78,
	/** Data byte received after slave address received, ACK transmitted */
	TWSI_STAT_SLAVE_RXDATA_ACK	= 0x80,
	/** Data byte received after slave address received, /ACK transmitted */
	TWSI_STAT_SLAVE_RXDATA_NAK	= 0x88,
	/**
	 * Data byte received after general call address received, ACK
	 * transmitted
	 */
	TWSI_STAT_GEN_RXADDR_ACK	= 0x90,
	/**
	 * Data byte received after general call address received, /ACK
	 * transmitted
	 */
	TWSI_STAT_GEN_RXADDR_NAK	= 0x98,
	/** STOP or repeated START condition received in slave mode */
	TWSI_STAT_STOP_MULTI_START	= 0xa0,
	/** Slave address + read bit received, ACK transmitted */
	TWSI_STAT_SLAVE_RXADDR2_ACK	= 0xa8,
	/**
	 * Arbitration lost in address as master, slave address + read bit
	 * received, ACK transmitted
	 */
	TWSI_STAT_RXDATA_ACK_ARB_LOST	= 0xb0,
	/** Data byte transmitted in slave mode, ACK received */
	TWSI_STAT_SLAVE_TXDATA_ACK	= 0xb8,
	/** Data byte transmitted in slave mode, /ACK received */
	TWSI_STAT_SLAVE_TXDATA_NAK	= 0xc0,
	/** Last byte transmitted in slave mode, ACK received */
	TWSI_STAT_SLAVE_TXDATA_END_ACK	= 0xc8,
	/** Second address byte + write bit transmitted, ACK received */
	TWSI_STAT_TXADDR2DATA_ACK	= 0xd0,
	/** Second address byte + write bit transmitted, /ACK received */
	TWSI_STAT_TXADDR2DATA_NAK	= 0xd8,
	/** No relevant status information */
	TWSI_STAT_IDLE			= 0xf8
};

#define CFG_SYS_I2C_OCTEON_SLAVE_ADDR	0x77

enum {
	PROBE_PCI = 0,		/* PCI based probing */
	PROBE_DT,		/* DT based probing */
};

enum {
	CLK_METHOD_OCTEON = 0,
	CLK_METHOD_OCTEONTX2,
};

/**
 * struct octeon_i2c_data - SoC specific data of this driver
 *
 * @probe:	Probing of this SoC (DT vs PCI)
 * @reg_offs:	Register offset
 * @thp:	THP define for divider calculation
 * @clk_method:	Clock calculation method
 */
struct octeon_i2c_data {
	int probe;
	u32 reg_offs;
	int thp;
	int clk_method;
};

/**
 * struct octeon_twsi - Private data of this driver
 *
 * @base:	Base address of i2c registers
 * @data:	Pointer to SoC specific data struct
 */
struct octeon_twsi {
	void __iomem *base;
	const struct octeon_i2c_data *data;
	struct clk clk;
};

static void twsi_unblock(void *base);
static int twsi_stop(void *base);

/**
 * Returns true if we lost arbitration
 *
 * @code	status code
 * @final_read	true if this is the final read operation
 * Return:	true if arbitration has been lost, false if it hasn't been lost.
 */
static int twsi_i2c_lost_arb(u8 code, int final_read)
{
	switch (code) {
	case TWSI_STAT_TX_ARB_LOST:
	case TWSI_STAT_TX_ACK_ARB_LOST:
	case TWSI_STAT_RX_GEN_ADDR_ARB_LOST:
	case TWSI_STAT_RXDATA_ACK_ARB_LOST:
		/* Arbitration lost */
		return -EAGAIN;

	case TWSI_STAT_SLAVE_RXADDR_ACK:
	case TWSI_STAT_RX_GEN_ADDR_ACK:
	case TWSI_STAT_GEN_RXADDR_ACK:
	case TWSI_STAT_GEN_RXADDR_NAK:
		/* Being addressed as slave, should back off and listen */
		return -EIO;

	case TWSI_STAT_SLAVE_RXDATA_ACK:
	case TWSI_STAT_SLAVE_RXDATA_NAK:
	case TWSI_STAT_STOP_MULTI_START:
	case TWSI_STAT_SLAVE_RXADDR2_ACK:
	case TWSI_STAT_SLAVE_TXDATA_ACK:
	case TWSI_STAT_SLAVE_TXDATA_NAK:
	case TWSI_STAT_SLAVE_TXDATA_END_ACK:
		/* Core busy as slave */
		return  -EIO;

	case TWSI_STAT_RXDATA_ACK_SENT:
		/* Ack allowed on pre-terminal bytes only */
		if (!final_read)
			return 0;
		return -EAGAIN;

	case TWSI_STAT_RXDATA_NAK_SENT:
		/* NAK allowed on terminal byte only */
		if (!final_read)
			return 0;
		return -EAGAIN;

	case TWSI_STAT_TXDATA_NAK:
	case TWSI_STAT_TXADDR_NAK:
	case TWSI_STAT_RXADDR_NAK:
	case TWSI_STAT_TXADDR2DATA_NAK:
		return -EAGAIN;
	}

	return 0;
}

/**
 * Writes to the MIO_TWS(0..5)_SW_TWSI register
 *
 * @base	Base address of i2c registers
 * @val		value to write
 * Return:	0 for success, otherwise error
 */
static u64 twsi_write_sw(void __iomem *base, u64 val)
{
	unsigned long start = get_timer(0);

	val &= ~TWSI_SW_R;
	val |= TWSI_SW_V;

	debug("%s(%p, 0x%llx)\n", __func__, base, val);
	writeq(val, base + TWSI_SW_TWSI);
	do {
		val = readq(base + TWSI_SW_TWSI);
	} while ((val & TWSI_SW_V) && (get_timer(start) < 50));

	if (val & TWSI_SW_V)
		debug("%s: timed out\n", __func__);
	return val;
}

/**
 * Reads the MIO_TWS(0..5)_SW_TWSI register
 *
 * @base	Base address of i2c registers
 * @val		value for eia and op, etc. to read
 * Return:	value of the register
 */
static u64 twsi_read_sw(void __iomem *base, u64 val)
{
	unsigned long start = get_timer(0);

	val |= TWSI_SW_R | TWSI_SW_V;

	debug("%s(%p, 0x%llx)\n", __func__, base, val);
	writeq(val, base + TWSI_SW_TWSI);

	do {
		val = readq(base + TWSI_SW_TWSI);
	} while ((val & TWSI_SW_V) && (get_timer(start) < 50));

	if (val & TWSI_SW_V)
		debug("%s: Error writing 0x%llx\n", __func__, val);

	debug("%s: Returning 0x%llx\n", __func__, val);
	return val;
}

/**
 * Write control register
 *
 * @base	Base address for i2c registers
 * @data	data to write
 */
static void twsi_write_ctl(void __iomem *base, u8 data)
{
	u64 val;

	debug("%s(%p, 0x%x)\n", __func__, base, data);
	val = data | FIELD_PREP(TWSI_SW_EOP_IA_MASK, TWSI_CTL) |
		FIELD_PREP(TWSI_SW_OP_MASK, TWSI_SW_EOP_IA);
	twsi_write_sw(base, val);
}

/**
 * Reads the TWSI Control Register
 *
 * @base	Base address for i2c
 * Return:	8-bit TWSI control register
 */
static u8 twsi_read_ctl(void __iomem *base)
{
	u64 val;

	val = FIELD_PREP(TWSI_SW_EOP_IA_MASK, TWSI_CTL) |
		FIELD_PREP(TWSI_SW_OP_MASK, TWSI_SW_EOP_IA);
	val = twsi_read_sw(base, val);

	debug("%s(%p): 0x%x\n", __func__, base, (u8)val);
	return (u8)val;
}

/**
 * Read i2c status register
 *
 * @base	Base address of i2c registers
 * Return:	value of status register
 */
static u8 twsi_read_status(void __iomem *base)
{
	u64 val;

	val = FIELD_PREP(TWSI_SW_EOP_IA_MASK, TWSI_STAT) |
		FIELD_PREP(TWSI_SW_OP_MASK, TWSI_SW_EOP_IA);

	return twsi_read_sw(base, val);
}

/**
 * Waits for an i2c operation to complete
 *
 * @param	base	Base address of registers
 * Return:	0 for success, 1 if timeout
 */
static int twsi_wait(void __iomem *base)
{
	unsigned long start = get_timer(0);
	u8 twsi_ctl;

	debug("%s(%p)\n", __func__, base);
	do {
		twsi_ctl = twsi_read_ctl(base);
		twsi_ctl &= TWSI_CTL_IFLG;
	} while (!twsi_ctl && get_timer(start) < 50);

	debug("  return: %u\n", !twsi_ctl);
	return !twsi_ctl;
}

/**
 * Unsticks the i2c bus
 *
 * @base	base address of registers
 */
static int twsi_start_unstick(void __iomem *base)
{
	twsi_stop(base);
	twsi_unblock(base);

	return 0;
}

/**
 * Sends an i2c start condition
 *
 * @base	base address of registers
 * Return:	0 for success, otherwise error
 */
static int twsi_start(void __iomem *base)
{
	int ret;
	u8 stat;

	debug("%s(%p)\n", __func__, base);
	twsi_write_ctl(base, TWSI_CTL_STA | TWSI_CTL_ENAB);
	ret = twsi_wait(base);
	if (ret) {
		stat = twsi_read_status(base);
		debug("%s: ret: 0x%x, status: 0x%x\n", __func__, ret, stat);
		switch (stat) {
		case TWSI_STAT_START:
		case TWSI_STAT_RSTART:
			return 0;
		case TWSI_STAT_RXADDR_ACK:
		default:
			return twsi_start_unstick(base);
		}
	}

	debug("%s: success\n", __func__);
	return 0;
}

/**
 * Sends an i2c stop condition
 *
 * @base	register base address
 * Return:	0 for success, -1 if error
 */
static int twsi_stop(void __iomem *base)
{
	u8 stat;

	twsi_write_ctl(base, TWSI_CTL_STP | TWSI_CTL_ENAB);

	stat = twsi_read_status(base);
	if (stat != TWSI_STAT_IDLE) {
		debug("%s: Bad status on bus@%p\n", __func__, base);
		return -1;
	}

	return 0;
}

/**
 * Writes data to the i2c bus
 *
 * @base	register base address
 * @slave_addr	address of slave to write to
 * @buffer	Pointer to buffer to write
 * @length	Number of bytes in buffer to write
 * Return:	0 for success, otherwise error
 */
static int twsi_write_data(void __iomem *base, u8  slave_addr,
			   u8 *buffer, unsigned int length)
{
	unsigned int curr = 0;
	u64 val;
	int ret;

	debug("%s(%p, 0x%x, %p, 0x%x)\n", __func__, base, slave_addr,
	      buffer, length);
	ret = twsi_start(base);
	if (ret) {
		debug("%s: Could not start BUS transaction\n", __func__);
		return -1;
	}

	ret = twsi_wait(base);
	if (ret) {
		debug("%s: wait failed\n", __func__);
		return ret;
	}

	val = (u32)(slave_addr << 1) | TWSI_OP_WRITE |
		FIELD_PREP(TWSI_SW_EOP_IA_MASK, TWSI_DATA) |
		FIELD_PREP(TWSI_SW_OP_MASK, TWSI_SW_EOP_IA);
	twsi_write_sw(base, val);
	twsi_write_ctl(base, TWSI_CTL_ENAB);

	debug("%s: Waiting\n", __func__);
	ret = twsi_wait(base);
	if (ret) {
		debug("%s: Timed out writing slave address 0x%x to target\n",
		      __func__, slave_addr);
		return ret;
	}

	ret = twsi_read_status(base);
	debug("%s: status: 0x%x\n", __func__, ret);
	if (ret != TWSI_STAT_TXADDR_ACK) {
		debug("%s: status: 0x%x\n", __func__, ret);
		twsi_stop(base);
		return twsi_i2c_lost_arb(ret, 0);
	}

	while (curr < length) {
		val = buffer[curr++] |
			FIELD_PREP(TWSI_SW_EOP_IA_MASK, TWSI_DATA) |
			FIELD_PREP(TWSI_SW_OP_MASK, TWSI_SW_EOP_IA);
		twsi_write_sw(base, val);
		twsi_write_ctl(base, TWSI_CTL_ENAB);

		debug("%s: Writing 0x%llx\n", __func__, val);

		ret = twsi_wait(base);
		if (ret) {
			debug("%s: Timed out writing data to 0x%x\n",
			      __func__, slave_addr);
			return ret;
		}
		ret = twsi_read_status(base);
		debug("%s: status: 0x%x\n", __func__, ret);
	}

	debug("%s: Stopping\n", __func__);
	return twsi_stop(base);
}

/**
 * Manually clear the I2C bus and send a stop
 *
 * @base	register base address
 */
static void twsi_unblock(void __iomem *base)
{
	int i;

	for (i = 0; i < 9; i++) {
		writeq(0, base + TWSI_INT);
		udelay(5);
		writeq(TWSI_INT_SCL_OVR, base + TWSI_INT);
		udelay(5);
	}
	writeq(TWSI_INT_SCL_OVR | TWSI_INT_SDA_OVR, base + TWSI_INT);
	udelay(5);
	writeq(TWSI_INT_SDA_OVR, base + TWSI_INT);
	udelay(5);
	writeq(0, base + TWSI_INT);
	udelay(5);
}

/**
 * Performs a read transaction on the i2c bus
 *
 * @base	Base address of twsi registers
 * @slave_addr	i2c bus address to read from
 * @buffer	buffer to read into
 * @length	number of bytes to read
 * Return:	0 for success, otherwise error
 */
static int twsi_read_data(void __iomem *base, u8 slave_addr,
			  u8 *buffer, unsigned int length)
{
	unsigned int curr = 0;
	u64 val;
	int ret;

	debug("%s(%p, 0x%x, %p, %u)\n", __func__, base, slave_addr,
	      buffer, length);
	ret = twsi_start(base);
	if (ret) {
		debug("%s: start failed\n", __func__);
		return ret;
	}

	ret = twsi_wait(base);
	if (ret) {
		debug("%s: wait failed\n", __func__);
		return ret;
	}

	val = (u32)(slave_addr << 1) | TWSI_OP_READ |
		FIELD_PREP(TWSI_SW_EOP_IA_MASK, TWSI_DATA) |
		FIELD_PREP(TWSI_SW_OP_MASK, TWSI_SW_EOP_IA);
	twsi_write_sw(base, val);
	twsi_write_ctl(base, TWSI_CTL_ENAB);

	ret = twsi_wait(base);
	if (ret) {
		debug("%s: waiting for sending addr failed\n", __func__);
		return ret;
	}

	ret = twsi_read_status(base);
	debug("%s: status: 0x%x\n", __func__, ret);
	if (ret != TWSI_STAT_RXADDR_ACK) {
		debug("%s: status: 0x%x\n", __func__, ret);
		twsi_stop(base);
		return twsi_i2c_lost_arb(ret, 0);
	}

	while (curr < length) {
		twsi_write_ctl(base, TWSI_CTL_ENAB |
			       ((curr < length - 1) ? TWSI_CTL_AAK : 0));

		ret = twsi_wait(base);
		if (ret) {
			debug("%s: waiting for data failed\n", __func__);
			return ret;
		}

		val = twsi_read_sw(base, val);
		buffer[curr++] = (u8)val;
	}

	twsi_stop(base);

	return 0;
}

/**
 * Calculate the divisor values
 *
 * @speed	Speed to set
 * @m_div	Pointer to M divisor
 * @n_div	Pointer to N divisor
 * Return:	0 for success, otherwise error
 */
static void twsi_calc_div(struct udevice *bus, ulong sclk, unsigned int speed,
			  int *m_div, int *n_div)
{
	struct octeon_twsi *twsi = dev_get_priv(bus);
	int thp = twsi->data->thp;
	int tclk, fsamp;
	int ndiv, mdiv;

	if (twsi->data->clk_method == CLK_METHOD_OCTEON) {
		tclk = sclk / (2 * (thp + 1));
	} else {
		/* Refclk src in mode register defaults to 100MHz clock */
		sclk = 100000000; /* 100 Mhz */
		tclk = sclk / (thp + 2);
	}
	debug("%s( io_clock %lu tclk %u)\n", __func__, sclk, tclk);

	/*
	 * Compute the clocks M divider:
	 *
	 * TWSI freq = (core freq) / (10 x (M+1) x 2 * (thp+1) x 2^N)
	 * M = ((core freq) / (10 x (TWSI freq) x 2 * (thp+1) x 2^N)) - 1
	 *
	 * For OcteonTX2 -
	 * TWSI freq = (core freq) / (10 x (M+1) x (thp+2) x 2^N)
	 * M = ((core freq) / (10 x (TWSI freq) x (thp+2) x 2^N)) - 1
	 */
	for (ndiv = 0; ndiv < 8; ndiv++) {
		fsamp = tclk / (1 << ndiv);
		mdiv = fsamp / speed / 10;
		mdiv -= 1;
		if (mdiv < 16)
			break;
	}

	*m_div = mdiv;
	*n_div = ndiv;
}

/**
 * Init I2C controller
 *
 * @base	Base address of twsi registers
 * @slave_addr	I2C slave address to configure this controller to
 * Return:	0 for success, otherwise error
 */
static int twsi_init(void __iomem *base, int slaveaddr)
{
	u64 val;

	debug("%s (%p, 0x%x)\n", __func__, base, slaveaddr);

	val = slaveaddr << 1 |
		FIELD_PREP(TWSI_SW_EOP_IA_MASK, 0) |
		FIELD_PREP(TWSI_SW_OP_MASK, TWSI_SW_EOP_IA) |
		TWSI_SW_V;
	twsi_write_sw(base, val);

	/* Set slave address */
	val = slaveaddr |
		FIELD_PREP(TWSI_SW_EOP_IA_MASK, TWSI_EOP_SLAVE_ADDR) |
		FIELD_PREP(TWSI_SW_OP_MASK, TWSI_SW_EOP_IA) |
		TWSI_SW_V;
	twsi_write_sw(base, val);

	return 0;
}

/**
 * Transfers data over the i2c bus
 *
 * @bus		i2c bus to transfer data over
 * @msg		Array of i2c messages
 * @nmsgs	Number of messages to send/receive
 * Return:	0 for success, otherwise error
 */
static int octeon_i2c_xfer(struct udevice *bus, struct i2c_msg *msg,
			   int nmsgs)
{
	struct octeon_twsi *twsi = dev_get_priv(bus);
	int ret;
	int i;

	debug("%s: %d messages\n", __func__, nmsgs);
	for (i = 0; i < nmsgs; i++, msg++) {
		debug("%s: chip=0x%x, len=0x%x\n", __func__, msg->addr,
		      msg->len);

		if (msg->flags & I2C_M_RD) {
			debug("%s: Reading data\n", __func__);
			ret = twsi_read_data(twsi->base, msg->addr,
					     msg->buf, msg->len);
		} else {
			debug("%s: Writing data\n", __func__);
			ret = twsi_write_data(twsi->base, msg->addr,
					      msg->buf, msg->len);
		}
		if (ret) {
			debug("%s: error sending\n", __func__);
			return -EREMOTEIO;
		}
	}

	return 0;
}

/**
 * Set I2C bus speed
 *
 * @bus		i2c bus to transfer data over
 * @speed	Speed in Hz to set
 * Return:	0 for success, otherwise error
 */
static int octeon_i2c_set_bus_speed(struct udevice *bus, unsigned int speed)
{
	struct octeon_twsi *twsi = dev_get_priv(bus);
	int m_div, n_div;
	ulong clk_rate;
	u64 val;

	debug("%s(%p, %u)\n", __func__, bus, speed);

	clk_rate = clk_get_rate(&twsi->clk);
	if (IS_ERR_VALUE(clk_rate))
		return -EINVAL;

	twsi_calc_div(bus, clk_rate, speed, &m_div, &n_div);
	if (m_div >= 16)
		return -1;

	val = (u32)(((m_div & 0xf) << 3) | ((n_div & 0x7) << 0)) |
		FIELD_PREP(TWSI_SW_EOP_IA_MASK, TWSI_CLKCTL) |
		FIELD_PREP(TWSI_SW_OP_MASK, TWSI_SW_EOP_IA) |
		TWSI_SW_V;
	/* Only init non-slave ports */
	writeq(val, twsi->base + TWSI_SW_TWSI);

	debug("%s: Wrote 0x%llx to sw_twsi\n", __func__, val);
	return 0;
}

static const struct octeon_i2c_data i2c_octeon_data = {
	.probe = PROBE_DT,
	.reg_offs = 0x0000,
	.thp = 3,
	.clk_method = CLK_METHOD_OCTEON,
};

static const struct octeon_i2c_data i2c_octeontx_data = {
	.probe = PROBE_PCI,
	.reg_offs = 0x1000,
	.thp = 24,
	.clk_method = CLK_METHOD_OCTEON,
};

static const struct octeon_i2c_data i2c_octeontx2_data = {
	.probe = PROBE_PCI,
	.reg_offs = 0x1000,
	.thp = 3,
	.clk_method = CLK_METHOD_OCTEONTX2,
};

/**
 * Driver probe function
 *
 * @dev		I2C device to probe
 * Return:	0 for success, otherwise error
 */
static int octeon_i2c_probe(struct udevice *dev)
{
	struct octeon_twsi *twsi = dev_get_priv(dev);
	u32 i2c_slave_addr;
	int ret;

	/* Octeon TX2 needs a different data struct */
	if (device_is_compatible(dev, "cavium,thunderx-i2c"))
		dev->driver_data = (long)&i2c_octeontx2_data;

	twsi->data = (const struct octeon_i2c_data *)dev_get_driver_data(dev);

	if (twsi->data->probe == PROBE_PCI) {
		pci_dev_t bdf = dm_pci_get_bdf(dev);

		debug("TWSI PCI device: %x\n", bdf);

		twsi->base = dm_pci_map_bar(dev, PCI_BASE_ADDRESS_0, 0, 0, PCI_REGION_TYPE,
					    PCI_REGION_MEM);
	} else {
		twsi->base = dev_remap_addr(dev);
	}
	twsi->base += twsi->data->reg_offs;

	i2c_slave_addr = dev_read_u32_default(dev, "i2c-sda-hold-time-ns",
					      CFG_SYS_I2C_OCTEON_SLAVE_ADDR);

	ret = clk_get_by_index(dev, 0, &twsi->clk);
	if (ret < 0)
		return ret;

	ret = clk_enable(&twsi->clk);
	if (ret)
		return ret;

	debug("TWSI bus %d at %p\n", dev_seq(dev), twsi->base);

	/* Start with standard speed, real speed set via DT or cmd */
	return twsi_init(twsi->base, i2c_slave_addr);
}

static const struct dm_i2c_ops octeon_i2c_ops = {
	.xfer		= octeon_i2c_xfer,
	.set_bus_speed	= octeon_i2c_set_bus_speed,
};

static const struct udevice_id octeon_i2c_ids[] = {
	{ .compatible = "cavium,octeon-7890-twsi",
	  .data = (ulong)&i2c_octeon_data },
	{ .compatible = "cavium,thunder-8890-twsi",
	  .data = (ulong)&i2c_octeontx_data },
	{ }
};

U_BOOT_DRIVER(octeon_pci_twsi) = {
	.name	= "i2c_octeon",
	.id	= UCLASS_I2C,
	.of_match = octeon_i2c_ids,
	.probe	= octeon_i2c_probe,
	.priv_auto	= sizeof(struct octeon_twsi),
	.ops	= &octeon_i2c_ops,
};
