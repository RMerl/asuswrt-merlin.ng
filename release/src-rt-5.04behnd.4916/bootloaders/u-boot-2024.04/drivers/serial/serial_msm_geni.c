// SPDX-License-Identifier: GPL-2.0+
/*
 * Qualcomm GENI serial engine UART driver
 *
 * (C) Copyright 2021 Dzmitry Sankouski <dsankouski@gmail.com>
 *
 * Based on Linux driver.
 */

#include <asm/io.h>
#include <clk.h>
#include <common.h>
#include <dm.h>
#include <errno.h>
#include <linux/delay.h>
#include <linux/time.h>
#include <misc.h>
#include <serial.h>

#define UART_OVERSAMPLING	32
#define STALE_TIMEOUT	160

/* Registers*/
#define GENI_FORCE_DEFAULT_REG	0x20
#define GENI_SER_M_CLK_CFG	0x48
#define GENI_SER_S_CLK_CFG	0x4C
#define SE_HW_PARAM_0	0xE24
#define SE_GENI_STATUS	0x40
#define SE_GENI_S_CMD0	0x630
#define SE_GENI_S_CMD_CTRL_REG	0x634
#define SE_GENI_S_IRQ_CLEAR	0x648
#define SE_GENI_S_IRQ_STATUS	0x640
#define SE_GENI_S_IRQ_EN	0x644
#define SE_GENI_M_CMD0	0x600
#define SE_GENI_M_CMD_CTRL_REG	0x604
#define SE_GENI_M_IRQ_CLEAR	0x618
#define SE_GENI_M_IRQ_STATUS	0x610
#define SE_GENI_M_IRQ_EN	0x614
#define SE_GENI_TX_FIFOn	0x700
#define SE_GENI_RX_FIFOn	0x780
#define SE_GENI_TX_FIFO_STATUS	0x800
#define SE_GENI_RX_FIFO_STATUS	0x804
#define SE_GENI_TX_WATERMARK_REG	0x80C
#define SE_GENI_TX_PACKING_CFG0	0x260
#define SE_GENI_TX_PACKING_CFG1	0x264
#define SE_GENI_RX_PACKING_CFG0	0x284
#define SE_GENI_RX_PACKING_CFG1	0x288
#define SE_UART_RX_STALE_CNT	0x294
#define SE_UART_TX_TRANS_LEN	0x270
#define SE_UART_TX_STOP_BIT_LEN	0x26c
#define SE_UART_TX_WORD_LEN	0x268
#define SE_UART_RX_WORD_LEN	0x28c
#define SE_UART_TX_TRANS_CFG	0x25c
#define SE_UART_TX_PARITY_CFG	0x2a4
#define SE_UART_RX_TRANS_CFG	0x280
#define SE_UART_RX_PARITY_CFG	0x2a8

#define M_TX_FIFO_WATERMARK_EN	(BIT(30))
#define DEF_TX_WM	2
/* GENI_FORCE_DEFAULT_REG fields */
#define FORCE_DEFAULT	(BIT(0))

#define S_CMD_ABORT_EN	(BIT(5))

#define UART_START_READ	0x1

/* GENI_M_CMD_CTRL_REG */
#define M_GENI_CMD_CANCEL	(BIT(2))
#define M_GENI_CMD_ABORT	(BIT(1))
#define M_GENI_DISABLE	(BIT(0))

#define M_CMD_ABORT_EN	(BIT(5))
#define M_CMD_DONE_EN	(BIT(0))
#define M_CMD_DONE_DISABLE_MASK	(~M_CMD_DONE_EN)

#define S_GENI_CMD_ABORT	(BIT(1))

/* GENI_S_CMD0 fields */
#define S_OPCODE_MSK	(GENMASK(31, 27))
#define S_PARAMS_MSK	(GENMASK(26, 0))

/* GENI_STATUS fields */
#define M_GENI_CMD_ACTIVE	(BIT(0))
#define S_GENI_CMD_ACTIVE	(BIT(12))
#define M_CMD_DONE_EN	(BIT(0))
#define S_CMD_DONE_EN	(BIT(0))

