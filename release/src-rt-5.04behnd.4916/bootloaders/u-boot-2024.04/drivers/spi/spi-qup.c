// SPDX-License-Identifier: GPL-2.0+
/*
 * Driver for Qualcomm QUP SPI controller
 * FIFO and Block modes supported, no DMA
 * mode support
 *
 * Copyright (c) 2020 Sartura Ltd.
 *
 * Author: Robert Marko <robert.marko@sartura.hr>
 * Author: Luka Kovacic <luka.kovacic@sartura.hr>
 *
 * Based on stock U-Boot and Linux drivers
 */

#include <asm/gpio.h>
#include <asm/io.h>
#include <clk.h>
#include <common.h>
#include <dm.h>
#include <errno.h>
#include <linux/delay.h>
#include <spi.h>

#define QUP_CONFIG				0x0000
#define QUP_STATE				0x0004
#define QUP_IO_M_MODES			0x0008
#define QUP_SW_RESET			0x000c
#define QUP_OPERATIONAL			0x0018
#define QUP_ERROR_FLAGS			0x001c
#define QUP_ERROR_FLAGS_EN		0x0020
#define QUP_OPERATIONAL_MASK	0x0028
#define QUP_HW_VERSION			0x0030
#define QUP_MX_OUTPUT_CNT		0x0100
#define QUP_OUTPUT_FIFO			0x0110
#define QUP_MX_WRITE_CNT		0x0150
#define QUP_MX_INPUT_CNT		0x0200
#define QUP_MX_READ_CNT			0x0208
#define QUP_INPUT_FIFO			0x0218

#define SPI_CONFIG				0x0300
#define SPI_IO_CONTROL			0x0304
#define SPI_ERROR_FLAGS			0x0308
#define SPI_ERROR_FLAGS_EN		0x030c

/* QUP_CONFIG fields */
#define QUP_CONFIG_SPI_MODE			BIT(8)
#define QUP_CONFIG_CLOCK_AUTO_GATE	BIT(13)
#define QUP_CONFIG_NO_INPUT			BIT(7)
#define QUP_CONFIG_NO_OUTPUT		BIT(6)
#define QUP_CONFIG_N				0x001f

/* QUP_STATE fields */
#define QUP_STATE_VALID			BIT(2)
#define QUP_STATE_RESET			0
#define QUP_STATE_RUN			1
#define QUP_STATE_PAUSE			3
#define QUP_STATE_MASK			3
#define QUP_STATE_CLEAR			2

/* QUP_IO_M_MODES fields */
#define QUP_IO_M_PACK_EN		BIT(15)
#define QUP_IO_M_UNPACK_EN		BIT(14)
#define QUP_IO_M_INPUT_MODE_MASK_SHIFT	12
#define QUP_IO_M_OUTPUT_MODE_MASK_SHIFT	10
#define QUP_IO_M_INPUT_MODE_MASK	(3 << QUP_IO_M_INPUT_MODE_MASK_SHIFT)
#define QUP_IO_M_OUTPUT_MODE_MASK	(3 << QUP_IO_M_OUTPUT_MODE_MASK_SHIFT)

#define QUP_IO_M_OUTPUT_BLOCK_SIZE(x)	(((x) & (0x03 << 0)) >> 0)
#define QUP_IO_M_OUTPUT_FIFO_SIZE(x)	(((x) & (0x07 << 2)) >> 2)
#define QUP_IO_M_INPUT_BLOCK_SIZE(x)	(((x) & (0x03 << 5)) >> 5)
#define QUP_IO_M_INPUT_FIFO_SIZE(x)	(((x) & (0x07 << 7)) >> 7)

#define QUP_IO_M_MODE_FIFO		0
#define QUP_IO_M_MODE_BLOCK		1
#define QUP_IO_M_MODE_DMOV		2
#define QUP_IO_M_MODE_BAM		3

