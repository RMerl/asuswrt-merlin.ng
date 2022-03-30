// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2013 Broadcom Corporation.
 *
 * NOTE: This driver should be converted to driver model before June 2017.
 * Please see doc/driver-model/i2c-howto.txt for instructions.
 */

#include <common.h>
#include <asm/io.h>
#include <linux/errno.h>
#include <asm/arch/sysmap.h>
#include <asm/kona-common/clk.h>
#include <i2c.h>

/* Hardware register offsets and field defintions */
#define CS_OFFSET				0x00000020
#define CS_ACK_SHIFT				3
#define CS_ACK_MASK				0x00000008
#define CS_ACK_CMD_GEN_START			0x00000000
#define CS_ACK_CMD_GEN_RESTART			0x00000001
#define CS_CMD_SHIFT				1
#define CS_CMD_CMD_NO_ACTION			0x00000000
#define CS_CMD_CMD_START_RESTART		0x00000001
#define CS_CMD_CMD_STOP				0x00000002
#define CS_EN_SHIFT				0
#define CS_EN_CMD_ENABLE_BSC			0x00000001

#define TIM_OFFSET				0x00000024
#define TIM_PRESCALE_SHIFT			6
#define TIM_P_SHIFT				3
#define TIM_NO_DIV_SHIFT			2
#define TIM_DIV_SHIFT				0

#define DAT_OFFSET				0x00000028

#define TOUT_OFFSET				0x0000002c

#define TXFCR_OFFSET				0x0000003c
#define TXFCR_FIFO_FLUSH_MASK			0x00000080
#define TXFCR_FIFO_EN_MASK			0x00000040

#define IER_OFFSET				0x00000044
#define IER_READ_COMPLETE_INT_MASK		0x00000010
#define IER_I2C_INT_EN_MASK			0x00000008
#define IER_FIFO_INT_EN_MASK			0x00000002
#define IER_NOACK_EN_MASK			0x00000001

#define ISR_OFFSET				0x00000048
#define ISR_RESERVED_MASK			0xffffff60
#define ISR_CMDBUSY_MASK			0x00000080
#define ISR_READ_COMPLETE_MASK			0x00000010
#define ISR_SES_DONE_MASK			0x00000008
#define ISR_ERR_MASK				0x00000004
#define ISR_TXFIFOEMPTY_MASK			0x00000002
#define ISR_NOACK_MASK				0x00000001

#define CLKEN_OFFSET				0x0000004c
#define CLKEN_AUTOSENSE_OFF_MASK		0x00000080
#define CLKEN_M_SHIFT				4
#define CLKEN_N_SHIFT				1
#define CLKEN_CLKEN_MASK			0x00000001

#define FIFO_STATUS_OFFSET			0x00000054
#define FIFO_STATUS_RXFIFO_EMPTY_MASK		0x00000004
#define FIFO_STATUS_TXFIFO_EMPTY_MASK		0x00000010

#define HSTIM_OFFSET				0x00000058
#define HSTIM_HS_MODE_MASK			0x00008000
#define HSTIM_HS_HOLD_SHIFT			10
#define HSTIM_HS_HIGH_PHASE_SHIFT		5
#define HSTIM_HS_SETUP_SHIFT			0

#define PADCTL_OFFSET				0x0000005c
#define PADCTL_PAD_OUT_EN_MASK			0x00000004

#define RXFCR_OFFSET				0x00000068
#define RXFCR_NACK_EN_SHIFT			7
#define RXFCR_READ_COUNT_SHIFT			0
#define RXFIFORDOUT_OFFSET			0x0000006c

/* Locally used constants */
#define MAX_RX_FIFO_SIZE		64U	/* bytes */
#define MAX_TX_FIFO_SIZE		64U	/* bytes */

#define I2C_TIMEOUT			100000	/* usecs */

#define WAIT_INT_CHK			100	/* usecs */
#if I2C_TIMEOUT % WAIT_INT_CHK
#error I2C_TIMEOUT must be a multiple of WAIT_INT_CHK
#endif

/* Operations that can be commanded to the controller */
enum bcm_kona_cmd_t {
	BCM_CMD_NOACTION = 0,
	BCM_CMD_START,
	BCM_CMD_RESTART,
	BCM_CMD_STOP,
};

enum bus_speed_index {
	BCM_SPD_100K = 0,
	BCM_SPD_400K,
	BCM_SPD_1MHZ,
};