#define M_OPCODE_SHIFT	27
#define S_OPCODE_SHIFT	27
#define M_TX_FIFO_WATERMARK_EN	(BIT(30))
#define UART_START_TX	0x1
#define UART_CTS_MASK	(BIT(1))
#define M_SEC_IRQ_EN	(BIT(31))
#define TX_FIFO_WC_MSK	(GENMASK(27, 0))
#define RX_FIFO_WC_MSK	(GENMASK(24, 0))

#define S_RX_FIFO_WATERMARK_EN	(BIT(26))
#define S_RX_FIFO_LAST_EN	(BIT(27))
#define M_RX_FIFO_WATERMARK_EN	(BIT(26))
#define M_RX_FIFO_LAST_EN	(BIT(27))

/* GENI_SER_M_CLK_CFG/GENI_SER_S_CLK_CFG */
#define SER_CLK_EN	(BIT(0))
#define CLK_DIV_MSK	(GENMASK(15, 4))
#define CLK_DIV_SHFT	4

/* SE_HW_PARAM_0 fields */
#define TX_FIFO_WIDTH_MSK	(GENMASK(29, 24))
#define TX_FIFO_WIDTH_SHFT	24
#define TX_FIFO_DEPTH_MSK	(GENMASK(21, 16))
#define TX_FIFO_DEPTH_SHFT	16

/* GENI SE QUP Registers */
#define QUP_HW_VER_REG		0x4
#define  QUP_SE_VERSION_2_5	0x20050000

/*
 * Predefined packing configuration of the serial engine (CFG0, CFG1 regs)
 * for uart mode.
 *
 * Defines following configuration:
 * - Bits of data per transfer word             8
 * - Number of words per fifo element           4
 * - Transfer from MSB to LSB or vice-versa     false
 */
#define UART_PACKING_CFG0   0xf
#define UART_PACKING_CFG1   0x0

DECLARE_GLOBAL_DATA_PTR;

struct msm_serial_data {
	phys_addr_t base;
	u32 baud;
	u32 oversampling;
};

unsigned long root_freq[] = {7372800,  14745600, 19200000, 29491200,
			     32000000, 48000000, 64000000, 80000000,
			     96000000, 100000000};

/**
 * get_clk_cfg() - Get clock rate to apply on clock supplier.
 * @clk_freq:	Desired clock frequency after build-in divider.
 *
 * Return: frequency, supported by clock supplier, multiple of clk_freq.
 */
static int get_clk_cfg(unsigned long clk_freq)
{
	for (int i = 0; i < ARRAY_SIZE(root_freq); i++) {
		if (!(root_freq[i] % clk_freq))
			return root_freq[i];
	}
	return 0;
}

/**
 * get_clk_div_rate() - Find clock supplier frequency, and calculate divisor.
 * @baud:	        Baudrate.
 * @sampling_rate:	Clock ticks per character.
 * @clk_div:	    Pointer to calculated divisor.
 *
 * This function searches for suitable frequency for clock supplier,
 * calculates divisor for internal divider, based on found frequency,
 * and stores divisor under clk_div pointer.
 *
 * Return: frequency, supported by clock supplier, multiple of clk_freq.
 */
static int get_clk_div_rate(u32 baud, u64 sampling_rate, u32 *clk_div)
{
	unsigned long ser_clk;
	unsigned long desired_clk;

	desired_clk = baud * sampling_rate;
	ser_clk = get_clk_cfg(desired_clk);
	if (!ser_clk) {
		pr_err("%s: Can't find matching DFS entry for baud %d\n",
								__func__, baud);
		return ser_clk;
	}

	*clk_div = ser_clk / desired_clk;
	return ser_clk;
}

static int geni_serial_set_clock_rate(struct udevice *dev, u64 rate)
{
	struct clk *clk;
	int ret;

	clk = devm_clk_get(dev, NULL);
	if (IS_ERR(clk))
		return PTR_ERR(clk);

	ret = clk_set_rate(clk, rate);
	return ret;
}

/**
 * geni_se_get_tx_fifo_depth() - Get the TX fifo depth of the serial engine
 * @base:	Pointer to the concerned serial engine.
 *
 * This function is used to get the depth i.e. number of elements in the
 * TX fifo of the serial engine.
 *
 * Return: TX fifo depth in units of FIFO words.
 */
static inline u32 geni_se_get_tx_fifo_depth(long base)
{
	u32 tx_fifo_depth;

	tx_fifo_depth = ((readl(base + SE_HW_PARAM_0) & TX_FIFO_DEPTH_MSK) >>
			 TX_FIFO_DEPTH_SHFT);
	return tx_fifo_depth;
}