/* QUP_OPERATIONAL fields */
#define QUP_OP_IN_BLOCK_READ_REQ	BIT(13)
#define QUP_OP_OUT_BLOCK_WRITE_REQ	BIT(12)
#define QUP_OP_MAX_INPUT_DONE_FLAG	BIT(11)
#define QUP_OP_MAX_OUTPUT_DONE_FLAG	BIT(10)
#define QUP_OP_IN_SERVICE_FLAG		BIT(9)
#define QUP_OP_OUT_SERVICE_FLAG		BIT(8)
#define QUP_OP_IN_FIFO_FULL		BIT(7)
#define QUP_OP_OUT_FIFO_FULL		BIT(6)
#define QUP_OP_IN_FIFO_NOT_EMPTY	BIT(5)
#define QUP_OP_OUT_FIFO_NOT_EMPTY	BIT(4)

/* QUP_ERROR_FLAGS and QUP_ERROR_FLAGS_EN fields */
#define QUP_ERROR_OUTPUT_OVER_RUN	BIT(5)
#define QUP_ERROR_INPUT_UNDER_RUN	BIT(4)
#define QUP_ERROR_OUTPUT_UNDER_RUN	BIT(3)
#define QUP_ERROR_INPUT_OVER_RUN	BIT(2)

/* SPI_CONFIG fields */
#define SPI_CONFIG_HS_MODE		BIT(10)
#define SPI_CONFIG_INPUT_FIRST		BIT(9)
#define SPI_CONFIG_LOOPBACK		BIT(8)

/* SPI_IO_CONTROL fields */
#define SPI_IO_C_FORCE_CS		BIT(11)
#define SPI_IO_C_CLK_IDLE_HIGH		BIT(10)
#define SPI_IO_C_MX_CS_MODE		BIT(8)
#define SPI_IO_C_CS_N_POLARITY_0	BIT(4)
#define SPI_IO_C_CS_SELECT(x)		(((x) & 3) << 2)
#define SPI_IO_C_CS_SELECT_MASK		0x000c
#define SPI_IO_C_TRISTATE_CS		BIT(1)
#define SPI_IO_C_NO_TRI_STATE		BIT(0)

/* SPI_ERROR_FLAGS and SPI_ERROR_FLAGS_EN fields */
#define SPI_ERROR_CLK_OVER_RUN		BIT(1)
#define SPI_ERROR_CLK_UNDER_RUN		BIT(0)

#define SPI_NUM_CHIPSELECTS		4

#define SPI_DELAY_THRESHOLD		1
#define SPI_DELAY_RETRY			10

#define SPI_RESET_STATE			0
#define SPI_RUN_STATE			1
#define SPI_CORE_RESET			0
#define SPI_CORE_RUNNING		1

#define DUMMY_DATA_VAL			0
#define TIMEOUT_CNT				100

#define QUP_STATE_VALID_BIT				2
#define QUP_CONFIG_MINI_CORE_MSK		(0x0F << 8)
#define QUP_CONFIG_MINI_CORE_SPI		BIT(8)
#define QUP_CONF_INPUT_MSK				BIT(7)
#define QUP_CONF_INPUT_ENA				(0 << 7)
#define QUP_CONF_NO_INPUT				BIT(7)
#define QUP_CONF_OUTPUT_MSK				BIT(6)
#define QUP_CONF_OUTPUT_ENA				(0 << 6)
#define QUP_CONF_NO_OUTPUT				BIT(6)
#define QUP_STATE_RUN_STATE				0x1
#define QUP_STATE_RESET_STATE			0x0
#define QUP_STATE_PAUSE_STATE			0x3
#define SPI_BIT_WORD_MSK				0x1F
#define SPI_8_BIT_WORD					0x07
#define LOOP_BACK_MSK					BIT(8)
#define NO_LOOP_BACK					(0 << 8)
#define SLAVE_OPERATION_MSK				BIT(5)
#define SLAVE_OPERATION					(0 << 5)
#define CLK_ALWAYS_ON					(0 << 9)
#define MX_CS_MODE						BIT(8)
#define CS_POLARITY_MASK				BIT(4)
#define NO_TRI_STATE					BIT(0)
#define FORCE_CS_MSK					BIT(11)
#define FORCE_CS_EN						BIT(11)
#define FORCE_CS_DIS					(0 << 11)
#define OUTPUT_BIT_SHIFT_MSK			BIT(16)
#define OUTPUT_BIT_SHIFT_EN				BIT(16)
#define INPUT_BLOCK_MODE_MSK			(0x03 << 12)
#define INPUT_BLOCK_MODE				(0x01 << 12)
#define OUTPUT_BLOCK_MODE_MSK			(0x03 << 10)
#define OUTPUT_BLOCK_MODE				(0x01 << 10)
#define INPUT_BAM_MODE					(0x3 << 12)
#define OUTPUT_BAM_MODE					(0x3 << 10)
#define PACK_EN							(0x1 << 15)
#define UNPACK_EN						(0x1 << 14)
#define PACK_EN_MSK						(0x1 << 15)
#define UNPACK_EN_MSK					(0x1 << 14)
#define OUTPUT_SERVICE_MSK				(0x1 << 8)
#define INPUT_SERVICE_MSK				(0x1 << 9)
#define OUTPUT_SERVICE_DIS				(0x1 << 8)
#define INPUT_SERVICE_DIS				(0x1 << 9)
#define BLSP0_SPI_DEASSERT_WAIT_REG		0x0310
#define QUP_DATA_AVAILABLE_FOR_READ		BIT(5)
#define SPI_INPUT_BLOCK_SIZE			4
#define SPI_OUTPUT_BLOCK_SIZE			4
#define SPI_BITLEN_MSK					0x07
#define MAX_COUNT_SIZE					0xffff