/* Internal divider settings for standard mode, fast mode and fast mode plus */
struct bus_speed_cfg {
	uint8_t time_m;		/* Number of cycles for setup time */
	uint8_t time_n;		/* Number of cycles for hold time */
	uint8_t prescale;	/* Prescale divider */
	uint8_t time_p;		/* Timing coefficient */
	uint8_t no_div;		/* Disable clock divider */
	uint8_t time_div;	/* Post-prescale divider */
};

static const struct bus_speed_cfg std_cfg_table[] = {
	[BCM_SPD_100K] = {0x01, 0x01, 0x03, 0x06, 0x00, 0x02},
	[BCM_SPD_400K] = {0x05, 0x01, 0x03, 0x05, 0x01, 0x02},
	[BCM_SPD_1MHZ] = {0x01, 0x01, 0x03, 0x01, 0x01, 0x03},
};

struct bcm_kona_i2c_dev {
	void *base;
	uint speed;
	const struct bus_speed_cfg *std_cfg;
};

/* Keep these two defines in sync */
#define DEF_SPD 100000
#define DEF_SPD_ENUM BCM_SPD_100K

#define DEF_DEVICE(num) \
{(void *)CONFIG_SYS_I2C_BASE##num, DEF_SPD, &std_cfg_table[DEF_SPD_ENUM]}

static struct bcm_kona_i2c_dev g_i2c_devs[CONFIG_SYS_MAX_I2C_BUS] = {
#ifdef CONFIG_SYS_I2C_BASE0
	DEF_DEVICE(0),
#endif
#ifdef CONFIG_SYS_I2C_BASE1
	DEF_DEVICE(1),
#endif
#ifdef CONFIG_SYS_I2C_BASE2
	DEF_DEVICE(2),
#endif
#ifdef CONFIG_SYS_I2C_BASE3
	DEF_DEVICE(3),
#endif
#ifdef CONFIG_SYS_I2C_BASE4
	DEF_DEVICE(4),
#endif
#ifdef CONFIG_SYS_I2C_BASE5
	DEF_DEVICE(5),
#endif
};

#define I2C_M_TEN	0x0010	/* ten bit address */
#define I2C_M_RD	0x0001	/* read data */
#define I2C_M_NOSTART	0x4000	/* no restart between msgs */

struct kona_i2c_msg {
	uint16_t addr;
	uint16_t flags;
	uint16_t len;
	uint8_t *buf;
};

static void bcm_kona_i2c_send_cmd_to_ctrl(struct bcm_kona_i2c_dev *dev,
					  enum bcm_kona_cmd_t cmd)
{
	debug("%s, %d\n", __func__, cmd);

	switch (cmd) {
	case BCM_CMD_NOACTION:
		writel((CS_CMD_CMD_NO_ACTION << CS_CMD_SHIFT) |
		       (CS_EN_CMD_ENABLE_BSC << CS_EN_SHIFT),
		       dev->base + CS_OFFSET);
		break;

	case BCM_CMD_START:
		writel((CS_ACK_CMD_GEN_START << CS_ACK_SHIFT) |
		       (CS_CMD_CMD_START_RESTART << CS_CMD_SHIFT) |
		       (CS_EN_CMD_ENABLE_BSC << CS_EN_SHIFT),
		       dev->base + CS_OFFSET);
		break;

	case BCM_CMD_RESTART:
		writel((CS_ACK_CMD_GEN_RESTART << CS_ACK_SHIFT) |
		       (CS_CMD_CMD_START_RESTART << CS_CMD_SHIFT) |
		       (CS_EN_CMD_ENABLE_BSC << CS_EN_SHIFT),
		       dev->base + CS_OFFSET);
		break;

	case BCM_CMD_STOP:
		writel((CS_CMD_CMD_STOP << CS_CMD_SHIFT) |
		       (CS_EN_CMD_ENABLE_BSC << CS_EN_SHIFT),
		       dev->base + CS_OFFSET);
		break;

	default:
		printf("Unknown command %d\n", cmd);
	}
}

static void bcm_kona_i2c_enable_clock(struct bcm_kona_i2c_dev *dev)
{
	writel(readl(dev->base + CLKEN_OFFSET) | CLKEN_CLKEN_MASK,
	       dev->base + CLKEN_OFFSET);
}

static void bcm_kona_i2c_disable_clock(struct bcm_kona_i2c_dev *dev)
{
	writel(readl(dev->base + CLKEN_OFFSET) & ~CLKEN_CLKEN_MASK,
	       dev->base + CLKEN_OFFSET);
}

/* Wait until at least one of the mask bit(s) are set */
static unsigned long wait_for_int_timeout(struct bcm_kona_i2c_dev *dev,
					  unsigned long time_left,
					  uint32_t mask)
{
	uint32_t status;

	while (time_left) {
		status = readl(dev->base + ISR_OFFSET);

		if ((status & ~ISR_RESERVED_MASK) == 0) {
			debug("Bogus I2C interrupt 0x%x\n", status);
			continue;
		}

		/* Must flush the TX FIFO when NAK detected */
		if (status & ISR_NOACK_MASK)
			writel(TXFCR_FIFO_FLUSH_MASK | TXFCR_FIFO_EN_MASK,
			       dev->base + TXFCR_OFFSET);

		writel(status & ~ISR_RESERVED_MASK, dev->base + ISR_OFFSET);

		if (status & mask) {
			/* We are done since one of the mask bits are set */
			return time_left;
		}
		udelay(WAIT_INT_CHK);
		time_left -= WAIT_INT_CHK;
	}
	return 0;
}

/* Send command to I2C bus */
static int bcm_kona_send_i2c_cmd(struct bcm_kona_i2c_dev *dev,
				 enum bcm_kona_cmd_t cmd)
{
	int rc = 0;
	unsigned long time_left = I2C_TIMEOUT;

	/* Send the command */
	bcm_kona_i2c_send_cmd_to_ctrl(dev, cmd);

	/* Wait for transaction to finish or timeout */
	time_left = wait_for_int_timeout(dev, time_left, IER_I2C_INT_EN_MASK);

	if (!time_left) {
		printf("controller timed out\n");
		rc = -ETIMEDOUT;
	}

	/* Clear command */
	bcm_kona_i2c_send_cmd_to_ctrl(dev, BCM_CMD_NOACTION);

	return rc;
}

/* Read a single RX FIFO worth of data from the i2c bus */
static int bcm_kona_i2c_read_fifo_single(struct bcm_kona_i2c_dev *dev,
					 uint8_t *buf, unsigned int len,
					 unsigned int last_byte_nak)
{
	unsigned long time_left = I2C_TIMEOUT;

	/* Start the RX FIFO */
	writel((last_byte_nak << RXFCR_NACK_EN_SHIFT) |
	       (len << RXFCR_READ_COUNT_SHIFT), dev->base + RXFCR_OFFSET);

	/* Wait for FIFO read to complete */
	time_left =
	    wait_for_int_timeout(dev, time_left, IER_READ_COMPLETE_INT_MASK);

	if (!time_left) {
		printf("RX FIFO time out\n");
		return -EREMOTEIO;
	}

	/* Read data from FIFO */
	for (; len > 0; len--, buf++)
		*buf = readl(dev->base + RXFIFORDOUT_OFFSET);

	return 0;
}

/* Read any amount of data using the RX FIFO from the i2c bus */
static int bcm_kona_i2c_read_fifo(struct bcm_kona_i2c_dev *dev,
				  struct kona_i2c_msg *msg)
{
	unsigned int bytes_to_read = MAX_RX_FIFO_SIZE;
	unsigned int last_byte_nak = 0;
	unsigned int bytes_read = 0;
	int rc;

	uint8_t *tmp_buf = msg->buf;

	while (bytes_read < msg->len) {
		if (msg->len - bytes_read <= MAX_RX_FIFO_SIZE) {
			last_byte_nak = 1;	/* NAK last byte of transfer */
			bytes_to_read = msg->len - bytes_read;
		}

		rc = bcm_kona_i2c_read_fifo_single(dev, tmp_buf, bytes_to_read,
						   last_byte_nak);
		if (rc < 0)
			return -EREMOTEIO;

		bytes_read += bytes_to_read;
		tmp_buf += bytes_to_read;
	}

	return 0;
}

/* Write a single byte of data to the i2c bus */
static int bcm_kona_i2c_write_byte(struct bcm_kona_i2c_dev *dev, uint8_t data,
				   unsigned int nak_expected)
{
	unsigned long time_left = I2C_TIMEOUT;
	unsigned int nak_received;

	/* Clear pending session done interrupt */
	writel(ISR_SES_DONE_MASK, dev->base + ISR_OFFSET);

	/* Send one byte of data */
	writel(data, dev->base + DAT_OFFSET);

	time_left = wait_for_int_timeout(dev, time_left, IER_I2C_INT_EN_MASK);

	if (!time_left) {
		debug("controller timed out\n");
		return -ETIMEDOUT;
	}

	nak_received = readl(dev->base + CS_OFFSET) & CS_ACK_MASK ? 1 : 0;

	if (nak_received ^ nak_expected) {
		debug("unexpected NAK/ACK\n");
		return -EREMOTEIO;
	}

	return 0;
}

/* Write a single TX FIFO worth of data to the i2c bus */
static int bcm_kona_i2c_write_fifo_single(struct bcm_kona_i2c_dev *dev,
					  uint8_t *buf, unsigned int len)
{
	int k;
	unsigned long time_left = I2C_TIMEOUT;
	unsigned int fifo_status;

	/* Write data into FIFO */
	for (k = 0; k < len; k++)
		writel(buf[k], (dev->base + DAT_OFFSET));

	/* Wait for FIFO to empty */
	do {
		time_left =
		    wait_for_int_timeout(dev, time_left,
					 (IER_FIFO_INT_EN_MASK |
					  IER_NOACK_EN_MASK));
		fifo_status = readl(dev->base + FIFO_STATUS_OFFSET);
	} while (time_left && !(fifo_status & FIFO_STATUS_TXFIFO_EMPTY_MASK));

	/* Check if there was a NAK */
	if (readl(dev->base + CS_OFFSET) & CS_ACK_MASK) {
		printf("unexpected NAK\n");
		return -EREMOTEIO;
	}

	/* Check if a timeout occurred */
	if (!time_left) {
		printf("completion timed out\n");
		return -EREMOTEIO;
	}

	return 0;
}

/* Write any amount of data using TX FIFO to the i2c bus */
static int bcm_kona_i2c_write_fifo(struct bcm_kona_i2c_dev *dev,
				   struct kona_i2c_msg *msg)
{
	unsigned int bytes_to_write = MAX_TX_FIFO_SIZE;
	unsigned int bytes_written = 0;
	int rc;

	uint8_t *tmp_buf = msg->buf;

	while (bytes_written < msg->len) {
		if (msg->len - bytes_written <= MAX_TX_FIFO_SIZE)
			bytes_to_write = msg->len - bytes_written;

		rc = bcm_kona_i2c_write_fifo_single(dev, tmp_buf,
						    bytes_to_write);
		if (rc < 0)
			return -EREMOTEIO;

		bytes_written += bytes_to_write;
		tmp_buf += bytes_to_write;
	}

	return 0;
}

/* Send i2c address */
static int bcm_kona_i2c_do_addr(struct bcm_kona_i2c_dev *dev,
				struct kona_i2c_msg *msg)
{
	unsigned char addr;

	if (msg->flags & I2C_M_TEN) {
		/* First byte is 11110XX0 where XX is upper 2 bits */
		addr = 0xf0 | ((msg->addr & 0x300) >> 7);
		if (bcm_kona_i2c_write_byte(dev, addr, 0) < 0)
			return -EREMOTEIO;

		/* Second byte is the remaining 8 bits */
		addr = msg->addr & 0xff;
		if (bcm_kona_i2c_write_byte(dev, addr, 0) < 0)
			return -EREMOTEIO;

		if (msg->flags & I2C_M_RD) {
			/* For read, send restart command */
			if (bcm_kona_send_i2c_cmd(dev, BCM_CMD_RESTART) < 0)
				return -EREMOTEIO;

			/* Then re-send the first byte with the read bit set */
			addr = 0xf0 | ((msg->addr & 0x300) >> 7) | 0x01;
			if (bcm_kona_i2c_write_byte(dev, addr, 0) < 0)
				return -EREMOTEIO;
		}
	} else {
		addr = msg->addr << 1;

		if (msg->flags & I2C_M_RD)
			addr |= 1;

		if (bcm_kona_i2c_write_byte(dev, addr, 0) < 0)
			return -EREMOTEIO;
	}

	return 0;
}

static void bcm_kona_i2c_enable_autosense(struct bcm_kona_i2c_dev *dev)
{
	writel(readl(dev->base + CLKEN_OFFSET) & ~CLKEN_AUTOSENSE_OFF_MASK,
	       dev->base + CLKEN_OFFSET);
}

static void bcm_kona_i2c_config_timing(struct bcm_kona_i2c_dev *dev)
{
	writel(readl(dev->base + HSTIM_OFFSET) & ~HSTIM_HS_MODE_MASK,
	       dev->base + HSTIM_OFFSET);

	writel((dev->std_cfg->prescale << TIM_PRESCALE_SHIFT) |
	       (dev->std_cfg->time_p << TIM_P_SHIFT) |
	       (dev->std_cfg->no_div << TIM_NO_DIV_SHIFT) |
	       (dev->std_cfg->time_div << TIM_DIV_SHIFT),
	       dev->base + TIM_OFFSET);

	writel((dev->std_cfg->time_m << CLKEN_M_SHIFT) |
	       (dev->std_cfg->time_n << CLKEN_N_SHIFT) |
	       CLKEN_CLKEN_MASK, dev->base + CLKEN_OFFSET);
}

/* Master transfer function */
static int bcm_kona_i2c_xfer(struct bcm_kona_i2c_dev *dev,
			     struct kona_i2c_msg msgs[], int num)
{
	struct kona_i2c_msg *pmsg;
	int rc = 0;
	int i;

	/* Enable pad output */
	writel(0, dev->base + PADCTL_OFFSET);

	/* Enable internal clocks */
	bcm_kona_i2c_enable_clock(dev);

	/* Send start command */
	rc = bcm_kona_send_i2c_cmd(dev, BCM_CMD_START);
	if (rc < 0) {
		printf("Start command failed rc = %d\n", rc);
		goto xfer_disable_pad;
	}

	/* Loop through all messages */
	for (i = 0; i < num; i++) {
		pmsg = &msgs[i];

		/* Send restart for subsequent messages */
		if ((i != 0) && ((pmsg->flags & I2C_M_NOSTART) == 0)) {
			rc = bcm_kona_send_i2c_cmd(dev, BCM_CMD_RESTART);
			if (rc < 0) {
				printf("restart cmd failed rc = %d\n", rc);
				goto xfer_send_stop;
			}
		}

		/* Send slave address */
		if (!(pmsg->flags & I2C_M_NOSTART)) {
			rc = bcm_kona_i2c_do_addr(dev, pmsg);
			if (rc < 0) {
				debug("NAK from addr %2.2x msg#%d rc = %d\n",
				      pmsg->addr, i, rc);
				goto xfer_send_stop;
			}
		}

		/* Perform data transfer */
		if (pmsg->flags & I2C_M_RD) {
			rc = bcm_kona_i2c_read_fifo(dev, pmsg);
			if (rc < 0) {
				printf("read failure\n");
				goto xfer_send_stop;
			}
		} else {
			rc = bcm_kona_i2c_write_fifo(dev, pmsg);
			if (rc < 0) {
				printf("write failure");
				goto xfer_send_stop;
			}
		}
	}

	rc = num;

xfer_send_stop:
	/* Send a STOP command */
	bcm_kona_send_i2c_cmd(dev, BCM_CMD_STOP);

xfer_disable_pad:
	/* Disable pad output */
	writel(PADCTL_PAD_OUT_EN_MASK, dev->base + PADCTL_OFFSET);

	/* Stop internal clock */
	bcm_kona_i2c_disable_clock(dev);

	return rc;
}

static uint bcm_kona_i2c_assign_bus_speed(struct bcm_kona_i2c_dev *dev,
					  uint speed)
{
	switch (speed) {
	case 100000:
		dev->std_cfg = &std_cfg_table[BCM_SPD_100K];
		break;
	case 400000:
		dev->std_cfg = &std_cfg_table[BCM_SPD_400K];
		break;
	case 1000000:
		dev->std_cfg = &std_cfg_table[BCM_SPD_1MHZ];
		break;
	default:
		printf("%d hz bus speed not supported\n", speed);
		return -EINVAL;
	}
	dev->speed = speed;
	return 0;
}

static void bcm_kona_i2c_init(struct bcm_kona_i2c_dev *dev)
{
	/* Parse bus speed */
	bcm_kona_i2c_assign_bus_speed(dev, dev->speed);

	/* Enable internal clocks */
	bcm_kona_i2c_enable_clock(dev);

	/* Configure internal dividers */
	bcm_kona_i2c_config_timing(dev);

	/* Disable timeout */
	writel(0, dev->base + TOUT_OFFSET);

	/* Enable autosense */
	bcm_kona_i2c_enable_autosense(dev);

	/* Enable TX FIFO */
	writel(TXFCR_FIFO_FLUSH_MASK | TXFCR_FIFO_EN_MASK,
	       dev->base + TXFCR_OFFSET);

	/* Mask all interrupts */
	writel(0, dev->base + IER_OFFSET);

	/* Clear all pending interrupts */
	writel(ISR_CMDBUSY_MASK |
	       ISR_READ_COMPLETE_MASK |
	       ISR_SES_DONE_MASK |
	       ISR_ERR_MASK |
	       ISR_TXFIFOEMPTY_MASK | ISR_NOACK_MASK, dev->base + ISR_OFFSET);

	/* Enable the controller but leave it idle */
	bcm_kona_i2c_send_cmd_to_ctrl(dev, BCM_CMD_NOACTION);

	/* Disable pad output */
	writel(PADCTL_PAD_OUT_EN_MASK, dev->base + PADCTL_OFFSET);
}

/*
 * uboot layer
 */
struct bcm_kona_i2c_dev *kona_get_dev(struct i2c_adapter *adap)
{
	return &g_i2c_devs[adap->hwadapnr];
}

static void kona_i2c_init(struct i2c_adapter *adap, int speed, int slaveaddr)
{
	struct bcm_kona_i2c_dev *dev = kona_get_dev(adap);

	if (clk_bsc_enable(dev->base))
		return;

	bcm_kona_i2c_init(dev);
}

static int kona_i2c_read(struct i2c_adapter *adap, uchar chip, uint addr,
			 int alen, uchar *buffer, int len)
{
	/* msg[0] writes the addr, msg[1] reads the data */
	struct kona_i2c_msg msg[2];
	unsigned char msgbuf0[64];
	struct bcm_kona_i2c_dev *dev = kona_get_dev(adap);

	msg[0].addr = chip;
	msg[0].flags = 0;
	msg[0].len = 1;
	msg[0].buf = msgbuf0;	/* msgbuf0 contains incrementing reg addr */

	msg[1].addr = chip;
	msg[1].flags = I2C_M_RD;
	/* msg[1].buf dest ptr increments each read */

	msgbuf0[0] = (unsigned char)addr;
	msg[1].buf = buffer;
	msg[1].len = len;
	if (bcm_kona_i2c_xfer(dev, msg, 2) < 0) {
		/* Sending 2 i2c messages */
		kona_i2c_init(adap, adap->speed, adap->slaveaddr);
		debug("I2C read: I/O error\n");
		return -EIO;
	}
	return 0;
}

static int kona_i2c_write(struct i2c_adapter *adap, uchar chip, uint addr,
			  int alen, uchar *buffer, int len)
{
	struct kona_i2c_msg msg[1];
	unsigned char msgbuf0[64];
	unsigned int i;
	struct bcm_kona_i2c_dev *dev = kona_get_dev(adap);

	msg[0].addr = chip;
	msg[0].flags = 0;
	msg[0].len = 2;		/* addr byte plus data */
	msg[0].buf = msgbuf0;

	for (i = 0; i < len; i++) {
		msgbuf0[0] = addr++;
		msgbuf0[1] = buffer[i];
		if (bcm_kona_i2c_xfer(dev, msg, 1) < 0) {
			kona_i2c_init(adap, adap->speed, adap->slaveaddr);
			debug("I2C write: I/O error\n");
			return -EIO;
		}
	}
	return 0;
}

static int kona_i2c_probe(struct i2c_adapter *adap, uchar chip)
{
	uchar tmp;

	/*
	 * read addr 0x0 of the given chip.
	 */
	return kona_i2c_read(adap, chip, 0x0, 1, &tmp, 1);
}

static uint kona_i2c_set_bus_speed(struct i2c_adapter *adap, uint speed)
{
	struct bcm_kona_i2c_dev *dev = kona_get_dev(adap);
	return bcm_kona_i2c_assign_bus_speed(dev, speed);
}

/*
 * Register kona i2c adapters. Keep the order below so
 * that the bus number matches the adapter number.
 */
#define DEF_ADAPTER(num) \
U_BOOT_I2C_ADAP_COMPLETE(kona##num, kona_i2c_init, kona_i2c_probe, \
			 kona_i2c_read, kona_i2c_write, \
			 kona_i2c_set_bus_speed, DEF_SPD, 0x00, num)

#ifdef CONFIG_SYS_I2C_BASE0
	DEF_ADAPTER(0)
#endif
#ifdef CONFIG_SYS_I2C_BASE1
	DEF_ADAPTER(1)
#endif
#ifdef CONFIG_SYS_I2C_BASE2
	DEF_ADAPTER(2)
#endif
#ifdef CONFIG_SYS_I2C_BASE3
	DEF_ADAPTER(3)
#endif
#ifdef CONFIG_SYS_I2C_BASE4
	DEF_ADAPTER(4)
#endif
#ifdef CONFIG_SYS_I2C_BASE5
	DEF_ADAPTER(5)
#endif