/**
 * geni_se_get_tx_fifo_width() - Get the TX fifo width of the serial engine
 * @base:	Pointer to the concerned serial engine.
 *
 * This function is used to get the width i.e. word size per element in the
 * TX fifo of the serial engine.
 *
 * Return: TX fifo width in bits
 */
static inline u32 geni_se_get_tx_fifo_width(long base)
{
	u32 tx_fifo_width;

	tx_fifo_width = ((readl(base + SE_HW_PARAM_0) & TX_FIFO_WIDTH_MSK) >>
			 TX_FIFO_WIDTH_SHFT);
	return tx_fifo_width;
}

static inline void geni_serial_baud(phys_addr_t base_address, u32 clk_div,
				    int baud)
{
	u32 s_clk_cfg = 0;

	s_clk_cfg |= SER_CLK_EN;
	s_clk_cfg |= (clk_div << CLK_DIV_SHFT);

	writel(s_clk_cfg, base_address + GENI_SER_M_CLK_CFG);
	writel(s_clk_cfg, base_address + GENI_SER_S_CLK_CFG);
}

static int msm_serial_setbrg(struct udevice *dev, int baud)
{
	struct msm_serial_data *priv = dev_get_priv(dev);
	u64 clk_rate;
	u32 clk_div;
	int ret;

	priv->baud = baud;

	clk_rate = get_clk_div_rate(baud, priv->oversampling, &clk_div);
	ret = geni_serial_set_clock_rate(dev, clk_rate);
	if (ret < 0) {
		pr_err("%s: Couldn't set clock rate: %d\n", __func__, ret);
		return ret;
	}
	geni_serial_baud(priv->base, clk_div, baud);

	return 0;
}

/**
 * qcom_geni_serial_poll_bit() - Poll reg bit until desired value or timeout.
 * @base:	Pointer to the concerned serial engine.
 * @offset:	Offset to register address.
 * @field:	AND bitmask for desired bit.
 * @set:	Desired bit value.
 *
 * This function is used to get the width i.e. word size per element in the
 * TX fifo of the serial engine.
 *
 * Return: true, when register bit equals desired value, false, when timeout
 * reached.
 */
static bool qcom_geni_serial_poll_bit(const struct udevice *dev, int offset,
				      int field, bool set)
{
	u32 reg;
	struct msm_serial_data *priv = dev_get_priv(dev);
	unsigned int baud;
	unsigned int tx_fifo_depth;
	unsigned int tx_fifo_width;
	unsigned int fifo_bits;
	unsigned long timeout_us = 10000;

	baud = 115200;

	if (priv) {
		baud = priv->baud;
		if (!baud)
			baud = 115200;
		tx_fifo_depth = geni_se_get_tx_fifo_depth(priv->base);
		tx_fifo_width = geni_se_get_tx_fifo_width(priv->base);
		fifo_bits = tx_fifo_depth * tx_fifo_width;
		/*
		 * Total polling iterations based on FIFO worth of bytes to be
		 * sent at current baud. Add a little fluff to the wait.
		 */
		timeout_us = ((fifo_bits * USEC_PER_SEC) / baud) + 500;
	}

	timeout_us = DIV_ROUND_UP(timeout_us, 10) * 10;
	while (timeout_us) {
		reg = readl(priv->base + offset);
		if ((bool)(reg & field) == set)
			return true;
		udelay(10);
		timeout_us -= 10;
	}
	return false;
}

static void qcom_geni_serial_setup_tx(u64 base, u32 xmit_size)
{
	u32 m_cmd;

	writel(xmit_size, base + SE_UART_TX_TRANS_LEN);
	m_cmd = UART_START_TX << M_OPCODE_SHIFT;
	writel(m_cmd, base + SE_GENI_M_CMD0);
}

static inline void qcom_geni_serial_poll_tx_done(const struct udevice *dev)
{
	struct msm_serial_data *priv = dev_get_priv(dev);
	int done = 0;
	u32 irq_clear = M_CMD_DONE_EN;

	done = qcom_geni_serial_poll_bit(dev, SE_GENI_M_IRQ_STATUS,
					 M_CMD_DONE_EN, true);
	if (!done) {
		writel(M_GENI_CMD_ABORT, priv->base + SE_GENI_M_CMD_CTRL_REG);
		irq_clear |= M_CMD_ABORT_EN;
		qcom_geni_serial_poll_bit(dev, SE_GENI_M_IRQ_STATUS,
					  M_CMD_ABORT_EN, true);
	}
	writel(irq_clear, priv->base + SE_GENI_M_IRQ_CLEAR);
}