struct qup_spi_priv {
	phys_addr_t base;
	struct clk clk;
	u32 num_cs;
	struct gpio_desc cs_gpios[SPI_NUM_CHIPSELECTS];
	bool cs_high;
	u32 core_state;
};

static int qup_spi_set_cs(struct udevice *dev, unsigned int cs, bool enable)
{
	struct qup_spi_priv *priv = dev_get_priv(dev);

	debug("%s: cs=%d enable=%d\n", __func__, cs, enable);

	if (cs >= SPI_NUM_CHIPSELECTS)
		return -ENODEV;

	if (!dm_gpio_is_valid(&priv->cs_gpios[cs]))
		return -EINVAL;

	if (priv->cs_high)
		enable = !enable;

	return dm_gpio_set_value(&priv->cs_gpios[cs], enable ? 1 : 0);
}

/*
 * Function to write data to OUTPUT FIFO
 */
static void qup_spi_write_byte(struct udevice *dev, unsigned char data)
{
	struct udevice *bus = dev_get_parent(dev);
	struct qup_spi_priv *priv = dev_get_priv(bus);
	/* Wait for space in the FIFO */
	while ((readl(priv->base + QUP_OPERATIONAL) & QUP_OP_OUT_FIFO_FULL))
		udelay(1);

	/* Write the byte of data */
	writel(data, priv->base + QUP_OUTPUT_FIFO);
}

/*
 * Function to read data from Input FIFO
 */
static unsigned char qup_spi_read_byte(struct udevice *dev)
{
	struct udevice *bus = dev_get_parent(dev);
	struct qup_spi_priv *priv = dev_get_priv(bus);
	/* Wait for Data in FIFO */
	while (!(readl(priv->base + QUP_OPERATIONAL) & QUP_DATA_AVAILABLE_FOR_READ)) {
		printf("Stuck at FIFO data wait\n");
		udelay(1);
	}

	/* Read a byte of data */
	return readl(priv->base + QUP_INPUT_FIFO) & 0xff;
}

/*
 * Function to check whether Input or Output FIFO
 * has data to be serviced
 */
static int qup_spi_check_fifo_status(struct udevice *dev, u32 reg_addr)
{
	struct udevice *bus = dev_get_parent(dev);
	struct qup_spi_priv *priv = dev_get_priv(bus);
	unsigned int count = TIMEOUT_CNT;
	unsigned int status_flag;
	unsigned int val;

	do {
		val = readl(priv->base + reg_addr);
		count--;
		if (count == 0)
			return -ETIMEDOUT;

		status_flag = ((val & QUP_OP_OUT_SERVICE_FLAG) | (val & QUP_OP_IN_SERVICE_FLAG));
	} while (!status_flag);

	return 0;
}

/*
 * Function to configure Input and Output enable/disable
 */
static void qup_spi_enable_io_config(struct udevice *dev, u32 write_cnt, u32 read_cnt)
{
	struct udevice *bus = dev_get_parent(dev);
	struct qup_spi_priv *priv = dev_get_priv(bus);

	if (write_cnt) {
		clrsetbits_le32(priv->base + QUP_CONFIG,
				QUP_CONF_OUTPUT_MSK, QUP_CONF_OUTPUT_ENA);
	} else {
		clrsetbits_le32(priv->base + QUP_CONFIG,
				QUP_CONF_OUTPUT_MSK, QUP_CONF_NO_OUTPUT);
	}

	if (read_cnt) {
		clrsetbits_le32(priv->base + QUP_CONFIG,
				QUP_CONF_INPUT_MSK, QUP_CONF_INPUT_ENA);
	} else {
		clrsetbits_le32(priv->base + QUP_CONFIG,
				QUP_CONF_INPUT_MSK, QUP_CONF_NO_INPUT);
	}
}

static int check_bit_state(struct udevice *dev, u32 reg_addr, int bit_num, int val,
						   int us_delay)
{
	struct udevice *bus = dev_get_parent(dev);
	struct qup_spi_priv *priv = dev_get_priv(bus);
	unsigned int count = TIMEOUT_CNT;
	unsigned int bit_val = ((readl(priv->base + reg_addr) >> bit_num) & 0x01);

	while (bit_val != val) {
		count--;
		if (count == 0)
			return -ETIMEDOUT;
		udelay(us_delay);
		bit_val = ((readl(priv->base + reg_addr) >> bit_num) & 0x01);
	}

	return 0;
}

/*
 * Check whether QUPn State is valid
 */
static int check_qup_state_valid(struct udevice *dev)
{
	return check_bit_state(dev, QUP_STATE, QUP_STATE_VALID, 1, 1);
}

/*
 * Configure QUPn Core state
 */
static int qup_spi_config_spi_state(struct udevice *dev, unsigned int state)
{
	struct udevice *bus = dev_get_parent(dev);
	struct qup_spi_priv *priv = dev_get_priv(bus);
	u32 val;
	int ret;

	ret = check_qup_state_valid(dev);
	if (ret != 0)
		return ret;

	switch (state) {
	case SPI_RUN_STATE:
		/* Set the state to RUN */
		val = ((readl(priv->base + QUP_STATE) & ~QUP_STATE_MASK)
					| QUP_STATE_RUN);
		writel(val, priv->base + QUP_STATE);
		ret = check_qup_state_valid(dev);
		if (ret != 0)
			return ret;
		priv->core_state = SPI_CORE_RUNNING;
		break;
	case SPI_RESET_STATE:
		/* Set the state to RESET */
		val = ((readl(priv->base + QUP_STATE) & ~QUP_STATE_MASK)
					| QUP_STATE_RESET);
		writel(val, priv->base + QUP_STATE);
		ret = check_qup_state_valid(dev);
		if (ret != 0)
			return ret;
		priv->core_state = SPI_CORE_RESET;
		break;
	default:
		printf("Unsupported QUP SPI state: %d\n", state);
		ret = -EINVAL;
		break;
	}
	return ret;
}

/*
 * Function to read bytes number of data from the Input FIFO
 */
static int __qup_spi_blsp_spi_read(struct udevice *dev, u8 *data_buffer, unsigned int bytes)
{
	struct udevice *bus = dev_get_parent(dev);
	struct qup_spi_priv *priv = dev_get_priv(bus);
	u32 val;
	unsigned int i;
	unsigned int read_bytes = bytes;
	unsigned int fifo_count;
	int ret = 0;
	int state_config;

	/* Configure no of bytes to read */
	state_config = qup_spi_config_spi_state(dev, SPI_RESET_STATE);
	if (state_config)
		return state_config;

	/* Configure input and output enable */
	qup_spi_enable_io_config(dev, 0, read_bytes);

	writel(bytes, priv->base + QUP_MX_INPUT_CNT);

	state_config = qup_spi_config_spi_state(dev, SPI_RUN_STATE);
	if (state_config)
		return state_config;

	while (read_bytes) {
		ret = qup_spi_check_fifo_status(dev, QUP_OPERATIONAL);
		if (ret != 0)
			goto out;

		val = readl(priv->base + QUP_OPERATIONAL);
		if (val & QUP_OP_IN_SERVICE_FLAG) {
			/*
			 * acknowledge to hw that software will
			 * read input data
			 */
			val &= QUP_OP_IN_SERVICE_FLAG;
			writel(val, priv->base + QUP_OPERATIONAL);

			fifo_count = ((read_bytes > SPI_INPUT_BLOCK_SIZE) ?
					SPI_INPUT_BLOCK_SIZE : read_bytes);

			for (i = 0; i < fifo_count; i++) {
				*data_buffer = qup_spi_read_byte(dev);
				data_buffer++;
				read_bytes--;
			}
		}
	}

out:
	/*
	 * Put the SPI Core back in the Reset State
	 * to end the transfer
	 */
	(void)qup_spi_config_spi_state(dev, SPI_RESET_STATE);

	return ret;
}