static u32 qcom_geni_serial_tx_empty(u64 base)
{
	return !readl(base + SE_GENI_TX_FIFO_STATUS);
}

/**
 * geni_se_setup_s_cmd() - Setup the secondary sequencer
 * @se:		Pointer to the concerned serial engine.
 * @cmd:	Command/Operation to setup in the secondary sequencer.
 * @params:	Parameter for the sequencer command.
 *
 * This function is used to configure the secondary sequencer with the
 * command and its associated parameters.
 */
static inline void geni_se_setup_s_cmd(u64 base, u32 cmd, u32 params)
{
	u32 s_cmd;

	s_cmd = readl(base + SE_GENI_S_CMD0);
	s_cmd &= ~(S_OPCODE_MSK | S_PARAMS_MSK);
	s_cmd |= (cmd << S_OPCODE_SHIFT);
	s_cmd |= (params & S_PARAMS_MSK);
	writel(s_cmd, base + SE_GENI_S_CMD0);
}

static void qcom_geni_serial_start_tx(u64 base)
{
	u32 irq_en;
	u32 status;

	status = readl(base + SE_GENI_STATUS);
	if (status & M_GENI_CMD_ACTIVE)
		return;

	if (!qcom_geni_serial_tx_empty(base))
		return;

	irq_en = readl(base + SE_GENI_M_IRQ_EN);
	irq_en |= M_TX_FIFO_WATERMARK_EN | M_CMD_DONE_EN;

	writel(DEF_TX_WM, base + SE_GENI_TX_WATERMARK_REG);
	writel(irq_en, base + SE_GENI_M_IRQ_EN);
}

static void qcom_geni_serial_start_rx(struct udevice *dev)
{
	u32 status;
	struct msm_serial_data *priv = dev_get_priv(dev);

	status = readl(priv->base + SE_GENI_STATUS);

	geni_se_setup_s_cmd(priv->base, UART_START_READ, 0);

	setbits_le32(priv->base + SE_GENI_S_IRQ_EN, S_RX_FIFO_WATERMARK_EN | S_RX_FIFO_LAST_EN);
	setbits_le32(priv->base + SE_GENI_M_IRQ_EN, M_RX_FIFO_WATERMARK_EN | M_RX_FIFO_LAST_EN);
}

static void qcom_geni_serial_abort_rx(struct udevice *dev)
{
	struct msm_serial_data *priv = dev_get_priv(dev);

	u32 irq_clear = S_CMD_DONE_EN | S_CMD_ABORT_EN;

	writel(S_GENI_CMD_ABORT, priv->base + SE_GENI_S_CMD_CTRL_REG);
	qcom_geni_serial_poll_bit(dev, SE_GENI_S_CMD_CTRL_REG,
					S_GENI_CMD_ABORT, false);
	writel(irq_clear, priv->base + SE_GENI_S_IRQ_CLEAR);
	writel(FORCE_DEFAULT, priv->base + GENI_FORCE_DEFAULT_REG);
}

static void msm_geni_serial_setup_rx(struct udevice *dev)
{
	struct msm_serial_data *priv = dev_get_priv(dev);

	qcom_geni_serial_abort_rx(dev);

	writel(UART_PACKING_CFG0, priv->base + SE_GENI_RX_PACKING_CFG0);
	writel(UART_PACKING_CFG1, priv->base + SE_GENI_RX_PACKING_CFG1);

	geni_se_setup_s_cmd(priv->base, UART_START_READ, 0);

	setbits_le32(priv->base + SE_GENI_S_IRQ_EN, S_RX_FIFO_WATERMARK_EN | S_RX_FIFO_LAST_EN);
	setbits_le32(priv->base + SE_GENI_M_IRQ_EN, M_RX_FIFO_WATERMARK_EN | M_RX_FIFO_LAST_EN);
}