static int qup_spi_blsp_spi_read(struct udevice *dev, u8 *data_buffer, unsigned int bytes)
{
	int length, ret;

	while (bytes) {
		length = (bytes < MAX_COUNT_SIZE) ? bytes : MAX_COUNT_SIZE;

		ret = __qup_spi_blsp_spi_read(dev, data_buffer, length);
		if (ret != 0)
			return ret;

		data_buffer += length;
		bytes -= length;
	}

	return 0;
}

/*
 * Function to write data to the Output FIFO
 */
static int __qup_blsp_spi_write(struct udevice *dev, const u8 *cmd_buffer, unsigned int bytes)
{
	struct udevice *bus = dev_get_parent(dev);
	struct qup_spi_priv *priv = dev_get_priv(bus);
	u32 val;
	unsigned int i;
	unsigned int write_len = bytes;
	unsigned int read_len = bytes;
	unsigned int fifo_count;
	int ret = 0;
	int state_config;

	state_config = qup_spi_config_spi_state(dev, SPI_RESET_STATE);
	if (state_config)
		return state_config;

	writel(bytes, priv->base + QUP_MX_OUTPUT_CNT);
	writel(bytes, priv->base + QUP_MX_INPUT_CNT);
	state_config = qup_spi_config_spi_state(dev, SPI_RUN_STATE);
	if (state_config)
		return state_config;

	/* Configure input and output enable */
	qup_spi_enable_io_config(dev, write_len, read_len);

	/*
	 * read_len considered to ensure that we read the dummy data for the
	 * write we performed. This is needed to ensure with WR-RD transaction
	 * to get the actual data on the subsequent read cycle that happens
	 */
	while (write_len || read_len) {
		ret = qup_spi_check_fifo_status(dev, QUP_OPERATIONAL);
		if (ret != 0)
			goto out;

		val = readl(priv->base + QUP_OPERATIONAL);
		if (val & QUP_OP_OUT_SERVICE_FLAG) {
			/*
			 * acknowledge to hw that software will write
			 * expected output data
			 */
			val &= QUP_OP_OUT_SERVICE_FLAG;
			writel(val, priv->base + QUP_OPERATIONAL);

			if (write_len > SPI_OUTPUT_BLOCK_SIZE)
				fifo_count = SPI_OUTPUT_BLOCK_SIZE;
			else
				fifo_count = write_len;

			for (i = 0; i < fifo_count; i++) {
				/* Write actual data to output FIFO */
				qup_spi_write_byte(dev, *cmd_buffer);
				cmd_buffer++;
				write_len--;
			}
		}
		if (val & QUP_OP_IN_SERVICE_FLAG) {
			/*
			 * acknowledge to hw that software
			 * will read input data
			 */
			val &= QUP_OP_IN_SERVICE_FLAG;
			writel(val, priv->base + QUP_OPERATIONAL);

			if (read_len > SPI_INPUT_BLOCK_SIZE)
				fifo_count = SPI_INPUT_BLOCK_SIZE;
			else
				fifo_count = read_len;

			for (i = 0; i < fifo_count; i++) {
				/* Read dummy data for the data written */
				(void)qup_spi_read_byte(dev);

				/* Decrement the write count after reading the
				 * dummy data from the device. This is to make
				 * sure we read dummy data before we write the
				 * data to fifo
				 */
				read_len--;
			}
		}
	}
out:
	/*
	 * Put the SPI Core back in the Reset State
	 * to end the transfer
	 */
	(void)qup_spi_config_spi_state(dev, SPI_RESET_STATE);

	return ret;
}

static int qup_spi_blsp_spi_write(struct udevice *dev, const u8 *cmd_buffer, unsigned int bytes)
{
	int length, ret;

	while (bytes) {
		length = (bytes < MAX_COUNT_SIZE) ? bytes : MAX_COUNT_SIZE;

		ret = __qup_blsp_spi_write(dev, cmd_buffer, length);
		if (ret != 0)
			return ret;

		cmd_buffer += length;
		bytes -= length;
	}

	return 0;
}

static int qup_spi_set_speed(struct udevice *dev, uint speed)
{
	return 0;
}

static int qup_spi_set_mode(struct udevice *dev, uint mode)
{
	struct qup_spi_priv *priv = dev_get_priv(dev);
	unsigned int clk_idle_state;
	unsigned int input_first_mode;
	u32 val;

	switch (mode) {
	case SPI_MODE_0:
		clk_idle_state = 0;
		input_first_mode = SPI_CONFIG_INPUT_FIRST;
		break;
	case SPI_MODE_1:
		clk_idle_state = 0;
		input_first_mode = 0;
		break;
	case SPI_MODE_2:
		clk_idle_state = 1;
		input_first_mode = SPI_CONFIG_INPUT_FIRST;
		break;
	case SPI_MODE_3:
		clk_idle_state = 1;
		input_first_mode = 0;
		break;
	default:
		printf("Unsupported spi mode: %d\n", mode);
		return -EINVAL;
	}

	if (mode & SPI_CS_HIGH)
		priv->cs_high = true;
	else
		priv->cs_high = false;

	val = readl(priv->base + SPI_CONFIG);
	val |= input_first_mode;
	writel(val, priv->base + SPI_CONFIG);

	val = readl(priv->base + SPI_IO_CONTROL);
	if (clk_idle_state)
		val |= SPI_IO_C_CLK_IDLE_HIGH;
	else
		val &= ~SPI_IO_C_CLK_IDLE_HIGH;

	writel(val, priv->base + SPI_IO_CONTROL);

	return 0;
}

static void qup_spi_reset(struct udevice *dev)
{
	struct udevice *bus = dev_get_parent(dev);
	struct qup_spi_priv *priv = dev_get_priv(bus);

	/* Driver may not be probed yet */
	if (!priv)
		return;

	writel(0x1, priv->base + QUP_SW_RESET);
	udelay(5);
}

static int qup_spi_hw_init(struct udevice *dev)
{
	struct udevice *bus = dev_get_parent(dev);
	struct qup_spi_priv *priv = dev_get_priv(bus);
	int ret;

	/* QUPn module configuration */
	qup_spi_reset(dev);

	/* Set the QUPn state */
	ret = qup_spi_config_spi_state(dev, SPI_RESET_STATE);
	if (ret)
		return ret;

	/*
	 * Configure Mini core to SPI core with Input Output enabled,
	 * SPI master, N = 8 bits
	 */
	clrsetbits_le32(priv->base + QUP_CONFIG, (QUP_CONFIG_MINI_CORE_MSK |
						QUP_CONF_INPUT_MSK |
						QUP_CONF_OUTPUT_MSK |
						SPI_BIT_WORD_MSK),
						(QUP_CONFIG_MINI_CORE_SPI |
						QUP_CONF_INPUT_ENA |
						QUP_CONF_OUTPUT_ENA |
						SPI_8_BIT_WORD));

	/*
	 * Configure Input first SPI protocol,
	 * SPI master mode and no loopback
	 */
	clrsetbits_le32(priv->base + SPI_CONFIG, (LOOP_BACK_MSK |
						SLAVE_OPERATION_MSK),
						(NO_LOOP_BACK |
						SLAVE_OPERATION));

	/*
	 * Configure SPI IO Control Register
	 * CLK_ALWAYS_ON = 0
	 * MX_CS_MODE = 0
	 * NO_TRI_STATE = 1
	 */
	writel((CLK_ALWAYS_ON | NO_TRI_STATE), priv->base + SPI_IO_CONTROL);

	/*
	 * Configure SPI IO Modes.
	 * OUTPUT_BIT_SHIFT_EN = 1
	 * INPUT_MODE = Block Mode
	 * OUTPUT MODE = Block Mode
	 */

	clrsetbits_le32(priv->base + QUP_IO_M_MODES, (OUTPUT_BIT_SHIFT_MSK |
				INPUT_BLOCK_MODE_MSK |
				OUTPUT_BLOCK_MODE_MSK),
				(OUTPUT_BIT_SHIFT_EN |
				INPUT_BLOCK_MODE |
				OUTPUT_BLOCK_MODE));

	/* Disable Error mask */
	writel(0, priv->base + SPI_ERROR_FLAGS_EN);
	writel(0, priv->base + QUP_ERROR_FLAGS_EN);
	writel(0, priv->base + BLSP0_SPI_DEASSERT_WAIT_REG);

	return ret;
}