static int msm_serial_putc(struct udevice *dev, const char ch)
{
	struct msm_serial_data *priv = dev_get_priv(dev);

	writel(DEF_TX_WM, priv->base + SE_GENI_TX_WATERMARK_REG);
	qcom_geni_serial_setup_tx(priv->base, 1);

	qcom_geni_serial_poll_bit(dev, SE_GENI_M_IRQ_STATUS,
				  M_TX_FIFO_WATERMARK_EN, true);

	writel(ch, priv->base + SE_GENI_TX_FIFOn);
	writel(M_TX_FIFO_WATERMARK_EN, priv->base + SE_GENI_M_IRQ_CLEAR);

	qcom_geni_serial_poll_tx_done(dev);

	return 0;
}

static int msm_serial_getc(struct udevice *dev)
{
	struct msm_serial_data *priv = dev_get_priv(dev);
	u32 rx_fifo;
	u32 m_irq_status;
	u32 s_irq_status;

	writel(1 << S_OPCODE_SHIFT, priv->base + SE_GENI_S_CMD0);

	qcom_geni_serial_poll_bit(dev, SE_GENI_M_IRQ_STATUS, M_SEC_IRQ_EN,
				  true);

	m_irq_status = readl(priv->base + SE_GENI_M_IRQ_STATUS);
	s_irq_status = readl(priv->base + SE_GENI_S_IRQ_STATUS);
	writel(m_irq_status, priv->base + SE_GENI_M_IRQ_CLEAR);
	writel(s_irq_status, priv->base + SE_GENI_S_IRQ_CLEAR);
	qcom_geni_serial_poll_bit(dev, SE_GENI_RX_FIFO_STATUS, RX_FIFO_WC_MSK,
				  true);

	if (!readl(priv->base + SE_GENI_RX_FIFO_STATUS))
		return 0;

	rx_fifo = readl(priv->base + SE_GENI_RX_FIFOn);
	return rx_fifo & 0xff;
}

static int msm_serial_pending(struct udevice *dev, bool input)
{
	struct msm_serial_data *priv = dev_get_priv(dev);

	if (input)
		return readl(priv->base + SE_GENI_RX_FIFO_STATUS) &
			   RX_FIFO_WC_MSK;
	else
		return readl(priv->base + SE_GENI_TX_FIFO_STATUS) &
			   TX_FIFO_WC_MSK;

	return 0;
}

static const struct dm_serial_ops msm_serial_ops = {
	.putc = msm_serial_putc,
	.pending = msm_serial_pending,
	.getc = msm_serial_getc,
	.setbrg = msm_serial_setbrg,
};

static int geni_set_oversampling(struct udevice *dev)
{
	struct msm_serial_data *priv = dev_get_priv(dev);
	ofnode parent_node = ofnode_get_parent(dev_ofnode(dev));
	u32 geni_se_version;
	fdt_addr_t addr;

	priv->oversampling = UART_OVERSAMPLING;

	/*
	 * It could happen that GENI SE IP is missing in the board's device
	 * tree or GENI UART node is a direct child of SoC device tree node.
	 */
	if (!ofnode_device_is_compatible(parent_node, "qcom,geni-se-qup")) {
		pr_err("%s: UART node must be a child of geniqup node\n",
		       __func__);
		return -ENODEV;
	}

	/* Read the HW_VER register relative to the parents address space */
	addr = ofnode_get_addr(parent_node);
	geni_se_version = readl(addr + QUP_HW_VER_REG);

	if (geni_se_version >= QUP_SE_VERSION_2_5)
		priv->oversampling /= 2;

	return 0;
}