static int qup_spi_claim_bus(struct udevice *dev)
{
	int ret;

	ret = qup_spi_hw_init(dev);
	if (ret)
		return -EIO;

	return 0;
}

static int qup_spi_release_bus(struct udevice *dev)
{
	/* Reset the SPI hardware */
	qup_spi_reset(dev);

	return 0;
}

static int qup_spi_xfer(struct udevice *dev, unsigned int bitlen,
						const void *dout, void *din, unsigned long flags)
{
	struct udevice *bus = dev_get_parent(dev);
	struct dm_spi_slave_plat *slave_plat = dev_get_parent_plat(dev);
	unsigned int len;
	const u8 *txp = dout;
	u8 *rxp = din;
	int ret = 0;

	if (bitlen & SPI_BITLEN_MSK) {
		printf("Invalid bit length\n");
		return -EINVAL;
	}

	len = bitlen >> 3;

	if (flags & SPI_XFER_BEGIN) {
		ret = qup_spi_hw_init(dev);
		if (ret != 0)
			return ret;

		ret = qup_spi_set_cs(bus, slave_plat->cs, false);
		if (ret != 0)
			return ret;
	}

	if (dout != NULL) {
		ret = qup_spi_blsp_spi_write(dev, txp, len);
		if (ret != 0)
			return ret;
	}

	if (din != NULL) {
		ret = qup_spi_blsp_spi_read(dev, rxp, len);
		if (ret != 0)
			return ret;
	}

	if (flags & SPI_XFER_END) {
		ret = qup_spi_set_cs(bus, slave_plat->cs, true);
		if (ret != 0)
			return ret;
	}

	return ret;
}

static int qup_spi_probe(struct udevice *dev)
{
	struct qup_spi_priv *priv = dev_get_priv(dev);
	int ret;

	priv->base = dev_read_addr(dev);
	if (priv->base == FDT_ADDR_T_NONE)
		return -EINVAL;

	ret = clk_get_by_index(dev, 0, &priv->clk);
	if (ret)
		return ret;

	ret = clk_enable(&priv->clk);
	if (ret < 0)
		return ret;

	priv->num_cs = dev_read_u32_default(dev, "num-cs", 1);

	ret = gpio_request_list_by_name(dev, "cs-gpios", priv->cs_gpios,
					priv->num_cs, GPIOD_IS_OUT | GPIOD_IS_OUT_ACTIVE);
	if (ret < 0) {
		printf("Can't get %s cs gpios: %d\n", dev->name, ret);
		return -EINVAL;
	}

	return 0;
}

static const struct dm_spi_ops qup_spi_ops = {
	.claim_bus	= qup_spi_claim_bus,
	.release_bus	= qup_spi_release_bus,
	.xfer		= qup_spi_xfer,
	.set_speed	= qup_spi_set_speed,
	.set_mode	= qup_spi_set_mode,
	/*
	 * cs_info is not needed, since we require all chip selects to be
	 * in the device tree explicitly
	 */
};

static const struct udevice_id qup_spi_ids[] = {
	{ .compatible = "qcom,spi-qup-v1.1.1", },
	{ .compatible = "qcom,spi-qup-v2.1.1", },
	{ .compatible = "qcom,spi-qup-v2.2.1", },
	{ }
};

U_BOOT_DRIVER(spi_qup) = {
	.name	= "spi_qup",
	.id	= UCLASS_SPI,
	.of_match = qup_spi_ids,
	.ops	= &qup_spi_ops,
	.priv_auto	= sizeof(struct qup_spi_priv),
	.probe	= qup_spi_probe,
};