static inline void geni_serial_init(struct udevice *dev)
{
	struct msm_serial_data *priv = dev_get_priv(dev);
	phys_addr_t base_address = priv->base;
	u32 tx_trans_cfg;
	u32 tx_parity_cfg = 0; /* Disable Tx Parity */
	u32 rx_trans_cfg = 0;
	u32 rx_parity_cfg = 0; /* Disable Rx Parity */
	u32 stop_bit_len = 0;  /* Default stop bit length - 1 bit */
	u32 bits_per_char;

	/*
	 * Ignore Flow control.
	 * n = 8.
	 */
	tx_trans_cfg = UART_CTS_MASK;
	bits_per_char = BITS_PER_BYTE;

	/*
	 * Make an unconditional cancel on the main sequencer to reset
	 * it else we could end up in data loss scenarios.
	 */
	qcom_geni_serial_poll_tx_done(dev);
	qcom_geni_serial_abort_rx(dev);

	writel(UART_PACKING_CFG0, base_address + SE_GENI_TX_PACKING_CFG0);
	writel(UART_PACKING_CFG1, base_address + SE_GENI_TX_PACKING_CFG1);
	writel(UART_PACKING_CFG0, base_address + SE_GENI_RX_PACKING_CFG0);
	writel(UART_PACKING_CFG1, base_address + SE_GENI_RX_PACKING_CFG1);

	writel(tx_trans_cfg, base_address + SE_UART_TX_TRANS_CFG);
	writel(tx_parity_cfg, base_address + SE_UART_TX_PARITY_CFG);
	writel(rx_trans_cfg, base_address + SE_UART_RX_TRANS_CFG);
	writel(rx_parity_cfg, base_address + SE_UART_RX_PARITY_CFG);
	writel(bits_per_char, base_address + SE_UART_TX_WORD_LEN);
	writel(bits_per_char, base_address + SE_UART_RX_WORD_LEN);
	writel(stop_bit_len, base_address + SE_UART_TX_STOP_BIT_LEN);
}

static int msm_serial_probe(struct udevice *dev)
{
	struct msm_serial_data *priv = dev_get_priv(dev);
	int ret;

	ret = geni_set_oversampling(dev);
	if (ret < 0)
		return ret;

	/* No need to reinitialize the UART after relocation */
	if (gd->flags & GD_FLG_RELOC)
		return 0;

	geni_serial_init(dev);
	msm_geni_serial_setup_rx(dev);
	qcom_geni_serial_start_rx(dev);
	qcom_geni_serial_start_tx(priv->base);

	return 0;
}

static int msm_serial_ofdata_to_platdata(struct udevice *dev)
{
	struct msm_serial_data *priv = dev_get_priv(dev);

	priv->base = dev_read_addr(dev);
	if (priv->base == FDT_ADDR_T_NONE)
		return -EINVAL;

	return 0;
}

static const struct udevice_id msm_serial_ids[] = {
	{ .compatible = "qcom,geni-debug-uart" },
	{ }
};

U_BOOT_DRIVER(serial_msm_geni) = {
	.name = "serial_msm_geni",
	.id = UCLASS_SERIAL,
	.of_match = msm_serial_ids,
	.of_to_plat = msm_serial_ofdata_to_platdata,
	.priv_auto = sizeof(struct msm_serial_data),
	.probe = msm_serial_probe,
	.ops = &msm_serial_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

#ifdef CONFIG_DEBUG_UART_MSM_GENI

static struct msm_serial_data init_serial_data = {
	.base = CONFIG_VAL(DEBUG_UART_BASE)
};

/* Serial dumb device, to reuse driver code */
static struct udevice init_dev = {
	.priv_ = &init_serial_data,
};

#include <debug_uart.h>

#define CLK_DIV (CONFIG_DEBUG_UART_CLOCK / \
					(CONFIG_BAUDRATE * UART_OVERSAMPLING))
#if (CONFIG_DEBUG_UART_CLOCK % (CONFIG_BAUDRATE * UART_OVERSAMPLING) > 0)
#error Clocks cannot be set at early debug. Change CONFIG_BAUDRATE
#endif

static inline void _debug_uart_init(void)
{
	phys_addr_t base = CONFIG_VAL(DEBUG_UART_BASE);

	geni_serial_init(&init_dev);
	geni_serial_baud(base, CLK_DIV, CONFIG_BAUDRATE);
	qcom_geni_serial_start_tx(base);
}

static inline void _debug_uart_putc(int ch)
{
	phys_addr_t base = CONFIG_VAL(DEBUG_UART_BASE);

	writel(DEF_TX_WM, base + SE_GENI_TX_WATERMARK_REG);
	qcom_geni_serial_setup_tx(base, 1);
	qcom_geni_serial_poll_bit(&init_dev, SE_GENI_M_IRQ_STATUS,
				  M_TX_FIFO_WATERMARK_EN, true);

	writel(ch, base + SE_GENI_TX_FIFOn);
	writel(M_TX_FIFO_WATERMARK_EN, base + SE_GENI_M_IRQ_CLEAR);
	qcom_geni_serial_poll_tx_done(&init_dev);
}

DEBUG_UART_FUNCS

#endif
